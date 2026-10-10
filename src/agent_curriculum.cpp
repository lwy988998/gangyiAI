#include "agent_curriculum.hpp"
#include "agent_lessons.hpp"
#include "agent_preferences.hpp"
#include "question_evidence.hpp"
#include "progress_service.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>

namespace gangyi {
namespace {
using Json = nlohmann::json;
Json parse(const std::string& value) { const auto j = Json::parse(value, nullptr, false); return j.is_object() ? j : Json::object(); }
std::string now() {
    const auto clock = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); std::tm value{};
#ifdef _WIN32
    gmtime_s(&value, &clock);
#else
    gmtime_r(&clock, &value);
#endif
    std::ostringstream out; out << std::put_time(&value, "%Y-%m-%dT%H:%M:%SZ"); return out.str();
}
std::string id(const char* prefix) {
    static thread_local std::mt19937_64 random(std::random_device{}());
    std::ostringstream out; out << prefix << '-' << std::hex << random() << random(); return out.str();
}
std::string text(const Json& value, const char* key) {
    if (!value.value(key, Json()).is_string() || value.at(key).get<std::string>().empty()) throw std::invalid_argument(std::string(key) + "不能为空");
    return value.at(key);
}
AgentAccess accessFor(const Json& task) { return {task.at("scopeId"), task.at("courseIds").get<std::vector<std::string>>()}; }
Course courseFor(Database& db, const AgentAccess& access, const std::string& courseId) {
    const auto course = db.getCourse(courseId);
    if (!course || course->status != "active" || std::find(access.courseIds.begin(), access.courseIds.end(), courseId) == access.courseIds.end())
        throw std::invalid_argument("课程不存在或无权访问");
    return *course;
}
std::string alias(const std::string& course, int phase, int topic) {
    return "agent-legacy:" + course + ":" + std::to_string(phase) + ":" + std::to_string(topic);
}
CourseSnapshot latest(Database& db, const std::string& course) {
    const auto rows = db.findSnapshotsByCourseId(course);
    if (rows.empty()) throw std::invalid_argument("课程尚无大纲");
    return *std::max_element(rows.begin(), rows.end(), [](const auto& a, const auto& b) { return a.version < b.version; });
}
Json normalizeOutline(Json payload, const std::string& course) {
    if (!payload.value("courseStructure", Json()).is_array() || payload.at("courseStructure").empty())
        throw std::invalid_argument("课程需要真实 AI 选择的阶段与知识点");
    std::set<std::string> seen; int phase = 0;
    for (auto& stage : payload["courseStructure"]) {
        ++phase; text(stage, "stage");
        if (!stage.value("topics", Json()).is_array() || stage.at("topics").empty()) throw std::invalid_argument("阶段知识点尚不完整");
        int topic = 0;
        for (auto& item : stage["topics"]) {
            ++topic;
            if (item.is_string()) item = Json{{"title", item}};
            text(item, "title");
            if (!item.contains("id")) item["id"] = "topic:" + course + ":" + std::to_string(phase) + ":" + std::to_string(topic);
            const auto stable = text(item, "id"); if (!seen.insert(stable).second) throw std::invalid_argument("知识点身份重复");
            if (!item.contains("legacyPhaseIndex")) item["legacyPhaseIndex"] = phase;
            if (!item.contains("legacyTopicIndex")) item["legacyTopicIndex"] = topic;
        }
    }
    // 老页面的路线展示是同一大纲的投影，不额外决定教学内容或题量。
    const auto original = payload.value("roadmap", Json::array()); payload["roadmap"] = Json::array();
    size_t index = 0;
    for (const auto& stage : payload.at("courseStructure")) {
        auto projection = original.is_array() && index < original.size() && original.at(index).is_object() ? original.at(index) : Json::object();
        projection["name"] = stage.at("stage"); projection["steps"] = stage.at("topics");
        payload["roadmap"].push_back(projection); ++index;
    }
    return payload;
}
Json topicMap(const Json& payload) {
    Json result = Json::object();
    for (const auto& stage : payload.at("courseStructure")) for (const auto& topic : stage.at("topics")) result[topic.at("id").get<std::string>()] = topic;
    return result;
}
}

Json agentOutline(Database& db, const AgentAccess& access, const std::string& courseId) {
    courseFor(db, access, courseId); return normalizeOutline(parse(latest(db, courseId).payload), courseId);
}
void agentValidateOutline(Database& db, const std::string& courseId, const Json& before, const Json& after) {
    const auto previous = topicMap(before), proposed = topicMap(after);
    std::map<std::pair<int, int>, std::string> oldAliases, aliases;
    for (const auto& entry : previous.items()) oldAliases[{entry.value().at("legacyPhaseIndex"), entry.value().at("legacyTopicIndex")}] = entry.key();
    for (const auto& entry : proposed.items()) {
        const auto item = entry.value();
        if (!item.at("legacyPhaseIndex").is_number_integer() || !item.at("legacyTopicIndex").is_number_integer()) throw std::invalid_argument("知识点映射无效");
        const int phase = item.at("legacyPhaseIndex"), topic = item.at("legacyTopicIndex"); const auto position = std::make_pair(phase, topic);
        if (phase < 1 || topic < 1 || aliases.count(position)) throw std::invalid_argument("原链接映射重复或无效");
        aliases[position] = entry.key();
        if (previous.contains(entry.key()) && (previous.at(entry.key()).at("legacyPhaseIndex") != phase || previous.at(entry.key()).at("legacyTopicIndex") != topic))
            throw std::invalid_argument("重排不能改变知识点原有映射");
        if (oldAliases.count(position) && oldAliases.at(position) != entry.key()) throw std::invalid_argument("新知识点不能占用其他知识点的旧链接");
    }
    for (const auto& entry : previous.items()) {
        const auto topic = entry.value(); const int phase = topic.at("legacyPhaseIndex"), index = topic.at("legacyTopicIndex");
        const auto progress = db.findLearningCardProgress(courseId, phase, index);
        const bool protectedTopic = db.findLearningSession(courseId, phase, index).has_value() ||
            !db.profileMeta(alias(courseId, phase, index)).empty() || (progress && progress->status != "not_started");
        if (protectedTopic && (!proposed.contains(entry.key()) || proposed.at(entry.key()) != topic)) throw std::invalid_argument("已展示或在学知识点及原有身份不能改写或删除");
    }
}

Json agentLegacyLesson(Database& db, const AgentAccess& access, const Json& scope) {
    const auto course = courseFor(db, access, text(scope, "courseId"));
    int phase = scope.value("phaseIndex", 1), topic = scope.value("topicIndex", 1);
    if (scope.value("topicId", "") != "") {
        const auto outline = normalizeOutline(parse(latest(db, course.id).payload), course.id);
        bool found = false;
        for (const auto& stage : outline.at("courseStructure")) for (const auto& item : stage.at("topics")) if (item.at("id") == scope.at("topicId")) {
            phase = item.at("legacyPhaseIndex"); topic = item.at("legacyTopicIndex"); found = true;
        }
        if (!found) throw std::invalid_argument("原知识点已调整，请查看最新课程路线");
    }
    const auto preparedTaskId = scope.value("lessonTaskId", "");
    auto session = db.findLearningSession(course.id, phase, topic);
    Json prepared;
    if (!preparedTaskId.empty()) {
        const auto row = db.getClassroomActivity("prepared-lesson:" + preparedTaskId);
        if (!row || row->kind != "prepared-lesson" || row->courseId != course.id) throw std::invalid_argument("原已备课堂不存在或所属课程不匹配");
        prepared = parse(row->payload);
        if (prepared.value("lessonTaskId", "") != preparedTaskId || !prepared.value("content", Json()).is_object()) throw std::invalid_argument("原已备课堂内容无效");
        phase = prepared.at("phaseIndex"); topic = prepared.at("topicIndex");
        LearningSession original; original.id = "prepared-" + preparedTaskId; original.courseId = course.id;
        original.phaseIndex = phase; original.topicIndex = topic; original.content = prepared.at("content").dump();
        original.title = prepared.value("topicTitle", course.title); session = original;
    }
    const auto key = preparedTaskId.empty() ? alias(course.id, phase, topic) : "agent-legacy-prepared:" + preparedTaskId;
    const auto previous = db.profileMeta(key);
    if (!previous.empty()) return {{"lessonId", previous}, {"href", "/agent-classroom.html?lessonId=" + previous}};
    if (!session) return Json::object();
    Json lesson = {{"id", "legacy-" + session->id}, {"scopeId", access.scopeId}, {"courseId", course.id},
        {"title", session->title}, {"purpose", "继续原有课堂，保留已展示课件与实际作答。"}, {"status", "ready"},
        {"version", 1}, {"sourceLearningVersion", db.learningRevision()}, {"entered", true}, {"updatedAt", now()},
        {"phaseIndex", phase}, {"topicIndex", topic}, {"legacySessionId", session->id}, {"sections", Json::array()}, {"model", ""}};
    if (!preparedTaskId.empty()) {
        const auto kind = prepared.value("kind", "lesson");
        lesson["intent"] = kind == "lesson" ? "advance" : kind == "review" ? "review" : "reinforce";
        lesson["completed"] = prepared.value("progress", Json::object()).value("status", "") == "completed";
        if (kind != "lesson") lesson.erase("legacySessionId");
    }
    const auto content = parse(session->content), blocks = content.value("blocks", content);
    const auto add = [&](const std::string& kind, int index, const Json& input, const std::string& title, bool question) {
        Json section = {{"id", lesson.at("id").get<std::string>() + ":" + kind + ":" + std::to_string(index)},
            {"kind", question ? "question" : "explanation"}, {"legacyKind", kind}, {"legacyIndex", index},
            {"title", title}, {"version", content.value("contentVersion", 1)}, {"displayed", true}};
        if (question) {
            auto value = input;
            if (!value.value("question", Json()).is_string()) value["question"] = value.value("task", value.value("content", ""));
            if (value.value("question", "").empty()) return;
            value["type"] = value.value("type", value.contains("options") ? "choice" : "open");
            if (!value.contains("expectedAnswer")) value["expectedAnswer"] = value.value("solution", value.value("check", ""));
            section["question"] = value; section["questionId"] = questionIdentity(course.id, value);
            section["legacyDialog"] = Json::array();
            const auto sourceKey = "classroom:" + course.id + ":" + std::to_string(phase) + ":" + std::to_string(topic) +
                (preparedTaskId.empty() ? "" : ":task:" + preparedTaskId) + ":" + kind + ":" + std::to_string(index);
            const auto history = db.getClassroomActivity(sourceKey + ":dialogue");
            if (history) for (const auto& turn : parse(history->payload).value("turns", Json::array())) {
                if (turn.value("user", Json()).is_string()) section["legacyDialog"].push_back({{"role", "user"}, {"text", turn.at("user")}});
                if (turn.value("assistant", Json()).is_string() && !turn.at("assistant").get<std::string>().empty())
                    section["legacyDialog"].push_back({{"role", "assistant"}, {"text", turn.at("assistant")}, {"partial", turn.value("status", "") != "ready"}});
            }
            const auto previous = db.getClassroomActivity(sourceKey);
            if (section.at("legacyDialog").empty() && previous) {
                const auto record = parse(previous->payload);
                if (record.value("status", "") == "answered" && record.contains("answer")) {
                    const auto answer = record.at("answer");
                    const auto content = answer.is_string() ? answer.get<std::string>() : answer.dump();
                    section["legacyDialog"].push_back({{"role", "user"}, {"text", content}});
                    if (record.value("feedback", Json()).is_string() && !record.at("feedback").get<std::string>().empty())
                        section["legacyDialog"].push_back({{"role", "assistant"}, {"text", record.at("feedback")}});
                }
            }
        } else {
            std::string body;
            for (const auto* field : {"explanation", "example", "action"}) if (input.value(field, Json()).is_string()) body += input.at(field).get<std::string>() + "\n\n";
            if (body.empty()) return;
            section["body"] = body;
        }
        lesson["sections"].push_back(std::move(section));
    };
    std::map<std::string, int> legacyIndices;
    for (const auto& row : db.listClassroomActivities(course.id)) if (row.phaseIndex == phase && row.topicIndex == topic &&
        (row.kind == "diagnostic" || row.kind == "interaction")) {
        const auto input = parse(row.payload);
        if (input.value("lessonTaskId", "") != preparedTaskId) continue;
        add(row.kind, input.value("index", legacyIndices[row.kind]++), input, row.kind == "diagnostic" ? "原诊断题" : "原互动题", true);
        if (!lesson.at("sections").empty()) lesson["sections"].back()["sourceActivityId"] = row.id;
    }
    for (const auto* name : {"steps", "examples", "practice", "quiz"}) {
        const auto block = blocks.value(name, Json::object());
        const auto values = block.is_array() ? block : block.value(std::string(name) == "steps" ? "lessonSteps" : name, Json::array());
        if (!values.is_array()) continue;
        int index = 0; for (const auto& item : values) {
            add(std::string(name) == "examples" ? "example" : name, index++, item, item.value("title", std::string(name) == "steps" ? "原讲解" : "原课堂题目"), std::string(name) != "steps");
        }
    }
    if (lesson.at("sections").empty()) return Json::object();
    db.transaction([&] {
        const auto concurrent = db.profileMeta(key);
        if (!concurrent.empty()) { lesson["id"] = concurrent; return; }
        if (!db.upsert(ClassroomActivity{lesson.at("id"), course.id, "agent-lesson", lesson.dump(), now(), phase, topic})) throw std::runtime_error("历史课堂身份保存失败");
        db.setProfileMeta(key, lesson.at("id"));
    });
    return {{"lessonId", lesson.at("id")}, {"href", "/agent-classroom.html?lessonId=" + lesson.at("id").get<std::string>()}};
}

void agentUpgradeLegacyLesson(Database& db, const AgentAccess& access, const std::string& lessonId) {
    const auto row = db.getClassroomActivity(lessonId);
    if (!row || row->kind != "agent-lesson") return;
    auto lesson = parse(row->payload);
    if (lesson.value("scopeId", "") != access.scopeId) throw std::invalid_argument("无权读取该课时");
    courseFor(db, access, row->courseId);
    if (lesson.value("legacyLayoutVersion", 0) >= 560) return;
    Json content; std::optional<LearningSession> session;
    if (!lesson.value("legacySessionId", "").empty()) session = db.getLearningSession(lesson.at("legacySessionId"));
    if (session) content = parse(session->content);
    else if (lessonId.find("legacy-prepared-") == 0) {
        const auto source = db.getClassroomActivity("prepared-lesson:" + lessonId.substr(16));
        if (source) content = parse(source->payload).value("content", Json::object());
    }
    if (!content.is_object() || content.empty()) return;
    const auto blocks = content.value("blocks", content);
    const auto append = [&](const std::string& kind, const std::string& title, const std::string& body) {
        if (body.empty()) return;
        const auto id = lessonId + ":" + kind + ":0";
        for (const auto& section : lesson.at("sections")) if (section.at("id") == id) return;
        lesson["sections"].push_back({{"id", id}, {"kind", kind == "overview" ? "explanation" : "summary"},
            {"legacyKind", kind}, {"legacyIndex", 0}, {"title", title}, {"body", body},
            {"version", content.value("contentVersion", 1)}, {"displayed", false}});
    };
    const auto listText = [](const Json& items) {
        std::string body;
        if (items.is_array()) for (const auto& item : items) body += "- " + (item.is_string() ? item.get<std::string>() : item.dump()) + "\n";
        return body;
    };
    const auto overview = blocks.value("overview", Json::object());
    append("overview", overview.value("title", "本节目标与关键概念"), overview.value("summary", "") + "\n\n" + listText(overview.value("keyConcepts", Json::array())));
    const auto assessment = blocks.value("assessment", Json::object());
    append("checkpoint", "学习检查点", listText(assessment.value("checkpoint", Json::array())));
    append("commonMistakes", "常见错误提醒", listText(assessment.value("commonMistakes", Json::array())));
    const auto resourceSummary = assessment.value("resourceSummary", Json());
    append("resourceSummary", "资料说明", resourceSummary.is_string() ? resourceSummary.get<std::string>() : listText(resourceSummary));
    Json references = content.value("references", Json::array());
    if (references.empty() && session && session->references) {
        const auto saved = Json::parse(*session->references, nullptr, false);
        if (saved.is_array()) references = saved;
    }
    lesson["references"] = references;
    for (auto& section : lesson["sections"]) {
        const auto kind = section.value("legacyKind", "");
        if (kind == "example" && section.at("kind") == "question") {
            const auto question = section.at("question");
            section["kind"] = "explanation"; section["presentation"] = "example";
            section["body"] = question.value("question", question.value("content", "")) + "\n\n" + question.value("solution", question.value("explanation", ""));
            // 保留私有原题及稳定身份供历史证据引用，公开示范不再成为评分题。
        } else if (section.at("kind") == "question" && !section.contains("questionKind"))
            section["questionKind"] = kind == "diagnostic" || kind == "interaction" ? "interaction" : "practice";
    }
    lesson["legacyLayoutVersion"] = 560; lesson["version"] = lesson.value("version", 1) + 1;
    if (!db.compareClassroomActivity(ClassroomActivity{row->id, row->courseId, row->kind, lesson.dump(), now(), row->phaseIndex, row->topicIndex}, row->payload))
        throw std::invalid_argument("课时已在另一个窗口更新，请重新读取");
}

Json agentLegacyEvent(Database& db, const AgentAccess& access, const Json& body) {
    const auto resolved = body.value("lessonId", "").empty() ? agentLegacyLesson(db, access, body) : Json{{"lessonId", body.at("lessonId")}};
    if (resolved.empty()) throw std::invalid_argument("课件尚未准备，请让 AI 备课后再作答");
    const auto lesson = agentLessonView(db, access, resolved.at("lessonId"));
    for (const auto& section : lesson.at("sections")) if (section.at("kind") == "question" &&
        (body.contains("sectionId") ? section.at("id") == body.at("sectionId") :
         body.contains("questionId") ? section.at("questionId") == body.at("questionId") :
         section.value("legacyKind", section.value("questionKind", "practice")) == body.value("kind", "diagnostic") && section.value("legacyIndex", -1) == body.value("index", 0))) {
        if (body.contains("questionId") && body.at("questionId") != section.at("questionId")) throw std::invalid_argument("原题身份已变化");
        if (body.contains("sectionVersion") && body.at("sectionVersion") != section.at("version")) throw std::invalid_argument("原题版本已变化");
        if (body.contains("contentVersion") && body.at("contentVersion") != section.at("version")) throw std::invalid_argument("课件版本已变化");
        const auto answer = body.value("question", body.value("answer", Json("")));
        auto value = answer.is_string() ? answer.get<std::string>() : answer.dump();
        if (answer.is_number_integer() && section.at("question").value("options", Json()).is_array()) {
            const int index = answer.get<int>(); const auto options = section.at("question").at("options");
            if (index < 0 || index >= static_cast<int>(options.size())) throw std::invalid_argument("选择的选项不存在");
            value = "我选" + std::to_string(index + 1) + "：" + options.at(index).get<std::string>();
        }
        if (value.empty() && body.value("action", "") == "skip") value = "我跳过这道题";
        return {{"type", "question_answer"}, {"courseId", body.at("courseId")}, {"lessonId", resolved.at("lessonId")},
            {"sectionId", section.at("id")}, {"sectionVersion", section.at("version")}, {"text", value},
            {"action", body.value("action", "answer")}, {"requestId", body.value("requestId", id("legacy-input"))}};
    }
    throw std::invalid_argument("原课堂题目不存在");
}

Json agentQuestionView(Database& db, const AgentAccess& access, const Json& scope) {
    const auto resolved = scope.value("lessonId", "").empty() ? agentLegacyLesson(db, access, scope) : Json{{"lessonId", scope.at("lessonId")}};
    if (resolved.empty()) throw std::invalid_argument("课堂尚未备好，请先让 AI 准备课程");
    const auto lesson = agentLessonView(db, access, resolved.at("lessonId")); Json questions = Json::array();
    int index = 0;
    for (const auto& section : lesson.at("sections")) if (section.at("kind") == "question") {
        const auto kind = section.value("legacyKind", section.value("questionKind", "practice"));
        if (scope.value("kind", "").empty() || scope.at("kind") == kind) {
            auto question = section.at("question"); question["kind"] = kind; question["index"] = section.value("legacyIndex", index);
            question["questionId"] = section.at("questionId"); question["sectionId"] = section.at("id");
            question["sectionVersion"] = section.at("version"); question["contentVersion"] = section.at("version");
            question["lessonId"] = lesson.at("id"); question["dialog"] = section.value("dialog", Json::array());
            if (section.contains("taskId")) { question["taskId"] = section.at("taskId"); question["taskStatus"] = section.at("taskStatus"); }
            questions.push_back(std::move(question));
        }
        ++index;
    }
    return {{"ok", true}, {"lessonId", lesson.at("id")}, {"questions", questions}, {"items", questions}, {"contentVersion", lesson.at("version")}};
}

void registerAgentCurriculumTools(LearningAgent& agent) {
    agent.registerTool("revise_outline", {"依据真实表现重组未来大纲。参数 courseId,payload(完整课程大纲),reason。保留知识点id和legacyPhaseIndex/legacyTopicIndex；已展示、在学、已作答内容必须保持原文，未来知识点数量与顺序由 AI 决定；调整可撤回。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto course = courseFor(db, accessFor(task), text(args, "courseId"));
            const auto saved = latest(db, course.id); const auto before = normalizeOutline(parse(saved.payload), course.id);
            auto after = normalizeOutline(args.at("payload"), course.id); agentValidateOutline(db, course.id, before, after);
            after["generation"] = {{"source", "ai"}, {"model", source.model}, {"promptVersion", "ai-agent-v551"}, {"generatedAt", now()}};
            const auto change = agentChangeRecord(db, task, source, args, "outline", course.id, before, after);
            const Json publicChange = {{"id", change.at("id")}, {"kind", "outline"}, {"target", course.id},
                {"reason", args.at("reason")}, {"status", "applied"}, {"model", source.model}};
            return PreparedAgentTool{{{"change", publicChange}, {"version", saved.version + 1}}, [saved, after, change, course](Database& connection) {
                if (latest(connection, course.id).id != saved.id) throw std::invalid_argument("大纲版本已经变化");
                if (!connection.insert(CourseSnapshot{id("snapshot"), course.id, saved.version + 1, after.dump(), now()})) throw std::runtime_error("大纲保存失败");
                connection.markLearningDirty(); agentSaveChange(connection, change, course.id);
            }};
        }});
    agent.registerTool("complete_lesson", {"AI 根据真实互动决定本课完成，不使用固定通过率。参数 lessonId,reason；只有实际进入的课时可完成。独立巩固课不增加原大纲的完成数。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto lessonId = text(args, "lessonId"); const auto row = db.getClassroomActivity(lessonId);
            if (!row || row->kind != "agent-lesson") throw std::invalid_argument("课时不存在");
            auto lesson = parse(row->payload); courseFor(db, accessFor(task), row->courseId);
            if (lesson.at("scopeId") != task.at("scopeId") || !lesson.value("entered", false)) throw std::invalid_argument("课时尚未实际进入");
            text(args, "reason"); lesson["completed"] = true; lesson["completionReason"] = args.at("reason"); lesson["completionModel"] = source.model;
            return PreparedAgentTool{{{"completed", true}, {"lessonId", lessonId}}, [row, lesson](Database& connection) {
                const auto current = connection.getClassroomActivity(row->id);
                if (!current || current->payload != row->payload) throw std::invalid_argument("课时已有后续修改");
                if (!connection.upsert(ClassroomActivity{row->id, row->courseId, row->kind, lesson.dump(), now(), row->phaseIndex, row->topicIndex})) throw std::runtime_error("课时完成保存失败");
                if (lesson.contains("legacySessionId") || lesson.value("intent", "") == "advance") {
                    const auto course = connection.getCourse(row->courseId); if (!course) throw std::invalid_argument("课程不存在");
                    const int phase = lesson.value("phaseIndex", 0), topic = lesson.value("topicIndex", 0);
                    if (phase < 1 || topic < 1) throw std::invalid_argument("推进课缺少稳定知识点映射");
                    const auto existing = connection.findLearningCardProgress(row->courseId, phase, topic);
                    auto progress = existing.value_or(LearningCardProgress{}); if (progress.id.empty()) progress.id = id("card");
                    progress.courseId = course->id; progress.anonymousId = course->anonymousId; progress.goal = course->goal; progress.mode = course->mode;
                    progress.phaseIndex = phase; progress.topicIndex = topic; progress.phaseName = lesson.value("phaseName", ""); progress.topicTitle = lesson.value("topicTitle", lesson.at("title").get<std::string>()); progress.status = "completed";
                    if (!(existing ? connection.update(progress) : connection.insert(progress))) throw std::runtime_error("原课进度保存失败");
                    if (!recomputeCourseProgress(connection, course->id, course->anonymousId.value_or(""), course->goal)) throw std::runtime_error("总进度保存失败");
                }
            }};
        }});
    agent.registerTool("create_course", {"由 AI 自主规划并保存新课程。参数 goal,title,mode:lite|deep,summary,payload:{courseStructure:[{stage,topics:[知识点文字或{title,id}]}],roadmap可选,其他公开规划资料可选}。阶段及知识点数量由你决定，不套固定模板。", false,
        [](Database&, const Json& args, const Json& task, const AIResult& source) {
            const auto courseId = id("course"), mode = args.value("mode", "deep");
            if (mode != "deep" && mode != "lite") throw std::invalid_argument("课程模式无效");
            auto payload = normalizeOutline(args.at("payload"), courseId);
            payload["title"] = text(args, "title"); payload["summary"] = args.value("summary", "");
            payload["generation"] = {{"source", "ai"}, {"model", source.model}, {"promptVersion", "ai-agent-v551"}, {"generatedAt", now()}};
            const Course course{courseId, {}, {}, text(args, "goal"), mode, text(args, "title"), args.value("summary", ""), "ai", "active", now(), now()};
            const CourseSnapshot snapshot{id("snapshot"), courseId, 1, payload.dump(), now()};
            return PreparedAgentTool{{{"createdCourseId", courseId}, {"course", {{"id", courseId}, {"title", course.title}, {"href", "/plan?courseId=" + courseId}}}},
                [course, snapshot, scope = task.at("scopeId")](Database& db) {
                    if (!db.insert(course) || !db.insert(snapshot)) throw std::runtime_error("AI 课程保存失败");
                    db.setProfileMeta("agent-course-owner:" + course.id, scope); db.markLearningDirty();
                }};
        }});
    agent.registerTool("schedule_review", {"由 AI 根据真实证据决定复习时间。参数 courseId,title,due:YYYY-MM-DD,reason,lessonId可选,phaseIndex/topicIndex可选。没有固定复习间隔，不自动生成替代题。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto course = courseFor(db, accessFor(task), text(args, "courseId"));
            const auto due = text(args, "due"); std::tm date{}; std::istringstream input(due); input >> std::get_time(&date, "%Y-%m-%d");
            if (input.fail() || due.size() != 10) throw std::invalid_argument("复习日期无效");
            date.tm_hour = 12; std::mktime(&date); std::ostringstream normalized; normalized << std::put_time(&date, "%Y-%m-%d");
            if (normalized.str() != due) throw std::invalid_argument("复习日期不存在");
            if (args.value("lessonId", "") != "" && agentLessonView(db, accessFor(task), args.at("lessonId")).at("courseId") != course.id)
                throw std::invalid_argument("复习课时所属课程不匹配");
            const auto reviewId = id("review");
            const Json value = {{"id", reviewId}, {"courseId", course.id}, {"title", text(args, "title")}, {"due", due},
                {"reason", text(args, "reason")}, {"day", 0}, {"status", "pending"}, {"phaseIndex", args.value("phaseIndex", 0)},
                {"topicIndex", args.value("topicIndex", 0)}, {"lessonId", args.value("lessonId", "")}, {"model", source.model}};
            return PreparedAgentTool{{{"reviewId", reviewId}, {"due", due}}, [value, reviewId, course](Database& connection) {
                if (!connection.upsert(ClassroomActivity{reviewId, course.id, "review", value.dump(), now(), value.at("phaseIndex"), value.at("topicIndex")})) throw std::runtime_error("复习安排保存失败");
            }};
        }});
    agent.registerTool("read_legacy_lesson", {"读取原有课堂并建立稳定课时映射，不重写旧题。参数 courseId,phaseIndex,topicIndex或topicId。可用 read_lesson 继续读取私有评分资料。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult&) { return PreparedAgentTool{agentLegacyLesson(db, accessFor(task), args), {}}; }});
    agent.registerTool("read_outline", {"读取带稳定知识点 id 与原链接映射的最新大纲。参数 courseId。重排保留既有 id 和映射，新知识点使用新 id 和未占用映射。", true,
        [](Database& db, const Json& args, const Json& task, const AIResult&) { return PreparedAgentTool{agentOutline(db, accessFor(task), text(args, "courseId")), {}}; }});
    agent.registerTool("update_course_preview", {"保存 AI 选择的课程路线说明。参数 courseId,slides:[{title,content,bullets:[]可选}],reason。数量和顺序自行决定，不重复生成已有有效预览。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto course = courseFor(db, accessFor(task), text(args, "courseId")); const auto snapshot = latest(db, course.id);
            const auto slides = args.at("slides"); if (!slides.is_array() || slides.empty()) throw std::invalid_argument("路线说明尚未完整");
            Json shown = Json::array(); for (const auto& slide : slides) { Json value = {{"title", text(slide, "title")}, {"content", text(slide, "content")}};
                if (slide.contains("bullets")) { if (!slide.at("bullets").is_array()) throw std::invalid_argument("路线要点格式无效"); for (const auto& point : slide.at("bullets")) if (!point.is_string()) throw std::invalid_argument("路线要点应为文字"); value["bullets"] = slide.at("bullets"); } shown.push_back(value); }
            const auto key = "course-preview:" + course.id; const auto old = db.getClassroomActivity(key);
            const Json value = {{"status", "ready"}, {"slides", shown}, {"message", text(args, "reason")}, {"model", source.model}, {"assessedCourseVersion", snapshot.version}, {"learningVersion", task.at("learningVersion")}};
            return PreparedAgentTool{{{"previewSaved", true}}, [key, old, value, course, snapshot](Database& connection) {
                if (latest(connection, course.id).id != snapshot.id || !connection.compareClassroomActivity({key, course.id, "preview", value.dump(), now(), 0, 0}, old ? old->payload : "")) throw std::invalid_argument("预览或课程版本已变化");
            }};
        }});
}
}

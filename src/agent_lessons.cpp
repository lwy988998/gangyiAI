#include "agent_lessons.hpp"
#include "question_evidence.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>

namespace gangyi {
namespace {
using Json = nlohmann::json;
std::string now() {
    const auto clock = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); std::tm value{};
#ifdef _WIN32
    gmtime_s(&value, &clock);
#else
    gmtime_r(&clock, &value);
#endif
    std::ostringstream out; out << std::put_time(&value, "%Y-%m-%dT%H:%M:%SZ"); return out.str();
}
std::string newId(const char* prefix) {
    static thread_local std::mt19937_64 generator(std::random_device{}());
    std::ostringstream out; out << prefix << '-' << std::hex << generator() << generator(); return out.str();
}
AgentAccess accessFor(const Json& task) { return {task.at("scopeId"), task.at("courseIds").get<std::vector<std::string>>()}; }
void requireCourse(Database& db, const AgentAccess& access, const std::string& courseId) {
    const auto found = db.getCourse(courseId);
    if (!found || found->status != "active" || std::find(access.courseIds.begin(), access.courseIds.end(), courseId) == access.courseIds.end())
        throw std::invalid_argument("课程不存在或无权读取");
}
Json lessonFor(Database& db, const AgentAccess& access, const std::string& id) {
    const auto row = db.getClassroomActivity(id);
    if (!row || row->kind != "agent-lesson") throw std::invalid_argument("AI 课时不存在");
    auto value = Json::parse(row->payload);
    if (value.at("scopeId") != access.scopeId) throw std::invalid_argument("无权读取该课时");
    requireCourse(db, access, row->courseId); return value;
}
Json publicLesson(const Json& lesson) {
    Json result;
    for (const auto* key : {"id", "courseId", "title", "purpose", "status", "version", "updatedAt", "model", "sourceLearningVersion", "entered"})
        if (lesson.contains(key)) result[key] = lesson[key];
    result["sections"] = Json::array();
    for (const auto& section : lesson.at("sections")) {
        Json shown;
        for (const auto* key : {"id", "kind", "title", "body", "displayed", "version"}) if (section.contains(key)) shown[key] = section[key];
        if (section.at("kind") == "question") {
            shown.erase("body"); Json question;
            for (const auto* key : {"question", "options", "type"}) if (section.at("question").contains(key)) question[key] = section["question"][key];
            shown["question"] = question; shown["questionId"] = section.at("questionId");
        }
        result["sections"].push_back(std::move(shown));
    }
    return result;
}
void saveLesson(Database& db, const Json& lesson) {
    if (!db.upsert(ClassroomActivity{lesson.at("id"), lesson.at("courseId"), "agent-lesson", lesson.dump(), now(), 0, 0}))
        throw std::runtime_error("AI 课时保存失败");
}
PreparedAgentTool update(Database& db, const Json& task, const std::string& id, const Json& after, Json result) {
    const auto old = lessonFor(db, accessFor(task), id).dump();
    return {std::move(result), [old, after, access = accessFor(task), id](Database& connection) {
        if (lessonFor(connection, access, id).dump() != old) throw std::invalid_argument("课时内容已经变化，旧输出不应用");
        saveLesson(connection, after);
    }};
}
std::string text(const Json& value, const char* key) {
    if (!value.value(key, Json()).is_string() || value[key].get<std::string>().empty())
        throw std::invalid_argument(std::string(key) + " 不能为空");
    return value[key];
}
Json link(const Json& lesson) {
    return {{"id", lesson.at("id")}, {"title", lesson.at("title")}, {"courseId", lesson.at("courseId")},
        {"href", "/agent-classroom.html?lessonId=" + lesson.at("id").get<std::string>()}, {"status", lesson.at("status")}};
}
}

void registerAgentLessonTools(LearningAgent& agent) {
    agent.registerTool("create_lesson", {"为可访问课程创建独立课时，可推进、补弱、复习或跨知识学习。参数 courseId、title、purpose。返回 lessonId，后续用 append_section 添加模型自主选择的板块；不要求固定板块或题数。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto access = accessFor(task); const auto courseId = text(args, "courseId"); requireCourse(db, access, courseId);
            Json lesson = {{"id", newId("lesson")}, {"scopeId", access.scopeId}, {"courseId", courseId},
                {"title", text(args, "title")}, {"purpose", text(args, "purpose")}, {"status", "draft"},
                {"sections", Json::array()}, {"version", 1}, {"model", source.model}, {"updatedAt", now()},
                {"sourceLearningVersion", task.at("learningVersion")}, {"taskId", task.at("id")}, {"entered", false}};
            return PreparedAgentTool{{{"lessonId", lesson.at("id")}, {"status", "draft"}},
                [lesson, access](Database& connection) { requireCourse(connection, access, lesson.at("courseId")); saveLesson(connection, lesson); }};
        }});
    agent.registerTool("read_lesson", {"读取指定 lessonId 的原始教学板块、题目标准和展示状态供 AI 判断；这些内部资料不直接发送到用户页面。", true,
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            return PreparedAgentTool{lessonFor(db, accessFor(task), text(args, "lessonId")), {}};
        }});
    agent.registerTool("append_section", {"追加教学板块，不改已展示内容。参数 lessonId、section:{kind:explanation|question|summary,title,body 或 question:{question,options 可选,type,expectedAnswer,rubric}}。题目标准只放 expectedAnswer/rubric，题干与公开正文不要提前给出答案。返回板块及稳定题目标识。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto id = text(args, "lessonId"); auto lesson = lessonFor(db, accessFor(task), id);
            if (lesson.at("status") != "draft" && lesson.at("status") != "ready") throw std::invalid_argument("课时状态不允许补充");
            const auto input = args.at("section"); const auto kind = text(input, "kind");
            if (kind != "explanation" && kind != "question" && kind != "summary") throw std::invalid_argument("教学板块类型无效");
            Json section = {{"id", newId("section")}, {"kind", kind}, {"title", text(input, "title")},
                {"version", 1}, {"displayed", false}, {"model", source.model}, {"createdAt", now()}};
            if (kind == "question") {
                const auto question = input.at("question"); text(question, "question");
                if (question.contains("options")) {
                    if (!question["options"].is_array()) throw std::invalid_argument("题目选项无效");
                    for (const auto& option : question["options"]) if (!option.is_string() || option.get<std::string>().empty())
                        throw std::invalid_argument("题目选项须为非空文字");
                }
                section["question"] = question;
                Json identity = {{"question", question.at("question")}};
                if (question.contains("options")) identity["options"] = question["options"];
                section["questionId"] = questionIdentity(lesson.at("courseId"), identity);
            } else section["body"] = text(input, "body");
            lesson["sections"].push_back(section); lesson["version"] = lesson.at("version").get<int>() + 1;
            lesson["model"] = source.model; lesson["updatedAt"] = now();
            return update(db, task, id, lesson, {{"sectionId", section.at("id")}, {"questionId", section.value("questionId", "")}, {"version", lesson["version"]}});
        }});
    agent.registerTool("finish_lesson", {"AI 判断课时已准备完整后提交。参数 lessonId。只保存模型实际生成的完整板块，不强制六板块、题数或先后顺序。完成后返回进入课堂链接。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto id = text(args, "lessonId"); auto lesson = lessonFor(db, accessFor(task), id);
            if (lesson.at("sections").empty()) throw std::invalid_argument("空课时不能进入课堂");
            lesson["status"] = "ready"; lesson["model"] = source.model; lesson["updatedAt"] = now();
            lesson["sourceLearningVersion"] = task.at("learningVersion"); lesson["version"] = lesson.at("version").get<int>() + 1;
            return update(db, task, id, lesson, {{"lesson", link(lesson)}, {"version", lesson.at("version")}});
        }});
    agent.registerTool("evaluate_answer", {"真实 AI 评价实际学生回答。参数 interactionId、score(0..100 或 null)、credible(bool)、feedback、reason、uncertainty。只能引用真实学生记录；同题多轮保留稳定 questionId，提示经历不丢弃。没有可靠回答或评价不完整时不要给正式分数。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto access = accessFor(task); const auto interactionId = text(args, "interactionId");
            Json answer; std::string courseId;
            for (const auto& interaction : db.listInteractions()) if (interaction.id == interactionId && interaction.kind == "chat-user") {
                courseId = interaction.courseId.value_or(""); answer = Json::parse(interaction.payload); break;
            }
            requireCourse(db, access, courseId);
            if (answer.empty() || !answer.value("questionId", Json()).is_string() || !answer.value("questionSnapshot", Json()).is_object())
                throw std::invalid_argument("缺少可靠原题与实际作答，不能建立正式评价");
            if (answer.value("scopeId", "") != access.scopeId) throw std::invalid_argument("无权评价该回答");
            if (answer.value("action", "answer") == "skip" || answer.value("action", "answer") == "stop")
                throw std::invalid_argument("跳过和停止不作为可靠评价");
            if (!args.value("credible", Json()).is_boolean() || !args.contains("score") ||
                (!args["score"].is_null() && (!args["score"].is_number_integer() || args["score"].get<int>() < 0 || args["score"].get<int>() > 100)))
                throw std::invalid_argument("评价分数与可信状态无效");
            if (!args["credible"].get<bool>() && !args["score"].is_null()) throw std::invalid_argument("不可靠评价不能提交正式分数");
            Json value = {{"interactionId", interactionId}, {"questionId", answer["questionId"]},
                {"questionSnapshot", answer["questionSnapshot"]}, {"response", answer["text"]},
                {"assisted", answer.value("assisted", false)}, {"hintHistory", answer.value("hintHistory", Json::array())},
                {"score", args["score"]}, {"credible", args["credible"]}, {"feedback", text(args, "feedback")},
                {"reason", text(args, "reason")}, {"uncertainty", text(args, "uncertainty")},
                {"source", "ai"}, {"model", source.model}, {"status", "valid"}, {"at", now()}};
            const auto id = newId("evaluation");
            return PreparedAgentTool{{{"evaluationId", id}, {"saved", true}}, [value, id, courseId](Database& connection) {
                if (!connection.insert(LearningInteraction{id, "practice", value.dump(), now(), courseId,
                    value["interactionId"].get<std::string>(), {}})) throw std::runtime_error("真实评价保存失败");
            }};
        }});
}

void agentRecordLearnerEvent(Database& db, const AgentAccess& access, const Json& event) {
    const auto type = event.value("type", "");
    if (type != "answer" && type != "chat" && type != "feedback" && type != "question_answer") return;
    Json value = {{"text", text(event, "text")}, {"scopeId", access.scopeId}, {"action", event.value("action", "answer")},
        {"requestId", event.at("requestId")}};
    const auto courseId = event.value("courseId", "");
    if (!courseId.empty()) requireCourse(db, access, courseId);
    if (event.contains("lessonId")) {
        const auto lesson = lessonFor(db, access, text(event, "lessonId"));
        if (lesson.at("status") != "ready" || lesson.at("courseId") != courseId) throw std::invalid_argument("课程尚未准备好或所属课程不匹配");
        const auto sectionId = text(event, "sectionId"); const Json* section = nullptr;
        for (const auto& item : lesson.at("sections")) if (item.at("id") == sectionId) { section = &item; break; }
        if (!section || section->at("kind") != "question" || !event.value("sectionVersion", Json()).is_number_integer() ||
            event.at("sectionVersion") != section->at("version")) throw std::invalid_argument("题目版本已变化，请保留输入并刷新");
        value["questionId"] = section->at("questionId"); value["questionSnapshot"] = section->at("question");
        value["lessonId"] = lesson.at("id"); value["sectionId"] = sectionId;
        Json hints = Json::array();
        for (const auto& row : db.listClassroomActivities(courseId)) if (row.kind == "agent-task") {
            const auto task = Json::parse(row.payload, nullptr, false);
            if (task.is_object() && task.value("scopeId", "") == access.scopeId &&
                (task.value("status", "") == "waiting_student" || task.value("status", "") == "ready") &&
                task.value("event", Json::object()).value("sectionId", "") == sectionId &&
                task.value("message", "") != "") hints.push_back(task["message"]);
        }
        value["hintHistory"] = hints; value["assisted"] = !hints.empty();
    }
    if (!db.insert(LearningInteraction{newId("student"), "chat-user", value.dump(), now(),
        courseId.empty() ? std::optional<std::string>{} : std::optional<std::string>{courseId}, event.at("requestId").get<std::string>(), {}}))
        throw std::runtime_error("学生回答保存失败");
}

Json agentLessonView(Database& db, const AgentAccess& access, const std::string& lessonId) {
    const auto saved = lessonFor(db, access, lessonId); auto shown = publicLesson(saved);
    const auto interactions = db.listInteractions();
    const auto tasks = db.listClassroomActivities(saved.at("courseId"));
    for (auto& section : shown["sections"]) if (section.at("kind") == "question") {
        section["dialog"] = Json::array();
        for (const auto& interaction : interactions) if (interaction.kind == "chat-user") {
            const auto input = Json::parse(interaction.payload, nullptr, false);
            if (!input.is_object() || input.value("scopeId", "") != access.scopeId || input.value("lessonId", "") != lessonId ||
                input.value("sectionId", "") != section.at("id")) continue;
            section["dialog"].push_back({{"role", "user"}, {"text", input.value("text", "")}, {"at", interaction.createdAt}});
            for (const auto& row : tasks) if (row.kind == "agent-task") {
                const auto task = Json::parse(row.payload, nullptr, false);
                if (!task.is_object() || task.value("scopeId", "") != access.scopeId || task.value("requestId", "") != input.value("requestId", "")) continue;
                section["taskId"] = task.at("id"); section["taskStatus"] = task.at("status");
                if ((task.at("status") == "ready" || task.at("status") == "waiting_student") && task.value("message", "") != "")
                    section["dialog"].push_back({{"role", "assistant"}, {"text", task["message"]}, {"at", task.at("updatedAt")}});
            }
        }
    }
    return shown;
}
Json agentEnterLesson(Database& db, const AgentAccess& access, const Json& body) {
    Json result;
    db.transaction([&] {
        auto lesson = lessonFor(db, access, text(body, "lessonId"));
        if (lesson.at("status") != "ready") throw std::invalid_argument("请等待真实 AI 完整备课后进入");
        if (!lesson.value("entered", false) && lesson.at("sourceLearningVersion") != db.learningRevision())
            throw std::invalid_argument("学习情况已有更新，请让 AI 重新准备后再进入");
        lesson["entered"] = true; lesson["version"] = lesson.at("version").get<int>() + 1;
        lesson["updatedAt"] = now(); saveLesson(db, lesson); result = link(lesson);
    });
    return result;
}
}

#include "agent_lessons.hpp"
#include "question_evidence.hpp"
#include "agent_curriculum.hpp"
#include "course_service.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <map>
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
// 课堂交流绑定学生实际所在课时；备课仍可创建或补充另一课时。
void requireTeachingTarget(const Json& task, const Json& lesson) {
    const auto& event = task.at("event");
    const auto type = event.value("type", "");
    if (type != "question_answer" && type != "chat" && type != "lesson_start") return;
    const auto current = event.value("lessonId", "");
    if (current.empty()) return;
    if (lesson.at("id") != current)
        throw std::invalid_argument("args.lessonId 必须使用 event.lessonId 中的当前课堂标识；不可把本次讲解或题目保存到另一课时");
    if (!lesson.value("teachingTaskId", "").empty() && lesson.at("teachingTaskId") != task.at("id"))
        throw std::invalid_argument("本课已有更新的教学任务，旧输出不应用");
}
// AI 选择或追加的当轮问题，应在学生正在使用的页面承接；题目身份与用途仍保留。
std::string responseView(const Json& lesson, const Json& task) {
    const auto& event = task.at("event");
    std::string view = event.value("view", "");
    if (view != "learn" && view != "practice") {
        view = "learn";
        for (const auto& source : lesson.at("sections")) if (source.at("id") == event.value("sectionId", "")) {
            const auto kind = source.value("questionKind", source.value("legacyKind", "practice"));
            if (kind != "interaction" && kind != "diagnostic") view = "practice";
        }
    }
    return view;
}
void selectResponseQuestion(Json& focus, const Json& lesson, const Json& task, const Json& section) {
    const auto& event = task.at("event");
    if (event.value("lessonId", "") != lesson.at("id") || section.at("kind") != "question") return;
    const auto type = event.value("type", "");
    if (type != "question_answer" && type != "chat" && type != "lesson_start") return;
    const auto view = responseView(lesson, task);
    focus["responseSectionId"] = section.at("id"); focus["responseView"] = view;
    focus["responseTaskId"] = task.at("id");
}
Json publicMaterial(const Json& value) {
    if (value.is_array()) { Json result = Json::array(); for (const auto& item : value) result.push_back(publicMaterial(item)); return result; }
    if (!value.is_object()) return value;
    Json result = Json::object();
    for (const auto* key : {"type", "title", "text", "body", "latex", "formula", "imageUrl", "alt", "rows", "columns", "headers", "values", "cells", "table"})
        if (value.contains(key)) result[key] = publicMaterial(value.at(key));
    return result;
}
Json publicLesson(const Json& lesson) {
    Json result;
    for (const auto* key : {"id", "courseId", "title", "purpose", "status", "version", "updatedAt", "model", "sourceLearningVersion", "entered", "completed", "phaseIndex", "topicIndex", "topicId", "phaseName", "topicTitle", "teachingTaskId", "initialTeachingTaskId"})
        if (lesson.contains(key)) result[key] = lesson[key];
    result["teachingFocus"] = Json::object();
    const auto focus = lesson.value("teachingFocus", Json::object());
    for (const auto* key : {"sectionId", "interactionSectionId", "practiceSectionId", "responseSectionId", "responseView", "responseTaskId", "taskId", "version", "updatedAt"})
        if (focus.contains(key)) result["teachingFocus"][key] = focus.at(key);
    result["references"] = Json::array();
    for (const auto& reference : lesson.value("references", Json::array())) {
        if (!reference.is_object()) continue;
        Json shown;
        for (const auto* key : {"title", "name", "description", "url", "href", "source"})
            if (reference.contains(key) && reference.at(key).is_string()) shown[key] = reference.at(key);
        result["references"].push_back(std::move(shown));
    }
    result["sections"] = Json::array();
    for (const auto& section : lesson.at("sections")) {
        Json shown;
        for (const auto* key : {"id", "kind", "title", "body", "displayed", "version", "legacyKind", "legacyIndex", "questionKind", "presentation"}) if (section.contains(key)) shown[key] = section[key];
        if (section.at("kind") == "question") {
            shown.erase("body"); Json question;
            for (const auto* key : {"question", "options", "type", "materials"}) if (section.at("question").contains(key)) question[key] = section["question"][key];
            if (question.contains("materials")) question["materials"] = publicMaterial(question.at("materials"));
            shown["question"] = question; shown["questionId"] = section.at("questionId");
        }
        result["sections"].push_back(std::move(shown));
    }
    return result;
}
Json displayedMessages(Database& db, const Json& task) {
    const auto raw = db.profileMeta("agent-exposure:" + task.at("id").get<std::string>());
    const auto exposure = Json::parse(raw, nullptr, false);
    const int observed = exposure.is_object() ? exposure.value("seq", 0) : 0;
    const auto displayed = exposure.is_object() ? exposure.value("displayedSequences", Json()) : Json();
    std::map<int, std::string> messages;
    for (const auto& event : task.value("events", Json::array()))
        if (event.value("type", "") == "delta" && event.value("field", "") == "message" && event.value("seq", 0) <= observed &&
            (!displayed.is_array() || std::find(displayed.begin(), displayed.end(), event.at("seq")) != displayed.end()))
            messages[event.value("step", 0)] += event.value("text", "");
    Json result = Json::array(); const auto validSteps = task.value("validSteps", Json::array());
    for (const auto& [step, value] : messages) if (!value.empty())
        result.push_back({{"role", "assistant"}, {"text", value}, {"step", step}, {"at", task.at("updatedAt")},
            {"partial", std::find(validSteps.begin(), validSteps.end(), step) == validSteps.end()}});
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
    if (!value.is_object()) throw std::invalid_argument(std::string("args.") + key + " 的父节点必须为对象");
    if (!value.value(key, Json()).is_string() || value[key].get<std::string>().empty())
        throw std::invalid_argument(std::string(key) + " 不能为空");
    return value[key];
}
Json link(const Json& lesson) {
    return {{"id", lesson.at("id")}, {"title", lesson.at("title")}, {"courseId", lesson.at("courseId")},
        {"href", "/learn?lessonId=" + lesson.at("id").get<std::string>()}, {"status", lesson.at("status")}};
}
}

void registerAgentLessonTools(LearningAgent& agent) {
    agent.registerTool("create_lesson", {"为可访问课程创建独立课时。参数 courseId,title,purpose,intent:advance|reinforce|review|general（默认reinforce）。推进大纲时提供 read_outline 中的 topicId；补弱及复习不增加原大纲完成数。返回 lessonId，后续添加自主选择的板块，不要求固定板块或题数。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto access = accessFor(task); const auto courseId = text(args, "courseId"); requireCourse(db, access, courseId);
            Json lesson = {{"id", newId("lesson")}, {"scopeId", access.scopeId}, {"courseId", courseId},
                {"title", text(args, "title")}, {"purpose", text(args, "purpose")}, {"status", "draft"},
                {"sections", Json::array()}, {"version", 1}, {"model", source.model}, {"updatedAt", now()},
                {"sourceLearningVersion", task.at("learningVersion")}, {"taskId", task.at("id")}, {"entered", false}};
            const auto intent = args.value("intent", "reinforce");
            if (intent != "advance" && intent != "reinforce" && intent != "review" && intent != "general") throw std::invalid_argument("课时意图无效");
            lesson["intent"] = intent;
            if (intent == "advance") {
                const auto outline = agentOutline(db, access, courseId); bool found = false;
                for (const auto& stage : outline.at("courseStructure")) for (const auto& topic : stage.at("topics")) if (topic.at("id") == text(args, "topicId")) {
                    lesson["topicId"] = topic.at("id"); lesson["phaseIndex"] = topic.at("legacyPhaseIndex"); lesson["topicIndex"] = topic.at("legacyTopicIndex");
                    lesson["phaseName"] = stage.at("stage"); lesson["topicTitle"] = topic.at("title"); found = true;
                }
                if (!found) throw std::invalid_argument("推进知识点不存在");
            }
            return PreparedAgentTool{{{"lessonId", lesson.at("id")}, {"status", "draft"}},
                [lesson, access](Database& connection) { requireCourse(connection, access, lesson.at("courseId")); saveLesson(connection, lesson); }};
        }});
    agent.registerTool("read_lesson", {"读取指定 lessonId 的原始教学板块、题目标准和展示状态供 AI 判断；这些内部资料不直接发送到用户页面。", true,
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            auto lesson = lessonFor(db, accessFor(task), text(args, "lessonId"));
            const auto exposure = Json::parse(db.profileMeta("agent-section-exposure:" + lesson.at("id").get<std::string>()), nullptr, false);
            for (auto& section : lesson["sections"])
                section["displayed"] = exposure.is_object() && exposure.value(section.at("id").get<std::string>(), 0) == section.at("version");
            return PreparedAgentTool{lesson, {}};
        }});
    agent.registerTool("select_teaching_focus", {"选择本课当前教学焦点。参数 lessonId，以及可选 sectionId（讲解段）、interactionSectionId（questionKind=interaction 的讲解互动题）、practiceSectionId（集中练习题）。省略保留当前值，空字符串清除相应焦点。先 read_lesson；只能选择已经保存的同一课时板块。依据学生真实反馈决定继续、补讲或再问，不能把切换页面当成掌握。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            const auto id = text(args, "lessonId"); auto lesson = lessonFor(db, accessFor(task), id);
            requireTeachingTarget(task, lesson);
            if (lesson.at("status") != "ready" && lesson.at("status") != "draft") throw std::invalid_argument("课时状态不允许选择教学焦点");
            if (!lesson.value("teachingTaskId", "").empty() && lesson.at("teachingTaskId") != task.at("id"))
                throw std::invalid_argument("本课已有更新的教学任务，旧输出不应用");
            auto focus = lesson.value("teachingFocus", Json::object()); bool selected = false;
            for (const auto* field : {"sectionId", "interactionSectionId", "practiceSectionId"}) {
                if (!args.contains(field)) continue;
                if (!args.at(field).is_string()) throw std::invalid_argument("教学焦点须使用稳定板块标识");
                selected = true; const auto sectionId = args.at(field).get<std::string>();
                if (!sectionId.empty()) {
                    const Json* section = nullptr;
                    for (const auto& item : lesson.at("sections")) if (item.at("id") == sectionId) { section = &item; break; }
                    if (!section) throw std::invalid_argument("教学焦点不属于该课时");
                    const bool explanation = std::string(field) == "sectionId";
                    if (section->at("kind") != (explanation ? "explanation" : "question")) throw std::invalid_argument("教学焦点板块类型不匹配");
                    if (!explanation) {
                        const auto legacyKind = section->value("legacyKind", "");
                        const auto kind = section->value("questionKind", legacyKind == "interaction" || legacyKind == "diagnostic" ? "interaction" : "practice");
                        if ((std::string(field) == "interactionSectionId") != (kind == "interaction" || kind == "diagnostic"))
                            throw std::invalid_argument("讲解互动题与集中练习题不能混用");
                        if (legacyKind == "example") throw std::invalid_argument("示范例题不作为学生评分题");
                    }
                }
                focus[field] = sectionId;
            }
            if (!selected) throw std::invalid_argument("请至少选择一个教学焦点");
            const auto preferred = responseView(lesson, task) == "practice" ? "practiceSectionId" : "interactionSectionId";
            const auto fallback = std::string(preferred) == "practiceSectionId" ? "interactionSectionId" : "practiceSectionId";
            const auto selectedQuestion = args.value(preferred, args.value(fallback, ""));
            for (const auto& section : lesson.at("sections")) if (section.at("id") == selectedQuestion)
                selectResponseQuestion(focus, lesson, task, section);
            focus["taskId"] = task.at("id"); focus["version"] = focus.value("version", 0) + 1; focus["updatedAt"] = now();
            lesson["teachingFocus"] = focus; lesson["version"] = lesson.at("version").get<int>() + 1; lesson["updatedAt"] = now();
            return update(db, task, id, lesson, {{"lessonId", id}, {"teachingFocus", focus}});
        }});
    agent.registerTool("append_section", {"追加教学板块，不改已展示内容。参数 lessonId、section:{kind:explanation|question|summary,title,body 或 question:{question,options 可选,type,expectedAnswer,rubric},questionKind:interaction|practice}，activate 可选。课堂互动追加的问题默认立即在学生当前页面显示；仅备好未来题目时用 activate=false。选择题必须有至少两个非空 options，不可只在 message 里承诺出题。题目标准只放 expectedAnswer/rubric，示范放 explanation，不评分。返回稳定板块与题目标识。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto id = text(args, "lessonId"); auto lesson = lessonFor(db, accessFor(task), id);
            requireTeachingTarget(task, lesson);
            if (lesson.at("status") != "draft" && lesson.at("status") != "ready") throw std::invalid_argument("课时状态不允许补充");
            const auto input = args.at("section"); const auto kind = text(input, "kind");
            if (kind != "explanation" && kind != "question" && kind != "summary") throw std::invalid_argument("教学板块类型无效");
            Json section = {{"id", newId("section")}, {"kind", kind}, {"title", text(input, "title")},
                {"version", 1}, {"displayed", false}, {"model", source.model}, {"createdAt", now()}};
            if (kind == "question") {
                const auto question = input.at("question");
                if (!question.is_object()) throw std::invalid_argument("section.question 必须为 JSON 对象，不能使用数组；一个题目对应一个 question 板块");
                text(question, "question");
                if (input.contains("questionKind") && args.contains("questionKind") && input.at("questionKind") != args.at("questionKind"))
                    throw std::invalid_argument("section.questionKind 与外层 questionKind 不一致，请明确题目用途");
                // 兼容模型将同名用途参数放在外层，避免默认为另一类题而导致后续焦点失败。
                section["questionKind"] = input.value("questionKind", args.value("questionKind", question.value("kind", "practice")));
                if (section.at("questionKind") != "interaction" && section.at("questionKind") != "practice" &&
                    section.at("questionKind") != "diagnostic" && section.at("questionKind") != "quiz" && section.at("questionKind") != "review")
                    throw std::invalid_argument("题目用途无效；示范例题请使用 explanation");
                if (question.contains("options")) {
                    if (!question["options"].is_array()) throw std::invalid_argument("题目选项无效");
                    for (const auto& option : question["options"]) if (!option.is_string() || option.get<std::string>().empty())
                        throw std::invalid_argument("题目选项须为非空文字");
                }
                const auto questionType = question.value("type", question.contains("options") ? "choice" : "open");
                if ((questionType == "choice" || questionType == "single_choice" || questionType == "multi_choice") &&
                    (!question.contains("options") || question.at("options").size() < 2))
                    throw std::invalid_argument("section.question.options：选择题必须保存至少两个选项");
                section["question"] = question;
                section["question"]["type"] = questionType;
                Json identity = {{"question", question.at("question")}};
                if (question.contains("options")) identity["options"] = question["options"];
                section["questionId"] = questionIdentity(lesson.at("courseId"), identity);
            } else {
                section["body"] = text(input, "body");
                if (input.contains("presentation")) {
                    if (input.at("presentation") != "example") throw std::invalid_argument("示范展示类型无效");
                    section["presentation"] = "example";
                }
            }
            lesson["sections"].push_back(section); lesson["version"] = lesson.at("version").get<int>() + 1;
            if (kind == "question" && args.value("activate", true)) {
                auto focus = lesson.value("teachingFocus", Json::object());
                selectResponseQuestion(focus, lesson, task, section);
                if (focus.value("responseTaskId", "") == task.at("id")) {
                    focus[section.at("questionKind") == "interaction" || section.at("questionKind") == "diagnostic"
                        ? "interactionSectionId" : "practiceSectionId"] = section.at("id");
                    focus["taskId"] = task.at("id"); focus["version"] = focus.value("version", 0) + 1;
                    focus["updatedAt"] = now(); lesson["teachingFocus"] = focus;
                }
            }
            lesson["model"] = source.model; lesson["updatedAt"] = now();
            return update(db, task, id, lesson, {{"sectionId", section.at("id")}, {"questionId", section.value("questionId", "")},
                {"questionKind", section.value("questionKind", "")}, {"version", lesson["version"]}});
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
            if (answer.empty()) throw std::invalid_argument("学生输入标识不存在：interactionId 必须使用 currentInputs 中的学生记录 ID，不能使用板块或题目 ID");
            requireCourse(db, access, courseId);
            if (answer.empty() || !answer.value("questionId", Json()).is_string() || !answer.value("questionSnapshot", Json()).is_object())
                throw std::invalid_argument("缺少可靠原题与实际作答，不能建立正式评价");
            if (answer.value("scopeId", "") != access.scopeId) throw std::invalid_argument("无权评价该回答");
            if (answer.value("action", "answer") == "skip" || answer.value("action", "answer") == "stop" ||
                answer.value("action", "answer") == "omitted" || answer.value("action", "answer") == "hint")
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
            bool currentInput = answer.value("requestId", "") == task.at("event").value("requestId", "");
            for (const auto& input : task.at("event").value("batchAnswers", Json::array())) currentInput = currentInput || input.at("requestId") == answer.at("requestId");
            return PreparedAgentTool{{{"evaluationId", id}, {"saved", true}, {"inputResolved", currentInput},
                {"resolvedRequestId", currentInput ? answer.at("requestId") : Json("")}, {"evaluation", value}}, [value, id, courseId](Database& connection) {
                if (!connection.upsert(ClassroomActivity{id, courseId, "agent-evaluation", value.dump(), now(), 0, 0}))
                    throw std::runtime_error("真实评价暂存失败");
            }};
        }});
    agent.registerTool("classify_input", {"判断本次题目输入是否只是追问。参数 interactionId、isAnswer:false、reason。真正作答或明确不会请用 evaluate_answer，不为纯追问增加评分。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            if (!args.value("isAnswer", Json()).is_boolean() || args.at("isAnswer").get<bool>())
                throw std::invalid_argument("实际作答必须先评价");
            text(args, "reason"); const auto id = text(args, "interactionId");
            for (const auto& row : db.listInteractions()) if (row.id == id && row.kind == "chat-user") {
                const auto input = Json::parse(row.payload);
                bool currentInput = input.value("requestId", "") == task.at("event").value("requestId", "");
                for (const auto& answer : task.at("event").value("batchAnswers", Json::array())) currentInput = currentInput || answer.at("requestId") == input.at("requestId");
                if (input.value("scopeId", "") != task.at("scopeId") || !currentInput)
                    throw std::invalid_argument("只能判断本次真实输入");
                return PreparedAgentTool{{{"inputResolved", true}, {"resolvedRequestId", input.at("requestId")}, {"isAnswer", false}, {"reason", args.at("reason")}}, {}};
            }
            throw std::invalid_argument("学生输入不存在");
        }});
}

void finalizeAgentEvaluations(Database& db, const Json& task) {
    for (const auto& entry : task.at("completedActions").items()) {
        const auto result = entry.value().value("result", Json::object());
        if (!result.contains("evaluationId")) continue;
        const auto id = result.at("evaluationId").get<std::string>();
        const auto row = db.getClassroomActivity(id);
        if (!row || row->kind != "agent-evaluation") throw std::runtime_error("暂存评价不存在");
        auto value = Json::parse(row->payload);
        if (value.value("published", false)) continue;
        if (!db.insert(LearningInteraction{id, "practice", value.dump(), now(), row->courseId,
            value.at("interactionId").get<std::string>(), {}})) throw std::runtime_error("完整教学评价保存失败");
        value["published"] = true;
        if (!db.upsert(ClassroomActivity{id, row->courseId, row->kind, value.dump(), now(), 0, 0}))
            throw std::runtime_error("评价提交标记保存失败");
    }
}

void agentRecordLearnerEvent(Database& db, const AgentAccess& access, const Json& event) {
    if (event.contains("batchAnswers")) {
        if (!event.at("batchAnswers").is_array() || event.at("batchAnswers").empty()) throw std::invalid_argument("批量作答列表无效");
        for (const auto& answer : event.at("batchAnswers")) agentRecordLearnerEvent(db, access, answer);
        return;
    }
    const auto type = event.value("type", "");
    if (type != "answer" && type != "chat" && type != "feedback" && type != "question_answer") return;
    Json value = {{"text", text(event, "text")}, {"scopeId", access.scopeId}, {"action", event.value("action", "answer")},
        {"requestId", event.at("requestId")}, {"type", type}, {"conversationId", event.value("conversationId", "general")}};
    for (const auto* field : {"reviewId", "day"}) if (event.contains(field)) value[field] = event.at(field);
    const auto courseId = event.value("courseId", "");
    if (!courseId.empty()) requireCourse(db, access, courseId);
    if (event.contains("lessonId")) {
        const auto lesson = lessonFor(db, access, text(event, "lessonId"));
        if (lesson.at("status") != "ready" || lesson.at("courseId") != courseId) throw std::invalid_argument("课程尚未准备好或所属课程不匹配");
        value["lessonId"] = lesson.at("id");
        if (type != "question_answer" && event.contains("sectionId")) {
            const auto sectionId = text(event, "sectionId"); bool found = false;
            for (const auto& section : lesson.at("sections")) if (section.at("id") == sectionId) found = true;
            if (!found) throw std::invalid_argument("交流板块不属于该课时");
            value["sectionId"] = sectionId;
        }
        if (type == "question_answer") {
        const auto sectionId = text(event, "sectionId"); const Json* section = nullptr;
        for (const auto& item : lesson.at("sections")) if (item.at("id") == sectionId) { section = &item; break; }
        if (!section || section->at("kind") != "question" || !event.value("sectionVersion", Json()).is_number_integer() ||
            event.at("sectionVersion") != section->at("version")) throw std::invalid_argument("题目版本已变化，请保留输入并刷新");
        if (section->value("legacyKind", "") == "example") throw std::invalid_argument("示范例题只用于讲解，请以追问方式交流");
        value["questionId"] = section->at("questionId"); value["questionSnapshot"] = section->at("question");
        value["lessonId"] = lesson.at("id"); value["sectionId"] = sectionId;
        Json hints = Json::array();
        const auto exposure = Json::parse(db.profileMeta("agent-section-exposure:" + lesson.at("id").get<std::string>()), nullptr, false);
        if (exposure.is_object()) for (const auto& item : lesson.at("sections"))
            if (item.at("kind") != "question" && exposure.value(item.at("id").get<std::string>(), 0) == item.at("version") && item.value("body", "") != "")
                hints.push_back(item.at("body"));
        for (const auto& item : section->value("legacyDialog", Json::array())) if (item.value("role", "") == "assistant") hints.push_back(item.at("text"));
        for (const auto& row : db.listClassroomActivities(courseId)) if (row.kind == "agent-task") {
            const auto task = Json::parse(row.payload, nullptr, false);
            if (task.is_object() && task.value("scopeId", "") == access.scopeId &&
                task.value("event", Json::object()).value("sectionId", "") == sectionId &&
                task.value("event", Json::object()).value("lessonId", "") == lesson.at("id"))
                for (const auto& item : displayedMessages(db, task)) hints.push_back(item.at("text"));
        }
        value["hintHistory"] = hints; value["assisted"] = !hints.empty();
        }
    }
    if (!db.insert(LearningInteraction{newId("student"), "chat-user", value.dump(), now(),
        courseId.empty() ? std::optional<std::string>{} : std::optional<std::string>{courseId}, event.value("conversationId", event.at("requestId").get<std::string>()), {}}))
        throw std::runtime_error("学生回答保存失败");
}

Json agentLessonView(Database& db, const AgentAccess& access, const std::string& lessonId) {
    agentUpgradeLegacyLesson(db, access, lessonId);
    const auto saved = lessonFor(db, access, lessonId); auto shown = publicLesson(saved);
    const auto sectionExposure = Json::parse(db.profileMeta("agent-section-exposure:" + lessonId), nullptr, false);
    for (auto& section : shown["sections"])
        section["displayed"] = sectionExposure.is_object() && sectionExposure.value(section.at("id").get<std::string>(), 0) == section.at("version");
    const auto interactions = db.listInteractions();
    const auto tasks = db.listClassroomActivities(saved.at("courseId"));
    // 历史版本已生成但未激活的题，从最后教学任务的成功动作恢复展示焦点；不改正文或评价记录。
    if (!shown["teachingFocus"].contains("responseSectionId")) for (const auto& row : tasks) {
        if (row.kind != "agent-task" || row.id != saved.value("teachingTaskId", "")) continue;
        const auto task = Json::parse(row.payload, nullptr, false);
        if (!task.is_object() || task.value("scopeId", "") != access.scopeId) continue;
        const auto type = task.value("event", Json::object()).value("type", "");
        if (type != "question_answer" && type != "chat") continue;
        const auto completed = task.value("completedActions", Json::object());
        if (!completed.is_object()) continue;
        for (const auto& event : task.value("events", Json::array())) {
            if (!event.is_object() || event.value("type", "") != "action") continue;
            const auto id = event.value("actionId", ""); if (!completed.contains(id)) continue;
            const auto action = Json::parse(completed[id].value("encoded", ""), nullptr, false);
            if (!action.is_object() || !action.value("args", Json()).is_object()) continue;
            const auto& args = action.at("args");
            if (args.value("lessonId", "") != lessonId) continue;
            std::string sectionId;
            if (action.value("tool", "") == "append_section" && args.value("activate", true))
                sectionId = completed[id].value("result", Json::object()).value("sectionId", "");
            else if (action.value("tool", "") == "select_teaching_focus") {
                const auto preferred = responseView(saved, task) == "practice" ? "practiceSectionId" : "interactionSectionId";
                sectionId = args.value(preferred, args.value(std::string(preferred) == "practiceSectionId" ? "interactionSectionId" : "practiceSectionId", ""));
            }
            for (const auto& section : saved.at("sections")) if (section.at("id") == sectionId)
                selectResponseQuestion(shown["teachingFocus"], saved, task, section);
        }
    }
    shown["dialog"] = Json::array();
    const auto appendReplies = [&](Json& dialog, const Json& input, const Json& task) {
        if (task.value("scopeId", "") != access.scopeId || task.value("requestId", "") != input.value("requestId", "")) return;
        for (const auto& item : displayedMessages(db, task)) dialog.push_back(item);
    };
    for (const auto& interaction : interactions) if (interaction.kind == "chat-user") {
        const auto input = Json::parse(interaction.payload, nullptr, false);
        if (!input.is_object() || input.value("scopeId", "") != access.scopeId || input.value("lessonId", "") != lessonId || input.contains("sectionId")) continue;
        shown["dialog"].push_back({{"role", "user"}, {"text", input.value("text", "")}, {"at", interaction.createdAt}});
        for (const auto& row : tasks) if (row.kind == "agent-task") {
            const auto task = Json::parse(row.payload, nullptr, false);
            if (!task.is_object() || task.value("requestId", "") != input.value("requestId", "")) continue;
            appendReplies(shown["dialog"], input, task); shown["taskId"] = task.at("id"); shown["taskStatus"] = task.at("status");
            const auto exposure = Json::parse(db.profileMeta("agent-exposure:" + task.at("id").get<std::string>()), nullptr, false);
            shown["afterSeq"] = exposure.is_object() ? exposure.value("seq", 0) : 0;
        }
    }
    for (auto& section : shown["sections"]) {
        section["dialog"] = Json::array();
        for (const auto& source : saved.at("sections")) if (source.at("id") == section.at("id"))
            for (const auto& item : source.value("legacyDialog", Json::array())) section["dialog"].push_back(item);
        for (const auto& interaction : interactions) if (interaction.kind == "chat-user") {
            const auto input = Json::parse(interaction.payload, nullptr, false);
            if (!input.is_object() || input.value("scopeId", "") != access.scopeId || input.value("lessonId", "") != lessonId ||
                input.value("sectionId", "") != section.at("id")) continue;
            section["dialog"].push_back({{"role", "user"}, {"text", input.value("text", "")}, {"at", interaction.createdAt}});
            for (const auto& row : tasks) if (row.kind == "agent-task") {
                const auto task = Json::parse(row.payload, nullptr, false);
                if (!task.is_object() || task.value("scopeId", "") != access.scopeId || task.value("requestId", "") != input.value("requestId", "")) continue;
                section["taskId"] = task.at("id"); section["taskStatus"] = task.at("status");
                appendReplies(section["dialog"], input, task);
                const auto exposure = Json::parse(db.profileMeta("agent-exposure:" + task.at("id").get<std::string>()), nullptr, false);
                section["afterSeq"] = exposure.is_object() ? exposure.value("seq", 0) : 0;
            }
        }
    }
    shown["teachingDialog"] = Json::array(); shown["evaluations"] = Json::array(); shown["historicalAssessments"] = Json::array();
    for (const auto& row : tasks) if (row.kind == "agent-task") {
        const auto task = Json::parse(row.payload, nullptr, false);
        if (!task.is_object() || task.value("scopeId", "") != access.scopeId ||
            task.value("event", Json::object()).value("lessonId", "") != lessonId) continue;
        const auto event = task.at("event");
        for (const auto& interaction : interactions) if (interaction.kind == "chat-user") {
            const auto input = Json::parse(interaction.payload, nullptr, false);
            if (input.is_object() && input.value("scopeId", "") == access.scopeId &&
                input.value("lessonId", "") == lessonId && input.value("requestId", "") == task.at("requestId"))
                shown["teachingDialog"].push_back({{"role", "user"}, {"text", input.value("text", "")},
                    {"sectionId", input.value("sectionId", "")}, {"taskId", task.at("id")}, {"at", interaction.createdAt}});
        }
        for (auto item : displayedMessages(db, task)) {
            item["taskId"] = task.at("id"); item["sectionId"] = event.value("sectionId", "");
            shown["teachingDialog"].push_back(std::move(item));
        }
    }
    for (const auto& row : interactions) if (row.kind == "practice" && row.courseId == saved.at("courseId").get<std::string>()) {
        const auto value = Json::parse(row.payload, nullptr, false);
        if (!value.is_object() || !value.contains("interactionId") || value.value("source", "") != "ai") continue;
        for (const auto& inputRow : interactions) if (inputRow.id == value.at("interactionId") && inputRow.kind == "chat-user") {
            const auto input = Json::parse(inputRow.payload, nullptr, false);
            if (!input.is_object() || input.value("scopeId", "") != access.scopeId || input.value("lessonId", "") != lessonId) continue;
            Json evaluation = {{"id", row.id}, {"sectionId", input.value("sectionId", "")}};
            for (const auto* field : {"questionId", "response", "score", "credible", "feedback", "reason", "uncertainty", "assisted", "at"})
                if (value.contains(field)) evaluation[field] = value.at(field);
            shown["evaluations"].push_back(std::move(evaluation));
        }
    }
    if (saved.contains("phaseIndex") && saved.contains("topicIndex")) {
        const auto preparedTask = lessonId.find("legacy-prepared-") == 0 ? lessonId.substr(16) : std::string();
        for (const auto& row : interactions) if (row.kind == "quiz" && row.courseId == saved.at("courseId").get<std::string>()) {
            const auto value = Json::parse(row.payload, nullptr, false);
            if (!value.is_object() || value.value("phaseIndex", 0) != saved.at("phaseIndex") || value.value("topicIndex", 0) != saved.at("topicIndex") ||
                value.value("lessonTaskId", "") != preparedTask) continue;
            Json record = {{"id", row.id}, {"kind", "quiz"}, {"at", row.createdAt}, {"answers", Json::array()}};
            for (const auto* field : {"score", "total"}) if (value.contains(field) && (value.at(field).is_number() || value.at(field).is_null())) record[field] = value.at(field);
            for (const auto& answer : value.value("answers", Json::array()))
                record["answers"].push_back(answer.is_string() || answer.is_number_integer() || answer.is_null() ? answer : Json());
            shown["historicalAssessments"].push_back(std::move(record));
        }
        for (const auto& row : tasks) if (row.kind == "practice" || row.kind == "diagnostic" || row.kind == "interaction" || row.kind == "quiz") {
            const auto value = Json::parse(row.payload, nullptr, false);
            if (!value.is_object() || row.phaseIndex != saved.at("phaseIndex") || row.topicIndex != saved.at("topicIndex") ||
                value.value("lessonTaskId", "") != preparedTask || value.value("status", "") != "answered" || !value.contains("answer")) continue;
            const auto identity = value.value("questionId", value.value("questionSnapshot", Json()).is_object() ? questionIdentity(saved.at("courseId"), value.at("questionSnapshot")) : "");
            bool matched = false;
            for (const auto& section : saved.at("sections"))
                if (section.value("questionId", "") == identity && !identity.empty()) matched = true;
            if (!matched) continue;
            Json record = {{"id", row.id}, {"kind", row.kind}, {"at", row.updatedAt}, {"questionId", identity},
                {"response", value.at("answer").is_string() ? value.at("answer") : Json(value.at("answer").dump())}};
            for (const auto* field : {"feedback", "credible", "correct", "assisted", "score"})
                if (value.contains(field) && (value.at(field).is_string() || value.at(field).is_boolean() || value.at(field).is_number() || value.at(field).is_null())) record[field] = value.at(field);
            shown["historicalAssessments"].push_back(std::move(record));
        }
    }
    if (shown.contains("teachingTaskId")) {
        const auto task = agentView(db, access, shown.at("teachingTaskId"));
        shown["teachingTaskStatus"] = task.at("status");
        const auto exposure = Json::parse(db.profileMeta("agent-exposure:" + task.at("id").get<std::string>()), nullptr, false);
        shown["teachingAfterSeq"] = exposure.is_object() ? exposure.value("seq", 0) : 0;
    }
    return shown;
}
Json agentEnterLesson(Database& db, const AgentAccess& access, const Json& body) {
    Json result;
    db.transaction([&] {
        auto lesson = lessonFor(db, access, text(body, "lessonId"));
        if (lesson.at("status") != "ready") throw std::invalid_argument("请等待真实 AI 完整备课后进入");
        rememberCurrentCourse(db, lesson.at("courseId"));
        if (lesson.value("entered", false)) { result = link(lesson); return; }
        if (!lesson.value("entered", false) && lesson.at("sourceLearningVersion") != db.learningRevision())
            throw std::invalid_argument("学习情况已有更新，请让 AI 重新准备后再进入");
        lesson["entered"] = true; lesson["version"] = lesson.at("version").get<int>() + 1;
        if (lesson.value("intent", "") == "advance") {
            db.setProfileMeta("agent-legacy:" + lesson.at("courseId").get<std::string>() + ":" + std::to_string(lesson.at("phaseIndex").get<int>()) + ":" + std::to_string(lesson.at("topicIndex").get<int>()), lesson.at("id"));
        }
        lesson["updatedAt"] = now(); saveLesson(db, lesson); result = link(lesson);
    });
    return result;
}
void agentAttachLessonTask(Database& db, const AgentAccess& access, const Json& event, const std::string& taskId) {
    if (!event.contains("lessonId")) return;
    auto lesson = lessonFor(db, access, text(event, "lessonId"));
    if (lesson.at("status") != "ready" || lesson.at("courseId") != event.value("courseId", ""))
        throw std::invalid_argument("课程尚未准备好或所属课程不匹配");
    if (event.value("type", "") == "lesson_start") {
        lesson["initialTeachingTaskId"] = taskId;
        if (!lesson.contains("teachingTaskId")) lesson["teachingTaskId"] = taskId;
    } else lesson["teachingTaskId"] = taskId;
    lesson["updatedAt"] = now(); saveLesson(db, lesson);
}
Json agentStartLesson(Database& db, const AgentAccess& access, const Json& body) {
    const auto id = text(body, "lessonId"); agentEnterLesson(db, access, body);
    const auto lesson = lessonFor(db, access, id);
    if (!lesson.value("initialTeachingTaskId", "").empty()) return agentView(db, access, lesson.at("initialTeachingTaskId"));
    return agentSubmit(db, access, {{"type", "lesson_start"}, {"requestId", "lesson-start:" + id},
        {"courseId", lesson.at("courseId")}, {"lessonId", id},
        {"text", "请读取本课已经保存的板块和真实学习记录，选择当前讲解段及合适的互动题，分段带我学习。围绕当前段边问边讲；等待我的反馈再决定继续、补讲或举例。已有课件无需重新生成，切换讲解、练习和小结不表示完成。"}});
}
Json agentExposeTask(Database& db, const AgentAccess& access, const Json& body) {
    if (body.contains("lessonId") && body.contains("sections")) {
        const auto id = text(body, "lessonId");
        if (!body.at("sections").is_array()) throw std::invalid_argument("展示板块列表无效");
        Json result;
        db.transaction([&] {
            const auto lesson = lessonFor(db, access, id);
            if (lesson.at("status") != "ready" || !lesson.value("entered", false)) throw std::invalid_argument("课时尚未进入");
            const auto key = "agent-section-exposure:" + id;
            const auto previous = Json::parse(db.profileMeta(key), nullptr, false);
            auto exposure = previous.is_object() ? previous : Json::object();
            for (const auto& entry : body.at("sections")) {
                const auto sectionId = text(entry, "id"); bool found = false;
                for (const auto& section : lesson.at("sections"))
                    if (section.at("id") == sectionId && entry.value("version", Json()).is_number_integer() && entry.at("version") == section.at("version")) found = true;
                if (!found) throw std::invalid_argument("展示板块身份或版本已变化");
                exposure[sectionId] = entry.at("version");
            }
            db.setProfileMeta(key, exposure.dump()); result = {{"lessonId", id}, {"seenSections", exposure.size()}};
        });
        return result;
    }
    const auto taskId = text(body, "taskId");
    if (!body.value("seq", Json()).is_number_integer()) throw std::invalid_argument("展示序号无效");
    Json result;
    db.transaction([&] {
        const auto taskRow = db.getClassroomActivity(taskId);
        if (!taskRow || taskRow->kind != "agent-task") throw std::invalid_argument("任务不存在");
        const auto task = Json::parse(taskRow->payload);
        if (task.at("scopeId") != access.scopeId) throw std::invalid_argument("无权记录该任务的展示状态");
        for (const auto& course : task.at("courseIds")) requireCourse(db, access, course);
        const int sequence = body.at("seq");
        if (sequence < 0 || sequence > task.value("seq", 0)) throw std::invalid_argument("展示序号尚未保存");
        const auto key = "agent-exposure:" + taskId;
        const auto previous = Json::parse(db.profileMeta(key), nullptr, false);
        const int old = previous.is_object() ? previous.value("seq", 0) : 0;
        result = {{"seq", std::max(sequence, old)}, {"taskId", taskId}};
        if (body.contains("displayedSequences") || (previous.is_object() && previous.contains("displayedSequences"))) {
            Json displayed = previous.is_object() ? previous.value("displayedSequences", Json::array()) : Json::array();
            if (!displayed.is_array()) throw std::invalid_argument("已保存的展示序号格式无效");
            // 历史客户端只有游标，保留它此前已经记录的展示；新客户端逐条记录可见文字。
            if (previous.is_object() && !previous.contains("displayedSequences"))
                for (const auto& item : task.value("events", Json::array()))
                    if (item.value("type", "") == "delta" && item.value("field", "") == "message" && item.value("seq", 0) <= old)
                        displayed.push_back(item.at("seq"));
            const auto incoming = body.value("displayedSequences", Json::array());
            if (!incoming.is_array()) throw std::invalid_argument("展示序号列表无效");
            for (const auto& value : incoming) {
                if (!value.is_number_integer() || value.get<int>() <= 0 || value.get<int>() > sequence)
                    throw std::invalid_argument("展示序号尚未保存");
                bool visibleMessage = false;
                for (const auto& item : task.value("events", Json::array()))
                    if (item.at("seq") == value && item.value("type", "") == "delta" && item.value("field", "") == "message") visibleMessage = true;
                if (!visibleMessage) throw std::invalid_argument("只能记录实际展示的教学文字");
                if (std::find(displayed.begin(), displayed.end(), value) == displayed.end()) displayed.push_back(value);
            }
            result["displayedSequences"] = displayed;
        }
        db.setProfileMeta(key, result.dump());
    });
    return result;
}
}

#include "learning_agent.hpp"
#include "agent_lessons.hpp"
#include "agent_profiles.hpp"
#include "question_evidence.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

using Json = nlohmann::json;
// 开发验收入口直接调用现有 C++ 主控；不进入安装包。评分标准只来自隔离用例。
int main(int argc, char** argv) {
    try {
        if (argc != 4) throw std::invalid_argument("参数：用例JSON 隔离数据库 输出JSON");
        std::ifstream input(std::filesystem::u8path(argv[1])); Json test; input >> test;
        gangyi::Database db; db.open(argv[2]); db.migrate();
        if (test.contains("preferences"))
            db.saveMyAIRecord({"preferences", "preferences", test.at("preferences").dump(), "2026-10-13", 0}, 0);
        const gangyi::AgentAccess access{"local-profile", {"eval-course"}};
        const std::string lessonId = "eval-lesson", courseId = "eval-course";
        db.insert(gangyi::Course{courseId, access.scopeId, {}, test.at("title"), "deep", test.at("title"),
            {}, "ai", "active", "2026-10-13", "2026-10-13"});
        Json sections = Json::array();
        sections.push_back({{"id", "concept"}, {"kind", "explanation"}, {"title", "学过的相关概念"},
            {"body", test.at("concept")}, {"version", 1}, {"helpType", "explanation"}});
        int index = 0;
        for (const auto& question : test.at("questions")) {
            if (index > 0 && !test.value("seedSecond", true)) break;
            const auto id = "question-" + std::to_string(index++);
            Json identity = {{"question", question.at("question")}};
            if (question.contains("options")) identity["options"] = question.at("options");
            sections.push_back({{"id", id}, {"kind", "question"}, {"title", "实际作答"}, {"questionKind", "practice"},
                {"version", 1}, {"question", question}, {"questionId", gangyi::questionIdentity(courseId, identity)}});
        }
        Json lesson = {{"id", lessonId}, {"scopeId", access.scopeId}, {"courseId", courseId}, {"title", test.at("title")},
            {"purpose", "依据真实回答诊断并安排独立复练"}, {"status", "ready"}, {"entered", true},
            {"version", 1}, {"model", "人工验收材料"}, {"updatedAt", "2026-10-13"}, {"sections", sections},
            {"sourceLearningVersion", db.learningRevision()}, {"teachingFocus", {{"practiceSectionId", "question-0"}}}};
        db.upsert(gangyi::ClassroomActivity{lessonId, courseId, "agent-lesson", lesson.dump(), "2026-10-13", 0, 0});
        gangyi::agentExposeTask(db, access, {{"lessonId", lessonId}, {"sections", {{{"id", "concept"}, {"version", 1}}}}});
        Json usage = Json::array();
        gangyi::LearningAgent agent(argv[2], [&](const gangyi::ChatOptions& options, const std::function<bool(const std::string&)>& sink) {
            if (test.value("fixtureMode", false)) {
                const auto context = Json::parse(options.messages[1].content);
                const auto event = context.at("event"); const auto actual = context.at("currentInputs").at(0);
                Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", "请继续独立作答。"},
                    {"actions", Json::array()}, {"state", "waiting_student"}};
                if (options.activity.purpose == "判断本次回答或追问") {
                    Json action;
                    if (event.at("text").get<std::string>().find("为什么") != std::string::npos)
                        action = {{"id", "classify"}, {"tool", "classify_input"}, {"args", {{"interactionId", actual.at("interactionId")},
                            {"isAnswer", false}, {"reason", "这是方法原因的追问"}}}};
                    else {
                        const bool incorrect = event.at("text").get<std::string>().find("2/5") != std::string::npos;
                        action = {{"id", "grade"}, {"tool", "evaluate_answer"}, {"args", {{"interactionId", actual.at("interactionId")},
                            {"score", incorrect ? 20 : 95}, {"credible", true}, {"feedback", incorrect ? "分母不同，先统一每份大小，再相加。" : "结果与通分过程正确，再用自己的话说明理由。"},
                            {"reason", "依据本题原题和实际回答"}, {"uncertainty", "固定页面演示记录，不代表真实教学验收。"},
                            {"diagnosis", {{"summary", incorrect ? "分子分母分别相加，忽略每份大小不同。" : "本次计算正确，需要结合帮助程度判断独立理解。"},
                                {"evidence", {event.at("text")}}, {"teachingDirection", "用等分单位解释通分的理由。"}, {"needsClarification", false}}}}}};
                    }
                    decision["actions"].push_back(action); decision["state"] = "continue"; decision["message"] = "";
                } else if (event.value("requestType", "") == "retry_question") {
                    decision["message"] = "下面是关联原问题的新题。请独立写出结果，并解释为什么先通分。";
                    decision["actions"].push_back({{"id", "retry-question"}, {"tool", "append_section"}, {"args", {{"lessonId", lessonId},
                        {"section", {{"kind", "question"}, {"title", "关联原题 · 独立复练"}, {"questionKind", "practice"},
                            {"rePracticeOf", actual.at("questionId")}, {"question", test.at("questions").at(1)}}}}}});
                } else if (event.value("requestType", "") == "worked_example") {
                    decision["teaching"]["helpType"] = "worked_example";
                    decision["message"] = "示范：先统一每份大小。1/2=3/6，1/3=2/6，相加得到5/6。接着换一道新题，检验能否独立完成。";
                } else {
                    decision["teaching"]["helpType"] = event.value("requestType", "") == "hint" ? "hint" : "explanation";
                    decision["message"] = "先想想：两种分数的每份大小相同吗？尝试把它们换成同样大小的份。";
                }
                const auto content = decision.dump();
                for (std::size_t offset=0;offset<content.size();offset+=13) sink(content.substr(offset,13));
                gangyi::AIResult result; result.content = content; result.model = "页面固定演示"; result.finishReason = "stop";
                return result;
            }
            const auto start = std::chrono::steady_clock::now();
            gangyi::AIClient client; gangyi::AIResult response;
            try { response = client.chatStream(options, sink); }
            catch (...) { usage.push_back({{"model", options.model}, {"tokenUsage", nullptr}, {"failed", true},
                {"durationMs", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count()}}); throw; }
            usage.push_back({{"model", response.model}, {"tokenUsage", response.tokenUsage},
                {"durationMs", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count()}});
            return response;
        });
        gangyi::registerAgentLessonTools(agent);
        Json turns = Json::array(); index = 0;
        for (auto event : test.at("turns")) {
            auto saved = Json::parse(db.getClassroomActivity(lessonId)->payload);
            if (event.value("sectionId", "") == "$latest") {
                for (auto it = saved["sections"].rbegin(); it != saved["sections"].rend(); ++it)
                    if (it->value("kind", "") == "question") { event["sectionId"] = it->at("id"); break; }
            }
            event["courseId"] = courseId; event["lessonId"] = lessonId; event["view"] = "practice";
            event["requestId"] = "eval-turn-" + std::to_string(index++);
            if (event.contains("sectionId")) event["sectionVersion"] = 1;
            const auto task = gangyi::agentSubmit(db, access, event); agent.process(db, task.at("id"));
            const auto row = db.getClassroomActivity(task.at("id")); const auto raw = Json::parse(row->payload);
            gangyi::agentExposeTask(db, access, {{"taskId", task.at("id")}, {"seq", raw.at("seq")}});
            Json exposed = Json::array(); saved = Json::parse(db.getClassroomActivity(lessonId)->payload);
            for (const auto& section : saved.at("sections")) exposed.push_back({{"id", section.at("id")}, {"version", section.at("version")}});
            gangyi::agentExposeTask(db, access, {{"lessonId", lessonId}, {"sections", exposed}});
            turns.push_back({{"event", event}, {"task", raw}, {"currentInputs", gangyi::agentContext(db, access, event).at("currentInputs")}});
            if (raw.at("status") == "failed" || raw.at("status") == "superseded") break;
        }
        Json interactions = Json::array();
        for (const auto& row : db.listInteractions()) interactions.push_back({{"id", row.id}, {"kind", row.kind}, {"payload", Json::parse(row.payload)}});
        Json result = {{"caseId", test.at("id")}, {"turns", turns}, {"interactions", interactions}, {"usage", usage},
            {"lesson", Json::parse(db.getClassroomActivity(lessonId)->payload)}, {"profileEvidence", gangyi::agentProfileEvidence(db, access)}};
        std::ofstream output(std::filesystem::u8path(argv[3])); output << result.dump(2);
        std::cout << test.at("id") << "：完成 " << turns.size() << " 轮，实际调用 " << usage.size() << " 次\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << "验收主控错误：" << error.what() << '\n'; return 1; }
}

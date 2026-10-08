#ifdef NDEBUG
#undef NDEBUG
#endif
#include "learning_agent.hpp"
#include "agent_preferences.hpp"
#include "agent_lessons.hpp"
#include "learning_flow.hpp"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

using Json = nlohmann::json;
gangyi::AIResult output(const Json& value, const std::function<bool(const std::string&)>& sink) {
    const auto text = value.dump();
    for (size_t i = 0; i < text.size(); i += 7) if (!sink(text.substr(i, 7)))
        throw gangyi::AIClientError("cancelled", "测试主动中断");
    gangyi::AIResult result; result.content = text; result.model = "fixture-real-protocol"; result.finishReason = "stop";
    return result;
}
int main() {
    const auto root = std::filesystem::temp_directory_path() /
        ("gangyi-agent-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    const auto path = (root / "fixture.db").string();
    {
        gangyi::Database db; db.open(path); db.migrate();
        assert(db.insert(gangyi::Course{"chem", "fixture", {}, "氧化还原", "deep", "化学",
            {}, "ai", "active", "2026-10-04", "2026-10-04"}));
        const gangyi::AgentAccess access{"fixture", {"chem"}};
        const auto initialFingerprint = gangyi::agentLearnerFingerprint(db, access);
        assert(db.insert(gangyi::LearningInteraction{"ai-only", "chat-assistant", "{\"text\":\"教师自己写出的答案\"}",
            "2026-10-04", "chem", "fixture", {}}));
        assert(gangyi::agentLearnerFingerprint(db, access) == initialFingerprint);
        assert(db.insert(gangyi::LearningInteraction{"student", "chat-user", "{\"text\":\"我认为化合价发生变化\"}",
            "2026-10-04", "chem", "fixture", {}}));
        assert(gangyi::agentLearnerFingerprint(db, access) != initialFingerprint);

        const Json event = {{"type", "answer"}, {"courseId", "chem"}, {"requestId", "one"}, {"text", "化合价升降"}};
        const auto task = gangyi::agentSubmit(db, access, event);
        assert(gangyi::agentSubmit(db, access, event)["id"] == task["id"]);
        bool mismatch = false;
        try { auto changed = event; changed["text"] = "不同回答"; gangyi::agentSubmit(db, access, changed); }
        catch (...) { mismatch = true; }
        assert(mismatch);
        bool denied = false;
        try { gangyi::agentView(db, {"another-user", {"chem"}}, task["id"]); }
        catch (...) { denied = true; }
        assert(denied);

        int calls = 0, writes = 0;
        gangyi::LearningAgent agent(path, [&](const auto& options, const auto& sink) {
            assert(options.maxTokens == 8192 && options.timeoutMs == 60000);
            ++calls;
            if (calls == 1) return output({
                {"message", "根据你写出的理由，先补上氧化与还原的关系。"},
                {"actions", {{{"id", "write"}, {"tool", "record_fixture"}, {"args", {{"value", "真实模型选择的动作"}}}}}},
                {"state", "continue"}}, sink);
            if (calls == 2) return output({{"message", "确认已经保存的动作。"},
                {"actions", {{{"id", "write"}, {"tool", "record_fixture"}, {"args", {{"value", "真实模型选择的动作"}}}}}},
                {"state", "continue"}}, sink);
            if (calls < 25) return output({{"message", "继续检查真实记录。"},
                {"actions", {{{"id", "read-" + std::to_string(calls)}, {"tool", "read_context"}, {"args", Json::object()}}}},
                {"state", "continue"}}, sink);
            return output({{"message", "请你解释为什么铁的化合价升高。"}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
        });
        agent.registerTool("record_fixture", {"保存虚构测试动作", false,
            [&](gangyi::Database&, const Json& args, const Json&, const gangyi::AIResult& provenance) {
                assert(provenance.model == "fixture-real-protocol");
                return gangyi::PreparedAgentTool{{{"ok", true}}, [&, value = args["value"]](gangyi::Database& connection) {
                    connection.setProfileMeta("fixture-agent-action", value.get<std::string>()); ++writes;
                }};
            }});
        agent.process(db, task["id"]);
        const auto finished = gangyi::agentView(db, access, task["id"]);
        assert(finished["status"] == "waiting_student" && finished["calls"] == 25 && writes == 1);
        assert(finished["model"] == "fixture-real-protocol");
        assert(db.profileMeta("fixture-agent-action") == "真实模型选择的动作");
        assert(!finished.contains("messages") && !finished.contains("results") && !finished.contains("courseIds"));
        for (int i = 0; i < 5; ++i) gangyi::agentView(db, access);
        agent.process(db, task["id"]); assert(calls == 25);

        int badCalls = 0;
        gangyi::LearningAgent bad(path, [&](const auto&, const auto& sink) {
            ++badCalls; return output({{"message", "错误输出"}, {"actions", "不是数组"}, {"state", "completed"}}, sink);
        });
        const auto failed = gangyi::agentSubmit(db, access, {{"type", "prepare_next"}, {"requestId", "bad"}});
        bad.process(db, failed["id"]);
        assert(badCalls == 3 && gangyi::agentView(db, access, failed["id"])["status"] == "failed");
        bad.process(db, failed["id"]); assert(badCalls == 3);
        const auto retry = gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", failed["id"]}});
        assert(retry["status"] == "pending");
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", failed["id"]}});

        int mixedCalls = 0;
        gangyi::LearningAgent mixed(path, [&](const auto&, const auto& sink) {
            if (++mixedCalls == 2) throw gangyi::AIClientError("timeout", "虚构超时");
            return output({{"message", "无效结构"}, {"actions", "无效"}, {"state", "completed"}}, sink);
        });
        const auto mixedTask = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "mixed"}});
        mixed.process(db, mixedTask["id"]);
        assert(mixedCalls == 3 && gangyi::agentView(db, access, mixedTask["id"])["status"] == "failed");

        // 保留模型的原始错误响应并反馈具体解析错误，不能补齐半成品后执行。
        const std::string malformed = R"({"message":"准备调整","actions":[{"id":"syntax-write","tool":"syntax_fixture","args":{}}],"state":"completed")";
        int syntaxCalls = 0, syntaxWrites = 0;
        gangyi::LearningAgent corrected(path, [&](const auto& options, const auto& sink) -> gangyi::AIResult {
            if (++syntaxCalls == 1) { sink(malformed); return {malformed, "fixture", 200, "stop", "not_requested", {}}; }
            assert(syntaxWrites == 0);
            bool original = false, preciseError = false;
            for (const auto& message : options.messages) {
                original = original || (message.role == "assistant" && message.content == malformed);
                preciseError = preciseError || message.content.find("parse_error") != std::string::npos;
            }
            assert(original && preciseError);
            return output({{"message", "模型已修正完整响应"}, {"actions", {{{"id", "syntax-write"}, {"tool", "syntax_fixture"}, {"args", Json::object()}}}}, {"state", "completed"}}, sink);
        });
        corrected.registerTool("syntax_fixture", {"验证只执行完整响应", false,
            [&](gangyi::Database&, const Json&, const Json&, const gangyi::AIResult&) {
                return gangyi::PreparedAgentTool{{{"ok", true}}, [&](gangyi::Database&) { ++syntaxWrites; }};
            }});
        const auto syntaxTask = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "syntax"}});
        corrected.process(db, syntaxTask["id"]);
        assert(syntaxCalls == 2 && syntaxWrites == 1 && gangyi::agentView(db, access, syntaxTask["id"])["status"] == "ready");

        const auto atomic = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "atomic"}});
        gangyi::LearningAgent rollback(path, [&](const auto&, const auto& sink) {
            return output({{"message", "调整"}, {"actions", {{{"id", "rollback"}, {"tool", "rollback_fixture"}, {"args", Json::object()}}}},
                {"state", "completed"}}, sink);
        });
        rollback.registerTool("rollback_fixture", {"验证事务恢复", false,
            [](gangyi::Database&, const Json&, const Json&, const gangyi::AIResult&) {
                return gangyi::PreparedAgentTool{{{"ok", true}}, [](gangyi::Database& connection) {
                    connection.setProfileMeta("must-not-remain", "半成品"); throw std::runtime_error("虚构保存失败");
                }};
            }});
        rollback.process(db, atomic["id"]);
        assert(db.profileMeta("must-not-remain").empty());
        assert(gangyi::agentView(db, access, atomic["id"])["status"] == "failed");

        const auto paused = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "paused"}});
        gangyi::agentControl(db, access, {{"command", "pause"}, {"taskId", paused["id"]}});
        const int previous = calls; agent.process(db, paused["id"]); assert(calls == previous);
        assert(gangyi::agentView(db, access, paused["id"])["status"] == "paused");
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", paused["id"]}});

        // 另一个窗口的新作答使旧行动失效，不能在重试中覆盖它。
        const auto stale = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "stale"}});
        gangyi::LearningAgent concurrent(path, [&](const auto&, const auto& sink) {
            return output({{"message", "准备调整"}, {"actions", {{{"id", "stale-write"}, {"tool", "concurrent_fixture"},
                {"args", Json::object()}}}}, {"state", "completed"}}, sink);
        });
        concurrent.registerTool("concurrent_fixture", {"模拟其他窗口作答", false,
            [&](gangyi::Database&, const Json&, const Json&, const gangyi::AIResult&) {
                gangyi::Database another; another.open(path);
                another.insert(gangyi::LearningInteraction{"new-answer", "chat-user", "{\"text\":\"另一窗口的新回答\"}",
                    "2026-10-04", "chem", "fixture", {}});
                return gangyi::PreparedAgentTool{{{"ok", true}}, [](gangyi::Database& connection) {
                    connection.setProfileMeta("stale-write", "不得提交");
                }};
            }});
        concurrent.process(db, stale["id"]);
        assert(gangyi::agentView(db, access, stale["id"])["status"] == "superseded");
        assert(db.profileMeta("stale-write").empty());

        // 关闭时暂停正在传输的任务，重新打开后恢复；完成后不自发增加调用。
        const auto interrupted = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "shutdown"}});
        std::atomic_int activeCalls{0};
        gangyi::LearningAgent background(path, [&](const auto& options, const auto& sink) -> gangyi::AIResult {
            ++activeCalls; sink("{\"message\":\"正在备课");
            while (!options.cancelled()) std::this_thread::sleep_for(std::chrono::milliseconds(5));
            throw gangyi::AIClientError("cancelled", "关闭软件");
        });
        background.start();
        for (int i = 0; activeCalls == 0 && i < 1000; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(5));
        assert(activeCalls == 1); background.stop();
        const auto pausedByShutdown = gangyi::agentView(db, access, interrupted["id"]);
        assert(pausedByShutdown["status"] == "paused" && pausedByShutdown["pauseReason"] == "shutdown");
        gangyi::LearningAgent recovered(path, [&](const auto&, const auto& sink) {
            ++activeCalls; return output({{"message", "已恢复完成。"}, {"actions", Json::array()}, {"state", "completed"}}, sink);
        });
        recovered.start();
        for (int i = 0; i < 1000 && gangyi::agentView(db, access, interrupted["id"])["status"] != "ready"; ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        assert(gangyi::agentView(db, access, interrupted["id"])["status"] == "ready");
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); recovered.stop();
        assert(activeCalls == 2);

        // 模型决定真实数据调整；撤回后不能立即重做，也不能覆盖后续用户编辑。
        Json toolArgs; std::string toolName; int preferenceCalls = 0;
        gangyi::LearningAgent preferences(path, [&](const auto&, const auto& sink) {
            ++preferenceCalls;
            return output({{"message", "按最新学习反馈调整，并保留撤回入口。"},
                {"actions", {{{"id", "change"}, {"tool", toolName}, {"args", toolArgs}}}}, {"state", "completed"}}, sink);
        });
        gangyi::registerAgentPreferenceTools(preferences);
        int preferenceEvent = 0;
        auto apply = [&](bool manual = false) {
            const auto request = gangyi::agentSubmit(db, access, {{"type", manual ? "schedule_replan" : "adjust"},
                {"requestId", "preference-" + std::to_string(++preferenceEvent)}},
                manual ? gangyi::AgentSubmissionOrigin::manualScheduleReplan : gangyi::AgentSubmissionOrigin::ordinary);
            preferences.process(db, request["id"]);
            return gangyi::agentView(db, access, request["id"]);
        };
        toolName = "adjust_goal";
        toolArgs = {{"courseId", "chem"}, {"goal", "先理解化合价，再学习氧化还原"}, {"reason", "实际回答显示概念需要梳理"}};
        const auto changedGoal = apply();
        assert(changedGoal["status"] == "ready" && changedGoal["changes"].size() == 1);
        assert(db.getCourse("chem")->goal == toolArgs["goal"]);
        const auto changeId = changedGoal["changes"][0]["id"];
        bool undoDenied = false;
        try { gangyi::agentControl(db, {"someone-else", {"chem"}}, {{"command", "undo"}, {"changeId", changeId}}); }
        catch (...) { undoDenied = true; }
        assert(undoDenied);
        const auto undoTask = gangyi::agentControl(db, access, {{"command", "undo"}, {"changeId", changeId}});
        assert(db.getCourse("chem")->goal == "氧化还原");
        assert(gangyi::agentChanges(db, access)[0]["status"] == "undone");
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", undoTask["id"]}});
        const int beforeRepeat = preferenceCalls;
        const auto repeated = apply();
        assert(repeated["status"] == "failed" && preferenceCalls - beforeRepeat == 3);
        assert(db.getCourse("chem")->goal == "氧化还原");
        db.insert(gangyi::LearningInteraction{"goal-feedback", "chat-user", "{\"text\":\"现在我想先补化合价\"}",
            "2026-10-04", "chem", "fixture", {}});
        const auto newGoal = apply(); assert(newGoal["status"] == "ready");
        auto editedCourse = *db.getCourse("chem"); editedCourse.goal = "用户后来修改的目标";
        editedCourse.updatedAt = "2099-01-01"; db.update(editedCourse); db.markLearningDirty();
        bool conflict = false;
        try { gangyi::agentControl(db, access, {{"command", "undo"}, {"changeId", newGoal["changes"][0]["id"]}}); }
        catch (...) { conflict = true; }
        assert(conflict && db.getCourse("chem")->goal == "用户后来修改的目标");

        toolName = "update_recommendations";
        toolArgs = {{"lite", {"先判断化合价", "比较氧化与还原", "辨认氧化剂", "尝试电池现象", "复习英语句子"}},
            {"deep", {"梳理氧化还原", "研究电子守恒", "阅读化学史", "系统了解电化学", "探索遗传概率"}},
            {"reason", "根据新学习情况自主选择补弱与探索比例"}};
        const auto recommendations = apply(); assert(recommendations["status"] == "ready");
        assert(gangyi::agentRecommendationView(db, access)["items"]["lite"].size() == 5);
        assert(gangyi::agentRecommendationView(db, {"another", {}}).empty());
        const int beforeRead = preferenceCalls;
        gangyi::agentRecommendationView(db, access); gangyi::agentChanges(db, access);
        assert(preferenceCalls == beforeRead);

        db.insert(gangyi::CourseSnapshot{"outline-fixture", "chem", 1,
            Json{{"courseStructure", {{{"topics", {"化合价", "氧化还原"}}}}}}.dump(), "2026-10-04"});
        std::tm calendar{}; calendar.tm_year = 2099 - 1900; calendar.tm_mon = 0; calendar.tm_mday = 5;
        calendar.tm_hour = 12; std::mktime(&calendar);
        const int day = calendar.tm_wday == 0 ? 7 : calendar.tm_wday;
        const Json entry = {{"taskId", "lesson:chem:1:1"}, {"courseId", "chem"}, {"kind", "lesson"},
            {"date", "2099-01-05"}, {"minutes", 30}, {"order", 0}, {"phaseIndex", 1}, {"topicIndex", 1}, {"manual", true}};
        const Json initialPlan = {{"plan", {{"version", 10}, {"availability", {{{"weekday", day}, {"minutes", 30}}}},
            {"entries", {entry}}}}, {"status", "ready"}};
        db.setProfileMeta("learning-flow", initialPlan.dump());
        toolName = "adjust_schedule";
        auto longer = entry; longer["minutes"] = 300;
        toolArgs = {{"availability", {{{"weekday", day}, {"minutes", 300}}}}, {"entries", {longer}},
            {"reason", "模型调整共享预算与手动安排，旧的240分钟限制不再决定教学"}};
        // 非手动任务即使模型直接调用工具，也不能调整或生成课表候选。
        const auto automatic = apply(); assert(automatic["status"] == "failed");
        assert(Json::parse(db.profileMeta("learning-flow")) == initialPlan);
        const auto scheduled = apply(true); assert(scheduled["status"] == "ready");
        auto savedPlan = Json::parse(db.profileMeta("learning-flow"));
        assert(savedPlan["plan"] == initialPlan["plan"] && savedPlan["proposal"]["plan"]["entries"][0]["minutes"] == 300);
        const std::string scheduleChangeId = savedPlan["proposal"]["id"];
        gangyi::studyPlanProposal(db, {{"version", 10}, {"proposalId", scheduleChangeId}}, "confirm");
        // 与接口确认动作相同，将同一候选记为已应用，供撤回校验。
        auto journalRow = *db.getClassroomActivity(scheduleChangeId); auto journalValue = Json::parse(journalRow.payload);
        journalValue["status"] = "applied"; journalValue["after"] = db.profileMeta("learning-flow"); journalRow.payload = journalValue.dump(); db.upsert(journalRow);
        savedPlan = Json::parse(db.profileMeta("learning-flow"));
        assert(savedPlan["plan"]["version"] == 11 && savedPlan["plan"]["entries"][0]["minutes"] == 300);
        const auto undonePlan = gangyi::agentControl(db, access, {{"command", "undo"}, {"changeId", scheduleChangeId}});
        savedPlan = Json::parse(db.profileMeta("learning-flow"));
        assert(savedPlan["plan"]["version"] == 12 && savedPlan["plan"]["entries"][0]["minutes"] == 30);
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", undonePlan["id"]}});

        auto overBudget = longer; overBudget["minutes"] = 400;
        toolArgs["entries"] = {overBudget};
        const auto invalidPlan = apply(true); assert(invalidPlan["status"] == "failed");
        assert(Json::parse(db.profileMeta("learning-flow")) == savedPlan);
        overBudget["completed"] = true; toolArgs["entries"] = {overBudget};
        assert(apply(true)["status"] == "failed");
        assert(Json::parse(db.profileMeta("learning-flow")) == savedPlan);

        const auto future = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) + 90;
        db.setProfileMeta("study-drafts", Json{{"editing-window", {{"expires", future}, {"text", "用户未保存的编辑"}}}}.dump());
        auto minute45 = entry; minute45["minutes"] = 45;
        toolArgs["availability"] = {{{"weekday", day}, {"minutes", 45}}}; toolArgs["entries"] = {minute45};
        const auto protectedDraft = apply(true); assert(protectedDraft["status"] == "ready");
        const auto protectedPlan = Json::parse(db.profileMeta("learning-flow"));
        assert(protectedPlan.at("plan") == savedPlan.at("plan"));
        assert(protectedPlan.at("proposal").at("previousPlan") == savedPlan.at("plan"));
        assert(protectedPlan.at("proposal").at("plan").at("entries")[0].at("minutes") == 45);
        assert(!db.profileMeta("agent-schedule-proposal").empty());
        assert(Json::parse(db.profileMeta("study-drafts"))["editing-window"]["text"] == "用户未保存的编辑");

        // AI 自主准备两块课件，程序不补齐固定六块；公开内容不能包含标准答案。
        int lessonCalls = 0; std::string preparedLessonId;
        gangyi::LearningAgent lessonAgent(path, [&](const auto& options, const auto& sink) {
            ++lessonCalls;
            Json actions = Json::array();
            if (lessonCalls == 1) actions.push_back({{"id", "new-lesson"}, {"tool", "create_lesson"},
                {"args", {{"courseId", "chem"}, {"title", "只先理解化合价"}, {"purpose", "依据实际回答补齐一个概念"}}}});
            if (lessonCalls == 2) {
                // 模型从上一次真实工具返回值取得课时身份。
                const auto returned = Json::parse(options.messages.back().content);
                preparedLessonId = returned.at("toolResults")[0]["result"]["lessonId"];
                actions.push_back({{"id", "explain"}, {"tool", "append_section"}, {"args", {{"lessonId", preparedLessonId},
                    {"section", {{"kind", "explanation"}, {"title", "先比较前后"}, {"body", "从元素化合价的变化开始比较。"}}}}}});
                actions.push_back({{"id", "one-question"}, {"tool", "append_section"}, {"args", {{"lessonId", preparedLessonId},
                    {"section", {{"kind", "question"}, {"title", "你来判断"}, {"question", {{"question", "铁变为亚铁离子时化合价如何变化？"},
                        {"type", "open"}, {"expectedAnswer", "由0升高到+2"}, {"rubric", "指出由0变为+2并联系失电子"}}}}}}}});
            }
            if (lessonCalls == 3) actions.push_back({{"id", "complete"}, {"tool", "finish_lesson"}, {"args", {{"lessonId", preparedLessonId}}}});
            return output({{"message", "正在根据你的情况准备下一课。"}, {"actions", actions},
                {"state", lessonCalls < 3 ? "continue" : "completed"}}, sink);
        });
        gangyi::registerAgentLessonTools(lessonAgent);
        const auto next = gangyi::agentSubmit(db, access, {{"type", "prepare_next"}, {"requestId", "flexible-lesson"}, {"navigate", true}});
        lessonAgent.process(db, next["id"]);
        const auto readyLessonTask = gangyi::agentView(db, access, next["id"]);
        assert(readyLessonTask["status"] == "ready" && lessonCalls == 3 && readyLessonTask["lesson"]["id"] == preparedLessonId);
        const auto shown = gangyi::agentLessonView(db, access, preparedLessonId);
        assert(shown["sections"].size() == 2 && shown["sections"][1]["kind"] == "question");
        assert(shown.dump().find("expectedAnswer") == std::string::npos && shown.dump().find("rubric") == std::string::npos);
        assert(shown.dump().find("由0升高到+2") == std::string::npos);
        const auto entered = gangyi::agentControl(db, access, {{"command", "enter_lesson"}, {"lessonId", preparedLessonId}});
        assert(entered["href"].get<std::string>().find("/learn?lessonId=") == 0);
        // 同一课时复用开课任务；真实主控选择教学焦点，读取不增加调用或掌握证据。
        const auto beforeStartRecords = db.listInteractions().size();
        const auto start = gangyi::agentControl(db, access, {{"command", "start_lesson"}, {"lessonId", preparedLessonId}});
        assert(gangyi::agentControl(db, access, {{"command", "start_lesson"}, {"lessonId", preparedLessonId}})["id"] == start["id"]);
        assert(db.listInteractions().size() == beforeStartRecords);
        int teachingCalls = 0; std::string interactionSectionId;
        gangyi::LearningAgent teacher(path, [&](const auto& options, const auto& sink) {
            Json actions = Json::array();
            if (++teachingCalls == 1) {
                actions.push_back({{"id", "interaction"}, {"tool", "append_section"}, {"args", {{"lessonId", preparedLessonId},
                    {"section", {{"kind", "question"}, {"title", "当前段先想一想"}, {"questionKind", "interaction"},
                        {"question", {{"question", "化合价升高和电子变化有什么联系？"}, {"type", "open"}, {"expectedAnswer", "失去电子"}}}}}}}});
            } else {
                const auto returned = Json::parse(options.messages.back().content);
                interactionSectionId = returned.at("toolResults")[0]["result"]["sectionId"];
                actions.push_back({{"id", "focus"}, {"tool", "select_teaching_focus"}, {"args", {{"lessonId", preparedLessonId},
                    {"sectionId", shown["sections"][0]["id"]}, {"interactionSectionId", interactionSectionId},
                    {"practiceSectionId", shown["sections"][1]["id"]}}}});
            }
            return output({{"message", teachingCalls == 1 ? "工具处理中，这段还没有显示。" : "先比较化合价，再说说你对电子变化的理解。"},
                {"actions", actions}, {"state", teachingCalls == 1 ? "continue" : "waiting_student"}}, sink);
        });
        gangyi::registerAgentLessonTools(teacher);
        teacher.process(db, start["id"]);
        const auto teaching = gangyi::agentView(db, access, start["id"]);
        assert(teaching["status"] == "waiting_student" && teachingCalls == 2);
        const auto focused = gangyi::agentLessonView(db, access, preparedLessonId);
        assert(focused["teachingFocus"]["interactionSectionId"] == interactionSectionId);
        assert(focused["teachingFocus"]["sectionId"] == shown["sections"][0]["id"]);
        assert(focused["teachingTaskId"] == start["id"] && focused["initialTeachingTaskId"] == start["id"]);
        assert(!focused["sections"][0]["displayed"].get<bool>());
        bool exposureDenied = false;
        try { gangyi::agentExposeTask(db, access, {{"lessonId", preparedLessonId}, {"sections", {{{"id", shown["sections"][0]["id"]}, {"version", 99}}}}}); }
        catch (...) { exposureDenied = true; }
        assert(exposureDenied && db.profileMeta("agent-section-exposure:" + preparedLessonId).empty());
        gangyi::agentExposeTask(db, access, {{"lessonId", preparedLessonId}, {"sections", {{{"id", shown["sections"][0]["id"]}, {"version", 1}}}}});
        const auto displayedLesson = gangyi::agentLessonView(db, access, preparedLessonId);
        assert(displayedLesson["sections"][0]["displayed"].get<bool>() && !displayedLesson["sections"][1]["displayed"].get<bool>());
        Json displayed = Json::array();
        for (const auto& item : teaching["events"])
            if (item.value("type", "") == "delta" && item.value("field", "") == "message" && item.value("step", 0) == 2)
                displayed.push_back(item.at("seq"));
        assert(!displayed.empty());
        gangyi::agentExposeTask(db, access, {{"taskId", start["id"]}, {"seq", teaching.at("seq")}, {"displayedSequences", displayed}});
        const auto readFocus = gangyi::agentLessonView(db, access, preparedLessonId);
        assert(readFocus["teachingDialog"].size() == 1);
        assert(readFocus["teachingDialog"][0]["text"] == "先比较化合价，再说说你对电子变化的理解。");
        assert(readFocus.dump().find("失去电子") == std::string::npos);
        for (int i = 0; i < 5; ++i) gangyi::agentControl(db, access, {{"command", "start_lesson"}, {"lessonId", preparedLessonId}});
        teacher.process(db, start["id"]);
        assert(teachingCalls == 2 && db.listInteractions().size() == beforeStartRecords);
        // 程序拒绝把练习题误选为讲解段，不允许用错误焦点覆盖已保存位置。
        const auto invalidFocus = gangyi::agentSubmit(db, access, {{"type", "lesson_focus_request"}, {"courseId", "chem"},
            {"lessonId", preparedLessonId}, {"requestId", "invalid-focus"}});
        gangyi::LearningAgent invalidTeacher(path, [&](const auto&, const auto& sink) {
            return output({{"message", "错误的板块类型"}, {"actions", {{{"id", "wrong-focus"}, {"tool", "select_teaching_focus"},
                {"args", {{"lessonId", preparedLessonId}, {"sectionId", shown["sections"][1]["id"]}}}}}}, {"state", "waiting_student"}}, sink);
        });
        gangyi::registerAgentLessonTools(invalidTeacher);
        invalidTeacher.process(db, invalidFocus["id"]);
        assert(gangyi::agentView(db, access, invalidFocus["id"])["status"] == "failed");
        assert(gangyi::agentLessonView(db, access, preparedLessonId)["teachingFocus"] == focused["teachingFocus"]);
        Json responseEvent = {{"type", "question_answer"}, {"requestId", "real-input"}, {"courseId", "chem"},
            {"lessonId", preparedLessonId}, {"sectionId", shown["sections"][1]["id"]}, {"sectionVersion", 1},
            {"text", "我认为由0到+2，升高了"}};
        const auto answering = gangyi::agentSubmit(db, access, responseEvent);
        const auto currentInputs = gangyi::agentContext(db, access, responseEvent).at("currentInputs");
        assert(currentInputs.size() == 1 && currentInputs[0].at("interactionId").get<std::string>().find("student-") == 0);
        assert(currentInputs[0].at("sectionId") == responseEvent.at("sectionId") && currentInputs[0].at("requestId") == "real-input");
        const auto beforeDuplicate = db.listInteractions().size();
        assert(gangyi::agentSubmit(db, access, responseEvent)["id"] == answering["id"]);
        assert(db.listInteractions().size() == beforeDuplicate);
        auto wrongVersion = responseEvent; wrongVersion["requestId"] = "wrong-version"; wrongVersion["sectionVersion"] = 2;
        bool versionDenied = false;
        try { gangyi::agentSubmit(db, access, wrongVersion); } catch (...) { versionDenied = true; }
        assert(versionDenied && db.listInteractions().size() == beforeDuplicate);
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", answering["id"]}});

        std::string actualAnswerId;
        for (const auto& item : db.listInteractions()) if (item.kind == "chat-user" &&
            Json::parse(item.payload).value("requestId", "") == "real-input") {
                actualAnswerId = item.id;
                const auto input = Json::parse(item.payload);
                assert(input["assisted"].get<bool>() && input["hintHistory"][0] == "从元素化合价的变化开始比较。");
            }
        assert(!actualAnswerId.empty());
        gangyi::LearningAgent evaluation(path, [&](const auto&, const auto& sink) {
            return output({{"message", "你指出了化合价升高，可进一步联系失电子。"},
                {"actions", {{{"id", "evaluate"}, {"tool", "evaluate_answer"}, {"args", {{"interactionId", actualAnswerId},
                    {"score", 80}, {"credible", true}, {"feedback", "化合价变化判断正确，过程可补充电子关系"},
                    {"reason", "引用学生实际写出的0到+2"}, {"uncertainty", "仅能确认本题判断，不能推断全部知识"}}}}}},
                {"state", "waiting_student"}}, sink);
        });
        gangyi::registerAgentLessonTools(evaluation);
        const auto assessing = gangyi::agentSubmit(db, access, {{"type", "assess"}, {"requestId", "one-real-question"}});
        evaluation.process(db, assessing["id"]);
        assert(gangyi::agentView(db, access, assessing["id"])["status"] == "waiting_student");
        bool assessmentFound = false;
        for (const auto& item : db.listInteractions()) if (item.kind == "practice") {
            const auto saved = Json::parse(item.payload);
            if (saved.value("interactionId", "") == actualAnswerId) {
                assessmentFound = true; assert(saved["score"] == 80 && saved["questionId"] == shown["sections"][1]["questionId"]);
                assert(saved["source"] == "ai" && saved["questionSnapshot"]["expectedAnswer"] == "由0升高到+2");
            }
        }
        assert(assessmentFound);
        const auto summary = gangyi::agentLessonView(db, access, preparedLessonId);
        assert(summary["evaluations"].size() == 1 && summary["evaluations"][0]["score"] == 80);
        assert(summary.dump().find("expectedAnswer") == std::string::npos && summary.dump().find("rubric") == std::string::npos);

        // 重现真实备课错误：证据是数组，但工具返回必须有对象外壳。
        int evidenceCalls = 0;
        gangyi::LearningAgent evidenceReader(path, [&](const auto& options, const auto& sink) {
            if (++evidenceCalls == 1) return output({{"message", "读取真实证据"}, {"actions", {{{"id", "evidence"}, {"tool", "read_profile_evidence"}, {"args", Json::object()}}}}, {"state", "continue"}}, sink);
            bool found = false;
            for (const auto& item : options.messages) if (item.role == "user") {
                const auto value = Json::parse(item.content, nullptr, false);
                if (value.is_object() && value.contains("toolResults")) found = value["toolResults"][0]["result"]["evidence"].is_array();
            }
            assert(found); return output({{"message", "证据读取完成"}, {"actions", Json::array()}, {"state", "completed"}}, sink);
        });
        const auto evidenceTask = gangyi::agentSubmit(db, access, {{"type", "inspect"}, {"requestId", "evidence-object"}});
        evidenceReader.process(db, evidenceTask["id"]);
        assert(evidenceCalls == 2 && gangyi::agentView(db, access, evidenceTask["id"])["status"] == "ready");

        // 服务商明确拒绝立即停止，可恢复错误才做三次有限重试。
        for (const auto& spec : std::vector<std::pair<std::string, int>>{{"missing_config",0},{"provider_rejected",402},{"auth_error",401},{"rate_limited",429},{"provider_5xx",503},{"timeout",0}}) {
            int attempts = 0;
            gangyi::LearningAgent failing(path, [&](const auto&, const auto&) -> gangyi::AIResult { ++attempts; throw gangyi::AIClientError(spec.first, "虚构服务商错误", spec.second); });
            const auto request = gangyi::agentSubmit(db, access, {{"type", "inspect"}, {"requestId", "provider-" + spec.first}});
            failing.process(db, request["id"]); const auto result = gangyi::agentView(db, access, request["id"]);
            const bool canRetry = spec.first != "provider_rejected" && spec.first != "auth_error" && spec.first != "missing_config";
            assert(attempts == (canRetry ? 3 : 1) && result["status"] == "failed");
            assert(result["failure"]["httpStatus"] == spec.second && result["failure"]["retryable"] == canRetry);
            if (spec.first == "missing_config") assert(result["failure"]["message"].get<std::string>().find("AI 设置") != std::string::npos);
        }

        // 学生在练习页请求选择题：AI 追加的互动题也必须接续显示在当前页。
        gangyi::LearningAgent followup(path, [&](const auto&, const auto& sink) {
            return output({{"message", "请在新选择题中作答"}, {"actions", {{{"id", "followup"}, {"tool", "append_section"},
                {"args", {{"lessonId", preparedLessonId}, {"questionKind", "interaction"}, {"section", {{"kind", "question"}, {"title", "根据回答追加的选择题"},
                    {"question", {{"question", "化合价升高表示什么？"}, {"type", "choice"}, {"options", {"失去电子", "得到电子"}}, {"expectedAnswer", "失去电子"}}}}}}}}}}, {"state", "waiting_student"}}, sink);
        });
        gangyi::registerAgentLessonTools(followup);
        const auto followupTask = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"lessonId", preparedLessonId}, {"courseId", "chem"},
            {"view", "practice"}, {"text", "请根据我的回答出一道选择题"}, {"requestId", "followup-choice"}});
        followup.process(db, followupTask["id"]);
        const auto followupLesson = gangyi::agentLessonView(db, access, preparedLessonId);
        assert(followupLesson["teachingFocus"]["responseView"] == "practice");
        assert(followupLesson["teachingFocus"]["responseSectionId"] == followupLesson["sections"].back()["id"]);
        assert(followupLesson["sections"].back()["question"]["options"].size() == 2);
        assert(followupLesson.dump().find("expectedAnswer") == std::string::npos);
        auto legacyRow = *db.getClassroomActivity(preparedLessonId);
        auto legacyLesson = Json::parse(legacyRow.payload);
        for (const auto* key : {"responseSectionId", "responseView", "responseTaskId"}) legacyLesson["teachingFocus"].erase(key);
        legacyRow.payload = legacyLesson.dump(); db.upsert(legacyRow);
        const auto recoveredLesson = gangyi::agentLessonView(db, access, preparedLessonId);
        assert(recoveredLesson["teachingFocus"]["responseSectionId"] == followupLesson["sections"].back()["id"]);
        assert(recoveredLesson["teachingFocus"]["responseView"] == "practice");
        assert(db.getClassroomActivity(preparedLessonId)->payload == legacyRow.payload);
    }
    std::filesystem::remove_all(root);
    std::cout << "多步 AI 主控、幂等、访问隔离、失败停止与事务回滚测试通过\n";
}

#pragma once

#include "teaching_preferences.hpp"
#include "learning_state.hpp"

// 固定模型只验证偏好、并发与恢复，不把模板回复当成真实教学效果。
inline void runTeachingPreferencesAgentChecks(const std::filesystem::path& path) {
    const gangyi::AgentAccess access{"teaching-intent-check", {"math"}};
    Json lastPreferences;
    int modelCalls = 0;
    {
        gangyi::Database db; db.open(path.string()); db.migrate();
        assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "分数", "deep", "数学", {}, "ai", "active", "2026-11-17", "2026-11-17"}));
        const Json lesson = {{"id", "intent-lesson"}, {"scopeId", access.scopeId}, {"courseId", "math"},
            {"title", "分数"}, {"purpose", "理解分数单位"}, {"status", "ready"}, {"entered", true},
            {"version", 1}, {"updatedAt", "2026-11-17"}, {"sourceLearningVersion", db.learningRevision()},
            {"teachingFocus", {{"practiceSectionId", "q1"}}},
            {"sections", {{{"id", "q1"}, {"kind", "question"}, {"questionId", "intent-q1"}, {"version", 1},
                {"question", {{"question", "1/2+1/3=?"}, {"expectedAnswer", "PRIVATE-INTENT-ANSWER"}}}}}}};
        assert(db.upsert(gangyi::ClassroomActivity{"intent-lesson", "math", "agent-lesson", lesson.dump(), "2026-11-17", 0, 0}));
        const auto shown = [&] { return gangyi::agentLessonView(db, access, "intent-lesson"); };
        const auto basis = gangyi::learningStateBasis(shown());
        const auto eventFor = [&](const std::string& intent, const std::string& requestId) -> Json {
            return {{"type", "teaching_request"}, {"courseId", "math"}, {"lessonId", "intent-lesson"},
                {"requestType", intent}, {"teachingPreferenceVersion", shown().at("teachingPreferences").at("version")},
                {"text", "这是学生明确选择的本课教学方式"}, {"requestId", requestId}};
        };
        const auto raw = [&](const Json& task) { return Json::parse(db.getClassroomActivity(task.at("id"))->payload); };
        gangyi::LearningAgent agent(path.string(), [&](const gangyi::ChatOptions& options, const auto& sink) {
            ++modelCalls;
            bool sawPreferences = false;
            for (const auto& message : options.messages) if (message.role == "user") {
                const auto value = Json::parse(message.content, nullptr, false);
                if (!value.is_object() || !value.contains("event")) continue;
                assert(value.at("lessonContext").at("teachingPreferences") == shown().at("teachingPreferences"));
                sawPreferences = true;
            }
            assert(sawPreferences);
            return output({{"teaching", {{"helpType", "none"}, {"demonstratedQuestions", Json::array()}}},
                {"message", "本课教学选择已保存，可以继续学习。"}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
        });
        gangyi::registerAgentLessonTools(agent);
        for (const auto& intent : {"simplify", "slow_down", "speed_up", "give_example", "independent_attempt", "resume_guidance", "reset_preferences"}) {
            const auto event = eventFor(intent, "intent-" + std::string(intent));
            const auto before = shown().at("teachingPreferences");
            const auto task = gangyi::agentSubmit(db, access, event);
            const auto updated = shown().at("teachingPreferences");
            assert(updated.at("version") == before.at("version").get<int>() + 1);
            assert(updated.at("lastIntent") == intent);
            const auto interactionCount = db.listInteractions().size();
            assert(gangyi::agentSubmit(db, access, event).at("id") == task.at("id"));
            assert(shown().at("teachingPreferences") == updated && db.listInteractions().size() == interactionCount);
            const auto callsBefore = modelCalls;
            agent.process(db, task.at("id"));
            assert(raw(task).at("status") == "waiting_student");
            if (std::string(intent) == "independent_attempt") {
                assert(modelCalls == callsBefore && raw(task).at("calls") == 0);
                assert(updated.at("guidance") == "independent");
                assert(!raw(task).contains("teachingHelp") && !raw(task).contains("validSteps"));
            } else assert(modelCalls == callsBefore + 1);
            assert(shown().at("evaluations").empty());
            assert(gangyi::learningStateBasis(shown()) == basis);
            assert(shown().dump().find("PRIVATE-INTENT") == std::string::npos);
        }
        assert(shown().at("teachingPreferences").at("pace") == "normal");
        assert(shown().at("teachingPreferences").at("explanation") == "standard");

        const auto chat = [](const std::string& requestId, const std::string& text) -> Json {
            return {{"type", "chat"}, {"courseId", "math"}, {"lessonId", "intent-lesson"},
                    {"requestId", requestId}, {"text", text}};
        };
        const auto slow = chat("typed-slow", " 请讲慢一点！ ");
        const auto slowTask = gangyi::agentSubmit(db, access, slow);
        assert(shown().at("teachingPreferences").at("pace") == "slow");
        agent.process(db, slowTask.at("id"));
        const auto rephrase = gangyi::agentSubmit(db, access, chat("typed-rephrase", "换种解释。"));
        assert(raw(rephrase).at("event").at("requestType") == "rephrase");
        agent.process(db, rephrase.at("id"));
        const auto retry = gangyi::agentSubmit(db, access, chat("typed-retry", "再测一次。"));
        assert(raw(retry).at("event").at("requestType") == "retry_question");
        assert(raw(retry).at("event").at("sectionId") == "q1" && raw(retry).at("event").at("sectionVersion") == 1);
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", retry.at("id")}});

        const auto typedSelf = gangyi::agentSubmit(db, access, chat("typed-self", "我想自己做。"));
        const int callsBeforeTypedSelf = modelCalls;
        agent.process(db, typedSelf.at("id"));
        assert(modelCalls == callsBeforeTypedSelf && raw(typedSelf).at("status") == "waiting_student");
        const auto typedPrefs = shown().at("teachingPreferences");
        assert(typedPrefs.at("pace") == "slow" && typedPrefs.at("guidance") == "independent");
        const auto interactionsBeforeRetry = db.listInteractions().size();
        assert(gangyi::agentSubmit(db, access, slow).at("id") == slowTask.at("id"));
        assert(shown().at("teachingPreferences") == typedPrefs && db.listInteractions().size() == interactionsBeforeRetry);
        auto different = slow; different["text"] = "请继续讲解";
        bool conflict = false;
        try { gangyi::agentSubmit(db, access, different); }
        catch (const std::invalid_argument&) { conflict = true; }
        assert(conflict && shown().at("teachingPreferences") == typedPrefs);

        const auto quoted = gangyi::agentSubmit(db, access, chat("typed-quote", "“慢一点”在这段话里是什么意思？"));
        assert(raw(quoted).at("event").at("type") == "chat");
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", quoted.at("id")}});
        auto formalAnswer = chat("typed-formal-answer", "我想自己做。");
        formalAnswer["type"] = "question_answer"; formalAnswer["sectionId"] = "q1"; formalAnswer["sectionVersion"] = 1;
        const auto answer = gangyi::agentSubmit(db, access, formalAnswer);
        assert(raw(answer).at("event").at("type") == "question_answer" && shown().at("teachingPreferences") == typedPrefs);
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", answer.at("id")}});

        // 陈旧窗口、非法版本和伪造成绩全部拒绝，并回滚本次输入与任务。
        for (int kind = 0; kind < 6; ++kind) {
            auto event = eventFor("slow_down", "intent-denied-" + std::to_string(kind));
            if (kind == 0) event["teachingPreferenceVersion"] = 0;
            if (kind == 1) event["teachingPreferenceVersion"] = true;
            if (kind == 2) event["teachingPreferenceVersion"] = UINT64_MAX;
            if (kind == 3) event["score"] = 100;
            if (kind == 4) event["teachingPreferences"] = {{"guidance", "independent"}};
            if (kind == 5) event["courseId"] = "another-course";
            const auto count = db.listInteractions().size(), tasks = db.listClassroomActivities("math").size();
            const auto previous = shown().at("teachingPreferences");
            bool denied = false;
            try { gangyi::agentSubmit(db, access, event); } catch (const std::exception&) { denied = true; }
            assert(denied && db.listInteractions().size() == count && db.listClassroomActivities("math").size() == tasks);
            assert(shown().at("teachingPreferences") == previous);
        }
        // 新的自己尝试请求使尚未运行的旧讲解失效；两者都不能暗中请求模型。
        const auto old = gangyi::agentSubmit(db, access, eventFor("slow_down", "intent-old-window"));
        const auto self = gangyi::agentSubmit(db, access, eventFor("independent_attempt", "intent-new-window"));
        const auto beforeCalls = modelCalls;
        agent.process(db, old.at("id")); agent.process(db, self.at("id"));
        assert(modelCalls == beforeCalls && raw(old).at("status") == "superseded");
        assert(raw(self).at("status") == "waiting_student");
        // 正在请求的旧回复也须撤销：新选择在模型调用期间到达，不发布晚到答案。
        const auto inflight = gangyi::agentSubmit(db, access, eventFor("simplify", "intent-inflight"));
        Json interrupt;
        int inflightCalls = 0;
        gangyi::LearningAgent interrupted(path.string(), [&](const gangyi::ChatOptions& options, const auto& sink) {
            ++inflightCalls;
            interrupt = gangyi::agentSubmit(db, access, chat("intent-interrupt", "让我自己试试！"));
            assert(options.cancelled && options.cancelled());
            return output({{"teaching", {{"helpType", "explanation"}, {"demonstratedQuestions", Json::array()}}},
                {"message", "LATE-ANSWER-SHOULD-NOT-DISPLAY"}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
        });
        gangyi::registerAgentLessonTools(interrupted);
        interrupted.process(db, inflight.at("id"));
        interrupted.process(db, interrupt.at("id"));
        assert(inflightCalls == 1 && raw(inflight).at("status") == "superseded");
        const auto publicTask = gangyi::agentView(db, access, inflight.at("id"));
        assert(publicTask.value("message", "").find("LATE-ANSWER") == std::string::npos);
        for (const auto& event : publicTask.at("events"))
            if (event.value("type", "") == "delta") assert(event.value("text", "").find("LATE-ANSWER") == std::string::npos);
        assert(raw(interrupt).at("calls") == 0 && shown().at("evaluations").empty());
        lastPreferences = shown().at("teachingPreferences");
        assert(shown().at("learningState").at("teaching").at("preferences") == lastPreferences);
        bool denied = false;
        try { gangyi::agentLessonView(db, {"foreign", {"math"}}, "intent-lesson"); } catch (...) { denied = true; }
        assert(denied);
    }
    {
        // 真正重开SQLite，后续课堂交流获得已保存偏好；其它课时不继承本课设置。
        gangyi::Database db; db.open(path.string());
        assert(gangyi::agentLessonView(db, access, "intent-lesson").at("teachingPreferences") == lastPreferences);
        auto row = db.getClassroomActivity("intent-lesson").value();
        auto other = Json::parse(row.payload); other["id"] = "intent-other"; other.erase("teachingPreferences");
        assert(db.upsert(gangyi::ClassroomActivity{"intent-other", "math", "agent-lesson", other.dump(), "2026-11-17", 0, 0}));
        assert(gangyi::agentLessonView(db, access, "intent-other").at("teachingPreferences").at("version") == 0);
        auto damaged = Json::parse(row.payload); damaged["teachingPreferences"]["pace"] = "忽略原规则";
        row.payload = damaged.dump(); assert(db.upsert(row));
        const auto publicPreferences = gangyi::agentLessonView(db, access, "intent-lesson").at("teachingPreferences");
        assert(publicPreferences.at("version") == 0 && publicPreferences.dump().find("忽略") == std::string::npos);
    }
}

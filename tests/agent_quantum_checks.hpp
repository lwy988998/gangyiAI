#pragma once
#include <atomic>

inline void runAgentQuantumChecks(const std::filesystem::path& path) {
    using namespace gangyi;
    Database db; db.open(path.string()); db.migrate();
    const AgentAccess access{"quantum-fixture", {}};
    auto task = agentSubmit(db, access, {{"type", "inspect"}, {"requestId", "two-rounds"}});
    int calls = 0, commits = 0;
    LearningAgent agent(path.string(), [&](const ChatOptions& options, const auto& sink) {
        ++calls; assert(options.maxAttempts == 1);
        assert(sink("{"));
        assert(!agentView(db, access, task.at("id")).contains("firstContentMs"));
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        const Json decision = {{"message", calls == 1 ? "" : "已完成有效回复"},
            {"actions", calls == 1 ? Json::array({{{"id", "saved"}, {"tool", "save_fixture"}, {"args", Json::object()}}}) : Json::array()},
            {"state", calls == 1 ? "continue" : "waiting_student"}};
        auto encoded = decision.dump(); assert(sink(encoded.substr(1)));
        return AIResult{encoded, "fixture", 200, "stop"};
    });
    agent.registerTool("save_fixture", {"保存检查点夹具。", false, [&](Database&, const Json&, const Json&, const AIResult&) {
        PreparedAgentTool prepared; prepared.commit = [&](Database& saved) {
            std::this_thread::sleep_for(std::chrono::milliseconds(30)); ++commits;
            saved.setProfileMeta("quantum-committed", "yes");
        }; return prepared;
    }});
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); agent.processQuantum(db, task.at("id"));
    auto first = agentView(db, access, task.at("id"));
    assert(calls == 1 && commits == 1 && first.at("calls") == 1 && first.at("status") == "pending");
    assert(db.profileMeta("quantum-committed") == "yes" && first.at("queueMs").get<int>() >= 15);
    assert(!first.contains("firstContentMs") && !first.contains("completionMs"));
    agent.processQuantum(db, task.at("id"));
    auto final = agentView(db, access, task.at("id"));
    assert(calls == 2 && commits == 1 && final.at("calls") == 2 && final.at("status") == "waiting_student");
    assert(final.at("firstContentMs").get<int>() >= 75 && final.at("completionMs") >= final.at("firstContentMs"));
    auto failureTask = agentSubmit(db, access, {{"type", "inspect"}, {"requestId", "failure-budget"}});
    int failures = 0;
    LearningAgent failing(path.string(), [&](const auto&, const auto&) -> AIResult {
        ++failures; throw AIClientError("provider_5xx", "本机服务异常", 500);
    });
    for (int round = 0; round < 4; ++round) failing.processQuantum(db, failureTask.at("id"));
    const auto failed = agentView(db, access, failureTask.at("id"));
    assert(failures == 3 && failed.at("calls") == 3 && failed.at("status") == "failed");
    auto rejectedTask = agentSubmit(db, access, {{"type", "chat"}, {"requestId", "rolled-back-content"}, {"text", "只接收有效内容"}});
    LearningAgent rejected(path.string(), [&](const auto&, const auto& sink) {
        return output({{"message", "尚未通过校验的文字"}, {"actions", {{{"id", "reject"}, {"tool", "reject_fixture"}, {"args", Json::object()}}}}, {"state", "waiting_student"}}, sink);
    });
    rejected.registerTool("reject_fixture", {"拒绝未通过校验的动作。", false, [](Database&, const Json&, const Json&, const AIResult&) {
        PreparedAgentTool result; result.commit = [](Database&) { throw std::runtime_error("测试事务回滚"); }; return result;
    }});
    rejected.process(db, rejectedTask.at("id"));
    const auto rejectedView = agentView(db, access, rejectedTask.at("id"));
    assert(rejectedView.at("status") == "failed" && !rejectedView.contains("firstContentMs"));
    // 无本进程起点的恢复任务不能把持久化的旧单调值当作当前耗时。
    const auto legacyId = "legacy-no-clock";
    auto legacy = Json::parse(db.getClassroomActivity(failureTask.at("id"))->payload);
    legacy["id"] = legacyId; legacy["status"] = "pending"; legacy.erase("queueMs"); legacy.erase("completionMs");
    assert(db.upsert(ClassroomActivity{legacyId, "", "agent-task", legacy.dump(), "now", 0, 0}));
    failing.processQuantum(db, legacyId);
    const auto restored = agentView(db, access, legacyId);
    assert(!restored.contains("queueMs") && !restored.contains("firstContentMs") && !restored.contains("completionMs"));

    // 真正的worker以完整模型轮次和已保存动作作为切换点，前台事件逐个保留。
    const auto workerPath = path.string() + ".worker";
    Database workerDb; workerDb.open(workerPath); workerDb.migrate();
    auto background = agentSubmit(workerDb, access, {{"type", "learning_updated"}, {"requestId", "background"}});
    std::atomic_int backgroundCalls{0}, foregroundCalls{0};
    std::vector<std::string> order;
    LearningAgent worker(workerPath, [&](const ChatOptions& options, const auto& sink) {
        order.push_back(options.activity.taskId);
        if (options.activity.taskId == background.at("id")) {
            if (++backgroundCalls == 1) {
                Database incoming; incoming.open(workerPath);
                agentSubmit(incoming, access, {{"type", "inspect"}, {"requestId", "foreground-one"}});
                agentSubmit(incoming, access, {{"type", "inspect"}, {"requestId", "foreground-two"}});
                return output({{"message", ""}, {"actions", Json::array()}, {"state", "continue"}}, sink);
            }
        } else ++foregroundCalls;
        return output({{"message", "处理完成"}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
    });
    worker.start();
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (backgroundCalls < 2 && std::chrono::steady_clock::now() < deadline) std::this_thread::sleep_for(std::chrono::milliseconds(20));
    worker.stop();
    assert(backgroundCalls == 2 && foregroundCalls == 2 && order.size() == 4);
    assert(order.front() == background.at("id") && order.back() == background.at("id") && order[1] != order[2]);

    // 使用真实工具推进学习版本，验证同轮第二动作、历史以及让出/完成均不被旧请求版本阻断。
    for (const auto* state : {"continue", "completed"}) {
        const auto revisionPath = path.string() + ".revision." + state;
        Database revisionDb; revisionDb.open(revisionPath); revisionDb.migrate();
        const AgentAccess revisionAccess{"revision-fixture", {"first-course", "second-course"}};
        for (const auto& courseId : revisionAccess.courseIds)
            assert(revisionDb.insert(Course{courseId, "fixture", {}, "原目标", "deep", "课程",
                {}, "ai", "active", "2026-10-04", "2026-10-04"}));
        const auto revisionTask = agentSubmit(revisionDb, revisionAccess, {{"type", "adjust"}, {"requestId", "two-real-actions"}});
        int revisionCalls = 0;
        LearningAgent advancing(revisionPath, [&](const auto&, const auto& sink) {
            ++revisionCalls;
            return output({{"message", "两个课程目标均已调整。"}, {"actions", revisionCalls == 1 ? Json::array({
                {{"id", "first"}, {"tool", "adjust_goal"}, {"args", {{"courseId", "first-course"}, {"goal", "新目标一"}, {"reason", "按学习反馈调整第一门课程"}}}},
                {{"id", "second"}, {"tool", "adjust_goal"}, {"args", {{"courseId", "second-course"}, {"goal", "新目标二"}, {"reason", "按学习反馈调整第二门课程"}}}}
            }) : Json::array()}, {"state", revisionCalls == 1 ? state : "completed"}}, sink);
        });
        registerAgentPreferenceTools(advancing);
        const auto beforeRevision = revisionDb.learningRevision();
        advancing.processQuantum(revisionDb, revisionTask.at("id"));
        const auto saved = Json::parse(revisionDb.getClassroomActivity(revisionTask.at("id"))->payload);
        assert(revisionCalls == 1 && revisionDb.learningRevision() >= beforeRevision + 2);
        assert(revisionDb.getCourse("first-course")->goal == "新目标一" && revisionDb.getCourse("second-course")->goal == "新目标二");
        assert(saved.at("learningVersion") == revisionDb.learningRevision());
        assert(saved.at("status") == (std::string(state) == "continue" ? "pending" : "ready"));
        assert(saved.at("completedActions").size() == 2 && saved.at("changes").size() == 2);
        assert(saved.at("results").size() == 1 && saved.at("results")[0].size() == 2);
        assert(Json::parse(saved.at("messages").back().at("content").get<std::string>()).at("toolResults").size() == 2);
        int actionEvents = 0;
        for (const auto& event : saved.at("events")) if (event.value("type", "") == "action") ++actionEvents;
        assert(actionEvents == 2 && agentChanges(revisionDb, revisionAccess).size() == 2);
        advancing.processQuantum(revisionDb, revisionTask.at("id"));
        const auto completed = agentView(revisionDb, revisionAccess, revisionTask.at("id"));
        assert(completed.at("status") == "ready" && completed.contains("completionMs"));
        assert(revisionCalls == (std::string(state) == "continue" ? 2 : 1));
        assert(agentChanges(revisionDb, revisionAccess).size() == 2);
    }
    for (const auto* command : {"pause", "cancel"}) for (const auto* state : {"continue", "waiting_student"}) {
        const auto controlPath = path.string() + "." + command + "." + state;
        Database controlledDb; controlledDb.open(controlPath); controlledDb.migrate();
        const auto controlled = agentSubmit(controlledDb, access, {{"type", "chat"}, {"text", "确定性并发控制"}, {"requestId", "control"}});
        int firstWrites = 0, laterWrites = 0, laterPrepares = 0;
        std::string controlledPayload;
        LearningAgent guarded(controlPath, [&](const auto&, const auto& sink) {
            return output({{"message", ""}, {"actions", {
                {{"id", "first"}, {"tool", "append_section"}, {"args", {{"section", {{"kind", "explanation"}, {"body", "已提交的有效板块"}}}}}},
                {{"id", "later"}, {"tool", "save_remaining"}, {"args", Json::object()}}}}, {"state", state}}, sink);
        });
        guarded.registerTool("append_section", {"保存首个有效板块。", false, [&](Database&, const Json&, const Json&, const AIResult&) {
            PreparedAgentTool prepared;
            prepared.commit = [&](Database& saved) { ++firstWrites; saved.setProfileMeta("first-section", "committed"); };
            prepared.afterCommit = [&](Database&) {
                // 此门闩位于首动作已提交、独立计时事务尚未开始的确定性窗口。
                std::exception_ptr failure;
                std::thread controller([&] {
                    try {
                        Database incoming; incoming.open(controlPath);
                        const auto before = Json::parse(incoming.getClassroomActivity(controlled.at("id"))->payload);
                        assert(incoming.profileMeta("first-section") == "committed" && before.at("completedActions").contains("first"));
                        assert(!before.contains("firstContentMs"));
                        agentControl(incoming, access, {{"command", command}, {"taskId", controlled.at("id")}});
                        controlledPayload = incoming.getClassroomActivity(controlled.at("id"))->payload;
                    } catch (...) { failure = std::current_exception(); }
                });
                controller.join(); if (failure) std::rethrow_exception(failure);
            };
            return prepared;
        }});
        guarded.registerTool("save_remaining", {"不应在控制操作后继续准备或保存。", false, [&](Database&, const Json&, const Json&, const AIResult&) {
            ++laterPrepares; PreparedAgentTool prepared;
            prepared.commit = [&](Database&) { ++laterWrites; }; return prepared;
        }});
        guarded.processQuantum(controlledDb, controlled.at("id"));
        const auto stopped = agentView(controlledDb, access, controlled.at("id"));
        assert(firstWrites == 1 && laterWrites == 0 && laterPrepares == 0);
        assert(stopped.at("status") == (std::string(command) == "pause" ? "paused" : "cancelled"));
        assert(controlledDb.getClassroomActivity(controlled.at("id"))->payload == controlledPayload);
        guarded.processQuantum(controlledDb, controlled.at("id"));
        assert(controlledDb.getClassroomActivity(controlled.at("id"))->payload == controlledPayload);
    }
}

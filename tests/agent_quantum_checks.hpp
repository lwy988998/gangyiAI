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
}

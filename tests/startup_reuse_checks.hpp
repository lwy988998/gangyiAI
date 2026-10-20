#pragma once

inline void runStartupReuseChecks(const std::filesystem::path& path) {
    using namespace gangyi;
    Database db; db.open(path.string()); db.migrate();
    const AgentAccess access{"startup-reuse", {"startup-math"}};
    assert(db.insert(Course{"startup-math", access.scopeId, {}, "分数", "deep", "分数学习", {}, "ai", "active", "now", "now"}));
    int calls = 0;
    LearningAgent agent(path.string(), [&](const auto& options, const auto& sink) {
        ++calls;
        assert(options.maxTokens == 8192 && options.temperature == 0.25);
        return output({{"message", "依据当前真实状态保存推荐。"}, {"state", "completed"},
            {"actions", {{{"id", "recommend"}, {"tool", "update_recommendations"},
                {"args", {{"lite", {"分数概念"}}, {"deep", {"分数运算"}}, {"reason", "依据当前学习记录"}}}}}}}, sink);
    });
    registerAgentPreferenceTools(agent);
    int sequence = 0;
    const auto event = [&] { return Json{{"type", "startup"}, {"requestId", "launch-" + std::to_string(++sequence)},
        {"text", "读取真实情况并自主决定推荐和下一步。"}}; };
    auto request = event();
    auto first = agentSubmit(db, access, request);
    assert(agentSubmit(db, access, request).at("id") == first.at("id"));
    // 尚未完成的任务不能被另一次启动当作成功结果。
    assert(agentSubmit(db, access, event()).at("id") != first.at("id"));
    agent.process(db, first.at("id"));
    assert(agentView(db, access, first.at("id")).at("status") == "ready" && calls == 1);
    const auto reusedRequest = event();
    auto reused = agentSubmit(db, access, reusedRequest);
    const auto originalTaskId = first.at("id");
    assert(reused.at("id") == first.at("id") && reused.at("reuseCount") == 1);
    assert(reused.at("reuseReason") == "unchanged_startup_state");
    agent.process(db, reused.at("id"));
    agentControl(db, access, {{"command", "retry"}, {"taskId", reused.at("id")}});
    assert(calls == 1);
    // 重新打开同一数据库仍复用，复用依据不依赖进程内内存。
    { Database reopened; reopened.open(path.string());
      assert(agentSubmit(reopened, access, event()).at("id") == first.at("id")); }
    const auto invalidated = [&](const auto& change) {
        change();
        const auto next = agentSubmit(db, access, event());
        assert(next.at("id") != first.at("id"));
        const auto before = calls; agent.process(db, next.at("id")); assert(calls == before + 1);
        assert(agentSubmit(db, access, event()).at("id") == next.at("id"));
        first = next;
    };
    invalidated([&] { assert(db.saveMyAIRecord({"preferences", "preferences", Json{{"instructions", "先举例"}}.dump(), "now", 0}, 0)); });
    assert(agentSubmit(db, access, reusedRequest).at("id") == originalTaskId);
    auto changedRequest = reusedRequest; changedRequest["text"] = "改了同一请求的内容";
    bool rejected = false;
    try { agentSubmit(db, access, changedRequest); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    invalidated([&] { assert(db.saveMyAIRecord({"memory", "memory", Json{{"content", "手动偏好"}, {"source", "manual"}}.dump(), "now", 0}, 0)); });
    invalidated([&] { assert(db.saveMyAIRecord({"memory", "memory", Json{{"deleted", true}}.dump(), "now", 0}, 1)); });
    invalidated([&] { db.setProfileMeta("library-revision", "1"); });
    invalidated([&] { db.setProfileMeta("ability-profile", Json{{"reason", "同版本画像正文已重评"}}.dump()); });
    const auto originalModel = std::getenv("AI_MODEL");
    const std::string savedModel = originalModel ? originalModel : "";
    invalidated([&] { efficiencySwitch("AI_MODEL", "different-fixture-model"); });
    invalidated([&] { efficiencySwitch("AI_MODEL", savedModel.c_str()); });
    invalidated([&] { assert(db.upsert(ClassroomActivity{"saved-lesson", "startup-math", "agent-lesson",
        Json{{"scopeId", access.scopeId}, {"version", 1}, {"status", "ready"}}.dump(), "now", 0, 0})); });
    invalidated([&] { assert(db.upsert(ClassroomActivity{"saved-lesson", "startup-math", "agent-lesson",
        Json{{"scopeId", access.scopeId}, {"version", 2}, {"status", "entered"}}.dump(), "now", 0, 0})); });
    invalidated([&] { assert(db.upsert(ClassroomActivity{"saved-review", "startup-math", "review",
        Json{{"scopeId", access.scopeId}, {"version", 1}, {"due", "2000-01-01"}, {"status", "pending"}}.dump(), "now", 0, 0})); });
    invalidated([&] { agentSubmit(db, access, {{"type", "prepare_next"}, {"courseId", "startup-math"}, {"requestId", "new-preparation"}}); });
    invalidated([&] { assert(db.insert(LearningInteraction{"new-answer", "chat-user", Json{{"scopeId", access.scopeId}, {"text", "2/5"}}.dump(),
        "now", "startup-math", "conversation", {}})); });
    auto fresh = event(); fresh["refresh"] = true;
    assert(agentSubmit(db, access, fresh).at("id") != first.at("id"));
    efficiencySwitch("GANGYI_AGENT_STARTUP_REUSE", "0");
    assert(agentSubmit(db, access, event()).at("id") != first.at("id"));
    efficiencySwitch("GANGYI_AGENT_STARTUP_REUSE", "");
    assert(agentSubmit(db, {"another-scope", access.courseIds}, event()).at("id") != first.at("id"));
    assert(agentView(db, access, first.at("id")).at("calls") == 1);
}

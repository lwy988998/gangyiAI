// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 定义函数 runStartupReuseChecks，执行该函数声明的职责。
inline void runStartupReuseChecks(const std::filesystem::path& path) {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 【逐行批注】 执行当前声明或表达式：Database db; db.open(path.string()); db.migrate();。
    Database db; db.open(path.string()); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：const AgentAccess access{"startup-reuse", {"startup-math"}};。
    const AgentAccess access{"startup-reuse", {"startup-math"}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(Course{"startup-math", access.scopeId, {}, "分数", "de…。
    assert(db.insert(Course{"startup-math", access.scopeId, {}, "分数", "deep", "分数学习", {}, "ai", "active", "now", "now"}));
    // 【逐行批注】 计算右侧结果并写入 int calls，更新当前运行状态。
    int calls = 0;
    // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
    LearningAgent agent(path.string(), [&](const auto& options, const auto& sink) {
        // 【逐行批注】 执行当前声明或表达式：++calls;。
        ++calls;
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.maxTokens == 8192 && options.temperature == 0.25);。
        assert(options.maxTokens == 8192 && options.temperature == 0.25);
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "依据当前真实状态保存推荐。"}, {"state", "completed"},。
        return output({{"message", "依据当前真实状态保存推荐。"}, {"state", "completed"},
            // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "recommend"}, {"tool", "update_recommendations"},。
            {"actions", {{{"id", "recommend"}, {"tool", "update_recommendations"},
                // 【逐行批注】 执行当前声明或表达式：{"args", {{"lite", {"分数概念"}}, {"deep", {"分数运算"}}, {"reason", "依据当前学习记录"}}}}}}}, sink…。
                {"args", {{"lite", {"分数概念"}}, {"deep", {"分数运算"}}, {"reason", "依据当前学习记录"}}}}}}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentPreferenceTools 完成本行所需操作：registerAgentPreferenceTools(agent);。
    registerAgentPreferenceTools(agent);
    // 【逐行批注】 计算右侧结果并写入 int sequence，更新当前运行状态。
    int sequence = 0;
    // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
    const auto event = [&] { return Json{{"type", "startup"}, {"requestId", "launch-" + std::to_string(++sequence)},
        // 【逐行批注】 执行当前声明或表达式：{"text", "读取真实情况并自主决定推荐和下一步。"}}; };。
        {"text", "读取真实情况并自主决定推荐和下一步。"}}; };
    // 【逐行批注】 计算右侧结果并写入 request，更新当前运行状态。
    auto request = event();
    // 【逐行批注】 计算右侧结果并写入 first，更新当前运行状态。
    auto first = agentSubmit(db, access, request);
    // 【逐行批注】 调用 assert、agentSubmit 完成本行所需操作：assert(agentSubmit(db, access, request).at("id") == first.at("id"));。
    assert(agentSubmit(db, access, request).at("id") == first.at("id"));
    // 尚未完成的任务不能被另一次启动当作成功结果。
    // 【逐行批注】 调用 assert、agentSubmit、event 完成本行所需操作：assert(agentSubmit(db, access, event()).at("id") != first.at("id"));。
    assert(agentSubmit(db, access, event()).at("id") != first.at("id"));
    // 【逐行批注】 执行当前声明或表达式：agent.process(db, first.at("id"));。
    agent.process(db, first.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, first.at("id")).at("status") == "ready" …。
    assert(agentView(db, access, first.at("id")).at("status") == "ready" && calls == 1);
    // 【逐行批注】 计算右侧结果并写入 auto reusedRequest，更新当前运行状态。
    const auto reusedRequest = event();
    // 【逐行批注】 计算右侧结果并写入 reused，更新当前运行状态。
    auto reused = agentSubmit(db, access, reusedRequest);
    // 【逐行批注】 计算右侧结果并写入 auto originalTaskId，更新当前运行状态。
    const auto originalTaskId = first.at("id");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(reused.at("id") == first.at("id") && reused.at("reuseCount") =…。
    assert(reused.at("id") == first.at("id") && reused.at("reuseCount") == 1);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(reused.at("reuseReason") == "unchanged_startup_state");。
    assert(reused.at("reuseReason") == "unchanged_startup_state");
    // 【逐行批注】 执行当前声明或表达式：agent.process(db, reused.at("id"));。
    agent.process(db, reused.at("id"));
    // 【逐行批注】 调用 agentControl 完成本行所需操作：agentControl(db, access, {{"command", "retry"}, {"taskId", reused.at(…。
    agentControl(db, access, {{"command", "retry"}, {"taskId", reused.at("id")}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(calls == 1);。
    assert(calls == 1);
    // 重新打开同一数据库仍复用，复用依据不依赖进程内内存。
    // 【逐行批注】 执行当前声明或表达式：{ Database reopened; reopened.open(path.string());。
    { Database reopened; reopened.open(path.string());
      // 【逐行批注】 调用 assert、agentSubmit、event 完成本行所需操作：assert(agentSubmit(reopened, access, event()).at("id") == first.at("i…。
      assert(agentSubmit(reopened, access, event()).at("id") == first.at("id")); }
    // 【逐行批注】 计算右侧结果并写入 auto invalidated，更新当前运行状态。
    const auto invalidated = [&](const auto& change) {
        // 【逐行批注】 调用 change 完成本行所需操作：change();。
        change();
        // 【逐行批注】 计算右侧结果并写入 auto next，更新当前运行状态。
        const auto next = agentSubmit(db, access, event());
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(next.at("id") != first.at("id"));。
        assert(next.at("id") != first.at("id"));
        // 【逐行批注】 计算右侧结果并写入 auto before，更新当前运行状态。
        const auto before = calls; agent.process(db, next.at("id")); assert(calls == before + 1);
        // 【逐行批注】 调用 assert、agentSubmit、event 完成本行所需操作：assert(agentSubmit(db, access, event()).at("id") == next.at("id"));。
        assert(agentSubmit(db, access, event()).at("id") == next.at("id"));
        // 【逐行批注】 计算右侧结果并写入 first，更新当前运行状态。
        first = next;
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 调用 invalidated、assert 完成本行所需操作：invalidated([&] { assert(db.saveMyAIRecord({"preferences", "preferenc…。
    invalidated([&] { assert(db.saveMyAIRecord({"preferences", "preferences", Json{{"instructions", "先举例"}}.dump(), "now", 0}, 0)); });
    // 【逐行批注】 调用 assert、agentSubmit 完成本行所需操作：assert(agentSubmit(db, access, reusedRequest).at("id") == originalTas…。
    assert(agentSubmit(db, access, reusedRequest).at("id") == originalTaskId);
    // 【逐行批注】 计算右侧结果并写入 changedRequest，更新当前运行状态。
    auto changedRequest = reusedRequest; changedRequest["text"] = "改了同一请求的内容";
    // 【逐行批注】 计算右侧结果并写入 bool rejected，更新当前运行状态。
    bool rejected = false;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { agentSubmit(db, access, changedRequest); } catch (const std::invalid_argument&) { rejected = true; }
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(rejected);。
    assert(rejected);
    // 【逐行批注】 调用 invalidated、assert 完成本行所需操作：invalidated([&] { assert(db.saveMyAIRecord({"memory", "memory", Json{…。
    invalidated([&] { assert(db.saveMyAIRecord({"memory", "memory", Json{{"content", "手动偏好"}, {"source", "manual"}}.dump(), "now", 0}, 0)); });
    // 【逐行批注】 调用 invalidated、assert 完成本行所需操作：invalidated([&] { assert(db.saveMyAIRecord({"memory", "memory", Json{…。
    invalidated([&] { assert(db.saveMyAIRecord({"memory", "memory", Json{{"deleted", true}}.dump(), "now", 0}, 1)); });
    // 【逐行批注】 调用 invalidated 完成本行所需操作：invalidated([&] { db.setProfileMeta("library-revision", "1"); });。
    invalidated([&] { db.setProfileMeta("library-revision", "1"); });
    // 【逐行批注】 调用 invalidated 完成本行所需操作：invalidated([&] { db.setProfileMeta("ability-profile", Json{{"reason"…。
    invalidated([&] { db.setProfileMeta("ability-profile", Json{{"reason", "同版本画像正文已重评"}}.dump()); });
    // 【逐行批注】 计算右侧结果并写入 TopicMastery topic; topic.courseId，更新当前运行状态。
    TopicMastery topic; topic.courseId = "startup-math"; topic.topic = "通分"; topic.score = 80; topic.status = "ready";
    // 【逐行批注】 计算右侧结果并写入 topic.version，更新当前运行状态。
    topic.version = db.profileRevision(); topic.nextReviewAt = "2099-01-01";
    // 【逐行批注】 调用 invalidated、assert 完成本行所需操作：invalidated([&] { assert(db.upsert(topic)); });。
    invalidated([&] { assert(db.upsert(topic)); });
    // 【逐行批注】 计算右侧结果并写入 auto learningRevision，更新当前运行状态。
    const auto learningRevision = db.learningRevision(), profileRevision = db.profileRevision();
    // 【逐行批注】 调用 invalidated 完成本行所需操作：invalidated([&] {。
    invalidated([&] {
        // 【逐行批注】 执行当前声明或表达式：Database other; other.open(path.string());。
        Database other; other.open(path.string());
        // 【逐行批注】 计算右侧结果并写入 topic.score.reset(); topic.status，更新当前运行状态。
        topic.score.reset(); topic.status = "insufficient"; topic.weakPoints = "[\"分数单位\"]";
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(other.upsert(topic));。
        assert(other.upsert(topic));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.learningRevision() == learningRevision && db.profileRevisio…。
        assert(db.learningRevision() == learningRevision && db.profileRevision() == profileRevision);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 计算右侧结果并写入 invalidated([&] { topic.nextReviewAt，更新当前运行状态。
    invalidated([&] { topic.nextReviewAt = "2000-01-01"; assert(db.upsert(topic)); });
    // 【逐行批注】 计算右侧结果并写入 SubjectMastery subject; subject.subject，更新当前运行状态。
    SubjectMastery subject; subject.subject = "数学"; subject.score = 80; subject.status = "ready";
    // 【逐行批注】 调用 invalidated、assert 完成本行所需操作：invalidated([&] { assert(db.upsert(subject)); });。
    invalidated([&] { assert(db.upsert(subject)); });
    // 【逐行批注】 计算右侧结果并写入 invalidated([&] { subject.weakPoints，更新当前运行状态。
    invalidated([&] { subject.weakPoints = "[\"单位理解\"]"; assert(db.upsert(subject)); });
    // 【逐行批注】 计算右侧结果并写入 auto originalModel，更新当前运行状态。
    const auto originalModel = std::getenv("AI_MODEL");
    // 【逐行批注】 计算右侧结果并写入 std::string savedModel，更新当前运行状态。
    const std::string savedModel = originalModel ? originalModel : "";
    // 【逐行批注】 调用 invalidated、efficiencySwitch 完成本行所需操作：invalidated([&] { efficiencySwitch("AI_MODEL", "different-fixture-mod…。
    invalidated([&] { efficiencySwitch("AI_MODEL", "different-fixture-model"); });
    // 【逐行批注】 调用 invalidated、efficiencySwitch 完成本行所需操作：invalidated([&] { efficiencySwitch("AI_MODEL", savedModel.c_str()); }…。
    invalidated([&] { efficiencySwitch("AI_MODEL", savedModel.c_str()); });
    // 【逐行批注】 调用 invalidated、assert 完成本行所需操作：invalidated([&] { assert(db.upsert(ClassroomActivity{"saved-lesson", …。
    invalidated([&] { assert(db.upsert(ClassroomActivity{"saved-lesson", "startup-math", "agent-lesson",
        // 【逐行批注】 执行当前声明或表达式：Json{{"scopeId", access.scopeId}, {"version", 1}, {"status", "ready"}}.dump(), "now"…。
        Json{{"scopeId", access.scopeId}, {"version", 1}, {"status", "ready"}}.dump(), "now", 0, 0})); });
    // 【逐行批注】 调用 invalidated、assert 完成本行所需操作：invalidated([&] { assert(db.upsert(ClassroomActivity{"saved-lesson", …。
    invalidated([&] { assert(db.upsert(ClassroomActivity{"saved-lesson", "startup-math", "agent-lesson",
        // 【逐行批注】 执行当前声明或表达式：Json{{"scopeId", access.scopeId}, {"version", 2}, {"status", "entered"}}.dump(), "no…。
        Json{{"scopeId", access.scopeId}, {"version", 2}, {"status", "entered"}}.dump(), "now", 0, 0})); });
    // 【逐行批注】 调用 invalidated、assert 完成本行所需操作：invalidated([&] { assert(db.upsert(ClassroomActivity{"saved-review", …。
    invalidated([&] { assert(db.upsert(ClassroomActivity{"saved-review", "startup-math", "review",
        // 【逐行批注】 执行当前声明或表达式：Json{{"scopeId", access.scopeId}, {"version", 1}, {"due", "2000-01-01"}, {"status", …。
        Json{{"scopeId", access.scopeId}, {"version", 1}, {"due", "2000-01-01"}, {"status", "pending"}}.dump(), "now", 0, 0})); });
    // 【逐行批注】 调用 invalidated、agentSubmit 完成本行所需操作：invalidated([&] { agentSubmit(db, access, {{"type", "prepare_next"}, …。
    invalidated([&] { agentSubmit(db, access, {{"type", "prepare_next"}, {"courseId", "startup-math"}, {"requestId", "new-preparation"}}); });
    // 【逐行批注】 调用 invalidated、assert 完成本行所需操作：invalidated([&] { assert(db.insert(LearningInteraction{"new-answer", …。
    invalidated([&] { assert(db.insert(LearningInteraction{"new-answer", "chat-user", Json{{"scopeId", access.scopeId}, {"text", "2/5"}}.dump(),
        // 【逐行批注】 执行当前声明或表达式："now", "startup-math", "conversation", {}})); });。
        "now", "startup-math", "conversation", {}})); });
    // 【逐行批注】 计算右侧结果并写入 fresh，更新当前运行状态。
    auto fresh = event(); fresh["refresh"] = true;
    // 【逐行批注】 调用 assert、agentSubmit 完成本行所需操作：assert(agentSubmit(db, access, fresh).at("id") != first.at("id"));。
    assert(agentSubmit(db, access, fresh).at("id") != first.at("id"));
    // 【逐行批注】 调用 efficiencySwitch 完成本行所需操作：efficiencySwitch("GANGYI_AGENT_STARTUP_REUSE", "0");。
    efficiencySwitch("GANGYI_AGENT_STARTUP_REUSE", "0");
    // 【逐行批注】 调用 assert、agentSubmit、event 完成本行所需操作：assert(agentSubmit(db, access, event()).at("id") != first.at("id"));。
    assert(agentSubmit(db, access, event()).at("id") != first.at("id"));
    // 【逐行批注】 调用 efficiencySwitch 完成本行所需操作：efficiencySwitch("GANGYI_AGENT_STARTUP_REUSE", "");。
    efficiencySwitch("GANGYI_AGENT_STARTUP_REUSE", "");
    // 【逐行批注】 调用 assert、agentSubmit、event 完成本行所需操作：assert(agentSubmit(db, {"another-scope", access.courseIds}, event()).…。
    assert(agentSubmit(db, {"another-scope", access.courseIds}, event()).at("id") != first.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, first.at("id")).at("calls") == 1);。
    assert(agentView(db, access, first.at("id")).at("calls") == 1);
// 【逐行批注】 结束当前代码块。
}

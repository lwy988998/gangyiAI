#pragma once

// 使用真实主控与临时数据库验证先计算、暂停恢复、幂等回执与后发布评价。
// 模型回复为固定夹具，不能据此宣称真实模型会正确建模或诊断。
inline void runExactCalculationAgentChecks(const std::filesystem::path& path) {
    gangyi::Database db;
    db.open(path.string()); db.migrate();
    const gangyi::AgentAccess access{"calculation-check", {"math"}};
    assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "分数加法", "deep", "数学", {}, "ai", "active", "2026-11-16", "2026-11-16"}));
    assert(db.insert(gangyi::CourseSnapshot{"calculation-outline", "math", 1,
        Json{{"subject", "数学"}}.dump(), "2026-11-17"}));
    const Json lesson = {{"id", "calculation-lesson"}, {"scopeId", access.scopeId}, {"courseId", "math"},
        {"title", "分数"}, {"purpose", "核对分数加法"}, {"status", "ready"}, {"entered", true},
        {"version", 1}, {"updatedAt", "2026-11-16"}, {"sourceLearningVersion", db.learningRevision()},
        {"sections", {{{"id", "q1"}, {"kind", "question"}, {"title", "计算并说明"}, {"questionKind", "practice"},
            {"questionId", "calculation-q1"}, {"version", 1},
            {"question", {{"question", "1/2+1/3=?"}, {"expectedAnswer", "PRIVATE-CALCULATION-STANDARD"}, {"rubric", "PRIVATE-CALCULATION-RUBRIC"}}}}}}};
    assert(db.upsert(gangyi::ClassroomActivity{"calculation-lesson", "math", "agent-lesson", lesson.dump(), "2026-11-16", 0, 0}));
    const auto submit = [&](const std::string& requestId) {
        return gangyi::agentSubmit(db, access, {{"type", "question_answer"}, {"courseId", "math"}, {"lessonId", "calculation-lesson"},
            {"sectionId", "q1"}, {"sectionVersion", 1}, {"view", "practice"}, {"text", "分母相加，所以2/5"}, {"requestId", requestId}});
    };
    const auto rawTask = [&](const Json& task) { return Json::parse(db.getClassroomActivity(task.at("id"))->payload); };
    const auto inputFor = [&](const Json& task) { return gangyi::agentContext(db, access, rawTask(task).at("event")).at("currentInputs")[0]; };
    const auto task = submit("calculate-before-diagnosis");
    const auto input = inputFor(task);
    const Json expression = {{"op", "add"}, {"left", {{"numerator", 1}, {"denominator", 2}}},
                            {"right", {{"numerator", 1}, {"denominator", 3}}}};
    const Json calculation = {{"id", "check-fraction"}, {"tool", "calculate_exact"},
                             {"args", {{"interactionId", input.at("interactionId")}, {"expression", expression}}}};
    int calls = 0;
    const auto model = [&](const gangyi::ChatOptions& options, const auto& sink) -> gangyi::AIResult {
        ++calls;
        // 检查真正送进主控的阶段规则，而不是仅测试策略选择函数。
        std::string policies;
        for (const auto& message : options.messages) if (message.role == "system") policies += message.content;
        assert(policies.find("[subjects/mathematics]") != std::string::npos);
        assert(policies.find("[subjects/history]") == std::string::npos);
        assert(policies.find("[components/component-generator]") == std::string::npos);
        if (calls <= 3) {
            assert(policies.find("[teaching/answer-diagnosis]") != std::string::npos);
            assert(policies.find("[teaching/concept-explainer]") == std::string::npos);
        } else {
            assert(policies.find("[teaching/answer-diagnosis]") == std::string::npos);
            assert(policies.find("[teaching/feedback-generator]") != std::string::npos);
        }
        if (calls == 1) {
            assert(options.messages[0].content.find("calculate_exact") != std::string::npos);
            return output({{"message", ""}, {"actions", {calculation}}, {"state", "continue"}}, sink);
        }
        // 暂停后重建主控也能读取相同有效回执，不生成假的计算成功记录。
        assert(rawTask(task).at("completedActions").at("check-fraction").at("result").at("exact") == "5/6");
        if (calls == 2) return output({{"message", ""}, {"actions", {calculation}}, {"state", "continue"}}, sink);
        if (calls == 3) {
            bool receiptInPrompt = false;
            for (const auto& message : options.messages) {
                const auto value = Json::parse(message.content, nullptr, false);
                if (!value.is_object()) continue;
                for (const auto& row : value.value("toolResults", Json::array()))
                    if (row.value("result", Json::object()).value("exact", "") == "5/6") receiptInPrompt = true;
            }
            assert(receiptInPrompt);
            Json args = {{"interactionId", input.at("interactionId")}, {"score", 20}, {"credible", true},
                {"feedback", "你进行了加法尝试，但需要先统一分数单位。"}, {"feedbackHelpType", "explanation"},
                {"reason", "实际回答把分母相加"}, {"uncertainty", "只看到本题回答，不能推断总体掌握"},
                {"diagnosis", {{"summary", "把分母直接相加"}, {"evidence", {"分母相加"}},
                               {"teachingDirection", "用相同分数单位重新比较"}, {"needsClarification", false}}}};
            return output({{"message", ""}, {"actions", {{{"id", "grade-fraction"}, {"tool", "evaluate_answer"}, {"args", args}}}}, {"state", "continue"}}, sink);
        }
        assert(calls == 4);
        return output({{"message", "先比较每份的单位，再试着把两个分数换成相同的分母。"},
                       {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
    };
    {
        gangyi::LearningAgent agent(path.string(), model);
        gangyi::registerAgentLessonTools(agent);
        agent.processQuantum(db, task.at("id"));
    }
    auto saved = rawTask(task);
    assert(saved.at("status") == "pending" && saved.at("completedActions").size() == 1);
    const auto firstPolicy = saved.at("lastTeachingPolicy");
    assert(!firstPolicy.empty() && saved.at("lastPromptRevision") == gangyi::kAgentPromptRevision);
    const auto receipt = saved.at("completedActions").at("check-fraction").at("result");
    assert(receipt.at("binding").at("interactionId") == input.at("interactionId"));
    assert(receipt.at("binding").at("questionId") == "calculation-q1");
    assert(gangyi::agentLessonView(db, access, "calculation-lesson").at("evaluations").empty());
    gangyi::agentControl(db, access, {{"command", "pause"}, {"taskId", task.at("id")}});
    assert(rawTask(task).at("status") == "paused");
    gangyi::agentControl(db, access, {{"command", "resume"}, {"taskId", task.at("id")}});
    gangyi::LearningAgent resumed(path.string(), model);
    gangyi::registerAgentLessonTools(resumed);
    resumed.processQuantum(db, task.at("id"));
    assert(rawTask(task).at("completedActions").size() == 1 && calls == 2);
    assert(rawTask(task).at("completedActions").at("check-fraction").at("result") == receipt);
    assert(rawTask(task).at("lastTeachingPolicy") == firstPolicy);
    resumed.processQuantum(db, task.at("id"));
    assert(calls == 3 && gangyi::agentLessonView(db, access, "calculation-lesson").at("evaluations").empty());
    resumed.processQuantum(db, task.at("id"));
    assert(calls == 4 && gangyi::agentView(db, access, task.at("id")).at("status") == "waiting_student");
    const auto shown = gangyi::agentLessonView(db, access, "calculation-lesson");
    assert(shown.at("evaluations").size() == 1);
    assert(shown.at("learningState").at("mastery").at("status") == "UNKNOWN");
    assert(shown.dump().find("PRIVATE-CALCULATION") == std::string::npos);
    assert(shown.dump().find("exact-calculation-v1") == std::string::npos);

    // 计算缺失/跨输入身份、提前讲解、同轮评分和非法算式全部失败且不形成新评价。
    for (int kind = 0; kind < 5; ++kind) {
        const auto deniedTask = submit("invalid-calculation-" + std::to_string(kind));
        const auto actual = inputFor(deniedTask);
        int attempts = 0;
        gangyi::LearningAgent denied(path.string(), [&](const auto&, const auto& sink) {
            ++attempts;
            auto action = calculation;
            action["args"]["interactionId"] = actual.at("interactionId");
            if (kind == 0) action["args"].erase("interactionId");
            if (kind == 1) action["args"]["interactionId"] = input.at("interactionId");
            if (kind == 4) action["args"]["expression"] = {{"op", "divide"}, {"left", 1}, {"right", 0}};
            Json actions = Json::array({action});
            if (kind == 3) actions.push_back({{"id", "premature-grade"}, {"tool", "evaluate_answer"}, {"args", Json::object()}});
            return output({{"message", kind == 2 ? "已核验，结果是5/6" : ""}, {"actions", actions}, {"state", "continue"}}, sink);
        });
        gangyi::registerAgentLessonTools(denied);
        denied.process(db, deniedTask.at("id"));
        assert(gangyi::agentView(db, access, deniedTask.at("id")).at("status") == "failed" && attempts <= 4);
        assert(rawTask(deniedTask).at("completedActions").empty());
        assert(gangyi::agentLessonView(db, access, "calculation-lesson").at("evaluations").size() == 1);
    }
}

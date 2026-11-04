// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 回放真实故障的两类括号错误；未经完整校验的文字和动作不得公开或保存。
// 【逐行批注】 定义函数 runTeachingProtocolChecks，执行该函数声明的职责。
inline void runTeachingProtocolChecks(const std::filesystem::path& path) {
    // 【逐行批注】 执行当前声明或表达式：const std::vector<std::string> malformed = {。
    const std::vector<std::string> malformed = {
        // 【逐行批注】 执行当前声明或表达式：R"({"teaching":{"helpType":"explanation"},"message":"INVALID-PUBLIC","actions":[{"id…。
        R"({"teaching":{"helpType":"explanation"},"message":"INVALID-PUBLIC","actions":[{"id":"bad-section","tool":"append_section","args":{"lessonId":"protocol-lesson","section":{"kind":"explanation","title":"不应应用","body":"INVALID-PUBLIC","helpType":"explanation"}}],"state":"waiting_student"})",
        // 【逐行批注】 执行当前声明或表达式：R"({"teaching":{"helpType":"explanation"},"message":"INVALID-PUBLIC",{"actions":[],"…。
        R"({"teaching":{"helpType":"explanation"},"message":"INVALID-PUBLIC",{"actions":[],"state":"waiting_student"}})"
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 按循环规则重复处理：for (std::size_t index = 0; index < malformed.size(); ++index) {。
    for (std::size_t index = 0; index < malformed.size(); ++index) {
        // 【逐行批注】 计算右侧结果并写入 auto database，更新当前运行状态。
        const auto database = path.string() + "." + std::to_string(index);
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(database); db.migrate();。
        gangyi::Database db; db.open(database); db.migrate();
        // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess access{"protocol-check", {"protocol-math"}};。
        const gangyi::AgentAccess access{"protocol-check", {"protocol-math"}};
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::Course{"protocol-math", access.scopeId, {}, …。
        assert(db.insert(gangyi::Course{"protocol-math", access.scopeId, {}, "分数", "deep", "分数", {}, "ai", "active", "now", "now"}));
        // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
        const Json lesson = {{"id", "protocol-lesson"}, {"scopeId", access.scopeId}, {"courseId", "protocol-math"},
            // 【逐行批注】 执行当前声明或表达式：{"title", "分数"}, {"purpose", "校验错误恢复"}, {"status", "ready"}, {"entered", true}, {"ve…。
            {"title", "分数"}, {"purpose", "校验错误恢复"}, {"status", "ready"}, {"entered", true}, {"version", 1},
            // 【逐行批注】 执行当前声明或表达式：{"updatedAt", "now"}, {"sourceLearningVersion", db.learningRevision()}, {"sections",…。
            {"updatedAt", "now"}, {"sourceLearningVersion", db.learningRevision()}, {"sections", {
                // 【逐行批注】 执行当前声明或表达式：{{"id", "q1"}, {"kind", "question"}, {"title", "实际作答"}, {"questionKind", "practice"},。
                {{"id", "q1"}, {"kind", "question"}, {"title", "实际作答"}, {"questionKind", "practice"},
                    // 【逐行批注】 计算右侧结果并写入 {"questionId", "protocol-q1"}, {"version", 1…，更新当前运行状态。
                    {"questionId", "protocol-q1"}, {"version", 1}, {"question", {{"question", "1/2+1/3=?"},
                    // 【逐行批注】 执行当前声明或表达式：{"expectedAnswer", "PRIVATE-5/6"}, {"rubric", "PRIVATE-STANDARD"}}}}。
                    {"expectedAnswer", "PRIVATE-5/6"}, {"rubric", "PRIVATE-STANDARD"}}}}
            // 【逐行批注】 执行当前声明或表达式：}}};。
            }}};
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(gangyi::ClassroomActivity{"protocol-lesson", "protoc…。
        assert(db.upsert(gangyi::ClassroomActivity{"protocol-lesson", "protocol-math", "agent-lesson", lesson.dump(), "now", 0, 0}));
        // 【逐行批注】 计算右侧结果并写入 Json event，更新当前运行状态。
        const Json event = {{"type", "question_answer"}, {"courseId", "protocol-math"}, {"lessonId", "protocol-lesson"},
            // 【逐行批注】 执行当前声明或表达式：{"sectionId", "q1"}, {"sectionVersion", 1}, {"view", "practice"}, {"requestId", "pro…。
            {"sectionId", "q1"}, {"sectionVersion", 1}, {"view", "practice"}, {"requestId", "protocol-answer"}, {"text", "2/5"}};
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = gangyi::agentSubmit(db, access, event);
        // 【逐行批注】 计算右侧结果并写入 auto actual，更新当前运行状态。
        const auto actual = gangyi::agentContext(db, access, event).at("currentInputs").at(0);
        // 【逐行批注】 计算右侧结果并写入 int assessmentCalls，更新当前运行状态。
        int assessmentCalls = 0, teachingCalls = 0;
        // 【逐行批注】 计算右侧结果并写入 bool recover，更新当前运行状态。
        bool recover = false;
        // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
        gangyi::LearningAgent agent(database, [&](const gangyi::ChatOptions& options, const auto& sink) {
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.maxTokens == 8192 && options.temperature == 0.25 && op…。
            assert(options.maxTokens == 8192 && options.temperature == 0.25 && options.maxAttempts == 1);
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.responseFormat == "json_object");。
            assert(options.responseFormat == "json_object");
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.messages.front().content.find(gangyi::kAgentTeachingAc…。
            assert(options.messages.front().content.find(gangyi::kAgentTeachingAccuracyRule) != std::string::npos);
            // 【逐行批注】 判断条件 (options.activity.purpose == "判断本次回答或追问") {，成立时执行对应分支。
            if (options.activity.purpose == "判断本次回答或追问") {
                // 【逐行批注】 调用 assert 完成本行所需操作：++assessmentCalls; assert(assessmentCalls == 1);。
                ++assessmentCalls; assert(assessmentCalls == 1);
                // 【逐行批注】 结束当前函数并返回结果：return output({{"message", ""}, {"teaching", {{"helpType", "none"}}},…。
                return output({{"message", ""}, {"teaching", {{"helpType", "none"}}}, {"state", "continue"},
                    // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "original-diagnosis"}, {"tool", "evaluate_answer"}, {"args", {。
                    {"actions", {{{"id", "original-diagnosis"}, {"tool", "evaluate_answer"}, {"args", {
                        // 【逐行批注】 执行当前声明或表达式：{"interactionId", actual.at("interactionId")}, {"score", 20}, {"credible", true},。
                        {"interactionId", actual.at("interactionId")}, {"score", 20}, {"credible", true},
                        // 【逐行批注】 执行当前声明或表达式：{"feedback", "分母表示整体等分的份数，需要统一分数单位。"}, {"feedbackHelpType", "explanation"},。
                        {"feedback", "分母表示整体等分的份数，需要统一分数单位。"}, {"feedbackHelpType", "explanation"},
                        // 【逐行批注】 执行当前声明或表达式：{"reason", "真实回答把分子分母分别相加"}, {"uncertainty", "只有一次回答"},。
                        {"reason", "真实回答把分子分母分别相加"}, {"uncertainty", "只有一次回答"},
                        // 【逐行批注】 执行当前声明或表达式：{"diagnosis", {{"summary", "混淆分数单位"}, {"evidence", {"2/5"}},。
                        {"diagnosis", {{"summary", "混淆分数单位"}, {"evidence", {"2/5"}},
                            // 【逐行批注】 执行当前声明或表达式：{"teachingDirection", "解释同一整体的等分份数与每份大小"}, {"needsClarification", false}}}。
                            {"teachingDirection", "解释同一整体的等分份数与每份大小"}, {"needsClarification", false}}}
                    // 【逐行批注】 执行当前声明或表达式：}}}}}}, sink);。
                    }}}}}}, sink);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 执行当前声明或表达式：++teachingCalls;。
            ++teachingCalls;
            // 【逐行批注】 判断条件 (!recover) {，成立时执行对应分支。
            if (!recover) {
                // 【逐行批注】 计算右侧结果并写入 auto& content，更新当前运行状态。
                const auto& content = malformed[index];
                // 【逐行批注】 按循环规则重复处理：for (char byte : content) sink(std::string(1, byte));。
                for (char byte : content) sink(std::string(1, byte));
                // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
                gangyi::AIResult result; result.content = content; result.model = "fixture"; result.finishReason = "stop";
                // 【逐行批注】 结束当前函数并返回结果：return result;。
                return result;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "分母表示把同一整体平均分成的份数；先统一分数单位，再解释你的方法。"},。
            return output({{"message", "分母表示把同一整体平均分成的份数；先统一分数单位，再解释你的方法。"},
                // 【逐行批注】 调用 array 完成本行所需操作：{"teaching", {{"helpType", "explanation"}, {"demonstratedQuestions", …。
                {"teaching", {{"helpType", "explanation"}, {"demonstratedQuestions", Json::array()}}},
                // 【逐行批注】 调用 array 完成本行所需操作：{"actions", Json::array()}, {"state", "waiting_student"}}, sink);。
                {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(agent); agent.process(db, task.at("i…。
        gangyi::registerAgentLessonTools(agent); agent.process(db, task.at("id"));
        // 【逐行批注】 计算右侧结果并写入 raw，更新当前运行状态。
        auto raw = Json::parse(db.getClassroomActivity(task.at("id"))->payload);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(raw.at("status") == "failed" && assessmentCalls == 1 && teachi…。
        assert(raw.at("status") == "failed" && assessmentCalls == 1 && teachingCalls == 3);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(raw.at("completedActions").size() == 1 && raw.at("completedAct…。
        assert(raw.at("completedActions").size() == 1 && raw.at("completedActions").contains("original-diagnosis"));
        // 【逐行批注】 计算右侧结果并写入 auto diagnostic，更新当前运行状态。
        const auto diagnostic = raw.at("failure").at("responseDiagnostic");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(diagnostic.at("matchesStream") == true && diagnostic.at("parse…。
        assert(diagnostic.at("matchesStream") == true && diagnostic.at("parseByte").get<std::size_t>() > 0);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(diagnostic.at("receivedBytes") == malformed[index].size() && d…。
        assert(diagnostic.at("receivedBytes") == malformed[index].size() && diagnostic.at("streamBytes") == malformed[index].size());
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : raw.at("events")) if (item.value("type", "") == "d…。
        for (const auto& item : raw.at("events")) if (item.value("type", "") == "delta")
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(item.value("text", "").find("INVALID-PUBLIC") == std::string::…。
            assert(item.value("text", "").find("INVALID-PUBLIC") == std::string::npos);
        // 【逐行批注】 计算右侧结果并写入 shown，更新当前运行状态。
        auto shown = gangyi::agentLessonView(db, access, "protocol-lesson");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.at("sections").size() == 1 && shown.at("evaluations").em…。
        assert(shown.at("sections").size() == 1 && shown.at("evaluations").empty());
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.dump().find("INVALID-PUBLIC") == std::string::npos && sh…。
        assert(shown.dump().find("INVALID-PUBLIC") == std::string::npos && shown.dump().find("PRIVATE-") == std::string::npos);
        // 【逐行批注】 计算右侧结果并写入 recover，更新当前运行状态。
        recover = true;
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", ta…。
        gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", task.at("id")}});
        // 【逐行批注】 执行当前声明或表达式：agent.process(db, task.at("id"));。
        agent.process(db, task.at("id"));
        // 【逐行批注】 计算右侧结果并写入 shown，更新当前运行状态。
        shown = gangyi::agentLessonView(db, access, "protocol-lesson");
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, task.at("id")).at("status") == "…。
        assert(gangyi::agentView(db, access, task.at("id")).at("status") == "waiting_student");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(assessmentCalls == 1 && teachingCalls == 4 && shown.at("evalua…。
        assert(assessmentCalls == 1 && teachingCalls == 4 && shown.at("evaluations").size() == 1);
        // 【逐行批注】 计算右侧结果并写入 int answers，更新当前运行状态。
        int answers = 0, assessments = 0;
        // 【逐行批注】 按循环规则重复处理：for (const auto& interaction : db.listInteractions()) {。
        for (const auto& interaction : db.listInteractions()) {
            // 【逐行批注】 判断条件 (interaction.kind == "chat-user") ++answers;，成立时执行对应分支。
            if (interaction.kind == "chat-user") ++answers;
            // 【逐行批注】 判断条件 (interaction.kind == "practice") ++assessments;，成立时执行对应分支。
            if (interaction.kind == "practice") ++assessments;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.at("evaluations").at(0).at("score") == 20 && answers == …。
        assert(shown.at("evaluations").at(0).at("score") == 20 && answers == 1 && assessments == 1);
        // 【逐行批注】 执行当前声明或表达式：agent.process(db, task.at("id"));。
        agent.process(db, task.at("id"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(assessmentCalls == 1 && teachingCalls == 4);。
        assert(assessmentCalls == 1 && teachingCalls == 4);
    // 【逐行批注】 结束当前代码块。
    }

    // 结构错误后的自动恢复必须丢弃上一轮坏 JSON，而不是继续把它带进上下文。
    // 【逐行批注】 计算右侧结果并写入 auto autoRecoveryDatabase，更新当前运行状态。
    const auto autoRecoveryDatabase = path.string() + ".auto-recovery";
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database autoRecoveryDb; autoRecoveryDb.open(autoRecoveryDatabase); autoReco…。
    gangyi::Database autoRecoveryDb; autoRecoveryDb.open(autoRecoveryDatabase); autoRecoveryDb.migrate();
    // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess autoRecoveryAccess{"protocol-auto-recovery", {"protocol-re…。
    const gangyi::AgentAccess autoRecoveryAccess{"protocol-auto-recovery", {"protocol-recovery"}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(autoRecoveryDb.insert(gangyi::Course{"protocol-recovery", auto…。
    assert(autoRecoveryDb.insert(gangyi::Course{"protocol-recovery", autoRecoveryAccess.scopeId, {}, "恢复", "deep", "恢复", {}, "ai", "active", "now", "now"}));
    // 【逐行批注】 计算右侧结果并写入 auto autoRecoveryTask，更新当前运行状态。
    const auto autoRecoveryTask = gangyi::agentSubmit(autoRecoveryDb, autoRecoveryAccess,
        // 【逐行批注】 执行当前声明或表达式：{{"type", "chat"}, {"courseId", "protocol-recovery"}, {"requestId", "auto-recovery"}…。
        {{"type", "chat"}, {"courseId", "protocol-recovery"}, {"requestId", "auto-recovery"}, {"text", "继续准备"}});
    // 【逐行批注】 计算右侧结果并写入 int recoveryCalls，更新当前运行状态。
    int recoveryCalls = 0;
    // 【逐行批注】 计算右侧结果并写入 bool carriedInvalidStructure，更新当前运行状态。
    bool carriedInvalidStructure = false;
    // 【逐行批注】 定义函数 autoRecoveryAgent，执行该函数声明的职责。
    gangyi::LearningAgent autoRecoveryAgent(autoRecoveryDatabase, [&](const gangyi::ChatOptions& options, const auto& sink) {
        // 【逐行批注】 执行当前声明或表达式：++recoveryCalls;。
        ++recoveryCalls;
        // 【逐行批注】 判断条件 (recoveryCalls > 1)，成立时执行对应分支。
        if (recoveryCalls > 1)
            // 【逐行批注】 按循环规则重复处理：for (const auto& message : options.messages)。
            for (const auto& message : options.messages)
                // 【逐行批注】 计算右侧结果并写入 carriedInvalidStructure，更新当前运行状态。
                carriedInvalidStructure = carriedInvalidStructure || message.content.find("INVALID-AUTO-RECOVERY") != std::string::npos;
        // 【逐行批注】 判断条件 (recoveryCalls == 1) {，成立时执行对应分支。
        if (recoveryCalls == 1) {
            // 【逐行批注】 计算右侧结果并写入 std::string invalid，更新当前运行状态。
            const std::string invalid = R"({"message":"INVALID-AUTO-RECOVERY","actions":[)";
            // 【逐行批注】 调用 sink 完成本行所需操作：sink(invalid);。
            sink(invalid);
            // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
            gangyi::AIResult result; result.content = invalid; result.model = "fixture"; result.finishReason = "stop";
            // 【逐行批注】 结束当前函数并返回结果：return result;。
            return result;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "已自动恢复。"}, {"teaching", {{"helpType", "non…。
        return output({{"message", "已自动恢复。"}, {"teaching", {{"helpType", "none"}, {"demonstratedQuestions", Json::array()}}},
            // 【逐行批注】 调用 array 完成本行所需操作：{"actions", Json::array()}, {"state", "waiting_student"}}, sink);。
            {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 执行当前声明或表达式：autoRecoveryAgent.process(autoRecoveryDb, autoRecoveryTask.at("id"));。
    autoRecoveryAgent.process(autoRecoveryDb, autoRecoveryTask.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto recoveredTask，更新当前运行状态。
    const auto recoveredTask = Json::parse(autoRecoveryDb.getClassroomActivity(autoRecoveryTask.at("id"))->payload);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(recoveryCalls == 2 && !carriedInvalidStructure);。
    assert(recoveryCalls == 2 && !carriedInvalidStructure);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(recoveredTask.at("status") == "waiting_student" && !recoveredT…。
    assert(recoveredTask.at("status") == "waiting_student" && !recoveredTask.contains("failure") && !recoveredTask.contains("failureCount"));

    // JSON 解析器的异常摘录可能截断汉字；记录错误时不得再次抛出并遗留运行中任务。
    // 【逐行批注】 计算右侧结果并写入 auto database，更新当前运行状态。
    const auto database = path.string() + ".error-text";
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(database); db.migrate();。
    gangyi::Database db; db.open(database); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess access{"protocol-error", {"error-math"}};。
    const gangyi::AgentAccess access{"protocol-error", {"error-math"}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::Course{"error-math", access.scopeId, {}, "分数…。
    assert(db.insert(gangyi::Course{"error-math", access.scopeId, {}, "分数", "deep", "分数", {}, "ai", "active", "now", "now"}));
    // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
    const auto task = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"courseId", "error-math"},
        // 【逐行批注】 执行当前声明或表达式：{"requestId", "invalid-error-text"}, {"text", "保留学生原始输入"}});。
        {"requestId", "invalid-error-text"}, {"text", "保留学生原始输入"}});
    // 【逐行批注】 计算右侧结果并写入 int calls，更新当前运行状态。
    int calls = 0;
    // 【逐行批注】 声明对象 agent，并通过构造函数初始化其状态。
    gangyi::LearningAgent agent(database, [&](const auto&, const auto&) -> gangyi::AIResult {
        // 【逐行批注】 执行当前声明或表达式：++calls;。
        ++calls;
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error(std::string("invalid excerpt: ") + static_ca…。
        throw std::runtime_error(std::string("invalid excerpt: ") + static_cast<char>(0xe4) + "'");
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 执行当前声明或表达式：agent.process(db, task.at("id"));。
    agent.process(db, task.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
    const auto raw = Json::parse(db.getClassroomActivity(task.at("id"))->payload);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(raw.at("status") == "failed" && calls == 3 && raw.at("failureC…。
    assert(raw.at("status") == "failed" && calls == 3 && raw.at("failureCount") == 3);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(raw.at("event").at("text") == "保留学生原始输入" && raw.at("completedA…。
    assert(raw.at("event").at("text") == "保留学生原始输入" && raw.at("completedActions").empty());
    // 【逐行批注】 计算右侧结果并写入 auto error，更新当前运行状态。
    const auto error = Json::parse(raw.at("messages").back().at("content").get<std::string>());
    // 【逐行批注】 调用 assert、string 完成本行所需操作：assert(error.at("toolError") == std::string("invalid excerpt: ") + u8…。
    assert(error.at("toolError") == std::string("invalid excerpt: ") + u8"�'");
// 【逐行批注】 结束当前代码块。
}

#pragma once

// 回放真实故障的两类括号错误；未经完整校验的文字和动作不得公开或保存。
inline void runTeachingProtocolChecks(const std::filesystem::path& path) {
    const std::vector<std::string> malformed = {
        R"({"teaching":{"helpType":"explanation"},"message":"INVALID-PUBLIC","actions":[{"id":"bad-section","tool":"append_section","args":{"lessonId":"protocol-lesson","section":{"kind":"explanation","title":"不应应用","body":"INVALID-PUBLIC","helpType":"explanation"}}],"state":"waiting_student"})",
        R"({"teaching":{"helpType":"explanation"},"message":"INVALID-PUBLIC",{"actions":[],"state":"waiting_student"}})"
    };
    for (std::size_t index = 0; index < malformed.size(); ++index) {
        const auto database = path.string() + "." + std::to_string(index);
        gangyi::Database db; db.open(database); db.migrate();
        const gangyi::AgentAccess access{"protocol-check", {"protocol-math"}};
        assert(db.insert(gangyi::Course{"protocol-math", access.scopeId, {}, "分数", "deep", "分数", {}, "ai", "active", "now", "now"}));
        const Json lesson = {{"id", "protocol-lesson"}, {"scopeId", access.scopeId}, {"courseId", "protocol-math"},
            {"title", "分数"}, {"purpose", "校验错误恢复"}, {"status", "ready"}, {"entered", true}, {"version", 1},
            {"updatedAt", "now"}, {"sourceLearningVersion", db.learningRevision()}, {"sections", {
                {{"id", "q1"}, {"kind", "question"}, {"title", "实际作答"}, {"questionKind", "practice"},
                    {"questionId", "protocol-q1"}, {"version", 1}, {"question", {{"question", "1/2+1/3=?"},
                    {"expectedAnswer", "PRIVATE-5/6"}, {"rubric", "PRIVATE-STANDARD"}}}}
            }}};
        assert(db.upsert(gangyi::ClassroomActivity{"protocol-lesson", "protocol-math", "agent-lesson", lesson.dump(), "now", 0, 0}));
        const Json event = {{"type", "question_answer"}, {"courseId", "protocol-math"}, {"lessonId", "protocol-lesson"},
            {"sectionId", "q1"}, {"sectionVersion", 1}, {"view", "practice"}, {"requestId", "protocol-answer"}, {"text", "2/5"}};
        const auto task = gangyi::agentSubmit(db, access, event);
        const auto actual = gangyi::agentContext(db, access, event).at("currentInputs").at(0);
        int assessmentCalls = 0, teachingCalls = 0;
        bool recover = false;
        gangyi::LearningAgent agent(database, [&](const gangyi::ChatOptions& options, const auto& sink) {
            assert(options.maxTokens == 8192 && options.temperature == 0.25 && options.maxAttempts == 1);
            assert(options.responseFormat == "json_object");
            assert(options.messages.front().content.find(gangyi::kAgentTeachingAccuracyRule) != std::string::npos);
            if (options.activity.purpose == "判断本次回答或追问") {
                ++assessmentCalls; assert(assessmentCalls == 1);
                return output({{"message", ""}, {"teaching", {{"helpType", "none"}}}, {"state", "continue"},
                    {"actions", {{{"id", "original-diagnosis"}, {"tool", "evaluate_answer"}, {"args", {
                        {"interactionId", actual.at("interactionId")}, {"score", 20}, {"credible", true},
                        {"feedback", "分母表示整体等分的份数，需要统一分数单位。"}, {"feedbackHelpType", "explanation"},
                        {"reason", "真实回答把分子分母分别相加"}, {"uncertainty", "只有一次回答"},
                        {"diagnosis", {{"summary", "混淆分数单位"}, {"evidence", {"2/5"}},
                            {"teachingDirection", "解释同一整体的等分份数与每份大小"}, {"needsClarification", false}}}
                    }}}}}}, sink);
            }
            ++teachingCalls;
            if (!recover) {
                const auto& content = malformed[index];
                for (char byte : content) sink(std::string(1, byte));
                gangyi::AIResult result; result.content = content; result.model = "fixture"; result.finishReason = "stop";
                return result;
            }
            return output({{"message", "分母表示把同一整体平均分成的份数；先统一分数单位，再解释你的方法。"},
                {"teaching", {{"helpType", "explanation"}, {"demonstratedQuestions", Json::array()}}},
                {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
        });
        gangyi::registerAgentLessonTools(agent); agent.process(db, task.at("id"));
        auto raw = Json::parse(db.getClassroomActivity(task.at("id"))->payload);
        assert(raw.at("status") == "failed" && assessmentCalls == 1 && teachingCalls == 3);
        assert(raw.at("completedActions").size() == 1 && raw.at("completedActions").contains("original-diagnosis"));
        const auto diagnostic = raw.at("failure").at("responseDiagnostic");
        assert(diagnostic.at("matchesStream") == true && diagnostic.at("parseByte").get<std::size_t>() > 0);
        assert(diagnostic.at("receivedBytes") == malformed[index].size() && diagnostic.at("streamBytes") == malformed[index].size());
        for (const auto& item : raw.at("events")) if (item.value("type", "") == "delta")
            assert(item.value("text", "").find("INVALID-PUBLIC") == std::string::npos);
        auto shown = gangyi::agentLessonView(db, access, "protocol-lesson");
        assert(shown.at("sections").size() == 1 && shown.at("evaluations").empty());
        assert(shown.dump().find("INVALID-PUBLIC") == std::string::npos && shown.dump().find("PRIVATE-") == std::string::npos);
        recover = true;
        gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", task.at("id")}});
        agent.process(db, task.at("id"));
        shown = gangyi::agentLessonView(db, access, "protocol-lesson");
        assert(gangyi::agentView(db, access, task.at("id")).at("status") == "waiting_student");
        assert(assessmentCalls == 1 && teachingCalls == 4 && shown.at("evaluations").size() == 1);
        int answers = 0, assessments = 0;
        for (const auto& interaction : db.listInteractions()) {
            if (interaction.kind == "chat-user") ++answers;
            if (interaction.kind == "practice") ++assessments;
        }
        assert(shown.at("evaluations").at(0).at("score") == 20 && answers == 1 && assessments == 1);
        agent.process(db, task.at("id"));
        assert(assessmentCalls == 1 && teachingCalls == 4);
    }

    // JSON 解析器的异常摘录可能截断汉字；记录错误时不得再次抛出并遗留运行中任务。
    const auto database = path.string() + ".error-text";
    gangyi::Database db; db.open(database); db.migrate();
    const gangyi::AgentAccess access{"protocol-error", {"error-math"}};
    assert(db.insert(gangyi::Course{"error-math", access.scopeId, {}, "分数", "deep", "分数", {}, "ai", "active", "now", "now"}));
    const auto task = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"courseId", "error-math"},
        {"requestId", "invalid-error-text"}, {"text", "保留学生原始输入"}});
    int calls = 0;
    gangyi::LearningAgent agent(database, [&](const auto&, const auto&) -> gangyi::AIResult {
        ++calls;
        throw std::runtime_error(std::string("invalid excerpt: ") + static_cast<char>(0xe4) + "'");
    });
    agent.process(db, task.at("id"));
    const auto raw = Json::parse(db.getClassroomActivity(task.at("id"))->payload);
    assert(raw.at("status") == "failed" && calls == 3 && raw.at("failureCount") == 3);
    assert(raw.at("event").at("text") == "保留学生原始输入" && raw.at("completedActions").empty());
    const auto error = Json::parse(raw.at("messages").back().at("content").get<std::string>());
    assert(error.at("toolError") == std::string("invalid excerpt: ") + u8"�'");
}

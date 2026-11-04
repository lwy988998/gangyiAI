// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 验证评价正文的真实展示会影响后续作答；确定性夹具不用于宣称教学效果。
// 【逐行批注】 定义函数 runEvaluationFeedbackChecks，执行该函数声明的职责。
inline void runEvaluationFeedbackChecks(const std::filesystem::path& path) {
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(path.string()); db.migrate();。
    gangyi::Database db; db.open(path.string()); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess access{"feedback-check", {"math"}};。
    const gangyi::AgentAccess access{"feedback-check", {"math"}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "数学", "de…。
    assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "数学", "deep", "数学", {}, "ai", "active", "2026-10-14", "2026-10-14"}));
    // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
    const Json lesson = {{"id", "feedback-lesson"}, {"scopeId", access.scopeId}, {"courseId", "math"}, {"title", "分数"},
        // 【逐行批注】 执行当前声明或表达式：{"purpose", "验证评价展示"}, {"status", "ready"}, {"entered", true}, {"version", 1}, {"upd…。
        {"purpose", "验证评价展示"}, {"status", "ready"}, {"entered", true}, {"version", 1}, {"updatedAt", "2026-10-14"},
        // 【逐行批注】 执行当前声明或表达式：{"sourceLearningVersion", db.learningRevision()}, {"sections", {。
        {"sourceLearningVersion", db.learningRevision()}, {"sections", {
            // 【逐行批注】 执行当前声明或表达式：{{"id", "q1"}, {"kind", "question"}, {"questionKind", "practice"}, {"questionId", "f…。
            {{"id", "q1"}, {"kind", "question"}, {"questionKind", "practice"}, {"questionId", "feedback-q1"}, {"version", 1},
                // 【逐行批注】 计算右侧结果并写入 {"question", {{"question", "1/2+1/3，更新当前运行状态。
                {"question", {{"question", "1/2+1/3=?"}, {"expectedAnswer", "PRIVATE-5/6"}, {"rubric", "PRIVATE-RUBRIC"}}}},
            // 【逐行批注】 执行当前声明或表达式：{{"id", "q2"}, {"kind", "question"}, {"questionKind", "practice"}, {"questionId", "f…。
            {{"id", "q2"}, {"kind", "question"}, {"questionKind", "practice"}, {"questionId", "feedback-q2"}, {"version", 1},
                // 【逐行批注】 计算右侧结果并写入 {"question", {{"question", "1/4+1/6，更新当前运行状态。
                {"question", {{"question", "1/4+1/6=?"}, {"expectedAnswer", "PRIVATE-5/12"}}}}
        // 【逐行批注】 执行当前声明或表达式：}}};。
        }}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(gangyi::ClassroomActivity{"feedback-lesson", "math",…。
    assert(db.upsert(gangyi::ClassroomActivity{"feedback-lesson", "math", "agent-lesson", lesson.dump(), "2026-10-14", 0, 0}));
    // 【逐行批注】 计算右侧结果并写入 int serial，更新当前运行状态。
    int serial = 0;
    // 【逐行批注】 计算右侧结果并写入 auto submit，更新当前运行状态。
    const auto submit = [&](const std::string& section) {
        // 【逐行批注】 结束当前函数并返回结果：return gangyi::agentSubmit(db, access, {{"type", "question_answer"}, …。
        return gangyi::agentSubmit(db, access, {{"type", "question_answer"}, {"text", "2/5"}, {"courseId", "math"},
            // 【逐行批注】 执行当前声明或表达式：{"lessonId", "feedback-lesson"}, {"sectionId", section}, {"sectionVersion", 1}, {"vi…。
            {"lessonId", "feedback-lesson"}, {"sectionId", section}, {"sectionVersion", 1}, {"view", "practice"},
            // 【逐行批注】 调用 to_string 完成本行所需操作：{"requestId", "feedback-" + std::to_string(++serial)}});。
            {"requestId", "feedback-" + std::to_string(++serial)}});
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto inputFor，更新当前运行状态。
    const auto inputFor = [&](const Json& task) {
        // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
        const auto raw = Json::parse(db.getClassroomActivity(task.at("id"))->payload);
        // 【逐行批注】 结束当前函数并返回结果：return gangyi::agentContext(db, access, raw.at("event")).at("currentI…。
        return gangyi::agentContext(db, access, raw.at("event")).at("currentInputs")[0];
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto inspect，更新当前运行状态。
    const auto inspect = [&](const std::string& section) {
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = submit(section), input = inputFor(task);
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", t…。
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", task.at("id")}});
        // 【逐行批注】 结束当前函数并返回结果：return input;。
        return input;
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto evaluate，更新当前运行状态。
    const auto evaluate = [&](const Json& task, const std::string& helpType, bool omit = false, bool failTeaching = false) {
        // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
        const auto input = inputFor(task); int calls = 0;
        // 【逐行批注】 声明对象 grader，并通过构造函数初始化其状态。
        gangyi::LearningAgent grader(path.string(), [&](const auto&, const auto& sink) -> gangyi::AIResult {
            // 【逐行批注】 判断条件 (++calls == 1) {，成立时执行对应分支。
            if (++calls == 1) {
                // 【逐行批注】 计算右侧结果并写入 Json args，更新当前运行状态。
                Json args = {{"interactionId", input.at("interactionId")}, {"score", 20}, {"credible", true},
                    // 【逐行批注】 计算右侧结果并写入 {"feedback", "通分：1/2，更新当前运行状态。
                    {"feedback", "通分：1/2=3/6，1/3=2/6，所以相加为5/6。"}, {"reason", "实际回答把分母相加"},
                    // 【逐行批注】 执行当前声明或表达式：{"uncertainty", "这一次回答不能证明掌握"}, {"diagnosis", {{"summary", "把分母相加"}, {"evidence", {"…。
                    {"uncertainty", "这一次回答不能证明掌握"}, {"diagnosis", {{"summary", "把分母相加"}, {"evidence", {"2/5"}},
                        // 【逐行批注】 执行当前声明或表达式：{"teachingDirection", "PRIVATE-DIRECTION：补充分数单位"}, {"needsClarification", false}}}};。
                        {"teachingDirection", "PRIVATE-DIRECTION：补充分数单位"}, {"needsClarification", false}}}};
                // 【逐行批注】 判断条件 (!omit) args["feedbackHelpType"] = helpType;，成立时执行对应分支。
                if (!omit) args["feedbackHelpType"] = helpType;
                // 【逐行批注】 结束当前函数并返回结果：return output({{"message", ""}, {"actions", {{{"id", "feedback-grade"…。
                return output({{"message", ""}, {"actions", {{{"id", "feedback-grade"}, {"tool", "evaluate_answer"}, {"args", args}}}},
                    // 【逐行批注】 执行当前声明或表达式：{"teaching", {{"helpType", "none"}}}, {"state", "continue"}}, sink);。
                    {"teaching", {{"helpType", "none"}}}, {"state", "continue"}}, sink);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (failTeaching) throw gangyi::AIClientError("fixture-failure", "模…，成立时执行对应分支。
            if (failTeaching) throw gangyi::AIClientError("fixture-failure", "模拟后续教学请求失败");
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "请再独立说明分数单位。"}, {"actions", Json::array()},。
            return output({{"message", "请再独立说明分数单位。"}, {"actions", Json::array()},
                // 【逐行批注】 执行当前声明或表达式：{"teaching", {{"helpType", "none"}}}, {"state", "waiting_student"}}, sink);。
                {"teaching", {{"helpType", "none"}}}, {"state", "waiting_student"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(grader); grader.process(db, task.at(…。
        gangyi::registerAgentLessonTools(grader); grader.process(db, task.at("id"));
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 新协议缺少明确类型或提交非法类型，不能发布评价。
    // 【逐行批注】 按循环规则重复处理：for (const auto& type : {"", "invalid"}) {。
    for (const auto& type : {"", "invalid"}) {
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = submit("q1"); evaluate(task, type, type[0] == '\0');
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, task.at("id")).at("status") == "…。
        assert(gangyi::agentView(db, access, task.at("id")).at("status") == "failed");
        // 【逐行批注】 调用 assert、agentLessonView 完成本行所需操作：assert(gangyi::agentLessonView(db, access, "feedback-lesson").at("eva…。
        assert(gangyi::agentLessonView(db, access, "feedback-lesson").at("evaluations").empty());
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto staged，更新当前运行状态。
    const auto staged = submit("q1"); evaluate(staged, "worked_example", false, true);
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, staged.at("id")).at("status") ==…。
    assert(gangyi::agentView(db, access, staged.at("id")).at("status") == "failed");
    // 【逐行批注】 执行当前声明或表达式：std::string stagedId;。
    std::string stagedId;
    // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.listClassroomActivities("math")) if (row.kind ==…。
    for (const auto& row : db.listClassroomActivities("math")) if (row.kind == "agent-evaluation") stagedId = row.id;
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!stagedId.empty());。
    assert(!stagedId.empty());
    // 【逐行批注】 计算右侧结果并写入 auto reject，更新当前运行状态。
    const auto reject = [&](const Json& body) {
        // 【逐行批注】 计算右侧结果并写入 auto before，更新当前运行状态。
        const auto before = db.profileMeta("agent-evaluation-exposure:feedback-lesson");
        // 【逐行批注】 计算右侧结果并写入 auto sectionsBefore，更新当前运行状态。
        const auto sectionsBefore = db.profileMeta("agent-section-exposure:feedback-lesson");
        // 【逐行批注】 计算右侧结果并写入 auto revision，更新当前运行状态。
        const auto revision = db.learningRevision(); bool denied = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::agentExposeTask(db, access, body); } catch (...) { denied = true; }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(denied && db.profileMeta("agent-evaluation-exposure:feedback-l…。
        assert(denied && db.profileMeta("agent-evaluation-exposure:feedback-lesson") == before);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("agent-section-exposure:feedback-lesson") == se…。
        assert(db.profileMeta("agent-section-exposure:feedback-lesson") == sectionsBefore && db.learningRevision() == revision);
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 std::string feedback，更新当前运行状态。
    const std::string feedback = "通分：1/2=3/6，1/3=2/6，所以相加为5/6。";
    // 【逐行批注】 调用 reject 完成本行所需操作：reject({{"lessonId", "feedback-lesson"}, {"evaluations", {{{"id", sta…。
    reject({{"lessonId", "feedback-lesson"}, {"evaluations", {{{"id", stagedId}, {"feedback", feedback}, {"complete", true}}}}});
    // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
    const auto first = submit("q1"); evaluate(first, "worked_example");
    // 【逐行批注】 计算右侧结果并写入 shown，更新当前运行状态。
    auto shown = gangyi::agentLessonView(db, access, "feedback-lesson");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.at("evaluations").size() == 1 && shown.at("evaluations")…。
    assert(shown.at("evaluations").size() == 1 && shown.at("evaluations")[0].at("feedbackHelpType") == "worked_example");
    // 【逐行批注】 计算右侧结果并写入 auto evaluationId，更新当前运行状态。
    const auto evaluationId = shown.at("evaluations")[0].at("id");
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q1").at("helpMode") == "independent");。
    assert(inspect("q1").at("helpMode") == "independent");
    // 【逐行批注】 计算右侧结果并写入 Json entry，更新当前运行状态。
    const Json entry = {{"id", evaluationId}, {"feedback", feedback}, {"complete", true}};
    // 【逐行批注】 计算右侧结果并写入 altered，更新当前运行状态。
    auto altered = entry; altered["feedback"] = "不是已发布的反馈";
    // 【逐行批注】 调用 reject、array 完成本行所需操作：reject({{"lessonId", "feedback-lesson"}, {"evaluations", Json::array(…。
    reject({{"lessonId", "feedback-lesson"}, {"evaluations", Json::array({altered})}});
    // 【逐行批注】 计算右侧结果并写入 pending，更新当前运行状态。
    auto pending = entry; pending["id"] = stagedId;
    // 【逐行批注】 调用 reject、array 完成本行所需操作：reject({{"lessonId", "feedback-lesson"}, {"sections", {{{"id", "q1"},…。
    reject({{"lessonId", "feedback-lesson"}, {"sections", {{{"id", "q1"}, {"version", 1}}}}, {"evaluations", Json::array({entry, pending})}});
    // 【逐行批注】 计算右侧结果并写入 foreign，更新当前运行状态。
    auto foreign = lesson; foreign["id"] = "other-lesson";
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(gangyi::ClassroomActivity{"other-lesson", "math", "a…。
    assert(db.upsert(gangyi::ClassroomActivity{"other-lesson", "math", "agent-lesson", foreign.dump(), "2026-10-14", 0, 0}));
    // 【逐行批注】 调用 reject、array 完成本行所需操作：reject({{"lessonId", "other-lesson"}, {"evaluations", Json::array({en…。
    reject({{"lessonId", "other-lesson"}, {"evaluations", Json::array({entry})}});
    // 【逐行批注】 计算右侧结果并写入 partial，更新当前运行状态。
    auto partial = entry; partial["complete"] = false;
    // 【逐行批注】 调用 agentExposeTask、array 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"},…。
    gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"}, {"evaluations", Json::array({partial})}});
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q1").at("helpMode") == "help_unknown");。
    assert(inspect("q1").at("helpMode") == "help_unknown");
    // 【逐行批注】 调用 agentExposeTask、array 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"},…。
    gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"}, {"evaluations", Json::array({entry})}});
    // 延迟的部分展示回执不能覆盖已经确认的完整展示，也不能重复加入同一反馈。
    // 【逐行批注】 调用 agentExposeTask、array 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"},…。
    gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"}, {"evaluations", Json::array({partial})}});
    // 【逐行批注】 计算右侧结果并写入 auto assisted，更新当前运行状态。
    const auto assisted = inspect("q1");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(assisted.at("helpMode") == "after_example" && assisted.at("hel…。
    assert(assisted.at("helpMode") == "after_example" && assisted.at("helpHistory").size() == 1);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(assisted.at("helpHistory")[0].at("source") == "evaluation" && …。
    assert(assisted.at("helpHistory")[0].at("source") == "evaluation" && !assisted.at("helpHistory")[0].at("partial").get<bool>());
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(assisted.at("helpHistory")[0].at("text").get<std::string>().fi…。
    assert(assisted.at("helpHistory")[0].at("text").get<std::string>().find("PRIVATE-") == std::string::npos);
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q2").at("helpMode") == "independent");。
    assert(inspect("q2").at("helpMode") == "independent");
    // 旧协议允许保存缺失类型，但必须明确标记 unknown；不能暗示独立掌握。
    // 【逐行批注】 计算右侧结果并写入 auto old，更新当前运行状态。
    const auto old = submit("q2"); auto oldRow = *db.getClassroomActivity(old.at("id"));
    // 【逐行批注】 计算右侧结果并写入 oldRaw，更新当前运行状态。
    auto oldRaw = Json::parse(oldRow.payload); oldRaw["teachingProtocolVersion"] = 2;
    // 【逐行批注】 计算右侧结果并写入 oldRow.payload，更新当前运行状态。
    oldRow.payload = oldRaw.dump(); assert(db.upsert(oldRow)); evaluate(old, "", true);
    // 【逐行批注】 计算右侧结果并写入 shown，更新当前运行状态。
    shown = gangyi::agentLessonView(db, access, "feedback-lesson");
    // 【逐行批注】 计算右侧结果并写入 auto oldEvaluation，更新当前运行状态。
    const auto oldEvaluation = shown.at("evaluations").back(); assert(oldEvaluation.at("feedbackHelpType") == "unknown");
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"},…。
    gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"}, {"evaluations", {{{"id", oldEvaluation.at("id")}, {"feedback", feedback}, {"complete", true}}}}});
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q2").at("helpMode") == "help_unknown");。
    assert(inspect("q2").at("helpMode") == "help_unknown");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.dump().find("PRIVATE-5") == std::string::npos && shown.d…。
    assert(shown.dump().find("PRIVATE-5") == std::string::npos && shown.dump().find("PRIVATE-RUBRIC") == std::string::npos &&
        // 【逐行批注】 执行当前声明或表达式：shown.dump().find("expectedAnswer") == std::string::npos);。
        shown.dump().find("expectedAnswer") == std::string::npos);
    // 【逐行批注】 计算右侧结果并写入 diagnosis，更新当前运行状态。
    auto diagnosis = Json{{"score", 20}, {"credible", true}, {"diagnosis", {{"summary", "分母相加"}, {"evidence", {"2/5"}},
        // 【逐行批注】 执行当前声明或表达式：{"teachingDirection", "补充分数单位"}, {"needsClarification", false}}}};。
        {"teachingDirection", "补充分数单位"}, {"needsClarification", false}}}};
    // 【逐行批注】 按循环规则重复处理：for (const auto& field : {"summary", "teachingDirection"}) {。
    for (const auto& field : {"summary", "teachingDirection"}) {
        // 【逐行批注】 计算右侧结果并写入 blank，更新当前运行状态。
        auto blank = diagnosis; blank["diagnosis"][field] = " \t\r\n"; bool denied = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::checkedDiagnosis(blank, "2/5"); } catch (...) { denied = true; } assert(denied);
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

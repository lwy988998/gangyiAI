#pragma once

// 验证评价正文的真实展示会影响后续作答；确定性夹具不用于宣称教学效果。
inline void runEvaluationFeedbackChecks(const std::filesystem::path& path) {
    gangyi::Database db; db.open(path.string()); db.migrate();
    const gangyi::AgentAccess access{"feedback-check", {"math"}};
    assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "数学", "deep", "数学", {}, "ai", "active", "2026-10-14", "2026-10-14"}));
    const Json lesson = {{"id", "feedback-lesson"}, {"scopeId", access.scopeId}, {"courseId", "math"}, {"title", "分数"},
        {"purpose", "验证评价展示"}, {"status", "ready"}, {"entered", true}, {"version", 1}, {"updatedAt", "2026-10-14"},
        {"sourceLearningVersion", db.learningRevision()}, {"sections", {
            {{"id", "q1"}, {"kind", "question"}, {"questionKind", "practice"}, {"questionId", "feedback-q1"}, {"version", 1},
                {"question", {{"question", "1/2+1/3=?"}, {"expectedAnswer", "PRIVATE-5/6"}, {"rubric", "PRIVATE-RUBRIC"}}}},
            {{"id", "q2"}, {"kind", "question"}, {"questionKind", "practice"}, {"questionId", "feedback-q2"}, {"version", 1},
                {"question", {{"question", "1/4+1/6=?"}, {"expectedAnswer", "PRIVATE-5/12"}}}}
        }}};
    assert(db.upsert(gangyi::ClassroomActivity{"feedback-lesson", "math", "agent-lesson", lesson.dump(), "2026-10-14", 0, 0}));
    int serial = 0;
    const auto submit = [&](const std::string& section) {
        return gangyi::agentSubmit(db, access, {{"type", "question_answer"}, {"text", "2/5"}, {"courseId", "math"},
            {"lessonId", "feedback-lesson"}, {"sectionId", section}, {"sectionVersion", 1}, {"view", "practice"},
            {"requestId", "feedback-" + std::to_string(++serial)}});
    };
    const auto inputFor = [&](const Json& task) {
        const auto raw = Json::parse(db.getClassroomActivity(task.at("id"))->payload);
        return gangyi::agentContext(db, access, raw.at("event")).at("currentInputs")[0];
    };
    const auto inspect = [&](const std::string& section) {
        const auto task = submit(section), input = inputFor(task);
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", task.at("id")}});
        return input;
    };
    const auto evaluate = [&](const Json& task, const std::string& helpType, bool omit = false, bool failTeaching = false) {
        const auto input = inputFor(task); int calls = 0;
        gangyi::LearningAgent grader(path.string(), [&](const auto&, const auto& sink) -> gangyi::AIResult {
            if (++calls == 1) {
                Json args = {{"interactionId", input.at("interactionId")}, {"score", 20}, {"credible", true},
                    {"feedback", "通分：1/2=3/6，1/3=2/6，所以相加为5/6。"}, {"reason", "实际回答把分母相加"},
                    {"uncertainty", "这一次回答不能证明掌握"}, {"diagnosis", {{"summary", "把分母相加"}, {"evidence", {"2/5"}},
                        {"teachingDirection", "PRIVATE-DIRECTION：补充分数单位"}, {"needsClarification", false}}}};
                if (!omit) args["feedbackHelpType"] = helpType;
                return output({{"message", ""}, {"actions", {{{"id", "feedback-grade"}, {"tool", "evaluate_answer"}, {"args", args}}}},
                    {"teaching", {{"helpType", "none"}}}, {"state", "continue"}}, sink);
            }
            if (failTeaching) throw gangyi::AIClientError("fixture-failure", "模拟后续教学请求失败");
            return output({{"message", "请再独立说明分数单位。"}, {"actions", Json::array()},
                {"teaching", {{"helpType", "none"}}}, {"state", "waiting_student"}}, sink);
        });
        gangyi::registerAgentLessonTools(grader); grader.process(db, task.at("id"));
    };
    // 新协议缺少明确类型或提交非法类型，不能发布评价。
    for (const auto& type : {"", "invalid"}) {
        const auto task = submit("q1"); evaluate(task, type, type[0] == '\0');
        assert(gangyi::agentView(db, access, task.at("id")).at("status") == "failed");
        assert(gangyi::agentLessonView(db, access, "feedback-lesson").at("evaluations").empty());
    }
    const auto staged = submit("q1"); evaluate(staged, "worked_example", false, true);
    assert(gangyi::agentView(db, access, staged.at("id")).at("status") == "failed");
    std::string stagedId;
    for (const auto& row : db.listClassroomActivities("math")) if (row.kind == "agent-evaluation") stagedId = row.id;
    assert(!stagedId.empty());
    const auto reject = [&](const Json& body) {
        const auto before = db.profileMeta("agent-evaluation-exposure:feedback-lesson");
        const auto sectionsBefore = db.profileMeta("agent-section-exposure:feedback-lesson");
        const auto revision = db.learningRevision(); bool denied = false;
        try { gangyi::agentExposeTask(db, access, body); } catch (...) { denied = true; }
        assert(denied && db.profileMeta("agent-evaluation-exposure:feedback-lesson") == before);
        assert(db.profileMeta("agent-section-exposure:feedback-lesson") == sectionsBefore && db.learningRevision() == revision);
    };
    const std::string feedback = "通分：1/2=3/6，1/3=2/6，所以相加为5/6。";
    reject({{"lessonId", "feedback-lesson"}, {"evaluations", {{{"id", stagedId}, {"feedback", feedback}, {"complete", true}}}}});
    const auto first = submit("q1"); evaluate(first, "worked_example");
    auto shown = gangyi::agentLessonView(db, access, "feedback-lesson");
    assert(shown.at("evaluations").size() == 1 && shown.at("evaluations")[0].at("feedbackHelpType") == "worked_example");
    const auto evaluationId = shown.at("evaluations")[0].at("id");
    assert(inspect("q1").at("helpMode") == "independent");
    const Json entry = {{"id", evaluationId}, {"feedback", feedback}, {"complete", true}};
    auto altered = entry; altered["feedback"] = "不是已发布的反馈";
    reject({{"lessonId", "feedback-lesson"}, {"evaluations", Json::array({altered})}});
    auto pending = entry; pending["id"] = stagedId;
    reject({{"lessonId", "feedback-lesson"}, {"sections", {{{"id", "q1"}, {"version", 1}}}}, {"evaluations", Json::array({entry, pending})}});
    auto foreign = lesson; foreign["id"] = "other-lesson";
    assert(db.upsert(gangyi::ClassroomActivity{"other-lesson", "math", "agent-lesson", foreign.dump(), "2026-10-14", 0, 0}));
    reject({{"lessonId", "other-lesson"}, {"evaluations", Json::array({entry})}});
    auto partial = entry; partial["complete"] = false;
    gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"}, {"evaluations", Json::array({partial})}});
    assert(inspect("q1").at("helpMode") == "help_unknown");
    gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"}, {"evaluations", Json::array({entry})}});
    // 延迟的部分展示回执不能覆盖已经确认的完整展示，也不能重复加入同一反馈。
    gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"}, {"evaluations", Json::array({partial})}});
    const auto assisted = inspect("q1");
    assert(assisted.at("helpMode") == "after_example" && assisted.at("helpHistory").size() == 1);
    assert(assisted.at("helpHistory")[0].at("source") == "evaluation" && !assisted.at("helpHistory")[0].at("partial").get<bool>());
    assert(assisted.at("helpHistory")[0].at("text").get<std::string>().find("PRIVATE-") == std::string::npos);
    assert(inspect("q2").at("helpMode") == "independent");
    // 旧协议允许保存缺失类型，但必须明确标记 unknown；不能暗示独立掌握。
    const auto old = submit("q2"); auto oldRow = *db.getClassroomActivity(old.at("id"));
    auto oldRaw = Json::parse(oldRow.payload); oldRaw["teachingProtocolVersion"] = 2;
    oldRow.payload = oldRaw.dump(); assert(db.upsert(oldRow)); evaluate(old, "", true);
    shown = gangyi::agentLessonView(db, access, "feedback-lesson");
    const auto oldEvaluation = shown.at("evaluations").back(); assert(oldEvaluation.at("feedbackHelpType") == "unknown");
    gangyi::agentExposeTask(db, access, {{"lessonId", "feedback-lesson"}, {"evaluations", {{{"id", oldEvaluation.at("id")}, {"feedback", feedback}, {"complete", true}}}}});
    assert(inspect("q2").at("helpMode") == "help_unknown");
    assert(shown.dump().find("PRIVATE-5") == std::string::npos && shown.dump().find("PRIVATE-RUBRIC") == std::string::npos &&
        shown.dump().find("expectedAnswer") == std::string::npos);
    auto diagnosis = Json{{"score", 20}, {"credible", true}, {"diagnosis", {{"summary", "分母相加"}, {"evidence", {"2/5"}},
        {"teachingDirection", "补充分数单位"}, {"needsClarification", false}}}};
    for (const auto& field : {"summary", "teachingDirection"}) {
        auto blank = diagnosis; blank["diagnosis"][field] = " \t\r\n"; bool denied = false;
        try { gangyi::checkedDiagnosis(blank, "2/5"); } catch (...) { denied = true; } assert(denied);
    }
}

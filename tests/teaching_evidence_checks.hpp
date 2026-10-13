#pragma once
#include "teaching_evidence.hpp"
#include "agent_profiles.hpp"
#include "my_ai.hpp"

// 接口回归使用确定性模型夹具；真实教学效果另由同模型多轮对照验证。
inline void runTeachingEvidenceChecks(const std::filesystem::path& path) {
    gangyi::Database db; db.open(path.string()); db.migrate();
    const gangyi::AgentAccess access{"teaching-check", {"math"}};
    assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "数学", "deep", "数学", {}, "ai", "active", "2026-10-13", "2026-10-13"}));
    Json lesson = {{"id", "lesson-check"}, {"scopeId", access.scopeId}, {"courseId", "math"}, {"title", "分数"},
        {"purpose", "验证教学记录"}, {"status", "ready"}, {"entered", true}, {"version", 1}, {"updatedAt", "2026-10-13"},
        {"sourceLearningVersion", db.learningRevision()}, {"sections", {
            {{"id", "concept"}, {"kind", "explanation"}, {"title", "分数概念"}, {"body", "分母表示每份大小"}, {"version", 1}},
            {{"id", "q1"}, {"kind", "question"}, {"title", "第一题"}, {"questionKind", "practice"}, {"questionId", "qid-1"}, {"version", 1},
                {"question", {{"question", "1/2+1/3=?"}, {"expectedAnswer", "5/6"}, {"rubric", "PRIVATE-CHECK"}}}},
            {{"id", "q2"}, {"kind", "question"}, {"title", "第二题"}, {"questionKind", "practice"}, {"questionId", "qid-2"}, {"version", 1},
                {"question", {{"question", "1/4+1/6=?"}, {"expectedAnswer", "5/12"}}}}
        }}};
    assert(db.upsert(gangyi::ClassroomActivity{"lesson-check", "math", "agent-lesson", lesson.dump(), "2026-10-13", 0, 0}));
    gangyi::agentExposeTask(db, access, {{"lessonId", "lesson-check"}, {"sections", {{{"id", "concept"}, {"version", 1}}}}});
    int identity = 0;
    const auto submit = [&](std::string section, std::string type, std::string text, std::string request = "") {
        Json event = {{"type", type}, {"text", text}, {"lessonId", "lesson-check"}, {"courseId", "math"},
            {"sectionId", section}, {"sectionVersion", 1}, {"view", "practice"}, {"requestId", "teaching-check-"+std::to_string(++identity)}};
        if (!request.empty()) event["requestType"] = request;
        return gangyi::agentSubmit(db, access, event);
    };
    const auto inputFor = [&](const Json& task) {
        const auto raw = Json::parse(db.getClassroomActivity(task.at("id"))->payload);
        return gangyi::agentContext(db, access, raw.at("event")).at("currentInputs")[0];
    };
    const auto first = submit("q1", "question_answer", "2/5");
    assert(inputFor(first).at("helpMode") == "independent" && inputFor(first).at("relatedLearning").size() == 1);
    gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", first.at("id")}});
    auto hint = submit("q1", "teaching_request", "给一点提示", "hint");
    assert(inputFor(hint).at("type") == "teaching_request");
    bool fullExample = true;
    gangyi::LearningAgent helper(path.string(), [&](const auto&, const auto& sink) {
        return output({{"message", fullExample ? "完整答案5/6" : "先想想两种分数的每份大小是否相同。"},
            {"teaching", {{"helpType", fullExample ? "worked_example" : "hint"}}}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
    });
    gangyi::registerAgentLessonTools(helper); helper.process(db, hint.at("id"));
    auto rejected = gangyi::agentView(db, access, hint.at("id")); assert(rejected.at("status") == "failed");
    for (const auto& event : rejected.at("events")) assert(event.value("field", "") != "message");
    fullExample = false; gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", hint.at("id")}});
    helper.process(db, hint.at("id"));
    auto rawHint = Json::parse(db.getClassroomActivity(hint.at("id"))->payload);
    gangyi::agentExposeTask(db, access, {{"taskId", hint.at("id")}, {"seq", rawHint.at("seq")}});
    const auto assisted = submit("q1", "question_answer", "5/6");
    assert(inputFor(assisted).at("helpMode") == "after_hint" && inputFor(assisted).at("helpHistory").size() == 1);
    gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", assisted.at("id")}});
    const auto independent = submit("q2", "question_answer", "5/12");
    assert(inputFor(independent).at("helpMode") == "independent" && inputFor(independent).at("helpHistory").empty());
    gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", independent.at("id")}});
    // 纯追问保留原话但不评分，后续不能再引用它产生掌握证据。
    auto followup = submit("q1", "question_answer", "为什么要通分？");
    const auto followupId = inputFor(followup).at("interactionId"); int calls = 0;
    gangyi::LearningAgent classifier(path.string(), [&](const auto&, const auto& sink) {
        return output({{"message", calls++ ? "需要先让每份大小相同。" : ""}, {"actions", calls == 1 ? Json{{{"id", "classify"},
            {"tool", "classify_input"}, {"args", {{"interactionId", followupId}, {"isAnswer", false}, {"reason", "学生只追问方法原因"}}}}} : Json::array()},
            {"teaching", {{"helpType", "explanation"}}}, {"state", calls == 1 ? "continue" : "waiting_student"}}, sink);
    });
    gangyi::registerAgentLessonTools(classifier); classifier.process(db, followup.at("id"));
    assert(gangyi::agentView(db, access, followup.at("id")).at("status") == "waiting_student");
    assert(!db.profileMeta("agent-input-classification:" + followupId.get<std::string>()).empty());
    auto diagnosisArgs = Json{{"score", 30}, {"credible", true}, {"diagnosis", {{"summary", "误把分母直接相加"},
        {"evidence", {"2/5"}}, {"teachingDirection", "补充分数单位概念"}, {"needsClarification", false}}}};
    assert(gangyi::checkedDiagnosis(diagnosisArgs, "2/5").at("status") == "supported");
    bool fabricated = false; try { gangyi::checkedDiagnosis(diagnosisArgs, "我暂时不会"); } catch (...) { fabricated = true; } assert(fabricated);
    diagnosisArgs["diagnosis"]["needsClarification"] = true;
    bool premature = false; try { gangyi::checkedDiagnosis(diagnosisArgs, "2/5"); } catch (...) { premature = true; } assert(premature);
    // 新题关联由主控实际保存，私有标准不能出现在学生接口。
    auto retry = submit("q1", "teaching_request", "再试一道", "retry_question");
    gangyi::LearningAgent practice(path.string(), [&](const auto&, const auto& sink) {
        const Json section = {{"kind", "question"}, {"title", "独立复练"}, {"rePracticeOf", "qid-1"},
            {"question", {{"question", "1/5+1/2=?"}, {"expectedAnswer", "7/10"}, {"rubric", "PRIVATE-RETRY"}}}};
        const Json action = {{"id", "new-question"}, {"tool", "append_section"}, {"args", {{"lessonId", "lesson-check"}, {"section", section}}}};
        return output({{"message", "试试这道新题，说明每一步。"}, {"actions", Json::array({action})},
            {"teaching", {{"helpType", "none"}}}, {"state", "waiting_student"}}, sink);
    });
    gangyi::registerAgentLessonTools(practice); practice.process(db, retry.at("id"));
    const auto shown = gangyi::agentLessonView(db, access, "lesson-check");
    assert(shown.at("sections").back().at("rePracticeOf") == "qid-1");
    assert(shown.dump().find("PRIVATE-") == std::string::npos && shown.dump().find("expectedAnswer") == std::string::npos);
    const auto retried = submit(shown.at("sections").back().at("id"), "question_answer", "7/10，通分后相加");
    assert(inputFor(retried).at("helpMode") == "independent" && inputFor(retried).at("rePracticeOf") == "qid-1");
    assert(gangyi::agentProfileEvidence(db, access).empty());
    const auto retriedInput = inputFor(retried); int gradeCalls = 0;
    gangyi::LearningAgent grader(path.string(), [&](const auto&, const auto& sink) {
        Json actions = Json::array();
        if (++gradeCalls == 1) actions.push_back({{"id", "grade-independent"}, {"tool", "evaluate_answer"}, {"args", {
            {"interactionId", retriedInput.at("interactionId")}, {"score", 90}, {"credible", true}, {"feedback", "关联新题的计算正确"},
            {"reason", "依据独立复练的真实回答"}, {"uncertainty", "还需继续观察迁移表现"},
            {"diagnosis", {{"summary", "本次新题计算正确"}, {"evidence", {"7/10"}}, {"teachingDirection", "让学生进一步解释通分理由"}, {"needsClarification", false}}}
        }}});
        return output({{"message", gradeCalls == 1 ? "" : "你独立完成了这道关联新题，再解释一下理由。"},
            {"actions", actions}, {"teaching", {{"helpType", "none"}}}, {"state", gradeCalls == 1 ? "continue" : "waiting_student"}}, sink);
    });
    gangyi::registerAgentLessonTools(grader); grader.process(db, retried.at("id"));
    const auto evidence = gangyi::agentProfileEvidence(db, access);
    assert(evidence.size() == 1 && evidence[0].at("helpMode") == "independent" && evidence[0].at("rePracticeOf") == "qid-1");
    const auto report = gangyi::myAIRead(db, "reports", {{"courseId", "math"}});
    bool requestShown = false, followupShown = false, practiceShown = false;
    for (const auto& record : report.at("evidence")) {
        assert(record.at("id") != retriedInput.at("interactionId"));
        requestShown = requestShown || record.value("inputKind", "") == "teaching_request";
        followupShown = followupShown || record.value("inputKind", "") == "followup";
        practiceShown = practiceShown || (record.value("rePracticeOf", "") == "qid-1" && record.value("assessed", false));
    }
    assert(requestShown && followupShown && practiceShown);
    assert(report.dump().find("PRIVATE-") == std::string::npos && report.dump().find("expectedAnswer") == std::string::npos);
    auto legacyRow = *db.getClassroomActivity("lesson-check"); auto legacy = Json::parse(legacyRow.payload);
    legacy["sections"].push_back({{"id", "old-example"}, {"kind", "explanation"}, {"title", "旧示范"},
        {"body", "旧示范未记录本题关联"}, {"presentation", "example"}, {"version", 1}});
    legacyRow.payload = legacy.dump(); assert(db.upsert(legacyRow));
    gangyi::agentExposeTask(db, access, {{"lessonId", "lesson-check"}, {"sections", {{{"id", "old-example"}, {"version", 1}}}}});
    const auto historical = submit("q2", "question_answer", "5/12，继续独立尝试");
    assert(inputFor(historical).at("helpMode") == "help_unknown");
    assert(inputFor(historical).at("helpHistory").empty());
    assert(inputFor(historical).at("relatedLearning").back().at("association") == "lesson_only");
}

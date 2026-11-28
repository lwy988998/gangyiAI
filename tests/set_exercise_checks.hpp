#pragma once

#include "set_exercise.hpp"

inline void runSetExerciseChecks(const std::filesystem::path& path) {
    const Json config = {{"operation", "intersection"}, {"universe", {"3", "2", "1"}}, {"setA", {"1", "2"}}, {"setB", {"2"}}};
    const auto exerciseFor = [](Json definition) { return Json{{"setExercise", std::move(definition)}}; };
    const auto checked = [&](const Json& definition) { return gangyi::checkedSetExerciseQuestion(exerciseFor(definition), Json::object()); };
    const Json compute = {{"schemaVersion", 1}, {"config", config}, {"options", Json::array({Json::array(), Json::array({"2"})})}};
    assert(checked(compute).at("answerIndex") == 1 && checked(compute).at("expectedAnswer") == "{2}");
    auto alternate = compute;
    alternate["config"]["operation"] = "union";
    alternate["options"] = {{"3"}, {"2", "1"}};
    assert(checked(alternate).at("expectedAnswer") == "{1, 2}");
    alternate["config"]["operation"] = "complement_b";
    alternate["options"] = {{"2"}, {"3", "1"}};
    assert(checked(alternate).at("expectedAnswer") == "{1, 3}");
    alternate["config"]["operation"] = "complement_a";
    alternate["config"]["setA"] = config.at("universe");
    alternate["options"] = Json::array({Json::array(), Json::array({"3"})});
    assert(checked(alternate).at("expectedAnswer") == u8"∅");

    auto relation = compute;
    relation.erase("options");
    relation["config"]["operation"] = "relations";
    const std::vector<std::pair<Json, Json>> pairs = {
        {Json::array(), Json::array()}, {Json::array(), Json::array({"1"})},
        {Json::array({"1"}), Json::array()}, {Json::array({"1"}), Json::array({"2"})}
    };
    for (std::size_t i = 0; i < pairs.size(); ++i) {
        relation["config"]["setA"] = pairs[i].first;
        relation["config"]["setB"] = pairs[i].second;
        assert(checked(relation).at("answerIndex") == i);
    }
    const auto rejects = [&](const Json& definition) {
        bool denied = false;
        try { checked(definition); } catch (const std::invalid_argument&) { denied = true; }
        assert(denied);
    };
    auto bad = compute; bad["options"] = {{"1"}, {"3"}}; rejects(bad);
    bad = compute; bad["options"] = {{"2"}, {"3", "1"}, {"1", "3"}}; rejects(bad);
    bad = compute; bad["options"] = {{"2"}, {"4"}}; rejects(bad);
    bad = compute; bad["config"]["universe"][0] = "1, 3"; rejects(bad);
    bad = compute; bad["mediaId"] = "same-lesson-set"; rejects(bad);
    rejects({{"schemaVersion", 1}, {"mediaId", "unknown-media"}, {"options", compute.at("options")}});
    bad = relation; bad["options"] = compute.at("options"); rejects(bad);

    gangyi::Database db; db.open(path.string()); db.migrate();
    const gangyi::AgentAccess access{"set-owner", {"math"}};
    assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "集合", "deep", "集合", {}, "ai", "active", "2026-11-21", "2026-11-21"}));
    auto media = config;
    media["id"] = "same-lesson-set"; media["type"] = "sets"; media["status"] = "ready";
    const Json lesson = {{"id", "set-lesson"}, {"scopeId", access.scopeId}, {"courseId", "math"},
        {"title", "集合"}, {"purpose", "理解交集"}, {"status", "ready"}, {"entered", true},
        {"updatedAt", "2026-11-21"}, {"version", 1}, {"sections", Json::array()},
        {"deepContent", {{"version", 4}, {"media", {media}}, {"lecture", {{{"id", "page-1"}, {"mediaIds", {"same-lesson-set"}}}}},
            {"slides", Json::array()}}}};
    assert(db.upsert(gangyi::ClassroomActivity{"set-lesson", "math", "agent-lesson", lesson.dump(), "2026-11-21", 0, 0}));
    const auto task = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"courseId", "math"}, {"lessonId", "set-lesson"},
        {"requestId", "set-exercise"}, {"text", "结合这里的集合组件出一道题"}});
    const auto shown = [&] { return gangyi::agentLessonView(db, access, "set-lesson"); };
    int calls = 0;
    gangyi::LearningAgent agent(path.string(), [&](const auto&, const auto& sink) {
        ++calls;
        if (calls == 3) assert(shown().at("sections").empty());
        auto definition = Json{{"schemaVersion", 1}, {"mediaId", calls == 1 ? "unknown-media" : "same-lesson-set"},
            {"options", calls == 2 ? Json{{"1"}, {"3"}} : compute.at("options")}};
        auto section = Json{{"kind", "question"}, {"title", "检查交集"}, {"question", exerciseFor(definition)}};
        if (calls == 1) section["question"]["expectedAnswer"] = "外部写入";
        return output({{"message", calls >= 3 ? "请根据题面的初始条件选择。" : ""},
            {"actions", {{{"id", "set-candidate-" + std::to_string(calls)}, {"tool", "append_section"},
                {"args", {{"lessonId", "set-lesson"}, {"section", section}}}}}},
            {"state", calls >= 3 ? "waiting_student" : "continue"}}, sink);
    });
    gangyi::registerAgentLessonTools(agent);
    agent.process(db, task.at("id"));
    assert(calls == 3);
    const auto view = shown();
    assert(view.at("sections").size() == 1 && view.at("evaluations").empty());
    const auto& section = view.at("sections")[0];
    const auto& publicQuestion = section.at("question");
    assert(publicQuestion.at("options") == Json::array({u8"∅", "{2}"}));
    assert(publicQuestion.at("verifiedChoice") == true);
    assert(!publicQuestion.contains("answerIndex") && !publicQuestion.contains("expectedAnswer") && !publicQuestion.contains("setExercise"));

    // 题面固定在出题时的配置。更新组件并探索后，正式提交仍绑定原问题。
    auto row = db.getClassroomActivity("set-lesson").value();
    auto current = Json::parse(row.payload);
    current["deepContent"]["version"] = 5;
    current["deepContent"]["media"][0]["setB"] = {"3"};
    row.payload = current.dump(); assert(db.upsert(row));
    const Json changedState = {{"schemaVersion", 1}, {"contentVersion", 5}, {"setA", {"1"}}, {"setB", {"3"}}};
    const Json trace = {{"schemaVersion", 1}, {"contentVersion", 5}, {"sessionId", "fixture-set"}, {"totalChanges", 1},
        {"truncated", false}, {"events", {{{"seq", 1}, {"kind", "finish"}, {"state", changedState}}}}};
    const Json submitted = {{"type", "question_answer"}, {"courseId", "math"}, {"lessonId", "set-lesson"},
        {"sectionId", section.at("id")}, {"sectionVersion", section.at("version")}, {"view", "practice"},
        {"requestId", "set-answer"}, {"text", "我选2：{2}"}, {"selectedOption", 1}, {"deepView", "lecture"}, {"contentId", "page-1"},
        {"componentActivities", {{"same-lesson-set", trace}}}, {"mediaState", {{"same-lesson-set", changedState}}}};
    for (const auto& invalid : Json::array({true, 1.0, -1, 2, UINT64_MAX})) {
        auto rejected = submitted;
        rejected["selectedOption"] = invalid;
        const auto before = db.listInteractions().size();
        bool denied = false;
        try { gangyi::agentSubmit(db, access, rejected); } catch (const std::invalid_argument&) { denied = true; }
        assert(denied && db.listInteractions().size() == before);
    }
    const auto answer = gangyi::agentSubmit(db, access, submitted);
    const auto context = gangyi::agentContext(db, access, submitted);
    const auto& input = context.at("currentInputs").at(0);
    assert(input.at("questionId") == section.at("questionId"));
    assert(input.at("questionSnapshot").at("answerIndex") == 1);
    assert(input.at("selectedOption") == 1);
    assert(input.at("questionSnapshot").at("setExercise").at("origin").at("contentVersion") == 4);
    assert(input.at("componentActivities").at("same-lesson-set").at("lastObserved").at("intersection").empty());
    assert(input.at("componentActivities").at("same-lesson-set").at("scoringEligible") == false);
    assert(shown().at("evaluations").empty());
    const auto feedback = "题面给出的共有元素是 2，你的选择符合这份题面。";
    const Json grade = {{"interactionId", input.at("interactionId")}, {"score", 100}, {"credible", true}, {"feedback", feedback},
        {"feedbackHelpType", "explanation"}, {"reason", "按提交时固定题面核对选项"}, {"uncertainty", "限于这道题"},
        {"diagnosis", {{"summary", "选中交集"}, {"evidence", {input.at("text")}},
            {"teachingDirection", "继续验证并集"}, {"needsClarification", false}}}};
    int gradingCalls = 0;
    gangyi::LearningAgent grader(path.string(), [&](const auto&, const auto& sink) {
        Json actions = Json::array();
        if (++gradingCalls == 1) actions.push_back({{"id", "grade-set"}, {"tool", "evaluate_answer"}, {"args", grade}});
        return output({{"message", feedback}, {"actions", actions}, {"state", "waiting_student"}}, sink);
    });
    gangyi::registerAgentLessonTools(grader);
    grader.process(db, answer.at("id"));
    const auto evaluated = shown().at("evaluations");
    assert(evaluated.size() == 1 && evaluated.at(0).at("score") == 100 && evaluated.at(0).at("choiceCorrect") == true);

    auto wrongChoice = submitted;
    wrongChoice["requestId"] = "set-wrong-choice";
    wrongChoice["selectedOption"] = 0;
    wrongChoice["text"] = "我选1：∅。我把探索后的集合当成题面的集合了。";
    const auto wrongTask = gangyi::agentSubmit(db, access, wrongChoice);
    const auto wrongInput = gangyi::agentContext(db, access, wrongChoice).at("currentInputs").at(0);
    auto mistakenGrade = grade;
    mistakenGrade["interactionId"] = wrongInput.at("interactionId");
    mistakenGrade["diagnosis"]["evidence"] = {wrongInput.at("text")};
    auto correctedGrade = mistakenGrade;
    correctedGrade["score"] = 0;
    correctedGrade["feedback"] = "观察结果是空集，原题仍按出题时的条件求交集。";
    correctedGrade["diagnosis"]["summary"] = "混淆了探索状态和题面条件";
    int correctionCalls = 0;
    gangyi::LearningAgent corrector(path.string(), [&](const auto&, const auto& sink) {
        Json actions = Json::array();
        ++correctionCalls;
        if (correctionCalls <= 3) assert(shown().at("evaluations") == evaluated);
        if (correctionCalls == 1) actions.push_back({{"id", "wrong-full-mark"}, {"tool", "evaluate_answer"}, {"args", mistakenGrade}});
        if (correctionCalls == 2) actions.push_back({{"id", "not-a-followup"}, {"tool", "classify_input"},
            {"args", {{"interactionId", wrongInput.at("interactionId")}, {"isAnswer", false}, {"reason", "模型误判了作答"}}}});
        if (correctionCalls == 3) actions.push_back({{"id", "confirmed-wrong-choice"}, {"tool", "evaluate_answer"}, {"args", correctedGrade}});
        return output({{"message", correctionCalls < 3 ? Json("") : correctedGrade.at("feedback")},
            {"actions", actions}, {"state", correctionCalls < 3 ? "continue" : "waiting_student"}}, sink);
    });
    gangyi::registerAgentLessonTools(corrector);
    corrector.process(db, wrongTask.at("id"));
    const auto all = shown().at("evaluations");
    assert(all.size() == 2);
    const auto wrongRecord = std::find_if(all.begin(), all.end(), [&](const Json& record) { return record.at("interactionId") == wrongInput.at("interactionId"); });
    assert(wrongRecord != all.end() && wrongRecord->at("selectedOption") == 0 && wrongRecord->at("score") == 0 && wrongRecord->at("choiceCorrect") == false);

    const auto beforeHint = shown();
    const auto hintTask = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"courseId", "math"}, {"lessonId", "set-lesson"},
        {"requestId", "set-hint-private-terms"}, {"text", "我应该先比对哪个元素？"}});
    int hintCalls = 0;
    gangyi::LearningAgent hints(path.string(), [&](const auto&, const auto& sink) {
        ++hintCalls;
        assert(shown().at("sections") == beforeHint.at("sections"));
        Json actions = Json::array();
        if (hintCalls == 2) actions.push_back({{"id", "internal-body"}, {"tool", "append_section"},
            {"args", {{"lessonId", "set-lesson"}, {"section", {{"kind", "explanation"}, {"title", "观察元素"},
                {"body", "在屏幕上找 complementB。"}, {"helpType", "hint"}}}}}});
        const std::string message = hintCalls == 1 ? "先看内部结果 aSubsetB。" : "试着在两个集合中分别找到元素 2。";
        return output({{"message", message}, {"teaching", {{"helpType", "hint"}}}, {"actions", actions}, {"state", "waiting_student"}}, sink);
    });
    gangyi::registerAgentLessonTools(hints);
    hints.process(db, hintTask.at("id"));
    const auto visibleHint = gangyi::agentView(db, access, hintTask.at("id"), false);
    const auto published = visibleHint.dump();
    assert(hintCalls == 3 && visibleHint.at("status") == "waiting_student");
    assert(published.find("aSubsetB") == std::string::npos && published.find("complementB") == std::string::npos);
    assert(visibleHint.at("message") == "试着在两个集合中分别找到元素 2。");
    assert(shown().at("sections") == beforeHint.at("sections") && shown().at("evaluations") == all);
}

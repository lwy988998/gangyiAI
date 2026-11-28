#ifdef NDEBUG
#undef NDEBUG
#endif

#include "learning_state.hpp"

#include <cassert>
#include <iostream>
#include <limits>

using Json = nlohmann::json;

int main() {
    const Json evaluation = {
        {"id", "evaluation-1"}, {"interactionId", "input-1"}, {"sectionId", "section-1"},
        {"questionId", "question-1"}, {"response", "分母相加得到5"}, {"score", 20}, {"credible", true},
        {"assisted", false}, {"helpMode", "independent"}, {"helpHistory", Json::array()},
        {"diagnosis", {{"status", "supported"}, {"needsClarification", false}, {"evidence", {"分母相加"}}}},
    };
    const Json lesson = {
        {"id", "lesson-1"}, {"courseId", "course-1"}, {"topicId", "topic-1"},
        {"topicTitle", "分数加法"}, {"purpose", "理解通分"}, {"status", "ready"}, {"version", 4},
        {"teachingFocus", {{"practiceSectionId", "section-1"}, {"sectionId", "other-lesson-section"}, {"answer", "SECRET"}}},
        {"sections", {{{"id", "section-1"}, {"kind", "question"}, {"questionId", "question-1"},
                        {"question", {{"expectedAnswer", "SECRET"}}}}}},
        {"evaluations", {evaluation}}, {"privateField", "SECRET"},
    };
    auto state = gangyi::learningStateProjection(lesson);
    assert(state["evidence"]["distinctQuestionCount"] == 1);
    assert(state["evidence"]["independentQuestionCount"] == 1);
    assert(state["mastery"]["status"] == "UNKNOWN" && state["mastery"]["assessed"] == false);
    assert(!state["teaching"]["focus"].contains("sectionId"));
    assert(state.dump().find("SECRET") == std::string::npos);
    assert(state["subject"].is_null() && state["nextTeachingAction"].is_null());
    assert(!state["interactionState"]["availableInProjection"].get<bool>());

    // 同题多轮去重；分数为零仍然是实际作答，不被伪装成未作答。
    auto repeated = lesson;
    auto second = evaluation;
    second["id"] = "evaluation-2"; second["score"] = 0;
    repeated["evaluations"].push_back(second);
    state = gangyi::learningStateProjection(repeated);
    assert(state["evidence"]["evaluationIds"].size() == 2);
    assert(state["evidence"]["distinctQuestionCount"] == 1);
    repeated["evaluations"].push_back(evaluation);
    assert(gangyi::learningStateProjection(repeated)["evidence"]["evaluationIds"].size() == 2);

    // 任何不可靠绑定、分数、澄清或伪造证据都不能进入正式证据引用。
    for (int kind = 0; kind < 9; ++kind) {
        auto invalid = lesson;
        auto& item = invalid["evaluations"][0];
        if (kind == 0) item["questionId"] = "another-question";
        if (kind == 1) item["sectionId"] = "other-section";
        if (kind == 2) item["diagnosis"]["evidence"] = {"从未说过"};
        if (kind == 3) item["diagnosis"]["needsClarification"] = true;
        if (kind == 4) item["credible"] = false;
        if (kind == 5) item["score"] = nullptr;
        if (kind == 6) item["score"] = true;
        if (kind == 7) item["response"] = "  ";
        if (kind == 8) item["score"] = std::numeric_limits<unsigned long long>::max();
        state = gangyi::learningStateProjection(invalid);
        assert(state["evidence"]["evaluationIds"].empty());
        assert(state["mastery"]["status"] == "UNKNOWN");
    }

    for (int kind = 0; kind < 5; ++kind) {
        auto helped = lesson;
        auto& item = helped["evaluations"][0];
        if (kind == 0) item["assisted"] = true;
        if (kind == 1) item["helpHistory"] = {{{"helpType", "worked_example"}}};
        if (kind == 2) item.erase("helpMode");
        if (kind == 3) item.erase("helpHistory");
        if (kind == 4) item["helpHistory"] = {{{"helpType", "unrecognized"}}};
        state = gangyi::learningStateProjection(helped);
        assert(state["evidence"]["evaluationIds"].size() == 1);
        assert(state["evidence"]["independentQuestionCount"] == 0);
    }
    auto empty = lesson;
    empty["evaluations"] = Json::array();
    empty["historicalAssessments"] = {{{"score", 100}}};
    assert(gangyi::learningStateProjection(empty)["evidence"]["distinctQuestionCount"] == 0);
    bool rejected = false;
    try { gangyi::learningStateProjection(Json::object()); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    assert(gangyi::learningStateProjection(lesson) == gangyi::learningStateProjection(lesson));
    auto active = lesson;
    active["taskId"] = "older-dialog"; active["taskStatus"] = "ready";
    active["teachingTaskId"] = "active-teaching"; active["teachingTaskStatus"] = "paused";
    assert(gangyi::learningStateProjection(active)["teaching"]["taskId"] == "active-teaching");
    assert(gangyi::learningStateProjection(active)["teaching"]["taskStatus"] == "paused");
    // 八态只接受真实正式证据；没有证据、同题重做或受帮助回答不能冒充独立掌握。
    auto learning = lesson;
    learning["entered"] = true;
    learning["sections"][0]["question"] = {{"question", "1/2+1/3=?"}};
    Json proposal = {{"status", "PARTIAL"}, {"reason", "已有真实回答但分数单位不清楚"},
        {"uncertainty", "只有一次回答，尚不能推断长期理解"}, {"evidenceIds", {"evaluation-1"}}, {"nextAction", "explain"}};
    const auto rejects = [&](const Json& source, const Json& value) {
        bool denied = false;
        try { gangyi::checkedLearningMastery(source, value); } catch (const std::exception&) { denied = true; }
        assert(denied);
    };
    for (const auto* status : {"UNKNOWN", "LEARNING", "PARTIAL", "MISUNDERSTOOD", "PRACTICING", "PROVISIONAL_MASTERY"}) {
        proposal["status"] = status;
        assert(gangyi::checkedLearningMastery(learning, proposal)["status"] == status);
    }
    proposal["status"] = "VERIFIED_MASTERY";
    rejects(learning, proposal);
    auto multiple = learning;
    auto another = evaluation;
    another["id"] = "evaluation-2"; another["questionId"] = "question-2"; another["sectionId"] = "section-2";
    multiple["evaluations"].push_back(another);
    auto question = learning["sections"][0]; question["id"] = "section-2"; question["questionId"] = "question-2";
    multiple["sections"].push_back(question);
    proposal["evidenceIds"].push_back("evaluation-2");
    rejects(multiple, proposal);

    multiple["sections"][0]["question"] = {{"question", "选择 1/2+1/3 的结果"}, {"options", {"5/6", "2/5"}}};
    auto firstQuestion = multiple["sections"][0]["question"];
    auto reordered = firstQuestion;
    reordered["question"] = " 选择\n1/2+1/3 的结果 ";
    reordered["options"] = {"2/5", " 5/6 "};
    multiple["sections"][1]["question"] = reordered;
    state = gangyi::learningStateProjection(multiple);
    assert(state["evidence"]["evaluationIds"].size() == 2);
    assert(state["evidence"]["distinctQuestionCount"] == 1 && state["evidence"]["independentQuestionCount"] == 1);
    rejects(multiple, proposal);
    assert(gangyi::teachingQuestionIdentity("course-1", firstQuestion) != gangyi::teachingQuestionIdentity("course-1", reordered));
    const Json shownExample = {{"demonstratedQuestions", {firstQuestion}}};
    assert(gangyi::matchesDemonstration(shownExample, "course-1", reordered));
    reordered["materials"] = {{{"text", "第一项改用1/4"}}};
    assert(!gangyi::matchesDemonstration(shownExample, "course-1", reordered));
    multiple["sections"][1]["question"] = reordered;
    assert(gangyi::checkedLearningMastery(multiple, proposal)["independentQuestionCount"] == 2);

    multiple["sections"][1]["question"]["question"] = "1/4+1/6=?";
    assert(gangyi::checkedLearningMastery(multiple, proposal)["status"] == "VERIFIED_MASTERY");
    multiple["evaluations"][1]["helpMode"] = "after_example";
    rejects(multiple, proposal);
    proposal["status"] = "PARTIAL"; proposal["evidenceIds"] = {"fake"};
    rejects(learning, proposal);
    proposal["evidenceIds"] = Json::array();
    rejects(learning, proposal);
    proposal["status"] = "NOT_STARTED";
    rejects(learning, proposal);
    auto notStarted = learning;
    notStarted["entered"] = false; notStarted["evaluations"] = Json::array();
    assert(gangyi::checkedLearningMastery(notStarted, proposal)["status"] == "NOT_STARTED");
    proposal["score"] = 100;
    rejects(notStarted, proposal);
    const auto basis = gangyi::learningStateBasis(learning);
    auto changed = learning; changed["evaluations"][0]["score"] = 100;
    assert(gangyi::learningStateBasis(changed) != basis);
    changed = learning; changed["sections"][0]["question"]["question"] = "另一题";
    assert(gangyi::learningStateBasis(changed) != basis);
    std::cout << "学习状态的身份、证据、帮助与私有答案边界检查通过\n";
}

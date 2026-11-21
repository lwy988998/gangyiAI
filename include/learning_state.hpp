#pragma once

#include <nlohmann/json.hpp>
#include "teaching_preferences.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

namespace gangyi {

// 只接受已经过课时权限检查的公开视图；不读取数据库，也不执行模型判断。
inline nlohmann::json learningStateProjection(const nlohmann::json& lesson) {
    using Json = nlohmann::json;
    const auto nonblank = [](const Json& value) {
        return value.is_string() && value.get_ref<const std::string&>().find_first_not_of(" \t\r\n") != std::string::npos;
    };
    const auto text = [&](const Json& object, const char* key) -> Json {
        const auto found = object.find(key);
        return found != object.end() && nonblank(*found) ? *found : Json();
    };
    if (!lesson.is_object() || !nonblank(lesson.value("id", Json())) ||
        !nonblank(lesson.value("courseId", Json())))
        throw std::invalid_argument("学习状态缺少已授权的课时与课程身份");

    Json result = {
        {"schemaVersion", 1}, {"protocol", "learning-state-v1"},
        {"courseId", lesson.at("courseId")}, {"lessonId", lesson.at("id")},
        {"subject", text(lesson, "subject")}, {"objective", text(lesson, "purpose")},
        {"knowledgePoint", {{"id", text(lesson, "topicId")}, {"title", text(lesson, "topicTitle")}}},
        {"lessonVersion", lesson.value("version", Json())},
        {"teaching", {{"lessonStatus", text(lesson, "status")}, {"focus", Json::object()},
                       {"preferences", teachingPreferencesProjection(lesson)},
                       {"taskId", lesson.contains("teachingTaskId") ? text(lesson, "teachingTaskId") : text(lesson, "taskId")},
                       {"taskStatus", lesson.contains("teachingTaskStatus") ? text(lesson, "teachingTaskStatus") : text(lesson, "taskStatus")}}},
        {"evidence", {{"evaluationIds", Json::array()}, {"distinctQuestionCount", 0},
                       {"independentEvaluationIds", Json::array()},
                       {"independentQuestionCount", 0}, {"assistedQuestionCount", 0},
                       {"excludedEvaluationCount", 0}}},
        {"mastery", {{"status", "UNKNOWN"}, {"assessed", false},
                      {"reason", "掌握状态尚未独立评估；正式作答引用不自动等于掌握。"}}},
        {"interactionState", {{"availableInProjection", false},
                               {"reason", "本视图未读取或恢复组件操作记录。"}}},
        {"nextTeachingAction", nullptr},
    };
    // 焦点仅复制当前课时中存在的板块，拒绝携带额外字段或私有内容。
    std::map<std::string, std::string> questions;
    std::set<std::string> sections;
    const auto items = lesson.find("sections");
    if (items != lesson.end() && items->is_array()) for (const auto& section : *items) {
        if (!section.is_object() || !nonblank(section.value("id", Json()))) continue;
        const auto id = section.at("id").get<std::string>();
        sections.insert(id);
        if (section.value("kind", "") == "question" && nonblank(section.value("questionId", Json())))
            questions[id] = section.at("questionId").get<std::string>();
    }
    const auto focus = lesson.find("teachingFocus");
    if (focus != lesson.end() && focus->is_object()) {
        for (const auto* key : {"sectionId", "interactionSectionId", "practiceSectionId", "responseSectionId"}) {
            const auto id = text(*focus, key);
            if (id.is_string() && sections.count(id.get<std::string>())) result["teaching"]["focus"][key] = id;
        }
    }

    std::set<std::string> ids, distinct, independent, assisted;
    const auto evaluations = lesson.find("evaluations");
    if (evaluations != lesson.end() && evaluations->is_array()) for (const auto& evaluation : *evaluations) {
        const auto supported = [&]() {
            if (!evaluation.is_object() || !nonblank(evaluation.value("id", Json())) ||
                !nonblank(evaluation.value("interactionId", Json())) ||
                !nonblank(evaluation.value("sectionId", Json())) ||
                !nonblank(evaluation.value("questionId", Json())) ||
                !nonblank(evaluation.value("response", Json())) ||
                evaluation.value("credible", Json()) != Json(true)) return false;
            const auto section = questions.find(evaluation.at("sectionId").get<std::string>());
            if (section == questions.end() || section->second != evaluation.at("questionId")) return false;
            const auto score = evaluation.find("score");
            if (score == evaluation.end() || !score->is_number_integer()) return false;
            if (score->is_number_unsigned()) {
                if (score->get<std::uint64_t>() > 100) return false;
            } else if (score->get<std::int64_t>() < 0 || score->get<std::int64_t>() > 100) return false;
            const auto diagnosis = evaluation.find("diagnosis");
            if (diagnosis == evaluation.end() || !diagnosis->is_object() ||
                diagnosis->value("status", "") != "supported" ||
                diagnosis->value("needsClarification", Json()) != Json(false)) return false;
            const auto quotes = diagnosis->find("evidence");
            if (quotes == diagnosis->end() || !quotes->is_array() || quotes->empty()) return false;
            for (const auto& quote : *quotes) {
                if (!nonblank(quote) || evaluation.at("response").get_ref<const std::string&>().find(quote.get<std::string>()) == std::string::npos)
                    return false;
            }
            return true;
        };
        if (!supported() || !ids.insert(evaluation.at("id").get<std::string>()).second) {
            result["evidence"]["excludedEvaluationCount"] = result["evidence"]["excludedEvaluationCount"].get<int>() + 1;
            continue;
        }
        const auto question = evaluation.at("questionId").get<std::string>();
        distinct.insert(question);
        result["evidence"]["evaluationIds"].push_back(evaluation.at("id"));
        // 帮助状态不完整时保留正式作答引用，但不擅自计入独立证据。
        bool hasHelp = evaluation.value("assisted", Json()) == Json(true);
        bool helpKnown = evaluation.value("assisted", Json()).is_boolean();
        const auto history = evaluation.find("helpHistory");
        if (history == evaluation.end() || !history->is_array()) helpKnown = false;
        else for (const auto& help : *history) {
            if (!help.is_object()) { helpKnown = false; continue; }
            const auto type = help.value("helpType", "unknown");
            hasHelp = hasHelp || type == "hint" || type == "worked_example";
            if ((type != "none" && type != "explanation" && type != "clarification" &&
                 type != "hint" && type != "worked_example") ||
                help.value("partial", Json()) == Json(true)) helpKnown = false;
        }
        if (hasHelp) assisted.insert(question);
        if (helpKnown && !hasHelp && evaluation.value("helpMode", "") == "independent") {
            independent.insert(question);
            result["evidence"]["independentEvaluationIds"].push_back(evaluation.at("id"));
        }
    }
    result["evidence"]["distinctQuestionCount"] = distinct.size();
    result["evidence"]["independentQuestionCount"] = independent.size();
    result["evidence"]["assistedQuestionCount"] = assisted.size();
    return result;
}

// 构造审核依据的确定性快照；由调用方散列后存储，不向模型信任自报的证据版本。
inline nlohmann::json learningStateBasis(const nlohmann::json& lesson) {
    using Json = nlohmann::json;
    Json sections = Json::array();
    for (const auto& section : lesson.value("sections", Json::array())) {
        Json item;
        for (const auto* key : {"id", "kind", "questionId", "question", "version"})
            if (section.contains(key)) item[key] = section.at(key);
        sections.push_back(std::move(item));
    }
    return {{"courseId", lesson.at("courseId")}, {"lessonId", lesson.at("id")},
            {"topicId", lesson.value("topicId", Json())}, {"entered", lesson.value("entered", false)},
            {"sections", sections}, {"evaluations", lesson.value("evaluations", Json::array())}};
}

inline nlohmann::json checkedLearningMastery(const nlohmann::json& lesson,
                                            const nlohmann::json& proposal) {
    using Json = nlohmann::json;
    const auto validText = [](const Json& value) {
        return value.is_string() && value.get_ref<const std::string&>().size() <= 4000 &&
            value.get_ref<const std::string&>().find_first_not_of(" \t\r\n") != std::string::npos;
    };
    const auto projected = learningStateProjection(lesson);
    const std::set<std::string> statuses = {"UNKNOWN", "NOT_STARTED", "LEARNING", "PARTIAL",
        "MISUNDERSTOOD", "PRACTICING", "PROVISIONAL_MASTERY", "VERIFIED_MASTERY"};
    if (!proposal.is_object() || !validText(proposal.value("status", Json())) ||
        !statuses.count(proposal.at("status").get<std::string>()) ||
        !validText(proposal.value("reason", Json())) || !validText(proposal.value("uncertainty", Json())) ||
        !proposal.value("evidenceIds", Json()).is_array() || proposal.at("evidenceIds").size() > 200)
        throw std::invalid_argument("掌握状态须含合法status、具体reason、uncertainty与真实evidenceIds");
    const std::set<std::string> fields = {"lessonId", "status", "reason", "uncertainty", "evidenceIds", "nextAction"};
    for (const auto& item : proposal.items()) if (!fields.count(item.key()))
        throw std::invalid_argument("理解判断包含未授权字段，不能直接覆盖成绩、身份或证据版本");
    const std::set<std::string> actions = {"explain", "ask", "hint", "practice", "review", "advance", "pause"};
    if (!validText(proposal.value("nextAction", Json())) || !actions.count(proposal.at("nextAction").get<std::string>()))
        throw std::invalid_argument("下一步教学建议类型无效");
    const auto& eligible = projected.at("evidence").at("evaluationIds");
    const auto& independent = projected.at("evidence").at("independentEvaluationIds");
    std::set<std::string> references, independentQuestions;
    for (const auto& id : proposal.at("evidenceIds")) {
        if (!id.is_string() || !references.insert(id.get<std::string>()).second ||
            std::find(eligible.begin(), eligible.end(), id) == eligible.end())
            throw std::invalid_argument("掌握状态只能引用本课已正式发布、可信且有原话依据的评价，不能重复引用");
        if (std::find(independent.begin(), independent.end(), id) == independent.end()) continue;
        // 用公开题干和选项去重，不能仅换questionId把同一题变成多次独立验证。
        for (const auto& evaluation : lesson.at("evaluations")) if (evaluation.at("id") == id)
            for (const auto& section : lesson.at("sections")) if (section.at("id") == evaluation.at("sectionId"))
                independentQuestions.insert(section.value("question", Json{{"questionId", section.value("questionId", "")}}).dump());
    }
    const auto status = proposal.at("status").get<std::string>();
    const bool learning = lesson.value("entered", false);
    if (status == "NOT_STARTED" && (learning || !eligible.empty() || !references.empty()))
        throw std::invalid_argument("已有学习或作答记录，不能宣称尚未学习");
    if (status == "LEARNING" && !learning) throw std::invalid_argument("课时尚未进入，不能凭空声明正在学习");
    if (status != "UNKNOWN" && status != "NOT_STARTED" && status != "LEARNING" && references.empty())
        throw std::invalid_argument("理解、误解与掌握判断必须有真实作答依据");
    if ((status == "PROVISIONAL_MASTERY" || status == "VERIFIED_MASTERY") && independentQuestions.empty())
        throw std::invalid_argument("提示、示范或帮助未知的回答不能单独证明初步掌握");
    if (status == "VERIFIED_MASTERY" && independentQuestions.size() < 2)
        throw std::invalid_argument("已验证掌握需要多次不同题目的独立验证，不能凭一次正确或同题重复认定");
    return {{"schemaVersion", 1}, {"status", status}, {"assessed", true}, {"reason", proposal.at("reason")},
            {"uncertainty", proposal.at("uncertainty")}, {"evidenceIds", proposal.at("evidenceIds")},
            {"independentQuestionCount", independentQuestions.size()}, {"nextAction", proposal.at("nextAction")},
            {"judgmentSource", "model_proposal_with_program_evidence_checks"}};
}

} // namespace gangyi

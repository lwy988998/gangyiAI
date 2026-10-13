#pragma once
#include "question_evidence.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace gangyi {
// 题目身份只使用学生看到的题干、选项和材料，校正私有标准不能增加独立证据。
inline nlohmann::json teachingMaterialSnapshot(const nlohmann::json& value) {
    using Json = nlohmann::json;
    if (value.is_array()) { Json result = Json::array(); for (const auto& item : value) result.push_back(teachingMaterialSnapshot(item)); return result; }
    if (!value.is_object()) return value;
    Json result = Json::object();
    for (const auto* key : {"type", "title", "text", "body", "latex", "formula", "imageUrl", "alt", "rows", "columns", "headers", "values", "cells", "table"})
        if (value.contains(key)) result[key] = teachingMaterialSnapshot(value.at(key));
    return result;
}
inline std::string teachingQuestionIdentity(const std::string& scope, const nlohmann::json& question) {
    nlohmann::json identity = {{"question", question.at("question")}};
    if (question.contains("options")) identity["options"] = question.at("options");
    if (question.contains("materials")) {
        const auto material = teachingMaterialSnapshot(question.at("materials"));
        if (!material.empty()) identity["materials"] = material;
    }
    return questionIdentity(scope, identity);
}
// 教法由模型选择；这里只校验记录结构，防止把讲过概念当成本题提示。
inline bool teachingHelpType(const std::string& type) {
    return type == "explanation" || type == "hint" || type == "worked_example" ||
           type == "clarification" || type == "none" || type == "unknown";
}
inline std::string answerHelpMode(const nlohmann::json& history) {
    bool hinted = false, unknown = false;
    for (const auto& item : history) {
        const auto type = item.value("helpType", "unknown");
        if (type == "worked_example") return "after_example";
        hinted = hinted || type == "hint";
        unknown = unknown || type == "unknown" || item.value("partial", false);
    }
    return unknown ? "help_unknown" : hinted ? "after_hint" : "independent";
}
inline nlohmann::json checkedDiagnosis(const nlohmann::json& args, const std::string& response) {
    using Json = nlohmann::json;
    if (!args.contains("diagnosis")) return {{"status", "unavailable"}};
    const auto& value = args.at("diagnosis");
    if (!value.is_object() || !value.value("summary", Json()).is_string() ||
        !value.value("evidence", Json()).is_array() || !value.value("teachingDirection", Json()).is_string() ||
        !value.value("needsClarification", Json()).is_boolean())
        throw std::invalid_argument("diagnosis 需要 summary、evidence 原话列表、teachingDirection 和 needsClarification");
    for (const auto& quote : value.at("evidence"))
        if (!quote.is_string() || quote.get<std::string>().empty() || response.find(quote.get<std::string>()) == std::string::npos)
            throw std::invalid_argument("diagnosis.evidence 必须逐字引用本次实际回答，不能编造依据");
    if (value.at("needsClarification").get<bool>() && (args.value("credible", false) || !args.at("score").is_null()))
        throw std::invalid_argument("需要澄清时不能形成可信评价或正式分数");
    if (value.at("evidence").empty() && args.value("credible", false))
        throw std::invalid_argument("缺少实际回答依据，不能建立可信诊断评价，请先澄清");
    Json result = {{"summary", value.at("summary")}, {"evidence", value.at("evidence")},
        {"teachingDirection", value.at("teachingDirection")}, {"needsClarification", value.at("needsClarification")}};
    result["status"] = value.at("evidence").empty() ? "uncertain" : "supported";
    return result;
}
}

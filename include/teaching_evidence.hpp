// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "question_evidence.hpp"，为后续实现提供类型或接口声明。
#include "question_evidence.hpp"
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 题目身份只使用学生看到的题干、选项和材料，校正私有标准不能增加独立证据。
// 【逐行批注】 定义函数 teachingMaterialSnapshot，执行该函数声明的职责。
inline nlohmann::json teachingMaterialSnapshot(const nlohmann::json& value) {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 【逐行批注】 判断条件 (value.is_array()) { Json result = Json::array(); for (const aut…，成立时执行对应分支。
    if (value.is_array()) { Json result = Json::array(); for (const auto& item : value) result.push_back(teachingMaterialSnapshot(item)); return result; }
    // 【逐行批注】 判断条件 (!value.is_object()) return value;，成立时执行对应分支。
    if (!value.is_object()) return value;
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = Json::object();
    // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"type", "title", "text", "body", "latex", "formula…。
    for (const auto* key : {"type", "title", "text", "body", "latex", "formula", "imageUrl", "alt", "rows", "columns", "headers", "values", "cells", "table"})
        // 【逐行批注】 判断条件 (value.contains(key)) result[key] = teachingMaterialSnapshot(val…，成立时执行对应分支。
        if (value.contains(key)) result[key] = teachingMaterialSnapshot(value.at(key));
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 teachingQuestionIdentity，执行该函数声明的职责。
inline std::string teachingQuestionIdentity(const std::string& scope, const nlohmann::json& question) {
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json identity，更新当前运行状态。
    nlohmann::json identity = {{"question", question.at("question")}};
    // 【逐行批注】 判断条件 (question.contains("options")) identity["options"] = question.at…，成立时执行对应分支。
    if (question.contains("options")) identity["options"] = question.at("options");
    // 【逐行批注】 判断条件 (question.contains("materials")) {，成立时执行对应分支。
    if (question.contains("materials")) {
        // 【逐行批注】 计算右侧结果并写入 auto material，更新当前运行状态。
        const auto material = teachingMaterialSnapshot(question.at("materials"));
        // 【逐行批注】 判断条件 (!material.empty()) identity["materials"] = material;，成立时执行对应分支。
        if (!material.empty()) identity["materials"] = material;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return questionIdentity(scope, identity);。
    return questionIdentity(scope, identity);
// 【逐行批注】 结束当前代码块。
}
inline std::string teachingQuestionEvidenceKey(const std::string& scope, nlohmann::json question) {
    // 仅用于证据去重。保留材料和数学符号，作答与评分继续绑定原题快照。
    const auto normalize = [](const std::string& text) {
        std::string result;
        bool space = false;
        for (const char character : text) {
            if (character == ' ' || character == '\t' || character == '\r' || character == '\n') {
                space = !result.empty();
                continue;
            }
            if (space) result.push_back(' ');
            result.push_back(character);
            space = false;
        }
        return result;
    };
    question["question"] = normalize(question.at("question").get_ref<const std::string&>());
    if (question.value("options", nlohmann::json()).is_array()) {
        auto& options = question.at("options");
        for (auto& option : options) if (option.is_string()) option = normalize(option.get_ref<const std::string&>());
        if (options.empty()) question.erase("options");
        else std::sort(options.begin(), options.end());
    }
    return teachingQuestionIdentity(scope, question);
}

// 完整示范可以使用尚未布置的另一题；只保存可见题干、选项和材料来关联以后的复做。
inline nlohmann::json checkedDemonstrations(const nlohmann::json& content) {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 【逐行批注】 计算右侧结果并写入 auto examples，更新当前运行状态。
    const auto examples = content.value("demonstratedQuestions", Json::array());
    // 【逐行批注】 判断条件 (!examples.is_array()) throw std::invalid_argument("demonstrated…，成立时执行对应分支。
    if (!examples.is_array()) throw std::invalid_argument("demonstratedQuestions 须为已完整示范的题目快照列表，没有示范时用空列表");
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = Json::array();
    // 【逐行批注】 按循环规则重复处理：for (const auto& example : examples) {。
    for (const auto& example : examples) {
        // 【逐行批注】 判断条件 (!example.is_object() || !example.value("question", Json()).is_s…，成立时执行对应分支。
        if (!example.is_object() || !example.value("question", Json()).is_string() ||
            // 【逐行批注】 执行当前声明或表达式：example.at("question").get<std::string>().find_first_not_of(" \t\r\n") == std::strin…。
            example.at("question").get<std::string>().find_first_not_of(" \t\r\n") == std::string::npos)
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("示范题快照须包含完整非空 question 题干，不能使用答案或题目标识代替");。
            throw std::invalid_argument("示范题快照须包含完整非空 question 题干，不能使用答案或题目标识代替");
        // 【逐行批注】 计算右侧结果并写入 Json snapshot，更新当前运行状态。
        Json snapshot = {{"question", example.at("question")}};
        // 【逐行批注】 判断条件 (example.contains("options")) {，成立时执行对应分支。
        if (example.contains("options")) {
            // 【逐行批注】 判断条件 (!example.at("options").is_array()) throw std::invalid_argument(…，成立时执行对应分支。
            if (!example.at("options").is_array()) throw std::invalid_argument("示范题 options 须为选项列表");
            // 【逐行批注】 按循环规则重复处理：for (const auto& option : example.at("options")) if (!option.is_string() |…。
            for (const auto& option : example.at("options")) if (!option.is_string() || option.get<std::string>().empty())
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("示范题选项须为非空文字");。
                throw std::invalid_argument("示范题选项须为非空文字");
            // 【逐行批注】 计算右侧结果并写入 snapshot["options"]，更新当前运行状态。
            snapshot["options"] = example.at("options");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (example.contains("materials")) snapshot["materials"] = teaching…，成立时执行对应分支。
        if (example.contains("materials")) snapshot["materials"] = teachingMaterialSnapshot(example.at("materials"));
        // 【逐行批注】 判断条件 (std::find(result.begin(), result.end(), snapshot) == result.end…，成立时执行对应分支。
        if (std::find(result.begin(), result.end(), snapshot) == result.end()) result.push_back(std::move(snapshot));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 matchesDemonstration，执行该函数声明的职责。
inline bool matchesDemonstration(const nlohmann::json& content, const std::string& course, const nlohmann::json& question) {
    // 【逐行批注】 计算右侧结果并写入 auto examples，更新当前运行状态。
    const auto examples = content.value("demonstratedQuestions", nlohmann::json::array());
    // 【逐行批注】 判断条件 (!examples.is_array()) return false;，成立时执行对应分支。
    if (!examples.is_array()) return false;
    // 【逐行批注】 按循环规则重复处理：for (const auto& example : examples) if (example.is_object() && example.va…。
    for (const auto& example : examples) if (example.is_object() && example.value("question", nlohmann::json()).is_string() &&
        teachingQuestionEvidenceKey(course, example) == teachingQuestionEvidenceKey(course, question)) return true;
    // 【逐行批注】 结束当前函数并返回结果：return false;。
    return false;
// 【逐行批注】 结束当前代码块。
}
// 只串联评价卡片公开的文字，不计入未展示的补讲方向或私有评分标准。
// 【逐行批注】 定义函数 evaluationFeedbackText，执行该函数声明的职责。
inline std::string evaluationFeedbackText(const nlohmann::json& value) {
    // 【逐行批注】 计算右侧结果并写入 std::string result，更新当前运行状态。
    std::string result = value.value("feedback", "");
    // 【逐行批注】 计算右侧结果并写入 auto append，更新当前运行状态。
    const auto append = [&](const std::string& text) { if (!text.empty()) { if (!result.empty()) result += "\n"; result += text; } };
    // 【逐行批注】 调用 append 完成本行所需操作：append(value.value("uncertainty", ""));。
    append(value.value("uncertainty", ""));
    // 【逐行批注】 计算右侧结果并写入 auto diagnosis，更新当前运行状态。
    const auto diagnosis = value.value("diagnosis", nlohmann::json::object());
    // 【逐行批注】 调用 append 完成本行所需操作：append(diagnosis.value("summary", ""));。
    append(diagnosis.value("summary", ""));
    // 【逐行批注】 按循环规则重复处理：for (const auto& quote : diagnosis.value("evidence", nlohmann::json::array…。
    for (const auto& quote : diagnosis.value("evidence", nlohmann::json::array())) if (quote.is_string()) append(quote.get<std::string>());
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 教法由模型选择；这里只校验记录结构，防止把讲过概念当成本题提示。
// 【逐行批注】 定义函数 teachingHelpType，执行该函数声明的职责。
inline bool teachingHelpType(const std::string& type) {
    // 【逐行批注】 结束当前函数并返回结果：return type == "explanation" || type == "hint" || type == "worked_exa…。
    return type == "explanation" || type == "hint" || type == "worked_example" ||
           // 【逐行批注】 执行当前声明或表达式：type == "clarification" || type == "none" || type == "unknown";。
           type == "clarification" || type == "none" || type == "unknown";
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 answerHelpMode，执行该函数声明的职责。
inline std::string answerHelpMode(const nlohmann::json& history) {
    // 【逐行批注】 计算右侧结果并写入 bool hinted，更新当前运行状态。
    bool hinted = false, unknown = false;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : history) {。
    for (const auto& item : history) {
        // 【逐行批注】 计算右侧结果并写入 auto type，更新当前运行状态。
        const auto type = item.value("helpType", "unknown");
        // 【逐行批注】 判断条件 (type == "worked_example") return "after_example";，成立时执行对应分支。
        if (type == "worked_example") return "after_example";
        // 【逐行批注】 计算右侧结果并写入 hinted，更新当前运行状态。
        hinted = hinted || type == "hint";
        // 【逐行批注】 计算右侧结果并写入 unknown，更新当前运行状态。
        unknown = unknown || type == "unknown" || item.value("partial", false);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return unknown ? "help_unknown" : hinted ? "after_hint" : "independen…。
    return unknown ? "help_unknown" : hinted ? "after_hint" : "independent";
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 checkedDiagnosis，执行该函数声明的职责。
inline nlohmann::json checkedDiagnosis(const nlohmann::json& args, const std::string& response) {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 【逐行批注】 判断条件 (!args.contains("diagnosis")) return {{"status", "unavailable"}};，成立时执行对应分支。
    if (!args.contains("diagnosis")) return {{"status", "unavailable"}};
    // 【逐行批注】 计算右侧结果并写入 auto& value，更新当前运行状态。
    const auto& value = args.at("diagnosis");
    // 【逐行批注】 判断条件 (!value.is_object() || !value.value("summary", Json()).is_string…，成立时执行对应分支。
    if (!value.is_object() || !value.value("summary", Json()).is_string() ||
        // 【逐行批注】 调用 Json 完成本行所需操作：!value.value("evidence", Json()).is_array() || !value.value("teaching…。
        !value.value("evidence", Json()).is_array() || !value.value("teachingDirection", Json()).is_string() ||
        // 【逐行批注】 调用 Json 完成本行所需操作：!value.value("needsClarification", Json()).is_boolean())。
        !value.value("needsClarification", Json()).is_boolean())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("diagnosis 需要 summary、evidence 原话列表、teach…。
        throw std::invalid_argument("diagnosis 需要 summary、evidence 原话列表、teachingDirection 和 needsClarification");
    // 【逐行批注】 判断条件 (value.at("summary").get<std::string>().find_first_not_of(" \t\r…，成立时执行对应分支。
    if (value.at("summary").get<std::string>().find_first_not_of(" \t\r\n") == std::string::npos ||
        // 【逐行批注】 执行当前声明或表达式：value.at("teachingDirection").get<std::string>().find_first_not_of(" \t\r\n") == std…。
        value.at("teachingDirection").get<std::string>().find_first_not_of(" \t\r\n") == std::string::npos)
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("diagnosis 的卡点说明和补讲方向不能为空，请提供具体诊断或先澄清");。
        throw std::invalid_argument("diagnosis 的卡点说明和补讲方向不能为空，请提供具体诊断或先澄清");
    // 【逐行批注】 按循环规则重复处理：for (const auto& quote : value.at("evidence"))。
    for (const auto& quote : value.at("evidence"))
        // 【逐行批注】 判断条件 (!quote.is_string() || quote.get<std::string>().empty() || respo…，成立时执行对应分支。
        if (!quote.is_string() || quote.get<std::string>().empty() || response.find(quote.get<std::string>()) == std::string::npos)
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("diagnosis.evidence 必须逐字引用本次实际回答，不能编造依据");。
            throw std::invalid_argument("diagnosis.evidence 必须逐字引用本次实际回答，不能编造依据");
    // 【逐行批注】 判断条件 (value.at("needsClarification").get<bool>() && (args.value("cred…，成立时执行对应分支。
    if (value.at("needsClarification").get<bool>() && (args.value("credible", false) || !args.at("score").is_null()))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("需要澄清时不能形成可信评价或正式分数");。
        throw std::invalid_argument("需要澄清时不能形成可信评价或正式分数");
    // 【逐行批注】 判断条件 (value.at("evidence").empty() && args.value("credible", false))，成立时执行对应分支。
    if (value.at("evidence").empty() && args.value("credible", false))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("缺少实际回答依据，不能建立可信诊断评价，请先澄清");。
        throw std::invalid_argument("缺少实际回答依据，不能建立可信诊断评价，请先澄清");
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = {{"summary", value.at("summary")}, {"evidence", value.at("evidence")},
        // 【逐行批注】 执行当前声明或表达式：{"teachingDirection", value.at("teachingDirection")}, {"needsClarification", value.a…。
        {"teachingDirection", value.at("teachingDirection")}, {"needsClarification", value.at("needsClarification")}};
    // 【逐行批注】 计算右侧结果并写入 result["status"]，更新当前运行状态。
    result["status"] = value.at("evidence").empty() ? "uncertain" : "supported";
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

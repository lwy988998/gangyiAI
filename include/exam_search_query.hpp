#pragma once

#include "exam_message_stream.hpp"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <string>
#include <vector>

namespace gangyi {

// 使用不同清晰题目的片段检索，避免将未识别完整的残题当作匹配线索。
inline std::vector<std::string> examSearchQueries(const nlohmann::json& prompt) {
    std::vector<std::string> result;
    const auto questions = prompt.value("questions", nlohmann::json::array());
    if (questions.is_array()) for (const auto& question : questions) {
        if (!question.is_object() || question.value("recognitionStatus", "") == "uncertain" ||
            !question.value("stem", nlohmann::json()).is_string()) continue;
        const auto stem = question.at("stem").get<std::string>();
        if (stem.size() < 24) continue;
        const auto query = examUtf8Prefix(stem, 180) + " 试卷 参考答案";
        if (std::find(result.begin(), result.end(), query) == result.end()) result.push_back(query);
        if (result.size() == 2) break;
    }
    if (result.empty()) {
        const auto title = prompt.value("title", std::string{});
        if (!title.empty()) result.push_back(examUtf8Prefix(title, 180) + " 试卷 参考答案");
    }
    return result;
}

} // namespace gangyi

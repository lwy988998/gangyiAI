#pragma once
#include <nlohmann/json.hpp>
#include <cstdint>
#include <sstream>
#include <string>

namespace gangyi {
// 仅保存评估所需的题目字段；题目变更时生成新的身份，重复练习不增加独立题目数。
inline nlohmann::json questionSnapshot(const nlohmann::json& question) {
    nlohmann::json snapshot = nlohmann::json::object();
    for (const char* key : {"question", "options", "answerIndex", "rubric", "type", "difficulty"})
        if (question.contains(key)) snapshot[key] = question[key];
    return snapshot;
}
inline std::string questionIdentity(const std::string& scope, const nlohmann::json& snapshot) {
    auto identity = snapshot;
    // 答案校正和难度描述变更不应把同一道原题算成新的独立证据。
    identity.erase("answerIndex");
    identity.erase("difficulty");
    std::uint64_t digest = 14695981039346656037ULL;
    for (const unsigned char byte : scope + ":" + identity.dump()) { digest ^= byte; digest *= 1099511628211ULL; }
    std::ostringstream output;
    output << "q-" << std::hex << digest;
    return output.str();
}
}

#pragma once

#include <nlohmann/json.hpp>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace gangyi {

class StructuredReplyError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// 只去除包住整份回复的围栏。正文、额外对象或截断内容均须通过严格解析。
inline nlohmann::json parseStructuredReply(const std::string& reply, std::size_t& prefixBytes) {
    prefixBytes = 0;
    if (reply.find('\0') != std::string::npos)
        throw StructuredReplyError("AI 回复包含无效的空字节");

    // 同一对象中的重复键会覆盖先前的值，可能改变已读到的操作或评分结果。
    using Json = nlohmann::json;
    std::vector<std::unordered_set<std::string>> objectKeys;
    const auto checkKey = [&](int, Json::parse_event_t event, Json& value) {
        if (event == Json::parse_event_t::object_start) objectKeys.emplace_back();
        if (event == Json::parse_event_t::object_end) objectKeys.pop_back();
        if (event == Json::parse_event_t::key && !objectKeys.back().insert(value.get<std::string>()).second)
            throw StructuredReplyError("AI 回复在同一个对象中重复使用字段");
        return true;
    };
    const auto first = reply.find_first_not_of(" \t\r\n");
    if (first != std::string::npos && reply.compare(first, 3, "```") == 0) {
        const auto lineEnd = reply.find('\n', first);
        const auto last = reply.find_last_not_of(" \t\r\n");
        if (lineEnd != std::string::npos && last >= lineEnd + 3 &&
            reply.compare(last - 2, 3, "```") == 0 && reply[last - 3] == '\n') {
            auto opening = reply.substr(first, lineEnd - first);
            if (!opening.empty() && opening.back() == '\r') opening.pop_back();
            if (opening == "```json" || opening == "```") {
                prefixBytes = lineEnd + 1;
                return Json::parse(reply.begin() + prefixBytes, reply.begin() + last - 2, checkKey);
            }
        }
    }
    return Json::parse(reply, checkKey);
}

// 旧调用方保留既有的错误分类，与主控共用解析规则。
nlohmann::json parseAIJson(const std::string& content);

inline nlohmann::json parseStructuredReply(const std::string& reply) {
    std::size_t prefixBytes = 0;
    return parseStructuredReply(reply, prefixBytes);
}

} // namespace gangyi

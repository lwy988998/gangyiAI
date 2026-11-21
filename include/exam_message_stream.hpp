#pragma once

#include <nlohmann/json.hpp>
#include <cstddef>
#include <string>
#include <vector>

namespace gangyi {

inline std::string examUtf8Prefix(const std::string& value, std::size_t maxBytes) {
    if (value.size() <= maxBytes) return value;
    std::size_t end = maxBytes;
    while (end > 0 && (static_cast<unsigned char>(value[end]) & 0xc0) == 0x80) --end;
    return value.substr(0, end);
}

// 仅修补一个嵌套对象或数组缺失的闭合符；不截断正文、不补全结尾。
inline nlohmann::json parseExamModelJson(const std::string& input) {
    auto parsed = nlohmann::json::parse(input, nullptr, false);
    if (!parsed.is_discarded()) return parsed;
    const auto first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos || input[first] != '{') return parsed;
    std::vector<char> stack;
    std::string repaired;
    repaired.reserve(input.size() + 1);
    bool inString = false, escaped = false, inserted = false;
    for (const char ch : input) {
        if (inString) {
            repaired += ch;
            if (escaped) escaped = false;
            else if (ch == '\\') escaped = true;
            else if (ch == '"') inString = false;
            continue;
        }
        if (ch == '"') inString = true;
        else if (ch == '{' || ch == '[') stack.push_back(ch == '{' ? '}' : ']');
        else if (ch == '}' || ch == ']') {
            if (stack.empty()) return parsed;
            if (stack.back() != ch) {
                if (inserted || stack.size() < 2 || stack[stack.size() - 2] != ch) return parsed;
                repaired += stack.back();
                stack.pop_back();
                inserted = true;
            }
            stack.pop_back();
        }
        repaired += ch;
    }
    if (inString || !stack.empty() || !inserted) return parsed;
    return nlohmann::json::parse(repaired, nullptr, false);
}

// 只增量展示首个 message 字段；JSON 不完整时不泄露其他字段或原始转义。
inline std::string examMessageStreamText(const std::string& input) {
    std::size_t pos = 0;
    const auto skip = [&] { while (pos < input.size() && (input[pos] == ' ' || input[pos] == '\n' || input[pos] == '\r' || input[pos] == '\t')) ++pos; };
    skip();
    if (pos >= input.size() || input[pos++] != '{') return {};
    skip();
    constexpr char key[] = "\"message\"";
    if (input.compare(pos, sizeof(key) - 1, key) != 0) return {};
    pos += sizeof(key) - 1;
    skip();
    if (pos >= input.size() || input[pos++] != ':') return {};
    skip();
    if (pos >= input.size() || input[pos++] != '"') return {};
    const auto begin = pos;
    bool escaped = false;
    for (; pos < input.size(); ++pos) {
        const char character = input[pos];
        if (!escaped && character == '"') break;
        if (!escaped && character == '\\') escaped = true;
        else escaped = false;
    }
    const auto length = pos - begin;
    // 分片可能截断 UTF-8 或 JSON 转义；只返回完整且合法的前缀。
    for (std::size_t trim = 0; trim <= 12 && trim <= length; ++trim) {
        const auto candidate = std::string("\"") + input.substr(begin, length - trim) + "\"";
        const auto parsed = nlohmann::json::parse(candidate, nullptr, false);
        if (parsed.is_string()) return parsed.get<std::string>();
    }
    return {};
}

}  // namespace gangyi

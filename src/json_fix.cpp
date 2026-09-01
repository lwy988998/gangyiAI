#include "json_fix.hpp"

#include "ai_client.hpp"

#include <algorithm>
#include <cctype>

namespace gangyi {
namespace {
std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string repair(std::string value) {
    value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
    // 全角引号 → 半角（CJK 模型常见）
    for (size_t i = 0; i < value.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(value[i]);
        if (c == 0xE2 && i + 2 < value.size()) {
            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            if (c1 == 0x80) {
                if (c2 == 0x9C) { value.replace(i, 3, "\""); }        // “
                else if (c2 == 0x9D) { value.replace(i, 3, "\""); }   // ”
                else if (c2 == 0x98) { value.replace(i, 3, "'"); }    // ‘
                else if (c2 == 0x99) { value.replace(i, 3, "'"); }    // ’
            }
        }
    }
    std::string result;
    bool inString = false;
    for (size_t i = 0; i < value.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(value[i]);
        if (c == '\n' && !inString) {
            size_t j = i + 1;
            while (j < value.size() && (value[j] == ' ' || value[j] == '\t')) ++j;
            if (value.compare(j, 2, "//") == 0) { i = value.find('\n', j); if (i == std::string::npos) break; }
            else result += '\n';
            continue;
        }
        if (c < 0x20 && c != '\n' && c != '\t') { result += ' '; continue; }
        if (c == '"') inString = !inString;
        result += value[i];
    }
    for (size_t pos = 0; (pos = result.find(',', pos)) != std::string::npos;) {
        size_t next = pos + 1; while (next < result.size() && std::isspace(static_cast<unsigned char>(result[next]))) ++next;
        if (next < result.size() && (result[next] == '}' || result[next] == ']')) result.erase(pos, next - pos);
        else ++pos;
    }
    return result;
}

std::string closeTruncated(const std::string& value) {
    std::vector<char> stack;
    bool string = false, escaped = false;
    for (char c : value) {
        if (string) { if (escaped) escaped = false; else if (c == '\\') escaped = true; else if (c == '"') string = false; continue; }
        if (c == '"') string = true;
        else if (c == '{') stack.push_back('}'); else if (c == '[') stack.push_back(']');
        else if ((c == '}' || c == ']') && !stack.empty()) stack.pop_back();
    }
    std::string result = value;
    if (string) {
        // 截断可能发生在 UTF-8 多字节字符中间（半个汉字）：补引号前先丢弃不完整的字符序列
        size_t i = result.size();
        int cont = 0;
        while (i > 0) {
            const unsigned char c = static_cast<unsigned char>(result[i - 1]);
            if (c >= 0x80 && c < 0xC0) { ++cont; --i; continue; }   // 续字节(10xxxxxx)
            if (c >= 0xC0) {                                         // 起始字节
                const int expected = c >= 0xF0 ? 3 : (c >= 0xE0 ? 2 : 1);
                if (cont < expected) result.erase(i - 1);            // 字符不完整：删除整个未完成序列
                break;
            }
            break;                                                   // ASCII：完整
        }
        result += '"';
    }
    while (!stack.empty()) { result += stack.back(); stack.pop_back(); }
    return result;
}
}

nlohmann::json parseAIJson(const std::string& content) {
    std::string value = content;
    if (value.size() >= 3 && static_cast<unsigned char>(value[0]) == 0xEF && static_cast<unsigned char>(value[1]) == 0xBB && static_cast<unsigned char>(value[2]) == 0xBF) value.erase(0, 3);
    value = trim(value);
    if (value.rfind("```", 0) == 0) {
        const auto start = value.find('\n');
        const auto end = value.rfind("```");
        if (start != std::string::npos && end > start) value = trim(value.substr(start + 1, end - start - 1));
    }
    const auto left = value.find('{'); const auto right = value.rfind('}');
    if (left != std::string::npos) value = value.substr(left, right == std::string::npos ? std::string::npos : right - left + 1);
    const std::string repaired = repair(value);
    for (const auto& candidate : {value, repaired, closeTruncated(repaired)}) {
        try { return nlohmann::json::parse(candidate); } catch (...) {}
    }
    int attempts = 0;
    for (size_t end = repaired.size(); end > 0 && attempts < 60; --end) {
        const size_t closing = repaired.rfind('}', end - 1);
        if (closing == std::string::npos) break;
        try { return nlohmann::json::parse(closeTruncated(repaired.substr(0, closing + 1))); } catch (...) {}
        end = closing + 1;
        attempts += 1;
    }
    throw AIClientError("json_parse_error", "unable to parse AI JSON");
}
}

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
    std::string result;
    result.reserve(value.size() + 8);
    bool inString = false, escaped = false;
    for (size_t i = 0; i < value.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(value[i]);
        // 全角引号 → 半角（CJK 模型常见）；只在字符串外替换分隔符，正文里的引号原样保留。
        if (!inString && c == 0xE2 && i + 2 < value.size() &&
            static_cast<unsigned char>(value[i + 1]) == 0x80) {
            const unsigned char tail = static_cast<unsigned char>(value[i + 2]);
            if (tail == 0x9C || tail == 0x9D) { result += '"'; i += 2; continue; }   // “ ”
            if (tail == 0x98 || tail == 0x99) { result += '\''; i += 2; continue; }  // ‘ ’
        }
        if (inString) {
            if (escaped) { escaped = false; result += value[i]; continue; }
            if (c == '\\') { escaped = true; result += value[i]; continue; }
            if (c == '"') { inString = false; result += value[i]; continue; }
            if (c == '\n') { result += "\\n"; continue; }
            if (c == '\t') { result += "\\t"; continue; }
            if (c < 0x20) { result += ' '; continue; }
            result += value[i];
            continue;
        }
        if (c == '"') { inString = true; result += value[i]; continue; }
        if (c == '\n') {
            size_t j = i + 1;
            while (j < value.size() && (value[j] == ' ' || value[j] == '\t')) ++j;
            if (value.compare(j, 2, "//") == 0) {
                const auto comment = value.find('\n', j);
                if (comment == std::string::npos) break;
                i = comment;
                continue;
            }
            result += '\n';
            continue;
        }
        if (c < 0x20 && c != '\t') { result += ' '; continue; }
        result += value[i];
    }
    // 去掉对象或数组结尾前多余的逗号，只在字符串外处理。
    std::string cleaned;
    cleaned.reserve(result.size());
    bool string = false, escape = false;
    for (size_t i = 0; i < result.size(); ++i) {
        const char c = result[i];
        if (string) {
            cleaned += c;
            if (escape) escape = false;
            else if (c == '\\') escape = true;
            else if (c == '"') string = false;
            continue;
        }
        if (c == '"') { string = true; cleaned += c; continue; }
        if (c == ',') {
            size_t next = i + 1;
            while (next < result.size() && (result[next] == ' ' || result[next] == '\t' || result[next] == '\n')) ++next;
            if (next < result.size() && (result[next] == '}' || result[next] == ']')) continue;
        }
        cleaned += c;
    }
    return cleaned;
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

std::string balanceBrackets(const std::string& value, bool insertMissing = true) {
    // 模型偶尔漏写或多写闭合括号。这里只按栈补齐缺失的 } 或 ]、丢弃多余的闭合符号，
    // 不添加、删除或改写任何实际内容；补全后的结果仍需通过主控结构校验。
    std::string result;
    result.reserve(value.size() + 8);
    std::vector<char> stack;
    bool string = false, escaped = false;
    for (char c : value) {
        if (string) {
            result += c;
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') string = false;
            continue;
        }
        if (c == '"') { string = true; result += c; continue; }
        if (c == '{') { stack.push_back('}'); result += c; continue; }
        if (c == '[') { stack.push_back(']'); result += c; continue; }
        if (c == '}' || c == ']') {
            if (insertMissing) while (!stack.empty() && stack.back() != c) { result += stack.back(); stack.pop_back(); }
            if (stack.empty()) continue;
            if (stack.back() != c) continue;  // 丢弃多余的闭合符号
            stack.pop_back();
            result += c;
            continue;
        }
        result += c;
    }
    if (string) result += '"';
    while (!stack.empty()) { result += stack.back(); stack.pop_back(); }
    return result;
}

std::vector<std::string> completeObjects(const std::string& value) {
    std::vector<std::string> objects;
    size_t start = std::string::npos;
    int depth = 0;
    bool inString = false, escaped = false;
    for (size_t index = 0; index < value.size(); ++index) {
        const char character = value[index];
        if (inString) {
            if (escaped) escaped = false;
            else if (character == '\\') escaped = true;
            else if (character == '"') inString = false;
            continue;
        }
        if (character == '"') inString = true;
        else if (character == '{') {
            if (depth++ == 0) start = index;
        } else if (character == '}' && depth > 0 && --depth == 0 && start != std::string::npos) {
            objects.push_back(value.substr(start, index - start + 1));
            start = std::string::npos;
        }
    }
    return objects;
}
}

nlohmann::json parseAIJson(const std::string& content) {
    std::string value = content;
    if (value.size() >= 3 && static_cast<unsigned char>(value[0]) == 0xEF && static_cast<unsigned char>(value[1]) == 0xBB && static_cast<unsigned char>(value[2]) == 0xBF) value.erase(0, 3);
    value = trim(value);
    try {
        const auto wrapped = nlohmann::json::parse(value);
        if (wrapped.is_string()) value = trim(wrapped.get<std::string>());
    } catch (...) {}
    if (value.rfind("```", 0) == 0) {
        const auto start = value.find('\n');
        const auto end = value.rfind("```");
        if (start != std::string::npos && end > start) value = trim(value.substr(start + 1, end - start - 1));
    }
    const auto left = value.find('{'); const auto right = value.rfind('}');
    if (left != std::string::npos) value = value.substr(left, right == std::string::npos ? std::string::npos : right - left + 1);
    const std::string repaired = repair(value);
    for (const auto& candidate : {value, balanceBrackets(value), balanceBrackets(value, false), repaired,
                                  balanceBrackets(repaired), balanceBrackets(repaired, false), closeTruncated(repaired)}) {
        try { return nlohmann::json::parse(candidate); } catch (...) {}
    }
    const auto objects = completeObjects(repaired);
    for (auto item = objects.rbegin(); item != objects.rend(); ++item) {
        try { return nlohmann::json::parse(repair(*item)); } catch (...) {}
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

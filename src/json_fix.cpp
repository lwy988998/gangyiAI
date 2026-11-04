// 【逐行批注】 引入头文件 "json_fix.hpp"，为后续实现提供类型或接口声明。
#include "json_fix.hpp"

// 【逐行批注】 引入头文件 "ai_client.hpp"，为后续实现提供类型或接口声明。
#include "ai_client.hpp"

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 定义函数 trim，执行该函数声明的职责。
std::string trim(std::string value) {
    // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
    const auto first = value.find_first_not_of(" \t\r\n");
    // 【逐行批注】 判断条件 (first == std::string::npos) return {};，成立时执行对应分支。
    if (first == std::string::npos) return {};
    // 【逐行批注】 计算右侧结果并写入 auto last，更新当前运行状态。
    const auto last = value.find_last_not_of(" \t\r\n");
    // 【逐行批注】 结束当前函数并返回结果：return value.substr(first, last - first + 1);。
    return value.substr(first, last - first + 1);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 repair，执行该函数声明的职责。
std::string repair(std::string value) {
    // 【逐行批注】 调用 remove 完成本行所需操作：value.erase(std::remove(value.begin(), value.end(), '\r'), value.end(…。
    value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
    // 【逐行批注】 执行当前声明或表达式：std::string result;。
    std::string result;
    // 【逐行批注】 执行当前声明或表达式：result.reserve(value.size() + 8);。
    result.reserve(value.size() + 8);
    // 【逐行批注】 计算右侧结果并写入 bool inString，更新当前运行状态。
    bool inString = false, escaped = false;
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < value.size(); ++i) {。
    for (size_t i = 0; i < value.size(); ++i) {
        // 【逐行批注】 计算右侧结果并写入 unsigned char c，更新当前运行状态。
        const unsigned char c = static_cast<unsigned char>(value[i]);
        // 全角引号 → 半角（CJK 模型常见）；只在字符串外替换分隔符，正文里的引号原样保留。
        // 【逐行批注】 判断条件 (!inString && c == 0xE2 && i + 2 < value.size() &&，成立时执行对应分支。
        if (!inString && c == 0xE2 && i + 2 < value.size() &&
            // 【逐行批注】 执行当前声明或表达式：static_cast<unsigned char>(value[i + 1]) == 0x80) {。
            static_cast<unsigned char>(value[i + 1]) == 0x80) {
            // 【逐行批注】 计算右侧结果并写入 unsigned char tail，更新当前运行状态。
            const unsigned char tail = static_cast<unsigned char>(value[i + 2]);
            // 【逐行批注】 判断条件 (tail == 0x9C || tail == 0x9D) { result += '"'; i += 2; continue…，成立时执行对应分支。
            if (tail == 0x9C || tail == 0x9D) { result += '"'; i += 2; continue; }   // “ ”
            // 【逐行批注】 判断条件 (tail == 0x98 || tail == 0x99) { result += '\''; i += 2; continu…，成立时执行对应分支。
            if (tail == 0x98 || tail == 0x99) { result += '\''; i += 2; continue; }  // ‘ ’
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (inString) {，成立时执行对应分支。
        if (inString) {
            // 【逐行批注】 判断条件 (escaped) { escaped = false; result += value[i]; continue; }，成立时执行对应分支。
            if (escaped) { escaped = false; result += value[i]; continue; }
            // 【逐行批注】 判断条件 (c == '\\') { escaped = true; result += value[i]; continue; }，成立时执行对应分支。
            if (c == '\\') { escaped = true; result += value[i]; continue; }
            // 【逐行批注】 判断条件 (c == '"') { inString = false; result += value[i]; continue; }，成立时执行对应分支。
            if (c == '"') { inString = false; result += value[i]; continue; }
            // 【逐行批注】 判断条件 (c == '\n') { result += "\\n"; continue; }，成立时执行对应分支。
            if (c == '\n') { result += "\\n"; continue; }
            // 【逐行批注】 判断条件 (c == '\t') { result += "\\t"; continue; }，成立时执行对应分支。
            if (c == '\t') { result += "\\t"; continue; }
            // 【逐行批注】 判断条件 (c < 0x20) { result += ' '; continue; }，成立时执行对应分支。
            if (c < 0x20) { result += ' '; continue; }
            // 【逐行批注】 计算右侧结果并写入 result +，更新当前运行状态。
            result += value[i];
            // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
            continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (c == '"') { inString = true; result += value[i]; continue; }，成立时执行对应分支。
        if (c == '"') { inString = true; result += value[i]; continue; }
        // 【逐行批注】 判断条件 (c == '\n') {，成立时执行对应分支。
        if (c == '\n') {
            // 【逐行批注】 计算右侧结果并写入 size_t j，更新当前运行状态。
            size_t j = i + 1;
            // 【逐行批注】 在条件成立期间循环处理：while (j < value.size() && (value[j] == ' ' || value[j] == '\t')) ++j;。
            while (j < value.size() && (value[j] == ' ' || value[j] == '\t')) ++j;
            // 【逐行批注】 判断条件 (value.compare(j, 2, "//") == 0) {，成立时执行对应分支。
            if (value.compare(j, 2, "//") == 0) {
                // 【逐行批注】 计算右侧结果并写入 auto comment，更新当前运行状态。
                const auto comment = value.find('\n', j);
                // 【逐行批注】 判断条件 (comment == std::string::npos) break;，成立时执行对应分支。
                if (comment == std::string::npos) break;
                // 【逐行批注】 计算右侧结果并写入 i，更新当前运行状态。
                i = comment;
                // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
                continue;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 result +，更新当前运行状态。
            result += '\n';
            // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
            continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (c < 0x20 && c != '\t') { result += ' '; continue; }，成立时执行对应分支。
        if (c < 0x20 && c != '\t') { result += ' '; continue; }
        // 【逐行批注】 计算右侧结果并写入 result +，更新当前运行状态。
        result += value[i];
    // 【逐行批注】 结束当前代码块。
    }
    // 去掉对象或数组结尾前多余的逗号，只在字符串外处理。
    // 【逐行批注】 执行当前声明或表达式：std::string cleaned;。
    std::string cleaned;
    // 【逐行批注】 执行当前声明或表达式：cleaned.reserve(result.size());。
    cleaned.reserve(result.size());
    // 【逐行批注】 计算右侧结果并写入 bool string，更新当前运行状态。
    bool string = false, escape = false;
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < result.size(); ++i) {。
    for (size_t i = 0; i < result.size(); ++i) {
        // 【逐行批注】 计算右侧结果并写入 char c，更新当前运行状态。
        const char c = result[i];
        // 【逐行批注】 判断条件 (string) {，成立时执行对应分支。
        if (string) {
            // 【逐行批注】 计算右侧结果并写入 cleaned +，更新当前运行状态。
            cleaned += c;
            // 【逐行批注】 判断条件 (escape) escape = false;，成立时执行对应分支。
            if (escape) escape = false;
            // 【逐行批注】 在前一条件不成立时继续判断：(c == '\\') escape = true;。
            else if (c == '\\') escape = true;
            // 【逐行批注】 在前一条件不成立时继续判断：(c == '"') string = false;。
            else if (c == '"') string = false;
            // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
            continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (c == '"') { string = true; cleaned += c; continue; }，成立时执行对应分支。
        if (c == '"') { string = true; cleaned += c; continue; }
        // 【逐行批注】 判断条件 (c == ',') {，成立时执行对应分支。
        if (c == ',') {
            // 【逐行批注】 计算右侧结果并写入 size_t next，更新当前运行状态。
            size_t next = i + 1;
            // 【逐行批注】 在条件成立期间循环处理：while (next < result.size() && (result[next] == ' ' || result[next] == '\t…。
            while (next < result.size() && (result[next] == ' ' || result[next] == '\t' || result[next] == '\n')) ++next;
            // 【逐行批注】 判断条件 (next < result.size() && (result[next] == '}' || result[next] ==…，成立时执行对应分支。
            if (next < result.size() && (result[next] == '}' || result[next] == ']')) continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 cleaned +，更新当前运行状态。
        cleaned += c;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return cleaned;。
    return cleaned;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 closeTruncated，执行该函数声明的职责。
std::string closeTruncated(const std::string& value) {
    // 【逐行批注】 执行当前声明或表达式：std::vector<char> stack;。
    std::vector<char> stack;
    // 【逐行批注】 计算右侧结果并写入 bool string，更新当前运行状态。
    bool string = false, escaped = false;
    // 【逐行批注】 按循环规则重复处理：for (char c : value) {。
    for (char c : value) {
        // 【逐行批注】 判断条件 (string) { if (escaped) escaped = false; else if (c == '\\') esc…，成立时执行对应分支。
        if (string) { if (escaped) escaped = false; else if (c == '\\') escaped = true; else if (c == '"') string = false; continue; }
        // 【逐行批注】 判断条件 (c == '"') string = true;，成立时执行对应分支。
        if (c == '"') string = true;
        // 【逐行批注】 在前一条件不成立时继续判断：(c == '{') stack.push_back('}'); else if (c == '[') stack.push_b…。
        else if (c == '{') stack.push_back('}'); else if (c == '[') stack.push_back(']');
        // 【逐行批注】 在前一条件不成立时继续判断：((c == '}' || c == ']') && !stack.empty()) stack.pop_back();。
        else if ((c == '}' || c == ']') && !stack.empty()) stack.pop_back();
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 std::string result，更新当前运行状态。
    std::string result = value;
    // 【逐行批注】 判断条件 (string) {，成立时执行对应分支。
    if (string) {
        // 截断可能发生在 UTF-8 多字节字符中间（半个汉字）：补引号前先丢弃不完整的字符序列
        // 【逐行批注】 计算右侧结果并写入 size_t i，更新当前运行状态。
        size_t i = result.size();
        // 【逐行批注】 计算右侧结果并写入 int cont，更新当前运行状态。
        int cont = 0;
        // 【逐行批注】 在条件成立期间循环处理：while (i > 0) {。
        while (i > 0) {
            // 【逐行批注】 计算右侧结果并写入 unsigned char c，更新当前运行状态。
            const unsigned char c = static_cast<unsigned char>(result[i - 1]);
            // 【逐行批注】 判断条件 (c >= 0x80 && c < 0xC0) { ++cont; --i; continue; } // 续字节(10xxxx…，成立时执行对应分支。
            if (c >= 0x80 && c < 0xC0) { ++cont; --i; continue; }   // 续字节(10xxxxxx)
            // 【逐行批注】 判断条件 (c >= 0xC0) { // 起始字节，成立时执行对应分支。
            if (c >= 0xC0) {                                         // 起始字节
                // 【逐行批注】 计算右侧结果并写入 int expected，更新当前运行状态。
                const int expected = c >= 0xF0 ? 3 : (c >= 0xE0 ? 2 : 1);
                // 【逐行批注】 判断条件 (cont < expected) result.erase(i - 1); // 字符不完整：删除整个未完成序列，成立时执行对应分支。
                if (cont < expected) result.erase(i - 1);            // 字符不完整：删除整个未完成序列
                // 【逐行批注】 跳出当前循环或分支。
                break;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 跳出当前循环或分支。
            break;                                                   // ASCII：完整
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 result +，更新当前运行状态。
        result += '"';
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 在条件成立期间循环处理：while (!stack.empty()) { result += stack.back(); stack.pop_back(); }。
    while (!stack.empty()) { result += stack.back(); stack.pop_back(); }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 balanceBrackets，执行该函数声明的职责。
std::string balanceBrackets(const std::string& value, bool insertMissing = true) {
    // 模型偶尔漏写或多写闭合括号。这里只按栈补齐缺失的 } 或 ]、丢弃多余的闭合符号，
    // 不添加、删除或改写任何实际内容；补全后的结果仍需通过主控结构校验。
    // 【逐行批注】 执行当前声明或表达式：std::string result;。
    std::string result;
    // 【逐行批注】 执行当前声明或表达式：result.reserve(value.size() + 8);。
    result.reserve(value.size() + 8);
    // 【逐行批注】 执行当前声明或表达式：std::vector<char> stack;。
    std::vector<char> stack;
    // 【逐行批注】 计算右侧结果并写入 bool string，更新当前运行状态。
    bool string = false, escaped = false;
    // 【逐行批注】 按循环规则重复处理：for (char c : value) {。
    for (char c : value) {
        // 【逐行批注】 判断条件 (string) {，成立时执行对应分支。
        if (string) {
            // 【逐行批注】 计算右侧结果并写入 result +，更新当前运行状态。
            result += c;
            // 【逐行批注】 判断条件 (escaped) escaped = false;，成立时执行对应分支。
            if (escaped) escaped = false;
            // 【逐行批注】 在前一条件不成立时继续判断：(c == '\\') escaped = true;。
            else if (c == '\\') escaped = true;
            // 【逐行批注】 在前一条件不成立时继续判断：(c == '"') string = false;。
            else if (c == '"') string = false;
            // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
            continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (c == '"') { string = true; result += c; continue; }，成立时执行对应分支。
        if (c == '"') { string = true; result += c; continue; }
        // 【逐行批注】 判断条件 (c == '{') { stack.push_back('}'); result += c; continue; }，成立时执行对应分支。
        if (c == '{') { stack.push_back('}'); result += c; continue; }
        // 【逐行批注】 判断条件 (c == '[') { stack.push_back(']'); result += c; continue; }，成立时执行对应分支。
        if (c == '[') { stack.push_back(']'); result += c; continue; }
        // 【逐行批注】 判断条件 (c == '}' || c == ']') {，成立时执行对应分支。
        if (c == '}' || c == ']') {
            // 【逐行批注】 判断条件 (insertMissing) while (!stack.empty() && stack.back() != c) { re…，成立时执行对应分支。
            if (insertMissing) while (!stack.empty() && stack.back() != c) { result += stack.back(); stack.pop_back(); }
            // 【逐行批注】 判断条件 (stack.empty()) continue;，成立时执行对应分支。
            if (stack.empty()) continue;
            // 【逐行批注】 判断条件 (stack.back() != c) continue; // 丢弃多余的闭合符号，成立时执行对应分支。
            if (stack.back() != c) continue;  // 丢弃多余的闭合符号
            // 【逐行批注】 执行当前声明或表达式：stack.pop_back();。
            stack.pop_back();
            // 【逐行批注】 计算右侧结果并写入 result +，更新当前运行状态。
            result += c;
            // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
            continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 result +，更新当前运行状态。
        result += c;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (string) result += '"';，成立时执行对应分支。
    if (string) result += '"';
    // 【逐行批注】 在条件成立期间循环处理：while (!stack.empty()) { result += stack.back(); stack.pop_back(); }。
    while (!stack.empty()) { result += stack.back(); stack.pop_back(); }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 completeObjects，执行该函数声明的职责。
std::vector<std::string> completeObjects(const std::string& value) {
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> objects;。
    std::vector<std::string> objects;
    // 【逐行批注】 计算右侧结果并写入 size_t start，更新当前运行状态。
    size_t start = std::string::npos;
    // 【逐行批注】 计算右侧结果并写入 int depth，更新当前运行状态。
    int depth = 0;
    // 【逐行批注】 计算右侧结果并写入 bool inString，更新当前运行状态。
    bool inString = false, escaped = false;
    // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < value.size(); ++index) {。
    for (size_t index = 0; index < value.size(); ++index) {
        // 【逐行批注】 计算右侧结果并写入 char character，更新当前运行状态。
        const char character = value[index];
        // 【逐行批注】 判断条件 (inString) {，成立时执行对应分支。
        if (inString) {
            // 【逐行批注】 判断条件 (escaped) escaped = false;，成立时执行对应分支。
            if (escaped) escaped = false;
            // 【逐行批注】 在前一条件不成立时继续判断：(character == '\\') escaped = true;。
            else if (character == '\\') escaped = true;
            // 【逐行批注】 在前一条件不成立时继续判断：(character == '"') inString = false;。
            else if (character == '"') inString = false;
            // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
            continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (character == '"') inString = true;，成立时执行对应分支。
        if (character == '"') inString = true;
        // 【逐行批注】 在前一条件不成立时继续判断：(character == '{') {。
        else if (character == '{') {
            // 【逐行批注】 判断条件 (depth++ == 0) start = index;，成立时执行对应分支。
            if (depth++ == 0) start = index;
        // 【逐行批注】 执行当前声明或表达式：} else if (character == '}' && depth > 0 && --depth == 0 && start != std::string::np…。
        } else if (character == '}' && depth > 0 && --depth == 0 && start != std::string::npos) {
            // 【逐行批注】 执行当前声明或表达式：objects.push_back(value.substr(start, index - start + 1));。
            objects.push_back(value.substr(start, index - start + 1));
            // 【逐行批注】 计算右侧结果并写入 start，更新当前运行状态。
            start = std::string::npos;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return objects;。
    return objects;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 parseAIJson，执行该函数声明的职责。
nlohmann::json parseAIJson(const std::string& content) {
    // 【逐行批注】 计算右侧结果并写入 std::string value，更新当前运行状态。
    std::string value = content;
    // 【逐行批注】 判断条件 (value.size() >= 3 && static_cast<unsigned char>(value[0]) == 0x…，成立时执行对应分支。
    if (value.size() >= 3 && static_cast<unsigned char>(value[0]) == 0xEF && static_cast<unsigned char>(value[1]) == 0xBB && static_cast<unsigned char>(value[2]) == 0xBF) value.erase(0, 3);
    // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
    value = trim(value);
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto wrapped，更新当前运行状态。
        const auto wrapped = nlohmann::json::parse(value);
        // 【逐行批注】 判断条件 (wrapped.is_string()) value = trim(wrapped.get<std::string>());，成立时执行对应分支。
        if (wrapped.is_string()) value = trim(wrapped.get<std::string>());
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {}。
    } catch (...) {}
    // 【逐行批注】 判断条件 (value.rfind("```", 0) == 0) {，成立时执行对应分支。
    if (value.rfind("```", 0) == 0) {
        // 【逐行批注】 计算右侧结果并写入 auto start，更新当前运行状态。
        const auto start = value.find('\n');
        // 【逐行批注】 计算右侧结果并写入 auto end，更新当前运行状态。
        const auto end = value.rfind("```");
        // 【逐行批注】 判断条件 (start != std::string::npos && end > start) value = trim(value.s…，成立时执行对应分支。
        if (start != std::string::npos && end > start) value = trim(value.substr(start + 1, end - start - 1));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto left，更新当前运行状态。
    const auto left = value.find('{'); const auto right = value.rfind('}');
    // 【逐行批注】 判断条件 (left != std::string::npos) value = value.substr(left, right == …，成立时执行对应分支。
    if (left != std::string::npos) value = value.substr(left, right == std::string::npos ? std::string::npos : right - left + 1);
    // 【逐行批注】 计算右侧结果并写入 std::string repaired，更新当前运行状态。
    const std::string repaired = repair(value);
    // 【逐行批注】 按循环规则重复处理：for (const auto& candidate : {value, balanceBrackets(value), balanceBracke…。
    for (const auto& candidate : {value, balanceBrackets(value), balanceBrackets(value, false), repaired,
                                  // 【逐行批注】 调用 balanceBrackets、closeTruncated 完成本行所需操作：balanceBrackets(repaired), balanceBrackets(repaired, false), closeTru…。
                                  balanceBrackets(repaired), balanceBrackets(repaired, false), closeTruncated(repaired)}) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { return nlohmann::json::parse(candidate); } catch (...) {}
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto objects，更新当前运行状态。
    const auto objects = completeObjects(repaired);
    // 【逐行批注】 按循环规则重复处理：for (auto item = objects.rbegin(); item != objects.rend(); ++item) {。
    for (auto item = objects.rbegin(); item != objects.rend(); ++item) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { return nlohmann::json::parse(repair(*item)); } catch (...) {}
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 int attempts，更新当前运行状态。
    int attempts = 0;
    // 【逐行批注】 按循环规则重复处理：for (size_t end = repaired.size(); end > 0 && attempts < 60; --end) {。
    for (size_t end = repaired.size(); end > 0 && attempts < 60; --end) {
        // 【逐行批注】 计算右侧结果并写入 size_t closing，更新当前运行状态。
        const size_t closing = repaired.rfind('}', end - 1);
        // 【逐行批注】 判断条件 (closing == std::string::npos) break;，成立时执行对应分支。
        if (closing == std::string::npos) break;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { return nlohmann::json::parse(closeTruncated(repaired.substr(0, closing + 1))); } catch (...) {}
        // 【逐行批注】 计算右侧结果并写入 end，更新当前运行状态。
        end = closing + 1;
        // 【逐行批注】 计算右侧结果并写入 attempts +，更新当前运行状态。
        attempts += 1;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 检测到错误情形时抛出异常：throw AIClientError("json_parse_error", "unable to parse AI JSON");。
    throw AIClientError("json_parse_error", "unable to parse AI JSON");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

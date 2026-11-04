// 【逐行批注】 引入头文件 "course_service.hpp"，为后续实现提供类型或接口声明。
#include "course_service.hpp"
// 【逐行批注】 引入头文件 "text_utils.hpp"，为后续实现提供类型或接口声明。
#include "text_utils.hpp"

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <limits>，为后续实现提供类型或接口声明。
#include <limits>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <unordered_map>，为后续实现提供类型或接口声明。
#include <unordered_map>
// 【逐行批注】 引入头文件 <unordered_set>，为后续实现提供类型或接口声明。
#include <unordered_set>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 using json，更新当前运行状态。
using json = nlohmann::json;

// 【逐行批注】 定义函数 trim，执行该函数声明的职责。
std::string trim(const std::string& value) {
    // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
    const auto first = value.find_first_not_of(" \t\r\n\f\v");
    // 【逐行批注】 判断条件 (first == std::string::npos) return {};，成立时执行对应分支。
    if (first == std::string::npos) return {};
    // 【逐行批注】 计算右侧结果并写入 auto last，更新当前运行状态。
    const auto last = value.find_last_not_of(" \t\r\n\f\v");
    // 【逐行批注】 结束当前函数并返回结果：return value.substr(first, last - first + 1);。
    return value.substr(first, last - first + 1);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 nowIso8601，执行该函数声明的职责。
std::string nowIso8601() {
    // 【逐行批注】 计算右侧结果并写入 auto now，更新当前运行状态。
    const auto now = std::chrono::system_clock::now();
    // 【逐行批注】 计算右侧结果并写入 std::time_t time，更新当前运行状态。
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    // 【逐行批注】 执行当前声明或表达式：std::tm tm{};。
    std::tm tm{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&tm, &time);。
    gmtime_s(&tm, &time);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&time, &tm);。
    gmtime_r(&time, &tm);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream out;。
    std::ostringstream out;
    // 【逐行批注】 调用 put_time 完成本行所需操作：out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");。
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    // 【逐行批注】 结束当前函数并返回结果：return out.str();。
    return out.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 containsSensitiveKey，执行该函数声明的职责。
bool containsSensitiveKey(const std::string& key) {
    // 【逐行批注】 执行当前声明或表达式：const std::string lowered;。
    const std::string lowered;
    // 【逐行批注】 计算右侧结果并写入 std::string lowerKey，更新当前运行状态。
    std::string lowerKey = key;
    // 【逐行批注】 调用 transform、tolower 完成本行所需操作：std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), []…。
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    // 【逐行批注】 执行当前声明或表达式：static const std::vector<std::string> needles = {。
    static const std::vector<std::string> needles = {
        // 【逐行批注】 执行当前声明或表达式："apikey", "api_key", "secret", "token", "authorization", "base64", "image", "rawerro…。
        "apikey", "api_key", "secret", "token", "authorization", "base64", "image", "rawerror", "stack"
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 按循环规则重复处理：for (const auto& needle : needles) {。
    for (const auto& needle : needles) {
        // 【逐行批注】 判断条件 (lowerKey.find(needle) != std::string::npos) return true;，成立时执行对应分支。
        if (lowerKey.find(needle) != std::string::npos) return true;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return false;。
    return false;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 clampString，执行该函数声明的职责。
json clampString(const json& value, size_t maxLen) {
    // 【逐行批注】 判断条件 (!value.is_string()) return value;，成立时执行对应分支。
    if (!value.is_string()) return value;
    // 【逐行批注】 计算右侧结果并写入 std::string str，更新当前运行状态。
    std::string str = value.get<std::string>();
    // 【逐行批注】 结束当前函数并返回结果：return truncateUtf8(str, maxLen);。
    return truncateUtf8(str, maxLen);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 sanitizeRecursive，并通过构造函数初始化其状态。
json sanitizeRecursive(const json& value, int depth);

// 【逐行批注】 定义函数 sanitizeArray，执行该函数声明的职责。
json sanitizeArray(const json& value, int depth) {
    // 【逐行批注】 计算右侧结果并写入 json out，更新当前运行状态。
    json out = json::array();
    // 【逐行批注】 判断条件 (!value.is_array()) return out;，成立时执行对应分支。
    if (!value.is_array()) return out;
    // 【逐行批注】 计算右侧结果并写入 size_t count，更新当前运行状态。
    size_t count = 0;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : value) {。
    for (const auto& item : value) {
        // 【逐行批注】 判断条件 (count >= 1000) break;，成立时执行对应分支。
        if (count >= 1000) break;
        // 【逐行批注】 调用 sanitizeRecursive 完成本行所需操作：out.push_back(sanitizeRecursive(item, depth + 1));。
        out.push_back(sanitizeRecursive(item, depth + 1));
        // 【逐行批注】 执行当前声明或表达式：++count;。
        ++count;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return out;。
    return out;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 sanitizeResourceItem，执行该函数声明的职责。
json sanitizeResourceItem(const json& value) {
    // 【逐行批注】 判断条件 (!value.is_object()) return json::object();，成立时执行对应分支。
    if (!value.is_object()) return json::object();
    // 【逐行批注】 执行当前声明或表达式：json out;。
    json out;
    // 【逐行批注】 计算右侧结果并写入 out["name"]，更新当前运行状态。
    out["name"] = clampString(value.value("name", ""), 300);
    // 【逐行批注】 计算右侧结果并写入 out["type"]，更新当前运行状态。
    out["type"] = clampString(value.value("type", ""), 100);
    // 【逐行批注】 计算右侧结果并写入 out["difficulty"]，更新当前运行状态。
    out["difficulty"] = clampString(value.value("difficulty", ""), 100);
    // 【逐行批注】 计算右侧结果并写入 out["free"]，更新当前运行状态。
    out["free"] = value.value("free", true);
    // 【逐行批注】 计算右侧结果并写入 out["description"]，更新当前运行状态。
    out["description"] = clampString(value.value("description", ""), 500);
    // 【逐行批注】 计算右侧结果并写入 out["href"]，更新当前运行状态。
    out["href"] = clampString(value.value("href", value.value("url", "")), 1000);
    // 【逐行批注】 计算右侧结果并写入 out["source"]，更新当前运行状态。
    out["source"] = clampString(value.value("source", ""), 200);
    // 【逐行批注】 计算右侧结果并写入 out["language"]，更新当前运行状态。
    out["language"] = clampString(value.value("language", ""), 50);
    // 【逐行批注】 计算右侧结果并写入 out["score"]，更新当前运行状态。
    out["score"] = value.value("score", 70.0);
    // 【逐行批注】 计算右侧结果并写入 out["reason"]，更新当前运行状态。
    out["reason"] = clampString(value.value("reason", ""), 500);
    // 【逐行批注】 结束当前函数并返回结果：return out;。
    return out;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 sanitizeObject，执行该函数声明的职责。
json sanitizeObject(const json& value, int depth) {
    // 【逐行批注】 执行当前声明或表达式：static const std::vector<std::string> whitelist = {。
    static const std::vector<std::string> whitelist = {
        // 【逐行批注】 执行当前声明或表达式："title", "duration", "summary", "courseIntro", "overview", "audience", "prerequisite…。
        "title", "duration", "summary", "courseIntro", "overview", "audience", "prerequisites", "outcome",
        // 【逐行批注】 执行当前声明或表达式："learningOutcomes", "slides", "mindMap", "roadmap", "courseStructure", "resources", …。
        "learningOutcomes", "slides", "mindMap", "roadmap", "courseStructure", "resources", "projects",
        // 【逐行批注】 执行当前声明或表达式："generation", "phaseExpansions"。
        "generation", "phaseExpansions"
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 json out，更新当前运行状态。
    json out = json::object();
    // 【逐行批注】 判断条件 (depth > 0) {，成立时执行对应分支。
    if (depth > 0) {
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : value.items()) {。
        for (const auto& item : value.items()) {
            // 【逐行批注】 判断条件 (containsSensitiveKey(item.key())) continue;，成立时执行对应分支。
            if (containsSensitiveKey(item.key())) continue;
            // 【逐行批注】 计算右侧结果并写入 out[item.key()]，更新当前运行状态。
            out[item.key()] = sanitizeRecursive(item.value(), depth + 1);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return out;。
        return out;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& key : whitelist) {。
    for (const auto& key : whitelist) {
        // 【逐行批注】 判断条件 (!value.contains(key)) continue;，成立时执行对应分支。
        if (!value.contains(key)) continue;
        // 【逐行批注】 判断条件 (containsSensitiveKey(key)) continue;，成立时执行对应分支。
        if (containsSensitiveKey(key)) continue;
        // 【逐行批注】 计算右侧结果并写入 auto& item，更新当前运行状态。
        const auto& item = value.at(key);
        // 【逐行批注】 判断条件 (key == "resources" && item.is_array()) {，成立时执行对应分支。
        if (key == "resources" && item.is_array()) {
            // 【逐行批注】 计算右侧结果并写入 json resources，更新当前运行状态。
            json resources = json::array();
            // 【逐行批注】 计算右侧结果并写入 size_t count，更新当前运行状态。
            size_t count = 0;
            // 【逐行批注】 按循环规则重复处理：for (const auto& resource : item) {。
            for (const auto& resource : item) {
                // 【逐行批注】 判断条件 (count >= 12) break;，成立时执行对应分支。
                if (count >= 12) break;
                // 【逐行批注】 判断条件 (!resource.is_object()) continue;，成立时执行对应分支。
                if (!resource.is_object()) continue;
                // 【逐行批注】 执行当前声明或表达式：std::string nameText;。
                std::string nameText;
                // 【逐行批注】 执行当前声明或表达式：std::string hrefText;。
                std::string hrefText;
                // 【逐行批注】 判断条件 (resource.contains("name") && resource["name"].is_string()) name…，成立时执行对应分支。
                if (resource.contains("name") && resource["name"].is_string()) nameText = trim(resource["name"].get<std::string>());
                // 【逐行批注】 判断条件 (resource.contains("href") && resource["href"].is_string()) href…，成立时执行对应分支。
                if (resource.contains("href") && resource["href"].is_string()) hrefText = trim(resource["href"].get<std::string>());
                // 【逐行批注】 判断条件 (hrefText.empty() && resource.contains("url") && resource["url"]…，成立时执行对应分支。
                if (hrefText.empty() && resource.contains("url") && resource["url"].is_string()) hrefText = trim(resource["url"].get<std::string>());
                // 【逐行批注】 判断条件 (nameText.empty() || hrefText.empty()) continue;，成立时执行对应分支。
                if (nameText.empty() || hrefText.empty()) continue;
                // 【逐行批注】 调用 sanitizeResourceItem 完成本行所需操作：resources.push_back(sanitizeResourceItem(resource));。
                resources.push_back(sanitizeResourceItem(resource));
                // 【逐行批注】 执行当前声明或表达式：++count;。
                ++count;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 out[key]，更新当前运行状态。
            out[key] = resources;
            // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
            continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (item.is_string()) {，成立时执行对应分支。
        if (item.is_string()) {
            // 【逐行批注】 计算右侧结果并写入 out[key]，更新当前运行状态。
            out[key] = clampString(item, 10000);
        // 【逐行批注】 执行当前声明或表达式：} else if (item.is_array()) {。
        } else if (item.is_array()) {
            // 【逐行批注】 计算右侧结果并写入 out[key]，更新当前运行状态。
            out[key] = sanitizeArray(item, depth + 1);
        // 【逐行批注】 执行当前声明或表达式：} else if (item.is_object()) {。
        } else if (item.is_object()) {
            // 【逐行批注】 计算右侧结果并写入 json nested，更新当前运行状态。
            json nested = json::object();
            // 【逐行批注】 按循环规则重复处理：for (const auto& nestedItem : item.items()) {。
            for (const auto& nestedItem : item.items()) {
                // 【逐行批注】 判断条件 (containsSensitiveKey(nestedItem.key())) continue;，成立时执行对应分支。
                if (containsSensitiveKey(nestedItem.key())) continue;
                // 【逐行批注】 计算右侧结果并写入 nested[nestedItem.key()]，更新当前运行状态。
                nested[nestedItem.key()] = sanitizeRecursive(nestedItem.value(), depth + 1);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 out[key]，更新当前运行状态。
            out[key] = nested;
        // 【逐行批注】 执行当前声明或表达式：} else {。
        } else {
            // 【逐行批注】 计算右侧结果并写入 out[key]，更新当前运行状态。
            out[key] = item;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return out;。
    return out;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 sanitizeRecursive，执行该函数声明的职责。
json sanitizeRecursive(const json& value, int depth) {
    // 【逐行批注】 判断条件 (depth > 8) return json::object();，成立时执行对应分支。
    if (depth > 8) return json::object();
    // 【逐行批注】 判断条件 (value.is_object()) return sanitizeObject(value, depth);，成立时执行对应分支。
    if (value.is_object()) return sanitizeObject(value, depth);
    // 【逐行批注】 判断条件 (value.is_array()) return sanitizeArray(value, depth);，成立时执行对应分支。
    if (value.is_array()) return sanitizeArray(value, depth);
    // 【逐行批注】 判断条件 (value.is_string()) return clampString(value, 10000);，成立时执行对应分支。
    if (value.is_string()) return clampString(value, 10000);
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 jsonSize，执行该函数声明的职责。
size_t jsonSize(const json& value) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 结束当前函数并返回结果：return value.dump().size();。
        return value.dump().size();
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 clampLargePayload，执行该函数声明的职责。
json clampLargePayload(json payload) {
    // 【逐行批注】 判断条件 (!payload.is_object()) return payload;，成立时执行对应分支。
    if (!payload.is_object()) return payload;
    // 【逐行批注】 判断条件 (jsonSize(payload) <= 900000) return payload;，成立时执行对应分支。
    if (jsonSize(payload) <= 900000) return payload;
    // 【逐行批注】 判断条件 (payload.contains("slides") && payload["slides"].is_array() && p…，成立时执行对应分支。
    if (payload.contains("slides") && payload["slides"].is_array() && payload["slides"].size() > 20) payload["slides"].erase(payload["slides"].begin() + 20, payload["slides"].end());
    // 【逐行批注】 判断条件 (payload.contains("roadmap") && payload["roadmap"].is_array() &&…，成立时执行对应分支。
    if (payload.contains("roadmap") && payload["roadmap"].is_array() && payload["roadmap"].size() > 12) payload["roadmap"].erase(payload["roadmap"].begin() + 12, payload["roadmap"].end());
    // 【逐行批注】 判断条件 (payload.contains("resources") && payload["resources"].is_array(…，成立时执行对应分支。
    if (payload.contains("resources") && payload["resources"].is_array() && payload["resources"].size() > 8) payload["resources"].erase(payload["resources"].begin() + 8, payload["resources"].end());
    // 【逐行批注】 判断条件 (payload.contains("projects") && payload["projects"].is_array() …，成立时执行对应分支。
    if (payload.contains("projects") && payload["projects"].is_array() && payload["projects"].size() > 12) payload["projects"].erase(payload["projects"].begin() + 12, payload["projects"].end());
    // 【逐行批注】 结束当前函数并返回结果：return payload;。
    return payload;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 pickCourseByIdentity，执行该函数声明的职责。
std::optional<Course> pickCourseByIdentity(Database& db, const std::string& anonymousId, const std::string& userId, const std::string& goal) {
    // 【逐行批注】 计算右侧结果并写入 auto courses，更新当前运行状态。
    const auto courses = db.listCourses();
    // 【逐行批注】 执行当前声明或表达式：std::optional<Course> found;。
    std::optional<Course> found;
    // 【逐行批注】 按循环规则重复处理：for (const auto& course : courses) {。
    for (const auto& course : courses) {
        // 【逐行批注】 判断条件 (course.status != "active") continue;，成立时执行对应分支。
        if (course.status != "active") continue;
        // 【逐行批注】 判断条件 (course.goal != goal) continue;，成立时执行对应分支。
        if (course.goal != goal) continue;
        // 【逐行批注】 计算右侧结果并写入 bool match，更新当前运行状态。
        bool match = false;
        // 【逐行批注】 判断条件 (!userId.empty() && course.userId && *course.userId == userId) m…，成立时执行对应分支。
        if (!userId.empty() && course.userId && *course.userId == userId) match = true;
        // 【逐行批注】 判断条件 (!anonymousId.empty() && course.anonymousId && *course.anonymous…，成立时执行对应分支。
        if (!anonymousId.empty() && course.anonymousId && *course.anonymousId == anonymousId) match = true;
        // 【逐行批注】 判断条件 (!match) continue;，成立时执行对应分支。
        if (!match) continue;
        // 【逐行批注】 判断条件 (!found || course.updatedAt > found->updatedAt) found = course;，成立时执行对应分支。
        if (!found || course.updatedAt > found->updatedAt) found = course;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return found;。
    return found;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 定义函数 sanitizeCoursePayload，执行该函数声明的职责。
json sanitizeCoursePayload(const json& payload) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 json sanitized，更新当前运行状态。
        json sanitized = sanitizeRecursive(payload, 0);
        // 【逐行批注】 计算右侧结果并写入 sanitized，更新当前运行状态。
        sanitized = clampLargePayload(std::move(sanitized));
        // 【逐行批注】 结束当前函数并返回结果：return sanitized;。
        return sanitized;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return payload;。
        return payload;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 saveCourseSnapshot，并通过构造函数初始化其状态。
std::optional<std::string> saveCourseSnapshot(Database& db, const std::string& anonymousId, const std::string& userId,
                                              // 【逐行批注】 执行当前声明或表达式：const std::string& goal, const std::string& mode, const std::string& title,。
                                              const std::string& goal, const std::string& mode, const std::string& title,
                                              // 【逐行批注】 执行当前声明或表达式：const std::string& summary, const std::string& source, const json& payload) {。
                                              const std::string& summary, const std::string& source, const json& payload) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 std::string now，更新当前运行状态。
        const std::string now = nowIso8601();
        // 【逐行批注】 调用 pickCourseByIdentity 完成本行所需操作：const std::optional<Course> existing = pickCourseByIdentity(db, anony…。
        const std::optional<Course> existing = pickCourseByIdentity(db, anonymousId, userId, goal);
        // 【逐行批注】 计算右侧结果并写入 json sanitized，更新当前运行状态。
        const json sanitized = sanitizeCoursePayload(payload);
        // 【逐行批注】 判断条件 (existing) {，成立时执行对应分支。
        if (existing) {
            // 【逐行批注】 计算右侧结果并写入 Course course，更新当前运行状态。
            Course course = *existing;
            // 【逐行批注】 计算右侧结果并写入 course.title，更新当前运行状态。
            course.title = title;
            // 【逐行批注】 计算右侧结果并写入 course.summary，更新当前运行状态。
            course.summary = summary;
            // 【逐行批注】 计算右侧结果并写入 course.source，更新当前运行状态。
            course.source = source;
            // 【逐行批注】 计算右侧结果并写入 course.mode，更新当前运行状态。
            course.mode = mode;
            // 【逐行批注】 计算右侧结果并写入 course.updatedAt，更新当前运行状态。
            course.updatedAt = now;
            // 【逐行批注】 判断条件 (!db.update(course)) { std::cerr << "[course-save] update course…，成立时执行对应分支。
            if (!db.update(course)) { std::cerr << "[course-save] update course failed" << std::endl; return std::nullopt; }
            // 【逐行批注】 计算右侧结果并写入 int maxVersion，更新当前运行状态。
            int maxVersion = 0;
            // 【逐行批注】 按循环规则重复处理：for (const auto& snapshot : db.findSnapshotsByCourseId(course.id)) {。
            for (const auto& snapshot : db.findSnapshotsByCourseId(course.id)) {
                // 【逐行批注】 判断条件 (snapshot.version > maxVersion) maxVersion = snapshot.version;，成立时执行对应分支。
                if (snapshot.version > maxVersion) maxVersion = snapshot.version;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 执行当前声明或表达式：CourseSnapshot snapshot;。
            CourseSnapshot snapshot;
            // 【逐行批注】 计算右侧结果并写入 snapshot.id，更新当前运行状态。
            snapshot.id = "";
            // 【逐行批注】 计算右侧结果并写入 snapshot.courseId，更新当前运行状态。
            snapshot.courseId = course.id;
            // 【逐行批注】 计算右侧结果并写入 snapshot.version，更新当前运行状态。
            snapshot.version = maxVersion + 1;
            // 【逐行批注】 计算右侧结果并写入 snapshot.payload，更新当前运行状态。
            snapshot.payload = sanitized.dump();
            // 【逐行批注】 计算右侧结果并写入 snapshot.createdAt，更新当前运行状态。
            snapshot.createdAt = now;
            // 【逐行批注】 判断条件 (!db.insert(snapshot)) { std::cerr << "[course-save] insert snap…，成立时执行对应分支。
            if (!db.insert(snapshot)) { std::cerr << "[course-save] insert snapshot failed for existing course" << std::endl; return std::nullopt; }
            // 【逐行批注】 结束当前函数并返回结果：return course.id;。
            return course.id;
        // 【逐行批注】 结束当前代码块。
        }

        // 【逐行批注】 执行当前声明或表达式：Course course;。
        Course course;
        // 【逐行批注】 计算右侧结果并写入 course.id，更新当前运行状态。
        course.id = "";
        // 【逐行批注】 计算右侧结果并写入 course.anonymousId，更新当前运行状态。
        course.anonymousId = anonymousId.empty() ? std::nullopt : std::optional<std::string>(anonymousId);
        // 【逐行批注】 计算右侧结果并写入 course.userId，更新当前运行状态。
        course.userId = userId.empty() ? std::nullopt : std::optional<std::string>(userId);
        // 【逐行批注】 计算右侧结果并写入 course.goal，更新当前运行状态。
        course.goal = goal;
        // 【逐行批注】 计算右侧结果并写入 course.mode，更新当前运行状态。
        course.mode = mode;
        // 【逐行批注】 计算右侧结果并写入 course.title，更新当前运行状态。
        course.title = title;
        // 【逐行批注】 计算右侧结果并写入 course.summary，更新当前运行状态。
        course.summary = summary;
        // 【逐行批注】 计算右侧结果并写入 course.source，更新当前运行状态。
        course.source = source;
        // 【逐行批注】 计算右侧结果并写入 course.status，更新当前运行状态。
        course.status = "active";
        // 【逐行批注】 计算右侧结果并写入 course.createdAt，更新当前运行状态。
        course.createdAt = now;
        // 【逐行批注】 计算右侧结果并写入 course.updatedAt，更新当前运行状态。
        course.updatedAt = now;
        // 【逐行批注】 判断条件 (!db.insert(course)) { std::cerr << "[course-save] insert course…，成立时执行对应分支。
        if (!db.insert(course)) { std::cerr << "[course-save] insert course failed" << std::endl; return std::nullopt; }
        // 【逐行批注】 计算右侧结果并写入 auto inserted，更新当前运行状态。
        const auto inserted = pickCourseByIdentity(db, anonymousId, userId, goal);
        // 【逐行批注】 判断条件 (!inserted) { std::cerr << "[course-save] inserted course could …，成立时执行对应分支。
        if (!inserted) { std::cerr << "[course-save] inserted course could not be reloaded" << std::endl; return std::nullopt; }
        // 【逐行批注】 执行当前声明或表达式：CourseSnapshot snapshot;。
        CourseSnapshot snapshot;
        // 【逐行批注】 计算右侧结果并写入 snapshot.id，更新当前运行状态。
        snapshot.id = "";
        // 【逐行批注】 计算右侧结果并写入 snapshot.courseId，更新当前运行状态。
        snapshot.courseId = inserted->id;
        // 【逐行批注】 计算右侧结果并写入 snapshot.version，更新当前运行状态。
        snapshot.version = 1;
        // 【逐行批注】 计算右侧结果并写入 snapshot.payload，更新当前运行状态。
        snapshot.payload = sanitized.dump();
        // 【逐行批注】 计算右侧结果并写入 snapshot.createdAt，更新当前运行状态。
        snapshot.createdAt = now;
        // 【逐行批注】 判断条件 (!db.insert(snapshot)) { std::cerr << "[course-save] insert snap…，成立时执行对应分支。
        if (!db.insert(snapshot)) { std::cerr << "[course-save] insert snapshot failed for new course" << std::endl; return std::nullopt; }
        // 【逐行批注】 结束当前函数并返回结果：return inserted->id;。
        return inserted->id;
    // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception&) {。
    } catch (const std::exception&) {
        // 【逐行批注】 执行当前声明或表达式：std::cerr << "[course-save] exception" << std::endl;。
        std::cerr << "[course-save] exception" << std::endl;
        // 【逐行批注】 结束当前函数并返回结果：return std::nullopt;。
        return std::nullopt;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 执行当前声明或表达式：std::cerr << "[course-save] unknown exception" << std::endl;。
        std::cerr << "[course-save] unknown exception" << std::endl;
        // 【逐行批注】 结束当前函数并返回结果：return std::nullopt;。
        return std::nullopt;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 getCourseWithSnapshot，执行该函数声明的职责。
std::optional<CourseWithSnapshot> getCourseWithSnapshot(Database& db, const std::string& courseId) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
        const auto course = db.getCourse(courseId);
        // 【逐行批注】 判断条件 (!course) return std::nullopt;，成立时执行对应分支。
        if (!course) return std::nullopt;
        // 【逐行批注】 计算右侧结果并写入 auto snapshots，更新当前运行状态。
        const auto snapshots = db.findSnapshotsByCourseId(course->id);
        // 【逐行批注】 判断条件 (snapshots.empty()) return std::nullopt;，成立时执行对应分支。
        if (snapshots.empty()) return std::nullopt;
        // 【逐行批注】 计算右侧结果并写入 auto latest，更新当前运行状态。
        const auto latest = *std::max_element(snapshots.begin(), snapshots.end(), [](const CourseSnapshot& a, const CourseSnapshot& b) {
            // 【逐行批注】 判断条件 (a.version != b.version) return a.version < b.version;，成立时执行对应分支。
            if (a.version != b.version) return a.version < b.version;
            // 【逐行批注】 结束当前函数并返回结果：return a.createdAt < b.createdAt;。
            return a.createdAt < b.createdAt;
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 parse 完成本行所需操作：CourseWithSnapshot result{*course, json::parse(latest.payload, nullpt…。
        CourseWithSnapshot result{*course, json::parse(latest.payload, nullptr, false)};
        // 【逐行批注】 判断条件 (result.payload.is_discarded()) return std::nullopt;，成立时执行对应分支。
        if (result.payload.is_discarded()) return std::nullopt;
        // 【逐行批注】 结束当前函数并返回结果：return result;。
        return result;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return std::nullopt;。
        return std::nullopt;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 listCoursesForIdentity，并通过构造函数初始化其状态。
std::vector<Course> listCoursesForIdentity(Database& db, const std::string& userId, const std::string& anonymousId,
                                           // 【逐行批注】 执行当前声明或表达式：size_t limit, size_t offset) {。
                                           size_t limit, size_t offset) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 执行当前声明或表达式：std::vector<Course> courses;。
        std::vector<Course> courses;
        // 【逐行批注】 按循环规则重复处理：for (const auto& course : db.listCourses()) {。
        for (const auto& course : db.listCourses()) {
            // 【逐行批注】 判断条件 (course.status != "active") continue;，成立时执行对应分支。
            if (course.status != "active") continue;
            // 【逐行批注】 计算右侧结果并写入 bool match，更新当前运行状态。
            bool match = false;
            // 【逐行批注】 判断条件 (!userId.empty() && course.userId && *course.userId == userId) m…，成立时执行对应分支。
            if (!userId.empty() && course.userId && *course.userId == userId) match = true;
            // 【逐行批注】 判断条件 (!anonymousId.empty() && course.anonymousId && *course.anonymous…，成立时执行对应分支。
            if (!anonymousId.empty() && course.anonymousId && *course.anonymousId == anonymousId) match = true;
            // 【逐行批注】 判断条件 (userId.empty() && anonymousId.empty()) match = true;，成立时执行对应分支。
            if (userId.empty() && anonymousId.empty()) match = true;
            // 【逐行批注】 判断条件 (!match) continue;，成立时执行对应分支。
            if (!match) continue;
            // 【逐行批注】 执行当前声明或表达式：courses.push_back(course);。
            courses.push_back(course);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 sort 完成本行所需操作：std::sort(courses.begin(), courses.end(), [](const Course& a, const C…。
        std::sort(courses.begin(), courses.end(), [](const Course& a, const Course& b) {
            // 【逐行批注】 判断条件 (a.updatedAt != b.updatedAt) return a.updatedAt > b.updatedAt;，成立时执行对应分支。
            if (a.updatedAt != b.updatedAt) return a.updatedAt > b.updatedAt;
            // 【逐行批注】 结束当前函数并返回结果：return a.id < b.id;。
            return a.id < b.id;
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 判断条件 (offset >= courses.size()) return {};，成立时执行对应分支。
        if (offset >= courses.size()) return {};
        // 【逐行批注】 计算右侧结果并写入 size_t end，更新当前运行状态。
        const size_t end = std::min(courses.size(), offset + limit);
        // 【逐行批注】 结束当前函数并返回结果：return std::vector<Course>(courses.begin() + static_cast<std::ptrdiff…。
        return std::vector<Course>(courses.begin() + static_cast<std::ptrdiff_t>(offset), courses.begin() + static_cast<std::ptrdiff_t>(end));
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return {};。
        return {};
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 currentCourseForIdentity，并通过构造函数初始化其状态。
std::optional<Course> currentCourseForIdentity(Database& db, const std::string& userId,
                                               // 【逐行批注】 执行当前声明或表达式：const std::string& anonymousId) {。
                                               const std::string& anonymousId) {
    // 【逐行批注】 执行当前声明或表达式：std::optional<Course> selected;。
    std::optional<Course> selected;
    // 【逐行批注】 计算右侧结果并写入 auto marker，更新当前运行状态。
    const auto marker = json::parse(db.profileMeta("current-course"), nullptr, false);
    // 【逐行批注】 判断条件 (marker.is_object() && marker.value("id", json()).is_string()) {，成立时执行对应分支。
    if (marker.is_object() && marker.value("id", json()).is_string()) {
        // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
        const auto course = db.getCourse(marker.at("id"));
        // 【逐行批注】 判断条件 (course && course->status == "active" && (userId.empty() || cour…，成立时执行对应分支。
        if (course && course->status == "active" && (userId.empty() || course->userId.value_or("") == userId) &&
            // 【逐行批注】 调用 getCourseWithSnapshot 完成本行所需操作：(anonymousId.empty() || course->anonymousId.value_or("") == anonymous…。
            (anonymousId.empty() || course->anonymousId.value_or("") == anonymousId) && getCourseWithSnapshot(db, course->id)) return course;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：std::string latest;。
    std::string latest;
    // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.listInteractions()) {。
    for (const auto& row : db.listInteractions()) {
        // 【逐行批注】 判断条件 (!row.courseId || (row.kind != "course-visit" && row.kind != "le…，成立时执行对应分支。
        if (!row.courseId || (row.kind != "course-visit" && row.kind != "lesson-visit" &&
            // 【逐行批注】 执行当前声明或表达式：row.kind != "chat-user" && row.kind != "practice" && row.kind != "quiz" && row.kind …。
            row.kind != "chat-user" && row.kind != "practice" && row.kind != "quiz" && row.kind != "review")) continue;
        // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
        const auto course = db.getCourse(*row.courseId);
        // 【逐行批注】 判断条件 (!course || course->status != "active") continue;，成立时执行对应分支。
        if (!course || course->status != "active") continue;
        // 【逐行批注】 判断条件 (!userId.empty() && course->userId.value_or("") != userId) conti…，成立时执行对应分支。
        if (!userId.empty() && course->userId.value_or("") != userId) continue;
        // 【逐行批注】 判断条件 (!anonymousId.empty() && course->anonymousId.value_or("") != ano…，成立时执行对应分支。
        if (!anonymousId.empty() && course->anonymousId.value_or("") != anonymousId) continue;
        // 【逐行批注】 判断条件 (!getCourseWithSnapshot(db, course->id)) continue;，成立时执行对应分支。
        if (!getCourseWithSnapshot(db, course->id)) continue;
        // 【逐行批注】 判断条件 (!selected || row.createdAt >= latest) { selected = course; late…，成立时执行对应分支。
        if (!selected || row.createdAt >= latest) { selected = course; latest = row.createdAt; }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return selected;。
    return selected;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 rememberCurrentCourse，执行该函数声明的职责。
void rememberCurrentCourse(Database& db, const std::string& courseId) {
    // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
    const auto course = db.getCourse(courseId);
    // 【逐行批注】 判断条件 (course && course->status == "active" && getCourseWithSnapshot(d…，成立时执行对应分支。
    if (course && course->status == "active" && getCourseWithSnapshot(db, courseId))
        // 【逐行批注】 执行当前声明或表达式：db.setProfileMeta("current-course", json{{"id", courseId}}.dump());。
        db.setProfileMeta("current-course", json{{"id", courseId}}.dump());
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 deleteCourseForIdentity，并通过构造函数初始化其状态。
bool deleteCourseForIdentity(Database& db, const std::string& courseId, const std::string& userId,
                             // 【逐行批注】 执行当前声明或表达式：const std::string& anonymousId) {。
                             const std::string& anonymousId) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
        const auto course = db.getCourse(courseId);
        // 【逐行批注】 判断条件 (!course || course->status != "active") return false;，成立时执行对应分支。
        if (!course || course->status != "active") return false;
        // 【逐行批注】 计算右侧结果并写入 bool ownedByUser，更新当前运行状态。
        const bool ownedByUser = !userId.empty() && course->userId && *course->userId == userId;
        // 【逐行批注】 计算右侧结果并写入 bool ownedByAnonymous，更新当前运行状态。
        const bool ownedByAnonymous = !anonymousId.empty() && course->anonymousId && *course->anonymousId == anonymousId;
        // 【逐行批注】 判断条件 (!ownedByUser && !ownedByAnonymous && (!userId.empty() || !anony…，成立时执行对应分支。
        if (!ownedByUser && !ownedByAnonymous && (!userId.empty() || !anonymousId.empty())) return false;

        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.findSnapshotsByCourseId(courseId)) db.deleteCou…。
        for (const auto& item : db.findSnapshotsByCourseId(courseId)) db.deleteCourseSnapshot(item.id);
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listTaskProgress()) if (item.courseId.value_or(…。
        for (const auto& item : db.listTaskProgress()) if (item.courseId.value_or("") == courseId) db.deleteTaskProgress(item.id);
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listLearningStepProgress()) if (item.courseId.v…。
        for (const auto& item : db.listLearningStepProgress()) if (item.courseId.value_or("") == courseId) db.deleteLearningStepProgress(item.id);
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listLearningCardProgress()) if (item.courseId.v…。
        for (const auto& item : db.listLearningCardProgress()) if (item.courseId.value_or("") == courseId) db.deleteLearningCardProgress(item.id);
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listLearningSessions()) if (item.courseId.value…。
        for (const auto& item : db.listLearningSessions()) if (item.courseId.value_or("") == courseId) db.deleteLearningSession(item.id);
        // 【逐行批注】 执行当前声明或表达式：db.deleteInteractionsForCourse(courseId);。
        db.deleteInteractionsForCourse(courseId);
        // 【逐行批注】 执行当前声明或表达式：db.deleteClassroomData(courseId);。
        db.deleteClassroomData(courseId);
        // 【逐行批注】 执行当前声明或表达式：db.deleteTopicMasteryForCourse(courseId);。
        db.deleteTopicMasteryForCourse(courseId);
        // 【逐行批注】 计算右侧结果并写入 bool deleted，更新当前运行状态。
        const bool deleted = db.deleteCourse(courseId);
        // 【逐行批注】 判断条件 (deleted) { db.markProfileDirty(); db.markLearningDirty(); }，成立时执行对应分支。
        if (deleted) { db.markProfileDirty(); db.markLearningDirty(); }
        // 【逐行批注】 结束当前函数并返回结果：return deleted;。
        return deleted;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

// 【逐行批注】 引入头文件 "plan_adapter.hpp"，为后续实现提供类型或接口声明。
#include "plan_adapter.hpp"

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>
// 【逐行批注】 引入头文件 <cmath>，为后续实现提供类型或接口声明。
#include <cmath>
// 【逐行批注】 引入头文件 <regex>，为后续实现提供类型或接口声明。
#include <regex>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

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

// 【逐行批注】 声明对象 safeTextValue，并通过构造函数初始化其状态。
std::string safeTextValue(const json& value, const std::string& fallback = {}) {
    // 【逐行批注】 判断条件 (!value.is_string()) return fallback;，成立时执行对应分支。
    if (!value.is_string()) return fallback;
    // 【逐行批注】 计算右侧结果并写入 std::string result，更新当前运行状态。
    const std::string result = trim(value.get<std::string>());
    // 【逐行批注】 结束当前函数并返回结果：return result.empty() ? fallback : result;。
    return result.empty() ? fallback : result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 safeText，并通过构造函数初始化其状态。
std::string safeText(const json& object, const std::string& key, const std::string& fallback = {}) {
    // 【逐行批注】 判断条件 (!object.is_object() || !object.contains(key)) return fallback;，成立时执行对应分支。
    if (!object.is_object() || !object.contains(key)) return fallback;
    // 【逐行批注】 结束当前函数并返回结果：return safeTextValue(object.at(key), fallback);。
    return safeTextValue(object.at(key), fallback);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 numberValue，执行该函数声明的职责。
double numberValue(const json& object, const std::string& key, double fallback) {
    // 【逐行批注】 判断条件 (!object.is_object() || !object.contains(key) || !object.at(key)…，成立时执行对应分支。
    if (!object.is_object() || !object.contains(key) || !object.at(key).is_number()) return fallback;
    // 【逐行批注】 结束当前函数并返回结果：return object.at(key).get<double>();。
    return object.at(key).get<double>();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 numberText，执行该函数声明的职责。
std::string numberText(double value) {
    // 【逐行批注】 判断条件 (!std::isfinite(value)) return {};，成立时执行对应分支。
    if (!std::isfinite(value)) return {};
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream out;。
    std::ostringstream out;
    // 【逐行批注】 判断条件 (std::floor(value) == value) out << static_cast<long long>(value…，成立时执行对应分支。
    if (std::floor(value) == value) out << static_cast<long long>(value);
    // 【逐行批注】 执行前一条件不成立时的分支。
    else out << value;
    // 【逐行批注】 结束当前函数并返回结果：return out.str();。
    return out.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 compactStrings，执行该函数声明的职责。
std::vector<std::string> compactStrings(const json& value, size_t limit) {
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> result;。
    std::vector<std::string> result;
    // 【逐行批注】 判断条件 (!value.is_array()) return result;，成立时执行对应分支。
    if (!value.is_array()) return result;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : value) {。
    for (const auto& item : value) {
        // 【逐行批注】 判断条件 (!item.is_string()) continue;，成立时执行对应分支。
        if (!item.is_string()) continue;
        // 【逐行批注】 计算右侧结果并写入 std::string text，更新当前运行状态。
        const std::string text = trim(item.get<std::string>());
        // 【逐行批注】 判断条件 (!text.empty()) result.push_back(text);，成立时执行对应分支。
        if (!text.empty()) result.push_back(text);
        // 【逐行批注】 判断条件 (result.size() >= limit) break;，成立时执行对应分支。
        if (result.size() >= limit) break;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 compactStrings，执行该函数声明的职责。
std::vector<std::string> compactStrings(const json& object, const std::string& key, size_t limit) {
    // 【逐行批注】 判断条件 (!object.is_object() || !object.contains(key)) return {};，成立时执行对应分支。
    if (!object.is_object() || !object.contains(key)) return {};
    // 【逐行批注】 结束当前函数并返回结果：return compactStrings(object.at(key), limit);。
    return compactStrings(object.at(key), limit);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 stringsToJson，执行该函数声明的职责。
json stringsToJson(const std::vector<std::string>& values) {
    // 【逐行批注】 计算右侧结果并写入 json result，更新当前运行状态。
    json result = json::array();
    // 【逐行批注】 按循环规则重复处理：for (const auto& value : values) result.push_back(value);。
    for (const auto& value : values) result.push_back(value);
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 arrayOrEmpty，执行该函数声明的职责。
json arrayOrEmpty(const json& object, const std::string& key) {
    // 【逐行批注】 判断条件 (!object.is_object() || !object.contains(key) || !object.at(key)…，成立时执行对应分支。
    if (!object.is_object() || !object.contains(key) || !object.at(key).is_array()) return json::array();
    // 【逐行批注】 结束当前函数并返回结果：return object.at(key);。
    return object.at(key);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 fallbackTitle，执行该函数声明的职责。
std::string fallbackTitle(const std::string& goal, const std::string& mode) {
    // 【逐行批注】 结束当前函数并返回结果：return mode == "lite" ? goal + "快速学习方案" : "钢一定制AI 学习方案";。
    return mode == "lite" ? goal + "快速学习方案" : "钢一定制AI 学习方案";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 skeletonStepsFromPhase，执行该函数声明的职责。
json skeletonStepsFromPhase(const json& phase) {
    // 【逐行批注】 计算右侧结果并写入 std::string phaseName，更新当前运行状态。
    const std::string phaseName = safeText(phase, "name", "本阶段");
    // 【逐行批注】 调用 compactStrings 完成本行所需操作：std::vector<std::string> topics = compactStrings(phase, "topics", 6);。
    std::vector<std::string> topics = compactStrings(phase, "topics", 6);
    // 【逐行批注】 判断条件 (topics.empty()) topics = compactStrings(phase, "tasks", 6);，成立时执行对应分支。
    if (topics.empty()) topics = compactStrings(phase, "tasks", 6);
    // 【逐行批注】 计算右侧结果并写入 json descriptions，更新当前运行状态。
    const json descriptions = arrayOrEmpty(phase, "topicDescriptions");
    // 【逐行批注】 计算右侧结果并写入 std::string checkpoint，更新当前运行状态。
    const std::string checkpoint = safeText(phase, "checkpoint", "");
    // 【逐行批注】 计算右侧结果并写入 json result，更新当前运行状态。
    json result = json::array();
    // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < topics.size(); ++index) {。
    for (size_t index = 0; index < topics.size(); ++index) {
        // 【逐行批注】 执行当前声明或表达式：std::string explanation;。
        std::string explanation;
        // 【逐行批注】 判断条件 (index < descriptions.size()) explanation = safeTextValue(descri…，成立时执行对应分支。
        if (index < descriptions.size()) explanation = safeTextValue(descriptions.at(index));
        // 【逐行批注】 判断条件 (explanation.empty()) explanation = phaseName + "中的学习点：" + topic…，成立时执行对应分支。
        if (explanation.empty()) explanation = phaseName + "中的学习点：" + topics[index];
        // 【逐行批注】 计算右侧结果并写入 std::string check，更新当前运行状态。
        const std::string check = checkpoint.empty() ? "能完成「" + topics[index] + "」相关练习" : checkpoint;
        // 【逐行批注】 执行当前声明或表达式：result.push_back({{"title", topics[index]},。
        result.push_back({{"title", topics[index]},
                          // 【逐行批注】 执行当前声明或表达式：{"explanation", explanation},。
                          {"explanation", explanation},
                          // 【逐行批注】 执行当前声明或表达式：{"example", ""},。
                          {"example", ""},
                          // 【逐行批注】 执行当前声明或表达式：{"action", "进入微课程学习「" + topics[index] + "」"},。
                          {"action", "进入微课程学习「" + topics[index] + "」"},
                          // 【逐行批注】 执行当前声明或表达式：{"check", check}});。
                          {"check", check}});
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 adaptStep，执行该函数声明的职责。
json adaptStep(const json& step, size_t index, const std::string& phaseName, const std::string& phaseDescription) {
    // 【逐行批注】 计算右侧结果并写入 std::string fallbackTitle，更新当前运行状态。
    const std::string fallbackTitle = "第 " + std::to_string(index + 1) + " 步：" + (phaseName.empty() ? "理解本阶段重点" : phaseName);
    // 【逐行批注】 计算右侧结果并写入 std::string title，更新当前运行状态。
    const std::string title = safeText(step, "title", fallbackTitle);
    // 【逐行批注】 结束当前函数并返回结果：return {{"title", title},。
    return {{"title", title},
            // 【逐行批注】 调用 safeText 完成本行所需操作：{"explanation", safeText(step, "explanation", phaseDescription.empty(…。
            {"explanation", safeText(step, "explanation", phaseDescription.empty() ? "先理解本阶段核心概念，再通过练习把知识转成可执行能力。" : phaseDescription)},
            // 【逐行批注】 调用 safeText 完成本行所需操作：{"example", safeText(step, "example", "")},。
            {"example", safeText(step, "example", "")},
            // 【逐行批注】 调用 safeText 完成本行所需操作：{"action", safeText(step, "action", "完成本步骤对应练习，并记录遇到的问题。")},。
            {"action", safeText(step, "action", "完成本步骤对应练习，并记录遇到的问题。")},
            // 【逐行批注】 调用 safeText 完成本行所需操作：{"check", safeText(step, "check", "能用自己的话解释本步骤，并独立完成一个小练习。")}};。
            {"check", safeText(step, "check", "能用自己的话解释本步骤，并独立完成一个小练习。")}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 adaptedSteps，执行该函数声明的职责。
json adaptedSteps(const json& phase, const std::string& phaseName, const std::string& phaseDescription) {
    // 【逐行批注】 计算右侧结果并写入 json raw，更新当前运行状态。
    json raw = arrayOrEmpty(phase, "steps");
    // 【逐行批注】 判断条件 (raw.empty()) raw = skeletonStepsFromPhase(phase);，成立时执行对应分支。
    if (raw.empty()) raw = skeletonStepsFromPhase(phase);
    // 【逐行批注】 计算右侧结果并写入 json result，更新当前运行状态。
    json result = json::array();
    // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < raw.size() && index < 6; ++index) {。
    for (size_t index = 0; index < raw.size() && index < 6; ++index) {
        // 【逐行批注】 判断条件 (!raw.at(index).is_object()) continue;，成立时执行对应分支。
        if (!raw.at(index).is_object()) continue;
        // 【逐行批注】 调用 adaptStep 完成本行所需操作：result.push_back(adaptStep(raw.at(index), index, phaseName, phaseDesc…。
        result.push_back(adaptStep(raw.at(index), index, phaseName, phaseDescription));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 adaptRoadmap，执行该函数声明的职责。
json adaptRoadmap(const json& plan) {
    // 【逐行批注】 计算右侧结果并写入 json result，更新当前运行状态。
    json result = json::array();
    // 【逐行批注】 计算右侧结果并写入 json phases，更新当前运行状态。
    const json phases = arrayOrEmpty(plan, "phases");
    // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < phases.size(); ++index) {。
    for (size_t index = 0; index < phases.size(); ++index) {
        // 【逐行批注】 计算右侧结果并写入 auto& phase，更新当前运行状态。
        const auto& phase = phases.at(index);
        // 【逐行批注】 判断条件 (!phase.is_object()) continue;，成立时执行对应分支。
        if (!phase.is_object()) continue;
        // 【逐行批注】 计算右侧结果并写入 std::string phaseName，更新当前运行状态。
        const std::string phaseName = safeText(phase, "name", "阶段" + std::to_string(index + 1));
        // 【逐行批注】 计算右侧结果并写入 std::string phaseDescription，更新当前运行状态。
        const std::string phaseDescription = safeText(phase, "description", safeText(phase, "overview",
            // 【逐行批注】 调用 safeText 完成本行所需操作：safeText(phase, "objective", "完成关键知识学习、练习和阶段产出。")));。
            safeText(phase, "objective", "完成关键知识学习、练习和阶段产出。")));
        // 【逐行批注】 计算右侧结果并写入 std::string duration，更新当前运行状态。
        const std::string duration = safeText(phase, "duration", numberText(numberValue(phase, "durationWeeks", 2.0)) + " 周");
        // 【逐行批注】 执行当前声明或表达式：result.push_back({{"name", phaseName},。
        result.push_back({{"name", phaseName},
                          // 【逐行批注】 执行当前声明或表达式：{"duration", duration},。
                          {"duration", duration},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"goal", safeText(phase, "objective", "掌握本阶段核心能力")},。
                          {"goal", safeText(phase, "objective", "掌握本阶段核心能力")},
                          // 【逐行批注】 执行当前声明或表达式：{"description", phaseDescription},。
                          {"description", phaseDescription},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"why", safeText(phase, "why", "")},。
                          {"why", safeText(phase, "why", "")},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"output", safeText(phase, "output", "")},。
                          {"output", safeText(phase, "output", "")},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"practice", safeText(phase, "practice", "")},。
                          {"practice", safeText(phase, "practice", "")},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"checkpoint", safeText(phase, "checkpoint", "")},。
                          {"checkpoint", safeText(phase, "checkpoint", "")},
                          // 【逐行批注】 调用 stringsToJson、compactStrings 完成本行所需操作：{"commonMistakes", stringsToJson(compactStrings(phase, "commonMistake…。
                          {"commonMistakes", stringsToJson(compactStrings(phase, "commonMistakes", 8))},
                          // 【逐行批注】 调用 stringsToJson、compactStrings 完成本行所需操作：{"tasks", stringsToJson(compactStrings(phase, "tasks", 6))},。
                          {"tasks", stringsToJson(compactStrings(phase, "tasks", 6))},
                          // 【逐行批注】 调用 adaptedSteps 完成本行所需操作：{"steps", adaptedSteps(phase, phaseName, phaseDescription)}});。
                          {"steps", adaptedSteps(phase, phaseName, phaseDescription)}});
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 phaseTasks，执行该函数声明的职责。
json phaseTasks(const json& phase) {
    // 【逐行批注】 计算右侧结果并写入 auto tasks，更新当前运行状态。
    const auto tasks = compactStrings(phase, "tasks", 5);
    // 【逐行批注】 判断条件 (!tasks.empty()) return stringsToJson(tasks);，成立时执行对应分支。
    if (!tasks.empty()) return stringsToJson(tasks);
    // 【逐行批注】 结束当前函数并返回结果：return stringsToJson(compactStrings(phase, "topics", 5));。
    return stringsToJson(compactStrings(phase, "topics", 5));
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 adaptCourseStructure，执行该函数声明的职责。
json adaptCourseStructure(const json& plan) {
    // 【逐行批注】 计算右侧结果并写入 json result，更新当前运行状态。
    json result = json::array();
    // 【逐行批注】 计算右侧结果并写入 json phases，更新当前运行状态。
    const json phases = arrayOrEmpty(plan, "phases");
    // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < phases.size(); ++index) {。
    for (size_t index = 0; index < phases.size(); ++index) {
        // 【逐行批注】 计算右侧结果并写入 auto& phase，更新当前运行状态。
        const auto& phase = phases.at(index);
        // 【逐行批注】 判断条件 (!phase.is_object()) continue;，成立时执行对应分支。
        if (!phase.is_object()) continue;
        // 【逐行批注】 计算右侧结果并写入 auto topics，更新当前运行状态。
        const auto topics = compactStrings(phase, "topics", 1000);
        // 【逐行批注】 调用 safeText 完成本行所需操作：result.push_back({{"stage", safeText(phase, "name", "学习阶段")},。
        result.push_back({{"stage", safeText(phase, "name", "学习阶段")},
                          // 【逐行批注】 调用 phaseTasks、stringsToJson 完成本行所需操作：{"topics", topics.empty() ? phaseTasks(phase) : stringsToJson(topics)…。
                          {"topics", topics.empty() ? phaseTasks(phase) : stringsToJson(topics)}});
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 adaptSlide，执行该函数声明的职责。
json adaptSlide(const json& slide, size_t index) {
    // 【逐行批注】 结束当前函数并返回结果：return {{"title", safeText(slide, "title", "课程课件 " + std::to_string(i…。
    return {{"title", safeText(slide, "title", "课程课件 " + std::to_string(index + 1))},
            // 【逐行批注】 调用 safeText 完成本行所需操作：{"subtitle", safeText(slide, "subtitle", "")},。
            {"subtitle", safeText(slide, "subtitle", "")},
            // 【逐行批注】 调用 safeText 完成本行所需操作：{"content", safeText(slide, "content", "围绕本页主题理解核心概念，并通过练习完成掌握检查。")},。
            {"content", safeText(slide, "content", "围绕本页主题理解核心概念，并通过练习完成掌握检查。")},
            // 【逐行批注】 调用 stringsToJson、compactStrings 完成本行所需操作：{"bullets", stringsToJson(compactStrings(slide, "bullets", 1000))},。
            {"bullets", stringsToJson(compactStrings(slide, "bullets", 1000))},
            // 【逐行批注】 调用 safeText 完成本行所需操作：{"speakerNote", safeText(slide, "speakerNote", "")},。
            {"speakerNote", safeText(slide, "speakerNote", "")},
            // 【逐行批注】 调用 safeText 完成本行所需操作：{"relatedPhase", safeText(slide, "relatedPhase", "")}};。
            {"relatedPhase", safeText(slide, "relatedPhase", "")}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 slidesFromPhases，执行该函数声明的职责。
json slidesFromPhases(const json& plan, const json& roadmap) {
    // 【逐行批注】 计算右侧结果并写入 std::string goal，更新当前运行状态。
    const std::string goal = safeText(plan, "goal", "学习目标");
    // 【逐行批注】 计算右侧结果并写入 std::string title，更新当前运行状态。
    const std::string title = safeText(plan, "title", "目标学习路径");
    // 【逐行批注】 计算右侧结果并写入 std::string summary，更新当前运行状态。
    const std::string summary = safeText(plan, "courseIntro", safeText(plan, "overview", safeText(plan, "summary", "本课程会把目标拆成具体阶段、步骤、练习和检查标准。")));
    // 【逐行批注】 计算右侧结果并写入 json slides，更新当前运行状态。
    json slides = json::array();
    // 【逐行批注】 执行当前声明或表达式：slides.push_back({{"title", title},。
    slides.push_back({{"title", title},
                      // 【逐行批注】 执行当前声明或表达式：{"subtitle", goal},。
                      {"subtitle", goal},
                      // 【逐行批注】 执行当前声明或表达式：{"content", summary},。
                      {"content", summary},
                      // 【逐行批注】 调用 stringsToJson、compactStrings 完成本行所需操作：{"bullets", stringsToJson(compactStrings(plan, "learningOutcomes", 4)…。
                      {"bullets", stringsToJson(compactStrings(plan, "learningOutcomes", 4))},
                      // 【逐行批注】 执行当前声明或表达式：{"speakerNote", "先建立课程全局认知，再进入阶段学习；每一阶段都要完成具体练习和检查标准。"},。
                      {"speakerNote", "先建立课程全局认知，再进入阶段学习；每一阶段都要完成具体练习和检查标准。"},
                      // 【逐行批注】 执行当前声明或表达式：{"relatedPhase", ""}});。
                      {"relatedPhase", ""}});

    // 【逐行批注】 按循环规则重复处理：for (size_t phaseIndex = 0; phaseIndex < roadmap.size(); ++phaseIndex) {。
    for (size_t phaseIndex = 0; phaseIndex < roadmap.size(); ++phaseIndex) {
        // 【逐行批注】 计算右侧结果并写入 auto& phase，更新当前运行状态。
        const auto& phase = roadmap.at(phaseIndex);
        // 【逐行批注】 判断条件 (!phase.is_object()) continue;，成立时执行对应分支。
        if (!phase.is_object()) continue;
        // 【逐行批注】 计算右侧结果并写入 std::string phaseName，更新当前运行状态。
        const std::string phaseName = safeText(phase, "name", "阶段" + std::to_string(phaseIndex + 1));
        // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> bullets;。
        std::vector<std::string> bullets;
        // 【逐行批注】 计算右侧结果并写入 std::string duration，更新当前运行状态。
        const std::string duration = safeText(phase, "duration", "");
        // 【逐行批注】 计算右侧结果并写入 std::string output，更新当前运行状态。
        const std::string output = safeText(phase, "output", "");
        // 【逐行批注】 计算右侧结果并写入 std::string checkpoint，更新当前运行状态。
        const std::string checkpoint = safeText(phase, "checkpoint", "");
        // 【逐行批注】 判断条件 (!duration.empty()) bullets.push_back(duration);，成立时执行对应分支。
        if (!duration.empty()) bullets.push_back(duration);
        // 【逐行批注】 判断条件 (!output.empty()) bullets.push_back(output);，成立时执行对应分支。
        if (!output.empty()) bullets.push_back(output);
        // 【逐行批注】 判断条件 (!checkpoint.empty()) bullets.push_back(checkpoint);，成立时执行对应分支。
        if (!checkpoint.empty()) bullets.push_back(checkpoint);
        // 【逐行批注】 执行当前声明或表达式：slides.push_back({{"title", phaseName},。
        slides.push_back({{"title", phaseName},
                          // 【逐行批注】 调用 safeText、to_string 完成本行所需操作：{"subtitle", safeText(phase, "goal", safeText(phase, "output", "第 " +…。
                          {"subtitle", safeText(phase, "goal", safeText(phase, "output", "第 " + std::to_string(phaseIndex + 1) + " 阶段"))},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"content", safeText(phase, "description", "围绕「" + phaseName + "」完成具体…。
                          {"content", safeText(phase, "description", "围绕「" + phaseName + "」完成具体学习点和可检查练习。")},
                          // 【逐行批注】 调用 stringsToJson 完成本行所需操作：{"bullets", stringsToJson(bullets)},。
                          {"bullets", stringsToJson(bullets)},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"speakerNote", safeText(phase, "why", "说明这一阶段为什么重要，并提醒用户做完检查点再进入下一阶段…。
                          {"speakerNote", safeText(phase, "why", "说明这一阶段为什么重要，并提醒用户做完检查点再进入下一阶段。")},
                          // 【逐行批注】 执行当前声明或表达式：{"relatedPhase", phaseName}});。
                          {"relatedPhase", phaseName}});
        // 【逐行批注】 计算右侧结果并写入 json steps，更新当前运行状态。
        const json steps = arrayOrEmpty(phase, "steps");
        // 【逐行批注】 按循环规则重复处理：for (size_t stepIndex = 0; stepIndex < steps.size() && stepIndex < 2; ++st…。
        for (size_t stepIndex = 0; stepIndex < steps.size() && stepIndex < 2; ++stepIndex) {
            // 【逐行批注】 计算右侧结果并写入 auto& step，更新当前运行状态。
            const auto& step = steps.at(stepIndex);
            // 【逐行批注】 判断条件 (!step.is_object()) continue;，成立时执行对应分支。
            if (!step.is_object()) continue;
            // 【逐行批注】 计算右侧结果并写入 std::string stepTitle，更新当前运行状态。
            const std::string stepTitle = safeText(step, "title", "第 " + std::to_string(stepIndex + 1) + " 步");
            // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> stepBullets;。
            std::vector<std::string> stepBullets;
            // 【逐行批注】 计算右侧结果并写入 std::string example，更新当前运行状态。
            const std::string example = safeText(step, "example", "");
            // 【逐行批注】 计算右侧结果并写入 std::string action，更新当前运行状态。
            const std::string action = safeText(step, "action", "完成「" + stepTitle + "」对应练习");
            // 【逐行批注】 计算右侧结果并写入 std::string check，更新当前运行状态。
            const std::string check = safeText(step, "check", "能拿出练习结果并说明关键步骤");
            // 【逐行批注】 判断条件 (!example.empty()) stepBullets.push_back(example);，成立时执行对应分支。
            if (!example.empty()) stepBullets.push_back(example);
            // 【逐行批注】 判断条件 (!action.empty()) stepBullets.push_back("行动：" + action);，成立时执行对应分支。
            if (!action.empty()) stepBullets.push_back("行动：" + action);
            // 【逐行批注】 判断条件 (!check.empty()) stepBullets.push_back("检查：" + check);，成立时执行对应分支。
            if (!check.empty()) stepBullets.push_back("检查：" + check);
            // 【逐行批注】 执行当前声明或表达式：slides.push_back({{"title", stepTitle},。
            slides.push_back({{"title", stepTitle},
                              // 【逐行批注】 执行当前声明或表达式：{"subtitle", phaseName},。
                              {"subtitle", phaseName},
                              // 【逐行批注】 调用 safeText 完成本行所需操作：{"content", safeText(step, "explanation", safeText(phase, "descriptio…。
                              {"content", safeText(step, "explanation", safeText(phase, "description", "先理解本步骤，再完成行动建议。"))},
                              // 【逐行批注】 调用 stringsToJson 完成本行所需操作：{"bullets", stringsToJson(stepBullets)},。
                              {"bullets", stringsToJson(stepBullets)},
                              // 【逐行批注】 执行当前声明或表达式：{"speakerNote", "这一页按“讲解—例子—行动—检查”的顺序带用户学习。"},。
                              {"speakerNote", "这一页按“讲解—例子—行动—检查”的顺序带用户学习。"},
                              // 【逐行批注】 执行当前声明或表达式：{"relatedPhase", phaseName}});。
                              {"relatedPhase", phaseName}});
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (slides.size() > 12) slides.erase(slides.begin() + 12, slides.en…，成立时执行对应分支。
    if (slides.size() > 12) slides.erase(slides.begin() + 12, slides.end());
    // 【逐行批注】 结束当前函数并返回结果：return slides;。
    return slides;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 adaptSlides，执行该函数声明的职责。
json adaptSlides(const json& plan, const json& roadmap, const std::string& mode) {
    // 【逐行批注】 计算右侧结果并写入 json slides，更新当前运行状态。
    json slides = arrayOrEmpty(plan, "slides");
    // 【逐行批注】 计算右侧结果并写入 json result，更新当前运行状态。
    json result = json::array();
    // 【逐行批注】 判断条件 (!slides.empty()) {，成立时执行对应分支。
    if (!slides.empty()) {
        // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < slides.size(); ++index) {。
        for (size_t index = 0; index < slides.size(); ++index) {
            // 【逐行批注】 判断条件 (slides.at(index).is_object()) result.push_back(adaptSlide(slide…，成立时执行对应分支。
            if (slides.at(index).is_object()) result.push_back(adaptSlide(slides.at(index), index));
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：} else {。
    } else {
        // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
        result = slidesFromPhases(plan, roadmap);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：(void)mode; // 展示真实 AI 预览时保留全部阶段，旧快照格式继续兼容。。
    (void)mode;  // 展示真实 AI 预览时保留全部阶段，旧快照格式继续兼容。
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 stripStepPrefix，执行该函数声明的职责。
std::string stripStepPrefix(const std::string& value) {
    // 【逐行批注】 调用 prefix 完成本行所需操作：static const std::regex prefix("^第\\s*[0-9]+\\s*[步阶段]?[:：、-]?\\s*");。
    static const std::regex prefix("^第\\s*[0-9]+\\s*[步阶段]?[:：、-]?\\s*");
    // 【逐行批注】 结束当前函数并返回结果：return trim(std::regex_replace(value, prefix, ""));。
    return trim(std::regex_replace(value, prefix, ""));
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 slug，执行该函数声明的职责。
std::string slug(const std::string& value, const std::string& fallback) {
    // 【逐行批注】 执行当前声明或表达式：std::string result;。
    std::string result;
    // 【逐行批注】 计算右侧结果并写入 bool dash，更新当前运行状态。
    bool dash = false;
    // 【逐行批注】 按循环规则重复处理：for (unsigned char ch : value) {。
    for (unsigned char ch : value) {
        // 【逐行批注】 判断条件 (std::isalnum(ch)) {，成立时执行对应分支。
        if (std::isalnum(ch)) {
            // 【逐行批注】 调用 tolower 完成本行所需操作：result.push_back(static_cast<char>(std::tolower(ch)));。
            result.push_back(static_cast<char>(std::tolower(ch)));
            // 【逐行批注】 计算右侧结果并写入 dash，更新当前运行状态。
            dash = false;
        // 【逐行批注】 执行当前声明或表达式：} else if (!result.empty() && !dash) {。
        } else if (!result.empty() && !dash) {
            // 【逐行批注】 执行当前声明或表达式：result.push_back('-');。
            result.push_back('-');
            // 【逐行批注】 计算右侧结果并写入 dash，更新当前运行状态。
            dash = true;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 在条件成立期间循环处理：while (!result.empty() && result.back() == '-') result.pop_back();。
    while (!result.empty() && result.back() == '-') result.pop_back();
    // 【逐行批注】 判断条件 (result.empty()) return fallback;，成立时执行对应分支。
    if (result.empty()) return fallback;
    // 【逐行批注】 判断条件 (result.size() > 48) result.resize(48);，成立时执行对应分支。
    if (result.size() > 48) result.resize(48);
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 adaptMindMapNode，执行该函数声明的职责。
json adaptMindMapNode(const json& node, const std::string& fallback) {
    // 【逐行批注】 计算右侧结果并写入 json children，更新当前运行状态。
    const json children = arrayOrEmpty(node, "children");
    // 【逐行批注】 计算右侧结果并写入 json adaptedChildren，更新当前运行状态。
    json adaptedChildren = json::array();
    // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < children.size(); ++index) {。
    for (size_t index = 0; index < children.size(); ++index) {
        // 【逐行批注】 判断条件 (children.at(index).is_object()) adaptedChildren.push_back(adapt…，成立时执行对应分支。
        if (children.at(index).is_object()) adaptedChildren.push_back(adaptMindMapNode(children.at(index), fallback + "-" + std::to_string(index + 1)));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return {{"id", safeText(node, "id", fallback)},。
    return {{"id", safeText(node, "id", fallback)},
            // 【逐行批注】 调用 safeText 完成本行所需操作：{"label", safeText(node, "label", "知识点")},。
            {"label", safeText(node, "label", "知识点")},
            // 【逐行批注】 执行当前声明或表达式：{"children", adaptedChildren}};。
            {"children", adaptedChildren}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 mindMapFromPhases，执行该函数声明的职责。
json mindMapFromPhases(const json& plan) {
    // 【逐行批注】 计算右侧结果并写入 json phases，更新当前运行状态。
    const json phases = arrayOrEmpty(plan, "phases");
    // 【逐行批注】 计算右侧结果并写入 json children，更新当前运行状态。
    json children = json::array();
    // 【逐行批注】 按循环规则重复处理：for (size_t phaseIndex = 0; phaseIndex < phases.size() && phaseIndex < 6; …。
    for (size_t phaseIndex = 0; phaseIndex < phases.size() && phaseIndex < 6; ++phaseIndex) {
        // 【逐行批注】 计算右侧结果并写入 auto& phase，更新当前运行状态。
        const auto& phase = phases.at(phaseIndex);
        // 【逐行批注】 判断条件 (!phase.is_object()) continue;，成立时执行对应分支。
        if (!phase.is_object()) continue;
        // 【逐行批注】 计算右侧结果并写入 std::string phaseName，更新当前运行状态。
        const std::string phaseName = safeText(phase, "name", "阶段 " + std::to_string(phaseIndex + 1));
        // 【逐行批注】 计算右侧结果并写入 std::string phaseId，更新当前运行状态。
        const std::string phaseId = slug(phaseName, "phase-" + std::to_string(phaseIndex + 1));
        // 【逐行批注】 计算右侧结果并写入 json labels，更新当前运行状态。
        json labels = json::array();
        // 【逐行批注】 计算右侧结果并写入 json steps，更新当前运行状态。
        const json steps = arrayOrEmpty(phase, "steps");
        // 【逐行批注】 判断条件 (!steps.empty()) {，成立时执行对应分支。
        if (!steps.empty()) {
            // 【逐行批注】 按循环规则重复处理：for (const auto& step : steps) {。
            for (const auto& step : steps) {
                // 【逐行批注】 判断条件 (!step.is_object() || labels.size() >= 5) continue;，成立时执行对应分支。
                if (!step.is_object() || labels.size() >= 5) continue;
                // 【逐行批注】 计算右侧结果并写入 std::string label，更新当前运行状态。
                const std::string label = stripStepPrefix(safeText(step, "title", "学习步骤"));
                // 【逐行批注】 判断条件 (!label.empty()) labels.push_back(label);，成立时执行对应分支。
                if (!label.empty()) labels.push_back(label);
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 执行当前声明或表达式：} else {。
        } else {
            // 【逐行批注】 按循环规则重复处理：for (const auto& topic : compactStrings(phase, "topics", 5)) labels.push_b…。
            for (const auto& topic : compactStrings(phase, "topics", 5)) labels.push_back(topic);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 json phaseChildren，更新当前运行状态。
        json phaseChildren = json::array();
        // 【逐行批注】 按循环规则重复处理：for (size_t childIndex = 0; childIndex < labels.size() && childIndex < 5; …。
        for (size_t childIndex = 0; childIndex < labels.size() && childIndex < 5; ++childIndex) {
            // 【逐行批注】 调用 to_string 完成本行所需操作：phaseChildren.push_back({{"id", phaseId + "-" + std::to_string(childI…。
            phaseChildren.push_back({{"id", phaseId + "-" + std::to_string(childIndex + 1)},
                                     // 【逐行批注】 执行当前声明或表达式：{"label", labels.at(childIndex)},。
                                     {"label", labels.at(childIndex)},
                                     // 【逐行批注】 调用 array 完成本行所需操作：{"children", json::array()}});。
                                     {"children", json::array()}});
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：children.push_back({{"id", phaseId}, {"label", phaseName}, {"children", phaseChildre…。
        children.push_back({{"id", phaseId}, {"label", phaseName}, {"children", phaseChildren}});
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return {{"title", "课程知识结构"},。
    return {{"title", "课程知识结构"},
            // 【逐行批注】 调用 array 完成本行所需操作：{"nodes", json::array({{{"id", "root"},。
            {"nodes", json::array({{{"id", "root"},
                                     // 【逐行批注】 调用 safeText 完成本行所需操作：{"label", safeText(plan, "goal", safeText(plan, "title", "钢一定制AI 课程")…。
                                     {"label", safeText(plan, "goal", safeText(plan, "title", "钢一定制AI 课程"))},
                                     // 【逐行批注】 执行当前声明或表达式：{"children", children}}})}};。
                                     {"children", children}}})}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 adaptMindMap，执行该函数声明的职责。
json adaptMindMap(const json& plan) {
    // 【逐行批注】 计算右侧结果并写入 json map，更新当前运行状态。
    const json map = plan.is_object() && plan.contains("mindMap") && plan.at("mindMap").is_object() ? plan.at("mindMap") : json::object();
    // 【逐行批注】 计算右侧结果并写入 json nodes，更新当前运行状态。
    const json nodes = arrayOrEmpty(map, "nodes");
    // 【逐行批注】 判断条件 (map.is_object() && !nodes.empty()) {，成立时执行对应分支。
    if (map.is_object() && !nodes.empty()) {
        // 【逐行批注】 计算右侧结果并写入 json adapted，更新当前运行状态。
        json adapted = json::array();
        // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < nodes.size(); ++index) {。
        for (size_t index = 0; index < nodes.size(); ++index) {
            // 【逐行批注】 判断条件 (nodes.at(index).is_object()) adapted.push_back(adaptMindMapNode…，成立时执行对应分支。
            if (nodes.at(index).is_object()) adapted.push_back(adaptMindMapNode(nodes.at(index), "node-" + std::to_string(index + 1)));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return {{"title", safeText(map, "title", "课程知识结构")}, {"nodes", adapte…。
        return {{"title", safeText(map, "title", "课程知识结构")}, {"nodes", adapted}};
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return mindMapFromPhases(plan);。
    return mindMapFromPhases(plan);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 adaptResources，执行该函数声明的职责。
json adaptResources(const json& plan) {
    // 【逐行批注】 计算右侧结果并写入 json result，更新当前运行状态。
    json result = json::array();
    // 【逐行批注】 计算右侧结果并写入 json resources，更新当前运行状态。
    const json resources = arrayOrEmpty(plan, "resources");
    // 【逐行批注】 按循环规则重复处理：for (const auto& resource : resources) {。
    for (const auto& resource : resources) {
        // 【逐行批注】 判断条件 (!resource.is_object()) continue;，成立时执行对应分支。
        if (!resource.is_object()) continue;
        // 【逐行批注】 调用 safeText 完成本行所需操作：result.push_back({{"name", safeText(resource, "name", "学习资源")},。
        result.push_back({{"name", safeText(resource, "name", "学习资源")},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"type", safeText(resource, "type", "图文教程")},。
                          {"type", safeText(resource, "type", "图文教程")},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"difficulty", safeText(resource, "difficulty", "入门")},。
                          {"difficulty", safeText(resource, "difficulty", "入门")},
                          // 【逐行批注】 执行当前声明或表达式：{"free", resource.contains("free") && resource.at("free").is_boolean() ? resource.at…。
                          {"free", resource.contains("free") && resource.at("free").is_boolean() ? resource.at("free").get<bool>() : false},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"description", safeText(resource, "description", "适合作为当前目标的补充学习资料。")…。
                          {"description", safeText(resource, "description", "适合作为当前目标的补充学习资料。")},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"href", safeText(resource, "url", safeText(resource, "href", "#"))}}…。
                          {"href", safeText(resource, "url", safeText(resource, "href", "#"))}});
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 adaptProjects，执行该函数声明的职责。
json adaptProjects(const json& plan) {
    // 【逐行批注】 计算右侧结果并写入 json result，更新当前运行状态。
    json result = json::array();
    // 【逐行批注】 计算右侧结果并写入 json projects，更新当前运行状态。
    const json projects = arrayOrEmpty(plan, "projects");
    // 【逐行批注】 按循环规则重复处理：for (const auto& project : projects) {。
    for (const auto& project : projects) {
        // 【逐行批注】 判断条件 (!project.is_object()) continue;，成立时执行对应分支。
        if (!project.is_object()) continue;
        // 【逐行批注】 计算右侧结果并写入 double hours，更新当前运行状态。
        const double hours = numberValue(project, "estimatedHours", 4.0);
        // 【逐行批注】 计算右侧结果并写入 auto criteria，更新当前运行状态。
        const auto criteria = compactStrings(project, "acceptanceCriteria", 1000);
        // 【逐行批注】 执行当前声明或表达式：std::ostringstream acceptance;。
        std::ostringstream acceptance;
        // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < criteria.size(); ++index) {。
        for (size_t index = 0; index < criteria.size(); ++index) {
            // 【逐行批注】 判断条件 (index > 0) acceptance << u8"；";，成立时执行对应分支。
            if (index > 0) acceptance << u8"；";
            // 【逐行批注】 执行当前声明或表达式：acceptance << criteria[index];。
            acceptance << criteria[index];
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (criteria.empty()) acceptance << "目标明确、过程可复盘、结果可展示。";，成立时执行对应分支。
        if (criteria.empty()) acceptance << "目标明确、过程可复盘、结果可展示。";
        // 【逐行批注】 调用 safeText 完成本行所需操作：result.push_back({{"name", safeText(project, "name", "阶段实践项目")},。
        result.push_back({{"name", safeText(project, "name", "阶段实践项目")},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"difficulty", safeText(project, "difficulty", "入门")},。
                          {"difficulty", safeText(project, "difficulty", "入门")},
                          // 【逐行批注】 调用 numberText 完成本行所需操作：{"duration", numberText(hours) + " 小时"},。
                          {"duration", numberText(hours) + " 小时"},
                          // 【逐行批注】 调用 safeText 完成本行所需操作：{"output", safeText(project, "output", "一个可检查的练习成果。")},。
                          {"output", safeText(project, "output", "一个可检查的练习成果。")},
                          // 【逐行批注】 执行当前声明或表达式：{"acceptance", acceptance.str()}});。
                          {"acceptance", acceptance.str()}});
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 定义函数 adaptGeneratedPlan，执行该函数声明的职责。
nlohmann::json adaptGeneratedPlan(const json& plan, const std::string& mode) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 std::string normalizedMode，更新当前运行状态。
        const std::string normalizedMode = mode == "lite" ? "lite" : "deep";
        // 【逐行批注】 计算右侧结果并写入 std::string goal，更新当前运行状态。
        const std::string goal = safeText(plan, "learnerGoal", safeText(plan, "goal", safeText(plan, "title", "你的目标")));
        // 【逐行批注】 计算右侧结果并写入 std::string titleFallback，更新当前运行状态。
        const std::string titleFallback = fallbackTitle(goal, normalizedMode);
        // 【逐行批注】 计算右侧结果并写入 std::string summaryFallback，更新当前运行状态。
        const std::string summaryFallback = normalizedMode == "lite"
            // 【逐行批注】 执行当前声明或表达式：? "这是一份快速可执行方案，优先给出核心步骤、练习方法、检查标准和常见错误。"。
            ? "这是一份快速可执行方案，优先给出核心步骤、练习方法、检查标准和常见错误。"
            // 【逐行批注】 执行当前声明或表达式：: "围绕你的目标生成阶段化学习路线。";。
            : "围绕你的目标生成阶段化学习路线。";
        // 【逐行批注】 计算右侧结果并写入 std::string title，更新当前运行状态。
        const std::string title = safeText(plan, "courseTitle", safeText(plan, "title", titleFallback));
        // 【逐行批注】 计算右侧结果并写入 std::string summary，更新当前运行状态。
        const std::string summary = safeText(plan, "courseSummary", safeText(plan, "summary", safeText(plan, "courseIntro", safeText(plan, "overview", summaryFallback))));
        // 【逐行批注】 计算右侧结果并写入 json roadmap，更新当前运行状态。
        const json roadmap = adaptRoadmap(plan);
        // 【逐行批注】 计算右侧结果并写入 double rawWeeks，更新当前运行状态。
        const double rawWeeks = numberValue(plan, "durationWeeks", normalizedMode == "lite" ? 1.0 : static_cast<double>(roadmap.size() * 2));
        // 【逐行批注】 计算右侧结果并写入 double weeks，更新当前运行状态。
        const double weeks = normalizedMode == "lite" ? std::clamp(rawWeeks, 1.0, 2.0) : rawWeeks;

        // 【逐行批注】 执行当前声明或表达式：json output;。
        json output;
        // 【逐行批注】 计算右侧结果并写入 output["title"]，更新当前运行状态。
        output["title"] = title;
        // 【逐行批注】 计算右侧结果并写入 output["duration"]，更新当前运行状态。
        output["duration"] = numberText(weeks) + " 周";
        // 【逐行批注】 计算右侧结果并写入 output["summary"]，更新当前运行状态。
        output["summary"] = summary;
        // 【逐行批注】 计算右侧结果并写入 output["courseIntro"]，更新当前运行状态。
        output["courseIntro"] = safeText(plan, "courseIntro", safeText(plan, "overview", normalizedMode == "lite"
            // 【逐行批注】 执行当前声明或表达式：? "这份快速规划会用更少步骤告诉你马上怎么做、怎么练、怎么检查。"。
            ? "这份快速规划会用更少步骤告诉你马上怎么做、怎么练、怎么检查。"
            // 【逐行批注】 执行当前声明或表达式：: "这门课程会通过阶段导学、分步讲解、练习和检查点帮助你真正掌握目标。"));。
            : "这门课程会通过阶段导学、分步讲解、练习和检查点帮助你真正掌握目标。"));
        // 【逐行批注】 计算右侧结果并写入 output["overview"]，更新当前运行状态。
        output["overview"] = safeText(plan, "overview", normalizedMode == "lite"
            // 【逐行批注】 执行当前声明或表达式：? "从准备、核心步骤、练习、自检和常见错误快速推进。"。
            ? "从准备、核心步骤、练习、自检和常见错误快速推进。"
            // 【逐行批注】 执行当前声明或表达式：: "从目标拆解、核心知识、练习任务到阶段产出，逐步推进学习。");。
            : "从目标拆解、核心知识、练习任务到阶段产出，逐步推进学习。");
        // 【逐行批注】 计算右侧结果并写入 output["audience"]，更新当前运行状态。
        output["audience"] = safeText(plan, "audience", normalizedMode == "lite"
            // 【逐行批注】 执行当前声明或表达式：? "希望快速上手并获得可执行步骤的学习者。"。
            ? "希望快速上手并获得可执行步骤的学习者。"
            // 【逐行批注】 执行当前声明或表达式：: "希望系统学习并通过练习获得实际产出的学习者。" );。
            : "希望系统学习并通过练习获得实际产出的学习者。" );
        // 【逐行批注】 计算右侧结果并写入 output["prerequisites"]，更新当前运行状态。
        output["prerequisites"] = stringsToJson(compactStrings(plan, "prerequisites", 1000));
        // 【逐行批注】 计算右侧结果并写入 output["outcome"]，更新当前运行状态。
        output["outcome"] = safeText(plan, "outcome", "完成后能够形成可展示的学习成果，并知道下一步如何继续提升。" );
        // 【逐行批注】 计算右侧结果并写入 output["learningOutcomes"]，更新当前运行状态。
        output["learningOutcomes"] = stringsToJson(compactStrings(plan, "learningOutcomes", 1000));
        // 【逐行批注】 计算右侧结果并写入 output["slides"]，更新当前运行状态。
        output["slides"] = adaptSlides(plan, roadmap, normalizedMode);
        // 【逐行批注】 计算右侧结果并写入 output["mindMap"]，更新当前运行状态。
        output["mindMap"] = adaptMindMap(plan);
        // 【逐行批注】 计算右侧结果并写入 output["roadmap"]，更新当前运行状态。
        output["roadmap"] = roadmap;
        // 【逐行批注】 计算右侧结果并写入 output["courseStructure"]，更新当前运行状态。
        output["courseStructure"] = adaptCourseStructure(plan);
        // 【逐行批注】 计算右侧结果并写入 output["resources"]，更新当前运行状态。
        output["resources"] = adaptResources(plan);
        // 【逐行批注】 计算右侧结果并写入 output["projects"]，更新当前运行状态。
        output["projects"] = adaptProjects(plan);
        // 【逐行批注】 结束当前函数并返回结果：return output;。
        return output;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return json::object();。
        return json::object();
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 isRenderablePlan，执行该函数声明的职责。
bool isRenderablePlan(const json& plan) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 结束当前函数并返回结果：return plan.is_object() && safeText(plan, "title").size() > 0 && safe…。
        return plan.is_object() && safeText(plan, "title").size() > 0 && safeText(plan, "summary").size() > 0 &&
               // 【逐行批注】 执行当前声明或表达式：plan.contains("roadmap") && plan.at("roadmap").is_array() && !plan.at("roadmap").emp…。
               plan.contains("roadmap") && plan.at("roadmap").is_array() && !plan.at("roadmap").empty() &&
               // 【逐行批注】 执行当前声明或表达式：plan.contains("courseStructure") && plan.at("courseStructure").is_array() && !plan.a…。
               plan.contains("courseStructure") && plan.at("courseStructure").is_array() && !plan.at("courseStructure").empty();
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

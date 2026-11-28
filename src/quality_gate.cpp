// 【逐行批注】 引入头文件 "quality_gate.hpp"，为后续实现提供类型或接口声明。
#include "quality_gate.hpp"

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <regex>，为后续实现提供类型或接口声明。
#include <regex>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
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

// 【逐行批注】 定义函数 lower，执行该函数声明的职责。
std::string lower(std::string value) {
    // 【逐行批注】 按循环规则重复处理：for (char& ch : value) ch = static_cast<char>(std::tolower(static_cast<uns…。
    for (char& ch : value) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 goalKeywords，执行该函数声明的职责。
std::vector<std::string> goalKeywords(const std::string& goal) {
    // 按 UTF-8 字符切分（中文 3 字节/字符、英文 1 字节/字符），避免把汉字切半
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> chars;。
    std::vector<std::string> chars;
    // 【逐行批注】 计算右侧结果并写入 std::string text，更新当前运行状态。
    const std::string text = lower(trim(goal));
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < text.size();) {。
    for (size_t i = 0; i < text.size();) {
        // 【逐行批注】 计算右侧结果并写入 unsigned char c，更新当前运行状态。
        const unsigned char c = static_cast<unsigned char>(text[i]);
        // 【逐行批注】 计算右侧结果并写入 size_t len，更新当前运行状态。
        size_t len = 1;
        // 【逐行批注】 判断条件 (c >= 0xF0) len = 4;，成立时执行对应分支。
        if (c >= 0xF0) len = 4;
        // 【逐行批注】 在前一条件不成立时继续判断：(c >= 0xE0) len = 3;。
        else if (c >= 0xE0) len = 3;
        // 【逐行批注】 在前一条件不成立时继续判断：(c >= 0xC0) len = 2;。
        else if (c >= 0xC0) len = 2;
        // 【逐行批注】 执行当前声明或表达式：chars.push_back(text.substr(i, len));。
        chars.push_back(text.substr(i, len));
        // 【逐行批注】 计算右侧结果并写入 i +，更新当前运行状态。
        i += len;
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> grams;。
    std::vector<std::string> grams;
    // 中文/混排：前 2-4 个字符的 n-gram
    // 【逐行批注】 按循环规则重复处理：for (size_t n = 2; n <= 4 && chars.size() >= n; ++n) {。
    for (size_t n = 2; n <= 4 && chars.size() >= n; ++n) {
        // 【逐行批注】 执行当前声明或表达式：std::string gram;。
        std::string gram;
        // 【逐行批注】 按循环规则重复处理：for (size_t k = 0; k < n; ++k) gram += chars[k];。
        for (size_t k = 0; k < n; ++k) gram += chars[k];
        // 【逐行批注】 执行当前声明或表达式：grams.push_back(gram);。
        grams.push_back(gram);
    // 【逐行批注】 结束当前代码块。
    }
    // ASCII 整词（如 python / excel）也加入
    // 【逐行批注】 执行当前声明或表达式：std::string asciiWord;。
    std::string asciiWord;
    // 【逐行批注】 按循环规则重复处理：for (const auto& ch : chars) {。
    for (const auto& ch : chars) {
        // 【逐行批注】 计算右侧结果并写入 unsigned char b，更新当前运行状态。
        const unsigned char b = static_cast<unsigned char>(ch[0]);
        // 【逐行批注】 判断条件 (b < 0x80 && std::isalnum(b)) {，成立时执行对应分支。
        if (b < 0x80 && std::isalnum(b)) {
            // 【逐行批注】 计算右侧结果并写入 asciiWord +，更新当前运行状态。
            asciiWord += ch;
        // 【逐行批注】 执行当前声明或表达式：} else if (!asciiWord.empty()) {。
        } else if (!asciiWord.empty()) {
            // 【逐行批注】 判断条件 (asciiWord.size() >= 3) grams.push_back(asciiWord);，成立时执行对应分支。
            if (asciiWord.size() >= 3) grams.push_back(asciiWord);
            // 【逐行批注】 执行当前声明或表达式：asciiWord.clear();。
            asciiWord.clear();
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!asciiWord.empty() && asciiWord.size() >= 3) grams.push_back(as…，成立时执行对应分支。
    if (!asciiWord.empty() && asciiWord.size() >= 3) grams.push_back(asciiWord);

    // 【逐行批注】 调用 sort 完成本行所需操作：std::sort(grams.begin(), grams.end());。
    std::sort(grams.begin(), grams.end());
    // 【逐行批注】 调用 unique 完成本行所需操作：grams.erase(std::unique(grams.begin(), grams.end()), grams.end());。
    grams.erase(std::unique(grams.begin(), grams.end()), grams.end());
    // 【逐行批注】 判断条件 (grams.size() > 6) grams.resize(6);，成立时执行对应分支。
    if (grams.size() > 6) grams.resize(6);
    // 【逐行批注】 结束当前函数并返回结果：return grams;。
    return grams;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 containsAny，执行该函数声明的职责。
bool containsAny(const std::string& value, const std::vector<std::string>& needles) {
    // 【逐行批注】 按循环规则重复处理：for (const auto& needle : needles) {。
    for (const auto& needle : needles) {
        // 【逐行批注】 判断条件 (!needle.empty() && value.find(needle) != std::string::npos) ret…，成立时执行对应分支。
        if (!needle.empty() && value.find(needle) != std::string::npos) return true;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return false;。
    return false;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 isGenericSentence，执行该函数声明的职责。
bool isGenericSentence(const std::string& text) {
    // 【逐行批注】 执行当前声明或表达式：static const std::vector<std::string> patterns = {。
    static const std::vector<std::string> patterns = {
        // 【逐行批注】 执行当前声明或表达式："关键抓手", "不要只背名词", "理解核心目标", "掌握基本概念", "提升综合能力", "深入学习相关知识",。
        "关键抓手", "不要只背名词", "理解核心目标", "掌握基本概念", "提升综合能力", "深入学习相关知识",
        // 【逐行批注】 执行当前声明或表达式："多加练习", "参考相关资料", "只给链接", "只列大纲", "课程导入", "至少完成一次解释和练习",。
        "多加练习", "参考相关资料", "只给链接", "只列大纲", "课程导入", "至少完成一次解释和练习",
        // 【逐行批注】 执行当前声明或表达式："理解阶段目标", "用练习把知识变成能力", "复盘并形成阶段产出", "建立学习节奏", "完成一次输出"。
        "理解阶段目标", "用练习把知识变成能力", "复盘并形成阶段产出", "建立学习节奏", "完成一次输出"
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 结束当前函数并返回结果：return containsAny(text, patterns);。
    return containsAny(text, patterns);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 countOverlap，执行该函数声明的职责。
int countOverlap(const std::string& text, const std::vector<std::string>& grams) {
    // 【逐行批注】 计算右侧结果并写入 int count，更新当前运行状态。
    int count = 0;
    // 【逐行批注】 按循环规则重复处理：for (const auto& gram : grams) {。
    for (const auto& gram : grams) {
        // 【逐行批注】 判断条件 (!gram.empty() && text.find(gram) != std::string::npos) ++count;，成立时执行对应分支。
        if (!gram.empty() && text.find(gram) != std::string::npos) ++count;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return count;。
    return count;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 collectTexts，执行该函数声明的职责。
std::vector<std::string> collectTexts(const json& plan) {
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> texts;。
    std::vector<std::string> texts;
    // 【逐行批注】 判断条件 (!plan.is_object()) return texts;，成立时执行对应分支。
    if (!plan.is_object()) return texts;
    // 【逐行批注】 判断条件 (plan.contains("summary") && plan["summary"].is_string()) texts.…，成立时执行对应分支。
    if (plan.contains("summary") && plan["summary"].is_string()) texts.push_back(plan["summary"].get<std::string>());
    // 【逐行批注】 判断条件 (plan.contains("courseIntro") && plan["courseIntro"].is_string()…，成立时执行对应分支。
    if (plan.contains("courseIntro") && plan["courseIntro"].is_string()) texts.push_back(plan["courseIntro"].get<std::string>());
    // 【逐行批注】 判断条件 (plan.contains("overview") && plan["overview"].is_string()) text…，成立时执行对应分支。
    if (plan.contains("overview") && plan["overview"].is_string()) texts.push_back(plan["overview"].get<std::string>());
    // 【逐行批注】 判断条件 (plan.contains("roadmap") && plan["roadmap"].is_array()) {，成立时执行对应分支。
    if (plan.contains("roadmap") && plan["roadmap"].is_array()) {
        // 【逐行批注】 按循环规则重复处理：for (const auto& phase : plan["roadmap"]) {。
        for (const auto& phase : plan["roadmap"]) {
            // 【逐行批注】 判断条件 (!phase.is_object()) continue;，成立时执行对应分支。
            if (!phase.is_object()) continue;
            // 【逐行批注】 判断条件 (phase.contains("description") && phase["description"].is_string…，成立时执行对应分支。
            if (phase.contains("description") && phase["description"].is_string()) texts.push_back(phase["description"].get<std::string>());
            // 【逐行批注】 判断条件 (phase.contains("goal") && phase["goal"].is_string()) texts.push…，成立时执行对应分支。
            if (phase.contains("goal") && phase["goal"].is_string()) texts.push_back(phase["goal"].get<std::string>());
            // 【逐行批注】 判断条件 (phase.contains("output") && phase["output"].is_string()) texts.…，成立时执行对应分支。
            if (phase.contains("output") && phase["output"].is_string()) texts.push_back(phase["output"].get<std::string>());
            // 【逐行批注】 判断条件 (phase.contains("steps") && phase["steps"].is_array()) {，成立时执行对应分支。
            if (phase.contains("steps") && phase["steps"].is_array()) {
                // 【逐行批注】 按循环规则重复处理：for (const auto& step : phase["steps"]) {。
                for (const auto& step : phase["steps"]) {
                    // 【逐行批注】 判断条件 (!step.is_object()) continue;，成立时执行对应分支。
                    if (!step.is_object()) continue;
                    // 【逐行批注】 判断条件 (step.contains("title") && step["title"].is_string()) texts.push…，成立时执行对应分支。
                    if (step.contains("title") && step["title"].is_string()) texts.push_back(step["title"].get<std::string>());
                    // 【逐行批注】 判断条件 (step.contains("explanation") && step["explanation"].is_string()…，成立时执行对应分支。
                    if (step.contains("explanation") && step["explanation"].is_string()) texts.push_back(step["explanation"].get<std::string>());
                    // 【逐行批注】 判断条件 (step.contains("action") && step["action"].is_string()) texts.pu…，成立时执行对应分支。
                    if (step.contains("action") && step["action"].is_string()) texts.push_back(step["action"].get<std::string>());
                    // 【逐行批注】 判断条件 (step.contains("check") && step["check"].is_string()) texts.push…，成立时执行对应分支。
                    if (step.contains("check") && step["check"].is_string()) texts.push_back(step["check"].get<std::string>());
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (plan.contains("courseStructure") && plan["courseStructure"].is_…，成立时执行对应分支。
    if (plan.contains("courseStructure") && plan["courseStructure"].is_array()) {
        // 【逐行批注】 按循环规则重复处理：for (const auto& stage : plan["courseStructure"]) {。
        for (const auto& stage : plan["courseStructure"]) {
            // 【逐行批注】 判断条件 (!stage.is_object()) continue;，成立时执行对应分支。
            if (!stage.is_object()) continue;
            // 【逐行批注】 判断条件 (stage.contains("stage") && stage["stage"].is_string()) texts.pu…，成立时执行对应分支。
            if (stage.contains("stage") && stage["stage"].is_string()) texts.push_back(stage["stage"].get<std::string>());
            // 【逐行批注】 判断条件 (stage.contains("topics") && stage["topics"].is_array()) {，成立时执行对应分支。
            if (stage.contains("topics") && stage["topics"].is_array()) {
                // 【逐行批注】 按循环规则重复处理：for (const auto& topic : stage["topics"]) if (topic.is_string()) texts.pus…。
                for (const auto& topic : stage["topics"]) if (topic.is_string()) texts.push_back(topic.get<std::string>());
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (plan.contains("slides") && plan["slides"].is_array()) {，成立时执行对应分支。
    if (plan.contains("slides") && plan["slides"].is_array()) {
        // 【逐行批注】 按循环规则重复处理：for (const auto& slide : plan["slides"]) {。
        for (const auto& slide : plan["slides"]) {
            // 【逐行批注】 判断条件 (!slide.is_object()) continue;，成立时执行对应分支。
            if (!slide.is_object()) continue;
            // 【逐行批注】 判断条件 (slide.contains("title") && slide["title"].is_string()) texts.pu…，成立时执行对应分支。
            if (slide.contains("title") && slide["title"].is_string()) texts.push_back(slide["title"].get<std::string>());
            // 【逐行批注】 判断条件 (slide.contains("content") && slide["content"].is_string()) text…，成立时执行对应分支。
            if (slide.contains("content") && slide["content"].is_string()) texts.push_back(slide["content"].get<std::string>());
            // 【逐行批注】 判断条件 (slide.contains("bullets") && slide["bullets"].is_array()) {，成立时执行对应分支。
            if (slide.contains("bullets") && slide["bullets"].is_array()) {
                // 【逐行批注】 按循环规则重复处理：for (const auto& bullet : slide["bullets"]) if (bullet.is_string()) texts.…。
                for (const auto& bullet : slide["bullets"]) if (bullet.is_string()) texts.push_back(bullet.get<std::string>());
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return texts;。
    return texts;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 定义函数 validateCourseContent，执行该函数声明的职责。
QualityResult validateCourseContent(const json& plan, const std::string& goal, const std::string& mode, const std::string& courseTitle) {
    // 【逐行批注】 执行当前声明或表达式：QualityResult result;。
    QualityResult result;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (!plan.is_object()) {，成立时执行对应分支。
        if (!plan.is_object()) {
            // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("课程内容不是对象结构。");。
            result.reasons.push_back("课程内容不是对象结构。");
            // 【逐行批注】 计算右侧结果并写入 result.score，更新当前运行状态。
            result.score = 0;
            // 【逐行批注】 结束当前函数并返回结果：return result;。
            return result;
        // 【逐行批注】 结束当前代码块。
        }

        // 【逐行批注】 计算右侧结果并写入 std::string goalText，更新当前运行状态。
        const std::string goalText = lower(trim(goal));
        // 【逐行批注】 调用 goalKeywords 完成本行所需操作：const std::vector<std::string> grams = goalKeywords(goal);。
        const std::vector<std::string> grams = goalKeywords(goal);
        // 【逐行批注】 计算右侧结果并写入 std::string mergedTexts，更新当前运行状态。
        const std::string mergedTexts = lower(trim(courseTitle + " " + goal));
        // 【逐行批注】 执行当前声明或表达式：std::set<std::string> genericTexts;。
        std::set<std::string> genericTexts;
        // 【逐行批注】 计算右侧结果并写入 int keywordMiss，更新当前运行状态。
        int keywordMiss = 0;
        // 【逐行批注】 计算右侧结果并写入 int repetitionPenalty，更新当前运行状态。
        int repetitionPenalty = 0;
        // 【逐行批注】 计算右侧结果并写入 int actionPenalty，更新当前运行状态。
        int actionPenalty = 0;

        // 【逐行批注】 判断条件 (plan.contains("roadmap") && plan["roadmap"].is_array()) {，成立时执行对应分支。
        if (plan.contains("roadmap") && plan["roadmap"].is_array()) {
            // 【逐行批注】 计算右侧结果并写入 int phaseCount，更新当前运行状态。
            const int phaseCount = static_cast<int>(plan["roadmap"].size());
            // 【逐行批注】 判断条件 (mode == "lite" && (phaseCount < 3 || phaseCount > 5)) {，成立时执行对应分支。
            if (mode == "lite" && (phaseCount < 3 || phaseCount > 5)) {
                // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("lite 模式阶段数应为 3-5。");。
                result.reasons.push_back("lite 模式阶段数应为 3-5。");
                // 【逐行批注】 计算右侧结果并写入 result.valid，更新当前运行状态。
                result.valid = false;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (mode != "lite" && (phaseCount < 4 || phaseCount > 6)) {，成立时执行对应分支。
            if (mode != "lite" && (phaseCount < 4 || phaseCount > 6)) {
                // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("deep 模式阶段数应为 4-6。");。
                result.reasons.push_back("deep 模式阶段数应为 4-6。");
                // 【逐行批注】 计算右侧结果并写入 result.valid，更新当前运行状态。
                result.valid = false;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 执行当前声明或表达式：std::set<std::string> uniqueTexts;。
            std::set<std::string> uniqueTexts;
            // 【逐行批注】 计算右侧结果并写入 size_t textCount，更新当前运行状态。
            size_t textCount = 0;
            // 【逐行批注】 计算右侧结果并写入 size_t uniqueCount，更新当前运行状态。
            size_t uniqueCount = 0;
            // 【逐行批注】 按循环规则重复处理：for (const auto& phase : plan["roadmap"]) {。
            for (const auto& phase : plan["roadmap"]) {
                // 【逐行批注】 判断条件 (!phase.is_object()) continue;，成立时执行对应分支。
                if (!phase.is_object()) continue;
                // 【逐行批注】 计算右侧结果并写入 std::string phaseName，更新当前运行状态。
                const std::string phaseName = lower(trim(phase.value("name", "")));
                // 【逐行批注】 计算右侧结果并写入 std::string phaseDesc，更新当前运行状态。
                const std::string phaseDesc = lower(trim(phase.value("description", "")));
                // 【逐行批注】 判断条件 (isGenericSentence(phaseName)) genericTexts.insert(phaseName);，成立时执行对应分支。
                if (isGenericSentence(phaseName)) genericTexts.insert(phaseName);
                // 【逐行批注】 判断条件 (isGenericSentence(phaseDesc)) genericTexts.insert(phaseDesc);，成立时执行对应分支。
                if (isGenericSentence(phaseDesc)) genericTexts.insert(phaseDesc);
                // 【逐行批注】 执行当前声明或表达式：uniqueTexts.insert(phaseName + "|" + phaseDesc);。
                uniqueTexts.insert(phaseName + "|" + phaseDesc);
                // 【逐行批注】 判断条件 (phase.contains("topics") && phase["topics"].is_array() && phase…，成立时执行对应分支。
                if (phase.contains("topics") && phase["topics"].is_array() && phase["topics"].size() < 2) {
                    // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("某阶段 topics 少于 2，内容过薄。");。
                    result.reasons.push_back("某阶段 topics 少于 2，内容过薄。");
                    // 【逐行批注】 计算右侧结果并写入 result.valid，更新当前运行状态。
                    result.valid = false;
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 判断条件 (phase.contains("steps") && phase["steps"].is_array()) {，成立时执行对应分支。
                if (phase.contains("steps") && phase["steps"].is_array()) {
                    // 【逐行批注】 按循环规则重复处理：for (const auto& step : phase["steps"]) {。
                    for (const auto& step : phase["steps"]) {
                        // 【逐行批注】 判断条件 (!step.is_object()) continue;，成立时执行对应分支。
                        if (!step.is_object()) continue;
                        // 【逐行批注】 计算右侧结果并写入 auto strOf，更新当前运行状态。
                        const auto strOf = [](const json& obj, const char* key) {
                            // 【逐行批注】 结束当前函数并返回结果：return obj.contains(key) && obj[key].is_string() ? obj[key].get<std::…。
                            return obj.contains(key) && obj[key].is_string() ? obj[key].get<std::string>() : std::string{};
                        // 【逐行批注】 执行当前声明或表达式：};。
                        };
                        // 【逐行批注】 计算右侧结果并写入 std::string stepText，更新当前运行状态。
                        const std::string stepText = lower(trim(
                            // 【逐行批注】 调用 strOf 完成本行所需操作：strOf(step, "title") + " " + strOf(step, "explanation") + " " +。
                            strOf(step, "title") + " " + strOf(step, "explanation") + " " +
                            // 【逐行批注】 调用 strOf 完成本行所需操作：strOf(step, "action") + " " + strOf(step, "check")));。
                            strOf(step, "action") + " " + strOf(step, "check")));
                        // 【逐行批注】 计算右侧结果并写入 std::string joined，更新当前运行状态。
                        const std::string joined = stepText;
                        // 【逐行批注】 执行当前声明或表达式：++textCount;。
                        ++textCount;
                        // 【逐行批注】 判断条件 (uniqueTexts.insert(joined).second) ++uniqueCount;，成立时执行对应分支。
                        if (uniqueTexts.insert(joined).second) ++uniqueCount;
                        // 【逐行批注】 判断条件 (isGenericSentence(joined)) genericTexts.insert(joined);，成立时执行对应分支。
                        if (isGenericSentence(joined)) genericTexts.insert(joined);
                    // 【逐行批注】 结束当前代码块。
                    }
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 double uniqueRatio，更新当前运行状态。
            const double uniqueRatio = textCount == 0 ? 1.0 : static_cast<double>(uniqueCount) / static_cast<double>(textCount);
            // 【逐行批注】 判断条件 (uniqueRatio < 0.55) {，成立时执行对应分支。
            if (uniqueRatio < 0.55) {
                // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("重复内容过高，唯一文本占比不足 55%。");。
                result.reasons.push_back("重复内容过高，唯一文本占比不足 55%。");
                // 【逐行批注】 计算右侧结果并写入 result.valid，更新当前运行状态。
                result.valid = false;
            // 【逐行批注】 执行当前声明或表达式：} else if (uniqueRatio < 0.70) {。
            } else if (uniqueRatio < 0.70) {
                // 【逐行批注】 计算右侧结果并写入 repetitionPenalty，更新当前运行状态。
                repetitionPenalty = 1;
                // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("重复内容偏高，唯一文本占比不足 70%。");。
                result.reasons.push_back("重复内容偏高，唯一文本占比不足 70%。");
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 执行当前声明或表达式：} else {。
        } else {
            // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("缺少 roadmap。\n");。
            result.reasons.push_back("缺少 roadmap。\n");
            // 【逐行批注】 计算右侧结果并写入 result.valid，更新当前运行状态。
            result.valid = false;
        // 【逐行批注】 结束当前代码块。
        }

        // 【逐行批注】 计算右侧结果并写入 auto allTexts，更新当前运行状态。
        const auto allTexts = collectTexts(plan);
        // 【逐行批注】 计算右侧结果并写入 std::string joinedAll，更新当前运行状态。
        const std::string joinedAll = [&]() {
            // 【逐行批注】 执行当前声明或表达式：std::ostringstream out;。
            std::ostringstream out;
            // 【逐行批注】 计算右侧结果并写入 bool first，更新当前运行状态。
            bool first = true;
            // 【逐行批注】 按循环规则重复处理：for (const auto& text : allTexts) {。
            for (const auto& text : allTexts) {
                // 【逐行批注】 判断条件 (!first) out << ' ';，成立时执行对应分支。
                if (!first) out << ' ';
                // 【逐行批注】 执行当前声明或表达式：out << text;。
                out << text;
                // 【逐行批注】 计算右侧结果并写入 first，更新当前运行状态。
                first = false;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return lower(out.str());。
            return lower(out.str());
        // 【逐行批注】 执行当前声明或表达式：}();。
        }();

        // 【逐行批注】 按循环规则重复处理：for (const auto& text : allTexts) {。
        for (const auto& text : allTexts) {
            // 【逐行批注】 计算右侧结果并写入 std::string lowered，更新当前运行状态。
            const std::string lowered = lower(trim(text));
            // 【逐行批注】 判断条件 (isGenericSentence(lowered)) genericTexts.insert(lowered);，成立时执行对应分支。
            if (isGenericSentence(lowered)) genericTexts.insert(lowered);
        // 【逐行批注】 结束当前代码块。
        }

        // 【逐行批注】 判断条件 (!grams.empty()) {，成立时执行对应分支。
        if (!grams.empty()) {
            // 【逐行批注】 计算右侧结果并写入 int hits，更新当前运行状态。
            const int hits = countOverlap(joinedAll + " " + mergedTexts, grams);
            // 【逐行批注】 计算右侧结果并写入 int missing，更新当前运行状态。
            const int missing = static_cast<int>(grams.size()) - hits;
            // 整体判定：目标关键词缺口允许少量遗漏（AI 生成的后段内容未必重复目标词），最多扣 2 档
            // 【逐行批注】 判断条件 (missing > 0) {，成立时执行对应分支。
            if (missing > 0) {
                // 【逐行批注】 计算右侧结果并写入 keywordMiss +，更新当前运行状态。
                keywordMiss += std::min(missing, 2);
                // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("课程内容与目标关键词相关性不足。");。
                result.reasons.push_back("课程内容与目标关键词相关性不足。");
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }

        // 【逐行批注】 执行当前声明或表达式：static const std::vector<std::string> actionWords = {"完成", "制作", "编写", "实现", "练习", "…。
        static const std::vector<std::string> actionWords = {"完成", "制作", "编写", "实现", "练习", "提交", "发布"};
        // 【逐行批注】 执行当前声明或表达式：static const std::vector<std::string> outputWords = {"作品", "报告", "项目", "代码", "作品集"};。
        static const std::vector<std::string> outputWords = {"作品", "报告", "项目", "代码", "作品集"};
        // 【逐行批注】 判断条件 (!containsAny(joinedAll, actionWords)) {，成立时执行对应分支。
        if (!containsAny(joinedAll, actionWords)) {
            // 【逐行批注】 计算右侧结果并写入 actionPenalty +，更新当前运行状态。
            actionPenalty += 1;
            // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("缺少明显动作词。\n");。
            result.reasons.push_back("缺少明显动作词。\n");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (!containsAny(joinedAll, outputWords)) {，成立时执行对应分支。
        if (!containsAny(joinedAll, outputWords)) {
            // 【逐行批注】 计算右侧结果并写入 actionPenalty +，更新当前运行状态。
            actionPenalty += 1;
            // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("缺少明确产出词。\n");。
            result.reasons.push_back("缺少明确产出词。\n");
        // 【逐行批注】 结束当前代码块。
        }

        // 【逐行批注】 计算右侧结果并写入 int genericHits，更新当前运行状态。
        const int genericHits = static_cast<int>(genericTexts.size());
        // 【逐行批注】 判断条件 (genericHits > 0) {，成立时执行对应分支。
        if (genericHits > 0) {
            // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("存在泛化表述，内容不够具体。");。
            result.reasons.push_back("存在泛化表述，内容不够具体。");
        // 【逐行批注】 结束当前代码块。
        }

        // 【逐行批注】 计算右侧结果并写入 int score，更新当前运行状态。
        int score = 100 - genericHits * 25 - keywordMiss * 15 - repetitionPenalty * 10 - actionPenalty * 15;
        // 【逐行批注】 判断条件 (score < 0) score = 0;，成立时执行对应分支。
        if (score < 0) score = 0;

        // 【逐行批注】 计算右侧结果并写入 result.score，更新当前运行状态。
        result.score = score;
        // 【逐行批注】 计算右侧结果并写入 bool fatal，更新当前运行状态。
        const bool fatal = result.reasons.end() != std::find_if(result.reasons.begin(), result.reasons.end(), [](const std::string& reason) {
            // 【逐行批注】 结束当前函数并返回结果：return reason.find("少于 2") != std::string::npos || reason.find("不足 55…。
            return reason.find("少于 2") != std::string::npos || reason.find("不足 55%") != std::string::npos ||
                   // 【逐行批注】 执行当前声明或表达式：reason.find("阶段数应为") != std::string::npos || reason.find("不是对象结构") != std::string::n…。
                   reason.find("阶段数应为") != std::string::npos || reason.find("不是对象结构") != std::string::npos ||
                   // 【逐行批注】 执行当前声明或表达式：reason.find("缺少 roadmap") != std::string::npos;。
                   reason.find("缺少 roadmap") != std::string::npos;
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 计算右侧结果并写入 result.valid，更新当前运行状态。
        result.valid = !fatal && score >= 55;
        // 【逐行批注】 判断条件 (!result.valid && result.reasons.empty()) result.reasons.push_ba…，成立时执行对应分支。
        if (!result.valid && result.reasons.empty()) result.reasons.push_back("课程内容未通过质量门禁。");
        // 【逐行批注】 结束当前函数并返回结果：return result;。
        return result;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 计算右侧结果并写入 result.valid，更新当前运行状态。
        result.valid = false;
        // 【逐行批注】 计算右侧结果并写入 result.score，更新当前运行状态。
        result.score = 0;
        // 【逐行批注】 执行当前声明或表达式：result.reasons.push_back("质量门禁执行失败。");。
        result.reasons.push_back("质量门禁执行失败。");
        // 【逐行批注】 结束当前函数并返回结果：return result;。
        return result;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

#include "quality_gate.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace gangyi {
namespace {

using json = nlohmann::json;

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n\f\v");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n\f\v");
    return value.substr(first, last - first + 1);
}

std::string lower(std::string value) {
    for (char& ch : value) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return value;
}

int chineseChars(const std::string& value) {
    int count = 0;
    for (size_t i = 0; i < value.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(value[i]);
        if (c >= 0xE4) ++count;
    }
    return count;
}

std::vector<std::string> goalKeywords(const std::string& goal) {
    // 按 UTF-8 字符切分（中文 3 字节/字符、英文 1 字节/字符），避免把汉字切半
    std::vector<std::string> chars;
    const std::string text = lower(trim(goal));
    for (size_t i = 0; i < text.size();) {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        size_t len = 1;
        if (c >= 0xF0) len = 4;
        else if (c >= 0xE0) len = 3;
        else if (c >= 0xC0) len = 2;
        chars.push_back(text.substr(i, len));
        i += len;
    }

    std::vector<std::string> grams;
    // 中文/混排：前 2-4 个字符的 n-gram
    for (size_t n = 2; n <= 4 && chars.size() >= n; ++n) {
        std::string gram;
        for (size_t k = 0; k < n; ++k) gram += chars[k];
        grams.push_back(gram);
    }
    // ASCII 整词（如 python / excel）也加入
    std::string asciiWord;
    for (const auto& ch : chars) {
        const unsigned char b = static_cast<unsigned char>(ch[0]);
        if (b < 0x80 && std::isalnum(b)) {
            asciiWord += ch;
        } else if (!asciiWord.empty()) {
            if (asciiWord.size() >= 3) grams.push_back(asciiWord);
            asciiWord.clear();
        }
    }
    if (!asciiWord.empty() && asciiWord.size() >= 3) grams.push_back(asciiWord);

    std::sort(grams.begin(), grams.end());
    grams.erase(std::unique(grams.begin(), grams.end()), grams.end());
    if (grams.size() > 6) grams.resize(6);
    return grams;
}

bool containsAny(const std::string& value, const std::vector<std::string>& needles) {
    for (const auto& needle : needles) {
        if (!needle.empty() && value.find(needle) != std::string::npos) return true;
    }
    return false;
}

bool isGenericSentence(const std::string& text) {
    static const std::vector<std::string> patterns = {
        "关键抓手", "不要只背名词", "理解核心目标", "掌握基本概念", "提升综合能力", "深入学习相关知识",
        "多加练习", "参考相关资料", "只给链接", "只列大纲", "课程导入", "至少完成一次解释和练习",
        "理解阶段目标", "用练习把知识变成能力", "复盘并形成阶段产出", "建立学习节奏", "完成一次输出"
    };
    return containsAny(text, patterns);
}

int countOverlap(const std::string& text, const std::vector<std::string>& grams) {
    int count = 0;
    for (const auto& gram : grams) {
        if (!gram.empty() && text.find(gram) != std::string::npos) ++count;
    }
    return count;
}

std::string joinStrings(const json& array, const char* sep = "；") {
    if (!array.is_array()) return {};
    std::ostringstream out;
    bool first = true;
    for (const auto& item : array) {
        if (!item.is_string()) continue;
        if (!first) out << sep;
        out << item.get<std::string>();
        first = false;
    }
    return out.str();
}

std::vector<std::string> collectTexts(const json& plan) {
    std::vector<std::string> texts;
    if (!plan.is_object()) return texts;
    if (plan.contains("summary") && plan["summary"].is_string()) texts.push_back(plan["summary"].get<std::string>());
    if (plan.contains("courseIntro") && plan["courseIntro"].is_string()) texts.push_back(plan["courseIntro"].get<std::string>());
    if (plan.contains("overview") && plan["overview"].is_string()) texts.push_back(plan["overview"].get<std::string>());
    if (plan.contains("roadmap") && plan["roadmap"].is_array()) {
        for (const auto& phase : plan["roadmap"]) {
            if (!phase.is_object()) continue;
            if (phase.contains("description") && phase["description"].is_string()) texts.push_back(phase["description"].get<std::string>());
            if (phase.contains("goal") && phase["goal"].is_string()) texts.push_back(phase["goal"].get<std::string>());
            if (phase.contains("output") && phase["output"].is_string()) texts.push_back(phase["output"].get<std::string>());
            if (phase.contains("steps") && phase["steps"].is_array()) {
                for (const auto& step : phase["steps"]) {
                    if (!step.is_object()) continue;
                    if (step.contains("title") && step["title"].is_string()) texts.push_back(step["title"].get<std::string>());
                    if (step.contains("explanation") && step["explanation"].is_string()) texts.push_back(step["explanation"].get<std::string>());
                    if (step.contains("action") && step["action"].is_string()) texts.push_back(step["action"].get<std::string>());
                    if (step.contains("check") && step["check"].is_string()) texts.push_back(step["check"].get<std::string>());
                }
            }
        }
    }
    if (plan.contains("courseStructure") && plan["courseStructure"].is_array()) {
        for (const auto& stage : plan["courseStructure"]) {
            if (!stage.is_object()) continue;
            if (stage.contains("stage") && stage["stage"].is_string()) texts.push_back(stage["stage"].get<std::string>());
            if (stage.contains("topics") && stage["topics"].is_array()) {
                for (const auto& topic : stage["topics"]) if (topic.is_string()) texts.push_back(topic.get<std::string>());
            }
        }
    }
    if (plan.contains("slides") && plan["slides"].is_array()) {
        for (const auto& slide : plan["slides"]) {
            if (!slide.is_object()) continue;
            if (slide.contains("title") && slide["title"].is_string()) texts.push_back(slide["title"].get<std::string>());
            if (slide.contains("content") && slide["content"].is_string()) texts.push_back(slide["content"].get<std::string>());
            if (slide.contains("bullets") && slide["bullets"].is_array()) {
                for (const auto& bullet : slide["bullets"]) if (bullet.is_string()) texts.push_back(bullet.get<std::string>());
            }
        }
    }
    return texts;
}

}  // namespace

QualityResult validateCourseContent(const json& plan, const std::string& goal, const std::string& mode, const std::string& courseTitle) {
    QualityResult result;
    try {
        if (!plan.is_object()) {
            result.reasons.push_back("课程内容不是对象结构。");
            result.score = 0;
            return result;
        }

        const std::string goalText = lower(trim(goal));
        const std::vector<std::string> grams = goalKeywords(goal);
        const std::string mergedTexts = lower(trim(courseTitle + " " + goal));
        std::set<std::string> genericTexts;
        int keywordMiss = 0;
        int repetitionPenalty = 0;
        int actionPenalty = 0;

        if (plan.contains("roadmap") && plan["roadmap"].is_array()) {
            const int phaseCount = static_cast<int>(plan["roadmap"].size());
            if (mode == "lite" && (phaseCount < 3 || phaseCount > 5)) {
                result.reasons.push_back("lite 模式阶段数应为 3-5。");
                result.valid = false;
            }
            if (mode != "lite" && (phaseCount < 4 || phaseCount > 6)) {
                result.reasons.push_back("deep 模式阶段数应为 4-6。");
                result.valid = false;
            }
            std::set<std::string> uniqueTexts;
            size_t textCount = 0;
            size_t uniqueCount = 0;
            for (const auto& phase : plan["roadmap"]) {
                if (!phase.is_object()) continue;
                const std::string phaseName = lower(trim(phase.value("name", "")));
                const std::string phaseDesc = lower(trim(phase.value("description", "")));
                if (isGenericSentence(phaseName)) genericTexts.insert(phaseName);
                if (isGenericSentence(phaseDesc)) genericTexts.insert(phaseDesc);
                uniqueTexts.insert(phaseName + "|" + phaseDesc);
                if (phase.contains("topics") && phase["topics"].is_array() && phase["topics"].size() < 2) {
                    result.reasons.push_back("某阶段 topics 少于 2，内容过薄。");
                    result.valid = false;
                }
                if (phase.contains("steps") && phase["steps"].is_array()) {
                    for (const auto& step : phase["steps"]) {
                        if (!step.is_object()) continue;
                        const auto strOf = [](const json& obj, const char* key) {
                            return obj.contains(key) && obj[key].is_string() ? obj[key].get<std::string>() : std::string{};
                        };
                        const std::string stepText = lower(trim(
                            strOf(step, "title") + " " + strOf(step, "explanation") + " " +
                            strOf(step, "action") + " " + strOf(step, "check")));
                        const std::string joined = stepText;
                        ++textCount;
                        if (uniqueTexts.insert(joined).second) ++uniqueCount;
                        if (isGenericSentence(joined)) genericTexts.insert(joined);
                    }
                }
            }
            const double uniqueRatio = textCount == 0 ? 1.0 : static_cast<double>(uniqueCount) / static_cast<double>(textCount);
            if (uniqueRatio < 0.55) {
                result.reasons.push_back("重复内容过高，唯一文本占比不足 55%。");
                result.valid = false;
            } else if (uniqueRatio < 0.70) {
                repetitionPenalty = 1;
                result.reasons.push_back("重复内容偏高，唯一文本占比不足 70%。");
            }
        } else {
            result.reasons.push_back("缺少 roadmap。\n");
            result.valid = false;
        }

        const auto allTexts = collectTexts(plan);
        const std::string joinedAll = [&]() {
            std::ostringstream out;
            bool first = true;
            for (const auto& text : allTexts) {
                if (!first) out << ' ';
                out << text;
                first = false;
            }
            return lower(out.str());
        }();

        for (const auto& text : allTexts) {
            const std::string lowered = lower(trim(text));
            if (isGenericSentence(lowered)) genericTexts.insert(lowered);
        }

        if (!grams.empty()) {
            const int hits = countOverlap(joinedAll + " " + mergedTexts, grams);
            const int missing = static_cast<int>(grams.size()) - hits;
            // 整体判定：目标关键词缺口允许少量遗漏（AI 生成的后段内容未必重复目标词），最多扣 2 档
            if (missing > 0) {
                keywordMiss += std::min(missing, 2);
                result.reasons.push_back("课程内容与目标关键词相关性不足。");
            }
        }

        static const std::vector<std::string> actionWords = {"完成", "制作", "编写", "实现", "练习", "提交", "发布"};
        static const std::vector<std::string> outputWords = {"作品", "报告", "项目", "代码", "作品集"};
        if (!containsAny(joinedAll, actionWords)) {
            actionPenalty += 1;
            result.reasons.push_back("缺少明显动作词。\n");
        }
        if (!containsAny(joinedAll, outputWords)) {
            actionPenalty += 1;
            result.reasons.push_back("缺少明确产出词。\n");
        }

        const int genericHits = static_cast<int>(genericTexts.size());
        if (genericHits > 0) {
            result.reasons.push_back("存在泛化表述，内容不够具体。");
        }

        int score = 100 - genericHits * 25 - keywordMiss * 15 - repetitionPenalty * 10 - actionPenalty * 15;
        if (score < 0) score = 0;

        result.score = score;
        const bool fatal = result.reasons.end() != std::find_if(result.reasons.begin(), result.reasons.end(), [](const std::string& reason) {
            return reason.find("少于 2") != std::string::npos || reason.find("不足 55%") != std::string::npos ||
                   reason.find("阶段数应为") != std::string::npos || reason.find("不是对象结构") != std::string::npos ||
                   reason.find("缺少 roadmap") != std::string::npos;
        });
        result.valid = !fatal && score >= 55;
        if (!result.valid && result.reasons.empty()) result.reasons.push_back("课程内容未通过质量门禁。");
        return result;
    } catch (...) {
        result.valid = false;
        result.score = 0;
        result.reasons.push_back("质量门禁执行失败。");
        return result;
    }
}

}  // namespace gangyi

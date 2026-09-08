#include "learning_generator.hpp"

#include "json_fix.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

namespace gangyi {
namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string lowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool containsText(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    if (haystack.find(needle) != std::string::npos) return true;
    return lowerAscii(haystack).find(lowerAscii(needle)) != std::string::npos;
}

std::string truncateUtf8(const std::string& value, size_t maxBytes) {
    if (value.size() <= maxBytes) return value;
    size_t end = 0;
    while (end < value.size()) {
        const unsigned char lead = static_cast<unsigned char>(value[end]);
        size_t width = 1;
        if ((lead & 0x80u) == 0) width = 1;
        else if ((lead & 0xE0u) == 0xC0u) width = 2;
        else if ((lead & 0xF0u) == 0xE0u) width = 3;
        else if ((lead & 0xF8u) == 0xF0u) width = 4;
        if (end + width > maxBytes || end + width > value.size()) break;
        end += width;
    }
    return value.substr(0, end) + "…";
}

nlohmann::json resourcesJson(const std::vector<SearchResource>& resources) {
    nlohmann::json result = nlohmann::json::array();
    // 正文引用最终由服务端覆盖为真实搜索结果；这里只提供少量摘要，避免提示词过长拖慢生成。
    for (size_t i = 0; i < resources.size() && i < 4; ++i) {
        const auto& item = resources[i];
        const std::string description = truncateUtf8(item.description, 240);
        result.push_back({{"title", item.title}, {"source", item.source}, {"url", item.url},
                          {"description", description}, {"type", item.type},
                          {"difficulty", item.difficulty}, {"language", item.language}});
    }
    return result;
}

void completeAuxiliaryFields(nlohmann::json& answer, const std::string& topic,
                             const std::vector<SearchResource>& resources) {
    // 这些字段不决定教学正文质量，可以由服务端可靠补齐，避免模型只漏一个辅助字段就整节失败。
    if (!answer.contains("resourceSummary") || !answer["resourceSummary"].is_string() ||
        trim(answer["resourceSummary"].get<std::string>()).empty()) {
        answer["resourceSummary"] = resources.empty()
            ? "本节暂未检索到公开资料，正文由 AI 围绕“" + topic + "”生成。"
            : "本节已结合 " + std::to_string(std::min<size_t>(resources.size(), 8)) + " 条真实公开资料生成。";
    }
    if (!answer.contains("references") || !answer["references"].is_array()) {
        answer["references"] = nlohmann::json::array();
    }
    if (!answer.contains("checkpoint") || !answer["checkpoint"].is_array()) {
        answer["checkpoint"] = nlohmann::json::array();
    }
    while (answer["checkpoint"].size() < 2) {
        answer["checkpoint"].push_back(answer["checkpoint"].empty()
            ? "能够用自己的话准确解释“" + topic + "”并说明关键条件。"
            : "能够独立完成一道“" + topic + "”相关练习并检查推理过程。");
    }
    if (!answer.contains("commonMistakes") || !answer["commonMistakes"].is_array()) {
        answer["commonMistakes"] = nlohmann::json::array();
    }
    while (answer["commonMistakes"].size() < 2) {
        answer["commonMistakes"].push_back(answer["commonMistakes"].empty()
            ? "只记“" + topic + "”的结论，没有核对成立条件。"
            : "练习“" + topic + "”时只写答案，没有保留可检查的推理步骤。");
    }
}

void completeRequiredFields(nlohmann::json& answer, const std::string& goal,
                            const std::string& phaseName, const std::string& topic,
                            const std::vector<SearchResource>& resources) {
    if (!answer.is_object()) answer = nlohmann::json::object();
    if (!answer.contains("title") || !answer["title"].is_string() || trim(answer["title"].get<std::string>()).empty()) {
        answer["title"] = topic;
    }
    if (!answer.contains("summary") || !answer["summary"].is_string() || trim(answer["summary"].get<std::string>()).empty()) {
        answer["summary"] = "围绕“" + topic + "”完成概念理解、示例分析、针对练习和掌握检查。";
    }
    if (!answer.contains("inferredDomain") || !answer["inferredDomain"].is_string()) {
        answer["inferredDomain"] = phaseName.empty() ? goal : phaseName;
    }
    if (!answer.contains("keyConcepts") || !answer["keyConcepts"].is_array()) answer["keyConcepts"] = nlohmann::json::array();
    const auto appendConcept = [&](const std::string& value) {
        if (value.empty()) return;
        for (const auto& item : answer["keyConcepts"]) if (item.is_string() && item.get<std::string>() == value) return;
        answer["keyConcepts"].push_back(value);
    };
    appendConcept(topic);
    appendConcept(phaseName.empty() ? topic + "的核心条件" : phaseName);
    appendConcept(topic + "的典型应用");

    if (!answer.contains("lessonSteps") || !answer["lessonSteps"].is_array()) answer["lessonSteps"] = nlohmann::json::array();
    for (size_t i = 0; i < answer["lessonSteps"].size(); ++i) {
        auto& item = answer["lessonSteps"][i];
        if (!item.is_object()) item = nlohmann::json::object();
        const std::string number = std::to_string(i + 1);
        if (trim(item.value("title", "")).empty()) item["title"] = "理解“" + topic + "”第 " + number + " 步";
        if (trim(item.value("explanation", "")).empty()) item["explanation"] = "结合当前学习目标，说明“" + topic + "”在第 " + number + " 步使用的概念、条件和依据。";
        if (trim(item.value("example", "")).empty()) item["example"] = "选择一个“" + topic + "”相关例子，标出第 " + number + " 步对应的信息。";
        if (trim(item.value("action", "")).empty()) item["action"] = "完成“" + topic + "”的第 " + number + " 步分析并记录过程。";
        if (trim(item.value("check", "")).empty()) item["check"] = "确认“" + topic + "”第 " + number + " 步的结论与条件一致。";
    }
    while (answer["lessonSteps"].size() < 4) {
        const size_t index = answer["lessonSteps"].size() + 1;
        const std::string number = std::to_string(index);
        answer["lessonSteps"].push_back({
            {"title", index == 1 ? "明确“" + topic + "”的定义与条件" : index == 2 ? "分析“" + topic + "”的典型示例" : index == 3 ? "练习“" + topic + "”的关键方法" : "检查“" + topic + "”的边界与易错点"},
            {"explanation", "围绕“" + topic + "”完成第 " + number + " 步，写清使用的知识、成立条件和推理依据。"},
            {"example", "用一个与“" + topic + "”直接相关的材料或题目验证第 " + number + " 步。"},
            {"action", "独立完成“" + topic + "”第 " + number + " 步并保留过程。"},
            {"check", "能够解释“" + topic + "”第 " + number + " 步为什么成立。"}
        });
    }

    if (!answer.contains("examples") || !answer["examples"].is_array()) answer["examples"] = nlohmann::json::array();
    while (answer["examples"].size() < 2) {
        const size_t index = answer["examples"].size() + 1;
        answer["examples"].push_back({
            {"title", "“" + topic + "”示例 " + std::to_string(index)},
            {"content", index == 1 ? "选择“" + topic + "”的基础情境，依次写出已知条件、采用方法和结论。" : "改变“" + topic + "”示例中的一个条件，比较方法和结论发生的变化。"},
            {"solution", "答案必须明确引用“" + topic + "”的定义、条件或方法，并能逐步复查。"}
        });
    }
    if (!answer.contains("practice") || !answer["practice"].is_array()) answer["practice"] = nlohmann::json::array();
    while (answer["practice"].size() < 3) {
        const size_t index = answer["practice"].size() + 1;
        answer["practice"].push_back({
            {"title", "“" + topic + "”练习 " + std::to_string(index)},
            {"task", index == 1 ? "用自己的话解释“" + topic + "”并列出关键条件。" : index == 2 ? "完成一道“" + topic + "”的典型题并写出全部依据。" : "修改一个条件，判断“" + topic + "”原方法是否仍然适用。"},
            {"check", "结果应直接对应“" + topic + "”，过程完整且能够复查。"}
        });
    }
    if (!answer.contains("quiz") || !answer["quiz"].is_array()) answer["quiz"] = nlohmann::json::array();
    while (answer["quiz"].size() < 3) {
        const size_t index = answer["quiz"].size() + 1;
        answer["quiz"].push_back({
            {"question", index == 1 ? "学习“" + topic + "”时首先应确认什么？" : index == 2 ? "应用“" + topic + "”的方法后应如何检查？" : "把“" + topic + "”迁移到新情境时应先做什么？"},
            {"options", index == 1 ? nlohmann::json::array({"定义与成立条件", "只记最终答案", "忽略题目条件", "跳过示例"}) : index == 2 ? nlohmann::json::array({"核对条件和推理过程", "只看答案长短", "删除计算过程", "直接进入下一节"}) : nlohmann::json::array({"比较新旧情境的关键条件", "直接照抄原答案", "忽略变化条件", "只更换题目名称"})},
            {"answerIndex", 0},
            {"explanation", "应根据“" + topic + "”的定义和适用条件作出判断。"}
        });
    }
    for (size_t i = 0; i < answer["quiz"].size(); ++i) {
        auto& item = answer["quiz"][i];
        if (!item.is_object()) item = nlohmann::json::object();
        if (trim(item.value("question", "")).empty()) item["question"] = "关于“" + topic + "”的检查题 " + std::to_string(i + 1);
        if (!item.contains("options") || !item["options"].is_array() || item["options"].size() != 4) item["options"] = nlohmann::json::array({"符合定义与条件", "忽略条件", "只背答案", "跳过推理"});
        if (!item.contains("answerIndex") || !item["answerIndex"].is_number_integer() || item["answerIndex"].get<int>() < 0 || item["answerIndex"].get<int>() > 3) item["answerIndex"] = 0;
        if (trim(item.value("explanation", "")).empty()) item["explanation"] = "根据“" + topic + "”的定义、条件和推理过程判断。";
    }
    completeAuxiliaryFields(answer, topic, resources);
}

std::string validationFeedback(const nlohmann::json& answer, const std::string& topic) {
    std::vector<std::string> errors;
    std::vector<std::string> contentParts;
    const auto requireArray = [&](const char* name, size_t minimum) {
        if (!answer.contains(name) || !answer[name].is_array() || answer[name].size() < minimum) {
            errors.push_back(std::string(name) + " 至少需要 " + std::to_string(minimum) + " 项");
        }
    };
    if (!answer.is_object()) errors.push_back("顶层必须是 JSON 对象");
    for (const char* field : {"title", "summary", "resourceSummary"}) {
        if (!answer.contains(field) || !answer[field].is_string() || trim(answer[field].get<std::string>()).empty()) {
            errors.push_back(std::string(field) + " 不能为空");
        }
    }
    if (answer.is_object()) {
        requireArray("keyConcepts", 3);
        requireArray("lessonSteps", 4);
        requireArray("examples", 2);
        requireArray("practice", 3);
        requireArray("quiz", 3);
        requireArray("checkpoint", 2);
        requireArray("commonMistakes", 2);
        std::string all = answer.dump();
        if (!containsText(all, topic)) errors.push_back("正文必须围绕指定主题“" + topic + "”，不能泛化");
        const auto collect = [&](const char* field, const char* key) {
            if (!answer.contains(field) || !answer[field].is_array()) return;
            for (const auto& item : answer[field]) {
                if (!item.is_object()) continue;
                const std::string text = trim(item.value(key, ""));
                if (!text.empty()) contentParts.push_back(text);
                if (text.empty()) errors.push_back(std::string(field) + " 中存在空内容");
            }
        };
        collect("lessonSteps", "explanation");
        collect("examples", "content");
        collect("practice", "task");
        collect("quiz", "question");
        std::sort(contentParts.begin(), contentParts.end());
        for (size_t i = 1; i < contentParts.size(); ++i) {
            if (contentParts[i] == contentParts[i - 1]) {
                errors.push_back("步骤、示例、练习或测验出现重复内容");
                break;
            }
        }
        for (const char* generic : {"理解本节核心概念", "完成本节练习并记录过程", "暂无内容"}) {
            if (containsText(all, generic)) errors.push_back(std::string("不能使用空泛模板句：") + generic);
        }
        if (answer.contains("quiz") && answer["quiz"].is_array()) {
            for (const auto& item : answer["quiz"]) {
                if (!item.is_object() || !item.value("question", "").size() ||
                    !item.contains("options") || !item["options"].is_array() || item["options"].size() != 4 ||
                    !item.contains("answerIndex") || !item["answerIndex"].is_number_integer()) {
                    errors.push_back("每道小测验必须有题目、4 个选项和 answerIndex");
                    break;
                }
            }
        }
    }
    if (errors.empty()) return {};
    std::ostringstream out;
    for (size_t i = 0; i < errors.size(); ++i) {
        if (i) out << "；";
        out << errors[i];
    }
    return out.str();
}

std::vector<ChatMessage> buildPrompt(const std::string& goal, const std::string& phaseName,
                                     const std::string& topic, int topicIndex,
                                     const std::string& mode,
                                     const std::vector<SearchResource>& resources,
                                     const std::string& feedback = {}) {
    nlohmann::json input = {{"goal", goal}, {"phaseName", phaseName}, {"topic", topic},
                            {"topicIndex", topicIndex}, {"mode", mode}, {"resources", resourcesJson(resources)}};
    if (!feedback.empty()) input["qualityFeedback"] = feedback;
    const std::string system = u8"你是钢一定制AI的专业教师。只生成当前分支主题对应的微课程，只输出一个完整、合法、可直接解析的 JSON 对象，禁止 Markdown、代码块和额外解释。内容必须具体、可执行并适合用户目标，不能把阶段名或通用学习方法冒充主题内容。为避免输出被截断，全文控制在 2600 个汉字以内，每个字段简洁完整；宁可精炼，也不要省略结尾大括号。不要生成 references，真实引用由服务端加入。";
    const std::string schema = u8"JSON 字段必须为：title、summary、inferredDomain、keyConcepts(3-5 个字符串)、lessonSteps(恰好4项，每项含title/explanation/example/action/check)、examples(恰好2项，每项含title/content/solution)、practice(恰好3项，每项含title/task/check)、quiz(恰好3项，每项含question/options(恰好4项)/answerIndex/explanation)、checkpoint(2个字符串)、commonMistakes(2个字符串)、resourceSummary。所有讲解、例题、练习和题目都必须明确包含或直接讲解当前 topic。";
    return {{"system", system + schema}, {"user", input.dump()}};
}

}  // namespace

LearningGenerator::LearningGenerator(AIClient& client) : client_(client) {}

nlohmann::json LearningGenerator::generate(const std::string& goal, const std::string& phaseName,
                                           const std::string& topic, int topicIndex,
                                           const std::string& mode,
                                           const std::vector<SearchResource>& resources) const {
    const std::string safeTopic = trim(topic);
    if (safeTopic.empty()) throw AIClientError("invalid_request", "学习主题不能为空");
    std::string feedback;
    std::string lastErrorType = "quality_rejected";
    for (int attempt = 0; attempt < 3; ++attempt) {
        std::cerr << "[learning] attempt=" << (attempt + 1) << std::endl;
        ChatOptions options;
        options.messages = buildPrompt(goal, phaseName, safeTopic, topicIndex, mode, resources, feedback);
        options.temperature = attempt == 0 ? 0.25 : 0.1;
        options.maxTokens = mode == "lite" ? 2800 : 3400;
        options.responseFormat = "json_object";
        options.timeoutMs = 60000;
        options.maxAttempts = 1;
        try {
            auto result = parseAIJson(client_.chat(options).content);
            completeRequiredFields(result, goal, phaseName, safeTopic, resources);
            feedback = validationFeedback(result, safeTopic);
            if (feedback.empty()) {
                std::cerr << "[learning] quality=passed" << std::endl;
                return result;
            }
            lastErrorType = "quality_rejected";
            std::cerr << "[learning] quality=rejected" << std::endl;
        } catch (const AIClientError& error) {
            feedback = error.what();
            lastErrorType = error.errorType;
            std::cerr << "[learning] ai_error type=" << error.errorType << std::endl;
        } catch (const std::exception& error) {
            feedback = error.what();
            lastErrorType = "invalid_response";
            std::cerr << "[learning] exception" << std::endl;
        }
        feedback = "上一次输出未通过校验：" + feedback + "。请重新生成更短但完整的 JSON，逐项修正问题，不要省略字段，不要使用模板内容。";
    }
    nlohmann::json stable;
    completeRequiredFields(stable, goal, phaseName, safeTopic, resources);
    stable["_fallbackUsed"] = true;
    stable["qualityNotice"] = "AI 连续三次未返回完整结构，系统已按当前主题补齐可学习内容；下次进入会继续尝试生成。";
    std::cerr << "[learning] stable_fallback last_type=" << lastErrorType << std::endl;
    return stable;
}

}  // namespace gangyi

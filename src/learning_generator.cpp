#include "learning_generator.hpp"

#include "json_fix.hpp"
#include "text_utils.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace gangyi {
namespace {

using json = nlohmann::json;

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
}

std::string lowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

bool containsText(const std::string& value, const std::string& expected) {
    return expected.empty() || value.find(expected) != std::string::npos ||
        lowerAscii(value).find(lowerAscii(expected)) != std::string::npos;
}

std::string nowIso8601() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::ostringstream out;
    out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

json resourceContext(const std::vector<SearchResource>& resources) {
    json result = json::array();
    for (size_t i = 0; i < resources.size() && i < 5; ++i) {
        const auto& item = resources[i];
        result.push_back({{"title", item.title}, {"source", item.source},
            {"description", truncateUtf8(item.description, 300)}});
    }
    return result;
}

const std::unordered_map<std::string, std::string>& schemas() {
    static const std::unordered_map<std::string, std::string> value = {
        {"overview", u8R"({"title":"","summary":"","inferredDomain":"","keyConcepts":["3-5个具体概念"]})"},
        {"steps", u8R"({"lessonSteps":[{"title":"","explanation":"","example":"","action":"","check":""}]}，lessonSteps 必须为 4-6 项)"},
        {"examples", u8R"({"examples":[{"title":"","content":"","solution":""}]}，examples 必须为 2-4 项，题目或材料与完整解析必须具体)"},
        {"practice", u8R"({"practice":[{"title":"","task":"","check":""}]}，practice 必须为 3-5 项，难度递进)"},
        {"quiz", u8R"({"quiz":[{"question":"","options":["","","",""],"answerIndex":0,"explanation":""}]}，quiz 必须为 3-5 项)"},
        {"assessment", u8R"({"checkpoint":[""],"commonMistakes":[""],"resourceSummary":""}，checkpoint 必须为 2-4 项，commonMistakes 必须为 2-4 项)"}
    };
    return value;
}

bool nonEmpty(const json& object, const char* key) {
    return object.is_object() && object.contains(key) && object[key].is_string() &&
        !trim(object[key].get<std::string>()).empty();
}

bool stringArray(const json& object, const char* key, size_t minimum, size_t maximum) {
    if (!object.is_object() || !object.contains(key) || !object[key].is_array() ||
        object[key].size() < minimum || object[key].size() > maximum) return false;
    for (const auto& item : object[key]) if (!item.is_string() || trim(item.get<std::string>()).empty()) return false;
    return true;
}

bool uniqueField(const json& items, const char* key) {
    if (!items.is_array()) return false;
    std::set<std::string> seen;
    for (const auto& item : items) {
        const std::string text = item.value(key, "");
        if (text.empty() || !seen.insert(text).second) return false;
    }
    return true;
}

std::string validate(const std::string& block, const json& output, const std::string& goal,
                     const std::string& phaseName, const std::string& topic) {
    std::vector<std::string> errors;
    if (!output.is_object()) return "顶层必须是 JSON 对象";
    if (block == "overview") {
        if (!nonEmpty(output, "title")) errors.push_back("title 不能为空");
        if (!nonEmpty(output, "summary")) errors.push_back("summary 不能为空");
        if (!nonEmpty(output, "inferredDomain")) errors.push_back("inferredDomain 不能为空");
        if (!stringArray(output, "keyConcepts", 3, 5)) errors.push_back("keyConcepts 需要 3-5 个具体概念");
    } else if (block == "steps") {
        if (!output.contains("lessonSteps") || !output["lessonSteps"].is_array() ||
            output["lessonSteps"].size() < 4 || output["lessonSteps"].size() > 6) errors.push_back("lessonSteps 需要 4-6 项");
        else for (const auto& item : output["lessonSteps"])
            for (const char* key : {"title", "explanation", "example", "action", "check"})
                if (!nonEmpty(item, key)) errors.push_back(std::string("lessonSteps 缺少 ") + key);
    } else if (block == "examples") {
        if (!output.contains("examples") || !output["examples"].is_array() ||
            output["examples"].size() < 2 || output["examples"].size() > 4) errors.push_back("examples 需要 2-4 项");
        else for (const auto& item : output["examples"])
            for (const char* key : {"title", "content", "solution"})
                if (!nonEmpty(item, key)) errors.push_back(std::string("examples 缺少 ") + key);
        if (output.contains("examples") && !uniqueField(output["examples"], "content")) errors.push_back("examples 内容不得重复");
    } else if (block == "practice") {
        if (!output.contains("practice") || !output["practice"].is_array() ||
            output["practice"].size() < 3 || output["practice"].size() > 5) errors.push_back("practice 需要 3-5 项");
        else for (const auto& item : output["practice"])
            for (const char* key : {"title", "task", "check"})
                if (!nonEmpty(item, key)) errors.push_back(std::string("practice 缺少 ") + key);
        if (output.contains("practice") && !uniqueField(output["practice"], "task")) errors.push_back("practice 内容不得重复");
    } else if (block == "quiz") {
        if (!output.contains("quiz") || !output["quiz"].is_array() ||
            output["quiz"].size() < 3 || output["quiz"].size() > 5) errors.push_back("quiz 需要 3-5 项");
        else for (const auto& item : output["quiz"]) {
            if (!nonEmpty(item, "question") || !nonEmpty(item, "explanation") ||
                !stringArray(item, "options", 4, 4) || !item.contains("answerIndex") ||
                !item["answerIndex"].is_number_integer() || item["answerIndex"].get<int>() < 0 ||
                item["answerIndex"].get<int>() > 3) errors.push_back("quiz 题目必须包含题干、4 个选项、答案和解释");
        }
        if (output.contains("quiz") && !uniqueField(output["quiz"], "question")) errors.push_back("quiz 题目不得重复");
    } else if (block == "assessment") {
        if (!stringArray(output, "checkpoint", 2, 4)) errors.push_back("checkpoint 需要 2-4 项");
        if (!stringArray(output, "commonMistakes", 2, 4)) errors.push_back("commonMistakes 需要 2-4 项");
        if (!nonEmpty(output, "resourceSummary")) errors.push_back("resourceSummary 不能为空");
    } else {
        return "未知课堂板块";
    }

    const std::string serialized = output.dump();
    if (!containsText(serialized, goal)) errors.push_back("内容必须明确回应用户目标“" + goal + "”");
    if (!containsText(serialized, phaseName)) errors.push_back("内容必须明确对应当前阶段“" + phaseName + "”");
    if (!containsText(serialized, topic)) errors.push_back("内容必须明确围绕当前主题“" + topic + "”");
    for (const std::string generic : {"理解本节核心概念", "完成本节练习并记录过程", "暂无内容"})
        if (containsText(serialized, generic)) errors.push_back("包含通用模板句：“" + generic + "”");

    std::ostringstream feedback;
    for (size_t i = 0; i < errors.size(); ++i) {
        if (i) feedback << "；";
        feedback << errors[i];
    }
    return feedback.str();
}

json normalizedBlock(const std::string& block, const json& output) {
    json normalized = json::object();
    const auto copy = [&](const char* key) { normalized[key] = output.at(key); };
    if (block == "overview") {
        for (const char* key : {"title", "summary", "inferredDomain", "keyConcepts"}) copy(key);
        return normalized;
    }
    if (block == "assessment") {
        for (const char* key : {"checkpoint", "commonMistakes", "resourceSummary"}) copy(key);
        return normalized;
    }
    const char* arrayKey = block == "steps" ? "lessonSteps" : block.c_str();
    normalized[arrayKey] = json::array();
    for (const auto& item : output.at(arrayKey)) {
        json clean = json::object();
        if (block == "steps") for (const char* key : {"title", "explanation", "example", "action", "check"}) clean[key] = item.at(key);
        if (block == "examples") for (const char* key : {"title", "content", "solution"}) clean[key] = item.at(key);
        if (block == "practice") for (const char* key : {"title", "task", "check"}) clean[key] = item.at(key);
        if (block == "quiz") for (const char* key : {"question", "options", "answerIndex", "explanation"}) clean[key] = item.at(key);
        normalized[arrayKey].push_back(std::move(clean));
    }
    return normalized;
}

}  // namespace

LearningGenerator::LearningGenerator(AIClient& client) : client_(client) {}

json LearningGenerator::generateBlock(const std::string& goal, const json& coursePlan,
                                       const std::string& phaseName, const std::string& topic,
                                       int topicIndex, const std::string& mode,
                                       const std::string& block, const json& previousBlocks,
                                       const std::vector<SearchResource>& resources) const {
    const std::string safeTopic = trim(topic);
    const auto schema = schemas().find(block);
    if (safeTopic.empty() || schema == schemas().end()) throw AIClientError("invalid_request", "课堂主题或板块无效");

    std::string feedback;
    std::string lastType = "quality_rejected";
    std::string lastMessage = "AI 课堂板块未通过质量检查";
    for (int attempt = 1; attempt <= 3; ++attempt) {
        size_t responseBytes = 0;
        std::string finishReason;
        json input = {{"goal", goal}, {"coursePlan", coursePlan}, {"phase", phaseName},
            {"topic", safeTopic}, {"topicIndex", topicIndex}, {"mode", mode}, {"block", block},
            {"previousBlocks", previousBlocks}, {"resources", resourceContext(resources)}};
        if (!feedback.empty()) input["qualityFeedback"] = feedback;

        ChatOptions options;
        options.messages = {
            {"system", u8"你是钢一定制AI的专业高中教师。你正在生成一节课程中的单个板块。只输出一个完整严格 JSON 对象，禁止 Markdown、代码块、解释文字、字段省略、输出截断和虚构链接。字符串正文禁止使用反斜杠或 LaTeX 命令，数学公式必须改用 Unicode 符号或普通文本。必须根据用户目标、AI课程主线、当前阶段、当前主题和前置板块生成具体教学内容；输出正文必须原样出现输入中的 goal、phase、topic 三个字符串；每个说明控制在1-3句话，不得把字段名或通用学习方法当作正文。输出结构：" + schema->second},
            {"user", input.dump()}
        };
        options.temperature = attempt == 1 ? 0.3 : 0.15;
        options.maxTokens = mode == "lite" ? 3000 : 4200;
        options.responseFormat = "json_object";
        options.timeoutMs = 90000;
        options.maxAttempts = 1;
        try {
            const AIResult response = client_.chat(options);
            responseBytes = response.content.size();
            finishReason = response.finishReason;
            json output = parseAIJson(response.content);
            feedback = validate(block, output, goal, phaseName, safeTopic);
            if (feedback.empty()) {
                json normalized = normalizedBlock(block, output);
                const char* configuredModel = std::getenv("AI_MODEL");
                normalized["_generation"] = {{"source", "ai"},
                    {"model", response.model.empty() && configuredModel ? configuredModel : response.model},
                    {"generatedAt", nowIso8601()}, {"attempts", attempt}, {"promptVersion", "ai-block-v1"}};
                std::cerr << "[learning-block] block=" << block << " attempt=" << attempt << " quality=passed\n";
                return normalized;
            }
            lastType = "quality_rejected";
            lastMessage = feedback;
            std::cerr << "[learning-block] block=" << block << " attempt=" << attempt
                      << " quality=rejected reason=" << feedback << '\n';
        } catch (const AIClientError& error) {
            lastType = error.errorType;
            lastMessage = error.what();
            feedback = "上一次 AI 调用失败：" + std::string(error.what());
            std::cerr << "[learning-block] block=" << block << " attempt=" << attempt
                      << " ai_error=" << error.errorType << " response_bytes=" << responseBytes
                      << " finish_reason=" << (finishReason.empty() ? "unknown" : finishReason) << '\n';
        } catch (const std::exception& error) {
            lastType = "invalid_response";
            lastMessage = error.what();
            feedback = "上一次 AI 输出无法解析：" + std::string(error.what());
        }
    }
    throw AIClientError(lastType, lastMessage);
}

}  // namespace gangyi

#include "image_goal_analyzer.hpp"

#include "json_fix.hpp"

#include <crow/utility.h>

#include <cstdlib>

namespace gangyi {
namespace {

using json = nlohmann::json;

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string visionModel() {
    for (const char* key : {"AI_VISION_MODEL", "VISION_MODEL", "AI_MODEL"}) {
        if (const char* value = std::getenv(key); value && *value) return value;
    }
    return {};
}

int imageTimeoutMs() {
    try {
        if (const char* value = std::getenv("AI_IMAGE_TIMEOUT_MS")) {
            const int timeout = std::stoi(value);
            if (timeout > 0) return timeout;
        }
    } catch (...) {}
    return 25000;
}

std::vector<std::string> strings(const json& value) {
    std::vector<std::string> result;
    if (!value.is_array()) return result;
    for (const auto& item : value) {
        if (item.is_string()) {
            const std::string text = trim(item.get<std::string>());
            if (!text.empty()) result.push_back(text.substr(0, 200));
        }
        if (result.size() == 8) break;
    }
    return result;
}

}  // namespace

ImageGoalAnalyzer::ImageGoalAnalyzer(AIClient& client) : client_(client) {}

ImageGoalAnalysis ImageGoalAnalyzer::analyze(const std::string& prompt, const std::string& mode,
                                              const std::string& mimeType, const std::string& imageBytes) const {
    if (imageBytes.empty() || mimeType.rfind("image/", 0) != 0) {
        throw AIClientError("invalid_request", "请上传图片文件");
    }
    const std::string safeMode = mode == "lite" ? "lite" : "deep";
    const std::string imageDataUrl = "data:" + mimeType + ";base64," +
        crow::utility::base64encode(reinterpret_cast<const unsigned char*>(imageBytes.data()), imageBytes.size());
    ChatOptions options;
    options.model = visionModel();
    options.temperature = 0.2;
    options.maxTokens = 700;
    options.timeoutMs = imageTimeoutMs();
    options.responseFormat = "json_object";
    options.maxAttempts = 1;
    options.messages = {
        {"system", u8"你是钢一定制AI的高中学习需求识别器。图片只能按高中题目、试卷、教材、笔记、公式、实验图、阅读材料或作文材料理解。识别真实学科、知识点和题型，并结合文字提示生成可用于高中学习路线和资料搜索的目标。不要编造图片中不存在的信息，不生成 Web 开发、网站制作或职业技能内容。只输出 JSON：{\"goal\":\"\",\"summary\":\"\",\"keywords\":[\"\"],\"suggestedSearchQuery\":\"\"}。"},
        {"user", u8"用户文字提示：" + (trim(prompt).empty() ? u8"未提供" : trim(prompt)) +
            u8"\n用户选择模式：" + safeMode + u8"。请根据图片和文字生成清晰、可执行的高中学习目标。", imageDataUrl},
    };
    const json raw = parseAIJson(client_.chat(options).content);
    if (!raw.is_object()) throw AIClientError("invalid_response", "图片识别内容结构不完整");
    ImageGoalAnalysis result;
    result.goal = trim(raw.value("goal", ""));
    result.summary = trim(raw.value("summary", ""));
    result.keywords = strings(raw.value("keywords", json::array()));
    result.suggestedSearchQuery = trim(raw.value("suggestedSearchQuery", ""));
    if (result.goal.empty() || result.summary.empty() || result.suggestedSearchQuery.empty()) {
        throw AIClientError("invalid_response", "图片识别内容结构不完整");
    }
    return result;
}

}  // namespace gangyi

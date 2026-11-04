// 【逐行批注】 引入头文件 "image_goal_analyzer.hpp"，为后续实现提供类型或接口声明。
#include "image_goal_analyzer.hpp"

// 【逐行批注】 引入头文件 "json_fix.hpp"，为后续实现提供类型或接口声明。
#include "json_fix.hpp"
// 【逐行批注】 引入头文件 "text_utils.hpp"，为后续实现提供类型或接口声明。
#include "text_utils.hpp"

// 【逐行批注】 引入头文件 <crow/utility.h>，为后续实现提供类型或接口声明。
#include <crow/utility.h>

// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 using json，更新当前运行状态。
using json = nlohmann::json;

// 【逐行批注】 定义函数 trim，执行该函数声明的职责。
std::string trim(const std::string& value) {
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

// 【逐行批注】 定义函数 visionModel，执行该函数声明的职责。
std::string visionModel() {
    // 【逐行批注】 按循环规则重复处理：for (const char* key : {"AI_VISION_MODEL", "VISION_MODEL", "AI_MODEL"}) {。
    for (const char* key : {"AI_VISION_MODEL", "VISION_MODEL", "AI_MODEL"}) {
        // 【逐行批注】 判断条件 (const char* value = std::getenv(key); value && *value) return v…，成立时执行对应分支。
        if (const char* value = std::getenv(key); value && *value) return value;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return {};。
    return {};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 imageTimeoutMs，执行该函数声明的职责。
int imageTimeoutMs() {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (const char* value = std::getenv("AI_IMAGE_TIMEOUT_MS")) {，成立时执行对应分支。
        if (const char* value = std::getenv("AI_IMAGE_TIMEOUT_MS")) {
            // 【逐行批注】 计算右侧结果并写入 int timeout，更新当前运行状态。
            const int timeout = std::stoi(value);
            // 【逐行批注】 判断条件 (timeout > 0) return timeout;，成立时执行对应分支。
            if (timeout > 0) return timeout;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {}。
    } catch (...) {}
    // 【逐行批注】 结束当前函数并返回结果：return 240000;。
    return 240000;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 strings，执行该函数声明的职责。
std::vector<std::string> strings(const json& value) {
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> result;。
    std::vector<std::string> result;
    // 【逐行批注】 判断条件 (!value.is_array()) return result;，成立时执行对应分支。
    if (!value.is_array()) return result;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : value) {。
    for (const auto& item : value) {
        // 【逐行批注】 判断条件 (item.is_string()) {，成立时执行对应分支。
        if (item.is_string()) {
            // 【逐行批注】 计算右侧结果并写入 std::string text，更新当前运行状态。
            const std::string text = trim(item.get<std::string>());
            // 【逐行批注】 判断条件 (!text.empty()) result.push_back(truncateUtf8(text, 200));，成立时执行对应分支。
            if (!text.empty()) result.push_back(truncateUtf8(text, 200));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (result.size() == 8) break;，成立时执行对应分支。
        if (result.size() == 8) break;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 调用 ImageGoalAnalyzer、client_ 完成本行所需操作：ImageGoalAnalyzer::ImageGoalAnalyzer(AIClient& client) : client_(clie…。
ImageGoalAnalyzer::ImageGoalAnalyzer(AIClient& client) : client_(client) {}

// 【逐行批注】 调用 analyze 完成本行所需操作：ImageGoalAnalysis ImageGoalAnalyzer::analyze(const std::string& promp…。
ImageGoalAnalysis ImageGoalAnalyzer::analyze(const std::string& prompt, const std::string& mode,
                                              // 【逐行批注】 执行当前声明或表达式：const std::string& mimeType, const std::string& imageBytes) const {。
                                              const std::string& mimeType, const std::string& imageBytes) const {
    // 【逐行批注】 判断条件 (imageBytes.empty() || mimeType.rfind("image/", 0) != 0) {，成立时执行对应分支。
    if (imageBytes.empty() || mimeType.rfind("image/", 0) != 0) {
        // 【逐行批注】 检测到错误情形时抛出异常：throw AIClientError("invalid_request", "请上传图片文件");。
        throw AIClientError("invalid_request", "请上传图片文件");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 std::string safeMode，更新当前运行状态。
    const std::string safeMode = mode == "lite" ? "lite" : "deep";
    // 【逐行批注】 计算右侧结果并写入 std::string imageDataUrl，更新当前运行状态。
    const std::string imageDataUrl = "data:" + mimeType + ";base64," +
        // 【逐行批注】 调用 base64encode 完成本行所需操作：crow::utility::base64encode(reinterpret_cast<const unsigned char*>(im…。
        crow::utility::base64encode(reinterpret_cast<const unsigned char*>(imageBytes.data()), imageBytes.size());
    // 【逐行批注】 执行当前声明或表达式：ChatOptions options;。
    ChatOptions options;
    // 【逐行批注】 计算右侧结果并写入 options.activity.source，更新当前运行状态。
    options.activity.source = "目标图片"; options.activity.purpose = "识别图片内容并提取学习目标";
    // 【逐行批注】 计算右侧结果并写入 options.model，更新当前运行状态。
    options.model = visionModel();
    // 【逐行批注】 计算右侧结果并写入 options.temperature，更新当前运行状态。
    options.temperature = 0.2;
    // 【逐行批注】 计算右侧结果并写入 options.maxTokens，更新当前运行状态。
    options.maxTokens = 1600;
    // 【逐行批注】 计算右侧结果并写入 options.timeoutMs，更新当前运行状态。
    options.timeoutMs = imageTimeoutMs();
    // 【逐行批注】 计算右侧结果并写入 options.responseFormat，更新当前运行状态。
    options.responseFormat = "json_object";
    // 【逐行批注】 计算右侧结果并写入 options.maxAttempts，更新当前运行状态。
    options.maxAttempts = 1;
    // 【逐行批注】 计算右侧结果并写入 options.messages，更新当前运行状态。
    options.messages = {
        // 【逐行批注】 执行当前声明或表达式：{"system", u8"你是钢一定制AI的高中学习需求识别器。图片只能按高中题目、试卷、教材、笔记、公式、实验图、阅读材料或作文材料理解。识别真实学科、知识点和题型…。
        {"system", u8"你是钢一定制AI的高中学习需求识别器。图片只能按高中题目、试卷、教材、笔记、公式、实验图、阅读材料或作文材料理解。识别真实学科、知识点和题型，并结合文字提示生成可用于高中学习路线和资料搜索的目标。不要编造图片中不存在的信息，不生成 Web 开发、网站制作或职业技能内容。只输出 JSON：{\"goal\":\"\",\"summary\":\"\",\"keywords\":[\"\"],\"suggestedSearchQuery\":\"\"}。"},
        // 【逐行批注】 调用 trim 完成本行所需操作：{"user", u8"用户文字提示：" + (trim(prompt).empty() ? u8"未提供" : trim(prompt)…。
        {"user", u8"用户文字提示：" + (trim(prompt).empty() ? u8"未提供" : trim(prompt)) +
            // 【逐行批注】 执行当前声明或表达式：u8"\n用户选择模式：" + safeMode + u8"。请根据图片和文字生成清晰、可执行的高中学习目标。", imageDataUrl},。
            u8"\n用户选择模式：" + safeMode + u8"。请根据图片和文字生成清晰、可执行的高中学习目标。", imageDataUrl},
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 json raw，更新当前运行状态。
    const json raw = parseAIJson(client_.chat(options).content);
    // 【逐行批注】 判断条件 (!raw.is_object()) throw AIClientError("invalid_response", "图片识别…，成立时执行对应分支。
    if (!raw.is_object()) throw AIClientError("invalid_response", "图片识别内容结构不完整");
    // 【逐行批注】 执行当前声明或表达式：ImageGoalAnalysis result;。
    ImageGoalAnalysis result;
    // 【逐行批注】 计算右侧结果并写入 result.goal，更新当前运行状态。
    result.goal = trim(raw.value("goal", ""));
    // 【逐行批注】 计算右侧结果并写入 result.summary，更新当前运行状态。
    result.summary = trim(raw.value("summary", ""));
    // 【逐行批注】 计算右侧结果并写入 result.keywords，更新当前运行状态。
    result.keywords = strings(raw.value("keywords", json::array()));
    // 【逐行批注】 计算右侧结果并写入 result.suggestedSearchQuery，更新当前运行状态。
    result.suggestedSearchQuery = trim(raw.value("suggestedSearchQuery", ""));
    // 【逐行批注】 判断条件 (result.goal.empty() || result.summary.empty() || result.suggest…，成立时执行对应分支。
    if (result.goal.empty() || result.summary.empty() || result.suggestedSearchQuery.empty()) {
        // 【逐行批注】 检测到错误情形时抛出异常：throw AIClientError("invalid_response", "图片识别内容结构不完整");。
        throw AIClientError("invalid_response", "图片识别内容结构不完整");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

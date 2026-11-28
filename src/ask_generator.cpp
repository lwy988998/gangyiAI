// 【逐行批注】 引入头文件 "ask_generator.hpp"，为后续实现提供类型或接口声明。
#include "ask_generator.hpp"

// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

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

// 【逐行批注】 定义函数 askTimeoutMs，执行该函数声明的职责。
int askTimeoutMs() {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (const char* value = std::getenv("AI_ASK_TIMEOUT_MS")) {，成立时执行对应分支。
        if (const char* value = std::getenv("AI_ASK_TIMEOUT_MS")) {
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

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 调用 AskGenerator、client_ 完成本行所需操作：AskGenerator::AskGenerator(AIClient& client) : client_(client) {}。
AskGenerator::AskGenerator(AIClient& client) : client_(client) {}

// 【逐行批注】 调用 options 完成本行所需操作：ChatOptions AskGenerator::options(const std::string& question, const …。
ChatOptions AskGenerator::options(const std::string& question, const std::string& profileContext,
                                 // 【逐行批注】 执行当前声明或表达式：const std::vector<ChatMessage>& history) {。
                                 const std::vector<ChatMessage>& history) {
    // 【逐行批注】 计算右侧结果并写入 std::string safeQuestion，更新当前运行状态。
    const std::string safeQuestion = trim(question);
    // 【逐行批注】 判断条件 (safeQuestion.empty() || safeQuestion.size() > 4000)，成立时执行对应分支。
    if (safeQuestion.empty() || safeQuestion.size() > 4000)
        // 【逐行批注】 检测到错误情形时抛出异常：throw AIClientError("invalid_request", "请输入有效问题（最多 4000 字节）");。
        throw AIClientError("invalid_request", "请输入有效问题（最多 4000 字节）");
    // 【逐行批注】 执行当前声明或表达式：ChatOptions options;。
    ChatOptions options;
    // 【逐行批注】 计算右侧结果并写入 options.activity.source，更新当前运行状态。
    options.activity.source = "AI 导师"; options.activity.purpose = "回答问题并进行连续交流";
    // 【逐行批注】 执行当前声明或表达式：options.messages.push_back({"system", u8"你是钢一定制AI的学习导师。结合最近对话理解追问和指代，直接、准确地回答。默认简体中文…。
    options.messages.push_back({"system", u8"你是钢一定制AI的学习导师。结合最近对话理解追问和指代，直接、准确地回答。默认简体中文；先定位误解，再给示例或一个引导问题。不要编造事实。学习画像仅作辅助，不得把低分当成学生能力定论。画像摘要：" + profileContext});
    // 【逐行批注】 计算右侧结果并写入 size_t start，更新当前运行状态。
    const size_t start = history.size() > 8 ? history.size() - 8 : 0;
    // 【逐行批注】 按循环规则重复处理：for (size_t i = start; i < history.size(); ++i) {。
    for (size_t i = start; i < history.size(); ++i) {
        // 【逐行批注】 计算右侧结果并写入 auto& message，更新当前运行状态。
        const auto& message = history[i];
        // 【逐行批注】 判断条件 ((message.role != "user" && message.role != "assistant") || mess…，成立时执行对应分支。
        if ((message.role != "user" && message.role != "assistant") || message.content.empty()) continue;
        // 【逐行批注】 判断条件 (message.content.size() > 16000) throw AIClientError("invalid_re…，成立时执行对应分支。
        if (message.content.size() > 16000) throw AIClientError("invalid_request", "历史消息过长");
        // 【逐行批注】 判断条件 (i + 1 == history.size() && message.role == "user" && trim(messa…，成立时执行对应分支。
        if (i + 1 == history.size() && message.role == "user" && trim(message.content) == safeQuestion) continue;
        // 【逐行批注】 执行当前声明或表达式：options.messages.push_back(message);。
        options.messages.push_back(message);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：options.messages.push_back({"user", safeQuestion});。
    options.messages.push_back({"user", safeQuestion});
    // 【逐行批注】 计算右侧结果并写入 options.temperature，更新当前运行状态。
    options.temperature = 0.5;
    // 【逐行批注】 计算右侧结果并写入 options.maxTokens，更新当前运行状态。
    options.maxTokens = 3000;
    // 【逐行批注】 计算右侧结果并写入 options.timeoutMs，更新当前运行状态。
    options.timeoutMs = askTimeoutMs();
    // 【逐行批注】 计算右侧结果并写入 options.maxAttempts，更新当前运行状态。
    options.maxAttempts = 1;
    // 【逐行批注】 计算右侧结果并写入 std::string subject，更新当前运行状态。
    std::string subject = u8"高中学科知识";
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : {u8"数学", u8"英语", u8"语文", u8"物理", u8"化学", u8"生物", u…。
    for (const auto& item : {u8"数学", u8"英语", u8"语文", u8"物理", u8"化学", u8"生物", u8"历史", u8"地理", u8"政治"})
        // 【逐行批注】 判断条件 (safeQuestion.find(item) != std::string::npos) { subject = item;…，成立时执行对应分支。
        if (safeQuestion.find(item) != std::string::npos) { subject = item; break; }
    // 【逐行批注】 计算右侧结果并写入 options.searchQuery，更新当前运行状态。
    options.searchQuery = subject.empty() ? u8"高中学科知识" : subject;

    // 【逐行批注】 结束当前函数并返回结果：return options;。
    return options;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 调用 generate 完成本行所需操作：AskAnswer AskGenerator::generate(const std::string& question, const s…。
AskAnswer AskGenerator::generate(const std::string& question, const std::string& profileContext,
                               // 【逐行批注】 执行当前声明或表达式：const std::vector<ChatMessage>& history) const {。
                               const std::vector<ChatMessage>& history) const {
    // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
    const auto result = client_.chat(options(question, profileContext, history));
    // 【逐行批注】 计算右侧结果并写入 std::string content，更新当前运行状态。
    const std::string content = trim(result.content);
    // 【逐行批注】 判断条件 (content.empty()) throw AIClientError("invalid_response", "AI 没有…，成立时执行对应分支。
    if (content.empty()) throw AIClientError("invalid_response", "AI 没有返回内容");
    // 【逐行批注】 结束当前函数并返回结果：return {content, result.model, result.searchStatus, result.sources};。
    return {content, result.model, result.searchStatus, result.sources};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

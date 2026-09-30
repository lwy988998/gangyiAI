#include "ask_generator.hpp"

#include <cstdlib>

namespace gangyi {
namespace {

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

int askTimeoutMs() {
    try {
        if (const char* value = std::getenv("AI_ASK_TIMEOUT_MS")) {
            const int timeout = std::stoi(value);
            if (timeout > 0) return timeout;
        }
    } catch (...) {}
    return 25000;
}

}  // namespace

AskGenerator::AskGenerator(AIClient& client) : client_(client) {}

ChatOptions AskGenerator::options(const std::string& question, const std::string& profileContext,
                                 const std::vector<ChatMessage>& history) {
    const std::string safeQuestion = trim(question);
    if (safeQuestion.empty() || safeQuestion.size() > 4000)
        throw AIClientError("invalid_request", "请输入有效问题（最多 4000 字节）");
    ChatOptions options;
    options.messages.push_back({"system", u8"你是钢一定制AI的学习导师。结合最近对话理解追问和指代，直接、准确地回答。默认简体中文；先定位误解，再给示例或一个引导问题。不要编造事实。学习画像仅作辅助，不得把低分当成学生能力定论。画像摘要：" + profileContext});
    const size_t start = history.size() > 8 ? history.size() - 8 : 0;
    for (size_t i = start; i < history.size(); ++i) {
        const auto& message = history[i];
        if ((message.role != "user" && message.role != "assistant") || message.content.empty()) continue;
        if (message.content.size() > 16000) throw AIClientError("invalid_request", "历史消息过长");
        if (i + 1 == history.size() && message.role == "user" && trim(message.content) == safeQuestion) continue;
        options.messages.push_back(message);
    }
    options.messages.push_back({"user", safeQuestion});
    options.temperature = 0.5;
    options.maxTokens = 3000;
    options.timeoutMs = askTimeoutMs();
    options.maxAttempts = 1;
    std::string subject = u8"高中学科知识";
    for (const auto& item : {u8"数学", u8"英语", u8"语文", u8"物理", u8"化学", u8"生物", u8"历史", u8"地理", u8"政治"})
        if (safeQuestion.find(item) != std::string::npos) { subject = item; break; }
    options.searchQuery = subject.empty() ? u8"高中学科知识" : subject;

    return options;
}

AskAnswer AskGenerator::generate(const std::string& question, const std::string& profileContext,
                               const std::vector<ChatMessage>& history) const {
    const auto result = client_.chat(options(question, profileContext, history));
    const std::string content = trim(result.content);
    if (content.empty()) throw AIClientError("invalid_response", "AI 没有返回内容");
    return {content, result.model, result.searchStatus, result.sources};
}

}  // namespace gangyi

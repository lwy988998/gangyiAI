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

std::vector<ChatMessage> prompt(const std::string& question, const std::string& profileContext) {
    return {
        {"system", u8"你是钢一定制AI。请直接、准确地回答用户的问题。默认使用简体中文；问题不清楚时先说明缺少的信息。不要编造事实。学习画像仅作辅助，不得把低分当成学生能力定论。画像摘要：" + profileContext, ""},
        {"user", question, ""},
    };
}

}  // namespace

AskGenerator::AskGenerator(AIClient& client) : client_(client) {}

AskAnswer AskGenerator::generate(const std::string& question, const std::string& profileContext) const {
    const std::string safeQuestion = trim(question);
    if (safeQuestion.empty()) throw AIClientError("invalid_request", "请提供问题");
    ChatOptions options;
    options.messages = prompt(safeQuestion, profileContext);
    options.temperature = 0.5;
    options.maxTokens = 3000;
    options.timeoutMs = askTimeoutMs();
    options.maxAttempts = 1;
    std::string subject = u8"高中学科知识";
    for (const auto& item : {u8"数学", u8"英语", u8"语文", u8"物理", u8"化学", u8"生物", u8"历史", u8"地理", u8"政治"})
        if (safeQuestion.find(item) != std::string::npos) { subject = item; break; }
    options.searchQuery = subject.empty() ? u8"高中学科知识" : subject;

    const auto result = client_.chat(options);
    const std::string content = trim(result.content);
    if (content.empty()) throw AIClientError("invalid_response", "AI 没有返回内容");
    return {content, result.model, result.searchStatus, result.sources};
}

}  // namespace gangyi

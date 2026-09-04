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

std::vector<ChatMessage> prompt(const std::string& question) {
    return {
        {"system", u8"你是钢一定制AI。请直接、准确地回答用户的问题。默认使用简体中文；问题不清楚时先说明缺少的信息。不要编造事实。"},
        {"user", question},
    };
}

}  // namespace

AskGenerator::AskGenerator(AIClient& client) : client_(client) {}

AskAnswer AskGenerator::generate(const std::string& question) const {
    const std::string safeQuestion = trim(question);
    if (safeQuestion.empty()) throw AIClientError("invalid_request", "请提供问题");
    ChatOptions options;
    options.messages = prompt(safeQuestion);
    options.model = "deepseek-v4-flash";
    options.temperature = 0.5;
    options.maxTokens = 3000;
    options.timeoutMs = askTimeoutMs();
    options.maxAttempts = 1;

    const std::string content = trim(client_.chat(options).content);
    if (content.empty()) throw AIClientError("invalid_response", "AI 没有返回内容");
    return {content};
}

}  // namespace gangyi

#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace gangyi {

struct ChatMessage {
    std::string role;
    std::string content;
    std::string imageDataUrl;
};

struct ChatOptions {
    std::vector<ChatMessage> messages;
    std::string model;
    double temperature = 0.7;
    int maxTokens = 4096;
    std::string responseFormat;
    int timeoutMs = 0;
    int maxAttempts = 0;
};

struct AIResult {
    std::string content;
    std::string model;
    int status = 0;
};

class AIClientError : public std::runtime_error {
public:
    AIClientError(const std::string& errorType, const std::string& message);
    std::string errorType;
};

class AIClient {
public:
    AIClient();
    AIResult chat(const ChatOptions& options) const;

private:
    std::string baseUrl_;
    std::string apiKey_;
    std::string model_;
    int timeoutMs_;
    int retryAttempts_;
    mutable int consecutiveFailures_ = 0;
    mutable long long circuitOpenedAtMs_ = 0;
};

}

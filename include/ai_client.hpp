#pragma once
#include "ai_activity.hpp"

#include <stdexcept>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace gangyi {

struct ChatMessage {
    std::string role;
    std::string content;
    std::string imageDataUrl;

    ChatMessage(std::string roleValue, std::string contentValue, std::string imageDataUrlValue = {})
        : role(std::move(roleValue)), content(std::move(contentValue)),
          imageDataUrl(std::move(imageDataUrlValue)) {}
};

struct ChatOptions {
    std::vector<ChatMessage> messages;
    std::string model;
    double temperature = 0.7;
    int maxTokens = 4096;
    std::string responseFormat;
    int timeoutMs = 0;
    int maxAttempts = 0;
    std::function<bool()> cancelled;
    std::string searchQuery;
    AIActivityInfo activity{};
    bool requestTokenUsage = true;
};

struct AIResult {
    std::string content;
    std::string model;
    int status = 0;
    std::string finishReason;
    std::string searchStatus = "not_requested";
    std::vector<std::string> sources;
    // 保留服务商实际返回的用量及缓存/思考明细；缺失保持空值，不按文字长度估算。
    nlohmann::json tokenUsage = nullptr;
};

struct AIClientConfig {
    std::string baseUrl;
    std::string apiKey;
    std::string model;
    int timeoutMs = 35000;
    int retryAttempts = 2;
};

class AIClientError : public std::runtime_error {
public:
    AIClientError(const std::string& errorType, const std::string& message, int httpStatus = 0);
    std::string errorType;
    int httpStatus = 0;
    bool retryable = false;
    long long retryAfterMs = 0;
    bool contentDelivered = false;
    // 仅保存固定错误码、阶段及连接里程碑，不包含端点、代理地址或凭据。
    nlohmann::json transportDiagnostic = nullptr;
};

class AIClient {
public:
    AIClient();
    explicit AIClient(AIClientConfig config);
    ~AIClient();
    AIResult chat(const ChatOptions& options) const;
    AIResult chatStream(const ChatOptions& options,
                        const std::function<bool(const std::string&)>& onChunk) const;
    std::vector<std::string> listModels(int timeoutMs = 15000) const;

private:
    std::string baseUrl_;
    std::string apiKey_;
    std::string model_;
    int timeoutMs_;
    int retryAttempts_;
};

}

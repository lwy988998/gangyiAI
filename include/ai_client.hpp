// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "ai_activity.hpp"，为后续实现提供类型或接口声明。
#include "ai_activity.hpp"

// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>
// 【逐行批注】 引入头文件 <functional>，为后续实现提供类型或接口声明。
#include <functional>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <utility>，为后续实现提供类型或接口声明。
#include <utility>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 声明数据结构或类型 ChatMessage 及其成员接口。
struct ChatMessage {
    // 【逐行批注】 执行当前声明或表达式：std::string role;。
    std::string role;
    // 【逐行批注】 执行当前声明或表达式：std::string content;。
    std::string content;
    // 【逐行批注】 执行当前声明或表达式：std::string imageDataUrl;。
    std::string imageDataUrl;

    // 【逐行批注】 计算右侧结果并写入 ChatMessage(std::string roleValue, std::stri…，更新当前运行状态。
    ChatMessage(std::string roleValue, std::string contentValue, std::string imageDataUrlValue = {})
        // 【逐行批注】 声明对象 role，并通过构造函数初始化其状态。
        : role(std::move(roleValue)), content(std::move(contentValue)),
          // 【逐行批注】 调用 imageDataUrl、move 完成本行所需操作：imageDataUrl(std::move(imageDataUrlValue)) {}。
          imageDataUrl(std::move(imageDataUrlValue)) {}
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 ChatOptions 及其成员接口。
struct ChatOptions {
    // 【逐行批注】 执行当前声明或表达式：std::vector<ChatMessage> messages;。
    std::vector<ChatMessage> messages;
    // 【逐行批注】 执行当前声明或表达式：std::string model;。
    std::string model;
    // 【逐行批注】 计算右侧结果并写入 double temperature，更新当前运行状态。
    double temperature = 0.7;
    // 【逐行批注】 计算右侧结果并写入 int maxTokens，更新当前运行状态。
    int maxTokens = 4096;
    // 【逐行批注】 执行当前声明或表达式：std::string responseFormat;。
    std::string responseFormat;
    // 【逐行批注】 计算右侧结果并写入 int timeoutMs，更新当前运行状态。
    int timeoutMs = 0;
    // 【逐行批注】 计算右侧结果并写入 int maxAttempts，更新当前运行状态。
    int maxAttempts = 0;
    // 【逐行批注】 调用 bool 完成本行所需操作：std::function<bool()> cancelled;。
    std::function<bool()> cancelled;
    // 【逐行批注】 执行当前声明或表达式：std::string searchQuery;。
    std::string searchQuery;
    // 【逐行批注】 执行当前声明或表达式：AIActivityInfo activity{};。
    AIActivityInfo activity{};
    // 【逐行批注】 计算右侧结果并写入 bool requestTokenUsage，更新当前运行状态。
    bool requestTokenUsage = true;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 AIResult 及其成员接口。
struct AIResult {
    // 【逐行批注】 执行当前声明或表达式：std::string content;。
    std::string content;
    // 【逐行批注】 执行当前声明或表达式：std::string model;。
    std::string model;
    // 【逐行批注】 计算右侧结果并写入 int status，更新当前运行状态。
    int status = 0;
    // 【逐行批注】 执行当前声明或表达式：std::string finishReason;。
    std::string finishReason;
    // 【逐行批注】 计算右侧结果并写入 std::string searchStatus，更新当前运行状态。
    std::string searchStatus = "not_requested";
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> sources;。
    std::vector<std::string> sources;
    // 保留服务商实际返回的用量及缓存/思考明细；缺失保持空值，不按文字长度估算。
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json tokenUsage，更新当前运行状态。
    nlohmann::json tokenUsage = nullptr;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 AIClientConfig 及其成员接口。
struct AIClientConfig {
    // 【逐行批注】 执行当前声明或表达式：std::string baseUrl;。
    std::string baseUrl;
    // 【逐行批注】 执行当前声明或表达式：std::string apiKey;。
    std::string apiKey;
    // 【逐行批注】 执行当前声明或表达式：std::string model;。
    std::string model;
    // 【逐行批注】 计算右侧结果并写入 int timeoutMs，更新当前运行状态。
    int timeoutMs = 35000;
    // 【逐行批注】 计算右侧结果并写入 int retryAttempts，更新当前运行状态。
    int retryAttempts = 2;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 AIClientError 及其成员接口。
class AIClientError : public std::runtime_error {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 计算右侧结果并写入 AIClientError(const std::string& errorType, …，更新当前运行状态。
    AIClientError(const std::string& errorType, const std::string& message, int httpStatus = 0);
    // 【逐行批注】 执行当前声明或表达式：std::string errorType;。
    std::string errorType;
    // 【逐行批注】 计算右侧结果并写入 int httpStatus，更新当前运行状态。
    int httpStatus = 0;
    // 【逐行批注】 计算右侧结果并写入 bool retryable，更新当前运行状态。
    bool retryable = false;
    // 【逐行批注】 计算右侧结果并写入 long long retryAfterMs，更新当前运行状态。
    long long retryAfterMs = 0;
    // 【逐行批注】 计算右侧结果并写入 bool contentDelivered，更新当前运行状态。
    bool contentDelivered = false;
    // 仅保存固定错误码、阶段及连接里程碑，不包含端点、代理地址或凭据。
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json transportDiagnostic，更新当前运行状态。
    nlohmann::json transportDiagnostic = nullptr;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 AIClient 及其成员接口。
class AIClient {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 调用 AIClient 完成本行所需操作：AIClient();。
    AIClient();
    // 【逐行批注】 声明对象 AIClient，并通过构造函数初始化其状态。
    explicit AIClient(AIClientConfig config);
    // 【逐行批注】 调用 AIClient 完成本行所需操作：~AIClient();。
    ~AIClient();
    // 【逐行批注】 声明对象 chat，并通过构造函数初始化其状态。
    AIResult chat(const ChatOptions& options) const;
    // 【逐行批注】 声明对象 chatStream，并通过构造函数初始化其状态。
    AIResult chatStream(const ChatOptions& options,
                        // 【逐行批注】 调用 bool 完成本行所需操作：const std::function<bool(const std::string&)>& onChunk) const;。
                        const std::function<bool(const std::string&)>& onChunk) const;
    // 【逐行批注】 声明对象 listModels，并通过构造函数初始化其状态。
    std::vector<std::string> listModels(int timeoutMs = 15000) const;

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：std::string baseUrl_;。
    std::string baseUrl_;
    // 【逐行批注】 执行当前声明或表达式：std::string apiKey_;。
    std::string apiKey_;
    // 【逐行批注】 执行当前声明或表达式：std::string model_;。
    std::string model_;
    // 【逐行批注】 执行当前声明或表达式：int timeoutMs_;。
    int timeoutMs_;
    // 【逐行批注】 执行当前声明或表达式：int retryAttempts_;。
    int retryAttempts_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 结束当前代码块。
}

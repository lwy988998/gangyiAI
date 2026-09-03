#include "ai_client.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

namespace gangyi {
namespace {

using json = nlohmann::json;

std::string env(const char* name, const std::string& fallback = {}) {
    const char* value = std::getenv(name);
    return value ? value : fallback;
}

int envInt(const char* name, int fallback) {
    try { return std::stoi(env(name, std::to_string(fallback))); } catch (...) { return fallback; }
}

size_t writeBody(char* data, size_t size, size_t count, void* user) {
    static_cast<std::string*>(user)->append(data, size * count);
    return size * count;
}

long long nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

struct Endpoint { std::string url; std::string key; std::string model; };

// 归一化 chat/completions 端点：baseUrl 可能带 /v1 或不带
std::string chatEndpoint(const std::string& baseUrl) {
    std::string url = baseUrl;
    while (!url.empty() && url.back() == '/') url.pop_back();
    if (url.find("/chat/completions") == std::string::npos) {
        url += "/chat/completions";
    }
    return url;
}
AIClientError errorFor(CURLcode code, long status, const std::string& message) {
    if (code == CURLE_OPERATION_TIMEDOUT) return AIClientError("timeout", message);
    if (code != CURLE_OK) return AIClientError("network_error", message);
    if (status == 401 || status == 403) return AIClientError("auth_error", message);
    if (status == 429) return AIClientError("rate_limited", message);
    if (status >= 500 && status <= 599) return AIClientError("provider_5xx", message);
    return AIClientError("unknown", message);
}

AIResult request(const Endpoint& endpoint, const ChatOptions& options, int timeoutMs) {
    if (endpoint.key.empty()) throw AIClientError("missing_config", "AI_API_KEY is not configured");
    CURL* curl = curl_easy_init();
    if (!curl) throw AIClientError("network_error", "unable to initialize curl");

    json body{{"model", options.model.empty() ? endpoint.model : options.model},
              {"temperature", options.temperature}, {"max_tokens", options.maxTokens}};
    body["messages"] = json::array();
    for (const auto& message : options.messages) {
        if (message.imageDataUrl.empty()) {
            body["messages"].push_back({{"role", message.role}, {"content", message.content}});
        } else {
            body["messages"].push_back({{"role", message.role}, {"content", json::array({
                {{"type", "text"}, {"text", message.content}},
                {{"type", "image_url"}, {"image_url", {{"url", message.imageDataUrl}}}},
            })}});
        }
    }
    if (!options.responseFormat.empty()) body["response_format"] = {{"type", options.responseFormat}};

    if (std::getenv("AI_DEBUG")) {
        std::cerr << "[ai-debug] POST " << chatEndpoint(endpoint.url) << std::endl
                  << "[ai-debug] body=" << body.dump().substr(0, 2000) << std::endl;
    }

    std::string responseBody;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    const std::string authorization = "Authorization: Bearer " + endpoint.key;
    headers = curl_slist_append(headers, authorization.c_str());
    // 必须存局部变量：body.dump() 的临时 string 在语句结束即析构，直接传 c_str() 会悬垂导致请求体为空
    const std::string postBody = body.dump();
    curl_easy_setopt(curl, CURLOPT_URL, chatEndpoint(endpoint.url).c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postBody.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(postBody.size()));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, static_cast<long>(timeoutMs));
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    CURLcode code = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    if (code != CURLE_OK || status < 200 || status >= 300) {
        if (std::getenv("AI_DEBUG")) {
            std::cerr << "[ai-debug] response status=" << status
                      << " body=" << responseBody.substr(0, 2000) << std::endl;
        }
        throw errorFor(code, status, code == CURLE_OK ? "AI provider returned HTTP " + std::to_string(status) : curl_easy_strerror(code));
    }
    try {
        const json parsed = json::parse(responseBody);
        const auto& choice = parsed.at("choices").at(0).at("message");
        return {choice.at("content").get<std::string>(), parsed.value("model", ""), static_cast<int>(status)};
    } catch (const std::exception& error) {
        throw AIClientError("invalid_response", error.what());
    }
}

AIResult attempt(const Endpoint& endpoint, const ChatOptions& options, int timeoutMs, int attempts) {
    AIClientError last("unknown", "AI request failed");
    for (int i = 0; i < attempts; ++i) {
        try { return request(endpoint, options, timeoutMs); }
        catch (const AIClientError& error) {
            last = error;
            const bool retryable = error.errorType == "timeout" || error.errorType == "network_error" ||
                error.errorType == "rate_limited" || error.errorType == "provider_5xx";
            if (!retryable || i + 1 == attempts) throw;
            std::this_thread::sleep_for(std::chrono::milliseconds(800 * (1 << i)));
        }
    }
    throw last;
}
}

AIClientError::AIClientError(const std::string& type, const std::string& message)
    : std::runtime_error(message), errorType(type) {}

AIClient::AIClient()
    : baseUrl_(env("AI_BASE_URL", "https://api.deepseek.com/v1")), apiKey_(env("AI_API_KEY")),
      model_(env("AI_MODEL", "deepseek-chat")), timeoutMs_(envInt("AI_TIMEOUT_MS", 35000)),
      retryAttempts_(envInt("AI_RETRY_ATTEMPTS", 2)) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

AIResult AIClient::chat(const ChatOptions& options) const {
    const int timeout = options.timeoutMs > 0 ? options.timeoutMs : timeoutMs_;
    const int attempts = options.maxAttempts > 0 ? options.maxAttempts : retryAttempts_;
    if (attempts < 1) throw AIClientError("unknown", "maxAttempts must be positive");
    if (circuitOpenedAtMs_ && nowMs() - circuitOpenedAtMs_ < 60000)
        throw AIClientError("network_error", "AI provider circuit is open");
    if (circuitOpenedAtMs_) { circuitOpenedAtMs_ = 0; consecutiveFailures_ = 0; }

    const Endpoint primary{baseUrl_, apiKey_, model_};
    try {
        AIResult result = attempt(primary, options, timeout, attempts);
        consecutiveFailures_ = 0;
        return result;
    } catch (const AIClientError& primaryError) {
        if (++consecutiveFailures_ >= 3) circuitOpenedAtMs_ = nowMs();
        throw primaryError;
    }
}
}

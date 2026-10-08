#include "ai_client.hpp"
#include "search_client.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <thread>
#include <utility>

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

AIClientConfig configFromEnvironment() {
    return {env("AI_BASE_URL", "https://api.deepseek.com/v1"), env("AI_API_KEY"),
        env("AI_MODEL", "deepseek-chat"), envInt("AI_TIMEOUT_MS", 35000),
        envInt("AI_RETRY_ATTEMPTS", 2)};
}

void secureClear(std::string& value) {
    volatile char* data = value.empty() ? nullptr : value.data();
    for (size_t index = 0; index < value.size(); ++index) data[index] = '\0';
    value.clear();
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

std::string modelsEndpoint(const std::string& baseUrl) {
    std::string url = baseUrl;
    while (!url.empty() && url.back() == '/') url.pop_back();
    constexpr const char* chatSuffix = "/chat/completions";
    if (url.size() >= std::char_traits<char>::length(chatSuffix) &&
        url.compare(url.size() - std::char_traits<char>::length(chatSuffix),
            std::char_traits<char>::length(chatSuffix), chatSuffix) == 0) {
        url.erase(url.size() - std::char_traits<char>::length(chatSuffix));
    }
    if (url.size() < 7 || url.compare(url.size() - 7, 7, "/models") != 0) url += "/models";
    return url;
}
AIClientError errorFor(CURLcode code, long status, const std::string& message) {
    if (code == CURLE_OPERATION_TIMEDOUT) return AIClientError("timeout", message);
    if (code != CURLE_OK) return AIClientError("network_error", message);
    if (status == 401 || status == 403) return AIClientError("auth_error", message, static_cast<int>(status));
    if (status == 402) return AIClientError("provider_rejected", message, 402);
    if (status == 429) return AIClientError("rate_limited", message, 429);
    if (status >= 500 && status <= 599) return AIClientError("provider_5xx", message, static_cast<int>(status));
    return AIClientError("provider_rejected", message, static_cast<int>(status));
}

AIResult request(const Endpoint& endpoint, const ChatOptions& options, int timeoutMs) {
    if (endpoint.key.empty()) throw AIClientError("missing_config", "AI_API_KEY is not configured");
    CURL* curl = curl_easy_init();
    if (!curl) throw AIClientError("network_error", "unable to initialize curl");

    const std::string selectedModel = options.model.empty() ? endpoint.model : options.model;
    json body{{"model", selectedModel},
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
        std::cerr << "[ai-debug] POST chat/completions" << std::endl
                  << "[ai-debug] request body omitted" << std::endl;
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
                      << " body omitted" << std::endl;
        }
        throw errorFor(code, status, code == CURLE_OK ? "AI provider returned HTTP " + std::to_string(status) : curl_easy_strerror(code));
    }
    try {
        const json parsed = json::parse(responseBody);
        const auto& choice = parsed.at("choices").at(0);
        const auto& message = choice.at("message");
        std::string responseModel = parsed.value("model", selectedModel);
        if (responseModel.empty()) responseModel = selectedModel;
        AIResult result;
        result.content = message.at("content").get<std::string>();
        result.model = std::move(responseModel);
        result.status = static_cast<int>(status);
        result.finishReason = choice.value("finish_reason", "");
        return result;
    } catch (const std::exception& error) {
        throw AIClientError("invalid_response", error.what());
    }
}

struct StreamState {
    std::string pending, content, model, finishReason;
    std::function<bool(const std::string&)> onChunk;
    bool cancelled = false;
    std::function<bool()> shouldCancel;
    bool completed = false;
    std::exception_ptr callbackError = nullptr;
};

int streamProgress(void* user, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    auto& state = *static_cast<StreamState*>(user);
    try {
        if (state.shouldCancel && state.shouldCancel()) { state.cancelled = true; return 1; }
    } catch (...) { state.callbackError = std::current_exception(); return 1; }
    return 0;
}

size_t writeStreamContent(char* data, size_t size, size_t count, void* user) {
    auto& state = *static_cast<StreamState*>(user);
    const size_t bytes = size * count;
    state.pending.append(data, bytes);
    size_t end = 0;
    while ((end = state.pending.find('\n')) != std::string::npos) {
        std::string line = state.pending.substr(0, end);
        state.pending.erase(0, end + 1);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.compare(0, 5, "data:") != 0) continue;
        std::string payload = line.substr(5);
        if (!payload.empty() && payload.front() == ' ') payload.erase(payload.begin());
        if (payload == "[DONE]") { state.completed = true; continue; }
        const json event = json::parse(payload, nullptr, false);
        if (!event.is_object() || !event.contains("choices") || !event["choices"].is_array() || event["choices"].empty()) continue;
        if (event.contains("model") && event["model"].is_string()) state.model = event["model"].get<std::string>();
        if (!event["choices"][0].is_object()) continue;
        const auto& choice = event["choices"][0];
        if (choice.value("finish_reason", json()).is_string()) state.finishReason = choice["finish_reason"].get<std::string>();
        if (choice.value("finish_reason", json()) == "stop") state.completed = true;
        const auto delta = choice.value("delta", json::object());
        if (!delta.is_object() || !delta.value("content", json()).is_string()) continue;
        const std::string chunk = delta["content"].get<std::string>();
        if (chunk.empty()) continue;
        state.content += chunk;
        if (!state.onChunk(chunk)) { state.cancelled = true; return 0; }
    }
    return bytes;
}

size_t writeStream(char* data, size_t size, size_t count, void* user) {
    // C 回调边界不传播 C++ 异常；释放 libcurl 资源后再交给主控处理。
    try { return writeStreamContent(data, size, count, user); }
    catch (...) {
        static_cast<StreamState*>(user)->callbackError = std::current_exception();
        return 0;
    }
}

AIResult streamRequest(const Endpoint& endpoint, const ChatOptions& options, int timeoutMs,
                       const std::function<bool(const std::string&)>& onChunk) {
    if (endpoint.key.empty()) throw AIClientError("missing_config", "AI_API_KEY is not configured");
    CURL* curl = curl_easy_init();
    if (!curl) throw AIClientError("network_error", "unable to initialize curl");
    const std::string model = options.model.empty() ? endpoint.model : options.model;
    json body{{"model", model}, {"temperature", options.temperature},
              {"max_tokens", options.maxTokens}, {"stream", true}, {"messages", json::array()}};
    if (!options.responseFormat.empty()) body["response_format"] = {{"type", options.responseFormat}};
    for (const auto& message : options.messages)
        body["messages"].push_back({{"role", message.role}, {"content", message.content}});
    const std::string postBody = body.dump();
    const std::string url = chatEndpoint(endpoint.url);
    const std::string authorization = "Authorization: Bearer " + endpoint.key;
    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, authorization.c_str());
    StreamState state{{}, {}, model, {}, onChunk, false, options.cancelled};
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postBody.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(postBody.size()));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeStream);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &state);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, static_cast<long>(timeoutMs));
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, streamProgress);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &state);
    const CURLcode code = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    if (state.callbackError) std::rethrow_exception(state.callbackError);
    if (state.cancelled) throw AIClientError("cancelled", "stream cancelled");
    if (code != CURLE_OK || status < 200 || status >= 300)
        throw errorFor(code, status, code == CURLE_OK ? "AI provider returned HTTP " + std::to_string(status) : curl_easy_strerror(code));
    if (state.content.empty() || !state.completed || state.finishReason == "length") throw AIClientError("invalid_response", "AI stream is incomplete");
    return {state.content, state.model, static_cast<int>(status), "stop", "not_requested", {}};
}

ChatOptions prepareSearch(const ChatOptions& options, std::string& searchStatus, std::vector<std::string>& sources) {
    ChatOptions prepared = options;
    if (!options.searchQuery.empty()) {
        try {
            SearchClient search;
            const auto resources = search.search(options.searchQuery, 5);
            searchStatus = resources.empty() ? "unavailable" : search.lastProvider();
            std::string context = u8"联网检索只用于知识内容出处，不得用它判断学生掌握度。";
            for (const auto& resource : resources) {
                context += "\n- " + resource.title + " | " + resource.url + " | " + resource.description;
                sources.push_back(resource.url);
            }
            if (resources.empty()) context += u8"未获得联网依据；不要虚构来源。";
            prepared.messages.insert(prepared.messages.begin(), {"system", context});
        } catch (...) {
            searchStatus = "unavailable";
            prepared.messages.insert(prepared.messages.begin(), {"system", u8"未获得联网依据；不要虚构来源。"});
        }
    }
    if (options.cancelled && options.cancelled()) throw AIClientError("cancelled", "stream cancelled");
    return prepared;
}

AIResult attempt(const Endpoint& endpoint, const ChatOptions& options, int timeoutMs, int attempts) {
    AIClientError last("unknown", "AI request failed");
    for (int i = 0; i < attempts; ++i) {
        try { return request(endpoint, options, timeoutMs); }
        catch (const AIClientError& error) {
            last = error;
            const bool retryable = error.errorType == "timeout" || error.errorType == "network_error" ||
                error.errorType == "rate_limited" || error.errorType == "provider_5xx" ||
                error.errorType == "invalid_response";
            if (!retryable || i + 1 == attempts) throw;
            std::this_thread::sleep_for(std::chrono::milliseconds(800 * (1 << i)));
        }
    }
    throw last;
}
}

AIClientError::AIClientError(const std::string& type, const std::string& message, int status)
    : std::runtime_error(message), errorType(type), httpStatus(status),
      retryable(type == "timeout" || type == "network_error" || type == "rate_limited" ||
                type == "provider_5xx" || type == "invalid_response") {}

AIClient::AIClient() : AIClient(configFromEnvironment()) {}

AIClient::AIClient(AIClientConfig config)
    : baseUrl_(std::move(config.baseUrl)), apiKey_(std::move(config.apiKey)),
      model_(std::move(config.model)), timeoutMs_(config.timeoutMs),
      retryAttempts_(config.retryAttempts) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::vector<std::string> requestModels(const Endpoint& endpoint, int timeoutMs) {
    if (endpoint.key.empty()) throw AIClientError("missing_config", "AI_API_KEY is not configured");
    CURL* curl = curl_easy_init();
    if (!curl) throw AIClientError("network_error", "unable to initialize curl");
    std::string responseBody;
    struct curl_slist* headers = nullptr;
    const std::string authorization = "Authorization: Bearer " + endpoint.key;
    headers = curl_slist_append(headers, authorization.c_str());
    const std::string url = modelsEndpoint(endpoint.url);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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
        if (status == 404) throw AIClientError("models_unsupported", "provider does not expose /models");
        throw errorFor(code, status, code == CURLE_OK ? "AI provider returned HTTP " + std::to_string(status) : curl_easy_strerror(code));
    }
    try {
        const json parsed = json::parse(responseBody);
        std::vector<std::string> models;
        for (const auto& item : parsed.at("data")) {
            if (item.contains("id") && item.at("id").is_string()) {
                const auto id = item.at("id").get<std::string>();
                if (!id.empty()) models.push_back(id);
            }
        }
        std::sort(models.begin(), models.end());
        models.erase(std::unique(models.begin(), models.end()), models.end());
        if (models.empty()) throw AIClientError("invalid_response", "model list is empty");
        return models;
    } catch (const AIClientError&) {
        throw;
    } catch (const std::exception& error) {
        throw AIClientError("invalid_response", error.what());
    }
}

AIClient::~AIClient() {
    secureClear(apiKey_);
}

AIResult AIClient::chat(const ChatOptions& options) const {
    std::string searchStatus = "not_requested";
    std::vector<std::string> sources;
    const auto prepared = prepareSearch(options, searchStatus, sources);
    const int timeout = options.timeoutMs > 0 ? options.timeoutMs : timeoutMs_;
    const int attempts = options.maxAttempts > 0 ? options.maxAttempts : retryAttempts_;
    if (attempts < 1) throw AIClientError("unknown", "maxAttempts must be positive");
    if (circuitOpenedAtMs_ && nowMs() - circuitOpenedAtMs_ < 60000)
        throw AIClientError("network_error", "AI provider circuit is open");
    if (circuitOpenedAtMs_) { circuitOpenedAtMs_ = 0; consecutiveFailures_ = 0; }

    const Endpoint primary{baseUrl_, apiKey_, model_};
    try {
        AIResult result = attempt(primary, prepared, timeout, attempts);
        result.searchStatus = searchStatus;
        result.sources = std::move(sources);
        consecutiveFailures_ = 0;
        return result;
    } catch (const AIClientError& primaryError) {
        if (++consecutiveFailures_ >= 3) circuitOpenedAtMs_ = nowMs();
        throw primaryError;
    }
}

AIResult AIClient::chatStream(const ChatOptions& options,
                              const std::function<bool(const std::string&)>& onChunk) const {
    std::string searchStatus = "not_requested";
    std::vector<std::string> sources;
    const auto prepared = prepareSearch(options, searchStatus, sources);
    auto result = streamRequest({baseUrl_, apiKey_, model_}, prepared,
        options.timeoutMs > 0 ? options.timeoutMs : timeoutMs_, onChunk);
    result.searchStatus = searchStatus;
    result.sources = std::move(sources);
    return result;
}

std::vector<std::string> AIClient::listModels(int timeoutMs) const {
    const Endpoint endpoint{baseUrl_, apiKey_, model_};
    return requestModels(endpoint, timeoutMs > 0 ? timeoutMs : timeoutMs_);
}
}

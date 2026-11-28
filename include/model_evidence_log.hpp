#pragma once

#include "ai_client.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <set>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace gangyi {

// 仅由隔离实验显式启用；活动面板仍不保存提示词或模型正文。
class ModelEvidenceLog {
    using Json = nlohmann::json;
    std::filesystem::path path_;
    std::string id_;

    static std::mutex& mutex() { static std::mutex value; return value; }
    static std::set<std::filesystem::path>& failedPaths() { static std::set<std::filesystem::path> value; return value; }
    static unsigned long processId() {
#ifdef _WIN32
        return GetCurrentProcessId();
#else
        return static_cast<unsigned long>(getpid());
#endif
    }
    static std::filesystem::path environmentDirectory() {
#ifdef _WIN32
        const auto count = GetEnvironmentVariableW(L"GANGYI_MODEL_EVIDENCE_DIR", nullptr, 0);
        if (count == 0) return {};
        std::wstring value(count, L'\0');
        const auto length = GetEnvironmentVariableW(L"GANGYI_MODEL_EVIDENCE_DIR", value.data(), count);
        if (length == 0 || length >= count) throw AIClientError("model_evidence", "模型记录目录无效，未发送请求");
        value.resize(length); return std::filesystem::path(value);
#else
        const auto* value = std::getenv("GANGYI_MODEL_EVIDENCE_DIR");
        return value && *value ? std::filesystem::path(value) : std::filesystem::path();
#endif
    }
    void append(Json record) {
        if (path_.empty()) return;
        std::lock_guard<std::mutex> lock(mutex());
        if (failedPaths().count(path_)) throw AIClientError("model_evidence", "原始模型记录曾保存失败，已停止后续请求");
        try {
            record["schemaVersion"] = 1; record["modelRequestId"] = id_;
            record["atUnixMs"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            std::filesystem::create_directories(path_.parent_path());
            std::ofstream stream(path_, std::ios::binary | std::ios::app);
            const auto encoded = record.dump() + '\n';
            stream.write(encoded.data(), static_cast<std::streamsize>(encoded.size())); stream.flush();
            if (!stream) throw std::runtime_error("write");
        } catch (...) {
            failedPaths().insert(path_);
            throw AIClientError("model_evidence", "原始模型记录无法保存，已停止后续请求");
        }
    }

public:
    explicit ModelEvidenceLog(const ChatOptions& options, const std::filesystem::path& directory = environmentDirectory()) {
        if (directory.empty()) return;
        if (!directory.is_absolute()) throw AIClientError("model_evidence", "模型记录须使用隔离的绝对目录，未发送请求");
        path_ = directory / ("model-" + std::to_string(processId()) + ".jsonl");
        static std::atomic<unsigned long long> sequence{0};
        id_ = std::to_string(processId()) + "-" + std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count()) + "-" + std::to_string(++sequence);
        Json messages = Json::array(); bool textOnly = true;
        for (const auto& message : options.messages) {
            const bool hasImage = !message.imageDataUrl.empty(); textOnly = textOnly && !hasImage;
            messages.push_back({{"role",message.role},{"content",message.content},{"hasImage",hasImage}});
        }
        append({{"kind","request"},{"taskId",options.activity.taskId},{"courseId",options.activity.courseId},
            {"lessonId",options.activity.lessonId},{"scopeId",options.activity.scopeId},{"purpose",options.activity.purpose},
            {"requestModel",options.model},{"maxTokens",options.maxTokens},{"temperature",options.temperature},
            {"timeoutMs",options.timeoutMs},{"responseFormat",options.responseFormat},{"textOnly",textOnly},{"messages",messages}});
    }
    void chunk(const std::string& text) { if (!text.empty()) append({{"kind","chunk"},{"text",text}}); }
    void completed(const AIResult& result) {
        append({{"kind","completed"},{"rawResponse",result.content},{"model",result.model},
            {"finishReason",result.finishReason},{"httpStatus",result.status},{"usage",result.tokenUsage}});
    }
    void failed() noexcept {
        try { append({{"kind","failed"},{"completed",false},{"partialMayExist",true}}); }
        catch (...) {}
    }
};

} // namespace gangyi

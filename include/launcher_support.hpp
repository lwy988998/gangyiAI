#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace gangyi::launcher {

enum class AIProvider { DeepSeek, OpenAI, Custom };

struct ProviderProfile {
    AIProvider provider;
    const wchar_t* id;
    const wchar_t* name;
    const wchar_t* baseUrl;
    const wchar_t* recommendedModel;
};

const ProviderProfile& providerProfile(AIProvider provider);
AIProvider providerFromId(const std::wstring& id);
AIProvider inferProvider(const std::wstring& baseUrl);
std::wstring sanitizeBaseUrl(const std::wstring& baseUrl);

class RestartPolicy {
public:
    std::optional<unsigned> recordFailure();
    void reset();
    int failureCount() const;

private:
    int failures_ = 0;
};

bool rotateLogFiles(const std::filesystem::path& activeLog, std::uintmax_t maxBytes = 5u * 1024u * 1024u,
                    int retainedFiles = 3);
bool validateDatabase(const std::filesystem::path& path, std::wstring& error);
bool backupDatabase(const std::filesystem::path& source, const std::filesystem::path& destination,
                    std::wstring& error);
bool restoreDatabase(const std::filesystem::path& backup, const std::filesystem::path& destination,
                     const std::filesystem::path& rollback, std::wstring& error);

struct DiagnosticInfo {
    std::wstring version;
    std::filesystem::path installDir;
    std::filesystem::path dataDir;
    std::wstring provider;
    std::wstring baseUrl;
    std::wstring model;
    bool serviceRunning = false;
    int port = 0;
    unsigned long exitCode = 0;
    int restartFailures = 0;
};

bool writeDiagnosticReport(const std::filesystem::path& destination, const DiagnosticInfo& info,
                           std::wstring& error);

}  // namespace gangyi::launcher

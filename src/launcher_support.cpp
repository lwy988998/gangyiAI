#include "launcher_support.hpp"
#include "db_schema_version.hpp"

#include <sqlite3.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <algorithm>
#include <chrono>
#include <cwctype>
#include <fstream>
#include <sstream>
#include <vector>

namespace gangyi::launcher {
namespace {

constexpr ProviderProfile kProfiles[] = {
    {AIProvider::DeepSeek, L"deepseek", L"DeepSeek", L"https://api.deepseek.com", L"deepseek-v4-flash"},
    {AIProvider::OpenAI, L"openai", L"OpenAI", L"https://api.openai.com/v1", L"gpt-5.2"},
    {AIProvider::Custom, L"custom", L"自定义", L"", L""},
};

void setError(std::wstring& error, const wchar_t* message) {
    error = message;
}

bool runQuickCheck(sqlite3* database, std::wstring& error) {
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(database, "PRAGMA quick_check", -1, &statement, nullptr) != SQLITE_OK) {
        setError(error, L"无法检查数据库完整性。");
        return false;
    }
    const int step = sqlite3_step(statement);
    const unsigned char* result = step == SQLITE_ROW ? sqlite3_column_text(statement, 0) : nullptr;
    const bool ok = result && std::string(reinterpret_cast<const char*>(result)) == "ok";
    sqlite3_finalize(statement);
    if (!ok) setError(error, L"数据库完整性检查未通过。");
    return ok;
}

bool hasRequiredTables(sqlite3* database, std::wstring& error) {
    constexpr const char* sql =
        "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name IN "
        "('Course','CourseSnapshot','CourseProgress','User','UserSession')";
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK) {
        setError(error, L"无法读取数据库结构。");
        return false;
    }
    const bool ok = sqlite3_step(statement) == SQLITE_ROW && sqlite3_column_int(statement, 0) == 5;
    sqlite3_finalize(statement);
    if (!ok) setError(error, L"所选文件不是有效的钢一定制AI课程数据库。");
    return ok;
}

bool hasCompatibleSchemaVersion(sqlite3* database, std::wstring& error) {
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(database, "PRAGMA user_version", -1, &statement, nullptr) != SQLITE_OK) {
        setError(error, L"无法读取数据库版本。");
        return false;
    }
    const bool hasVersion = sqlite3_step(statement) == SQLITE_ROW;
    const int version = hasVersion ? sqlite3_column_int(statement, 0) : -1;
    sqlite3_finalize(statement);
    if (!hasVersion || version < 0 || version > gangyi::kDatabaseSchemaVersion) {
        setError(error, L"数据库来自不兼容的新版本，无法恢复。");
        return false;
    }
    return true;
}

bool openDatabase(const std::filesystem::path& path, int flags, sqlite3** database, std::wstring& error) {
    const std::string utf8 = path.u8string();
    if (sqlite3_open_v2(utf8.c_str(), database, flags, nullptr) == SQLITE_OK) return true;
    if (*database) sqlite3_close(*database);
    *database = nullptr;
    setError(error, L"无法打开数据库文件。");
    return false;
}

bool copyDatabase(const std::filesystem::path& source, const std::filesystem::path& destination,
                  std::wstring& error) {
    sqlite3* sourceDb = nullptr;
    sqlite3* destinationDb = nullptr;
    if (!openDatabase(source, SQLITE_OPEN_READONLY, &sourceDb, error)) return false;
    std::error_code filesystemError;
    if (!destination.parent_path().empty()) {
        std::filesystem::create_directories(destination.parent_path(), filesystemError);
        if (filesystemError) {
            sqlite3_close(sourceDb);
            setError(error, L"无法创建数据库目标目录。");
            return false;
        }
    }
    std::filesystem::remove(destination, filesystemError);
    if (!openDatabase(destination, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, &destinationDb, error)) {
        sqlite3_close(sourceDb);
        return false;
    }
    sqlite3_busy_timeout(sourceDb, 5000);
    sqlite3_busy_timeout(destinationDb, 5000);
    sqlite3_backup* backup = sqlite3_backup_init(destinationDb, "main", sourceDb, "main");
    bool ok = backup != nullptr;
    if (backup) {
        int result = SQLITE_OK;
        do {
            result = sqlite3_backup_step(backup, 128);
            if (result == SQLITE_BUSY || result == SQLITE_LOCKED) sqlite3_sleep(50);
        } while (result == SQLITE_OK || result == SQLITE_BUSY || result == SQLITE_LOCKED);
        ok = result == SQLITE_DONE && sqlite3_backup_finish(backup) == SQLITE_OK;
    }
    if (ok) ok = runQuickCheck(destinationDb, error);
    sqlite3_close(destinationDb);
    sqlite3_close(sourceDb);
    if (!ok) {
        std::filesystem::remove(destination, filesystemError);
        if (error.empty()) setError(error, L"数据库复制失败，请检查磁盘空间和文件权限。");
    }
    return ok;
}

bool replaceFile(const std::filesystem::path& source, const std::filesystem::path& destination) {
#ifdef _WIN32
    return MoveFileExW(source.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == TRUE;
#else
    std::error_code error;
    std::filesystem::rename(source, destination, error);
    return !error;
#endif
}

std::string toUtf8(const std::wstring& value) {
#ifdef _WIN32
    if (value.empty()) return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        nullptr, 0, nullptr, nullptr);
    std::string output(static_cast<size_t>(std::max(size, 0)), '\0');
    if (size > 0) WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        output.data(), size, nullptr, nullptr);
    return output;
#else
    return std::string(value.begin(), value.end());
#endif
}

std::wstring boolText(bool value) {
    return value ? L"是" : L"否";
}

}  // namespace

const ProviderProfile& providerProfile(AIProvider provider) {
    for (const auto& profile : kProfiles) if (profile.provider == provider) return profile;
    return kProfiles[2];
}

AIProvider providerFromId(const std::wstring& id) {
    for (const auto& profile : kProfiles) if (id == profile.id) return profile.provider;
    return AIProvider::Custom;
}

AIProvider inferProvider(const std::wstring& baseUrl) {
    std::wstring lower = baseUrl;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](wchar_t value) { return std::towlower(value); });
    if (lower.find(L"api.deepseek.com") != std::wstring::npos) return AIProvider::DeepSeek;
    if (lower.find(L"api.openai.com") != std::wstring::npos) return AIProvider::OpenAI;
    return AIProvider::Custom;
}

std::wstring sanitizeBaseUrl(const std::wstring& baseUrl) {
    const size_t scheme = baseUrl.find(L"://");
    if (scheme == std::wstring::npos) return L"无效地址";
    const size_t authorityStart = scheme + 3;
    size_t authorityEnd = baseUrl.find_first_of(L"/?#", authorityStart);
    if (authorityEnd == std::wstring::npos) authorityEnd = baseUrl.size();
    std::wstring authority = baseUrl.substr(authorityStart, authorityEnd - authorityStart);
    const size_t at = authority.rfind(L'@');
    if (at != std::wstring::npos) authority.erase(0, at + 1);
    return baseUrl.substr(0, scheme + 3) + authority;
}

std::optional<unsigned> RestartPolicy::recordFailure() {
    constexpr std::array<unsigned, 3> delays{2, 5, 15};
    if (failures_ >= static_cast<int>(delays.size())) return std::nullopt;
    return delays[static_cast<size_t>(failures_++)];
}

void RestartPolicy::reset() { failures_ = 0; }
int RestartPolicy::failureCount() const { return failures_; }

bool rotateLogFiles(const std::filesystem::path& activeLog, std::uintmax_t maxBytes, int retainedFiles) {
    std::error_code error;
    if (!std::filesystem::exists(activeLog, error) || std::filesystem::file_size(activeLog, error) < maxBytes)
        return !error;
    for (int index = retainedFiles; index >= 1; --index) {
        const auto current = std::filesystem::path(activeLog.wstring() + L"." + std::to_wstring(index));
        if (index == retainedFiles) std::filesystem::remove(current, error);
        if (index > 1) {
            const auto previous = std::filesystem::path(activeLog.wstring() + L"." + std::to_wstring(index - 1));
            if (std::filesystem::exists(previous, error)) std::filesystem::rename(previous, current, error);
        }
    }
    const auto first = std::filesystem::path(activeLog.wstring() + L".1");
    std::filesystem::rename(activeLog, first, error);
    return !error;
}

bool validateDatabase(const std::filesystem::path& path, std::wstring& error) {
    error.clear();
    sqlite3* database = nullptr;
    if (!std::filesystem::is_regular_file(path)) {
        setError(error, L"数据库文件不存在或不是普通文件。");
        return false;
    }
    if (!openDatabase(path, SQLITE_OPEN_READONLY, &database, error)) return false;
    const bool ok = runQuickCheck(database, error) && hasRequiredTables(database, error) &&
        hasCompatibleSchemaVersion(database, error);
    sqlite3_close(database);
    return ok;
}

bool backupDatabase(const std::filesystem::path& source, const std::filesystem::path& destination,
                    std::wstring& error) {
    error.clear();
    std::error_code equivalentError;
    if (std::filesystem::exists(destination) && std::filesystem::equivalent(source, destination, equivalentError)) {
        setError(error, L"备份目标不能是当前课程数据库。");
        return false;
    }
    if (!validateDatabase(source, error)) return false;
    const auto temporary = std::filesystem::path(destination.wstring() + L".tmp");
    if (!copyDatabase(source, temporary, error)) return false;
    std::error_code filesystemError;
    if (!replaceFile(temporary, destination)) {
        std::filesystem::remove(temporary, filesystemError);
        setError(error, L"无法写入目标备份文件，请检查路径和权限。");
        return false;
    }
    return true;
}

bool restoreDatabase(const std::filesystem::path& backup, const std::filesystem::path& destination,
                     const std::filesystem::path& rollback, std::wstring& error) {
    error.clear();
    std::error_code equivalentError;
    if (std::filesystem::exists(destination) && std::filesystem::equivalent(backup, destination, equivalentError)) {
        setError(error, L"请选择与当前课程数据库不同的备份文件。");
        return false;
    }
    if (!validateDatabase(backup, error)) return false;
    if (std::filesystem::exists(destination) && !backupDatabase(destination, rollback, error)) return false;
    const auto temporary = std::filesystem::path(destination.wstring() + L".restore.tmp");
    if (!copyDatabase(backup, temporary, error)) return false;
    std::error_code filesystemError;
    if (!replaceFile(temporary, destination)) {
        if (std::filesystem::exists(rollback)) copyDatabase(rollback, destination, error);
        setError(error, L"无法替换课程数据库，已保留恢复前备份。");
        return false;
    }
    return true;
}

bool writeDiagnosticReport(const std::filesystem::path& destination, const DiagnosticInfo& info,
                           std::wstring& error) {
    error.clear();
    std::wostringstream report;
    report << L"钢一定制AI 诊断报告\r\n"
           << L"版本: " << info.version << L"\r\n"
           << L"系统: Windows " << (sizeof(void*) == 8 ? L"x64" : L"x86") << L"\r\n"
           << L"安装目录: " << info.installDir.wstring() << L"\r\n"
           << L"数据目录: " << info.dataDir.wstring() << L"\r\n"
           << L"服务商: " << info.provider << L"\r\n"
           << L"接口主机: " << sanitizeBaseUrl(info.baseUrl) << L"\r\n"
           << L"模型: " << info.model << L"\r\n"
           << L"服务运行中: " << boolText(info.serviceRunning) << L"\r\n"
           << L"端口: " << info.port << L"\r\n"
           << L"最近退出码: " << info.exitCode << L"\r\n"
           << L"连续重启次数: " << info.restartFailures << L"\r\n";
    for (const auto* name : {L"gangyiAI.exe", L"gangyiAI-launcher.exe", L"libcurl-x64.dll",
                              L"public\\school-logo.png", L"public\\styles.css"}) {
        const auto path = info.installDir / name;
        std::error_code filesystemError;
        report << L"文件 " << name << L": " << (std::filesystem::is_regular_file(path, filesystemError) ? L"存在" : L"缺失");
        if (!filesystemError && std::filesystem::is_regular_file(path)) report << L" (" << std::filesystem::file_size(path, filesystemError) << L" 字节)";
        report << L"\r\n";
    }
    std::ofstream output(destination, std::ios::binary | std::ios::trunc);
    if (!output) {
        setError(error, L"无法创建诊断报告文件。");
        return false;
    }
    const std::string utf8 = toUtf8(report.str());
    output.write("\xEF\xBB\xBF", 3);
    output.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
    if (!output.good()) {
        setError(error, L"写入诊断报告失败。");
        return false;
    }
    return true;
}

}  // namespace gangyi::launcher

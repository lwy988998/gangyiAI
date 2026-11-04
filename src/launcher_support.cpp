// 【逐行批注】 引入头文件 "launcher_support.hpp"，为后续实现提供类型或接口声明。
#include "launcher_support.hpp"
// 【逐行批注】 引入头文件 "db_schema_version.hpp"，为后续实现提供类型或接口声明。
#include "db_schema_version.hpp"

// 【逐行批注】 引入头文件 <sqlite3.h>，为后续实现提供类型或接口声明。
#include <sqlite3.h>

// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
// 【逐行批注】 处理条件编译指令：#ifndef NOMINMAX。
#ifndef NOMINMAX
// 【逐行批注】 定义编译期常量或宏：#define NOMINMAX。
#define NOMINMAX
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 <windows.h>，为后续实现提供类型或接口声明。
#include <windows.h>
// 【逐行批注】 处理条件编译指令：#else。
#else
// 【逐行批注】 引入头文件 <fcntl.h>，为后续实现提供类型或接口声明。
#include <fcntl.h>
// 【逐行批注】 引入头文件 <unistd.h>，为后续实现提供类型或接口声明。
#include <unistd.h>
// 【逐行批注】 处理条件编译指令：#endif。
#endif

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <atomic>，为后续实现提供类型或接口声明。
#include <atomic>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <cwctype>，为后续实现提供类型或接口声明。
#include <cwctype>
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi::launcher {。
namespace gangyi::launcher {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 constexpr ProviderProfile kProfiles[]，更新当前运行状态。
constexpr ProviderProfile kProfiles[] = {
    // 【逐行批注】 执行当前声明或表达式：{AIProvider::DeepSeek, L"deepseek", L"DeepSeek", L"https://api.deepseek.com", L"deep…。
    {AIProvider::DeepSeek, L"deepseek", L"DeepSeek", L"https://api.deepseek.com", L"deepseek-v4-flash"},
    // 【逐行批注】 执行当前声明或表达式：{AIProvider::OpenAI, L"openai", L"OpenAI", L"https://api.openai.com/v1", L"gpt-5.2"},。
    {AIProvider::OpenAI, L"openai", L"OpenAI", L"https://api.openai.com/v1", L"gpt-5.2"},
    // 【逐行批注】 执行当前声明或表达式：{AIProvider::Custom, L"custom", L"自定义", L"", L""},。
    {AIProvider::Custom, L"custom", L"自定义", L"", L""},
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 setError，执行该函数声明的职责。
void setError(std::wstring& error, const wchar_t* message) {
    // 【逐行批注】 计算右侧结果并写入 error，更新当前运行状态。
    error = message;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 runQuickCheck，执行该函数声明的职责。
bool runQuickCheck(sqlite3* database, std::wstring& error) {
    // 【逐行批注】 计算右侧结果并写入 sqlite3_stmt* statement，更新当前运行状态。
    sqlite3_stmt* statement = nullptr;
    // 【逐行批注】 判断条件 (sqlite3_prepare_v2(database, "PRAGMA quick_check", -1, &stateme…，成立时执行对应分支。
    if (sqlite3_prepare_v2(database, "PRAGMA quick_check", -1, &statement, nullptr) != SQLITE_OK) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"无法检查数据库完整性。");。
        setError(error, L"无法检查数据库完整性。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 int step，更新当前运行状态。
    const int step = sqlite3_step(statement);
    // 【逐行批注】 计算右侧结果并写入 unsigned char* result，更新当前运行状态。
    const unsigned char* result = step == SQLITE_ROW ? sqlite3_column_text(statement, 0) : nullptr;
    // 【逐行批注】 计算右侧结果并写入 bool ok，更新当前运行状态。
    const bool ok = result && std::string(reinterpret_cast<const char*>(result)) == "ok";
    // 【逐行批注】 调用 sqlite3_finalize 完成本行所需操作：sqlite3_finalize(statement);。
    sqlite3_finalize(statement);
    // 【逐行批注】 判断条件 (!ok) setError(error, L"数据库完整性检查未通过。");，成立时执行对应分支。
    if (!ok) setError(error, L"数据库完整性检查未通过。");
    // 【逐行批注】 结束当前函数并返回结果：return ok;。
    return ok;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 hasRequiredTables，执行该函数声明的职责。
bool hasRequiredTables(sqlite3* database, std::wstring& error) {
    // 【逐行批注】 计算右侧结果并写入 constexpr const char* sql，更新当前运行状态。
    constexpr const char* sql =
        // 【逐行批注】 计算右侧结果并写入 "SELECT COUNT(*) FROM sqlite_master WHERE ty…，更新当前运行状态。
        "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name IN "
        // 【逐行批注】 执行当前声明或表达式："('Course','CourseSnapshot','CourseProgress','User','UserSession')";。
        "('Course','CourseSnapshot','CourseProgress','User','UserSession')";
    // 【逐行批注】 计算右侧结果并写入 sqlite3_stmt* statement，更新当前运行状态。
    sqlite3_stmt* statement = nullptr;
    // 【逐行批注】 判断条件 (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != S…，成立时执行对应分支。
    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"无法读取数据库结构。");。
        setError(error, L"无法读取数据库结构。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 bool ok，更新当前运行状态。
    const bool ok = sqlite3_step(statement) == SQLITE_ROW && sqlite3_column_int(statement, 0) == 5;
    // 【逐行批注】 调用 sqlite3_finalize 完成本行所需操作：sqlite3_finalize(statement);。
    sqlite3_finalize(statement);
    // 【逐行批注】 判断条件 (!ok) setError(error, L"所选文件不是有效的钢一定制AI课程数据库。");，成立时执行对应分支。
    if (!ok) setError(error, L"所选文件不是有效的钢一定制AI课程数据库。");
    // 【逐行批注】 结束当前函数并返回结果：return ok;。
    return ok;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 hasCompatibleSchemaVersion，执行该函数声明的职责。
bool hasCompatibleSchemaVersion(sqlite3* database, std::wstring& error) {
    // 【逐行批注】 计算右侧结果并写入 sqlite3_stmt* statement，更新当前运行状态。
    sqlite3_stmt* statement = nullptr;
    // 【逐行批注】 判断条件 (sqlite3_prepare_v2(database, "PRAGMA user_version", -1, &statem…，成立时执行对应分支。
    if (sqlite3_prepare_v2(database, "PRAGMA user_version", -1, &statement, nullptr) != SQLITE_OK) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"无法读取数据库版本。");。
        setError(error, L"无法读取数据库版本。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 bool hasVersion，更新当前运行状态。
    const bool hasVersion = sqlite3_step(statement) == SQLITE_ROW;
    // 【逐行批注】 计算右侧结果并写入 int version，更新当前运行状态。
    const int version = hasVersion ? sqlite3_column_int(statement, 0) : -1;
    // 【逐行批注】 调用 sqlite3_finalize 完成本行所需操作：sqlite3_finalize(statement);。
    sqlite3_finalize(statement);
    // 【逐行批注】 判断条件 (!hasVersion || version < 0 || version > gangyi::kDatabaseSchema…，成立时执行对应分支。
    if (!hasVersion || version < 0 || version > gangyi::kDatabaseSchemaVersion) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"数据库来自不兼容的新版本，无法恢复。");。
        setError(error, L"数据库来自不兼容的新版本，无法恢复。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 openDatabase，执行该函数声明的职责。
bool openDatabase(const std::filesystem::path& path, int flags, sqlite3** database, std::wstring& error) {
    // 【逐行批注】 计算右侧结果并写入 std::string utf8，更新当前运行状态。
    const std::string utf8 = path.u8string();
    // 【逐行批注】 判断条件 (sqlite3_open_v2(utf8.c_str(), database, flags, nullptr) == SQLI…，成立时执行对应分支。
    if (sqlite3_open_v2(utf8.c_str(), database, flags, nullptr) == SQLITE_OK) return true;
    // 【逐行批注】 判断条件 (*database) sqlite3_close(*database);，成立时执行对应分支。
    if (*database) sqlite3_close(*database);
    *database = nullptr;
    // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"无法打开数据库文件。");。
    setError(error, L"无法打开数据库文件。");
    // 【逐行批注】 结束当前函数并返回结果：return false;。
    return false;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明数据结构或类型 DatabaseHandle 及其成员接口。
struct DatabaseHandle {
    // 【逐行批注】 计算右侧结果并写入 sqlite3* value，更新当前运行状态。
    sqlite3* value = nullptr;
    // 【逐行批注】 计算右侧结果并写入 DatabaseHandle()，更新当前运行状态。
    DatabaseHandle() = default;
    // 【逐行批注】 计算右侧结果并写入 DatabaseHandle(const DatabaseHandle&)，更新当前运行状态。
    DatabaseHandle(const DatabaseHandle&) = delete;
    // 【逐行批注】 计算右侧结果并写入 DatabaseHandle& operator，更新当前运行状态。
    DatabaseHandle& operator=(const DatabaseHandle&) = delete;
    // 【逐行批注】 调用 DatabaseHandle、sqlite3_close 完成本行所需操作：~DatabaseHandle() { if (value) sqlite3_close(value); }。
    ~DatabaseHandle() { if (value) sqlite3_close(value); }
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 OwnedDatabaseFile 及其成员接口。
struct OwnedDatabaseFile {
    // 【逐行批注】 执行当前声明或表达式：std::filesystem::path path;。
    std::filesystem::path path;
    // 【逐行批注】 计算右侧结果并写入 bool removeOnExit，更新当前运行状态。
    bool removeOnExit = false;
    // 【逐行批注】 调用 OwnedDatabaseFile 完成本行所需操作：~OwnedDatabaseFile() {。
    ~OwnedDatabaseFile() {
        // 【逐行批注】 判断条件 (!removeOnExit) return;，成立时执行对应分支。
        if (!removeOnExit) return;
        // 【逐行批注】 执行当前声明或表达式：std::error_code ignored;。
        std::error_code ignored;
        // 仅清理本次新建且已经关闭的临时数据库，绝不删除原档案的附属文件。
        // 【逐行批注】 按循环规则重复处理：for (const auto* suffix : {L"", L"-wal", L"-shm", L"-journal"})。
        for (const auto* suffix : {L"", L"-wal", L"-shm", L"-journal"})
            // 【逐行批注】 调用 remove、path 完成本行所需操作：std::filesystem::remove(std::filesystem::path(path.wstring() + suffix…。
            std::filesystem::remove(std::filesystem::path(path.wstring() + suffix), ignored);
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 temporaryDatabase，执行该函数声明的职责。
std::filesystem::path temporaryDatabase(const std::filesystem::path& destination) {
    // 【逐行批注】 执行当前声明或表达式：static std::atomic_uint64_t sequence{0};。
    static std::atomic_uint64_t sequence{0};
    // 【逐行批注】 结束当前函数并返回结果：return std::filesystem::path(destination.wstring() + L".tmp-" +。
    return std::filesystem::path(destination.wstring() + L".tmp-" +
        // 【逐行批注】 调用 to_wstring、now 完成本行所需操作：std::to_wstring(std::chrono::steady_clock::now().time_since_epoch().c…。
        std::to_wstring(std::chrono::steady_clock::now().time_since_epoch().count()) + L"-" +
        // 【逐行批注】 调用 to_wstring 完成本行所需操作：std::to_wstring(++sequence));。
        std::to_wstring(++sequence));
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 reserveDatabaseFile，执行该函数声明的职责。
bool reserveDatabaseFile(const std::filesystem::path& path, std::wstring& error) {
    // 原子占用新路径，失败时不能清理另一个进程刚创建的档案。
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 计算右侧结果并写入 auto file，更新当前运行状态。
    const auto file = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE,
        // 【逐行批注】 执行当前声明或表达式：FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, null…。
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    // 【逐行批注】 判断条件 (file != INVALID_HANDLE_VALUE) { CloseHandle(file); return true;…，成立时执行对应分支。
    if (file != INVALID_HANDLE_VALUE) { CloseHandle(file); return true; }
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 计算右侧结果并写入 int file，更新当前运行状态。
    const int file = open(path.c_str(), O_CREAT | O_EXCL | O_RDWR, 0600);
    // 【逐行批注】 判断条件 (file >= 0) { close(file); return true; }，成立时执行对应分支。
    if (file >= 0) { close(file); return true; }
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"新快照路径已被占用或无法创建，请选择新的备份位置。");。
    setError(error, L"新快照路径已被占用或无法创建，请选择新的备份位置。");
    // 【逐行批注】 结束当前函数并返回结果：return false;。
    return false;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 copyDatabaseContent，执行该函数声明的职责。
bool copyDatabaseContent(sqlite3* sourceDb, sqlite3* destinationDb, std::wstring& error) {
    // 【逐行批注】 计算右侧结果并写入 sqlite3_backup* backup，更新当前运行状态。
    sqlite3_backup* backup = sqlite3_backup_init(destinationDb, "main", sourceDb, "main");
    // 【逐行批注】 判断条件 (!backup) {，成立时执行对应分支。
    if (!backup) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"无法创建数据库快照，原档案没有替换。");。
        setError(error, L"无法创建数据库快照，原档案没有替换。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto deadline，更新当前运行状态。
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    // 【逐行批注】 执行当前声明或表达式：int result;。
    int result;
    // 【逐行批注】 执行当前声明或表达式：do {。
    do {
        // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
        result = sqlite3_backup_step(backup, 128);
        // 【逐行批注】 判断条件 (result == SQLITE_BUSY || result == SQLITE_LOCKED) {，成立时执行对应分支。
        if (result == SQLITE_BUSY || result == SQLITE_LOCKED) {
            // 【逐行批注】 判断条件 (std::chrono::steady_clock::now() >= deadline) break;，成立时执行对应分支。
            if (std::chrono::steady_clock::now() >= deadline) break;
            // 【逐行批注】 调用 sqlite3_sleep 完成本行所需操作：sqlite3_sleep(50);。
            sqlite3_sleep(50);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：} while (result == SQLITE_OK || result == SQLITE_BUSY || result == SQLITE_LOCKED);。
    } while (result == SQLITE_OK || result == SQLITE_BUSY || result == SQLITE_LOCKED);
    // 无论成功失败均结束备份；失败的写事务由 SQLite 回滚，不能遗留活动备份句柄。
    // 【逐行批注】 计算右侧结果并写入 int finish，更新当前运行状态。
    const int finish = sqlite3_backup_finish(backup);
    // 【逐行批注】 判断条件 (result != SQLITE_DONE || finish != SQLITE_OK) {，成立时执行对应分支。
    if (result != SQLITE_DONE || finish != SQLITE_OK) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"数据库复制失败或正在使用，请关闭其他连接后重试；原档案未替换。");。
        setError(error, L"数据库复制失败或正在使用，请关闭其他连接后重试；原档案未替换。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return runQuickCheck(destinationDb, error);。
    return runQuickCheck(destinationDb, error);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 copyOpenDatabase，执行该函数声明的职责。
bool copyOpenDatabase(sqlite3* sourceDb, const std::filesystem::path& destination, std::wstring& error) {
    // 【逐行批注】 执行当前声明或表达式：std::error_code filesystemError;。
    std::error_code filesystemError;
    // 【逐行批注】 判断条件 (std::filesystem::exists(destination, filesystemError) || filesy…，成立时执行对应分支。
    if (std::filesystem::exists(destination, filesystemError) || filesystemError) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"快照文件已经存在或无法检查，请选择新的备份位置。");。
        setError(error, L"快照文件已经存在或无法检查，请选择新的备份位置。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!destination.parent_path().empty()) {，成立时执行对应分支。
    if (!destination.parent_path().empty()) {
        // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(destination.parent_path(), filesy…。
        std::filesystem::create_directories(destination.parent_path(), filesystemError);
        // 【逐行批注】 判断条件 (filesystemError) {，成立时执行对应分支。
        if (filesystemError) {
            // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"无法创建数据库目标目录。");。
            setError(error, L"无法创建数据库目标目录。");
            // 【逐行批注】 结束当前函数并返回结果：return false;。
            return false;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!reserveDatabaseFile(destination, error)) return false;，成立时执行对应分支。
    if (!reserveDatabaseFile(destination, error)) return false;
    // 【逐行批注】 执行当前声明或表达式：OwnedDatabaseFile created{destination, true};。
    OwnedDatabaseFile created{destination, true};
    // 【逐行批注】 执行当前声明或表达式：DatabaseHandle destinationDb;。
    DatabaseHandle destinationDb;
    // 【逐行批注】 判断条件 (!openDatabase(destination, SQLITE_OPEN_READWRITE, &destinationD…，成立时执行对应分支。
    if (!openDatabase(destination, SQLITE_OPEN_READWRITE, &destinationDb.value, error)) return false;
    // 【逐行批注】 调用 sqlite3_busy_timeout 完成本行所需操作：sqlite3_busy_timeout(sourceDb, 5000);。
    sqlite3_busy_timeout(sourceDb, 5000);
    // 【逐行批注】 调用 sqlite3_busy_timeout 完成本行所需操作：sqlite3_busy_timeout(destinationDb.value, 5000);。
    sqlite3_busy_timeout(destinationDb.value, 5000);
    // 【逐行批注】 计算右侧结果并写入 bool ok，更新当前运行状态。
    const bool ok = copyDatabaseContent(sourceDb, destinationDb.value, error);
    // 【逐行批注】 计算右侧结果并写入 created.removeOnExit，更新当前运行状态。
    created.removeOnExit = !ok;
    // 【逐行批注】 结束当前函数并返回结果：return ok;。
    return ok;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 copyDatabase，并通过构造函数初始化其状态。
bool copyDatabase(const std::filesystem::path& source, const std::filesystem::path& destination,
                  // 【逐行批注】 执行当前声明或表达式：std::wstring& error) {。
                  std::wstring& error) {
    // 【逐行批注】 执行当前声明或表达式：DatabaseHandle sourceDb;。
    DatabaseHandle sourceDb;
    // 【逐行批注】 结束当前函数并返回结果：return openDatabase(source, SQLITE_OPEN_READONLY, &sourceDb.value, er…。
    return openDatabase(source, SQLITE_OPEN_READONLY, &sourceDb.value, error) &&
        // 【逐行批注】 调用 copyOpenDatabase 完成本行所需操作：copyOpenDatabase(sourceDb.value, destination, error);。
        copyOpenDatabase(sourceDb.value, destination, error);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 sameDatabasePath，执行该函数声明的职责。
bool sameDatabasePath(const std::filesystem::path& left, const std::filesystem::path& right) {
    // 【逐行批注】 执行当前声明或表达式：std::error_code error;。
    std::error_code error;
    // 【逐行批注】 判断条件 (std::filesystem::equivalent(left, right, error)) return true;，成立时执行对应分支。
    if (std::filesystem::equivalent(left, right, error)) return true;
    // 【逐行批注】 计算右侧结果并写入 error.clear(); const auto first，更新当前运行状态。
    error.clear(); const auto first = std::filesystem::weakly_canonical(left, error);
    // 【逐行批注】 判断条件 (error) return false;，成立时执行对应分支。
    if (error) return false;
    // 【逐行批注】 计算右侧结果并写入 auto second，更新当前运行状态。
    const auto second = std::filesystem::weakly_canonical(right, error);
    // 【逐行批注】 结束当前函数并返回结果：return !error && first == second;。
    return !error && first == second;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 installDatabaseSnapshot，并通过构造函数初始化其状态。
bool installDatabaseSnapshot(const std::filesystem::path& snapshot, const std::filesystem::path& destination,
                             // 【逐行批注】 执行当前声明或表达式：const std::filesystem::path& rollback, std::wstring& error) {。
                             const std::filesystem::path& rollback, std::wstring& error) {
    // 【逐行批注】 执行当前声明或表达式：std::error_code filesystemError;。
    std::error_code filesystemError;
    // 【逐行批注】 计算右侧结果并写入 bool existed，更新当前运行状态。
    const bool existed = std::filesystem::exists(destination, filesystemError);
    // 【逐行批注】 判断条件 (filesystemError) { setError(error, L"无法检查目标数据库。"); return false…，成立时执行对应分支。
    if (filesystemError) { setError(error, L"无法检查目标数据库。"); return false; }
    // 【逐行批注】 判断条件 (!destination.parent_path().empty()) {，成立时执行对应分支。
    if (!destination.parent_path().empty()) {
        // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(destination.parent_path(), filesy…。
        std::filesystem::create_directories(destination.parent_path(), filesystemError);
        // 【逐行批注】 判断条件 (filesystemError) { setError(error, L"无法创建数据库目标目录。"); return fal…，成立时执行对应分支。
        if (filesystemError) { setError(error, L"无法创建数据库目标目录。"); return false; }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!existed && !reserveDatabaseFile(destination, error)) return fa…，成立时执行对应分支。
    if (!existed && !reserveDatabaseFile(destination, error)) return false;
    // 【逐行批注】 执行当前声明或表达式：OwnedDatabaseFile created{destination, !existed};。
    OwnedDatabaseFile created{destination, !existed};
    // 【逐行批注】 执行当前声明或表达式：DatabaseHandle destinationDb;。
    DatabaseHandle destinationDb;
    // 【逐行批注】 判断条件 (!openDatabase(destination, SQLITE_OPEN_READWRITE, &destinationD…，成立时执行对应分支。
    if (!openDatabase(destination, SQLITE_OPEN_READWRITE, &destinationDb.value, error)) return false;
    // 【逐行批注】 调用 sqlite3_busy_timeout 完成本行所需操作：sqlite3_busy_timeout(destinationDb.value, 250);。
    sqlite3_busy_timeout(destinationDb.value, 250);
    // 独占模式在 COMMIT 后仍持锁，覆盖回滚副本、检查点和写回；其他活动读写连接必须退出。
    // 【逐行批注】 判断条件 (sqlite3_exec(destinationDb.value, "PRAGMA locking_mode=EXCLUSIV…，成立时执行对应分支。
    if (sqlite3_exec(destinationDb.value, "PRAGMA locking_mode=EXCLUSIVE;BEGIN EXCLUSIVE;COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"目标数据库正在使用，未执行替换；请关闭其他连接后重试。");。
        setError(error, L"目标数据库正在使用，未执行替换；请关闭其他连接后重试。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (existed && !rollback.empty() && !copyOpenDatabase(destinationDb…，成立时执行对应分支。
    if (existed && !rollback.empty() && !copyOpenDatabase(destinationDb.value, rollback, error)) return false;

    // 由 SQLite 合并 WAL 并处理附属文件，不直接删除可能尚未写回主文件的学习记录。
    // 【逐行批注】 计算右侧结果并写入 sqlite3_stmt* journal，更新当前运行状态。
    sqlite3_stmt* journal = nullptr;
    // 【逐行批注】 计算右侧结果并写入 bool checkpointed，更新当前运行状态。
    bool checkpointed = sqlite3_prepare_v2(destinationDb.value, "PRAGMA journal_mode=DELETE", -1, &journal, nullptr) == SQLITE_OK;
    // 【逐行批注】 判断条件 (checkpointed) {，成立时执行对应分支。
    if (checkpointed) {
        // 【逐行批注】 计算右侧结果并写入 int result，更新当前运行状态。
        const int result = sqlite3_step(journal);
        // 【逐行批注】 计算右侧结果并写入 auto* mode，更新当前运行状态。
        const auto* mode = result == SQLITE_ROW ? sqlite3_column_text(journal, 0) : nullptr;
        // 【逐行批注】 计算右侧结果并写入 checkpointed，更新当前运行状态。
        checkpointed = mode && std::string(reinterpret_cast<const char*>(mode)) == "delete";
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 sqlite3_finalize 完成本行所需操作：sqlite3_finalize(journal);。
    sqlite3_finalize(journal);
    // 【逐行批注】 判断条件 (!checkpointed) {，成立时执行对应分支。
    if (!checkpointed) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"目标数据库仍有活动连接，无法完成安全检查点；原档案和恢复前副本已保留。");。
        setError(error, L"目标数据库仍有活动连接，无法完成安全检查点；原档案和恢复前副本已保留。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：DatabaseHandle snapshotDb;。
    DatabaseHandle snapshotDb;
    // 【逐行批注】 判断条件 (!openDatabase(snapshot, SQLITE_OPEN_READONLY, &snapshotDb.value…，成立时执行对应分支。
    if (!openDatabase(snapshot, SQLITE_OPEN_READONLY, &snapshotDb.value, error)) return false;
    // 【逐行批注】 判断条件 (!copyDatabaseContent(snapshotDb.value, destinationDb.value, err…，成立时执行对应分支。
    if (!copyDatabaseContent(snapshotDb.value, destinationDb.value, error)) return false;
    // 【逐行批注】 计算右侧结果并写入 created.removeOnExit，更新当前运行状态。
    created.removeOnExit = false;
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 toUtf8，执行该函数声明的职责。
std::string toUtf8(const std::wstring& value) {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 判断条件 (value.empty()) return {};，成立时执行对应分支。
    if (value.empty()) return {};
    // 【逐行批注】 计算右侧结果并写入 int size，更新当前运行状态。
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        // 【逐行批注】 执行当前声明或表达式：nullptr, 0, nullptr, nullptr);。
        nullptr, 0, nullptr, nullptr);
    // 【逐行批注】 声明对象 output，并通过构造函数初始化其状态。
    std::string output(static_cast<size_t>(std::max(size, 0)), '\0');
    // 【逐行批注】 判断条件 (size > 0) WideCharToMultiByte(CP_UTF8, 0, value.data(), static_…，成立时执行对应分支。
    if (size > 0) WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        // 【逐行批注】 执行当前声明或表达式：output.data(), size, nullptr, nullptr);。
        output.data(), size, nullptr, nullptr);
    // 【逐行批注】 结束当前函数并返回结果：return output;。
    return output;
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 结束当前函数并返回结果：return std::string(value.begin(), value.end());。
    return std::string(value.begin(), value.end());
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 boolText，执行该函数声明的职责。
std::wstring boolText(bool value) {
    // 【逐行批注】 结束当前函数并返回结果：return value ? L"是" : L"否";。
    return value ? L"是" : L"否";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 计算右侧结果并写入 constexpr int kMinimumWindowWidth，更新当前运行状态。
constexpr int kMinimumWindowWidth = 1024;
// 【逐行批注】 计算右侧结果并写入 constexpr int kMinimumWindowHeight，更新当前运行状态。
constexpr int kMinimumWindowHeight = 640;

// 只调整位置与尺寸，不改变用户是否最大化的选择；越界时整体收回工作区内。
// 【逐行批注】 声明对象 clampToWorkArea，并通过构造函数初始化其状态。
WindowPlacement clampToWorkArea(const WindowRect& workArea, int x, int y, int width, int height,
                               // 【逐行批注】 执行当前声明或表达式：bool maximized) {。
                               bool maximized) {
    // 【逐行批注】 判断条件 (width > workArea.width) width = workArea.width;，成立时执行对应分支。
    if (width > workArea.width) width = workArea.width;
    // 【逐行批注】 判断条件 (height > workArea.height) height = workArea.height;，成立时执行对应分支。
    if (height > workArea.height) height = workArea.height;
    // 【逐行批注】 判断条件 (x < workArea.x) x = workArea.x;，成立时执行对应分支。
    if (x < workArea.x) x = workArea.x;
    // 【逐行批注】 判断条件 (y < workArea.y) y = workArea.y;，成立时执行对应分支。
    if (y < workArea.y) y = workArea.y;
    // 【逐行批注】 判断条件 (x + width > workArea.x + workArea.width) x = workArea.x + workA…，成立时执行对应分支。
    if (x + width > workArea.x + workArea.width) x = workArea.x + workArea.width - width;
    // 【逐行批注】 判断条件 (y + height > workArea.y + workArea.height) y = workArea.y + wor…，成立时执行对应分支。
    if (y + height > workArea.y + workArea.height) y = workArea.y + workArea.height - height;
    // 【逐行批注】 结束当前函数并返回结果：return {x, y, width, height, maximized};。
    return {x, y, width, height, maximized};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 声明对象 computeWindowPlacement，并通过构造函数初始化其状态。
WindowPlacement computeWindowPlacement(const WindowRect& workArea, bool hasSavedGeometry,
                                       // 【逐行批注】 执行当前声明或表达式：const WindowRect& savedGeometry, bool savedVisible,。
                                       const WindowRect& savedGeometry, bool savedVisible,
                                       // 【逐行批注】 执行当前声明或表达式：bool savedMaximized) {。
                                       bool savedMaximized) {
    // 【逐行批注】 判断条件 (workArea.width <= 0 || workArea.height <= 0) return {};，成立时执行对应分支。
    if (workArea.width <= 0 || workArea.height <= 0) return {};
    // 【逐行批注】 判断条件 (hasSavedGeometry && savedVisible && savedGeometry.width >= kMin…，成立时执行对应分支。
    if (hasSavedGeometry && savedVisible && savedGeometry.width >= kMinimumWindowWidth &&
        // 【逐行批注】 执行当前声明或表达式：savedGeometry.height >= kMinimumWindowHeight)。
        savedGeometry.height >= kMinimumWindowHeight)
        // 【逐行批注】 结束当前函数并返回结果：return clampToWorkArea(workArea, savedGeometry.x, savedGeometry.y, sa…。
        return clampToWorkArea(workArea, savedGeometry.x, savedGeometry.y, savedGeometry.width,
                               // 【逐行批注】 执行当前声明或表达式：savedGeometry.height, savedMaximized);。
                               savedGeometry.height, savedMaximized);
    // 没有可用的保存值时取工作区九成并居中；工作区本身更小时不超过工作区。
    // 【逐行批注】 计算右侧结果并写入 int width，更新当前运行状态。
    int width = workArea.width * 9 / 10;
    // 【逐行批注】 计算右侧结果并写入 int height，更新当前运行状态。
    int height = workArea.height * 9 / 10;
    // 【逐行批注】 计算右侧结果并写入 width，更新当前运行状态。
    width = std::max(width, std::min(workArea.width, kMinimumWindowWidth));
    // 【逐行批注】 计算右侧结果并写入 height，更新当前运行状态。
    height = std::max(height, std::min(workArea.height, kMinimumWindowHeight));
    // 【逐行批注】 结束当前函数并返回结果：return {workArea.x + (workArea.width - width) / 2, workArea.y + (work…。
    return {workArea.x + (workArea.width - width) / 2, workArea.y + (workArea.height - height) / 2,
            // 【逐行批注】 执行当前声明或表达式：width, height, false};。
            width, height, false};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 providerProfile，执行该函数声明的职责。
const ProviderProfile& providerProfile(AIProvider provider) {
    // 【逐行批注】 按循环规则重复处理：for (const auto& profile : kProfiles) if (profile.provider == provider) re…。
    for (const auto& profile : kProfiles) if (profile.provider == provider) return profile;
    // 【逐行批注】 结束当前函数并返回结果：return kProfiles[2];。
    return kProfiles[2];
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 providerFromId，执行该函数声明的职责。
AIProvider providerFromId(const std::wstring& id) {
    // 【逐行批注】 按循环规则重复处理：for (const auto& profile : kProfiles) if (id == profile.id) return profile…。
    for (const auto& profile : kProfiles) if (id == profile.id) return profile.provider;
    // 【逐行批注】 结束当前函数并返回结果：return AIProvider::Custom;。
    return AIProvider::Custom;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 inferProvider，执行该函数声明的职责。
AIProvider inferProvider(const std::wstring& baseUrl) {
    // 【逐行批注】 计算右侧结果并写入 std::wstring lower，更新当前运行状态。
    std::wstring lower = baseUrl;
    // 【逐行批注】 调用 transform、towlower 完成本行所需操作：std::transform(lower.begin(), lower.end(), lower.begin(), [](wchar_t …。
    std::transform(lower.begin(), lower.end(), lower.begin(), [](wchar_t value) { return std::towlower(value); });
    // 【逐行批注】 判断条件 (lower.find(L"api.deepseek.com") != std::wstring::npos) return A…，成立时执行对应分支。
    if (lower.find(L"api.deepseek.com") != std::wstring::npos) return AIProvider::DeepSeek;
    // 【逐行批注】 判断条件 (lower.find(L"api.openai.com") != std::wstring::npos) return AIP…，成立时执行对应分支。
    if (lower.find(L"api.openai.com") != std::wstring::npos) return AIProvider::OpenAI;
    // 【逐行批注】 结束当前函数并返回结果：return AIProvider::Custom;。
    return AIProvider::Custom;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 sanitizeBaseUrl，执行该函数声明的职责。
std::wstring sanitizeBaseUrl(const std::wstring& baseUrl) {
    // 【逐行批注】 计算右侧结果并写入 size_t scheme，更新当前运行状态。
    const size_t scheme = baseUrl.find(L"://");
    // 【逐行批注】 判断条件 (scheme == std::wstring::npos) return L"无效地址";，成立时执行对应分支。
    if (scheme == std::wstring::npos) return L"无效地址";
    // 【逐行批注】 计算右侧结果并写入 size_t authorityStart，更新当前运行状态。
    const size_t authorityStart = scheme + 3;
    // 【逐行批注】 计算右侧结果并写入 size_t authorityEnd，更新当前运行状态。
    size_t authorityEnd = baseUrl.find_first_of(L"/?#", authorityStart);
    // 【逐行批注】 判断条件 (authorityEnd == std::wstring::npos) authorityEnd = baseUrl.size…，成立时执行对应分支。
    if (authorityEnd == std::wstring::npos) authorityEnd = baseUrl.size();
    // 【逐行批注】 计算右侧结果并写入 std::wstring authority，更新当前运行状态。
    std::wstring authority = baseUrl.substr(authorityStart, authorityEnd - authorityStart);
    // 【逐行批注】 计算右侧结果并写入 size_t at，更新当前运行状态。
    const size_t at = authority.rfind(L'@');
    // 【逐行批注】 判断条件 (at != std::wstring::npos) authority.erase(0, at + 1);，成立时执行对应分支。
    if (at != std::wstring::npos) authority.erase(0, at + 1);
    // 【逐行批注】 结束当前函数并返回结果：return baseUrl.substr(0, scheme + 3) + authority;。
    return baseUrl.substr(0, scheme + 3) + authority;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 RestartPolicy::recordFailure，执行该函数声明的职责。
std::optional<unsigned> RestartPolicy::recordFailure() {
    // 【逐行批注】 执行当前声明或表达式：constexpr std::array<unsigned, 3> delays{2, 5, 15};。
    constexpr std::array<unsigned, 3> delays{2, 5, 15};
    // 【逐行批注】 判断条件 (failures_ >= static_cast<int>(delays.size())) return std::nullo…，成立时执行对应分支。
    if (failures_ >= static_cast<int>(delays.size())) return std::nullopt;
    // 【逐行批注】 结束当前函数并返回结果：return delays[static_cast<size_t>(failures_++)];。
    return delays[static_cast<size_t>(failures_++)];
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 RestartPolicy::reset，执行该函数声明的职责。
void RestartPolicy::reset() { failures_ = 0; }
// 【逐行批注】 定义函数 RestartPolicy::failureCount，执行该函数声明的职责。
int RestartPolicy::failureCount() const { return failures_; }

// 【逐行批注】 定义函数 rotateLogFiles，执行该函数声明的职责。
bool rotateLogFiles(const std::filesystem::path& activeLog, std::uintmax_t maxBytes, int retainedFiles) {
    // 【逐行批注】 执行当前声明或表达式：std::error_code error;。
    std::error_code error;
    // 【逐行批注】 判断条件 (!std::filesystem::exists(activeLog, error) || std::filesystem::…，成立时执行对应分支。
    if (!std::filesystem::exists(activeLog, error) || std::filesystem::file_size(activeLog, error) < maxBytes)
        // 【逐行批注】 结束当前函数并返回结果：return !error;。
        return !error;
    // 【逐行批注】 按循环规则重复处理：for (int index = retainedFiles; index >= 1; --index) {。
    for (int index = retainedFiles; index >= 1; --index) {
        // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
        const auto current = std::filesystem::path(activeLog.wstring() + L"." + std::to_wstring(index));
        // 【逐行批注】 判断条件 (index == retainedFiles) std::filesystem::remove(current, error);，成立时执行对应分支。
        if (index == retainedFiles) std::filesystem::remove(current, error);
        // 【逐行批注】 判断条件 (index > 1) {，成立时执行对应分支。
        if (index > 1) {
            // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
            const auto previous = std::filesystem::path(activeLog.wstring() + L"." + std::to_wstring(index - 1));
            // 【逐行批注】 判断条件 (std::filesystem::exists(previous, error)) std::filesystem::rena…，成立时执行对应分支。
            if (std::filesystem::exists(previous, error)) std::filesystem::rename(previous, current, error);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
    const auto first = std::filesystem::path(activeLog.wstring() + L".1");
    // 【逐行批注】 调用 rename 完成本行所需操作：std::filesystem::rename(activeLog, first, error);。
    std::filesystem::rename(activeLog, first, error);
    // 【逐行批注】 结束当前函数并返回结果：return !error;。
    return !error;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 validateDatabase，执行该函数声明的职责。
bool validateDatabase(const std::filesystem::path& path, std::wstring& error) {
    // 【逐行批注】 执行当前声明或表达式：error.clear();。
    error.clear();
    // 【逐行批注】 计算右侧结果并写入 sqlite3* database，更新当前运行状态。
    sqlite3* database = nullptr;
    // 【逐行批注】 判断条件 (!std::filesystem::is_regular_file(path)) {，成立时执行对应分支。
    if (!std::filesystem::is_regular_file(path)) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"数据库文件不存在或不是普通文件。");。
        setError(error, L"数据库文件不存在或不是普通文件。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!openDatabase(path, SQLITE_OPEN_READONLY, &database, error)) re…，成立时执行对应分支。
    if (!openDatabase(path, SQLITE_OPEN_READONLY, &database, error)) return false;
    // 【逐行批注】 计算右侧结果并写入 bool ok，更新当前运行状态。
    const bool ok = runQuickCheck(database, error) && hasRequiredTables(database, error) &&
        // 【逐行批注】 调用 hasCompatibleSchemaVersion 完成本行所需操作：hasCompatibleSchemaVersion(database, error);。
        hasCompatibleSchemaVersion(database, error);
    // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(database);。
    sqlite3_close(database);
    // 【逐行批注】 结束当前函数并返回结果：return ok;。
    return ok;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 backupDatabase，并通过构造函数初始化其状态。
bool backupDatabase(const std::filesystem::path& source, const std::filesystem::path& destination,
                    // 【逐行批注】 执行当前声明或表达式：std::wstring& error) {。
                    std::wstring& error) {
    // 【逐行批注】 执行当前声明或表达式：error.clear();。
    error.clear();
    // 【逐行批注】 判断条件 (sameDatabasePath(source, destination)) {，成立时执行对应分支。
    if (sameDatabasePath(source, destination)) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"备份目标不能是当前课程数据库。");。
        setError(error, L"备份目标不能是当前课程数据库。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!validateDatabase(source, error)) return false;，成立时执行对应分支。
    if (!validateDatabase(source, error)) return false;
    // 【逐行批注】 计算右侧结果并写入 auto temporary，更新当前运行状态。
    const auto temporary = temporaryDatabase(destination);
    // 【逐行批注】 判断条件 (!copyDatabase(source, temporary, error)) return false;，成立时执行对应分支。
    if (!copyDatabase(source, temporary, error)) return false;
    // 【逐行批注】 执行当前声明或表达式：OwnedDatabaseFile snapshot{temporary, true};。
    OwnedDatabaseFile snapshot{temporary, true};
    // 【逐行批注】 结束当前函数并返回结果：return installDatabaseSnapshot(temporary, destination, {}, error);。
    return installDatabaseSnapshot(temporary, destination, {}, error);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 restoreDatabase，并通过构造函数初始化其状态。
bool restoreDatabase(const std::filesystem::path& backup, const std::filesystem::path& destination,
                     // 【逐行批注】 执行当前声明或表达式：const std::filesystem::path& rollback, std::wstring& error) {。
                     const std::filesystem::path& rollback, std::wstring& error) {
    // 【逐行批注】 执行当前声明或表达式：error.clear();。
    error.clear();
    // 【逐行批注】 判断条件 (sameDatabasePath(backup, destination)) {，成立时执行对应分支。
    if (sameDatabasePath(backup, destination)) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"请选择与当前课程数据库不同的备份文件。");。
        setError(error, L"请选择与当前课程数据库不同的备份文件。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (rollback.empty() || sameDatabasePath(backup, rollback) || sameD…，成立时执行对应分支。
    if (rollback.empty() || sameDatabasePath(backup, rollback) || sameDatabasePath(destination, rollback)) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"恢复前副本必须使用不同于当前档案及所选备份的新路径。");。
        setError(error, L"恢复前副本必须使用不同于当前档案及所选备份的新路径。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!validateDatabase(backup, error)) return false;，成立时执行对应分支。
    if (!validateDatabase(backup, error)) return false;
    // 【逐行批注】 计算右侧结果并写入 auto temporary，更新当前运行状态。
    const auto temporary = temporaryDatabase(destination);
    // 【逐行批注】 判断条件 (!copyDatabase(backup, temporary, error)) return false;，成立时执行对应分支。
    if (!copyDatabase(backup, temporary, error)) return false;
    // 【逐行批注】 执行当前声明或表达式：OwnedDatabaseFile snapshot{temporary, true};。
    OwnedDatabaseFile snapshot{temporary, true};
    // 【逐行批注】 结束当前函数并返回结果：return installDatabaseSnapshot(temporary, destination, rollback, erro…。
    return installDatabaseSnapshot(temporary, destination, rollback, error);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 writeDiagnosticReport，并通过构造函数初始化其状态。
bool writeDiagnosticReport(const std::filesystem::path& destination, const DiagnosticInfo& info,
                           // 【逐行批注】 执行当前声明或表达式：std::wstring& error) {。
                           std::wstring& error) {
    // 【逐行批注】 执行当前声明或表达式：error.clear();。
    error.clear();
    // 【逐行批注】 执行当前声明或表达式：std::wostringstream report;。
    std::wostringstream report;
    // 【逐行批注】 执行当前声明或表达式：report << L"钢一定制AI 诊断报告\r\n"。
    report << L"钢一定制AI 诊断报告\r\n"
           // 【逐行批注】 执行当前声明或表达式：<< L"版本: " << info.version << L"\r\n"。
           << L"版本: " << info.version << L"\r\n"
           // 【逐行批注】 执行当前声明或表达式：<< L"系统: Windows " << (sizeof(void*) == 8 ? L"x64" : L"x86") << L"\r\n"。
           << L"系统: Windows " << (sizeof(void*) == 8 ? L"x64" : L"x86") << L"\r\n"
           // 【逐行批注】 执行当前声明或表达式：<< L"安装目录: " << info.installDir.wstring() << L"\r\n"。
           << L"安装目录: " << info.installDir.wstring() << L"\r\n"
           // 【逐行批注】 执行当前声明或表达式：<< L"数据目录: " << info.dataDir.wstring() << L"\r\n"。
           << L"数据目录: " << info.dataDir.wstring() << L"\r\n"
           // 【逐行批注】 执行当前声明或表达式：<< L"服务商: " << info.provider << L"\r\n"。
           << L"服务商: " << info.provider << L"\r\n"
           // 【逐行批注】 调用 sanitizeBaseUrl 完成本行所需操作：<< L"接口主机: " << sanitizeBaseUrl(info.baseUrl) << L"\r\n"。
           << L"接口主机: " << sanitizeBaseUrl(info.baseUrl) << L"\r\n"
           // 【逐行批注】 执行当前声明或表达式：<< L"模型: " << info.model << L"\r\n"。
           << L"模型: " << info.model << L"\r\n"
           // 【逐行批注】 调用 boolText 完成本行所需操作：<< L"服务运行中: " << boolText(info.serviceRunning) << L"\r\n"。
           << L"服务运行中: " << boolText(info.serviceRunning) << L"\r\n"
           // 【逐行批注】 执行当前声明或表达式：<< L"端口: " << info.port << L"\r\n"。
           << L"端口: " << info.port << L"\r\n"
           // 【逐行批注】 执行当前声明或表达式：<< L"最近退出码: " << info.exitCode << L"\r\n"。
           << L"最近退出码: " << info.exitCode << L"\r\n"
           // 【逐行批注】 执行当前声明或表达式：<< L"连续重启次数: " << info.restartFailures << L"\r\n";。
           << L"连续重启次数: " << info.restartFailures << L"\r\n";
    // 【逐行批注】 按循环规则重复处理：for (const auto* name : {L"gangyiAI.exe", L"gangyiAI-launcher.exe", L"libc…。
    for (const auto* name : {L"gangyiAI.exe", L"gangyiAI-launcher.exe", L"libcurl-x64.dll",
                              // 【逐行批注】 执行当前声明或表达式：L"public\\school-logo.png", L"public\\campus-background.jpg",。
                              L"public\\school-logo.png", L"public\\campus-background.jpg",
                              // 【逐行批注】 执行当前声明或表达式：L"public\\styles.css"}) {。
                              L"public\\styles.css"}) {
        // 【逐行批注】 计算右侧结果并写入 auto path，更新当前运行状态。
        const auto path = info.installDir / name;
        // 【逐行批注】 执行当前声明或表达式：std::error_code filesystemError;。
        std::error_code filesystemError;
        // 【逐行批注】 调用 is_regular_file 完成本行所需操作：report << L"文件 " << name << L": " << (std::filesystem::is_regular_fil…。
        report << L"文件 " << name << L": " << (std::filesystem::is_regular_file(path, filesystemError) ? L"存在" : L"缺失");
        // 【逐行批注】 判断条件 (!filesystemError && std::filesystem::is_regular_file(path)) rep…，成立时执行对应分支。
        if (!filesystemError && std::filesystem::is_regular_file(path)) report << L" (" << std::filesystem::file_size(path, filesystemError) << L" 字节)";
        // 【逐行批注】 执行当前声明或表达式：report << L"\r\n";。
        report << L"\r\n";
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 声明对象 output，并通过构造函数初始化其状态。
    std::ofstream output(destination, std::ios::binary | std::ios::trunc);
    // 【逐行批注】 判断条件 (!output) {，成立时执行对应分支。
    if (!output) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"无法创建诊断报告文件。");。
        setError(error, L"无法创建诊断报告文件。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 std::string utf8，更新当前运行状态。
    const std::string utf8 = toUtf8(report.str());
    // 【逐行批注】 执行当前声明或表达式：output.write("\xEF\xBB\xBF", 3);。
    output.write("\xEF\xBB\xBF", 3);
    // 【逐行批注】 执行当前声明或表达式：output.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));。
    output.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
    // 【逐行批注】 判断条件 (!output.good()) {，成立时执行对应分支。
    if (!output.good()) {
        // 【逐行批注】 调用 setError 完成本行所需操作：setError(error, L"写入诊断报告失败。");。
        setError(error, L"写入诊断报告失败。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi::launcher。
}  // namespace gangyi::launcher

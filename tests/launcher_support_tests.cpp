#include "launcher_support.hpp"

#include <sqlite3.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "失败: " << message << '\n';
        ++failures;
    }
}

void createDatabase(const std::filesystem::path& path) {
    sqlite3* database = nullptr;
    sqlite3_open(path.string().c_str(), &database);
    sqlite3_exec(database,
        "CREATE TABLE Course(id TEXT PRIMARY KEY);"
        "CREATE TABLE CourseSnapshot(id TEXT PRIMARY KEY);"
        "CREATE TABLE CourseProgress(id TEXT PRIMARY KEY);"
        "CREATE TABLE User(id TEXT PRIMARY KEY);"
        "CREATE TABLE UserSession(id TEXT PRIMARY KEY);"
        "INSERT INTO Course VALUES('course-1');", nullptr, nullptr, nullptr);
    sqlite3_close(database);
}

}  // namespace

int main() {
    using namespace gangyi::launcher;
    expect(inferProvider(L"https://api.deepseek.com/v1") == AIProvider::DeepSeek, "识别 DeepSeek");
    expect(inferProvider(L"https://api.openai.com/v1") == AIProvider::OpenAI, "识别 OpenAI");
    expect(inferProvider(L"https://example.com/v1") == AIProvider::Custom, "识别自定义服务商");
    expect(sanitizeBaseUrl(L"https://user:key@example.com/v1?token=secret") == L"https://example.com",
        "诊断地址必须移除凭据、路径和查询参数");

    RestartPolicy policy;
    expect(policy.recordFailure() == 2, "第一次重启等待 2 秒");
    expect(policy.recordFailure() == 5, "第二次重启等待 5 秒");
    expect(policy.recordFailure() == 15, "第三次重启等待 15 秒");
    expect(!policy.recordFailure().has_value(), "第四次失败停止重启");
    policy.reset();
    expect(policy.failureCount() == 0, "稳定运行后重置计数");

    const auto root = std::filesystem::temp_directory_path() / "gangyiAI-launcher-tests";
    std::error_code filesystemError;
    std::filesystem::remove_all(root, filesystemError);
    std::filesystem::create_directories(root);
    const auto source = root / "source.db";
    const auto backup = root / "课程备份.db";
    const auto restored = root / "restored.db";
    const auto rollback = root / "rollback.db";
    createDatabase(source);
    createDatabase(restored);
    std::wstring error;
    expect(validateDatabase(source, error), "验证有效数据库");
    expect(!backupDatabase(source, source, error), "拒绝覆盖正在使用的源数据库");
    expect(backupDatabase(source, backup, error), "创建数据库备份");
    expect(validateDatabase(backup, error), "验证备份数据库");
    expect(restoreDatabase(backup, restored, rollback, error), "恢复数据库并创建回滚副本");
    expect(validateDatabase(rollback, error), "验证恢复前回滚副本");
    sqlite3* restoredDatabase = nullptr;
    sqlite3_open(restored.string().c_str(), &restoredDatabase);
    sqlite3_stmt* countStatement = nullptr;
    sqlite3_prepare_v2(restoredDatabase, "SELECT COUNT(*) FROM Course WHERE id='course-1'", -1,
                       &countStatement, nullptr);
    expect(sqlite3_step(countStatement) == SQLITE_ROW && sqlite3_column_int(countStatement, 0) == 1,
           "恢复后课程数据完整");
    sqlite3_finalize(countStatement);
    sqlite3_close(restoredDatabase);
    {
        std::ofstream corrupt(root / "corrupt.db");
        corrupt << "not a sqlite database";
    }
    expect(!validateDatabase(root / "corrupt.db", error), "拒绝损坏的数据库");
    sqlite3* futureDatabase = nullptr;
    sqlite3_open16(backup.c_str(), &futureDatabase);
    sqlite3_exec(futureDatabase, "PRAGMA user_version=999", nullptr, nullptr, nullptr);
    sqlite3_close(futureDatabase);
    expect(!validateDatabase(backup, error), "拒绝未来不兼容版本的数据库");

    const auto log = root / "gangyiAI.log";
    {
        std::ofstream output(log, std::ios::binary);
        output << std::string(1024, 'x');
    }
    expect(rotateLogFiles(log, 100, 3), "轮换超限日志");
    expect(std::filesystem::exists(root / "gangyiAI.log.1"), "保留第一份历史日志");

    DiagnosticInfo diagnostic;
    diagnostic.version = L"v0.2.1";
    diagnostic.installDir = root;
    diagnostic.dataDir = root;
    diagnostic.provider = L"自定义";
    diagnostic.baseUrl = L"https://user:secret@example.com/v1?token=secret";
    diagnostic.model = L"test-model";
    const auto report = root / "diagnostic.txt";
    expect(writeDiagnosticReport(report, diagnostic, error), "生成诊断报告");
    std::ifstream input(report, std::ios::binary);
    const std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    expect(contents.find("secret") == std::string::npos, "诊断报告不得泄漏地址凭据");

    std::filesystem::remove_all(root, filesystemError);
    return failures == 0 ? 0 : 1;
}

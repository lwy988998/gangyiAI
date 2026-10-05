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

    {
        // 无保存值：工作区九成并居中；工作区本身更小时不超过工作区。
        const WindowPlacement fresh = computeWindowPlacement({0, 0, 1280, 720}, false, {}, false, false);
        expect(fresh.x == 64 && fresh.y == 36 && fresh.width == 1152 && fresh.height == 648,
               "默认窗口应占工作区九成并居中");
        expect(!fresh.maximized, "首次打开不应自动最大化");
        const WindowPlacement smallArea = computeWindowPlacement({0, 0, 800, 600}, false, {}, false, false);
        expect(smallArea.width == 800 && smallArea.height == 600,
               "工作区小于最小尺寸时窗口不超过工作区");
    }
    {
        // 保存值可见且不小于最小尺寸：沿用用户上次的大小与位置。
        const WindowPlacement kept =
            computeWindowPlacement({0, 0, 1920, 1080}, true, {120, 90, 1500, 900}, true, false);
        expect(kept.x == 120 && kept.y == 90 && kept.width == 1500 && kept.height == 900,
               "保存的窗口尺寸与位置应原样沿用");
        const WindowPlacement left =
            computeWindowPlacement({-1920, 0, 1920, 1080}, true, {-1800, 100, 1200, 800}, true, false);
        expect(left.x == -1800 && left.y == 100 && left.width == 1200,
               "副屏负坐标的保存位置不应被拉回主屏");
        const WindowPlacement zoomed =
            computeWindowPlacement({0, 0, 1920, 1080}, true, {100, 100, 1200, 800}, true, true);
        expect(zoomed.maximized && zoomed.width == 1200, "上次最大化关闭时应沿用最大化");
    }
    {
        // 保存值异常：越界收缩、落在已移除显示器外或小到不可用时回退默认尺寸。
        const WindowPlacement larger =
            computeWindowPlacement({0, 0, 1280, 720}, true, {0, 0, 1600, 900}, true, false);
        expect(larger.width == 1280 && larger.height == 720 && larger.x == 0 && larger.y == 0,
               "保存尺寸超出工作区时应收缩进工作区");
        const WindowPlacement partial =
            computeWindowPlacement({0, 0, 1280, 720}, true, {600, 300, 1200, 800}, true, false);
        expect(partial.x == 80 && partial.y == 0 && partial.width == 1200 && partial.height == 720,
               "超出右下的保存位置应整体收回工作区");
        const WindowPlacement removed =
            computeWindowPlacement({0, 0, 1280, 720}, true, {-3000, 200, 1200, 800}, false, false);
        expect(removed.x == 64 && removed.y == 36 && removed.width == 1152,
               "保存位置落在已移除的显示器外时应回退默认居中");
        const WindowPlacement tiny =
            computeWindowPlacement({0, 0, 1280, 720}, true, {10, 10, 800, 600}, true, false);
        expect(tiny.width == 1152 && tiny.height == 648, "保存尺寸小于最小可用尺寸时应回退默认");
        const WindowPlacement empty =
            computeWindowPlacement({0, 0, 1280, 720}, true, {10, 10, 0, 0}, true, false);
        expect(empty.width == 1152, "保存尺寸为零时应回退默认");
    }

    std::filesystem::remove_all(root, filesystemError);
    return failures == 0 ? 0 : 1;
}

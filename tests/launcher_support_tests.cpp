// 【逐行批注】 引入头文件 "launcher_support.hpp"，为后续实现提供类型或接口声明。
#include "launcher_support.hpp"

// 【逐行批注】 引入头文件 <sqlite3.h>，为后续实现提供类型或接口声明。
#include <sqlite3.h>

// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 int failures，更新当前运行状态。
int failures = 0;

// 【逐行批注】 定义函数 expect，执行该函数声明的职责。
void expect(bool condition, const char* message) {
    // 【逐行批注】 判断条件 (!condition) {，成立时执行对应分支。
    if (!condition) {
        // 【逐行批注】 执行当前声明或表达式：std::cerr << "失败: " << message << '\n';。
        std::cerr << "失败: " << message << '\n';
        // 【逐行批注】 执行当前声明或表达式：++failures;。
        ++failures;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 createDatabase，执行该函数声明的职责。
void createDatabase(const std::filesystem::path& path) {
    // 【逐行批注】 计算右侧结果并写入 sqlite3* database，更新当前运行状态。
    sqlite3* database = nullptr;
    // 【逐行批注】 调用 sqlite3_open 完成本行所需操作：sqlite3_open(path.string().c_str(), &database);。
    sqlite3_open(path.string().c_str(), &database);
    // 【逐行批注】 调用 sqlite3_exec 完成本行所需操作：sqlite3_exec(database,。
    sqlite3_exec(database,
        // 【逐行批注】 调用 Course 完成本行所需操作："CREATE TABLE Course(id TEXT PRIMARY KEY);"。
        "CREATE TABLE Course(id TEXT PRIMARY KEY);"
        // 【逐行批注】 调用 CourseSnapshot 完成本行所需操作："CREATE TABLE CourseSnapshot(id TEXT PRIMARY KEY);"。
        "CREATE TABLE CourseSnapshot(id TEXT PRIMARY KEY);"
        // 【逐行批注】 调用 CourseProgress 完成本行所需操作："CREATE TABLE CourseProgress(id TEXT PRIMARY KEY);"。
        "CREATE TABLE CourseProgress(id TEXT PRIMARY KEY);"
        // 【逐行批注】 调用 User 完成本行所需操作："CREATE TABLE User(id TEXT PRIMARY KEY);"。
        "CREATE TABLE User(id TEXT PRIMARY KEY);"
        // 【逐行批注】 调用 UserSession 完成本行所需操作："CREATE TABLE UserSession(id TEXT PRIMARY KEY);"。
        "CREATE TABLE UserSession(id TEXT PRIMARY KEY);"
        // 【逐行批注】 调用 MyAIRecord 完成本行所需操作："CREATE TABLE MyAIRecord(id TEXT PRIMARY KEY,kind TEXT NOT NULL,paylo…。
        "CREATE TABLE MyAIRecord(id TEXT PRIMARY KEY,kind TEXT NOT NULL,payload TEXT NOT NULL,updatedAt TEXT NOT NULL,version INTEGER NOT NULL);"
        // 【逐行批注】 调用 VALUES 完成本行所需操作："INSERT INTO MyAIRecord VALUES('fixed-source','library-source','{\"sn…。
        "INSERT INTO MyAIRecord VALUES('fixed-source','library-source','{\"snapshotId\":\"fixed-snapshot\"}','2026-10-14',3);"
        // 【逐行批注】 调用 VALUES 完成本行所需操作："INSERT INTO MyAIRecord VALUES('fixed-snapshot','library-snapshot','{…。
        "INSERT INTO MyAIRecord VALUES('fixed-snapshot','library-snapshot','{\"segments\":[{\"id\":\"p1-1\",\"page\":1,\"text\":\"saved evidence\"}]}','2026-10-14',1);"
        // 【逐行批注】 调用 VALUES 完成本行所需操作："INSERT INTO MyAIRecord VALUES('manual-choice','library-choice','{\"m…。
        "INSERT INTO MyAIRecord VALUES('manual-choice','library-choice','{\"manual\":true,\"state\":\"excluded\"}','2026-10-14',2);"
        // 【逐行批注】 调用 VALUES 完成本行所需操作："INSERT INTO Course VALUES('course-1');", nullptr, nullptr, nullptr);。
        "INSERT INTO Course VALUES('course-1');", nullptr, nullptr, nullptr);
    // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(database);。
    sqlite3_close(database);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 引入头文件 "launcher_restore_checks.hpp"，为后续实现提供类型或接口声明。
#include "launcher_restore_checks.hpp"

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main(int argc, char** argv) {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi::launcher;。
    using namespace gangyi::launcher;
    // 仅测试工具提供备份恢复入口，CI升级验证复用产品真实SQLite机制；不进入安装包。
    // 【逐行批注】 判断条件 (argc >= 2) {，成立时执行对应分支。
    if (argc >= 2) {
        // 【逐行批注】 执行当前声明或表达式：std::wstring error;。
        std::wstring error;
        // 【逐行批注】 判断条件 (argc == 3 && std::string(argv[1]) == "--crash-wal")，成立时执行对应分支。
        if (argc == 3 && std::string(argv[1]) == "--crash-wal")
            // 【逐行批注】 结束当前函数并返回结果：return launcher_restore_checks::crashWal(std::filesystem::u8path(argv…。
            return launcher_restore_checks::crashWal(std::filesystem::u8path(argv[2]));
        // 【逐行批注】 判断条件 (argc == 4 && std::string(argv[1]) == "--backup")，成立时执行对应分支。
        if (argc == 4 && std::string(argv[1]) == "--backup")
            // 【逐行批注】 结束当前函数并返回结果：return backupDatabase(std::filesystem::u8path(argv[2]), std::filesyst…。
            return backupDatabase(std::filesystem::u8path(argv[2]), std::filesystem::u8path(argv[3]), error) ? 0 : 1;
        // 【逐行批注】 判断条件 (argc == 5 && std::string(argv[1]) == "--restore")，成立时执行对应分支。
        if (argc == 5 && std::string(argv[1]) == "--restore")
            // 【逐行批注】 结束当前函数并返回结果：return restoreDatabase(std::filesystem::u8path(argv[2]), std::filesys…。
            return restoreDatabase(std::filesystem::u8path(argv[2]), std::filesystem::u8path(argv[3]), std::filesystem::u8path(argv[4]), error) ? 0 : 1;
        // 【逐行批注】 结束当前函数并返回结果：return 2;。
        return 2;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 expect、inferProvider 完成本行所需操作：expect(inferProvider(L"https://api.deepseek.com/v1") == AIProvider::D…。
    expect(inferProvider(L"https://api.deepseek.com/v1") == AIProvider::DeepSeek, "识别 DeepSeek");
    // 【逐行批注】 调用 expect、inferProvider 完成本行所需操作：expect(inferProvider(L"https://api.openai.com/v1") == AIProvider::Ope…。
    expect(inferProvider(L"https://api.openai.com/v1") == AIProvider::OpenAI, "识别 OpenAI");
    // 【逐行批注】 调用 expect、inferProvider 完成本行所需操作：expect(inferProvider(L"https://example.com/v1") == AIProvider::Custom…。
    expect(inferProvider(L"https://example.com/v1") == AIProvider::Custom, "识别自定义服务商");
    // 【逐行批注】 计算右侧结果并写入 expect(sanitizeBaseUrl(L"https://user:key@ex…，更新当前运行状态。
    expect(sanitizeBaseUrl(L"https://user:key@example.com/v1?token=secret") == L"https://example.com",
        // 【逐行批注】 执行当前声明或表达式："诊断地址必须移除凭据、路径和查询参数");。
        "诊断地址必须移除凭据、路径和查询参数");

    // 【逐行批注】 执行当前声明或表达式：RestartPolicy policy;。
    RestartPolicy policy;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(policy.recordFailure() == 2, "第一次重启等待 2 秒");。
    expect(policy.recordFailure() == 2, "第一次重启等待 2 秒");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(policy.recordFailure() == 5, "第二次重启等待 5 秒");。
    expect(policy.recordFailure() == 5, "第二次重启等待 5 秒");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(policy.recordFailure() == 15, "第三次重启等待 15 秒");。
    expect(policy.recordFailure() == 15, "第三次重启等待 15 秒");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(!policy.recordFailure().has_value(), "第四次失败停止重启");。
    expect(!policy.recordFailure().has_value(), "第四次失败停止重启");
    // 【逐行批注】 执行当前声明或表达式：policy.reset();。
    policy.reset();
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(policy.failureCount() == 0, "稳定运行后重置计数");。
    expect(policy.failureCount() == 0, "稳定运行后重置计数");

    // 【逐行批注】 计算右侧结果并写入 auto root，更新当前运行状态。
    const auto root = std::filesystem::temp_directory_path() / "gangyiAI-launcher-tests";
    // 【逐行批注】 执行当前声明或表达式：std::error_code filesystemError;。
    std::error_code filesystemError;
    // 【逐行批注】 调用 remove_all 完成本行所需操作：std::filesystem::remove_all(root, filesystemError);。
    std::filesystem::remove_all(root, filesystemError);
    // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(root);。
    std::filesystem::create_directories(root);
    // 【逐行批注】 计算右侧结果并写入 auto source，更新当前运行状态。
    const auto source = root / "source.db";
    // 【逐行批注】 计算右侧结果并写入 auto backup，更新当前运行状态。
    const auto backup = root / "课程备份.db";
    // 【逐行批注】 计算右侧结果并写入 auto restored，更新当前运行状态。
    const auto restored = root / "restored.db";
    // 【逐行批注】 计算右侧结果并写入 auto rollback，更新当前运行状态。
    const auto rollback = root / "rollback.db";
    // 【逐行批注】 调用 createDatabase 完成本行所需操作：createDatabase(source);。
    createDatabase(source);
    // 【逐行批注】 调用 createDatabase 完成本行所需操作：createDatabase(restored);。
    createDatabase(restored);
    // 【逐行批注】 执行当前声明或表达式：std::wstring error;。
    std::wstring error;
    // 【逐行批注】 调用 expect、validateDatabase 完成本行所需操作：expect(validateDatabase(source, error), "验证有效数据库");。
    expect(validateDatabase(source, error), "验证有效数据库");
    // 【逐行批注】 调用 expect、backupDatabase 完成本行所需操作：expect(!backupDatabase(source, source, error), "拒绝覆盖正在使用的源数据库");。
    expect(!backupDatabase(source, source, error), "拒绝覆盖正在使用的源数据库");
    // 【逐行批注】 调用 expect、backupDatabase 完成本行所需操作：expect(backupDatabase(source, backup, error), "创建数据库备份");。
    expect(backupDatabase(source, backup, error), "创建数据库备份");
    // 【逐行批注】 调用 expect、validateDatabase 完成本行所需操作：expect(validateDatabase(backup, error), "验证备份数据库");。
    expect(validateDatabase(backup, error), "验证备份数据库");
    // 【逐行批注】 调用 expect、restoreDatabase 完成本行所需操作：expect(restoreDatabase(backup, restored, rollback, error), "恢复数据库并创建回…。
    expect(restoreDatabase(backup, restored, rollback, error), "恢复数据库并创建回滚副本");
    // 【逐行批注】 调用 expect、validateDatabase 完成本行所需操作：expect(validateDatabase(rollback, error), "验证恢复前回滚副本");。
    expect(validateDatabase(rollback, error), "验证恢复前回滚副本");
    // 【逐行批注】 计算右侧结果并写入 sqlite3* restoredDatabase，更新当前运行状态。
    sqlite3* restoredDatabase = nullptr;
    // 【逐行批注】 调用 sqlite3_open 完成本行所需操作：sqlite3_open(restored.string().c_str(), &restoredDatabase);。
    sqlite3_open(restored.string().c_str(), &restoredDatabase);
    // 【逐行批注】 计算右侧结果并写入 sqlite3_stmt* countStatement，更新当前运行状态。
    sqlite3_stmt* countStatement = nullptr;
    // 【逐行批注】 计算右侧结果并写入 sqlite3_prepare_v2(restoredDatabase, "SELECT…，更新当前运行状态。
    sqlite3_prepare_v2(restoredDatabase, "SELECT COUNT(*) FROM Course WHERE id='course-1'", -1,
                       // 【逐行批注】 执行当前声明或表达式：&countStatement, nullptr);。
                       &countStatement, nullptr);
    // 【逐行批注】 调用 expect、sqlite3_step、sqlite3_column_int 完成本行所需操作：expect(sqlite3_step(countStatement) == SQLITE_ROW && sqlite3_column_i…。
    expect(sqlite3_step(countStatement) == SQLITE_ROW && sqlite3_column_int(countStatement, 0) == 1,
           // 【逐行批注】 执行当前声明或表达式："恢复后课程数据完整");。
           "恢复后课程数据完整");
    // 【逐行批注】 调用 sqlite3_finalize 完成本行所需操作：sqlite3_finalize(countStatement);。
    sqlite3_finalize(countStatement);
    // 【逐行批注】 计算右侧结果并写入 sqlite3_prepare_v2(restoredDatabase, "SELECT…，更新当前运行状态。
    sqlite3_prepare_v2(restoredDatabase, "SELECT payload,version FROM MyAIRecord WHERE id='fixed-snapshot'", -1, &countStatement, nullptr);
    // 【逐行批注】 调用 expect、sqlite3_step、string、sqlite3_column_text 完成本行所需操作：expect(sqlite3_step(countStatement) == SQLITE_ROW && std::string(rein…。
    expect(sqlite3_step(countStatement) == SQLITE_ROW && std::string(reinterpret_cast<const char*>(sqlite3_column_text(countStatement,0))) ==
        // 【逐行批注】 调用 sqlite3_column_int 完成本行所需操作："{\"segments\":[{\"id\":\"p1-1\",\"page\":1,\"text\":\"saved evidence…。
        "{\"segments\":[{\"id\":\"p1-1\",\"page\":1,\"text\":\"saved evidence\"}]}" && sqlite3_column_int(countStatement,1) == 1,
        // 【逐行批注】 执行当前声明或表达式："备份恢复保留固定原文、页码和版本");。
        "备份恢复保留固定原文、页码和版本");
    // 【逐行批注】 调用 sqlite3_finalize 完成本行所需操作：sqlite3_finalize(countStatement);。
    sqlite3_finalize(countStatement);
    // 【逐行批注】 计算右侧结果并写入 sqlite3_prepare_v2(restoredDatabase, "SELECT…，更新当前运行状态。
    sqlite3_prepare_v2(restoredDatabase, "SELECT payload,version FROM MyAIRecord WHERE id='manual-choice'", -1, &countStatement, nullptr);
    // 【逐行批注】 调用 expect、sqlite3_step、string、sqlite3_column_text 完成本行所需操作：expect(sqlite3_step(countStatement) == SQLITE_ROW && std::string(rein…。
    expect(sqlite3_step(countStatement) == SQLITE_ROW && std::string(reinterpret_cast<const char*>(sqlite3_column_text(countStatement,0))) ==
        // 【逐行批注】 调用 sqlite3_column_int 完成本行所需操作："{\"manual\":true,\"state\":\"excluded\"}" && sqlite3_column_int(coun…。
        "{\"manual\":true,\"state\":\"excluded\"}" && sqlite3_column_int(countStatement,1) == 2,
        // 【逐行批注】 执行当前声明或表达式："备份恢复保留用户排除和选择版本");。
        "备份恢复保留用户排除和选择版本");
    // 【逐行批注】 调用 sqlite3_finalize 完成本行所需操作：sqlite3_finalize(countStatement);。
    sqlite3_finalize(countStatement);
    // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(restoredDatabase);。
    sqlite3_close(restoredDatabase);
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 corrupt，并通过构造函数初始化其状态。
        std::ofstream corrupt(root / "corrupt.db");
        // 【逐行批注】 执行当前声明或表达式：corrupt << "not a sqlite database";。
        corrupt << "not a sqlite database";
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 expect、validateDatabase 完成本行所需操作：expect(!validateDatabase(root / "corrupt.db", error), "拒绝损坏的数据库");。
    expect(!validateDatabase(root / "corrupt.db", error), "拒绝损坏的数据库");
    // 【逐行批注】 计算右侧结果并写入 sqlite3* futureDatabase，更新当前运行状态。
    sqlite3* futureDatabase = nullptr;
    // 【逐行批注】 调用 sqlite3_open 完成本行所需操作：sqlite3_open(backup.u8string().c_str(), &futureDatabase);。
    sqlite3_open(backup.u8string().c_str(), &futureDatabase);
    // 【逐行批注】 计算右侧结果并写入 sqlite3_exec(futureDatabase, "PRAGMA user_ve…，更新当前运行状态。
    sqlite3_exec(futureDatabase, "PRAGMA user_version=999", nullptr, nullptr, nullptr);
    // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(futureDatabase);。
    sqlite3_close(futureDatabase);
    // 【逐行批注】 调用 expect、validateDatabase 完成本行所需操作：expect(!validateDatabase(backup, error), "拒绝未来不兼容版本的数据库");。
    expect(!validateDatabase(backup, error), "拒绝未来不兼容版本的数据库");

    // 【逐行批注】 计算右侧结果并写入 auto log，更新当前运行状态。
    const auto log = root / "gangyiAI.log";
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 output，并通过构造函数初始化其状态。
        std::ofstream output(log, std::ios::binary);
        // 【逐行批注】 调用 string 完成本行所需操作：output << std::string(1024, 'x');。
        output << std::string(1024, 'x');
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 expect、rotateLogFiles 完成本行所需操作：expect(rotateLogFiles(log, 100, 3), "轮换超限日志");。
    expect(rotateLogFiles(log, 100, 3), "轮换超限日志");
    // 【逐行批注】 调用 expect、exists 完成本行所需操作：expect(std::filesystem::exists(root / "gangyiAI.log.1"), "保留第一份历史日志");。
    expect(std::filesystem::exists(root / "gangyiAI.log.1"), "保留第一份历史日志");

    // 【逐行批注】 执行当前声明或表达式：DiagnosticInfo diagnostic;。
    DiagnosticInfo diagnostic;
    // 【逐行批注】 计算右侧结果并写入 diagnostic.version，更新当前运行状态。
    diagnostic.version = L"v0.2.1";
    // 【逐行批注】 计算右侧结果并写入 diagnostic.installDir，更新当前运行状态。
    diagnostic.installDir = root;
    // 【逐行批注】 计算右侧结果并写入 diagnostic.dataDir，更新当前运行状态。
    diagnostic.dataDir = root;
    // 【逐行批注】 计算右侧结果并写入 diagnostic.provider，更新当前运行状态。
    diagnostic.provider = L"自定义";
    // 【逐行批注】 计算右侧结果并写入 diagnostic.baseUrl，更新当前运行状态。
    diagnostic.baseUrl = L"https://user:secret@example.com/v1?token=secret";
    // 【逐行批注】 计算右侧结果并写入 diagnostic.model，更新当前运行状态。
    diagnostic.model = L"test-model";
    // 【逐行批注】 计算右侧结果并写入 auto report，更新当前运行状态。
    const auto report = root / "diagnostic.txt";
    // 【逐行批注】 调用 expect、writeDiagnosticReport 完成本行所需操作：expect(writeDiagnosticReport(report, diagnostic, error), "生成诊断报告");。
    expect(writeDiagnosticReport(report, diagnostic, error), "生成诊断报告");
    // 【逐行批注】 声明对象 input，并通过构造函数初始化其状态。
    std::ifstream input(report, std::ios::binary);
    // 【逐行批注】 调用 contents 完成本行所需操作：const std::string contents((std::istreambuf_iterator<char>(input)), s…。
    const std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(contents.find("secret") == std::string::npos, "诊断报告不得泄漏地址凭据");。
    expect(contents.find("secret") == std::string::npos, "诊断报告不得泄漏地址凭据");

    // 【逐行批注】 开始当前代码块。
    {
        // 无保存值：工作区九成并居中；工作区本身更小时不超过工作区。
        // 【逐行批注】 计算右侧结果并写入 WindowPlacement fresh，更新当前运行状态。
        const WindowPlacement fresh = computeWindowPlacement({0, 0, 1280, 720}, false, {}, false, false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(fresh.x == 64 && fresh.y == 36 && fresh.width == 1152 && fresh…。
        expect(fresh.x == 64 && fresh.y == 36 && fresh.width == 1152 && fresh.height == 648,
               // 【逐行批注】 执行当前声明或表达式："默认窗口应占工作区九成并居中");。
               "默认窗口应占工作区九成并居中");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(!fresh.maximized, "首次打开不应自动最大化");。
        expect(!fresh.maximized, "首次打开不应自动最大化");
        // 【逐行批注】 计算右侧结果并写入 WindowPlacement smallArea，更新当前运行状态。
        const WindowPlacement smallArea = computeWindowPlacement({0, 0, 800, 600}, false, {}, false, false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(smallArea.width == 800 && smallArea.height == 600,。
        expect(smallArea.width == 800 && smallArea.height == 600,
               // 【逐行批注】 执行当前声明或表达式："工作区小于最小尺寸时窗口不超过工作区");。
               "工作区小于最小尺寸时窗口不超过工作区");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 保存值可见且不小于最小尺寸：沿用用户上次的大小与位置。
        // 【逐行批注】 计算右侧结果并写入 WindowPlacement kept，更新当前运行状态。
        const WindowPlacement kept =
            // 【逐行批注】 调用 computeWindowPlacement 完成本行所需操作：computeWindowPlacement({0, 0, 1920, 1080}, true, {120, 90, 1500, 900}…。
            computeWindowPlacement({0, 0, 1920, 1080}, true, {120, 90, 1500, 900}, true, false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(kept.x == 120 && kept.y == 90 && kept.width == 1500 && kept.he…。
        expect(kept.x == 120 && kept.y == 90 && kept.width == 1500 && kept.height == 900,
               // 【逐行批注】 执行当前声明或表达式："保存的窗口尺寸与位置应原样沿用");。
               "保存的窗口尺寸与位置应原样沿用");
        // 【逐行批注】 计算右侧结果并写入 WindowPlacement left，更新当前运行状态。
        const WindowPlacement left =
            // 【逐行批注】 调用 computeWindowPlacement 完成本行所需操作：computeWindowPlacement({-1920, 0, 1920, 1080}, true, {-1800, 100, 120…。
            computeWindowPlacement({-1920, 0, 1920, 1080}, true, {-1800, 100, 1200, 800}, true, false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(left.x == -1800 && left.y == 100 && left.width == 1200,。
        expect(left.x == -1800 && left.y == 100 && left.width == 1200,
               // 【逐行批注】 执行当前声明或表达式："副屏负坐标的保存位置不应被拉回主屏");。
               "副屏负坐标的保存位置不应被拉回主屏");
        // 【逐行批注】 计算右侧结果并写入 WindowPlacement zoomed，更新当前运行状态。
        const WindowPlacement zoomed =
            // 【逐行批注】 调用 computeWindowPlacement 完成本行所需操作：computeWindowPlacement({0, 0, 1920, 1080}, true, {100, 100, 1200, 800…。
            computeWindowPlacement({0, 0, 1920, 1080}, true, {100, 100, 1200, 800}, true, true);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(zoomed.maximized && zoomed.width == 1200, "上次最大化关闭时应沿用最大化");。
        expect(zoomed.maximized && zoomed.width == 1200, "上次最大化关闭时应沿用最大化");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 保存值异常：越界收缩、落在已移除显示器外或小到不可用时回退默认尺寸。
        // 【逐行批注】 计算右侧结果并写入 WindowPlacement larger，更新当前运行状态。
        const WindowPlacement larger =
            // 【逐行批注】 调用 computeWindowPlacement 完成本行所需操作：computeWindowPlacement({0, 0, 1280, 720}, true, {0, 0, 1600, 900}, tr…。
            computeWindowPlacement({0, 0, 1280, 720}, true, {0, 0, 1600, 900}, true, false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(larger.width == 1280 && larger.height == 720 && larger.x == 0 …。
        expect(larger.width == 1280 && larger.height == 720 && larger.x == 0 && larger.y == 0,
               // 【逐行批注】 执行当前声明或表达式："保存尺寸超出工作区时应收缩进工作区");。
               "保存尺寸超出工作区时应收缩进工作区");
        // 【逐行批注】 计算右侧结果并写入 WindowPlacement partial，更新当前运行状态。
        const WindowPlacement partial =
            // 【逐行批注】 调用 computeWindowPlacement 完成本行所需操作：computeWindowPlacement({0, 0, 1280, 720}, true, {600, 300, 1200, 800}…。
            computeWindowPlacement({0, 0, 1280, 720}, true, {600, 300, 1200, 800}, true, false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(partial.x == 80 && partial.y == 0 && partial.width == 1200 && …。
        expect(partial.x == 80 && partial.y == 0 && partial.width == 1200 && partial.height == 720,
               // 【逐行批注】 执行当前声明或表达式："超出右下的保存位置应整体收回工作区");。
               "超出右下的保存位置应整体收回工作区");
        // 【逐行批注】 计算右侧结果并写入 WindowPlacement removed，更新当前运行状态。
        const WindowPlacement removed =
            // 【逐行批注】 调用 computeWindowPlacement 完成本行所需操作：computeWindowPlacement({0, 0, 1280, 720}, true, {-3000, 200, 1200, 80…。
            computeWindowPlacement({0, 0, 1280, 720}, true, {-3000, 200, 1200, 800}, false, false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(removed.x == 64 && removed.y == 36 && removed.width == 1152,。
        expect(removed.x == 64 && removed.y == 36 && removed.width == 1152,
               // 【逐行批注】 执行当前声明或表达式："保存位置落在已移除的显示器外时应回退默认居中");。
               "保存位置落在已移除的显示器外时应回退默认居中");
        // 【逐行批注】 计算右侧结果并写入 WindowPlacement tiny，更新当前运行状态。
        const WindowPlacement tiny =
            // 【逐行批注】 调用 computeWindowPlacement 完成本行所需操作：computeWindowPlacement({0, 0, 1280, 720}, true, {10, 10, 800, 600}, t…。
            computeWindowPlacement({0, 0, 1280, 720}, true, {10, 10, 800, 600}, true, false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(tiny.width == 1152 && tiny.height == 648, "保存尺寸小于最小可用尺寸时应回退默认"…。
        expect(tiny.width == 1152 && tiny.height == 648, "保存尺寸小于最小可用尺寸时应回退默认");
        // 【逐行批注】 计算右侧结果并写入 WindowPlacement empty，更新当前运行状态。
        const WindowPlacement empty =
            // 【逐行批注】 调用 computeWindowPlacement 完成本行所需操作：computeWindowPlacement({0, 0, 1280, 720}, true, {10, 10, 0, 0}, true,…。
            computeWindowPlacement({0, 0, 1280, 720}, true, {10, 10, 0, 0}, true, false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(empty.width == 1152, "保存尺寸为零时应回退默认");。
        expect(empty.width == 1152, "保存尺寸为零时应回退默认");
    // 【逐行批注】 结束当前代码块。
    }

// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // Windows 子进程通过 GetModuleFileNameW 取得真实宽字符路径；argv[0] 在中文目录下
    // 是本地编码，不能再次按 UTF-8 解码，否则会抛 filesystem conversion_error。
    // 【逐行批注】 调用 run 完成本行所需操作：launcher_restore_checks::run(root, {});。
    launcher_restore_checks::run(root, {});
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 run、absolute、path 完成本行所需操作：launcher_restore_checks::run(root, std::filesystem::absolute(std::fil…。
    launcher_restore_checks::run(root, std::filesystem::absolute(std::filesystem::path(argv[0])));
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 调用 remove_all 完成本行所需操作：std::filesystem::remove_all(root, filesystemError);。
    std::filesystem::remove_all(root, filesystemError);
    // 【逐行批注】 结束当前函数并返回结果：return failures == 0 ? 0 : 1;。
    return failures == 0 ? 0 : 1;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
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
// 【逐行批注】 引入头文件 <sys/wait.h>，为后续实现提供类型或接口声明。
#include <sys/wait.h>
// 【逐行批注】 引入头文件 <unistd.h>，为后续实现提供类型或接口声明。
#include <unistd.h>
// 【逐行批注】 处理条件编译指令：#endif。
#endif

// 【逐行批注】 声明或进入命名空间：namespace launcher_restore_checks {。
namespace launcher_restore_checks {
// 【逐行批注】 定义函数 sql，执行该函数声明的职责。
inline bool sql(sqlite3* database, const char* statement) {
    // 【逐行批注】 结束当前函数并返回结果：return sqlite3_exec(database, statement, nullptr, nullptr, nullptr) =…。
    return sqlite3_exec(database, statement, nullptr, nullptr, nullptr) == SQLITE_OK;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 setCourse，执行该函数声明的职责。
inline void setCourse(const std::filesystem::path& path, const char* marker) {
    // 【逐行批注】 计算右侧结果并写入 sqlite3* database，更新当前运行状态。
    sqlite3* database = nullptr;
    // 【逐行批注】 调用 expect、sqlite3_open 完成本行所需操作：expect(sqlite3_open(path.u8string().c_str(), &database) == SQLITE_OK,…。
    expect(sqlite3_open(path.u8string().c_str(), &database) == SQLITE_OK, "打开合成档案");
    // 【逐行批注】 调用 expect、sql 完成本行所需操作：expect(sql(database, "DELETE FROM Course"), "准备合成课程标记");。
    expect(sql(database, "DELETE FROM Course"), "准备合成课程标记");
    // 【逐行批注】 计算右侧结果并写入 sqlite3_stmt* statement，更新当前运行状态。
    sqlite3_stmt* statement = nullptr;
    // 【逐行批注】 计算右侧结果并写入 bool prepared，更新当前运行状态。
    const bool prepared = sqlite3_prepare_v2(database, "INSERT INTO Course VALUES(?)", -1, &statement, nullptr) == SQLITE_OK;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(prepared, "准备合成课程写入");。
    expect(prepared, "准备合成课程写入");
    // 【逐行批注】 判断条件 (prepared) {，成立时执行对应分支。
    if (prepared) {
        // 【逐行批注】 调用 sqlite3_bind_text 完成本行所需操作：sqlite3_bind_text(statement, 1, marker, -1, SQLITE_TRANSIENT);。
        sqlite3_bind_text(statement, 1, marker, -1, SQLITE_TRANSIENT);
        // 【逐行批注】 调用 expect、sqlite3_step 完成本行所需操作：expect(sqlite3_step(statement) == SQLITE_DONE, "保存合成课程标记");。
        expect(sqlite3_step(statement) == SQLITE_DONE, "保存合成课程标记");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 sqlite3_finalize、sqlite3_close 完成本行所需操作：sqlite3_finalize(statement); sqlite3_close(database);。
    sqlite3_finalize(statement); sqlite3_close(database);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 course，执行该函数声明的职责。
inline std::string course(const std::filesystem::path& path) {
    // 【逐行批注】 计算右侧结果并写入 sqlite3* database，更新当前运行状态。
    sqlite3* database = nullptr;
    // 【逐行批注】 判断条件 (sqlite3_open_v2(path.u8string().c_str(), &database, SQLITE_OPEN…，成立时执行对应分支。
    if (sqlite3_open_v2(path.u8string().c_str(), &database, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        // 【逐行批注】 判断条件 (database) sqlite3_close(database); return {};，成立时执行对应分支。
        if (database) sqlite3_close(database); return {};
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 sqlite3_stmt* statement，更新当前运行状态。
    sqlite3_stmt* statement = nullptr; std::string value;
    // 【逐行批注】 判断条件 (sqlite3_prepare_v2(database, "SELECT id FROM Course", -1, &stat…，成立时执行对应分支。
    if (sqlite3_prepare_v2(database, "SELECT id FROM Course", -1, &statement, nullptr) == SQLITE_OK && sqlite3_step(statement) == SQLITE_ROW)
        // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
        value = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
    // 【逐行批注】 调用 sqlite3_finalize、sqlite3_close 完成本行所需操作：sqlite3_finalize(statement); sqlite3_close(database); return value;。
    sqlite3_finalize(statement); sqlite3_close(database); return value;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 crashWal，执行该函数声明的职责。
inline int crashWal(const std::filesystem::path& path) {
    // 【逐行批注】 调用 createDatabase 完成本行所需操作：createDatabase(path);。
    createDatabase(path);
    // 【逐行批注】 计算右侧结果并写入 sqlite3* database，更新当前运行状态。
    sqlite3* database = nullptr;
    // 【逐行批注】 判断条件 (sqlite3_open(path.u8string().c_str(), &database) != SQLITE_OK ||，成立时执行对应分支。
    if (sqlite3_open(path.u8string().c_str(), &database) != SQLITE_OK ||
        // 【逐行批注】 调用 sql、VALUES 完成本行所需操作：!sql(database, "PRAGMA journal_mode=WAL;DELETE FROM Course;INSERT INT…。
        !sql(database, "PRAGMA journal_mode=WAL;DELETE FROM Course;INSERT INTO Course VALUES('crash-current');")) return 91;
    // 模拟进程异常退出，跳过析构和 sqlite3_close，保留已提交但未检查点的真实 WAL。
    // 【逐行批注】 调用 _Exit 完成本行所需操作：std::_Exit(0);。
    std::_Exit(0);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 runCrashWriter，执行该函数声明的职责。
inline bool runCrashWriter(const std::filesystem::path& executable, const std::filesystem::path& path) {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 执行当前声明或表达式：(void)executable;。
    (void)executable;
    // 【逐行批注】 执行当前声明或表达式：wchar_t module[MAX_PATH]{};。
    wchar_t module[MAX_PATH]{};
    // 【逐行批注】 判断条件 (!GetModuleFileNameW(nullptr, module, MAX_PATH)) return false;，成立时执行对应分支。
    if (!GetModuleFileNameW(nullptr, module, MAX_PATH)) return false;
    // 【逐行批注】 计算右侧结果并写入 command，更新当前运行状态。
    auto command = L"\"" + std::wstring(module) + L"\" --crash-wal \"" + path.wstring() + L"\"";
    // 【逐行批注】 计算右侧结果并写入 STARTUPINFOW startup{}; startup.cb，更新当前运行状态。
    STARTUPINFOW startup{}; startup.cb = sizeof(startup);
    // 【逐行批注】 执行当前声明或表达式：PROCESS_INFORMATION process{};。
    PROCESS_INFORMATION process{};
    // 【逐行批注】 判断条件 (!CreateProcessW(module, command.data(), nullptr, nullptr, FALSE…，成立时执行对应分支。
    if (!CreateProcessW(module, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process)) return false;
    // 【逐行批注】 计算右侧结果并写入 bool completed，更新当前运行状态。
    const bool completed = WaitForSingleObject(process.hProcess, 10000) == WAIT_OBJECT_0;
    // 【逐行批注】 计算右侧结果并写入 DWORD exit，更新当前运行状态。
    DWORD exit = 1;
    // 【逐行批注】 判断条件 (completed) GetExitCodeProcess(process.hProcess, &exit);，成立时执行对应分支。
    if (completed) GetExitCodeProcess(process.hProcess, &exit);
    // 【逐行批注】 执行前一条件不成立时的分支。
    else { TerminateProcess(process.hProcess, 92); WaitForSingleObject(process.hProcess, 1000); }
    // 【逐行批注】 调用 CloseHandle 完成本行所需操作：CloseHandle(process.hThread); CloseHandle(process.hProcess);。
    CloseHandle(process.hThread); CloseHandle(process.hProcess);
    // 【逐行批注】 结束当前函数并返回结果：return completed && exit == 0;。
    return completed && exit == 0;
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 计算右侧结果并写入 auto child，更新当前运行状态。
    const auto child = fork();
    // 【逐行批注】 判断条件 (child < 0) return false;，成立时执行对应分支。
    if (child < 0) return false;
    // 【逐行批注】 判断条件 (!child) { execl(executable.c_str(), executable.c_str(), "--cras…，成立时执行对应分支。
    if (!child) { execl(executable.c_str(), executable.c_str(), "--crash-wal", path.c_str(), static_cast<char*>(nullptr)); std::_Exit(93); }
    // 【逐行批注】 计算右侧结果并写入 int status，更新当前运行状态。
    int status = 0;
    // 【逐行批注】 结束当前函数并返回结果：return waitpid(child, &status, 0) == child && WIFEXITED(status) && WE…。
    return waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0;
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 run，执行该函数声明的职责。
inline void run(const std::filesystem::path& root, const std::filesystem::path& executable) {
    // 【逐行批注】 执行当前声明或表达式：using gangyi::launcher::backupDatabase;。
    using gangyi::launcher::backupDatabase;
    // 【逐行批注】 执行当前声明或表达式：using gangyi::launcher::restoreDatabase;。
    using gangyi::launcher::restoreDatabase;
    // 【逐行批注】 计算右侧结果并写入 auto selected，更新当前运行状态。
    const auto selected = root / "restore-selected.db";
    // 【逐行批注】 调用 createDatabase、setCourse 完成本行所需操作：createDatabase(selected); setCourse(selected, "selected-backup");。
    createDatabase(selected); setCourse(selected, "selected-backup");
    // 【逐行批注】 执行当前声明或表达式：std::wstring error;。
    std::wstring error;

    // 【逐行批注】 计算右侧结果并写入 auto ordinary，更新当前运行状态。
    const auto ordinary = root / "restore-delete.db", ordinaryRollback = root / "rollback-delete.db";
    // 【逐行批注】 调用 createDatabase、setCourse 完成本行所需操作：createDatabase(ordinary); setCourse(ordinary, "delete-current");。
    createDatabase(ordinary); setCourse(ordinary, "delete-current");
    // 【逐行批注】 调用 expect、restoreDatabase 完成本行所需操作：expect(restoreDatabase(selected, ordinary, ordinaryRollback, error), …。
    expect(restoreDatabase(selected, ordinary, ordinaryRollback, error), "普通 DELETE 档案恢复成功");
    // 【逐行批注】 调用 expect、course 完成本行所需操作：expect(course(ordinary) == "selected-backup" && course(ordinaryRollba…。
    expect(course(ordinary) == "selected-backup" && course(ordinaryRollback) == "delete-current", "普通恢复保留原档案快照");

    // 【逐行批注】 计算右侧结果并写入 auto wal，更新当前运行状态。
    const auto wal = root / "restore-crash-wal.db", walRollback = root / "rollback-crash-wal.db";
    // 【逐行批注】 计算右侧结果并写入 bool crashed，更新当前运行状态。
    const bool crashed = runCrashWriter(executable, wal);
    // 【逐行批注】 调用 expect、exists 完成本行所需操作：expect(crashed && std::filesystem::exists(wal.string() + "-wal"), "异常…。
    expect(crashed && std::filesystem::exists(wal.string() + "-wal"), "异常退出留下已提交 WAL");
    // 【逐行批注】 判断条件 (crashed) {，成立时执行对应分支。
    if (crashed) {
        // 【逐行批注】 调用 expect、restoreDatabase 完成本行所需操作：expect(restoreDatabase(selected, wal, walRollback, error), "带遗留 WAL 的…。
        expect(restoreDatabase(selected, wal, walRollback, error), "带遗留 WAL 的档案安全恢复");
        // 【逐行批注】 调用 expect、course 完成本行所需操作：expect(course(wal) == "selected-backup", "旧 WAL 不得覆盖所选备份");。
        expect(course(wal) == "selected-backup", "旧 WAL 不得覆盖所选备份");
        // 【逐行批注】 调用 expect、course 完成本行所需操作：expect(course(walRollback) == "crash-current", "恢复前副本包含 WAL 中已提交的原记录"…。
        expect(course(walRollback) == "crash-current", "恢复前副本包含 WAL 中已提交的原记录");
        // 【逐行批注】 计算右侧结果并写入 auto secondRollback，更新当前运行状态。
        const auto secondRollback = root / "rollback-second.db";
        // 【逐行批注】 调用 expect、restoreDatabase 完成本行所需操作：expect(restoreDatabase(walRollback, wal, secondRollback, error), "恢复前…。
        expect(restoreDatabase(walRollback, wal, secondRollback, error), "恢复前副本可以再次恢复");
        // 【逐行批注】 调用 expect、course 完成本行所需操作：expect(course(wal) == "crash-current" && course(secondRollback) == "s…。
        expect(course(wal) == "crash-current" && course(secondRollback) == "selected-backup", "再次恢复保留两版合成记录");
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 计算右侧结果并写入 auto busy，更新当前运行状态。
    const auto busy = root / "restore-busy.db", busyRollback = root / "rollback-busy.db";
    // 【逐行批注】 调用 createDatabase、setCourse 完成本行所需操作：createDatabase(busy); setCourse(busy, "busy-current");。
    createDatabase(busy); setCourse(busy, "busy-current");
    // 【逐行批注】 计算右侧结果并写入 sqlite3* writer，更新当前运行状态。
    sqlite3* writer = nullptr;
    // 【逐行批注】 调用 expect、sqlite3_open 完成本行所需操作：expect(sqlite3_open(busy.u8string().c_str(), &writer) == SQLITE_OK, "…。
    expect(sqlite3_open(busy.u8string().c_str(), &writer) == SQLITE_OK, "打开合成 WAL 写连接");
    // 【逐行批注】 计算右侧结果并写入 expect(sql(writer, "PRAGMA journal_mode，更新当前运行状态。
    expect(sql(writer, "PRAGMA journal_mode=WAL;BEGIN IMMEDIATE;INSERT INTO Course VALUES('uncommitted');"), "保持活动 WAL 写事务");
    // 【逐行批注】 计算右侧结果并写入 auto began，更新当前运行状态。
    const auto began = std::chrono::steady_clock::now();
    // 【逐行批注】 调用 expect、restoreDatabase 完成本行所需操作：expect(!restoreDatabase(selected, busy, busyRollback, error), "活动 WAL…。
    expect(!restoreDatabase(selected, busy, busyRollback, error), "活动 WAL 写连接使恢复明确失败");
    // 【逐行批注】 调用 expect、now、seconds 完成本行所需操作：expect(std::chrono::steady_clock::now() - began < std::chrono::second…。
    expect(std::chrono::steady_clock::now() - began < std::chrono::seconds(5), "忙碌档案不会无限等待");
    // 【逐行批注】 调用 expect、exists 完成本行所需操作：expect(!std::filesystem::exists(busyRollback), "未取得独占锁时不创建恢复前副本");。
    expect(!std::filesystem::exists(busyRollback), "未取得独占锁时不创建恢复前副本");
    // 【逐行批注】 调用 expect、sql、sqlite3_close 完成本行所需操作：expect(sql(writer, "ROLLBACK"), "解除合成写事务"); sqlite3_close(writer);。
    expect(sql(writer, "ROLLBACK"), "解除合成写事务"); sqlite3_close(writer);
    // 【逐行批注】 调用 expect、course 完成本行所需操作：expect(course(busy) == "busy-current", "恢复被拒后原记录完整保留");。
    expect(course(busy) == "busy-current", "恢复被拒后原记录完整保留");

    // 【逐行批注】 计算右侧结果并写入 sqlite3* reader，更新当前运行状态。
    sqlite3* reader = nullptr;
    // 【逐行批注】 调用 expect、sqlite3_open 完成本行所需操作：expect(sqlite3_open(ordinary.u8string().c_str(), &reader) == SQLITE_O…。
    expect(sqlite3_open(ordinary.u8string().c_str(), &reader) == SQLITE_OK, "打开合成 DELETE 读连接");
    // 【逐行批注】 调用 expect、sql 完成本行所需操作：expect(sql(reader, "BEGIN;SELECT id FROM Course;"), "保持活动 DELETE 读事务"…。
    expect(sql(reader, "BEGIN;SELECT id FROM Course;"), "保持活动 DELETE 读事务");
    // 【逐行批注】 调用 expect、restoreDatabase 完成本行所需操作：expect(!restoreDatabase(selected, ordinary, root / "rollback-reader.d…。
    expect(!restoreDatabase(selected, ordinary, root / "rollback-reader.db", error), "活动读连接使恢复明确失败");
    // 【逐行批注】 调用 expect、backupDatabase 完成本行所需操作：expect(!backupDatabase(selected, ordinary, error), "活动目标连接使备份覆写明确失败");。
    expect(!backupDatabase(selected, ordinary, error), "活动目标连接使备份覆写明确失败");
    // 【逐行批注】 调用 expect、sql、sqlite3_close 完成本行所需操作：expect(sql(reader, "ROLLBACK"), "解除合成读事务"); sqlite3_close(reader);。
    expect(sql(reader, "ROLLBACK"), "解除合成读事务"); sqlite3_close(reader);
    // 【逐行批注】 调用 expect、course 完成本行所需操作：expect(course(ordinary) == "selected-backup", "忙碌备份覆写不损坏已有档案");。
    expect(course(ordinary) == "selected-backup", "忙碌备份覆写不损坏已有档案");

    // 【逐行批注】 计算右侧结果并写入 auto failed，更新当前运行状态。
    const auto failed = root / "restore-failed.db", occupiedRollback = root / "rollback-occupied.db";
    // 【逐行批注】 调用 createDatabase、setCourse 完成本行所需操作：createDatabase(failed); setCourse(failed, "must-stay");。
    createDatabase(failed); setCourse(failed, "must-stay");
    // 【逐行批注】 调用 createDatabase、setCourse 完成本行所需操作：createDatabase(occupiedRollback); setCourse(occupiedRollback, "prior-…。
    createDatabase(occupiedRollback); setCourse(occupiedRollback, "prior-snapshot");
    // 【逐行批注】 调用 expect、restoreDatabase 完成本行所需操作：expect(!restoreDatabase(selected, failed, occupiedRollback, error), "…。
    expect(!restoreDatabase(selected, failed, occupiedRollback, error), "不覆盖已有恢复前副本");
    // 【逐行批注】 调用 expect、course 完成本行所需操作：expect(course(failed) == "must-stay" && course(occupiedRollback) == "…。
    expect(course(failed) == "must-stay" && course(occupiedRollback) == "prior-snapshot", "回滚副本创建失败时两份原档案均保留");
    // 【逐行批注】 调用 expect、restoreDatabase 完成本行所需操作：expect(!restoreDatabase(selected, failed, selected, error), "拒绝恢复前副本与…。
    expect(!restoreDatabase(selected, failed, selected, error), "拒绝恢复前副本与所选备份同路径");
    // 【逐行批注】 调用 expect、restoreDatabase 完成本行所需操作：expect(!restoreDatabase(selected, failed, failed, error), "拒绝恢复前副本与目标…。
    expect(!restoreDatabase(selected, failed, failed, error), "拒绝恢复前副本与目标档案同路径");
    // 【逐行批注】 调用 expect、course 完成本行所需操作：expect(course(failed) == "must-stay" && course(selected) == "selected…。
    expect(course(failed) == "must-stay" && course(selected) == "selected-backup", "路径冲突不修改原档案或所选备份");

    // 【逐行批注】 计算右侧结果并写入 auto backupWal，更新当前运行状态。
    const auto backupWal = root / "backup-crash-wal.db";
    // 【逐行批注】 计算右侧结果并写入 bool backupCrashed，更新当前运行状态。
    const bool backupCrashed = runCrashWriter(executable, backupWal);
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(backupCrashed, "准备带遗留 WAL 的旧备份目标");。
    expect(backupCrashed, "准备带遗留 WAL 的旧备份目标");
    // 【逐行批注】 判断条件 (backupCrashed) {，成立时执行对应分支。
    if (backupCrashed) {
        // 【逐行批注】 调用 expect、backupDatabase 完成本行所需操作：expect(backupDatabase(selected, backupWal, error), "覆写旧备份也由 SQLite 安全…。
        expect(backupDatabase(selected, backupWal, error), "覆写旧备份也由 SQLite 安全处理 WAL");
        // 【逐行批注】 调用 expect、course 完成本行所需操作：expect(course(backupWal) == "selected-backup", "备份覆写结果不被旧 WAL 覆盖");。
        expect(course(backupWal) == "selected-backup", "备份覆写结果不被旧 WAL 覆盖");
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

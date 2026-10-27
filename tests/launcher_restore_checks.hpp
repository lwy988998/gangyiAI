#pragma once
#include <chrono>
#include <cstdlib>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace launcher_restore_checks {
inline bool sql(sqlite3* database, const char* statement) {
    return sqlite3_exec(database, statement, nullptr, nullptr, nullptr) == SQLITE_OK;
}

inline void setCourse(const std::filesystem::path& path, const char* marker) {
    sqlite3* database = nullptr;
    expect(sqlite3_open(path.u8string().c_str(), &database) == SQLITE_OK, "打开合成档案");
    expect(sql(database, "DELETE FROM Course"), "准备合成课程标记");
    sqlite3_stmt* statement = nullptr;
    const bool prepared = sqlite3_prepare_v2(database, "INSERT INTO Course VALUES(?)", -1, &statement, nullptr) == SQLITE_OK;
    expect(prepared, "准备合成课程写入");
    if (prepared) {
        sqlite3_bind_text(statement, 1, marker, -1, SQLITE_TRANSIENT);
        expect(sqlite3_step(statement) == SQLITE_DONE, "保存合成课程标记");
    }
    sqlite3_finalize(statement); sqlite3_close(database);
}

inline std::string course(const std::filesystem::path& path) {
    sqlite3* database = nullptr;
    if (sqlite3_open_v2(path.u8string().c_str(), &database, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        if (database) sqlite3_close(database); return {};
    }
    sqlite3_stmt* statement = nullptr; std::string value;
    if (sqlite3_prepare_v2(database, "SELECT id FROM Course", -1, &statement, nullptr) == SQLITE_OK && sqlite3_step(statement) == SQLITE_ROW)
        value = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
    sqlite3_finalize(statement); sqlite3_close(database); return value;
}

inline int crashWal(const std::filesystem::path& path) {
    createDatabase(path);
    sqlite3* database = nullptr;
    if (sqlite3_open(path.u8string().c_str(), &database) != SQLITE_OK ||
        !sql(database, "PRAGMA journal_mode=WAL;DELETE FROM Course;INSERT INTO Course VALUES('crash-current');")) return 91;
    // 模拟进程异常退出，跳过析构和 sqlite3_close，保留已提交但未检查点的真实 WAL。
    std::_Exit(0);
}

inline bool runCrashWriter(const std::filesystem::path& executable, const std::filesystem::path& path) {
#ifdef _WIN32
    (void)executable;
    wchar_t module[MAX_PATH]{};
    if (!GetModuleFileNameW(nullptr, module, MAX_PATH)) return false;
    auto command = L"\"" + std::wstring(module) + L"\" --crash-wal \"" + path.wstring() + L"\"";
    STARTUPINFOW startup{}; startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(module, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process)) return false;
    const bool completed = WaitForSingleObject(process.hProcess, 10000) == WAIT_OBJECT_0;
    DWORD exit = 1;
    if (completed) GetExitCodeProcess(process.hProcess, &exit);
    else { TerminateProcess(process.hProcess, 92); WaitForSingleObject(process.hProcess, 1000); }
    CloseHandle(process.hThread); CloseHandle(process.hProcess);
    return completed && exit == 0;
#else
    const auto child = fork();
    if (child < 0) return false;
    if (!child) { execl(executable.c_str(), executable.c_str(), "--crash-wal", path.c_str(), static_cast<char*>(nullptr)); std::_Exit(93); }
    int status = 0;
    return waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0;
#endif
}

inline void run(const std::filesystem::path& root, const std::filesystem::path& executable) {
    using gangyi::launcher::backupDatabase;
    using gangyi::launcher::restoreDatabase;
    const auto selected = root / "restore-selected.db";
    createDatabase(selected); setCourse(selected, "selected-backup");
    std::wstring error;

    const auto ordinary = root / "restore-delete.db", ordinaryRollback = root / "rollback-delete.db";
    createDatabase(ordinary); setCourse(ordinary, "delete-current");
    expect(restoreDatabase(selected, ordinary, ordinaryRollback, error), "普通 DELETE 档案恢复成功");
    expect(course(ordinary) == "selected-backup" && course(ordinaryRollback) == "delete-current", "普通恢复保留原档案快照");

    const auto wal = root / "restore-crash-wal.db", walRollback = root / "rollback-crash-wal.db";
    const bool crashed = runCrashWriter(executable, wal);
    expect(crashed && std::filesystem::exists(wal.string() + "-wal"), "异常退出留下已提交 WAL");
    if (crashed) {
        expect(restoreDatabase(selected, wal, walRollback, error), "带遗留 WAL 的档案安全恢复");
        expect(course(wal) == "selected-backup", "旧 WAL 不得覆盖所选备份");
        expect(course(walRollback) == "crash-current", "恢复前副本包含 WAL 中已提交的原记录");
        const auto secondRollback = root / "rollback-second.db";
        expect(restoreDatabase(walRollback, wal, secondRollback, error), "恢复前副本可以再次恢复");
        expect(course(wal) == "crash-current" && course(secondRollback) == "selected-backup", "再次恢复保留两版合成记录");
    }

    const auto busy = root / "restore-busy.db", busyRollback = root / "rollback-busy.db";
    createDatabase(busy); setCourse(busy, "busy-current");
    sqlite3* writer = nullptr;
    expect(sqlite3_open(busy.u8string().c_str(), &writer) == SQLITE_OK, "打开合成 WAL 写连接");
    expect(sql(writer, "PRAGMA journal_mode=WAL;BEGIN IMMEDIATE;INSERT INTO Course VALUES('uncommitted');"), "保持活动 WAL 写事务");
    const auto began = std::chrono::steady_clock::now();
    expect(!restoreDatabase(selected, busy, busyRollback, error), "活动 WAL 写连接使恢复明确失败");
    expect(std::chrono::steady_clock::now() - began < std::chrono::seconds(5), "忙碌档案不会无限等待");
    expect(!std::filesystem::exists(busyRollback), "未取得独占锁时不创建恢复前副本");
    expect(sql(writer, "ROLLBACK"), "解除合成写事务"); sqlite3_close(writer);
    expect(course(busy) == "busy-current", "恢复被拒后原记录完整保留");

    sqlite3* reader = nullptr;
    expect(sqlite3_open(ordinary.u8string().c_str(), &reader) == SQLITE_OK, "打开合成 DELETE 读连接");
    expect(sql(reader, "BEGIN;SELECT id FROM Course;"), "保持活动 DELETE 读事务");
    expect(!restoreDatabase(selected, ordinary, root / "rollback-reader.db", error), "活动读连接使恢复明确失败");
    expect(!backupDatabase(selected, ordinary, error), "活动目标连接使备份覆写明确失败");
    expect(sql(reader, "ROLLBACK"), "解除合成读事务"); sqlite3_close(reader);
    expect(course(ordinary) == "selected-backup", "忙碌备份覆写不损坏已有档案");

    const auto failed = root / "restore-failed.db", occupiedRollback = root / "rollback-occupied.db";
    createDatabase(failed); setCourse(failed, "must-stay");
    createDatabase(occupiedRollback); setCourse(occupiedRollback, "prior-snapshot");
    expect(!restoreDatabase(selected, failed, occupiedRollback, error), "不覆盖已有恢复前副本");
    expect(course(failed) == "must-stay" && course(occupiedRollback) == "prior-snapshot", "回滚副本创建失败时两份原档案均保留");
    expect(!restoreDatabase(selected, failed, selected, error), "拒绝恢复前副本与所选备份同路径");
    expect(!restoreDatabase(selected, failed, failed, error), "拒绝恢复前副本与目标档案同路径");
    expect(course(failed) == "must-stay" && course(selected) == "selected-backup", "路径冲突不修改原档案或所选备份");

    const auto backupWal = root / "backup-crash-wal.db";
    const bool backupCrashed = runCrashWriter(executable, backupWal);
    expect(backupCrashed, "准备带遗留 WAL 的旧备份目标");
    if (backupCrashed) {
        expect(backupDatabase(selected, backupWal, error), "覆写旧备份也由 SQLite 安全处理 WAL");
        expect(course(backupWal) == "selected-backup", "备份覆写结果不被旧 WAL 覆盖");
    }
}
}

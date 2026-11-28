// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 "db_schema_version.hpp"，为后续实现提供类型或接口声明。
#include "db_schema_version.hpp"

// 【逐行批注】 引入头文件 <sqlite3.h>，为后续实现提供类型或接口声明。
#include <sqlite3.h>
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
#include <atomic>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <random>，为后续实现提供类型或接口声明。
#include <random>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 定义函数 check，执行该函数声明的职责。
void check(int rc, sqlite3* db, const char* action) { if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw std::runtime_error(std::string(action) + ": " + sqlite3_errmsg(db)); }
// 【逐行批注】 定义函数 exec，执行该函数声明的职责。
void exec(sqlite3* db, const char* sql) { char* error = nullptr; const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &error); if (rc != SQLITE_OK) { std::string message = error ? error : sqlite3_errmsg(db); sqlite3_free(error); throw std::runtime_error(message); } }
// 【逐行批注】 定义函数 id，执行该函数声明的职责。
std::string id() { static thread_local std::mt19937_64 rng(std::random_device{}()); std::ostringstream out; out << std::hex << std::setfill('0'); for (int i = 0; i < 2; ++i) out << std::setw(16) << rng(); return out.str(); }
// 【逐行批注】 声明数据结构或类型 Stmt 及其成员接口。
struct Stmt { sqlite3_stmt* p = nullptr; sqlite3* db; Stmt(sqlite3* d, const char* sql) : db(d) { check(sqlite3_prepare_v2(db, sql, -1, &p, nullptr), db, "prepare"); } ~Stmt() { sqlite3_finalize(p); } };
// SQLite 的连接互斥量可重入；跨整个事务持有，避免同连接的另一请求混入保存点。
struct ConnectionGuard {
    sqlite3_mutex* mutex;
    explicit ConnectionGuard(sqlite3* db) : mutex(sqlite3_db_mutex(db)) { sqlite3_mutex_enter(mutex); }
    ~ConnectionGuard() { sqlite3_mutex_leave(mutex); }
    ConnectionGuard(const ConnectionGuard&) = delete;
    ConnectionGuard& operator=(const ConnectionGuard&) = delete;
};
// 【逐行批注】 定义函数 text，执行该函数声明的职责。
void text(Stmt& s, int n, const std::string& v) { check(sqlite3_bind_text(s.p, n, v.c_str(), -1, SQLITE_TRANSIENT), s.db, "bind"); }
// 【逐行批注】 定义函数 opt，执行该函数声明的职责。
void opt(Stmt& s, int n, const std::optional<std::string>& v) { if (v) text(s, n, *v); else check(sqlite3_bind_null(s.p, n), s.db, "bind"); }
// 【逐行批注】 定义函数 integer，执行该函数声明的职责。
void integer(Stmt& s, int n, int v) { check(sqlite3_bind_int(s.p, n, v), s.db, "bind"); }
// 【逐行批注】 定义函数 optint，执行该函数声明的职责。
void optint(Stmt& s, int n, const std::optional<int>& v) { if (v) integer(s, n, *v); else check(sqlite3_bind_null(s.p, n), s.db, "bind"); }
// 【逐行批注】 定义函数 str，执行该函数声明的职责。
std::string str(sqlite3_stmt* s, int n) { const auto* p = sqlite3_column_text(s, n); return p ? reinterpret_cast<const char*>(p) : ""; }
// 【逐行批注】 定义函数 ostr，执行该函数声明的职责。
std::optional<std::string> ostr(sqlite3_stmt* s, int n) { const auto* p = sqlite3_column_text(s, n); return p ? std::optional<std::string>(reinterpret_cast<const char*>(p)) : std::nullopt; }
// 【逐行批注】 定义函数 oint，执行该函数声明的职责。
std::optional<int> oint(sqlite3_stmt* s, int n) { return sqlite3_column_type(s, n) == SQLITE_NULL ? std::nullopt : std::optional<int>(sqlite3_column_int(s, n)); }
// 【逐行批注】 定义函数 done，执行该函数声明的职责。
bool done(Stmt& s) { const int rc = sqlite3_step(s.p); check(rc, s.db, "execute"); return sqlite3_changes(s.db) > 0; }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 调用 Database、close 完成本行所需操作：Database::~Database() { close(); }。
Database::~Database() { close(); }
// 【逐行批注】 定义函数 Database::deleteInteractionsForConversation，执行该函数声明的职责。
void Database::deleteInteractionsForConversation(const std::string& key) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_,"DELETE FROM LearningInteraction WHERE conversationId=?");text(s,1,key);done(s);
    // 【逐行批注】 声明对象 events，并通过构造函数初始化其状态。
    Stmt events(db_, "DELETE FROM AgentRunEvent WHERE taskId IN (SELECT id FROM ClassroomActivity "
                     // 【逐行批注】 计算右侧结果并写入 "WHERE kind，更新当前运行状态。
                     "WHERE kind='agent-task' AND CASE WHEN json_valid(payload) THEN "
                     // 【逐行批注】 计算右侧结果并写入 "json_extract(payload,'$.event.chatId') ELSE…，更新当前运行状态。
                     "json_extract(payload,'$.event.chatId') ELSE NULL END=?)");
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(events,1,key);done(events);。
    text(events,1,key);done(events);
    // 【逐行批注】 声明对象 tasks，并通过构造函数初始化其状态。
    Stmt tasks(db_,"DELETE FROM ClassroomActivity WHERE kind='agent-task' AND CASE WHEN json_valid(payload) THEN json_extract(payload,'$.event.chatId') ELSE NULL END=?");text(tasks,1,key);done(tasks);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::myAIRecord，执行该函数声明的职责。
std::optional<MyAIRecord> Database::myAIRecord(const std::string& key) const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT id,kind,payload,updatedAt,version FROM MyAIRecord WHERE id=?"); text(s,1,key);
    // 【逐行批注】 判断条件 (sqlite3_step(s.p)!=SQLITE_ROW) return {};，成立时执行对应分支。
    if (sqlite3_step(s.p)!=SQLITE_ROW) return {};
    // 【逐行批注】 结束当前函数并返回结果：return MyAIRecord{str(s.p,0),str(s.p,1),str(s.p,2),str(s.p,3),sqlite3…。
    return MyAIRecord{str(s.p,0),str(s.p,1),str(s.p,2),str(s.p,3),sqlite3_column_int(s.p,4)};
// 【逐行批注】 结束当前代码块。
}
bool Database::myAIRecordExists(const std::string& id, const std::string& kind) const {
    Stmt statement(db_, "SELECT 1 FROM MyAIRecord WHERE id=? AND kind=?");
    text(statement, 1, id);
    text(statement, 2, kind);
    return sqlite3_step(statement.p) == SQLITE_ROW;
}
// 【逐行批注】 定义函数 Database::myAIRecords，执行该函数声明的职责。
std::vector<MyAIRecord> Database::myAIRecords(const std::string& kind) const {
    // 【逐行批注】 调用 s、text 完成本行所需操作：std::vector<MyAIRecord> result; Stmt s(db_, "SELECT id,kind,payload,u…。
    std::vector<MyAIRecord> result; Stmt s(db_, "SELECT id,kind,payload,updatedAt,version FROM MyAIRecord WHERE kind=? ORDER BY updatedAt DESC,id"); text(s,1,kind);
    // 【逐行批注】 在条件成立期间循环处理：while (sqlite3_step(s.p)==SQLITE_ROW) result.push_back({str(s.p,0),str(s.p…。
    while (sqlite3_step(s.p)==SQLITE_ROW) result.push_back({str(s.p,0),str(s.p,1),str(s.p,2),str(s.p,3),sqlite3_column_int(s.p,4)});
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::saveMyAIRecord，执行该函数声明的职责。
bool Database::saveMyAIRecord(const MyAIRecord& row, int expected) {
    // 【逐行批注】 判断条件 (expected < 0) throw std::invalid_argument("记录版本无效");，成立时执行对应分支。
    if (expected < 0) throw std::invalid_argument("记录版本无效");
    // 【逐行批注】 判断条件 (!expected) {，成立时执行对应分支。
    if (!expected) {
        // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
        Stmt s(db_, "INSERT OR IGNORE INTO MyAIRecord(id,kind,payload,updatedAt,version) VALUES(?,?,?,?,1)");
        // 【逐行批注】 调用 text、done 完成本行所需操作：text(s,1,row.id);text(s,2,row.kind);text(s,3,row.payload);text(s,4,ro…。
        text(s,1,row.id);text(s,2,row.kind);text(s,3,row.payload);text(s,4,row.updatedAt);return done(s);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "UPDATE MyAIRecord SET payload=?,updatedAt=?,version=version+1 WHERE id=? AND kind=? AND version=?");
    // 【逐行批注】 调用 text、integer、done 完成本行所需操作：text(s,1,row.payload);text(s,2,row.updatedAt);text(s,3,row.id);text(s…。
    text(s,1,row.payload);text(s,2,row.updatedAt);text(s,3,row.id);text(s,4,row.kind);integer(s,5,expected);return done(s);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::myAIContextVersions，执行该函数声明的职责。
std::string Database::myAIContextVersions(const std::string& chatId) const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT id,kind,version FROM MyAIRecord WHERE kind IN ('profile','preferences','memory') OR (kind='chat' AND id=?) ORDER BY kind,id");
    // 【逐行批注】 调用 text 完成本行所需操作：text(s, 1, chatId);。
    text(s, 1, chatId);
    // 【逐行批注】 计算右侧结果并写入 versions，更新当前运行状态。
    auto versions = nlohmann::json::array();
    // 【逐行批注】 在条件成立期间循环处理：while (sqlite3_step(s.p) == SQLITE_ROW) versions.push_back({str(s.p, 0), s…。
    while (sqlite3_step(s.p) == SQLITE_ROW) versions.push_back({str(s.p, 0), str(s.p, 1), sqlite3_column_int(s.p, 2)});
    // 【逐行批注】 结束当前函数并返回结果：return versions.dump();。
    return versions.dump();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::readSnapshot，执行该函数声明的职责。
void Database::readSnapshot(const std::function<void()>& action) {
    ConnectionGuard guard(db_);
    // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "BEGIN");。
    exec(db_, "BEGIN");
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { action(); exec(db_, "COMMIT"); }
    // 【逐行批注】 捕获并处理异常：catch (...) { sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr…。
    catch (...) { sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr); throw; }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::transaction，执行该函数声明的职责。
void Database::transaction(const std::function<void()>& action) {
    ConnectionGuard guard(db_);
    // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "BEGIN IMMEDIATE");。
    exec(db_, "BEGIN IMMEDIATE");
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 调用 action 完成本行所需操作：action();。
        action();
        // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "COMMIT");。
        exec(db_, "COMMIT");
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 调用 sqlite3_exec 完成本行所需操作：sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);。
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        // 【逐行批注】 检测到错误情形时抛出异常：throw;。
        throw;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}
// 保存点既可作为独立原子单元，也可嵌入同线程已有的写事务。
void Database::savepoint(const std::function<void()>& action) {
    ConnectionGuard guard(db_);
    static std::atomic<unsigned long long> sequence{0};
    const auto name = "gangyi_operation_" + std::to_string(++sequence);
    exec(db_, ("SAVEPOINT " + name).c_str());
    try {
        action();
        exec(db_, ("RELEASE SAVEPOINT " + name).c_str());
    } catch (...) {
        // 嵌入确认事务时不能提交或撤销调用者的其它步骤。
        const int rolledBack = sqlite3_exec(db_, ("ROLLBACK TO SAVEPOINT " + name).c_str(), nullptr, nullptr, nullptr);
        if (rolledBack == SQLITE_OK)
            sqlite3_exec(db_, ("RELEASE SAVEPOINT " + name).c_str(), nullptr, nullptr, nullptr);
        throw;
    }
}

int Database::learningRevision() const {
    // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
    const auto value = profileMeta("learning-revision");
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { return value.empty() ? 1 : std::stoi(value); } catch (...) { return 1; }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::markLearningDirty，执行该函数声明的职责。
void Database::markLearningDirty() {
    // 【逐行批注】 计算右侧结果并写入 exec(db_, "INSERT INTO ProfileMeta(key,value…，更新当前运行状态。
    exec(db_, "INSERT INTO ProfileMeta(key,value) VALUES('learning-revision','2') ON CONFLICT(key) DO UPDATE SET value=CAST(value AS INTEGER)+1");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::compareProfileMeta，执行该函数声明的职责。
bool Database::compareProfileMeta(const std::string& key, const std::string& expected, const std::string& value) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO ProfileMeta(key,value) SELECT ?,? WHERE COALESCE((SELECT value FROM ProfileMeta WHERE key=?),'')=? ON CONFLICT(key) DO UPDATE SET value=excluded.value WHERE ProfileMeta.value=? RETURNING key");
    // 【逐行批注】 调用 text 完成本行所需操作：text(s,1,key);text(s,2,value);text(s,3,key);text(s,4,expected);text(s…。
    text(s,1,key);text(s,2,value);text(s,3,key);text(s,4,expected);text(s,5,expected);
    // 【逐行批注】 计算右侧结果并写入 int result，更新当前运行状态。
    const int result = sqlite3_step(s.p); check(result, db_, "compare and save"); return result == SQLITE_ROW;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 调用 compareClassroomActivity 完成本行所需操作：bool Database::compareClassroomActivity(const ClassroomActivity& valu…。
bool Database::compareClassroomActivity(const ClassroomActivity& value, const std::string& expected,
                                        // 【逐行批注】 执行当前声明或表达式：const std::string& guardId, const std::string& guardPayload) {。
                                        const std::string& guardId, const std::string& guardPayload) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO ClassroomActivity(id,courseId,phaseIndex,topicIndex,kind,payload,updatedAt) SELECT ?,?,?,?,?,?,? WHERE COALESCE((SELECT payload FROM ClassroomActivity WHERE id=?),'')=? AND EXISTS(SELECT 1 FROM Course WHERE id=? AND status='active') AND (?='' OR EXISTS(SELECT 1 FROM ClassroomActivity WHERE id=? AND payload=?)) ON CONFLICT(id) DO UPDATE SET payload=excluded.payload,updatedAt=excluded.updatedAt WHERE ClassroomActivity.payload=? RETURNING id");
    // 【逐行批注】 调用 text、integer 完成本行所需操作：text(s,1,value.id);text(s,2,value.courseId);integer(s,3,value.phaseIn…。
    text(s,1,value.id);text(s,2,value.courseId);integer(s,3,value.phaseIndex);integer(s,4,value.topicIndex);
    // 【逐行批注】 调用 text 完成本行所需操作：text(s,5,value.kind);text(s,6,value.payload);text(s,7,value.updatedAt…。
    text(s,5,value.kind);text(s,6,value.payload);text(s,7,value.updatedAt);text(s,8,value.id);
    // 【逐行批注】 调用 text 完成本行所需操作：text(s,9,expected);text(s,10,value.courseId);text(s,11,guardId);text(…。
    text(s,9,expected);text(s,10,value.courseId);text(s,11,guardId);text(s,12,guardId);text(s,13,guardPayload);text(s,14,expected);
    // 【逐行批注】 计算右侧结果并写入 int result，更新当前运行状态。
    const int result = sqlite3_step(s.p); check(result, db_, "compare and save"); return result == SQLITE_ROW;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 调用 updateLearningSessionAtRevision 完成本行所需操作：bool Database::updateLearningSessionAtRevision(const LearningSession&…。
bool Database::updateLearningSessionAtRevision(const LearningSession& value, const std::string& expected,
                                               // 【逐行批注】 执行当前声明或表达式：int revision, const std::string& exposureKey,。
                                               int revision, const std::string& exposureKey,
                                               // 【逐行批注】 执行当前声明或表达式：const std::string& exposure) {。
                                               const std::string& exposure) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "UPDATE LearningSession SET content=?,title=?,summary=?,`references`=? WHERE id=? AND content=? AND COALESCE((SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='learning-revision'),1)=? AND COALESCE((SELECT value FROM ProfileMeta WHERE key=?),'')=? AND EXISTS(SELECT 1 FROM Course WHERE id=LearningSession.courseId AND status='active') RETURNING id");
    // 【逐行批注】 调用 text、opt 完成本行所需操作：text(s,1,value.content);text(s,2,value.title);opt(s,3,value.summary);…。
    text(s,1,value.content);text(s,2,value.title);opt(s,3,value.summary);opt(s,4,value.references);
    // 【逐行批注】 调用 text、integer 完成本行所需操作：text(s,5,value.id);text(s,6,expected);integer(s,7,revision);text(s,8,…。
    text(s,5,value.id);text(s,6,expected);integer(s,7,revision);text(s,8,exposureKey);text(s,9,exposure);
    // 【逐行批注】 计算右侧结果并写入 int result，更新当前运行状态。
    const int result = sqlite3_step(s.p); check(result, db_, "compare and save"); return result == SQLITE_ROW;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::open，执行该函数声明的职责。
void Database::open(const std::string& path) {
    // 【逐行批注】 调用 close 完成本行所需操作：close();。
    close();
    // 【逐行批注】 调用 check、sqlite3_open 完成本行所需操作：check(sqlite3_open(path.c_str(), &db_), db_, "open database");。
    check(sqlite3_open(path.c_str(), &db_), db_, "open database");
    // 【逐行批注】 调用 sqlite3_extended_result_codes 完成本行所需操作：sqlite3_extended_result_codes(db_, 1);。
    sqlite3_extended_result_codes(db_, 1);
    // 【逐行批注】 调用 sqlite3_busy_timeout 完成本行所需操作：sqlite3_busy_timeout(db_, 5000);。
    sqlite3_busy_timeout(db_, 5000);
    // 两个工作线程各自持有连接；WAL 让读取不被长事务阻塞，写入仍由 SQLite 串行提交。
    // 【逐行批注】 计算右侧结果并写入 exec(db_, "PRAGMA journal_mode，更新当前运行状态。
    exec(db_, "PRAGMA journal_mode=WAL;PRAGMA synchronous=NORMAL;PRAGMA foreign_keys=ON;");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::close，执行该函数声明的职责。
void Database::close() { if (db_) { sqlite3_close(db_); db_ = nullptr; } }
// 【逐行批注】 定义函数 Database::migrate，执行该函数声明的职责。
void Database::migrate() {
    // 【逐行批注】 判断条件 (!db_) throw std::runtime_error("database is not open");，成立时执行对应分支。
    if (!db_) throw std::runtime_error("database is not open");
    // 【逐行批注】 计算右侧结果并写入 sqlite3_stmt* versionStatement，更新当前运行状态。
    sqlite3_stmt* versionStatement = nullptr;
    // 【逐行批注】 调用 check、sqlite3_prepare_v2 完成本行所需操作：check(sqlite3_prepare_v2(db_, "PRAGMA user_version", -1, &versionStat…。
    check(sqlite3_prepare_v2(db_, "PRAGMA user_version", -1, &versionStatement, nullptr), db_, "read schema version");
    // 【逐行批注】 计算右侧结果并写入 int versionResult，更新当前运行状态。
    const int versionResult = sqlite3_step(versionStatement);
    // 【逐行批注】 计算右侧结果并写入 int currentVersion，更新当前运行状态。
    const int currentVersion = versionResult == SQLITE_ROW ? sqlite3_column_int(versionStatement, 0) : -1;
    // 【逐行批注】 调用 sqlite3_finalize 完成本行所需操作：sqlite3_finalize(versionStatement);。
    sqlite3_finalize(versionStatement);
    // 【逐行批注】 判断条件 (versionResult != SQLITE_ROW || currentVersion > kDatabaseSchema…，成立时执行对应分支。
    if (versionResult != SQLITE_ROW || currentVersion > kDatabaseSchemaVersion) {
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("database schema is newer than this applicat…。
        throw std::runtime_error("database schema is newer than this application");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (currentVersion > 0 && currentVersion < kDatabaseSchemaVersion) {，成立时执行对应分支。
    if (currentVersion > 0 && currentVersion < kDatabaseSchemaVersion) {
        // 【逐行批注】 计算右侧结果并写入 char* filename，更新当前运行状态。
        const char* filename = sqlite3_db_filename(db_, "main");
        // 【逐行批注】 判断条件 (filename && *filename && std::string(filename) != ":memory:") {，成立时执行对应分支。
        if (filename && *filename && std::string(filename) != ":memory:") {
            // 备份后缀记录本次迁移要到达的版本，既避免在升级过程中覆盖更早的基线，
            // 也让 v3/v4/v5 等历史版本升级后的回滚文件仍可被原测试与运维流程识别。
            // 【逐行批注】 计算右侧结果并写入 auto backupPath，更新当前运行状态。
            const auto backupPath = std::filesystem::u8path(filename).u8string() +
                // 【逐行批注】 执行当前声明或表达式：(currentVersion < 4 ? ".pre-v4.db" : currentVersion < 5 ? ".pre-v5.db" :。
                (currentVersion < 4 ? ".pre-v4.db" : currentVersion < 5 ? ".pre-v5.db" :
                 // 【逐行批注】 执行当前声明或表达式：currentVersion < 6 ? ".pre-v6.db" : currentVersion < 7 ? ".pre-v7.db" :。
                 currentVersion < 6 ? ".pre-v6.db" : currentVersion < 7 ? ".pre-v7.db" :
                 // 【逐行批注】 执行当前声明或表达式：currentVersion < 8 ? ".pre-v8.db" : ".pre-v9.db");。
                 currentVersion < 8 ? ".pre-v8.db" : ".pre-v9.db");
            // 【逐行批注】 判断条件 (!std::filesystem::exists(std::filesystem::u8path(backupPath))) {，成立时执行对应分支。
            if (!std::filesystem::exists(std::filesystem::u8path(backupPath))) {
                // 【逐行批注】 计算右侧结果并写入 sqlite3* backupDb，更新当前运行状态。
                sqlite3* backupDb = nullptr;
                // 【逐行批注】 调用 check、sqlite3_open 完成本行所需操作：check(sqlite3_open(backupPath.c_str(), &backupDb), backupDb, "open mi…。
                check(sqlite3_open(backupPath.c_str(), &backupDb), backupDb, "open migration backup");
                // 【逐行批注】 计算右侧结果并写入 sqlite3_backup* backup，更新当前运行状态。
                sqlite3_backup* backup = sqlite3_backup_init(backupDb, "main", db_, "main");
                // 【逐行批注】 判断条件 (!backup) { sqlite3_close(backupDb); throw std::runtime_error("m…，成立时执行对应分支。
                if (!backup) { sqlite3_close(backupDb); throw std::runtime_error("migration backup failed"); }
                // 【逐行批注】 计算右侧结果并写入 int result，更新当前运行状态。
                const int result = sqlite3_backup_step(backup, -1);
                // 【逐行批注】 计算右侧结果并写入 int finishResult，更新当前运行状态。
                const int finishResult = sqlite3_backup_finish(backup);
                // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(backupDb);。
                sqlite3_close(backupDb);
                // 【逐行批注】 判断条件 (result != SQLITE_DONE || finishResult != SQLITE_OK) {，成立时执行对应分支。
                if (result != SQLITE_DONE || finishResult != SQLITE_OK) {
                    // 【逐行批注】 执行当前声明或表达式：std::error_code ignored;。
                    std::error_code ignored;
                    // 【逐行批注】 调用 remove、u8path 完成本行所需操作：std::filesystem::remove(std::filesystem::u8path(backupPath), ignored);。
                    std::filesystem::remove(std::filesystem::u8path(backupPath), ignored);
                    // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("migration backup failed");。
                    throw std::runtime_error("migration backup failed");
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 exec、SQL 完成本行所需操作：exec(db_, R"SQL(。
    exec(db_, R"SQL(
CREATE TABLE IF NOT EXISTS Course(id TEXT PRIMARY KEY, anonymousId TEXT, userId TEXT, goal TEXT NOT NULL, mode TEXT NOT NULL, title TEXT NOT NULL, summary TEXT, source TEXT NOT NULL DEFAULT 'ai', status TEXT NOT NULL DEFAULT 'active', createdAt TEXT NOT NULL, updatedAt TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS User(id TEXT PRIMARY KEY, email TEXT NOT NULL UNIQUE, name TEXT, passwordHash TEXT NOT NULL, membershipTier TEXT NOT NULL DEFAULT 'free', membershipStatus TEXT NOT NULL DEFAULT 'active', membershipStartedAt TEXT, membershipExpiresAt TEXT, createdAt TEXT NOT NULL, updatedAt TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS UserSession(id TEXT PRIMARY KEY, userId TEXT NOT NULL, tokenHash TEXT NOT NULL UNIQUE, createdAt TEXT NOT NULL, expiresAt TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS UsageCounter(id TEXT PRIMARY KEY, scopeId TEXT NOT NULL, scopeType TEXT NOT NULL, type TEXT NOT NULL, dateKey TEXT NOT NULL, count INTEGER NOT NULL DEFAULT 0, UNIQUE(scopeId,scopeType,type,dateKey));
CREATE TABLE IF NOT EXISTS CourseProgress(id TEXT PRIMARY KEY, courseId TEXT NOT NULL UNIQUE, anonymousId TEXT, userId TEXT, goal TEXT NOT NULL, mode TEXT, overallPercent INTEGER NOT NULL DEFAULT 0, completedCount INTEGER NOT NULL DEFAULT 0, totalCount INTEGER NOT NULL DEFAULT 0, lastVisitedUrl TEXT, lastPageType TEXT, lastPhaseIndex INTEGER, lastPhaseName TEXT, lastTopicIndex INTEGER, lastTopicTitle TEXT, updatedAt TEXT NOT NULL, createdAt TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS CourseSnapshot(id TEXT PRIMARY KEY, courseId TEXT NOT NULL, version INTEGER NOT NULL DEFAULT 1, payload TEXT NOT NULL, createdAt TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS TaskProgress(id TEXT PRIMARY KEY, courseId TEXT, anonymousId TEXT, goal TEXT NOT NULL, mode TEXT, phaseIndex INTEGER NOT NULL, phaseName TEXT NOT NULL, taskIndex INTEGER NOT NULL, taskTitle TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'not_started', UNIQUE(courseId,phaseIndex,taskIndex));
CREATE TABLE IF NOT EXISTS LearningStepProgress(id TEXT PRIMARY KEY, courseId TEXT, anonymousId TEXT, goal TEXT NOT NULL, mode TEXT, phaseIndex INTEGER NOT NULL, phaseName TEXT NOT NULL, stepIndex INTEGER NOT NULL, stepTitle TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'not_started', UNIQUE(courseId,phaseIndex,stepIndex));
CREATE TABLE IF NOT EXISTS LearningCardProgress(id TEXT PRIMARY KEY, courseId TEXT, anonymousId TEXT, goal TEXT NOT NULL, mode TEXT, phaseIndex INTEGER NOT NULL, phaseName TEXT NOT NULL, topicIndex INTEGER NOT NULL, topicTitle TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'not_started', UNIQUE(courseId,phaseIndex,topicIndex));
CREATE TABLE IF NOT EXISTS LearningSession(id TEXT PRIMARY KEY, courseId TEXT, anonymousId TEXT, goal TEXT NOT NULL, mode TEXT, phaseIndex INTEGER NOT NULL, phaseName TEXT NOT NULL, topicIndex INTEGER NOT NULL, topicTitle TEXT NOT NULL, title TEXT NOT NULL, summary TEXT, searchQuery TEXT, content TEXT NOT NULL, `references` TEXT, fallbackUsed INTEGER NOT NULL DEFAULT 0, source TEXT NOT NULL DEFAULT 'ai', UNIQUE(courseId,phaseIndex,topicIndex));
CREATE INDEX IF NOT EXISTS Course_anonymousId_idx ON Course(anonymousId); CREATE INDEX IF NOT EXISTS Course_userId_idx ON Course(userId); CREATE INDEX IF NOT EXISTS Course_goal_idx ON Course(goal); CREATE INDEX IF NOT EXISTS Course_updatedAt_idx ON Course(updatedAt);
CREATE INDEX IF NOT EXISTS UserSession_userId_idx ON UserSession(userId); CREATE INDEX IF NOT EXISTS UsageCounter_scope_idx ON UsageCounter(scopeId,scopeType); CREATE INDEX IF NOT EXISTS CourseProgress_anonymousId_idx ON CourseProgress(anonymousId); CREATE INDEX IF NOT EXISTS CourseProgress_userId_idx ON CourseProgress(userId); CREATE INDEX IF NOT EXISTS CourseProgress_updatedAt_idx ON CourseProgress(updatedAt); CREATE INDEX IF NOT EXISTS CourseSnapshot_courseId_idx ON CourseSnapshot(courseId); CREATE INDEX IF NOT EXISTS TaskProgress_courseId_idx ON TaskProgress(courseId); CREATE INDEX IF NOT EXISTS LearningStepProgress_courseId_idx ON LearningStepProgress(courseId); CREATE INDEX IF NOT EXISTS LearningCardProgress_courseId_idx ON LearningCardProgress(courseId); CREATE INDEX IF NOT EXISTS LearningSession_courseId_idx ON LearningSession(courseId);
)SQL");
    // 【逐行批注】 判断条件 (currentVersion < 3) {，成立时执行对应分支。
    if (currentVersion < 3) {
        // 【逐行批注】 计算右侧结果并写入 exec(db_, "UPDATE LearningSession SET fallba…，更新当前运行状态。
        exec(db_, "UPDATE LearningSession SET fallbackUsed=1, source='legacy' "
            // 【逐行批注】 执行当前声明或表达式："WHERE source<>'ai' OR fallbackUsed<>0 OR content NOT LIKE '%\"promptVersion\":\"ai-…。
            "WHERE source<>'ai' OR fallbackUsed<>0 OR content NOT LIKE '%\"promptVersion\":\"ai-block-v1\"%'");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (currentVersion < 4) {，成立时执行对应分支。
    if (currentVersion < 4) {
        // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "BEGIN IMMEDIATE");。
        exec(db_, "BEGIN IMMEDIATE");
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 调用 exec、SQL 完成本行所需操作：exec(db_, R"SQL(。
            exec(db_, R"SQL(
CREATE TABLE IF NOT EXISTS LearningInteraction(id TEXT PRIMARY KEY, courseId TEXT, conversationId TEXT, subject TEXT, kind TEXT NOT NULL, payload TEXT NOT NULL, createdAt TEXT NOT NULL);
CREATE INDEX IF NOT EXISTS LearningInteraction_course_idx ON LearningInteraction(courseId);
CREATE INDEX IF NOT EXISTS LearningInteraction_conversation_idx ON LearningInteraction(conversationId,createdAt);
CREATE TABLE IF NOT EXISTS SubjectMastery(subject TEXT PRIMARY KEY, score INTEGER CHECK(score BETWEEN 0 AND 100), rationale TEXT NOT NULL DEFAULT '', weakPoints TEXT NOT NULL DEFAULT '[]', recommendation TEXT NOT NULL DEFAULT '', evidenceCount INTEGER NOT NULL DEFAULT 0, model TEXT NOT NULL DEFAULT '', status TEXT NOT NULL DEFAULT 'pending', updatedAt TEXT NOT NULL DEFAULT '');
CREATE TABLE IF NOT EXISTS ProfileMeta(key TEXT PRIMARY KEY, value TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS MyAIRecord(id TEXT PRIMARY KEY,kind TEXT NOT NULL,payload TEXT NOT NULL,updatedAt TEXT NOT NULL,version INTEGER NOT NULL DEFAULT 1);
CREATE INDEX IF NOT EXISTS MyAIRecordKind ON MyAIRecord(kind,updatedAt);
INSERT OR IGNORE INTO ProfileMeta(key,value) VALUES('revision','1');
INSERT OR IGNORE INTO ProfileMeta(key,value) VALUES('assessed','0');
INSERT OR IGNORE INTO ProfileMeta(key,value) VALUES('error','');
)SQL");
            // 【逐行批注】 计算右侧结果并写入 exec(db_, "PRAGMA user_version，更新当前运行状态。
            exec(db_, "PRAGMA user_version=4");
            // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "COMMIT");。
            exec(db_, "COMMIT");
        // 【逐行批注】 调用 exec 完成本行所需操作：} catch (...) { exec(db_, "ROLLBACK"); throw; }。
        } catch (...) { exec(db_, "ROLLBACK"); throw; }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 exec、MyAIRecord 完成本行所需操作：exec(db_,"CREATE TABLE IF NOT EXISTS MyAIRecord(id TEXT PRIMARY KEY,k…。
    exec(db_,"CREATE TABLE IF NOT EXISTS MyAIRecord(id TEXT PRIMARY KEY,kind TEXT NOT NULL,payload TEXT NOT NULL,updatedAt TEXT NOT NULL,version INTEGER NOT NULL DEFAULT 1);CREATE INDEX IF NOT EXISTS MyAIRecordKind ON MyAIRecord(kind,updatedAt);");
    // 【逐行批注】 判断条件 (currentVersion < 5) {，成立时执行对应分支。
    if (currentVersion < 5) {
        // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "BEGIN IMMEDIATE");。
        exec(db_, "BEGIN IMMEDIATE");
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 调用 exec、SQL 完成本行所需操作：exec(db_, R"SQL(。
            exec(db_, R"SQL(
CREATE TABLE IF NOT EXISTS TopicMastery(courseId TEXT NOT NULL, phaseIndex INTEGER NOT NULL, topic TEXT NOT NULL,
 score INTEGER CHECK(score BETWEEN 0 AND 100), evidenceCount INTEGER NOT NULL DEFAULT 0,
 rationale TEXT NOT NULL DEFAULT '', weakPoints TEXT NOT NULL DEFAULT '[]', recommendation TEXT NOT NULL DEFAULT '',
 evidenceIds TEXT NOT NULL DEFAULT '[]', nextReviewAt TEXT NOT NULL DEFAULT '', model TEXT NOT NULL DEFAULT '',
 status TEXT NOT NULL DEFAULT 'insufficient', version INTEGER NOT NULL DEFAULT 0, updatedAt TEXT NOT NULL DEFAULT '',
 PRIMARY KEY(courseId,phaseIndex,topic));
UPDATE SubjectMastery SET status='legacy' WHERE status='ready';
)SQL");
            // 【逐行批注】 计算右侧结果并写入 exec(db_, "PRAGMA user_version，更新当前运行状态。
            exec(db_, "PRAGMA user_version=5");
            // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "COMMIT");。
            exec(db_, "COMMIT");
        // 【逐行批注】 调用 exec 完成本行所需操作：} catch (...) { exec(db_, "ROLLBACK"); throw; }。
        } catch (...) { exec(db_, "ROLLBACK"); throw; }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (currentVersion < 6) {，成立时执行对应分支。
    if (currentVersion < 6) {
        // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "BEGIN IMMEDIATE");。
        exec(db_, "BEGIN IMMEDIATE");
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 调用 exec、SQL 完成本行所需操作：exec(db_, R"SQL(。
            exec(db_, R"SQL(
CREATE TABLE IF NOT EXISTS ClassroomActivity(id TEXT PRIMARY KEY, courseId TEXT NOT NULL, phaseIndex INTEGER NOT NULL, topicIndex INTEGER NOT NULL, kind TEXT NOT NULL, payload TEXT NOT NULL, updatedAt TEXT NOT NULL);
CREATE INDEX IF NOT EXISTS ClassroomActivity_lesson_idx ON ClassroomActivity(courseId,phaseIndex,topicIndex,kind);
CREATE TABLE IF NOT EXISTS WeeklyPlan(courseId TEXT PRIMARY KEY, payload TEXT NOT NULL, version INTEGER NOT NULL DEFAULT 1, updatedAt TEXT NOT NULL);
)SQL");
            // 【逐行批注】 计算右侧结果并写入 exec(db_, "PRAGMA user_version，更新当前运行状态。
            exec(db_, "PRAGMA user_version=6");
            // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "COMMIT");。
            exec(db_, "COMMIT");
        // 【逐行批注】 调用 exec 完成本行所需操作：} catch (...) { exec(db_, "ROLLBACK"); throw; }。
        } catch (...) { exec(db_, "ROLLBACK"); throw; }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 exec 完成本行所需操作：if(currentVersion<7)exec(db_,"PRAGMA user_version=7");。
    if(currentVersion<7)exec(db_,"PRAGMA user_version=7");
    // 【逐行批注】 调用 exec、SQL 完成本行所需操作：exec(db_, R"SQL(。
    exec(db_, R"SQL(
CREATE TABLE IF NOT EXISTS AgentRunEvent(
 taskId TEXT NOT NULL, seq INTEGER NOT NULL, payload TEXT NOT NULL, createdAt TEXT NOT NULL,
 PRIMARY KEY(taskId,seq));
CREATE INDEX IF NOT EXISTS AgentRunEvent_task_idx ON AgentRunEvent(taskId,seq);
)SQL");
    // 【逐行批注】 调用 exec 完成本行所需操作：if(currentVersion<8)exec(db_,"PRAGMA user_version=8");。
    if(currentVersion<8)exec(db_,"PRAGMA user_version=8");
    // 【逐行批注】 调用 exec、SQL 完成本行所需操作：exec(db_, R"SQL(。
    exec(db_, R"SQL(
CREATE TABLE IF NOT EXISTS IndependentChat(
 lessonId TEXT PRIMARY KEY, webEnabled INTEGER NOT NULL DEFAULT 0,
 activeTaskId TEXT NOT NULL DEFAULT '', createdAt TEXT NOT NULL, updatedAt TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS IndependentChatMessage(
 id TEXT PRIMARY KEY, lessonId TEXT NOT NULL, requestId TEXT NOT NULL DEFAULT '',
 role TEXT NOT NULL, content TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'ready',
 sources TEXT NOT NULL DEFAULT '[]', model TEXT NOT NULL DEFAULT '', error TEXT NOT NULL DEFAULT '',
 createdAt TEXT NOT NULL, updatedAt TEXT NOT NULL);
CREATE INDEX IF NOT EXISTS IndependentChatMessage_session_idx
 ON IndependentChatMessage(lessonId,createdAt);
)SQL");
    // 【逐行批注】 调用 exec 完成本行所需操作：if(currentVersion<9)exec(db_,"PRAGMA user_version=9");。
    if(currentVersion<9)exec(db_,"PRAGMA user_version=9");
// 【逐行批注】 结束当前代码块。
}

// Course and User are kept explicit below; the repeated progress tables use the same SQL shape.
// 【逐行批注】 定义函数 bind_Course，执行该函数声明的职责。
static void bind_Course(Stmt&s,const Course&v){text(s,1,v.id);opt(s,2,v.anonymousId);opt(s,3,v.userId);text(s,4,v.goal);text(s,5,v.mode);text(s,6,v.title);opt(s,7,v.summary);text(s,8,v.source);text(s,9,v.status);text(s,10,v.createdAt);text(s,11,v.updatedAt);}
// 【逐行批注】 定义函数 map_Course，执行该函数声明的职责。
static Course map_Course(sqlite3_stmt*s){Course v;v.id=str(s,0);v.anonymousId=ostr(s,1);v.userId=ostr(s,2);v.goal=str(s,3);v.mode=str(s,4);v.title=str(s,5);v.summary=ostr(s,6);v.source=str(s,7);v.status=str(s,8);v.createdAt=str(s,9);v.updatedAt=str(s,10);return v;}
// 【逐行批注】 定义函数 Database::insert，执行该函数声明的职责。
bool Database::insert(const Course& v){ if(v.id.empty()) { Course copy=v; copy.id=id(); return insert(copy); } Stmt s(db_,"INSERT INTO Course(id,anonymousId,userId,goal,mode,title,summary,source,status,createdAt,updatedAt) VALUES(?,?,?,?,?,?,?,?,?,?,?)");bind_Course(s,v);return done(s);}
// 【逐行批注】 定义函数 Database::getCourse，执行该函数声明的职责。
std::optional<Course> Database::getCourse(const std::string& key) const {Stmt s(db_,"SELECT id,anonymousId,userId,goal,mode,title,summary,source,status,createdAt,updatedAt FROM Course WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_Course(s.p);}
// 【逐行批注】 定义函数 Database::listCourses，执行该函数声明的职责。
std::vector<Course> Database::listCourses() const {std::vector<Course> r;Stmt s(db_,"SELECT id,anonymousId,userId,goal,mode,title,summary,source,status,createdAt,updatedAt FROM Course ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_Course(s.p));return r;}
// 【逐行批注】 定义函数 Database::update，执行该函数声明的职责。
bool Database::update(const Course&v){Stmt s(db_,"UPDATE Course SET anonymousId=?,userId=?,goal=?,mode=?,title=?,summary=?,source=?,status=?,createdAt=?,updatedAt=? WHERE id=?");opt(s,1,v.anonymousId);opt(s,2,v.userId);text(s,3,v.goal);text(s,4,v.mode);text(s,5,v.title);opt(s,6,v.summary);text(s,7,v.source);text(s,8,v.status);text(s,9,v.createdAt);text(s,10,v.updatedAt);text(s,11,v.id);return done(s);}
// 【逐行批注】 定义函数 Database::deleteCourse，执行该函数声明的职责。
bool Database::deleteCourse(const std::string&key){Stmt s(db_,"DELETE FROM Course WHERE id=?");text(s,1,key);return done(s);}

// 【逐行批注】 定义函数 bind_User，执行该函数声明的职责。
static void bind_User(Stmt&s,const User&v){text(s,1,v.id);text(s,2,v.email);opt(s,3,v.name);text(s,4,v.passwordHash);text(s,5,v.membershipTier);text(s,6,v.membershipStatus);opt(s,7,v.membershipStartedAt);opt(s,8,v.membershipExpiresAt);text(s,9,v.createdAt);text(s,10,v.updatedAt);}
// 【逐行批注】 定义函数 map_User，执行该函数声明的职责。
static User map_User(sqlite3_stmt*s){User v;v.id=str(s,0);v.email=str(s,1);v.name=ostr(s,2);v.passwordHash=str(s,3);v.membershipTier=str(s,4);v.membershipStatus=str(s,5);v.membershipStartedAt=ostr(s,6);v.membershipExpiresAt=ostr(s,7);v.createdAt=str(s,8);v.updatedAt=str(s,9);return v;}
// 【逐行批注】 定义函数 Database::insert，执行该函数声明的职责。
bool Database::insert(const User&v){Stmt s(db_,"INSERT INTO User(id,email,name,passwordHash,membershipTier,membershipStatus,membershipStartedAt,membershipExpiresAt,createdAt,updatedAt) VALUES(?,?,?,?,?,?,?,?,?,?)");bind_User(s,v);return done(s);}
// 【逐行批注】 定义函数 Database::getUser，执行该函数声明的职责。
std::optional<User> Database::getUser(const std::string&key)const{Stmt s(db_,"SELECT id,email,name,passwordHash,membershipTier,membershipStatus,membershipStartedAt,membershipExpiresAt,createdAt,updatedAt FROM User WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_User(s.p);}
// 【逐行批注】 定义函数 Database::findByEmail，执行该函数声明的职责。
std::optional<User> Database::findByEmail(const std::string&email)const{Stmt s(db_,"SELECT id,email,name,passwordHash,membershipTier,membershipStatus,membershipStartedAt,membershipExpiresAt,createdAt,updatedAt FROM User WHERE email=?");text(s,1,email);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_User(s.p);}
// 【逐行批注】 定义函数 Database::listUsers，执行该函数声明的职责。
std::vector<User> Database::listUsers()const{std::vector<User>r;Stmt s(db_,"SELECT id,email,name,passwordHash,membershipTier,membershipStatus,membershipStartedAt,membershipExpiresAt,createdAt,updatedAt FROM User ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_User(s.p));return r;}
// 【逐行批注】 定义函数 Database::update，执行该函数声明的职责。
bool Database::update(const User&v){Stmt s(db_,"UPDATE User SET email=?,name=?,passwordHash=?,membershipTier=?,membershipStatus=?,membershipStartedAt=?,membershipExpiresAt=?,createdAt=?,updatedAt=? WHERE id=?");text(s,1,v.email);opt(s,2,v.name);text(s,3,v.passwordHash);text(s,4,v.membershipTier);text(s,5,v.membershipStatus);opt(s,6,v.membershipStartedAt);opt(s,7,v.membershipExpiresAt);text(s,8,v.createdAt);text(s,9,v.updatedAt);text(s,10,v.id);return done(s);}
// 【逐行批注】 定义函数 Database::deleteUser，执行该函数声明的职责。
bool Database::deleteUser(const std::string&key){Stmt s(db_,"DELETE FROM User WHERE id=?");text(s,1,key);return done(s);}

// ---- UserSession ----
// 【逐行批注】 定义函数 bind_UserSession，执行该函数声明的职责。
static void bind_UserSession(Stmt&s,const UserSession&v){text(s,1,v.id);text(s,2,v.userId);text(s,3,v.tokenHash);text(s,4,v.createdAt);text(s,5,v.expiresAt);}
// 【逐行批注】 定义函数 map_UserSession，执行该函数声明的职责。
static UserSession map_UserSession(sqlite3_stmt*s){UserSession v;v.id=str(s,0);v.userId=str(s,1);v.tokenHash=str(s,2);v.createdAt=str(s,3);v.expiresAt=str(s,4);return v;}
// 【逐行批注】 定义函数 Database::insert，执行该函数声明的职责。
bool Database::insert(const UserSession&v){Stmt s(db_,"INSERT INTO UserSession(id,userId,tokenHash,createdAt,expiresAt) VALUES(?,?,?,?,?)");bind_UserSession(s,v);return done(s);}
// 【逐行批注】 定义函数 Database::getUserSession，执行该函数声明的职责。
std::optional<UserSession> Database::getUserSession(const std::string&key)const{Stmt s(db_,"SELECT id,userId,tokenHash,createdAt,expiresAt FROM UserSession WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_UserSession(s.p);}
// 【逐行批注】 定义函数 Database::listUserSessions，执行该函数声明的职责。
std::vector<UserSession> Database::listUserSessions()const{std::vector<UserSession>r;Stmt s(db_,"SELECT id,userId,tokenHash,createdAt,expiresAt FROM UserSession ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_UserSession(s.p));return r;}
// 【逐行批注】 定义函数 Database::update，执行该函数声明的职责。
bool Database::update(const UserSession&v){Stmt s(db_,"UPDATE UserSession SET userId=?,tokenHash=?,createdAt=?,expiresAt=? WHERE id=?");text(s,1,v.userId);text(s,2,v.tokenHash);text(s,3,v.createdAt);text(s,4,v.expiresAt);text(s,5,v.id);return done(s);}
// 【逐行批注】 定义函数 Database::deleteUserSession，执行该函数声明的职责。
bool Database::deleteUserSession(const std::string&key){Stmt s(db_,"DELETE FROM UserSession WHERE id=?");text(s,1,key);return done(s);}
// 【逐行批注】 定义函数 Database::findSessionByTokenHash，执行该函数声明的职责。
std::optional<UserSession> Database::findSessionByTokenHash(const std::string&hash)const{Stmt s(db_,"SELECT id,userId,tokenHash,createdAt,expiresAt FROM UserSession WHERE tokenHash=?");text(s,1,hash);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_UserSession(s.p);}

// ---- UsageCounter ----
// 【逐行批注】 定义函数 bind_UsageCounter，执行该函数声明的职责。
static void bind_UsageCounter(Stmt&s,const UsageCounter&v){text(s,1,v.id);text(s,2,v.scopeId);text(s,3,v.scopeType);text(s,4,v.type);text(s,5,v.dateKey);integer(s,6,v.count);}
// 【逐行批注】 定义函数 map_UsageCounter，执行该函数声明的职责。
static UsageCounter map_UsageCounter(sqlite3_stmt*s){UsageCounter v;v.id=str(s,0);v.scopeId=str(s,1);v.scopeType=str(s,2);v.type=str(s,3);v.dateKey=str(s,4);v.count=sqlite3_column_int(s,5);return v;}
// 【逐行批注】 定义函数 Database::insert，执行该函数声明的职责。
bool Database::insert(const UsageCounter&v){Stmt s(db_,"INSERT INTO UsageCounter(id,scopeId,scopeType,type,dateKey,count) VALUES(?,?,?,?,?,?)");bind_UsageCounter(s,v);return done(s);}
// 【逐行批注】 定义函数 Database::getUsageCounter，执行该函数声明的职责。
std::optional<UsageCounter> Database::getUsageCounter(const std::string&key)const{Stmt s(db_,"SELECT id,scopeId,scopeType,type,dateKey,count FROM UsageCounter WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_UsageCounter(s.p);}
// 【逐行批注】 定义函数 Database::listUsageCounters，执行该函数声明的职责。
std::vector<UsageCounter> Database::listUsageCounters()const{std::vector<UsageCounter>r;Stmt s(db_,"SELECT id,scopeId,scopeType,type,dateKey,count FROM UsageCounter ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_UsageCounter(s.p));return r;}
// 【逐行批注】 定义函数 Database::update，执行该函数声明的职责。
bool Database::update(const UsageCounter&v){Stmt s(db_,"UPDATE UsageCounter SET scopeId=?,scopeType=?,type=?,dateKey=?,count=? WHERE id=?");text(s,1,v.scopeId);text(s,2,v.scopeType);text(s,3,v.type);text(s,4,v.dateKey);integer(s,5,v.count);text(s,6,v.id);return done(s);}
// 【逐行批注】 定义函数 Database::deleteUsageCounter，执行该函数声明的职责。
bool Database::deleteUsageCounter(const std::string&key){Stmt s(db_,"DELETE FROM UsageCounter WHERE id=?");text(s,1,key);return done(s);}
// 【逐行批注】 定义函数 Database::findUsageCounter，执行该函数声明的职责。
std::optional<UsageCounter> Database::findUsageCounter(const std::string&scopeId,const std::string&scopeType,const std::string&type,const std::string&dateKey)const{Stmt s(db_,"SELECT id,scopeId,scopeType,type,dateKey,count FROM UsageCounter WHERE scopeId=? AND scopeType=? AND type=? AND dateKey=?");text(s,1,scopeId);text(s,2,scopeType);text(s,3,type);text(s,4,dateKey);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_UsageCounter(s.p);}

// ---- CourseProgress ----
// 【逐行批注】 定义函数 bind_CourseProgress，执行该函数声明的职责。
static void bind_CourseProgress(Stmt&s,const CourseProgress&v){text(s,1,v.id);text(s,2,v.courseId);opt(s,3,v.anonymousId);opt(s,4,v.userId);text(s,5,v.goal);opt(s,6,v.mode);integer(s,7,v.overallPercent);integer(s,8,v.completedCount);integer(s,9,v.totalCount);opt(s,10,v.lastVisitedUrl);opt(s,11,v.lastPageType);optint(s,12,v.lastPhaseIndex);opt(s,13,v.lastPhaseName);optint(s,14,v.lastTopicIndex);opt(s,15,v.lastTopicTitle);text(s,16,v.updatedAt);text(s,17,v.createdAt);}
// 【逐行批注】 定义函数 map_CourseProgress，执行该函数声明的职责。
static CourseProgress map_CourseProgress(sqlite3_stmt*s){CourseProgress v;v.id=str(s,0);v.courseId=str(s,1);v.anonymousId=ostr(s,2);v.userId=ostr(s,3);v.goal=str(s,4);v.mode=ostr(s,5);v.overallPercent=sqlite3_column_int(s,6);v.completedCount=sqlite3_column_int(s,7);v.totalCount=sqlite3_column_int(s,8);v.lastVisitedUrl=ostr(s,9);v.lastPageType=ostr(s,10);v.lastPhaseIndex=oint(s,11);v.lastPhaseName=ostr(s,12);v.lastTopicIndex=oint(s,13);v.lastTopicTitle=ostr(s,14);v.updatedAt=str(s,15);v.createdAt=str(s,16);return v;}
// 【逐行批注】 定义函数 Database::insert，执行该函数声明的职责。
bool Database::insert(const CourseProgress&v){Stmt s(db_,"INSERT INTO CourseProgress(id,courseId,anonymousId,userId,goal,mode,overallPercent,completedCount,totalCount,lastVisitedUrl,lastPageType,lastPhaseIndex,lastPhaseName,lastTopicIndex,lastTopicTitle,updatedAt,createdAt) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");bind_CourseProgress(s,v);return done(s);}
// 【逐行批注】 定义函数 Database::getCourseProgress，执行该函数声明的职责。
std::optional<CourseProgress> Database::getCourseProgress(const std::string&key)const{Stmt s(db_,"SELECT id,courseId,anonymousId,userId,goal,mode,overallPercent,completedCount,totalCount,lastVisitedUrl,lastPageType,lastPhaseIndex,lastPhaseName,lastTopicIndex,lastTopicTitle,updatedAt,createdAt FROM CourseProgress WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_CourseProgress(s.p);}
// 【逐行批注】 定义函数 Database::listCourseProgress，执行该函数声明的职责。
std::vector<CourseProgress> Database::listCourseProgress()const{std::vector<CourseProgress>r;Stmt s(db_,"SELECT id,courseId,anonymousId,userId,goal,mode,overallPercent,completedCount,totalCount,lastVisitedUrl,lastPageType,lastPhaseIndex,lastPhaseName,lastTopicIndex,lastTopicTitle,updatedAt,createdAt FROM CourseProgress ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_CourseProgress(s.p));return r;}
// 【逐行批注】 定义函数 Database::update，执行该函数声明的职责。
bool Database::update(const CourseProgress&v){Stmt s(db_,"UPDATE CourseProgress SET courseId=?,anonymousId=?,userId=?,goal=?,mode=?,overallPercent=?,completedCount=?,totalCount=?,lastVisitedUrl=?,lastPageType=?,lastPhaseIndex=?,lastPhaseName=?,lastTopicIndex=?,lastTopicTitle=?,updatedAt=?,createdAt=? WHERE id=?");text(s,1,v.courseId);opt(s,2,v.anonymousId);opt(s,3,v.userId);text(s,4,v.goal);opt(s,5,v.mode);integer(s,6,v.overallPercent);integer(s,7,v.completedCount);integer(s,8,v.totalCount);opt(s,9,v.lastVisitedUrl);opt(s,10,v.lastPageType);optint(s,11,v.lastPhaseIndex);opt(s,12,v.lastPhaseName);optint(s,13,v.lastTopicIndex);opt(s,14,v.lastTopicTitle);text(s,15,v.updatedAt);text(s,16,v.createdAt);text(s,17,v.id);return done(s);}
// 【逐行批注】 定义函数 Database::deleteCourseProgress，执行该函数声明的职责。
bool Database::deleteCourseProgress(const std::string&key){Stmt s(db_,"DELETE FROM CourseProgress WHERE id=?");text(s,1,key);return done(s);}
// 【逐行批注】 定义函数 Database::findProgressByCourseId，执行该函数声明的职责。
std::optional<CourseProgress> Database::findProgressByCourseId(const std::string&courseId)const{Stmt s(db_,"SELECT id,courseId,anonymousId,userId,goal,mode,overallPercent,completedCount,totalCount,lastVisitedUrl,lastPageType,lastPhaseIndex,lastPhaseName,lastTopicIndex,lastTopicTitle,updatedAt,createdAt FROM CourseProgress WHERE courseId=?");text(s,1,courseId);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_CourseProgress(s.p);}

// ---- CourseSnapshot ----
// 【逐行批注】 定义函数 bind_CourseSnapshot，执行该函数声明的职责。
static void bind_CourseSnapshot(Stmt&s,const CourseSnapshot&v){text(s,1,v.id);text(s,2,v.courseId);integer(s,3,v.version);text(s,4,v.payload);text(s,5,v.createdAt);}
// 【逐行批注】 定义函数 map_CourseSnapshot，执行该函数声明的职责。
static CourseSnapshot map_CourseSnapshot(sqlite3_stmt*s){CourseSnapshot v;v.id=str(s,0);v.courseId=str(s,1);v.version=sqlite3_column_int(s,2);v.payload=str(s,3);v.createdAt=str(s,4);return v;}
// 【逐行批注】 定义函数 Database::insert，执行该函数声明的职责。
bool Database::insert(const CourseSnapshot&v){ if(v.id.empty()) { CourseSnapshot copy=v; copy.id=id(); return insert(copy); } Stmt s(db_,"INSERT INTO CourseSnapshot(id,courseId,version,payload,createdAt) VALUES(?,?,?,?,?)");bind_CourseSnapshot(s,v);const bool saved=done(s);
    // 【逐行批注】 判断条件 (saved) {，成立时执行对应分支。
    if (saved) {
        // 【逐行批注】 调用 markLearningDirty 完成本行所需操作：markLearningDirty();。
        markLearningDirty();
        // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
        const auto previous=getClassroomActivity("course-preview:"+v.courseId);
        // 【逐行批注】 计算右侧结果并写入 payload，更新当前运行状态。
        auto payload=previous?nlohmann::json::parse(previous->payload,nullptr,false):nlohmann::json::object();
        // 【逐行批注】 判断条件 (!payload.is_object()) payload=nlohmann::json::object();，成立时执行对应分支。
        if (!payload.is_object()) payload=nlohmann::json::object();
        // 【逐行批注】 计算右侧结果并写入 payload["status"]，更新当前运行状态。
        payload["status"]="pending";payload["assessedCourseVersion"]=v.version;payload["learningVersion"]=learningRevision();
        // 【逐行批注】 计算右侧结果并写入 payload["message"]，更新当前运行状态。
        payload["message"]="真实 AI 正在生成课程路线预览…";
        // 【逐行批注】 判断条件 (!payload.contains("slides")) payload["slides"]=nlohmann::json::…，成立时执行对应分支。
        if (!payload.contains("slides")) payload["slides"]=nlohmann::json::array();
        // 【逐行批注】 调用 upsert 完成本行所需操作：upsert(ClassroomActivity{"course-preview:"+v.courseId,v.courseId,"pre…。
        upsert(ClassroomActivity{"course-preview:"+v.courseId,v.courseId,"preview",payload.dump(),v.createdAt,0,0});
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return saved;}。
    return saved;}
// 【逐行批注】 定义函数 Database::getCourseSnapshot，执行该函数声明的职责。
std::optional<CourseSnapshot> Database::getCourseSnapshot(const std::string&key)const{Stmt s(db_,"SELECT id,courseId,version,payload,createdAt FROM CourseSnapshot WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_CourseSnapshot(s.p);}
// 【逐行批注】 定义函数 Database::listCourseSnapshots，执行该函数声明的职责。
std::vector<CourseSnapshot> Database::listCourseSnapshots()const{std::vector<CourseSnapshot>r;Stmt s(db_,"SELECT id,courseId,version,payload,createdAt FROM CourseSnapshot ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_CourseSnapshot(s.p));return r;}
// 【逐行批注】 定义函数 Database::update，执行该函数声明的职责。
bool Database::update(const CourseSnapshot&v){Stmt s(db_,"UPDATE CourseSnapshot SET courseId=?,version=?,payload=?,createdAt=? WHERE id=?");text(s,1,v.courseId);integer(s,2,v.version);text(s,3,v.payload);text(s,4,v.createdAt);text(s,5,v.id);return done(s);}
// 【逐行批注】 定义函数 Database::deleteCourseSnapshot，执行该函数声明的职责。
bool Database::deleteCourseSnapshot(const std::string&key){Stmt s(db_,"DELETE FROM CourseSnapshot WHERE id=?");text(s,1,key);return done(s);}
// 【逐行批注】 定义函数 Database::findSnapshotsByCourseId，执行该函数声明的职责。
std::vector<CourseSnapshot> Database::findSnapshotsByCourseId(const std::string&courseId)const{std::vector<CourseSnapshot>r;Stmt s(db_,"SELECT id,courseId,version,payload,createdAt FROM CourseSnapshot WHERE courseId=? ORDER BY version");text(s,1,courseId);while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_CourseSnapshot(s.p));return r;}

// ---- 通用进度表模式（TaskProgress / LearningStepProgress / LearningCardProgress）----
// 【逐行批注】 执行当前声明或表达式：template <typename T>。
template <typename T>
// 【逐行批注】 声明数据结构或类型 ProgressTraits 及其成员接口。
struct ProgressTraits;
// 【逐行批注】 执行当前声明或表达式：template <> struct ProgressTraits<TaskProgress> {。
template <> struct ProgressTraits<TaskProgress> {
    // 【逐行批注】 计算右侧结果并写入 constexpr const char* table，更新当前运行状态。
    static constexpr const char* table = "TaskProgress";
    // 【逐行批注】 计算右侧结果并写入 constexpr const char* index_col，更新当前运行状态。
    static constexpr const char* index_col = "taskIndex";
    // 【逐行批注】 计算右侧结果并写入 constexpr const char* index_title，更新当前运行状态。
    static constexpr const char* index_title = "taskTitle";
    // 【逐行批注】 定义函数 key，执行该函数声明的职责。
    static std::string key(const TaskProgress& v) { return v.courseId ? *v.courseId : v.id; }
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 执行当前声明或表达式：template <> struct ProgressTraits<LearningStepProgress> {。
template <> struct ProgressTraits<LearningStepProgress> {
    // 【逐行批注】 计算右侧结果并写入 constexpr const char* table，更新当前运行状态。
    static constexpr const char* table = "LearningStepProgress";
    // 【逐行批注】 计算右侧结果并写入 constexpr const char* index_col，更新当前运行状态。
    static constexpr const char* index_col = "stepIndex";
    // 【逐行批注】 计算右侧结果并写入 constexpr const char* index_title，更新当前运行状态。
    static constexpr const char* index_title = "stepTitle";
    // 【逐行批注】 定义函数 key，执行该函数声明的职责。
    static std::string key(const LearningStepProgress& v) { return v.courseId ? *v.courseId : v.id; }
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 执行当前声明或表达式：template <> struct ProgressTraits<LearningCardProgress> {。
template <> struct ProgressTraits<LearningCardProgress> {
    // 【逐行批注】 计算右侧结果并写入 constexpr const char* table，更新当前运行状态。
    static constexpr const char* table = "LearningCardProgress";
    // 【逐行批注】 计算右侧结果并写入 constexpr const char* index_col，更新当前运行状态。
    static constexpr const char* index_col = "topicIndex";
    // 【逐行批注】 计算右侧结果并写入 constexpr const char* index_title，更新当前运行状态。
    static constexpr const char* index_title = "topicTitle";
    // 【逐行批注】 定义函数 key，执行该函数声明的职责。
    static std::string key(const LearningCardProgress& v) { return v.courseId ? *v.courseId : v.id; }
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义编译期常量或宏：#define DEFINE_PROGRESS_CRUD(TYPE, IDXCOL, IDXTITLE) \。
#define DEFINE_PROGRESS_CRUD(TYPE, IDXCOL, IDXTITLE) \
static void bind_##TYPE(Stmt&s,const TYPE&v){text(s,1,v.id);opt(s,2,v.courseId);opt(s,3,v.anonymousId);text(s,4,v.goal);opt(s,5,v.mode);integer(s,6,v.phaseIndex);text(s,7,v.phaseName);integer(s,8,v.IDXCOL);text(s,9,v.IDXTITLE);text(s,10,v.status);} \
static TYPE map_##TYPE(sqlite3_stmt*s){TYPE v;v.id=str(s,0);v.courseId=ostr(s,1);v.anonymousId=ostr(s,2);v.goal=str(s,3);v.mode=ostr(s,4);v.phaseIndex=sqlite3_column_int(s,5);v.phaseName=str(s,6);v.IDXCOL=sqlite3_column_int(s,7);v.IDXTITLE=str(s,8);v.status=str(s,9);return v;} \
bool Database::insert(const TYPE&v){Stmt s(db_,"INSERT INTO " #TYPE "(id,courseId,anonymousId,goal,mode,phaseIndex,phaseName," #IDXCOL "," #IDXTITLE ",status) VALUES(?,?,?,?,?,?,?,?,?,?)");bind_##TYPE(s,v);return done(s);} \
std::optional<TYPE> Database::get##TYPE(const std::string&key)const{Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName," #IDXCOL "," #IDXTITLE ",status FROM " #TYPE " WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_##TYPE(s.p);} \
std::vector<TYPE> Database::list##TYPE()const{std::vector<TYPE>r;Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName," #IDXCOL "," #IDXTITLE ",status FROM " #TYPE " ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_##TYPE(s.p));return r;} \
bool Database::update(const TYPE&v){Stmt s(db_,"UPDATE " #TYPE " SET courseId=?,anonymousId=?,goal=?,mode=?,phaseIndex=?,phaseName=?," #IDXCOL "=?," #IDXTITLE "=?,status=? WHERE id=?");opt(s,1,v.courseId);opt(s,2,v.anonymousId);text(s,3,v.goal);opt(s,4,v.mode);integer(s,5,v.phaseIndex);text(s,6,v.phaseName);integer(s,7,v.IDXCOL);text(s,8,v.IDXTITLE);text(s,9,v.status);text(s,10,v.id);return done(s);} \
bool Database::delete##TYPE(const std::string&key){Stmt s(db_,"DELETE FROM " #TYPE " WHERE id=?");text(s,1,key);return done(s);} \
std::optional<TYPE> Database::find##TYPE(const std::string&courseId,int phaseIndex,int idx)const{Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName," #IDXCOL "," #IDXTITLE ",status FROM " #TYPE " WHERE courseId=? AND phaseIndex=? AND " #IDXCOL "=?");text(s,1,courseId);integer(s,2,phaseIndex);integer(s,3,idx);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_##TYPE(s.p);}

// 【逐行批注】 调用 DEFINE_PROGRESS_CRUD 完成本行所需操作：DEFINE_PROGRESS_CRUD(TaskProgress, taskIndex, taskTitle)。
DEFINE_PROGRESS_CRUD(TaskProgress, taskIndex, taskTitle)
// 【逐行批注】 调用 DEFINE_PROGRESS_CRUD 完成本行所需操作：DEFINE_PROGRESS_CRUD(LearningStepProgress, stepIndex, stepTitle)。
DEFINE_PROGRESS_CRUD(LearningStepProgress, stepIndex, stepTitle)
// 【逐行批注】 调用 DEFINE_PROGRESS_CRUD 完成本行所需操作：DEFINE_PROGRESS_CRUD(LearningCardProgress, topicIndex, topicTitle)。
DEFINE_PROGRESS_CRUD(LearningCardProgress, topicIndex, topicTitle)

// ---- LearningSession ----
// 【逐行批注】 定义函数 bind_LearningSession，执行该函数声明的职责。
static void bind_LearningSession(Stmt&s,const LearningSession&v){text(s,1,v.id);opt(s,2,v.courseId);opt(s,3,v.anonymousId);text(s,4,v.goal);opt(s,5,v.mode);integer(s,6,v.phaseIndex);text(s,7,v.phaseName);integer(s,8,v.topicIndex);text(s,9,v.topicTitle);text(s,10,v.title);opt(s,11,v.summary);opt(s,12,v.searchQuery);text(s,13,v.content);opt(s,14,v.references);integer(s,15,v.fallbackUsed);text(s,16,v.source);}
// 【逐行批注】 定义函数 map_LearningSession，执行该函数声明的职责。
static LearningSession map_LearningSession(sqlite3_stmt*s){LearningSession v;v.id=str(s,0);v.courseId=ostr(s,1);v.anonymousId=ostr(s,2);v.goal=str(s,3);v.mode=ostr(s,4);v.phaseIndex=sqlite3_column_int(s,5);v.phaseName=str(s,6);v.topicIndex=sqlite3_column_int(s,7);v.topicTitle=str(s,8);v.title=str(s,9);v.summary=ostr(s,10);v.searchQuery=ostr(s,11);v.content=str(s,12);v.references=ostr(s,13);v.fallbackUsed=sqlite3_column_int(s,14);v.source=str(s,15);return v;}
// 【逐行批注】 定义函数 Database::insert，执行该函数声明的职责。
bool Database::insert(const LearningSession&v){Stmt s(db_,"INSERT INTO LearningSession(id,courseId,anonymousId,goal,mode,phaseIndex,phaseName,topicIndex,topicTitle,title,summary,searchQuery,content,`references`,fallbackUsed,source) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");bind_LearningSession(s,v);return done(s);}
// 【逐行批注】 定义函数 Database::getLearningSession，执行该函数声明的职责。
std::optional<LearningSession> Database::getLearningSession(const std::string&key)const{Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName,topicIndex,topicTitle,title,summary,searchQuery,content,`references`,fallbackUsed,source FROM LearningSession WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_LearningSession(s.p);}
// 【逐行批注】 定义函数 Database::listLearningSessions，执行该函数声明的职责。
std::vector<LearningSession> Database::listLearningSessions()const{std::vector<LearningSession>r;Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName,topicIndex,topicTitle,title,summary,searchQuery,content,`references`,fallbackUsed,source FROM LearningSession ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_LearningSession(s.p));return r;}
// 【逐行批注】 定义函数 Database::update，执行该函数声明的职责。
bool Database::update(const LearningSession&v){Stmt s(db_,"UPDATE LearningSession SET courseId=?,anonymousId=?,goal=?,mode=?,phaseIndex=?,phaseName=?,topicIndex=?,topicTitle=?,title=?,summary=?,searchQuery=?,content=?,`references`=?,fallbackUsed=?,source=? WHERE id=?");opt(s,1,v.courseId);opt(s,2,v.anonymousId);text(s,3,v.goal);opt(s,4,v.mode);integer(s,5,v.phaseIndex);text(s,6,v.phaseName);integer(s,7,v.topicIndex);text(s,8,v.topicTitle);text(s,9,v.title);opt(s,10,v.summary);opt(s,11,v.searchQuery);text(s,12,v.content);opt(s,13,v.references);integer(s,14,v.fallbackUsed);text(s,15,v.source);text(s,16,v.id);return done(s);}
// 【逐行批注】 定义函数 Database::deleteLearningSession，执行该函数声明的职责。
bool Database::deleteLearningSession(const std::string&key){Stmt s(db_,"DELETE FROM LearningSession WHERE id=?");text(s,1,key);return done(s);}
// 【逐行批注】 定义函数 Database::findLearningSession，执行该函数声明的职责。
std::optional<LearningSession> Database::findLearningSession(const std::string&courseId,int phaseIndex,int topicIndex)const{Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName,topicIndex,topicTitle,title,summary,searchQuery,content,`references`,fallbackUsed,source FROM LearningSession WHERE courseId=? AND phaseIndex=? AND topicIndex=?");text(s,1,courseId);integer(s,2,phaseIndex);integer(s,3,topicIndex);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_LearningSession(s.p);}

// 【逐行批注】 定义函数 Database::insert，执行该函数声明的职责。
bool Database::insert(const LearningInteraction& value) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO LearningInteraction(id,courseId,conversationId,subject,kind,payload,createdAt) VALUES(?,?,?,?,?,?,?)");
    // 【逐行批注】 调用 text、id、opt 完成本行所需操作：text(s,1,value.id.empty()?id():value.id);opt(s,2,value.courseId);opt(…。
    text(s,1,value.id.empty()?id():value.id);opt(s,2,value.courseId);opt(s,3,value.conversationId);
    // 【逐行批注】 调用 opt、text 完成本行所需操作：opt(s,4,value.subject);text(s,5,value.kind);text(s,6,value.payload);t…。
    opt(s,4,value.subject);text(s,5,value.kind);text(s,6,value.payload);text(s,7,value.createdAt);
    // 【逐行批注】 计算右侧结果并写入 bool saved，更新当前运行状态。
    const bool saved=done(s);
    // 【逐行批注】 调用 markProfileDirty 完成本行所需操作：if(saved && (value.kind=="quiz" || value.kind=="practice" || value.ki…。
    if(saved && (value.kind=="quiz" || value.kind=="practice" || value.kind=="review")) markProfileDirty();
    // 【逐行批注】 执行当前声明或表达式：if(saved && (value.kind=="quiz" || value.kind=="practice" || value.kind=="review" ||。
    if(saved && (value.kind=="quiz" || value.kind=="practice" || value.kind=="review" ||
        // 【逐行批注】 调用 markLearningDirty 完成本行所需操作：value.kind=="chat-user" || value.kind=="lesson" || value.kind=="step"…。
        value.kind=="chat-user" || value.kind=="lesson" || value.kind=="step")) markLearningDirty();
    // 【逐行批注】 结束当前函数并返回结果：return saved;。
    return saved;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::listInteractions，执行该函数声明的职责。
std::vector<LearningInteraction> Database::listInteractions() const {
    // 【逐行批注】 执行当前声明或表达式：std::vector<LearningInteraction> rows;。
    std::vector<LearningInteraction> rows;
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT id,courseId,conversationId,subject,kind,payload,createdAt FROM LearningInteraction ORDER BY rowid");
    // 【逐行批注】 在条件成立期间循环处理：while(sqlite3_step(s.p)==SQLITE_ROW) rows.push_back({str(s.p,0),str(s.p,4)…。
    while(sqlite3_step(s.p)==SQLITE_ROW) rows.push_back({str(s.p,0),str(s.p,4),str(s.p,5),str(s.p,6),ostr(s.p,1),ostr(s.p,2),ostr(s.p,3)});
    // 【逐行批注】 结束当前函数并返回结果：return rows;。
    return rows;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::upsert，执行该函数声明的职责。
bool Database::upsert(const ClassroomActivity& value) {
    // 【逐行批注】 计算右侧结果并写入 bool assessment，更新当前运行状态。
    const bool assessment = value.kind == "diagnostic" || value.kind == "interaction";
    // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
    const auto previous = assessment ? getClassroomActivity(value.id) : std::optional<ClassroomActivity>{};
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO ClassroomActivity(id,courseId,phaseIndex,topicIndex,kind,payload,updatedAt) VALUES(?,?,?,?,?,?,?) ON CONFLICT(id) DO UPDATE SET payload=excluded.payload,updatedAt=excluded.updatedAt");
    // 【逐行批注】 调用 text、integer 完成本行所需操作：text(s,1,value.id);text(s,2,value.courseId);integer(s,3,value.phaseIn…。
    text(s,1,value.id);text(s,2,value.courseId);integer(s,3,value.phaseIndex);integer(s,4,value.topicIndex);
    // 【逐行批注】 调用 text 完成本行所需操作：text(s,5,value.kind);text(s,6,value.payload);text(s,7,value.updatedAt…。
    text(s,5,value.kind);text(s,6,value.payload);text(s,7,value.updatedAt);
    // 【逐行批注】 计算右侧结果并写入 bool saved，更新当前运行状态。
    const bool saved = done(s);
    // 【逐行批注】 判断条件 (saved && assessment && (!previous || previous->payload != value…，成立时执行对应分支。
    if (saved && assessment && (!previous || previous->payload != value.payload)) {
        // 【逐行批注】 计算右侧结果并写入 auto payload，更新当前运行状态。
        const auto payload = nlohmann::json::parse(value.payload, nullptr, false);
        // 【逐行批注】 判断条件 (payload.is_object() && payload.value("status", "") == "answered…，成立时执行对应分支。
        if (payload.is_object() && payload.value("status", "") == "answered" && payload.value("credible", false)) {
            // 【逐行批注】 计算右侧结果并写入 auto old，更新当前运行状态。
            const auto old = previous ? nlohmann::json::parse(previous->payload, nullptr, false) : nlohmann::json();
            // 提示展开、反馈展示等界面变化不算新答案，避免额外触发 AI 评估。
            // 【逐行批注】 计算右侧结果并写入 bool changed，更新当前运行状态。
            bool changed = !old.is_object() || old.value("status", "") != "answered" || !old.value("credible", false);
            // 【逐行批注】 按循环规则重复处理：for (const char* key : {"question", "options", "rubric", "type", "answer",…。
            for (const char* key : {"question", "options", "rubric", "type", "answer", "correct", "unknown"})
                // 【逐行批注】 判断条件 (!old.is_object() || old.value(key, nlohmann::json()) != payload…，成立时执行对应分支。
                if (!old.is_object() || old.value(key, nlohmann::json()) != payload.value(key, nlohmann::json())) changed = true;
            // 【逐行批注】 判断条件 (changed) { markProfileDirty(); markLearningDirty(); }，成立时执行对应分支。
            if (changed) { markProfileDirty(); markLearningDirty(); }
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return saved;。
    return saved;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 调用 appendAgentRunEvent 完成本行所需操作：bool Database::appendAgentRunEvent(const std::string& taskId, int seq…。
bool Database::appendAgentRunEvent(const std::string& taskId, int seq, const std::string& payload,
                                   // 【逐行批注】 执行当前声明或表达式：const std::string& createdAt) {。
                                   const std::string& createdAt) {
    // 【逐行批注】 判断条件 (taskId.empty() || seq < 1 || payload.empty() || createdAt.empty…，成立时执行对应分支。
    if (taskId.empty() || seq < 1 || payload.empty() || createdAt.empty()) return false;
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT OR IGNORE INTO AgentRunEvent(taskId,seq,payload,createdAt) VALUES(?,?,?,?)");
    // 【逐行批注】 调用 text、integer 完成本行所需操作：text(s, 1, taskId); integer(s, 2, seq); text(s, 3, payload); text(s, …。
    text(s, 1, taskId); integer(s, 2, seq); text(s, 3, payload); text(s, 4, createdAt);
    // 【逐行批注】 调用 done 完成本行所需操作：done(s);。
    done(s);
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 调用 agentRunEvents 完成本行所需操作：std::vector<std::string> Database::agentRunEvents(const std::string& …。
std::vector<std::string> Database::agentRunEvents(const std::string& taskId, int afterSeq,
                                                  // 【逐行批注】 执行当前声明或表达式：int limit) const {。
                                                  int limit) const {
    // 【逐行批注】 判断条件 (taskId.empty()) return {};，成立时执行对应分支。
    if (taskId.empty()) return {};
    // 【逐行批注】 计算右侧结果并写入 limit，更新当前运行状态。
    limit = std::clamp(limit, 1, 2000);
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> result;。
    std::vector<std::string> result;
    // 从已确认序号后按正序读取下一页，长回复可连续恢复而不跳过中间事件。
    Stmt s(db_, "SELECT payload FROM AgentRunEvent WHERE taskId=? AND seq>? ORDER BY seq LIMIT ?");
    text(s, 1, taskId); integer(s, 2, std::max(0, afterSeq)); integer(s, 3, limit);
    // 【逐行批注】 在条件成立期间循环处理：while (sqlite3_step(s.p) == SQLITE_ROW) result.push_back(str(s.p, 0));。
    while (sqlite3_step(s.p) == SQLITE_ROW) result.push_back(str(s.p, 0));
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
std::optional<std::string> Database::agentRunEvent(const std::string& taskId, int seq) const {
    if (taskId.empty() || seq <= 0) return std::nullopt;
    Stmt s(db_, "SELECT payload FROM AgentRunEvent WHERE taskId=? AND seq=?");
    text(s, 1, taskId); integer(s, 2, seq);
    if (sqlite3_step(s.p) != SQLITE_ROW) return std::nullopt;
    return str(s.p, 0);
}

// 【逐行批注】 调用 independentChatSession 完成本行所需操作：std::optional<IndependentChatSession> Database::independentChatSessio…。
std::optional<IndependentChatSession> Database::independentChatSession(
    // 【逐行批注】 执行当前声明或表达式：const std::string& lessonId) const {。
    const std::string& lessonId) const {
    // 【逐行批注】 判断条件 (lessonId.empty()) return std::nullopt;，成立时执行对应分支。
    if (lessonId.empty()) return std::nullopt;
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT lessonId,webEnabled,activeTaskId,createdAt,updatedAt "
                // 【逐行批注】 计算右侧结果并写入 "FROM IndependentChat WHERE lessonId，更新当前运行状态。
                "FROM IndependentChat WHERE lessonId=?");
    // 【逐行批注】 调用 text 完成本行所需操作：text(s, 1, lessonId);。
    text(s, 1, lessonId);
    // 【逐行批注】 判断条件 (sqlite3_step(s.p) != SQLITE_ROW) return std::nullopt;，成立时执行对应分支。
    if (sqlite3_step(s.p) != SQLITE_ROW) return std::nullopt;
    // 【逐行批注】 结束当前函数并返回结果：return IndependentChatSession{str(s.p, 0), str(s.p, 2), str(s.p, 3), …。
    return IndependentChatSession{str(s.p, 0), str(s.p, 2), str(s.p, 3), str(s.p, 4),
        // 【逐行批注】 调用 sqlite3_column_int 完成本行所需操作：sqlite3_column_int(s.p, 1) != 0};。
        sqlite3_column_int(s.p, 1) != 0};
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::saveIndependentChatSession，执行该函数声明的职责。
bool Database::saveIndependentChatSession(const IndependentChatSession& value) {
    // 【逐行批注】 判断条件 (value.lessonId.empty() || value.createdAt.empty() || value.upda…，成立时执行对应分支。
    if (value.lessonId.empty() || value.createdAt.empty() || value.updatedAt.empty()) return false;
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO IndependentChat(lessonId,webEnabled,activeTaskId,createdAt,updatedAt) "
                // 【逐行批注】 调用 VALUES、CONFLICT 完成本行所需操作："VALUES(?,?,?,?,?) ON CONFLICT(lessonId) DO UPDATE SET "。
                "VALUES(?,?,?,?,?) ON CONFLICT(lessonId) DO UPDATE SET "
                // 【逐行批注】 计算右侧结果并写入 "webEnabled，更新当前运行状态。
                "webEnabled=excluded.webEnabled,activeTaskId=excluded.activeTaskId,updatedAt=excluded.updatedAt");
    // 【逐行批注】 调用 text、integer 完成本行所需操作：text(s, 1, value.lessonId); integer(s, 2, value.webEnabled ? 1 : 0); …。
    text(s, 1, value.lessonId); integer(s, 2, value.webEnabled ? 1 : 0); text(s, 3, value.activeTaskId);
    // 【逐行批注】 调用 text 完成本行所需操作：text(s, 4, value.createdAt); text(s, 5, value.updatedAt);。
    text(s, 4, value.createdAt); text(s, 5, value.updatedAt);
    // 【逐行批注】 结束当前函数并返回结果：return done(s);。
    return done(s);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 调用 independentChatMessages 完成本行所需操作：std::vector<IndependentChatMessage> Database::independentChatMessages(。
std::vector<IndependentChatMessage> Database::independentChatMessages(
    // 【逐行批注】 执行当前声明或表达式：const std::string& lessonId, int limit) const {。
    const std::string& lessonId, int limit) const {
    // 【逐行批注】 执行当前声明或表达式：std::vector<IndependentChatMessage> result;。
    std::vector<IndependentChatMessage> result;
    // 【逐行批注】 判断条件 (lessonId.empty()) return result;，成立时执行对应分支。
    if (lessonId.empty()) return result;
    // 【逐行批注】 计算右侧结果并写入 limit，更新当前运行状态。
    limit = std::clamp(limit, 1, 500);
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "WITH recent AS (SELECT rowid,id,lessonId,requestId,role,content,status,sources,model,error,createdAt,updatedAt "
                // 【逐行批注】 计算右侧结果并写入 "FROM IndependentChatMessage WHERE lessonId，更新当前运行状态。
                "FROM IndependentChatMessage WHERE lessonId=? ORDER BY rowid DESC LIMIT ?) "
                // 【逐行批注】 执行当前声明或表达式："SELECT id,lessonId,requestId,role,content,status,sources,model,error,createdAt,upda…。
                "SELECT id,lessonId,requestId,role,content,status,sources,model,error,createdAt,updatedAt "
                // 【逐行批注】 执行当前声明或表达式："FROM recent ORDER BY rowid");。
                "FROM recent ORDER BY rowid");
    // 【逐行批注】 调用 text、integer 完成本行所需操作：text(s, 1, lessonId); integer(s, 2, limit);。
    text(s, 1, lessonId); integer(s, 2, limit);
    // 【逐行批注】 在条件成立期间循环处理：while (sqlite3_step(s.p) == SQLITE_ROW) {。
    while (sqlite3_step(s.p) == SQLITE_ROW) {
        // 【逐行批注】 调用 str 完成本行所需操作：result.push_back({str(s.p, 0), str(s.p, 1), str(s.p, 2), str(s.p, 3),…。
        result.push_back({str(s.p, 0), str(s.p, 1), str(s.p, 2), str(s.p, 3), str(s.p, 4),
            // 【逐行批注】 调用 str 完成本行所需操作：str(s.p, 5), str(s.p, 6), str(s.p, 7), str(s.p, 8), str(s.p, 9), str(…。
            str(s.p, 5), str(s.p, 6), str(s.p, 7), str(s.p, 8), str(s.p, 9), str(s.p, 10)});
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::upsertIndependentChatMessage，执行该函数声明的职责。
bool Database::upsertIndependentChatMessage(const IndependentChatMessage& value) {
    // 【逐行批注】 判断条件 (value.id.empty() || value.lessonId.empty() || value.role.empty(…，成立时执行对应分支。
    if (value.id.empty() || value.lessonId.empty() || value.role.empty() ||
        // 【逐行批注】 执行当前声明或表达式：value.createdAt.empty() || value.updatedAt.empty()) return false;。
        value.createdAt.empty() || value.updatedAt.empty()) return false;
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO IndependentChatMessage(id,lessonId,requestId,role,content,status,sources,model,error,createdAt,updatedAt) "
                // 【逐行批注】 调用 VALUES、CONFLICT 完成本行所需操作："VALUES(?,?,?,?,?,?,?,?,?,?,?) ON CONFLICT(id) DO UPDATE SET "。
                "VALUES(?,?,?,?,?,?,?,?,?,?,?) ON CONFLICT(id) DO UPDATE SET "
                // 【逐行批注】 计算右侧结果并写入 "content，更新当前运行状态。
                "content=excluded.content,status=excluded.status,sources=excluded.sources,"
                // 【逐行批注】 计算右侧结果并写入 "model，更新当前运行状态。
                "model=excluded.model,error=excluded.error,updatedAt=excluded.updatedAt");
    // 【逐行批注】 调用 text 完成本行所需操作：text(s, 1, value.id); text(s, 2, value.lessonId); text(s, 3, value.re…。
    text(s, 1, value.id); text(s, 2, value.lessonId); text(s, 3, value.requestId);
    // 【逐行批注】 调用 text 完成本行所需操作：text(s, 4, value.role); text(s, 5, value.content); text(s, 6, value.s…。
    text(s, 4, value.role); text(s, 5, value.content); text(s, 6, value.status);
    // 【逐行批注】 调用 text 完成本行所需操作：text(s, 7, value.sources); text(s, 8, value.model); text(s, 9, value.…。
    text(s, 7, value.sources); text(s, 8, value.model); text(s, 9, value.error);
    // 【逐行批注】 调用 text 完成本行所需操作：text(s, 10, value.createdAt); text(s, 11, value.updatedAt);。
    text(s, 10, value.createdAt); text(s, 11, value.updatedAt);
    // 【逐行批注】 结束当前函数并返回结果：return done(s);。
    return done(s);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::clearIndependentChat，执行该函数声明的职责。
bool Database::clearIndependentChat(const std::string& lessonId) {
    // 【逐行批注】 判断条件 (lessonId.empty()) return false;，成立时执行对应分支。
    if (lessonId.empty()) return false;
    // 【逐行批注】 计算右侧结果并写入 bool saved，更新当前运行状态。
    bool saved = false;
    // 【逐行批注】 调用 transaction 完成本行所需操作：transaction([&] {。
    transaction([&] {
        // 【逐行批注】 声明对象 messages，并通过构造函数初始化其状态。
        Stmt messages(db_, "DELETE FROM IndependentChatMessage WHERE lessonId=?");
        // 【逐行批注】 调用 text、done 完成本行所需操作：text(messages, 1, lessonId); done(messages);。
        text(messages, 1, lessonId); done(messages);
        // updatedAt 与消息时间使用同一 ISO 格式；不能用随机标识冒充时间。
        // 【逐行批注】 声明对象 session，并通过构造函数初始化其状态。
        Stmt session(db_, "UPDATE IndependentChat SET activeTaskId='',"
                          // 【逐行批注】 计算右侧结果并写入 "updatedAt，更新当前运行状态。
                          "updatedAt=strftime('%Y-%m-%dT%H:%M:%fZ','now') WHERE lessonId=?");
        // 【逐行批注】 调用 text、done 完成本行所需操作：text(session, 1, lessonId); done(session);。
        text(session, 1, lessonId); done(session);
        // 【逐行批注】 计算右侧结果并写入 saved，更新当前运行状态。
        saved = true;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 结束当前函数并返回结果：return saved;。
    return saved;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::getClassroomActivity，执行该函数声明的职责。
std::optional<ClassroomActivity> Database::getClassroomActivity(const std::string& key) const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT id,courseId,phaseIndex,topicIndex,kind,payload,updatedAt FROM ClassroomActivity WHERE id=?");text(s,1,key);
    // 【逐行批注】 调用 sqlite3_step 完成本行所需操作：if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;。
    if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;
    // 【逐行批注】 结束当前函数并返回结果：return ClassroomActivity{str(s.p,0),str(s.p,1),str(s.p,4),str(s.p,5),…。
    return ClassroomActivity{str(s.p,0),str(s.p,1),str(s.p,4),str(s.p,5),str(s.p,6),sqlite3_column_int(s.p,2),sqlite3_column_int(s.p,3)};
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::classroomPayloadSnapshot，执行该函数声明的职责。
std::vector<std::string> Database::classroomPayloadSnapshot(const std::vector<std::string>& keys) const {
    // 【逐行批注】 判断条件 (keys.empty() || keys.size() > 16) throw std::invalid_argument("…，成立时执行对应分支。
    if (keys.empty() || keys.size() > 16) throw std::invalid_argument("课堂快照参数无效");
    // 【逐行批注】 计算右侧结果并写入 std::string sql，更新当前运行状态。
    std::string sql = "SELECT ";
    // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < keys.size(); ++index) {。
    for (size_t index = 0; index < keys.size(); ++index) {
        // 【逐行批注】 判断条件 (index) sql += ',';，成立时执行对应分支。
        if (index) sql += ',';
        // 【逐行批注】 计算右侧结果并写入 sql +，更新当前运行状态。
        sql += "(SELECT payload FROM ClassroomActivity WHERE id=?)";
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 声明对象 statement，并通过构造函数初始化其状态。
    Stmt statement(db_, sql.c_str());
    // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < keys.size(); ++index) text(statement, stati…。
    for (size_t index = 0; index < keys.size(); ++index) text(statement, static_cast<int>(index + 1), keys[index]);
    // 【逐行批注】 计算右侧结果并写入 int result，更新当前运行状态。
    const int result = sqlite3_step(statement.p); check(result, db_, "read classroom snapshot");
    // 【逐行批注】 判断条件 (result != SQLITE_ROW) throw std::runtime_error("课堂状态暂不可读");，成立时执行对应分支。
    if (result != SQLITE_ROW) throw std::runtime_error("课堂状态暂不可读");
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> values;。
    std::vector<std::string> values;
    // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < keys.size(); ++index) values.push_back(str(…。
    for (size_t index = 0; index < keys.size(); ++index) values.push_back(str(statement.p, static_cast<int>(index)));
    // 【逐行批注】 结束当前函数并返回结果：return values;。
    return values;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::listClassroomActivities，执行该函数声明的职责。
std::vector<ClassroomActivity> Database::listClassroomActivities(const std::string& courseId) const {
    // 【逐行批注】 执行当前声明或表达式：std::vector<ClassroomActivity> rows;。
    std::vector<ClassroomActivity> rows;
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT id,courseId,phaseIndex,topicIndex,kind,payload,updatedAt FROM ClassroomActivity WHERE courseId=? ORDER BY rowid");text(s,1,courseId);
    // 【逐行批注】 在条件成立期间循环处理：while(sqlite3_step(s.p)==SQLITE_ROW)rows.push_back({str(s.p,0),str(s.p,1),…。
    while(sqlite3_step(s.p)==SQLITE_ROW)rows.push_back({str(s.p,0),str(s.p,1),str(s.p,4),str(s.p,5),str(s.p,6),sqlite3_column_int(s.p,2),sqlite3_column_int(s.p,3)});
    // 【逐行批注】 结束当前函数并返回结果：return rows;。
    return rows;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::upsert，执行该函数声明的职责。
bool Database::upsert(const WeeklyPlan& value) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO WeeklyPlan(courseId,payload,version,updatedAt) VALUES(?,?,?,?) ON CONFLICT(courseId) DO UPDATE SET payload=excluded.payload,version=excluded.version,updatedAt=excluded.updatedAt");
    // 【逐行批注】 调用 text、integer、done 完成本行所需操作：text(s,1,value.courseId);text(s,2,value.payload);integer(s,3,value.ve…。
    text(s,1,value.courseId);text(s,2,value.payload);integer(s,3,value.version);text(s,4,value.updatedAt);return done(s);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::getWeeklyPlan，执行该函数声明的职责。
std::optional<WeeklyPlan> Database::getWeeklyPlan(const std::string& courseId) const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT courseId,payload,version,updatedAt FROM WeeklyPlan WHERE courseId=?");text(s,1,courseId);
    // 【逐行批注】 调用 sqlite3_step 完成本行所需操作：if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;。
    if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;
    // 【逐行批注】 结束当前函数并返回结果：return WeeklyPlan{str(s.p,0),str(s.p,1),str(s.p,3),sqlite3_column_int…。
    return WeeklyPlan{str(s.p,0),str(s.p,1),str(s.p,3),sqlite3_column_int(s.p,2)};
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::deleteClassroomData，执行该函数声明的职责。
void Database::deleteClassroomData(const std::string& courseId) {
    // 图片字节独立于课堂列表，避免每次教学读取整批图片；删除课程时一并清理。
    // 【逐行批注】 声明对象 images，并通过构造函数初始化其状态。
    Stmt images(db_, "DELETE FROM MyAIRecord WHERE kind='deep-image' AND CASE WHEN json_valid(payload) THEN json_extract(payload,'$.courseId') END=?");
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(images,1,courseId);done(images);。
    text(images,1,courseId);done(images);
    // 【逐行批注】 声明对象 events，并通过构造函数初始化其状态。
    Stmt events(db_, "DELETE FROM AgentRunEvent WHERE taskId IN "
                     // 【逐行批注】 计算右侧结果并写入 "(SELECT id FROM ClassroomActivity WHERE cou…，更新当前运行状态。
                     "(SELECT id FROM ClassroomActivity WHERE courseId=?)");
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(events,1,courseId);done(events);。
    text(events,1,courseId);done(events);
    // 【逐行批注】 声明对象 independentMessages，并通过构造函数初始化其状态。
    Stmt independentMessages(db_, "DELETE FROM IndependentChatMessage WHERE lessonId IN "
        // 【逐行批注】 计算右侧结果并写入 "(SELECT id FROM ClassroomActivity WHERE cou…，更新当前运行状态。
        "(SELECT id FROM ClassroomActivity WHERE courseId=? AND kind='agent-lesson')");
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(independentMessages,1,courseId);done(independentMessages);。
    text(independentMessages,1,courseId);done(independentMessages);
    // 【逐行批注】 声明对象 independentChats，并通过构造函数初始化其状态。
    Stmt independentChats(db_, "DELETE FROM IndependentChat WHERE lessonId IN "
        // 【逐行批注】 计算右侧结果并写入 "(SELECT id FROM ClassroomActivity WHERE cou…，更新当前运行状态。
        "(SELECT id FROM ClassroomActivity WHERE courseId=? AND kind='agent-lesson')");
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(independentChats,1,courseId);done(independentChats);。
    text(independentChats,1,courseId);done(independentChats);
    // 【逐行批注】 声明对象 activities，并通过构造函数初始化其状态。
    Stmt activities(db_, "DELETE FROM ClassroomActivity WHERE courseId=?");text(activities,1,courseId);done(activities);
    // 【逐行批注】 声明对象 weekly，并通过构造函数初始化其状态。
    Stmt weekly(db_, "DELETE FROM WeeklyPlan WHERE courseId=?");text(weekly,1,courseId);done(weekly);
    // 【逐行批注】 计算右侧结果并写入 auto prefix，更新当前运行状态。
    const auto prefix = "lesson-reached:" + courseId + ":";
    // 【逐行批注】 声明对象 reached，并通过构造函数初始化其状态。
    Stmt reached(db_, "DELETE FROM ProfileMeta WHERE substr(key,1,length(?))=?");
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(reached,1,prefix);text(reached,2,prefix);done(reached);。
    text(reached,1,prefix);text(reached,2,prefix);done(reached);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::deleteConversation，执行该函数声明的职责。
bool Database::deleteConversation(const std::string& conversationId) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "DELETE FROM LearningInteraction WHERE conversationId=?");text(s,1,conversationId);
    // 【逐行批注】 计算右侧结果并写入 bool changed，更新当前运行状态。
    const bool changed=done(s);return changed;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::deleteInteractionsForCourse，执行该函数声明的职责。
bool Database::deleteInteractionsForCourse(const std::string& courseId) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "DELETE FROM LearningInteraction WHERE courseId=?");text(s,1,courseId);
    // 【逐行批注】 计算右侧结果并写入 bool changed，更新当前运行状态。
    const bool changed=done(s);if(changed)markProfileDirty();return changed;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::upsert，执行该函数声明的职责。
bool Database::upsert(const SubjectMastery& value) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO SubjectMastery(subject,score,rationale,weakPoints,recommendation,evidenceCount,model,status,updatedAt) VALUES(?,?,?,?,?,?,?,?,?) ON CONFLICT(subject) DO UPDATE SET score=excluded.score,rationale=excluded.rationale,weakPoints=excluded.weakPoints,recommendation=excluded.recommendation,evidenceCount=excluded.evidenceCount,model=excluded.model,status=excluded.status,updatedAt=excluded.updatedAt");
    // 【逐行批注】 调用 text、optint 完成本行所需操作：text(s,1,value.subject);optint(s,2,value.score);text(s,3,value.ration…。
    text(s,1,value.subject);optint(s,2,value.score);text(s,3,value.rationale);text(s,4,value.weakPoints);
    // 【逐行批注】 调用 text、integer 完成本行所需操作：text(s,5,value.recommendation);integer(s,6,value.evidenceCount);text(…。
    text(s,5,value.recommendation);integer(s,6,value.evidenceCount);text(s,7,value.model);
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(s,8,value.status);text(s,9,value.updatedAt);return done(s);。
    text(s,8,value.status);text(s,9,value.updatedAt);return done(s);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::upsert，执行该函数声明的职责。
bool Database::upsert(const TopicMastery& value) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO TopicMastery(courseId,phaseIndex,topic,score,evidenceCount,rationale,weakPoints,recommendation,evidenceIds,nextReviewAt,model,status,version,updatedAt) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?) ON CONFLICT(courseId,phaseIndex,topic) DO UPDATE SET score=excluded.score,evidenceCount=excluded.evidenceCount,rationale=excluded.rationale,weakPoints=excluded.weakPoints,recommendation=excluded.recommendation,evidenceIds=excluded.evidenceIds,nextReviewAt=excluded.nextReviewAt,model=excluded.model,status=excluded.status,version=excluded.version,updatedAt=excluded.updatedAt");
    // 【逐行批注】 调用 text、integer、optint 完成本行所需操作：text(s,1,value.courseId);integer(s,2,value.phaseIndex);text(s,3,value…。
    text(s,1,value.courseId);integer(s,2,value.phaseIndex);text(s,3,value.topic);optint(s,4,value.score);
    // 【逐行批注】 调用 integer、text 完成本行所需操作：integer(s,5,value.evidenceCount);text(s,6,value.rationale);text(s,7,v…。
    integer(s,5,value.evidenceCount);text(s,6,value.rationale);text(s,7,value.weakPoints);
    // 【逐行批注】 调用 text 完成本行所需操作：text(s,8,value.recommendation);text(s,9,value.evidenceIds);text(s,10,…。
    text(s,8,value.recommendation);text(s,9,value.evidenceIds);text(s,10,value.nextReviewAt);
    // 【逐行批注】 调用 text、integer 完成本行所需操作：text(s,11,value.model);text(s,12,value.status);integer(s,13,value.ver…。
    text(s,11,value.model);text(s,12,value.status);integer(s,13,value.version);text(s,14,value.updatedAt);
    // 【逐行批注】 结束当前函数并返回结果：return done(s);。
    return done(s);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::listTopicMastery，执行该函数声明的职责。
std::vector<TopicMastery> Database::listTopicMastery() const {
    // 【逐行批注】 执行当前声明或表达式：std::vector<TopicMastery> rows;。
    std::vector<TopicMastery> rows;
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT courseId,phaseIndex,topic,score,evidenceCount,rationale,weakPoints,recommendation,evidenceIds,nextReviewAt,model,status,version,updatedAt FROM TopicMastery ORDER BY courseId,phaseIndex,topic");
    // 【逐行批注】 在条件成立期间循环处理：while (sqlite3_step(s.p) == SQLITE_ROW) {。
    while (sqlite3_step(s.p) == SQLITE_ROW) {
        // 【逐行批注】 执行当前声明或表达式：TopicMastery value;。
        TopicMastery value;
        // 【逐行批注】 计算右侧结果并写入 value.courseId，更新当前运行状态。
        value.courseId=str(s.p,0);value.phaseIndex=sqlite3_column_int(s.p,1);value.topic=str(s.p,2);
        // 【逐行批注】 计算右侧结果并写入 value.score，更新当前运行状态。
        value.score=oint(s.p,3);value.evidenceCount=sqlite3_column_int(s.p,4);value.rationale=str(s.p,5);
        // 【逐行批注】 计算右侧结果并写入 value.weakPoints，更新当前运行状态。
        value.weakPoints=str(s.p,6);value.recommendation=str(s.p,7);value.evidenceIds=str(s.p,8);
        // 【逐行批注】 计算右侧结果并写入 value.nextReviewAt，更新当前运行状态。
        value.nextReviewAt=str(s.p,9);value.model=str(s.p,10);value.status=str(s.p,11);
        // 【逐行批注】 计算右侧结果并写入 value.version，更新当前运行状态。
        value.version=sqlite3_column_int(s.p,12);value.updatedAt=str(s.p,13);rows.push_back(std::move(value));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return rows;。
    return rows;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::deleteTopicMasteryForCourse，执行该函数声明的职责。
bool Database::deleteTopicMasteryForCourse(const std::string& courseId) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "DELETE FROM TopicMastery WHERE courseId=?");
    // 【逐行批注】 调用 text 完成本行所需操作：text(s,1,courseId);。
    text(s,1,courseId);
    // 【逐行批注】 结束当前函数并返回结果：return done(s);。
    return done(s);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::listMastery，执行该函数声明的职责。
std::vector<SubjectMastery> Database::listMastery() const {
    // 【逐行批注】 执行当前声明或表达式：std::vector<SubjectMastery> rows;。
    std::vector<SubjectMastery> rows;
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT subject,score,rationale,weakPoints,recommendation,evidenceCount,model,status,updatedAt FROM SubjectMastery ORDER BY subject");
    // 【逐行批注】 在条件成立期间循环处理：while(sqlite3_step(s.p)==SQLITE_ROW) rows.push_back({str(s.p,0),str(s.p,2)…。
    while(sqlite3_step(s.p)==SQLITE_ROW) rows.push_back({str(s.p,0),str(s.p,2),str(s.p,3),str(s.p,4),str(s.p,6),str(s.p,7),str(s.p,8),oint(s.p,1),sqlite3_column_int(s.p,5)});
    // 【逐行批注】 结束当前函数并返回结果：return rows;。
    return rows;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::replaceMastery，执行该函数声明的职责。
void Database::replaceMastery(const std::vector<SubjectMastery>& values) {
    // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "BEGIN IMMEDIATE");。
    exec(db_, "BEGIN IMMEDIATE");
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "DELETE FROM SubjectMastery");。
        exec(db_, "DELETE FROM SubjectMastery");
        // 【逐行批注】 按循环规则重复处理：for(const auto& value:values) upsert(value);。
        for(const auto& value:values) upsert(value);
        // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "COMMIT");。
        exec(db_, "COMMIT");
    // 【逐行批注】 调用 exec 完成本行所需操作：} catch (...) { exec(db_, "ROLLBACK"); throw; }。
    } catch (...) { exec(db_, "ROLLBACK"); throw; }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::profileDirty，执行该函数声明的职责。
bool Database::profileDirty() const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT (SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='revision') > (SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='assessed')");
    // 【逐行批注】 结束当前函数并返回结果：return sqlite3_step(s.p)==SQLITE_ROW && sqlite3_column_int(s.p,0)!=0;。
    return sqlite3_step(s.p)==SQLITE_ROW && sqlite3_column_int(s.p,0)!=0;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::profileRevision，执行该函数声明的职责。
int Database::profileRevision() const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='revision'");
    // 【逐行批注】 结束当前函数并返回结果：return sqlite3_step(s.p)==SQLITE_ROW ? sqlite3_column_int(s.p,0) : 0;。
    return sqlite3_step(s.p)==SQLITE_ROW ? sqlite3_column_int(s.p,0) : 0;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::profileAssessedRevision，执行该函数声明的职责。
int Database::profileAssessedRevision() const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='assessed'");
    // 【逐行批注】 结束当前函数并返回结果：return sqlite3_step(s.p)==SQLITE_ROW ? sqlite3_column_int(s.p,0) : 0;。
    return sqlite3_step(s.p)==SQLITE_ROW ? sqlite3_column_int(s.p,0) : 0;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::markProfileDirty，执行该函数声明的职责。
void Database::markProfileDirty() {
    // 【逐行批注】 计算右侧结果并写入 exec(db_, "UPDATE ProfileMeta SET value，更新当前运行状态。
    exec(db_, "UPDATE ProfileMeta SET value=CAST(value AS INTEGER)+1 WHERE key='revision'");
    // 【逐行批注】 调用 setProfileError 完成本行所需操作：setProfileError("");。
    setProfileError("");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::setProfileAssessed，执行该函数声明的职责。
void Database::setProfileAssessed(int revision) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "UPDATE ProfileMeta SET value=? WHERE key='assessed'");
    // 【逐行批注】 调用 text、to_string、done 完成本行所需操作：text(s,1,std::to_string(revision));done(s);。
    text(s,1,std::to_string(revision));done(s);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::profileError，执行该函数声明的职责。
std::string Database::profileError() const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT value FROM ProfileMeta WHERE key='error'");
    // 【逐行批注】 结束当前函数并返回结果：return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "";。
    return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "";
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::replaceMasteryAtRevision，执行该函数声明的职责。
bool Database::replaceMasteryAtRevision(const std::vector<SubjectMastery>& values, int revision) {
    // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "BEGIN IMMEDIATE");。
    exec(db_, "BEGIN IMMEDIATE");
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (profileRevision() != revision) { exec(db_, "ROLLBACK"); return …，成立时执行对应分支。
        if (profileRevision() != revision) { exec(db_, "ROLLBACK"); return false; }
        // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "DELETE FROM SubjectMastery");。
        exec(db_, "DELETE FROM SubjectMastery");
        // 【逐行批注】 按循环规则重复处理：for (const auto& value : values) upsert(value);。
        for (const auto& value : values) upsert(value);
        // 【逐行批注】 调用 setProfileAssessed 完成本行所需操作：setProfileAssessed(revision);。
        setProfileAssessed(revision);
        // 【逐行批注】 调用 setProfileError 完成本行所需操作：setProfileError("");。
        setProfileError("");
        // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "COMMIT");。
        exec(db_, "COMMIT");
        // 【逐行批注】 结束当前函数并返回结果：return true;。
        return true;
    // 【逐行批注】 调用 exec 完成本行所需操作：} catch (...) { exec(db_, "ROLLBACK"); throw; }。
    } catch (...) { exec(db_, "ROLLBACK"); throw; }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::upsertTopicMasteryAtRevision，执行该函数声明的职责。
bool Database::upsertTopicMasteryAtRevision(const TopicMastery& value, int revision) {
    // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "BEGIN IMMEDIATE");。
    exec(db_, "BEGIN IMMEDIATE");
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (profileRevision() != revision) { exec(db_, "ROLLBACK"); return …，成立时执行对应分支。
        if (profileRevision() != revision) { exec(db_, "ROLLBACK"); return false; }
        // 【逐行批注】 调用 upsert 完成本行所需操作：upsert(value);。
        upsert(value);
        // 【逐行批注】 调用 exec 完成本行所需操作：exec(db_, "COMMIT");。
        exec(db_, "COMMIT");
        // 【逐行批注】 结束当前函数并返回结果：return true;。
        return true;
    // 【逐行批注】 调用 exec 完成本行所需操作：} catch (...) { exec(db_, "ROLLBACK"); throw; }。
    } catch (...) { exec(db_, "ROLLBACK"); throw; }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::profileMeta，执行该函数声明的职责。
std::string Database::profileMeta(const std::string& key) const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT value FROM ProfileMeta WHERE key=?");
    // 【逐行批注】 调用 text 完成本行所需操作：text(s,1,key);。
    text(s,1,key);
    // 【逐行批注】 结束当前函数并返回结果：return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "";。
    return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "";
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::setProfileMeta，执行该函数声明的职责。
void Database::setProfileMeta(const std::string& key, const std::string& value) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO ProfileMeta(key,value) VALUES(?,?) ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(s,1,key);text(s,2,value);done(s);。
    text(s,1,key);text(s,2,value);done(s);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::setProfileMetaAtRevision，执行该函数声明的职责。
bool Database::setProfileMetaAtRevision(const std::string& key, const std::string& value, int revision) {
    // 单条 SQL 校验版本并保存，避免新作答写入后被旧 AI 结果覆盖。
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO ProfileMeta(key,value) SELECT ?,? WHERE (SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='revision')=? ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    // 【逐行批注】 调用 text、integer、done 完成本行所需操作：text(s,1,key);text(s,2,value);integer(s,3,revision);done(s);。
    text(s,1,key);text(s,2,value);integer(s,3,revision);done(s);
    // 【逐行批注】 结束当前函数并返回结果：return sqlite3_changes(db_) == 1;。
    return sqlite3_changes(db_) == 1;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::setProfileError，执行该函数声明的职责。
void Database::setProfileError(const std::string& error) {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "INSERT INTO ProfileMeta(key,value) VALUES('error',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(s,1,error);done(s);。
    text(s,1,error);done(s);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::profileSearchStatus，执行该函数声明的职责。
std::string Database::profileSearchStatus() const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT value FROM ProfileMeta WHERE key='searchStatus'");
    // 【逐行批注】 结束当前函数并返回结果：return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "not_requested";。
    return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "not_requested";
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::profileSources，执行该函数声明的职责。
std::string Database::profileSources() const {
    // 【逐行批注】 声明对象 s，并通过构造函数初始化其状态。
    Stmt s(db_, "SELECT value FROM ProfileMeta WHERE key='sources'");
    // 【逐行批注】 结束当前函数并返回结果：return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "[]";。
    return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "[]";
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 Database::setProfileSearch，执行该函数声明的职责。
void Database::setProfileSearch(const std::string& status, const std::string& sources) {
    // 【逐行批注】 声明对象 state，并通过构造函数初始化其状态。
    Stmt state(db_, "INSERT INTO ProfileMeta(key,value) VALUES('searchStatus',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(state,1,status);done(state);。
    text(state,1,status);done(state);
    // 【逐行批注】 声明对象 links，并通过构造函数初始化其状态。
    Stmt links(db_, "INSERT INTO ProfileMeta(key,value) VALUES('sources',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    // 【逐行批注】 调用 text、done 完成本行所需操作：text(links,1,sources);done(links);。
    text(links,1,sources);done(links);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

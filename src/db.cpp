#include "db.hpp"
#include <nlohmann/json.hpp>
#include "db_schema_version.hpp"

#include <sqlite3.h>
#include <filesystem>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>

namespace gangyi {
namespace {
void check(int rc, sqlite3* db, const char* action) { if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw std::runtime_error(std::string(action) + ": " + sqlite3_errmsg(db)); }
void exec(sqlite3* db, const char* sql) { char* error = nullptr; const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &error); if (rc != SQLITE_OK) { std::string message = error ? error : sqlite3_errmsg(db); sqlite3_free(error); throw std::runtime_error(message); } }
std::string id() { static thread_local std::mt19937_64 rng(std::random_device{}()); std::ostringstream out; out << std::hex << std::setfill('0'); for (int i = 0; i < 2; ++i) out << std::setw(16) << rng(); return out.str(); }
struct Stmt { sqlite3_stmt* p = nullptr; sqlite3* db; Stmt(sqlite3* d, const char* sql) : db(d) { check(sqlite3_prepare_v2(db, sql, -1, &p, nullptr), db, "prepare"); } ~Stmt() { sqlite3_finalize(p); } };
void text(Stmt& s, int n, const std::string& v) { check(sqlite3_bind_text(s.p, n, v.c_str(), -1, SQLITE_TRANSIENT), s.db, "bind"); }
void opt(Stmt& s, int n, const std::optional<std::string>& v) { if (v) text(s, n, *v); else check(sqlite3_bind_null(s.p, n), s.db, "bind"); }
void integer(Stmt& s, int n, int v) { check(sqlite3_bind_int(s.p, n, v), s.db, "bind"); }
void optint(Stmt& s, int n, const std::optional<int>& v) { if (v) integer(s, n, *v); else check(sqlite3_bind_null(s.p, n), s.db, "bind"); }
std::string str(sqlite3_stmt* s, int n) { const auto* p = sqlite3_column_text(s, n); return p ? reinterpret_cast<const char*>(p) : ""; }
std::optional<std::string> ostr(sqlite3_stmt* s, int n) { const auto* p = sqlite3_column_text(s, n); return p ? std::optional<std::string>(reinterpret_cast<const char*>(p)) : std::nullopt; }
std::optional<int> oint(sqlite3_stmt* s, int n) { return sqlite3_column_type(s, n) == SQLITE_NULL ? std::nullopt : std::optional<int>(sqlite3_column_int(s, n)); }
bool done(Stmt& s) { const int rc = sqlite3_step(s.p); check(rc, s.db, "execute"); return sqlite3_changes(s.db) > 0; }
}

Database::~Database() { close(); }
void Database::transaction(const std::function<void()>& action) {
    exec(db_, "BEGIN IMMEDIATE");
    try {
        action();
        exec(db_, "COMMIT");
    } catch (...) {
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        throw;
    }
}
int Database::learningRevision() const {
    const auto value = profileMeta("learning-revision");
    try { return value.empty() ? 1 : std::stoi(value); } catch (...) { return 1; }
}
void Database::markLearningDirty() {
    exec(db_, "INSERT INTO ProfileMeta(key,value) VALUES('learning-revision','2') ON CONFLICT(key) DO UPDATE SET value=CAST(value AS INTEGER)+1");
}
bool Database::compareProfileMeta(const std::string& key, const std::string& expected, const std::string& value) {
    Stmt s(db_, "INSERT INTO ProfileMeta(key,value) SELECT ?,? WHERE COALESCE((SELECT value FROM ProfileMeta WHERE key=?),'')=? ON CONFLICT(key) DO UPDATE SET value=excluded.value WHERE ProfileMeta.value=? RETURNING key");
    text(s,1,key);text(s,2,value);text(s,3,key);text(s,4,expected);text(s,5,expected);
    const int result = sqlite3_step(s.p); check(result, db_, "compare and save"); return result == SQLITE_ROW;
}
bool Database::compareClassroomActivity(const ClassroomActivity& value, const std::string& expected,
                                        const std::string& guardId, const std::string& guardPayload) {
    Stmt s(db_, "INSERT INTO ClassroomActivity(id,courseId,phaseIndex,topicIndex,kind,payload,updatedAt) SELECT ?,?,?,?,?,?,? WHERE COALESCE((SELECT payload FROM ClassroomActivity WHERE id=?),'')=? AND EXISTS(SELECT 1 FROM Course WHERE id=? AND status='active') AND (?='' OR EXISTS(SELECT 1 FROM ClassroomActivity WHERE id=? AND payload=?)) ON CONFLICT(id) DO UPDATE SET payload=excluded.payload,updatedAt=excluded.updatedAt WHERE ClassroomActivity.payload=? RETURNING id");
    text(s,1,value.id);text(s,2,value.courseId);integer(s,3,value.phaseIndex);integer(s,4,value.topicIndex);
    text(s,5,value.kind);text(s,6,value.payload);text(s,7,value.updatedAt);text(s,8,value.id);
    text(s,9,expected);text(s,10,value.courseId);text(s,11,guardId);text(s,12,guardId);text(s,13,guardPayload);text(s,14,expected);
    const int result = sqlite3_step(s.p); check(result, db_, "compare and save"); return result == SQLITE_ROW;
}
bool Database::updateLearningSessionAtRevision(const LearningSession& value, const std::string& expected,
                                               int revision, const std::string& exposureKey,
                                               const std::string& exposure) {
    Stmt s(db_, "UPDATE LearningSession SET content=?,title=?,summary=?,`references`=? WHERE id=? AND content=? AND COALESCE((SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='learning-revision'),1)=? AND COALESCE((SELECT value FROM ProfileMeta WHERE key=?),'')=? AND EXISTS(SELECT 1 FROM Course WHERE id=LearningSession.courseId AND status='active') RETURNING id");
    text(s,1,value.content);text(s,2,value.title);opt(s,3,value.summary);opt(s,4,value.references);
    text(s,5,value.id);text(s,6,expected);integer(s,7,revision);text(s,8,exposureKey);text(s,9,exposure);
    const int result = sqlite3_step(s.p); check(result, db_, "compare and save"); return result == SQLITE_ROW;
}
void Database::open(const std::string& path) { close(); check(sqlite3_open(path.c_str(), &db_), db_, "open database"); sqlite3_extended_result_codes(db_, 1); sqlite3_busy_timeout(db_, 5000); }
void Database::close() { if (db_) { sqlite3_close(db_); db_ = nullptr; } }
void Database::migrate() {
    if (!db_) throw std::runtime_error("database is not open");
    sqlite3_stmt* versionStatement = nullptr;
    check(sqlite3_prepare_v2(db_, "PRAGMA user_version", -1, &versionStatement, nullptr), db_, "read schema version");
    const int versionResult = sqlite3_step(versionStatement);
    const int currentVersion = versionResult == SQLITE_ROW ? sqlite3_column_int(versionStatement, 0) : -1;
    sqlite3_finalize(versionStatement);
    if (versionResult != SQLITE_ROW || currentVersion > kDatabaseSchemaVersion) {
        throw std::runtime_error("database schema is newer than this application");
    }
    if (currentVersion > 0 && currentVersion < kDatabaseSchemaVersion) {
        const char* filename = sqlite3_db_filename(db_, "main");
        if (filename && *filename && std::string(filename) != ":memory:") {
            const auto backupPath = std::filesystem::u8path(filename).u8string() +
                (currentVersion < 4 ? ".pre-v4.db" : currentVersion < 5 ? ".pre-v5.db" : ".pre-v6.db");
            if (!std::filesystem::exists(std::filesystem::u8path(backupPath))) {
                sqlite3* backupDb = nullptr;
                check(sqlite3_open(backupPath.c_str(), &backupDb), backupDb, "open migration backup");
                sqlite3_backup* backup = sqlite3_backup_init(backupDb, "main", db_, "main");
                if (!backup) { sqlite3_close(backupDb); throw std::runtime_error("migration backup failed"); }
                const int result = sqlite3_backup_step(backup, -1);
                const int finishResult = sqlite3_backup_finish(backup);
                sqlite3_close(backupDb);
                if (result != SQLITE_DONE || finishResult != SQLITE_OK) {
                    std::error_code ignored;
                    std::filesystem::remove(std::filesystem::u8path(backupPath), ignored);
                    throw std::runtime_error("migration backup failed");
                }
            }
        }
    }
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
    if (currentVersion < 3) {
        exec(db_, "UPDATE LearningSession SET fallbackUsed=1, source='legacy' "
            "WHERE source<>'ai' OR fallbackUsed<>0 OR content NOT LIKE '%\"promptVersion\":\"ai-block-v1\"%'");
    }
    if (currentVersion < 4) {
        exec(db_, "BEGIN IMMEDIATE");
        try {
            exec(db_, R"SQL(
CREATE TABLE IF NOT EXISTS LearningInteraction(id TEXT PRIMARY KEY, courseId TEXT, conversationId TEXT, subject TEXT, kind TEXT NOT NULL, payload TEXT NOT NULL, createdAt TEXT NOT NULL);
CREATE INDEX IF NOT EXISTS LearningInteraction_course_idx ON LearningInteraction(courseId);
CREATE INDEX IF NOT EXISTS LearningInteraction_conversation_idx ON LearningInteraction(conversationId,createdAt);
CREATE TABLE IF NOT EXISTS SubjectMastery(subject TEXT PRIMARY KEY, score INTEGER CHECK(score BETWEEN 0 AND 100), rationale TEXT NOT NULL DEFAULT '', weakPoints TEXT NOT NULL DEFAULT '[]', recommendation TEXT NOT NULL DEFAULT '', evidenceCount INTEGER NOT NULL DEFAULT 0, model TEXT NOT NULL DEFAULT '', status TEXT NOT NULL DEFAULT 'pending', updatedAt TEXT NOT NULL DEFAULT '');
CREATE TABLE IF NOT EXISTS ProfileMeta(key TEXT PRIMARY KEY, value TEXT NOT NULL);
INSERT OR IGNORE INTO ProfileMeta(key,value) VALUES('revision','1');
INSERT OR IGNORE INTO ProfileMeta(key,value) VALUES('assessed','0');
INSERT OR IGNORE INTO ProfileMeta(key,value) VALUES('error','');
)SQL");
            exec(db_, "PRAGMA user_version=4");
            exec(db_, "COMMIT");
        } catch (...) { exec(db_, "ROLLBACK"); throw; }
    }
    if (currentVersion < 5) {
        exec(db_, "BEGIN IMMEDIATE");
        try {
            exec(db_, R"SQL(
CREATE TABLE IF NOT EXISTS TopicMastery(courseId TEXT NOT NULL, phaseIndex INTEGER NOT NULL, topic TEXT NOT NULL,
 score INTEGER CHECK(score BETWEEN 0 AND 100), evidenceCount INTEGER NOT NULL DEFAULT 0,
 rationale TEXT NOT NULL DEFAULT '', weakPoints TEXT NOT NULL DEFAULT '[]', recommendation TEXT NOT NULL DEFAULT '',
 evidenceIds TEXT NOT NULL DEFAULT '[]', nextReviewAt TEXT NOT NULL DEFAULT '', model TEXT NOT NULL DEFAULT '',
 status TEXT NOT NULL DEFAULT 'insufficient', version INTEGER NOT NULL DEFAULT 0, updatedAt TEXT NOT NULL DEFAULT '',
 PRIMARY KEY(courseId,phaseIndex,topic));
UPDATE SubjectMastery SET status='legacy' WHERE status='ready';
)SQL");
            exec(db_, "PRAGMA user_version=5");
            exec(db_, "COMMIT");
        } catch (...) { exec(db_, "ROLLBACK"); throw; }
    }
    if (currentVersion < 6) {
        exec(db_, "BEGIN IMMEDIATE");
        try {
            exec(db_, R"SQL(
CREATE TABLE IF NOT EXISTS ClassroomActivity(id TEXT PRIMARY KEY, courseId TEXT NOT NULL, phaseIndex INTEGER NOT NULL, topicIndex INTEGER NOT NULL, kind TEXT NOT NULL, payload TEXT NOT NULL, updatedAt TEXT NOT NULL);
CREATE INDEX IF NOT EXISTS ClassroomActivity_lesson_idx ON ClassroomActivity(courseId,phaseIndex,topicIndex,kind);
CREATE TABLE IF NOT EXISTS WeeklyPlan(courseId TEXT PRIMARY KEY, payload TEXT NOT NULL, version INTEGER NOT NULL DEFAULT 1, updatedAt TEXT NOT NULL);
)SQL");
            exec(db_, "PRAGMA user_version=6");
            exec(db_, "COMMIT");
        } catch (...) { exec(db_, "ROLLBACK"); throw; }
    }
}

// Course and User are kept explicit below; the repeated progress tables use the same SQL shape.
static void bind_Course(Stmt&s,const Course&v){text(s,1,v.id);opt(s,2,v.anonymousId);opt(s,3,v.userId);text(s,4,v.goal);text(s,5,v.mode);text(s,6,v.title);opt(s,7,v.summary);text(s,8,v.source);text(s,9,v.status);text(s,10,v.createdAt);text(s,11,v.updatedAt);}
static Course map_Course(sqlite3_stmt*s){Course v;v.id=str(s,0);v.anonymousId=ostr(s,1);v.userId=ostr(s,2);v.goal=str(s,3);v.mode=str(s,4);v.title=str(s,5);v.summary=ostr(s,6);v.source=str(s,7);v.status=str(s,8);v.createdAt=str(s,9);v.updatedAt=str(s,10);return v;}
bool Database::insert(const Course& v){ if(v.id.empty()) { Course copy=v; copy.id=id(); return insert(copy); } Stmt s(db_,"INSERT INTO Course(id,anonymousId,userId,goal,mode,title,summary,source,status,createdAt,updatedAt) VALUES(?,?,?,?,?,?,?,?,?,?,?)");bind_Course(s,v);return done(s);}
std::optional<Course> Database::getCourse(const std::string& key) const {Stmt s(db_,"SELECT id,anonymousId,userId,goal,mode,title,summary,source,status,createdAt,updatedAt FROM Course WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_Course(s.p);}
std::vector<Course> Database::listCourses() const {std::vector<Course> r;Stmt s(db_,"SELECT id,anonymousId,userId,goal,mode,title,summary,source,status,createdAt,updatedAt FROM Course ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_Course(s.p));return r;}
bool Database::update(const Course&v){Stmt s(db_,"UPDATE Course SET anonymousId=?,userId=?,goal=?,mode=?,title=?,summary=?,source=?,status=?,createdAt=?,updatedAt=? WHERE id=?");opt(s,1,v.anonymousId);opt(s,2,v.userId);text(s,3,v.goal);text(s,4,v.mode);text(s,5,v.title);opt(s,6,v.summary);text(s,7,v.source);text(s,8,v.status);text(s,9,v.createdAt);text(s,10,v.updatedAt);text(s,11,v.id);return done(s);}
bool Database::deleteCourse(const std::string&key){Stmt s(db_,"DELETE FROM Course WHERE id=?");text(s,1,key);return done(s);}

static void bind_User(Stmt&s,const User&v){text(s,1,v.id);text(s,2,v.email);opt(s,3,v.name);text(s,4,v.passwordHash);text(s,5,v.membershipTier);text(s,6,v.membershipStatus);opt(s,7,v.membershipStartedAt);opt(s,8,v.membershipExpiresAt);text(s,9,v.createdAt);text(s,10,v.updatedAt);}
static User map_User(sqlite3_stmt*s){User v;v.id=str(s,0);v.email=str(s,1);v.name=ostr(s,2);v.passwordHash=str(s,3);v.membershipTier=str(s,4);v.membershipStatus=str(s,5);v.membershipStartedAt=ostr(s,6);v.membershipExpiresAt=ostr(s,7);v.createdAt=str(s,8);v.updatedAt=str(s,9);return v;}
bool Database::insert(const User&v){Stmt s(db_,"INSERT INTO User(id,email,name,passwordHash,membershipTier,membershipStatus,membershipStartedAt,membershipExpiresAt,createdAt,updatedAt) VALUES(?,?,?,?,?,?,?,?,?,?)");bind_User(s,v);return done(s);}
std::optional<User> Database::getUser(const std::string&key)const{Stmt s(db_,"SELECT id,email,name,passwordHash,membershipTier,membershipStatus,membershipStartedAt,membershipExpiresAt,createdAt,updatedAt FROM User WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_User(s.p);}
std::optional<User> Database::findByEmail(const std::string&email)const{Stmt s(db_,"SELECT id,email,name,passwordHash,membershipTier,membershipStatus,membershipStartedAt,membershipExpiresAt,createdAt,updatedAt FROM User WHERE email=?");text(s,1,email);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_User(s.p);}
std::vector<User> Database::listUsers()const{std::vector<User>r;Stmt s(db_,"SELECT id,email,name,passwordHash,membershipTier,membershipStatus,membershipStartedAt,membershipExpiresAt,createdAt,updatedAt FROM User ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_User(s.p));return r;}
bool Database::update(const User&v){Stmt s(db_,"UPDATE User SET email=?,name=?,passwordHash=?,membershipTier=?,membershipStatus=?,membershipStartedAt=?,membershipExpiresAt=?,createdAt=?,updatedAt=? WHERE id=?");text(s,1,v.email);opt(s,2,v.name);text(s,3,v.passwordHash);text(s,4,v.membershipTier);text(s,5,v.membershipStatus);opt(s,6,v.membershipStartedAt);opt(s,7,v.membershipExpiresAt);text(s,8,v.createdAt);text(s,9,v.updatedAt);text(s,10,v.id);return done(s);}
bool Database::deleteUser(const std::string&key){Stmt s(db_,"DELETE FROM User WHERE id=?");text(s,1,key);return done(s);}

// ---- UserSession ----
static void bind_UserSession(Stmt&s,const UserSession&v){text(s,1,v.id);text(s,2,v.userId);text(s,3,v.tokenHash);text(s,4,v.createdAt);text(s,5,v.expiresAt);}
static UserSession map_UserSession(sqlite3_stmt*s){UserSession v;v.id=str(s,0);v.userId=str(s,1);v.tokenHash=str(s,2);v.createdAt=str(s,3);v.expiresAt=str(s,4);return v;}
bool Database::insert(const UserSession&v){Stmt s(db_,"INSERT INTO UserSession(id,userId,tokenHash,createdAt,expiresAt) VALUES(?,?,?,?,?)");bind_UserSession(s,v);return done(s);}
std::optional<UserSession> Database::getUserSession(const std::string&key)const{Stmt s(db_,"SELECT id,userId,tokenHash,createdAt,expiresAt FROM UserSession WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_UserSession(s.p);}
std::vector<UserSession> Database::listUserSessions()const{std::vector<UserSession>r;Stmt s(db_,"SELECT id,userId,tokenHash,createdAt,expiresAt FROM UserSession ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_UserSession(s.p));return r;}
bool Database::update(const UserSession&v){Stmt s(db_,"UPDATE UserSession SET userId=?,tokenHash=?,createdAt=?,expiresAt=? WHERE id=?");text(s,1,v.userId);text(s,2,v.tokenHash);text(s,3,v.createdAt);text(s,4,v.expiresAt);text(s,5,v.id);return done(s);}
bool Database::deleteUserSession(const std::string&key){Stmt s(db_,"DELETE FROM UserSession WHERE id=?");text(s,1,key);return done(s);}
std::optional<UserSession> Database::findSessionByTokenHash(const std::string&hash)const{Stmt s(db_,"SELECT id,userId,tokenHash,createdAt,expiresAt FROM UserSession WHERE tokenHash=?");text(s,1,hash);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_UserSession(s.p);}

// ---- UsageCounter ----
static void bind_UsageCounter(Stmt&s,const UsageCounter&v){text(s,1,v.id);text(s,2,v.scopeId);text(s,3,v.scopeType);text(s,4,v.type);text(s,5,v.dateKey);integer(s,6,v.count);}
static UsageCounter map_UsageCounter(sqlite3_stmt*s){UsageCounter v;v.id=str(s,0);v.scopeId=str(s,1);v.scopeType=str(s,2);v.type=str(s,3);v.dateKey=str(s,4);v.count=sqlite3_column_int(s,5);return v;}
bool Database::insert(const UsageCounter&v){Stmt s(db_,"INSERT INTO UsageCounter(id,scopeId,scopeType,type,dateKey,count) VALUES(?,?,?,?,?,?)");bind_UsageCounter(s,v);return done(s);}
std::optional<UsageCounter> Database::getUsageCounter(const std::string&key)const{Stmt s(db_,"SELECT id,scopeId,scopeType,type,dateKey,count FROM UsageCounter WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_UsageCounter(s.p);}
std::vector<UsageCounter> Database::listUsageCounters()const{std::vector<UsageCounter>r;Stmt s(db_,"SELECT id,scopeId,scopeType,type,dateKey,count FROM UsageCounter ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_UsageCounter(s.p));return r;}
bool Database::update(const UsageCounter&v){Stmt s(db_,"UPDATE UsageCounter SET scopeId=?,scopeType=?,type=?,dateKey=?,count=? WHERE id=?");text(s,1,v.scopeId);text(s,2,v.scopeType);text(s,3,v.type);text(s,4,v.dateKey);integer(s,5,v.count);text(s,6,v.id);return done(s);}
bool Database::deleteUsageCounter(const std::string&key){Stmt s(db_,"DELETE FROM UsageCounter WHERE id=?");text(s,1,key);return done(s);}
std::optional<UsageCounter> Database::findUsageCounter(const std::string&scopeId,const std::string&scopeType,const std::string&type,const std::string&dateKey)const{Stmt s(db_,"SELECT id,scopeId,scopeType,type,dateKey,count FROM UsageCounter WHERE scopeId=? AND scopeType=? AND type=? AND dateKey=?");text(s,1,scopeId);text(s,2,scopeType);text(s,3,type);text(s,4,dateKey);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_UsageCounter(s.p);}

// ---- CourseProgress ----
static void bind_CourseProgress(Stmt&s,const CourseProgress&v){text(s,1,v.id);text(s,2,v.courseId);opt(s,3,v.anonymousId);opt(s,4,v.userId);text(s,5,v.goal);opt(s,6,v.mode);integer(s,7,v.overallPercent);integer(s,8,v.completedCount);integer(s,9,v.totalCount);opt(s,10,v.lastVisitedUrl);opt(s,11,v.lastPageType);optint(s,12,v.lastPhaseIndex);opt(s,13,v.lastPhaseName);optint(s,14,v.lastTopicIndex);opt(s,15,v.lastTopicTitle);text(s,16,v.updatedAt);text(s,17,v.createdAt);}
static CourseProgress map_CourseProgress(sqlite3_stmt*s){CourseProgress v;v.id=str(s,0);v.courseId=str(s,1);v.anonymousId=ostr(s,2);v.userId=ostr(s,3);v.goal=str(s,4);v.mode=ostr(s,5);v.overallPercent=sqlite3_column_int(s,6);v.completedCount=sqlite3_column_int(s,7);v.totalCount=sqlite3_column_int(s,8);v.lastVisitedUrl=ostr(s,9);v.lastPageType=ostr(s,10);v.lastPhaseIndex=oint(s,11);v.lastPhaseName=ostr(s,12);v.lastTopicIndex=oint(s,13);v.lastTopicTitle=ostr(s,14);v.updatedAt=str(s,15);v.createdAt=str(s,16);return v;}
bool Database::insert(const CourseProgress&v){Stmt s(db_,"INSERT INTO CourseProgress(id,courseId,anonymousId,userId,goal,mode,overallPercent,completedCount,totalCount,lastVisitedUrl,lastPageType,lastPhaseIndex,lastPhaseName,lastTopicIndex,lastTopicTitle,updatedAt,createdAt) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");bind_CourseProgress(s,v);return done(s);}
std::optional<CourseProgress> Database::getCourseProgress(const std::string&key)const{Stmt s(db_,"SELECT id,courseId,anonymousId,userId,goal,mode,overallPercent,completedCount,totalCount,lastVisitedUrl,lastPageType,lastPhaseIndex,lastPhaseName,lastTopicIndex,lastTopicTitle,updatedAt,createdAt FROM CourseProgress WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_CourseProgress(s.p);}
std::vector<CourseProgress> Database::listCourseProgress()const{std::vector<CourseProgress>r;Stmt s(db_,"SELECT id,courseId,anonymousId,userId,goal,mode,overallPercent,completedCount,totalCount,lastVisitedUrl,lastPageType,lastPhaseIndex,lastPhaseName,lastTopicIndex,lastTopicTitle,updatedAt,createdAt FROM CourseProgress ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_CourseProgress(s.p));return r;}
bool Database::update(const CourseProgress&v){Stmt s(db_,"UPDATE CourseProgress SET courseId=?,anonymousId=?,userId=?,goal=?,mode=?,overallPercent=?,completedCount=?,totalCount=?,lastVisitedUrl=?,lastPageType=?,lastPhaseIndex=?,lastPhaseName=?,lastTopicIndex=?,lastTopicTitle=?,updatedAt=?,createdAt=? WHERE id=?");text(s,1,v.courseId);opt(s,2,v.anonymousId);opt(s,3,v.userId);text(s,4,v.goal);opt(s,5,v.mode);integer(s,6,v.overallPercent);integer(s,7,v.completedCount);integer(s,8,v.totalCount);opt(s,9,v.lastVisitedUrl);opt(s,10,v.lastPageType);optint(s,11,v.lastPhaseIndex);opt(s,12,v.lastPhaseName);optint(s,13,v.lastTopicIndex);opt(s,14,v.lastTopicTitle);text(s,15,v.updatedAt);text(s,16,v.createdAt);text(s,17,v.id);return done(s);}
bool Database::deleteCourseProgress(const std::string&key){Stmt s(db_,"DELETE FROM CourseProgress WHERE id=?");text(s,1,key);return done(s);}
std::optional<CourseProgress> Database::findProgressByCourseId(const std::string&courseId)const{Stmt s(db_,"SELECT id,courseId,anonymousId,userId,goal,mode,overallPercent,completedCount,totalCount,lastVisitedUrl,lastPageType,lastPhaseIndex,lastPhaseName,lastTopicIndex,lastTopicTitle,updatedAt,createdAt FROM CourseProgress WHERE courseId=?");text(s,1,courseId);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_CourseProgress(s.p);}

// ---- CourseSnapshot ----
static void bind_CourseSnapshot(Stmt&s,const CourseSnapshot&v){text(s,1,v.id);text(s,2,v.courseId);integer(s,3,v.version);text(s,4,v.payload);text(s,5,v.createdAt);}
static CourseSnapshot map_CourseSnapshot(sqlite3_stmt*s){CourseSnapshot v;v.id=str(s,0);v.courseId=str(s,1);v.version=sqlite3_column_int(s,2);v.payload=str(s,3);v.createdAt=str(s,4);return v;}
bool Database::insert(const CourseSnapshot&v){ if(v.id.empty()) { CourseSnapshot copy=v; copy.id=id(); return insert(copy); } Stmt s(db_,"INSERT INTO CourseSnapshot(id,courseId,version,payload,createdAt) VALUES(?,?,?,?,?)");bind_CourseSnapshot(s,v);const bool saved=done(s);
    if (saved) {
        markLearningDirty();
        const auto previous=getClassroomActivity("course-preview:"+v.courseId);
        auto payload=previous?nlohmann::json::parse(previous->payload,nullptr,false):nlohmann::json::object();
        if (!payload.is_object()) payload=nlohmann::json::object();
        payload["status"]="pending";payload["assessedCourseVersion"]=v.version;payload["learningVersion"]=learningRevision();
        payload["message"]="真实 AI 正在生成课程路线预览…";
        if (!payload.contains("slides")) payload["slides"]=nlohmann::json::array();
        upsert(ClassroomActivity{"course-preview:"+v.courseId,v.courseId,"preview",payload.dump(),v.createdAt,0,0});
    }
    return saved;}
std::optional<CourseSnapshot> Database::getCourseSnapshot(const std::string&key)const{Stmt s(db_,"SELECT id,courseId,version,payload,createdAt FROM CourseSnapshot WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_CourseSnapshot(s.p);}
std::vector<CourseSnapshot> Database::listCourseSnapshots()const{std::vector<CourseSnapshot>r;Stmt s(db_,"SELECT id,courseId,version,payload,createdAt FROM CourseSnapshot ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_CourseSnapshot(s.p));return r;}
bool Database::update(const CourseSnapshot&v){Stmt s(db_,"UPDATE CourseSnapshot SET courseId=?,version=?,payload=?,createdAt=? WHERE id=?");text(s,1,v.courseId);integer(s,2,v.version);text(s,3,v.payload);text(s,4,v.createdAt);text(s,5,v.id);return done(s);}
bool Database::deleteCourseSnapshot(const std::string&key){Stmt s(db_,"DELETE FROM CourseSnapshot WHERE id=?");text(s,1,key);return done(s);}
std::vector<CourseSnapshot> Database::findSnapshotsByCourseId(const std::string&courseId)const{std::vector<CourseSnapshot>r;Stmt s(db_,"SELECT id,courseId,version,payload,createdAt FROM CourseSnapshot WHERE courseId=? ORDER BY version");text(s,1,courseId);while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_CourseSnapshot(s.p));return r;}

// ---- 通用进度表模式（TaskProgress / LearningStepProgress / LearningCardProgress）----
template <typename T>
struct ProgressTraits;
template <> struct ProgressTraits<TaskProgress> {
    static constexpr const char* table = "TaskProgress";
    static constexpr const char* index_col = "taskIndex";
    static constexpr const char* index_title = "taskTitle";
    static std::string key(const TaskProgress& v) { return v.courseId ? *v.courseId : v.id; }
};
template <> struct ProgressTraits<LearningStepProgress> {
    static constexpr const char* table = "LearningStepProgress";
    static constexpr const char* index_col = "stepIndex";
    static constexpr const char* index_title = "stepTitle";
    static std::string key(const LearningStepProgress& v) { return v.courseId ? *v.courseId : v.id; }
};
template <> struct ProgressTraits<LearningCardProgress> {
    static constexpr const char* table = "LearningCardProgress";
    static constexpr const char* index_col = "topicIndex";
    static constexpr const char* index_title = "topicTitle";
    static std::string key(const LearningCardProgress& v) { return v.courseId ? *v.courseId : v.id; }
};

#define DEFINE_PROGRESS_CRUD(TYPE, IDXCOL, IDXTITLE) \
static void bind_##TYPE(Stmt&s,const TYPE&v){text(s,1,v.id);opt(s,2,v.courseId);opt(s,3,v.anonymousId);text(s,4,v.goal);opt(s,5,v.mode);integer(s,6,v.phaseIndex);text(s,7,v.phaseName);integer(s,8,v.IDXCOL);text(s,9,v.IDXTITLE);text(s,10,v.status);} \
static TYPE map_##TYPE(sqlite3_stmt*s){TYPE v;v.id=str(s,0);v.courseId=ostr(s,1);v.anonymousId=ostr(s,2);v.goal=str(s,3);v.mode=ostr(s,4);v.phaseIndex=sqlite3_column_int(s,5);v.phaseName=str(s,6);v.IDXCOL=sqlite3_column_int(s,7);v.IDXTITLE=str(s,8);v.status=str(s,9);return v;} \
bool Database::insert(const TYPE&v){Stmt s(db_,"INSERT INTO " #TYPE "(id,courseId,anonymousId,goal,mode,phaseIndex,phaseName," #IDXCOL "," #IDXTITLE ",status) VALUES(?,?,?,?,?,?,?,?,?,?)");bind_##TYPE(s,v);return done(s);} \
std::optional<TYPE> Database::get##TYPE(const std::string&key)const{Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName," #IDXCOL "," #IDXTITLE ",status FROM " #TYPE " WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_##TYPE(s.p);} \
std::vector<TYPE> Database::list##TYPE()const{std::vector<TYPE>r;Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName," #IDXCOL "," #IDXTITLE ",status FROM " #TYPE " ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_##TYPE(s.p));return r;} \
bool Database::update(const TYPE&v){Stmt s(db_,"UPDATE " #TYPE " SET courseId=?,anonymousId=?,goal=?,mode=?,phaseIndex=?,phaseName=?," #IDXCOL "=?," #IDXTITLE "=?,status=? WHERE id=?");opt(s,1,v.courseId);opt(s,2,v.anonymousId);text(s,3,v.goal);opt(s,4,v.mode);integer(s,5,v.phaseIndex);text(s,6,v.phaseName);integer(s,7,v.IDXCOL);text(s,8,v.IDXTITLE);text(s,9,v.status);text(s,10,v.id);return done(s);} \
bool Database::delete##TYPE(const std::string&key){Stmt s(db_,"DELETE FROM " #TYPE " WHERE id=?");text(s,1,key);return done(s);} \
std::optional<TYPE> Database::find##TYPE(const std::string&courseId,int phaseIndex,int idx)const{Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName," #IDXCOL "," #IDXTITLE ",status FROM " #TYPE " WHERE courseId=? AND phaseIndex=? AND " #IDXCOL "=?");text(s,1,courseId);integer(s,2,phaseIndex);integer(s,3,idx);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_##TYPE(s.p);}

DEFINE_PROGRESS_CRUD(TaskProgress, taskIndex, taskTitle)
DEFINE_PROGRESS_CRUD(LearningStepProgress, stepIndex, stepTitle)
DEFINE_PROGRESS_CRUD(LearningCardProgress, topicIndex, topicTitle)

// ---- LearningSession ----
static void bind_LearningSession(Stmt&s,const LearningSession&v){text(s,1,v.id);opt(s,2,v.courseId);opt(s,3,v.anonymousId);text(s,4,v.goal);opt(s,5,v.mode);integer(s,6,v.phaseIndex);text(s,7,v.phaseName);integer(s,8,v.topicIndex);text(s,9,v.topicTitle);text(s,10,v.title);opt(s,11,v.summary);opt(s,12,v.searchQuery);text(s,13,v.content);opt(s,14,v.references);integer(s,15,v.fallbackUsed);text(s,16,v.source);}
static LearningSession map_LearningSession(sqlite3_stmt*s){LearningSession v;v.id=str(s,0);v.courseId=ostr(s,1);v.anonymousId=ostr(s,2);v.goal=str(s,3);v.mode=ostr(s,4);v.phaseIndex=sqlite3_column_int(s,5);v.phaseName=str(s,6);v.topicIndex=sqlite3_column_int(s,7);v.topicTitle=str(s,8);v.title=str(s,9);v.summary=ostr(s,10);v.searchQuery=ostr(s,11);v.content=str(s,12);v.references=ostr(s,13);v.fallbackUsed=sqlite3_column_int(s,14);v.source=str(s,15);return v;}
bool Database::insert(const LearningSession&v){Stmt s(db_,"INSERT INTO LearningSession(id,courseId,anonymousId,goal,mode,phaseIndex,phaseName,topicIndex,topicTitle,title,summary,searchQuery,content,`references`,fallbackUsed,source) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");bind_LearningSession(s,v);return done(s);}
std::optional<LearningSession> Database::getLearningSession(const std::string&key)const{Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName,topicIndex,topicTitle,title,summary,searchQuery,content,`references`,fallbackUsed,source FROM LearningSession WHERE id=?");text(s,1,key);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_LearningSession(s.p);}
std::vector<LearningSession> Database::listLearningSessions()const{std::vector<LearningSession>r;Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName,topicIndex,topicTitle,title,summary,searchQuery,content,`references`,fallbackUsed,source FROM LearningSession ORDER BY id");while(sqlite3_step(s.p)==SQLITE_ROW)r.push_back(map_LearningSession(s.p));return r;}
bool Database::update(const LearningSession&v){Stmt s(db_,"UPDATE LearningSession SET courseId=?,anonymousId=?,goal=?,mode=?,phaseIndex=?,phaseName=?,topicIndex=?,topicTitle=?,title=?,summary=?,searchQuery=?,content=?,`references`=?,fallbackUsed=?,source=? WHERE id=?");opt(s,1,v.courseId);opt(s,2,v.anonymousId);text(s,3,v.goal);opt(s,4,v.mode);integer(s,5,v.phaseIndex);text(s,6,v.phaseName);integer(s,7,v.topicIndex);text(s,8,v.topicTitle);text(s,9,v.title);opt(s,10,v.summary);opt(s,11,v.searchQuery);text(s,12,v.content);opt(s,13,v.references);integer(s,14,v.fallbackUsed);text(s,15,v.source);text(s,16,v.id);return done(s);}
bool Database::deleteLearningSession(const std::string&key){Stmt s(db_,"DELETE FROM LearningSession WHERE id=?");text(s,1,key);return done(s);}
std::optional<LearningSession> Database::findLearningSession(const std::string&courseId,int phaseIndex,int topicIndex)const{Stmt s(db_,"SELECT id,courseId,anonymousId,goal,mode,phaseIndex,phaseName,topicIndex,topicTitle,title,summary,searchQuery,content,`references`,fallbackUsed,source FROM LearningSession WHERE courseId=? AND phaseIndex=? AND topicIndex=?");text(s,1,courseId);integer(s,2,phaseIndex);integer(s,3,topicIndex);if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;return map_LearningSession(s.p);}

bool Database::insert(const LearningInteraction& value) {
    Stmt s(db_, "INSERT INTO LearningInteraction(id,courseId,conversationId,subject,kind,payload,createdAt) VALUES(?,?,?,?,?,?,?)");
    text(s,1,value.id.empty()?id():value.id);opt(s,2,value.courseId);opt(s,3,value.conversationId);
    opt(s,4,value.subject);text(s,5,value.kind);text(s,6,value.payload);text(s,7,value.createdAt);
    const bool saved=done(s);
    if(saved && (value.kind=="quiz" || value.kind=="practice" || value.kind=="review")) markProfileDirty();
    if(saved && (value.kind=="quiz" || value.kind=="practice" || value.kind=="review" ||
        value.kind=="chat-user" || value.kind=="lesson" || value.kind=="step")) markLearningDirty();
    return saved;
}
std::vector<LearningInteraction> Database::listInteractions() const {
    std::vector<LearningInteraction> rows;
    Stmt s(db_, "SELECT id,courseId,conversationId,subject,kind,payload,createdAt FROM LearningInteraction ORDER BY rowid");
    while(sqlite3_step(s.p)==SQLITE_ROW) rows.push_back({str(s.p,0),str(s.p,4),str(s.p,5),str(s.p,6),ostr(s.p,1),ostr(s.p,2),ostr(s.p,3)});
    return rows;
}
bool Database::upsert(const ClassroomActivity& value) {
    const bool assessment = value.kind == "diagnostic" || value.kind == "interaction";
    const auto previous = assessment ? getClassroomActivity(value.id) : std::optional<ClassroomActivity>{};
    Stmt s(db_, "INSERT INTO ClassroomActivity(id,courseId,phaseIndex,topicIndex,kind,payload,updatedAt) VALUES(?,?,?,?,?,?,?) ON CONFLICT(id) DO UPDATE SET payload=excluded.payload,updatedAt=excluded.updatedAt");
    text(s,1,value.id);text(s,2,value.courseId);integer(s,3,value.phaseIndex);integer(s,4,value.topicIndex);
    text(s,5,value.kind);text(s,6,value.payload);text(s,7,value.updatedAt);
    const bool saved = done(s);
    if (saved && assessment && (!previous || previous->payload != value.payload)) {
        const auto payload = nlohmann::json::parse(value.payload, nullptr, false);
        if (payload.is_object() && payload.value("status", "") == "answered" && payload.value("credible", false)) {
            const auto old = previous ? nlohmann::json::parse(previous->payload, nullptr, false) : nlohmann::json();
            // 提示展开、反馈展示等界面变化不算新答案，避免额外触发 AI 评估。
            bool changed = !old.is_object() || old.value("status", "") != "answered" || !old.value("credible", false);
            for (const char* key : {"question", "options", "rubric", "type", "answer", "correct", "unknown"})
                if (!old.is_object() || old.value(key, nlohmann::json()) != payload.value(key, nlohmann::json())) changed = true;
            if (changed) { markProfileDirty(); markLearningDirty(); }
        }
    }
    return saved;
}
std::optional<ClassroomActivity> Database::getClassroomActivity(const std::string& key) const {
    Stmt s(db_, "SELECT id,courseId,phaseIndex,topicIndex,kind,payload,updatedAt FROM ClassroomActivity WHERE id=?");text(s,1,key);
    if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;
    return ClassroomActivity{str(s.p,0),str(s.p,1),str(s.p,4),str(s.p,5),str(s.p,6),sqlite3_column_int(s.p,2),sqlite3_column_int(s.p,3)};
}
std::vector<std::string> Database::classroomPayloadSnapshot(const std::vector<std::string>& keys) const {
    if (keys.empty() || keys.size() > 16) throw std::invalid_argument("课堂快照参数无效");
    std::string sql = "SELECT ";
    for (size_t index = 0; index < keys.size(); ++index) {
        if (index) sql += ',';
        sql += "(SELECT payload FROM ClassroomActivity WHERE id=?)";
    }
    Stmt statement(db_, sql.c_str());
    for (size_t index = 0; index < keys.size(); ++index) text(statement, static_cast<int>(index + 1), keys[index]);
    const int result = sqlite3_step(statement.p); check(result, db_, "read classroom snapshot");
    if (result != SQLITE_ROW) throw std::runtime_error("课堂状态暂不可读");
    std::vector<std::string> values;
    for (size_t index = 0; index < keys.size(); ++index) values.push_back(str(statement.p, static_cast<int>(index)));
    return values;
}
std::vector<ClassroomActivity> Database::listClassroomActivities(const std::string& courseId) const {
    std::vector<ClassroomActivity> rows;
    Stmt s(db_, "SELECT id,courseId,phaseIndex,topicIndex,kind,payload,updatedAt FROM ClassroomActivity WHERE courseId=? ORDER BY rowid");text(s,1,courseId);
    while(sqlite3_step(s.p)==SQLITE_ROW)rows.push_back({str(s.p,0),str(s.p,1),str(s.p,4),str(s.p,5),str(s.p,6),sqlite3_column_int(s.p,2),sqlite3_column_int(s.p,3)});
    return rows;
}
bool Database::upsert(const WeeklyPlan& value) {
    Stmt s(db_, "INSERT INTO WeeklyPlan(courseId,payload,version,updatedAt) VALUES(?,?,?,?) ON CONFLICT(courseId) DO UPDATE SET payload=excluded.payload,version=excluded.version,updatedAt=excluded.updatedAt");
    text(s,1,value.courseId);text(s,2,value.payload);integer(s,3,value.version);text(s,4,value.updatedAt);return done(s);
}
std::optional<WeeklyPlan> Database::getWeeklyPlan(const std::string& courseId) const {
    Stmt s(db_, "SELECT courseId,payload,version,updatedAt FROM WeeklyPlan WHERE courseId=?");text(s,1,courseId);
    if(sqlite3_step(s.p)!=SQLITE_ROW)return std::nullopt;
    return WeeklyPlan{str(s.p,0),str(s.p,1),str(s.p,3),sqlite3_column_int(s.p,2)};
}
void Database::deleteClassroomData(const std::string& courseId) {
    Stmt activities(db_, "DELETE FROM ClassroomActivity WHERE courseId=?");text(activities,1,courseId);done(activities);
    Stmt weekly(db_, "DELETE FROM WeeklyPlan WHERE courseId=?");text(weekly,1,courseId);done(weekly);
}
bool Database::deleteConversation(const std::string& conversationId) {
    Stmt s(db_, "DELETE FROM LearningInteraction WHERE conversationId=?");text(s,1,conversationId);
    const bool changed=done(s);return changed;
}
bool Database::deleteInteractionsForCourse(const std::string& courseId) {
    Stmt s(db_, "DELETE FROM LearningInteraction WHERE courseId=?");text(s,1,courseId);
    const bool changed=done(s);if(changed)markProfileDirty();return changed;
}
bool Database::upsert(const SubjectMastery& value) {
    Stmt s(db_, "INSERT INTO SubjectMastery(subject,score,rationale,weakPoints,recommendation,evidenceCount,model,status,updatedAt) VALUES(?,?,?,?,?,?,?,?,?) ON CONFLICT(subject) DO UPDATE SET score=excluded.score,rationale=excluded.rationale,weakPoints=excluded.weakPoints,recommendation=excluded.recommendation,evidenceCount=excluded.evidenceCount,model=excluded.model,status=excluded.status,updatedAt=excluded.updatedAt");
    text(s,1,value.subject);optint(s,2,value.score);text(s,3,value.rationale);text(s,4,value.weakPoints);
    text(s,5,value.recommendation);integer(s,6,value.evidenceCount);text(s,7,value.model);
    text(s,8,value.status);text(s,9,value.updatedAt);return done(s);
}
bool Database::upsert(const TopicMastery& value) {
    Stmt s(db_, "INSERT INTO TopicMastery(courseId,phaseIndex,topic,score,evidenceCount,rationale,weakPoints,recommendation,evidenceIds,nextReviewAt,model,status,version,updatedAt) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?) ON CONFLICT(courseId,phaseIndex,topic) DO UPDATE SET score=excluded.score,evidenceCount=excluded.evidenceCount,rationale=excluded.rationale,weakPoints=excluded.weakPoints,recommendation=excluded.recommendation,evidenceIds=excluded.evidenceIds,nextReviewAt=excluded.nextReviewAt,model=excluded.model,status=excluded.status,version=excluded.version,updatedAt=excluded.updatedAt");
    text(s,1,value.courseId);integer(s,2,value.phaseIndex);text(s,3,value.topic);optint(s,4,value.score);
    integer(s,5,value.evidenceCount);text(s,6,value.rationale);text(s,7,value.weakPoints);
    text(s,8,value.recommendation);text(s,9,value.evidenceIds);text(s,10,value.nextReviewAt);
    text(s,11,value.model);text(s,12,value.status);integer(s,13,value.version);text(s,14,value.updatedAt);
    return done(s);
}
std::vector<TopicMastery> Database::listTopicMastery() const {
    std::vector<TopicMastery> rows;
    Stmt s(db_, "SELECT courseId,phaseIndex,topic,score,evidenceCount,rationale,weakPoints,recommendation,evidenceIds,nextReviewAt,model,status,version,updatedAt FROM TopicMastery ORDER BY courseId,phaseIndex,topic");
    while (sqlite3_step(s.p) == SQLITE_ROW) {
        TopicMastery value;
        value.courseId=str(s.p,0);value.phaseIndex=sqlite3_column_int(s.p,1);value.topic=str(s.p,2);
        value.score=oint(s.p,3);value.evidenceCount=sqlite3_column_int(s.p,4);value.rationale=str(s.p,5);
        value.weakPoints=str(s.p,6);value.recommendation=str(s.p,7);value.evidenceIds=str(s.p,8);
        value.nextReviewAt=str(s.p,9);value.model=str(s.p,10);value.status=str(s.p,11);
        value.version=sqlite3_column_int(s.p,12);value.updatedAt=str(s.p,13);rows.push_back(std::move(value));
    }
    return rows;
}
bool Database::deleteTopicMasteryForCourse(const std::string& courseId) {
    Stmt s(db_, "DELETE FROM TopicMastery WHERE courseId=?");
    text(s,1,courseId);
    return done(s);
}
std::vector<SubjectMastery> Database::listMastery() const {
    std::vector<SubjectMastery> rows;
    Stmt s(db_, "SELECT subject,score,rationale,weakPoints,recommendation,evidenceCount,model,status,updatedAt FROM SubjectMastery ORDER BY subject");
    while(sqlite3_step(s.p)==SQLITE_ROW) rows.push_back({str(s.p,0),str(s.p,2),str(s.p,3),str(s.p,4),str(s.p,6),str(s.p,7),str(s.p,8),oint(s.p,1),sqlite3_column_int(s.p,5)});
    return rows;
}
void Database::replaceMastery(const std::vector<SubjectMastery>& values) {
    exec(db_, "BEGIN IMMEDIATE");
    try {
        exec(db_, "DELETE FROM SubjectMastery");
        for(const auto& value:values) upsert(value);
        exec(db_, "COMMIT");
    } catch (...) { exec(db_, "ROLLBACK"); throw; }
}
bool Database::profileDirty() const {
    Stmt s(db_, "SELECT (SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='revision') > (SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='assessed')");
    return sqlite3_step(s.p)==SQLITE_ROW && sqlite3_column_int(s.p,0)!=0;
}
int Database::profileRevision() const {
    Stmt s(db_, "SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='revision'");
    return sqlite3_step(s.p)==SQLITE_ROW ? sqlite3_column_int(s.p,0) : 0;
}
int Database::profileAssessedRevision() const {
    Stmt s(db_, "SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='assessed'");
    return sqlite3_step(s.p)==SQLITE_ROW ? sqlite3_column_int(s.p,0) : 0;
}
void Database::markProfileDirty() {
    exec(db_, "UPDATE ProfileMeta SET value=CAST(value AS INTEGER)+1 WHERE key='revision'");
    setProfileError("");
}
void Database::setProfileAssessed(int revision) {
    Stmt s(db_, "UPDATE ProfileMeta SET value=? WHERE key='assessed'");
    text(s,1,std::to_string(revision));done(s);
}
std::string Database::profileError() const {
    Stmt s(db_, "SELECT value FROM ProfileMeta WHERE key='error'");
    return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "";
}
bool Database::replaceMasteryAtRevision(const std::vector<SubjectMastery>& values, int revision) {
    exec(db_, "BEGIN IMMEDIATE");
    try {
        if (profileRevision() != revision) { exec(db_, "ROLLBACK"); return false; }
        exec(db_, "DELETE FROM SubjectMastery");
        for (const auto& value : values) upsert(value);
        setProfileAssessed(revision);
        setProfileError("");
        exec(db_, "COMMIT");
        return true;
    } catch (...) { exec(db_, "ROLLBACK"); throw; }
}
bool Database::upsertTopicMasteryAtRevision(const TopicMastery& value, int revision) {
    exec(db_, "BEGIN IMMEDIATE");
    try {
        if (profileRevision() != revision) { exec(db_, "ROLLBACK"); return false; }
        upsert(value);
        exec(db_, "COMMIT");
        return true;
    } catch (...) { exec(db_, "ROLLBACK"); throw; }
}
std::string Database::profileMeta(const std::string& key) const {
    Stmt s(db_, "SELECT value FROM ProfileMeta WHERE key=?");
    text(s,1,key);
    return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "";
}
void Database::setProfileMeta(const std::string& key, const std::string& value) {
    Stmt s(db_, "INSERT INTO ProfileMeta(key,value) VALUES(?,?) ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    text(s,1,key);text(s,2,value);done(s);
}
bool Database::setProfileMetaAtRevision(const std::string& key, const std::string& value, int revision) {
    // 单条 SQL 校验版本并保存，避免新作答写入后被旧 AI 结果覆盖。
    Stmt s(db_, "INSERT INTO ProfileMeta(key,value) SELECT ?,? WHERE (SELECT CAST(value AS INTEGER) FROM ProfileMeta WHERE key='revision')=? ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    text(s,1,key);text(s,2,value);integer(s,3,revision);done(s);
    return sqlite3_changes(db_) == 1;
}
void Database::setProfileError(const std::string& error) {
    Stmt s(db_, "INSERT INTO ProfileMeta(key,value) VALUES('error',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    text(s,1,error);done(s);
}
std::string Database::profileSearchStatus() const {
    Stmt s(db_, "SELECT value FROM ProfileMeta WHERE key='searchStatus'");
    return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "not_requested";
}
std::string Database::profileSources() const {
    Stmt s(db_, "SELECT value FROM ProfileMeta WHERE key='sources'");
    return sqlite3_step(s.p)==SQLITE_ROW ? str(s.p,0) : "[]";
}
void Database::setProfileSearch(const std::string& status, const std::string& sources) {
    Stmt state(db_, "INSERT INTO ProfileMeta(key,value) VALUES('searchStatus',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    text(state,1,status);done(state);
    Stmt links(db_, "INSERT INTO ProfileMeta(key,value) VALUES('sources',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    text(links,1,sources);done(links);
}
}

#include "db.hpp"
#include "db_schema_version.hpp"

#include <sqlite3.h>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>

namespace gangyi {
namespace {
void check(int rc, sqlite3* db, const char* action) { if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw std::runtime_error(std::string(action) + ": " + sqlite3_errmsg(db)); }
void exec(sqlite3* db, const char* sql) { char* error = nullptr; const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &error); if (rc != SQLITE_OK) { std::string message = error ? error : sqlite3_errmsg(db); sqlite3_free(error); throw std::runtime_error(message); } }
std::string id() { static std::mt19937_64 rng(std::random_device{}()); std::ostringstream out; out << std::hex << std::setfill('0'); for (int i = 0; i < 2; ++i) out << std::setw(16) << rng(); return out.str(); }
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
void Database::open(const std::string& path) { close(); check(sqlite3_open(path.c_str(), &db_), db_, "open database"); sqlite3_extended_result_codes(db_, 1); }
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
    exec(db_, ("PRAGMA user_version=" + std::to_string(kDatabaseSchemaVersion)).c_str());
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
bool Database::insert(const CourseSnapshot&v){ if(v.id.empty()) { CourseSnapshot copy=v; copy.id=id(); return insert(copy); } Stmt s(db_,"INSERT INTO CourseSnapshot(id,courseId,version,payload,createdAt) VALUES(?,?,?,?,?)");bind_CourseSnapshot(s,v);return done(s);}
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
}

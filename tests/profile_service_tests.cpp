#include "db.hpp"
#include "profile_service.hpp"

#include <sqlite3.h>
#include <filesystem>
#include <iostream>

int main() {
    const auto path = std::filesystem::temp_directory_path() / "gangyiAI-profile-tests.db";
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
    std::filesystem::remove(path.u8string() + ".pre-v4.db", ignored);
    sqlite3* legacy = nullptr;
    if (sqlite3_open(path.u8string().c_str(), &legacy) != SQLITE_OK) return 1;
    const char* sql = "CREATE TABLE Course(id TEXT PRIMARY KEY,anonymousId TEXT,userId TEXT,goal TEXT NOT NULL,mode TEXT NOT NULL,title TEXT NOT NULL,summary TEXT,source TEXT NOT NULL,status TEXT NOT NULL,createdAt TEXT NOT NULL,updatedAt TEXT NOT NULL);"
        "INSERT INTO Course VALUES('old','anonymous-old',NULL,'学习二次函数','deep','数学课程',NULL,'ai','active','2026-01-01','2026-01-01');PRAGMA user_version=3;";
    if (sqlite3_exec(legacy, sql, nullptr, nullptr, nullptr) != SQLITE_OK) return 2;
    sqlite3_close(legacy);

    gangyi::Database db;
    db.open(path.u8string());
    db.migrate();
    if (!db.getCourse("old") || !std::filesystem::exists(path.u8string() + ".pre-v4.db") ||
        !db.profileDirty() || !db.listTopicMastery().empty()) return 3;
    const auto evidence = gangyi::profileEvidenceSummary(db);
    if (evidence["courseCount"] != 1 || !db.listMastery().empty()) return 4;

    gangyi::LearningInteraction question;
    question.kind = "chat-user";
    question.payload = R"({"text":"二次函数顶点怎么求？","topic":"二次函数"})";
    question.createdAt = "2026-01-02T00:00:00Z";
    question.conversationId = "general";
    if (!db.insert(question) || db.listInteractions().size() != 1) return 5;
    gangyi::SubjectMastery mastery;
    mastery.subject = "数学";
    mastery.score = 66;
    mastery.rationale = "测试依据";
    mastery.evidenceCount = 1;
    db.replaceMastery({mastery});
    if (db.listMastery().at(0).score != 66) return 6;
    const int revision = db.profileRevision();
    db.setProfileAssessed(revision);
    if (db.profileDirty() || !db.deleteConversation("general") || !db.listInteractions().empty() || db.profileDirty()) return 7;
    gangyi::LearningInteraction quiz;
    quiz.courseId = "old";
    quiz.kind = "quiz";
    quiz.payload = R"({"topic":"二次函数","phaseIndex":1,"results":[{"questionIndex":0,"correct":false,"answered":true}]})";
    quiz.createdAt = "2026-01-03T00:00:00Z";
    if (!db.insert(quiz) || !db.profileDirty() || db.profileAssessedRevision() != revision ||
        db.profileRevision() <= db.profileAssessedRevision()) return 9;
    if (gangyi::profileView(db)["subjects"].size() != 1) return 8; // 清除后保留旧值，等待后台重评。
    db.close();
    std::filesystem::remove(path, ignored);
    std::filesystem::remove(path.u8string() + ".pre-v4.db", ignored);
    const auto v4Path = std::filesystem::temp_directory_path() / "gangyiAI-profile-v4-tests.db";
    std::filesystem::remove(v4Path, ignored);
    std::filesystem::remove(v4Path.u8string() + ".pre-v5.db", ignored);
    std::filesystem::remove(v4Path.u8string() + ".pre-v6.db", ignored);
    if (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) return 10;
    if (sqlite3_exec(legacy,
        "CREATE TABLE LearningInteraction(id TEXT PRIMARY KEY,courseId TEXT,conversationId TEXT,subject TEXT,kind TEXT NOT NULL,payload TEXT NOT NULL,createdAt TEXT NOT NULL);"
        "CREATE TABLE SubjectMastery(subject TEXT PRIMARY KEY,score INTEGER,rationale TEXT NOT NULL,weakPoints TEXT NOT NULL,recommendation TEXT NOT NULL,evidenceCount INTEGER NOT NULL,model TEXT NOT NULL,status TEXT NOT NULL,updatedAt TEXT NOT NULL);"
        "INSERT INTO SubjectMastery VALUES('数学',66,'旧画像','[]','复习',1,'old','ready','2026-01-01');"
        "CREATE TABLE ProfileMeta(key TEXT PRIMARY KEY,value TEXT NOT NULL);"
        "INSERT INTO ProfileMeta VALUES('revision','1'),('assessed','1'),('error','');"
        "PRAGMA user_version=4;", nullptr, nullptr, nullptr) != SQLITE_OK) return 11;
    sqlite3_close(legacy);
    db.open(v4Path.u8string());
    db.migrate();
    if (!std::filesystem::exists(v4Path.u8string() + ".pre-v5.db") ||
        db.listMastery().size() != 1 || db.listMastery()[0].score != 66 ||
        db.listMastery()[0].status != "legacy" ||
        gangyi::profileView(db)["subjects"][0]["score"] != nullptr ||
        gangyi::profileView(db)["subjects"][0]["historicalScore"] != 66) return 12;
    db.close();
    // 重现已标记为 v5、但缺少课堂表的真实升级路径。
    if (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) return 13;
    if (sqlite3_exec(legacy, "DROP TABLE ClassroomActivity; DROP TABLE WeeklyPlan; PRAGMA user_version=5;",
        nullptr, nullptr, nullptr) != SQLITE_OK) return 14;
    sqlite3_close(legacy);
    db.open(v4Path.u8string());
    db.migrate();
    if (!std::filesystem::exists(v4Path.u8string() + ".pre-v6.db") ||
        db.listMastery().size() != 1 || db.listMastery()[0].score != 66) return 15;
    gangyi::WeeklyPlan week;
    week.courseId = "old"; week.payload = "{\"entries\":[]}"; week.version = 7; week.updatedAt = "2026-09-30";
    gangyi::ClassroomActivity activity;
    activity.id = "old-activity"; activity.courseId = "old"; activity.kind = "diagnostic";
    activity.payload = "{\"status\":\"answered\"}"; activity.updatedAt = "2026-09-30";
    if (!db.upsert(week) || !db.upsert(activity)) return 16;
    db.close();
    if (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) return 20;
    sqlite3_exec(legacy, "PRAGMA user_version=5", nullptr, nullptr, nullptr);
    sqlite3_close(legacy);
    db.open(v4Path.u8string());
    db.migrate();
    db.migrate();
    if (!db.getWeeklyPlan("old") || db.getWeeklyPlan("old")->version != 7 ||
        !db.getClassroomActivity("old-activity")) return 17;
    db.close();
    if (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) return 18;
    sqlite3_stmt* statement = nullptr;
    sqlite3_prepare_v2(legacy, "PRAGMA user_version", -1, &statement, nullptr);
    if (sqlite3_step(statement) != SQLITE_ROW || sqlite3_column_int(statement, 0) != 6) return 19;
    sqlite3_finalize(statement);
    sqlite3_close(legacy);
    std::filesystem::remove(v4Path, ignored);
    std::filesystem::remove(v4Path.u8string() + ".pre-v5.db", ignored);
    std::filesystem::remove(v4Path.u8string() + ".pre-v6.db", ignored);
    std::cout << "画像迁移与本机记录测试通过\n";
}

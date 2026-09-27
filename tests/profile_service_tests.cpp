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
    if (!db.getCourse("old") || !std::filesystem::exists(path.u8string() + ".pre-v4.db") || !db.profileDirty()) return 3;
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
    if (db.profileDirty() || !db.deleteConversation("general") || !db.listInteractions().empty() || !db.profileDirty()) return 7;
    if (gangyi::profileView(db)["subjects"].size() != 1) return 8; // 清除后保留旧值，等待后台重评。
    db.close();
    std::filesystem::remove(path, ignored);
    std::filesystem::remove(path.u8string() + ".pre-v4.db", ignored);
    std::cout << "画像迁移与本机记录测试通过\n";
}

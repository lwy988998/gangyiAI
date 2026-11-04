// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"
// 【逐行批注】 引入头文件 "profile_service.hpp"，为后续实现提供类型或接口声明。
#include "profile_service.hpp"
// 【逐行批注】 引入头文件 "db_schema_version.hpp"，为后续实现提供类型或接口声明。
#include "db_schema_version.hpp"
// 【逐行批注】 引入头文件 "question_evidence.hpp"，为后续实现提供类型或接口声明。
#include "question_evidence.hpp"

// 【逐行批注】 引入头文件 <sqlite3.h>，为后续实现提供类型或接口声明。
#include <sqlite3.h>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
    // 【逐行批注】 计算右侧结果并写入 auto path，更新当前运行状态。
    const auto path = std::filesystem::temp_directory_path() / "gangyiAI-profile-tests.db";
    // 【逐行批注】 执行当前声明或表达式：std::error_code ignored;。
    std::error_code ignored;
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(path, ignored);。
    std::filesystem::remove(path, ignored);
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(path.u8string() + ".pre-v4.db", ignored);。
    std::filesystem::remove(path.u8string() + ".pre-v4.db", ignored);
    // 【逐行批注】 计算右侧结果并写入 sqlite3* legacy，更新当前运行状态。
    sqlite3* legacy = nullptr;
    // 【逐行批注】 判断条件 (sqlite3_open(path.u8string().c_str(), &legacy) != SQLITE_OK) re…，成立时执行对应分支。
    if (sqlite3_open(path.u8string().c_str(), &legacy) != SQLITE_OK) return 1;
    // 【逐行批注】 计算右侧结果并写入 char* sql，更新当前运行状态。
    const char* sql = "CREATE TABLE Course(id TEXT PRIMARY KEY,anonymousId TEXT,userId TEXT,goal TEXT NOT NULL,mode TEXT NOT NULL,title TEXT NOT NULL,summary TEXT,source TEXT NOT NULL,status TEXT NOT NULL,createdAt TEXT NOT NULL,updatedAt TEXT NOT NULL);"
        // 【逐行批注】 计算右侧结果并写入 "INSERT INTO Course VALUES('old','anonymous-…，更新当前运行状态。
        "INSERT INTO Course VALUES('old','anonymous-old',NULL,'学习二次函数','deep','数学课程',NULL,'ai','active','2026-01-01','2026-01-01');PRAGMA user_version=3;";
    // 【逐行批注】 判断条件 (sqlite3_exec(legacy, sql, nullptr, nullptr, nullptr) != SQLITE_…，成立时执行对应分支。
    if (sqlite3_exec(legacy, sql, nullptr, nullptr, nullptr) != SQLITE_OK) return 2;
    // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(legacy);。
    sqlite3_close(legacy);

    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db;。
    gangyi::Database db;
    // 【逐行批注】 执行当前声明或表达式：db.open(path.u8string());。
    db.open(path.u8string());
    // 【逐行批注】 执行当前声明或表达式：db.migrate();。
    db.migrate();
    // 【逐行批注】 判断条件 (!db.getCourse("old") || !std::filesystem::exists(path.u8string(…，成立时执行对应分支。
    if (!db.getCourse("old") || !std::filesystem::exists(path.u8string() + ".pre-v4.db") ||
        // 【逐行批注】 执行当前声明或表达式：!db.profileDirty() || !db.listTopicMastery().empty()) return 3;。
        !db.profileDirty() || !db.listTopicMastery().empty()) return 3;
    // 【逐行批注】 计算右侧结果并写入 auto evidence，更新当前运行状态。
    const auto evidence = gangyi::profileEvidenceSummary(db);
    // 【逐行批注】 判断条件 (evidence["courseCount"] != 1 || !db.listMastery().empty()) retu…，成立时执行对应分支。
    if (evidence["courseCount"] != 1 || !db.listMastery().empty()) return 4;

    // 【逐行批注】 执行当前声明或表达式：gangyi::LearningInteraction question;。
    gangyi::LearningInteraction question;
    // 【逐行批注】 计算右侧结果并写入 question.kind，更新当前运行状态。
    question.kind = "chat-user";
    // 【逐行批注】 计算右侧结果并写入 question.payload，更新当前运行状态。
    question.payload = R"({"text":"二次函数顶点怎么求？","topic":"二次函数"})";
    // 【逐行批注】 计算右侧结果并写入 question.createdAt，更新当前运行状态。
    question.createdAt = "2026-01-02T00:00:00Z";
    // 【逐行批注】 计算右侧结果并写入 question.conversationId，更新当前运行状态。
    question.conversationId = "general";
    // 【逐行批注】 判断条件 (!db.insert(question) || db.listInteractions().size() != 1) retu…，成立时执行对应分支。
    if (!db.insert(question) || db.listInteractions().size() != 1) return 5;
    // 【逐行批注】 执行当前声明或表达式：gangyi::SubjectMastery mastery;。
    gangyi::SubjectMastery mastery;
    // 【逐行批注】 计算右侧结果并写入 mastery.subject，更新当前运行状态。
    mastery.subject = "数学";
    // 【逐行批注】 计算右侧结果并写入 mastery.score，更新当前运行状态。
    mastery.score = 66;
    // 【逐行批注】 计算右侧结果并写入 mastery.rationale，更新当前运行状态。
    mastery.rationale = "测试依据";
    // 【逐行批注】 计算右侧结果并写入 mastery.evidenceCount，更新当前运行状态。
    mastery.evidenceCount = 1;
    // 【逐行批注】 执行当前声明或表达式：db.replaceMastery({mastery});。
    db.replaceMastery({mastery});
    // 【逐行批注】 判断条件 (db.listMastery().at(0).score != 66) return 6;，成立时执行对应分支。
    if (db.listMastery().at(0).score != 66) return 6;
    // 【逐行批注】 计算右侧结果并写入 int revision，更新当前运行状态。
    const int revision = db.profileRevision();
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json originalQuestion，更新当前运行状态。
    const nlohmann::json originalQuestion = {{"question", "虚构题目"}, {"options", {"甲", "乙"}}, {"answerIndex", 0}};
    // 【逐行批注】 计算右侧结果并写入 repeatedQuestion，更新当前运行状态。
    auto repeatedQuestion = originalQuestion;
    // 【逐行批注】 计算右侧结果并写入 repeatedQuestion["status"]，更新当前运行状态。
    repeatedQuestion["status"] = "answered";
    // 【逐行批注】 计算右侧结果并写入 repeatedQuestion["answer"]，更新当前运行状态。
    repeatedQuestion["answer"] = 1;
    // 【逐行批注】 计算右侧结果并写入 repeatedQuestion["answerIndex"]，更新当前运行状态。
    repeatedQuestion["answerIndex"] = 1;
    // 【逐行批注】 计算右侧结果并写入 repeatedQuestion["difficulty"]，更新当前运行状态。
    repeatedQuestion["difficulty"] = "基础";
    // 【逐行批注】 判断条件 (gangyi::questionIdentity("old", gangyi::questionSnapshot(origin…，成立时执行对应分支。
    if (gangyi::questionIdentity("old", gangyi::questionSnapshot(originalQuestion)) !=
        // 【逐行批注】 调用 questionIdentity、questionSnapshot 完成本行所需操作：gangyi::questionIdentity("old", gangyi::questionSnapshot(repeatedQues…。
        gangyi::questionIdentity("old", gangyi::questionSnapshot(repeatedQuestion))) return 30;
    // 【逐行批注】 判断条件 (!db.setProfileMetaAtRevision("test-version", "有效", revision) ||，成立时执行对应分支。
    if (!db.setProfileMetaAtRevision("test-version", "有效", revision) ||
        // 【逐行批注】 执行当前声明或表达式：db.setProfileMetaAtRevision("test-version", "过期", revision - 1) ||。
        db.setProfileMetaAtRevision("test-version", "过期", revision - 1) ||
        // 【逐行批注】 执行当前声明或表达式：db.profileMeta("test-version") != "有效") return 31;。
        db.profileMeta("test-version") != "有效") return 31;
    // 【逐行批注】 判断条件 (db.replaceMasteryAtRevision({}, revision - 1) || db.listMastery…，成立时执行对应分支。
    if (db.replaceMasteryAtRevision({}, revision - 1) || db.listMastery().at(0).score != 66) return 32;
    // 【逐行批注】 执行当前声明或表达式：db.setProfileAssessed(revision);。
    db.setProfileAssessed(revision);
    // 【逐行批注】 判断条件 (db.profileDirty() || !db.deleteConversation("general") || !db.l…，成立时执行对应分支。
    if (db.profileDirty() || !db.deleteConversation("general") || !db.listInteractions().empty() || db.profileDirty()) return 7;
    // 【逐行批注】 执行当前声明或表达式：gangyi::LearningInteraction quiz;。
    gangyi::LearningInteraction quiz;
    // 【逐行批注】 计算右侧结果并写入 quiz.courseId，更新当前运行状态。
    quiz.courseId = "old";
    // 【逐行批注】 计算右侧结果并写入 quiz.kind，更新当前运行状态。
    quiz.kind = "quiz";
    // 【逐行批注】 计算右侧结果并写入 quiz.payload，更新当前运行状态。
    quiz.payload = R"({"topic":"二次函数","phaseIndex":1,"results":[{"questionIndex":0,"correct":false,"answered":true}]})";
    // 【逐行批注】 计算右侧结果并写入 quiz.createdAt，更新当前运行状态。
    quiz.createdAt = "2026-01-03T00:00:00Z";
    // 【逐行批注】 判断条件 (!db.insert(quiz) || !db.profileDirty() || db.profileAssessedRev…，成立时执行对应分支。
    if (!db.insert(quiz) || !db.profileDirty() || db.profileAssessedRevision() != revision ||
        // 【逐行批注】 执行当前声明或表达式：db.profileRevision() <= db.profileAssessedRevision()) return 9;。
        db.profileRevision() <= db.profileAssessedRevision()) return 9;
    // 【逐行批注】 判断条件 (gangyi::profileView(db)["subjects"].size() != 1) return 8; // 清…，成立时执行对应分支。
    if (gangyi::profileView(db)["subjects"].size() != 1) return 8; // 清除后保留旧值，等待后台重评。
    // 【逐行批注】 执行当前声明或表达式：db.setProfileAssessed(db.profileRevision());。
    db.setProfileAssessed(db.profileRevision());
    // 【逐行批注】 按循环规则重复处理：for (int i = 0; i < 4; ++i) {。
    for (int i = 0; i < 4; ++i) {
        // 【逐行批注】 执行当前声明或表达式：gangyi::Course course;。
        gangyi::Course course;
        // 【逐行批注】 计算右侧结果并写入 course.id，更新当前运行状态。
        course.id = "recent-" + std::to_string(i);
        // 【逐行批注】 计算右侧结果并写入 course.goal，更新当前运行状态。
        course.goal = course.title = "课程" + std::to_string(i);
        // 【逐行批注】 计算右侧结果并写入 course.mode，更新当前运行状态。
        course.mode = "deep";
        // 【逐行批注】 计算右侧结果并写入 course.createdAt，更新当前运行状态。
        course.createdAt = "2026-01-0" + std::to_string(i + 4) + "T00:00:00Z";
        // 【逐行批注】 计算右侧结果并写入 course.updatedAt，更新当前运行状态。
        course.updatedAt = "2099-01-01T00:00:00Z"; // 刷新进度不应改变最近学习排序。
        // 【逐行批注】 判断条件 (i == 3) course.status = "deleted";，成立时执行对应分支。
        if (i == 3) course.status = "deleted";
        // 【逐行批注】 判断条件 (!db.insert(course)) return 21;，成立时执行对应分支。
        if (!db.insert(course)) return 21;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 recent，更新当前运行状态。
    auto recent = gangyi::recentCourses(db);
    // 【逐行批注】 判断条件 (recent.size() != 3 || recent[0]["courseId"] != "recent-2" ||，成立时执行对应分支。
    if (recent.size() != 3 || recent[0]["courseId"] != "recent-2" ||
        // 【逐行批注】 执行当前声明或表达式：recent[2]["courseId"] != "recent-0") return 22;。
        recent[2]["courseId"] != "recent-0") return 22;
    // 【逐行批注】 执行当前声明或表达式：gangyi::CourseSnapshot snapshot;。
    gangyi::CourseSnapshot snapshot;
    // 【逐行批注】 计算右侧结果并写入 snapshot.courseId，更新当前运行状态。
    snapshot.courseId = "old";
    // 【逐行批注】 计算右侧结果并写入 snapshot.payload，更新当前运行状态。
    snapshot.payload = R"({"courseStructure":[{"topics":["二次函数","顶点公式"]}]})";
    // 【逐行批注】 计算右侧结果并写入 snapshot.createdAt，更新当前运行状态。
    snapshot.createdAt = "2026-01-01T00:00:00Z";
    // 【逐行批注】 判断条件 (!db.insert(snapshot)) return 23;，成立时执行对应分支。
    if (!db.insert(snapshot)) return 23;
    // 【逐行批注】 执行当前声明或表达式：gangyi::LearningCardProgress completed;。
    gangyi::LearningCardProgress completed;
    // 【逐行批注】 计算右侧结果并写入 completed.courseId，更新当前运行状态。
    completed.courseId = "old"; completed.goal = "学习二次函数";
    // 【逐行批注】 计算右侧结果并写入 completed.phaseIndex，更新当前运行状态。
    completed.phaseIndex = 1; completed.topicIndex = 1; completed.topicTitle = "二次函数";
    // 【逐行批注】 计算右侧结果并写入 completed.status，更新当前运行状态。
    completed.status = "completed";
    // 【逐行批注】 判断条件 (!db.insert(completed)) return 24;，成立时执行对应分支。
    if (!db.insert(completed)) return 24;
    // 【逐行批注】 执行当前声明或表达式：gangyi::LearningInteraction visit;。
    gangyi::LearningInteraction visit;
    // 【逐行批注】 计算右侧结果并写入 visit.courseId，更新当前运行状态。
    visit.courseId = "old"; visit.kind = "lesson-visit";
    // 【逐行批注】 计算右侧结果并写入 visit.createdAt，更新当前运行状态。
    visit.createdAt = "2026-02-01T00:00:00Z";
    // 【逐行批注】 计算右侧结果并写入 visit.payload，更新当前运行状态。
    visit.payload = R"({"phaseIndex":1,"topicIndex":2,"topic":"顶点公式"})";
    // 【逐行批注】 判断条件 (!db.insert(visit)) return 25;，成立时执行对应分支。
    if (!db.insert(visit)) return 25;
    // 【逐行批注】 计算右侧结果并写入 visit.id.clear(); visit.kind，更新当前运行状态。
    visit.id.clear(); visit.kind = "course-visit"; visit.payload = "{}";
    // 【逐行批注】 计算右侧结果并写入 visit.createdAt，更新当前运行状态。
    visit.createdAt = "2026-02-02T00:00:00Z";
    // 【逐行批注】 判断条件 (!db.insert(visit) || db.profileDirty()) return 26;，成立时执行对应分支。
    if (!db.insert(visit) || db.profileDirty()) return 26;
    // 【逐行批注】 计算右侧结果并写入 recent，更新当前运行状态。
    recent = gangyi::recentCourses(db);
    // 【逐行批注】 判断条件 (recent.size() != 3 || recent[0]["courseId"] != "old" ||，成立时执行对应分支。
    if (recent.size() != 3 || recent[0]["courseId"] != "old" ||
        // 【逐行批注】 执行当前声明或表达式：recent[0]["latestTopic"] != "顶点公式" || recent[0]["totalTopics"] != 2 ||。
        recent[0]["latestTopic"] != "顶点公式" || recent[0]["totalTopics"] != 2 ||
        // 【逐行批注】 执行当前声明或表达式：recent[0]["doneTopics"] != 1 || recent[0]["percent"] != 50 ||。
        recent[0]["doneTopics"] != 1 || recent[0]["percent"] != 50 ||
        // 【逐行批注】 执行当前声明或表达式：recent[0]["lastActivityAt"] != visit.createdAt || recent[1]["courseId"] != "recent-2…。
        recent[0]["lastActivityAt"] != visit.createdAt || recent[1]["courseId"] != "recent-2") return 27;
    // 【逐行批注】 执行当前声明或表达式：gangyi::ClassroomActivity recentActivity;。
    gangyi::ClassroomActivity recentActivity;
    // 【逐行批注】 计算右侧结果并写入 recentActivity.id，更新当前运行状态。
    recentActivity.id = "recent-practice"; recentActivity.courseId = "recent-0";
    // 【逐行批注】 计算右侧结果并写入 recentActivity.kind，更新当前运行状态。
    recentActivity.kind = "practice"; recentActivity.payload = "{}";
    // 【逐行批注】 计算右侧结果并写入 recentActivity.phaseIndex，更新当前运行状态。
    recentActivity.phaseIndex = recentActivity.topicIndex = 1;
    // 【逐行批注】 计算右侧结果并写入 recentActivity.updatedAt，更新当前运行状态。
    recentActivity.updatedAt = "2026-02-03T00:00:00Z";
    // 【逐行批注】 判断条件 (!db.upsert(recentActivity) || gangyi::recentCourses(db)[0]["cou…，成立时执行对应分支。
    if (!db.upsert(recentActivity) || gangyi::recentCourses(db)[0]["courseId"] != "recent-0") return 28;
    // 【逐行批注】 执行当前声明或表达式：gangyi::ClassroomActivity answered;。
    gangyi::ClassroomActivity answered;
    // 【逐行批注】 计算右侧结果并写入 answered.id，更新当前运行状态。
    answered.id = "reliable-answer"; answered.courseId = "old"; answered.kind = "interaction";
    // 【逐行批注】 计算右侧结果并写入 answered.payload，更新当前运行状态。
    answered.payload = R"({"status":"answered","credible":true,"question":"虚构问题","answer":0,"correct":true})";
    // 【逐行批注】 判断条件 (!db.upsert(answered)) return 33;，成立时执行对应分支。
    if (!db.upsert(answered)) return 33;
    // 【逐行批注】 计算右侧结果并写入 int answerRevision，更新当前运行状态。
    const int answerRevision = db.profileRevision();
    // 【逐行批注】 计算右侧结果并写入 answered.payload，更新当前运行状态。
    answered.payload = R"({"status":"answered","credible":true,"question":"虚构问题","answer":0,"correct":true,"hintLevel":1})";
    // 【逐行批注】 判断条件 (!db.upsert(answered) || db.profileRevision() != answerRevision)…，成立时执行对应分支。
    if (!db.upsert(answered) || db.profileRevision() != answerRevision) return 34;
    // 【逐行批注】 计算右侧结果并写入 answered.payload，更新当前运行状态。
    answered.payload = R"({"status":"answered","credible":true,"question":"虚构问题","answer":1,"correct":false})";
    // 【逐行批注】 判断条件 (!db.upsert(answered) || db.profileRevision() != answerRevision …，成立时执行对应分支。
    if (!db.upsert(answered) || db.profileRevision() != answerRevision + 1) return 35;
    // 【逐行批注】 执行当前声明或表达式：db.close();。
    db.close();
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(path, ignored);。
    std::filesystem::remove(path, ignored);
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(path.u8string() + ".pre-v4.db", ignored);。
    std::filesystem::remove(path.u8string() + ".pre-v4.db", ignored);
    // 【逐行批注】 计算右侧结果并写入 auto v4Path，更新当前运行状态。
    const auto v4Path = std::filesystem::temp_directory_path() / "gangyiAI-profile-v4-tests.db";
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(v4Path, ignored);。
    std::filesystem::remove(v4Path, ignored);
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(v4Path.u8string() + ".pre-v5.db", ignored);。
    std::filesystem::remove(v4Path.u8string() + ".pre-v5.db", ignored);
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(v4Path.u8string() + ".pre-v6.db", ignored);。
    std::filesystem::remove(v4Path.u8string() + ".pre-v6.db", ignored);
    // 【逐行批注】 判断条件 (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) …，成立时执行对应分支。
    if (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) return 10;
    // 【逐行批注】 判断条件 (sqlite3_exec(legacy,，成立时执行对应分支。
    if (sqlite3_exec(legacy,
        // 【逐行批注】 调用 LearningInteraction 完成本行所需操作："CREATE TABLE LearningInteraction(id TEXT PRIMARY KEY,courseId TEXT,c…。
        "CREATE TABLE LearningInteraction(id TEXT PRIMARY KEY,courseId TEXT,conversationId TEXT,subject TEXT,kind TEXT NOT NULL,payload TEXT NOT NULL,createdAt TEXT NOT NULL);"
        // 【逐行批注】 调用 SubjectMastery 完成本行所需操作："CREATE TABLE SubjectMastery(subject TEXT PRIMARY KEY,score INTEGER,r…。
        "CREATE TABLE SubjectMastery(subject TEXT PRIMARY KEY,score INTEGER,rationale TEXT NOT NULL,weakPoints TEXT NOT NULL,recommendation TEXT NOT NULL,evidenceCount INTEGER NOT NULL,model TEXT NOT NULL,status TEXT NOT NULL,updatedAt TEXT NOT NULL);"
        // 【逐行批注】 调用 VALUES 完成本行所需操作："INSERT INTO SubjectMastery VALUES('数学',66,'旧画像','[]','复习',1,'old','r…。
        "INSERT INTO SubjectMastery VALUES('数学',66,'旧画像','[]','复习',1,'old','ready','2026-01-01');"
        // 【逐行批注】 调用 ProfileMeta 完成本行所需操作："CREATE TABLE ProfileMeta(key TEXT PRIMARY KEY,value TEXT NOT NULL);"。
        "CREATE TABLE ProfileMeta(key TEXT PRIMARY KEY,value TEXT NOT NULL);"
        // 【逐行批注】 调用 VALUES 完成本行所需操作："INSERT INTO ProfileMeta VALUES('revision','1'),('assessed','1'),('er…。
        "INSERT INTO ProfileMeta VALUES('revision','1'),('assessed','1'),('error','');"
        // 【逐行批注】 计算右侧结果并写入 "PRAGMA user_version，更新当前运行状态。
        "PRAGMA user_version=4;", nullptr, nullptr, nullptr) != SQLITE_OK) return 11;
    // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(legacy);。
    sqlite3_close(legacy);
    // 【逐行批注】 执行当前声明或表达式：db.open(v4Path.u8string());。
    db.open(v4Path.u8string());
    // 【逐行批注】 执行当前声明或表达式：db.migrate();。
    db.migrate();
    // 【逐行批注】 判断条件 (!std::filesystem::exists(v4Path.u8string() + ".pre-v5.db") ||，成立时执行对应分支。
    if (!std::filesystem::exists(v4Path.u8string() + ".pre-v5.db") ||
        // 【逐行批注】 执行当前声明或表达式：db.listMastery().size() != 1 || db.listMastery()[0].score != 66 ||。
        db.listMastery().size() != 1 || db.listMastery()[0].score != 66 ||
        // 【逐行批注】 执行当前声明或表达式：db.listMastery()[0].status != "legacy" ||。
        db.listMastery()[0].status != "legacy" ||
        // 【逐行批注】 调用 profileView 完成本行所需操作：gangyi::profileView(db)["subjects"][0]["score"] != nullptr ||。
        gangyi::profileView(db)["subjects"][0]["score"] != nullptr ||
        // 【逐行批注】 调用 profileView 完成本行所需操作：gangyi::profileView(db)["subjects"][0]["historicalScore"] != 66) retu…。
        gangyi::profileView(db)["subjects"][0]["historicalScore"] != 66) return 12;
    // 【逐行批注】 判断条件 (!gangyi::recentCourses(db).empty()) return 29;，成立时执行对应分支。
    if (!gangyi::recentCourses(db).empty()) return 29;
    // 【逐行批注】 执行当前声明或表达式：db.close();。
    db.close();
    // 重现已标记为 v5、但缺少课堂表的真实升级路径。
    // 【逐行批注】 判断条件 (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) …，成立时执行对应分支。
    if (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) return 13;
    // 【逐行批注】 判断条件 (sqlite3_exec(legacy, "DROP TABLE ClassroomActivity; DROP TABLE …，成立时执行对应分支。
    if (sqlite3_exec(legacy, "DROP TABLE ClassroomActivity; DROP TABLE WeeklyPlan; PRAGMA user_version=5;",
        // 【逐行批注】 执行当前声明或表达式：nullptr, nullptr, nullptr) != SQLITE_OK) return 14;。
        nullptr, nullptr, nullptr) != SQLITE_OK) return 14;
    // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(legacy);。
    sqlite3_close(legacy);
    // 【逐行批注】 执行当前声明或表达式：db.open(v4Path.u8string());。
    db.open(v4Path.u8string());
    // 【逐行批注】 执行当前声明或表达式：db.migrate();。
    db.migrate();
    // 【逐行批注】 判断条件 (!std::filesystem::exists(v4Path.u8string() + ".pre-v6.db") ||，成立时执行对应分支。
    if (!std::filesystem::exists(v4Path.u8string() + ".pre-v6.db") ||
        // 【逐行批注】 执行当前声明或表达式：db.listMastery().size() != 1 || db.listMastery()[0].score != 66) return 15;。
        db.listMastery().size() != 1 || db.listMastery()[0].score != 66) return 15;
    // 【逐行批注】 执行当前声明或表达式：gangyi::WeeklyPlan week;。
    gangyi::WeeklyPlan week;
    // 【逐行批注】 计算右侧结果并写入 week.courseId，更新当前运行状态。
    week.courseId = "old"; week.payload = "{\"entries\":[]}"; week.version = 7; week.updatedAt = "2026-09-30";
    // 【逐行批注】 执行当前声明或表达式：gangyi::ClassroomActivity activity;。
    gangyi::ClassroomActivity activity;
    // 【逐行批注】 计算右侧结果并写入 activity.id，更新当前运行状态。
    activity.id = "old-activity"; activity.courseId = "old"; activity.kind = "diagnostic";
    // 【逐行批注】 计算右侧结果并写入 activity.payload，更新当前运行状态。
    activity.payload = "{\"status\":\"answered\"}"; activity.updatedAt = "2026-09-30";
    // 【逐行批注】 判断条件 (!db.upsert(week) || !db.upsert(activity)) return 16;，成立时执行对应分支。
    if (!db.upsert(week) || !db.upsert(activity)) return 16;
    // 【逐行批注】 执行当前声明或表达式：db.close();。
    db.close();
    // 【逐行批注】 判断条件 (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) …，成立时执行对应分支。
    if (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) return 20;
    // 【逐行批注】 计算右侧结果并写入 sqlite3_exec(legacy, "PRAGMA user_version，更新当前运行状态。
    sqlite3_exec(legacy, "PRAGMA user_version=5", nullptr, nullptr, nullptr);
    // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(legacy);。
    sqlite3_close(legacy);
    // 【逐行批注】 执行当前声明或表达式：db.open(v4Path.u8string());。
    db.open(v4Path.u8string());
    // 【逐行批注】 执行当前声明或表达式：db.migrate();。
    db.migrate();
    // 【逐行批注】 执行当前声明或表达式：db.migrate();。
    db.migrate();
    // 【逐行批注】 判断条件 (!db.getWeeklyPlan("old") || db.getWeeklyPlan("old")->version !=…，成立时执行对应分支。
    if (!db.getWeeklyPlan("old") || db.getWeeklyPlan("old")->version != 7 ||
        // 【逐行批注】 执行当前声明或表达式：!db.getClassroomActivity("old-activity")) return 17;。
        !db.getClassroomActivity("old-activity")) return 17;
    // 【逐行批注】 执行当前声明或表达式：db.close();。
    db.close();
    // 【逐行批注】 判断条件 (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) …，成立时执行对应分支。
    if (sqlite3_open(v4Path.u8string().c_str(), &legacy) != SQLITE_OK) return 18;
    // 【逐行批注】 计算右侧结果并写入 sqlite3_stmt* statement，更新当前运行状态。
    sqlite3_stmt* statement = nullptr;
    // 【逐行批注】 调用 sqlite3_prepare_v2 完成本行所需操作：sqlite3_prepare_v2(legacy, "PRAGMA user_version", -1, &statement, nul…。
    sqlite3_prepare_v2(legacy, "PRAGMA user_version", -1, &statement, nullptr);
    // 【逐行批注】 判断条件 (sqlite3_step(statement) != SQLITE_ROW || sqlite3_column_int(sta…，成立时执行对应分支。
    if (sqlite3_step(statement) != SQLITE_ROW || sqlite3_column_int(statement, 0) != gangyi::kDatabaseSchemaVersion) return 19;
    // 【逐行批注】 调用 sqlite3_finalize 完成本行所需操作：sqlite3_finalize(statement);。
    sqlite3_finalize(statement);
    // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(legacy);。
    sqlite3_close(legacy);
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(v4Path, ignored);。
    std::filesystem::remove(v4Path, ignored);
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(v4Path.u8string() + ".pre-v5.db", ignored);。
    std::filesystem::remove(v4Path.u8string() + ".pre-v5.db", ignored);
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(v4Path.u8string() + ".pre-v6.db", ignored);。
    std::filesystem::remove(v4Path.u8string() + ".pre-v6.db", ignored);
    // 【逐行批注】 执行当前声明或表达式：std::cout << "画像迁移与本机记录测试通过\n";。
    std::cout << "画像迁移与本机记录测试通过\n";
// 【逐行批注】 结束当前代码块。
}

#include "course_service.hpp"
#include <sqlite3.h>
#include <chrono>
#include <filesystem>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Json = nlohmann::json;
void expect(bool value, const std::string& message) {
    if (!value) throw std::runtime_error(message);
}
struct Fixture {
    std::filesystem::path path = std::filesystem::temp_directory_path() /
        ("gangyi-delete-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".db");
    gangyi::Database db;
    sqlite3* sql = nullptr;
    Fixture() {
        db.open(path.u8string()); db.migrate();
        expect(sqlite3_open(path.u8string().c_str(), &sql) == SQLITE_OK, "隔离检查连接失败");
        for (const std::string id : {"a", "ab"}) seed(id);
    }
    ~Fixture() {
        if (sql) sqlite3_close(sql);
        db.close();
        std::error_code error; std::filesystem::remove(path, error);
    }
    void exec(const std::string& query) {
        char* error = nullptr;
        if (sqlite3_exec(sql, query.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
            const std::string text = error ? error : "SQLite错误"; sqlite3_free(error); throw std::runtime_error(text);
        }
    }
    Json rows(const std::string& query) {
        sqlite3_stmt* statement = nullptr;
        expect(sqlite3_prepare_v2(sql, query.c_str(), -1, &statement, nullptr) == SQLITE_OK, "检查查询失败");
        Json result = Json::array(); int status;
        while ((status = sqlite3_step(statement)) == SQLITE_ROW) {
            Json row = Json::array();
            for (int column = 0; column < sqlite3_column_count(statement); ++column) {
                const auto* text = sqlite3_column_text(statement, column);
                row.push_back(text ? Json(reinterpret_cast<const char*>(text)) : Json(nullptr));
            }
            result.push_back(row);
        }
        sqlite3_finalize(statement); expect(status == SQLITE_DONE, "检查查询未完成"); return result;
    }
    Json snapshot() {
        Json result = Json::object();
        for (const auto& name : {"Course", "CourseProgress", "CourseSnapshot", "TaskProgress", "LearningStepProgress",
            "LearningCardProgress", "LearningSession", "LearningInteraction", "ClassroomActivity", "AgentRunEvent",
            "IndependentChat", "IndependentChatMessage", "WeeklyPlan", "TopicMastery", "MyAIRecord", "ProfileMeta"})
            result[name] = rows(std::string("SELECT * FROM ") + name + " ORDER BY 1");
        return result;
    }
    void seed(const std::string& id) {
        // 只使用本测试固定身份，SQL 字符串中不拼接外部输入。
        exec("INSERT INTO Course(id,userId,goal,mode,title,createdAt,updatedAt) VALUES('"+id+"','owner-"+id+"','目标','deep','课程','now','now')");
        exec("INSERT INTO CourseProgress(id,courseId,goal,createdAt,updatedAt) VALUES('progress-"+id+"','"+id+"','目标','now','now')");
        exec("INSERT INTO CourseSnapshot(id,courseId,payload,createdAt) VALUES('snapshot-"+id+"','"+id+"','{}','now')");
        for (const auto& pair : std::vector<std::pair<std::string,std::string>>{{"TaskProgress","taskIndex,taskTitle"},
            {"LearningStepProgress","stepIndex,stepTitle"},{"LearningCardProgress","topicIndex,topicTitle"}})
            exec("INSERT INTO "+pair.first+"(id,courseId,goal,phaseIndex,phaseName,"+pair.second+") VALUES('"+
                pair.first+"-"+id+"','"+id+"','目标',1,'阶段',1,'主题')");
        exec("INSERT INTO LearningSession(id,courseId,goal,phaseIndex,phaseName,topicIndex,topicTitle,title,content) VALUES('session-"+id+"','"+id+"','目标',1,'阶段',1,'主题','课时','正文')");
        exec("INSERT INTO LearningInteraction(id,courseId,kind,payload,createdAt) VALUES('input-"+id+"','"+id+"','agent-input','{\"componentActivities\":{}}','now')");
        for (const auto& kind : {"agent-lesson", "agent-task", "agent-component-draft"})
            exec("INSERT INTO ClassroomActivity(id,courseId,phaseIndex,topicIndex,kind,payload,updatedAt) VALUES('"+
                std::string(kind)+"-"+id+"','"+id+"',1,1,'"+kind+"','{}','now')");
        exec("INSERT INTO AgentRunEvent(taskId,seq,payload,createdAt) VALUES('agent-task-"+id+"',1,'{\"text\":\"轨迹\"}','now')");
        exec("INSERT INTO IndependentChat(lessonId,createdAt,updatedAt) VALUES('agent-lesson-"+id+"','now','now')");
        exec("INSERT INTO IndependentChatMessage(id,lessonId,role,content,createdAt,updatedAt) VALUES('chat-"+id+"','agent-lesson-"+id+"','user','课程答疑','now','now')");
        exec("INSERT INTO WeeklyPlan(courseId,payload,updatedAt) VALUES('"+id+"','{}','now')");
        exec("INSERT INTO TopicMastery(courseId,phaseIndex,topic) VALUES('"+id+"',1,'主题')");
        exec("INSERT INTO MyAIRecord(id,kind,payload,updatedAt,version) VALUES('image-"+id+"','deep-image','{\"courseId\":\""+id+"\",\"bytes\":\"合成图片\"}','now',1)");
        exec("INSERT INTO MyAIRecord(id,kind,payload,updatedAt,version) VALUES('cost-"+id+"','usage','{\"courseId\":\""+id+"\",\"tokens\":null}','now',1)");
        exec("INSERT INTO MyAIRecord(id,kind,payload,updatedAt,version) VALUES('library-"+id+"','library-source','{\"courseIds\":[\""+id+"\"]}','now',1)");
        exec("INSERT INTO ProfileMeta(key,value) VALUES('lesson-reached:"+id+":1:1','true')");
    }
};

void failureRollsBack(const std::string& table) {
    Fixture fixture;
    const auto before = fixture.snapshot();
    fixture.exec("CREATE TRIGGER fail_deletion BEFORE DELETE ON "+table+" BEGIN SELECT RAISE(ABORT,'隔离测试注入删除失败'); END");
    expect(!gangyi::deleteCourseForIdentity(fixture.db,"a","owner-a",""), "故障应返回删除失败");
    expect(fixture.snapshot() == before, "删除失败留下部分删除结果："+table);
}

void deletionRemovesOnlyCourse() {
    Fixture fixture;
    const auto revision = fixture.db.learningRevision();
    expect(gangyi::deleteCourseForIdentity(fixture.db,"a","owner-a",""), "合法课程删除失败");
    for (const auto& table : {"Course", "CourseProgress", "CourseSnapshot", "TaskProgress", "LearningStepProgress", "LearningCardProgress",
        "LearningSession", "LearningInteraction", "ClassroomActivity", "WeeklyPlan", "TopicMastery"}) {
        const auto key = std::string(table) == "Course" ? "id" : "courseId";
        expect(fixture.rows(std::string("SELECT * FROM ")+table+" WHERE "+key+"='a'").empty(), std::string("课程残留：")+table);
        expect(!fixture.rows(std::string("SELECT * FROM ")+table+" WHERE "+key+"='ab'").empty(), std::string("误删另一课程：")+table);
    }
    expect(fixture.rows("SELECT * FROM AgentRunEvent WHERE taskId='agent-task-a'").empty(), "任务轨迹残留");
    expect(fixture.rows("SELECT * FROM IndependentChatMessage WHERE lessonId='agent-lesson-a'").empty(), "独立答疑残留");
    expect(fixture.rows("SELECT * FROM IndependentChat WHERE lessonId='agent-lesson-a'").empty(), "独立答疑会话残留");
    expect(fixture.rows("SELECT * FROM MyAIRecord WHERE id='image-a'").empty(), "图片字节残留");
    expect(fixture.rows("SELECT * FROM ProfileMeta WHERE key='lesson-reached:a:1:1'").empty(), "到达记录残留");
    expect(fixture.rows("SELECT * FROM AgentRunEvent WHERE taskId='agent-task-ab'").size()==1, "另一课程任务轨迹误删");
    expect(fixture.rows("SELECT * FROM IndependentChatMessage WHERE lessonId='agent-lesson-ab'").size()==1, "另一课程答疑误删");
    expect(fixture.rows("SELECT * FROM MyAIRecord WHERE id IN ('cost-a','library-a','image-ab','cost-ab','library-ab')").size()==5,
        "费用、独立资料或另一课程图片被误删");
    expect(fixture.db.learningRevision() > revision, "删除后学习依据未失效");
    const auto after = fixture.snapshot();
    expect(!gangyi::deleteCourseForIdentity(fixture.db,"a","owner-a",""), "重复删除应返回不存在");
    expect(after==fixture.snapshot(), "重复删除改变了其它记录");
}

void permissionAndNestedTransactions() {
    Fixture fixture;
    const auto before = fixture.snapshot();
    expect(!gangyi::deleteCourseForIdentity(fixture.db,"a","owner-ab",""), "跨身份删除未拒绝");
    expect(fixture.snapshot()==before, "身份不符却修改了数据");
    bool reverted = false;
    try {
        fixture.db.transaction([&]{
            expect(gangyi::deleteCourseForIdentity(fixture.db,"a","owner-a",""), "确认事务内删除失败");
            throw std::runtime_error("确认后的步骤失败");
        });
    } catch (...) { reverted = true; }
    expect(reverted && fixture.snapshot()==before, "外层确认失败未恢复完整课程");
    fixture.exec("CREATE TRIGGER fail_deletion BEFORE DELETE ON Course BEGIN SELECT RAISE(ABORT,'隔离失败'); END");
    fixture.db.transaction([&]{
        expect(!gangyi::deleteCourseForIdentity(fixture.db,"a","owner-a",""), "内层错误未报告");
        fixture.db.setProfileMeta("outer-continues","true");
    });
    fixture.exec("DELETE FROM ProfileMeta WHERE key='outer-continues'");
    expect(fixture.snapshot()==before, "内层失败破坏外层事务或留下局部删除");
}

void sharedConnectionDoesNotExposePartialDelete() {
    Fixture fixture;
    const auto before = fixture.snapshot();
    std::promise<void> changed, release, readerStarted;
    const auto continueWork = release.get_future().share();
    auto writer = std::async(std::launch::async, [&] {
        try {
            fixture.db.transaction([&] {
                expect(gangyi::deleteCourseForIdentity(fixture.db,"a","owner-a",""), "并发测试的内层删除失败");
                changed.set_value(); continueWork.wait();
                throw std::runtime_error("并发测试要求外层回滚");
            });
        } catch (...) {}
    });
    auto changedSignal = changed.get_future();
    if (changedSignal.wait_for(std::chrono::seconds(3)) != std::future_status::ready) {
        release.set_value(); writer.get(); throw std::runtime_error("并发删除未进入预期状态");
    }
    auto reader = std::async(std::launch::async, [&] {
        readerStarted.set_value(); return fixture.db.getCourse("a").has_value();
    });
    readerStarted.get_future().wait();
    const bool blocked = reader.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout;
    release.set_value(); writer.get();
    expect(blocked && reader.get(), "同连接另一请求读到了未提交的部分删除");
    expect(fixture.snapshot() == before, "并发回滚改变了课程记录");
}
void outerSavepointReservesWriteBeforeReading() {
    Fixture fixture;
    fixture.db.savepoint([&] {
        expect(fixture.db.getCourse("a").has_value(), "写事务内课程读取失败");
        // 另一个连接不能在读取之后抢先提交，使原事务无法升级为写事务。
        const auto competing = sqlite3_exec(fixture.sql, "UPDATE Course SET title='外部写入' WHERE id='ab'", nullptr, nullptr, nullptr);
        expect(competing == SQLITE_BUSY || competing == SQLITE_LOCKED, "最外层保存点未在读取前取得写入保留锁");
        expect(gangyi::deleteCourseForIdentity(fixture.db, "a", "owner-a", ""), "保留写入后课程删除失败");
    });
    const auto other = fixture.db.getCourse("ab");
    expect(!fixture.db.getCourse("a") && other && other->title == "课程", "并发连接改变了事务中的其它课程");
    fixture.exec("UPDATE Course SET title='外部写入' WHERE id='ab'");
    expect(fixture.db.getCourse("ab")->title == "外部写入", "提交后未释放跨连接写入保留锁");
}
} // namespace

int main(int argc, char** argv) {
    try {
        // 可单独重现第一处删除晚期失败，保留修复前的确定性失败证据。
        failureRollsBack("Course");
        if (argc==2 && std::string(argv[1])=="--failure-only") return 0;
        failureRollsBack("ClassroomActivity");
        deletionRemovesOnlyCourse();
        permissionAndNestedTransactions();
        sharedConnectionDoesNotExposePartialDelete();
        outerSavepointReservesWriteBeforeReading();
        std::cout << "课程删除：失败整体回滚、关联清理、课程隔离、嵌套事务、重复删除和跨连接写入保留通过\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "课程删除未通过：" << error.what() << '\n'; return 1;
    }
}

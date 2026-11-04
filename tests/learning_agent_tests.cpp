// 【逐行批注】 处理条件编译指令：#ifdef NDEBUG。
#ifdef NDEBUG
// 【逐行批注】 执行当前声明或表达式：#undef NDEBUG。
#undef NDEBUG
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"
// 【逐行批注】 引入头文件 "agent_preferences.hpp"，为后续实现提供类型或接口声明。
#include "agent_preferences.hpp"
// 【逐行批注】 引入头文件 "agent_lessons.hpp"，为后续实现提供类型或接口声明。
#include "agent_lessons.hpp"
// 【逐行批注】 引入头文件 "learning_flow.hpp"，为后续实现提供类型或接口声明。
#include "learning_flow.hpp"
// 【逐行批注】 引入头文件 "db_schema_version.hpp"，为后续实现提供类型或接口声明。
#include "db_schema_version.hpp"
// 【逐行批注】 引入头文件 "independent_chat.hpp"，为后续实现提供类型或接口声明。
#include "independent_chat.hpp"
// 【逐行批注】 引入头文件 <sqlite3.h>，为后续实现提供类型或接口声明。
#include <sqlite3.h>
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <cassert>，为后续实现提供类型或接口声明。
#include <cassert>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>

// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 定义函数 output，执行该函数声明的职责。
gangyi::AIResult output(const Json& value, const std::function<bool(const std::string&)>& sink) {
    // 【逐行批注】 计算右侧结果并写入 encoded，更新当前运行状态。
    auto encoded = value;
    // 【逐行批注】 判断条件 (!encoded.contains("teaching")) encoded["teaching"] = {{"helpTyp…，成立时执行对应分支。
    if (!encoded.contains("teaching")) encoded["teaching"] = {{"helpType", "explanation"}};
    // 【逐行批注】 判断条件 (encoded.value("actions", Json()).is_array()) for (auto& action …，成立时执行对应分支。
    if (encoded.value("actions", Json()).is_array()) for (auto& action : encoded["actions"])
        // 【逐行批注】 判断条件 (action.value("tool", "") == "append_section" && action.value("a…，成立时执行对应分支。
        if (action.value("tool", "") == "append_section" && action.value("args", Json()).is_object() &&
            // 【逐行批注】 调用 Json 完成本行所需操作：action["args"].value("section", Json()).is_object() && action["args"]…。
            action["args"].value("section", Json()).is_object() && action["args"]["section"].value("kind", "") != "question" &&
            // 【逐行批注】 执行当前声明或表达式：!action["args"]["section"].contains("helpType")) action["args"]["section"]["helpType…。
            !action["args"]["section"].contains("helpType")) action["args"]["section"]["helpType"] = "explanation";
    // 【逐行批注】 计算右侧结果并写入 auto text，更新当前运行状态。
    const auto text = encoded.dump();
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < text.size(); i += 7) if (!sink(text.substr(i, 7)))。
    for (size_t i = 0; i < text.size(); i += 7) if (!sink(text.substr(i, 7)))
        // 【逐行批注】 检测到错误情形时抛出异常：throw gangyi::AIClientError("cancelled", "测试主动中断");。
        throw gangyi::AIClientError("cancelled", "测试主动中断");
    // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
    gangyi::AIResult result; result.content = text; result.model = "fixture-real-protocol"; result.finishReason = "stop";
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 声明对象 runLibraryChecks，并通过构造函数初始化其状态。
void runLibraryChecks();
// 【逐行批注】 声明对象 runDeepCourseChecks，并通过构造函数初始化其状态。
void runDeepCourseChecks();
// schema 7 及更早版本没有事件归档表、事件序号和任务重试字段。升级必须无损保留旧课程、
// 作答、画像、课件和失败任务，并允许旧任务在新版本里继续恢复。
// 【逐行批注】 定义函数 runLegacySchemaMigrationChecks，执行该函数声明的职责。
void runLegacySchemaMigrationChecks(const std::string& path) {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 【逐行批注】 计算右侧结果并写入 std::string legacyTaskId，更新当前运行状态。
    const std::string legacyTaskId = "legacy-agent-task";
    // 【逐行批注】 执行当前声明或表达式：std::string legacyLessonPayload;。
    std::string legacyLessonPayload;
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 执行当前声明或表达式：Database db; db.open(path); db.migrate();。
        Database db; db.open(path); db.migrate();
        // 【逐行批注】 执行当前声明或表达式：db.migrate(); // 重复迁移必须保持幂等。。
        db.migrate(); // 重复迁移必须保持幂等。
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(Course{"legacy-course", "fixture", {}, "旧学习目标", "dee…。
        assert(db.insert(Course{"legacy-course", "fixture", {}, "旧学习目标", "deep", "旧课程", {}, "ai", "active",
            // 【逐行批注】 执行当前声明或表达式："2026-09-01", "2026-09-01"}));。
            "2026-09-01", "2026-09-01"}));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(LearningInteraction{"legacy-answer", "chat-user", "{…。
        assert(db.insert(LearningInteraction{"legacy-answer", "chat-user", "{\"text\":\"旧作答记录\"}",
            // 【逐行批注】 执行当前声明或表达式："2026-09-02", "legacy-course", "fixture", {}}));。
            "2026-09-02", "legacy-course", "fixture", {}}));
        // 【逐行批注】 执行当前声明或表达式：db.setProfileMeta("legacy-profile", "保留的画像");。
        db.setProfileMeta("legacy-profile", "保留的画像");
        // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
        const Json lesson = {{"id", "legacy-lesson"}, {"kind", "agent-lesson"}, {"scopeId", "legacy-scope"},
            // 【逐行批注】 执行当前声明或表达式：{"courseId", "legacy-course"}, {"title", "旧课时"}, {"status", "ready"}, {"version", 1},。
            {"courseId", "legacy-course"}, {"title", "旧课时"}, {"status", "ready"}, {"version", 1},
            // 【逐行批注】 调用 array 完成本行所需操作：{"sections", Json::array({{{"id", "legacy-section"}, {"kind", "explan…。
            {"sections", Json::array({{{"id", "legacy-section"}, {"kind", "explanation"}, {"title", "旧讲解"},
                // 【逐行批注】 执行当前声明或表达式：{"body", "旧正文"}, {"version", 1}}})}};。
                {"body", "旧正文"}, {"version", 1}}})}};
        // 【逐行批注】 计算右侧结果并写入 legacyLessonPayload，更新当前运行状态。
        legacyLessonPayload = lesson.dump();
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(ClassroomActivity{"legacy-lesson", "legacy-course", …。
        assert(db.upsert(ClassroomActivity{"legacy-lesson", "legacy-course", "agent-lesson", legacyLessonPayload,
            // 【逐行批注】 执行当前声明或表达式："2026-09-02T09:00:00Z", 0, 0}));。
            "2026-09-02T09:00:00Z", 0, 0}));
        // 【逐行批注】 计算右侧结果并写入 Json legacy，更新当前运行状态。
        const Json legacy = {{"id", legacyTaskId}, {"kind", "agent-task"}, {"scopeId", "legacy-scope"},
            // 【逐行批注】 调用 array 完成本行所需操作：{"courseIds", Json::array({"legacy-course"})}, {"status", "failed"}, …。
            {"courseIds", Json::array({"legacy-course"})}, {"status", "failed"}, {"calls", 2},
            // 【逐行批注】 执行当前声明或表达式：{"createdAt", "2026-09-02T10:00:00Z"}, {"updatedAt", "2026-09-02T10:01:00Z"},。
            {"createdAt", "2026-09-02T10:00:00Z"}, {"updatedAt", "2026-09-02T10:01:00Z"},
            // 【逐行批注】 执行当前声明或表达式：{"event", {{"type", "startup"}, {"courseId", "legacy-course"}, {"requestId", "legacy…。
            {"event", {{"type", "startup"}, {"courseId", "legacy-course"}, {"requestId", "legacy-request"}}},
            // 【逐行批注】 执行当前声明或表达式：{"message", "旧失败说明"}, {"error", "旧超时"},。
            {"message", "旧失败说明"}, {"error", "旧超时"},
            // 【逐行批注】 执行当前声明或表达式：{"failure", {{"kind", "timeout"}, {"message", "旧超时"}}},。
            {"failure", {{"kind", "timeout"}, {"message", "旧超时"}}},
            // 【逐行批注】 调用 object、array 完成本行所需操作：{"completedActions", Json::object()}, {"messages", Json::array()}, {"…。
            {"completedActions", Json::object()}, {"messages", Json::array()}, {"results", Json::array()},
            // 【逐行批注】 调用 array 完成本行所需操作：{"events", Json::array({{{"type", "failed"}, {"message", "旧超时"}}})}};。
            {"events", Json::array({{{"type", "failed"}, {"message", "旧超时"}}})}};
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(ClassroomActivity{legacyTaskId, "legacy-course", "ag…。
        assert(db.upsert(ClassroomActivity{legacyTaskId, "legacy-course", "agent-task", legacy.dump(),
            // 【逐行批注】 执行当前声明或表达式："2026-09-02T10:01:00Z", 0, 0}));。
            "2026-09-02T10:01:00Z", 0, 0}));
        // schema 7 由 agentSubmit 同步维护任务索引；模拟旧库时必须保留这项真实结构，
        // 否则重启扫描不会把孤立任务载荷当成可恢复任务。
        // 【逐行批注】 调用 array 完成本行所需操作：db.setProfileMeta("agent-index", Json::array({legacyTaskId}).dump());。
        db.setProfileMeta("agent-index", Json::array({legacyTaskId}).dump());
    // 【逐行批注】 结束当前代码块。
    }
    // 退回 schema 7：删除新增的事件归档表并恢复旧版本号，模拟从上一版本升级。
    // 【逐行批注】 计算右侧结果并写入 sqlite3* raw，更新当前运行状态。
    sqlite3* raw = nullptr;
    // 【逐行批注】 调用 assert、sqlite3_open 完成本行所需操作：assert(sqlite3_open(path.c_str(), &raw) == SQLITE_OK);。
    assert(sqlite3_open(path.c_str(), &raw) == SQLITE_OK);
    // 【逐行批注】 计算右侧结果并写入 assert(sqlite3_exec(raw, "DROP TABLE AgentRu…，更新当前运行状态。
    assert(sqlite3_exec(raw, "DROP TABLE AgentRunEvent; PRAGMA user_version=7;", nullptr, nullptr, nullptr) == SQLITE_OK);
    // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(raw);。
    sqlite3_close(raw);
    // 【逐行批注】 执行当前声明或表达式：Database db; db.open(path); db.migrate();。
    Database db; db.open(path); db.migrate();
    // 【逐行批注】 计算右侧结果并写入 sqlite3* verify，更新当前运行状态。
    sqlite3* verify = nullptr; sqlite3_stmt* statement = nullptr;
    // 【逐行批注】 调用 assert、sqlite3_open 完成本行所需操作：assert(sqlite3_open(path.c_str(), &verify) == SQLITE_OK);。
    assert(sqlite3_open(path.c_str(), &verify) == SQLITE_OK);
    // 【逐行批注】 调用 assert、sqlite3_prepare_v2 完成本行所需操作：assert(sqlite3_prepare_v2(verify, "PRAGMA user_version", -1, &stateme…。
    assert(sqlite3_prepare_v2(verify, "PRAGMA user_version", -1, &statement, nullptr) == SQLITE_OK);
    // 【逐行批注】 调用 assert、sqlite3_step、sqlite3_column_int 完成本行所需操作：assert(sqlite3_step(statement) == SQLITE_ROW && sqlite3_column_int(st…。
    assert(sqlite3_step(statement) == SQLITE_ROW && sqlite3_column_int(statement, 0) == kDatabaseSchemaVersion);
    // 【逐行批注】 调用 sqlite3_finalize 完成本行所需操作：sqlite3_finalize(statement);。
    sqlite3_finalize(statement);
    // 【逐行批注】 调用 assert、sqlite3_prepare_v2、COUNT 完成本行所需操作：assert(sqlite3_prepare_v2(verify, "SELECT COUNT(*) FROM AgentRunEvent…。
    assert(sqlite3_prepare_v2(verify, "SELECT COUNT(*) FROM AgentRunEvent", -1, &statement, nullptr) == SQLITE_OK);
    // 【逐行批注】 调用 assert、sqlite3_step、sqlite3_column_int 完成本行所需操作：assert(sqlite3_step(statement) == SQLITE_ROW && sqlite3_column_int(st…。
    assert(sqlite3_step(statement) == SQLITE_ROW && sqlite3_column_int(statement, 0) == 0);
    // 【逐行批注】 调用 sqlite3_finalize、sqlite3_close 完成本行所需操作：sqlite3_finalize(statement); sqlite3_close(verify);。
    sqlite3_finalize(statement); sqlite3_close(verify);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse("legacy-course") && db.getCourse("legacy-course")…。
    assert(db.getCourse("legacy-course") && db.getCourse("legacy-course")->title == "旧课程");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse("legacy-course")->goal == "旧学习目标");。
    assert(db.getCourse("legacy-course")->goal == "旧学习目标");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("legacy-profile") == "保留的画像");。
    assert(db.profileMeta("legacy-profile") == "保留的画像");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity("legacy-lesson") && db.getClassroomAct…。
    assert(db.getClassroomActivity("legacy-lesson") && db.getClassroomActivity("legacy-lesson")->payload == legacyLessonPayload);
    // 【逐行批注】 执行当前声明或表达式：const AgentAccess access{"legacy-scope", {"legacy-course"}};。
    const AgentAccess access{"legacy-scope", {"legacy-course"}};
    // 【逐行批注】 计算右侧结果并写入 auto legacyView，更新当前运行状态。
    const auto legacyView = agentView(db, access, legacyTaskId);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(legacyView["status"] == "failed" && legacyView["failure"]["mes…。
    assert(legacyView["status"] == "failed" && legacyView["failure"]["message"] == "旧超时");
    // 旧失败任务在新版本里仍可重试；保存时新事件写入归档表，旧字段与旧数据不被覆盖。
    // 【逐行批注】 调用 assert、agentControl 完成本行所需操作：assert(agentControl(db, access, {{"command", "retry"}, {"taskId", leg…。
    assert(agentControl(db, access, {{"command", "retry"}, {"taskId", legacyTaskId}})["status"] == "pending");
    // 【逐行批注】 调用 assert、agentControl 完成本行所需操作：assert(agentControl(db, access, {{"command", "cancel"}, {"taskId", le…。
    assert(agentControl(db, access, {{"command", "cancel"}, {"taskId", legacyTaskId}})["status"] == "cancelled");
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, legacyTaskId)["status"] == "cancelled");。
    assert(agentView(db, access, legacyTaskId)["status"] == "cancelled");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!db.agentRunEvents(legacyTaskId, 0, 500).empty());。
    assert(!db.agentRunEvents(legacyTaskId, 0, 500).empty());
    // 软件重启时遗留的 running 任务必须恢复为 pending，而不是被当成已完成或重复执行。
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 row，更新当前运行状态。
        auto row = *db.getClassroomActivity(legacyTaskId);
        // 【逐行批注】 计算右侧结果并写入 payload，更新当前运行状态。
        auto payload = Json::parse(row.payload);
        // 【逐行批注】 计算右侧结果并写入 payload["status"]，更新当前运行状态。
        payload["status"] = "running";
        // 【逐行批注】 计算右侧结果并写入 row.payload，更新当前运行状态。
        row.payload = payload.dump();
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(row));。
        assert(db.upsert(row));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 restart，并通过构造函数初始化其状态。
        LearningAgent restart(path, [&](const auto&, const auto&) -> AIResult {
            // 【逐行批注】 结束当前函数并返回结果：return {R"({"message":"","actions":[],"state":"completed"})", "fixtur…。
            return {R"({"message":"","actions":[],"state":"completed"})", "fixture", 200, "stop", "not_requested", {}};
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 执行当前声明或表达式：restart.start();。
        restart.start();
        // 【逐行批注】 执行当前声明或表达式：restart.stop();。
        restart.stop();
    // 【逐行批注】 结束当前代码块。
    }
    // 重启后不允许残留「执行中」状态：要么已恢复执行完，要么被暂停等待恢复。
    // 【逐行批注】 计算右侧结果并写入 auto afterRestart，更新当前运行状态。
    const auto afterRestart = agentView(db, access, legacyTaskId)["status"].get<std::string>();
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(afterRestart != "running" && afterRestart != "pending");。
    assert(afterRestart != "running" && afterRestart != "pending");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse("legacy-course")->goal == "旧学习目标" && db.profileMe…。
    assert(db.getCourse("legacy-course")->goal == "旧学习目标" && db.profileMeta("legacy-profile") == "保留的画像");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 runIndependentChatChecks，执行该函数声明的职责。
void runIndependentChatChecks(const std::string& path) {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 【逐行批注】 执行当前声明或表达式：Database db; db.open(path); db.migrate();。
    Database db; db.open(path); db.migrate();
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(Course{"independent-course", "fixture", {}, "不能泄露的课程…。
    assert(db.insert(Course{"independent-course", "fixture", {}, "不能泄露的课程目标", "deep",
        // 【逐行批注】 执行当前声明或表达式："不能泄露的课程标题", {}, "ai", "active", "2026-10-30", "2026-10-30"}));。
        "不能泄露的课程标题", {}, "ai", "active", "2026-10-30", "2026-10-30"}));
    // 【逐行批注】 按循环规则重复处理：for (const auto* id : {"chat-lesson-a", "chat-lesson-b"}) {。
    for (const auto* id : {"chat-lesson-a", "chat-lesson-b"}) {
        // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
        const Json lesson = {{"id", id}, {"scopeId", "independent"}, {"courseId", "independent-course"},
            // 【逐行批注】 执行当前声明或表达式：{"title", "不能泄露的课时标题"}, {"purpose", "不能泄露的课时内容"}, {"status", "ready"},。
            {"title", "不能泄露的课时标题"}, {"purpose", "不能泄露的课时内容"}, {"status", "ready"},
            // 【逐行批注】 调用 array 完成本行所需操作：{"version", 1}, {"sections", Json::array()}, {"timeline", Json::array…。
            {"version", 1}, {"sections", Json::array()}, {"timeline", Json::array()}, {"dialog", Json::array()}};
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(ClassroomActivity{id, "independent-course", "agent-l…。
        assert(db.upsert(ClassroomActivity{id, "independent-course", "agent-lesson", lesson.dump(),
            // 【逐行批注】 执行当前声明或表达式："2026-10-30", 0, 0}));。
            "2026-10-30", 0, 0}));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：const AgentAccess access{"independent", {"independent-course"}};。
    const AgentAccess access{"independent", {"independent-course"}};
    // 【逐行批注】 计算右侧结果并写入 auto revision，更新当前运行状态。
    const auto revision = db.learningRevision();
    // 【逐行批注】 计算右侧结果并写入 auto sent，更新当前运行状态。
    const auto sent = independentChatSend(db, access, {{"lessonId", "chat-lesson-a"}, {"text", "今天有什么科技新闻？"},
        // 【逐行批注】 执行当前声明或表达式：{"requestId", "independent-request"}, {"webEnabled", true}});。
        {"requestId", "independent-request"}, {"webEnabled", true}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(sent.at("chat").at("messages").size() == 1);。
    assert(sent.at("chat").at("messages").size() == 1);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(sent.at("chat").at("webEnabled") == true);。
    assert(sent.at("chat").at("webEnabled") == true);
    // 【逐行批注】 计算右侧结果并写入 bool decided，更新当前运行状态。
    bool decided = false, answered = false;
    // 【逐行批注】 定义函数 chat，执行该函数声明的职责。
    LearningAgent chat(path, [&](const ChatOptions& options, const auto& sink) {
        // 【逐行批注】 判断条件 (options.activity.purpose == "判断是否需要联网") {，成立时执行对应分支。
        if (options.activity.purpose == "判断是否需要联网") {
            // 【逐行批注】 计算右侧结果并写入 decided，更新当前运行状态。
            decided = true;
            // 【逐行批注】 计算右侧结果并写入 auto prompt，更新当前运行状态。
            const auto prompt = options.messages.back().content;
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(prompt.find("不能泄露的课程") == std::string::npos);。
            assert(prompt.find("不能泄露的课程") == std::string::npos);
            // 【逐行批注】 结束当前函数并返回结果：return output({{"needsSearch", true}, {"query", "今日科技新闻"}}, sink);。
            return output({{"needsSearch", true}, {"query", "今日科技新闻"}}, sink);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.activity.purpose == "联网检索并回答");。
        assert(options.activity.purpose == "联网检索并回答");
        // 【逐行批注】 计算右侧结果并写入 answered，更新当前运行状态。
        answered = true;
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.searchQuery == "今日科技新闻");。
        assert(options.searchQuery == "今日科技新闻");
        // 【逐行批注】 按循环规则重复处理：for (const auto& message : options.messages)。
        for (const auto& message : options.messages)
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(message.content.find("不能泄露的课程") == std::string::npos);。
            assert(message.content.find("不能泄露的课程") == std::string::npos);
        // 【逐行批注】 计算右侧结果并写入 std::string text，更新当前运行状态。
        const std::string text = "这是独立聊天的联网回答。";
        // 【逐行批注】 调用 assert、sink 完成本行所需操作：assert(sink(text.substr(0, 6)) && sink(text.substr(6)));。
        assert(sink(text.substr(0, 6)) && sink(text.substr(6)));
        // 【逐行批注】 计算右侧结果并写入 AIResult result; result.content，更新当前运行状态。
        AIResult result; result.content = text; result.model = "fixture-independent";
        // 【逐行批注】 计算右侧结果并写入 result.finishReason，更新当前运行状态。
        result.finishReason = "stop"; result.sources = {"https://example.com/news"};
        // 【逐行批注】 结束当前函数并返回结果：return result;。
        return result;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 执行当前声明或表达式：chat.process(db, sent.at("task").at("id"));。
    chat.process(db, sent.at("task").at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto view，更新当前运行状态。
    const auto view = independentChatView(db, access, "chat-lesson-a");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(decided && answered && view.at("messages").size() == 2);。
    assert(decided && answered && view.at("messages").size() == 2);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(view.at("messages").back().at("content") == "这是独立聊天的联网回答。");。
    assert(view.at("messages").back().at("content") == "这是独立聊天的联网回答。");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(view.at("messages").back().at("sources").at(0) == "https://exa…。
    assert(view.at("messages").back().at("sources").at(0) == "https://example.com/news");
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, sent.at("task").at("id")).at("status") =…。
    assert(agentView(db, access, sent.at("task").at("id")).at("status") == "ready");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.learningRevision() == revision);。
    assert(db.learningRevision() == revision);
    // 【逐行批注】 计算右侧结果并写入 auto other，更新当前运行状态。
    const auto other = independentChatView(db, access, "chat-lesson-b");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(other.at("messages").empty() && other.at("webEnabled") == fals…。
    assert(other.at("messages").empty() && other.at("webEnabled") == false);
    // 未开启联网时第一条流式回复到达前任务还没有 activity，也必须能正常保存。
    const auto plain = independentChatSend(db, access, {{"lessonId", "chat-lesson-b"}, {"text", "普通问答"},
        {"requestId", "plain-independent-request"}, {"webEnabled", false}});
    LearningAgent plainChat(path, [&](const ChatOptions& options, const auto& sink) {
        assert(options.activity.purpose == "回答消息" && options.searchQuery.empty());
        const std::string text = "普通独立回答。";
        assert(sink(text.substr(0, 6)) && sink(text.substr(6)));
        AIResult result; result.content = text; result.model = "fixture-plain";
        result.finishReason = "stop";
        return result;
    });
    plainChat.process(db, plain.at("task").at("id"));
    const auto plainView = independentChatView(db, access, "chat-lesson-b");
    assert(plainView.at("messages").size() == 2);
    assert(plainView.at("messages").back().at("content") == "普通独立回答。");
    // 【逐行批注】 调用 assert、independentChatClear 完成本行所需操作：assert(independentChatClear(db, access, {{"lessonId", "chat-lesson-a"…。
    assert(independentChatClear(db, access, {{"lessonId", "chat-lesson-a"}}).at("messages").empty());
    // 【逐行批注】 调用 assert、independentChatView 完成本行所需操作：assert(independentChatView(db, access, "chat-lesson-a").at("messages"…。
    assert(independentChatView(db, access, "chat-lesson-a").at("messages").empty());
    // 【逐行批注】 计算右侧结果并写入 auto clearedSession，更新当前运行状态。
    const auto clearedSession = db.independentChatSession("chat-lesson-a");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(clearedSession && clearedSession->activeTaskId.empty() &&。
    assert(clearedSession && clearedSession->activeTaskId.empty() &&
        // 【逐行批注】 执行当前声明或表达式：clearedSession->updatedAt.size() >= 20 && clearedSession->updatedAt[4] == '-');。
        clearedSession->updatedAt.size() >= 20 && clearedSession->updatedAt[4] == '-');

    // 较早的流式正文已转入归档表时，展示回执仍必须可以校验。
    // 【逐行批注】 计算右侧结果并写入 Json archivedTask，更新当前运行状态。
    const Json archivedTask = {{"id", "archived-exposure-task"}, {"scopeId", "independent"},
        // 【逐行批注】 执行当前声明或表达式：{"courseIds", {"independent-course"}}, {"status", "ready"}, {"seq", 20},。
        {"courseIds", {"independent-course"}}, {"status", "ready"}, {"seq", 20},
        // 【逐行批注】 调用 array 完成本行所需操作：{"events", Json::array({{{"seq", 20}, {"type", "activity"}, {"activit…。
        {"events", Json::array({{{"seq", 20}, {"type", "activity"}, {"activity", {{"title", "已完成"}}}}})}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(ClassroomActivity{"archived-exposure-task", "indepen…。
    assert(db.upsert(ClassroomActivity{"archived-exposure-task", "independent-course", "agent-task",
        // 【逐行批注】 执行当前声明或表达式：archivedTask.dump(), "2026-10-30", 0, 0}));。
        archivedTask.dump(), "2026-10-30", 0, 0}));
    // 【逐行批注】 计算右侧结果并写入 Json archivedDelta，更新当前运行状态。
    const Json archivedDelta = {{"seq", 5}, {"type", "delta"}, {"field", "message"},
        // 【逐行批注】 执行当前声明或表达式：{"text", "已经展示过的正文"}};。
        {"text", "已经展示过的正文"}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.appendAgentRunEvent("archived-exposure-task", 5, archivedDe…。
    assert(db.appendAgentRunEvent("archived-exposure-task", 5, archivedDelta.dump(), "2026-10-30"));
    // 【逐行批注】 计算右侧结果并写入 auto exposure，更新当前运行状态。
    const auto exposure = agentExposeTask(db, access, {{"taskId", "archived-exposure-task"},
        // 【逐行批注】 执行当前声明或表达式：{"seq", 20}, {"displayedSequences", {5}}});。
        {"seq", 20}, {"displayedSequences", {5}}});
    // 【逐行批注】 调用 assert、find 完成本行所需操作：assert(std::find(exposure["displayedSequences"].begin(), exposure["di…。
    assert(std::find(exposure["displayedSequences"].begin(), exposure["displayedSequences"].end(), 5) !=
        // 【逐行批注】 执行当前声明或表达式：exposure["displayedSequences"].end());。
        exposure["displayedSequences"].end());
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 引入头文件 "teaching_evidence_checks.hpp"，为后续实现提供类型或接口声明。
#include "teaching_evidence_checks.hpp"
// 【逐行批注】 引入头文件 "evaluation_feedback_checks.hpp"，为后续实现提供类型或接口声明。
#include "evaluation_feedback_checks.hpp"
// 【逐行批注】 引入头文件 "demonstrated_question_checks.hpp"，为后续实现提供类型或接口声明。
#include "demonstrated_question_checks.hpp"
// 【逐行批注】 引入头文件 "agent_efficiency_checks.hpp"，为后续实现提供类型或接口声明。
#include "agent_efficiency_checks.hpp"
// 【逐行批注】 引入头文件 "agent_quantum_checks.hpp"，为后续实现提供类型或接口声明。
#include "agent_quantum_checks.hpp"
// 【逐行批注】 引入头文件 "startup_reuse_checks.hpp"，为后续实现提供类型或接口声明。
#include "startup_reuse_checks.hpp"
// 【逐行批注】 引入头文件 "section_navigation_checks.hpp"，为后续实现提供类型或接口声明。
#include "section_navigation_checks.hpp"
// 【逐行批注】 引入头文件 "lesson_start_binding_checks.hpp"，为后续实现提供类型或接口声明。
#include "lesson_start_binding_checks.hpp"
// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main(int argc, char** argv) {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 _putenv_s 完成本行所需操作：_putenv_s("GANGYI_AGENT_RETRY_BASE_MS", "1");。
    _putenv_s("GANGYI_AGENT_RETRY_BASE_MS", "1");
    // 【逐行批注】 调用 _putenv_s 完成本行所需操作：_putenv_s("GANGYI_AI_RETRY_BASE_MS", "1");。
    _putenv_s("GANGYI_AI_RETRY_BASE_MS", "1");
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 setenv 完成本行所需操作：setenv("GANGYI_AGENT_RETRY_BASE_MS", "1", 1);。
    setenv("GANGYI_AGENT_RETRY_BASE_MS", "1", 1);
    // 【逐行批注】 调用 setenv 完成本行所需操作：setenv("GANGYI_AI_RETRY_BASE_MS", "1", 1);。
    setenv("GANGYI_AI_RETRY_BASE_MS", "1", 1);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 判断条件 (argc == 2 && std::string(argv[1]) == "--deep-course-only") {，成立时执行对应分支。
    if (argc == 2 && std::string(argv[1]) == "--deep-course-only") {
        // 【逐行批注】 调用 runDeepCourseChecks 完成本行所需操作：runDeepCourseChecks(); return 0;。
        runDeepCourseChecks(); return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (argc == 2 && std::string(argv[1]) == "--lesson-start-only") {，成立时执行对应分支。
    if (argc == 2 && std::string(argv[1]) == "--lesson-start-only") {
        // 【逐行批注】 计算右侧结果并写入 auto isolated，更新当前运行状态。
        const auto isolated = std::filesystem::temp_directory_path() /
            // 【逐行批注】 调用 to_string、now 完成本行所需操作：("gangyi-lesson-start-" + std::to_string(std::chrono::steady_clock::n…。
            ("gangyi-lesson-start-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(isolated);。
        std::filesystem::create_directories(isolated);
        // 【逐行批注】 调用 runLessonStartBindingChecks 完成本行所需操作：runLessonStartBindingChecks(isolated / "fixture.db");。
        runLessonStartBindingChecks(isolated / "fixture.db");
        // 【逐行批注】 调用 remove_all 完成本行所需操作：std::filesystem::remove_all(isolated);。
        std::filesystem::remove_all(isolated);
        // 【逐行批注】 执行当前声明或表达式：std::cout << "先练后讲、执行中延后、跨连接接管和旧任务隔离检查通过；付费请求0。\n"; return 0;。
        std::cout << "先练后讲、执行中延后、跨连接接管和旧任务隔离检查通过；付费请求0。\n"; return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (argc == 2 && std::string(argv[1]) == "--section-navigation-only…，成立时执行对应分支。
    if (argc == 2 && std::string(argv[1]) == "--section-navigation-only") {
        // 【逐行批注】 计算右侧结果并写入 auto isolated，更新当前运行状态。
        const auto isolated = std::filesystem::temp_directory_path() /
            // 【逐行批注】 调用 to_string、now 完成本行所需操作：("gangyi-section-navigation-" + std::to_string(std::chrono::steady_cl…。
            ("gangyi-section-navigation-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(isolated);。
        std::filesystem::create_directories(isolated);
        // 【逐行批注】 调用 runSectionNavigationChecks 完成本行所需操作：runSectionNavigationChecks(isolated / "fixture.db");。
        runSectionNavigationChecks(isolated / "fixture.db");
        // 【逐行批注】 调用 remove_all 完成本行所需操作：std::filesystem::remove_all(isolated);。
        std::filesystem::remove_all(isolated);
        // 【逐行批注】 执行当前声明或表达式：std::cout << "原卡片回执、页面归属、旧回执兼容与权限检查通过；付费请求0。\n"; return 0;。
        std::cout << "原卡片回执、页面归属、旧回执兼容与权限检查通过；付费请求0。\n"; return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (argc == 2 && std::string(argv[1]) == "--startup-reuse-only") {，成立时执行对应分支。
    if (argc == 2 && std::string(argv[1]) == "--startup-reuse-only") {
        // 【逐行批注】 计算右侧结果并写入 auto isolated，更新当前运行状态。
        const auto isolated = std::filesystem::temp_directory_path() /
            // 【逐行批注】 调用 to_string、now 完成本行所需操作：("gangyi-startup-reuse-" + std::to_string(std::chrono::steady_clock::…。
            ("gangyi-startup-reuse-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(isolated);。
        std::filesystem::create_directories(isolated);
        // 【逐行批注】 调用 runStartupReuseChecks 完成本行所需操作：runStartupReuseChecks(isolated / "fixture.db");。
        runStartupReuseChecks(isolated / "fixture.db");
        // 【逐行批注】 调用 remove_all 完成本行所需操作：std::filesystem::remove_all(isolated);。
        std::filesystem::remove_all(isolated);
        // 【逐行批注】 执行当前声明或表达式：std::cout << "启动复用、跨连接恢复、状态失效和开关检查通过；付费请求0。\n"; return 0;。
        std::cout << "启动复用、跨连接恢复、状态失效和开关检查通过；付费请求0。\n"; return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (argc == 2 && std::string(argv[1]) == "--teaching-protocol-only"…，成立时执行对应分支。
    if (argc == 2 && std::string(argv[1]) == "--teaching-protocol-only") {
        // 【逐行批注】 计算右侧结果并写入 auto isolated，更新当前运行状态。
        const auto isolated = std::filesystem::temp_directory_path() /
            // 【逐行批注】 调用 to_string、now 完成本行所需操作：("gangyi-teaching-protocol-" + std::to_string(std::chrono::steady_clo…。
            ("gangyi-teaching-protocol-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(isolated);。
        std::filesystem::create_directories(isolated);
        // 【逐行批注】 调用 runTeachingProtocolChecks 完成本行所需操作：runTeachingProtocolChecks(isolated / "fixture.db");。
        runTeachingProtocolChecks(isolated / "fixture.db");
        // 【逐行批注】 调用 remove_all 完成本行所需操作：std::filesystem::remove_all(isolated);。
        std::filesystem::remove_all(isolated);
        // 【逐行批注】 执行当前声明或表达式：std::cout << "坏 JSON 丢弃、自动恢复与手动兜底检查通过；付费请求0。\n"; return 0;。
        std::cout << "坏 JSON 丢弃、自动恢复与手动兜底检查通过；付费请求0。\n"; return 0;
    // 【逐行批注】 结束当前代码块。
    }
    if (argc == 2 && std::string(argv[1]) == "--independent-chat-only") {
        const auto isolated = std::filesystem::temp_directory_path() /
            ("gangyi-independent-chat-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(isolated);
        runIndependentChatChecks((isolated / "fixture.db").string());
        std::filesystem::remove_all(isolated);
        std::cout << "独立聊天流式回复、联网隔离与按课时保存检查通过；付费请求0。\n"; return 0;
    }
    // 【逐行批注】 调用 runLibraryChecks 完成本行所需操作：runLibraryChecks();。
    runLibraryChecks();
    // 【逐行批注】 计算右侧结果并写入 auto root，更新当前运行状态。
    const auto root = std::filesystem::temp_directory_path() /
        // 【逐行批注】 调用 to_string、now 完成本行所需操作：("gangyi-agent-" + std::to_string(std::chrono::steady_clock::now().ti…。
        ("gangyi-agent-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(root);。
    std::filesystem::create_directories(root);
    // 【逐行批注】 调用 runLegacySchemaMigrationChecks 完成本行所需操作：runLegacySchemaMigrationChecks((root / "legacy-schema.db").string());。
    runLegacySchemaMigrationChecks((root / "legacy-schema.db").string());
    // 【逐行批注】 执行当前声明或表达式：std::cout << "旧库与旧任务迁移到 schema " << gangyi::kDatabaseSchemaVersion << " 无损通过\n";。
    std::cout << "旧库与旧任务迁移到 schema " << gangyi::kDatabaseSchemaVersion << " 无损通过\n";
    // 【逐行批注】 调用 runIndependentChatChecks 完成本行所需操作：runIndependentChatChecks((root / "independent-chat.db").string());。
    runIndependentChatChecks((root / "independent-chat.db").string());
    // 【逐行批注】 执行当前声明或表达式：std::cout << "独立聊天隔离、按课时保存、手动联网与清理检查通过\n";。
    std::cout << "独立聊天隔离、按课时保存、手动联网与清理检查通过\n";
    // 【逐行批注】 计算右侧结果并写入 auto path，更新当前运行状态。
    const auto path = (root / "fixture.db").string();
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(path); db.migrate();。
        gangyi::Database db; db.open(path); db.migrate();
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::Course{"chem", "fixture", {}, "氧化还原", "deep"…。
        assert(db.insert(gangyi::Course{"chem", "fixture", {}, "氧化还原", "deep", "化学",
            // 【逐行批注】 执行当前声明或表达式：{}, "ai", "active", "2026-10-04", "2026-10-04"}));。
            {}, "ai", "active", "2026-10-04", "2026-10-04"}));
        // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess access{"fixture", {"chem"}};。
        const gangyi::AgentAccess access{"fixture", {"chem"}};
        // 【逐行批注】 计算右侧结果并写入 auto initialFingerprint，更新当前运行状态。
        const auto initialFingerprint = gangyi::agentLearnerFingerprint(db, access);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::LearningInteraction{"ai-only", "chat-assista…。
        assert(db.insert(gangyi::LearningInteraction{"ai-only", "chat-assistant", "{\"text\":\"教师自己写出的答案\"}",
            // 【逐行批注】 执行当前声明或表达式："2026-10-04", "chem", "fixture", {}}));。
            "2026-10-04", "chem", "fixture", {}}));
        // 【逐行批注】 调用 assert、agentLearnerFingerprint 完成本行所需操作：assert(gangyi::agentLearnerFingerprint(db, access) == initialFingerpr…。
        assert(gangyi::agentLearnerFingerprint(db, access) == initialFingerprint);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::LearningInteraction{"student", "chat-user", …。
        assert(db.insert(gangyi::LearningInteraction{"student", "chat-user", "{\"text\":\"我认为化合价发生变化\"}",
            // 【逐行批注】 执行当前声明或表达式："2026-10-04", "chem", "fixture", {}}));。
            "2026-10-04", "chem", "fixture", {}}));
        // 【逐行批注】 调用 assert、agentLearnerFingerprint 完成本行所需操作：assert(gangyi::agentLearnerFingerprint(db, access) != initialFingerpr…。
        assert(gangyi::agentLearnerFingerprint(db, access) != initialFingerprint);

        // 【逐行批注】 计算右侧结果并写入 Json event，更新当前运行状态。
        const Json event = {{"type", "answer"}, {"courseId", "chem"}, {"requestId", "one"}, {"text", "化合价升降"}};
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = gangyi::agentSubmit(db, access, event);
        // 【逐行批注】 调用 assert、agentSubmit 完成本行所需操作：assert(gangyi::agentSubmit(db, access, event)["id"] == task["id"]);。
        assert(gangyi::agentSubmit(db, access, event)["id"] == task["id"]);
        // 【逐行批注】 计算右侧结果并写入 bool mismatch，更新当前运行状态。
        bool mismatch = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { auto changed = event; changed["text"] = "不同回答"; gangyi::agentSubmit(db, access, changed); }
        // 【逐行批注】 捕获并处理异常：catch (...) { mismatch = true; }。
        catch (...) { mismatch = true; }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(mismatch);。
        assert(mismatch);
        // 【逐行批注】 计算右侧结果并写入 bool denied，更新当前运行状态。
        bool denied = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::agentView(db, {"another-user", {"chem"}}, task["id"]); }
        // 【逐行批注】 捕获并处理异常：catch (...) { denied = true; }。
        catch (...) { denied = true; }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(denied);。
        assert(denied);

        // 【逐行批注】 计算右侧结果并写入 int calls，更新当前运行状态。
        int calls = 0, writes = 0;
        // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
        gangyi::LearningAgent agent(path, [&](const auto& options, const auto& sink) {
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.maxTokens == 8192 && options.timeoutMs == 240000);。
            assert(options.maxTokens == 8192 && options.timeoutMs == 240000);
            // 【逐行批注】 执行当前声明或表达式：++calls;。
            ++calls;
            // 【逐行批注】 计算右侧结果并写入 auto inFlight，更新当前运行状态。
            const auto inFlight = gangyi::agentView(db, access, task["id"]);
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(inFlight["status"] == "running" && inFlight["activity"]["phase…。
            assert(inFlight["status"] == "running" && inFlight["activity"]["phase"] == "request");
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(inFlight["activity"]["call"] == calls && inFlight["purpose"] =…。
            assert(inFlight["activity"]["call"] == calls && inFlight["purpose"] == "处理学习请求");
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(inFlight["target"]["courseTitle"] == "化学");。
            assert(inFlight["target"]["courseTitle"] == "化学");
            // 【逐行批注】 判断条件 (calls == 1) return output({，成立时执行对应分支。
            if (calls == 1) return output({
                // 【逐行批注】 执行当前声明或表达式：{"message", "根据你写出的理由，先补上氧化与还原的关系。"},。
                {"message", "根据你写出的理由，先补上氧化与还原的关系。"},
                // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "write"}, {"tool", "record_fixture"}, {"args", {{"value", "真实模型…。
                {"actions", {{{"id", "write"}, {"tool", "record_fixture"}, {"args", {{"value", "真实模型选择的动作"}, {"expectedAnswer", "PRIVATE-ACTIVITY-ARGS"}}}}}},
                // 【逐行批注】 执行当前声明或表达式：{"state", "continue"}}, sink);。
                {"state", "continue"}}, sink);
            // 【逐行批注】 判断条件 (calls == 2) return output({{"message", "确认已经保存的动作。"},，成立时执行对应分支。
            if (calls == 2) return output({{"message", "确认已经保存的动作。"},
                // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "write"}, {"tool", "record_fixture"}, {"args", {{"value", "真实模型…。
                {"actions", {{{"id", "write"}, {"tool", "record_fixture"}, {"args", {{"value", "真实模型选择的动作"}, {"expectedAnswer", "PRIVATE-ACTIVITY-ARGS"}}}}}},
                // 【逐行批注】 执行当前声明或表达式：{"state", "continue"}}, sink);。
                {"state", "continue"}}, sink);
            // 【逐行批注】 判断条件 (calls < 25) return output({{"message", "继续检查真实记录。"},，成立时执行对应分支。
            if (calls < 25) return output({{"message", "继续检查真实记录。"},
                // 【逐行批注】 调用 to_string、object 完成本行所需操作：{"actions", {{{"id", "read-" + std::to_string(calls)}, {"tool", "read…。
                {"actions", {{{"id", "read-" + std::to_string(calls)}, {"tool", "read_context"}, {"args", Json::object()}}}},
                // 【逐行批注】 执行当前声明或表达式：{"state", "continue"}}, sink);。
                {"state", "continue"}}, sink);
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "请你解释为什么铁的化合价升高。"}, {"actions", Json::arra…。
            return output({{"message", "请你解释为什么铁的化合价升高。"}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("record_fixture", {"保存虚构测试动作", false,。
        agent.registerTool("record_fixture", {"保存虚构测试动作", false,
            // 【逐行批注】 执行当前声明或表达式：[&](gangyi::Database&, const Json& args, const Json&, const gangyi::AIResult& proven…。
            [&](gangyi::Database&, const Json& args, const Json&, const gangyi::AIResult& provenance) {
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(provenance.model == "fixture-real-protocol");。
                assert(provenance.model == "fixture-real-protocol");
                // 【逐行批注】 计算右侧结果并写入 auto executing，更新当前运行状态。
                const auto executing = gangyi::agentView(db, access, task["id"]);
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(executing["activity"]["phase"] == "tool" && executing["activit…。
                assert(executing["activity"]["phase"] == "tool" && executing["activity"]["state"] == "started");
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(executing["activity"]["tool"] == "record_fixture");。
                assert(executing["activity"]["tool"] == "record_fixture");
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(executing.dump().find("PRIVATE-ACTIVITY-ARGS") == std::string:…。
                assert(executing.dump().find("PRIVATE-ACTIVITY-ARGS") == std::string::npos);
                // 【逐行批注】 结束当前函数并返回结果：return gangyi::PreparedAgentTool{{{"ok", true}, {"rubric", "PRIVATE-A…。
                return gangyi::PreparedAgentTool{{{"ok", true}, {"rubric", "PRIVATE-ACTIVITY-RESULT"}}, [&, value = args["value"]](gangyi::Database& connection) {
                    // 【逐行批注】 执行当前声明或表达式：connection.setProfileMeta("fixture-agent-action", value.get<std::string>()); ++write…。
                    connection.setProfileMeta("fixture-agent-action", value.get<std::string>()); ++writes;
                // 【逐行批注】 执行当前声明或表达式：}};。
                }};
            // 【逐行批注】 执行当前声明或表达式：}});。
            }});
        // 【逐行批注】 执行当前声明或表达式：agent.process(db, task["id"]);。
        agent.process(db, task["id"]);
        // 【逐行批注】 计算右侧结果并写入 auto finished，更新当前运行状态。
        const auto finished = gangyi::agentView(db, access, task["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(finished["status"] == "waiting_student" && finished["calls"] =…。
        assert(finished["status"] == "waiting_student" && finished["calls"] == 25 && writes == 1);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(finished["model"] == "fixture-real-protocol");。
        assert(finished["model"] == "fixture-real-protocol");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("fixture-agent-action") == "真实模型选择的动作");。
        assert(db.profileMeta("fixture-agent-action") == "真实模型选择的动作");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(!finished.contains("messages") && !finished.contains("results"…。
        assert(!finished.contains("messages") && !finished.contains("results") && !finished.contains("courseIds"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(finished.dump().find("PRIVATE-ACTIVITY") == std::string::npos);。
        assert(finished.dump().find("PRIVATE-ACTIVITY") == std::string::npos);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(finished["activity"]["phase"] == "finished" && finished["activ…。
        assert(finished["activity"]["phase"] == "finished" && finished["activity"]["state"] == "completed");
        // 【逐行批注】 计算右侧结果并写入 int savedActions，更新当前运行状态。
        int savedActions = 0;
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : finished["events"]) if (item["type"] == "action") {。
        for (const auto& item : finished["events"]) if (item["type"] == "action") {
            // 【逐行批注】 调用 assert 完成本行所需操作：++savedActions; assert(item["activity"]["state"] == "completed");。
            ++savedActions; assert(item["activity"]["state"] == "completed");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(savedActions == 23); // 缓存命中的动作不重复显示为新完成步骤。。
        assert(savedActions == 23); // 缓存命中的动作不重复显示为新完成步骤。
        // 【逐行批注】 计算右侧结果并写入 auto compactPayload，更新当前运行状态。
        const auto compactPayload = Json::parse(db.getClassroomActivity(task["id"])->payload);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(compactPayload.at("events").size() <= 96);。
        assert(compactPayload.at("events").size() <= 96);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(!db.agentRunEvents(task["id"], 0, 500).empty());。
        assert(!db.agentRunEvents(task["id"], 0, 500).empty());
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(finished.at("events").size() >= 23);。
        assert(finished.at("events").size() >= 23);
        // 【逐行批注】 按循环规则重复处理：for (int i = 0; i < 5; ++i) gangyi::agentView(db, access);。
        for (int i = 0; i < 5; ++i) gangyi::agentView(db, access);
        // 【逐行批注】 调用 assert 完成本行所需操作：agent.process(db, task["id"]); assert(calls == 25);。
        agent.process(db, task["id"]); assert(calls == 25);

        // 【逐行批注】 计算右侧结果并写入 int badCalls，更新当前运行状态。
        int badCalls = 0;
        // 【逐行批注】 定义函数 bad，执行该函数声明的职责。
        gangyi::LearningAgent bad(path, [&](const auto&, const auto& sink) {
            // 【逐行批注】 调用 output 完成本行所需操作：++badCalls; return output({{"message", "错误输出"}, {"actions", "不是数组"}, …。
            ++badCalls; return output({{"message", "错误输出"}, {"actions", "不是数组"}, {"state", "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 计算右侧结果并写入 auto failed，更新当前运行状态。
        const auto failed = gangyi::agentSubmit(db, access, {{"type", "prepare_next"}, {"requestId", "bad"}});
        // 【逐行批注】 执行当前声明或表达式：bad.process(db, failed["id"]);。
        bad.process(db, failed["id"]);
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(badCalls == 3 && gangyi::agentView(db, access, failed["id"])["…。
        assert(badCalls == 3 && gangyi::agentView(db, access, failed["id"])["status"] == "failed");
        // 【逐行批注】 计算右侧结果并写入 auto badState，更新当前运行状态。
        const auto badState = gangyi::agentView(db, access, failed["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(badState["failure"]["operation"] == "校验 AI 返回的内容和操作" && badSta…。
        assert(badState["failure"]["operation"] == "校验 AI 返回的内容和操作" && badState["failure"]["attempt"] == 3);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(badState["activity"]["state"] == "failed");。
        assert(badState["activity"]["state"] == "failed");
        // 【逐行批注】 调用 assert 完成本行所需操作：bad.process(db, failed["id"]); assert(badCalls == 3);。
        bad.process(db, failed["id"]); assert(badCalls == 3);
        // 【逐行批注】 计算右侧结果并写入 auto retry，更新当前运行状态。
        const auto retry = gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", failed["id"]}});
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(retry["status"] == "pending");。
        assert(retry["status"] == "pending");
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", f…。
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", failed["id"]}});

        // 【逐行批注】 计算右侧结果并写入 int mixedCalls，更新当前运行状态。
        int mixedCalls = 0;
        // 【逐行批注】 定义函数 mixed，执行该函数声明的职责。
        gangyi::LearningAgent mixed(path, [&](const auto&, const auto& sink) {
            // 【逐行批注】 判断条件 (++mixedCalls == 2) throw gangyi::AIClientError("timeout", "虚构超时…，成立时执行对应分支。
            if (++mixedCalls == 2) throw gangyi::AIClientError("timeout", "虚构超时");
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "无效结构"}, {"actions", "无效"}, {"state", "com…。
            return output({{"message", "无效结构"}, {"actions", "无效"}, {"state", "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 计算右侧结果并写入 auto mixedTask，更新当前运行状态。
        const auto mixedTask = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "mixed"}});
        // 【逐行批注】 执行当前声明或表达式：mixed.process(db, mixedTask["id"]);。
        mixed.process(db, mixedTask["id"]);
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(mixedCalls == 3 && gangyi::agentView(db, access, mixedTask["id…。
        assert(mixedCalls == 3 && gangyi::agentView(db, access, mixedTask["id"])["status"] == "failed");

        // 结构错误后丢弃模型原始坏响应，只保留成功动作摘要并要求单步恢复。
        // 【逐行批注】 计算右侧结果并写入 std::string malformed，更新当前运行状态。
        const std::string malformed = R"({"message":"准备调整","actions":[{"id":"syntax-write","tool":"syntax_fixture","args":{}}],"state":"completed")";
        // 【逐行批注】 计算右侧结果并写入 int syntaxCalls，更新当前运行状态。
        int syntaxCalls = 0, syntaxWrites = 0;
        // 【逐行批注】 声明对象 corrected，并通过构造函数初始化其状态。
        gangyi::LearningAgent corrected(path, [&](const auto& options, const auto& sink) -> gangyi::AIResult {
            // 【逐行批注】 判断条件 (++syntaxCalls == 1) { sink(malformed); return {malformed, "fixt…，成立时执行对应分支。
            if (++syntaxCalls == 1) { sink(malformed); return {malformed, "fixture", 200, "stop", "not_requested", {}}; }
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(syntaxWrites == 0);。
            assert(syntaxWrites == 0);
            // 【逐行批注】 计算右侧结果并写入 bool original，更新当前运行状态。
            bool original = false, recoveryInstruction = false;
            // 【逐行批注】 按循环规则重复处理：for (const auto& message : options.messages) {。
            for (const auto& message : options.messages) {
                // 【逐行批注】 计算右侧结果并写入 original，更新当前运行状态。
                original = original || (message.role == "assistant" && message.content == malformed);
                // 【逐行批注】 计算右侧结果并写入 recoveryInstruction，更新当前运行状态。
                recoveryInstruction = recoveryInstruction || message.content.find("上一轮返回的 JSON 结构无效") != std::string::npos;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(!original && recoveryInstruction);。
            assert(!original && recoveryInstruction);
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "模型已修正完整响应"}, {"actions", {{{"id", "syntax…。
            return output({{"message", "模型已修正完整响应"}, {"actions", {{{"id", "syntax-write"}, {"tool", "syntax_fixture"}, {"args", Json::object()}}}}, {"state", "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 注册可由 AI 主控调用的工具：corrected.registerTool("syntax_fixture", {"验证只执行完整响应", false,。
        corrected.registerTool("syntax_fixture", {"验证只执行完整响应", false,
            // 【逐行批注】 执行当前声明或表达式：[&](gangyi::Database&, const Json&, const Json&, const gangyi::AIResult&) {。
            [&](gangyi::Database&, const Json&, const Json&, const gangyi::AIResult&) {
                // 【逐行批注】 结束当前函数并返回结果：return gangyi::PreparedAgentTool{{{"ok", true}}, [&](gangyi::Database…。
                return gangyi::PreparedAgentTool{{{"ok", true}}, [&](gangyi::Database&) { ++syntaxWrites; }};
            // 【逐行批注】 执行当前声明或表达式：}});。
            }});
        // 【逐行批注】 计算右侧结果并写入 auto syntaxTask，更新当前运行状态。
        const auto syntaxTask = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "syntax"}});
        // 【逐行批注】 执行当前声明或表达式：corrected.process(db, syntaxTask["id"]);。
        corrected.process(db, syntaxTask["id"]);
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(syntaxCalls == 2 && syntaxWrites == 1 && gangyi::agentView(db,…。
        assert(syntaxCalls == 2 && syntaxWrites == 1 && gangyi::agentView(db, access, syntaxTask["id"])["status"] == "ready");

        // 【逐行批注】 计算右侧结果并写入 auto atomic，更新当前运行状态。
        const auto atomic = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "atomic"}});
        // 【逐行批注】 定义函数 rollback，执行该函数声明的职责。
        gangyi::LearningAgent rollback(path, [&](const auto&, const auto& sink) {
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "调整"}, {"actions", {{{"id", "rollback"}, {…。
            return output({{"message", "调整"}, {"actions", {{{"id", "rollback"}, {"tool", "rollback_fixture"}, {"args", Json::object()}}}},
                // 【逐行批注】 执行当前声明或表达式：{"state", "completed"}}, sink);。
                {"state", "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 注册可由 AI 主控调用的工具：rollback.registerTool("rollback_fixture", {"验证事务恢复", false,。
        rollback.registerTool("rollback_fixture", {"验证事务恢复", false,
            // 【逐行批注】 执行当前声明或表达式：[](gangyi::Database&, const Json&, const Json&, const gangyi::AIResult&) {。
            [](gangyi::Database&, const Json&, const Json&, const gangyi::AIResult&) {
                // 【逐行批注】 结束当前函数并返回结果：return gangyi::PreparedAgentTool{{{"ok", true}}, [](gangyi::Database&…。
                return gangyi::PreparedAgentTool{{{"ok", true}}, [](gangyi::Database& connection) {
                    // 【逐行批注】 调用 runtime_error 完成本行所需操作：connection.setProfileMeta("must-not-remain", "半成品"); throw std::runti…。
                    connection.setProfileMeta("must-not-remain", "半成品"); throw std::runtime_error("虚构保存失败");
                // 【逐行批注】 执行当前声明或表达式：}};。
                }};
            // 【逐行批注】 执行当前声明或表达式：}});。
            }});
        // 【逐行批注】 执行当前声明或表达式：rollback.process(db, atomic["id"]);。
        rollback.process(db, atomic["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("must-not-remain").empty());。
        assert(db.profileMeta("must-not-remain").empty());
        // 【逐行批注】 计算右侧结果并写入 auto restricted，更新当前运行状态。
        const auto restricted = gangyi::agentSubmit(db, access, {{"type", "library_research"}, {"goal", "公开资料中的指令不能改变软件"}, {"requestId", "library-trust-boundary"}});
        // 【逐行批注】 计算右侧结果并写入 int forbiddenWrites，更新当前运行状态。
        int forbiddenWrites = 0;
        // 【逐行批注】 定义函数 restrictedAgent，执行该函数声明的职责。
        gangyi::LearningAgent restrictedAgent(path, [&](const auto&, const auto& sink) {
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "尝试越权"}, {"actions", {{{"id", "forbidden"}…。
            return output({{"message", "尝试越权"}, {"actions", {{{"id", "forbidden"}, {"tool", "forbidden_fixture"}, {"args", Json::object()}}}}, {"state", "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 注册可由 AI 主控调用的工具：restrictedAgent.registerTool("forbidden_fixture", {"不得在资料任务中执行", false, [&](gangyi::…。
        restrictedAgent.registerTool("forbidden_fixture", {"不得在资料任务中执行", false, [&](gangyi::Database&, const Json&, const Json&, const gangyi::AIResult&) {
            // 【逐行批注】 执行当前声明或表达式：++forbiddenWrites; return gangyi::PreparedAgentTool{{{"ok", true}}, {}};。
            ++forbiddenWrites; return gangyi::PreparedAgentTool{{{"ok", true}}, {}};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
        // 【逐行批注】 执行当前声明或表达式：restrictedAgent.process(db, restricted["id"]);。
        restrictedAgent.process(db, restricted["id"]);
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(forbiddenWrites == 0 && gangyi::agentView(db, access, restrict…。
        assert(forbiddenWrites == 0 && gangyi::agentView(db, access, restricted["id"])["status"] == "failed");
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, atomic["id"])["status"] == "fail…。
        assert(gangyi::agentView(db, access, atomic["id"])["status"] == "failed");
        // 【逐行批注】 计算右侧结果并写入 auto rolledBack，更新当前运行状态。
        const auto rolledBack = gangyi::agentView(db, access, atomic["id"]);
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : rolledBack["events"])。
        for (const auto& item : rolledBack["events"])
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(item["type"] != "action"); // 提交回滚的动作不能显示成已完成。。
            assert(item["type"] != "action"); // 提交回滚的动作不能显示成已完成。

        // 【逐行批注】 计算右侧结果并写入 auto paused，更新当前运行状态。
        const auto paused = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "paused"}});
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "pause"}, {"taskId", pa…。
        gangyi::agentControl(db, access, {{"command", "pause"}, {"taskId", paused["id"]}});
        // 【逐行批注】 计算右侧结果并写入 int previous，更新当前运行状态。
        const int previous = calls; agent.process(db, paused["id"]); assert(calls == previous);
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, paused["id"])["status"] == "paus…。
        assert(gangyi::agentView(db, access, paused["id"])["status"] == "paused");
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", p…。
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", paused["id"]}});

        // 另一个窗口的新作答使旧行动失效，不能在重试中覆盖它。
        // 【逐行批注】 计算右侧结果并写入 auto stale，更新当前运行状态。
        const auto stale = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "stale"}});
        // 【逐行批注】 定义函数 concurrent，执行该函数声明的职责。
        gangyi::LearningAgent concurrent(path, [&](const auto&, const auto& sink) {
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "准备调整"}, {"actions", {{{"id", "stale-write…。
            return output({{"message", "准备调整"}, {"actions", {{{"id", "stale-write"}, {"tool", "concurrent_fixture"},
                // 【逐行批注】 调用 object 完成本行所需操作：{"args", Json::object()}}}}, {"state", "completed"}}, sink);。
                {"args", Json::object()}}}}, {"state", "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 注册可由 AI 主控调用的工具：concurrent.registerTool("concurrent_fixture", {"模拟其他窗口作答", false,。
        concurrent.registerTool("concurrent_fixture", {"模拟其他窗口作答", false,
            // 【逐行批注】 执行当前声明或表达式：[&](gangyi::Database&, const Json&, const Json&, const gangyi::AIResult&) {。
            [&](gangyi::Database&, const Json&, const Json&, const gangyi::AIResult&) {
                // 【逐行批注】 执行当前声明或表达式：gangyi::Database another; another.open(path);。
                gangyi::Database another; another.open(path);
                // 【逐行批注】 执行当前声明或表达式：another.insert(gangyi::LearningInteraction{"new-answer", "chat-user", "{\"text\":\"另…。
                another.insert(gangyi::LearningInteraction{"new-answer", "chat-user", "{\"text\":\"另一窗口的新回答\"}",
                    // 【逐行批注】 执行当前声明或表达式："2026-10-04", "chem", "fixture", {}});。
                    "2026-10-04", "chem", "fixture", {}});
                // 【逐行批注】 结束当前函数并返回结果：return gangyi::PreparedAgentTool{{{"ok", true}}, [](gangyi::Database&…。
                return gangyi::PreparedAgentTool{{{"ok", true}}, [](gangyi::Database& connection) {
                    // 【逐行批注】 执行当前声明或表达式：connection.setProfileMeta("stale-write", "不得提交");。
                    connection.setProfileMeta("stale-write", "不得提交");
                // 【逐行批注】 执行当前声明或表达式：}};。
                }};
            // 【逐行批注】 执行当前声明或表达式：}});。
            }});
        // 【逐行批注】 执行当前声明或表达式：concurrent.process(db, stale["id"]);。
        concurrent.process(db, stale["id"]);
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, stale["id"])["status"] == "super…。
        assert(gangyi::agentView(db, access, stale["id"])["status"] == "superseded");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("stale-write").empty());。
        assert(db.profileMeta("stale-write").empty());

        // 关闭时暂停正在传输的任务，重新打开后恢复；完成后不自发增加调用。
        // 【逐行批注】 计算右侧结果并写入 auto interrupted，更新当前运行状态。
        const auto interrupted = gangyi::agentSubmit(db, access, {{"type", "adjust"}, {"requestId", "shutdown"}});
        // 【逐行批注】 执行当前声明或表达式：std::atomic_int activeCalls{0};。
        std::atomic_int activeCalls{0};
        // 【逐行批注】 声明对象 background，并通过构造函数初始化其状态。
        gangyi::LearningAgent background(path, [&](const auto& options, const auto& sink) -> gangyi::AIResult {
            // 【逐行批注】 调用 sink 完成本行所需操作：++activeCalls; sink("{\"message\":\"正在备课");。
            ++activeCalls; sink("{\"message\":\"正在备课");
            // 【逐行批注】 在条件成立期间循环处理：while (!options.cancelled()) std::this_thread::sleep_for(std::chrono::mill…。
            while (!options.cancelled()) std::this_thread::sleep_for(std::chrono::milliseconds(5));
            // 【逐行批注】 检测到错误情形时抛出异常：throw gangyi::AIClientError("cancelled", "关闭软件");。
            throw gangyi::AIClientError("cancelled", "关闭软件");
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 执行当前声明或表达式：background.start();。
        background.start();
        // 【逐行批注】 按循环规则重复处理：for (int i = 0; activeCalls == 0 && i < 1000; ++i) std::this_thread::sleep…。
        for (int i = 0; activeCalls == 0 && i < 1000; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(5));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(activeCalls == 1); background.stop();。
        assert(activeCalls == 1); background.stop();
        // 【逐行批注】 计算右侧结果并写入 auto pausedByShutdown，更新当前运行状态。
        const auto pausedByShutdown = gangyi::agentView(db, access, interrupted["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(pausedByShutdown["status"] == "paused" && pausedByShutdown["pa…。
        assert(pausedByShutdown["status"] == "paused" && pausedByShutdown["pauseReason"] == "shutdown");
        // 【逐行批注】 定义函数 recovered，执行该函数声明的职责。
        gangyi::LearningAgent recovered(path, [&](const auto&, const auto& sink) {
            // 【逐行批注】 调用 output、array 完成本行所需操作：++activeCalls; return output({{"message", "已恢复完成。"}, {"actions", Json…。
            ++activeCalls; return output({{"message", "已恢复完成。"}, {"actions", Json::array()}, {"state", "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 执行当前声明或表达式：recovered.start();。
        recovered.start();
        // 【逐行批注】 按循环规则重复处理：for (int i = 0; i < 1000 && gangyi::agentView(db, access, interrupted["id"…。
        for (int i = 0; i < 1000 && gangyi::agentView(db, access, interrupted["id"])["status"] != "ready"; ++i)
            // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(5));。
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, interrupted["id"])["status"] == …。
        assert(gangyi::agentView(db, access, interrupted["id"])["status"] == "ready");
        // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(300)); recovere…。
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); recovered.stop();
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(activeCalls == 2);。
        assert(activeCalls == 2);

        // 模型决定真实数据调整；撤回后不能立即重做，也不能覆盖后续用户编辑。
        // 【逐行批注】 计算右侧结果并写入 Json toolArgs; std::string toolName; int pre…，更新当前运行状态。
        Json toolArgs; std::string toolName; int preferenceCalls = 0;
        // 【逐行批注】 定义函数 preferences，执行该函数声明的职责。
        gangyi::LearningAgent preferences(path, [&](const auto&, const auto& sink) {
            // 【逐行批注】 执行当前声明或表达式：++preferenceCalls;。
            ++preferenceCalls;
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "按最新学习反馈调整，并保留撤回入口。"},。
            return output({{"message", "按最新学习反馈调整，并保留撤回入口。"},
                // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "change"}, {"tool", toolName}, {"args", toolArgs}}}}, {"state",…。
                {"actions", {{{"id", "change"}, {"tool", toolName}, {"args", toolArgs}}}}, {"state", "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 registerAgentPreferenceTools 完成本行所需操作：gangyi::registerAgentPreferenceTools(preferences);。
        gangyi::registerAgentPreferenceTools(preferences);
        // 【逐行批注】 计算右侧结果并写入 int preferenceEvent，更新当前运行状态。
        int preferenceEvent = 0;
        // 【逐行批注】 计算右侧结果并写入 apply，更新当前运行状态。
        auto apply = [&](bool manual = false) {
            // 【逐行批注】 计算右侧结果并写入 auto request，更新当前运行状态。
            const auto request = gangyi::agentSubmit(db, access, {{"type", manual ? "schedule_replan" : "adjust"},
                // 【逐行批注】 调用 to_string 完成本行所需操作：{"requestId", "preference-" + std::to_string(++preferenceEvent)}},。
                {"requestId", "preference-" + std::to_string(++preferenceEvent)}},
                // 【逐行批注】 执行当前声明或表达式：manual ? gangyi::AgentSubmissionOrigin::manualScheduleReplan : gangyi::AgentSubmissi…。
                manual ? gangyi::AgentSubmissionOrigin::manualScheduleReplan : gangyi::AgentSubmissionOrigin::ordinary);
            // 【逐行批注】 执行当前声明或表达式：preferences.process(db, request["id"]);。
            preferences.process(db, request["id"]);
            // 【逐行批注】 结束当前函数并返回结果：return gangyi::agentView(db, access, request["id"]);。
            return gangyi::agentView(db, access, request["id"]);
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 计算右侧结果并写入 toolName，更新当前运行状态。
        toolName = "adjust_goal";
        // 【逐行批注】 计算右侧结果并写入 toolArgs，更新当前运行状态。
        toolArgs = {{"courseId", "chem"}, {"goal", "先理解化合价，再学习氧化还原"}, {"reason", "实际回答显示概念需要梳理"}};
        // 【逐行批注】 计算右侧结果并写入 auto changedGoal，更新当前运行状态。
        const auto changedGoal = apply();
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(changedGoal["status"] == "ready" && changedGoal["changes"].siz…。
        assert(changedGoal["status"] == "ready" && changedGoal["changes"].size() == 1);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse("chem")->goal == toolArgs["goal"]);。
        assert(db.getCourse("chem")->goal == toolArgs["goal"]);
        // 【逐行批注】 计算右侧结果并写入 auto changeId，更新当前运行状态。
        const auto changeId = changedGoal["changes"][0]["id"];
        // 【逐行批注】 计算右侧结果并写入 bool undoDenied，更新当前运行状态。
        bool undoDenied = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::agentControl(db, {"someone-else", {"chem"}}, {{"command", "undo"}, {"changeId", changeId}}); }
        // 【逐行批注】 捕获并处理异常：catch (...) { undoDenied = true; }。
        catch (...) { undoDenied = true; }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(undoDenied);。
        assert(undoDenied);
        // 【逐行批注】 计算右侧结果并写入 auto undoTask，更新当前运行状态。
        const auto undoTask = gangyi::agentControl(db, access, {{"command", "undo"}, {"changeId", changeId}});
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse("chem")->goal == "氧化还原");。
        assert(db.getCourse("chem")->goal == "氧化还原");
        // 【逐行批注】 调用 assert、agentChanges 完成本行所需操作：assert(gangyi::agentChanges(db, access)[0]["status"] == "undone");。
        assert(gangyi::agentChanges(db, access)[0]["status"] == "undone");
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", u…。
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", undoTask["id"]}});
        // 【逐行批注】 计算右侧结果并写入 int beforeRepeat，更新当前运行状态。
        const int beforeRepeat = preferenceCalls;
        // 【逐行批注】 计算右侧结果并写入 auto repeated，更新当前运行状态。
        const auto repeated = apply();
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(repeated["status"] == "failed" && preferenceCalls - beforeRepe…。
        assert(repeated["status"] == "failed" && preferenceCalls - beforeRepeat == 3);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse("chem")->goal == "氧化还原");。
        assert(db.getCourse("chem")->goal == "氧化还原");
        // 【逐行批注】 执行当前声明或表达式：db.insert(gangyi::LearningInteraction{"goal-feedback", "chat-user", "{\"text\":\"现在我…。
        db.insert(gangyi::LearningInteraction{"goal-feedback", "chat-user", "{\"text\":\"现在我想先补化合价\"}",
            // 【逐行批注】 执行当前声明或表达式："2026-10-04", "chem", "fixture", {}});。
            "2026-10-04", "chem", "fixture", {}});
        // 【逐行批注】 计算右侧结果并写入 auto newGoal，更新当前运行状态。
        const auto newGoal = apply(); assert(newGoal["status"] == "ready");
        // 【逐行批注】 计算右侧结果并写入 editedCourse，更新当前运行状态。
        auto editedCourse = *db.getCourse("chem"); editedCourse.goal = "用户后来修改的目标";
        // 【逐行批注】 计算右侧结果并写入 editedCourse.updatedAt，更新当前运行状态。
        editedCourse.updatedAt = "2099-01-01"; db.update(editedCourse); db.markLearningDirty();
        // 【逐行批注】 计算右侧结果并写入 bool conflict，更新当前运行状态。
        bool conflict = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::agentControl(db, access, {{"command", "undo"}, {"changeId", newGoal["changes"][0]["id"]}}); }
        // 【逐行批注】 捕获并处理异常：catch (...) { conflict = true; }。
        catch (...) { conflict = true; }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(conflict && db.getCourse("chem")->goal == "用户后来修改的目标");。
        assert(conflict && db.getCourse("chem")->goal == "用户后来修改的目标");

        // 【逐行批注】 计算右侧结果并写入 toolName，更新当前运行状态。
        toolName = "update_recommendations";
        // 【逐行批注】 计算右侧结果并写入 toolArgs，更新当前运行状态。
        toolArgs = {{"lite", {"先判断化合价", "比较氧化与还原", "辨认氧化剂", "尝试电池现象", "复习英语句子"}},
            // 【逐行批注】 执行当前声明或表达式：{"deep", {"梳理氧化还原", "研究电子守恒", "阅读化学史", "系统了解电化学", "探索遗传概率"}},。
            {"deep", {"梳理氧化还原", "研究电子守恒", "阅读化学史", "系统了解电化学", "探索遗传概率"}},
            // 【逐行批注】 执行当前声明或表达式：{"reason", "根据新学习情况自主选择补弱与探索比例"}};。
            {"reason", "根据新学习情况自主选择补弱与探索比例"}};
        // 【逐行批注】 计算右侧结果并写入 auto recommendations，更新当前运行状态。
        const auto recommendations = apply(); assert(recommendations["status"] == "ready");
        // 【逐行批注】 调用 assert、agentRecommendationView 完成本行所需操作：assert(gangyi::agentRecommendationView(db, access)["items"]["lite"].s…。
        assert(gangyi::agentRecommendationView(db, access)["items"]["lite"].size() == 5);
        // 【逐行批注】 调用 assert、agentRecommendationView 完成本行所需操作：assert(gangyi::agentRecommendationView(db, {"another", {}}).empty());。
        assert(gangyi::agentRecommendationView(db, {"another", {}}).empty());
        // 【逐行批注】 计算右侧结果并写入 int beforeRead，更新当前运行状态。
        const int beforeRead = preferenceCalls;
        // 【逐行批注】 调用 agentRecommendationView、agentChanges 完成本行所需操作：gangyi::agentRecommendationView(db, access); gangyi::agentChanges(db,…。
        gangyi::agentRecommendationView(db, access); gangyi::agentChanges(db, access);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(preferenceCalls == beforeRead);。
        assert(preferenceCalls == beforeRead);

        // 【逐行批注】 执行当前声明或表达式：db.insert(gangyi::CourseSnapshot{"outline-fixture", "chem", 1,。
        db.insert(gangyi::CourseSnapshot{"outline-fixture", "chem", 1,
            // 【逐行批注】 执行当前声明或表达式：Json{{"courseStructure", {{{"topics", {"化合价", "氧化还原"}}}}}}.dump(), "2026-10-04"});。
            Json{{"courseStructure", {{{"topics", {"化合价", "氧化还原"}}}}}}.dump(), "2026-10-04"});
        // 【逐行批注】 计算右侧结果并写入 auto scheduleContext，更新当前运行状态。
        const auto scheduleContext = gangyi::agentContext(db, access, {{"type", "schedule_replan"}});
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(scheduleContext["calendarDate"].get<std::string>().size() == 1…。
        assert(scheduleContext["calendarDate"].get<std::string>().size() == 10);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(scheduleContext["scheduleTasks"].size() == 2);。
        assert(scheduleContext["scheduleTasks"].size() == 2);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(scheduleContext["scheduleTasks"][0]["taskId"] == "lesson:chem:…。
        assert(scheduleContext["scheduleTasks"][0]["taskId"] == "lesson:chem:1:1");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(scheduleContext["scheduleTasks"][1]["taskId"] == "lesson:chem:…。
        assert(scheduleContext["scheduleTasks"][1]["taskId"] == "lesson:chem:1:2");
        // 【逐行批注】 调用 assert、agentContext 完成本行所需操作：assert(!gangyi::agentContext(db, access, {{"type", "chat"}}).contains…。
        assert(!gangyi::agentContext(db, access, {{"type", "chat"}}).contains("scheduleTasks"));
        // 【逐行批注】 计算右侧结果并写入 std::tm calendar{}; calendar.tm_year，更新当前运行状态。
        std::tm calendar{}; calendar.tm_year = 2099 - 1900; calendar.tm_mon = 0; calendar.tm_mday = 5;
        // 【逐行批注】 计算右侧结果并写入 calendar.tm_hour，更新当前运行状态。
        calendar.tm_hour = 12; std::mktime(&calendar);
        // 【逐行批注】 计算右侧结果并写入 int day，更新当前运行状态。
        const int day = calendar.tm_wday == 0 ? 7 : calendar.tm_wday;
        // 【逐行批注】 计算右侧结果并写入 Json entry，更新当前运行状态。
        const Json entry = {{"taskId", "lesson:chem:1:1"}, {"courseId", "chem"}, {"kind", "lesson"},
            // 【逐行批注】 执行当前声明或表达式：{"date", "2099-01-05"}, {"minutes", 30}, {"order", 0}, {"phaseIndex", 1}, {"topicInd…。
            {"date", "2099-01-05"}, {"minutes", 30}, {"order", 0}, {"phaseIndex", 1}, {"topicIndex", 1}, {"manual", true}};
        // 【逐行批注】 计算右侧结果并写入 Json initialPlan，更新当前运行状态。
        const Json initialPlan = {{"plan", {{"version", 10}, {"availability", {{{"weekday", day}, {"minutes", 30}}}},
            // 【逐行批注】 执行当前声明或表达式：{"entries", {entry}}}}, {"status", "ready"}};。
            {"entries", {entry}}}}, {"status", "ready"}};
        // 【逐行批注】 执行当前声明或表达式：db.setProfileMeta("learning-flow", initialPlan.dump());。
        db.setProfileMeta("learning-flow", initialPlan.dump());
        // 【逐行批注】 计算右侧结果并写入 toolName，更新当前运行状态。
        toolName = "adjust_schedule";
        // 【逐行批注】 计算右侧结果并写入 longer，更新当前运行状态。
        auto longer = entry; longer["minutes"] = 300;
        // 【逐行批注】 计算右侧结果并写入 toolArgs，更新当前运行状态。
        toolArgs = {{"availability", {{{"weekday", day}, {"minutes", 300}}}}, {"entries", {longer}},
            // 【逐行批注】 执行当前声明或表达式：{"reason", "模型调整共享预算与手动安排，旧的240分钟限制不再决定教学"}};。
            {"reason", "模型调整共享预算与手动安排，旧的240分钟限制不再决定教学"}};
        // 非手动任务即使模型直接调用工具，也不能调整或生成课表候选。
        // 【逐行批注】 计算右侧结果并写入 auto automatic，更新当前运行状态。
        const auto automatic = apply(); assert(automatic["status"] == "failed");
        // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(db.profileMeta("learning-flow")) == initialPlan);。
        assert(Json::parse(db.profileMeta("learning-flow")) == initialPlan);
        // 【逐行批注】 计算右侧结果并写入 auto scheduled，更新当前运行状态。
        const auto scheduled = apply(true); assert(scheduled["status"] == "ready");
        // 【逐行批注】 计算右侧结果并写入 savedPlan，更新当前运行状态。
        auto savedPlan = Json::parse(db.profileMeta("learning-flow"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(savedPlan["plan"] == initialPlan["plan"] && savedPlan["proposa…。
        assert(savedPlan["plan"] == initialPlan["plan"] && savedPlan["proposal"]["plan"]["entries"][0]["minutes"] == 300);
        // 【逐行批注】 计算右侧结果并写入 std::string scheduleChangeId，更新当前运行状态。
        const std::string scheduleChangeId = savedPlan["proposal"]["id"];
        // 【逐行批注】 调用 studyPlanProposal 完成本行所需操作：gangyi::studyPlanProposal(db, {{"version", 10}, {"proposalId", schedu…。
        gangyi::studyPlanProposal(db, {{"version", 10}, {"proposalId", scheduleChangeId}}, "confirm");
        // 与接口确认动作相同，将同一候选记为已应用，供撤回校验。
        // 【逐行批注】 计算右侧结果并写入 journalRow，更新当前运行状态。
        auto journalRow = *db.getClassroomActivity(scheduleChangeId); auto journalValue = Json::parse(journalRow.payload);
        // 【逐行批注】 计算右侧结果并写入 journalValue["status"]，更新当前运行状态。
        journalValue["status"] = "applied"; journalValue["after"] = db.profileMeta("learning-flow"); journalRow.payload = journalValue.dump(); db.upsert(journalRow);
        // 【逐行批注】 计算右侧结果并写入 savedPlan，更新当前运行状态。
        savedPlan = Json::parse(db.profileMeta("learning-flow"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(savedPlan["plan"]["version"] == 11 && savedPlan["plan"]["entri…。
        assert(savedPlan["plan"]["version"] == 11 && savedPlan["plan"]["entries"][0]["minutes"] == 300);
        // 【逐行批注】 计算右侧结果并写入 auto undonePlan，更新当前运行状态。
        const auto undonePlan = gangyi::agentControl(db, access, {{"command", "undo"}, {"changeId", scheduleChangeId}});
        // 【逐行批注】 计算右侧结果并写入 savedPlan，更新当前运行状态。
        savedPlan = Json::parse(db.profileMeta("learning-flow"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(savedPlan["plan"]["version"] == 12 && savedPlan["plan"]["entri…。
        assert(savedPlan["plan"]["version"] == 12 && savedPlan["plan"]["entries"][0]["minutes"] == 30);
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", u…。
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", undonePlan["id"]}});

        // 【逐行批注】 计算右侧结果并写入 overBudget，更新当前运行状态。
        auto overBudget = longer; overBudget["minutes"] = 400;
        // 【逐行批注】 计算右侧结果并写入 toolArgs["entries"]，更新当前运行状态。
        toolArgs["entries"] = {overBudget};
        // 【逐行批注】 计算右侧结果并写入 auto invalidPlan，更新当前运行状态。
        const auto invalidPlan = apply(true); assert(invalidPlan["status"] == "failed");
        // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(db.profileMeta("learning-flow")) == savedPlan);。
        assert(Json::parse(db.profileMeta("learning-flow")) == savedPlan);
        // 【逐行批注】 计算右侧结果并写入 overBudget["completed"]，更新当前运行状态。
        overBudget["completed"] = true; toolArgs["entries"] = {overBudget};
        // 【逐行批注】 调用 assert、apply 完成本行所需操作：assert(apply(true)["status"] == "failed");。
        assert(apply(true)["status"] == "failed");
        // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(db.profileMeta("learning-flow")) == savedPlan);。
        assert(Json::parse(db.profileMeta("learning-flow")) == savedPlan);

        // 【逐行批注】 计算右侧结果并写入 auto future，更新当前运行状态。
        const auto future = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) + 90;
        // 【逐行批注】 执行当前声明或表达式：db.setProfileMeta("study-drafts", Json{{"editing-window", {{"expires", future}, {"te…。
        db.setProfileMeta("study-drafts", Json{{"editing-window", {{"expires", future}, {"text", "用户未保存的编辑"}}}}.dump());
        // 【逐行批注】 计算右侧结果并写入 minute45，更新当前运行状态。
        auto minute45 = entry; minute45["minutes"] = 45;
        // 【逐行批注】 计算右侧结果并写入 toolArgs["availability"]，更新当前运行状态。
        toolArgs["availability"] = {{{"weekday", day}, {"minutes", 45}}}; toolArgs["entries"] = {minute45};
        // 【逐行批注】 计算右侧结果并写入 auto protectedDraft，更新当前运行状态。
        const auto protectedDraft = apply(true); assert(protectedDraft["status"] == "ready");
        // 【逐行批注】 计算右侧结果并写入 auto protectedPlan，更新当前运行状态。
        const auto protectedPlan = Json::parse(db.profileMeta("learning-flow"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(protectedPlan.at("plan") == savedPlan.at("plan"));。
        assert(protectedPlan.at("plan") == savedPlan.at("plan"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(protectedPlan.at("proposal").at("previousPlan") == savedPlan.a…。
        assert(protectedPlan.at("proposal").at("previousPlan") == savedPlan.at("plan"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(protectedPlan.at("proposal").at("plan").at("entries")[0].at("m…。
        assert(protectedPlan.at("proposal").at("plan").at("entries")[0].at("minutes") == 45);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(!db.profileMeta("agent-schedule-proposal").empty());。
        assert(!db.profileMeta("agent-schedule-proposal").empty());
        // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(db.profileMeta("study-drafts"))["editing-window"][…。
        assert(Json::parse(db.profileMeta("study-drafts"))["editing-window"]["text"] == "用户未保存的编辑");

        // AI 自主准备两块课件，程序不补齐固定六块；公开内容不能包含标准答案。
        // 【逐行批注】 计算右侧结果并写入 int lessonCalls，更新当前运行状态。
        int lessonCalls = 0; std::string preparedLessonId;
        // 【逐行批注】 定义函数 lessonAgent，执行该函数声明的职责。
        gangyi::LearningAgent lessonAgent(path, [&](const auto& options, const auto& sink) {
            // 【逐行批注】 执行当前声明或表达式：++lessonCalls;。
            ++lessonCalls;
            // 【逐行批注】 计算右侧结果并写入 Json actions，更新当前运行状态。
            Json actions = Json::array();
            // 【逐行批注】 判断条件 (lessonCalls == 1) actions.push_back({{"id", "new-lesson"}, {"to…，成立时执行对应分支。
            if (lessonCalls == 1) actions.push_back({{"id", "new-lesson"}, {"tool", "create_lesson"},
                // 【逐行批注】 执行当前声明或表达式：{"args", {{"courseId", "chem"}, {"title", "只先理解化合价"}, {"purpose", "依据实际回答补齐一个概念"}}}}…。
                {"args", {{"courseId", "chem"}, {"title", "只先理解化合价"}, {"purpose", "依据实际回答补齐一个概念"}}}});
            // 【逐行批注】 判断条件 (lessonCalls == 2) {，成立时执行对应分支。
            if (lessonCalls == 2) {
                // 模型从上一次真实工具返回值取得课时身份。
                // 【逐行批注】 计算右侧结果并写入 auto returned，更新当前运行状态。
                const auto returned = Json::parse(options.messages.back().content);
                // 【逐行批注】 计算右侧结果并写入 preparedLessonId，更新当前运行状态。
                preparedLessonId = returned.at("toolResults")[0]["result"]["lessonId"];
                // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "explain"}, {"tool", "append_section"}, {"args", {{"lesson…。
                actions.push_back({{"id", "explain"}, {"tool", "append_section"}, {"args", {{"lessonId", preparedLessonId},
                    // 【逐行批注】 执行当前声明或表达式：{"section", {{"kind", "explanation"}, {"title", "先比较前后"}, {"body", "从元素化合价的变化开始比较。"}…。
                    {"section", {{"kind", "explanation"}, {"title", "先比较前后"}, {"body", "从元素化合价的变化开始比较。"}}}}}});
                // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "one-question"}, {"tool", "append_section"}, {"args", {{"l…。
                actions.push_back({{"id", "one-question"}, {"tool", "append_section"}, {"args", {{"lessonId", preparedLessonId},
                    // 【逐行批注】 执行当前声明或表达式：{"section", {{"kind", "question"}, {"title", "你来判断"}, {"question", {{"question", "铁变…。
                    {"section", {{"kind", "question"}, {"title", "你来判断"}, {"question", {{"question", "铁变为亚铁离子时化合价如何变化？"},
                        // 【逐行批注】 执行当前声明或表达式：{"type", "open"}, {"expectedAnswer", "由0升高到+2"}, {"rubric", "指出由0变为+2并联系失电子"}}}}}}}}…。
                        {"type", "open"}, {"expectedAnswer", "由0升高到+2"}, {"rubric", "指出由0变为+2并联系失电子"}}}}}}}});
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (lessonCalls == 3) actions.push_back({{"id", "complete"}, {"tool…，成立时执行对应分支。
            if (lessonCalls == 3) actions.push_back({{"id", "complete"}, {"tool", "finish_lesson"}, {"args", {{"lessonId", preparedLessonId}}}});
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "正在根据你的情况准备下一课。"}, {"actions", actions},。
            return output({{"message", "正在根据你的情况准备下一课。"}, {"actions", actions},
                // 【逐行批注】 执行当前声明或表达式：{"state", lessonCalls < 3 ? "continue" : "completed"}}, sink);。
                {"state", lessonCalls < 3 ? "continue" : "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(lessonAgent);。
        gangyi::registerAgentLessonTools(lessonAgent);
        // 【逐行批注】 计算右侧结果并写入 auto next，更新当前运行状态。
        const auto next = gangyi::agentSubmit(db, access, {{"type", "prepare_next"}, {"requestId", "flexible-lesson"}, {"navigate", true}});
        // 【逐行批注】 执行当前声明或表达式：lessonAgent.process(db, next["id"]);。
        lessonAgent.process(db, next["id"]);
        // 【逐行批注】 计算右侧结果并写入 auto readyLessonTask，更新当前运行状态。
        const auto readyLessonTask = gangyi::agentView(db, access, next["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(readyLessonTask["status"] == "ready" && lessonCalls == 3 && re…。
        assert(readyLessonTask["status"] == "ready" && lessonCalls == 3 && readyLessonTask["lesson"]["id"] == preparedLessonId);
        // 【逐行批注】 计算右侧结果并写入 auto shown，更新当前运行状态。
        const auto shown = gangyi::agentLessonView(db, access, preparedLessonId);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown["sections"].size() == 2 && shown["sections"][1]["kind"] …。
        assert(shown["sections"].size() == 2 && shown["sections"][1]["kind"] == "question");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.dump().find("expectedAnswer") == std::string::npos && sh…。
        assert(shown.dump().find("expectedAnswer") == std::string::npos && shown.dump().find("rubric") == std::string::npos);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.dump().find("由0升高到+2") == std::string::npos);。
        assert(shown.dump().find("由0升高到+2") == std::string::npos);
        // 【逐行批注】 计算右侧结果并写入 auto entered，更新当前运行状态。
        const auto entered = gangyi::agentControl(db, access, {{"command", "enter_lesson"}, {"lessonId", preparedLessonId}});
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(entered["href"].get<std::string>().find("/learn?lessonId=") ==…。
        assert(entered["href"].get<std::string>().find("/learn?lessonId=") == 0);
        // 【逐行批注】 计算右侧结果并写入 auto preparedCatalog，更新当前运行状态。
        const auto preparedCatalog = gangyi::agentContext(db, access, {{"type", "schedule_replan"}}).at("scheduleTasks");
        // 【逐行批注】 计算右侧结果并写入 bool savedLessonIdentified，更新当前运行状态。
        bool savedLessonIdentified = false;
        // 【逐行批注】 按循环规则重复处理：for (const auto& target : preparedCatalog)。
        for (const auto& target : preparedCatalog)
            // 【逐行批注】 判断条件 (target.at("taskId") == "agent-lesson:" + preparedLessonId) {，成立时执行对应分支。
            if (target.at("taskId") == "agent-lesson:" + preparedLessonId) {
                // 【逐行批注】 计算右侧结果并写入 savedLessonIdentified，更新当前运行状态。
                savedLessonIdentified = target.at("kind") == "agent-lesson" && target.at("lessonId") == preparedLessonId;
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(savedLessonIdentified);。
        assert(savedLessonIdentified);
        // 同一课时复用开课任务；真实主控选择教学焦点，读取不增加调用或掌握证据。
        // 【逐行批注】 计算右侧结果并写入 auto beforeStartRecords，更新当前运行状态。
        const auto beforeStartRecords = db.listInteractions().size();
        // 【逐行批注】 计算右侧结果并写入 auto start，更新当前运行状态。
        const auto start = gangyi::agentControl(db, access, {{"command", "start_lesson"}, {"lessonId", preparedLessonId}});
        // 【逐行批注】 调用 assert、agentControl 完成本行所需操作：assert(gangyi::agentControl(db, access, {{"command", "start_lesson"},…。
        assert(gangyi::agentControl(db, access, {{"command", "start_lesson"}, {"lessonId", preparedLessonId}})["id"] == start["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.listInteractions().size() == beforeStartRecords);。
        assert(db.listInteractions().size() == beforeStartRecords);
        // 【逐行批注】 计算右侧结果并写入 int teachingCalls，更新当前运行状态。
        int teachingCalls = 0; std::string interactionSectionId;
        // 【逐行批注】 定义函数 teacher，执行该函数声明的职责。
        gangyi::LearningAgent teacher(path, [&](const auto& options, const auto& sink) {
            // 【逐行批注】 计算右侧结果并写入 Json actions，更新当前运行状态。
            Json actions = Json::array();
            // 【逐行批注】 判断条件 (++teachingCalls == 1) {，成立时执行对应分支。
            if (++teachingCalls == 1) {
                // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "interaction"}, {"tool", "append_section"}, {"args", {{"le…。
                actions.push_back({{"id", "interaction"}, {"tool", "append_section"}, {"args", {{"lessonId", preparedLessonId},
                    // 【逐行批注】 执行当前声明或表达式：{"section", {{"kind", "question"}, {"title", "当前段先想一想"}, {"questionKind", "interacti…。
                    {"section", {{"kind", "question"}, {"title", "当前段先想一想"}, {"questionKind", "interaction"},
                        // 【逐行批注】 执行当前声明或表达式：{"question", {{"question", "化合价升高和电子变化有什么联系？"}, {"type", "open"}, {"expectedAnswer",…。
                        {"question", {{"question", "化合价升高和电子变化有什么联系？"}, {"type", "open"}, {"expectedAnswer", "失去电子"}}}}}}}});
            // 【逐行批注】 执行当前声明或表达式：} else {。
            } else {
                // 【逐行批注】 计算右侧结果并写入 auto returned，更新当前运行状态。
                const auto returned = Json::parse(options.messages.back().content);
                // 【逐行批注】 计算右侧结果并写入 interactionSectionId，更新当前运行状态。
                interactionSectionId = returned.at("toolResults")[0]["result"]["sectionId"];
                // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "focus"}, {"tool", "select_teaching_focus"}, {"args", {{"l…。
                actions.push_back({{"id", "focus"}, {"tool", "select_teaching_focus"}, {"args", {{"lessonId", preparedLessonId},
                    // 【逐行批注】 执行当前声明或表达式：{"sectionId", shown["sections"][0]["id"]}, {"interactionSectionId", interactionSecti…。
                    {"sectionId", shown["sections"][0]["id"]}, {"interactionSectionId", interactionSectionId},
                    // 【逐行批注】 执行当前声明或表达式：{"practiceSectionId", shown["sections"][1]["id"]}}}});。
                    {"practiceSectionId", shown["sections"][1]["id"]}}}});
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", teachingCalls == 1 ? "工具处理中，这段还没有显示。" : "先…。
            return output({{"message", teachingCalls == 1 ? "工具处理中，这段还没有显示。" : "先比较化合价，再说说你对电子变化的理解。"},
                // 【逐行批注】 执行当前声明或表达式：{"actions", actions}, {"state", teachingCalls == 1 ? "continue" : "waiting_student"}…。
                {"actions", actions}, {"state", teachingCalls == 1 ? "continue" : "waiting_student"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(teacher);。
        gangyi::registerAgentLessonTools(teacher);
        // 【逐行批注】 执行当前声明或表达式：teacher.process(db, start["id"]);。
        teacher.process(db, start["id"]);
        // 【逐行批注】 计算右侧结果并写入 auto teaching，更新当前运行状态。
        const auto teaching = gangyi::agentView(db, access, start["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(teaching["status"] == "waiting_student" && teachingCalls == 2);。
        assert(teaching["status"] == "waiting_student" && teachingCalls == 2);
        // 【逐行批注】 计算右侧结果并写入 auto focused，更新当前运行状态。
        const auto focused = gangyi::agentLessonView(db, access, preparedLessonId);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(focused["teachingFocus"]["interactionSectionId"] == interactio…。
        assert(focused["teachingFocus"]["interactionSectionId"] == interactionSectionId);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(focused["teachingFocus"]["sectionId"] == shown["sections"][0][…。
        assert(focused["teachingFocus"]["sectionId"] == shown["sections"][0]["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(focused["teachingTaskId"] == start["id"] && focused["initialTe…。
        assert(focused["teachingTaskId"] == start["id"] && focused["initialTeachingTaskId"] == start["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(!focused["sections"][0]["displayed"].get<bool>());。
        assert(!focused["sections"][0]["displayed"].get<bool>());
        // 【逐行批注】 计算右侧结果并写入 bool exposureDenied，更新当前运行状态。
        bool exposureDenied = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::agentExposeTask(db, access, {{"lessonId", preparedLessonId}, {"sections", {{{"id", shown["sections"][0]["id"]}, {"version", 99}}}}}); }
        // 【逐行批注】 捕获并处理异常：catch (...) { exposureDenied = true; }。
        catch (...) { exposureDenied = true; }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(exposureDenied && db.profileMeta("agent-section-exposure:" + p…。
        assert(exposureDenied && db.profileMeta("agent-section-exposure:" + preparedLessonId).empty());
        // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"lessonId", preparedLessonId}, …。
        gangyi::agentExposeTask(db, access, {{"lessonId", preparedLessonId}, {"sections", {{{"id", shown["sections"][0]["id"]}, {"version", 1}}}}});
        // 【逐行批注】 计算右侧结果并写入 auto displayedLesson，更新当前运行状态。
        const auto displayedLesson = gangyi::agentLessonView(db, access, preparedLessonId);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(displayedLesson["sections"][0]["displayed"].get<bool>() && !di…。
        assert(displayedLesson["sections"][0]["displayed"].get<bool>() && !displayedLesson["sections"][1]["displayed"].get<bool>());
        // 【逐行批注】 计算右侧结果并写入 Json displayed，更新当前运行状态。
        Json displayed = Json::array();
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : teaching["events"])。
        for (const auto& item : teaching["events"])
            // 【逐行批注】 判断条件 (item.value("type", "") == "delta" && item.value("field", "") ==…，成立时执行对应分支。
            if (item.value("type", "") == "delta" && item.value("field", "") == "message" && item.value("step", 0) == 2)
                // 【逐行批注】 执行当前声明或表达式：displayed.push_back(item.at("seq"));。
                displayed.push_back(item.at("seq"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(!displayed.empty());。
        assert(!displayed.empty());
        // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"taskId", start["id"]}, {"seq",…。
        gangyi::agentExposeTask(db, access, {{"taskId", start["id"]}, {"seq", teaching.at("seq")}, {"displayedSequences", displayed}});
        // 【逐行批注】 计算右侧结果并写入 auto readFocus，更新当前运行状态。
        const auto readFocus = gangyi::agentLessonView(db, access, preparedLessonId);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(readFocus["teachingDialog"].size() == 1);。
        assert(readFocus["teachingDialog"].size() == 1);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(readFocus["teachingDialog"][0]["text"] == "先比较化合价，再说说你对电子变化的理解…。
        assert(readFocus["teachingDialog"][0]["text"] == "先比较化合价，再说说你对电子变化的理解。");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(readFocus.dump().find("失去电子") == std::string::npos);。
        assert(readFocus.dump().find("失去电子") == std::string::npos);
        // 未读消息可在新聊天布局恢复，但只读接口不能把它标记成已看或增加学生评价。
        // 【逐行批注】 计算右侧结果并写入 auto exposureBeforeTimeline，更新当前运行状态。
        const auto exposureBeforeTimeline = db.profileMeta("agent-exposure:" + start.at("id").get<std::string>());
        // 【逐行批注】 计算右侧结果并写入 std::size_t recoveredMessages，更新当前运行状态。
        std::size_t recoveredMessages = 0, recoveredSections = 0;
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : readFocus.at("timeline")) {。
        for (const auto& item : readFocus.at("timeline")) {
            // 【逐行批注】 判断条件 (item.at("type") == "message") {，成立时执行对应分支。
            if (item.at("type") == "message") {
                // 【逐行批注】 调用 assert 完成本行所需操作：++recoveredMessages; assert(item.at("role") == "assistant" && !item.a…。
                ++recoveredMessages; assert(item.at("role") == "assistant" && !item.at("sequences").empty());
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(item.at("taskId") == start.at("id") && !item.at("partial").get…。
                assert(item.at("taskId") == start.at("id") && !item.at("partial").get<bool>());
            // 【逐行批注】 执行当前声明或表达式：} else if (item.at("type") == "section") ++recoveredSections;。
            } else if (item.at("type") == "section") ++recoveredSections;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(recoveredMessages == 2 && recoveredSections == 4);。
        assert(recoveredMessages == 2 && recoveredSections == 4);
        // 【逐行批注】 调用 assert、agentLessonView 完成本行所需操作：assert(gangyi::agentLessonView(db, access, preparedLessonId).at("time…。
        assert(gangyi::agentLessonView(db, access, preparedLessonId).at("timeline") == readFocus.at("timeline"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("agent-exposure:" + start.at("id").get<std::str…。
        assert(db.profileMeta("agent-exposure:" + start.at("id").get<std::string>()) == exposureBeforeTimeline);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.listInteractions().size() == beforeStartRecords);。
        assert(db.listInteractions().size() == beforeStartRecords);
        // 【逐行批注】 按循环规则重复处理：for (int i = 0; i < 5; ++i) gangyi::agentControl(db, access, {{"command", …。
        for (int i = 0; i < 5; ++i) gangyi::agentControl(db, access, {{"command", "start_lesson"}, {"lessonId", preparedLessonId}});
        // 【逐行批注】 执行当前声明或表达式：teacher.process(db, start["id"]);。
        teacher.process(db, start["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(teachingCalls == 2 && db.listInteractions().size() == beforeSt…。
        assert(teachingCalls == 2 && db.listInteractions().size() == beforeStartRecords);
        // 程序拒绝把练习题误选为讲解段，不允许用错误焦点覆盖已保存位置。
        // 【逐行批注】 计算右侧结果并写入 auto invalidFocus，更新当前运行状态。
        const auto invalidFocus = gangyi::agentSubmit(db, access, {{"type", "lesson_focus_request"}, {"courseId", "chem"},
            // 【逐行批注】 执行当前声明或表达式：{"lessonId", preparedLessonId}, {"requestId", "invalid-focus"}});。
            {"lessonId", preparedLessonId}, {"requestId", "invalid-focus"}});
        // 【逐行批注】 定义函数 invalidTeacher，执行该函数声明的职责。
        gangyi::LearningAgent invalidTeacher(path, [&](const auto&, const auto& sink) {
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "错误的板块类型"}, {"actions", {{{"id", "wrong-fo…。
            return output({{"message", "错误的板块类型"}, {"actions", {{{"id", "wrong-focus"}, {"tool", "select_teaching_focus"},
                // 【逐行批注】 执行当前声明或表达式：{"args", {{"lessonId", preparedLessonId}, {"sectionId", shown["sections"][1]["id"]}}…。
                {"args", {{"lessonId", preparedLessonId}, {"sectionId", shown["sections"][1]["id"]}}}}}}, {"state", "waiting_student"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(invalidTeacher);。
        gangyi::registerAgentLessonTools(invalidTeacher);
        // 【逐行批注】 执行当前声明或表达式：invalidTeacher.process(db, invalidFocus["id"]);。
        invalidTeacher.process(db, invalidFocus["id"]);
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, invalidFocus["id"])["status"] ==…。
        assert(gangyi::agentView(db, access, invalidFocus["id"])["status"] == "failed");
        // 【逐行批注】 调用 assert、agentLessonView 完成本行所需操作：assert(gangyi::agentLessonView(db, access, preparedLessonId)["teachin…。
        assert(gangyi::agentLessonView(db, access, preparedLessonId)["teachingFocus"] == focused["teachingFocus"]);
        // 【逐行批注】 计算右侧结果并写入 Json responseEvent，更新当前运行状态。
        Json responseEvent = {{"type", "question_answer"}, {"requestId", "real-input"}, {"courseId", "chem"},
            // 【逐行批注】 执行当前声明或表达式：{"lessonId", preparedLessonId}, {"sectionId", shown["sections"][1]["id"]}, {"section…。
            {"lessonId", preparedLessonId}, {"sectionId", shown["sections"][1]["id"]}, {"sectionVersion", 1},
            // 【逐行批注】 执行当前声明或表达式：{"text", "我认为由0到+2，升高了"}};。
            {"text", "我认为由0到+2，升高了"}};
        // 【逐行批注】 计算右侧结果并写入 auto answering，更新当前运行状态。
        const auto answering = gangyi::agentSubmit(db, access, responseEvent);
        // 【逐行批注】 计算右侧结果并写入 auto currentInputs，更新当前运行状态。
        const auto currentInputs = gangyi::agentContext(db, access, responseEvent).at("currentInputs");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(currentInputs.size() == 1 && currentInputs[0].at("interactionI…。
        assert(currentInputs.size() == 1 && currentInputs[0].at("interactionId").get<std::string>().find("student-") == 0);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(currentInputs[0].at("sectionId") == responseEvent.at("sectionI…。
        assert(currentInputs[0].at("sectionId") == responseEvent.at("sectionId") && currentInputs[0].at("requestId") == "real-input");
        // 【逐行批注】 计算右侧结果并写入 auto beforeDuplicate，更新当前运行状态。
        const auto beforeDuplicate = db.listInteractions().size();
        // 【逐行批注】 调用 assert、agentSubmit 完成本行所需操作：assert(gangyi::agentSubmit(db, access, responseEvent)["id"] == answer…。
        assert(gangyi::agentSubmit(db, access, responseEvent)["id"] == answering["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.listInteractions().size() == beforeDuplicate);。
        assert(db.listInteractions().size() == beforeDuplicate);
        // 【逐行批注】 计算右侧结果并写入 wrongVersion，更新当前运行状态。
        auto wrongVersion = responseEvent; wrongVersion["requestId"] = "wrong-version"; wrongVersion["sectionVersion"] = 2;
        // 【逐行批注】 计算右侧结果并写入 bool versionDenied，更新当前运行状态。
        bool versionDenied = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::agentSubmit(db, access, wrongVersion); } catch (...) { versionDenied = true; }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(versionDenied && db.listInteractions().size() == beforeDuplica…。
        assert(versionDenied && db.listInteractions().size() == beforeDuplicate);
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", a…。
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", answering["id"]}});

        // 【逐行批注】 执行当前声明或表达式：std::string actualAnswerId;。
        std::string actualAnswerId;
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listInteractions()) if (item.kind == "chat-user…。
        for (const auto& item : db.listInteractions()) if (item.kind == "chat-user" &&
            // 【逐行批注】 调用 parse 完成本行所需操作：Json::parse(item.payload).value("requestId", "") == "real-input") {。
            Json::parse(item.payload).value("requestId", "") == "real-input") {
                // 【逐行批注】 计算右侧结果并写入 actualAnswerId，更新当前运行状态。
                actualAnswerId = item.id;
                // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
                const auto input = Json::parse(item.payload);
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(!input["assisted"].get<bool>() && input["hintHistory"].empty()…。
                assert(!input["assisted"].get<bool>() && input["hintHistory"].empty());
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(input["relatedLearning"][0]["text"] == "从元素化合价的变化开始比较。");。
                assert(input["relatedLearning"][0]["text"] == "从元素化合价的变化开始比较。");
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(input["helpMode"] == "independent");。
                assert(input["helpMode"] == "independent");
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(!actualAnswerId.empty());。
        assert(!actualAnswerId.empty());
        // 【逐行批注】 定义函数 evaluation，执行该函数声明的职责。
        gangyi::LearningAgent evaluation(path, [&](const auto&, const auto& sink) {
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "你指出了化合价升高，可进一步联系失电子。"},。
            return output({{"message", "你指出了化合价升高，可进一步联系失电子。"},
                // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "evaluate"}, {"tool", "evaluate_answer"}, {"args", {{"interacti…。
                {"actions", {{{"id", "evaluate"}, {"tool", "evaluate_answer"}, {"args", {{"interactionId", actualAnswerId},
                    // 【逐行批注】 执行当前声明或表达式：{"score", 80}, {"credible", true}, {"feedback", "化合价变化判断正确，过程可补充电子关系"}, {"feedbackHe…。
                    {"score", 80}, {"credible", true}, {"feedback", "化合价变化判断正确，过程可补充电子关系"}, {"feedbackHelpType", "hint"},
                    // 【逐行批注】 执行当前声明或表达式：{"diagnosis", {{"summary", "尚未说明电子变化"}, {"evidence", {"由0到+2"}},。
                    {"diagnosis", {{"summary", "尚未说明电子变化"}, {"evidence", {"由0到+2"}},
                        // 【逐行批注】 执行当前声明或表达式：{"teachingDirection", "联系失电子与化合价升高"}, {"needsClarification", false}}},。
                        {"teachingDirection", "联系失电子与化合价升高"}, {"needsClarification", false}}},
                    // 【逐行批注】 执行当前声明或表达式：{"reason", "引用学生实际写出的0到+2"}, {"uncertainty", "仅能确认本题判断，不能推断全部知识"}}}}}},。
                    {"reason", "引用学生实际写出的0到+2"}, {"uncertainty", "仅能确认本题判断，不能推断全部知识"}}}}}},
                // 【逐行批注】 执行当前声明或表达式：{"state", "waiting_student"}}, sink);。
                {"state", "waiting_student"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(evaluation);。
        gangyi::registerAgentLessonTools(evaluation);
        // 【逐行批注】 计算右侧结果并写入 auto assessing，更新当前运行状态。
        const auto assessing = gangyi::agentSubmit(db, access, {{"type", "assess"}, {"requestId", "one-real-question"}});
        // 【逐行批注】 执行当前声明或表达式：evaluation.process(db, assessing["id"]);。
        evaluation.process(db, assessing["id"]);
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, assessing["id"])["status"] == "w…。
        assert(gangyi::agentView(db, access, assessing["id"])["status"] == "waiting_student");
        // 【逐行批注】 计算右侧结果并写入 bool assessmentFound，更新当前运行状态。
        bool assessmentFound = false;
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listInteractions()) if (item.kind == "practice"…。
        for (const auto& item : db.listInteractions()) if (item.kind == "practice") {
            // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
            const auto saved = Json::parse(item.payload);
            // 【逐行批注】 判断条件 (saved.value("interactionId", "") == actualAnswerId) {，成立时执行对应分支。
            if (saved.value("interactionId", "") == actualAnswerId) {
                // 【逐行批注】 计算右侧结果并写入 assessmentFound，更新当前运行状态。
                assessmentFound = true; assert(saved["score"] == 80 && saved["questionId"] == shown["sections"][1]["questionId"]);
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(saved["source"] == "ai" && saved["questionSnapshot"]["expected…。
                assert(saved["source"] == "ai" && saved["questionSnapshot"]["expectedAnswer"] == "由0升高到+2");
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(assessmentFound);。
        assert(assessmentFound);
        // 【逐行批注】 计算右侧结果并写入 auto summary，更新当前运行状态。
        const auto summary = gangyi::agentLessonView(db, access, preparedLessonId);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(summary["evaluations"].size() == 1 && summary["evaluations"][0…。
        assert(summary["evaluations"].size() == 1 && summary["evaluations"][0]["score"] == 80);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(summary.dump().find("expectedAnswer") == std::string::npos && …。
        assert(summary.dump().find("expectedAnswer") == std::string::npos && summary.dump().find("rubric") == std::string::npos);

        // 重现真实备课错误：证据是数组，但工具返回必须有对象外壳。
        // 【逐行批注】 计算右侧结果并写入 int evidenceCalls，更新当前运行状态。
        int evidenceCalls = 0;
        // 【逐行批注】 定义函数 evidenceReader，执行该函数声明的职责。
        gangyi::LearningAgent evidenceReader(path, [&](const auto& options, const auto& sink) {
            // 【逐行批注】 判断条件 (++evidenceCalls == 1) return output({{"message", "读取真实证据"}, {"a…，成立时执行对应分支。
            if (++evidenceCalls == 1) return output({{"message", "读取真实证据"}, {"actions", {{{"id", "evidence"}, {"tool", "read_profile_evidence"}, {"args", Json::object()}}}}, {"state", "continue"}}, sink);
            // 【逐行批注】 计算右侧结果并写入 bool found，更新当前运行状态。
            bool found = false;
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : options.messages) if (item.role == "user") {。
            for (const auto& item : options.messages) if (item.role == "user") {
                // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
                const auto value = Json::parse(item.content, nullptr, false);
                // 【逐行批注】 判断条件 (value.is_object() && value.contains("toolResults")) found = val…，成立时执行对应分支。
                if (value.is_object() && value.contains("toolResults")) found = value["toolResults"][0]["result"]["evidence"].is_array();
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 assert、output、array 完成本行所需操作：assert(found); return output({{"message", "证据读取完成"}, {"actions", Json…。
            assert(found); return output({{"message", "证据读取完成"}, {"actions", Json::array()}, {"state", "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 计算右侧结果并写入 auto evidenceTask，更新当前运行状态。
        const auto evidenceTask = gangyi::agentSubmit(db, access, {{"type", "inspect"}, {"requestId", "evidence-object"}});
        // 【逐行批注】 执行当前声明或表达式：evidenceReader.process(db, evidenceTask["id"]);。
        evidenceReader.process(db, evidenceTask["id"]);
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(evidenceCalls == 2 && gangyi::agentView(db, access, evidenceTa…。
        assert(evidenceCalls == 2 && gangyi::agentView(db, access, evidenceTask["id"])["status"] == "ready");

        // 服务商明确拒绝立即停止，可恢复错误才做三次有限重试。
        // 【逐行批注】 按循环规则重复处理：for (const auto& spec : std::vector<std::pair<std::string, int>>{{"missing…。
        for (const auto& spec : std::vector<std::pair<std::string, int>>{{"missing_config",0},{"provider_rejected",402},{"auth_error",401},{"rate_limited",429},{"provider_5xx",503},{"timeout",0}}) {
            // 【逐行批注】 计算右侧结果并写入 int attempts，更新当前运行状态。
            int attempts = 0;
            // 【逐行批注】 声明对象 failing，并通过构造函数初始化其状态。
            gangyi::LearningAgent failing(path, [&](const auto&, const auto&) -> gangyi::AIResult { ++attempts; throw gangyi::AIClientError(spec.first, "虚构服务商错误", spec.second); });
            // 【逐行批注】 计算右侧结果并写入 auto request，更新当前运行状态。
            const auto request = gangyi::agentSubmit(db, access, {{"type", "inspect"}, {"requestId", "provider-" + spec.first}});
            // 【逐行批注】 计算右侧结果并写入 failing.process(db, request["id"]); const au…，更新当前运行状态。
            failing.process(db, request["id"]); const auto result = gangyi::agentView(db, access, request["id"]);
            // 【逐行批注】 计算右侧结果并写入 bool canRetry，更新当前运行状态。
            const bool canRetry = spec.first != "provider_rejected" && spec.first != "auth_error" && spec.first != "missing_config";
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(attempts == (canRetry ? 3 : 1) && result["status"] == "failed"…。
            assert(attempts == (canRetry ? 3 : 1) && result["status"] == "failed");
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(result["failure"]["httpStatus"] == spec.second && result["fail…。
            assert(result["failure"]["httpStatus"] == spec.second && result["failure"]["retryable"] == canRetry);
            // 【逐行批注】 判断条件 (spec.first == "missing_config") assert(result["failure"]["messa…，成立时执行对应分支。
            if (spec.first == "missing_config") assert(result["failure"]["message"].get<std::string>().find("AI 设置") != std::string::npos);
        // 【逐行批注】 结束当前代码块。
        }

        // 学生在练习页请求选择题：AI 追加的互动题也必须接续显示在当前页。
        // 【逐行批注】 定义函数 followup，执行该函数声明的职责。
        gangyi::LearningAgent followup(path, [&](const auto&, const auto& sink) {
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "请在新选择题中作答"}, {"actions", {{{"id", "follow…。
            return output({{"message", "请在新选择题中作答"}, {"actions", {{{"id", "followup"}, {"tool", "append_section"},
                // 【逐行批注】 执行当前声明或表达式：{"args", {{"lessonId", preparedLessonId}, {"questionKind", "interaction"}, {"section…。
                {"args", {{"lessonId", preparedLessonId}, {"questionKind", "interaction"}, {"section", {{"kind", "question"}, {"title", "根据回答追加的选择题"},
                    // 【逐行批注】 执行当前声明或表达式：{"question", {{"question", "化合价升高表示什么？"}, {"type", "choice"}, {"options", {"失去电子", "…。
                    {"question", {{"question", "化合价升高表示什么？"}, {"type", "choice"}, {"options", {"失去电子", "得到电子"}}, {"expectedAnswer", "失去电子"}}}}}}}}}}, {"state", "waiting_student"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(followup);。
        gangyi::registerAgentLessonTools(followup);
        // 【逐行批注】 计算右侧结果并写入 auto followupTask，更新当前运行状态。
        const auto followupTask = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"lessonId", preparedLessonId}, {"courseId", "chem"},
            // 【逐行批注】 执行当前声明或表达式：{"view", "practice"}, {"text", "请根据我的回答出一道选择题"}, {"requestId", "followup-choice"}});。
            {"view", "practice"}, {"text", "请根据我的回答出一道选择题"}, {"requestId", "followup-choice"}});
        // 【逐行批注】 执行当前声明或表达式：followup.process(db, followupTask["id"]);。
        followup.process(db, followupTask["id"]);
        // 【逐行批注】 计算右侧结果并写入 auto followupLesson，更新当前运行状态。
        const auto followupLesson = gangyi::agentLessonView(db, access, preparedLessonId);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(followupLesson["teachingFocus"]["responseView"] == "practice");。
        assert(followupLesson["teachingFocus"]["responseView"] == "practice");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(followupLesson["teachingFocus"]["responseSectionId"] == follow…。
        assert(followupLesson["teachingFocus"]["responseSectionId"] == followupLesson["sections"].back()["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(followupLesson["sections"].back()["question"]["options"].size(…。
        assert(followupLesson["sections"].back()["question"]["options"].size() == 2);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(followupLesson.dump().find("expectedAnswer") == std::string::n…。
        assert(followupLesson.dump().find("expectedAnswer") == std::string::npos);
        // 【逐行批注】 计算右侧结果并写入 legacyRow，更新当前运行状态。
        auto legacyRow = *db.getClassroomActivity(preparedLessonId);
        // 【逐行批注】 计算右侧结果并写入 legacyLesson，更新当前运行状态。
        auto legacyLesson = Json::parse(legacyRow.payload);
        // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"responseSectionId", "responseView", "responseTask…。
        for (const auto* key : {"responseSectionId", "responseView", "responseTaskId"}) legacyLesson["teachingFocus"].erase(key);
        // 【逐行批注】 计算右侧结果并写入 legacyRow.payload，更新当前运行状态。
        legacyRow.payload = legacyLesson.dump(); db.upsert(legacyRow);
        // 【逐行批注】 计算右侧结果并写入 auto recoveredLesson，更新当前运行状态。
        const auto recoveredLesson = gangyi::agentLessonView(db, access, preparedLessonId);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(recoveredLesson["teachingFocus"]["responseSectionId"] == follo…。
        assert(recoveredLesson["teachingFocus"]["responseSectionId"] == followupLesson["sections"].back()["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(recoveredLesson["teachingFocus"]["responseView"] == "practice"…。
        assert(recoveredLesson["teachingFocus"]["responseView"] == "practice");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity(preparedLessonId)->payload == legacyRo…。
        assert(db.getClassroomActivity(preparedLessonId)->payload == legacyRow.payload);

        // 重现真实失败：同课程还有另一课时，模型误用它的 ID 时不能静默保存新题。
        // 【逐行批注】 计算右侧结果并写入 otherRow，更新当前运行状态。
        auto otherRow = *db.getClassroomActivity(preparedLessonId);
        // 【逐行批注】 计算右侧结果并写入 otherLesson，更新当前运行状态。
        auto otherLesson = Json::parse(otherRow.payload);
        // 【逐行批注】 计算右侧结果并写入 otherRow.id，更新当前运行状态。
        otherRow.id = "another-saved-lesson"; otherLesson["id"] = otherRow.id;
        // 【逐行批注】 执行当前声明或表达式：otherLesson.erase("teachingTaskId"); otherLesson.erase("initialTeachingTaskId");。
        otherLesson.erase("teachingTaskId"); otherLesson.erase("initialTeachingTaskId");
        // 【逐行批注】 计算右侧结果并写入 otherRow.payload，更新当前运行状态。
        otherRow.payload = otherLesson.dump(); assert(db.upsert(otherRow));
        // 【逐行批注】 按循环规则重复处理：for (const std::string tool : {"append_section", "select_teaching_focus"})…。
        for (const std::string tool : {"append_section", "select_teaching_focus"}) {
            // 【逐行批注】 计算右侧结果并写入 bool corrected，更新当前运行状态。
            bool corrected = false; int targetCalls = 0;
            // 【逐行批注】 定义函数 targetGuard，执行该函数声明的职责。
            gangyi::LearningAgent targetGuard(path, [&](const auto&, const auto& sink) {
                // 【逐行批注】 执行当前声明或表达式：++targetCalls;。
                ++targetCalls;
                // 【逐行批注】 计算右侧结果并写入 Json args，更新当前运行状态。
                Json args = {{"lessonId", corrected ? preparedLessonId : otherRow.id}};
                // 【逐行批注】 判断条件 (tool == "append_section") args["section"] = {{"kind", "question…，成立时执行对应分支。
                if (tool == "append_section") args["section"] = {{"kind", "question"}, {"title", "当前课的补充题"},
                    // 【逐行批注】 执行当前声明或表达式：{"question", {{"question", "电子转移与化合价怎样对应？"}, {"type", "choice"},。
                    {"question", {{"question", "电子转移与化合价怎样对应？"}, {"type", "choice"},
                        // 【逐行批注】 执行当前声明或表达式：{"options", {"失电子时升高", "失电子时降低"}}, {"expectedAnswer", "失电子时升高"}}}};。
                        {"options", {"失电子时升高", "失电子时降低"}}, {"expectedAnswer", "失电子时升高"}}}};
                // 【逐行批注】 执行前一条件不成立时的分支。
                else args["sectionId"] = shown["sections"][0]["id"];
                // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "请继续当前课"}, {"actions", {{{"id", "targeted-…。
                return output({{"message", "请继续当前课"}, {"actions", {{{"id", "targeted-action"}, {"tool", tool}, {"args", args}}}},
                    // 【逐行批注】 执行当前声明或表达式：{"state", "waiting_student"}}, sink);。
                    {"state", "waiting_student"}}, sink);
            // 【逐行批注】 执行当前声明或表达式：});。
            });
            // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(targetGuard);。
            gangyi::registerAgentLessonTools(targetGuard);
            // 【逐行批注】 计算右侧结果并写入 auto guarded，更新当前运行状态。
            const auto guarded = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"lessonId", preparedLessonId},
                // 【逐行批注】 执行当前声明或表达式：{"courseId", "chem"}, {"view", "practice"}, {"text", "请在本课继续"}, {"requestId", "wrong…。
                {"courseId", "chem"}, {"view", "practice"}, {"text", "请在本课继续"}, {"requestId", "wrong-target-" + tool}});
            // 【逐行批注】 计算右侧结果并写入 auto beforeTarget，更新当前运行状态。
            const auto beforeTarget = db.getClassroomActivity(preparedLessonId)->payload;
            // 【逐行批注】 执行当前声明或表达式：targetGuard.process(db, guarded["id"]);。
            targetGuard.process(db, guarded["id"]);
            // 【逐行批注】 计算右侧结果并写入 auto rejected，更新当前运行状态。
            const auto rejected = gangyi::agentView(db, access, guarded["id"]);
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(rejected["status"] == "failed" && targetCalls == 3);。
            assert(rejected["status"] == "failed" && targetCalls == 3);
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(rejected["failure"]["field"] == "args.lessonId" && rejected["f…。
            assert(rejected["failure"]["field"] == "args.lessonId" && rejected["failure"]["tool"] == tool);
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity(otherRow.id)->payload == otherRow.payl…。
            assert(db.getClassroomActivity(otherRow.id)->payload == otherRow.payload);
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity(preparedLessonId)->payload == beforeTa…。
            assert(db.getClassroomActivity(preparedLessonId)->payload == beforeTarget);
            // 【逐行批注】 计算右侧结果并写入 corrected，更新当前运行状态。
            corrected = true;
            // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", gu…。
            gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", guarded["id"]}});
            // 【逐行批注】 执行当前声明或表达式：targetGuard.process(db, guarded["id"]);。
            targetGuard.process(db, guarded["id"]);
            // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, guarded["id"])["status"] == "wai…。
            assert(gangyi::agentView(db, access, guarded["id"])["status"] == "waiting_student" && targetCalls == 4);
            // 【逐行批注】 计算右侧结果并写入 auto correctedLesson，更新当前运行状态。
            const auto correctedLesson = gangyi::agentLessonView(db, access, preparedLessonId);
            // 【逐行批注】 判断条件 (tool == "append_section") {，成立时执行对应分支。
            if (tool == "append_section") {
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(correctedLesson["teachingFocus"]["responseSectionId"] == corre…。
                assert(correctedLesson["teachingFocus"]["responseSectionId"] == correctedLesson["sections"].back()["id"]);
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(correctedLesson["sections"].back()["question"]["options"].size…。
                assert(correctedLesson["sections"].back()["question"]["options"].size() == 2);
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(correctedLesson.dump().find("expectedAnswer") == std::string::…。
                assert(correctedLesson.dump().find("expectedAnswer") == std::string::npos);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity(otherRow.id)->payload == otherRow.payl…。
            assert(db.getClassroomActivity(otherRow.id)->payload == otherRow.payload);
        // 【逐行批注】 结束当前代码块。
        }

        // 旧任务仍可读历史，但不能在已被新任务接手的课时追加或激活新题。
        // 【逐行批注】 计算右侧结果并写入 auto staleAppend，更新当前运行状态。
        const auto staleAppend = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"lessonId", preparedLessonId},
            // 【逐行批注】 执行当前声明或表达式：{"courseId", "chem"}, {"text", "旧交流"}, {"requestId", "stale-append"}});。
            {"courseId", "chem"}, {"text", "旧交流"}, {"requestId", "stale-append"}});
        // 【逐行批注】 计算右侧结果并写入 claimedRow，更新当前运行状态。
        auto claimedRow = *db.getClassroomActivity(preparedLessonId);
        // 【逐行批注】 计算右侧结果并写入 claimed，更新当前运行状态。
        auto claimed = Json::parse(claimedRow.payload); claimed["teachingTaskId"] = "newer-teaching-task";
        // 【逐行批注】 计算右侧结果并写入 claimedRow.payload，更新当前运行状态。
        claimedRow.payload = claimed.dump(); assert(db.upsert(claimedRow));
        // 【逐行批注】 执行当前声明或表达式：followup.process(db, staleAppend["id"]);。
        followup.process(db, staleAppend["id"]);
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, staleAppend["id"])["status"] == …。
        assert(gangyi::agentView(db, access, staleAppend["id"])["status"] == "failed");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity(preparedLessonId)->payload == claimedR…。
        assert(db.getClassroomActivity(preparedLessonId)->payload == claimedRow.payload);
        // 【逐行批注】 声明对象 shutdown，并通过构造函数初始化其状态。
        gangyi::LearningAgent shutdown(path, [&](const auto&, const auto&) -> gangyi::AIResult {
            // 【逐行批注】 调用 shutdown、AIClientError 完成本行所需操作：gangyi::AIActivity::shutdown(); throw gangyi::AIClientError("cancelle…。
            gangyi::AIActivity::shutdown(); throw gangyi::AIClientError("cancelled", "软件正常退出");
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 计算右侧结果并写入 auto shutdownTask，更新当前运行状态。
        const auto shutdownTask = gangyi::agentSubmit(db, access, {{"type", "inspect"}, {"requestId", "shutdown-recovery"}});
        // 【逐行批注】 执行当前声明或表达式：shutdown.process(db, shutdownTask["id"]);。
        shutdown.process(db, shutdownTask["id"]);
        // 【逐行批注】 计算右侧结果并写入 auto shutdownState，更新当前运行状态。
        const auto shutdownState = gangyi::agentView(db, access, shutdownTask["id"]);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(shutdownState["status"] == "paused" && shutdownState["pauseRea…。
        assert(shutdownState["status"] == "paused" && shutdownState["pauseReason"] == "shutdown" && !shutdownState.contains("failure"));
        // 【逐行批注】 调用 configure 完成本行所需操作：gangyi::AIActivity::configure("");。
        gangyi::AIActivity::configure("");
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "resume"}, {"taskId", s…。
        gangyi::agentControl(db, access, {{"command", "resume"}, {"taskId", shutdownTask["id"]}});
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, shutdownTask["id"])["status"] ==…。
        assert(gangyi::agentView(db, access, shutdownTask["id"])["status"] == "pending");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 runTeachingEvidenceChecks 完成本行所需操作：runTeachingEvidenceChecks(root / "teaching.db");。
    runTeachingEvidenceChecks(root / "teaching.db");
    // 【逐行批注】 调用 runEvaluationFeedbackChecks 完成本行所需操作：runEvaluationFeedbackChecks(root / "evaluation-feedback.db");。
    runEvaluationFeedbackChecks(root / "evaluation-feedback.db");
    // 【逐行批注】 调用 runDemonstratedQuestionChecks 完成本行所需操作：runDemonstratedQuestionChecks(root / "demonstrated-question.db");。
    runDemonstratedQuestionChecks(root / "demonstrated-question.db");
    // 【逐行批注】 调用 runAgentEfficiencyChecks 完成本行所需操作：runAgentEfficiencyChecks(root / "efficiency.db");。
    runAgentEfficiencyChecks(root / "efficiency.db");
    // 【逐行批注】 调用 runAgentQuantumChecks 完成本行所需操作：runAgentQuantumChecks(root / "quantum.db");。
    runAgentQuantumChecks(root / "quantum.db");
    // 【逐行批注】 调用 runSectionNavigationChecks 完成本行所需操作：runSectionNavigationChecks(root / "section-navigation.db");。
    runSectionNavigationChecks(root / "section-navigation.db");
    // 【逐行批注】 调用 runLessonStartBindingChecks 完成本行所需操作：runLessonStartBindingChecks(root / "lesson-start-binding.db");。
    runLessonStartBindingChecks(root / "lesson-start-binding.db");
    // 【逐行批注】 调用 remove_all 完成本行所需操作：std::filesystem::remove_all(root);。
    std::filesystem::remove_all(root);
    // 【逐行批注】 执行当前声明或表达式：std::cout << "多步 AI 主控、幂等、访问隔离、失败停止与事务回滚测试通过\n";。
    std::cout << "多步 AI 主控、幂等、访问隔离、失败停止与事务回滚测试通过\n";
// 【逐行批注】 结束当前代码块。
}

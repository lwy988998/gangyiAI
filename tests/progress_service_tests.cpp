// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"
// 【逐行批注】 引入头文件 "progress_service.hpp"，为后续实现提供类型或接口声明。
#include "progress_service.hpp"

// 【逐行批注】 引入头文件 <sqlite3.h>，为后续实现提供类型或接口声明。
#include <sqlite3.h>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>

// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 int failures，更新当前运行状态。
int failures = 0;

// 【逐行批注】 定义函数 expect，执行该函数声明的职责。
void expect(bool condition, const char* message) {
    // 【逐行批注】 判断条件 (!condition) {，成立时执行对应分支。
    if (!condition) {
        // 【逐行批注】 执行当前声明或表达式：std::cerr << "失败: " << message << '\n';。
        std::cerr << "失败: " << message << '\n';
        // 【逐行批注】 执行当前声明或表达式：++failures;。
        ++failures;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
    // 【逐行批注】 计算右侧结果并写入 auto path，更新当前运行状态。
    const auto path = std::filesystem::temp_directory_path() / "gangyiAI-progress-service-tests.db";
    // 【逐行批注】 执行当前声明或表达式：std::error_code filesystemError;。
    std::error_code filesystemError;
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(path, filesystemError);。
    std::filesystem::remove(path, filesystemError);

    // 【逐行批注】 执行当前声明或表达式：gangyi::Database database;。
    gangyi::Database database;
    // 【逐行批注】 执行当前声明或表达式：database.open(path.u8string());。
    database.open(path.u8string());
    // 【逐行批注】 执行当前声明或表达式：database.migrate();。
    database.migrate();

    // 【逐行批注】 执行当前声明或表达式：gangyi::Course course;。
    gangyi::Course course;
    // 【逐行批注】 计算右侧结果并写入 course.id，更新当前运行状态。
    course.id = "course-1";
    // 【逐行批注】 计算右侧结果并写入 course.anonymousId，更新当前运行状态。
    course.anonymousId = "anonymous-1";
    // 【逐行批注】 计算右侧结果并写入 course.goal，更新当前运行状态。
    course.goal = "掌握函数单调性";
    // 【逐行批注】 计算右侧结果并写入 course.mode，更新当前运行状态。
    course.mode = "deep";
    // 【逐行批注】 计算右侧结果并写入 course.title，更新当前运行状态。
    course.title = "函数单调性课程";
    // 【逐行批注】 计算右侧结果并写入 course.createdAt，更新当前运行状态。
    course.createdAt = "2026-09-07T00:00:00Z";
    // 【逐行批注】 计算右侧结果并写入 course.updatedAt，更新当前运行状态。
    course.updatedAt = course.createdAt;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(database.insert(course), "应创建测试课程");。
    expect(database.insert(course), "应创建测试课程");

    // 【逐行批注】 执行当前声明或表达式：gangyi::CourseSnapshot snapshot;。
    gangyi::CourseSnapshot snapshot;
    // 【逐行批注】 计算右侧结果并写入 snapshot.id，更新当前运行状态。
    snapshot.id = "snapshot-1";
    // 【逐行批注】 计算右侧结果并写入 snapshot.courseId，更新当前运行状态。
    snapshot.courseId = course.id;
    // 【逐行批注】 执行当前声明或表达式：nlohmann::json payload;。
    nlohmann::json payload;
    // 【逐行批注】 计算右侧结果并写入 payload["roadmap"]，更新当前运行状态。
    payload["roadmap"] = nlohmann::json::array();
    // 【逐行批注】 调用 array 完成本行所需操作：payload["roadmap"].push_back({{"tasks", nlohmann::json::array({"任务一",…。
    payload["roadmap"].push_back({{"tasks", nlohmann::json::array({"任务一", "任务二"})},
                                   // 【逐行批注】 调用 array 完成本行所需操作：{"steps", nlohmann::json::array({{{"title", "步骤一"}}})}});。
                                   {"steps", nlohmann::json::array({{{"title", "步骤一"}}})}});
    // 【逐行批注】 计算右侧结果并写入 payload["courseStructure"]，更新当前运行状态。
    payload["courseStructure"] = nlohmann::json::array();
    // 【逐行批注】 调用 array 完成本行所需操作：payload["courseStructure"].push_back({{"topics", nlohmann::json::arra…。
    payload["courseStructure"].push_back({{"topics", nlohmann::json::array({"主题一", "主题二"})}});
    // 【逐行批注】 计算右侧结果并写入 snapshot.payload，更新当前运行状态。
    snapshot.payload = payload.dump();
    // 【逐行批注】 计算右侧结果并写入 snapshot.createdAt，更新当前运行状态。
    snapshot.createdAt = course.createdAt;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(database.insert(snapshot), "应创建课程快照");。
    expect(database.insert(snapshot), "应创建课程快照");

    // 【逐行批注】 计算右侧结果并写入 saved，更新当前运行状态。
    auto saved = gangyi::saveTaskProgress(database, {{"courseId", course.id}, {"anonymousId", "anonymous-1"},
        // 【逐行批注】 执行当前声明或表达式：{"goal", course.goal}, {"mode", "deep"}, {"phaseIndex", 1}, {"phaseName", "基础阶段"},。
        {"goal", course.goal}, {"mode", "deep"}, {"phaseIndex", 1}, {"phaseName", "基础阶段"},
        // 【逐行批注】 执行当前声明或表达式：{"taskIndex", 0}, {"taskTitle", "任务一"}, {"status", "completed"}});。
        {"taskIndex", 0}, {"taskTitle", "任务一"}, {"status", "completed"}});
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(saved.ok && saved.item.value("status", "") == "completed", "应保…。
    expect(saved.ok && saved.item.value("status", "") == "completed", "应保存任务完成状态");

    // 【逐行批注】 计算右侧结果并写入 saved，更新当前运行状态。
    saved = gangyi::saveTaskProgress(database, {{"courseId", course.id}, {"phaseIndex", 1},
        // 【逐行批注】 执行当前声明或表达式：{"taskIndex", 1}, {"status", "非法状态"}});。
        {"taskIndex", 1}, {"status", "非法状态"}});
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(saved.ok && saved.item.value("status", "") == "not_started", "…。
    expect(saved.ok && saved.item.value("status", "") == "not_started", "非法任务状态应归一化");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(database.listTaskProgress().size() == 2, "任务进度应按课程、阶段和序号 upser…。
    expect(database.listTaskProgress().size() == 2, "任务进度应按课程、阶段和序号 upsert");

    // 【逐行批注】 执行当前声明或表达式：gangyi::LearningStepProgress step;。
    gangyi::LearningStepProgress step;
    // 【逐行批注】 计算右侧结果并写入 step.id，更新当前运行状态。
    step.id = "step-1";
    // 【逐行批注】 计算右侧结果并写入 step.courseId，更新当前运行状态。
    step.courseId = course.id;
    // 【逐行批注】 计算右侧结果并写入 step.anonymousId，更新当前运行状态。
    step.anonymousId = "anonymous-1";
    // 【逐行批注】 计算右侧结果并写入 step.goal，更新当前运行状态。
    step.goal = course.goal;
    // 【逐行批注】 计算右侧结果并写入 step.mode，更新当前运行状态。
    step.mode = "deep";
    // 【逐行批注】 计算右侧结果并写入 step.phaseIndex，更新当前运行状态。
    step.phaseIndex = 1;
    // 【逐行批注】 计算右侧结果并写入 step.phaseName，更新当前运行状态。
    step.phaseName = "基础阶段";
    // 【逐行批注】 计算右侧结果并写入 step.stepIndex，更新当前运行状态。
    step.stepIndex = 0;
    // 【逐行批注】 计算右侧结果并写入 step.stepTitle，更新当前运行状态。
    step.stepTitle = "步骤一";
    // 【逐行批注】 计算右侧结果并写入 step.status，更新当前运行状态。
    step.status = "understood";
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(database.insert(step), "应创建步骤进度");。
    expect(database.insert(step), "应创建步骤进度");

    // 【逐行批注】 执行当前声明或表达式：gangyi::LearningCardProgress card;。
    gangyi::LearningCardProgress card;
    // 【逐行批注】 计算右侧结果并写入 card.id，更新当前运行状态。
    card.id = "card-1";
    // 【逐行批注】 计算右侧结果并写入 card.courseId，更新当前运行状态。
    card.courseId = course.id;
    // 【逐行批注】 计算右侧结果并写入 card.anonymousId，更新当前运行状态。
    card.anonymousId = "anonymous-1";
    // 【逐行批注】 计算右侧结果并写入 card.goal，更新当前运行状态。
    card.goal = course.goal;
    // 【逐行批注】 计算右侧结果并写入 card.mode，更新当前运行状态。
    card.mode = "deep";
    // 【逐行批注】 计算右侧结果并写入 card.phaseIndex，更新当前运行状态。
    card.phaseIndex = 1;
    // 【逐行批注】 计算右侧结果并写入 card.phaseName，更新当前运行状态。
    card.phaseName = "基础阶段";
    // 【逐行批注】 计算右侧结果并写入 card.topicIndex，更新当前运行状态。
    card.topicIndex = 1;
    // 【逐行批注】 计算右侧结果并写入 card.topicTitle，更新当前运行状态。
    card.topicTitle = "主题一";
    // 【逐行批注】 计算右侧结果并写入 card.status，更新当前运行状态。
    card.status = "completed";
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(database.insert(card), "应创建学习卡进度");。
    expect(database.insert(card), "应创建学习卡进度");

    // 【逐行批注】 计算右侧结果并写入 progress，更新当前运行状态。
    auto progress = gangyi::recomputeCourseProgress(database, course.id, "anonymous-1", course.goal);
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(progress.has_value(), "应重算课程进度");。
    expect(progress.has_value(), "应重算课程进度");
    // 【逐行批注】 调用 expect、value 完成本行所需操作：expect(progress && progress->value("completedCount", 0) == 3, "应合并任务、…。
    expect(progress && progress->value("completedCount", 0) == 3, "应合并任务、步骤和学习卡完成数");
    // 【逐行批注】 调用 expect、value 完成本行所需操作：expect(progress && progress->value("totalCount", 0) == 5, "应按课程快照计算总项…。
    expect(progress && progress->value("totalCount", 0) == 5, "应按课程快照计算总项目数");
    // 【逐行批注】 调用 expect、value 完成本行所需操作：expect(progress && progress->value("overallPercent", 0) == 60, "课程完成度…。
    expect(progress && progress->value("overallPercent", 0) == 60, "课程完成度应正确取整");

    // 【逐行批注】 调用 expect、updateLastVisited 完成本行所需操作：expect(gangyi::updateLastVisited(database, course.id, "anonymous-1", …。
    expect(gangyi::updateLastVisited(database, course.id, "anonymous-1", course.goal, "deep",
        // 【逐行批注】 计算右侧结果并写入 {{"lastVisitedUrl", "/learn?courseId，更新当前运行状态。
        {{"lastVisitedUrl", "/learn?courseId=course-1"}, {"lastPageType", "learn"},
         // 【逐行批注】 执行当前声明或表达式：{"lastPhaseIndex", 1}, {"lastTopicIndex", 1}}), "应保存站内学习断点");。
         {"lastPhaseIndex", 1}, {"lastTopicIndex", 1}}), "应保存站内学习断点");
    // 【逐行批注】 计算右侧结果并写入 stored，更新当前运行状态。
    auto stored = database.findProgressByCourseId(course.id);
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(stored && stored->lastVisitedUrl.value_or("") == "/learn?cours…。
    expect(stored && stored->lastVisitedUrl.value_or("") == "/learn?courseId=course-1", "应恢复站内断点地址");
    // 【逐行批注】 调用 expect、updateLastVisited 完成本行所需操作：expect(gangyi::updateLastVisited(database, course.id, "anonymous-1", …。
    expect(gangyi::updateLastVisited(database, course.id, "anonymous-1", course.goal, "deep",
        // 【逐行批注】 执行当前声明或表达式：{{"lastVisitedUrl", "//example.com/unsafe"}}), "忽略外部地址时仍应保留进度记录");。
        {{"lastVisitedUrl", "//example.com/unsafe"}}), "忽略外部地址时仍应保留进度记录");
    // 【逐行批注】 计算右侧结果并写入 stored，更新当前运行状态。
    stored = database.findProgressByCourseId(course.id);
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(stored && stored->lastVisitedUrl.value_or("") == "/learn?cours…。
    expect(stored && stored->lastVisitedUrl.value_or("") == "/learn?courseId=course-1", "不得保存协议相对外部地址");
    // 【逐行批注】 调用 expect、updateLastVisited 完成本行所需操作：expect(gangyi::updateLastVisited(database, course.id, "anonymous-1", …。
    expect(gangyi::updateLastVisited(database, course.id, "anonymous-1", course.goal, "deep",
        // 【逐行批注】 计算右侧结果并写入 {{"lastVisitedUrl", "/learn?next，更新当前运行状态。
        {{"lastVisitedUrl", "/learn?next=https://example.com/unsafe"}}), "忽略外部跳转参数时仍应保留进度记录");
    // 【逐行批注】 计算右侧结果并写入 stored，更新当前运行状态。
    stored = database.findProgressByCourseId(course.id);
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(stored && stored->lastVisitedUrl.value_or("") == "/learn?cours…。
    expect(stored && stored->lastVisitedUrl.value_or("") == "/learn?courseId=course-1", "不得保存包含外部双斜杠的地址");

    // 【逐行批注】 计算右侧结果并写入 progress，更新当前运行状态。
    progress = gangyi::resetCourseProgress(database, course.id, "anonymous-1", course.goal);
    // 【逐行批注】 调用 expect、value 完成本行所需操作：expect(progress && progress->value("completedCount", -1) == 0, "重置后完成…。
    expect(progress && progress->value("completedCount", -1) == 0, "重置后完成数应归零");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(database.listTaskProgress().empty() && database.listLearningSt…。
    expect(database.listTaskProgress().empty() && database.listLearningStepProgress().empty() &&
        // 【逐行批注】 执行当前声明或表达式：database.listLearningCardProgress().empty(), "重置应清理三类学习进度");。
        database.listLearningCardProgress().empty(), "重置应清理三类学习进度");

    // 【逐行批注】 执行当前声明或表达式：gangyi::LearningSession legacy;。
    gangyi::LearningSession legacy;
    // 【逐行批注】 计算右侧结果并写入 legacy.id，更新当前运行状态。
    legacy.id = "legacy-session";
    // 【逐行批注】 计算右侧结果并写入 legacy.courseId，更新当前运行状态。
    legacy.courseId = course.id;
    // 【逐行批注】 计算右侧结果并写入 legacy.goal，更新当前运行状态。
    legacy.goal = course.goal;
    // 【逐行批注】 计算右侧结果并写入 legacy.mode，更新当前运行状态。
    legacy.mode = "deep";
    // 【逐行批注】 计算右侧结果并写入 legacy.phaseIndex，更新当前运行状态。
    legacy.phaseIndex = 1;
    // 【逐行批注】 计算右侧结果并写入 legacy.phaseName，更新当前运行状态。
    legacy.phaseName = "基础阶段";
    // 【逐行批注】 计算右侧结果并写入 legacy.topicIndex，更新当前运行状态。
    legacy.topicIndex = 1;
    // 【逐行批注】 计算右侧结果并写入 legacy.topicTitle，更新当前运行状态。
    legacy.topicTitle = "旧模板主题";
    // 【逐行批注】 计算右侧结果并写入 legacy.title，更新当前运行状态。
    legacy.title = "旧模板课堂";
    // 【逐行批注】 计算右侧结果并写入 legacy.content，更新当前运行状态。
    legacy.content = "{}";
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(database.insert(legacy), "应创建待迁移的旧课堂记录");。
    expect(database.insert(legacy), "应创建待迁移的旧课堂记录");

    // 【逐行批注】 执行当前声明或表达式：database.close();。
    database.close();
    // 【逐行批注】 计算右侧结果并写入 sqlite3* raw，更新当前运行状态。
    sqlite3* raw = nullptr;
    // 【逐行批注】 调用 expect、sqlite3_open 完成本行所需操作：expect(sqlite3_open(path.u8string().c_str(), &raw) == SQLITE_OK, "应打开…。
    expect(sqlite3_open(path.u8string().c_str(), &raw) == SQLITE_OK, "应打开迁移测试数据库");
    // 【逐行批注】 判断条件 (raw) {，成立时执行对应分支。
    if (raw) {
        // 【逐行批注】 计算右侧结果并写入 expect(sqlite3_exec(raw, "PRAGMA user_version，更新当前运行状态。
        expect(sqlite3_exec(raw, "PRAGMA user_version=2", nullptr, nullptr, nullptr) == SQLITE_OK,
            // 【逐行批注】 执行当前声明或表达式："应模拟旧数据库版本");。
            "应模拟旧数据库版本");
        // 【逐行批注】 调用 sqlite3_close 完成本行所需操作：sqlite3_close(raw);。
        sqlite3_close(raw);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：database.open(path.u8string());。
    database.open(path.u8string());
    // 【逐行批注】 执行当前声明或表达式：database.migrate();。
    database.migrate();
    // 【逐行批注】 计算右侧结果并写入 auto migrated，更新当前运行状态。
    const auto migrated = database.getLearningSession(legacy.id);
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(migrated && migrated->source == "legacy" && migrated->fallback…。
    expect(migrated && migrated->source == "legacy" && migrated->fallbackUsed == 1,
        // 【逐行批注】 执行当前声明或表达式："旧课堂正文应标记为待重新生成");。
        "旧课堂正文应标记为待重新生成");
    // 【逐行批注】 执行当前声明或表达式：database.close();。
    database.close();
    // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(path, filesystemError);。
    std::filesystem::remove(path, filesystemError);
    // 【逐行批注】 结束当前函数并返回结果：return failures == 0 ? 0 : 1;。
    return failures == 0 ? 0 : 1;
// 【逐行批注】 结束当前代码块。
}

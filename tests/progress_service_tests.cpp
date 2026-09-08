#include "db.hpp"
#include "progress_service.hpp"

#include <filesystem>
#include <iostream>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "失败: " << message << '\n';
        ++failures;
    }
}

}  // namespace

int main() {
    const auto path = std::filesystem::temp_directory_path() / "gangyiAI-progress-service-tests.db";
    std::error_code filesystemError;
    std::filesystem::remove(path, filesystemError);

    gangyi::Database database;
    database.open(path.u8string());
    database.migrate();

    gangyi::Course course;
    course.id = "course-1";
    course.anonymousId = "anonymous-1";
    course.goal = "掌握函数单调性";
    course.mode = "deep";
    course.title = "函数单调性课程";
    course.createdAt = "2026-09-07T00:00:00Z";
    course.updatedAt = course.createdAt;
    expect(database.insert(course), "应创建测试课程");

    gangyi::CourseSnapshot snapshot;
    snapshot.id = "snapshot-1";
    snapshot.courseId = course.id;
    nlohmann::json payload;
    payload["roadmap"] = nlohmann::json::array();
    payload["roadmap"].push_back({{"tasks", nlohmann::json::array({"任务一", "任务二"})},
                                   {"steps", nlohmann::json::array({{{"title", "步骤一"}}})}});
    payload["courseStructure"] = nlohmann::json::array();
    payload["courseStructure"].push_back({{"topics", nlohmann::json::array({"主题一", "主题二"})}});
    snapshot.payload = payload.dump();
    snapshot.createdAt = course.createdAt;
    expect(database.insert(snapshot), "应创建课程快照");

    auto saved = gangyi::saveTaskProgress(database, {{"courseId", course.id}, {"anonymousId", "anonymous-1"},
        {"goal", course.goal}, {"mode", "deep"}, {"phaseIndex", 1}, {"phaseName", "基础阶段"},
        {"taskIndex", 0}, {"taskTitle", "任务一"}, {"status", "completed"}});
    expect(saved.ok && saved.item.value("status", "") == "completed", "应保存任务完成状态");

    saved = gangyi::saveTaskProgress(database, {{"courseId", course.id}, {"phaseIndex", 1},
        {"taskIndex", 1}, {"status", "非法状态"}});
    expect(saved.ok && saved.item.value("status", "") == "not_started", "非法任务状态应归一化");
    expect(database.listTaskProgress().size() == 2, "任务进度应按课程、阶段和序号 upsert");

    gangyi::LearningStepProgress step;
    step.id = "step-1";
    step.courseId = course.id;
    step.anonymousId = "anonymous-1";
    step.goal = course.goal;
    step.mode = "deep";
    step.phaseIndex = 1;
    step.phaseName = "基础阶段";
    step.stepIndex = 0;
    step.stepTitle = "步骤一";
    step.status = "understood";
    expect(database.insert(step), "应创建步骤进度");

    gangyi::LearningCardProgress card;
    card.id = "card-1";
    card.courseId = course.id;
    card.anonymousId = "anonymous-1";
    card.goal = course.goal;
    card.mode = "deep";
    card.phaseIndex = 1;
    card.phaseName = "基础阶段";
    card.topicIndex = 1;
    card.topicTitle = "主题一";
    card.status = "completed";
    expect(database.insert(card), "应创建学习卡进度");

    auto progress = gangyi::recomputeCourseProgress(database, course.id, "anonymous-1", course.goal);
    expect(progress.has_value(), "应重算课程进度");
    expect(progress && progress->value("completedCount", 0) == 3, "应合并任务、步骤和学习卡完成数");
    expect(progress && progress->value("totalCount", 0) == 5, "应按课程快照计算总项目数");
    expect(progress && progress->value("overallPercent", 0) == 60, "课程完成度应正确取整");

    expect(gangyi::updateLastVisited(database, course.id, "anonymous-1", course.goal, "deep",
        {{"lastVisitedUrl", "/learn?courseId=course-1"}, {"lastPageType", "learn"},
         {"lastPhaseIndex", 1}, {"lastTopicIndex", 1}}), "应保存站内学习断点");
    auto stored = database.findProgressByCourseId(course.id);
    expect(stored && stored->lastVisitedUrl.value_or("") == "/learn?courseId=course-1", "应恢复站内断点地址");
    expect(gangyi::updateLastVisited(database, course.id, "anonymous-1", course.goal, "deep",
        {{"lastVisitedUrl", "//example.com/unsafe"}}), "忽略外部地址时仍应保留进度记录");
    stored = database.findProgressByCourseId(course.id);
    expect(stored && stored->lastVisitedUrl.value_or("") == "/learn?courseId=course-1", "不得保存协议相对外部地址");
    expect(gangyi::updateLastVisited(database, course.id, "anonymous-1", course.goal, "deep",
        {{"lastVisitedUrl", "/learn?next=https://example.com/unsafe"}}), "忽略外部跳转参数时仍应保留进度记录");
    stored = database.findProgressByCourseId(course.id);
    expect(stored && stored->lastVisitedUrl.value_or("") == "/learn?courseId=course-1", "不得保存包含外部双斜杠的地址");

    progress = gangyi::resetCourseProgress(database, course.id, "anonymous-1", course.goal);
    expect(progress && progress->value("completedCount", -1) == 0, "重置后完成数应归零");
    expect(database.listTaskProgress().empty() && database.listLearningStepProgress().empty() &&
        database.listLearningCardProgress().empty(), "重置应清理三类学习进度");

    database.close();
    std::filesystem::remove(path, filesystemError);
    return failures == 0 ? 0 : 1;
}

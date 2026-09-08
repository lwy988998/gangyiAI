#include "progress_service.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>

namespace gangyi {
namespace {

using json = nlohmann::json;

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string bounded(const std::string& value, size_t limit) {
    const std::string result = trim(value);
    return result.substr(0, std::min(limit, result.size()));
}

std::string textValue(const json& object, const char* key) {
    if (!object.is_object() || !object.contains(key) || object[key].is_null()) return {};
    return object[key].is_string() ? object[key].get<std::string>() : object[key].dump();
}

int intValue(const json& object, const char* key, int fallback = 0) {
    if (!object.is_object() || !object.contains(key)) return fallback;
    try {
        if (object[key].is_number_integer()) return object[key].get<int>();
        if (object[key].is_number()) return static_cast<int>(object[key].get<double>());
        if (object[key].is_string()) return std::stoi(object[key].get<std::string>());
    } catch (...) {}
    return fallback;
}

std::optional<std::string> optionalText(const std::string& value) {
    return value.empty() ? std::nullopt : std::optional<std::string>(value);
}

std::string randomId() {
    static std::mt19937_64 random(std::random_device{}());
    std::ostringstream output;
    output << std::hex << std::setfill('0') << std::setw(16) << random() << std::setw(16) << random();
    return output.str();
}

std::string nowIso8601() {
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm value{};
#ifdef _WIN32
    gmtime_s(&value, &now);
#else
    gmtime_r(&now, &value);
#endif
    std::ostringstream output;
    output << std::put_time(&value, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

size_t arrayLength(const json& object, const char* key) {
    return object.is_object() && object.contains(key) && object[key].is_array() ? object[key].size() : 0;
}

template <typename Callback>
void forEachStage(const json& value, Callback&& callback) {
    if (value.is_array()) {
        for (const auto& stage : value) callback(stage);
    } else if (value.is_object()) {
        for (const auto& item : value.items()) callback(item.value());
    }
}

size_t stageItemCount(const json& payload, const char* container,
                      const std::initializer_list<const char*>& keys) {
    if (!payload.is_object() || !payload.contains(container)) return 0;
    size_t total = 0;
    forEachStage(payload[container], [&](const json& stage) {
        for (const char* key : keys) {
            const size_t count = arrayLength(stage, key);
            if (count > 0) {
                total += count;
                break;
            }
        }
    });
    return total;
}

int completedTaskCount(Database& db, const std::string& courseId) {
    int count = 0;
    for (const auto& item : db.listTaskProgress())
        if (item.courseId.value_or("") == courseId && item.status == "completed") ++count;
    return count;
}

int understoodStepCount(Database& db, const std::string& courseId) {
    int count = 0;
    for (const auto& item : db.listLearningStepProgress())
        if (item.courseId.value_or("") == courseId && item.status == "understood") ++count;
    return count;
}

int completedCardCount(Database& db, const std::string& courseId) {
    int count = 0;
    for (const auto& item : db.listLearningCardProgress())
        if (item.courseId.value_or("") == courseId && item.status == "completed") ++count;
    return count;
}

json progressJson(const CourseProgress& value) {
    return {{"overallPercent", value.overallPercent}, {"completedCount", value.completedCount},
            {"totalCount", value.totalCount}, {"lastVisitedUrl", value.lastVisitedUrl.value_or("")},
            {"lastPageType", value.lastPageType.value_or("")},
            {"lastPhaseIndex", value.lastPhaseIndex ? json(*value.lastPhaseIndex) : json(nullptr)},
            {"lastPhaseName", value.lastPhaseName.value_or("")},
            {"lastTopicIndex", value.lastTopicIndex ? json(*value.lastTopicIndex) : json(nullptr)},
            {"lastTopicTitle", value.lastTopicTitle.value_or("")}, {"updatedAt", value.updatedAt}};
}

}  // namespace

std::string normalizeCardStatus(const std::string& status) {
    const std::string value = trim(status);
    return value == "not_started" || value == "in_progress" || value == "completed"
        ? value : "not_started";
}

std::string normalizeStepStatus(const std::string& status) {
    const std::string value = trim(status);
    return value == "unset" || value == "understood" || value == "review" ? value : "unset";
}

ProgressSaveResult saveTaskProgress(Database& db, const json& body) {
    try {
        const std::string courseId = bounded(textValue(body, "courseId"), 500);
        const int phaseIndex = intValue(body, "phaseIndex");
        const int taskIndex = intValue(body, "taskIndex", -1);
        if (courseId.empty() || phaseIndex < 1 || taskIndex < 0) return {};

        auto existing = db.findTaskProgress(courseId, phaseIndex, taskIndex);
        TaskProgress value = existing.value_or(TaskProgress{});
        if (value.id.empty()) value.id = randomId();
        value.courseId = courseId;
        value.anonymousId = optionalText(bounded(textValue(body, "anonymousId"), 500));
        value.goal = bounded(textValue(body, "goal"), 500);
        value.mode = optionalText(bounded(textValue(body, "mode"), 40));
        value.phaseIndex = phaseIndex;
        value.phaseName = bounded(textValue(body, "phaseName"), 500);
        if (value.phaseName.empty()) value.phaseName = "阶段" + std::to_string(phaseIndex);
        value.taskIndex = taskIndex;
        value.taskTitle = bounded(textValue(body, "taskTitle"), 500);
        if (value.taskTitle.empty()) value.taskTitle = "任务" + std::to_string(taskIndex + 1);
        value.status = normalizeCardStatus(textValue(body, "status"));
        const bool saved = existing ? db.update(value) : db.insert(value);
        if (!saved) return {};
        return {true, {{"id", value.id}, {"courseId", courseId}, {"anonymousId", value.anonymousId.value_or("")},
            {"goal", value.goal}, {"mode", value.mode.value_or("")}, {"phaseIndex", phaseIndex},
            {"phaseName", value.phaseName}, {"taskIndex", taskIndex}, {"taskTitle", value.taskTitle},
            {"status", value.status}}};
    } catch (...) {
        return {};
    }
}

std::optional<json> recomputeCourseProgress(Database& db, const std::string& courseId,
                                             const std::string& anonymousId,
                                             const std::string& goal) {
    try {
        const auto course = db.getCourse(courseId);
        if (!course) return std::nullopt;
        const auto snapshots = db.findSnapshotsByCourseId(courseId);
        if (snapshots.empty()) return std::nullopt;
        const auto latest = std::max_element(snapshots.begin(), snapshots.end(), [](const auto& left, const auto& right) {
            return left.version != right.version ? left.version < right.version : left.createdAt < right.createdAt;
        });
        const json payload = json::parse(latest->payload, nullptr, false);
        if (!payload.is_object()) return std::nullopt;

        size_t cards = stageItemCount(payload, "courseStructure", {"topics"});
        if (cards == 0) cards = stageItemCount(payload, "roadmap", {"topics", "learningCards", "cards"});
        const size_t tasks = stageItemCount(payload, "roadmap", {"tasks", "checklist", "practices", "projects"});
        const size_t steps = stageItemCount(payload, "roadmap", {"steps"});
        const int completed = completedTaskCount(db, courseId) + understoodStepCount(db, courseId) +
            completedCardCount(db, courseId);
        const int total = std::max(completed, static_cast<int>(cards + tasks + steps));
        const int percent = total == 0 ? 0 : std::clamp(static_cast<int>(std::round(completed * 100.0 / total)), 0, 100);

        auto existing = db.findProgressByCourseId(courseId);
        CourseProgress value = existing.value_or(CourseProgress{});
        if (value.id.empty()) value.id = randomId();
        value.courseId = courseId;
        value.anonymousId = optionalText(bounded(anonymousId, 500));
        value.userId = course->userId;
        value.goal = bounded(goal, 500).empty() ? course->goal : bounded(goal, 500);
        value.mode = course->mode;
        value.overallPercent = percent;
        value.completedCount = completed;
        value.totalCount = total;
        value.updatedAt = nowIso8601();
        if (value.createdAt.empty()) value.createdAt = value.updatedAt;
        const bool saved = existing ? db.update(value) : db.insert(value);
        if (!saved) return std::nullopt;
        return progressJson(value);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> resetCourseProgress(Database& db, const std::string& courseId,
                                        const std::string& anonymousId, const std::string& goal) {
    try {
        for (const auto& item : db.listLearningCardProgress())
            if (item.courseId.value_or("") == courseId) db.deleteLearningCardProgress(item.id);
        for (const auto& item : db.listLearningStepProgress())
            if (item.courseId.value_or("") == courseId) db.deleteLearningStepProgress(item.id);
        for (const auto& item : db.listTaskProgress())
            if (item.courseId.value_or("") == courseId) db.deleteTaskProgress(item.id);
        if (const auto progress = db.findProgressByCourseId(courseId)) db.deleteCourseProgress(progress->id);
        return recomputeCourseProgress(db, courseId, anonymousId, goal);
    } catch (...) {
        return std::nullopt;
    }
}

bool updateLastVisited(Database& db, const std::string& courseId, const std::string& anonymousId,
                       const std::string& goal, const std::string& mode, const json& body) {
    try {
        const auto course = db.getCourse(courseId);
        if (!course) return false;
        auto existing = db.findProgressByCourseId(courseId);
        CourseProgress value = existing.value_or(CourseProgress{});
        if (value.id.empty()) value.id = randomId();
        value.courseId = courseId;
        value.anonymousId = optionalText(bounded(anonymousId, 500));
        value.userId = course->userId;
        value.goal = bounded(goal, 500).empty() ? course->goal : bounded(goal, 500);
        value.mode = optionalText(bounded(mode, 40));
        const std::string url = textValue(body, "lastVisitedUrl");
        if (!url.empty() && url.front() == '/' && url.find("//") == std::string::npos)
            value.lastVisitedUrl = bounded(url, 1200);
        value.lastPageType = optionalText(bounded(textValue(body, "lastPageType"), 40));
        if (body.contains("lastPhaseIndex")) value.lastPhaseIndex = intValue(body, "lastPhaseIndex");
        value.lastPhaseName = optionalText(bounded(textValue(body, "lastPhaseName"), 500));
        if (body.contains("lastTopicIndex")) value.lastTopicIndex = intValue(body, "lastTopicIndex");
        value.lastTopicTitle = optionalText(bounded(textValue(body, "lastTopicTitle"), 500));
        value.updatedAt = nowIso8601();
        if (value.createdAt.empty()) value.createdAt = value.updatedAt;
        return existing ? db.update(value) : db.insert(value);
    } catch (...) {
        return false;
    }
}

}  // namespace gangyi

#include "progress_service.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>

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

std::string text(const json& object, const char* key) {
    if (!object.is_object() || !object.contains(key) || object[key].is_null()) return {};
    if (object[key].is_string()) return object[key].get<std::string>();
    return object[key].dump();
}

int integer(const json& object, const char* key, int fallback = 0) {
    if (!object.is_object() || !object.contains(key)) return fallback;
    const auto& value = object[key];
    try {
        if (value.is_number_integer()) return value.get<int>();
        if (value.is_number()) return static_cast<int>(value.get<double>());
        if (value.is_string()) return std::stoi(value.get<std::string>());
    } catch (...) {}
    return fallback;
}

std::string id() {
    static std::mt19937_64 rng(std::random_device{}());
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (int i = 0; i < 2; ++i) out << std::setw(16) << rng();
    return out.str();
}

std::string now() {
    const auto stamp = std::chrono::system_clock::now().time_since_epoch();
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(stamp).count();
    return std::to_string(seconds);
}

std::optional<std::string> optionalText(const std::string& value) {
    return value.empty() ? std::nullopt : std::optional<std::string>(value);
}

json progressJson(const TaskProgress& v) {
    return {{"id", v.id}, {"courseId", v.courseId.value_or("")}, {"anonymousId", v.anonymousId.value_or("")},
            {"goal", v.goal}, {"mode", v.mode.value_or("")}, {"phaseIndex", v.phaseIndex}, {"phaseName", v.phaseName},
            {"taskIndex", v.taskIndex}, {"taskTitle", v.taskTitle}, {"status", v.status}};
}
json progressJson(const LearningStepProgress& v) {
    return {{"id", v.id}, {"courseId", v.courseId.value_or("")}, {"anonymousId", v.anonymousId.value_or("")},
            {"goal", v.goal}, {"mode", v.mode.value_or("")}, {"phaseIndex", v.phaseIndex}, {"phaseName", v.phaseName},
            {"stepIndex", v.stepIndex}, {"stepTitle", v.stepTitle}, {"status", v.status}};
}
json progressJson(const LearningCardProgress& v) {
    return {{"id", v.id}, {"courseId", v.courseId.value_or("")}, {"anonymousId", v.anonymousId.value_or("")},
            {"goal", v.goal}, {"mode", v.mode.value_or("")}, {"phaseIndex", v.phaseIndex}, {"phaseName", v.phaseName},
            {"topicIndex", v.topicIndex}, {"topicTitle", v.topicTitle}, {"status", v.status}};
}

bool sameIdentity(const std::optional<std::string>& anonymousId, const std::string& goal,
                  const std::optional<std::string>& mode, int phase, int index,
                  const std::optional<std::string>& rowAnonymous, const std::string& rowGoal,
                  const std::optional<std::string>& rowMode, int rowPhase, int rowIndex) {
    return anonymousId.value_or("") == rowAnonymous.value_or("") && goal == rowGoal &&
           mode.value_or("") == rowMode.value_or("") && phase == rowPhase && index == rowIndex;
}

template <typename T>
ProgressSaveResult saveProgress(Database& db, const json& body, int index, const std::string& title,
                                const std::string& status, const std::string& phaseName, bool hasCourse,
                                std::optional<T> existing,
                                const std::function<std::vector<T>()>& list,
                                const std::function<bool(const T&)>& insert,
                                const std::function<bool(const T&)>& update,
                                const std::function<json(const T&)>& toJson) {
    try {
        T value = existing.value_or(T{});
        if (value.id.empty()) value.id = id();
        value.courseId = optionalText(bounded(text(body, "courseId"), 500));
        value.anonymousId = optionalText(bounded(text(body, "anonymousId"), 500));
        value.goal = bounded(text(body, "goal"), 500);
        value.mode = optionalText(bounded(text(body, "mode"), 40));
        value.phaseIndex = integer(body, "phaseIndex");
        value.phaseName = phaseName;
        value.goal = bounded(value.goal, 500);
        if constexpr (std::is_same_v<T, TaskProgress>) { value.taskIndex = index; value.taskTitle = title; }
        else if constexpr (std::is_same_v<T, LearningStepProgress>) { value.stepIndex = index; value.stepTitle = title; }
        else { value.topicIndex = index; value.topicTitle = title; }
        value.status = status;
        const bool saved = existing ? update(value) : insert(value);
        return {saved, saved ? toJson(value) : json::object()};
    } catch (...) { return {false, json::object()}; }
}

size_t arrayLength(const json& object, const char* key) {
    return object.is_object() && object.contains(key) && object[key].is_array() ? object[key].size() : 0;
}

template <typename F>
void forEachStage(const json& value, F&& fn) {
    if (value.is_array()) for (const auto& stage : value) fn(stage);
    else if (value.is_object()) for (const auto& item : value.items()) fn(item.value());
}

size_t stageCount(const json& payload, const char* container, const std::vector<const char*>& keys) {
    size_t total = 0;
    if (!payload.is_object() || !payload.contains(container)) return 0;
    forEachStage(payload[container], [&](const json& stage) {
        for (const char* key : keys) {
            const size_t n = arrayLength(stage, key);
            if (n != 0) { total += n; break; }
        }
    });
    return total;
}

std::string sessionUpdatedAt(const LearningSession& value) {
    return value.fallbackUsed ? "" : "";
}

json courseProgressJson(const CourseProgress& value) {
    return {{"overallPercent", value.overallPercent}, {"completedCount", value.completedCount},
            {"totalCount", value.totalCount}, {"lastVisitedUrl", value.lastVisitedUrl.value_or("")},
            {"lastPageType", value.lastPageType.value_or("")},
            {"lastPhaseIndex", value.lastPhaseIndex ? json(*value.lastPhaseIndex) : json(nullptr)},
            {"lastPhaseName", value.lastPhaseName.value_or("")},
            {"lastTopicIndex", value.lastTopicIndex ? json(*value.lastTopicIndex) : json(nullptr)},
            {"lastTopicTitle", value.lastTopicTitle.value_or("")}, {"updatedAt", value.updatedAt}};
}

} // namespace

std::string normalizeCardStatus(const std::string& status) {
    const std::string value = trim(status);
    return value == "in_progress" || value == "completed" || value == "not_started" ? value : "not_started";
}

std::string normalizeStepStatus(const std::string& status) {
    const std::string value = trim(status);
    return value == "understood" || value == "review" || value == "unset" ? value : "unset";
}

ProgressSaveResult saveLearningCardProgress(Database& db, const json& body) {
    try {
        const int phase = integer(body, "phaseIndex"), index = integer(body, "topicIndex");
        const auto course = optionalText(bounded(text(body, "courseId"), 500));
        const auto anonymous = optionalText(bounded(text(body, "anonymousId"), 500));
        const std::string goal = bounded(text(body, "goal"), 500), mode = bounded(text(body, "mode"), 40);
        std::optional<LearningCardProgress> found = course ? db.findLearningCardProgress(*course, phase, index) : std::nullopt;
        if (!found) for (const auto& row : db.listLearningCardProgress())
            if (!course && sameIdentity(anonymous, goal, optionalText(mode), phase, index, row.anonymousId, row.goal, row.mode, row.phaseIndex, row.topicIndex)) { found = row; break; }
        const std::string name = bounded(text(body, "phaseName"), 500).empty() ? "阶段" + std::to_string(phase) : bounded(text(body, "phaseName"), 500);
        return saveProgress<LearningCardProgress>(db, body, index, bounded(text(body, "topicTitle"), 500), normalizeCardStatus(text(body, "status")), name, static_cast<bool>(course), found,
            [&] { return db.listLearningCardProgress(); }, [&](const auto& v) { return db.insert(v); }, [&](const auto& v) { return db.update(v); },
            [](const LearningCardProgress& v) { return progressJson(v); });
    } catch (...) { return {false, json::object()}; }
}

ProgressSaveResult saveLearningStepProgress(Database& db, const json& body) {
    try {
        const int phase = integer(body, "phaseIndex"), index = integer(body, "stepIndex");
        const auto course = optionalText(bounded(text(body, "courseId"), 500));
        const auto anonymous = optionalText(bounded(text(body, "anonymousId"), 500));
        const std::string goal = bounded(text(body, "goal"), 500), mode = bounded(text(body, "mode"), 40);
        std::optional<LearningStepProgress> found = course ? db.findLearningStepProgress(*course, phase, index) : std::nullopt;
        if (!found) for (const auto& row : db.listLearningStepProgress())
            if (!course && sameIdentity(anonymous, goal, optionalText(mode), phase, index, row.anonymousId, row.goal, row.mode, row.phaseIndex, row.stepIndex)) { found = row; break; }
        const std::string name = bounded(text(body, "phaseName"), 500).empty() ? "阶段" + std::to_string(phase) : bounded(text(body, "phaseName"), 500);
        return saveProgress<LearningStepProgress>(db, body, index, bounded(text(body, "stepTitle"), 500), normalizeStepStatus(text(body, "status")), name, static_cast<bool>(course), found,
            [&] { return db.listLearningStepProgress(); }, [&](const auto& v) { return db.insert(v); }, [&](const auto& v) { return db.update(v); },
            [](const LearningStepProgress& v) { return progressJson(v); });
    } catch (...) { return {false, json::object()}; }
}

ProgressSaveResult saveTaskProgress(Database& db, const json& body) {
    try {
        const int phase = integer(body, "phaseIndex"), index = integer(body, "taskIndex");
        const auto course = optionalText(bounded(text(body, "courseId"), 500));
        const auto anonymous = optionalText(bounded(text(body, "anonymousId"), 500));
        const std::string goal = bounded(text(body, "goal"), 500), mode = bounded(text(body, "mode"), 40);
        std::optional<TaskProgress> found = course ? db.findTaskProgress(*course, phase, index) : std::nullopt;
        if (!found) for (const auto& row : db.listTaskProgress())
            if (!course && sameIdentity(anonymous, goal, optionalText(mode), phase, index, row.anonymousId, row.goal, row.mode, row.phaseIndex, row.taskIndex)) { found = row; break; }
        const std::string name = bounded(text(body, "phaseName"), 500).empty() ? "阶段" + std::to_string(phase) : bounded(text(body, "phaseName"), 500);
        return saveProgress<TaskProgress>(db, body, index, bounded(text(body, "taskTitle"), 500), normalizeCardStatus(text(body, "status")), name, static_cast<bool>(course), found,
            [&] { return db.listTaskProgress(); }, [&](const auto& v) { return db.insert(v); }, [&](const auto& v) { return db.update(v); },
            [](const TaskProgress& v) { return progressJson(v); });
    } catch (...) { return {false, json::object()}; }
}

int countCompletedCards(Database& db, const std::string& courseId) { try { int n=0; for (const auto& v:db.listLearningCardProgress()) if (v.courseId.value_or("")==courseId && v.status=="completed") ++n; return n; } catch (...) { return 0; } }
int countUnderstoodSteps(Database& db, const std::string& courseId) { try { int n=0; for (const auto& v:db.listLearningStepProgress()) if (v.courseId.value_or("")==courseId && v.status=="understood") ++n; return n; } catch (...) { return 0; } }
int countCompletedTasks(Database& db, const std::string& courseId) { try { int n=0; for (const auto& v:db.listTaskProgress()) if (v.courseId.value_or("")==courseId && v.status=="completed") ++n; return n; } catch (...) { return 0; } }

std::optional<json> recomputeCourseProgress(Database& db, const std::string& courseId, const std::string& anonymousId, const std::string& goal) {
    try {
        const auto course = db.getCourse(courseId);
        if (!course) return std::nullopt;
        const auto snapshots = db.findSnapshotsByCourseId(courseId);
        if (snapshots.empty()) return std::nullopt;
        const auto latest = std::max_element(snapshots.begin(), snapshots.end(), [](const auto& a, const auto& b) {
            if (a.version != b.version) return a.version < b.version;
            return a.createdAt < b.createdAt;
        });
        const json payload = json::parse(latest->payload, nullptr, false);
        if (!payload.is_object()) return std::nullopt;
        size_t tasks = stageCount(payload, "roadmap", {"tasks", "checklist", "practices", "projects"});
        size_t cards = stageCount(payload, "courseStructure", {"topics"});
        if (cards == 0) cards = stageCount(payload, "roadmap", {"topics", "learningCards", "cards"});
        const size_t steps = stageCount(payload, "roadmap", {"steps"});
        const int total = static_cast<int>(tasks + cards + steps);
        const int completed = countCompletedTasks(db, courseId) + countUnderstoodSteps(db, courseId) + countCompletedCards(db, courseId);
        const int percent = total == 0 ? 0 : std::max(0, std::min(100, static_cast<int>(std::round(completed * 100.0 / total))));
        CourseProgress value;
        if (const auto existing = db.findProgressByCourseId(courseId)) value = *existing; else value.id = id();
        value.courseId = courseId;
        value.anonymousId = optionalText(bounded(anonymousId, 500));
        value.userId = course->userId;
        value.goal = bounded(goal, 500);
        if (value.goal.empty()) value.goal = course->goal;
        value.mode = optionalText(course->mode);
        value.overallPercent = percent; value.completedCount = completed; value.totalCount = total; value.updatedAt = now(); if (value.createdAt.empty()) value.createdAt = value.updatedAt;
        if (!db.update(value) && !db.findProgressByCourseId(courseId)) if (!db.insert(value)) return std::nullopt;
        return courseProgressJson(value);
    } catch (...) { return std::nullopt; }
}

bool updateLastVisited(Database& db, const std::string& courseId, const std::string& anonymousId, const std::string& goal, const std::string& mode, const json& body) {
    try {
        auto existing = db.findProgressByCourseId(courseId);
        if (!existing && courseId.empty() && bounded(goal, 500).empty()) return false;
        CourseProgress value = existing.value_or(CourseProgress{}); if (value.id.empty()) value.id=id(); value.courseId=courseId; value.anonymousId=optionalText(bounded(anonymousId,500)); value.goal=bounded(goal,500); value.mode=optionalText(bounded(mode,40));
        const std::string url = text(body, "lastVisitedUrl"); if (!url.empty() && url.front()=='/' && url.find("//")==std::string::npos) value.lastVisitedUrl=url.substr(0,1200);
        value.lastPageType=optionalText(bounded(text(body,"lastPageType"),40)); value.lastPhaseIndex=integer(body,"lastPhaseIndex"); value.lastPhaseName=optionalText(bounded(text(body,"lastPhaseName"),500)); value.lastTopicIndex=integer(body,"lastTopicIndex"); value.lastTopicTitle=optionalText(bounded(text(body,"lastTopicTitle"),500)); value.updatedAt=now(); if(value.createdAt.empty()) value.createdAt=value.updatedAt;
        return existing ? db.update(value) : db.insert(value);
    } catch (...) { return false; }
}

std::optional<json> findLearningSession(Database& db, const std::string& courseId, const std::string& anonymousId, const std::string& goal, const std::string& mode, int phaseIndex, int topicIndex) {
    try {
        std::optional<LearningSession> found = courseId.empty() ? std::nullopt : db.findLearningSession(courseId, phaseIndex, topicIndex);
        if (courseId.empty()) for (const auto& row : db.listLearningSessions()) if (row.anonymousId.value_or("")==anonymousId && row.goal==goal && row.mode.value_or("")==mode && row.phaseIndex==phaseIndex && row.topicIndex==topicIndex && (!found || row.id>found->id)) found=row;
        if (!found) return std::nullopt;
        return json{{"id",found->id},{"title",found->title},{"summary",found->summary.value_or("")},{"content",found->content},{"references",found->references.value_or("")},{"fallbackUsed",found->fallbackUsed != 0},{"source",found->source},{"updatedAt",sessionUpdatedAt(*found)}};
    } catch (...) { return std::nullopt; }
}

bool upsertLearningSession(Database& db, const json& body) {
    try {
        LearningSession value; const std::string courseId=bounded(text(body,"courseId"),500); auto found=courseId.empty()?std::optional<LearningSession>{}:db.findLearningSession(courseId,integer(body,"phaseIndex"),integer(body,"topicIndex")); if(!found) for(const auto& row:db.listLearningSessions()) if(courseId.empty() && row.anonymousId.value_or("")==bounded(text(body,"anonymousId"),500) && row.goal==bounded(text(body,"goal"),500) && row.mode.value_or("")==bounded(text(body,"mode"),40) && row.phaseIndex==integer(body,"phaseIndex") && row.topicIndex==integer(body,"topicIndex")){found=row;break;} if(found)value=*found; else value.id=id();
        value.courseId=optionalText(courseId); value.anonymousId=optionalText(bounded(text(body,"anonymousId"),500)); value.goal=bounded(text(body,"goal"),500); value.mode=optionalText(bounded(text(body,"mode"),40)); value.phaseIndex=integer(body,"phaseIndex"); value.phaseName=bounded(text(body,"phaseName"),500); value.topicIndex=integer(body,"topicIndex"); value.topicTitle=bounded(text(body,"topicTitle"),500); value.title=bounded(text(body,"title"),500); value.summary=optionalText(bounded(text(body,"summary"),500)); value.searchQuery=optionalText(bounded(text(body,"searchQuery"),500));
        json content = body.contains("content") ? body["content"] : json::object();
        if (content.is_string()) content = json::parse(content.get<std::string>(), nullptr, false);
        if (!content.is_object()) return false;
        auto trimArray = [&](const char* key, size_t max) {
            if (content.contains(key) && content[key].is_array() && content[key].size() > max) {
                content[key] = json(content[key].begin(), content[key].begin() + static_cast<std::ptrdiff_t>(max));
            }
        };
        trimArray("lessonSteps", 8); trimArray("examples", 6); trimArray("practice", 8); trimArray("references", 8);
        for (auto& item : content.items()) {
            if (item.value().is_array() && item.value().size() > 50) {
                item.value() = json(item.value().begin(), item.value().begin() + 50);
            }
        }
        while (content.dump().size() > 900000) {
            bool reduced = false;
            for (auto& item : content.items()) {
                if (item.value().is_array() && !item.value().empty()) {
                    item.value().erase(item.value().end() - 1);
                    reduced = true;
                    break;
                }
            }
            if (!reduced) break;
        }
        if (content.dump().size() > 900000) return false;
        value.content = content.dump();
        value.references = body.contains("references") ? optionalText(body["references"].dump()) : std::nullopt;
        value.fallbackUsed = body.value("fallbackUsed", false) ? 1 : 0;
        value.source = bounded(text(body, "source"), 40);
        if (value.source.empty()) value.source = "ai";
        return found ? db.update(value) : db.insert(value);
    } catch (...) { return false; }
}

} // namespace gangyi

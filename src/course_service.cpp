#include "course_service.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <limits>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace gangyi {
namespace {

using json = nlohmann::json;

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n\f\v");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n\f\v");
    return value.substr(first, last - first + 1);
}

std::string nowIso8601() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &time);
#else
    gmtime_r(&time, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

bool containsSensitiveKey(const std::string& key) {
    const std::string lowered;
    std::string lowerKey = key;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    static const std::vector<std::string> needles = {
        "apikey", "api_key", "secret", "token", "authorization", "base64", "image", "rawerror", "stack"
    };
    for (const auto& needle : needles) {
        if (lowerKey.find(needle) != std::string::npos) return true;
    }
    return false;
}

json clampString(const json& value, size_t maxLen) {
    if (!value.is_string()) return value;
    std::string str = value.get<std::string>();
    if (str.size() > maxLen) str.resize(maxLen);
    return str;
}

std::string safeString(const json& value) {
    if (!value.is_string()) return {};
    std::string str = trim(value.get<std::string>());
    if (str.size() > 10000) str.resize(10000);
    return str;
}

json sanitizeRecursive(const json& value, int depth);

json sanitizeArray(const json& value, int depth) {
    json out = json::array();
    if (!value.is_array()) return out;
    size_t count = 0;
    for (const auto& item : value) {
        if (count >= 1000) break;
        out.push_back(sanitizeRecursive(item, depth + 1));
        ++count;
    }
    return out;
}

json sanitizeResourceItem(const json& value, int depth) {
    if (!value.is_object()) return json::object();
    json out;
    out["name"] = clampString(value.value("name", ""), 300);
    out["type"] = clampString(value.value("type", ""), 100);
    out["difficulty"] = clampString(value.value("difficulty", ""), 100);
    out["free"] = value.value("free", true);
    out["description"] = clampString(value.value("description", ""), 500);
    out["href"] = clampString(value.value("href", value.value("url", "")), 1000);
    out["source"] = clampString(value.value("source", ""), 200);
    out["language"] = clampString(value.value("language", ""), 50);
    out["score"] = value.value("score", 70.0);
    out["reason"] = clampString(value.value("reason", ""), 500);
    return out;
}

json sanitizeObject(const json& value, int depth) {
    static const std::vector<std::string> whitelist = {
        "title", "duration", "summary", "courseIntro", "overview", "audience", "prerequisites", "outcome",
        "learningOutcomes", "slides", "mindMap", "roadmap", "courseStructure", "resources", "projects"
    };
    json out = json::object();
    if (depth > 0) {
        for (const auto& item : value.items()) {
            if (containsSensitiveKey(item.key())) continue;
            out[item.key()] = sanitizeRecursive(item.value(), depth + 1);
        }
        return out;
    }
    for (const auto& key : whitelist) {
        if (!value.contains(key)) continue;
        if (containsSensitiveKey(key)) continue;
        const auto& item = value.at(key);
        if (key == "resources" && item.is_array()) {
            json resources = json::array();
            size_t count = 0;
            for (const auto& resource : item) {
                if (count >= 12) break;
                if (!resource.is_object()) continue;
                std::string nameText;
                std::string hrefText;
                if (resource.contains("name") && resource["name"].is_string()) nameText = trim(resource["name"].get<std::string>());
                if (resource.contains("href") && resource["href"].is_string()) hrefText = trim(resource["href"].get<std::string>());
                if (hrefText.empty() && resource.contains("url") && resource["url"].is_string()) hrefText = trim(resource["url"].get<std::string>());
                if (nameText.empty() || hrefText.empty()) continue;
                resources.push_back(sanitizeResourceItem(resource, depth + 1));
                ++count;
            }
            out[key] = resources;
            continue;
        }
        if (item.is_string()) {
            out[key] = clampString(item, 10000);
        } else if (item.is_array()) {
            out[key] = sanitizeArray(item, depth + 1);
        } else if (item.is_object()) {
            json nested = json::object();
            for (const auto& nestedItem : item.items()) {
                if (containsSensitiveKey(nestedItem.key())) continue;
                nested[nestedItem.key()] = sanitizeRecursive(nestedItem.value(), depth + 1);
            }
            out[key] = nested;
        } else {
            out[key] = item;
        }
    }
    return out;
}

json sanitizeRecursive(const json& value, int depth) {
    if (depth > 8) return json::object();
    if (value.is_object()) return sanitizeObject(value, depth);
    if (value.is_array()) return sanitizeArray(value, depth);
    if (value.is_string()) return clampString(value, 10000);
    return value;
}

size_t jsonSize(const json& value) {
    try {
        return value.dump().size();
    } catch (...) {
        return 0;
    }
}

json clampLargePayload(json payload) {
    if (!payload.is_object()) return payload;
    if (jsonSize(payload) <= 900000) return payload;
    if (payload.contains("slides") && payload["slides"].is_array() && payload["slides"].size() > 20) payload["slides"].erase(payload["slides"].begin() + 20, payload["slides"].end());
    if (payload.contains("roadmap") && payload["roadmap"].is_array() && payload["roadmap"].size() > 12) payload["roadmap"].erase(payload["roadmap"].begin() + 12, payload["roadmap"].end());
    if (payload.contains("resources") && payload["resources"].is_array() && payload["resources"].size() > 8) payload["resources"].erase(payload["resources"].begin() + 8, payload["resources"].end());
    if (payload.contains("projects") && payload["projects"].is_array() && payload["projects"].size() > 12) payload["projects"].erase(payload["projects"].begin() + 12, payload["projects"].end());
    return payload;
}

std::optional<Course> pickCourseByIdentity(Database& db, const std::string& anonymousId, const std::string& userId, const std::string& goal) {
    const auto courses = db.listCourses();
    std::optional<Course> found;
    for (const auto& course : courses) {
        if (course.status != "active") continue;
        if (course.goal != goal) continue;
        bool match = false;
        if (!userId.empty() && course.userId && *course.userId == userId) match = true;
        if (!anonymousId.empty() && course.anonymousId && *course.anonymousId == anonymousId) match = true;
        if (!match) continue;
        if (!found || course.updatedAt > found->updatedAt) found = course;
    }
    return found;
}

std::string courseIdFromSnapshot(const std::optional<Course>& course) {
    return course ? course->id : std::string{};
}

}  // namespace

json sanitizeCoursePayload(const json& payload) {
    try {
        json sanitized = sanitizeRecursive(payload, 0);
        sanitized = clampLargePayload(std::move(sanitized));
        return sanitized;
    } catch (...) {
        return payload;
    }
}

std::optional<std::string> saveCourseSnapshot(Database& db, const std::string& anonymousId, const std::string& userId,
                                              const std::string& goal, const std::string& mode, const std::string& title,
                                              const std::string& summary, const std::string& source, const json& payload) {
    try {
        const std::string now = nowIso8601();
        const std::optional<Course> existing = pickCourseByIdentity(db, anonymousId, userId, goal);
        const json sanitized = sanitizeCoursePayload(payload);
        if (existing) {
            Course course = *existing;
            course.title = title;
            course.summary = summary;
            course.source = source;
            course.mode = mode;
            course.updatedAt = now;
            if (!db.update(course)) return std::nullopt;
            int maxVersion = 0;
            for (const auto& snapshot : db.findSnapshotsByCourseId(course.id)) {
                if (snapshot.version > maxVersion) maxVersion = snapshot.version;
            }
            CourseSnapshot snapshot;
            snapshot.id = "";
            snapshot.courseId = course.id;
            snapshot.version = maxVersion + 1;
            snapshot.payload = sanitized.dump();
            snapshot.createdAt = now;
            if (!db.insert(snapshot)) return std::nullopt;
            return course.id;
        }

        Course course;
        course.id = "";
        course.anonymousId = anonymousId.empty() ? std::nullopt : std::optional<std::string>(anonymousId);
        course.userId = userId.empty() ? std::nullopt : std::optional<std::string>(userId);
        course.goal = goal;
        course.mode = mode;
        course.title = title;
        course.summary = summary;
        course.source = source;
        course.status = "active";
        course.createdAt = now;
        course.updatedAt = now;
        if (!db.insert(course)) return std::nullopt;
        const auto inserted = pickCourseByIdentity(db, anonymousId, userId, goal);
        if (!inserted) return std::nullopt;
        CourseSnapshot snapshot;
        snapshot.id = "";
        snapshot.courseId = inserted->id;
        snapshot.version = 1;
        snapshot.payload = sanitized.dump();
        snapshot.createdAt = now;
        if (!db.insert(snapshot)) return std::nullopt;
        return inserted->id;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<CourseWithSnapshot> getCourseWithSnapshot(Database& db, const std::string& courseId) {
    try {
        const auto course = db.getCourse(courseId);
        if (!course) return std::nullopt;
        const auto snapshots = db.findSnapshotsByCourseId(course->id);
        if (snapshots.empty()) return std::nullopt;
        const auto latest = *std::max_element(snapshots.begin(), snapshots.end(), [](const CourseSnapshot& a, const CourseSnapshot& b) {
            if (a.version != b.version) return a.version < b.version;
            return a.createdAt < b.createdAt;
        });
        CourseWithSnapshot result{*course, json::parse(latest.payload, nullptr, false)};
        if (result.payload.is_discarded()) return std::nullopt;
        return result;
    } catch (...) {
        return std::nullopt;
    }
}

std::vector<Course> listCoursesForIdentity(Database& db, const std::string& userId, const std::string& anonymousId,
                                           size_t limit, size_t offset) {
    try {
        std::vector<Course> courses;
        for (const auto& course : db.listCourses()) {
            if (course.status != "active") continue;
            bool match = false;
            if (!userId.empty() && course.userId && *course.userId == userId) match = true;
            if (!anonymousId.empty() && course.anonymousId && *course.anonymousId == anonymousId) match = true;
            if (!match) continue;
            courses.push_back(course);
        }
        std::sort(courses.begin(), courses.end(), [](const Course& a, const Course& b) {
            if (a.updatedAt != b.updatedAt) return a.updatedAt > b.updatedAt;
            return a.id < b.id;
        });
        if (offset >= courses.size()) return {};
        const size_t end = std::min(courses.size(), offset + limit);
        return std::vector<Course>(courses.begin() + static_cast<std::ptrdiff_t>(offset), courses.begin() + static_cast<std::ptrdiff_t>(end));
    } catch (...) {
        return {};
    }
}

bool deleteCourseForIdentity(Database& db, const std::string& courseId, const std::string& userId,
                             const std::string& anonymousId) {
    try {
        const auto course = db.getCourse(courseId);
        if (!course || course->status != "active") return false;
        const bool ownedByUser = !userId.empty() && course->userId && *course->userId == userId;
        const bool ownedByAnonymous = !anonymousId.empty() && course->anonymousId && *course->anonymousId == anonymousId;
        if (!ownedByUser && !ownedByAnonymous) return false;

        for (const auto& item : db.findSnapshotsByCourseId(courseId)) db.deleteCourseSnapshot(item.id);
        if (const auto item = db.findProgressByCourseId(courseId)) db.deleteCourseProgress(item->id);
        for (const auto& item : db.listTaskProgress()) if (item.courseId.value_or("") == courseId) db.deleteTaskProgress(item.id);
        for (const auto& item : db.listLearningStepProgress()) if (item.courseId.value_or("") == courseId) db.deleteLearningStepProgress(item.id);
        for (const auto& item : db.listLearningCardProgress()) if (item.courseId.value_or("") == courseId) db.deleteLearningCardProgress(item.id);
        for (const auto& item : db.listLearningSessions()) if (item.courseId.value_or("") == courseId) db.deleteLearningSession(item.id);
        return db.deleteCourse(courseId);
    } catch (...) {
        return false;
    }
}

}  // namespace gangyi

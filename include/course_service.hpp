#pragma once

#include "db.hpp"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace gangyi {

struct CourseWithSnapshot {
    Course course;
    nlohmann::json payload;  // 最新快照 payload（已脱敏的 MockPlan 形状）
};

// 保存课程 + 快照（对齐校园版 lib/course/courseRepository.ts 的 createOrUpdateCourseSnapshot）：
// 按 (anonymousId 或 userId) + goal + status='active' 查找；命中 → 更新 title/summary/source/
// mode/updatedAt 并新增 CourseSnapshot（version = 旧最大 + 1）；未命中 → 新建 Course + 第一条快照
// （version = 1）。返回课程 id；失败返回 nullopt。调用方应先过 validateCourseContent 门禁并
// 对 payload 做 sanitizeCoursePayload。
std::optional<std::string> saveCourseSnapshot(Database& db, const std::string& anonymousId,
                                              const std::string& userId, const std::string& goal,
                                              const std::string& mode, const std::string& title,
                                              const std::string& summary, const std::string& source,
                                              const nlohmann::json& payload);

// 读取课程 + 最新快照；课程不存在或无快照返回 nullopt。
std::optional<CourseWithSnapshot> getCourseWithSnapshot(Database& db, const std::string& courseId);

// 按 userId 或 anonymousId 列出课程（status='active'，updatedAt 降序，分页），
// 供 /api/courses GET 与历史课堂下拉使用。
std::vector<Course> listCoursesForIdentity(Database& db, const std::string& userId,
                                           const std::string& anonymousId, size_t limit, size_t offset);

// 删除属于指定身份的课程及其快照、进度和学习会话。身份不匹配时返回 false。
bool deleteCourseForIdentity(Database& db, const std::string& courseId, const std::string& userId,
                             const std::string& anonymousId);

// 入库前脱敏（对齐校园版 sanitizeCoursePayload + stripUnsafeKeys）：
// 深度 ≤8；剔除键名含 apikey/api_key/secret/token/authorization/base64/image/rawerror/stack 的字段；
// 字符串截 10,000；白名单重排 title,duration,summary,courseIntro,overview,audience,prerequisites,
// outcome,learningOutcomes,slides,mindMap,roadmap,courseStructure,resources,projects、generation、phaseExpansions；
// resources 每项 name≤300/description≤500/href≤1000 且最多 12 条（过滤无 name 或 href 者）；
// 总 JSON > 900,000 时裁剪 slides≤20/roadmap≤12/resources≤8/projects≤12。
nlohmann::json sanitizeCoursePayload(const nlohmann::json& payload);

}  // namespace gangyi

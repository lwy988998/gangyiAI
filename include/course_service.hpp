// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <optional>，为后续实现提供类型或接口声明。
#include <optional>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 声明数据结构或类型 CourseWithSnapshot 及其成员接口。
struct CourseWithSnapshot {
    // 【逐行批注】 执行当前声明或表达式：Course course;。
    Course course;
    // 【逐行批注】 执行当前声明或表达式：nlohmann::json payload; // 最新快照 payload（已脱敏的 MockPlan 形状）。
    nlohmann::json payload;  // 最新快照 payload（已脱敏的 MockPlan 形状）
// 【逐行批注】 执行当前声明或表达式：};。
};

// 保存课程 + 快照（对齐校园版 lib/course/courseRepository.ts 的 createOrUpdateCourseSnapshot）：
// 按 (anonymousId 或 userId) + goal + status='active' 查找；命中 → 更新 title/summary/source/
// mode/updatedAt 并新增 CourseSnapshot（version = 旧最大 + 1）；未命中 → 新建 Course + 第一条快照
// （version = 1）。返回课程 id；失败返回 nullopt。调用方应先过 validateCourseContent 门禁并
// 对 payload 做 sanitizeCoursePayload。
// 【逐行批注】 声明对象 saveCourseSnapshot，并通过构造函数初始化其状态。
std::optional<std::string> saveCourseSnapshot(Database& db, const std::string& anonymousId,
                                              // 【逐行批注】 执行当前声明或表达式：const std::string& userId, const std::string& goal,。
                                              const std::string& userId, const std::string& goal,
                                              // 【逐行批注】 执行当前声明或表达式：const std::string& mode, const std::string& title,。
                                              const std::string& mode, const std::string& title,
                                              // 【逐行批注】 执行当前声明或表达式：const std::string& summary, const std::string& source,。
                                              const std::string& summary, const std::string& source,
                                              // 【逐行批注】 执行当前声明或表达式：const nlohmann::json& payload);。
                                              const nlohmann::json& payload);

// 读取课程 + 最新快照；课程不存在或无快照返回 nullopt。
// 【逐行批注】 声明对象 getCourseWithSnapshot，并通过构造函数初始化其状态。
std::optional<CourseWithSnapshot> getCourseWithSnapshot(Database& db, const std::string& courseId);

// 按 userId 或 anonymousId 列出课程（status='active'，updatedAt 降序，分页），
// 供 /api/courses GET 与历史课堂下拉使用。
// 【逐行批注】 声明对象 listCoursesForIdentity，并通过构造函数初始化其状态。
std::vector<Course> listCoursesForIdentity(Database& db, const std::string& userId,
                                           // 【逐行批注】 执行当前声明或表达式：const std::string& anonymousId, size_t limit, size_t offset);。
                                           const std::string& anonymousId, size_t limit, size_t offset);

// 从真实访问或学习记录恢复最近的有效课程，不把创建时间当成学习记录。
// 【逐行批注】 声明对象 currentCourseForIdentity，并通过构造函数初始化其状态。
std::optional<Course> currentCourseForIdentity(Database& db, const std::string& userId = {},
                                               // 【逐行批注】 计算右侧结果并写入 std::string& anonymousId，更新当前运行状态。
                                               const std::string& anonymousId = {});
// 【逐行批注】 声明对象 rememberCurrentCourse，并通过构造函数初始化其状态。
void rememberCurrentCourse(Database& db, const std::string& courseId);

// 删除属于指定身份的课程及其快照、进度和学习会话。身份不匹配时返回 false。
// 【逐行批注】 声明对象 deleteCourseForIdentity，并通过构造函数初始化其状态。
bool deleteCourseForIdentity(Database& db, const std::string& courseId, const std::string& userId,
                             // 【逐行批注】 执行当前声明或表达式：const std::string& anonymousId);。
                             const std::string& anonymousId);

// 入库前脱敏（对齐校园版 sanitizeCoursePayload + stripUnsafeKeys）：
// 深度 ≤8；剔除键名含 apikey/api_key/secret/token/authorization/base64/image/rawerror/stack 的字段；
// 字符串截 10,000；白名单重排 title,duration,summary,courseIntro,overview,audience,prerequisites,
// outcome,learningOutcomes,slides,mindMap,roadmap,courseStructure,resources,projects、generation、phaseExpansions；
// resources 每项 name≤300/description≤500/href≤1000 且最多 12 条（过滤无 name 或 href 者）；
// 总 JSON > 900,000 时裁剪 slides≤20/roadmap≤12/resources≤8/projects≤12。
// 【逐行批注】 声明对象 sanitizeCoursePayload，并通过构造函数初始化其状态。
nlohmann::json sanitizeCoursePayload(const nlohmann::json& payload);

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

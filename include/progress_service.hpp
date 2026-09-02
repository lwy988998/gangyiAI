#pragma once

#include "db.hpp"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace gangyi {

// 进度/会话 repository 层（db.cpp 之上），对齐 docs/phase-b-spec.md B1/B2。

// 进度三表 + 会话的通用 upsert 结果
struct ProgressSaveResult {
    bool ok = false;
    nlohmann::json item = nlohmann::json::object();  // 保存后的完整记录（含 id/status 等）
};

// 归一化状态白名单：learning-card/task 用 not_started|in_progress|completed；learning-step 用 unset|understood|review
std::string normalizeCardStatus(const std::string& status);
std::string normalizeStepStatus(const std::string& status);

// upsert LearningCardProgress：唯一键 (courseId, phaseIndex, topicIndex)；无 courseId 兜底 (anonymousId+goal+mode+phaseIndex+topicIndex)
ProgressSaveResult saveLearningCardProgress(Database& db, const nlohmann::json& body);
// upsert LearningStepProgress：唯一键 (courseId, phaseIndex, stepIndex)
ProgressSaveResult saveLearningStepProgress(Database& db, const nlohmann::json& body);
// upsert TaskProgress：唯一键 (courseId, phaseIndex, taskIndex)
ProgressSaveResult saveTaskProgress(Database& db, const nlohmann::json& body);

// 读取某课程三表完成情况（供 recompute 与页面使用）
int countCompletedCards(Database& db, const std::string& courseId);
int countUnderstoodSteps(Database& db, const std::string& courseId);
int countCompletedTasks(Database& db, const std::string& courseId);

// 全量重算并 upsert CourseProgress（从最新快照估算总量 + 三表统计完成量；percent clamp 0-100）
// 返回重算后的进度 JSON；课程不存在或无快照返回 nullopt
std::optional<nlohmann::json> recomputeCourseProgress(Database& db, const std::string& courseId,
                                                      const std::string& anonymousId, const std::string& goal);

// 清空课程的学习卡、步骤、任务和断点记录，并返回重新计算后的零进度。
std::optional<nlohmann::json> resetCourseProgress(Database& db, const std::string& courseId,
                                                   const std::string& anonymousId, const std::string& goal);

// 更新断点续学字段（lastVisitedUrl 必须是站内相对路径且以 / 开头且不含 //，否则忽略该字段）
bool updateLastVisited(Database& db, const std::string& courseId, const std::string& anonymousId,
                       const std::string& goal, const std::string& mode, const nlohmann::json& body);

// ---- LearningSession 会话存取 ----
// 查询：优先 (courseId, phaseIndex, topicIndex)；无 courseId 按 (anonymousId+goal+mode+phaseIndex+topicIndex) 最新一条
std::optional<nlohmann::json> findLearningSession(Database& db, const std::string& courseId,
                                                  const std::string& anonymousId, const std::string& goal,
                                                  const std::string& mode, int phaseIndex, int topicIndex);
// upsert 会话（content 存 JSON 字符串，≤900KB 超限裁剪 lessonSteps≤8/examples≤6/practice≤8/references≤8）
bool upsertLearningSession(Database& db, const nlohmann::json& body);

}  // namespace gangyi

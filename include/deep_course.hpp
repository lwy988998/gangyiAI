#pragma once
#include "learning_agent.hpp"
#include <utility>

namespace gangyi {
bool deepCourseEnabled(Database& db, const std::string& courseId);
// 只读取已保存内容；课程呈现类型由版本标记识别，历史 mode 不迁移。
AgentJson deepCourseView(Database& db, const AgentAccess& access, const AgentJson& scope);
AgentJson deepCourseStart(Database& db, const AgentAccess& access, const AgentJson& scope);
AgentJson deepCourseRetryMedia(Database& db, const AgentAccess& access, const AgentJson& scope);
std::pair<std::string, std::string> deepCourseImage(Database& db, const AgentAccess& access,
    const std::string& lessonId, const std::string& mediaId);
AgentJson normalizeDeepContent(const AgentJson& content);
AgentJson deepImageCandidates(const std::string& html, const std::string& pageUrl,
    const std::string& query = {}, const std::string& title = {});
void registerDeepCourseTools(LearningAgent& agent);
std::string deepCoursePrompt(Database& db, const AgentJson& task);
std::string renderDeepCoursePage();
}

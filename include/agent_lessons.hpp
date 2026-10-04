#pragma once
#include "learning_agent.hpp"

namespace gangyi {
// 保存模型自主选择的教学板块；公开读取始终排除题目答案与评分资料。
void registerAgentLessonTools(LearningAgent& agent);
AgentJson agentLessonView(Database& db, const AgentAccess& access, const std::string& lessonId);
AgentJson agentEnterLesson(Database& db, const AgentAccess& access, const AgentJson& body);
// 在请求去重之后保存学生输入，题目快照从服务端已保存课件读取。
void agentRecordLearnerEvent(Database& db, const AgentAccess& access, const AgentJson& event);
}

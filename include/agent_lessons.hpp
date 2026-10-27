#pragma once
#include "learning_agent.hpp"

namespace gangyi {
// 保存模型自主选择的教学板块；公开读取始终排除题目答案与评分资料。
void registerAgentLessonTools(LearningAgent& agent);
// 只计算已保存板块的真实页面和直达链接；调用方仍须校验课时权限。
AgentJson agentSectionLocation(const AgentJson& lesson, const AgentJson& section,
                              const AgentJson& originTask = AgentJson::object());
AgentJson agentLessonView(Database& db, const AgentAccess& access, const std::string& lessonId);
AgentJson agentEnterLesson(Database& db, const AgentAccess& access, const AgentJson& body);
// 开课请求使用固定身份；原任务未完成时延后，完成后在原事务中接管首次讲解。
AgentJson agentStartLesson(Database& db, const AgentAccess& access, const AgentJson& body);
// 在主控事件的同一事务中保存当前教学任务，旧窗口不能覆盖新反馈。
void agentAttachLessonTask(Database& db, const AgentAccess& access, const AgentJson& event, const std::string& taskId);
// 在请求去重之后保存学生输入，题目快照从服务端已保存课件读取。
void agentRecordLearnerEvent(Database& db, const AgentAccess& access, const AgentJson& event);
// 完整教学回复成功后才提交暂存评价；中断与失败不会新增可靠证据。
void finalizeAgentEvaluations(Database& db, const AgentJson& task);
// 仅记录浏览器实际展示过的公开事件，用于提示经历与中断恢复。
AgentJson agentExposeTask(Database& db, const AgentAccess& access, const AgentJson& body);
}

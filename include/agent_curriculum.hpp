#pragma once
#include "learning_agent.hpp"

namespace gangyi {
// 同时保护稳定身份与已经进入的知识点，撤回也使用同一校验。
void agentValidateOutline(Database& db, const std::string& courseId, const AgentJson& before, const AgentJson& after);
AgentJson agentOutline(Database& db, const AgentAccess& access, const std::string& courseId);
// 历史课件只转换身份和展示结构，不调用模型或改写原始内容。
AgentJson agentLegacyLesson(Database& db, const AgentAccess& access, const AgentJson& scope);
// 将原概览、示范、检查点和资料接入新分区，保留原课时与题目身份。
void agentUpgradeLegacyLesson(Database& db, const AgentAccess& access, const std::string& lessonId);
AgentJson agentLegacyEvent(Database& db, const AgentAccess& access, const AgentJson& body);
AgentJson agentQuestionView(Database& db, const AgentAccess& access, const AgentJson& scope);
void registerAgentCurriculumTools(LearningAgent& agent);
}

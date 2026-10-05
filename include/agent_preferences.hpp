#pragma once
#include "learning_agent.hpp"

namespace gangyi {
// AI 决定目标、时间和推荐；此层只校验结构、保存版本并记录撤回数据。
void registerAgentPreferenceTools(LearningAgent& agent);
AgentJson agentUndoChange(Database& db, const AgentAccess& access, const AgentJson& body);
AgentJson agentChanges(Database& db, const AgentAccess& access);
AgentJson agentRecommendationView(Database& db, const AgentAccess& access);
AgentJson agentChangeRecord(Database& db, const AgentJson& task, const AIResult& source, const AgentJson& args,
    const std::string& kind, const std::string& target, const AgentJson& before, const AgentJson& after);
void agentSaveChange(Database& db, const AgentJson& change, const std::string& courseId = {});
}

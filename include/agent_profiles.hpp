#pragma once
#include "learning_agent.hpp"

namespace gangyi {
// 证据充分性由模型决定；程序只拒绝不存在的原题、回答、引用与错误数据结构。
AgentJson agentProfileEvidence(Database& db, const AgentAccess& access);
void registerAgentProfileTools(LearningAgent& agent);
}

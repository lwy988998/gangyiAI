// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 证据充分性由模型决定；程序只拒绝不存在的原题、回答、引用与错误数据结构。
// 【逐行批注】 声明对象 agentProfileEvidence，并通过构造函数初始化其状态。
AgentJson agentProfileEvidence(Database& db, const AgentAccess& access);
// 【逐行批注】 声明对象 registerAgentProfileTools，并通过构造函数初始化其状态。
void registerAgentProfileTools(LearningAgent& agent);
// 【逐行批注】 结束当前代码块。
}

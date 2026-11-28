// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// AI 决定目标、时间和推荐；此层只校验结构、保存版本并记录撤回数据。
// 【逐行批注】 声明对象 registerAgentPreferenceTools，并通过构造函数初始化其状态。
void registerAgentPreferenceTools(LearningAgent& agent);
// 【逐行批注】 声明对象 agentUndoChange，并通过构造函数初始化其状态。
AgentJson agentUndoChange(Database& db, const AgentAccess& access, const AgentJson& body);
// 【逐行批注】 声明对象 agentChanges，并通过构造函数初始化其状态。
AgentJson agentChanges(Database& db, const AgentAccess& access);
// 【逐行批注】 声明对象 agentRecommendationView，并通过构造函数初始化其状态。
AgentJson agentRecommendationView(Database& db, const AgentAccess& access);
// 【逐行批注】 声明对象 agentChangeRecord，并通过构造函数初始化其状态。
AgentJson agentChangeRecord(Database& db, const AgentJson& task, const AIResult& source, const AgentJson& args,
    // 【逐行批注】 执行当前声明或表达式：const std::string& kind, const std::string& target, const AgentJson& before, const A…。
    const std::string& kind, const std::string& target, const AgentJson& before, const AgentJson& after);
// 【逐行批注】 声明对象 agentSaveChange，并通过构造函数初始化其状态。
void agentSaveChange(Database& db, const AgentJson& change, const std::string& courseId = {});
// 【逐行批注】 结束当前代码块。
}

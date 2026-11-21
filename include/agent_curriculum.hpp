// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 同时保护稳定身份与已经进入的知识点，撤回也使用同一校验。
// 【逐行批注】 声明对象 agentValidateOutline，并通过构造函数初始化其状态。
void agentValidateOutline(Database& db, const std::string& courseId, const AgentJson& before, const AgentJson& after);
// 【逐行批注】 声明对象 agentOutline，并通过构造函数初始化其状态。
AgentJson agentOutline(Database& db, const AgentAccess& access, const std::string& courseId);
// 旧复习记录返回空对象。新记录根据当前作答重新核对依据。
AgentJson agentScheduledReview(Database& db, const AgentAccess& access, const std::string& courseId,
                               const std::string& reviewId);
// 历史课件只转换身份和展示结构，不调用模型或改写原始内容。
// 【逐行批注】 声明对象 agentLegacyLesson，并通过构造函数初始化其状态。
AgentJson agentLegacyLesson(Database& db, const AgentAccess& access, const AgentJson& scope);
// 将原概览、示范、检查点和资料接入新分区，保留原课时与题目身份。
// 【逐行批注】 声明对象 agentUpgradeLegacyLesson，并通过构造函数初始化其状态。
void agentUpgradeLegacyLesson(Database& db, const AgentAccess& access, const std::string& lessonId);
// 【逐行批注】 声明对象 agentLegacyEvent，并通过构造函数初始化其状态。
AgentJson agentLegacyEvent(Database& db, const AgentAccess& access, const AgentJson& body);
// 【逐行批注】 声明对象 agentQuestionView，并通过构造函数初始化其状态。
AgentJson agentQuestionView(Database& db, const AgentAccess& access, const AgentJson& scope);
// 【逐行批注】 声明对象 registerAgentCurriculumTools，并通过构造函数初始化其状态。
void registerAgentCurriculumTools(LearningAgent& agent);
// 【逐行批注】 结束当前代码块。
}

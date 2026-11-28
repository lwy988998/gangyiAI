// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 将 AI 原始输出（GeneratedPlan 形状：inferredDomain/learnerGoal/courseTitle/courseSummary/
// title/goal/durationWeeks/summary/courseIntro/overview/audience/prerequisites/outcome/
// learningOutcomes/phases[].{name,durationWeeks,duration,objective,why,description,overview,
// topics,topicDescriptions,tasks,practice,checkpoint,output,commonMistakes,steps}/slides/
// mindMap/resources/projects）适配为前端渲染形状（MockPlan 形状，见 docs/phase-a-spec.md A3）。
// 对齐校园版 lib/ai/adaptGeneratedPlan.ts 的行为（含 slides/mindMap 从 phases 推导、skeletonSteps 兜底等）。
// 【逐行批注】 声明对象 adaptGeneratedPlan，并通过构造函数初始化其状态。
nlohmann::json adaptGeneratedPlan(const nlohmann::json& plan, const std::string& mode);

// 判定适配结果是否可渲染：title/summary 非空、roadmap 非空、courseStructure 非空。
// 【逐行批注】 声明对象 isRenderablePlan，并通过构造函数初始化其状态。
bool isRenderablePlan(const nlohmann::json& plan);

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

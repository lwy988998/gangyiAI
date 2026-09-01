#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace gangyi {

// 将 AI 原始输出（GeneratedPlan 形状：inferredDomain/learnerGoal/courseTitle/courseSummary/
// title/goal/durationWeeks/summary/courseIntro/overview/audience/prerequisites/outcome/
// learningOutcomes/phases[].{name,durationWeeks,duration,objective,why,description,overview,
// topics,topicDescriptions,tasks,practice,checkpoint,output,commonMistakes,steps}/slides/
// mindMap/resources/projects）适配为前端渲染形状（MockPlan 形状，见 docs/phase-a-spec.md A3）。
// 对齐校园版 lib/ai/adaptGeneratedPlan.ts 的行为（含 slides/mindMap 从 phases 推导、skeletonSteps 兜底等）。
nlohmann::json adaptGeneratedPlan(const nlohmann::json& plan, const std::string& mode);

// 判定适配结果是否可渲染：title/summary 非空、roadmap 非空、courseStructure 非空。
bool isRenderablePlan(const nlohmann::json& plan);

}  // namespace gangyi

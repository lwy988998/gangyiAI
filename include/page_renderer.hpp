#pragma once

#include <string>

#include <nlohmann/json.hpp>

namespace gangyi {

std::string renderHomePage();
std::string renderPlanPage(const std::string& goal, const std::string& mode,
                           const std::string& courseId = {}, const std::string& anonymousId = {});
std::string renderLearnPage(const std::string& courseId, const std::string& goal, const std::string& mode,
                            const std::string& phaseIndex, const std::string& phaseName,
                            const std::string& topicIndex, const std::string& topic,
                            const std::string& anonymousId = {}, const std::string& regenerate = {},
                            const std::string& forceLearn = {}, const std::string& retry = {});
std::string renderProgressPage(const std::string& courseId, const std::string& anonymousId,
                               const nlohmann::json& data = {});
// /phase 阶段页：courseId 课程快照(plan: MockPlan) + cardStatus(topicNo->completed/in_progress) 服务端渲染。
// 无 courseId 或快照结构不完整时渲染“阶段内容未生成/未找到”降级态。
std::string renderPhasePage(const std::string& courseId, const std::string& anonymousId,
                            const std::string& goal, const std::string& mode,
                            const std::string& phaseIndex, const std::string& phaseName,
                            const nlohmann::json& plan, const nlohmann::json& cardStatus);
std::string renderLoginPage();
std::string renderRegisterPage();
std::string renderAskPage(const std::string& goal, const std::string& question = {}, const std::string& mode = "deep");
std::string renderMyCoursesPage(const nlohmann::json& data = {});
std::string renderAdminPage(const std::string& state);

}  // namespace gangyi

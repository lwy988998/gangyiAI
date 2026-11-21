// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 声明对象 renderClassroomPage，并通过构造函数初始化其状态。
std::string renderClassroomPage(const std::string& view);
// 【逐行批注】 声明对象 renderHomePage，并通过构造函数初始化其状态。
std::string renderHomePage();
// 【逐行批注】 声明对象 renderStartupPage，并通过构造函数初始化其状态。
std::string renderStartupPage();
// 【逐行批注】 声明对象 renderPlanPage，并通过构造函数初始化其状态。
std::string renderPlanPage(const std::string& goal, const std::string& mode,
                           // 【逐行批注】 计算右侧结果并写入 std::string& courseId，更新当前运行状态。
                           const std::string& courseId = {}, const std::string& anonymousId = {});
// /phase 阶段页：courseId 课程快照(plan: MockPlan) + cardStatus(topicNo->completed/in_progress) 服务端渲染。
// 无 courseId 或快照结构不完整时渲染“阶段内容未生成/未找到”降级态。
// 【逐行批注】 声明对象 renderPhasePage，并通过构造函数初始化其状态。
std::string renderPhasePage(const std::string& courseId, const std::string& anonymousId,
                            // 【逐行批注】 执行当前声明或表达式：const std::string& goal, const std::string& mode,。
                            const std::string& goal, const std::string& mode,
                            // 【逐行批注】 执行当前声明或表达式：const std::string& phaseIndex, const std::string& phaseName,。
                            const std::string& phaseIndex, const std::string& phaseName,
                            // 【逐行批注】 执行当前声明或表达式：const nlohmann::json& plan, const nlohmann::json& cardStatus);。
                            const nlohmann::json& plan, const nlohmann::json& cardStatus);
// 【逐行批注】 声明对象 renderLearnPage，并通过构造函数初始化其状态。
std::string renderLearnPage(const std::string& courseId, const std::string& goal, const std::string& mode,
                            // 【逐行批注】 执行当前声明或表达式：const std::string& phaseIndex, const std::string& phaseName,。
                            const std::string& phaseIndex, const std::string& phaseName,
                            // 【逐行批注】 执行当前声明或表达式：const std::string& topicIndex, const std::string& topic,。
                            const std::string& topicIndex, const std::string& topic,
                            // 【逐行批注】 计算右侧结果并写入 std::string& anonymousId, const std::string&…，更新当前运行状态。
                            const std::string& anonymousId, const std::string& regenerate = {},
                            // 【逐行批注】 计算右侧结果并写入 std::string& forceLearn，更新当前运行状态。
                            const std::string& forceLearn = {}, const std::string& retry = {});
// 【逐行批注】 声明对象 renderAskPage，并通过构造函数初始化其状态。
std::string renderAskPage(const std::string& question = {}, const nlohmann::json& material = nullptr);
std::string renderYesterdayReviewPage(const nlohmann::json& review, const std::string& error = {});
// 备课任务页仅订阅真实生成过程，完整保存后再进入课堂。
// 【逐行批注】 声明对象 renderMyCoursesPage，并通过构造函数初始化其状态。
std::string renderMyCoursesPage(const nlohmann::json& data = {});
// 【逐行批注】 声明对象 renderCurrentCourseEmptyPage，并通过构造函数初始化其状态。
std::string renderCurrentCourseEmptyPage(bool hasCourses = false);
std::string renderExamExplanationPage();

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"
// 【逐行批注】 引入头文件 <utility>，为后续实现提供类型或接口声明。
#include <utility>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明对象 deepCourseEnabled，并通过构造函数初始化其状态。
bool deepCourseEnabled(Database& db, const std::string& courseId);
// 只读取已保存内容；课程呈现类型由版本标记识别，历史 mode 不迁移。
// 【逐行批注】 声明对象 deepCourseView，并通过构造函数初始化其状态。
AgentJson deepCourseView(Database& db, const AgentAccess& access, const AgentJson& scope);
// 组件草稿只恢复探索过程，不触发模型调用或生成正式成绩。
AgentJson deepCourseSaveComponentDraft(Database& db, const AgentAccess& access, const AgentJson& body);
// 【逐行批注】 声明对象 deepCourseStart，并通过构造函数初始化其状态。
AgentJson deepCourseStart(Database& db, const AgentAccess& access, const AgentJson& scope);
// 【逐行批注】 声明对象 deepCourseRetryMedia，并通过构造函数初始化其状态。
AgentJson deepCourseRetryMedia(Database& db, const AgentAccess& access, const AgentJson& scope);
// 【逐行批注】 调用 deepCourseImage 完成本行所需操作：std::pair<std::string, std::string> deepCourseImage(Database& db, con…。
std::pair<std::string, std::string> deepCourseImage(Database& db, const AgentAccess& access,
    // 【逐行批注】 执行当前声明或表达式：const std::string& lessonId, const std::string& mediaId);。
    const std::string& lessonId, const std::string& mediaId);
// 【逐行批注】 声明对象 normalizeDeepContent，并通过构造函数初始化其状态。
AgentJson normalizeDeepContent(const AgentJson& content);
// 【逐行批注】 声明对象 deepImageCandidates，并通过构造函数初始化其状态。
AgentJson deepImageCandidates(const std::string& html, const std::string& pageUrl,
    // 【逐行批注】 计算右侧结果并写入 std::string& query，更新当前运行状态。
    const std::string& query = {}, const std::string& title = {});
// 【逐行批注】 声明对象 registerDeepCourseTools，并通过构造函数初始化其状态。
void registerDeepCourseTools(LearningAgent& agent);
// 【逐行批注】 声明对象 deepCoursePrompt，并通过构造函数初始化其状态。
std::string deepCoursePrompt(Database& db, const AgentJson& task);
// 【逐行批注】 声明对象 renderDeepCoursePage，并通过构造函数初始化其状态。
std::string renderDeepCoursePage();
// 【逐行批注】 结束当前代码块。
}

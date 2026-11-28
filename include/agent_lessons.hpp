// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 保存模型自主选择的教学板块；公开读取始终排除题目答案与评分资料。
// 【逐行批注】 声明对象 registerAgentLessonTools，并通过构造函数初始化其状态。
void registerAgentLessonTools(LearningAgent& agent);
// 只计算已保存板块的真实页面和直达链接；调用方仍须校验课时权限。
// 【逐行批注】 声明对象 agentSectionLocation，并通过构造函数初始化其状态。
AgentJson agentSectionLocation(const AgentJson& lesson, const AgentJson& section,
                              // 【逐行批注】 计算右侧结果并写入 AgentJson& originTask，更新当前运行状态。
                              const AgentJson& originTask = AgentJson::object());
// 【逐行批注】 声明对象 agentLessonView，并通过构造函数初始化其状态。
AgentJson agentLessonView(Database& db, const AgentAccess& access, const std::string& lessonId);
// 访问记录只证明学生打开过课时，不能充当掌握证据。
AgentJson agentReviewLessons(Database& db, const AgentAccess& access, const AgentJson& window);
// 只投影同档案、同课程、同知识点的正式评价与练习经历。
AgentJson agentTopicEvidence(Database& db, const AgentAccess& access, const std::string& lessonId);
AgentJson agentTopicEvidenceForPreparation(Database& db, const AgentAccess& access,
                                           const std::string& courseId, const std::string& topicId);
// 【逐行批注】 声明对象 agentEnterLesson，并通过构造函数初始化其状态。
AgentJson agentEnterLesson(Database& db, const AgentAccess& access, const AgentJson& body);
// 开课请求使用固定身份；原任务未完成时延后，完成后在原事务中接管首次讲解。
// 【逐行批注】 声明对象 agentStartLesson，并通过构造函数初始化其状态。
AgentJson agentStartLesson(Database& db, const AgentAccess& access, const AgentJson& body);
// 在主控事件的同一事务中保存当前教学任务，旧窗口不能覆盖新反馈。
// 【逐行批注】 声明对象 agentAttachLessonTask，并通过构造函数初始化其状态。
void agentAttachLessonTask(Database& db, const AgentAccess& access, const AgentJson& event, const std::string& taskId);
// 仅在已进入的课时解析明确的自由交流请求，题卡作答继续沿用原协议。
AgentJson agentResolveLessonUtterance(Database& db, const AgentAccess& access, const AgentJson& event);
// 在请求去重之后保存学生输入，题目快照从服务端已保存课件读取。
// 【逐行批注】 声明对象 agentRecordLearnerEvent，并通过构造函数初始化其状态。
void agentRecordLearnerEvent(Database& db, const AgentAccess& access, const AgentJson& event);
// 完整教学回复成功后才提交暂存评价；中断与失败不会新增可靠证据。
// 【逐行批注】 声明对象 finalizeAgentEvaluations，并通过构造函数初始化其状态。
bool finalizeAgentEvaluations(Database& db, const AgentJson& task);
// 仅记录浏览器实际展示过的公开事件，用于提示经历与中断恢复。
// 【逐行批注】 声明对象 agentExposeTask，并通过构造函数初始化其状态。
AgentJson agentExposeTask(Database& db, const AgentAccess& access, const AgentJson& body);
// 【逐行批注】 结束当前代码块。
}

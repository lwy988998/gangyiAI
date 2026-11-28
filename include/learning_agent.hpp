// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "ai_client.hpp"，为后续实现提供类型或接口声明。
#include "ai_client.hpp"
// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <atomic>，为后续实现提供类型或接口声明。
#include <atomic>
// 【逐行批注】 引入头文件 <functional>，为后续实现提供类型或接口声明。
#include <functional>
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 计算右侧结果并写入 using AgentJson，更新当前运行状态。
using AgentJson = nlohmann::json;
// 【逐行批注】 声明数据结构或类型 AgentAccess 及其成员接口。
struct AgentAccess {
    // 【逐行批注】 执行当前声明或表达式：std::string scopeId;。
    std::string scopeId;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> courseIds;。
    std::vector<std::string> courseIds;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 PreparedAgentTool 及其成员接口。
struct PreparedAgentTool {
    // 【逐行批注】 计算右侧结果并写入 AgentJson result，更新当前运行状态。
    AgentJson result = AgentJson::object();
    // 真实模型调用在准备阶段完成；提交闭包只执行数据库操作。
    // 【逐行批注】 调用 void 完成本行所需操作：std::function<void(Database&)> commit;。
    std::function<void(Database&)> commit;
    // 已持久化的控制操作在短事务结束后执行，避免嵌套写事务。
    // 【逐行批注】 调用 void 完成本行所需操作：std::function<void(Database&)> afterCommit = {};。
    std::function<void(Database&)> afterCommit = {};
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 AgentTool 及其成员接口。
struct AgentTool {
    // 【逐行批注】 执行当前声明或表达式：std::string description;。
    std::string description;
    // 【逐行批注】 计算右侧结果并写入 bool readOnly，更新当前运行状态。
    bool readOnly = false;
    // 【逐行批注】 调用 PreparedAgentTool 完成本行所需操作：std::function<PreparedAgentTool(Database&, const AgentJson&, const Ag…。
    std::function<PreparedAgentTool(Database&, const AgentJson&, const AgentJson&, const AIResult&)> prepare;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明对象 agentLearnerFingerprint，并通过构造函数初始化其状态。
std::string agentLearnerFingerprint(Database& db, const AgentAccess& access);
// 【逐行批注】 声明对象 agentContext，并通过构造函数初始化其状态。
AgentJson agentContext(Database& db, const AgentAccess& access, const AgentJson& event);
// 【逐行批注】 声明数据结构或类型 AgentSubmissionOrigin 及其成员接口。
enum class AgentSubmissionOrigin { ordinary, manualScheduleReplan, yesterdayReview };
// 【逐行批注】 声明对象 agentSubmit，并通过构造函数初始化其状态。
AgentJson agentSubmit(Database& db, const AgentAccess& access, AgentJson event,
                      // 【逐行批注】 计算右侧结果并写入 AgentSubmissionOrigin origin，更新当前运行状态。
                      AgentSubmissionOrigin origin = AgentSubmissionOrigin::ordinary);
// 【逐行批注】 声明对象 agentView，并通过构造函数初始化其状态。
AgentJson agentView(Database& db, const AgentAccess& access, const std::string& taskId = {}, bool compact = false, int afterSeq = 0);
// 按课程与知识点重新定位仍在准备或失败可恢复的备课任务，供页面切换后重连。
// 【逐行批注】 声明对象 agentDeepPreparationView，并通过构造函数初始化其状态。
AgentJson agentDeepPreparationView(Database& db, const AgentAccess& access, const AgentJson& scope);
// 【逐行批注】 声明对象 agentActivityView，并通过构造函数初始化其状态。
AgentJson agentActivityView(Database& db, const AgentAccess& access);
// 【逐行批注】 声明对象 agentResumeGlobalTasks，并通过构造函数初始化其状态。
void agentResumeGlobalTasks(Database& db, const AgentAccess& access);
// 【逐行批注】 声明对象 agentControl，并通过构造函数初始化其状态。
AgentJson agentControl(Database& db, const AgentAccess& access, const AgentJson& body);

// 主控由真实模型选择工具与结束时机；程序只负责执行、持久化和冲突校验。
// 【逐行批注】 声明数据结构或类型 LearningAgent 及其成员接口。
class LearningAgent {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 计算右侧结果并写入 using ModelCall，更新当前运行状态。
    using ModelCall = std::function<AIResult(const ChatOptions&, const std::function<bool(const std::string&)>&)>;
    // 【逐行批注】 声明对象 LearningAgent，并通过构造函数初始化其状态。
    explicit LearningAgent(std::string databasePath, ModelCall model = {});
    // 【逐行批注】 调用 LearningAgent 完成本行所需操作：~LearningAgent();。
    ~LearningAgent();
    // 【逐行批注】 注册可由 AI 主控调用的工具：void registerTool(const std::string& name, AgentTool tool);。
    void registerTool(const std::string& name, AgentTool tool);
    // 【逐行批注】 声明对象 start，并通过构造函数初始化其状态。
    void start();
    // 【逐行批注】 声明对象 stop，并通过构造函数初始化其状态。
    void stop();
    // 【逐行批注】 声明对象 process，并通过构造函数初始化其状态。
    void process(Database& db, const std::string& taskId);
    // 【逐行批注】 声明对象 processQuantum，并通过构造函数初始化其状态。
    void processQuantum(Database& db, const std::string& taskId);
// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 声明对象 processImpl，并通过构造函数初始化其状态。
    void processImpl(Database& db, const std::string& taskId, bool quantum);
    // 【逐行批注】 声明对象 processIndependentChat，并通过构造函数初始化其状态。
    void processIndependentChat(Database& db, AgentJson& task);
    // 处理试卷讲解的识别、联网补充和自然语言教学对话。
    void processExamExplanation(Database& db, AgentJson& task);
    // 【逐行批注】 声明对象 runWorker，并通过构造函数初始化其状态。
    void runWorker(int lane);
    // 【逐行批注】 声明对象 claimNext，并通过构造函数初始化其状态。
    std::string claimNext(Database& db, int lane);
    // 【逐行批注】 执行当前声明或表达式：std::string databasePath_;。
    std::string databasePath_;
    // 【逐行批注】 执行当前声明或表达式：ModelCall model_;。
    ModelCall model_;
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, AgentTool> tools_;。
    std::map<std::string, AgentTool> tools_;
    // 【逐行批注】 执行当前声明或表达式：std::atomic_bool stopped_{false};。
    std::atomic_bool stopped_{false};
    // 【逐行批注】 执行当前声明或表达式：std::atomic_bool started_{false};。
    std::atomic_bool started_{false};
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::thread> workers_;。
    std::vector<std::thread> workers_;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
} // namespace gangyi

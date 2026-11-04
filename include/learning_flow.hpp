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
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 计算右侧结果并写入 using FlowJson，更新当前运行状态。
using FlowJson = nlohmann::json;
// 【逐行批注】 声明对象 learningContext，并通过构造函数初始化其状态。
std::string learningContext(Database& db, const std::string& courseId = {});
// 【逐行批注】 声明对象 studyPlanView，并通过构造函数初始化其状态。
FlowJson studyPlanView(Database& db);
// 【逐行批注】 声明对象 editStudyPlan，并通过构造函数初始化其状态。
FlowJson editStudyPlan(Database& db, const FlowJson& body);
// 【逐行批注】 声明对象 studyPlanProposal，并通过构造函数初始化其状态。
FlowJson studyPlanProposal(Database& db, const FlowJson& body, const std::string& action);
// 【逐行批注】 声明对象 studyPlanDraft，并通过构造函数初始化其状态。
FlowJson studyPlanDraft(Database& db, const FlowJson& body);
// 【逐行批注】 声明对象 preparationView，并通过构造函数初始化其状态。
FlowJson preparationView(Database& db, const std::string& courseId = {});
// 【逐行批注】 声明对象 nextStepView，并通过构造函数初始化其状态。
FlowJson nextStepView(Database& db, const std::string& courseId = {});
// 【逐行批注】 声明对象 coursePreviewView，并通过构造函数初始化其状态。
FlowJson coursePreviewView(Database& db, const std::string& courseId);
// 【逐行批注】 声明对象 requestCoursePreview，并通过构造函数初始化其状态。
FlowJson requestCoursePreview(Database& db, const std::string& courseId, bool retry = false);
// 【逐行批注】 声明对象 exposeLearningBlocks，并通过构造函数初始化其状态。
FlowJson exposeLearningBlocks(Database& db, const FlowJson& body);
// 【逐行批注】 声明对象 dialogueView，并通过构造函数初始化其状态。
FlowJson dialogueView(Database& db, const FlowJson& body);
// 【逐行批注】 声明对象 beginDialogue，并通过构造函数初始化其状态。
FlowJson beginDialogue(Database& db, const FlowJson& body);
// 【逐行批注】 声明对象 evaluateDialogue，并通过构造函数初始化其状态。
FlowJson evaluateDialogue(Database& db, const FlowJson& body, FlowJson& turn,
                         // 【逐行批注】 调用 bool 完成本行所需操作：const std::function<bool()>& cancelled = {});。
                         const std::function<bool()>& cancelled = {});
// 只追加真实分片的展示记录；对话版本已变更时拒绝继续显示旧回复。
// 【逐行批注】 声明对象 recordDialogueAssistance，并通过构造函数初始化其状态。
bool recordDialogueAssistance(Database& db, const FlowJson& turn, const std::string& chunk);
// 【逐行批注】 声明对象 dialogueOptions，并通过构造函数初始化其状态。
ChatOptions dialogueOptions(Database& db, const FlowJson& body, const FlowJson& turn);
// 【逐行批注】 声明对象 finishDialogue，并通过构造函数初始化其状态。
FlowJson finishDialogue(Database& db, const FlowJson& body, const FlowJson& turn,
                        // 【逐行批注】 执行当前声明或表达式：const std::string& answer, const std::string& model,。
                        const std::string& answer, const std::string& model,
                        // 【逐行批注】 计算右侧结果并写入 std::string& failure，更新当前运行状态。
                        const std::string& failure = {});

// 【逐行批注】 声明数据结构或类型 LearningFlow 及其成员接口。
class LearningFlow {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 LearningFlow，并通过构造函数初始化其状态。
    explicit LearningFlow(std::string databasePath);
    // 【逐行批注】 调用 LearningFlow 完成本行所需操作：~LearningFlow();。
    ~LearningFlow();
    // 【逐行批注】 声明对象 start，并通过构造函数初始化其状态。
    void start();
    // 【逐行批注】 声明对象 stop，并通过构造函数初始化其状态。
    void stop();
// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 声明对象 run，并通过构造函数初始化其状态。
    void run();
    // 【逐行批注】 声明对象 evaluate，并通过构造函数初始化其状态。
    void evaluate(Database& db, const ClassroomActivity& activity);
    // 【逐行批注】 声明对象 preview，并通过构造函数初始化其状态。
    void preview(Database& db, const ClassroomActivity& activity);
    // 【逐行批注】 声明对象 coordinate，并通过构造函数初始化其状态。
    void coordinate(Database& db, int revision);
    // 【逐行批注】 声明对象 prepare，并通过构造函数初始化其状态。
    void prepare(Database& db, const FlowJson& teaching, int revision);
    // 【逐行批注】 声明对象 call，并通过构造函数初始化其状态。
    AIResult call(const std::string& system, const FlowJson& input,
                  // 【逐行批注】 调用 bool 完成本行所需操作：const std::function<bool()>& additionalCancelled = {});。
                  const std::function<bool()>& additionalCancelled = {});
    // 【逐行批注】 执行当前声明或表达式：std::string databasePath_;。
    std::string databasePath_;
    // 【逐行批注】 执行当前声明或表达式：std::atomic_bool stopped_{false};。
    std::atomic_bool stopped_{false};
    // 【逐行批注】 执行当前声明或表达式：std::thread worker_;。
    std::thread worker_;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <cstdint>，为后续实现提供类型或接口声明。
#include <cstdint>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明数据结构或类型 AIActivityInfo 及其成员接口。
struct AIActivityInfo {
    // 【逐行批注】 执行当前声明或表达式：std::string taskId;。
    std::string taskId;
    // 【逐行批注】 计算右侧结果并写入 std::string source，更新当前运行状态。
    std::string source = "AI 服务";
    // 【逐行批注】 计算右侧结果并写入 std::string purpose，更新当前运行状态。
    std::string purpose = "生成 AI 内容";
    // 【逐行批注】 执行当前声明或表达式：std::string scopeId;。
    std::string scopeId;
    // 【逐行批注】 执行当前声明或表达式：std::string courseId;。
    std::string courseId;
    // 【逐行批注】 执行当前声明或表达式：std::string lessonId;。
    std::string lessonId;
    // 【逐行批注】 计算右侧结果并写入 int legacyCalls，更新当前运行状态。
    int legacyCalls = 0;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 启动器与服务共享调用元数据和暂停开关，不保存提示词、密钥或模型正文。
// 【逐行批注】 声明数据结构或类型 AIActivity 及其成员接口。
class AIActivity {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 调用 configure 完成本行所需操作：static void configure(const std::string& path);。
    static void configure(const std::string& path);
    // 【逐行批注】 调用 configureUsage 完成本行所需操作：static void configureUsage(const std::string& databasePath);。
    static void configureUsage(const std::string& databasePath);
    // 【逐行批注】 调用 requestFinished 完成本行所需操作：static void requestFinished(const std::string& id, const std::string&…。
    static void requestFinished(const std::string& id, const std::string& status,
                                // 【逐行批注】 计算右侧结果并写入 nlohmann::json& tokens，更新当前运行状态。
                                const nlohmann::json& tokens = nullptr, const std::string& model = {},
                                // 【逐行批注】 计算右侧结果并写入 std::string& receipt，更新当前运行状态。
                                const std::string& receipt = {});
    // 【逐行批注】 调用 snapshot 完成本行所需操作：static nlohmann::json snapshot();。
    static nlohmann::json snapshot();
    // 【逐行批注】 调用 setPaused 完成本行所需操作：static void setPaused(bool paused);。
    static void setPaused(bool paused);
    // 【逐行批注】 调用 paused 完成本行所需操作：static bool paused();。
    static bool paused();
    // 【逐行批注】 调用 pauseEpoch 完成本行所需操作：static std::uint64_t pauseEpoch();。
    static std::uint64_t pauseEpoch();
    // 【逐行批注】 调用 begin 完成本行所需操作：static std::string begin(const AIActivityInfo& info, const std::strin…。
    static std::string begin(const AIActivityInfo& info, const std::string& model);
    // 【逐行批注】 计算右侧结果并写入 bool requestStarted(const std::string& id, s…，更新当前运行状态。
    static bool requestStarted(const std::string& id, std::string* receipt = nullptr);
    // 【逐行批注】 计算右侧结果并写入 void update(const std::string& id, const std…，更新当前运行状态。
    static void update(const std::string& id, const std::string& status, const std::string& model = {});
    // 【逐行批注】 调用 shutdown 完成本行所需操作：static void shutdown();。
    static void shutdown();
    // 【逐行批注】 调用 stopping 完成本行所需操作：static bool stopping();。
    static bool stopping();
    // 任务耗时使用进程内单调时钟；跨进程恢复没有起点时保持未知。
    // 【逐行批注】 调用 taskQueued 完成本行所需操作：static void taskQueued(const std::string& id);。
    static void taskQueued(const std::string& id);
    // 【逐行批注】 调用 taskRunning 完成本行所需操作：static void taskRunning(const std::string& id);。
    static void taskRunning(const std::string& id);
    // 【逐行批注】 调用 taskContent 完成本行所需操作：static void taskContent(const std::string& id);。
    static void taskContent(const std::string& id);
    // 【逐行批注】 调用 taskFinished 完成本行所需操作：static void taskFinished(const std::string& id);。
    static void taskFinished(const std::string& id);
    // 【逐行批注】 调用 taskTiming 完成本行所需操作：static nlohmann::json taskTiming(const std::string& id);。
    static nlohmann::json taskTiming(const std::string& id);
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 结束当前代码块。
}

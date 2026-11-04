// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <functional>，为后续实现提供类型或接口声明。
#include <functional>
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 声明数据结构或类型 AgentStreamDelta 及其成员接口。
struct AgentStreamDelta {
    // 【逐行批注】 执行当前声明或表达式：std::string field;。
    std::string field;
    // 【逐行批注】 计算右侧结果并写入 int actionIndex，更新当前运行状态。
    int actionIndex = -1;
    // 【逐行批注】 计算右侧结果并写入 int optionIndex，更新当前运行状态。
    int optionIndex = -1;
    // 【逐行批注】 执行当前声明或表达式：std::string text;。
    std::string text;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 从真实模型的 JSON 分片中只提取面向学生的文字，隐藏答案和行动参数。
// 【逐行批注】 声明数据结构或类型 AgentPublicStream 及其成员接口。
class AgentPublicStream {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 计算右侧结果并写入 using Sink，更新当前运行状态。
    using Sink = std::function<void(const AgentStreamDelta&)>;
    // 【逐行批注】 声明对象 AgentPublicStream，并通过构造函数初始化其状态。
    explicit AgentPublicStream(Sink sink);
    // 【逐行批注】 声明对象 push，并通过构造函数初始化其状态。
    void push(const std::string& chunk);
    // 【逐行批注】 定义函数 content，执行该函数声明的职责。
    const std::string& content() const { return content_; }
    // 【逐行批注】 定义函数 helpType，执行该函数声明的职责。
    const std::string& helpType() const { return helpType_; }
// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：Sink sink_;。
    Sink sink_;
    // 【逐行批注】 执行当前声明或表达式：std::string content_;。
    std::string content_;
    // 【逐行批注】 执行当前声明或表达式：std::string helpType_;。
    std::string helpType_;
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, std::string> emitted_;。
    std::map<std::string, std::string> emitted_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
} // namespace gangyi

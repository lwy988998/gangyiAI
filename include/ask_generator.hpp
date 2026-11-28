// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 "ai_client.hpp"，为后续实现提供类型或接口声明。
#include "ai_client.hpp"

// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 声明数据结构或类型 AskAnswer 及其成员接口。
struct AskAnswer {
    // 【逐行批注】 执行当前声明或表达式：std::string content;。
    std::string content;
    // 【逐行批注】 执行当前声明或表达式：std::string model;。
    std::string model;
    // 【逐行批注】 执行当前声明或表达式：std::string searchStatus;。
    std::string searchStatus;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> sources;。
    std::vector<std::string> sources;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 AskGenerator 及其成员接口。
class AskGenerator {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 AskGenerator，并通过构造函数初始化其状态。
    explicit AskGenerator(AIClient& client);
    // 【逐行批注】 计算右侧结果并写入 ChatOptions options(const std::string& quest…，更新当前运行状态。
    static ChatOptions options(const std::string& question, const std::string& profileContext = {},
                               // 【逐行批注】 执行当前声明或表达式：const std::vector<ChatMessage>& history = {});。
                               const std::vector<ChatMessage>& history = {});
    // 【逐行批注】 声明对象 generate，并通过构造函数初始化其状态。
    AskAnswer generate(const std::string& question, const std::string& profileContext = {},
                       // 【逐行批注】 执行当前声明或表达式：const std::vector<ChatMessage>& history = {}) const;。
                       const std::vector<ChatMessage>& history = {}) const;

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：AIClient& client_;。
    AIClient& client_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

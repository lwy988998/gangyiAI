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

// 【逐行批注】 声明数据结构或类型 ImageGoalAnalysis 及其成员接口。
struct ImageGoalAnalysis {
    // 【逐行批注】 执行当前声明或表达式：std::string goal;。
    std::string goal;
    // 【逐行批注】 执行当前声明或表达式：std::string summary;。
    std::string summary;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> keywords;。
    std::vector<std::string> keywords;
    // 【逐行批注】 执行当前声明或表达式：std::string suggestedSearchQuery;。
    std::string suggestedSearchQuery;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 ImageGoalAnalyzer 及其成员接口。
class ImageGoalAnalyzer {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 ImageGoalAnalyzer，并通过构造函数初始化其状态。
    explicit ImageGoalAnalyzer(AIClient& client);
    // 【逐行批注】 声明对象 analyze，并通过构造函数初始化其状态。
    ImageGoalAnalysis analyze(const std::string& prompt, const std::string& mode,
                              // 【逐行批注】 执行当前声明或表达式：const std::string& mimeType, const std::string& imageBytes) const;。
                              const std::string& mimeType, const std::string& imageBytes) const;

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：AIClient& client_;。
    AIClient& client_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

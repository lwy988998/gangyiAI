// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 "ai_client.hpp"，为后续实现提供类型或接口声明。
#include "ai_client.hpp"
// 【逐行批注】 引入头文件 "resource_types.hpp"，为后续实现提供类型或接口声明。
#include "resource_types.hpp"

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 公开备课内容只按完整路径取字段，私有答案和评分依据不下发。
// 【逐行批注】 声明对象 publicPreparationBlock，并通过构造函数初始化其状态。
nlohmann::json publicPreparationBlock(const std::string& block, const nlohmann::json& content);

// 从真实模型 JSON 分片中即时提取公开字符串；不完整转义保留到下一片。
// 【逐行批注】 声明数据结构或类型 PublicPreparationStream 及其成员接口。
class PublicPreparationStream {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 PublicPreparationStream，并通过构造函数初始化其状态。
    explicit PublicPreparationStream(std::string block);
    // 【逐行批注】 声明对象 feed，并通过构造函数初始化其状态。
    std::vector<std::string> feed(const std::string& chunk);
// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：std::string block_, buffer_;。
    std::string block_, buffer_;
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, std::string> emitted_;。
    std::map<std::string, std::string> emitted_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 基于具体学习目标和分支主题生成单节微课程，失败时由调用方返回可重试错误。
// 【逐行批注】 声明数据结构或类型 LearningGenerator 及其成员接口。
class LearningGenerator {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 LearningGenerator，并通过构造函数初始化其状态。
    explicit LearningGenerator(AIClient& client);

    // 每次只生成一个课堂板块。只有通过完整性与主题质量检查的 AI 输出才会返回。
    // 【逐行批注】 声明对象 generateBlock，并通过构造函数初始化其状态。
    nlohmann::json generateBlock(const std::string& goal, const nlohmann::json& coursePlan,
                                 // 【逐行批注】 执行当前声明或表达式：const std::string& phaseName, const std::string& topic,。
                                 const std::string& phaseName, const std::string& topic,
                                 // 【逐行批注】 执行当前声明或表达式：int topicIndex, const std::string& mode,。
                                 int topicIndex, const std::string& mode,
                                 // 【逐行批注】 执行当前声明或表达式：const std::string& block,。
                                 const std::string& block,
                                 // 【逐行批注】 执行当前声明或表达式：const nlohmann::json& previousBlocks,。
                                 const nlohmann::json& previousBlocks,
                                 // 【逐行批注】 执行当前声明或表达式：const std::vector<SearchResource>& resources,。
                                 const std::vector<SearchResource>& resources,
                                 // 【逐行批注】 计算右侧结果并写入 int maxAttempts，更新当前运行状态。
                                 int maxAttempts = 3,
                                 // 【逐行批注】 调用 bool 完成本行所需操作：std::function<bool()> cancelled = {}) const;。
                                 std::function<bool()> cancelled = {}) const;

    // 一次请求重备所有未展示板块，逐项沿用正式课堂的结构与主题检查。
    // 【逐行批注】 声明对象 adaptBlocks，并通过构造函数初始化其状态。
    nlohmann::json adaptBlocks(const std::string& goal, const nlohmann::json& coursePlan,
                               // 【逐行批注】 执行当前声明或表达式：const std::string& phaseName, const std::string& topic,。
                               const std::string& phaseName, const std::string& topic,
                               // 【逐行批注】 执行当前声明或表达式：const std::vector<std::string>& blocks,。
                               const std::vector<std::string>& blocks,
                               // 【逐行批注】 执行当前声明或表达式：const nlohmann::json& previousBlocks,。
                               const nlohmann::json& previousBlocks,
                               // 【逐行批注】 调用 bool 完成本行所需操作：std::function<bool()> cancelled) const;。
                               std::function<bool()> cancelled) const;

    // 主动备课每阶段只发起一次真实请求，公开分片经白名单过滤后交给订阅者。
    // 【逐行批注】 声明对象 generateBlockStream，并通过构造函数初始化其状态。
    nlohmann::json generateBlockStream(const std::string& goal, const nlohmann::json& coursePlan,
                                      // 【逐行批注】 执行当前声明或表达式：const std::string& phaseName, const std::string& topic,。
                                      const std::string& phaseName, const std::string& topic,
                                      // 【逐行批注】 执行当前声明或表达式：int topicIndex, const std::string& mode, const std::string& block,。
                                      int topicIndex, const std::string& mode, const std::string& block,
                                      // 【逐行批注】 执行当前声明或表达式：const nlohmann::json& previousBlocks,。
                                      const nlohmann::json& previousBlocks,
                                      // 【逐行批注】 调用 bool 完成本行所需操作：const std::function<bool(const std::string&)>& onPublicChunk,。
                                      const std::function<bool(const std::string&)>& onPublicChunk,
                                      // 【逐行批注】 调用 bool 完成本行所需操作：std::function<bool()> cancelled = {}) const;。
                                      std::function<bool()> cancelled = {}) const;

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：AIClient& client_;。
    AIClient& client_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

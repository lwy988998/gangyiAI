#pragma once

#include "ai_client.hpp"
#include "resource_types.hpp"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

namespace gangyi {

// 公开备课内容只按完整路径取字段，私有答案和评分依据不下发。
nlohmann::json publicPreparationBlock(const std::string& block, const nlohmann::json& content);

// 从真实模型 JSON 分片中即时提取公开字符串；不完整转义保留到下一片。
class PublicPreparationStream {
public:
    explicit PublicPreparationStream(std::string block);
    std::vector<std::string> feed(const std::string& chunk);
private:
    std::string block_, buffer_;
    std::map<std::string, std::string> emitted_;
};

// 基于具体学习目标和分支主题生成单节微课程，失败时由调用方返回可重试错误。
class LearningGenerator {
public:
    explicit LearningGenerator(AIClient& client);

    // 每次只生成一个课堂板块。只有通过完整性与主题质量检查的 AI 输出才会返回。
    nlohmann::json generateBlock(const std::string& goal, const nlohmann::json& coursePlan,
                                 const std::string& phaseName, const std::string& topic,
                                 int topicIndex, const std::string& mode,
                                 const std::string& block,
                                 const nlohmann::json& previousBlocks,
                                 const std::vector<SearchResource>& resources,
                                 int maxAttempts = 3,
                                 std::function<bool()> cancelled = {}) const;

    // 一次请求重备所有未展示板块，逐项沿用正式课堂的结构与主题检查。
    nlohmann::json adaptBlocks(const std::string& goal, const nlohmann::json& coursePlan,
                               const std::string& phaseName, const std::string& topic,
                               const std::vector<std::string>& blocks,
                               const nlohmann::json& previousBlocks,
                               std::function<bool()> cancelled) const;

    // 主动备课每阶段只发起一次真实请求，公开分片经白名单过滤后交给订阅者。
    nlohmann::json generateBlockStream(const std::string& goal, const nlohmann::json& coursePlan,
                                      const std::string& phaseName, const std::string& topic,
                                      int topicIndex, const std::string& mode, const std::string& block,
                                      const nlohmann::json& previousBlocks,
                                      const std::function<bool(const std::string&)>& onPublicChunk,
                                      std::function<bool()> cancelled = {}) const;

private:
    AIClient& client_;
};

}  // namespace gangyi

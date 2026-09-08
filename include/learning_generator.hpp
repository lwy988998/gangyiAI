#pragma once

#include "ai_client.hpp"
#include "resource_types.hpp"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace gangyi {

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
                                 const std::vector<SearchResource>& resources) const;

private:
    AIClient& client_;
};

}  // namespace gangyi

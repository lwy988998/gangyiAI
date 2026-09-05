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

    nlohmann::json generate(const std::string& goal, const std::string& phaseName,
                            const std::string& topic, int topicIndex,
                            const std::string& mode,
                            const std::vector<SearchResource>& resources) const;

private:
    AIClient& client_;
};

}  // namespace gangyi

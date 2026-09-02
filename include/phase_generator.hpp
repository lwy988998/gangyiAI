#pragma once

#include "ai_client.hpp"
#include "learning_generator.hpp"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace gangyi {

// 阶段展开生成（对齐 docs/phase-b-spec.md B1.7 与校园版 lib/ai/generatePhaseExpansion.ts）。
// 输入：goal/mode/phaseIndex/stage/topics/resources；输出：{objective, overview, steps[],
// tasks[], checklist[], commonMistakes[]}。
class PhaseGenerator {
public:
    explicit PhaseGenerator(AIClient& client);

    // 生成阶段展开内容；AI 失败或质量不过返回 nullopt（调用方降级展示）。
    // resources 最多取前 5 条作为参考。
    std::optional<nlohmann::json> generate(const std::string& goal, const std::string& mode,
                                           int phaseIndex, const std::string& stage,
                                           const std::vector<std::string>& topics,
                                           const std::vector<SearchResource>& resources) const;

private:
    AIClient& client_;
};

}  // namespace gangyi

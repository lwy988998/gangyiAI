// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 "ai_client.hpp"，为后续实现提供类型或接口声明。
#include "ai_client.hpp"
// 【逐行批注】 引入头文件 "resource_types.hpp"，为后续实现提供类型或接口声明。
#include "resource_types.hpp"

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <optional>，为后续实现提供类型或接口声明。
#include <optional>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 阶段展开生成（对齐 docs/phase-b-spec.md B1.7 与校园版 lib/ai/generatePhaseExpansion.ts）。
// 输入：goal/mode/phaseIndex/stage/topics/resources；输出：{objective, overview, steps[],
// tasks[], checklist[], commonMistakes[]}。
// 【逐行批注】 声明数据结构或类型 PhaseGenerator 及其成员接口。
class PhaseGenerator {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 PhaseGenerator，并通过构造函数初始化其状态。
    explicit PhaseGenerator(AIClient& client);

    // 生成阶段展开内容；最多调用 AI 三次，失败或质量不过时抛出可重试错误。
    // resources 最多取前 5 条作为参考。
    // 【逐行批注】 声明对象 generate，并通过构造函数初始化其状态。
    std::optional<nlohmann::json> generate(const std::string& goal, const std::string& mode,
                                           // 【逐行批注】 执行当前声明或表达式：int phaseIndex, const std::string& stage,。
                                           int phaseIndex, const std::string& stage,
                                           // 【逐行批注】 执行当前声明或表达式：const std::vector<std::string>& topics,。
                                           const std::vector<std::string>& topics,
                                           // 【逐行批注】 执行当前声明或表达式：const std::vector<SearchResource>& resources,。
                                           const std::vector<SearchResource>& resources,
                                           // 【逐行批注】 计算右侧结果并写入 std::string& profileContext，更新当前运行状态。
                                           const std::string& profileContext = {}) const;

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：AIClient& client_;。
    AIClient& client_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

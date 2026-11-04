// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <cstdint>，为后续实现提供类型或接口声明。
#include <cstdint>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 仅保存评估所需的题目字段；题目变更时生成新的身份，重复练习不增加独立题目数。
// 【逐行批注】 定义函数 questionSnapshot，执行该函数声明的职责。
inline nlohmann::json questionSnapshot(const nlohmann::json& question) {
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json snapshot，更新当前运行状态。
    nlohmann::json snapshot = nlohmann::json::object();
    // 【逐行批注】 按循环规则重复处理：for (const char* key : {"question", "materials", "options", "answerIndex",…。
    for (const char* key : {"question", "materials", "options", "answerIndex", "rubric", "type", "difficulty",
                            // 【逐行批注】 执行当前声明或表达式："solution", "check", "explanation", "standardAnswer"})。
                            "solution", "check", "explanation", "standardAnswer"})
        // 【逐行批注】 判断条件 (question.contains(key)) snapshot[key] = question[key];，成立时执行对应分支。
        if (question.contains(key)) snapshot[key] = question[key];
    // 【逐行批注】 结束当前函数并返回结果：return snapshot;。
    return snapshot;
// 【逐行批注】 结束当前代码块。
}
// 逐题核对用户实际看到的快照，防止后台换题后按旧下标判分。
// 【逐行批注】 定义函数 questionSnapshotsMatch，执行该函数声明的职责。
inline bool questionSnapshotsMatch(const nlohmann::json& displayed, const nlohmann::json& stored) {
    // 【逐行批注】 判断条件 (!displayed.is_array() || !stored.is_array() || displayed.size()…，成立时执行对应分支。
    if (!displayed.is_array() || !stored.is_array() || displayed.size() != stored.size()) return false;
    // 【逐行批注】 按循环规则重复处理：for (std::size_t index = 0; index < stored.size(); ++index)。
    for (std::size_t index = 0; index < stored.size(); ++index)
        // 【逐行批注】 判断条件 (questionSnapshot(displayed[index]) != questionSnapshot(stored[i…，成立时执行对应分支。
        if (questionSnapshot(displayed[index]) != questionSnapshot(stored[index])) return false;
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 questionIdentity，执行该函数声明的职责。
inline std::string questionIdentity(const std::string& scope, const nlohmann::json& snapshot) {
    // 【逐行批注】 计算右侧结果并写入 identity，更新当前运行状态。
    auto identity = snapshot;
    // 答案校正和难度描述变更不应把同一道原题算成新的独立证据。
    // 【逐行批注】 执行当前声明或表达式：identity.erase("answerIndex");。
    identity.erase("answerIndex");
    // 【逐行批注】 执行当前声明或表达式：identity.erase("difficulty");。
    identity.erase("difficulty");
    // 【逐行批注】 按循环规则重复处理：for (const char* key : {"rubric", "solution", "check", "explanation", "sta…。
    for (const char* key : {"rubric", "solution", "check", "explanation", "standardAnswer"}) identity.erase(key);
    // 【逐行批注】 计算右侧结果并写入 std::uint64_t digest，更新当前运行状态。
    std::uint64_t digest = 14695981039346656037ULL;
    // 【逐行批注】 按循环规则重复处理：for (const unsigned char byte : scope + ":" + identity.dump()) { digest ^=…。
    for (const unsigned char byte : scope + ":" + identity.dump()) { digest ^= byte; digest *= 1099511628211ULL; }
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream output;。
    std::ostringstream output;
    // 【逐行批注】 执行当前声明或表达式：output << "q-" << std::hex << digest;。
    output << "q-" << std::hex << digest;
    // 【逐行批注】 结束当前函数并返回结果：return output.str();。
    return output.str();
// 【逐行批注】 结束当前代码块。
}
// 读取旧可靠记录时校验其原身份，再统一到当前去重身份；来源不作改写。
// 【逐行批注】 定义函数 legacyQuestionIdentity，执行该函数声明的职责。
inline std::string legacyQuestionIdentity(const std::string& scope, const nlohmann::json& snapshot) {
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json identity，更新当前运行状态。
    nlohmann::json identity = nlohmann::json::object();
    // 【逐行批注】 按循环规则重复处理：for (const char* key : {"question", "options", "rubric", "type"}) if (snap…。
    for (const char* key : {"question", "options", "rubric", "type"}) if (snapshot.contains(key)) identity[key] = snapshot[key];
    // 【逐行批注】 计算右侧结果并写入 std::uint64_t digest，更新当前运行状态。
    std::uint64_t digest = 14695981039346656037ULL;
    // 【逐行批注】 按循环规则重复处理：for (const unsigned char byte : scope + ":" + identity.dump()) { digest ^=…。
    for (const unsigned char byte : scope + ":" + identity.dump()) { digest ^= byte; digest *= 1099511628211ULL; }
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream output; output << "q-" << std::hex << digest; return output.str();。
    std::ostringstream output; output << "q-" << std::hex << digest; return output.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

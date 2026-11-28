// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <optional>，为后续实现提供类型或接口声明。
#include <optional>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 计划缓存：data/ai-plan-cache/{sha256("mode:normalizedGoal")}.json，TTL 默认 7 天。
// 对齐校园版 lib/ai/planCache.ts 的行为。
// 【逐行批注】 声明数据结构或类型 PlanCache 及其成员接口。
class PlanCache {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 PlanCache，并通过构造函数初始化其状态。
    explicit PlanCache(const std::string& cacheDir = "data/ai-plan-cache");

    // 命中且新鲜且结构合法（isValidPlan）才返回 plan；否则返回 nullopt。不抛异常。
    // 【逐行批注】 声明对象 read，并通过构造函数初始化其状态。
    std::optional<nlohmann::json> read(const std::string& goal, const std::string& mode) const;

    // 结构合法才写入（写失败返回 false 但不抛异常，调用方仅记录日志）。
    // 【逐行批注】 声明对象 write，并通过构造函数初始化其状态。
    bool write(const std::string& goal, const std::string& mode, const nlohmann::json& plan) const;

    // 结构校验（对齐校园版 isValidGeneratedPlan：title/goal/durationWeeks/summary、
    // phases 非空且每项含 name/durationWeeks/objective/description/topics、
    // resources 数组形状、projects 非空数组形状）。
    // 【逐行批注】 调用 isValidPlan 完成本行所需操作：static bool isValidPlan(const nlohmann::json& plan);。
    static bool isValidPlan(const nlohmann::json& plan);

    // trim → 连续空白折叠为单空格 → 转小写。
    // 【逐行批注】 调用 normalizeGoal 完成本行所需操作：static std::string normalizeGoal(const std::string& goal);。
    static std::string normalizeGoal(const std::string& goal);

    // key = sha256_hex(normalizeMode + ":" + normalizeGoal)；mode 归一化为 lite|deep。
    // 【逐行批注】 调用 cacheKey 完成本行所需操作：static std::string cacheKey(const std::string& goal, const std::strin…。
    static std::string cacheKey(const std::string& goal, const std::string& mode);

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：std::string dir_;。
    std::string dir_;
    // 【逐行批注】 执行当前声明或表达式：long long ttlSeconds_;。
    long long ttlSeconds_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

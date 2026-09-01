#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace gangyi {

// 计划缓存：data/ai-plan-cache/{sha256("mode:normalizedGoal")}.json，TTL 默认 7 天。
// 对齐校园版 lib/ai/planCache.ts 的行为。
class PlanCache {
public:
    explicit PlanCache(const std::string& cacheDir = "data/ai-plan-cache");

    // 命中且新鲜且结构合法（isValidPlan）才返回 plan；否则返回 nullopt。不抛异常。
    std::optional<nlohmann::json> read(const std::string& goal, const std::string& mode) const;

    // 结构合法才写入（写失败返回 false 但不抛异常，调用方仅记录日志）。
    bool write(const std::string& goal, const std::string& mode, const nlohmann::json& plan) const;

    // 结构校验（对齐校园版 isValidGeneratedPlan：title/goal/durationWeeks/summary、
    // phases 非空且每项含 name/durationWeeks/objective/description/topics、
    // resources 数组形状、projects 非空数组形状）。
    static bool isValidPlan(const nlohmann::json& plan);

    // trim → 连续空白折叠为单空格 → 转小写。
    static std::string normalizeGoal(const std::string& goal);

    // key = sha256_hex(normalizeMode + ":" + normalizeGoal)；mode 归一化为 lite|deep。
    static std::string cacheKey(const std::string& goal, const std::string& mode);

private:
    std::string dir_;
    long long ttlSeconds_;
};

}  // namespace gangyi

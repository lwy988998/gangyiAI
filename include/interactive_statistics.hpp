#pragma once

#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace gangyi {

inline nlohmann::json checkedStatisticsConfig(const nlohmann::json& source) {
    using Json = nlohmann::json;
    if (source.value("type", "") != "statistics_lab" || source.value("schemaVersion", Json(1)) != 1)
        throw std::invalid_argument("统计组件类型或协议无效");
    const auto values = source.value("values", Json::array({2,3,3,4,8}));
    if (!values.is_array() || values.empty() || values.size() > 32)
        throw std::invalid_argument("统计探索需要1至32项数据，重复项也算一项");
    for (const auto& item : values) {
        if (!item.is_number()) throw std::invalid_argument("统计数据须为数值，不能是布尔值或文字");
        const auto value = item.get<double>();
        if (!std::isfinite(value) || value < -100 || value > 100 || std::floor(value * 2) != value * 2)
            throw std::invalid_argument("统计数值范围为−100至100，步长0.5");
    }
    const auto basis = source.value("varianceBasis", "population");
    if (basis != "population" && basis != "sample") throw std::invalid_argument("方差口径只支持总体或样本");
    return {{"type","statistics_lab"},{"schemaVersion",1},{"values",values},{"varianceBasis",basis}};
}

inline nlohmann::json checkedStatisticsObservation(const nlohmann::json& source,
                                                 const nlohmann::json& state, int contentVersion) {
    using Json = nlohmann::json;
    const auto config = checkedStatisticsConfig(source);
    if (contentVersion < 1 || !state.is_object() || state.size() != 4 || !state.contains("values") || !state.contains("varianceBasis") ||
        !state.value("schemaVersion", Json()).is_number_integer() || state.at("schemaVersion") != 1 ||
        !state.value("contentVersion", Json()).is_number_integer() || state.at("contentVersion") != contentVersion)
        throw std::invalid_argument("统计状态只接受当前版本的数据与方差口径，不能自报统计结果或成绩");
    auto input = config; input["values"] = state.at("values"); input["varianceBasis"] = state.at("varianceBasis");
    const auto checked = checkedStatisticsConfig(input);
    std::vector<long long> doubled;
    std::map<long long, int> counts;
    long long sum = 0, sumSquares = 0;
    for (const auto& item : checked.at("values")) {
        const auto value = static_cast<long long>(item.get<double>() * 2);
        doubled.push_back(value); ++counts[value]; sum += value; sumSquares += value * value;
    }
    std::sort(doubled.begin(), doubled.end());
    const auto n = static_cast<long long>(doubled.size());
    const auto fraction = [](long long numerator, long long denominator) {
        const auto factor = std::gcd(numerator, denominator);
        return Json{{"numerator",numerator / factor},{"denominator",denominator / factor}};
    };
    const auto approximate = [](const Json& value) { return value.at("numerator").get<double>() / value.at("denominator").get<double>(); };
    // 半整数数据先放大两倍，所有计数与平方和都在整数安全范围内完成。
    const auto mean = fraction(sum, 2*n), median = n%2 ? fraction(doubled[n/2], 2) : fraction(doubled[n/2-1]+doubled[n/2], 4);
    const auto centered = n*sumSquares - sum*sum;
    const auto population = fraction(centered, 4*n*n), sample = n>1 ? fraction(centered, 4*n*(n-1)) : Json(nullptr);
    Json modes = Json::array(), frequencies = Json::array(); int maximumCount = 0;
    for (const auto& [value,count] : counts) maximumCount = std::max(maximumCount,count);
    for (const auto& [value,count] : counts) {
        frequencies.push_back({{"value",value/2.0},{"count",count}});
        if (maximumCount > 1 && count == maximumCount) modes.push_back(value/2.0);
    }
    const auto variance = checked.at("varianceBasis") == "sample" ? sample : population;
    Json result = state;
    result.update({{"kind","exploration"},{"scoringEligible",false},{"inputSource","browser_report"},
        {"resultSource","program_calculation"},{"model","finite_univariate_descriptive_statistics"},
        {"count",n},{"sum",sum/2.0},{"minimum",doubled.front()/2.0},{"maximum",doubled.back()/2.0},
        {"range",(doubled.back()-doubled.front())/2.0},{"mean",approximate(mean)},{"median",approximate(median)},
        {"modeValues",modes},{"maxFrequency",maximumCount},{"frequencies",frequencies},
        {"populationVariance",approximate(population)},{"sampleVariance",sample.is_null()?Json(nullptr):Json(approximate(sample))},
        {"variance",variance.is_null()?Json(nullptr):Json(approximate(variance))},
        {"varianceDenominator",checked.at("varianceBasis") == "sample" ? n-1 : n},
        {"exact",{{"mean",mean},{"median",median},{"populationVariance",population},{"sampleVariance",sample}}}});
    return result;
}

} // namespace gangyi

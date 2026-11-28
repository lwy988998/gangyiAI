#pragma once

#include "exact_calculation.hpp"
#include <nlohmann/json.hpp>
#include <set>
#include <stdexcept>
#include <string>

namespace gangyi {
namespace equation_lab {
using Json = nlohmann::json;

inline long long integer(const Json& value, long long bound) {
    const auto result = exact_calculation::checkedInteger(value);
    if (result < -bound || result > bound) throw std::invalid_argument("方程参数超过当前范围");
    return result;
}

inline Json rational(const Json& expression) {
    const auto result = calculateExact(expression);
    return {{"numerator",result.at("numerator")},{"denominator",result.at("denominator")}};
}
inline Json operation(const std::string& kind, const Json& a, const Json& b) {
    return rational({{"op",kind},{"left",a},{"right",b}});
}
inline std::string text(const Json& value) {
    return std::to_string(value.at("numerator").get<long long>()) + (value.at("denominator") == 1 ? "" :
        "/" + std::to_string(value.at("denominator").get<long long>()));
}

inline Json checkedSteps(const Json& source) {
    if (!source.is_array() || source.size() > 12) throw std::invalid_argument("方程探索最多12步，可以撤销或重置后重试");
    Json result = Json::array();
    for (const auto& step : source) {
        if (!step.is_object() || step.size() != 2 || !step.contains("op") || !step.contains("value"))
            throw std::invalid_argument("每步只接受操作和一个整数参数");
        const auto op = step.at("op").get<std::string>();
        if (op != "add_x" && op != "add_constant" && op != "multiply" && op != "divide")
            throw std::invalid_argument("方程只支持两边同加一次项、同加常数、同乘或同除");
        const auto value = integer(step.at("value"), 10);
        if ((op == "multiply" || op == "divide") && value == 0)
            throw std::invalid_argument("保持同解的变形不能同乘零，也不能除以零");
        result.push_back({{"op",op},{"value",value}});
    }
    return result;
}
} // namespace equation_lab

inline nlohmann::json checkedEquationConfig(const nlohmann::json& source) {
    using namespace equation_lab;
    if (source.value("type", "") != "equation_lab" || source.value("schemaVersion", Json(1)) != 1)
        throw std::invalid_argument("方程组件类型或协议无效");
    Json result = {{"type","equation_lab"},{"schemaVersion",1}};
    for (const auto* key : {"left", "right"}) {
        const auto values = source.value(key, std::string(key) == "left" ? Json::array({2,3}) : Json::array({1,7}));
        if (!values.is_array() || values.size() != 2) throw std::invalid_argument("方程每边须为[一次项系数,常数]");
        result[key] = Json::array({integer(values[0],20),integer(values[1],20)});
    }
    return result;
}

inline nlohmann::json checkedEquationObservation(const nlohmann::json& source,
                                                const nlohmann::json& state, int contentVersion) {
    using namespace equation_lab;
    const auto config = checkedEquationConfig(source);
    if (contentVersion < 1 || !state.is_object() || state.size() != 3 || !state.contains("steps") ||
        !state.value("schemaVersion", Json()).is_number_integer() || state.at("schemaVersion") != 1 ||
        !state.value("contentVersion", Json()).is_number_integer() || state.at("contentVersion") != contentVersion)
        throw std::invalid_argument("方程状态只接受当前版本的变形步骤，不能替换原题或自报结果");
    const auto steps = checkedSteps(state.at("steps"));
    Json sides = {{"left",Json::array({rational(config.at("left")[0]),rational(config.at("left")[1])})},
                  {"right",Json::array({rational(config.at("right")[0]),rational(config.at("right")[1])})}};
    Json history = Json::array();
    for (const auto& step : steps) {
        const std::string kind = step.at("op");
        for (const auto* key : {"left", "right"}) {
            auto& side = sides[key];
            for (int i = 0; i < 2; ++i) {
                if ((kind == "add_x" && i != 0) || (kind == "add_constant" && i != 1)) continue;
                const auto result = operation(kind == "add_x" || kind == "add_constant" ? "add" : kind,
                                              side[i],step.at("value"));
                if (exact_calculation::magnitude(result.at("numerator").get<long long>()) > 1000000 ||
                    result.at("denominator").get<long long>() > 1000000)
                    throw std::invalid_argument("变形后的分子或分母超过10^6，请撤销或使用更小的数");
                side[i] = result;
            }
        }
        history.push_back({{"step",step},{"left",sides.at("left")},{"right",sides.at("right")},{"equivalent",true}});
    }
    const auto coefficient = operation("subtract",sides.at("left")[0],sides.at("right")[0]),
        constant = operation("subtract",sides.at("right")[1],sides.at("left")[1]);
    const std::string kind = coefficient.at("numerator") != 0 ? "unique" : constant.at("numerator") == 0 ? "all_real" : "none";
    const auto solution = kind == "unique" ? operation("divide",constant,coefficient) : Json(nullptr);
    const auto isolated = [&](const char* a, const char* b) {
        return sides.at(a)[0] == Json{{"numerator",1},{"denominator",1}} && sides.at(a)[1].at("numerator") == 0 &&
            sides.at(b)[0].at("numerator") == 0;
    };
    Json result = state;
    result.update({{"kind","exploration"},{"scoringEligible",false},{"inputSource","browser_report"},
        {"resultSource","program_calculation"},{"model","linear_equation_equivalent_steps"},
        {"left",sides.at("left")},{"right",sides.at("right")},{"history",history},
        {"solutionKind",kind},{"solution",solution},{"solutionExact",solution.is_null()?Json(nullptr):Json(text(solution))},
        {"isolated",kind == "unique" && (isolated("left","right") || isolated("right","left"))},
        {"equivalencePreserved",true},{"stepCount",steps.size()}});
    return result;
}

} // namespace gangyi

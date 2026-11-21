#pragma once

#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>

namespace gangyi {

// 半整数坐标在当前范围内可以精确保存点积和行列式，不接受浏览器自报结论。
inline double vectorNumber(const nlohmann::json& value, double bound) {
    if (!value.is_number()) throw std::invalid_argument("向量参数须为数值");
    const double number = value.get<double>();
    if (!std::isfinite(number) || std::abs(number) > bound || std::floor(number * 2) != number * 2)
        throw std::invalid_argument("向量参数超出范围，数值步长为0.5");
    return number;
}

inline nlohmann::json checkedVectorConfig(const nlohmann::json& source) {
    using Json = nlohmann::json;
    if (source.value("type", "") != "vector_lab" || source.value("schemaVersion", Json(1)) != 1)
        throw std::invalid_argument("向量组件类型或版本无效");
    Json config = {{"type","vector_lab"},{"schemaVersion",1}};
    for (const auto* key : {"vectorA", "vectorB"}) {
        const auto value = source.value(key, std::string(key) == "vectorA" ? Json::array({3,0}) : Json::array({0,4}));
        if (!value.is_array() || value.size() != 2) throw std::invalid_argument("平面向量须有两个坐标");
        config[key] = Json::array({vectorNumber(value[0], 10), vectorNumber(value[1], 10)});
    }
    config["scalar"] = vectorNumber(source.value("scalar", Json(2)), 3);
    const auto operation = source.value("operation", "add");
    if (operation != "add" && operation != "subtract" && operation != "scale_a")
        throw std::invalid_argument("向量只支持加、减与实数倍");
    config["operation"] = operation;
    return config;
}

inline nlohmann::json checkedVectorObservation(const nlohmann::json& source,
                                              const nlohmann::json& state, int contentVersion) {
    using Json = nlohmann::json;
    const auto config = checkedVectorConfig(source);
    const std::set<std::string> fields = {"schemaVersion", "contentVersion", "vectorA", "vectorB", "scalar", "operation"};
    if (contentVersion < 1 || !state.is_object() || state.size() != fields.size() ||
        !state.value("schemaVersion", Json()).is_number_integer() || state.at("schemaVersion") != 1 ||
        !state.value("contentVersion", Json()).is_number_integer() || state.at("contentVersion") != contentVersion)
        throw std::invalid_argument("向量探索状态已变化或缺少参数");
    auto input = config;
    for (const auto& item : state.items()) {
        if (!fields.count(item.key())) throw std::invalid_argument("向量状态不能自报结果或额外字段");
        if (item.key() != "schemaVersion" && item.key() != "contentVersion") input[item.key()] = item.value();
    }
    const auto checked = checkedVectorConfig(input);
    const double ax = checked.at("vectorA")[0], ay = checked.at("vectorA")[1],
        bx = checked.at("vectorB")[0], by = checked.at("vectorB")[1], scalar = checked.at("scalar");
    const std::string operation = checked.at("operation");
    const double rx = operation == "scale_a" ? scalar * ax : ax + (operation == "subtract" ? -bx : bx),
        ry = operation == "scale_a" ? scalar * ay : ay + (operation == "subtract" ? -by : by);
    const double normA = std::hypot(ax, ay), normB = std::hypot(bx, by), dot = ax * bx + ay * by,
        determinant = ax * by - ay * bx;
    const bool directionsDefined = normA > 0 && normB > 0;
    const double angle = determinant == 0 ? (dot < 0 ? 180 : 0) :
        std::acos(std::clamp(dot / (normA * normB), -1.0, 1.0)) * 180 / std::acos(-1.0);
    Json result = state;
    result.update({{"kind","exploration"},{"scoringEligible",false},{"inputSource","browser_report"},
        {"resultSource","program_calculation"},{"model","euclidean_plane_vectors"},
        {"vectorResult",{rx,ry}},{"normA",normA},{"normB",normB},{"resultNorm",std::hypot(rx,ry)},
        {"dotProduct",dot},{"determinant",determinant},{"directionsDefined",directionsDefined},
        {"angleDegrees",directionsDefined ? Json(angle) : Json(nullptr)},
        {"parallelDirections",directionsDefined ? Json(determinant == 0) : Json(nullptr)},
        {"perpendicularDirections",directionsDefined ? Json(dot == 0) : Json(nullptr)}});
    return result;
}

} // namespace gangyi

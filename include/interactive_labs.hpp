#pragma once

#include "interactive_subjects.hpp"
#include "interactive_vectors.hpp"
#include "interactive_equations.hpp"
#include "interactive_statistics.hpp"
#include "interactive_geometry.hpp"
#include "interactive_waves.hpp"
#include "interactive_elements.hpp"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace gangyi {

inline bool isInteractiveLab(const std::string& type) {
    return type == "function_lab" || type == "circuit_lab" || type == "chemical_balance" || type == "motion_lab" || type == "number_line" || type == "vector_lab" || type == "equation_lab" || type == "statistics_lab" || type == "triangle_lab" || type == "wave_lab" || type == "periodic_lab" || isSubjectLab(type);
}

inline double labNumber(const nlohmann::json& value, double low, double high, bool integer = false) {
    if (!value.is_number()) throw std::invalid_argument("互动参数须为数值");
    const double number = value.get<double>();
    if (!std::isfinite(number) || number < low || number > high || (integer && std::floor(number) != number))
        throw std::invalid_argument("互动参数超出当前模型的范围");
    return number;
}

struct ChemicalReaction {
    std::vector<std::string> formulas;
    std::vector<std::map<std::string, int>> atoms;
    std::size_t leftCount;
};

// 只支持已核对的教学反应；没有任意公式解析、反应预测或实验操作指令。
inline const ChemicalReaction& labReaction(const std::string& id) {
    static const std::map<std::string, ChemicalReaction> reactions = {
        {"water", {{"H₂", "O₂", "H₂O"}, {{{"H",2}}, {{"O",2}}, {{"H",2},{"O",1}}}, 2}},
        {"methane", {{"CH₄", "O₂", "CO₂", "H₂O"}, {{{"C",1},{"H",4}}, {{"O",2}}, {{"C",1},{"O",2}}, {{"H",2},{"O",1}}}, 2}},
        {"iron_oxide", {{"Fe", "O₂", "Fe₂O₃"}, {{{"Fe",1}}, {{"O",2}}, {{"Fe",2},{"O",3}}}, 2}}
    };
    const auto found = reactions.find(id);
    if (found == reactions.end()) throw std::invalid_argument("暂不支持这个反应，请选择已注册的配平示例");
    return found->second;
}

inline nlohmann::json checkedLabConfig(const nlohmann::json& source) {
    using Json = nlohmann::json;
    const auto type = source.value("type", "");
    if (type == "periodic_lab") return checkedPeriodicConfig(source);
    if (type == "wave_lab") return checkedWaveConfig(source);
    if (type == "triangle_lab") return checkedTriangleConfig(source);
    if (type == "statistics_lab") return checkedStatisticsConfig(source);
    if (type == "equation_lab") return checkedEquationConfig(source);
    if (type == "vector_lab") return checkedVectorConfig(source);
    if (isSubjectLab(type)) return checkedSubjectConfig(source);
    if (!isInteractiveLab(type) || source.value("schemaVersion", Json(1)) != 1)
        throw std::invalid_argument("互动实验类型或协议版本无效");
    Json result = {{"type", type}, {"schemaVersion", 1}};
    if (type == "function_lab") {
        const auto coefficients = source.value("coefficients", Json::array({1,0,0}));
        if (!coefficients.is_array() || coefficients.size() != 3) throw std::invalid_argument("函数须有三个系数");
        for (const auto& value : coefficients) {
            const auto number = labNumber(value, -10, 10);
            if (std::floor(number * 2) != number * 2) throw std::invalid_argument("函数探索的系数步长为0.5");
        }
        result["coefficients"] = coefficients;
        if (source.contains("domain")) {
            const auto& domain = source.at("domain");
            if (!domain.is_object() || domain.size() != 2 || !domain.contains("min") || !domain.contains("max"))
                throw std::invalid_argument("函数定义域须为含min、max的闭区间");
            const double minimum = labNumber(domain.at("min"), -20, 20), maximum = labNumber(domain.at("max"), -20, 20);
            if (minimum >= maximum) throw std::invalid_argument("函数定义域下界须小于上界");
            result["domain"] = {{"min", minimum}, {"max", maximum}};
        }
        result["xMin"] = labNumber(source.value("xMin", result.contains("domain") ? result.at("domain").at("min") : Json(-5)), -20, 20);
        result["xMax"] = labNumber(source.value("xMax", result.contains("domain") ? result.at("domain").at("max") : Json(5)), -20, 20);
        if (result.at("xMin").get<double>() >= result.at("xMax").get<double>()) throw std::invalid_argument("函数横轴范围无效");
        if (result.contains("domain") && (result.at("xMax").get<double>() <= result.at("domain").at("min").get<double>() ||
            result.at("xMin").get<double>() >= result.at("domain").at("max").get<double>()))
            throw std::invalid_argument("当前视窗无法显示定义域内的曲线，请调整横轴范围");
    } else if (type == "number_line") {
        for (const auto* key : {"lower", "upper", "point"}) {
            const auto value = source.value(key, std::string(key) == "lower" ? Json(-2) : std::string(key) == "upper" ? Json(3) : Json(0));
            if (std::string(key) != "point" && value.is_null()) { result[key] = nullptr; continue; }
            const double number = labNumber(value, -20, 20);
            if (std::floor(number * 2) != number * 2) throw std::invalid_argument("数轴数值步长为0.5");
            result[key] = number;
        }
        for (const auto* key : {"lowerClosed", "upperClosed"}) {
            const auto value = source.value(key, Json(std::string(key) == "upperClosed" && !result.at("upper").is_null()));
            if (!value.is_boolean()) throw std::invalid_argument("数轴端点的开闭须为布尔值");
            result[key] = value;
        }
        if ((!result.at("lower").is_null() && !result.at("upper").is_null() &&
            result.at("lower").get<double>() > result.at("upper").get<double>()) ||
            (result.at("lower").is_null() && result.at("lowerClosed") == true) ||
            (result.at("upper").is_null() && result.at("upperClosed") == true))
            throw std::invalid_argument("左端点不能大于右端点，无穷端不能闭合");
    } else if (type == "motion_lab") {
        result["initialPositionM"] = labNumber(source.value("initialPositionM", Json(0)), -100, 100);
        result["initialVelocityMps"] = labNumber(source.value("initialVelocityMps", Json(6)), -20, 20);
        result["accelerationMps2"] = labNumber(source.value("accelerationMps2", Json(-2)), -10, 10);
        result["timeSeconds"] = labNumber(source.value("timeSeconds", Json(0)), 0, 20);
    } else if (type == "circuit_lab") {
        const auto mode = source.value("mode", "series");
        if (mode != "series" && mode != "parallel") throw std::invalid_argument("电路只支持两电阻串联或并联");
        result["mode"] = mode;
        result["voltage"] = labNumber(source.value("voltage", Json(6)), 1, 24, true);
        result["r1"] = labNumber(source.value("r1", Json(10)), 1, 100, true);
        result["r2"] = labNumber(source.value("r2", Json(20)), 1, 100, true);
    } else {
        const auto reactionId = source.value("reactionId", "water");
        const auto& reaction = labReaction(reactionId);
        result["reactionId"] = reactionId;
        const auto coefficients = source.value("coefficients", Json(std::vector<int>(reaction.formulas.size(), 1)));
        if (!coefficients.is_array() || coefficients.size() != reaction.formulas.size()) throw std::invalid_argument("配平系数数量不符");
        for (const auto& value : coefficients) labNumber(value, 1, 8, true);
        result["coefficients"] = coefficients;
    }
    return result;
}

// 定义域独立于图像视窗；null上下界分别表示负无穷、正无穷，不表示未知。
inline nlohmann::json labFunctionAnalysis(const nlohmann::json& checked) {
    using Json = nlohmann::json;
    const auto& coefficients = checked.at("coefficients");
    const double a = coefficients.at(0), b = coefficients.at(1), c = coefficients.at(2);
    const bool bounded = checked.contains("domain");
    const double low = bounded ? checked.at("domain").at("min").get<double>() : 0;
    const double high = bounded ? checked.at("domain").at("max").get<double>() : 0;
    const auto inside = [&](double x) { return !bounded || (x >= low && x <= high); };
    const auto value = [&](double x) { return (a * x + b) * x + c; };
    Json result = {{"kind", a != 0 ? "quadratic" : b != 0 ? "linear" : "constant"},
        {"domain", bounded ? Json{{"kind","closed_interval"},{"min",low},{"max",high}} : Json{{"kind","real"}}},
        {"range", {{"min",nullptr},{"max",nullptr}}}, {"turningPoint",nullptr},
        {"zeros", {{"kind", a == 0 && b == 0 && c == 0 ? "all" : "finite"},{"values",Json::array()}}}};
    if (a != 0) {
        const double x = -b / (2 * a) + 0.0, y = value(x);
        result["turningPoint"] = {{"x",x},{"y",y},{"inDomain",inside(x)}};
        if (!bounded) result["range"][a > 0 ? "min" : "max"] = y;
        const double discriminant = b * b - 4 * a * c;
        if (discriminant >= 0) {
            const double root = std::sqrt(discriminant);
            std::vector<double> roots = {(-b - root) / (2 * a), (-b + root) / (2 * a)};
            std::sort(roots.begin(), roots.end());
            for (const double xRoot : roots) if (inside(xRoot) &&
                (result["zeros"]["values"].empty() || result["zeros"]["values"].back().get<double>() != xRoot))
                result["zeros"]["values"].push_back(xRoot + 0.0);
        }
    } else if (b != 0) {
        const double x = -c / b + 0.0;
        if (inside(x)) result["zeros"]["values"].push_back(x);
    } else result["range"] = {{"min",c},{"max",c}};
    if (bounded) {
        double minimum = std::min(value(low), value(high)), maximum = std::max(value(low), value(high));
        if (a != 0 && result.at("turningPoint").at("inDomain").get<bool>()) {
            const double y = result.at("turningPoint").at("y");
            minimum = std::min(minimum, y); maximum = std::max(maximum, y);
        }
        result["range"] = {{"min",minimum},{"max",maximum}};
    }
    return result;
}

inline nlohmann::json checkedLabObservation(const nlohmann::json& source,
                                           const nlohmann::json& state, int contentVersion) {
    using Json = nlohmann::json;
    if (source.value("type", "") == "periodic_lab") return checkedPeriodicObservation(source, state, contentVersion);
    if (source.value("type", "") == "wave_lab") return checkedWaveObservation(source, state, contentVersion);
    if (source.value("type", "") == "triangle_lab") return checkedTriangleObservation(source, state, contentVersion);
    if (source.value("type", "") == "vector_lab") return checkedVectorObservation(source, state, contentVersion);
    if (source.value("type", "") == "equation_lab") return checkedEquationObservation(source, state, contentVersion);
    if (source.value("type", "") == "statistics_lab") return checkedStatisticsObservation(source, state, contentVersion);
    if (isSubjectLab(source.value("type", ""))) return checkedSubjectObservation(source, state, contentVersion);
    const auto config = checkedLabConfig(source);
    const auto type = config.at("type").get<std::string>();
    if (!state.is_object() || !state.value("schemaVersion", Json()).is_number_integer() || state.at("schemaVersion") != 1 ||
        !state.value("contentVersion", Json()).is_number_integer() || state.at("contentVersion") != contentVersion)
        throw std::invalid_argument("实验状态版本已变化，请重新探索当前内容");
    std::set<std::string> fields = {"schemaVersion", "contentVersion", "coefficients"};
    if (type == "circuit_lab") fields = {"schemaVersion", "contentVersion", "mode", "voltage", "r1", "r2"};
    if (type == "motion_lab") fields = {"schemaVersion", "contentVersion", "initialPositionM", "initialVelocityMps", "accelerationMps2", "timeSeconds"};
    if (type == "number_line") fields = {"schemaVersion", "contentVersion", "lower", "upper", "lowerClosed", "upperClosed", "point"};
    for (const auto& item : state.items()) if (!fields.count(item.key()))
        throw std::invalid_argument("实验不能自报计算结果、成绩或额外字段");
    auto input = config;
    for (const auto& field : fields) if (field != "schemaVersion" && field != "contentVersion") {
        if (!state.contains(field)) throw std::invalid_argument("实验状态缺少当前操作参数");
        input[field] = state.at(field);
    }
    const auto checked = checkedLabConfig(input);
    Json result = state;
    result["kind"] = "exploration"; result["scoringEligible"] = false;
    result["resultSource"] = "program_calculation"; result["inputSource"] = "browser_report";
    if (type == "function_lab") {
        const auto c = checked.at("coefficients");
        const auto inDomain = [&](double x) { return !checked.contains("domain") ||
            (x >= checked.at("domain").at("min").get<double>() && x <= checked.at("domain").at("max").get<double>()); };
        result["yAtZero"] = inDomain(0) ? c.at(2) : Json(nullptr);
        result["yAtOne"] = inDomain(1) ? Json(c.at(0).get<double>() + c.at(1).get<double>() + c.at(2).get<double>()) : Json(nullptr);
        result["functionAnalysis"] = labFunctionAnalysis(checked);
    } else if (type == "number_line") {
        const bool hasLower = !checked.at("lower").is_null(), hasUpper = !checked.at("upper").is_null();
        const double lower = hasLower ? checked.at("lower").get<double>() : 0,
            upper = hasUpper ? checked.at("upper").get<double>() : 0, point = checked.at("point");
        const bool lowerClosed = checked.at("lowerClosed"), upperClosed = checked.at("upperClosed");
        const bool equal = hasLower && hasUpper && lower == upper, empty = equal && !(lowerClosed && upperClosed);
        result["intervalKind"] = empty ? "empty" : equal ? "singleton" : !hasLower && !hasUpper ? "real_line" :
            !hasLower ? "left_ray" : !hasUpper ? "right_ray" : "interval";
        result["containsPoint"] = !empty && (!hasLower || point > lower || (point == lower && lowerClosed)) &&
            (!hasUpper || point < upper || (point == upper && upperClosed));
        result["length"] = hasLower && hasUpper ? Json(upper - lower) : Json(nullptr);
    } else if (type == "motion_lab") {
        const double x = checked.at("initialPositionM"), v = checked.at("initialVelocityMps"),
            a = checked.at("accelerationMps2"), t = checked.at("timeSeconds");
        const double displacement = v * t + a * t * t / 2;
        double distance = std::abs(displacement);
        const double turningTime = a != 0 ? -v / a : -1;
        // 速度在区间内部变号时，路程分别累计往返两段。
        if (turningTime > 0 && turningTime < t) {
            const double turningDisplacement = v * turningTime + a * turningTime * turningTime / 2;
            distance = std::abs(turningDisplacement) + std::abs(displacement - turningDisplacement);
        }
        result["positionM"] = x + displacement; result["displacementM"] = displacement;
        result["velocityMps"] = v + a * t; result["speedMps"] = std::abs(v + a * t);
        result["travelDistanceM"] = distance; result["model"] = "one_dimensional_constant_acceleration";
    } else if (type == "circuit_lab") {
        const auto voltage = checked.at("voltage").get<double>(), r1 = checked.at("r1").get<double>(), r2 = checked.at("r2").get<double>();
        const bool series = checked.at("mode") == "series";
        const auto resistance = series ? r1 + r2 : r1 * r2 / (r1 + r2);
        result["equivalentResistance"] = resistance; result["totalCurrent"] = voltage / resistance;
        result["current1"] = series ? voltage / resistance : voltage / r1;
        result["current2"] = series ? voltage / resistance : voltage / r2;
        result["voltage1"] = series ? voltage * r1 / resistance : voltage;
        result["voltage2"] = series ? voltage * r2 / resistance : voltage;
        result["model"] = "ideal_dc_two_resistors";
    } else {
        const auto& reactionId = config.at("reactionId").get_ref<const std::string&>();
        const auto& reaction = labReaction(reactionId);
        Json left = Json::object(), right = Json::object();
        for (std::size_t index = 0; index < reaction.atoms.size(); ++index) for (const auto& [element, count] : reaction.atoms[index]) {
            auto& side = index < reaction.leftCount ? left : right;
            side[element] = side.value(element, 0) + count * checked.at("coefficients").at(index).get<int>();
        }
        result["leftAtoms"] = left; result["rightAtoms"] = right; result["balanced"] = left == right;
        result["reactionId"] = config.at("reactionId");
    }
    return result;
}

} // namespace gangyi

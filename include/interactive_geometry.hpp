#pragma once

#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>

namespace gangyi {

// 半整数坐标使长度平方、行列式及直角判断可精确计算，不信任前端自报结论。
inline double triangleCoordinate(const nlohmann::json& value) {
    if (!value.is_number()) throw std::invalid_argument("顶点坐标须为数值");
    const double coordinate = value.get<double>();
    if (!std::isfinite(coordinate) || std::abs(coordinate) > 10 || std::floor(coordinate * 2) != coordinate * 2)
        throw std::invalid_argument("顶点坐标须在 −10 至 10 内，步长为 0.5");
    return coordinate;
}

inline nlohmann::json checkedTriangleConfig(const nlohmann::json& source) {
    using Json = nlohmann::json;
    if (source.value("type", "") != "triangle_lab" || source.value("schemaVersion", Json(1)) != 1)
        throw std::invalid_argument("三角形组件类型或版本无效");
    const auto points = source.value("points", Json::array({Json::array({0,0}), Json::array({4,0}), Json::array({0,3})}));
    if (!points.is_array() || points.size() != 3) throw std::invalid_argument("三角形须提供三个平面顶点");
    Json checked = Json::array();
    for (const auto& point : points) {
        if (!point.is_array() || point.size() != 2) throw std::invalid_argument("每个顶点须有两个坐标");
        checked.push_back(Json::array({triangleCoordinate(point[0]), triangleCoordinate(point[1])}));
    }
    return {{"type","triangle_lab"},{"schemaVersion",1},{"points",checked}};
}

inline nlohmann::json checkedTriangleObservation(const nlohmann::json& source,
                                                 const nlohmann::json& state, int contentVersion) {
    using Json = nlohmann::json;
    const auto config = checkedTriangleConfig(source);
    const std::set<std::string> fields = {"schemaVersion", "contentVersion", "points"};
    if (contentVersion < 1 || !state.is_object() || state.size() != fields.size() ||
        !state.value("schemaVersion", Json()).is_number_integer() || state.at("schemaVersion") != 1 ||
        !state.value("contentVersion", Json()).is_number_integer() || state.at("contentVersion") != contentVersion)
        throw std::invalid_argument("三角形探索状态已变化或缺少顶点");
    for (const auto& item : state.items())
        if (!fields.count(item.key())) throw std::invalid_argument("三角形状态不能自报结果或额外字段");
    auto input = config;
    input["points"] = state.at("points");
    const auto points = checkedTriangleConfig(input).at("points");
    std::array<std::array<double,2>,3> p{};
    for (std::size_t i = 0; i < p.size(); ++i) {
        p[i][0] = points[i][0].get<double>();
        p[i][1] = points[i][1].get<double>();
    }
    const auto squareDistance = [&](int a, int b) {
        const double dx = p[a][0] - p[b][0], dy = p[a][1] - p[b][1];
        return dx * dx + dy * dy;
    };
    const double ab = squareDistance(0,1), bc = squareDistance(1,2), ca = squareDistance(2,0);
    const double cross = (p[1][0]-p[0][0])*(p[2][1]-p[0][1]) -
                         (p[1][1]-p[0][1])*(p[2][0]-p[0][0]);
    const bool valid = cross != 0;
    Json angles = nullptr, sideKind = nullptr, angleKind = nullptr;
    if (valid) {
        const auto angle = [&](int vertex, int first, int second) {
            const double ux = p[first][0]-p[vertex][0], uy = p[first][1]-p[vertex][1];
            const double vx = p[second][0]-p[vertex][0], vy = p[second][1]-p[vertex][1];
            return std::acos(std::clamp((ux*vx + uy*vy) / (std::hypot(ux,uy)*std::hypot(vx,vy)), -1.0, 1.0)) * 180 / std::acos(-1.0);
        };
        angles = {{"A",angle(0,1,2)},{"B",angle(1,0,2)},{"C",angle(2,0,1)}};
        sideKind = ab == bc && bc == ca ? "equilateral" : ab == bc || bc == ca || ca == ab ? "isosceles" : "scalene";
        std::array<double,3> ordered{ab,bc,ca};
        std::sort(ordered.begin(), ordered.end());
        const double comparison = ordered[0] + ordered[1] - ordered[2];
        angleKind = comparison == 0 ? "right" : comparison > 0 ? "acute" : "obtuse";
    }
    Json result = state;
    result.update({{"kind","exploration"},{"scoringEligible",false},{"inputSource","browser_report"},
        {"resultSource","program_calculation"},{"model","euclidean_plane_triangle"},
        {"validTriangle",valid},{"orientation",cross > 0 ? "counterclockwise" : cross < 0 ? "clockwise" : "collinear"},
        {"area",std::abs(cross)/2},{"sideLengths",{{"AB",std::sqrt(ab)},{"BC",std::sqrt(bc)},{"CA",std::sqrt(ca)}}},
        {"perimeter",valid ? Json(std::sqrt(ab)+std::sqrt(bc)+std::sqrt(ca)) : Json(nullptr)},
        {"angleDegrees",angles},{"sideKind",sideKind},{"angleKind",angleKind}});
    return result;
}

} // namespace gangyi

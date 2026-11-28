#pragma once

#include <nlohmann/json.hpp>
#include <set>
#include <stdexcept>
#include <string>

namespace gangyi {

inline const std::set<std::string>& setOperations() {
    static const std::set<std::string> operations = {"intersection", "union", "complement_a", "complement_b", "relations"};
    return operations;
}

// 受控集合配置与浏览器状态共用校验，不允许客户端自报结果或成绩。
inline nlohmann::json checkedSetMembers(const nlohmann::json& values,
                                       const std::set<std::string>* universe = nullptr) {
    if (!values.is_array() || values.size() > 16) throw std::invalid_argument("集合须为不超过16个元素的文字列表");
    std::set<std::string> seen;
    for (const auto& item : values) {
        if (!item.is_string()) throw std::invalid_argument("集合元素须为文字");
        const auto label = item.get<std::string>();
        if (label.empty() || label.size() > 64 || label.find_first_not_of(" \t\r\n") == std::string::npos ||
            !seen.insert(label).second || (universe && !universe->count(label)))
            throw std::invalid_argument("集合元素为空、重复、过长或不属于当前全集");
    }
    return values;
}

inline nlohmann::json checkedSetConfig(const nlohmann::json& config) {
    using Json = nlohmann::json;
    if (!config.is_object()) throw std::invalid_argument("集合配置须为对象");
    if (config.contains("schemaVersion") && (!config.at("schemaVersion").is_number_integer() || config.at("schemaVersion") != 1))
        throw std::invalid_argument("集合配置协议版本不受支持");
    const auto universe = checkedSetMembers(config.value("universe", Json()));
    if (universe.empty()) throw std::invalid_argument("集合探索至少需要一个全集元素");
    const std::set<std::string> allowed(universe.begin(), universe.end());
    const auto operation = config.value("operation", std::string("intersection"));
    if (!setOperations().count(operation)) throw std::invalid_argument("集合操作仅支持交、并、相对全集的补集和集合关系");
    return {{"schemaVersion", 1}, {"universe", universe},
            {"operation", operation},
            {"setA", checkedSetMembers(config.value("setA", Json()), &allowed)},
            {"setB", checkedSetMembers(config.value("setB", Json()), &allowed)}};
}

inline nlohmann::json checkedSetObservation(const nlohmann::json& config,
                                            const nlohmann::json& state, int contentVersion) {
    using Json = nlohmann::json;
    const auto checked = checkedSetConfig(config);
    if (!state.is_object() || state.value("schemaVersion", Json()) != Json(1) ||
        !state.value("schemaVersion", Json()).is_number_integer() ||
        !state.value("contentVersion", Json()).is_number_integer() ||
        state.at("contentVersion") != contentVersion)
        throw std::invalid_argument("集合操作状态版本已变化，请重新读取当前实验");
    const std::set<std::string> keys = {"schemaVersion", "contentVersion", "setA", "setB"};
    for (const auto& item : state.items()) if (!keys.count(item.key()))
        throw std::invalid_argument("集合操作不能自报计算结果、成绩或额外字段");
    const auto universe = checked.at("universe");
    const std::set<std::string> allowed(universe.begin(), universe.end());
    const auto a = checkedSetMembers(state.value("setA", Json()), &allowed);
    const auto b = checkedSetMembers(state.value("setB", Json()), &allowed);
    const std::set<std::string> membersA(a.begin(), a.end()), membersB(b.begin(), b.end());
    Json intersection = Json::array(), united = Json::array(), complementA = Json::array(), complementB = Json::array();
    bool aSubsetB = true, bSubsetA = true;
    for (const auto& value : universe) {
        const bool inA = membersA.count(value.get<std::string>()), inB = membersB.count(value.get<std::string>());
        if (inA && inB) intersection.push_back(value);
        if (inA || inB) united.push_back(value);
        if (!inA) complementA.push_back(value);
        if (!inB) complementB.push_back(value);
        if (inA && !inB) aSubsetB = false;
        if (inB && !inA) bSubsetA = false;
    }
    const bool equal = aSubsetB && bSubsetA;
    return {{"schemaVersion", 1}, {"contentVersion", contentVersion}, {"setA", a}, {"setB", b},
            {"universe", universe}, {"operation", checked.at("operation")}, {"union", united},
            {"complementA", complementA}, {"complementB", complementB},
            {"relations", {{"aSubsetB", aSubsetB}, {"bSubsetA", bSubsetA}, {"equal", equal},
                {"aProperSubsetB", aSubsetB && !equal}, {"bProperSubsetA", bSubsetA && !equal}}},
            {"intersection", intersection}, {"kind", "exploration"}, {"scoringEligible", false},
            {"inputSource", "browser_report"}, {"resultSource", "program_calculation"}};
}

} // namespace gangyi

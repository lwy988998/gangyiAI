#pragma once

#include "periodic_elements_data.hpp"
#include <nlohmann/json.hpp>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>

namespace gangyi {

inline const nlohmann::json& periodicElementsData() {
    static const auto data = nlohmann::json::parse(kPeriodicElementsJson);
    return data;
}

inline int periodicAtomicNumber(const nlohmann::json& value) {
    if (!value.is_number()) throw std::invalid_argument("原子序数须为 1 至 118 的整数");
    const auto number = value.get<double>();
    if (!std::isfinite(number) || number < 1 || number > 118 || std::floor(number) != number)
        throw std::invalid_argument("原子序数须为 1 至 118 的整数");
    return static_cast<int>(number);
}

inline nlohmann::json checkedPeriodicConfig(const nlohmann::json& source) {
    using Json = nlohmann::json;
    if (source.value("type", "") != "periodic_lab" || source.value("schemaVersion", Json(1)) != 1 ||
        source.value("datasetId", Json(kPeriodicElementsId)) != kPeriodicElementsId)
        throw std::invalid_argument("元素组件类型、协议或资料版本无效");
    const auto highlight = source.value("highlight", Json("none"));
    if (highlight != "none" && highlight != "period" && highlight != "group")
        throw std::invalid_argument("元素高亮只支持当前元素、同周期或同族");
    return {{"type", "periodic_lab"}, {"schemaVersion", 1}, {"datasetId", kPeriodicElementsId},
        {"selectedZ", periodicAtomicNumber(source.value("selectedZ", Json(11)))},
        {"comparisonZ", periodicAtomicNumber(source.value("comparisonZ", Json(17)))}, {"highlight", highlight}};
}

inline nlohmann::json periodicElementFacts(int atomicNumber) {
    const auto& row = periodicElementsData().at("elements").at(atomicNumber - 1);
    return {{"atomicNumber", atomicNumber}, {"symbol", row.at("symbol")}, {"name", row.at("name")},
        {"period", row.at("period")}, {"group", row.at("group")}, {"series", row.at("series")},
        {"neutralShells", row.at("neutralShells")}, {"protons", atomicNumber}, {"electrons", atomicNumber},
        {"neutrons", nullptr}, {"atomKind", "neutral_ground_state"}};
}

inline nlohmann::json checkedPeriodicObservation(const nlohmann::json& source,
                                                  const nlohmann::json& state, int contentVersion) {
    using Json = nlohmann::json;
    const auto config = checkedPeriodicConfig(source);
    const std::set<std::string> fields = {"schemaVersion", "contentVersion", "datasetId", "selectedZ", "comparisonZ", "highlight"};
    if (contentVersion < 1 || !state.is_object() || state.size() != fields.size() ||
        !state.value("schemaVersion", Json()).is_number_integer() || state.at("schemaVersion") != 1 ||
        !state.value("contentVersion", Json()).is_number_integer() || state.at("contentVersion") != contentVersion)
        throw std::invalid_argument("元素探索状态已变化或缺少选择");
    for (const auto& item : state.items())
        if (!fields.count(item.key())) throw std::invalid_argument("元素状态不能自报原子组成或成绩");
    auto input = config;
    for (const auto& name : {"datasetId", "selectedZ", "comparisonZ", "highlight"}) input[name] = state.at(name);
    const auto checked = checkedPeriodicConfig(input);
    const int selectedZ = checked.at("selectedZ"), comparisonZ = checked.at("comparisonZ");
    const auto selected = periodicElementFacts(selectedZ), comparison = periodicElementFacts(comparisonZ);
    Json highlighted = Json::array();
    for (const auto& row : periodicElementsData().at("elements")) {
        if (row.at("atomicNumber") == selectedZ ||
            (checked.at("highlight") == "period" && row.at("period") == selected.at("period")) ||
            (checked.at("highlight") == "group" && !selected.at("group").is_null() && row.at("group") == selected.at("group")))
            highlighted.push_back(row.at("atomicNumber"));
    }
    Json sameGroup = nullptr;
    if (!selected.at("group").is_null() && !comparison.at("group").is_null()) sameGroup = selected.at("group") == comparison.at("group");
    Json result = state;
    result.update({{"kind", "exploration"}, {"scoringEligible", false}, {"inputSource", "browser_report"},
        {"resultSource", "program_calculation"}, {"factSource", "registered_pubchem_snapshot"}, {"model", "registered_periodic_table_neutral_atoms"},
        {"selected", selected}, {"comparison", comparison}, {"samePeriod", selected.at("period") == comparison.at("period")},
        {"sameGroup", sameGroup}, {"highlightedAtomicNumbers", highlighted},
        {"sourceUrl", periodicElementsData().at("sourceUrl")}, {"sourceSha256", periodicElementsData().at("sourceSha256")}});
    return result;
}

}

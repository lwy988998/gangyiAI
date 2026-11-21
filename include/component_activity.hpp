#pragma once

#include "interactive_sets.hpp"
#include "interactive_labs.hpp"
#include "search_client.hpp"
#include <algorithm>
#include <set>

namespace gangyi {

// 浏览器报告只是一种证据来源；服务端核对归属、版本、每步状态并独立重算。
inline nlohmann::json checkedComponentActivities(const nlohmann::json& lesson, const nlohmann::json& event) {
    using Json = nlohmann::json;
    const auto supplied = event.value("componentActivities", Json::object());
    if (!supplied.is_object() || supplied.size() > 32 || supplied.dump().size() > 160000)
        throw std::invalid_argument("组件活动记录格式或大小无效");
    if (supplied.empty()) return Json::object();
    const auto& content = lesson.at("deepContent");
    const auto version = content.at("version").get<int>();
    const auto view = event.value("deepView", "lecture");
    if (view != "lecture" && view != "slides") throw std::invalid_argument("组件活动页面类型无效");
    const auto pageId = event.value("contentId", "");
    std::set<std::string> attached;
    for (const auto& page : content.at(view)) if (page.value("id", "") == pageId)
        for (const auto& id : page.value("mediaIds", Json::array())) attached.insert(id.get<std::string>());
    Json result = Json::object();
    for (const auto& entry : supplied.items()) {
        if (!attached.count(entry.key())) throw std::invalid_argument("活动组件不属于当前已保存板块");
        const Json* config = nullptr;
        for (const auto& media : content.at("media")) if (media.value("id", "") == entry.key()) { config = &media; break; }
        if (!config || (config->value("type", "") != "sets" && !isInteractiveLab(config->value("type", ""))))
            throw std::invalid_argument("活动类型没有注册受控组件");
        const auto& trace = entry.value();
        if (!trace.is_object()) throw std::invalid_argument("组件轨迹必须为对象");
        const std::set<std::string> fields = {"schemaVersion", "contentVersion", "sessionId", "totalChanges", "truncated", "events"};
        for (const auto& field : trace.items()) if (!fields.count(field.key())) throw std::invalid_argument("组件活动含未允许字段");
        const auto session = trace.value("sessionId", "");
        if (!trace.value("schemaVersion", Json()).is_number_integer() || !trace.value("contentVersion", Json()).is_number_integer() ||
            trace.value("schemaVersion", Json()) != 1 || trace.value("contentVersion", Json()) != version ||
            session.empty() || session.size() > 100 || !trace.at("totalChanges").is_number_integer() ||
            !trace.at("truncated").is_boolean() || !trace.at("events").is_array() || trace.at("events").size() > 200)
            throw std::invalid_argument("组件活动版本、序列或截断标记无效");
        const auto total = trace.at("totalChanges").get<long long>();
        if (total < 0 || total > 1000000 || (total > 0 && trace.at("events").empty()) || total < static_cast<long long>(trace.at("events").size()) ||
            trace.at("truncated").get<bool>() != (total > static_cast<long long>(trace.at("events").size())))
            throw std::invalid_argument("组件活动总量与轨迹不一致");
        Json changes = Json::array();
        long long previous = total - static_cast<long long>(trace.at("events").size());
        for (const auto& change : trace.at("events")) {
            if (!change.is_object() || change.size() != 3 || !change.contains("seq") || !change.contains("kind") || !change.contains("state") ||
                !change.at("seq").is_number_integer() || change.at("seq").get<long long>() != ++previous)
                throw std::invalid_argument("组件活动顺序或字段无效");
            const auto kind = change.at("kind").get<std::string>();
            if (kind != "change" && kind != "reset" && kind != "explain" && kind != "finish")
                throw std::invalid_argument("组件活动行为类型无效");
            const auto observation = config->value("type", "") == "sets" ? checkedSetObservation(*config, change.at("state"), version)
                : checkedLabObservation(*config, change.at("state"), version);
            changes.push_back({{"seq", previous}, {"kind", kind}, {"state", change.at("state")}, {"observation", observation}});
        }
        if (!changes.empty()) {
            const auto states = event.value("mediaState", Json::object());
            if (!states.contains(entry.key()) || states.at(entry.key()) != changes.back().at("state"))
                throw std::invalid_argument("活动最后状态与当前组件状态不一致");
        }
        const auto traceId = resourceDigest(Json::array({lesson.at("id"), version, entry.key(), session, event.at("requestId")}).dump());
        result[entry.key()] = {{"schemaVersion", 1}, {"traceId", traceId}, {"componentId", entry.key()},
            {"lessonId", lesson.at("id")}, {"contentId", pageId}, {"contentVersion", version}, {"sessionId", session},
            {"totalChanges", total}, {"truncated", trace.at("truncated")}, {"events", changes},
            {"inputSource", "browser_report_validated"}, {"resultSource", "program_calculation"}, {"scoringEligible", false}};
    }
    return result;
}

inline nlohmann::json componentActivitySummaries(const nlohmann::json& activities) {
    using Json = nlohmann::json;
    Json result = Json::object();
    for (const auto& entry : activities.items()) {
        auto summary = entry.value();
        const auto events = summary.at("events"); summary.erase("events");
        Json recent = Json::array();
        const auto begin = events.size() > 20 ? events.size() - 20 : 0;
        for (std::size_t index = begin; index < events.size(); ++index) recent.push_back(events[index]);
        summary["recentEvents"] = recent; summary["storedEvents"] = events.size();
        summary["detailsTool"] = "read_component_activity";
        summary["summaryTruncated"] = events.size() > 20;
        if (!events.empty()) {
            summary["firstObserved"] = events.front().at("observation");
            summary["lastObserved"] = events.back().at("observation");
        }
        summary["limitations"] = "记录反映浏览器上报并经程序校验的操作；不能单凭操作推断掌握、动机或成绩。";
        result[entry.key()] = summary;
    }
    return result;
}

}

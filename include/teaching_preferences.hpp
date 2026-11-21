#pragma once

#include <nlohmann/json.hpp>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

namespace gangyi {

inline bool isTeachingPreferenceIntent(const std::string& value) {
    static const std::set<std::string> values = {"simplify", "slow_down", "speed_up", "give_example",
                                               "independent_attempt", "resume_guidance", "reset_preferences"};
    return values.count(value) != 0;
}

// 只返回固定枚举和版本，不把损坏记录、题目答案或任意偏好指令带入公开视图。
inline nlohmann::json teachingPreferencesProjection(const nlohmann::json& lesson) {
    using Json = nlohmann::json;
    Json result = {{"version", 0}, {"pace", "normal"}, {"explanation", "standard"},
                   {"guidance", "guided"}, {"lastIntent", nullptr}};
    const auto found = lesson.find("teachingPreferences");
    if (found == lesson.end() || !found->is_object()) return result;
    const auto version = found->value("version", Json());
    if (!version.is_number_integer() || (version.is_number_unsigned() ? version.get<std::uint64_t>() > 1000000000ULL
        : version.get<std::int64_t>() < 0 || version.get<std::int64_t>() > 1000000000LL)) return result;
    for (const auto& [field, allowed] : std::map<std::string, std::set<std::string>>{
        {"pace", {"normal", "slow", "fast"}}, {"explanation", {"standard", "simple"}},
        {"guidance", {"guided", "independent"}}}) {
        const auto value = found->value(field, Json());
        if (!value.is_string() || !allowed.count(value.get<std::string>())) return result;
    }
    for (const auto* key : {"version", "pace", "explanation", "guidance"}) result[key] = found->at(key);
    const auto intent = found->value("lastIntent", Json());
    if (intent.is_string() && isTeachingPreferenceIntent(intent.get<std::string>())) result["lastIntent"] = intent;
    return result;
}

inline nlohmann::json checkedTeachingPreferenceIntent(const nlohmann::json& lesson,
                                                     const nlohmann::json& event) {
    using Json = nlohmann::json;
    const auto intent = event.value("requestType", "");
    if (event.value("type", "") != "teaching_request" || !isTeachingPreferenceIntent(intent))
        throw std::invalid_argument("教学意图类型无效");
    if (!lesson.value("entered", false) || lesson.value("status", "") != "ready" ||
        lesson.value("id", "") != event.value("lessonId", "") ||
        lesson.value("courseId", "") != event.value("courseId", ""))
        throw std::invalid_argument("教学偏好只能用于已经进入的当前课时");
    auto next = teachingPreferencesProjection(lesson);
    const auto expected = event.value("teachingPreferenceVersion", Json());
    if (!expected.is_number_integer() || expected != next.at("version"))
        throw std::invalid_argument("本课教学偏好已变化，请刷新后重新选择");
    if (event.contains("teachingPreferences") || event.contains("score") || event.contains("masteryStatus"))
        throw std::invalid_argument("教学意图不能自报偏好状态、成绩或掌握程度");
    if (next.at("version").get<std::int64_t>() >= 1000000000LL)
        throw std::invalid_argument("教学偏好版本已达上限，不能覆盖现有状态");
    next["version"] = next.at("version").get<std::int64_t>() + 1;
    next["lastIntent"] = intent;
    if (intent == "simplify") next["explanation"] = "simple";
    if (intent == "slow_down") next["pace"] = "slow";
    if (intent == "speed_up") next["pace"] = "fast";
    if (intent == "independent_attempt") next["guidance"] = "independent";
    if (intent == "resume_guidance") next["guidance"] = "guided";
    if (intent == "reset_preferences") {
        next["pace"] = "normal"; next["explanation"] = "standard"; next["guidance"] = "guided";
    }
    // 举例是本次教学要求，只记录意图；不能擅自重置学生已选择的节奏和讲解方式。
    return next;
}

} // namespace gangyi

#pragma once
#include <nlohmann/json.hpp>
#include <map>
#include <set>
#include <string>

namespace gangyi {
// 只精简发送给模型的副本，完整学习记录及按需读取接口保持原样。
inline nlohmann::json agentPromptContext(nlohmann::json context, bool assessment) {
    using Json = nlohmann::json;
    const auto event = context.value("event", Json::object());
    if (event.value("lessonId", "").empty()) return context;
    const auto courseId = event.value("courseId", "");
    Json courses = Json::array();
    for (const auto& item : context.value("courses", Json::array())) if (item.value("id", "") == courseId) {
        auto course = item; course.erase("outline"); courses.push_back(std::move(course));
    }
    context["courses"] = courses;
    Json feedback = Json::array(); std::size_t omitted = 0;
    const std::size_t recentLimit = assessment ? 3 : 8;
    for (const auto& item : context.value("feedback", Json::array())) {
        const auto payload = item.value("payload", Json::object());
        if (item.value("courseId", "") != courseId || payload.value("requestId", "") == event.value("requestId", "") ||
            feedback.size() >= recentLimit) { ++omitted; continue; }
        Json saved = Json::object();
        for (const auto* key : {"text", "type", "response", "questionId", "questionSnapshot", "score", "credible", "feedback", "reason",
                               "diagnosis", "helpMode", "rePracticeOf", "status", "results", "action"})
            if (payload.contains(key)) saved[key] = payload.at(key);
        auto record = item; record["payload"] = std::move(saved); feedback.push_back(std::move(record));
    }
    context["feedback"] = feedback;
    context.erase("studyPlan"); context.erase("adjustments"); context.erase("reviews");
    context["contextAvailability"] = {{"omittedFeedback", omitted}, {"historyTool", "read_history"},
        {"fullContextTool", "read_context"}, {"outlineTool", "read_outline"}};
    if (assessment) context.erase("abilities");
    return context;
}

// 同一任务内内容完全相同的只读结果仅保留一次，写入凭据与错误原文保留供修正。
inline nlohmann::json agentPromptHistory(const nlohmann::json& messages, const nlohmann::json& actions,
                                         const std::set<std::string>& readOnlyTools) {
    using Json = nlohmann::json;
    Json result = messages;
    std::map<std::string, std::string> seen;
    for (auto& message : result) {
        if (message.value("role", "") != "user") continue;
        auto content = Json::parse(message.value("content", ""), nullptr, false);
        if (!content.is_object() || !content.value("toolResults", Json()).is_array()) continue;
        for (auto& item : content["toolResults"]) {
            const auto id = item.value("id", "");
            if (!actions.contains(id)) continue;
            const auto encoded = Json::parse(actions.at(id).value("encoded", ""), nullptr, false);
            if (!encoded.is_object() || !readOnlyTools.count(encoded.value("tool", ""))) continue;
            const auto key = encoded.dump() + ":" + item.value("result", Json()).dump();
            const auto previous = seen.find(key);
            if (previous == seen.end()) seen[key] = id;
            else item["result"] = {{"unchangedFrom", previous->second}};
        }
        message["content"] = content.dump();
    }
    return result;
}
}

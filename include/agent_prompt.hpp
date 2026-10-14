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
    if (event.value("type", "") == "library_research") {
        // 检索过程的原文已在工具结果中按段返回，避免每轮再携带整批候选的正文。
        auto& library = context["savedLibrary"];
        for (auto& source : library["sources"]) {
            Json brief = Json::object();
            for (const auto* key : {"id", "title", "url", "source", "type", "language", "publishedAt", "sourceUpdatedAt",
                                   "readStatus", "snapshotId", "lastCheckedAt", "contentGroup", "choice"})
                if (source.contains(key)) brief[key] = source.at(key);
            brief["detailsTool"] = "read_resource";
            source = std::move(brief);
        }
    }
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
    if (assessment) { context.erase("abilities"); context.erase("savedLibrary"); }
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
        if (!content.is_object()) continue;
        const auto compact = [&](const std::string& id, Json& value) {
            if (!actions.contains(id)) return;
            const auto encoded = Json::parse(actions.at(id).value("encoded", ""), nullptr, false);
            if (!encoded.is_object() || !readOnlyTools.count(encoded.value("tool", ""))) return;
            const auto key = encoded.dump() + ":" + value.dump();
            const auto previous = seen.find(key);
            if (previous == seen.end()) seen[key] = id;
            else value = {{"unchangedFrom", previous->second}};
        };
        if (content.value("toolResults", Json()).is_array()) for (auto& item : content["toolResults"]) {
            compact(item.value("id", ""), item["result"]);
        }
        // 失败恢复记录保留原始错误和已执行的行动；已读正文用之前的真实工具结果引用。
        if (content.value("completedActions", Json()).is_object()) for (auto& item : content["completedActions"].items()) {
            if (item.value().contains("result")) compact(item.key(), item.value()["result"]);
        }
        message["content"] = content.dump();
    }
    return result;
}
}

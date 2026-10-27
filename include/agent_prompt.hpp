#pragma once
#include <nlohmann/json.hpp>
#include <map>
#include <set>
#include <string>
#include "agent_efficiency.hpp"

namespace gangyi {
inline constexpr const char* kAgentPromptRevision = "teaching-accuracy-equivalence-2026-10-27-r2";
// 诊断会重建提示；共用这段约束，避免评价与后续讲解使用不同定义。
inline constexpr const char* kAgentTeachingAccuracyRule =
    u8"公开反馈、讲解和示范应使用准确且一致的术语，区分定义与由定义推得的含义。"
    u8"例如，分母表示把同一整体平均分成的份数，每份相当于整体的1/分母；不能直接把分母定义为每份的大小。"
    u8"生成私有评分标准和评价时，以实际题意为界，不把参考答案采用的一种正确方法限定为唯一方法；题目未明确要求最简形式或特定方法时，应接受正确的等价表示与其它合法解法。"
    u8"例如只要求通分或说明公分母时，任取原分母的公倍数正确通分并得到等价结果都应认可，不能擅自把最小公倍数作为满分必要条件；若题目明确要求最小公分母，则按该要求评价。"
    u8"结构或动作校验失败的回复尚未展示或保存，不能把其中的讲解、示范或新题当作已完成；恢复时依据成功回执补齐必要教学与学生明确的讲解偏好，再进入下一步。"
    u8"历史已展示表述有误时新增明确的更正说明，保留原文，不改原题、真实回答、评分依据和实际帮助记录。";

// 只精简发送给模型的副本，完整学习记录及按需读取接口保持原样。
inline nlohmann::json agentPromptContext(nlohmann::json context, bool assessment) {
    using Json = nlohmann::json;
    if (!agentEfficiencyEnabled("GANGYI_AGENT_CONTEXT_PROJECTION")) return context;
    const auto event = context.value("event", Json::object());
    const bool research = event.value("type", "") == "library_research";
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
    const auto courseId = event.value("courseId", "");
    const bool scheduling = event.value("type", "") == "schedule_replan" || event.value("type", "") == "assistant_chat";
    Json omittedCourses = Json::array(), omittedRecords = Json::array(), omittedMemories = Json::array();
    if (!scheduling) {
        Json courses = Json::array();
        for (auto course : context.value("courses", Json::array())) {
            if (!courseId.empty() && course.value("id", "") != courseId) {
                omittedCourses.push_back(course.value("id", "")); continue;
            }
            if (event.value("type", "") != "prepare_next" && event.value("type", "") != "plan_course") course.erase("outline");
            courses.push_back(std::move(course));
        }
        context["courses"] = std::move(courses);
    }
    Json feedback = Json::array();
    const std::size_t recentLimit = assessment ? 3 : 8;
    for (const auto& item : context.value("feedback", Json::array())) {
        const auto payload = item.value("payload", Json::object());
        const bool current = !event.value("requestId", "").empty() && payload.value("requestId", "") == event.value("requestId", "");
        if ((!courseId.empty() && item.value("courseId", "") != courseId) || current || feedback.size() >= recentLimit) {
            omittedRecords.push_back(item.value("id", "")); continue;
        }
        // 历史沿用既有精确字段投影；当前原题、回答和实际帮助由 currentInputs 完整直送。
        Json brief = Json::object();
        for (const auto* key : {"text", "type", "response", "questionId", "questionSnapshot", "score", "credible", "feedback", "reason",
                               "diagnosis", "helpMode", "rePracticeOf", "status", "results", "action", "citations", "materialOrigin"})
            if (payload.contains(key)) brief[key] = payload.at(key);
        auto record = item; record["payload"] = std::move(brief);
        record["originalRecord"] = research ? Json{{"tool", "read_context"}, {"recordId", item.value("id", "")}}
            : Json{{"tool", "read_history"}, {"ids", {item.value("id", "")}}};
        feedback.push_back(std::move(record));
    }
    context["feedback"] = std::move(feedback);
    if (!courseId.empty() && context.value("personalCenter", Json::object()).value("memories", Json()).is_array()) {
        Json memories = Json::array();
        for (const auto& memory : context["personalCenter"]["memories"]) {
            // 旧记录没有明确课程归属时保留，不能靠关键词猜测删除偏好。
            const auto owner = memory.value("courseId", "");
            if (!owner.empty() && owner != courseId && memory.value("source", "") != "manual")
                omittedMemories.push_back(memory.value("id", ""));
            else memories.push_back(memory);
        }
        context["personalCenter"]["memories"] = std::move(memories);
    }
    if (!scheduling && !event.value("lessonId", "").empty()) {
        context.erase("studyPlan"); context.erase("adjustments"); context.erase("reviews");
    }
    context["contextAvailability"] = {{"omittedFeedback", omittedRecords.size()}, {"recordIds", omittedRecords},
        {"courseIds", omittedCourses}, {"memoryIds", omittedMemories}, {"historyTool", research ? "read_context" : "read_history"},
        {"fullContextTool", "read_context"}, {"outlineTool", research ? "read_context" : "read_outline"}, {"resourceTool", "read_resource"},
        {"instruction", research
            ? "历史是字段投影；资料任务用read_context补读完整载荷并按记录ID定位。read_resource用sourceId/snapshotId回查原文，未载入不等于不存在。"
            : assessment ? "currentInputs完整直送本次原题、回答和实际帮助。本轮仅诊断；历史字段投影的原件可在后续教学阶段用read_history({ids})补读，不据未载入字段断言不存在。"
            : "历史是字段投影，需要完整帮助或相关讲解时用read_history({ids})补读。currentInputs保留本次完整主证据；read_context可传courseId补读授权课程，read_resource用sourceId/snapshotId回查原文。未载入不等于不存在。"}};
    if (assessment) { context.erase("abilities"); context.erase("savedLibrary"); }
    return context;
}

// 相同只读结果和重复恢复凭据只发送一次，数据库中的原始记录保持完整。
inline nlohmann::json agentPromptHistory(const nlohmann::json& messages, const nlohmann::json& actions,
                                         const std::set<std::string>& readOnlyTools) {
    using Json = nlohmann::json;
    if (!agentEfficiencyEnabled("GANGYI_AGENT_RECEIPT_DEDUP")) return messages;
    Json result = messages;
    std::map<std::string, std::string> seen;
    std::map<std::string, Json> receipts;
    const auto referenceIfSmaller = [](const std::string& id, Json& value) {
        Json reference = {{"unchangedFrom", id}};
        if (reference.dump().size() < value.dump().size()) value = std::move(reference);
    };
    for (auto& message : result) {
        if (message.value("role", "") != "user") continue;
        auto content = Json::parse(message.value("content", ""), nullptr, false);
        if (!content.is_object()) continue;
        const auto compact = [&](const std::string& id, Json& value) {
            if (!actions.contains(id)) return;
            const auto encoded = Json::parse(actions.at(id).value("encoded", ""), nullptr, false);
            if (!encoded.is_object() || !readOnlyTools.count(encoded.value("tool", ""))) return;
            // 默认参数显式写出时，仍可引用同一工具已经发送过的完全相同回执；原行动参数保留。
            const auto key = encoded.at("tool").dump() + ":" + value.dump();
            const auto previous = seen.find(key);
            if (previous == seen.end()) seen[key] = id;
            else referenceIfSmaller(previous->second, value);
        };
        if (content.value("toolResults", Json()).is_array()) for (auto& item : content["toolResults"]) {
            const auto id = item.value("id", "");
            // 首次实际发送的凭据保留，不能仅因数据库存在缓存就删除。
            if (actions.contains(id)) receipts[id] = {{"encoded", actions.at(id).value("encoded", "")}, {"result", item.value("result", Json())}};
            compact(id, item["result"]);
        }
        for (const auto* field : {"completedActions", "savedActions"}) {
            if (!content.value(field, Json()).is_object()) continue;
            for (auto& item : content[field].items()) {
                auto& receipt = item.value();
                if (!receipt.is_object() || !receipt.contains("encoded") || !receipt.contains("result")) continue;
                const auto previous = receipts.find(item.key());
                if (previous != receipts.end() && previous->second == receipt)
                    referenceIfSmaller(item.key(), receipt["result"]);
                else {
                    receipts[item.key()] = receipt;
                    compact(item.key(), receipt["result"]);
                }
            }
        }
        message["content"] = content.dump();
    }
    return result;
}
}

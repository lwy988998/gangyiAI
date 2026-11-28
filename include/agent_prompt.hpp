// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 "agent_efficiency.hpp"，为后续实现提供类型或接口声明。
#include "agent_efficiency.hpp"
#include "teaching_policy.hpp"

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 计算右侧结果并写入 inline constexpr const char* kAgentPromptRev…，更新当前运行状态。
inline constexpr const char* kAgentPromptRevision = "interactive-evidence-2026-11-20-r2";
// 诊断会重建提示；共用这段约束，避免评价与后续讲解使用不同定义。
// 【逐行批注】 计算右侧结果并写入 inline constexpr const char* kAgentTeachingA…，更新当前运行状态。
inline const char* kAgentTeachingAccuracyRule = teachingPromptModule("evaluation/factuality-checker").body;

// 只精简发送给模型的副本，完整学习记录及按需读取接口保持原样。
// 【逐行批注】 定义函数 agentPromptContext，执行该函数声明的职责。
inline nlohmann::json agentPromptContext(nlohmann::json context, bool assessment) {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 【逐行批注】 判断条件 (!agentEfficiencyEnabled("GANGYI_AGENT_CONTEXT_PROJECTION")) ret…，成立时执行对应分支。
    if (!agentEfficiencyEnabled("GANGYI_AGENT_CONTEXT_PROJECTION")) return context;
    // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
    const auto event = context.value("event", Json::object());
    // 【逐行批注】 计算右侧结果并写入 bool research，更新当前运行状态。
    const bool research = event.value("type", "") == "library_research";
    // 【逐行批注】 判断条件 (event.value("type", "") == "library_research") {，成立时执行对应分支。
    if (event.value("type", "") == "library_research") {
        // 检索过程的原文已在工具结果中按段返回，避免每轮再携带整批候选的正文。
        // 【逐行批注】 计算右侧结果并写入 auto& library，更新当前运行状态。
        auto& library = context["savedLibrary"];
        // 【逐行批注】 按循环规则重复处理：for (auto& source : library["sources"]) {。
        for (auto& source : library["sources"]) {
            // 【逐行批注】 计算右侧结果并写入 Json brief，更新当前运行状态。
            Json brief = Json::object();
            // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"id", "title", "url", "source", "type", "language"…。
            for (const auto* key : {"id", "title", "url", "source", "type", "language", "publishedAt", "sourceUpdatedAt",
                                   // 【逐行批注】 执行当前声明或表达式："readStatus", "snapshotId", "lastCheckedAt", "contentGroup", "choice"})。
                                   "readStatus", "snapshotId", "lastCheckedAt", "contentGroup", "choice"})
                // 【逐行批注】 判断条件 (source.contains(key)) brief[key] = source.at(key);，成立时执行对应分支。
                if (source.contains(key)) brief[key] = source.at(key);
            // 【逐行批注】 计算右侧结果并写入 brief["detailsTool"]，更新当前运行状态。
            brief["detailsTool"] = "read_resource";
            // 【逐行批注】 计算右侧结果并写入 source，更新当前运行状态。
            source = std::move(brief);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
    const auto courseId = event.value("courseId", "");
    // 【逐行批注】 计算右侧结果并写入 bool scheduling，更新当前运行状态。
    const bool scheduling = event.value("type", "") == "schedule_replan" || event.value("type", "") == "assistant_chat";
    // 【逐行批注】 计算右侧结果并写入 Json omittedCourses，更新当前运行状态。
    Json omittedCourses = Json::array(), omittedRecords = Json::array(), omittedMemories = Json::array();
    // 【逐行批注】 判断条件 (!scheduling) {，成立时执行对应分支。
    if (!scheduling) {
        // 【逐行批注】 计算右侧结果并写入 Json courses，更新当前运行状态。
        Json courses = Json::array();
        // 【逐行批注】 按循环规则重复处理：for (auto course : context.value("courses", Json::array())) {。
        for (auto course : context.value("courses", Json::array())) {
            // 【逐行批注】 判断条件 (!courseId.empty() && course.value("id", "") != courseId) {，成立时执行对应分支。
            if (!courseId.empty() && course.value("id", "") != courseId) {
                // 【逐行批注】 执行当前声明或表达式：omittedCourses.push_back(course.value("id", "")); continue;。
                omittedCourses.push_back(course.value("id", "")); continue;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (event.value("type", "") != "prepare_next" && event.value("type"…，成立时执行对应分支。
            if (event.value("type", "") != "prepare_next" && event.value("type", "") != "plan_course") course.erase("outline");
            // 【逐行批注】 调用 move 完成本行所需操作：courses.push_back(std::move(course));。
            courses.push_back(std::move(course));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 context["courses"]，更新当前运行状态。
        context["courses"] = std::move(courses);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 Json feedback，更新当前运行状态。
    Json feedback = Json::array();
    // 【逐行批注】 计算右侧结果并写入 std::size_t recentLimit，更新当前运行状态。
    const std::size_t recentLimit = assessment ? 3 : 8;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : context.value("feedback", Json::array())) {。
    for (const auto& item : context.value("feedback", Json::array())) {
        // 【逐行批注】 计算右侧结果并写入 auto payload，更新当前运行状态。
        const auto payload = item.value("payload", Json::object());
        // 【逐行批注】 计算右侧结果并写入 bool current，更新当前运行状态。
        const bool current = !event.value("requestId", "").empty() && payload.value("requestId", "") == event.value("requestId", "");
        // 【逐行批注】 判断条件 ((!courseId.empty() && item.value("courseId", "") != courseId) |…，成立时执行对应分支。
        if ((!courseId.empty() && item.value("courseId", "") != courseId) || current || feedback.size() >= recentLimit) {
            // 【逐行批注】 执行当前声明或表达式：omittedRecords.push_back(item.value("id", "")); continue;。
            omittedRecords.push_back(item.value("id", "")); continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 历史沿用既有精确字段投影；当前原题、回答和实际帮助由 currentInputs 完整直送。
        // 【逐行批注】 计算右侧结果并写入 Json brief，更新当前运行状态。
        Json brief = Json::object();
        // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"text", "type", "response", "questionId", "questio…。
        for (const auto* key : {"text", "type", "response", "questionId", "questionSnapshot", "score", "credible", "feedback", "reason",
                               // 【逐行批注】 执行当前声明或表达式："diagnosis", "helpMode", "rePracticeOf", "status", "results", "action", "citations",…。
                               "diagnosis", "helpMode", "rePracticeOf", "status", "results", "action", "citations", "materialOrigin"})
            // 【逐行批注】 判断条件 (payload.contains(key)) brief[key] = payload.at(key);，成立时执行对应分支。
            if (payload.contains(key)) brief[key] = payload.at(key);
        // 【逐行批注】 计算右侧结果并写入 record，更新当前运行状态。
        auto record = item; record["payload"] = std::move(brief);
        // 【逐行批注】 计算右侧结果并写入 record["originalRecord"]，更新当前运行状态。
        record["originalRecord"] = research ? Json{{"tool", "read_context"}, {"recordId", item.value("id", "")}}
            // 【逐行批注】 执行当前声明或表达式：: Json{{"tool", "read_history"}, {"ids", {item.value("id", "")}}};。
            : Json{{"tool", "read_history"}, {"ids", {item.value("id", "")}}};
        // 【逐行批注】 调用 move 完成本行所需操作：feedback.push_back(std::move(record));。
        feedback.push_back(std::move(record));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 context["feedback"]，更新当前运行状态。
    context["feedback"] = std::move(feedback);
    // 【逐行批注】 判断条件 (!courseId.empty() && context.value("personalCenter", Json::obje…，成立时执行对应分支。
    if (!courseId.empty() && context.value("personalCenter", Json::object()).value("memories", Json()).is_array()) {
        // 【逐行批注】 计算右侧结果并写入 Json memories，更新当前运行状态。
        Json memories = Json::array();
        // 【逐行批注】 按循环规则重复处理：for (const auto& memory : context["personalCenter"]["memories"]) {。
        for (const auto& memory : context["personalCenter"]["memories"]) {
            // 旧记录没有明确课程归属时保留，不能靠关键词猜测删除偏好。
            // 【逐行批注】 计算右侧结果并写入 auto owner，更新当前运行状态。
            const auto owner = memory.value("courseId", "");
            // 【逐行批注】 判断条件 (!owner.empty() && owner != courseId && memory.value("source", "…，成立时执行对应分支。
            if (!owner.empty() && owner != courseId && memory.value("source", "") != "manual")
                // 【逐行批注】 执行当前声明或表达式：omittedMemories.push_back(memory.value("id", ""));。
                omittedMemories.push_back(memory.value("id", ""));
            // 【逐行批注】 执行前一条件不成立时的分支。
            else memories.push_back(memory);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 context["personalCenter"]["memories"]，更新当前运行状态。
        context["personalCenter"]["memories"] = std::move(memories);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!scheduling && !event.value("lessonId", "").empty()) {，成立时执行对应分支。
    if (!scheduling && !event.value("lessonId", "").empty()) {
        // 【逐行批注】 执行当前声明或表达式：context.erase("studyPlan"); context.erase("adjustments"); context.erase("reviews");。
        context.erase("studyPlan"); context.erase("adjustments"); context.erase("reviews");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 context["contextAvailability"]，更新当前运行状态。
    context["contextAvailability"] = {{"omittedFeedback", omittedRecords.size()}, {"recordIds", omittedRecords},
        // 【逐行批注】 执行当前声明或表达式：{"courseIds", omittedCourses}, {"memoryIds", omittedMemories}, {"historyTool", resea…。
        {"courseIds", omittedCourses}, {"memoryIds", omittedMemories}, {"historyTool", research ? "read_context" : "read_history"},
        // 【逐行批注】 执行当前声明或表达式：{"fullContextTool", "read_context"}, {"outlineTool", research ? "read_context" : "re…。
        {"fullContextTool", "read_context"}, {"outlineTool", research ? "read_context" : "read_outline"}, {"resourceTool", "read_resource"},
        // 【逐行批注】 执行当前声明或表达式：{"instruction", research。
        {"instruction", research
            // 【逐行批注】 执行当前声明或表达式：? "历史是字段投影；资料任务用read_context补读完整载荷并按记录ID定位。read_resource用sourceId/snapshotId回查原文，未载入…。
            ? "历史是字段投影；资料任务用read_context补读完整载荷并按记录ID定位。read_resource用sourceId/snapshotId回查原文，未载入不等于不存在。"
            // 【逐行批注】 执行当前声明或表达式：: assessment ? "currentInputs完整直送本次原题、回答和实际帮助。本轮仅诊断；历史字段投影的原件可在后续教学阶段用read_history({…。
            : assessment ? "currentInputs完整直送本次原题、回答和实际帮助。本轮仅诊断；历史字段投影的原件可在后续教学阶段用read_history({ids})补读，不据未载入字段断言不存在。"
            // 【逐行批注】 执行当前声明或表达式：: "历史是字段投影，需要完整帮助或相关讲解时用read_history({ids})补读。currentInputs保留本次完整主证据；read_context可传c…。
            : "历史是字段投影，需要完整帮助或相关讲解时用read_history({ids})补读。currentInputs保留本次完整主证据；read_context可传courseId补读授权课程，read_resource用sourceId/snapshotId回查原文。未载入不等于不存在。"}};
    // 【逐行批注】 判断条件 (assessment) { context.erase("abilities"); context.erase("savedL…，成立时执行对应分支。
    if (assessment) { context.erase("abilities"); context.erase("savedLibrary"); }
    // 【逐行批注】 结束当前函数并返回结果：return context;。
    return context;
// 【逐行批注】 结束当前代码块。
}

// 相同只读结果和重复恢复凭据只发送一次，数据库中的原始记录保持完整。
// 【逐行批注】 调用 agentPromptHistory 完成本行所需操作：inline nlohmann::json agentPromptHistory(const nlohmann::json& messag…。
inline nlohmann::json agentPromptHistory(const nlohmann::json& messages, const nlohmann::json& actions,
                                         // 【逐行批注】 执行当前声明或表达式：const std::set<std::string>& readOnlyTools) {。
                                         const std::set<std::string>& readOnlyTools) {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 【逐行批注】 判断条件 (!agentEfficiencyEnabled("GANGYI_AGENT_RECEIPT_DEDUP")) return m…，成立时执行对应分支。
    if (!agentEfficiencyEnabled("GANGYI_AGENT_RECEIPT_DEDUP")) return messages;
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = messages;
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, std::string> seen;。
    std::map<std::string, std::string> seen;
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, Json> receipts;。
    std::map<std::string, Json> receipts;
    // 【逐行批注】 计算右侧结果并写入 auto referenceIfSmaller，更新当前运行状态。
    const auto referenceIfSmaller = [](const std::string& id, Json& value) {
        // 【逐行批注】 计算右侧结果并写入 Json reference，更新当前运行状态。
        Json reference = {{"unchangedFrom", id}};
        // 【逐行批注】 判断条件 (reference.dump().size() < value.dump().size()) value = std::mov…，成立时执行对应分支。
        if (reference.dump().size() < value.dump().size()) value = std::move(reference);
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 按循环规则重复处理：for (auto& message : result) {。
    for (auto& message : result) {
        // 【逐行批注】 判断条件 (message.value("role", "") != "user") continue;，成立时执行对应分支。
        if (message.value("role", "") != "user") continue;
        // 【逐行批注】 计算右侧结果并写入 content，更新当前运行状态。
        auto content = Json::parse(message.value("content", ""), nullptr, false);
        // 【逐行批注】 判断条件 (!content.is_object()) continue;，成立时执行对应分支。
        if (!content.is_object()) continue;
        // 【逐行批注】 计算右侧结果并写入 auto compact，更新当前运行状态。
        const auto compact = [&](const std::string& id, Json& value) {
            // 【逐行批注】 判断条件 (!actions.contains(id)) return;，成立时执行对应分支。
            if (!actions.contains(id)) return;
            // 【逐行批注】 计算右侧结果并写入 auto encoded，更新当前运行状态。
            const auto encoded = Json::parse(actions.at(id).value("encoded", ""), nullptr, false);
            // 【逐行批注】 判断条件 (!encoded.is_object() || !readOnlyTools.count(encoded.value("too…，成立时执行对应分支。
            if (!encoded.is_object() || !readOnlyTools.count(encoded.value("tool", ""))) return;
            // 默认参数显式写出时，仍可引用同一工具已经发送过的完全相同回执；原行动参数保留。
            // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
            const auto key = encoded.at("tool").dump() + ":" + value.dump();
            // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
            const auto previous = seen.find(key);
            // 【逐行批注】 判断条件 (previous == seen.end()) seen[key] = id;，成立时执行对应分支。
            if (previous == seen.end()) seen[key] = id;
            // 【逐行批注】 执行前一条件不成立时的分支。
            else referenceIfSmaller(previous->second, value);
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 判断条件 (content.value("toolResults", Json()).is_array()) for (auto& ite…，成立时执行对应分支。
        if (content.value("toolResults", Json()).is_array()) for (auto& item : content["toolResults"]) {
            // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
            const auto id = item.value("id", "");
            // 首次实际发送的凭据保留，不能仅因数据库存在缓存就删除。
            // 【逐行批注】 判断条件 (actions.contains(id)) receipts[id] = {{"encoded", actions.at(id…，成立时执行对应分支。
            if (actions.contains(id)) receipts[id] = {{"encoded", actions.at(id).value("encoded", "")}, {"result", item.value("result", Json())}};
            // 【逐行批注】 调用 compact 完成本行所需操作：compact(id, item["result"]);。
            compact(id, item["result"]);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"completedActions", "savedActions"}) {。
        for (const auto* field : {"completedActions", "savedActions"}) {
            // 【逐行批注】 判断条件 (!content.value(field, Json()).is_object()) continue;，成立时执行对应分支。
            if (!content.value(field, Json()).is_object()) continue;
            // 【逐行批注】 按循环规则重复处理：for (auto& item : content[field].items()) {。
            for (auto& item : content[field].items()) {
                // 【逐行批注】 计算右侧结果并写入 auto& receipt，更新当前运行状态。
                auto& receipt = item.value();
                // 【逐行批注】 判断条件 (!receipt.is_object() || !receipt.contains("encoded") || !receip…，成立时执行对应分支。
                if (!receipt.is_object() || !receipt.contains("encoded") || !receipt.contains("result")) continue;
                // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
                const auto previous = receipts.find(item.key());
                // 【逐行批注】 判断条件 (previous != receipts.end() && previous->second == receipt)，成立时执行对应分支。
                if (previous != receipts.end() && previous->second == receipt)
                    // 【逐行批注】 调用 referenceIfSmaller 完成本行所需操作：referenceIfSmaller(item.key(), receipt["result"]);。
                    referenceIfSmaller(item.key(), receipt["result"]);
                // 【逐行批注】 执行前一条件不成立时的分支。
                else {
                    // 【逐行批注】 计算右侧结果并写入 receipts[item.key()]，更新当前运行状态。
                    receipts[item.key()] = receipt;
                    // 【逐行批注】 调用 compact 完成本行所需操作：compact(item.key(), receipt["result"]);。
                    compact(item.key(), receipt["result"]);
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 message["content"]，更新当前运行状态。
        message["content"] = content.dump();
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

#pragma once
#include "learning_agent.hpp"
#include <cstdlib>
#include <set>

namespace gangyi {
// 内部回退开关：仅显式设为 0 时关闭；不改变模型或思考设置。
inline bool agentEfficiencyEnabled(const char* name) {
    const auto* value = std::getenv(name);
    return !value || std::string(value) != "0";
}

// 权限独立于目录说明。评分专用阶段仍由主控严格限制为原有的两个工具。
inline bool agentToolAuthorized(const std::string& name, const AgentJson& task) {
    const auto type = task.at("event").value("type", "");
    if (type == "library_research" && name != "read_context" && name != "read_library" &&
        name != "search_resources" && name != "read_resource" && name != "select_resources" && name != "describe_tools") return false;
    if ((type == "startup" || type == "profile_update" || type == "learning_updated" || type == "schedule_changed") &&
        (name == "search_resources" || name == "read_resource")) return false;
    return name != "adjust_schedule" || task.value("manualScheduleReplan", false);
}

inline std::string agentToolStage(const AgentJson& task) {
    const auto type = task.at("event").value("type", "");
    if (type == "assistant_chat") return "assistant";
    if (type == "plan_course" || type == "course_preview") return "planning";
    if (type == "prepare_next") return "preparation";
    if (type == "library_research") return "research";
    const auto action = task.at("event").value("action", "answer");
    if (type == "question_answer" && action != "skip" && action != "hint" && action != "omitted" && !task.value("inputResolved", false)) return "assessment";
    if (!task.at("event").value("lessonId", "").empty()) return "teaching";
    return "background";
}

inline AgentJson agentToolCatalog(const std::map<std::string, AgentTool>& tools, const AgentJson& task) {
    static const std::map<std::string, std::set<std::string>> stages = {
        {"assistant", {"read_personal_center", "set_teaching_preferences", "remember_preference", "propose_personal_action", "control_ai_task", "open_personal_page", "adjust_schedule"}},
        {"planning", {"create_course", "read_outline", "update_course_preview", "read_library", "search_resources", "read_resource", "select_resources"}},
        {"preparation", {"read_outline", "read_lesson", "create_lesson", "append_section", "finish_lesson", "read_library", "search_resources", "read_resource", "select_resources"}},
        {"assessment", {"evaluate_answer", "classify_input"}},
        {"teaching", {"read_lesson", "append_section", "select_teaching_focus", "complete_lesson", "read_resource", "schedule_review"}},
        {"research", {"read_library", "search_resources", "read_resource", "select_resources"}},
        {"background", {"read_profile_evidence", "update_profiles", "update_topic_mastery", "update_recommendations", "read_outline", "adjust_goal", "revise_outline", "adjust_schedule"}}
    };
    const auto& detailed = stages.at(agentToolStage(task));
    const bool enabled = agentEfficiencyEnabled("GANGYI_AGENT_STAGE_TOOLS");
    AgentJson catalog = AgentJson::array();
    for (const auto& [name, tool] : tools) {
        if (!agentToolAuthorized(name, task)) continue;
        const bool full = !enabled || detailed.count(name) || name == "describe_tools" || name == "read_context" || name == "read_history";
        auto description = tool.description;
        if (!full) {
            // 使用原说明的首句，保留准确用途；不按字节截断中文。
            const auto end = description.find("。");
            if (end != std::string::npos) description.resize(end + std::string("。").size());
        }
        AgentJson entry = {{"name", name}, {"description", description}};
        if (!full) entry["detailsTool"] = "describe_tools";
        catalog.push_back(std::move(entry));
    }
    return catalog;
}

// 单次 process 的只读上下文副本；无全局状态，恢复或切换数据库时必然重新读取。
class AgentContextReuse {
public:
    AgentJson read(Database& db, const AgentAccess& access, const AgentJson& event,
                   const std::function<AgentJson()>& build) {
        if (!agentEfficiencyEnabled("GANGYI_AGENT_CONTEXT_REUSE")) return build();
        const auto signature = [&] {
            // 画像重评可在相同 revision 写正文；仅核对上下文实际使用的画像，不重建学习历史。
            AgentJson subjects = AgentJson::array();
            for (const auto& item : db.listMastery())
                subjects.push_back({{"subject", item.subject}, {"score", item.score ? AgentJson(*item.score) : AgentJson()},
                    {"rationale", item.rationale}, {"recommendation", item.recommendation}});
            return AgentJson{{"scope", access.scopeId}, {"courses", access.courseIds}, {"event", event},
                {"learning", db.learningRevision()}, {"profile", db.profileRevision()}, {"assessedProfile", db.profileAssessedRevision()},
                {"abilities", db.profileMeta("ability-profile")}, {"subjects", subjects},
                {"library", db.profileMeta("library-revision")}, {"personal", db.myAIContextVersions(event.value("chatId", ""))}}.dump();
        };
        const auto key = signature();
        if (database_ == &db && key == key_) return value_;
        auto value = build();
        // 并发变更时本次结果不进入缓存；主控的学习版本校验仍负责拒绝过时输出。
        if (signature() == key) { database_ = &db; key_ = key; value_ = value; }
        else clear();
        return value;
    }
    void clear() { key_.clear(); value_ = AgentJson(); database_ = nullptr; }
private:
    const Database* database_ = nullptr;
    std::string key_;
    AgentJson value_;
};
}

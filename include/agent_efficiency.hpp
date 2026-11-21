// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 内部回退开关：仅显式设为 0 时关闭；不改变模型或思考设置。
// 【逐行批注】 定义函数 agentEfficiencyEnabled，执行该函数声明的职责。
inline bool agentEfficiencyEnabled(const char* name) {
    // 【逐行批注】 计算右侧结果并写入 auto* value，更新当前运行状态。
    const auto* value = std::getenv(name);
    // 【逐行批注】 结束当前函数并返回结果：return !value || std::string(value) != "0";。
    return !value || std::string(value) != "0";
// 【逐行批注】 结束当前代码块。
}

// 权限独立于目录说明；评分阶段仅允许评价/分类和无副作用的精确计算。
// 【逐行批注】 定义函数 agentToolAuthorized，执行该函数声明的职责。
inline bool agentToolAuthorized(const std::string& name, const AgentJson& task) {
    if (task.at("event").contains("yesterdayReview")) {
        static const std::set<std::string> reviewTools = {"read_lesson", "calculate_exact", "append_section",
            "select_teaching_focus", "start_teaching_round", "read_interactive_capabilities", "read_component_activity"};
        return reviewTools.count(name) != 0;
    }
    if (task.at("event").contains("examQuestion")) return name == "calculate_exact";
    if (task.at("event").contains("material"))
        return name == "read_resource" || name == "calculate_exact";
    if (name == "read_interactive_capabilities") return true;
    if (name == "read_component_activity" && task.at("event").value("lessonId", "").empty()) return false;
    // 【逐行批注】 计算右侧结果并写入 auto type，更新当前运行状态。
    const auto type = task.at("event").value("type", "");
    if (name == "start_teaching_round" && type == "prepare_next") return true;
    if (name == "start_teaching_round" || name == "complete_teaching_round") {
        const auto& event = task.at("event");
        return !event.value("lessonId", "").empty() &&
            (type == "lesson_start" || type == "chat" || type == "question_answer" || type == "teaching_request");
    }
    if (name == "save_learning_state" && task.at("event").value("lessonId", "").empty()) return false;
    // 【逐行批注】 判断条件 (type == "plan_course" && task.at("event").value("mode", "") == …，成立时执行对应分支。
    if (type == "plan_course" && task.at("event").value("mode", "") == "deep") {
        // 新模式规划只读取已有资料并保存新大纲，不能借旧课程上下文修改教学、进度或画像。
        // 【逐行批注】 执行当前声明或表达式：static const std::set<std::string> planningTools = {"create_course", "read_context",…。
        static const std::set<std::string> planningTools = {"create_course", "read_context", "read_history", "read_outline",
            // 【逐行批注】 执行当前声明或表达式："read_library", "read_resource", "search_resources", "describe_tools", "read_profile…。
            "read_library", "read_resource", "search_resources", "describe_tools", "read_profile_evidence", "read_personal_center"};
        // 【逐行批注】 判断条件 (!planningTools.count(name)) return false;，成立时执行对应分支。
        if (!planningTools.count(name)) return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (type == "prepare_next" && task.at("event").value("resumeDraft",…，成立时执行对应分支。
    if (type == "prepare_next" && task.at("event").value("resumeDraft", false)) {
        // 【逐行批注】 执行当前声明或表达式：static const std::set<std::string> recoveryTools = {"read_context", "read_history", …。
        static const std::set<std::string> recoveryTools = {"read_context", "read_history", "read_outline", "read_lesson", "read_library",
            // 【逐行批注】 执行当前声明或表达式："read_resource", "search_resources", "describe_tools", "append_section", "set_deep_c…。
            "read_resource", "search_resources", "describe_tools", "start_teaching_round", "append_section", "set_deep_content", "search_lesson_images", "finish_lesson", "calculate_exact"};
        // 【逐行批注】 判断条件 (!recoveryTools.count(name)) return false;，成立时执行对应分支。
        if (!recoveryTools.count(name)) return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 ((name == "set_deep_content" || name == "search_lesson_images") …，成立时执行对应分支。
    if ((name == "set_deep_content" || name == "search_lesson_images") &&
        // 【逐行批注】 执行当前声明或表达式：type != "prepare_next" && task.at("event").value("lessonId", "").empty()) return fal…。
        type != "prepare_next" && task.at("event").value("lessonId", "").empty()) return false;
    // 【逐行批注】 判断条件 (type == "library_research" && name != "read_context" && name !=…，成立时执行对应分支。
    if (type == "library_research" && name != "read_context" && name != "read_library" &&
        // 【逐行批注】 执行当前声明或表达式：name != "search_resources" && name != "read_resource" && name != "select_resources" …。
        name != "search_resources" && name != "read_resource" && name != "select_resources" && name != "describe_tools") return false;
    // 【逐行批注】 判断条件 ((type == "startup" || type == "profile_update" || type == "lear…，成立时执行对应分支。
    if ((type == "startup" || type == "profile_update" || type == "learning_updated" || type == "schedule_changed") &&
        // 【逐行批注】 执行当前声明或表达式：(name == "search_resources" || name == "read_resource")) return false;。
        (name == "search_resources" || name == "read_resource")) return false;
    // 【逐行批注】 结束当前函数并返回结果：return name != "adjust_schedule" || task.value("manualScheduleReplan"…。
    return name != "adjust_schedule" || task.value("manualScheduleReplan", false);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentToolStage，执行该函数声明的职责。
inline std::string agentToolStage(const AgentJson& task) {
    if (task.at("event").contains("material") || task.at("event").contains("examQuestion")) return "material";
    // 【逐行批注】 计算右侧结果并写入 auto type，更新当前运行状态。
    const auto type = task.at("event").value("type", "");
    // 【逐行批注】 判断条件 (type == "assistant_chat") return "assistant";，成立时执行对应分支。
    if (type == "assistant_chat") return "assistant";
    // 【逐行批注】 判断条件 (type == "plan_course" || type == "course_preview") return "plan…，成立时执行对应分支。
    if (type == "plan_course" || type == "course_preview") return "planning";
    // 【逐行批注】 判断条件 (type == "prepare_next") return "preparation";，成立时执行对应分支。
    if (type == "prepare_next") return "preparation";
    // 【逐行批注】 判断条件 (type == "library_research") return "research";，成立时执行对应分支。
    if (type == "library_research") return "research";
    // 【逐行批注】 计算右侧结果并写入 auto action，更新当前运行状态。
    const auto action = task.at("event").value("action", "answer");
    // 【逐行批注】 判断条件 (type == "question_answer" && action != "skip" && action != "hin…，成立时执行对应分支。
    if (type == "question_answer" && action != "skip" && action != "hint" && action != "omitted" && !task.value("inputResolved", false)) return "assessment";
    // 【逐行批注】 判断条件 (!task.at("event").value("lessonId", "").empty()) return "teachi…，成立时执行对应分支。
    if (!task.at("event").value("lessonId", "").empty()) return "teaching";
    // 【逐行批注】 结束当前函数并返回结果：return "background";。
    return "background";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentToolCatalog，执行该函数声明的职责。
inline AgentJson agentToolCatalog(const std::map<std::string, AgentTool>& tools, const AgentJson& task) {
    // 【逐行批注】 执行当前声明或表达式：static const std::map<std::string, std::set<std::string>> stages = {。
    static const std::map<std::string, std::set<std::string>> stages = {
        // 【逐行批注】 执行当前声明或表达式：{"assistant", {"read_personal_center", "set_teaching_preferences", "remember_prefere…。
        {"assistant", {"read_personal_center", "set_teaching_preferences", "remember_preference", "propose_personal_action", "control_ai_task", "open_personal_page", "adjust_schedule"}},
        // 【逐行批注】 执行当前声明或表达式：{"planning", {"create_course", "read_outline", "update_course_preview", "read_librar…。
        {"planning", {"create_course", "read_outline", "update_course_preview", "read_library", "search_resources", "read_resource", "select_resources"}},
        // 【逐行批注】 执行当前声明或表达式：{"preparation", {"read_outline", "read_lesson", "create_lesson", "append_section", "…。
        {"preparation", {"read_outline", "read_lesson", "create_lesson", "start_teaching_round", "append_section", "finish_lesson", "set_deep_content", "search_lesson_images", "read_library", "search_resources", "read_resource", "select_resources"}},
        // 【逐行批注】 执行当前声明或表达式：{"assessment", {"evaluate_answer", "classify_input"}},。
        {"assessment", {"evaluate_answer", "classify_input", "calculate_exact"}},
        // 【逐行批注】 执行当前声明或表达式：{"teaching", {"read_lesson", "append_section", "set_deep_content", "search_lesson_im…。
        {"teaching", {"read_lesson", "start_teaching_round", "complete_teaching_round", "append_section", "set_deep_content", "search_lesson_images", "select_teaching_focus", "complete_lesson", "read_resource", "schedule_review", "save_learning_state"}},
        // 【逐行批注】 执行当前声明或表达式：{"research", {"read_library", "search_resources", "read_resource", "select_resources…。
        {"research", {"read_library", "search_resources", "read_resource", "select_resources"}},
        {"material", {"read_resource", "calculate_exact"}},
        // 【逐行批注】 执行当前声明或表达式：{"background", {"read_profile_evidence", "update_profiles", "update_topic_mastery", …。
        {"background", {"read_profile_evidence", "update_profiles", "update_topic_mastery", "update_recommendations", "read_outline", "adjust_goal", "revise_outline", "adjust_schedule"}}
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto& detailed，更新当前运行状态。
    const auto& detailed = stages.at(agentToolStage(task));
    // 【逐行批注】 计算右侧结果并写入 bool enabled，更新当前运行状态。
    const bool enabled = agentEfficiencyEnabled("GANGYI_AGENT_STAGE_TOOLS");
    // 【逐行批注】 计算右侧结果并写入 AgentJson catalog，更新当前运行状态。
    AgentJson catalog = AgentJson::array();
    // 【逐行批注】 按循环规则重复处理：for (const auto& [name, tool] : tools) {。
    for (const auto& [name, tool] : tools) {
        // 【逐行批注】 判断条件 (!agentToolAuthorized(name, task)) continue;，成立时执行对应分支。
        if (!agentToolAuthorized(name, task)) continue;
        // 【逐行批注】 计算右侧结果并写入 bool full，更新当前运行状态。
        const bool full = !enabled || detailed.count(name) || name == "describe_tools" || name == "read_context" || name == "read_history" || name == "calculate_exact" || name == "read_component_activity" || name == "read_interactive_capabilities";
        // 【逐行批注】 计算右侧结果并写入 description，更新当前运行状态。
        auto description = tool.description;
        // 【逐行批注】 判断条件 (!full) {，成立时执行对应分支。
        if (!full) {
            // 使用原说明的首句，保留准确用途；不按字节截断中文。
            // 【逐行批注】 计算右侧结果并写入 auto end，更新当前运行状态。
            const auto end = description.find("。");
            // 【逐行批注】 判断条件 (end != std::string::npos) description.resize(end + std::string(…，成立时执行对应分支。
            if (end != std::string::npos) description.resize(end + std::string("。").size());
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 AgentJson entry，更新当前运行状态。
        AgentJson entry = {{"name", name}, {"description", description}};
        // 【逐行批注】 判断条件 (!full) entry["detailsTool"] = "describe_tools";，成立时执行对应分支。
        if (!full) entry["detailsTool"] = "describe_tools";
        // 【逐行批注】 调用 move 完成本行所需操作：catalog.push_back(std::move(entry));。
        catalog.push_back(std::move(entry));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return catalog;。
    return catalog;
// 【逐行批注】 结束当前代码块。
}

// 单次 process 的只读上下文副本；无全局状态，恢复或切换数据库时必然重新读取。
// 【逐行批注】 声明数据结构或类型 AgentContextReuse 及其成员接口。
class AgentContextReuse {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 read，并通过构造函数初始化其状态。
    AgentJson read(Database& db, const AgentAccess& access, const AgentJson& event,
                   // 【逐行批注】 调用 AgentJson 完成本行所需操作：const std::function<AgentJson()>& build) {。
                   const std::function<AgentJson()>& build) {
        // 【逐行批注】 判断条件 (!agentEfficiencyEnabled("GANGYI_AGENT_CONTEXT_REUSE")) return b…，成立时执行对应分支。
        if (!agentEfficiencyEnabled("GANGYI_AGENT_CONTEXT_REUSE")) return build();
        // 【逐行批注】 计算右侧结果并写入 auto signature，更新当前运行状态。
        const auto signature = [&] {
            // 画像重评可在相同 revision 写正文；仅核对上下文实际使用的画像，不重建学习历史。
            // 【逐行批注】 计算右侧结果并写入 AgentJson subjects，更新当前运行状态。
            AgentJson subjects = AgentJson::array();
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listMastery())。
            for (const auto& item : db.listMastery())
                // 【逐行批注】 调用 AgentJson 完成本行所需操作：subjects.push_back({{"subject", item.subject}, {"score", item.score ?…。
                subjects.push_back({{"subject", item.subject}, {"score", item.score ? AgentJson(*item.score) : AgentJson()},
                    // 【逐行批注】 执行当前声明或表达式：{"rationale", item.rationale}, {"recommendation", item.recommendation}});。
                    {"rationale", item.rationale}, {"recommendation", item.recommendation}});
            // 【逐行批注】 结束当前函数并返回结果：return AgentJson{{"scope", access.scopeId}, {"courses", access.course…。
            return AgentJson{{"scope", access.scopeId}, {"courses", access.courseIds}, {"event", event},
                // 【逐行批注】 执行当前声明或表达式：{"learning", db.learningRevision()}, {"profile", db.profileRevision()}, {"assessedPr…。
                {"learning", db.learningRevision()}, {"profile", db.profileRevision()}, {"assessedProfile", db.profileAssessedRevision()},
                // 【逐行批注】 执行当前声明或表达式：{"abilities", db.profileMeta("ability-profile")}, {"subjects", subjects},。
                {"abilities", db.profileMeta("ability-profile")}, {"subjects", subjects},
                // 【逐行批注】 执行当前声明或表达式：{"library", db.profileMeta("library-revision")}, {"personal", db.myAIContextVersions…。
                {"library", db.profileMeta("library-revision")}, {"personal", db.myAIContextVersions(event.value("chatId", ""))}}.dump();
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
        const auto key = signature();
        // 【逐行批注】 判断条件 (database_ == &db && key == key_) return value_;，成立时执行对应分支。
        if (database_ == &db && key == key_) return value_;
        // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
        auto value = build();
        // 并发变更时本次结果不进入缓存；主控的学习版本校验仍负责拒绝过时输出。
        // 【逐行批注】 判断条件 (signature() == key) { database_ = &db; key_ = key; value_ = val…，成立时执行对应分支。
        if (signature() == key) { database_ = &db; key_ = key; value_ = value; }
        // 【逐行批注】 执行前一条件不成立时的分支。
        else clear();
        // 【逐行批注】 结束当前函数并返回结果：return value;。
        return value;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 clear，执行该函数声明的职责。
    void clear() { key_.clear(); value_ = AgentJson(); database_ = nullptr; }
// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 计算右侧结果并写入 Database* database_，更新当前运行状态。
    const Database* database_ = nullptr;
    // 【逐行批注】 执行当前声明或表达式：std::string key_;。
    std::string key_;
    // 【逐行批注】 执行当前声明或表达式：AgentJson value_;。
    AgentJson value_;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 结束当前代码块。
}

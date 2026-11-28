#pragma once

#include <nlohmann/json.hpp>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include "teaching_prompt_modules.hpp"

namespace gangyi {

inline const TeachingPromptModule& teachingPromptModule(const std::string& id) {
    for (const auto& module : kTeachingPromptModules) if (id == module.id) return module;
    throw std::logic_error("未注册的教学提示模块：" + id);
}

struct TeachingPolicySelection {
    std::string prompt;
    nlohmann::json manifest = nlohmann::json::array();
};

// 只选择规则，不执行教学行动、评分或调用模型。自然语言要求仍交由原主控理解。
inline TeachingPolicySelection selectTeachingPolicy(const nlohmann::json& context,
                                                    bool assessment, bool deepMode) {
    using Json = nlohmann::json;
    TeachingPolicySelection result;
    std::set<std::string> included;
    const auto add = [&](const std::string& id, bool append = true) {
        if (!included.insert(id).second) return;
        const auto& module = teachingPromptModule(id);
        result.manifest.push_back({{"id", id}, {"version", module.version}, {"sha256", module.digest}});
        if (append) result.prompt += "\n[" + id + "]\n" + module.body;
    };
    // 原主控与证据边界已在原消息位置嵌入，此处只记录身份，不重复发送。
    if (!assessment) { add("core/system", false); add("core/behavior-policy", false); }
    add("evaluation/factuality-checker", false);

    const auto event = context.value("event", Json::object());
    const auto type = event.value("type", "");
    const std::set<std::string> teachingEvents = {"plan_course", "prepare_next", "lesson_start",
        "lesson_finish_request", "question_answer", "teaching_request", "chat"};
    if (!teachingEvents.count(type)) return result;
    add("core/safety-policy");
    if (assessment) {
        add("teaching/answer-diagnosis");
    } else if (type == "plan_course" || type == "prepare_next") {
        add("teaching/teaching-planner");
        if (deepMode) { add("components/component-selector"); add("components/component-generator"); }
    } else if (type == "lesson_finish_request") {
        add("teaching/mastery-evaluator");
        add("teaching/feedback-generator");
    } else if (type == "teaching_request") {
        add("teaching/socratic-tutor");
        const auto request = event.value("requestType", "");
        if (request == "rephrase" || request == "simplify" || request == "slow_down" || request == "speed_up" ||
            request == "give_example" || request == "resume_guidance" || request == "reset_preferences")
            add("teaching/concept-explainer");
    } else {
        add("teaching/concept-explainer");
        if (type == "question_answer") add("teaching/feedback-generator");
        else add("teaching/socratic-tutor");
    }
    const auto lesson = context.value("lessonContext", Json::object());
    if (!assessment && !lesson.value("currentDeepPage", Json::object()).value("media", Json::array()).empty())
        add("components/component-explainer");

    // 学科只取当前授权课程的显式元数据；不能把问题、教材或其它课程的文字升级为规则。
    const std::map<std::string, std::string> subjects = {
        {"数学", "mathematics"}, {"math", "mathematics"}, {"mathematics", "mathematics"},
        {"物理", "physics"}, {"physics", "physics"}, {"化学", "chemistry"}, {"chemistry", "chemistry"},
        {"生物", "biology"}, {"biology", "biology"}, {"历史", "history"}, {"history", "history"},
        {"地理", "geography"}, {"geography", "geography"}, {"语文", "chinese"}, {"chinese", "chinese"},
        {"英语", "english"}, {"english", "english"}, {"计算机", "computer-science"}, {"computer-science", "computer-science"},
        {"computer_science", "computer-science"}};
    const auto courseId = event.value("courseId", "");
    if (!courseId.empty()) for (const auto& course : context.value("courses", Json::array())) {
        if (course.value("id", "") != courseId) continue;
        const auto found = subjects.find(course.value("subject", ""));
        if (found != subjects.end()) add("subjects/" + found->second);
        break;
    }
    add("evaluation/quality-evaluator");
    return result;
}

} // namespace gangyi

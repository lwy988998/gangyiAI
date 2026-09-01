#include "plan_adapter.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace gangyi {
namespace {

using json = nlohmann::json;

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n\f\v");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n\f\v");
    return value.substr(first, last - first + 1);
}

std::string safeTextValue(const json& value, const std::string& fallback = {}) {
    if (!value.is_string()) return fallback;
    const std::string result = trim(value.get<std::string>());
    return result.empty() ? fallback : result;
}

std::string safeText(const json& object, const std::string& key, const std::string& fallback = {}) {
    if (!object.is_object() || !object.contains(key)) return fallback;
    return safeTextValue(object.at(key), fallback);
}

double numberValue(const json& object, const std::string& key, double fallback) {
    if (!object.is_object() || !object.contains(key) || !object.at(key).is_number()) return fallback;
    return object.at(key).get<double>();
}

std::string numberText(double value) {
    if (!std::isfinite(value)) return {};
    std::ostringstream out;
    if (std::floor(value) == value) out << static_cast<long long>(value);
    else out << value;
    return out.str();
}

std::vector<std::string> compactStrings(const json& value, size_t limit) {
    std::vector<std::string> result;
    if (!value.is_array()) return result;
    for (const auto& item : value) {
        if (!item.is_string()) continue;
        const std::string text = trim(item.get<std::string>());
        if (!text.empty()) result.push_back(text);
        if (result.size() >= limit) break;
    }
    return result;
}

std::vector<std::string> compactStrings(const json& object, const std::string& key, size_t limit) {
    if (!object.is_object() || !object.contains(key)) return {};
    return compactStrings(object.at(key), limit);
}

json stringsToJson(const std::vector<std::string>& values) {
    json result = json::array();
    for (const auto& value : values) result.push_back(value);
    return result;
}

json arrayOrEmpty(const json& object, const std::string& key) {
    if (!object.is_object() || !object.contains(key) || !object.at(key).is_array()) return json::array();
    return object.at(key);
}

std::string fallbackTitle(const std::string& goal, const std::string& mode) {
    return mode == "lite" ? goal + "快速学习方案" : "钢一定制AI 学习方案";
}

json skeletonStepsFromPhase(const json& phase) {
    const std::string phaseName = safeText(phase, "name", "本阶段");
    std::vector<std::string> topics = compactStrings(phase, "topics", 6);
    if (topics.empty()) topics = compactStrings(phase, "tasks", 6);
    const json descriptions = arrayOrEmpty(phase, "topicDescriptions");
    const std::string checkpoint = safeText(phase, "checkpoint", "");
    json result = json::array();
    for (size_t index = 0; index < topics.size(); ++index) {
        std::string explanation;
        if (index < descriptions.size()) explanation = safeTextValue(descriptions.at(index));
        if (explanation.empty()) explanation = phaseName + "中的学习点：" + topics[index];
        const std::string check = checkpoint.empty() ? "能完成「" + topics[index] + "」相关练习" : checkpoint;
        result.push_back({{"title", topics[index]},
                          {"explanation", explanation},
                          {"example", ""},
                          {"action", "进入微课程学习「" + topics[index] + "」"},
                          {"check", check}});
    }
    return result;
}

json adaptStep(const json& step, size_t index, const std::string& phaseName, const std::string& phaseDescription) {
    const std::string fallbackTitle = "第 " + std::to_string(index + 1) + " 步：" + (phaseName.empty() ? "理解本阶段重点" : phaseName);
    const std::string title = safeText(step, "title", fallbackTitle);
    return {{"title", title},
            {"explanation", safeText(step, "explanation", phaseDescription.empty() ? "先理解本阶段核心概念，再通过练习把知识转成可执行能力。" : phaseDescription)},
            {"example", safeText(step, "example", "")},
            {"action", safeText(step, "action", "完成本步骤对应练习，并记录遇到的问题。")},
            {"check", safeText(step, "check", "能用自己的话解释本步骤，并独立完成一个小练习。")}};
}

json adaptedSteps(const json& phase, const std::string& phaseName, const std::string& phaseDescription) {
    json raw = arrayOrEmpty(phase, "steps");
    if (raw.empty()) raw = skeletonStepsFromPhase(phase);
    json result = json::array();
    for (size_t index = 0; index < raw.size() && index < 6; ++index) {
        if (!raw.at(index).is_object()) continue;
        result.push_back(adaptStep(raw.at(index), index, phaseName, phaseDescription));
    }
    return result;
}

json adaptRoadmap(const json& plan) {
    json result = json::array();
    const json phases = arrayOrEmpty(plan, "phases");
    for (size_t index = 0; index < phases.size(); ++index) {
        const auto& phase = phases.at(index);
        if (!phase.is_object()) continue;
        const std::string phaseName = safeText(phase, "name", "阶段" + std::to_string(index + 1));
        const std::string phaseDescription = safeText(phase, "description", safeText(phase, "overview", "完成关键知识学习、练习和阶段产出。"));
        const std::string duration = safeText(phase, "duration", numberText(numberValue(phase, "durationWeeks", 2.0)) + " 周");
        result.push_back({{"name", phaseName},
                          {"duration", duration},
                          {"goal", safeText(phase, "objective", "掌握本阶段核心能力")},
                          {"description", phaseDescription},
                          {"why", safeText(phase, "why", "")},
                          {"output", safeText(phase, "output", "")},
                          {"practice", safeText(phase, "practice", "")},
                          {"checkpoint", safeText(phase, "checkpoint", "")},
                          {"commonMistakes", stringsToJson(compactStrings(phase, "commonMistakes", 8))},
                          {"tasks", stringsToJson(compactStrings(phase, "tasks", 6))},
                          {"steps", adaptedSteps(phase, phaseName, phaseDescription)}});
    }
    return result;
}

json phaseTasks(const json& phase) {
    const auto tasks = compactStrings(phase, "tasks", 5);
    if (!tasks.empty()) return stringsToJson(tasks);
    return stringsToJson(compactStrings(phase, "topics", 5));
}

json adaptCourseStructure(const json& plan) {
    json result = json::array();
    const json phases = arrayOrEmpty(plan, "phases");
    for (size_t index = 0; index < phases.size(); ++index) {
        const auto& phase = phases.at(index);
        if (!phase.is_object()) continue;
        const auto topics = compactStrings(phase, "topics", 1000);
        result.push_back({{"stage", safeText(phase, "name", "学习阶段")},
                          {"topics", topics.empty() ? phaseTasks(phase) : stringsToJson(topics)}});
    }
    return result;
}

json adaptSlide(const json& slide, size_t index) {
    return {{"title", safeText(slide, "title", "课程课件 " + std::to_string(index + 1))},
            {"subtitle", safeText(slide, "subtitle", "")},
            {"content", safeText(slide, "content", "围绕本页主题理解核心概念，并通过练习完成掌握检查。")},
            {"bullets", stringsToJson(compactStrings(slide, "bullets", 1000))},
            {"speakerNote", safeText(slide, "speakerNote", "")},
            {"relatedPhase", safeText(slide, "relatedPhase", "")}};
}

json slidesFromPhases(const json& plan, const json& roadmap) {
    const std::string goal = safeText(plan, "goal", "学习目标");
    const std::string title = safeText(plan, "title", "目标学习路径");
    const std::string summary = safeText(plan, "courseIntro", safeText(plan, "overview", safeText(plan, "summary", "本课程会把目标拆成具体阶段、步骤、练习和检查标准。")));
    json slides = json::array();
    slides.push_back({{"title", title},
                      {"subtitle", goal},
                      {"content", summary},
                      {"bullets", stringsToJson(compactStrings(plan, "learningOutcomes", 4))},
                      {"speakerNote", "先建立课程全局认知，再进入阶段学习；每一阶段都要完成具体练习和检查标准。"},
                      {"relatedPhase", ""}});

    for (size_t phaseIndex = 0; phaseIndex < roadmap.size(); ++phaseIndex) {
        const auto& phase = roadmap.at(phaseIndex);
        if (!phase.is_object()) continue;
        const std::string phaseName = safeText(phase, "name", "阶段" + std::to_string(phaseIndex + 1));
        std::vector<std::string> bullets;
        const std::string duration = safeText(phase, "duration", "");
        const std::string output = safeText(phase, "output", "");
        const std::string checkpoint = safeText(phase, "checkpoint", "");
        if (!duration.empty()) bullets.push_back(duration);
        if (!output.empty()) bullets.push_back(output);
        if (!checkpoint.empty()) bullets.push_back(checkpoint);
        slides.push_back({{"title", phaseName},
                          {"subtitle", safeText(phase, "goal", safeText(phase, "output", "第 " + std::to_string(phaseIndex + 1) + " 阶段"))},
                          {"content", safeText(phase, "description", "围绕「" + phaseName + "」完成具体学习点和可检查练习。")},
                          {"bullets", stringsToJson(bullets)},
                          {"speakerNote", safeText(phase, "why", "说明这一阶段为什么重要，并提醒用户做完检查点再进入下一阶段。")},
                          {"relatedPhase", phaseName}});
        const json steps = arrayOrEmpty(phase, "steps");
        for (size_t stepIndex = 0; stepIndex < steps.size() && stepIndex < 2; ++stepIndex) {
            const auto& step = steps.at(stepIndex);
            if (!step.is_object()) continue;
            const std::string stepTitle = safeText(step, "title", "第 " + std::to_string(stepIndex + 1) + " 步");
            std::vector<std::string> stepBullets;
            const std::string example = safeText(step, "example", "");
            const std::string action = safeText(step, "action", "完成「" + stepTitle + "」对应练习");
            const std::string check = safeText(step, "check", "能拿出练习结果并说明关键步骤");
            if (!example.empty()) stepBullets.push_back(example);
            if (!action.empty()) stepBullets.push_back("行动：" + action);
            if (!check.empty()) stepBullets.push_back("检查：" + check);
            slides.push_back({{"title", stepTitle},
                              {"subtitle", phaseName},
                              {"content", safeText(step, "explanation", safeText(phase, "description", "先理解本步骤，再完成行动建议。"))},
                              {"bullets", stringsToJson(stepBullets)},
                              {"speakerNote", "这一页按“讲解—例子—行动—检查”的顺序带用户学习。"},
                              {"relatedPhase", phaseName}});
        }
    }
    if (slides.size() > 12) slides.erase(slides.begin() + 12, slides.end());
    return slides;
}

json adaptSlides(const json& plan, const json& roadmap, const std::string& mode) {
    json slides = arrayOrEmpty(plan, "slides");
    json result = json::array();
    if (!slides.empty()) {
        for (size_t index = 0; index < slides.size(); ++index) {
            if (slides.at(index).is_object()) result.push_back(adaptSlide(slides.at(index), index));
        }
    } else {
        result = slidesFromPhases(plan, roadmap);
    }
    const size_t limit = mode == "lite" ? 1 : 2;
    if (result.size() > limit) result.erase(result.begin() + static_cast<std::ptrdiff_t>(limit), result.end());
    return result;
}

std::string stripStepPrefix(const std::string& value) {
    static const std::regex prefix("^第\\s*[0-9]+\\s*[步阶段]?[:：、-]?\\s*");
    return trim(std::regex_replace(value, prefix, ""));
}

std::string slug(const std::string& value, const std::string& fallback) {
    std::string result;
    bool dash = false;
    for (unsigned char ch : value) {
        if (std::isalnum(ch)) {
            result.push_back(static_cast<char>(std::tolower(ch)));
            dash = false;
        } else if (!result.empty() && !dash) {
            result.push_back('-');
            dash = true;
        }
    }
    while (!result.empty() && result.back() == '-') result.pop_back();
    if (result.empty()) return fallback;
    if (result.size() > 48) result.resize(48);
    return result;
}

json adaptMindMapNode(const json& node, const std::string& fallback) {
    const json children = arrayOrEmpty(node, "children");
    json adaptedChildren = json::array();
    for (size_t index = 0; index < children.size(); ++index) {
        if (children.at(index).is_object()) adaptedChildren.push_back(adaptMindMapNode(children.at(index), fallback + "-" + std::to_string(index + 1)));
    }
    return {{"id", safeText(node, "id", fallback)},
            {"label", safeText(node, "label", "知识点")},
            {"children", adaptedChildren}};
}

json mindMapFromPhases(const json& plan) {
    const json phases = arrayOrEmpty(plan, "phases");
    json children = json::array();
    for (size_t phaseIndex = 0; phaseIndex < phases.size() && phaseIndex < 6; ++phaseIndex) {
        const auto& phase = phases.at(phaseIndex);
        if (!phase.is_object()) continue;
        const std::string phaseName = safeText(phase, "name", "阶段 " + std::to_string(phaseIndex + 1));
        const std::string phaseId = slug(phaseName, "phase-" + std::to_string(phaseIndex + 1));
        json labels = json::array();
        const json steps = arrayOrEmpty(phase, "steps");
        if (!steps.empty()) {
            for (const auto& step : steps) {
                if (!step.is_object() || labels.size() >= 5) continue;
                const std::string label = stripStepPrefix(safeText(step, "title", "学习步骤"));
                if (!label.empty()) labels.push_back(label);
            }
        } else {
            for (const auto& topic : compactStrings(phase, "topics", 5)) labels.push_back(topic);
        }
        json phaseChildren = json::array();
        for (size_t childIndex = 0; childIndex < labels.size() && childIndex < 5; ++childIndex) {
            phaseChildren.push_back({{"id", phaseId + "-" + std::to_string(childIndex + 1)},
                                     {"label", labels.at(childIndex)},
                                     {"children", json::array()}});
        }
        children.push_back({{"id", phaseId}, {"label", phaseName}, {"children", phaseChildren}});
    }
    return {{"title", "课程知识结构"},
            {"nodes", json::array({{{"id", "root"},
                                     {"label", safeText(plan, "goal", safeText(plan, "title", "钢一定制AI 课程"))},
                                     {"children", children}}})}};
}

json adaptMindMap(const json& plan) {
    const json map = plan.is_object() && plan.contains("mindMap") && plan.at("mindMap").is_object() ? plan.at("mindMap") : json::object();
    const json nodes = arrayOrEmpty(map, "nodes");
    if (map.is_object() && !nodes.empty()) {
        json adapted = json::array();
        for (size_t index = 0; index < nodes.size(); ++index) {
            if (nodes.at(index).is_object()) adapted.push_back(adaptMindMapNode(nodes.at(index), "node-" + std::to_string(index + 1)));
        }
        return {{"title", safeText(map, "title", "课程知识结构")}, {"nodes", adapted}};
    }
    return mindMapFromPhases(plan);
}

json adaptResources(const json& plan) {
    json result = json::array();
    const json resources = arrayOrEmpty(plan, "resources");
    for (const auto& resource : resources) {
        if (!resource.is_object()) continue;
        result.push_back({{"name", safeText(resource, "name", "学习资源")},
                          {"type", safeText(resource, "type", "图文教程")},
                          {"difficulty", safeText(resource, "difficulty", "入门")},
                          {"free", resource.contains("free") && resource.at("free").is_boolean() ? resource.at("free").get<bool>() : false},
                          {"description", safeText(resource, "description", "适合作为当前目标的补充学习资料。")},
                          {"href", safeText(resource, "url", safeText(resource, "href", "#"))}});
    }
    return result;
}

json adaptProjects(const json& plan) {
    json result = json::array();
    const json projects = arrayOrEmpty(plan, "projects");
    for (const auto& project : projects) {
        if (!project.is_object()) continue;
        const double hours = numberValue(project, "estimatedHours", 4.0);
        const auto criteria = compactStrings(project, "acceptanceCriteria", 1000);
        std::ostringstream acceptance;
        for (size_t index = 0; index < criteria.size(); ++index) {
            if (index > 0) acceptance << u8"；";
            acceptance << criteria[index];
        }
        if (criteria.empty()) acceptance << "目标明确、过程可复盘、结果可展示。";
        result.push_back({{"name", safeText(project, "name", "阶段实践项目")},
                          {"difficulty", safeText(project, "difficulty", "入门")},
                          {"duration", numberText(hours) + " 小时"},
                          {"output", safeText(project, "output", "一个可检查的练习成果。")},
                          {"acceptance", acceptance.str()}});
    }
    return result;
}

}  // namespace

nlohmann::json adaptGeneratedPlan(const json& plan, const std::string& mode) {
    try {
        const std::string normalizedMode = mode == "lite" ? "lite" : "deep";
        const std::string goal = safeText(plan, "learnerGoal", safeText(plan, "goal", safeText(plan, "title", "你的目标")));
        const std::string titleFallback = fallbackTitle(goal, normalizedMode);
        const std::string summaryFallback = normalizedMode == "lite"
            ? "这是一份快速可执行方案，优先给出核心步骤、练习方法、检查标准和常见错误。"
            : "围绕你的目标生成阶段化学习路线。";
        const std::string title = safeText(plan, "courseTitle", safeText(plan, "title", titleFallback));
        const std::string summary = safeText(plan, "courseSummary", safeText(plan, "summary", safeText(plan, "courseIntro", safeText(plan, "overview", summaryFallback))));
        const json roadmap = adaptRoadmap(plan);
        const double rawWeeks = numberValue(plan, "durationWeeks", normalizedMode == "lite" ? 1.0 : static_cast<double>(roadmap.size() * 2));
        const double weeks = normalizedMode == "lite" ? std::clamp(rawWeeks, 1.0, 2.0) : rawWeeks;

        json output;
        output["title"] = title;
        output["duration"] = numberText(weeks) + " 周";
        output["summary"] = summary;
        output["courseIntro"] = safeText(plan, "courseIntro", safeText(plan, "overview", normalizedMode == "lite"
            ? "这份快速规划会用更少步骤告诉你马上怎么做、怎么练、怎么检查。"
            : "这门课程会通过阶段导学、分步讲解、练习和检查点帮助你真正掌握目标。"));
        output["overview"] = safeText(plan, "overview", normalizedMode == "lite"
            ? "从准备、核心步骤、练习、自检和常见错误快速推进。"
            : "从目标拆解、核心知识、练习任务到阶段产出，逐步推进学习。");
        output["audience"] = safeText(plan, "audience", normalizedMode == "lite"
            ? "希望快速上手并获得可执行步骤的学习者。"
            : "希望系统学习并通过练习获得实际产出的学习者。" );
        output["prerequisites"] = stringsToJson(compactStrings(plan, "prerequisites", 1000));
        output["outcome"] = safeText(plan, "outcome", "完成后能够形成可展示的学习成果，并知道下一步如何继续提升。" );
        output["learningOutcomes"] = stringsToJson(compactStrings(plan, "learningOutcomes", 1000));
        output["slides"] = adaptSlides(plan, roadmap, normalizedMode);
        output["mindMap"] = adaptMindMap(plan);
        output["roadmap"] = roadmap;
        output["courseStructure"] = adaptCourseStructure(plan);
        output["resources"] = adaptResources(plan);
        output["projects"] = adaptProjects(plan);
        return output;
    } catch (...) {
        return json::object();
    }
}

bool isRenderablePlan(const json& plan) {
    try {
        return plan.is_object() && safeText(plan, "title").size() > 0 && safeText(plan, "summary").size() > 0 &&
               plan.contains("roadmap") && plan.at("roadmap").is_array() && !plan.at("roadmap").empty() &&
               plan.contains("courseStructure") && plan.at("courseStructure").is_array() && !plan.at("courseStructure").empty();
    } catch (...) {
        return false;
    }
}

}  // namespace gangyi

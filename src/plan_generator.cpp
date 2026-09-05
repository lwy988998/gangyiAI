#include "plan_generator.hpp"

#include "json_fix.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace gangyi {
namespace {

using json = nlohmann::json;

const char* sharedSkeletonRules = R"(你是钢一定制AI的高中课程架构师，只服务普通高中学生。只输出严格 JSON，禁止 markdown、解释、代码块和编造链接。

课程只属于高中语文、数学、英语、物理、化学、生物、历史、地理、政治或信息技术；围绕教材知识点、题型、实验、阅读、写作、复习与考试设计。不要生成 Web 开发、网站制作、职业技能、摄影或乐器等非高中课程。目标模糊时，按最接近的高中学科与知识点补全。

这是课程骨架，不生成教材、长讲义、完整课件或完整测验。固定顶层字段只有 inferredDomain、learnerGoal、courseTitle、courseSummary、title、goal、durationWeeks、phases。每个 phase 只需 name、durationWeeks、objective、topics、tasks、checkpoint、output、commonMistakes；topics 为 3-4 个短标题，tasks 为 2-3 个可执行任务。每个字段必须具体到高中知识点、题型、实验或学习产出，禁止空泛句。)";

std::string modeRules(const std::string& mode) {
    if (mode == "lite") return "mode=lite：快速规划骨架。durationWeeks 1-2；phases 3 个；每阶段 topics 3 个。输出短、直接、马上能开始。";
    return "mode=deep：深度课程骨架。durationWeeks 6-8；phases 4 个；每阶段 topics 3-4 个。更系统，但仍然只生成目录、目标、产出和检查点。不要长文。";
}

std::string stringValue(const json& object, const char* key) {
    if (!object.is_object() || !object.contains(key) || object[key].is_null()) return {};
    if (object[key].is_string()) return object[key].get<std::string>();
    return object[key].dump();
}

int intValue(const json& object, const char* key) {
    if (!object.is_object() || !object.contains(key)) return 0;
    const auto& value = object[key];
    if (value.is_number_integer()) return value.get<int>();
    if (value.is_number()) return static_cast<int>(value.get<double>());
    if (value.is_string()) {
        try { return std::stoi(value.get<std::string>()); } catch (...) {}
    }
    return 0;
}

std::vector<std::string> strings(const json& object, const char* key) {
    std::vector<std::string> result;
    if (!object.is_object() || !object.contains(key) || !object[key].is_array()) return result;
    for (const auto& value : object[key]) {
        if (value.is_string()) result.push_back(value.get<std::string>());
    }
    return result;
}

Step parseStep(const json& value) {
    return {stringValue(value, "title"), stringValue(value, "explanation"),
            stringValue(value, "example"), stringValue(value, "action"), stringValue(value, "check")};
}

Phase parsePhase(const json& value) {
    Phase phase;
    phase.name = stringValue(value, "name");
    phase.durationWeeks = intValue(value, "durationWeeks");
    phase.duration = stringValue(value, "duration");
    phase.objective = stringValue(value, "objective");
    if (phase.objective.empty()) phase.objective = stringValue(value, "goal");
    phase.why = stringValue(value, "why");
    phase.description = stringValue(value, "description");
    phase.overview = stringValue(value, "overview");
    phase.topics = strings(value, "topics");
    phase.topicDescriptions = strings(value, "topicDescriptions");
    phase.tasks = strings(value, "tasks");
    phase.practice = stringValue(value, "practice");
    phase.checkpoint = stringValue(value, "checkpoint");
    phase.output = stringValue(value, "output");
    phase.commonMistakes = strings(value, "commonMistakes");
    if (value.contains("steps") && value["steps"].is_array())
        for (const auto& step : value["steps"]) phase.steps.push_back(parseStep(step));
    return phase;
}

GeneratedPlan parsePlan(const json& value) {
    if (!value.is_object()) throw AIClientError("json_parse_error", "plan JSON must be an object");
    GeneratedPlan plan;
    plan.inferredDomain = stringValue(value, "inferredDomain");
    plan.learnerGoal = stringValue(value, "learnerGoal");
    plan.courseTitle = stringValue(value, "courseTitle");
    plan.courseSummary = stringValue(value, "courseSummary");
    plan.title = stringValue(value, "title");
    plan.goal = stringValue(value, "goal");
    plan.durationWeeks = intValue(value, "durationWeeks");
    plan.summary = stringValue(value, "summary");
    plan.courseIntro = stringValue(value, "courseIntro");
    plan.overview = stringValue(value, "overview");
    plan.audience = stringValue(value, "audience");
    plan.prerequisites = stringValue(value, "prerequisites");
    plan.outcome = stringValue(value, "outcome");
    plan.learningOutcomes = strings(value, "learningOutcomes");
    plan.projects = strings(value, "projects");
    if (value.contains("phases") && value["phases"].is_array())
        for (const auto& phase : value["phases"]) plan.phases.push_back(parsePhase(phase));
    if (value.contains("slides") && value["slides"].is_array()) {
        for (const auto& valueSlide : value["slides"]) {
            Slide slide{stringValue(valueSlide, "title"), stringValue(valueSlide, "subtitle"),
                stringValue(valueSlide, "content"), strings(valueSlide, "bullets"),
                stringValue(valueSlide, "speakerNote"), stringValue(valueSlide, "relatedPhase")};
            plan.slides.push_back(std::move(slide));
        }
    }
    if (value.contains("mindMap")) plan.mindMap = value["mindMap"];
    if (value.contains("resources")) plan.resources = value["resources"];
    return plan;
}

void validate(const GeneratedPlan& plan, const std::string& mode) {
    const size_t minimum = mode == "lite" ? 3 : 4;
    const size_t maximum = mode == "lite" ? 5 : 6;
    if (plan.title.empty() || plan.goal.empty() || plan.phases.size() < minimum || plan.phases.size() > maximum)
        throw AIClientError("quality_rejected", "generated plan failed quality gate");
}

ChatOptions options(const std::string& system, const std::string& user, const std::string& mode) {
    return {{ {"system", system}, {"user", user} }, {}, 0.7,
        mode == "lite" ? 2600 : 3600, "json_object", mode == "lite" ? 30000 : 40000, 2};
}

}

PlanGenerator::PlanGenerator(AIClient& client) : client_(client) {}

GeneratedPlan PlanGenerator::generate(const std::string& goal, const std::string& mode,
                                      const std::string& qualityFeedback) {
    if (mode != "lite" && mode != "deep") throw AIClientError("invalid_request", "mode must be lite or deep");
    const auto first = goal.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) throw AIClientError("invalid_request", "goal must not be empty");

    const std::string safeGoal = goal.substr(first, goal.find_last_not_of(" \t\r\n") - first + 1);
    const std::string system = std::string(sharedSkeletonRules) + "\n" + modeRules(mode);
    std::string user = "学习目标：" + safeGoal + "\n模式：" + mode + "\n请生成 Level 1 Plan Skeleton 严格 JSON。先输出 inferredDomain 和 learnerGoal，再生成课程目录、阶段目标、topics、topicDescriptions、阶段产出和 checkpoint。";
    if (!qualityFeedback.empty()) {
        user += "\n\n上一次生成未通过质量检查，请根据下面的问题重新生成完整 JSON，不要只解释问题：\n";
        user += qualityFeedback;
        user += "\n修正要求：每个阶段都要有具体知识点、可执行任务、明确产出和检查点；不要复用空泛句；必须严格满足阶段数量和字段结构。";
    }
    const GeneratedPlan plan = parsePlan(parseAIJson(client_.chat(options(system, user, mode)).content));
    validate(plan, mode);
    return plan;
}

}

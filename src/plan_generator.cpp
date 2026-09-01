#include "plan_generator.hpp"

#include "json_fix.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace gangyi {
namespace {

using json = nlohmann::json;

const char* sharedSkeletonRules = R"(你是 钢一定制AI 的通用课程架构师。只输出严格 JSON，禁止 markdown、解释、代码块。不要 mock/fallback/demo/template。不要编造链接。

你的能力边界不是任何预设领域列表。用户输入任何学习目标，你都必须先理解目标、推断它所属的真实学习领域 inferredDomain，再按该领域的真实学习路径设计课程。小众目标也要生成可学习课程；不能套用固定领域模板，不能只输出通用教学框架。

这是 Level 1：Plan Skeleton。只生成课程骨架，不生成整本教材、不生成长篇讲义、不生成完整课件、不生成完整测验。

固定顶层字段：inferredDomain, learnerGoal, courseTitle, courseSummary, title, goal, durationWeeks, summary, courseIntro, overview, audience, prerequisites, outcome, learningOutcomes, phases, slides, mindMap, resources, projects。slides 必须为空数组或最多 1-2 张总览短卡；mindMap 只允许 root -> phase -> topic；resources 可为空数组；projects 只写 0-2 个最终产出项目。

每个 phase 必须包含：name, durationWeeks, duration, objective, why, description, overview, topics, topicDescriptions, tasks, practice, checkpoint, output, commonMistakes, steps。topics 为 3-6 个短标题，topicDescriptions 与 topics 等长且简短；tasks 只写阶段级任务标题。steps 只能是 topic 的短映射，禁止长讲解。

课程必须贴合用户真实目标：短目标要在 learnerGoal 中合理补全；具体目标必须保留时间、场景、输出成果和限制条件。每个 phase 的 name、topics、output、checkpoint 必须出现该领域的真实对象、动作、工具、材料、场景、作品或验收方式。

禁止空泛句：深入学习相关知识、掌握基本概念、多加练习、提升综合能力、理解阶段目标、用练习把知识变成能力、复盘并形成阶段产出、关键抓手、不要只背名词、至少完成一次解释和练习、建立学习节奏、完成一次输出。deep 更深但不是更长，详细内容交给后续阶段。)";

std::string modeRules(const std::string& mode) {
    if (mode == "lite") return "mode=lite：快速规划骨架。durationWeeks 1-2；phases 3-4 个；每阶段 topics 3-4 个。输出短、直接、马上能开始。";
    return "mode=deep：深度课程骨架。durationWeeks 6-10；phases 4-6 个；每阶段 topics 4-6 个。更系统，但仍然只生成目录、目标、产出和检查点。不要长文。";
}

std::string retrySuffix() {
    return "\n\n上一次输出不是合法 JSON，或未通过结构质量检查。请重新生成，只输出一个完整、严格、未截断的 JSON 对象：不要 Markdown 代码块围栏，不要任何解释文字，不要注释，不要尾逗号，并确保 phases 数量符合模式要求。";
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
    // 计划生成专用参数：max_tokens 需覆盖 v4-flash 的完整骨架输出（lite 4800 会截断）；
    // 单次超时对齐生成量（lite 100s / deep 150s，2 次 attempt + 退避构成总时长）
    return {{ {"system", system}, {"user", user} }, {}, 0.7,
        mode == "lite" ? 6000 : 8000, "json_object",
        mode == "lite" ? 100000 : 150000, 2};
}

}

PlanGenerator::PlanGenerator(AIClient& client) : client_(client) {}

GeneratedPlan PlanGenerator::generate(const std::string& goal, const std::string& mode) {
    if (mode != "lite" && mode != "deep") throw AIClientError("invalid_request", "mode must be lite or deep");
    const auto first = goal.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) throw AIClientError("invalid_request", "goal must not be empty");

    const std::string safeGoal = goal.substr(first, goal.find_last_not_of(" \t\r\n") - first + 1);
    const std::string system = std::string(sharedSkeletonRules) + "\n" + modeRules(mode);
    const std::string user = "学习目标：" + safeGoal + "\n模式：" + mode + "\n请生成 Level 1 Plan Skeleton 严格 JSON。先输出 inferredDomain 和 learnerGoal，再生成课程目录、阶段目标、topics、topicDescriptions、阶段产出和 checkpoint。";
    std::string content = client_.chat(options(system, user, mode)).content;
    for (int attempt = 0; attempt < 2; ++attempt) {
        try {
            GeneratedPlan plan = parsePlan(parseAIJson(content));
            validate(plan, mode);
            return plan;
        } catch (const AIClientError& error) {
            if (attempt == 1) throw;
            const std::string retryUser = user + retrySuffix();
            content = client_.chat(options(system, retryUser, mode)).content;
        }
    }
    throw AIClientError("unknown", "plan generation failed");
}

}

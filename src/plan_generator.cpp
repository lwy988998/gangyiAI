// 【逐行批注】 引入头文件 "plan_generator.hpp"，为后续实现提供类型或接口声明。
#include "plan_generator.hpp"

// 【逐行批注】 引入头文件 "structured_reply.hpp"，为后续实现提供类型或接口声明。
#include "structured_reply.hpp"

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 using json，更新当前运行状态。
using json = nlohmann::json;

// 【逐行批注】 计算右侧结果并写入 char* sharedSkeletonRules，更新当前运行状态。
const char* sharedSkeletonRules = R"(你是钢一定制AI的高中课程架构师，只服务普通高中学生。只输出严格 JSON，禁止 markdown、解释、代码块和编造链接。

// 【逐行批注】 执行当前声明或表达式：课程只属于高中语文、数学、英语、物理、化学、生物、历史、地理、政治或信息技术；围绕教材知识点、题型、实验、阅读、写作、复习与考试设计。不要生成 Web 开发、网站制作、…。
课程只属于高中语文、数学、英语、物理、化学、生物、历史、地理、政治或信息技术；围绕教材知识点、题型、实验、阅读、写作、复习与考试设计。不要生成 Web 开发、网站制作、职业技能、摄影或乐器等非高中课程。目标模糊时，按最接近的高中学科与知识点补全。

// 【逐行批注】 执行当前声明或表达式：这是课程骨架，不生成教材、长讲义、完整课件或完整测验。固定顶层字段只有 inferredDomain、learnerGoal、courseTitle、courseSum…。
这是课程骨架，不生成教材、长讲义、完整课件或完整测验。固定顶层字段只有 inferredDomain、learnerGoal、courseTitle、courseSummary、durationWeeks、phases，不要重复输出 title 或 goal。每个 phase 只需 name、durationWeeks、objective、topics、tasks、checkpoint、output、commonMistakes；topics 为 3-4 个短标题，tasks 为 2-3 个可执行任务。每个字段必须具体到高中知识点、题型、实验或学习产出，禁止空泛句。)";

// 【逐行批注】 定义函数 modeRules，执行该函数声明的职责。
std::string modeRules(const std::string& mode) {
    // 【逐行批注】 判断条件 (mode == "lite") return "mode=lite：快速规划骨架。durationWeeks 1-2；phas…，成立时执行对应分支。
    if (mode == "lite") return "mode=lite：快速规划骨架。durationWeeks 1-2；phases 3 个；每阶段 topics 3 个。输出短、直接、马上能开始。";
    // 【逐行批注】 结束当前函数并返回结果：return "mode=deep：深度课程骨架。durationWeeks 6-8；phases 4 个；每阶段 topics 3-4 …。
    return "mode=deep：深度课程骨架。durationWeeks 6-8；phases 4 个；每阶段 topics 3-4 个。更系统，但仍然只生成目录、目标、产出和检查点。不要长文。";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 stringValue，执行该函数声明的职责。
std::string stringValue(const json& object, const char* key) {
    // 【逐行批注】 判断条件 (!object.is_object() || !object.contains(key) || object[key].is_…，成立时执行对应分支。
    if (!object.is_object() || !object.contains(key) || object[key].is_null()) return {};
    // 【逐行批注】 判断条件 (object[key].is_string()) return object[key].get<std::string>();，成立时执行对应分支。
    if (object[key].is_string()) return object[key].get<std::string>();
    // 【逐行批注】 结束当前函数并返回结果：return object[key].dump();。
    return object[key].dump();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 intValue，执行该函数声明的职责。
int intValue(const json& object, const char* key) {
    // 【逐行批注】 判断条件 (!object.is_object() || !object.contains(key)) return 0;，成立时执行对应分支。
    if (!object.is_object() || !object.contains(key)) return 0;
    // 【逐行批注】 计算右侧结果并写入 auto& value，更新当前运行状态。
    const auto& value = object[key];
    // 【逐行批注】 判断条件 (value.is_number_integer()) return value.get<int>();，成立时执行对应分支。
    if (value.is_number_integer()) return value.get<int>();
    // 【逐行批注】 判断条件 (value.is_number()) return static_cast<int>(value.get<double>());，成立时执行对应分支。
    if (value.is_number()) return static_cast<int>(value.get<double>());
    // 【逐行批注】 判断条件 (value.is_string()) {，成立时执行对应分支。
    if (value.is_string()) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { return std::stoi(value.get<std::string>()); } catch (...) {}
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return 0;。
    return 0;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 strings，执行该函数声明的职责。
std::vector<std::string> strings(const json& object, const char* key) {
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> result;。
    std::vector<std::string> result;
    // 【逐行批注】 判断条件 (!object.is_object() || !object.contains(key) || !object[key].is…，成立时执行对应分支。
    if (!object.is_object() || !object.contains(key) || !object[key].is_array()) return result;
    // 【逐行批注】 按循环规则重复处理：for (const auto& value : object[key]) {。
    for (const auto& value : object[key]) {
        // 【逐行批注】 判断条件 (value.is_string()) result.push_back(value.get<std::string>());，成立时执行对应分支。
        if (value.is_string()) result.push_back(value.get<std::string>());
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 parseStep，执行该函数声明的职责。
Step parseStep(const json& value) {
    // 【逐行批注】 结束当前函数并返回结果：return {stringValue(value, "title"), stringValue(value, "explanation"…。
    return {stringValue(value, "title"), stringValue(value, "explanation"),
            // 【逐行批注】 调用 stringValue 完成本行所需操作：stringValue(value, "example"), stringValue(value, "action"), stringVa…。
            stringValue(value, "example"), stringValue(value, "action"), stringValue(value, "check")};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 parsePhase，执行该函数声明的职责。
Phase parsePhase(const json& value) {
    // 【逐行批注】 执行当前声明或表达式：Phase phase;。
    Phase phase;
    // 【逐行批注】 计算右侧结果并写入 phase.name，更新当前运行状态。
    phase.name = stringValue(value, "name");
    // 【逐行批注】 计算右侧结果并写入 phase.durationWeeks，更新当前运行状态。
    phase.durationWeeks = intValue(value, "durationWeeks");
    // 【逐行批注】 计算右侧结果并写入 phase.duration，更新当前运行状态。
    phase.duration = stringValue(value, "duration");
    // 【逐行批注】 计算右侧结果并写入 phase.objective，更新当前运行状态。
    phase.objective = stringValue(value, "objective");
    // 【逐行批注】 判断条件 (phase.objective.empty()) phase.objective = stringValue(value, "…，成立时执行对应分支。
    if (phase.objective.empty()) phase.objective = stringValue(value, "goal");
    // 【逐行批注】 计算右侧结果并写入 phase.why，更新当前运行状态。
    phase.why = stringValue(value, "why");
    // 【逐行批注】 计算右侧结果并写入 phase.description，更新当前运行状态。
    phase.description = stringValue(value, "description");
    // 【逐行批注】 计算右侧结果并写入 phase.overview，更新当前运行状态。
    phase.overview = stringValue(value, "overview");
    // 【逐行批注】 计算右侧结果并写入 phase.topics，更新当前运行状态。
    phase.topics = strings(value, "topics");
    // 【逐行批注】 计算右侧结果并写入 phase.topicDescriptions，更新当前运行状态。
    phase.topicDescriptions = strings(value, "topicDescriptions");
    // 【逐行批注】 计算右侧结果并写入 phase.tasks，更新当前运行状态。
    phase.tasks = strings(value, "tasks");
    // 【逐行批注】 计算右侧结果并写入 phase.practice，更新当前运行状态。
    phase.practice = stringValue(value, "practice");
    // 【逐行批注】 计算右侧结果并写入 phase.checkpoint，更新当前运行状态。
    phase.checkpoint = stringValue(value, "checkpoint");
    // 【逐行批注】 计算右侧结果并写入 phase.output，更新当前运行状态。
    phase.output = stringValue(value, "output");
    // 【逐行批注】 计算右侧结果并写入 phase.commonMistakes，更新当前运行状态。
    phase.commonMistakes = strings(value, "commonMistakes");
    // 【逐行批注】 判断条件 (value.contains("steps") && value["steps"].is_array())，成立时执行对应分支。
    if (value.contains("steps") && value["steps"].is_array())
        // 【逐行批注】 按循环规则重复处理：for (const auto& step : value["steps"]) phase.steps.push_back(parseStep(st…。
        for (const auto& step : value["steps"]) phase.steps.push_back(parseStep(step));
    // 【逐行批注】 结束当前函数并返回结果：return phase;。
    return phase;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 parsePlan，执行该函数声明的职责。
GeneratedPlan parsePlan(const json& value) {
    // 【逐行批注】 判断条件 (!value.is_object()) throw AIClientError("json_parse_error", "pl…，成立时执行对应分支。
    if (!value.is_object()) throw AIClientError("json_parse_error", "plan JSON must be an object");
    // 【逐行批注】 执行当前声明或表达式：GeneratedPlan plan;。
    GeneratedPlan plan;
    // 【逐行批注】 计算右侧结果并写入 plan.inferredDomain，更新当前运行状态。
    plan.inferredDomain = stringValue(value, "inferredDomain");
    // 【逐行批注】 计算右侧结果并写入 plan.learnerGoal，更新当前运行状态。
    plan.learnerGoal = stringValue(value, "learnerGoal");
    // 【逐行批注】 计算右侧结果并写入 plan.courseTitle，更新当前运行状态。
    plan.courseTitle = stringValue(value, "courseTitle");
    // 【逐行批注】 计算右侧结果并写入 plan.courseSummary，更新当前运行状态。
    plan.courseSummary = stringValue(value, "courseSummary");
    // 【逐行批注】 计算右侧结果并写入 plan.title，更新当前运行状态。
    plan.title = stringValue(value, "title");
    // 【逐行批注】 计算右侧结果并写入 plan.goal，更新当前运行状态。
    plan.goal = stringValue(value, "goal");
    // 【逐行批注】 判断条件 (plan.title.empty()) plan.title = plan.courseTitle;，成立时执行对应分支。
    if (plan.title.empty()) plan.title = plan.courseTitle;
    // 【逐行批注】 判断条件 (plan.courseTitle.empty()) plan.courseTitle = plan.title;，成立时执行对应分支。
    if (plan.courseTitle.empty()) plan.courseTitle = plan.title;
    // 【逐行批注】 判断条件 (plan.goal.empty()) plan.goal = plan.learnerGoal;，成立时执行对应分支。
    if (plan.goal.empty()) plan.goal = plan.learnerGoal;
    // 【逐行批注】 判断条件 (plan.learnerGoal.empty()) plan.learnerGoal = plan.goal;，成立时执行对应分支。
    if (plan.learnerGoal.empty()) plan.learnerGoal = plan.goal;
    // 【逐行批注】 计算右侧结果并写入 plan.durationWeeks，更新当前运行状态。
    plan.durationWeeks = intValue(value, "durationWeeks");
    // 【逐行批注】 计算右侧结果并写入 plan.summary，更新当前运行状态。
    plan.summary = stringValue(value, "summary");
    // 【逐行批注】 计算右侧结果并写入 plan.courseIntro，更新当前运行状态。
    plan.courseIntro = stringValue(value, "courseIntro");
    // 【逐行批注】 计算右侧结果并写入 plan.overview，更新当前运行状态。
    plan.overview = stringValue(value, "overview");
    // 【逐行批注】 计算右侧结果并写入 plan.audience，更新当前运行状态。
    plan.audience = stringValue(value, "audience");
    // 【逐行批注】 计算右侧结果并写入 plan.prerequisites，更新当前运行状态。
    plan.prerequisites = stringValue(value, "prerequisites");
    // 【逐行批注】 计算右侧结果并写入 plan.outcome，更新当前运行状态。
    plan.outcome = stringValue(value, "outcome");
    // 【逐行批注】 计算右侧结果并写入 plan.learningOutcomes，更新当前运行状态。
    plan.learningOutcomes = strings(value, "learningOutcomes");
    // 【逐行批注】 计算右侧结果并写入 plan.projects，更新当前运行状态。
    plan.projects = strings(value, "projects");
    // 【逐行批注】 判断条件 (value.contains("phases") && value["phases"].is_array())，成立时执行对应分支。
    if (value.contains("phases") && value["phases"].is_array())
        // 【逐行批注】 按循环规则重复处理：for (const auto& phase : value["phases"]) plan.phases.push_back(parsePhase…。
        for (const auto& phase : value["phases"]) plan.phases.push_back(parsePhase(phase));
    // 【逐行批注】 判断条件 (value.contains("slides") && value["slides"].is_array()) {，成立时执行对应分支。
    if (value.contains("slides") && value["slides"].is_array()) {
        // 【逐行批注】 按循环规则重复处理：for (const auto& valueSlide : value["slides"]) {。
        for (const auto& valueSlide : value["slides"]) {
            // 【逐行批注】 调用 stringValue 完成本行所需操作：Slide slide{stringValue(valueSlide, "title"), stringValue(valueSlide,…。
            Slide slide{stringValue(valueSlide, "title"), stringValue(valueSlide, "subtitle"),
                // 【逐行批注】 调用 stringValue、strings 完成本行所需操作：stringValue(valueSlide, "content"), strings(valueSlide, "bullets"),。
                stringValue(valueSlide, "content"), strings(valueSlide, "bullets"),
                // 【逐行批注】 调用 stringValue 完成本行所需操作：stringValue(valueSlide, "speakerNote"), stringValue(valueSlide, "rela…。
                stringValue(valueSlide, "speakerNote"), stringValue(valueSlide, "relatedPhase")};
            // 【逐行批注】 调用 move 完成本行所需操作：plan.slides.push_back(std::move(slide));。
            plan.slides.push_back(std::move(slide));
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (value.contains("mindMap")) plan.mindMap = value["mindMap"];，成立时执行对应分支。
    if (value.contains("mindMap")) plan.mindMap = value["mindMap"];
    // 【逐行批注】 判断条件 (value.contains("resources")) plan.resources = value["resources"…，成立时执行对应分支。
    if (value.contains("resources")) plan.resources = value["resources"];
    // 【逐行批注】 结束当前函数并返回结果：return plan;。
    return plan;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 validate，执行该函数声明的职责。
void validate(const GeneratedPlan& plan, const std::string& mode) {
    // 【逐行批注】 计算右侧结果并写入 size_t minimum，更新当前运行状态。
    const size_t minimum = mode == "lite" ? 3 : 4;
    // 【逐行批注】 计算右侧结果并写入 size_t maximum，更新当前运行状态。
    const size_t maximum = mode == "lite" ? 5 : 6;
    // 【逐行批注】 判断条件 (plan.title.empty()) throw AIClientError("quality_rejected", "课程…，成立时执行对应分支。
    if (plan.title.empty()) throw AIClientError("quality_rejected", "课程标题缺失");
    // 【逐行批注】 判断条件 (plan.goal.empty()) throw AIClientError("quality_rejected", "学习目…，成立时执行对应分支。
    if (plan.goal.empty()) throw AIClientError("quality_rejected", "学习目标缺失");
    // 【逐行批注】 判断条件 (plan.phases.size() < minimum || plan.phases.size() > maximum)，成立时执行对应分支。
    if (plan.phases.size() < minimum || plan.phases.size() > maximum)
        // 【逐行批注】 检测到错误情形时抛出异常：throw AIClientError("quality_rejected", "课程阶段数量不符合当前模式要求");。
        throw AIClientError("quality_rejected", "课程阶段数量不符合当前模式要求");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 options，执行该函数声明的职责。
ChatOptions options(const std::string& system, const std::string& user, const std::string& mode) {
    // 【逐行批注】 结束当前函数并返回结果：return {{ {"system", system}, {"user", user} }, {}, 0.7,。
    return {{ {"system", system}, {"user", user} }, {}, 0.7,
        // 【逐行批注】 执行当前声明或表达式：mode == "lite" ? 4500 : 6000, "json_object", 90000, 1, {}, {}};。
        mode == "lite" ? 4500 : 6000, "json_object", 90000, 1, {}, {}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 调用 PlanGenerator、client_ 完成本行所需操作：PlanGenerator::PlanGenerator(AIClient& client) : client_(client) {}。
PlanGenerator::PlanGenerator(AIClient& client) : client_(client) {}

// 【逐行批注】 调用 generate 完成本行所需操作：GeneratedPlan PlanGenerator::generate(const std::string& goal, const …。
GeneratedPlan PlanGenerator::generate(const std::string& goal, const std::string& mode,
                                      // 【逐行批注】 执行当前声明或表达式：const std::string& qualityFeedback,。
                                      const std::string& qualityFeedback,
                                      // 【逐行批注】 执行当前声明或表达式：const std::string& profileContext) {。
                                      const std::string& profileContext) {
    // 【逐行批注】 判断条件 (mode != "lite" && mode != "deep") throw AIClientError("invalid_…，成立时执行对应分支。
    if (mode != "lite" && mode != "deep") throw AIClientError("invalid_request", "课程模式只能是快速规划或深度规划");
    // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
    const auto first = goal.find_first_not_of(" \t\r\n");
    // 【逐行批注】 判断条件 (first == std::string::npos) throw AIClientError("invalid_reques…，成立时执行对应分支。
    if (first == std::string::npos) throw AIClientError("invalid_request", "请填写学习目标");

    // 【逐行批注】 计算右侧结果并写入 std::string safeGoal，更新当前运行状态。
    const std::string safeGoal = goal.substr(first, goal.find_last_not_of(" \t\r\n") - first + 1);
    // 【逐行批注】 计算右侧结果并写入 std::string system，更新当前运行状态。
    const std::string system = std::string(sharedSkeletonRules) + "\n" + modeRules(mode);
    // 【逐行批注】 计算右侧结果并写入 int phaseCount，更新当前运行状态。
    const int phaseCount = mode == "lite" ? 3 : 4;
    // 【逐行批注】 计算右侧结果并写入 std::string user，更新当前运行状态。
    std::string user = "学习目标：" + safeGoal + "\n模式：" + mode +
        // 【逐行批注】 执行当前声明或表达式："\n请生成 Level 1 Plan Skeleton 完整严格 JSON。只使用规定字段，字段顺序固定为 inferredDomain、learnerGoal、co…。
        "\n请生成 Level 1 Plan Skeleton 完整严格 JSON。只使用规定字段，字段顺序固定为 inferredDomain、learnerGoal、courseTitle、courseSummary、durationWeeks、phases。"
        // 【逐行批注】 调用 to_string 完成本行所需操作："learnerGoal 必须原样等于学习目标；phases 必须恰好 " + std::to_string(phaseCount) +。
        "learnerGoal 必须原样等于学习目标；phases 必须恰好 " + std::to_string(phaseCount) +
        // 【逐行批注】 执行当前声明或表达式：" 项。每个 phase 必须依次包含 name、durationWeeks、objective、topics、tasks、checkpoint、output、comm…。
        " 项。每个 phase 必须依次包含 name、durationWeeks、objective、topics、tasks、checkpoint、output、commonMistakes；"
        // 【逐行批注】 执行当前声明或表达式："topics 恰好3项，tasks 恰好2项，commonMistakes 恰好2项，全部使用 JSON 数组。"。
        "topics 恰好3项，tasks 恰好2项，commonMistakes 恰好2项，全部使用 JSON 数组。"
        // 【逐行批注】 执行当前声明或表达式："每个字符串不超过80个汉字，courseSummary 不超过150个汉字，总长度不超过4500个汉字；禁止省略字段、增加字段或输出截断。";。
        "每个字符串不超过80个汉字，courseSummary 不超过150个汉字，总长度不超过4500个汉字；禁止省略字段、增加字段或输出截断。";
    // 【逐行批注】 判断条件 (!qualityFeedback.empty()) {，成立时执行对应分支。
    if (!qualityFeedback.empty()) {
        // 【逐行批注】 计算右侧结果并写入 user +，更新当前运行状态。
        user += "\n\n上一次生成未通过质量检查，请根据下面的问题重新生成完整 JSON，不要只解释问题：\n";
        // 【逐行批注】 计算右侧结果并写入 user +，更新当前运行状态。
        user += qualityFeedback;
        // 【逐行批注】 计算右侧结果并写入 user +，更新当前运行状态。
        user += "\n修正要求：每个阶段都要有具体知识点、可执行任务、明确产出和检查点；不要复用空泛句；必须严格满足阶段数量和字段结构。";
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!profileContext.empty() && profileContext != "[]")，成立时执行对应分支。
    if (!profileContext.empty() && profileContext != "[]")
        // 【逐行批注】 计算右侧结果并写入 user +，更新当前运行状态。
        user += "\n本机学习画像摘要（仅用于调整学习顺序和讲解重点，不得改写学习目标）：" + profileContext;
    // 【逐行批注】 计算右侧结果并写入 ChatOptions request，更新当前运行状态。
    ChatOptions request = options(system, user, mode);
    // 【逐行批注】 计算右侧结果并写入 request.activity.source，更新当前运行状态。
    request.activity.source = "课程规划"; request.activity.purpose = "生成课程目标、阶段与学习路线";
    // 【逐行批注】 计算右侧结果并写入 request.searchQuery，更新当前运行状态。
    request.searchQuery = safeGoal;
    // 【逐行批注】 计算右侧结果并写入 AIResult response，更新当前运行状态。
    const AIResult response = client_.chat(request);
    // 【逐行批注】 计算右侧结果并写入 GeneratedPlan plan，更新当前运行状态。
    GeneratedPlan plan = parsePlan(parseAIJson(response.content));
    // 【逐行批注】 计算右侧结果并写入 plan.generationModel，更新当前运行状态。
    plan.generationModel = response.model;
    // 【逐行批注】 计算右侧结果并写入 plan.searchStatus，更新当前运行状态。
    plan.searchStatus = response.searchStatus;
    // 【逐行批注】 计算右侧结果并写入 plan.searchSources，更新当前运行状态。
    plan.searchSources = response.sources;
    // 【逐行批注】 调用 validate 完成本行所需操作：validate(plan, mode);。
    validate(plan, mode);
    // 【逐行批注】 结束当前函数并返回结果：return plan;。
    return plan;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 结束当前代码块。
}

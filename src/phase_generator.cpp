#include "phase_generator.hpp"

#include "json_fix.hpp"

#include <algorithm>
#include <sstream>

namespace gangyi {
namespace {
using json = nlohmann::json;

std::string value(const json& object, const char* key) {
    if (!object.is_object() || !object.contains(key) || object[key].is_null()) return {};
    return object[key].is_string() ? object[key].get<std::string>() : object[key].dump();
}

bool validString(const json& object, const char* key) { return !value(object, key).empty(); }

json normalize(const json& input) {
    json output = json::object();
    output["objective"] = value(input, "objective");
    output["overview"] = value(input, "overview");
    output["steps"] = json::array();
    if (input.contains("steps") && input["steps"].is_array()) for (const auto& item : input["steps"]) {
        if (!item.is_object()) continue;
        output["steps"].push_back({{"title", value(item, "title")}, {"explanation", value(item, "explanation")},
            {"example", value(item, "example")}, {"action", value(item, "action")}, {"check", value(item, "check")} });
    }
    output["tasks"] = json::array();
    if (input.contains("tasks") && input["tasks"].is_array()) for (const auto& item : input["tasks"]) {
        if (!item.is_object()) continue;
        json actionSteps = item.contains("actionSteps") && item["actionSteps"].is_array() ? item["actionSteps"] : json::array();
        json checklist = item.contains("checklist") && item["checklist"].is_array() ? item["checklist"] : json::array();
        output["tasks"].push_back({{"title", value(item, "title")}, {"description", value(item, "description")},
            {"duration", value(item, "duration")}, {"output", value(item, "output")},
            {"actionSteps", actionSteps}, {"checklist", checklist} });
    }
    output["checklist"] = input.contains("checklist") && input["checklist"].is_array() ? input["checklist"] : json::array();
    output["commonMistakes"] = input.contains("commonMistakes") && input["commonMistakes"].is_array() ? input["commonMistakes"] : json::array();
    return output;
}

bool validate(const json& output) {
    if (!validString(output, "objective") || !validString(output, "overview") || !output["steps"].is_array() || output["steps"].empty() || !output["tasks"].is_array() || output["tasks"].empty()) return false;
    for (const auto& step : output["steps"]) if (!validString(step,"title") || !validString(step,"explanation") || !validString(step,"action") || !validString(step,"check")) return false;
    for (const auto& task : output["tasks"]) if (!validString(task,"title") || !validString(task,"description") || !validString(task,"duration") || !validString(task,"output")) return false;
    return true;
}

ChatOptions options(const std::string& system, const std::string& user, const std::string& mode) {
    return {{{"system", system}, {"user", user}}, {}, 0.3, mode == "lite" ? 2500 : 3500, "json_object", 45000, 1};
}

json fallbackPhase(const std::string& stage, const std::vector<std::string>& topics) {
    const std::string topic = topics.empty() ? stage : topics.front();
    return {
        {"objective", u8"围绕“" + stage + u8"”，掌握核心概念、适用条件和一个典型应用。"},
        {"overview", u8"本阶段使用可直接执行的学习单推进：解释概念、分析材料、完成练习并记录错误。"},
        {"steps", json::array({
            {{"title", u8"明确本阶段问题"}, {"explanation", u8"用自己的话写出“" + topic + u8"”研究什么，以及完成本阶段后能解决什么问题。"}, {"example", u8"把教材中的一个例题或材料作为验证对象。"}, {"action", u8"写出定义、两个条件和一个用途。"}, {"check", u8"不看资料也能完整复述。"}},
            {{"title", u8"完成一次材料分析"}, {"explanation", u8"按“已知信息—使用方法—得出结论”拆解一道典型题或材料。"}, {"example", u8"先圈出关键词，再对应本阶段的定义或规律。"}, {"action", u8"完成一份三步分析并标注依据。"}, {"check", u8"每一步都有材料或概念依据。"}},
            {{"title", u8"复盘并纠错"}, {"explanation", u8"把易混概念、遗漏条件和错误结论分别记下来，形成下次可复用的检查表。"}, {"example", u8"对比正确解法与自己的首次思路。"}, {"action", u8"记录至少两条易错点。"}, {"check", u8"能说明每条错误为什么会发生。"}}
        })},
        {"tasks", json::array({
            {{"title", u8"概念卡片"}, {"description", u8"整理“" + topic + u8"”的定义、条件和用途。"}, {"duration", u8"20 分钟"}, {"output", u8"一张可复习的概念卡片"}, {"actionSteps", json::array({u8"写定义", u8"列条件", u8"补一个例子"})}, {"checklist", json::array({u8"定义完整", u8"条件具体", u8"例子相关"})}},
            {{"title", u8"典型题分析"}, {"description", u8"选择一道与本阶段相关的练习题，完整写出推理过程。"}, {"duration", u8"30 分钟"}, {"output", u8"一份三步分析与错误记录"}, {"actionSteps", json::array({u8"提取信息", u8"选择方法", u8"检验结论"})}, {"checklist", json::array({u8"使用全部关键条件", u8"过程可复查"})}}
        })},
        {"checklist", json::array({u8"能复述核心概念", u8"能列出适用条件", u8"能完成一道典型题分析", u8"已记录易错点"})},
        {"commonMistakes", json::array({u8"只背结论，不核对条件", u8"只写答案，缺少推理过程", u8"看到关键词就机械套用方法"})}
    };
}

} // namespace

PhaseGenerator::PhaseGenerator(AIClient& client) : client_(client) {}

std::optional<json> PhaseGenerator::generate(const std::string& goal, const std::string& mode, int phaseIndex,
                                              const std::string& stage, const std::vector<std::string>& topics,
                                              const std::vector<SearchResource>& resources) const {
    try {
        std::ostringstream topicText;
        for (size_t i = 0; i < topics.size(); ++i) { if (i) topicText << "、"; topicText << topics[i]; }
        std::ostringstream resourceText;
        for (size_t i = 0; i < std::min<size_t>(5, resources.size()); ++i)
            resourceText << "\n- " << resources[i].title << " | " << resources[i].source << " | " << resources[i].description;
        const std::string system = "你是钢一定制AI的课程阶段导师。只输出严格 JSON，禁止 Markdown、代码块、注释和解释文字。";
        const std::string user = "学习目标：" + goal + "\n模式：" + mode + "\n阶段序号：" + std::to_string(phaseIndex) +
            "\n阶段名：" + stage + "\ntopics：" + topicText.str() + "\n参考资源摘要：" + resourceText.str() +
            "\n请输出严格 JSON：{objective, overview, steps[5-8条，每条含title/explanation/example/action/check], tasks[3-5条，每条含title/description/duration/output/actionSteps/checklist], checklist[3-6条], commonMistakes[2-4条]}。objective和overview必须非空，steps和tasks必须非空。";
        const json output = normalize(parseAIJson(client_.chat(options(system, user, mode)).content));
        if (validate(output)) return output;
    } catch (...) {}
    return fallbackPhase(stage, topics);
}

} // namespace gangyi

#include "phase_generator.hpp"

#include "json_fix.hpp"

#include <algorithm>
#include <sstream>

namespace gangyi {
namespace {
using json = nlohmann::json;

std::string retrySuffix() {
    return "\n\n请修正上一轮输出：只返回一个完整、严格、未截断的 JSON 对象，不要 Markdown、代码围栏、注释或解释文字。";
}

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
    return {{{"system", system}, {"user", user}}, {}, 0.3, mode == "lite" ? 6000 : 8000, "json_object", mode == "lite" ? 90000 : 120000, 1};
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
        std::string content;
        for (int attempt = 0; attempt < 2; ++attempt) {
            try {
                content = client_.chat(options(system, attempt == 0 ? user : user + retrySuffix(), mode)).content;
                const json output = normalize(parseAIJson(content));
                if (validate(output)) return output;
            } catch (...) { if (attempt == 1) return std::nullopt; }
        }
    } catch (...) {}
    return std::nullopt;
}

} // namespace gangyi

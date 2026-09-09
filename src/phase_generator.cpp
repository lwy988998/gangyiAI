#include "phase_generator.hpp"

#include "json_fix.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace gangyi {
namespace {
using json = nlohmann::json;

std::string value(const json& object, const char* key) {
    if (!object.is_object() || !object.contains(key) || !object[key].is_string()) return {};
    return object[key].get<std::string>();
}

bool validStringArray(const json& value, size_t minimum) {
    if (!value.is_array() || value.size() < minimum) return false;
    for (const auto& item : value) if (!item.is_string() || item.get<std::string>().empty()) return false;
    return true;
}

std::string validate(const json& output, const std::string& goal, const std::string& stage) {
    std::vector<std::string> errors;
    if (!output.is_object()) return "顶层必须是 JSON 对象";
    if (value(output, "objective").empty()) errors.push_back("objective 不能为空");
    if (value(output, "overview").empty()) errors.push_back("overview 不能为空");
    if (!output.contains("steps") || !output["steps"].is_array() || output["steps"].size() < 5) errors.push_back("steps 至少需要 5 项");
    else for (const auto& step : output["steps"])
        for (const char* key : {"title", "explanation", "example", "action", "check"})
            if (value(step, key).empty()) errors.push_back(std::string("steps 缺少 ") + key);
    if (!output.contains("tasks") || !output["tasks"].is_array() || output["tasks"].size() < 3) errors.push_back("tasks 至少需要 3 项");
    else for (const auto& task : output["tasks"])
        for (const char* key : {"title", "description", "duration", "output"})
            if (value(task, key).empty()) errors.push_back(std::string("tasks 缺少 ") + key);
    if (output.contains("tasks") && output["tasks"].is_array()) {
        for (const auto& task : output["tasks"]) {
            if (!task.contains("actionSteps") || !validStringArray(task["actionSteps"], 2)) errors.push_back("tasks.actionSteps 至少需要 2 项");
            if (!task.contains("checklist") || !validStringArray(task["checklist"], 1)) errors.push_back("tasks.checklist 至少需要 1 项");
        }
    }
    if (!output.contains("checklist") || !validStringArray(output["checklist"], 3)) errors.push_back("checklist 至少需要 3 项");
    if (!output.contains("commonMistakes") || !validStringArray(output["commonMistakes"], 2)) errors.push_back("commonMistakes 至少需要 2 项");
    const std::string serialized = output.dump();
    if (!goal.empty() && serialized.find(goal) == std::string::npos) errors.push_back("内容必须明确回应用户目标");
    if (!stage.empty() && serialized.find(stage) == std::string::npos) errors.push_back("内容必须明确围绕当前阶段");
    for (const std::string generic : {"明确本阶段问题", "完成一次材料分析", "复盘并纠错"})
        if (serialized.find(generic) != std::string::npos) errors.push_back("包含阶段模板句");
    std::ostringstream result;
    for (size_t i = 0; i < errors.size(); ++i) { if (i) result << "；"; result << errors[i]; }
    return result.str();
}

ChatOptions options(const std::string& system, const std::string& user, const std::string& mode) {
    return {{{"system", system}, {"user", user}}, {}, 0.25,
        mode == "lite" ? 4500 : 6000, "json_object", 90000, 1};
}

std::string nowIso8601() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::ostringstream out;
    out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

}  // namespace

PhaseGenerator::PhaseGenerator(AIClient& client) : client_(client) {}

std::optional<json> PhaseGenerator::generate(const std::string& goal, const std::string& mode, int phaseIndex,
                                              const std::string& stage, const std::vector<std::string>& topics,
                                              const std::vector<SearchResource>& resources) const {
    std::ostringstream topicText;
    for (size_t i = 0; i < topics.size(); ++i) { if (i) topicText << "、"; topicText << topics[i]; }
    std::ostringstream resourceText;
    for (size_t i = 0; i < std::min<size_t>(5, resources.size()); ++i)
        resourceText << "\n- " << resources[i].title << " | " << resources[i].source << " | " << resources[i].description;

    std::string feedback;
    std::string lastType = "quality_rejected";
    std::string lastMessage = "阶段内容未通过质量检查";
    for (int attempt = 1; attempt <= 3; ++attempt) {
        const std::string system = "你是钢一定制AI的课程阶段导师。只输出一个完整严格 JSON 对象，禁止 Markdown、代码块、注释、解释文字和通用模板内容。所有规定字段必须一次性完整输出，不得截断或省略。";
        std::string user = "学习目标：" + goal + "\n模式：" + mode + "\n阶段序号：" + std::to_string(phaseIndex) +
            "\n阶段名：" + stage + "\ntopics：" + topicText.str() + "\n参考资源摘要：" + resourceText.str() +
            "\n请生成严格 JSON，字段顺序固定为：objective、overview、tasks、checklist、commonMistakes、steps。"
            "objective 和 overview 必须原样包含学习目标“" + goal + "”与阶段名“" + stage + "”。"
            "tasks 恰好3条，每条含title/description/duration/output/actionSteps(恰好2条)/checklist(恰好1条)；"
            "checklist 恰好3条；commonMistakes 恰好2条；steps 恰好5条，每条含title/explanation/example/action/check。"
            "每个说明控制在1-2句话，总长度不超过6000个汉字；禁止省略任何字段。";
        if (!feedback.empty()) user += "\n上一次输出未通过检查：" + feedback + "。请完整重新生成并逐项修正。";
        try {
            const AIResult response = client_.chat(options(system, user, mode));
            json output = parseAIJson(response.content);
            feedback = validate(output, goal, stage);
            if (feedback.empty()) {
                json normalized = {{"objective", output.at("objective")}, {"overview", output.at("overview")},
                    {"checklist", output.at("checklist")}, {"commonMistakes", output.at("commonMistakes")},
                    {"steps", json::array()}, {"tasks", json::array()}};
                for (const auto& step : output.at("steps")) {
                    json clean;
                    for (const char* key : {"title", "explanation", "example", "action", "check"}) clean[key] = step.at(key);
                    normalized["steps"].push_back(std::move(clean));
                }
                for (const auto& task : output.at("tasks")) {
                    json clean;
                    for (const char* key : {"title", "description", "duration", "output", "actionSteps", "checklist"}) clean[key] = task.at(key);
                    normalized["tasks"].push_back(std::move(clean));
                }
                const char* configuredModel = std::getenv("AI_MODEL");
                normalized["_generation"] = {{"source", "ai"},
                    {"model", response.model.empty() && configuredModel ? configuredModel : response.model},
                    {"generatedAt", nowIso8601()}, {"attempts", attempt}, {"promptVersion", "ai-phase-v1"}};
                std::cerr << "[phase] phase=" << phaseIndex << " attempt=" << attempt << " quality=passed\n";
                return normalized;
            }
            lastType = "quality_rejected";
            lastMessage = feedback;
            std::cerr << "[phase] phase=" << phaseIndex << " attempt=" << attempt
                      << " quality=rejected reason=" << feedback << '\n';
        } catch (const AIClientError& error) {
            lastType = error.errorType;
            lastMessage = error.what();
            feedback = "AI 调用失败：" + std::string(error.what());
            std::cerr << "[phase] phase=" << phaseIndex << " attempt=" << attempt
                      << " ai_error=" << error.errorType << '\n';
        } catch (const std::exception& error) {
            lastType = "invalid_response";
            lastMessage = error.what();
            feedback = "AI 输出无法解析：" + std::string(error.what());
        }
    }
    throw AIClientError(lastType, lastMessage);
}

}  // namespace gangyi

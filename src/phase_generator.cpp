// 【逐行批注】 引入头文件 "phase_generator.hpp"，为后续实现提供类型或接口声明。
#include "phase_generator.hpp"

// 【逐行批注】 引入头文件 "structured_reply.hpp"，为后续实现提供类型或接口声明。
#include "structured_reply.hpp"

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <ctime>，为后续实现提供类型或接口声明。
#include <ctime>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <utility>，为后续实现提供类型或接口声明。
#include <utility>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 using json，更新当前运行状态。
using json = nlohmann::json;

// 【逐行批注】 定义函数 value，执行该函数声明的职责。
std::string value(const json& object, const char* key) {
    // 【逐行批注】 判断条件 (!object.is_object() || !object.contains(key) || !object[key].is…，成立时执行对应分支。
    if (!object.is_object() || !object.contains(key) || !object[key].is_string()) return {};
    // 【逐行批注】 结束当前函数并返回结果：return object[key].get<std::string>();。
    return object[key].get<std::string>();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 validStringArray，执行该函数声明的职责。
bool validStringArray(const json& value, size_t expected) {
    // 【逐行批注】 判断条件 (!value.is_array() || value.size() != expected) return false;，成立时执行对应分支。
    if (!value.is_array() || value.size() != expected) return false;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : value) if (!item.is_string() || item.get<std::stri…。
    for (const auto& item : value) if (!item.is_string() || item.get<std::string>().empty()) return false;
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 validate，执行该函数声明的职责。
std::string validate(const json& output, const std::string& goal, const std::string& stage) {
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> errors;。
    std::vector<std::string> errors;
    // 【逐行批注】 判断条件 (!output.is_object()) return "顶层必须是 JSON 对象";，成立时执行对应分支。
    if (!output.is_object()) return "顶层必须是 JSON 对象";
    // 【逐行批注】 判断条件 (value(output, "objective").empty()) errors.push_back("objective…，成立时执行对应分支。
    if (value(output, "objective").empty()) errors.push_back("objective 不能为空");
    // 【逐行批注】 判断条件 (value(output, "overview").empty()) errors.push_back("overview 不…，成立时执行对应分支。
    if (value(output, "overview").empty()) errors.push_back("overview 不能为空");
    // 【逐行批注】 判断条件 (!output.contains("steps") || !output["steps"].is_array() || out…，成立时执行对应分支。
    if (!output.contains("steps") || !output["steps"].is_array() || output["steps"].size() != 5) errors.push_back("steps 必须恰好 5 项");
    // 【逐行批注】 执行前一条件不成立时的分支。
    else for (const auto& step : output["steps"])
        // 【逐行批注】 按循环规则重复处理：for (const char* key : {"title", "explanation", "example", "action", "chec…。
        for (const char* key : {"title", "explanation", "example", "action", "check"})
            // 【逐行批注】 判断条件 (value(step, key).empty()) errors.push_back(std::string("steps 缺…，成立时执行对应分支。
            if (value(step, key).empty()) errors.push_back(std::string("steps 缺少 ") + key);
    // 【逐行批注】 判断条件 (!output.contains("tasks") || !output["tasks"].is_array() || out…，成立时执行对应分支。
    if (!output.contains("tasks") || !output["tasks"].is_array() || output["tasks"].size() != 3) errors.push_back("tasks 必须恰好 3 项");
    // 【逐行批注】 执行前一条件不成立时的分支。
    else for (const auto& task : output["tasks"])
        // 【逐行批注】 按循环规则重复处理：for (const char* key : {"title", "description", "duration", "output"})。
        for (const char* key : {"title", "description", "duration", "output"})
            // 【逐行批注】 判断条件 (value(task, key).empty()) errors.push_back(std::string("tasks 缺…，成立时执行对应分支。
            if (value(task, key).empty()) errors.push_back(std::string("tasks 缺少 ") + key);
    // 【逐行批注】 判断条件 (output.contains("tasks") && output["tasks"].is_array()) {，成立时执行对应分支。
    if (output.contains("tasks") && output["tasks"].is_array()) {
        // 【逐行批注】 按循环规则重复处理：for (const auto& task : output["tasks"]) {。
        for (const auto& task : output["tasks"]) {
            // 【逐行批注】 判断条件 (!task.contains("actionSteps") || !validStringArray(task["action…，成立时执行对应分支。
            if (!task.contains("actionSteps") || !validStringArray(task["actionSteps"], 2)) errors.push_back("tasks.actionSteps 必须恰好 2 项");
            // 【逐行批注】 判断条件 (!task.contains("checklist") || !validStringArray(task["checklis…，成立时执行对应分支。
            if (!task.contains("checklist") || !validStringArray(task["checklist"], 1)) errors.push_back("tasks.checklist 必须恰好 1 项");
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!output.contains("checklist") || !validStringArray(output["chec…，成立时执行对应分支。
    if (!output.contains("checklist") || !validStringArray(output["checklist"], 3)) errors.push_back("checklist 必须恰好 3 项");
    // 【逐行批注】 判断条件 (!output.contains("commonMistakes") || !validStringArray(output[…，成立时执行对应分支。
    if (!output.contains("commonMistakes") || !validStringArray(output["commonMistakes"], 2)) errors.push_back("commonMistakes 必须恰好 2 项");
    // 【逐行批注】 计算右侧结果并写入 std::string serialized，更新当前运行状态。
    const std::string serialized = output.dump();
    // 【逐行批注】 判断条件 (!goal.empty() && serialized.find(goal) == std::string::npos) er…，成立时执行对应分支。
    if (!goal.empty() && serialized.find(goal) == std::string::npos) errors.push_back("内容必须明确回应用户目标");
    // 【逐行批注】 判断条件 (!stage.empty() && serialized.find(stage) == std::string::npos) …，成立时执行对应分支。
    if (!stage.empty() && serialized.find(stage) == std::string::npos) errors.push_back("内容必须明确围绕当前阶段");
    // 【逐行批注】 按循环规则重复处理：for (const std::string generic : {"明确本阶段问题", "完成一次材料分析", "复盘并纠错"})。
    for (const std::string generic : {"明确本阶段问题", "完成一次材料分析", "复盘并纠错"})
        // 【逐行批注】 判断条件 (serialized.find(generic) != std::string::npos) errors.push_back…，成立时执行对应分支。
        if (serialized.find(generic) != std::string::npos) errors.push_back("包含阶段模板句");
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream result;。
    std::ostringstream result;
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < errors.size(); ++i) { if (i) result << "；"; result …。
    for (size_t i = 0; i < errors.size(); ++i) { if (i) result << "；"; result << errors[i]; }
    // 【逐行批注】 结束当前函数并返回结果：return result.str();。
    return result.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 options，执行该函数声明的职责。
ChatOptions options(const std::string& system, const std::string& user, const std::string& mode) {
    // 【逐行批注】 结束当前函数并返回结果：return {{{"system", system}, {"user", user}}, {}, 0.25,。
    return {{{"system", system}, {"user", user}}, {}, 0.25,
        // 【逐行批注】 执行当前声明或表达式：mode == "lite" ? 4500 : 6000, "json_object", 90000, 1, {}, {}};。
        mode == "lite" ? 4500 : 6000, "json_object", 90000, 1, {}, {}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 nowIso8601，执行该函数声明的职责。
std::string nowIso8601() {
    // 【逐行批注】 计算右侧结果并写入 auto now，更新当前运行状态。
    const auto now = std::chrono::system_clock::now();
    // 【逐行批注】 计算右侧结果并写入 std::time_t time，更新当前运行状态。
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    // 【逐行批注】 执行当前声明或表达式：std::tm utc{};。
    std::tm utc{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&utc, &time);。
    gmtime_s(&utc, &time);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&time, &utc);。
    gmtime_r(&time, &utc);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream out;。
    std::ostringstream out;
    // 【逐行批注】 调用 put_time 完成本行所需操作：out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");。
    out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    // 【逐行批注】 结束当前函数并返回结果：return out.str();。
    return out.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 调用 PhaseGenerator、client_ 完成本行所需操作：PhaseGenerator::PhaseGenerator(AIClient& client) : client_(client) {}。
PhaseGenerator::PhaseGenerator(AIClient& client) : client_(client) {}

// 【逐行批注】 调用 generate 完成本行所需操作：std::optional<json> PhaseGenerator::generate(const std::string& goal,…。
std::optional<json> PhaseGenerator::generate(const std::string& goal, const std::string& mode, int phaseIndex,
                                              // 【逐行批注】 执行当前声明或表达式：const std::string& stage, const std::vector<std::string>& topics,。
                                              const std::string& stage, const std::vector<std::string>& topics,
                                              // 【逐行批注】 执行当前声明或表达式：const std::vector<SearchResource>& resources,。
                                              const std::vector<SearchResource>& resources,
                                              // 【逐行批注】 执行当前声明或表达式：const std::string& profileContext) const {。
                                              const std::string& profileContext) const {
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream topicText;。
    std::ostringstream topicText;
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < topics.size(); ++i) { if (i) topicText << "、"; topi…。
    for (size_t i = 0; i < topics.size(); ++i) { if (i) topicText << "、"; topicText << topics[i]; }
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream resourceText;。
    std::ostringstream resourceText;
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < std::min<size_t>(5, resources.size()); ++i)。
    for (size_t i = 0; i < std::min<size_t>(5, resources.size()); ++i)
        // 【逐行批注】 执行当前声明或表达式：resourceText << "\n- " << resources[i].title << " | " << resources[i].source << " | …。
        resourceText << "\n- " << resources[i].title << " | " << resources[i].source << " | " << resources[i].description;

    // 【逐行批注】 执行当前声明或表达式：std::string feedback;。
    std::string feedback;
    // 【逐行批注】 计算右侧结果并写入 std::string lastType，更新当前运行状态。
    std::string lastType = "quality_rejected";
    // 【逐行批注】 计算右侧结果并写入 std::string lastMessage，更新当前运行状态。
    std::string lastMessage = "阶段内容未通过质量检查";
    // 【逐行批注】 按循环规则重复处理：for (int attempt = 1; attempt <= 3; ++attempt) {。
    for (int attempt = 1; attempt <= 3; ++attempt) {
        // 【逐行批注】 计算右侧结果并写入 std::string system，更新当前运行状态。
        const std::string system = "你是钢一定制AI的课程阶段导师。只输出一个完整严格 JSON 对象，禁止 Markdown、代码块、注释、解释文字和通用模板内容。所有规定字段必须一次性完整输出，不得截断或省略。";
        // 【逐行批注】 计算右侧结果并写入 std::string user，更新当前运行状态。
        std::string user = "学习目标：" + goal + "\n模式：" + mode + "\n阶段序号：" + std::to_string(phaseIndex) +
            // 【逐行批注】 执行当前声明或表达式："\n阶段名：" + stage + "\ntopics：" + topicText.str() + "\n参考资源摘要：" + resourceText.str() +。
            "\n阶段名：" + stage + "\ntopics：" + topicText.str() + "\n参考资源摘要：" + resourceText.str() +
            // 【逐行批注】 执行当前声明或表达式："\n请生成严格 JSON，字段顺序固定为：objective、overview、tasks、checklist、commonMistakes、steps。"。
            "\n请生成严格 JSON，字段顺序固定为：objective、overview、tasks、checklist、commonMistakes、steps。"
            // 【逐行批注】 执行当前声明或表达式："objective 和 overview 必须原样包含学习目标“" + goal + "”与阶段名“" + stage + "”。"。
            "objective 和 overview 必须原样包含学习目标“" + goal + "”与阶段名“" + stage + "”。"
            // 【逐行批注】 调用 actionSteps、checklist 完成本行所需操作："tasks 恰好3条，每条含title/description/duration/output/actionSteps(恰好2条)/ch…。
            "tasks 恰好3条，每条含title/description/duration/output/actionSteps(恰好2条)/checklist(恰好1条)；"
            // 【逐行批注】 执行当前声明或表达式："checklist 恰好3条；commonMistakes 恰好2条；steps 恰好5条，每条含title/explanation/example/action/c…。
            "checklist 恰好3条；commonMistakes 恰好2条；steps 恰好5条，每条含title/explanation/example/action/check。"
            // 【逐行批注】 执行当前声明或表达式："所有数组必须使用 JSON 数组，不得把数组写成字符串。严格按照下面形状逐项替换占位文字，不得增删对象或字段："。
            "所有数组必须使用 JSON 数组，不得把数组写成字符串。严格按照下面形状逐项替换占位文字，不得增删对象或字段："
            // 【逐行批注】 执行当前声明或表达式：R"({"objective":"含目标和阶段的具体目标","overview":"含目标和阶段的具体概述","tasks":[{"title":"任务1","desc…。
            R"({"objective":"含目标和阶段的具体目标","overview":"含目标和阶段的具体概述","tasks":[{"title":"任务1","description":"具体说明","duration":"时长","output":"具体产出","actionSteps":["操作1","操作2"],"checklist":["检查1"]},{"title":"任务2","description":"具体说明","duration":"时长","output":"具体产出","actionSteps":["操作1","操作2"],"checklist":["检查1"]},{"title":"任务3","description":"具体说明","duration":"时长","output":"具体产出","actionSteps":["操作1","操作2"],"checklist":["检查1"]}],"checklist":["阶段检查1","阶段检查2","阶段检查3"],"commonMistakes":["具体错误1","具体错误2"],"steps":[{"title":"步骤1","explanation":"讲解","example":"示例","action":"操作","check":"检查"},{"title":"步骤2","explanation":"讲解","example":"示例","action":"操作","check":"检查"},{"title":"步骤3","explanation":"讲解","example":"示例","action":"操作","check":"检查"},{"title":"步骤4","explanation":"讲解","example":"示例","action":"操作","check":"检查"},{"title":"步骤5","explanation":"讲解","example":"示例","action":"操作","check":"检查"}]})"
            // 【逐行批注】 执行当前声明或表达式："每个字符串最多2句话，description/explanation/example/action/check 各不超过80个汉字，总长度不超过4500个汉字；禁止省…。
            "每个字符串最多2句话，description/explanation/example/action/check 各不超过80个汉字，总长度不超过4500个汉字；禁止省略任何字段。";
        // 【逐行批注】 判断条件 (!feedback.empty()) user += "\n上一次输出未通过检查：" + feedback + "。请完整重新…，成立时执行对应分支。
        if (!feedback.empty()) user += "\n上一次输出未通过检查：" + feedback + "。请完整重新生成并逐项修正。";
        // 【逐行批注】 判断条件 (!profileContext.empty()) user += u8"\n本机学习摘要：" + profileContext…，成立时执行对应分支。
        if (!profileContext.empty()) user += u8"\n本机学习摘要：" + profileContext +
            // 【逐行批注】 执行当前声明或表达式：u8"。仅调整讲解和练习难度，不得改变课程目标与已有阶段主线。";。
            u8"。仅调整讲解和练习难度，不得改变课程目标与已有阶段主线。";
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 ChatOptions request，更新当前运行状态。
            ChatOptions request = options(system, user, mode);
    // 【逐行批注】 计算右侧结果并写入 request.activity.source，更新当前运行状态。
    request.activity.source = "阶段规划"; request.activity.purpose = "生成本阶段的知识点与教学内容";
            // 【逐行批注】 计算右侧结果并写入 request.searchQuery，更新当前运行状态。
            request.searchQuery = goal;
            // 【逐行批注】 计算右侧结果并写入 AIResult response，更新当前运行状态。
            const AIResult response = client_.chat(request);
            // 【逐行批注】 计算右侧结果并写入 json output，更新当前运行状态。
            json output = parseAIJson(response.content);
            // 【逐行批注】 计算右侧结果并写入 feedback，更新当前运行状态。
            feedback = validate(output, goal, stage);
            // 【逐行批注】 判断条件 (feedback.empty()) {，成立时执行对应分支。
            if (feedback.empty()) {
                // 【逐行批注】 计算右侧结果并写入 json normalized，更新当前运行状态。
                json normalized = {{"objective", output.at("objective")}, {"overview", output.at("overview")},
                    // 【逐行批注】 执行当前声明或表达式：{"checklist", output.at("checklist")}, {"commonMistakes", output.at("commonMistakes"…。
                    {"checklist", output.at("checklist")}, {"commonMistakes", output.at("commonMistakes")},
                    // 【逐行批注】 调用 array 完成本行所需操作：{"steps", json::array()}, {"tasks", json::array()}};。
                    {"steps", json::array()}, {"tasks", json::array()}};
                // 【逐行批注】 按循环规则重复处理：for (const auto& step : output.at("steps")) {。
                for (const auto& step : output.at("steps")) {
                    // 【逐行批注】 执行当前声明或表达式：json clean;。
                    json clean;
                    // 【逐行批注】 按循环规则重复处理：for (const char* key : {"title", "explanation", "example", "action", "chec…。
                    for (const char* key : {"title", "explanation", "example", "action", "check"}) clean[key] = step.at(key);
                    // 【逐行批注】 调用 move 完成本行所需操作：normalized["steps"].push_back(std::move(clean));。
                    normalized["steps"].push_back(std::move(clean));
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 按循环规则重复处理：for (const auto& task : output.at("tasks")) {。
                for (const auto& task : output.at("tasks")) {
                    // 【逐行批注】 执行当前声明或表达式：json clean;。
                    json clean;
                    // 【逐行批注】 按循环规则重复处理：for (const char* key : {"title", "description", "duration", "output", "act…。
                    for (const char* key : {"title", "description", "duration", "output", "actionSteps", "checklist"}) clean[key] = task.at(key);
                    // 【逐行批注】 调用 move 完成本行所需操作：normalized["tasks"].push_back(std::move(clean));。
                    normalized["tasks"].push_back(std::move(clean));
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 char* configuredModel，更新当前运行状态。
                const char* configuredModel = std::getenv("AI_MODEL");
                // 【逐行批注】 计算右侧结果并写入 normalized["_generation"]，更新当前运行状态。
                normalized["_generation"] = {{"source", "ai"},
                    // 【逐行批注】 执行当前声明或表达式：{"model", response.model.empty() && configuredModel ? configuredModel : response.mod…。
                    {"model", response.model.empty() && configuredModel ? configuredModel : response.model},
                    // 【逐行批注】 调用 nowIso8601 完成本行所需操作：{"generatedAt", nowIso8601()}, {"attempts", attempt}, {"promptVersion…。
                    {"generatedAt", nowIso8601()}, {"attempts", attempt}, {"promptVersion", "ai-phase-v1"},
                    // 【逐行批注】 执行当前声明或表达式：{"searchStatus", response.searchStatus}, {"sources", response.sources}};。
                    {"searchStatus", response.searchStatus}, {"sources", response.sources}};
                // 【逐行批注】 执行当前声明或表达式：std::cerr << "[phase] phase=" << phaseIndex << " attempt=" << attempt << " quality=p…。
                std::cerr << "[phase] phase=" << phaseIndex << " attempt=" << attempt << " quality=passed\n";
                // 【逐行批注】 结束当前函数并返回结果：return normalized;。
                return normalized;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 lastType，更新当前运行状态。
            lastType = "quality_rejected";
            // 【逐行批注】 计算右侧结果并写入 lastMessage，更新当前运行状态。
            lastMessage = feedback;
            // 【逐行批注】 执行当前声明或表达式：std::cerr << "[phase] phase=" << phaseIndex << " attempt=" << attempt。
            std::cerr << "[phase] phase=" << phaseIndex << " attempt=" << attempt
                      // 【逐行批注】 执行当前声明或表达式：<< " quality=rejected reason=" << feedback << '\n';。
                      << " quality=rejected reason=" << feedback << '\n';
        // 【逐行批注】 执行当前声明或表达式：} catch (const AIClientError& error) {。
        } catch (const AIClientError& error) {
            // 【逐行批注】 计算右侧结果并写入 lastType，更新当前运行状态。
            lastType = error.errorType;
            // 【逐行批注】 计算右侧结果并写入 lastMessage，更新当前运行状态。
            lastMessage = error.what();
            // 【逐行批注】 计算右侧结果并写入 feedback，更新当前运行状态。
            feedback = "AI 调用失败：" + std::string(error.what());
            // 【逐行批注】 执行当前声明或表达式：std::cerr << "[phase] phase=" << phaseIndex << " attempt=" << attempt。
            std::cerr << "[phase] phase=" << phaseIndex << " attempt=" << attempt
                      // 【逐行批注】 执行当前声明或表达式：<< " ai_error=" << error.errorType << '\n';。
                      << " ai_error=" << error.errorType << '\n';
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception& error) {。
        } catch (const std::exception& error) {
            // 【逐行批注】 计算右侧结果并写入 lastType，更新当前运行状态。
            lastType = "invalid_response";
            // 【逐行批注】 计算右侧结果并写入 lastMessage，更新当前运行状态。
            lastMessage = error.what();
            // 【逐行批注】 计算右侧结果并写入 feedback，更新当前运行状态。
            feedback = "AI 输出无法解析：" + std::string(error.what());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 检测到错误情形时抛出异常：throw AIClientError(lastType, lastMessage);。
    throw AIClientError(lastType, lastMessage);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

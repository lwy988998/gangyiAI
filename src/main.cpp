// 【逐行批注】 引入头文件 "my_ai.hpp"，为后续实现提供类型或接口声明。
#include "my_ai.hpp"
// 【逐行批注】 引入头文件 "local_request_guard.hpp"，为后续实现提供类型或接口声明。
#include "local_request_guard.hpp"
// 【逐行批注】 引入头文件 "learning_library.hpp"，为后续实现提供类型或接口声明。
#include "learning_library.hpp"
// 【逐行批注】 引入头文件 "config.hpp"，为后续实现提供类型或接口声明。
#include "config.hpp"
// 【逐行批注】 引入头文件 "version.hpp"，为后续实现提供类型或接口声明。
#include "version.hpp"
// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"
// 【逐行批注】 引入头文件 "ai_client.hpp"，为后续实现提供类型或接口声明。
#include "ai_client.hpp"
// 【逐行批注】 引入头文件 "page_renderer.hpp"，为后续实现提供类型或接口声明。
#include "page_renderer.hpp"
// 【逐行批注】 引入头文件 "plan_generator.hpp"，为后续实现提供类型或接口声明。
#include "plan_generator.hpp"
// 【逐行批注】 引入头文件 "plan_adapter.hpp"，为后续实现提供类型或接口声明。
#include "plan_adapter.hpp"
// 【逐行批注】 引入头文件 "search_client.hpp"，为后续实现提供类型或接口声明。
#include "search_client.hpp"
// 【逐行批注】 引入头文件 "quality_gate.hpp"，为后续实现提供类型或接口声明。
#include "quality_gate.hpp"
// 【逐行批注】 引入头文件 "course_service.hpp"，为后续实现提供类型或接口声明。
#include "course_service.hpp"
// 【逐行批注】 引入头文件 "progress_service.hpp"，为后续实现提供类型或接口声明。
#include "progress_service.hpp"
// 【逐行批注】 引入头文件 "phase_generator.hpp"，为后续实现提供类型或接口声明。
#include "phase_generator.hpp"
// 【逐行批注】 引入头文件 "ask_generator.hpp"，为后续实现提供类型或接口声明。
#include "ask_generator.hpp"
// 【逐行批注】 引入头文件 "learning_generator.hpp"，为后续实现提供类型或接口声明。
#include "learning_generator.hpp"
// 【逐行批注】 引入头文件 "image_goal_analyzer.hpp"，为后续实现提供类型或接口声明。
#include "image_goal_analyzer.hpp"
// 【逐行批注】 引入头文件 "profile_service.hpp"，为后续实现提供类型或接口声明。
#include "profile_service.hpp"
// 【逐行批注】 引入头文件 "question_evidence.hpp"，为后续实现提供类型或接口声明。
#include "question_evidence.hpp"
// 【逐行批注】 引入头文件 "home_recommendations.hpp"，为后续实现提供类型或接口声明。
#include "home_recommendations.hpp"
// 【逐行批注】 引入头文件 "learning_flow.hpp"，为后续实现提供类型或接口声明。
#include "learning_flow.hpp"
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"
// 【逐行批注】 引入头文件 "agent_preferences.hpp"，为后续实现提供类型或接口声明。
#include "agent_preferences.hpp"
// 【逐行批注】 引入头文件 "agent_lessons.hpp"，为后续实现提供类型或接口声明。
#include "agent_lessons.hpp"
// 【逐行批注】 引入头文件 "agent_curriculum.hpp"，为后续实现提供类型或接口声明。
#include "agent_curriculum.hpp"
// 【逐行批注】 引入头文件 "deep_course.hpp"，为后续实现提供类型或接口声明。
#include "deep_course.hpp"
// 【逐行批注】 引入头文件 "independent_chat.hpp"，为后续实现提供类型或接口声明。
#include "independent_chat.hpp"
// 【逐行批注】 引入头文件 "classroom_service.hpp"，为后续实现提供类型或接口声明。
#include "classroom_service.hpp"
// 【逐行批注】 引入头文件 "next_lesson.hpp"，为后续实现提供类型或接口声明。
#include "next_lesson.hpp"
// 【逐行批注】 引入头文件 "json_fix.hpp"，为后续实现提供类型或接口声明。
#include "json_fix.hpp"

// 【逐行批注】 引入头文件 <crow.h>，为后续实现提供类型或接口声明。
#include <crow.h>
// 【逐行批注】 引入头文件 <crow/multipart.h>，为后续实现提供类型或接口声明。
#include <crow/multipart.h>
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>

// 【逐行批注】 处理条件编译指令：#ifdef DELETE。
#ifdef DELETE
// 【逐行批注】 执行当前声明或表达式：#undef DELETE。
#undef DELETE
// 【逐行批注】 处理条件编译指令：#endif。
#endif

// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <atomic>，为后续实现提供类型或接口声明。
#include <atomic>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <mutex>，为后续实现提供类型或接口声明。
#include <mutex>
// 【逐行批注】 引入头文件 <memory>，为后续实现提供类型或接口声明。
#include <memory>
// 【逐行批注】 引入头文件 <unordered_map>，为后续实现提供类型或接口声明。
#include <unordered_map>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 定义函数 content_type_for，执行该函数声明的职责。
std::string content_type_for(const std::filesystem::path& path) {
    // 【逐行批注】 计算右侧结果并写入 auto extension，更新当前运行状态。
    const auto extension = path.extension().string();
    // 【逐行批注】 判断条件 (extension == ".html") return "text/html; charset=utf-8";，成立时执行对应分支。
    if (extension == ".html") return "text/html; charset=utf-8";
    // 【逐行批注】 判断条件 (extension == ".css") return "text/css; charset=utf-8";，成立时执行对应分支。
    if (extension == ".css") return "text/css; charset=utf-8";
    // 【逐行批注】 判断条件 (extension == ".js") return "application/javascript; charset=utf…，成立时执行对应分支。
    if (extension == ".js") return "application/javascript; charset=utf-8";
    // 【逐行批注】 判断条件 (extension == ".json") return "application/json; charset=utf-8";，成立时执行对应分支。
    if (extension == ".json") return "application/json; charset=utf-8";
    // 【逐行批注】 判断条件 (extension == ".png") return "image/png";，成立时执行对应分支。
    if (extension == ".png") return "image/png";
    // 【逐行批注】 判断条件 (extension == ".jpg" || extension == ".jpeg") return "image/jpeg…，成立时执行对应分支。
    if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    // 【逐行批注】 判断条件 (extension == ".svg") return "image/svg+xml";，成立时执行对应分支。
    if (extension == ".svg") return "image/svg+xml";
    // 【逐行批注】 判断条件 (extension == ".ico") return "image/x-icon";，成立时执行对应分支。
    if (extension == ".ico") return "image/x-icon";
    // 【逐行批注】 判断条件 (extension == ".woff2") return "font/woff2";，成立时执行对应分支。
    if (extension == ".woff2") return "font/woff2";
    // 【逐行批注】 判断条件 (extension == ".woff") return "font/woff";，成立时执行对应分支。
    if (extension == ".woff") return "font/woff";
    // 【逐行批注】 判断条件 (extension == ".ttf") return "font/ttf";，成立时执行对应分支。
    if (extension == ".ttf") return "font/ttf";
    // 【逐行批注】 结束当前函数并返回结果：return "application/octet-stream";。
    return "application/octet-stream";
// 【逐行批注】 结束当前代码块。
}

// 查询参数百分号编码，保证课程 ID 与复习 ID 可安全拼进链接。
// 【逐行批注】 定义函数 queryValue，执行该函数声明的职责。
std::string queryValue(const std::string& value) {
    // 【逐行批注】 计算右侧结果并写入 constexpr char kHex[]，更新当前运行状态。
    static constexpr char kHex[] = "0123456789ABCDEF";
    // 【逐行批注】 执行当前声明或表达式：std::string encoded;。
    std::string encoded;
    // 【逐行批注】 执行当前声明或表达式：encoded.reserve(value.size());。
    encoded.reserve(value.size());
    // 【逐行批注】 按循环规则重复处理：for (const unsigned char character : value) {。
    for (const unsigned char character : value) {
        // 【逐行批注】 计算右侧结果并写入 bool plain，更新当前运行状态。
        const bool plain = (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
            // 【逐行批注】 执行当前声明或表达式：(character >= '0' && character <= '9') || character == '-' || character == '_' ||。
            (character >= '0' && character <= '9') || character == '-' || character == '_' ||
            // 【逐行批注】 执行当前声明或表达式：character == '.' || character == '~';。
            character == '.' || character == '~';
        // 【逐行批注】 判断条件 (plain) {，成立时执行对应分支。
        if (plain) {
            // 【逐行批注】 执行当前声明或表达式：encoded.push_back(static_cast<char>(character));。
            encoded.push_back(static_cast<char>(character));
        // 【逐行批注】 执行当前声明或表达式：} else {。
        } else {
            // 【逐行批注】 执行当前声明或表达式：encoded.push_back('%');。
            encoded.push_back('%');
            // 【逐行批注】 执行当前声明或表达式：encoded.push_back(kHex[character >> 4]);。
            encoded.push_back(kHex[character >> 4]);
            // 【逐行批注】 执行当前声明或表达式：encoded.push_back(kHex[character & 0x0F]);。
            encoded.push_back(kHex[character & 0x0F]);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return encoded;。
    return encoded;
// 【逐行批注】 结束当前代码块。
}

// 各题型共用同一身份参数，恢复和重试时保留巩固课作用域。
// 【逐行批注】 定义函数 questionRequestBody，执行该函数声明的职责。
nlohmann::json questionRequestBody(const crow::request& req) {
    // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
    const auto value = [&req](const char* name) {
        // 【逐行批注】 计算右侧结果并写入 char* raw，更新当前运行状态。
        const char* raw = req.url_params.get(name);
        // 【逐行批注】 结束当前函数并返回结果：return std::string(raw ? raw : "");。
        return std::string(raw ? raw : "");
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json body，更新当前运行状态。
    nlohmann::json body = {{"courseId", value("courseId")},
        // 【逐行批注】 调用 value、stoi 完成本行所需操作：{"phaseIndex", value("phaseIndex").empty() ? 1 : std::stoi(value("pha…。
        {"phaseIndex", value("phaseIndex").empty() ? 1 : std::stoi(value("phaseIndex"))}, {"topicIndex", value("topicIndex").empty() ? 1 : std::stoi(value("topicIndex"))}};
    // 【逐行批注】 按循环规则重复处理：for (const auto* name : {"kind", "lessonTaskId", "lessonId", "topicId", "r…。
    for (const auto* name : {"kind", "lessonTaskId", "lessonId", "topicId", "reviewId"})
        // 【逐行批注】 判断条件 (!value(name).empty()) body[name] = value(name);，成立时执行对应分支。
        if (!value(name).empty()) body[name] = value(name);
    // 【逐行批注】 判断条件 (!value("index").empty()) body["index"] = std::stoi(value("index…，成立时执行对应分支。
    if (!value("index").empty()) body["index"] = std::stoi(value("index"));
    // 【逐行批注】 按循环规则重复处理：for (const auto* name : {"review", "day"})。
    for (const auto* name : {"review", "day"})
        // 【逐行批注】 判断条件 (!value(name).empty()) body[name] = std::stoi(value(name));，成立时执行对应分支。
        if (!value(name).empty()) body[name] = std::stoi(value(name));
    // 【逐行批注】 结束当前函数并返回结果：return body;。
    return body;
// 【逐行批注】 结束当前代码块。
}

// 批量兼容提交保存一个任务，避免每题提交互相使旧任务过期。
// 【逐行批注】 定义函数 submitClassroomBatch，执行该函数声明的职责。
nlohmann::json submitClassroomBatch(gangyi::Database& db, const gangyi::AgentAccess& access, nlohmann::json body) {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 【逐行批注】 判断条件 (!body.value("answers", Json()).is_array()) throw std::invalid_a…，成立时执行对应分支。
    if (!body.value("answers", Json()).is_array()) throw std::invalid_argument("答案列表无效");
    // 【逐行批注】 判断条件 (!body.contains("kind")) body["kind"] = "quiz";，成立时执行对应分支。
    if (!body.contains("kind")) body["kind"] = "quiz";
    // 【逐行批注】 计算右侧结果并写入 auto view，更新当前运行状态。
    const auto view = gangyi::agentQuestionView(db, access, body);
    // 【逐行批注】 计算右侧结果并写入 auto questions，更新当前运行状态。
    const auto questions = view.at("questions");
    // 【逐行批注】 判断条件 (questions.size() != body.at("answers").size()) throw std::inval…，成立时执行对应分支。
    if (questions.size() != body.at("answers").size()) throw std::invalid_argument("题目与作答数量不一致");
    // 【逐行批注】 执行当前声明或表达式：static std::atomic_uint64_t sequence{0};。
    static std::atomic_uint64_t sequence{0};
    // 【逐行批注】 计算右侧结果并写入 auto request，更新当前运行状态。
    const auto request = body.value("requestId", "batch-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-" + std::to_string(++sequence));
    // 【逐行批注】 计算右侧结果并写入 Json inputs，更新当前运行状态。
    Json inputs = Json::array();
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < questions.size(); ++i) {。
    for (size_t i = 0; i < questions.size(); ++i) {
        // 【逐行批注】 计算右侧结果并写入 input，更新当前运行状态。
        auto input = body; input.erase("answers"); input.erase("questions"); input["lessonId"] = view.at("lessonId");
        // 【逐行批注】 计算右侧结果并写入 input["sectionId"]，更新当前运行状态。
        input["sectionId"] = questions[i].at("sectionId"); input["sectionVersion"] = questions[i].at("sectionVersion");
        // 【逐行批注】 计算右侧结果并写入 input["questionId"]，更新当前运行状态。
        input["questionId"] = questions[i].at("questionId"); input["requestId"] = request + ":" + std::to_string(i);
        // 【逐行批注】 计算右侧结果并写入 input["answer"]，更新当前运行状态。
        input["answer"] = body.at("answers").at(i);
        // 【逐行批注】 判断条件 (input.at("answer").is_null()) { input["action"] = "omitted"; in…，成立时执行对应分支。
        if (input.at("answer").is_null()) { input["action"] = "omitted"; input["question"] = "这道题尚未作答"; }
        // 【逐行批注】 在前一条件不成立时继续判断：(input.at("answer") == "unknown") { input["action"] = "unknown";…。
        else if (input.at("answer") == "unknown") { input["action"] = "unknown"; input["question"] = "我暂时不会这道题"; }
        // 【逐行批注】 调用 agentLegacyEvent 完成本行所需操作：inputs.push_back(gangyi::agentLegacyEvent(db, access, input));。
        inputs.push_back(gangyi::agentLegacyEvent(db, access, input));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 bool unanswered，更新当前运行状态。
    const bool unanswered = std::all_of(inputs.begin(), inputs.end(), [](const auto& input) { return input.value("action", "answer") == "omitted" || input.value("action", "answer") == "skip"; });
    // 【逐行批注】 结束当前函数并返回结果：return gangyi::agentSubmit(db, access, {{"type", "question_answer"}, …。
    return gangyi::agentSubmit(db, access, {{"type", "question_answer"}, {"action", unanswered ? "omitted" : "answer"}, {"requestId", request}, {"courseId", body.at("courseId")},
        // 【逐行批注】 执行当前声明或表达式：{"lessonId", view.at("lessonId")}, {"batchAnswers", inputs}, {"text", "请逐项区分真实作答、不会和…。
        {"lessonId", view.at("lessonId")}, {"batchAnswers", inputs}, {"text", "请逐项区分真实作答、不会和漏答，先评价全部实际作答，再讲解，不采用固定通过率。"}});
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

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

// 【逐行批注】 定义函数 todayDate，执行该函数声明的职责。
std::string todayDate() {
    // 【逐行批注】 计算右侧结果并写入 auto now，更新当前运行状态。
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // 【逐行批注】 执行当前声明或表达式：std::tm local{};。
    std::tm local{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 localtime_s 完成本行所需操作：localtime_s(&local, &now);。
    localtime_s(&local, &now);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 localtime_r 完成本行所需操作：localtime_r(&now, &local);。
    localtime_r(&now, &local);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream output;。
    std::ostringstream output;
    // 【逐行批注】 调用 put_time 完成本行所需操作：output << std::put_time(&local, "%Y-%m-%d");。
    output << std::put_time(&local, "%Y-%m-%d");
    // 【逐行批注】 结束当前函数并返回结果：return output.str();。
    return output.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 调用 mondayDate 完成本行所需操作：[[maybe_unused]] std::string mondayDate() {。
[[maybe_unused]] std::string mondayDate() {
    // 【逐行批注】 计算右侧结果并写入 std::string today，更新当前运行状态。
    const std::string today = todayDate();
    // 【逐行批注】 执行当前声明或表达式：std::tm day{};。
    std::tm day{};
    // 【逐行批注】 声明对象 input，并通过构造函数初始化其状态。
    std::istringstream input(today);
    // 【逐行批注】 调用 get_time 完成本行所需操作：input >> std::get_time(&day, "%Y-%m-%d");。
    input >> std::get_time(&day, "%Y-%m-%d");
    // 【逐行批注】 计算右侧结果并写入 day.tm_hour，更新当前运行状态。
    day.tm_hour = 12;
    // 【逐行批注】 调用 mktime 完成本行所需操作：std::mktime(&day);。
    std::mktime(&day);
    // 【逐行批注】 计算右侧结果并写入 day.tm_mday -，更新当前运行状态。
    day.tm_mday -= (day.tm_wday + 6) % 7;
    // 【逐行批注】 调用 mktime 完成本行所需操作：std::mktime(&day);。
    std::mktime(&day);
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream output;。
    std::ostringstream output;
    // 【逐行批注】 调用 put_time 完成本行所需操作：output << std::put_time(&day, "%Y-%m-%d");。
    output << std::put_time(&day, "%Y-%m-%d");
    // 【逐行批注】 结束当前函数并返回结果：return output.str();。
    return output.str();
// 【逐行批注】 结束当前代码块。
}

// 课程结构里每个主题依次回调 (阶段序号, 主题序号, 主题名)。
// 【逐行批注】 执行当前声明或表达式：template <typename Callback>。
template <typename Callback>
// 【逐行批注】 定义函数 forEachTopic，执行该函数声明的职责。
void forEachTopic(const nlohmann::json& payload, Callback callback) {
    // 【逐行批注】 判断条件 (!payload.is_object()) return;，成立时执行对应分支。
    if (!payload.is_object()) return;
    // 【逐行批注】 计算右侧结果并写入 auto stages，更新当前运行状态。
    const auto stages = payload.value("courseStructure", nlohmann::json::array());
    // 【逐行批注】 判断条件 (!stages.is_array()) return;，成立时执行对应分支。
    if (!stages.is_array()) return;
    // 【逐行批注】 计算右侧结果并写入 int phaseIndex，更新当前运行状态。
    int phaseIndex = 0;
    // 【逐行批注】 按循环规则重复处理：for (const auto& stage : stages) {。
    for (const auto& stage : stages) {
        // 【逐行批注】 执行当前声明或表达式：++phaseIndex;。
        ++phaseIndex;
        // 【逐行批注】 判断条件 (!stage.is_object()) continue;，成立时执行对应分支。
        if (!stage.is_object()) continue;
        // 【逐行批注】 计算右侧结果并写入 auto items，更新当前运行状态。
        const auto items = stage.value("topics", nlohmann::json::array());
        // 【逐行批注】 判断条件 (!items.is_array()) continue;，成立时执行对应分支。
        if (!items.is_array()) continue;
        // 【逐行批注】 计算右侧结果并写入 int topicIndex，更新当前运行状态。
        int topicIndex = 0;
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : items) {。
        for (const auto& item : items) {
            // 【逐行批注】 执行当前声明或表达式：++topicIndex;。
            ++topicIndex;
            // 【逐行批注】 判断条件 (item.is_string()) callback(phaseIndex, topicIndex, item.get<std…，成立时执行对应分支。
            if (item.is_string()) callback(phaseIndex, topicIndex, item.get<std::string>());
            // 【逐行批注】 在前一条件不成立时继续判断：(item.is_object()) callback(item.value("legacyPhaseIndex", phase…。
            else if (item.is_object()) callback(item.value("legacyPhaseIndex", phaseIndex), item.value("legacyTopicIndex", topicIndex), item.value("title", ""));
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 调用 courseTopics 完成本行所需操作：[[maybe_unused]] nlohmann::json courseTopics(gangyi::Database& db, co…。
[[maybe_unused]] nlohmann::json courseTopics(gangyi::Database& db, const std::string& courseId, const nlohmann::json& payload) {
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json topics，更新当前运行状态。
    nlohmann::json topics = nlohmann::json::array();
    // 【逐行批注】 按循环规则重复处理：forEachTopic(payload, [&](int phaseIndex, int topicIndex, const std::strin…。
    forEachTopic(payload, [&](int phaseIndex, int topicIndex, const std::string& name) {
        // 【逐行批注】 计算右侧结果并写入 auto progress，更新当前运行状态。
        const auto progress = db.findLearningCardProgress(courseId, phaseIndex, topicIndex);
        // 【逐行批注】 判断条件 (!progress || progress->status != "completed") topics.push_back(…，成立时执行对应分支。
        if (!progress || progress->status != "completed") topics.push_back({{"title", name}, {"phaseIndex", phaseIndex}, {"topicIndex", topicIndex}});
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 结束当前函数并返回结果：return topics;。
    return topics;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明数据结构或类型 ClassroomSocket 及其成员接口。
struct ClassroomSocket {
    // 【逐行批注】 执行当前声明或表达式：std::mutex mutex;。
    std::mutex mutex;
    // 【逐行批注】 执行当前声明或表达式：crow::request request;。
    crow::request request;
    // 【逐行批注】 计算右侧结果并写入 crow::websocket::connection* connection，更新当前运行状态。
    crow::websocket::connection* connection = nullptr;
    // 【逐行批注】 执行当前声明或表达式：std::atomic<bool> cancelled{false};。
    std::atomic<bool> cancelled{false};
    // 【逐行批注】 执行当前声明或表达式：std::atomic<bool> busy{false};。
    std::atomic<bool> busy{false};
    // 【逐行批注】 执行当前声明或表达式：std::atomic<bool> stopRequested{false};。
    std::atomic<bool> stopRequested{false};
    // 【逐行批注】 执行当前声明或表达式：std::string taskId;。
    std::string taskId;
    // 【逐行批注】 定义函数 send，执行该函数声明的职责。
    void send(const nlohmann::json& event) {
        // 【逐行批注】 声明对象 guard，并通过构造函数初始化其状态。
        std::lock_guard<std::mutex> guard(mutex);
        // 【逐行批注】 判断条件 (connection) connection->send_text(event.dump());，成立时执行对应分支。
        if (connection) connection->send_text(event.dump());
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 requesterCanReadCourse，执行该函数声明的职责。
bool requesterCanReadCourse(gangyi::Database&, const gangyi::Course& course, const crow::request&) {
    // 【逐行批注】 结束当前函数并返回结果：return course.status == "active";。
    return course.status == "active";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 requesterCanAccessCourse，并通过构造函数初始化其状态。
bool requesterCanAccessCourse(gangyi::Database& db, const crow::request&, const std::string& courseId,
                              // 【逐行批注】 执行当前声明或表达式：const std::string&) {。
                              const std::string&) {
    // 【逐行批注】 判断条件 (courseId.empty()) return true;，成立时执行对应分支。
    if (courseId.empty()) return true;
    // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
    const auto course = db.getCourse(courseId);
    // 【逐行批注】 结束当前函数并返回结果：return course && course->status == "active";。
    return course && course->status == "active";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 localAgentAccess，执行该函数声明的职责。
gangyi::AgentAccess localAgentAccess(gangyi::Database& db) {
    // 当前桌面服务共用本机档案；可访问课程列表由服务端读取，不接收客户端扩大范围。
    // 【逐行批注】 执行当前声明或表达式：gangyi::AgentAccess access{"local-profile", {}};。
    gangyi::AgentAccess access{"local-profile", {}};
    // 【逐行批注】 按循环规则重复处理：for (const auto& course : db.listCourses()) if (course.status == "active")…。
    for (const auto& course : db.listCourses()) if (course.status == "active") access.courseIds.push_back(course.id);
    // 【逐行批注】 结束当前函数并返回结果：return access;。
    return access;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 localAgentView，并通过构造函数初始化其状态。
nlohmann::json localAgentView(gangyi::Database& db, const std::string& taskId = {}) {
    // 【逐行批注】 执行当前声明或表达式：nlohmann::json result;。
    nlohmann::json result;
    // 【逐行批注】 计算右侧结果并写入 db.readSnapshot([&] { result，更新当前运行状态。
    db.readSnapshot([&] { result = gangyi::agentView(db, localAgentAccess(db), taskId); });
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 studyPlanAgentAction，执行该函数声明的职责。
nlohmann::json studyPlanAgentAction(gangyi::Database& db, nlohmann::json body, const std::string& action) {
    // 【逐行批注】 执行当前声明或表达式：static std::mutex manualPlanMutex;。
    static std::mutex manualPlanMutex;
    // 【逐行批注】 声明对象 guard，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> guard(manualPlanMutex);
    // 【逐行批注】 执行当前声明或表达式：static std::atomic_uint64_t sequence{0};。
    static std::atomic_uint64_t sequence{0};
    // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
    const auto access = localAgentAccess(db);
    // 【逐行批注】 执行当前声明或表达式：nlohmann::json event;。
    nlohmann::json event;
    // 【逐行批注】 判断条件 (action == "replan") {，成立时执行对应分支。
    if (action == "replan") {
        // 【逐行批注】 计算右侧结果并写入 body["requestId"]，更新当前运行状态。
        body["requestId"] = body.value("requestId", "schedule-" + nowIso8601() + "-" + std::to_string(++sequence));
        // 【逐行批注】 计算右侧结果并写入 event，更新当前运行状态。
        event = {{"type", "schedule_replan"}, {"requestId", body.at("requestId")}, {"scheduleRequest", body},
            // 【逐行批注】 执行当前声明或表达式：{"text", "我手动请求重新排课。请根据最新时间设置、全部课程和真实表现生成候选安排，保留当前课表与未保存编辑，等待我确认。"}};。
            {"text", "我手动请求重新排课。请根据最新时间设置、全部课程和真实表现生成候选安排，保留当前课表与未保存编辑，等待我确认。"}};
        // 重复提交复用同一任务，不再次改变学习版本或生成候选。
        // 【逐行批注】 计算右侧结果并写入 auto index，更新当前运行状态。
        const auto index = nlohmann::json::parse(db.profileMeta("agent-index"), nullptr, false);
        // 【逐行批注】 判断条件 (index.is_array()) for (const auto& id : index) {，成立时执行对应分支。
        if (index.is_array()) for (const auto& id : index) {
            // 【逐行批注】 判断条件 (!id.is_string()) continue;，成立时执行对应分支。
            if (!id.is_string()) continue;
            // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
            const auto row = db.getClassroomActivity(id); if (!row) continue;
            // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
            const auto task = nlohmann::json::parse(row->payload, nullptr, false);
            // 【逐行批注】 判断条件 (!task.is_object() || task.value("scopeId", "") != access.scopeI…，成立时执行对应分支。
            if (!task.is_object() || task.value("scopeId", "") != access.scopeId || task.value("requestId", "") != body.at("requestId")) continue;
            // 【逐行批注】 判断条件 (!task.value("manualScheduleReplan", false) || task.at("event") …，成立时执行对应分支。
            if (!task.value("manualScheduleReplan", false) || task.at("event") != event) throw std::invalid_argument("重复排课请求的内容不一致");
            // 【逐行批注】 结束当前函数并返回结果：return {{"ok", true}, {"plan", gangyi::studyPlanView(db)}, {"task", g…。
            return {{"ok", true}, {"plan", gangyi::studyPlanView(db)}, {"task", gangyi::agentView(db, access, id)}};
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
    const auto result = gangyi::studyPlanProposal(db, body, action);
    // 【逐行批注】 判断条件 (action == "confirm" || action == "cancel") {，成立时执行对应分支。
    if (action == "confirm" || action == "cancel") {
        // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
        const auto row = db.getClassroomActivity(body.at("proposalId"));
        // 【逐行批注】 判断条件 (row && row->kind == "agent-change") {，成立时执行对应分支。
        if (row && row->kind == "agent-change") {
            // 【逐行批注】 计算右侧结果并写入 change，更新当前运行状态。
            auto change = nlohmann::json::parse(row->payload);
            // 【逐行批注】 判断条件 (change.at("scopeId") != access.scopeId) throw std::invalid_argu…，成立时执行对应分支。
            if (change.at("scopeId") != access.scopeId) throw std::invalid_argument("无权操作该课表候选");
            // 【逐行批注】 计算右侧结果并写入 change["status"]，更新当前运行状态。
            change["status"] = action == "confirm" ? "applied" : "undone";
            // 【逐行批注】 判断条件 (action == "confirm") change["after"] = db.profileMeta("learning…，成立时执行对应分支。
            if (action == "confirm") change["after"] = db.profileMeta("learning-flow");
            // 【逐行批注】 执行前一条件不成立时的分支。
            else { change["undoneAt"] = nowIso8601(); change["undoFingerprint"] = gangyi::agentLearnerFingerprint(db, access); }
            // 【逐行批注】 调用 agentSaveChange 完成本行所需操作：gangyi::agentSaveChange(db, change);。
            gangyi::agentSaveChange(db, change);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (action != "replan") return result;，成立时执行对应分支。
    if (action != "replan") return result;
    // 【逐行批注】 计算右侧结果并写入 view，更新当前运行状态。
    auto view = result; view["task"] = gangyi::agentSubmit(db, access, event, gangyi::AgentSubmissionOrigin::manualScheduleReplan);
    // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
    const auto raw = db.profileMeta("learning-flow"); auto state = nlohmann::json::parse(raw);
    // 【逐行批注】 计算右侧结果并写入 state["replanTaskId"]，更新当前运行状态。
    state["replanTaskId"] = view["task"]["id"];
    // 【逐行批注】 判断条件 (!db.compareProfileMeta("learning-flow", raw, state.dump())) thr…，成立时执行对应分支。
    if (!db.compareProfileMeta("learning-flow", raw, state.dump())) throw std::invalid_argument("课表状态已变化，请刷新读取已提交的任务");
    // 【逐行批注】 计算右侧结果并写入 view["plan"]，更新当前运行状态。
    view["plan"] = gangyi::studyPlanView(db);
    // 【逐行批注】 结束当前函数并返回结果：return view;。
    return view;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 queryValueFromUrl，执行该函数声明的职责。
std::string queryValueFromUrl(const std::string& url, const std::string& key) {
    // 【逐行批注】 计算右侧结果并写入 std::string needle，更新当前运行状态。
    const std::string needle = key + "=";
    // 【逐行批注】 计算右侧结果并写入 auto queryStart，更新当前运行状态。
    const auto queryStart = url.find('?');
    // 【逐行批注】 判断条件 (queryStart == std::string::npos) return {};，成立时执行对应分支。
    if (queryStart == std::string::npos) return {};
    // 【逐行批注】 计算右侧结果并写入 position，更新当前运行状态。
    auto position = url.find(needle, queryStart + 1);
    // 【逐行批注】 在条件成立期间循环处理：while (position != std::string::npos) {。
    while (position != std::string::npos) {
        // 【逐行批注】 判断条件 (position == queryStart + 1 || url[position - 1] == '&') {，成立时执行对应分支。
        if (position == queryStart + 1 || url[position - 1] == '&') {
            // 【逐行批注】 计算右侧结果并写入 auto start，更新当前运行状态。
            const auto start = position + needle.size();
            // 【逐行批注】 计算右侧结果并写入 auto end，更新当前运行状态。
            const auto end = url.find('&', start);
            // 【逐行批注】 结束当前函数并返回结果：return url.substr(start, end == std::string::npos ? std::string::npos…。
            return url.substr(start, end == std::string::npos ? std::string::npos : end - start);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 position，更新当前运行状态。
        position = url.find(needle, position + needle.size());
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return {};。
    return {};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 requestAnonymousId，执行该函数声明的职责。
std::string requestAnonymousId(const crow::request& req, const nlohmann::json* body = nullptr) {
    // 【逐行批注】 判断条件 (const char* value = req.url_params.get("anonymousId")) return v…，成立时执行对应分支。
    if (const char* value = req.url_params.get("anonymousId")) return value;
    // 【逐行批注】 判断条件 (body && body->is_object() && body->contains("anonymousId") && (…，成立时执行对应分支。
    if (body && body->is_object() && body->contains("anonymousId") && (*body)["anonymousId"].is_string()) {
        // 【逐行批注】 结束当前函数并返回结果：return (*body)["anonymousId"].get<std::string>();。
        return (*body)["anonymousId"].get<std::string>();
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 std::string cookies，更新当前运行状态。
    const std::string cookies = req.get_header_value("Cookie");
    // 【逐行批注】 计算右侧结果并写入 std::string name，更新当前运行状态。
    const std::string name = "gangyi_anonymous_id=";
    // 【逐行批注】 计算右侧结果并写入 auto start，更新当前运行状态。
    const auto start = cookies.find(name);
    // 【逐行批注】 判断条件 (start != std::string::npos) {，成立时执行对应分支。
    if (start != std::string::npos) {
        // 【逐行批注】 计算右侧结果并写入 auto valueStart，更新当前运行状态。
        const auto valueStart = start + name.size();
        // 【逐行批注】 计算右侧结果并写入 auto end，更新当前运行状态。
        const auto end = cookies.find(';', valueStart);
        // 【逐行批注】 结束当前函数并返回结果：return cookies.substr(valueStart, end == std::string::npos ? std::str…。
        return cookies.substr(valueStart, end == std::string::npos ? std::string::npos : end - valueStart);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 std::string referer，更新当前运行状态。
    const std::string referer = req.get_header_value("Referer");
    // 【逐行批注】 结束当前函数并返回结果：return queryValueFromUrl(referer, "anonymousId");。
    return queryValueFromUrl(referer, "anonymousId");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 recordInteraction，并通过构造函数初始化其状态。
void recordInteraction(gangyi::Database& db, const std::string& kind, const nlohmann::json& payload,
                       // 【逐行批注】 计算右侧结果并写入 std::string& courseId，更新当前运行状态。
                       const std::string& courseId = {}, const std::string& conversationId = {}) {
    // 【逐行批注】 执行当前声明或表达式：gangyi::LearningInteraction item;。
    gangyi::LearningInteraction item;
    // 【逐行批注】 计算右侧结果并写入 item.kind，更新当前运行状态。
    item.kind = kind;
    // 【逐行批注】 计算右侧结果并写入 item.payload，更新当前运行状态。
    item.payload = payload.dump();
    // 【逐行批注】 计算右侧结果并写入 item.createdAt，更新当前运行状态。
    item.createdAt = nowIso8601();
    // 【逐行批注】 判断条件 (!courseId.empty()) item.courseId = courseId;，成立时执行对应分支。
    if (!courseId.empty()) item.courseId = courseId;
    // 【逐行批注】 判断条件 (!conversationId.empty()) item.conversationId = conversationId;，成立时执行对应分支。
    if (!conversationId.empty()) item.conversationId = conversationId;
    // 【逐行批注】 判断条件 (!db.insert(item)) throw std::runtime_error("学习记录保存失败");，成立时执行对应分支。
    if (!db.insert(item)) throw std::runtime_error("学习记录保存失败");
    // 【逐行批注】 判断条件 (kind == "course-visit" || kind == "lesson-visit") gangyi::remem…，成立时执行对应分支。
    if (kind == "course-visit" || kind == "lesson-visit") gangyi::rememberCurrentCourse(db, courseId);
// 【逐行批注】 结束当前代码块。
}


// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
    // 【逐行批注】 计算右侧结果并写入 auto config，更新当前运行状态。
    const auto config = gangyi::Config::from_environment();
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db;。
    gangyi::Database db;
    // 自动创建数据库所在目录（首次部署时 data/ 可能不存在，避免 sqlite 打开失败）
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 std::filesystem::path dbPath，更新当前运行状态。
        const std::filesystem::path dbPath = std::filesystem::u8path(config.database_path);
        // 【逐行批注】 判断条件 (dbPath.has_parent_path()) std::filesystem::create_directories(d…，成立时执行对应分支。
        if (dbPath.has_parent_path()) std::filesystem::create_directories(dbPath.parent_path());
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {}。
    } catch (...) {}
    // 【逐行批注】 执行当前声明或表达式：db.open(config.database_path);。
    db.open(config.database_path);
    // 【逐行批注】 执行当前声明或表达式：db.migrate();。
    db.migrate();
    // 【逐行批注】 调用 configureUsage 完成本行所需操作：gangyi::AIActivity::configureUsage(config.database_path);。
    gangyi::AIActivity::configureUsage(config.database_path);
    // 【逐行批注】 计算右侧结果并写入 auto activityPath，更新当前运行状态。
    const auto activityPath = std::filesystem::u8path(config.database_path).parent_path() / "ai-activity.json";
    // 【逐行批注】 计算右侧结果并写入 bool activityExists，更新当前运行状态。
    const bool activityExists = std::filesystem::exists(activityPath);
    // 【逐行批注】 调用 configure 完成本行所需操作：gangyi::AIActivity::configure(activityPath.u8string());。
    gangyi::AIActivity::configure(activityPath.u8string());
    // 【逐行批注】 判断条件 (!activityExists && gangyi::agentView(db, localAgentAccess(db)).…，成立时执行对应分支。
    if (!activityExists && gangyi::agentView(db, localAgentAccess(db)).value("paused", false)) gangyi::AIActivity::setPaused(true);
    // 【逐行批注】 声明对象 learningAgent，并通过构造函数初始化其状态。
    gangyi::LearningAgent learningAgent(config.database_path);
    // 【逐行批注】 调用 registerAgentPreferenceTools 完成本行所需操作：gangyi::registerAgentPreferenceTools(learningAgent);。
    gangyi::registerAgentPreferenceTools(learningAgent);
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(learningAgent);。
    gangyi::registerAgentLessonTools(learningAgent);
    // 【逐行批注】 执行当前声明或表达式：crow::App<gangyi::LocalRequestGuard> app;。
    crow::App<gangyi::LocalRequestGuard> app;
    // 【逐行批注】 计算右侧结果并写入 auto& requestGuard，更新当前运行状态。
    auto& requestGuard = app.get_middleware<gangyi::LocalRequestGuard>();
    // 【逐行批注】 计算右侧结果并写入 requestGuard.port，更新当前运行状态。
    requestGuard.port = config.port;
    // 访问路径可能包含用户填写的学习目标，避免将其写入信息级访问日志。
    // 【逐行批注】 执行当前声明或表达式：app.loglevel(crow::LogLevel::Warning);。
    app.loglevel(crow::LogLevel::Warning);


    // 【逐行批注】 计算右侧结果并写入 auto personalPage，更新当前运行状态。
    const auto personalPage=[](const std::string& page){
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { crow::response response(gangyi::renderMyAIPage(page));response.set_header("Content-Type","text/html; charset=utf-8");return response; }
        // 【逐行批注】 捕获并处理异常：catch(...) {return crow::response(404,"页面不存在");}。
        catch(...) {return crow::response(404,"页面不存在");}
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app,"/my-ai")([personalPage]{return personalPage("overview");});。
    CROW_ROUTE(app,"/my-ai")([personalPage]{return personalPage("overview");});
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app,"/my-ai/<string>")([personalPage](const std::string& page){return per…。
    CROW_ROUTE(app,"/my-ai/<string>")([personalPage](const std::string& page){return personalPage(page);});
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app,"/api/my-ai/<string>").methods(crow::HTTPMethod::GET,crow::HTTPMethod…。
    CROW_ROUTE(app,"/api/my-ai/<string>").methods(crow::HTTPMethod::GET,crow::HTTPMethod::POST)([path=config.database_path](const crow::request& req,const std::string& section){
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection;connection.open(path);nlohmann::json value;。
            gangyi::Database connection;connection.open(path);nlohmann::json value;
            // 【逐行批注】 执行当前声明或表达式：if(req.method==crow::HTTPMethod::GET){。
            if(req.method==crow::HTTPMethod::GET){
                // 【逐行批注】 计算右侧结果并写入 nlohmann::json query，更新当前运行状态。
                nlohmann::json query=nlohmann::json::object();
                // 【逐行批注】 按循环规则重复处理：for(const auto* key:{"id","search","courseId","subject","from","to","model…。
                for(const auto* key:{"id","search","courseId","subject","from","to","model","snapshotId","filter"})if(req.url_params.get(key))query[key]=req.url_params.get(key);
                // 【逐行批注】 计算右侧结果并写入 query["archived"]，更新当前运行状态。
                query["archived"]=req.url_params.get("archived")&&std::string(req.url_params.get("archived"))=="true";
                // 【逐行批注】 执行当前声明或表达式：if(section=="tasks") {。
                if(section=="tasks") {
                    // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
                    const auto access=localAgentAccess(connection);value={{"items",nlohmann::json::array()}};
                    // 【逐行批注】 计算右侧结果并写入 auto ids，更新当前运行状态。
                    const auto ids=nlohmann::json::parse(connection.profileMeta("agent-index"),nullptr,false);
                    // 【逐行批注】 计算右侧结果并写入 if(ids.is_array())for(auto it，更新当前运行状态。
                    if(ids.is_array())for(auto it=ids.rbegin();it!=ids.rend();++it)try{value["items"].push_back(gangyi::agentView(connection,access,it->get<std::string>(),true));}catch(const std::exception&){}
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 执行前一条件不成立时的分支。
                else connection.readSnapshot([&]{value=section=="library" ? gangyi::libraryRead(connection,query) : gangyi::myAIRead(connection,section,query);});
            // 【逐行批注】 执行当前声明或表达式：}else{。
            }else{
                // 【逐行批注】 调用 response 完成本行所需操作：if(req.body.size()>400000)return crow::response(413,"内容太大");。
                if(req.body.size()>400000)return crow::response(413,"内容太大");
                // 【逐行批注】 计算右侧结果并写入 auto body，更新当前运行状态。
                const auto body=nlohmann::json::parse(req.body);
                // 【逐行批注】 调用 myAIProposal 完成本行所需操作：if(section=="proposal")connection.transaction([&]{value=gangyi::myAIP…。
                if(section=="proposal")connection.transaction([&]{value=gangyi::myAIProposal(connection,body,body.at("requestId"));});
                // 【逐行批注】 在前一条件不成立时继续判断：(section=="confirm")value=gangyi::myAIConfirm(connection,body.at…。
                else if(section=="confirm")value=gangyi::myAIConfirm(connection,body.at("id"),body);
                // 【逐行批注】 在前一条件不成立时继续判断：(section=="undo") {。
                else if(section=="undo") {
                    // 【逐行批注】 计算右侧结果并写入 connection.transaction([&]{const auto row，更新当前运行状态。
                    connection.transaction([&]{const auto row=connection.myAIRecord(body.at("id"));if(!row||row->kind!="undo")throw std::invalid_argument("撤回记录不存在");
                        // 【逐行批注】 计算右侧结果并写入 change，更新当前运行状态。
                        auto change=nlohmann::json::parse(row->payload);if(change["state"]=="undone"){value=change;return;}
                        // 【逐行批注】 计算右侧结果并写入 before，更新当前运行状态。
                        auto before=change.at("before");before["version"]=change.at("afterVersion");value=gangyi::myAIWrite(connection,"preferences",before,false);
                        // 【逐行批注】 计算右侧结果并写入 change["state"]，更新当前运行状态。
                        change["state"]="undone";if(!connection.saveMyAIRecord({row->id,row->kind,change.dump(),nowIso8601(),0},row->version))throw std::runtime_error("撤回状态已改变");
                    // 【逐行批注】 执行当前声明或表达式：});。
                    });
                // 【逐行批注】 计算右侧结果并写入 }else value，更新当前运行状态。
                }else value=section=="library" ? gangyi::libraryWrite(connection,body) : gangyi::myAIWrite(connection,section,body);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(value.dump());response.set_header("Content-Type","application/json; charset=utf-8");response.set_header("Cache-Control","no-store");return response;
        // 【逐行批注】 调用 response 完成本行所需操作：}catch(const std::invalid_argument& error){return crow::response(400,…。
        }catch(const std::invalid_argument& error){return crow::response(400,nlohmann::json{{"error",error.what()}}.dump());}
        // 【逐行批注】 捕获并处理异常：catch(const std::exception& error){return crow::response(409,nlohmann…。
        catch(const std::exception& error){return crow::response(409,nlohmann::json{{"error",error.what()}}.dump());}
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app,"/api/my-ai/library/usage")([path=config.database_path]{。
    CROW_ROUTE(app,"/api/my-ai/library/usage")([path=config.database_path]{
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection;connection.open(path);nlohmann::json value;。
        gangyi::Database connection;connection.open(path);nlohmann::json value;
        // 【逐行批注】 计算右侧结果并写入 connection.readSnapshot([&]{value，更新当前运行状态。
        connection.readSnapshot([&]{value=gangyi::libraryUsage(connection);});
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(value.dump());response.set_header("Content-Type","application/json; charset=utf-8");response.set_header("Cache-Control","no-store");return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app,"/api/my-ai/library/research").methods(crow::HTTPMethod::POST)([path=…。
    CROW_ROUTE(app,"/api/my-ai/library/research").methods(crow::HTTPMethod::POST)([path=config.database_path](const crow::request& req){
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 调用 response 完成本行所需操作：if(req.body.size()>100000)return crow::response(413,"内容太大");。
            if(req.body.size()>100000)return crow::response(413,"内容太大");
            // 【逐行批注】 计算右侧结果并写入 gangyi::Database connection;connection.open(…，更新当前运行状态。
            gangyi::Database connection;connection.open(path);const auto body=nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 goal，更新当前运行状态。
            auto goal=body.value("goal",""),sourceId=body.value("sourceId","");
            // 【逐行批注】 调用 libraryRead 完成本行所需操作：if(!sourceId.empty()){const auto source=gangyi::libraryRead(connectio…。
            if(!sourceId.empty()){const auto source=gangyi::libraryRead(connection,{{"id",sourceId}});
                // 【逐行批注】 调用 runtime_error 完成本行所需操作：if(body.contains("version")&&body.at("version")!=source.at("version")…。
                if(body.contains("version")&&body.at("version")!=source.at("version"))throw std::runtime_error("资料版本已变化，请刷新后更新");
                // 【逐行批注】 计算右侧结果并写入 goal，更新当前运行状态。
                goal=source.value("title",goal);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(goal.empty()||goal.size()>3000)throw std::invalid_argument("请填写本次学…。
            if(goal.empty()||goal.size()>3000)throw std::invalid_argument("请填写本次学习目标或问题");
            // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
            const auto event=nlohmann::json{{"type","library_research"},{"goal",goal},{"text",body.value("text",goal)},
                // 【逐行批注】 执行当前声明或表达式：{"courseId",body.value("courseId","")},{"sourceId",sourceId},{"refresh",body.value("…。
                {"courseId",body.value("courseId","")},{"sourceId",sourceId},{"refresh",body.value("refresh",false)},{"requestId",body.at("requestId")}};
            // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
            const auto task=gangyi::agentSubmit(connection,localAgentAccess(connection),event);
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(202,task.dump());response.set_header("Content-Type","application/json; charset=utf-8");return response;
        // 【逐行批注】 调用 response 完成本行所需操作：}catch(const std::invalid_argument& error){return crow::response(400,…。
        }catch(const std::invalid_argument& error){return crow::response(400,nlohmann::json{{"error",error.what()}}.dump());}
        // 【逐行批注】 捕获并处理异常：catch(const std::exception& error){return crow::response(409,nlohmann…。
        catch(const std::exception& error){return crow::response(409,nlohmann::json{{"error",error.what()}}.dump());}
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app,"/api/my-ai/chat/send").methods(crow::HTTPMethod::POST)([path=config.…。
    CROW_ROUTE(app,"/api/my-ai/chat/send").methods(crow::HTTPMethod::POST)([path=config.database_path](const crow::request& req){
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 gangyi::Database connection;connection.open(…，更新当前运行状态。
            gangyi::Database connection;connection.open(path);const auto body=nlohmann::json::parse(req.body);const std::string chatId=body.at("id");
            // 【逐行批注】 计算右侧结果并写入 auto message，更新当前运行状态。
            const auto message=gangyi::myAIChatMessage(connection,chatId,body);const auto access=localAgentAccess(connection);
            // 【逐行批注】 执行当前声明或表达式：nlohmann::json task;。
            nlohmann::json task;
            // 【逐行批注】 计算右侧结果并写入 if(message.contains("taskId"))task，更新当前运行状态。
            if(message.contains("taskId"))task=gangyi::agentView(connection,access,message.at("taskId"));
            // 【逐行批注】 执行前一条件不成立时的分支。
            else {
                // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
                const auto event=nlohmann::json{{"type","assistant_chat"},{"chatId",chatId},{"text",body.at("text")},{"requestId","my-ai:"+chatId+":"+body.at("requestId").get<std::string>()}};
                // 【逐行批注】 计算右侧结果并写入 task，更新当前运行状态。
                task=gangyi::agentSubmit(connection,access,event);gangyi::myAIChatTask(connection,chatId,body.at("requestId"),task);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202,task.dump());。
            return crow::response(202,task.dump());
        // 【逐行批注】 调用 response 完成本行所需操作：}catch(const std::exception& error){return crow::response(409,nlohman…。
        }catch(const std::exception& error){return crow::response(409,nlohmann::json{{"error",error.what()}}.dump());}
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/")([] {。
    CROW_ROUTE(app, "/")([] {
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(gangyi::renderHomePage());
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/ht…，更新当前运行状态。
        response.set_header("Content-Type", "text/html; charset=utf-8");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/plan")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/plan")([&db](const crow::request& req) {
        // 【逐行批注】 计算右侧结果并写入 char* goal，更新当前运行状态。
        const char* goal = req.url_params.get("goal");
        // 【逐行批注】 计算右侧结果并写入 char* mode，更新当前运行状态。
        const char* mode = req.url_params.get("mode");
        // 【逐行批注】 计算右侧结果并写入 char* courseId，更新当前运行状态。
        const char* courseId = req.url_params.get("courseId");
        // 【逐行批注】 计算右侧结果并写入 char* anonymousId，更新当前运行状态。
        const char* anonymousId = req.url_params.get("anonymousId");
        // 访问记录只用于最近课程排序，不作为掌握度证据。
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 判断条件 (courseId && *courseId && requesterCanAccessCourse(db, req, cour…，成立时执行对应分支。
            if (courseId && *courseId && requesterCanAccessCourse(db, req, courseId, requestAnonymousId(req)))
                // 【逐行批注】 调用 recordInteraction、object 完成本行所需操作：recordInteraction(db, "course-visit", nlohmann::json::object(), cours…。
                recordInteraction(db, "course-visit", nlohmann::json::object(), courseId);
        // 【逐行批注】 执行当前声明或表达式：} catch (...) { /* 记录暂时不可用时，仍允许打开课程。 * / }。
        } catch (...) { /* 记录暂时不可用时，仍允许打开课程。 */ }
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(gangyi::renderPlanPage(goal ? goal : "", mode ? mode : "deep",
            // 【逐行批注】 执行当前声明或表达式：courseId ? courseId : "", anonymousId ? anonymousId : ""));。
            courseId ? courseId : "", anonymousId ? anonymousId : ""));
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/ht…，更新当前运行状态。
        response.set_header("Content-Type", "text/html; charset=utf-8");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });


    // /phase 阶段页：从 courseId 课程快照 SSR 渲染阶段与主题网格
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/phase")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/phase")([&db](const crow::request& req) {
        // 【逐行批注】 计算右侧结果并写入 char* courseIdP，更新当前运行状态。
        const char* courseIdP = req.url_params.get("courseId");
        // 【逐行批注】 计算右侧结果并写入 char* anonymousIdP，更新当前运行状态。
        const char* anonymousIdP = req.url_params.get("anonymousId");
        // 【逐行批注】 计算右侧结果并写入 char* goalP，更新当前运行状态。
        const char* goalP = req.url_params.get("goal");
        // 【逐行批注】 计算右侧结果并写入 char* modeP，更新当前运行状态。
        const char* modeP = req.url_params.get("mode");
        // 【逐行批注】 计算右侧结果并写入 char* phaseIndexP，更新当前运行状态。
        const char* phaseIndexP = req.url_params.get("phaseIndex");
        // 【逐行批注】 计算右侧结果并写入 char* phaseNameP，更新当前运行状态。
        const char* phaseNameP = req.url_params.get("phaseName");
        // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
        std::string courseId = courseIdP ? courseIdP : "";
        // 【逐行批注】 计算右侧结果并写入 std::string anonymousId，更新当前运行状态。
        std::string anonymousId = anonymousIdP ? anonymousIdP : "";
        // 【逐行批注】 计算右侧结果并写入 std::string goal，更新当前运行状态。
        std::string goal = goalP ? goalP : "";
        // 【逐行批注】 计算右侧结果并写入 std::string mode，更新当前运行状态。
        std::string mode = modeP && *modeP ? modeP : "";
        // 【逐行批注】 计算右侧结果并写入 std::string phaseIndex，更新当前运行状态。
        std::string phaseIndex = phaseIndexP ? phaseIndexP : "1";
        // 【逐行批注】 计算右侧结果并写入 std::string phaseName，更新当前运行状态。
        std::string phaseName = phaseNameP ? phaseNameP : "";
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json plan，更新当前运行状态。
        nlohmann::json plan = nlohmann::json::object();
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json card，更新当前运行状态。
        nlohmann::json card = nlohmann::json::object();
        // 【逐行批注】 判断条件 (!courseId.empty() && requesterCanAccessCourse(db, req, courseId…，成立时执行对应分支。
        if (!courseId.empty() && requesterCanAccessCourse(db, req, courseId, anonymousId)) {
            // 【逐行批注】 判断条件 (const auto found = gangyi::getCourseWithSnapshot(db, courseId))…，成立时执行对应分支。
            if (const auto found = gangyi::getCourseWithSnapshot(db, courseId)) {
                // 【逐行批注】 判断条件 (found->payload.is_object()) plan = found->payload;，成立时执行对应分支。
                if (found->payload.is_object()) plan = found->payload;
                // 【逐行批注】 判断条件 (goal.empty()) goal = found->course.goal;，成立时执行对应分支。
                if (goal.empty()) goal = found->course.goal;
                // 【逐行批注】 判断条件 (mode.empty()) mode = found->course.mode;，成立时执行对应分支。
                if (mode.empty()) mode = found->course.mode;
                // 【逐行批注】 计算右侧结果并写入 auto stages，更新当前运行状态。
                const auto stages = plan.value("courseStructure", nlohmann::json::array());
                // 【逐行批注】 计算右侧结果并写入 int selected，更新当前运行状态。
                int selected = 0;
                // 【逐行批注】 开始可能抛出异常的保护代码块。
                try { selected = std::max(0, std::stoi(phaseIndex) - 1); } catch (...) { /* 无效显示序号回到第一阶段。 */ }
                // 【逐行批注】 判断条件 (selected >= 0 && selected < static_cast<int>(stages.size())) {，成立时执行对应分支。
                if (selected >= 0 && selected < static_cast<int>(stages.size())) {
                    // 【逐行批注】 计算右侧结果并写入 int displayIndex，更新当前运行状态。
                    int displayIndex = 0;
                    // 【逐行批注】 按循环规则重复处理：for (const auto& item : stages.at(selected).value("topics", nlohmann::json…。
                    for (const auto& item : stages.at(selected).value("topics", nlohmann::json::array())) {
                        // 【逐行批注】 执行当前声明或表达式：++displayIndex;。
                        ++displayIndex;
                        // 【逐行批注】 计算右侧结果并写入 int legacyPhase，更新当前运行状态。
                        const int legacyPhase = item.is_object() ? item.value("legacyPhaseIndex", selected + 1) : selected + 1;
                        // 【逐行批注】 计算右侧结果并写入 int legacyTopic，更新当前运行状态。
                        const int legacyTopic = item.is_object() ? item.value("legacyTopicIndex", displayIndex) : displayIndex;
                        // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
                        const auto saved = db.findLearningCardProgress(courseId, legacyPhase, legacyTopic);
                        // 【逐行批注】 判断条件 (saved) card[std::to_string(displayIndex)] = saved->status;，成立时执行对应分支。
                        if (saved) card[std::to_string(displayIndex)] = saved->status;
                    // 【逐行批注】 结束当前代码块。
                    }
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (mode.empty()) mode = "deep";，成立时执行对应分支。
        if (mode.empty()) mode = "deep";
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(gangyi::renderPhasePage(courseId, anonymousId, goal, mode, phaseIndex, phaseName,
            // 【逐行批注】 调用 publicCoursePayload 完成本行所需操作：gangyi::publicCoursePayload(plan), card));。
            gangyi::publicCoursePayload(plan), card));
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/ht…，更新当前运行状态。
        response.set_header("Content-Type", "text/html; charset=utf-8");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 三个课堂页面共用已保存课时；旧索引只做身份解析。
    // 【逐行批注】 计算右侧结果并写入 auto classroomPage，更新当前运行状态。
    const auto classroomPage = [&db](const crow::request& req, const std::string& view) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 auto scope，更新当前运行状态。
            const auto scope = questionRequestBody(req);
            // 【逐行批注】 计算右侧结果并写入 std::string target，更新当前运行状态。
            const std::string target = req.url_params.get("review") ? "practice" : view;
            // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
            std::string courseId = scope.value("courseId", "");
            // 【逐行批注】 判断条件 (!scope.value("lessonId", "").empty())，成立时执行对应分支。
            if (!scope.value("lessonId", "").empty())
                // 【逐行批注】 计算右侧结果并写入 courseId，更新当前运行状态。
                courseId = gangyi::agentLessonView(db, localAgentAccess(db), scope.at("lessonId")).at("courseId");
            // 【逐行批注】 判断条件 (!courseId.empty() && gangyi::deepCourseEnabled(db, courseId)) {，成立时执行对应分支。
            if (!courseId.empty() && gangyi::deepCourseEnabled(db, courseId)) {
                // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
                crow::response response(gangyi::renderDeepCoursePage());
                // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/ht…，更新当前运行状态。
                response.set_header("Content-Type", "text/html; charset=utf-8"); return response;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (!scope.value("lessonId", "").empty()) {，成立时执行对应分支。
            if (!scope.value("lessonId", "").empty()) {
                // 【逐行批注】 调用 agentLessonView、localAgentAccess 完成本行所需操作：gangyi::agentLessonView(db, localAgentAccess(db), scope.at("lessonId"…。
                gangyi::agentLessonView(db, localAgentAccess(db), scope.at("lessonId"));
                // 【逐行批注】 判断条件 (target != view) { crow::response response(302); response.set_he…，成立时执行对应分支。
                if (target != view) { crow::response response(302); response.set_header("Location", "/practice?" + req.raw_url.substr(req.raw_url.find('?') + 1)); return response; }
                // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
                crow::response response(gangyi::renderClassroomPage(view));
                // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/ht…，更新当前运行状态。
                response.set_header("Content-Type", "text/html; charset=utf-8"); return response;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
            const auto saved = gangyi::agentLegacyLesson(db, localAgentAccess(db), scope);
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(302);
            // 【逐行批注】 判断条件 (!saved.empty()) response.set_header("Location", "/" + target + …，成立时执行对应分支。
            if (!saved.empty()) response.set_header("Location", "/" + target + "?lessonId=" + saved.at("lessonId").get<std::string>() +
                // 【逐行批注】 调用 string 完成本行所需操作：(req.raw_url.find('?') == std::string::npos ? std::string() : "&" + r…。
                (req.raw_url.find('?') == std::string::npos ? std::string() : "&" + req.raw_url.substr(req.raw_url.find('?') + 1)));
            // 【逐行批注】 执行前一条件不成立时的分支。
            else response.set_header("Location", "/agent-prepare.html?" + (req.raw_url.find('?') == std::string::npos ? std::string() : req.raw_url.substr(req.raw_url.find('?') + 1)) + "&view=" + target);
            // 【逐行批注】 结束当前函数并返回结果：return response;。
            return response;
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception&) {。
        } catch (const std::exception&) {
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(409, "课堂入口已失效，请返回课程选择知识点，或让 AI 重新备课。");
            // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/pl…，更新当前运行状态。
            response.set_header("Content-Type", "text/plain; charset=utf-8"); return response;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/learn")([classroomPage](const crow::request& req) { return classro…。
    CROW_ROUTE(app, "/learn")([classroomPage](const crow::request& req) { return classroomPage(req, "learn"); });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/practice")([classroomPage](const crow::request& req) { return clas…。
    CROW_ROUTE(app, "/practice")([classroomPage](const crow::request& req) { return classroomPage(req, "practice"); });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/summary")([classroomPage](const crow::request& req) { return class…。
    CROW_ROUTE(app, "/summary")([classroomPage](const crow::request& req) { return classroomPage(req, "summary"); });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/learn/next")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/learn/next")([&db](const crow::request& req) {
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(302);
        // 【逐行批注】 判断条件 (const char* id = req.url_params.get("id")) {，成立时执行对应分支。
        if (const char* id = req.url_params.get("id")) {
            // 【逐行批注】 判断条件 (const auto row = db.getClassroomActivity(std::string("next-prep…，成立时执行对应分支。
            if (const auto row = db.getClassroomActivity(std::string("next-preparation:") + id)) {
                // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
                const auto saved = nlohmann::json::parse(row->payload, nullptr, false);
                // 【逐行批注】 判断条件 (saved.is_object() && saved.value("status", "") == "ready" && sa…，成立时执行对应分支。
                if (saved.is_object() && saved.value("status", "") == "ready" && saved.value("classroomUrl", "").rfind("/learn?", 0) == 0) {
                    // 【逐行批注】 执行当前声明或表达式：response.set_header("Location", saved.at("classroomUrl").get<std::string>()); return…。
                    response.set_header("Location", saved.at("classroomUrl").get<std::string>()); return response;
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 response.set_header("Location", "/agent-prep…，更新当前运行状态。
                response.set_header("Location", "/agent-prepare.html?courseId=" + queryValue(row->courseId)); return response;
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 string 完成本行所需操作：response.set_header("Location", "/agent-prepare.html" + (req.raw_url.…。
        response.set_header("Location", "/agent-prepare.html" + (req.raw_url.find('?') == std::string::npos ? std::string() : req.raw_url.substr(req.raw_url.find('?'))));
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learn/next").methods(crow::HTTPMethod::POST)([&config](const c…。
    CROW_ROUTE(app, "/api/learn/next").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 gangyi::Database db; db.open(config.database…，更新当前运行状态。
            gangyi::Database db; db.open(config.database_path); auto event = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 event["type"]，更新当前运行状态。
            event["type"] = "prepare_next"; event["navigate"] = true;
            // 【逐行批注】 判断条件 (!event.contains("requestId")) event["requestId"] = "next-" + no…，成立时执行对应分支。
            if (!event.contains("requestId")) event["requestId"] = "next-" + nowIso8601();
            // 【逐行批注】 判断条件 (!event.contains("text")) event["text"] = "依据全部活跃课程与最新实际反馈决定下一课，…，成立时执行对应分支。
            if (!event.contains("text")) event["text"] = "依据全部活跃课程与最新实际反馈决定下一课，完整备课并保存后进入。";
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(d…。
            return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(db), event).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learn/next/<string>")([&config](const crow::request&, const st…。
    CROW_ROUTE(app, "/api/learn/next/<string>")([&config](const crow::request&, const std::string& id) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto task = localAgentView(db, id);
            // 【逐行批注】 判断条件 (task.contains("lesson")) task["href"] = task["lesson"]["href"];，成立时执行对应分支。
            if (task.contains("lesson")) task["href"] = task["lesson"]["href"];
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, task.dump());。
            return crow::response(200, task.dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(404, nl…。
        } catch (const std::exception& error) { return crow::response(404, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learn/next/<string>/cancel").methods(crow::HTTPMethod::POST)([…。
    CROW_ROUTE(app, "/api/learn/next/<string>/cancel").methods(crow::HTTPMethod::POST)([&config](const crow::request&, const std::string& id) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentControl(db, localAgentAccess(…。
            return crow::response(202, gangyi::agentControl(db, localAgentAccess(db), {{"command", "cancel"}, {"taskId", id}}).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learn/next/<string>/retry").methods(crow::HTTPMethod::POST)([&…。
    CROW_ROUTE(app, "/api/learn/next/<string>/retry").methods(crow::HTTPMethod::POST)([&config](const crow::request&, const std::string& id) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentControl(db, localAgentAccess(…。
            return crow::response(202, gangyi::agentControl(db, localAgentAccess(db), {{"command", "retry"}, {"taskId", id}}).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // GET /api/learn —— 已准备课堂直接读取提交的内容。
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learn")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/api/learn")([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 auto scope，更新当前运行状态。
            const auto scope = questionRequestBody(req); const auto access = localAgentAccess(db);
            // 【逐行批注】 判断条件 (scope.contains("lessonId")) return crow::response(200, gangyi::…，成立时执行对应分支。
            if (scope.contains("lessonId")) return crow::response(200, gangyi::agentLessonView(db, access, scope.at("lessonId")).dump());
            // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
            const auto saved = gangyi::agentLegacyLesson(db, access, scope);
            // 【逐行批注】 判断条件 (saved.empty()) return crow::response(409, nlohmann::json{{"stat…，成立时执行对应分支。
            if (saved.empty()) return crow::response(409, nlohmann::json{{"status", "unprepared"}, {"error", "请先让 AI 准备课堂。"}, {"href", "/agent-prepare.html?courseId=" + queryValue(scope.at("courseId").get<std::string>())}}.dump());
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, gangyi::agentLessonView(db, access, saved.…。
            return crow::response(200, gangyi::agentLessonView(db, access, saved.at("lessonId")).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 学习页进度接口：页面已经恢复，步骤和完成状态也必须能稳定保存。
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learning-step-progress")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/api/learning-step-progress")([&db](const crow::request& req) {
        // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
        const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
        // 【逐行批注】 计算右侧结果并写入 std::string anonymousId，更新当前运行状态。
        const std::string anonymousId = requestAnonymousId(req);
        // 【逐行批注】 判断条件 (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {，成立时执行对应分支。
        if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课…。
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 int phaseIndex，更新当前运行状态。
        int phaseIndex = 0;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { if (req.url_params.get("phaseIndex")) phaseIndex = std::stoi(req.url_params.get("phaseIndex")); } catch (...) {}
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json items，更新当前运行状态。
        nlohmann::json items = nlohmann::json::array();
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listLearningStepProgress()) {。
        for (const auto& item : db.listLearningStepProgress()) {
            // 【逐行批注】 判断条件 (item.courseId.value_or("") == courseId && item.phaseIndex == ph…，成立时执行对应分支。
            if (item.courseId.value_or("") == courseId && item.phaseIndex == phaseIndex) {
                // 【逐行批注】 执行当前声明或表达式：items.push_back({{"stepIndex", item.stepIndex}, {"stepTitle", item.stepTitle}, {"sta…。
                items.push_back({{"stepIndex", item.stepIndex}, {"stepTitle", item.stepTitle}, {"status", item.status}});
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}, {"items", ite…。
        return crow::response(200, nlohmann::json{{"ok", true}, {"items", items}}.dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learning-step-progress").methods(crow::HTTPMethod::POST)([&db]…。
    CROW_ROUTE(app, "/api/learning-step-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 body，更新当前运行状态。
            auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
            const std::string courseId = body.value("courseId", "");
            // 【逐行批注】 计算右侧结果并写入 std::string anonymousId，更新当前运行状态。
            const std::string anonymousId = requestAnonymousId(req, &body);
            // 【逐行批注】 判断条件 (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {，成立时执行对应分支。
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课…。
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
            const auto course = courseId.empty() ? std::optional<gangyi::Course>{} : db.getCourse(courseId);
            // 【逐行批注】 计算右侧结果并写入 int phaseIndex，更新当前运行状态。
            const int phaseIndex = body.value("phaseIndex", 0);
            // 【逐行批注】 计算右侧结果并写入 int stepIndex，更新当前运行状态。
            const int stepIndex = body.value("stepIndex", 0);
            // 【逐行批注】 计算右侧结果并写入 auto old，更新当前运行状态。
            const auto old = db.findLearningStepProgress(courseId, phaseIndex, stepIndex);
            // 【逐行批注】 执行当前声明或表达式：gangyi::LearningStepProgress item;。
            gangyi::LearningStepProgress item;
            // 【逐行批注】 计算右侧结果并写入 item.id，更新当前运行状态。
            item.id = old ? old->id : "step-" + courseId + "-" + std::to_string(phaseIndex) + "-" + std::to_string(stepIndex);
            // 【逐行批注】 计算右侧结果并写入 item.courseId，更新当前运行状态。
            item.courseId = courseId.empty() ? std::nullopt : std::optional<std::string>(courseId);
            // 【逐行批注】 计算右侧结果并写入 item.anonymousId，更新当前运行状态。
            item.anonymousId = anonymousId.empty() ? std::nullopt : std::optional<std::string>(anonymousId);
            // 【逐行批注】 计算右侧结果并写入 item.goal，更新当前运行状态。
            item.goal = body.value("goal", course ? course->goal : "");
            // 【逐行批注】 计算右侧结果并写入 item.mode，更新当前运行状态。
            item.mode = body.value("mode", course ? course->mode : "deep");
            // 【逐行批注】 计算右侧结果并写入 item.phaseIndex，更新当前运行状态。
            item.phaseIndex = phaseIndex;
            // 【逐行批注】 计算右侧结果并写入 item.phaseName，更新当前运行状态。
            item.phaseName = body.value("phaseName", "当前阶段");
            // 【逐行批注】 计算右侧结果并写入 item.stepIndex，更新当前运行状态。
            item.stepIndex = stepIndex;
            // 【逐行批注】 计算右侧结果并写入 item.stepTitle，更新当前运行状态。
            item.stepTitle = body.value("stepTitle", "学习步骤");
            // 【逐行批注】 计算右侧结果并写入 item.status，更新当前运行状态。
            item.status = gangyi::normalizeStepStatus(body.value("status", "unset"));
            // 【逐行批注】 计算右侧结果并写入 bool saved，更新当前运行状态。
            const bool saved = old ? db.update(item) : db.insert(item);
            // 【逐行批注】 判断条件 (saved) recordInteraction(db, "step", {{"topic", item.stepTitle}…，成立时执行对应分支。
            if (saved) recordInteraction(db, "step", {{"topic", item.stepTitle}, {"state", item.status}}, courseId);
            // 【逐行批注】 计算右侧结果并写入 auto progress，更新当前运行状态。
            const auto progress = saved ? gangyi::recomputeCourseProgress(db, courseId, anonymousId, item.goal)
                                        // 【逐行批注】 执行当前声明或表达式：: std::optional<nlohmann::json>{};。
                                        : std::optional<nlohmann::json>{};
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(saved ? 200 : 400, nlohmann::json{{"ok", saved},。
            return crow::response(saved ? 200 : 400, nlohmann::json{{"ok", saved},
                // 【逐行批注】 调用 object 完成本行所需操作：{"progress", progress ? *progress : nlohmann::json::object()}}.dump()…。
                {"progress", progress ? *progress : nlohmann::json::object()}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请…。
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learn/progress").methods(crow::HTTPMethod::POST)([&config](con…。
    CROW_ROUTE(app, "/api/learn/progress").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body); const auto access = localAgentAccess(db);
            // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
            const auto saved = body.contains("lessonId") ? nlohmann::json{{"lessonId", body.at("lessonId")}} : gangyi::agentLegacyLesson(db, access, body);
            // 【逐行批注】 判断条件 (saved.empty()) throw std::invalid_argument("课堂尚未准备好");，成立时执行对应分支。
            if (saved.empty()) throw std::invalid_argument("课堂尚未准备好");
            // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
            const auto event = nlohmann::json{{"type", "lesson_finish_request"}, {"courseId", body.at("courseId")}, {"lessonId", saved.at("lessonId")},
                // 【逐行批注】 调用 to_string、now 完成本行所需操作：{"requestId", body.value("requestId", "finish-" + std::to_string(std:…。
                {"requestId", body.value("requestId", "finish-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))},
                // 【逐行批注】 执行当前声明或表达式：{"text", "我希望结束或更新本课，请按实际互动决定完成情况，不使用固定通过率。"}};。
                {"text", "我希望结束或更新本课，请按实际互动决定完成情况，不使用固定通过率。"}};
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, access, event).dum…。
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/ask")([](const crow::request& req) {。
    CROW_ROUTE(app, "/ask")([](const crow::request& req) {
        // 【逐行批注】 计算右侧结果并写入 char* question，更新当前运行状态。
        const char* question = req.url_params.get("question");
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(gangyi::renderAskPage(question ? question : ""));
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/ht…，更新当前运行状态。
        response.set_header("Content-Type", "text/html; charset=utf-8");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/current-course")([&db] {。
    CROW_ROUTE(app, "/current-course")([&db] {
        // 【逐行批注】 判断条件 (const auto course = gangyi::currentCourseForIdentity(db)) {，成立时执行对应分支。
        if (const auto course = gangyi::currentCourseForIdentity(db)) {
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(302);
            // 【逐行批注】 计算右侧结果并写入 response.set_header("Location", "/plan?cours…，更新当前运行状态。
            response.set_header("Location", "/plan?courseId=" + course->id);
            // 【逐行批注】 执行当前声明或表达式：response.set_header("Cache-Control", "no-store");。
            response.set_header("Cache-Control", "no-store");
            // 【逐行批注】 结束当前函数并返回结果：return response;。
            return response;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 bool hasCourses，更新当前运行状态。
        const bool hasCourses = !gangyi::listCoursesForIdentity(db, "", "", 1, 0).empty();
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(gangyi::renderCurrentCourseEmptyPage(hasCourses));
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/ht…，更新当前运行状态。
        response.set_header("Content-Type", "text/html; charset=utf-8");
        // 【逐行批注】 执行当前声明或表达式：response.set_header("Cache-Control", "no-store");。
        response.set_header("Cache-Control", "no-store");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/my-courses")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/my-courses")([&db](const crow::request& req) {
        // 【逐行批注】 计算右侧结果并写入 std::string anonymousId，更新当前运行状态。
        const std::string anonymousId = requestAnonymousId(req);
        // 【逐行批注】 计算右侧结果并写入 auto courses，更新当前运行状态。
        const auto courses = gangyi::listCoursesForIdentity(db, "", "", 100, 0);
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json list，更新当前运行状态。
        nlohmann::json list = nlohmann::json::array();
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json dueReviews，更新当前运行状态。
        nlohmann::json dueReviews = nlohmann::json::array();
        // 【逐行批注】 计算右侧结果并写入 int totalTopics，更新当前运行状态。
        int totalTopics = 0, doneTopics = 0, dueTotal = 0;
        // 【逐行批注】 按循环规则重复处理：for (const auto& c : courses) {。
        for (const auto& c : courses) {
            // 【逐行批注】 计算右侧结果并写入 std::string title，更新当前运行状态。
            const std::string title = c.title.empty() ? (c.goal.empty() ? c.id : c.goal) : c.title;
            // 【逐行批注】 计算右侧结果并写入 int courseTotal，更新当前运行状态。
            int courseTotal = 0, courseDone = 0;
            // 【逐行批注】 判断条件 (const auto found = gangyi::getCourseWithSnapshot(db, c.id)) {，成立时执行对应分支。
            if (const auto found = gangyi::getCourseWithSnapshot(db, c.id)) {
                // 【逐行批注】 按循环规则重复处理：forEachTopic(found->payload, [&](int phaseIndex, int topicIndex, const std…。
                forEachTopic(found->payload, [&](int phaseIndex, int topicIndex, const std::string&) {
                    // 【逐行批注】 执行当前声明或表达式：++courseTotal;。
                    ++courseTotal;
                    // 【逐行批注】 计算右侧结果并写入 auto progress，更新当前运行状态。
                    const auto progress = db.findLearningCardProgress(c.id, phaseIndex, topicIndex);
                    // 【逐行批注】 判断条件 (progress && progress->status == "completed") ++courseDone;，成立时执行对应分支。
                    if (progress && progress->status == "completed") ++courseDone;
                // 【逐行批注】 执行当前声明或表达式：});。
                });
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 int courseDue，更新当前运行状态。
            int courseDue = 0;
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try {
                // 【逐行批注】 按循环规则重复处理：for (const auto& review : gangyi::dueReviews(db, c.id, todayDate())) {。
                for (const auto& review : gangyi::dueReviews(db, c.id, todayDate())) {
                    // 【逐行批注】 执行当前声明或表达式：++courseDue;。
                    ++courseDue;
                    // 【逐行批注】 计算右侧结果并写入 std::string reviewHref，更新当前运行状态。
                    std::string reviewHref = "/learn?courseId=" + queryValue(c.id) +
                        // 【逐行批注】 计算右侧结果并写入 "&phaseIndex，更新当前运行状态。
                        "&phaseIndex=" + std::to_string(review.value("phaseIndex", 0)) +
                        // 【逐行批注】 计算右侧结果并写入 "&topicIndex，更新当前运行状态。
                        "&topicIndex=" + std::to_string(review.value("topicIndex", 0)) +
                        // 【逐行批注】 计算右侧结果并写入 "&review，更新当前运行状态。
                        "&review=" + std::to_string(review.value("day", 0));
                    // 【逐行批注】 计算右侧结果并写入 std::string reviewId，更新当前运行状态。
                    const std::string reviewId = review.value("reviewId", std::string());
                    // 【逐行批注】 判断条件 (!reviewId.empty()) reviewHref += "&reviewId=" + queryValue(revi…，成立时执行对应分支。
                    if (!reviewId.empty()) reviewHref += "&reviewId=" + queryValue(reviewId);
                    // 【逐行批注】 执行当前声明或表达式：dueReviews.push_back({{"course", title},。
                    dueReviews.push_back({{"course", title},
                        // 【逐行批注】 调用 string 完成本行所需操作：{"title", review.value("title", std::string("到期复习"))},。
                        {"title", review.value("title", std::string("到期复习"))},
                        // 【逐行批注】 调用 string 完成本行所需操作：{"due", review.value("due", std::string())},。
                        {"due", review.value("due", std::string())},
                        // 【逐行批注】 执行当前声明或表达式：{"href", reviewHref}});。
                        {"href", reviewHref}});
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 执行当前声明或表达式：} catch (...) {}。
            } catch (...) {}
            // 【逐行批注】 计算右侧结果并写入 totalTopics +，更新当前运行状态。
            totalTopics += courseTotal;
            // 【逐行批注】 计算右侧结果并写入 doneTopics +，更新当前运行状态。
            doneTopics += courseDone;
            // 【逐行批注】 计算右侧结果并写入 dueTotal +，更新当前运行状态。
            dueTotal += courseDue;
            // 【逐行批注】 执行当前声明或表达式：list.push_back({{"courseId", c.id}, {"title", title}, {"goal", c.goal}, {"mode", c.m…。
            list.push_back({{"courseId", c.id}, {"title", title}, {"goal", c.goal}, {"mode", c.mode},
                // 【逐行批注】 调用 deepCourseEnabled 完成本行所需操作：{"pedagogy", gangyi::deepCourseEnabled(db, c.id) ? "deep-v1" : "quick…。
                {"pedagogy", gangyi::deepCourseEnabled(db, c.id) ? "deep-v1" : "quick"},
                // 【逐行批注】 执行当前声明或表达式：{"source", c.source}, {"createdAt", c.createdAt}, {"updatedAt", c.updatedAt},。
                {"source", c.source}, {"createdAt", c.createdAt}, {"updatedAt", c.updatedAt},
                // 【逐行批注】 执行当前声明或表达式：{"status", "generated"}, {"totalTopics", courseTotal}, {"doneTopics", courseDone},。
                {"status", "generated"}, {"totalTopics", courseTotal}, {"doneTopics", courseDone},
                // 【逐行批注】 执行当前声明或表达式：{"percent", courseTotal > 0 ? courseDone * 100 / courseTotal : 0},。
                {"percent", courseTotal > 0 ? courseDone * 100 / courseTotal : 0},
                // 【逐行批注】 执行当前声明或表达式：{"dueCount", courseDue}});。
                {"dueCount", courseDue}});
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(gangyi::renderMyCoursesPage({{"courses", list}, {"anonymousId", anonymousId},
            // 【逐行批注】 执行当前声明或表达式：{"dueReviews", dueReviews},。
            {"dueReviews", dueReviews},
            // 【逐行批注】 执行当前声明或表达式：{"stats", {{"total", static_cast<int>(courses.size())}, {"totalTopics", totalTopics},。
            {"stats", {{"total", static_cast<int>(courses.size())}, {"totalTopics", totalTopics},
                // 【逐行批注】 执行当前声明或表达式：{"doneTopics", doneTopics}, {"due", dueTotal},。
                {"doneTopics", doneTopics}, {"due", dueTotal},
                // 【逐行批注】 执行当前声明或表达式：{"percent", totalTopics > 0 ? doneTopics * 100 / totalTopics : 0}}}}));。
                {"percent", totalTopics > 0 ? doneTopics * 100 / totalTopics : 0}}}}));
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/ht…，更新当前运行状态。
        response.set_header("Content-Type", "text/html; charset=utf-8");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });




    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/ask").methods(crow::HTTPMethod::POST)([&config](const crow::re…。
    CROW_ROUTE(app, "/api/ask").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 执行当前声明或表达式：static std::atomic_uint64_t sequence{0};。
            static std::atomic_uint64_t sequence{0};
            // 【逐行批注】 计算右侧结果并写入 event，更新当前运行状态。
            auto event = body; event["type"] = "chat"; event["text"] = body.value("question", body.value("text", ""));
            // 【逐行批注】 计算右侧结果并写入 event["requestId"]，更新当前运行状态。
            event["requestId"] = body.value("requestId", "chat-" + nowIso8601() + "-" + std::to_string(++sequence));
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(d…。
            return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(db), event).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 追问入口只读取已有内容，不用固定建议替代 AI 教学。
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/ask/suggestions").methods(crow::HTTPMethod::POST)([] {。
    CROW_ROUTE(app, "/api/ask/suggestions").methods(crow::HTTPMethod::POST)([] {
        // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}, {"suggestions…。
        return crow::response(200, nlohmann::json{{"ok", true}, {"suggestions", nlohmann::json::array()}}.dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 课堂内上下文对话：问题必须携带当前课程目标、阶段和主题，避免退化为全局闲聊。
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learning-chat").methods(crow::HTTPMethod::POST)([&config](cons…。
    CROW_ROUTE(app, "/api/learning-chat").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 执行当前声明或表达式：static std::atomic_uint64_t sequence{0};。
            static std::atomic_uint64_t sequence{0};
            // 【逐行批注】 计算右侧结果并写入 event，更新当前运行状态。
            auto event = body; event["type"] = "chat"; event["text"] = body.value("question", body.value("text", ""));
            // 【逐行批注】 计算右侧结果并写入 event["requestId"]，更新当前运行状态。
            event["requestId"] = body.value("requestId", "chat-" + nowIso8601() + "-" + std::to_string(++sequence));
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(d…。
            return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(db), event).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 执行当前声明或表达式：std::mutex socketMapMutex;。
    std::mutex socketMapMutex;
    // 【逐行批注】 执行当前声明或表达式：std::atomic<int> socketWorkerCount{0};。
    std::atomic<int> socketWorkerCount{0};
    // 【逐行批注】 执行当前声明或表达式：std::unordered_map<crow::websocket::connection*, std::shared_ptr<ClassroomSocket>> s…。
    std::unordered_map<crow::websocket::connection*, std::shared_ptr<ClassroomSocket>> socketMap;
    // 【逐行批注】 计算右侧结果并写入 auto socketAccept，更新当前运行状态。
    const auto socketAccept = [&requestGuard](const crow::request& request, void** userData) {
            // 【逐行批注】 判断条件 (!requestGuard.allows(request)) return false;，成立时执行对应分支。
            if (!requestGuard.allows(request)) return false;
            *userData = new crow::request(request);
            // 【逐行批注】 结束当前函数并返回结果：return true;。
            return true;
        // 【逐行批注】 执行当前声明或表达式：};。
        };
    // 【逐行批注】 计算右侧结果并写入 auto socketOpen，更新当前运行状态。
    const auto socketOpen = [&](crow::websocket::connection& connection) {
            // 【逐行批注】 计算右侧结果并写入 state，更新当前运行状态。
            auto state = std::make_shared<ClassroomSocket>();
            // 【逐行批注】 声明对象 request，并通过构造函数初始化其状态。
            std::unique_ptr<crow::request> request(static_cast<crow::request*>(connection.userdata()));
            // 【逐行批注】 判断条件 (request) state->request = *request;，成立时执行对应分支。
            if (request) state->request = *request;
            // 【逐行批注】 执行当前声明或表达式：connection.userdata(nullptr);。
            connection.userdata(nullptr);
            // 【逐行批注】 执行当前声明或表达式：state->connection = &connection;。
            state->connection = &connection;
            // 【逐行批注】 声明对象 guard，并通过构造函数初始化其状态。
            std::lock_guard<std::mutex> guard(socketMapMutex);
            // 【逐行批注】 计算右侧结果并写入 socketMap[&connection]，更新当前运行状态。
            socketMap[&connection] = std::move(state);
        // 【逐行批注】 执行当前声明或表达式：};。
        };
    // 【逐行批注】 计算右侧结果并写入 auto socketClose，更新当前运行状态。
    const auto socketClose = [&](crow::websocket::connection& connection, const std::string&, uint16_t) {
            // 【逐行批注】 执行当前声明或表达式：std::shared_ptr<ClassroomSocket> state;。
            std::shared_ptr<ClassroomSocket> state;
            // 【逐行批注】 开始当前代码块。
            {
                // 【逐行批注】 声明对象 guard，并通过构造函数初始化其状态。
                std::lock_guard<std::mutex> guard(socketMapMutex);
                // 【逐行批注】 计算右侧结果并写入 auto it，更新当前运行状态。
                const auto it = socketMap.find(&connection);
                // 【逐行批注】 判断条件 (it != socketMap.end()) { state = it->second; socketMap.erase(it…，成立时执行对应分支。
                if (it != socketMap.end()) { state = it->second; socketMap.erase(it); }
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (state) {，成立时执行对应分支。
            if (state) {
                // 【逐行批注】 执行当前声明或表达式：state->cancelled = true;。
                state->cancelled = true;
                // 【逐行批注】 声明对象 guard，并通过构造函数初始化其状态。
                std::lock_guard<std::mutex> guard(state->mutex);
                // 【逐行批注】 执行当前声明或表达式：state->connection = nullptr;。
                state->connection = nullptr;
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 执行当前声明或表达式：};。
        };
    // 【逐行批注】 计算右侧结果并写入 auto socketMessage，更新当前运行状态。
    const auto socketMessage = [&](crow::websocket::connection& connection, const std::string& message, bool binary, bool classroom) {
        // 【逐行批注】 执行当前声明或表达式：std::shared_ptr<ClassroomSocket> state;。
        std::shared_ptr<ClassroomSocket> state;
        // 【逐行批注】 调用 guard 完成本行所需操作：{ std::lock_guard<std::mutex> guard(socketMapMutex); const auto found…。
        { std::lock_guard<std::mutex> guard(socketMapMutex); const auto found = socketMap.find(&connection);
          // 【逐行批注】 判断条件 (found != socketMap.end()) state = found->second; }，成立时执行对应分支。
          if (found != socketMap.end()) state = found->second; }
        // 【逐行批注】 判断条件 (!state || binary) return;，成立时执行对应分支。
        if (!state || binary) return;
        // 【逐行批注】 计算右侧结果并写入 auto body，更新当前运行状态。
        const auto body = nlohmann::json::parse(message, nullptr, false);
        // 【逐行批注】 判断条件 (!body.is_object()) { state->send({{"type", "error"}, {"message"…，成立时执行对应分支。
        if (!body.is_object()) { state->send({{"type", "error"}, {"message", "消息格式无效"}}); return; }
        // 【逐行批注】 判断条件 (body.value("type", "") == "stop") {，成立时执行对应分支。
        if (body.value("type", "") == "stop") {
            // 停止生成后仍发送终态，让客户端解除等待；只有断开连接才终止投递。
            // 【逐行批注】 执行当前声明或表达式：state->stopRequested = true;。
            state->stopRequested = true;
            // 【逐行批注】 调用 guard 完成本行所需操作：std::string taskId; { std::lock_guard<std::mutex> guard(state->mutex)…。
            std::string taskId; { std::lock_guard<std::mutex> guard(state->mutex); taskId = state->taskId; }
            // 【逐行批注】 判断条件 (!taskId.empty()) try { gangyi::Database db; db.open(config.data…，成立时执行对应分支。
            if (!taskId.empty()) try { gangyi::Database db; db.open(config.database_path);
                // 【逐行批注】 调用 agentControl、localAgentAccess 完成本行所需操作：gangyi::agentControl(db, localAgentAccess(db), {{"command", "cancel"}…。
                gangyi::agentControl(db, localAgentAccess(db), {{"command", "cancel"}, {"taskId", taskId}});
            // 【逐行批注】 执行当前声明或表达式：} catch (...) { /* 已完成或失效任务仍保留已有结果。 * / }。
            } catch (...) { /* 已完成或失效任务仍保留已有结果。 */ }
            // 【逐行批注】 结束当前函数并返回结果：return;。
            return;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (body.value("type", "") != "ask" || state->busy.exchange(true)) …，成立时执行对应分支。
        if (body.value("type", "") != "ask" || state->busy.exchange(true)) return;
        // 【逐行批注】 执行当前声明或表达式：state->cancelled = false; state->stopRequested = false; ++socketWorkerCount;。
        state->cancelled = false; state->stopRequested = false; ++socketWorkerCount;
        // 【逐行批注】 调用 thread 完成本行所需操作：std::thread([state, body, classroom, &config, &socketWorkerCount] {。
        std::thread([state, body, classroom, &config, &socketWorkerCount] {
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try {
                // 【逐行批注】 计算右侧结果并写入 gangyi::Database db; db.open(config.database…，更新当前运行状态。
                gangyi::Database db; db.open(config.database_path); const auto access = localAgentAccess(db);
                // 【逐行批注】 执行当前声明或表达式：nlohmann::json event;。
                nlohmann::json event;
                // 【逐行批注】 判断条件 (classroom && body.contains("kind")) event = gangyi::agentLegacy…，成立时执行对应分支。
                if (classroom && body.contains("kind")) event = gangyi::agentLegacyEvent(db, access, body);
                // 【逐行批注】 执行前一条件不成立时的分支。
                else {
                    // 【逐行批注】 执行当前声明或表达式：static std::atomic_uint64_t sequence{0};。
                    static std::atomic_uint64_t sequence{0};
                    // 【逐行批注】 计算右侧结果并写入 event，更新当前运行状态。
                    event = {{"type", "chat"}, {"courseId", body.value("courseId", "")}, {"text", body.at("question")},
                        // 【逐行批注】 调用 nowIso8601、to_string 完成本行所需操作：{"requestId", body.value("requestId", "chat-" + nowIso8601() + "-" + …。
                        {"requestId", body.value("requestId", "chat-" + nowIso8601() + "-" + std::to_string(++sequence))},
                        // 【逐行批注】 执行当前声明或表达式：{"conversationId", body.value("conversationId", "general")}};。
                        {"conversationId", body.value("conversationId", "general")}};
                    // 【逐行批注】 判断条件 (body.contains("lessonId")) event["lessonId"] = body.at("lessonI…，成立时执行对应分支。
                    if (body.contains("lessonId")) event["lessonId"] = body.at("lessonId");
                    // 【逐行批注】 判断条件 (body.contains("selection")) event["selection"] = body.at("selec…，成立时执行对应分支。
                    if (body.contains("selection")) event["selection"] = body.at("selection");
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 auto created，更新当前运行状态。
                const auto created = gangyi::agentSubmit(db, access, event); const auto taskId = created.at("id").get<std::string>();
                // 【逐行批注】 调用 guard 完成本行所需操作：{ std::lock_guard<std::mutex> guard(state->mutex); state->taskId = ta…。
                { std::lock_guard<std::mutex> guard(state->mutex); state->taskId = taskId; }
                // 【逐行批注】 判断条件 (state->stopRequested) gangyi::agentControl(db, access, {{"comma…，成立时执行对应分支。
                if (state->stopRequested) gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", taskId}});
                // 【逐行批注】 计算右侧结果并写入 int sequence，更新当前运行状态。
                int sequence = 0;
                // 【逐行批注】 在条件成立期间循环处理：while (!state->cancelled) {。
                while (!state->cancelled) {
                    // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
                    const auto task = localAgentView(db, taskId);
                    // 【逐行批注】 按循环规则重复处理：for (const auto& item : task.at("events")) if (item.value("seq", 0) > sequ…。
                    for (const auto& item : task.at("events")) if (item.value("seq", 0) > sequence) {
                        // 【逐行批注】 计算右侧结果并写入 sequence，更新当前运行状态。
                        sequence = item.at("seq");
                        // 【逐行批注】 判断条件 (item.value("type", "") == "delta" && item.value("field", "") ==…，成立时执行对应分支。
                        if (item.value("type", "") == "delta" && item.value("field", "") == "message")
                            // 【逐行批注】 调用 send 完成本行所需操作：state->send({{"type", "delta"}, {"text", item.at("text")}, {"taskId",…。
                            state->send({{"type", "delta"}, {"text", item.at("text")}, {"taskId", taskId}, {"seq", sequence}});
                    // 【逐行批注】 结束当前代码块。
                    }
                    // 【逐行批注】 计算右侧结果并写入 auto status，更新当前运行状态。
                    const auto status = task.value("status", "");
                    // 【逐行批注】 判断条件 (status != "pending" && status != "running") {，成立时执行对应分支。
                    if (status != "pending" && status != "running") {
                        // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
                        const bool valid = status == "ready" || status == "waiting_student";
                        // 【逐行批注】 调用 send 完成本行所需操作：state->send({{"type", valid || status == "cancelled" ? "done" : "erro…。
                        state->send({{"type", valid || status == "cancelled" ? "done" : "error"}, {"cancelled", status == "cancelled"},
                            // 【逐行批注】 执行当前声明或表达式：{"message", task.value("error", task.value("message", "本次任务已停止，已有记录保留。"))},。
                            {"message", task.value("error", task.value("message", "本次任务已停止，已有记录保留。"))},
                            // 【逐行批注】 执行当前声明或表达式：{"model", task.value("model", "")}, {"taskId", taskId}, {"version", task.value("vers…。
                            {"model", task.value("model", "")}, {"taskId", taskId}, {"version", task.value("version", 0)}, {"taskStatus", status}});
                        // 【逐行批注】 跳出当前循环或分支。
                        break;
                    // 【逐行批注】 结束当前代码块。
                    }
                    // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(100));。
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 调用 send 完成本行所需操作：} catch (const std::exception& error) { state->send({{"type", "error"…。
            } catch (const std::exception& error) { state->send({{"type", "error"}, {"message", error.what()}}); }
            // 【逐行批注】 执行当前声明或表达式：state->busy = false; --socketWorkerCount;。
            state->busy = false; --socketWorkerCount;
        // 【逐行批注】 执行当前声明或表达式：}).detach();。
        }).detach();
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 调用 CROW_WEBSOCKET_ROUTE 完成本行所需操作：CROW_WEBSOCKET_ROUTE(app, "/ws/classroom").onaccept(socketAccept).ono…。
    CROW_WEBSOCKET_ROUTE(app, "/ws/classroom").onaccept(socketAccept).onopen(socketOpen).onclose(socketClose)
        // 【逐行批注】 执行当前声明或表达式：.onmessage([&](crow::websocket::connection& connection, const std::string& message, …。
        .onmessage([&](crow::websocket::connection& connection, const std::string& message, bool binary) {
            // 【逐行批注】 调用 socketMessage 完成本行所需操作：socketMessage(connection, message, binary, true);。
            socketMessage(connection, message, binary, true);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
    // 【逐行批注】 调用 CROW_WEBSOCKET_ROUTE 完成本行所需操作：CROW_WEBSOCKET_ROUTE(app, "/ws/ask").onaccept(socketAccept).onopen(so…。
    CROW_WEBSOCKET_ROUTE(app, "/ws/ask").onaccept(socketAccept).onopen(socketOpen).onclose(socketClose)
        // 【逐行批注】 执行当前声明或表达式：.onmessage([&](crow::websocket::connection& connection, const std::string& message, …。
        .onmessage([&](crow::websocket::connection& connection, const std::string& message, bool binary) {
            // 【逐行批注】 调用 socketMessage 完成本行所需操作：socketMessage(connection, message, binary, false);。
            socketMessage(connection, message, binary, false);
        // 【逐行批注】 执行当前声明或表达式：});。
        });

    // 新旧流式订阅共用任务事件，刷新与断线恢复不重复请求模型。
    // 【逐行批注】 计算右侧结果并写入 auto subscribeAgentTask，更新当前运行状态。
    const auto subscribeAgentTask = [&](crow::websocket::connection& connection, const std::string& message, bool binary) {
        // 【逐行批注】 执行当前声明或表达式：std::shared_ptr<ClassroomSocket> state;。
        std::shared_ptr<ClassroomSocket> state;
        // 【逐行批注】 调用 guard 完成本行所需操作：{ std::lock_guard<std::mutex> guard(socketMapMutex); const auto found…。
        { std::lock_guard<std::mutex> guard(socketMapMutex); const auto found = socketMap.find(&connection);
          // 【逐行批注】 判断条件 (found != socketMap.end()) state = found->second; }，成立时执行对应分支。
          if (found != socketMap.end()) state = found->second; }
        // 【逐行批注】 判断条件 (!state || binary || state->busy.exchange(true)) return;，成立时执行对应分支。
        if (!state || binary || state->busy.exchange(true)) return;
        // 【逐行批注】 计算右侧结果并写入 auto body，更新当前运行状态。
        const auto body = nlohmann::json::parse(message, nullptr, false);
        // 【逐行批注】 判断条件 (!body.is_object() || (!body.value("taskId", nlohmann::json()).i…，成立时执行对应分支。
        if (!body.is_object() || (!body.value("taskId", nlohmann::json()).is_string() && !body.value("id", nlohmann::json()).is_string()) ||
            // 【逐行批注】 调用 json 完成本行所需操作：!body.value("afterSeq", nlohmann::json(0)).is_number_integer()) {。
            !body.value("afterSeq", nlohmann::json(0)).is_number_integer()) {
            // 【逐行批注】 调用 send 完成本行所需操作：state->send({{"type", "failed"}, {"message", "任务标识无效。"}}); state->bus…。
            state->send({{"type", "failed"}, {"message", "任务标识无效。"}}); state->busy = false; return;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：++socketWorkerCount;。
        ++socketWorkerCount;
        // 【逐行批注】 计算右侧结果并写入 std::thread([state, body, databasePath，更新当前运行状态。
        std::thread([state, body, databasePath = config.database_path, &socketWorkerCount] {
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try {
                // 【逐行批注】 计算右侧结果并写入 gangyi::Database db; db.open(databasePath); …，更新当前运行状态。
                gangyi::Database db; db.open(databasePath); int sequence = body.value("afterSeq", 0);
                // 【逐行批注】 计算右侧结果并写入 auto taskId，更新当前运行状态。
                const auto taskId = body.value("taskId", body.value("id", ""));
                // 【逐行批注】 在条件成立期间循环处理：while (!state->cancelled) {。
                while (!state->cancelled) {
                    // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
                    const auto task = localAgentView(db, taskId);
                    // 【逐行批注】 按循环规则重复处理：for (const auto& event : task.value("events", nlohmann::json::array())) if…。
                    for (const auto& event : task.value("events", nlohmann::json::array())) if (event.value("seq", 0) > sequence) {
                        // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
                        auto value = event; value["taskId"] = task.at("id");
                        // 【逐行批注】 判断条件 (value.value("type", "") == "ready" && task.contains("lesson")) …，成立时执行对应分支。
                        if (value.value("type", "") == "ready" && task.contains("lesson")) value["href"] = task.at("lesson").at("href");
                        // 【逐行批注】 调用 send 完成本行所需操作：state->send(value); sequence = event.at("seq");。
                        state->send(value); sequence = event.at("seq");
                    // 【逐行批注】 结束当前代码块。
                    }
                    // 【逐行批注】 调用 send 完成本行所需操作：state->send({{"type", "state"}, {"task", task}});。
                    state->send({{"type", "state"}, {"task", task}});
                    // 【逐行批注】 计算右侧结果并写入 auto status，更新当前运行状态。
                    const auto status = task.value("status", "");
                    // 【逐行批注】 判断条件 (status != "pending" && status != "running") break;，成立时执行对应分支。
                    if (status != "pending" && status != "running") break;
                    // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(100));。
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 调用 send 完成本行所需操作：} catch (const std::exception& error) { state->send({{"type", "failed…。
            } catch (const std::exception& error) { state->send({{"type", "failed"}, {"message", error.what()}}); }
            // 【逐行批注】 执行当前声明或表达式：state->busy = false; --socketWorkerCount;。
            state->busy = false; --socketWorkerCount;
        // 【逐行批注】 执行当前声明或表达式：}).detach();。
        }).detach();
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 调用 CROW_WEBSOCKET_ROUTE 完成本行所需操作：CROW_WEBSOCKET_ROUTE(app, "/ws/learn/next").onaccept(socketAccept).on…。
    CROW_WEBSOCKET_ROUTE(app, "/ws/learn/next").onaccept(socketAccept).onopen(socketOpen).onclose(socketClose).onmessage(subscribeAgentTask);
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/questions")([&config](const crow::request& req) {。
    CROW_ROUTE(app, "/api/classroom/questions")([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, gangyi::agentQuestionView(db, localAgentAc…。
            return crow::response(200, gangyi::agentQuestionView(db, localAgentAccess(db), questionRequestBody(req)).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/question/skip").methods(crow::HTTPMethod::POST)([&co…。
    CROW_ROUTE(app, "/api/classroom/question/skip").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 body["action"]，更新当前运行状态。
            body["action"] = "skip"; if (!body.contains("question") && !body.contains("answer")) body["question"] = "我跳过这道题";
            // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
            const auto access = localAgentAccess(db); const auto event = gangyi::agentLegacyEvent(db, access, body);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, access, event).dum…。
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/start")([&config](const crow::request& req) {。
    CROW_ROUTE(app, "/api/classroom/start")([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, gangyi::agentQuestionView(db, localAgentAc…。
            return crow::response(200, gangyi::agentQuestionView(db, localAgentAccess(db), questionRequestBody(req)).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/submit").methods(crow::HTTPMethod::POST)([&config](c…。
    CROW_ROUTE(app, "/api/classroom/submit").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);

            // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
            const auto access = localAgentAccess(db); const auto event = gangyi::agentLegacyEvent(db, access, body);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, access, event).dum…。
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/skip").methods(crow::HTTPMethod::POST)([&config](con…。
    CROW_ROUTE(app, "/api/classroom/skip").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 body["type"]，更新当前运行状态。
            body["type"] = "feedback"; body["text"] = "我跳过当前课堂，请据此调整下一步。";
            // 【逐行批注】 计算右侧结果并写入 std::atomic_uint64_t sequence{0}; body["requ…，更新当前运行状态。
            static std::atomic_uint64_t sequence{0}; body["requestId"] = body.value("requestId", "skip-lesson-" + nowIso8601() + "-" + std::to_string(++sequence));
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(d…。
            return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(db), body).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/activity/skip").methods(crow::HTTPMethod::POST)([&co…。
    CROW_ROUTE(app, "/api/classroom/activity/skip").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 body["action"]，更新当前运行状态。
            body["action"] = "skip"; if (!body.contains("question") && !body.contains("answer")) body["question"] = "我跳过这道题";
            // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
            const auto access = localAgentAccess(db); const auto event = gangyi::agentLegacyEvent(db, access, body);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, access, event).dum…。
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/state")([&config](const crow::request& req) {。
    CROW_ROUTE(app, "/api/classroom/state")([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); const auto access = localAgentAccess(db); const auto scope = questionRequestBody(req);
            // 【逐行批注】 计算右侧结果并写入 auto resolved，更新当前运行状态。
            const auto resolved = scope.contains("lessonId") ? nlohmann::json{{"lessonId", scope.at("lessonId")}} : gangyi::agentLegacyLesson(db, access, scope);
            // 【逐行批注】 判断条件 (resolved.empty()) throw std::invalid_argument("课堂尚未准备好，请先让 AI 备…，成立时执行对应分支。
            if (resolved.empty()) throw std::invalid_argument("课堂尚未准备好，请先让 AI 备课");
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, gangyi::agentLessonView(db, access, resolv…。
            return crow::response(200, gangyi::agentLessonView(db, access, resolved.at("lessonId")).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/hint").methods(crow::HTTPMethod::POST)([&config](con…。
    CROW_ROUTE(app, "/api/classroom/hint").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 body["action"]，更新当前运行状态。
            body["action"] = "hint"; if (!body.contains("question") && !body.contains("answer")) body["question"] = "请给我一点提示，不要直接展示答案。";
            // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
            const auto access = localAgentAccess(db); const auto event = gangyi::agentLegacyEvent(db, access, body);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, access, event).dum…。
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/remedial")([&config](const crow::request& req) {。
    CROW_ROUTE(app, "/api/classroom/remedial")([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); const auto access = localAgentAccess(db); const auto scope = questionRequestBody(req);
            // 【逐行批注】 计算右侧结果并写入 auto resolved，更新当前运行状态。
            const auto resolved = scope.contains("lessonId") ? nlohmann::json{{"lessonId", scope.at("lessonId")}} : gangyi::agentLegacyLesson(db, access, scope);
            // 【逐行批注】 判断条件 (resolved.empty()) throw std::invalid_argument("课堂尚未准备好，请先让 AI 备…，成立时执行对应分支。
            if (resolved.empty()) throw std::invalid_argument("课堂尚未准备好，请先让 AI 备课");
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, gangyi::agentLessonView(db, access, resolv…。
            return crow::response(200, gangyi::agentLessonView(db, access, resolved.at("lessonId")).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/review/submit").methods(crow::HTTPMethod::POST)([&co…。
    CROW_ROUTE(app, "/api/classroom/review/submit").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 body["kind"]，更新当前运行状态。
            body["kind"] = "review";
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, submitClassroomBatch(db, localAgentAccess(…。
            return crow::response(202, submitClassroomBatch(db, localAgentAccess(db), body).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/reviews")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/api/classroom/reviews")([&db](const crow::request& req) {
        // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
        const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
        // 【逐行批注】 判断条件 (!requesterCanAccessCourse(db, req, courseId, "")) return crow::…，成立时执行对应分支。
        if (!requesterCanAccessCourse(db, req, courseId, "")) return crow::response(404, "{}");
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { return crow::response(200, nlohmann::json{{"items", gangyi::dueReviews(db, courseId, todayDate())}}.dump()); }
        // 【逐行批注】 捕获并处理异常：catch (...) { return crow::response(503, "{}"); }。
        catch (...) { return crow::response(503, "{}"); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/week")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/api/classroom/week")([&db](const crow::request& req) {
        // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
        const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
        // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
        const auto found = gangyi::getCourseWithSnapshot(db, courseId);
        // 【逐行批注】 判断条件 (!found || !requesterCanReadCourse(db, found->course, req)) retu…，成立时执行对应分支。
        if (!found || !requesterCanReadCourse(db, found->course, req)) return crow::response(404, "{}");
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 plan，更新当前运行状态。
            auto plan = gangyi::studyPlanView(db); auto entries = nlohmann::json::array();
            // 【逐行批注】 按循环规则重复处理：for (const auto& entry : plan["entries"]) if (entry.value("courseId", "") …。
            for (const auto& entry : plan["entries"]) if (entry.value("courseId", "") == courseId) entries.push_back(entry);
            // 【逐行批注】 计算右侧结果并写入 plan["entries"]，更新当前运行状态。
            plan["entries"] = entries; plan["sharedBudget"] = true;
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, plan.dump());。
            return crow::response(200, plan.dump());
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 捕获并处理异常：catch (...) { return crow::response(503, nlohmann::json{{"error", "周计…。
        catch (...) { return crow::response(503, nlohmann::json{{"error", "周计划读取失败"}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/week/edit").methods(crow::HTTPMethod::POST)([&db](co…。
    CROW_ROUTE(app, "/api/classroom/week/edit").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 body，更新当前运行状态。
            auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
            const std::string courseId = body.at("courseId").get<std::string>();
            // 【逐行批注】 判断条件 (!requesterCanAccessCourse(db, req, courseId, "")) return crow::…，成立时执行对应分支。
            if (!requesterCanAccessCourse(db, req, courseId, "")) return crow::response(404, "{}");
            // 【逐行批注】 计算右侧结果并写入 all，更新当前运行状态。
            auto all = gangyi::studyPlanView(db);
            // 【逐行批注】 判断条件 (body.contains("entries")) {，成立时执行对应分支。
            if (body.contains("entries")) {
                // 【逐行批注】 计算右侧结果并写入 merged，更新当前运行状态。
                auto merged = nlohmann::json::array();
                // 【逐行批注】 按循环规则重复处理：for (const auto& entry : all["entries"]) if (entry.value("courseId", "") !…。
                for (const auto& entry : all["entries"]) if (entry.value("courseId", "") != courseId) merged.push_back(entry);
                // 【逐行批注】 按循环规则重复处理：for (auto entry : body["entries"]) {。
                for (auto entry : body["entries"]) {
                    // 【逐行批注】 计算右侧结果并写入 entry["courseId"]，更新当前运行状态。
                    entry["courseId"] = courseId;
                    // 【逐行批注】 判断条件 (!entry.contains("taskId")) entry["taskId"] = "lesson:" + course…，成立时执行对应分支。
                    if (!entry.contains("taskId")) entry["taskId"] = "lesson:" + courseId + ":" +
                        // 【逐行批注】 调用 to_string 完成本行所需操作：std::to_string(entry.value("phaseIndex", 1)) + ":" + std::to_string(e…。
                        std::to_string(entry.value("phaseIndex", 1)) + ":" + std::to_string(entry.value("topicIndex", 1));
                    // 【逐行批注】 执行当前声明或表达式：merged.push_back(entry);。
                    merged.push_back(entry);
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 body["entries"]，更新当前运行状态。
                body["entries"] = merged;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, gangyi::editStudyPlan(db, body).dump());。
            return crow::response(200, gangyi::editStudyPlan(db, body).dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::invalid_argument& error) {。
        } catch (const std::invalid_argument& error) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(409, nlohmann::json{{"error", error.what()}}.du…。
            return crow::response(409, nlohmann::json{{"error", error.what()}}.dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (...) { return crow::response(400, nlohmann::json{{"error", "…。
        } catch (...) { return crow::response(400, nlohmann::json{{"error", "周计划保存失败"}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/week/replan").methods(crow::HTTPMethod::POST)([&db](…。
    CROW_ROUTE(app, "/api/classroom/week/replan").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 body，更新当前运行状态。
            auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
            const std::string courseId = body.at("courseId").get<std::string>();
            // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
            const auto found = gangyi::getCourseWithSnapshot(db, courseId);
            // 【逐行批注】 判断条件 (!found || !requesterCanReadCourse(db, found->course, req)) retu…，成立时执行对应分支。
            if (!found || !requesterCanReadCourse(db, found->course, req)) return crow::response(404, "{}");
            // 【逐行批注】 判断条件 (!body.contains("version")) body["version"] = gangyi::studyPlanV…，成立时执行对应分支。
            if (!body.contains("version")) body["version"] = gangyi::studyPlanView(db)["version"];
            // 【逐行批注】 判断条件 (body.contains("previewId")) body["proposalId"] = body["previewI…，成立时执行对应分支。
            if (body.contains("previewId")) body["proposalId"] = body["previewId"];
            // 【逐行批注】 判断条件 (body.contains("entries")) {，成立时执行对应分支。
            if (body.contains("entries")) {
                // 【逐行批注】 计算右侧结果并写入 all，更新当前运行状态。
                auto all = gangyi::studyPlanView(db), merged = nlohmann::json::array();
                // 【逐行批注】 按循环规则重复处理：for (const auto& entry : all["entries"]) if (entry.value("courseId", "") !…。
                for (const auto& entry : all["entries"]) if (entry.value("courseId", "") != courseId) merged.push_back(entry);
                // 【逐行批注】 按循环规则重复处理：for (auto entry : body["entries"]) {。
                for (auto entry : body["entries"]) {
                    // 【逐行批注】 计算右侧结果并写入 entry["courseId"]，更新当前运行状态。
                    entry["courseId"] = courseId;
                    // 【逐行批注】 判断条件 (!entry.contains("taskId")) entry["taskId"] = "lesson:" + course…，成立时执行对应分支。
                    if (!entry.contains("taskId")) entry["taskId"] = "lesson:" + courseId + ":" +
                        // 【逐行批注】 调用 to_string 完成本行所需操作：std::to_string(entry.value("phaseIndex", 1)) + ":" + std::to_string(e…。
                        std::to_string(entry.value("phaseIndex", 1)) + ":" + std::to_string(entry.value("topicIndex", 1));
                    // 【逐行批注】 执行当前声明或表达式：merged.push_back(entry);。
                    merged.push_back(entry);
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 body["entries"]，更新当前运行状态。
                body["entries"] = merged;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, studyPlanAgentAction(db, body, body.value(…。
            return crow::response(200, studyPlanAgentAction(db, body, body.value("confirm", false) ? "confirm" : "replan").dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::invalid_argument& error) { return crow::response(…。
        } catch (const std::invalid_argument& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
        // 【逐行批注】 捕获并处理异常：catch (...) { return crow::response(400, nlohmann::json{{"error", "重排…。
        catch (...) { return crow::response(400, nlohmann::json{{"error", "重排失败"}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/finish").methods(crow::HTTPMethod::POST)([&config](c…。
    CROW_ROUTE(app, "/api/classroom/finish").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
            const auto access = localAgentAccess(db); const auto saved = body.contains("lessonId") ? nlohmann::json{{"lessonId", body.at("lessonId")}} : gangyi::agentLegacyLesson(db, access, body);
            // 【逐行批注】 判断条件 (saved.empty()) throw std::invalid_argument("课堂尚未准备好");，成立时执行对应分支。
            if (saved.empty()) throw std::invalid_argument("课堂尚未准备好");
            // 【逐行批注】 执行当前声明或表达式：static std::atomic_uint64_t sequence{0};。
            static std::atomic_uint64_t sequence{0};
            // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
            const auto event = nlohmann::json{{"type", "lesson_finish_request"}, {"courseId", body.at("courseId")}, {"lessonId", saved.at("lessonId")},
                // 【逐行批注】 调用 nowIso8601、to_string 完成本行所需操作：{"requestId", body.value("requestId", "finish-" + nowIso8601() + "-" …。
                {"requestId", body.value("requestId", "finish-" + nowIso8601() + "-" + std::to_string(++sequence))},
                // 【逐行批注】 执行当前声明或表达式：{"text", "我想结束本课。请根据我的实际记录判断是否完成、需要怎样复习或补充，不使用固定通过率。"}};。
                {"text", "我想结束本课。请根据我的实际记录判断是否完成、需要怎样复习或补充，不使用固定通过率。"}};
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, access, event).dum…。
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/recent-courses")([&db] {。
    CROW_ROUTE(app, "/api/recent-courses")([&db] {
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(200, nlohmann::json{{"courses", gangyi::recentCourses(db)}}.dump());
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
        response.set_header("Content-Type", "application/json; charset=utf-8");
        // 【逐行批注】 执行当前声明或表达式：response.set_header("Cache-Control", "no-store");。
        response.set_header("Cache-Control", "no-store");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/profile")([&config] {。
    CROW_ROUTE(app, "/api/profile")([&config] {
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(config.database_path);。
        gangyi::Database db; db.open(config.database_path);
        // 【逐行批注】 计算右侧结果并写入 view，更新当前运行状态。
        auto view = gangyi::profileView(db);
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = localAgentView(db);
        // 【逐行批注】 计算右侧结果并写入 auto state，更新当前运行状态。
        const auto state = task.value("status", "idle");
        // 【逐行批注】 计算右侧结果并写入 bool active，更新当前运行状态。
        const bool active = (state == "pending" || state == "running") && !task.value("paused", false);
        // 【逐行批注】 计算右侧结果并写入 auto error，更新当前运行状态。
        const auto error = state == "failed" ? task.value("error", "AI 已暂停，可保留结果后重试。") : std::string();
        // 【逐行批注】 计算右侧结果并写入 view["updating"]，更新当前运行状态。
        view["updating"] = active; view["error"] = error; view["controllerStatus"] = state;
        // 【逐行批注】 计算右侧结果并写入 view["abilityStatus"]["updating"]，更新当前运行状态。
        view["abilityStatus"]["updating"] = active; view["abilityStatus"]["error"] = error;
        // 【逐行批注】 计算右侧结果并写入 view["abilityStatus"]["status"]，更新当前运行状态。
        view["abilityStatus"]["status"] = active ? "updating" : error.empty() ? "ready" : "waiting";
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(200, view.dump());
        // 【逐行批注】 执行当前声明或表达式：response.set_header("Cache-Control", "no-store");。
        response.set_header("Cache-Control", "no-store");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/dialogue")([&config](const crow::request& req) {。
    CROW_ROUTE(app, "/api/classroom/dialogue")([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto scope = questionRequestBody(req);
            // 【逐行批注】 计算右侧结果并写入 auto view，更新当前运行状态。
            const auto view = gangyi::agentQuestionView(db, localAgentAccess(db), scope);
            // 【逐行批注】 按循环规则重复处理：for (const auto& question : view.at("questions")) if (question.value("inde…。
            for (const auto& question : view.at("questions")) if (question.value("index", -1) == scope.value("index", 0))
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"question", question}, {"t…。
                return crow::response(200, nlohmann::json{{"question", question}, {"turns", question.at("dialog")}, {"lessonId", view.at("lessonId")},
                    // 【逐行批注】 执行当前声明或表达式：{"taskId", question.value("taskId", "")}, {"version", question.at("sectionVersion")}…。
                    {"taskId", question.value("taskId", "")}, {"version", question.at("sectionVersion")}}.dump());
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("本题不存在");。
            throw std::invalid_argument("本题不存在");
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/evaluation/retry").methods(crow::HTTPMethod::POST)([…。
    CROW_ROUTE(app, "/api/classroom/evaluation/retry").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentControl(db, localAgentAccess(…。
            return crow::response(202, gangyi::agentControl(db, localAgentAccess(db), {{"command", "retry"}, {"taskId", body.at("taskId")}}).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learn/exposure").methods(crow::HTTPMethod::POST)([&config](con…。
    CROW_ROUTE(app, "/api/learn/exposure").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, gangyi::agentExposeTask(db, localAgentAcce…。
            return crow::response(200, gangyi::agentExposeTask(db, localAgentAccess(db), body).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/preparation")([&config](const crow::request&) {。
    CROW_ROUTE(app, "/api/classroom/preparation")([&config](const crow::request&) {
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(config.database_path);。
        gangyi::Database db; db.open(config.database_path);
        // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, localAgentView(db).dump());。
        return crow::response(200, localAgentView(db).dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/classroom/preparation/retry").methods(crow::HTTPMethod::POST)(…。
    CROW_ROUTE(app, "/api/classroom/preparation/retry").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body.empty() ? "{}" : req.body);
            // 【逐行批注】 计算右侧结果并写入 body["command"]，更新当前运行状态。
            body["command"] = "retry"; return crow::response(202, gangyi::agentControl(db, localAgentAccess(db), body).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/study-plan")([&db](const crow::request&) {。
    CROW_ROUTE(app, "/api/study-plan")([&db](const crow::request&) {
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(200, gangyi::studyPlanView(db).dump()); response.set_header("Cache-Control", "no-store"); return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/study-plan").methods(crow::HTTPMethod::PUT)([&db](const crow::…。
    CROW_ROUTE(app, "/api/study-plan").methods(crow::HTTPMethod::PUT)([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { const auto body = nlohmann::json::parse(req.body); auto result = gangyi::editStudyPlan(db, body);
            // 【逐行批注】 计算右侧结果并写入 std::atomic_uint64_t sequence{0}; result["ta…，更新当前运行状态。
            static std::atomic_uint64_t sequence{0}; result["task"] = gangyi::agentSubmit(db, localAgentAccess(db), {{"type", "schedule_changed"}, {"requestId", body.value("requestId", "time-" + nowIso8601() + "-" + std::to_string(++sequence))}, {"text", "我已保存新的时间和手动安排，请结合最新设置决定后续教学。"}}); return crow::response(200, result.dump()); }
        // 【逐行批注】 捕获并处理异常：catch (const std::exception& error) { return crow::response(409, nloh…。
        catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/study-plan/draft").methods(crow::HTTPMethod::POST)([&db](const…。
    CROW_ROUTE(app, "/api/study-plan/draft").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { return crow::response(200, gangyi::studyPlanDraft(db, nlohmann::json::parse(req.body)).dump()); }
        // 【逐行批注】 捕获并处理异常：catch (...) { return crow::response(409, nlohmann::json{{"error", "编辑…。
        catch (...) { return crow::response(409, nlohmann::json{{"error", "编辑状态无效，请刷新"}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/study-plan/<string>").methods(crow::HTTPMethod::POST)([&db](co…。
    CROW_ROUTE(app, "/api/study-plan/<string>").methods(crow::HTTPMethod::POST)([&db](const crow::request& req, const std::string& action) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { return crow::response(200, studyPlanAgentAction(db, nlohmann::json::parse(req.body), action).dump()); }
        // 【逐行批注】 捕获并处理异常：catch (const std::exception& error) { return crow::response(409, nloh…。
        catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/home/next-step")([&config](const crow::request&) {。
    CROW_ROUTE(app, "/api/home/next-step")([&config](const crow::request&) {
        // 【逐行批注】 计算右侧结果并写入 gangyi::Database db; db.open(config.database…，更新当前运行状态。
        gangyi::Database db; db.open(config.database_path); auto task = localAgentView(db);
        // 【逐行批注】 判断条件 (task.value("status", "") == "ready" && task.contains("lesson"))…，成立时执行对应分支。
        if (task.value("status", "") == "ready" && task.contains("lesson")) task["next"] = task.at("lesson");
        // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, task.dump());。
        return crow::response(200, task.dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/courses/<string>/preview")([&config](const crow::request&, con…。
    CROW_ROUTE(app, "/api/courses/<string>/preview")([&config](const crow::request&, const std::string& course) {
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(config.database_path);。
        gangyi::Database db; db.open(config.database_path);
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
            auto value = gangyi::coursePreviewView(db, course);
            // 【逐行批注】 判断条件 (value.value("status", "") == "pending") {，成立时执行对应分支。
            if (value.value("status", "") == "pending") {
                // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
                const auto task = localAgentView(db); bool preparing = false;
                // 【逐行批注】 判断条件 (task.value("status", "") == "pending" || task.value("status", "…，成立时执行对应分支。
                if (task.value("status", "") == "pending" || task.value("status", "") == "running") {
                    // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
                    const auto row = db.getClassroomActivity(task.at("id"));
                    // 【逐行批注】 判断条件 (row && row->courseId == course) {，成立时执行对应分支。
                    if (row && row->courseId == course) {
                        // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
                        const auto saved = nlohmann::json::parse(row->payload);
                        // 【逐行批注】 计算右侧结果并写入 preparing，更新当前运行状态。
                        preparing = saved.at("event").value("type", "") == "course_preview";
                    // 【逐行批注】 结束当前代码块。
                    }
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 判断条件 (!preparing) { value["status"] = "waiting"; value["message"] = "…，成立时执行对应分支。
                if (!preparing) { value["status"] = "waiting"; value["message"] = "路线说明等待 AI 更新，可按最新情况重新准备。"; }
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(200, value.dump()); response.set_header("Cache-Control", "no-store"); return response;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 捕获并处理异常：catch (...) { return crow::response(404, nlohmann::json{{"error", "课程…。
        catch (...) { return crow::response(404, nlohmann::json{{"error", "课程不存在"}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/courses/<string>/preview").methods(crow::HTTPMethod::POST)([&d…。
    CROW_ROUTE(app, "/api/courses/<string>/preview").methods(crow::HTTPMethod::POST)([&db](const crow::request& req, const std::string& course) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { const auto body = nlohmann::json::parse(req.body); static std::atomic_uint64_t sequence{0}; const auto task = gangyi::agentSubmit(db, localAgentAccess(db), {{"type", "course_preview"}, {"courseId", course}, {"requestId", body.value("requestId", "preview-" + nowIso8601() + "-" + std::to_string(++sequence))}, {"text", "请读取同一份最新大纲，调用 update_course_preview 保存你选择的课程路线说明。"}}); return crow::response(202, task.dump()); }
        // 【逐行批注】 捕获并处理异常：catch (...) { return crow::response(409, nlohmann::json{{"error", "预览…。
        catch (...) { return crow::response(409, nlohmann::json{{"error", "预览状态变化，请刷新"}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/profile/radar-preferences")([&db] {。
    CROW_ROUTE(app, "/api/profile/radar-preferences")([&db] {
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(200, gangyi::radarPreferences(db).dump());
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
        response.set_header("Content-Type", "application/json; charset=utf-8");
        // 【逐行批注】 执行当前声明或表达式：response.set_header("Cache-Control", "no-store");。
        response.set_header("Cache-Control", "no-store");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/profile/radar-preferences").methods(crow::HTTPMethod::PUT)([&d…。
    CROW_ROUTE(app, "/api/profile/radar-preferences").methods(crow::HTTPMethod::PUT)([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { return crow::response(200, gangyi::saveRadarPreferences(db, nlohmann::json::parse(req.body)).dump()); }
        // 【逐行批注】 捕获并处理异常：catch (...) { return crow::response(400, nlohmann::json{{"error", "请选…。
        catch (...) { return crow::response(400, nlohmann::json{{"error", "请选择九科中的六门不同学科，并指定有效的显示模式。"}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/next-learning")([&config](const crow::request&) {。
    CROW_ROUTE(app, "/api/next-learning")([&config](const crow::request&) {
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(config.database_path);。
        gangyi::Database db; db.open(config.database_path);
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = localAgentView(db);
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json result，更新当前运行状态。
        nlohmann::json result = {{"ok", true}, {"action", "waiting"}, {"reason", task.value("message", "等待 AI 根据实际表现准备下一步。")}};
        // 【逐行批注】 判断条件 (task.contains("lesson")) { result["action"] = "prepared"; resul…，成立时执行对应分支。
        if (task.contains("lesson")) { result["action"] = "prepared"; result["href"] = task.at("lesson").at("href"); result["lesson"] = task.at("lesson"); }
        // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, result.dump());。
        return crow::response(200, result.dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/topic-mastery")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/api/topic-mastery")([&db](const crow::request& req) {
        // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
        const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
        // 【逐行批注】 判断条件 (courseId.empty() || !requesterCanAccessCourse(db, req, courseId…，成立时执行对应分支。
        if (courseId.empty() || !requesterCanAccessCourse(db, req, courseId, requestAnonymousId(req)))
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课…。
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在"}}.dump());
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json topics，更新当前运行状态。
        nlohmann::json topics = nlohmann::json::array();
        // 【逐行批注】 按循环规则重复处理：for (const auto& value : db.listTopicMastery()) if (value.courseId == cour…。
        for (const auto& value : db.listTopicMastery()) if (value.courseId == courseId)
            // 【逐行批注】 执行当前声明或表达式：topics.push_back({{"phaseIndex", value.phaseIndex}, {"topic", value.topic},。
            topics.push_back({{"phaseIndex", value.phaseIndex}, {"topic", value.topic},
                // 【逐行批注】 调用 json 完成本行所需操作：{"score", value.score ? nlohmann::json(*value.score) : nlohmann::json…。
                {"score", value.score ? nlohmann::json(*value.score) : nlohmann::json(nullptr)},
                // 【逐行批注】 执行当前声明或表达式：{"status", value.status}, {"rationale", value.rationale},。
                {"status", value.status}, {"rationale", value.rationale},
                // 【逐行批注】 调用 parse 完成本行所需操作：{"weakPoints", nlohmann::json::parse(value.weakPoints, nullptr, false…。
                {"weakPoints", nlohmann::json::parse(value.weakPoints, nullptr, false)},
                // 【逐行批注】 执行当前声明或表达式：{"recommendation", value.recommendation}, {"evidenceCount", value.evidenceCount},。
                {"recommendation", value.recommendation}, {"evidenceCount", value.evidenceCount},
                // 【逐行批注】 调用 parse 完成本行所需操作：{"evidenceIds", nlohmann::json::parse(value.evidenceIds, nullptr, fal…。
                {"evidenceIds", nlohmann::json::parse(value.evidenceIds, nullptr, false)},
                // 【逐行批注】 执行当前声明或表达式：{"nextReviewAt", value.nextReviewAt}, {"version", value.version}, {"updatedAt", valu…。
                {"nextReviewAt", value.nextReviewAt}, {"version", value.version}, {"updatedAt", value.updatedAt}});
        // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}, {"topics", to…。
        return crow::response(200, nlohmann::json{{"ok", true}, {"topics", topics}}.dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/home/recommendations")([databasePath = config.database_path](c…。
    CROW_ROUTE(app, "/api/home/recommendations")([databasePath = config.database_path](const crow::request&) {
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
        gangyi::Database connection; connection.open(databasePath);
        // 【逐行批注】 计算右侧结果并写入 current，更新当前运行状态。
        auto current = gangyi::agentRecommendationView(connection, localAgentAccess(connection));
        // 【逐行批注】 判断条件 (current.empty()) current = nlohmann::json{{"status", "pending"}…，成立时执行对应分支。
        if (current.empty()) current = nlohmann::json{{"status", "pending"}, {"items", {{"lite", nlohmann::json::array()}, {"deep", nlohmann::json::array()}}}, {"message", "等待真实 AI 完成启动推荐"}};
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(current.dump());
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
        response.set_header("Content-Type", "application/json; charset=utf-8");
        // 【逐行批注】 执行当前声明或表达式：response.set_header("Cache-Control", "no-store");。
        response.set_header("Cache-Control", "no-store");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/ai-activity")([databasePath = config.database_path] {。
    CROW_ROUTE(app, "/api/ai-activity")([databasePath = config.database_path] {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(gangyi::agentActivityView(connection, localAgentAccess(connection)).dump());
            // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
            response.set_header("Content-Type", "application/json; charset=utf-8");
            // 【逐行批注】 执行当前声明或表达式：response.set_header("Cache-Control", "no-store"); return response;。
            response.set_header("Cache-Control", "no-store"); return response;
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/ai-activity/control").methods(crow::HTTPMethod::POST)([databas…。
    CROW_ROUTE(app, "/api/ai-activity/control").methods(crow::HTTPMethod::POST)([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 auto body，更新当前运行状态。
            const auto body = nlohmann::json::parse(req.body); const auto command = body.value("command", "");
            // 【逐行批注】 判断条件 (command != "pause" && command != "resume") throw std::invalid_a…，成立时执行对应分支。
            if (command != "pause" && command != "resume") throw std::invalid_argument("全软件 AI 控制指令无效");
            // 【逐行批注】 判断条件 (command == "pause") gangyi::AIActivity::setPaused(true);，成立时执行对应分支。
            if (command == "pause") gangyi::AIActivity::setPaused(true);
            // 【逐行批注】 计算右侧结果并写入 gangyi::Database connection; connection.open…，更新当前运行状态。
            gangyi::Database connection; connection.open(databasePath); const auto access = localAgentAccess(connection);
            // 【逐行批注】 判断条件 (command == "resume") {，成立时执行对应分支。
            if (command == "resume") {
                // 【逐行批注】 调用 agentResumeGlobalTasks 完成本行所需操作：gangyi::agentResumeGlobalTasks(connection, access);。
                gangyi::agentResumeGlobalTasks(connection, access);
                // 兼容升级前用户暂停的当前任务；历史已结束任务不重新发起。
                // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
                const auto current = gangyi::agentView(connection, access);
                // 【逐行批注】 判断条件 (current.contains("id") && (current.value("scopePaused", false) …，成立时执行对应分支。
                if (current.contains("id") && (current.value("scopePaused", false) || current.value("status", "") == "paused") &&
                    // 【逐行批注】 执行当前声明或表达式：(current.value("status", "") == "paused" || current.value("status", "") == "pending"…。
                    (current.value("status", "") == "paused" || current.value("status", "") == "pending" || current.value("status", "") == "running"))
                    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(connection, access, {{"command", "resume"}, {"ta…。
                    gangyi::agentControl(connection, access, {{"command", "resume"}, {"taskId", current.at("id")}});
                // 【逐行批注】 调用 setPaused 完成本行所需操作：gangyi::AIActivity::setPaused(false);。
                gangyi::AIActivity::setPaused(false);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(gangyi::agentActivityView(connection, access).dump());
            // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
            response.set_header("Content-Type", "application/json; charset=utf-8"); return response;
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learning-agent/events").methods(crow::HTTPMethod::POST)([datab…。
    CROW_ROUTE(app, "/api/learning-agent/events").methods(crow::HTTPMethod::POST)([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
            const auto value = gangyi::agentSubmit(connection, localAgentAccess(connection), nlohmann::json::parse(req.body));
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(202, value.dump()); response.set_header("Content-Type", "application/json; charset=utf-8");
            // 【逐行批注】 执行当前声明或表达式：response.set_header("Cache-Control", "no-store"); return response;。
            response.set_header("Cache-Control", "no-store"); return response;
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learning-agent")([databasePath = config.database_path](const c…。
    CROW_ROUTE(app, "/api/learning-agent")([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
            auto value = localAgentView(connection, req.url_params.get("taskId") ? req.url_params.get("taskId") : "");
            // 【逐行批注】 计算右侧结果并写入 value["adjustments"]，更新当前运行状态。
            value["adjustments"] = value.value("changeHistory", nlohmann::json::array());
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(value.dump()); response.set_header("Content-Type", "application/json; charset=utf-8");
            // 【逐行批注】 执行当前声明或表达式：response.set_header("Cache-Control", "no-store"); return response;。
            response.set_header("Cache-Control", "no-store"); return response;
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learning-agent/control").methods(crow::HTTPMethod::POST)([data…。
    CROW_ROUTE(app, "/api/learning-agent/control").methods(crow::HTTPMethod::POST)([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(gangyi::agentControl(connection, localAgentAccess(connection), nlohmann::json::parse(req.body)).dump());
            // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
            response.set_header("Content-Type", "application/json; charset=utf-8"); return response;
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/independent-chat")([databasePath = config.database_path](const…。
    CROW_ROUTE(app, "/api/independent-chat")([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 计算右侧结果并写入 auto lessonId，更新当前运行状态。
            const auto lessonId = req.url_params.get("lessonId") ? req.url_params.get("lessonId") : "";
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, gangyi::independentChatView(。
            return crow::response(200, gangyi::independentChatView(
                // 【逐行批注】 调用 localAgentAccess 完成本行所需操作：connection, localAgentAccess(connection), lessonId).dump());。
                connection, localAgentAccess(connection), lessonId).dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::invalid_argument& error) {。
        } catch (const std::invalid_argument& error) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"error", error.what()}}.du…。
            return crow::response(400, nlohmann::json{{"error", error.what()}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception& error) {。
        } catch (const std::exception& error) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(409, nlohmann::json{{"error", error.what()}}.du…。
            return crow::response(409, nlohmann::json{{"error", error.what()}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/independent-chat/settings").methods(crow::HTTPMethod::POST)([d…。
    CROW_ROUTE(app, "/api/independent-chat/settings").methods(crow::HTTPMethod::POST)([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, gangyi::independentChatSettings(。
            return crow::response(200, gangyi::independentChatSettings(
                // 【逐行批注】 调用 parse 完成本行所需操作：connection, nlohmann::json::parse(req.body)).dump());。
                connection, nlohmann::json::parse(req.body)).dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::invalid_argument& error) {。
        } catch (const std::invalid_argument& error) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"error", error.what()}}.du…。
            return crow::response(400, nlohmann::json{{"error", error.what()}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception& error) {。
        } catch (const std::exception& error) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(409, nlohmann::json{{"error", error.what()}}.du…。
            return crow::response(409, nlohmann::json{{"error", error.what()}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/independent-chat/send").methods(crow::HTTPMethod::POST)([datab…。
    CROW_ROUTE(app, "/api/independent-chat/send").methods(crow::HTTPMethod::POST)([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::independentChatSend(。
            return crow::response(202, gangyi::independentChatSend(
                // 【逐行批注】 调用 localAgentAccess、parse 完成本行所需操作：connection, localAgentAccess(connection), nlohmann::json::parse(req.b…。
                connection, localAgentAccess(connection), nlohmann::json::parse(req.body)).dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::invalid_argument& error) {。
        } catch (const std::invalid_argument& error) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"error", error.what()}}.du…。
            return crow::response(400, nlohmann::json{{"error", error.what()}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception& error) {。
        } catch (const std::exception& error) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(409, nlohmann::json{{"error", error.what()}}.du…。
            return crow::response(409, nlohmann::json{{"error", error.what()}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/independent-chat/clear").methods(crow::HTTPMethod::POST)([data…。
    CROW_ROUTE(app, "/api/independent-chat/clear").methods(crow::HTTPMethod::POST)([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, gangyi::independentChatClear(。
            return crow::response(200, gangyi::independentChatClear(
                // 【逐行批注】 调用 localAgentAccess、parse 完成本行所需操作：connection, localAgentAccess(connection), nlohmann::json::parse(req.b…。
                connection, localAgentAccess(connection), nlohmann::json::parse(req.body)).dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::invalid_argument& error) {。
        } catch (const std::invalid_argument& error) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"error", error.what()}}.du…。
            return crow::response(400, nlohmann::json{{"error", error.what()}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception& error) {。
        } catch (const std::exception& error) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(409, nlohmann::json{{"error", error.what()}}.du…。
            return crow::response(409, nlohmann::json{{"error", error.what()}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/deep-course")([] {。
    CROW_ROUTE(app, "/deep-course")([] {
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(gangyi::renderDeepCoursePage());
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/ht…，更新当前运行状态。
        response.set_header("Content-Type", "text/html; charset=utf-8"); return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/deep-course")([databasePath = config.database_path](const crow…。
    CROW_ROUTE(app, "/api/deep-course")([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 计算右侧结果并写入 nlohmann::json scope，更新当前运行状态。
            nlohmann::json scope = nlohmann::json::object();
            // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"courseId", "lessonId", "topicId", "phaseIndex", "…。
            for (const auto* key : {"courseId", "lessonId", "topicId", "phaseIndex", "topicIndex"})
                // 【逐行批注】 判断条件 (const char* value = req.url_params.get(key)) scope[key] = value;，成立时执行对应分支。
                if (const char* value = req.url_params.get(key)) scope[key] = value;
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(gangyi::deepCourseView(connection, localAgentAccess(connection), scope).dump());
            // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
            response.set_header("Content-Type", "application/json; charset=utf-8"); response.set_header("Cache-Control", "no-store"); return response;
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/deep-course/start").methods(crow::HTTPMethod::POST)([databaseP…。
    CROW_ROUTE(app, "/api/deep-course/start").methods(crow::HTTPMethod::POST)([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(202, gangyi::deepCourseStart(connection, localAgentAccess(connection), nlohmann::json::parse(req.body)).dump());
            // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
            response.set_header("Content-Type", "application/json; charset=utf-8"); return response;
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/deep-course/retry-media").methods(crow::HTTPMethod::POST)([dat…。
    CROW_ROUTE(app, "/api/deep-course/retry-media").methods(crow::HTTPMethod::POST)([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(202, gangyi::deepCourseRetryMedia(connection, localAgentAccess(connection), nlohmann::json::parse(req.body)).dump());
            // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
            response.set_header("Content-Type", "application/json; charset=utf-8"); return response;
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/deep-course/image")([databasePath = config.database_path](cons…。
    CROW_ROUTE(app, "/api/deep-course/image")([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 计算右侧结果并写入 auto image，更新当前运行状态。
            const auto image = gangyi::deepCourseImage(connection, localAgentAccess(connection), req.url_params.get("lessonId") ? req.url_params.get("lessonId") : "", req.url_params.get("mediaId") ? req.url_params.get("mediaId") : "");
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(image.second); response.set_header("Content-Type", image.first);
            // 【逐行批注】 计算右侧结果并写入 response.set_header("X-Content-Type-Options"…，更新当前运行状态。
            response.set_header("X-Content-Type-Options", "nosniff"); response.set_header("Cache-Control", "private, max-age=86400"); return response;
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception&) { return crow::response(404, "图片尚未准备完…。
        } catch (const std::exception&) { return crow::response(404, "图片尚未准备完成"); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learning-agent/lesson")([databasePath = config.database_path](…。
    CROW_ROUTE(app, "/api/learning-agent/lesson")([databasePath = config.database_path](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath);。
            gangyi::Database connection; connection.open(databasePath);
            // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
            crow::response response(gangyi::agentLessonView(connection, localAgentAccess(connection),
                // 【逐行批注】 执行当前声明或表达式：req.url_params.get("lessonId") ? req.url_params.get("lessonId") : "").dump());。
                req.url_params.get("lessonId") ? req.url_params.get("lessonId") : "").dump());
            // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
            response.set_header("Content-Type", "application/json; charset=utf-8"); return response;
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 CROW_WEBSOCKET_ROUTE 完成本行所需操作：CROW_WEBSOCKET_ROUTE(app, "/ws/learning-agent").onaccept(socketAccept…。
    CROW_WEBSOCKET_ROUTE(app, "/ws/learning-agent").onaccept(socketAccept).onopen(socketOpen).onclose(socketClose).onmessage(subscribeAgentTask);

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/startup")([] {。
    CROW_ROUTE(app, "/startup")([] {
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(gangyi::renderStartupPage());
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "text/ht…，更新当前运行状态。
        response.set_header("Content-Type", "text/html; charset=utf-8");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/profile/refresh").methods(crow::HTTPMethod::POST)([databasePat…。
    CROW_ROUTE(app, "/api/profile/refresh").methods(crow::HTTPMethod::POST)([databasePath = config.database_path] {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database connection; connection.open(databasePath); connection.markProfileDi…。
            gangyi::Database connection; connection.open(databasePath); connection.markProfileDirty();
            // 【逐行批注】 执行当前声明或表达式：static std::atomic_uint64_t sequence{0};。
            static std::atomic_uint64_t sequence{0};
            // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
            const auto task = gangyi::agentSubmit(connection, localAgentAccess(connection), {{"type", "profile_update"},
                // 【逐行批注】 调用 nowIso8601、to_string 完成本行所需操作：{"requestId", "profile-" + nowIso8601() + "-" + std::to_string(++sequ…。
                {"requestId", "profile-" + nowIso8601() + "-" + std::to_string(++sequence)},
                // 【逐行批注】 执行当前声明或表达式：{"text", "读取真实原题、实际回答、提示经历和可靠评价，判断哪些学科与能力维度已具备证据，并给出评分、范围和不确定性。不采用固定题数门槛；缺证据时解释还需要了解…。
                {"text", "读取真实原题、实际回答、提示经历和可靠评价，判断哪些学科与能力维度已具备证据，并给出评分、范围和不确定性。不采用固定题数门槛；缺证据时解释还需要了解什么，保留其他有效画像。"}});
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, nlohmann::json{{"ok", true}, {"updating", …。
            return crow::response(202, nlohmann::json{{"ok", true}, {"updating", true}, {"taskId", task.at("id")}}.dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception&) { return crow::response(503, nlohmann…。
        } catch (const std::exception&) { return crow::response(503, nlohmann::json{{"error", "画像更新未提交，原有结果保留。"}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/conversations/<string>")([&db](const crow::request&, std::stri…。
    CROW_ROUTE(app, "/api/conversations/<string>")([&db](const crow::request&, std::string conversationId) {
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json messages，更新当前运行状态。
        nlohmann::json messages = nlohmann::json::array();
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listInteractions()) {。
        for (const auto& item : db.listInteractions()) {
            // 【逐行批注】 判断条件 (item.conversationId.value_or("") != conversationId ||，成立时执行对应分支。
            if (item.conversationId.value_or("") != conversationId ||
                // 【逐行批注】 执行当前声明或表达式：(item.kind != "chat-user" && item.kind != "chat-assistant")) continue;。
                (item.kind != "chat-user" && item.kind != "chat-assistant")) continue;
            // 【逐行批注】 计算右侧结果并写入 auto payload，更新当前运行状态。
            const auto payload = nlohmann::json::parse(item.payload, nullptr, false);
            // 【逐行批注】 判断条件 (!payload.is_object()) continue;，成立时执行对应分支。
            if (!payload.is_object()) continue;
            // 【逐行批注】 执行当前声明或表达式：messages.push_back({{"role", item.kind == "chat-user" ? "user" : "assistant"},。
            messages.push_back({{"role", item.kind == "chat-user" ? "user" : "assistant"},
                // 【逐行批注】 执行当前声明或表达式：{"text", payload.value("text", "")}, {"createdAt", item.createdAt}});。
                {"text", payload.value("text", "")}, {"createdAt", item.createdAt}});
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"messages", messages}}.dum…。
        return crow::response(200, nlohmann::json{{"messages", messages}}.dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/conversations/<string>").methods(crow::HTTPMethod::DELETE)([&d…。
    CROW_ROUTE(app, "/api/conversations/<string>").methods(crow::HTTPMethod::DELETE)([&db](const crow::request&, std::string conversationId) {
        // 【逐行批注】 执行当前声明或表达式：db.deleteConversation(conversationId);。
        db.deleteConversation(conversationId);
        // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}}.dump());。
        return crow::response(200, nlohmann::json{{"ok", true}}.dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/quiz-attempts").methods(crow::HTTPMethod::POST)([&config](cons…。
    CROW_ROUTE(app, "/api/quiz-attempts").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 body["kind"]，更新当前运行状态。
            body["kind"] = "quiz";
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, submitClassroomBatch(db, localAgentAccess(…。
            return crow::response(202, submitClassroomBatch(db, localAgentAccess(db), body).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/learning-interactions").methods(crow::HTTPMethod::POST)([&db](…。
    CROW_ROUTE(app, "/api/learning-interactions").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 auto body，更新当前运行状态。
            const auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
            const std::string courseId = body.value("courseId", "");
            // 【逐行批注】 计算右侧结果并写入 std::string kind，更新当前运行状态。
            const std::string kind = body.value("kind", "");
            // 【逐行批注】 计算右侧结果并写入 int phaseIndex，更新当前运行状态。
            const int phaseIndex = body.value("phaseIndex", 0);
            // 【逐行批注】 计算右侧结果并写入 int topicIndex，更新当前运行状态。
            const int topicIndex = body.value("topicIndex", 0);
            // 【逐行批注】 判断条件 (courseId.empty() || !requesterCanAccessCourse(db, req, courseId…，成立时执行对应分支。
            if (courseId.empty() || !requesterCanAccessCourse(db, req, courseId, requestAnonymousId(req, &body)) ||
                // 【逐行批注】 执行当前声明或表达式：(kind != "practice" && kind != "review") || phaseIndex < 1 || topicIndex < 1)。
                (kind != "practice" && kind != "review") || phaseIndex < 1 || topicIndex < 1)
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"ok", false}, {"error", "学…。
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "学习记录参数无效"}}.dump());
            // 【逐行批注】 计算右侧结果并写入 auto session，更新当前运行状态。
            const auto session = db.findLearningSession(courseId, phaseIndex, topicIndex);
            // 【逐行批注】 判断条件 (!session) return crow::response(404, nlohmann::json{{"ok", fals…，成立时执行对应分支。
            if (!session) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "微课尚未生成"}}.dump());
            // 【逐行批注】 计算右侧结果并写入 std::string state，更新当前运行状态。
            const std::string state = body.value("state", std::string());
            // 【逐行批注】 判断条件 (state != "completed" && state != "reopened" && state != "too_ha…，成立时执行对应分支。
            if (state != "completed" && state != "reopened" && state != "too_hard" &&
                // 【逐行批注】 执行当前声明或表达式：state != "too_easy" && state != "unsuitable")。
                state != "too_easy" && state != "unsuitable")
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"ok", false}, {"error", "反…。
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "反馈状态无效"}}.dump());
            // 【逐行批注】 调用 recordInteraction 完成本行所需操作：recordInteraction(db, kind, {{"topic", session->topicTitle}, {"phaseI…。
            recordInteraction(db, kind, {{"topic", session->topicTitle}, {"phaseIndex", phaseIndex},
                // 【逐行批注】 执行当前声明或表达式：{"topicIndex", topicIndex}, {"state", state}}, courseId);。
                {"topicIndex", topicIndex}, {"state", state}}, courseId);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}}.dump());。
            return crow::response(200, nlohmann::json{{"ok", true}}.dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (...) { return crow::response(400, nlohmann::json{{"ok", fals…。
        } catch (...) { return crow::response(400, nlohmann::json{{"ok", false}, {"error", "学习记录保存失败"}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/analyze-image-goal").methods(crow::HTTPMethod::POST)([](const …。
    CROW_ROUTE(app, "/api/analyze-image-goal").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 判断条件 (req.get_header_value("Content-Type").find("multipart/form-data"…，成立时执行对应分支。
            if (req.get_header_value("Content-Type").find("multipart/form-data") == std::string::npos) {
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"success", false}, {"messa…。
                return crow::response(400, nlohmann::json{{"success", false}, {"message", "请上传图片文件"}}.dump());
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 声明对象 form，并通过构造函数初始化其状态。
            crow::multipart::message form(req);
            // 【逐行批注】 计算右侧结果并写入 auto image，更新当前运行状态。
            const auto image = form.get_part_by_name("image");
            // 【逐行批注】 计算右侧结果并写入 std::string prompt，更新当前运行状态。
            const std::string prompt = form.get_part_by_name("prompt").body;
            // 【逐行批注】 计算右侧结果并写入 std::string mode，更新当前运行状态。
            const std::string mode = form.get_part_by_name("mode").body == "lite" ? "lite" : "deep";
            // 【逐行批注】 计算右侧结果并写入 std::string mimeType，更新当前运行状态。
            const std::string mimeType = image.get_header_object("Content-Type").value;
            // 【逐行批注】 判断条件 (image.body.empty() || mimeType.rfind("image/", 0) != 0) {，成立时执行对应分支。
            if (image.body.empty() || mimeType.rfind("image/", 0) != 0) {
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"success", false}, {"goal"…。
                return crow::response(400, nlohmann::json{{"success", false}, {"goal", prompt}, {"mode", mode}, {"message", "请上传图片文件"}}.dump());
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (image.body.size() > 5 * 1024 * 1024) {，成立时执行对应分支。
            if (image.body.size() > 5 * 1024 * 1024) {
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(413, nlohmann::json{{"success", false}, {"goal"…。
                return crow::response(413, nlohmann::json{{"success", false}, {"goal", prompt}, {"mode", mode}, {"message", "图片过大，请上传 5MB 以内的图片"}}.dump());
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 执行当前声明或表达式：gangyi::AIClient ai;。
            gangyi::AIClient ai;
            // 【逐行批注】 声明对象 analyzer，并通过构造函数初始化其状态。
            gangyi::ImageGoalAnalyzer analyzer(ai);
            // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
            const auto result = analyzer.analyze(prompt, mode, mimeType, image.body);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"success", true}, {"goal",…。
            return crow::response(200, nlohmann::json{{"success", true}, {"goal", result.goal}, {"summary", result.summary},
                // 【逐行批注】 执行当前声明或表达式：{"keywords", result.keywords}, {"suggestedSearchQuery", result.suggestedSearchQuery}…。
                {"keywords", result.keywords}, {"suggestedSearchQuery", result.suggestedSearchQuery}, {"mode", mode}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (const gangyi::AIClientError& error) {。
        } catch (const gangyi::AIClientError& error) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"success", false}, {"messa…。
            return crow::response(200, nlohmann::json{{"success", false}, {"message", "图片识别未完成，请补充文字描述后重试"}, {"type", error.errorType}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"success", false}, {"messa…。
            return crow::response(400, nlohmann::json{{"success", false}, {"message", "图片格式无法识别，请补充文字描述后重试"}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/health")([] {。
    CROW_ROUTE(app, "/health")([] {
        // 【逐行批注】 声明对象 response，并通过构造函数初始化其状态。
        crow::response response(nlohmann::json{{"status", "healthy"}}.dump());
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
        response.set_header("Content-Type", "application/json; charset=utf-8");
        // 【逐行批注】 结束当前函数并返回结果：return response;。
        return response;
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // POST /api/generate-plan —— 每门课程首次均由 AI 生成；课程保存后由快照负责复用。
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/generate-plan").methods(crow::HTTPMethod::POST)([&config](cons…。
    CROW_ROUTE(app, "/api/generate-plan").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); auto event = nlohmann::json::parse(req.body);
            // 【逐行批注】 判断条件 (!event.value("goal", nlohmann::json()).is_string() || event.at(…，成立时执行对应分支。
            if (!event.value("goal", nlohmann::json()).is_string() || event.at("goal").get<std::string>().empty())
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"error", "请填写学习目标。"}}.dump…。
                return crow::response(400, nlohmann::json{{"error", "请填写学习目标。"}}.dump());
            // 【逐行批注】 计算右侧结果并写入 event["type"]，更新当前运行状态。
            event["type"] = "plan_course"; event["text"] = "按我的目标规划并保存课程，阶段及题量由你决定：" + event.at("goal").get<std::string>();
            // 【逐行批注】 执行当前声明或表达式：static std::atomic_uint64_t sequence{0};。
            static std::atomic_uint64_t sequence{0};
            // 【逐行批注】 判断条件 (!event.contains("requestId")) event["requestId"] = "plan-" + no…，成立时执行对应分支。
            if (!event.contains("requestId")) event["requestId"] = "plan-" + nowIso8601() + "-" + std::to_string(++sequence);
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(d…。
            return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(db), event).dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // POST /api/courses —— 保存课程 + 快照（质量门禁 + 脱敏 + upsert）
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/courses").methods(crow::HTTPMethod::POST)([&config](const crow…。
    CROW_ROUTE(app, "/api/courses").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body); const auto task = localAgentView(db, body.at("taskId"));
            // 【逐行批注】 判断条件 (task.at("status") != "ready" || !task.contains("course")) throw…，成立时执行对应分支。
            if (task.at("status") != "ready" || !task.contains("course")) throw std::invalid_argument("课程尚未由主控完整保存");
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}, {"courseId", …。
            return crow::response(200, nlohmann::json{{"ok", true}, {"courseId", task.at("course").at("id")}, {"href", task.at("course").at("href")}}.dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // GET /api/courses —— 按身份列出课程（历史课堂 / 我的课程用）
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/courses")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/api/courses")([&db](const crow::request& req) {
        // 【逐行批注】 计算右侧结果并写入 char* anonymousId，更新当前运行状态。
        const char* anonymousId = req.url_params.get("anonymousId");
        // 【逐行批注】 计算右侧结果并写入 char* limitParam，更新当前运行状态。
        const char* limitParam = req.url_params.get("limit");
        // 【逐行批注】 计算右侧结果并写入 char* offsetParam，更新当前运行状态。
        const char* offsetParam = req.url_params.get("offset");
        // 【逐行批注】 计算右侧结果并写入 size_t limit，更新当前运行状态。
        size_t limit = 50, offset = 0;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { if (limitParam) limit = static_cast<size_t>(std::stoi(limitParam)); } catch (...) {}
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { if (offsetParam) offset = static_cast<size_t>(std::stoi(offsetParam)); } catch (...) {}
        // 【逐行批注】 判断条件 (limit < 1) limit = 1;，成立时执行对应分支。
        if (limit < 1) limit = 1;
        // 【逐行批注】 判断条件 (limit > 100) limit = 100;，成立时执行对应分支。
        if (limit > 100) limit = 100;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 std::string safeAnonymousId，更新当前运行状态。
            const std::string safeAnonymousId = anonymousId ? anonymousId : "";
            // 【逐行批注】 计算右侧结果并写入 auto courses，更新当前运行状态。
            const auto courses = gangyi::listCoursesForIdentity(db, "", "", limit, offset);
            // 【逐行批注】 计算右侧结果并写入 nlohmann::json arr，更新当前运行状态。
            nlohmann::json arr = nlohmann::json::array();
            // 【逐行批注】 按循环规则重复处理：for (const auto& c : courses) {。
            for (const auto& c : courses) {
                // 【逐行批注】 计算右侧结果并写入 std::string href，更新当前运行状态。
                std::string href = "/plan?courseId=" + c.id;
                // 【逐行批注】 判断条件 (anonymousId) href += "&anonymousId=" + std::string(anonymousId);，成立时执行对应分支。
                if (anonymousId) href += "&anonymousId=" + std::string(anonymousId);
                // 【逐行批注】 执行当前声明或表达式：arr.push_back({{"id", c.id}, {"goal", c.goal}, {"mode", c.mode}, {"title", c.title},。
                arr.push_back({{"id", c.id}, {"goal", c.goal}, {"mode", c.mode}, {"title", c.title},
                    // 【逐行批注】 调用 deepCourseEnabled 完成本行所需操作：{"pedagogy", gangyi::deepCourseEnabled(db, c.id) ? "deep-v1" : "quick…。
                    {"pedagogy", gangyi::deepCourseEnabled(db, c.id) ? "deep-v1" : "quick"},
                    // 【逐行批注】 调用 json 完成本行所需操作：{"summary", c.summary ? nlohmann::json(*c.summary) : nlohmann::json(n…。
                    {"summary", c.summary ? nlohmann::json(*c.summary) : nlohmann::json(nullptr)},
                    // 【逐行批注】 执行当前声明或表达式：{"createdAt", c.createdAt}, {"updatedAt", c.updatedAt}, {"href", href}});。
                    {"createdAt", c.createdAt}, {"updatedAt", c.updatedAt}, {"href", href}});
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 nlohmann::json current，更新当前运行状态。
            nlohmann::json current = nullptr;
            // 【逐行批注】 判断条件 (const auto course = gangyi::currentCourseForIdentity(db))，成立时执行对应分支。
            if (const auto course = gangyi::currentCourseForIdentity(db))
                // 【逐行批注】 计算右侧结果并写入 current，更新当前运行状态。
                current = {{"id", course->id}, {"title", course->title}, {"href", "/plan?courseId=" + course->id}};
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"courses", arr}, {"current…。
            return crow::response(200, nlohmann::json{{"courses", arr}, {"currentCourse", current}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"courses", nlohmann::json:…。
            return crow::response(200, nlohmann::json{{"courses", nlohmann::json::array()}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // GET /api/courses/<courseId> —— 恢复课程 + 最新快照
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/courses/<string>")([&db](const crow::request& req, std::string…。
    CROW_ROUTE(app, "/api/courses/<string>")([&db](const crow::request& req, std::string courseId) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
            const auto found = gangyi::getCourseWithSnapshot(db, courseId);
            // 【逐行批注】 判断条件 (!found || !requesterCanReadCourse(db, found->course, req)) {，成立时执行对应分支。
            if (!found || !requesterCanReadCourse(db, found->course, req)) {
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(404, nlohmann::json{{"error", "课程不存在或无权访问。"}}.d…。
                return crow::response(404, nlohmann::json{{"error", "课程不存在或无权访问。"}}.dump());
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 nlohmann::json course，更新当前运行状态。
            const nlohmann::json course = {{"id", found->course.id}, {"goal", found->course.goal},
                // 【逐行批注】 执行当前声明或表达式：{"mode", found->course.mode}, {"title", found->course.title},。
                {"mode", found->course.mode}, {"title", found->course.title},
                // 【逐行批注】 调用 deepCourseEnabled 完成本行所需操作：{"pedagogy", gangyi::deepCourseEnabled(db, courseId) ? "deep-v1" : "q…。
                {"pedagogy", gangyi::deepCourseEnabled(db, courseId) ? "deep-v1" : "quick"},
                // 【逐行批注】 调用 json 完成本行所需操作：{"summary", found->course.summary ? nlohmann::json(*found->course.sum…。
                {"summary", found->course.summary ? nlohmann::json(*found->course.summary) : nlohmann::json(nullptr)},
                // 【逐行批注】 执行当前声明或表达式：{"source", found->course.source},。
                {"source", found->course.source},
                // 【逐行批注】 执行当前声明或表达式：{"createdAt", found->course.createdAt}, {"updatedAt", found->course.updatedAt}};。
                {"createdAt", found->course.createdAt}, {"updatedAt", found->course.updatedAt}};
            // 【逐行批注】 计算右侧结果并写入 nlohmann::json cards，更新当前运行状态。
            nlohmann::json cards = nlohmann::json::array();
            // 【逐行批注】 按循环规则重复处理：forEachTopic(found->payload, [&](int phaseIndex, int topicIndex, const std…。
            forEachTopic(found->payload, [&](int phaseIndex, int topicIndex, const std::string&) {
                // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
                const auto saved = db.findLearningCardProgress(courseId, phaseIndex, topicIndex);
                // 【逐行批注】 执行当前声明或表达式：cards.push_back({{"phaseIndex", phaseIndex}, {"topicIndex", topicIndex},。
                cards.push_back({{"phaseIndex", phaseIndex}, {"topicIndex", topicIndex},
                    // 【逐行批注】 执行当前声明或表达式：{"status", saved ? saved->status : "not_started"}});。
                    {"status", saved ? saved->status : "not_started"}});
            // 【逐行批注】 执行当前声明或表达式：});。
            });
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"course", course},。
            return crow::response(200, nlohmann::json{{"course", course},
                // 【逐行批注】 调用 publicCoursePayload 完成本行所需操作：{"snapshot", {{"payload", gangyi::publicCoursePayload(found->payload)…。
                {"snapshot", {{"payload", gangyi::publicCoursePayload(found->payload)}}}, {"cards", cards}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(500, nlohmann::json{{"error", "课程读取失败，请稍后重试。"}}…。
            return crow::response(500, nlohmann::json{{"error", "课程读取失败，请稍后重试。"}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/my-courses/<string>").methods(crow::HTTPMethod::DELETE)([&db](…。
    CROW_ROUTE(app, "/api/my-courses/<string>").methods(crow::HTTPMethod::DELETE)([&db](const crow::request& req, std::string courseId) {
        // 【逐行批注】 计算右侧结果并写入 std::string anonymousId，更新当前运行状态。
        const std::string anonymousId = requestAnonymousId(req);
        // 【逐行批注】 判断条件 (!gangyi::deleteCourseForIdentity(db, courseId, "", "")) {，成立时执行对应分支。
        if (!gangyi::deleteCourseForIdentity(db, courseId, "", "")) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课…。
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}}.dump());。
        return crow::response(200, nlohmann::json{{"ok", true}}.dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // ---- 阶段 B：学习体验 API ----

    // 阶段页仍使用任务进度，课程页仍使用总体进度与重置接口。
    // GET /api/task-progress —— 恢复阶段展开任务的三态进度。
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/task-progress")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/api/task-progress")([&db](const crow::request& req) {
        // 【逐行批注】 计算右侧结果并写入 char* courseId，更新当前运行状态。
        const char* courseId = req.url_params.get("courseId");
        // 【逐行批注】 计算右侧结果并写入 std::string anonymousId，更新当前运行状态。
        const std::string anonymousId = requestAnonymousId(req);
        // 【逐行批注】 计算右侧结果并写入 char* goal，更新当前运行状态。
        const char* goal = req.url_params.get("goal");
        // 【逐行批注】 计算右侧结果并写入 char* mode，更新当前运行状态。
        const char* mode = req.url_params.get("mode");
        // 【逐行批注】 计算右侧结果并写入 char* phaseIndex，更新当前运行状态。
        const char* phaseIndex = req.url_params.get("phaseIndex");
        // 【逐行批注】 判断条件 (!requesterCanAccessCourse(db, req, courseId ? courseId : "", an…，成立时执行对应分支。
        if (!requesterCanAccessCourse(db, req, courseId ? courseId : "", anonymousId)) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课…。
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 int phase，更新当前运行状态。
        int phase = 0;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { if (phaseIndex) phase = std::stoi(phaseIndex); } catch (...) {}
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json items，更新当前运行状态。
        nlohmann::json items = nlohmann::json::array();
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listTaskProgress()) {。
        for (const auto& item : db.listTaskProgress()) {
            // 【逐行批注】 计算右侧结果并写入 bool sameCourse，更新当前运行状态。
            const bool sameCourse = courseId && *courseId && item.courseId.value_or("") == courseId;
            // 【逐行批注】 计算右侧结果并写入 bool sameAnonymous，更新当前运行状态。
            const bool sameAnonymous = (!courseId || !*courseId) && !anonymousId.empty() &&
                // 【逐行批注】 执行当前声明或表达式：item.anonymousId.value_or("") == anonymousId && item.goal == (goal ? goal : "") &&。
                item.anonymousId.value_or("") == anonymousId && item.goal == (goal ? goal : "") &&
                // 【逐行批注】 执行当前声明或表达式：item.mode.value_or("deep") == (mode && *mode ? mode : "deep");。
                item.mode.value_or("deep") == (mode && *mode ? mode : "deep");
            // 【逐行批注】 判断条件 ((sameCourse || sameAnonymous) && item.phaseIndex == phase) {，成立时执行对应分支。
            if ((sameCourse || sameAnonymous) && item.phaseIndex == phase) {
                // 【逐行批注】 执行当前声明或表达式：items.push_back({{"taskIndex", item.taskIndex}, {"taskTitle", item.taskTitle}, {"sta…。
                items.push_back({{"taskIndex", item.taskIndex}, {"taskTitle", item.taskTitle}, {"status", item.status}});
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}, {"items", ite…。
        return crow::response(200, nlohmann::json{{"ok", true}, {"items", items}}.dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // POST /api/task-progress —— 阶段任务状态（写后自动重算）
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/task-progress").methods(crow::HTTPMethod::POST)([&db](const cr…。
    CROW_ROUTE(app, "/api/task-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 body，更新当前运行状态。
            auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
            const std::string courseId = body.value("courseId", "");
            // 【逐行批注】 计算右侧结果并写入 std::string anonymousId，更新当前运行状态。
            const std::string anonymousId = requestAnonymousId(req, &body);
            // 【逐行批注】 判断条件 (!requesterCanAccessCourse(db, req, courseId, anonymousId)) retu…，成立时执行对应分支。
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            // 【逐行批注】 判断条件 (!anonymousId.empty()) body["anonymousId"] = anonymousId;，成立时执行对应分支。
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
            auto result = gangyi::saveTaskProgress(db, body);
            // 【逐行批注】 判断条件 (!result.ok) {，成立时执行对应分支。
            if (!result.ok) {
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请…。
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (body.contains("courseId") && body["courseId"].is_string() && !b…，成立时执行对应分支。
            if (body.contains("courseId") && body["courseId"].is_string() && !body["courseId"].get<std::string>().empty()) {
                // 【逐行批注】 调用 recomputeCourseProgress 完成本行所需操作：gangyi::recomputeCourseProgress(db, body["courseId"].get<std::string>…。
                gangyi::recomputeCourseProgress(db, body["courseId"].get<std::string>(),
                    // 【逐行批注】 执行当前声明或表达式：body.value("anonymousId", ""), body.value("goal", ""));。
                    body.value("anonymousId", ""), body.value("goal", ""));
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}, {"item", resu…。
            return crow::response(200, nlohmann::json{{"ok", true}, {"item", result.item}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请…。
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });


    // POST /api/course-progress —— 全量重算 / 断点记录
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/course-progress").methods(crow::HTTPMethod::POST)([&db](const …。
    CROW_ROUTE(app, "/api/course-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 body，更新当前运行状态。
            auto body = nlohmann::json::parse(req.body);
            // 【逐行批注】 计算右侧结果并写入 std::string action，更新当前运行状态。
            const std::string action = body.value("action", "recompute");
            // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
            const std::string courseId = body.value("courseId", "");
            // 【逐行批注】 计算右侧结果并写入 std::string anonymousId，更新当前运行状态。
            const std::string anonymousId = requestAnonymousId(req, &body);
            // 【逐行批注】 判断条件 (!requesterCanAccessCourse(db, req, courseId, anonymousId)) retu…，成立时执行对应分支。
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            // 【逐行批注】 判断条件 (!anonymousId.empty()) body["anonymousId"] = anonymousId;，成立时执行对应分支。
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            // 【逐行批注】 计算右侧结果并写入 std::string goal，更新当前运行状态。
            const std::string goal = body.value("goal", "");
            // 【逐行批注】 计算右侧结果并写入 std::string mode，更新当前运行状态。
            const std::string mode = body.value("mode", "deep");
            // 【逐行批注】 执行当前声明或表达式：std::optional<nlohmann::json> progress;。
            std::optional<nlohmann::json> progress;
            // 【逐行批注】 判断条件 (action == "reset") {，成立时执行对应分支。
            if (action == "reset") {
                // 【逐行批注】 计算右侧结果并写入 progress，更新当前运行状态。
                progress = gangyi::resetCourseProgress(db, courseId, anonymousId, goal);
            // 【逐行批注】 执行当前声明或表达式：} else {。
            } else {
                // 【逐行批注】 判断条件 (action == "lastVisited") {，成立时执行对应分支。
                if (action == "lastVisited") {
                    // 【逐行批注】 调用 updateLastVisited 完成本行所需操作：gangyi::updateLastVisited(db, courseId, anonymousId, goal, mode, body…。
                    gangyi::updateLastVisited(db, courseId, anonymousId, goal, mode, body);
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 progress，更新当前运行状态。
                progress = gangyi::recomputeCourseProgress(db, courseId, anonymousId, goal);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (!progress) progress = nlohmann::json{{"overallPercent", 0}, {"c…，成立时执行对应分支。
            if (!progress) progress = nlohmann::json{{"overallPercent", 0}, {"completedCount", 0}, {"totalCount", 0}};
            // 【逐行批注】 判断条件 (action == "reset") db.markProfileDirty();，成立时执行对应分支。
            if (action == "reset") db.markProfileDirty();
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}, {"progress", …。
            return crow::response(200, nlohmann::json{{"ok", true}, {"progress", *progress}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请…。
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/course-progress")([&db](const crow::request& req) {。
    CROW_ROUTE(app, "/api/course-progress")([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 char* courseId，更新当前运行状态。
            const char* courseId = req.url_params.get("courseId");
            // 【逐行批注】 计算右侧结果并写入 char* anonymousId，更新当前运行状态。
            const char* anonymousId = req.url_params.get("anonymousId");
            // 【逐行批注】 判断条件 (!courseId || !*courseId) return crow::response(400, nlohmann::j…，成立时执行对应分支。
            if (!courseId || !*courseId) return crow::response(400, nlohmann::json{{"ok", false}, {"error", "缺少课程编号。"}}.dump());
            // 【逐行批注】 判断条件 (!requesterCanAccessCourse(db, req, courseId, anonymousId ? anon…，成立时执行对应分支。
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId ? anonymousId : "")) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            // 【逐行批注】 计算右侧结果并写入 auto progress，更新当前运行状态。
            const auto progress = gangyi::recomputeCourseProgress(db, courseId, anonymousId ? anonymousId : "", "");
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}, {"progress", …。
            return crow::response(200, nlohmann::json{{"ok", true}, {"progress", progress ? *progress : nlohmann::json::object()}}.dump());
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(500, nlohmann::json{{"ok", false}, {"error", "课…。
            return crow::response(500, nlohmann::json{{"ok", false}, {"error", "课程进度读取失败，请稍后重试。"}}.dump());
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });


    // POST /api/phase-expansion —— 首次由 AI 生成并保存，后续复用课程快照。

    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/api/phase-expansion").methods(crow::HTTPMethod::POST)([&db](const …。
    CROW_ROUTE(app, "/api/phase-expansion").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { const auto body = nlohmann::json::parse(req.body); const auto found = gangyi::getCourseWithSnapshot(db, body.at("courseId"));
            // 【逐行批注】 判断条件 (!found) throw std::invalid_argument("课程不存在");，成立时执行对应分支。
            if (!found) throw std::invalid_argument("课程不存在");
            // 【逐行批注】 计算右侧结果并写入 int index，更新当前运行状态。
            const int index = body.value("phaseIndex", 1) - 1; const auto outline = found->payload.value("courseStructure", nlohmann::json::array());
            // 【逐行批注】 判断条件 (index < 0 || index >= static_cast<int>(outline.size())) throw s…，成立时执行对应分支。
            if (index < 0 || index >= static_cast<int>(outline.size())) throw std::invalid_argument("阶段不存在");
            // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}, {"cached", tr…。
            return crow::response(200, nlohmann::json{{"ok", true}, {"cached", true}, {"phase", outline.at(index)}, {"resources", found->payload.value("resources", nlohmann::json::array())}}.dump());
        // 【逐行批注】 调用 response 完成本行所需操作：} catch (const std::exception& error) { return crow::response(409, nl…。
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册 HTTP 路由并绑定对应处理函数：CROW_ROUTE(app, "/<path>")([](const crow::request&, crow::response& response, std::s…。
    CROW_ROUTE(app, "/<path>")([](const crow::request&, crow::response& response, std::string path) {
        // 【逐行批注】 计算右侧结果并写入 auto public_root，更新当前运行状态。
        const auto public_root = std::filesystem::weakly_canonical("public");
        // 【逐行批注】 计算右侧结果并写入 auto requested，更新当前运行状态。
        const auto requested = std::filesystem::weakly_canonical(public_root / path);
        // 【逐行批注】 计算右侧结果并写入 auto root_text，更新当前运行状态。
        const auto root_text = public_root.generic_string();
        // 【逐行批注】 计算右侧结果并写入 auto requested_text，更新当前运行状态。
        const auto requested_text = requested.generic_string();
        // 【逐行批注】 计算右侧结果并写入 bool is_under_public，更新当前运行状态。
        const bool is_under_public = requested_text == root_text ||
            // 【逐行批注】 执行当前声明或表达式：(requested_text.rfind(root_text, 0) == 0 &&。
            (requested_text.rfind(root_text, 0) == 0 &&
             // 【逐行批注】 执行当前声明或表达式：requested_text[root_text.size()] == '/');。
             requested_text[root_text.size()] == '/');
        // 【逐行批注】 判断条件 (!is_under_public || !std::filesystem::is_regular_file(requested…，成立时执行对应分支。
        if (!is_under_public || !std::filesystem::is_regular_file(requested)) {
            // 【逐行批注】 计算右侧结果并写入 response.code，更新当前运行状态。
            response.code = 404;
            // 【逐行批注】 执行当前声明或表达式：response.end("Not found");。
            response.end("Not found");
            // 【逐行批注】 结束当前函数并返回结果：return;。
            return;
        // 【逐行批注】 结束当前代码块。
        }

        // 【逐行批注】 声明对象 file，并通过构造函数初始化其状态。
        std::ifstream file(requested, std::ios::binary);
        // 【逐行批注】 执行当前声明或表达式：std::ostringstream body;。
        std::ostringstream body;
        // 【逐行批注】 执行当前声明或表达式：body << file.rdbuf();。
        body << file.rdbuf();
        // 【逐行批注】 调用 content_type_for 完成本行所需操作：response.set_header("Content-Type", content_type_for(requested));。
        response.set_header("Content-Type", content_type_for(requested));
        // 脚本与样式升级后必须立即生效，避免 WebView 继续运行旧版聊天界面。
        // 【逐行批注】 计算右侧结果并写入 auto extension，更新当前运行状态。
        const auto extension = requested.extension().string();
        // 【逐行批注】 判断条件 (extension == ".js" || extension == ".css")，成立时执行对应分支。
        if (extension == ".js" || extension == ".css")
            // 【逐行批注】 计算右侧结果并写入 response.set_header("Cache-Control", "no-sto…，更新当前运行状态。
            response.set_header("Cache-Control", "no-store, max-age=0");
        // 【逐行批注】 计算右侧结果并写入 response.code，更新当前运行状态。
        response.code = 200;
        // 【逐行批注】 执行当前声明或表达式：response.end(body.str());。
        response.end(body.str());
    // 【逐行批注】 执行当前声明或表达式：});。
    });

    // 【逐行批注】 判断条件 (!config.local_control_token.empty() && config.host == "127.0.0.…，成立时执行对应分支。
    if (!config.local_control_token.empty() && config.host == "127.0.0.1") {
        // 【逐行批注】 执行当前声明或表达式：app.route_dynamic("/internal/shutdown")。
        app.route_dynamic("/internal/shutdown")
            // 【逐行批注】 执行当前声明或表达式：.methods(crow::HTTPMethod::POST)。
            .methods(crow::HTTPMethod::POST)
            // 【逐行批注】 计算右侧结果并写入 ([&app, token，更新当前运行状态。
            ([&app, token = config.local_control_token](const crow::request& req) {
                // 【逐行批注】 判断条件 (req.get_header_value("X-Gangyi-Control-Token") != token) {，成立时执行对应分支。
                if (req.get_header_value("X-Gangyi-Control-Token") != token) {
                    // 【逐行批注】 结束当前函数并返回结果：return crow::response(403, nlohmann::json{{"ok", false}}.dump());。
                    return crow::response(403, nlohmann::json{{"ok", false}}.dump());
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 调用 thread 完成本行所需操作：std::thread([&app] {。
                std::thread([&app] {
                    // 【逐行批注】 调用 shutdown 完成本行所需操作：gangyi::AIActivity::shutdown();。
                    gangyi::AIActivity::shutdown();
                    // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(100));。
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    // 【逐行批注】 执行当前声明或表达式：app.stop();。
                    app.stop();
                // 【逐行批注】 执行当前声明或表达式：}).detach();。
                }).detach();
                // 【逐行批注】 结束当前函数并返回结果：return crow::response(200, nlohmann::json{{"ok", true}}.dump());。
                return crow::response(200, nlohmann::json{{"ok", true}}.dump());
            // 【逐行批注】 执行当前声明或表达式：});。
            });
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 计算右侧结果并写入 auto launchId，更新当前运行状态。
    const auto launchId = config.launch_session_id.empty() ? nowIso8601() : config.launch_session_id;
    // 【逐行批注】 调用 agentSubmit、localAgentAccess 完成本行所需操作：gangyi::agentSubmit(db, localAgentAccess(db), {{"type", "startup"}, {…。
    gangyi::agentSubmit(db, localAgentAccess(db), {{"type", "startup"}, {"requestId", "startup-" + launchId},
        // 【逐行批注】 执行当前声明或表达式：{"text", "读取真实学习情况，生成快速规划和深度课程各五条推荐，并自主决定需要准备的下一步。无记录时不推测掌握度。"}});。
        {"text", "读取真实学习情况，生成快速规划和深度课程各五条推荐，并自主决定需要准备的下一步。无记录时不推测掌握度。"}});
    // 【逐行批注】 执行当前声明或表达式：learningAgent.start();。
    learningAgent.start();
    // 画像由同一真实 AI 主控按学习事件更新，不再启动独立评分线程。
    // 【逐行批注】 执行当前声明或表达式：std::cout << "gangyiAI " << gangyi::kVersion << " listening on " << config.host << '…。
    std::cout << "gangyiAI " << gangyi::kVersion << " listening on " << config.host << ':' << config.port << '\n';
    // 【逐行批注】 执行当前声明或表达式：app.bindaddr(config.host).port(config.port).multithreaded().run();。
    app.bindaddr(config.host).port(config.port).multithreaded().run();
    // 【逐行批注】 调用 shutdown 完成本行所需操作：gangyi::AIActivity::shutdown();。
    gangyi::AIActivity::shutdown();
    // 【逐行批注】 执行当前声明或表达式：learningAgent.stop();。
    learningAgent.stop();
    // 【逐行批注】 调用 guard 完成本行所需操作：{ std::lock_guard<std::mutex> guard(socketMapMutex); for (const auto&…。
    { std::lock_guard<std::mutex> guard(socketMapMutex); for (const auto& [connection, state] : socketMap) state->cancelled = true; }
    // 【逐行批注】 在条件成立期间循环处理：while (socketWorkerCount > 0) std::this_thread::sleep_for(std::chrono::mil…。
    while (socketWorkerCount > 0) std::this_thread::sleep_for(std::chrono::milliseconds(100));
// 【逐行批注】 结束当前代码块。
}

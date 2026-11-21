// 【逐行批注】 引入头文件 "deep_course.hpp"，为后续实现提供类型或接口声明。
#include "deep_course.hpp"
#include "interactive_capabilities.hpp"
#include "interactive_sets.hpp"
#include "interactive_labs.hpp"
#include "component_activity.hpp"
// 【逐行批注】 引入头文件 "agent_curriculum.hpp"，为后续实现提供类型或接口声明。
#include "agent_curriculum.hpp"
// 【逐行批注】 引入头文件 "agent_lessons.hpp"，为后续实现提供类型或接口声明。
#include "agent_lessons.hpp"
// 【逐行批注】 引入头文件 "library_io.hpp"，为后续实现提供类型或接口声明。
#include "library_io.hpp"
// 【逐行批注】 引入头文件 "search_client.hpp"，为后续实现提供类型或接口声明。
#include "search_client.hpp"
// 【逐行批注】 引入头文件 <curl/curl.h>，为后续实现提供类型或接口声明。
#include <curl/curl.h>
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <cmath>，为后续实现提供类型或接口声明。
#include <cmath>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>
// 【逐行批注】 引入头文件 <memory>，为后续实现提供类型或接口声明。
#include <memory>
// 【逐行批注】 引入头文件 <regex>，为后续实现提供类型或接口声明。
#include <regex>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 定义函数 parsed，执行该函数声明的职责。
Json parsed(const std::string& value) {
    // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
    const auto result = Json::parse(value, nullptr, false);
    // 【逐行批注】 结束当前函数并返回结果：return result.is_object() ? result : Json::object();。
    return result.is_object() ? result : Json::object();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 text，执行该函数声明的职责。
std::string text(const Json& value, const char* key, size_t limit = 64000) {
    // 【逐行批注】 判断条件 (!value.is_object() || !value.value(key, Json()).is_string()) th…，成立时执行对应分支。
    if (!value.is_object() || !value.value(key, Json()).is_string()) throw std::invalid_argument(std::string(key) + "须为文字");
    // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
    const auto result = value.at(key).get<std::string>();
    // 【逐行批注】 判断条件 (result.empty() || result.size() > limit) throw std::invalid_arg…，成立时执行对应分支。
    if (result.empty() || result.size() > limit) throw std::invalid_argument(std::string(key) + "为空或过长");
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 number，执行该函数声明的职责。
double number(const Json& value, const char* key, double fallback, double minimum, double maximum) {
    // 【逐行批注】 判断条件 (!value.contains(key)) return fallback;，成立时执行对应分支。
    if (!value.contains(key)) return fallback;
    // 【逐行批注】 判断条件 (!value.at(key).is_number()) throw std::invalid_argument(std::st…，成立时执行对应分支。
    if (!value.at(key).is_number()) throw std::invalid_argument(std::string(key) + "须为数字");
    // 【逐行批注】 计算右侧结果并写入 double result，更新当前运行状态。
    const double result = value.at(key).get<double>();
    // 【逐行批注】 判断条件 (!std::isfinite(result) || result < minimum || result > maximum)…，成立时执行对应分支。
    if (!std::isfinite(result) || result < minimum || result > maximum) throw std::invalid_argument(std::string(key) + "超出有效范围");
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 positiveIndex，执行该函数声明的职责。
int positiveIndex(const Json& value, const char* key) {
    // 【逐行批注】 判断条件 (!value.contains(key)) return 1;，成立时执行对应分支。
    if (!value.contains(key)) return 1;
    // 【逐行批注】 计算右侧结果并写入 long long index，更新当前运行状态。
    long long index = 0;
    // 【逐行批注】 判断条件 (value.at(key).is_number_integer()) index = value.at(key).get<lo…，成立时执行对应分支。
    if (value.at(key).is_number_integer()) index = value.at(key).get<long long>();
    // 【逐行批注】 在前一条件不成立时继续判断：(value.at(key).is_string()) {。
    else if (value.at(key).is_string()) {
        // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
        const auto input = value.at(key).get<std::string>();
        // 【逐行批注】 判断条件 (input.empty() || input.size() > 6 || !std::all_of(input.begin()…，成立时执行对应分支。
        if (input.empty() || input.size() > 6 || !std::all_of(input.begin(), input.end(), [](unsigned char c) { return c >= '0' && c <= '9'; }))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument(std::string(key) + "须为正整数");。
            throw std::invalid_argument(std::string(key) + "须为正整数");
        // 【逐行批注】 计算右侧结果并写入 index，更新当前运行状态。
        index = std::stoll(input);
    // 【逐行批注】 调用 invalid_argument、string 完成本行所需操作：} else throw std::invalid_argument(std::string(key) + "须为正整数");。
    } else throw std::invalid_argument(std::string(key) + "须为正整数");
    // 【逐行批注】 判断条件 (index < 1 || index > 100000) throw std::invalid_argument(std::s…，成立时执行对应分支。
    if (index < 1 || index > 100000) throw std::invalid_argument(std::string(key) + "超出有效范围");
    // 【逐行批注】 结束当前函数并返回结果：return static_cast<int>(index);。
    return static_cast<int>(index);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 accessFor，执行该函数声明的职责。
AgentAccess accessFor(const Json& task) { return {task.at("scopeId"), task.at("courseIds").get<std::vector<std::string>>()}; }
// 【逐行批注】 定义函数 requireCourse，执行该函数声明的职责。
void requireCourse(Database& db, const AgentAccess& access, const std::string& id) {
    // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
    const auto course = db.getCourse(id);
    // 【逐行批注】 判断条件 (!course || course->status != "active" || std::find(access.cours…，成立时执行对应分支。
    if (!course || course->status != "active" || std::find(access.courseIds.begin(), access.courseIds.end(), id) == access.courseIds.end())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课程不存在或无权读取");。
        throw std::invalid_argument("课程不存在或无权读取");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 savedLesson，执行该函数声明的职责。
Json savedLesson(Database& db, const AgentAccess& access, const std::string& id) {
    // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
    const auto row = db.getClassroomActivity(id);
    // 【逐行批注】 判断条件 (!row || row->kind != "agent-lesson") throw std::invalid_argumen…，成立时执行对应分支。
    if (!row || row->kind != "agent-lesson") throw std::invalid_argument("课时不存在");
    // 【逐行批注】 计算右侧结果并写入 lesson，更新当前运行状态。
    auto lesson = parsed(row->payload);
    // 【逐行批注】 判断条件 (lesson.value("scopeId", "") != access.scopeId) throw std::inval…，成立时执行对应分支。
    if (lesson.value("scopeId", "") != access.scopeId) throw std::invalid_argument("无权读取课时");
    // 【逐行批注】 调用 requireCourse 完成本行所需操作：requireCourse(db, access, row->courseId);。
    requireCourse(db, access, row->courseId);
    // 【逐行批注】 判断条件 (!deepCourseEnabled(db, row->courseId)) throw std::invalid_argum…，成立时执行对应分支。
    if (!deepCourseEnabled(db, row->courseId)) throw std::invalid_argument("该课程使用快速规划课堂");
    // 【逐行批注】 结束当前函数并返回结果：return lesson;。
    return lesson;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 validateTarget，执行该函数声明的职责。
void validateTarget(const Json& lesson, const Json& task) {
    // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
    const auto current = task.at("event").value("lessonId", "");
    // 【逐行批注】 判断条件 (!current.empty() && current != lesson.at("id")) throw std::inva…，成立时执行对应分支。
    if (!current.empty() && current != lesson.at("id")) throw std::invalid_argument("只能修改当前课时的教学素材");
    // 【逐行批注】 判断条件 (!lesson.value("teachingTaskId", "").empty() && !current.empty()…，成立时执行对应分支。
    if (!lesson.value("teachingTaskId", "").empty() && !current.empty() && lesson.at("teachingTaskId") != task.at("id"))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("旧任务不能覆盖当前课时素材");。
        throw std::invalid_argument("旧任务不能覆盖当前课时素材");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 updateLesson，执行该函数声明的职责。
PreparedAgentTool updateLesson(Database& db, const Json& task, Json lesson, Json result) {
    // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
    const auto access = accessFor(task); const auto id = lesson.at("id").get<std::string>();
    // 【逐行批注】 计算右侧结果并写入 auto before，更新当前运行状态。
    const auto before = savedLesson(db, access, id);
    // 【逐行批注】 计算右侧结果并写入 lesson["updatedAt"]，更新当前运行状态。
    lesson["updatedAt"] = libraryNow(); lesson["version"] = before.value("version", 1) + 1;
    // 【逐行批注】 结束当前函数并返回结果：return {std::move(result), [access, id, before, lesson](Database& con…。
    return {std::move(result), [access, id, before, lesson](Database& connection) {
        // 【逐行批注】 判断条件 (savedLesson(connection, access, id) != before) throw std::inval…，成立时执行对应分支。
        if (savedLesson(connection, access, id) != before) throw std::invalid_argument("课时已经更新，请重新读取后重试");
        // 【逐行批注】 判断条件 (!connection.upsert(ClassroomActivity{id, lesson.at("courseId"),…，成立时执行对应分支。
        if (!connection.upsert(ClassroomActivity{id, lesson.at("courseId"), "agent-lesson", lesson.dump(), libraryNow(), 0, 0}))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("教学素材保存失败");。
            throw std::runtime_error("教学素材保存失败");
    // 【逐行批注】 执行当前声明或表达式：}};。
    }};
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 mediaIds，执行该函数声明的职责。
Json mediaIds(const Json& block) {
    // 【逐行批注】 计算右侧结果并写入 auto ids，更新当前运行状态。
    const auto ids = block.value("mediaIds", Json::array());
    // 【逐行批注】 判断条件 (!ids.is_array() || ids.size() > 32) throw std::invalid_argument…，成立时执行对应分支。
    if (!ids.is_array() || ids.size() > 32) throw std::invalid_argument("mediaIds须为素材标识列表");
    // 【逐行批注】 按循环规则重复处理：for (const auto& id : ids) if (!id.is_string()) throw std::invalid_argumen…。
    for (const auto& id : ids) if (!id.is_string()) throw std::invalid_argument("mediaIds只能使用文字标识");
    // 【逐行批注】 结束当前函数并返回结果：return ids;。
    return ids;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 bindDeepSections，执行该函数声明的职责。
Json bindDeepSections(const Json& lesson, Json input) {
    // 【逐行批注】 判断条件 (!input.is_object()) throw std::invalid_argument("课件内容须为对象");，成立时执行对应分支。
    if (!input.is_object()) throw std::invalid_argument("课件内容须为对象");
    // 【逐行批注】 计算右侧结果并写入 auto find，更新当前运行状态。
    const auto find = [&](const std::string& id, bool lecture) -> const Json& {
        // 【逐行批注】 按循环规则重复处理：for (const auto& section : lesson.at("sections")) if (section.at("id") == …。
        for (const auto& section : lesson.at("sections")) if (section.at("id") == id) {
            // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
            const auto kind = section.value("kind", "");
            // 【逐行批注】 判断条件 (kind != "explanation" && (lecture || kind != "summary"))，成立时执行对应分支。
            if (kind != "explanation" && (lecture || kind != "summary"))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("讲义与课件只能引用本课已保存的公开讲解或小结，不能引用题目私有标准");。
                throw std::invalid_argument("讲义与课件只能引用本课已保存的公开讲解或小结，不能引用题目私有标准");
            // 【逐行批注】 调用 text 完成本行所需操作：text(section, "body"); return section;。
            text(section, "body"); return section;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课件引用的讲解板块不属于当前课时，请先append_section保存讲解");。
        throw std::invalid_argument("课件引用的讲解板块不属于当前课时，请先append_section保存讲解");
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 按循环规则重复处理：for (auto& block : input["lecture"]) {。
    for (auto& block : input["lecture"]) {
        // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
        const auto id = block.value("sectionId", block.value("id", ""));
        // 【逐行批注】 计算右侧结果并写入 auto& section，更新当前运行状态。
        const auto& section = find(id, true);
        // 【逐行批注】 计算右侧结果并写入 block["id"]，更新当前运行状态。
        block["id"] = id; block["sectionId"] = id;
        // 【逐行批注】 计算右侧结果并写入 block["title"]，更新当前运行状态。
        block["title"] = section.at("title"); block["body"] = section.at("body");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (auto& slide : input["slides"]) {。
    for (auto& slide : input["slides"]) {
        // 【逐行批注】 计算右侧结果并写入 auto ids，更新当前运行状态。
        const auto ids = slide.value("sourceSectionIds", Json::array());
        // 【逐行批注】 判断条件 (!ids.is_array() || ids.empty() || ids.size() > 20) throw std::i…，成立时执行对应分支。
        if (!ids.is_array() || ids.empty() || ids.size() > 20) throw std::invalid_argument("课件须填写本课真实sourceSectionIds");
        // 【逐行批注】 执行当前声明或表达式：std::string body;。
        std::string body;
        // 【逐行批注】 按循环规则重复处理：for (const auto& id : ids) { if (!id.is_string()) throw std::invalid_argum…。
        for (const auto& id : ids) { if (!id.is_string()) throw std::invalid_argument("课件来源板块标识无效"); const auto& section = find(id.get<std::string>(), false); if (!body.empty()) body += "\n\n"; body += section.at("body").get<std::string>(); }
        // 【逐行批注】 计算右侧结果并写入 slide["body"]，更新当前运行状态。
        slide["body"] = body;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return input;。
    return input;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 encode64，执行该函数声明的职责。
std::string encode64(const std::string& input) {
    // 【逐行批注】 计算右侧结果并写入 constexpr char alphabet[]，更新当前运行状态。
    static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // 【逐行批注】 执行当前声明或表达式：std::string output; output.reserve((input.size() + 2) / 3 * 4);。
    std::string output; output.reserve((input.size() + 2) / 3 * 4);
    // 【逐行批注】 计算右侧结果并写入 unsigned int value，更新当前运行状态。
    unsigned int value = 0; int bits = -6;
    // 【逐行批注】 按循环规则重复处理：for (const unsigned char byte : input) { value = (value << 8) | byte; bits…。
    for (const unsigned char byte : input) { value = (value << 8) | byte; bits += 8; while (bits >= 0) { output += alphabet[(value >> bits) & 63]; bits -= 6; } }
    // 【逐行批注】 判断条件 (bits > -6) output += alphabet[((value << 8) >> (bits + 8)) & 63…，成立时执行对应分支。
    if (bits > -6) output += alphabet[((value << 8) >> (bits + 8)) & 63];
    // 【逐行批注】 在条件成立期间循环处理：while (output.size() % 4) output += '=';。
    while (output.size() % 4) output += '=';
    // 【逐行批注】 结束当前函数并返回结果：return output;。
    return output;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 decode64，执行该函数声明的职责。
std::string decode64(const std::string& input) {
    // 【逐行批注】 计算右侧结果并写入 const std::string alphabet，更新当前运行状态。
    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // 【逐行批注】 计算右侧结果并写入 std::string output; unsigned int value，更新当前运行状态。
    std::string output; unsigned int value = 0; int bits = -8;
    // 【逐行批注】 按循环规则重复处理：for (const char byte : input) { if (byte == '=') break; const auto index =…。
    for (const char byte : input) { if (byte == '=') break; const auto index = alphabet.find(byte); if (index == std::string::npos) throw std::invalid_argument("图片缓存无效"); value = (value << 6) | static_cast<unsigned int>(index); bits += 6; if (bits >= 0) { output += static_cast<char>((value >> bits) & 255); bits -= 8; } }
    // 【逐行批注】 结束当前函数并返回结果：return output;。
    return output;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 urlPart，执行该函数声明的职责。
std::string urlPart(const std::string& input) {
    // 【逐行批注】 计算右侧结果并写入 constexpr char digits[]，更新当前运行状态。
    static constexpr char digits[] = "0123456789ABCDEF";
    // 【逐行批注】 执行当前声明或表达式：std::string output;。
    std::string output;
    // 【逐行批注】 按循环规则重复处理：for (const unsigned char byte : input) {。
    for (const unsigned char byte : input) {
        // 【逐行批注】 判断条件 ((byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') ||…，成立时执行对应分支。
        if ((byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') || (byte >= '0' && byte <= '9') || byte == '-' || byte == '_' || byte == '.') output += static_cast<char>(byte);
        // 【逐行批注】 执行前一条件不成立时的分支。
        else { output += '%'; output += digits[byte >> 4]; output += digits[byte & 15]; }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return output;。
    return output;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 imageMime，执行该函数声明的职责。
std::string imageMime(const std::string& body) {
    // 【逐行批注】 判断条件 (body.size() >= 8 && body.compare(0, 8, "\x89PNG\r\n\x1a\n", 8) …，成立时执行对应分支。
    if (body.size() >= 8 && body.compare(0, 8, "\x89PNG\r\n\x1a\n", 8) == 0) return "image/png";
    // 【逐行批注】 判断条件 (body.size() >= 3 && static_cast<unsigned char>(body[0]) == 0xff…，成立时执行对应分支。
    if (body.size() >= 3 && static_cast<unsigned char>(body[0]) == 0xff && static_cast<unsigned char>(body[1]) == 0xd8 && static_cast<unsigned char>(body[2]) == 0xff) return "image/jpeg";
    // 【逐行批注】 判断条件 (body.size() >= 12 && body.compare(0, 4, "RIFF") == 0 && body.co…，成立时执行对应分支。
    if (body.size() >= 12 && body.compare(0, 4, "RIFF") == 0 && body.compare(8, 4, "WEBP") == 0) return "image/webp";
    // 【逐行批注】 判断条件 (body.size() >= 6 && (body.compare(0, 6, "GIF87a") == 0 || body.…，成立时执行对应分支。
    if (body.size() >= 6 && (body.compare(0, 6, "GIF87a") == 0 || body.compare(0, 6, "GIF89a") == 0)) return "image/gif";
    // 【逐行批注】 结束当前函数并返回结果：return {};。
    return {};
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 attribute，执行该函数声明的职责。
std::string attribute(const std::string& tag, const std::string& name) {
    // 【逐行批注】 调用 expression 完成本行所需操作：const std::regex expression("(?:^|[\\s<])" + name + "\\s*=\\s*([\"'])…。
    const std::regex expression("(?:^|[\\s<])" + name + "\\s*=\\s*([\"'])(.*?)\\1", std::regex::icase);
    // 【逐行批注】 执行当前声明或表达式：std::smatch match;。
    std::smatch match;
    // 【逐行批注】 结束当前函数并返回结果：return std::regex_search(tag, match, expression) ? match[2].str() : s…。
    return std::regex_search(tag, match, expression) ? match[2].str() : std::string();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 lowerText，执行该函数声明的职责。
std::string lowerText(std::string value) {
    // 【逐行批注】 调用 transform、tolower 完成本行所需操作：std::transform(value.begin(), value.end(), value.begin(), [](unsigned…。
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 imageTerms，执行该函数声明的职责。
std::set<std::string> imageTerms(const std::string& text) {
    // 【逐行批注】 执行当前声明或表达式：static const std::set<std::string> generic = {"the", "and", "for", "with", "from", "…。
    static const std::set<std::string> generic = {"the", "and", "for", "with", "from", "this", "that", "image", "picture", "reference", "illustration", "图片", "参考", "插图", "示意", "教学"};
    // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
    const auto value = lowerText(text);
    // 【逐行批注】 执行当前声明或表达式：std::set<std::string> result;。
    std::set<std::string> result;
    // 【逐行批注】 执行当前声明或表达式：std::string word, previousCJK;。
    std::string word, previousCJK;
    // 【逐行批注】 计算右侧结果并写入 auto append，更新当前运行状态。
    const auto append = [&] {
        // 【逐行批注】 判断条件 (word.size() > 4 && word.back() == 's') word.pop_back();，成立时执行对应分支。
        if (word.size() > 4 && word.back() == 's') word.pop_back();
        // 【逐行批注】 判断条件 (word.size() >= 3 && !generic.count(word)) result.insert(word);，成立时执行对应分支。
        if (word.size() >= 3 && !generic.count(word)) result.insert(word);
        // 【逐行批注】 执行当前声明或表达式：word.clear();。
        word.clear();
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < value.size();) {。
    for (size_t i = 0; i < value.size();) {
        // 【逐行批注】 计算右侧结果并写入 auto c，更新当前运行状态。
        const auto c = static_cast<unsigned char>(value[i]);
        // 【逐行批注】 判断条件 (c < 128) {，成立时执行对应分支。
        if (c < 128) {
            // 【逐行批注】 判断条件 (std::isalnum(c)) word += static_cast<char>(c); else append();，成立时执行对应分支。
            if (std::isalnum(c)) word += static_cast<char>(c); else append();
            // 【逐行批注】 执行当前声明或表达式：previousCJK.clear(); ++i; continue;。
            previousCJK.clear(); ++i; continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 append 完成本行所需操作：append();。
        append();
        // 【逐行批注】 计算右侧结果并写入 size_t length，更新当前运行状态。
        const size_t length = c >= 0xf0 ? 4 : c >= 0xe0 ? 3 : c >= 0xc0 ? 2 : 1;
        // 【逐行批注】 判断条件 (i + length > value.size()) break;，成立时执行对应分支。
        if (i + length > value.size()) break;
        // 中文使用相邻二字片段匹配，不把整句或UTF-8的半个字符当成关键词。
        // 【逐行批注】 计算右侧结果并写入 auto codepoint，更新当前运行状态。
        const auto codepoint = length == 3 ? ((c & 15U) << 12) | ((static_cast<unsigned char>(value[i + 1]) & 63U) << 6) | (static_cast<unsigned char>(value[i + 2]) & 63U) : 0U;
        // 【逐行批注】 计算右侧结果并写入 bool cjk，更新当前运行状态。
        const bool cjk = codepoint >= 0x3400 && codepoint <= 0x9fff;
        // 【逐行批注】 计算右侧结果并写入 auto character，更新当前运行状态。
        const auto character = value.substr(i, length);
        // 【逐行批注】 判断条件 (cjk && !previousCJK.empty() && !generic.count(previousCJK + cha…，成立时执行对应分支。
        if (cjk && !previousCJK.empty() && !generic.count(previousCJK + character)) result.insert(previousCJK + character);
        // 【逐行批注】 计算右侧结果并写入 previousCJK，更新当前运行状态。
        previousCJK = cjk ? character : std::string(); i += length;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 append 完成本行所需操作：append(); return result;。
    append(); return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 imageFigureCaption，执行该函数声明的职责。
std::string imageFigureCaption(const std::string& html, const std::string& lower, size_t position) {
    // 【逐行批注】 计算右侧结果并写入 size_t begin，更新当前运行状态。
    size_t begin = lower.rfind("<figure", position), end = std::string::npos;
    // 【逐行批注】 判断条件 (begin != std::string::npos) end = lower.find("</figure", begin);，成立时执行对应分支。
    if (begin != std::string::npos) end = lower.find("</figure", begin);
    // 【逐行批注】 判断条件 (begin == std::string::npos || end == std::string::npos || end <…，成立时执行对应分支。
    if (begin == std::string::npos || end == std::string::npos || end < position || end - begin > 20000) {
        // OpenStax等教材使用div[data-type=figure]；按当前容器的div深度定位，不借用相邻图的说明。
        // npos不能参与max选择，否则会覆盖另一个有效位置。
        // 【逐行批注】 计算右侧结果并写入 auto doubleQuoted，更新当前运行状态。
        const auto doubleQuoted = lower.rfind("data-type=\"figure\"", position), singleQuoted = lower.rfind("data-type='figure'", position);
        // 【逐行批注】 计算右侧结果并写入 begin，更新当前运行状态。
        begin = doubleQuoted == std::string::npos ? singleQuoted : singleQuoted == std::string::npos ? doubleQuoted : std::max(doubleQuoted, singleQuoted);
        // 【逐行批注】 判断条件 (begin == std::string::npos) return {};，成立时执行对应分支。
        if (begin == std::string::npos) return {};
        // 【逐行批注】 计算右侧结果并写入 begin，更新当前运行状态。
        begin = lower.rfind('<', begin);
        // 【逐行批注】 判断条件 (begin == std::string::npos || lower.compare(begin, 4, "<div") !…，成立时执行对应分支。
        if (begin == std::string::npos || lower.compare(begin, 4, "<div") != 0) return {};
        // 【逐行批注】 计算右侧结果并写入 auto fragment，更新当前运行状态。
        const auto fragment = lower.substr(begin, 20000);
        // 【逐行批注】 计算右侧结果并写入 end，更新当前运行状态。
        end = std::string::npos;
        // 【逐行批注】 调用 containers 完成本行所需操作：const std::regex containers("</?div\\b[^>]{0,4000}>", std::regex::ica…。
        const std::regex containers("</?div\\b[^>]{0,4000}>", std::regex::icase);
        // 【逐行批注】 计算右侧结果并写入 int depth，更新当前运行状态。
        int depth = 0;
        // 【逐行批注】 按循环规则重复处理：for (auto it = std::sregex_iterator(fragment.begin(), fragment.end(), cont…。
        for (auto it = std::sregex_iterator(fragment.begin(), fragment.end(), containers); it != std::sregex_iterator(); ++it) {
            // 【逐行批注】 计算右侧结果并写入 depth +，更新当前运行状态。
            depth += it->str().compare(0, 2, "</") == 0 ? -1 : 1;
            // 【逐行批注】 判断条件 (depth == 0) { end = begin + static_cast<size_t>(it->position())…，成立时执行对应分支。
            if (depth == 0) { end = begin + static_cast<size_t>(it->position()); break; }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (end == std::string::npos || end < position) return {};，成立时执行对应分支。
        if (end == std::string::npos || end < position) return {};
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 caption，更新当前运行状态。
    auto caption = lower.find("<figcaption", begin);
    // 【逐行批注】 计算右侧结果并写入 std::string closing，更新当前运行状态。
    std::string closing = "</figcaption";
    // 【逐行批注】 判断条件 (caption == std::string::npos || caption >= end) {，成立时执行对应分支。
    if (caption == std::string::npos || caption >= end) {
        // 【逐行批注】 计算右侧结果并写入 caption，更新当前运行状态。
        caption = lower.find("data-type=\"caption\"", begin);
        // 【逐行批注】 判断条件 (caption == std::string::npos || caption >= end) caption = lower…，成立时执行对应分支。
        if (caption == std::string::npos || caption >= end) caption = lower.find("data-type='caption'", begin);
        // 【逐行批注】 计算右侧结果并写入 closing，更新当前运行状态。
        closing = "</span";
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (caption == std::string::npos || caption >= end) return {};，成立时执行对应分支。
    if (caption == std::string::npos || caption >= end) return {};
    // 【逐行批注】 计算右侧结果并写入 auto start，更新当前运行状态。
    const auto start = lower.find('>', caption), finish = lower.find(closing, caption);
    // 【逐行批注】 判断条件 (start == std::string::npos || finish == std::string::npos || fi…，成立时执行对应分支。
    if (start == std::string::npos || finish == std::string::npos || finish > end || finish <= start) return {};
    // 【逐行批注】 结束当前函数并返回结果：return std::regex_replace(html.substr(start + 1, std::min<size_t>(fin…。
    return std::regex_replace(html.substr(start + 1, std::min<size_t>(finish - start - 1, 3000)), std::regex("<[^>]{0,1000}>"), " ");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 resolveURL，执行该函数声明的职责。
std::string resolveURL(const std::string& base, std::string reference) {
    // 【逐行批注】 按循环规则重复处理：for (const auto& entity : {std::pair<std::string, std::string>{"&amp;", "&…。
    for (const auto& entity : {std::pair<std::string, std::string>{"&amp;", "&"}, {"&#38;", "&"}}) {
        // 【逐行批注】 计算右侧结果并写入 size_t position，更新当前运行状态。
        size_t position = 0; while ((position = reference.find(entity.first, position)) != std::string::npos) { reference.replace(position, entity.first.size(), entity.second); position += entity.second.size(); }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 decltype、url、curl_url 完成本行所需操作：std::unique_ptr<CURLU, decltype(&curl_url_cleanup)> url(curl_url(), c…。
    std::unique_ptr<CURLU, decltype(&curl_url_cleanup)> url(curl_url(), curl_url_cleanup);
    // 【逐行批注】 判断条件 (!url || curl_url_set(url.get(), CURLUPART_URL, base.c_str(), 0)…，成立时执行对应分支。
    if (!url || curl_url_set(url.get(), CURLUPART_URL, base.c_str(), 0) != CURLUE_OK || curl_url_set(url.get(), CURLUPART_URL, reference.c_str(), 0) != CURLUE_OK) return {};
    // 【逐行批注】 计算右侧结果并写入 char* value，更新当前运行状态。
    char* value = nullptr;
    // 【逐行批注】 判断条件 (curl_url_get(url.get(), CURLUPART_URL, &value, 0) != CURLUE_OK)…，成立时执行对应分支。
    if (curl_url_get(url.get(), CURLUPART_URL, &value, 0) != CURLUE_OK) return {};
    // 【逐行批注】 计算右侧结果并写入 std::string result，更新当前运行状态。
    const std::string result = value; curl_free(value);
    // 【逐行批注】 结束当前函数并返回结果：return libraryPublicURL(result) ? result : std::string();。
    return libraryPublicURL(result) ? result : std::string();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 locateLesson，执行该函数声明的职责。
Json locateLesson(Database& db, const AgentAccess& access, const Json& scope) {
    // 【逐行批注】 判断条件 (!scope.value("lessonId", "").empty()) {，成立时执行对应分支。
    if (!scope.value("lessonId", "").empty()) {
        // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
        const auto lesson = savedLesson(db, access, text(scope, "lessonId"));
        // 【逐行批注】 判断条件 ((!scope.value("courseId", "").empty() && scope.at("courseId") !…，成立时执行对应分支。
        if ((!scope.value("courseId", "").empty() && scope.at("courseId") != lesson.at("courseId")) ||
            // 【逐行批注】 执行当前声明或表达式：(!scope.value("topicId", "").empty() && scope.at("topicId") != lesson.value("topicId…。
            (!scope.value("topicId", "").empty() && scope.at("topicId") != lesson.value("topicId", "")))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课时与当前课程或知识点不匹配");。
            throw std::invalid_argument("课时与当前课程或知识点不匹配");
        // 【逐行批注】 结束当前函数并返回结果：return lesson;。
        return lesson;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
    const auto courseId = text(scope, "courseId"); requireCourse(db, access, courseId);
    // 【逐行批注】 计算右侧结果并写入 Json found，更新当前运行状态。
    Json found = Json::object(); const int phase = positiveIndex(scope, "phaseIndex"), topic = positiveIndex(scope, "topicIndex");
    // 【逐行批注】 计算右侧结果并写入 auto topicId，更新当前运行状态。
    const auto topicId = scope.value("topicId", "");
    // 【逐行批注】 判断条件 (!topicId.empty()) {，成立时执行对应分支。
    if (!topicId.empty()) {
        // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
        bool valid = false;
        // 【逐行批注】 计算右侧结果并写入 auto outline，更新当前运行状态。
        const auto outline = agentOutline(db, access, courseId);
        // 【逐行批注】 按循环规则重复处理：for (const auto& stage : outline.at("courseStructure"))。
        for (const auto& stage : outline.at("courseStructure"))
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : stage.at("topics")) valid = valid || item.at("id")…。
            for (const auto& item : stage.at("topics")) valid = valid || item.at("id") == topicId;
        // 【逐行批注】 判断条件 (!valid) throw std::invalid_argument("知识点已不在当前课程主线中，请从课程目录重新选择");，成立时执行对应分支。
        if (!valid) throw std::invalid_argument("知识点已不在当前课程主线中，请从课程目录重新选择");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.listClassroomActivities(courseId)) if (row.kind …。
    for (const auto& row : db.listClassroomActivities(courseId)) if (row.kind == "agent-lesson") {
        // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
        const auto lesson = parsed(row.payload);
        // 【逐行批注】 计算右侧结果并写入 bool matches，更新当前运行状态。
        const bool matches = !topicId.empty() ? lesson.value("topicId", "") == topicId : lesson.value("phaseIndex", 0) == phase && lesson.value("topicIndex", 0) == topic;
        // 【逐行批注】 判断条件 (lesson.value("scopeId", "") != access.scopeId || !matches || le…，成立时执行对应分支。
        if (lesson.value("scopeId", "") != access.scopeId || !matches || lesson.value("intent", "") != "advance") continue;
        // 【逐行批注】 判断条件 (found.empty() || lesson.value("updatedAt", "") > found.value("u…，成立时执行对应分支。
        if (found.empty() || lesson.value("updatedAt", "") > found.value("updatedAt", "")) found = lesson;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return found;。
    return found;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 recordMediaUsage，执行该函数声明的职责。
void recordMediaUsage(Database& db, const Json& task, Json entry) {
    // 【逐行批注】 计算右侧结果并写入 entry["taskId"]，更新当前运行状态。
    entry["taskId"] = task.at("id"); entry["at"] = libraryNow(); entry["purpose"] = "课堂教学图片";
    // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
    const auto id = "library-usage:" + libraryDigest(task.at("id").get<std::string>() + entry.dump() + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    // 【逐行批注】 判断条件 (!db.saveMyAIRecord({id, "library-usage", entry.dump(), libraryN…，成立时执行对应分支。
    if (!db.saveMyAIRecord({id, "library-usage", entry.dump(), libraryNow(), 1}, 0)) throw std::runtime_error("联网图片请求统计保存失败");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 publicContent，执行该函数声明的职责。
Json publicContent(const Json& lesson) {
    // 【逐行批注】 计算右侧结果并写入 content，更新当前运行状态。
    auto content = lesson.value("deepContent", Json::object());
    // 【逐行批注】 判断条件 (!content.is_object() || content.empty()) return nullptr;，成立时执行对应分支。
    if (!content.is_object() || content.empty()) return nullptr;
    // 【逐行批注】 按循环规则重复处理：for (auto& media : content["media"]) if (media.value("type", "") == "image…。
    for (auto& media : content["media"]) if (media.value("type", "") == "image") {
        // 【逐行批注】 执行当前声明或表达式：media.erase("blobId");。
        media.erase("blobId");
        // 【逐行批注】 判断条件 (media.value("status", "") == "ready") media["url"] = "/api/deep…，成立时执行对应分支。
        if (media.value("status", "") == "ready") media["url"] = "/api/deep-course/image?lessonId=" + urlPart(lesson.at("id")) + "&mediaId=" + urlPart(media.at("id")) + "&v=" + std::to_string(content.value("version", 1));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return content;。
    return content;
// 【逐行批注】 结束当前代码块。
}

Json submittedComponentRecovery(Database& db, const AgentAccess& access, const Json& lesson) {
    Json recovered = Json::object();
    if (!lesson.value("deepContent", Json()).is_object()) return recovered;
    const auto& content = lesson.at("deepContent");
    if (!content.value("version", Json()).is_number_integer() || !content.value("media", Json()).is_array()) return recovered;
    const int version = content.at("version").get<int>();
    const auto courseId = lesson.at("courseId").get<std::string>();
    const auto lessonId = lesson.at("id").get<std::string>();
    for (const auto& row : db.listInteractions()) {
        if (row.kind != "chat-user" || !row.courseId || *row.courseId != courseId) continue;
        const auto input = parsed(row.payload);
        if (input.value("scopeId", "") != access.scopeId || input.value("lessonId", "") != lessonId) continue;
        const auto activities = input.value("componentActivities", Json::object());
        if (!activities.is_object()) continue;
        for (const auto& entry : activities.items()) {
            try {
                const auto& activity = entry.value();
                if (!activity.is_object() || activity.value("componentId", "") != entry.key() ||
                    activity.value("lessonId", "") != lessonId || activity.value("inputSource", "") != "browser_report_validated" ||
                    activity.value("contentVersion", Json()) != version ||
                    !activity.value("events", Json()).is_array() || activity.at("events").empty()) continue;
                const auto pageId = activity.value("contentId", "");
                bool attached = false;
                for (const auto* view : {"lecture", "slides"}) {
                    const auto pages = content.value(view, Json::array());
                    if (!pages.is_array()) continue;
                    for (const auto& page : pages) {
                        const auto ids = page.value("mediaIds", Json::array());
                        if (page.value("id", "") == pageId && ids.is_array() &&
                            std::find(ids.begin(), ids.end(), entry.key()) != ids.end()) attached = true;
                    }
                }
                if (!attached) continue;
                const auto& state = activity.at("events").back().at("state");
                for (const auto& media : content.at("media")) {
                    if (media.value("id", "") != entry.key()) continue;
                    const auto type = media.value("type", "");
                    if (type == "sets") checkedSetObservation(media, state, version);
                    else if (isInteractiveLab(type)) checkedLabObservation(media, state, version);
                    else break;
                    recovered[entry.key()] = {{"contentVersion", version}, {"state", state},
                                              {"source", "submitted_component_activity"}};
                    break;
                }
            } catch (const std::exception&) {
                // 已损坏或不再匹配的历史活动不能作为恢复状态。
            }
        }
    }
    return recovered;
}

std::string componentDraftId(const AgentAccess& access, const Json& lesson, const std::string& componentId) {
    return "component-draft:" + resourceDigest(Json::array({access.scopeId, lesson.at("id"), componentId}).dump());
}

void validateComponentDraft(const Json& lesson, const Json& draft) {
    const auto componentId = text(draft, "componentId", 100);
    const auto state = draft.at("state"), trace = draft.at("trace");
    if (!trace.value("events", Json()).is_array() || trace.at("events").empty())
        throw std::invalid_argument("组件草稿没有可恢复的实际操作");
    const Json event = {{"requestId", "component-draft"}, {"deepView", text(draft, "deepView", 16)},
        {"contentId", text(draft, "contentId", 100)}, {"mediaState", {{componentId, state}}},
        {"componentActivities", {{componentId, trace}}}};
    checkedComponentActivities(lesson, event);
}

Json componentDrafts(Database& db, const AgentAccess& access, const Json& lesson) {
    Json result = Json::object();
    const auto content = lesson.value("deepContent", Json::object());
    if (!content.value("media", Json()).is_array()) return result;
    for (const auto& media : content.at("media")) {
        if (!media.value("id", Json()).is_string()) continue;
        const auto id = media.at("id").get<std::string>();
        const auto row = db.getClassroomActivity(componentDraftId(access, lesson, id));
        if (!row || row->kind != "agent-component-draft" || row->courseId != lesson.at("courseId")) continue;
        try {
            const auto draft = parsed(row->payload);
            if (draft.value("scopeId", "") != access.scopeId || draft.value("lessonId", "") != lesson.at("id") ||
                draft.value("componentId", "") != id || !draft.value("revision", Json()).is_number_integer() ||
                draft.at("revision").get<int>() < 1) continue;
            validateComponentDraft(lesson, draft);
            result[id] = {{"revision", draft.at("revision")}, {"contentVersion", draft.at("trace").at("contentVersion")},
                          {"state", draft.at("state")}, {"trace", draft.at("trace")}};
        } catch (const std::exception&) {
            // 已过期或损坏的草稿不能覆盖当前组件。
        }
    }
    return result;
}
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 deepCourseEnabled，执行该函数声明的职责。
bool deepCourseEnabled(Database& db, const std::string& courseId) {
    // 【逐行批注】 计算右侧结果并写入 auto values，更新当前运行状态。
    const auto values = db.findSnapshotsByCourseId(courseId);
    // 【逐行批注】 判断条件 (values.empty()) return false;，成立时执行对应分支。
    if (values.empty()) return false;
    // 【逐行批注】 计算右侧结果并写入 auto latest，更新当前运行状态。
    const auto latest = std::max_element(values.begin(), values.end(), [](const auto& a, const auto& b) { return a.version < b.version; });
    // 【逐行批注】 结束当前函数并返回结果：return parsed(latest->payload).value("pedagogy", "") == "deep-v1";。
    return parsed(latest->payload).value("pedagogy", "") == "deep-v1";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 normalizeDeepContent，执行该函数声明的职责。
Json normalizeDeepContent(const Json& input) {
    // 【逐行批注】 判断条件 (!input.is_object()) throw std::invalid_argument("深度课件须为对象");，成立时执行对应分支。
    if (!input.is_object()) throw std::invalid_argument("深度课件须为对象");
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = {{"version", 1}, {"lecture", Json::array()}, {"slides", Json::array()}, {"media", Json::array()}};
    // 【逐行批注】 执行当前声明或表达式：std::set<std::string> identities;。
    std::set<std::string> identities;
    // 【逐行批注】 按循环规则重复处理：for (const auto* kind : {"lecture", "slides"}) {。
    for (const auto* kind : {"lecture", "slides"}) {
        // 【逐行批注】 计算右侧结果并写入 auto blocks，更新当前运行状态。
        const auto blocks = input.value(kind, Json::array());
        // 【逐行批注】 判断条件 (!blocks.is_array() || blocks.size() > 80) throw std::invalid_ar…，成立时执行对应分支。
        if (!blocks.is_array() || blocks.size() > 80) throw std::invalid_argument("课件板块数量无效");
        // 【逐行批注】 按循环规则重复处理：for (const auto& block : blocks) {。
        for (const auto& block : blocks) {
            // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
            const auto id = text(block, "id", 100);
            // 【逐行批注】 判断条件 (!identities.insert(std::string(kind) + ":" + id).second) throw …，成立时执行对应分支。
            if (!identities.insert(std::string(kind) + ":" + id).second) throw std::invalid_argument("课件板块标识重复");
            // 【逐行批注】 计算右侧结果并写入 Json item，更新当前运行状态。
            Json item = {{"id", id}, {"title", text(block, "title", 500)}, {"body", text(block, "body")}, {"mediaIds", mediaIds(block)}};
            // 【逐行批注】 判断条件 (block.contains("sectionId")) item["sectionId"] = text(block, "s…，成立时执行对应分支。
            if (block.contains("sectionId")) item["sectionId"] = text(block, "sectionId", 160);
            // 【逐行批注】 判断条件 (block.contains("sourceSectionIds")) {，成立时执行对应分支。
            if (block.contains("sourceSectionIds")) {
                // 【逐行批注】 计算右侧结果并写入 auto ids，更新当前运行状态。
                const auto ids = block.at("sourceSectionIds");
                // 【逐行批注】 判断条件 (!ids.is_array() || ids.size() > 20) throw std::invalid_argument…，成立时执行对应分支。
                if (!ids.is_array() || ids.size() > 20) throw std::invalid_argument("课件来源板块无效");
                // 【逐行批注】 按循环规则重复处理：for (const auto& source : ids) if (!source.is_string()) throw std::invalid…。
                for (const auto& source : ids) if (!source.is_string()) throw std::invalid_argument("课件来源须为板块标识");
                // 【逐行批注】 计算右侧结果并写入 item["sourceSectionIds"]，更新当前运行状态。
                item["sourceSectionIds"] = ids;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 move 完成本行所需操作：result[kind].push_back(std::move(item));。
            result[kind].push_back(std::move(item));
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (result["lecture"].empty()) throw std::invalid_argument("讲义不能为空"…，成立时执行对应分支。
    if (result["lecture"].empty()) throw std::invalid_argument("讲义不能为空");
    // 【逐行批注】 计算右侧结果并写入 auto media，更新当前运行状态。
    const auto media = input.value("media", Json::array());
    // 【逐行批注】 判断条件 (!media.is_array() || media.size() > 32) throw std::invalid_argu…，成立时执行对应分支。
    if (!media.is_array() || media.size() > 32) throw std::invalid_argument("教学素材列表无效");
    // 【逐行批注】 执行当前声明或表达式：std::set<std::string> mediaNames;。
    std::set<std::string> mediaNames;
    // 【逐行批注】 按循环规则重复处理：for (const auto& source : media) {。
    for (const auto& source : media) {
        // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
        const auto id = text(source, "id", 100), type = text(source, "type", 40);
        // 【逐行批注】 判断条件 (!mediaNames.insert(id).second) throw std::invalid_argument("教学素…，成立时执行对应分支。
        if (!mediaNames.insert(id).second) throw std::invalid_argument("教学素材标识重复");
        // 【逐行批注】 计算右侧结果并写入 Json item，更新当前运行状态。
        Json item = {{"id", id}, {"type", type}, {"title", text(source, "title", 500)}, {"caption", source.value("caption", "")}, {"status", "ready"}};
        // 【逐行批注】 判断条件 (source.value("status", "ready") == "failed" || source.value("st…，成立时执行对应分支。
        if (source.value("status", "ready") == "failed" || source.value("status", "ready") == "pending") {
            // 【逐行批注】 计算右侧结果并写入 item["status"]，更新当前运行状态。
            item["status"] = source.at("status"); item["error"] = source.value("error", "该素材尚未完成，可继续学习讲义并重试。");
        // 【逐行批注】 执行当前声明或表达式：} else if (type == "timeline") {。
        } else if (type == "timeline") {
            // 【逐行批注】 计算右侧结果并写入 auto events，更新当前运行状态。
            const auto events = source.value("events", Json::array());
            // 【逐行批注】 判断条件 (!events.is_array() || events.empty() || events.size() > 100) th…，成立时执行对应分支。
            if (!events.is_array() || events.empty() || events.size() > 100) throw std::invalid_argument("时间轴事件无效");
            // 【逐行批注】 计算右侧结果并写入 item["events"]，更新当前运行状态。
            item["events"] = Json::array();
            // 【逐行批注】 按循环规则重复处理：for (const auto& event : events) item["events"].push_back({{"label", text(…。
            for (const auto& event : events) item["events"].push_back({{"label", text(event, "label", 300)}, {"text", text(event, "text", 12000)}});
        // 【逐行批注】 执行当前声明或表达式：} else if (type == "flowchart") {。
        } else if (type == "flowchart") {
            // 【逐行批注】 计算右侧结果并写入 auto nodes，更新当前运行状态。
            const auto nodes = source.value("nodes", Json::array()), edges = source.value("edges", Json::array());
            // 【逐行批注】 判断条件 (!nodes.is_array() || nodes.empty() || nodes.size() > 60 || !edg…，成立时执行对应分支。
            if (!nodes.is_array() || nodes.empty() || nodes.size() > 60 || !edges.is_array() || edges.size() > 100) throw std::invalid_argument("流程图节点或连线无效");
            // 【逐行批注】 调用 array 完成本行所需操作：std::set<std::string> names; item["nodes"] = Json::array(); item["edg…。
            std::set<std::string> names; item["nodes"] = Json::array(); item["edges"] = Json::array();
            // 【逐行批注】 按循环规则重复处理：for (const auto& node : nodes) { const auto name = text(node, "id", 100); …。
            for (const auto& node : nodes) { const auto name = text(node, "id", 100); if (!names.insert(name).second) throw std::invalid_argument("流程图节点重复"); item["nodes"].push_back({{"id", name}, {"label", text(node, "label", 1000)}}); }
            // 【逐行批注】 按循环规则重复处理：for (const auto& edge : edges) { const auto from = text(edge, "from", 100)…。
            for (const auto& edge : edges) { const auto from = text(edge, "from", 100), to = text(edge, "to", 100); if (!names.count(from) || !names.count(to)) throw std::invalid_argument("流程图连线指向不存在的节点"); item["edges"].push_back({{"from", from}, {"to", to}, {"label", edge.value("label", "")}}); }
        // 【逐行批注】 执行当前声明或表达式：} else if (type == "function") {。
        } else if (type == "function") {
            // 【逐行批注】 计算右侧结果并写入 auto coefficients，更新当前运行状态。
            const auto coefficients = source.value("coefficients", Json::array());
            // 【逐行批注】 判断条件 (!coefficients.is_array() || coefficients.size() != 3) throw std…，成立时执行对应分支。
            if (!coefficients.is_array() || coefficients.size() != 3) throw std::invalid_argument("函数图使用三个二次多项式系数");
            // 【逐行批注】 按循环规则重复处理：for (const auto& coefficient : coefficients) if (!coefficient.is_number() …。
            for (const auto& coefficient : coefficients) if (!coefficient.is_number() || !std::isfinite(coefficient.get<double>()) || std::abs(coefficient.get<double>()) > 100000) throw std::invalid_argument("函数系数无效");
            // 【逐行批注】 计算右侧结果并写入 item["coefficients"]，更新当前运行状态。
            item["coefficients"] = coefficients; item["xMin"] = number(source, "xMin", -10, -10000, 10000); item["xMax"] = number(source, "xMax", 10, -10000, 10000);
            // 【逐行批注】 判断条件 (item["xMin"].get<double>() >= item["xMax"].get<double>()) throw…，成立时执行对应分支。
            if (item["xMin"].get<double>() >= item["xMax"].get<double>()) throw std::invalid_argument("函数横轴范围无效");
            // 【逐行批注】 计算右侧结果并写入 auto parameter，更新当前运行状态。
            const auto parameter = source.value("parameter", Json::object());
            // 【逐行批注】 判断条件 (!parameter.empty()) {，成立时执行对应分支。
            if (!parameter.empty()) {
                // 【逐行批注】 计算右侧结果并写入 auto index，更新当前运行状态。
                const auto index = parameter.value("coefficientIndex", 0);
                // 【逐行批注】 判断条件 (index < 0 || index > 2) throw std::invalid_argument("可调函数系数标识无效…，成立时执行对应分支。
                if (index < 0 || index > 2) throw std::invalid_argument("可调函数系数标识无效");
                // 【逐行批注】 计算右侧结果并写入 auto minimum，更新当前运行状态。
                const auto minimum = number(parameter, "min", -5, -100000, 100000), maximum = number(parameter, "max", 5, -100000, 100000);
                // 【逐行批注】 判断条件 (minimum >= maximum) throw std::invalid_argument("参数范围无效");，成立时执行对应分支。
                if (minimum >= maximum) throw std::invalid_argument("参数范围无效");
                // 【逐行批注】 计算右侧结果并写入 item["parameter"]，更新当前运行状态。
                item["parameter"] = {{"name", text(parameter, "name", 100)}, {"min", minimum}, {"max", maximum}, {"step", number(parameter, "step", .1, .0001, 10000)}, {"value", number(parameter, "value", coefficients.at(index).get<double>(), minimum, maximum)}, {"coefficientIndex", index}};
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 执行当前声明或表达式：} else if (type == "forces") {。
        } else if (isInteractiveLab(type)) {
            const auto lab = checkedLabConfig(source);
            for (const auto& field : lab.items()) item[field.key()] = field.value();
        } else if (type == "sets") {
            const auto sets = checkedSetConfig(source);
            for (const auto& field : sets.items()) item[field.key()] = field.value();
        } else if (type == "forces") {
            // 【逐行批注】 计算右侧结果并写入 item["mass"]，更新当前运行状态。
            item["mass"] = number(source, "mass", 1, .001, 100000); item["force"] = number(source, "force", 10, -100000, 100000); item["friction"] = number(source, "friction", 0, 0, 100000); item["gravity"] = number(source, "gravity", 9.8, 0, 1000);
        // 【逐行批注】 执行当前声明或表达式：} else if (type == "image") {。
        } else if (type == "image") {
            // 图片地址仅来自已经检索并校验的本课图片缓存，模型不能自造远程地址。
            // 【逐行批注】 计算右侧结果并写入 item["status"]，更新当前运行状态。
            item["status"] = "pending"; item["error"] = "图片等待联网检索或重新加载。";
        // 【逐行批注】 调用 invalid_argument 完成本行所需操作：} else throw std::invalid_argument("教学图示类型暂不支持，请使用时间轴、流程图、函数、受力或联网图片"…。
        } else throw std::invalid_argument("教学图示类型暂不支持，请使用时间轴、流程图、函数、受力或联网图片");
        // 【逐行批注】 调用 move 完成本行所需操作：result["media"].push_back(std::move(item));。
        result["media"].push_back(std::move(item));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto* kind : {"lecture", "slides"}) for (const auto& block : re…。
    for (const auto* kind : {"lecture", "slides"}) for (const auto& block : result[kind]) for (const auto& id : block.at("mediaIds"))
        // 【逐行批注】 判断条件 (!mediaNames.count(id.get<std::string>())) throw std::invalid_ar…，成立时执行对应分支。
        if (!mediaNames.count(id.get<std::string>())) throw std::invalid_argument("课件引用了不存在的素材标识");
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 deepImageCandidates，执行该函数声明的职责。
Json deepImageCandidates(const std::string& html, const std::string& pageUrl, const std::string& query, const std::string& title) {
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = Json::array(); std::map<std::string, size_t> seen;
    // 【逐行批注】 调用 tags 完成本行所需操作：const std::regex tags("<(?:meta|img)\\b[^>]{1,4000}>", std::regex::ic…。
    const std::regex tags("<(?:meta|img)\\b[^>]{1,4000}>", std::regex::icase);
    // 【逐行批注】 计算右侧结果并写入 auto add，更新当前运行状态。
    const auto add = [&](const std::string& raw, const std::string& alt, const std::string& caption, bool metadata) {
        // 【逐行批注】 计算右侧结果并写入 auto url，更新当前运行状态。
        const auto url = resolveURL(pageUrl, raw); auto lower = url;
        // 【逐行批注】 调用 transform、tolower 完成本行所需操作：std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned…。
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        // 【逐行批注】 判断条件 (url.empty() || lower.find("logo") != std::string::npos || lower…，成立时执行对应分支。
        if (url.empty() || lower.find("logo") != std::string::npos || lower.find("avatar") != std::string::npos || lower.find("favicon") != std::string::npos) return;
        // 【逐行批注】 判断条件 (seen.count(url)) {，成立时执行对应分支。
        if (seen.count(url)) {
            // 【逐行批注】 计算右侧结果并写入 auto& previous，更新当前运行状态。
            auto& previous = result.at(seen.at(url));
            // 【逐行批注】 判断条件 (!alt.empty()) previous["alt"] = alt;，成立时执行对应分支。
            if (!alt.empty()) previous["alt"] = alt;
            // 【逐行批注】 判断条件 (!caption.empty()) previous["sourceCaption"] = caption;，成立时执行对应分支。
            if (!caption.empty()) previous["sourceCaption"] = caption;
            // 【逐行批注】 判断条件 (!metadata) previous["metadata"] = false;，成立时执行对应分支。
            if (!metadata) previous["metadata"] = false;
            // 【逐行批注】 结束当前函数并返回结果：return;。
            return;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 seen[url]，更新当前运行状态。
        seen[url] = result.size();
        // 【逐行批注】 执行当前声明或表达式：result.push_back({{"url", url}, {"alt", alt}, {"sourceCaption", caption}, {"metadata…。
        result.push_back({{"url", url}, {"alt", alt}, {"sourceCaption", caption}, {"metadata", metadata}});
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto limited，更新当前运行状态。
    const auto limited = html.substr(0, 1024 * 1024), lower = lowerText(limited);
    // 【逐行批注】 计算右侧结果并写入 int inspected，更新当前运行状态。
    int inspected = 0;
    // 【逐行批注】 按循环规则重复处理：for (auto iterator = std::sregex_iterator(limited.begin(), limited.end(), …。
    for (auto iterator = std::sregex_iterator(limited.begin(), limited.end(), tags); iterator != std::sregex_iterator() && inspected++ < 160; ++iterator) {
        // 【逐行批注】 计算右侧结果并写入 auto tag，更新当前运行状态。
        const auto tag = iterator->str(), property = attribute(tag, "property"), name = attribute(tag, "name");
        // 【逐行批注】 判断条件 (property == "og:image" || name == "twitter:image") add(attribut…，成立时执行对应分支。
        if (property == "og:image" || name == "twitter:image") add(attribute(tag, "content"), "", "", true);
        // 【逐行批注】 在前一条件不成立时继续判断：(lowerText(tag).compare(0, 4, "<img") == 0) {。
        else if (lowerText(tag).compare(0, 4, "<img") == 0) {
            // 【逐行批注】 计算右侧结果并写入 auto alt，更新当前运行状态。
            const auto alt = attribute(tag, "alt");
            // 【逐行批注】 计算右侧结果并写入 auto caption，更新当前运行状态。
            const auto caption = imageFigureCaption(limited, lower, static_cast<size_t>(iterator->position()));
            // 【逐行批注】 判断条件 (!alt.empty() || !caption.empty()) add(attribute(tag, "src"), al…，成立时执行对应分支。
            if (!alt.empty() || !caption.empty()) add(attribute(tag, "src"), alt, caption, false);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto terms，更新当前运行状态。
    const auto terms = imageTerms(query + " " + title);
    // 【逐行批注】 按循环规则重复处理：for (auto& candidate : result) {。
    for (auto& candidate : result) {
        // 【逐行批注】 计算右侧结果并写入 auto alt，更新当前运行状态。
        const auto alt = imageTerms(candidate.at("alt")), caption = imageTerms(candidate.at("sourceCaption"));
        // 只计图片自身路径，不借来源页标题给无关配图增加相关度。
        // 【逐行批注】 计算右侧结果并写入 auto url，更新当前运行状态。
        const auto url = candidate.at("url").get<std::string>();
        // 【逐行批注】 计算右侧结果并写入 auto start，更新当前运行状态。
        const auto start = url.find('/', url.find("://") + 3);
        // 【逐行批注】 计算右侧结果并写入 auto path，更新当前运行状态。
        const auto path = imageTerms(start == std::string::npos ? "" : url.substr(start));
        // 【逐行批注】 计算右侧结果并写入 int score，更新当前运行状态。
        int score = 0; Json matches = Json::array();
        // 【逐行批注】 按循环规则重复处理：for (const auto& term : terms) if (alt.count(term) || caption.count(term) …。
        for (const auto& term : terms) if (alt.count(term) || caption.count(term) || path.count(term)) {
            // 【逐行批注】 计算右侧结果并写入 score +，更新当前运行状态。
            score += alt.count(term) ? 5 : caption.count(term) ? 3 : 1; matches.push_back(term);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 auto decoration，更新当前运行状态。
        const auto decoration = lowerText(candidate.at("url").get<std::string>() + " " + candidate.at("alt").get<std::string>());
        // 【逐行批注】 判断条件 (std::regex_search(decoration, std::regex("(^|[^a-z])(cover|hero…，成立时执行对应分支。
        if (std::regex_search(decoration, std::regex("(^|[^a-z])(cover|hero|banner|thumbnail|icon)([^a-z]|$)"))) score -= 6;
        // 【逐行批注】 判断条件 (candidate.at("metadata") == true) score -= 3;，成立时执行对应分支。
        if (candidate.at("metadata") == true) score -= 3;
        // 【逐行批注】 计算右侧结果并写入 candidate.erase("metadata"); candidate["rele…，更新当前运行状态。
        candidate.erase("metadata"); candidate["relevanceScore"] = score; candidate["textMatches"] = matches;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!query.empty() || !title.empty()) {，成立时执行对应分支。
    if (!query.empty() || !title.empty()) {
        // 【逐行批注】 调用 stable_sort 完成本行所需操作：std::stable_sort(result.begin(), result.end(), [](const auto& a, cons…。
        std::stable_sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.at("relevanceScore").template get<int>() > b.at("relevanceScore").template get<int>(); });
        // 【逐行批注】 调用 remove_if 完成本行所需操作：result.erase(std::remove_if(result.begin(), result.end(), [](const au…。
        result.erase(std::remove_if(result.begin(), result.end(), [](const auto& item) { return item.at("relevanceScore").template get<int>() <= 0; }), result.end());
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (result.size() > 8) result.erase(result.begin() + 8, result.end(…，成立时执行对应分支。
    if (result.size() > 8) result.erase(result.begin() + 8, result.end());
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 deepCourseView，执行该函数声明的职责。
Json deepCourseView(Database& db, const AgentAccess& access, const Json& scope) {
    // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
    const auto lesson = locateLesson(db, access, scope);
    // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
    const auto courseId = lesson.empty() ? text(scope, "courseId") : lesson.at("courseId").get<std::string>();
    // 【逐行批注】 调用 requireCourse 完成本行所需操作：requireCourse(db, access, courseId);。
    requireCourse(db, access, courseId);
    // 【逐行批注】 判断条件 (!deepCourseEnabled(db, courseId)) return {{"enabled", false}, {…，成立时执行对应分支。
    if (!deepCourseEnabled(db, courseId)) return {{"enabled", false}, {"status", "legacy"}};
    // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
    const auto course = db.getCourse(courseId);
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = {{"enabled", true}, {"status", lesson.empty() ? "unprepared" : lesson.value("status", "draft")}, {"course", {{"id", courseId}, {"title", course->title}, {"goal", course->goal}, {"pedagogy", "deep-v1"}}}, {"outline", agentOutline(db, access, courseId)}, {"lesson", nullptr}, {"content", nullptr}};
    // 【逐行批注】 判断条件 (!lesson.empty()) { result["lesson"] = agentLessonView(db, acces…，成立时执行对应分支。
    if (!lesson.empty()) {
        result["lesson"] = agentLessonView(db, access, lesson.at("id"));
        result["content"] = publicContent(lesson);
        result["componentRecovery"] = submittedComponentRecovery(db, access, lesson);
        result["componentDrafts"] = componentDrafts(db, access, lesson);
    }
    // 页面在任务刚提交、尚未创建课时时也能按课程/知识点重新找到真实任务。
    // 【逐行批注】 计算右侧结果并写入 result["activeTask"]，更新当前运行状态。
    result["activeTask"] = agentDeepPreparationView(db, access, scope);
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

Json deepCourseSaveComponentDraft(Database& db, const AgentAccess& access, const Json& body) {
    if (!body.is_object() || body.dump().size() > 160000 || !body.value("expectedRevision", Json()).is_number_integer())
        throw std::invalid_argument("组件草稿格式或版本无效");
    const auto expected = body.at("expectedRevision").get<long long>();
    if (expected < 0 || expected > 1000000) throw std::invalid_argument("组件草稿版本无效");
    Json result;
    db.transaction([&] {
        const auto lesson = savedLesson(db, access, text(body, "lessonId", 100));
        if (lesson.value("status", "") != "ready") throw std::invalid_argument("课时尚未准备好");
        const auto componentId = text(body, "componentId", 100);
        validateComponentDraft(lesson, body);
        const auto id = componentDraftId(access, lesson, componentId);
        const auto row = db.getClassroomActivity(id);
        if (row && (row->kind != "agent-component-draft" || row->courseId != lesson.at("courseId")))
            throw std::invalid_argument("组件草稿归属无效");
        const auto previous = row ? parsed(row->payload) : Json::object();
        const auto contentVersion = body.at("trace").at("contentVersion");
        const bool current = previous.value("scopeId", "") == access.scopeId &&
            previous.value("lessonId", "") == lesson.at("id") && previous.value("componentId", "") == componentId &&
            previous.value("trace", Json::object()).value("contentVersion", Json()) == contentVersion;
        const auto revision = current ? previous.value("revision", 0) : 0;
        if (current && previous.value("state", Json()) == body.at("state") && previous.value("trace", Json()) == body.at("trace") &&
            previous.value("deepView", "") == body.at("deepView") && previous.value("contentId", "") == body.at("contentId")) {
            result = {{"revision", revision}, {"unchanged", true}};
            return;
        }
        if (expected != revision) throw std::invalid_argument("组件探索已在其他窗口更新，本地草稿已保留，请刷新后核对。");
        if (current && previous.at("trace").value("sessionId", "") == body.at("trace").value("sessionId", "")) {
            const auto& oldTrace = previous.at("trace");
            const auto& newTrace = body.at("trace");
            if (newTrace.at("totalChanges").get<long long>() < oldTrace.at("totalChanges").get<long long>())
                throw std::invalid_argument("组件探索轨迹不能回退");
            for (const auto& oldEvent : oldTrace.at("events")) for (const auto& newEvent : newTrace.at("events"))
                if (oldEvent.at("seq") == newEvent.at("seq") && oldEvent != newEvent)
                    throw std::invalid_argument("已保存的组件探索轨迹不能更改");
        }
        const Json draft = {{"scopeId", access.scopeId}, {"lessonId", lesson.at("id")}, {"componentId", componentId},
            {"deepView", body.at("deepView")}, {"contentId", body.at("contentId")}, {"state", body.at("state")},
            {"trace", body.at("trace")}, {"revision", revision + 1}};
        if (!db.upsert(ClassroomActivity{id, lesson.at("courseId"), "agent-component-draft", draft.dump(), libraryNow(), 0, 0}))
            throw std::runtime_error("组件探索保存失败，本地草稿仍保留");
        result = {{"revision", revision + 1}, {"unchanged", false}};
    });
    return result;
}

// 【逐行批注】 定义函数 deepCourseStart，执行该函数声明的职责。
Json deepCourseStart(Database& db, const AgentAccess& access, const Json& scope) {
    // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
    const auto lesson = locateLesson(db, access, scope);
    // 【逐行批注】 判断条件 (!lesson.empty()) {，成立时执行对应分支。
    if (!lesson.empty()) {
        // 【逐行批注】 判断条件 (!deepCourseEnabled(db, lesson.at("courseId"))) throw std::inval…，成立时执行对应分支。
        if (!deepCourseEnabled(db, lesson.at("courseId"))) throw std::invalid_argument("该课使用快速规划课堂");
        // 【逐行批注】 判断条件 (lesson.value("status", "") == "ready") {，成立时执行对应分支。
        if (lesson.value("status", "") == "ready") {
            // 【逐行批注】 调用 agentEnterLesson 完成本行所需操作：agentEnterLesson(db, access, {{"lessonId", lesson.at("id")}});。
            agentEnterLesson(db, access, {{"lessonId", lesson.at("id")}});
            // 【逐行批注】 结束当前函数并返回结果：return {{"lessonId", lesson.at("id")}, {"href", "/deep-course?lessonI…。
            return {{"lessonId", lesson.at("id")}, {"href", "/deep-course?lessonId=" + lesson.at("id").get<std::string>()}, {"cached", true}};
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 auto taskId，更新当前运行状态。
        const auto taskId = lesson.value("taskId", "");
        // 【逐行批注】 判断条件 (!taskId.empty()) {，成立时执行对应分支。
        if (!taskId.empty()) {
            // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
            const auto current = agentView(db, access, taskId);
            // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
            const auto row = db.getClassroomActivity(taskId);
            // 【逐行批注】 计算右侧结果并写入 auto owner，更新当前运行状态。
            const auto owner = row ? parsed(row->payload) : Json::object();
            // 【逐行批注】 计算右侧结果并写入 auto ownerEvent，更新当前运行状态。
            const auto ownerEvent = owner.value("event", Json::object());
            // 【逐行批注】 计算右侧结果并写入 bool unfinishedPlanning，更新当前运行状态。
            const bool unfinishedPlanning = ownerEvent.value("type", "") == "plan_course" && ownerEvent.value("mode", "") == "deep";
            // 【逐行批注】 判断条件 (current.value("status", "") == "failed" || current.value("statu…，成立时执行对应分支。
            if (current.value("status", "") == "failed" || current.value("status", "") == "cancelled" ||
                // 【逐行批注】 执行当前声明或表达式：current.value("status", "") == "superseded")。
                current.value("status", "") == "superseded")
                // 【逐行批注】 结束当前函数并返回结果：return {{"task", agentControl(db, access, {{"command", "retry"}, {"ta…。
                return {{"task", agentControl(db, access, {{"command", "retry"}, {"taskId", taskId}})}};
            // 【逐行批注】 判断条件 (!unfinishedPlanning || (current.value("status", "") != "ready" …，成立时执行对应分支。
            if (!unfinishedPlanning || (current.value("status", "") != "ready" && current.value("status", "") != "failed" &&
                // 【逐行批注】 执行当前声明或表达式：current.value("status", "") != "cancelled" && current.value("status", "") != "supers…。
                current.value("status", "") != "cancelled" && current.value("status", "") != "superseded")) return {{"task", current}};
            // 【逐行批注】 判断条件 (lesson.value("sourceLearningVersion", -1) != db.learningRevisio…，成立时执行对应分支。
            if (lesson.value("sourceLearningVersion", -1) != db.learningRevision())
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("已有备课草稿对应的学习情况已变化，请重新确认课程主线后继续");。
                throw std::invalid_argument("已有备课草稿对应的学习情况已变化，请重新确认课程主线后继续");
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
    const auto courseId = lesson.empty() ? text(scope, "courseId") : lesson.at("courseId").get<std::string>(); requireCourse(db, access, courseId);
    // 【逐行批注】 判断条件 (!deepCourseEnabled(db, courseId)) throw std::invalid_argument("…，成立时执行对应分支。
    if (!deepCourseEnabled(db, courseId)) throw std::invalid_argument("该课使用快速规划课堂");
    // 【逐行批注】 计算右侧结果并写入 auto outline，更新当前运行状态。
    const auto outline = agentOutline(db, access, courseId); const int phase = positiveIndex(scope, "phaseIndex"), topicIndex = positiveIndex(scope, "topicIndex");
    // 【逐行批注】 执行当前声明或表达式：Json selected;。
    Json selected;
    // 【逐行批注】 计算右侧结果并写入 auto topicId，更新当前运行状态。
    const auto topicId = lesson.empty() ? scope.value("topicId", "") : lesson.value("topicId", "");
    // 【逐行批注】 按循环规则重复处理：for (const auto& stage : outline.at("courseStructure")) for (const auto& t…。
    for (const auto& stage : outline.at("courseStructure")) for (const auto& topic : stage.at("topics"))
        // 【逐行批注】 判断条件 ((!topicId.empty() && topic.at("id") == topicId) ||，成立时执行对应分支。
        if ((!topicId.empty() && topic.at("id") == topicId) ||
            // 【逐行批注】 执行当前声明或表达式：(topicId.empty() && topic.value("legacyPhaseIndex", 0) == phase && topic.value("lega…。
            (topicId.empty() && topic.value("legacyPhaseIndex", 0) == phase && topic.value("legacyTopicIndex", 0) == topicIndex)) selected = topic;
    // 【逐行批注】 判断条件 (selected.empty()) throw std::invalid_argument("知识点不存在，请返回课程目录选择…，成立时执行对应分支。
    if (selected.empty()) throw std::invalid_argument("知识点不存在，请返回课程目录选择");
    // 同一知识点在课时尚未创建前也使用固定请求身份，跨窗口点击不重复备课。
    // 【逐行批注】 计算右侧结果并写入 auto requestId，更新当前运行状态。
    const auto requestId = "deep-prepare:" + libraryDigest(courseId + ":" + selected.at("id").get<std::string>() + (lesson.empty() ? "" : ":resume:" + lesson.at("id").get<std::string>()));
    // 【逐行批注】 计算右侧结果并写入 Json event，更新当前运行状态。
    Json event = {{"type", "prepare_next"}, {"courseId", courseId}, {"topicId", selected.at("id")}, {"phaseIndex", selected.at("legacyPhaseIndex")}, {"topicIndex", selected.at("legacyTopicIndex")}, {"requestId", requestId}, {"navigate", false}, {"text", "请按当前课程主线，为指定topicId准备这一课的完整深度讲义与翻页课件；用create_lesson(intent=advance)保存稳定知识点。按教学需要选择图片、时间轴或互动图，不要求每课配齐。先保存可用内容，媒体失败单独保留重试状态；不更改课程主线，不替我作答。"}};
    // 【逐行批注】 判断条件 (!lesson.empty()) {，成立时执行对应分支。
    if (!lesson.empty()) {
        // 【逐行批注】 计算右侧结果并写入 event["lessonId"]，更新当前运行状态。
        event["lessonId"] = lesson.at("id"); event["resumeDraft"] = true;
        // 【逐行批注】 计算右侧结果并写入 event["text"]，更新当前运行状态。
        event["text"] = "继续本次事件lessonId指定的已保存深度课时草稿，不创建课程或新课时，不重复已有讲解与题目。先核对现有板块，使用本课真实sectionId补充尚未保存的set_deep_content讲义、课件和适用图示，再finish_lesson。保留已有教学内容与私有题目标准，不替学生作答。";
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return {{"task", agentSubmit(db, access, event)}};。
    return {{"task", agentSubmit(db, access, event)}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 deepCourseRetryMedia，执行该函数声明的职责。
Json deepCourseRetryMedia(Database& db, const AgentAccess& access, const Json& scope) {
    // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
    const auto lesson = savedLesson(db, access, text(scope, "lessonId"));
    // 【逐行批注】 计算右侧结果并写入 auto mediaId，更新当前运行状态。
    const auto mediaId = text(scope, "mediaId", 100);
    // 【逐行批注】 计算右侧结果并写入 bool found，更新当前运行状态。
    bool found = false; const bool repair = scope.value("repair", false);
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : lesson.value("deepContent", Json::object()).value(…。
    for (const auto& item : lesson.value("deepContent", Json::object()).value("media", Json::array())) if (item.at("id") == mediaId) {
        // 【逐行批注】 判断条件 (repair && item.value("type", "") != "image") throw std::invalid…，成立时执行对应分支。
        if (repair && item.value("type", "") != "image") throw std::invalid_argument("图片修复只适用于联网图片");
        // 【逐行批注】 判断条件 (item.value("status", "") == "ready" && !repair) return {{"cache…，成立时执行对应分支。
        if (item.value("status", "") == "ready" && !repair) return {{"cached", true}, {"lessonId", lesson.at("id")}};
        // 【逐行批注】 计算右侧结果并写入 found，更新当前运行状态。
        found = true;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!found) throw std::invalid_argument("素材不属于当前课时");，成立时执行对应分支。
    if (!found) throw std::invalid_argument("素材不属于当前课时");
    // 【逐行批注】 结束当前函数并返回结果：return {{"task", agentSubmit(db, access, {{"type", "chat"}, {"courseI…。
    return {{"task", agentSubmit(db, access, {{"type", "chat"}, {"courseId", lesson.at("courseId")}, {"lessonId", lesson.at("id")}, {"view", "learn"}, {"requestId", text(scope, "requestId", 160)}, {"deepMediaRetry", mediaId}, {"deepImageRepair", repair}, {"text", "只重试本课素材 " + mediaId + "，保留已经生成的讲义、课件和成功素材。图片请调用search_lesson_images指定mediaId；本次图片修复会重新获取实际图片缓存。其他图示修正后调用set_deep_content，避免重复生成整课。此请求不是学生作答，不评分。"}})}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 deepCourseImage，执行该函数声明的职责。
std::pair<std::string, std::string> deepCourseImage(Database& db, const AgentAccess& access, const std::string& lessonId, const std::string& mediaId) {
    // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
    const auto lesson = savedLesson(db, access, lessonId);
    // 【逐行批注】 按循环规则重复处理：for (const auto& media : lesson.value("deepContent", Json::object()).value…。
    for (const auto& media : lesson.value("deepContent", Json::object()).value("media", Json::array())) if (media.value("id", "") == mediaId && media.value("type", "") == "image" && media.value("status", "") == "ready") {
        // 【逐行批注】 计算右侧结果并写入 auto image，更新当前运行状态。
        const auto image = db.myAIRecord(media.value("blobId", ""));
        // 【逐行批注】 判断条件 (!image || image->kind != "deep-image") throw std::invalid_argum…，成立时执行对应分支。
        if (!image || image->kind != "deep-image") throw std::invalid_argument("图片缓存不存在，请重试素材");
        // 【逐行批注】 计算右侧结果并写入 auto data，更新当前运行状态。
        const auto data = parsed(image->payload);
        // 【逐行批注】 判断条件 (data.value("courseId", "") != lesson.at("courseId")) throw std:…，成立时执行对应分支。
        if (data.value("courseId", "") != lesson.at("courseId")) throw std::invalid_argument("图片不属于本课");
        // 【逐行批注】 计算右侧结果并写入 auto bytes，更新当前运行状态。
        const auto bytes = decode64(data.at("data"));
        // 【逐行批注】 判断条件 (imageMime(bytes).empty()) throw std::invalid_argument("图片缓存格式无效…，成立时执行对应分支。
        if (imageMime(bytes).empty()) throw std::invalid_argument("图片缓存格式无效");
        // 【逐行批注】 结束当前函数并返回结果：return {imageMime(bytes), bytes};。
        return {imageMime(bytes), bytes};
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("图片不存在或尚未完成");。
    throw std::invalid_argument("图片不存在或尚未完成");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 registerDeepCourseTools，执行该函数声明的职责。
void registerDeepCourseTools(LearningAgent& agent) {
    agent.registerTool("read_interactive_capabilities", {u8"读取实际注册组件的用途、合法默认配置、字段范围、资料限制和活动回传能力，无副作用。生成组件前若能力或参数不清楚，先读此工具；不能假设同一学科全部能力都已实现。参数types可选：最多10个类型名，省略返回全部。示例仅示范合法参数，不是预生成课程或题库。", true,
        [](Database&, const Json& args, const Json&, const AIResult&) {
            auto result = interactiveCapabilities();
            if (args.contains("types")) {
                if (!args.at("types").is_array() || args.at("types").empty() || args.at("types").size() > 10)
                    throw std::invalid_argument("组件能力查询需要1至10个类型");
                Json selected = Json::array(); std::set<std::string> seen;
                for (const auto& type : args.at("types")) {
                    if (!type.is_string() || !seen.insert(type.get<std::string>()).second) throw std::invalid_argument("组件类型须为不重复文字");
                    bool found = false;
                    for (const auto& item : result.at("components")) if (item.at("type") == type) { selected.push_back(item); found = true; }
                    if (!found) throw std::invalid_argument("组件类型尚未注册");
                }
                result["components"] = selected;
            }
            return PreparedAgentTool{result,{}};
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("set_deep_content", {u8"保存当前深度课时的真实AI讲义、翻页课件与受控图示。参数 lessonId,con…。
    agent.registerTool("set_deep_content", {u8"保存当前深度课时的真实AI讲义、翻页课件与受控图示。参数 lessonId,content:{lecture:[{sectionId,mediaIds:[]}],slides:[{id,title,sourceSectionIds:[],mediaIds:[]}],media:[{id,type,title,caption,...}]}。讲义的sectionId必须使用append_section返回的本课explanation板块ID，标题正文由程序读取已保存原文；课件sourceSectionIds引用本课已保存公开讲解或小结，正文由这些板块组合。先保存完整真实讲解再组织课件，不能假借私有题目或另一课时的板块。课件按教学需要组织而非切碎长文。图示类型及字段：cell_lab.cellType:plant|animal,selectedPart:membrane|cytoplasm|nucleus|mitochondrion，plant另有wall|chloroplast|vacuole，用于典型真核细胞结构功能探索，非比例图；history_lab.datasetId:apollo11,order:[launch,landing,return的一个排列],selectedEvent:三事件之一，使用NASA核对的日期，不能自造历史数据集或把时间先后当因果；climate_lab.datasetId:illustrative_north,month/compareMonth:1至12整数，仅自编教学数据，不冒充真实城市；annotation_lab.fragments:[2至16个本课原文片段，每项最多240字节，总计2400字节],marks:[同数量0至3整数，0未标注/1意象/2表达/3疑问]用于学生阅读标注，无唯一标准；annotation_lab可选mode:selection，原文为fragments直接拼接，selections:[{start,end,label:1至3,reason}]最多16条；start/end采用Unicode码点索引，含start不含end，reason最多480字节。该模式状态只提交selections，服务器从原文重新截取引文，允许交叠与不同解读，不自动评分；sentence_lab.fragments同上,roles:[同数量0至4整数，0未标注/1主语/2谓语动词/3宾语/4修饰或状语]用于句法分析；algorithm_lab.values:[2至8项0至99整数]，algorithm:bubble_sort(默认)|binary_search；默认排序的step为0至n*(n-1)/2；二分查找须提供升序values和target:0至99整数，step限实际查找路径内，重复值只返回本次命中索引；只接受步数状态，结果由程序重算，不执行任意代码；这六种组件仅在符合当前目标时选择，均为探索不评分。statistics_lab.values:[1至32项-100至100的数，步长0.5，重复项保留],varianceBasis:population(默认，除以n)|sample(除以n-1，n=1无定义)。探索平均数、中位数、众数、极差和方差，服务端重算并保留精确分数；所有值只出现一次时此处记为无众数；学生状态只含values、varianceBasis和版本。仅有限单变量描述统计，不进行总体推断或自动评分；equation_lab.left/right:[一次项系数,常数]，均为-20至20整数，默认[2,3]=[1,7]即2x+3=x+7；学生状态仅steps:[{op:add_x|add_constant|multiply|divide,value:-10至10整数}]和版本，最多12步，同乘同除必须非零。服务端从原方程重放并保留精确分数，每步分子分母限10^6，区分唯一解/无解/恒等式。不得替换原题、自报解或成绩，不支持非线性方程及不等式；vector_lab.vectorA/vectorB:[x,y]，坐标-10至10且步长0.5；scalar:-3至3且步长0.5；operation:add(默认)|subtract|scale_a。计算平面向量加减、实数倍、长度、数量积和夹角，含零向量时夹角及方向关系为null，不支持空间向量；状态只含两个向量、scalar和operation，不得自报计算结果或成绩；number_line.lower/upper:-20至20且步长0.5，lower为null代表负无穷，upper为null代表正无穷；lowerClosed/upperClosed为布尔值，无穷端必须false，有限左端默认开放、有限右端默认闭合；point:-20至20且步长0.5。左端不能大于右端，相等时两端都闭合才是单点，其他为空集；探索端点、开闭及观察点归属，不支持多区间运算；function_lab.coefficients:[a,b,c]（每个-10到10，步长0.5）,xMin/xMax（-20到20）只控制初始视窗，可选domain:{min,max}指定闭区间（-20到20且min<max），省略domain为全体实数；服务端按实际系数计算定义域内值域、零点和顶点，学生不能自报分析结果，用于常数/一次/二次函数探索；motion_lab.initialPositionM:-100至100米,initialVelocityMps:-20至20米每秒,accelerationMps2:-10至10米每二次方秒,timeSeconds:0至20秒，用于一维恒定加速度运动，允许速度变号，服务端计算位置、位移、路程、速度与速率；不使用摩擦、碰撞或变加速度推断；circuit_lab.mode:series|parallel,voltage:1到24整数V,r1/r2:1到100整数Ω用于理想两电阻直流实验；chemical_balance.reactionId:water|methane|iron_oxide,coefficients:[每项1到8整数，数量与物质数一致]用于原子守恒探索，water和iron_oxide有3项，methane有4项；这些实验仅探索不评分。sets.universe:[不重复文字元素，最多16个，每个最多64字节],setA:[全集内元素],setB:[全集内元素],operation:intersection|union|complement_a|complement_b|relations（可省略，默认intersection），用于交并补和子集/真子集/相等关系探索，补集必须相对universe，操作不评分；timeline.events:[{label,text}]；flowchart.nodes:[{id,label}],edges:[{from,to,label}]；function.coefficients:[a,b,c]表示a*x²+b*x+c,xMin,xMax,parameter可选{name,min,max,step,value,coefficientIndex:0|1|2}；forces.mass(kg),force(N),friction(最大可抵消阻力N，静止起步的简化模型，不模拟速度位移)；image只使用search_lesson_images已保存的同一素材id，不能自造地址。未完成图示可写status:failed,error。题目及标准仍通过append_section保存，私有答案不能放入讲义或课件。重试保留已有成功媒体，完成后finish_lesson。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 lesson，更新当前运行状态。
            auto lesson = savedLesson(db, accessFor(task), text(args, "lessonId")); validateTarget(lesson, task);
            // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
            const auto previous = lesson.value("deepContent", Json::object());
            // 【逐行批注】 计算右侧结果并写入 input，更新当前运行状态。
            auto input = args.at("content");
            // 【逐行批注】 计算右侧结果并写入 auto retryId，更新当前运行状态。
            const auto retryId = task.at("event").value("deepMediaRetry", "");
            // 【逐行批注】 判断条件 (!retryId.empty()) {，成立时执行对应分支。
            if (!retryId.empty()) {
                // 【逐行批注】 计算右侧结果并写入 preserved，更新当前运行状态。
                auto preserved = previous; bool replaced = false;
                // 【逐行批注】 按循环规则重复处理：for (const auto& requested : input.value("media", Json::array())) if (requ…。
                for (const auto& requested : input.value("media", Json::array())) if (requested.value("id", "") == retryId)
                    // 【逐行批注】 按循环规则重复处理：for (auto& item : preserved["media"]) if (item.value("id", "") == retryId)…。
                    for (auto& item : preserved["media"]) if (item.value("id", "") == retryId) { item = requested; replaced = true; }
                // 【逐行批注】 判断条件 (!replaced) throw std::invalid_argument("只重试指定素材时须提交该mediaId的新结构…，成立时执行对应分支。
                if (!replaced) throw std::invalid_argument("只重试指定素材时须提交该mediaId的新结构");
                // 【逐行批注】 计算右侧结果并写入 input，更新当前运行状态。
                input = std::move(preserved);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 content，更新当前运行状态。
            auto content = normalizeDeepContent(bindDeepSections(lesson, input));
            // 【逐行批注】 按循环规则重复处理：for (auto& item : content["media"]) if (item.at("type") == "image") for (c…。
            for (auto& item : content["media"]) if (item.at("type") == "image") for (const auto& old : previous.value("media", Json::array()))
                // 【逐行批注】 判断条件 (old.value("id", "") == item.at("id") && old.value("type", "") =…，成立时执行对应分支。
                if (old.value("id", "") == item.at("id") && old.value("type", "") == "image") item = old;
            // 【逐行批注】 计算右侧结果并写入 content["version"]，更新当前运行状态。
            content["version"] = previous.value("version", 0) + 1; content["model"] = source.model; content["generatedAt"] = libraryNow();
            // 【逐行批注】 计算右侧结果并写入 lesson["deepContent"]，更新当前运行状态。
            lesson["deepContent"] = content;
            // 【逐行批注】 结束当前函数并返回结果：return updateLesson(db, task, lesson, {{"lessonId", lesson.at("id")},…。
            return updateLesson(db, task, lesson, {{"lessonId", lesson.at("id")}, {"contentVersion", content.at("version")}, {"lectureCount", content.at("lecture").size()}, {"slideCount", content.at("slides").size()}});
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("search_lesson_images", {u8"围绕本课具体教学用途检索联网图片，复用现有双搜索和安全原文读取。参数les…。
    agent.registerTool("search_lesson_images", {u8"围绕本课具体教学用途检索联网图片，复用现有双搜索和安全原文读取。参数lessonId,query,title,mediaId（稳定标识），可选refresh:true。检索、图片二进制和来源一起保存；失败只记录该素材failed，已成功的讲义与图示保持。图片内容由检索来源提供，不把logo或猜测地址当教学图。返回已保存媒体，set_deep_content可按mediaId引用。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult&) {。
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            // 【逐行批注】 计算右侧结果并写入 lesson，更新当前运行状态。
            auto lesson = savedLesson(db, accessFor(task), text(args, "lessonId")); validateTarget(lesson, task);
            // 【逐行批注】 计算右侧结果并写入 auto mediaId，更新当前运行状态。
            const auto mediaId = text(args, "mediaId", 100), query = text(args, "query", 1000), title = text(args, "title", 500);
            // 【逐行批注】 计算右侧结果并写入 auto retryId，更新当前运行状态。
            const auto retryId = task.at("event").value("deepMediaRetry", "");
            // 【逐行批注】 判断条件 (!retryId.empty() && retryId != mediaId) throw std::invalid_argu…，成立时执行对应分支。
            if (!retryId.empty() && retryId != mediaId) throw std::invalid_argument("只能重试当前指定的素材");
            // 【逐行批注】 计算右侧结果并写入 bool refresh，更新当前运行状态。
            const bool refresh = args.value("refresh", false) || task.at("event").value("deepImageRepair", false);
            // 【逐行批注】 计算右侧结果并写入 content，更新当前运行状态。
            auto content = lesson.value("deepContent", Json{{"version", 0}, {"lecture", Json::array()}, {"slides", Json::array()}, {"media", Json::array()}});
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : content.at("media")) if (item.value("id", "") == m…。
            for (const auto& item : content.at("media")) if (item.value("id", "") == mediaId && item.value("status", "") == "ready" && !refresh) return PreparedAgentTool{{{"media", item}, {"cached", true}}, {}};
            // 【逐行批注】 计算右侧结果并写入 Json media，更新当前运行状态。
            Json media = {{"id", mediaId}, {"type", "image"}, {"title", title}, {"caption", title}, {"status", "failed"}, {"error", "图片暂未获取，可继续学习并重试。"}, {"query", query}};
            // 【逐行批注】 计算右侧结果并写入 Json image，更新当前运行状态。
            Json image = Json::object(); std::string imageId;
            // 【逐行批注】 计算右侧结果并写入 auto cancelled，更新当前运行状态。
            const auto cancelled = [&] {
                // 【逐行批注】 判断条件 (AIActivity::paused() || AIActivity::stopping() || db.learningRe…，成立时执行对应分支。
                if (AIActivity::paused() || AIActivity::stopping() || db.learningRevision() != task.at("learningVersion")) return true;
                // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
                const auto row = db.getClassroomActivity(task.at("id"));
                // 【逐行批注】 结束当前函数并返回结果：return !row || parsed(row->payload).value("status", "") != "running";。
                return !row || parsed(row->payload).value("status", "") != "running";
            // 【逐行批注】 执行当前声明或表达式：};。
            };
            // 【逐行批注】 计算右侧结果并写入 ResourceQuery request; request.query，更新当前运行状态。
            ResourceQuery request; request.query = query; request.count = 6; request.refresh = refresh; request.cancelled = cancelled; request.scopeId = task.at("scopeId").get<std::string>() + ":" + lesson.at("courseId").get<std::string>();
            // 【逐行批注】 计算右侧结果并写入 auto search，更新当前运行状态。
            const auto search = SearchClient().query(request);
            // 【逐行批注】 按循环规则重复处理：for (const auto& provider : search.providers) recordMediaUsage(db, task, {…。
            for (const auto& provider : search.providers) recordMediaUsage(db, task, {{"kind", "search"}, {"provider", provider.at("provider")}, {"status", provider.at("status")}, {"attempts", provider.at("attempts")}, {"cached", provider.at("cached")}, {"shared", provider.value("shared", false)}});
            // 【逐行批注】 计算右侧结果并写入 auto fetch，更新当前运行状态。
            const auto fetch = [&](const std::string& url) {
                // 【逐行批注】 计算右侧结果并写入 document，更新当前运行状态。
                auto document = libraryFetch(url, cancelled);
                // 【逐行批注】 调用 recordMediaUsage 完成本行所需操作：recordMediaUsage(db, task, {{"kind", "fetch"}, {"status", document.er…。
                recordMediaUsage(db, task, {{"kind", "fetch"}, {"status", document.error.empty() ? "completed" : document.error}, {"attempts", document.attempts}, {"cached", false}});
                // 【逐行批注】 结束当前函数并返回结果：return document;。
                return document;
            // 【逐行批注】 执行当前声明或表达式：};。
            };
            // 【逐行批注】 计算右侧结果并写入 int inspected，更新当前运行状态。
            int inspected = 0;
            // 【逐行批注】 按循环规则重复处理：for (const auto& resource : search.resources) {。
            for (const auto& resource : search.resources) {
                // 【逐行批注】 判断条件 (cancelled() || inspected++ >= 3) break;，成立时执行对应分支。
                if (cancelled() || inspected++ >= 3) break;
                // 【逐行批注】 计算右侧结果并写入 auto document，更新当前运行状态。
                const auto document = fetch(resource.url);
                // 【逐行批注】 判断条件 (!document.error.empty()) continue;，成立时执行对应分支。
                if (!document.error.empty()) continue;
                // 【逐行批注】 计算右侧结果并写入 candidates，更新当前运行状态。
                auto candidates = deepImageCandidates(document.body, document.url, query, title);
                // 【逐行批注】 判断条件 (!imageMime(document.body).empty()) candidates.insert(candidates…，成立时执行对应分支。
                if (!imageMime(document.body).empty()) candidates.insert(candidates.begin(), Json{{"url", document.url}, {"alt", resource.title}});
                // 【逐行批注】 计算右侧结果并写入 int attempts，更新当前运行状态。
                int attempts = 0;
                // 【逐行批注】 按循环规则重复处理：for (const auto& candidate : candidates) {。
                for (const auto& candidate : candidates) {
                    // 【逐行批注】 判断条件 (attempts++ >= 2 || cancelled()) break;，成立时执行对应分支。
                    if (attempts++ >= 2 || cancelled()) break;
                    // 【逐行批注】 计算右侧结果并写入 auto picture，更新当前运行状态。
                    const auto picture = candidate.at("url") == document.url ? document : fetch(candidate.at("url"));
                    // 【逐行批注】 计算右侧结果并写入 auto mime，更新当前运行状态。
                    const auto mime = imageMime(picture.body);
                    // 【逐行批注】 判断条件 (!picture.error.empty() || mime.empty() || picture.body.size() >…，成立时执行对应分支。
                    if (!picture.error.empty() || mime.empty() || picture.body.size() > 3 * 1024 * 1024) continue;
                    // 【逐行批注】 计算右侧结果并写入 imageId，更新当前运行状态。
                    imageId = "deep-image:" + libraryDigest(lesson.at("courseId").get<std::string>() + picture.url + picture.body);
                    // 【逐行批注】 计算右侧结果并写入 image，更新当前运行状态。
                    image = {{"data", encode64(picture.body)}, {"mime", mime}, {"url", picture.url}, {"courseId", lesson.at("courseId")}};
                    // 【逐行批注】 计算右侧结果并写入 media，更新当前运行状态。
                    media = {{"id", mediaId}, {"type", "image"}, {"title", title}, {"caption", title}, {"status", "ready"}, {"sourceUrl", document.url}, {"sourceTitle", resource.title}, {"alt", candidate.value("alt", title)}, {"sourceCaption", candidate.value("sourceCaption", "")}, {"selectionBasis", "source_text_match"}, {"blobId", imageId}, {"query", query}};
                    // 【逐行批注】 跳出当前循环或分支。
                    break;
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 判断条件 (!image.empty()) break;，成立时执行对应分支。
                if (!image.empty()) break;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 auto& mediaList，更新当前运行状态。
            auto& mediaList = content["media"]; bool replaced = false;
            // 【逐行批注】 按循环规则重复处理：for (auto& old : mediaList) if (old.value("id", "") == mediaId) { old = me…。
            for (auto& old : mediaList) if (old.value("id", "") == mediaId) { old = media; replaced = true; }
            // 【逐行批注】 判断条件 (!replaced) { if (mediaList.size() >= 32) throw std::invalid_arg…，成立时执行对应分支。
            if (!replaced) { if (mediaList.size() >= 32) throw std::invalid_argument("本课素材数量过多"); mediaList.push_back(media); }
            // 【逐行批注】 计算右侧结果并写入 content["version"]，更新当前运行状态。
            content["version"] = content.value("version", 0) + 1; lesson["deepContent"] = content;
            // 【逐行批注】 计算右侧结果并写入 prepared，更新当前运行状态。
            auto prepared = updateLesson(db, task, lesson, {{"media", media}, {"searchStatus", search.status}, {"cached", search.cached}});
            // 【逐行批注】 计算右侧结果并写入 auto commit，更新当前运行状态。
            const auto commit = prepared.commit;
            // 【逐行批注】 计算右侧结果并写入 prepared.commit，更新当前运行状态。
            prepared.commit = [commit, image, imageId](Database& connection) {
                // 【逐行批注】 判断条件 (!image.empty()) {，成立时执行对应分支。
                if (!image.empty()) {
                    // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
                    const auto previous = connection.myAIRecord(imageId);
                    // 【逐行批注】 判断条件 (previous && previous->kind != "deep-image") throw std::invalid_…，成立时执行对应分支。
                    if (previous && previous->kind != "deep-image") throw std::invalid_argument("图片缓存身份冲突");
                    // 【逐行批注】 判断条件 ((!previous || previous->payload != image.dump()) &&，成立时执行对应分支。
                    if ((!previous || previous->payload != image.dump()) &&
                        // 【逐行批注】 调用 libraryNow 完成本行所需操作：!connection.saveMyAIRecord(MyAIRecord{imageId, "deep-image", image.du…。
                        !connection.saveMyAIRecord(MyAIRecord{imageId, "deep-image", image.dump(), libraryNow(), 1}, previous ? previous->version : 0))
                        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("教学图片保存失败");。
                        throw std::runtime_error("教学图片保存失败");
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 调用 commit 完成本行所需操作：commit(connection);。
                commit(connection);
            // 【逐行批注】 执行当前声明或表达式：};。
            };
            // 【逐行批注】 结束当前函数并返回结果：return prepared;。
            return prepared;
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 deepCoursePrompt，执行该函数声明的职责。
std::string deepCoursePrompt(Database& db, const Json& task) {
    // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
    const auto event = task.at("event");
    // 【逐行批注】 计算右侧结果并写入 bool planning，更新当前运行状态。
    const bool planning = event.value("type", "") == "plan_course" && event.value("mode", "") == "deep";
    // 【逐行批注】 计算右侧结果并写入 bool deep，更新当前运行状态。
    const bool deep = !event.value("courseId", "").empty() && deepCourseEnabled(db, event.at("courseId"));
    // 【逐行批注】 判断条件 (!planning && !deep) return {};，成立时执行对应分支。
    if (!planning && !deep) return {};
    // 【逐行批注】 判断条件 (planning) return u8"本次新建深度课程：支持任意领域目标，按领域真实对象和任务组织courseStructu…，成立时执行对应分支。
    if (planning) return u8"本次新建深度课程：支持任意领域目标，按领域真实对象和任务组织courseStructure。只先生成并保存阶段、知识点、学习目标和主线。调用create_course后以state=completed结束本次规划，不创建课时或生成讲义；图片和逐课课件在进入课时时按需生成。create_course 的mode须保留event.mode。";
    // 【逐行批注】 结束当前函数并返回结果：return u8"当前课程使用deep-v1深度课堂：主区域是已保存讲义／翻页课件，右侧是可收起的辅助答疑。课程主线保持，讲解和练习按真…。
    return u8"当前课程使用deep-v1深度课堂：主区域是已保存讲义／翻页课件，右侧是可收起的辅助答疑。课程主线保持，讲解和练习按真实学习表现调整。备课时为指定topicId建立课时，使用append_section保存真实讲解与题目，用set_deep_content保存完整图文讲义及按需课件；finish_lesson前讲义必须已保存。讲义不泄露未作答题的私有标准，课件应有教学层次而非把长文本截断。按需要选择已注册交互：function_lab函数图像、number_line数轴与区间、vector_lab平面向量、equation_lab一次方程同解变形、statistics_lab有限数据描述统计、motion_lab一维恒加速度运动、circuit_lab串并联、chemical_balance原子守恒配平、sets集合、cell_lab细胞结构、history_lab事件排序、climate_lab气候比较、annotation_lab阅读标注、sentence_lab句子结构、algorithm_lab冒泡排序或二分查找；旧时间轴与流程图仍是静态资料，不称为互动；不适用时用准确图文讲义，不强制每课都有媒体。需要真实图片时调用search_lesson_images并保存来源；图片失败保留讲义与单项重试状态，不让整课失败。已有内容直接复用，翻页、换视图和操作本地图示不生成评分。学生新作答仍先评价后讲解，纯答疑不评分。只重试素材时保留其余已保存内容。";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 renderDeepCoursePage，执行该函数声明的职责。
std::string renderDeepCoursePage() {
    // 【逐行批注】 结束当前函数并返回结果：return R"HTML(<!doctype html><html lang="zh-CN"><head><meta charset="…。
    return R"HTML(<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta name="theme-color" content="#f5f7fc"><title>深度课堂 · 钢一定制AI</title><link rel="icon" href="/school-logo.png"><link rel="stylesheet" href="/vendor/katex/katex.min.css"><link rel="stylesheet" href="/deep-course.css"><link rel="stylesheet" href="/agent-trace.css"><link rel="stylesheet" href="/clarity.css"><link rel="stylesheet" href="/workspace.css"><script src="/workspace.js"></script><script>try{if(sessionStorage.getItem('gy-classroom-fullscreen')==='1')document.documentElement.classList.add('gy-classroom-fullscreen')}catch(_){}</script><script defer src="/vendor/katex/katex.min.js"></script><script defer src="/chat-render.js"></script><script defer src="/learning-agent.js"></script><script defer src="/agent-trace.js"></script><script defer src="/agent-shell.js"></script><script defer src="/clarity.js"></script><link rel="stylesheet" href="/vendor/jsxgraph/jsxgraph.css"><script defer src="/vendor/jsxgraph/jsxgraphcore.js"></script><script defer src="/interactive-subjects.js"></script><script defer src="/interactive-vectors.js"></script><script defer src="/interactive-equations.js"></script><script defer src="/interactive-statistics.js"></script><script defer src="/interactive-labs.js"></script><script defer src="/interactive-learning.js"></script><script defer src="/deep-course.js"></script></head><body class="clarity-app"><header class="site-header dark-header"><div class="dark-nav-shell"><a class="dark-brand" href="/"><img class="dark-brand-mark" src="/school-logo.png" alt="柳州市钢一中学校徽"><span>钢一<b>定制AI</b></span></a><nav class="dark-nav-links" aria-label="主导航"><a class="dark-nav-link" href="/">首页</a><a class="dark-nav-link" href="/my-courses">我的课程</a><a class="dark-nav-link is-active" href="/current-course" aria-current="page">当前课程</a><a class="dark-nav-link" href="/ask">AI 导师</a><a class="dark-nav-link" href="/my-ai">我的AI</a></nav><a class="dark-nav-cta" href="/my-ai/reports">我的画像</a><details class="dark-mobile-nav"><summary aria-label="打开主导航">菜单</summary><nav aria-label="窄屏主导航"><a href="/">首页</a><a href="/my-courses">我的课程</a><a href="/current-course">当前课程</a><a href="/ask">AI 导师</a><a href="/my-ai">我的AI</a></nav></details></div></header><main id="deep-course-app" aria-label="深度课堂"><p role="status">正在读取已保存的课堂…</p></main></body></html>)HTML";
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

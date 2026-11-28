// 【逐行批注】 引入头文件 "agent_preferences.hpp"，为后续实现提供类型或接口声明。
#include "agent_preferences.hpp"
// 【逐行批注】 引入头文件 "agent_curriculum.hpp"，为后续实现提供类型或接口声明。
#include "agent_curriculum.hpp"
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <ctime>，为后续实现提供类型或接口声明。
#include <ctime>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>
// 【逐行批注】 引入头文件 <random>，为后续实现提供类型或接口声明。
#include <random>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 定义函数 parse，执行该函数声明的职责。
Json parse(const std::string& text, Json fallback = Json::object()) {
    // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
    const auto value = Json::parse(text, nullptr, false);
    // 【逐行批注】 结束当前函数并返回结果：return value.is_discarded() ? fallback : value;。
    return value.is_discarded() ? fallback : value;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 fingerprint，执行该函数声明的职责。
std::string fingerprint(const std::string& text) {
    // 【逐行批注】 计算右侧结果并写入 unsigned long long value，更新当前运行状态。
    unsigned long long value = 14695981039346656037ULL;
    // 【逐行批注】 按循环规则重复处理：for (const unsigned char byte : text) { value ^= byte; value *= 1099511628…。
    for (const unsigned char byte : text) { value ^= byte; value *= 1099511628211ULL; }
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream out; out << std::hex << value; return out.str();。
    std::ostringstream out; out << std::hex << value; return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 now，执行该函数声明的职责。
std::string now() {
    // 【逐行批注】 计算右侧结果并写入 auto time，更新当前运行状态。
    const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // 【逐行批注】 执行当前声明或表达式：std::tm value{};。
    std::tm value{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&value, &time);。
    gmtime_s(&value, &time);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&time, &value);。
    gmtime_r(&time, &value);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 调用 put_time 完成本行所需操作：std::ostringstream out; out << std::put_time(&value, "%Y-%m-%dT%H:%M:…。
    std::ostringstream out; out << std::put_time(&value, "%Y-%m-%dT%H:%M:%SZ"); return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 newId，执行该函数声明的职责。
std::string newId() {
    // 【逐行批注】 调用 generator 完成本行所需操作：static thread_local std::mt19937_64 generator(std::random_device{}());。
    static thread_local std::mt19937_64 generator(std::random_device{}());
    // 【逐行批注】 调用 generator 完成本行所需操作：std::ostringstream out; out << "change-" << std::hex << generator() <…。
    std::ostringstream out; out << "change-" << std::hex << generator() << generator(); return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 accessFor，执行该函数声明的职责。
AgentAccess accessFor(const Json& task) {
    // 【逐行批注】 结束当前函数并返回结果：return {task.at("scopeId"), task.at("courseIds").get<std::vector<std:…。
    return {task.at("scopeId"), task.at("courseIds").get<std::vector<std::string>>()};
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 allowed，执行该函数声明的职责。
bool allowed(const AgentAccess& access, const std::string& courseId) {
    // 【逐行批注】 结束当前函数并返回结果：return std::find(access.courseIds.begin(), access.courseIds.end(), co…。
    return std::find(access.courseIds.begin(), access.courseIds.end(), courseId) != access.courseIds.end();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 requireCourse，执行该函数声明的职责。
Course requireCourse(Database& db, const AgentAccess& access, const std::string& courseId) {
    // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
    const auto found = db.getCourse(courseId);
    // 【逐行批注】 判断条件 (!found || found->status != "active" || !allowed(access, courseI…，成立时执行对应分支。
    if (!found || found->status != "active" || !allowed(access, courseId))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课程不存在或无权修改");。
        throw std::invalid_argument("课程不存在或无权修改");
    // 【逐行批注】 结束当前函数并返回结果：return *found;。
    return *found;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 requireText，执行该函数声明的职责。
std::string requireText(const Json& value, const char* key) {
    // 【逐行批注】 判断条件 (!value.value(key, Json()).is_string() || value[key].get<std::st…，成立时执行对应分支。
    if (!value.value(key, Json()).is_string() || value[key].get<std::string>().empty())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument(std::string(key) + " 不能为空");。
        throw std::invalid_argument(std::string(key) + " 不能为空");
    // 【逐行批注】 结束当前函数并返回结果：return value[key];。
    return value[key];
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 recommendationKey，执行该函数声明的职责。
std::string recommendationKey(const AgentAccess& access) { return "agent-recommendations:" + fingerprint(access.scopeId); }
// 【逐行批注】 定义函数 changeIds，执行该函数声明的职责。
std::vector<std::string> changeIds(Database& db) {
    // 【逐行批注】 计算右侧结果并写入 auto ids，更新当前运行状态。
    const auto ids = parse(db.profileMeta("agent-changes"), Json::array());
    // 【逐行批注】 结束当前函数并返回结果：return ids.is_array() ? ids.get<std::vector<std::string>>() : std::ve…。
    return ids.is_array() ? ids.get<std::vector<std::string>>() : std::vector<std::string>{};
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 声明对象 journal，并通过构造函数初始化其状态。
Json journal(Database& db, const Json& task, const AIResult& source, const Json& args,
             // 【逐行批注】 执行当前声明或表达式：const std::string& kind, const std::string& target, const Json& before, const Json& …。
             const std::string& kind, const std::string& target, const Json& before, const Json& after) {
    // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
    const auto access = accessFor(task);
    // 【逐行批注】 计算右侧结果并写入 semantic，更新当前运行状态。
    auto semantic = args; semantic.erase("reason"); semantic.erase("version");
    // 【逐行批注】 计算右侧结果并写入 auto signature，更新当前运行状态。
    const auto signature = fingerprint(Json{{"kind", kind}, {"target", target}, {"args", semantic}}.dump());
    // 【逐行批注】 计算右侧结果并写入 auto learner，更新当前运行状态。
    const auto learner = agentLearnerFingerprint(db, access);
    // 【逐行批注】 按循环规则重复处理：for (const auto& id : changeIds(db)) {。
    for (const auto& id : changeIds(db)) {
        // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
        const auto row = db.getClassroomActivity(id); if (!row || row->kind != "agent-change") continue;
        // 【逐行批注】 计算右侧结果并写入 auto old，更新当前运行状态。
        const auto old = parse(row->payload);
        // 【逐行批注】 判断条件 (old.value("scopeId", "") == access.scopeId && old.value("status…，成立时执行对应分支。
        if (old.value("scopeId", "") == access.scopeId && old.value("status", "") == "undone" &&
            // 【逐行批注】 执行当前声明或表达式：old.value("signature", "") == signature && old.value("undoFingerprint", "") == learn…。
            old.value("signature", "") == signature && old.value("undoFingerprint", "") == learner)
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("用户已撤回同一改动；等待新的学习反馈后再决定，不能立即重新应用");。
            throw std::invalid_argument("用户已撤回同一改动；等待新的学习反馈后再决定，不能立即重新应用");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return {{"id", newId()}, {"scopeId", access.scopeId}, {"taskId", task…。
    return {{"id", newId()}, {"scopeId", access.scopeId}, {"taskId", task.at("id")},
        // 【逐行批注】 执行当前声明或表达式：{"kind", kind}, {"target", target}, {"before", before}, {"after", after}, {"signatur…。
        {"kind", kind}, {"target", target}, {"before", before}, {"after", after}, {"signature", signature},
        // 【逐行批注】 调用 requireText、now 完成本行所需操作：{"reason", requireText(args, "reason")}, {"model", source.model}, {"c…。
        {"reason", requireText(args, "reason")}, {"model", source.model}, {"createdAt", now()},
        // 【逐行批注】 执行当前声明或表达式：{"status", "applied"}, {"sourceLearningVersion", task.at("learningVersion")}};。
        {"status", "applied"}, {"sourceLearningVersion", task.at("learningVersion")}};
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 publicChange，执行该函数声明的职责。
Json publicChange(const Json& change) {
    // 【逐行批注】 执行当前声明或表达式：Json value;。
    Json value;
    // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"id", "kind", "target", "reason", "model", "create…。
    for (const auto* key : {"id", "kind", "target", "reason", "model", "createdAt", "status", "undoneAt"})
        // 【逐行批注】 判断条件 (change.contains(key)) value[key] = change[key];，成立时执行对应分支。
        if (change.contains(key)) value[key] = change[key];
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 声明对象 saveChange，并通过构造函数初始化其状态。
void saveChange(Database& db, const Json& change, const std::string& courseId = {}) {
    // 【逐行批注】 判断条件 (!db.upsert(ClassroomActivity{change.at("id"), courseId, "agent-…，成立时执行对应分支。
    if (!db.upsert(ClassroomActivity{change.at("id"), courseId, "agent-change", change.dump(), now(), 0, 0}))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("调整记录保存失败");。
        throw std::runtime_error("调整记录保存失败");
    // 【逐行批注】 计算右侧结果并写入 ids，更新当前运行状态。
    auto ids = changeIds(db);
    // 【逐行批注】 判断条件 (std::find(ids.begin(), ids.end(), change.at("id")) == ids.end()…，成立时执行对应分支。
    if (std::find(ids.begin(), ids.end(), change.at("id")) == ids.end()) ids.push_back(change.at("id"));
    // 【逐行批注】 调用 Json 完成本行所需操作：db.setProfileMeta("agent-changes", Json(ids).dump());。
    db.setProfileMeta("agent-changes", Json(ids).dump());
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 dateValue，执行该函数声明的职责。
std::tm dateValue(const std::string& date) {
    // 【逐行批注】 调用 input、get_time 完成本行所需操作：std::tm value{}; std::istringstream input(date); input >> std::get_ti…。
    std::tm value{}; std::istringstream input(date); input >> std::get_time(&value, "%Y-%m-%d");
    // 【逐行批注】 判断条件 (input.fail() || date.size() != 10) throw std::invalid_argument(…，成立时执行对应分支。
    if (input.fail() || date.size() != 10) throw std::invalid_argument("学习日期无效");
    // 【逐行批注】 计算右侧结果并写入 value.tm_hour，更新当前运行状态。
    value.tm_hour = 12; std::mktime(&value);
    // 【逐行批注】 调用 put_time 完成本行所需操作：std::ostringstream normalized; normalized << std::put_time(&value, "%…。
    std::ostringstream normalized; normalized << std::put_time(&value, "%Y-%m-%d");
    // 【逐行批注】 判断条件 (normalized.str() != date) throw std::invalid_argument("学习日期不存在"…，成立时执行对应分支。
    if (normalized.str() != date) throw std::invalid_argument("学习日期不存在");
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 today，执行该函数声明的职责。
std::string today() {
    // 【逐行批注】 计算右侧结果并写入 auto clock，更新当前运行状态。
    const auto clock = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // 【逐行批注】 执行当前声明或表达式：std::tm value{};。
    std::tm value{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 localtime_s 完成本行所需操作：localtime_s(&value, &clock);。
    localtime_s(&value, &clock);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 localtime_r 完成本行所需操作：localtime_r(&clock, &value);。
    localtime_r(&clock, &value);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 调用 put_time 完成本行所需操作：std::ostringstream out; out << std::put_time(&value, "%Y-%m-%d"); ret…。
    std::ostringstream out; out << std::put_time(&value, "%Y-%m-%d"); return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 samePosition，执行该函数声明的职责。
bool samePosition(const Json& a, const Json& b) {
    // 【逐行批注】 结束当前函数并返回结果：return a.value("date", "") == b.value("date", "") && a.value("minutes…。
    return a.value("date", "") == b.value("date", "") && a.value("minutes", 0) == b.value("minutes", 0) &&
        // 【逐行批注】 执行当前声明或表达式：a.value("order", 0) == b.value("order", 0);。
        a.value("order", 0) == b.value("order", 0);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 validatePlan，执行该函数声明的职责。
void validatePlan(Database& db, const AgentAccess& access, const Json& old, const Json& plan) {
    // 【逐行批注】 判断条件 (!plan.value("availability", Json()).is_array() || !plan.value("…，成立时执行对应分支。
    if (!plan.value("availability", Json()).is_array() || !plan.value("entries", Json()).is_array())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("学习时间与任务列表无效");。
        throw std::invalid_argument("学习时间与任务列表无效");
    // 【逐行批注】 执行当前声明或表达式：std::map<int, int> budgets;。
    std::map<int, int> budgets;
    // 【逐行批注】 按循环规则重复处理：for (const auto& slot : plan["availability"]) {。
    for (const auto& slot : plan["availability"]) {
        // 【逐行批注】 判断条件 (!slot.value("weekday", Json()).is_number_integer() || !slot.val…，成立时执行对应分支。
        if (!slot.value("weekday", Json()).is_number_integer() || !slot.value("minutes", Json()).is_number_integer())
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("可用时间格式无效");。
            throw std::invalid_argument("可用时间格式无效");
        // 【逐行批注】 计算右侧结果并写入 int day，更新当前运行状态。
        const int day = slot["weekday"], minutes = slot["minutes"];
        // 【逐行批注】 判断条件 (day < 1 || day > 7 || minutes < 1 || minutes > 1440 || budgets.…，成立时执行对应分支。
        if (day < 1 || day > 7 || minutes < 1 || minutes > 1440 || budgets.count(day))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("学习日应为 1 至 7，时长应在一天以内，且不能重复");。
            throw std::invalid_argument("学习日应为 1 至 7，时长应在一天以内，且不能重复");
        // 【逐行批注】 计算右侧结果并写入 budgets[day]，更新当前运行状态。
        budgets[day] = minutes;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, int> totals;。
    std::map<std::string, int> totals;
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, Json> entries;。
    std::map<std::string, Json> entries;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : plan["entries"]) {。
    for (const auto& item : plan["entries"]) {
        // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
        const auto courseId = requireText(item, "courseId"); requireCourse(db, access, courseId);
        // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
        const auto id = requireText(item, "taskId"), date = requireText(item, "date");
        // 【逐行批注】 计算右侧结果并写入 auto stamp，更新当前运行状态。
        const auto stamp = dateValue(date);
        // 【逐行批注】 判断条件 (!item.value("minutes", Json()).is_number_integer() || item["min…，成立时执行对应分支。
        if (!item.value("minutes", Json()).is_number_integer() || item["minutes"].get<int>() < 1 ||
            // 【逐行批注】 调用 invalid_argument 完成本行所需操作：item["minutes"].get<int>() > 1440 || entries.count(id)) throw std::in…。
            item["minutes"].get<int>() > 1440 || entries.count(id)) throw std::invalid_argument("任务时长或身份无效");
        // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
        const auto kind = item.value("kind", "lesson");
        // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
        bool valid = false;
        // 【逐行批注】 判断条件 (kind == "review") {，成立时执行对应分支。
        if (kind == "review") {
            // 【逐行批注】 计算右侧结果并写入 auto review，更新当前运行状态。
            const auto review = db.getClassroomActivity(item.value("reviewId", ""));
            // 【逐行批注】 计算右侧结果并写入 valid，更新当前运行状态。
            valid = review && review->courseId == courseId && review->kind == "review" && id == "review:" + review->id;
        // 【逐行批注】 执行当前声明或表达式：} else if (kind == "agent-lesson") {。
        } else if (kind == "agent-lesson") {
            // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
            const auto lesson = db.getClassroomActivity(item.value("lessonId", ""));
            // 【逐行批注】 计算右侧结果并写入 valid，更新当前运行状态。
            valid = lesson && lesson->courseId == courseId && lesson->kind == "agent-lesson" && id == "agent-lesson:" + lesson->id;
        // 【逐行批注】 执行当前声明或表达式：} else if (kind == "lesson") {。
        } else if (kind == "lesson") {
            // 【逐行批注】 计算右侧结果并写入 int phase，更新当前运行状态。
            const int phase = item.value("phaseIndex", 0), topic = item.value("topicIndex", 0);
            // 【逐行批注】 计算右侧结果并写入 Json outline; int version，更新当前运行状态。
            Json outline; int version = 0;
            // 【逐行批注】 按循环规则重复处理：for (const auto& snapshot : db.findSnapshotsByCourseId(courseId)) if (snap…。
            for (const auto& snapshot : db.findSnapshotsByCourseId(courseId)) if (snapshot.version > version) {
                // 【逐行批注】 计算右侧结果并写入 outline，更新当前运行状态。
                outline = parse(snapshot.payload); version = snapshot.version;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 auto stages，更新当前运行状态。
            const auto stages = outline.value("courseStructure", Json::array());
            // 【逐行批注】 计算右侧结果并写入 valid，更新当前运行状态。
            valid = phase >= 1 && phase <= static_cast<int>(stages.size()) && topic >= 1 &&
                // 【逐行批注】 调用 array 完成本行所需操作：topic <= static_cast<int>(stages[phase - 1].value("topics", Json::arr…。
                topic <= static_cast<int>(stages[phase - 1].value("topics", Json::array()).size()) &&
                // 【逐行批注】 调用 to_string 完成本行所需操作：id == "lesson:" + courseId + ":" + std::to_string(phase) + ":" + std:…。
                id == "lesson:" + courseId + ":" + std::to_string(phase) + ":" + std::to_string(topic);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (!valid) throw std::invalid_argument("课表中的任务身份不存在；请使用 scheduleTa…，成立时执行对应分支。
        if (!valid) throw std::invalid_argument("课表中的任务身份不存在；请使用 scheduleTasks 中配套的 taskId、kind、courseId 和课时索引，不把备课任务 ID 当作课时 ID");
        // 【逐行批注】 计算右侧结果并写入 entries[id]，更新当前运行状态。
        entries[id] = item;
        // 【逐行批注】 计算右侧结果并写入 auto actualProgress，更新当前运行状态。
        const auto actualProgress = db.findLearningCardProgress(courseId, item.value("phaseIndex", 0), item.value("topicIndex", 0));
        // 【逐行批注】 计算右侧结果并写入 bool actuallyCompleted，更新当前运行状态。
        const bool actuallyCompleted = kind == "lesson" && actualProgress && actualProgress->status == "completed";
        // 【逐行批注】 判断条件 (item.value("completed", false) && !actuallyCompleted) throw std…，成立时执行对应分支。
        if (item.value("completed", false) && !actuallyCompleted) throw std::invalid_argument("不能把未完成任务标成已完成以绕过预算");
        // 【逐行批注】 判断条件 (!actuallyCompleted && date >= today()) {，成立时执行对应分支。
        if (!actuallyCompleted && date >= today()) {
            // 【逐行批注】 计算右侧结果并写入 auto day，更新当前运行状态。
            const auto day = stamp.tm_wday == 0 ? 7 : stamp.tm_wday;
            // 【逐行批注】 计算右侧结果并写入 totals[date] +，更新当前运行状态。
            totals[date] += item["minutes"].get<int>();
            // 【逐行批注】 判断条件 (totals[date] > budgets[day]) throw std::invalid_argument("当天全部课…，成立时执行对应分支。
            if (totals[date] > budgets[day]) throw std::invalid_argument("当天全部课程的时长超过共享预算");
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& previous : old.value("entries", Json::array())) {。
    for (const auto& previous : old.value("entries", Json::array())) {
        // 【逐行批注】 计算右侧结果并写入 auto progress，更新当前运行状态。
        const auto progress = db.findLearningCardProgress(previous.value("courseId", ""),
            // 【逐行批注】 执行当前声明或表达式：previous.value("phaseIndex", 0), previous.value("topicIndex", 0));。
            previous.value("phaseIndex", 0), previous.value("topicIndex", 0));
        // 【逐行批注】 计算右侧结果并写入 bool stable，更新当前运行状态。
        const bool stable = previous.value("completed", false) || previous.value("inProgress", false) ||
            // 【逐行批注】 执行当前声明或表达式：(progress && progress->status != "not_started");。
            (progress && progress->status != "not_started");
        // 【逐行批注】 判断条件 (stable && (!entries.count(previous.value("taskId", "")) || !sam…，成立时执行对应分支。
        if (stable && (!entries.count(previous.value("taskId", "")) || !samePosition(previous, entries.at(previous["taskId"]))))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("已完成或正在学习的任务必须保留原位置和时长");。
            throw std::invalid_argument("已完成或正在学习的任务必须保留原位置和时长");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& [id, item] : entries) {。
    for (const auto& [id, item] : entries) {
        // 【逐行批注】 计算右侧结果并写入 auto requirement，更新当前运行状态。
        const auto requirement = item.value("requires", "");
        // 【逐行批注】 判断条件 (!requirement.empty() && (!entries.count(requirement) ||，成立时执行对应分支。
        if (!requirement.empty() && (!entries.count(requirement) ||
            // 【逐行批注】 执行当前声明或表达式：entries.at(requirement).value("date", "") > item.value("date", "") ||。
            entries.at(requirement).value("date", "") > item.value("date", "") ||
            // 【逐行批注】 执行当前声明或表达式：(entries.at(requirement).value("date", "") == item.value("date", "") &&。
            (entries.at(requirement).value("date", "") == item.value("date", "") &&
                // 【逐行批注】 执行当前声明或表达式：entries.at(requirement).value("order", 0) > item.value("order", 0))))。
                entries.at(requirement).value("order", 0) > item.value("order", 0))))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("安排顺序不满足明确的前置关系");。
            throw std::invalid_argument("安排顺序不满足明确的前置关系");
        // 【逐行批注】 判断条件 (item.value("date", "") < today()) {，成立时执行对应分支。
        if (item.value("date", "") < today()) {
            // 【逐行批注】 计算右侧结果并写入 bool historical，更新当前运行状态。
            bool historical = false;
            // 【逐行批注】 按循环规则重复处理：for (const auto& previous : old.value("entries", Json::array()))。
            for (const auto& previous : old.value("entries", Json::array()))
                // 【逐行批注】 判断条件 (previous.value("taskId", "") == id && samePosition(previous, it…，成立时执行对应分支。
                if (previous.value("taskId", "") == id && samePosition(previous, item)) historical = true;
            // 【逐行批注】 判断条件 (!historical) throw std::invalid_argument("新的待学习任务不能安排到过去");，成立时执行对应分支。
            if (!historical) throw std::invalid_argument("新的待学习任务不能安排到过去");
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentChanges，执行该函数声明的职责。
Json agentChanges(Database& db, const AgentAccess& access) {
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = Json::array();
    // 【逐行批注】 按循环规则重复处理：for (const auto& id : changeIds(db)) {。
    for (const auto& id : changeIds(db)) {
        // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
        const auto row = db.getClassroomActivity(id); if (!row) continue;
        // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
        const auto value = parse(row->payload);
        // 【逐行批注】 判断条件 (value.value("scopeId", "") == access.scopeId) result.push_back(…，成立时执行对应分支。
        if (value.value("scopeId", "") == access.scopeId) result.push_back(publicChange(value));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 声明对象 agentChangeRecord，并通过构造函数初始化其状态。
Json agentChangeRecord(Database& db, const Json& task, const AIResult& source, const Json& args,
    // 【逐行批注】 执行当前声明或表达式：const std::string& kind, const std::string& target, const Json& before, const Json& …。
    const std::string& kind, const std::string& target, const Json& before, const Json& after) {
    // 【逐行批注】 结束当前函数并返回结果：return journal(db, task, source, args, kind, target, before, after);。
    return journal(db, task, source, args, kind, target, before, after);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 agentSaveChange，执行该函数声明的职责。
void agentSaveChange(Database& db, const Json& change, const std::string& courseId) { saveChange(db, change, courseId); }
// 【逐行批注】 定义函数 agentRecommendationView，执行该函数声明的职责。
Json agentRecommendationView(Database& db, const AgentAccess& access) {
    // 【逐行批注】 结束当前函数并返回结果：return parse(db.profileMeta(recommendationKey(access)));。
    return parse(db.profileMeta(recommendationKey(access)));
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 registerAgentPreferenceTools，执行该函数声明的职责。
void registerAgentPreferenceTools(LearningAgent& agent) {
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("adjust_goal", {"自主调整可访问课程目标。参数 courseId、goal、reason；不修改历史课程身份，改动…。
    agent.registerTool("adjust_goal", {"自主调整可访问课程目标。参数 courseId、goal、reason；不修改历史课程身份，改动可撤回。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
            const auto access = accessFor(task);
            // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
            const auto course = requireCourse(db, access, requireText(args, "courseId"));
            // 【逐行批注】 计算右侧结果并写入 auto goal，更新当前运行状态。
            const auto goal = requireText(args, "goal");
            // 【逐行批注】 判断条件 (goal == course.goal) return PreparedAgentTool{{{"changed", fals…，成立时执行对应分支。
            if (goal == course.goal) return PreparedAgentTool{{{"changed", false}}, {}};
            // 【逐行批注】 计算右侧结果并写入 auto change，更新当前运行状态。
            const auto change = journal(db, task, source, args, "goal", course.id,
                // 【逐行批注】 调用 now 完成本行所需操作：Json{{"goal", course.goal}, {"updatedAt", course.updatedAt}}, Json{{"…。
                Json{{"goal", course.goal}, {"updatedAt", course.updatedAt}}, Json{{"goal", goal}, {"updatedAt", now()}});
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"changed", true}, {"change", publicChange(…。
            return PreparedAgentTool{{{"changed", true}, {"change", publicChange(change)}},
                // 【逐行批注】 执行当前声明或表达式：[course, change, access](Database& connection) {。
                [course, change, access](Database& connection) {
                    // 【逐行批注】 计算右侧结果并写入 current，更新当前运行状态。
                    auto current = requireCourse(connection, access, course.id);
                    // 【逐行批注】 判断条件 (current.goal != course.goal || current.updatedAt != course.upda…，成立时执行对应分支。
                    if (current.goal != course.goal || current.updatedAt != course.updatedAt)
                        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课程目标已经变化，旧调整不应用");。
                        throw std::invalid_argument("课程目标已经变化，旧调整不应用");
                    // 【逐行批注】 计算右侧结果并写入 current.goal，更新当前运行状态。
                    current.goal = change["after"]["goal"]; current.updatedAt = change["after"]["updatedAt"];
                    // 【逐行批注】 判断条件 (!connection.update(current)) throw std::runtime_error("目标保存失败");，成立时执行对应分支。
                    if (!connection.update(current)) throw std::runtime_error("目标保存失败");
                    // 【逐行批注】 调用 saveChange 完成本行所需操作：connection.markLearningDirty(); saveChange(connection, change, course…。
                    connection.markLearningDirty(); saveChange(connection, change, course.id);
                // 【逐行批注】 执行当前声明或表达式：}};。
                }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("adjust_schedule", {"仅在用户手动重新排课的任务中生成统一课表候选。参数 availability:[{wee…。
    agent.registerTool("adjust_schedule", {"仅在用户手动重新排课的任务中生成统一课表候选。参数 availability:[{weekday:1..7,minutes:1..1440}]、entries:[{taskId,courseId,kind,date,minutes,order,phaseIndex,topicIndex,requires}]、reason。使用上下文 scheduleTasks 中真实存在的任务身份：大纲知识点 kind=lesson，taskId=lesson:<courseId>:<phaseIndex>:<topicIndex>；已备课 kind=agent-lesson，taskId=agent-lesson:<lessonId>，附 lessonId；复习 kind=review，taskId=review:<reviewId>，附 reviewId。不可用 agent 备课任务 ID 或裸课时 ID 替代。calendarDate 是本机今天，新安排不得放在过去。始终保留原课表及编辑草稿，等待用户确认；不得直接应用。保留已完成和在学任务；日总量不可超预算。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 判断条件 (!task.value("manualScheduleReplan", false) || (task.at("event")…，成立时执行对应分支。
            if (!task.value("manualScheduleReplan", false) || (task.at("event").value("type", "") != "schedule_replan" && task.at("event").value("type", "") != "assistant_chat"))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("只有用户手动重新排课的任务可以生成课表候选");。
                throw std::invalid_argument("只有用户手动重新排课的任务可以生成课表候选");
            // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
            const auto access = accessFor(task);
            // 【逐行批注】 按循环规则重复处理：for (const auto& course : db.listCourses()) if (course.status == "active" …。
            for (const auto& course : db.listCourses()) if (course.status == "active" && !allowed(access, course.id))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("无权修改其他用户课程的共享预算");。
                throw std::invalid_argument("无权修改其他用户课程的共享预算");
            // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
            const auto raw = db.profileMeta("learning-flow"); auto state = parse(raw);
            // 【逐行批注】 计算右侧结果并写入 old，更新当前运行状态。
            auto old = state.value("plan", Json::object());
            // 【逐行批注】 计算右侧结果并写入 Json plan，更新当前运行状态。
            Json plan = old; plan["availability"] = args.at("availability"); plan["entries"] = args.at("entries");
            // 【逐行批注】 调用 validatePlan 完成本行所需操作：validatePlan(db, access, old, plan);。
            validatePlan(db, access, old, plan);
            // 页面标题来自实际课程与课件，不要求模型重复抄写，也不显示内部标识。
            // 【逐行批注】 按循环规则重复处理：for (auto& entry : plan["entries"]) {。
            for (auto& entry : plan["entries"]) {
                // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
                const auto courseId = entry.at("courseId").get<std::string>();
                // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
                const auto course = requireCourse(db, access, courseId);
                // 【逐行批注】 计算右侧结果并写入 entry["courseTitle"]，更新当前运行状态。
                entry["courseTitle"] = course.title;
                // 【逐行批注】 执行当前声明或表达式：std::string title;。
                std::string title;
                // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
                const auto kind = entry.value("kind", "lesson");
                // 【逐行批注】 判断条件 (kind == "agent-lesson" || kind == "review") {，成立时执行对应分支。
                if (kind == "agent-lesson" || kind == "review") {
                    // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
                    const auto saved = db.getClassroomActivity(entry.at(kind == "review" ? "reviewId" : "lessonId"));
                    // 【逐行批注】 判断条件 (!saved) throw std::invalid_argument("课时已变化，请重新读取任务目录");，成立时执行对应分支。
                    if (!saved) throw std::invalid_argument("课时已变化，请重新读取任务目录");
                    // 【逐行批注】 计算右侧结果并写入 title，更新当前运行状态。
                    title = parse(saved->payload).value("title", "");
                // 【逐行批注】 执行当前声明或表达式：} else {。
                } else {
                    // 【逐行批注】 计算右侧结果并写入 auto snapshots，更新当前运行状态。
                    const auto snapshots = db.findSnapshotsByCourseId(courseId);
                    // 【逐行批注】 判断条件 (snapshots.empty()) throw std::invalid_argument("课程大纲已变化，请重新读取任务…，成立时执行对应分支。
                    if (snapshots.empty()) throw std::invalid_argument("课程大纲已变化，请重新读取任务目录");
                    // 【逐行批注】 计算右侧结果并写入 auto latest，更新当前运行状态。
                    const auto latest = std::max_element(snapshots.begin(), snapshots.end(),
                        // 【逐行批注】 执行当前声明或表达式：[](const auto& a, const auto& b) { return a.version < b.version; });。
                        [](const auto& a, const auto& b) { return a.version < b.version; });
                    // 【逐行批注】 计算右侧结果并写入 auto topic，更新当前运行状态。
                    const auto topic = parse(latest->payload).at("courseStructure").at(entry.at("phaseIndex").get<int>() - 1)
                        // 【逐行批注】 执行当前声明或表达式：.at("topics").at(entry.at("topicIndex").get<int>() - 1);。
                        .at("topics").at(entry.at("topicIndex").get<int>() - 1);
                    // 【逐行批注】 计算右侧结果并写入 title，更新当前运行状态。
                    title = topic.is_string() ? topic.get<std::string>() : topic.is_object() ? topic.value("title", "") : "";
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 entry["title"]，更新当前运行状态。
                entry["title"] = title.empty() ? course.title : title;
            // 【逐行批注】 结束当前代码块。
            }
            // 标题中的日期范围跟随候选任务，避免新安排仍显示旧课表的过期区间。
            // 【逐行批注】 判断条件 (!plan["entries"].empty()) {，成立时执行对应分支。
            if (!plan["entries"].empty()) {
                // 【逐行批注】 执行当前声明或表达式：std::string first, last;。
                std::string first, last;
                // 【逐行批注】 按循环规则重复处理：for (const auto& entry : plan["entries"]) {。
                for (const auto& entry : plan["entries"]) {
                    // 【逐行批注】 计算右侧结果并写入 auto date，更新当前运行状态。
                    const auto date = entry.at("date").get<std::string>();
                    // 【逐行批注】 判断条件 (first.empty() || date < first) first = date;，成立时执行对应分支。
                    if (first.empty() || date < first) first = date;
                    // 【逐行批注】 判断条件 (last.empty() || date > last) last = date;，成立时执行对应分支。
                    if (last.empty() || date > last) last = date;
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 plan["weekStart"]，更新当前运行状态。
                plan["weekStart"] = first; plan["weekEnd"] = last;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 plan["version"]，更新当前运行状态。
            plan["version"] = old.value("version", 0) + 1; plan["source"] = "ai"; plan["updatedAt"] = now();
            // 【逐行批注】 计算右侧结果并写入 after，更新当前运行状态。
            auto after = state; after["plan"] = plan; after["status"] = "ready"; after["model"] = source.model; after["agentControlled"] = true;
            // 【逐行批注】 计算右侧结果并写入 after["reason"]，更新当前运行状态。
            after["reason"] = requireText(args, "reason"); after.erase("proposal");
            // 【逐行批注】 计算右侧结果并写入 auto change，更新当前运行状态。
            const auto change = journal(db, task, source, args, "schedule", "learning-flow", raw, after.dump());
            // 【逐行批注】 计算右侧结果并写入 auto draftRaw，更新当前运行状态。
            const auto draftRaw = db.profileMeta("study-drafts");
            // 【逐行批注】 计算右侧结果并写入 auto drafts，更新当前运行状态。
            const auto drafts = parse(draftRaw);
            // 【逐行批注】 判断条件 (!drafts.is_object()) throw std::invalid_argument("study-drafts …，成立时执行对应分支。
            if (!drafts.is_object()) throw std::invalid_argument("study-drafts 必须为对象，原编辑内容未改动");
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"changed", false}, {"draftProtected", true…。
            return PreparedAgentTool{{{"changed", false}, {"draftProtected", true}, {"proposalId", change.at("id")},
                // 【逐行批注】 执行当前声明或表达式：{"message", "AI 候选安排已保存，等待用户确认，原课表和编辑内容保留。"}},。
                {"message", "AI 候选安排已保存，等待用户确认，原课表和编辑内容保留。"}},
                // 【逐行批注】 计算右侧结果并写入 [after, draftRaw, raw, old, change, taskId，更新当前运行状态。
                [after, draftRaw, raw, old, change, taskId = task.at("id").get<std::string>()](Database& connection) {
                    // 【逐行批注】 计算右侧结果并写入 auto taskRow，更新当前运行状态。
                    const auto taskRow = connection.getClassroomActivity(taskId);
                    // 【逐行批注】 判断条件 (!taskRow || !parse(taskRow->payload).value("manualScheduleRepla…，成立时执行对应分支。
                    if (!taskRow || !parse(taskRow->payload).value("manualScheduleReplan", false))
                        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("本次任务没有手动排课授权");。
                        throw std::invalid_argument("本次任务没有手动排课授权");
                    // 【逐行批注】 判断条件 (connection.profileMeta("study-drafts") != draftRaw) throw std::…，成立时执行对应分支。
                    if (connection.profileMeta("study-drafts") != draftRaw) throw std::invalid_argument("编辑状态已经变化");
                    // 【逐行批注】 执行当前声明或表达式：connection.setProfileMeta("agent-schedule-proposal", after.dump());。
                    connection.setProfileMeta("agent-schedule-proposal", after.dump());
                    // 【逐行批注】 计算右侧结果并写入 candidate，更新当前运行状态。
                    auto candidate = change; candidate["status"] = "candidate";
                    // 【逐行批注】 计算右侧结果并写入 state，更新当前运行状态。
                    auto state = parse(raw); state["agentControlled"] = true; state["status"] = "ready";
                    // 【逐行批注】 计算右侧结果并写入 state["proposal"]，更新当前运行状态。
                    state["proposal"] = {{"id", change.at("id")}, {"plan", after.at("plan")}, {"previousPlan", old},
                        // 【逐行批注】 执行当前声明或表达式：{"reason", change.at("reason")}, {"learningVersion", connection.learningRevision()}};。
                        {"reason", change.at("reason")}, {"learningVersion", connection.learningRevision()}};
                    // 【逐行批注】 计算右侧结果并写入 state["message"]，更新当前运行状态。
                    state["message"] = "候选已生成，请预览并确认；原课表和编辑内容已保留。";
                    // 【逐行批注】 判断条件 (!connection.compareProfileMeta("learning-flow", raw, state.dump…，成立时执行对应分支。
                    if (!connection.compareProfileMeta("learning-flow", raw, state.dump())) throw std::invalid_argument("课表版本已变化");
                    // 【逐行批注】 调用 saveChange 完成本行所需操作：saveChange(connection, candidate);。
                    saveChange(connection, candidate);
                // 【逐行批注】 执行当前声明或表达式：}};。
                }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("update_recommendations", {"根据最新真实表现决定更新首页推荐；无需固定补弱与探索比例。参数 lite、…。
    agent.registerTool("update_recommendations", {"根据最新真实表现决定更新首页推荐；无需固定补弱与探索比例。参数 lite、deep 两组非重复纯文本目标，以及 reason；每组数量自行决定。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 按循环规则重复处理：for (const auto* mode : {"lite", "deep"}) {。
            for (const auto* mode : {"lite", "deep"}) {
                // 【逐行批注】 判断条件 (!args.value(mode, Json()).is_array() || args[mode].empty()) thr…，成立时执行对应分支。
                if (!args.value(mode, Json()).is_array() || args[mode].empty()) throw std::invalid_argument("每组推荐须有实际目标");
                // 【逐行批注】 执行当前声明或表达式：std::set<std::string> seen;。
                std::set<std::string> seen;
                // 【逐行批注】 按循环规则重复处理：for (const auto& item : args[mode]) if (!item.is_string() || item.get<std:…。
                for (const auto& item : args[mode]) if (!item.is_string() || item.get<std::string>().empty() ||
                    // 【逐行批注】 调用 invalid_argument 完成本行所需操作：!seen.insert(item.get<std::string>()).second) throw std::invalid_argu…。
                    !seen.insert(item.get<std::string>()).second) throw std::invalid_argument("推荐文字无效或重复");
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
            const auto key = recommendationKey(accessFor(task)), raw = db.profileMeta(key);
            // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
            const auto value = Json{{"status", "ready"}, {"items", {{"lite", args["lite"]}, {"deep", args["deep"]}}},
                // 【逐行批注】 调用 requireText、now 完成本行所需操作：{"reason", requireText(args, "reason")}, {"model", source.model}, {"g…。
                {"reason", requireText(args, "reason")}, {"model", source.model}, {"generatedAt", now()},
                // 【逐行批注】 执行当前声明或表达式：{"learningVersion", task.at("learningVersion")}, {"taskId", task.at("id")}};。
                {"learningVersion", task.at("learningVersion")}, {"taskId", task.at("id")}};
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"updated", true}}, [raw, value, key](Datab…。
            return PreparedAgentTool{{{"updated", true}}, [raw, value, key](Database& connection) {
                // 【逐行批注】 判断条件 (!connection.compareProfileMeta(key, raw, value.dump())) throw s…，成立时执行对应分支。
                if (!connection.compareProfileMeta(key, raw, value.dump())) throw std::invalid_argument("推荐已被其他任务更新");
            // 【逐行批注】 执行当前声明或表达式：}};。
            }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentUndoChange，执行该函数声明的职责。
Json agentUndoChange(Database& db, const AgentAccess& access, const Json& body) {
    // 【逐行批注】 执行当前声明或表达式：Json change;。
    Json change;
    // 【逐行批注】 执行当前声明或表达式：db.transaction([&] {。
    db.transaction([&] {
        // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
        const auto row = db.getClassroomActivity(requireText(body, "changeId"));
        // 【逐行批注】 判断条件 (!row || row->kind != "agent-change") throw std::invalid_argumen…，成立时执行对应分支。
        if (!row || row->kind != "agent-change") throw std::invalid_argument("调整记录不存在");
        // 【逐行批注】 计算右侧结果并写入 change，更新当前运行状态。
        change = parse(row->payload);
        // 【逐行批注】 判断条件 (change.value("scopeId", "") != access.scopeId) throw std::inval…，成立时执行对应分支。
        if (change.value("scopeId", "") != access.scopeId) throw std::invalid_argument("无权撤回该调整");
        // 【逐行批注】 判断条件 (change.value("status", "") == "undone") return;，成立时执行对应分支。
        if (change.value("status", "") == "undone") return;
        // 【逐行批注】 判断条件 (change.at("kind") == "goal") {，成立时执行对应分支。
        if (change.at("kind") == "goal") {
            // 【逐行批注】 计算右侧结果并写入 course，更新当前运行状态。
            auto course = requireCourse(db, access, change.at("target"));
            // 【逐行批注】 判断条件 (course.goal != change["after"]["goal"] || course.updatedAt != c…，成立时执行对应分支。
            if (course.goal != change["after"]["goal"] || course.updatedAt != change["after"]["updatedAt"])
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("目标已有后续修改，撤回不能覆盖新的决定");。
                throw std::invalid_argument("目标已有后续修改，撤回不能覆盖新的决定");
            // 【逐行批注】 计算右侧结果并写入 course.goal，更新当前运行状态。
            course.goal = change["before"]["goal"]; course.updatedAt = now();
            // 【逐行批注】 判断条件 (!db.update(course)) throw std::runtime_error("目标撤回失败");，成立时执行对应分支。
            if (!db.update(course)) throw std::runtime_error("目标撤回失败");
        // 【逐行批注】 执行当前声明或表达式：} else if (change.at("kind") == "schedule") {。
        } else if (change.at("kind") == "schedule") {
            // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
            const auto key = change.at("target").get<std::string>(), raw = db.profileMeta(key);
            // 【逐行批注】 判断条件 (raw != change["after"]) throw std::invalid_argument("课表已有后续修改，撤…，成立时执行对应分支。
            if (raw != change["after"]) throw std::invalid_argument("课表已有后续修改，撤回不能覆盖新的安排");
            // 【逐行批注】 计算右侧结果并写入 before，更新当前运行状态。
            auto before = parse(change["before"]), after = parse(raw);
            // 【逐行批注】 计算右侧结果并写入 before["plan"]["version"]，更新当前运行状态。
            before["plan"]["version"] = after["plan"].value("version", 0) + 1;
            // 【逐行批注】 计算右侧结果并写入 before["plan"]["updatedAt"]，更新当前运行状态。
            before["plan"]["updatedAt"] = now();
            // 【逐行批注】 判断条件 (!db.compareProfileMeta(key, raw, before.dump())) throw std::inv…，成立时执行对应分支。
            if (!db.compareProfileMeta(key, raw, before.dump())) throw std::invalid_argument("撤回时课表已经变化");
        // 【逐行批注】 执行当前声明或表达式：} else if (change.at("kind") == "outline") {。
        } else if (change.at("kind") == "outline") {
            // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
            const auto courseId = change.at("target").get<std::string>(); requireCourse(db, access, courseId);
            // 【逐行批注】 计算右侧结果并写入 auto snapshots，更新当前运行状态。
            const auto snapshots = db.findSnapshotsByCourseId(courseId);
            // 【逐行批注】 判断条件 (snapshots.empty()) throw std::invalid_argument("大纲不存在");，成立时执行对应分支。
            if (snapshots.empty()) throw std::invalid_argument("大纲不存在");
            // 【逐行批注】 计算右侧结果并写入 auto latest，更新当前运行状态。
            const auto latest = std::max_element(snapshots.begin(), snapshots.end(), [](const auto& a, const auto& b) { return a.version < b.version; });
            // 【逐行批注】 判断条件 (parse(latest->payload) != change.at("after")) throw std::invali…，成立时执行对应分支。
            if (parse(latest->payload) != change.at("after")) throw std::invalid_argument("大纲已有后续修改，不能覆盖");
            // 【逐行批注】 调用 agentValidateOutline 完成本行所需操作：agentValidateOutline(db, courseId, change.at("after"), change.at("bef…。
            agentValidateOutline(db, courseId, change.at("after"), change.at("before"));
            // 【逐行批注】 判断条件 (!db.insert(CourseSnapshot{newId(), courseId, latest->version + …，成立时执行对应分支。
            if (!db.insert(CourseSnapshot{newId(), courseId, latest->version + 1, change.at("before").dump(), now()}))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("大纲撤回失败");。
                throw std::runtime_error("大纲撤回失败");
        // 【逐行批注】 调用 invalid_argument 完成本行所需操作：} else throw std::invalid_argument("该调整暂不支持撤回");。
        } else throw std::invalid_argument("该调整暂不支持撤回");
        // 【逐行批注】 计算右侧结果并写入 db.markLearningDirty(); change["status"]，更新当前运行状态。
        db.markLearningDirty(); change["status"] = "undone"; change["undoneAt"] = now();
        // 【逐行批注】 计算右侧结果并写入 change["undoFingerprint"]，更新当前运行状态。
        change["undoFingerprint"] = agentLearnerFingerprint(db, access);
        // 【逐行批注】 调用 saveChange 完成本行所需操作：saveChange(db, change, row->courseId);。
        saveChange(db, change, row->courseId);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 结束当前函数并返回结果：return agentSubmit(db, access, {{"type", "undo"}, {"requestId", "undo…。
    return agentSubmit(db, access, {{"type", "undo"}, {"requestId", "undo-" + change.at("id").get<std::string>()},
        // 【逐行批注】 调用 publicChange 完成本行所需操作：{"change", publicChange(change)}, {"text", "我撤回了这次调整，请尊重我的反馈，不立即重复相同改…。
        {"change", publicChange(change)}, {"text", "我撤回了这次调整，请尊重我的反馈，不立即重复相同改动。"}});
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

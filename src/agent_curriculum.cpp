// 【逐行批注】 引入头文件 "agent_curriculum.hpp"，为后续实现提供类型或接口声明。
#include "agent_curriculum.hpp"
// 【逐行批注】 引入头文件 "agent_lessons.hpp"，为后续实现提供类型或接口声明。
#include "agent_lessons.hpp"
// 【逐行批注】 引入头文件 "deep_course.hpp"，为后续实现提供类型或接口声明。
#include "deep_course.hpp"
// 【逐行批注】 引入头文件 "agent_preferences.hpp"，为后续实现提供类型或接口声明。
#include "agent_preferences.hpp"
// 【逐行批注】 引入头文件 "question_evidence.hpp"，为后续实现提供类型或接口声明。
#include "question_evidence.hpp"
// 【逐行批注】 引入头文件 "progress_service.hpp"，为后续实现提供类型或接口声明。
#include "progress_service.hpp"
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
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
Json parse(const std::string& value) { const auto j = Json::parse(value, nullptr, false); return j.is_object() ? j : Json::object(); }
// 【逐行批注】 定义函数 now，执行该函数声明的职责。
std::string now() {
    // 【逐行批注】 计算右侧结果并写入 auto clock，更新当前运行状态。
    const auto clock = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); std::tm value{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&value, &clock);。
    gmtime_s(&value, &clock);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&clock, &value);。
    gmtime_r(&clock, &value);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 调用 put_time 完成本行所需操作：std::ostringstream out; out << std::put_time(&value, "%Y-%m-%dT%H:%M:…。
    std::ostringstream out; out << std::put_time(&value, "%Y-%m-%dT%H:%M:%SZ"); return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 id，执行该函数声明的职责。
std::string id(const char* prefix) {
    // 【逐行批注】 调用 random 完成本行所需操作：static thread_local std::mt19937_64 random(std::random_device{}());。
    static thread_local std::mt19937_64 random(std::random_device{}());
    // 【逐行批注】 调用 random 完成本行所需操作：std::ostringstream out; out << prefix << '-' << std::hex << random() …。
    std::ostringstream out; out << prefix << '-' << std::hex << random() << random(); return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 text，执行该函数声明的职责。
std::string text(const Json& value, const char* key) {
    // 【逐行批注】 判断条件 (!value.value(key, Json()).is_string() || value.at(key).get<std:…，成立时执行对应分支。
    if (!value.value(key, Json()).is_string() || value.at(key).get<std::string>().empty()) throw std::invalid_argument(std::string(key) + "不能为空");
    // 【逐行批注】 结束当前函数并返回结果：return value.at(key);。
    return value.at(key);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 accessFor，执行该函数声明的职责。
AgentAccess accessFor(const Json& task) { return {task.at("scopeId"), task.at("courseIds").get<std::vector<std::string>>()}; }
// 【逐行批注】 定义函数 courseFor，执行该函数声明的职责。
Course courseFor(Database& db, const AgentAccess& access, const std::string& courseId) {
    // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
    const auto course = db.getCourse(courseId);
    // 【逐行批注】 判断条件 (!course || course->status != "active" || std::find(access.cours…，成立时执行对应分支。
    if (!course || course->status != "active" || std::find(access.courseIds.begin(), access.courseIds.end(), courseId) == access.courseIds.end())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课程不存在或无权访问");。
        throw std::invalid_argument("课程不存在或无权访问");
    // 【逐行批注】 结束当前函数并返回结果：return *course;。
    return *course;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 alias，执行该函数声明的职责。
std::string alias(const std::string& course, int phase, int topic) {
    // 【逐行批注】 结束当前函数并返回结果：return "agent-legacy:" + course + ":" + std::to_string(phase) + ":" +…。
    return "agent-legacy:" + course + ":" + std::to_string(phase) + ":" + std::to_string(topic);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 latest，执行该函数声明的职责。
CourseSnapshot latest(Database& db, const std::string& course) {
    // 【逐行批注】 计算右侧结果并写入 auto rows，更新当前运行状态。
    const auto rows = db.findSnapshotsByCourseId(course);
    // 【逐行批注】 判断条件 (rows.empty()) throw std::invalid_argument("课程尚无大纲");，成立时执行对应分支。
    if (rows.empty()) throw std::invalid_argument("课程尚无大纲");
    // 【逐行批注】 结束当前函数并返回结果：return *std::max_element(rows.begin(), rows.end(), [](const auto& a, …。
    return *std::max_element(rows.begin(), rows.end(), [](const auto& a, const auto& b) { return a.version < b.version; });
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 normalizeOutline，执行该函数声明的职责。
Json normalizeOutline(Json payload, const std::string& course) {
    // 【逐行批注】 判断条件 (!payload.value("courseStructure", Json()).is_array() || payload…，成立时执行对应分支。
    if (!payload.value("courseStructure", Json()).is_array() || payload.at("courseStructure").empty())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课程需要真实 AI 选择的阶段与知识点");。
        throw std::invalid_argument("课程需要真实 AI 选择的阶段与知识点");
    // 【逐行批注】 执行当前声明或表达式：std::set<std::string> seen; int phase = 0;。
    std::set<std::string> seen; int phase = 0;
    // 【逐行批注】 按循环规则重复处理：for (auto& stage : payload["courseStructure"]) {。
    for (auto& stage : payload["courseStructure"]) {
        // 【逐行批注】 调用 text 完成本行所需操作：++phase; text(stage, "stage");。
        ++phase; text(stage, "stage");
        // 【逐行批注】 判断条件 (!stage.value("topics", Json()).is_array() || stage.at("topics")…，成立时执行对应分支。
        if (!stage.value("topics", Json()).is_array() || stage.at("topics").empty()) throw std::invalid_argument("阶段知识点尚不完整");
        // 【逐行批注】 计算右侧结果并写入 int topic，更新当前运行状态。
        int topic = 0;
        // 【逐行批注】 按循环规则重复处理：for (auto& item : stage["topics"]) {。
        for (auto& item : stage["topics"]) {
            // 【逐行批注】 执行当前声明或表达式：++topic;。
            ++topic;
            // 【逐行批注】 判断条件 (item.is_string()) item = Json{{"title", item}};，成立时执行对应分支。
            if (item.is_string()) item = Json{{"title", item}};
            // 【逐行批注】 调用 text 完成本行所需操作：text(item, "title");。
            text(item, "title");
            // 【逐行批注】 判断条件 (!item.contains("id")) item["id"] = "topic:" + course + ":" + st…，成立时执行对应分支。
            if (!item.contains("id")) item["id"] = "topic:" + course + ":" + std::to_string(phase) + ":" + std::to_string(topic);
            // 【逐行批注】 计算右侧结果并写入 auto stable，更新当前运行状态。
            const auto stable = text(item, "id"); if (!seen.insert(stable).second) throw std::invalid_argument("知识点身份重复");
            // 【逐行批注】 判断条件 (!item.contains("legacyPhaseIndex")) item["legacyPhaseIndex"] = …，成立时执行对应分支。
            if (!item.contains("legacyPhaseIndex")) item["legacyPhaseIndex"] = phase;
            // 【逐行批注】 判断条件 (!item.contains("legacyTopicIndex")) item["legacyTopicIndex"] = …，成立时执行对应分支。
            if (!item.contains("legacyTopicIndex")) item["legacyTopicIndex"] = topic;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 老页面的路线展示是同一大纲的投影，不额外决定教学内容或题量。
    // 【逐行批注】 计算右侧结果并写入 auto original，更新当前运行状态。
    const auto original = payload.value("roadmap", Json::array()); payload["roadmap"] = Json::array();
    // 【逐行批注】 计算右侧结果并写入 size_t index，更新当前运行状态。
    size_t index = 0;
    // 【逐行批注】 按循环规则重复处理：for (const auto& stage : payload.at("courseStructure")) {。
    for (const auto& stage : payload.at("courseStructure")) {
        // 【逐行批注】 计算右侧结果并写入 projection，更新当前运行状态。
        auto projection = original.is_array() && index < original.size() && original.at(index).is_object() ? original.at(index) : Json::object();
        // 【逐行批注】 计算右侧结果并写入 projection["name"]，更新当前运行状态。
        projection["name"] = stage.at("stage"); projection["steps"] = stage.at("topics");
        // 【逐行批注】 执行当前声明或表达式：payload["roadmap"].push_back(projection); ++index;。
        payload["roadmap"].push_back(projection); ++index;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return payload;。
    return payload;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 topicMap，执行该函数声明的职责。
Json topicMap(const Json& payload) {
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = Json::object();
    // 【逐行批注】 按循环规则重复处理：for (const auto& stage : payload.at("courseStructure")) for (const auto& t…。
    for (const auto& stage : payload.at("courseStructure")) for (const auto& topic : stage.at("topics")) result[topic.at("id").get<std::string>()] = topic;
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentOutline，执行该函数声明的职责。
Json agentOutline(Database& db, const AgentAccess& access, const std::string& courseId) {
    // 【逐行批注】 调用 courseFor、normalizeOutline、parse、latest 完成本行所需操作：courseFor(db, access, courseId); return normalizeOutline(parse(latest…。
    courseFor(db, access, courseId); return normalizeOutline(parse(latest(db, courseId).payload), courseId);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 agentValidateOutline，执行该函数声明的职责。
void agentValidateOutline(Database& db, const std::string& courseId, const Json& before, const Json& after) {
    // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
    const auto previous = topicMap(before), proposed = topicMap(after);
    // 【逐行批注】 执行当前声明或表达式：std::map<std::pair<int, int>, std::string> oldAliases, aliases;。
    std::map<std::pair<int, int>, std::string> oldAliases, aliases;
    // 【逐行批注】 按循环规则重复处理：for (const auto& entry : previous.items()) oldAliases[{entry.value().at("l…。
    for (const auto& entry : previous.items()) oldAliases[{entry.value().at("legacyPhaseIndex"), entry.value().at("legacyTopicIndex")}] = entry.key();
    // 【逐行批注】 按循环规则重复处理：for (const auto& entry : proposed.items()) {。
    for (const auto& entry : proposed.items()) {
        // 【逐行批注】 计算右侧结果并写入 auto item，更新当前运行状态。
        const auto item = entry.value();
        // 【逐行批注】 判断条件 (!item.at("legacyPhaseIndex").is_number_integer() || !item.at("l…，成立时执行对应分支。
        if (!item.at("legacyPhaseIndex").is_number_integer() || !item.at("legacyTopicIndex").is_number_integer()) throw std::invalid_argument("知识点映射无效");
        // 【逐行批注】 计算右侧结果并写入 int phase，更新当前运行状态。
        const int phase = item.at("legacyPhaseIndex"), topic = item.at("legacyTopicIndex"); const auto position = std::make_pair(phase, topic);
        // 【逐行批注】 判断条件 (phase < 1 || topic < 1 || aliases.count(position)) throw std::i…，成立时执行对应分支。
        if (phase < 1 || topic < 1 || aliases.count(position)) throw std::invalid_argument("原链接映射重复或无效");
        // 【逐行批注】 计算右侧结果并写入 aliases[position]，更新当前运行状态。
        aliases[position] = entry.key();
        // 【逐行批注】 判断条件 (previous.contains(entry.key()) && (previous.at(entry.key()).at(…，成立时执行对应分支。
        if (previous.contains(entry.key()) && (previous.at(entry.key()).at("legacyPhaseIndex") != phase || previous.at(entry.key()).at("legacyTopicIndex") != topic))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("重排不能改变知识点原有映射");。
            throw std::invalid_argument("重排不能改变知识点原有映射");
        // 【逐行批注】 判断条件 (oldAliases.count(position) && oldAliases.at(position) != entry.…，成立时执行对应分支。
        if (oldAliases.count(position) && oldAliases.at(position) != entry.key()) throw std::invalid_argument("新知识点不能占用其他知识点的旧链接");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& entry : previous.items()) {。
    for (const auto& entry : previous.items()) {
        // 【逐行批注】 计算右侧结果并写入 auto topic，更新当前运行状态。
        const auto topic = entry.value(); const int phase = topic.at("legacyPhaseIndex"), index = topic.at("legacyTopicIndex");
        // 【逐行批注】 计算右侧结果并写入 auto progress，更新当前运行状态。
        const auto progress = db.findLearningCardProgress(courseId, phase, index);
        // 【逐行批注】 计算右侧结果并写入 bool protectedTopic，更新当前运行状态。
        const bool protectedTopic = db.findLearningSession(courseId, phase, index).has_value() ||
            // 【逐行批注】 调用 alias 完成本行所需操作：!db.profileMeta(alias(courseId, phase, index)).empty() || (progress &…。
            !db.profileMeta(alias(courseId, phase, index)).empty() || (progress && progress->status != "not_started");
        // 【逐行批注】 判断条件 (protectedTopic && (!proposed.contains(entry.key()) || proposed.…，成立时执行对应分支。
        if (protectedTopic && (!proposed.contains(entry.key()) || proposed.at(entry.key()) != topic)) throw std::invalid_argument("已展示或在学知识点及原有身份不能改写或删除");
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentLegacyLesson，执行该函数声明的职责。
Json agentLegacyLesson(Database& db, const AgentAccess& access, const Json& scope) {
    // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
    const auto course = courseFor(db, access, text(scope, "courseId"));
    // 【逐行批注】 计算右侧结果并写入 int phase，更新当前运行状态。
    int phase = scope.value("phaseIndex", 1), topic = scope.value("topicIndex", 1);
    // 【逐行批注】 判断条件 (scope.value("topicId", "") != "") {，成立时执行对应分支。
    if (scope.value("topicId", "") != "") {
        // 【逐行批注】 计算右侧结果并写入 auto outline，更新当前运行状态。
        const auto outline = normalizeOutline(parse(latest(db, course.id).payload), course.id);
        // 【逐行批注】 计算右侧结果并写入 bool found，更新当前运行状态。
        bool found = false;
        // 【逐行批注】 按循环规则重复处理：for (const auto& stage : outline.at("courseStructure")) for (const auto& i…。
        for (const auto& stage : outline.at("courseStructure")) for (const auto& item : stage.at("topics")) if (item.at("id") == scope.at("topicId")) {
            // 【逐行批注】 计算右侧结果并写入 phase，更新当前运行状态。
            phase = item.at("legacyPhaseIndex"); topic = item.at("legacyTopicIndex"); found = true;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (!found) throw std::invalid_argument("原知识点已调整，请查看最新课程路线");，成立时执行对应分支。
        if (!found) throw std::invalid_argument("原知识点已调整，请查看最新课程路线");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto preparedTaskId，更新当前运行状态。
    const auto preparedTaskId = scope.value("lessonTaskId", "");
    // 【逐行批注】 计算右侧结果并写入 session，更新当前运行状态。
    auto session = db.findLearningSession(course.id, phase, topic);
    // 【逐行批注】 执行当前声明或表达式：Json prepared;。
    Json prepared;
    // 【逐行批注】 判断条件 (!preparedTaskId.empty()) {，成立时执行对应分支。
    if (!preparedTaskId.empty()) {
        // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
        const auto row = db.getClassroomActivity("prepared-lesson:" + preparedTaskId);
        // 【逐行批注】 判断条件 (!row || row->kind != "prepared-lesson" || row->courseId != cour…，成立时执行对应分支。
        if (!row || row->kind != "prepared-lesson" || row->courseId != course.id) throw std::invalid_argument("原已备课堂不存在或所属课程不匹配");
        // 【逐行批注】 计算右侧结果并写入 prepared，更新当前运行状态。
        prepared = parse(row->payload);
        // 【逐行批注】 判断条件 (prepared.value("lessonTaskId", "") != preparedTaskId || !prepar…，成立时执行对应分支。
        if (prepared.value("lessonTaskId", "") != preparedTaskId || !prepared.value("content", Json()).is_object()) throw std::invalid_argument("原已备课堂内容无效");
        // 【逐行批注】 计算右侧结果并写入 phase，更新当前运行状态。
        phase = prepared.at("phaseIndex"); topic = prepared.at("topicIndex");
        // 【逐行批注】 计算右侧结果并写入 LearningSession original; original.id，更新当前运行状态。
        LearningSession original; original.id = "prepared-" + preparedTaskId; original.courseId = course.id;
        // 【逐行批注】 计算右侧结果并写入 original.phaseIndex，更新当前运行状态。
        original.phaseIndex = phase; original.topicIndex = topic; original.content = prepared.at("content").dump();
        // 【逐行批注】 计算右侧结果并写入 original.title，更新当前运行状态。
        original.title = prepared.value("topicTitle", course.title); session = original;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
    const auto key = preparedTaskId.empty() ? alias(course.id, phase, topic) : "agent-legacy-prepared:" + preparedTaskId;
    // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
    const auto previous = db.profileMeta(key);
    // 【逐行批注】 判断条件 (!previous.empty()) return {{"lessonId", previous}, {"href", "/a…，成立时执行对应分支。
    if (!previous.empty()) return {{"lessonId", previous}, {"href", "/agent-classroom.html?lessonId=" + previous}};
    // 【逐行批注】 判断条件 (!session) return Json::object();，成立时执行对应分支。
    if (!session) return Json::object();
    // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
    Json lesson = {{"id", "legacy-" + session->id}, {"scopeId", access.scopeId}, {"courseId", course.id},
        // 【逐行批注】 执行当前声明或表达式：{"title", session->title}, {"purpose", "继续原有课堂，保留已展示课件与实际作答。"}, {"status", "ready"},。
        {"title", session->title}, {"purpose", "继续原有课堂，保留已展示课件与实际作答。"}, {"status", "ready"},
        // 【逐行批注】 调用 now 完成本行所需操作：{"version", 1}, {"sourceLearningVersion", db.learningRevision()}, {"e…。
        {"version", 1}, {"sourceLearningVersion", db.learningRevision()}, {"entered", true}, {"updatedAt", now()},
        // 【逐行批注】 调用 array 完成本行所需操作：{"phaseIndex", phase}, {"topicIndex", topic}, {"legacySessionId", ses…。
        {"phaseIndex", phase}, {"topicIndex", topic}, {"legacySessionId", session->id}, {"sections", Json::array()}, {"model", ""}};
    // 【逐行批注】 判断条件 (!preparedTaskId.empty()) {，成立时执行对应分支。
    if (!preparedTaskId.empty()) {
        // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
        const auto kind = prepared.value("kind", "lesson");
        // 【逐行批注】 计算右侧结果并写入 lesson["intent"]，更新当前运行状态。
        lesson["intent"] = kind == "lesson" ? "advance" : kind == "review" ? "review" : "reinforce";
        // 【逐行批注】 计算右侧结果并写入 lesson["completed"]，更新当前运行状态。
        lesson["completed"] = prepared.value("progress", Json::object()).value("status", "") == "completed";
        // 【逐行批注】 判断条件 (kind != "lesson") lesson.erase("legacySessionId");，成立时执行对应分支。
        if (kind != "lesson") lesson.erase("legacySessionId");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto content，更新当前运行状态。
    const auto content = parse(session->content), blocks = content.value("blocks", content);
    // 【逐行批注】 计算右侧结果并写入 auto add，更新当前运行状态。
    const auto add = [&](const std::string& kind, int index, const Json& input, const std::string& title, bool question) {
        // 【逐行批注】 计算右侧结果并写入 Json section，更新当前运行状态。
        Json section = {{"id", lesson.at("id").get<std::string>() + ":" + kind + ":" + std::to_string(index)},
            // 【逐行批注】 执行当前声明或表达式：{"kind", question ? "question" : "explanation"}, {"legacyKind", kind}, {"legacyIndex…。
            {"kind", question ? "question" : "explanation"}, {"legacyKind", kind}, {"legacyIndex", index},
            // 【逐行批注】 执行当前声明或表达式：{"title", title}, {"version", content.value("contentVersion", 1)}, {"displayed", tru…。
            {"title", title}, {"version", content.value("contentVersion", 1)}, {"displayed", true}};
        // 【逐行批注】 判断条件 (question) {，成立时执行对应分支。
        if (question) {
            // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
            auto value = input;
            // 【逐行批注】 判断条件 (!value.value("question", Json()).is_string()) value["question"]…，成立时执行对应分支。
            if (!value.value("question", Json()).is_string()) value["question"] = value.value("task", value.value("content", ""));
            // 【逐行批注】 判断条件 (value.value("question", "").empty()) return;，成立时执行对应分支。
            if (value.value("question", "").empty()) return;
            // 【逐行批注】 计算右侧结果并写入 value["type"]，更新当前运行状态。
            value["type"] = value.value("type", value.contains("options") ? "choice" : "open");
            // 【逐行批注】 判断条件 (!value.contains("expectedAnswer")) value["expectedAnswer"] = va…，成立时执行对应分支。
            if (!value.contains("expectedAnswer")) value["expectedAnswer"] = value.value("solution", value.value("check", ""));
            // 【逐行批注】 计算右侧结果并写入 section["question"]，更新当前运行状态。
            section["question"] = value; section["questionId"] = questionIdentity(course.id, value);
            // 【逐行批注】 计算右侧结果并写入 section["legacyDialog"]，更新当前运行状态。
            section["legacyDialog"] = Json::array();
            // 【逐行批注】 计算右侧结果并写入 auto sourceKey，更新当前运行状态。
            const auto sourceKey = "classroom:" + course.id + ":" + std::to_string(phase) + ":" + std::to_string(topic) +
                // 【逐行批注】 调用 to_string 完成本行所需操作：(preparedTaskId.empty() ? "" : ":task:" + preparedTaskId) + ":" + kin…。
                (preparedTaskId.empty() ? "" : ":task:" + preparedTaskId) + ":" + kind + ":" + std::to_string(index);
            // 【逐行批注】 计算右侧结果并写入 auto history，更新当前运行状态。
            const auto history = db.getClassroomActivity(sourceKey + ":dialogue");
            // 【逐行批注】 判断条件 (history) for (const auto& turn : parse(history->payload).value(…，成立时执行对应分支。
            if (history) for (const auto& turn : parse(history->payload).value("turns", Json::array())) {
                // 【逐行批注】 判断条件 (turn.value("user", Json()).is_string()) section["legacyDialog"]…，成立时执行对应分支。
                if (turn.value("user", Json()).is_string()) section["legacyDialog"].push_back({{"role", "user"}, {"text", turn.at("user")}});
                // 【逐行批注】 判断条件 (turn.value("assistant", Json()).is_string() && !turn.at("assist…，成立时执行对应分支。
                if (turn.value("assistant", Json()).is_string() && !turn.at("assistant").get<std::string>().empty())
                    // 【逐行批注】 执行当前声明或表达式：section["legacyDialog"].push_back({{"role", "assistant"}, {"text", turn.at("assistan…。
                    section["legacyDialog"].push_back({{"role", "assistant"}, {"text", turn.at("assistant")}, {"partial", turn.value("status", "") != "ready"}});
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
            const auto previous = db.getClassroomActivity(sourceKey);
            // 【逐行批注】 判断条件 (section.at("legacyDialog").empty() && previous) {，成立时执行对应分支。
            if (section.at("legacyDialog").empty() && previous) {
                // 【逐行批注】 计算右侧结果并写入 auto record，更新当前运行状态。
                const auto record = parse(previous->payload);
                // 【逐行批注】 判断条件 (record.value("status", "") == "answered" && record.contains("an…，成立时执行对应分支。
                if (record.value("status", "") == "answered" && record.contains("answer")) {
                    // 【逐行批注】 计算右侧结果并写入 auto answer，更新当前运行状态。
                    const auto answer = record.at("answer");
                    // 【逐行批注】 计算右侧结果并写入 auto content，更新当前运行状态。
                    const auto content = answer.is_string() ? answer.get<std::string>() : answer.dump();
                    // 【逐行批注】 执行当前声明或表达式：section["legacyDialog"].push_back({{"role", "user"}, {"text", content}});。
                    section["legacyDialog"].push_back({{"role", "user"}, {"text", content}});
                    // 【逐行批注】 判断条件 (record.value("feedback", Json()).is_string() && !record.at("fee…，成立时执行对应分支。
                    if (record.value("feedback", Json()).is_string() && !record.at("feedback").get<std::string>().empty())
                        // 【逐行批注】 执行当前声明或表达式：section["legacyDialog"].push_back({{"role", "assistant"}, {"text", record.at("feedba…。
                        section["legacyDialog"].push_back({{"role", "assistant"}, {"text", record.at("feedback")}});
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 执行当前声明或表达式：} else {。
        } else {
            // 【逐行批注】 执行当前声明或表达式：std::string body;。
            std::string body;
            // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"explanation", "example", "action"}) if (input.v…。
            for (const auto* field : {"explanation", "example", "action"}) if (input.value(field, Json()).is_string()) body += input.at(field).get<std::string>() + "\n\n";
            // 【逐行批注】 判断条件 (body.empty()) return;，成立时执行对应分支。
            if (body.empty()) return;
            // 【逐行批注】 计算右侧结果并写入 section["body"]，更新当前运行状态。
            section["body"] = body;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 move 完成本行所需操作：lesson["sections"].push_back(std::move(section));。
        lesson["sections"].push_back(std::move(section));
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, int> legacyIndices;。
    std::map<std::string, int> legacyIndices;
    // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.listClassroomActivities(course.id)) if (row.phas…。
    for (const auto& row : db.listClassroomActivities(course.id)) if (row.phaseIndex == phase && row.topicIndex == topic &&
        // 【逐行批注】 执行当前声明或表达式：(row.kind == "diagnostic" || row.kind == "interaction")) {。
        (row.kind == "diagnostic" || row.kind == "interaction")) {
        // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
        const auto input = parse(row.payload);
        // 【逐行批注】 判断条件 (input.value("lessonTaskId", "") != preparedTaskId) continue;，成立时执行对应分支。
        if (input.value("lessonTaskId", "") != preparedTaskId) continue;
        // 【逐行批注】 调用 add 完成本行所需操作：add(row.kind, input.value("index", legacyIndices[row.kind]++), input,…。
        add(row.kind, input.value("index", legacyIndices[row.kind]++), input, row.kind == "diagnostic" ? "原诊断题" : "原互动题", true);
        // 【逐行批注】 判断条件 (!lesson.at("sections").empty()) lesson["sections"].back()["sour…，成立时执行对应分支。
        if (!lesson.at("sections").empty()) lesson["sections"].back()["sourceActivityId"] = row.id;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto* name : {"steps", "examples", "practice", "quiz"}) {。
    for (const auto* name : {"steps", "examples", "practice", "quiz"}) {
        // 【逐行批注】 计算右侧结果并写入 auto block，更新当前运行状态。
        const auto block = blocks.value(name, Json::object());
        // 【逐行批注】 计算右侧结果并写入 auto values，更新当前运行状态。
        const auto values = block.is_array() ? block : block.value(std::string(name) == "steps" ? "lessonSteps" : name, Json::array());
        // 【逐行批注】 判断条件 (!values.is_array()) continue;，成立时执行对应分支。
        if (!values.is_array()) continue;
        // 【逐行批注】 计算右侧结果并写入 int index，更新当前运行状态。
        int index = 0; for (const auto& item : values) {
            // 【逐行批注】 调用 add、string 完成本行所需操作：add(std::string(name) == "examples" ? "example" : name, index++, item…。
            add(std::string(name) == "examples" ? "example" : name, index++, item, item.value("title", std::string(name) == "steps" ? "原讲解" : "原课堂题目"), std::string(name) != "steps");
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (lesson.at("sections").empty()) return Json::object();，成立时执行对应分支。
    if (lesson.at("sections").empty()) return Json::object();
    // 【逐行批注】 执行当前声明或表达式：db.transaction([&] {。
    db.transaction([&] {
        // 【逐行批注】 计算右侧结果并写入 auto concurrent，更新当前运行状态。
        const auto concurrent = db.profileMeta(key);
        // 【逐行批注】 判断条件 (!concurrent.empty()) { lesson["id"] = concurrent; return; }，成立时执行对应分支。
        if (!concurrent.empty()) { lesson["id"] = concurrent; return; }
        // 【逐行批注】 判断条件 (!db.upsert(ClassroomActivity{lesson.at("id"), course.id, "agent…，成立时执行对应分支。
        if (!db.upsert(ClassroomActivity{lesson.at("id"), course.id, "agent-lesson", lesson.dump(), now(), phase, topic})) throw std::runtime_error("历史课堂身份保存失败");
        // 【逐行批注】 执行当前声明或表达式：db.setProfileMeta(key, lesson.at("id"));。
        db.setProfileMeta(key, lesson.at("id"));
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 结束当前函数并返回结果：return {{"lessonId", lesson.at("id")}, {"href", "/agent-classroom.htm…。
    return {{"lessonId", lesson.at("id")}, {"href", "/agent-classroom.html?lessonId=" + lesson.at("id").get<std::string>()}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentUpgradeLegacyLesson，执行该函数声明的职责。
void agentUpgradeLegacyLesson(Database& db, const AgentAccess& access, const std::string& lessonId) {
    // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
    const auto row = db.getClassroomActivity(lessonId);
    // 【逐行批注】 判断条件 (!row || row->kind != "agent-lesson") return;，成立时执行对应分支。
    if (!row || row->kind != "agent-lesson") return;
    // 【逐行批注】 计算右侧结果并写入 lesson，更新当前运行状态。
    auto lesson = parse(row->payload);
    // 【逐行批注】 判断条件 (lesson.value("scopeId", "") != access.scopeId) throw std::inval…，成立时执行对应分支。
    if (lesson.value("scopeId", "") != access.scopeId) throw std::invalid_argument("无权读取该课时");
    // 【逐行批注】 调用 courseFor 完成本行所需操作：courseFor(db, access, row->courseId);。
    courseFor(db, access, row->courseId);
    // 【逐行批注】 判断条件 (lesson.value("legacyLayoutVersion", 0) >= 560) return;，成立时执行对应分支。
    if (lesson.value("legacyLayoutVersion", 0) >= 560) return;
    // 【逐行批注】 执行当前声明或表达式：Json content; std::optional<LearningSession> session;。
    Json content; std::optional<LearningSession> session;
    // 【逐行批注】 判断条件 (!lesson.value("legacySessionId", "").empty()) session = db.getL…，成立时执行对应分支。
    if (!lesson.value("legacySessionId", "").empty()) session = db.getLearningSession(lesson.at("legacySessionId"));
    // 【逐行批注】 判断条件 (session) content = parse(session->content);，成立时执行对应分支。
    if (session) content = parse(session->content);
    // 【逐行批注】 在前一条件不成立时继续判断：(lessonId.find("legacy-prepared-") == 0) {。
    else if (lessonId.find("legacy-prepared-") == 0) {
        // 【逐行批注】 计算右侧结果并写入 auto source，更新当前运行状态。
        const auto source = db.getClassroomActivity("prepared-lesson:" + lessonId.substr(16));
        // 【逐行批注】 判断条件 (source) content = parse(source->payload).value("content", Json:…，成立时执行对应分支。
        if (source) content = parse(source->payload).value("content", Json::object());
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!content.is_object() || content.empty()) return;，成立时执行对应分支。
    if (!content.is_object() || content.empty()) return;
    // 【逐行批注】 计算右侧结果并写入 auto blocks，更新当前运行状态。
    const auto blocks = content.value("blocks", content);
    // 【逐行批注】 计算右侧结果并写入 auto append，更新当前运行状态。
    const auto append = [&](const std::string& kind, const std::string& title, const std::string& body) {
        // 【逐行批注】 判断条件 (body.empty()) return;，成立时执行对应分支。
        if (body.empty()) return;
        // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
        const auto id = lessonId + ":" + kind + ":0";
        // 【逐行批注】 按循环规则重复处理：for (const auto& section : lesson.at("sections")) if (section.at("id") == …。
        for (const auto& section : lesson.at("sections")) if (section.at("id") == id) return;
        // 【逐行批注】 执行当前声明或表达式：lesson["sections"].push_back({{"id", id}, {"kind", kind == "overview" ? "explanation…。
        lesson["sections"].push_back({{"id", id}, {"kind", kind == "overview" ? "explanation" : "summary"},
            // 【逐行批注】 执行当前声明或表达式：{"legacyKind", kind}, {"legacyIndex", 0}, {"title", title}, {"body", body},。
            {"legacyKind", kind}, {"legacyIndex", 0}, {"title", title}, {"body", body},
            // 【逐行批注】 执行当前声明或表达式：{"version", content.value("contentVersion", 1)}, {"displayed", false}});。
            {"version", content.value("contentVersion", 1)}, {"displayed", false}});
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto listText，更新当前运行状态。
    const auto listText = [](const Json& items) {
        // 【逐行批注】 执行当前声明或表达式：std::string body;。
        std::string body;
        // 【逐行批注】 判断条件 (items.is_array()) for (const auto& item : items) body += "- " +…，成立时执行对应分支。
        if (items.is_array()) for (const auto& item : items) body += "- " + (item.is_string() ? item.get<std::string>() : item.dump()) + "\n";
        // 【逐行批注】 结束当前函数并返回结果：return body;。
        return body;
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto overview，更新当前运行状态。
    const auto overview = blocks.value("overview", Json::object());
    // 【逐行批注】 调用 append、listText、array 完成本行所需操作：append("overview", overview.value("title", "本节目标与关键概念"), overview.val…。
    append("overview", overview.value("title", "本节目标与关键概念"), overview.value("summary", "") + "\n\n" + listText(overview.value("keyConcepts", Json::array())));
    // 【逐行批注】 计算右侧结果并写入 auto assessment，更新当前运行状态。
    const auto assessment = blocks.value("assessment", Json::object());
    // 【逐行批注】 调用 append、listText、array 完成本行所需操作：append("checkpoint", "学习检查点", listText(assessment.value("checkpoint",…。
    append("checkpoint", "学习检查点", listText(assessment.value("checkpoint", Json::array())));
    // 【逐行批注】 调用 append、listText、array 完成本行所需操作：append("commonMistakes", "常见错误提醒", listText(assessment.value("commonM…。
    append("commonMistakes", "常见错误提醒", listText(assessment.value("commonMistakes", Json::array())));
    // 【逐行批注】 计算右侧结果并写入 auto resourceSummary，更新当前运行状态。
    const auto resourceSummary = assessment.value("resourceSummary", Json());
    // 【逐行批注】 调用 append、listText 完成本行所需操作：append("resourceSummary", "资料说明", resourceSummary.is_string() ? resou…。
    append("resourceSummary", "资料说明", resourceSummary.is_string() ? resourceSummary.get<std::string>() : listText(resourceSummary));
    // 【逐行批注】 计算右侧结果并写入 Json references，更新当前运行状态。
    Json references = content.value("references", Json::array());
    // 【逐行批注】 判断条件 (references.empty() && session && session->references) {，成立时执行对应分支。
    if (references.empty() && session && session->references) {
        // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
        const auto saved = Json::parse(*session->references, nullptr, false);
        // 【逐行批注】 判断条件 (saved.is_array()) references = saved;，成立时执行对应分支。
        if (saved.is_array()) references = saved;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 lesson["references"]，更新当前运行状态。
    lesson["references"] = references;
    // 【逐行批注】 按循环规则重复处理：for (auto& section : lesson["sections"]) {。
    for (auto& section : lesson["sections"]) {
        // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
        const auto kind = section.value("legacyKind", "");
        // 【逐行批注】 判断条件 (kind == "example" && section.at("kind") == "question") {，成立时执行对应分支。
        if (kind == "example" && section.at("kind") == "question") {
            // 【逐行批注】 计算右侧结果并写入 auto question，更新当前运行状态。
            const auto question = section.at("question");
            // 【逐行批注】 计算右侧结果并写入 section["kind"]，更新当前运行状态。
            section["kind"] = "explanation"; section["presentation"] = "example";
            // 【逐行批注】 计算右侧结果并写入 section["body"]，更新当前运行状态。
            section["body"] = question.value("question", question.value("content", "")) + "\n\n" + question.value("solution", question.value("explanation", ""));
            // 保留私有原题及稳定身份供历史证据引用，公开示范不再成为评分题。
        // 【逐行批注】 执行当前声明或表达式：} else if (section.at("kind") == "question" && !section.contains("questionKind"))。
        } else if (section.at("kind") == "question" && !section.contains("questionKind"))
            // 【逐行批注】 计算右侧结果并写入 section["questionKind"]，更新当前运行状态。
            section["questionKind"] = kind == "diagnostic" || kind == "interaction" ? "interaction" : "practice";
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 lesson["legacyLayoutVersion"]，更新当前运行状态。
    lesson["legacyLayoutVersion"] = 560; lesson["version"] = lesson.value("version", 1) + 1;
    // 【逐行批注】 判断条件 (!db.compareClassroomActivity(ClassroomActivity{row->id, row->co…，成立时执行对应分支。
    if (!db.compareClassroomActivity(ClassroomActivity{row->id, row->courseId, row->kind, lesson.dump(), now(), row->phaseIndex, row->topicIndex}, row->payload))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课时已在另一个窗口更新，请重新读取");。
        throw std::invalid_argument("课时已在另一个窗口更新，请重新读取");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentLegacyEvent，执行该函数声明的职责。
Json agentLegacyEvent(Database& db, const AgentAccess& access, const Json& body) {
    // 【逐行批注】 计算右侧结果并写入 auto resolved，更新当前运行状态。
    const auto resolved = body.value("lessonId", "").empty() ? agentLegacyLesson(db, access, body) : Json{{"lessonId", body.at("lessonId")}};
    // 【逐行批注】 判断条件 (resolved.empty()) throw std::invalid_argument("课件尚未准备，请让 AI 备课后…，成立时执行对应分支。
    if (resolved.empty()) throw std::invalid_argument("课件尚未准备，请让 AI 备课后再作答");
    // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
    const auto lesson = agentLessonView(db, access, resolved.at("lessonId"));
    // 【逐行批注】 按循环规则重复处理：for (const auto& section : lesson.at("sections")) if (section.at("kind") =…。
    for (const auto& section : lesson.at("sections")) if (section.at("kind") == "question" &&
        // 【逐行批注】 执行当前声明或表达式：(body.contains("sectionId") ? section.at("id") == body.at("sectionId") :。
        (body.contains("sectionId") ? section.at("id") == body.at("sectionId") :
         // 【逐行批注】 执行当前声明或表达式：body.contains("questionId") ? section.at("questionId") == body.at("questionId") :。
         body.contains("questionId") ? section.at("questionId") == body.at("questionId") :
         // 【逐行批注】 执行当前声明或表达式：section.value("legacyKind", section.value("questionKind", "practice")) == body.value…。
         section.value("legacyKind", section.value("questionKind", "practice")) == body.value("kind", "diagnostic") && section.value("legacyIndex", -1) == body.value("index", 0))) {
        // 【逐行批注】 判断条件 (body.contains("questionId") && body.at("questionId") != section…，成立时执行对应分支。
        if (body.contains("questionId") && body.at("questionId") != section.at("questionId")) throw std::invalid_argument("原题身份已变化");
        // 【逐行批注】 判断条件 (body.contains("sectionVersion") && body.at("sectionVersion") !=…，成立时执行对应分支。
        if (body.contains("sectionVersion") && body.at("sectionVersion") != section.at("version")) throw std::invalid_argument("原题版本已变化");
        // 【逐行批注】 判断条件 (body.contains("contentVersion") && body.at("contentVersion") !=…，成立时执行对应分支。
        if (body.contains("contentVersion") && body.at("contentVersion") != section.at("version")) throw std::invalid_argument("课件版本已变化");
        // 【逐行批注】 计算右侧结果并写入 auto answer，更新当前运行状态。
        const auto answer = body.value("question", body.value("answer", Json("")));
        // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
        auto value = answer.is_string() ? answer.get<std::string>() : answer.dump();
        // 【逐行批注】 判断条件 (answer.is_number_integer() && section.at("question").value("opt…，成立时执行对应分支。
        if (answer.is_number_integer() && section.at("question").value("options", Json()).is_array()) {
            // 【逐行批注】 计算右侧结果并写入 int index，更新当前运行状态。
            const int index = answer.get<int>(); const auto options = section.at("question").at("options");
            // 【逐行批注】 判断条件 (index < 0 || index >= static_cast<int>(options.size())) throw s…，成立时执行对应分支。
            if (index < 0 || index >= static_cast<int>(options.size())) throw std::invalid_argument("选择的选项不存在");
            // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
            value = "我选" + std::to_string(index + 1) + "：" + options.at(index).get<std::string>();
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (value.empty() && body.value("action", "") == "skip") value = "我…，成立时执行对应分支。
        if (value.empty() && body.value("action", "") == "skip") value = "我跳过这道题";
        // 【逐行批注】 结束当前函数并返回结果：return {{"type", "question_answer"}, {"courseId", body.at("courseId")…。
        return {{"type", "question_answer"}, {"courseId", body.at("courseId")}, {"lessonId", resolved.at("lessonId")},
            // 【逐行批注】 执行当前声明或表达式：{"sectionId", section.at("id")}, {"sectionVersion", section.at("version")}, {"text",…。
            {"sectionId", section.at("id")}, {"sectionVersion", section.at("version")}, {"text", value},
            // 【逐行批注】 调用 id 完成本行所需操作：{"action", body.value("action", "answer")}, {"requestId", body.value(…。
            {"action", body.value("action", "answer")}, {"requestId", body.value("requestId", id("legacy-input"))}};
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("原课堂题目不存在");。
    throw std::invalid_argument("原课堂题目不存在");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentQuestionView，执行该函数声明的职责。
Json agentQuestionView(Database& db, const AgentAccess& access, const Json& scope) {
    // 【逐行批注】 计算右侧结果并写入 auto resolved，更新当前运行状态。
    const auto resolved = scope.value("lessonId", "").empty() ? agentLegacyLesson(db, access, scope) : Json{{"lessonId", scope.at("lessonId")}};
    // 【逐行批注】 判断条件 (resolved.empty()) throw std::invalid_argument("课堂尚未备好，请先让 AI 准备…，成立时执行对应分支。
    if (resolved.empty()) throw std::invalid_argument("课堂尚未备好，请先让 AI 准备课程");
    // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
    const auto lesson = agentLessonView(db, access, resolved.at("lessonId")); Json questions = Json::array();
    // 【逐行批注】 计算右侧结果并写入 int index，更新当前运行状态。
    int index = 0;
    // 【逐行批注】 按循环规则重复处理：for (const auto& section : lesson.at("sections")) if (section.at("kind") =…。
    for (const auto& section : lesson.at("sections")) if (section.at("kind") == "question") {
        // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
        const auto kind = section.value("legacyKind", section.value("questionKind", "practice"));
        // 【逐行批注】 判断条件 (scope.value("kind", "").empty() || scope.at("kind") == kind) {，成立时执行对应分支。
        if (scope.value("kind", "").empty() || scope.at("kind") == kind) {
            // 【逐行批注】 计算右侧结果并写入 question，更新当前运行状态。
            auto question = section.at("question"); question["kind"] = kind; question["index"] = section.value("legacyIndex", index);
            // 【逐行批注】 计算右侧结果并写入 question["questionId"]，更新当前运行状态。
            question["questionId"] = section.at("questionId"); question["sectionId"] = section.at("id");
            // 【逐行批注】 计算右侧结果并写入 question["sectionVersion"]，更新当前运行状态。
            question["sectionVersion"] = section.at("version"); question["contentVersion"] = section.at("version");
            // 【逐行批注】 计算右侧结果并写入 question["lessonId"]，更新当前运行状态。
            question["lessonId"] = lesson.at("id"); question["dialog"] = section.value("dialog", Json::array());
            // 【逐行批注】 判断条件 (section.contains("taskId")) { question["taskId"] = section.at("…，成立时执行对应分支。
            if (section.contains("taskId")) { question["taskId"] = section.at("taskId"); question["taskStatus"] = section.at("taskStatus"); }
            // 【逐行批注】 调用 move 完成本行所需操作：questions.push_back(std::move(question));。
            questions.push_back(std::move(question));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：++index;。
        ++index;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return {{"ok", true}, {"lessonId", lesson.at("id")}, {"questions", qu…。
    return {{"ok", true}, {"lessonId", lesson.at("id")}, {"questions", questions}, {"items", questions}, {"contentVersion", lesson.at("version")}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 registerAgentCurriculumTools，执行该函数声明的职责。
void registerAgentCurriculumTools(LearningAgent& agent) {
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("revise_outline", {"依据真实表现重组未来大纲。参数 courseId,payload(完整课程大纲),reas…。
    agent.registerTool("revise_outline", {"依据真实表现重组未来大纲。参数 courseId,payload(完整课程大纲),reason。保留知识点id和legacyPhaseIndex/legacyTopicIndex；已展示、在学、已作答内容必须保持原文，未来知识点数量与顺序由 AI 决定；调整可撤回。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
            const auto course = courseFor(db, accessFor(task), text(args, "courseId"));
            // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
            const auto saved = latest(db, course.id); const auto before = normalizeOutline(parse(saved.payload), course.id);
            // 【逐行批注】 计算右侧结果并写入 after，更新当前运行状态。
            auto after = normalizeOutline(args.at("payload"), course.id); agentValidateOutline(db, course.id, before, after);
            // 【逐行批注】 判断条件 (before.value("pedagogy", "") == "deep-v1") after["pedagogy"] = …，成立时执行对应分支。
            if (before.value("pedagogy", "") == "deep-v1") after["pedagogy"] = "deep-v1";
            // 【逐行批注】 执行前一条件不成立时的分支。
            else after.erase("pedagogy");
            // 【逐行批注】 计算右侧结果并写入 after["generation"]，更新当前运行状态。
            after["generation"] = {{"source", "ai"}, {"model", source.model}, {"promptVersion", "ai-agent-v551"}, {"generatedAt", now()}};
            // 【逐行批注】 计算右侧结果并写入 auto change，更新当前运行状态。
            const auto change = agentChangeRecord(db, task, source, args, "outline", course.id, before, after);
            // 【逐行批注】 计算右侧结果并写入 Json publicChange，更新当前运行状态。
            const Json publicChange = {{"id", change.at("id")}, {"kind", "outline"}, {"target", course.id},
                // 【逐行批注】 执行当前声明或表达式：{"reason", args.at("reason")}, {"status", "applied"}, {"model", source.model}};。
                {"reason", args.at("reason")}, {"status", "applied"}, {"model", source.model}};
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"change", publicChange}, {"version", saved…。
            return PreparedAgentTool{{{"change", publicChange}, {"version", saved.version + 1}}, [saved, after, change, course](Database& connection) {
                // 【逐行批注】 判断条件 (latest(connection, course.id).id != saved.id) throw std::invali…，成立时执行对应分支。
                if (latest(connection, course.id).id != saved.id) throw std::invalid_argument("大纲版本已经变化");
                // 【逐行批注】 判断条件 (!connection.insert(CourseSnapshot{id("snapshot"), course.id, sa…，成立时执行对应分支。
                if (!connection.insert(CourseSnapshot{id("snapshot"), course.id, saved.version + 1, after.dump(), now()})) throw std::runtime_error("大纲保存失败");
                // 【逐行批注】 调用 agentSaveChange 完成本行所需操作：connection.markLearningDirty(); agentSaveChange(connection, change, c…。
                connection.markLearningDirty(); agentSaveChange(connection, change, course.id);
            // 【逐行批注】 执行当前声明或表达式：}};。
            }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("complete_lesson", {"AI 根据真实互动决定本课完成，不使用固定通过率。参数 lessonId,reason；…。
    agent.registerTool("complete_lesson", {"AI 根据真实互动决定本课完成，不使用固定通过率。参数 lessonId,reason；只有实际进入的课时可完成。独立巩固课不增加原大纲的完成数。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto lessonId，更新当前运行状态。
            const auto lessonId = text(args, "lessonId"); const auto row = db.getClassroomActivity(lessonId);
            // 【逐行批注】 判断条件 (!row || row->kind != "agent-lesson") throw std::invalid_argumen…，成立时执行对应分支。
            if (!row || row->kind != "agent-lesson") throw std::invalid_argument("课时不存在");
            // 【逐行批注】 计算右侧结果并写入 lesson，更新当前运行状态。
            auto lesson = parse(row->payload); courseFor(db, accessFor(task), row->courseId);
            // 【逐行批注】 判断条件 (lesson.at("scopeId") != task.at("scopeId") || !lesson.value("en…，成立时执行对应分支。
            if (lesson.at("scopeId") != task.at("scopeId") || !lesson.value("entered", false)) throw std::invalid_argument("课时尚未实际进入");
            // 【逐行批注】 计算右侧结果并写入 text(args, "reason"); lesson["completed"]，更新当前运行状态。
            text(args, "reason"); lesson["completed"] = true; lesson["completionReason"] = args.at("reason"); lesson["completionModel"] = source.model;
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"completed", true}, {"lessonId", lessonId}…。
            return PreparedAgentTool{{{"completed", true}, {"lessonId", lessonId}}, [row, lesson](Database& connection) {
                // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
                const auto current = connection.getClassroomActivity(row->id);
                // 【逐行批注】 判断条件 (!current || current->payload != row->payload) throw std::invali…，成立时执行对应分支。
                if (!current || current->payload != row->payload) throw std::invalid_argument("课时已有后续修改");
                // 【逐行批注】 判断条件 (!connection.upsert(ClassroomActivity{row->id, row->courseId, ro…，成立时执行对应分支。
                if (!connection.upsert(ClassroomActivity{row->id, row->courseId, row->kind, lesson.dump(), now(), row->phaseIndex, row->topicIndex})) throw std::runtime_error("课时完成保存失败");
                // 【逐行批注】 判断条件 (lesson.contains("legacySessionId") || lesson.value("intent", ""…，成立时执行对应分支。
                if (lesson.contains("legacySessionId") || lesson.value("intent", "") == "advance") {
                    // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
                    const auto course = connection.getCourse(row->courseId); if (!course) throw std::invalid_argument("课程不存在");
                    // 【逐行批注】 计算右侧结果并写入 int phase，更新当前运行状态。
                    const int phase = lesson.value("phaseIndex", 0), topic = lesson.value("topicIndex", 0);
                    // 【逐行批注】 判断条件 (phase < 1 || topic < 1) throw std::invalid_argument("推进课缺少稳定知识点…，成立时执行对应分支。
                    if (phase < 1 || topic < 1) throw std::invalid_argument("推进课缺少稳定知识点映射");
                    // 【逐行批注】 计算右侧结果并写入 auto existing，更新当前运行状态。
                    const auto existing = connection.findLearningCardProgress(row->courseId, phase, topic);
                    // 【逐行批注】 计算右侧结果并写入 progress，更新当前运行状态。
                    auto progress = existing.value_or(LearningCardProgress{}); if (progress.id.empty()) progress.id = id("card");
                    // 【逐行批注】 计算右侧结果并写入 progress.courseId，更新当前运行状态。
                    progress.courseId = course->id; progress.anonymousId = course->anonymousId; progress.goal = course->goal; progress.mode = course->mode;
                    // 【逐行批注】 计算右侧结果并写入 progress.phaseIndex，更新当前运行状态。
                    progress.phaseIndex = phase; progress.topicIndex = topic; progress.phaseName = lesson.value("phaseName", ""); progress.topicTitle = lesson.value("topicTitle", lesson.at("title").get<std::string>()); progress.status = "completed";
                    // 【逐行批注】 判断条件 (!(existing ? connection.update(progress) : connection.insert(pr…，成立时执行对应分支。
                    if (!(existing ? connection.update(progress) : connection.insert(progress))) throw std::runtime_error("原课进度保存失败");
                    // 【逐行批注】 判断条件 (!recomputeCourseProgress(connection, course->id, course->anonym…，成立时执行对应分支。
                    if (!recomputeCourseProgress(connection, course->id, course->anonymousId.value_or(""), course->goal)) throw std::runtime_error("总进度保存失败");
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 执行当前声明或表达式：}};。
            }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("create_course", {"由 AI 自主规划并保存新课程。参数 goal,title,mode:lite|deep,s…。
    agent.registerTool("create_course", {"由 AI 自主规划并保存新课程。参数 goal,title,mode:lite|deep,summary,payload:{courseStructure:[{stage,topics:[知识点文字或{title,id}]}],roadmap可选,其他公开规划资料可选}。阶段及知识点数量由你决定，不套固定模板。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database&, const Json& args, const Json& task, const AIResult& source) {。
        [](Database&, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
            const auto courseId = id("course");
            // 【逐行批注】 计算右侧结果并写入 auto mode，更新当前运行状态。
            const auto mode = task.at("event").value("type", "") == "plan_course"
                // 【逐行批注】 执行当前声明或表达式：? task.at("event").value("mode", args.value("mode", "deep")) : args.value("mode", "d…。
                ? task.at("event").value("mode", args.value("mode", "deep")) : args.value("mode", "deep");
            // 【逐行批注】 判断条件 (mode != "deep" && mode != "lite") throw std::invalid_argument("…，成立时执行对应分支。
            if (mode != "deep" && mode != "lite") throw std::invalid_argument("课程模式无效");
            // 【逐行批注】 计算右侧结果并写入 payload，更新当前运行状态。
            auto payload = normalizeOutline(args.at("payload"), courseId);
            // 只给本次明确选择深度的新建课程加标记，旧课原始mode保持不变。
            // 【逐行批注】 执行当前声明或表达式：payload.erase("pedagogy");。
            payload.erase("pedagogy");
            // 【逐行批注】 判断条件 (task.at("event").value("type", "") == "plan_course" && task.at(…，成立时执行对应分支。
            if (task.at("event").value("type", "") == "plan_course" && task.at("event").value("mode", "") == "deep")
                // 【逐行批注】 计算右侧结果并写入 payload["pedagogy"]，更新当前运行状态。
                payload["pedagogy"] = "deep-v1";
            // 【逐行批注】 计算右侧结果并写入 payload["title"]，更新当前运行状态。
            payload["title"] = text(args, "title"); payload["summary"] = args.value("summary", "");
            // 【逐行批注】 计算右侧结果并写入 payload["generation"]，更新当前运行状态。
            payload["generation"] = {{"source", "ai"}, {"model", source.model}, {"promptVersion", "ai-agent-v551"}, {"generatedAt", now()}};
            // 【逐行批注】 调用 text、now 完成本行所需操作：const Course course{courseId, {}, {}, text(args, "goal"), mode, text(…。
            const Course course{courseId, {}, {}, text(args, "goal"), mode, text(args, "title"), args.value("summary", ""), "ai", "active", now(), now()};
            // 【逐行批注】 调用 id、now 完成本行所需操作：const CourseSnapshot snapshot{id("snapshot"), courseId, 1, payload.du…。
            const CourseSnapshot snapshot{id("snapshot"), courseId, 1, payload.dump(), now()};
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"createdCourseId", courseId}, {"course", {…。
            return PreparedAgentTool{{{"createdCourseId", courseId}, {"course", {{"id", courseId}, {"title", course.title}, {"href", "/plan?courseId=" + courseId}}}},
                // 【逐行批注】 计算右侧结果并写入 [course, snapshot, scope，更新当前运行状态。
                [course, snapshot, scope = task.at("scopeId")](Database& db) {
                    // 【逐行批注】 判断条件 (!db.insert(course) || !db.insert(snapshot)) throw std::runtime_…，成立时执行对应分支。
                    if (!db.insert(course) || !db.insert(snapshot)) throw std::runtime_error("AI 课程保存失败");
                    // 【逐行批注】 执行当前声明或表达式：db.setProfileMeta("agent-course-owner:" + course.id, scope); db.markLearningDirty();。
                    db.setProfileMeta("agent-course-owner:" + course.id, scope); db.markLearningDirty();
                // 【逐行批注】 执行当前声明或表达式：}};。
                }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("schedule_review", {"由 AI 根据真实证据决定复习时间。参数 courseId,title,due:YYYY…。
    agent.registerTool("schedule_review", {"由 AI 根据真实证据决定复习时间。参数 courseId,title,due:YYYY-MM-DD,reason,lessonId可选,phaseIndex/topicIndex可选。没有固定复习间隔，不自动生成替代题。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
            const auto course = courseFor(db, accessFor(task), text(args, "courseId"));
            // 【逐行批注】 计算右侧结果并写入 auto due，更新当前运行状态。
            const auto due = text(args, "due"); std::tm date{}; std::istringstream input(due); input >> std::get_time(&date, "%Y-%m-%d");
            // 【逐行批注】 判断条件 (input.fail() || due.size() != 10) throw std::invalid_argument("…，成立时执行对应分支。
            if (input.fail() || due.size() != 10) throw std::invalid_argument("复习日期无效");
            // 【逐行批注】 计算右侧结果并写入 date.tm_hour，更新当前运行状态。
            date.tm_hour = 12; std::mktime(&date); std::ostringstream normalized; normalized << std::put_time(&date, "%Y-%m-%d");
            // 【逐行批注】 判断条件 (normalized.str() != due) throw std::invalid_argument("复习日期不存在");，成立时执行对应分支。
            if (normalized.str() != due) throw std::invalid_argument("复习日期不存在");
            // 【逐行批注】 判断条件 (args.value("lessonId", "") != "" && agentLessonView(db, accessF…，成立时执行对应分支。
            if (args.value("lessonId", "") != "" && agentLessonView(db, accessFor(task), args.at("lessonId")).at("courseId") != course.id)
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("复习课时所属课程不匹配");。
                throw std::invalid_argument("复习课时所属课程不匹配");
            // 【逐行批注】 计算右侧结果并写入 auto reviewId，更新当前运行状态。
            const auto reviewId = id("review");
            // 【逐行批注】 计算右侧结果并写入 Json value，更新当前运行状态。
            const Json value = {{"id", reviewId}, {"courseId", course.id}, {"title", text(args, "title")}, {"due", due},
                // 【逐行批注】 调用 text 完成本行所需操作：{"reason", text(args, "reason")}, {"day", 0}, {"status", "pending"}, …。
                {"reason", text(args, "reason")}, {"day", 0}, {"status", "pending"}, {"phaseIndex", args.value("phaseIndex", 0)},
                // 【逐行批注】 执行当前声明或表达式：{"topicIndex", args.value("topicIndex", 0)}, {"lessonId", args.value("lessonId", "")…。
                {"topicIndex", args.value("topicIndex", 0)}, {"lessonId", args.value("lessonId", "")}, {"model", source.model}};
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"reviewId", reviewId}, {"due", due}}, [val…。
            return PreparedAgentTool{{{"reviewId", reviewId}, {"due", due}}, [value, reviewId, course](Database& connection) {
                // 【逐行批注】 判断条件 (!connection.upsert(ClassroomActivity{reviewId, course.id, "revi…，成立时执行对应分支。
                if (!connection.upsert(ClassroomActivity{reviewId, course.id, "review", value.dump(), now(), value.at("phaseIndex"), value.at("topicIndex")})) throw std::runtime_error("复习安排保存失败");
            // 【逐行批注】 执行当前声明或表达式：}};。
            }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("read_legacy_lesson", {"读取原有课堂并建立稳定课时映射，不重写旧题。参数 courseId,phaseIn…。
    agent.registerTool("read_legacy_lesson", {"读取原有课堂并建立稳定课时映射，不重写旧题。参数 courseId,phaseIndex,topicIndex或topicId。可用 read_lesson 继续读取私有评分资料。", false,
        // 【逐行批注】 调用 agentLegacyLesson、accessFor 完成本行所需操作：[](Database& db, const Json& args, const Json& task, const AIResult&)…。
        [](Database& db, const Json& args, const Json& task, const AIResult&) { return PreparedAgentTool{agentLegacyLesson(db, accessFor(task), args), {}}; }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("read_outline", {"读取带稳定知识点 id 与原链接映射的最新大纲。参数 courseId。重排保留既有 id 和…。
    agent.registerTool("read_outline", {"读取带稳定知识点 id 与原链接映射的最新大纲。参数 courseId。重排保留既有 id 和映射，新知识点使用新 id 和未占用映射。", true,
        // 【逐行批注】 调用 agentOutline、accessFor、text 完成本行所需操作：[](Database& db, const Json& args, const Json& task, const AIResult&)…。
        [](Database& db, const Json& args, const Json& task, const AIResult&) { return PreparedAgentTool{agentOutline(db, accessFor(task), text(args, "courseId")), {}}; }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("update_course_preview", {"保存 AI 选择的课程路线说明。参数 courseId,slides:[{t…。
    agent.registerTool("update_course_preview", {"保存 AI 选择的课程路线说明。参数 courseId,slides:[{title,content,bullets:[]可选}],reason。数量和顺序自行决定，不重复生成已有有效预览。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
            const auto course = courseFor(db, accessFor(task), text(args, "courseId")); const auto snapshot = latest(db, course.id);
            // 【逐行批注】 计算右侧结果并写入 auto slides，更新当前运行状态。
            const auto slides = args.at("slides"); if (!slides.is_array() || slides.empty()) throw std::invalid_argument("路线说明尚未完整");
            // 【逐行批注】 计算右侧结果并写入 Json shown，更新当前运行状态。
            Json shown = Json::array(); for (const auto& slide : slides) { Json value = {{"title", text(slide, "title")}, {"content", text(slide, "content")}};
                // 【逐行批注】 判断条件 (slide.contains("bullets")) { if (!slide.at("bullets").is_array(…，成立时执行对应分支。
                if (slide.contains("bullets")) { if (!slide.at("bullets").is_array()) throw std::invalid_argument("路线要点格式无效"); for (const auto& point : slide.at("bullets")) if (!point.is_string()) throw std::invalid_argument("路线要点应为文字"); value["bullets"] = slide.at("bullets"); } shown.push_back(value); }
            // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
            const auto key = "course-preview:" + course.id; const auto old = db.getClassroomActivity(key);
            // 【逐行批注】 计算右侧结果并写入 Json value，更新当前运行状态。
            const Json value = {{"status", "ready"}, {"slides", shown}, {"message", text(args, "reason")}, {"model", source.model}, {"assessedCourseVersion", snapshot.version}, {"learningVersion", task.at("learningVersion")}};
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"previewSaved", true}}, [key, old, value, …。
            return PreparedAgentTool{{{"previewSaved", true}}, [key, old, value, course, snapshot](Database& connection) {
                // 【逐行批注】 判断条件 (latest(connection, course.id).id != snapshot.id || !connection.…，成立时执行对应分支。
                if (latest(connection, course.id).id != snapshot.id || !connection.compareClassroomActivity({key, course.id, "preview", value.dump(), now(), 0, 0}, old ? old->payload : "")) throw std::invalid_argument("预览或课程版本已变化");
            // 【逐行批注】 执行当前声明或表达式：}};。
            }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

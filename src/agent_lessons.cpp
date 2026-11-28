// 【逐行批注】 引入头文件 "agent_lessons.hpp"，为后续实现提供类型或接口声明。
#include "agent_lessons.hpp"
// 【逐行批注】 引入头文件 "learning_library.hpp"，为后续实现提供类型或接口声明。
#include "learning_library.hpp"
// 【逐行批注】 引入头文件 "question_evidence.hpp"，为后续实现提供类型或接口声明。
#include "question_evidence.hpp"
// 【逐行批注】 引入头文件 "teaching_evidence.hpp"，为后续实现提供类型或接口声明。
#include "teaching_evidence.hpp"
#include "learning_state.hpp"
#include "component_activity.hpp"
#include "set_exercise.hpp"
// 【逐行批注】 引入头文件 "agent_curriculum.hpp"，为后续实现提供类型或接口声明。
#include "agent_curriculum.hpp"
// 【逐行批注】 引入头文件 "deep_course.hpp"，为后续实现提供类型或接口声明。
#include "deep_course.hpp"
// 【逐行批注】 引入头文件 "course_service.hpp"，为后续实现提供类型或接口声明。
#include "course_service.hpp"
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
// 【逐行批注】 定义函数 newId，执行该函数声明的职责。
std::string newId(const char* prefix) {
    // 【逐行批注】 调用 generator 完成本行所需操作：static thread_local std::mt19937_64 generator(std::random_device{}());。
    static thread_local std::mt19937_64 generator(std::random_device{}());
    // 【逐行批注】 调用 generator 完成本行所需操作：std::ostringstream out; out << prefix << '-' << std::hex << generator…。
    std::ostringstream out; out << prefix << '-' << std::hex << generator() << generator(); return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 accessFor，执行该函数声明的职责。
AgentAccess accessFor(const Json& task) { return {task.at("scopeId"), task.at("courseIds").get<std::vector<std::string>>()}; }
// 【逐行批注】 定义函数 requireCourse，执行该函数声明的职责。
void requireCourse(Database& db, const AgentAccess& access, const std::string& courseId) {
    // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
    const auto found = db.getCourse(courseId);
    // 【逐行批注】 判断条件 (!found || found->status != "active" || std::find(access.courseI…，成立时执行对应分支。
    if (!found || found->status != "active" || std::find(access.courseIds.begin(), access.courseIds.end(), courseId) == access.courseIds.end())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课程不存在或无权读取");。
        throw std::invalid_argument("课程不存在或无权读取");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 lessonFor，执行该函数声明的职责。
Json lessonFor(Database& db, const AgentAccess& access, const std::string& id) {
    // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
    const auto row = db.getClassroomActivity(id);
    // 【逐行批注】 判断条件 (!row || row->kind != "agent-lesson") throw std::invalid_argumen…，成立时执行对应分支。
    if (!row || row->kind != "agent-lesson") throw std::invalid_argument("AI 课时不存在");
    // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
    auto value = Json::parse(row->payload);
    // 【逐行批注】 判断条件 (value.at("scopeId") != access.scopeId) throw std::invalid_argum…，成立时执行对应分支。
    if (value.at("scopeId") != access.scopeId) throw std::invalid_argument("无权读取该课时");
    // 【逐行批注】 调用 requireCourse 完成本行所需操作：requireCourse(db, access, row->courseId); return value;。
    requireCourse(db, access, row->courseId); return value;
// 【逐行批注】 结束当前代码块。
}

// 新版课堂轮次由程序冻结题数；旧课时没有该字段时继续使用兼容路径。
bool activeTeachingRound(const Json& lesson) {
    return lesson.value("teachingRound", Json::object()).value("status", "") == "active";
}

void requireRoundTask(const Json& task, const Json& lesson) {
    const auto& event = task.at("event");
    // 草稿只能由创建或接续它的备课任务开启轮次，不授予任意课时写入权限。
    if (lesson.at("status") == "draft" && event.value("type", "") == "prepare_next" &&
        lesson.value("taskId", "") == task.at("id") &&
        (event.value("courseId", "").empty() || event.at("courseId") == lesson.at("courseId")) &&
        (event.value("lessonId", "").empty() || event.at("lessonId") == lesson.at("id"))) return;
    if (task.at("event").value("lessonId", "") != lesson.at("id") ||
        lesson.value("teachingTaskId", "") != task.at("id") || lesson.at("status") != "ready")
        throw std::invalid_argument("教学轮次只能由当前课时的在用教学任务修改");
}

int checkedRoundQuestionCount(const Json& value) {
    if (!value.is_number_integer()) throw std::invalid_argument("本轮题数必须是整数");
    if (value.is_number_unsigned()) {
        const auto count = value.get<unsigned long long>();
        if (count < 1 || count > 20) throw std::invalid_argument("本轮题数必须为1至20");
        return static_cast<int>(count);
    }
    const auto count = value.get<long long>();
    if (count < 1 || count > 20) throw std::invalid_argument("本轮题数必须为1至20");
    return static_cast<int>(count);
}

bool roundContainsQuestion(const Json& round, const std::string& questionId) {
    const auto& ids = round.at("questionIds");
    return std::find(ids.begin(), ids.end(), questionId) != ids.end();
}

Json newTeachingRound(const std::string& goal, int count) {
    return {{"roundId", newId("round")}, {"questionCount", count}, {"goal", goal}, {"status", "active"},
        {"questionIds", Json::array()}, {"evaluatedQuestionIds", Json::array()}, {"createdAt", now()}};
}

void saveCurrentRound(Json& lesson, Json round) {
    if (lesson.contains("teachingRound")) {
        if (!lesson.contains("teachingRoundHistory")) lesson["teachingRoundHistory"] = Json::array();
        lesson["teachingRoundHistory"].push_back(lesson.at("teachingRound"));
    }
    lesson["teachingRound"] = std::move(round);
    lesson["version"] = lesson.at("version").get<int>() + 1;
    lesson["updatedAt"] = now();
}

Json publicTeachingRound(const Json& round) {
    Json shown;
    for (const auto* key : {"roundId", "goal", "questionCount", "status", "pendingSectionId", "supersedesRoundId"})
        if (round.contains(key)) shown[key] = round.at(key);
    shown["questionNumber"] = round.at("questionIds").size();
    shown["feedbackCount"] = round.at("evaluatedQuestionIds").size();
    return shown;
}

void requireRoundQuestion(const Json& lesson, const Json& section) {
    if (!lesson.contains("teachingRound")) return;
    if (!activeTeachingRound(lesson) || lesson.at("teachingRound").value("pendingSectionId", "") != section.at("id"))
        throw std::invalid_argument("只能作答或激活本轮当前题，历史题仍可回看和追问");
}
// 课堂交流及显式草稿恢复绑定实际课时；普通旧备课保留原有范围。
// 【逐行批注】 定义函数 requireTeachingTarget，执行该函数声明的职责。
void requireTeachingTarget(const Json& task, const Json& lesson) {
    // 【逐行批注】 计算右侧结果并写入 auto& event，更新当前运行状态。
    const auto& event = task.at("event");
    // 【逐行批注】 计算右侧结果并写入 auto type，更新当前运行状态。
    const auto type = event.value("type", "");
    // 【逐行批注】 判断条件 (type != "question_answer" && type != "chat" && type != "lesson_…，成立时执行对应分支。
    if (type != "question_answer" && type != "chat" && type != "lesson_start" && type != "teaching_request" &&
        // 【逐行批注】 执行当前声明或表达式：!(type == "prepare_next" && event.value("resumeDraft", false))) return;。
        !(type == "prepare_next" && event.value("resumeDraft", false))) return;
    // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
    const auto current = event.value("lessonId", "");
    // 【逐行批注】 判断条件 (current.empty()) return;，成立时执行对应分支。
    if (current.empty()) return;
    // 【逐行批注】 判断条件 (lesson.at("id") != current)，成立时执行对应分支。
    if (lesson.at("id") != current)
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("args.lessonId 必须使用 event.lessonId 中的当前课堂…。
        throw std::invalid_argument("args.lessonId 必须使用 event.lessonId 中的当前课堂标识；不可把本次讲解或题目保存到另一课时");
    // 【逐行批注】 判断条件 (!lesson.value("teachingTaskId", "").empty() && lesson.at("teach…，成立时执行对应分支。
    if (!lesson.value("teachingTaskId", "").empty() && lesson.at("teachingTaskId") != task.at("id"))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("本课已有更新的教学任务，旧输出不应用");。
        throw std::invalid_argument("本课已有更新的教学任务，旧输出不应用");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 currentTeachingTask，执行该函数声明的职责。
Json currentTeachingTask(Database& db, const AgentAccess& access, const Json& lesson) {
    // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
    const auto id = lesson.value("teachingTaskId", "");
    // 【逐行批注】 判断条件 (id.empty()) return Json::object();，成立时执行对应分支。
    if (id.empty()) return Json::object();
    // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
    const auto row = db.getClassroomActivity(id);
    // 【逐行批注】 判断条件 (!row || row->kind != "agent-task") throw std::invalid_argument(…，成立时执行对应分支。
    if (!row || row->kind != "agent-task") throw std::invalid_argument("当前课堂任务不存在");
    // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
    const auto task = Json::parse(row->payload);
    // 【逐行批注】 判断条件 (task.value("scopeId", "") != access.scopeId || task.value("cour…，成立时执行对应分支。
    if (task.value("scopeId", "") != access.scopeId || task.value("courseId", "") != lesson.at("courseId") ||
        // 【逐行批注】 调用 object 完成本行所需操作：task.value("event", Json::object()).value("lessonId", "") != lesson.a…。
        task.value("event", Json::object()).value("lessonId", "") != lesson.at("id"))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("当前课堂任务归属不匹配");。
        throw std::invalid_argument("当前课堂任务归属不匹配");
    // 【逐行批注】 结束当前函数并返回结果：return task;。
    return task;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 canReplaceTeachingTask，执行该函数声明的职责。
bool canReplaceTeachingTask(const Json& task) {
    // 【逐行批注】 计算右侧结果并写入 auto status，更新当前运行状态。
    const auto status = task.value("status", "");
    // 【逐行批注】 结束当前函数并返回结果：return status == "ready" || status == "waiting_student" || status == …。
    return status == "ready" || status == "waiting_student" || status == "cancelled" || status == "superseded";
// 【逐行批注】 结束当前代码块。
}
// AI 选择或追加的当轮问题，应在学生正在使用的页面承接；题目身份与用途仍保留。
// 【逐行批注】 定义函数 responseView，执行该函数声明的职责。
std::string responseView(const Json& lesson, const Json& task) {
    // 【逐行批注】 计算右侧结果并写入 auto& event，更新当前运行状态。
    const auto& event = task.at("event");
    // 【逐行批注】 计算右侧结果并写入 std::string view，更新当前运行状态。
    std::string view = event.value("view", "");
    // 【逐行批注】 判断条件 (view != "learn" && view != "practice") {，成立时执行对应分支。
    if (view != "learn" && view != "practice") {
        // 【逐行批注】 计算右侧结果并写入 view，更新当前运行状态。
        view = "learn";
        // 【逐行批注】 按循环规则重复处理：for (const auto& source : lesson.at("sections")) if (source.at("id") == ev…。
        for (const auto& source : lesson.at("sections")) if (source.at("id") == event.value("sectionId", "")) {
            // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
            const auto kind = source.value("questionKind", source.value("legacyKind", "practice"));
            // 【逐行批注】 判断条件 (kind != "interaction" && kind != "diagnostic") view = "practice…，成立时执行对应分支。
            if (kind != "interaction" && kind != "diagnostic") view = "practice";
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return view;。
    return view;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 lessonTaskView，执行该函数声明的职责。
std::string lessonTaskView(const Json& lesson, const Json& task) {
    // 【逐行批注】 计算右侧结果并写入 auto& event，更新当前运行状态。
    const auto& event = task.at("event");
    // 【逐行批注】 判断条件 (event.value("type", "") == "lesson_finish_request" || event.val…，成立时执行对应分支。
    if (event.value("type", "") == "lesson_finish_request" || event.value("view", "") == "summary") return "summary";
    // 【逐行批注】 结束当前函数并返回结果：return responseView(lesson, task);。
    return responseView(lesson, task);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 queryComponent，执行该函数声明的职责。
std::string queryComponent(const std::string& value) {
    // 【逐行批注】 计算右侧结果并写入 constexpr char digits[]，更新当前运行状态。
    static constexpr char digits[] = "0123456789ABCDEF";
    // 【逐行批注】 执行当前声明或表达式：std::string encoded;。
    std::string encoded;
    // 【逐行批注】 按循环规则重复处理：for (const unsigned char byte : value) {。
    for (const unsigned char byte : value) {
        // 【逐行批注】 判断条件 ((byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') ||，成立时执行对应分支。
        if ((byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') ||
            // 【逐行批注】 执行当前声明或表达式：(byte >= '0' && byte <= '9') || byte == '-' || byte == '_' || byte == '.' || byte ==…。
            (byte >= '0' && byte <= '9') || byte == '-' || byte == '_' || byte == '.' || byte == '~') encoded += static_cast<char>(byte);
        // 【逐行批注】 执行前一条件不成立时的分支。
        else { encoded += '%'; encoded += digits[byte >> 4]; encoded += digits[byte & 15]; }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return encoded;。
    return encoded;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 selectResponseQuestion，执行该函数声明的职责。
void selectResponseQuestion(Json& focus, const Json& lesson, const Json& task, const Json& section) {
    // 【逐行批注】 计算右侧结果并写入 auto& event，更新当前运行状态。
    const auto& event = task.at("event");
    // 【逐行批注】 判断条件 (event.value("lessonId", "") != lesson.at("id") || section.at("k…，成立时执行对应分支。
    if (event.value("lessonId", "") != lesson.at("id") || section.at("kind") != "question") return;
    // 【逐行批注】 计算右侧结果并写入 auto type，更新当前运行状态。
    const auto type = event.value("type", "");
    // 【逐行批注】 判断条件 (type != "question_answer" && type != "chat" && type != "lesson_…，成立时执行对应分支。
    if (type != "question_answer" && type != "chat" && type != "lesson_start" && type != "teaching_request") return;
    // 【逐行批注】 计算右侧结果并写入 auto view，更新当前运行状态。
    const auto view = responseView(lesson, task);
    // 【逐行批注】 计算右侧结果并写入 focus["responseSectionId"]，更新当前运行状态。
    focus["responseSectionId"] = section.at("id"); focus["responseView"] = view;
    // 【逐行批注】 计算右侧结果并写入 focus["responseTaskId"]，更新当前运行状态。
    focus["responseTaskId"] = task.at("id");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 publicLesson，执行该函数声明的职责。
Json publicLesson(const Json& lesson) {
    const auto preferences = teachingPreferencesProjection(lesson);
    // 【逐行批注】 执行当前声明或表达式：Json result;。
    Json result;
    result["teachingPreferences"] = preferences;
    if (lesson.contains("teachingRound")) result["teachingRound"] = publicTeachingRound(lesson.at("teachingRound"));
    if (lesson.contains("teachingRoundHistory")) {
        result["teachingRoundHistory"] = Json::array();
        for (const auto& round : lesson.at("teachingRoundHistory"))
            result["teachingRoundHistory"].push_back(publicTeachingRound(round));
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"id", "courseId", "title", "purpose", "status", "v…。
    for (const auto* key : {"id", "courseId", "title", "purpose", "status", "version", "updatedAt", "model", "sourceLearningVersion", "entered", "completed", "phaseIndex", "topicIndex", "topicId", "phaseName", "topicTitle", "teachingTaskId", "initialTeachingTaskId"})
        // 【逐行批注】 判断条件 (lesson.contains(key)) result[key] = lesson[key];，成立时执行对应分支。
        if (lesson.contains(key)) result[key] = lesson[key];
    // 【逐行批注】 计算右侧结果并写入 result["teachingFocus"]，更新当前运行状态。
    result["teachingFocus"] = Json::object();
    // 【逐行批注】 计算右侧结果并写入 auto focus，更新当前运行状态。
    const auto focus = lesson.value("teachingFocus", Json::object());
    // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"sectionId", "interactionSectionId", "practiceSect…。
    for (const auto* key : {"sectionId", "interactionSectionId", "practiceSectionId", "responseSectionId", "responseView", "responseTaskId", "taskId", "version", "updatedAt"})
        // 【逐行批注】 判断条件 (focus.contains(key)) result["teachingFocus"][key] = focus.at(ke…，成立时执行对应分支。
        if (focus.contains(key)) result["teachingFocus"][key] = focus.at(key);
    // 【逐行批注】 计算右侧结果并写入 result["references"]，更新当前运行状态。
    result["references"] = Json::array();
    // 【逐行批注】 按循环规则重复处理：for (const auto& reference : lesson.value("references", Json::array())) {。
    for (const auto& reference : lesson.value("references", Json::array())) {
        // 【逐行批注】 判断条件 (!reference.is_object()) continue;，成立时执行对应分支。
        if (!reference.is_object()) continue;
        // 【逐行批注】 执行当前声明或表达式：Json shown;。
        Json shown;
        // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"title", "name", "description", "url", "href", "so…。
        for (const auto* key : {"title", "name", "description", "url", "href", "source"})
            // 【逐行批注】 判断条件 (reference.contains(key) && reference.at(key).is_string()) shown…，成立时执行对应分支。
            if (reference.contains(key) && reference.at(key).is_string()) shown[key] = reference.at(key);
        // 【逐行批注】 调用 move 完成本行所需操作：result["references"].push_back(std::move(shown));。
        result["references"].push_back(std::move(shown));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 result["sections"]，更新当前运行状态。
    result["sections"] = Json::array();
    // 【逐行批注】 按循环规则重复处理：for (const auto& section : lesson.at("sections")) {。
    for (const auto& section : lesson.at("sections")) {
        // 【逐行批注】 执行当前声明或表达式：Json shown;。
        Json shown;
        // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"id", "kind", "title", "body", "displayed", "versi…。
        for (const auto* key : {"id", "kind", "title", "body", "displayed", "version", "legacyKind", "legacyIndex", "questionKind", "presentation", "createdAt", "materialOrigin", "citations", "helpType", "targetQuestionId", "rePracticeOf"}) if (section.contains(key)) shown[key] = section[key];
        // 【逐行批注】 判断条件 (section.at("kind") == "question") {，成立时执行对应分支。
        if (section.at("kind") == "question") {
            // 【逐行批注】 执行当前声明或表达式：shown.erase("body"); Json question;。
            shown.erase("body"); Json question;
            // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"question", "options", "type", "materials"}) if (s…。
            for (const auto* key : {"question", "options", "type", "materials"}) if (section.at("question").contains(key)) question[key] = section["question"][key];
            if (section.at("question").contains("setExercise")) question["verifiedChoice"] = true;
            // 【逐行批注】 判断条件 (question.contains("materials")) question["materials"] = teachin…，成立时执行对应分支。
            if (question.contains("materials")) question["materials"] = teachingMaterialSnapshot(question.at("materials"));
            // 【逐行批注】 计算右侧结果并写入 shown["question"]，更新当前运行状态。
            shown["question"] = question; shown["questionId"] = section.at("questionId");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 move 完成本行所需操作：result["sections"].push_back(std::move(shown));。
        result["sections"].push_back(std::move(shown));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
Json publishedEvaluation(const LearningInteraction& row, const Json& value,
                         const LearningInteraction& inputRow, const Json& input) {
    Json result = {{"id", row.id}, {"sectionId", input.value("sectionId", "")},
        {"interactionId", inputRow.id}, {"requestId", input.value("requestId", "")}};
    for (const auto* field : {"questionId", "response", "score", "credible", "status", "feedback", "feedbackHelpType",
                             "reason", "uncertainty", "assisted", "at", "diagnosis", "helpHistory", "helpMode", "rePracticeOf",
                             "selectedOption", "choiceCorrect"})
        if (value.contains(field)) result[field] = value.at(field);
    return result;
}

struct TopicLink {
    std::string topicId;
    std::string edition;
    std::string section;
    std::string source;
};

constexpr auto setOperationsId = "cn-hs-math:set-operations";

TopicLink verifiedSetOperations(Database& db, const AgentAccess& access, const std::string& courseId) {
    const auto chapter = libraryVerifiedCurriculumChapter(db, courseId);
    if (!chapter.is_object()) return {};
    const bool editionA = chapter.at("id") == "pep-a-sets-logic";
    const bool editionB = chapter.at("id") == "pep-b-sets-logic";
    if (!editionA && !editionB) return {};
    const std::string section = editionA ? "1.3" : "1.1.3";
    constexpr auto title = "集合的基本运算";
    // 只接受已对照官方节级记录的完整标题。同名多处的大纲需要先消除歧义。
    std::string topicId;
    try {
        const auto outline = agentOutline(db, access, courseId);
        for (const auto& stage : outline.at("courseStructure")) for (const auto& topic : stage.at("topics")) {
            const auto name = topic.at("title").get<std::string>();
            if (name != title && name != section + " " + title && name != section + "　" + title) continue;
            if (!topicId.empty()) return {};
            topicId = topic.at("id");
        }
    } catch (const std::invalid_argument&) {
        return {};
    }
    return {topicId, chapter.at("edition"), section,
        editionA ? "https://www.moe.gov.cn/jyb_xxgk/s5743/s5744/A06/202304/W020230420309231498801.pdf#page=46"
                 : "https://www.pep.com.cn/gzsxb/xrjbgzsxb/xrjgsbwd/201909/t20190903_1944844.html"};
}

std::map<std::string, TopicLink> relatedTopics(Database& db, const AgentAccess& access,
                                             const std::string& courseId, const std::string& topicId) {
    auto origin = verifiedSetOperations(db, access, courseId);
    if (origin.topicId != topicId) return {{courseId, {topicId, {}, {}, {}}}};
    std::map<std::string, TopicLink> topics{{courseId, origin}};
    for (const auto& related : access.courseIds) {
        if (related == courseId) continue;
        auto target = verifiedSetOperations(db, access, related);
        if (!target.topicId.empty() && target.edition != origin.edition) topics.emplace(related, std::move(target));
    }
    return topics;
}

Json topicEvidence(Database& db, const AgentAccess& access, const Json& lesson,
                   const std::vector<ClassroomActivity>& activities, const std::vector<LearningInteraction>& interactions) {
    const auto topic = lesson.value("topicId", Json());
    if (!topic.is_string() || topic.get_ref<const std::string&>().empty()) return {{"status", "unmapped"}};
    const auto topicId = topic.get<std::string>();
    const auto courseId = lesson.at("courseId").get<std::string>();
    if (std::find(access.courseIds.begin(), access.courseIds.end(), courseId) == access.courseIds.end())
        throw std::invalid_argument("无权读取知识点所属课程");
    const auto topics = relatedTopics(db, access, courseId, topicId);
    const auto& link = topics.at(courseId);
    const auto questionScope = link.edition.empty() ? courseId : setOperationsId;
    std::map<std::string, Json> relatedLessons;
    const auto collect = [&](const std::string& originCourse, const TopicLink& originTopic,
                             const std::vector<ClassroomActivity>& rows) {
        for (const auto& row : rows) {
            if (row.kind != "agent-lesson" || row.courseId != originCourse) continue;
            auto item = Json::parse(row.payload, nullptr, false);
            if (!item.is_object() || item.value("id", "") != row.id || item.value("courseId", "") != originCourse ||
                item.value("scopeId", "") != access.scopeId || item.value("topicId", Json()) != originTopic.topicId ||
                item.value("status", "") != "ready" || !item.value("entered", false) ||
                !item.value("sections", Json()).is_array()) continue;
            relatedLessons.emplace(row.id, std::move(item));
        }
    };
    collect(courseId, link, activities);
    for (const auto& [otherCourse, otherTopic] : topics)
        if (otherCourse != courseId) collect(otherCourse, otherTopic, db.listClassroomActivities(otherCourse));

    std::map<std::string, const LearningInteraction*> inputs;
    for (const auto& row : interactions)
        if (row.kind == "chat-user" && row.courseId && topics.count(*row.courseId)) inputs.emplace(row.id, &row);
    Json basis = {{"id", lesson.at("id")}, {"courseId", courseId},
                  {"sections", Json::array()}, {"evaluations", Json::array()}};
    std::map<std::string, Json> latestEvaluations;
    std::set<std::string> sectionKeys, participatingLessons;
    for (const auto& row : interactions) {
        if (row.kind != "practice" || !row.courseId) continue;
        const auto originTopic = topics.find(*row.courseId);
        if (originTopic == topics.end()) continue;
        const auto value = Json::parse(row.payload, nullptr, false);
        if (!value.is_object() || value.value("source", "") != "ai" || value.value("status", "") != "valid") continue;
        const auto found = inputs.find(value.value("interactionId", ""));
        if (found == inputs.end() || found->second->courseId != row.courseId) continue;
        const auto& inputRow = *found->second;
        const auto input = Json::parse(inputRow.payload, nullptr, false);
        if (!input.is_object() || input.value("scopeId", "") != access.scopeId ||
            (input.contains("topicId") && input.at("topicId") != originTopic->second.topicId) ||
            input.value("type", "") != "question_answer") continue;
        const auto action = input.value("action", "answer");
        if (action != "answer" && action != "unknown") continue;
        const auto origin = relatedLessons.find(input.value("lessonId", ""));
        if (origin == relatedLessons.end() || origin->second.at("courseId") != *row.courseId) continue;
        const auto snapshot = input.value("questionSnapshot", Json::object());
        const auto questionId = value.value("questionId", "");
        if (!snapshot.is_object() || !snapshot.value("question", Json()).is_string() || questionId.empty() ||
            input.value("questionId", "") != questionId || !input.value("text", Json()).is_string() ||
            value.value("response", Json()) != input.at("text") || value.value("questionSnapshot", Json()) != snapshot) continue;
        const auto& sections = origin->second.at("sections");
        const auto item = std::find_if(sections.begin(), sections.end(), [&](const Json& section) {
            return section.is_object() && section.value("id", "") == input.value("sectionId", "") && section.value("kind", "") == "question" &&
                section.value("questionId", "") == questionId && section.value("legacyKind", "") != "example";
        });
        if (item == sections.end() || item->value("question", Json()) != snapshot) continue;
        const auto key = teachingQuestionEvidenceKey(questionScope, snapshot);
        const auto sectionKey = Json::array({origin->first, item->at("id")}).dump();
        if (sectionKeys.insert(sectionKey).second)
            basis["sections"].push_back({{"id", sectionKey}, {"kind", "question"}, {"questionId", key}});
        auto evaluation = publishedEvaluation(row, value, inputRow, input);
        evaluation["questionId"] = key;
        evaluation["sectionId"] = sectionKey;
        evaluation["originLessonId"] = origin->first;
        evaluation["originCourseId"] = *row.courseId;
        evaluation["at"] = inputRow.createdAt;
        latestEvaluations[inputRow.id] = std::move(evaluation);
        participatingLessons.insert(origin->first);
    }

    // 同一输入重复发布评价时取最新一条，按学生实际提交的顺序还原题目经历。
    for (const auto& row : interactions) {
        const auto found = latestEvaluations.find(row.id);
        if (found != latestEvaluations.end()) basis["evaluations"].push_back(std::move(found->second));
    }
    const auto evidence = learningStateProjection(basis).at("evidence");
    const auto eligible = evidence.at("evaluationIds").get<std::set<std::string>>();
    const auto independent = evidence.at("independentEvaluationIds").get<std::set<std::string>>();
    std::set<std::string> seenQuestions;
    Json recent = Json::array();
    std::size_t independentCount = 0;
    for (const auto& evaluation : basis.at("evaluations")) {
        const auto id = evaluation.at("id").get<std::string>();
        const bool supported = eligible.count(id) != 0;
        const bool repeat = !seenQuestions.insert(evaluation.at("questionId").get<std::string>()).second;
        const bool independentFirstAttempt = !repeat && independent.count(id) != 0;
        if (independentFirstAttempt) ++independentCount;
        Json item = {{"evaluationId", id}, {"lessonId", evaluation.at("originLessonId")}, {"questionKey", evaluation.at("questionId")},
            {"at", evaluation.at("at")}, {"supported", supported}, {"repeat", repeat},
            {"independentFirstAttempt", independentFirstAttempt}};
        const auto originCourse = evaluation.at("originCourseId").get<std::string>();
        if (originCourse != courseId) {
            const auto& origin = topics.at(originCourse);
            item["curriculum"] = {{"edition", origin.edition}, {"section", origin.section}, {"href", origin.source}};
        }
        if (supported) item["score"] = evaluation.at("score");
        const auto diagnosis = evaluation.value("diagnosis", Json::object());
        if (diagnosis.is_object()) {
            item["needsClarification"] = diagnosis.value("needsClarification", false);
            const auto summary = diagnosis.value("summary", "");
            if (!summary.empty() && summary.size() <= 800) item["diagnosisSummary"] = summary;
        }
        item["helpMode"] = evaluation.value("helpMode", "unknown");
        recent.push_back(std::move(item));
        if (recent.size() > 12) recent.erase(recent.begin());
    }
    Json result = {{"schemaVersion", 1}, {"status", recent.empty() ? "no_evidence" : "available"},
        {"courseId", courseId}, {"topicId", topicId}, {"lessonCount", participatingLessons.size()},
        {"evaluationCount", basis.at("evaluations").size()}, {"distinctSupportedQuestionCount", evidence.at("distinctQuestionCount")},
        {"assistedQuestionCount", evidence.at("assistedQuestionCount")},
        {"independentFirstAttemptCount", independentCount}, {"recentAttempts", std::move(recent)}};
    if (!link.edition.empty()) result["curriculum"] = {{"knowledgeId", setOperationsId}, {"edition", link.edition},
        {"section", link.section}, {"href", link.source}};
    return result;
}

Json lessonTaskEvents(Database& db, const Json& task) {
    std::map<int, Json> ordered;
    int cursor = 0;
    while (true) {
        const auto rows = db.agentRunEvents(task.at("id"), cursor, 500);
        if (rows.empty()) break;
        const auto previous = cursor;
        for (const auto& raw : rows) {
            const auto event = Json::parse(raw, nullptr, false);
            if (!event.is_object() || !event.value("seq", Json()).is_number_integer()) continue;
            const int seq = event.at("seq"); ordered[seq] = event; cursor = std::max(cursor, seq);
        }
        if (cursor == previous || rows.size() < 500) break;
    }
    for (const auto& event : task.value("events", Json::array())) ordered[event.value("seq", 0)] = event;
    Json result = Json::array();
    for (const auto& item : ordered) if (item.first > 0) result.push_back(item.second);
    return result;
}

Json displayedMessages(Database& db, const Json& task) {
    // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
    const auto raw = db.profileMeta("agent-exposure:" + task.at("id").get<std::string>());
    // 【逐行批注】 计算右侧结果并写入 auto exposure，更新当前运行状态。
    const auto exposure = Json::parse(raw, nullptr, false);
    // 【逐行批注】 计算右侧结果并写入 int observed，更新当前运行状态。
    const int observed = exposure.is_object() ? exposure.value("seq", 0) : 0;
    // 【逐行批注】 计算右侧结果并写入 auto displayed，更新当前运行状态。
    const auto displayed = exposure.is_object() ? exposure.value("displayedSequences", Json()) : Json();
    // 【逐行批注】 计算右侧结果并写入 auto complete，更新当前运行状态。
    const auto complete = exposure.is_object() ? exposure.value("completeSequences", Json::array()) : Json::array();
    // 【逐行批注】 执行当前声明或表达式：std::map<int, std::string> messages;。
    std::map<int, std::string> messages;
    // 【逐行批注】 执行当前声明或表达式：std::map<int, bool> fullyDisplayed;。
    std::map<int, bool> fullyDisplayed;
    // 【逐行批注】 按循环规则重复处理：for (const auto& event : lessonTaskEvents(db, task))。
    for (const auto& event : lessonTaskEvents(db, task))
        // 【逐行批注】 判断条件 (event.value("type", "") == "delta" && event.value("field", "") …，成立时执行对应分支。
        if (event.value("type", "") == "delta" && event.value("field", "") == "message") {
            // 【逐行批注】 计算右侧结果并写入 int step，更新当前运行状态。
            const int step = event.value("step", 0);
            // 【逐行批注】 判断条件 (!fullyDisplayed.count(step)) fullyDisplayed[step] = true;，成立时执行对应分支。
            if (!fullyDisplayed.count(step)) fullyDisplayed[step] = true;
            // 【逐行批注】 计算右侧结果并写入 fullyDisplayed[step]，更新当前运行状态。
            fullyDisplayed[step] = fullyDisplayed[step] && complete.is_array() &&
                // 【逐行批注】 调用 find 完成本行所需操作：std::find(complete.begin(), complete.end(), event.at("seq")) != compl…。
                std::find(complete.begin(), complete.end(), event.at("seq")) != complete.end();
            // 【逐行批注】 判断条件 (event.value("seq", 0) <= observed && (!displayed.is_array() || …，成立时执行对应分支。
            if (event.value("seq", 0) <= observed && (!displayed.is_array() || std::find(displayed.begin(), displayed.end(), event.at("seq")) != displayed.end()))
            // 【逐行批注】 计算右侧结果并写入 messages[event.value("step", 0)] +，更新当前运行状态。
            if (event.value("replace", false)) messages[step] = event.value("text", "");
            else messages[step] += event.value("text", "");
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = Json::array(); const auto validSteps = task.value("validSteps", Json::array());
    // 【逐行批注】 按循环规则重复处理：for (const auto& [step, value] : messages) if (!value.empty())。
    for (const auto& [step, value] : messages) if (!value.empty())
        // 【逐行批注】 执行当前声明或表达式：result.push_back({{"role", "assistant"}, {"text", value}, {"step", step}, {"at", tas…。
        result.push_back({{"role", "assistant"}, {"text", value}, {"step", step}, {"at", task.at("updatedAt")},
            // 【逐行批注】 执行当前声明或表达式：{"displayComplete", fullyDisplayed[step]},。
            {"displayComplete", fullyDisplayed[step]},
            // 【逐行批注】 调用 find 完成本行所需操作：{"partial", std::find(validSteps.begin(), validSteps.end(), step) == …。
            {"partial", std::find(validSteps.begin(), validSteps.end(), step) == validSteps.end()}});
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 saveLesson，执行该函数声明的职责。
void saveLesson(Database& db, const Json& lesson) {
    // 【逐行批注】 判断条件 (!db.upsert(ClassroomActivity{lesson.at("id"), lesson.at("course…，成立时执行对应分支。
    if (!db.upsert(ClassroomActivity{lesson.at("id"), lesson.at("courseId"), "agent-lesson", lesson.dump(), now(), 0, 0}))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("AI 课时保存失败");。
        throw std::runtime_error("AI 课时保存失败");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 update，执行该函数声明的职责。
PreparedAgentTool update(Database& db, const Json& task, const std::string& id, const Json& after, Json result) {
    // 【逐行批注】 计算右侧结果并写入 auto old，更新当前运行状态。
    const auto old = lessonFor(db, accessFor(task), id).dump();
    // 【逐行批注】 结束当前函数并返回结果：return {std::move(result), [old, after, access = accessFor(task), id]…。
    return {std::move(result), [old, after, access = accessFor(task), id](Database& connection) {
        // 【逐行批注】 判断条件 (lessonFor(connection, access, id).dump() != old) throw std::inv…，成立时执行对应分支。
        if (lessonFor(connection, access, id).dump() != old) throw std::invalid_argument("课时内容已经变化，旧输出不应用");
        // 【逐行批注】 调用 saveLesson 完成本行所需操作：saveLesson(connection, after);。
        saveLesson(connection, after);
    // 【逐行批注】 执行当前声明或表达式：}};。
    }};
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 text，执行该函数声明的职责。
std::string text(const Json& value, const char* key) {
    // 【逐行批注】 判断条件 (!value.is_object()) throw std::invalid_argument(std::string("ar…，成立时执行对应分支。
    if (!value.is_object()) throw std::invalid_argument(std::string("args.") + key + " 的父节点必须为对象");
    // 【逐行批注】 判断条件 (!value.value(key, Json()).is_string() || value[key].get<std::st…，成立时执行对应分支。
    if (!value.value(key, Json()).is_string() || value[key].get<std::string>().empty())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument(std::string(key) + " 不能为空");。
        throw std::invalid_argument(std::string(key) + " 不能为空");
    // 【逐行批注】 结束当前函数并返回结果：return value[key];。
    return value[key];
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 link，执行该函数声明的职责。
Json link(const Json& lesson) {
    // 【逐行批注】 结束当前函数并返回结果：return {{"id", lesson.at("id")}, {"title", lesson.at("title")}, {"cou…。
    return {{"id", lesson.at("id")}, {"title", lesson.at("title")}, {"courseId", lesson.at("courseId")},
        // 【逐行批注】 计算右侧结果并写入 {"href", "/learn?lessonId，更新当前运行状态。
        {"href", "/learn?lessonId=" + lesson.at("id").get<std::string>()}, {"status", lesson.at("status")}};
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentSectionLocation，执行该函数声明的职责。
AgentJson agentSectionLocation(const Json& lesson, const Json& section, const Json& originTask) {
    // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
    const auto kind = section.value("questionKind", section.value("legacyKind", "practice"));
    // 【逐行批注】 计算右侧结果并写入 std::string view，更新当前运行状态。
    std::string view = section.at("kind") == "summary" ? "summary" : section.at("kind") == "question" &&
        // 【逐行批注】 执行当前声明或表达式：kind != "interaction" && kind != "diagnostic" && kind != "example" ? "practice" : "l…。
        kind != "interaction" && kind != "diagnostic" && kind != "example" ? "practice" : "learn";
    // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
    const auto event = originTask.value("event", Json::object());
    // 练习中的补讲和追问题留在当次练习，类型名称不能代替实际页面归属。
    // 【逐行批注】 判断条件 (event.value("lessonId", "") == lesson.at("id") && section.at("k…，成立时执行对应分支。
    if (event.value("lessonId", "") == lesson.at("id") && section.at("kind") != "summary" &&
        // 【逐行批注】 调用 lessonTaskView 完成本行所需操作：lessonTaskView(lesson, originTask) == "practice") view = "practice";。
        lessonTaskView(lesson, originTask) == "practice") view = "practice";
    // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
    const auto id = section.at("id").get<std::string>();
    // 【逐行批注】 结束当前函数并返回结果：return {{"sectionId", id}, {"view", view},。
    return {{"sectionId", id}, {"view", view},
        // 【逐行批注】 计算右侧结果并写入 {"href", "/" + view + "?lessonId，更新当前运行状态。
        {"href", "/" + view + "?lessonId=" + queryComponent(lesson.at("id").get<std::string>()) + "&sectionId=" + queryComponent(id)}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 registerAgentLessonTools，执行该函数声明的职责。
void registerAgentLessonTools(LearningAgent& agent) {
    agent.registerTool("read_component_activity", {u8"按需读取本课真实组件操作轨迹，不评分。参数lessonId,interactionId（currentInputs提供的真实输入ID）,componentId,offset:非负整数默认0,limit:1至50默认50。只返回已保存并校验的原始操作及程序重算结果；用于复核摘要、连续错误、异常或学生要求分析。不能把点击/重置当作掌握证据。", true,
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            const auto access = accessFor(task);
            const auto lessonId = text(args, "lessonId");
            const auto lesson = lessonFor(db, access, lessonId);
            if (task.at("event").value("lessonId", "") != lessonId) throw std::invalid_argument("轨迹读取必须绑定当前课时");
            const auto interactionId = text(args, "interactionId"), componentId = text(args, "componentId");
            const auto pageValue = [](const Json& value, int minimum, int maximum) {
                if (!value.is_number_integer()) throw std::invalid_argument("轨迹分页须为整数");
                if (value.is_number_unsigned()) {
                    const auto number = value.get<unsigned long long>();
                    if (number < static_cast<unsigned long long>(minimum) || number > static_cast<unsigned long long>(maximum))
                        throw std::invalid_argument("轨迹分页超出范围");
                    return static_cast<int>(number);
                }
                const auto number = value.get<long long>();
                if (number < minimum || number > maximum) throw std::invalid_argument("轨迹分页超出范围");
                return static_cast<int>(number);
            };
            const auto offset = pageValue(args.value("offset", Json(0)), 0, 200);
            const auto limit = pageValue(args.value("limit", Json(50)), 1, 50);
            for (const auto& record : db.listInteractions()) if (record.id == interactionId && record.kind == "chat-user") {
                const auto value = Json::parse(record.payload);
                if (value.value("scopeId", "") != access.scopeId || value.value("lessonId", "") != lessonId ||
                    !record.courseId || *record.courseId != lesson.at("courseId").get<std::string>() ||
                    std::find(access.courseIds.begin(), access.courseIds.end(), *record.courseId) == access.courseIds.end())
                    throw std::invalid_argument("无权读取其他课堂轨迹");
                const auto activities = value.value("componentActivities", Json::object());
                if (!activities.contains(componentId)) throw std::invalid_argument("本次输入没有该组件活动");
                auto result = activities.at(componentId); const auto events = result.at("events");
                result["events"] = Json::array();
                for (std::size_t index = static_cast<std::size_t>(offset); index < events.size() && index < static_cast<std::size_t>(offset + limit); ++index)
                    result["events"].push_back(events[index]);
                result["interactionId"] = interactionId; result["offset"] = offset; result["storedEvents"] = events.size();
                result["hasMore"] = static_cast<std::size_t>(offset + limit) < events.size();
                return PreparedAgentTool{result, {}};
            }
            throw std::invalid_argument("组件活动输入不存在");
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("create_lesson", {"为可访问课程创建独立课时。参数 courseId,title,purpose,intent:…。
    agent.registerTool("create_lesson", {"为可访问课程创建独立课时。参数 courseId,title,purpose,intent:advance|reinforce|review|general（默认reinforce）。推进大纲必须提供 read_outline 的 topicId；补弱及复习可提供同一课程的 topicId 以关联历史，不增加原大纲完成数。返回 lessonId。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
            const auto access = accessFor(task); const auto courseId = text(args, "courseId"); requireCourse(db, access, courseId);
            // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
            Json lesson = {{"id", newId("lesson")}, {"scopeId", access.scopeId}, {"courseId", courseId},
                // 【逐行批注】 调用 text 完成本行所需操作：{"title", text(args, "title")}, {"purpose", text(args, "purpose")}, {…。
                {"title", text(args, "title")}, {"purpose", text(args, "purpose")}, {"status", "draft"},
                // 【逐行批注】 调用 array、now 完成本行所需操作：{"sections", Json::array()}, {"version", 1}, {"model", source.model},…。
                {"sections", Json::array()}, {"version", 1}, {"model", source.model}, {"updatedAt", now()},
                // 【逐行批注】 执行当前声明或表达式：{"sourceLearningVersion", task.at("learningVersion")}, {"taskId", task.at("id")}, {"…。
                {"sourceLearningVersion", task.at("learningVersion")}, {"taskId", task.at("id")}, {"entered", false}};
            // 【逐行批注】 计算右侧结果并写入 auto intent，更新当前运行状态。
            const auto intent = args.value("intent", "reinforce");
            const auto& event = task.at("event");
            if (event.value("type", "") == "prepare_next" && deepCourseEnabled(db, courseId))
                lesson["roundProtocolVersion"] = 1;
            if (event.at("type") == "prepare_next" && !event.value("reviewId", "").empty()) {
                const auto review = agentScheduledReview(db, access, event.value("courseId", ""), event.at("reviewId"));
                if (!review.empty()) {
                    if (!review.at("available").get<bool>() || review.at("courseId") != courseId || intent != "review" ||
                        args.value("topicId", "") != review.at("topicId"))
                        throw std::invalid_argument("请按指定课程和知识点创建到期复习课时");
                    lesson["scheduledReviewId"] = review.at("id");
                    lesson["scheduledReviewBasisStatus"] = review.at("basisStatus");
                    lesson["scheduledReviewBasisRevision"] = review.value("basisRevision", "");
                }
            }
            // 【逐行批注】 判断条件 (intent != "advance" && intent != "reinforce" && intent != "revi…，成立时执行对应分支。
            if (intent != "advance" && intent != "reinforce" && intent != "review" && intent != "general") throw std::invalid_argument("课时意图无效");
            // 【逐行批注】 计算右侧结果并写入 lesson["intent"]，更新当前运行状态。
            lesson["intent"] = intent;
            // 【逐行批注】 判断条件 (intent == "advance") {，成立时执行对应分支。
            if (intent == "advance" || ((intent == "reinforce" || intent == "review") && args.contains("topicId"))) {
                // 【逐行批注】 计算右侧结果并写入 auto outline，更新当前运行状态。
                const auto outline = agentOutline(db, access, courseId); bool found = false;
                // 【逐行批注】 按循环规则重复处理：for (const auto& stage : outline.at("courseStructure")) for (const auto& t…。
                for (const auto& stage : outline.at("courseStructure")) for (const auto& topic : stage.at("topics")) if (topic.at("id") == text(args, "topicId")) {
                    // 【逐行批注】 计算右侧结果并写入 lesson["topicId"]，更新当前运行状态。
                    lesson["topicId"] = topic.at("id");
                    if (intent == "advance") {
                        lesson["phaseIndex"] = topic.at("legacyPhaseIndex");
                        lesson["topicIndex"] = topic.at("legacyTopicIndex");
                    }
                    // 【逐行批注】 计算右侧结果并写入 lesson["phaseName"]，更新当前运行状态。
                    lesson["phaseName"] = stage.at("stage"); lesson["topicTitle"] = topic.at("title"); found = true;
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 判断条件 (!found) throw std::invalid_argument("推进知识点不存在");，成立时执行对应分支。
                if (!found) throw std::invalid_argument("推进知识点不存在");
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"lessonId", lesson.at("id")}, {"status", "…。
            return PreparedAgentTool{{{"lessonId", lesson.at("id")}, {"status", "draft"}},
                // 【逐行批注】 调用 requireCourse、saveLesson 完成本行所需操作：[lesson, access](Database& connection) { requireCourse(connection, ac…。
                [lesson, access](Database& connection) { requireCourse(connection, access, lesson.at("courseId")); saveLesson(connection, lesson); }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("read_lesson", {"读取指定 lessonId 的原始教学板块、题目标准和展示状态供 AI 判断；可选 sectio…。
    agent.registerTool("read_lesson", {"读取指定 lessonId 的原始教学板块、题目标准和展示状态供 AI 判断；可选 sectionIds:[板块ID] 只读所需板块，省略读取整课。这些内部资料不直接发送到用户页面。", true,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult&) {。
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            // 【逐行批注】 计算右侧结果并写入 lesson，更新当前运行状态。
            auto lesson = lessonFor(db, accessFor(task), text(args, "lessonId"));
            // 【逐行批注】 计算右侧结果并写入 auto exposure，更新当前运行状态。
            const auto exposure = Json::parse(db.profileMeta("agent-section-exposure:" + lesson.at("id").get<std::string>()), nullptr, false);
            // 【逐行批注】 按循环规则重复处理：for (auto& section : lesson["sections"])。
            for (auto& section : lesson["sections"])
                // 【逐行批注】 计算右侧结果并写入 section["displayed"]，更新当前运行状态。
                section["displayed"] = exposure.is_object() && exposure.value(section.at("id").get<std::string>(), 0) == section.at("version");
            // 【逐行批注】 判断条件 (args.contains("sectionIds")) {，成立时执行对应分支。
            if (args.contains("sectionIds")) {
                // 【逐行批注】 判断条件 (!args.at("sectionIds").is_array() || args.at("sectionIds").empt…，成立时执行对应分支。
                if (!args.at("sectionIds").is_array() || args.at("sectionIds").empty()) throw std::invalid_argument("sectionIds 必须为非空板块ID列表");
                // 【逐行批注】 计算右侧结果并写入 Json selected，更新当前运行状态。
                Json selected = Json::array();
                // 【逐行批注】 按循环规则重复处理：for (const auto& id : args.at("sectionIds")) {。
                for (const auto& id : args.at("sectionIds")) {
                    // 【逐行批注】 判断条件 (!id.is_string()) throw std::invalid_argument("sectionIds 必须使用文字…，成立时执行对应分支。
                    if (!id.is_string()) throw std::invalid_argument("sectionIds 必须使用文字ID");
                    // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
                    const auto found = std::find_if(lesson["sections"].begin(), lesson["sections"].end(), [&](const Json& section) { return section.at("id") == id; });
                    // 【逐行批注】 判断条件 (found == lesson["sections"].end()) throw std::invalid_argument(…，成立时执行对应分支。
                    if (found == lesson["sections"].end()) throw std::invalid_argument("指定板块不属于当前课时");
                    // 【逐行批注】 执行当前声明或表达式：selected.push_back(*found);。
                    selected.push_back(*found);
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 lesson["sections"]，更新当前运行状态。
                lesson["sections"] = std::move(selected); lesson["partialSections"] = true;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{lesson, {}};。
            // 课时原文仍供内部教学使用；状态与可引用评价另给公开投影，便于真实引用。
            const auto view = agentLessonView(db, accessFor(task), lesson.at("id"));
            lesson["learningState"] = view.at("learningState");
            lesson["publishedEvaluations"] = view.at("evaluations");
            lesson["topicEvidence"] = view.at("topicEvidence");
            lesson["teachingPreferences"] = view.at("teachingPreferences");
            return PreparedAgentTool{lesson, {}};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("select_teaching_focus", {"选择本课当前教学焦点。参数 lessonId，以及可选 sectionId（…。
    agent.registerTool("select_teaching_focus", {"选择本课当前教学焦点。参数 lessonId，以及可选 sectionId（讲解段）、interactionSectionId（questionKind=interaction 的讲解互动题）、practiceSectionId（集中练习题）。省略保留当前值，空字符串清除相应焦点。先 read_lesson；只能选择已经保存的同一课时板块。依据学生真实反馈决定继续、补讲或再问，不能把切换页面当成掌握。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult&) {。
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
            const auto id = text(args, "lessonId"); auto lesson = lessonFor(db, accessFor(task), id);
            // 【逐行批注】 调用 requireTeachingTarget 完成本行所需操作：requireTeachingTarget(task, lesson);。
            requireTeachingTarget(task, lesson);
            // 【逐行批注】 判断条件 (lesson.at("status") != "ready" && lesson.at("status") != "draft…，成立时执行对应分支。
            if (lesson.at("status") != "ready" && lesson.at("status") != "draft") throw std::invalid_argument("课时状态不允许选择教学焦点");
            // 【逐行批注】 判断条件 (!lesson.value("teachingTaskId", "").empty() && lesson.at("teach…，成立时执行对应分支。
            if (!lesson.value("teachingTaskId", "").empty() && lesson.at("teachingTaskId") != task.at("id"))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("本课已有更新的教学任务，旧输出不应用");。
                throw std::invalid_argument("本课已有更新的教学任务，旧输出不应用");
            // 【逐行批注】 计算右侧结果并写入 focus，更新当前运行状态。
            auto focus = lesson.value("teachingFocus", Json::object()); bool selected = false;
            // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"sectionId", "interactionSectionId", "practiceSe…。
            for (const auto* field : {"sectionId", "interactionSectionId", "practiceSectionId"}) {
                // 【逐行批注】 判断条件 (!args.contains(field)) continue;，成立时执行对应分支。
                if (!args.contains(field)) continue;
                // 【逐行批注】 判断条件 (!args.at(field).is_string()) throw std::invalid_argument("教学焦点须…，成立时执行对应分支。
                if (!args.at(field).is_string()) throw std::invalid_argument("教学焦点须使用稳定板块标识");
                // 【逐行批注】 计算右侧结果并写入 selected，更新当前运行状态。
                selected = true; const auto sectionId = args.at(field).get<std::string>();
                // 【逐行批注】 判断条件 (!sectionId.empty()) {，成立时执行对应分支。
                if (!sectionId.empty()) {
                    // 【逐行批注】 计算右侧结果并写入 Json* section，更新当前运行状态。
                    const Json* section = nullptr;
                    // 【逐行批注】 按循环规则重复处理：for (const auto& item : lesson.at("sections")) if (item.at("id") == sectio…。
                    for (const auto& item : lesson.at("sections")) if (item.at("id") == sectionId) { section = &item; break; }
                    // 【逐行批注】 判断条件 (!section) throw std::invalid_argument("教学焦点不属于该课时");，成立时执行对应分支。
                    if (!section) throw std::invalid_argument("教学焦点不属于该课时");
                    // 【逐行批注】 计算右侧结果并写入 bool explanation，更新当前运行状态。
                    const bool explanation = std::string(field) == "sectionId";
                    // 【逐行批注】 判断条件 (section->at("kind") != (explanation ? "explanation" : "question…，成立时执行对应分支。
                    if (section->at("kind") != (explanation ? "explanation" : "question")) throw std::invalid_argument("教学焦点板块类型不匹配");
                    // 【逐行批注】 判断条件 (!explanation) {，成立时执行对应分支。
                    if (!explanation) {
                        // 【逐行批注】 计算右侧结果并写入 auto legacyKind，更新当前运行状态。
                        const auto legacyKind = section->value("legacyKind", "");
                        // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
                        const auto kind = section->value("questionKind", legacyKind == "interaction" || legacyKind == "diagnostic" ? "interaction" : "practice");
                        // 【逐行批注】 判断条件 ((std::string(field) == "interactionSectionId") != (kind == "int…，成立时执行对应分支。
                        if ((std::string(field) == "interactionSectionId") != (kind == "interaction" || kind == "diagnostic"))
                            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("讲解互动题与集中练习题不能混用");。
                            throw std::invalid_argument("讲解互动题与集中练习题不能混用");
                        // 【逐行批注】 判断条件 (legacyKind == "example") throw std::invalid_argument("示范例题不作为学生…，成立时执行对应分支。
                        if (legacyKind == "example") throw std::invalid_argument("示范例题不作为学生评分题");
                        requireRoundQuestion(lesson, *section);
                    // 【逐行批注】 结束当前代码块。
                    }
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 focus[field]，更新当前运行状态。
                focus[field] = sectionId;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (!selected) throw std::invalid_argument("请至少选择一个教学焦点");，成立时执行对应分支。
            if (!selected) throw std::invalid_argument("请至少选择一个教学焦点");
            // 【逐行批注】 计算右侧结果并写入 auto preferred，更新当前运行状态。
            const auto preferred = responseView(lesson, task) == "practice" ? "practiceSectionId" : "interactionSectionId";
            // 【逐行批注】 计算右侧结果并写入 auto fallback，更新当前运行状态。
            const auto fallback = std::string(preferred) == "practiceSectionId" ? "interactionSectionId" : "practiceSectionId";
            // 【逐行批注】 计算右侧结果并写入 auto selectedQuestion，更新当前运行状态。
            const auto selectedQuestion = args.value(preferred, args.value(fallback, ""));
            // 【逐行批注】 按循环规则重复处理：for (const auto& section : lesson.at("sections")) if (section.at("id") == …。
            for (const auto& section : lesson.at("sections")) if (section.at("id") == selectedQuestion)
                // 【逐行批注】 调用 selectResponseQuestion 完成本行所需操作：selectResponseQuestion(focus, lesson, task, section);。
                selectResponseQuestion(focus, lesson, task, section);
            // 【逐行批注】 计算右侧结果并写入 focus["taskId"]，更新当前运行状态。
            focus["taskId"] = task.at("id"); focus["version"] = focus.value("version", 0) + 1; focus["updatedAt"] = now();
            // 【逐行批注】 计算右侧结果并写入 lesson["teachingFocus"]，更新当前运行状态。
            lesson["teachingFocus"] = focus; lesson["version"] = lesson.at("version").get<int>() + 1; lesson["updatedAt"] = now();
            // 【逐行批注】 结束当前函数并返回结果：return update(db, task, id, lesson, {{"lessonId", id}, {"teachingFocu…。
            return update(db, task, id, lesson, {{"lessonId", id}, {"teachingFocus", focus}});
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    agent.registerTool("start_teaching_round", {u8"开始一轮课堂并冻结题数。参数 lessonId、questionCount:1至20的整数、goal。已有活动轮次不能改题数。先向学生说明本轮目标与题数，再逐题出题。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto id = text(args, "lessonId"); auto lesson = lessonFor(db, accessFor(task), id);
            requireRoundTask(task, lesson);
            const auto count = checkedRoundQuestionCount(args.value("questionCount", Json()));
            if (activeTeachingRound(lesson)) throw std::invalid_argument("已有进行中的教学轮次，不能静默改题数");
            const auto goal = text(args, "goal");
            auto round = newTeachingRound(goal, count);
            round["model"] = source.model;
            saveCurrentRound(lesson, round);
            return update(db, task, id, lesson, {{"lessonId", id}, {"roundId", round.at("roundId")}, {"questionCount", count}, {"goal", goal}, {"status", "active"}});
        }});
    agent.registerTool("complete_teaching_round", {u8"结束当前教学轮次。参数lessonId、roundId。只有本轮已声明题数的每道题都已完成真实评价时才能结束；补强题安排到下一轮。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            const auto id = text(args, "lessonId"); auto lesson = lessonFor(db, accessFor(task), id);
            requireRoundTask(task, lesson);
            auto round = lesson.value("teachingRound", Json::object());
            if (round.value("status", "") != "active" || round.value("roundId", "") != text(args, "roundId")) throw std::invalid_argument("教学轮次不存在或已结束");
            const auto questions = round.value("questionIds", Json::array()); const auto evaluated = round.value("evaluatedQuestionIds", Json::array());
            if (!questions.is_array() || !evaluated.is_array() || questions.size() != static_cast<std::size_t>(round.at("questionCount").get<int>())) throw std::invalid_argument("本轮题数尚未完成，不能结束");
            for (const auto& question : questions) if (std::find(evaluated.begin(), evaluated.end(), question) == evaluated.end())
                throw std::invalid_argument("每道题必须先发布即时反馈，不能跳过反馈结束本轮。如果本题评价已暂存，请先将针对学生作答的反馈写入 message，actions 留空并令 state 为 continue；反馈发布后才能结束轮次。");
            round["status"] = "completed"; round["completedAt"] = now(); lesson["teachingRound"] = round; lesson["version"] = lesson.at("version").get<int>() + 1; lesson["updatedAt"] = now();
            return update(db, task, id, lesson, {{"lessonId", id}, {"roundId", round.at("roundId")}, {"status", "completed"}, {"questionCount", round.at("questionCount")}, {"evaluatedQuestionCount", evaluated.size()}, {"nextRoundRequired", true}});
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("append_section", {"追加教学板块，不改已展示内容。参数 lessonId、section:{kind:expl…。
    agent.registerTool("append_section", {"追加教学板块，不改已展示内容。参数 lessonId、section:{kind:explanation|question|summary,title,body 或 question:{question,options 可选,type,expectedAnswer,rubric},questionKind:interaction|practice}，activate 可选。带body的explanation和summary必须另填section.helpType:none|explanation|hint|worked_example|clarification|unknown；普通概念讲解用explanation。hint和worked_example还必须填section.targetQuestionId，使用currentInputs实际选中原题的questionId，不用新的教学焦点。课堂互动追加的问题默认立即在学生当前页面显示；仅备好未来题目时用 activate=false。选择题必须有至少两个非空 options，不可只在 message 里承诺出题。有限集合交并补和关系单选题优先使用 section.question:{setExercise:{schemaVersion:1,config:{universe,setA,setB,operation},options}}，operation 为 intersection|union|complement_a|complement_b|relations。可用同课已保存的集合组件 mediaId 替代 config，按保存的初始条件出题。运算题 options 是 2 至 6 个由全集内元素组成的数组，关系题省略 options。程序生成题面并核验唯一答案，无须另写 question、expectedAnswer 或 rubric。题目标准只放 expectedAnswer/rubric，示范放 explanation，不评分。完整示范另一题时，section.demonstratedQuestions 保存实际示范的题干及 options/materials，不能只填答案。返回稳定板块与题目标识。嵌套动作格式示例：{\"id\":\"new-question\",\"tool\":\"append_section\",\"args\":{\"lessonId\":\"实际课时ID\",\"section\":{\"kind\":\"question\",\"title\":\"练习\",\"questionKind\":\"practice\",\"materialOrigin\":\"ai_exercise\",\"question\":{\"question\":\"题干\",\"type\":\"open\",\"expectedAnswer\":\"私有答案\",\"rubric\":\"私有标准\"}}}}。引用依据写在section.citations中；不要把私有标准放到公开body。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
            const auto id = text(args, "lessonId"); auto lesson = lessonFor(db, accessFor(task), id);
            // 【逐行批注】 调用 requireTeachingTarget 完成本行所需操作：requireTeachingTarget(task, lesson);。
            requireTeachingTarget(task, lesson);
            // 【逐行批注】 判断条件 (lesson.at("status") != "draft" && lesson.at("status") != "ready…，成立时执行对应分支。
            if (lesson.at("status") != "draft" && lesson.at("status") != "ready") throw std::invalid_argument("课时状态不允许补充");
            // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
            const auto input = args.at("section"); const auto kind = text(input, "kind");
            // 【逐行批注】 判断条件 (kind != "explanation" && kind != "question" && kind != "summary…，成立时执行对应分支。
            if (kind != "explanation" && kind != "question" && kind != "summary") throw std::invalid_argument("教学板块类型无效");
            // 【逐行批注】 计算右侧结果并写入 Json section，更新当前运行状态。
            Json section = {{"id", newId("section")}, {"kind", kind}, {"title", text(input, "title")},
                // 【逐行批注】 调用 now 完成本行所需操作：{"version", 1}, {"displayed", false}, {"model", source.model}, {"crea…。
                {"version", 1}, {"displayed", false}, {"model", source.model}, {"createdAt", now()}};
            // 【逐行批注】 计算右侧结果并写入 auto origin，更新当前运行状态。
            const auto origin = input.value("materialOrigin", kind == "question" ? "ai_exercise" : "ai_supplement");
            // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(origin!="source_based"&&origin!="ai_supplement"&&origin!="ai_exerc…。
            if(origin!="source_based"&&origin!="ai_supplement"&&origin!="ai_exercise")throw std::invalid_argument("教学内容来源类型无效");
            // 【逐行批注】 计算右侧结果并写入 auto citations，更新当前运行状态。
            const auto citations=libraryValidateCitations(db,input.value("citations",Json::array()),true,lesson.at("courseId"));
            // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(origin=="source_based"&&citations.empty())throw std::invalid_argum…。
            if(origin=="source_based"&&citations.empty())throw std::invalid_argument("依据资料的讲解需要实际原文引用");
            // 【逐行批注】 计算右侧结果并写入 section["materialOrigin"]，更新当前运行状态。
            section["materialOrigin"]=origin;section["citations"]=citations;
            // 【逐行批注】 按循环规则重复处理：for(const auto& citation:citations) {。
            for(const auto& citation:citations) {
                // 【逐行批注】 调用 array 完成本行所需操作：if(!lesson.contains("references"))lesson["references"]=Json::array();。
                if(!lesson.contains("references"))lesson["references"]=Json::array();
                // 【逐行批注】 执行当前声明或表达式：lesson["references"].push_back(citation);。
                lesson["references"].push_back(citation);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (kind == "question") {，成立时执行对应分支。
            if (kind == "question") {
                // 【逐行批注】 计算右侧结果并写入 auto question，更新当前运行状态。
                auto question = input.at("question");
                // 【逐行批注】 判断条件 (!question.is_object()) throw std::invalid_argument("section.que…，成立时执行对应分支。
                if (!question.is_object()) throw std::invalid_argument("section.question 必须为 JSON 对象，不能使用数组；一个题目对应一个 question 板块");
                // 【逐行批注】 调用 text 完成本行所需操作：text(question, "question");。
                if (question.contains("setExercise")) {
                    const auto content = lesson.find("deepContent");
                    question = checkedSetExerciseQuestion(question, content == lesson.end() ? Json::object() : *content);
                }
                text(question, "question");
                // 【逐行批注】 判断条件 (input.contains("questionKind") && args.contains("questionKind")…，成立时执行对应分支。
                if (input.contains("questionKind") && args.contains("questionKind") && input.at("questionKind") != args.at("questionKind"))
                    // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("section.questionKind 与外层 questionKind 不一…。
                    throw std::invalid_argument("section.questionKind 与外层 questionKind 不一致，请明确题目用途");
                // 兼容模型将同名用途参数放在外层，避免默认为另一类题而导致后续焦点失败。
                // 【逐行批注】 计算右侧结果并写入 section["questionKind"]，更新当前运行状态。
                section["questionKind"] = input.value("questionKind", args.value("questionKind", question.value("kind", "practice")));
                // 【逐行批注】 判断条件 (section.at("questionKind") != "interaction" && section.at("ques…，成立时执行对应分支。
                if (section.at("questionKind") != "interaction" && section.at("questionKind") != "practice" &&
                    // 【逐行批注】 执行当前声明或表达式：section.at("questionKind") != "diagnostic" && section.at("questionKind") != "quiz" &…。
                    section.at("questionKind") != "diagnostic" && section.at("questionKind") != "quiz" && section.at("questionKind") != "review")
                    // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("题目用途无效；示范例题请使用 explanation");。
                    throw std::invalid_argument("题目用途无效；示范例题请使用 explanation");
                // 【逐行批注】 判断条件 (question.contains("options")) {，成立时执行对应分支。
                if (question.contains("options")) {
                    // 【逐行批注】 判断条件 (!question["options"].is_array()) throw std::invalid_argument("题…，成立时执行对应分支。
                    if (!question["options"].is_array()) throw std::invalid_argument("题目选项无效");
                    // 【逐行批注】 按循环规则重复处理：for (const auto& option : question["options"]) if (!option.is_string() || …。
                    for (const auto& option : question["options"]) if (!option.is_string() || option.get<std::string>().empty())
                        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("题目选项须为非空文字");。
                        throw std::invalid_argument("题目选项须为非空文字");
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 auto questionType，更新当前运行状态。
                const auto questionType = question.value("type", question.contains("options") ? "choice" : "open");
                // 【逐行批注】 判断条件 ((questionType == "choice" || questionType == "single_choice" ||…，成立时执行对应分支。
                if ((questionType == "choice" || questionType == "single_choice" || questionType == "multi_choice") &&
                    // 【逐行批注】 执行当前声明或表达式：(!question.contains("options") || question.at("options").size() < 2))。
                    (!question.contains("options") || question.at("options").size() < 2))
                    // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("section.question.options：选择题必须保存至少两个选项");。
                    throw std::invalid_argument("section.question.options：选择题必须保存至少两个选项");
                // 【逐行批注】 计算右侧结果并写入 section["question"]，更新当前运行状态。
                section["question"] = question;
                // 【逐行批注】 计算右侧结果并写入 section["question"]["type"]，更新当前运行状态。
                section["question"]["type"] = questionType;
                // 【逐行批注】 计算右侧结果并写入 section["questionId"]，更新当前运行状态。
                section["questionId"] = teachingQuestionIdentity(lesson.at("courseId"), question);
                if (lesson.value("roundProtocolVersion", 0) >= 1 && !lesson.contains("teachingRound"))
                    throw std::invalid_argument("新课堂出题前请先调用 start_teaching_round 声明并冻结本轮目标和题数，再用返回的 roundId 生成第一题");
                if (lesson.contains("teachingRound")) {
                    requireRoundTask(task, lesson);
                    if (!activeTeachingRound(lesson)) throw std::invalid_argument("本轮已结束，补强题需要开始下一轮");
                    auto& round = lesson.at("teachingRound");
                    // 任务已绑定当前课时且只有一个活动轮次时，可从服务端状态补齐遗漏标识。
                    if (args.contains("roundId") && (!args.at("roundId").is_string() || args.at("roundId") != round.at("roundId")))
                        throw std::invalid_argument("追加题目的轮次标识与当前教学轮次不一致");
                    if (!args.value("activate", true)) throw std::invalid_argument("本轮只生成并展示当前一道题");
                    if (round.contains("pendingSectionId")) throw std::invalid_argument("当前题必须先完成并发布反馈，才能安排下一题");
                    // 作答仍绑定保存时的题目标识与选项顺序，独立题数使用统一的证据去重规则。
                    const auto& courseId = lesson.at("courseId").get_ref<const std::string&>();
                    const auto key = teachingQuestionEvidenceKey(courseId, question);
                    const auto& sections = lesson.at("sections");
                    const bool repeated = std::any_of(sections.begin(), sections.end(), [&](const Json& previous) {
                        if (previous.value("roundId", Json()) != round.at("roundId")) return false;
                        const auto snapshot = previous.find("question");
                        return snapshot != previous.end() && snapshot->is_object() &&
                            snapshot->value("question", Json()).is_string() &&
                            teachingQuestionEvidenceKey(courseId, *snapshot) == key;
                    });
                    if (roundContainsQuestion(round, section["questionId"].get<std::string>()) || repeated)
                        throw std::invalid_argument("本轮不能重复追加同一道题。请根据上一题反馈设计有实质差异的下一题。");
                    const auto current = round.at("questionIds").size();
                    if (current >= static_cast<std::size_t>(round.at("questionCount").get<int>())) throw std::invalid_argument("本轮题数已满，请结束本轮并在下一轮补强");
                    section["roundId"] = round.at("roundId");
                    round["questionIds"].push_back(section["questionId"]);
                    round["pendingSectionId"] = section.at("id");
                }
                // 【逐行批注】 判断条件 (task.at("event").value("type", "") == "teaching_request" &&，成立时执行对应分支。
                if (task.at("event").value("type", "") == "teaching_request" &&
                    // 【逐行批注】 执行当前声明或表达式：task.at("event").value("requestType", "") == "retry_question") {。
                    task.at("event").value("requestType", "") == "retry_question") {
                    // 【逐行批注】 按循环规则重复处理：for (const auto& original : lesson.at("sections"))。
                    for (const auto& original : lesson.at("sections"))
                        // 【逐行批注】 判断条件 (original.at("id") == task.at("event").value("sectionId", "") &&，成立时执行对应分支。
                        if (original.at("id") == task.at("event").value("sectionId", "") &&
                            // 【逐行批注】 执行当前声明或表达式：input.value("rePracticeOf", "") != original.value("questionId", ""))。
                            input.value("rePracticeOf", "") != original.value("questionId", ""))
                            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("section.rePracticeOf 必须关联本次再试一道请求实际选中的原题…。
                            throw std::invalid_argument("section.rePracticeOf 必须关联本次再试一道请求实际选中的原题");
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 判断条件 (input.contains("rePracticeOf")) {，成立时执行对应分支。
                if (input.contains("rePracticeOf")) {
                    // 【逐行批注】 计算右侧结果并写入 auto original，更新当前运行状态。
                    const auto original = text(input, "rePracticeOf"); bool found = false;
                    // 【逐行批注】 按循环规则重复处理：for (const auto& previous : lesson.at("sections")) if (previous.value("que…。
                    for (const auto& previous : lesson.at("sections")) if (previous.value("questionId", "") == original) {
                        // 【逐行批注】 计算右侧结果并写入 found，更新当前运行状态。
                        found = true;
                        if (previous.contains("question") &&
                            teachingQuestionEvidenceKey(lesson.at("courseId"), previous.at("question")) ==
                                teachingQuestionEvidenceKey(lesson.at("courseId"), question))
                            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("复练题干、选项和材料必须与原题不同，不能用新的板块标识重复原题");。
                            throw std::invalid_argument("复练题干、选项和材料必须与原题不同，不能用新的板块标识重复原题");
                    // 【逐行批注】 结束当前代码块。
                    }
                    // 【逐行批注】 判断条件 (!found || original == section.at("questionId")) throw std::inva…，成立时执行对应分支。
                    if (!found || original == section.at("questionId")) throw std::invalid_argument("复练必须关联本课已保存的原问题，并生成不同的新题");
                    // 【逐行批注】 计算右侧结果并写入 section["rePracticeOf"]，更新当前运行状态。
                    section["rePracticeOf"] = original;
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 执行当前声明或表达式：} else {。
            } else {
                // 【逐行批注】 计算右侧结果并写入 section["body"]，更新当前运行状态。
                section["body"] = text(input, "body");
                // 【逐行批注】 判断条件 (task.value("teachingProtocolVersion", 1) >= 2 && !input.contain…，成立时执行对应分支。
                if (task.value("teachingProtocolVersion", 1) >= 2 && !input.contains("helpType"))
                    // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("section.helpType 必须说明实际帮助程度：补概念用 explana…。
                    throw std::invalid_argument("section.helpType 必须说明实际帮助程度：补概念用 explanation，本题提示用 hint，给出本题完整步骤或答案用 worked_example；后两者还须 targetQuestionId");
                // 【逐行批注】 计算右侧结果并写入 auto help，更新当前运行状态。
                const auto help = input.value("helpType", "explanation");
                // 【逐行批注】 判断条件 (!teachingHelpType(help)) throw std::invalid_argument("section.h…，成立时执行对应分支。
                if (!teachingHelpType(help)) throw std::invalid_argument("section.helpType 无效");
                // 【逐行批注】 计算右侧结果并写入 section["helpType"]，更新当前运行状态。
                section["helpType"] = help;
                // 【逐行批注】 计算右侧结果并写入 auto demonstrations，更新当前运行状态。
                const auto demonstrations = checkedDemonstrations(input);
                // 【逐行批注】 判断条件 (!demonstrations.empty()) {，成立时执行对应分支。
                if (!demonstrations.empty()) {
                    // 【逐行批注】 判断条件 (task.at("event").value("requestType", "") == "hint")，成立时执行对应分支。
                    if (task.at("event").value("requestType", "") == "hint")
                        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("学生只求小提示，本轮不能包含完整示范题");。
                        throw std::invalid_argument("学生只求小提示，本轮不能包含完整示范题");
                    // 【逐行批注】 计算右侧结果并写入 section["demonstratedQuestions"]，更新当前运行状态。
                    section["demonstratedQuestions"] = demonstrations;
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 判断条件 (help == "hint" || help == "worked_example") {，成立时执行对应分支。
                if (help == "hint" || help == "worked_example") {
                    // 【逐行批注】 计算右侧结果并写入 auto target，更新当前运行状态。
                    const auto target = text(input, "targetQuestionId"); bool found = false;
                    // 【逐行批注】 按循环规则重复处理：for (const auto& previous : lesson.at("sections")) if (previous.value("que…。
                    for (const auto& previous : lesson.at("sections")) if (previous.value("questionId", "") == target) found = true;
                    // 【逐行批注】 判断条件 (!found) throw std::invalid_argument("本题帮助必须关联本课真实题目标识");，成立时执行对应分支。
                    if (!found) throw std::invalid_argument("本题帮助必须关联本课真实题目标识");
                    // 【逐行批注】 计算右侧结果并写入 auto& event，更新当前运行状态。
                    const auto& event = task.at("event");
                    // 【逐行批注】 计算右侧结果并写入 auto request，更新当前运行状态。
                    const auto request = event.value("requestType", "");
                    // 【逐行批注】 判断条件 (event.value("type", "") == "teaching_request" &&，成立时执行对应分支。
                    if (event.value("type", "") == "teaching_request" &&
                        // 【逐行批注】 执行当前声明或表达式：(request == "hint" || request == "worked_example" || request == "rephrase")) {。
                        (request == "hint" || request == "worked_example" || request == "rephrase")) {
                        // 【逐行批注】 按循环规则重复处理：for (const auto& original : lesson.at("sections"))。
                        for (const auto& original : lesson.at("sections"))
                            // 【逐行批注】 判断条件 (original.at("id") == event.value("sectionId", "") && original.v…，成立时执行对应分支。
                            if (original.at("id") == event.value("sectionId", "") && original.value("questionId", "") != target)
                                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("section.targetQuestionId 必须使用本次教学请求实际选中的…。
                                throw std::invalid_argument("section.targetQuestionId 必须使用本次教学请求实际选中的原题，不能沿用其他教学焦点的题目");
                    // 【逐行批注】 结束当前代码块。
                    }
                    // 【逐行批注】 判断条件 (task.at("event").value("requestType", "") == "hint" && help == …，成立时执行对应分支。
                    if (task.at("event").value("requestType", "") == "hint" && help == "worked_example")
                        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("学生只求一点提示，本轮不能提供完整示范");。
                        throw std::invalid_argument("学生只求一点提示，本轮不能提供完整示范");
                    // 【逐行批注】 计算右侧结果并写入 section["targetQuestionId"]，更新当前运行状态。
                    section["targetQuestionId"] = target;
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 判断条件 (input.contains("presentation")) {，成立时执行对应分支。
                if (input.contains("presentation")) {
                    // 【逐行批注】 判断条件 (input.at("presentation") != "example") throw std::invalid_argum…，成立时执行对应分支。
                    if (input.at("presentation") != "example") throw std::invalid_argument("示范展示类型无效");
                    // 【逐行批注】 计算右侧结果并写入 section["presentation"]，更新当前运行状态。
                    section["presentation"] = "example";
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 lesson["sections"].push_back(section); lesso…，更新当前运行状态。
            lesson["sections"].push_back(section); lesson["version"] = lesson.at("version").get<int>() + 1;
            // 【逐行批注】 判断条件 (kind == "question" && args.value("activate", true)) {，成立时执行对应分支。
            if (kind == "question" && args.value("activate", true)) {
                // 【逐行批注】 计算右侧结果并写入 focus，更新当前运行状态。
                auto focus = lesson.value("teachingFocus", Json::object());
                // 【逐行批注】 调用 selectResponseQuestion 完成本行所需操作：selectResponseQuestion(focus, lesson, task, section);。
                selectResponseQuestion(focus, lesson, task, section);
                // 【逐行批注】 判断条件 (focus.value("responseTaskId", "") == task.at("id")) {，成立时执行对应分支。
                if (focus.value("responseTaskId", "") == task.at("id")) {
                    // 【逐行批注】 执行当前声明或表达式：focus[section.at("questionKind") == "interaction" || section.at("questionKind") == "…。
                    focus[section.at("questionKind") == "interaction" || section.at("questionKind") == "diagnostic"
                        // 【逐行批注】 计算右侧结果并写入 ? "interactionSectionId" : "practiceSectionI…，更新当前运行状态。
                        ? "interactionSectionId" : "practiceSectionId"] = section.at("id");
                    // 【逐行批注】 计算右侧结果并写入 focus["taskId"]，更新当前运行状态。
                    focus["taskId"] = task.at("id"); focus["version"] = focus.value("version", 0) + 1;
                    // 【逐行批注】 计算右侧结果并写入 focus["updatedAt"]，更新当前运行状态。
                    focus["updatedAt"] = now(); lesson["teachingFocus"] = focus;
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 lesson["model"]，更新当前运行状态。
            lesson["model"] = source.model; lesson["updatedAt"] = now();
            // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
            auto result = agentSectionLocation(lesson, section, task);
            // 【逐行批注】 计算右侧结果并写入 result["questionId"]，更新当前运行状态。
            result["questionId"] = section.value("questionId", ""); result["questionKind"] = section.value("questionKind", "");
            if (section.contains("roundId")) result["roundId"] = section.at("roundId");
            // 【逐行批注】 计算右侧结果并写入 result["version"]，更新当前运行状态。
            result["version"] = lesson["version"];
            // 【逐行批注】 结束当前函数并返回结果：return update(db, task, id, lesson, std::move(result));。
            return update(db, task, id, lesson, std::move(result));
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("finish_lesson", {"AI 判断课时已准备完整后提交。参数 lessonId。只保存模型实际生成的完整板块，不强制…。
    agent.registerTool("finish_lesson", {"AI 判断课时已准备完整后提交。参数 lessonId。只保存模型实际生成的完整板块，不强制六板块、题数或先后顺序。完成后返回进入课堂链接。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
            const auto id = text(args, "lessonId"); auto lesson = lessonFor(db, accessFor(task), id);
            // 【逐行批注】 判断条件 (task.at("event").value("type", "") == "prepare_next" && task.at…，成立时执行对应分支。
            if (task.at("event").value("type", "") == "prepare_next" && task.at("event").value("resumeDraft", false)) requireTeachingTarget(task, lesson);
            // 【逐行批注】 判断条件 (lesson.at("sections").empty()) throw std::invalid_argument("空课时…，成立时执行对应分支。
            if (lesson.at("sections").empty()) throw std::invalid_argument("空课时不能进入课堂");
            // 【逐行批注】 判断条件 (deepCourseEnabled(db, lesson.at("courseId")) &&，成立时执行对应分支。
            if (deepCourseEnabled(db, lesson.at("courseId")) &&
                // 【逐行批注】 调用 object、array 完成本行所需操作：lesson.value("deepContent", Json::object()).value("lecture", Json::ar…。
                lesson.value("deepContent", Json::object()).value("lecture", Json::array()).empty())
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("深度课时尚未保存讲义，请先调用set_deep_content；失败媒体可保留单…。
                throw std::invalid_argument("深度课时尚未保存讲义，请先调用set_deep_content；失败媒体可保留单独重试状态");
            // 【逐行批注】 计算右侧结果并写入 lesson["status"]，更新当前运行状态。
            lesson["status"] = "ready"; lesson["model"] = source.model; lesson["updatedAt"] = now();
            // 【逐行批注】 计算右侧结果并写入 lesson["sourceLearningVersion"]，更新当前运行状态。
            lesson["sourceLearningVersion"] = task.at("learningVersion"); lesson["version"] = lesson.at("version").get<int>() + 1;
            // 【逐行批注】 结束当前函数并返回结果：return update(db, task, id, lesson, {{"lesson", link(lesson)}, {"vers…。
            return update(db, task, id, lesson, {{"lesson", link(lesson)}, {"version", lesson.at("version")}});
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("evaluate_answer", {"真实 AI 评价实际学生回答。args 参数 interactionId、score(0…。
    agent.registerTool("evaluate_answer", {"真实 AI 评价实际学生回答。程序核验集合单选题使用 currentInputs.selectedOption（从0开始）对照 questionSnapshot.answerIndex，选对100，选错0，理由另外诊断。无选项的文字先澄清，不猜选项。args 参数 interactionId、score(0..100 或 null)、credible(bool)、feedback、reason、uncertainty；后三项必须是非空文字。新版 args 内必须有 diagnosis:{summary,evidence:[回答原话],teachingDirection,needsClarification:bool}，不能放在根对象；另须 feedbackHelpType:none|explanation|hint|worked_example|clarification，按公开反馈整体分类（feedback、uncertainty、diagnosis.summary/evidence）：仅指出对错用none，本题方向用hint，给本题答案或完整步骤用worked_example。若公开反馈完整示范另一题，另填 demonstratedQuestions:[{question,options/materials可选}]。只能引用真实学生记录；同题多轮保留稳定 questionId，提示经历不丢弃。没有可靠回答或评价不完整时不要给正式分数。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
            const auto access = accessFor(task); const auto interactionId = text(args, "interactionId");
            const auto completedActions = task.value("completedActions", Json::object());
            for (const auto& action : completedActions.items()) {
                const auto result = action.value().value("result", Json::object());
                const auto evaluation = result.value("evaluation", Json::object());
                if (evaluation.is_object() && evaluation.value("interactionId", "") == interactionId)
                    throw std::invalid_argument("本次回答已有评价，请复用已保存结果并完成反馈，不要重复评价或覆盖分数");
            }
            // 【逐行批注】 执行当前声明或表达式：Json answer; std::string courseId;。
            Json answer; std::string courseId;
            // 【逐行批注】 按循环规则重复处理：for (const auto& interaction : db.listInteractions()) if (interaction.id =…。
            for (const auto& interaction : db.listInteractions()) if (interaction.id == interactionId && interaction.kind == "chat-user") {
                // 【逐行批注】 计算右侧结果并写入 courseId，更新当前运行状态。
                courseId = interaction.courseId.value_or(""); answer = Json::parse(interaction.payload); break;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (answer.empty()) throw std::invalid_argument("学生输入标识不存在：interact…，成立时执行对应分支。
            if (answer.empty()) throw std::invalid_argument("学生输入标识不存在：interactionId 必须使用 currentInputs 中的学生记录 ID，不能使用板块或题目 ID");
            // 【逐行批注】 调用 requireCourse 完成本行所需操作：requireCourse(db, access, courseId);。
            requireCourse(db, access, courseId);
            // 【逐行批注】 判断条件 (answer.empty() || !answer.value("questionId", Json()).is_string…，成立时执行对应分支。
            if (answer.empty() || !answer.value("questionId", Json()).is_string() || !answer.value("questionSnapshot", Json()).is_object())
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("缺少可靠原题与实际作答，不能建立正式评价");。
                throw std::invalid_argument("缺少可靠原题与实际作答，不能建立正式评价");
            // 【逐行批注】 判断条件 (answer.value("scopeId", "") != access.scopeId) throw std::inval…，成立时执行对应分支。
            if (answer.value("scopeId", "") != access.scopeId) throw std::invalid_argument("无权评价该回答");
            // 【逐行批注】 判断条件 (answer.value("type", "") == "teaching_request" || !db.profileMe…，成立时执行对应分支。
            if (answer.value("type", "") == "teaching_request" || !db.profileMeta("agent-input-classification:" + interactionId).empty() ||
                // 【逐行批注】 执行当前声明或表达式：answer.value("action", "answer") == "skip" || answer.value("action", "answer") == "s…。
                answer.value("action", "answer") == "skip" || answer.value("action", "answer") == "stop" ||
                // 【逐行批注】 执行当前声明或表达式：answer.value("action", "answer") == "omitted" || answer.value("action", "answer") ==…。
                answer.value("action", "answer") == "omitted" || answer.value("action", "answer") == "hint")
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("跳过和停止不作为可靠评价");。
                throw std::invalid_argument("跳过和停止不作为可靠评价");
            // 【逐行批注】 判断条件 (!args.value("credible", Json()).is_boolean() || !args.contains(…，成立时执行对应分支。
            if (!args.value("credible", Json()).is_boolean() || !args.contains("score") ||
                // 【逐行批注】 执行当前声明或表达式：(!args["score"].is_null() && (!args["score"].is_number_integer() || args["score"].ge…。
                (!args["score"].is_null() && (!args["score"].is_number_integer() || args["score"].get<int>() < 0 || args["score"].get<int>() > 100)))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("评价分数与可信状态无效");。
                throw std::invalid_argument("评价分数与可信状态无效");
            // 【逐行批注】 判断条件 (!args["credible"].get<bool>() && !args["score"].is_null()) thro…，成立时执行对应分支。
            if (!args["credible"].get<bool>() && !args["score"].is_null()) throw std::invalid_argument("不可靠评价不能提交正式分数");
            const auto& snapshot = answer.at("questionSnapshot");
            Json checkedChoice;
            if (snapshot.contains("setExercise") && !args.at("score").is_null()) {
                if (!answer.contains("selectedOption")) {
                    if (answer.value("action", "") != "unknown" || args.at("score") != 0)
                        throw std::invalid_argument("此题没有明确提交的选项。请先澄清，不能给出正式选项分数");
                } else {
                    const auto index = checkedSetOption(snapshot, answer.at("selectedOption"));
                    const bool correct = index == snapshot.at("answerIndex");
                    if (args.at("score") != (correct ? 100 : 0))
                        throw std::invalid_argument("分数与题面及实际提交的选项不一致。请根据 selectedOption 和 answerIndex 修正分数，理由另行诊断");
                    checkedChoice = {{"selectedOption", index}, {"choiceCorrect", correct}};
                }
            }
            // 【逐行批注】 计算右侧结果并写入 Json value，更新当前运行状态。
            Json value = {{"interactionId", interactionId}, {"questionId", answer["questionId"]},
                // 【逐行批注】 执行当前声明或表达式：{"questionSnapshot", answer["questionSnapshot"]}, {"response", answer["text"]},。
                {"questionSnapshot", answer["questionSnapshot"]}, {"response", answer["text"]},
                // 【逐行批注】 调用 array 完成本行所需操作：{"assisted", answer.value("assisted", false)}, {"hintHistory", answer…。
                {"assisted", answer.value("assisted", false)}, {"hintHistory", answer.value("hintHistory", Json::array())},
                // 【逐行批注】 调用 text 完成本行所需操作：{"score", args["score"]}, {"credible", args["credible"]}, {"feedback"…。
                {"score", args["score"]}, {"credible", args["credible"]}, {"feedback", text(args, "feedback")},
                // 【逐行批注】 调用 text 完成本行所需操作：{"reason", text(args, "reason")}, {"uncertainty", text(args, "uncerta…。
                {"reason", text(args, "reason")}, {"uncertainty", text(args, "uncertainty")},
                // 【逐行批注】 调用 now 完成本行所需操作：{"source", "ai"}, {"model", source.model}, {"status", "valid"}, {"at"…。
                {"source", "ai"}, {"model", source.model}, {"status", "valid"}, {"at", now()}};
            // 【逐行批注】 判断条件 (task.value("teachingProtocolVersion", 1) >= 2 && !args.contains…，成立时执行对应分支。
            if (task.value("teachingProtocolVersion", 1) >= 2 && !args.contains("diagnosis"))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("本次评价需要 diagnosis：具体卡点、实际回答原话依据、补讲方向和是否先澄…。
                throw std::invalid_argument("本次评价需要 diagnosis：具体卡点、实际回答原话依据、补讲方向和是否先澄清");
            // 【逐行批注】 计算右侧结果并写入 auto feedbackHelp，更新当前运行状态。
            const auto feedbackHelp = args.value("feedbackHelpType", "unknown");
            // 【逐行批注】 判断条件 (!teachingHelpType(feedbackHelp) || (task.value("teachingProtoco…，成立时执行对应分支。
            if (!teachingHelpType(feedbackHelp) || (task.value("teachingProtocolVersion", 1) >= 3 && feedbackHelp == "unknown"))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("feedbackHelpType 必须明确反馈实际帮助程度：none、expla…。
                throw std::invalid_argument("feedbackHelpType 必须明确反馈实际帮助程度：none、explanation、hint、worked_example 或 clarification；给出本题答案或完整步骤属于 worked_example");
            // 【逐行批注】 计算右侧结果并写入 value["feedbackHelpType"]，更新当前运行状态。
            value["feedbackHelpType"] = feedbackHelp;
            // 【逐行批注】 计算右侧结果并写入 auto demonstrations，更新当前运行状态。
            const auto demonstrations = checkedDemonstrations(args);
            // 【逐行批注】 判断条件 (!demonstrations.empty()) value["demonstratedQuestions"] = demon…，成立时执行对应分支。
            if (!demonstrations.empty()) value["demonstratedQuestions"] = demonstrations;
            // 【逐行批注】 计算右侧结果并写入 value["diagnosis"]，更新当前运行状态。
            value["diagnosis"] = checkedDiagnosis(args, answer.at("text"));
            if (checkedChoice.is_object()) value.update(checkedChoice);
            // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"lessonId", "sectionId", "view", "helpHistory", …。
            for (const auto* field : {"lessonId", "sectionId", "roundId", "view", "helpHistory", "relatedLearning", "helpMode", "rePracticeOf"})
                // 【逐行批注】 判断条件 (answer.contains(field)) value[field] = answer.at(field);，成立时执行对应分支。
                if (answer.contains(field)) value[field] = answer.at(field);
            // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
            const auto id = newId("evaluation");
            // 【逐行批注】 计算右侧结果并写入 bool currentInput，更新当前运行状态。
            bool currentInput = answer.value("requestId", "") == task.at("event").value("requestId", "");
            // 【逐行批注】 按循环规则重复处理：for (const auto& input : task.at("event").value("batchAnswers", Json::arra…。
            for (const auto& input : task.at("event").value("batchAnswers", Json::array())) currentInput = currentInput || input.at("requestId") == answer.at("requestId");
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"evaluationId", id}, {"saved", true}, {"in…。
            return PreparedAgentTool{{{"evaluationId", id}, {"saved", true}, {"publicationAtSave", "staged"}, {"inputResolved", currentInput},
                // 【逐行批注】 调用 Json 完成本行所需操作：{"resolvedRequestId", currentInput ? answer.at("requestId") : Json(""…。
                {"resolvedRequestId", currentInput ? answer.at("requestId") : Json("")}, {"evaluation", value}}, [value, id, courseId](Database& connection) {
                // 【逐行批注】 判断条件 (!connection.upsert(ClassroomActivity{id, courseId, "agent-evalu…，成立时执行对应分支。
                if (!connection.upsert(ClassroomActivity{id, courseId, "agent-evaluation", value.dump(), now(), 0, 0}))
                    // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("真实评价暂存失败");。
                    throw std::runtime_error("真实评价暂存失败");
            // 【逐行批注】 执行当前声明或表达式：}};。
            }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("classify_input", {"判断本次题目输入是否只是追问。参数 interactionId、isAnswer:fals…。
    agent.registerTool("save_learning_state", {u8"提出并保存本课理解状态，不修改正式成绩。参数lessonId,status:UNKNOWN|NOT_STARTED|LEARNING|PARTIAL|MISUNDERSTOOD|PRACTICING|PROVISIONAL_MASTERY|VERIFIED_MASTERY,reason,uncertainty,evidenceIds:[本课已发布评价ID],nextAction:explain|ask|hint|practice|review|advance|pause。只读公开课时的evaluations提供可用ID；先完成实际教学并发布评价再判断。不能把讲解、浏览、探索、未答或教师示范当掌握。初步掌握须独立作答，已验证掌握须多次不同题独立验证；语义相同或已示范题不能算新证据。reason解释依据及局限，证据变化后旧判断会失效，可按新表现回退。nextAction仅是保存的教学建议，后续行动仍通过原主控工具执行。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto access = accessFor(task);
            const auto id = text(args, "lessonId");
            const auto rawLesson = lessonFor(db, access, id);
            requireTeachingTarget(task, rawLesson);
            if (task.at("event").value("lessonId", "") != id)
                throw std::invalid_argument("理解状态只能保存到当前教学事件的实际课时");
            const auto shown = agentLessonView(db, access, id);
            auto assessment = checkedLearningMastery(shown, args);
            assessment["scopeId"] = access.scopeId;
            assessment["courseId"] = rawLesson.at("courseId");
            assessment["lessonId"] = id;
            assessment["basisDigest"] = libraryDigest(learningStateBasis(shown).dump());
            assessment["model"] = source.model;
            assessment["at"] = now();
            const auto recordId = "learning-state:" + id;
            const auto previous = db.getClassroomActivity(recordId);
            const auto before = previous ? previous->payload : std::string();
            assessment["version"] = previous ? Json::parse(before).value("version", 0) + 1 : 1;
            return PreparedAgentTool{{{"learningStateSaved", true}, {"status", assessment.at("status")},
                                      {"lessonId", id}, {"version", assessment.at("version")}},
                [access, id, rawLesson, assessment, recordId, before](Database& connection) {
                    if (lessonFor(connection, access, id) != rawLesson ||
                        libraryDigest(learningStateBasis(agentLessonView(connection, access, id)).dump()) != assessment.at("basisDigest"))
                        throw std::invalid_argument("课时或学习证据已变化，旧理解判断不能应用");
                    const auto current = connection.getClassroomActivity(recordId);
                    if ((current ? current->payload : std::string()) != before)
                        throw std::invalid_argument("理解状态已由其它任务更新，请重新读取");
                    if (!connection.upsert(ClassroomActivity{recordId, rawLesson.at("courseId"), "agent-learning-state",
                                                            assessment.dump(), assessment.at("at"), 0, 0}))
                        throw std::runtime_error("理解状态保存失败，原成绩与作答保持原样");
                }};
        }});
    agent.registerTool("classify_input", {"判断本次题目输入是否只是追问。参数 interactionId、isAnswer:false、reason。真正作答或明确不会请用 evaluate_answer，不为纯追问增加评分。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult&) {。
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            // 【逐行批注】 判断条件 (!args.value("isAnswer", Json()).is_boolean() || args.at("isAnsw…，成立时执行对应分支。
            if (!args.value("isAnswer", Json()).is_boolean() || args.at("isAnswer").get<bool>())
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("实际作答必须先评价");。
                throw std::invalid_argument("实际作答必须先评价");
            // 【逐行批注】 计算右侧结果并写入 text(args, "reason"); const auto id，更新当前运行状态。
            text(args, "reason"); const auto id = text(args, "interactionId");
            // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.listInteractions()) if (row.id == id && row.kind…。
            for (const auto& row : db.listInteractions()) if (row.id == id && row.kind == "chat-user") {
                // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
                const auto input = Json::parse(row.payload);
                // 【逐行批注】 计算右侧结果并写入 bool currentInput，更新当前运行状态。
                bool currentInput = input.value("requestId", "") == task.at("event").value("requestId", "");
                // 【逐行批注】 按循环规则重复处理：for (const auto& answer : task.at("event").value("batchAnswers", Json::arr…。
                for (const auto& answer : task.at("event").value("batchAnswers", Json::array())) currentInput = currentInput || answer.at("requestId") == input.at("requestId");
                // 【逐行批注】 判断条件 (input.value("scopeId", "") != task.at("scopeId") || !currentInp…，成立时执行对应分支。
                if (input.value("scopeId", "") != task.at("scopeId") || !currentInput)
                    // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("只能判断本次真实输入");。
                    throw std::invalid_argument("只能判断本次真实输入");
                if (input.contains("selectedOption"))
                    throw std::invalid_argument("学生已经明确提交选项，须按实际选项评价，不能当成纯追问");
                // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"inputResolved", true}, {"resolvedRequestI…。
                return PreparedAgentTool{{{"inputResolved", true}, {"resolvedRequestId", input.at("requestId")}, {"isAnswer", false}, {"reason", args.at("reason")}},
                    // 【逐行批注】 计算右侧结果并写入 [id, reason，更新当前运行状态。
                    [id, reason = args.at("reason")](Database& connection) {
                        // 【逐行批注】 执行当前声明或表达式：connection.setProfileMeta("agent-input-classification:" + id, Json{{"isAnswer", fals…。
                        connection.setProfileMeta("agent-input-classification:" + id, Json{{"isAnswer", false}, {"reason", reason}}.dump()); }};
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("学生输入不存在");。
            throw std::invalid_argument("学生输入不存在");
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
// 【逐行批注】 结束当前代码块。
}

bool finalizeAgentEvaluations(Database& db, const Json& task) {
    bool roundAdvanced = false;
    // 【逐行批注】 按循环规则重复处理：for (const auto& entry : task.at("completedActions").items()) {。
    for (const auto& entry : task.at("completedActions").items()) {
        // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
        const auto result = entry.value().value("result", Json::object());
        // 【逐行批注】 判断条件 (!result.contains("evaluationId")) continue;，成立时执行对应分支。
        if (!result.contains("evaluationId")) continue;
        // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
        const auto id = result.at("evaluationId").get<std::string>();
        // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
        const auto row = db.getClassroomActivity(id);
        // 【逐行批注】 判断条件 (!row || row->kind != "agent-evaluation") throw std::runtime_err…，成立时执行对应分支。
        if (!row || row->kind != "agent-evaluation") throw std::runtime_error("暂存评价不存在");
        // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
        auto value = Json::parse(row->payload);
        // 【逐行批注】 判断条件 (value.value("published", false)) continue;，成立时执行对应分支。
        if (value.value("published", false)) continue;
        // 同轮若又被确认是纯追问，保留暂存历史但不发布成掌握证据。
        // 【逐行批注】 判断条件 (!db.profileMeta("agent-input-classification:" + value.at("inter…，成立时执行对应分支。
        if (!db.profileMeta("agent-input-classification:" + value.at("interactionId").get<std::string>()).empty()) {
            // 【逐行批注】 计算右侧结果并写入 value["status"]，更新当前运行状态。
            value["status"] = "discarded_followup"; value["published"] = true;
            // 【逐行批注】 判断条件 (!db.upsert(ClassroomActivity{id, row->courseId, row->kind, valu…，成立时执行对应分支。
            if (!db.upsert(ClassroomActivity{id, row->courseId, row->kind, value.dump(), now(), 0, 0}))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("追问评价撤销标记保存失败");。
                throw std::runtime_error("追问评价撤销标记保存失败");
            // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
            continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (!db.insert(LearningInteraction{id, "practice", value.dump(), no…，成立时执行对应分支。
        Json lesson;
        if (value.contains("roundId")) {
            lesson = lessonFor(db, accessFor(task), value.at("lessonId"));
            if (!activeTeachingRound(lesson) || lesson.value("teachingTaskId", "") != task.at("id") ||
                lesson.at("teachingRound").at("roundId") != value.at("roundId") ||
                lesson.at("teachingRound").value("pendingSectionId", "") != value.at("sectionId") ||
                !roundContainsQuestion(lesson.at("teachingRound"), value.at("questionId"))) continue;
        }
        if (!db.insert(LearningInteraction{id, "practice", value.dump(), now(), row->courseId,
            // 【逐行批注】 调用 runtime_error 完成本行所需操作：value.at("interactionId").get<std::string>(), {}})) throw std::runtim…。
            value.at("interactionId").get<std::string>(), {}})) throw std::runtime_error("完整教学评价保存失败");
        // 【逐行批注】 计算右侧结果并写入 value["published"]，更新当前运行状态。
        value["published"] = true;
        // 【逐行批注】 判断条件 (!db.upsert(ClassroomActivity{id, row->courseId, row->kind, valu…，成立时执行对应分支。
        if (!db.upsert(ClassroomActivity{id, row->courseId, row->kind, value.dump(), now(), 0, 0}))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("评价提交标记保存失败");。
            throw std::runtime_error("评价提交标记保存失败");
        // 发布反馈和释放下一题使用同一事务。需要澄清的回答继续留在当前题。
        if (!lesson.is_object() || value.value("diagnosis", Json::object()).value("needsClarification", false)) continue;
        auto& round = lesson.at("teachingRound");
        round["evaluatedQuestionIds"].push_back(value.at("questionId"));
        round.erase("pendingSectionId");
        lesson["version"] = lesson.at("version").get<int>() + 1;
        lesson["updatedAt"] = now();
        saveLesson(db, lesson);
        roundAdvanced = true;
    }
    return roundAdvanced;
}

// 【逐行批注】 定义函数 agentRecordLearnerEvent，执行该函数声明的职责。
Json agentResolveLessonUtterance(Database& db, const AgentAccess& access, const Json& event) {
    if (event.value("type", "") != "chat" || !event.value("lessonId", Json()).is_string() ||
        !event.value("text", Json()).is_string() || event.contains("requestType") || event.contains("batchAnswers")) return nullptr;

    const auto& body = event.at("text").get_ref<const std::string&>();
    if (body.size() > 160) return nullptr;
    const auto start = body.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return nullptr;
    auto phrase = body.substr(start, body.find_last_not_of(" \t\r\n") - start + 1);
    if (phrase.rfind(u8"请", 0) == 0) phrase.erase(0, sizeof(u8"请") - 1);
    for (const std::string ending : {u8"。", u8"？", u8"！", ".", "?", "!"}) {
        if (phrase.size() >= ending.size() && phrase.compare(phrase.size() - ending.size(), ending.size(), ending) == 0) {
            phrase.erase(phrase.size() - ending.size());
            break;
        }
    }
    // 只匹配整句明确请求，引用、提问和附带理由的作答继续由原对话处理。
    static const std::map<std::string, std::string> intents = {
        {u8"慢一点", "slow_down"}, {u8"讲慢一点", "slow_down"}, {u8"讲慢一点吧", "slow_down"},
        {u8"能讲慢一点吗", "slow_down"}, {u8"能不能讲慢一点", "slow_down"},
        {u8"快一点", "speed_up"}, {u8"讲快一点", "speed_up"},
        {u8"讲简单一点", "simplify"}, {u8"能讲简单一点吗", "simplify"},
        {u8"举个例子", "give_example"}, {u8"举个例子吧", "give_example"},
        {u8"我想自己做", "independent_attempt"}, {u8"我想自己尝试", "independent_attempt"},
        {u8"让我自己试试", "independent_attempt"}, {u8"我想先自己做", "independent_attempt"},
        {u8"继续引导我", "resume_guidance"}, {u8"继续讲解", "resume_guidance"},
        {u8"恢复默认讲法", "reset_preferences"}, {u8"恢复默认讲解方式", "reset_preferences"},
        {u8"换一种讲法", "rephrase"}, {u8"换种解释", "rephrase"}, {u8"换种说法", "rephrase"},
        {u8"再测一次", "retry_question"}, {u8"再试一道", "retry_question"}, {u8"再来一道题", "retry_question"}
    };
    const auto matched = intents.find(phrase);
    if (matched == intents.end()) return nullptr;
    const auto lesson = lessonFor(db, access, event.at("lessonId"));
    if (lesson.at("status") != "ready" || lesson.at("courseId") != event.value("courseId", ""))
        throw std::invalid_argument("课程尚未准备好或所属课程不匹配");
    if (!lesson.value("entered", false)) return nullptr;
    const auto& sections = lesson.at("sections");
    const auto findSection = [&](const std::string& id) {
        return std::find_if(sections.begin(), sections.end(), [&](const Json& section) { return section.at("id") == id; });
    };
    auto section = sections.end();
    if (event.contains("sectionId")) {
        section = findSection(text(event, "sectionId"));
        if (section == sections.end()) throw std::invalid_argument("交流板块不属于该课时");
    }

    auto resolved = event;
    resolved.erase("sectionId");
    resolved.erase("sectionVersion");
    const auto& intent = matched->second;
    if (intent == "retry_question") {
        if (section == sections.end())
            section = findSection(lesson.value("teachingFocus", Json::object()).value("practiceSectionId", ""));
        // 待答题优先保留；没有可关联原题时继续走普通课堂交流。
        if (section == sections.end() || section->value("kind", "") != "question" ||
            section->value("legacyKind", "") == "example" ||
            (activeTeachingRound(lesson) && lesson.at("teachingRound").contains("pendingSectionId"))) return nullptr;
        resolved["sectionId"] = section->at("id");
        resolved["sectionVersion"] = event.value("sectionVersion", section->at("version"));
    }
    resolved["type"] = "teaching_request";
    resolved["requestType"] = intent;
    if (isTeachingPreferenceIntent(intent) && !event.contains("teachingPreferenceVersion"))
        resolved["teachingPreferenceVersion"] = teachingPreferencesProjection(lesson).at("version");
    return resolved;
}

void agentRecordLearnerEvent(Database& db, const AgentAccess& access, const Json& event) {
    // 【逐行批注】 判断条件 (event.contains("batchAnswers")) {，成立时执行对应分支。
    if (event.contains("batchAnswers")) {
        // 【逐行批注】 判断条件 (!event.at("batchAnswers").is_array() || event.at("batchAnswers"…，成立时执行对应分支。
        if (!event.at("batchAnswers").is_array() || event.at("batchAnswers").empty()) throw std::invalid_argument("批量作答列表无效");
        // 【逐行批注】 按循环规则重复处理：for (const auto& answer : event.at("batchAnswers")) agentRecordLearnerEven…。
        for (const auto& answer : event.at("batchAnswers")) agentRecordLearnerEvent(db, access, answer);
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto type，更新当前运行状态。
    const auto type = event.value("type", "");
    // 【逐行批注】 判断条件 (type != "answer" && type != "chat" && type != "feedback" && typ…，成立时执行对应分支。
    if (type != "answer" && type != "chat" && type != "feedback" && type != "question_answer" && type != "teaching_request") return;
    if (event.contains("selectedOption") && (type != "question_answer" || event.value("action", "answer") != "answer" || !event.contains("lessonId")))
        throw std::invalid_argument("选项只用于当前题的正式提交");
    // 【逐行批注】 判断条件 (type == "teaching_request" && !event.contains("lessonId")) thro…，成立时执行对应分支。
    if (type == "teaching_request" && !event.contains("lessonId")) throw std::invalid_argument("教学请求必须关联当前课时");
    // 【逐行批注】 计算右侧结果并写入 Json value，更新当前运行状态。
    Json value = {{"text", text(event, "text")}, {"scopeId", access.scopeId}, {"action", event.value("action", "answer")},
        // 【逐行批注】 执行当前声明或表达式：{"requestId", event.at("requestId")}, {"type", type}, {"conversationId", event.value…。
        {"requestId", event.at("requestId")}, {"type", type}, {"conversationId", event.value("conversationId", "general")}};
    // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"reviewId", "day"}) if (event.contains(field)) v…。
    for (const auto* field : {"reviewId", "day"}) if (event.contains(field)) value[field] = event.at(field);
    // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
    const auto courseId = event.value("courseId", "");
    // 【逐行批注】 判断条件 (!courseId.empty()) requireCourse(db, access, courseId);，成立时执行对应分支。
    if (!courseId.empty()) requireCourse(db, access, courseId);
    // 【逐行批注】 判断条件 (event.contains("lessonId")) {，成立时执行对应分支。
    if (event.contains("componentActivities") && !event.contains("lessonId"))
        throw std::invalid_argument("组件活动必须绑定课时");
    if (event.contains("lessonId")) {
        // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
        const auto lesson = lessonFor(db, access, text(event, "lessonId"));
        // 【逐行批注】 判断条件 (lesson.at("status") != "ready" || lesson.at("courseId") != cour…，成立时执行对应分支。
        if (lesson.at("status") != "ready" || lesson.at("courseId") != courseId) throw std::invalid_argument("课程尚未准备好或所属课程不匹配");
        // 【逐行批注】 计算右侧结果并写入 value["lessonId"]，更新当前运行状态。
        value["lessonId"] = lesson.at("id");
        if (lesson.contains("topicId")) value["topicId"] = lesson.at("topicId");
        if (event.contains("componentActivities")) value["componentActivities"] = checkedComponentActivities(lesson, event);
        // 【逐行批注】 判断条件 (type != "question_answer" && type != "teaching_request" && even…，成立时执行对应分支。
        if (type != "question_answer" && type != "teaching_request" && event.contains("sectionId")) {
            // 【逐行批注】 计算右侧结果并写入 auto sectionId，更新当前运行状态。
            const auto sectionId = text(event, "sectionId"); bool found = false;
            // 【逐行批注】 按循环规则重复处理：for (const auto& section : lesson.at("sections")) if (section.at("id") == …。
            for (const auto& section : lesson.at("sections")) if (section.at("id") == sectionId) found = true;
            // 【逐行批注】 判断条件 (!found) throw std::invalid_argument("交流板块不属于该课时");，成立时执行对应分支。
            if (!found) throw std::invalid_argument("交流板块不属于该课时");
            // 【逐行批注】 计算右侧结果并写入 value["sectionId"]，更新当前运行状态。
            value["sectionId"] = sectionId;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (type == "question_answer" || (type == "teaching_request" && eve…，成立时执行对应分支。
        if (type == "question_answer" || (type == "teaching_request" && event.contains("sectionId"))) {
        // 【逐行批注】 计算右侧结果并写入 auto sectionId，更新当前运行状态。
        const auto sectionId = text(event, "sectionId"); const Json* section = nullptr;
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : lesson.at("sections")) if (item.at("id") == sectio…。
        for (const auto& item : lesson.at("sections")) if (item.at("id") == sectionId) { section = &item; break; }
        // 【逐行批注】 判断条件 (!section || section->at("kind") != "question" || !event.value("…，成立时执行对应分支。
        if (!section || section->at("kind") != "question" || !event.value("sectionVersion", Json()).is_number_integer() ||
            // 【逐行批注】 调用 at、invalid_argument 完成本行所需操作：event.at("sectionVersion") != section->at("version")) throw std::inva…。
            event.at("sectionVersion") != section->at("version")) throw std::invalid_argument("题目版本已变化，请保留输入并刷新");
        // 【逐行批注】 判断条件 (section->value("legacyKind", "") == "example") throw std::inval…，成立时执行对应分支。
        if (section->value("legacyKind", "") == "example") throw std::invalid_argument("示范例题只用于讲解，请以追问方式交流");
        if (type == "question_answer") {
            requireRoundQuestion(lesson, *section);
            if (section->contains("roundId")) value["roundId"] = section->at("roundId");
            if (event.contains("selectedOption")) {
                if (!section->at("question").contains("setExercise")) throw std::invalid_argument("当前题不接受程序核验的单选项");
                value["selectedOption"] = checkedSetOption(section->at("question"), event.at("selectedOption"));
            }
        }
        // 【逐行批注】 计算右侧结果并写入 value["questionId"]，更新当前运行状态。
        value["questionId"] = section->at("questionId"); value["questionSnapshot"] = section->at("question");
        // 【逐行批注】 计算右侧结果并写入 value["view"]，更新当前运行状态。
        value["view"] = responseView(lesson, Json{{"event", event}});
        // 【逐行批注】 计算右侧结果并写入 value["lessonId"]，更新当前运行状态。
        value["lessonId"] = lesson.at("id"); value["sectionId"] = sectionId;
        // 【逐行批注】 计算右侧结果并写入 Json hints，更新当前运行状态。
        Json hints = Json::array(), help = Json::array(), related = Json::array(), shownDemonstrations = Json::array(); bool unassignedExample = false;
        // 【逐行批注】 计算右侧结果并写入 auto questionId，更新当前运行状态。
        const auto questionId = section->at("questionId");
        // 【逐行批注】 判断条件 (section->contains("rePracticeOf")) value["rePracticeOf"] = sect…，成立时执行对应分支。
        if (section->contains("rePracticeOf")) value["rePracticeOf"] = section->at("rePracticeOf");
        // 规划新题也需要看见其他题的真实示范；索引独立于本题帮助，不改变精确匹配结果。
        // 【逐行批注】 计算右侧结果并写入 auto rememberDemonstrations，更新当前运行状态。
        const auto rememberDemonstrations = [&](const Json& content, bool partial, const Json& source) {
            // 【逐行批注】 执行当前声明或表达式：Json snapshots;。
            Json snapshots;
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try { snapshots = checkedDemonstrations(content); }
            // 【逐行批注】 捕获并处理异常：catch (const std::exception&) { value["demonstrationHistoryIncomplete…。
            catch (const std::exception&) { value["demonstrationHistoryIncomplete"] = true; return; }
            // 【逐行批注】 按循环规则重复处理：for (const auto& snapshot : snapshots) {。
            for (const auto& snapshot : snapshots) {
                // 【逐行批注】 计算右侧结果并写入 auto identity，更新当前运行状态。
                const auto identity = teachingQuestionIdentity(courseId, snapshot); bool found = false;
                // 【逐行批注】 按循环规则重复处理：for (auto& shown : shownDemonstrations) if (shown.at("questionId") == iden…。
                for (auto& shown : shownDemonstrations) if (shown.at("questionId") == identity) {
                    // 【逐行批注】 计算右侧结果并写入 shown["complete"]，更新当前运行状态。
                    shown["complete"] = shown.at("complete").get<bool>() || !partial;
                    // 【逐行批注】 判断条件 (std::find(shown["sources"].begin(), shown["sources"].end(), sou…，成立时执行对应分支。
                    if (std::find(shown["sources"].begin(), shown["sources"].end(), source) == shown["sources"].end()) shown["sources"].push_back(source);
                    // 【逐行批注】 计算右侧结果并写入 found，更新当前运行状态。
                    found = true; break;
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 判断条件 (!found) shownDemonstrations.push_back({{"questionId", identity}…，成立时执行对应分支。
                if (!found) shownDemonstrations.push_back({{"questionId", identity}, {"questionSnapshot", snapshot},
                    // 【逐行批注】 调用 array 完成本行所需操作：{"complete", !partial}, {"sources", Json::array({source})}});。
                    {"complete", !partial}, {"sources", Json::array({source})}});
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 计算右侧结果并写入 auto remember，更新当前运行状态。
        const auto remember = [&](Json entry) {
            // 【逐行批注】 计算右侧结果并写入 auto helpType，更新当前运行状态。
            const auto helpType = entry.value("helpType", "unknown");
            // 【逐行批注】 判断条件 (helpType == "explanation" || entry.value("association", "") == …，成立时执行对应分支。
            if (helpType == "explanation" || entry.value("association", "") == "lesson_only") {
                // 【逐行批注】 判断条件 (entry.contains("sectionId")) for (const auto& shown : lesson.at…，成立时执行对应分支。
                if (entry.contains("sectionId")) for (const auto& shown : lesson.at("sections"))
                    // 【逐行批注】 判断条件 (shown.at("id") == entry.at("sectionId") && shown.contains("demo…，成立时执行对应分支。
                    if (shown.at("id") == entry.at("sectionId") && shown.contains("demonstratedQuestions"))
                        // 【逐行批注】 计算右侧结果并写入 entry["demonstratedQuestions"]，更新当前运行状态。
                        entry["demonstratedQuestions"] = shown.at("demonstratedQuestions");
                // 【逐行批注】 执行当前声明或表达式：related.push_back(entry);。
                related.push_back(entry);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 在前一条件不成立时继续判断：(helpType == "hint" || helpType == "worked_example" || helpType …。
            else if (helpType == "hint" || helpType == "worked_example" || helpType == "unknown") {
                // 【逐行批注】 执行当前声明或表达式：help.push_back(entry); hints.push_back(entry.at("text"));。
                help.push_back(entry); hints.push_back(entry.at("text"));
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 计算右侧结果并写入 auto exposure，更新当前运行状态。
        const auto exposure = Json::parse(db.profileMeta("agent-section-exposure:" + lesson.at("id").get<std::string>()), nullptr, false);
        // 【逐行批注】 计算右侧结果并写入 auto displayDetails，更新当前运行状态。
        const auto displayDetails = Json::parse(db.profileMeta("agent-section-display:" + lesson.at("id").get<std::string>()), nullptr, false);
        // 【逐行批注】 判断条件 (exposure.is_object()) for (const auto& item : lesson.at("sectio…，成立时执行对应分支。
        if (exposure.is_object()) for (const auto& item : lesson.at("sections"))
            // 【逐行批注】 判断条件 (item.at("kind") != "question" && exposure.value(item.at("id").g…，成立时执行对应分支。
            if (item.at("kind") != "question" && exposure.value(item.at("id").get<std::string>(), 0) == item.at("version") && item.value("body", "") != "") {
                // 【逐行批注】 计算右侧结果并写入 auto detail，更新当前运行状态。
                const auto detail = displayDetails.is_object() ? displayDetails.value(item.at("id").get<std::string>(), Json::object()) : Json::object();
                // 【逐行批注】 计算右侧结果并写入 bool complete，更新当前运行状态。
                const bool complete = detail.value("version", Json()) == item.at("version") && detail.value("complete", false);
                // 【逐行批注】 调用 rememberDemonstrations 完成本行所需操作：rememberDemonstrations(item, !complete, {{"kind", "section"}, {"id", …。
                rememberDemonstrations(item, !complete, {{"kind", "section"}, {"id", item.at("id")}, {"version", item.at("version")}});
                // 【逐行批注】 判断条件 (matchesDemonstration(item, courseId, section->at("question"))) {，成立时执行对应分支。
                if (matchesDemonstration(item, courseId, section->at("question"))) {
                    // 【逐行批注】 调用 remember 完成本行所需操作：remember({{"helpType", complete ? "worked_example" : "unknown"}, {"pa…。
                    remember({{"helpType", complete ? "worked_example" : "unknown"}, {"partial", !complete}, {"text", item.at("body")}, {"questionId", questionId},
                        // 【逐行批注】 执行当前声明或表达式：{"sectionId", item.at("id")}, {"sectionVersion", item.at("version")}, {"source", "de…。
                        {"sectionId", item.at("id")}, {"sectionVersion", item.at("version")}, {"source", "demonstrated_question"}});
                // 【逐行批注】 执行当前声明或表达式：} else if (!item.contains("helpType") && item.value("presentation", "") == "example"…。
                } else if (!item.contains("helpType") && item.value("presentation", "") == "example") {
                    // 【逐行批注】 计算右侧结果并写入 unassignedExample，更新当前运行状态。
                    unassignedExample = true;
                    // 【逐行批注】 调用 remember 完成本行所需操作：remember({{"helpType", "unknown"}, {"association", "lesson_only"}, {"…。
                    remember({{"helpType", "unknown"}, {"association", "lesson_only"}, {"text", item.at("body")},
                        // 【逐行批注】 执行当前声明或表达式：{"sectionId", item.at("id")}, {"sectionVersion", item.at("version")}, {"source", "le…。
                        {"sectionId", item.at("id")}, {"sectionVersion", item.at("version")}, {"source", "legacy"}});
                // 【逐行批注】 执行当前声明或表达式：} else if (item.value("helpType", "explanation") == "explanation" || item.value("tar…。
                } else if (item.value("helpType", "explanation") == "explanation" || item.value("targetQuestionId", "") == questionId)
                    // 【逐行批注】 调用 remember 完成本行所需操作：remember({{"helpType", !complete && (item.value("helpType", "explanat…。
                    remember({{"helpType", !complete && (item.value("helpType", "explanation") == "hint" || item.value("helpType", "explanation") == "worked_example") ? "unknown" : item.value("helpType", "explanation")}, {"partial", !complete}, {"text", item.at("body")},
                        // 【逐行批注】 执行当前声明或表达式：{"sectionId", item.at("id")}, {"sectionVersion", item.at("version")}, {"questionId",…。
                        {"sectionId", item.at("id")}, {"sectionVersion", item.at("version")}, {"questionId", item.value("targetQuestionId", "")}});
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : section->value("legacyDialog", Json::array())) if …。
        for (const auto& item : section->value("legacyDialog", Json::array())) if (item.value("role", "") == "assistant")
            // 【逐行批注】 调用 remember 完成本行所需操作：remember({{"helpType", "unknown"}, {"text", item.at("text")}, {"quest…。
            remember({{"helpType", "unknown"}, {"text", item.at("text")}, {"questionId", questionId}, {"source", "legacy"}});
        // 【逐行批注】 计算右侧结果并写入 auto evaluationExposure，更新当前运行状态。
        const auto evaluationExposure = Json::parse(db.profileMeta("agent-evaluation-exposure:" + lesson.at("id").get<std::string>()), nullptr, false);
        // 【逐行批注】 判断条件 (evaluationExposure.is_object()) for (const auto& item : evaluat…，成立时执行对应分支。
        if (evaluationExposure.is_object()) for (const auto& item : evaluationExposure.items()) {
            // 【逐行批注】 计算右侧结果并写入 entry，更新当前运行状态。
            auto entry = item.value();
            // 【逐行批注】 调用 rememberDemonstrations 完成本行所需操作：rememberDemonstrations(entry, entry.value("partial", false), {{"kind"…。
            rememberDemonstrations(entry, entry.value("partial", false), {{"kind", "evaluation"}, {"id", item.key()}});
            // 【逐行批注】 计算右侧结果并写入 bool demonstrated，更新当前运行状态。
            const bool demonstrated = matchesDemonstration(entry, courseId, section->at("question"));
            // 【逐行批注】 判断条件 (demonstrated) { entry["helpType"] = entry.value("partial", fals…，成立时执行对应分支。
            if (demonstrated) { entry["helpType"] = entry.value("partial", false) ? "unknown" : "worked_example"; entry["questionId"] = questionId; }
            // 【逐行批注】 判断条件 (demonstrated || entry.value("questionIdentity", "") == teaching…，成立时执行对应分支。
            if (demonstrated || entry.value("questionIdentity", "") == teachingQuestionIdentity(courseId, section->at("question"))) remember(entry);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.listClassroomActivities(courseId)) if (row.kind …。
        for (const auto& row : db.listClassroomActivities(courseId)) if (row.kind == "agent-task") {
            // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
            const auto task = Json::parse(row.payload, nullptr, false);
            // 【逐行批注】 判断条件 (task.is_object() && task.value("scopeId", "") == access.scopeId…，成立时执行对应分支。
            if (task.is_object() && task.value("scopeId", "") == access.scopeId &&
                // 【逐行批注】 调用 object 完成本行所需操作：task.value("event", Json::object()).value("lessonId", "") == lesson.a…。
                task.value("event", Json::object()).value("lessonId", "") == lesson.at("id"))
                // 【逐行批注】 按循环规则重复处理：for (const auto& item : displayedMessages(db, task)) {。
                for (const auto& item : displayedMessages(db, task)) {
                    // 【逐行批注】 计算右侧结果并写入 auto metadata，更新当前运行状态。
                    const auto metadata = task.value("teachingHelp", Json::object()).value(std::to_string(item.value("step", 0)), Json::object());
                    // 【逐行批注】 计算右侧结果并写入 bool partial，更新当前运行状态。
                    const bool partial = item.value("partial", false) || !item.value("displayComplete", false);
                    // 【逐行批注】 调用 rememberDemonstrations 完成本行所需操作：rememberDemonstrations(metadata, partial, {{"kind", "message"}, {"tas…。
                    rememberDemonstrations(metadata, partial, {{"kind", "message"}, {"taskId", task.at("id")}, {"step", item.at("step")}});
                    // 【逐行批注】 计算右侧结果并写入 auto& origin，更新当前运行状态。
                    const auto& origin = task.at("event");
                    // 【逐行批注】 计算右侧结果并写入 bool sameSection，更新当前运行状态。
                    const bool sameSection = origin.value("sectionId", "") == sectionId &&
                        // 【逐行批注】 调用 at 完成本行所需操作：origin.value("sectionVersion", section->at("version").get<int>()) == …。
                        origin.value("sectionVersion", section->at("version").get<int>()) == section->at("version");
                    // 【逐行批注】 计算右侧结果并写入 bool sameQuestion，更新当前运行状态。
                    const bool sameQuestion = metadata.value("questionIdentity", "") == teachingQuestionIdentity(courseId, section->at("question"));
                    // 【逐行批注】 计算右侧结果并写入 bool demonstrated，更新当前运行状态。
                    const bool demonstrated = matchesDemonstration(metadata, courseId, section->at("question"));
                    // 【逐行批注】 判断条件 (!sameSection && !sameQuestion && !demonstrated) continue;，成立时执行对应分支。
                    if (!sameSection && !sameQuestion && !demonstrated) continue;
                    // 【逐行批注】 判断条件 (metadata.contains("questionId") && metadata.at("questionId") !=…，成立时执行对应分支。
                    if (metadata.contains("questionId") && metadata.at("questionId") != questionId && !sameQuestion && !demonstrated) continue;
                    // 【逐行批注】 计算右侧结果并写入 auto declared，更新当前运行状态。
                    const auto declared = demonstrated ? "worked_example" : metadata.value("helpType", "unknown");
                    // 【逐行批注】 调用 remember 完成本行所需操作：remember({{"helpType", partial && declared != "explanation" && declar…。
                    remember({{"helpType", partial && declared != "explanation" && declared != "none" && declared != "clarification" ? "unknown" : declared},
                        // 【逐行批注】 执行当前声明或表达式：{"text", item.at("text")}, {"questionId", questionId}, {"taskId", task.at("id")},。
                        {"text", item.at("text")}, {"questionId", questionId}, {"taskId", task.at("id")},
                        // 【逐行批注】 执行当前声明或表达式：{"step", item.at("step")}, {"partial", partial}});。
                        {"step", item.at("step")}, {"partial", partial}});
                // 【逐行批注】 结束当前代码块。
                }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 value["hintHistory"]，更新当前运行状态。
        if (value.value("topicId", Json()).is_string() && value.at("topicId") != "") {
            // 只在实际提交时关联已展示的历史课时示范。事后打开讲解不会改写旧作答。
            const auto topics = relatedTopics(db, access, courseId, value.at("topicId"));
            const auto key = teachingQuestionEvidenceKey(courseId, section->at("question"));
            for (const auto& [otherCourse, otherTopic] : topics) {
                for (const auto& row : db.listClassroomActivities(otherCourse)) {
                    if (row.kind != "agent-lesson" || row.id == lesson.at("id")) continue;
                    const auto other = Json::parse(row.payload, nullptr, false);
                    if (!other.is_object() || other.value("id", "") != row.id || other.value("courseId", "") != otherCourse ||
                        other.value("scopeId", "") != access.scopeId || other.value("topicId", Json()) != otherTopic.topicId ||
                        other.value("status", "") != "ready") continue;
                    const auto seen = Json::parse(db.profileMeta("agent-section-exposure:" + row.id), nullptr, false);
                    if (!seen.is_object()) continue;
                    const auto details = Json::parse(db.profileMeta("agent-section-display:" + row.id), nullptr, false);
                    for (const auto& item : other.value("sections", Json::array())) {
                        if (!item.is_object() || item.value("kind", "") == "question" || !item.contains("version") ||
                            seen.value(item.value("id", ""), Json()) != item.at("version")) continue;
                        const auto examples = item.value("demonstratedQuestions", Json::array());
                        if (!examples.is_array()) continue;
                        const auto match = std::find_if(examples.begin(), examples.end(), [&](const Json& example) {
                            return example.is_object() && example.value("question", Json()).is_string() &&
                                teachingQuestionEvidenceKey(courseId, example) == key;
                        });
                        if (match == examples.end()) continue;
                        const auto detail = details.is_object() ? details.value(item.value("id", ""), Json::object()) : Json::object();
                        const bool complete = detail.value("version", Json()) == item.at("version") && detail.value("complete", false);
                        remember({{"helpType", complete ? "worked_example" : "unknown"}, {"partial", !complete},
                            {"text", "这道题在同一知识点的另一节课已有示范。"}, {"questionId", questionId},
                            {"source", "topic_demonstrated_question"}, {"sourceLessonId", row.id}, {"sectionId", item.at("id")},
                            {"sectionVersion", item.at("version")}});
                    }
                }
            }
        }
        value["hintHistory"] = hints; value["assisted"] = !hints.empty();
        // 【逐行批注】 计算右侧结果并写入 value["helpHistory"]，更新当前运行状态。
        value["helpHistory"] = help; value["relatedLearning"] = related; value["helpMode"] = answerHelpMode(help);
        // 【逐行批注】 计算右侧结果并写入 value["shownDemonstrations"]，更新当前运行状态。
        value["shownDemonstrations"] = shownDemonstrations;
        // 【逐行批注】 判断条件 (unassignedExample && value["helpMode"] != "after_example") valu…，成立时执行对应分支。
        if (unassignedExample && value["helpMode"] != "after_example") value["helpMode"] = "help_unknown";
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (type == "teaching_request") {，成立时执行对应分支。
    if (type == "teaching_request") {
        // 【逐行批注】 计算右侧结果并写入 auto request，更新当前运行状态。
        const auto request = text(event, "requestType");
        // 【逐行批注】 判断条件 (request != "hint" && request != "worked_example" && request != …，成立时执行对应分支。
        if (request != "hint" && request != "worked_example" && request != "rephrase" && request != "retry_question" && request != "revise_round" &&
            !isTeachingPreferenceIntent(request))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("教学请求类型无效");。
            throw std::invalid_argument("教学请求类型无效");
        // 【逐行批注】 判断条件 ((request == "hint" || request == "worked_example" || request ==…，成立时执行对应分支。
        if ((request == "hint" || request == "worked_example" || request == "retry_question") && !value.contains("questionId"))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("请先选择需要帮助或复练的题目");。
            throw std::invalid_argument("请先选择需要帮助或复练的题目");
        // 【逐行批注】 计算右侧结果并写入 value["requestType"]，更新当前运行状态。
        value["requestType"] = request; value["action"] = "teaching_request";
        if (request == "revise_round") value["roundRevision"] = event.at("roundRevision");
        // 在创建任务之前核对偏好版本；失败时同一事务不会留下输入或任务。
        if (isTeachingPreferenceIntent(request)) {
            checkedTeachingPreferenceIntent(lessonFor(db, access, text(event, "lessonId")), event);
            value["teachingPreferenceVersion"] = event.at("teachingPreferenceVersion");
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!db.insert(LearningInteraction{newId("student"), "chat-user", v…，成立时执行对应分支。
    if (!db.insert(LearningInteraction{newId("student"), "chat-user", value.dump(), now(),
        // 【逐行批注】 执行当前声明或表达式：courseId.empty() ? std::optional<std::string>{} : std::optional<std::string>{courseI…。
        courseId.empty() ? std::optional<std::string>{} : std::optional<std::string>{courseId}, event.value("conversationId", event.at("requestId").get<std::string>()), {}}))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("学生回答保存失败");。
        throw std::runtime_error("学生回答保存失败");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentLessonView，执行该函数声明的职责。
Json agentReviewLessons(Database& db, const AgentAccess& access, const Json& window) {
    const auto begin = window.at("startAt").get<std::string>();
    const auto end = window.at("endAt").get<std::string>();
    std::map<std::string, Json> byLesson;
    for (const auto& row : db.listInteractions()) {
        if (!row.courseId || row.createdAt < begin || row.createdAt >= end ||
            (row.kind != "lesson-visit" && row.kind != "chat-user")) continue;
        if (std::find(access.courseIds.begin(), access.courseIds.end(), *row.courseId) == access.courseIds.end()) continue;
        const auto payload = Json::parse(row.payload, nullptr, false);
        if (!payload.is_object() || payload.value("scopeId", Json()) != access.scopeId ||
            !payload.value("lessonId", Json()).is_string()) continue;
        const auto lessonId = payload.at("lessonId").get<std::string>();
        const auto type = payload.value("type", Json());
        if (lessonId.empty() || (row.kind == "chat-user" && type != "chat" &&
            (type != "question_answer" || payload.value("action", Json()) == "omitted"))) continue;
        auto item = byLesson.find(lessonId);
        if (item == byLesson.end()) {
            try {
                const auto lesson = lessonFor(db, access, lessonId);
                if (lesson.value("status", "") != "ready" || !lesson.value("entered", false) || lesson.at("courseId") != *row.courseId) continue;
                const auto course = db.getCourse(*row.courseId);
                if (!course || course->status != "active") continue;
                auto entry = link(lesson);
                entry["courseTitle"] = course->title; entry["topicId"] = lesson.value("topicId", Json());
                entry["visits"] = 0; entry["exchanges"] = 0; entry["lastActivityAt"] = row.createdAt;
                item = byLesson.emplace(lessonId, std::move(entry)).first;
            } catch (const std::invalid_argument&) { continue; }
        }
        auto& entry = item->second;
        if (entry.at("courseId") != *row.courseId) continue;
        auto& count = entry[row.kind == "lesson-visit" ? "visits" : "exchanges"];
        count = count.get<int>() + 1;
        if (row.createdAt > entry.at("lastActivityAt").get_ref<const std::string&>()) entry["lastActivityAt"] = row.createdAt;
    }
    Json results = Json::array();
    for (auto& [_, item] : byLesson) results.push_back(std::move(item));
    std::stable_sort(results.begin(), results.end(), [](const Json& left, const Json& right) {
        return left.at("lastActivityAt") > right.at("lastActivityAt");
    });
    return results;
}

Json agentTopicEvidence(Database& db, const AgentAccess& access, const std::string& lessonId) {
    const auto lesson = lessonFor(db, access, lessonId);
    const auto topic = lesson.value("topicId", Json());
    if (!topic.is_string() || topic.get_ref<const std::string&>().empty()) return {{"status", "unmapped"}};
    return topicEvidence(db, access, lesson, db.listClassroomActivities(lesson.at("courseId")), db.listInteractions());
}

Json agentTopicEvidenceForPreparation(Database& db, const AgentAccess& access,
                                      const std::string& courseId, const std::string& topicId) {
    if (courseId.empty() || topicId.empty()) return {{"status", "unmapped"}};
    // 备课还没有课时，先用课程大纲验证目标标识，再索引同一知识点的实际课时。
    Json outline;
    try { outline = agentOutline(db, access, courseId); }
    catch (const std::invalid_argument&) { return {{"status", "unmapped"}}; }
    for (const auto& stage : outline.at("courseStructure")) for (const auto& topic : stage.at("topics")) {
        if (topic.at("id") == topicId)
            return topicEvidence(db, access, {{"id", topicId}, {"courseId", courseId}, {"topicId", topicId}},
                                 db.listClassroomActivities(courseId), db.listInteractions());
    }
    return {{"status", "unmapped"}};
}

Json agentLessonView(Database& db, const AgentAccess& access, const std::string& lessonId) {
    // 【逐行批注】 调用 agentUpgradeLegacyLesson 完成本行所需操作：agentUpgradeLegacyLesson(db, access, lessonId);。
    agentUpgradeLegacyLesson(db, access, lessonId);
    // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
    const auto saved = lessonFor(db, access, lessonId); auto shown = publicLesson(saved);
    // 【逐行批注】 计算右侧结果并写入 auto sectionExposure，更新当前运行状态。
    const auto sectionExposure = Json::parse(db.profileMeta("agent-section-exposure:" + lessonId), nullptr, false);
    // 【逐行批注】 按循环规则重复处理：for (auto& section : shown["sections"])。
    for (auto& section : shown["sections"])
        // 【逐行批注】 计算右侧结果并写入 section["displayed"]，更新当前运行状态。
        section["displayed"] = sectionExposure.is_object() && sectionExposure.value(section.at("id").get<std::string>(), 0) == section.at("version");
    // 【逐行批注】 计算右侧结果并写入 auto interactions，更新当前运行状态。
    const auto interactions = db.listInteractions();
    // 【逐行批注】 计算右侧结果并写入 auto tasks，更新当前运行状态。
    const auto tasks = db.listClassroomActivities(saved.at("courseId"));
    shown["topicEvidence"] = topicEvidence(db, access, saved, tasks, interactions);
    // 历史版本已生成但未激活的题，从最后教学任务的成功动作恢复展示焦点；不改正文或评价记录。
    // 【逐行批注】 判断条件 (!shown["teachingFocus"].contains("responseSectionId")) for (con…，成立时执行对应分支。
    if (!shown["teachingFocus"].contains("responseSectionId")) for (const auto& row : tasks) {
        // 【逐行批注】 判断条件 (row.kind != "agent-task" || row.id != saved.value("teachingTask…，成立时执行对应分支。
        if (row.kind != "agent-task" || row.id != saved.value("teachingTaskId", "")) continue;
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = Json::parse(row.payload, nullptr, false);
        // 【逐行批注】 判断条件 (!task.is_object() || task.value("scopeId", "") != access.scopeI…，成立时执行对应分支。
        if (!task.is_object() || task.value("scopeId", "") != access.scopeId) continue;
        // 【逐行批注】 计算右侧结果并写入 auto type，更新当前运行状态。
        const auto type = task.value("event", Json::object()).value("type", "");
        // 【逐行批注】 判断条件 (type != "question_answer" && type != "chat") continue;，成立时执行对应分支。
        if (type != "question_answer" && type != "chat") continue;
        // 【逐行批注】 计算右侧结果并写入 auto completed，更新当前运行状态。
        const auto completed = task.value("completedActions", Json::object());
        // 【逐行批注】 判断条件 (!completed.is_object()) continue;，成立时执行对应分支。
        if (!completed.is_object()) continue;
        // 【逐行批注】 按循环规则重复处理：for (const auto& event : task.value("events", Json::array())) {。
        for (const auto& event : task.value("events", Json::array())) {
            // 【逐行批注】 判断条件 (!event.is_object() || event.value("type", "") != "action") cont…，成立时执行对应分支。
            if (!event.is_object() || event.value("type", "") != "action") continue;
            // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
            const auto id = event.value("actionId", ""); if (!completed.contains(id)) continue;
            // 【逐行批注】 计算右侧结果并写入 auto action，更新当前运行状态。
            const auto action = Json::parse(completed[id].value("encoded", ""), nullptr, false);
            // 【逐行批注】 判断条件 (!action.is_object() || !action.value("args", Json()).is_object(…，成立时执行对应分支。
            if (!action.is_object() || !action.value("args", Json()).is_object()) continue;
            // 【逐行批注】 计算右侧结果并写入 auto& args，更新当前运行状态。
            const auto& args = action.at("args");
            // 【逐行批注】 判断条件 (args.value("lessonId", "") != lessonId) continue;，成立时执行对应分支。
            if (args.value("lessonId", "") != lessonId) continue;
            // 【逐行批注】 执行当前声明或表达式：std::string sectionId;。
            std::string sectionId;
            // 【逐行批注】 判断条件 (action.value("tool", "") == "append_section" && args.value("act…，成立时执行对应分支。
            if (action.value("tool", "") == "append_section" && args.value("activate", true))
                // 【逐行批注】 计算右侧结果并写入 sectionId，更新当前运行状态。
                sectionId = completed[id].value("result", Json::object()).value("sectionId", "");
            // 【逐行批注】 在前一条件不成立时继续判断：(action.value("tool", "") == "select_teaching_focus") {。
            else if (action.value("tool", "") == "select_teaching_focus") {
                // 【逐行批注】 计算右侧结果并写入 auto preferred，更新当前运行状态。
                const auto preferred = responseView(saved, task) == "practice" ? "practiceSectionId" : "interactionSectionId";
                // 【逐行批注】 计算右侧结果并写入 sectionId，更新当前运行状态。
                sectionId = args.value(preferred, args.value(std::string(preferred) == "practiceSectionId" ? "interactionSectionId" : "practiceSectionId", ""));
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 按循环规则重复处理：for (const auto& section : saved.at("sections")) if (section.at("id") == s…。
            for (const auto& section : saved.at("sections")) if (section.at("id") == sectionId)
                // 【逐行批注】 调用 selectResponseQuestion 完成本行所需操作：selectResponseQuestion(shown["teachingFocus"], saved, task, section);。
                selectResponseQuestion(shown["teachingFocus"], saved, task, section);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 shown["dialog"]，更新当前运行状态。
    shown["dialog"] = Json::array();
    // 【逐行批注】 计算右侧结果并写入 auto appendReplies，更新当前运行状态。
    const auto appendReplies = [&](Json& dialog, const Json& input, const Json& task) {
        // 【逐行批注】 判断条件 (task.value("scopeId", "") != access.scopeId || task.value("requ…，成立时执行对应分支。
        if (task.value("scopeId", "") != access.scopeId || task.value("requestId", "") != input.value("requestId", "")) return;
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : displayedMessages(db, task)) dialog.push_back(item…。
        for (const auto& item : displayedMessages(db, task)) dialog.push_back(item);
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 按循环规则重复处理：for (const auto& interaction : interactions) if (interaction.kind == "chat…。
    for (const auto& interaction : interactions) if (interaction.kind == "chat-user") {
        // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
        const auto input = Json::parse(interaction.payload, nullptr, false);
        // 【逐行批注】 判断条件 (!input.is_object() || input.value("scopeId", "") != access.scop…，成立时执行对应分支。
        if (!input.is_object() || input.value("scopeId", "") != access.scopeId || input.value("lessonId", "") != lessonId || input.contains("sectionId")) continue;
        // 【逐行批注】 执行当前声明或表达式：shown["dialog"].push_back({{"role", "user"}, {"text", input.value("text", "")}, {"at…。
        shown["dialog"].push_back({{"role", "user"}, {"text", input.value("text", "")}, {"at", interaction.createdAt}});
        // 【逐行批注】 按循环规则重复处理：for (const auto& row : tasks) if (row.kind == "agent-task") {。
        for (const auto& row : tasks) if (row.kind == "agent-task") {
            // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
            const auto task = Json::parse(row.payload, nullptr, false);
            // 【逐行批注】 判断条件 (!task.is_object() || task.value("requestId", "") != input.value…，成立时执行对应分支。
            if (!task.is_object() || task.value("requestId", "") != input.value("requestId", "")) continue;
            // 【逐行批注】 计算右侧结果并写入 appendReplies(shown["dialog"], input, task);…，更新当前运行状态。
            appendReplies(shown["dialog"], input, task); shown["taskId"] = task.at("id"); shown["taskStatus"] = task.at("status");
            // 【逐行批注】 计算右侧结果并写入 auto exposure，更新当前运行状态。
            const auto exposure = Json::parse(db.profileMeta("agent-exposure:" + task.at("id").get<std::string>()), nullptr, false);
            // 【逐行批注】 计算右侧结果并写入 shown["afterSeq"]，更新当前运行状态。
            shown["afterSeq"] = exposure.is_object() ? exposure.value("seq", 0) : 0;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (auto& section : shown["sections"]) {。
    for (auto& section : shown["sections"]) {
        // 【逐行批注】 计算右侧结果并写入 section["dialog"]，更新当前运行状态。
        section["dialog"] = Json::array();
        // 【逐行批注】 计算右侧结果并写入 section["legacyDialog"]，更新当前运行状态。
        section["legacyDialog"] = Json::array();
        // 【逐行批注】 按循环规则重复处理：for (const auto& source : saved.at("sections")) if (source.at("id") == sec…。
        for (const auto& source : saved.at("sections")) if (source.at("id") == section.at("id"))
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : source.value("legacyDialog", Json::array())) {。
            for (const auto& item : source.value("legacyDialog", Json::array())) {
                // 【逐行批注】 执行当前声明或表达式：section["legacyDialog"].push_back(item); section["dialog"].push_back(item);。
                section["legacyDialog"].push_back(item); section["dialog"].push_back(item);
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 按循环规则重复处理：for (const auto& interaction : interactions) if (interaction.kind == "chat…。
        for (const auto& interaction : interactions) if (interaction.kind == "chat-user") {
            // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
            const auto input = Json::parse(interaction.payload, nullptr, false);
            // 【逐行批注】 判断条件 (!input.is_object() || input.value("scopeId", "") != access.scop…，成立时执行对应分支。
            if (!input.is_object() || input.value("scopeId", "") != access.scopeId || input.value("lessonId", "") != lessonId ||
                // 【逐行批注】 执行当前声明或表达式：input.value("sectionId", "") != section.at("id")) continue;。
                input.value("sectionId", "") != section.at("id")) continue;
            // 【逐行批注】 执行当前声明或表达式：section["dialog"].push_back({{"role", "user"}, {"text", input.value("text", "")}, {"…。
            section["dialog"].push_back({{"role", "user"}, {"text", input.value("text", "")}, {"at", interaction.createdAt}});
            // 【逐行批注】 按循环规则重复处理：for (const auto& row : tasks) if (row.kind == "agent-task") {。
            for (const auto& row : tasks) if (row.kind == "agent-task") {
                // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
                const auto task = Json::parse(row.payload, nullptr, false);
                // 【逐行批注】 判断条件 (!task.is_object() || task.value("scopeId", "") != access.scopeI…，成立时执行对应分支。
                if (!task.is_object() || task.value("scopeId", "") != access.scopeId || task.value("requestId", "") != input.value("requestId", "")) continue;
                // 【逐行批注】 计算右侧结果并写入 section["taskId"]，更新当前运行状态。
                section["taskId"] = task.at("id"); section["taskStatus"] = task.at("status");
                // 【逐行批注】 调用 appendReplies 完成本行所需操作：appendReplies(section["dialog"], input, task);。
                appendReplies(section["dialog"], input, task);
                // 【逐行批注】 计算右侧结果并写入 auto exposure，更新当前运行状态。
                const auto exposure = Json::parse(db.profileMeta("agent-exposure:" + task.at("id").get<std::string>()), nullptr, false);
                // 【逐行批注】 计算右侧结果并写入 section["afterSeq"]，更新当前运行状态。
                section["afterSeq"] = exposure.is_object() ? exposure.value("seq", 0) : 0;
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 shown["teachingDialog"]，更新当前运行状态。
    shown["teachingDialog"] = Json::array(); shown["evaluations"] = Json::array(); shown["historicalAssessments"] = Json::array();
    // 【逐行批注】 按循环规则重复处理：for (const auto& row : tasks) if (row.kind == "agent-task") {。
    for (const auto& row : tasks) if (row.kind == "agent-task") {
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = Json::parse(row.payload, nullptr, false);
        // 【逐行批注】 判断条件 (!task.is_object() || task.value("scopeId", "") != access.scopeI…，成立时执行对应分支。
        if (!task.is_object() || task.value("scopeId", "") != access.scopeId ||
            // 【逐行批注】 调用 object 完成本行所需操作：task.value("event", Json::object()).value("lessonId", "") != lessonId…。
            task.value("event", Json::object()).value("lessonId", "") != lessonId) continue;
        // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
        const auto event = task.at("event");
        // 【逐行批注】 按循环规则重复处理：for (const auto& interaction : interactions) if (interaction.kind == "chat…。
        for (const auto& interaction : interactions) if (interaction.kind == "chat-user") {
            // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
            const auto input = Json::parse(interaction.payload, nullptr, false);
            // 【逐行批注】 判断条件 (input.is_object() && input.value("scopeId", "") == access.scope…，成立时执行对应分支。
            if (input.is_object() && input.value("scopeId", "") == access.scopeId &&
                // 【逐行批注】 执行当前声明或表达式：input.value("lessonId", "") == lessonId && input.value("requestId", "") == task.at("…。
                input.value("lessonId", "") == lessonId && input.value("requestId", "") == task.at("requestId"))
                // 【逐行批注】 执行当前声明或表达式：shown["teachingDialog"].push_back({{"role", "user"}, {"text", input.value("text", ""…。
                shown["teachingDialog"].push_back({{"role", "user"}, {"text", input.value("text", "")},
                    // 【逐行批注】 执行当前声明或表达式：{"sectionId", input.value("sectionId", "")}, {"taskId", task.at("id")}, {"at", inter…。
                    {"sectionId", input.value("sectionId", "")}, {"taskId", task.at("id")}, {"at", interaction.createdAt}});
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for (auto item : displayedMessages(db, task)) {。
        for (auto item : displayedMessages(db, task)) {
            // 【逐行批注】 计算右侧结果并写入 item["taskId"]，更新当前运行状态。
            item["taskId"] = task.at("id"); item["sectionId"] = event.value("sectionId", "");
            // 【逐行批注】 调用 move 完成本行所需操作：shown["teachingDialog"].push_back(std::move(item));。
            shown["teachingDialog"].push_back(std::move(item));
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& row : interactions) if (row.kind == "practice" && row.cou…。
    for (const auto& row : interactions) if (row.kind == "practice" && row.courseId == saved.at("courseId").get<std::string>()) {
        // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
        const auto value = Json::parse(row.payload, nullptr, false);
        // 【逐行批注】 判断条件 (!value.is_object() || !value.contains("interactionId") || value…，成立时执行对应分支。
        if (!value.is_object() || !value.contains("interactionId") || value.value("source", "") != "ai") continue;
        // 【逐行批注】 按循环规则重复处理：for (const auto& inputRow : interactions) if (inputRow.id == value.at("int…。
        for (const auto& inputRow : interactions) if (inputRow.id == value.at("interactionId") && inputRow.kind == "chat-user") {
            // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
            const auto input = Json::parse(inputRow.payload, nullptr, false);
            // 【逐行批注】 判断条件 (!input.is_object() || input.value("scopeId", "") != access.scop…，成立时执行对应分支。
            if (!input.is_object() || input.value("scopeId", "") != access.scopeId || input.value("lessonId", "") != lessonId) continue;
            // 【逐行批注】 计算右侧结果并写入 Json evaluation，更新当前运行状态。
            shown["evaluations"].push_back(publishedEvaluation(row, value, inputRow, input));
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (saved.contains("phaseIndex") && saved.contains("topicIndex")) {，成立时执行对应分支。
    if (saved.contains("phaseIndex") && saved.contains("topicIndex")) {
        // 【逐行批注】 计算右侧结果并写入 auto preparedTask，更新当前运行状态。
        const auto preparedTask = lessonId.find("legacy-prepared-") == 0 ? lessonId.substr(16) : std::string();
        // 【逐行批注】 按循环规则重复处理：for (const auto& row : interactions) if (row.kind == "quiz" && row.courseI…。
        for (const auto& row : interactions) if (row.kind == "quiz" && row.courseId == saved.at("courseId").get<std::string>()) {
            // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
            const auto value = Json::parse(row.payload, nullptr, false);
            // 【逐行批注】 判断条件 (!value.is_object() || value.value("phaseIndex", 0) != saved.at(…，成立时执行对应分支。
            if (!value.is_object() || value.value("phaseIndex", 0) != saved.at("phaseIndex") || value.value("topicIndex", 0) != saved.at("topicIndex") ||
                // 【逐行批注】 执行当前声明或表达式：value.value("lessonTaskId", "") != preparedTask) continue;。
                value.value("lessonTaskId", "") != preparedTask) continue;
            // 【逐行批注】 计算右侧结果并写入 Json record，更新当前运行状态。
            Json record = {{"id", row.id}, {"kind", "quiz"}, {"at", row.createdAt}, {"answers", Json::array()}};
            // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"score", "total"}) if (value.contains(field) && …。
            for (const auto* field : {"score", "total"}) if (value.contains(field) && (value.at(field).is_number() || value.at(field).is_null())) record[field] = value.at(field);
            // 【逐行批注】 按循环规则重复处理：for (const auto& answer : value.value("answers", Json::array()))。
            for (const auto& answer : value.value("answers", Json::array()))
                // 【逐行批注】 调用 Json 完成本行所需操作：record["answers"].push_back(answer.is_string() || answer.is_number_in…。
                record["answers"].push_back(answer.is_string() || answer.is_number_integer() || answer.is_null() ? answer : Json());
            // 【逐行批注】 调用 move 完成本行所需操作：shown["historicalAssessments"].push_back(std::move(record));。
            shown["historicalAssessments"].push_back(std::move(record));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for (const auto& row : tasks) if (row.kind == "practice" || row.kind == "d…。
        for (const auto& row : tasks) if (row.kind == "practice" || row.kind == "diagnostic" || row.kind == "interaction" || row.kind == "quiz") {
            // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
            const auto value = Json::parse(row.payload, nullptr, false);
            // 【逐行批注】 判断条件 (!value.is_object() || row.phaseIndex != saved.at("phaseIndex") …，成立时执行对应分支。
            if (!value.is_object() || row.phaseIndex != saved.at("phaseIndex") || row.topicIndex != saved.at("topicIndex") ||
                // 【逐行批注】 执行当前声明或表达式：value.value("lessonTaskId", "") != preparedTask || value.value("status", "") != "ans…。
                value.value("lessonTaskId", "") != preparedTask || value.value("status", "") != "answered" || !value.contains("answer")) continue;
            // 【逐行批注】 计算右侧结果并写入 auto identity，更新当前运行状态。
            const auto identity = value.value("questionId", value.value("questionSnapshot", Json()).is_object() ? questionIdentity(saved.at("courseId"), value.at("questionSnapshot")) : "");
            // 【逐行批注】 计算右侧结果并写入 bool matched，更新当前运行状态。
            bool matched = false;
            // 【逐行批注】 按循环规则重复处理：for (const auto& section : saved.at("sections"))。
            for (const auto& section : saved.at("sections"))
                // 【逐行批注】 判断条件 (section.value("questionId", "") == identity && !identity.empty(…，成立时执行对应分支。
                if (section.value("questionId", "") == identity && !identity.empty()) matched = true;
            // 【逐行批注】 判断条件 (!matched) continue;，成立时执行对应分支。
            if (!matched) continue;
            // 【逐行批注】 计算右侧结果并写入 Json record，更新当前运行状态。
            Json record = {{"id", row.id}, {"kind", row.kind}, {"at", row.updatedAt}, {"questionId", identity},
                // 【逐行批注】 调用 Json 完成本行所需操作：{"response", value.at("answer").is_string() ? value.at("answer") : Js…。
                {"response", value.at("answer").is_string() ? value.at("answer") : Json(value.at("answer").dump())}};
            // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"feedback", "credible", "correct", "assisted", "…。
            for (const auto* field : {"feedback", "credible", "correct", "assisted", "score"})
                // 【逐行批注】 判断条件 (value.contains(field) && (value.at(field).is_string() || value.…，成立时执行对应分支。
                if (value.contains(field) && (value.at(field).is_string() || value.at(field).is_boolean() || value.at(field).is_number() || value.at(field).is_null())) record[field] = value.at(field);
            // 【逐行批注】 调用 move 完成本行所需操作：shown["historicalAssessments"].push_back(std::move(record));。
            shown["historicalAssessments"].push_back(std::move(record));
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 聊天课堂重放真实任务顺序。仅公开教师文字和已提交板块的身份，不能带出工具参数、答案或评分标准。
    // teachingDialog 继续保留原来的“实际已展示”协议；timeline 可读取未读回复，读取本身不修改已读证据。
    // 【逐行批注】 计算右侧结果并写入 shown["timeline"]，更新当前运行状态。
    shown["timeline"] = Json::array();
    // 【逐行批注】 计算右侧结果并写入 auto taskView，更新当前运行状态。
    const auto taskView = [&](const Json& task) { return lessonTaskView(saved, task); };
    // 【逐行批注】 按循环规则重复处理：for (auto& section : shown["sections"]) {。
    for (auto& section : shown["sections"]) {
        // 【逐行批注】 计算右侧结果并写入 auto location，更新当前运行状态。
        const auto location = agentSectionLocation(saved, section);
        // 【逐行批注】 计算右侧结果并写入 section["view"]，更新当前运行状态。
        section["view"] = location.at("view"); section["href"] = location.at("href");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& row : tasks) if (row.kind == "agent-task") {。
    for (const auto& row : tasks) if (row.kind == "agent-task") {
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = Json::parse(row.payload, nullptr, false);
        // 【逐行批注】 判断条件 (!task.is_object() || task.value("scopeId", "") != access.scopeI…，成立时执行对应分支。
        if (!task.is_object() || task.value("scopeId", "") != access.scopeId ||
            // 【逐行批注】 调用 object 完成本行所需操作：task.value("event", Json::object()).value("lessonId", "") != lessonId…。
            task.value("event", Json::object()).value("lessonId", "") != lessonId) continue;
        // 【逐行批注】 计算右侧结果并写入 auto taskId，更新当前运行状态。
        const auto taskId = task.at("id");
        // 【逐行批注】 计算右侧结果并写入 auto view，更新当前运行状态。
        const auto view = taskView(task);
        // 【逐行批注】 按循环规则重复处理：for (const auto& interaction : interactions) if (interaction.kind == "chat…。
        for (const auto& interaction : interactions) if (interaction.kind == "chat-user") {
            // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
            const auto input = Json::parse(interaction.payload, nullptr, false);
            // 【逐行批注】 判断条件 (!input.is_object() || input.value("scopeId", "") != access.scop…，成立时执行对应分支。
            if (!input.is_object() || input.value("scopeId", "") != access.scopeId || input.value("lessonId", "") != lessonId ||
                // 【逐行批注】 执行当前声明或表达式：input.value("requestId", "") != task.at("requestId")) continue;。
                input.value("requestId", "") != task.at("requestId")) continue;
            // 【逐行批注】 执行当前声明或表达式：shown["timeline"].push_back({{"type", "message"}, {"id", interaction.id}, {"role", "…。
            shown["timeline"].push_back({{"type", "message"}, {"id", interaction.id}, {"role", "user"},
                // 【逐行批注】 执行当前声明或表达式：{"text", input.value("text", "")}, {"view", view}, {"sectionId", input.value("sectio…。
                {"text", input.value("text", "")}, {"view", view}, {"sectionId", input.value("sectionId", "")}, {"taskId", taskId}, {"at", interaction.createdAt}});
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 auto valid，更新当前运行状态。
        const auto valid = task.value("validSteps", Json::array());
        // 【逐行批注】 计算右侧结果并写入 auto completed，更新当前运行状态。
        const auto completed = task.value("completedActions", Json::object());
        // 【逐行批注】 执行当前声明或表达式：std::map<int, std::size_t> messages;。
        std::map<int, std::size_t> messages;
        std::map<int, std::map<std::string, std::string>> bodies;
        // 【逐行批注】 执行当前声明或表达式：std::set<std::string> evaluations;。
        std::set<std::string> evaluations;
        // 【逐行批注】 按循环规则重复处理：for (const auto& event : lessonTaskEvents(db, task)) {。
        for (const auto& event : lessonTaskEvents(db, task)) {
            const auto step = event.value("step", 0);
            if (event.value("type", "") == "delta" && event.value("field", "") == "body") {
                auto& body = bodies[step][event.value("streamId", "body")];
                body = (event.value("replace", false) ? std::string() : body) + event.value("text", "");
            }
            if (event.value("type", "") == "stream_status" && event.value("state", "") == "confirmed") {
                const auto revisions = event.value("revisions", Json::object());
                if (messages.count(step) && revisions.contains("message")) shown["timeline"][messages.at(step)]["revisions"] = revisions.at("message");
                if (revisions.contains("body")) for (const auto& body : bodies[step])
                    for (auto& section : shown["sections"]) if (section.value("body", "") == body.second)
                        section["streamRevisions"] = revisions.at("body");
            }

            // 【逐行批注】 判断条件 (event.value("type", "") == "delta" && event.value("field", "") …，成立时执行对应分支。
            if (event.value("type", "") == "delta" && event.value("field", "") == "message") {
                // 【逐行批注】 计算右侧结果并写入 auto step，更新当前运行状态。
                const auto step = event.value("step", 0);
                // 草稿由实时订阅恢复，正式时间线只合并成功校验的轮次。
                if (std::find(valid.begin(), valid.end(), step) == valid.end() &&
                    (task.contains("validSteps") || event.value("provisional", false))) continue;
                // 【逐行批注】 判断条件 (!messages.count(step)) {，成立时执行对应分支。
                if (!messages.count(step)) {
                    // 【逐行批注】 计算右侧结果并写入 messages[step]，更新当前运行状态。
                    messages[step] = shown["timeline"].size();
                    // 【逐行批注】 调用 to_string 完成本行所需操作：shown["timeline"].push_back({{"type", "message"}, {"id", taskId.get<s…。
                    shown["timeline"].push_back({{"type", "message"}, {"id", taskId.get<std::string>() + ":step:" + std::to_string(step)},
                        // 【逐行批注】 调用 array 完成本行所需操作：{"role", "assistant"}, {"text", ""}, {"view", view}, {"taskId", taskI…。
                        {"role", "assistant"}, {"text", ""}, {"view", view}, {"taskId", taskId}, {"step", step}, {"sequences", Json::array()},
                        // 【逐行批注】 调用 find 完成本行所需操作：{"partial", std::find(valid.begin(), valid.end(), step) == valid.end(…。
                        {"partial", std::find(valid.begin(), valid.end(), step) == valid.end()}, {"at", task.value("createdAt", row.updatedAt)}});
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 auto& message，更新当前运行状态。
                auto& message = shown["timeline"][messages.at(step)];
                // 【逐行批注】 计算右侧结果并写入 message["text"]，更新当前运行状态。
                message["text"] = (event.value("replace", false) ? std::string() : message.at("text").get<std::string>()) + event.value("text", "");
                // 【逐行批注】 执行当前声明或表达式：message["sequences"].push_back(event.at("seq"));。
                message["sequences"].push_back(event.at("seq"));
            // 【逐行批注】 执行当前声明或表达式：} else if (event.value("type", "") == "action" && completed.is_object()) {。
            } else if (event.value("type", "") == "action" && completed.is_object()) {
                // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
                const auto id = event.value("actionId", ""); if (!completed.contains(id)) continue;
                // 【逐行批注】 计算右侧结果并写入 auto action，更新当前运行状态。
                const auto action = Json::parse(completed.at(id).value("encoded", ""), nullptr, false);
                // 【逐行批注】 判断条件 (!action.is_object() || !action.value("args", Json()).is_object(…，成立时执行对应分支。
                if (!action.is_object() || !action.value("args", Json()).is_object()) continue;
                // 【逐行批注】 计算右侧结果并写入 auto tool，更新当前运行状态。
                const auto tool = action.value("tool", "");
                // 【逐行批注】 判断条件 (tool == "evaluate_answer") for (const auto& evaluation : shown[…，成立时执行对应分支。
                if (tool == "evaluate_answer") for (const auto& evaluation : shown["evaluations"])
                    // 【逐行批注】 判断条件 (evaluation.value("interactionId", "") == action.at("args").valu…，成立时执行对应分支。
                    if (evaluation.value("interactionId", "") == action.at("args").value("interactionId", "")) {
                        // 【逐行批注】 执行当前声明或表达式：shown["timeline"].push_back({{"type", "evaluation"}, {"id", evaluation.at("id")}, {"…。
                        shown["timeline"].push_back({{"type", "evaluation"}, {"id", evaluation.at("id")}, {"view", view}, {"taskId", taskId}});
                        // 【逐行批注】 执行当前声明或表达式：evaluations.insert(evaluation.at("id").get<std::string>());。
                        evaluations.insert(evaluation.at("id").get<std::string>());
                    // 【逐行批注】 结束当前代码块。
                    }
                // 【逐行批注】 判断条件 (action.at("args").value("lessonId", "") != lessonId) continue;，成立时执行对应分支。
                if (action.at("args").value("lessonId", "") != lessonId) continue;
                // 【逐行批注】 判断条件 (tool == "append_section") {，成立时执行对应分支。
                if (tool == "append_section") {
                    // 【逐行批注】 计算右侧结果并写入 auto sectionId，更新当前运行状态。
                    const auto sectionId = completed.at(id).value("result", Json::object()).value("sectionId", "");
                    // 【逐行批注】 按循环规则重复处理：for (auto& section : shown["sections"]) if (section.at("id") == sectionId)…。
                    for (auto& section : shown["sections"]) if (section.at("id") == sectionId) {
                        // 老回执没有位置字段时，也按原任务重建同一地址，不迁移记录。
                        // 【逐行批注】 计算右侧结果并写入 auto location，更新当前运行状态。
                        const auto location = agentSectionLocation(saved, section, task);
                        // 【逐行批注】 计算右侧结果并写入 section["view"]，更新当前运行状态。
                        section["view"] = location.at("view"); section["href"] = location.at("href");
                    // 【逐行批注】 结束当前代码块。
                    }
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> identities;。
                std::vector<std::string> identities;
                // 【逐行批注】 判断条件 (tool == "append_section" && action.at("args").value("activate",…，成立时执行对应分支。
                if (tool == "append_section" && action.at("args").value("activate", true))
                    // 【逐行批注】 调用 object 完成本行所需操作：identities.push_back(completed.at(id).value("result", Json::object())…。
                    identities.push_back(completed.at(id).value("result", Json::object()).value("sectionId", ""));
                // 【逐行批注】 在前一条件不成立时继续判断：(tool == "select_teaching_focus") for (const auto* key : {"secti…。
                else if (tool == "select_teaching_focus") for (const auto* key : {"sectionId", "interactionSectionId", "practiceSectionId"})
                    // 【逐行批注】 判断条件 (action.at("args").contains(key)) identities.push_back(action.at…，成立时执行对应分支。
                    if (action.at("args").contains(key)) identities.push_back(action.at("args").value(key, ""));
                // 【逐行批注】 按循环规则重复处理：for (const auto& identity : identities) for (auto& section : shown["sectio…。
                for (const auto& identity : identities) for (auto& section : shown["sections"]) if (section.at("id") == identity) {
                    // 【逐行批注】 执行当前声明或表达式：shown["timeline"].push_back({{"type", "section"}, {"id", taskId.get<std::string>() +…。
                    shown["timeline"].push_back({{"type", "section"}, {"id", taskId.get<std::string>() + ":" + id + ":" + identity},
                        // 【逐行批注】 执行当前声明或表达式：{"sectionId", identity}, {"view", section.at("view")}, {"href", section.at("href")},…。
                        {"sectionId", identity}, {"view", section.at("view")}, {"href", section.at("href")}, {"taskId", taskId}});
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for (const auto& evaluation : shown["evaluations"]) if (evaluation.value("…。
        for (const auto& evaluation : shown["evaluations"]) if (evaluation.value("requestId", "") == task.at("requestId") &&
            // 【逐行批注】 执行当前声明或表达式：!evaluations.count(evaluation.at("id").get<std::string>()))。
            !evaluations.count(evaluation.at("id").get<std::string>()))
            // 【逐行批注】 执行当前声明或表达式：shown["timeline"].push_back({{"type", "evaluation"}, {"id", evaluation.at("id")}, {"…。
            shown["timeline"].push_back({{"type", "evaluation"}, {"id", evaluation.at("id")}, {"view", view}, {"taskId", taskId}});
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (shown.contains("teachingTaskId")) {，成立时执行对应分支。
    if (shown.contains("teachingTaskId")) {
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = agentView(db, access, shown.at("teachingTaskId"));
        // 【逐行批注】 计算右侧结果并写入 shown["teachingTaskStatus"]，更新当前运行状态。
        shown["teachingTaskStatus"] = task.at("status");
        // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
        const auto current = db.getClassroomActivity(shown.at("teachingTaskId"));
        // 【逐行批注】 计算右侧结果并写入 shown["teachingTaskView"]，更新当前运行状态。
        shown["teachingTaskView"] = current ? taskView(Json::parse(current->payload)) : task.value("view", "learn");
        // 【逐行批注】 计算右侧结果并写入 auto exposure，更新当前运行状态。
        const auto exposure = Json::parse(db.profileMeta("agent-exposure:" + task.at("id").get<std::string>()), nullptr, false);
        // 【逐行批注】 计算右侧结果并写入 shown["teachingAfterSeq"]，更新当前运行状态。
        shown["teachingAfterSeq"] = exposure.is_object() ? exposure.value("seq", 0) : 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return shown;。
    // 投影复用已授权、已过滤的公开课时，不增加写入或模型请求。
    shown["learningState"] = learningStateProjection(shown);
    const auto stateRow = db.getClassroomActivity("learning-state:" + lessonId);
    if (stateRow && stateRow->kind == "agent-learning-state" && stateRow->courseId == saved.at("courseId")) {
        const auto assessment = Json::parse(stateRow->payload, nullptr, false);
        if (assessment.is_object() && assessment.value("scopeId", "") == access.scopeId &&
            assessment.value("lessonId", "") == lessonId && assessment.value("courseId", "") == saved.at("courseId")) {
            // 新作答、评分或题目改变会让旧依据失效；读取不能自行重算成绩或猜测新掌握状态。
            if (assessment.value("basisDigest", "") == libraryDigest(learningStateBasis(shown).dump())) {
                try {
                    Json proposal;
                    for (const auto* field : {"status", "reason", "uncertainty", "evidenceIds", "nextAction"})
                        proposal[field] = assessment.at(field);
                    auto checked = checkedLearningMastery(shown, proposal);
                    checked.erase("nextAction");
                    for (const auto* field : {"model", "at", "version"}) if (assessment.contains(field)) checked[field] = assessment.at(field);
                    shown["learningState"]["mastery"] = std::move(checked);
                    shown["learningState"]["nextTeachingAction"] = {{"type", assessment.at("nextAction")}, {"executed", false}};
                } catch (const std::exception&) {
                    shown["learningState"]["mastery"]["reason"] = "保存的理解判断未通过当前证据校验，需要重新核对。";
                }
            } else {
                shown["learningState"]["mastery"]["reason"] = "学习证据已更新，原理解判断需重新核对。";
                shown["learningState"]["mastery"]["stale"] = true;
            }
        }
    }
    return shown;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 agentEnterLesson，执行该函数声明的职责。
Json agentEnterLesson(Database& db, const AgentAccess& access, const Json& body) {
    // 【逐行批注】 执行当前声明或表达式：Json result;。
    Json result;
    // 【逐行批注】 执行当前声明或表达式：db.transaction([&] {。
    db.transaction([&] {
        // 【逐行批注】 计算右侧结果并写入 lesson，更新当前运行状态。
        auto lesson = lessonFor(db, access, text(body, "lessonId"));
        // 【逐行批注】 判断条件 (lesson.at("status") != "ready") throw std::invalid_argument("请等…，成立时执行对应分支。
        if (lesson.at("status") != "ready") throw std::invalid_argument("请等待真实 AI 完整备课后进入");
        // 【逐行批注】 调用 rememberCurrentCourse 完成本行所需操作：rememberCurrentCourse(db, lesson.at("courseId"));。
        rememberCurrentCourse(db, lesson.at("courseId"));
        // 【逐行批注】 判断条件 (lesson.value("entered", false)) { result = link(lesson); return…，成立时执行对应分支。
        const auto instant = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &instant);
#else
        localtime_r(&instant, &local);
#endif
        std::ostringstream date;
        date << std::put_time(&local, "%Y-%m-%d");
        const auto visit = [&] {
            if (lesson.value("lastVisitDay", "") == date.str()) return false;
            if (!db.insert(LearningInteraction{newId("lesson-visit"), "lesson-visit", Json{{"scopeId", access.scopeId},
                {"lessonId", lesson.at("id")}}.dump(), now(), lesson.at("courseId").get<std::string>(), {}, {}}))
                throw std::runtime_error("课时访问记录保存失败");
            lesson["lastVisitDay"] = date.str();
            return true;
        };
        if (lesson.value("entered", false)) {
            if (visit()) saveLesson(db, lesson);
            result = link(lesson);
            return;
        }
        // 【逐行批注】 判断条件 (!lesson.value("entered", false) && lesson.at("sourceLearningVer…，成立时执行对应分支。
        if (lesson.contains("scheduledReviewBasisStatus")) {
            const auto review = agentScheduledReview(db, access, lesson.at("courseId"), lesson.at("scheduledReviewId"));
            if (review.empty() || !review.at("available").get<bool>() ||
                review.at("basisStatus") != lesson.at("scheduledReviewBasisStatus") ||
                review.value("basisRevision", "") != lesson.at("scheduledReviewBasisRevision"))
                throw std::invalid_argument("复习依据已有更新，请从待复习清单重新进入");
        } else if (lesson.at("sourceLearningVersion") != db.learningRevision())
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("学习情况已有更新，请让 AI 重新准备后再进入");。
            throw std::invalid_argument("学习情况已有更新，请让 AI 重新准备后再进入");
        // 【逐行批注】 计算右侧结果并写入 lesson["entered"]，更新当前运行状态。
        lesson["entered"] = true; lesson["version"] = lesson.at("version").get<int>() + 1;
        // 【逐行批注】 判断条件 (lesson.value("intent", "") == "advance") {，成立时执行对应分支。
        if (lesson.value("intent", "") == "advance") {
            // 【逐行批注】 调用 to_string 完成本行所需操作：db.setProfileMeta("agent-legacy:" + lesson.at("courseId").get<std::st…。
            db.setProfileMeta("agent-legacy:" + lesson.at("courseId").get<std::string>() + ":" + std::to_string(lesson.at("phaseIndex").get<int>()) + ":" + std::to_string(lesson.at("topicIndex").get<int>()), lesson.at("id"));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 lesson["updatedAt"]，更新当前运行状态。
        visit();
        lesson["updatedAt"] = now(); saveLesson(db, lesson); result = link(lesson);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 agentAttachLessonTask，执行该函数声明的职责。
void agentAttachLessonTask(Database& db, const AgentAccess& access, const Json& event, const std::string& taskId) {
    // 【逐行批注】 判断条件 (!event.contains("lessonId")) return;，成立时执行对应分支。
    if (!event.contains("lessonId")) return;
    // 【逐行批注】 判断条件 (event.value("type", "") == "independent_chat") return;，成立时执行对应分支。
    if (event.value("type", "") == "independent_chat") return;
    // 【逐行批注】 计算右侧结果并写入 lesson，更新当前运行状态。
    auto lesson = lessonFor(db, access, text(event, "lessonId"));
    // 【逐行批注】 判断条件 (event.value("type", "") == "prepare_next" && event.value("resum…，成立时执行对应分支。
    if (event.value("type", "") == "prepare_next" && event.value("resumeDraft", false)) {
        // 【逐行批注】 计算右侧结果并写入 auto ownerRow，更新当前运行状态。
        const auto ownerRow = db.getClassroomActivity(lesson.value("taskId", ""));
        // 【逐行批注】 计算右侧结果并写入 auto owner，更新当前运行状态。
        const auto owner = ownerRow ? Json::parse(ownerRow->payload) : Json::object();
        // 【逐行批注】 计算右侧结果并写入 auto original，更新当前运行状态。
        const auto original = owner.value("event", Json::object());
        // 【逐行批注】 计算右侧结果并写入 auto status，更新当前运行状态。
        const auto status = owner.value("status", "");
        // 【逐行批注】 判断条件 (lesson.at("status") != "draft" || lesson.at("courseId") != even…，成立时执行对应分支。
        if (lesson.at("status") != "draft" || lesson.at("courseId") != event.value("courseId", "") ||
            // 【逐行批注】 执行当前声明或表达式：lesson.value("topicId", "") != event.value("topicId", "") || lesson.value("intent", …。
            lesson.value("topicId", "") != event.value("topicId", "") || lesson.value("intent", "") != "advance" ||
            // 【逐行批注】 调用 deepCourseEnabled 完成本行所需操作：lesson.value("sourceLearningVersion", -1) != db.learningRevision() ||…。
            lesson.value("sourceLearningVersion", -1) != db.learningRevision() || !deepCourseEnabled(db, lesson.at("courseId")) ||
            // 【逐行批注】 执行当前声明或表达式：owner.value("scopeId", "") != access.scopeId || original.value("type", "") != "plan_…。
            owner.value("scopeId", "") != access.scopeId || original.value("type", "") != "plan_course" || original.value("mode", "") != "deep" ||
            // 【逐行批注】 调用 object 完成本行所需操作：owner.value("course", Json::object()).value("id", "") != lesson.at("c…。
            owner.value("course", Json::object()).value("id", "") != lesson.at("courseId") ||
            // 【逐行批注】 执行当前声明或表达式：(status != "ready" && status != "failed" && status != "cancelled" && status != "supe…。
            (status != "ready" && status != "failed" && status != "cancelled" && status != "superseded"))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("只能继续同范围、同知识点、学习情况未变且原规划任务已结束的深度备课草稿");。
            throw std::invalid_argument("只能继续同范围、同知识点、学习情况未变且原规划任务已结束的深度备课草稿");
        // 【逐行批注】 计算右侧结果并写入 lesson["taskId"]，更新当前运行状态。
        lesson["taskId"] = taskId; lesson["teachingTaskId"] = taskId;
        // 【逐行批注】 计算右侧结果并写入 lesson["updatedAt"]，更新当前运行状态。
        lesson["updatedAt"] = now(); saveLesson(db, lesson); return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (lesson.at("status") != "ready" || lesson.at("courseId") != even…，成立时执行对应分支。
    if (lesson.at("status") != "ready" || lesson.at("courseId") != event.value("courseId", ""))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课程尚未准备好或所属课程不匹配");。
        throw std::invalid_argument("课程尚未准备好或所属课程不匹配");
    // 【逐行批注】 判断条件 (event.value("type", "") == "lesson_start") {，成立时执行对应分支。
    if (event.contains("yesterdayReview")) {
        const auto current = currentTeachingTask(db, access, lesson);
        if (!current.empty() && !canReplaceTeachingTask(current))
            throw std::invalid_argument("本课已有教学任务，请回到课堂继续或处理该任务");
        if (lesson.value("initialTeachingTaskId", "").empty()) lesson["initialTeachingTaskId"] = taskId;
    }
    if (event.value("type", "") == "teaching_request" && isTeachingPreferenceIntent(event.value("requestType", "")))
        lesson["teachingPreferences"] = checkedTeachingPreferenceIntent(lesson, event);
    if (event.value("type", "") == "teaching_request" && event.value("requestType", "") == "revise_round") {
        const auto& revision = event.at("roundRevision");
        const auto roundId = text(revision, "roundId");
        if (!activeTeachingRound(lesson) || lesson.at("teachingRound").at("roundId") != roundId)
            throw std::invalid_argument("当前教学轮次已变化，请刷新课堂后重试");
        const auto goal = text(revision, "goal");
        if (goal.size() > 800 || goal.find_first_not_of(" \r\n\t") == std::string::npos)
            throw std::invalid_argument("请输入有效的本轮学习目标");
        auto next = newTeachingRound(goal, checkedRoundQuestionCount(revision.value("questionCount", Json())));
        next["supersedesRoundId"] = roundId;
        auto& current = lesson.at("teachingRound");
        current["status"] = "revised";
        current["revisedAt"] = now();
        current.erase("pendingSectionId");
        saveCurrentRound(lesson, std::move(next));
    }
    if (event.value("type", "") == "lesson_start") {
        // 新开课只接管本课终态；取消和过时记录保留原样，不重放旧请求。
        // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
        const auto current = currentTeachingTask(db, access, lesson);
        // 【逐行批注】 判断条件 (!current.empty() && current.at("id") != taskId && !canReplaceTe…，成立时执行对应分支。
        if (!current.empty() && current.at("id") != taskId && !canReplaceTeachingTask(current))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("本课当前教学任务尚未完成，请先继续或恢复该任务");。
            throw std::invalid_argument("本课当前教学任务尚未完成，请先继续或恢复该任务");
        // 【逐行批注】 计算右侧结果并写入 lesson["initialTeachingTaskId"]，更新当前运行状态。
        lesson["initialTeachingTaskId"] = taskId;
        // 【逐行批注】 计算右侧结果并写入 lesson["teachingTaskId"]，更新当前运行状态。
        lesson["teachingTaskId"] = taskId;
    // 【逐行批注】 计算右侧结果并写入 } else lesson["teachingTaskId"]，更新当前运行状态。
    } else lesson["teachingTaskId"] = taskId;
    // 【逐行批注】 计算右侧结果并写入 lesson["updatedAt"]，更新当前运行状态。
    lesson["updatedAt"] = now(); saveLesson(db, lesson);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 agentStartLesson，执行该函数声明的职责。
Json agentStartLesson(Database& db, const AgentAccess& access, const Json& body) {
    // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
    const auto id = text(body, "lessonId"); agentEnterLesson(db, access, body);
    // 【逐行批注】 按循环规则重复处理：for (int attempt = 0; attempt < 2; ++attempt) {。
    for (int attempt = 0; attempt < 2; ++attempt) {
        // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
        const auto lesson = lessonFor(db, access, id);
        // 【逐行批注】 判断条件 (!lesson.value("initialTeachingTaskId", "").empty()) return agen…，成立时执行对应分支。
        if (!lesson.value("initialTeachingTaskId", "").empty()) return agentView(db, access, lesson.at("initialTeachingTaskId"));
        // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
        const auto current = currentTeachingTask(db, access, lesson);
        // 【逐行批注】 判断条件 (!current.empty() && !canReplaceTeachingTask(current)) {，成立时执行对应分支。
        if (!current.empty() && !canReplaceTeachingTask(current)) {
            // 继续观察或恢复原请求；等它有效结束后再开讲，不能因切页打断原回答。
            // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
            auto result = agentView(db, access, current.at("id")); result["initialTeachingDeferred"] = true; return result;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 结束当前函数并返回结果：return agentSubmit(db, access, {{"type", "lesson_start"}, {"requestId…。
            return agentSubmit(db, access, {{"type", "lesson_start"}, {"requestId", "lesson-start:" + id},
                // 【逐行批注】 执行当前声明或表达式：{"courseId", lesson.at("courseId")}, {"lessonId", id},。
                {"courseId", lesson.at("courseId")}, {"lessonId", id},
                // 【逐行批注】 执行当前声明或表达式：{"text", "请读取本课已经保存的板块和真实学习记录，选择当前讲解段及合适的互动题，分段带我学习。围绕当前段边问边讲；等待我的反馈再决定继续、补讲或举例。已有课件…。
                {"text", "请读取本课已经保存的板块和真实学习记录，选择当前讲解段及合适的互动题，分段带我学习。围绕当前段边问边讲；等待我的反馈再决定继续、补讲或举例。已有课件无需重新生成，切换讲解、练习和小结不表示完成。"}});
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::invalid_argument& error) {。
        } catch (const std::invalid_argument& error) {
            // 【逐行批注】 判断条件 (std::string(error.what()) != "本课当前教学任务尚未完成，请先继续或恢复该任务" || attem…，成立时执行对应分支。
            if (std::string(error.what()) != "本课当前教学任务尚未完成，请先继续或恢复该任务" || attempt != 0) throw;
            // 读取与提交间若出现新任务，重读后返回它；不重复生成或覆盖并发任务。
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("当前课堂任务已经变化，请保留输入后重试");。
    throw std::runtime_error("当前课堂任务已经变化，请保留输入后重试");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 agentExposeTask，执行该函数声明的职责。
Json agentExposeTask(Database& db, const AgentAccess& access, const Json& body) {
    // 【逐行批注】 判断条件 (body.contains("lessonId") && (body.contains("sections") || body…，成立时执行对应分支。
    if (body.contains("lessonId") && (body.contains("sections") || body.contains("evaluations"))) {
        // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
        const auto id = text(body, "lessonId");
        // 【逐行批注】 计算右侧结果并写入 auto sections，更新当前运行状态。
        const auto sections = body.value("sections", Json::array()), evaluations = body.value("evaluations", Json::array());
        // 【逐行批注】 判断条件 (!sections.is_array() || !evaluations.is_array()) throw std::inv…，成立时执行对应分支。
        if (!sections.is_array() || !evaluations.is_array()) throw std::invalid_argument("展示板块或评价列表无效");
        // 【逐行批注】 执行当前声明或表达式：Json result;。
        Json result;
        // 【逐行批注】 执行当前声明或表达式：db.transaction([&] {。
        db.transaction([&] {
            // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
            const auto lesson = lessonFor(db, access, id);
            // 【逐行批注】 判断条件 (lesson.at("status") != "ready" || !lesson.value("entered", fals…，成立时执行对应分支。
            if (lesson.at("status") != "ready" || !lesson.value("entered", false)) throw std::invalid_argument("课时尚未进入");
            // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
            const auto key = "agent-section-exposure:" + id;
            // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
            const auto previous = Json::parse(db.profileMeta(key), nullptr, false);
            // 【逐行批注】 计算右侧结果并写入 exposure，更新当前运行状态。
            auto exposure = previous.is_object() ? previous : Json::object();
            // 【逐行批注】 计算右侧结果并写入 auto detailKey，更新当前运行状态。
            const auto detailKey = "agent-section-display:" + id;
            // 【逐行批注】 计算右侧结果并写入 details，更新当前运行状态。
            auto details = Json::parse(db.profileMeta(detailKey), nullptr, false);
            // 【逐行批注】 判断条件 (!details.is_object()) details = Json::object();，成立时执行对应分支。
            if (!details.is_object()) details = Json::object();
            // 【逐行批注】 按循环规则重复处理：for (const auto& entry : sections) {。
            for (const auto& entry : sections) {
                // 【逐行批注】 计算右侧结果并写入 auto sectionId，更新当前运行状态。
                const auto sectionId = text(entry, "id"); bool found = false;
                // 【逐行批注】 按循环规则重复处理：for (const auto& section : lesson.at("sections"))。
                for (const auto& section : lesson.at("sections"))
                    // 【逐行批注】 判断条件 (section.at("id") == sectionId && entry.value("version", Json())…，成立时执行对应分支。
                    if (section.at("id") == sectionId && entry.value("version", Json()).is_number_integer() && entry.at("version") == section.at("version")) found = true;
                // 【逐行批注】 判断条件 (!found) throw std::invalid_argument("展示板块身份或版本已变化");，成立时执行对应分支。
                if (!found) throw std::invalid_argument("展示板块身份或版本已变化");
                // 【逐行批注】 计算右侧结果并写入 auto complete，更新当前运行状态。
                const auto complete = entry.value("complete", body.value("complete", Json()));
                // 【逐行批注】 判断条件 (!complete.is_null() && !complete.is_boolean()) throw std::inval…，成立时执行对应分支。
                if (!complete.is_null() && !complete.is_boolean()) throw std::invalid_argument("板块完整展示标记无效");
                // 【逐行批注】 计算右侧结果并写入 auto prior，更新当前运行状态。
                const auto prior = details.value(sectionId, Json::object());
                // 【逐行批注】 计算右侧结果并写入 details[sectionId]，更新当前运行状态。
                details[sectionId] = {{"version", entry.at("version")}, {"complete", complete == true ||
                    // 【逐行批注】 调用 Json 完成本行所需操作：(prior.value("version", Json()) == entry.at("version") && prior.value…。
                    (prior.value("version", Json()) == entry.at("version") && prior.value("complete", false))}};
                // 【逐行批注】 计算右侧结果并写入 exposure[sectionId]，更新当前运行状态。
                exposure[sectionId] = entry.at("version");
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 执行当前声明或表达式：db.setProfileMeta(detailKey, details.dump());。
            db.setProfileMeta(detailKey, details.dump());
            // 【逐行批注】 计算右侧结果并写入 db.setProfileMeta(key, exposure.dump()); res…，更新当前运行状态。
            db.setProfileMeta(key, exposure.dump()); result = {{"lessonId", id}, {"seenSections", exposure.size()}};
            // 【逐行批注】 判断条件 (!evaluations.empty()) {，成立时执行对应分支。
            if (!evaluations.empty()) {
                // 【逐行批注】 计算右侧结果并写入 auto evaluationKey，更新当前运行状态。
                const auto evaluationKey = "agent-evaluation-exposure:" + id;
                // 【逐行批注】 计算右侧结果并写入 saved，更新当前运行状态。
                auto saved = Json::parse(db.profileMeta(evaluationKey), nullptr, false);
                // 【逐行批注】 判断条件 (!saved.is_object()) saved = Json::object();，成立时执行对应分支。
                if (!saved.is_object()) saved = Json::object();
                // 【逐行批注】 计算右侧结果并写入 auto interactions，更新当前运行状态。
                const auto interactions = db.listInteractions();
                // 【逐行批注】 按循环规则重复处理：for (const auto& entry : evaluations) {。
                for (const auto& entry : evaluations) {
                    // 【逐行批注】 计算右侧结果并写入 auto evaluationId，更新当前运行状态。
                    const auto evaluationId = text(entry, "id");
                    // 【逐行批注】 判断条件 (!entry.value("complete", Json()).is_boolean()) throw std::inval…，成立时执行对应分支。
                    if (!entry.value("complete", Json()).is_boolean()) throw std::invalid_argument("评价展示须说明反馈是否已完整展示");
                    // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
                    const auto found = std::find_if(interactions.begin(), interactions.end(), [&](const auto& row) {
                        // 【逐行批注】 结束当前函数并返回结果：return row.id == evaluationId && row.kind == "practice" && row.course…。
                        return row.id == evaluationId && row.kind == "practice" && row.courseId == lesson.at("courseId").get<std::string>();
                    // 【逐行批注】 执行当前声明或表达式：});。
                    });
                    // 【逐行批注】 判断条件 (found == interactions.end()) throw std::invalid_argument("评价未发布…，成立时执行对应分支。
                    if (found == interactions.end()) throw std::invalid_argument("评价未发布或不属于当前课程");
                    // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
                    const auto value = Json::parse(found->payload);
                    // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
                    const auto input = std::find_if(interactions.begin(), interactions.end(), [&](const auto& row) {
                        // 【逐行批注】 结束当前函数并返回结果：return row.id == value.value("interactionId", "") && row.kind == "cha…。
                        return row.id == value.value("interactionId", "") && row.kind == "chat-user";
                    // 【逐行批注】 执行当前声明或表达式：});。
                    });
                    // 【逐行批注】 判断条件 (input == interactions.end()) throw std::invalid_argument("评价缺少真…，成立时执行对应分支。
                    if (input == interactions.end()) throw std::invalid_argument("评价缺少真实输入关联");
                    // 【逐行批注】 计算右侧结果并写入 auto answer，更新当前运行状态。
                    const auto answer = Json::parse(input->payload);
                    // 【逐行批注】 判断条件 (answer.value("scopeId", "") != access.scopeId || answer.value("…，成立时执行对应分支。
                    if (answer.value("scopeId", "") != access.scopeId || answer.value("lessonId", "") != id ||
                        // 【逐行批注】 执行当前声明或表达式：value.value("source", "") != "ai" || value.value("status", "") != "valid" ||。
                        value.value("source", "") != "ai" || value.value("status", "") != "valid" ||
                        // 【逐行批注】 调用 text、Json 完成本行所需操作：text(entry, "feedback") != value.value("feedback", "") || !value.valu…。
                        text(entry, "feedback") != value.value("feedback", "") || !value.value("questionSnapshot", Json()).is_object())
                        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("评价展示身份或反馈原文不匹配");。
                        throw std::invalid_argument("评价展示身份或反馈原文不匹配");
                    // 【逐行批注】 计算右侧结果并写入 auto declared，更新当前运行状态。
                    const auto declared = value.value("feedbackHelpType", "unknown");
                    // 【逐行批注】 判断条件 (!teachingHelpType(declared)) throw std::invalid_argument("评价帮助类…，成立时执行对应分支。
                    if (!teachingHelpType(declared)) throw std::invalid_argument("评价帮助类型无效");
                    // 【逐行批注】 计算右侧结果并写入 bool complete，更新当前运行状态。
                    const bool complete = entry.at("complete").get<bool>() ||
                        // 【逐行批注】 执行当前声明或表达式：(saved.contains(evaluationId) && !saved.at(evaluationId).value("partial", true));。
                        (saved.contains(evaluationId) && !saved.at(evaluationId).value("partial", true));
                    // 【逐行批注】 计算右侧结果并写入 saved[evaluationId]，更新当前运行状态。
                    saved[evaluationId] = {{"evaluationId", evaluationId}, {"source", "evaluation"},
                        // 【逐行批注】 调用 teachingQuestionIdentity 完成本行所需操作：{"questionId", value.at("questionId")}, {"questionIdentity", teaching…。
                        {"questionId", value.at("questionId")}, {"questionIdentity", teachingQuestionIdentity(lesson.at("courseId"), value.at("questionSnapshot"))},
                        // 【逐行批注】 执行当前声明或表达式：{"declaredHelpType", declared}, {"helpType", !complete && (declared == "hint" || dec…。
                        {"declaredHelpType", declared}, {"helpType", !complete && (declared == "hint" || declared == "worked_example") ? "unknown" : declared},
                        // 【逐行批注】 调用 evaluationFeedbackText 完成本行所需操作：{"partial", !complete}, {"text", evaluationFeedbackText(value)}, {"se…。
                        {"partial", !complete}, {"text", evaluationFeedbackText(value)}, {"sectionId", answer.value("sectionId", "")}};
                    // 【逐行批注】 判断条件 (value.contains("demonstratedQuestions")) saved[evaluationId]["d…，成立时执行对应分支。
                    if (value.contains("demonstratedQuestions")) saved[evaluationId]["demonstratedQuestions"] = value.at("demonstratedQuestions");
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 db.setProfileMeta(evaluationKey, saved.dump(…，更新当前运行状态。
                db.setProfileMeta(evaluationKey, saved.dump()); result["seenEvaluations"] = saved.size();
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 结束当前函数并返回结果：return result;。
        return result;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto taskId，更新当前运行状态。
    const auto taskId = text(body, "taskId");
    // 【逐行批注】 判断条件 (!body.value("seq", Json()).is_number_integer()) throw std::inva…，成立时执行对应分支。
    if (!body.value("seq", Json()).is_number_integer()) throw std::invalid_argument("展示序号无效");
    // 【逐行批注】 执行当前声明或表达式：Json result;。
    Json result;
    // 【逐行批注】 执行当前声明或表达式：db.transaction([&] {。
    db.transaction([&] {
        // 【逐行批注】 计算右侧结果并写入 auto taskRow，更新当前运行状态。
        const auto taskRow = db.getClassroomActivity(taskId);
        // 【逐行批注】 判断条件 (!taskRow || taskRow->kind != "agent-task") throw std::invalid_a…，成立时执行对应分支。
        if (!taskRow || taskRow->kind != "agent-task") throw std::invalid_argument("任务不存在");
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = Json::parse(taskRow->payload);
        // 【逐行批注】 判断条件 (task.at("scopeId") != access.scopeId) throw std::invalid_argume…，成立时执行对应分支。
        if (task.at("scopeId") != access.scopeId) throw std::invalid_argument("无权记录该任务的展示状态");
        // 【逐行批注】 按循环规则重复处理：for (const auto& course : task.at("courseIds")) requireCourse(db, access, …。
        for (const auto& course : task.at("courseIds")) requireCourse(db, access, course);
        // 【逐行批注】 计算右侧结果并写入 int sequence，更新当前运行状态。
        const int sequence = body.at("seq");
        // 【逐行批注】 判断条件 (sequence < 0 || sequence > task.value("seq", 0)) throw std::inv…，成立时执行对应分支。
        if (sequence < 0 || sequence > task.value("seq", 0)) throw std::invalid_argument("展示序号尚未保存");
        // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
        const auto key = "agent-exposure:" + taskId;
        // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
        const auto previous = Json::parse(db.profileMeta(key), nullptr, false);
        // 【逐行批注】 计算右侧结果并写入 int old，更新当前运行状态。
        const int old = previous.is_object() ? previous.value("seq", 0) : 0;
        // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
        result = {{"seq", std::max(sequence, old)}, {"taskId", taskId}};
        // 任务只保留最近一段事件；较早但仍已提交的流式正文在归档表中。
        // 【逐行批注】 计算右侧结果并写入 auto visibleMessage，更新当前运行状态。
        const auto visibleMessage = [&](int wanted) {
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : task.value("events", Json::array()))。
            for (const auto& item : task.value("events", Json::array()))
                // 【逐行批注】 判断条件 (item.value("seq", 0) == wanted && item.value("type", "") == "de…，成立时执行对应分支。
                if (item.value("seq", 0) == wanted && item.value("type", "") == "delta" &&
                    // 【逐行批注】 执行当前声明或表达式：item.value("field", "") == "message") return true;。
                    item.value("field", "") == "message") return true;
            // 【逐行批注】 按循环规则重复处理：for (const auto& raw : db.agentRunEvents(taskId, wanted - 1, 1)) {。
            if (const auto raw = db.agentRunEvent(taskId, wanted)) {
                // 【逐行批注】 计算右侧结果并写入 auto item，更新当前运行状态。
                const auto item = Json::parse(*raw, nullptr, false);
                // 【逐行批注】 判断条件 (item.is_object() && item.value("seq", 0) == wanted &&，成立时执行对应分支。
                if (item.is_object() && item.value("seq", 0) == wanted &&
                    // 【逐行批注】 执行当前声明或表达式：item.value("type", "") == "delta" && item.value("field", "") == "message") return tr…。
                    item.value("type", "") == "delta" && item.value("field", "") == "message") return true;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return false;。
            return false;
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 判断条件 (body.contains("complete") && !body.at("complete").is_boolean())…，成立时执行对应分支。
        if (body.contains("complete") && !body.at("complete").is_boolean()) throw std::invalid_argument("消息完整展示标记无效");
        // 【逐行批注】 判断条件 (body.contains("displayedSequences") || (previous.is_object() &&…，成立时执行对应分支。
        if (body.contains("displayedSequences") || (previous.is_object() && previous.contains("displayedSequences"))) {
            // 【逐行批注】 计算右侧结果并写入 Json displayed，更新当前运行状态。
            Json displayed = previous.is_object() ? previous.value("displayedSequences", Json::array()) : Json::array();
            // 【逐行批注】 判断条件 (!displayed.is_array()) throw std::invalid_argument("已保存的展示序号格式无…，成立时执行对应分支。
            if (!displayed.is_array()) throw std::invalid_argument("已保存的展示序号格式无效");
            // 历史客户端只有游标，保留它此前已经记录的展示；新客户端逐条记录可见文字。
            // 【逐行批注】 判断条件 (previous.is_object() && !previous.contains("displayedSequences"…，成立时执行对应分支。
            if (previous.is_object() && !previous.contains("displayedSequences"))
                // 【逐行批注】 按循环规则重复处理：for (const auto& item : task.value("events", Json::array()))。
                for (const auto& item : task.value("events", Json::array()))
                    // 【逐行批注】 判断条件 (item.value("type", "") == "delta" && item.value("field", "") ==…，成立时执行对应分支。
                    if (item.value("type", "") == "delta" && item.value("field", "") == "message" && item.value("seq", 0) <= old)
                        // 【逐行批注】 执行当前声明或表达式：displayed.push_back(item.at("seq"));。
                        displayed.push_back(item.at("seq"));
            // 【逐行批注】 计算右侧结果并写入 auto incoming，更新当前运行状态。
            const auto incoming = body.value("displayedSequences", Json::array());
            // 【逐行批注】 判断条件 (!incoming.is_array()) throw std::invalid_argument("展示序号列表无效");，成立时执行对应分支。
            if (!incoming.is_array()) throw std::invalid_argument("展示序号列表无效");
            // 【逐行批注】 按循环规则重复处理：for (const auto& value : incoming) {。
            for (const auto& value : incoming) {
                // 【逐行批注】 判断条件 (!value.is_number_integer() || value.get<int>() <= 0 || value.ge…，成立时执行对应分支。
                if (!value.is_number_integer() || value.get<int>() <= 0 || value.get<int>() > sequence)
                    // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("展示序号尚未保存");。
                    throw std::invalid_argument("展示序号尚未保存");
                // 【逐行批注】 判断条件 (!visibleMessage(value.get<int>())) throw std::invalid_argument(…，成立时执行对应分支。
                if (!visibleMessage(value.get<int>())) throw std::invalid_argument("只能记录实际展示的教学文字");
                // 【逐行批注】 判断条件 (std::find(displayed.begin(), displayed.end(), value) == display…，成立时执行对应分支。
                if (std::find(displayed.begin(), displayed.end(), value) == displayed.end()) displayed.push_back(value);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 result["displayedSequences"]，更新当前运行状态。
            result["displayedSequences"] = displayed;
        // 【逐行批注】 结束当前代码块。
        }
        // 新客户端仅将正文完整进入视区的片段列入；旧游标只证明曾展示，不能推断完整。
        // 【逐行批注】 计算右侧结果并写入 complete，更新当前运行状态。
        auto complete = previous.is_object() ? previous.value("completeSequences", Json::array()) : Json::array();
        // 【逐行批注】 计算右侧结果并写入 incomingComplete，更新当前运行状态。
        auto incomingComplete = body.value("completeSequences", Json::array());
        // 【逐行批注】 判断条件 (!complete.is_array() || !incomingComplete.is_array()) throw std…，成立时执行对应分支。
        if (!complete.is_array() || !incomingComplete.is_array()) throw std::invalid_argument("完整展示序号列表无效");
        // 【逐行批注】 判断条件 (body.value("complete", false)) for (const auto& item : task.val…，成立时执行对应分支。
        if (body.value("complete", false)) for (const auto& item : task.value("events", Json::array()))
            // 【逐行批注】 判断条件 (item.value("type", "") == "delta" && item.value("field", "") ==…，成立时执行对应分支。
            if (item.value("type", "") == "delta" && item.value("field", "") == "message" && item.value("seq", 0) <= sequence &&
                // 【逐行批注】 调用 find 完成本行所需操作：(!result.contains("displayedSequences") || std::find(result["displaye…。
                (!result.contains("displayedSequences") || std::find(result["displayedSequences"].begin(), result["displayedSequences"].end(), item.at("seq")) != result["displayedSequences"].end()))
                // 【逐行批注】 执行当前声明或表达式：incomingComplete.push_back(item.at("seq"));。
                incomingComplete.push_back(item.at("seq"));
        // 【逐行批注】 按循环规则重复处理：for (const auto& value : incomingComplete) {。
        for (const auto& value : incomingComplete) {
            // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
            const bool valid = value.is_number_integer() && value.get<int>() > 0 &&
                // 【逐行批注】 调用 visibleMessage 完成本行所需操作：value.get<int>() <= sequence && visibleMessage(value.get<int>());。
                value.get<int>() <= sequence && visibleMessage(value.get<int>());
            // 【逐行批注】 判断条件 (!valid || (result.contains("displayedSequences") && std::find(r…，成立时执行对应分支。
            if (!valid || (result.contains("displayedSequences") && std::find(result["displayedSequences"].begin(), result["displayedSequences"].end(), value) == result["displayedSequences"].end()))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("完整展示序号必须属于已展示的教学文字");。
                throw std::invalid_argument("完整展示序号必须属于已展示的教学文字");
            // 【逐行批注】 判断条件 (std::find(complete.begin(), complete.end(), value) == complete.…，成立时执行对应分支。
            if (std::find(complete.begin(), complete.end(), value) == complete.end()) complete.push_back(value);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 result["completeSequences"]，更新当前运行状态。
        result["completeSequences"] = complete;
        // 【逐行批注】 执行当前声明或表达式：db.setProfileMeta(key, result.dump());。
        db.setProfileMeta(key, result.dump());
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

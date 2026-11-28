// 【逐行批注】 引入头文件 "agent_profiles.hpp"，为后续实现提供类型或接口声明。
#include "agent_profiles.hpp"
// 【逐行批注】 引入头文件 "question_evidence.hpp"，为后续实现提供类型或接口声明。
#include "question_evidence.hpp"
// 【逐行批注】 引入头文件 "teaching_evidence.hpp"，为后续实现提供类型或接口声明。
#include "teaching_evidence.hpp"
// 【逐行批注】 引入头文件 "agent_curriculum.hpp"，为后续实现提供类型或接口声明。
#include "agent_curriculum.hpp"
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <array>，为后续实现提供类型或接口声明。
#include <array>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>
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
// 【逐行批注】 执行当前声明或表达式：const std::array<std::string, 9> subjects = {"语文", "数学", "英语", "物理", "化学", "生物", "政治…。
const std::array<std::string, 9> subjects = {"语文", "数学", "英语", "物理", "化学", "生物", "政治", "历史", "地理"};
// 【逐行批注】 执行当前声明或表达式：const std::array<std::string, 6> abilities = {"memory", "understanding", "applicatio…。
const std::array<std::string, 6> abilities = {"memory", "understanding", "application", "reasoning", "expression", "transfer"};
// 【逐行批注】 执行当前声明或表达式：const std::array<std::string, 6> names = {"知识记忆", "概念理解", "方法应用", "逻辑推理", "表达说明", "综…。
const std::array<std::string, 6> names = {"知识记忆", "概念理解", "方法应用", "逻辑推理", "表达说明", "综合迁移"};
// 【逐行批注】 定义函数 parse，执行该函数声明的职责。
Json parse(const std::string& text, Json fallback = Json::object()) {
    // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
    const auto value = Json::parse(text, nullptr, false); return value.is_discarded() ? fallback : value;
// 【逐行批注】 结束当前代码块。
}
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
    // 【逐行批注】 调用 put_time 完成本行所需操作：std::ostringstream result; result << std::put_time(&value, "%Y-%m-%dT…。
    std::ostringstream result; result << std::put_time(&value, "%Y-%m-%dT%H:%M:%SZ"); return result.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 accessFor，执行该函数声明的职责。
AgentAccess accessFor(const Json& task) { return {task.at("scopeId"), task.at("courseIds").get<std::vector<std::string>>()}; }
// 【逐行批注】 定义函数 allowed，执行该函数声明的职责。
bool allowed(Database& db, const AgentAccess& access, const std::string& courseId) {
    // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
    const auto course = db.getCourse(courseId);
    // 【逐行批注】 结束当前函数并返回结果：return course && course->status == "active" && std::find(access.cours…。
    return course && course->status == "active" && std::find(access.courseIds.begin(), access.courseIds.end(), courseId) != access.courseIds.end();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 text，执行该函数声明的职责。
std::string text(const Json& item, const char* key) {
    // 【逐行批注】 判断条件 (!item.value(key, Json()).is_string() || item.at(key).get<std::s…，成立时执行对应分支。
    if (!item.value(key, Json()).is_string() || item.at(key).get<std::string>().empty())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument(std::string(key) + " 不能为空");。
        throw std::invalid_argument(std::string(key) + " 不能为空");
    // 【逐行批注】 结束当前函数并返回结果：return item.at(key);。
    return item.at(key);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 声明数据结构或类型 CheckedRating 及其成员接口。
struct CheckedRating {
    // 【逐行批注】 执行当前声明或表达式：std::optional<int> score;。
    std::optional<int> score;
    // 【逐行批注】 计算右侧结果并写入 Json references，更新当前运行状态。
    Json references = Json::array();
    // 【逐行批注】 计算右侧结果并写入 int distinctQuestions，更新当前运行状态。
    int distinctQuestions = 0;
    // 【逐行批注】 执行当前声明或表达式：std::string rationale, recommendation, uncertainty;。
    std::string rationale, recommendation, uncertainty;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 定义函数 checked，执行该函数声明的职责。
CheckedRating checked(const Json& item, const Json& evidence) {
    // 【逐行批注】 判断条件 (!item.contains("score") || !item.value("sufficient", Json()).is…，成立时执行对应分支。
    if (!item.contains("score") || !item.value("sufficient", Json()).is_boolean() || !item.value("evidenceIds", Json()).is_array())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("AI 评分、证据充分性与引用结构不完整");。
        throw std::invalid_argument("AI 评分、证据充分性与引用结构不完整");
    // 【逐行批注】 执行当前声明或表达式：CheckedRating rating;。
    CheckedRating rating;
    // 【逐行批注】 计算右侧结果并写入 rating.rationale，更新当前运行状态。
    rating.rationale = text(item, "rationale"); rating.recommendation = text(item, "recommendation");
    // 【逐行批注】 计算右侧结果并写入 rating.uncertainty，更新当前运行状态。
    rating.uncertainty = text(item, "uncertainty");
    // 【逐行批注】 执行当前声明或表达式：std::set<std::string> ids, questions;。
    std::set<std::string> ids, questions;
    // 【逐行批注】 按循环规则重复处理：for (const auto& id : item.at("evidenceIds")) {。
    for (const auto& id : item.at("evidenceIds")) {
        // 【逐行批注】 判断条件 (!id.is_string() || !ids.insert(id.get<std::string>()).second) t…，成立时执行对应分支。
        if (!id.is_string() || !ids.insert(id.get<std::string>()).second) throw std::invalid_argument("画像引用无效或重复");
        // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
        const auto found = std::find_if(evidence.begin(), evidence.end(), [&](const Json& value) { return value.at("id") == id; });
        // 【逐行批注】 判断条件 (found == evidence.end()) throw std::invalid_argument("画像引用不存在的可…，成立时执行对应分支。
        if (found == evidence.end()) throw std::invalid_argument("画像引用不存在的可靠原题或实际回答");
        // 【逐行批注】 调用 at 完成本行所需操作：questions.insert(found->at("questionId")); rating.references.push_bac…。
        questions.insert(found->at("questionId")); rating.references.push_back(id);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 rating.distinctQuestions，更新当前运行状态。
    rating.distinctQuestions = static_cast<int>(questions.size());
    // 【逐行批注】 判断条件 (!item.at("score").is_null()) {，成立时执行对应分支。
    if (!item.at("score").is_null()) {
        // 【逐行批注】 判断条件 (!item.at("score").is_number_integer() || item.at("score").get<i…，成立时执行对应分支。
        if (!item.at("score").is_number_integer() || item.at("score").get<int>() < 0 || item.at("score").get<int>() > 100 ||
            // 【逐行批注】 执行当前声明或表达式：!item.at("sufficient").get<bool>() || questions.empty())。
            !item.at("sufficient").get<bool>() || questions.empty())
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("正式分数必须有真实作答引用和 AI 的充分性判断");。
            throw std::invalid_argument("正式分数必须有真实作答引用和 AI 的充分性判断");
        // 【逐行批注】 计算右侧结果并写入 rating.score，更新当前运行状态。
        rating.score = item.at("score").get<int>();
    // 【逐行批注】 调用 invalid_argument 完成本行所需操作：} else if (item.at("sufficient").get<bool>()) throw std::invalid_argu…。
    } else if (item.at("sufficient").get<bool>()) throw std::invalid_argument("充分性与空评分不一致");
    // 【逐行批注】 结束当前函数并返回结果：return rating;。
    return rating;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 agentProfileEvidence，执行该函数声明的职责。
Json agentProfileEvidence(Database& db, const AgentAccess& access) {
    // 【逐行批注】 计算右侧结果并写入 Json evidence，更新当前运行状态。
    Json evidence = Json::array();
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, Json> actualInputs;。
    std::map<std::string, Json> actualInputs;
    // 【逐行批注】 计算右侧结果并写入 auto interactions，更新当前运行状态。
    const auto interactions = db.listInteractions();
    // 【逐行批注】 按循环规则重复处理：for (const auto& row : interactions) if (row.kind == "chat-user" && row.co…。
    for (const auto& row : interactions) if (row.kind == "chat-user" && row.courseId && allowed(db, access, *row.courseId)) {
        // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
        const auto value = parse(row.payload);
        // 【逐行批注】 判断条件 (value.value("scopeId", "") == access.scopeId) actualInputs[row.…，成立时执行对应分支。
        if (value.value("scopeId", "") == access.scopeId) actualInputs[row.id] = value;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto append，更新当前运行状态。
    const auto append = [&](const std::string& id, const std::string& courseId, const Json& snapshot, const Json& answer,
                            // 【逐行批注】 执行当前声明或表达式：const std::string& at, bool unknown, bool assisted, const Json& hints, const std::st…。
                            const std::string& at, bool unknown, bool assisted, const Json& hints, const std::string& topic) {
        // 【逐行批注】 判断条件 (!allowed(db, access, courseId) || !snapshot.is_object() || !sna…，成立时执行对应分支。
        if (!allowed(db, access, courseId) || !snapshot.is_object() || !snapshot.value("question", Json()).is_string() ||
            // 【逐行批注】 执行当前声明或表达式：snapshot.at("question").get<std::string>().empty() || answer.is_null() ||。
            snapshot.at("question").get<std::string>().empty() || answer.is_null() ||
            // 【逐行批注】 执行当前声明或表达式：(answer.is_string() && answer.get<std::string>().empty())) return;。
            (answer.is_string() && answer.get<std::string>().empty())) return;
        // 【逐行批注】 调用 teachingQuestionIdentity 完成本行所需操作：evidence.push_back({{"id", id}, {"courseId", courseId}, {"questionId"…。
        evidence.push_back({{"id", id}, {"courseId", courseId}, {"questionId", teachingQuestionEvidenceKey(courseId, snapshot)},
            // 【逐行批注】 执行当前声明或表达式：{"question", snapshot}, {"answer", answer}, {"unknown", unknown}, {"assisted", assis…。
            {"question", snapshot}, {"answer", answer}, {"unknown", unknown}, {"assisted", assisted},
            // 【逐行批注】 执行当前声明或表达式：{"hintHistory", hints}, {"at", at}, {"topic", topic}});。
            {"hintHistory", hints}, {"at", at}, {"topic", topic}});
        // 【逐行批注】 计算右侧结果并写入 evidence.back()["helpMode"]，更新当前运行状态。
        evidence.back()["helpMode"] = "help_unknown";
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 按循环规则重复处理：for (const auto& row : interactions) {。
    for (const auto& row : interactions) {
        // 【逐行批注】 判断条件 (!row.courseId || !allowed(db, access, *row.courseId)) continue;，成立时执行对应分支。
        if (!row.courseId || !allowed(db, access, *row.courseId)) continue;
        // 【逐行批注】 计算右侧结果并写入 auto item，更新当前运行状态。
        const auto item = parse(row.payload);
        // 【逐行批注】 判断条件 (row.kind == "practice" && item.value("source", "") == "ai" && i…，成立时执行对应分支。
        if (row.kind == "practice" && item.value("source", "") == "ai" && item.value("status", "") == "valid" && item.value("credible", false)) {
            // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
            const auto input = actualInputs.find(item.value("interactionId", ""));
            // 【逐行批注】 判断条件 (input == actualInputs.end() || !db.profileMeta("agent-input-cla…，成立时执行对应分支。
            if (input == actualInputs.end() || !db.profileMeta("agent-input-classification:" + item.value("interactionId", "")).empty() ||
                // 【逐行批注】 执行当前声明或表达式：input->second.value("action", "") == "skip" || input->second.value("action", "") == …。
                input->second.value("action", "") == "skip" || input->second.value("action", "") == "stop" ||
                // 【逐行批注】 调用 Json 完成本行所需操作：item.value("response", Json()) != input->second.value("text", Json())…。
                item.value("response", Json()) != input->second.value("text", Json()) ||
                // 【逐行批注】 调用 Json 完成本行所需操作：item.value("questionSnapshot", Json()) != input->second.value("questi…。
                item.value("questionSnapshot", Json()) != input->second.value("questionSnapshot", Json())) continue;
            // 【逐行批注】 调用 append、Json 完成本行所需操作：append(row.id, *row.courseId, item.value("questionSnapshot", Json()),…。
            append(row.id, *row.courseId, item.value("questionSnapshot", Json()), item.value("response", Json()), row.createdAt,
                // 【逐行批注】 调用 array 完成本行所需操作：input->second.value("action", "") == "unknown", item.value("assisted"…。
                input->second.value("action", "") == "unknown", item.value("assisted", false), item.value("hintHistory", Json::array()), item.value("topic", ""));
            // 【逐行批注】 判断条件 (!evidence.empty() && evidence.back().at("id") == row.id)，成立时执行对应分支。
            if (!evidence.empty() && evidence.back().at("id") == row.id)
                // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"diagnosis", "helpHistory", "relatedLearning", "…。
                for (const auto* field : {"diagnosis", "helpHistory", "relatedLearning", "helpMode", "rePracticeOf"})
                    // 【逐行批注】 判断条件 (item.contains(field)) evidence.back()[field] = item.at(field);，成立时执行对应分支。
                    if (item.contains(field)) evidence.back()[field] = item.at(field);
        // 【逐行批注】 调用 Json 完成本行所需操作：} else if ((row.kind == "quiz" || row.kind == "question-evaluation") …。
        } else if ((row.kind == "quiz" || row.kind == "question-evaluation") && item.value("results", Json()).is_array()) {
            // 【逐行批注】 计算右侧结果并写入 size_t index，更新当前运行状态。
            size_t index = 0;
            // 【逐行批注】 按循环规则重复处理：for (const auto& answer : item.at("results")) {。
            for (const auto& answer : item.at("results")) {
                // 【逐行批注】 判断条件 (answer.value("answered", false) && answer.value("credible", fal…，成立时执行对应分支。
                if (answer.value("answered", false) && answer.value("credible", false))
                    // 【逐行批注】 调用 append、to_string、Json 完成本行所需操作：append(row.id + ":" + std::to_string(index), *row.courseId, answer.va…。
                    append(row.id + ":" + std::to_string(index), *row.courseId, answer.value("questionSnapshot", Json()),
                        // 【逐行批注】 调用 Json 完成本行所需操作：answer.value("givenAnswer", Json()), row.createdAt, answer.value("unk…。
                        answer.value("givenAnswer", Json()), row.createdAt, answer.value("unknown", false), answer.value("assisted", false),
                        // 【逐行批注】 调用 array 完成本行所需操作：answer.value("hintHistory", Json::array()), item.value("topic", ""));。
                        answer.value("hintHistory", Json::array()), item.value("topic", ""));
                // 【逐行批注】 执行当前声明或表达式：++index;。
                ++index;
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& courseId : access.courseIds) if (allowed(db, access, cour…。
    for (const auto& courseId : access.courseIds) if (allowed(db, access, courseId)) {
        // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.listClassroomActivities(courseId)) {。
        for (const auto& row : db.listClassroomActivities(courseId)) {
            // 【逐行批注】 判断条件 (row.kind != "diagnostic" && row.kind != "interaction" && row.ki…，成立时执行对应分支。
            if (row.kind != "diagnostic" && row.kind != "interaction" && row.kind != "question" &&
                // 【逐行批注】 执行当前声明或表达式：row.kind != "example" && row.kind != "practice" && row.kind != "quiz" && row.kind !=…。
                row.kind != "example" && row.kind != "practice" && row.kind != "quiz" && row.kind != "review") continue;
            // 【逐行批注】 计算右侧结果并写入 auto item，更新当前运行状态。
            const auto item = parse(row.payload);
            // 【逐行批注】 判断条件 (item.value("status", "") != "answered" || !item.value("credible…，成立时执行对应分支。
            if (item.value("status", "") != "answered" || !item.value("credible", false) ||
                // 【逐行批注】 执行当前声明或表达式：item.value("evaluationStatus", "valid") == "failed" || item.value("evaluationStatus"…。
                item.value("evaluationStatus", "valid") == "failed" || item.value("evaluationStatus", "valid") == "pending") continue;
            // 【逐行批注】 计算右侧结果并写入 auto snapshot，更新当前运行状态。
            const auto snapshot = item.contains("questionSnapshot") ? item.at("questionSnapshot") : questionSnapshot(item);
            // 【逐行批注】 调用 append、Json 完成本行所需操作：append(row.id, courseId, snapshot, item.value("answer", Json()), row.…。
            append(row.id, courseId, snapshot, item.value("answer", Json()), row.updatedAt, item.value("unknown", false),
                // 【逐行批注】 调用 array 完成本行所需操作：item.value("assisted", false), item.value("hintHistory", Json::array(…。
                item.value("assisted", false), item.value("hintHistory", Json::array()), item.value("topic", "课堂任务"));
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return evidence;。
    return evidence;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 registerAgentProfileTools，执行该函数声明的职责。
void registerAgentProfileTools(LearningAgent& agent) {
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("update_topic_mastery", {"AI 判断具体知识点的证据充分性与掌握情况，不使用三题门槛。参数 course…。
    agent.registerTool("update_topic_mastery", {"AI 判断具体知识点的证据充分性与掌握情况，不使用三题门槛。参数 courseId,topicId,score:0..100或null,sufficient:bool,evidenceIds,rationale,recommendation,uncertainty,weakPoints:[]可选,nextReviewAt:日期或空。引用 read_profile_evidence 中同一课程的实际证据。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto access，更新当前运行状态。
            const auto access = accessFor(task); const auto courseId = text(args, "courseId");
            // 【逐行批注】 判断条件 (!allowed(db, access, courseId)) throw std::invalid_argument("无权…，成立时执行对应分支。
            if (!allowed(db, access, courseId)) throw std::invalid_argument("无权评估该课程");
            // 【逐行批注】 计算右侧结果并写入 evidence，更新当前运行状态。
            auto evidence = agentProfileEvidence(db, access); Json relevant = Json::array();
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : evidence) if (item.at("courseId") == courseId) rel…。
            for (const auto& item : evidence) if (item.at("courseId") == courseId) relevant.push_back(item);
            // 【逐行批注】 计算右侧结果并写入 auto rating，更新当前运行状态。
            const auto rating = checked(args, relevant); const auto outline = agentOutline(db, access, courseId); Json topic;
            // 【逐行批注】 按循环规则重复处理：for (const auto& stage : outline.at("courseStructure")) for (const auto& i…。
            for (const auto& stage : outline.at("courseStructure")) for (const auto& item : stage.at("topics")) if (item.at("id") == text(args, "topicId")) topic = item;
            // 【逐行批注】 判断条件 (topic.empty()) throw std::invalid_argument("评估知识点不存在");，成立时执行对应分支。
            if (topic.empty()) throw std::invalid_argument("评估知识点不存在");
            // 【逐行批注】 计算右侧结果并写入 TopicMastery value; value.courseId，更新当前运行状态。
            TopicMastery value; value.courseId = courseId; value.topic = topic.at("title"); value.phaseIndex = topic.at("legacyPhaseIndex");
            // 【逐行批注】 计算右侧结果并写入 value.score，更新当前运行状态。
            value.score = rating.score; value.rationale = rating.rationale + "；不确定性：" + rating.uncertainty; value.recommendation = rating.recommendation;
            // 【逐行批注】 计算右侧结果并写入 auto weakPoints，更新当前运行状态。
            const auto weakPoints = args.value("weakPoints", Json::array()); if (!weakPoints.is_array()) throw std::invalid_argument("薄弱点应为文字列表");
            // 【逐行批注】 按循环规则重复处理：for (const auto& point : weakPoints) if (!point.is_string()) throw std::in…。
            for (const auto& point : weakPoints) if (!point.is_string()) throw std::invalid_argument("薄弱点应为文字");
            // 【逐行批注】 计算右侧结果并写入 value.weakPoints，更新当前运行状态。
            value.weakPoints = weakPoints.dump(); value.evidenceIds = rating.references.dump(); value.evidenceCount = rating.distinctQuestions;
            // 【逐行批注】 计算右侧结果并写入 value.nextReviewAt，更新当前运行状态。
            value.nextReviewAt = args.value("nextReviewAt", ""); value.model = source.model; value.status = rating.score ? "ready" : "insufficient";
            // 【逐行批注】 计算右侧结果并写入 value.updatedAt，更新当前运行状态。
            value.updatedAt = now(); value.version = db.profileRevision();
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"topicUpdated", true}, {"evidenceCount", r…。
            return PreparedAgentTool{{{"topicUpdated", true}, {"evidenceCount", rating.distinctQuestions}}, [value](Database& connection) {
                // 【逐行批注】 判断条件 (connection.profileRevision() != value.version) throw std::inval…，成立时执行对应分支。
                if (connection.profileRevision() != value.version) throw std::invalid_argument("知识点评估证据已变化");
                // 【逐行批注】 判断条件 (!connection.upsert(value)) throw std::runtime_error("知识点画像保存失败"…，成立时执行对应分支。
                if (!connection.upsert(value)) throw std::runtime_error("知识点画像保存失败");
            // 【逐行批注】 执行当前声明或表达式：}};。
            }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("read_profile_evidence", {"读取有权限的真实原题、实际回答、不会反馈、提示经历和评价来源。返回稳定题目标…。
    agent.registerTool("read_profile_evidence", {"读取有权限的真实原题、实际回答、不会反馈、提示经历和评价来源。返回稳定题目标识，同题多轮仍是一道题；漏答、跳过与失败不纳入。画像充分性、适用维度和评分由 AI 判断，不要求三道题。", true,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json&, const Json& task, const AIResult&) {。
        [](Database& db, const Json&, const Json& task, const AIResult&) {
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"evidence", agentProfileEvidence(db, acces…。
            return PreparedAgentTool{{{"evidence", agentProfileEvidence(db, accessFor(task))}}, {}};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("update_profiles", {"由真实 AI 更新学科和学习能力画像。参数 subjects:[{subject,sco…。
    agent.registerTool("update_profiles", {"由真实 AI 更新学科和学习能力画像。参数 subjects:[{subject,score:0..100或null,sufficient:bool,rationale,recommendation,uncertainty,evidenceIds,weakPoints:[]}],abilities:[{id:memory|understanding|application|reasoning|expression|transfer,score,sufficient,rationale,recommendation,uncertainty,evidenceIds}]。可只更新相关维度，保留其他真实结果。每项正式评分引用 read_profile_evidence 的实际记录ID，由你判断证据是否充分及题目是否适用；不机械复制正确率或学科分数，不把教师答案、浏览及完成标记当证据。维度含义：事实回忆、概念解释、已知方法解题、有依据的推导、说明过程、新情境运用。", false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db, const Json& args, const Json& task, const AIResult& source) {。
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            // 【逐行批注】 计算右侧结果并写入 auto evidence，更新当前运行状态。
            const auto evidence = agentProfileEvidence(db, accessFor(task)); const int revision = db.profileRevision();
            // 【逐行批注】 计算右侧结果并写入 auto rawAbilities，更新当前运行状态。
            const auto rawAbilities = db.profileMeta("ability-profile"), rawEvidence = db.profileMeta("agent-profile-evidence");
            // 【逐行批注】 计算右侧结果并写入 oldAbilities，更新当前运行状态。
            auto oldAbilities = parse(rawAbilities); auto references = parse(rawEvidence);
            // 【逐行批注】 判断条件 (!oldAbilities.is_object()) oldAbilities = Json::object();，成立时执行对应分支。
            if (!oldAbilities.is_object()) oldAbilities = Json::object();
            // 【逐行批注】 判断条件 (!references.is_object()) references = Json::object();，成立时执行对应分支。
            if (!references.is_object()) references = Json::object();
            // 【逐行批注】 计算右侧结果并写入 Json dimensions，更新当前运行状态。
            Json dimensions = Json::array();
            // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < abilities.size(); ++index) {。
            for (size_t index = 0; index < abilities.size(); ++index) {
                // 【逐行批注】 计算右侧结果并写入 Json value，更新当前运行状态。
                Json value = {{"id", abilities[index]}, {"name", names[index]}, {"score", nullptr}, {"evidenceCount", 0},
                    // 【逐行批注】 执行当前声明或表达式：{"rationale", "等待 AI 根据真实作答评估。"}, {"recommendation", ""}, {"updatedAt", ""}, {"scope…。
                    {"rationale", "等待 AI 根据真实作答评估。"}, {"recommendation", ""}, {"updatedAt", ""}, {"scope", "全部课程中已测任务的表现"}};
                // 【逐行批注】 按循环规则重复处理：for (const auto& previous : oldAbilities.value("dimensions", Json::array()…。
                for (const auto& previous : oldAbilities.value("dimensions", Json::array())) if (previous.value("id", "") == abilities[index]) value = previous;
                // 【逐行批注】 执行当前声明或表达式：dimensions.push_back(value);。
                dimensions.push_back(value);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 auto requestedSubjects，更新当前运行状态。
            const auto requestedSubjects = args.value("subjects", Json::array()), requestedAbilities = args.value("abilities", Json::array());
            // 【逐行批注】 判断条件 (!requestedSubjects.is_array() || !requestedAbilities.is_array()…，成立时执行对应分支。
            if (!requestedSubjects.is_array() || !requestedAbilities.is_array() || (requestedSubjects.empty() && requestedAbilities.empty()))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("至少提供一项 AI 画像判断");。
                throw std::invalid_argument("至少提供一项 AI 画像判断");
            // 【逐行批注】 执行当前声明或表达式：std::vector<SubjectMastery> values; std::set<std::string> seen;。
            std::vector<SubjectMastery> values; std::set<std::string> seen;
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : requestedSubjects) {。
            for (const auto& item : requestedSubjects) {
                // 【逐行批注】 计算右侧结果并写入 auto subject，更新当前运行状态。
                const auto subject = text(item, "subject");
                // 【逐行批注】 判断条件 (std::find(subjects.begin(), subjects.end(), subject) == subject…，成立时执行对应分支。
                if (std::find(subjects.begin(), subjects.end(), subject) == subjects.end() || !seen.insert(subject).second)
                    // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("学科范围或重复项无效");。
                    throw std::invalid_argument("学科范围或重复项无效");
                // 【逐行批注】 计算右侧结果并写入 auto rating，更新当前运行状态。
                const auto rating = checked(item, evidence);
                // 【逐行批注】 计算右侧结果并写入 auto weakPoints，更新当前运行状态。
                const auto weakPoints = item.value("weakPoints", Json::array());
                // 【逐行批注】 判断条件 (!weakPoints.is_array()) throw std::invalid_argument("薄弱点应为文字列表"…，成立时执行对应分支。
                if (!weakPoints.is_array()) throw std::invalid_argument("薄弱点应为文字列表");
                // 【逐行批注】 按循环规则重复处理：for (const auto& point : weakPoints) if (!point.is_string()) throw std::in…。
                for (const auto& point : weakPoints) if (!point.is_string()) throw std::invalid_argument("薄弱点格式无效");
                // 【逐行批注】 计算右侧结果并写入 SubjectMastery value; value.subject，更新当前运行状态。
                SubjectMastery value; value.subject = subject; value.score = rating.score;
                // 【逐行批注】 计算右侧结果并写入 value.rationale，更新当前运行状态。
                value.rationale = rating.rationale + "；评估范围与不确定性：" + rating.uncertainty;
                // 【逐行批注】 计算右侧结果并写入 value.recommendation，更新当前运行状态。
                value.recommendation = rating.recommendation; value.weakPoints = weakPoints.dump();
                // 【逐行批注】 计算右侧结果并写入 value.evidenceCount，更新当前运行状态。
                value.evidenceCount = rating.distinctQuestions; value.model = source.model; value.updatedAt = now();
                // 【逐行批注】 计算右侧结果并写入 value.status，更新当前运行状态。
                value.status = rating.score ? "ready" : "insufficient"; values.push_back(value);
                // 【逐行批注】 计算右侧结果并写入 references["subjects"][subject]，更新当前运行状态。
                references["subjects"][subject] = {{"evidenceIds", rating.references}, {"uncertainty", rating.uncertainty}, {"model", source.model}, {"at", value.updatedAt}};
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 执行当前声明或表达式：seen.clear();。
            seen.clear();
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : requestedAbilities) {。
            for (const auto& item : requestedAbilities) {
                // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
                const auto id = text(item, "id"); const auto position = std::find(abilities.begin(), abilities.end(), id);
                // 【逐行批注】 判断条件 (position == abilities.end() || !seen.insert(id).second) throw s…，成立时执行对应分支。
                if (position == abilities.end() || !seen.insert(id).second) throw std::invalid_argument("能力维度范围或重复项无效");
                // 【逐行批注】 计算右侧结果并写入 auto rating，更新当前运行状态。
                const auto rating = checked(item, evidence); auto& value = dimensions[static_cast<size_t>(position - abilities.begin())];
                // 【逐行批注】 计算右侧结果并写入 value["score"]，更新当前运行状态。
                value["score"] = rating.score ? Json(*rating.score) : Json(); value["rationale"] = rating.rationale;
                // 【逐行批注】 计算右侧结果并写入 value["recommendation"]，更新当前运行状态。
                value["recommendation"] = rating.recommendation; value["uncertainty"] = rating.uncertainty;
                // 【逐行批注】 计算右侧结果并写入 value["evidenceCount"]，更新当前运行状态。
                value["evidenceCount"] = rating.distinctQuestions; value["evidenceIds"] = rating.references;
                // 【逐行批注】 计算右侧结果并写入 value["model"]，更新当前运行状态。
                value["model"] = source.model; value["updatedAt"] = now();
                // 【逐行批注】 计算右侧结果并写入 references["abilities"][id]，更新当前运行状态。
                references["abilities"][id] = {{"evidenceIds", rating.references}, {"uncertainty", rating.uncertainty}, {"model", source.model}, {"at", value["updatedAt"]}};
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 updatedAbilities，更新当前运行状态。
            auto updatedAbilities = oldAbilities; updatedAbilities["dimensions"] = dimensions;
            // 【逐行批注】 计算右侧结果并写入 updatedAbilities["version"]，更新当前运行状态。
            updatedAbilities["version"] = revision; updatedAbilities["attemptVersion"] = revision; updatedAbilities["pendingVersion"] = revision;
            // 【逐行批注】 计算右侧结果并写入 updatedAbilities["model"]，更新当前运行状态。
            updatedAbilities["model"] = source.model; updatedAbilities["updatedAt"] = now(); updatedAbilities["error"] = "";
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"subjectsUpdated", values.size()}, {"abili…。
            return PreparedAgentTool{{{"subjectsUpdated", values.size()}, {"abilitiesUpdated", requestedAbilities.size()}, {"source", "ai"}},
                // 【逐行批注】 执行当前声明或表达式：[values, requestedAbilities, updatedAbilities, references, rawAbilities, rawEvidence…。
                [values, requestedAbilities, updatedAbilities, references, rawAbilities, rawEvidence, revision](Database& connection) {
                    // 【逐行批注】 判断条件 (connection.profileRevision() != revision || connection.profileM…，成立时执行对应分支。
                    if (connection.profileRevision() != revision || connection.profileMeta("agent-profile-evidence") != rawEvidence ||
                        // 【逐行批注】 执行当前声明或表达式：(!requestedAbilities.empty() && connection.profileMeta("ability-profile") != rawAbil…。
                        (!requestedAbilities.empty() && connection.profileMeta("ability-profile") != rawAbilities))
                        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("画像证据或其他窗口结果已有更新，旧评分不应用");。
                        throw std::invalid_argument("画像证据或其他窗口结果已有更新，旧评分不应用");
                    // 【逐行批注】 按循环规则重复处理：for (const auto& value : values) if (!connection.upsert(value)) throw std:…。
                    for (const auto& value : values) if (!connection.upsert(value)) throw std::runtime_error("学科画像保存失败");
                    // 【逐行批注】 判断条件 (!requestedAbilities.empty()) connection.setProfileMeta("ability…，成立时执行对应分支。
                    if (!requestedAbilities.empty()) connection.setProfileMeta("ability-profile", updatedAbilities.dump());
                    // 【逐行批注】 执行当前声明或表达式：connection.setProfileMeta("agent-profile-evidence", references.dump());。
                    connection.setProfileMeta("agent-profile-evidence", references.dump());
                    // 【逐行批注】 执行当前声明或表达式：connection.setProfileAssessed(revision); connection.setProfileError("");。
                    connection.setProfileAssessed(revision); connection.setProfileError("");
                // 【逐行批注】 执行当前声明或表达式：}};。
                }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

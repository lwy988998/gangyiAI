#include "agent_profiles.hpp"
#include "question_evidence.hpp"
#include "agent_curriculum.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace gangyi {
namespace {
using Json = nlohmann::json;
const std::array<std::string, 9> subjects = {"语文", "数学", "英语", "物理", "化学", "生物", "政治", "历史", "地理"};
const std::array<std::string, 6> abilities = {"memory", "understanding", "application", "reasoning", "expression", "transfer"};
const std::array<std::string, 6> names = {"知识记忆", "概念理解", "方法应用", "逻辑推理", "表达说明", "综合迁移"};
Json parse(const std::string& text, Json fallback = Json::object()) {
    const auto value = Json::parse(text, nullptr, false); return value.is_discarded() ? fallback : value;
}
std::string now() {
    const auto clock = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); std::tm value{};
#ifdef _WIN32
    gmtime_s(&value, &clock);
#else
    gmtime_r(&clock, &value);
#endif
    std::ostringstream result; result << std::put_time(&value, "%Y-%m-%dT%H:%M:%SZ"); return result.str();
}
AgentAccess accessFor(const Json& task) { return {task.at("scopeId"), task.at("courseIds").get<std::vector<std::string>>()}; }
bool allowed(Database& db, const AgentAccess& access, const std::string& courseId) {
    const auto course = db.getCourse(courseId);
    return course && course->status == "active" && std::find(access.courseIds.begin(), access.courseIds.end(), courseId) != access.courseIds.end();
}
std::string text(const Json& item, const char* key) {
    if (!item.value(key, Json()).is_string() || item.at(key).get<std::string>().empty())
        throw std::invalid_argument(std::string(key) + " 不能为空");
    return item.at(key);
}
struct CheckedRating {
    std::optional<int> score;
    Json references = Json::array();
    int distinctQuestions = 0;
    std::string rationale, recommendation, uncertainty;
};
CheckedRating checked(const Json& item, const Json& evidence) {
    if (!item.contains("score") || !item.value("sufficient", Json()).is_boolean() || !item.value("evidenceIds", Json()).is_array())
        throw std::invalid_argument("AI 评分、证据充分性与引用结构不完整");
    CheckedRating rating;
    rating.rationale = text(item, "rationale"); rating.recommendation = text(item, "recommendation");
    rating.uncertainty = text(item, "uncertainty");
    std::set<std::string> ids, questions;
    for (const auto& id : item.at("evidenceIds")) {
        if (!id.is_string() || !ids.insert(id.get<std::string>()).second) throw std::invalid_argument("画像引用无效或重复");
        const auto found = std::find_if(evidence.begin(), evidence.end(), [&](const Json& value) { return value.at("id") == id; });
        if (found == evidence.end()) throw std::invalid_argument("画像引用不存在的可靠原题或实际回答");
        questions.insert(found->at("questionId")); rating.references.push_back(id);
    }
    rating.distinctQuestions = static_cast<int>(questions.size());
    if (!item.at("score").is_null()) {
        if (!item.at("score").is_number_integer() || item.at("score").get<int>() < 0 || item.at("score").get<int>() > 100 ||
            !item.at("sufficient").get<bool>() || questions.empty())
            throw std::invalid_argument("正式分数必须有真实作答引用和 AI 的充分性判断");
        rating.score = item.at("score").get<int>();
    } else if (item.at("sufficient").get<bool>()) throw std::invalid_argument("充分性与空评分不一致");
    return rating;
}
}

Json agentProfileEvidence(Database& db, const AgentAccess& access) {
    Json evidence = Json::array();
    std::map<std::string, Json> actualInputs;
    const auto interactions = db.listInteractions();
    for (const auto& row : interactions) if (row.kind == "chat-user" && row.courseId && allowed(db, access, *row.courseId)) {
        const auto value = parse(row.payload);
        if (value.value("scopeId", "") == access.scopeId) actualInputs[row.id] = value;
    }
    const auto append = [&](const std::string& id, const std::string& courseId, const Json& snapshot, const Json& answer,
                            const std::string& at, bool unknown, bool assisted, const Json& hints, const std::string& topic) {
        if (!allowed(db, access, courseId) || !snapshot.is_object() || !snapshot.value("question", Json()).is_string() ||
            snapshot.at("question").get<std::string>().empty() || answer.is_null() ||
            (answer.is_string() && answer.get<std::string>().empty())) return;
        Json identity = {{"question", snapshot.at("question")}};
        if (snapshot.contains("options")) identity["options"] = snapshot.at("options");
        evidence.push_back({{"id", id}, {"courseId", courseId}, {"questionId", questionIdentity(courseId, identity)},
            {"question", snapshot}, {"answer", answer}, {"unknown", unknown}, {"assisted", assisted},
            {"hintHistory", hints}, {"at", at}, {"topic", topic}});
    };
    for (const auto& row : interactions) {
        if (!row.courseId || !allowed(db, access, *row.courseId)) continue;
        const auto item = parse(row.payload);
        if (row.kind == "practice" && item.value("source", "") == "ai" && item.value("status", "") == "valid" && item.value("credible", false)) {
            const auto input = actualInputs.find(item.value("interactionId", ""));
            if (input == actualInputs.end() || input->second.value("action", "") == "skip" || input->second.value("action", "") == "stop" ||
                item.value("response", Json()) != input->second.value("text", Json()) ||
                item.value("questionSnapshot", Json()) != input->second.value("questionSnapshot", Json())) continue;
            append(row.id, *row.courseId, item.value("questionSnapshot", Json()), item.value("response", Json()), row.createdAt,
                input->second.value("action", "") == "unknown", item.value("assisted", false), item.value("hintHistory", Json::array()), item.value("topic", ""));
        } else if (row.kind == "quiz" && item.value("results", Json()).is_array()) {
            size_t index = 0;
            for (const auto& answer : item.at("results")) {
                if (answer.value("answered", false) && answer.value("credible", false))
                    append(row.id + ":" + std::to_string(index), *row.courseId, answer.value("questionSnapshot", Json()),
                        answer.value("givenAnswer", Json()), row.createdAt, answer.value("unknown", false), answer.value("assisted", false),
                        answer.value("hintHistory", Json::array()), item.value("topic", ""));
                ++index;
            }
        }
    }
    for (const auto& courseId : access.courseIds) if (allowed(db, access, courseId)) {
        for (const auto& row : db.listClassroomActivities(courseId)) {
            if (row.kind != "diagnostic" && row.kind != "interaction" && row.kind != "question") continue;
            const auto item = parse(row.payload);
            if (item.value("status", "") != "answered" || !item.value("credible", false) ||
                item.value("evaluationStatus", "valid") == "failed" || item.value("evaluationStatus", "valid") == "pending") continue;
            const auto snapshot = item.contains("questionSnapshot") ? item.at("questionSnapshot") : questionSnapshot(item);
            append(row.id, courseId, snapshot, item.value("answer", Json()), row.updatedAt, item.value("unknown", false),
                item.value("assisted", false), item.value("hintHistory", Json::array()), item.value("topic", "课堂任务"));
        }
    }
    return evidence;
}

void registerAgentProfileTools(LearningAgent& agent) {
    agent.registerTool("update_topic_mastery", {"AI 判断具体知识点的证据充分性与掌握情况，不使用三题门槛。参数 courseId,topicId,score:0..100或null,sufficient:bool,evidenceIds,rationale,recommendation,uncertainty,weakPoints:[]可选,nextReviewAt:日期或空。引用 read_profile_evidence 中同一课程的实际证据。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto access = accessFor(task); const auto courseId = text(args, "courseId");
            if (!allowed(db, access, courseId)) throw std::invalid_argument("无权评估该课程");
            auto evidence = agentProfileEvidence(db, access); Json relevant = Json::array();
            for (const auto& item : evidence) if (item.at("courseId") == courseId) relevant.push_back(item);
            const auto rating = checked(args, relevant); const auto outline = agentOutline(db, access, courseId); Json topic;
            for (const auto& stage : outline.at("courseStructure")) for (const auto& item : stage.at("topics")) if (item.at("id") == text(args, "topicId")) topic = item;
            if (topic.empty()) throw std::invalid_argument("评估知识点不存在");
            TopicMastery value; value.courseId = courseId; value.topic = topic.at("title"); value.phaseIndex = topic.at("legacyPhaseIndex");
            value.score = rating.score; value.rationale = rating.rationale + "；不确定性：" + rating.uncertainty; value.recommendation = rating.recommendation;
            const auto weakPoints = args.value("weakPoints", Json::array()); if (!weakPoints.is_array()) throw std::invalid_argument("薄弱点应为文字列表");
            for (const auto& point : weakPoints) if (!point.is_string()) throw std::invalid_argument("薄弱点应为文字");
            value.weakPoints = weakPoints.dump(); value.evidenceIds = rating.references.dump(); value.evidenceCount = rating.distinctQuestions;
            value.nextReviewAt = args.value("nextReviewAt", ""); value.model = source.model; value.status = rating.score ? "ready" : "insufficient";
            value.updatedAt = now(); value.version = db.profileRevision();
            return PreparedAgentTool{{{"topicUpdated", true}, {"evidenceCount", rating.distinctQuestions}}, [value](Database& connection) {
                if (connection.profileRevision() != value.version) throw std::invalid_argument("知识点评估证据已变化");
                if (!connection.upsert(value)) throw std::runtime_error("知识点画像保存失败");
            }};
        }});
    agent.registerTool("read_profile_evidence", {"读取有权限的真实原题、实际回答、不会反馈、提示经历和评价来源。返回稳定题目标识，同题多轮仍是一道题；漏答、跳过与失败不纳入。画像充分性、适用维度和评分由 AI 判断，不要求三道题。", true,
        [](Database& db, const Json&, const Json& task, const AIResult&) { return PreparedAgentTool{agentProfileEvidence(db, accessFor(task)), {}}; }});
    agent.registerTool("update_profiles", {"由真实 AI 更新学科和学习能力画像。参数 subjects:[{subject,score:0..100或null,sufficient:bool,rationale,recommendation,uncertainty,evidenceIds,weakPoints:[]}],abilities:[{id:memory|understanding|application|reasoning|expression|transfer,score,sufficient,rationale,recommendation,uncertainty,evidenceIds}]。可只更新相关维度，保留其他真实结果。每项正式评分引用 read_profile_evidence 的实际记录ID，由你判断证据是否充分及题目是否适用；不机械复制正确率或学科分数，不把教师答案、浏览及完成标记当证据。维度含义：事实回忆、概念解释、已知方法解题、有依据的推导、说明过程、新情境运用。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto evidence = agentProfileEvidence(db, accessFor(task)); const int revision = db.profileRevision();
            const auto rawAbilities = db.profileMeta("ability-profile"), rawEvidence = db.profileMeta("agent-profile-evidence");
            auto oldAbilities = parse(rawAbilities); auto references = parse(rawEvidence);
            if (!oldAbilities.is_object()) oldAbilities = Json::object();
            if (!references.is_object()) references = Json::object();
            Json dimensions = Json::array();
            for (size_t index = 0; index < abilities.size(); ++index) {
                Json value = {{"id", abilities[index]}, {"name", names[index]}, {"score", nullptr}, {"evidenceCount", 0},
                    {"rationale", "等待 AI 根据真实作答评估。"}, {"recommendation", ""}, {"updatedAt", ""}, {"scope", "全部课程中已测任务的表现"}};
                for (const auto& previous : oldAbilities.value("dimensions", Json::array())) if (previous.value("id", "") == abilities[index]) value = previous;
                dimensions.push_back(value);
            }
            const auto requestedSubjects = args.value("subjects", Json::array()), requestedAbilities = args.value("abilities", Json::array());
            if (!requestedSubjects.is_array() || !requestedAbilities.is_array() || (requestedSubjects.empty() && requestedAbilities.empty()))
                throw std::invalid_argument("至少提供一项 AI 画像判断");
            std::vector<SubjectMastery> values; std::set<std::string> seen;
            for (const auto& item : requestedSubjects) {
                const auto subject = text(item, "subject");
                if (std::find(subjects.begin(), subjects.end(), subject) == subjects.end() || !seen.insert(subject).second)
                    throw std::invalid_argument("学科范围或重复项无效");
                const auto rating = checked(item, evidence);
                const auto weakPoints = item.value("weakPoints", Json::array());
                if (!weakPoints.is_array()) throw std::invalid_argument("薄弱点应为文字列表");
                for (const auto& point : weakPoints) if (!point.is_string()) throw std::invalid_argument("薄弱点格式无效");
                SubjectMastery value; value.subject = subject; value.score = rating.score;
                value.rationale = rating.rationale + "；评估范围与不确定性：" + rating.uncertainty;
                value.recommendation = rating.recommendation; value.weakPoints = weakPoints.dump();
                value.evidenceCount = rating.distinctQuestions; value.model = source.model; value.updatedAt = now();
                value.status = rating.score ? "ready" : "insufficient"; values.push_back(value);
                references["subjects"][subject] = {{"evidenceIds", rating.references}, {"uncertainty", rating.uncertainty}, {"model", source.model}, {"at", value.updatedAt}};
            }
            seen.clear();
            for (const auto& item : requestedAbilities) {
                const auto id = text(item, "id"); const auto position = std::find(abilities.begin(), abilities.end(), id);
                if (position == abilities.end() || !seen.insert(id).second) throw std::invalid_argument("能力维度范围或重复项无效");
                const auto rating = checked(item, evidence); auto& value = dimensions[static_cast<size_t>(position - abilities.begin())];
                value["score"] = rating.score ? Json(*rating.score) : Json(); value["rationale"] = rating.rationale;
                value["recommendation"] = rating.recommendation; value["uncertainty"] = rating.uncertainty;
                value["evidenceCount"] = rating.distinctQuestions; value["evidenceIds"] = rating.references;
                value["model"] = source.model; value["updatedAt"] = now();
                references["abilities"][id] = {{"evidenceIds", rating.references}, {"uncertainty", rating.uncertainty}, {"model", source.model}, {"at", value["updatedAt"]}};
            }
            auto updatedAbilities = oldAbilities; updatedAbilities["dimensions"] = dimensions;
            updatedAbilities["version"] = revision; updatedAbilities["attemptVersion"] = revision; updatedAbilities["pendingVersion"] = revision;
            updatedAbilities["model"] = source.model; updatedAbilities["updatedAt"] = now(); updatedAbilities["error"] = "";
            return PreparedAgentTool{{{"subjectsUpdated", values.size()}, {"abilitiesUpdated", requestedAbilities.size()}, {"source", "ai"}},
                [values, requestedAbilities, updatedAbilities, references, rawAbilities, rawEvidence, revision](Database& connection) {
                    if (connection.profileRevision() != revision || connection.profileMeta("agent-profile-evidence") != rawEvidence ||
                        (!requestedAbilities.empty() && connection.profileMeta("ability-profile") != rawAbilities))
                        throw std::invalid_argument("画像证据或其他窗口结果已有更新，旧评分不应用");
                    for (const auto& value : values) if (!connection.upsert(value)) throw std::runtime_error("学科画像保存失败");
                    if (!requestedAbilities.empty()) connection.setProfileMeta("ability-profile", updatedAbilities.dump());
                    connection.setProfileMeta("agent-profile-evidence", references.dump());
                    connection.setProfileAssessed(revision); connection.setProfileError("");
                }};
        }});
}
}

#include "profile_service.hpp"

#include "ai_client.hpp"
#include "db.hpp"
#include "json_fix.hpp"
#include "text_utils.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <set>
#include <map>
#include <tuple>
#include <sstream>
#include <stdexcept>

namespace gangyi {
namespace {
using json = nlohmann::json;

std::string nowIso8601() {
    const std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

std::string shortText(const std::string& input, size_t limit) {
    return truncateUtf8(input, limit);
}

json interactionSignal(const LearningInteraction& item) {
    const json payload = json::parse(item.payload, nullptr, false);
    json signal = {{"kind", item.kind}, {"subject", item.subject.value_or("")},
        {"date", item.createdAt.substr(0, std::min<size_t>(10, item.createdAt.size()))}};
    if (!payload.is_object()) return signal;
    if (item.kind == "quiz") {
        signal["score"] = payload.value("score", 0);
        signal["total"] = payload.value("total", 0);
        signal["topic"] = shortText(payload.value("topic", ""), 100);
    } else if (item.kind == "chat-user") {
        signal["topic"] = shortText(payload.value("topic", ""), 100);
        signal["questionDigest"] = shortText(payload.value("text", ""), 120);
    } else {
        signal["topic"] = shortText(payload.value("topic", ""), 100);
        signal["state"] = shortText(payload.value("state", ""), 30);
    }
    return signal;
}
}

json recentCourses(Database& db) {
    struct Activity {
        std::string time, topicTime, topic;
        int phaseIndex = 0, topicIndex = 0;
    };
    std::map<std::string, Activity> activities;
    const auto updateActivity = [&](const std::string& courseId, const std::string& time,
                                    const std::string& topic, int phase, int index) {
        if (courseId.empty() || time.empty()) return;
        auto& current = activities[courseId];
        current.time = std::max(current.time, time);
        // 打开课程目录也更新排序，但不能抹掉上一节实际学习的课时。
        if ((!topic.empty() || (phase > 0 && index > 0)) && time >= current.topicTime) {
            current.topicTime = time;
            current.topic = topic;
            current.phaseIndex = phase;
            current.topicIndex = index;
        }
    };
    for (const auto& item : db.listInteractions()) {
        const auto payload = json::parse(item.payload, nullptr, false);
        if (!payload.is_object()) continue;
        const std::string topic = payload.contains("topic") && payload["topic"].is_string() ?
            payload["topic"].get<std::string>() : "";
        const int phase = payload.contains("phaseIndex") && payload["phaseIndex"].is_number_integer() ?
            payload["phaseIndex"].get<int>() : 0;
        const int index = payload.contains("topicIndex") && payload["topicIndex"].is_number_integer() ?
            payload["topicIndex"].get<int>() : 0;
        updateActivity(item.courseId.value_or(""), item.createdAt, topic, phase, index);
    }
    auto courses = db.listCourses();
    courses.erase(std::remove_if(courses.begin(), courses.end(), [](const Course& item) {
        return item.status != "active";
    }), courses.end());
    for (const auto& course : courses) {
        for (const auto& item : db.listClassroomActivities(course.id))
            updateActivity(course.id, item.updatedAt, "", item.phaseIndex, item.topicIndex);
    }
    const auto activityTime = [&](const Course& course) {
        const auto found = activities.find(course.id);
        return found == activities.end() ? course.createdAt : std::max(course.createdAt, found->second.time);
    };
    std::sort(courses.begin(), courses.end(), [&](const Course& left, const Course& right) {
        const auto leftTime = activityTime(left), rightTime = activityTime(right);
        if (leftTime != rightTime) return leftTime > rightTime;
        if (left.createdAt != right.createdAt) return left.createdAt > right.createdAt;
        return left.id < right.id;
    });
    if (courses.size() > 3) courses.resize(3);
    json result = json::array();
    for (const auto& course : courses) {
        const auto found = activities.find(course.id);
        const Activity activity = found == activities.end() ? Activity{} : found->second;
        int total = 0, done = 0;
        std::string latestTopic;
        const auto snapshots = db.findSnapshotsByCourseId(course.id);
        const auto payload = snapshots.empty() ? json::object() : json::parse(snapshots.back().payload, nullptr, false);
        if (payload.is_object() && payload.contains("courseStructure") && payload["courseStructure"].is_array()) {
            int phase = 0;
            for (const auto& stage : payload["courseStructure"]) {
                ++phase;
                if (!stage.is_object() || !stage.contains("topics") || !stage["topics"].is_array()) continue;
                int index = 0;
                for (const auto& topic : stage["topics"]) {
                    ++index;
                    if (!topic.is_string()) continue;
                    ++total;
                    const auto progress = db.findLearningCardProgress(course.id, phase, index);
                    if (progress && progress->status == "completed") ++done;
                    const auto title = topic.get<std::string>();
                    if ((phase == activity.phaseIndex && index == activity.topicIndex) || title == activity.topic)
                        latestTopic = title;
                }
            }
        }
        result.push_back({{"courseId", course.id}, {"title", course.title.empty() ? course.goal : course.title},
            {"goal", course.goal}, {"mode", course.mode}, {"createdAt", course.createdAt},
            {"lastActivityAt", activity.time}, {"latestTopic", latestTopic},
            {"totalTopics", total}, {"doneTopics", done}, {"percent", total ? done * 100 / total : 0}});
    }
    return result;
}

json profileEvidenceSummary(Database& db) {
    json summary = {{"courses", json::array()}, {"interactions", json::array()}};
    for (const auto& course : db.listCourses()) {
        if (course.status != "active") continue;
        json row = {{"id", course.id}, {"title", shortText(course.title, 120)}, {"goal", shortText(course.goal, 160)},
            {"updatedAt", course.updatedAt}};
        if (const auto progress = db.findProgressByCourseId(course.id)) {
            row["progress"] = {{"completed", progress->completedCount}, {"total", progress->totalCount},
                {"percent", progress->overallPercent}};
        }
        summary["courses"].push_back(std::move(row));
    }
    const auto interactions = db.listInteractions();
    const size_t start = interactions.size() > 100 ? interactions.size() - 100 : 0;
    for (size_t i = start; i < interactions.size(); ++i) {
        if (interactions[i].kind == "chat-assistant") continue;
        summary["interactions"].push_back(interactionSignal(interactions[i]));
    }
    summary["courseCount"] = summary["courses"].size();
    summary["interactionCount"] = summary["interactions"].size();
    summary["topicStates"] = json::array();
    for (const auto& value : db.listTopicMastery()) {
        if (!value.score) continue;
        summary["topicStates"].push_back({{"courseId", value.courseId}, {"phaseIndex", value.phaseIndex},
            {"topic", value.topic}, {"score", *value.score}, {"evidenceCount", value.evidenceCount},
            {"rationale", value.rationale}, {"evidenceIds", json::parse(value.evidenceIds, nullptr, false)}});
    }
    return summary;
}

json profileView(Database& db) {
    json subjects = json::array();
    for (const auto& value : db.listMastery()) {
        json weakPoints = json::parse(value.weakPoints, nullptr, false);
        if (!weakPoints.is_array()) weakPoints = json::array();
        subjects.push_back({{"subject", value.subject},
            {"score", value.score && value.status == "ready" ? json(*value.score) : json(nullptr)},
            {"historicalScore", value.score && value.status == "legacy" ? json(*value.score) : json(nullptr)},
            {"status", value.status},
            {"rationale", value.rationale}, {"weakPoints", weakPoints},
            {"recommendation", value.recommendation}, {"evidenceCount", value.evidenceCount},
            {"model", value.model}, {"updatedAt", value.updatedAt}});
    }
    const auto courses = db.listCourses();
    return {{"subjects", subjects}, {"updating", db.profileDirty()}, {"error", db.profileError()},
        {"searchStatus", db.profileSearchStatus()},
        {"sources", json::parse(db.profileSources(), nullptr, false)},
        {"profileVersion", db.profileAssessedRevision()},
        {"hasEvidence", std::any_of(courses.begin(), courses.end(), [](const auto& course) { return course.status == "active"; }) ||
            !db.listInteractions().empty()}};
}

std::string profileContext(Database& db) {
    json context = {{"subjects", json::array()}, {"topics", json::array()}};
    const int assessedVersion = db.profileAssessedRevision();
    for (const auto& value : db.listMastery()) {
        if (!value.score || value.status != "ready") continue;
        context["subjects"].push_back({{"subject", value.subject}, {"strength", *value.score},
            {"weakPoints", json::parse(value.weakPoints, nullptr, false)},
            {"recommendation", value.recommendation}});
    }
    for (const auto& value : db.listTopicMastery()) {
        if (!value.score || value.version > assessedVersion || context["topics"].size() >= 12) continue;
        context["topics"].push_back({{"courseId", value.courseId}, {"phaseIndex", value.phaseIndex},
            {"topic", shortText(value.topic, 80)}, {"score", *value.score},
            {"recommendation", shortText(value.recommendation, 120)}});
    }
    while (context.dump().size() > 3500 && !context["topics"].empty()) context["topics"].erase(context["topics"].end() - 1);
    while (context.dump().size() > 3500 && !context["subjects"].empty()) context["subjects"].erase(context["subjects"].end() - 1);
    return context.dump();
}

bool refreshTopicMastery(Database& db, AIClient& ai, std::string& error) {
    using Key = std::tuple<std::string, int, std::string>;
    std::map<Key, json> groups;
    std::map<Key, std::set<std::string>> ids;
    for (const auto& item : db.listInteractions()) {
        if (!item.courseId || (item.kind != "quiz" && item.kind != "practice" && item.kind != "review")) continue;
        const json payload = json::parse(item.payload, nullptr, false);
        if (!payload.is_object() || !payload.value("topic", json()).is_string()) continue;
        const std::string topic = shortText(payload.value("topic", ""), 120);
        const int phase = payload.value("phaseIndex", 0);
        if (topic.empty() || phase < 1) continue;
        const Key key{*item.courseId, phase, topic};
        json event = {{"id", item.id}, {"kind", item.kind}};
        if (item.kind == "quiz") {
            if (!payload.contains("results") || !payload["results"].is_array()) continue;
            event["results"] = json::array();
            for (const auto& result : payload["results"]) {
                if (!result.is_object() || !result.value("answered", false)) continue;
                event["results"].push_back({{"questionIndex", result.value("questionIndex", 0)},
                    {"correct", result.value("correct", false)}, {"unknown", result.value("unknown", false)}});
            }
        } else event["state"] = shortText(payload.value("state", ""), 30);
        groups[key].push_back(std::move(event));
        ids[key].insert(item.id);
    }
    const int revision = db.profileRevision();
    for (const auto& [key, events] : groups) {
        const auto& [courseId, phase, topic] = key;
        TopicMastery state;
        state.courseId = courseId; state.phaseIndex = phase; state.topic = topic;
        state.version = revision; state.updatedAt = nowIso8601();
        state.evidenceCount = static_cast<int>(events.size());
        int answers = 0;
        for (const auto& event : events) if (event.value("kind", "") == "quiz")
            answers += static_cast<int>(event["results"].size());
        if (answers < 3) {
            state.rationale = "数据不足：至少需要 3 道已作答测验题。";
            db.upsert(state);
            continue;
        }
        try {
            ChatOptions options;
            options.messages = {
                {"system", u8"你是学习诊断教师。只能根据逐题正确情况判断当前主题掌握度；普通浏览和课程完成率不是证据。unknown=true 表示学习者明确反馈暂时不会，是入门诊断反馈，不是漏答或选错某个选项；依据说明必须区分这些情况。建议从基础概念开始，不得把单个主题的入门诊断外推为整门学科能力。只输出 JSON：{\"score\":0到100整数,\"rationale\":\"具体依据\",\"weakPoints\":[\"薄弱点\"],\"recommendation\":\"下一步动作\",\"evidenceIds\":[\"所引用的真实记录ID\"],\"nextReviewAt\":\"YYYY-MM-DD 或空字符串\"}。不得引用输入中不存在的记录 ID。", ""},
                {"user", json{{"courseId", courseId}, {"phaseIndex", phase}, {"topic", topic}, {"events", events}}.dump(), ""}
            };
            options.temperature = 0.1; options.maxTokens = 900; options.responseFormat = "json_object";
            options.timeoutMs = 45000; options.maxAttempts = 1;
            options.searchQuery = topic;
            const auto response = ai.chat(options);
            db.setProfileSearch(response.searchStatus, json(response.sources).dump());
            const json result = parseAIJson(response.content);
            if (!result.is_object() || !result.value("score", json()).is_number_integer() ||
                !result.value("rationale", json()).is_string() || !result.value("recommendation", json()).is_string() ||
                !result.value("weakPoints", json()).is_array() || !result.value("evidenceIds", json()).is_array())
                throw std::runtime_error("主题评估字段无效");
            const int score = result["score"].get<int>();
            if (score < 0 || score > 100 || result["evidenceIds"].empty() || result["evidenceIds"].size() > ids[key].size())
                throw std::runtime_error("主题评估分数或证据无效");
            std::set<std::string> cited;
            for (const auto& citedId : result["evidenceIds"]) {
                if (!citedId.is_string() || !ids[key].count(citedId.get<std::string>()) ||
                    !cited.insert(citedId.get<std::string>()).second) throw std::runtime_error("主题评估引用了无效记录");
            }
            for (const auto& point : result["weakPoints"]) if (!point.is_string()) throw std::runtime_error("薄弱点格式无效");
            const std::string review = result.value("nextReviewAt", "");
            if (!review.empty() && (review.size() != 10 || review[4] != '-' || review[7] != '-'))
                throw std::runtime_error("复习日期格式无效");
            state.score = score; state.status = "ready";
            state.rationale = shortText(result["rationale"].get<std::string>(), 500);
            state.weakPoints = result["weakPoints"].dump();
            state.recommendation = shortText(result["recommendation"].get<std::string>(), 300);
            state.evidenceIds = result["evidenceIds"].dump();
            state.nextReviewAt = review; state.model = response.model;
            db.upsert(state);
        } catch (const std::exception& exception) {
            error = exception.what();
            return false;
        }
    }
    return true;
}

json nextLearning(Database& db, const std::string& courseId) {
    const auto states = db.listTopicMastery();
    const TopicMastery* chosen = nullptr;
    const std::string today = nowIso8601().substr(0, 10);
    const auto progress = db.findProgressByCourseId(courseId);
    int best = -100000;
    std::map<std::pair<int, std::string>, std::pair<std::string, std::string>> feedback;
    std::map<std::pair<int, std::string>, int> topicIndices;
    for (const auto& event : db.listInteractions()) {
        if (event.courseId.value_or("") != courseId) continue;
        const json payload = json::parse(event.payload, nullptr, false);
        if (!payload.is_object() || !payload.value("topic", json()).is_string() ||
            !payload.value("phaseIndex", json()).is_number_integer()) continue;
        const auto key = std::make_pair(payload["phaseIndex"].get<int>(), payload["topic"].get<std::string>());
        if (payload.value("topicIndex", json()).is_number_integer()) topicIndices[key] = payload["topicIndex"].get<int>();
        if (event.kind == "practice" || event.kind == "review") feedback[key] = {payload.value("state", ""), event.id};
    }
    for (const auto& value : states) {
        if (value.courseId != courseId || !value.score) continue;
        const auto key = std::make_pair(value.phaseIndex, value.topic);
        const std::string state = feedback[key].first;
        if (state == "unsuitable") continue;
        const int priority = (value.nextReviewAt.empty() || value.nextReviewAt > today ? 0 : 1000) +
            100 - *value.score + (state == "too_hard" ? 20 : state == "too_easy" ? -20 : 0) +
            (progress && progress->lastPhaseIndex == value.phaseIndex ? 10 : 0);
        if (priority > best) { best = priority; chosen = &value; }
    }
    if (!chosen) return {{"status", "insufficient"}, {"reason", "数据不足：尚无可靠的逐题测验评估。"}};
    const auto key = std::make_pair(chosen->phaseIndex, chosen->topic);
    const auto [state, feedbackId] = feedback[key];
    std::string action = chosen->recommendation;
    if (state == "too_hard") action = "先补讲基础并完成一组较简单的练习；" + action;
    if (state == "too_easy") action = "尝试进阶迁移练习；" + action;
    return {{"status", "ready"}, {"courseId", courseId}, {"phaseIndex", chosen->phaseIndex},
        {"topicIndex", std::max(1, topicIndices[key])}, {"topic", chosen->topic}, {"action", action}, {"score", *chosen->score},
        {"reason", chosen->rationale + (feedbackId.empty() ? "" : "；反馈记录 " + feedbackId)},
        {"evidenceIds", json::parse(chosen->evidenceIds, nullptr, false)},
        {"nextReviewAt", chosen->nextReviewAt}};
}

bool refreshProfile(Database& db, AIClient& ai, std::string& error) {
    const int revision = db.profileRevision();
    const json evidence = profileEvidenceSummary(db);
    if (evidence["topicStates"].empty()) {
        db.setProfileAssessed(revision);
        db.setProfileError("");
        return true;
    }
    try {
        ChatOptions options;
        options.messages = {
            {"system", u8"你是高中学习画像评估 AI。仅根据已验证的 topicStates 判断学科掌握度；课程目标仅用于识别学科，不以课程进度或对话当作掌握证据。无可靠主题评分时 score 为 null。只返回 JSON：{\"subjects\":[{\"subject\":\"数学\",\"score\":72,\"rationale\":\"依据说明\",\"weakPoints\":[\"知识点\"],\"recommendation\":\"下一步建议\",\"evidenceCount\":3,\"evidenceIds\":[\"真实记录ID\"]}]}。每个有分数的学科必须引用 topicStates 中真实 evidenceIds。", ""},
            {"user", json{{"courses", evidence["courses"]}, {"topicStates", evidence["topicStates"]}}.dump(), ""},
        };
        options.temperature = 0.2;
        options.maxTokens = 2400;
        options.timeoutMs = 45000;
        options.maxAttempts = 1;
        options.searchQuery = evidence["topicStates"].at(0).value("topic", "高中学科知识");
        const auto result = ai.chat(options);
        db.setProfileSearch(result.searchStatus, json(result.sources).dump());
        const json parsed = parseAIJson(result.content);
        if (!parsed.is_object() || !parsed.contains("subjects") || !parsed["subjects"].is_array() || parsed["subjects"].size() > 12)
            throw std::runtime_error("画像格式无效");
        std::vector<SubjectMastery> values;
        std::set<std::string> seen;
        for (const auto& item : parsed["subjects"]) {
            if (!item.is_object() || !item.value("subject", json()).is_string()) throw std::runtime_error("学科格式无效");
            SubjectMastery value;
            value.subject = shortText(item["subject"].get<std::string>(), 40);
            if (value.subject.empty() || !seen.insert(value.subject).second) throw std::runtime_error("学科重复或为空");
            if (item.contains("score") && !item["score"].is_null()) {
                if (!item["score"].is_number_integer()) throw std::runtime_error("掌握强度格式无效");
                const int score = item["score"].get<int>();
                if (score < 0 || score > 100) throw std::runtime_error("掌握强度超出范围");
                value.score = score;
            }
            if (value.score) {
                if (!item.value("evidenceIds", json()).is_array() || item["evidenceIds"].empty())
                    throw std::runtime_error("画像缺少记录依据");
                std::set<std::string> available;
                for (const auto& state : evidence["topicStates"]) if (state["evidenceIds"].is_array())
                    for (const auto& id : state["evidenceIds"]) if (id.is_string()) available.insert(id.get<std::string>());
                for (const auto& id : item["evidenceIds"]) if (!id.is_string() || !available.count(id.get<std::string>()))
                    throw std::runtime_error("画像引用了无效记录");
            }
            value.rationale = shortText(item.value("rationale", ""), 500);
            if (value.score) value.rationale += " [" + item["evidenceIds"].dump() + "]";
            value.recommendation = shortText(item.value("recommendation", ""), 500);
            const json weakPoints = item.value("weakPoints", json::array());
            if (!weakPoints.is_array()) throw std::runtime_error("薄弱点格式无效");
            value.weakPoints = weakPoints.dump();
            value.evidenceCount = value.score ? static_cast<int>(item["evidenceIds"].size()) : 0;
            value.model = result.model;
            value.status = value.score ? "ready" : "insufficient";
            value.updatedAt = nowIso8601();
            values.push_back(std::move(value));
        }
        db.replaceMastery(values);
        db.setProfileAssessed(revision);
        db.setProfileError("");
        return true;
    } catch (const std::exception& exception) {
        error = exception.what();
        db.setProfileError(u8"画像暂未更新，请检查 AI 配置或稍后重试。");
        return false;
    }
}
}

#include "classroom_service.hpp"
#include "ai_client.hpp"
#include "course_service.hpp"
#include "json_fix.hpp"
#include "question_evidence.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <stdexcept>

namespace gangyi {
namespace {

// ponytail: 单实例全局锁串行化课堂题生成；并发课程增多时改为按课时锁。
std::mutex classroomMutex;
// 周计划写入与预览确认使用同一把锁，避免跨窗口覆盖新版本。
std::recursive_mutex weeklyMutex;
struct WeeklyProposal {
    Json plan;
    int version;
    std::string id, message, source;
    std::chrono::steady_clock::time_point expires;
};
std::map<std::string, WeeklyProposal> weeklyProposals;
std::atomic_uint64_t proposalSequence{0};

std::string now() {
    const auto clock = std::chrono::system_clock::now();
    const auto value = std::chrono::system_clock::to_time_t(clock);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &value);
#else
    gmtime_r(&value, &utc);
#endif
    std::ostringstream result;
    result << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return result.str();
}

std::string addDays(const std::string& date, int days) {
    std::tm value{};
    std::istringstream input(date);
    input >> std::get_time(&value, "%Y-%m-%d");
    if (input.fail() || date.size() != 10) throw std::invalid_argument("日期格式无效");
    value.tm_hour = 12;
    value.tm_mday += days;
    std::mktime(&value);
    std::ostringstream output;
    output << std::put_time(&value, "%Y-%m-%d");
    return output.str();
}

std::string itemId(const std::string& key, const std::string& kind, int index) {
    return key + ":" + kind + ":" + std::to_string(index);
}

Json readPayload(const ClassroomActivity& row) {
    auto parsed = Json::parse(row.payload, nullptr, false);
    return parsed.is_object() ? parsed : Json::object();
}

void save(Database& db, const std::string& key, const std::string& courseId, int phaseIndex,
          int topicIndex, const std::string& kind, const Json& payload) {
    if (!db.upsert(ClassroomActivity{key, courseId, kind, payload.dump(), now(), phaseIndex, topicIndex}))
        throw std::runtime_error("课堂活动保存失败");
}

Json savedQuestion(Database& db, const std::string& key, const std::string& kind, int index) {
    const auto row = db.getClassroomActivity(itemId(key, kind, index));
    return row ? readPayload(*row) : Json();
}

Json questionsFromAI(const std::string& topic, const std::string& kind) {
    ChatOptions options;
    options.temperature = 0.3;
    options.maxTokens = 1800;
    options.timeoutMs = 18000;
    options.maxAttempts = 1;
    options.messages.push_back({"system", u8"你是高中课堂出题教师。只输出 JSON 对象，不输出 Markdown。选择题须四个选项且只有一个正确答案，不能通过措辞泄露答案。", ""});
    if (kind == "diagnostic") {
        options.messages.push_back({"user", u8"围绕主题“" + topic + u8"”生成3道简短、难度递进的诊断选择题。JSON格式：{\"questions\":[{\"question\":\"\",\"options\":[\"\",\"\",\"\",\"\"],\"answerIndex\":0,\"explanation\":\"\"}]}。第三题仅用于前两题结果不明确时。", ""});
    } else {
        options.messages.push_back({"user", u8"围绕主题“" + topic + u8"”生成三项课堂活动，依次是预测选择题、用自己的话讲解的开放题、新情境应用选择题。JSON格式：{\"questions\":[{\"question\":\"\",\"type\":\"choice\",\"options\":[\"\",\"\",\"\",\"\"],\"answerIndex\":0,\"rubric\":\"\",\"explanation\":\"\"},{\"question\":\"\",\"type\":\"open\",\"rubric\":\"具体评分依据\",\"explanation\":\"\"},{\"question\":\"\",\"type\":\"choice\",\"options\":[\"\",\"\",\"\",\"\"],\"answerIndex\":0,\"rubric\":\"\",\"explanation\":\"\"}]}。", ""});
    }
    AIClient ai;
    Json generated = parseAIJson(ai.chat(options).content);
    if (!generated.is_object() || !generated.contains("questions") || !generated["questions"].is_array() || generated["questions"].size() != 3)
        throw std::runtime_error("课堂题格式无效");
    for (size_t i = 0; i < 3; ++i) {
        auto& item = generated["questions"][i];
        if (!item.is_object() || !item.value("question", Json()).is_string() || item.value("question", "").empty())
            throw std::runtime_error("课堂题缺少题干");
        const bool open = kind == "interaction" && i == 1;
        if (open) {
            if (!item.value("rubric", Json()).is_string() || item.value("rubric", "").size() < 8)
                throw std::runtime_error("开放题评分依据无效");
        } else if (!item.value("options", Json()).is_array() || item["options"].size() != 4 ||
                   !item.value("answerIndex", Json()).is_number_integer() ||
                   item["answerIndex"].get<int>() < 0 || item["answerIndex"].get<int>() > 3) {
            throw std::runtime_error("选择题答案无效");
        }
        item["type"] = open ? "open" : "choice";
        item["status"] = "pending";
        item["evidenceSource"] = "ai-generated";
    }
    return generated["questions"];
}

Json immediateDiagnostics(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                          const std::string& topic) {
    if (const auto session = db.findLearningSession(courseId, phaseIndex, topicIndex)) {
        const Json content = Json::parse(session->content, nullptr, false);
        if (content.is_object() && content.value("blocks", Json()).is_object() &&
            content["blocks"].value("quiz", Json()).is_object()) {
            const Json quiz = content["blocks"]["quiz"].value("quiz", Json::array());
            if (quiz.is_array() && quiz.size() >= 3) {
                Json result = Json::array();
                for (int i = 0; i < 3; ++i) {
                    const auto& item = quiz[i];
                    if (!item.is_object() || !item.value("question", Json()).is_string() ||
                        !item.value("options", Json()).is_array() || item["options"].size() != 4 ||
                        !item.value("answerIndex", Json()).is_number_integer() ||
                        item["answerIndex"].get<int>() < 0 || item["answerIndex"].get<int>() > 3) break;
                    Json copy = item;
                    copy["type"] = "choice";
                    copy["status"] = "pending";
                    copy["evidenceSource"] = "saved-lesson-quiz";
                    result.push_back(copy);
                }
                if (result.size() == 3) return result;
            }
        }
    }
    std::string criteria = topic;
    if (const auto found = getCourseWithSnapshot(db, courseId)) {
        const Json roadmap = found->payload.value("roadmap", Json::array());
        if (roadmap.is_array() && phaseIndex >= 1 && phaseIndex <= static_cast<int>(roadmap.size()) &&
            roadmap[phaseIndex - 1].is_object()) {
            const auto& stage = roadmap[phaseIndex - 1];
            criteria += "；阶段目标：" + stage.value("goal", "") + "；检查点：" + stage.value("checkpoint", "");
        }
    }
    const std::vector<std::string> prompts = {
        "用一句话解释「" + topic + "」的含义和关键条件。",
        "举一个新例子，说明怎样应用「" + topic + "」。",
        "如果题目条件改变，你会如何重新判断「" + topic + "」？"
    };
    Json result = Json::array();
    for (const auto& prompt : prompts) result.push_back({{"question", prompt}, {"type", "open"},
        {"rubric", "依据学科知识评价准确性和关键条件；" + criteria}, {"status", "pending"},
        {"evidenceSource", "course-snapshot"}});
    return result;
}

Json gradeOpen(const Json& question, const std::string& answer) {
    ChatOptions options;
    options.temperature = 0.1;
    options.maxTokens = 450;
    options.timeoutMs = 15000;
    options.maxAttempts = 1;
    options.messages = {{"system", u8"严格依据评分标准评价学生答案。只返回 JSON：{\"correct\":true,\"confidence\":0.0,\"feedback\":\"具体错误或正确点\",\"followUp\":\"先追问的一个问题\"}。不确定时降低 confidence。", ""},
        {"user", Json{{"question", question.value("question", "")}, {"rubric", question.value("rubric", "")}, {"answer", answer}}.dump(), ""}};
    AIClient ai;
    return parseAIJson(ai.chat(options).content);
}

Json stateFor(Database& db, const std::string& key) {
    const auto row = db.getClassroomActivity(key + ":state");
    return row ? readPayload(*row) : Json{{"diagnosticMode", "pending"}, {"completed", false}};
}

Json normalizedTopics(const Json& raw) {
    Json topics = Json::array();
    if (!raw.is_array()) return topics;
    for (const auto& item : raw) {
        if (item.is_string()) topics.push_back(item);
        else if (item.is_object()) {
            const std::string title = item.value("title", item.value("name", ""));
            if (!title.empty()) topics.push_back(title);
        }
    }
    return topics;
}

Json suggestTopicOrder(const Json& topics, const Json& performance, bool* usedAI = nullptr) {
    if (usedAI) *usedAI = false;
    const Json names = normalizedTopics(topics);
    if (names.empty()) return topics;
    if (std::set<Json>(names.begin(), names.end()).size() != names.size()) return topics;
    try {
        ChatOptions options;
        options.temperature = 0.2;
        options.maxTokens = 350;
        options.timeoutMs = 10000;
        options.maxAttempts = 1;
        options.messages = {{"system", u8"你是排课助手，只输出 JSON：{\"topics\":[原主题名称,...]}。只能从给定主题中挑选和排序本周最多7项，不得改写主题或决定日期时长。优先考虑到期复习和近期薄弱。", ""},
            {"user", Json{{"topics", names}, {"performance", performance}}.dump(), ""}};
        AIClient ai;
        const Json reply = parseAIJson(ai.chat(options).content);
        const Json proposed = reply.at("topics");
        if (!proposed.is_array() || proposed.empty() || proposed.size() > 7) return topics;
        std::set<std::string> seen;
        for (const auto& item : proposed) {
            if (!item.is_string() || std::find(names.begin(), names.end(), item) == names.end() ||
                !seen.insert(item.get<std::string>()).second) return topics;
        }
        Json ordered = Json::array();
        for (const auto& name : proposed) {
            for (size_t i = 0; i < names.size(); ++i) if (names[i] == name) { ordered.push_back(topics[i]); break; }
        }
        // 模型只建议前几项，其余课时仍保留，不能丢失待学习内容。
        for (size_t i = 0; i < names.size(); ++i) if (!seen.count(names[i].get<std::string>())) ordered.push_back(topics[i]);
        if (usedAI) *usedAI = true;
        return ordered;
    } catch (...) { return topics; }
}

Json recentPerformance(Database& db, const std::string& courseId, const Json& reviews) {
    Json result = {{"dueReviews", reviews}, {"activities", Json::array()}, {"quizzes", Json::array()}};
    for (const auto& row : db.listClassroomActivities(courseId)) {
        if (row.kind != "diagnostic" && row.kind != "interaction") continue;
        const Json item = readPayload(row);
        if (item.value("status", "") == "answered") result["activities"].push_back({
            {"kind", row.kind}, {"phaseIndex", row.phaseIndex}, {"topicIndex", row.topicIndex},
            {"correct", item.value("correct", false)}, {"credible", item.value("credible", false)}});
    }
    for (const auto& row : db.listInteractions()) {
        if (row.courseId.value_or("") != courseId || row.kind != "quiz") continue;
        const Json quiz = Json::parse(row.payload, nullptr, false);
        if (quiz.is_object()) result["quizzes"].push_back({{"score", quiz.value("score", 0)},
            {"total", quiz.value("total", 0)}, {"topic", quiz.value("topic", "")},
            {"phaseIndex", quiz.value("phaseIndex", 0)}, {"topicIndex", quiz.value("topicIndex", 0)},
            {"unknownCount", quiz.value("unknownCount", 0)}, {"results", quiz.value("results", Json::array())},
            {"createdAt", row.createdAt}});
    }
    for (const char* key : {"activities", "quizzes"}) if (result[key].size() > 20)
        result[key].erase(result[key].begin(), result[key].end() - 20);
    return result;
}

// 客观作答优先于旧画像；无模型配置时也能完成真正的补弱排序。
Json prioritizeTopics(Database& db, const std::string& courseId, const Json& topics, const Json& performance) {
    std::map<std::string, bool> weak;
    const auto key = [](const Json& item) {
        return std::to_string(item.value("phaseIndex", 0)) + ":" +
            std::to_string(item.value("topicIndex", 0)) + ":" + item.value("title", item.value("topic", ""));
    };
    for (const auto& state : db.listTopicMastery()) if (state.courseId == courseId && state.score && state.evidenceCount > 0)
        for (const auto& topic : topics) if (topic.is_object() && topic.value("phaseIndex", 0) == state.phaseIndex &&
            topic.value("title", "") == state.topic) weak[key(topic)] = *state.score < 70;
    for (const auto& item : performance["activities"]) if (item.value("credible", false))
        for (const auto& topic : topics) if (topic.is_object() && topic.value("phaseIndex", 0) == item.value("phaseIndex", 0) &&
            topic.value("topicIndex", 0) == item.value("topicIndex", 0)) weak[key(topic)] = !item.value("correct", false);
    for (const auto& quiz : performance["quizzes"]) if (quiz.value("total", 0) > 0) {
        const auto results = quiz.value("results", Json::array());
        if (!results.empty() && std::none_of(results.begin(), results.end(), [](const Json& item) { return item.value("answered", false); })) continue;
        for (const auto& topic : topics) if (topic.is_object() && topic.value("phaseIndex", 0) == quiz.value("phaseIndex", 0) &&
            topic.value("topicIndex", 0) == quiz.value("topicIndex", 0))
            weak[key(topic)] = quiz.value("score", 0) * 10 < quiz.value("total", 0) * 7;
    }
    std::vector<Json> ordered(topics.begin(), topics.end());
    std::stable_sort(ordered.begin(), ordered.end(), [&](const Json& a, const Json& b) {
        return (a.is_object() && weak[key(a)]) > (b.is_object() && weak[key(b)]);
    });
    return Json(ordered);
}

std::string availableWeek(const Json& availability, const std::string& monday, const std::string& today) {
    for (const auto& slot : availability)
        if (addDays(monday, slot.at("weekday").get<int>() - 1) >= today) return monday;
    return addDays(monday, 7);
}

Json comparableEntries(Json entries) {
    for (auto& item : entries) item.erase("manual");
    return entries;
}

bool futureStarted(Database& db, const std::string& courseId, int phaseIndex, int topicIndex) {
    if (const auto visited = db.findProgressByCourseId(courseId))
        if (visited->lastPhaseIndex && visited->lastTopicIndex &&
            (*visited->lastPhaseIndex > phaseIndex ||
             (*visited->lastPhaseIndex == phaseIndex && *visited->lastTopicIndex > topicIndex))) return true;
    const auto sessions = db.listLearningSessions();
    for (const auto& row : sessions)
        if (row.courseId.value_or("") == courseId && (row.phaseIndex > phaseIndex ||
            (row.phaseIndex == phaseIndex && row.topicIndex > topicIndex))) return true;
    const auto progress = db.listLearningCardProgress();
    for (const auto& row : progress)
        if (row.courseId.value_or("") == courseId && row.status != "not_started" &&
            (row.phaseIndex > phaseIndex || (row.phaseIndex == phaseIndex && row.topicIndex > topicIndex))) return true;
    return false;
}

bool adjustFuturePath(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                      const std::string& direction) {
    if (futureStarted(db, courseId, phaseIndex, topicIndex)) return false;
    const auto found = getCourseWithSnapshot(db, courseId);
    if (!found) return false;
    Json payload = found->payload;
    if (!payload.value("courseStructure", Json()).is_array() || phaseIndex < 1 ||
        phaseIndex > static_cast<int>(payload["courseStructure"].size())) return false;
    auto& stage = payload["courseStructure"][phaseIndex - 1];
    if (!stage.is_object() || !stage.value("topics", Json()).is_array() ||
        topicIndex < 1 || topicIndex > static_cast<int>(stage["topics"].size())) return false;
    auto& topics = stage["topics"];
    const std::string current = topics[topicIndex - 1].is_string() ? topics[topicIndex - 1].get<std::string>() : "";
    if (current.empty()) return false;
    const bool skip = direction == "strong";
    if (skip && (topicIndex >= static_cast<int>(topics.size()) || stage.contains("prerequisites"))) return false;
    ChatOptions options;
    options.temperature = 0.1;
    options.maxTokens = 180;
    options.timeoutMs = 12000;
    options.maxAttempts = 1;
    options.messages = {{"system", u8"你是课程路径建议器。只输出 JSON：{\"action\":\"skip或insert\",\"topic\":\"主题名\"}。仅可对当前课后一个未来主题做操作。skip 时必须原样返回 nextTopic；insert 时给与 currentTopic 紧密相关的短补弱主题。", ""},
        {"user", Json{{"direction", direction}, {"currentTopic", current},
            {"nextTopic", skip ? topics[topicIndex].get<std::string>() : ""}}.dump(), ""}};
    AIClient ai;
    const Json proposal = parseAIJson(ai.chat(options).content);
    if (!proposal.is_object() || !proposal.value("topic", Json()).is_string()) return false;
    const std::string candidate = proposal.value("topic", "");
    if (candidate.empty() || candidate.size() > 100 || proposal.value("action", "") != (skip ? "skip" : "insert")) return false;
    if (skip) {
        if (candidate != topics[topicIndex].get<std::string>()) return false;
        topics.erase(topics.begin() + topicIndex);
        if (stage.value("topicIds", Json()).is_array() && topicIndex < static_cast<int>(stage["topicIds"].size()))
            stage["topicIds"].erase(stage["topicIds"].begin() + topicIndex);
    } else {
        if (std::find(topics.begin(), topics.end(), candidate) != topics.end()) return false;
        topics.insert(topics.begin() + topicIndex, candidate);
        if (stage.value("topicIds", Json()).is_array())
            stage["topicIds"].insert(stage["topicIds"].begin() + topicIndex,
                "topic-" + courseId + "-remedial-" + std::to_string(phaseIndex) + "-" + std::to_string(topicIndex));
    }
    if (payload.value("roadmap", Json()).is_array() && phaseIndex <= static_cast<int>(payload["roadmap"].size())) {
        auto& road = payload["roadmap"][phaseIndex - 1];
        if (road.is_object() && road.value("topics", Json()).is_array()) {
            auto& items = road["topics"];
            if (skip) {
                if (topicIndex < static_cast<int>(items.size()) && items[topicIndex] == candidate)
                    items.erase(items.begin() + topicIndex);
            } else if (topicIndex <= static_cast<int>(items.size())) items.insert(items.begin() + topicIndex, candidate);
        }
    }
    const auto snapshots = db.findSnapshotsByCourseId(courseId);
    const int version = snapshots.empty() ? 1 : snapshots.back().version + 1;
    payload["pathAdjustment"] = {{"direction", direction}, {"phaseIndex", phaseIndex},
        {"topicIndex", topicIndex}, {"topic", candidate}, {"fromVersion", version - 1}};
    return db.insert(CourseSnapshot{"", courseId, version, payload.dump(), now()});
}

bool validAvailability(const Json& availability) {
    if (!availability.is_array() || availability.empty() || availability.size() > 7) return false;
    std::set<int> days;
    for (const auto& slot : availability) {
        if (!slot.is_object() || !slot.value("weekday", Json()).is_number_integer() ||
            !slot.value("minutes", Json()).is_number_integer()) return false;
        const int day = slot["weekday"], minutes = slot["minutes"];
        if (day < 1 || day > 7 || minutes < 10 || minutes > 240 || !days.insert(day).second) return false;
    }
    return true;
}

bool validEntries(const Json& entries) {
    if (!entries.is_array() || entries.size() > 100) return false;
    for (const auto& entry : entries) {
        if (!entry.is_object() || !entry.value("date", Json()).is_string() ||
            !entry.value("minutes", Json()).is_number_integer() ||
            !entry.value("title", Json()).is_string()) return false;
        const int minutes = entry["minutes"];
        if (minutes < 5 || minutes > 240 || entry["title"].get<std::string>().size() > 300) return false;
        try { addDays(entry["date"].get<std::string>(), 0); } catch (...) { return false; }
    }
    return true;
}
}

std::string classroomKey(const std::string& courseId, int phaseIndex, int topicIndex) {
    return "classroom:" + courseId + ":" + std::to_string(phaseIndex) + ":" + std::to_string(topicIndex);
}

std::string diagnosticMode(const Json& answers, bool skipped) {
    if (skipped) return "full";
    if (!answers.is_array() || answers.size() < 2) return "pending";
    int correct = 0;
    bool uncertain = false;
    for (const auto& item : answers) {
        if (!item.is_boolean()) uncertain = true;
        else if (item.get<bool>()) ++correct;
    }
    if (uncertain) return answers.size() == 2 ? "third" : "full";
    if (answers.size() == 2 && correct == 1) return "third";
    if (answers.size() == 2) return correct == 2 ? "familiar" : "weak";
    return correct == 3 ? "familiar" : correct <= 1 ? "weak" : "full";
}

bool credibleOpenEvaluation(const Json& evaluation) {
    return evaluation.is_object() && evaluation.value("correct", Json()).is_boolean() &&
        evaluation.value("confidence", Json()).is_number() &&
        evaluation["confidence"].get<double>() >= 0.75 && evaluation["confidence"].get<double>() <= 1.0 &&
        evaluation.value("feedback", Json()).is_string() && evaluation.value("feedback", "").size() >= 8 &&
        evaluation.value("followUp", Json()).is_string();
}

bool sufficientPathEvidence(const Json& evidence, const std::string& direction) {
    if (!evidence.is_array() || (direction != "weak" && direction != "strong")) return false;
    int count = 0;
    bool objective = false;
    std::set<std::string> kinds;
    for (const auto& item : evidence) {
        if (!item.is_object() || item.value("direction", "") != direction || !item.value("credible", false)) continue;
        ++count;
        const std::string kind = item.value("kind", "");
        kinds.insert(kind);
        if (kind == "quiz") objective = true;
    }
    return count >= 3 && kinds.size() >= 2 && objective;
}

Json defaultAvailability() { return Json::array({{{"weekday", 1}, {"minutes", 30}}, {{"weekday", 3}, {"minutes", 30}}, {{"weekday", 5}, {"minutes", 30}}}); }

Json buildWeeklyDraft(const Json& availability, const Json& topics, const Json& reviews, const std::string& monday, const std::string& notBefore) {
    if (!validAvailability(availability)) throw std::invalid_argument("可用时间无效");
    Json entries = Json::array();
    const Json names = normalizedTopics(topics);
    size_t topicIndex = 0;
    std::vector<Json> pending;
    if (reviews.is_array()) for (const auto& review : reviews)
        if (review.is_object() && review.value("due", Json()).is_string()) pending.push_back(review);
    std::sort(pending.begin(), pending.end(), [](const Json& a, const Json& b) {
        return a.value("due", "") < b.value("due", "");
    });
    for (int day = 1; day <= 7; ++day) {
        const auto slot = std::find_if(availability.begin(), availability.end(), [day](const Json& item) { return item.value("weekday", 0) == day; });
        if (slot == availability.end()) continue;
        const std::string date = addDays(monday, day - 1);
        if (!notBefore.empty() && date < notBefore) continue;
        int remaining = (*slot)["minutes"].get<int>();
        while (!pending.empty() && pending.front().value("due", "") <= date && remaining >= 10) {
            entries.push_back({{"date", date}, {"title", pending.front().value("title", "到期复习")}, {"kind", "review"},
                {"minutes", 10}, {"manual", false}, {"order", entries.size()},
                {"reviewId", pending.front().value("reviewId", "")}});
            remaining -= 10;
            pending.erase(pending.begin());
        }
        if (topicIndex < names.size() && remaining >= 10) {
            Json entry = {{"date", date}, {"title", names[topicIndex]}, {"kind", "lesson"},
                {"minutes", std::min(30, remaining)}, {"manual", false}, {"order", entries.size()}};
            if (topics[topicIndex].is_object()) for (const char* field : {"phaseIndex", "topicIndex"})
                if (topics[topicIndex].contains(field)) entry[field] = topics[topicIndex][field];
            entries.push_back(std::move(entry));
            ++topicIndex;
        }
    }
    return entries;
}

Json publicQuestion(const Json& item) {
    if (!item.is_object()) return Json::object();
    Json result = {{"question", item.value("question", "")}, {"type", item.value("type", "choice")},
        {"status", item.value("status", "pending")}};
    if (item.contains("options")) result["options"] = item["options"];
    if (item.contains("answer")) result["answer"] = item["answer"];
    if (item.contains("feedback")) result["feedback"] = item["feedback"];
    return result;
}

Json classroomStart(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                    const std::string& kind, const std::string& topic) {
    if (kind != "diagnostic" && kind != "interaction") throw std::invalid_argument("活动类型无效");
    const std::string key = classroomKey(courseId, phaseIndex, topicIndex);
    std::lock_guard<std::mutex> guard(classroomMutex);
    Json questions = Json::array();
    for (int i = 0; i < 3; ++i) {
        const Json item = savedQuestion(db, key, kind, i);
        if (!item.is_object() || item.empty()) break;
        questions.push_back(item);
    }
    if (questions.size() != 3) {
        questions = kind == "diagnostic" ? immediateDiagnostics(db, courseId, phaseIndex, topicIndex, topic) :
            questionsFromAI(topic, kind);
        for (int i = 0; i < 3; ++i)
            save(db, itemId(key, kind, i), courseId, phaseIndex, topicIndex, kind, questions[i]);
    }
    Json visible = Json::array();
    const Json state = stateFor(db, key);
    const std::string mode = state.value("diagnosticMode", "pending");
    if (kind == "diagnostic" && state.value("diagnosticSkipped", false))
        return {{"ok", true}, {"questions", visible}, {"mode", "full"}, {"skipped", true}};
    const bool thirdWasUsed = kind == "diagnostic" &&
        (mode == "third" || questions[2].value("status", "pending") != "pending");
    const int count = kind == "diagnostic" && !thirdWasUsed ? 2 : 3;
    for (int i = 0; i < count; ++i) visible.push_back(publicQuestion(questions[i]));
    return {{"ok", true}, {"questions", visible}, {"mode", mode}};
}

Json classroomSubmit(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                     const std::string& kind, int index, const Json& answer) {
    if ((kind != "diagnostic" && kind != "interaction") || index < 0 || index > 2)
        throw std::invalid_argument("活动参数无效");
    const std::string key = classroomKey(courseId, phaseIndex, topicIndex);
    if (kind == "diagnostic" && stateFor(db, key).value("diagnosticSkipped", false))
        return {{"ok", true}, {"mode", "full"}, {"feedback", "诊断已跳过，不形成掌握证据。"}, {"credible", false}};
    Json item = savedQuestion(db, key, kind, index);
    if (!item.is_object() || item.empty()) throw std::invalid_argument("活动尚未开始");
    if (item.value("status", "pending") == "answered") return {{"ok", true}, {"item", publicQuestion(item)}, {"mode", stateFor(db, key).value("diagnosticMode", "pending")}};
    const bool open = item.value("type", "choice") == "open";
    Json evaluation = Json::object();
    bool correct = false, credible = true;
    if (open) {
        if (!answer.is_string() || answer.get<std::string>().empty() || answer.get<std::string>().size() > 4000)
            throw std::invalid_argument("开放回答无效");
        try { evaluation = gradeOpen(item, answer.get<std::string>()); }
        catch (...) { evaluation = {{"feedback", "评价暂不可用，可继续阅读和稍后重试。"}, {"followUp", ""}}; }
        credible = credibleOpenEvaluation(evaluation);
        correct = credible && evaluation.value("correct", false);
    } else {
        if (!answer.is_number_integer() || answer.get<int>() < 0 || answer.get<int>() > 3)
            throw std::invalid_argument("选择答案无效");
        correct = answer.get<int>() == item.value("answerIndex", -1);
        evaluation = {{"feedback", correct ? "回答正确。" : item.value("explanation", "请再想一想关键条件。")},
            {"followUp", correct ? "" : "你选择的依据是什么？"}};
    }
    item["answer"] = answer;
    item["status"] = "answered";
    item["correct"] = correct;
    item["credible"] = credible;
    item["feedback"] = evaluation.value("feedback", "请再想一想。");
    item["followUp"] = evaluation.value("followUp", "");
    item["hintLevel"] = correct ? 0 : 1;
    item["evidenceSource"] = open ? "ai-evaluation" : "objective-choice";
    item["questionSnapshot"] = questionSnapshot(item);
    item["questionId"] = questionIdentity(courseId, item["questionSnapshot"]);
    save(db, itemId(key, kind, index), courseId, phaseIndex, topicIndex, kind, item);
    std::string mode = stateFor(db, key).value("diagnosticMode", "pending");
    if (kind == "diagnostic") {
        Json answers = Json::array();
        for (int i = 0; i < 3; ++i) {
            const Json question = savedQuestion(db, key, kind, i);
            if (question.value("status", "pending") != "answered") break;
            answers.push_back(question.value("credible", false) ? Json(question.value("correct", false)) : Json(nullptr));
        }
        mode = diagnosticMode(answers, false);
        Json state = stateFor(db, key);
        state["diagnosticMode"] = mode;
        save(db, key + ":state", courseId, phaseIndex, topicIndex, "state", state);
    }
    Json result = {{"ok", true}, {"correct", correct}, {"credible", credible}, {"feedback", item["feedback"]},
        {"followUp", item["followUp"]}, {"hintLevel", item["hintLevel"]}, {"mode", mode}};
    if (kind == "diagnostic" && mode == "third") result["nextQuestion"] = publicQuestion(savedQuestion(db, key, kind, 2));
    return result;
}

Json classroomSkip(Database& db, const std::string& courseId, int phaseIndex, int topicIndex) {
    const std::string key = classroomKey(courseId, phaseIndex, topicIndex);
    Json state = stateFor(db, key);
    state["diagnosticMode"] = "full";
    state["diagnosticSkipped"] = true;
    save(db, key + ":state", courseId, phaseIndex, topicIndex, "state", state);
    return {{"ok", true}, {"mode", "full"}};
}

Json classroomSkipActivity(Database& db, const std::string& courseId, int phaseIndex, int topicIndex, int index) {
    if (index < 0 || index > 2) throw std::invalid_argument("活动序号无效");
    const std::string key = classroomKey(courseId, phaseIndex, topicIndex);
    Json item = savedQuestion(db, key, "interaction", index);
    if (!item.is_object() || item.empty()) throw std::invalid_argument("活动尚未开始");
    if (item.value("status", "pending") == "pending") {
        item["status"] = "skipped";
        item["answer"] = nullptr;
        item["feedback"] = "学生跳过活动，未形成掌握证据。";
        item["credible"] = false;
        save(db, itemId(key, "interaction", index), courseId, phaseIndex, topicIndex, "interaction", item);
    }
    return {{"ok", true}, {"item", publicQuestion(item)}};
}

Json classroomState(Database& db, const std::string& courseId, int phaseIndex, int topicIndex) {
    return stateFor(db, classroomKey(courseId, phaseIndex, topicIndex));
}

Json classroomHint(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                   const std::string& kind, int index) {
    std::lock_guard<std::mutex> guard(classroomMutex);
    const std::string key = classroomKey(courseId, phaseIndex, topicIndex);
    Json item = savedQuestion(db, key, kind, index);
    if (item.empty() || item.value("status", "pending") != "answered" || item.value("correct", false))
        throw std::invalid_argument("这道题无需提示");
    const int level = std::min(3, item.value("hintLevel", 1) + 1);
    std::string hint;
    if (level == 2) {
        ChatOptions options;
        options.temperature = 0.2;
        options.maxTokens = 250;
        options.timeoutMs = 12000;
        options.maxAttempts = 1;
        options.messages = {{"system", u8"你是课堂教师。学生已经看过追问，现在给一个不直接说答案、指向具体错误的简短提示。只输出提示文本。", ""},
            {"user", Json{{"question", item.value("question", "")}, {"answer", item.value("answer", Json())},
                {"explanation", item.value("explanation", "")}}.dump(), ""}};
        try { AIClient ai; hint = ai.chat(options).content; } catch (...) { hint = item.value("explanation", "请检查关键条件。"); }
    } else hint = item.value("explanation", "请重新对照题目条件。");
    item["hintLevel"] = level;
    item["lastHint"] = hint;
    save(db, itemId(key, kind, index), courseId, phaseIndex, topicIndex, kind, item);
    return {{"ok", true}, {"level", level}, {"hint", hint}};
}

Json classroomRemedial(Database& db, const std::string& courseId, int phaseIndex, int topicIndex) {
    std::lock_guard<std::mutex> guard(classroomMutex);
    const std::string key = classroomKey(courseId, phaseIndex, topicIndex);
    if (const auto existing = db.getClassroomActivity(key + ":remedial")) return readPayload(*existing);
    const auto session = db.findLearningSession(courseId, phaseIndex, topicIndex);
    const auto found = getCourseWithSnapshot(db, courseId);
    if (!found) throw std::invalid_argument("课程不存在");
    const auto stages = found->payload.value("courseStructure", Json::array());
    if (!stages.is_array() || phaseIndex < 1 || phaseIndex > static_cast<int>(stages.size())) throw std::invalid_argument("课时不存在");
    const auto topics = stages[phaseIndex - 1].value("topics", Json::array());
    if (!topics.is_array() || topicIndex < 1 || topicIndex > static_cast<int>(topics.size())) throw std::invalid_argument("课时不存在");
    const std::string title = topics[topicIndex - 1].get<std::string>();
    Json wrong = Json::array();
    for (int i = 0; i < 3; ++i) {
        const Json item = savedQuestion(db, key, "diagnostic", i);
        if (item.is_object() && item.value("status", "pending") == "answered" && !item.value("correct", false))
            wrong.push_back({{"question", item.value("question", "")}, {"feedback", item.value("feedback", "")}});
    }
    ChatOptions options;
    options.temperature = 0.3;
    options.maxTokens = 900;
    options.timeoutMs = 18000;
    options.maxAttempts = 1;
    options.messages = {{"system", u8"你是高中教师。只输出 JSON：{\"title\":\"\",\"content\":\"\",\"check\":\"\"}。生成可在5至10分钟读完的针对性补讲，先澄清错误，再用一个具体例子说明，最后给一个自检问题。", ""},
        {"user", Json{{"topic", title}, {"wrong", wrong}, {"lessonSummary", session ? session->summary.value_or("") : ""}}.dump(), ""}};
    AIClient ai;
    Json remedial = parseAIJson(ai.chat(options).content);
    if (!remedial.is_object() || !remedial.value("title", Json()).is_string() ||
        !remedial.value("content", Json()).is_string() || remedial.value("content", "").size() < 80 ||
        !remedial.value("check", Json()).is_string()) throw std::runtime_error("补讲内容无效");
    remedial["minutes"] = 7;
    save(db, key + ":remedial", courseId, phaseIndex, topicIndex, "remedial", remedial);
    return remedial;
}

Json submitReview(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                  int day, const Json& answers, const std::string& today,
                  const std::string& reviewId) {
    std::lock_guard<std::mutex> guard(classroomMutex);
    const std::string key = classroomKey(courseId, phaseIndex, topicIndex);
    std::string id = reviewId.empty() ? key + ":review:" + std::to_string(day) : reviewId;
    if (reviewId.empty() && day == -1) for (const auto& candidate : db.listClassroomActivities(courseId)) {
        const Json value = readPayload(candidate);
        if (candidate.kind == "review" && candidate.phaseIndex == phaseIndex && candidate.topicIndex == topicIndex &&
            value.value("day", 0) == -1 && value.value("status", "") == "pending" && value.value("due", "") <= today) {
            id = candidate.id;
            break;
        }
    }
    const auto row = db.getClassroomActivity(id);
    if (!row || row->courseId != courseId || row->phaseIndex != phaseIndex ||
        row->topicIndex != topicIndex || row->kind != "review" || !answers.is_array())
        throw std::invalid_argument("复习任务无效");
    Json review = readPayload(*row);
    if (review.value("status", "pending") != "pending") return {{"ok", true}, {"passed", review.value("status", "") == "passed"}};
    const auto session = db.findLearningSession(courseId, phaseIndex, topicIndex);
    if (!session) throw std::invalid_argument("本节测验尚未生成");
    const Json content = Json::parse(session->content);
    const Json quiz = content.at("blocks").at("quiz").at("quiz");
    if (!quiz.is_array() || quiz.empty() || answers.size() != quiz.size()) throw std::invalid_argument("复习答案数量无效");
    int score = 0;
    for (size_t i = 0; i < quiz.size(); ++i) {
        if (!answers[i].is_number_integer() || answers[i].get<int>() < 0 || answers[i].get<int>() > 3)
            throw std::invalid_argument("复习答案无效");
        if (answers[i].get<int>() == quiz[i].at("answerIndex").get<int>()) ++score;
    }
    const bool passed = score * 10 >= static_cast<int>(quiz.size()) * 7;
    review["status"] = passed ? "passed" : "failed";
    review["answers"] = answers;
    review["score"] = score;
    review["completedAt"] = today;
    save(db, id, courseId, phaseIndex, topicIndex, "review", review);
    bool newlyMastered = false;
    if (passed) {
        Json state = stateFor(db, key);
        newlyMastered = !state.value("quizPassed", false);
        if (newlyMastered) {
            state["quizPassed"] = true;
            save(db, key + ":state", courseId, phaseIndex, topicIndex, "state", state);
            for (const int reviewDay : {1, 3, 7})
                save(db, key + ":review:mastered:" + today + ":" + std::to_string(reviewDay),
                    courseId, phaseIndex, topicIndex, "review",
                    {{"title", review.value("title", "复习")}, {"due", addDays(today, reviewDay)},
                     {"status", "pending"}, {"phaseIndex", phaseIndex}, {"topicIndex", topicIndex},
                     {"day", reviewDay}});
        }
    }
    if (!passed) save(db, key + ":review:retry:" + today, courseId, phaseIndex, topicIndex, "review",
        {{"title", review.value("title", "复习")}, {"due", addDays(today, 1)}, {"status", "pending"},
         {"phaseIndex", phaseIndex}, {"topicIndex", topicIndex}, {"day", -1}});
    return {{"ok", true}, {"score", score}, {"total", quiz.size()}, {"passed", passed},
        {"nextDue", passed && !newlyMastered ? "" : addDays(today, 1)}};
}

Json dueReviews(Database& db, const std::string& courseId, const std::string& today) {
    Json due = Json::array();
    for (const auto& row : db.listClassroomActivities(courseId)) {
        if (row.kind != "review") continue;
        const Json item = readPayload(row);
        if (item.value("status", "pending") == "pending" && item.value("due", "") <= today) {
            Json visible = item;
            visible["reviewId"] = row.id;
            due.push_back(std::move(visible));
        }
    }
    return due;
}

Json getWeeklyPlan(Database& db, const std::string& courseId, const Json& topics, const std::string& monday,
                   const std::string& today) {
    std::lock_guard<std::recursive_mutex> guard(weeklyMutex);
    const auto saved = db.getWeeklyPlan(courseId);
    Json availability = defaultAvailability();
    int version = 1;
    if (saved) {
        Json plan = Json::parse(saved->payload, nullptr, false);
        if (plan.is_object()) {
            plan["version"] = saved->version;
            // 顺延到下周的计划不能在本周再次读取时被重置。
            const auto start = plan.value("weekStart", "");
            if (start >= monday && start <= addDays(monday, 7)) return plan;
            availability = plan.value("availability", availability);
            version = saved->version + 1;
        }
    }
    const std::string from = today.empty() ? monday : today;
    const auto start = availableWeek(availability, monday, from);
    const Json reviews = dueReviews(db, courseId, addDays(start, 6));
    const Json performance = recentPerformance(db, courseId, reviews);
    const Json order = prioritizeTopics(db, courseId, suggestTopicOrder(topics, performance), performance);
    const Json entries = buildWeeklyDraft(availability, order, reviews, start, from);
    Json plan = {{"availability", availability}, {"entries", entries}, {"weekStart", start}, {"version", version}};
    if (!db.upsert(WeeklyPlan{courseId, plan.dump(), now(), version})) throw std::runtime_error("周计划保存失败");
    return plan;
}

Json editWeeklyPlan(Database& db, const std::string& courseId, const Json& body, const std::string& today) {
    std::lock_guard<std::recursive_mutex> guard(weeklyMutex);
    const auto saved = db.getWeeklyPlan(courseId);
    if (!saved) throw std::invalid_argument("请先读取周计划");
    if (body.value("version", 0) != saved->version) throw std::invalid_argument("周计划版本已变化，请刷新");
    Json plan = Json::parse(saved->payload);
    const Json availability = body.value("availability", plan.value("availability", defaultAvailability()));
    Json entries = body.value("entries", plan.value("entries", Json::array()));
    if (!validAvailability(availability) || !validEntries(entries)) throw std::invalid_argument("周计划内容无效");
    if (!body.contains("entries")) {
        const auto found = getCourseWithSnapshot(db, courseId);
        Json topics = Json::array();
        int phase = 0;
        if (found && found->payload.value("courseStructure", Json()).is_array())
            for (const auto& stage : found->payload["courseStructure"]) {
                ++phase;
                int index = 0;
                if (stage.is_object() && stage.value("topics", Json()).is_array()) for (const auto& item : stage["topics"]) {
                    ++index;
                    const auto progress = db.findLearningCardProgress(courseId, phase, index);
                    if (!progress || progress->status != "completed") topics.push_back({{"title", item}, {"phaseIndex", phase}, {"topicIndex", index}});
                }
            }
        const auto from = today.empty() ? plan.value("weekStart", "") : today;
        const auto start = availableWeek(availability, plan.value("weekStart", ""), from);
        plan["weekStart"] = start;
        const auto reviews = dueReviews(db, courseId, addDays(start, 6));
        entries = buildWeeklyDraft(availability, prioritizeTopics(db, courseId, topics, recentPerformance(db, courseId, reviews)), reviews, start, from);
    }
    const Json oldEntries = plan.value("entries", Json::array());
    plan["availability"] = availability;
    plan["entries"] = entries;
    plan["manualAvailability"] = body.contains("availability");
    for (size_t i = 0; i < plan["entries"].size(); ++i) {
        const bool changed = body.contains("entries") && (i >= oldEntries.size() ||
            comparableEntries(Json::array({entries[i]})) != comparableEntries(Json::array({oldEntries[i]})));
        plan["entries"][i]["manual"] = changed || entries[i].value("manual", false);
    }
    plan["version"] = saved->version + 1;
    if (!db.upsert(WeeklyPlan{courseId, plan.dump(), now(), saved->version + 1})) throw std::runtime_error("周计划保存失败");
    weeklyProposals.erase(courseId);
    return plan;
}

Json replanWeeklyPlan(Database& db, const std::string& courseId, const Json& topics,
                      const std::string& monday, bool confirm, const Json& edits, const std::string& today) {
    std::lock_guard<std::recursive_mutex> guard(weeklyMutex);
    const auto saved = db.getWeeklyPlan(courseId);
    if (!saved) throw std::invalid_argument("请先读取周计划");
    if (edits.contains("version") && edits.at("version") != saved->version)
        throw std::invalid_argument("周计划版本已变化，请刷新后重排");
    Json current = Json::parse(saved->payload);
    current["version"] = saved->version;
    if (confirm) {
        const auto pending = weeklyProposals.find(courseId);
        if (pending == weeklyProposals.end() || pending->second.version != saved->version ||
            pending->second.expires < std::chrono::steady_clock::now() ||
            (edits.contains("proposalId") && edits.at("proposalId") != pending->second.id))
            throw std::invalid_argument("重排预览已失效，请重新重排");
        auto proposal = pending->second;
        proposal.plan["version"] = saved->version + 1;
        if (!db.upsert(WeeklyPlan{courseId, proposal.plan.dump(), now(), saved->version + 1})) throw std::runtime_error("周计划保存失败");
        weeklyProposals.erase(pending);
        return {{"requiresConfirmation", false}, {"plan", proposal.plan}, {"changed", true},
            {"message", proposal.message}, {"source", proposal.source}};
    }
    const Json availability = edits.value("availability", current.value("availability", defaultAvailability()));
    const Json visibleEntries = edits.value("entries", current.value("entries", Json::array()));
    if (!validAvailability(availability) || !validEntries(visibleEntries)) throw std::invalid_argument("请选择有效学习日和时长");
    const auto from = today.empty() ? monday : today;
    const auto start = availableWeek(availability, monday, from);
    const auto reviews = dueReviews(db, courseId, addDays(start, 6));
    if (topics.empty() && reviews.empty()) return {{"requiresConfirmation", false}, {"plan", current},
        {"changed", false}, {"message", "没有待安排的课时或复习任务。"}, {"source", "rules"}};
    const auto performance = recentPerformance(db, courseId, reviews);
    bool usedAI = false;
    const auto order = prioritizeTopics(db, courseId, suggestTopicOrder(topics, performance, &usedAI), performance);
    const auto proposed = buildWeeklyDraft(availability, order, reviews, start, from);
    Json next = current;
    next["availability"] = availability;
    next["entries"] = proposed;
    next["weekStart"] = start;
    const bool changed = current.value("weekStart", "") != start || current.at("availability") != availability ||
        comparableEntries(current.at("entries")) != comparableEntries(proposed);
    const bool visibleChanged = comparableEntries(visibleEntries) != comparableEntries(proposed);
    const std::string prefix = usedAI ? "" : "AI 暂不可用，已按学习记录重排。";
    const std::string message = prefix + (changed || visibleChanged ?
        (start > monday ? "已顺延到下周：" : "已更新学习安排：") + start + " 至 " + addDays(start, 6) + "。" :
        "已检查最新表现，当前安排无需变化。");
    bool manual = comparableEntries(visibleEntries) != comparableEntries(current.at("entries"));
    for (const auto& entry : visibleEntries) manual = manual || entry.value("manual", false);
    if (manual && visibleChanged) {
        const auto clock = std::chrono::steady_clock::now();
        for (auto item = weeklyProposals.begin(); item != weeklyProposals.end(); )
            if (item->second.expires < clock) item = weeklyProposals.erase(item); else ++item;
        const auto id = std::to_string(clock.time_since_epoch().count()) + "-" + std::to_string(++proposalSequence);
        weeklyProposals.insert_or_assign(courseId, WeeklyProposal{next, saved->version, id, message,
            usedAI ? "ai" : "rules", clock + std::chrono::minutes(10)});
        Json visible = current; visible["entries"] = visibleEntries; visible["availability"] = availability;
        return {{"requiresConfirmation", true}, {"current", visible}, {"proposed", proposed},
            {"proposalId", id}, {"weekStart", start}, {"message", message}, {"source", usedAI ? "ai" : "rules"}};
    }
    if (changed) {
        next["version"] = saved->version + 1;
        if (!db.upsert(WeeklyPlan{courseId, next.dump(), now(), saved->version + 1})) throw std::runtime_error("周计划保存失败");
    }
    weeklyProposals.erase(courseId);
    return {{"requiresConfirmation", false}, {"plan", next}, {"changed", changed},
        {"message", message}, {"source", usedAI ? "ai" : "rules"}};
}

Json finishClassroom(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                     bool passed, const std::string& today) {
    std::lock_guard<std::mutex> guard(classroomMutex);
    const std::string key = classroomKey(courseId, phaseIndex, topicIndex);
    Json state = stateFor(db, key);
    const bool familiar = state.value("diagnosticMode", "") == "familiar";
    const Json challenge = savedQuestion(db, key, "interaction", 2);
    const bool challengePassed = challenge.is_object() && challenge.value("status", "") == "answered" &&
        challenge.value("correct", false);
    if (familiar && !challengePassed) passed = false;
    state["completed"] = true;
    state["quizPassed"] = passed;
    save(db, key + ":state", courseId, phaseIndex, topicIndex, "state", state);
    const auto session = db.findLearningSession(courseId, phaseIndex, topicIndex);
    const std::string title = session ? session->topicTitle : "本节内容";
    const std::vector<int> days = passed ? std::vector<int>{1, 3, 7} : std::vector<int>{1};
    for (const int day : days) {
        const std::string id = key + ":review:" + std::to_string(day);
        if (db.getClassroomActivity(id)) continue;
        save(db, id, courseId, phaseIndex, topicIndex, "review",
            {{"title", title}, {"due", addDays(today, day)}, {"status", "pending"},
             {"phaseIndex", phaseIndex}, {"topicIndex", topicIndex}, {"day", day}});
    }
    Json evidence = Json::array();
    if (!(familiar && !challengePassed)) for (const auto& row : db.listClassroomActivities(courseId)) {
        if (row.phaseIndex != phaseIndex || row.topicIndex != topicIndex ||
            (row.kind != "diagnostic" && row.kind != "interaction")) continue;
        if (row.kind == "diagnostic" && state.value("diagnosticSkipped", false)) continue;
        const Json item = readPayload(row);
        if (item.value("status", "") != "answered") continue;
        evidence.push_back({{"kind", row.kind}, {"direction", item.value("correct", false) ? "strong" : "weak"},
            {"credible", item.value("credible", false)}});
    }
    bool quizFound = false, latestQuizPassed = false;
    for (const auto& row : db.listInteractions()) {
        if (row.courseId.value_or("") != courseId || row.kind != "quiz") continue;
        const Json quiz = Json::parse(row.payload, nullptr, false);
        if (!quiz.is_object() || quiz.value("phaseIndex", 0) != phaseIndex || quiz.value("topicIndex", 0) != topicIndex) continue;
        quizFound = true;
        latestQuizPassed = quiz.value("score", 0) * 10 >= quiz.value("total", 1) * 7;
    }
    if (quizFound) evidence.push_back({{"kind", "quiz"}, {"direction", latestQuizPassed ? "strong" : "weak"}, {"credible", true}});
    std::string adjustment;
    if (quizFound && !state.value("pathAdjusted", false)) {
        const std::string direction = sufficientPathEvidence(evidence, "weak") ? "weak" :
            sufficientPathEvidence(evidence, "strong") ? "strong" : "";
        if (!direction.empty()) try {
            if (adjustFuturePath(db, courseId, phaseIndex, topicIndex, direction)) {
                adjustment = direction;
                state["pathAdjusted"] = true;
                save(db, key + ":state", courseId, phaseIndex, topicIndex, "state", state);
            }
        } catch (...) {}
    }
    return {{"ok", true}, {"nextStep", passed ? "review" : "remedial"},
        {"reviewDue", addDays(today, 1)}, {"newLessonAllowed", true},
        {"pathAdjustment", adjustment}, {"challengeRequired", familiar && !challengePassed}};
}

}

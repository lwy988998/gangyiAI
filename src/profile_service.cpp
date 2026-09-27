#include "profile_service.hpp"

#include "ai_client.hpp"
#include "db.hpp"
#include "json_fix.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <set>
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
    return input.size() <= limit ? input : input.substr(0, limit);
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

json profileEvidenceSummary(Database& db) {
    json summary = {{"courses", json::array()}, {"interactions", json::array()}};
    for (const auto& course : db.listCourses()) {
        if (course.status != "active") continue;
        json row = {{"title", shortText(course.title, 120)}, {"goal", shortText(course.goal, 160)},
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
    return summary;
}

json profileView(Database& db) {
    json subjects = json::array();
    for (const auto& value : db.listMastery()) {
        json weakPoints = json::parse(value.weakPoints, nullptr, false);
        if (!weakPoints.is_array()) weakPoints = json::array();
        subjects.push_back({{"subject", value.subject},
            {"score", value.score ? json(*value.score) : json(nullptr)},
            {"rationale", value.rationale}, {"weakPoints", weakPoints},
            {"recommendation", value.recommendation}, {"evidenceCount", value.evidenceCount},
            {"model", value.model}, {"updatedAt", value.updatedAt}});
    }
    const auto courses = db.listCourses();
    return {{"subjects", subjects}, {"updating", db.profileDirty()}, {"error", db.profileError()},
        {"hasEvidence", std::any_of(courses.begin(), courses.end(), [](const auto& course) { return course.status == "active"; }) ||
            !db.listInteractions().empty()}};
}

std::string profileContext(Database& db) {
    json context = json::array();
    for (const auto& value : db.listMastery()) {
        if (!value.score) continue;
        context.push_back({{"subject", value.subject}, {"strength", *value.score},
            {"weakPoints", json::parse(value.weakPoints, nullptr, false)},
            {"recommendation", value.recommendation}});
    }
    return context.dump();
}

bool refreshProfile(Database& db, AIClient& ai, std::string& error) {
    const int revision = db.profileRevision();
    const json evidence = profileEvidenceSummary(db);
    if (evidence["courseCount"] == 0 && evidence["interactionCount"] == 0) {
        db.replaceMastery({});
        db.setProfileAssessed(revision);
        db.setProfileError("");
        return true;
    }
    try {
        ChatOptions options;
        options.messages = {
            {"system", u8"你是高中学习画像评估 AI。仅根据提供的匿名学习摘要，识别已学学科并自主评估每科 0-100 的掌握强度。完成度不是掌握度；没有可靠证据时 score 用 null。只返回 JSON：{\"subjects\":[{\"subject\":\"数学\",\"score\":72,\"rationale\":\"依据说明\",\"weakPoints\":[\"知识点\"],\"recommendation\":\"下一步建议\",\"evidenceCount\":3}]}。不得编造测验成绩或学科。", ""},
            {"user", evidence.dump(), ""},
        };
        options.temperature = 0.2;
        options.maxTokens = 2400;
        options.timeoutMs = 45000;
        options.maxAttempts = 1;
        const auto result = ai.chat(options);
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
            value.rationale = shortText(item.value("rationale", ""), 500);
            value.recommendation = shortText(item.value("recommendation", ""), 500);
            const json weakPoints = item.value("weakPoints", json::array());
            if (!weakPoints.is_array()) throw std::runtime_error("薄弱点格式无效");
            value.weakPoints = weakPoints.dump();
            value.evidenceCount = item.value("evidenceCount", 0);
            if (value.evidenceCount < 0 || value.evidenceCount > evidence["courseCount"].get<int>() + evidence["interactionCount"].get<int>())
                throw std::runtime_error("证据数量无效");
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

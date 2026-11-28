#pragma once

#include "component_activity.hpp"
#include "interactive_capabilities.hpp"
#include <cassert>
#include <functional>

inline void runComponentActivityChecks() {
    using Json = nlohmann::json;
    const Json state = {{"schemaVersion", 1}, {"contentVersion", 4}, {"setA", {"1", "2"}}, {"setB", {"2"}}};
    const Json media = {{"id", "sets-1"}, {"type", "sets"}, {"universe", {"1", "2", "3"}},
                        {"setA", {"1", "2"}}, {"setB", {"2"}}};
    const Json lesson = {{"id", "lesson-1"}, {"deepContent", {{"version", 4}, {"media", {media}},
        {"lecture", {{{"id", "page-1"}, {"mediaIds", {"sets-1"}}}}}, {"slides", Json::array()}}}};
    const Json trace = {{"schemaVersion", 1}, {"contentVersion", 4}, {"sessionId", "session-1"},
        {"totalChanges", 1}, {"truncated", false}, {"events", {{{"seq", 1}, {"kind", "finish"}, {"state", state}}}}};
    const Json event = {{"requestId", "request-1"}, {"deepView", "lecture"}, {"contentId", "page-1"},
        {"mediaState", {{"sets-1", state}}}, {"componentActivities", {{"sets-1", trace}}}};
    const auto checked = gangyi::checkedComponentActivities(lesson, event);
    assert(checked.at("sets-1").at("events")[0].at("observation").at("intersection") == Json::array({"2"}));
    assert(checked.at("sets-1").at("scoringEligible") == false);
    const auto summary = gangyi::componentActivitySummaries(checked).at("sets-1");
    assert(!summary.contains("events") && summary.at("storedEvents") == 1);
    assert(summary.at("detailsTool") == "read_component_activity");

    const auto rejects = [&](const Json& bad) {
        bool rejected = false;
        try { gangyi::checkedComponentActivities(lesson, bad); } catch (const std::exception&) { rejected = true; }
        assert(rejected);
    };
    auto bad = event; bad["contentId"] = "other-page"; rejects(bad);
    bad = event; bad["componentActivities"]["other-component"] = trace; rejects(bad);
    bad = event; bad["componentActivities"]["sets-1"]["contentVersion"] = 3; rejects(bad);
    bad = event; bad["componentActivities"]["sets-1"]["events"][0]["seq"] = 2; rejects(bad);
    bad = event; bad["componentActivities"]["sets-1"]["truncated"] = true; rejects(bad);
    bad = event; bad["mediaState"]["sets-1"]["setB"] = Json::array(); rejects(bad);
    bad = event; bad["componentActivities"]["sets-1"]["events"][0]["state"]["score"] = 100; rejects(bad);

    const auto capabilities = gangyi::interactiveCapabilities();
    assert(capabilities.at("components").size() == 18);
    for (const auto& capability : capabilities.at("components")) {
        assert(capability.at("activityReading").at("detailsTool") == "read_component_activity");
        assert(capability.at("scoringEligible") == false);
    }
}

// 通过正式主控调用轨迹工具；固定模型只验证权限与分页，不评价教学质量。
inline void runComponentActivityToolChecks(const std::filesystem::path& path) {
    using Json = nlohmann::json;
    gangyi::Database db; db.open(path.string()); db.migrate();
    const gangyi::AgentAccess access{"activity-owner", {"math", "other-course"}};
    for (const auto* id : {"math", "other-course"})
        assert(db.insert(gangyi::Course{id, access.scopeId, {}, id, "deep", id, {}, "ai", "active", "2026-11-18", "2026-11-18"}));
    const Json state = {{"schemaVersion", 1}, {"contentVersion", 4}, {"setA", {"1", "2"}}, {"setB", {"2"}}};
    const Json media = {{"id", "sets-1"}, {"type", "sets"}, {"universe", {"1", "2", "3"}}, {"setA", {"1", "2"}}, {"setB", {"2"}}};
    const Json lesson = {{"id", "lesson-1"}, {"scopeId", access.scopeId}, {"courseId", "math"},
        {"title", "集合关系"}, {"purpose", "检查活动"}, {"status", "ready"}, {"entered", true},
        {"version", 1}, {"updatedAt", "2026-11-18"}, {"sections", Json::array()},
        {"deepContent", {{"version", 4}, {"media", {media}}, {"lecture", {{{"id", "page-1"}, {"mediaIds", {"sets-1"}}}}}, {"slides", Json::array()}}}};
    assert(db.upsert(gangyi::ClassroomActivity{"lesson-1", "math", "agent-lesson", lesson.dump(), "2026-11-18", 0, 0}));
    auto otherLesson = lesson; otherLesson["id"] = "lesson-2";
    assert(db.upsert(gangyi::ClassroomActivity{"lesson-2", "math", "agent-lesson", otherLesson.dump(), "2026-11-18", 0, 0}));

    Json events = Json::array();
    for (int seq = 1; seq <= 60; ++seq) events.push_back({{"seq", seq}, {"kind", "change"}, {"state", state}});
    const Json trace = {{"schemaVersion", 1}, {"contentVersion", 4}, {"sessionId", "fixture-session"},
        {"totalChanges", 60}, {"truncated", false}, {"events", events}};
    const Json event = {{"requestId", "original-activity"}, {"deepView", "lecture"}, {"contentId", "page-1"},
        {"mediaState", {{"sets-1", state}}}, {"componentActivities", {{"sets-1", trace}}}};
    const auto activities = gangyi::checkedComponentActivities(lesson, event);
    const auto insertInput = [&](const std::string& id, const std::string& scope, const std::string& lessonId, const std::string& course) {
        const Json input = {{"scopeId", scope}, {"lessonId", lessonId}, {"requestId", id}, {"text", "探索记录"},
                            {"type", "chat"}, {"componentActivities", activities}};
        assert(db.insert(gangyi::LearningInteraction{id, "chat-user", input.dump(), "2026-11-18", course, {}, {}}));
    };
    insertInput("own-input", access.scopeId, "lesson-1", "math");
    insertInput("foreign-input", "another-owner", "lesson-1", "math");
    insertInput("other-lesson-input", access.scopeId, "lesson-2", "math");
    insertInput("mismatched-course-input", access.scopeId, "lesson-1", "other-course");
    db.close();
    db.open(path.string());

    int serial = 0;
    const auto attempt = [&](Json args, bool allowed, const std::string& errorText) {
        ++serial;
        const auto task = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"courseId", "math"}, {"lessonId", "lesson-1"},
            {"requestId", "activity-tool-" + std::to_string(serial)}, {"text", "请核对这次探索"}});
        int calls = 0;
        gangyi::LearningAgent agent(path.string(), [&](const gangyi::ChatOptions&, const auto& sink) {
            ++calls;
            if (calls == 1) return output({{"message", ""}, {"actions", Json::array({{{"id", "read-trace"},
                {"tool", "read_component_activity"}, {"args", args}}})}, {"state", "continue"}}, sink);
            return output({{"message", "已核对可读取的记录。"}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
        });
        gangyi::registerAgentLessonTools(agent);
        agent.process(db, task.at("id"));
        const auto saved = Json::parse(db.getClassroomActivity(task.at("id").get<std::string>())->payload);
        const auto& completed = saved.at("completedActions");
        if (allowed) {
            assert(completed.contains("read-trace"));
            return completed.at("read-trace").at("result");
        }
        assert(!completed.contains("read-trace"));
        assert(saved.at("messages").dump().find(errorText) != std::string::npos);
        return Json::object();
    };
    const Json base = {{"lessonId", "lesson-1"}, {"interactionId", "own-input"}, {"componentId", "sets-1"}};
    auto first = base; first["limit"] = 50;
    const auto firstPage = attempt(first, true, "");
    assert(firstPage.at("events").size() == 50 && firstPage.at("hasMore") == true && firstPage.at("storedEvents") == 60);
    assert(firstPage.at("traceId") == activities.at("sets-1").at("traceId"));
    auto second = base; second["offset"] = 50; second["limit"] = 50;
    const auto secondPage = attempt(second, true, "");
    assert(secondPage.at("events").size() == 10 && secondPage.at("events")[0].at("seq") == 51 && secondPage.at("hasMore") == false);
    for (const auto* id : {"foreign-input", "other-lesson-input", "mismatched-course-input"}) {
        auto bad = base; bad["interactionId"] = id; attempt(bad, false, "无权读取其他课堂轨迹");
    }
    auto bad = base; bad["lessonId"] = "lesson-2"; attempt(bad, false, "轨迹读取必须绑定当前课时");
    bad = base; bad["componentId"] = "other-component"; attempt(bad, false, "本次输入没有该组件活动");
    bad = base; bad["limit"] = 51; attempt(bad, false, "轨迹分页超出范围");
    bad = base; bad["offset"] = -1; attempt(bad, false, "轨迹分页超出范围");
    bad = base; bad["offset"] = 4294967296ULL; attempt(bad, false, "轨迹分页超出范围");
    bad = base; bad["limit"] = 4294967297ULL; attempt(bad, false, "轨迹分页超出范围");
    bad = base; bad["limit"] = true; attempt(bad, false, "轨迹分页须为整数");
}

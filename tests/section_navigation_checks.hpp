#pragma once
#include <algorithm>

// 使用确定性工具动作验证保存回执与公开页面的一致性，不发起外部模型请求。
inline void runSectionNavigationChecks(const std::filesystem::path& path) {
    using namespace gangyi;
    Database db; db.open(path.string()); db.migrate();
    const AgentAccess access{"section-navigation", {"navigation-math"}};
    assert(db.insert(Course{"navigation-math", access.scopeId, {}, "分数", "deep", "分数", {}, "ai", "active", "now", "now"}));
    Json lesson = {{"id", "navigation-lesson"}, {"scopeId", access.scopeId}, {"courseId", "navigation-math"},
        {"title", "原卡片导航"}, {"purpose", "验证保存位置"}, {"status", "ready"}, {"entered", true},
        {"version", 1}, {"sourceLearningVersion", db.learningRevision()}, {"sections", {
            {{"id", "original-question"}, {"kind", "question"}, {"title", "原练习"}, {"questionKind", "practice"},
                {"questionId", "original-question-id"}, {"version", 1}, {"question", {
                    {"question", "1/2+1/3=?"}, {"type", "open"}, {"expectedAnswer", "PRIVATE-NAV-ANSWER"}, {"rubric", "PRIVATE-NAV-RUBRIC"}}}}
        }}};
    assert(db.upsert(ClassroomActivity{"navigation-lesson", "navigation-math", "agent-lesson", lesson.dump(), "now", 0, 0}));
    int calls = 0;
    LearningAgent agent(path.string(), [&](const auto&, const auto& sink) {
        ++calls;
        Json actions = Json::array();
        const auto append = [&](const std::string& id, const Json& section, bool activate) {
            actions.push_back({{"id", id}, {"tool", "append_section"}, {"args", {
                {"lessonId", "navigation-lesson"}, {"section", section}, {"activate", activate}}}});
        };
        append("saved-explanation", {{"kind", "explanation"}, {"title", "原题的一点提示"}, {"body", "先比较两个分数的单位。"},
            {"helpType", "hint"}, {"targetQuestionId", "original-question-id"}}, false);
        for (const auto* kind : {"interaction", "practice"}) append(kind,
            {{"kind", "question"}, {"questionKind", kind}, {"title", std::string(kind) + "新题"},
                {"question", {{"question", std::string(kind) + "：你怎样判断分数单位？"}, {"type", "open"},
                    {"expectedAnswer", "PRIVATE-NAV-ANSWER"}, {"rubric", "PRIVATE-NAV-RUBRIC"}}}}, true);
        append("summary", {{"kind", "summary"}, {"title", "本次小结"}, {"body", "同一整体和相同分数单位需要分清。"}, {"helpType", "explanation"}}, true);
        return output({{"message", "内容已保存，请按原卡片位置查看。"}, {"teaching", {{"helpType", "explanation"}}},
            {"actions", actions}, {"state", "waiting_student"}}, sink);
    });
    registerAgentLessonTools(agent);
    for (int index = 0; index < 3; ++index) {
        Json event = {{"type", "chat"}, {"courseId", "navigation-math"}, {"lessonId", "navigation-lesson"},
            {"text", "继续解释分数单位"}, {"requestId", "navigation-" + std::to_string(index)}};
        if (index < 2) event["view"] = index == 0 ? "practice" : "learn";
        else event["sectionId"] = "original-question";
        const auto task = agentSubmit(db, access, event);
        agent.process(db, task.at("id"));
        const auto finished = agentView(db, access, task.at("id"));
        assert(finished.at("status") == "waiting_student" && calls == index + 1);
        auto taskRow = *db.getClassroomActivity(task.at("id"));
        auto raw = Json::parse(taskRow.payload);
        const auto beforeLesson = db.getClassroomActivity("navigation-lesson")->payload;
        const auto beforeSectionExposure = db.profileMeta("agent-section-exposure:navigation-lesson");
        const auto beforeMessageExposure = db.profileMeta("agent-exposure:" + task.at("id").get<std::string>());
        const auto shown = agentLessonView(db, access, "navigation-lesson");
        std::size_t matchingTimeline = 0;
        for (const auto& action : raw.at("completedActions").items()) {
            const auto& receipt = action.value().at("result");
            const auto id = receipt.at("sectionId");
            const auto section = std::find_if(shown.at("sections").begin(), shown.at("sections").end(),
                [&](const Json& value) { return value.at("id") == id; });
            assert(section != shown.at("sections").end());
            const auto expectedView = section->at("kind") == "summary" ? "summary" : index != 1 ? "practice" :
                section->value("questionKind", "") == "practice" ? "practice" : "learn";
            assert(receipt.at("view") == expectedView && section->at("view") == expectedView);
            assert(receipt.at("href") == section->at("href"));
            assert(receipt.at("href") == "/" + std::string(expectedView) + "?lessonId=navigation-lesson&sectionId=" + id.get<std::string>());
            for (const auto& item : shown.at("timeline")) if (item.value("type", "") == "section" && item.at("sectionId") == id) {
                ++matchingTimeline; assert(item.at("href") == receipt.at("href") && item.at("view") == receipt.at("view"));
            }
            if (action.key() == "saved-explanation") {
                assert(section->at("helpType") == "hint" && section->at("targetQuestionId") == "original-question-id");
                assert(!section->at("displayed").get<bool>());
            }
        }
        assert(matchingTimeline == 3);
        assert(shown.dump().find("PRIVATE-NAV") == std::string::npos);
        assert(shown.dump().find("expectedAnswer") == std::string::npos && shown.dump().find("rubric") == std::string::npos);
        assert(db.getClassroomActivity("navigation-lesson")->payload == beforeLesson);
        assert(db.profileMeta("agent-section-exposure:navigation-lesson") == beforeSectionExposure);
        assert(db.profileMeta("agent-exposure:" + task.at("id").get<std::string>()) == beforeMessageExposure);
        // 旧版回执缺少新字段时，从原任务恢复同一板块归属，读取不回写旧记录。
        for (auto& action : raw["completedActions"].items()) { action.value()["result"].erase("view"); action.value()["result"].erase("href"); }
        taskRow.payload = raw.dump(); assert(db.upsert(taskRow));
        const auto recovered = agentLessonView(db, access, "navigation-lesson");
        assert(recovered.at("sections") == shown.at("sections") && recovered.at("timeline") == shown.at("timeline"));
        assert(db.getClassroomActivity(task.at("id"))->payload == taskRow.payload && calls == index + 1);
    }
    bool denied = false;
    try { agentLessonView(db, {"another-scope", {"navigation-math"}}, "navigation-lesson"); }
    catch (const std::invalid_argument&) { denied = true; }
    assert(denied);
    denied = false;
    try { agentLessonView(db, {access.scopeId, {}}, "navigation-lesson"); }
    catch (const std::invalid_argument&) { denied = true; }
    assert(denied);
    // 地址按查询参数编码，特殊字符不能截断课时和板块身份。
    const auto encoded = agentSectionLocation({{"id", "lesson &/"}}, {{"id", "section?一"}, {"kind", "explanation"}});
    assert(encoded.at("href") == "/learn?lessonId=lesson%20%26%2F&sectionId=section%3F%E4%B8%80");
}

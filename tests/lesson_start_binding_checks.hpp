#pragma once

// 新开课接管允许的终态；未完成或需恢复的作答继续由原任务处理。
inline void runLessonStartBindingChecks(const std::filesystem::path& path) {
    using namespace gangyi;
    Database db; db.open(path.string()); db.migrate();
    const AgentAccess access{"lesson-start-binding", {"binding-math"}};
    assert(db.insert(Course{"binding-math", access.scopeId, {}, "分数", "deep", "分数", {}, "ai", "active", "now", "now"}));
    const auto saveLesson = [&](const std::string& id) {
        const Json lesson = {{"id", id}, {"scopeId", access.scopeId}, {"courseId", "binding-math"}, {"title", "先练后讲"},
            {"purpose", "验证任务接管"}, {"status", "ready"}, {"entered", true}, {"version", 1},
            {"sourceLearningVersion", db.learningRevision()}, {"sections", {
                {{"id", "concept"}, {"kind", "explanation"}, {"title", "分数单位"}, {"body", "先统一分数单位。"}, {"version", 1}},
                {{"id", "interaction"}, {"kind", "question"}, {"title", "说出单位"}, {"questionKind", "interaction"},
                    {"questionId", "binding-interaction"}, {"question", {{"question", "你如何理解分数单位？"}, {"type", "open"}}}, {"version", 1}},
                {{"id", "practice"}, {"kind", "question"}, {"title", "练习"}, {"questionKind", "practice"},
                    {"questionId", "binding-practice"}, {"question", {{"question", "比较1/2与1/3。"}, {"type", "open"}}}, {"version", 1}}
            }}};
        assert(db.upsert(ClassroomActivity{id, "binding-math", "agent-lesson", lesson.dump(), "now", 0, 0}));
    };
    saveLesson("binding-lesson");
    const auto practice = agentSubmit(db, access, {{"type", "chat"}, {"courseId", "binding-math"}, {"lessonId", "binding-lesson"},
        {"view", "practice"}, {"text", "先解释练习中的分数单位。"}, {"requestId", "binding-first-practice"}});
    const auto start = [&](Database& connection) { return agentStartLesson(connection, access, {{"lessonId", "binding-lesson"}}); };
    auto deferred = start(db);
    assert(deferred.at("id") == practice.at("id") && deferred.at("initialTeachingDeferred") == true);
    assert(!Json::parse(db.getClassroomActivity("binding-lesson")->payload).contains("initialTeachingTaskId"));
    int calls = 0; bool replayOld = false;
    LearningAgent agent(path.string(), [&](const auto& options, const auto& sink) {
        ++calls;
        if (options.activity.taskId == practice.at("id") && !replayOld) {
            Database another; another.open(path.string());
            const auto active = start(another);
            assert(active.at("id") == practice.at("id") && active.at("status") == "running" && active.at("initialTeachingDeferred") == true);
            assert(!Json::parse(another.getClassroomActivity("binding-lesson")->payload).contains("initialTeachingTaskId"));
            const auto before = another.getClassroomActivity("binding-lesson")->payload;
            bool rejected = false;
            try { another.transaction([&] { agentAttachLessonTask(another, access,
                {{"type", "lesson_start"}, {"courseId", "binding-math"}, {"lessonId", "binding-lesson"}}, "concurrent-start"); }); }
            catch (const std::invalid_argument&) { rejected = true; }
            assert(rejected && another.getClassroomActivity("binding-lesson")->payload == before);
        }
        Json action = {{"id", replayOld ? "late-focus" : "choose-focus"}, {"tool", "select_teaching_focus"}, {"args", {
            {"lessonId", "binding-lesson"}, {"sectionId", "concept"}, {"interactionSectionId", "interaction"}, {"practiceSectionId", "practice"}}}};
        return output({{"message", "请先说出你的理解，再继续教学。"}, {"actions", Json::array({action})}, {"state", "waiting_student"}}, sink);
    });
    registerAgentLessonTools(agent); agent.process(db, practice.at("id"));
    assert(agentView(db, access, practice.at("id")).at("status") == "waiting_student" && calls == 1);
    const auto initial = start(db);
    assert(initial.at("id") != practice.at("id") && initial.at("status") == "pending");
    auto lesson = Json::parse(db.getClassroomActivity("binding-lesson")->payload);
    assert(lesson.at("initialTeachingTaskId") == initial.at("id") && lesson.at("teachingTaskId") == initial.at("id"));
    assert(start(db).at("id") == initial.at("id") && calls == 1);
    agent.process(db, initial.at("id"));
    assert(agentView(db, access, initial.at("id")).at("status") == "waiting_student" && calls == 2);
    assert(start(db).at("id") == initial.at("id") && calls == 2);
    // 即使手动恢复旧任务，它也不能重新覆盖新任务保存的教学焦点。
    auto oldRow = *db.getClassroomActivity(practice.at("id")); auto old = Json::parse(oldRow.payload);
    old["status"] = "failed"; oldRow.payload = old.dump(); assert(db.upsert(oldRow));
    const auto beforeReplay = db.getClassroomActivity("binding-lesson")->payload;
    replayOld = true; agentControl(db, access, {{"command", "retry"}, {"taskId", practice.at("id")}});
    agent.process(db, practice.at("id"));
    assert(agentView(db, access, practice.at("id")).at("status") == "failed");
    assert(db.getClassroomActivity("binding-lesson")->payload == beforeReplay);
    for (const auto* status : {"pending", "running", "paused", "failed", "cancelled", "superseded", "ready"}) {
        const auto id = std::string("binding-") + status; saveLesson(id);
        const auto task = agentSubmit(db, access, {{"type", "chat"}, {"courseId", "binding-math"}, {"lessonId", id},
            {"view", "practice"}, {"text", "保留本次真实请求。"}, {"requestId", id}});
        auto row = *db.getClassroomActivity(task.at("id")); auto raw = Json::parse(row.payload); raw["status"] = status;
        row.payload = raw.dump(); assert(db.upsert(row));
        const auto resumed = agentStartLesson(db, access, {{"lessonId", id}});
        if (std::string(status) == "ready" || std::string(status) == "cancelled" || std::string(status) == "superseded") {
            assert(resumed.at("id") != task.at("id"));
            const auto bound = Json::parse(db.getClassroomActivity(id)->payload);
            assert(bound.at("teachingTaskId") == resumed.at("id") && bound.at("initialTeachingTaskId") == resumed.at("id"));
            assert(db.getClassroomActivity(task.at("id"))->payload == row.payload);
            assert(agentStartLesson(db, access, {{"lessonId", id}}).at("id") == resumed.at("id"));
        }
        else {
            assert(resumed.at("id") == task.at("id") && resumed.at("initialTeachingDeferred") == true);
            assert(!Json::parse(db.getClassroomActivity(id)->payload).contains("initialTeachingTaskId"));
        }
    }
}

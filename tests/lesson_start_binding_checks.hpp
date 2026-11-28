// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 新开课接管允许的终态；未完成或需恢复的作答继续由原任务处理。
// 【逐行批注】 定义函数 runLessonStartBindingChecks，执行该函数声明的职责。
inline void runLessonStartBindingChecks(const std::filesystem::path& path) {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 【逐行批注】 执行当前声明或表达式：Database db; db.open(path.string()); db.migrate();。
    Database db; db.open(path.string()); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：const AgentAccess access{"lesson-start-binding", {"binding-math"}};。
    const AgentAccess access{"lesson-start-binding", {"binding-math"}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(Course{"binding-math", access.scopeId, {}, "分数", "de…。
    assert(db.insert(Course{"binding-math", access.scopeId, {}, "分数", "deep", "分数", {}, "ai", "active", "now", "now"}));
    // 【逐行批注】 计算右侧结果并写入 auto saveLesson，更新当前运行状态。
    const auto saveLesson = [&](const std::string& id) {
        // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
        const Json lesson = {{"id", id}, {"scopeId", access.scopeId}, {"courseId", "binding-math"}, {"title", "先练后讲"},
            // 【逐行批注】 执行当前声明或表达式：{"purpose", "验证任务接管"}, {"status", "ready"}, {"entered", true}, {"version", 1},。
            {"purpose", "验证任务接管"}, {"status", "ready"}, {"entered", true}, {"version", 1},
            // 【逐行批注】 执行当前声明或表达式：{"sourceLearningVersion", db.learningRevision()}, {"sections", {。
            {"sourceLearningVersion", db.learningRevision()}, {"sections", {
                // 【逐行批注】 执行当前声明或表达式：{{"id", "concept"}, {"kind", "explanation"}, {"title", "分数单位"}, {"body", "先统一分数单位。"}…。
                {{"id", "concept"}, {"kind", "explanation"}, {"title", "分数单位"}, {"body", "先统一分数单位。"}, {"version", 1}},
                // 【逐行批注】 执行当前声明或表达式：{{"id", "interaction"}, {"kind", "question"}, {"title", "说出单位"}, {"questionKind", "i…。
                {{"id", "interaction"}, {"kind", "question"}, {"title", "说出单位"}, {"questionKind", "interaction"},
                    // 【逐行批注】 执行当前声明或表达式：{"questionId", "binding-interaction"}, {"question", {{"question", "你如何理解分数单位？"}, {"t…。
                    {"questionId", "binding-interaction"}, {"question", {{"question", "你如何理解分数单位？"}, {"type", "open"}}}, {"version", 1}},
                // 【逐行批注】 执行当前声明或表达式：{{"id", "practice"}, {"kind", "question"}, {"title", "练习"}, {"questionKind", "practi…。
                {{"id", "practice"}, {"kind", "question"}, {"title", "练习"}, {"questionKind", "practice"},
                    // 【逐行批注】 执行当前声明或表达式：{"questionId", "binding-practice"}, {"question", {{"question", "比较1/2与1/3。"}, {"type…。
                    {"questionId", "binding-practice"}, {"question", {{"question", "比较1/2与1/3。"}, {"type", "open"}}}, {"version", 1}}
            // 【逐行批注】 执行当前声明或表达式：}}};。
            }}};
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(ClassroomActivity{id, "binding-math", "agent-lesson"…。
        assert(db.upsert(ClassroomActivity{id, "binding-math", "agent-lesson", lesson.dump(), "now", 0, 0}));
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 调用 saveLesson 完成本行所需操作：saveLesson("binding-lesson");。
    saveLesson("binding-lesson");
    // 【逐行批注】 计算右侧结果并写入 auto practice，更新当前运行状态。
    const auto practice = agentSubmit(db, access, {{"type", "chat"}, {"courseId", "binding-math"}, {"lessonId", "binding-lesson"},
        // 【逐行批注】 执行当前声明或表达式：{"view", "practice"}, {"text", "先解释练习中的分数单位。"}, {"requestId", "binding-first-practic…。
        {"view", "practice"}, {"text", "先解释练习中的分数单位。"}, {"requestId", "binding-first-practice"}});
    // 【逐行批注】 计算右侧结果并写入 auto start，更新当前运行状态。
    const auto start = [&](Database& connection) { return agentStartLesson(connection, access, {{"lessonId", "binding-lesson"}}); };
    // 【逐行批注】 计算右侧结果并写入 deferred，更新当前运行状态。
    auto deferred = start(db);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(deferred.at("id") == practice.at("id") && deferred.at("initial…。
    assert(deferred.at("id") == practice.at("id") && deferred.at("initialTeachingDeferred") == true);
    // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(!Json::parse(db.getClassroomActivity("binding-lesson")->payloa…。
    assert(!Json::parse(db.getClassroomActivity("binding-lesson")->payload).contains("initialTeachingTaskId"));
    // 【逐行批注】 计算右侧结果并写入 int calls，更新当前运行状态。
    int calls = 0; bool replayOld = false;
    // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
    LearningAgent agent(path.string(), [&](const auto& options, const auto& sink) {
        // 【逐行批注】 执行当前声明或表达式：++calls;。
        ++calls;
        // 【逐行批注】 判断条件 (options.activity.taskId == practice.at("id") && !replayOld) {，成立时执行对应分支。
        if (options.activity.taskId == practice.at("id") && !replayOld) {
            // 【逐行批注】 执行当前声明或表达式：Database another; another.open(path.string());。
            Database another; another.open(path.string());
            // 【逐行批注】 计算右侧结果并写入 auto active，更新当前运行状态。
            const auto active = start(another);
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(active.at("id") == practice.at("id") && active.at("status") ==…。
            assert(active.at("id") == practice.at("id") && active.at("status") == "running" && active.at("initialTeachingDeferred") == true);
            // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(!Json::parse(another.getClassroomActivity("binding-lesson")->p…。
            assert(!Json::parse(another.getClassroomActivity("binding-lesson")->payload).contains("initialTeachingTaskId"));
            // 【逐行批注】 计算右侧结果并写入 auto before，更新当前运行状态。
            const auto before = another.getClassroomActivity("binding-lesson")->payload;
            // 【逐行批注】 计算右侧结果并写入 bool rejected，更新当前运行状态。
            bool rejected = false;
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try { another.transaction([&] { agentAttachLessonTask(another, access,
                // 【逐行批注】 执行当前声明或表达式：{{"type", "lesson_start"}, {"courseId", "binding-math"}, {"lessonId", "binding-lesso…。
                {{"type", "lesson_start"}, {"courseId", "binding-math"}, {"lessonId", "binding-lesson"}}, "concurrent-start"); }); }
            // 【逐行批注】 捕获并处理异常：catch (const std::invalid_argument&) { rejected = true; }。
            catch (const std::invalid_argument&) { rejected = true; }
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(rejected && another.getClassroomActivity("binding-lesson")->pa…。
            assert(rejected && another.getClassroomActivity("binding-lesson")->payload == before);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 Json action，更新当前运行状态。
        Json action = {{"id", replayOld ? "late-focus" : "choose-focus"}, {"tool", "select_teaching_focus"}, {"args", {
            // 【逐行批注】 执行当前声明或表达式：{"lessonId", "binding-lesson"}, {"sectionId", "concept"}, {"interactionSectionId", "…。
            {"lessonId", "binding-lesson"}, {"sectionId", "concept"}, {"interactionSectionId", "interaction"}, {"practiceSectionId", "practice"}}}};
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "请先说出你的理解，再继续教学。"}, {"actions", Json::arra…。
        return output({{"message", "请先说出你的理解，再继续教学。"}, {"actions", Json::array({action})}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：registerAgentLessonTools(agent); agent.process(db, practice.at("id"));。
    registerAgentLessonTools(agent); agent.process(db, practice.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, practice.at("id")).at("status") == "wait…。
    assert(agentView(db, access, practice.at("id")).at("status") == "waiting_student" && calls == 1);
    // 【逐行批注】 计算右侧结果并写入 auto initial，更新当前运行状态。
    const auto initial = start(db);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(initial.at("id") != practice.at("id") && initial.at("status") …。
    assert(initial.at("id") != practice.at("id") && initial.at("status") == "pending");
    // 【逐行批注】 计算右侧结果并写入 lesson，更新当前运行状态。
    auto lesson = Json::parse(db.getClassroomActivity("binding-lesson")->payload);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(lesson.at("initialTeachingTaskId") == initial.at("id") && less…。
    assert(lesson.at("initialTeachingTaskId") == initial.at("id") && lesson.at("teachingTaskId") == initial.at("id"));
    // 【逐行批注】 调用 assert、start 完成本行所需操作：assert(start(db).at("id") == initial.at("id") && calls == 1);。
    assert(start(db).at("id") == initial.at("id") && calls == 1);
    // 【逐行批注】 执行当前声明或表达式：agent.process(db, initial.at("id"));。
    agent.process(db, initial.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, initial.at("id")).at("status") == "waiti…。
    assert(agentView(db, access, initial.at("id")).at("status") == "waiting_student" && calls == 2);
    // 【逐行批注】 调用 assert、start 完成本行所需操作：assert(start(db).at("id") == initial.at("id") && calls == 2);。
    assert(start(db).at("id") == initial.at("id") && calls == 2);
    // 即使手动恢复旧任务，它也不能重新覆盖新任务保存的教学焦点。
    // 【逐行批注】 计算右侧结果并写入 oldRow，更新当前运行状态。
    auto oldRow = *db.getClassroomActivity(practice.at("id")); auto old = Json::parse(oldRow.payload);
    // 【逐行批注】 计算右侧结果并写入 old["status"]，更新当前运行状态。
    old["status"] = "failed"; oldRow.payload = old.dump(); assert(db.upsert(oldRow));
    // 【逐行批注】 计算右侧结果并写入 auto beforeReplay，更新当前运行状态。
    const auto beforeReplay = db.getClassroomActivity("binding-lesson")->payload;
    // 【逐行批注】 计算右侧结果并写入 replayOld，更新当前运行状态。
    replayOld = true; agentControl(db, access, {{"command", "retry"}, {"taskId", practice.at("id")}});
    // 【逐行批注】 执行当前声明或表达式：agent.process(db, practice.at("id"));。
    agent.process(db, practice.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, practice.at("id")).at("status") == "fail…。
    assert(agentView(db, access, practice.at("id")).at("status") == "failed");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity("binding-lesson")->payload == beforeRe…。
    assert(db.getClassroomActivity("binding-lesson")->payload == beforeReplay);
    // 【逐行批注】 按循环规则重复处理：for (const auto* status : {"pending", "running", "paused", "failed", "canc…。
    for (const auto* status : {"pending", "running", "paused", "failed", "cancelled", "superseded", "ready"}) {
        // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
        const auto id = std::string("binding-") + status; saveLesson(id);
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = agentSubmit(db, access, {{"type", "chat"}, {"courseId", "binding-math"}, {"lessonId", id},
            // 【逐行批注】 执行当前声明或表达式：{"view", "practice"}, {"text", "保留本次真实请求。"}, {"requestId", id}});。
            {"view", "practice"}, {"text", "保留本次真实请求。"}, {"requestId", id}});
        // 【逐行批注】 计算右侧结果并写入 row，更新当前运行状态。
        auto row = *db.getClassroomActivity(task.at("id")); auto raw = Json::parse(row.payload); raw["status"] = status;
        // 【逐行批注】 计算右侧结果并写入 row.payload，更新当前运行状态。
        row.payload = raw.dump(); assert(db.upsert(row));
        // 【逐行批注】 计算右侧结果并写入 auto resumed，更新当前运行状态。
        const auto resumed = agentStartLesson(db, access, {{"lessonId", id}});
        // 【逐行批注】 判断条件 (std::string(status) == "ready" || std::string(status) == "cance…，成立时执行对应分支。
        if (std::string(status) == "ready" || std::string(status) == "cancelled" || std::string(status) == "superseded") {
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(resumed.at("id") != task.at("id"));。
            assert(resumed.at("id") != task.at("id"));
            // 【逐行批注】 计算右侧结果并写入 auto bound，更新当前运行状态。
            const auto bound = Json::parse(db.getClassroomActivity(id)->payload);
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(bound.at("teachingTaskId") == resumed.at("id") && bound.at("in…。
            assert(bound.at("teachingTaskId") == resumed.at("id") && bound.at("initialTeachingTaskId") == resumed.at("id"));
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity(task.at("id"))->payload == row.payload…。
            assert(db.getClassroomActivity(task.at("id"))->payload == row.payload);
            // 【逐行批注】 调用 assert、agentStartLesson 完成本行所需操作：assert(agentStartLesson(db, access, {{"lessonId", id}}).at("id") == r…。
            assert(agentStartLesson(db, access, {{"lessonId", id}}).at("id") == resumed.at("id"));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行前一条件不成立时的分支。
        else {
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(resumed.at("id") == task.at("id") && resumed.at("initialTeachi…。
            assert(resumed.at("id") == task.at("id") && resumed.at("initialTeachingDeferred") == true);
            // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(!Json::parse(db.getClassroomActivity(id)->payload).contains("i…。
            assert(!Json::parse(db.getClassroomActivity(id)->payload).contains("initialTeachingTaskId"));
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

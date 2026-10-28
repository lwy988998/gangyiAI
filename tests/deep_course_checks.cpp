#ifdef NDEBUG
#undef NDEBUG
#endif
#include "deep_course.hpp"
#include "agent_lessons.hpp"
#include "agent_efficiency.hpp"
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace {
using Json = nlohmann::json;
void rejects(const std::function<void()>& operation) {
    bool rejected = false;
    try { operation(); } catch (const std::exception&) { rejected = true; }
    assert(rejected);
}
Json content() {
    Json result;
    result["lecture"] = Json::array({{{"id", "concept"}, {"title", "观察系数变化"}, {"body", "拖动参数，观察二次项系数与曲线开口的关系。"}, {"mediaIds", {"curve"}}}});
    result["slides"] = Json::array({{{"id", "slide-1"}, {"title", "从变化中理解"}, {"body", "先观察，再尝试解释。"}, {"mediaIds", {"curve"}}}});
    result["media"] = Json::array({{{"id", "curve"}, {"type", "function"}, {"title", "二次项系数"}, {"coefficients", {1, 0, 0}}, {"xMin", -5}, {"xMax", 5},
        {"parameter", {{"name", "a"}, {"min", -3}, {"max", 3}, {"value", 1}, {"step", .1}, {"coefficientIndex", 0}}}}});
    return result;
}

void runDeepPreparationProtocolChecks() {
    // 与生产入口一样注册原课堂工具，核对完整目录、补读说明与逐步备课协议。
    gangyi::Database db; db.open(":memory:"); db.migrate();
    const Json outline = {{"pedagogy", "deep-v1"}, {"courseStructure", {{{"stage", "函数"},
        {"topics", {{{"id", "a-role"}, {"title", "系数的作用"}, {"legacyPhaseIndex", 1}, {"legacyTopicIndex", 1}}}}}}}};
    assert(db.insert(gangyi::Course{"protocol", {}, {}, "观察系数", "deep", "观察系数", {}, "ai", "active", "2026-10-28", "2026-10-28"}));
    assert(db.insert(gangyi::CourseSnapshot{"protocol-outline", "protocol", 1, outline.dump(), "2026-10-28"}));
    const gangyi::AgentAccess access{"protocol-fixture", {"protocol"}};
    const Json scope = {{"courseId", "protocol"}, {"topicId", "a-role"}};
    const auto task = gangyi::deepCourseStart(db, access, scope).at("task");
    const Json names = {"create_lesson", "append_section", "finish_lesson", "set_deep_content", "search_lesson_images"};
    int calls = 0;
    std::string lessonId;
    gangyi::LearningAgent agent("", [&](const gangyi::ChatOptions& options, const auto&) {
        ++calls;
        Json actions = Json::array();
        if (calls == 1) {
            Json catalog;
            for (const auto& message : options.messages) {
                const auto marker = message.content.find("可用工具：");
                if (message.role == "system" && marker != std::string::npos)
                    catalog = Json::parse(message.content.substr(marker + std::string("可用工具：").size()));
            }
            assert(catalog.is_array());
            for (const auto& name : names) {
                bool found = false;
                for (const auto& tool : catalog) if (tool.at("name") == name) {
                    found = true;
                    assert(!tool.contains("detailsTool"));
                    assert(tool.at("description").get<std::string>().find("参数") != std::string::npos);
                }
                assert(found);
            }
            actions.push_back({{"id", "describe"}, {"tool", "describe_tools"}, {"args", {{"names", names}}}});
        } else {
            const auto returned = Json::parse(options.messages.back().content).at("toolResults");
            if (calls == 2) {
                const auto descriptions = returned.at(0).at("result").at("tools");
                assert(descriptions.size() == names.size());
                for (const auto& tool : descriptions) {
                    const auto description = tool.at("description").get<std::string>();
                    if (tool.at("name") == "set_deep_content")
                        assert(description.find("sectionId") != std::string::npos && description.find("sourceSectionIds") != std::string::npos && description.find("coefficients") != std::string::npos);
                    if (tool.at("name") == "search_lesson_images")
                        assert(description.find("mediaId") != std::string::npos && description.find("refresh:true") != std::string::npos);
                }
                actions.push_back({{"id", "create"}, {"tool", "create_lesson"}, {"args", {{"courseId", "protocol"}, {"topicId", "a-role"},
                    {"title", "观察系数"}, {"purpose", "依据函数式解释图像变化"}, {"intent", "advance"}}}});
            } else if (calls == 3) {
                lessonId = returned.at(0).at("result").at("lessonId");
                actions.push_back({{"id", "explain"}, {"tool", "append_section"}, {"args", {{"lessonId", lessonId},
                    {"section", {{"kind", "explanation"}, {"title", "观察系数"}, {"body", "保持常数项与一次项不变，比较二次项系数。"}, {"helpType", "explanation"}}}}}});
                actions.push_back({{"id", "question"}, {"tool", "append_section"}, {"args", {{"lessonId", lessonId},
                    {"section", {{"kind", "question"}, {"title", "试着解释"}, {"questionKind", "practice"}, {"question", {{"question", "改变二次项系数后观察到了什么？"},
                        {"type", "open"}, {"expectedAnswer", "备课私有答案"}, {"rubric", "备课私有标准"}}}}}}}});
            } else if (calls == 4) {
                const auto sectionId = returned.at(0).at("result").at("sectionId");
                auto teaching = content(); teaching["lecture"][0]["sectionId"] = sectionId;
                teaching["slides"][0]["sourceSectionIds"] = {sectionId};
                actions.push_back({{"id", "content"}, {"tool", "set_deep_content"}, {"args", {{"lessonId", lessonId}, {"content", teaching}}}});
            } else {
                assert(calls == 5 && returned.at(0).at("result").at("lectureCount") == 1);
                actions.push_back({{"id", "finish"}, {"tool", "finish_lesson"}, {"args", {{"lessonId", lessonId}}}});
            }
        }
        const Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", ""}, {"actions", actions}, {"state", calls == 5 ? "completed" : "continue"}};
        gangyi::AIResult result; result.content = decision.dump(); result.model = "fixture-complete-deep-protocol"; result.finishReason = "stop"; return result;
    });
    gangyi::registerAgentLessonTools(agent);
    agent.process(db, task.at("id"));
    const auto completed = gangyi::agentView(db, access, task.at("id"));
    if (completed.at("status") != "ready") std::cerr << completed.dump(2) << '\n';
    assert(completed.at("status") == "ready" && calls == 5);
    const auto view = gangyi::deepCourseView(db, access, scope);
    assert(view.at("lesson").at("id") == lessonId && view.at("lesson").at("status") == "ready");
    assert(view.at("lesson").at("sections").size() == 2 && view.at("content").at("slides").size() == 1);
    assert(view.at("content").at("lecture").at(0).at("body") == "保持常数项与一次项不变，比较二次项系数。");
    assert(view.dump().find("备课私有") == std::string::npos && db.listCourses().size() == 1);
}

void runDeepDraftRecoveryChecks() {
    gangyi::Database db; db.open(":memory:"); db.migrate();
    gangyi::AgentAccess access{"draft-fixture", {}};
    const Json outline = {{"courseStructure", {{{"stage", "事件顺序"}, {"topics", {{{"id", "timeline"}, {"title", "四个事件"}}}}}}}};
    int planningCalls = 0;
    gangyi::LearningAgent planner("", [&](const gangyi::ChatOptions&, const auto&) {
        ++planningCalls; assert(planningCalls == 1);
        const Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", ""}, {"state", "continue"},
            {"actions", {{{"id", "course"}, {"tool", "create_course"}, {"args", {{"goal", "事件先后"}, {"title", "事件先后"}, {"mode", "deep"}, {"payload", outline}}}},
                {{"id", "premature-lesson"}, {"tool", "create_lesson"}, {"args", Json::object()}}}}};
        gangyi::AIResult result; result.content = decision.dump(); result.model = "fixture-outline-only"; result.finishReason = "stop"; return result;
    });
    gangyi::registerAgentLessonTools(planner);
    const auto plan = gangyi::agentSubmit(db, access, {{"type", "plan_course"}, {"mode", "deep"}, {"goal", "事件先后"}, {"requestId", "outline-first"}});
    planner.process(db, plan.at("id"));
    for (const auto& course : db.listCourses()) access.courseIds.push_back(course.id);
    const auto courseId = db.listCourses().at(0).id;
    assert(gangyi::agentView(db, access, plan.at("id")).at("status") == "ready" && planningCalls == 1);
    for (const auto& row : db.listClassroomActivities(courseId)) assert(row.kind != "agent-lesson");
    const Json planningTask = {{"event", {{"type", "plan_course"}, {"mode", "deep"}}}};
    for (const auto* name : {"create_lesson", "append_section", "finish_lesson", "complete_lesson", "select_teaching_focus", "set_deep_content", "search_lesson_images",
        "revise_outline", "adjust_goal", "update_course_preview", "schedule_review", "read_legacy_lesson", "select_resources", "update_topic_mastery", "update_profiles"})
        assert(!gangyi::agentToolAuthorized(name, planningTask));
    assert(gangyi::agentToolAuthorized("create_lesson", {{"event", {{"type", "plan_course"}, {"mode", "lite"}}}}));

    // 模拟旧执行路径已保留课程与草稿后失败：恢复规划不调用模型，也不把草稿标为ready。
    auto savedPlan = *db.getClassroomActivity(plan.at("id")); auto failed = Json::parse(savedPlan.payload);
    failed["status"] = "failed"; failed["error"] = "测试预算不足"; savedPlan.payload = failed.dump(); assert(db.upsert(savedPlan));
    Json draft = {{"id", "saved-draft"}, {"scopeId", access.scopeId}, {"courseId", courseId}, {"topicId", "timeline"}, {"phaseIndex", 1}, {"topicIndex", 1},
        {"title", "已生成的事件讲义"}, {"purpose", "理解先后"}, {"intent", "advance"}, {"status", "draft"}, {"version", 1}, {"entered", false},
        {"sourceLearningVersion", db.learningRevision()}, {"taskId", plan.at("id")}, {"updatedAt", "2026-10-28"},
        {"sections", {{{"id", "saved-explanation"}, {"kind", "explanation"}, {"title", "已生成讲义"}, {"body", "先比较年份，再比较月份。"}, {"helpType", "explanation"}, {"version", 1}, {"displayed", false}}}}};
    assert(db.upsert(gangyi::ClassroomActivity{"saved-draft", courseId, "agent-lesson", draft.dump(), "2026-10-28", 1, 1}));
    gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", plan.at("id")}});
    planner.process(db, plan.at("id"));
    assert(planningCalls == 1 && gangyi::agentView(db, access, plan.at("id")).at("status") == "ready");
    assert(Json::parse(db.getClassroomActivity("saved-draft")->payload).at("status") == "draft");
    rejects([&] { gangyi::deepCourseStart(db, {"another-scope", {courseId}}, {{"lessonId", "saved-draft"}}); });
    rejects([&] { gangyi::deepCourseStart(db, access, {{"lessonId", "saved-draft"}, {"topicId", "another-topic"}}); });
    auto staleRow = *db.getClassroomActivity("saved-draft"); auto stale = draft; stale["sourceLearningVersion"] = db.learningRevision() - 1;
    staleRow.payload = stale.dump(); assert(db.upsert(staleRow));
    rejects([&] { gangyi::deepCourseStart(db, access, {{"lessonId", "saved-draft"}}); });
    staleRow.payload = draft.dump(); assert(db.upsert(staleRow));

    const auto resumed = gangyi::deepCourseStart(db, access, {{"lessonId", "saved-draft"}}).at("task");
    const auto same = gangyi::deepCourseStart(db, access, {{"courseId", courseId}, {"topicId", "timeline"}, {"requestId", "another-window"}}).at("task");
    assert(resumed.at("id") == same.at("id") && resumed.at("id") != plan.at("id"));
    const auto resumedRaw = Json::parse(db.getClassroomActivity(resumed.at("id"))->payload);
    assert(resumedRaw.at("event").at("resumeDraft") == true && resumedRaw.at("event").at("lessonId") == "saved-draft");
    assert(!gangyi::agentToolAuthorized("create_lesson", resumedRaw));
    for (const auto* name : {"complete_lesson", "select_teaching_focus", "update_profiles", "adjust_goal", "revise_outline"})
        assert(!gangyi::agentToolAuthorized(name, resumedRaw));
    auto other = draft; other["id"] = "other-saved-lesson"; other["status"] = "ready"; other["intent"] = "general"; other["topicId"] = "old-topic";
    other["deepContent"] = gangyi::normalizeDeepContent(content());
    assert(db.upsert(gangyi::ClassroomActivity{"other-saved-lesson", courseId, "agent-lesson", other.dump(), "2026-10-28", 1, 2}));
    const auto otherPayload = db.getClassroomActivity("other-saved-lesson")->payload;
    int preparationCalls = 0;
    gangyi::LearningAgent teacher("", [&](const gangyi::ChatOptions&, const auto&) {
        ++preparationCalls; assert(preparationCalls <= 3);
        assert(db.getClassroomActivity("other-saved-lesson")->payload == otherPayload);
        auto material = content(); material["lecture"][0]["sectionId"] = "saved-explanation"; material["slides"][0]["sourceSectionIds"] = {"saved-explanation"};
        Json actions = {
            {{"id", "content"}, {"tool", "set_deep_content"}, {"args", {{"lessonId", "saved-draft"}, {"content", material}}}},
            {{"id", "finish"}, {"tool", "finish_lesson"}, {"args", {{"lessonId", "saved-draft"}}}}};
        if (preparationCalls == 1) actions = {{{"id", "wrong-append"}, {"tool", "append_section"}, {"args", {{"lessonId", "other-saved-lesson"},
            {"section", {{"kind", "explanation"}, {"title", "不能写到旧课"}, {"body", "错误位置"}, {"helpType", "explanation"}}}}}}};
        if (preparationCalls == 2) actions = {{{"id", "wrong-finish"}, {"tool", "finish_lesson"}, {"args", {{"lessonId", "other-saved-lesson"}}}}};
        const Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", ""}, {"state", "completed"}, {"actions", actions}};
        gangyi::AIResult result; result.content = decision.dump(); result.model = "fixture-resume-draft"; result.finishReason = "stop"; return result;
    });
    gangyi::registerAgentLessonTools(teacher); teacher.process(db, resumed.at("id"));
    const auto completed = gangyi::agentView(db, access, resumed.at("id"));
    if (completed.at("status") != "ready") std::cerr << completed.dump(2) << '\n';
    assert(completed.at("status") == "ready" && preparationCalls == 3 && db.listCourses().size() == 1);
    assert(completed.at("lesson").at("id") == "saved-draft" && db.getClassroomActivity("other-saved-lesson")->payload == otherPayload);
    const auto restored = Json::parse(db.getClassroomActivity("saved-draft")->payload);
    assert(restored.at("status") == "ready" && restored.at("sections") == draft.at("sections"));
    int lessonCount = 0; for (const auto& row : db.listClassroomActivities(courseId)) if (row.kind == "agent-lesson") ++lessonCount;
    assert(lessonCount == 2);

    // 即使模型把旧课完成操作放在新建大纲之前，执行层也不能改旧课状态与进度。
    gangyi::agentEnterLesson(db, access, {{"lessonId", "saved-draft"}});
    const auto existingLesson = db.getClassroomActivity("saved-draft")->payload;
    assert(!db.findLearningCardProgress(courseId, 1, 1));
    const auto previousGoal = db.getCourse(courseId)->goal, previousAbilities = db.profileMeta("ability-profile");
    for (const std::string forbidden : {"complete_lesson", "adjust_goal", "update_profiles"}) {
      int guardedCalls = 0;
      gangyi::LearningAgent guarded("", [&](const gangyi::ChatOptions&, const auto&) {
        ++guardedCalls; assert(guardedCalls <= 2);
        assert(db.getClassroomActivity("saved-draft")->payload == existingLesson && !db.findLearningCardProgress(courseId, 1, 1));
        assert(db.getCourse(courseId)->goal == previousGoal && db.profileMeta("ability-profile") == previousAbilities);
        Json actions = Json::array();
        Json forbiddenArgs = {{"lessonId", "saved-draft"}, {"reason", "无权借规划修改旧进度"}};
        if (forbidden == "adjust_goal") forbiddenArgs = {{"courseId", courseId}, {"goal", "无权替换旧目标"}, {"reason", "新建课程不能修改旧目标"}};
        if (forbidden == "update_profiles") forbiddenArgs = {{"subjects", {{{"subject", "数学"}, {"score", nullptr}, {"sufficient", false}, {"rationale", "不能清除旧画像"}, {"recommendation", "等待"}, {"uncertainty", "未测试"}, {"evidenceIds", Json::array()}}}}};
        if (guardedCalls == 1) actions.push_back({{"id", "forbidden-old-write"}, {"tool", forbidden}, {"args", forbiddenArgs}});
        actions.push_back({{"id", "new-course"}, {"tool", "create_course"}, {"args", {{"goal", "另一个新目标"}, {"title", "另一个新目标"}, {"mode", "deep"}, {"payload", outline}}}});
        const Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", ""}, {"state", "completed"}, {"actions", actions}};
        gangyi::AIResult result; result.content = decision.dump(); result.model = "fixture-planning-write-boundary"; result.finishReason = "stop"; return result;
      });
      gangyi::registerAgentLessonTools(guarded);
      const auto guardedTask = gangyi::agentSubmit(db, access, {{"type", "plan_course"}, {"mode", "deep"}, {"goal", "另一个新目标"}, {"requestId", "planning-write-boundary-" + forbidden}});
      guarded.process(db, guardedTask.at("id"));
      for (const auto& course : db.listCourses()) if (std::find(access.courseIds.begin(), access.courseIds.end(), course.id) == access.courseIds.end()) access.courseIds.push_back(course.id);
      assert(gangyi::agentView(db, access, guardedTask.at("id")).at("status") == "ready" && guardedCalls == 2);
      assert(db.getClassroomActivity("saved-draft")->payload == existingLesson && !db.findLearningCardProgress(courseId, 1, 1));
      assert(db.getCourse(courseId)->goal == previousGoal && db.profileMeta("ability-profile") == previousAbilities);
    }
    assert(db.listCourses().size() == 4);
}
}

void runDeepCourseChecks() {
    runDeepPreparationProtocolChecks();
    runDeepDraftRecoveryChecks();
    const auto normalized = gangyi::normalizeDeepContent(content());
    assert(normalized.at("media").at(0).at("coefficients").size() == 3);
    auto invalid = content(); invalid["media"][0]["xMin"] = 10;
    rejects([&] { gangyi::normalizeDeepContent(invalid); });
    invalid = content(); invalid["media"][0]["parameter"]["coefficientIndex"] = 4;
    rejects([&] { gangyi::normalizeDeepContent(invalid); });
    invalid = content(); invalid["lecture"][0]["mediaIds"] = {"missing"};
    rejects([&] { gangyi::normalizeDeepContent(invalid); });
    invalid = content(); invalid["media"][0]["type"] = "javascript";
    rejects([&] { gangyi::normalizeDeepContent(invalid); });
    auto extra = content(); extra["lecture"][0]["script"] = "throw new Error('not allowed')";
    assert(!gangyi::normalizeDeepContent(extra)["lecture"][0].contains("script"));
    auto imageInput = content(); imageInput["media"] = {{{"id", "curve"}, {"type", "image"}, {"title", "图片"}, {"url", "http://127.0.0.1/private"}}};
    const auto sanitized = gangyi::normalizeDeepContent(imageInput)["media"][0];
    assert(sanitized.at("status") == "pending" && !sanitized.contains("url"));
    const auto images = gangyi::deepImageCandidates("<meta content='/pictures/lesson.png' property='og:image'><img alt='标志' src='/logo.png'><img alt='不可访问' src='http://127.0.0.1/a.png'>", "https://example.org/course/lesson");
    assert(images.size() == 1 && images.at(0).at("url") == "https://example.org/pictures/lesson.png");
    const std::string imagePage = "<meta property='og:image' content='/hero.webp'><img src='/satellite.webp' alt='Satellite dishes.'>"
        "<figure><img src='/figure-12.webp' alt='Graph of a parabola opening upward.'><figcaption>Quadratic function graph.</figcaption></figure>"
        "<img src='/function-cover.webp' alt='Cover of the function textbook.'>";
    const auto ranked = gangyi::deepImageCandidates(imagePage, "https://example.org/quadratic-functions", "quadratic function parabola graph opens upward downward", "二次函数参考图");
    assert(!ranked.empty() && ranked.at(0).at("url") == "https://example.org/figure-12.webp");
    for (const auto& item : ranked) assert(item.at("url") != "https://example.org/satellite.webp" && item.at("url") != "https://example.org/hero.webp");
    assert(gangyi::deepImageCandidates("<img src='/photo.webp' alt='Satellite dishes.'>", "https://example.org/quadratic-functions", "parabola graph", "函数图").empty());
    const auto captionMatch = gangyi::deepImageCandidates("<div data-type='figure'><div><img src='/event.webp' alt='示意图'></div><span data-type='caption'>法国大革命攻占巴士底狱的历史场景。</span></div>",
        "https://example.org/history", "法国大革命 攻占巴士底狱", "历史图片");
    assert(captionMatch.size() == 1 && !captionMatch.at(0).at("sourceCaption").get<std::string>().empty());
    const auto duplicateImage = gangyi::deepImageCandidates("<meta property='og:image' content='/curve.webp'><figure><img src='/curve.webp' alt='Parabola graph'><figcaption>Graph of a function.</figcaption></figure>",
        "https://example.org/math", "parabola graph", "参考图");
    assert(duplicateImage.size() == 1 && duplicateImage.at(0).at("alt") == "Parabola graph");
    std::string laterFigure;
    for (int i = 0; i < 12; ++i) laterFigure += "<img src='/photo-" + std::to_string(i) + ".webp' alt='Unrelated photograph'>";
    laterFigure += "<figure><img src='/late-graph.webp' alt='Parabola graph'></figure>";
    assert(gangyi::deepImageCandidates(laterFigure, "https://example.org/math", "parabola graph", "参考图").at(0).at("url") == "https://example.org/late-graph.webp");

    gangyi::Database db; db.open(":memory:"); db.migrate();
    const auto outline = Json{{"courseStructure", {{{"stage", "函数的变化"}, {"topics", {{{"id", "topic-1"}, {"title", "二次函数"}, {"legacyPhaseIndex", 1}, {"legacyTopicIndex", 1}}}}}}}};
    assert(db.insert(gangyi::Course{"old", {}, {}, "旧课程", "deep", "旧课程", {}, "ai", "active", "2026-10-28", "2026-10-28"}));
    assert(db.insert(gangyi::CourseSnapshot{"snapshot-old", "old", 1, outline.dump(), "2026-10-28"}));
    assert(!gangyi::deepCourseEnabled(db, "old"));
    auto deepOutline = outline; deepOutline["pedagogy"] = "deep-v1";
    assert(db.insert(gangyi::Course{"new", {}, {}, "二次函数", "deep", "二次函数", {}, "ai", "active", "2026-10-28", "2026-10-28"}));
    assert(db.insert(gangyi::CourseSnapshot{"snapshot-new", "new", 1, deepOutline.dump(), "2026-10-28"}));
    gangyi::AgentAccess access{"fixture", {"old", "new"}};
    assert(gangyi::deepCourseEnabled(db, "new"));
    for (const std::string mode : {"lite", "deep"}) {
        gangyi::LearningAgent planner("", [&](const gangyi::ChatOptions&, const auto&) {
            Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", ""}, {"state", "completed"},
                {"actions", {{{"id", "create"}, {"tool", "create_course"}, {"args", {{"goal", "课程模式校验"}, {"title", "课程模式校验"}, {"mode", mode == "deep" ? "lite" : "deep"}, {"payload", deepOutline}}}}}}};
            gangyi::AIResult result; result.content = decision.dump(); result.model = "fixture-mode-choice"; result.finishReason = "stop"; return result;
        });
        const auto planning = gangyi::agentSubmit(db, access, {{"type", "plan_course"}, {"mode", mode}, {"goal", "课程模式校验"}, {"requestId", "mode-" + mode}});
        planner.process(db, planning.at("id"));
        // 产品每个HTTP请求都会重新读取活跃课程；测试也要刷新新建课程的访问范围。
        const auto previousAccess = access;
        access.courseIds.clear();
        for (const auto& course : db.listCourses()) if (course.status == "active") access.courseIds.push_back(course.id);
        rejects([&] { gangyi::agentView(db, previousAccess, planning.at("id")); });
        const auto planned = gangyi::agentView(db, access, planning.at("id"));
        if (planned.at("status") != "ready") std::cerr << planned.dump(2) << '\n';
        assert(planned.at("status") == "ready");
        const auto createdId = planned.at("course").at("id").get<std::string>();
        assert(db.getCourse(createdId)->mode == mode);
        assert(gangyi::deepCourseEnabled(db, createdId) == (mode == "deep"));
    }
    const auto classroomSnapshot = [&] {
        Json result = Json::object();
        for (const auto& row : db.listClassroomActivities("new"))
            result[row.id] = {{"courseId", row.courseId}, {"kind", row.kind}, {"payload", row.payload},
                {"updatedAt", row.updatedAt}, {"phaseIndex", row.phaseIndex}, {"topicIndex", row.topicIndex}};
        return result;
    };
    const auto revision = db.learningRevision();
    const auto beforeRead = classroomSnapshot();
    assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}}).at("status") == "unprepared");
    assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"phaseIndex", "1"}, {"topicIndex", "1"}}).at("status") == "unprepared");
    rejects([&] { gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"phaseIndex", "1.2"}}); });
    rejects([&] { gangyi::deepCourseStart(db, access, {{"courseId", "new"}, {"phaseIndex", -1}}); });
    assert(db.learningRevision() == revision);
    assert(classroomSnapshot() == beforeRead);
    rejects([&] { gangyi::deepCourseView(db, {"other", {}}, {{"courseId", "new"}}); });
    const auto preparing = gangyi::deepCourseStart(db, access, {{"courseId", "new"}, {"phaseIndex", "1"}, {"topicIndex", "1"}, {"requestId", "client-a"}});
    const auto preparingOtherWindow = gangyi::deepCourseStart(db, access, {{"courseId", "new"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"requestId", "client-b"}});
    assert(preparing.at("task").at("id") == preparingOtherWindow.at("task").at("id"));

    auto lesson = Json{{"id", "lesson-new"}, {"scopeId", "fixture"}, {"courseId", "new"}, {"title", "二次函数"}, {"purpose", "观察系数与图像"},
        {"status", "ready"}, {"version", 1}, {"sourceLearningVersion", db.learningRevision()}, {"entered", true}, {"intent", "advance"},
        {"phaseIndex", 1}, {"topicIndex", 1}, {"topicId", "topic-1"}, {"updatedAt", "2026-10-28"},
        {"sections", {{{"id", "section-q"}, {"kind", "question"}, {"title", "试着解释"}, {"version", 1}, {"displayed", false},
            {"questionKind", "practice"}, {"questionId", "question-new"}, {"question", {{"question", "a增加时图像怎样变化？"}, {"type", "open"}, {"expectedAnswer", "私有答案"}, {"rubric", "私有标准"}}}}}},
        {"deepContent", normalized}};
    lesson["sections"].push_back({{"id", "concept"}, {"kind", "explanation"}, {"title", "观察系数变化"}, {"body", "已保存且实际可展示的讲解正文。"}, {"helpType", "explanation"}, {"version", 1}, {"displayed", false}});
    assert(db.upsert(gangyi::ClassroomActivity{"lesson-new", "new", "agent-lesson", lesson.dump(), "2026-10-28", 0, 0}));
    const auto view = gangyi::deepCourseView(db, access, {{"lessonId", "lesson-new"}});
    assert(view.at("lesson").at("sections").at(0).at("question").at("question") == "a增加时图像怎样变化？");
    assert(view.dump().find("私有答案") == std::string::npos && view.dump().find("私有标准") == std::string::npos);
    assert(view.at("content").at("slides").size() == 1);
    assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"phaseIndex", 1}, {"topicIndex", 1}}).at("lesson").at("id") == "lesson-new");
    assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"topicId", "topic-1"}, {"phaseIndex", 99}, {"topicIndex", 99}}).at("lesson").at("id") == "lesson-new");
    rejects([&] { gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"topicId", "removed-topic"}}); });

    int calls = 0;
    gangyi::LearningAgent agent("", [&](const gangyi::ChatOptions&, const auto&) {
        ++calls;
        auto changed = content(); changed["lecture"][0]["body"] = "不能冒充已保存正文的其它文字。";
        changed["lecture"][0]["sectionId"] = "concept";
        changed["slides"][0]["sourceSectionIds"] = {"concept"};
        Json output = {{"teaching", {{"helpType", "none"}, {"demonstratedQuestions", Json::array()}}}, {"message", ""},
            {"actions", {{{"id", "save-content"}, {"tool", "set_deep_content"}, {"args", {{"lessonId", "lesson-new"}, {"content", changed}}}}}}, {"state", "completed"}};
        gangyi::AIResult result; result.content = output.dump(); result.model = "fixture-deep-protocol"; result.finishReason = "stop"; return result;
    });
    const auto task = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"courseId", "new"}, {"lessonId", "lesson-new"}, {"text", "补充观察步骤"}, {"requestId", "deep-change"}});
    agent.process(db, task.at("id"));
    const auto savedTask = gangyi::agentView(db, access, task.at("id"));
    if (savedTask.at("status") != "ready") std::cerr << savedTask.dump(2) << '\n';
    assert(savedTask.at("status") == "ready");
    assert(calls == 1);
    const auto updated = gangyi::deepCourseView(db, access, {{"lessonId", "lesson-new"}});
    assert(updated.at("content").at("lecture").at(0).at("body") == "已保存且实际可展示的讲解正文。");
    assert(updated.at("content").at("slides").at(0).at("sourceSectionIds").at(0) == "concept");
    assert(updated.at("content").at("slides").at(0).at("body") == "已保存且实际可展示的讲解正文。");
    assert(gangyi::deepCourseRetryMedia(db, access, {{"lessonId", "lesson-new"}, {"mediaId", "curve"}, {"requestId", "retry-ready"}}).at("cached") == true);
    const auto updatedRevision = db.learningRevision();
    for (int count = 0; count < 3; ++count) gangyi::deepCourseView(db, access, {{"lessonId", "lesson-new"}});
    assert(calls == 1 && db.learningRevision() == updatedRevision);

    const auto second = gangyi::deepCourseStart(db, access, {{"courseId", "new"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"requestId", "start"}});
    assert(second.at("lessonId") == "lesson-new");
    const auto same = gangyi::deepCourseStart(db, access, {{"courseId", "new"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"requestId", "start"}});
    assert(second.at("cached") == true && same.at("cached") == true && !second.contains("task"));
    rejects([&] { gangyi::deepCourseImage(db, access, "lesson-new", "curve"); });
    assert(db.saveMyAIRecord({"image-new", "deep-image", Json{{"courseId", "new"}, {"data", "image-bytes-must-not-enter-context"}}.dump(), "2026-10-28", 1}, 0));
    assert(db.saveMyAIRecord({"image-old", "deep-image", Json{{"courseId", "old"}, {"data", "other-course"}}.dump(), "2026-10-28", 1}, 0));
    assert(gangyi::agentContext(db, access, {{"type", "chat"}, {"courseId", "new"}}).dump().find("image-bytes-must-not-enter-context") == std::string::npos);
    db.deleteClassroomData("new");
    assert(!db.myAIRecord("image-new") && db.myAIRecord("image-old"));
    std::cout << "深度课程标记兼容、结构校验、图片来源解析、私有答案过滤、主控素材保存与读取复用检查通过；付费请求0。\n";
}

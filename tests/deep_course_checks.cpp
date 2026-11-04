// 【逐行批注】 处理条件编译指令：#ifdef NDEBUG。
#ifdef NDEBUG
// 【逐行批注】 执行当前声明或表达式：#undef NDEBUG。
#undef NDEBUG
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 "deep_course.hpp"，为后续实现提供类型或接口声明。
#include "deep_course.hpp"
// 【逐行批注】 引入头文件 "agent_lessons.hpp"，为后续实现提供类型或接口声明。
#include "agent_lessons.hpp"
// 【逐行批注】 引入头文件 "agent_efficiency.hpp"，为后续实现提供类型或接口声明。
#include "agent_efficiency.hpp"
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <cassert>，为后续实现提供类型或接口声明。
#include <cassert>
// 【逐行批注】 引入头文件 <functional>，为后续实现提供类型或接口声明。
#include <functional>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>

// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 定义函数 rejects，执行该函数声明的职责。
void rejects(const std::function<void()>& operation) {
    // 【逐行批注】 计算右侧结果并写入 bool rejected，更新当前运行状态。
    bool rejected = false;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { operation(); } catch (const std::exception&) { rejected = true; }
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(rejected);。
    assert(rejected);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 content，执行该函数声明的职责。
Json content() {
    // 【逐行批注】 执行当前声明或表达式：Json result;。
    Json result;
    // 【逐行批注】 计算右侧结果并写入 result["lecture"]，更新当前运行状态。
    result["lecture"] = Json::array({{{"id", "concept"}, {"title", "观察系数变化"}, {"body", "拖动参数，观察二次项系数与曲线开口的关系。"}, {"mediaIds", {"curve"}}}});
    // 【逐行批注】 计算右侧结果并写入 result["slides"]，更新当前运行状态。
    result["slides"] = Json::array({{{"id", "slide-1"}, {"title", "从变化中理解"}, {"body", "先观察，再尝试解释。"}, {"mediaIds", {"curve"}}}});
    // 【逐行批注】 计算右侧结果并写入 result["media"]，更新当前运行状态。
    result["media"] = Json::array({{{"id", "curve"}, {"type", "function"}, {"title", "二次项系数"}, {"coefficients", {1, 0, 0}}, {"xMin", -5}, {"xMax", 5},
        // 【逐行批注】 执行当前声明或表达式：{"parameter", {{"name", "a"}, {"min", -3}, {"max", 3}, {"value", 1}, {"step", .1}, {…。
        {"parameter", {{"name", "a"}, {"min", -3}, {"max", 3}, {"value", 1}, {"step", .1}, {"coefficientIndex", 0}}}}});
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 runDeepPreparationProtocolChecks，执行该函数声明的职责。
void runDeepPreparationProtocolChecks() {
    // 与生产入口一样注册原课堂工具，核对完整目录、补读说明与逐步备课协议。
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(":memory:"); db.migrate();。
    gangyi::Database db; db.open(":memory:"); db.migrate();
    // 【逐行批注】 计算右侧结果并写入 Json outline，更新当前运行状态。
    const Json outline = {{"pedagogy", "deep-v1"}, {"courseStructure", {{{"stage", "函数"},
        // 【逐行批注】 执行当前声明或表达式：{"topics", {{{"id", "a-role"}, {"title", "系数的作用"}, {"legacyPhaseIndex", 1}, {"legacy…。
        {"topics", {{{"id", "a-role"}, {"title", "系数的作用"}, {"legacyPhaseIndex", 1}, {"legacyTopicIndex", 1}}}}}}}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::Course{"protocol", {}, {}, "观察系数", "deep", "…。
    assert(db.insert(gangyi::Course{"protocol", {}, {}, "观察系数", "deep", "观察系数", {}, "ai", "active", "2026-10-28", "2026-10-28"}));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::CourseSnapshot{"protocol-outline", "protocol…。
    assert(db.insert(gangyi::CourseSnapshot{"protocol-outline", "protocol", 1, outline.dump(), "2026-10-28"}));
    // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess access{"protocol-fixture", {"protocol"}};。
    const gangyi::AgentAccess access{"protocol-fixture", {"protocol"}};
    // 【逐行批注】 计算右侧结果并写入 Json scope，更新当前运行状态。
    const Json scope = {{"courseId", "protocol"}, {"topicId", "a-role"}};
    // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
    const auto task = gangyi::deepCourseStart(db, access, scope).at("task");
    // 【逐行批注】 计算右侧结果并写入 Json names，更新当前运行状态。
    const Json names = {"create_lesson", "append_section", "finish_lesson", "set_deep_content", "search_lesson_images"};
    // 【逐行批注】 计算右侧结果并写入 int calls，更新当前运行状态。
    int calls = 0;
    // 【逐行批注】 执行当前声明或表达式：std::string lessonId;。
    std::string lessonId;
    // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
    gangyi::LearningAgent agent("", [&](const gangyi::ChatOptions& options, const auto&) {
        // 【逐行批注】 执行当前声明或表达式：++calls;。
        ++calls;
        // 【逐行批注】 计算右侧结果并写入 Json actions，更新当前运行状态。
        Json actions = Json::array();
        // 【逐行批注】 判断条件 (calls == 1) {，成立时执行对应分支。
        if (calls == 1) {
            // 【逐行批注】 执行当前声明或表达式：Json catalog;。
            Json catalog;
            // 【逐行批注】 按循环规则重复处理：for (const auto& message : options.messages) {。
            for (const auto& message : options.messages) {
                // 【逐行批注】 计算右侧结果并写入 auto marker，更新当前运行状态。
                const auto marker = message.content.find("可用工具：");
                // 【逐行批注】 判断条件 (message.role == "system" && marker != std::string::npos)，成立时执行对应分支。
                if (message.role == "system" && marker != std::string::npos)
                    // 【逐行批注】 计算右侧结果并写入 catalog，更新当前运行状态。
                    catalog = Json::parse(message.content.substr(marker + std::string("可用工具：").size()));
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(catalog.is_array());。
            assert(catalog.is_array());
            // 【逐行批注】 按循环规则重复处理：for (const auto& name : names) {。
            for (const auto& name : names) {
                // 【逐行批注】 计算右侧结果并写入 bool found，更新当前运行状态。
                bool found = false;
                // 【逐行批注】 按循环规则重复处理：for (const auto& tool : catalog) if (tool.at("name") == name) {。
                for (const auto& tool : catalog) if (tool.at("name") == name) {
                    // 【逐行批注】 计算右侧结果并写入 found，更新当前运行状态。
                    found = true;
                    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!tool.contains("detailsTool"));。
                    assert(!tool.contains("detailsTool"));
                    // 【逐行批注】 调用 assert 完成本行所需操作：assert(tool.at("description").get<std::string>().find("参数") != std::s…。
                    assert(tool.at("description").get<std::string>().find("参数") != std::string::npos);
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(found);。
                assert(found);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "describe"}, {"tool", "describe_tools"}, {"args", {{"names…。
            actions.push_back({{"id", "describe"}, {"tool", "describe_tools"}, {"args", {{"names", names}}}});
        // 【逐行批注】 执行当前声明或表达式：} else {。
        } else {
            // 【逐行批注】 计算右侧结果并写入 auto returned，更新当前运行状态。
            const auto returned = Json::parse(options.messages.back().content).at("toolResults");
            // 【逐行批注】 判断条件 (calls == 2) {，成立时执行对应分支。
            if (calls == 2) {
                // 【逐行批注】 计算右侧结果并写入 auto descriptions，更新当前运行状态。
                const auto descriptions = returned.at(0).at("result").at("tools");
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(descriptions.size() == names.size());。
                assert(descriptions.size() == names.size());
                // 【逐行批注】 按循环规则重复处理：for (const auto& tool : descriptions) {。
                for (const auto& tool : descriptions) {
                    // 【逐行批注】 计算右侧结果并写入 auto description，更新当前运行状态。
                    const auto description = tool.at("description").get<std::string>();
                    // 【逐行批注】 判断条件 (tool.at("name") == "set_deep_content")，成立时执行对应分支。
                    if (tool.at("name") == "set_deep_content")
                        // 【逐行批注】 调用 assert 完成本行所需操作：assert(description.find("sectionId") != std::string::npos && descript…。
                        assert(description.find("sectionId") != std::string::npos && description.find("sourceSectionIds") != std::string::npos && description.find("coefficients") != std::string::npos);
                    // 【逐行批注】 判断条件 (tool.at("name") == "search_lesson_images")，成立时执行对应分支。
                    if (tool.at("name") == "search_lesson_images")
                        // 【逐行批注】 调用 assert 完成本行所需操作：assert(description.find("mediaId") != std::string::npos && descriptio…。
                        assert(description.find("mediaId") != std::string::npos && description.find("refresh:true") != std::string::npos);
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "create"}, {"tool", "create_lesson"}, {"args", {{"courseId…。
                actions.push_back({{"id", "create"}, {"tool", "create_lesson"}, {"args", {{"courseId", "protocol"}, {"topicId", "a-role"},
                    // 【逐行批注】 执行当前声明或表达式：{"title", "观察系数"}, {"purpose", "依据函数式解释图像变化"}, {"intent", "advance"}}}});。
                    {"title", "观察系数"}, {"purpose", "依据函数式解释图像变化"}, {"intent", "advance"}}}});
            // 【逐行批注】 执行当前声明或表达式：} else if (calls == 3) {。
            } else if (calls == 3) {
                // 【逐行批注】 计算右侧结果并写入 lessonId，更新当前运行状态。
                lessonId = returned.at(0).at("result").at("lessonId");
                // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "explain"}, {"tool", "append_section"}, {"args", {{"lesson…。
                actions.push_back({{"id", "explain"}, {"tool", "append_section"}, {"args", {{"lessonId", lessonId},
                    // 【逐行批注】 执行当前声明或表达式：{"section", {{"kind", "explanation"}, {"title", "观察系数"}, {"body", "保持常数项与一次项不变，比较二次项…。
                    {"section", {{"kind", "explanation"}, {"title", "观察系数"}, {"body", "保持常数项与一次项不变，比较二次项系数。"}, {"helpType", "explanation"}}}}}});
                // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "question"}, {"tool", "append_section"}, {"args", {{"lesso…。
                actions.push_back({{"id", "question"}, {"tool", "append_section"}, {"args", {{"lessonId", lessonId},
                    // 【逐行批注】 执行当前声明或表达式：{"section", {{"kind", "question"}, {"title", "试着解释"}, {"questionKind", "practice"}, …。
                    {"section", {{"kind", "question"}, {"title", "试着解释"}, {"questionKind", "practice"}, {"question", {{"question", "改变二次项系数后观察到了什么？"},
                        // 【逐行批注】 执行当前声明或表达式：{"type", "open"}, {"expectedAnswer", "备课私有答案"}, {"rubric", "备课私有标准"}}}}}}}});。
                        {"type", "open"}, {"expectedAnswer", "备课私有答案"}, {"rubric", "备课私有标准"}}}}}}}});
            // 【逐行批注】 执行当前声明或表达式：} else if (calls == 4) {。
            } else if (calls == 4) {
                // 【逐行批注】 计算右侧结果并写入 auto sectionId，更新当前运行状态。
                const auto sectionId = returned.at(0).at("result").at("sectionId");
                // 【逐行批注】 计算右侧结果并写入 teaching，更新当前运行状态。
                auto teaching = content(); teaching["lecture"][0]["sectionId"] = sectionId;
                // 【逐行批注】 计算右侧结果并写入 teaching["slides"][0]["sourceSectionIds"]，更新当前运行状态。
                teaching["slides"][0]["sourceSectionIds"] = {sectionId};
                // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "content"}, {"tool", "set_deep_content"}, {"args", {{"less…。
                actions.push_back({{"id", "content"}, {"tool", "set_deep_content"}, {"args", {{"lessonId", lessonId}, {"content", teaching}}}});
            // 【逐行批注】 执行当前声明或表达式：} else {。
            } else {
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(calls == 5 && returned.at(0).at("result").at("lectureCount") =…。
                assert(calls == 5 && returned.at(0).at("result").at("lectureCount") == 1);
                // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "finish"}, {"tool", "finish_lesson"}, {"args", {{"lessonId…。
                actions.push_back({{"id", "finish"}, {"tool", "finish_lesson"}, {"args", {{"lessonId", lessonId}}}});
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 Json decision，更新当前运行状态。
        const Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", ""}, {"actions", actions}, {"state", calls == 5 ? "completed" : "continue"}};
        // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
        gangyi::AIResult result; result.content = decision.dump(); result.model = "fixture-complete-deep-protocol"; result.finishReason = "stop"; return result;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(agent);。
    gangyi::registerAgentLessonTools(agent);
    // 【逐行批注】 执行当前声明或表达式：agent.process(db, task.at("id"));。
    agent.process(db, task.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto completed，更新当前运行状态。
    const auto completed = gangyi::agentView(db, access, task.at("id"));
    // 【逐行批注】 判断条件 (completed.at("status") != "ready") std::cerr << completed.dump(…，成立时执行对应分支。
    if (completed.at("status") != "ready") std::cerr << completed.dump(2) << '\n';
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(completed.at("status") == "ready" && calls == 5);。
    assert(completed.at("status") == "ready" && calls == 5);
    // 【逐行批注】 计算右侧结果并写入 auto view，更新当前运行状态。
    const auto view = gangyi::deepCourseView(db, access, scope);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(view.at("lesson").at("id") == lessonId && view.at("lesson").at…。
    assert(view.at("lesson").at("id") == lessonId && view.at("lesson").at("status") == "ready");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(view.at("lesson").at("sections").size() == 2 && view.at("conte…。
    assert(view.at("lesson").at("sections").size() == 2 && view.at("content").at("slides").size() == 1);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(view.at("content").at("lecture").at(0).at("body") == "保持常数项与一次…。
    assert(view.at("content").at("lecture").at(0).at("body") == "保持常数项与一次项不变，比较二次项系数。");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(view.dump().find("备课私有") == std::string::npos && db.listCourse…。
    assert(view.dump().find("备课私有") == std::string::npos && db.listCourses().size() == 1);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 runDeepDraftRecoveryChecks，执行该函数声明的职责。
void runDeepDraftRecoveryChecks() {
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(":memory:"); db.migrate();。
    gangyi::Database db; db.open(":memory:"); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：gangyi::AgentAccess access{"draft-fixture", {}};。
    gangyi::AgentAccess access{"draft-fixture", {}};
    // 【逐行批注】 计算右侧结果并写入 Json outline，更新当前运行状态。
    const Json outline = {{"courseStructure", {{{"stage", "事件顺序"}, {"topics", {{{"id", "timeline"}, {"title", "四个事件"}}}}}}}};
    // 【逐行批注】 计算右侧结果并写入 int planningCalls，更新当前运行状态。
    int planningCalls = 0;
    // 【逐行批注】 定义函数 planner，执行该函数声明的职责。
    gangyi::LearningAgent planner("", [&](const gangyi::ChatOptions&, const auto&) {
        // 【逐行批注】 调用 assert 完成本行所需操作：++planningCalls; assert(planningCalls == 1);。
        ++planningCalls; assert(planningCalls == 1);
        // 【逐行批注】 计算右侧结果并写入 Json decision，更新当前运行状态。
        const Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", ""}, {"state", "continue"},
            // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "course"}, {"tool", "create_course"}, {"args", {{"goal", "事件先后"…。
            {"actions", {{{"id", "course"}, {"tool", "create_course"}, {"args", {{"goal", "事件先后"}, {"title", "事件先后"}, {"mode", "deep"}, {"payload", outline}}}},
                // 【逐行批注】 调用 object 完成本行所需操作：{{"id", "premature-lesson"}, {"tool", "create_lesson"}, {"args", Json…。
                {{"id", "premature-lesson"}, {"tool", "create_lesson"}, {"args", Json::object()}}}}};
        // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
        gangyi::AIResult result; result.content = decision.dump(); result.model = "fixture-outline-only"; result.finishReason = "stop"; return result;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(planner);。
    gangyi::registerAgentLessonTools(planner);
    // 【逐行批注】 计算右侧结果并写入 auto plan，更新当前运行状态。
    const auto plan = gangyi::agentSubmit(db, access, {{"type", "plan_course"}, {"mode", "deep"}, {"goal", "事件先后"}, {"requestId", "outline-first"}});
    // 【逐行批注】 执行当前声明或表达式：planner.process(db, plan.at("id"));。
    planner.process(db, plan.at("id"));
    // 【逐行批注】 按循环规则重复处理：for (const auto& course : db.listCourses()) access.courseIds.push_back(cou…。
    for (const auto& course : db.listCourses()) access.courseIds.push_back(course.id);
    // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
    const auto courseId = db.listCourses().at(0).id;
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, plan.at("id")).at("status") == "…。
    assert(gangyi::agentView(db, access, plan.at("id")).at("status") == "ready" && planningCalls == 1);
    // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.listClassroomActivities(courseId)) assert(row.ki…。
    for (const auto& row : db.listClassroomActivities(courseId)) assert(row.kind != "agent-lesson");
    // 【逐行批注】 计算右侧结果并写入 Json planningTask，更新当前运行状态。
    const Json planningTask = {{"event", {{"type", "plan_course"}, {"mode", "deep"}}}};
    // 【逐行批注】 按循环规则重复处理：for (const auto* name : {"create_lesson", "append_section", "finish_lesson…。
    for (const auto* name : {"create_lesson", "append_section", "finish_lesson", "complete_lesson", "select_teaching_focus", "set_deep_content", "search_lesson_images",
        // 【逐行批注】 执行当前声明或表达式："revise_outline", "adjust_goal", "update_course_preview", "schedule_review", "read_l…。
        "revise_outline", "adjust_goal", "update_course_preview", "schedule_review", "read_legacy_lesson", "select_resources", "update_topic_mastery", "update_profiles"})
        // 【逐行批注】 调用 assert、agentToolAuthorized 完成本行所需操作：assert(!gangyi::agentToolAuthorized(name, planningTask));。
        assert(!gangyi::agentToolAuthorized(name, planningTask));
    // 【逐行批注】 调用 assert、agentToolAuthorized 完成本行所需操作：assert(gangyi::agentToolAuthorized("create_lesson", {{"event", {{"typ…。
    assert(gangyi::agentToolAuthorized("create_lesson", {{"event", {{"type", "plan_course"}, {"mode", "lite"}}}}));

    // 模拟旧执行路径已保留课程与草稿后失败：恢复规划不调用模型，也不把草稿标为ready。
    // 【逐行批注】 计算右侧结果并写入 savedPlan，更新当前运行状态。
    auto savedPlan = *db.getClassroomActivity(plan.at("id")); auto failed = Json::parse(savedPlan.payload);
    // 【逐行批注】 计算右侧结果并写入 failed["status"]，更新当前运行状态。
    failed["status"] = "failed"; failed["error"] = "测试预算不足"; savedPlan.payload = failed.dump(); assert(db.upsert(savedPlan));
    // 【逐行批注】 计算右侧结果并写入 Json draft，更新当前运行状态。
    Json draft = {{"id", "saved-draft"}, {"scopeId", access.scopeId}, {"courseId", courseId}, {"topicId", "timeline"}, {"phaseIndex", 1}, {"topicIndex", 1},
        // 【逐行批注】 执行当前声明或表达式：{"title", "已生成的事件讲义"}, {"purpose", "理解先后"}, {"intent", "advance"}, {"status", "draft…。
        {"title", "已生成的事件讲义"}, {"purpose", "理解先后"}, {"intent", "advance"}, {"status", "draft"}, {"version", 1}, {"entered", false},
        // 【逐行批注】 执行当前声明或表达式：{"sourceLearningVersion", db.learningRevision()}, {"taskId", plan.at("id")}, {"updat…。
        {"sourceLearningVersion", db.learningRevision()}, {"taskId", plan.at("id")}, {"updatedAt", "2026-10-28"},
        // 【逐行批注】 执行当前声明或表达式：{"sections", {{{"id", "saved-explanation"}, {"kind", "explanation"}, {"title", "已生成讲…。
        {"sections", {{{"id", "saved-explanation"}, {"kind", "explanation"}, {"title", "已生成讲义"}, {"body", "先比较年份，再比较月份。"}, {"helpType", "explanation"}, {"version", 1}, {"displayed", false}}}}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(gangyi::ClassroomActivity{"saved-draft", courseId, "…。
    assert(db.upsert(gangyi::ClassroomActivity{"saved-draft", courseId, "agent-lesson", draft.dump(), "2026-10-28", 1, 1}));
    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", pl…。
    gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", plan.at("id")}});
    // 【逐行批注】 执行当前声明或表达式：planner.process(db, plan.at("id"));。
    planner.process(db, plan.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(planningCalls == 1 && gangyi::agentView(db, access, plan.at("i…。
    assert(planningCalls == 1 && gangyi::agentView(db, access, plan.at("id")).at("status") == "ready");
    // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(db.getClassroomActivity("saved-draft")->payload).a…。
    assert(Json::parse(db.getClassroomActivity("saved-draft")->payload).at("status") == "draft");
    // 【逐行批注】 调用 rejects、deepCourseStart 完成本行所需操作：rejects([&] { gangyi::deepCourseStart(db, {"another-scope", {courseId…。
    rejects([&] { gangyi::deepCourseStart(db, {"another-scope", {courseId}}, {{"lessonId", "saved-draft"}}); });
    // 【逐行批注】 调用 rejects、deepCourseStart 完成本行所需操作：rejects([&] { gangyi::deepCourseStart(db, access, {{"lessonId", "save…。
    rejects([&] { gangyi::deepCourseStart(db, access, {{"lessonId", "saved-draft"}, {"topicId", "another-topic"}}); });
    // 【逐行批注】 计算右侧结果并写入 staleRow，更新当前运行状态。
    auto staleRow = *db.getClassroomActivity("saved-draft"); auto stale = draft; stale["sourceLearningVersion"] = db.learningRevision() - 1;
    // 【逐行批注】 计算右侧结果并写入 staleRow.payload，更新当前运行状态。
    staleRow.payload = stale.dump(); assert(db.upsert(staleRow));
    // 【逐行批注】 调用 rejects、deepCourseStart 完成本行所需操作：rejects([&] { gangyi::deepCourseStart(db, access, {{"lessonId", "save…。
    rejects([&] { gangyi::deepCourseStart(db, access, {{"lessonId", "saved-draft"}}); });
    // 【逐行批注】 计算右侧结果并写入 staleRow.payload，更新当前运行状态。
    staleRow.payload = draft.dump(); assert(db.upsert(staleRow));

    // 【逐行批注】 计算右侧结果并写入 auto resumed，更新当前运行状态。
    const auto resumed = gangyi::deepCourseStart(db, access, {{"lessonId", "saved-draft"}}).at("task");
    // 【逐行批注】 计算右侧结果并写入 auto same，更新当前运行状态。
    const auto same = gangyi::deepCourseStart(db, access, {{"courseId", courseId}, {"topicId", "timeline"}, {"requestId", "another-window"}}).at("task");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(resumed.at("id") == same.at("id") && resumed.at("id") != plan.…。
    assert(resumed.at("id") == same.at("id") && resumed.at("id") != plan.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto resumedRaw，更新当前运行状态。
    const auto resumedRaw = Json::parse(db.getClassroomActivity(resumed.at("id"))->payload);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(resumedRaw.at("event").at("resumeDraft") == true && resumedRaw…。
    assert(resumedRaw.at("event").at("resumeDraft") == true && resumedRaw.at("event").at("lessonId") == "saved-draft");
    // 【逐行批注】 调用 assert、agentToolAuthorized 完成本行所需操作：assert(!gangyi::agentToolAuthorized("create_lesson", resumedRaw));。
    assert(!gangyi::agentToolAuthorized("create_lesson", resumedRaw));
    // 【逐行批注】 按循环规则重复处理：for (const auto* name : {"complete_lesson", "select_teaching_focus", "upda…。
    for (const auto* name : {"complete_lesson", "select_teaching_focus", "update_profiles", "adjust_goal", "revise_outline"})
        // 【逐行批注】 调用 assert、agentToolAuthorized 完成本行所需操作：assert(!gangyi::agentToolAuthorized(name, resumedRaw));。
        assert(!gangyi::agentToolAuthorized(name, resumedRaw));
    // 【逐行批注】 计算右侧结果并写入 other，更新当前运行状态。
    auto other = draft; other["id"] = "other-saved-lesson"; other["status"] = "ready"; other["intent"] = "general"; other["topicId"] = "old-topic";
    // 【逐行批注】 计算右侧结果并写入 other["deepContent"]，更新当前运行状态。
    other["deepContent"] = gangyi::normalizeDeepContent(content());
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(gangyi::ClassroomActivity{"other-saved-lesson", cour…。
    assert(db.upsert(gangyi::ClassroomActivity{"other-saved-lesson", courseId, "agent-lesson", other.dump(), "2026-10-28", 1, 2}));
    // 【逐行批注】 计算右侧结果并写入 auto otherPayload，更新当前运行状态。
    const auto otherPayload = db.getClassroomActivity("other-saved-lesson")->payload;
    // 【逐行批注】 计算右侧结果并写入 int preparationCalls，更新当前运行状态。
    int preparationCalls = 0;
    // 【逐行批注】 定义函数 teacher，执行该函数声明的职责。
    gangyi::LearningAgent teacher("", [&](const gangyi::ChatOptions&, const auto&) {
        // 【逐行批注】 调用 assert 完成本行所需操作：++preparationCalls; assert(preparationCalls <= 3);。
        ++preparationCalls; assert(preparationCalls <= 3);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity("other-saved-lesson")->payload == othe…。
        assert(db.getClassroomActivity("other-saved-lesson")->payload == otherPayload);
        // 【逐行批注】 计算右侧结果并写入 material，更新当前运行状态。
        auto material = content(); material["lecture"][0]["sectionId"] = "saved-explanation"; material["slides"][0]["sourceSectionIds"] = {"saved-explanation"};
        // 【逐行批注】 计算右侧结果并写入 Json actions，更新当前运行状态。
        Json actions = {
            // 【逐行批注】 执行当前声明或表达式：{{"id", "content"}, {"tool", "set_deep_content"}, {"args", {{"lessonId", "saved-draf…。
            {{"id", "content"}, {"tool", "set_deep_content"}, {"args", {{"lessonId", "saved-draft"}, {"content", material}}}},
            // 【逐行批注】 执行当前声明或表达式：{{"id", "finish"}, {"tool", "finish_lesson"}, {"args", {{"lessonId", "saved-draft"}}…。
            {{"id", "finish"}, {"tool", "finish_lesson"}, {"args", {{"lessonId", "saved-draft"}}}}};
        // 【逐行批注】 判断条件 (preparationCalls == 1) actions = {{{"id", "wrong-append"}, {"to…，成立时执行对应分支。
        if (preparationCalls == 1) actions = {{{"id", "wrong-append"}, {"tool", "append_section"}, {"args", {{"lessonId", "other-saved-lesson"},
            // 【逐行批注】 执行当前声明或表达式：{"section", {{"kind", "explanation"}, {"title", "不能写到旧课"}, {"body", "错误位置"}, {"helpT…。
            {"section", {{"kind", "explanation"}, {"title", "不能写到旧课"}, {"body", "错误位置"}, {"helpType", "explanation"}}}}}}};
        // 【逐行批注】 判断条件 (preparationCalls == 2) actions = {{{"id", "wrong-finish"}, {"to…，成立时执行对应分支。
        if (preparationCalls == 2) actions = {{{"id", "wrong-finish"}, {"tool", "finish_lesson"}, {"args", {{"lessonId", "other-saved-lesson"}}}}};
        // 【逐行批注】 计算右侧结果并写入 Json decision，更新当前运行状态。
        const Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", ""}, {"state", "completed"}, {"actions", actions}};
        // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
        gangyi::AIResult result; result.content = decision.dump(); result.model = "fixture-resume-draft"; result.finishReason = "stop"; return result;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(teacher); teacher.process(db, resume…。
    gangyi::registerAgentLessonTools(teacher); teacher.process(db, resumed.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto completed，更新当前运行状态。
    const auto completed = gangyi::agentView(db, access, resumed.at("id"));
    // 【逐行批注】 判断条件 (completed.at("status") != "ready") std::cerr << completed.dump(…，成立时执行对应分支。
    if (completed.at("status") != "ready") std::cerr << completed.dump(2) << '\n';
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(completed.at("status") == "ready" && preparationCalls == 3 && …。
    assert(completed.at("status") == "ready" && preparationCalls == 3 && db.listCourses().size() == 1);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(completed.at("lesson").at("id") == "saved-draft" && db.getClas…。
    assert(completed.at("lesson").at("id") == "saved-draft" && db.getClassroomActivity("other-saved-lesson")->payload == otherPayload);
    // 【逐行批注】 计算右侧结果并写入 auto restored，更新当前运行状态。
    const auto restored = Json::parse(db.getClassroomActivity("saved-draft")->payload);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(restored.at("status") == "ready" && restored.at("sections") ==…。
    assert(restored.at("status") == "ready" && restored.at("sections") == draft.at("sections"));
    // 【逐行批注】 计算右侧结果并写入 int lessonCount，更新当前运行状态。
    int lessonCount = 0; for (const auto& row : db.listClassroomActivities(courseId)) if (row.kind == "agent-lesson") ++lessonCount;
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(lessonCount == 2);。
    assert(lessonCount == 2);

    // 即使模型把旧课完成操作放在新建大纲之前，执行层也不能改旧课状态与进度。
    // 【逐行批注】 调用 agentEnterLesson 完成本行所需操作：gangyi::agentEnterLesson(db, access, {{"lessonId", "saved-draft"}});。
    gangyi::agentEnterLesson(db, access, {{"lessonId", "saved-draft"}});
    // 【逐行批注】 计算右侧结果并写入 auto existingLesson，更新当前运行状态。
    const auto existingLesson = db.getClassroomActivity("saved-draft")->payload;
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!db.findLearningCardProgress(courseId, 1, 1));。
    assert(!db.findLearningCardProgress(courseId, 1, 1));
    // 【逐行批注】 计算右侧结果并写入 auto previousGoal，更新当前运行状态。
    const auto previousGoal = db.getCourse(courseId)->goal, previousAbilities = db.profileMeta("ability-profile");
    // 【逐行批注】 按循环规则重复处理：for (const std::string forbidden : {"complete_lesson", "adjust_goal", "upd…。
    for (const std::string forbidden : {"complete_lesson", "adjust_goal", "update_profiles"}) {
      // 【逐行批注】 计算右侧结果并写入 int guardedCalls，更新当前运行状态。
      int guardedCalls = 0;
      // 【逐行批注】 定义函数 guarded，执行该函数声明的职责。
      gangyi::LearningAgent guarded("", [&](const gangyi::ChatOptions&, const auto&) {
        // 【逐行批注】 调用 assert 完成本行所需操作：++guardedCalls; assert(guardedCalls <= 2);。
        ++guardedCalls; assert(guardedCalls <= 2);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity("saved-draft")->payload == existingLes…。
        assert(db.getClassroomActivity("saved-draft")->payload == existingLesson && !db.findLearningCardProgress(courseId, 1, 1));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse(courseId)->goal == previousGoal && db.profileMeta…。
        assert(db.getCourse(courseId)->goal == previousGoal && db.profileMeta("ability-profile") == previousAbilities);
        // 【逐行批注】 计算右侧结果并写入 Json actions，更新当前运行状态。
        Json actions = Json::array();
        // 【逐行批注】 计算右侧结果并写入 Json forbiddenArgs，更新当前运行状态。
        Json forbiddenArgs = {{"lessonId", "saved-draft"}, {"reason", "无权借规划修改旧进度"}};
        // 【逐行批注】 判断条件 (forbidden == "adjust_goal") forbiddenArgs = {{"courseId", cours…，成立时执行对应分支。
        if (forbidden == "adjust_goal") forbiddenArgs = {{"courseId", courseId}, {"goal", "无权替换旧目标"}, {"reason", "新建课程不能修改旧目标"}};
        // 【逐行批注】 判断条件 (forbidden == "update_profiles") forbiddenArgs = {{"subjects", {…，成立时执行对应分支。
        if (forbidden == "update_profiles") forbiddenArgs = {{"subjects", {{{"subject", "数学"}, {"score", nullptr}, {"sufficient", false}, {"rationale", "不能清除旧画像"}, {"recommendation", "等待"}, {"uncertainty", "未测试"}, {"evidenceIds", Json::array()}}}}};
        // 【逐行批注】 判断条件 (guardedCalls == 1) actions.push_back({{"id", "forbidden-old-wri…，成立时执行对应分支。
        if (guardedCalls == 1) actions.push_back({{"id", "forbidden-old-write"}, {"tool", forbidden}, {"args", forbiddenArgs}});
        // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", "new-course"}, {"tool", "create_course"}, {"args", {{"goal…。
        actions.push_back({{"id", "new-course"}, {"tool", "create_course"}, {"args", {{"goal", "另一个新目标"}, {"title", "另一个新目标"}, {"mode", "deep"}, {"payload", outline}}}});
        // 【逐行批注】 计算右侧结果并写入 Json decision，更新当前运行状态。
        const Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", ""}, {"state", "completed"}, {"actions", actions}};
        // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
        gangyi::AIResult result; result.content = decision.dump(); result.model = "fixture-planning-write-boundary"; result.finishReason = "stop"; return result;
      // 【逐行批注】 执行当前声明或表达式：});。
      });
      // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(guarded);。
      gangyi::registerAgentLessonTools(guarded);
      // 【逐行批注】 计算右侧结果并写入 auto guardedTask，更新当前运行状态。
      const auto guardedTask = gangyi::agentSubmit(db, access, {{"type", "plan_course"}, {"mode", "deep"}, {"goal", "另一个新目标"}, {"requestId", "planning-write-boundary-" + forbidden}});
      // 【逐行批注】 执行当前声明或表达式：guarded.process(db, guardedTask.at("id"));。
      guarded.process(db, guardedTask.at("id"));
      // 【逐行批注】 按循环规则重复处理：for (const auto& course : db.listCourses()) if (std::find(access.courseIds…。
      for (const auto& course : db.listCourses()) if (std::find(access.courseIds.begin(), access.courseIds.end(), course.id) == access.courseIds.end()) access.courseIds.push_back(course.id);
      // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, guardedTask.at("id")).at("status…。
      assert(gangyi::agentView(db, access, guardedTask.at("id")).at("status") == "ready" && guardedCalls == 2);
      // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity("saved-draft")->payload == existingLes…。
      assert(db.getClassroomActivity("saved-draft")->payload == existingLesson && !db.findLearningCardProgress(courseId, 1, 1));
      // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse(courseId)->goal == previousGoal && db.profileMeta…。
      assert(db.getCourse(courseId)->goal == previousGoal && db.profileMeta("ability-profile") == previousAbilities);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.listCourses().size() == 4);。
    assert(db.listCourses().size() == 4);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 runDeepCourseChecks，执行该函数声明的职责。
void runDeepCourseChecks() {
    // 【逐行批注】 计算右侧结果并写入 auto fullscreenPage，更新当前运行状态。
    const auto fullscreenPage = gangyi::renderDeepCoursePage();
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(fullscreenPage.find("gy-classroom-fullscreen") != std::string:…。
    assert(fullscreenPage.find("gy-classroom-fullscreen") != std::string::npos);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(fullscreenPage.find("/deep-course.js") != std::string::npos &&。
    assert(fullscreenPage.find("/deep-course.js") != std::string::npos &&
           // 【逐行批注】 执行当前声明或表达式：fullscreenPage.find("/agent-trace.js") != std::string::npos);。
           fullscreenPage.find("/agent-trace.js") != std::string::npos);
    // 【逐行批注】 调用 runDeepPreparationProtocolChecks 完成本行所需操作：runDeepPreparationProtocolChecks();。
    runDeepPreparationProtocolChecks();
    // 【逐行批注】 调用 runDeepDraftRecoveryChecks 完成本行所需操作：runDeepDraftRecoveryChecks();。
    runDeepDraftRecoveryChecks();
    // 【逐行批注】 计算右侧结果并写入 auto normalized，更新当前运行状态。
    const auto normalized = gangyi::normalizeDeepContent(content());
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(normalized.at("media").at(0).at("coefficients").size() == 3);。
    assert(normalized.at("media").at(0).at("coefficients").size() == 3);
    // 【逐行批注】 计算右侧结果并写入 invalid，更新当前运行状态。
    auto invalid = content(); invalid["media"][0]["xMin"] = 10;
    // 【逐行批注】 调用 rejects、normalizeDeepContent 完成本行所需操作：rejects([&] { gangyi::normalizeDeepContent(invalid); });。
    rejects([&] { gangyi::normalizeDeepContent(invalid); });
    // 【逐行批注】 计算右侧结果并写入 invalid，更新当前运行状态。
    invalid = content(); invalid["media"][0]["parameter"]["coefficientIndex"] = 4;
    // 【逐行批注】 调用 rejects、normalizeDeepContent 完成本行所需操作：rejects([&] { gangyi::normalizeDeepContent(invalid); });。
    rejects([&] { gangyi::normalizeDeepContent(invalid); });
    // 【逐行批注】 计算右侧结果并写入 invalid，更新当前运行状态。
    invalid = content(); invalid["lecture"][0]["mediaIds"] = {"missing"};
    // 【逐行批注】 调用 rejects、normalizeDeepContent 完成本行所需操作：rejects([&] { gangyi::normalizeDeepContent(invalid); });。
    rejects([&] { gangyi::normalizeDeepContent(invalid); });
    // 【逐行批注】 计算右侧结果并写入 invalid，更新当前运行状态。
    invalid = content(); invalid["media"][0]["type"] = "javascript";
    // 【逐行批注】 调用 rejects、normalizeDeepContent 完成本行所需操作：rejects([&] { gangyi::normalizeDeepContent(invalid); });。
    rejects([&] { gangyi::normalizeDeepContent(invalid); });
    // 【逐行批注】 计算右侧结果并写入 extra，更新当前运行状态。
    auto extra = content(); extra["lecture"][0]["script"] = "throw new Error('not allowed')";
    // 【逐行批注】 调用 assert、normalizeDeepContent 完成本行所需操作：assert(!gangyi::normalizeDeepContent(extra)["lecture"][0].contains("s…。
    assert(!gangyi::normalizeDeepContent(extra)["lecture"][0].contains("script"));
    // 【逐行批注】 计算右侧结果并写入 imageInput，更新当前运行状态。
    auto imageInput = content(); imageInput["media"] = {{{"id", "curve"}, {"type", "image"}, {"title", "图片"}, {"url", "http://127.0.0.1/private"}}};
    // 【逐行批注】 计算右侧结果并写入 auto sanitized，更新当前运行状态。
    const auto sanitized = gangyi::normalizeDeepContent(imageInput)["media"][0];
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(sanitized.at("status") == "pending" && !sanitized.contains("ur…。
    assert(sanitized.at("status") == "pending" && !sanitized.contains("url"));
    // 【逐行批注】 计算右侧结果并写入 auto images，更新当前运行状态。
    const auto images = gangyi::deepImageCandidates("<meta content='/pictures/lesson.png' property='og:image'><img alt='标志' src='/logo.png'><img alt='不可访问' src='http://127.0.0.1/a.png'>", "https://example.org/course/lesson");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(images.size() == 1 && images.at(0).at("url") == "https://examp…。
    assert(images.size() == 1 && images.at(0).at("url") == "https://example.org/pictures/lesson.png");
    // 【逐行批注】 计算右侧结果并写入 std::string imagePage，更新当前运行状态。
    const std::string imagePage = "<meta property='og:image' content='/hero.webp'><img src='/satellite.webp' alt='Satellite dishes.'>"
        // 【逐行批注】 执行当前声明或表达式："<figure><img src='/figure-12.webp' alt='Graph of a parabola opening upward.'><figca…。
        "<figure><img src='/figure-12.webp' alt='Graph of a parabola opening upward.'><figcaption>Quadratic function graph.</figcaption></figure>"
        // 【逐行批注】 执行当前声明或表达式："<img src='/function-cover.webp' alt='Cover of the function textbook.'>";。
        "<img src='/function-cover.webp' alt='Cover of the function textbook.'>";
    // 【逐行批注】 计算右侧结果并写入 auto ranked，更新当前运行状态。
    const auto ranked = gangyi::deepImageCandidates(imagePage, "https://example.org/quadratic-functions", "quadratic function parabola graph opens upward downward", "二次函数参考图");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!ranked.empty() && ranked.at(0).at("url") == "https://example.…。
    assert(!ranked.empty() && ranked.at(0).at("url") == "https://example.org/figure-12.webp");
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : ranked) assert(item.at("url") != "https://example.…。
    for (const auto& item : ranked) assert(item.at("url") != "https://example.org/satellite.webp" && item.at("url") != "https://example.org/hero.webp");
    // 【逐行批注】 调用 assert、deepImageCandidates 完成本行所需操作：assert(gangyi::deepImageCandidates("<img src='/photo.webp' alt='Satel…。
    assert(gangyi::deepImageCandidates("<img src='/photo.webp' alt='Satellite dishes.'>", "https://example.org/quadratic-functions", "parabola graph", "函数图").empty());
    // 【逐行批注】 调用 assert、deepImageCandidates 完成本行所需操作：assert(gangyi::deepImageCandidates(imagePage, "https://example.org/ma…。
    assert(gangyi::deepImageCandidates(imagePage, "https://example.org/math", "AI", "AI").empty());
    // 【逐行批注】 调用 assert、deepImageCandidates 完成本行所需操作：assert(gangyi::deepImageCandidates(imagePage, "https://example.org/ma…。
    assert(gangyi::deepImageCandidates(imagePage, "https://example.org/math", "Go", "图片 参考").empty());
    // 【逐行批注】 调用 assert、deepImageCandidates 完成本行所需操作：assert(gangyi::deepImageCandidates(imagePage, "https://example.org/ma…。
    assert(gangyi::deepImageCandidates(imagePage, "https://example.org/math", "图片", "参考").empty());
    // 【逐行批注】 计算右侧结果并写入 auto captionMatch，更新当前运行状态。
    const auto captionMatch = gangyi::deepImageCandidates("<div data-type='figure'><div><img src='/event.webp' alt='示意图'></div><span data-type='caption'>法国大革命攻占巴士底狱的历史场景。</span></div>",
        // 【逐行批注】 执行当前声明或表达式："https://example.org/history", "法国大革命 攻占巴士底狱", "历史图片");。
        "https://example.org/history", "法国大革命 攻占巴士底狱", "历史图片");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(captionMatch.size() == 1 && !captionMatch.at(0).at("sourceCapt…。
    assert(captionMatch.size() == 1 && !captionMatch.at(0).at("sourceCaption").get<std::string>().empty());
    // 【逐行批注】 计算右侧结果并写入 auto duplicateImage，更新当前运行状态。
    const auto duplicateImage = gangyi::deepImageCandidates("<meta property='og:image' content='/curve.webp'><figure><img src='/curve.webp' alt='Parabola graph'><figcaption>Graph of a function.</figcaption></figure>",
        // 【逐行批注】 执行当前声明或表达式："https://example.org/math", "parabola graph", "参考图");。
        "https://example.org/math", "parabola graph", "参考图");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(duplicateImage.size() == 1 && duplicateImage.at(0).at("alt") =…。
    assert(duplicateImage.size() == 1 && duplicateImage.at(0).at("alt") == "Parabola graph");
    // 【逐行批注】 执行当前声明或表达式：std::string laterFigure;。
    std::string laterFigure;
    // 【逐行批注】 按循环规则重复处理：for (int i = 0; i < 12; ++i) laterFigure += "<img src='/photo-" + std::to_…。
    for (int i = 0; i < 12; ++i) laterFigure += "<img src='/photo-" + std::to_string(i) + ".webp' alt='Unrelated photograph'>";
    // 【逐行批注】 计算右侧结果并写入 laterFigure +，更新当前运行状态。
    laterFigure += "<figure><img src='/late-graph.webp' alt='Parabola graph'></figure>";
    // 【逐行批注】 调用 assert、deepImageCandidates 完成本行所需操作：assert(gangyi::deepImageCandidates(laterFigure, "https://example.org/…。
    assert(gangyi::deepImageCandidates(laterFigure, "https://example.org/math", "parabola graph", "参考图").at(0).at("url") == "https://example.org/late-graph.webp");

    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(":memory:"); db.migrate();。
    gangyi::Database db; db.open(":memory:"); db.migrate();
    // 【逐行批注】 计算右侧结果并写入 auto outline，更新当前运行状态。
    const auto outline = Json{{"courseStructure", {{{"stage", "函数的变化"}, {"topics", {{{"id", "topic-1"}, {"title", "二次函数"}, {"legacyPhaseIndex", 1}, {"legacyTopicIndex", 1}}}}}}}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::Course{"old", {}, {}, "旧课程", "deep", "旧课程", …。
    assert(db.insert(gangyi::Course{"old", {}, {}, "旧课程", "deep", "旧课程", {}, "ai", "active", "2026-10-28", "2026-10-28"}));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::CourseSnapshot{"snapshot-old", "old", 1, out…。
    assert(db.insert(gangyi::CourseSnapshot{"snapshot-old", "old", 1, outline.dump(), "2026-10-28"}));
    // 【逐行批注】 调用 assert、deepCourseEnabled 完成本行所需操作：assert(!gangyi::deepCourseEnabled(db, "old"));。
    assert(!gangyi::deepCourseEnabled(db, "old"));
    // 【逐行批注】 计算右侧结果并写入 deepOutline，更新当前运行状态。
    auto deepOutline = outline; deepOutline["pedagogy"] = "deep-v1";
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::Course{"new", {}, {}, "二次函数", "deep", "二次函数"…。
    assert(db.insert(gangyi::Course{"new", {}, {}, "二次函数", "deep", "二次函数", {}, "ai", "active", "2026-10-28", "2026-10-28"}));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::CourseSnapshot{"snapshot-new", "new", 1, dee…。
    assert(db.insert(gangyi::CourseSnapshot{"snapshot-new", "new", 1, deepOutline.dump(), "2026-10-28"}));
    // 【逐行批注】 执行当前声明或表达式：gangyi::AgentAccess access{"fixture", {"old", "new"}};。
    gangyi::AgentAccess access{"fixture", {"old", "new"}};
    // 【逐行批注】 调用 assert、deepCourseEnabled 完成本行所需操作：assert(gangyi::deepCourseEnabled(db, "new"));。
    assert(gangyi::deepCourseEnabled(db, "new"));
    // 【逐行批注】 按循环规则重复处理：for (const std::string mode : {"lite", "deep"}) {。
    for (const std::string mode : {"lite", "deep"}) {
        // 【逐行批注】 定义函数 planner，执行该函数声明的职责。
        gangyi::LearningAgent planner("", [&](const gangyi::ChatOptions&, const auto&) {
            // 【逐行批注】 计算右侧结果并写入 Json decision，更新当前运行状态。
            Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", ""}, {"state", "completed"},
                // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "create"}, {"tool", "create_course"}, {"args", {{"goal", "课程模式校…。
                {"actions", {{{"id", "create"}, {"tool", "create_course"}, {"args", {{"goal", "课程模式校验"}, {"title", "课程模式校验"}, {"mode", mode == "deep" ? "lite" : "deep"}, {"payload", deepOutline}}}}}}};
            // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
            gangyi::AIResult result; result.content = decision.dump(); result.model = "fixture-mode-choice"; result.finishReason = "stop"; return result;
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 计算右侧结果并写入 auto planning，更新当前运行状态。
        const auto planning = gangyi::agentSubmit(db, access, {{"type", "plan_course"}, {"mode", mode}, {"goal", "课程模式校验"}, {"requestId", "mode-" + mode}});
        // 【逐行批注】 执行当前声明或表达式：planner.process(db, planning.at("id"));。
        planner.process(db, planning.at("id"));
        // 产品每个HTTP请求都会重新读取活跃课程；测试也要刷新新建课程的访问范围。
        // 【逐行批注】 计算右侧结果并写入 auto previousAccess，更新当前运行状态。
        const auto previousAccess = access;
        // 【逐行批注】 执行当前声明或表达式：access.courseIds.clear();。
        access.courseIds.clear();
        // 【逐行批注】 按循环规则重复处理：for (const auto& course : db.listCourses()) if (course.status == "active")…。
        for (const auto& course : db.listCourses()) if (course.status == "active") access.courseIds.push_back(course.id);
        // 【逐行批注】 调用 rejects、agentView 完成本行所需操作：rejects([&] { gangyi::agentView(db, previousAccess, planning.at("id")…。
        rejects([&] { gangyi::agentView(db, previousAccess, planning.at("id")); });
        // 【逐行批注】 计算右侧结果并写入 auto planned，更新当前运行状态。
        const auto planned = gangyi::agentView(db, access, planning.at("id"));
        // 【逐行批注】 判断条件 (planned.at("status") != "ready") std::cerr << planned.dump(2) <…，成立时执行对应分支。
        if (planned.at("status") != "ready") std::cerr << planned.dump(2) << '\n';
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(planned.at("status") == "ready");。
        assert(planned.at("status") == "ready");
        // 【逐行批注】 计算右侧结果并写入 auto createdId，更新当前运行状态。
        const auto createdId = planned.at("course").at("id").get<std::string>();
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse(createdId)->mode == mode);。
        assert(db.getCourse(createdId)->mode == mode);
        // 【逐行批注】 调用 assert、deepCourseEnabled 完成本行所需操作：assert(gangyi::deepCourseEnabled(db, createdId) == (mode == "deep"));。
        assert(gangyi::deepCourseEnabled(db, createdId) == (mode == "deep"));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto classroomSnapshot，更新当前运行状态。
    const auto classroomSnapshot = [&] {
        // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
        Json result = Json::object();
        // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.listClassroomActivities("new"))。
        for (const auto& row : db.listClassroomActivities("new"))
            // 【逐行批注】 计算右侧结果并写入 result[row.id]，更新当前运行状态。
            result[row.id] = {{"courseId", row.courseId}, {"kind", row.kind}, {"payload", row.payload},
                // 【逐行批注】 执行当前声明或表达式：{"updatedAt", row.updatedAt}, {"phaseIndex", row.phaseIndex}, {"topicIndex", row.top…。
                {"updatedAt", row.updatedAt}, {"phaseIndex", row.phaseIndex}, {"topicIndex", row.topicIndex}};
        // 【逐行批注】 结束当前函数并返回结果：return result;。
        return result;
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto revision，更新当前运行状态。
    const auto revision = db.learningRevision();
    // 【逐行批注】 计算右侧结果并写入 auto beforeRead，更新当前运行状态。
    const auto beforeRead = classroomSnapshot();
    // 【逐行批注】 调用 assert、deepCourseView 完成本行所需操作：assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}}).at("…。
    assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}}).at("status") == "unprepared");
    // 【逐行批注】 调用 assert、deepCourseView 完成本行所需操作：assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"pha…。
    assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"phaseIndex", "1"}, {"topicIndex", "1"}}).at("status") == "unprepared");
    // 【逐行批注】 调用 rejects、deepCourseView 完成本行所需操作：rejects([&] { gangyi::deepCourseView(db, access, {{"courseId", "new"}…。
    rejects([&] { gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"phaseIndex", "1.2"}}); });
    // 【逐行批注】 调用 rejects、deepCourseStart 完成本行所需操作：rejects([&] { gangyi::deepCourseStart(db, access, {{"courseId", "new"…。
    rejects([&] { gangyi::deepCourseStart(db, access, {{"courseId", "new"}, {"phaseIndex", -1}}); });
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.learningRevision() == revision);。
    assert(db.learningRevision() == revision);
    // 【逐行批注】 调用 assert、classroomSnapshot 完成本行所需操作：assert(classroomSnapshot() == beforeRead);。
    assert(classroomSnapshot() == beforeRead);
    // 【逐行批注】 调用 rejects、deepCourseView 完成本行所需操作：rejects([&] { gangyi::deepCourseView(db, {"other", {}}, {{"courseId",…。
    rejects([&] { gangyi::deepCourseView(db, {"other", {}}, {{"courseId", "new"}}); });
    // 【逐行批注】 计算右侧结果并写入 auto preparing，更新当前运行状态。
    const auto preparing = gangyi::deepCourseStart(db, access, {{"courseId", "new"}, {"phaseIndex", "1"}, {"topicIndex", "1"}, {"requestId", "client-a"}});
    // 【逐行批注】 计算右侧结果并写入 auto preparingOtherWindow，更新当前运行状态。
    const auto preparingOtherWindow = gangyi::deepCourseStart(db, access, {{"courseId", "new"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"requestId", "client-b"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(preparing.at("task").at("id") == preparingOtherWindow.at("task…。
    assert(preparing.at("task").at("id") == preparingOtherWindow.at("task").at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto preparingView，更新当前运行状态。
    const auto preparingView = gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"topicId", "topic-1"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(preparingView.at("activeTask").at("id") == preparing.at("task"…。
    assert(preparingView.at("activeTask").at("id") == preparing.at("task").at("id"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(preparingView.at("activeTask").at("status") == "pending");。
    assert(preparingView.at("activeTask").at("status") == "pending");

    // 【逐行批注】 计算右侧结果并写入 lesson，更新当前运行状态。
    auto lesson = Json{{"id", "lesson-new"}, {"scopeId", "fixture"}, {"courseId", "new"}, {"title", "二次函数"}, {"purpose", "观察系数与图像"},
        // 【逐行批注】 执行当前声明或表达式：{"status", "ready"}, {"version", 1}, {"sourceLearningVersion", db.learningRevision()…。
        {"status", "ready"}, {"version", 1}, {"sourceLearningVersion", db.learningRevision()}, {"entered", true}, {"intent", "advance"},
        // 【逐行批注】 执行当前声明或表达式：{"phaseIndex", 1}, {"topicIndex", 1}, {"topicId", "topic-1"}, {"updatedAt", "2026-10…。
        {"phaseIndex", 1}, {"topicIndex", 1}, {"topicId", "topic-1"}, {"updatedAt", "2026-10-28"},
        // 【逐行批注】 执行当前声明或表达式：{"sections", {{{"id", "section-q"}, {"kind", "question"}, {"title", "试着解释"}, {"versi…。
        {"sections", {{{"id", "section-q"}, {"kind", "question"}, {"title", "试着解释"}, {"version", 1}, {"displayed", false},
            // 【逐行批注】 执行当前声明或表达式：{"questionKind", "practice"}, {"questionId", "question-new"}, {"question", {{"questi…。
            {"questionKind", "practice"}, {"questionId", "question-new"}, {"question", {{"question", "a增加时图像怎样变化？"}, {"type", "open"}, {"expectedAnswer", "私有答案"}, {"rubric", "私有标准"}}}}}},
        // 【逐行批注】 执行当前声明或表达式：{"deepContent", normalized}};。
        {"deepContent", normalized}};
    // 【逐行批注】 执行当前声明或表达式：lesson["sections"].push_back({{"id", "concept"}, {"kind", "explanation"}, {"title", …。
    lesson["sections"].push_back({{"id", "concept"}, {"kind", "explanation"}, {"title", "观察系数变化"}, {"body", "已保存且实际可展示的讲解正文。"}, {"helpType", "explanation"}, {"version", 1}, {"displayed", false}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(gangyi::ClassroomActivity{"lesson-new", "new", "agen…。
    assert(db.upsert(gangyi::ClassroomActivity{"lesson-new", "new", "agent-lesson", lesson.dump(), "2026-10-28", 0, 0}));
    // 【逐行批注】 计算右侧结果并写入 auto view，更新当前运行状态。
    const auto view = gangyi::deepCourseView(db, access, {{"lessonId", "lesson-new"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(view.at("lesson").at("sections").at(0).at("question").at("ques…。
    assert(view.at("lesson").at("sections").at(0).at("question").at("question") == "a增加时图像怎样变化？");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(view.dump().find("私有答案") == std::string::npos && view.dump().f…。
    assert(view.dump().find("私有答案") == std::string::npos && view.dump().find("私有标准") == std::string::npos);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(view.at("content").at("slides").size() == 1);。
    assert(view.at("content").at("slides").size() == 1);
    // 【逐行批注】 调用 assert、deepCourseView 完成本行所需操作：assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"pha…。
    assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"phaseIndex", 1}, {"topicIndex", 1}}).at("lesson").at("id") == "lesson-new");
    // 【逐行批注】 调用 assert、deepCourseView 完成本行所需操作：assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"top…。
    assert(gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"topicId", "topic-1"}, {"phaseIndex", 99}, {"topicIndex", 99}}).at("lesson").at("id") == "lesson-new");
    // 【逐行批注】 调用 rejects、deepCourseView 完成本行所需操作：rejects([&] { gangyi::deepCourseView(db, access, {{"courseId", "new"}…。
    rejects([&] { gangyi::deepCourseView(db, access, {{"courseId", "new"}, {"topicId", "removed-topic"}}); });

    // 【逐行批注】 计算右侧结果并写入 int calls，更新当前运行状态。
    int calls = 0;
    // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
    gangyi::LearningAgent agent("", [&](const gangyi::ChatOptions&, const auto&) {
        // 【逐行批注】 执行当前声明或表达式：++calls;。
        ++calls;
        // 【逐行批注】 计算右侧结果并写入 changed，更新当前运行状态。
        auto changed = content(); changed["lecture"][0]["body"] = "不能冒充已保存正文的其它文字。";
        // 【逐行批注】 计算右侧结果并写入 changed["lecture"][0]["sectionId"]，更新当前运行状态。
        changed["lecture"][0]["sectionId"] = "concept";
        // 【逐行批注】 计算右侧结果并写入 changed["slides"][0]["sourceSectionIds"]，更新当前运行状态。
        changed["slides"][0]["sourceSectionIds"] = {"concept"};
        // 【逐行批注】 计算右侧结果并写入 Json output，更新当前运行状态。
        Json output = {{"teaching", {{"helpType", "none"}, {"demonstratedQuestions", Json::array()}}}, {"message", ""},
            // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "save-content"}, {"tool", "set_deep_content"}, {"args", {{"less…。
            {"actions", {{{"id", "save-content"}, {"tool", "set_deep_content"}, {"args", {{"lessonId", "lesson-new"}, {"content", changed}}}}}}, {"state", "completed"}};
        // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
        gangyi::AIResult result; result.content = output.dump(); result.model = "fixture-deep-protocol"; result.finishReason = "stop"; return result;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
    const auto task = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"courseId", "new"}, {"lessonId", "lesson-new"}, {"text", "补充观察步骤"}, {"requestId", "deep-change"}});
    // 【逐行批注】 执行当前声明或表达式：agent.process(db, task.at("id"));。
    agent.process(db, task.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto savedTask，更新当前运行状态。
    const auto savedTask = gangyi::agentView(db, access, task.at("id"));
    // 【逐行批注】 判断条件 (savedTask.at("status") != "ready") std::cerr << savedTask.dump(…，成立时执行对应分支。
    if (savedTask.at("status") != "ready") std::cerr << savedTask.dump(2) << '\n';
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(savedTask.at("status") == "ready");。
    assert(savedTask.at("status") == "ready");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(calls == 1);。
    assert(calls == 1);
    // 【逐行批注】 计算右侧结果并写入 auto updated，更新当前运行状态。
    const auto updated = gangyi::deepCourseView(db, access, {{"lessonId", "lesson-new"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(updated.at("content").at("lecture").at(0).at("body") == "已保存且实…。
    assert(updated.at("content").at("lecture").at(0).at("body") == "已保存且实际可展示的讲解正文。");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(updated.at("content").at("slides").at(0).at("sourceSectionIds"…。
    assert(updated.at("content").at("slides").at(0).at("sourceSectionIds").at(0) == "concept");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(updated.at("content").at("slides").at(0).at("body") == "已保存且实际…。
    assert(updated.at("content").at("slides").at(0).at("body") == "已保存且实际可展示的讲解正文。");
    // 【逐行批注】 调用 assert、deepCourseRetryMedia 完成本行所需操作：assert(gangyi::deepCourseRetryMedia(db, access, {{"lessonId", "lesson…。
    assert(gangyi::deepCourseRetryMedia(db, access, {{"lessonId", "lesson-new"}, {"mediaId", "curve"}, {"requestId", "retry-ready"}}).at("cached") == true);
    // 【逐行批注】 计算右侧结果并写入 auto updatedRevision，更新当前运行状态。
    const auto updatedRevision = db.learningRevision();
    // 【逐行批注】 按循环规则重复处理：for (int count = 0; count < 3; ++count) gangyi::deepCourseView(db, access,…。
    for (int count = 0; count < 3; ++count) gangyi::deepCourseView(db, access, {{"lessonId", "lesson-new"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(calls == 1 && db.learningRevision() == updatedRevision);。
    assert(calls == 1 && db.learningRevision() == updatedRevision);

    // 【逐行批注】 计算右侧结果并写入 auto second，更新当前运行状态。
    const auto second = gangyi::deepCourseStart(db, access, {{"courseId", "new"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"requestId", "start"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(second.at("lessonId") == "lesson-new");。
    assert(second.at("lessonId") == "lesson-new");
    // 【逐行批注】 计算右侧结果并写入 auto same，更新当前运行状态。
    const auto same = gangyi::deepCourseStart(db, access, {{"courseId", "new"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"requestId", "start"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(second.at("cached") == true && same.at("cached") == true && !s…。
    assert(second.at("cached") == true && same.at("cached") == true && !second.contains("task"));
    // 【逐行批注】 调用 rejects、deepCourseImage 完成本行所需操作：rejects([&] { gangyi::deepCourseImage(db, access, "lesson-new", "curv…。
    rejects([&] { gangyi::deepCourseImage(db, access, "lesson-new", "curve"); });
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"image-new", "deep-image", Json{{"courseId"…。
    assert(db.saveMyAIRecord({"image-new", "deep-image", Json{{"courseId", "new"}, {"data", "image-bytes-must-not-enter-context"}}.dump(), "2026-10-28", 1}, 0));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"image-old", "deep-image", Json{{"courseId"…。
    assert(db.saveMyAIRecord({"image-old", "deep-image", Json{{"courseId", "old"}, {"data", "other-course"}}.dump(), "2026-10-28", 1}, 0));
    // 【逐行批注】 调用 assert、agentContext 完成本行所需操作：assert(gangyi::agentContext(db, access, {{"type", "chat"}, {"courseId…。
    assert(gangyi::agentContext(db, access, {{"type", "chat"}, {"courseId", "new"}}).dump().find("image-bytes-must-not-enter-context") == std::string::npos);
    // 【逐行批注】 执行当前声明或表达式：db.deleteClassroomData("new");。
    db.deleteClassroomData("new");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!db.myAIRecord("image-new") && db.myAIRecord("image-old"));。
    assert(!db.myAIRecord("image-new") && db.myAIRecord("image-old"));
    // 【逐行批注】 执行当前声明或表达式：std::cout << "深度课程标记兼容、结构校验、图片来源解析、私有答案过滤、主控素材保存与读取复用检查通过；付费请求0。\n";。
    std::cout << "深度课程标记兼容、结构校验、图片来源解析、私有答案过滤、主控素材保存与读取复用检查通过；付费请求0。\n";
// 【逐行批注】 结束当前代码块。
}

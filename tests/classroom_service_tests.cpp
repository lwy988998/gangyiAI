#include "classroom_service.hpp"
#include "db_schema_version.hpp"

#include <sqlite3.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>

int main() {
#ifdef _WIN32
    _putenv_s("AI_API_KEY", "");
#else
    setenv("AI_API_KEY", "", 1);
#endif
    int failures = 0;
    const auto check = [&](bool value, const char* message) {
        if (!value) { std::cerr << "失败：" << message << '\n'; ++failures; }
    };
    using gangyi::Json;
    check(gangyi::diagnosticMode(Json::array({true, false}), false) == "third", "诊断不明确应追加第三题");
    check(gangyi::diagnosticMode(Json::array({false, false}), false) == "weak", "两题均错应补弱");
    check(gangyi::diagnosticMode(Json::array({true, true}), false) == "familiar", "两题均对应精简");
    check(gangyi::diagnosticMode(Json::array({true, false, true}), false) == "full", "三题一般水平应完整教学");
    check(gangyi::diagnosticMode(Json::array(), true) == "full", "跳过诊断应完整教学");
    check(gangyi::diagnosticMode(Json::array({nullptr, true}), false) == "third", "低可信诊断应追加第三题");
    check(gangyi::diagnosticMode(Json::array({nullptr, true, false}), false) == "full", "低可信诊断不能推断掌握");
    check(!gangyi::credibleOpenEvaluation({{"correct", true}, {"confidence", 0.5}, {"feedback", "具体错误说明"}, {"followUp", "请解释原因"}}), "低可信评价不得计入证据");
    check(gangyi::credibleOpenEvaluation({{"correct", true}, {"confidence", 0.9}, {"feedback", "解释准确且有例子"}, {"followUp", "换个例子？"}}), "有效评价应被接纳");
    const Json tooFew = Json::array({{{"kind", "quiz"}, {"direction", "weak"}, {"credible", true}},
        {{"kind", "diagnostic"}, {"direction", "weak"}, {"credible", true}}});
    check(!gangyi::sufficientPathEvidence(tooFew, "weak"), "一次错答不能调整课程");
    Json lowConfidence = tooFew;
    lowConfidence.push_back({{"kind", "interaction"}, {"direction", "weak"}, {"credible", false}});
    check(!gangyi::sufficientPathEvidence(lowConfidence, "weak"), "低可信 AI 评价不得补足调课证据");
    Json enough = tooFew;
    enough.push_back({{"kind", "diagnostic"}, {"direction", "weak"}, {"credible", true}});
    check(gangyi::sufficientPathEvidence(enough, "weak"), "三条两类且含客观测验才能调整");
    check(!gangyi::sufficientPathEvidence(enough, "strong"), "证据方向不能混用");
    const Json draft = gangyi::buildWeeklyDraft(gangyi::defaultAvailability(), Json::array({"A", "B", "C"}),
        Json::array({{{"title", "A复习"}, {"due", "2026-09-30"}}}), "2026-09-28");
    check(draft.size() == 4 && draft[1]["kind"] == "review" && draft[1]["minutes"] == 10,
          "周三先排到期短复习");

    gangyi::Database db;
    db.open(":memory:"); db.migrate();
    bool diagnosticFailed = false;
    try { gangyi::classroomStart(db, "course-fast", 1, 1, "diagnostic", "函数单调性"); }
    catch (...) { diagnosticFailed = true; }
    check(diagnosticFailed, "没有真实 AI 或已保存题目时不能生成模板诊断题");
    for (int i = 0; i < 3; ++i) db.upsert(gangyi::ClassroomActivity{"classroom:course-fast:1:1:diagnostic:" + std::to_string(i),
        "course-fast", "diagnostic", Json{{"question", "虚构题"}, {"type", "open"}, {"status", "pending"}}.dump(), "2026-09-28", 1, 1});
    const auto immediate = gangyi::classroomStart(db, "course-fast", 1, 1, "diagnostic", "函数单调性");
    check(immediate["questions"].size() == 2, "已有真实题目应直接读取");
    check(gangyi::classroomStart(db, "course-fast", 1, 1, "diagnostic", "函数单调性")["questions"] == immediate["questions"],
          "刷新后应复用已保存诊断题");
    check(db.upsert(gangyi::ClassroomActivity{"activity-1", "course-1", "interaction", "{}", "2026-09-28", 1, 1}), "课堂活动应保存");
    check(db.getClassroomActivity("activity-1").has_value(), "课堂活动应读取");
    const auto skipped = gangyi::classroomSkip(db, "course-1", 1, 1);
    check(skipped.value("mode", "") == "full", "跳过诊断应保留完整课堂");
    const auto finished = gangyi::finishClassroom(db, "course-1", 1, 1, true, "2026-09-28");
    check(finished.value("newLessonAllowed", false), "复习不应锁住下一课");
    const auto due = gangyi::dueReviews(db, "course-1", "2026-10-05");
    check(due.size() == 3, "掌握后应安排第1、3、7天复习");
    check(db.upsert(gangyi::ClassroomActivity{"classroom:familiar:1:1:state", "familiar", "state",
        Json{{"diagnosticMode", "familiar"}}.dump(), "2026-09-28", 1, 1}), "熟悉分流状态应保存");
    check(db.upsert(gangyi::ClassroomActivity{"classroom:familiar:1:1:interaction:2", "familiar", "interaction",
        Json{{"status", "pending"}, {"correct", false}}.dump(), "2026-09-28", 1, 1}), "挑战题应保存");
    const auto blockedMastery = gangyi::finishClassroom(db, "familiar", 1, 1, true, "2026-09-28");
    check(blockedMastery["challengeRequired"] == true && blockedMastery["nextStep"] == "remedial",
          "熟悉分流未通过新情境挑战时不得记录掌握");
    check(db.upsert(gangyi::ClassroomActivity{"classroom:familiar:1:1:interaction:2", "familiar", "interaction",
        Json{{"status", "answered"}, {"correct", true}, {"credible", true}}.dump(), "2026-09-28", 1, 1}), "挑战结果应保存");
    const auto acceptedMastery = gangyi::finishClassroom(db, "familiar", 1, 1, true, "2026-09-28");
    check(acceptedMastery["challengeRequired"] == false && acceptedMastery["nextStep"] == "review",
          "熟悉分流通过新情境挑战后可记录掌握");

    gangyi::Course noAi{"no-ai", std::string("anon"), std::nullopt, "目标", "deep", "课程",
        std::nullopt, "ai", "active", "2026-09-28", "2026-09-28"};
    check(db.insert(noAi), "AI 失败场景课程应保存");
    check(db.insert(gangyi::CourseSnapshot{"", "no-ai", 1,
        Json{{"courseStructure", Json::array({{{"stage", "阶段"}, {"topics", Json::array({"A", "B"})}}})},
             {"roadmap", Json::array({{{"name", "阶段"}, {"topics", Json::array({"A", "B"})}}})}}.dump(),
        "2026-09-28"}), "AI 失败场景快照应保存");
    for (int index = 0; index < 2; ++index)
        check(db.upsert(gangyi::ClassroomActivity{"classroom:no-ai:1:1:diagnostic:" + std::to_string(index),
            "no-ai", "diagnostic", Json{{"status", "answered"}, {"correct", false}, {"credible", true}}.dump(),
            "2026-09-28", 1, 1}), "调课证据应保存");
    check(db.insert(gangyi::LearningInteraction{"", "quiz",
        Json{{"phaseIndex", 1}, {"topicIndex", 1}, {"score", 0}, {"total", 3}}.dump(), "2026-09-28",
        std::string("no-ai"), std::nullopt, std::nullopt}), "客观测验证据应保存");
    const auto noAiFinish = gangyi::finishClassroom(db, "no-ai", 1, 1, false, "2026-09-28");
    check(noAiFinish["pathAdjustment"] == "" && db.findSnapshotsByCourseId("no-ai").size() == 1,
          "AI 失败时不得调整路径或写入新快照");

    const Json recoveryQuiz = Json::array({
        {{"question", "一"}, {"options", Json::array({"对", "错", "其他", "不确定"})}, {"answerIndex", 0}},
        {{"question", "二"}, {"options", Json::array({"对", "错", "其他", "不确定"})}, {"answerIndex", 0}},
        {{"question", "三"}, {"options", Json::array({"对", "错", "其他", "不确定"})}, {"answerIndex", 0}}});
    check(db.insert(gangyi::LearningSession{"recover-session", std::string("recover"), std::string("anon"),
        "目标", std::string("deep"), 1, "阶段", 1, "主题", "主题", std::nullopt, std::nullopt,
        Json{{"blocks", {{"quiz", {{"quiz", recoveryQuiz}}}}}}.dump(), std::nullopt, 0, "ai"}),
        "复测课堂应保存");
    gangyi::finishClassroom(db, "recover", 1, 1, false, "2026-09-28");
    const auto recovered = gangyi::submitReview(db, "recover", 1, 1, 1, Json::array({0, 0, 0}), "2026-09-29");
    check(recovered["passed"] == true && recovered["nextDue"] == "2026-09-30" &&
        gangyi::dueReviews(db, "recover", "2026-10-06").size() == 3,
        "补弱复测通过后应从掌握日重新安排第1、3、7天复习");
    const auto recoveredDue = gangyi::dueReviews(db, "recover", "2026-09-30");
    check(recoveredDue.size() == 1 && recoveredDue[0].contains("reviewId"),
        "重新安排的复习应返回唯一任务标识");
    const auto repeated = gangyi::submitReview(db, "recover", 1, 1,
        recoveredDue[0].at("day").get<int>(), Json::array({0, 0, 0}), "2026-09-30",
        recoveredDue[0].at("reviewId").get<std::string>());
    check(repeated["passed"] == true && repeated["nextDue"] == "",
        "唯一任务标识应提交对应复习且不得重复建立复习周期");

    gangyi::WeeklyPlan plan{"course-1", Json{{"availability", gangyi::defaultAvailability()},
        {"entries", draft}, {"weekStart", "2026-09-28"}, {"version", 1}}.dump(), "2026-09-28", 1};
    check(db.upsert(plan), "周计划应保存");
    Json edited = draft;
    edited[0]["minutes"] = 25;
    const auto saved = gangyi::editWeeklyPlan(db, "course-1", {{"version", 1}, {"entries", edited}});
    check(saved["entries"][0]["manual"] == true && saved["entries"][1]["manual"] == false,
        "仅被修改的安排应标记为手动");
    const auto proposed = gangyi::replanWeeklyPlan(db, "course-1", Json::array({"A", "B", "C"}), "2026-09-28", false);
    check(proposed.value("requiresConfirmation", false), "覆盖手动安排前必须确认");
    check(db.getWeeklyPlan("course-1")->version == 2, "未确认的重排不得写入");
    const auto nextWeek = gangyi::getWeeklyPlan(db, "course-1", Json::array({"A", "B", "C"}), "2026-10-05");
    check(nextWeek["weekStart"] == "2026-10-05" && nextWeek["version"] == 3 &&
        nextWeek["availability"] == gangyi::defaultAvailability(), "跨周应重新排课并保留可用时间");
    const Json indexed = Json::array({{{"title", "A"}, {"phaseIndex", 1}, {"topicIndex", 1}},
        {{"title", "B"}, {"phaseIndex", 1}, {"topicIndex", 2}},
        {{"title", "C"}, {"phaseIndex", 1}, {"topicIndex", 3}}});
    check(db.upsert(gangyi::WeeklyPlan{"course-1", Json{{"availability", gangyi::defaultAvailability()},
        {"entries", draft}, {"weekStart", "2026-09-28"}, {"version", 20}}.dump(), "2026-09-28", 20}), "旧课表应保存");
    const auto weekend = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        {{"version", 20}}, "2026-10-03");
    check(weekend["plan"]["weekStart"] == "2026-10-05" && weekend["changed"] == true &&
        weekend["source"] == "rules", "周末应顺延下周，AI 失败仍按规则排课");
    for (const auto& entry : weekend["plan"]["entries"]) check(entry["date"].get<std::string>() >= "2026-10-03", "重排不得安排到过去");
    const auto preserved = gangyi::getWeeklyPlan(db, "course-1", indexed, "2026-09-28", "2026-10-03");
    check(preserved == weekend["plan"], "本周读取必须保留顺延结果");
    const auto same = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        {{"version", 21}}, "2026-10-03");
    check(same["changed"] == false && same["plan"]["version"] == 21, "无需变化时不得增加版本");
    const auto saturday = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        {{"version", 21}, {"availability", Json::array({{{"weekday", 6}, {"minutes", 45}}})}}, "2026-10-03");
    check(saturday["plan"]["entries"][0]["date"] == "2026-10-03" &&
        saturday["plan"]["availability"][0]["minutes"] == 45, "应使用页面未保存的可用时间");
    check(db.insert(gangyi::LearningInteraction{"", "quiz", Json{{"phaseIndex", 1}, {"topicIndex", 2},
        {"topic", "B"}, {"score", 0}, {"total", 3}, {"results", Json::array({{{"answered", true}, {"unknown", true}}})}}.dump(),
        "2026-10-03T12:00:00Z", std::string("course-1"), std::nullopt, std::nullopt}), "明确不会的证据应保存");
    const auto weakFirst = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        {{"version", 22}, {"availability", gangyi::defaultAvailability()}}, "2026-09-29");
    const auto firstLesson = std::find_if(weakFirst["plan"]["entries"].begin(), weakFirst["plan"]["entries"].end(),
        [](const Json& entry) { return entry.value("kind", "") == "lesson"; });
    check(firstLesson != weakFirst["plan"]["entries"].end() && (*firstLesson)["title"] == "B" && (*firstLesson)["date"] == "2026-09-30",
        "规则应优先安排薄弱课时，跳过已过去的周一");
    Json manual = weakFirst["plan"]["entries"]; manual[0]["minutes"] = 17;
    const auto preview = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        {{"version", 23}, {"entries", manual}}, "2026-09-29");
    check(preview["requiresConfirmation"] == true && db.getWeeklyPlan("course-1")->version == 23,
        "未保存手动修改也必须预览，预览不写数据库");
    const auto accepted = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", true,
        {{"version", 23}, {"proposalId", preview["proposalId"]}}, "2026-09-29");
    check(accepted["plan"]["entries"] == preview["proposed"] && accepted["plan"]["version"] == 24,
        "确认应应用相同预览并只增加一次版本");
    const auto stale = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        {{"version", 24}, {"entries", manual}}, "2026-09-29");
    gangyi::editWeeklyPlan(db, "course-1", {{"version", 24}, {"entries", accepted["plan"]["entries"]}});
    bool conflict = false;
    try { gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", true,
        {{"version", 24}, {"proposalId", stale["proposalId"]}}, "2026-09-29"); }
    catch (const std::invalid_argument&) { conflict = true; }
    check(conflict && db.getWeeklyPlan("course-1")->version == 25, "其他窗口修改后必须拒绝旧确认");
    for (auto row : db.listClassroomActivities("course-1")) if (row.kind == "review") {
        Json value = Json::parse(row.payload); value["status"] = "passed"; row.payload = value.dump(); db.upsert(row);
    }
    const auto empty = gangyi::replanWeeklyPlan(db, "course-1", Json::array(), "2026-09-28", false,
        {{"version", 25}}, "2026-09-29");
    check(empty["changed"] == false && empty["plan"]["version"] == 25, "没有待安排内容时应给出明确结果");
    std::cout << (failures ? "课堂规则测试失败" : "课堂规则测试通过") << '\n';
    return failures ? 1 : 0;
}

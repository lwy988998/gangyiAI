// 【逐行批注】 引入头文件 "classroom_service.hpp"，为后续实现提供类型或接口声明。
#include "classroom_service.hpp"
// 【逐行批注】 引入头文件 "db_schema_version.hpp"，为后续实现提供类型或接口声明。
#include "db_schema_version.hpp"
// 【逐行批注】 引入头文件 "question_evidence.hpp"，为后续实现提供类型或接口声明。
#include "question_evidence.hpp"

// 【逐行批注】 引入头文件 <sqlite3.h>，为后续实现提供类型或接口声明。
#include <sqlite3.h>
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 _putenv_s 完成本行所需操作：_putenv_s("AI_API_KEY", "");。
    _putenv_s("AI_API_KEY", "");
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 setenv 完成本行所需操作：setenv("AI_API_KEY", "", 1);。
    setenv("AI_API_KEY", "", 1);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 计算右侧结果并写入 int failures，更新当前运行状态。
    int failures = 0;
    // 【逐行批注】 计算右侧结果并写入 auto check，更新当前运行状态。
    const auto check = [&](bool value, const char* message) {
        // 【逐行批注】 判断条件 (!value) { std::cerr << "失败：" << message << '\n'; ++failures; }，成立时执行对应分支。
        if (!value) { std::cerr << "失败：" << message << '\n'; ++failures; }
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 执行当前声明或表达式：using gangyi::Json;。
    using gangyi::Json;
    // 【逐行批注】 计算右侧结果并写入 Json privateQuestion，更新当前运行状态。
    const Json privateQuestion = {{"question", "只公开题干"}, {"options", Json::array({Json{{"text", "选项一"}, {"correct", true}}, "选项二"})},
        // 【逐行批注】 执行当前声明或表达式：{"answerIndex", 0}, {"rubric", "私有评分"}, {"solution", "私有解析"}, {"status", "pending"}};。
        {"answerIndex", 0}, {"rubric", "私有评分"}, {"solution", "私有解析"}, {"status", "pending"}};
    // 【逐行批注】 计算右侧结果并写入 auto publicQuestion，更新当前运行状态。
    const auto publicQuestion = gangyi::publicQuestion(privateQuestion);
    // 【逐行批注】 调用 check 完成本行所需操作：check(!publicQuestion.contains("answerIndex") && !publicQuestion.cont…。
    check(!publicQuestion.contains("answerIndex") && !publicQuestion.contains("rubric") && !publicQuestion.contains("solution") &&
        // 【逐行批注】 执行当前声明或表达式：publicQuestion["options"][0] == "选项一", "公开题目连嵌套选项的正确标记也必须隐藏");。
        publicQuestion["options"][0] == "选项一", "公开题目连嵌套选项的正确标记也必须隐藏");
    // 【逐行批注】 计算右侧结果并写入 correctedStandard，更新当前运行状态。
    auto correctedStandard = privateQuestion; correctedStandard["rubric"] = "校正后的评分"; correctedStandard["solution"] = "校正后的解法";
    // 【逐行批注】 调用 check、questionIdentity、questionSnapshot 完成本行所需操作：check(gangyi::questionIdentity("同一课程", gangyi::questionSnapshot(priva…。
    check(gangyi::questionIdentity("同一课程", gangyi::questionSnapshot(privateQuestion)) ==
        // 【逐行批注】 调用 questionIdentity、questionSnapshot 完成本行所需操作：gangyi::questionIdentity("同一课程", gangyi::questionSnapshot(correctedSt…。
        gangyi::questionIdentity("同一课程", gangyi::questionSnapshot(correctedStandard)), "校正标准答案不能增加独立题目证据");
    // 【逐行批注】 调用 check、diagnosticMode、array 完成本行所需操作：check(gangyi::diagnosticMode(Json::array({true, false}), false) == "t…。
    check(gangyi::diagnosticMode(Json::array({true, false}), false) == "third", "诊断不明确应追加第三题");
    // 【逐行批注】 调用 check、diagnosticMode、array 完成本行所需操作：check(gangyi::diagnosticMode(Json::array({false, false}), false) == "…。
    check(gangyi::diagnosticMode(Json::array({false, false}), false) == "weak", "两题均错应补弱");
    // 【逐行批注】 调用 check、diagnosticMode、array 完成本行所需操作：check(gangyi::diagnosticMode(Json::array({true, true}), false) == "fa…。
    check(gangyi::diagnosticMode(Json::array({true, true}), false) == "familiar", "两题均对应精简");
    // 【逐行批注】 调用 check、diagnosticMode、array 完成本行所需操作：check(gangyi::diagnosticMode(Json::array({true, false, true}), false)…。
    check(gangyi::diagnosticMode(Json::array({true, false, true}), false) == "full", "三题一般水平应完整教学");
    // 【逐行批注】 调用 check、diagnosticMode、array 完成本行所需操作：check(gangyi::diagnosticMode(Json::array(), true) == "full", "跳过诊断应完整…。
    check(gangyi::diagnosticMode(Json::array(), true) == "full", "跳过诊断应完整教学");
    // 【逐行批注】 调用 check、diagnosticMode、array 完成本行所需操作：check(gangyi::diagnosticMode(Json::array({nullptr, true}), false) == …。
    check(gangyi::diagnosticMode(Json::array({nullptr, true}), false) == "third", "低可信诊断应追加第三题");
    // 【逐行批注】 调用 check、diagnosticMode、array 完成本行所需操作：check(gangyi::diagnosticMode(Json::array({nullptr, true, false}), fal…。
    check(gangyi::diagnosticMode(Json::array({nullptr, true, false}), false) == "full", "低可信诊断不能推断掌握");
    // 【逐行批注】 调用 check、credibleOpenEvaluation 完成本行所需操作：check(!gangyi::credibleOpenEvaluation({{"correct", true}, {"confidenc…。
    check(!gangyi::credibleOpenEvaluation({{"correct", true}, {"confidence", 0.5}, {"feedback", "具体错误说明"}, {"followUp", "请解释原因"}}), "低可信评价不得计入证据");
    // 【逐行批注】 调用 check、credibleOpenEvaluation 完成本行所需操作：check(gangyi::credibleOpenEvaluation({{"correct", true}, {"confidence…。
    check(gangyi::credibleOpenEvaluation({{"correct", true}, {"confidence", 0.9}, {"feedback", "解释准确且有例子"}, {"followUp", "换个例子？"}}), "有效评价应被接纳");
    // 【逐行批注】 计算右侧结果并写入 Json tooFew，更新当前运行状态。
    const Json tooFew = Json::array({{{"kind", "quiz"}, {"direction", "weak"}, {"credible", true}},
        // 【逐行批注】 执行当前声明或表达式：{{"kind", "diagnostic"}, {"direction", "weak"}, {"credible", true}}});。
        {{"kind", "diagnostic"}, {"direction", "weak"}, {"credible", true}}});
    // 【逐行批注】 调用 check、sufficientPathEvidence 完成本行所需操作：check(!gangyi::sufficientPathEvidence(tooFew, "weak"), "一次错答不能调整课程");。
    check(!gangyi::sufficientPathEvidence(tooFew, "weak"), "一次错答不能调整课程");
    // 【逐行批注】 计算右侧结果并写入 Json lowConfidence，更新当前运行状态。
    Json lowConfidence = tooFew;
    // 【逐行批注】 执行当前声明或表达式：lowConfidence.push_back({{"kind", "interaction"}, {"direction", "weak"}, {"credible"…。
    lowConfidence.push_back({{"kind", "interaction"}, {"direction", "weak"}, {"credible", false}});
    // 【逐行批注】 调用 check、sufficientPathEvidence 完成本行所需操作：check(!gangyi::sufficientPathEvidence(lowConfidence, "weak"), "低可信 AI…。
    check(!gangyi::sufficientPathEvidence(lowConfidence, "weak"), "低可信 AI 评价不得补足调课证据");
    // 【逐行批注】 计算右侧结果并写入 Json enough，更新当前运行状态。
    Json enough = tooFew;
    // 【逐行批注】 执行当前声明或表达式：enough.push_back({{"kind", "diagnostic"}, {"direction", "weak"}, {"credible", true}}…。
    enough.push_back({{"kind", "diagnostic"}, {"direction", "weak"}, {"credible", true}});
    // 【逐行批注】 调用 check、sufficientPathEvidence 完成本行所需操作：check(gangyi::sufficientPathEvidence(enough, "weak"), "三条两类且含客观测验才能调整…。
    check(gangyi::sufficientPathEvidence(enough, "weak"), "三条两类且含客观测验才能调整");
    // 【逐行批注】 调用 check、sufficientPathEvidence 完成本行所需操作：check(!gangyi::sufficientPathEvidence(enough, "strong"), "证据方向不能混用");。
    check(!gangyi::sufficientPathEvidence(enough, "strong"), "证据方向不能混用");
    // 【逐行批注】 计算右侧结果并写入 Json draft，更新当前运行状态。
    const Json draft = gangyi::buildWeeklyDraft(gangyi::defaultAvailability(), Json::array({"A", "B", "C"}),
        // 【逐行批注】 调用 array 完成本行所需操作：Json::array({{{"title", "A复习"}, {"due", "2026-09-30"}}}), "2026-09-28…。
        Json::array({{{"title", "A复习"}, {"due", "2026-09-30"}}}), "2026-09-28");
    // 【逐行批注】 调用 check 完成本行所需操作：check(draft.size() == 4 && draft[1]["kind"] == "review" && draft[1]["…。
    check(draft.size() == 4 && draft[1]["kind"] == "review" && draft[1]["minutes"] == 10,
          // 【逐行批注】 执行当前声明或表达式："周三先排到期短复习");。
          "周三先排到期短复习");

    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db;。
    gangyi::Database db;
    // 【逐行批注】 执行当前声明或表达式：db.open(":memory:"); db.migrate();。
    db.open(":memory:"); db.migrate();
    // 【逐行批注】 计算右侧结果并写入 bool diagnosticFailed，更新当前运行状态。
    bool diagnosticFailed = false;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { gangyi::classroomStart(db, "course-fast", 1, 1, "diagnostic", "函数单调性"); }
    // 【逐行批注】 捕获并处理异常：catch (...) { diagnosticFailed = true; }。
    catch (...) { diagnosticFailed = true; }
    // 【逐行批注】 调用 check 完成本行所需操作：check(diagnosticFailed, "没有真实 AI 或已保存题目时不能生成模板诊断题");。
    check(diagnosticFailed, "没有真实 AI 或已保存题目时不能生成模板诊断题");
    // 【逐行批注】 按循环规则重复处理：for (int i = 0; i < 3; ++i) db.upsert(gangyi::ClassroomActivity{"classroom…。
    for (int i = 0; i < 3; ++i) db.upsert(gangyi::ClassroomActivity{"classroom:course-fast:1:1:diagnostic:" + std::to_string(i),
        // 【逐行批注】 执行当前声明或表达式："course-fast", "diagnostic", Json{{"question", "虚构题"}, {"type", "open"}, {"status", …。
        "course-fast", "diagnostic", Json{{"question", "虚构题"}, {"type", "open"}, {"status", "pending"}}.dump(), "2026-09-28", 1, 1});
    // 【逐行批注】 计算右侧结果并写入 auto immediate，更新当前运行状态。
    const auto immediate = gangyi::classroomStart(db, "course-fast", 1, 1, "diagnostic", "函数单调性");
    // 【逐行批注】 调用 check 完成本行所需操作：check(immediate["questions"].size() == 2, "已有真实题目应直接读取");。
    check(immediate["questions"].size() == 2, "已有真实题目应直接读取");
    // 【逐行批注】 调用 check、classroomStart 完成本行所需操作：check(gangyi::classroomStart(db, "course-fast", 1, 1, "diagnostic", "…。
    check(gangyi::classroomStart(db, "course-fast", 1, 1, "diagnostic", "函数单调性")["questions"] == immediate["questions"],
          // 【逐行批注】 执行当前声明或表达式："刷新后应复用已保存诊断题");。
          "刷新后应复用已保存诊断题");
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.upsert(gangyi::ClassroomActivity{"activity-1", "course-1", "…。
    check(db.upsert(gangyi::ClassroomActivity{"activity-1", "course-1", "interaction", "{}", "2026-09-28", 1, 1}), "课堂活动应保存");
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.getClassroomActivity("activity-1").has_value(), "课堂活动应读取");。
    check(db.getClassroomActivity("activity-1").has_value(), "课堂活动应读取");
    // 【逐行批注】 计算右侧结果并写入 auto skipped，更新当前运行状态。
    const auto skipped = gangyi::classroomSkip(db, "course-1", 1, 1);
    // 【逐行批注】 调用 check 完成本行所需操作：check(skipped.value("mode", "") == "full", "跳过诊断应保留完整课堂");。
    check(skipped.value("mode", "") == "full", "跳过诊断应保留完整课堂");
    // 【逐行批注】 计算右侧结果并写入 auto finished，更新当前运行状态。
    const auto finished = gangyi::finishClassroom(db, "course-1", 1, 1, true, "2026-09-28");
    // 【逐行批注】 调用 check 完成本行所需操作：check(finished.value("newLessonAllowed", false), "复习不应锁住下一课");。
    check(finished.value("newLessonAllowed", false), "复习不应锁住下一课");
    // 【逐行批注】 计算右侧结果并写入 auto due，更新当前运行状态。
    const auto due = gangyi::dueReviews(db, "course-1", "2026-10-05");
    // 【逐行批注】 调用 check 完成本行所需操作：check(due.size() == 3, "掌握后应安排第1、3、7天复习");。
    check(due.size() == 3, "掌握后应安排第1、3、7天复习");
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.upsert(gangyi::ClassroomActivity{"classroom:familiar:1:1:sta…。
    check(db.upsert(gangyi::ClassroomActivity{"classroom:familiar:1:1:state", "familiar", "state",
        // 【逐行批注】 执行当前声明或表达式：Json{{"diagnosticMode", "familiar"}}.dump(), "2026-09-28", 1, 1}), "熟悉分流状态应保存");。
        Json{{"diagnosticMode", "familiar"}}.dump(), "2026-09-28", 1, 1}), "熟悉分流状态应保存");
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.upsert(gangyi::ClassroomActivity{"classroom:familiar:1:1:int…。
    check(db.upsert(gangyi::ClassroomActivity{"classroom:familiar:1:1:interaction:2", "familiar", "interaction",
        // 【逐行批注】 执行当前声明或表达式：Json{{"status", "pending"}, {"correct", false}}.dump(), "2026-09-28", 1, 1}), "挑战题应保…。
        Json{{"status", "pending"}, {"correct", false}}.dump(), "2026-09-28", 1, 1}), "挑战题应保存");
    // 【逐行批注】 计算右侧结果并写入 auto blockedMastery，更新当前运行状态。
    const auto blockedMastery = gangyi::finishClassroom(db, "familiar", 1, 1, true, "2026-09-28");
    // 【逐行批注】 调用 check 完成本行所需操作：check(blockedMastery["challengeRequired"] == true && blockedMastery["…。
    check(blockedMastery["challengeRequired"] == true && blockedMastery["nextStep"] == "remedial",
          // 【逐行批注】 执行当前声明或表达式："熟悉分流未通过新情境挑战时不得记录掌握");。
          "熟悉分流未通过新情境挑战时不得记录掌握");
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.upsert(gangyi::ClassroomActivity{"classroom:familiar:1:1:int…。
    check(db.upsert(gangyi::ClassroomActivity{"classroom:familiar:1:1:interaction:2", "familiar", "interaction",
        // 【逐行批注】 执行当前声明或表达式：Json{{"status", "answered"}, {"correct", true}, {"credible", true}}.dump(), "2026-09…。
        Json{{"status", "answered"}, {"correct", true}, {"credible", true}}.dump(), "2026-09-28", 1, 1}), "挑战结果应保存");
    // 【逐行批注】 计算右侧结果并写入 auto acceptedMastery，更新当前运行状态。
    const auto acceptedMastery = gangyi::finishClassroom(db, "familiar", 1, 1, true, "2026-09-28");
    // 【逐行批注】 调用 check 完成本行所需操作：check(acceptedMastery["challengeRequired"] == false && acceptedMaster…。
    check(acceptedMastery["challengeRequired"] == false && acceptedMastery["nextStep"] == "review",
          // 【逐行批注】 执行当前声明或表达式："熟悉分流通过新情境挑战后可记录掌握");。
          "熟悉分流通过新情境挑战后可记录掌握");

    // 【逐行批注】 调用 string 完成本行所需操作：gangyi::Course noAi{"no-ai", std::string("anon"), std::nullopt, "目标",…。
    gangyi::Course noAi{"no-ai", std::string("anon"), std::nullopt, "目标", "deep", "课程",
        // 【逐行批注】 执行当前声明或表达式：std::nullopt, "ai", "active", "2026-09-28", "2026-09-28"};。
        std::nullopt, "ai", "active", "2026-09-28", "2026-09-28"};
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.insert(noAi), "AI 失败场景课程应保存");。
    check(db.insert(noAi), "AI 失败场景课程应保存");
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.insert(gangyi::CourseSnapshot{"", "no-ai", 1,。
    check(db.insert(gangyi::CourseSnapshot{"", "no-ai", 1,
        // 【逐行批注】 调用 array 完成本行所需操作：Json{{"courseStructure", Json::array({{{"stage", "阶段"}, {"topics", Js…。
        Json{{"courseStructure", Json::array({{{"stage", "阶段"}, {"topics", Json::array({"A", "B"})}}})},
             // 【逐行批注】 调用 array 完成本行所需操作：{"roadmap", Json::array({{{"name", "阶段"}, {"topics", Json::array({"A"…。
             {"roadmap", Json::array({{{"name", "阶段"}, {"topics", Json::array({"A", "B"})}}})}}.dump(),
        // 【逐行批注】 执行当前声明或表达式："2026-09-28"}), "AI 失败场景快照应保存");。
        "2026-09-28"}), "AI 失败场景快照应保存");
    // 【逐行批注】 按循环规则重复处理：for (int index = 0; index < 2; ++index)。
    for (int index = 0; index < 2; ++index)
        // 【逐行批注】 调用 check、to_string 完成本行所需操作：check(db.upsert(gangyi::ClassroomActivity{"classroom:no-ai:1:1:diagno…。
        check(db.upsert(gangyi::ClassroomActivity{"classroom:no-ai:1:1:diagnostic:" + std::to_string(index),
            // 【逐行批注】 执行当前声明或表达式："no-ai", "diagnostic", Json{{"status", "answered"}, {"correct", false}, {"credible",…。
            "no-ai", "diagnostic", Json{{"status", "answered"}, {"correct", false}, {"credible", true}}.dump(),
            // 【逐行批注】 执行当前声明或表达式："2026-09-28", 1, 1}), "调课证据应保存");。
            "2026-09-28", 1, 1}), "调课证据应保存");
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.insert(gangyi::LearningInteraction{"", "quiz",。
    check(db.insert(gangyi::LearningInteraction{"", "quiz",
        // 【逐行批注】 执行当前声明或表达式：Json{{"phaseIndex", 1}, {"topicIndex", 1}, {"score", 0}, {"total", 3}}.dump(), "2026…。
        Json{{"phaseIndex", 1}, {"topicIndex", 1}, {"score", 0}, {"total", 3}}.dump(), "2026-09-28",
        // 【逐行批注】 调用 string 完成本行所需操作：std::string("no-ai"), std::nullopt, std::nullopt}), "客观测验证据应保存");。
        std::string("no-ai"), std::nullopt, std::nullopt}), "客观测验证据应保存");
    // 【逐行批注】 计算右侧结果并写入 auto noAiFinish，更新当前运行状态。
    const auto noAiFinish = gangyi::finishClassroom(db, "no-ai", 1, 1, false, "2026-09-28");
    // 【逐行批注】 调用 check 完成本行所需操作：check(noAiFinish["pathAdjustment"] == "" && db.findSnapshotsByCourseI…。
    check(noAiFinish["pathAdjustment"] == "" && db.findSnapshotsByCourseId("no-ai").size() == 1,
          // 【逐行批注】 执行当前声明或表达式："AI 失败时不得调整路径或写入新快照");。
          "AI 失败时不得调整路径或写入新快照");

    // 【逐行批注】 计算右侧结果并写入 Json recoveryQuiz，更新当前运行状态。
    const Json recoveryQuiz = Json::array({
        // 【逐行批注】 调用 array 完成本行所需操作：{{"question", "一"}, {"options", Json::array({"对", "错", "其他", "不确定"})}…。
        {{"question", "一"}, {"options", Json::array({"对", "错", "其他", "不确定"})}, {"answerIndex", 0}},
        // 【逐行批注】 调用 array 完成本行所需操作：{{"question", "二"}, {"options", Json::array({"对", "错", "其他", "不确定"})}…。
        {{"question", "二"}, {"options", Json::array({"对", "错", "其他", "不确定"})}, {"answerIndex", 0}},
        // 【逐行批注】 调用 array 完成本行所需操作：{{"question", "三"}, {"options", Json::array({"对", "错", "其他", "不确定"})}…。
        {{"question", "三"}, {"options", Json::array({"对", "错", "其他", "不确定"})}, {"answerIndex", 0}}});
    // 【逐行批注】 调用 check、string 完成本行所需操作：check(db.insert(gangyi::LearningSession{"recover-session", std::strin…。
    check(db.insert(gangyi::LearningSession{"recover-session", std::string("recover"), std::string("anon"),
        // 【逐行批注】 调用 string 完成本行所需操作："目标", std::string("deep"), 1, "阶段", 1, "主题", "主题", std::nullopt, std:…。
        "目标", std::string("deep"), 1, "阶段", 1, "主题", "主题", std::nullopt, std::nullopt,
        // 【逐行批注】 执行当前声明或表达式：Json{{"blocks", {{"quiz", {{"quiz", recoveryQuiz}}}}}}.dump(), std::nullopt, 0, "ai"…。
        Json{{"blocks", {{"quiz", {{"quiz", recoveryQuiz}}}}}}.dump(), std::nullopt, 0, "ai"}),
        // 【逐行批注】 执行当前声明或表达式："复测课堂应保存");。
        "复测课堂应保存");
    // 【逐行批注】 调用 finishClassroom 完成本行所需操作：gangyi::finishClassroom(db, "recover", 1, 1, false, "2026-09-28");。
    gangyi::finishClassroom(db, "recover", 1, 1, false, "2026-09-28");
    // 【逐行批注】 计算右侧结果并写入 auto recovered，更新当前运行状态。
    const auto recovered = gangyi::submitReview(db, "recover", 1, 1, 1, Json::array({0, 0, 0}), "2026-09-29");
    // 【逐行批注】 调用 check 完成本行所需操作：check(recovered["passed"] == true && recovered["nextDue"] == "2026-09…。
    check(recovered["passed"] == true && recovered["nextDue"] == "2026-09-30" &&
        // 【逐行批注】 调用 dueReviews 完成本行所需操作：gangyi::dueReviews(db, "recover", "2026-10-06").size() == 3,。
        gangyi::dueReviews(db, "recover", "2026-10-06").size() == 3,
        // 【逐行批注】 执行当前声明或表达式："补弱复测通过后应从掌握日重新安排第1、3、7天复习");。
        "补弱复测通过后应从掌握日重新安排第1、3、7天复习");
    // 【逐行批注】 计算右侧结果并写入 auto recoveredDue，更新当前运行状态。
    const auto recoveredDue = gangyi::dueReviews(db, "recover", "2026-09-30");
    // 【逐行批注】 调用 check 完成本行所需操作：check(recoveredDue.size() == 1 && recoveredDue[0].contains("reviewId"…。
    check(recoveredDue.size() == 1 && recoveredDue[0].contains("reviewId"),
        // 【逐行批注】 执行当前声明或表达式："重新安排的复习应返回唯一任务标识");。
        "重新安排的复习应返回唯一任务标识");
    // 【逐行批注】 计算右侧结果并写入 auto repeated，更新当前运行状态。
    const auto repeated = gangyi::submitReview(db, "recover", 1, 1,
        // 【逐行批注】 调用 array 完成本行所需操作：recoveredDue[0].at("day").get<int>(), Json::array({0, 0, 0}), "2026-0…。
        recoveredDue[0].at("day").get<int>(), Json::array({0, 0, 0}), "2026-09-30",
        // 【逐行批注】 执行当前声明或表达式：recoveredDue[0].at("reviewId").get<std::string>());。
        recoveredDue[0].at("reviewId").get<std::string>());
    // 【逐行批注】 调用 check 完成本行所需操作：check(repeated["passed"] == true && repeated["nextDue"] == "",。
    check(repeated["passed"] == true && repeated["nextDue"] == "",
        // 【逐行批注】 执行当前声明或表达式："唯一任务标识应提交对应复习且不得重复建立复习周期");。
        "唯一任务标识应提交对应复习且不得重复建立复习周期");

    // 【逐行批注】 调用 defaultAvailability 完成本行所需操作：gangyi::WeeklyPlan plan{"course-1", Json{{"availability", gangyi::def…。
    gangyi::WeeklyPlan plan{"course-1", Json{{"availability", gangyi::defaultAvailability()},
        // 【逐行批注】 执行当前声明或表达式：{"entries", draft}, {"weekStart", "2026-09-28"}, {"version", 1}}.dump(), "2026-09-28…。
        {"entries", draft}, {"weekStart", "2026-09-28"}, {"version", 1}}.dump(), "2026-09-28", 1};
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.upsert(plan), "周计划应保存");。
    check(db.upsert(plan), "周计划应保存");
    // 【逐行批注】 计算右侧结果并写入 Json edited，更新当前运行状态。
    Json edited = draft;
    // 【逐行批注】 计算右侧结果并写入 edited[0]["minutes"]，更新当前运行状态。
    edited[0]["minutes"] = 25;
    // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
    const auto saved = gangyi::editWeeklyPlan(db, "course-1", {{"version", 1}, {"entries", edited}});
    // 【逐行批注】 调用 check 完成本行所需操作：check(saved["entries"][0]["manual"] == true && saved["entries"][1]["m…。
    check(saved["entries"][0]["manual"] == true && saved["entries"][1]["manual"] == false,
        // 【逐行批注】 执行当前声明或表达式："仅被修改的安排应标记为手动");。
        "仅被修改的安排应标记为手动");
    // 【逐行批注】 计算右侧结果并写入 auto proposed，更新当前运行状态。
    const auto proposed = gangyi::replanWeeklyPlan(db, "course-1", Json::array({"A", "B", "C"}), "2026-09-28", false);
    // 【逐行批注】 调用 check 完成本行所需操作：check(proposed.value("requiresConfirmation", false), "覆盖手动安排前必须确认");。
    check(proposed.value("requiresConfirmation", false), "覆盖手动安排前必须确认");
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.getWeeklyPlan("course-1")->version == 2, "未确认的重排不得写入");。
    check(db.getWeeklyPlan("course-1")->version == 2, "未确认的重排不得写入");
    // 【逐行批注】 计算右侧结果并写入 auto nextWeek，更新当前运行状态。
    const auto nextWeek = gangyi::getWeeklyPlan(db, "course-1", Json::array({"A", "B", "C"}), "2026-10-05");
    // 【逐行批注】 调用 check 完成本行所需操作：check(nextWeek["weekStart"] == "2026-10-05" && nextWeek["version"] ==…。
    check(nextWeek["weekStart"] == "2026-10-05" && nextWeek["version"] == 3 &&
        // 【逐行批注】 调用 defaultAvailability 完成本行所需操作：nextWeek["availability"] == gangyi::defaultAvailability(), "跨周应重新排课并保…。
        nextWeek["availability"] == gangyi::defaultAvailability(), "跨周应重新排课并保留可用时间");
    // 【逐行批注】 计算右侧结果并写入 Json indexed，更新当前运行状态。
    const Json indexed = Json::array({{{"title", "A"}, {"phaseIndex", 1}, {"topicIndex", 1}},
        // 【逐行批注】 执行当前声明或表达式：{{"title", "B"}, {"phaseIndex", 1}, {"topicIndex", 2}},。
        {{"title", "B"}, {"phaseIndex", 1}, {"topicIndex", 2}},
        // 【逐行批注】 执行当前声明或表达式：{{"title", "C"}, {"phaseIndex", 1}, {"topicIndex", 3}}});。
        {{"title", "C"}, {"phaseIndex", 1}, {"topicIndex", 3}}});
    // 【逐行批注】 调用 check、defaultAvailability 完成本行所需操作：check(db.upsert(gangyi::WeeklyPlan{"course-1", Json{{"availability", …。
    check(db.upsert(gangyi::WeeklyPlan{"course-1", Json{{"availability", gangyi::defaultAvailability()},
        // 【逐行批注】 执行当前声明或表达式：{"entries", draft}, {"weekStart", "2026-09-28"}, {"version", 20}}.dump(), "2026-09-2…。
        {"entries", draft}, {"weekStart", "2026-09-28"}, {"version", 20}}.dump(), "2026-09-28", 20}), "旧课表应保存");
    // 【逐行批注】 计算右侧结果并写入 auto weekend，更新当前运行状态。
    const auto weekend = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        // 【逐行批注】 执行当前声明或表达式：{{"version", 20}}, "2026-10-03");。
        {{"version", 20}}, "2026-10-03");
    // 【逐行批注】 调用 check 完成本行所需操作：check(weekend["plan"]["weekStart"] == "2026-10-05" && weekend["change…。
    check(weekend["plan"]["weekStart"] == "2026-10-05" && weekend["changed"] == true &&
        // 【逐行批注】 执行当前声明或表达式：weekend["source"] == "rules", "周末应顺延下周，AI 失败仍按规则排课");。
        weekend["source"] == "rules", "周末应顺延下周，AI 失败仍按规则排课");
    // 【逐行批注】 按循环规则重复处理：for (const auto& entry : weekend["plan"]["entries"]) check(entry["date"].g…。
    for (const auto& entry : weekend["plan"]["entries"]) check(entry["date"].get<std::string>() >= "2026-10-03", "重排不得安排到过去");
    // 【逐行批注】 计算右侧结果并写入 auto preserved，更新当前运行状态。
    const auto preserved = gangyi::getWeeklyPlan(db, "course-1", indexed, "2026-09-28", "2026-10-03");
    // 【逐行批注】 调用 check 完成本行所需操作：check(preserved == weekend["plan"], "本周读取必须保留顺延结果");。
    check(preserved == weekend["plan"], "本周读取必须保留顺延结果");
    // 【逐行批注】 计算右侧结果并写入 auto same，更新当前运行状态。
    const auto same = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        // 【逐行批注】 执行当前声明或表达式：{{"version", 21}}, "2026-10-03");。
        {{"version", 21}}, "2026-10-03");
    // 【逐行批注】 调用 check 完成本行所需操作：check(same["changed"] == false && same["plan"]["version"] == 21, "无需变…。
    check(same["changed"] == false && same["plan"]["version"] == 21, "无需变化时不得增加版本");
    // 【逐行批注】 计算右侧结果并写入 auto saturday，更新当前运行状态。
    const auto saturday = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        // 【逐行批注】 调用 array 完成本行所需操作：{{"version", 21}, {"availability", Json::array({{{"weekday", 6}, {"mi…。
        {{"version", 21}, {"availability", Json::array({{{"weekday", 6}, {"minutes", 45}}})}}, "2026-10-03");
    // 【逐行批注】 调用 check 完成本行所需操作：check(saturday["plan"]["entries"][0]["date"] == "2026-10-03" &&。
    check(saturday["plan"]["entries"][0]["date"] == "2026-10-03" &&
        // 【逐行批注】 执行当前声明或表达式：saturday["plan"]["availability"][0]["minutes"] == 45, "应使用页面未保存的可用时间");。
        saturday["plan"]["availability"][0]["minutes"] == 45, "应使用页面未保存的可用时间");
    // 【逐行批注】 调用 check 完成本行所需操作：check(db.insert(gangyi::LearningInteraction{"", "quiz", Json{{"phaseI…。
    check(db.insert(gangyi::LearningInteraction{"", "quiz", Json{{"phaseIndex", 1}, {"topicIndex", 2},
        // 【逐行批注】 调用 array 完成本行所需操作：{"topic", "B"}, {"score", 0}, {"total", 3}, {"results", Json::array({…。
        {"topic", "B"}, {"score", 0}, {"total", 3}, {"results", Json::array({{{"answered", true}, {"unknown", true}}})}}.dump(),
        // 【逐行批注】 调用 string 完成本行所需操作："2026-10-03T12:00:00Z", std::string("course-1"), std::nullopt, std::n…。
        "2026-10-03T12:00:00Z", std::string("course-1"), std::nullopt, std::nullopt}), "明确不会的证据应保存");
    // 【逐行批注】 计算右侧结果并写入 auto weakFirst，更新当前运行状态。
    const auto weakFirst = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        // 【逐行批注】 调用 defaultAvailability 完成本行所需操作：{{"version", 22}, {"availability", gangyi::defaultAvailability()}}, "…。
        {{"version", 22}, {"availability", gangyi::defaultAvailability()}}, "2026-09-29");
    // 【逐行批注】 计算右侧结果并写入 auto firstLesson，更新当前运行状态。
    const auto firstLesson = std::find_if(weakFirst["plan"]["entries"].begin(), weakFirst["plan"]["entries"].end(),
        // 【逐行批注】 执行当前声明或表达式：[](const Json& entry) { return entry.value("kind", "") == "lesson"; });。
        [](const Json& entry) { return entry.value("kind", "") == "lesson"; });
    // 【逐行批注】 调用 check 完成本行所需操作：check(firstLesson != weakFirst["plan"]["entries"].end() && (*firstLes…。
    check(firstLesson != weakFirst["plan"]["entries"].end() && (*firstLesson)["title"] == "B" && (*firstLesson)["date"] == "2026-09-30",
        // 【逐行批注】 执行当前声明或表达式："规则应优先安排薄弱课时，跳过已过去的周一");。
        "规则应优先安排薄弱课时，跳过已过去的周一");
    // 【逐行批注】 计算右侧结果并写入 Json manual，更新当前运行状态。
    Json manual = weakFirst["plan"]["entries"]; manual[0]["minutes"] = 17;
    // 【逐行批注】 计算右侧结果并写入 auto preview，更新当前运行状态。
    const auto preview = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        // 【逐行批注】 执行当前声明或表达式：{{"version", 23}, {"entries", manual}}, "2026-09-29");。
        {{"version", 23}, {"entries", manual}}, "2026-09-29");
    // 【逐行批注】 调用 check 完成本行所需操作：check(preview["requiresConfirmation"] == true && db.getWeeklyPlan("co…。
    check(preview["requiresConfirmation"] == true && db.getWeeklyPlan("course-1")->version == 23,
        // 【逐行批注】 执行当前声明或表达式："未保存手动修改也必须预览，预览不写数据库");。
        "未保存手动修改也必须预览，预览不写数据库");
    // 【逐行批注】 计算右侧结果并写入 auto accepted，更新当前运行状态。
    const auto accepted = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", true,
        // 【逐行批注】 执行当前声明或表达式：{{"version", 23}, {"proposalId", preview["proposalId"]}}, "2026-09-29");。
        {{"version", 23}, {"proposalId", preview["proposalId"]}}, "2026-09-29");
    // 【逐行批注】 调用 check 完成本行所需操作：check(accepted["plan"]["entries"] == preview["proposed"] && accepted[…。
    check(accepted["plan"]["entries"] == preview["proposed"] && accepted["plan"]["version"] == 24,
        // 【逐行批注】 执行当前声明或表达式："确认应应用相同预览并只增加一次版本");。
        "确认应应用相同预览并只增加一次版本");
    // 【逐行批注】 计算右侧结果并写入 auto stale，更新当前运行状态。
    const auto stale = gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", false,
        // 【逐行批注】 执行当前声明或表达式：{{"version", 24}, {"entries", manual}}, "2026-09-29");。
        {{"version", 24}, {"entries", manual}}, "2026-09-29");
    // 【逐行批注】 调用 editWeeklyPlan 完成本行所需操作：gangyi::editWeeklyPlan(db, "course-1", {{"version", 24}, {"entries", …。
    gangyi::editWeeklyPlan(db, "course-1", {{"version", 24}, {"entries", accepted["plan"]["entries"]}});
    // 【逐行批注】 计算右侧结果并写入 bool conflict，更新当前运行状态。
    bool conflict = false;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { gangyi::replanWeeklyPlan(db, "course-1", indexed, "2026-09-28", true,
        // 【逐行批注】 执行当前声明或表达式：{{"version", 24}, {"proposalId", stale["proposalId"]}}, "2026-09-29"); }。
        {{"version", 24}, {"proposalId", stale["proposalId"]}}, "2026-09-29"); }
    // 【逐行批注】 捕获并处理异常：catch (const std::invalid_argument&) { conflict = true; }。
    catch (const std::invalid_argument&) { conflict = true; }
    // 【逐行批注】 调用 check 完成本行所需操作：check(conflict && db.getWeeklyPlan("course-1")->version == 25, "其他窗口修…。
    check(conflict && db.getWeeklyPlan("course-1")->version == 25, "其他窗口修改后必须拒绝旧确认");
    // 【逐行批注】 按循环规则重复处理：for (auto row : db.listClassroomActivities("course-1")) if (row.kind == "r…。
    for (auto row : db.listClassroomActivities("course-1")) if (row.kind == "review") {
        // 【逐行批注】 计算右侧结果并写入 Json value，更新当前运行状态。
        Json value = Json::parse(row.payload); value["status"] = "passed"; row.payload = value.dump(); db.upsert(row);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto empty，更新当前运行状态。
    const auto empty = gangyi::replanWeeklyPlan(db, "course-1", Json::array(), "2026-09-28", false,
        // 【逐行批注】 执行当前声明或表达式：{{"version", 25}}, "2026-09-29");。
        {{"version", 25}}, "2026-09-29");
    // 【逐行批注】 调用 check 完成本行所需操作：check(empty["changed"] == false && empty["plan"]["version"] == 25, "没…。
    check(empty["changed"] == false && empty["plan"]["version"] == 25, "没有待安排内容时应给出明确结果");
    // 【逐行批注】 执行当前声明或表达式：std::cout << (failures ? "课堂规则测试失败" : "课堂规则测试通过") << '\n';。
    std::cout << (failures ? "课堂规则测试失败" : "课堂规则测试通过") << '\n';
    // 【逐行批注】 结束当前函数并返回结果：return failures ? 1 : 0;。
    return failures ? 1 : 0;
// 【逐行批注】 结束当前代码块。
}

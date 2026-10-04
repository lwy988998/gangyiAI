#include "classroom_service.hpp"
#include "learning_flow.hpp"
#include <iostream>

int main() {
    using gangyi::Json;
    int failures = 0;
    const auto check = [&](bool ok, const char* message) {
        if (!ok) { std::cerr << "失败：" << message << '\n'; ++failures; }
    };
    const Json secret = {{"blocks", {{"examples", {{"examples", Json::array({{{"title", "例题"}, {"content", "求值"}, {"solution", "隐藏解答"}, {"internal", "私密"}}})}}},
        {"practice", {{"practice", Json::array({{{"task", "证明"}, {"check", "隐藏标准"}}})}}},
        {"quiz", {{"quiz", Json::array({{{"question", "选择"}, {"options", {"甲", "乙"}}, {"answerIndex", 1}, {"explanation", "隐藏解析"}, {"rubric", "隐藏评分"}}})}}}}},
        {"generations", {{"quiz", {{"model", "真实模型"}, {"prompt", "内部提示"}, {"raw", "隐藏原文"}}}}}};
    const auto visible = gangyi::publicLearningContent(secret).dump();
    check(visible.find("隐藏") == std::string::npos && visible.find("内部提示") == std::string::npos,
          "整个课堂响应必须隐藏答案与生成内部字段");
    check(visible.find("真实模型") != std::string::npos, "公开内容保留模型来源");
    const Json legacy = {{"roadmap", Json::array({{{"name", "函数阶段"}, {"topics", {"函数"}}}})},
        {"courseStructure", Json::array({{{"stage", "函数阶段"}, {"topics", {"函数"}}, {"milestones", {"保持课纲"}}}})},
        {"phaseExpansions", {{"1", {{"content", {{"lessons", Json::array({{{"title", "函数课堂"}, {"quiz", Json::array({
            {{"question", "旧选择题"}, {"options", {"甲", "乙"}}, {"answerIndex", 1}, {"explanation", "隐藏旧解析"},
              {"markingGrid", "隐藏评分结构"}}})}, {"practice", Json::array({{{"task", "旧练习"}, {"check", "隐藏标准"}}})},
              {"examples", Json::array({{{"content", "旧例题"}, {"solution", "隐藏解答"}}})},
              {"lessonSteps", Json::array({{{"explanation", "公开旧讲解"}}})}}})}}},
            {"generation", {{"source", "ai"}, {"model", "真实旧模型"}, {"promptVersion", "ai-phase-v1"},
              {"raw", "隐藏模型原文"}, {"prompt", "隐藏内部提示"}}}}}}}};
    const auto publicLegacy = gangyi::publicCoursePayload(legacy);
    check(publicLegacy.dump().find("隐藏") == std::string::npos, "旧课程快照和阶段扩展的公开内容不包含标准与内部评分");
    check(publicLegacy["courseStructure"] == legacy["courseStructure"] && publicLegacy["roadmap"] == legacy["roadmap"] &&
        publicLegacy["phaseExpansions"].contains("1") && publicLegacy.dump().find("公开旧讲解") != std::string::npos,
        "公开旧课程保留课纲动态阶段键和必要讲解");
    check(publicLegacy["phaseExpansions"]["1"]["generation"]["model"] == "真实旧模型" &&
        legacy.dump().find("隐藏解答") != std::string::npos, "公开投影保留真实来源且不修改私有课程标准");
    const auto prose = gangyi::publicLearningContent(Json{{"blocks", {{"overview", {{"inferredDomain", "数学"}, {"keyConcepts", {"函数"}}}},
        {"steps", {{"lessonSteps", Json::array({{{"title", "概念"}, {"explanation", "公开讲解"}, {"example", "公开示意"}, {"action", "观察条件"}, {"check", "内部标准"}}})}}},
        {"assessment", {{"checkpoint", {"回顾"}}, {"resourceSummary", "资源说明"}}}}}}).dump();
    check(prose.find("公开讲解") != std::string::npos && prose.find("资源说明") != std::string::npos && prose.find("内部标准") == std::string::npos,
        "公开讲解与总结不被答案过滤误删");
    gangyi::Database db; db.open(":memory:"); db.migrate();
    db.insert(gangyi::Course{"q-course", {}, {}, "函数目标", "deep", "函数课程", {}, "ai", "active", "2026-10-04", "2026-10-04"});
    auto content = secret; content["contentVersion"] = 7;
    db.insert(gangyi::LearningSession{"q-session", std::string("q-course"), {}, "函数目标", std::string("deep"),
        1, "阶段", 1, "函数", "函数", {}, {}, content.dump(), {}, 0, "ai"});
    Json body = {{"courseId", "q-course"}, {"phaseIndex", 1}, {"topicIndex", 1}};
    auto questions = gangyi::classroomQuestions(db, body);
    check(questions["questions"].size() == 3, "例题练习测验均应进入同一题目接口");
    check(questions.dump().find("隐藏") == std::string::npos, "题目接口作答前不泄露标准答案");
    auto q = questions["questions"][0];
    body["kind"] = q["kind"]; body["index"] = q["index"]; body["questionId"] = q["questionId"];
    body["contentVersion"] = q["contentVersion"]; body["version"] = 0; body["requestId"] = "request-1";
    body["question"] = "我得到 2，因为代入后得出。";
    auto turn = gangyi::beginDialogue(db, body);
    check(turn["thread"]["turns"].back()["status"] == "pending", "输入先保存为待评价状态");
    check(gangyi::beginDialogue(db, body)["pending"] == true, "重复请求复用在途输入，不能重复请求真实 AI");
    auto conflict = body; conflict["requestId"] = "another-window";
    bool rejected = false; try { gangyi::beginDialogue(db, conflict); } catch (...) { rejected = true; }
    check(rejected, "另一个窗口不能覆盖正在评价的作答");
    turn["evaluation"] = {{"isAnswer", true}, {"correct", true}, {"unknown", false}, {"confidence", 0.95}, {"feedback", "代入方法合理且答案正确。"}};
    turn["evaluationModel"] = "评价模型";
    const auto options = gangyi::dialogueOptions(db, body, turn);
    check(options.messages[0].content.find("先用通俗中文解释一小点") == std::string::npos &&
        options.messages[0].content.find("已完成真实评价") != std::string::npos, "讲解使用完成评价并自主选择教法");
    auto finished = gangyi::finishDialogue(db, body, turn, "你的代入过程正确。", "讲解模型");
    check(finished["evaluationStatus"] == "ready", "评价与连续讲解必须一次性完成提交");
    auto saved = gangyi::dialogueView(db, body);
    check(saved["credible"] == true && saved["question"]["status"] == "answered", "可靠评价与问题状态同步保存");
    body["requestId"] = "request-2"; body["version"] = saved["version"];
    body["question"] = "我再算一下。";
    auto failed = gangyi::beginDialogue(db, body);
    failed["evaluation"] = {{"isAnswer", true}, {"correct", false}, {"unknown", false}, {"confidence", 0.95}, {"feedback", "此次答案存在计算错误。"}};
    failed["evaluationModel"] = "评价模型";
    gangyi::finishDialogue(db, body, failed, "截断文本", "讲解模型", "length");
    check(gangyi::dialogueView(db, body)["correct"] == true, "截断讲解不得覆盖原有可靠评价");
    body["requestId"] = "request-3"; body["version"] = gangyi::dialogueView(db, body)["version"];
    body["contentVersion"] = 6;
    bool stale = false; try { gangyi::beginDialogue(db, body); } catch (...) { stale = true; }
    check(stale, "内容版本变化必须拒绝旧窗口作答");
    auto changedSession = *db.findLearningSession("q-course", 1, 1);
    auto otherBlockChanged = content; otherBlockChanged["contentVersion"] = 8;
    otherBlockChanged["blocks"]["overview"] = {{"title", "只更新其他板块"}};
    changedSession.content = otherBlockChanged.dump(); db.update(changedSession);
    check(gangyi::classroomQuestion(db, body)["contentVersion"] == 7, "只更新其他板块时题目版本保持稳定且能恢复作答");
    body["contentVersion"] = 7; body["requestId"] = "request-4"; body["question"] = "能介绍另一种方法吗？";
    const auto followup = gangyi::beginDialogue(db, body);
    check(followup["assisted"] == true, "看过讲解后的新作答必须标记受帮助");
    auto ask = followup;
    ask["evaluation"] = {{"isAnswer", false}, {"correct", false}, {"unknown", false}, {"confidence", 0.9}, {"feedback", "学生请求其他解法，并未重新作答。"}};
    ask["evaluationModel"] = "评价模型";
    gangyi::finishDialogue(db, body, ask, "此题也可以换元。", "讲解模型");
    check(gangyi::dialogueView(db, body)["correct"] == true, "纯追问不能覆盖原题成绩");
    check(gangyi::beginDialogue(db, body)["cached"] == true, "同一已完成requestId复用对话而不重复AI");
    auto partialBody = Json{{"courseId", "q-course"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"kind", "practice"},
        {"index", 0}, {"requestId", "partial-stop"}, {"question", "我先试着证明。"}, {"version", 0}};
    const auto partialQuestion = gangyi::classroomQuestion(db, partialBody);
    partialBody["questionId"] = partialQuestion["questionId"]; partialBody["contentVersion"] = partialQuestion["contentVersion"];
    auto partialTurn = gangyi::beginDialogue(db, partialBody);
    check(partialTurn["assisted"] == false, "首次作答没有看过帮助时保持独立状态");
    check(gangyi::recordDialogueAssistance(db, partialTurn, "先观察函数定义中的条件。"), "真实讲解分片保存独立展示记录");
    gangyi::finishDialogue(db, partialBody, partialTurn, "", "", "cancelled");
    const auto partialView = gangyi::dialogueView(db, partialBody);
    check(partialView["turns"].back()["partialAssistant"] == "先观察函数定义中的条件。" &&
        partialView["turns"].back()["incomplete"] == true, "停止后刷新保留已看过的真实片段并标注未完成");
    check(!partialView.value("credible", false) && partialView["question"]["status"] == "pending",
        "只有部分讲解时不产生可靠评价");
    check(!gangyi::recordDialogueAssistance(db, partialTurn, "已经过期的分片"), "停止后旧流不能继续写入展示记录");
    partialBody["version"] = partialView["version"];
    auto partialRetry = gangyi::beginDialogue(db, partialBody);
    check(partialRetry["assisted"] == true, "看过部分讲解后重试必须记录为受帮助作答");
    check(gangyi::dialogueView(db, partialBody)["turns"].back()["previousPartialReplies"].size() == 1,
        "同一请求重试保留此前已经展示过的片段");
    gangyi::finishDialogue(db, partialBody, partialRetry, "", "", "cancelled");
    bool badChunkRejected = false;
    try { badChunkRejected = !gangyi::recordDialogueAssistance(db, Json::object(), "无效请求"); } catch (...) {}
    check(badChunkRejected, "流式回调的无效数据返回失败且不向 C 回调抛出异常");
    const Json prepared = {{"lessonTaskId", "isolated-task"}, {"courseId", "q-course"}, {"phaseIndex", 1}, {"topicIndex", 1},
        {"kind", "consolidation"}, {"status", "ready"}, {"content", content}, {"exposure", {{"blocks", Json::array()}}},
        {"progress", {{"status", "not_started"}}}};
    db.upsert(gangyi::ClassroomActivity{"prepared-lesson:isolated-task", "q-course", "prepared-lesson", prepared.dump(), "2026-10-04", 1, 1});
    auto independent = Json{{"courseId", "q-course"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"lessonTaskId", "isolated-task"}};
    const auto consolidation = gangyi::classroomQuestions(db, independent);
    check(consolidation["questions"][0]["status"] == "pending", "巩固课作答状态独立于原课堂");
    check(consolidation["questions"][0]["questionId"] == q["questionId"], "同一道原题在巩固课中仍只算一道证据");
    db.upsert(gangyi::ClassroomActivity{"classroom:q-course:1:1:review:1", "q-course", "review",
        Json{{"title", "函数复习"}, {"due", "2026-10-04"}, {"day", 1}, {"status", "pending"}}.dump(), "2026-10-04", 1, 1});
    auto reviewBody = Json{{"courseId", "q-course"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"kind", "review"}, {"day", 1}, {"today", "2026-10-04"}};
    const auto reviewQuestions = gangyi::classroomQuestions(db, reviewBody);
    check(reviewQuestions["questions"].size() == 1, "复习题接入同一公开接口");
    check(gangyi::dueReviews(db, "q-course", "2026-10-04").size() == 1, "复习逐题活动不冒充待办复习任务");
    const auto noEvidence = gangyi::completeReviewFromEvaluations(db, reviewBody, Json::array({{{"answered", false}, {"credible", false}}}));
    check(noEvidence["total"] == 0 && noEvidence["evaluationStatus"] == "insufficient", "漏答不算不会且不自动完成复习");
    return failures ? 1 : 0;
}

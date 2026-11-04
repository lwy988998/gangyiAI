// 【逐行批注】 引入头文件 "classroom_service.hpp"，为后续实现提供类型或接口声明。
#include "classroom_service.hpp"
// 【逐行批注】 引入头文件 "learning_flow.hpp"，为后续实现提供类型或接口声明。
#include "learning_flow.hpp"
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
    // 【逐行批注】 执行当前声明或表达式：using gangyi::Json;。
    using gangyi::Json;
    // 【逐行批注】 计算右侧结果并写入 int failures，更新当前运行状态。
    int failures = 0;
    // 【逐行批注】 计算右侧结果并写入 auto check，更新当前运行状态。
    const auto check = [&](bool ok, const char* message) {
        // 【逐行批注】 判断条件 (!ok) { std::cerr << "失败：" << message << '\n'; ++failures; }，成立时执行对应分支。
        if (!ok) { std::cerr << "失败：" << message << '\n'; ++failures; }
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 Json secret，更新当前运行状态。
    const Json secret = {{"blocks", {{"examples", {{"examples", Json::array({{{"title", "例题"}, {"content", "求值"}, {"solution", "隐藏解答"}, {"internal", "私密"}}})}}},
        // 【逐行批注】 调用 array 完成本行所需操作：{"practice", {{"practice", Json::array({{{"task", "证明"}, {"check", "隐…。
        {"practice", {{"practice", Json::array({{{"task", "证明"}, {"check", "隐藏标准"}}})}}},
        // 【逐行批注】 调用 array 完成本行所需操作：{"quiz", {{"quiz", Json::array({{{"question", "选择"}, {"options", {"甲"…。
        {"quiz", {{"quiz", Json::array({{{"question", "选择"}, {"options", {"甲", "乙"}}, {"answerIndex", 1}, {"explanation", "隐藏解析"}, {"rubric", "隐藏评分"}}})}}}}},
        // 【逐行批注】 执行当前声明或表达式：{"generations", {{"quiz", {{"model", "真实模型"}, {"prompt", "内部提示"}, {"raw", "隐藏原文"}}}}…。
        {"generations", {{"quiz", {{"model", "真实模型"}, {"prompt", "内部提示"}, {"raw", "隐藏原文"}}}}}};
    // 【逐行批注】 计算右侧结果并写入 auto visible，更新当前运行状态。
    const auto visible = gangyi::publicLearningContent(secret).dump();
    // 【逐行批注】 调用 check 完成本行所需操作：check(visible.find("隐藏") == std::string::npos && visible.find("内部提示")…。
    check(visible.find("隐藏") == std::string::npos && visible.find("内部提示") == std::string::npos,
          // 【逐行批注】 执行当前声明或表达式："整个课堂响应必须隐藏答案与生成内部字段");。
          "整个课堂响应必须隐藏答案与生成内部字段");
    // 【逐行批注】 调用 check 完成本行所需操作：check(visible.find("真实模型") != std::string::npos, "公开内容保留模型来源");。
    check(visible.find("真实模型") != std::string::npos, "公开内容保留模型来源");
    // 【逐行批注】 计算右侧结果并写入 Json legacy，更新当前运行状态。
    const Json legacy = {{"roadmap", Json::array({{{"name", "函数阶段"}, {"topics", {"函数"}}}})},
        // 【逐行批注】 调用 array 完成本行所需操作：{"courseStructure", Json::array({{{"stage", "函数阶段"}, {"topics", {"函数"…。
        {"courseStructure", Json::array({{{"stage", "函数阶段"}, {"topics", {"函数"}}, {"milestones", {"保持课纲"}}}})},
        // 【逐行批注】 调用 array 完成本行所需操作：{"phaseExpansions", {{"1", {{"content", {{"lessons", Json::array({{{"…。
        {"phaseExpansions", {{"1", {{"content", {{"lessons", Json::array({{{"title", "函数课堂"}, {"quiz", Json::array({
            // 【逐行批注】 执行当前声明或表达式：{{"question", "旧选择题"}, {"options", {"甲", "乙"}}, {"answerIndex", 1}, {"explanation", …。
            {{"question", "旧选择题"}, {"options", {"甲", "乙"}}, {"answerIndex", 1}, {"explanation", "隐藏旧解析"},
              // 【逐行批注】 调用 array 完成本行所需操作：{"markingGrid", "隐藏评分结构"}}})}, {"practice", Json::array({{{"task", "旧…。
              {"markingGrid", "隐藏评分结构"}}})}, {"practice", Json::array({{{"task", "旧练习"}, {"check", "隐藏标准"}}})},
              // 【逐行批注】 调用 array 完成本行所需操作：{"examples", Json::array({{{"content", "旧例题"}, {"solution", "隐藏解答"}}}…。
              {"examples", Json::array({{{"content", "旧例题"}, {"solution", "隐藏解答"}}})},
              // 【逐行批注】 调用 array 完成本行所需操作：{"lessonSteps", Json::array({{{"explanation", "公开旧讲解"}}})}}})}}},。
              {"lessonSteps", Json::array({{{"explanation", "公开旧讲解"}}})}}})}}},
            // 【逐行批注】 执行当前声明或表达式：{"generation", {{"source", "ai"}, {"model", "真实旧模型"}, {"promptVersion", "ai-phase-v1…。
            {"generation", {{"source", "ai"}, {"model", "真实旧模型"}, {"promptVersion", "ai-phase-v1"},
              // 【逐行批注】 执行当前声明或表达式：{"raw", "隐藏模型原文"}, {"prompt", "隐藏内部提示"}}}}}}}};。
              {"raw", "隐藏模型原文"}, {"prompt", "隐藏内部提示"}}}}}}}};
    // 【逐行批注】 计算右侧结果并写入 auto publicLegacy，更新当前运行状态。
    const auto publicLegacy = gangyi::publicCoursePayload(legacy);
    // 【逐行批注】 调用 check 完成本行所需操作：check(publicLegacy.dump().find("隐藏") == std::string::npos, "旧课程快照和阶段扩…。
    check(publicLegacy.dump().find("隐藏") == std::string::npos, "旧课程快照和阶段扩展的公开内容不包含标准与内部评分");
    // 【逐行批注】 调用 check 完成本行所需操作：check(publicLegacy["courseStructure"] == legacy["courseStructure"] &&…。
    check(publicLegacy["courseStructure"] == legacy["courseStructure"] && publicLegacy["roadmap"] == legacy["roadmap"] &&
        // 【逐行批注】 执行当前声明或表达式：publicLegacy["phaseExpansions"].contains("1") && publicLegacy.dump().find("公开旧讲解") !…。
        publicLegacy["phaseExpansions"].contains("1") && publicLegacy.dump().find("公开旧讲解") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："公开旧课程保留课纲动态阶段键和必要讲解");。
        "公开旧课程保留课纲动态阶段键和必要讲解");
    // 【逐行批注】 调用 check 完成本行所需操作：check(publicLegacy["phaseExpansions"]["1"]["generation"]["model"] == …。
    check(publicLegacy["phaseExpansions"]["1"]["generation"]["model"] == "真实旧模型" &&
        // 【逐行批注】 执行当前声明或表达式：legacy.dump().find("隐藏解答") != std::string::npos, "公开投影保留真实来源且不修改私有课程标准");。
        legacy.dump().find("隐藏解答") != std::string::npos, "公开投影保留真实来源且不修改私有课程标准");
    // 【逐行批注】 计算右侧结果并写入 auto prose，更新当前运行状态。
    const auto prose = gangyi::publicLearningContent(Json{{"blocks", {{"overview", {{"inferredDomain", "数学"}, {"keyConcepts", {"函数"}}}},
        // 【逐行批注】 调用 array 完成本行所需操作：{"steps", {{"lessonSteps", Json::array({{{"title", "概念"}, {"explanati…。
        {"steps", {{"lessonSteps", Json::array({{{"title", "概念"}, {"explanation", "公开讲解"}, {"example", "公开示意"}, {"action", "观察条件"}, {"check", "内部标准"}}})}}},
        // 【逐行批注】 执行当前声明或表达式：{"assessment", {{"checkpoint", {"回顾"}}, {"resourceSummary", "资源说明"}}}}}}).dump();。
        {"assessment", {{"checkpoint", {"回顾"}}, {"resourceSummary", "资源说明"}}}}}}).dump();
    // 【逐行批注】 调用 check 完成本行所需操作：check(prose.find("公开讲解") != std::string::npos && prose.find("资源说明") !…。
    check(prose.find("公开讲解") != std::string::npos && prose.find("资源说明") != std::string::npos && prose.find("内部标准") == std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："公开讲解与总结不被答案过滤误删");。
        "公开讲解与总结不被答案过滤误删");
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(":memory:"); db.migrate();。
    gangyi::Database db; db.open(":memory:"); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：db.insert(gangyi::Course{"q-course", {}, {}, "函数目标", "deep", "函数课程", {}, "ai", "acti…。
    db.insert(gangyi::Course{"q-course", {}, {}, "函数目标", "deep", "函数课程", {}, "ai", "active", "2026-10-04", "2026-10-04"});
    // 【逐行批注】 计算右侧结果并写入 content，更新当前运行状态。
    auto content = secret; content["contentVersion"] = 7;
    // 【逐行批注】 调用 string 完成本行所需操作：db.insert(gangyi::LearningSession{"q-session", std::string("q-course"…。
    db.insert(gangyi::LearningSession{"q-session", std::string("q-course"), {}, "函数目标", std::string("deep"),
        // 【逐行批注】 执行当前声明或表达式：1, "阶段", 1, "函数", "函数", {}, {}, content.dump(), {}, 0, "ai"});。
        1, "阶段", 1, "函数", "函数", {}, {}, content.dump(), {}, 0, "ai"});
    // 【逐行批注】 计算右侧结果并写入 Json body，更新当前运行状态。
    Json body = {{"courseId", "q-course"}, {"phaseIndex", 1}, {"topicIndex", 1}};
    // 【逐行批注】 计算右侧结果并写入 questions，更新当前运行状态。
    auto questions = gangyi::classroomQuestions(db, body);
    // 【逐行批注】 调用 check 完成本行所需操作：check(questions["questions"].size() == 3, "例题练习测验均应进入同一题目接口");。
    check(questions["questions"].size() == 3, "例题练习测验均应进入同一题目接口");
    // 【逐行批注】 调用 check 完成本行所需操作：check(questions.dump().find("隐藏") == std::string::npos, "题目接口作答前不泄露标准…。
    check(questions.dump().find("隐藏") == std::string::npos, "题目接口作答前不泄露标准答案");
    // 【逐行批注】 计算右侧结果并写入 q，更新当前运行状态。
    auto q = questions["questions"][0];
    // 【逐行批注】 计算右侧结果并写入 body["kind"]，更新当前运行状态。
    body["kind"] = q["kind"]; body["index"] = q["index"]; body["questionId"] = q["questionId"];
    // 【逐行批注】 计算右侧结果并写入 body["contentVersion"]，更新当前运行状态。
    body["contentVersion"] = q["contentVersion"]; body["version"] = 0; body["requestId"] = "request-1";
    // 【逐行批注】 计算右侧结果并写入 body["question"]，更新当前运行状态。
    body["question"] = "我得到 2，因为代入后得出。";
    // 【逐行批注】 计算右侧结果并写入 turn，更新当前运行状态。
    auto turn = gangyi::beginDialogue(db, body);
    // 【逐行批注】 调用 check 完成本行所需操作：check(turn["thread"]["turns"].back()["status"] == "pending", "输入先保存为待…。
    check(turn["thread"]["turns"].back()["status"] == "pending", "输入先保存为待评价状态");
    // 【逐行批注】 调用 check、beginDialogue 完成本行所需操作：check(gangyi::beginDialogue(db, body)["pending"] == true, "重复请求复用在途输入…。
    check(gangyi::beginDialogue(db, body)["pending"] == true, "重复请求复用在途输入，不能重复请求真实 AI");
    // 【逐行批注】 计算右侧结果并写入 conflict，更新当前运行状态。
    auto conflict = body; conflict["requestId"] = "another-window";
    // 【逐行批注】 计算右侧结果并写入 bool rejected，更新当前运行状态。
    bool rejected = false; try { gangyi::beginDialogue(db, conflict); } catch (...) { rejected = true; }
    // 【逐行批注】 调用 check 完成本行所需操作：check(rejected, "另一个窗口不能覆盖正在评价的作答");。
    check(rejected, "另一个窗口不能覆盖正在评价的作答");
    // 【逐行批注】 计算右侧结果并写入 turn["evaluation"]，更新当前运行状态。
    turn["evaluation"] = {{"isAnswer", true}, {"correct", true}, {"unknown", false}, {"confidence", 0.95}, {"feedback", "代入方法合理且答案正确。"}};
    // 【逐行批注】 计算右侧结果并写入 turn["evaluationModel"]，更新当前运行状态。
    turn["evaluationModel"] = "评价模型";
    // 【逐行批注】 计算右侧结果并写入 auto options，更新当前运行状态。
    const auto options = gangyi::dialogueOptions(db, body, turn);
    // 【逐行批注】 调用 check 完成本行所需操作：check(options.messages[0].content.find("先用通俗中文解释一小点") == std::string:…。
    check(options.messages[0].content.find("先用通俗中文解释一小点") == std::string::npos &&
        // 【逐行批注】 执行当前声明或表达式：options.messages[0].content.find("已完成真实评价") != std::string::npos, "讲解使用完成评价并自主选择教法");。
        options.messages[0].content.find("已完成真实评价") != std::string::npos, "讲解使用完成评价并自主选择教法");
    // 【逐行批注】 计算右侧结果并写入 finished，更新当前运行状态。
    auto finished = gangyi::finishDialogue(db, body, turn, "你的代入过程正确。", "讲解模型");
    // 【逐行批注】 调用 check 完成本行所需操作：check(finished["evaluationStatus"] == "ready", "评价与连续讲解必须一次性完成提交");。
    check(finished["evaluationStatus"] == "ready", "评价与连续讲解必须一次性完成提交");
    // 【逐行批注】 计算右侧结果并写入 saved，更新当前运行状态。
    auto saved = gangyi::dialogueView(db, body);
    // 【逐行批注】 调用 check 完成本行所需操作：check(saved["credible"] == true && saved["question"]["status"] == "an…。
    check(saved["credible"] == true && saved["question"]["status"] == "answered", "可靠评价与问题状态同步保存");
    // 【逐行批注】 计算右侧结果并写入 body["requestId"]，更新当前运行状态。
    body["requestId"] = "request-2"; body["version"] = saved["version"];
    // 【逐行批注】 计算右侧结果并写入 body["question"]，更新当前运行状态。
    body["question"] = "我再算一下。";
    // 【逐行批注】 计算右侧结果并写入 failed，更新当前运行状态。
    auto failed = gangyi::beginDialogue(db, body);
    // 【逐行批注】 计算右侧结果并写入 failed["evaluation"]，更新当前运行状态。
    failed["evaluation"] = {{"isAnswer", true}, {"correct", false}, {"unknown", false}, {"confidence", 0.95}, {"feedback", "此次答案存在计算错误。"}};
    // 【逐行批注】 计算右侧结果并写入 failed["evaluationModel"]，更新当前运行状态。
    failed["evaluationModel"] = "评价模型";
    // 【逐行批注】 调用 finishDialogue 完成本行所需操作：gangyi::finishDialogue(db, body, failed, "截断文本", "讲解模型", "length");。
    gangyi::finishDialogue(db, body, failed, "截断文本", "讲解模型", "length");
    // 【逐行批注】 调用 check、dialogueView 完成本行所需操作：check(gangyi::dialogueView(db, body)["correct"] == true, "截断讲解不得覆盖原有可…。
    check(gangyi::dialogueView(db, body)["correct"] == true, "截断讲解不得覆盖原有可靠评价");
    // 【逐行批注】 计算右侧结果并写入 body["requestId"]，更新当前运行状态。
    body["requestId"] = "request-3"; body["version"] = gangyi::dialogueView(db, body)["version"];
    // 【逐行批注】 计算右侧结果并写入 body["contentVersion"]，更新当前运行状态。
    body["contentVersion"] = 6;
    // 【逐行批注】 计算右侧结果并写入 bool stale，更新当前运行状态。
    bool stale = false; try { gangyi::beginDialogue(db, body); } catch (...) { stale = true; }
    // 【逐行批注】 调用 check 完成本行所需操作：check(stale, "内容版本变化必须拒绝旧窗口作答");。
    check(stale, "内容版本变化必须拒绝旧窗口作答");
    // 【逐行批注】 计算右侧结果并写入 changedSession，更新当前运行状态。
    auto changedSession = *db.findLearningSession("q-course", 1, 1);
    // 【逐行批注】 计算右侧结果并写入 otherBlockChanged，更新当前运行状态。
    auto otherBlockChanged = content; otherBlockChanged["contentVersion"] = 8;
    // 【逐行批注】 计算右侧结果并写入 otherBlockChanged["blocks"]["overview"]，更新当前运行状态。
    otherBlockChanged["blocks"]["overview"] = {{"title", "只更新其他板块"}};
    // 【逐行批注】 计算右侧结果并写入 changedSession.content，更新当前运行状态。
    changedSession.content = otherBlockChanged.dump(); db.update(changedSession);
    // 【逐行批注】 调用 check、classroomQuestion 完成本行所需操作：check(gangyi::classroomQuestion(db, body)["contentVersion"] == 7, "只更…。
    check(gangyi::classroomQuestion(db, body)["contentVersion"] == 7, "只更新其他板块时题目版本保持稳定且能恢复作答");
    // 【逐行批注】 计算右侧结果并写入 body["contentVersion"]，更新当前运行状态。
    body["contentVersion"] = 7; body["requestId"] = "request-4"; body["question"] = "能介绍另一种方法吗？";
    // 【逐行批注】 计算右侧结果并写入 auto followup，更新当前运行状态。
    const auto followup = gangyi::beginDialogue(db, body);
    // 【逐行批注】 调用 check 完成本行所需操作：check(followup["assisted"] == true, "看过讲解后的新作答必须标记受帮助");。
    check(followup["assisted"] == true, "看过讲解后的新作答必须标记受帮助");
    // 【逐行批注】 计算右侧结果并写入 ask，更新当前运行状态。
    auto ask = followup;
    // 【逐行批注】 计算右侧结果并写入 ask["evaluation"]，更新当前运行状态。
    ask["evaluation"] = {{"isAnswer", false}, {"correct", false}, {"unknown", false}, {"confidence", 0.9}, {"feedback", "学生请求其他解法，并未重新作答。"}};
    // 【逐行批注】 计算右侧结果并写入 ask["evaluationModel"]，更新当前运行状态。
    ask["evaluationModel"] = "评价模型";
    // 【逐行批注】 调用 finishDialogue 完成本行所需操作：gangyi::finishDialogue(db, body, ask, "此题也可以换元。", "讲解模型");。
    gangyi::finishDialogue(db, body, ask, "此题也可以换元。", "讲解模型");
    // 【逐行批注】 调用 check、dialogueView 完成本行所需操作：check(gangyi::dialogueView(db, body)["correct"] == true, "纯追问不能覆盖原题成绩…。
    check(gangyi::dialogueView(db, body)["correct"] == true, "纯追问不能覆盖原题成绩");
    // 【逐行批注】 调用 check、beginDialogue 完成本行所需操作：check(gangyi::beginDialogue(db, body)["cached"] == true, "同一已完成reques…。
    check(gangyi::beginDialogue(db, body)["cached"] == true, "同一已完成requestId复用对话而不重复AI");
    // 【逐行批注】 计算右侧结果并写入 partialBody，更新当前运行状态。
    auto partialBody = Json{{"courseId", "q-course"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"kind", "practice"},
        // 【逐行批注】 执行当前声明或表达式：{"index", 0}, {"requestId", "partial-stop"}, {"question", "我先试着证明。"}, {"version", 0}…。
        {"index", 0}, {"requestId", "partial-stop"}, {"question", "我先试着证明。"}, {"version", 0}};
    // 【逐行批注】 计算右侧结果并写入 auto partialQuestion，更新当前运行状态。
    const auto partialQuestion = gangyi::classroomQuestion(db, partialBody);
    // 【逐行批注】 计算右侧结果并写入 partialBody["questionId"]，更新当前运行状态。
    partialBody["questionId"] = partialQuestion["questionId"]; partialBody["contentVersion"] = partialQuestion["contentVersion"];
    // 【逐行批注】 计算右侧结果并写入 partialTurn，更新当前运行状态。
    auto partialTurn = gangyi::beginDialogue(db, partialBody);
    // 【逐行批注】 调用 check 完成本行所需操作：check(partialTurn["assisted"] == false, "首次作答没有看过帮助时保持独立状态");。
    check(partialTurn["assisted"] == false, "首次作答没有看过帮助时保持独立状态");
    // 【逐行批注】 调用 check、recordDialogueAssistance 完成本行所需操作：check(gangyi::recordDialogueAssistance(db, partialTurn, "先观察函数定义中的条件。…。
    check(gangyi::recordDialogueAssistance(db, partialTurn, "先观察函数定义中的条件。"), "真实讲解分片保存独立展示记录");
    // 【逐行批注】 调用 finishDialogue 完成本行所需操作：gangyi::finishDialogue(db, partialBody, partialTurn, "", "", "cancell…。
    gangyi::finishDialogue(db, partialBody, partialTurn, "", "", "cancelled");
    // 【逐行批注】 计算右侧结果并写入 auto partialView，更新当前运行状态。
    const auto partialView = gangyi::dialogueView(db, partialBody);
    // 【逐行批注】 调用 check 完成本行所需操作：check(partialView["turns"].back()["partialAssistant"] == "先观察函数定义中的条件…。
    check(partialView["turns"].back()["partialAssistant"] == "先观察函数定义中的条件。" &&
        // 【逐行批注】 执行当前声明或表达式：partialView["turns"].back()["incomplete"] == true, "停止后刷新保留已看过的真实片段并标注未完成");。
        partialView["turns"].back()["incomplete"] == true, "停止后刷新保留已看过的真实片段并标注未完成");
    // 【逐行批注】 调用 check 完成本行所需操作：check(!partialView.value("credible", false) && partialView["question"…。
    check(!partialView.value("credible", false) && partialView["question"]["status"] == "pending",
        // 【逐行批注】 执行当前声明或表达式："只有部分讲解时不产生可靠评价");。
        "只有部分讲解时不产生可靠评价");
    // 【逐行批注】 调用 check、recordDialogueAssistance 完成本行所需操作：check(!gangyi::recordDialogueAssistance(db, partialTurn, "已经过期的分片"), …。
    check(!gangyi::recordDialogueAssistance(db, partialTurn, "已经过期的分片"), "停止后旧流不能继续写入展示记录");
    // 【逐行批注】 计算右侧结果并写入 partialBody["version"]，更新当前运行状态。
    partialBody["version"] = partialView["version"];
    // 【逐行批注】 计算右侧结果并写入 partialRetry，更新当前运行状态。
    auto partialRetry = gangyi::beginDialogue(db, partialBody);
    // 【逐行批注】 调用 check 完成本行所需操作：check(partialRetry["assisted"] == true, "看过部分讲解后重试必须记录为受帮助作答");。
    check(partialRetry["assisted"] == true, "看过部分讲解后重试必须记录为受帮助作答");
    // 【逐行批注】 调用 check、dialogueView 完成本行所需操作：check(gangyi::dialogueView(db, partialBody)["turns"].back()["previous…。
    check(gangyi::dialogueView(db, partialBody)["turns"].back()["previousPartialReplies"].size() == 1,
        // 【逐行批注】 执行当前声明或表达式："同一请求重试保留此前已经展示过的片段");。
        "同一请求重试保留此前已经展示过的片段");
    // 【逐行批注】 调用 finishDialogue 完成本行所需操作：gangyi::finishDialogue(db, partialBody, partialRetry, "", "", "cancel…。
    gangyi::finishDialogue(db, partialBody, partialRetry, "", "", "cancelled");
    // 【逐行批注】 计算右侧结果并写入 bool badChunkRejected，更新当前运行状态。
    bool badChunkRejected = false;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { badChunkRejected = !gangyi::recordDialogueAssistance(db, Json::object(), "无效请求"); } catch (...) {}
    // 【逐行批注】 调用 check 完成本行所需操作：check(badChunkRejected, "流式回调的无效数据返回失败且不向 C 回调抛出异常");。
    check(badChunkRejected, "流式回调的无效数据返回失败且不向 C 回调抛出异常");
    // 【逐行批注】 计算右侧结果并写入 Json prepared，更新当前运行状态。
    const Json prepared = {{"lessonTaskId", "isolated-task"}, {"courseId", "q-course"}, {"phaseIndex", 1}, {"topicIndex", 1},
        // 【逐行批注】 调用 array 完成本行所需操作：{"kind", "consolidation"}, {"status", "ready"}, {"content", content},…。
        {"kind", "consolidation"}, {"status", "ready"}, {"content", content}, {"exposure", {{"blocks", Json::array()}}},
        // 【逐行批注】 执行当前声明或表达式：{"progress", {{"status", "not_started"}}}};。
        {"progress", {{"status", "not_started"}}}};
    // 【逐行批注】 执行当前声明或表达式：db.upsert(gangyi::ClassroomActivity{"prepared-lesson:isolated-task", "q-course", "pr…。
    db.upsert(gangyi::ClassroomActivity{"prepared-lesson:isolated-task", "q-course", "prepared-lesson", prepared.dump(), "2026-10-04", 1, 1});
    // 【逐行批注】 计算右侧结果并写入 independent，更新当前运行状态。
    auto independent = Json{{"courseId", "q-course"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"lessonTaskId", "isolated-task"}};
    // 【逐行批注】 计算右侧结果并写入 auto consolidation，更新当前运行状态。
    const auto consolidation = gangyi::classroomQuestions(db, independent);
    // 【逐行批注】 调用 check 完成本行所需操作：check(consolidation["questions"][0]["status"] == "pending", "巩固课作答状态独…。
    check(consolidation["questions"][0]["status"] == "pending", "巩固课作答状态独立于原课堂");
    // 【逐行批注】 调用 check 完成本行所需操作：check(consolidation["questions"][0]["questionId"] == q["questionId"],…。
    check(consolidation["questions"][0]["questionId"] == q["questionId"], "同一道原题在巩固课中仍只算一道证据");
    // 【逐行批注】 执行当前声明或表达式：db.upsert(gangyi::ClassroomActivity{"classroom:q-course:1:1:review:1", "q-course", "…。
    db.upsert(gangyi::ClassroomActivity{"classroom:q-course:1:1:review:1", "q-course", "review",
        // 【逐行批注】 执行当前声明或表达式：Json{{"title", "函数复习"}, {"due", "2026-10-04"}, {"day", 1}, {"status", "pending"}}.du…。
        Json{{"title", "函数复习"}, {"due", "2026-10-04"}, {"day", 1}, {"status", "pending"}}.dump(), "2026-10-04", 1, 1});
    // 【逐行批注】 计算右侧结果并写入 reviewBody，更新当前运行状态。
    auto reviewBody = Json{{"courseId", "q-course"}, {"phaseIndex", 1}, {"topicIndex", 1}, {"kind", "review"}, {"day", 1}, {"today", "2026-10-04"}};
    // 【逐行批注】 计算右侧结果并写入 auto reviewQuestions，更新当前运行状态。
    const auto reviewQuestions = gangyi::classroomQuestions(db, reviewBody);
    // 【逐行批注】 调用 check 完成本行所需操作：check(reviewQuestions["questions"].size() == 1, "复习题接入同一公开接口");。
    check(reviewQuestions["questions"].size() == 1, "复习题接入同一公开接口");
    // 【逐行批注】 调用 check、dueReviews 完成本行所需操作：check(gangyi::dueReviews(db, "q-course", "2026-10-04").size() == 1, "…。
    check(gangyi::dueReviews(db, "q-course", "2026-10-04").size() == 1, "复习逐题活动不冒充待办复习任务");
    // 【逐行批注】 计算右侧结果并写入 auto noEvidence，更新当前运行状态。
    const auto noEvidence = gangyi::completeReviewFromEvaluations(db, reviewBody, Json::array({{{"answered", false}, {"credible", false}}}));
    // 【逐行批注】 调用 check 完成本行所需操作：check(noEvidence["total"] == 0 && noEvidence["evaluationStatus"] == "…。
    check(noEvidence["total"] == 0 && noEvidence["evaluationStatus"] == "insufficient", "漏答不算不会且不自动完成复习");
    // 【逐行批注】 结束当前函数并返回结果：return failures ? 1 : 0;。
    return failures ? 1 : 0;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "teaching_evidence.hpp"，为后续实现提供类型或接口声明。
#include "teaching_evidence.hpp"
// 【逐行批注】 引入头文件 "agent_profiles.hpp"，为后续实现提供类型或接口声明。
#include "agent_profiles.hpp"
// 【逐行批注】 引入头文件 "my_ai.hpp"，为后续实现提供类型或接口声明。
#include "my_ai.hpp"
// 【逐行批注】 引入头文件 "agent_prompt.hpp"，为后续实现提供类型或接口声明。
#include "agent_prompt.hpp"
// 【逐行批注】 引入头文件 "teaching_protocol_checks.hpp"，为后续实现提供类型或接口声明。
#include "teaching_protocol_checks.hpp"

// 接口回归使用确定性模型夹具；真实教学效果另由同模型多轮对照验证。
// 【逐行批注】 定义函数 runTeachingEvidenceChecks，执行该函数声明的职责。
inline void runTeachingEvidenceChecks(const std::filesystem::path& path) {
    // 【逐行批注】 调用 runTeachingProtocolChecks 完成本行所需操作：runTeachingProtocolChecks(path.parent_path() / "teaching-protocol.db"…。
    runTeachingProtocolChecks(path.parent_path() / "teaching-protocol.db");
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(path.string()); db.migrate();。
    gangyi::Database db; db.open(path.string()); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess access{"teaching-check", {"math"}};。
    const gangyi::AgentAccess access{"teaching-check", {"math"}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "数学", "de…。
    assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "数学", "deep", "数学", {}, "ai", "active", "2026-10-13", "2026-10-13"}));
    // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
    Json lesson = {{"id", "lesson-check"}, {"scopeId", access.scopeId}, {"courseId", "math"}, {"title", "分数"},
        // 【逐行批注】 执行当前声明或表达式：{"purpose", "验证教学记录"}, {"status", "ready"}, {"entered", true}, {"version", 1}, {"upd…。
        {"purpose", "验证教学记录"}, {"status", "ready"}, {"entered", true}, {"version", 1}, {"updatedAt", "2026-10-13"},
        // 【逐行批注】 执行当前声明或表达式：{"sourceLearningVersion", db.learningRevision()}, {"sections", {。
        {"sourceLearningVersion", db.learningRevision()}, {"sections", {
            // 【逐行批注】 执行当前声明或表达式：{{"id", "concept"}, {"kind", "explanation"}, {"title", "分数概念"}, {"body", "分母表示把同一整体平…。
            {{"id", "concept"}, {"kind", "explanation"}, {"title", "分数概念"}, {"body", "分母表示把同一整体平均分成的份数，每份相当于整体的1/分母"}, {"version", 1}},
            // 【逐行批注】 执行当前声明或表达式：{{"id", "q1"}, {"kind", "question"}, {"title", "第一题"}, {"questionKind", "practice"},…。
            {{"id", "q1"}, {"kind", "question"}, {"title", "第一题"}, {"questionKind", "practice"}, {"questionId", "qid-1"}, {"version", 1},
                // 【逐行批注】 计算右侧结果并写入 {"question", {{"question", "1/2+1/3，更新当前运行状态。
                {"question", {{"question", "1/2+1/3=?"}, {"expectedAnswer", "5/6"}, {"rubric", "PRIVATE-CHECK"}}}},
            // 【逐行批注】 执行当前声明或表达式：{{"id", "q2"}, {"kind", "question"}, {"title", "第二题"}, {"questionKind", "practice"},…。
            {{"id", "q2"}, {"kind", "question"}, {"title", "第二题"}, {"questionKind", "practice"}, {"questionId", "qid-2"}, {"version", 1},
                // 【逐行批注】 计算右侧结果并写入 {"question", {{"question", "1/4+1/6，更新当前运行状态。
                {"question", {{"question", "1/4+1/6=?"}, {"expectedAnswer", "5/12"}}}}
        // 【逐行批注】 执行当前声明或表达式：}}};。
        }}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(gangyi::ClassroomActivity{"lesson-check", "math", "a…。
    assert(db.upsert(gangyi::ClassroomActivity{"lesson-check", "math", "agent-lesson", lesson.dump(), "2026-10-13", 0, 0}));
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"lessonId", "lesson-check"}, {"…。
    gangyi::agentExposeTask(db, access, {{"lessonId", "lesson-check"}, {"sections", {{{"id", "concept"}, {"version", 1}}}}});
    // 【逐行批注】 计算右侧结果并写入 int identity，更新当前运行状态。
    int identity = 0;
    // 【逐行批注】 计算右侧结果并写入 auto submit，更新当前运行状态。
    const auto submit = [&](std::string section, std::string type, std::string text, std::string request = "") {
        // 【逐行批注】 计算右侧结果并写入 Json event，更新当前运行状态。
        Json event = {{"type", type}, {"text", text}, {"lessonId", "lesson-check"}, {"courseId", "math"},
            // 【逐行批注】 调用 to_string 完成本行所需操作：{"sectionId", section}, {"sectionVersion", 1}, {"view", "practice"}, …。
            {"sectionId", section}, {"sectionVersion", 1}, {"view", "practice"}, {"requestId", "teaching-check-"+std::to_string(++identity)}};
        // 【逐行批注】 判断条件 (!request.empty()) event["requestType"] = request;，成立时执行对应分支。
        if (!request.empty()) event["requestType"] = request;
        // 【逐行批注】 结束当前函数并返回结果：return gangyi::agentSubmit(db, access, event);。
        return gangyi::agentSubmit(db, access, event);
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto inputFor，更新当前运行状态。
    const auto inputFor = [&](const Json& task) {
        // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
        const auto raw = Json::parse(db.getClassroomActivity(task.at("id"))->payload);
        // 【逐行批注】 结束当前函数并返回结果：return gangyi::agentContext(db, access, raw.at("event")).at("currentI…。
        return gangyi::agentContext(db, access, raw.at("event")).at("currentInputs")[0];
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
    const auto first = submit("q1", "question_answer", "2/5");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(first).at("helpMode") == "independent" && inputFor(fi…。
    assert(inputFor(first).at("helpMode") == "independent" && inputFor(first).at("relatedLearning").size() == 1);
    // 【逐行批注】 计算右侧结果并写入 auto sourceContext，更新当前运行状态。
    const auto sourceContext = gangyi::agentContext(db, access, Json::parse(db.getClassroomActivity(first.at("id"))->payload).at("event"));
    // 【逐行批注】 计算右侧结果并写入 longContext，更新当前运行状态。
    auto longContext = sourceContext;
    // 【逐行批注】 调用 string 完成本行所需操作：longContext["courses"].push_back({{"id", "other-course"}, {"outline",…。
    longContext["courses"].push_back({{"id", "other-course"}, {"outline", std::string(12000, 'x')}});
    // 【逐行批注】 计算右侧结果并写入 longContext["studyPlan"]，更新当前运行状态。
    longContext["studyPlan"] = {{"large", std::string(12000, 'x')}};
    // 【逐行批注】 计算右侧结果并写入 auto compactContext，更新当前运行状态。
    const auto compactContext = gangyi::agentPromptContext(longContext, true);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(compactContext.at("currentInputs") == sourceContext.at("curren…。
    assert(compactContext.at("currentInputs") == sourceContext.at("currentInputs"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(compactContext.at("personalCenter") == sourceContext.at("perso…。
    assert(compactContext.at("personalCenter") == sourceContext.at("personalCenter"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(compactContext.at("courses").size() == 1 && !compactContext.co…。
    assert(compactContext.at("courses").size() == 1 && !compactContext.contains("studyPlan"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(longContext.at("courses").size() == 2 && longContext.contains(…。
    assert(longContext.at("courses").size() == 2 && longContext.contains("studyPlan"));
    // 调用状态文件重建后，同一任务仍从第1次计数开始，也必须保留两笔独立用量。
    // 【逐行批注】 调用 configureUsage 完成本行所需操作：gangyi::AIActivity::configureUsage(path.string());。
    gangyi::AIActivity::configureUsage(path.string());
    // 【逐行批注】 计算右侧结果并写入 gangyi::AIActivityInfo usageInfo; usageInfo.…，更新当前运行状态。
    gangyi::AIActivityInfo usageInfo; usageInfo.taskId = "usage-recovery-check"; usageInfo.purpose = "免费用量恢复回归";
    // 【逐行批注】 计算右侧结果并写入 int usageIndex，更新当前运行状态。
    int usageIndex = 0;
    // 【逐行批注】 按循环规则重复处理：for (const auto* suffix : {".usage-before.json", ".usage-after.json"}) {。
    for (const auto* suffix : {".usage-before.json", ".usage-after.json"}) {
        // 【逐行批注】 调用 configure 完成本行所需操作：gangyi::AIActivity::configure(path.string()+suffix);。
        gangyi::AIActivity::configure(path.string()+suffix);
        // 【逐行批注】 计算右侧结果并写入 auto usageTask，更新当前运行状态。
        const auto usageTask = gangyi::AIActivity::begin(usageInfo, "fixture");
        // 【逐行批注】 调用 assert、requestStarted 完成本行所需操作：assert(gangyi::AIActivity::requestStarted(usageTask));。
        assert(gangyi::AIActivity::requestStarted(usageTask));
        // 【逐行批注】 调用 requestFinished 完成本行所需操作：gangyi::AIActivity::requestFinished(usageTask, "completed", {{"total_…。
        gangyi::AIActivity::requestFinished(usageTask, "completed", {{"total_tokens", ++usageIndex*11}}, "fixture");
        // 【逐行批注】 调用 update 完成本行所需操作：gangyi::AIActivity::update(usageTask, "completed");。
        gangyi::AIActivity::update(usageTask, "completed");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto recoveredUsage，更新当前运行状态。
    const auto recoveredUsage = db.myAIRecords("usage");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(recoveredUsage.size() == 2 && recoveredUsage[0].id != recovere…。
    assert(recoveredUsage.size() == 2 && recoveredUsage[0].id != recoveredUsage[1].id);
    // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(recoveredUsage[0].payload).at("tokens").at("total_…。
    assert(Json::parse(recoveredUsage[0].payload).at("tokens").at("total_tokens").get<int>()+
        // 【逐行批注】 调用 parse 完成本行所需操作：Json::parse(recoveredUsage[1].payload).at("tokens").at("total_tokens"…。
        Json::parse(recoveredUsage[1].payload).at("tokens").at("total_tokens").get<int>() == 33);
    // 【逐行批注】 调用 configureUsage、configure 完成本行所需操作：gangyi::AIActivity::configureUsage(""); gangyi::AIActivity::configure…。
    gangyi::AIActivity::configureUsage(""); gangyi::AIActivity::configure("");
    // 【逐行批注】 计算右侧结果并写入 Json duplicateRead，更新当前运行状态。
    const Json duplicateRead = {{"sections", {std::string(12000, 'x')}}, {"version", 1}};
    // 【逐行批注】 计算右侧结果并写入 Json changedRead，更新当前运行状态。
    const Json changedRead = {{"sections", {std::string(12000, 'y')}}, {"version", 2}};
    // 【逐行批注】 计算右侧结果并写入 Json readAction，更新当前运行状态。
    const Json readAction = {{"encoded", Json{{"tool", "read_lesson"}, {"args", {{"lessonId", "lesson-check"}}}}.dump()}};
    // 【逐行批注】 计算右侧结果并写入 Json writeAction，更新当前运行状态。
    const Json writeAction = {{"encoded", Json{{"tool", "append_section"}, {"args", Json::object()}}.dump()}};
    // 【逐行批注】 计算右侧结果并写入 Json history，更新当前运行状态。
    Json history = Json::array();
    // 【逐行批注】 按循环规则重复处理：for (const auto& id : {"read1", "read2", "read3", "write1", "write2"})。
    for (const auto& id : {"read1", "read2", "read3", "write1", "write2"})
        // 【逐行批注】 调用 string 完成本行所需操作：history.push_back({{"role", "user"}, {"content", Json{{"toolResults",…。
        history.push_back({{"role", "user"}, {"content", Json{{"toolResults", {{{"id", id}, {"result", std::string(id) == "read3" ? changedRead : duplicateRead}}}}}.dump()}});
    // 【逐行批注】 执行当前声明或表达式：history.push_back({{"role", "user"}, {"content", Json{{"toolError", "需要修正实际字段"}}.dum…。
    history.push_back({{"role", "user"}, {"content", Json{{"toolError", "需要修正实际字段"}}.dump()}});
    // 【逐行批注】 计算右侧结果并写入 Json actionIndex，更新当前运行状态。
    const Json actionIndex = {{"read1", readAction}, {"read2", readAction}, {"read3", readAction}, {"write1", writeAction}, {"write2", writeAction}};
    // 【逐行批注】 计算右侧结果并写入 auto compactHistory，更新当前运行状态。
    const auto compactHistory = gangyi::agentPromptHistory(history, actionIndex, {"read_lesson"});
    // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(compactHistory[1].at("content").get<std::string>()…。
    assert(Json::parse(compactHistory[1].at("content").get<std::string>()).at("toolResults")[0].at("result").at("unchangedFrom") == "read1");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(compactHistory[0] == history[0] && compactHistory[2] == histor…。
    assert(compactHistory[0] == history[0] && compactHistory[2] == history[2] && compactHistory[3] == history[3] &&
        // 【逐行批注】 执行当前声明或表达式：compactHistory[4] == history[4] && compactHistory[5] == history[5]);。
        compactHistory[4] == history[4] && compactHistory[5] == history[5]);
    // 短结果和长行动标识不应因引用变大；普通结果及恢复凭据都适用。
    // 【逐行批注】 按循环规则重复处理：for (const auto& tiny : Json::array({nullptr, true, 7, "OK", Json::array()…。
    for (const auto& tiny : Json::array({nullptr, true, 7, "OK", Json::array(), Json::object()})) {
        // 【逐行批注】 计算右侧结果并写入 Json tinyReceipt，更新当前运行状态。
        const Json tinyReceipt={{"encoded",readAction.at("encoded")},{"result",tiny}};
        // 【逐行批注】 计算右侧结果并写入 Json tinyActions，更新当前运行状态。
        const Json tinyActions={{"one",tinyReceipt},{"two",tinyReceipt}};
        // 【逐行批注】 计算右侧结果并写入 Json tinyHistory，更新当前运行状态。
        const Json tinyHistory=Json::array({
            // 【逐行批注】 执行当前声明或表达式：{{"role","user"},{"content",Json{{"toolResults",{{{"id","one"},{"result",tiny}},{{"i…。
            {{"role","user"},{"content",Json{{"toolResults",{{{"id","one"},{"result",tiny}},{{"id","two"},{"result",tiny}}}}}.dump()}},
            // 【逐行批注】 执行当前声明或表达式：{{"role","user"},{"content",Json{{"completedActions",{{"one",tinyReceipt}}}}.dump()}}。
            {{"role","user"},{"content",Json{{"completedActions",{{"one",tinyReceipt}}}}.dump()}}
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 assert、agentPromptHistory 完成本行所需操作：assert(gangyi::agentPromptHistory(tinyHistory,tinyActions,{"read_less…。
        assert(gangyi::agentPromptHistory(tinyHistory,tinyActions,{"read_lesson"})==tinyHistory);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 longActionId 完成本行所需操作：const std::string longActionId(512,'a');。
    const std::string longActionId(512,'a');
    // 【逐行批注】 计算右侧结果并写入 Json shortResult，更新当前运行状态。
    const Json shortResult={{"text",std::string(128,'x')}};
    // 【逐行批注】 计算右侧结果并写入 Json longIdActions，更新当前运行状态。
    const Json longIdActions={{longActionId,readAction},{"short",readAction}};
    // 【逐行批注】 计算右侧结果并写入 Json longIdHistory，更新当前运行状态。
    const Json longIdHistory=Json::array({{{"role","user"},{"content",Json{{"toolResults",{
        // 【逐行批注】 执行当前声明或表达式：{{"id",longActionId},{"result",shortResult}},{{"id","short"},{"result",shortResult}}。
        {{"id",longActionId},{"result",shortResult}},{{"id","short"},{"result",shortResult}}
    // 【逐行批注】 执行当前声明或表达式：}}}.dump()}}});。
    }}}.dump()}}});
    // 【逐行批注】 调用 assert、agentPromptHistory 完成本行所需操作：assert(gangyi::agentPromptHistory(longIdHistory,longIdActions,{"read_…。
    assert(gangyi::agentPromptHistory(longIdHistory,longIdActions,{"read_lesson"})==longIdHistory);
    // 真实对照发现工具报错会反复携带全量成功结果；只引用已经在历史中发送过的相同凭据。
    // 【逐行批注】 计算右侧结果并写入 Json savedWrite，更新当前运行状态。
    const Json savedWrite = {{"encoded", writeAction.at("encoded")}, {"result", duplicateRead}};
    // 【逐行批注】 计算右侧结果并写入 Json newlySaved，更新当前运行状态。
    const Json newlySaved = {{"encoded", readAction.at("encoded")}, {"result", changedRead}};
    // 【逐行批注】 计算右侧结果并写入 Json correction，更新当前运行状态。
    const Json correction = {{"toolError", "画像引用不存在的可靠原题或实际回答"},
        // 【逐行批注】 执行当前声明或表达式：{"completedActions", {{"write1", savedWrite}, {"newRead", newlySaved}}}};。
        {"completedActions", {{"write1", savedWrite}, {"newRead", newlySaved}}}};
    // 【逐行批注】 计算右侧结果并写入 Json retryHistory，更新当前运行状态。
    Json retryHistory = Json::array({history[3], {{"role", "user"}, {"content", correction.dump()}},
        // 【逐行批注】 执行当前声明或表达式：{{"role", "user"}, {"content", Json{{"toolError", "仍需修正"}, {"savedActions", correcti…。
        {{"role", "user"}, {"content", Json{{"toolError", "仍需修正"}, {"savedActions", correction.at("completedActions")}}.dump()}}});
    // 【逐行批注】 计算右侧结果并写入 auto compactRetries，更新当前运行状态。
    const auto compactRetries = gangyi::agentPromptHistory(retryHistory, actionIndex, {"read_lesson"});
    // 【逐行批注】 计算右侧结果并写入 auto firstCorrection，更新当前运行状态。
    const auto firstCorrection = Json::parse(compactRetries[1].at("content").get<std::string>());
    // 【逐行批注】 计算右侧结果并写入 auto laterCorrection，更新当前运行状态。
    const auto laterCorrection = Json::parse(compactRetries[2].at("content").get<std::string>());
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(firstCorrection.at("toolError") == correction.at("toolError"));。
    assert(firstCorrection.at("toolError") == correction.at("toolError"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(firstCorrection.at("completedActions").at("write1").at("encode…。
    assert(firstCorrection.at("completedActions").at("write1").at("encoded") == savedWrite.at("encoded"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(firstCorrection.at("completedActions").at("write1").at("result…。
    assert(firstCorrection.at("completedActions").at("write1").at("result").at("unchangedFrom") == "write1");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(firstCorrection.at("completedActions").at("newRead") == newlyS…。
    assert(firstCorrection.at("completedActions").at("newRead") == newlySaved);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(laterCorrection.at("savedActions").at("newRead").at("result").…。
    assert(laterCorrection.at("savedActions").at("newRead").at("result").at("unchangedFrom") == "newRead");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(retryHistory[1].at("content") == correction.dump());。
    assert(retryHistory[1].at("content") == correction.dump());
    // 【逐行批注】 计算右侧结果并写入 changedReceipt，更新当前运行状态。
    auto changedReceipt = correction;
    // 【逐行批注】 计算右侧结果并写入 changedReceipt["completedActions"]["write1"]…，更新当前运行状态。
    changedReceipt["completedActions"]["write1"]["result"] = changedRead;
    // 【逐行批注】 执行当前声明或表达式：retryHistory.push_back({{"role", "user"}, {"content", changedReceipt.dump()}});。
    retryHistory.push_back({{"role", "user"}, {"content", changedReceipt.dump()}});
    // 【逐行批注】 计算右侧结果并写入 auto changedRetries，更新当前运行状态。
    const auto changedRetries = gangyi::agentPromptHistory(retryHistory, actionIndex, {"read_lesson"});
    // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(changedRetries[3].at("content").get<std::string>()…。
    assert(Json::parse(changedRetries[3].at("content").get<std::string>()).at("completedActions").at("write1").at("result") == changedRead);
    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", f…。
    gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", first.at("id")}});
    // 【逐行批注】 计算右侧结果并写入 hint，更新当前运行状态。
    auto hint = submit("q1", "teaching_request", "给一点提示", "hint");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(hint).at("type") == "teaching_request");。
    assert(inputFor(hint).at("type") == "teaching_request");
    // 【逐行批注】 计算右侧结果并写入 bool fullExample，更新当前运行状态。
    bool fullExample = true, wrongTarget = false;
    // 【逐行批注】 定义函数 helper，执行该函数声明的职责。
    gangyi::LearningAgent helper(path.string(), [&](const auto&, const auto& sink) {
        // 【逐行批注】 计算右侧结果并写入 Json actions，更新当前运行状态。
        Json actions = Json::array();
        // 【逐行批注】 判断条件 (wrongTarget) actions.push_back({{"id", "wrong-hint-target"}, {"…，成立时执行对应分支。
        if (wrongTarget) actions.push_back({{"id", "wrong-hint-target"}, {"tool", "append_section"}, {"args", {
            // 【逐行批注】 执行当前声明或表达式：{"lessonId", "lesson-check"}, {"section", {{"kind", "explanation"}, {"title", "另一题提示…。
            {"lessonId", "lesson-check"}, {"section", {{"kind", "explanation"}, {"title", "另一题提示"},
                // 【逐行批注】 执行当前声明或表达式：{"body", "这是另一题的提示，不应该展示"}, {"helpType", "hint"}, {"targetQuestionId", "qid-2"}}}。
                {"body", "这是另一题的提示，不应该展示"}, {"helpType", "hint"}, {"targetQuestionId", "qid-2"}}}
        // 【逐行批注】 执行当前声明或表达式：}}});。
        }}});
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", fullExample ? "完整答案5/6" : "先想想两种分数的每份大小是否相…。
        return output({{"message", fullExample ? "完整答案5/6" : "先想想两种分数的每份大小是否相同。"},
            // 【逐行批注】 执行当前声明或表达式：{"teaching", {{"helpType", fullExample ? "worked_example" : "hint"}}}, {"actions", a…。
            {"teaching", {{"helpType", fullExample ? "worked_example" : "hint"}}}, {"actions", actions}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(helper); helper.process(db, hint.at(…。
    gangyi::registerAgentLessonTools(helper); helper.process(db, hint.at("id"));
    // 【逐行批注】 计算右侧结果并写入 rejected，更新当前运行状态。
    auto rejected = gangyi::agentView(db, access, hint.at("id")); assert(rejected.at("status") == "failed");
    // 【逐行批注】 按循环规则重复处理：for (const auto& event : rejected.at("events")) assert(event.value("field"…。
    for (const auto& event : rejected.at("events")) assert(event.value("field", "") != "message" || event.value("provisional", false));
    // 【逐行批注】 计算右侧结果并写入 fullExample，更新当前运行状态。
    fullExample = false; wrongTarget = true;
    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", hi…。
    gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", hint.at("id")}}); helper.process(db, hint.at("id"));
    // 【逐行批注】 计算右侧结果并写入 rejected，更新当前运行状态。
    rejected = gangyi::agentView(db, access, hint.at("id")); assert(rejected.at("status") == "failed");
    // 【逐行批注】 调用 assert、agentLessonView 完成本行所需操作：assert(gangyi::agentLessonView(db, access, "lesson-check").at("sectio…。
    assert(gangyi::agentLessonView(db, access, "lesson-check").at("sections").size() == 3);
    // 【逐行批注】 按循环规则重复处理：for (const auto& event : rejected.at("events")) assert(event.value("field"…。
    for (const auto& event : rejected.at("events")) assert((event.value("field", "") != "message" && event.value("field", "") != "body") || event.value("provisional", false));
    // 【逐行批注】 计算右侧结果并写入 wrongTarget，更新当前运行状态。
    wrongTarget = false; gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", hint.at("id")}});
    // 【逐行批注】 执行当前声明或表达式：helper.process(db, hint.at("id"));。
    helper.process(db, hint.at("id"));
    // 【逐行批注】 计算右侧结果并写入 rawHint，更新当前运行状态。
    auto rawHint = Json::parse(db.getClassroomActivity(hint.at("id"))->payload);
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"taskId", hint.at("id")}, {"seq…。
    gangyi::agentExposeTask(db, access, {{"taskId", hint.at("id")}, {"seq", rawHint.at("seq")}});
    // 【逐行批注】 计算右侧结果并写入 auto partialAnswer，更新当前运行状态。
    const auto partialAnswer = submit("q1", "question_answer", "还没看完整提示");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(partialAnswer).at("helpMode") == "help_unknown");。
    assert(inputFor(partialAnswer).at("helpMode") == "help_unknown");
    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", p…。
    gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", partialAnswer.at("id")}});
    // 【逐行批注】 计算右侧结果并写入 auto otherAnswer，更新当前运行状态。
    const auto otherAnswer = submit("q2", "question_answer", "没有看本题提示");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(otherAnswer).at("helpMode") == "independent");。
    assert(inputFor(otherAnswer).at("helpMode") == "independent");
    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", o…。
    gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", otherAnswer.at("id")}});
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"taskId", hint.at("id")}, {"seq…。
    gangyi::agentExposeTask(db, access, {{"taskId", hint.at("id")}, {"seq", rawHint.at("seq")}, {"complete", true}});
    // 【逐行批注】 调用 agentExposeTask、array 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"taskId", hint.at("id")}, {"seq…。
    gangyi::agentExposeTask(db, access, {{"taskId", hint.at("id")}, {"seq", rawHint.at("seq")}, {"completeSequences", Json::array()}});
    // 【逐行批注】 计算右侧结果并写入 auto wrongExample，更新当前运行状态。
    const auto wrongExample = submit("q1", "teaching_request", "看本题示范", "worked_example");
    // 【逐行批注】 定义函数 exampleGuard，执行该函数声明的职责。
    gangyi::LearningAgent exampleGuard(path.string(), [&](const auto&, const auto& sink) {
        // 【逐行批注】 计算右侧结果并写入 Json section，更新当前运行状态。
        const Json section = {{"kind", "explanation"}, {"title", "另一题示范"}, {"body", "另一题完整步骤不能显示"},
            // 【逐行批注】 执行当前声明或表达式：{"helpType", "worked_example"}, {"targetQuestionId", "qid-2"}};。
            {"helpType", "worked_example"}, {"targetQuestionId", "qid-2"}};
        // 【逐行批注】 计算右侧结果并写入 Json action，更新当前运行状态。
        const Json action = {{"id", "wrong-example-target"}, {"tool", "append_section"}, {"args", {{"lessonId", "lesson-check"}, {"section", section}}}};
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "另一题完整步骤不能显示"}, {"teaching", {{"helpType",…。
        return output({{"message", "另一题完整步骤不能显示"}, {"teaching", {{"helpType", "worked_example"}}},
            // 【逐行批注】 调用 array 完成本行所需操作：{"actions", Json::array({action})}, {"state", "waiting_student"}}, si…。
            {"actions", Json::array({action})}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(exampleGuard); exampleGuard.process(…。
    gangyi::registerAgentLessonTools(exampleGuard); exampleGuard.process(db, wrongExample.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto deniedExample，更新当前运行状态。
    const auto deniedExample = gangyi::agentView(db, access, wrongExample.at("id")); assert(deniedExample.at("status") == "failed");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(deniedExample.value("message", "").empty());。
    assert(deniedExample.value("message", "").empty());
    // 【逐行批注】 按循环规则重复处理：for (const auto& event : deniedExample.at("events")) assert(event.value("f…。
    for (const auto& event : deniedExample.at("events")) assert((event.value("field", "") != "message" && event.value("field", "") != "body") || event.value("provisional", false));
    // 【逐行批注】 计算右侧结果并写入 auto assisted，更新当前运行状态。
    const auto assisted = submit("q1", "question_answer", "5/6");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(assisted).at("helpMode") == "after_hint" && inputFor(…。
    // 失败轮次的公开草稿已实际展示，后续修正不能把这次帮助重新算成独立作答。
    assert(inputFor(assisted).at("helpMode") == "help_unknown" && inputFor(assisted).at("helpHistory").size() > 1);
    const auto assistedHistory = inputFor(assisted).at("helpHistory");
    assert(std::any_of(assistedHistory.begin(), assistedHistory.end(), [](const auto& entry) {
        return entry.value("helpType", "") == "hint" && !entry.value("partial", true);
    }));
    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", a…。
    gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", assisted.at("id")}});
    // 【逐行批注】 计算右侧结果并写入 auto independent，更新当前运行状态。
    const auto independent = submit("q2", "question_answer", "5/12");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(independent).at("helpMode") == "independent" && input…。
    assert(inputFor(independent).at("helpMode") == "independent" && inputFor(independent).at("helpHistory").empty());
    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", i…。
    gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", independent.at("id")}});
    // 纯追问保留原话但不评分，后续不能再引用它产生掌握证据。
    // 【逐行批注】 计算右侧结果并写入 followup，更新当前运行状态。
    auto followup = submit("q1", "question_answer", "为什么要通分？");
    // 【逐行批注】 计算右侧结果并写入 auto followupId，更新当前运行状态。
    const auto followupId = inputFor(followup).at("interactionId"); int calls = 0;
    // 【逐行批注】 定义函数 classifier，执行该函数声明的职责。
    gangyi::LearningAgent classifier(path.string(), [&](const auto&, const auto& sink) {
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", calls++ ? "需要先让每份大小相同。" : ""}, {"actions",…。
        return output({{"message", calls++ ? "需要先让每份大小相同。" : ""}, {"actions", calls == 1 ? Json{{{"id", "classify"},
            // 【逐行批注】 调用 array 完成本行所需操作：{"tool", "classify_input"}, {"args", {{"interactionId", followupId}, …。
            {"tool", "classify_input"}, {"args", {{"interactionId", followupId}, {"isAnswer", false}, {"reason", "学生只追问方法原因"}}}}} : Json::array()},
            // 【逐行批注】 执行当前声明或表达式：{"teaching", {{"helpType", "explanation"}}}, {"state", calls == 1 ? "continue" : "wa…。
            {"teaching", {{"helpType", "explanation"}}}, {"state", calls == 1 ? "continue" : "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(classifier); classifier.process(db, …。
    gangyi::registerAgentLessonTools(classifier); classifier.process(db, followup.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, followup.at("id")).at("status") …。
    assert(gangyi::agentView(db, access, followup.at("id")).at("status") == "waiting_student");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!db.profileMeta("agent-input-classification:" + followupId.get…。
    assert(!db.profileMeta("agent-input-classification:" + followupId.get<std::string>()).empty());
    // 【逐行批注】 计算右侧结果并写入 diagnosisArgs，更新当前运行状态。
    auto diagnosisArgs = Json{{"score", 30}, {"credible", true}, {"diagnosis", {{"summary", "误把分母直接相加"},
        // 【逐行批注】 执行当前声明或表达式：{"evidence", {"2/5"}}, {"teachingDirection", "补充分数单位概念"}, {"needsClarification", fal…。
        {"evidence", {"2/5"}}, {"teachingDirection", "补充分数单位概念"}, {"needsClarification", false}}}};
    // 【逐行批注】 调用 assert、checkedDiagnosis 完成本行所需操作：assert(gangyi::checkedDiagnosis(diagnosisArgs, "2/5").at("status") ==…。
    assert(gangyi::checkedDiagnosis(diagnosisArgs, "2/5").at("status") == "supported");
    // 【逐行批注】 计算右侧结果并写入 bool fabricated，更新当前运行状态。
    bool fabricated = false; try { gangyi::checkedDiagnosis(diagnosisArgs, "我暂时不会"); } catch (...) { fabricated = true; } assert(fabricated);
    // 【逐行批注】 计算右侧结果并写入 diagnosisArgs["diagnosis"]["needsClarificati…，更新当前运行状态。
    diagnosisArgs["diagnosis"]["needsClarification"] = true;
    // 【逐行批注】 计算右侧结果并写入 bool premature，更新当前运行状态。
    bool premature = false; try { gangyi::checkedDiagnosis(diagnosisArgs, "2/5"); } catch (...) { premature = true; } assert(premature);
    // 新题关联由主控实际保存，私有标准不能出现在学生接口。
    // 【逐行批注】 计算右侧结果并写入 retry，更新当前运行状态。
    auto retry = submit("q1", "teaching_request", "再试一道", "retry_question");
    // 【逐行批注】 计算右侧结果并写入 int relationMode，更新当前运行状态。
    int relationMode = 0;
    // 【逐行批注】 定义函数 practice，执行该函数声明的职责。
    gangyi::LearningAgent practice(path.string(), [&](const auto&, const auto& sink) {
        // 【逐行批注】 计算右侧结果并写入 Json section，更新当前运行状态。
        Json section = {{"kind", "question"}, {"title", "独立复练"}, {"rePracticeOf", "qid-1"},
            // 【逐行批注】 计算右侧结果并写入 {"question", {{"question", "1/5+1/2，更新当前运行状态。
            {"question", {{"question", "1/5+1/2=?"}, {"expectedAnswer", "7/10"}, {"rubric", "PRIVATE-RETRY"}}}};
        // 【逐行批注】 判断条件 (relationMode == 0) section.erase("rePracticeOf");，成立时执行对应分支。
        if (relationMode == 0) section.erase("rePracticeOf");
        // 【逐行批注】 判断条件 (relationMode == 1) section["rePracticeOf"] = "qid-2";，成立时执行对应分支。
        if (relationMode == 1) section["rePracticeOf"] = "qid-2";
        // 【逐行批注】 计算右侧结果并写入 Json action，更新当前运行状态。
        const Json action = {{"id", "new-question"}, {"tool", "append_section"}, {"args", {{"lessonId", "lesson-check"}, {"section", section}}}};
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "试试这道新题，说明每一步。"}, {"actions", Json::array(…。
        return output({{"message", "试试这道新题，说明每一步。"}, {"actions", Json::array({action})},
            // 【逐行批注】 执行当前声明或表达式：{"teaching", {{"helpType", "none"}}}, {"state", "waiting_student"}}, sink);。
            {"teaching", {{"helpType", "none"}}}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(practice);。
    gangyi::registerAgentLessonTools(practice);
    // 【逐行批注】 按循环规则重复处理：for (relationMode = 0; relationMode < 2; ++relationMode) {。
    for (relationMode = 0; relationMode < 2; ++relationMode) {
        // 【逐行批注】 执行当前声明或表达式：practice.process(db, retry.at("id"));。
        practice.process(db, retry.at("id"));
        // 【逐行批注】 计算右侧结果并写入 auto invalid，更新当前运行状态。
        const auto invalid = gangyi::agentView(db, access, retry.at("id")); assert(invalid.at("status") == "failed");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(invalid.value("message", "").empty());。
        assert(invalid.value("message", "").empty());
        // 【逐行批注】 调用 assert、agentLessonView 完成本行所需操作：assert(gangyi::agentLessonView(db, access, "lesson-check").at("sectio…。
        assert(gangyi::agentLessonView(db, access, "lesson-check").at("sections").size() == 3);
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", re…。
        gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", retry.at("id")}});
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：practice.process(db, retry.at("id"));。
    practice.process(db, retry.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto shown，更新当前运行状态。
    const auto shown = gangyi::agentLessonView(db, access, "lesson-check");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.at("sections").back().at("rePracticeOf") == "qid-1");。
    assert(shown.at("sections").back().at("rePracticeOf") == "qid-1");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.dump().find("PRIVATE-") == std::string::npos && shown.du…。
    assert(shown.dump().find("PRIVATE-") == std::string::npos && shown.dump().find("expectedAnswer") == std::string::npos);
    // 【逐行批注】 计算右侧结果并写入 auto retried，更新当前运行状态。
    const auto retried = submit(shown.at("sections").back().at("id"), "question_answer", "7/10，通分后相加");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(retried).at("helpMode") == "independent" && inputFor(…。
    assert(inputFor(retried).at("helpMode") == "independent" && inputFor(retried).at("rePracticeOf") == "qid-1");
    // 【逐行批注】 调用 assert、agentProfileEvidence 完成本行所需操作：assert(gangyi::agentProfileEvidence(db, access).empty());。
    assert(gangyi::agentProfileEvidence(db, access).empty());
    // 【逐行批注】 计算右侧结果并写入 auto retriedInput，更新当前运行状态。
    const auto retriedInput = inputFor(retried); int gradeCalls = 0;
    // 【逐行批注】 定义函数 grader，执行该函数声明的职责。
    gangyi::LearningAgent grader(path.string(), [&](const auto&, const auto& sink) {
        // 【逐行批注】 计算右侧结果并写入 Json actions，更新当前运行状态。
        Json actions = Json::array();
        // 【逐行批注】 判断条件 (++gradeCalls == 1) actions.push_back({{"id", "grade-independent…，成立时执行对应分支。
        if (++gradeCalls == 1) actions.push_back({{"id", "grade-independent"}, {"tool", "evaluate_answer"}, {"args", {
            // 【逐行批注】 执行当前声明或表达式：{"interactionId", retriedInput.at("interactionId")}, {"score", 90}, {"credible", tru…。
            {"interactionId", retriedInput.at("interactionId")}, {"score", 90}, {"credible", true}, {"feedback", "关联新题的计算正确"}, {"feedbackHelpType", "none"},
            // 【逐行批注】 执行当前声明或表达式：{"reason", "依据独立复练的真实回答"}, {"uncertainty", "还需继续观察迁移表现"},。
            {"reason", "依据独立复练的真实回答"}, {"uncertainty", "还需继续观察迁移表现"},
            // 【逐行批注】 执行当前声明或表达式：{"diagnosis", {{"summary", "本次新题计算正确"}, {"evidence", {"7/10"}}, {"teachingDirection"…。
            {"diagnosis", {{"summary", "本次新题计算正确"}, {"evidence", {"7/10"}}, {"teachingDirection", "让学生进一步解释通分理由"}, {"needsClarification", false}}}
        // 【逐行批注】 执行当前声明或表达式：}}});。
        }}});
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", gradeCalls == 1 ? "" : "你独立完成了这道关联新题，再解释一下…。
        return output({{"message", gradeCalls == 1 ? "" : "你独立完成了这道关联新题，再解释一下理由。"},
            // 【逐行批注】 执行当前声明或表达式：{"actions", actions}, {"teaching", {{"helpType", "none"}}}, {"state", gradeCalls == …。
            {"actions", actions}, {"teaching", {{"helpType", "none"}}}, {"state", gradeCalls == 1 ? "continue" : "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(grader); grader.process(db, retried.…。
    gangyi::registerAgentLessonTools(grader); grader.process(db, retried.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto evidence，更新当前运行状态。
    const auto evidence = gangyi::agentProfileEvidence(db, access);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(evidence.size() == 1 && evidence[0].at("helpMode") == "indepen…。
    assert(evidence.size() == 1 && evidence[0].at("helpMode") == "independent" && evidence[0].at("rePracticeOf") == "qid-1");
    // 【逐行批注】 计算右侧结果并写入 auto report，更新当前运行状态。
    const auto report = gangyi::myAIRead(db, "reports", {{"courseId", "math"}});
    // 【逐行批注】 计算右侧结果并写入 bool requestShown，更新当前运行状态。
    bool requestShown = false, followupShown = false, practiceShown = false;
    // 【逐行批注】 按循环规则重复处理：for (const auto& record : report.at("evidence")) {。
    for (const auto& record : report.at("evidence")) {
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(record.at("id") != retriedInput.at("interactionId"));。
        assert(record.at("id") != retriedInput.at("interactionId"));
        // 【逐行批注】 计算右侧结果并写入 requestShown，更新当前运行状态。
        requestShown = requestShown || record.value("inputKind", "") == "teaching_request";
        // 【逐行批注】 计算右侧结果并写入 followupShown，更新当前运行状态。
        followupShown = followupShown || record.value("inputKind", "") == "followup";
        // 【逐行批注】 计算右侧结果并写入 practiceShown，更新当前运行状态。
        practiceShown = practiceShown || (record.value("rePracticeOf", "") == "qid-1" && record.value("assessed", false));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(requestShown && followupShown && practiceShown);。
    assert(requestShown && followupShown && practiceShown);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(report.dump().find("PRIVATE-") == std::string::npos && report.…。
    assert(report.dump().find("PRIVATE-") == std::string::npos && report.dump().find("expectedAnswer") == std::string::npos);
    // 【逐行批注】 计算右侧结果并写入 legacyRow，更新当前运行状态。
    auto legacyRow = *db.getClassroomActivity("lesson-check"); auto legacy = Json::parse(legacyRow.payload);
    // 【逐行批注】 执行当前声明或表达式：legacy["sections"].push_back({{"id", "old-example"}, {"kind", "explanation"}, {"titl…。
    legacy["sections"].push_back({{"id", "old-example"}, {"kind", "explanation"}, {"title", "旧示范"},
        // 【逐行批注】 执行当前声明或表达式：{"body", "旧示范未记录本题关联"}, {"presentation", "example"}, {"version", 1}});。
        {"body", "旧示范未记录本题关联"}, {"presentation", "example"}, {"version", 1}});
    // 【逐行批注】 计算右侧结果并写入 legacyRow.payload，更新当前运行状态。
    legacyRow.payload = legacy.dump(); assert(db.upsert(legacyRow));
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"lessonId", "lesson-check"}, {"…。
    gangyi::agentExposeTask(db, access, {{"lessonId", "lesson-check"}, {"sections", {{{"id", "old-example"}, {"version", 1}}}}});
    // 【逐行批注】 计算右侧结果并写入 auto historical，更新当前运行状态。
    const auto historical = submit("q2", "question_answer", "5/12，继续独立尝试");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(historical).at("helpMode") == "help_unknown");。
    assert(inputFor(historical).at("helpMode") == "help_unknown");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(historical).at("helpHistory").empty());。
    assert(inputFor(historical).at("helpHistory").empty());
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(historical).at("relatedLearning").back().at("associat…。
    assert(inputFor(historical).at("relatedLearning").back().at("association") == "lesson_only");
    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", h…。
    gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", historical.at("id")}});
    // 阅读题题干相同而材料不同，仍是关联的新题；重复同一材料不能伪装成独立复练。
    // 【逐行批注】 计算右侧结果并写入 cleanRow，更新当前运行状态。
    auto cleanRow = *db.getClassroomActivity("lesson-check"); auto clean = Json::parse(cleanRow.payload);
    // 【逐行批注】 计算右侧结果并写入 clean["sections"].erase(clean["sections"].en…，更新当前运行状态。
    clean["sections"].erase(clean["sections"].end()-1); cleanRow.payload = clean.dump(); assert(db.upsert(cleanRow));
    // 【逐行批注】 计算右侧结果并写入 Json materialQuestion，更新当前运行状态。
    Json materialQuestion = {{"question", "概括材料主旨。"}, {"materials", Json::array({{{"text", "节约用水，从随手关紧水龙头做起。"},
        // 【逐行批注】 执行当前声明或表达式：{"standardAnswer", "PRIVATE-MATERIAL"}}})}, {"expectedAnswer", "节约用水"}};。
        {"standardAnswer", "PRIVATE-MATERIAL"}}})}, {"expectedAnswer", "节约用水"}};
    // 【逐行批注】 计算右侧结果并写入 auto originalMaterial，更新当前运行状态。
    const auto originalMaterial = materialQuestion;
    // 【逐行批注】 计算右侧结果并写入 correctedMaterial，更新当前运行状态。
    auto correctedMaterial = materialQuestion; correctedMaterial["materials"][0]["standardAnswer"] = "PRIVATE-CORRECTED";
    // 【逐行批注】 调用 assert、teachingQuestionIdentity 完成本行所需操作：assert(gangyi::teachingQuestionIdentity("math", originalMaterial) == …。
    assert(gangyi::teachingQuestionIdentity("math", originalMaterial) == gangyi::teachingQuestionIdentity("math", correctedMaterial));
    // 【逐行批注】 执行当前声明或表达式：std::string originalId;。
    std::string originalId;
    // 【逐行批注】 定义函数 materialMaker，执行该函数声明的职责。
    gangyi::LearningAgent materialMaker(path.string(), [&](const auto&, const auto& sink) {
        // 【逐行批注】 计算右侧结果并写入 Json section，更新当前运行状态。
        Json section = {{"kind", "question"}, {"title", "阅读材料题"}, {"question", materialQuestion}};
        // 【逐行批注】 判断条件 (!originalId.empty()) section["rePracticeOf"] = originalId;，成立时执行对应分支。
        if (!originalId.empty()) section["rePracticeOf"] = originalId;
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "请依据这份材料回答。"}, {"teaching", {{"helpType", …。
        return output({{"message", "请依据这份材料回答。"}, {"teaching", {{"helpType", "none"}}},
            // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "material-question"}, {"tool", "append_section"}, {"args", {{"l…。
            {"actions", {{{"id", "material-question"}, {"tool", "append_section"}, {"args", {{"lessonId", "lesson-check"}, {"section", section}}}}}},
            // 【逐行批注】 执行当前声明或表达式：{"state", "waiting_student"}}, sink);。
            {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(materialMaker);。
    gangyi::registerAgentLessonTools(materialMaker);
    // 【逐行批注】 计算右侧结果并写入 auto createMaterial，更新当前运行状态。
    const auto createMaterial = submit("q2", "chat", "增加材料题"); materialMaker.process(db, createMaterial.at("id"));
    // 【逐行批注】 计算右侧结果并写入 materialLesson，更新当前运行状态。
    auto materialLesson = gangyi::agentLessonView(db, access, "lesson-check");
    // 【逐行批注】 计算右侧结果并写入 auto originalSection，更新当前运行状态。
    const auto originalSection = materialLesson.at("sections").back().at("id").get<std::string>();
    // 模拟保留旧导入题目标识，后续关系仍使用这个原标识。
    // 【逐行批注】 计算右侧结果并写入 originalId，更新当前运行状态。
    originalId = "legacy-material-id";
    // 【逐行批注】 计算右侧结果并写入 importedRow，更新当前运行状态。
    auto importedRow = *db.getClassroomActivity("lesson-check"); auto importedLesson = Json::parse(importedRow.payload);
    // 【逐行批注】 按循环规则重复处理：for (auto& section : importedLesson["sections"]) if (section.at("id") == o…。
    for (auto& section : importedLesson["sections"]) if (section.at("id") == originalSection) section["questionId"] = originalId;
    // 【逐行批注】 计算右侧结果并写入 importedRow.payload，更新当前运行状态。
    importedRow.payload = importedLesson.dump(); assert(db.upsert(importedRow));
    // 【逐行批注】 计算右侧结果并写入 auto differentMaterial，更新当前运行状态。
    const auto differentMaterial = submit(originalSection, "teaching_request", "再试一道材料题", "retry_question");
    // 【逐行批注】 计算右侧结果并写入 materialQuestion["materials"][0]["text"]，更新当前运行状态。
    materialQuestion["materials"][0]["text"] = "珍惜粮食，从按需取餐开始。"; materialQuestion["expectedAnswer"] = "珍惜粮食";
    // 【逐行批注】 执行当前声明或表达式：materialMaker.process(db, differentMaterial.at("id"));。
    materialMaker.process(db, differentMaterial.at("id"));
    // 【逐行批注】 计算右侧结果并写入 materialLesson，更新当前运行状态。
    materialLesson = gangyi::agentLessonView(db, access, "lesson-check");
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, differentMaterial.at("id")).at("…。
    assert(gangyi::agentView(db, access, differentMaterial.at("id")).at("status") == "waiting_student");
    // 【逐行批注】 计算右侧结果并写入 auto newSection，更新当前运行状态。
    const auto newSection = materialLesson.at("sections").back().at("id").get<std::string>();
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(materialLesson.at("sections").back().at("questionId") != origi…。
    assert(materialLesson.at("sections").back().at("questionId") != originalId);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(materialLesson.at("sections").back().at("rePracticeOf") == ori…。
    assert(materialLesson.at("sections").back().at("rePracticeOf") == originalId);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(materialLesson.dump().find("PRIVATE-") == std::string::npos);。
    assert(materialLesson.dump().find("PRIVATE-") == std::string::npos);
    // 【逐行批注】 计算右侧结果并写入 auto sameMaterial，更新当前运行状态。
    const auto sameMaterial = submit(originalSection, "teaching_request", "再试一道材料题", "retry_question");
    // 【逐行批注】 计算右侧结果并写入 materialQuestion，更新当前运行状态。
    materialQuestion = correctedMaterial; materialMaker.process(db, sameMaterial.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db, access, sameMaterial.at("id")).at("statu…。
    assert(gangyi::agentView(db, access, sameMaterial.at("id")).at("status") == "failed");
    // 【逐行批注】 调用 assert、agentLessonView 完成本行所需操作：assert(gangyi::agentLessonView(db, access, "lesson-check").at("sectio…。
    assert(gangyi::agentLessonView(db, access, "lesson-check").at("sections").size() == materialLesson.at("sections").size());
    // 【逐行批注】 执行当前声明或表达式：originalId.clear();。
    originalId.clear();
    // 【逐行批注】 计算右侧结果并写入 auto repeatedMaterial，更新当前运行状态。
    const auto repeatedMaterial = submit(originalSection, "chat", "再展示同一道材料题");
    // 【逐行批注】 执行当前声明或表达式：materialMaker.process(db, repeatedMaterial.at("id"));。
    materialMaker.process(db, repeatedMaterial.at("id"));
    // 【逐行批注】 计算右侧结果并写入 materialLesson，更新当前运行状态。
    materialLesson = gangyi::agentLessonView(db, access, "lesson-check");
    // 【逐行批注】 计算右侧结果并写入 auto repeatedSection，更新当前运行状态。
    const auto repeatedSection = materialLesson.at("sections").back().at("id").get<std::string>();
    // 【逐行批注】 计算右侧结果并写入 auto materialDemo，更新当前运行状态。
    const auto materialDemo = submit(originalSection, "teaching_request", "看示范", "worked_example");
    // 【逐行批注】 定义函数 materialTeacher，执行该函数声明的职责。
    gangyi::LearningAgent materialTeacher(path.string(), [&](const auto&, const auto& sink) {
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "原材料要求随手关紧水龙头，主旨是节约用水。"},。
        return output({{"message", "原材料要求随手关紧水龙头，主旨是节约用水。"},
            // 【逐行批注】 调用 array 完成本行所需操作：{"teaching", {{"helpType", "worked_example"}}}, {"actions", Json::arr…。
            {"teaching", {{"helpType", "worked_example"}}}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(materialTeacher); materialTeacher.pr…。
    gangyi::registerAgentLessonTools(materialTeacher); materialTeacher.process(db, materialDemo.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto rawDemo，更新当前运行状态。
    const auto rawDemo = Json::parse(db.getClassroomActivity(materialDemo.at("id"))->payload);
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"taskId", materialDemo.at("id")…。
    gangyi::agentExposeTask(db, access, {{"taskId", materialDemo.at("id")}, {"seq", rawDemo.at("seq")}, {"complete", true}});
    // 【逐行批注】 计算右侧结果并写入 auto independentMaterial，更新当前运行状态。
    const auto independentMaterial = submit(newSection, "question_answer", "珍惜粮食");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(independentMaterial).at("helpMode") == "independent");。
    assert(inputFor(independentMaterial).at("helpMode") == "independent");
    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", i…。
    gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", independentMaterial.at("id")}});
    // 【逐行批注】 计算右侧结果并写入 auto repeatedAnswer，更新当前运行状态。
    const auto repeatedAnswer = submit(repeatedSection, "question_answer", "节约用水");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(repeatedAnswer).at("helpMode") == "after_example");。
    assert(inputFor(repeatedAnswer).at("helpMode") == "after_example");
    // 【逐行批注】 调用 assert、inputFor 完成本行所需操作：assert(inputFor(repeatedAnswer).at("helpHistory").size() == 1);。
    assert(inputFor(repeatedAnswer).at("helpHistory").size() == 1);
// 【逐行批注】 结束当前代码块。
}

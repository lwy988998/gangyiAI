// 试卷讲解本地服务测试：不启动 HTTP、不调用模型，验证本机档案可以保存和恢复。
#include "exam_explanation.hpp"
#include "exam_sources.hpp"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {

using gangyi::ExamJson;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

ExamJson page(int order) {
    static const std::string png = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+/8fkAAAAASUVORK5CYII=";
    return {{"name", "第" + std::to_string(order + 1) + "页.png"}, {"mimeType", "image/png"}, {"order", order}, {"dataUrl", png},
            {"pageNumber", order + 1}, {"originalFileIndex", 0}};
}

}  // namespace

int main() {
    const auto path = std::filesystem::temp_directory_path() / "gangyi-exam-explanation-service-test.db";
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
    try {
        ExamJson pages = ExamJson::array();
        for (int index = 0; index < 5; ++index) pages.push_back(page(index));
        const std::string originalPdf = "data:application/pdf;base64,JVBERi0xLjcKb3JpZ2luYWw=";
        ExamJson body = {{"requestId", "service-test-upload"}, {"title", "本地服务测试试卷"}, {"pages", pages}};
        body["originalFiles"] = ExamJson::array({{{"name", "本地服务测试试卷.pdf"}, {"mimeType", "application/pdf"}, {"dataUrl", originalPdf}}});

        gangyi::Database db;
        db.open(path.string());
        db.migrate();
        const auto created = gangyi::examExplanationCreate(db, body);
        const auto id = created.at("id").get<std::string>();
        require(created.at("pages").size() == 5, "创建后页面数量不正确");
        require(!created.at("pages")[0].contains("dataUrl"), "公开会话不应返回页面原始数据");
        bool prematureMessageRejected = false;
        try { gangyi::examExplanationAppendUserMessage(db, id, {{"requestId", "too-early"}, {"text", "先说第一题"}}); }
        catch (const std::invalid_argument&) { prematureMessageRejected = true; }
        require(prematureMessageRejected && gangyi::examExplanationView(db, id).at("messages").empty(),
                "识别期间不能保存消息或取代识别任务");

        const auto rawPage = gangyi::examExplanationPage(db, id, 0);
        require(rawPage.first == "image/png" && rawPage.second.size() > 8, "原始页面没有按 MIME 和字节保存");
        const auto rawFile = gangyi::examExplanationFile(db, id, 0);
        require(rawFile.first == "application/pdf" && rawFile.second.rfind("%PDF-1.7", 0) == 0, "原始 PDF 文件字节没有保存");

        const auto reserved = gangyi::examExplanationReserveTask(db, id, "recognize-request", "recognizing", std::string{});
        require(reserved.value("taskAccepted", false), "识别任务没有成功占用会话");
        const auto bound = gangyi::examExplanationSetTask(db, id, "task-1", "recognizing", "recognize-request");
        require(bound.value("taskAccepted", false), "识别任务没有绑定到会话");

        gangyi::AIResult recognition;
        recognition.content = R"({"title":"测试历史试卷","questions":[{"id":"q1","number":"1","subject":"历史","type":"材料题","stem":"请说明材料中的因果关系。","materials":["材料一"],"subquestions":[],"handwrittenAnswer":"识别答案","handwrittenAnswerConfidence":0.99,"recognitionConfidence":0.95,"pageIndex":0}]})";
        gangyi::examExplanationApplyModel(db, id, "recognize", "recognize-request", recognition, false, "task-1", 0);
        auto afterBatch = gangyi::examExplanationView(db, id);
        require(afterBatch.value("activeRequestId", "") == "recognize-request", "非最后识别批次不应释放请求占用");
        require(afterBatch.at("questions").size() == 1, "第一批识别结果没有保存");
        require(afterBatch.at("questions")[0].value("handwrittenAnswerUncertain", false), "高置信度手写识别也需要人工核对");
        bool duplicateBatchRejected = false;
        try { gangyi::examExplanationApplyModel(db, id, "recognize", "recognize-request", recognition, false, "task-1", 0); }
        catch (const std::runtime_error&) { duplicateBatchRejected = true; }
        require(duplicateBatchRejected, "已经保存的识别批次不能重复提交");
        require(gangyi::examExplanationView(db, id).at("questions").size() == 1, "重复批次改变了已识别题目");
        gangyi::examExplanationFailTask(db, id, "recognize-request");
        const auto failed = gangyi::examExplanationView(db, id);
        require(failed.value("status", "") == "failed" && failed["progress"]["current"] == 4,
                "识别失败后应保留已提交的页面进度");
        bool failedMessageRejected = false;
        try { gangyi::examExplanationAppendUserMessage(db, id, {{"requestId", "failed-input"}, {"text", "下一题"}}); }
        catch (const std::invalid_argument&) { failedMessageRejected = true; }
        require(failedMessageRejected && gangyi::examExplanationView(db, id).at("messages").empty(),
                "识别尚未完成且失败时不能用聊天取代重试");
        db.close();
        db.open(path.string());
        db.migrate();
        const auto retry = gangyi::examExplanationRetryRecognition(db, id, "recognize-retry-1");
        require(retry.value("taskAccepted", false) && !retry.value("duplicate", true), "未能为失败识别预约重试");
        require(retry.at("questions").size() == 1 && retry["progress"]["current"] == 4,
                "重试不应清空已经识别的题目和进度");
        const auto duplicateRetry = gangyi::examExplanationRetryRecognition(db, id, "recognize-retry-1");
        require(duplicateRetry.value("duplicate", false) && !duplicateRetry.value("taskAccepted", true),
                "相同重试请求不能重复排队");
        const auto retryTask = gangyi::examExplanationSetTask(db, id, "task-retry", "recognizing", "recognize-retry-1");
        require(retryTask.value("taskAccepted", false), "未能绑定识别重试任务");
        recognition.content = R"json({"questions":[{"id":"q2","number":"2","subject":"历史","type":"材料题","stem":"选择正确表述。","materials":[],"subquestions":[{"label":"(1)","text":"说明原因","handwrittenAnswer":"错误识别","handwrittenAnswerConfidence":0.8,"referenceAnswer":"参考答案","rubric":"评分依据"},"原始字符串小问",42],"handwrittenAnswer":"错误识别","recognitionConfidence":0.94,"pageIndex":0}]})json";
        bool staleRecognitionRejected = false;
        try { gangyi::examExplanationApplyModel(db, id, "recognize", "recognize-request", recognition, true, "task-1", 4); }
        catch (const std::runtime_error&) { staleRecognitionRejected = true; }
        require(staleRecognitionRejected, "过期识别任务不能覆盖重试结果");
        gangyi::examExplanationApplyModel(db, id, "recognize", "recognize-retry-1", recognition, true, "task-retry", 4);
        afterBatch = gangyi::examExplanationView(db, id);
        require(afterBatch.value("activeRequestId", "") == "", "最后识别批次完成后应释放请求占用");
        require(afterBatch.at("questions").size() == 2, "多批识别结果没有合并");
        require(afterBatch.at("questions")[1].at("pageIndex") == 4, "重试批次的页码应从已保存进度继续");
        ExamJson studyRef = {{"examId", id}, {"questionId", "q2"}, {"revision", 0}, {"part", 0}};
        const auto study = gangyi::examTeachingQuestion(db, studyRef);
        require(study.at("subquestions").size() == 1 && study.at("subquestions")[0].at("text") == "说明原因",
                "导师只能读取所选小问");
        require(study.at("pages")[0].at("index") == 4 && study.dump().find("参考答案") == std::string::npos &&
                study.dump().find("错误识别") == std::string::npos, "导师需要原卷页码和题干，不需要手写答案或评分依据");
        auto invalidStudy = studyRef;
        invalidStudy["part"] = 2;
        bool unavailablePart = false;
        try { (void)gangyi::examTeachingQuestion(db, invalidStudy); }
        catch (const std::invalid_argument&) { unavailablePart = true; }
        require(unavailablePart, "缺失文字的小问必须先核对");
        const auto joined = gangyi::examExplanationUpdate(db, id, {{"questionId", "q1"}, {"pageIndexes", ExamJson::array({0, 4})}});
        require(joined.at("questions")[0].at("pageIndexes") == ExamJson::array({0, 4}) &&
                joined.at("questions")[0].at("sourceRevision") == 1, "跨识别批次的页码校正应更新题目来源版本");
        auto crossedRef = studyRef;
        crossedRef["questionId"] = "q1"; crossedRef["revision"] = 1; crossedRef["part"] = -1;
        require(gangyi::examTeachingQuestion(db, crossedRef).at("pages").size() == 2, "导师原卷定位应完整保留校正页码");
        const auto finishedRetry = gangyi::examExplanationRetryRecognition(db, id, "recognize-retry-1");
        require(finishedRetry.value("duplicate", false) && !finishedRetry.value("taskAccepted", true),
                "重试完成后重复请求不应重新排队");
        bool completedRetryRejected = false;
        try { gangyi::examExplanationRetryRecognition(db, id, "recognize-retry-new"); }
        catch (const std::invalid_argument&) { completedRetryRejected = true; }
        require(completedRetryRejected, "识别完成后不应再次启动识别");

        gangyi::SearchResource confirmedCandidate;
        confirmedCandidate.url = "https://example.org/question-two";
        gangyi::examExplanationAttachSources(db, id, {confirmedCandidate});
        const auto sourceSnapshot = gangyi::examSourceRead(db, id, confirmedCandidate.url, {},
            [](const std::string& url, const gangyi::LibraryCancel&) {
                gangyi::LibraryDocument document;
                document.url = url; document.status = 200; document.mime = "text/plain";
                document.body = "Question two statement with sufficient evidence.\n\nAnswer: A.";
                return document;
            });
        gangyi::examSourceConfirm(db, id, {{"questionId", "q2"}, {"snapshotId", sourceSnapshot.at("id")},
            {"questionSegmentId", "s1"}, {"questionQuote", "Question two statement"},
            {"answerSegmentId", "s2"}, {"answerQuote", "Answer: A"}, {"confirmed", true}});
        require(gangyi::examSourceConfirmations(db, id).at("confirmations")[0].value("gradingEligible", false),
                "校正前来源确认应有效");
        const auto objectEdit = gangyi::examExplanationUpdate(db, id, {{"questionId", "q2"}, {"subquestionIndex", 0}, {"subquestionText", "修正后的小问"}});
        const auto objectSub = objectEdit.at("questions")[1].at("subquestions")[0];
        require(objectSub.at("text") == "修正后的小问" && objectSub.at("label") == "(1)" &&
                objectSub.at("handwrittenAnswer") == "错误识别" && objectSub.at("handwrittenAnswerConfidence") == 0.8 &&
                objectSub.at("referenceAnswer") == "参考答案" && objectSub.at("rubric") == "评分依据",
                "校正对象小问应保留标签、手写答案和评分资料");
        require(objectEdit.at("questions")[1].at("sourceRevision") == 1 && objectEdit.at("questions")[1].at("recognitionStatus") == "edited",
                "校正小问题干必须更新来源版本并标为已编辑");
        bool staleStudy = false;
        try { (void)gangyi::examTeachingQuestion(db, studyRef); }
        catch (const std::invalid_argument&) { staleStudy = true; }
        require(staleStudy, "小问编辑后不能继续用旧版本问导师");
        require(!gangyi::examSourceConfirmations(db, id).at("confirmations")[0].value("gradingEligible", true),
                "校正小问后旧的同卷确认必须失效");
        const auto stringEdit = gangyi::examExplanationUpdate(db, id, {{"questionId", "q2"}, {"subquestionIndex", 1}, {"subquestionText", "修正字符串小问"}});
        require(stringEdit.at("questions")[1].at("subquestions")[1] == "修正字符串小问" &&
                stringEdit.at("questions")[1].at("sourceRevision") == 2, "字符串小问校正或版本递增失败");
        const auto beforeInvalid = stringEdit.at("questions");
        auto rejectSubquestion = [&](const ExamJson& patch) {
            bool rejected = false;
            try { (void)gangyi::examExplanationUpdate(db, id, patch); }
            catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "非法小问校正必须拒绝");
            require(gangyi::examExplanationView(db, id).at("questions") == beforeInvalid, "非法校正不得改变试卷");
        };
        rejectSubquestion({{"questionId", "q2"}, {"subquestionIndex", 0}});
        rejectSubquestion({{"questionId", "q2"}, {"subquestionText", "缺少索引"}});
        rejectSubquestion({{"questionId", "q2"}, {"subquestionIndex", -1}, {"subquestionText", "错误"}});
        rejectSubquestion({{"questionId", "q2"}, {"subquestionIndex", 3}, {"subquestionText", "越界"}});
        rejectSubquestion({{"questionId", "q2"}, {"subquestionIndex", "0"}, {"subquestionText", "错误"}});
        rejectSubquestion({{"questionId", "q2"}, {"subquestionIndex", 0}, {"subquestionText", 1}});
        rejectSubquestion({{"questionId", "q2"}, {"subquestionIndex", 2}, {"subquestionText", "不支持的原始类型"}});
        rejectSubquestion({{"questionId", "q2"}, {"subquestionIndex", 0}, {"subquestionText", std::string(5001, 'x')}});
        rejectSubquestion({{"questionId", "q2"}, {"subquestionIndex", 0}, {"subquestionText", "冲突"}, {"subquestions", ExamJson::array()}});
        rejectSubquestion({{"questionId", "q1"}, {"subquestionIndex", 0}, {"subquestionText", "没有小问"}});

        const auto message = gangyi::examExplanationAppendUserMessage(db, id, {{"requestId", "message-1"}, {"text", "跳过这一题"}});
        require(!message.value("duplicate", true), "第一次消息被错误判为重复");
        const auto firstTask = gangyi::examExplanationSetTask(db, id, "task-message-1", "ready", "message-1");
        require(firstTask.value("taskAccepted", false), "第一条消息没有绑定任务");
        const auto nextMessage = gangyi::examExplanationAppendUserMessage(db, id, {{"requestId", "message-2"}, {"text", "请继续解释材料。"}});
        require(nextMessage.value("previousTaskId", "") == "task-message-1", "新消息没有标记需要取消的旧任务");
        const auto nextReservation = gangyi::examExplanationReserveTask(db, id, "message-2", "ready", std::string("message-2"));
        require(nextReservation.value("taskAccepted", false), "新消息没有占用会话");
        const auto nextTask = gangyi::examExplanationSetTask(db, id, "task-message-2", "ready", "message-2");
        require(nextTask.value("taskAccepted", false), "新任务没有绑定");
        gangyi::AIResult staleReply;
        staleReply.content = R"({"message":"过期回复","artifacts":[],"currentQuestionId":"q1","state":"continue"})";
        bool staleRejected = false;
        try { gangyi::examExplanationApplyModel(db, id, "chat", "message-1", staleReply, true, "task-message-1"); }
        catch (const std::runtime_error&) { staleRejected = true; }
        require(staleRejected, "旧任务的模型回复不应覆盖新消息");
        gangyi::AIResult currentReply;
        currentReply.content = R"({"message":"根据材料可以看出……","artifacts":[],"currentQuestionId":"q1","state":"continue"})";
        gangyi::examExplanationApplyModel(db, id, "chat", "message-2", currentReply, true, "task-message-2");
        const auto duplicate = gangyi::examExplanationAppendUserMessage(db, id, {{"requestId", "message-1"}, {"text", "跳过这一题"}});
        require(duplicate.value("duplicate", false), "重复消息没有幂等返回");
        const auto updated = gangyi::examExplanationUpdate(db, id, {{"questionId", "q1"}, {"text", "修正后的题干"},
            {"materials", ExamJson::array({"修正后的材料"})}, {"subquestions", ExamJson::array({"说明因果关系"})}});
        require(updated.at("questions")[0].at("stem") == "修正后的题干", "题干校正没有保存");
        require(updated.at("questions")[0].at("materials")[0] == "修正后的材料", "材料校正没有保存");
        const auto correctedAnswer = gangyi::examExplanationUpdate(db, id, {{"questionId", "q1"}, {"handwrittenAnswer", "人工修正答案"}});
        require(correctedAnswer.at("questions")[0].at("handwrittenAnswerStatus") == "edited" &&
                !correctedAnswer.at("questions")[0].at("handwrittenAnswerUncertain").get<bool>(), "手写答案修正状态没有保存");
        const auto correctedSecond = gangyi::examExplanationUpdate(db, id, {{"questionId", "q2"}, {"handwrittenAnswer", "用户校正的答案"}});
        const auto& correctedSub = correctedSecond.at("questions")[1].at("subquestions")[0];
        require(!correctedSub.contains("handwrittenAnswer") && !correctedSub.contains("handwrittenAnswerConfidence") &&
                correctedSub.at("text") == "修正后的小问" && correctedSub.at("referenceAnswer") == "参考答案" && correctedSub.at("rubric") == "评分依据",
                "修正手写答案应清除旧识别并保留小问题干与评分资料");
        require(correctedSecond.at("questions")[1].value("sourceRevision", -1) == 2 &&
                correctedSecond.at("questions")[0].value("sourceRevision", -1) == joined.at("questions")[0].at("sourceRevision").get<int>() + 1,
                "手写答案修改不能改变题目来源版本，题干修改应递增版本");
        gangyi::SearchResource source;
        source.title = "候选试卷资料";
        source.url = "https://example.invalid/exam";
        gangyi::examExplanationAttachSources(db, id, {source});
        require(gangyi::examExplanationView(db, id).at("sources")[0].at("match") == "unverified", "候选来源必须保持未核实状态");
        require(gangyi::examExplanationSources(db, id).at("sources")[0].at("url") == source.url, "来源独立接口没有返回搜索结果");
        bool missingSourceSessionRejected = false;
        try { (void)gangyi::examExplanationSources(db, "不存在的试卷"); }
        catch (const std::invalid_argument&) { missingSourceSessionRejected = true; }
        require(missingSourceSessionRejected, "不存在的试卷不能读取来源");
        const auto prompt = gangyi::examExplanationPrompt(db, id, "chat", "继续学习", "prompt-test");
        require(prompt.at("questions")[0].at("stem") == "修正后的题干", "模型上下文必须包含最新题干");
        require(!prompt.at("questions")[1].at("subquestions")[0].contains("handwrittenAnswer"), "模型上下文不能包含过时的小问手写识别");
        require(!prompt.at("pages")[0].contains("recognitionText"), "模型上下文不能包含过时的页面识别摘要");
        require(prompt.at("sources").size() == 1 && prompt.at("sources")[0].at("match") == "unverified" &&
                    prompt.at("sources")[0].at("gradingEligible") == false,
                "模型上下文应包含未核实候选来源，并禁止将其作为批改依据");
        auto reply = [&](const std::string& requestId, const ExamJson& result) {
            gangyi::examExplanationAppendUserMessage(db, id, {{"requestId", requestId}, {"text", "图示请求"}});
            gangyi::examExplanationSetTask(db, id, "task-" + requestId, "ready", requestId);
            gangyi::AIResult response;
            response.content = result.dump();
            gangyi::examExplanationApplyModel(db, id, "chat", requestId, response, true, "task-" + requestId);
        };
        const ExamJson mapOne = {{"root", {{"label", "主题一"}, {"children", ExamJson::array({{{"label", "分支"}}})}}}};
        const ExamJson mapTwo = {{"root", {{"label", "主题二"}}}};
        reply("diagram-create", {{"message", "第一幅：[[artifact:0]] 中间说明 [[artifact:1]] 完成"},
            {"artifacts", ExamJson::array({{{"type", "mindmap"}, {"content", mapOne}}, {{"type", "mindmap"}, {"content", mapTwo}}})}});
        auto diagramView = gangyi::examExplanationView(db, id);
        const auto origin = diagramView.at("messages").back();
        const auto firstId = origin.at("artifactIds")[0].get<std::string>();
        const auto secondId = origin.at("artifactIds")[1].get<std::string>();
        require(firstId != secondId && origin.at("artifactParts").size() == 5 &&
                origin.at("artifactParts")[1].at("id") == firstId && origin.at("artifactParts")[3].at("id") == secondId,
                "多个图示应按回复位置绑定不同标识");
        require(diagramView.at("artifacts")[0].at("originMessageId") == origin.at("id") &&
                diagramView.at("artifacts")[0].at("revision") == 1, "图示缺少原消息或初始版本");
        const auto mapPrompt = gangyi::examExplanationPrompt(db, id, "chat", "修改", "diagram-prompt");
        require(mapPrompt.at("artifacts").size() == 2 && mapPrompt.at("artifacts")[0].at("id") == firstId,
                "模型上下文应包含可修改图示及版本");
        const ExamJson mapUpdated = {{"root", {{"label", "主题一"}, {"children", ExamJson::array({{{"label", "新背景"}}})}}}};
        reply("diagram-update", {{"message", "前文 [[artifact:0]] 后文"}, {"artifacts", ExamJson::array({
            {{"type", "mindmap"}, {"content", mapUpdated}, {"updateId", firstId}, {"baseRevision", 1}}})}});
        diagramView = gangyi::examExplanationView(db, id);
        require(diagramView.at("artifacts")[0].at("revision") == 2 &&
                diagramView.at("artifacts")[0].at("history")[0].at("revision") == 1 &&
                diagramView.at("artifacts")[1].at("revision") == 1 &&
                diagramView.at("messages").back().at("updatedArtifactIds")[0] == firstId,
                "原位修改应递增版本、保留历史且不影响其他图");
        require(diagramView.at("messages").back().at("content") == "前文 原图已更新 后文", "更新标记应由服务端替换并保持文本顺序");
        require(diagramView.at("messages")[4].at("id") == origin.at("id") &&
                diagramView.at("messages")[4].at("artifactIds")[0] == firstId,
                "更新后原消息应保持图示关联");
        auto rejectedUpdate = [&](const std::string& requestId, const ExamJson& artifact) {
            gangyi::examExplanationAppendUserMessage(db, id, {{"requestId", requestId}, {"text", "修改"}});
            gangyi::examExplanationSetTask(db, id, "task-" + requestId, "ready", requestId);
            gangyi::AIResult response;
            response.content = ExamJson({{"message", "完成"}, {"artifacts", ExamJson::array({artifact})}}).dump();
            bool rejected = false;
            try { gangyi::examExplanationApplyModel(db, id, "chat", requestId, response, true, "task-" + requestId); }
            catch (const std::runtime_error&) { rejected = true; }
            require(rejected, "非法图示更新必须拒绝");
        };
        auto otherBody = body;
        otherBody["requestId"] = "other-session-upload";
        const auto otherId = gangyi::examExplanationCreate(db, otherBody).at("id").get<std::string>();
        gangyi::examExplanationReserveTask(db, otherId, "other-diagram", "ready");
        gangyi::examExplanationSetTask(db, otherId, "other-task", "ready", "other-diagram");
        gangyi::AIResult otherResponse;
        otherResponse.content = ExamJson({{"message", "另一个会话的图"}, {"artifacts", ExamJson::array({{{"type", "mindmap"}, {"content", mapTwo}}})}}).dump();
        gangyi::examExplanationApplyModel(db, otherId, "chat", "other-diagram", otherResponse, true, "other-task");
        const auto otherArtifactId = gangyi::examExplanationView(db, otherId).at("artifacts")[0].at("id").get<std::string>();
        rejectedUpdate("diagram-stale", {{"type", "mindmap"}, {"content", mapUpdated}, {"updateId", firstId}, {"baseRevision", 1}});
        rejectedUpdate("diagram-other-session", {{"type", "mindmap"}, {"content", mapUpdated}, {"updateId", otherArtifactId}, {"baseRevision", 1}});
        rejectedUpdate("diagram-wrong-type", {{"type", "flowchart"}, {"content", {{"nodes", ExamJson::array({{{"id", "a"}, {"label", "A"}}})}, {"edges", ExamJson::array()}}}, {"updateId", firstId}, {"baseRevision", 2}});
        rejectedUpdate("diagram-invalid", {{"type", "mindmap"}, {"content", {{"root", {{"label", ""}}}}}, {"updateId", firstId}, {"baseRevision", 2}});
        diagramView = gangyi::examExplanationView(db, id);
        require(diagramView.at("artifacts")[0].at("revision") == 2 && diagramView.at("artifacts")[0].at("history").size() == 1,
                "拒绝非法更新后图示不得改变");
        auto rejectedContent = [&](const std::string& requestId, const std::string& content) {
            gangyi::examExplanationAppendUserMessage(db, id, {{"requestId", requestId}, {"text", "请继续"}});
            gangyi::examExplanationSetTask(db, id, "task-" + requestId, "ready", requestId);
            gangyi::AIResult response;
            response.content = ExamJson({{"message", content}, {"artifacts", ExamJson::array()}}).dump();
            bool rejected = false;
            try { gangyi::examExplanationApplyModel(db, id, "chat", requestId, response, true, "task-" + requestId); }
            catch (const std::runtime_error&) { rejected = true; }
            require(rejected, "空白回复和无对应图示的标记必须拒绝");
        };
        rejectedContent("blank-answer", "  \n  ");
        rejectedContent("missing-artifact", "已更新 [[artifact:0]]");
        db.close();

        gangyi::Database reopened;
        reopened.open(path.string());
        reopened.migrate();
        const auto restored = gangyi::examExplanationView(reopened, id);
        require(restored.at("messages").size() == 13, "重启后消息和当前回复没有恢复");
        require(restored.at("artifacts")[0].at("revision") == 2 && restored.at("artifacts")[0].at("history").size() == 1 &&
                restored.at("messages")[4].at("artifactIds")[0] == firstId, "重启后图示关联、版本和历史没有恢复");
        require(!restored.at("questions")[1].at("subquestions")[0].contains("handwrittenAnswer"), "重启后不应恢复旧手写识别");
        require(restored.at("questions")[1].at("subquestions")[0].at("text") == "修正后的小问" &&
                restored.at("questions")[1].at("subquestions")[0].at("label") == "(1)" &&
                restored.at("questions")[1].at("subquestions")[0].at("referenceAnswer") == "参考答案" &&
                restored.at("questions")[1].at("subquestions")[1] == "修正字符串小问" &&
                restored.at("questions")[1].at("sourceRevision") == 2, "重启后小问校正或来源版本没有恢复");
        require(restored.at("questions")[0].at("recognitionStatus") == "edited", "重启后校正状态没有恢复");
        require(gangyi::examExplanationList(reopened).at("sessions").size() == 2, "重启后会话列表没有恢复");
        require(restored.at("sources")[0].at("match") == "unverified", "重启后候选来源状态没有恢复");
        reopened.close();
        std::filesystem::remove(path, ignored);
        std::cout << "试卷讲解服务持久化测试通过\n";
        return 0;
    } catch (...) {
        std::filesystem::remove(path, ignored);
        throw;
    }
}

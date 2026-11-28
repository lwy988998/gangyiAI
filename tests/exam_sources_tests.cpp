// 用隔离数据库和模拟网页验证试卷来源的逐题核验。
#include "exam_sources.hpp"
#include "exam_explanation.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>

using gangyi::ExamSourceJson;

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

template <typename F> void rejects(F action, const char* message) {
    try { action(); } catch (const std::exception&) { return; }
    throw std::runtime_error(message);
}

void put(gangyi::Database& db, const std::string& id, const std::string& kind, const ExamSourceJson& value) {
    const auto old = db.myAIRecord(id);
    const int version = old ? old->version : 0;
    require(db.saveMyAIRecord({id, kind, value.dump(), gangyi::libraryNow(), version}, version), "保存测试数据失败");
}
}

int main() {
    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto path = std::filesystem::temp_directory_path() / ("gangyi-exam-source-test-" + std::to_string(unique) + ".db");
    try {
        gangyi::Database db;
        db.open(path.string()); db.migrate();
        const std::string examId = "exam-fixture";
        const std::string url = "https://example.org/paper";
        ExamSourceJson exam = {{"currentQuestionId", "q1"}, {"questions", ExamSourceJson::array({
            {{"id", "q1"}, {"stem", "题目一原始内容"}, {"sourceRevision", 0}, {"materials", ExamSourceJson::array({"材料一"})}},
            {{"id", "q2"}, {"stem", "题目二原始内容"}, {"materials", ExamSourceJson::array()}}
        })}};
        put(db, examId, "exam-explanation", exam);
        put(db, examId + ":sources", "exam-source", ExamSourceJson::array({{{"url", url}}}));

        int calls = 0;
        auto fetch = [&](const std::string& target, const gangyi::LibraryCancel&) {
            ++calls;
            require(target == url, "抓取了非候选地址");
            gangyi::LibraryDocument document;
            document.url = target; document.status = 200; document.mime = "text/plain";
            document.body = "题目一原始内容是一个事实。\n\n参考答案：A，理由是材料一。";
            return document;
        };
        rejects([&] { gangyi::examSourceRead(db, examId, "https://other.example/paper", {}, fetch); }, "未列入候选的 URL 不应读取");
        require(calls == 0, "验证候选应发生在联网之前");
        const auto snap = gangyi::examSourceRead(db, examId, url, {}, fetch);
        require(snap.at("segments").size() == 2, "未保存原文段落");
        const auto snapshotId = snap.at("id").get<std::string>();
        require(gangyi::examSourceSnapshot(db, examId, snapshotId).at("digest") == snap.at("digest"), "无法回查快照");
        rejects([&] { gangyi::examSourceSnapshot(db, "other-exam", snapshotId); }, "不得跨会话读取快照");

        ExamSourceJson confirm = {{"questionId", "q1"}, {"snapshotId", snapshotId},
            {"questionSegmentId", "s1"}, {"questionQuote", "题目一原始内容"},
            {"answerSegmentId", "s2"}, {"answerQuote", "参考答案：A"}, {"confirmed", true}};
        auto invalid = confirm; invalid["answerQuote"] = "伪造的答案";
        rejects([&] { gangyi::examSourceConfirm(db, examId, invalid); }, "不能确认不在原文中的答案");
        invalid = confirm; invalid["questionSegmentId"] = "s2";
        rejects([&] { gangyi::examSourceConfirm(db, examId, invalid); }, "题目证据必须来自指定段落");
        invalid = confirm; invalid["confirmed"] = false;
        rejects([&] { gangyi::examSourceConfirm(db, examId, invalid); }, "必须明确确认");
        invalid = confirm; invalid["questionQuote"] = "            ";
        rejects([&] { gangyi::examSourceConfirm(db, examId, invalid); }, "不能以空白内容确认题目");
        require(gangyi::examSourceConfirm(db, examId, confirm).value("gradingEligible", false), "合法确认未生效");
        auto list = gangyi::examSourceConfirmations(db, examId).at("confirmations");
        require(list.size() == 1 && list[0].value("questionId", "") == "q1" && list[0].value("gradingEligible", false), "确认未绑定指定题目");
        auto prompt = gangyi::examExplanationPrompt(db, examId, "chat", "讲评", "test");
        require(prompt.at("confirmedSources").size() == 1 && prompt["confirmedSources"][0]["answerQuote"] == "参考答案：A", "当前题应使用有效确认的原文");
        exam["currentQuestionId"] = "q2";
        put(db, examId, "exam-explanation", exam);
        prompt = gangyi::examExplanationPrompt(db, examId, "chat", "继续", "test");
        require(prompt.at("confirmedSources").empty(), "其他题目的确认不能进入当前题上下文");
        exam["currentQuestionId"] = "q1";
        put(db, examId, "exam-explanation", exam);

        exam["questions"][0]["stem"] = "已修正的题目一";
        exam["questions"][0]["sourceRevision"] = 1;
        put(db, examId, "exam-explanation", exam);
        require(!gangyi::examSourceConfirmations(db, examId)["confirmations"][0].value("gradingEligible", true), "题目修改后确认必须失效");
        prompt = gangyi::examExplanationPrompt(db, examId, "chat", "继续", "test");
        require(prompt.at("confirmedSources").empty(), "失效来源不能进入教学上下文");
        exam["questions"][0]["stem"] = "题目一原始内容";
        exam["questions"][0]["sourceRevision"] = 2;
        put(db, examId, "exam-explanation", exam);
        require(!gangyi::examSourceConfirmations(db, examId)["confirmations"][0].value("gradingEligible", true), "题目改回后旧确认不能复活");
        gangyi::examSourceRevoke(db, examId, {{"questionId", "q1"}, {"sourceUrl", url}});
        require(!gangyi::examSourceConfirmations(db, examId)["confirmations"][0].value("gradingEligible", true), "撤销后不能评分");
        gangyi::examSourceConfirm(db, examId, confirm);

        auto changedFetch = [&](const std::string& target, const gangyi::LibraryCancel&) {
            auto doc = fetch(target, {}); doc.body += "\n\n网页内容已更新。"; return doc;
        };
        const auto changed = gangyi::examSourceRead(db, examId, url, {}, changedFetch);
        require(changed.at("id") != snapshotId, "不同原文应创建新快照");
        require(!gangyi::examSourceConfirmations(db, examId)["confirmations"][0].value("gradingEligible", true), "来源更新后旧确认必须失效");
        rejects([&] { gangyi::examSourceConfirm(db, examId, confirm); }, "旧快照不能重新确认");
        gangyi::examSourceRead(db, examId, url, {}, fetch);
        require(!gangyi::examSourceConfirmations(db, examId)["confirmations"][0].value("gradingEligible", true), "来源改回后旧确认不能复活");

        auto racingFetch = [&](const std::string& target, const gangyi::LibraryCancel&) {
            gangyi::examSourceRead(db, examId, target, {}, changedFetch);
            return fetch(target, {});
        };
        rejects([&] { gangyi::examSourceRead(db, examId, url, {}, racingFetch); }, "较慢的旧抓取不能覆盖新快照");
        auto deniedFetch = [&](const std::string& target, const gangyi::LibraryCancel&) {
            auto doc = fetch(target, {}); doc.error = "login_or_access_required"; doc.status = 403; return doc;
        };
        rejects([&] { gangyi::examSourceRead(db, examId, url, {}, deniedFetch); }, "登录页面不能作为原文快照");

        const auto row = db.myAIRecord(changed.at("id").get<std::string>());
        auto corrupted = ExamSourceJson::parse(row->payload);
        corrupted["segments"][0]["text"] = "被篡改";
        put(db, row->id, row->kind, corrupted);
        rejects([&] { gangyi::examSourceSnapshot(db, examId, row->id); }, "篡改快照必须被发现");
        db.close(); db.open(path.string()); db.migrate();
        require(!gangyi::examSourceConfirmations(db, examId)["confirmations"][0].value("gradingEligible", true), "重启后失效状态应保持");
        db.close();
        std::filesystem::remove(path);
        std::cout << "试卷来源核验测试通过\n";
        return 0;
    } catch (...) {
        std::error_code ignored; std::filesystem::remove(path, ignored);
        throw;
    }
}

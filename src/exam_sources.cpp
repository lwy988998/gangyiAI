#include "exam_sources.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace gangyi {
namespace {
using Json = ExamSourceJson;

std::string field(const Json& object, const char* key, std::size_t maxSize) {
    if (!object.is_object() || !object.contains(key) || !object.at(key).is_string())
        throw std::invalid_argument("来源核验字段缺失");
    const auto result = object.at(key).get<std::string>();
    if (result.empty() || result.size() > maxSize) throw std::invalid_argument("来源核验字段无效");
    return result;
}

Json readRecord(Database& db, const std::string& id, const std::string& kind) {
    const auto record = db.myAIRecord(id);
    if (!record || record->kind != kind) throw std::invalid_argument("来源记录不存在");
    auto value = Json::parse(record->payload, nullptr, false);
    if (value.is_discarded()) throw std::runtime_error("来源记录损坏");
    return value;
}

Json exam(Database& db, const std::string& id) {
    auto value = readRecord(db, id, "exam-explanation");
    if (!value.is_object() || !value.value("questions", Json()).is_array())
        throw std::runtime_error("试卷记录损坏");
    return value;
}

Json question(Database& db, const std::string& examId, const std::string& questionId) {
    const auto value = exam(db, examId);
    Json result;
    int matches = 0;
    for (const auto& item : value.at("questions")) {
        if (item.is_object() && item.value("id", "") == questionId) { result = item; ++matches; }
    }
    if (matches != 1) throw std::invalid_argument("题目不存在或标识重复");
    return result;
}

std::string questionDigest(const Json& item) {
    Json content = Json::object();
    for (const auto* key : {"id", "number", "subject", "type", "stem", "materials", "subquestions", "pageIndex", "sourceRevision"})
        if (item.contains(key)) content[key] = item.at(key);
    return libraryDigest(content.dump());
}

std::string currentId(const std::string& examId, const std::string& url) {
    return "exam-source-current:" + libraryDigest(Json::array({examId, url}).dump());
}

std::string snapshotId(const std::string& examId, const std::string& url, const std::string& digest) {
    return "exam-source-snapshot:" + libraryDigest(Json::array({examId, url, digest}).dump());
}

void requireCandidate(Database& db, const std::string& examId, const std::string& url) {
    if (!db.myAIRecordExists(examId, "exam-explanation")) throw std::invalid_argument("试卷讲解会话不存在");
    const auto record = db.myAIRecord(examId + ":sources");
    if (!record || record->kind != "exam-source") throw std::invalid_argument("该试卷没有搜索候选");
    const auto sources = Json::parse(record->payload, nullptr, false);
    if (!sources.is_array()) throw std::runtime_error("搜索候选记录损坏");
    for (const auto& item : sources)
        if (item.is_object() && item.value("url", "") == url && libraryPublicURL(url)) return;
    throw std::invalid_argument("只能读取本试卷的公开搜索候选");
}

void saveRecord(Database& db, const std::string& id, const std::string& kind, const Json& value) {
    const auto old = db.myAIRecord(id);
    if (old && old->kind != kind) throw std::runtime_error("来源记录类型冲突");
    const int version = old ? old->version : 0;
    if (!db.saveMyAIRecord({id, kind, value.dump(), libraryNow(), version}, version))
        throw std::runtime_error("来源记录已更新，请刷新后重试");
}

Json validatedSnapshot(Database& db, const std::string& examId, const std::string& id) {
    auto snap = readRecord(db, id, "exam-source-snapshot");
    if (!snap.is_object() || snap.value("examId", "") != examId || !snap.value("segments", Json()).is_array())
        throw std::invalid_argument("来源快照不属于本试卷");
    const auto url = snap.value("sourceUrl", "");
    const auto digest = libraryDigest(Json::array({snap.value("finalUrl", ""), snap.at("segments")}).dump());
    if (digest != snap.value("digest", "") || id != snapshotId(examId, url, digest))
        throw std::runtime_error("来源快照校验失败");
    return snap;
}

bool isCurrent(Database& db, const std::string& examId, const Json& snap, const std::string& id) {
    const auto row = db.myAIRecord(currentId(examId, snap.value("sourceUrl", "")));
    if (!row || row->kind != "exam-source-current") return false;
    const auto value = Json::parse(row->payload, nullptr, false);
    return value.is_object() && value.value("examId", "") == examId && value.value("snapshotId", "") == id;
}

int currentVersion(Database& db, const std::string& examId, const Json& snap) {
    const auto row = db.myAIRecord(currentId(examId, snap.value("sourceUrl", "")));
    return row && row->kind == "exam-source-current" ? row->version : 0;
}

std::string segmentText(const Json& snap, const std::string& segmentId) {
    for (const auto& segment : snap.at("segments"))
        if (segment.is_object() && segment.value("id", "") == segmentId && segment.value("text", Json()).is_string())
            return segment.at("text").get<std::string>();
    throw std::invalid_argument("来源原文段落不存在");
}

Json confirmations(Database& db, const std::string& examId) {
    const auto row = db.myAIRecord(examId + ":source-confirmations");
    if (!row) return Json::array();
    if (row->kind != "exam-source-confirmations") throw std::runtime_error("来源核验记录类型冲突");
    const auto value = Json::parse(row->payload, nullptr, false);
    if (!value.is_array()) throw std::runtime_error("来源核验记录损坏");
    return value;
}

bool validConfirmation(Database& db, const std::string& examId, const Json& value) {
    try {
        if (!value.is_object() || value.value("revoked", true)) return false;
        const auto questionId = field(value, "questionId", 160);
        if (questionDigest(question(db, examId, questionId)) != value.value("questionDigest", "")) return false;
        const auto id = field(value, "snapshotId", 200);
        const auto snap = validatedSnapshot(db, examId, id);
        if (!isCurrent(db, examId, snap, id) || currentVersion(db, examId, snap) != value.value("sourceVersion", 0)) return false;
        for (const auto* prefix : {"question", "answer"}) {
            const auto segmentId = field(value, (std::string(prefix) + "SegmentId").c_str(), 100);
            const auto quote = field(value, (std::string(prefix) + "Quote").c_str(), 8000);
            if (segmentText(snap, segmentId).find(quote) == std::string::npos) return false;
        }
        return true;
    } catch (const std::exception&) { return false; }
}
} // namespace

ExamSourceJson examSourceRead(Database& db, const std::string& examId, const std::string& url,
                              const LibraryCancel& cancelled, const ExamSourceFetcher& fetcher) {
    requireCandidate(db, examId, url);
    const auto previous = db.myAIRecord(currentId(examId, url));
    const int previousVersion = previous ? previous->version : 0;
    const auto document = fetcher ? fetcher(url, cancelled) : libraryFetch(url, cancelled);
    if (!document.error.empty() || document.status < 200 || document.status >= 300 || !libraryPublicURL(document.url))
        throw std::runtime_error("来源原文读取失败或需要登录");
    const auto parsed = libraryParseDocument(document, cancelled);
    const auto status = parsed.value("status", "");
    if ((status != "read" && status != "partial") || !parsed.value("segments", Json()).is_array())
        throw std::runtime_error("来源原文无法提取，不能用于确认");
    Json segments = Json::array();
    std::size_t size = 0;
    for (const auto& item : parsed.at("segments")) {
        if (!item.is_object() || !item.value("text", Json()).is_string()) continue;
        const auto text = item.at("text").get<std::string>();
        if (text.empty()) continue;
        size += text.size();
        if (size > 2 * 1024 * 1024 || segments.size() >= 10000) throw std::runtime_error("来源原文过长");
        Json entry = {{"id", "s" + std::to_string(segments.size() + 1)}, {"text", text}};
        for (const auto* key : {"page", "start", "end"}) if (item.contains(key)) entry[key] = item.at(key);
        segments.push_back(std::move(entry));
    }
    if (segments.empty()) throw std::runtime_error("来源没有可核对的文字");
    const auto digest = libraryDigest(Json::array({document.url, segments}).dump());
    const auto id = snapshotId(examId, url, digest);
    Json snap = {{"id", id}, {"examId", examId}, {"sourceUrl", url}, {"finalUrl", document.url},
                 {"digest", digest}, {"checkedAt", libraryNow()}, {"status", status},
                 {"coverage", parsed.value("coverage", "仅包含可提取文字")}, {"segments", segments}};
    db.transaction([&] {
        requireCandidate(db, examId, url);
        const auto current = db.myAIRecord(currentId(examId, url));
        if ((current ? current->version : 0) != previousVersion)
            throw std::runtime_error("来源已被其他操作更新，请重新读取");
        if (const auto old = db.myAIRecord(id)) {
            const auto existing = validatedSnapshot(db, examId, id);
            snap = existing;
        } else saveRecord(db, id, "exam-source-snapshot", snap);
        if (!isCurrent(db, examId, snap, id))
            saveRecord(db, currentId(examId, url), "exam-source-current",
                       {{"examId", examId}, {"sourceUrl", url}, {"snapshotId", id}});
    });
    return snap;
}

ExamSourceJson examSourceSnapshot(Database& db, const std::string& examId, const std::string& id) {
    if (!db.myAIRecordExists(examId, "exam-explanation")) throw std::invalid_argument("试卷讲解会话不存在");
    return validatedSnapshot(db, examId, id);
}

ExamSourceJson examSourceConfirm(Database& db, const std::string& examId, const ExamSourceJson& request) {
    const auto questionId = field(request, "questionId", 160);
    const auto id = field(request, "snapshotId", 200);
    const auto questionSegmentId = field(request, "questionSegmentId", 100);
    const auto questionQuote = field(request, "questionQuote", 8000);
    const auto answerSegmentId = field(request, "answerSegmentId", 100);
    const auto answerQuote = field(request, "answerQuote", 8000);
    if (!request.value("confirmed", false)) throw std::invalid_argument("需要明确确认同题及评分依据");
    const auto substantiveBytes = [](const std::string& quote) {
        return std::count_if(quote.begin(), quote.end(), [](unsigned char c) { return !std::isspace(c); });
    };
    if (substantiveBytes(questionQuote) < 12 || substantiveBytes(answerQuote) == 0)
        throw std::invalid_argument("来源证据摘录过短或为空");
    Json result;
    db.transaction([&] {
        const auto item = question(db, examId, questionId);
        const auto snap = validatedSnapshot(db, examId, id);
        if (!isCurrent(db, examId, snap, id)) throw std::invalid_argument("来源快照已更新，请重新核对");
        if (segmentText(snap, questionSegmentId).find(questionQuote) == std::string::npos ||
            segmentText(snap, answerSegmentId).find(answerQuote) == std::string::npos)
            throw std::invalid_argument("确认摘录不在保存的来源原文中");
        result = {{"questionId", questionId}, {"questionDigest", questionDigest(item)}, {"snapshotId", id},
                  {"sourceUrl", snap.at("sourceUrl")}, {"finalUrl", snap.at("finalUrl")},
                  {"sourceVersion", currentVersion(db, examId, snap)},
                  {"questionSegmentId", questionSegmentId}, {"questionQuote", questionQuote},
                  {"answerSegmentId", answerSegmentId}, {"answerQuote", answerQuote},
                  {"confirmedAt", libraryNow()}, {"revoked", false}, {"match", "user_confirmed"},
                  {"label", "用户确认，非官方"}};
        auto items = confirmations(db, examId);
        Json next = Json::array();
        for (const auto& item : items)
            if (!(item.is_object() && item.value("questionId", "") == questionId && item.value("sourceUrl", "") == snap.at("sourceUrl")))
                next.push_back(item);
        next.push_back(result);
        saveRecord(db, examId + ":source-confirmations", "exam-source-confirmations", next);
    });
    result["gradingEligible"] = true;
    return result;
}

ExamSourceJson examSourceRevoke(Database& db, const std::string& examId, const ExamSourceJson& request) {
    const auto questionId = field(request, "questionId", 160);
    const auto url = field(request, "sourceUrl", 8000);
    db.transaction([&] {
        (void)exam(db, examId);
        auto items = confirmations(db, examId);
        bool found = false;
        for (auto& item : items) if (item.is_object() && item.value("questionId", "") == questionId && item.value("sourceUrl", "") == url) {
            item["revoked"] = true; item["revokedAt"] = libraryNow(); found = true;
        }
        if (!found) throw std::invalid_argument("来源确认记录不存在");
        saveRecord(db, examId + ":source-confirmations", "exam-source-confirmations", items);
    });
    return {{"revoked", true}};
}

ExamSourceJson examSourceConfirmations(Database& db, const std::string& examId) {
    if (!db.myAIRecordExists(examId, "exam-explanation")) throw std::invalid_argument("试卷讲解会话不存在");
    auto items = confirmations(db, examId);
    for (auto& item : items) if (item.is_object()) item["gradingEligible"] = validConfirmation(db, examId, item);
    return {{"confirmations", items}};
}

} // namespace gangyi

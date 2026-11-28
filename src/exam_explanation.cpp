#include "exam_explanation.hpp"
#include "exam_sources.hpp"
#include "structured_reply.hpp"

#include "search_client.hpp"

#include <atomic>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace gangyi {
namespace {

using Json = ExamJson;
std::atomic_uint64_t sequence{0};

std::string now() {
    const auto value = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &value);
#else
    gmtime_r(&value, &utc);
#endif
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

std::string makeId(const char* prefix) {
    return std::string(prefix) + "-" + now() + "-" + std::to_string(++sequence);
}

std::string requiredText(const Json& body, const char* key, std::size_t maxLength) {
    if (!body.contains(key) || !body.at(key).is_string()) throw std::invalid_argument("试卷请求字段缺失");
    const auto value = body.at(key).get<std::string>();
    if (value.empty() || value.size() > maxLength) throw std::invalid_argument("试卷请求字段无效");
    return value;
}

std::string decode64(const std::string& input) {
    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string output;
    unsigned int value = 0;
    int bits = -8;
    for (const unsigned char character : input) {
        if (character == '=') break;
        const auto index = alphabet.find(character);
        if (index == std::string::npos) continue;
        value = (value << 6) | static_cast<unsigned int>(index);
        bits += 6;
        if (bits >= 0) {
            output.push_back(static_cast<char>((value >> bits) & 255));
            bits -= 8;
        }
    }
    return output;
}

std::pair<std::string, std::string> dataUrlBytes(const std::string& dataUrl) {
    const auto comma = dataUrl.find(',');
    if (comma == std::string::npos || dataUrl.rfind("data:", 0) != 0)
        throw std::invalid_argument("试卷页数据格式无效");
    const auto header = dataUrl.substr(5, comma - 5);
    const auto semicolon = header.find(';');
    const auto mime = header.substr(0, semicolon);
    if (mime != "image/png" && mime != "image/jpeg" && mime != "image/webp" && mime != "image/gif" && mime != "image/bmp" && mime != "image/tiff" && mime != "application/pdf")
        throw std::invalid_argument("试卷页格式暂不支持");
    if (header.find(";base64") == std::string::npos) throw std::invalid_argument("试卷页必须使用 base64 数据");
    auto bytes = decode64(dataUrl.substr(comma + 1));
    if (bytes.empty() || bytes.size() > 50 * 1024 * 1024) throw std::invalid_argument("试卷页大小无效");
    const bool png = bytes.size() >= 8 && static_cast<unsigned char>(bytes[0]) == 0x89 && bytes.compare(1, 3, "PNG") == 0;
    const bool jpeg = bytes.size() >= 3 && static_cast<unsigned char>(bytes[0]) == 0xff && static_cast<unsigned char>(bytes[1]) == 0xd8 && static_cast<unsigned char>(bytes[2]) == 0xff;
    const bool webp = bytes.size() >= 12 && bytes.compare(0, 4, "RIFF") == 0 && bytes.compare(8, 4, "WEBP") == 0;
    const bool gif = bytes.size() >= 6 && (bytes.compare(0, 6, "GIF87a") == 0 || bytes.compare(0, 6, "GIF89a") == 0);
    const bool bmp = bytes.size() >= 2 && bytes.compare(0, 2, "BM") == 0;
    const bool tiff = bytes.size() >= 4 && ((bytes.compare(0, 4, "II*\0") == 0) || (bytes.compare(0, 4, "MM\0*") == 0));
    const bool pdf = bytes.size() >= 5 && bytes.compare(0, 5, "%PDF-") == 0;
    if ((mime == "image/png" && !png) || (mime == "image/jpeg" && !jpeg) || (mime == "image/webp" && !webp) || (mime == "image/gif" && !gif) || (mime == "image/bmp" && !bmp) || (mime == "image/tiff" && !tiff) || (mime == "application/pdf" && !pdf))
        throw std::invalid_argument("文件内容与声明格式不匹配");
    return {mime, std::move(bytes)};
}

Json parsePayload(const MyAIRecord& row) {
    auto value = Json::parse(row.payload, nullptr, false);
    if (!value.is_object()) throw std::runtime_error("试卷档案损坏");
    value["_version"] = row.version;
    return value;
}

std::optional<MyAIRecord> get(Database& db, const std::string& id) {
    const auto row = db.myAIRecord(id);
    if (!row || row->kind != "exam-explanation") throw std::invalid_argument("试卷讲解会话不存在");
    return row;
}

void save(Database& db, const Json& value, int version) {
    const auto id = value.at("id").get<std::string>();
    auto payload = value;
    payload.erase("_version");
    if (!db.saveMyAIRecord({id, "exam-explanation", payload.dump(), now(), version}, version))
        throw std::runtime_error("试卷讲解档案已被其他操作更新，请刷新后重试");
}

Json publicPage(const Json& page) {
    Json result = page;
    result.erase("dataUrl");
    return result;
}

Json publicView(const Json& value) {
    Json result = value;
    result.erase("_version");
    if (result.contains("files") && result.at("files").is_array()) for (auto& file : result["files"]) file.erase("dataUrl");
    if (result.contains("pages") && result.at("pages").is_array()) {
        for (auto& page : result["pages"]) page = publicPage(page);
    }
    return result;
}

std::string asString(const Json& value, const char* key, const std::string& fallback = {}) {
    return value.contains(key) && value.at(key).is_string() ? value.at(key).get<std::string>() : fallback;
}

bool artifactNumber(const Json& value, double bound) {
    if (!value.is_number()) return false;
    const auto number = value.get<double>();
    return std::isfinite(number) && std::abs(number) <= bound;
}

std::size_t utf16Length(const std::string& value) {
    std::size_t length = 0;
    for (const unsigned char ch : value) {
        if ((ch & 0xc0) != 0x80) length += (ch & 0xf8) == 0xf0 ? 2 : 1;
    }
    return length;
}

bool linkedArtifact(const Json& value, const Json& artifact) {
    if (!artifact.is_object() || !artifact.value("id", Json()).is_string() ||
        !artifact.value("originMessageId", Json()).is_string() || !artifact.value("revision", Json()).is_number_integer()) return false;
    const auto id = artifact.at("id").get<std::string>();
    const auto origin = artifact.at("originMessageId").get<std::string>();
    for (const auto& message : value.value("messages", Json::array())) {
        if (!message.is_object() || message.value("id", "") != origin || message.value("role", "") != "assistant") continue;
        const auto ids = message.value("artifactIds", Json::array());
        if (!ids.is_array()) return false;
        for (const auto& entry : ids) if (entry.is_string() && entry.get<std::string>() == id) return true;
    }
    return false;
}

std::string uniqueRecordId(const Json& value, const char* collection, const char* prefix) {
    for (;;) {
        const auto candidate = makeId(prefix);
        bool exists = false;
        const auto entries = value.value(collection, Json::array());
        if (entries.is_array()) for (const auto& entry : entries)
            if (entry.is_object() && entry.value("id", "") == candidate) { exists = true; break; }
        if (!exists) return candidate;
    }
}

void validateArtifact(const std::string& type, const Json& value) {
    if (!value.is_object()) throw std::runtime_error("图示数据必须是 JSON 对象");
    if (type == "mindmap") {
        std::size_t count = 0;
        std::function<void(const Json&, int)> visit = [&](const Json& node, int depth) {
            if (!node.is_object() || depth > 8 || ++count > 120) throw std::runtime_error("知识导图层级无效");
            const auto label = asString(node, "label", asString(node, "text"));
            if (label.find_first_not_of(" \t\r\n") == std::string::npos || utf16Length(label) > 180) throw std::runtime_error("知识导图节点无效");
            const auto children = node.value("children", Json::array());
            if (!children.is_array()) throw std::runtime_error("知识导图子节点无效");
            for (const auto& child : children) visit(child, depth + 1);
        };
        if (!value.contains("root")) throw std::runtime_error("知识导图缺少根节点");
        visit(value.at("root"), 0);
    } else if (type == "flowchart") {
        const auto nodes = value.value("nodes", Json());
        const auto edges = value.value("edges", Json());
        if (!nodes.is_array() || nodes.empty() || nodes.size() > 120 || !edges.is_array() || edges.size() > 300)
            throw std::runtime_error("流程图节点或连线无效");
        std::set<std::string> ids;
        for (const auto& node : nodes) {
            if (!node.is_object()) throw std::runtime_error("流程图节点无效");
            const auto id = asString(node, "id"), label = asString(node, "label");
            if (id.empty() || utf16Length(id) > 64 || utf16Length(label) > 180 || !node.value("label", Json()).is_string() || !ids.insert(id).second)
                throw std::runtime_error("流程图节点无效");
        }
        for (const auto& edge : edges) {
            if (!edge.is_object()) throw std::runtime_error("流程图连线无效");
            const auto from = asString(edge, "from"), to = asString(edge, "to");
            if (!ids.count(from) || !ids.count(to) || from == to ||
                (edge.contains("label") && !edge.at("label").is_null() && (!edge.at("label").is_string() || utf16Length(edge.at("label").get<std::string>()) > 100)))
                throw std::runtime_error("流程图连线无效");
        }
    } else if (type == "chart") {
        const auto style = asString(value, "type");
        const auto categories = value.value("categories", Json());
        const auto series = value.value("series", Json());
        if ((style != "line" && style != "bar" && style != "scatter") || !categories.is_array() || categories.empty() || categories.size() > 100 ||
            !series.is_array() || series.empty() || series.size() > 8) throw std::runtime_error("图表格式无效");
        for (const auto& category : categories) if (!category.is_string() && !artifactNumber(category, 1e12)) throw std::runtime_error("图表类别无效");
        for (const auto& item : series) {
            if (!item.is_object() || !item.value("name", Json()).is_string() || !item.value("data", Json()).is_array() || item.at("data").size() != categories.size())
                throw std::runtime_error("图表数据无效");
            for (const auto& number : item.at("data")) if (!artifactNumber(number, 1e12)) throw std::runtime_error("图表数值无效");
        }
    } else if (type == "function") {
        const auto coefficients = value.value("coefficients", Json());
        if (!coefficients.is_array() || coefficients.empty() || coefficients.size() > 4 ||
            !artifactNumber(value.value("xMin", Json()), 1e12) || !artifactNumber(value.value("xMax", Json()), 1e12))
            throw std::runtime_error("函数图数据无效");
        for (const auto& coefficient : coefficients) if (!artifactNumber(coefficient, 1e6)) throw std::runtime_error("函数系数无效");
        const double minimum = value.at("xMin").get<double>(), maximum = value.at("xMax").get<double>();
        if (minimum >= maximum || maximum - minimum > 1e5) throw std::runtime_error("函数范围无效");
        if (value.contains("parameter") && !value.at("parameter").is_null()) {
            const auto& p = value.at("parameter");
            if (!p.is_object() || !p.value("coefficientIndex", Json()).is_number_integer() ||
                p.at("coefficientIndex").get<int>() < 0 || p.at("coefficientIndex").get<int>() >= static_cast<int>(coefficients.size()) ||
                !artifactNumber(p.value("min", Json()), 1e12) || !artifactNumber(p.value("max", Json()), 1e12) ||
                !artifactNumber(p.value("step", Json()), 1e12) || !artifactNumber(p.value("value", Json()), 1e12)) throw std::runtime_error("函数参数无效");
            const auto lo = p.at("min").get<double>(), hi = p.at("max").get<double>(), v = p.at("value").get<double>();
            if (lo >= hi || hi - lo > 1e6 || p.at("step").get<double>() <= 0 || v < lo || v > hi) throw std::runtime_error("函数参数范围无效");
        }
    } else throw std::runtime_error("图示类型不受支持");
}

} // namespace

ExamJson examExplanationList(Database& db) {
    Json sessions = Json::array();
    for (const auto& row : db.myAIRecords("exam-explanation")) {
        try {
            const auto value = parsePayload(row);
            sessions.push_back({{"id", value.value("id", row.id)}, {"title", value.value("title", "试卷讲解")},
                {"status", value.value("status", "ready")}, {"updatedAt", value.value("updatedAt", row.updatedAt)},
                {"progress", value.value("progress", Json::object())}});
        } catch (...) {}
    }
    return {{"sessions", sessions}};
}

ExamJson examExplanationView(Database& db, const std::string& id) {
    const auto row = get(db, id);
    auto result = publicView(parsePayload(*row));
    if (const auto sources = db.myAIRecord(id + ":sources"); sources && sources->kind == "exam-source") {
        const auto stored = Json::parse(sources->payload, nullptr, false);
        if (stored.is_array()) result["sources"] = stored;
    }
    return result;
}

ExamJson examExplanationSources(Database& db, const std::string& id) {
    if (!db.myAIRecordExists(id, "exam-explanation")) throw std::invalid_argument("试卷讲解会话不存在");
    Json sources = Json::array();
    if (const auto stored = db.myAIRecord(id + ":sources"); stored && stored->kind == "exam-source") {
        const auto parsed = Json::parse(stored->payload, nullptr, false);
        if (parsed.is_array()) sources = parsed;
    }
    return {{"sources", sources}};
}

ExamJson examTeachingQuestion(Database& db, const ExamJson& reference) {
    if (!reference.is_object() || reference.size() != 4 ||
        !reference.value("revision", Json()).is_number_integer() ||
        !reference.value("part", Json()).is_number_integer() ||
        reference.at("revision") < 0 || reference.at("revision") > 2147483646 ||
        reference.at("part") < -1 || reference.at("part") >= 50)
        throw std::invalid_argument("试卷题目或校正版本无效");
    const auto examId = requiredText(reference, "examId", 160);
    const auto questionId = requiredText(reference, "questionId", 160);
    const auto value = parsePayload(*get(db, examId));
    const auto& pages = value.at("pages");
    if (pages.empty() || value.at("progress").at("current") != pages.size())
        throw std::invalid_argument("请先完成试卷识别并核对题目");
    const auto& questions = value.at("questions");
    const auto matches = [&](const Json& item) { return item.value("id", "") == questionId; };
    const auto found = std::find_if(questions.begin(), questions.end(), matches);
    if (found == questions.end() || std::find_if(std::next(found), questions.end(), matches) != questions.end())
        throw std::invalid_argument("原卷题目标识缺失或重复");
    const auto& question = *found;
    if (question.value("sourceRevision", 0) != reference.at("revision"))
        throw std::invalid_argument("题目内容已校正，请回到试卷重新选择");
    const auto state = question.value("recognitionStatus", "");
    if (state != "ready" && state != "edited") throw std::invalid_argument("请先核对并校正识别不清的题目");
    const auto stem = requiredText(question, "stem", 30000);
    if (stem.find_first_not_of(" \t\r\n") == std::string::npos) throw std::invalid_argument("请先补齐题干");
    const auto startPage = question.value("pageIndex", -1);
    const auto indexes = question.value("pageIndexes", Json::array({startPage}));
    if (!indexes.is_array() || indexes.empty() || indexes.size() > pages.size())
        throw std::invalid_argument("原卷页码缺失，请回到试卷核对");
    Json locations = Json::array();
    for (const auto& page : indexes) {
        if (!page.is_number_integer() || page < 0 || page >= pages.size())
            throw std::invalid_argument("原卷页码超出范围，请回到试卷核对");
        locations.push_back({{"index", page}, {"name", asString(pages.at(page.get<std::size_t>()), "name")}});
    }

    Json missing = Json::array(), materials = Json::array(), subquestions = Json::array();
    const auto asText = [](const Json& item) {
        if (item.is_string()) return item.get<std::string>();
        if (!item.is_object()) return std::string();
        return asString(item, "text", asString(item, "content", asString(item, "stem")));
    };
    const auto published = question.value("subquestions", Json::array());
    const int part = reference.at("part");
    if (part >= 0 && (part >= static_cast<int>(published.size()) || asText(published.at(part)).empty()))
        throw std::invalid_argument("该小问缺少可用文字，请对照原卷补齐");
    std::size_t bytes = stem.size();
    const auto add = [&](Json& output, const Json& item, const std::string& label) {
        const auto text = asText(item);
        if (text.empty() || bytes + text.size() > 45000) { missing.push_back(label); return; }
        bytes += text.size();
        output.push_back({{"label", label}, {"text", text}});
    };
    for (std::size_t index = 0; index < published.size(); ++index) if (part < 0 || index == static_cast<std::size_t>(part))
        add(subquestions, published.at(index), "小问 " + std::to_string(index + 1));
    const auto sourceMaterials = question.value("materials", Json::array());
    for (std::size_t index = 0; index < sourceMaterials.size(); ++index)
        add(materials, sourceMaterials.at(index), "材料 " + std::to_string(index + 1));
    return {{"title", asString(value, "title", "试卷")}, {"number", asString(question, "number")}, {"subject", asString(question, "subject")},
        {"stem", stem}, {"materials", materials}, {"subquestions", subquestions}, {"missingText", missing},
        {"part", part}, {"pages", locations}, {"recognitionStatus", state}, {"revision", reference.at("revision")}};
}

std::string examTeachingConversationId(const std::string& scopeId, const std::string& courseId, const ExamJson& reference) {
    return "exam-study-" + resourceDigest(Json::array({scopeId, courseId, reference}).dump());
}

ExamJson examExplanationCreate(Database& db, const ExamJson& body) {
    if (!body.contains("pages") || !body.at("pages").is_array() || body.at("pages").empty() || body.at("pages").size() > 80)
        throw std::invalid_argument("请上传 1 至 80 页试卷");
    const auto requestId = body.value("requestId", "");
    if (!requestId.empty() && (!body.contains("requestId") || !body.at("requestId").is_string() || requestId.size() > 160))
        throw std::invalid_argument("上传请求标识无效");
    const auto id = makeId("exam");
    if (!requestId.empty()) for (const auto& previous : db.myAIRecords("exam-explanation")) {
        const auto existing = parsePayload(previous);
        if (existing.value("requestId", "") == requestId) return publicView(existing);
    }
    Json value = {
        {"schemaVersion", 1}, {"id", id}, {"title", body.value("title", "未命名试卷")},
        {"status", "recognizing"}, {"createdAt", now()}, {"updatedAt", now()},
        {"files", Json::array()}, {"pages", Json::array()}, {"questions", Json::array()}, {"messages", Json::array()},
        {"sources", Json::array()}, {"artifacts", Json::array()}, {"progress", {{"current", 0}, {"total", 0}}},
        {"currentQuestionId", ""}, {"activeTaskId", ""}, {"activeRequestId", ""},
        {"latestRequestId", ""}, {"requestId", requestId}
    };
    std::size_t totalBytes = 0;
    if (body.contains("originalFiles") && body.at("originalFiles").is_array()) {
        if (body.at("originalFiles").size() > 20) throw std::invalid_argument("原始文件数量过多");
        for (const auto& item : body.at("originalFiles")) {
            if (!item.is_object()) continue;
            const auto [mime, bytes] = dataUrlBytes(requiredText(item, "dataUrl", 70 * 1024 * 1024));
            totalBytes += bytes.size();
            if (totalBytes > 50 * 1024 * 1024) throw std::invalid_argument("整套试卷文件总大小不能超过 50 MB");
            value["files"].push_back({{"id", makeId("file")}, {"name", item.value("name", "试卷文件")},
                {"mimeType", mime}, {"dataUrl", item.at("dataUrl")}});
        }
    }
    std::size_t pageBytes = 0;
    int order = 0;
    for (const auto& item : body.at("pages")) {
        if (!item.is_object()) throw std::invalid_argument("试卷页格式无效");
        const auto dataUrl = requiredText(item, "dataUrl", 70 * 1024 * 1024);
        const auto [mime, bytes] = dataUrlBytes(dataUrl);
        pageBytes += bytes.size();
        if (pageBytes > 50 * 1024 * 1024) throw std::invalid_argument("试卷页面总大小不能超过 50 MB");
        Json page = {
            {"id", makeId("page")}, {"name", item.value("name", "第" + std::to_string(order + 1) + "页")},
            {"mimeType", item.value("mimeType", mime)}, {"order", order++}, {"dataUrl", dataUrl},
            {"originalFileIndex", item.value("originalFileIndex", 0)}, {"pageNumber", item.value("pageNumber", 1)},
            {"recognitionText", ""}, {"recognitionConfidence", nullptr}, {"recognitionStatus", "pending"}
        };
        value["pages"].push_back(std::move(page));
    }
    value["progress"]["total"] = value["pages"].size();
    db.saveMyAIRecord({id, "exam-explanation", value.dump(), now(), 0}, 0);
    return publicView(value);
}

ExamJson examExplanationUpdate(Database& db, const std::string& id, const ExamJson& body) {
    const auto row = get(db, id);
    auto value = parsePayload(*row);
    const auto questionId = asString(body, "questionId");
    const bool hasStem = body.contains("text");
    const bool hasSubject = body.contains("subject");
    const bool hasType = body.contains("type");
    const bool hasHandwritten = body.contains("handwrittenAnswer");
    const bool hasMaterials = body.contains("materials");
    const bool hasSubquestions = body.contains("subquestions");
    const bool hasSubquestionIndex = body.contains("subquestionIndex");
    const bool hasSubquestionText = body.contains("subquestionText");
    const bool hasPages = body.contains("pageIndexes");
    Json correctedPages = Json::array();
    if (hasPages) {
        const auto& indexes = body.at("pageIndexes");
        const auto pageCount = value.at("pages").size();
        if (!indexes.is_array() || indexes.empty() || indexes.size() > pageCount)
            throw std::invalid_argument("请至少选择一页原卷");
        std::set<std::size_t> unique;
        for (const auto& index : indexes) {
            if (!index.is_number_integer() || index < 0 || index >= pageCount)
                throw std::invalid_argument("原卷页码超出范围");
            unique.insert(index.get<std::size_t>());
        }
        correctedPages = unique;
    }
    if (hasSubquestionIndex != hasSubquestionText || (hasSubquestions && hasSubquestionIndex))
        throw std::invalid_argument("单个小问校正必须提供索引和文字，不能同时替换全部小问");
    if (hasSubquestionIndex && (!body.at("subquestionIndex").is_number_integer() ||
        body.at("subquestionIndex") < 0 || body.at("subquestionIndex") >= 5000 ||
        !body.at("subquestionText").is_string() || body.at("subquestionText").get_ref<const std::string&>().size() > 5000))
        throw std::invalid_argument("小问索引或文字无效");
    if (questionId.empty() || !(hasStem || hasSubject || hasType || hasHandwritten || hasMaterials || hasSubquestions || hasSubquestionIndex || hasPages))
        throw std::invalid_argument("识别修正内容无效");
    for (const auto* key : {"text", "subject", "type", "handwrittenAnswer"})
        if (body.contains(key) && !body.at(key).is_string()) throw std::invalid_argument("识别修正字段必须是文字");
    for (const auto* key : {"materials", "subquestions"}) if (body.contains(key)) {
        if (!body.at(key).is_array() || body.at(key).size() > 50) throw std::invalid_argument("材料或小问格式无效");
        for (const auto& item : body.at(key))
            if (!item.is_string() || item.get<std::string>().size() > 5000) throw std::invalid_argument("材料或小问文字过长");
    }
    std::size_t contentLength = 0;
    for (const auto* key : {"text", "subject", "type", "handwrittenAnswer"})
        if (body.contains(key)) contentLength += body.at(key).get<std::string>().size();
    for (const auto* key : {"materials", "subquestions"}) if (body.contains(key))
        for (const auto& item : body.at(key)) contentLength += item.get<std::string>().size();
    if (hasSubquestionText) contentLength += body.at("subquestionText").get_ref<const std::string&>().size();
    if (contentLength > 30000) throw std::invalid_argument("识别修正内容过长");
    bool found = false;
    for (auto& question : value["questions"]) if (question.value("id", "") == questionId) {
        if (hasStem) question["stem"] = body.at("text");
        if (hasSubject) question["subject"] = body.at("subject");
        if (hasType) question["type"] = body.at("type");
        if (hasHandwritten) {
            question["handwrittenAnswer"] = body.at("handwrittenAnswer");
            question["handwrittenAnswerUncertain"] = false;
            question["handwrittenAnswerStatus"] = "edited";
        }
        if (hasMaterials) question["materials"] = body.at("materials");
        if (hasSubquestions) question["subquestions"] = body.at("subquestions");
        if (hasPages) {
            question["pageIndexes"] = correctedPages;
            question["pageIndex"] = correctedPages.front();
        }
        if (hasSubquestionIndex) {
            if (!question.value("subquestions", Json()).is_array()) throw std::invalid_argument("题目没有可校正的小问");
            const auto index = body.at("subquestionIndex").get<std::size_t>();
            if (index >= question["subquestions"].size()) throw std::invalid_argument("小问索引超出范围");
            auto& subquestion = question["subquestions"][index];
            if (subquestion.is_string()) subquestion = body.at("subquestionText");
            else if (subquestion.is_object()) subquestion["text"] = body.at("subquestionText");
            else throw std::invalid_argument("小问格式不支持校正");
        }
        if (hasHandwritten && question.value("subquestions", Json()).is_array()) {
            for (auto& subquestion : question["subquestions"]) if (subquestion.is_object()) {
                subquestion.erase("handwrittenAnswer");
                subquestion.erase("handwrittenAnswerConfidence");
                subquestion.erase("handwrittenAnswerStatus");
                subquestion.erase("handwrittenAnswerUncertain");
            }
        }
        const bool editedText = hasStem || hasSubject || hasType || hasMaterials || hasSubquestions || hasSubquestionIndex;
        if (editedText) question["recognitionStatus"] = "edited";
        if (editedText || hasPages) {
            const auto revision = question.value("sourceRevision", Json());
            if (!revision.is_null() && (!revision.is_number_integer() || revision.get<int>() < 0 || revision.get<int>() >= 2147483646))
                throw std::runtime_error("题目来源版本无效");
            question["sourceRevision"] = (revision.is_null() ? 0 : revision.get<int>()) + 1;
        }
        found = true;
    }
    if (!found) throw std::invalid_argument("题目不存在");
    value["updatedAt"] = now();
    save(db, value, row->version);
    return publicView(value);
}

ExamJson examExplanationAppendUserMessage(Database& db, const std::string& id, const ExamJson& body) {
    const auto row = get(db, id);
    auto value = parsePayload(*row);
    const auto text = requiredText(body, "text", 16000);
    const auto requestId = requiredText(body, "requestId", 160);
    const auto status = value.value("status", "");
    const auto current = value.value("progress", Json::object()).value("current", 0);
    const auto pageCount = static_cast<int>(value.value("pages", Json::array()).size());
    if (status == "recognizing" || (status == "failed" && current < pageCount))
        throw std::invalid_argument("试卷尚未识别完成，请等待识别或先重试识别");
    for (const auto& message : value["messages"]) if (message.value("requestId", "") == requestId) {
        auto result = publicView(value); result["duplicate"] = true; return result;
    }
    value["messages"].push_back({{"id", uniqueRecordId(value, "messages", "message")}, {"requestId", requestId}, {"role", "user"},
        {"content", text}, {"status", "ready"}, {"createdAt", now()}});
    const auto previousTaskId = value.value("activeTaskId", "");
    value["latestRequestId"] = requestId;
    value["activeRequestId"] = requestId;
    value["activeTaskId"] = "";
    value["updatedAt"] = now();
    save(db, value, row->version);
    auto result = publicView(value); result["duplicate"] = false; result["previousTaskId"] = previousTaskId; return result;
}

ExamJson examExplanationReserveTask(Database& db, const std::string& id, const std::string& requestId,
                                    const std::string& status,
                                    const std::optional<std::string>& expectedLatestRequestId) {
    const auto row = get(db, id);
    auto value = parsePayload(*row);
    if (expectedLatestRequestId && value.value("latestRequestId", "") != *expectedLatestRequestId) {
        auto result = publicView(value); result["taskAccepted"] = false; return result;
    }
    value["activeRequestId"] = requestId;
    value["activeTaskId"] = "";
    value["status"] = status;
    value["updatedAt"] = now();
    save(db, value, row->version);
    auto result = publicView(value); result["taskAccepted"] = true; return result;
}

ExamJson examExplanationSetTask(Database& db, const std::string& id, const std::string& taskId,
                                const std::string& status, const std::string& requestId) {
    const auto row = get(db, id);
    auto value = parsePayload(*row);
    if (!requestId.empty() && value.value("activeRequestId", "") != requestId) {
        auto result = publicView(value); result["taskAccepted"] = false; return result;
    }
    value["activeTaskId"] = taskId;
    value["status"] = status;
    value["updatedAt"] = now();
    save(db, value, row->version);
    auto result = publicView(value); result["taskAccepted"] = true; return result;
}

ExamJson examExplanationRetryRecognition(Database& db, const std::string& id, const std::string& requestId) {
    if (requestId.empty() || requestId.size() > 160) throw std::invalid_argument("重试请求标识无效");
    const auto row = get(db, id);
    auto value = parsePayload(*row);
    if (value.value("lastRecognitionRetryId", "") == requestId) {
        auto result = publicView(value);
        result["taskAccepted"] = false;
        result["duplicate"] = true;
        return result;
    }
    if (value.value("status", "") != "failed" || !value.value("activeRequestId", "").empty() ||
        !value.value("activeTaskId", "").empty())
        throw std::invalid_argument("只有识别失败且没有运行任务时才能重试");
    if (!value.value("pages", Json()).is_array() || !value.value("progress", Json()).is_object() ||
        !value["progress"].value("current", Json()).is_number_integer())
        throw std::runtime_error("试卷识别进度无效，无法安全重试");
    const int current = value["progress"]["current"].get<int>();
    const int total = static_cast<int>(value["pages"].size());
    if (current < 0 || current >= total || current % 4 != 0)
        throw std::invalid_argument("试卷没有可继续的识别批次");
    value["activeRequestId"] = requestId;
    value["activeTaskId"] = "";
    value["lastRecognitionRetryId"] = requestId;
    value["status"] = "recognizing";
    value["updatedAt"] = now();
    save(db, value, row->version);
    auto result = publicView(value);
    result["taskAccepted"] = true;
    result["duplicate"] = false;
    return result;
}

void examExplanationFailTask(Database& db, const std::string& id, const std::string& requestId) {
    const auto row = get(db, id);
    auto value = parsePayload(*row);
    if (value.value("activeRequestId", "") != requestId) return;
    value["activeRequestId"] = "";
    value["activeTaskId"] = "";
    value["status"] = "failed";
    value["updatedAt"] = now();
    save(db, value, row->version);
}

void examExplanationClearTask(Database& db, const std::string& id, const std::string& taskId) {
    const auto row = get(db, id);
    auto value = parsePayload(*row);
    if (value.value("activeTaskId", "") != taskId) return;
    value["activeTaskId"] = "";
    value["updatedAt"] = now();
    save(db, value, row->version);
}

ExamJson examExplanationPrompt(Database& db, const std::string& id, const std::string& action,
                               const std::string& text, const std::string& taskId, int fromPage, int pageLimit) {
    const auto row = get(db, id);
    const auto value = parsePayload(*row);
    Json prompt = {
        {"id", id}, {"action", action}, {"taskId", taskId}, {"title", value.value("title", "未命名试卷")},
        {"questions", value.value("questions", Json::array())}, {"messages", value.value("messages", Json::array())},
        {"sources", value.value("sources", Json::array())}, {"pages", Json::array()},
        {"currentQuestionId", value.value("currentQuestionId", "")}
    };
    prompt["artifacts"] = Json::array();
    const auto storedArtifacts = value.value("artifacts", Json::array());
    if (storedArtifacts.is_array()) for (const auto& artifact : storedArtifacts) if (linkedArtifact(value, artifact)) {
        prompt["artifacts"].push_back({{"id", artifact.at("id")}, {"type", artifact.value("type", "")},
            {"content", artifact.value("content", "")}, {"revision", artifact.at("revision")},
            {"originMessageId", artifact.at("originMessageId")}});
    }
    // 联网候选必须明确保持未核实，不能作为评分依据。
    if (const auto stored = db.myAIRecord(id + ":sources"); stored && stored->kind == "exam-source") {
        auto sources = Json::parse(stored->payload, nullptr, false);
        if (sources.is_array()) prompt["sources"] = std::move(sources);
    }
    auto candidateSources = prompt.value("sources", Json::array());
    prompt["sources"] = Json::array();
    if (candidateSources.is_array()) for (auto source : candidateSources) if (source.is_object()) {
        source["match"] = "unverified";
        source["gradingEligible"] = false;
        prompt["sources"].push_back(std::move(source));
    }
    prompt["confirmedSources"] = Json::array();
    if (action != "recognize" && action != "search") {
        const auto current = value.value("currentQuestionId", "");
        const auto confirmations = examSourceConfirmations(db, id).value("confirmations", Json::array());
        for (const auto& source : confirmations) if (source.is_object() && !current.empty() &&
            source.value("questionId", "") == current && source.value("gradingEligible", false)) {
            prompt["confirmedSources"].push_back(source);
        }
    }
    // 识别阶段需要原页；后续对话只保留页名，避免重复发送整套试卷。
    if (action == "recognize") {
        const auto& pages = value.value("pages", Json::array());
        if (fromPage < 0 || pageLimit < 1 || pageLimit > 8) throw std::invalid_argument("识别批次范围无效");
        for (int index = fromPage; index < static_cast<int>(pages.size()) && index < fromPage + pageLimit; ++index) {
            auto page = pages[index];
            page["batchIndex"] = index - fromPage;
            page.erase("originalFileIndex");
            prompt["pages"].push_back(std::move(page));
        }
    }
    else for (const auto& page : value.value("pages", Json::array()))
        // questions 是唯一可编辑识别层；不再附带可能过时的页面识别摘要。
        prompt["pages"].push_back({{"id", page.value("id", "")}, {"name", page.value("name", "")}});
    if (!text.empty()) prompt["text"] = text;
    return prompt;
}

void examExplanationAttachSources(Database& db, const std::string& id, const std::vector<SearchResource>& resources) {
    (void)get(db, id);
    Json sources = Json::array();
    for (const auto& item : resources) {
        sources.push_back({{"title", item.title}, {"url", item.url}, {"source", item.source},
            {"description", item.description}, {"publishedAt", item.publishedAt}, {"updatedAt", item.updatedAt},
            {"match", "unverified"}});
    }
    const auto recordId = id + ":sources";
    if (const auto previous = db.myAIRecord(recordId))
        db.saveMyAIRecord({recordId, "exam-source", sources.dump(), now(), previous->version}, previous->version);
    else db.saveMyAIRecord({recordId, "exam-source", sources.dump(), now(), 0}, 0);
}

void examExplanationApplyModel(Database& db, const std::string& id, const std::string& action,
                               const std::string& requestId, const AIResult& response, bool finalBatch,
                               const std::string& taskId, int expectedPageOffset) {
    const auto row = get(db, id);
    auto value = parsePayload(*row);
    if (action != "search" && !taskId.empty() && value.value("activeRequestId", "") != requestId)
        throw std::runtime_error("这条回复已被新的学习请求接替");
    Json parsed;
    try {
        parsed = parseStructuredReply(response.content);
    } catch (const Json::exception&) {
        throw StructuredReplyError("试卷回复不是完整的 JSON 对象");
    }
    if (!parsed.is_object()) throw StructuredReplyError("试卷回复应为一个 JSON 对象");
    if (action == "recognize") {
        if (!parsed.value("questions", Json()).is_array() ||
            (parsed.contains("title") && !parsed.at("title").is_string()))
            throw StructuredReplyError("试卷识别需要题目列表和有效标题");
        value["title"] = parsed.value("title", value.value("title", "未命名试卷"));
        Json merged = value.value("questions", Json::array());
        if (merged.size() + parsed.at("questions").size() > 500) throw std::runtime_error("识别结果题目过多");
        int pageOffset = static_cast<int>(value["progress"].value("current", 0));
        if (expectedPageOffset >= 0 && expectedPageOffset != pageOffset)
            throw std::runtime_error("识别批次已处理或不按顺序，未重复保存");
        if (expectedPageOffset >= 0 && finalBatch != (pageOffset + 4 >= static_cast<int>(value["pages"].size())))
            throw std::runtime_error("识别批次结束状态与页面数量不匹配");
        std::set<std::string> questionIds;
        for (const auto& question : merged) questionIds.insert(question.value("id", ""));
        for (const auto& source : parsed.at("questions")) {
            if (!source.is_object()) throw StructuredReplyError("识别结果中的每个题目应为对象");
            Json question = Json::object();
            for (const auto* key : {"id", "number", "subject", "type", "stem", "materials", "subquestions",
                                    "handwrittenAnswer", "handwrittenAnswerConfidence", "referenceAnswer", "rubric",
                                    "recognitionConfidence", "pageIndex"})
                if (source.contains(key)) question[key] = source.at(key);
            if (!question.contains("id") || !question.at("id").is_string() || question.at("id").get<std::string>().empty())
                question["id"] = makeId("question");
            while (!questionIds.insert(question.at("id").get<std::string>()).second) question["id"] = makeId("question");
            if (!question.contains("number") || !question.at("number").is_string()) question["number"] = std::to_string(merged.size() + 1);
            if (!question.contains("stem") || !question.at("stem").is_string()) question["stem"] = "";
            if (!question.contains("materials") || !question.at("materials").is_array()) question["materials"] = Json::array();
            if (!question.contains("subquestions") || !question.at("subquestions").is_array()) question["subquestions"] = Json::array();
            const int batchCount = std::min(4, static_cast<int>(value["pages"].size()) - pageOffset);
            const auto localPage = question.value("pageIndex", Json(0));
            const bool validStart = localPage.is_number_integer() && localPage >= 0 && localPage < batchCount;
            question["pageIndex"] = validStart ? localPage.get<int>() + pageOffset : -1;
            const auto relatedPages = source.value("pageIndexes", Json::array());
            if (validStart && relatedPages.is_array()) {
                std::set<int> indexes{question.at("pageIndex").get<int>()};
                for (const auto& page : relatedPages)
                    if (page.is_number_integer() && page >= 0 && page < batchCount) indexes.insert(page.get<int>() + pageOffset);
                question["pageIndexes"] = indexes;
            }
            const double confidence = question.value("recognitionConfidence", Json()).is_number() ? question.at("recognitionConfidence").get<double>() : 0.5;
            question["recognitionStatus"] = confidence < 0.7 ? "uncertain" : "ready";
            question["sourceRevision"] = 0;
            const bool hasHandwriting = question.value("handwrittenAnswer", Json()).is_string() &&
                !question.at("handwrittenAnswer").get<std::string>().empty();
            question["handwrittenAnswerUncertain"] = hasHandwriting;
            question["handwrittenAnswerStatus"] = hasHandwriting ? "needs_review" : "none";
            merged.push_back(std::move(question));
        }
        value["questions"] = std::move(merged);
        value["status"] = finalBatch ? "ready" : "recognizing";
        value["progress"]["total"] = value["pages"].size();
        value["progress"]["current"] = std::min<std::size_t>(value["pages"].size(), pageOffset + 4);
        if (finalBatch) {
            value["currentQuestionId"] = value["questions"].empty() ? "" : value["questions"][0].value("id", "");
            for (auto& page : value["pages"]) {
                std::string text;
                for (const auto& question : value["questions"]) if (question.value("pageIndex", -1) == page.value("order", -2)) {
                    if (!text.empty()) text += "\n\n";
                    text += question.value("number", "") + " " + question.value("stem", "");
                    for (const auto& material : question.value("materials", Json::array())) if (material.is_string()) text += "\n" + material.get<std::string>();
                    for (const auto& sub : question.value("subquestions", Json::array())) if (sub.is_string()) text += "\n" + sub.get<std::string>();
                    if (question.contains("handwrittenAnswer")) text += "\n手写答案（识别可能不确定）: " + question.value("handwrittenAnswer", "");
                }
                page["recognitionText"] = std::move(text);
                page["recognitionStatus"] = "ready";
            }
        }
    } else {
        if (!parsed.value("message", Json()).is_string() ||
            (parsed.contains("artifacts") && !parsed.at("artifacts").is_array()) ||
            (parsed.contains("currentQuestionId") && !parsed.at("currentQuestionId").is_string()) ||
            (parsed.contains("state") && !parsed.at("state").is_string()))
            throw StructuredReplyError("试卷讲解需要有效的正文及可选图示");
        const auto content = parsed.at("message").get<std::string>();
        const auto messageId = uniqueRecordId(value, "messages", "message");
        Json artifactIds = Json::array();
        Json updatedArtifactIds = Json::array();
        Json artifactParts = Json::array();
        std::vector<std::string> proposedIds;
        {
            if (parsed.value("artifacts", Json()).is_array()) {
                if (parsed.at("artifacts").size() > 10) throw std::runtime_error("单条回复的图示过多");
                if (!value.value("artifacts", Json()).is_array()) value["artifacts"] = Json::array();
                std::set<std::string> touched;
                for (const auto& proposed : parsed.at("artifacts")) {
                    if (!proposed.is_object() || !proposed.value("type", Json()).is_string() || !proposed.contains("content"))
                        throw std::runtime_error("模型图示格式无效，请重新生成");
                    const auto type = proposed.at("type").get<std::string>();
                    if (proposed.at("content").is_string() && proposed.at("content").get_ref<const std::string&>().size() > 60000)
                        throw std::runtime_error("图示内容过长");
                    const auto data = proposed.at("content").is_string() ? Json::parse(proposed.at("content").get<std::string>(), nullptr, false) : proposed.at("content");
                    validateArtifact(type, data);
                    const auto payload = data.dump();
                    if (payload.size() > 60000) throw std::runtime_error("图示内容过长");
                    const auto updateId = asString(proposed, "updateId");
                    if (proposed.contains("updateId") && (!proposed.at("updateId").is_string() || updateId.empty()))
                        throw std::runtime_error("图示更新标识无效");
                    if (updateId.empty()) {
                        if (proposed.contains("baseRevision")) throw std::runtime_error("新图示不能携带原版本");
                        const auto artifactId = uniqueRecordId(value, "artifacts", "artifact");
                        value["artifacts"].push_back({{"id", artifactId}, {"type", type}, {"content", payload},
                            {"revision", 1}, {"originMessageId", messageId}, {"history", Json::array()}});
                        artifactIds.push_back(artifactId);
                        proposedIds.push_back(artifactId);
                    } else {
                        if (!touched.insert(updateId).second || !proposed.value("baseRevision", Json()).is_number_integer())
                            throw std::runtime_error("图示更新版本无效");
                        bool found = false;
                        for (auto& existing : value["artifacts"]) if (existing.is_object() && existing.value("id", "") == updateId) {
                            if (!linkedArtifact(value, existing) || existing.value("type", "") != type ||
                                existing.at("revision") != proposed.at("baseRevision"))
                                throw std::runtime_error("图示更新目标或版本不匹配，请重新提出修改");
                            if (!existing.value("history", Json()).is_array()) throw std::runtime_error("图示历史记录无效");
                            existing["history"].push_back({{"revision", existing.at("revision")}, {"content", existing.at("content")}});
                            existing["content"] = payload;
                            existing["revision"] = existing.at("revision").get<int>() + 1;
                            updatedArtifactIds.push_back(updateId);
                            found = true;
                            break;
                        }
                        if (!found) throw std::runtime_error("当前会话中不存在指定图示，请重新提出修改");
                        proposedIds.emplace_back();
                    }
                }
            }
            if (parsed.value("currentQuestionId", Json()).is_string()) {
                const auto nextQuestionId = parsed.at("currentQuestionId").get<std::string>();
                bool foundQuestion = nextQuestionId.empty();
                for (const auto& question : value.value("questions", Json::array()))
                    if (question.is_object() && question.value("id", "") == nextQuestionId) foundQuestion = true;
                if (!foundQuestion) throw std::runtime_error("模型返回了不存在的题目，请重新生成");
                if (!nextQuestionId.empty()) value["currentQuestionId"] = nextQuestionId;
            }
            if (parsed.value("state", Json()).is_string()) value["conversationState"] = parsed.at("state");
        }
        std::set<std::size_t> positionedIndexes;
        std::set<std::string> positioned;
        std::size_t position = 0;
        std::size_t textStart = 0;
        while (position < content.size()) {
            const auto start = content.find("[[artifact:", position);
            if (start == std::string::npos) break;
            const auto end = content.find("]]", start + 11);
            if (end == std::string::npos) throw std::runtime_error("图示占位符不完整，请重新生成");
            const auto indexText = content.substr(start + 11, end - start - 11);
            const bool digits = !indexText.empty() && indexText.size() <= 2 &&
                std::all_of(indexText.begin(), indexText.end(), [](unsigned char c) { return std::isdigit(c); });
            const auto index = digits ? static_cast<std::size_t>(std::stoul(indexText)) : proposedIds.size();
            if (index >= proposedIds.size() || !positionedIndexes.insert(index).second)
                throw std::runtime_error("图示占位符没有对应内容，请重新生成");
            const auto text = content.substr(textStart, start - textStart);
            if (!text.empty()) artifactParts.push_back({{"type", "text"}, {"text", text}});
            if (proposedIds[index].empty()) artifactParts.push_back({{"type", "text"}, {"text", "原图已更新"}});
            else {
                positioned.insert(proposedIds[index]);
                artifactParts.push_back({{"type", "artifact"}, {"id", proposedIds[index]}});
            }
            textStart = end + 2;
            position = end + 2;
        }
        const auto remaining = content.substr(textStart);
        if (!remaining.empty()) artifactParts.push_back({{"type", "text"}, {"text", remaining}});
        for (const auto& entry : artifactIds) {
            const auto artifactId = entry.get<std::string>();
            if (!positioned.count(artifactId)) artifactParts.push_back({{"type", "artifact"}, {"id", artifactId}});
        }
        std::string displayContent;
        for (const auto& part : artifactParts) if (part.value("type", "") == "text") displayContent += part.value("text", "");
        if (displayContent.find_first_not_of(" \t\r\n") == std::string::npos && artifactIds.empty() && updatedArtifactIds.empty())
            throw std::runtime_error("模型返回了空白回复，请重新生成");
        value["messages"].push_back({{"id", messageId}, {"requestId", requestId}, {"role", "assistant"},
            {"content", displayContent}, {"status", "ready"}, {"sources", response.sources},
            {"model", response.model}, {"createdAt", now()}, {"artifactIds", artifactIds},
            {"updatedArtifactIds", updatedArtifactIds}, {"artifactParts", artifactParts}});
    }
    if (action != "recognize" || finalBatch) {
        value["activeTaskId"] = "";
        value["activeRequestId"] = "";
    }
    value["updatedAt"] = now();
    save(db, value, row->version);
}

std::pair<std::string, std::string> examExplanationPage(Database& db, const std::string& id, int pageIndex) {
    const auto row = get(db, id);
    const auto value = parsePayload(*row);
    if (pageIndex < 0 || !value.value("pages", Json::array()).is_array() ||
        pageIndex >= static_cast<int>(value["pages"].size())) throw std::invalid_argument("试卷页不存在");
    const auto data = dataUrlBytes(value["pages"][pageIndex].value("dataUrl", ""));
    return data;
}

std::pair<std::string, std::string> examExplanationFile(Database& db, const std::string& id, int fileIndex) {
    const auto row = get(db, id);
    const auto value = parsePayload(*row);
    const auto files = value.value("files", Json::array());
    if (fileIndex < 0 || !files.is_array() || fileIndex >= static_cast<int>(files.size()))
        throw std::invalid_argument("试卷原始文件不存在");
    return dataUrlBytes(files[fileIndex].value("dataUrl", ""));
}

} // namespace gangyi

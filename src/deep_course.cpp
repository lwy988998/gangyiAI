#include "deep_course.hpp"
#include "agent_curriculum.hpp"
#include "agent_lessons.hpp"
#include "library_io.hpp"
#include "search_client.hpp"
#include <curl/curl.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>

namespace gangyi {
namespace {
using Json = nlohmann::json;
Json parsed(const std::string& value) {
    const auto result = Json::parse(value, nullptr, false);
    return result.is_object() ? result : Json::object();
}
std::string text(const Json& value, const char* key, size_t limit = 64000) {
    if (!value.is_object() || !value.value(key, Json()).is_string()) throw std::invalid_argument(std::string(key) + "须为文字");
    const auto result = value.at(key).get<std::string>();
    if (result.empty() || result.size() > limit) throw std::invalid_argument(std::string(key) + "为空或过长");
    return result;
}
double number(const Json& value, const char* key, double fallback, double minimum, double maximum) {
    if (!value.contains(key)) return fallback;
    if (!value.at(key).is_number()) throw std::invalid_argument(std::string(key) + "须为数字");
    const double result = value.at(key).get<double>();
    if (!std::isfinite(result) || result < minimum || result > maximum) throw std::invalid_argument(std::string(key) + "超出有效范围");
    return result;
}
int positiveIndex(const Json& value, const char* key) {
    if (!value.contains(key)) return 1;
    long long index = 0;
    if (value.at(key).is_number_integer()) index = value.at(key).get<long long>();
    else if (value.at(key).is_string()) {
        const auto input = value.at(key).get<std::string>();
        if (input.empty() || input.size() > 6 || !std::all_of(input.begin(), input.end(), [](unsigned char c) { return c >= '0' && c <= '9'; }))
            throw std::invalid_argument(std::string(key) + "须为正整数");
        index = std::stoll(input);
    } else throw std::invalid_argument(std::string(key) + "须为正整数");
    if (index < 1 || index > 100000) throw std::invalid_argument(std::string(key) + "超出有效范围");
    return static_cast<int>(index);
}
AgentAccess accessFor(const Json& task) { return {task.at("scopeId"), task.at("courseIds").get<std::vector<std::string>>()}; }
void requireCourse(Database& db, const AgentAccess& access, const std::string& id) {
    const auto course = db.getCourse(id);
    if (!course || course->status != "active" || std::find(access.courseIds.begin(), access.courseIds.end(), id) == access.courseIds.end())
        throw std::invalid_argument("课程不存在或无权读取");
}
Json savedLesson(Database& db, const AgentAccess& access, const std::string& id) {
    const auto row = db.getClassroomActivity(id);
    if (!row || row->kind != "agent-lesson") throw std::invalid_argument("课时不存在");
    auto lesson = parsed(row->payload);
    if (lesson.value("scopeId", "") != access.scopeId) throw std::invalid_argument("无权读取课时");
    requireCourse(db, access, row->courseId);
    if (!deepCourseEnabled(db, row->courseId)) throw std::invalid_argument("该课程使用快速规划课堂");
    return lesson;
}
void validateTarget(const Json& lesson, const Json& task) {
    const auto current = task.at("event").value("lessonId", "");
    if (!current.empty() && current != lesson.at("id")) throw std::invalid_argument("只能修改当前课时的教学素材");
    if (!lesson.value("teachingTaskId", "").empty() && !current.empty() && lesson.at("teachingTaskId") != task.at("id"))
        throw std::invalid_argument("旧任务不能覆盖当前课时素材");
}
PreparedAgentTool updateLesson(Database& db, const Json& task, Json lesson, Json result) {
    const auto access = accessFor(task); const auto id = lesson.at("id").get<std::string>();
    const auto before = savedLesson(db, access, id);
    lesson["updatedAt"] = libraryNow(); lesson["version"] = before.value("version", 1) + 1;
    return {std::move(result), [access, id, before, lesson](Database& connection) {
        if (savedLesson(connection, access, id) != before) throw std::invalid_argument("课时已经更新，请重新读取后重试");
        if (!connection.upsert(ClassroomActivity{id, lesson.at("courseId"), "agent-lesson", lesson.dump(), libraryNow(), 0, 0}))
            throw std::runtime_error("教学素材保存失败");
    }};
}
Json mediaIds(const Json& block) {
    const auto ids = block.value("mediaIds", Json::array());
    if (!ids.is_array() || ids.size() > 32) throw std::invalid_argument("mediaIds须为素材标识列表");
    for (const auto& id : ids) if (!id.is_string()) throw std::invalid_argument("mediaIds只能使用文字标识");
    return ids;
}
Json bindDeepSections(const Json& lesson, Json input) {
    if (!input.is_object()) throw std::invalid_argument("课件内容须为对象");
    const auto find = [&](const std::string& id, bool lecture) -> const Json& {
        for (const auto& section : lesson.at("sections")) if (section.at("id") == id) {
            const auto kind = section.value("kind", "");
            if (kind != "explanation" && (lecture || kind != "summary"))
                throw std::invalid_argument("讲义与课件只能引用本课已保存的公开讲解或小结，不能引用题目私有标准");
            text(section, "body"); return section;
        }
        throw std::invalid_argument("课件引用的讲解板块不属于当前课时，请先append_section保存讲解");
    };
    for (auto& block : input["lecture"]) {
        const auto id = block.value("sectionId", block.value("id", ""));
        const auto& section = find(id, true);
        block["id"] = id; block["sectionId"] = id;
        block["title"] = section.at("title"); block["body"] = section.at("body");
    }
    for (auto& slide : input["slides"]) {
        const auto ids = slide.value("sourceSectionIds", Json::array());
        if (!ids.is_array() || ids.empty() || ids.size() > 20) throw std::invalid_argument("课件须填写本课真实sourceSectionIds");
        std::string body;
        for (const auto& id : ids) { if (!id.is_string()) throw std::invalid_argument("课件来源板块标识无效"); const auto& section = find(id.get<std::string>(), false); if (!body.empty()) body += "\n\n"; body += section.at("body").get<std::string>(); }
        slide["body"] = body;
    }
    return input;
}
std::string encode64(const std::string& input) {
    static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string output; output.reserve((input.size() + 2) / 3 * 4);
    unsigned int value = 0; int bits = -6;
    for (const unsigned char byte : input) { value = (value << 8) | byte; bits += 8; while (bits >= 0) { output += alphabet[(value >> bits) & 63]; bits -= 6; } }
    if (bits > -6) output += alphabet[((value << 8) >> (bits + 8)) & 63];
    while (output.size() % 4) output += '=';
    return output;
}
std::string decode64(const std::string& input) {
    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string output; unsigned int value = 0; int bits = -8;
    for (const char byte : input) { if (byte == '=') break; const auto index = alphabet.find(byte); if (index == std::string::npos) throw std::invalid_argument("图片缓存无效"); value = (value << 6) | static_cast<unsigned int>(index); bits += 6; if (bits >= 0) { output += static_cast<char>((value >> bits) & 255); bits -= 8; } }
    return output;
}
std::string urlPart(const std::string& input) {
    static constexpr char digits[] = "0123456789ABCDEF";
    std::string output;
    for (const unsigned char byte : input) {
        if ((byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') || (byte >= '0' && byte <= '9') || byte == '-' || byte == '_' || byte == '.') output += static_cast<char>(byte);
        else { output += '%'; output += digits[byte >> 4]; output += digits[byte & 15]; }
    }
    return output;
}
std::string imageMime(const std::string& body) {
    if (body.size() >= 8 && body.compare(0, 8, "\x89PNG\r\n\x1a\n", 8) == 0) return "image/png";
    if (body.size() >= 3 && static_cast<unsigned char>(body[0]) == 0xff && static_cast<unsigned char>(body[1]) == 0xd8 && static_cast<unsigned char>(body[2]) == 0xff) return "image/jpeg";
    if (body.size() >= 12 && body.compare(0, 4, "RIFF") == 0 && body.compare(8, 4, "WEBP") == 0) return "image/webp";
    if (body.size() >= 6 && (body.compare(0, 6, "GIF87a") == 0 || body.compare(0, 6, "GIF89a") == 0)) return "image/gif";
    return {};
}
std::string attribute(const std::string& tag, const std::string& name) {
    const std::regex expression("(?:^|[\\s<])" + name + "\\s*=\\s*([\"'])(.*?)\\1", std::regex::icase);
    std::smatch match;
    return std::regex_search(tag, match, expression) ? match[2].str() : std::string();
}
std::string lowerText(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}
std::set<std::string> imageTerms(const std::string& text) {
    static const std::set<std::string> generic = {"the", "and", "for", "with", "from", "this", "that", "image", "picture", "reference", "illustration", "图片", "参考", "插图", "示意", "教学"};
    const auto value = lowerText(text);
    std::set<std::string> result;
    std::string word, previousCJK;
    const auto append = [&] {
        if (word.size() > 4 && word.back() == 's') word.pop_back();
        if (word.size() >= 3 && !generic.count(word)) result.insert(word);
        word.clear();
    };
    for (size_t i = 0; i < value.size();) {
        const auto c = static_cast<unsigned char>(value[i]);
        if (c < 128) {
            if (std::isalnum(c)) word += static_cast<char>(c); else append();
            previousCJK.clear(); ++i; continue;
        }
        append();
        const size_t length = c >= 0xf0 ? 4 : c >= 0xe0 ? 3 : c >= 0xc0 ? 2 : 1;
        if (i + length > value.size()) break;
        // 中文使用相邻二字片段匹配，不把整句或UTF-8的半个字符当成关键词。
        const auto codepoint = length == 3 ? ((c & 15U) << 12) | ((static_cast<unsigned char>(value[i + 1]) & 63U) << 6) | (static_cast<unsigned char>(value[i + 2]) & 63U) : 0U;
        const bool cjk = codepoint >= 0x3400 && codepoint <= 0x9fff;
        const auto character = value.substr(i, length);
        if (cjk && !previousCJK.empty() && !generic.count(previousCJK + character)) result.insert(previousCJK + character);
        previousCJK = cjk ? character : std::string(); i += length;
    }
    append(); return result;
}
std::string imageFigureCaption(const std::string& html, const std::string& lower, size_t position) {
    size_t begin = lower.rfind("<figure", position), end = std::string::npos;
    if (begin != std::string::npos) end = lower.find("</figure", begin);
    if (begin == std::string::npos || end == std::string::npos || end < position || end - begin > 20000) {
        // OpenStax等教材使用div[data-type=figure]；按当前容器的div深度定位，不借用相邻图的说明。
        // npos不能参与max选择，否则会覆盖另一个有效位置。
        const auto doubleQuoted = lower.rfind("data-type=\"figure\"", position), singleQuoted = lower.rfind("data-type='figure'", position);
        begin = doubleQuoted == std::string::npos ? singleQuoted : singleQuoted == std::string::npos ? doubleQuoted : std::max(doubleQuoted, singleQuoted);
        if (begin == std::string::npos) return {};
        begin = lower.rfind('<', begin);
        if (begin == std::string::npos || lower.compare(begin, 4, "<div") != 0) return {};
        const auto fragment = lower.substr(begin, 20000);
        end = std::string::npos;
        const std::regex containers("</?div\\b[^>]{0,4000}>", std::regex::icase);
        int depth = 0;
        for (auto it = std::sregex_iterator(fragment.begin(), fragment.end(), containers); it != std::sregex_iterator(); ++it) {
            depth += it->str().compare(0, 2, "</") == 0 ? -1 : 1;
            if (depth == 0) { end = begin + static_cast<size_t>(it->position()); break; }
        }
        if (end == std::string::npos || end < position) return {};
    }
    auto caption = lower.find("<figcaption", begin);
    std::string closing = "</figcaption";
    if (caption == std::string::npos || caption >= end) {
        caption = lower.find("data-type=\"caption\"", begin);
        if (caption == std::string::npos || caption >= end) caption = lower.find("data-type='caption'", begin);
        closing = "</span";
    }
    if (caption == std::string::npos || caption >= end) return {};
    const auto start = lower.find('>', caption), finish = lower.find(closing, caption);
    if (start == std::string::npos || finish == std::string::npos || finish > end || finish <= start) return {};
    return std::regex_replace(html.substr(start + 1, std::min<size_t>(finish - start - 1, 3000)), std::regex("<[^>]{0,1000}>"), " ");
}
std::string resolveURL(const std::string& base, std::string reference) {
    for (const auto& entity : {std::pair<std::string, std::string>{"&amp;", "&"}, {"&#38;", "&"}}) {
        size_t position = 0; while ((position = reference.find(entity.first, position)) != std::string::npos) { reference.replace(position, entity.first.size(), entity.second); position += entity.second.size(); }
    }
    std::unique_ptr<CURLU, decltype(&curl_url_cleanup)> url(curl_url(), curl_url_cleanup);
    if (!url || curl_url_set(url.get(), CURLUPART_URL, base.c_str(), 0) != CURLUE_OK || curl_url_set(url.get(), CURLUPART_URL, reference.c_str(), 0) != CURLUE_OK) return {};
    char* value = nullptr;
    if (curl_url_get(url.get(), CURLUPART_URL, &value, 0) != CURLUE_OK) return {};
    const std::string result = value; curl_free(value);
    return libraryPublicURL(result) ? result : std::string();
}
Json locateLesson(Database& db, const AgentAccess& access, const Json& scope) {
    if (!scope.value("lessonId", "").empty()) {
        const auto lesson = savedLesson(db, access, text(scope, "lessonId"));
        if ((!scope.value("courseId", "").empty() && scope.at("courseId") != lesson.at("courseId")) ||
            (!scope.value("topicId", "").empty() && scope.at("topicId") != lesson.value("topicId", "")))
            throw std::invalid_argument("课时与当前课程或知识点不匹配");
        return lesson;
    }
    const auto courseId = text(scope, "courseId"); requireCourse(db, access, courseId);
    Json found = Json::object(); const int phase = positiveIndex(scope, "phaseIndex"), topic = positiveIndex(scope, "topicIndex");
    const auto topicId = scope.value("topicId", "");
    if (!topicId.empty()) {
        bool valid = false;
        const auto outline = agentOutline(db, access, courseId);
        for (const auto& stage : outline.at("courseStructure"))
            for (const auto& item : stage.at("topics")) valid = valid || item.at("id") == topicId;
        if (!valid) throw std::invalid_argument("知识点已不在当前课程主线中，请从课程目录重新选择");
    }
    for (const auto& row : db.listClassroomActivities(courseId)) if (row.kind == "agent-lesson") {
        const auto lesson = parsed(row.payload);
        const bool matches = !topicId.empty() ? lesson.value("topicId", "") == topicId : lesson.value("phaseIndex", 0) == phase && lesson.value("topicIndex", 0) == topic;
        if (lesson.value("scopeId", "") != access.scopeId || !matches || lesson.value("intent", "") != "advance") continue;
        if (found.empty() || lesson.value("updatedAt", "") > found.value("updatedAt", "")) found = lesson;
    }
    return found;
}
void recordMediaUsage(Database& db, const Json& task, Json entry) {
    entry["taskId"] = task.at("id"); entry["at"] = libraryNow(); entry["purpose"] = "课堂教学图片";
    const auto id = "library-usage:" + libraryDigest(task.at("id").get<std::string>() + entry.dump() + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    if (!db.saveMyAIRecord({id, "library-usage", entry.dump(), libraryNow(), 1}, 0)) throw std::runtime_error("联网图片请求统计保存失败");
}
Json publicContent(const Json& lesson) {
    auto content = lesson.value("deepContent", Json::object());
    if (!content.is_object() || content.empty()) return nullptr;
    for (auto& media : content["media"]) if (media.value("type", "") == "image") {
        media.erase("blobId");
        if (media.value("status", "") == "ready") media["url"] = "/api/deep-course/image?lessonId=" + urlPart(lesson.at("id")) + "&mediaId=" + urlPart(media.at("id")) + "&v=" + std::to_string(content.value("version", 1));
    }
    return content;
}
}

bool deepCourseEnabled(Database& db, const std::string& courseId) {
    const auto values = db.findSnapshotsByCourseId(courseId);
    if (values.empty()) return false;
    const auto latest = std::max_element(values.begin(), values.end(), [](const auto& a, const auto& b) { return a.version < b.version; });
    return parsed(latest->payload).value("pedagogy", "") == "deep-v1";
}

Json normalizeDeepContent(const Json& input) {
    if (!input.is_object()) throw std::invalid_argument("深度课件须为对象");
    Json result = {{"version", 1}, {"lecture", Json::array()}, {"slides", Json::array()}, {"media", Json::array()}};
    std::set<std::string> identities;
    for (const auto* kind : {"lecture", "slides"}) {
        const auto blocks = input.value(kind, Json::array());
        if (!blocks.is_array() || blocks.size() > 80) throw std::invalid_argument("课件板块数量无效");
        for (const auto& block : blocks) {
            const auto id = text(block, "id", 100);
            if (!identities.insert(std::string(kind) + ":" + id).second) throw std::invalid_argument("课件板块标识重复");
            Json item = {{"id", id}, {"title", text(block, "title", 500)}, {"body", text(block, "body")}, {"mediaIds", mediaIds(block)}};
            if (block.contains("sectionId")) item["sectionId"] = text(block, "sectionId", 160);
            if (block.contains("sourceSectionIds")) {
                const auto ids = block.at("sourceSectionIds");
                if (!ids.is_array() || ids.size() > 20) throw std::invalid_argument("课件来源板块无效");
                for (const auto& source : ids) if (!source.is_string()) throw std::invalid_argument("课件来源须为板块标识");
                item["sourceSectionIds"] = ids;
            }
            result[kind].push_back(std::move(item));
        }
    }
    if (result["lecture"].empty()) throw std::invalid_argument("讲义不能为空");
    const auto media = input.value("media", Json::array());
    if (!media.is_array() || media.size() > 32) throw std::invalid_argument("教学素材列表无效");
    std::set<std::string> mediaNames;
    for (const auto& source : media) {
        const auto id = text(source, "id", 100), type = text(source, "type", 40);
        if (!mediaNames.insert(id).second) throw std::invalid_argument("教学素材标识重复");
        Json item = {{"id", id}, {"type", type}, {"title", text(source, "title", 500)}, {"caption", source.value("caption", "")}, {"status", "ready"}};
        if (source.value("status", "ready") == "failed" || source.value("status", "ready") == "pending") {
            item["status"] = source.at("status"); item["error"] = source.value("error", "该素材尚未完成，可继续学习讲义并重试。");
        } else if (type == "timeline") {
            const auto events = source.value("events", Json::array());
            if (!events.is_array() || events.empty() || events.size() > 100) throw std::invalid_argument("时间轴事件无效");
            item["events"] = Json::array();
            for (const auto& event : events) item["events"].push_back({{"label", text(event, "label", 300)}, {"text", text(event, "text", 12000)}});
        } else if (type == "flowchart") {
            const auto nodes = source.value("nodes", Json::array()), edges = source.value("edges", Json::array());
            if (!nodes.is_array() || nodes.empty() || nodes.size() > 60 || !edges.is_array() || edges.size() > 100) throw std::invalid_argument("流程图节点或连线无效");
            std::set<std::string> names; item["nodes"] = Json::array(); item["edges"] = Json::array();
            for (const auto& node : nodes) { const auto name = text(node, "id", 100); if (!names.insert(name).second) throw std::invalid_argument("流程图节点重复"); item["nodes"].push_back({{"id", name}, {"label", text(node, "label", 1000)}}); }
            for (const auto& edge : edges) { const auto from = text(edge, "from", 100), to = text(edge, "to", 100); if (!names.count(from) || !names.count(to)) throw std::invalid_argument("流程图连线指向不存在的节点"); item["edges"].push_back({{"from", from}, {"to", to}, {"label", edge.value("label", "")}}); }
        } else if (type == "function") {
            const auto coefficients = source.value("coefficients", Json::array());
            if (!coefficients.is_array() || coefficients.size() != 3) throw std::invalid_argument("函数图使用三个二次多项式系数");
            for (const auto& coefficient : coefficients) if (!coefficient.is_number() || !std::isfinite(coefficient.get<double>()) || std::abs(coefficient.get<double>()) > 100000) throw std::invalid_argument("函数系数无效");
            item["coefficients"] = coefficients; item["xMin"] = number(source, "xMin", -10, -10000, 10000); item["xMax"] = number(source, "xMax", 10, -10000, 10000);
            if (item["xMin"].get<double>() >= item["xMax"].get<double>()) throw std::invalid_argument("函数横轴范围无效");
            const auto parameter = source.value("parameter", Json::object());
            if (!parameter.empty()) {
                const auto index = parameter.value("coefficientIndex", 0);
                if (index < 0 || index > 2) throw std::invalid_argument("可调函数系数标识无效");
                const auto minimum = number(parameter, "min", -5, -100000, 100000), maximum = number(parameter, "max", 5, -100000, 100000);
                if (minimum >= maximum) throw std::invalid_argument("参数范围无效");
                item["parameter"] = {{"name", text(parameter, "name", 100)}, {"min", minimum}, {"max", maximum}, {"step", number(parameter, "step", .1, .0001, 10000)}, {"value", number(parameter, "value", coefficients.at(index).get<double>(), minimum, maximum)}, {"coefficientIndex", index}};
            }
        } else if (type == "forces") {
            item["mass"] = number(source, "mass", 1, .001, 100000); item["force"] = number(source, "force", 10, -100000, 100000); item["friction"] = number(source, "friction", 0, 0, 100000); item["gravity"] = number(source, "gravity", 9.8, 0, 1000);
        } else if (type == "image") {
            // 图片地址仅来自已经检索并校验的本课图片缓存，模型不能自造远程地址。
            item["status"] = "pending"; item["error"] = "图片等待联网检索或重新加载。";
        } else throw std::invalid_argument("教学图示类型暂不支持，请使用时间轴、流程图、函数、受力或联网图片");
        result["media"].push_back(std::move(item));
    }
    for (const auto* kind : {"lecture", "slides"}) for (const auto& block : result[kind]) for (const auto& id : block.at("mediaIds"))
        if (!mediaNames.count(id.get<std::string>())) throw std::invalid_argument("课件引用了不存在的素材标识");
    return result;
}

Json deepImageCandidates(const std::string& html, const std::string& pageUrl, const std::string& query, const std::string& title) {
    Json result = Json::array(); std::map<std::string, size_t> seen;
    const std::regex tags("<(?:meta|img)\\b[^>]{1,4000}>", std::regex::icase);
    const auto add = [&](const std::string& raw, const std::string& alt, const std::string& caption, bool metadata) {
        const auto url = resolveURL(pageUrl, raw); auto lower = url;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (url.empty() || lower.find("logo") != std::string::npos || lower.find("avatar") != std::string::npos || lower.find("favicon") != std::string::npos) return;
        if (seen.count(url)) {
            auto& previous = result.at(seen.at(url));
            if (!alt.empty()) previous["alt"] = alt;
            if (!caption.empty()) previous["sourceCaption"] = caption;
            if (!metadata) previous["metadata"] = false;
            return;
        }
        seen[url] = result.size();
        result.push_back({{"url", url}, {"alt", alt}, {"sourceCaption", caption}, {"metadata", metadata}});
    };
    const auto limited = html.substr(0, 1024 * 1024), lower = lowerText(limited);
    int inspected = 0;
    for (auto iterator = std::sregex_iterator(limited.begin(), limited.end(), tags); iterator != std::sregex_iterator() && inspected++ < 160; ++iterator) {
        const auto tag = iterator->str(), property = attribute(tag, "property"), name = attribute(tag, "name");
        if (property == "og:image" || name == "twitter:image") add(attribute(tag, "content"), "", "", true);
        else if (lowerText(tag).compare(0, 4, "<img") == 0) {
            const auto alt = attribute(tag, "alt");
            const auto caption = imageFigureCaption(limited, lower, static_cast<size_t>(iterator->position()));
            if (!alt.empty() || !caption.empty()) add(attribute(tag, "src"), alt, caption, false);
        }
    }
    const auto terms = imageTerms(query + " " + title);
    for (auto& candidate : result) {
        const auto alt = imageTerms(candidate.at("alt")), caption = imageTerms(candidate.at("sourceCaption"));
        // 只计图片自身路径，不借来源页标题给无关配图增加相关度。
        const auto url = candidate.at("url").get<std::string>();
        const auto start = url.find('/', url.find("://") + 3);
        const auto path = imageTerms(start == std::string::npos ? "" : url.substr(start));
        int score = 0; Json matches = Json::array();
        for (const auto& term : terms) if (alt.count(term) || caption.count(term) || path.count(term)) {
            score += alt.count(term) ? 5 : caption.count(term) ? 3 : 1; matches.push_back(term);
        }
        const auto decoration = lowerText(candidate.at("url").get<std::string>() + " " + candidate.at("alt").get<std::string>());
        if (std::regex_search(decoration, std::regex("(^|[^a-z])(cover|hero|banner|thumbnail|icon)([^a-z]|$)"))) score -= 6;
        if (candidate.at("metadata") == true) score -= 3;
        candidate.erase("metadata"); candidate["relevanceScore"] = score; candidate["textMatches"] = matches;
    }
    if (!terms.empty()) {
        std::stable_sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.at("relevanceScore").template get<int>() > b.at("relevanceScore").template get<int>(); });
        result.erase(std::remove_if(result.begin(), result.end(), [](const auto& item) { return item.at("relevanceScore").template get<int>() <= 0; }), result.end());
    }
    if (result.size() > 8) result.erase(result.begin() + 8, result.end());
    return result;
}

Json deepCourseView(Database& db, const AgentAccess& access, const Json& scope) {
    const auto lesson = locateLesson(db, access, scope);
    const auto courseId = lesson.empty() ? text(scope, "courseId") : lesson.at("courseId").get<std::string>();
    requireCourse(db, access, courseId);
    if (!deepCourseEnabled(db, courseId)) return {{"enabled", false}, {"status", "legacy"}};
    const auto course = db.getCourse(courseId);
    Json result = {{"enabled", true}, {"status", lesson.empty() ? "unprepared" : lesson.value("status", "draft")}, {"course", {{"id", courseId}, {"title", course->title}, {"goal", course->goal}, {"pedagogy", "deep-v1"}}}, {"outline", agentOutline(db, access, courseId)}, {"lesson", nullptr}, {"content", nullptr}};
    if (!lesson.empty()) { result["lesson"] = agentLessonView(db, access, lesson.at("id")); result["content"] = publicContent(lesson); }
    return result;
}

Json deepCourseStart(Database& db, const AgentAccess& access, const Json& scope) {
    const auto lesson = locateLesson(db, access, scope);
    if (!lesson.empty()) {
        if (!deepCourseEnabled(db, lesson.at("courseId"))) throw std::invalid_argument("该课使用快速规划课堂");
        if (lesson.value("status", "") == "ready") {
            agentEnterLesson(db, access, {{"lessonId", lesson.at("id")}});
            return {{"lessonId", lesson.at("id")}, {"href", "/deep-course?lessonId=" + lesson.at("id").get<std::string>()}, {"cached", true}};
        }
        const auto taskId = lesson.value("taskId", "");
        if (!taskId.empty()) {
            const auto current = agentView(db, access, taskId);
            const auto row = db.getClassroomActivity(taskId);
            const auto owner = row ? parsed(row->payload) : Json::object();
            const auto ownerEvent = owner.value("event", Json::object());
            const bool unfinishedPlanning = ownerEvent.value("type", "") == "plan_course" && ownerEvent.value("mode", "") == "deep";
            if (!unfinishedPlanning || (current.value("status", "") != "ready" && current.value("status", "") != "failed" &&
                current.value("status", "") != "cancelled" && current.value("status", "") != "superseded")) return {{"task", current}};
            if (lesson.value("sourceLearningVersion", -1) != db.learningRevision())
                throw std::invalid_argument("已有备课草稿对应的学习情况已变化，请重新确认课程主线后继续");
        }
    }
    const auto courseId = lesson.empty() ? text(scope, "courseId") : lesson.at("courseId").get<std::string>(); requireCourse(db, access, courseId);
    if (!deepCourseEnabled(db, courseId)) throw std::invalid_argument("该课使用快速规划课堂");
    const auto outline = agentOutline(db, access, courseId); const int phase = positiveIndex(scope, "phaseIndex"), topicIndex = positiveIndex(scope, "topicIndex");
    Json selected;
    const auto topicId = lesson.empty() ? scope.value("topicId", "") : lesson.value("topicId", "");
    for (const auto& stage : outline.at("courseStructure")) for (const auto& topic : stage.at("topics"))
        if ((!topicId.empty() && topic.at("id") == topicId) ||
            (topicId.empty() && topic.value("legacyPhaseIndex", 0) == phase && topic.value("legacyTopicIndex", 0) == topicIndex)) selected = topic;
    if (selected.empty()) throw std::invalid_argument("知识点不存在，请返回课程目录选择");
    // 同一知识点在课时尚未创建前也使用固定请求身份，跨窗口点击不重复备课。
    const auto requestId = "deep-prepare:" + libraryDigest(courseId + ":" + selected.at("id").get<std::string>() + (lesson.empty() ? "" : ":resume:" + lesson.at("id").get<std::string>()));
    Json event = {{"type", "prepare_next"}, {"courseId", courseId}, {"topicId", selected.at("id")}, {"phaseIndex", selected.at("legacyPhaseIndex")}, {"topicIndex", selected.at("legacyTopicIndex")}, {"requestId", requestId}, {"navigate", false}, {"text", "请按当前课程主线，为指定topicId准备这一课的完整深度讲义与翻页课件；用create_lesson(intent=advance)保存稳定知识点。按教学需要选择图片、时间轴或互动图，不要求每课配齐。先保存可用内容，媒体失败单独保留重试状态；不更改课程主线，不替我作答。"}};
    if (!lesson.empty()) {
        event["lessonId"] = lesson.at("id"); event["resumeDraft"] = true;
        event["text"] = "继续本次事件lessonId指定的已保存深度课时草稿，不创建课程或新课时，不重复已有讲解与题目。先核对现有板块，使用本课真实sectionId补充尚未保存的set_deep_content讲义、课件和适用图示，再finish_lesson。保留已有教学内容与私有题目标准，不替学生作答。";
    }
    return {{"task", agentSubmit(db, access, event)}};
}

Json deepCourseRetryMedia(Database& db, const AgentAccess& access, const Json& scope) {
    const auto lesson = savedLesson(db, access, text(scope, "lessonId"));
    const auto mediaId = text(scope, "mediaId", 100);
    bool found = false; const bool repair = scope.value("repair", false);
    for (const auto& item : lesson.value("deepContent", Json::object()).value("media", Json::array())) if (item.at("id") == mediaId) {
        if (repair && item.value("type", "") != "image") throw std::invalid_argument("图片修复只适用于联网图片");
        if (item.value("status", "") == "ready" && !repair) return {{"cached", true}, {"lessonId", lesson.at("id")}};
        found = true;
    }
    if (!found) throw std::invalid_argument("素材不属于当前课时");
    return {{"task", agentSubmit(db, access, {{"type", "chat"}, {"courseId", lesson.at("courseId")}, {"lessonId", lesson.at("id")}, {"view", "learn"}, {"requestId", text(scope, "requestId", 160)}, {"deepMediaRetry", mediaId}, {"deepImageRepair", repair}, {"text", "只重试本课素材 " + mediaId + "，保留已经生成的讲义、课件和成功素材。图片请调用search_lesson_images指定mediaId；本次图片修复会重新获取实际图片缓存。其他图示修正后调用set_deep_content，避免重复生成整课。此请求不是学生作答，不评分。"}})}};
}

std::pair<std::string, std::string> deepCourseImage(Database& db, const AgentAccess& access, const std::string& lessonId, const std::string& mediaId) {
    const auto lesson = savedLesson(db, access, lessonId);
    for (const auto& media : lesson.value("deepContent", Json::object()).value("media", Json::array())) if (media.value("id", "") == mediaId && media.value("type", "") == "image" && media.value("status", "") == "ready") {
        const auto image = db.myAIRecord(media.value("blobId", ""));
        if (!image || image->kind != "deep-image") throw std::invalid_argument("图片缓存不存在，请重试素材");
        const auto data = parsed(image->payload);
        if (data.value("courseId", "") != lesson.at("courseId")) throw std::invalid_argument("图片不属于本课");
        const auto bytes = decode64(data.at("data"));
        if (imageMime(bytes).empty()) throw std::invalid_argument("图片缓存格式无效");
        return {imageMime(bytes), bytes};
    }
    throw std::invalid_argument("图片不存在或尚未完成");
}

void registerDeepCourseTools(LearningAgent& agent) {
    agent.registerTool("set_deep_content", {u8"保存当前深度课时的真实AI讲义、翻页课件与受控图示。参数 lessonId,content:{lecture:[{sectionId,mediaIds:[]}],slides:[{id,title,sourceSectionIds:[],mediaIds:[]}],media:[{id,type,title,caption,...}]}。讲义的sectionId必须使用append_section返回的本课explanation板块ID，标题正文由程序读取已保存原文；课件sourceSectionIds引用本课已保存公开讲解或小结，正文由这些板块组合。先保存完整真实讲解再组织课件，不能假借私有题目或另一课时的板块。课件按教学需要组织而非切碎长文。图示类型及字段：timeline.events:[{label,text}]；flowchart.nodes:[{id,label}],edges:[{from,to,label}]；function.coefficients:[a,b,c]表示a*x²+b*x+c,xMin,xMax,parameter可选{name,min,max,step,value,coefficientIndex:0|1|2}；forces.mass(kg),force(N),friction(最大可抵消阻力N，静止起步的简化模型，不模拟速度位移)；image只使用search_lesson_images已保存的同一素材id，不能自造地址。未完成图示可写status:failed,error。题目及标准仍通过append_section保存，私有答案不能放入讲义或课件。重试保留已有成功媒体，完成后finish_lesson。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            auto lesson = savedLesson(db, accessFor(task), text(args, "lessonId")); validateTarget(lesson, task);
            const auto previous = lesson.value("deepContent", Json::object());
            auto input = args.at("content");
            const auto retryId = task.at("event").value("deepMediaRetry", "");
            if (!retryId.empty()) {
                auto preserved = previous; bool replaced = false;
                for (const auto& requested : input.value("media", Json::array())) if (requested.value("id", "") == retryId)
                    for (auto& item : preserved["media"]) if (item.value("id", "") == retryId) { item = requested; replaced = true; }
                if (!replaced) throw std::invalid_argument("只重试指定素材时须提交该mediaId的新结构");
                input = std::move(preserved);
            }
            auto content = normalizeDeepContent(bindDeepSections(lesson, input));
            for (auto& item : content["media"]) if (item.at("type") == "image") for (const auto& old : previous.value("media", Json::array()))
                if (old.value("id", "") == item.at("id") && old.value("type", "") == "image") item = old;
            content["version"] = previous.value("version", 0) + 1; content["model"] = source.model; content["generatedAt"] = libraryNow();
            lesson["deepContent"] = content;
            return updateLesson(db, task, lesson, {{"lessonId", lesson.at("id")}, {"contentVersion", content.at("version")}, {"lectureCount", content.at("lecture").size()}, {"slideCount", content.at("slides").size()}});
        }});
    agent.registerTool("search_lesson_images", {u8"围绕本课具体教学用途检索联网图片，复用现有双搜索和安全原文读取。参数lessonId,query,title,mediaId（稳定标识），可选refresh:true。检索、图片二进制和来源一起保存；失败只记录该素材failed，已成功的讲义与图示保持。图片内容由检索来源提供，不把logo或猜测地址当教学图。返回已保存媒体，set_deep_content可按mediaId引用。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult&) {
            auto lesson = savedLesson(db, accessFor(task), text(args, "lessonId")); validateTarget(lesson, task);
            const auto mediaId = text(args, "mediaId", 100), query = text(args, "query", 1000), title = text(args, "title", 500);
            const auto retryId = task.at("event").value("deepMediaRetry", "");
            if (!retryId.empty() && retryId != mediaId) throw std::invalid_argument("只能重试当前指定的素材");
            const bool refresh = args.value("refresh", false) || task.at("event").value("deepImageRepair", false);
            auto content = lesson.value("deepContent", Json{{"version", 0}, {"lecture", Json::array()}, {"slides", Json::array()}, {"media", Json::array()}});
            for (const auto& item : content.at("media")) if (item.value("id", "") == mediaId && item.value("status", "") == "ready" && !refresh) return PreparedAgentTool{{{"media", item}, {"cached", true}}, {}};
            Json media = {{"id", mediaId}, {"type", "image"}, {"title", title}, {"caption", title}, {"status", "failed"}, {"error", "图片暂未获取，可继续学习并重试。"}, {"query", query}};
            Json image = Json::object(); std::string imageId;
            const auto cancelled = [&] {
                if (AIActivity::paused() || AIActivity::stopping() || db.learningRevision() != task.at("learningVersion")) return true;
                const auto row = db.getClassroomActivity(task.at("id"));
                return !row || parsed(row->payload).value("status", "") != "running";
            };
            ResourceQuery request; request.query = query; request.count = 6; request.refresh = refresh; request.cancelled = cancelled; request.scopeId = task.at("scopeId").get<std::string>() + ":" + lesson.at("courseId").get<std::string>();
            const auto search = SearchClient().query(request);
            for (const auto& provider : search.providers) recordMediaUsage(db, task, {{"kind", "search"}, {"provider", provider.at("provider")}, {"status", provider.at("status")}, {"attempts", provider.at("attempts")}, {"cached", provider.at("cached")}, {"shared", provider.value("shared", false)}});
            const auto fetch = [&](const std::string& url) {
                auto document = libraryFetch(url, cancelled);
                recordMediaUsage(db, task, {{"kind", "fetch"}, {"status", document.error.empty() ? "completed" : document.error}, {"attempts", document.attempts}, {"cached", false}});
                return document;
            };
            int inspected = 0;
            for (const auto& resource : search.resources) {
                if (cancelled() || inspected++ >= 3) break;
                const auto document = fetch(resource.url);
                if (!document.error.empty()) continue;
                auto candidates = deepImageCandidates(document.body, document.url, query, title);
                if (!imageMime(document.body).empty()) candidates.insert(candidates.begin(), Json{{"url", document.url}, {"alt", resource.title}});
                int attempts = 0;
                for (const auto& candidate : candidates) {
                    if (attempts++ >= 2 || cancelled()) break;
                    const auto picture = candidate.at("url") == document.url ? document : fetch(candidate.at("url"));
                    const auto mime = imageMime(picture.body);
                    if (!picture.error.empty() || mime.empty() || picture.body.size() > 3 * 1024 * 1024) continue;
                    imageId = "deep-image:" + libraryDigest(lesson.at("courseId").get<std::string>() + picture.url + picture.body);
                    image = {{"data", encode64(picture.body)}, {"mime", mime}, {"url", picture.url}, {"courseId", lesson.at("courseId")}};
                    media = {{"id", mediaId}, {"type", "image"}, {"title", title}, {"caption", title}, {"status", "ready"}, {"sourceUrl", document.url}, {"sourceTitle", resource.title}, {"alt", candidate.value("alt", title)}, {"sourceCaption", candidate.value("sourceCaption", "")}, {"selectionBasis", "source_text_match"}, {"blobId", imageId}, {"query", query}};
                    break;
                }
                if (!image.empty()) break;
            }
            auto& mediaList = content["media"]; bool replaced = false;
            for (auto& old : mediaList) if (old.value("id", "") == mediaId) { old = media; replaced = true; }
            if (!replaced) { if (mediaList.size() >= 32) throw std::invalid_argument("本课素材数量过多"); mediaList.push_back(media); }
            content["version"] = content.value("version", 0) + 1; lesson["deepContent"] = content;
            auto prepared = updateLesson(db, task, lesson, {{"media", media}, {"searchStatus", search.status}, {"cached", search.cached}});
            const auto commit = prepared.commit;
            prepared.commit = [commit, image, imageId](Database& connection) {
                if (!image.empty()) {
                    const auto previous = connection.myAIRecord(imageId);
                    if (previous && previous->kind != "deep-image") throw std::invalid_argument("图片缓存身份冲突");
                    if ((!previous || previous->payload != image.dump()) &&
                        !connection.saveMyAIRecord(MyAIRecord{imageId, "deep-image", image.dump(), libraryNow(), 1}, previous ? previous->version : 0))
                        throw std::runtime_error("教学图片保存失败");
                }
                commit(connection);
            };
            return prepared;
        }});
}

std::string deepCoursePrompt(Database& db, const Json& task) {
    const auto event = task.at("event");
    const bool planning = event.value("type", "") == "plan_course" && event.value("mode", "") == "deep";
    const bool deep = !event.value("courseId", "").empty() && deepCourseEnabled(db, event.at("courseId"));
    if (!planning && !deep) return {};
    if (planning) return u8"本次新建深度课程：支持任意领域目标，按领域真实对象和任务组织courseStructure。只先生成并保存阶段、知识点、学习目标和主线。调用create_course后以state=completed结束本次规划，不创建课时或生成讲义；图片和逐课课件在进入课时时按需生成。create_course 的mode须保留event.mode。";
    return u8"当前课程使用deep-v1深度课堂：主区域是已保存讲义／翻页课件，右侧是可收起的辅助答疑。课程主线保持，讲解和练习按真实学习表现调整。备课时为指定topicId建立课时，使用append_section保存真实讲解与题目，用set_deep_content保存完整图文讲义及按需课件；finish_lesson前讲义必须已保存。讲义不泄露未作答题的私有标准，课件应有教学层次而非把长文本截断。按需要选择时间轴、流程图、函数参数或受力演示；不适用时用准确图文讲义，不强制每课都有媒体。需要真实图片时调用search_lesson_images并保存来源；图片失败保留讲义与单项重试状态，不让整课失败。已有内容直接复用，翻页、换视图和操作本地图示不生成评分。学生新作答仍先评价后讲解，纯答疑不评分。只重试素材时保留其余已保存内容。";
}

std::string renderDeepCoursePage() {
    return R"HTML(<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta name="theme-color" content="#f5f7fc"><title>深度课堂 · 钢一定制AI</title><link rel="icon" href="/school-logo.png"><link rel="stylesheet" href="/vendor/katex/katex.min.css"><link rel="stylesheet" href="/deep-course.css"><link rel="stylesheet" href="/clarity.css"><script defer src="/vendor/katex/katex.min.js"></script><script defer src="/chat-render.js"></script><script defer src="/learning-agent.js"></script><script defer src="/agent-shell.js"></script><script defer src="/clarity.js"></script><script defer src="/deep-course.js"></script></head><body class="clarity-app"><header class="site-header dark-header"><div class="dark-nav-shell"><a class="dark-brand" href="/"><img class="dark-brand-mark" src="/school-logo.png" alt="柳州市钢一中学校徽"><span>钢一<b>定制AI</b></span></a><nav class="dark-nav-links" aria-label="主导航"><a class="dark-nav-link" href="/">首页</a><a class="dark-nav-link" href="/my-courses">我的课程</a><a class="dark-nav-link is-active" href="/current-course" aria-current="page">当前课程</a><a class="dark-nav-link" href="/ask">AI 导师</a><a class="dark-nav-link" href="/my-ai">我的AI</a></nav><a class="dark-nav-cta" href="/my-ai/reports">我的画像</a><details class="dark-mobile-nav"><summary aria-label="打开主导航">菜单</summary><nav aria-label="窄屏主导航"><a href="/">首页</a><a href="/my-courses">我的课程</a><a href="/current-course">当前课程</a><a href="/ask">AI 导师</a><a href="/my-ai">我的AI</a></nav></details></div></header><main id="deep-course-app" aria-label="深度课堂"><p role="status">正在读取已保存的课堂…</p></main></body></html>)HTML";
}
}

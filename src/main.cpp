#include "config.hpp"
#include "version.hpp"
#include "db.hpp"
#include "ai_client.hpp"
#include "page_renderer.hpp"
#include "plan_generator.hpp"
#include "plan_adapter.hpp"
#include "search_client.hpp"
#include "quality_gate.hpp"
#include "course_service.hpp"
#include "progress_service.hpp"
#include "phase_generator.hpp"
#include "ask_generator.hpp"
#include "learning_generator.hpp"
#include "image_goal_analyzer.hpp"
#include "profile_service.hpp"
#include "question_evidence.hpp"
#include "home_recommendations.hpp"
#include "learning_flow.hpp"
#include "learning_agent.hpp"
#include "agent_preferences.hpp"
#include "agent_lessons.hpp"
#include "classroom_service.hpp"
#include "json_fix.hpp"

#include <crow.h>
#include <crow/multipart.h>
#include <nlohmann/json.hpp>

#ifdef DELETE
#undef DELETE
#endif

#include <filesystem>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

std::string content_type_for(const std::filesystem::path& path) {
    const auto extension = path.extension().string();
    if (extension == ".html") return "text/html; charset=utf-8";
    if (extension == ".css") return "text/css; charset=utf-8";
    if (extension == ".js") return "application/javascript; charset=utf-8";
    if (extension == ".json") return "application/json; charset=utf-8";
    if (extension == ".png") return "image/png";
    if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    if (extension == ".svg") return "image/svg+xml";
    if (extension == ".ico") return "image/x-icon";
    if (extension == ".woff2") return "font/woff2";
    if (extension == ".woff") return "font/woff";
    if (extension == ".ttf") return "font/ttf";
    return "application/octet-stream";
}

// 按 UTF-8 字符边界截断，避免 json 序列化遇到半个字符而失败。
std::string utf8Truncate(const std::string& text, size_t maxBytes) {
    if (text.size() <= maxBytes) return text;
    size_t cut = maxBytes;
    while (cut > 0 && (static_cast<unsigned char>(text[cut]) & 0xC0) == 0x80) --cut;
    return text.substr(0, cut);
}

// 查询参数百分号编码，保证课程 ID 与复习 ID 可安全拼进链接。
std::string queryValue(const std::string& value) {
    static constexpr char kHex[] = "0123456789ABCDEF";
    std::string encoded;
    encoded.reserve(value.size());
    for (const unsigned char character : value) {
        const bool plain = (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9') || character == '-' || character == '_' ||
            character == '.' || character == '~';
        if (plain) {
            encoded.push_back(static_cast<char>(character));
        } else {
            encoded.push_back('%');
            encoded.push_back(kHex[character >> 4]);
            encoded.push_back(kHex[character & 0x0F]);
        }
    }
    return encoded;
}

}  // namespace

namespace {

nlohmann::json stepToJson(const gangyi::Step& s) {
    return {{"title", s.title}, {"explanation", s.explanation}, {"example", s.example}, {"action", s.action}, {"check", s.check}};
}

nlohmann::json phaseToJson(const gangyi::Phase& p) {
    nlohmann::json steps = nlohmann::json::array();
    for (const auto& s : p.steps) steps.push_back(stepToJson(s));
    return {{"name", p.name}, {"durationWeeks", p.durationWeeks}, {"duration", p.duration},
            {"objective", p.objective}, {"why", p.why}, {"description", p.description}, {"overview", p.overview},
            {"topics", p.topics}, {"topicDescriptions", p.topicDescriptions}, {"tasks", p.tasks},
            {"practice", p.practice}, {"checkpoint", p.checkpoint}, {"output", p.output},
            {"commonMistakes", p.commonMistakes}, {"steps", steps}};
}

nlohmann::json slideToJson(const gangyi::Slide& s) {
    return {{"title", s.title}, {"subtitle", s.subtitle}, {"content", s.content}, {"bullets", s.bullets},
            {"speakerNote", s.speakerNote}, {"relatedPhase", s.relatedPhase}};
}

nlohmann::json planToJson(const gangyi::GeneratedPlan& plan) {
    nlohmann::json phases = nlohmann::json::array();
    for (const auto& p : plan.phases) phases.push_back(phaseToJson(p));
    nlohmann::json slides = nlohmann::json::array();
    for (const auto& s : plan.slides) slides.push_back(slideToJson(s));
    return {{"inferredDomain", plan.inferredDomain}, {"learnerGoal", plan.learnerGoal},
            {"courseTitle", plan.courseTitle}, {"courseSummary", plan.courseSummary},
            {"title", plan.title}, {"goal", plan.goal}, {"durationWeeks", plan.durationWeeks},
            {"summary", plan.summary}, {"courseIntro", plan.courseIntro}, {"overview", plan.overview},
            {"audience", plan.audience}, {"prerequisites", plan.prerequisites}, {"outcome", plan.outcome},
            {"learningOutcomes", plan.learningOutcomes}, {"phases", phases}, {"slides", slides},
            {"mindMap", plan.mindMap}, {"resources", plan.resources}, {"projects", plan.projects}};
}

std::string nowIso8601() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::ostringstream out;
    out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

std::string todayDate() {
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    std::ostringstream output;
    output << std::put_time(&local, "%Y-%m-%d");
    return output.str();
}

[[maybe_unused]] std::string mondayDate() {
    const std::string today = todayDate();
    std::tm day{};
    std::istringstream input(today);
    input >> std::get_time(&day, "%Y-%m-%d");
    day.tm_hour = 12;
    std::mktime(&day);
    day.tm_mday -= (day.tm_wday + 6) % 7;
    std::mktime(&day);
    std::ostringstream output;
    output << std::put_time(&day, "%Y-%m-%d");
    return output.str();
}

// 课程结构里每个主题依次回调 (阶段序号, 主题序号, 主题名)。
template <typename Callback>
void forEachTopic(const nlohmann::json& payload, Callback callback) {
    if (!payload.is_object()) return;
    const auto stages = payload.value("courseStructure", nlohmann::json::array());
    if (!stages.is_array()) return;
    int phaseIndex = 0;
    for (const auto& stage : stages) {
        ++phaseIndex;
        if (!stage.is_object()) continue;
        const auto items = stage.value("topics", nlohmann::json::array());
        if (!items.is_array()) continue;
        int topicIndex = 0;
        for (const auto& item : items) {
            ++topicIndex;
            if (item.is_string()) callback(phaseIndex, topicIndex, item.get<std::string>());
        }
    }
}

[[maybe_unused]] nlohmann::json courseTopics(gangyi::Database& db, const std::string& courseId, const nlohmann::json& payload) {
    nlohmann::json topics = nlohmann::json::array();
    forEachTopic(payload, [&](int phaseIndex, int topicIndex, const std::string& name) {
        const auto progress = db.findLearningCardProgress(courseId, phaseIndex, topicIndex);
        if (!progress || progress->status != "completed") topics.push_back({{"title", name}, {"phaseIndex", phaseIndex}, {"topicIndex", topicIndex}});
    });
    return topics;
}

std::string savedTopic(gangyi::Database& db, const std::string& courseId, int phase, int topic) {
    static std::mutex topicIdMutex;
    std::lock_guard<std::mutex> guard(topicIdMutex);
    const auto found = gangyi::getCourseWithSnapshot(db, courseId);
    if (!found || phase < 1 || topic < 1) return {};
    auto payload = found->payload;
    auto stages = payload.value("courseStructure", nlohmann::json::array());
    bool upgraded = false;
    if (stages.is_array()) for (size_t p = 0; p < stages.size(); ++p) {
        if (!stages[p].is_object() || !stages[p].value("topics", nlohmann::json()).is_array()) continue;
        auto& ids = stages[p]["topicIds"];
        if (!ids.is_array()) { ids = nlohmann::json::array(); upgraded = true; }
        while (ids.size() < stages[p]["topics"].size()) {
            ids.push_back("topic-" + courseId + "-" + std::to_string(p + 1) + "-" + std::to_string(ids.size() + 1));
            upgraded = true;
        }
    }
    if (upgraded) {
        payload["courseStructure"] = stages;
        const auto versions = db.findSnapshotsByCourseId(courseId);
        const int version = versions.empty() ? 1 : versions.back().version + 1;
        db.insert(gangyi::CourseSnapshot{"", courseId, version, payload.dump(), nowIso8601()});
    }
    if (!stages.is_array() || phase > static_cast<int>(stages.size()) || !stages[phase - 1].is_object()) return {};
    const auto topics = stages[phase - 1].value("topics", nlohmann::json::array());
    if (!topics.is_array() || topic > static_cast<int>(topics.size()) || !topics[topic - 1].is_string()) return {};
    return topics[topic - 1].get<std::string>();
}

struct ClassroomSocket {
    std::mutex mutex;
    crow::websocket::connection* connection = nullptr;
    std::atomic<bool> cancelled{false};
    std::atomic<bool> busy{false};
    void send(const nlohmann::json& event) {
        std::lock_guard<std::mutex> guard(mutex);
        if (connection) connection->send_text(event.dump());
    }
};

std::string qualityFeedback(const gangyi::QualityResult& result) {
    std::ostringstream out;
    out << "质量评分：" << result.score << "。问题：";
    if (result.reasons.empty()) out << "课程结构或内容不完整。";
    else for (const auto& reason : result.reasons) out << "\n- " << reason;
    return out.str();
}

std::string qualityIssueCodes(const gangyi::QualityResult& result) {
    std::set<std::string> codes;
    for (const auto& reason : result.reasons) {
        if (reason.find("阶段数") != std::string::npos) codes.insert("phase_count");
        else if (reason.find("topics") != std::string::npos) codes.insert("topics");
        else if (reason.find("重复") != std::string::npos) codes.insert("repetition");
        else if (reason.find("相关性") != std::string::npos) codes.insert("relevance");
        else if (reason.find("动作词") != std::string::npos) codes.insert("action");
        else if (reason.find("产出词") != std::string::npos) codes.insert("output");
        else if (reason.find("泛化") != std::string::npos) codes.insert("generic");
        else if (reason.find("roadmap") != std::string::npos) codes.insert("roadmap");
        else codes.insert("other");
    }
    std::ostringstream output;
    for (const auto& code : codes) {
        if (output.tellp() > 0) output << ',';
        output << code;
    }
    return output.str();
}

bool requesterCanReadCourse(gangyi::Database&, const gangyi::Course& course, const crow::request&) {
    return course.status == "active";
}

bool requesterCanAccessCourse(gangyi::Database& db, const crow::request&, const std::string& courseId,
                              const std::string&) {
    if (courseId.empty()) return true;
    const auto course = db.getCourse(courseId);
    return course && course->status == "active";
}

gangyi::AgentAccess localAgentAccess(gangyi::Database& db) {
    // 当前桌面服务共用本机档案；可访问课程列表由服务端读取，不接收客户端扩大范围。
    gangyi::AgentAccess access{"local-profile", {}};
    for (const auto& course : db.listCourses()) if (course.status == "active") access.courseIds.push_back(course.id);
    return access;
}

std::string queryValueFromUrl(const std::string& url, const std::string& key) {
    const std::string needle = key + "=";
    const auto queryStart = url.find('?');
    if (queryStart == std::string::npos) return {};
    auto position = url.find(needle, queryStart + 1);
    while (position != std::string::npos) {
        if (position == queryStart + 1 || url[position - 1] == '&') {
            const auto start = position + needle.size();
            const auto end = url.find('&', start);
            return url.substr(start, end == std::string::npos ? std::string::npos : end - start);
        }
        position = url.find(needle, position + needle.size());
    }
    return {};
}

std::string requestAnonymousId(const crow::request& req, const nlohmann::json* body = nullptr) {
    if (const char* value = req.url_params.get("anonymousId")) return value;
    if (body && body->is_object() && body->contains("anonymousId") && (*body)["anonymousId"].is_string()) {
        return (*body)["anonymousId"].get<std::string>();
    }
    const std::string cookies = req.get_header_value("Cookie");
    const std::string name = "gangyi_anonymous_id=";
    const auto start = cookies.find(name);
    if (start != std::string::npos) {
        const auto valueStart = start + name.size();
        const auto end = cookies.find(';', valueStart);
        return cookies.substr(valueStart, end == std::string::npos ? std::string::npos : end - valueStart);
    }
    const std::string referer = req.get_header_value("Referer");
    return queryValueFromUrl(referer, "anonymousId");
}

void recordInteraction(gangyi::Database& db, const std::string& kind, const nlohmann::json& payload,
                       const std::string& courseId = {}, const std::string& conversationId = {}) {
    gangyi::LearningInteraction item;
    item.kind = kind;
    item.payload = payload.dump();
    item.createdAt = nowIso8601();
    if (!courseId.empty()) item.courseId = courseId;
    if (!conversationId.empty()) item.conversationId = conversationId;
    if (!db.insert(item)) throw std::runtime_error("学习记录保存失败");
}

// 普通导师与每个课时只读取自己的最近八条消息。
// ponytail: 本机少量历史沿用现有读取接口；记录量明显增长时改为按会话 SQL LIMIT 8。
std::vector<gangyi::ChatMessage> chatHistory(gangyi::Database& db, const std::string& conversation,
                                           const nlohmann::json& body = nlohmann::json::object()) {
    std::vector<gangyi::ChatMessage> history;
    if (body.contains("messages")) {
        const auto& items = body.at("messages");
        if (!items.is_array() || items.size() > 100)
            throw gangyi::AIClientError("invalid_request", "历史消息格式无效");
        const size_t start = items.size() > 8 ? items.size() - 8 : 0;
        for (size_t i = start; i < items.size(); ++i) {
            const auto& item = items[i];
            if (!item.is_object() || !item.contains("role") || !item["role"].is_string() ||
                !item.contains("content") || !item["content"].is_string())
                throw gangyi::AIClientError("invalid_request", "历史消息格式无效");
            const auto role = item["role"].get<std::string>(), content = item["content"].get<std::string>();
            if ((role != "user" && role != "assistant") || content.size() > 16000)
                throw gangyi::AIClientError("invalid_request", "历史消息角色或长度无效");
            if (!content.empty()) history.push_back({role, content});
        }
    } else {
        for (const auto& row : db.listInteractions()) {
            if (row.conversationId.value_or("") != conversation ||
                (row.kind != "chat-user" && row.kind != "chat-assistant")) continue;
            const auto payload = nlohmann::json::parse(row.payload, nullptr, false);
            if (payload.is_object() && payload.contains("text") && payload["text"].is_string())
                history.push_back({row.kind == "chat-user" ? "user" : "assistant", payload["text"].get<std::string>()});
        }
        if (history.size() > 8) history.erase(history.begin(), history.end() - 8);
    }
    return history;
}

int aiHttpStatus(const gangyi::AIClientError& error) {
    if (error.errorType == "invalid_request") return 400;
    if (error.errorType == "rate_limited") return 429;
    if (error.errorType == "missing_config" || error.errorType == "auth_error") return 503;
    if (error.errorType == "timeout") return 504;
    return 502;
}

std::string publicAIErrorMessage(const gangyi::AIClientError& error) {
    if (error.errorType == "invalid_request") return error.what();
    if (error.errorType == "missing_config") return "AI 服务尚未完成配置，请先在启动器中填写配置。";
    if (error.errorType == "auth_error") return "AI 服务认证失败，请检查 API Key 和模型权限。";
    if (error.errorType == "rate_limited") return "AI 服务请求过于频繁或余额不足，请稍后重试。";
    if (error.errorType == "timeout") return "AI 服务响应超时，请检查网络后重试。";
    if (error.errorType == "network_error") return "暂时无法连接 AI 服务，请检查网络和接口地址。";
    if (error.errorType == "provider_5xx") return "AI 服务暂时不可用，请稍后重试。";
    if (error.errorType == "invalid_response" || error.errorType == "json_parse_error" ||
        error.errorType == "quality_rejected") return "AI 返回的内容格式不完整，请重新生成。";
    return "AI 服务暂时不可用，请稍后重试。";
}

}  // namespace

int main() {
    const auto config = gangyi::Config::from_environment();
    gangyi::Database db;
    // 自动创建数据库所在目录（首次部署时 data/ 可能不存在，避免 sqlite 打开失败）
    try {
        const std::filesystem::path dbPath = std::filesystem::u8path(config.database_path);
        if (dbPath.has_parent_path()) std::filesystem::create_directories(dbPath.parent_path());
    } catch (...) {}
    db.open(config.database_path);
    db.migrate();
    gangyi::HomeRecommendations homeRecommendations(config.database_path, config.launch_session_id);
    gangyi::LearningFlow learningFlow(config.database_path);
    gangyi::LearningAgent learningAgent(config.database_path);
    gangyi::registerAgentPreferenceTools(learningAgent);
    gangyi::registerAgentLessonTools(learningAgent);
    crow::SimpleApp app;
    // 访问路径可能包含用户填写的学习目标，避免将其写入信息级访问日志。
    app.loglevel(crow::LogLevel::Warning);

    CROW_ROUTE(app, "/")([] {
        crow::response response(gangyi::renderHomePage());
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/plan")([&db](const crow::request& req) {
        const char* goal = req.url_params.get("goal");
        const char* mode = req.url_params.get("mode");
        const char* courseId = req.url_params.get("courseId");
        const char* anonymousId = req.url_params.get("anonymousId");
        // 访问记录只用于最近课程排序，不作为掌握度证据。
        try {
            if (courseId && *courseId && requesterCanAccessCourse(db, req, courseId, requestAnonymousId(req)))
                recordInteraction(db, "course-visit", nlohmann::json::object(), courseId);
        } catch (...) { /* 记录暂时不可用时，仍允许打开课程。 */ }
        crow::response response(gangyi::renderPlanPage(goal ? goal : "", mode ? mode : "deep",
            courseId ? courseId : "", anonymousId ? anonymousId : ""));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

#if 0  // 顶部进度页面已删除。
    CROW_ROUTE(app, "/progress")([&db](const crow::request& req) {
        const char* courseIdP = req.url_params.get("courseId");
        const char* anonymousIdP = req.url_params.get("anonymousId");
        const char* goalP = req.url_params.get("goal");
        const char* modeP = req.url_params.get("mode");
        const std::string courseId = courseIdP ? courseIdP : "";
        const std::string anonymousId = anonymousIdP ? anonymousIdP : "";
        const std::string goal = goalP ? goalP : "";
        const std::string mode = modeP && *modeP ? modeP : "deep";
        nlohmann::json data = {{"ready", false}, {"courseTitle", courseId}};
        if (!courseId.empty() && requesterCanAccessCourse(db, req, courseId, anonymousId)) {
            if (const auto found = gangyi::getCourseWithSnapshot(db, courseId)) {
                if (found->payload.is_object() && found->payload.contains("roadmap") && found->payload["roadmap"].is_array()) {
                    data["ready"] = true;
                    data["courseTitle"] = found->course.title.empty() ? found->course.goal : found->course.title;
                    if (const auto prog = gangyi::recomputeCourseProgress(db, courseId, anonymousId, found->course.goal)) {
                        data["overallPercent"] = prog->value("overallPercent", 0);
                        data["completedCount"] = prog->value("completedCount", 0);
                        data["totalCount"] = prog->value("totalCount", 0);
                        data["updatedAt"] = prog->value("updatedAt", "");
                    }
                    if (const auto cp = db.findProgressByCourseId(courseId)) {
                        if (cp->lastVisitedUrl && !cp->lastVisitedUrl->empty()) {
                            data["hasBreakpoint"] = true;
                            data["lastVisitedUrl"] = *cp->lastVisitedUrl;
                            std::string bp = (cp->lastPhaseName && !cp->lastPhaseName->empty()) ? *cp->lastPhaseName : "上次学习位置";
                            if (cp->lastTopicTitle && !cp->lastTopicTitle->empty()) bp += " · " + *cp->lastTopicTitle;
                            data["breakpointText"] = bp;
                        }
                    }
                    const auto roadmap = found->payload["roadmap"];
                    const nlohmann::json courseStructure = found->payload.contains("courseStructure") && found->payload["courseStructure"].is_array() ? found->payload["courseStructure"] : nlohmann::json::array();
                    const auto cards = db.listLearningCardProgress();
                    nlohmann::json phases = nlohmann::json::array();
                    int idx = 0;
                    for (const auto& st : roadmap) {
                        ++idx;
                        int totalTopics = 0;
                        if (idx - 1 < static_cast<int>(courseStructure.size()) && courseStructure[idx - 1].contains("topics") && courseStructure[idx - 1]["topics"].is_array())
                            totalTopics = static_cast<int>(courseStructure[idx - 1]["topics"].size());
                        if (totalTopics == 0 && st.contains("tasks") && st["tasks"].is_array()) totalTopics = static_cast<int>(st["tasks"].size());
                        int done = 0;
                        for (const auto& c : cards)
                            if (c.courseId.value_or("") == courseId &&
                                (c.phaseIndex == idx || (idx == 1 && c.phaseIndex == 0)) &&
                                c.status == "completed") ++done;
                        const int pct = totalTopics > 0 ? static_cast<int>(done * 100.0 / totalTopics + 0.5) : 0;
                        const std::string name = (st.contains("name") && st["name"].is_string()) ? st["name"].get<std::string>() : ("阶段" + std::to_string(idx));
                        std::string status = "not_started";
                        if (totalTopics > 0 && done >= totalTopics) status = "completed";
                        else if (done > 0) status = "in_progress";
                        const std::string href = "/phase?courseId=" + courseId + "&phaseIndex=" + std::to_string(idx) +
                            "&goal=" + goal + "&mode=" + mode +
                            (anonymousId.empty() ? "" : "&anonymousId=" + anonymousId);
                        phases.push_back({{"index", idx}, {"name", name}, {"total", totalTopics},
                            {"completed", done}, {"percent", pct}, {"status", status}, {"href", href}});
                    }
                    data["phases"] = phases;
                }
            }
        }
        crow::response response(gangyi::renderProgressPage(courseId, anonymousId, data));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });
#endif

    // /phase 阶段页：从 courseId 课程快照 SSR 渲染阶段与主题网格
    CROW_ROUTE(app, "/phase")([&db](const crow::request& req) {
        const char* courseIdP = req.url_params.get("courseId");
        const char* anonymousIdP = req.url_params.get("anonymousId");
        const char* goalP = req.url_params.get("goal");
        const char* modeP = req.url_params.get("mode");
        const char* phaseIndexP = req.url_params.get("phaseIndex");
        const char* phaseNameP = req.url_params.get("phaseName");
        std::string courseId = courseIdP ? courseIdP : "";
        std::string anonymousId = anonymousIdP ? anonymousIdP : "";
        std::string goal = goalP ? goalP : "";
        std::string mode = modeP && *modeP ? modeP : "deep";
        std::string phaseIndex = phaseIndexP ? phaseIndexP : "1";
        std::string phaseName = phaseNameP ? phaseNameP : "";
        nlohmann::json plan = nlohmann::json::object();
        nlohmann::json card = nlohmann::json::object();
        if (!courseId.empty() && requesterCanAccessCourse(db, req, courseId, anonymousId)) {
            if (const auto found = gangyi::getCourseWithSnapshot(db, courseId)) {
                if (found->payload.is_object()) plan = found->payload;
                if (goal.empty()) goal = found->course.goal;
                if (mode.empty()) mode = found->course.mode;
            }
        }
        crow::response response(gangyi::renderPhasePage(courseId, anonymousId, goal, mode, phaseIndex, phaseName, plan, card));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    // /learn 微课程页：课程页和阶段页仍会生成此链接，必须保持与学习 API 配套注册。
    CROW_ROUTE(app, "/learn")([&db](const crow::request& req) {
        const auto value = [&req](const char* name) {
            const char* item = req.url_params.get(name);
            return std::string(item ? item : "");
        };
        const std::string anonymousId = requestAnonymousId(req);
        try {
            const auto courseId = value("courseId");
            if (!courseId.empty() && requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                const int phase = std::stoi(value("phaseIndex")), index = std::stoi(value("topicIndex"));
                const auto topic = savedTopic(db, courseId, phase, index);
                if (!topic.empty()) recordInteraction(db, "lesson-visit",
                    {{"phaseIndex", phase}, {"topicIndex", index}, {"topic", topic}}, courseId);
            }
        } catch (...) { /* 不让访问记录影响课堂页面加载。 */ }
        crow::response response(gangyi::renderLearnPage(
            value("courseId"), value("goal"), value("mode").empty() ? "deep" : value("mode"),
            value("phaseIndex"), value("phaseName"), value("topicIndex"), value("topic"),
            anonymousId, value("regenerate"), value("forceLearn"), value("retry")));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    // GET /api/learn —— 按板块生成、校验并保存真实 AI 微课堂内容。
    CROW_ROUTE(app, "/api/learn")([&db](const crow::request& req) {
        // ponytail: 全局锁避免同课时并发重复生成；并发课程变多时改按课时锁。
        static std::mutex generationMutex;
        std::unique_lock<std::mutex> generationGuard(generationMutex, std::defer_lock);
        const char* requestedBlock = req.url_params.get("block");
        if (requestedBlock && *requestedBlock) generationGuard.lock();
        std::string activeBlock;
        try {
            const auto value = [&req](const char* name) {
                const char* item = req.url_params.get(name);
                return std::string(item ? item : "");
            };
            const std::string courseId = value("courseId");
            const std::string anonymousId = requestAnonymousId(req);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            }

            std::string goal = value("goal");
            std::string mode = value("mode") == "lite" ? "lite" : "deep";
            nlohmann::json plan = nlohmann::json::object();
            if (!courseId.empty()) {
                if (const auto found = gangyi::getCourseWithSnapshot(db, courseId)) {
                    plan = found->payload;
                    goal = found->course.goal;
                    mode = found->course.mode == "lite" ? "lite" : "deep";
                }
            }
            if (goal.empty()) return crow::response(400, nlohmann::json{{"ok", false}, {"type", "learning_context_missing"}, {"error", "缺少学习目标"}}.dump());

            int phaseNumber = 1, topicNumber = 1;
            try { if (!value("phaseIndex").empty()) phaseNumber = std::max(1, std::stoi(value("phaseIndex"))); } catch (...) {}
            try { if (!value("topicIndex").empty()) topicNumber = std::max(1, std::stoi(value("topicIndex"))); } catch (...) {}
            const int phaseIndex = phaseNumber - 1;
            const int topicIndex = topicNumber - 1;
            nlohmann::json stage = nlohmann::json::object();
            const auto roadmap = plan.value("roadmap", nlohmann::json::array());
            if (roadmap.is_array() && phaseIndex < static_cast<int>(roadmap.size())) stage = roadmap[phaseIndex];
            const auto structure = plan.value("courseStructure", nlohmann::json::array());
            if (structure.is_array() && phaseIndex < static_cast<int>(structure.size()) && structure[phaseIndex].is_object()) {
                if (stage.empty()) stage = nlohmann::json::object();
                if (!stage.contains("topics")) stage["topics"] = structure[phaseIndex].value("topics", nlohmann::json::array());
                if (!stage.contains("name")) stage["name"] = structure[phaseIndex].value("stage", "");
            }
            const std::string phaseName = courseId.empty() ? value("phaseName") : stage.value("name", "");
            std::string topic = courseId.empty() ? value("topic") : "";
            const auto topics = stage.value("topics", nlohmann::json::array());
            if (topic.empty() && topics.is_array() && topicIndex < static_cast<int>(topics.size()) && topics[topicIndex].is_string()) topic = topics[topicIndex].get<std::string>();
            if (phaseName.empty() || topic.empty()) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"type", "learning_context_missing"},
                    {"error", "当前课程缺少 AI 生成的阶段或主题，请重新生成课程。"}, {"canRetry", false}}.dump());
            }

            const int profileVersion = db.profileAssessedRevision();
            const int learningVersion = db.learningRevision();
            nlohmann::json stored = {{"schemaVersion", 2}, {"promptVersion", "ai-block-v1"},
                {"contentVersion", 1}, {"learningVersion", learningVersion},
                {"blocks", nlohmann::json::object()}, {"generations", nlohmann::json::object()},
                {"references", nlohmann::json::array()}, {"profileVersion", profileVersion}};
            std::optional<gangyi::LearningSession> existing;
            if (!courseId.empty()) existing = db.findLearningSession(courseId, phaseNumber, topicNumber);
            if (existing && existing->source == "ai" && existing->fallbackUsed == 0) {
                try {
                    const auto parsed = nlohmann::json::parse(existing->content);
                    if (parsed.value("promptVersion", "") == "ai-block-v1" && parsed.value("blocks", nlohmann::json()).is_object()) stored = parsed;
                } catch (...) {}

            }
            bool touched = false;
            const std::string initialExposure = db.profileMeta("learning-exposure:" + courseId + ":" + std::to_string(phaseNumber) + ":" + std::to_string(topicNumber));
            const auto seenContent = nlohmann::json::parse(initialExposure, nullptr, false);
            if (seenContent.is_object() && !seenContent.value("blocks", nlohmann::json::array()).empty()) touched = true;
            if (const auto progress = db.findLearningCardProgress(courseId, phaseNumber, topicNumber))
                touched = progress->status != "not_started";
            if (!touched) for (const auto& item : db.listInteractions()) {
                if (item.courseId.value_or("") != courseId ||
                    (item.kind != "quiz" && item.kind != "practice" && item.kind != "review")) continue;
                const auto payload = nlohmann::json::parse(item.payload, nullptr, false);
                if (payload.is_object() && payload.value("phaseIndex", 0) == phaseNumber &&
                    payload.value("topicIndex", 0) == topicNumber) { touched = true; break; }
            }

            const std::string block = value("block");
            const std::vector<std::string> allowed = {"overview", "steps", "examples", "practice", "quiz", "assessment"};
            const bool generateAll = block == "all";
            const auto hasAiBlock = [](const nlohmann::json& content, const std::string& name) {
                if (!content.value("blocks", nlohmann::json()).is_object() ||
                    !content["blocks"].contains(name) ||
                    !content.value("generations", nlohmann::json()).is_object() ||
                    !content["generations"].contains(name) ||
                    !content["generations"][name].is_object()) return false;
                const auto& generation = content["generations"][name];
                return generation.value("source", "") == "ai" &&
                    (generation.value("promptVersion", "") == "ai-block-v1" || generation.value("promptVersion", "") == "ai-adaptation-v1") &&
                    !generation.value("model", "").empty();
            };
            if (block.empty()) {
                return crow::response(200, nlohmann::json{{"ok", true}, {"cached", true}, {"goal", goal},
                    {"phaseName", phaseName}, {"topicTitle", topic}, {"blocks", stored["blocks"]},
                    {"generations", stored["generations"]}, {"contentVersion", stored.value("contentVersion", 1)}, {"references", stored["references"]}}.dump());
            }
            if (!generateAll && std::find(allowed.begin(), allowed.end(), block) == allowed.end()) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"type", "invalid_request"}, {"error", "未知课堂板块"}}.dump());
            }

            const bool regenerateAll = !touched && (value("regenerate") == "1" ||
                (existing && stored.value("profileVersion", 0) < profileVersion));
            const bool retryBlock = !touched && value("retry") == "1";
            if (generateAll && !regenerateAll && std::all_of(allowed.begin(), allowed.end(),
                    [&](const std::string& name) { return hasAiBlock(stored, name); })) {
                return crow::response(200, nlohmann::json{{"ok", true}, {"cached", true},
                    {"phaseName", phaseName}, {"topicTitle", topic}, {"blocks", stored["blocks"]},
                    {"generations", stored["generations"]}, {"contentVersion", stored.value("contentVersion", 1)}, {"references", stored["references"]}}.dump());
            }
            if (!generateAll && !regenerateAll && !retryBlock && hasAiBlock(stored, block)) {
                return crow::response(200, nlohmann::json{{"ok", true}, {"cached", true}, {"block", block},
                    {"content", stored["blocks"][block]}, {"generation", stored["generations"].value(block, nlohmann::json::object())},
                    {"references", stored["references"]}}.dump());
            }

            nlohmann::json working = stored;
            working["profileVersion"] = profileVersion;
            if (regenerateAll) {
                working["blocks"] = nlohmann::json::object();
                working["generations"] = nlohmann::json::object();
                working["references"] = nlohmann::json::array();
            }

            std::vector<gangyi::SearchResource> resources;
            try {
                gangyi::SearchClient search;
                resources = search.search(goal, 8);
            } catch (...) {}

            if (!resources.empty()) {
                working["references"] = nlohmann::json::array();
                for (const auto& resource : resources) working["references"].push_back({
                    {"title", resource.title}, {"source", resource.source}, {"url", resource.url},
                    {"type", resource.type}, {"description", resource.description},
                    {"difficulty", resource.difficulty}, {"language", resource.language}, {"free", resource.free}});
            }

            const auto persist = [&](const nlohmann::json& content) {
                if (courseId.empty()) return;
                const auto overview = content["blocks"].value("overview", nlohmann::json::object());
                gangyi::LearningSession session;
                session.id = existing ? existing->id : "learn-" + courseId + "-" + std::to_string(phaseNumber) + "-" + std::to_string(topicNumber);
                session.courseId = courseId;
                session.anonymousId = anonymousId;
                session.goal = goal;
                session.mode = mode;
                session.phaseIndex = phaseNumber;
                session.phaseName = phaseName;
                session.topicIndex = topicNumber;
                session.topicTitle = topic;
                session.title = overview.value("title", topic);
                session.summary = overview.value("summary", "");
                session.searchQuery = goal + " " + phaseName + " " + topic;
                session.content = content.dump();
                session.references = content["references"].dump();
                session.fallbackUsed = 0;
                session.source = "ai";
                const std::string exposureKey = "learning-exposure:" + courseId + ":" + std::to_string(phaseNumber) + ":" + std::to_string(topicNumber);
                const auto exposure = initialExposure;
                const bool saved = existing ? db.updateLearningSessionAtRevision(session, existing->content, learningVersion, exposureKey, exposure) :
                    (db.learningRevision() == learningVersion && db.insert(session));
                if (!saved) throw std::runtime_error("课堂板块保存失败");
                existing = session;
            };

            gangyi::AIClient ai;
            gangyi::LearningGenerator generator(ai);
            const auto generate = [&](const std::string& name) {
                activeBlock = name;
                nlohmann::json learningPlan = plan;
                learningPlan["personalLearning"] = gangyi::learningContext(db, courseId);
                learningPlan["latestLearning"] = nlohmann::json::parse(gangyi::learningContext(db, courseId));
                for (const auto& instruction : learningPlan["latestLearning"].value("teaching", nlohmann::json::array()))
                    if (instruction.value("courseId", "") == courseId && instruction.value("phaseIndex", 0) == phaseNumber &&
                        instruction.value("topicIndex", 0) == topicNumber) learningPlan["teachingInstruction"] = instruction;
                nlohmann::json generated = generator.generateBlock(goal, learningPlan, phaseName, topic, topicNumber,
                    mode, name, working["blocks"], resources, 1);
                const nlohmann::json metadata = generated.value("_generation", nlohmann::json::object());
                if (metadata.value("source", "") != "ai") throw gangyi::AIClientError("invalid_response", "AI 生成来源校验失败");
                generated.erase("_generation");
                working["blocks"][name] = generated;
                working["generations"][name] = metadata;
                working["generations"][name]["profileVersion"] = profileVersion;
                if (!generateAll || !regenerateAll) persist(working);
                std::cerr << "[learning-block] saved course=" << courseId << " phase=" << phaseNumber
                          << " topic=" << topicNumber << " block=" << name << '\n';
                return std::pair<nlohmann::json, nlohmann::json>{std::move(generated), working["generations"][name]};
            };

            if (generateAll) {
                bool generatedAny = false;
                for (const auto& name : allowed) {
                    if (!regenerateAll && hasAiBlock(working, name)) continue;
                    generate(name);
                    generatedAny = true;
                }
                if (regenerateAll) persist(working);
                activeBlock.clear();
                return crow::response(200, nlohmann::json{{"ok", true}, {"cached", !generatedAny},
                    {"phaseName", phaseName}, {"topicTitle", topic}, {"blocks", working["blocks"]},
                    {"generations", working["generations"]}, {"contentVersion", working.value("contentVersion", 1)}, {"references", working["references"]}}.dump());
            }

            auto [generated, metadata] = generate(block);
            activeBlock.clear();
            return crow::response(200, nlohmann::json{{"ok", true}, {"cached", false}, {"block", block},
                {"content", generated}, {"generation", metadata}, {"references", working["references"]}}.dump());
        } catch (const gangyi::AIClientError& error) {
            nlohmann::json response = {{"ok", false}, {"type", error.errorType},
                {"error", publicAIErrorMessage(error)}, {"canRetry", true}, {"attempts", 3}};
            if (!activeBlock.empty()) response["failedBlock"] = activeBlock;
            return crow::response(aiHttpStatus(error), response.dump());
        } catch (const std::exception& error) {
            std::cerr << "[learning-block] internal_error=" << error.what() << '\n';
            return crow::response(500, nlohmann::json{{"ok", false}, {"type", "internal_error"},
                {"error", "课堂内容保存失败，请稍后重试。"}, {"canRetry", true}}.dump());

        }
    });
    // 学习页进度接口：页面已经恢复，步骤和完成状态也必须能稳定保存。
    CROW_ROUTE(app, "/api/learning-step-progress")([&db](const crow::request& req) {
        const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
        const std::string anonymousId = requestAnonymousId(req);
        if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
        }
        int phaseIndex = 0;
        try { if (req.url_params.get("phaseIndex")) phaseIndex = std::stoi(req.url_params.get("phaseIndex")); } catch (...) {}
        nlohmann::json items = nlohmann::json::array();
        for (const auto& item : db.listLearningStepProgress()) {
            if (item.courseId.value_or("") == courseId && item.phaseIndex == phaseIndex) {
                items.push_back({{"stepIndex", item.stepIndex}, {"stepTitle", item.stepTitle}, {"status", item.status}});
            }
        }
        return crow::response(200, nlohmann::json{{"ok", true}, {"items", items}}.dump());
    });

    CROW_ROUTE(app, "/api/learning-step-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            }
            const auto course = courseId.empty() ? std::optional<gangyi::Course>{} : db.getCourse(courseId);
            const int phaseIndex = body.value("phaseIndex", 0);
            const int stepIndex = body.value("stepIndex", 0);
            const auto old = db.findLearningStepProgress(courseId, phaseIndex, stepIndex);
            gangyi::LearningStepProgress item;
            item.id = old ? old->id : "step-" + courseId + "-" + std::to_string(phaseIndex) + "-" + std::to_string(stepIndex);
            item.courseId = courseId.empty() ? std::nullopt : std::optional<std::string>(courseId);
            item.anonymousId = anonymousId.empty() ? std::nullopt : std::optional<std::string>(anonymousId);
            item.goal = body.value("goal", course ? course->goal : "");
            item.mode = body.value("mode", course ? course->mode : "deep");
            item.phaseIndex = phaseIndex;
            item.phaseName = body.value("phaseName", "当前阶段");
            item.stepIndex = stepIndex;
            item.stepTitle = body.value("stepTitle", "学习步骤");
            item.status = gangyi::normalizeStepStatus(body.value("status", "unset"));
            const bool saved = old ? db.update(item) : db.insert(item);
            if (saved) recordInteraction(db, "step", {{"topic", item.stepTitle}, {"state", item.status}}, courseId);
            const auto progress = saved ? gangyi::recomputeCourseProgress(db, courseId, anonymousId, item.goal)
                                        : std::optional<nlohmann::json>{};
            return crow::response(saved ? 200 : 400, nlohmann::json{{"ok", saved},
                {"progress", progress ? *progress : nlohmann::json::object()}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        }
    });

    CROW_ROUTE(app, "/api/learn/progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            }
            const auto course = courseId.empty() ? std::optional<gangyi::Course>{} : db.getCourse(courseId);
            const int phaseIndex = body.value("phaseIndex", 0);
            const int topicIndex = body.value("topicIndex", 0);
            const auto old = db.findLearningCardProgress(courseId, phaseIndex, topicIndex);
            gangyi::LearningCardProgress item;
            item.id = old ? old->id : "card-" + courseId + "-" + std::to_string(phaseIndex) + "-" + std::to_string(topicIndex);
            item.courseId = courseId.empty() ? std::nullopt : std::optional<std::string>(courseId);
            item.anonymousId = anonymousId.empty() ? std::nullopt : std::optional<std::string>(anonymousId);
            item.goal = body.value("goal", course ? course->goal : "");
            item.mode = body.value("mode", course ? course->mode : "deep");
            item.phaseIndex = phaseIndex;
            item.phaseName = body.value("phaseName", "当前阶段");
            item.topicIndex = topicIndex;
            item.topicTitle = body.value("topicTitle", "当前学习内容");
            item.status = gangyi::normalizeCardStatus(body.value("status", "completed"));
            const bool saved = old ? db.update(item) : db.insert(item);
            if (saved) recordInteraction(db, "lesson", {{"topic", item.topicTitle}, {"state", item.status}}, courseId);
            if (saved && body.contains("lastVisitedUrl")) {
                gangyi::updateLastVisited(db, courseId, anonymousId, item.goal,
                    item.mode.value_or("deep"), body);
            }
            const auto progress = saved ? gangyi::recomputeCourseProgress(db, courseId, anonymousId, item.goal)
                                        : std::optional<nlohmann::json>{};
            return crow::response(saved ? 200 : 400, nlohmann::json{{"ok", saved},
                {"progress", progress ? *progress : nlohmann::json::object()}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        }
    });

    CROW_ROUTE(app, "/ask")([](const crow::request& req) {
        const char* question = req.url_params.get("question");
        crow::response response(gangyi::renderAskPage(question ? question : ""));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/my-courses")([&db](const crow::request& req) {
        const std::string anonymousId = requestAnonymousId(req);
        const auto courses = gangyi::listCoursesForIdentity(db, "", "", 100, 0);
        nlohmann::json list = nlohmann::json::array();
        nlohmann::json dueReviews = nlohmann::json::array();
        int totalTopics = 0, doneTopics = 0, dueTotal = 0;
        for (const auto& c : courses) {
            const std::string title = c.title.empty() ? (c.goal.empty() ? c.id : c.goal) : c.title;
            int courseTotal = 0, courseDone = 0;
            if (const auto found = gangyi::getCourseWithSnapshot(db, c.id)) {
                forEachTopic(found->payload, [&](int phaseIndex, int topicIndex, const std::string&) {
                    ++courseTotal;
                    const auto progress = db.findLearningCardProgress(c.id, phaseIndex, topicIndex);
                    if (progress && progress->status == "completed") ++courseDone;
                });
            }
            int courseDue = 0;
            try {
                for (const auto& review : gangyi::dueReviews(db, c.id, todayDate())) {
                    ++courseDue;
                    std::string reviewHref = "/learn?courseId=" + queryValue(c.id) +
                        "&phaseIndex=" + std::to_string(review.value("phaseIndex", 0)) +
                        "&topicIndex=" + std::to_string(review.value("topicIndex", 0)) +
                        "&review=" + std::to_string(review.value("day", 0));
                    const std::string reviewId = review.value("reviewId", std::string());
                    if (!reviewId.empty()) reviewHref += "&reviewId=" + queryValue(reviewId);
                    dueReviews.push_back({{"course", title},
                        {"title", review.value("title", std::string("到期复习"))},
                        {"due", review.value("due", std::string())},
                        {"href", reviewHref}});
                }
            } catch (...) {}
            totalTopics += courseTotal;
            doneTopics += courseDone;
            dueTotal += courseDue;
            list.push_back({{"courseId", c.id}, {"title", title}, {"goal", c.goal}, {"mode", c.mode},
                {"source", c.source}, {"createdAt", c.createdAt}, {"updatedAt", c.updatedAt},
                {"status", "generated"}, {"totalTopics", courseTotal}, {"doneTopics", courseDone},
                {"percent", courseTotal > 0 ? courseDone * 100 / courseTotal : 0},
                {"dueCount", courseDue}});
        }
        crow::response response(gangyi::renderMyCoursesPage({{"courses", list}, {"anonymousId", anonymousId},
            {"dueReviews", dueReviews},
            {"stats", {{"total", static_cast<int>(courses.size())}, {"totalTopics", totalTopics},
                {"doneTopics", doneTopics}, {"due", dueTotal},
                {"percent", totalTopics > 0 ? doneTopics * 100 / totalTopics : 0}}}}));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

#if 0  // 微课程页面已移除，旧接口不再注册。
    CROW_ROUTE(app, "/api/learn")([&db](const crow::request& req) {
        try {
            gangyi::AIClient ai;
            gangyi::LearningGenerator generator(ai);
            const char* courseId = req.url_params.get("courseId");
            const char* phaseIndexP = req.url_params.get("phaseIndex");
            const char* topicIndexP = req.url_params.get("topicIndex");
            const char* mode = req.url_params.get("mode");
            const char* goal = req.url_params.get("goal");
            const char* phaseName = req.url_params.get("phaseName");
            const char* topic = req.url_params.get("topic");
            const char* anonymousId = req.url_params.get("anonymousId");
            const char* regenerate = req.url_params.get("regenerate");
            const char* forceLearn = req.url_params.get("forceLearn");
            const char* retry = req.url_params.get("retry");
            const std::string safeCourseId = courseId ? courseId : "";
            const std::string safeAnonymousId = requestAnonymousId(req);
            if (!requesterCanAccessCourse(db, req, safeCourseId, safeAnonymousId)) {
                return crow::response(404, nlohmann::json{{"error", "课程不存在或无权访问。"}}.dump());
            }
            int phaseIndex = 0, topicIndex = 0;
            try { if (phaseIndexP) phaseIndex = std::stoi(phaseIndexP); } catch (...) {}
            try { if (topicIndexP) topicIndex = std::stoi(topicIndexP); } catch (...) {}
            std::string safeMode = mode ? mode : "deep";
            std::string safeGoal = goal && *goal ? goal : "";
            std::string safePhaseName = phaseName && *phaseName ? phaseName : "";
            std::string safeTopic = topic && *topic ? topic : "";
            if (!safeCourseId.empty()) {
                if (const auto found = gangyi::getCourseWithSnapshot(db, safeCourseId)) {
                    if (safeGoal.empty()) safeGoal = found->course.goal;
                    if (safeMode.empty()) safeMode = found->course.mode;
                    const auto& payload = found->payload;
                    if (payload.is_object()) {
                        const int phaseOffset = phaseIndex > 0 ? phaseIndex - 1 : 0;
                        const int topicOffset = topicIndex > 0 ? topicIndex - 1 : 0;
                        if (safePhaseName.empty() && payload.contains("courseStructure") && payload["courseStructure"].is_array() && phaseOffset >= 0 && phaseOffset < static_cast<int>(payload["courseStructure"].size())) {
                            const auto& stage = payload["courseStructure"][phaseOffset];
                            if (stage.is_object() && stage.contains("stage") && stage["stage"].is_string()) safePhaseName = stage["stage"].get<std::string>();
                            if (safeTopic.empty() && stage.is_object() && stage.contains("topics") && stage["topics"].is_array() && topicOffset >= 0 && topicOffset < static_cast<int>(stage["topics"].size()) && stage["topics"][topicOffset].is_string()) {
                                safeTopic = stage["topics"][topicOffset].get<std::string>();
                            }
                        }
                        if (safePhaseName.empty() && payload.contains("roadmap") && payload["roadmap"].is_array() && phaseOffset >= 0 && phaseOffset < static_cast<int>(payload["roadmap"].size())) {
                            const auto& stage = payload["roadmap"][phaseOffset];
                            if (stage.is_object() && stage.contains("name") && stage["name"].is_string()) safePhaseName = stage["name"].get<std::string>();
                        }
                    }
                }
            }
            if (safeMode != "lite") safeMode = "deep";
            if (safeGoal.empty()) safeGoal = "学习";
            if (safePhaseName.empty()) safePhaseName = "当前阶段";
            if (safeTopic.empty()) safeTopic = safeGoal;
            const std::string referer = req.get_header_value("Referer");
            const bool force = (regenerate && std::string(regenerate) == "1") ||
                (forceLearn && std::string(forceLearn) == "1") || (retry && *retry);
            const bool forceFromPage = queryValueFromUrl(referer, "regenerate") == "1";
            const int sessionPhaseIndex = phaseIndex <= 0 ? 1 : phaseIndex;
            const int sessionTopicIndex = topicIndex <= 0 ? 1 : topicIndex;

            // 1) 会话恢复（有 courseId 且非强制重新生成时）
            if (!safeCourseId.empty() && !force && !forceFromPage) {
                auto session = gangyi::findLearningSession(db, safeCourseId, safeAnonymousId,
                    safeGoal, safeMode, sessionPhaseIndex, sessionTopicIndex);
                if (!session && sessionPhaseIndex == 1 && sessionTopicIndex == 1) {
                    session = gangyi::findLearningSession(db, safeCourseId, safeAnonymousId,
                        safeGoal, safeMode, 0, 0);
                }
                if (session && session->contains("content") && (*session)["content"].is_string()) {
                    try {
                        auto content = nlohmann::json::parse((*session)["content"].get<std::string>());
                        if (content.is_object()) {
                            content["notice"] = "已恢复上次生成的学习内容";
                            return crow::response(200, content.dump());
                        }
                    } catch (...) {}
                }
            }

            // 2) 生成微课
            std::vector<gangyi::SearchResource> resources;
            const auto answer = generator.generate(safeGoal, safePhaseName, safeTopic, safeMode, resources);
            nlohmann::json lessonSteps = nlohmann::json::array();
            for (const auto& step : answer.lessonSteps) {
                lessonSteps.push_back({{"title", step.title}, {"explanation", step.explanation}, {"example", step.example}, {"action", step.action}, {"check", step.check}});
            }
            nlohmann::json examples = nlohmann::json::array();
            for (const auto& item : answer.examples) {
                examples.push_back({{"title", item.title}, {"content", item.content}, {"solution", item.solution}});
            }
            nlohmann::json practice = nlohmann::json::array();
            for (const auto& item : answer.practice) {
                practice.push_back({{"title", item.title}, {"difficulty", item.difficulty}, {"task", item.task}, {"check", item.check}});
            }
            nlohmann::json quiz = nlohmann::json::array();
            for (const auto& item : answer.quiz) {
                quiz.push_back({{"question", item.question}, {"options", item.options}, {"answerIndex", item.answerIndex}, {"explanation", item.explanation}});
            }
            nlohmann::json references = nlohmann::json::array();
            for (const auto& item : answer.references) {
                references.push_back({{"title", item.title}, {"source", item.source}, {"url", item.url}, {"type", item.type}});
            }
            const nlohmann::json result = {
                {"inferredDomain", answer.inferredDomain},
                {"title", answer.title},
                {"summary", answer.summary},
                {"goal", safeGoal},
                {"mode", safeMode},
                {"phaseName", safePhaseName},
                {"topicTitle", safeTopic},
                {"keyConcepts", answer.keyConcepts},
                {"lessonSteps", lessonSteps},
                {"examples", examples},
                {"practice", practice},
                {"quiz", quiz},
                {"commonMistakes", answer.commonMistakes},
                {"checkpoint", answer.checkpoint},
                {"resourceSummary", answer.resourceSummary},
                {"references", references},
                {"notice", answer.notice ? nlohmann::json(*answer.notice) : nlohmann::json(nullptr)}
            };

            // 3) 保存会话（有 courseId 或 anonymousId 时），供下次恢复
            if (!safeCourseId.empty() || !safeAnonymousId.empty()) {
                try {
                    nlohmann::json sessionBody = {
                        {"courseId", safeCourseId.empty() ? nlohmann::json(nullptr) : nlohmann::json(safeCourseId)},
                        {"anonymousId", safeAnonymousId.empty() ? nlohmann::json(nullptr) : nlohmann::json(safeAnonymousId)},
                        {"goal", safeGoal}, {"mode", safeMode},
                        {"phaseIndex", sessionPhaseIndex}, {"phaseName", safePhaseName},
                        {"topicIndex", sessionTopicIndex}, {"topicTitle", safeTopic},
                        {"title", answer.title},
                        {"summary", answer.summary.empty() ? nlohmann::json(nullptr) : nlohmann::json(answer.summary)},
                        {"searchQuery", nullptr},
                        {"content", result.dump()},
                        {"references", references},
                        {"fallbackUsed", answer.notice.has_value()},
                        {"source", answer.notice.has_value() ? "fallback" : "ai"},
                    };
                    gangyi::upsertLearningSession(db, sessionBody);
                } catch (...) {}
            }
            return crow::response(200, result.dump());
        } catch (const gangyi::AIClientError& e) {
            // 错误→HTTP 映射（对齐 docs/ai-spec.md）：missing_config/auth_error→503，timeout→504，其余→502
            int code = 502;
            if (e.errorType == "missing_config" || e.errorType == "auth_error") code = 503;
            else if (e.errorType == "timeout") code = 504;
            return crow::response(code, nlohmann::json{{"error", publicAIErrorMessage(e)}, {"type", e.errorType}}.dump());
        } catch (const std::exception&) {
            return crow::response(502, nlohmann::json{{"error", "微课程生成失败，请稍后重试。"}}.dump());
        }
    });
#endif

    /* 用户账号与管理员 API 已移除。
    CROW_ROUTE(app, "/api/auth/register").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const auto result = gangyi::registerUser(db, body.value("email", ""), body.value("name", ""),
                body.value("password", ""), requestAnonymousId(req, &body));
            crow::response response(result.ok ? 201 : 400, nlohmann::json{{"ok", result.ok},
                {"error", result.error}, {"user", result.ok ? gangyi::publicUser(result.user) : nlohmann::json(nullptr)}}.dump());
            if (result.ok) response.set_header("Set-Cookie", "ailines_session=" + result.token + "; Path=/; HttpOnly; SameSite=Lax; Max-Age=2592000");
            return response;
        } catch (...) { return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容无效"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/auth/login").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const auto result = gangyi::loginUser(db, body.value("email", ""), body.value("password", ""), requestAnonymousId(req, &body));
            crow::response response(result.ok ? 200 : 401, nlohmann::json{{"ok", result.ok},
                {"error", result.error}, {"user", result.ok ? gangyi::publicUser(result.user) : nlohmann::json(nullptr)}}.dump());
            if (result.ok) response.set_header("Set-Cookie", "ailines_session=" + result.token + "; Path=/; HttpOnly; SameSite=Lax; Max-Age=2592000");
            return response;
        } catch (...) { return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容无效"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/auth/me")([&db](const crow::request& req) {
        const auto user = gangyi::currentUser(db, req);
        return crow::response(200, nlohmann::json{{"ok", static_cast<bool>(user)},
            {"user", user ? gangyi::publicUser(*user) : nlohmann::json(nullptr)}}.dump());
    });

    CROW_ROUTE(app, "/api/auth/logout").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        gangyi::logoutUser(db, req);
        crow::response response(200, nlohmann::json{{"ok", true}}.dump());
        response.set_header("Set-Cookie", "ailines_session=; Path=/; HttpOnly; SameSite=Lax; Max-Age=0");
        return response;
    });

    CROW_ROUTE(app, "/api/admin/overview")([&db, &config](const crow::request& req) {
        const auto requester = gangyi::currentUser(db, req);
        if (!requester || !isAdmin(*requester, config)) return crow::response(requester ? 403 : 401, nlohmann::json{{"error", requester ? "你没有访问管理员后台的权限。" : "请先登录管理员账号。"}}.dump());
        int freeUsers = 0, proUsers = 0, maxUsers = 0, recentUsers = 0, recentCourses = 0;
        const auto daysAgo = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - std::chrono::hours(24 * 7));
        std::tm utc{};
#ifdef _WIN32
        gmtime_s(&utc, &daysAgo);
#else
        gmtime_r(&daysAgo, &utc);
#endif
        std::ostringstream date;
        date << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
        const auto users = db.listUsers();
        const auto courses = db.listCourses();
        for (const auto& user : users) {
            if (user.membershipTier == "pro") ++proUsers; else if (user.membershipTier == "max") ++maxUsers; else ++freeUsers;
            if (user.createdAt >= date.str()) ++recentUsers;
        }
        for (const auto& course : courses) if (course.createdAt >= date.str()) ++recentCourses;
        return crow::response(200, nlohmann::json{{"stats", {{"totalUsers", static_cast<int>(users.size())}, {"freeUsers", freeUsers}, {"proUsers", proUsers}, {"maxUsers", maxUsers}, {"totalCourses", static_cast<int>(courses.size())}, {"recentUsers", recentUsers}, {"recentCourses", recentCourses}}}}.dump());
    });

    CROW_ROUTE(app, "/api/admin/users")([&db, &config](const crow::request& req) {
        const auto requester = gangyi::currentUser(db, req);
        if (!requester || !isAdmin(*requester, config)) return crow::response(requester ? 403 : 401, nlohmann::json{{"error", requester ? "你没有访问管理员后台的权限。" : "请先登录管理员账号。"}}.dump());
        const std::string query = req.url_params.get("q") ? req.url_params.get("q") : "";
        const std::string tier = req.url_params.get("tier") ? req.url_params.get("tier") : "";
        nlohmann::json items = nlohmann::json::array();
        for (const auto& user : db.listUsers()) {
            if ((!query.empty() && user.email.find(query) == std::string::npos) || (!tier.empty() && user.membershipTier != tier)) continue;
            int courseCount = 0;
            for (const auto& course : db.listCourses()) if (course.userId.value_or("") == user.id) ++courseCount;
            items.push_back({{"id", user.id}, {"email", user.email}, {"name", user.name.value_or("")}, {"tier", user.membershipTier}, {"membershipStatus", user.membershipStatus}, {"createdAt", user.createdAt}, {"updatedAt", user.updatedAt}, {"lastActiveAt", user.updatedAt}, {"courseCount", courseCount}});
        }
        return crow::response(200, nlohmann::json{{"users", items}}.dump());
    });

    CROW_ROUTE(app, "/api/admin/courses")([&db, &config](const crow::request& req) {
        const auto requester = gangyi::currentUser(db, req);
        if (!requester || !isAdmin(*requester, config)) return crow::response(requester ? 403 : 401, nlohmann::json{{"error", requester ? "你没有访问管理员后台的权限。" : "请先登录管理员账号。"}}.dump());
        nlohmann::json items = nlohmann::json::array();
        for (const auto& course : db.listCourses()) {
            const auto owner = course.userId ? db.getUser(*course.userId) : std::nullopt;
            items.push_back({{"id", course.id}, {"title", course.title}, {"goal", course.goal}, {"mode", course.mode}, {"status", course.status}, {"ownerEmail", owner ? nlohmann::json(owner->email) : nlohmann::json(nullptr)}, {"createdAt", course.createdAt}, {"updatedAt", course.updatedAt}, {"planUrl", "/plan?courseId=" + encodeQueryValue(course.id)}});
        }
        return crow::response(200, nlohmann::json{{"courses", items}}.dump());
    });

    CROW_ROUTE(app, "/api/admin/users/<string>/tier").methods(crow::HTTPMethod::POST)([&db, &config](const crow::request& req, std::string userId) {
        const auto requester = gangyi::currentUser(db, req);
        if (!requester || !isAdmin(*requester, config)) return crow::response(requester ? 403 : 401, nlohmann::json{{"error", requester ? "你没有访问管理员后台的权限。" : "请先登录管理员账号。"}}.dump());
        try {
            const std::string tier = nlohmann::json::parse(req.body).value("tier", "");
            if (tier != "free" && tier != "pro" && tier != "max") return crow::response(400, nlohmann::json{{"error", "会员等级参数不正确。"}}.dump());
            auto user = db.getUser(userId);
            if (!user) return crow::response(404, nlohmann::json{{"error", "用户不存在。"}}.dump());
            user->membershipTier = tier;
            if (!db.update(*user)) return crow::response(500, nlohmann::json{{"error", "会员等级更新失败。"}}.dump());
            return crow::response(200, nlohmann::json{{"user", gangyi::publicUser(*user)}}.dump());
        } catch (...) { return crow::response(400, nlohmann::json{{"error", "请求内容无效"}}.dump()); }
    });
    */

    CROW_ROUTE(app, "/api/ask").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string question = body.value("question", "");
            const std::string conversationId = body.value("conversationId", std::string("general")).substr(0, 100);
            gangyi::AIClient ai;
            gangyi::AskGenerator generator(ai);
            const auto answer = generator.generate(question, gangyi::learningContext(db), chatHistory(db, conversationId, body));
            recordInteraction(db, "chat-user", {{"text", question}, {"topic", ""}}, "", conversationId);
            recordInteraction(db, "chat-assistant", {{"text", answer.content}, {"model", answer.model}}, "", conversationId);
            return crow::response(200, nlohmann::json{{"ok", true}, {"answer", answer.content},
                {"model", answer.model}, {"searchStatus", answer.searchStatus}, {"sources", answer.sources},
                {"profileVersion", db.profileAssessedRevision()}}.dump());
        } catch (const gangyi::AIClientError& error) {
            return crow::response(aiHttpStatus(error), nlohmann::json{{"ok", false},
                {"error", publicAIErrorMessage(error)}, {"type", error.errorType}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        }
    });

    // 追问建议：一次轻量请求；任何失败都返回空数组，前端会退回固定建议。
    CROW_ROUTE(app, "/api/ask/suggestions").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        nlohmann::json suggestions = nlohmann::json::array();
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string question = utf8Truncate(body.value("question", std::string()), 2000);
            const std::string answer = utf8Truncate(body.value("answer", std::string()), 6000);
            const std::string topic = utf8Truncate(body.value("topic", std::string()), 120);
            if (!question.empty() && !answer.empty()) {
                gangyi::AIClient ai;
                gangyi::ChatOptions options;
                options.messages.push_back({"system", u8"你是钢一定制AI。根据刚结束的一轮问答，给出三条学生最可能继续追问的短问题。每条不超过 24 个汉字，必须指向本轮话题的具体下一步，不要重复原问题。只输出 JSON。"});
                options.messages.push_back({"user", nlohmann::json{{"question", question}, {"answer", answer},
                    {"topic", topic}}.dump() + u8"\n输出格式：{\"suggestions\":[\"\",\"\",\"\"]}"});
                options.temperature = 0.6;
                options.maxTokens = 8192;
                options.timeoutMs = 60000;
                options.maxAttempts = 1;
                const auto parsed = gangyi::parseAIJson(ai.chat(options).content);
                if (parsed.is_object() && parsed.contains("suggestions") && parsed["suggestions"].is_array()) {
                    for (const auto& item : parsed["suggestions"]) {
                        if (!item.is_string()) continue;
                        const std::string text = item.get<std::string>();
                        if (text.empty() || text.size() > 120) continue;
                        suggestions.push_back(text);
                        if (suggestions.size() >= 3) break;
                    }
                }
            }
        } catch (...) {}
        return crow::response(200, nlohmann::json{{"ok", true}, {"suggestions", suggestions}}.dump());
    });

    // 课堂内上下文对话：问题必须携带当前课程目标、阶段和主题，避免退化为全局闲聊。
    CROW_ROUTE(app, "/api/learning-chat").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string question = body.value("question", "");
            if (question.find_first_not_of(" \t\r\n") == std::string::npos) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"type", "invalid_request"}, {"error", "请先输入问题"}}.dump());
            }
            nlohmann::json context = {
                {"pageType", body.value("pageType", "learn")},
                {"goal", body.value("goal", "")},
                {"mode", body.value("mode", "deep")},
                {"phaseName", body.value("phaseName", "")},
                {"topic", body.value("topic", body.value("topicTitle", ""))},
                {"contextTitle", body.value("contextTitle", "")},
                {"contextSummary", body.value("contextSummary", "")}
            };
            std::vector<gangyi::ChatMessage> messages;
            messages.push_back({"system", u8"你是钢一定制AI的课堂辅导老师。请严格结合给定的课程目标、阶段和当前主题回答问题；优先解释当前主题，必要时给出分步骤示例和练习提示。默认使用简体中文，不要编造资料，不要把无关主题的内容混进来。\n课堂上下文：" + context.dump() + u8"\n本机学习画像：" + gangyi::learningContext(db), ""});
            if (body.contains("messages") && body["messages"].is_array()) {
                const auto& history = body["messages"];
                const size_t start = history.size() > 8 ? history.size() - 8 : 0;
                for (size_t i = start; i < history.size(); ++i) {
                    if (!history[i].is_object()) continue;
                    const std::string role = history[i].value("role", "user");
                    const std::string content = history[i].value("content", "");
                    if (i + 1 == history.size() && role == "user" && content == question) continue;
                    if ((role == "user" || role == "assistant") && !content.empty()) messages.push_back({role, content, ""});
                }
            }
            messages.push_back({"user", question, ""});
            gangyi::ChatOptions options;
            options.messages = std::move(messages);
            options.temperature = 0.45;
            options.maxTokens = 8192;
            options.timeoutMs = 60000;
            options.maxAttempts = 1;
            const std::string searchCourseId = body.value("courseId", "");
            const auto searchCourse = searchCourseId.empty() ? std::optional<gangyi::Course>{} : db.getCourse(searchCourseId);
            options.searchQuery = searchCourse ? searchCourse->goal : u8"高中学科知识";
            gangyi::AIClient ai;
            const auto result = ai.chat(options);
            if (result.content.empty()) throw gangyi::AIClientError("invalid_response", "AI 没有返回课堂回答");
            const std::string courseId = body.value("courseId", "");
            const std::string conversationId = body.value("conversationId", courseId.empty() ? "lesson" : "course-" + courseId).substr(0, 100);
            recordInteraction(db, "chat-user", {{"text", question}, {"topic", context.value("topic", "")}}, courseId, conversationId);
            recordInteraction(db, "chat-assistant", {{"text", result.content}, {"model", result.model}}, courseId, conversationId);
            return crow::response(200, nlohmann::json{{"ok", true}, {"answer", result.content},
                {"model", result.model}, {"searchStatus", result.searchStatus}, {"sources", result.sources},
                {"profileVersion", db.profileAssessedRevision()}}.dump());
        } catch (const gangyi::AIClientError& error) {
            return crow::response(aiHttpStatus(error), nlohmann::json{{"ok", false}, {"type", error.errorType},
                {"error", publicAIErrorMessage(error)}, {"canRetry", true}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"type", "invalid_request"},
                {"error", "课堂问题格式不正确"}}.dump());
        }
    });

    std::mutex socketMapMutex;
    std::atomic<int> socketWorkerCount{0};
    std::unordered_map<crow::websocket::connection*, std::shared_ptr<ClassroomSocket>> socketMap;
    const auto socketOpen = [&](crow::websocket::connection& connection) {
            auto state = std::make_shared<ClassroomSocket>();
            state->connection = &connection;
            std::lock_guard<std::mutex> guard(socketMapMutex);
            socketMap[&connection] = std::move(state);
        };
    const auto socketClose = [&](crow::websocket::connection& connection, const std::string&, uint16_t) {
            std::shared_ptr<ClassroomSocket> state;
            {
                std::lock_guard<std::mutex> guard(socketMapMutex);
                const auto it = socketMap.find(&connection);
                if (it != socketMap.end()) { state = it->second; socketMap.erase(it); }
            }
            if (state) {
                state->cancelled = true;
                std::lock_guard<std::mutex> guard(state->mutex);
                state->connection = nullptr;
            }
        };
    const auto socketMessage = [&](crow::websocket::connection& connection, const std::string& message, bool binary, bool classroom) {
        std::shared_ptr<ClassroomSocket> state;
        {
            std::lock_guard<std::mutex> guard(socketMapMutex);
            const auto it = socketMap.find(&connection);
            if (it != socketMap.end()) state = it->second;
        }
        if (!state || binary) return;
        const auto body = nlohmann::json::parse(message, nullptr, false);
        if (!body.is_object() || !body.contains("type") || !body["type"].is_string()) {
            state->send({{"type", "error"}, {"message", "消息格式无效"}}); return;
        }
        if (body["type"] == "stop") { state->cancelled = true; return; }
        if (body["type"] != "ask" || state->busy.exchange(true)) return;
        state->cancelled = false;
        ++socketWorkerCount;
        std::thread([state, body, classroom, &db, &socketWorkerCount] {
            nlohmann::json terminal;
            try {
                const std::string question = body.at("question").get<std::string>();
                std::string courseId, title, selection, conversation = "general";
                nlohmann::json dialogueTurn;
                const bool dialogue = classroom && body.contains("kind");
                gangyi::ChatOptions options;
                if (classroom) {
                    courseId = body.at("courseId").get<std::string>();
                    const int phase = body.at("phaseIndex").get<int>(), topic = body.at("topicIndex").get<int>();
                    selection = body.value("selection", "");
                    title = savedTopic(db, courseId, phase, topic);
                    if (title.empty() || question.find_first_not_of(" \t\r\n") == std::string::npos ||
                        question.size() > 4000 || selection.size() > 2000)
                        throw std::invalid_argument("问题或课时无效");
                    const auto course = db.getCourse(courseId);
                    if (!course || course->status != "active") throw std::invalid_argument("课程不存在");
                    const auto session = db.findLearningSession(courseId, phase, topic);
                    conversation = "lesson-" + courseId + "-" + std::to_string(phase) + "-" + std::to_string(topic);
                    options.temperature = 0.4;
                    options.maxTokens = 8192;
                    options.maxAttempts = 1;
                    options.searchQuery = course->goal;
                    options.messages.push_back({"system", u8"你是课堂辅导老师，只回答当前已保存课程和课时相关的问题。先定位具体误解，再引导学生思考；默认简体中文。课堂上下文：" +
                        nlohmann::json{{"goal", course->goal}, {"topic", title},
                            {"summary", session ? session->summary.value_or("") : ""}}.dump() +
                        u8"\n最新学习上下文：" + gangyi::learningContext(db, courseId)});
                    for (const auto& item : chatHistory(db, conversation)) options.messages.push_back(item);
                    options.messages.push_back({"user", selection.empty() ? question : u8"选中文字：" + selection + u8"\n问题：" + question});
                    if (dialogue) {
                        dialogueTurn = gangyi::beginDialogue(db, body);
                        if (dialogueTurn.value("cached", false)) {
                            state->send({{"type", "delta"}, {"text", dialogueTurn.value("answer", "")}});
                            state->send({{"type", "done"}, {"version", dialogueTurn["version"]}, {"cached", true}});
                            state->busy = false; --socketWorkerCount; return;
                        }
                        options = gangyi::dialogueOptions(db, body, dialogueTurn);
                    }
                } else {
                    options = gangyi::AskGenerator::options(question, gangyi::learningContext(db), chatHistory(db, conversation, body));
                }
                options.timeoutMs = 60000;
                options.cancelled = [state] { return state->cancelled.load(); };
                gangyi::AIClient ai;
                const auto result = ai.chatStream(options, [state](const std::string& chunk) {
                    if (state->cancelled) return false;
                    state->send({{"type", "delta"}, {"text", chunk}});
                    return !state->cancelled;
                });
                if (state->cancelled) terminal = {{"type", "done"}, {"message", "已停止"}, {"cancelled", true}};
                else {
                    if (!dialogue) {
                        recordInteraction(db, "chat-user", {{"text", question}, {"topic", title}, {"selection", selection},
                            {"phaseIndex", body.value("phaseIndex", 0)}, {"topicIndex", body.value("topicIndex", 0)}}, courseId, conversation);
                        recordInteraction(db, "chat-assistant", {{"text", result.content}, {"model", result.model}}, courseId, conversation);
                    }
                    terminal = {{"type", "done"}, {"model", result.model}, {"searchStatus", result.searchStatus},
                                {"sources", result.sources}, {"profileVersion", db.profileAssessedRevision()}, {"cancelled", false}};
                    if (dialogue) {
                        const auto saved = gangyi::finishDialogue(db, body, dialogueTurn, result.content, result.model);
                        for (const auto& field : saved.items()) terminal[field.key()] = field.value();
                    }
                }
            } catch (const gangyi::AIClientError& error) {
                terminal = {{"type", error.errorType == "cancelled" ? "done" : "error"},
                            {"cancelled", error.errorType == "cancelled"},
                            {"message", error.errorType == "cancelled" ? "已停止" : publicAIErrorMessage(error)}};
            } catch (const std::exception&) {
                terminal = {{"type", "error"}, {"message", "问题格式无效或回答未保存，请重试。"}};
            }
            if (classroom && !terminal.is_null() && (terminal.value("cancelled", false) || terminal.value("type", "") == "error") && body.contains("kind")) {
                // 中断状态由对话接口保存，刷新后可以重新发送同一次用户输入。
                try {
                    auto view = gangyi::dialogueView(db, body);
                    const auto row = db.getClassroomActivity(gangyi::classroomKey(body.at("courseId"), body.at("phaseIndex"), body.at("topicIndex")) + ":" + body.at("kind").get<std::string>() + ":" + std::to_string(body.at("index").get<int>()) + ":dialogue");
                    if (row && view.value("turns", nlohmann::json::array()).size()) {
                        auto thread = nlohmann::json::parse(row->payload);
                        if (thread["turns"].back().value("status", "") == "pending" &&
                            thread["turns"].back().value("requestId", "") == body.value("requestId", "")) {
                            thread["turns"].back()["status"] = "failed";
                            thread["turns"].back()["error"] = terminal.value("message", "等待 AI 回复，可重试。");
                            thread["version"] = thread.value("version", 0) + 1;
                            auto saved = *row; saved.payload = thread.dump(); db.compareClassroomActivity(saved, row->payload);
                            terminal["version"] = thread["version"];
                        }
                    }
                } catch (...) { /* 保留用户输入，后台启动时恢复中断状态。 */ }
            }
            state->busy = false;
            state->send(terminal);
            --socketWorkerCount;
        }).detach();
    };
    CROW_WEBSOCKET_ROUTE(app, "/ws/classroom").onopen(socketOpen).onclose(socketClose)
        .onmessage([&](crow::websocket::connection& connection, const std::string& message, bool binary) {
            socketMessage(connection, message, binary, true);
        });
    CROW_WEBSOCKET_ROUTE(app, "/ws/ask").onopen(socketOpen).onclose(socketClose)
        .onmessage([&](crow::websocket::connection& connection, const std::string& message, bool binary) {
            socketMessage(connection, message, binary, false);
        });

    CROW_ROUTE(app, "/api/classroom/start")([&db](const crow::request& req) {
        try {
            const auto value = [&req](const char* name) { const char* raw = req.url_params.get(name); return std::string(raw ? raw : ""); };
            const std::string courseId = value("courseId"), kind = value("kind");
            const int phase = std::stoi(value("phaseIndex")), topic = std::stoi(value("topicIndex"));
            if (courseId.empty() || !requesterCanAccessCourse(db, req, courseId, ""))
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在"}}.dump());
            const std::string title = savedTopic(db, courseId, phase, topic);
            if (title.empty()) return crow::response(400, nlohmann::json{{"ok", false}, {"error", "课时不存在"}}.dump());
            return crow::response(200, gangyi::classroomStart(db, courseId, phase, topic, kind, title, gangyi::learningContext(db, courseId)).dump());
        } catch (const std::exception& error) {
            return crow::response(503, nlohmann::json{{"ok", false}, {"error", "活动题准备失败，可先继续阅读课程。"}}.dump());
        }
    });

    CROW_ROUTE(app, "/api/classroom/submit").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.at("courseId").get<std::string>();
            const int phase = body.at("phaseIndex").get<int>(), topic = body.at("topicIndex").get<int>();
            if (!requesterCanAccessCourse(db, req, courseId, "") || savedTopic(db, courseId, phase, topic).empty())
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课时不存在"}}.dump());
            auto submission = body;
            const auto view = gangyi::dialogueView(db, body);
            const auto answer = body.at("answer");
            if (!answer.is_string() && !answer.is_number_integer()) throw std::invalid_argument("回答格式无效");
            std::string text = answer.is_string() ? answer.get<std::string>() : "";
            if (answer.is_number_integer()) {
                const int choice = answer.get<int>(); const auto choices = view["question"].value("options", nlohmann::json::array());
                if (choice == 4) text = "我暂时不会这道题";
                else if (choice < 0 || choice >= static_cast<int>(choices.size())) throw std::invalid_argument("选择无效");
                else text = choices[choice].get<std::string>();
            }
            submission["question"] = text;
            submission["questionId"] = body.value("questionId", view.value("questionId", ""));
            submission["version"] = body.value("version", view.value("version", 0));
            submission["requestId"] = body.value("requestId", "compat:" + gangyi::questionIdentity(courseId, nlohmann::json{{"question", view["questionId"]}, {"answer", answer}}));
            const auto turn = gangyi::beginDialogue(db, submission);
            if (!turn.value("cached", false)) {
                try {
                    gangyi::AIClient ai; const auto response = ai.chat(gangyi::dialogueOptions(db, submission, turn));
                    if (response.content.empty() || response.finishReason == "length") throw std::runtime_error("AI 回答不完整");
                    gangyi::finishDialogue(db, submission, turn, response.content, response.model);
                } catch (...) {
                    gangyi::finishDialogue(db, submission, turn, "", "", "等待 AI 回答，输入保留，可重试。");
                    throw;
                }
            }
            auto result = gangyi::dialogueView(db, submission); result["ok"] = true;
            result["pending"] = result.value("evaluationStatus", "") == "pending";
            return crow::response(200, result.dump());
        } catch (const std::invalid_argument& error) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", error.what()}}.dump());
        } catch (...) {
            return crow::response(503, nlohmann::json{{"ok", false}, {"error", "活动暂时未保存，请稍后重试。"}}.dump());
        }
    });

    CROW_ROUTE(app, "/api/classroom/skip").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.at("courseId").get<std::string>();
            const int phase = body.at("phaseIndex").get<int>(), topic = body.at("topicIndex").get<int>();
            if (!requesterCanAccessCourse(db, req, courseId, "") || savedTopic(db, courseId, phase, topic).empty())
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课时不存在"}}.dump());
            return crow::response(200, gangyi::classroomSkip(db, courseId, phase, topic).dump());
        } catch (...) { return crow::response(400, nlohmann::json{{"ok", false}, {"error", "跳过状态保存失败"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/activity/skip").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.at("courseId").get<std::string>();
            const int phase = body.at("phaseIndex").get<int>(), topic = body.at("topicIndex").get<int>();
            if (!requesterCanAccessCourse(db, req, courseId, "") || savedTopic(db, courseId, phase, topic).empty())
                return crow::response(404, "{}");
            return crow::response(200, gangyi::classroomSkipActivity(db, courseId, phase, topic,
                body.at("index").get<int>()).dump());
        } catch (...) { return crow::response(400, nlohmann::json{{"error", "活动跳过失败"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/state")([&db](const crow::request& req) {
        try {
            const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
            const int phase = std::stoi(req.url_params.get("phaseIndex")), topic = std::stoi(req.url_params.get("topicIndex"));
            if (!requesterCanAccessCourse(db, req, courseId, "") || savedTopic(db, courseId, phase, topic).empty())
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课时不存在"}}.dump());
            return crow::response(200, gangyi::classroomState(db, courseId, phase, topic).dump());
        } catch (...) { return crow::response(400, nlohmann::json{{"ok", false}, {"error", "课时参数无效"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/hint").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.at("courseId").get<std::string>();
            const int phase = body.at("phaseIndex").get<int>(), topic = body.at("topicIndex").get<int>();
            if (!requesterCanAccessCourse(db, req, courseId, "") || savedTopic(db, courseId, phase, topic).empty())
                return crow::response(404, "{}");
            return crow::response(200, gangyi::classroomHint(db, courseId, phase, topic,
                body.at("kind").get<std::string>(), body.at("index").get<int>()).dump());
        } catch (...) { return crow::response(400, nlohmann::json{{"error", "提示暂时不可用"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/remedial")([&db](const crow::request& req) {
        try {
            const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
            const int phase = std::stoi(req.url_params.get("phaseIndex")), topic = std::stoi(req.url_params.get("topicIndex"));
            if (!requesterCanAccessCourse(db, req, courseId, "") || savedTopic(db, courseId, phase, topic).empty())
                return crow::response(404, "{}");
            return crow::response(200, gangyi::classroomRemedial(db, courseId, phase, topic).dump());
        } catch (...) { return crow::response(503, nlohmann::json{{"error", "补讲暂未就绪，可继续完整课堂。"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/review/submit").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.at("courseId").get<std::string>();
            const int phase = body.at("phaseIndex").get<int>(), topic = body.at("topicIndex").get<int>();
            if (!requesterCanAccessCourse(db, req, courseId, "") || savedTopic(db, courseId, phase, topic).empty())
                return crow::response(404, "{}");
            return crow::response(200, gangyi::submitReview(db, courseId, phase, topic,
                body.at("day").get<int>(), body.at("answers"), todayDate(),
                body.value("reviewId", ""), body.value("questions", nlohmann::json())).dump());
        } catch (const std::invalid_argument& error) { return crow::response(400, nlohmann::json{{"error", error.what()}}.dump()); }
        catch (const std::runtime_error& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
        catch (...) { return crow::response(503, nlohmann::json{{"error", "复习结果未保存"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/reviews")([&db](const crow::request& req) {
        const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
        if (!requesterCanAccessCourse(db, req, courseId, "")) return crow::response(404, "{}");
        try { return crow::response(200, nlohmann::json{{"items", gangyi::dueReviews(db, courseId, todayDate())}}.dump()); }
        catch (...) { return crow::response(503, "{}"); }
    });

    CROW_ROUTE(app, "/api/classroom/week")([&db](const crow::request& req) {
        const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
        const auto found = gangyi::getCourseWithSnapshot(db, courseId);
        if (!found || !requesterCanReadCourse(db, found->course, req)) return crow::response(404, "{}");
        try {
            auto plan = gangyi::studyPlanView(db); auto entries = nlohmann::json::array();
            for (const auto& entry : plan["entries"]) if (entry.value("courseId", "") == courseId) entries.push_back(entry);
            plan["entries"] = entries; plan["sharedBudget"] = true;
            return crow::response(200, plan.dump());
        }
        catch (...) { return crow::response(503, nlohmann::json{{"error", "周计划读取失败"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/week/edit").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.at("courseId").get<std::string>();
            if (!requesterCanAccessCourse(db, req, courseId, "")) return crow::response(404, "{}");
            auto all = gangyi::studyPlanView(db);
            if (body.contains("entries")) {
                auto merged = nlohmann::json::array();
                for (const auto& entry : all["entries"]) if (entry.value("courseId", "") != courseId) merged.push_back(entry);
                for (auto entry : body["entries"]) {
                    entry["courseId"] = courseId;
                    if (!entry.contains("taskId")) entry["taskId"] = "lesson:" + courseId + ":" +
                        std::to_string(entry.value("phaseIndex", 1)) + ":" + std::to_string(entry.value("topicIndex", 1));
                    merged.push_back(entry);
                }
                body["entries"] = merged;
            }
            return crow::response(200, gangyi::editStudyPlan(db, body).dump());
        } catch (const std::invalid_argument& error) {
            return crow::response(409, nlohmann::json{{"error", error.what()}}.dump());
        } catch (...) { return crow::response(400, nlohmann::json{{"error", "周计划保存失败"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/week/replan").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.at("courseId").get<std::string>();
            const auto found = gangyi::getCourseWithSnapshot(db, courseId);
            if (!found || !requesterCanReadCourse(db, found->course, req)) return crow::response(404, "{}");
            if (!body.contains("version")) body["version"] = gangyi::studyPlanView(db)["version"];
            if (body.contains("previewId")) body["proposalId"] = body["previewId"];
            if (body.contains("entries")) {
                auto all = gangyi::studyPlanView(db), merged = nlohmann::json::array();
                for (const auto& entry : all["entries"]) if (entry.value("courseId", "") != courseId) merged.push_back(entry);
                for (auto entry : body["entries"]) {
                    entry["courseId"] = courseId;
                    if (!entry.contains("taskId")) entry["taskId"] = "lesson:" + courseId + ":" +
                        std::to_string(entry.value("phaseIndex", 1)) + ":" + std::to_string(entry.value("topicIndex", 1));
                    merged.push_back(entry);
                }
                body["entries"] = merged;
            }
            return crow::response(200, gangyi::studyPlanProposal(db, body, body.value("confirm", false) ? "confirm" : "replan").dump());
        } catch (const std::invalid_argument& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
        catch (...) { return crow::response(400, nlohmann::json{{"error", "重排失败"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/finish").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.at("courseId").get<std::string>();
            const int phase = body.at("phaseIndex").get<int>(), topic = body.at("topicIndex").get<int>();
            if (!requesterCanAccessCourse(db, req, courseId, "") || savedTopic(db, courseId, phase, topic).empty())
                return crow::response(404, "{}");
            const auto progress = db.findLearningCardProgress(courseId, phase, topic);
            if (!progress || progress->status != "completed")
                return crow::response(409, nlohmann::json{{"error", "请先保存本节完成状态"}}.dump());
            bool passed = false;
            for (const auto& row : db.listInteractions()) {
                if (row.kind != "quiz" || row.courseId.value_or("") != courseId) continue;
                const auto attempt = nlohmann::json::parse(row.payload, nullptr, false);
                if (attempt.is_object() && attempt.value("phaseIndex", 0) == phase && attempt.value("topicIndex", 0) == topic)
                    passed = attempt.value("score", 0) * 10 >= attempt.value("total", 1) * 7;
            }
            return crow::response(200, gangyi::finishClassroom(db, courseId, phase, topic, passed, todayDate()).dump());
        } catch (...) { return crow::response(503, nlohmann::json{{"error", "下一步暂未保存，原课程仍可继续。"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/recent-courses")([&db] {
        crow::response response(200, nlohmann::json{{"courses", gangyi::recentCourses(db)}}.dump());
        response.set_header("Content-Type", "application/json; charset=utf-8");
        response.set_header("Cache-Control", "no-store");
        return response;
    });

    CROW_ROUTE(app, "/api/profile")([&db] {
        crow::response response(200, gangyi::profileView(db).dump());
        response.set_header("Cache-Control", "no-store");
        return response;
    });

    CROW_ROUTE(app, "/api/classroom/dialogue")([&db](const crow::request& req) {
        try {
            const auto value = [&](const char* name) { const char* raw = req.url_params.get(name); return std::string(raw ? raw : ""); };
            nlohmann::json body = {{"courseId", value("courseId")}, {"phaseIndex", std::stoi(value("phaseIndex"))},
                {"topicIndex", std::stoi(value("topicIndex"))}, {"kind", value("kind")}, {"index", std::stoi(value("index"))}};
            crow::response response(200, gangyi::dialogueView(db, body).dump()); response.set_header("Cache-Control", "no-store"); return response;
        } catch (...) { return crow::response(404, nlohmann::json{{"error", "题目或对话不存在"}}.dump()); }
    });
    CROW_ROUTE(app, "/api/classroom/evaluation/retry").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            gangyi::dialogueView(db, body);
            const auto id = gangyi::classroomKey(body.at("courseId"), body.at("phaseIndex"), body.at("topicIndex")) + ":" +
                body.at("kind").get<std::string>() + ":" + std::to_string(body.at("index").get<int>()) + ":evaluation";
            const auto row = db.getClassroomActivity(id); if (!row) throw std::invalid_argument("没有待重试的评价");
            auto payload = nlohmann::json::parse(row->payload); payload["status"] = "pending";
            auto next = *row; next.payload = payload.dump();
            if (!db.compareClassroomActivity(next, row->payload)) throw std::invalid_argument("评价已变化");
            return crow::response(202, nlohmann::json{{"ok", true}}.dump());
        } catch (...) { return crow::response(409, nlohmann::json{{"error", "评价状态已变化，请刷新"}}.dump()); }
    });
    CROW_ROUTE(app, "/api/learn/exposure").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try { return crow::response(200, gangyi::exposeLearningBlocks(db, nlohmann::json::parse(req.body)).dump()); }
        catch (...) { return crow::response(409, nlohmann::json{{"error", "课堂版本变化，请刷新"}}.dump()); }
    });
    CROW_ROUTE(app, "/api/classroom/preparation")([&db](const crow::request& req) {
        const char* course = req.url_params.get("courseId");
        crow::response response(200, gangyi::preparationView(db, course ? course : "").dump());
        response.set_header("Cache-Control", "no-store"); return response;
    });
    CROW_ROUTE(app, "/api/classroom/preparation/retry").methods(crow::HTTPMethod::POST)([&db](const crow::request&) {
        db.markLearningDirty(); return crow::response(202, nlohmann::json{{"ok", true}, {"status", "pending"}}.dump());
    });
    CROW_ROUTE(app, "/api/study-plan")([&db](const crow::request&) {
        crow::response response(200, gangyi::studyPlanView(db).dump()); response.set_header("Cache-Control", "no-store"); return response;
    });
    CROW_ROUTE(app, "/api/study-plan").methods(crow::HTTPMethod::PUT)([&db](const crow::request& req) {
        try { return crow::response(200, gangyi::editStudyPlan(db, nlohmann::json::parse(req.body)).dump()); }
        catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/study-plan/draft").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try { return crow::response(200, gangyi::studyPlanDraft(db, nlohmann::json::parse(req.body)).dump()); }
        catch (...) { return crow::response(409, nlohmann::json{{"error", "编辑状态无效，请刷新"}}.dump()); }
    });
    CROW_ROUTE(app, "/api/study-plan/<string>").methods(crow::HTTPMethod::POST)([&db](const crow::request& req, const std::string& action) {
        try { return crow::response(200, gangyi::studyPlanProposal(db, nlohmann::json::parse(req.body), action).dump()); }
        catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/home/next-step")([&db](const crow::request& req) {
        const char* course = req.url_params.get("courseId");
        crow::response response(200, gangyi::nextStepView(db, course ? course : "").dump());
        response.set_header("Cache-Control", "no-store"); return response;
    });
    CROW_ROUTE(app, "/api/courses/<string>/preview")([&db](const crow::request&, const std::string& course) {
        try { crow::response response(200, gangyi::coursePreviewView(db, course).dump()); response.set_header("Cache-Control", "no-store"); return response; }
        catch (...) { return crow::response(404, nlohmann::json{{"error", "课程不存在"}}.dump()); }
    });
    CROW_ROUTE(app, "/api/courses/<string>/preview").methods(crow::HTTPMethod::POST)([&db](const crow::request& req, const std::string& course) {
        try { const auto body = nlohmann::json::parse(req.body); return crow::response(202, gangyi::requestCoursePreview(db, course, body.value("retry", false)).dump()); }
        catch (...) { return crow::response(409, nlohmann::json{{"error", "预览状态变化，请刷新"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/profile/radar-preferences")([&db] {
        crow::response response(200, gangyi::radarPreferences(db).dump());
        response.set_header("Content-Type", "application/json; charset=utf-8");
        response.set_header("Cache-Control", "no-store");
        return response;
    });
    CROW_ROUTE(app, "/api/profile/radar-preferences").methods(crow::HTTPMethod::PUT)([&db](const crow::request& req) {
        try { return crow::response(200, gangyi::saveRadarPreferences(db, nlohmann::json::parse(req.body)).dump()); }
        catch (...) { return crow::response(400, nlohmann::json{{"error", "请选择九科中的六门不同学科，并指定有效的显示模式。"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/next-learning")([&db](const crow::request& req) {
        const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
        if (courseId.empty() || !requesterCanAccessCourse(db, req, courseId, requestAnonymousId(req)))
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在"}}.dump());
        auto result = gangyi::nextLearning(db, courseId);
        result["ok"] = true;
        return crow::response(200, result.dump());
    });

    CROW_ROUTE(app, "/api/topic-mastery")([&db](const crow::request& req) {
        const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
        if (courseId.empty() || !requesterCanAccessCourse(db, req, courseId, requestAnonymousId(req)))
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在"}}.dump());
        nlohmann::json topics = nlohmann::json::array();
        for (const auto& value : db.listTopicMastery()) if (value.courseId == courseId)
            topics.push_back({{"phaseIndex", value.phaseIndex}, {"topic", value.topic},
                {"score", value.score ? nlohmann::json(*value.score) : nlohmann::json(nullptr)},
                {"status", value.status}, {"rationale", value.rationale},
                {"weakPoints", nlohmann::json::parse(value.weakPoints, nullptr, false)},
                {"recommendation", value.recommendation}, {"evidenceCount", value.evidenceCount},
                {"evidenceIds", nlohmann::json::parse(value.evidenceIds, nullptr, false)},
                {"nextReviewAt", value.nextReviewAt}, {"version", value.version}, {"updatedAt", value.updatedAt}});
        return crow::response(200, nlohmann::json{{"ok", true}, {"topics", topics}}.dump());
    });

    CROW_ROUTE(app, "/api/home/recommendations")([&homeRecommendations, databasePath = config.database_path](const crow::request&) {
        gangyi::Database connection; connection.open(databasePath);
        auto current = gangyi::agentRecommendationView(connection, localAgentAccess(connection));
        if (current.empty()) current = homeRecommendations.view();
        crow::response response(current.dump());
        response.set_header("Content-Type", "application/json; charset=utf-8");
        response.set_header("Cache-Control", "no-store");
        return response;
    });

    CROW_ROUTE(app, "/api/learning-agent/events").methods(crow::HTTPMethod::POST)([databasePath = config.database_path](const crow::request& req) {
        try {
            gangyi::Database connection; connection.open(databasePath);
            const auto value = gangyi::agentSubmit(connection, localAgentAccess(connection), nlohmann::json::parse(req.body));
            crow::response response(202, value.dump()); response.set_header("Content-Type", "application/json; charset=utf-8");
            response.set_header("Cache-Control", "no-store"); return response;
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/learning-agent")([databasePath = config.database_path](const crow::request& req) {
        try {
            gangyi::Database connection; connection.open(databasePath);
            const auto access = localAgentAccess(connection);
            auto value = gangyi::agentView(connection, access, req.url_params.get("taskId") ? req.url_params.get("taskId") : "");
            value["adjustments"] = gangyi::agentChanges(connection, access);
            crow::response response(value.dump()); response.set_header("Content-Type", "application/json; charset=utf-8");
            response.set_header("Cache-Control", "no-store"); return response;
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/learning-agent/control").methods(crow::HTTPMethod::POST)([databasePath = config.database_path](const crow::request& req) {
        try {
            gangyi::Database connection; connection.open(databasePath);
            crow::response response(gangyi::agentControl(connection, localAgentAccess(connection), nlohmann::json::parse(req.body)).dump());
            response.set_header("Content-Type", "application/json; charset=utf-8"); return response;
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/learning-agent/lesson")([databasePath = config.database_path](const crow::request& req) {
        try {
            gangyi::Database connection; connection.open(databasePath);
            crow::response response(gangyi::agentLessonView(connection, localAgentAccess(connection),
                req.url_params.get("lessonId") ? req.url_params.get("lessonId") : "").dump());
            response.set_header("Content-Type", "application/json; charset=utf-8"); return response;
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_WEBSOCKET_ROUTE(app, "/ws/learning-agent").onopen(socketOpen).onclose(socketClose)
        .onmessage([&, databasePath = config.database_path](crow::websocket::connection& connection, const std::string& message, bool binary) {
            std::shared_ptr<ClassroomSocket> state;
            { std::lock_guard<std::mutex> guard(socketMapMutex); const auto found = socketMap.find(&connection);
              if (found != socketMap.end()) state = found->second; }
            if (!state || binary || state->busy.exchange(true)) return;
            const auto body = nlohmann::json::parse(message, nullptr, false);
            if (!body.is_object() || !body.value("taskId", nlohmann::json()).is_string()) {
                state->send({{"type", "failed"}, {"message", "任务标识无效。"}}); state->busy = false; return;
            }
            ++socketWorkerCount;
            std::thread([state, body, databasePath, &socketWorkerCount] {
                try {
                    gangyi::Database connection; connection.open(databasePath);
                    const auto access = localAgentAccess(connection); int sequence = body.value("afterSeq", 0);
                    while (!state->cancelled) {
                        const auto task = gangyi::agentView(connection, access, body.at("taskId"));
                        for (const auto& event : task.value("events", nlohmann::json::array())) if (event.value("seq", 0) > sequence) {
                            auto value = event; value["taskId"] = task["id"]; state->send(value); sequence = event["seq"];
                        }
                        state->send({{"type", "state"}, {"task", task}});
                        const auto status = task.value("status", "");
                        if (status != "pending" && status != "running") break;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                } catch (const std::exception& error) { state->send({{"type", "failed"}, {"message", error.what()}}); }
                state->busy = false; --socketWorkerCount;
            }).detach();
        });

    CROW_ROUTE(app, "/startup")([] {
        crow::response response(gangyi::renderStartupPage());
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/api/profile/refresh").methods(crow::HTTPMethod::POST)([databasePath = config.database_path] {
        try {
            gangyi::Database connection; connection.open(databasePath); connection.markProfileDirty();
            static std::atomic_uint64_t sequence{0};
            const auto task = gangyi::agentSubmit(connection, localAgentAccess(connection), {{"type", "profile_update"},
                {"requestId", "profile-" + nowIso8601() + "-" + std::to_string(++sequence)},
                {"text", "读取真实原题、实际回答、提示经历和可靠评价，判断哪些学科与能力维度已具备证据，并给出评分、范围和不确定性。不采用固定题数门槛；缺证据时解释还需要了解什么，保留其他有效画像。"}});
            return crow::response(202, nlohmann::json{{"ok", true}, {"updating", true}, {"taskId", task.at("id")}}.dump());
        } catch (const std::exception&) { return crow::response(503, nlohmann::json{{"error", "画像更新未提交，原有结果保留。"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/conversations/<string>")([&db](const crow::request&, std::string conversationId) {
        nlohmann::json messages = nlohmann::json::array();
        for (const auto& item : db.listInteractions()) {
            if (item.conversationId.value_or("") != conversationId ||
                (item.kind != "chat-user" && item.kind != "chat-assistant")) continue;
            const auto payload = nlohmann::json::parse(item.payload, nullptr, false);
            if (!payload.is_object()) continue;
            messages.push_back({{"role", item.kind == "chat-user" ? "user" : "assistant"},
                {"text", payload.value("text", "")}, {"createdAt", item.createdAt}});
        }
        return crow::response(200, nlohmann::json{{"messages", messages}}.dump());
    });

    CROW_ROUTE(app, "/api/conversations/<string>").methods(crow::HTTPMethod::DELETE)([&db](const crow::request&, std::string conversationId) {
        db.deleteConversation(conversationId);
        return crow::response(200, nlohmann::json{{"ok", true}}.dump());
    });

    CROW_ROUTE(app, "/api/quiz-attempts").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const int phaseIndex = body.value("phaseIndex", 0);
            const int topicIndex = body.value("topicIndex", 0);
            if (!requesterCanAccessCourse(db, req, courseId, requestAnonymousId(req, &body)) || courseId.empty() ||
                phaseIndex < 1 || topicIndex < 1 || !body.contains("answers") || !body["answers"].is_array())
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "测验参数无效"}}.dump());
            const auto session = db.findLearningSession(courseId, phaseIndex, topicIndex);
            if (!session) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "测验尚未生成"}}.dump());
            const auto content = nlohmann::json::parse(session->content);
            const auto quiz = content.at("blocks").at("quiz").at("quiz");
            const bool matchingQuestions = body.contains("questions") && gangyi::questionSnapshotsMatch(body.at("questions"), quiz);
            if ((body.contains("questions") && !matchingQuestions) ||
                (!matchingQuestions && body.contains("contentVersion") && body.at("contentVersion") != content.value("contentVersion", 1)))
                return crow::response(409, nlohmann::json{{"error", "测验内容已更新，请刷新，旧答案不会按新题判分。"}}.dump());
            if (!quiz.is_array() || quiz.empty() || body["answers"].size() != quiz.size())
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "测验答案数量不正确"}}.dump());
            int score = 0, unknownCount = 0;
            nlohmann::json results = nlohmann::json::array();
            for (size_t i = 0; i < quiz.size(); ++i) {
                const auto& choice = body["answers"][i];
                const bool unknown = choice.is_string() && choice.get<std::string>() == "unknown";
                if (!choice.is_null() && !unknown && (!choice.is_number_integer() || choice.get<int>() < 0 ||
                    choice.get<int>() >= static_cast<int>(quiz[i].at("options").size())))
                    return crow::response(400, nlohmann::json{{"ok", false}, {"error", "测验答案无效"}}.dump());
                const bool correct = !choice.is_null() && !unknown &&
                    choice.get<int>() == quiz[i].at("answerIndex").get<int>();
                if (correct) ++score;
                if (unknown) ++unknownCount;
                const auto snapshot = gangyi::questionSnapshot(quiz[i]);
                results.push_back({{"questionIndex", i}, {"topic", session->topicTitle},
                    {"correct", correct}, {"answered", !choice.is_null()}, {"unknown", unknown},
                    {"credible", !choice.is_null()}, {"givenAnswer", choice}, {"questionSnapshot", snapshot},
                    {"questionId", gangyi::questionIdentity(courseId, snapshot)}});
            }
            const int total = static_cast<int>(quiz.size());
            recordInteraction(db, "quiz", {{"score", score}, {"total", total}, {"answers", body["answers"]},
                {"results", results}, {"phaseIndex", phaseIndex}, {"topicIndex", topicIndex},
                {"topic", session->topicTitle}}, courseId);
            return crow::response(200, nlohmann::json{{"ok", true}, {"score", score}, {"total", total},
                {"unknownCount", unknownCount}, {"passed", score * 10 >= total * 7}}.dump());
        } catch (...) { return crow::response(400, nlohmann::json{{"ok", false}, {"error", "测验保存失败"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/learning-interactions").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string kind = body.value("kind", "");
            const int phaseIndex = body.value("phaseIndex", 0);
            const int topicIndex = body.value("topicIndex", 0);
            if (courseId.empty() || !requesterCanAccessCourse(db, req, courseId, requestAnonymousId(req, &body)) ||
                (kind != "practice" && kind != "review") || phaseIndex < 1 || topicIndex < 1)
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "学习记录参数无效"}}.dump());
            const auto session = db.findLearningSession(courseId, phaseIndex, topicIndex);
            if (!session) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "微课尚未生成"}}.dump());
            const std::string state = body.value("state", std::string());
            if (state != "completed" && state != "reopened" && state != "too_hard" &&
                state != "too_easy" && state != "unsuitable")
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "反馈状态无效"}}.dump());
            recordInteraction(db, kind, {{"topic", session->topicTitle}, {"phaseIndex", phaseIndex},
                {"topicIndex", topicIndex}, {"state", state}}, courseId);
            return crow::response(200, nlohmann::json{{"ok", true}}.dump());
        } catch (...) { return crow::response(400, nlohmann::json{{"ok", false}, {"error", "学习记录保存失败"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/analyze-image-goal").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        try {
            if (req.get_header_value("Content-Type").find("multipart/form-data") == std::string::npos) {
                return crow::response(400, nlohmann::json{{"success", false}, {"message", "请上传图片文件"}}.dump());
            }
            crow::multipart::message form(req);
            const auto image = form.get_part_by_name("image");
            const std::string prompt = form.get_part_by_name("prompt").body;
            const std::string mode = form.get_part_by_name("mode").body == "lite" ? "lite" : "deep";
            const std::string mimeType = image.get_header_object("Content-Type").value;
            if (image.body.empty() || mimeType.rfind("image/", 0) != 0) {
                return crow::response(400, nlohmann::json{{"success", false}, {"goal", prompt}, {"mode", mode}, {"message", "请上传图片文件"}}.dump());
            }
            if (image.body.size() > 5 * 1024 * 1024) {
                return crow::response(413, nlohmann::json{{"success", false}, {"goal", prompt}, {"mode", mode}, {"message", "图片过大，请上传 5MB 以内的图片"}}.dump());
            }
            gangyi::AIClient ai;
            gangyi::ImageGoalAnalyzer analyzer(ai);
            const auto result = analyzer.analyze(prompt, mode, mimeType, image.body);
            return crow::response(200, nlohmann::json{{"success", true}, {"goal", result.goal}, {"summary", result.summary},
                {"keywords", result.keywords}, {"suggestedSearchQuery", result.suggestedSearchQuery}, {"mode", mode}}.dump());
        } catch (const gangyi::AIClientError& error) {
            return crow::response(200, nlohmann::json{{"success", false}, {"message", "图片识别未完成，请补充文字描述后重试"}, {"type", error.errorType}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"success", false}, {"message", "图片格式无法识别，请补充文字描述后重试"}}.dump());
        }
    });

    CROW_ROUTE(app, "/health")([] {
        crow::response response(nlohmann::json{{"status", "healthy"}}.dump());
        response.set_header("Content-Type", "application/json; charset=utf-8");
        return response;
    });

    // POST /api/generate-plan —— 每门课程首次均由 AI 生成；课程保存后由快照负责复用。
    CROW_ROUTE(app, "/api/generate-plan").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string goal = body.value("goal", "");
            const std::string mode = body.value("mode", "deep");
            if (goal.empty()) {
                return crow::response(400, nlohmann::json{{"error", "请填写学习目标。"}}.dump());
            }
            gangyi::AIClient ai;
            gangyi::PlanGenerator generator(ai);

            nlohmann::json adapted;
            gangyi::QualityResult gate;
            std::string feedback;
            bool qualityValid = false;
            int successfulAttempt = 0;
            std::string generatedModel;
            std::string searchStatus = "unavailable";
            std::vector<std::string> searchSources;
            std::string lastErrorType = "quality_rejected";
            for (int attempt = 0; attempt < 3 && !qualityValid; ++attempt) {
                    std::cerr << "[generate-plan] attempt=" << (attempt + 1)
                              << " mode=" << mode << std::endl;
                    try {
                        const auto plan = generator.generate(goal, mode, feedback, gangyi::learningContext(db));
                        generatedModel = plan.generationModel;
                        searchStatus = plan.searchStatus;
                        searchSources = plan.searchSources;
                        const auto raw = planToJson(plan);
                        adapted = gangyi::adaptGeneratedPlan(raw, mode);
                        gate = gangyi::validateCourseContent(adapted, goal, mode,
                            adapted.value("title", goal));
                        qualityValid = gate.valid;
                        if (qualityValid) successfulAttempt = attempt + 1;
                        if (!qualityValid) {
                            lastErrorType = "quality_rejected";
                            feedback = qualityFeedback(gate);
                            std::cerr << "[generate-plan] quality rejected attempt="
                                      << (attempt + 1) << " score=" << gate.score
                                      << " issues=" << qualityIssueCodes(gate) << std::endl;
                        }
                    } catch (const gangyi::AIClientError& error) {
                        lastErrorType = error.errorType;
                        std::cerr << "[generate-plan] AI error attempt=" << (attempt + 1)
                                  << " type=" << error.errorType << " reason=" << error.what() << std::endl;
                        feedback = "上一次模型输出无法使用：" + std::string(error.what()) +
                            "。请重新输出完整、严格符合字段结构的 JSON。";
                    } catch (const std::exception& error) {
                        lastErrorType = "invalid_response";
                        std::cerr << "[generate-plan] error attempt=" << (attempt + 1)
                                  << " message=" << error.what() << std::endl;

                        feedback = "上一次生成结果解析失败：" + std::string(error.what()) +
                            "。请重新输出完整 JSON，不要输出解释文字。";
                    }
            }
            if (!qualityValid) throw gangyi::AIClientError(lastErrorType, feedback.empty() ? "AI 连续三次未生成合格课程结构" : feedback);
            const char* configuredModel = std::getenv("AI_MODEL");
            adapted["generation"] = {{"source", "ai"}, {"model", generatedModel.empty() ? (configuredModel ? configuredModel : "") : generatedModel},
                {"generatedAt", nowIso8601()}, {"attempts", successfulAttempt}, {"promptVersion", "ai-plan-v1"},
                {"searchStatus", searchStatus}, {"sources", searchSources}, {"profileVersion", db.profileAssessedRevision()}};

            // 联网搜索补充真实资源（失败静默降级，不阻断主流程）
            try {
                gangyi::SearchClient search;
                const auto resources = search.search(goal, 8);
                if (!resources.empty()) {
                    nlohmann::json arr = nlohmann::json::array();
                    for (const auto& r : resources) {
                        arr.push_back({{"name", r.title}, {"type", r.type}, {"difficulty", r.difficulty},
                                       {"free", r.free}, {"description", r.description}, {"href", r.url}});
                    }
                    adapted["resources"] = arr;
                    adapted["resourceSourceMessage"] = "已为你补充全网真实学习资源";
                }
            } catch (...) {}

            return crow::response(200, adapted.dump());
        } catch (const gangyi::AIClientError& e) {
            std::cerr << "[generate-plan] unhandled AI error type=" << e.errorType << std::endl;
            // 错误→HTTP 映射（对齐 docs/ai-spec.md）：missing_config/auth_error→503，timeout→504，其余→502
            int code = 502;
            if (e.errorType == "missing_config" || e.errorType == "auth_error") code = 503;
            else if (e.errorType == "timeout") code = 504;
            return crow::response(code, nlohmann::json{{"ok", false}, {"error", publicAIErrorMessage(e)},
                {"type", e.errorType}, {"canRetry", true}, {"attempts", 3}}.dump());
        } catch (const std::exception& e) {
            std::cerr << "[generate-plan] unhandled error" << std::endl;
            return crow::response(502, nlohmann::json{{"error", "课程规划生成失败，请稍后重试。"}}.dump());
        }
    });

    // POST /api/courses —— 保存课程 + 快照（质量门禁 + 脱敏 + upsert）
    CROW_ROUTE(app, "/api/courses").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string goal = body.value("goal", "");
            const std::string title = body.value("title", "");
            if (goal.empty() || title.empty() || !body.contains("payload")) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "COURSE_SAVE_INVALID_INPUT"},
                    {"message", "课程信息不完整。"}, {"canRetry", true}}.dump());
            }
            const std::string mode = body.value("mode", "deep");
            const std::string summary = body.value("summary", "");
            const std::string source = body.value("source", "ai");
            const std::string anonymousId = requestAnonymousId(req, &body);
            const std::string userId;

            const auto generation = body["payload"].value("generation", nlohmann::json::object());
            if (source != "ai" || generation.value("source", "") != "ai" ||
                generation.value("promptVersion", "") != "ai-plan-v1") {
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "COURSE_AI_PROVENANCE_REQUIRED"},
                    {"message", "课程必须由当前 AI 生成链路创建，请重新生成。"}, {"canRetry", true}}.dump());
            }

            const auto gate = gangyi::validateCourseContent(body["payload"], goal, mode, title);
            if (!gate.valid) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "COURSE_SNAPSHOT_INVALID"},
                    {"message", "课程内容暂未生成完成，请重新生成后再保存。"}, {"canRetry", true},
                    {"reasons", gate.reasons}, {"score", gate.score}}.dump());
            }
            const auto sanitized = gangyi::sanitizeCoursePayload(body["payload"]);
            const auto courseId = gangyi::saveCourseSnapshot(db, anonymousId, userId, goal, mode, title,
                summary.empty() ? "" : summary, source, sanitized);
            if (!courseId) {
                return crow::response(500, nlohmann::json{{"ok", false}, {"error", "COURSE_SAVE_FAILED"},
                    {"message", "课程保存失败，但你可以重新生成或稍后重试。"}, {"canRetry", true}}.dump());
            }
            db.markProfileDirty();
            std::string href = "/plan?courseId=" + *courseId;
            if (userId.empty() && !anonymousId.empty()) href += "&anonymousId=" + anonymousId;
            return crow::response(200, nlohmann::json{{"ok", true}, {"courseId", *courseId}, {"href", href}}.dump());
        } catch (const std::exception&) {
            return crow::response(500, nlohmann::json{{"ok", false}, {"error", "COURSE_SAVE_FAILED"},
                {"message", "课程保存失败。"}, {"canRetry", true}}.dump());
        }
    });

    // GET /api/courses —— 按身份列出课程（历史课堂 / 我的课程用）
    CROW_ROUTE(app, "/api/courses")([&db](const crow::request& req) {
        const char* anonymousId = req.url_params.get("anonymousId");
        const char* limitParam = req.url_params.get("limit");
        const char* offsetParam = req.url_params.get("offset");
        size_t limit = 50, offset = 0;
        try { if (limitParam) limit = static_cast<size_t>(std::stoi(limitParam)); } catch (...) {}
        try { if (offsetParam) offset = static_cast<size_t>(std::stoi(offsetParam)); } catch (...) {}
        if (limit < 1) limit = 1;
        if (limit > 100) limit = 100;
        try {
            const std::string safeAnonymousId = anonymousId ? anonymousId : "";
            const auto courses = gangyi::listCoursesForIdentity(db, "", "", limit, offset);
            nlohmann::json arr = nlohmann::json::array();
            for (const auto& c : courses) {
                std::string href = "/plan?courseId=" + c.id;
                if (anonymousId) href += "&anonymousId=" + std::string(anonymousId);
                arr.push_back({{"id", c.id}, {"goal", c.goal}, {"mode", c.mode}, {"title", c.title},
                    {"summary", c.summary ? nlohmann::json(*c.summary) : nlohmann::json(nullptr)},
                    {"createdAt", c.createdAt}, {"updatedAt", c.updatedAt}, {"href", href}});
            }
            return crow::response(200, nlohmann::json{{"courses", arr}}.dump());
        } catch (...) {
            return crow::response(200, nlohmann::json{{"courses", nlohmann::json::array()}}.dump());
        }
    });

    // GET /api/courses/<courseId> —— 恢复课程 + 最新快照
    CROW_ROUTE(app, "/api/courses/<string>")([&db](const crow::request& req, std::string courseId) {
        try {
            const auto found = gangyi::getCourseWithSnapshot(db, courseId);
            if (!found || !requesterCanReadCourse(db, found->course, req)) {
                return crow::response(404, nlohmann::json{{"error", "课程不存在或无权访问。"}}.dump());
            }
            const nlohmann::json course = {{"id", found->course.id}, {"goal", found->course.goal},
                {"mode", found->course.mode}, {"title", found->course.title},
                {"summary", found->course.summary ? nlohmann::json(*found->course.summary) : nlohmann::json(nullptr)},
                {"source", found->course.source},
                {"createdAt", found->course.createdAt}, {"updatedAt", found->course.updatedAt}};
            return crow::response(200, nlohmann::json{{"course", course},
                {"snapshot", {{"payload", found->payload}}}}.dump());
        } catch (...) {
            return crow::response(500, nlohmann::json{{"error", "课程读取失败，请稍后重试。"}}.dump());
        }
    });

    CROW_ROUTE(app, "/api/my-courses/<string>").methods(crow::HTTPMethod::DELETE)([&db](const crow::request& req, std::string courseId) {
        const std::string anonymousId = requestAnonymousId(req);
        if (!gangyi::deleteCourseForIdentity(db, courseId, "", "")) {
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
        }
        return crow::response(200, nlohmann::json{{"ok", true}}.dump());
    });

    // ---- 阶段 B：学习体验 API ----
#if 0  // 学习微课程页面已删除，这些接口不再对外提供。

    // POST /api/learning-card-progress —— 学习卡状态（写后自动重算）
    CROW_ROUTE(app, "/api/learning-card-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            auto result = gangyi::saveLearningCardProgress(db, body);
            if (!result.ok) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
            }
            if (body.contains("courseId") && body["courseId"].is_string() && !body["courseId"].get<std::string>().empty()) {
                gangyi::recomputeCourseProgress(db, body["courseId"].get<std::string>(),
                    body.value("anonymousId", ""), body.value("goal", ""));
            }
            return crow::response(200, nlohmann::json{{"ok", true}, {"item", result.item}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        }
    });

    // GET /api/learning-step-progress —— 恢复阶段展开步骤的理解状态。
    CROW_ROUTE(app, "/api/learning-step-progress")([&db](const crow::request& req) {
        const char* courseId = req.url_params.get("courseId");
        const std::string anonymousId = requestAnonymousId(req);
        const char* goal = req.url_params.get("goal");
        const char* mode = req.url_params.get("mode");
        const char* phaseIndex = req.url_params.get("phaseIndex");
        if (!requesterCanAccessCourse(db, req, courseId ? courseId : "", anonymousId)) {
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
        }
        int phase = 0;
        try { if (phaseIndex) phase = std::stoi(phaseIndex); } catch (...) {}
        nlohmann::json items = nlohmann::json::array();
        for (const auto& item : db.listLearningStepProgress()) {
            const bool sameCourse = courseId && *courseId && item.courseId.value_or("") == courseId;
            const bool sameAnonymous = (!courseId || !*courseId) && !anonymousId.empty() &&
                item.anonymousId.value_or("") == anonymousId && item.goal == (goal ? goal : "") &&
                item.mode.value_or("deep") == (mode && *mode ? mode : "deep");
            if ((sameCourse || sameAnonymous) && item.phaseIndex == phase) {
                items.push_back({{"stepIndex", item.stepIndex}, {"stepTitle", item.stepTitle}, {"status", item.status}});
            }
        }
        return crow::response(200, nlohmann::json{{"ok", true}, {"items", items}}.dump());
    });

    // POST /api/learning-step-progress —— 步骤理解状态（写后自动重算）
    CROW_ROUTE(app, "/api/learning-step-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            auto result = gangyi::saveLearningStepProgress(db, body);
            if (!result.ok) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
            }
            if (body.contains("courseId") && body["courseId"].is_string() && !body["courseId"].get<std::string>().empty()) {
                gangyi::recomputeCourseProgress(db, body["courseId"].get<std::string>(),
                    body.value("anonymousId", ""), body.value("goal", ""));
            }
            return crow::response(200, nlohmann::json{{"ok", true}, {"item", result.item}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        }
    });

#endif

    // 阶段页仍使用任务进度，课程页仍使用总体进度与重置接口。
    // GET /api/task-progress —— 恢复阶段展开任务的三态进度。
    CROW_ROUTE(app, "/api/task-progress")([&db](const crow::request& req) {
        const char* courseId = req.url_params.get("courseId");
        const std::string anonymousId = requestAnonymousId(req);
        const char* goal = req.url_params.get("goal");
        const char* mode = req.url_params.get("mode");
        const char* phaseIndex = req.url_params.get("phaseIndex");
        if (!requesterCanAccessCourse(db, req, courseId ? courseId : "", anonymousId)) {
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
        }
        int phase = 0;
        try { if (phaseIndex) phase = std::stoi(phaseIndex); } catch (...) {}
        nlohmann::json items = nlohmann::json::array();
        for (const auto& item : db.listTaskProgress()) {
            const bool sameCourse = courseId && *courseId && item.courseId.value_or("") == courseId;
            const bool sameAnonymous = (!courseId || !*courseId) && !anonymousId.empty() &&
                item.anonymousId.value_or("") == anonymousId && item.goal == (goal ? goal : "") &&
                item.mode.value_or("deep") == (mode && *mode ? mode : "deep");
            if ((sameCourse || sameAnonymous) && item.phaseIndex == phase) {
                items.push_back({{"taskIndex", item.taskIndex}, {"taskTitle", item.taskTitle}, {"status", item.status}});
            }
        }
        return crow::response(200, nlohmann::json{{"ok", true}, {"items", items}}.dump());
    });

    // POST /api/task-progress —— 阶段任务状态（写后自动重算）
    CROW_ROUTE(app, "/api/task-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            auto result = gangyi::saveTaskProgress(db, body);
            if (!result.ok) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
            }
            if (body.contains("courseId") && body["courseId"].is_string() && !body["courseId"].get<std::string>().empty()) {
                gangyi::recomputeCourseProgress(db, body["courseId"].get<std::string>(),
                    body.value("anonymousId", ""), body.value("goal", ""));
            }
            return crow::response(200, nlohmann::json{{"ok", true}, {"item", result.item}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        }
    });

#if 0  // 学习微课程页面已删除。
    // POST /api/learn/progress —— 三合一：学习卡 + 断点记录 + 全量重算
    CROW_ROUTE(app, "/api/learn/progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            const std::string goal = body.value("goal", "");
            const std::string mode = body.value("mode", "deep");
            // 1) upsert 学习卡（topicIndex 0→1；phaseIndex ≥1）
            nlohmann::json cardBody = body;
            if (cardBody.contains("phaseIndex") && cardBody["phaseIndex"].is_number_integer() && cardBody["phaseIndex"].get<int>() <= 0) {
                cardBody["phaseIndex"] = 1;
            }
            if (cardBody.contains("topicIndex") && cardBody["topicIndex"].is_number_integer() && cardBody["topicIndex"].get<int>() <= 0) {
                cardBody["topicIndex"] = 1;
            }
            cardBody["status"] = body.value("status", "completed");
            gangyi::saveLearningCardProgress(db, cardBody);
            // 2) 断点记录
            gangyi::updateLastVisited(db, courseId, anonymousId, goal, mode, body);
            // 3) 全量重算
            auto progress = gangyi::recomputeCourseProgress(db, courseId, anonymousId, goal);
            if (!progress) progress = nlohmann::json{{"overallPercent", 0}, {"completedCount", 0}, {"totalCount", 0}};
            return crow::response(200, nlohmann::json{{"ok", true}, {"progress", *progress}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        }
    });

#endif

    // POST /api/course-progress —— 全量重算 / 断点记录
    CROW_ROUTE(app, "/api/course-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string action = body.value("action", "recompute");
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            const std::string goal = body.value("goal", "");
            const std::string mode = body.value("mode", "deep");
            std::optional<nlohmann::json> progress;
            if (action == "reset") {
                progress = gangyi::resetCourseProgress(db, courseId, anonymousId, goal);
            } else {
                if (action == "lastVisited") {
                    gangyi::updateLastVisited(db, courseId, anonymousId, goal, mode, body);
                }
                progress = gangyi::recomputeCourseProgress(db, courseId, anonymousId, goal);
            }
            if (!progress) progress = nlohmann::json{{"overallPercent", 0}, {"completedCount", 0}, {"totalCount", 0}};
            if (action == "reset") db.markProfileDirty();
            return crow::response(200, nlohmann::json{{"ok", true}, {"progress", *progress}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        }
    });

    CROW_ROUTE(app, "/api/course-progress")([&db](const crow::request& req) {
        try {
            const char* courseId = req.url_params.get("courseId");
            const char* anonymousId = req.url_params.get("anonymousId");
            if (!courseId || !*courseId) return crow::response(400, nlohmann::json{{"ok", false}, {"error", "缺少课程编号。"}}.dump());
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId ? anonymousId : "")) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            const auto progress = gangyi::recomputeCourseProgress(db, courseId, anonymousId ? anonymousId : "", "");
            return crow::response(200, nlohmann::json{{"ok", true}, {"progress", progress ? *progress : nlohmann::json::object()}}.dump());
        } catch (...) {
            return crow::response(500, nlohmann::json{{"ok", false}, {"error", "课程进度读取失败，请稍后重试。"}}.dump());
        }
    });

#if 0  // 学习微课程页面已删除。
    // GET /api/learning-sessions —— 微课会话查询（恢复优先）
    CROW_ROUTE(app, "/api/learning-sessions")([&db](const crow::request& req) {
        const char* courseId = req.url_params.get("courseId");
        const char* anonymousId = req.url_params.get("anonymousId");
        const char* goal = req.url_params.get("goal");
        const char* mode = req.url_params.get("mode");
        const char* phaseIndexP = req.url_params.get("phaseIndex");
        const char* topicIndexP = req.url_params.get("topicIndex");
        int phaseIndex = 0, topicIndex = 0;
        try { if (phaseIndexP) phaseIndex = std::stoi(phaseIndexP); } catch (...) {}
        try { if (topicIndexP) topicIndex = std::stoi(topicIndexP); } catch (...) {}
        try {
            if (!requesterCanAccessCourse(db, req, courseId ? courseId : "", anonymousId ? anonymousId : "")) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            const auto session = gangyi::findLearningSession(db, courseId ? courseId : "",
                anonymousId ? anonymousId : "", goal ? goal : "", mode ? mode : "deep", phaseIndex, topicIndex);
            if (!session) {
                return crow::response(200, nlohmann::json{{"ok", true}, {"session", nullptr}}.dump());
            }
            return crow::response(200, nlohmann::json{{"ok", true}, {"session", *session}}.dump());
        } catch (...) {
            return crow::response(500, nlohmann::json{{"ok", false}, {"error", "学习记录读取失败，请稍后重试。"}}.dump());
        }
    });

    // POST /api/learning-sessions —— 微课会话保存
    CROW_ROUTE(app, "/api/learning-sessions").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            }
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            const bool ok = gangyi::upsertLearningSession(db, body);
            return crow::response(ok ? 200 : 400, nlohmann::json{{"ok", ok}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "请求内容格式不正确。"}}.dump());
        }
    });

#endif

    // POST /api/phase-expansion —— 首次由 AI 生成并保存，后续复用课程快照。

    CROW_ROUTE(app, "/api/phase-expansion").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "课程不存在或无权访问。"}}.dump());
            }
            std::string goal = body.value("goal", "");
            std::string mode = body.value("mode", "deep");
            const int phaseIndex = std::max(1, body.value("phaseIndex", 1));
            std::string stage = courseId.empty() ? body.value("stage", "") : "";
            nlohmann::json plan = nlohmann::json::object();
            if (!courseId.empty()) {
                if (const auto found = gangyi::getCourseWithSnapshot(db, courseId)) {
                    plan = found->payload;
                    goal = found->course.goal;
                    mode = found->course.mode;
                }
                const auto provenance = plan.value("generation", nlohmann::json::object());
                if (provenance.value("source", "") != "ai" || provenance.value("promptVersion", "") != "ai-plan-v1") {
                    return crow::response(409, nlohmann::json{{"ok", false}, {"type", "course_regeneration_required"},
                        {"error", "该课程来自旧生成链路，请重新生成课程。"}, {"canRetry", false}}.dump());
                }
            }
            if (goal.empty()) return crow::response(400, nlohmann::json{{"ok", false}, {"error", "goal 不能为空"}}.dump());

            const std::string phaseKey = std::to_string(phaseIndex);
            const bool regenerate = body.value("regenerate", false);
            const auto expansions = plan.value("phaseExpansions", nlohmann::json::object());
            if (!regenerate && expansions.is_object() && expansions.contains(phaseKey)) {
                const auto cached = expansions[phaseKey];
                if (cached.value("generation", nlohmann::json::object()).value("source", "") == "ai") {
                    return crow::response(200, nlohmann::json{{"ok", true}, {"cached", true},
                        {"phase", cached.value("content", nlohmann::json::object())},
                        {"generation", cached.value("generation", nlohmann::json::object())},
                        {"resources", cached.value("resources", nlohmann::json::array())}}.dump());
                }
            }

            std::vector<std::string> topics;
            if (courseId.empty() && body.contains("topics") && body["topics"].is_array())
                for (const auto& topic : body["topics"]) if (topic.is_string()) topics.push_back(topic.get<std::string>());
            const auto roadmap = plan.value("roadmap", nlohmann::json::array());
            if (roadmap.is_array() && phaseIndex <= static_cast<int>(roadmap.size()) && roadmap[phaseIndex - 1].is_object()) {
                const auto& phase = roadmap[phaseIndex - 1];
                if (stage.empty()) stage = phase.value("name", "");
                if (topics.empty() && phase.contains("topics") && phase["topics"].is_array())
                    for (const auto& topic : phase["topics"]) if (topic.is_string()) topics.push_back(topic.get<std::string>());
            }
            const auto structure = plan.value("courseStructure", nlohmann::json::array());
            if (structure.is_array() && phaseIndex <= static_cast<int>(structure.size()) && structure[phaseIndex - 1].is_object()) {
                const auto& phase = structure[phaseIndex - 1];
                if (stage.empty()) stage = phase.value("stage", "");
                if (topics.empty() && phase.contains("topics") && phase["topics"].is_array())
                    for (const auto& topic : phase["topics"]) if (topic.is_string()) topics.push_back(topic.get<std::string>());
            }
            if (stage.empty() || topics.empty()) return crow::response(400, nlohmann::json{{"ok", false},
                {"type", "phase_context_missing"}, {"error", "课程缺少 AI 生成的阶段或主题"}}.dump());

            std::vector<gangyi::SearchResource> resources;
            try { gangyi::SearchClient search; resources = search.search(goal, 8); } catch (...) {}
            gangyi::AIClient ai;
            gangyi::PhaseGenerator generator(ai);
            auto generated = *generator.generate(goal, mode, phaseIndex, stage, topics, resources, gangyi::learningContext(db));
            auto metadata = generated.value("_generation", nlohmann::json::object());
            metadata["profileVersion"] = db.profileAssessedRevision();
            if (metadata.value("source", "") != "ai") throw gangyi::AIClientError("invalid_response", "阶段生成来源校验失败");
            generated.erase("_generation");

            nlohmann::json resourceItems = nlohmann::json::array();
            for (const auto& resource : resources) resourceItems.push_back({
                {"title", resource.title}, {"description", resource.description}, {"url", resource.url},
                {"source", resource.source}, {"type", resource.type}, {"difficulty", resource.difficulty}, {"free", resource.free}});

            if (!courseId.empty()) {
                auto snapshots = db.findSnapshotsByCourseId(courseId);
                if (!snapshots.empty()) {
                    auto latest = std::max_element(snapshots.begin(), snapshots.end(),
                        [](const auto& left, const auto& right) { return left.version < right.version; });
                    auto payload = nlohmann::json::parse(latest->payload);
                    if (!payload.contains("phaseExpansions") || !payload["phaseExpansions"].is_object()) payload["phaseExpansions"] = nlohmann::json::object();
                    payload["phaseExpansions"][phaseKey] = {{"content", generated}, {"generation", metadata}, {"resources", resourceItems}};
                    latest->payload = payload.dump();
                    if (!db.update(*latest)) throw std::runtime_error("阶段内容保存失败");
                }
            }
            std::cerr << "[phase] saved course=" << courseId << " phase=" << phaseIndex << '\n';
            return crow::response(200, nlohmann::json{{"ok", true}, {"cached", false},
                {"phase", generated}, {"generation", metadata}, {"resources", resourceItems}}.dump());
        } catch (const gangyi::AIClientError& error) {
            return crow::response(aiHttpStatus(error), nlohmann::json{{"ok", false}, {"error", publicAIErrorMessage(error)},
                {"type", error.errorType}, {"canRetry", true}, {"attempts", 3}}.dump());
        } catch (const std::exception& error) {
            std::cerr << "[phase] internal_error=" << error.what() << '\n';
            return crow::response(502, nlohmann::json{{"ok", false},
                {"error", "阶段内容保存失败，请稍后重试。"}, {"canRetry", true}}.dump());

        }
    });
    CROW_ROUTE(app, "/<path>")([](const crow::request&, crow::response& response, std::string path) {
        const auto public_root = std::filesystem::weakly_canonical("public");
        const auto requested = std::filesystem::weakly_canonical(public_root / path);
        const auto root_text = public_root.generic_string();
        const auto requested_text = requested.generic_string();
        const bool is_under_public = requested_text == root_text ||
            (requested_text.rfind(root_text, 0) == 0 &&
             requested_text[root_text.size()] == '/');
        if (!is_under_public || !std::filesystem::is_regular_file(requested)) {
            response.code = 404;
            response.end("Not found");
            return;
        }

        std::ifstream file(requested, std::ios::binary);
        std::ostringstream body;
        body << file.rdbuf();
        response.set_header("Content-Type", content_type_for(requested));
        response.code = 200;
        response.end(body.str());
    });

    if (!config.local_control_token.empty() && config.host == "127.0.0.1") {
        app.route_dynamic("/internal/shutdown")
            .methods(crow::HTTPMethod::POST)
            ([&app, token = config.local_control_token](const crow::request& req) {
                if (req.get_header_value("X-Gangyi-Control-Token") != token) {
                    return crow::response(403, nlohmann::json{{"ok", false}}.dump());
                }
                std::thread([&app] {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    app.stop();
                }).detach();
                return crow::response(200, nlohmann::json{{"ok", true}}.dump());
            });
    }

    const auto launchId = config.launch_session_id.empty() ? nowIso8601() : config.launch_session_id;
    gangyi::agentSubmit(db, localAgentAccess(db), {{"type", "startup"}, {"requestId", "startup-" + launchId},
        {"text", "读取真实学习情况，生成快速规划和深度课程各五条推荐，并自主决定需要准备的下一步。无记录时不推测掌握度。"}});
    learningAgent.start();
    learningFlow.start();
    // 画像由同一真实 AI 主控按学习事件更新，不再启动独立评分线程。
    std::cout << "gangyiAI " << gangyi::kVersion << " listening on " << config.host << ':' << config.port << '\n';
    app.bindaddr(config.host).port(config.port).multithreaded().run();
    learningAgent.stop();
    { std::lock_guard<std::mutex> guard(socketMapMutex); for (const auto& [connection, state] : socketMap) state->cancelled = true; }
    while (socketWorkerCount > 0) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    homeRecommendations.stop();
    learningFlow.stop();
}

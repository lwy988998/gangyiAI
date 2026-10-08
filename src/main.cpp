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
#include "agent_curriculum.hpp"
#include "classroom_service.hpp"
#include "next_lesson.hpp"
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

// 各题型共用同一身份参数，恢复和重试时保留巩固课作用域。
nlohmann::json questionRequestBody(const crow::request& req) {
    const auto value = [&req](const char* name) {
        const char* raw = req.url_params.get(name);
        return std::string(raw ? raw : "");
    };
    nlohmann::json body = {{"courseId", value("courseId")},
        {"phaseIndex", value("phaseIndex").empty() ? 1 : std::stoi(value("phaseIndex"))}, {"topicIndex", value("topicIndex").empty() ? 1 : std::stoi(value("topicIndex"))}};
    for (const auto* name : {"kind", "lessonTaskId", "lessonId", "topicId", "reviewId"})
        if (!value(name).empty()) body[name] = value(name);
    if (!value("index").empty()) body["index"] = std::stoi(value("index"));
    for (const auto* name : {"review", "day"})
        if (!value(name).empty()) body[name] = std::stoi(value(name));
    return body;
}

// 批量兼容提交保存一个任务，避免每题提交互相使旧任务过期。
nlohmann::json submitClassroomBatch(gangyi::Database& db, const gangyi::AgentAccess& access, nlohmann::json body) {
    using Json = nlohmann::json;
    if (!body.value("answers", Json()).is_array()) throw std::invalid_argument("答案列表无效");
    if (!body.contains("kind")) body["kind"] = "quiz";
    const auto view = gangyi::agentQuestionView(db, access, body);
    const auto questions = view.at("questions");
    if (questions.size() != body.at("answers").size()) throw std::invalid_argument("题目与作答数量不一致");
    static std::atomic_uint64_t sequence{0};
    const auto request = body.value("requestId", "batch-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-" + std::to_string(++sequence));
    Json inputs = Json::array();
    for (size_t i = 0; i < questions.size(); ++i) {
        auto input = body; input.erase("answers"); input.erase("questions"); input["lessonId"] = view.at("lessonId");
        input["sectionId"] = questions[i].at("sectionId"); input["sectionVersion"] = questions[i].at("sectionVersion");
        input["questionId"] = questions[i].at("questionId"); input["requestId"] = request + ":" + std::to_string(i);
        input["answer"] = body.at("answers").at(i);
        if (input.at("answer").is_null()) { input["action"] = "omitted"; input["question"] = "这道题尚未作答"; }
        else if (input.at("answer") == "unknown") { input["action"] = "unknown"; input["question"] = "我暂时不会这道题"; }
        inputs.push_back(gangyi::agentLegacyEvent(db, access, input));
    }
    const bool unanswered = std::all_of(inputs.begin(), inputs.end(), [](const auto& input) { return input.value("action", "answer") == "omitted" || input.value("action", "answer") == "skip"; });
    return gangyi::agentSubmit(db, access, {{"type", "question_answer"}, {"action", unanswered ? "omitted" : "answer"}, {"requestId", request}, {"courseId", body.at("courseId")},
        {"lessonId", view.at("lessonId")}, {"batchAnswers", inputs}, {"text", "请逐项区分真实作答、不会和漏答，先评价全部实际作答，再讲解，不采用固定通过率。"}});
}

}  // namespace

namespace {

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
            else if (item.is_object()) callback(item.value("legacyPhaseIndex", phaseIndex), item.value("legacyTopicIndex", topicIndex), item.value("title", ""));
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

struct ClassroomSocket {
    std::mutex mutex;
    crow::request request;
    crow::websocket::connection* connection = nullptr;
    std::atomic<bool> cancelled{false};
    std::atomic<bool> busy{false};
    std::atomic<bool> stopRequested{false};
    std::string taskId;
    void send(const nlohmann::json& event) {
        std::lock_guard<std::mutex> guard(mutex);
        if (connection) connection->send_text(event.dump());
    }
};

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

nlohmann::json localAgentView(gangyi::Database& db, const std::string& taskId = {}) {
    nlohmann::json result;
    db.readSnapshot([&] { result = gangyi::agentView(db, localAgentAccess(db), taskId); });
    return result;
}

nlohmann::json studyPlanAgentAction(gangyi::Database& db, nlohmann::json body, const std::string& action) {
    static std::mutex manualPlanMutex;
    std::lock_guard<std::mutex> guard(manualPlanMutex);
    static std::atomic_uint64_t sequence{0};
    const auto access = localAgentAccess(db);
    nlohmann::json event;
    if (action == "replan") {
        body["requestId"] = body.value("requestId", "schedule-" + nowIso8601() + "-" + std::to_string(++sequence));
        event = {{"type", "schedule_replan"}, {"requestId", body.at("requestId")}, {"scheduleRequest", body},
            {"text", "我手动请求重新排课。请根据最新时间设置、全部课程和真实表现生成候选安排，保留当前课表与未保存编辑，等待我确认。"}};
        // 重复提交复用同一任务，不再次改变学习版本或生成候选。
        const auto index = nlohmann::json::parse(db.profileMeta("agent-index"), nullptr, false);
        if (index.is_array()) for (const auto& id : index) {
            if (!id.is_string()) continue;
            const auto row = db.getClassroomActivity(id); if (!row) continue;
            const auto task = nlohmann::json::parse(row->payload, nullptr, false);
            if (!task.is_object() || task.value("scopeId", "") != access.scopeId || task.value("requestId", "") != body.at("requestId")) continue;
            if (!task.value("manualScheduleReplan", false) || task.at("event") != event) throw std::invalid_argument("重复排课请求的内容不一致");
            return {{"ok", true}, {"plan", gangyi::studyPlanView(db)}, {"task", gangyi::agentView(db, access, id)}};
        }
    }
    const auto result = gangyi::studyPlanProposal(db, body, action);
    if (action == "confirm" || action == "cancel") {
        const auto row = db.getClassroomActivity(body.at("proposalId"));
        if (row && row->kind == "agent-change") {
            auto change = nlohmann::json::parse(row->payload);
            if (change.at("scopeId") != access.scopeId) throw std::invalid_argument("无权操作该课表候选");
            change["status"] = action == "confirm" ? "applied" : "undone";
            if (action == "confirm") change["after"] = db.profileMeta("learning-flow");
            else { change["undoneAt"] = nowIso8601(); change["undoFingerprint"] = gangyi::agentLearnerFingerprint(db, access); }
            gangyi::agentSaveChange(db, change);
        }
    }
    if (action != "replan") return result;
    auto view = result; view["task"] = gangyi::agentSubmit(db, access, event, gangyi::AgentSubmissionOrigin::manualScheduleReplan);
    const auto raw = db.profileMeta("learning-flow"); auto state = nlohmann::json::parse(raw);
    state["replanTaskId"] = view["task"]["id"];
    if (!db.compareProfileMeta("learning-flow", raw, state.dump())) throw std::invalid_argument("课表状态已变化，请刷新读取已提交的任务");
    view["plan"] = gangyi::studyPlanView(db);
    return view;
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
    if (kind == "course-visit" || kind == "lesson-visit") gangyi::rememberCurrentCourse(db, courseId);
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
        std::string mode = modeP && *modeP ? modeP : "";
        std::string phaseIndex = phaseIndexP ? phaseIndexP : "1";
        std::string phaseName = phaseNameP ? phaseNameP : "";
        nlohmann::json plan = nlohmann::json::object();
        nlohmann::json card = nlohmann::json::object();
        if (!courseId.empty() && requesterCanAccessCourse(db, req, courseId, anonymousId)) {
            if (const auto found = gangyi::getCourseWithSnapshot(db, courseId)) {
                if (found->payload.is_object()) plan = found->payload;
                if (goal.empty()) goal = found->course.goal;
                if (mode.empty()) mode = found->course.mode;
                const auto stages = plan.value("courseStructure", nlohmann::json::array());
                int selected = 0;
                try { selected = std::max(0, std::stoi(phaseIndex) - 1); } catch (...) { /* 无效显示序号回到第一阶段。 */ }
                if (selected >= 0 && selected < static_cast<int>(stages.size())) {
                    int displayIndex = 0;
                    for (const auto& item : stages.at(selected).value("topics", nlohmann::json::array())) {
                        ++displayIndex;
                        const int legacyPhase = item.is_object() ? item.value("legacyPhaseIndex", selected + 1) : selected + 1;
                        const int legacyTopic = item.is_object() ? item.value("legacyTopicIndex", displayIndex) : displayIndex;
                        const auto saved = db.findLearningCardProgress(courseId, legacyPhase, legacyTopic);
                        if (saved) card[std::to_string(displayIndex)] = saved->status;
                    }
                }
            }
        }
        if (mode.empty()) mode = "deep";
        crow::response response(gangyi::renderPhasePage(courseId, anonymousId, goal, mode, phaseIndex, phaseName,
            gangyi::publicCoursePayload(plan), card));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    // 三个课堂页面共用已保存课时；旧索引只做身份解析。
    const auto classroomPage = [&db](const crow::request& req, const std::string& view) {
        try {
            const auto scope = questionRequestBody(req);
            const std::string target = req.url_params.get("review") ? "practice" : view;
            if (!scope.value("lessonId", "").empty()) {
                gangyi::agentLessonView(db, localAgentAccess(db), scope.at("lessonId"));
                if (target != view) { crow::response response(302); response.set_header("Location", "/practice?" + req.raw_url.substr(req.raw_url.find('?') + 1)); return response; }
                crow::response response(gangyi::renderClassroomPage(view));
                response.set_header("Content-Type", "text/html; charset=utf-8"); return response;
            }
            const auto saved = gangyi::agentLegacyLesson(db, localAgentAccess(db), scope);
            crow::response response(302);
            if (!saved.empty()) response.set_header("Location", "/" + target + "?lessonId=" + saved.at("lessonId").get<std::string>() +
                (req.raw_url.find('?') == std::string::npos ? std::string() : "&" + req.raw_url.substr(req.raw_url.find('?') + 1)));
            else response.set_header("Location", "/agent-prepare.html?" + (req.raw_url.find('?') == std::string::npos ? std::string() : req.raw_url.substr(req.raw_url.find('?') + 1)) + "&view=" + target);
            return response;
        } catch (const std::exception&) {
            crow::response response(409, "课堂入口已失效，请返回课程选择知识点，或让 AI 重新备课。");
            response.set_header("Content-Type", "text/plain; charset=utf-8"); return response;
        }
    };
    CROW_ROUTE(app, "/learn")([classroomPage](const crow::request& req) { return classroomPage(req, "learn"); });
    CROW_ROUTE(app, "/practice")([classroomPage](const crow::request& req) { return classroomPage(req, "practice"); });
    CROW_ROUTE(app, "/summary")([classroomPage](const crow::request& req) { return classroomPage(req, "summary"); });

    CROW_ROUTE(app, "/learn/next")([&db](const crow::request& req) {
        crow::response response(302);
        if (const char* id = req.url_params.get("id")) {
            if (const auto row = db.getClassroomActivity(std::string("next-preparation:") + id)) {
                const auto saved = nlohmann::json::parse(row->payload, nullptr, false);
                if (saved.is_object() && saved.value("status", "") == "ready" && saved.value("classroomUrl", "").rfind("/learn?", 0) == 0) {
                    response.set_header("Location", saved.at("classroomUrl").get<std::string>()); return response;
                }
                response.set_header("Location", "/agent-prepare.html?courseId=" + queryValue(row->courseId)); return response;
            }
        }
        response.set_header("Location", "/agent-prepare.html" + (req.raw_url.find('?') == std::string::npos ? std::string() : req.raw_url.substr(req.raw_url.find('?'))));
        return response;
    });
    CROW_ROUTE(app, "/api/learn/next").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try {
            gangyi::Database db; db.open(config.database_path); auto event = nlohmann::json::parse(req.body);
            event["type"] = "prepare_next"; event["navigate"] = true;
            if (!event.contains("requestId")) event["requestId"] = "next-" + nowIso8601();
            if (!event.contains("text")) event["text"] = "依据全部活跃课程与最新实际反馈决定下一课，完整备课并保存后进入。";
            return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(db), event).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/learn/next/<string>")([&config](const crow::request&, const std::string& id) {
        try { gangyi::Database db; db.open(config.database_path); auto task = localAgentView(db, id);
            if (task.contains("lesson")) task["href"] = task["lesson"]["href"];
            return crow::response(200, task.dump());
        } catch (const std::exception& error) { return crow::response(404, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/learn/next/<string>/cancel").methods(crow::HTTPMethod::POST)([&config](const crow::request&, const std::string& id) {
        try { gangyi::Database db; db.open(config.database_path);
            return crow::response(202, gangyi::agentControl(db, localAgentAccess(db), {{"command", "cancel"}, {"taskId", id}}).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/learn/next/<string>/retry").methods(crow::HTTPMethod::POST)([&config](const crow::request&, const std::string& id) {
        try { gangyi::Database db; db.open(config.database_path);
            return crow::response(202, gangyi::agentControl(db, localAgentAccess(db), {{"command", "retry"}, {"taskId", id}}).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    // GET /api/learn —— 已准备课堂直接读取提交的内容。
    CROW_ROUTE(app, "/api/learn")([&db](const crow::request& req) {
        try {
            const auto scope = questionRequestBody(req); const auto access = localAgentAccess(db);
            if (scope.contains("lessonId")) return crow::response(200, gangyi::agentLessonView(db, access, scope.at("lessonId")).dump());
            const auto saved = gangyi::agentLegacyLesson(db, access, scope);
            if (saved.empty()) return crow::response(409, nlohmann::json{{"status", "unprepared"}, {"error", "请先让 AI 准备课堂。"}, {"href", "/agent-prepare.html?courseId=" + queryValue(scope.at("courseId").get<std::string>())}}.dump());
            return crow::response(200, gangyi::agentLessonView(db, access, saved.at("lessonId")).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
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

    CROW_ROUTE(app, "/api/learn/progress").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body); const auto access = localAgentAccess(db);
            const auto saved = body.contains("lessonId") ? nlohmann::json{{"lessonId", body.at("lessonId")}} : gangyi::agentLegacyLesson(db, access, body);
            if (saved.empty()) throw std::invalid_argument("课堂尚未准备好");
            const auto event = nlohmann::json{{"type", "lesson_finish_request"}, {"courseId", body.at("courseId")}, {"lessonId", saved.at("lessonId")},
                {"requestId", body.value("requestId", "finish-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))},
                {"text", "我希望结束或更新本课，请按实际互动决定完成情况，不使用固定通过率。"}};
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
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




    CROW_ROUTE(app, "/api/ask").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body);
            static std::atomic_uint64_t sequence{0};
            auto event = body; event["type"] = "chat"; event["text"] = body.value("question", body.value("text", ""));
            event["requestId"] = body.value("requestId", "chat-" + nowIso8601() + "-" + std::to_string(++sequence));
            return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(db), event).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    // 追问入口只读取已有内容，不用固定建议替代 AI 教学。
    CROW_ROUTE(app, "/api/ask/suggestions").methods(crow::HTTPMethod::POST)([] {
        return crow::response(200, nlohmann::json{{"ok", true}, {"suggestions", nlohmann::json::array()}}.dump());
    });

    // 课堂内上下文对话：问题必须携带当前课程目标、阶段和主题，避免退化为全局闲聊。
    CROW_ROUTE(app, "/api/learning-chat").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body);
            static std::atomic_uint64_t sequence{0};
            auto event = body; event["type"] = "chat"; event["text"] = body.value("question", body.value("text", ""));
            event["requestId"] = body.value("requestId", "chat-" + nowIso8601() + "-" + std::to_string(++sequence));
            return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(db), event).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    std::mutex socketMapMutex;
    std::atomic<int> socketWorkerCount{0};
    std::unordered_map<crow::websocket::connection*, std::shared_ptr<ClassroomSocket>> socketMap;
    const auto socketAccept = [](const crow::request& request, void** userData) {
            *userData = new crow::request(request);
            return true;
        };
    const auto socketOpen = [&](crow::websocket::connection& connection) {
            auto state = std::make_shared<ClassroomSocket>();
            std::unique_ptr<crow::request> request(static_cast<crow::request*>(connection.userdata()));
            if (request) state->request = *request;
            connection.userdata(nullptr);
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
        { std::lock_guard<std::mutex> guard(socketMapMutex); const auto found = socketMap.find(&connection);
          if (found != socketMap.end()) state = found->second; }
        if (!state || binary) return;
        const auto body = nlohmann::json::parse(message, nullptr, false);
        if (!body.is_object()) { state->send({{"type", "error"}, {"message", "消息格式无效"}}); return; }
        if (body.value("type", "") == "stop") {
            state->stopRequested = true; state->cancelled = true;
            std::string taskId; { std::lock_guard<std::mutex> guard(state->mutex); taskId = state->taskId; }
            if (!taskId.empty()) try { gangyi::Database db; db.open(config.database_path);
                gangyi::agentControl(db, localAgentAccess(db), {{"command", "cancel"}, {"taskId", taskId}});
            } catch (...) { /* 已完成或失效任务仍保留已有结果。 */ }
            return;
        }
        if (body.value("type", "") != "ask" || state->busy.exchange(true)) return;
        state->cancelled = false; state->stopRequested = false; ++socketWorkerCount;
        std::thread([state, body, classroom, &config, &socketWorkerCount] {
            try {
                gangyi::Database db; db.open(config.database_path); const auto access = localAgentAccess(db);
                nlohmann::json event;
                if (classroom && body.contains("kind")) event = gangyi::agentLegacyEvent(db, access, body);
                else {
                    static std::atomic_uint64_t sequence{0};
                    event = {{"type", "chat"}, {"courseId", body.value("courseId", "")}, {"text", body.at("question")},
                        {"requestId", body.value("requestId", "chat-" + nowIso8601() + "-" + std::to_string(++sequence))},
                        {"conversationId", body.value("conversationId", "general")}};
                    if (body.contains("lessonId")) event["lessonId"] = body.at("lessonId");
                    if (body.contains("selection")) event["selection"] = body.at("selection");
                }
                const auto created = gangyi::agentSubmit(db, access, event); const auto taskId = created.at("id").get<std::string>();
                { std::lock_guard<std::mutex> guard(state->mutex); state->taskId = taskId; }
                if (state->stopRequested) gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", taskId}});
                int sequence = 0;
                while (!state->cancelled) {
                    const auto task = localAgentView(db, taskId);
                    for (const auto& item : task.at("events")) if (item.value("seq", 0) > sequence) {
                        sequence = item.at("seq");
                        if (item.value("type", "") == "delta" && item.value("field", "") == "message")
                            state->send({{"type", "delta"}, {"text", item.at("text")}, {"taskId", taskId}, {"seq", sequence}});
                    }
                    const auto status = task.value("status", "");
                    if (status != "pending" && status != "running") {
                        const bool valid = status == "ready" || status == "waiting_student";
                        state->send({{"type", valid || status == "cancelled" ? "done" : "error"}, {"cancelled", status == "cancelled"},
                            {"message", task.value("error", task.value("message", "本次任务已停止，已有记录保留。"))},
                            {"model", task.value("model", "")}, {"taskId", taskId}, {"version", task.value("version", 0)}, {"taskStatus", status}});
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            } catch (const std::exception& error) { state->send({{"type", "error"}, {"message", error.what()}}); }
            state->busy = false; --socketWorkerCount;
        }).detach();
    };
    CROW_WEBSOCKET_ROUTE(app, "/ws/classroom").onaccept(socketAccept).onopen(socketOpen).onclose(socketClose)
        .onmessage([&](crow::websocket::connection& connection, const std::string& message, bool binary) {
            socketMessage(connection, message, binary, true);
        });
    CROW_WEBSOCKET_ROUTE(app, "/ws/ask").onaccept(socketAccept).onopen(socketOpen).onclose(socketClose)
        .onmessage([&](crow::websocket::connection& connection, const std::string& message, bool binary) {
            socketMessage(connection, message, binary, false);
        });

    // 新旧流式订阅共用任务事件，刷新与断线恢复不重复请求模型。
    const auto subscribeAgentTask = [&](crow::websocket::connection& connection, const std::string& message, bool binary) {
        std::shared_ptr<ClassroomSocket> state;
        { std::lock_guard<std::mutex> guard(socketMapMutex); const auto found = socketMap.find(&connection);
          if (found != socketMap.end()) state = found->second; }
        if (!state || binary || state->busy.exchange(true)) return;
        const auto body = nlohmann::json::parse(message, nullptr, false);
        if (!body.is_object() || (!body.value("taskId", nlohmann::json()).is_string() && !body.value("id", nlohmann::json()).is_string()) ||
            !body.value("afterSeq", nlohmann::json(0)).is_number_integer()) {
            state->send({{"type", "failed"}, {"message", "任务标识无效。"}}); state->busy = false; return;
        }
        ++socketWorkerCount;
        std::thread([state, body, databasePath = config.database_path, &socketWorkerCount] {
            try {
                gangyi::Database db; db.open(databasePath); int sequence = body.value("afterSeq", 0);
                const auto taskId = body.value("taskId", body.value("id", ""));
                while (!state->cancelled) {
                    const auto task = localAgentView(db, taskId);
                    for (const auto& event : task.value("events", nlohmann::json::array())) if (event.value("seq", 0) > sequence) {
                        auto value = event; value["taskId"] = task.at("id");
                        if (value.value("type", "") == "ready" && task.contains("lesson")) value["href"] = task.at("lesson").at("href");
                        state->send(value); sequence = event.at("seq");
                    }
                    state->send({{"type", "state"}, {"task", task}});
                    const auto status = task.value("status", "");
                    if (status != "pending" && status != "running") break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            } catch (const std::exception& error) { state->send({{"type", "failed"}, {"message", error.what()}}); }
            state->busy = false; --socketWorkerCount;
        }).detach();
    };
    CROW_WEBSOCKET_ROUTE(app, "/ws/learn/next").onaccept(socketAccept).onopen(socketOpen).onclose(socketClose).onmessage(subscribeAgentTask);
    CROW_ROUTE(app, "/api/classroom/questions")([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path);
            return crow::response(200, gangyi::agentQuestionView(db, localAgentAccess(db), questionRequestBody(req)).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/classroom/question/skip").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            body["action"] = "skip"; if (!body.contains("question") && !body.contains("answer")) body["question"] = "我跳过这道题";
            const auto access = localAgentAccess(db); const auto event = gangyi::agentLegacyEvent(db, access, body);
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/start")([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path);
            return crow::response(200, gangyi::agentQuestionView(db, localAgentAccess(db), questionRequestBody(req)).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/submit").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);

            const auto access = localAgentAccess(db); const auto event = gangyi::agentLegacyEvent(db, access, body);
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/skip").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            body["type"] = "feedback"; body["text"] = "我跳过当前课堂，请据此调整下一步。";
            static std::atomic_uint64_t sequence{0}; body["requestId"] = body.value("requestId", "skip-lesson-" + nowIso8601() + "-" + std::to_string(++sequence));
            return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(db), body).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/activity/skip").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            body["action"] = "skip"; if (!body.contains("question") && !body.contains("answer")) body["question"] = "我跳过这道题";
            const auto access = localAgentAccess(db); const auto event = gangyi::agentLegacyEvent(db, access, body);
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/state")([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); const auto access = localAgentAccess(db); const auto scope = questionRequestBody(req);
            const auto resolved = scope.contains("lessonId") ? nlohmann::json{{"lessonId", scope.at("lessonId")}} : gangyi::agentLegacyLesson(db, access, scope);
            if (resolved.empty()) throw std::invalid_argument("课堂尚未准备好，请先让 AI 备课");
            return crow::response(200, gangyi::agentLessonView(db, access, resolved.at("lessonId")).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/hint").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            body["action"] = "hint"; if (!body.contains("question") && !body.contains("answer")) body["question"] = "请给我一点提示，不要直接展示答案。";
            const auto access = localAgentAccess(db); const auto event = gangyi::agentLegacyEvent(db, access, body);
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/remedial")([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); const auto access = localAgentAccess(db); const auto scope = questionRequestBody(req);
            const auto resolved = scope.contains("lessonId") ? nlohmann::json{{"lessonId", scope.at("lessonId")}} : gangyi::agentLegacyLesson(db, access, scope);
            if (resolved.empty()) throw std::invalid_argument("课堂尚未准备好，请先让 AI 备课");
            return crow::response(200, gangyi::agentLessonView(db, access, resolved.at("lessonId")).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/review/submit").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            body["kind"] = "review";
            return crow::response(202, submitClassroomBatch(db, localAgentAccess(db), body).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
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
            return crow::response(200, studyPlanAgentAction(db, body, body.value("confirm", false) ? "confirm" : "replan").dump());
        } catch (const std::invalid_argument& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
        catch (...) { return crow::response(400, nlohmann::json{{"error", "重排失败"}}.dump()); }
    });

    CROW_ROUTE(app, "/api/classroom/finish").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body);
            const auto access = localAgentAccess(db); const auto saved = body.contains("lessonId") ? nlohmann::json{{"lessonId", body.at("lessonId")}} : gangyi::agentLegacyLesson(db, access, body);
            if (saved.empty()) throw std::invalid_argument("课堂尚未准备好");
            static std::atomic_uint64_t sequence{0};
            const auto event = nlohmann::json{{"type", "lesson_finish_request"}, {"courseId", body.at("courseId")}, {"lessonId", saved.at("lessonId")},
                {"requestId", body.value("requestId", "finish-" + nowIso8601() + "-" + std::to_string(++sequence))},
                {"text", "我想结束本课。请根据我的实际记录判断是否完成、需要怎样复习或补充，不使用固定通过率。"}};
            return crow::response(202, gangyi::agentSubmit(db, access, event).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    CROW_ROUTE(app, "/api/recent-courses")([&db] {
        crow::response response(200, nlohmann::json{{"courses", gangyi::recentCourses(db)}}.dump());
        response.set_header("Content-Type", "application/json; charset=utf-8");
        response.set_header("Cache-Control", "no-store");
        return response;
    });

    CROW_ROUTE(app, "/api/profile")([&config] {
        gangyi::Database db; db.open(config.database_path);
        auto view = gangyi::profileView(db);
        const auto task = localAgentView(db);
        const auto state = task.value("status", "idle");
        const bool active = (state == "pending" || state == "running") && !task.value("paused", false);
        const auto error = state == "failed" ? task.value("error", "AI 已暂停，可保留结果后重试。") : std::string();
        view["updating"] = active; view["error"] = error; view["controllerStatus"] = state;
        view["abilityStatus"]["updating"] = active; view["abilityStatus"]["error"] = error;
        view["abilityStatus"]["status"] = active ? "updating" : error.empty() ? "ready" : "waiting";
        crow::response response(200, view.dump());
        response.set_header("Cache-Control", "no-store");
        return response;
    });

    CROW_ROUTE(app, "/api/classroom/dialogue")([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto scope = questionRequestBody(req);
            const auto view = gangyi::agentQuestionView(db, localAgentAccess(db), scope);
            for (const auto& question : view.at("questions")) if (question.value("index", -1) == scope.value("index", 0))
                return crow::response(200, nlohmann::json{{"question", question}, {"turns", question.at("dialog")}, {"lessonId", view.at("lessonId")},
                    {"taskId", question.value("taskId", "")}, {"version", question.at("sectionVersion")}}.dump());
            throw std::invalid_argument("本题不存在");
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/classroom/evaluation/retry").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body);
            return crow::response(202, gangyi::agentControl(db, localAgentAccess(db), {{"command", "retry"}, {"taskId", body.at("taskId")}}).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/learn/exposure").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body);
            return crow::response(200, gangyi::agentExposeTask(db, localAgentAccess(db), body).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/classroom/preparation")([&config](const crow::request&) {
        gangyi::Database db; db.open(config.database_path);
        return crow::response(200, localAgentView(db).dump());
    });
    CROW_ROUTE(app, "/api/classroom/preparation/retry").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body.empty() ? "{}" : req.body);
            body["command"] = "retry"; return crow::response(202, gangyi::agentControl(db, localAgentAccess(db), body).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/study-plan")([&db](const crow::request&) {
        crow::response response(200, gangyi::studyPlanView(db).dump()); response.set_header("Cache-Control", "no-store"); return response;
    });
    CROW_ROUTE(app, "/api/study-plan").methods(crow::HTTPMethod::PUT)([&db](const crow::request& req) {
        try { const auto body = nlohmann::json::parse(req.body); auto result = gangyi::editStudyPlan(db, body);
            static std::atomic_uint64_t sequence{0}; result["task"] = gangyi::agentSubmit(db, localAgentAccess(db), {{"type", "schedule_changed"}, {"requestId", body.value("requestId", "time-" + nowIso8601() + "-" + std::to_string(++sequence))}, {"text", "我已保存新的时间和手动安排，请结合最新设置决定后续教学。"}}); return crow::response(200, result.dump()); }
        catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/study-plan/draft").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try { return crow::response(200, gangyi::studyPlanDraft(db, nlohmann::json::parse(req.body)).dump()); }
        catch (...) { return crow::response(409, nlohmann::json{{"error", "编辑状态无效，请刷新"}}.dump()); }
    });
    CROW_ROUTE(app, "/api/study-plan/<string>").methods(crow::HTTPMethod::POST)([&db](const crow::request& req, const std::string& action) {
        try { return crow::response(200, studyPlanAgentAction(db, nlohmann::json::parse(req.body), action).dump()); }
        catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });
    CROW_ROUTE(app, "/api/home/next-step")([&config](const crow::request&) {
        gangyi::Database db; db.open(config.database_path); auto task = localAgentView(db);
        if (task.value("status", "") == "ready" && task.contains("lesson")) task["next"] = task.at("lesson");
        return crow::response(200, task.dump());
    });
    CROW_ROUTE(app, "/api/courses/<string>/preview")([&config](const crow::request&, const std::string& course) {
        gangyi::Database db; db.open(config.database_path);
        try {
            auto value = gangyi::coursePreviewView(db, course);
            if (value.value("status", "") == "pending") {
                const auto task = localAgentView(db); bool preparing = false;
                if (task.value("status", "") == "pending" || task.value("status", "") == "running") {
                    const auto row = db.getClassroomActivity(task.at("id"));
                    if (row && row->courseId == course) {
                        const auto saved = nlohmann::json::parse(row->payload);
                        preparing = saved.at("event").value("type", "") == "course_preview";
                    }
                }
                if (!preparing) { value["status"] = "waiting"; value["message"] = "路线说明等待 AI 更新，可按最新情况重新准备。"; }
            }
            crow::response response(200, value.dump()); response.set_header("Cache-Control", "no-store"); return response;
        }
        catch (...) { return crow::response(404, nlohmann::json{{"error", "课程不存在"}}.dump()); }
    });
    CROW_ROUTE(app, "/api/courses/<string>/preview").methods(crow::HTTPMethod::POST)([&db](const crow::request& req, const std::string& course) {
        try { const auto body = nlohmann::json::parse(req.body); static std::atomic_uint64_t sequence{0}; const auto task = gangyi::agentSubmit(db, localAgentAccess(db), {{"type", "course_preview"}, {"courseId", course}, {"requestId", body.value("requestId", "preview-" + nowIso8601() + "-" + std::to_string(++sequence))}, {"text", "请读取同一份最新大纲，调用 update_course_preview 保存你选择的课程路线说明。"}}); return crow::response(202, task.dump()); }
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

    CROW_ROUTE(app, "/api/next-learning")([&config](const crow::request&) {
        gangyi::Database db; db.open(config.database_path);
        const auto task = localAgentView(db);
        nlohmann::json result = {{"ok", true}, {"action", "waiting"}, {"reason", task.value("message", "等待 AI 根据实际表现准备下一步。")}};
        if (task.contains("lesson")) { result["action"] = "prepared"; result["href"] = task.at("lesson").at("href"); result["lesson"] = task.at("lesson"); }
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

    CROW_ROUTE(app, "/api/home/recommendations")([databasePath = config.database_path](const crow::request&) {
        gangyi::Database connection; connection.open(databasePath);
        auto current = gangyi::agentRecommendationView(connection, localAgentAccess(connection));
        if (current.empty()) current = nlohmann::json{{"status", "pending"}, {"items", {{"lite", nlohmann::json::array()}, {"deep", nlohmann::json::array()}}}, {"message", "等待真实 AI 完成启动推荐"}};
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
            auto value = localAgentView(connection, req.url_params.get("taskId") ? req.url_params.get("taskId") : "");
            value["adjustments"] = value.value("changeHistory", nlohmann::json::array());
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
    CROW_WEBSOCKET_ROUTE(app, "/ws/learning-agent").onaccept(socketAccept).onopen(socketOpen).onclose(socketClose).onmessage(subscribeAgentTask);

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

    CROW_ROUTE(app, "/api/quiz-attempts").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto body = nlohmann::json::parse(req.body);
            body["kind"] = "quiz";
            return crow::response(202, submitClassroomBatch(db, localAgentAccess(db), body).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
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
    CROW_ROUTE(app, "/api/generate-plan").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); auto event = nlohmann::json::parse(req.body);
            if (!event.value("goal", nlohmann::json()).is_string() || event.at("goal").get<std::string>().empty())
                return crow::response(400, nlohmann::json{{"error", "请填写学习目标。"}}.dump());
            event["type"] = "plan_course"; event["text"] = "按我的目标规划并保存课程，阶段及题量由你决定：" + event.at("goal").get<std::string>();
            static std::atomic_uint64_t sequence{0};
            if (!event.contains("requestId")) event["requestId"] = "plan-" + nowIso8601() + "-" + std::to_string(++sequence);
            return crow::response(202, gangyi::agentSubmit(db, localAgentAccess(db), event).dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
    });

    // POST /api/courses —— 保存课程 + 快照（质量门禁 + 脱敏 + upsert）
    CROW_ROUTE(app, "/api/courses").methods(crow::HTTPMethod::POST)([&config](const crow::request& req) {
        try { gangyi::Database db; db.open(config.database_path); const auto body = nlohmann::json::parse(req.body); const auto task = localAgentView(db, body.at("taskId"));
            if (task.at("status") != "ready" || !task.contains("course")) throw std::invalid_argument("课程尚未由主控完整保存");
            return crow::response(200, nlohmann::json{{"ok", true}, {"courseId", task.at("course").at("id")}, {"href", task.at("course").at("href")}}.dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
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
            nlohmann::json current = nullptr;
            if (const auto course = gangyi::currentCourseForIdentity(db))
                current = {{"id", course->id}, {"title", course->title}, {"href", "/plan?courseId=" + course->id}};
            return crow::response(200, nlohmann::json{{"courses", arr}, {"currentCourse", current}}.dump());
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
            nlohmann::json cards = nlohmann::json::array();
            forEachTopic(found->payload, [&](int phaseIndex, int topicIndex, const std::string&) {
                const auto saved = db.findLearningCardProgress(courseId, phaseIndex, topicIndex);
                cards.push_back({{"phaseIndex", phaseIndex}, {"topicIndex", topicIndex},
                    {"status", saved ? saved->status : "not_started"}});
            });
            return crow::response(200, nlohmann::json{{"course", course},
                {"snapshot", {{"payload", gangyi::publicCoursePayload(found->payload)}}}, {"cards", cards}}.dump());
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


    // POST /api/phase-expansion —— 首次由 AI 生成并保存，后续复用课程快照。

    CROW_ROUTE(app, "/api/phase-expansion").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try { const auto body = nlohmann::json::parse(req.body); const auto found = gangyi::getCourseWithSnapshot(db, body.at("courseId"));
            if (!found) throw std::invalid_argument("课程不存在");
            const int index = body.value("phaseIndex", 1) - 1; const auto outline = found->payload.value("courseStructure", nlohmann::json::array());
            if (index < 0 || index >= static_cast<int>(outline.size())) throw std::invalid_argument("阶段不存在");
            return crow::response(200, nlohmann::json{{"ok", true}, {"cached", true}, {"phase", outline.at(index)}, {"resources", found->payload.value("resources", nlohmann::json::array())}}.dump());
        } catch (const std::exception& error) { return crow::response(409, nlohmann::json{{"error", error.what()}}.dump()); }
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
    // 画像由同一真实 AI 主控按学习事件更新，不再启动独立评分线程。
    std::cout << "gangyiAI " << gangyi::kVersion << " listening on " << config.host << ':' << config.port << '\n';
    app.bindaddr(config.host).port(config.port).multithreaded().run();
    learningAgent.stop();
    { std::lock_guard<std::mutex> guard(socketMapMutex); for (const auto& [connection, state] : socketMap) state->cancelled = true; }
    while (socketWorkerCount > 0) std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

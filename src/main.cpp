#include "config.hpp"
#include "version.hpp"
#include "db.hpp"
#include "ai_client.hpp"
#include "page_renderer.hpp"
#include "plan_generator.hpp"
#include "learning_generator.hpp"
#include "plan_cache.hpp"
#include "plan_adapter.hpp"
#include "search_client.hpp"
#include "quality_gate.hpp"
#include "course_service.hpp"
#include "progress_service.hpp"
#include "phase_generator.hpp"

#include <crow.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string content_type_for(const std::filesystem::path& path) {
    const auto extension = path.extension().string();
    if (extension == ".html") return "text/html; charset=utf-8";
    if (extension == ".css") return "text/css; charset=utf-8";
    if (extension == ".js") return "application/javascript; charset=utf-8";
    if (extension == ".json") return "application/json; charset=utf-8";
    return "application/octet-stream";
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

bool requesterCanReadCourse(const gangyi::Course& course, const crow::request& req) {
    const char* anonymousId = req.url_params.get("anonymousId");
    const std::string requesterAnonymousId = anonymousId ? anonymousId : "";
    if (course.userId && !course.userId->empty()) return false;
    if (course.anonymousId && !course.anonymousId->empty()) {
        return *course.anonymousId == requesterAnonymousId;
    }
    return true;
}

bool anonymousCanAccessCourse(gangyi::Database& db, const std::string& courseId, const std::string& anonymousId) {
    if (courseId.empty()) return true;
    const auto course = db.getCourse(courseId);
    if (!course || course->status != "active") return false;
    if (course->userId && !course->userId->empty()) return false;
    if (course->anonymousId && !course->anonymousId->empty()) return *course->anonymousId == anonymousId;
    return true;
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
    const std::string referer = req.get_header_value("Referer");
    return queryValueFromUrl(referer, "anonymousId");
}

}  // namespace

int main() {
    const auto config = gangyi::Config::from_environment();
    gangyi::Database db;
    // 自动创建数据库所在目录（首次部署时 data/ 可能不存在，避免 sqlite 打开失败）
    try {
        const std::filesystem::path dbPath(config.database_path);
        if (dbPath.has_parent_path()) std::filesystem::create_directories(dbPath.parent_path());
    } catch (...) {}
    db.open(config.database_path);
    db.migrate();
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([] {
        crow::response response(gangyi::renderHomePage());
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/plan")([](const crow::request& req) {
        const char* goal = req.url_params.get("goal");
        const char* mode = req.url_params.get("mode");
        const char* courseId = req.url_params.get("courseId");
        const char* anonymousId = req.url_params.get("anonymousId");
        crow::response response(gangyi::renderPlanPage(goal ? goal : "", mode ? mode : "deep",
            courseId ? courseId : "", anonymousId ? anonymousId : ""));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/learn")([](const crow::request& req) {
        const char* courseId = req.url_params.get("courseId");
        const char* goal = req.url_params.get("goal");
        const char* mode = req.url_params.get("mode");
        const char* phaseName = req.url_params.get("phaseName");
        const char* topic = req.url_params.get("topic");
        const char* phaseIndex = req.url_params.get("phaseIndex");
        const char* topicIndex = req.url_params.get("topicIndex");
        const char* anonymousId = req.url_params.get("anonymousId");
        const char* regenerate = req.url_params.get("regenerate");
        const char* forceLearn = req.url_params.get("forceLearn");
        const char* retry = req.url_params.get("retry");
        crow::response response(gangyi::renderLearnPage(
            courseId ? courseId : "",
            goal ? goal : "",
            mode ? mode : "deep",
            phaseIndex ? phaseIndex : "1",
            phaseName ? phaseName : "",
            topicIndex ? topicIndex : "1",
            topic ? topic : "",
            anonymousId ? anonymousId : "",
            regenerate ? regenerate : "",
            forceLearn ? forceLearn : "",
            retry ? retry : ""));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

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
        if (!courseId.empty()) {
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
                            if (c.courseId.value_or("") == courseId && c.phaseIndex == idx && c.status == "completed") ++done;
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
        if (!courseId.empty()) {
            if (const auto found = gangyi::getCourseWithSnapshot(db, courseId)) {
                if (found->payload.is_object()) plan = found->payload;
                if (goal.empty()) goal = found->course.goal;
                if (mode.empty()) mode = found->course.mode;
                int phase = 1;
                try { phase = std::stoi(phaseIndex); if (phase < 1) phase = 1; } catch (...) {}
                for (const auto& r : db.listLearningCardProgress()) {
                    if (r.courseId.value_or("") == courseId && r.phaseIndex == phase &&
                        (r.status == "completed" || r.status == "in_progress")) {
                        card[std::to_string(r.topicIndex)] = r.status;
                    }
                }
            }
        }
        crow::response response(gangyi::renderPhasePage(courseId, anonymousId, goal, mode, phaseIndex, phaseName, plan, card));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/login")([] {
        crow::response response(gangyi::renderLoginPage());
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/ask")([](const crow::request& req) {
        const char* goal = req.url_params.get("goal");
        crow::response response(gangyi::renderAskPage(goal ? goal : ""));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/my-courses")([&db] {
        auto courses = db.listCourses();
        std::sort(courses.begin(), courses.end(), [](const gangyi::Course& a, const gangyi::Course& b) { return a.updatedAt > b.updatedAt; });
        nlohmann::json list = nlohmann::json::array();
        for (const auto& c : courses) {
            int pct = 0;
            if (const auto cp = db.findProgressByCourseId(c.id)) pct = cp->overallPercent;
            const std::string status = pct >= 100 ? "completed" : pct > 0 ? "in_progress" : "not_started";
            const std::string title = c.title.empty() ? (c.goal.empty() ? c.id : c.goal) : c.title;
            list.push_back({{"courseId", c.id}, {"title", title}, {"goal", c.goal}, {"mode", c.mode},
                {"source", c.source}, {"createdAt", c.createdAt}, {"updatedAt", c.updatedAt},
                {"overallPercent", pct}, {"status", status},
                {"learnHref", "/learn?courseId=" + c.id + "&phaseIndex=1&topicIndex=1"},
                {"progressHref", "/progress?courseId=" + c.id}});
        }
        crow::response response(gangyi::renderMyCoursesPage({{"courses", list}}));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

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
            if (!anonymousCanAccessCourse(db, safeCourseId, safeAnonymousId)) {
                return crow::response(404, nlohmann::json{{"error", "course not found"}}.dump());
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
            const bool force = (regenerate && std::string(regenerate) == "1") ||
                (forceLearn && std::string(forceLearn) == "1") || (retry && *retry);

            // 1) 会话恢复（有 courseId 且非强制重新生成时）
            if (!safeCourseId.empty() && !force) {
                const auto session = gangyi::findLearningSession(db, safeCourseId, safeAnonymousId,
                    safeGoal, safeMode, phaseIndex, topicIndex);
                if (session && session->contains("content") && (*session)["content"].is_string()) {
                    const bool fallback = (*session).value("fallbackUsed", false) ||
                        (*session).value("source", "") == "fallback";
                    if (!fallback) {
                        try {
                            auto content = nlohmann::json::parse((*session)["content"].get<std::string>());
                            if (content.is_object()) {
                                content["notice"] = "已恢复上次生成的学习内容";
                                return crow::response(200, content.dump());
                            }
                        } catch (...) {}
                    }
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
                        {"phaseIndex", phaseIndex}, {"phaseName", safePhaseName},
                        {"topicIndex", topicIndex}, {"topicTitle", safeTopic},
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
            return crow::response(code, nlohmann::json{{"error", e.what()}, {"type", e.errorType}}.dump());
        } catch (const std::exception& e) {
            return crow::response(502, nlohmann::json{{"error", e.what()}}.dump());
        }
    });

    CROW_ROUTE(app, "/api/auth/login").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        const auto body = req.body;
        return crow::response(200, nlohmann::json{{"ok", true}, {"message", "登录接口已接通"}, {"bodyLength", static_cast<int>(body.size())}}.dump());
    });

    CROW_ROUTE(app, "/api/ask").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        return crow::response(200, nlohmann::json{{"ok", true}, {"message", "问答接口已接通"}, {"bodyLength", static_cast<int>(req.body.size())}}.dump());
    });

    CROW_ROUTE(app, "/health")([] {
        crow::response response(nlohmann::json{{"status", "healthy"}}.dump());
        response.set_header("Content-Type", "application/json; charset=utf-8");
        return response;
    });

    // POST /api/generate-plan —— 课程规划生成（缓存 → 生成 → 适配 MockPlan 形状 → 搜索补充资源）
    CROW_ROUTE(app, "/api/generate-plan").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string goal = body.value("goal", "");
            const std::string mode = body.value("mode", "deep");
            const bool bypassCache = body.value("bypassCache", false) ||
                body.value("forcePlan", false) || body.contains("retry");
            if (goal.empty()) {
                return crow::response(400, nlohmann::json{{"error", "goal 不能为空"}}.dump());
            }
            gangyi::AIClient ai;
            gangyi::PlanGenerator generator(ai);
            gangyi::PlanCache cache;

            nlohmann::json adapted;
            bool fromCache = false;
            if (!bypassCache) {
                if (auto cached = cache.read(goal, mode)) {
                    const auto& raw = *cached;
                    // 缓存内容可能是适配后的 MockPlan 形状（roadmap/courseStructure）或原始 GeneratedPlan 形状
                    if (raw.contains("roadmap") && raw.contains("courseStructure")) {
                        adapted = raw;
                    } else {
                        adapted = gangyi::adaptGeneratedPlan(raw, mode);
                    }
                    fromCache = true;
                }
            }
            if (!fromCache) {
                const auto plan = generator.generate(goal, mode);
                const auto raw = planToJson(plan);
                adapted = gangyi::adaptGeneratedPlan(raw, mode);
                cache.write(goal, mode, adapted);
            }

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
            // 错误→HTTP 映射（对齐 docs/ai-spec.md）：missing_config/auth_error→503，timeout→504，其余→502
            int code = 502;
            if (e.errorType == "missing_config" || e.errorType == "auth_error") code = 503;
            else if (e.errorType == "timeout") code = 504;
            return crow::response(code, nlohmann::json{{"error", e.what()}, {"type", e.errorType}}.dump());
        } catch (const std::exception& e) {
            return crow::response(502, nlohmann::json{{"error", e.what()}}.dump());
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

            const auto gate = gangyi::validateCourseContent(body["payload"], goal, mode, title);
            if (!gate.valid) {
                return crow::response(422, nlohmann::json{{"ok", false}, {"error", "COURSE_SNAPSHOT_INVALID"},
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
            std::string href = "/plan?courseId=" + *courseId;
            if (!anonymousId.empty()) href += "&anonymousId=" + anonymousId;
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
            const auto courses = gangyi::listCoursesForIdentity(db, "", anonymousId ? anonymousId : "", limit, offset);
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
            if (!found || !requesterCanReadCourse(found->course, req)) {
                return crow::response(404, nlohmann::json{{"error", "course not found"}}.dump());
            }
            const nlohmann::json course = {{"id", found->course.id}, {"goal", found->course.goal},
                {"mode", found->course.mode}, {"title", found->course.title},
                {"summary", found->course.summary ? nlohmann::json(*found->course.summary) : nlohmann::json(nullptr)},
                {"source", found->course.source},
                {"createdAt", found->course.createdAt}, {"updatedAt", found->course.updatedAt}};
            return crow::response(200, nlohmann::json{{"course", course},
                {"snapshot", {{"payload", found->payload}}}}.dump());
        } catch (...) {
            return crow::response(500, nlohmann::json{{"error", "course load failed"}}.dump());
        }
    });

    // ---- 阶段 B：学习体验 API ----

    // POST /api/learning-card-progress —— 学习卡状态（写后自动重算）
    CROW_ROUTE(app, "/api/learning-card-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!anonymousCanAccessCourse(db, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            auto result = gangyi::saveLearningCardProgress(db, body);
            if (!result.ok) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
            }
            if (body.contains("courseId") && body["courseId"].is_string() && !body["courseId"].get<std::string>().empty()) {
                gangyi::recomputeCourseProgress(db, body["courseId"].get<std::string>(),
                    body.value("anonymousId", ""), body.value("goal", ""));
            }
            return crow::response(200, nlohmann::json{{"ok", true}, {"item", result.item}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
        }
    });

    // POST /api/learning-step-progress —— 步骤理解状态（写后自动重算）
    CROW_ROUTE(app, "/api/learning-step-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!anonymousCanAccessCourse(db, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            auto result = gangyi::saveLearningStepProgress(db, body);
            if (!result.ok) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
            }
            if (body.contains("courseId") && body["courseId"].is_string() && !body["courseId"].get<std::string>().empty()) {
                gangyi::recomputeCourseProgress(db, body["courseId"].get<std::string>(),
                    body.value("anonymousId", ""), body.value("goal", ""));
            }
            return crow::response(200, nlohmann::json{{"ok", true}, {"item", result.item}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
        }
    });

    // POST /api/task-progress —— 阶段任务状态（写后自动重算）
    CROW_ROUTE(app, "/api/task-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!anonymousCanAccessCourse(db, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            auto result = gangyi::saveTaskProgress(db, body);
            if (!result.ok) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
            }
            if (body.contains("courseId") && body["courseId"].is_string() && !body["courseId"].get<std::string>().empty()) {
                gangyi::recomputeCourseProgress(db, body["courseId"].get<std::string>(),
                    body.value("anonymousId", ""), body.value("goal", ""));
            }
            return crow::response(200, nlohmann::json{{"ok", true}, {"item", result.item}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
        }
    });

    // POST /api/learn/progress —— 三合一：学习卡 + 断点记录 + 全量重算
    CROW_ROUTE(app, "/api/learn/progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!anonymousCanAccessCourse(db, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            const std::string goal = body.value("goal", "");
            const std::string mode = body.value("mode", "deep");
            // 1) upsert 学习卡（topicIndex 0→1；phaseIndex ≥1）
            nlohmann::json cardBody = body;
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
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
        }
    });

    // POST /api/course-progress —— 全量重算 / 断点记录
    CROW_ROUTE(app, "/api/course-progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string action = body.value("action", "recompute");
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!anonymousCanAccessCourse(db, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            const std::string goal = body.value("goal", "");
            const std::string mode = body.value("mode", "deep");
            if (action == "lastVisited") {
                gangyi::updateLastVisited(db, courseId, anonymousId, goal, mode, body);
            }
            auto progress = gangyi::recomputeCourseProgress(db, courseId, anonymousId, goal);
            if (!progress) progress = nlohmann::json{{"overallPercent", 0}, {"completedCount", 0}, {"totalCount", 0}};
            return crow::response(200, nlohmann::json{{"ok", true}, {"progress", *progress}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
        }
    });

    CROW_ROUTE(app, "/api/course-progress")([&db](const crow::request& req) {
        try {
            const char* courseId = req.url_params.get("courseId");
            const char* anonymousId = req.url_params.get("anonymousId");
            if (!courseId || !*courseId) return crow::response(400, nlohmann::json{{"ok", false}, {"error", "courseId required"}}.dump());
            if (!anonymousCanAccessCourse(db, courseId, anonymousId ? anonymousId : "")) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
            const auto progress = gangyi::recomputeCourseProgress(db, courseId, anonymousId ? anonymousId : "", "");
            return crow::response(200, nlohmann::json{{"ok", true}, {"progress", progress ? *progress : nlohmann::json::object()}}.dump());
        } catch (...) {
            return crow::response(500, nlohmann::json{{"ok", false}, {"error", "progress load failed"}}.dump());
        }
    });

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
            if (!anonymousCanAccessCourse(db, courseId ? courseId : "", anonymousId ? anonymousId : "")) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
            const auto session = gangyi::findLearningSession(db, courseId ? courseId : "",
                anonymousId ? anonymousId : "", goal ? goal : "", mode ? mode : "deep", phaseIndex, topicIndex);
            if (!session) {
                return crow::response(200, nlohmann::json{{"ok", true}, {"session", nullptr}}.dump());
            }
            return crow::response(200, nlohmann::json{{"ok", true}, {"session", *session}}.dump());
        } catch (...) {
            return crow::response(500, nlohmann::json{{"ok", false}, {"error", "session load failed"}}.dump());
        }
    });

    // POST /api/learning-sessions —— 微课会话保存
    CROW_ROUTE(app, "/api/learning-sessions").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const bool ok = gangyi::upsertLearningSession(db, body);
            return crow::response(ok ? 200 : 400, nlohmann::json{{"ok", ok}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
        }
    });

    // POST /api/phase-expansion —— 阶段展开生成
    CROW_ROUTE(app, "/api/phase-expansion").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string goal = body.value("goal", "");
            const std::string mode = body.value("mode", "deep");
            const int phaseIndex = body.value("phaseIndex", 1);
            const std::string stage = body.value("stage", "");
            std::vector<std::string> topics;
            if (body.contains("topics") && body["topics"].is_array()) {
                for (const auto& t : body["topics"]) if (t.is_string()) topics.push_back(t.get<std::string>());
            }
            if (goal.empty()) {
                return crow::response(400, nlohmann::json{{"ok", false}, {"error", "goal 不能为空"}}.dump());
            }
            gangyi::AIClient ai;
            gangyi::PhaseGenerator generator(ai);
            const auto result = generator.generate(goal, mode, phaseIndex, stage, topics, {});
            if (!result) {
                return crow::response(200, nlohmann::json{{"ok", false},
                    {"message", "阶段内容暂未生成完成，请稍后重试。"}}.dump());
            }
            return crow::response(200, nlohmann::json{{"ok", true}, {"phase", *result}}.dump());
        } catch (const gangyi::AIClientError& e) {
            int code = 502;
            if (e.errorType == "missing_config" || e.errorType == "auth_error") code = 503;
            else if (e.errorType == "timeout") code = 504;
            return crow::response(code, nlohmann::json{{"error", e.what()}, {"type", e.errorType}}.dump());
        } catch (const std::exception& e) {
            return crow::response(502, nlohmann::json{{"error", e.what()}}.dump());
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

    std::cout << "gangyiAI " << gangyi::kVersion << " listening on 0.0.0.0:" << config.port << '\n';
    app.port(config.port).multithreaded().run();
}

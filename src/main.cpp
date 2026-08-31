#include "config.hpp"
#include "version.hpp"
#include "db.hpp"
#include "ai_client.hpp"
#include "page_renderer.hpp"
#include "plan_generator.hpp"
#include "learning_generator.hpp"

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

}  // namespace

int main() {
    const auto config = gangyi::Config::from_environment();
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([] {
        crow::response response(gangyi::renderHomePage());
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/plan")([](const crow::request& req) {
        const char* goal = req.url_params.get("goal");
        const char* mode = req.url_params.get("mode");
        crow::response response(gangyi::renderPlanPage(goal ? goal : "", mode ? mode : "deep"));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/learn")([](const crow::request& req) {
        const char* courseId = req.url_params.get("courseId");
        const char* phaseIndex = req.url_params.get("phaseIndex");
        const char* topicIndex = req.url_params.get("topicIndex");
        crow::response response(gangyi::renderLearnPage(courseId ? courseId : "", phaseIndex ? phaseIndex : "0", topicIndex ? topicIndex : "0"));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/progress")([](const crow::request& req) {
        const char* courseId = req.url_params.get("courseId");
        crow::response response(gangyi::renderProgressPage(courseId ? courseId : ""));
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

    CROW_ROUTE(app, "/my-courses")([] {
        crow::response response(gangyi::renderMyCoursesPage());
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/api/learn")([](const crow::request& req) {
        try {
            gangyi::AIClient ai;
            gangyi::LearningGenerator generator(ai);
            const char* courseId = req.url_params.get("courseId");
            const char* phaseIndex = req.url_params.get("phaseIndex");
            const char* topicIndex = req.url_params.get("topicIndex");
            const char* mode = req.url_params.get("mode");
            const std::string safeCourseId = courseId ? courseId : "";
            const std::string safePhaseIndex = phaseIndex ? phaseIndex : "0";
            const std::string safeTopicIndex = topicIndex ? topicIndex : "0";
            const std::string safeMode = mode ? mode : "deep";
            const std::string phaseName = "阶段 " + safePhaseIndex;
            const std::string topic = "主题 " + safeTopicIndex;
            std::vector<gangyi::SearchResource> resources;
            const auto answer = generator.generate(safeCourseId.empty() ? topic : safeCourseId, phaseName, topic, safeMode, resources);
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
            return crow::response(200, result.dump());
        } catch (const gangyi::AIClientError& e) {
            return crow::response(502, nlohmann::json{{"error", e.what()}, {"type", e.errorType}}.dump());
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

    // POST /api/generate-plan —— 课程规划生成
    CROW_ROUTE(app, "/api/generate-plan").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string goal = body.value("goal", "");
            const std::string mode = body.value("mode", "deep");
            if (goal.empty()) {
                return crow::response(400, nlohmann::json{{"error", "goal 不能为空"}}.dump());
            }
            gangyi::AIClient ai;
            gangyi::PlanGenerator generator(ai);
            const auto plan = generator.generate(goal, mode);
            const auto result = planToJson(plan);
            return crow::response(200, result.dump());
        } catch (const gangyi::AIClientError& e) {
            return crow::response(502, nlohmann::json{{"error", e.what()}, {"type", e.errorType}}.dump());
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

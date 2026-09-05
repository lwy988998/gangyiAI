#include "config.hpp"
#include "version.hpp"
#include "db.hpp"
#include "ai_client.hpp"
#include "page_renderer.hpp"
#include "plan_generator.hpp"
#include "plan_cache.hpp"
#include "plan_adapter.hpp"
#include "search_client.hpp"
#include "quality_gate.hpp"
#include "course_service.hpp"
#include "phase_generator.hpp"
#include "ask_generator.hpp"
#include "image_goal_analyzer.hpp"

#include <crow.h>
#include <crow/multipart.h>
#include <nlohmann/json.hpp>

#ifdef DELETE
#undef DELETE
#endif

#include <filesystem>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

nlohmann::json fallbackCoursePlan(const std::string& goal, const std::string& mode) {
    const int phaseCount = mode == "lite" ? 3 : 4;
    nlohmann::json roadmap = nlohmann::json::array();
    nlohmann::json structure = nlohmann::json::array();
    nlohmann::json slides = nlohmann::json::array();
    for (int index = 1; index <= phaseCount; ++index) {
        const std::string phaseName = goal + "·第" + std::to_string(index) + "阶段";
        const std::vector<std::string> topics = {
            phaseName + "核心概念", phaseName + "典型练习", phaseName + "应用检查"};
        const std::vector<std::string> tasks = {
            "完成「" + goal + "」核心概念练习并记录过程",
            "提交一份「" + goal + "」阶段报告或作品"};
        nlohmann::json steps = nlohmann::json::array();
        for (size_t step = 0; step < topics.size(); ++step) {
            steps.push_back({
                {"title", topics[step]},
                {"explanation", "围绕「" + topics[step] + "」整理定义、方法和一个具体例子。"},
                {"action", "完成「" + topics[step] + "」练习并记录关键步骤。"},
                {"check", "用自己的话解释「" + topics[step] + "」，并提交一份可检查的练习作品。"}});
        }
        roadmap.push_back({
            {"name", phaseName}, {"duration", mode == "lite" ? "1 周" : "2 周"},
            {"goal", "掌握「" + phaseName + "」的核心知识与方法"},
            {"description", "从具体知识点出发，完成练习、复盘和阶段产出。"},
            {"topics", topics}, {"tasks", tasks}, {"steps", steps},
            {"output", "一份可检查的「" + goal + "」阶段作品或报告"},
            {"checkpoint", "完成练习并提交阶段作品，能够说明关键步骤。"},
            {"commonMistakes", nlohmann::json::array({"只看讲解不完成练习", "没有记录检查结果"})}});
        structure.push_back({{"stage", phaseName}, {"topics", topics}});
        slides.push_back({{"title", phaseName}, {"subtitle", goal},
                          {"content", "本阶段围绕具体知识点、练习任务和阶段作品推进。"},
                          {"bullets", topics}});
    }
    return {{"title", goal + (mode == "lite" ? "快速学习方案" : "系统学习方案")},
            {"summary", "围绕你的目标生成可执行的阶段路线、练习和检查标准。"},
            {"courseIntro", "每个阶段都有明确知识点、行动任务、阶段作品和检查点。"},
            {"overview", "按阶段完成知识学习、练习、复盘和成果提交。"},
            {"duration", mode == "lite" ? "3 周" : "8 周"}, {"roadmap", roadmap},
            {"courseStructure", structure}, {"slides", slides},
            {"mindMap", {{"title", "课程知识结构"}, {"nodes", nlohmann::json::array()}}},
            {"resources", nlohmann::json::array()},
            {"projects", nlohmann::json::array({{{"name", goal + "阶段作品"}, {"difficulty", "入门"},
                {"duration", mode == "lite" ? "2 小时" : "4 小时"},
                {"output", "一份可展示的" + goal + "作品"}}})}};
}

std::string qualityFeedback(const gangyi::QualityResult& result) {
    std::ostringstream out;
    out << "质量评分：" << result.score << "。问题：";
    if (result.reasons.empty()) out << "课程结构或内容不完整。";
    else for (const auto& reason : result.reasons) out << "\n- " << reason;
    return out.str();
}

bool requesterCanReadCourse(gangyi::Database& db, const gangyi::Course& course, const crow::request& req) {
    const char* anonymousId = req.url_params.get("anonymousId");
    const std::string requesterAnonymousId = anonymousId ? anonymousId : "";
    if (course.userId && !course.userId->empty()) return false;
    if (course.anonymousId && !course.anonymousId->empty()) {
        return *course.anonymousId == requesterAnonymousId;
    }
    return true;
}

bool requesterCanAccessCourse(gangyi::Database& db, const crow::request& req, const std::string& courseId, const std::string& anonymousId) {
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

std::string encodeQueryValue(const std::string& value) {
    std::ostringstream encoded;
    encoded << std::uppercase << std::hex << std::setfill('0');
    for (const unsigned char character : value) {
        if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9') || character == '-' || character == '_' ||
            character == '.' || character == '~') {
            encoded << character;
        } else {
            encoded << '%' << std::setw(2) << static_cast<int>(character) << std::setw(0);
        }
    }
    return encoded.str();
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

    CROW_ROUTE(app, "/ask")([](const crow::request& req) {
        const char* question = req.url_params.get("question");
        crow::response response(gangyi::renderAskPage(question ? question : ""));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/my-courses")([&db](const crow::request& req) {
        const std::string anonymousId = requestAnonymousId(req);
        const auto courses = gangyi::listCoursesForIdentity(db, "", anonymousId, 100, 0);
        nlohmann::json list = nlohmann::json::array();
        for (const auto& c : courses) {
            const std::string title = c.title.empty() ? (c.goal.empty() ? c.id : c.goal) : c.title;
            list.push_back({{"courseId", c.id}, {"title", title}, {"goal", c.goal}, {"mode", c.mode},
                {"source", c.source}, {"createdAt", c.createdAt}, {"updatedAt", c.updatedAt},
                {"status", "generated"}});
        }
        crow::response response(gangyi::renderMyCoursesPage({{"courses", list}, {"anonymousId", anonymousId},
            {"stats", {{"total", static_cast<int>(courses.size())}}}}));
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
            return crow::response(code, nlohmann::json{{"error", e.what()}, {"type", e.errorType}}.dump());
        } catch (const std::exception& e) {
            return crow::response(502, nlohmann::json{{"error", e.what()}}.dump());
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

    CROW_ROUTE(app, "/api/ask").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            gangyi::AIClient ai;
            gangyi::AskGenerator generator(ai);
            const auto answer = generator.generate(body.value("question", ""));
            return crow::response(200, nlohmann::json{{"ok", true}, {"answer", answer.content},
                {"model", "deepseek-v4-flash"}}.dump());
        } catch (const gangyi::AIClientError& error) {
            const int status = error.errorType == "invalid_request" ? 400 :
                (error.errorType == "missing_config" || error.errorType == "auth_error" ? 503 :
                (error.errorType == "timeout" ? 504 : 502));
            return crow::response(status, nlohmann::json{{"ok", false}, {"error", error.what()}, {"type", error.errorType}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
        }
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
            gangyi::QualityResult gate;
            std::string feedback;
            bool fromCache = false;
            bool qualityValid = false;
            if (!bypassCache) {
                if (auto cached = cache.read(goal, mode)) {
                    const auto& raw = *cached;
                    // 缓存内容可能是适配后的 MockPlan 形状（roadmap/courseStructure）或原始 GeneratedPlan 形状
                    if (raw.contains("roadmap") && raw.contains("courseStructure")) {
                        adapted = raw;
                    } else {
                        adapted = gangyi::adaptGeneratedPlan(raw, mode);
                    }
                    gate = gangyi::validateCourseContent(adapted, goal, mode,
                        adapted.value("title", goal));
                    qualityValid = gate.valid;
                    fromCache = qualityValid;
                    if (!qualityValid) feedback = qualityFeedback(gate);
                }
            }
            if (!fromCache) {
                for (int attempt = 0; attempt < 3 && !qualityValid; ++attempt) {
                    try {
                        const auto plan = generator.generate(goal, mode, feedback);
                        const auto raw = planToJson(plan);
                        adapted = gangyi::adaptGeneratedPlan(raw, mode);
                        gate = gangyi::validateCourseContent(adapted, goal, mode,
                            adapted.value("title", goal));
                        qualityValid = gate.valid;
                        if (!qualityValid) feedback = qualityFeedback(gate);
                    } catch (const gangyi::AIClientError& error) {
                        feedback = "上一次模型输出无法使用：" + std::string(error.what()) +
                            "。请重新输出完整、严格符合字段结构的 JSON。";
                    } catch (const std::exception& error) {
                        feedback = "上一次生成结果解析失败：" + std::string(error.what()) +
                            "。请重新输出完整 JSON，不要输出解释文字。";
                    }
                }
                if (!qualityValid) {
                    adapted = fallbackCoursePlan(goal, mode);
                    gate = gangyi::validateCourseContent(adapted, goal, mode,
                        adapted.value("title", goal));
                    qualityValid = gate.valid;
                    adapted["qualityNotice"] = "模型结果已根据质量检查反馈自动重试；当前展示的是稳定可用的课程结构。";
                }
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
            const auto courses = gangyi::listCoursesForIdentity(db, "", safeAnonymousId, limit, offset);
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

    CROW_ROUTE(app, "/api/my-courses/<string>").methods(crow::HTTPMethod::DELETE)([&db](const crow::request& req, std::string courseId) {
        const std::string anonymousId = requestAnonymousId(req);
        if (!gangyi::deleteCourseForIdentity(db, courseId, "", anonymousId)) {
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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

    // GET /api/learning-step-progress —— 恢复阶段展开步骤的理解状态。
    CROW_ROUTE(app, "/api/learning-step-progress")([&db](const crow::request& req) {
        const char* courseId = req.url_params.get("courseId");
        const std::string anonymousId = requestAnonymousId(req);
        const char* goal = req.url_params.get("goal");
        const char* mode = req.url_params.get("mode");
        const char* phaseIndex = req.url_params.get("phaseIndex");
        if (!requesterCanAccessCourse(db, req, courseId ? courseId : "", anonymousId)) {
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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

#endif

#if 0  // 顶部进度功能已删除，相关进度接口不再注册。
    // GET /api/task-progress —— 恢复阶段展开任务的三态进度。
    CROW_ROUTE(app, "/api/task-progress")([&db](const crow::request& req) {
        const char* courseId = req.url_params.get("courseId");
        const std::string anonymousId = requestAnonymousId(req);
        const char* goal = req.url_params.get("goal");
        const char* mode = req.url_params.get("mode");
        const char* phaseIndex = req.url_params.get("phaseIndex");
        if (!requesterCanAccessCourse(db, req, courseId ? courseId : "", anonymousId)) {
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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

#if 0  // 学习微课程页面已删除。
    // POST /api/learn/progress —— 三合一：学习卡 + 断点记录 + 全量重算
    CROW_ROUTE(app, "/api/learn/progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
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
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId ? anonymousId : "")) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
            const auto progress = gangyi::recomputeCourseProgress(db, courseId, anonymousId ? anonymousId : "", "");
            return crow::response(200, nlohmann::json{{"ok", true}, {"progress", progress ? *progress : nlohmann::json::object()}}.dump());
        } catch (...) {
            return crow::response(500, nlohmann::json{{"ok", false}, {"error", "progress load failed"}}.dump());
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
            if (!requesterCanAccessCourse(db, req, courseId ? courseId : "", anonymousId ? anonymousId : "")) return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
            }
            if (!anonymousId.empty()) body["anonymousId"] = anonymousId;
            const bool ok = gangyi::upsertLearningSession(db, body);
            return crow::response(ok ? 200 : 400, nlohmann::json{{"ok", ok}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
        }
    });

#endif

#endif

    // POST /api/phase-expansion —— 阶段展开生成
    CROW_ROUTE(app, "/api/phase-expansion").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
            }
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
            std::vector<gangyi::SearchResource> resources;
            try {
                gangyi::SearchClient search;
                resources = search.search(goal + " " + stage, 8);
            } catch (...) {}
            gangyi::AIClient ai;
            gangyi::PhaseGenerator generator(ai);
            const auto result = generator.generate(goal, mode, phaseIndex, stage, topics, resources);
            if (!result) {
                return crow::response(200, nlohmann::json{{"ok", false},
                    {"message", "阶段内容暂未生成完成，请稍后重试。"}}.dump());
            }
            nlohmann::json resourceItems = nlohmann::json::array();
            for (const auto& resource : resources) {
                resourceItems.push_back({{"title", resource.title}, {"description", resource.description},
                    {"url", resource.url}, {"source", resource.source}, {"type", resource.type},
                    {"difficulty", resource.difficulty}, {"free", resource.free}});
            }
            return crow::response(200, nlohmann::json{{"ok", true}, {"phase", *result}, {"resources", resourceItems}}.dump());
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

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

    // /learn 微课程页：课程页和阶段页仍会生成此链接，必须保持与学习 API 配套注册。
    CROW_ROUTE(app, "/learn")([&db](const crow::request& req) {
        const auto value = [&req](const char* name) {
            const char* item = req.url_params.get(name);
            return std::string(item ? item : "");
        };
        const std::string anonymousId = requestAnonymousId(req);
        crow::response response(gangyi::renderLearnPage(
            value("courseId"), value("goal"), value("mode").empty() ? "deep" : value("mode"),
            value("phaseIndex"), value("phaseName"), value("topicIndex"), value("topic"),
            anonymousId, value("regenerate"), value("forceLearn"), value("retry")));
        response.set_header("Content-Type", "text/html; charset=utf-8");
        return response;
    });

    // GET /api/learn 微课程内容：从已生成课程快照构造可执行的单节学习内容。
    CROW_ROUTE(app, "/api/learn")([&db](const crow::request& req) {
        try {
            const auto value = [&req](const char* name) {
                const char* item = req.url_params.get(name);
                return std::string(item ? item : "");
            };
            const std::string courseId = value("courseId");
            const std::string anonymousId = requestAnonymousId(req);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                return crow::response(404, nlohmann::json{{"error", "course not found"}}.dump());
            }
            std::string goal = value("goal");
            std::string mode = value("mode");
            if (mode != "lite") mode = "deep";
            nlohmann::json plan = nlohmann::json::object();
            if (!courseId.empty()) {
                if (const auto found = gangyi::getCourseWithSnapshot(db, courseId)) {
                    plan = found->payload;
                    if (goal.empty()) goal = found->course.goal;
                    if (value("mode").empty()) mode = found->course.mode == "lite" ? "lite" : "deep";
                }
            }
            if (goal.empty()) goal = "你的学习目标";
            int phaseIndex = 0;
            int topicIndex = 0;
            try { if (!value("phaseIndex").empty()) phaseIndex = std::max(0, std::stoi(value("phaseIndex"))); } catch (...) {}
            try { if (!value("topicIndex").empty()) topicIndex = std::max(0, std::stoi(value("topicIndex"))); } catch (...) {}

            const auto roadmap = plan.value("roadmap", nlohmann::json::array());
            nlohmann::json stage = nlohmann::json::object();
            if (roadmap.is_array() && !roadmap.empty()) {
                phaseIndex = std::min(phaseIndex, static_cast<int>(roadmap.size()) - 1);
                stage = roadmap[phaseIndex];
            }
            const std::string phaseName = value("phaseName").empty()
                ? stage.value("name", "当前阶段") : value("phaseName");
            auto topics = nlohmann::json::array();
            if (stage.contains("topics") && stage["topics"].is_array()) {
                for (const auto& item : stage["topics"]) if (item.is_string()) topics.push_back(item);
            }
            const std::string requestedTopic = value("topic");
            const std::string searchTopic = requestedTopic.empty()
                ? (topics.empty() ? goal : topics[std::min(topicIndex, static_cast<int>(topics.size()) - 1)].get<std::string>())
                : requestedTopic;
            // 只把公开类别发送给搜索服务，不外传用户原始目标、阶段名或主题。
            std::string publicSearchTopic = "公开学习资料 教程 练习";
            const std::string categoryText = goal + " " + searchTopic;
            if (categoryText.find("数学") != std::string::npos || categoryText.find("函数") != std::string::npos || categoryText.find("几何") != std::string::npos) publicSearchTopic += " 数学";
            else if (categoryText.find("英语") != std::string::npos || categoryText.find("语言") != std::string::npos || categoryText.find("词汇") != std::string::npos) publicSearchTopic += " 英语";
            else if (categoryText.find("编程") != std::string::npos || categoryText.find("代码") != std::string::npos || categoryText.find("Python") != std::string::npos) publicSearchTopic += " 编程";
            else if (categoryText.find("人工智能") != std::string::npos || categoryText.find("AI") != std::string::npos || categoryText.find("模型") != std::string::npos) publicSearchTopic += " 人工智能";
            std::vector<gangyi::SearchResource> liveResources;
            std::string liveResourceProvider;
            try {
                gangyi::SearchClient search;
                liveResources = search.search(publicSearchTopic, 8);
                liveResourceProvider = search.lastProvider();
            } catch (...) {}
            const auto resourcesToJson = [](const std::vector<gangyi::SearchResource>& resources) {
                nlohmann::json output = nlohmann::json::array();
                for (const auto& resource : resources) {
                    output.push_back({
                        {"title", resource.title}, {"source", resource.source}, {"url", resource.url},
                        {"type", resource.type}, {"description", resource.description},
                        {"difficulty", resource.difficulty}, {"language", resource.language},
                        {"free", resource.free}, {"reason", resource.reason}
                    });
                }
                return output;
            };
            nlohmann::json rawSteps = stage.value("steps", nlohmann::json::array());
            nlohmann::json lessonSteps = nlohmann::json::array();
            if (rawSteps.is_array()) {
                for (const auto& item : rawSteps) {
                    if (!item.is_object()) continue;
                    lessonSteps.push_back({
                        {"title", item.value("title", "理解本节核心概念")},
                        {"explanation", item.value("explanation", "结合课程目标理解概念、条件和解题方法。")},
                        {"example", item.value("example", "选择一道对应题目或一段教材材料进行分析。")},
                        {"action", item.value("action", "写出关键条件，并完成一份具体练习。")},
                        {"check", item.value("check", "能够用自己的话复述方法，并说明适用条件。")}
                    });
                }
            }
            if (lessonSteps.empty()) {
                lessonSteps = nlohmann::json::array({
                    {{"title", "理解" + searchTopic}, {"explanation", "明确“" + searchTopic + "”的定义、条件和解决的问题。"}, {"example", "从教材或练习中找一个“" + searchTopic + "”的例子。"}, {"action", "写出定义、两个关键条件和一个应用场景。"}, {"check", "不看资料能准确解释核心概念。"}},
                    {{"title", "练习" + searchTopic}, {"explanation", "按已知信息、方法选择、结论检验三个步骤完成练习。"}, {"example", "圈出题目中的关键条件，再选择对应方法。"}, {"action", "完成一道题并记录每一步依据。"}, {"check", "每一步都有明确依据，结论符合条件。"}},
                    {{"title", "复盘" + searchTopic}, {"explanation", "整理本节的易错点，形成下一次可直接使用的检查清单。"}, {"example", "对比自己的首次思路与标准解法。"}, {"action", "记录至少两条错误原因和改进办法。"}, {"check", "能说明错误为什么发生以及如何避免。"}}
                });
            }
            // 课程快照通常只有 3 步，这里补齐为完整的五步学习闭环。
            while (lessonSteps.size() < 5) {
                const size_t index = lessonSteps.size();
                if (index == 3) {
                    lessonSteps.push_back({
                        {"title", "迁移应用" + searchTopic},
                        {"explanation", "把本节方法放到一个稍有变化的新情境中，确认你掌握的是方法而不是原题答案。"},
                        {"example", "换一个数据、材料或应用场景，重新完成同一类分析。"},
                        {"action", "写出新情境与原题的相同点、不同点和调整后的做法。"},
                        {"check", "能够说明方法为什么仍然适用，或指出需要更换的方法。"}
                    });
                } else {
                    lessonSteps.push_back({
                        {"title", "自测与总结" + searchTopic},
                        {"explanation", "用一句话总结本节结论，再用一个反例检验结论的边界。"},
                        {"example", "找一个容易误用本方法的反例，说明它为什么不满足条件。"},
                        {"action", "完成‘结论—条件—反例’三行总结。"},
                        {"check", "能说清结论、条件和边界，而不是只背结论。"}
                    });
                }
            }
            topicIndex = std::min(topicIndex, static_cast<int>(lessonSteps.size()) - 1);
            const auto selectedStep = lessonSteps[std::max(0, topicIndex)];
            const std::string topicTitle = requestedTopic.empty()
                ? selectedStep.value("title", phaseName) : requestedTopic;

            nlohmann::json examples = nlohmann::json::array();
            for (const auto& step : lessonSteps) {
                if (step.value("example", "").empty()) continue;
                examples.push_back({{"title", step.value("title", "示例")}, {"content", step.value("example", "")}, {"solution", step.value("check", "")}});
            }
            while (examples.size() < 5) {
                examples.push_back({
                    {"title", "资料对照示例" + std::to_string(examples.size() + 1)},
                    {"content", "打开下方真实参考资料，找出其中一个与“" + searchTopic + "”相关的定义、步骤或案例，并用自己的话复述。"},
                    {"solution", "复述时同时写出资料来源、适用条件和一个你自己的例子。"}
                });
            }
            nlohmann::json practice = nlohmann::json::array();
            if (stage.contains("tasks") && stage["tasks"].is_array()) {
                for (const auto& task : stage["tasks"]) {
                    if (task.is_string()) practice.push_back({{"title", "阶段任务"}, {"task", task.get<std::string>()}, {"check", "完成后记录过程和结果。"}});
                    else if (task.is_object()) practice.push_back({{"title", task.value("title", "阶段任务")}, {"task", task.value("description", "完成本阶段练习。")}, {"check", task.value("output", "形成可检查的学习产出。")}});
                }
            }
            if (practice.empty()) practice.push_back({{"title", "完成本节练习"}, {"task", "完成一道与“" + topicTitle + "”相关的练习并记录过程。"}, {"check", "能够复查步骤并说明结论依据。"}});
            while (practice.size() < 4) {
                const size_t index = practice.size();
                practice.push_back({
                    {"title", index == 1 ? "错题复盘" : index == 2 ? "资料提炼" : "迁移练习"},
                    {"task", index == 1 ? "回看一次错误或卡住的过程，标出遗漏的条件和下一次的检查动作。" :
                        index == 2 ? "从真实参考资料中摘录一个关键观点，注明来源并写出你的理解。" :
                        "改变题目中的一个条件或应用场景，重新完成分析并解释调整原因。"},
                    {"check", "过程可复查，结论有依据，并能说明与本节主题的关系。"}
                });
            }
            nlohmann::json quiz = nlohmann::json::array({
                {{"question", "本节学习的第一步是什么？"}, {"options", {"明确概念与适用条件", "直接背答案", "跳过练习"}}, {"answerIndex", 0}, {"explanation", "先明确概念和条件，后续练习才有依据。"}},
                {{"question", "完成练习后还需要做什么？"}, {"options", {"检查过程并记录错误", "不看过程只看分数", "直接进入下一节"}}, {"answerIndex", 0}, {"explanation", "复盘过程能发现遗漏条件和错误方法。"}},
                {{"question", "如何判断方法是否适用于新情境？"}, {"options", {"核对关键条件是否满足", "只看题目长短", "直接套用原答案"}}, {"answerIndex", 0}, {"explanation", "先核对条件，再决定是否沿用方法。"}},
                {{"question", "使用真实参考资料时最重要的动作是什么？"}, {"options", {"记录来源并用自己的话复述", "只收藏链接", "复制整段文字"}}, {"answerIndex", 0}, {"explanation", "注明来源并复述，才能把资料转化为自己的理解。"}}
            });
            nlohmann::json checkpoint = nlohmann::json::array();
            checkpoint.push_back(stage.value("checkpoint", "能解释核心概念并完成一份练习。"));
            checkpoint.push_back(stage.value("output", "形成一份可检查的阶段学习产出。"));
            nlohmann::json mistakes = stage.value("commonMistakes", nlohmann::json::array());
            if (!mistakes.is_array() || mistakes.empty()) mistakes = nlohmann::json::array({"只背结论，不核对适用条件", "只写答案，不记录推理过程"});
            nlohmann::json references = resourcesToJson(liveResources);
            if (references.empty()) {
                const auto resources = plan.value("resources", nlohmann::json::array());
                if (resources.is_array()) for (const auto& item : resources) if (item.is_object()) references.push_back({
                    {"title", item.value("name", item.value("title", "参考资料"))}, {"source", item.value("type", "课程资料")},
                    {"url", item.value("href", item.value("url", ""))}, {"type", item.value("type", "参考资料")},
                    {"description", item.value("description", "课程计划中的参考资料")}, {"difficulty", item.value("difficulty", "入门")}});
            }
            return crow::response(200, nlohmann::json{
                {"ok", true}, {"title", topicTitle + "·微课程"},
                {"summary", stage.value("description", stage.value("goal", "围绕本节目标完成理解、练习和复盘。"))},
                {"goal", goal}, {"mode", mode}, {"phaseName", phaseName}, {"topicTitle", topicTitle},
                {"keyConcepts", topics}, {"lessonSteps", lessonSteps}, {"examples", examples},
                {"practice", practice}, {"quiz", quiz}, {"checkpoint", checkpoint},
                {"commonMistakes", mistakes},
                {"resourceSummary", liveResources.empty() ? "暂无联网资料，先完成本节学习内容。" : "已补充 Bocha 联网真实学习资料"},
                {"resourceProvider", liveResourceProvider}, {"references", references}
            }.dump());
        } catch (const std::exception& error) {
            return crow::response(500, nlohmann::json{{"ok", false}, {"error", error.what()}}.dump());
        }
    });

    // 学习页进度接口：页面已经恢复，步骤和完成状态也必须能稳定保存。
    CROW_ROUTE(app, "/api/learning-step-progress")([&db](const crow::request& req) {
        const std::string courseId = req.url_params.get("courseId") ? req.url_params.get("courseId") : "";
        const std::string anonymousId = requestAnonymousId(req);
        if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
            return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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
            item.status = body.value("status", "unset");
            const bool saved = old ? db.update(item) : db.insert(item);
            return crow::response(saved ? 200 : 400, nlohmann::json{{"ok", saved}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
        }
    });

    CROW_ROUTE(app, "/api/learn/progress").methods(crow::HTTPMethod::POST)([&db](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            const std::string courseId = body.value("courseId", "");
            const std::string anonymousId = requestAnonymousId(req, &body);
            if (!requesterCanAccessCourse(db, req, courseId, anonymousId)) {
                return crow::response(404, nlohmann::json{{"ok", false}, {"error", "course not found"}}.dump());
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
            item.status = body.value("status", "completed");
            const bool saved = old ? db.update(item) : db.insert(item);
            return crow::response(saved ? 200 : 400, nlohmann::json{{"ok", saved}}.dump());
        } catch (...) {
            return crow::response(400, nlohmann::json{{"ok", false}, {"error", "INVALID_INPUT"}}.dump());
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
                // 质量重试最多两次，避免多层重试叠加后超过前端等待上限。
                for (int attempt = 0; attempt < 2 && !qualityValid; ++attempt) {
                    std::cerr << "[generate-plan] attempt=" << (attempt + 1)
                              << " mode=" << mode << std::endl;
                    try {
                        const auto plan = generator.generate(goal, mode, feedback);
                        const auto raw = planToJson(plan);
                        adapted = gangyi::adaptGeneratedPlan(raw, mode);
                        gate = gangyi::validateCourseContent(adapted, goal, mode,
                            adapted.value("title", goal));
                        qualityValid = gate.valid;
                        if (!qualityValid) {
                            feedback = qualityFeedback(gate);
                            std::cerr << "[generate-plan] quality rejected attempt="
                                      << (attempt + 1) << " feedback=" << feedback << std::endl;
                        }
                    } catch (const gangyi::AIClientError& error) {
                        std::cerr << "[generate-plan] AI error attempt=" << (attempt + 1)
                                  << " type=" << error.errorType << " message=" << error.what() << std::endl;
                        feedback = "上一次模型输出无法使用：" + std::string(error.what()) +
                            "。请重新输出完整、严格符合字段结构的 JSON。";
                    } catch (const std::exception& error) {
                        std::cerr << "[generate-plan] error attempt=" << (attempt + 1)
                                  << " message=" << error.what() << std::endl;
                        feedback = "上一次生成结果解析失败：" + std::string(error.what()) +
                            "。请重新输出完整 JSON，不要输出解释文字。";
                    }
                }
                if (!qualityValid) {
                    std::cerr << "[generate-plan] using deterministic fallback after quality retries" << std::endl;
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
            std::cerr << "[generate-plan] unhandled AI error type=" << e.errorType
                      << " message=" << e.what() << std::endl;
            // 错误→HTTP 映射（对齐 docs/ai-spec.md）：missing_config/auth_error→503，timeout→504，其余→502
            int code = 502;
            if (e.errorType == "missing_config" || e.errorType == "auth_error") code = 503;
            else if (e.errorType == "timeout") code = 504;
            return crow::response(code, nlohmann::json{{"error", e.what()}, {"type", e.errorType}}.dump());
        } catch (const std::exception& e) {
            std::cerr << "[generate-plan] unhandled error message=" << e.what() << std::endl;
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

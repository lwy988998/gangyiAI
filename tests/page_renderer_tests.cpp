#include "page_renderer.hpp"

#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "失败: " << message << '\n';
        ++failures;
    }
}

}  // namespace

int main() {
    const std::string ask = gangyi::renderAskPage();
    expect(ask.find("当前配置的 AI 模型") != std::string::npos, "问答页应使用服务商中立说明");
    expect(ask.find("DeepSeek V4 Flash") == std::string::npos, "问答页不得写死模型名称");

    nlohmann::json plan;
    plan["roadmap"] = nlohmann::json::array();
    plan["roadmap"].push_back({{"name", "基础阶段"}, {"goal", "理解函数单调性"},
        {"tasks", nlohmann::json::array({"完成例题"})}, {"duration", "1 周"}});
    plan["courseStructure"] = nlohmann::json::array();
    plan["courseStructure"].push_back({{"stage", "基础阶段"},
        {"topics", nlohmann::json::array({"函数单调性"})}});
    const std::string phase = gangyi::renderPhasePage("course-1", "anonymous-1", "掌握函数单调性",
        "deep", "1", "基础阶段", plan, nlohmann::json::object());
    expect(phase.find("深度课程规划") != std::string::npos, "阶段页应显示正确的深度模式名称");
    expect(phase.find("深度 钢一定制AI 规划") == std::string::npos, "阶段页不得出现错误模式名称");
    expect(phase.find("阶段验收清单") != std::string::npos, "阶段展开应使用中文验收清单");
    expect(phase.find("问问钢一定制AI") != std::string::npos, "阶段页问答入口文案应自然完整");
    expect(phase.find("阶段验收 checklist") == std::string::npos, "阶段展开不得混用 checklist");
    expect(phase.find("Step '+") == std::string::npos, "阶段展开不得显示英文 Step");
    expect(phase.find("/ )HTML") == std::string::npos, "阶段进度不得包含多余空格");

    const std::string home = gangyi::renderHomePage();
    expect(home.find("school-logo.png") != std::string::npos, "首页应加载校徽");
    expect(home.find("class=\"home-page ") != std::string::npos, "首页应包含校园背景容器");
    expect(home.find("home-goal-surface") != std::string::npos, "首页应包含学习目标对话框");

    const std::string learn = gangyi::renderLearnPage("course-1", "掌握函数单调性", "deep",
        "1", "基础阶段", "1", "函数单调性", "anonymous-1", "", "", "");
    expect(learn.find("order=['overview','steps','examples','practice','quiz','assessment']") != std::string::npos,
        "微课堂必须校验六个 AI 板块全部存在");
    expect(learn.find("&block=all") != std::string::npos,
        "进入微课堂必须请求服务端生成全部 AI 板块");
    expect(learn.find("data-retry-lesson") != std::string::npos,
        "失败后应提供整节课程重试入口");
    expect(learn.find("data-retry-block") == std::string::npos,
        "页面不得要求用户逐板块重试");
    expect(learn.find("$('learn-content').classList.add('hidden')") != std::string::npos,
        "全部 AI 板块完成前必须隐藏课程正文");
    expect(learn.find("params.delete('regenerate')") != std::string::npos,
        "整课重新生成成功后必须清除一次性参数并恢复缓存复用");

    const std::string planPage = gangyi::renderPlanPage("掌握函数单调性", "deep", "", "anonymous-1");
    expect(planPage.find("250000") != std::string::npos,
        "主线课程三次 AI 尝试应有足够的前端等待时间");

    nlohmann::json courses = {
        {"anonymousId", "anonymous-1"}, {"authenticated", false},
        {"stats", {{"total", 1}}},
        {"courses", nlohmann::json::array({{
            {"courseId", "course-1"}, {"title", "函数课程"}, {"goal", "掌握函数"},
            {"createdAt", "2026-09-08T05:00:00Z"}
        }})}
    };
    const std::string myCourses = gangyi::renderMyCoursesPage(courses);
    expect(myCourses.find("data-course-id=\"course-1\"") != std::string::npos,
        "课程删除按钮应包含完整闭合的课程编号属性");
    return failures == 0 ? 0 : 1;
}

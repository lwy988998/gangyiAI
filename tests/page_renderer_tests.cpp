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
    expect(home.find("/learning-flow.js") < home.find("/classroom.js") && home.find("/learning-flow.css") != std::string::npos,
        "全流程 AI 组件必须先于课堂脚本加载并随页面提供样式");
    expect(home.find("startup-playing") == std::string::npos, "普通首页不播放开机动画");
    const std::string startup = gangyi::renderStartupPage();
    expect(startup.find("id=\"startup-overlay\"") != std::string::npos, "启动页包含动画覆盖层");
    expect(startup.find("home-goal-surface") != std::string::npos, "启动页使用真实首页内容");
    expect(startup.find("/startup.js") != std::string::npos, "启动页加载动画脚本");
    expect(startup.find("data-startup-variant") == std::string::npos, "启动页不再暴露旧动画变体");
    expect(startup.find("location.replace('/')") == std::string::npos, "启动页不得硬切换到首页");
    expect(home.find("school-logo.png") != std::string::npos, "首页应加载校徽");
    expect(home.find("class=\"home-page ") != std::string::npos, "首页应包含校园背景容器");
    expect(home.find("home-goal-surface") != std::string::npos, "首页应包含学习目标对话框");
    expect(home.find("data-profile-radar") != std::string::npos && home.find("/profile-radar.js") != std::string::npos,
           "首页应使用共享六维画像组件");
    expect(home.find("home-ticker") != std::string::npos, "首页应保留动态功能导览");
    expect(home.find("data-reveal") != std::string::npos, "首页区块应支持滚动入场");

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
    expect(learn.find("setInterval(pollProgress,2500)") != std::string::npos,
        "全课生成期间必须显示已持久化的真实 AI 板块进度");
    expect(learn.find("data.generations?.[block]?.source==='ai'") != std::string::npos,
        "进度只能统计具有真实 AI 来源元数据的板块");
    expect(learn.find("item.solution") == std::string::npos && learn.find("item.check") == std::string::npos &&
        learn.find("id=\"quiz-submit\"") == std::string::npos,
        "题目作答前不得渲染答案、自检结果或旧批量测验入口");
    expect(learn.find("mountQuestionKind") != std::string::npos && learn.find("lessonTaskId") != std::string::npos,
        "全部课堂题目应转统一对话，并按备课任务读取已保存内容");
    expect(learn.find("让 AI 准备下一课") != std::string::npos && learn.find("继续下一节 →") == std::string::npos,
        "每个课堂必须提供主动备课入口，不直接跳入另一课");
    const std::string preparation = gangyi::renderNextLessonPage();
    expect(preparation.find("id=\"next-lesson-page\"") != std::string::npos &&
        preparation.find("id=\"next-lesson-stop\"") != std::string::npos &&
        preparation.find("id=\"next-lesson-retry\"") != std::string::npos,
        "备课页面必须能恢复、停止和重试任务");

    const std::string planPage = gangyi::renderPlanPage("掌握函数单调性", "deep", "", "anonymous-1");
    expect(planPage.find("&phaseIndex=0&topicIndex=0") == std::string::npos &&
        planPage.find("&phaseIndex=1&topicIndex=1") != std::string::npos,
        "计划页首课入口必须使用一基课时序号");
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
    expect(myCourses.find("API 接口") != std::string::npos,
        "我的课程页应显示 API 接口入口");
    expect(myCourses.find("/user-center.js") != std::string::npos,
        "用户中心应加载独立脚本");
    expect(myCourses.find("id=\"uc-overview\"") != std::string::npos &&
        myCourses.find("id=\"uc-courses\"") != std::string::npos &&
        myCourses.find("id=\"uc-time\"") != std::string::npos &&
        myCourses.find("id=\"uc-profile\"") != std::string::npos &&
        myCourses.find("id=\"uc-settings\"") != std::string::npos,
        "用户中心应包含五个区块");
    expect(myCourses.find("id=\"uc-availability-grid\"") != std::string::npos,
        "每周学习时间应搬到用户中心");
    expect(myCourses.find("待复习清单") != std::string::npos,
        "用户中心应展示待复习清单");
    return failures == 0 ? 0 : 1;
}

#include "page_renderer.hpp"
#include <iostream>
#include <string>
namespace {
int failures = 0;
void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "失败: " << message << '\n'; ++failures; }
}
}
int main() {
    const auto ask = gangyi::renderAskPage();
    expect(ask.find("当前配置的 AI 模型") != std::string::npos && ask.find("DeepSeek V4 Flash") == std::string::npos,
        "问答页应显示配置模型，不写死服务商");
    const auto home = gangyi::renderHomePage();
    expect(home.find("gy-theme-lab") != std::string::npos && home.find("school-logo.png") != std::string::npos,
        "正式页面使用用户选定的 C 方向与原校徽");
    expect(home.find("/learning-flow.js") < home.find("/classroom.js") && home.find("/app-layout.js") != std::string::npos,
        "统一课堂与目录资源必须加载");
    expect(home.find("data-profile-radar") != std::string::npos && home.find("home-goal-surface") != std::string::npos,
        "首页保留目标输入与真实画像组件");
    expect(home.find("home-ticker") == std::string::npos && home.find("feature-card") == std::string::npos,
        "首页清除无关滚动展示");
    expect(home.find("startup-playing") == std::string::npos, "普通首页不播放开机动画");
    const auto startup = gangyi::renderStartupPage();
    expect(startup.find("id=\"startup-overlay\"") != std::string::npos && startup.find("/startup.js") != std::string::npos,
        "启动页保留独立过渡与真实首页");
    expect(startup.find("location.replace('/')") == std::string::npos, "启动完成不能强制刷新首页");
    nlohmann::json plan = {{"courseStructure", {{{"stage", "概念阶段"}, {"goal", "比较变化"}, {"why", "已有 AI 排列说明"},
        {"topics", {{{"title", "化合价比较<测试>"}, {"id", "stable-topic"}, {"legacyPhaseIndex", 3}, {"legacyTopicIndex", 4}}}}}}}};
    const auto phase = gangyi::renderPhasePage("course-1", "anonymous-1", "学习目标", "deep", "1", "概念阶段", plan, nlohmann::json::object());
    expect(phase.find("化合价比较&lt;测试&gt;") != std::string::npos && phase.find("化合价比较<测试>") == std::string::npos,
        "阶段页按文本转义已有内容");
    expect(phase.find("legacyPhaseIndex") == std::string::npos && phase.find("phaseIndex=3") != std::string::npos && phase.find("topicIndex=4") != std::string::npos,
        "重排后显示阶段保持原知识点入口身份");
    expect(phase.find("已有 AI 排列说明") != std::string::npos && phase.find("本阶段知识点") != std::string::npos,
        "阶段显示已保存 AI 说明与知识点清单");
    const auto learn = gangyi::renderLearnPage("course-1", "目标", "deep", "1", "概念", "1", "化合价", "anonymous-1", "", "", "");
    for (const auto* view : {"learn", "practice", "summary"}) {
        const auto page = gangyi::renderClassroomPage(view);
        expect(page.find(std::string("data-lesson-view=\"") + view + "\"") != std::string::npos,
            "旧路由必须保留对应内容的定位标识");
        expect(page.find("/agent-classroom.js") != std::string::npos && page.find("id=\"lesson-sections\"") != std::string::npos,
            "三页都绑定同一真实课堂控制器");
        expect(page.find("data-lesson-link=\"learn\"") != std::string::npos && page.find("data-lesson-link=\"practice\"") != std::string::npos && page.find("data-lesson-link=\"summary\"") != std::string::npos,
            "聊天课堂保留三个独立页面入口");
        expect(page.find("id=\"lesson-scroll\"") != std::string::npos && page.find("id=\"lesson-input\"") != std::string::npos &&
            page.find("id=\"lesson-latest\"") != std::string::npos, "课堂使用独立消息滚动区与常驻输入框");
    }
    expect(learn.find("expectedAnswer") == std::string::npos && learn.find("quiz-submit") == std::string::npos,
        "页面骨架不携带答案或旧批量评分入口");
    expect(learn.find("id=\"finish-lesson\"") != std::string::npos && learn.find("id=\"lesson-stop\"") != std::string::npos && learn.find("id=\"lesson-retry\"") != std::string::npos,
        "真实主控完成入口共用常驻的停止和重试操作");
    const auto planPage = gangyi::renderPlanPage("目标", "deep", "", "anonymous-1");
    expect(planPage.find("course-stages") != std::string::npos && planPage.find("course-more") != std::string::npos,
        "课程阶段突出显示，其余完整内容默认折叠");
    const nlohmann::json courses = {{"anonymousId", "anonymous-1"}, {"stats", {{"total", 1}}},
        {"courses", {{{"courseId", "course-1"}, {"title", "函数课程"}, {"goal", "学习函数"}, {"createdAt", "2026-09-08"}}}}};
    const auto myCourses = gangyi::renderMyCoursesPage(courses);
    expect(myCourses.find("data-course-id=\"course-1\"") != std::string::npos && myCourses.find("/user-center.js") != std::string::npos,
        "课程删除与设置继续绑定原控制器");
    expect(myCourses.find("aria-controls=\"courses-panel\"") != std::string::npos && myCourses.find("aria-controls=\"profile-panel\"") != std::string::npos,
        "课程和画像分为两个可访问页签");
    expect(myCourses.find("id=\"uc-time\"") != std::string::npos && myCourses.find("id=\"uc-availability-grid\"") != std::string::npos,
        "时间设置集中在画像页");
    expect(myCourses.find("id=\"open-api-settings\"") != std::string::npos, "原 AI 配置入口继续存在");
    return failures == 0 ? 0 : 1;
}

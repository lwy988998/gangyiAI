// 【逐行批注】 引入头文件 "page_renderer.hpp"，为后续实现提供类型或接口声明。
#include "page_renderer.hpp"
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 int failures，更新当前运行状态。
int failures = 0;
// 【逐行批注】 定义函数 expect，执行该函数声明的职责。
void expect(bool condition, const char* message) {
    // 【逐行批注】 判断条件 (!condition) { std::cerr << "失败: " << message << '\n'; ++failure…，成立时执行对应分支。
    if (!condition) { std::cerr << "失败: " << message << '\n'; ++failures; }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 expectNavigation，执行该函数声明的职责。
void expectNavigation(const std::string& page, const std::string& expectedHref) {
    // 【逐行批注】 计算右侧结果并写入 auto label，更新当前运行状态。
    const auto label = page.find("aria-label=\"主导航\"");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(label != std::string::npos, "页面应提供顶部主导航");。
    expect(label != std::string::npos, "页面应提供顶部主导航");
    // 【逐行批注】 判断条件 (label == std::string::npos) return;，成立时执行对应分支。
    if (label == std::string::npos) return;
    // 【逐行批注】 计算右侧结果并写入 auto end，更新当前运行状态。
    const auto end = page.find("</nav>", label);
    // 【逐行批注】 计算右侧结果并写入 auto navigation，更新当前运行状态。
    const auto navigation = page.substr(label, end - label);
    // 【逐行批注】 计算右侧结果并写入 std::string current，更新当前运行状态。
    const std::string current = "aria-current=\"page\"";
    // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
    const auto first = navigation.find(current);
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(first != std::string::npos && navigation.find(current, first +…。
    expect(first != std::string::npos && navigation.find(current, first + current.size()) == std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："顶部主导航只能标记一个当前页面");。
        "顶部主导航只能标记一个当前页面");
    // 【逐行批注】 计算右侧结果并写入 auto href，更新当前运行状态。
    const auto href = navigation.find("href=\"" + expectedHref + "\"");
    // 【逐行批注】 判断条件 (href == std::string::npos) { expect(false, "顶部主导航应保留预期入口"); ret…，成立时执行对应分支。
    if (href == std::string::npos) { expect(false, "顶部主导航应保留预期入口"); return; }
    // 【逐行批注】 计算右侧结果并写入 auto anchorStart，更新当前运行状态。
    const auto anchorStart = navigation.rfind("<a", href);
    // 【逐行批注】 计算右侧结果并写入 auto anchorEnd，更新当前运行状态。
    const auto anchorEnd = navigation.find('>', href);
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(navigation.substr(anchorStart, anchorEnd - anchorStart).find(c…。
    expect(navigation.substr(anchorStart, anchorEnd - anchorStart).find(current) != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："顶部主导航应高亮页面所属入口");。
        "顶部主导航应高亮页面所属入口");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
    // 【逐行批注】 计算右侧结果并写入 auto ask，更新当前运行状态。
    const auto ask = gangyi::renderAskPage();
    // 【逐行批注】 调用 expectNavigation 完成本行所需操作：expectNavigation(ask, "/ask");。
    expectNavigation(ask, "/ask");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(ask.find("当前配置的 AI 模型") != std::string::npos && ask.find("Deep…。
    expect(ask.find("当前配置的 AI 模型") != std::string::npos && ask.find("DeepSeek V4 Flash") == std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："问答页应显示配置模型，不写死服务商");。
        "问答页应显示配置模型，不写死服务商");
    // 【逐行批注】 计算右侧结果并写入 auto home，更新当前运行状态。
    const auto home = gangyi::renderHomePage();
    // 【逐行批注】 调用 expectNavigation 完成本行所需操作：expectNavigation(home, "/");。
    expectNavigation(home, "/");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(home.find("clarity-app") != std::string::npos && home.find("/c…。
    expect(home.find("clarity-app") != std::string::npos && home.find("/clarity.css") != std::string::npos &&
        // 【逐行批注】 执行当前声明或表达式：home.find("school-logo.png") != std::string::npos && home.find("gy-theme-lab") == st…。
        home.find("school-logo.png") != std::string::npos && home.find("gy-theme-lab") == std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："正式页面采用用户选定的澄明方向并保留原校徽");。
        "正式页面采用用户选定的澄明方向并保留原校徽");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(home.find("/learning-flow.js") < home.find("/classroom.js") &&…。
    expect(home.find("/learning-flow.js") < home.find("/classroom.js") && home.find("/app-layout.js") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："统一课堂与目录资源必须加载");。
        "统一课堂与目录资源必须加载");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(home.find("/agent-trace.css") != std::string::npos && home.fin…。
    expect(home.find("/agent-trace.css") != std::string::npos && home.find("/agent-trace.js") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："全站页面必须加载统一 AI 执行轨迹");。
        "全站页面必须加载统一 AI 执行轨迹");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(home.find("data-profile-radar") != std::string::npos && home.f…。
    expect(home.find("data-profile-radar") != std::string::npos && home.find("home-goal-surface") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："首页保留目标输入与真实画像组件");。
        "首页保留目标输入与真实画像组件");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(home.find("home-ticker") == std::string::npos && home.find("fe…。
    expect(home.find("home-ticker") == std::string::npos && home.find("feature-card") == std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："首页清除无关滚动展示");。
        "首页清除无关滚动展示");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(home.find("startup-playing") == std::string::npos, "普通首页不播放开机动…。
    expect(home.find("startup-playing") == std::string::npos, "普通首页不播放开机动画");
    // 【逐行批注】 计算右侧结果并写入 auto startup，更新当前运行状态。
    const auto startup = gangyi::renderStartupPage();
    // 【逐行批注】 调用 expectNavigation 完成本行所需操作：expectNavigation(startup, "/");。
    expectNavigation(startup, "/");
    // 【逐行批注】 计算右侧结果并写入 expect(startup.find("id，更新当前运行状态。
    expect(startup.find("id=\"startup-overlay\"") != std::string::npos && startup.find("/startup.js") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："启动页保留独立过渡与真实首页");。
        "启动页保留独立过渡与真实首页");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(startup.find("location.replace('/')") == std::string::npos, "启…。
    expect(startup.find("location.replace('/')") == std::string::npos, "启动完成不能强制刷新首页");
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json plan，更新当前运行状态。
    nlohmann::json plan = {{"courseStructure", {{{"stage", "概念阶段"}, {"goal", "比较变化"}, {"why", "已有 AI 排列说明"},
        // 【逐行批注】 执行当前声明或表达式：{"topics", {{{"title", "化合价比较<测试>"}, {"id", "stable-topic"}, {"legacyPhaseIndex", 3}…。
        {"topics", {{{"title", "化合价比较<测试>"}, {"id", "stable-topic"}, {"legacyPhaseIndex", 3}, {"legacyTopicIndex", 4}}}}}}}};
    // 【逐行批注】 计算右侧结果并写入 auto phase，更新当前运行状态。
    const auto phase = gangyi::renderPhasePage("course-1", "anonymous-1", "学习目标", "deep", "1", "概念阶段", plan, nlohmann::json::object());
    // 【逐行批注】 调用 expectNavigation 完成本行所需操作：expectNavigation(phase, "/current-course");。
    expectNavigation(phase, "/current-course");
    // 【逐行批注】 调用 expectNavigation、renderPhasePage、object 完成本行所需操作：expectNavigation(gangyi::renderPhasePage("course-1", "anonymous-1", "…。
    expectNavigation(gangyi::renderPhasePage("course-1", "anonymous-1", "学习目标", "deep", "2", "", plan, nlohmann::json::object()), "/current-course");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(phase.find("化合价比较&lt;测试&gt;") != std::string::npos && phase.fi…。
    expect(phase.find("化合价比较&lt;测试&gt;") != std::string::npos && phase.find("化合价比较<测试>") == std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："阶段页按文本转义已有内容");。
        "阶段页按文本转义已有内容");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(phase.find("legacyPhaseIndex") == std::string::npos && phase.f…。
    expect(phase.find("legacyPhaseIndex") == std::string::npos && phase.find("phaseIndex=3") != std::string::npos && phase.find("topicIndex=4") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："重排后显示阶段保持原知识点入口身份");。
        "重排后显示阶段保持原知识点入口身份");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(phase.find("已有 AI 排列说明") != std::string::npos && phase.find("本…。
    expect(phase.find("已有 AI 排列说明") != std::string::npos && phase.find("本阶段知识点") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："阶段显示已保存 AI 说明与知识点清单");。
        "阶段显示已保存 AI 说明与知识点清单");
    // 【逐行批注】 计算右侧结果并写入 auto learn，更新当前运行状态。
    const auto learn = gangyi::renderLearnPage("course-1", "目标", "deep", "1", "概念", "1", "化合价", "anonymous-1", "", "", "");
    // 【逐行批注】 按循环规则重复处理：for (const auto* view : {"learn", "practice", "summary"}) {。
    for (const auto* view : {"learn", "practice", "summary"}) {
        // 【逐行批注】 计算右侧结果并写入 auto page，更新当前运行状态。
        const auto page = gangyi::renderClassroomPage(view);
        // 【逐行批注】 调用 expectNavigation 完成本行所需操作：expectNavigation(page, "/current-course");。
        expectNavigation(page, "/current-course");
        // 【逐行批注】 计算右侧结果并写入 expect(page.find(std::string("data-lesson-vi…，更新当前运行状态。
        expect(page.find(std::string("data-lesson-view=\"") + view + "\"") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："旧路由必须保留对应内容的定位标识");。
            "旧路由必须保留对应内容的定位标识");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(page.find("/agent-classroom.js") != std::string::npos && page.…。
        expect(page.find("/agent-classroom.js") != std::string::npos && page.find("id=\"lesson-sections\"") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："三页都绑定同一真实课堂控制器");。
            "三页都绑定同一真实课堂控制器");
        // 【逐行批注】 计算右侧结果并写入 expect(page.find("id，更新当前运行状态。
        expect(page.find("id=\"classroom-agent-trace\"") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："课堂必须就地显示 AI 阶段与恢复状态");。
            "课堂必须就地显示 AI 阶段与恢复状态");
        // 【逐行批注】 计算右侧结果并写入 expect(page.find("data-lesson-link，更新当前运行状态。
        expect(page.find("data-lesson-link=\"learn\"") != std::string::npos && page.find("data-lesson-link=\"practice\"") != std::string::npos && page.find("data-lesson-link=\"summary\"") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："聊天课堂保留三个独立页面入口");。
            "聊天课堂保留三个独立页面入口");
        // 【逐行批注】 计算右侧结果并写入 expect(page.find("id，更新当前运行状态。
        expect(page.find("id=\"lesson-scroll\"") != std::string::npos && page.find("id=\"lesson-input\"") != std::string::npos &&
            // 【逐行批注】 计算右侧结果并写入 page.find("id，更新当前运行状态。
            page.find("id=\"lesson-latest\"") != std::string::npos, "课堂使用独立消息滚动区与常驻输入框");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(learn.find("expectedAnswer") == std::string::npos && learn.fin…。
    expect(learn.find("expectedAnswer") == std::string::npos && learn.find("quiz-submit") == std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："页面骨架不携带答案或旧批量评分入口");。
        "页面骨架不携带答案或旧批量评分入口");
    // 【逐行批注】 计算右侧结果并写入 expect(learn.find("id，更新当前运行状态。
    expect(learn.find("id=\"finish-lesson\"") != std::string::npos && learn.find("id=\"lesson-stop\"") != std::string::npos && learn.find("id=\"lesson-retry\"") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："真实主控完成入口共用常驻的停止和重试操作");。
        "真实主控完成入口共用常驻的停止和重试操作");
    // 【逐行批注】 计算右侧结果并写入 auto planPage，更新当前运行状态。
    const auto planPage = gangyi::renderPlanPage("目标", "deep", "", "anonymous-1");
    // 【逐行批注】 调用 expectNavigation 完成本行所需操作：expectNavigation(planPage, "/current-course");。
    expectNavigation(planPage, "/current-course");
    // 【逐行批注】 调用 expectNavigation、renderPlanPage 完成本行所需操作：expectNavigation(gangyi::renderPlanPage("目标", "lite", "course-1", "an…。
    expectNavigation(gangyi::renderPlanPage("目标", "lite", "course-1", "anonymous-1"), "/current-course");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(planPage.find("course-stages") != std::string::npos && planPag…。
    expect(planPage.find("course-stages") != std::string::npos && planPage.find("course-more") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："课程阶段突出显示，其余完整内容默认折叠");。
        "课程阶段突出显示，其余完整内容默认折叠");
    // agent-trace.js 以 defer 加载，规划页内联脚本在解析期执行；轨迹只能按需创建，
    // 不能在顶层读取 window.GangyiTrace，否则页面永远没有执行轨迹。
    // 【逐行批注】 计算右侧结果并写入 auto traceCreate，更新当前运行状态。
    const auto traceCreate = planPage.find("window.GangyiTrace.create");
    // 【逐行批注】 计算右侧结果并写入 auto tracePaint，更新当前运行状态。
    const auto tracePaint = planPage.find("paintTask(task)");
    // 【逐行批注】 计算右侧结果并写入 expect(planPage.find("id，更新当前运行状态。
    expect(planPage.find("id=\"plan-agent-trace\"") != std::string::npos && tracePaint != std::string::npos &&
        // 【逐行批注】 执行当前声明或表达式：traceCreate != std::string::npos && traceCreate > tracePaint,。
        traceCreate != std::string::npos && traceCreate > tracePaint,
        // 【逐行批注】 执行当前声明或表达式："课程规划必须在拿到任务后按需创建并展示真实执行轨迹");。
        "课程规划必须在拿到任务后按需创建并展示真实执行轨迹");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(planPage.find("agentTrace?.update(task)") == std::string::npos,。
    expect(planPage.find("agentTrace?.update(task)") == std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："规划页不得在 defer 脚本可用前读取全局轨迹组件");。
        "规划页不得在 defer 脚本可用前读取全局轨迹组件");
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json courses，更新当前运行状态。
    const nlohmann::json courses = {{"anonymousId", "anonymous-1"}, {"stats", {{"total", 1}}},
        // 【逐行批注】 执行当前声明或表达式：{"courses", {{{"courseId", "course-1"}, {"title", "函数课程"}, {"goal", "学习函数"}, {"creat…。
        {"courses", {{{"courseId", "course-1"}, {"title", "函数课程"}, {"goal", "学习函数"}, {"createdAt", "2026-09-08"}}}}};
    // 【逐行批注】 计算右侧结果并写入 auto myCourses，更新当前运行状态。
    const auto myCourses = gangyi::renderMyCoursesPage(courses);
    // 【逐行批注】 计算右侧结果并写入 auto emptyCourse，更新当前运行状态。
    const auto emptyCourse = gangyi::renderCurrentCourseEmptyPage();
    // 【逐行批注】 调用 expectNavigation 完成本行所需操作：expectNavigation(myCourses, "/my-courses");。
    expectNavigation(myCourses, "/my-courses");
    // 【逐行批注】 调用 expectNavigation 完成本行所需操作：expectNavigation(emptyCourse, "/current-course");。
    expectNavigation(emptyCourse, "/current-course");
    // 【逐行批注】 调用 expectNavigation、renderCurrentCourseEmptyPage 完成本行所需操作：expectNavigation(gangyi::renderCurrentCourseEmptyPage(true), "/curren…。
    expectNavigation(gangyi::renderCurrentCourseEmptyPage(true), "/current-course");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(emptyCourse.find("当前没有课程") != std::string::npos && emptyCourse…。
    expect(emptyCourse.find("当前没有课程") != std::string::npos && emptyCourse.find("href=\"/current-course\"") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："空档案的当前课程入口可点击，进入独立空状态页面");。
        "空档案的当前课程入口可点击，进入独立空状态页面");
    // 【逐行批注】 计算右侧结果并写入 expect(myCourses.find("data-course-id，更新当前运行状态。
    expect(myCourses.find("data-course-id=\"course-1\"") != std::string::npos && myCourses.find("/user-center.js") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："课程删除与设置继续绑定原控制器");。
        "课程删除与设置继续绑定原控制器");
    // 【逐行批注】 计算右侧结果并写入 expect(myCourses.find("aria-controls，更新当前运行状态。
    expect(myCourses.find("aria-controls=\"courses-panel\"") != std::string::npos && myCourses.find("href=\"/my-ai/reports\"") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："课程管理提供独立学习报告入口");。
        "课程管理提供独立学习报告入口");
    // 【逐行批注】 计算右侧结果并写入 expect(myCourses.find("href，更新当前运行状态。
    expect(myCourses.find("href=\"/my-ai/plan\"") != std::string::npos && myCourses.find("id=\"uc-availability-grid\"") == std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："时间设置迁入独立学习计划页");。
        "时间设置迁入独立学习计划页");
    // 【逐行批注】 计算右侧结果并写入 expect(myCourses.find("id，更新当前运行状态。
    expect(myCourses.find("id=\"open-api-settings\"") != std::string::npos, "原 AI 配置入口继续存在");
    // 【逐行批注】 计算右侧结果并写入 legacyCourses，更新当前运行状态。
    auto legacyCourses = courses;
    // 【逐行批注】 计算右侧结果并写入 legacyCourses["courses"][0]["mode"]，更新当前运行状态。
    legacyCourses["courses"][0]["mode"] = "deep";
    // 【逐行批注】 调用 expect、renderMyCoursesPage 完成本行所需操作：expect(gangyi::renderMyCoursesPage(legacyCourses).find("快速规划 · 完成") !…。
    expect(gangyi::renderMyCoursesPage(legacyCourses).find("快速规划 · 完成") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："历史deep参数课程按原教学方式显示为快速规划");。
        "历史deep参数课程按原教学方式显示为快速规划");
    // 【逐行批注】 计算右侧结果并写入 legacyCourses["courses"][0]["pedagogy"]，更新当前运行状态。
    legacyCourses["courses"][0]["pedagogy"] = "deep-v1";
    // 【逐行批注】 调用 expect、renderMyCoursesPage 完成本行所需操作：expect(gangyi::renderMyCoursesPage(legacyCourses).find("深度课程 · 完成") !…。
    expect(gangyi::renderMyCoursesPage(legacyCourses).find("深度课程 · 完成") != std::string::npos,
        // 【逐行批注】 执行当前声明或表达式："新深度课程依据明确呈现协议显示模式");。
        "新深度课程依据明确呈现协议显示模式");
    // 【逐行批注】 结束当前函数并返回结果：return failures == 0 ? 0 : 1;。
    return failures == 0 ? 0 : 1;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 引入头文件 "my_ai.hpp"，为后续实现提供类型或接口声明。
#include "my_ai.hpp"
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>
// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 定义函数 renderMyAIPage，执行该函数声明的职责。
std::string renderMyAIPage(const std::string& page) {
    // 【逐行批注】 执行当前声明或表达式：const std::map<std::string,std::string> titles={{"overview","个人概览"},{"chat","AI管家"},…。
    const std::map<std::string,std::string> titles={{"overview","个人概览"},{"chat","AI管家"},{"library","资料与知识库"},{"reports","学习报告"},{"plan","学习计划"},{"preferences","教学偏好"},{"memories","AI记忆"},{"tasks","AI任务"},{"usage","使用情况"},{"profile","个人资料"},{"settings","软件设置"},{"data","数据管理"}};
    // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(!titles.count(page))throw std::invalid_argument("页面不存在");。
    if(!titles.count(page))throw std::invalid_argument("页面不存在");
    // 【逐行批注】 执行当前声明或表达式：std::string menu;for(const auto& key:{"overview","chat","library","reports","plan","…。
    std::string menu;for(const auto& key:{"overview","chat","library","reports","plan","preferences","memories","tasks","usage","profile","settings","data"}) {
        // 【逐行批注】 计算右侧结果并写入 auto href，更新当前运行状态。
        const auto href=std::string("/my-ai")+(std::string(key)=="overview"?"":"/"+std::string(key));
        // 【逐行批注】 计算右侧结果并写入 menu+，更新当前运行状态。
        menu+="<a href=\""+href+"\""+(page==key?" class=\"is-active\" aria-current=\"page\"":"")+">"+titles.at(key)+"</a>";
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return R"HTML(<!DOCTYPE html><html lang="zh-CN"><head><meta charset="…。
    return R"HTML(<!DOCTYPE html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>)HTML"+titles.at(page)+R"HTML( - 我的AI · 钢一定制AI</title><link rel="icon" href="/school-logo.png"><link rel="stylesheet" href="/dark.css"><link rel="stylesheet" href="/app-layout.css"><link rel="stylesheet" href="/profile-radar.css"><link rel="stylesheet" href="/agent-trace.css"><link rel="stylesheet" href="/vendor/katex/katex.min.css"><link rel="stylesheet" href="/my-ai.css"><link rel="stylesheet" href="/library.css"><script defer src="/vendor/katex/katex.min.js"></script><script defer src="/chat-render.js"></script><script defer src="/profile-radar.js"></script><script defer src="/vendor/echarts/echarts.min.js"></script><script defer src="/learning-agent.js"></script><script defer src="/agent-trace.js"></script><script defer src="/agent-shell.js"></script><script defer src="/my-ai.js"></script><script defer src="/library.js"></script><link rel="stylesheet" href="/clarity.css"><link rel="stylesheet" href="/workspace.css"><script src="/workspace.js"></script><script defer src="/clarity.js"></script></head><body class="clarity-app my-ai-body" data-my-ai-page=")HTML"+page+R"HTML("><header class="my-ai-top"><a class="my-ai-brand" href="/"><img src="/school-logo.png" alt="柳州市钢一中学校徽"><span>钢一<b>定制AI</b></span></a><nav aria-label="主导航"><a href="/">首页</a><a href="/my-courses">我的课程</a><a href="/current-course">当前课程</a><a href="/ask">AI导师</a><a href="/my-ai" aria-current="page">我的AI</a></nav></header><div class="my-ai-shell"><aside class="my-ai-sidebar"><a class="my-ai-sidebar-title" href="/my-ai">我的AI</a><p>本机个人档案</p><nav aria-label="我的AI功能目录">)HTML"+menu+R"HTML(</nav><a class="my-ai-back" href="/">← 返回首页</a></aside><main class="my-ai-main"><header class="my-ai-heading"><div><h1>)HTML"+titles.at(page)+R"HTML(</h1></div><a id="return-chat" hidden>返回刚才的聊天 →</a></header><p id="my-ai-status" role="status" aria-live="polite">正在读取本机档案…</p><section id="my-ai-content" aria-label=")HTML"+titles.at(page)+R"HTML("></section></main></div></body></html>)HTML";
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

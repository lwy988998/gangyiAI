#include "my_ai.hpp"
#include <map>
#include <stdexcept>
namespace gangyi {
std::string renderMyAIPage(const std::string& page) {
    const std::map<std::string,std::string> titles={{"overview","个人概览"},{"chat","AI管家"},{"reports","学习报告"},{"plan","学习计划"},{"preferences","教学偏好"},{"memories","AI记忆"},{"tasks","AI任务"},{"usage","使用情况"},{"profile","个人资料"},{"settings","软件设置"},{"data","数据管理"}};
    if(!titles.count(page))throw std::invalid_argument("页面不存在");
    std::string menu;for(const auto& key:{"overview","chat","reports","plan","preferences","memories","tasks","usage","profile","settings","data"}) {
        const auto href=std::string("/my-ai")+(std::string(key)=="overview"?"":"/"+std::string(key));
        menu+="<a href=\""+href+"\""+(page==key?" class=\"is-active\" aria-current=\"page\"":"")+">"+titles.at(key)+"</a>";
    }
    return R"HTML(<!DOCTYPE html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>)HTML"+titles.at(page)+R"HTML( - 我的AI · 钢一定制AI</title><link rel="icon" href="/school-logo.png"><link rel="stylesheet" href="/dark.css"><link rel="stylesheet" href="/profile-radar.css"><link rel="stylesheet" href="/vendor/katex/katex.min.css"><link rel="stylesheet" href="/my-ai.css"><script defer src="/vendor/katex/katex.min.js"></script><script defer src="/chat-render.js"></script><script defer src="/profile-radar.js"></script><script defer src="/vendor/echarts/echarts.min.js"></script><script defer src="/my-ai.js"></script></head><body class="dark-app my-ai-body" data-my-ai-page=")HTML"+page+R"HTML("><header class="my-ai-top"><a class="my-ai-brand" href="/"><img src="/school-logo.png" alt="柳州市钢一中学校徽"><span>钢一<b>定制AI</b></span></a><nav aria-label="主导航"><a href="/">首页</a><a href="/my-courses">我的课程</a><a href="/current-course">当前课程</a><a href="/ask">AI导师</a><a href="/my-ai" aria-current="page">我的AI</a></nav></header><div class="my-ai-shell"><aside class="my-ai-sidebar"><a class="my-ai-sidebar-title" href="/my-ai">我的AI</a><p>本机个人档案</p><nav aria-label="我的AI功能目录">)HTML"+menu+R"HTML(</nav><a class="my-ai-back" href="/">← 返回首页</a></aside><main class="my-ai-main"><header class="my-ai-heading"><div><p class="my-ai-eyebrow">我的AI</p><h1>)HTML"+titles.at(page)+R"HTML(</h1></div><a id="return-chat" hidden>返回刚才的聊天 →</a></header><p id="my-ai-status" role="status" aria-live="polite">正在读取本机档案…</p><section id="my-ai-content" aria-label=")HTML"+titles.at(page)+R"HTML("></section></main></div></body></html>)HTML";
}
}

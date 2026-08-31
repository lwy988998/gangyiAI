#include "page_renderer.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace gangyi {
namespace {

std::string htmlEscape(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    for (const char character : value) {
        switch (character) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            default: result += character; break;
        }
    }
    return result;
}

std::string jsString(const std::string& value) {
    return nlohmann::json(value).dump();
}

std::string urlEncode(const std::string& value) {
    std::ostringstream encoded;
    encoded << std::uppercase << std::hex << std::setfill('0');
    for (const unsigned char character : value) {
        if ((character >= 'a' && character <= 'z') ||
            (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9') ||
            character == '-' || character == '_' || character == '.' || character == '~') {
            encoded << character;
        } else {
            encoded << '%' << std::setw(2) << static_cast<int>(character) << std::setw(0);
        }
    }
    return encoded.str();
}

std::string document(const std::string& title, const std::string& body, const std::string& script = {}) {
    return "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no\"><title>" +
        htmlEscape(title) + "</title><link rel=\"stylesheet\" href=\"/styles.css\"></head><body class=\"geist_a71539c9-module__T19VSG__variable geist_mono_8d43a2aa-module__8Li5zG__variable antialiased page-transition\">" + body + script + "</body></html>";
}

std::string navLink(const std::string& href, const std::string& label, bool active) {
    return std::string("<a class=\"rounded-lg px-3 py-2 transition ") +
        (active ? "text-sky-700 hover:bg-sky-50" : "text-slate-600 hover:bg-slate-50") +
        "\" href=\"" + href + "\">" + label + "</a>";
}

std::string headerShell(const std::string& active) {
    const auto is = [&](const std::string& key) { return active == key; };
    return std::string(R"HTML(<header class="sticky top-0 z-30 w-full max-w-full border-b border-slate-200/70 bg-white/92 backdrop-blur-xl"><div class="mx-auto flex min-h-13 w-full max-w-7xl items-center justify-between gap-2 px-3 py-2 sm:min-h-16 md:px-6 lg:px-8"><div class="flex min-w-0 items-center gap-2 sm:gap-3"><a class="shrink-0 whitespace-nowrap text-base font-semibold tracking-tight text-sky-900 md:text-lg" href="/">钢一定制AI</a><a class="inline-flex min-h-9 shrink-0 rounded-full border border-slate-200 bg-white/80 px-2.5 py-1.5 text-sm font-medium text-slate-700 shadow-sm transition hover:border-sky-300 hover:bg-sky-50 hover:text-sky-700 focus:outline-none focus:ring-2 focus:ring-sky-300 sm:px-3" href="/login">登录</a></div><nav class="hidden min-w-0 flex-1 items-center justify-end gap-2 text-sm font-medium text-slate-600 md:flex lg:gap-4">)HTML") +
        navLink("/", "首页", is("home")) +
        navLink("/learn", "学习", is("learn")) +
        navLink("/progress", "进度", is("progress")) +
        navLink("/my-courses", "我的课程", is("my-courses")) +
        navLink("/ask", "问答", is("ask")) +
        navLink("/login", "账号", is("login")) +
        R"HTML(</nav><details class="group relative shrink-0 md:hidden"><summary class="list-none rounded-full border border-slate-200 bg-white px-3 py-2 text-sm font-semibold text-slate-700 shadow-sm marker:hidden focus:outline-none focus:ring-2 focus:ring-sky-300 [&::-webkit-details-marker]:hidden"><span class="inline-flex items-center gap-1.5"><span class="text-base leading-none">≡</span>菜单</span></summary><nav class="fixed left-3 right-3 top-14 mt-2 flex max-h-[calc(100dvh-4rem)] flex-col gap-1 overflow-y-auto rounded-2xl border border-slate-200 bg-white p-2 text-sm font-medium text-slate-700 shadow-xl shadow-slate-900/10 sm:left-auto sm:right-4 sm:w-[min(18rem,calc(100vw-1.5rem))] md:absolute md:right-0 md:top-full">)HTML" +
        navLink("/", "首页", is("home")) +
        navLink("/learn", "学习", is("learn")) +
        navLink("/progress", "进度", is("progress")) +
        navLink("/my-courses", "我的课程", is("my-courses")) +
        navLink("/ask", "问答", is("ask")) +
        navLink("/login", "登录", is("login")) +
        R"HTML(</nav></details></div></header>)HTML";
}

std::string loadingSpinner(const std::string& title, const std::string& subtitle) {
    return R"HTML(<div class="rounded-3xl border border-sky-100 bg-white p-8 text-center shadow-sm shadow-sky-900/5"><div class="mx-auto h-10 w-10 animate-spin rounded-full border-4 border-sky-100 border-t-sky-600"></div><p class="mt-5 font-semibold text-slate-800">)HTML" +
        htmlEscape(title) +
        R"HTML(</p><p class="mt-2 text-sm text-slate-500">)HTML" +
        htmlEscape(subtitle) +
        R"HTML(</p></div>)HTML";
}

std::string sectionTitle(const std::string& eyebrow, const std::string& title, const std::string& subtitle) {
    return R"HTML(<div><p class="text-sm font-semibold text-sky-700">)HTML" +
        htmlEscape(eyebrow) +
        R"HTML(</p><h1 class="mt-3 text-3xl font-bold tracking-tight sm:text-4xl">)HTML" +
        htmlEscape(title) +
        R"HTML(</h1><p class="mt-3 text-slate-600">)HTML" +
        htmlEscape(subtitle) +
        R"HTML(</p></div>)HTML";
}

}  // namespace

std::string renderHomePage() {
    const std::string body = headerShell("home") + R"HTML(
<main class="min-h-screen bg-[radial-gradient(circle_at_top,rgba(219,234,254,0.9),rgba(248,251,255,0.98)_44%,rgba(255,255,255,1))] text-slate-950">
  <section class="mx-auto flex w-full max-w-6xl flex-col items-center px-4 pb-16 pt-14 text-center sm:px-6 sm:pt-20">
    <p class="text-sm font-semibold uppercase tracking-[0.18em] text-sky-700">柳州市钢一中学</p>
    <h1 class="mt-4 max-w-3xl text-3xl font-bold tracking-tight sm:text-5xl">柳州市钢一中学2629班定制AI</h1>
    <p class="mt-5 max-w-2xl text-base leading-7 text-slate-600 sm:text-lg">把学习目标变成清晰、可执行、能持续跟踪的个人课程。</p>
    <form class="mt-10 w-full max-w-3xl rounded-3xl border border-slate-200 bg-white p-5 text-left shadow-xl shadow-slate-900/8 sm:p-7" action="/plan" method="get">
      <label class="text-sm font-semibold text-slate-800" for="goal">你的学习目标</label>
      <textarea id="goal" name="goal" required rows="3" class="mt-2 w-full resize-y rounded-xl border border-slate-200 bg-slate-50 px-4 py-3 text-base text-slate-900 outline-none transition placeholder:text-slate-400 focus:border-sky-500 focus:bg-white focus:ring-4 focus:ring-sky-100" placeholder="例如：我想在三个月内掌握 Python 基础并完成一个小项目"></textarea>
      <div class="mt-5 flex flex-col gap-4 sm:flex-row sm:items-end sm:justify-between">
        <fieldset><legend class="text-sm font-semibold text-slate-800">规划模式</legend><div class="mt-2 flex gap-2"><label class="cursor-pointer"><input class="peer sr-only" type="radio" name="mode" value="lite"><span class="inline-flex rounded-xl border border-slate-200 px-4 py-2 text-sm text-slate-600 peer-checked:border-sky-600 peer-checked:bg-sky-50 peer-checked:text-sky-800">快速规划</span></label><label class="cursor-pointer"><input class="peer sr-only" type="radio" name="mode" value="deep" checked><span class="inline-flex rounded-xl border border-slate-200 px-4 py-2 text-sm text-slate-600 peer-checked:border-sky-600 peer-checked:bg-sky-50 peer-checked:text-sky-800">深度规划</span></label></div></fieldset>
        <button class="inline-flex min-h-12 items-center justify-center rounded-xl bg-sky-700 px-6 text-sm font-semibold text-white transition hover:bg-sky-800 focus:outline-none focus:ring-4 focus:ring-sky-200" type="submit">生成我的课程 <span class="ml-2" aria-hidden="true">→</span></button>
      </div>
    </form>
  </section>
  <section class="mx-auto grid w-full max-w-6xl gap-4 px-4 pb-20 sm:grid-cols-2 sm:px-6 lg:grid-cols-4">
    <article class="rounded-2xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><div class="text-2xl text-sky-600">01</div><h2 class="mt-4 font-semibold">阶段任务</h2><p class="mt-2 text-sm leading-6 text-slate-600">把大目标拆成每个阶段都能完成的行动。</p></article>
    <article class="rounded-2xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><div class="text-2xl text-sky-600">02</div><h2 class="mt-4 font-semibold">微课程讲解</h2><p class="mt-2 text-sm leading-6 text-slate-600">围绕你的目标组织重点知识与学习顺序。</p></article>
    <article class="rounded-2xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><div class="text-2xl text-sky-600">03</div><h2 class="mt-4 font-semibold">练习测验</h2><p class="mt-2 text-sm leading-6 text-slate-600">用练习和检查点确认每一步真正掌握。</p></article>
    <article class="rounded-2xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><div class="text-2xl text-sky-600">04</div><h2 class="mt-4 font-semibold">进度跟踪</h2><p class="mt-2 text-sm leading-6 text-slate-600">持续记录进展，及时调整接下来的路线。</p></article>
  </section>
</main>)HTML";
    return document("柳州市钢一中学2629班定制AI - 把学习目标变成可执行课程", body);
}

std::string renderPlanPage(const std::string& goal, const std::string& mode) {
    const std::string encodedGoal = urlEncode(goal);
    const std::string body = headerShell("home") +
        R"HTML(<main class="min-h-screen bg-[#f5f9ff] text-slate-950"><section class="mx-auto w-full max-w-4xl px-4 py-10 sm:px-6 sm:py-16">)HTML" +
        sectionTitle("课程规划 · " + std::string(mode == "lite" ? "快速模式" : "深度模式"), "正在生成你的课程总览", "目标：" + goal) +
        R"HTML(<section id="loading" class="mt-8">)HTML" +
        loadingSpinner("AI 正在为你整理学习路线", "通常需要一点时间，请保持页面打开。") +
        R"HTML(</section><section id="result" class="hidden"></section></section></main>)HTML";

    const std::string script = R"HTML(<script>(()=>{const goal=)HTML" + jsString(goal) + R"HTML(;const mode=)HTML" + jsString(mode) + R"HTML(;const loading=document.getElementById('loading');const result=document.getElementById('result');const esc=(s)=>{const d=document.createElement('div');d.textContent=s??'';return d.innerHTML;};fetch('/api/generate-plan',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({goal,mode})}).then(r=>r.json().then(data=>({ok:r.ok,data}))).then(({ok,data})=>{if(!ok)throw new Error(data.error||'生成失败');document.querySelector('h1').textContent=data.courseTitle||data.title||'你的课程总览';loading.classList.add('hidden');result.classList.remove('hidden');result.className='grid gap-4';result.innerHTML='<div class="rounded-3xl border border-sky-100 bg-white p-6 shadow-sm shadow-sky-900/5"><p class="text-slate-600">'+esc(data.courseSummary||data.summary||'课程已生成')+'</p></div>'+(data.phases||[]).map((p,i)=>'<article class="rounded-2xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><p class="text-sm font-semibold text-sky-700">阶段 '+(i+1)+' · '+esc(p.duration||'')+'</p><h2 class="mt-2 text-xl font-semibold">'+esc(p.name||'学习阶段')+'</h2><p class="mt-2 leading-7 text-slate-600">'+esc(p.objective||p.description||'')+'</p></article>').join('');}).catch(error=>{loading.innerHTML='<div class="rounded-3xl border border-rose-100 bg-rose-50 p-8 text-center"><p class="font-semibold text-rose-700">课程生成失败</p><p class="mt-2 text-sm text-slate-600">'+esc(error.message)+'</p><a class="mt-5 inline-flex rounded-xl bg-sky-700 px-5 py-3 text-sm font-semibold text-white" href="/plan?goal=)HTML" + encodedGoal + R"HTML(&mode=)HTML" + htmlEscape(mode) + R"HTML(">重试</a></div>';});})();</script>)HTML";
    return document("课程规划 - 钢一定制AI", body, script);
}

std::string renderLearnPage(const std::string& courseId, const std::string& phaseIndex, const std::string& topicIndex) {
    const std::string body = headerShell("learn") + R"HTML(
<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950">
  <section class="mx-auto flex w-full max-w-6xl flex-col px-4 py-6 sm:px-6">
    <div class="flex flex-col gap-4 lg:flex-row lg:items-start lg:justify-between">
      <div class="max-w-3xl">
        <p class="text-sm font-semibold text-sky-700">微课程 · 学习中</p>
        <h1 id="learn-title" class="mt-2 text-3xl font-bold tracking-tight sm:text-4xl">正在加载学习内容</h1>
        <p id="learn-summary" class="mt-3 max-w-3xl text-base leading-7 text-slate-600">系统会先渲染课程框架，随后根据 /api/learn 返回的内容填充步骤、示例、练习、测验与参考资料。</p>
      </div>
      <div class="grid gap-2 rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:min-w-72">
        <div class="flex items-center justify-between gap-3 text-sm"><span class="text-slate-500">课程</span><span id="learn-course-id" class="font-semibold text-slate-800"></span></div>
        <div class="flex items-center justify-between gap-3 text-sm"><span class="text-slate-500">阶段</span><span id="learn-phase-index" class="font-semibold text-slate-800"></span></div>
        <div class="flex items-center justify-between gap-3 text-sm"><span class="text-slate-500">主题</span><span id="learn-topic-index" class="font-semibold text-slate-800"></span></div>
      </div>
    </div>
    <section class="mt-6 rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-6">
      <div id="learn-loading" class="rounded-3xl border border-sky-100 bg-sky-50/60 p-8 text-center">
        <div class="mx-auto h-10 w-10 animate-spin rounded-full border-4 border-sky-100 border-t-sky-600"></div>
        <p class="mt-5 font-semibold text-slate-800">正在加载学习内容</p>
        <p class="mt-2 text-sm text-slate-600">页面骨架已就绪，课程步骤会在数据返回后自动展开。</p>
      </div>
      <div id="learn-content" class="hidden space-y-6">
        <section class="grid gap-4 lg:grid-cols-[1.3fr_0.9fr]">
          <article class="rounded-3xl border border-sky-100 bg-sky-50/70 p-5">
            <p class="text-sm font-semibold text-sky-700">本课概览</p>
            <h2 id="learn-content-title" class="mt-2 text-2xl font-bold tracking-tight text-slate-950"></h2>
            <p id="learn-content-summary" class="mt-3 leading-7 text-slate-600"></p>
          </article>
          <article class="rounded-3xl border border-slate-200 bg-white p-5">
            <p class="text-sm font-semibold text-slate-700">关键概念</p>
            <div id="learn-key-concepts" class="mt-3 flex flex-wrap gap-2"></div>
          </article>
        </section>
        <section class="grid gap-4 lg:grid-cols-2">
          <article class="rounded-3xl border border-sky-100 bg-white p-5">
            <p class="text-sm font-semibold text-sky-700">lessonSteps</p>
            <div id="learn-steps" class="mt-4 space-y-3"></div>
          </article>
          <article class="rounded-3xl border border-sky-100 bg-white p-5">
            <p class="text-sm font-semibold text-sky-700">示例</p>
            <div id="learn-examples" class="mt-4 space-y-3"></div>
          </article>
        </section>
        <section class="grid gap-4 lg:grid-cols-2">
          <article class="rounded-3xl border border-sky-100 bg-white p-5">
            <p class="text-sm font-semibold text-sky-700">练习</p>
            <div id="learn-practice" class="mt-4 space-y-3"></div>
          </article>
          <article class="rounded-3xl border border-sky-100 bg-white p-5">
            <p class="text-sm font-semibold text-sky-700">测验</p>
            <div id="learn-quiz" class="mt-4 space-y-3"></div>
          </article>
        </section>
        <section class="grid gap-4 lg:grid-cols-2">
          <article class="rounded-3xl border border-sky-100 bg-white p-5">
            <p class="text-sm font-semibold text-sky-700">常见错误</p>
            <div id="learn-mistakes" class="mt-4 space-y-2 text-sm leading-7 text-slate-700"></div>
          </article>
          <article class="rounded-3xl border border-sky-100 bg-white p-5">
            <p class="text-sm font-semibold text-sky-700">参考资料</p>
            <div id="learn-references" class="mt-4 space-y-3"></div>
          </article>
        </section>
      </div>
    </section>
  </section>
</main>)HTML";

    const std::string script = R"HTML(<script>(()=>{const courseId=)HTML" + jsString(courseId) + R"HTML(;const phaseIndex=)HTML" + jsString(phaseIndex) + R"HTML(;const topicIndex=)HTML" + jsString(topicIndex) + R"HTML(;const q=new URLSearchParams({courseId,phaseIndex,topicIndex});document.getElementById('learn-course-id').textContent=courseId||'未提供';document.getElementById('learn-phase-index').textContent=phaseIndex||'0';document.getElementById('learn-topic-index').textContent=topicIndex||'0';const loading=document.getElementById('learn-loading');const content=document.getElementById('learn-content');const title=document.getElementById('learn-content-title');const summary=document.getElementById('learn-content-summary');const concepts=document.getElementById('learn-key-concepts');const steps=document.getElementById('learn-steps');const examples=document.getElementById('learn-examples');const practice=document.getElementById('learn-practice');const quiz=document.getElementById('learn-quiz');const mistakes=document.getElementById('learn-mistakes');const references=document.getElementById('learn-references');const esc=(s)=>{const d=document.createElement('div');d.textContent=s??'';return d.innerHTML;};const pill=(text)=>'<span class="inline-flex rounded-full border border-sky-200 bg-sky-50 px-3 py-1 text-sm font-medium text-sky-800">'+esc(text)+'</span>';const list=(items,renderItem)=>Array.isArray(items)&&items.length?items.map(renderItem).join(''):'<p class="text-sm text-slate-500">暂无内容</p>';const renderStep=(step,index)=>'<div class="rounded-2xl border border-sky-100 bg-sky-50/50 p-4"><p class="text-xs font-semibold uppercase tracking-[0.16em] text-sky-700">Step '+(index+1)+'</p><h3 class="mt-2 font-semibold text-slate-950">'+esc(step.title||step.name||('步骤 '+(index+1)))+'</h3><p class="mt-2 text-sm leading-7 text-slate-600">'+esc(step.explanation||step.content||'')+'</p><div class="mt-3 grid gap-2 text-sm text-slate-700"><p><span class="font-semibold text-slate-900">示例：</span>'+esc(step.example||'')+'</p><p><span class="font-semibold text-slate-900">行动：</span>'+esc(step.action||'')+'</p><p><span class="font-semibold text-slate-900">检查：</span>'+esc(step.check||'')+'</p></div></div>';const renderExample=(item,index)=>'<div class="rounded-2xl border border-slate-200 bg-slate-50 p-4"><p class="text-sm font-semibold text-slate-900">'+esc(item.title||('示例 '+(index+1)))+'</p><pre class="mt-2 whitespace-pre-wrap rounded-xl bg-white p-3 text-sm leading-7 text-slate-700">'+esc(item.content||item.example||'')+'</pre></div>';const renderPractice=(item,index)=>'<div class="rounded-2xl border border-slate-200 bg-white p-4"><div class="flex flex-wrap items-center gap-2"><p class="font-semibold text-slate-900">'+esc(item.title||('练习 '+(index+1)))+'</p><span class="rounded-full bg-sky-50 px-3 py-1 text-xs font-semibold text-sky-700">'+esc(item.difficulty||'')+'</span></div><p class="mt-2 text-sm leading-7 text-slate-600">'+esc(item.task||item.content||'')+'</p><p class="mt-3 text-sm text-slate-700"><span class="font-semibold text-slate-900">检查：</span>'+esc(item.check||'')+'</p></div>';const renderQuiz=(item,index)=>'<div class="rounded-2xl border border-slate-200 bg-white p-4"><p class="text-sm font-semibold text-slate-900">题目 '+(index+1)+'</p><p class="mt-2 text-sm leading-7 text-slate-700">'+esc(item.question||'')+'</p><div class="mt-3 grid gap-2">'+(Array.isArray(item.options)?item.options.map((option,optionIndex)=>'<div class="rounded-xl border border-slate-200 bg-slate-50 px-3 py-2 text-sm text-slate-700">'+String.fromCharCode(65+optionIndex)+'. '+esc(option)+'</div>').join(''):'')+'</div><p class="mt-3 text-sm text-slate-600"><span class="font-semibold text-slate-900">解析：</span>'+esc(item.explanation||'')+'</p></div>';const renderReference=(item,index)=>'<a class="block rounded-2xl border border-sky-100 bg-sky-50/50 p-4 transition hover:border-sky-300 hover:bg-sky-50" href="'+esc(item.url||'#')+'" target="_blank" rel="noreferrer"><p class="text-sm font-semibold text-slate-900">'+esc(item.title||('参考资料 '+(index+1)))+'</p><p class="mt-1 text-sm text-slate-600">'+esc(item.source||item.type||'')+'</p><p class="mt-2 break-all text-xs text-sky-700">'+esc(item.url||'')+'</p></a>';fetch('/api/learn?'+q.toString()).then(async response=>{const data=await response.json();if(!response.ok) throw new Error(data.error||'加载失败');return data;}).then(data=>{title.textContent=data.title||data.courseTitle||'微课程';summary.textContent=data.summary||data.description||'';concepts.innerHTML=list(data.keyConcepts||data.concepts||[],pill);steps.innerHTML=list(data.lessonSteps||data.steps||[],renderStep);examples.innerHTML=list(data.examples||[],renderExample);practice.innerHTML=list(data.practice||[],renderPractice);quiz.innerHTML=list(data.quiz||[],renderQuiz);mistakes.innerHTML=(data.commonMistakes||[]).length?(data.commonMistakes||[]).map(item=>'<div class="rounded-xl border border-amber-100 bg-amber-50 px-3 py-2">'+esc(item)+'</div>').join(''):'<p class="text-sm text-slate-500">暂无内容</p>';references.innerHTML=list(data.references||[],renderReference);loading.classList.add('hidden');content.classList.remove('hidden');}).catch(error=>{loading.innerHTML='<div class="rounded-3xl border border-rose-100 bg-rose-50 p-6 text-center"><p class="font-semibold text-rose-700">学习内容暂时无法加载</p><p class="mt-2 text-sm text-slate-600">'+esc(error.message)+'</p><p class="mt-4 text-sm text-slate-500">请稍后重试，或先保留当前页面骨架等待数据接口接通。</p></div>';});})();</script>)HTML";
    return document("柳州市钢一中学2629班定制AI - 高中学习规划助手", body, script);
}

std::string renderProgressPage(const std::string& courseId) {
    const std::string body = headerShell("progress") + R"HTML(
<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950">
  <section class="mx-auto w-full max-w-6xl px-4 py-6 sm:px-6">
    <div class="grid gap-6 lg:grid-cols-[1.2fr_0.8fr]">
      <article class="rounded-3xl border border-sky-100 bg-white p-6 shadow-sm shadow-sky-900/5 sm:p-8">
        <p class="text-sm font-semibold text-sky-700">学习进度</p>
        <h1 class="mt-2 text-3xl font-bold tracking-tight sm:text-4xl">正在加载学习进度</h1>
        <p class="mt-3 text-base leading-7 text-slate-600">这里会展示课程总进度、阶段完成情况和最近的学习记录。</p>
        <div class="mt-6 rounded-2xl border border-slate-200 bg-slate-50 p-4">
          <div class="flex items-center justify-between text-sm text-slate-600"><span>当前课程</span><span class="font-semibold text-slate-900">)HTML" + htmlEscape(courseId.empty() ? "未提供课程 ID" : courseId) + R"HTML(</span></div>
          <div class="mt-3 h-3 overflow-hidden rounded-full bg-slate-200"><div class="h-full w-2/5 rounded-full bg-sky-600"></div></div>
          <div class="mt-2 flex items-center justify-between text-sm text-slate-600"><span>总体完成度</span><span class="font-semibold text-sky-700">40%</span></div>
        </div>
      </article>
      <article class="rounded-3xl border border-sky-100 bg-white p-6 shadow-sm shadow-sky-900/5 sm:p-8">
        <p class="text-sm font-semibold text-sky-700">状态摘要</p>
        <div class="mt-4 space-y-3">
          <div class="rounded-2xl border border-slate-200 bg-slate-50 p-4"><p class="text-sm font-semibold text-slate-900">阶段一：基础理解</p><p class="mt-1 text-sm text-slate-600">已完成 2 / 5 个步骤</p></div>
          <div class="rounded-2xl border border-slate-200 bg-slate-50 p-4"><p class="text-sm font-semibold text-slate-900">阶段二：动手实践</p><p class="mt-1 text-sm text-slate-600">进行中，建议先完成一个小练习</p></div>
          <div class="rounded-2xl border border-slate-200 bg-slate-50 p-4"><p class="text-sm font-semibold text-slate-900">阶段三：测验巩固</p><p class="mt-1 text-sm text-slate-600">等待进入</p></div>
        </div>
      </article>
    </div>
    <section class="mt-6 grid gap-4 lg:grid-cols-3">
      <article class="rounded-3xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><p class="text-sm font-semibold text-sky-700">最近任务</p><p class="mt-2 text-sm leading-7 text-slate-600">这里会显示最近一次学习、提交和测验记录。</p></article>
      <article class="rounded-3xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><p class="text-sm font-semibold text-sky-700">待完成</p><p class="mt-2 text-sm leading-7 text-slate-600">下一步建议基于最近掌握情况自动生成。</p></article>
      <article class="rounded-3xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><p class="text-sm font-semibold text-sky-700">提示</p><p class="mt-2 text-sm leading-7 text-slate-600">进度页先提供框架，接口接通后会自动刷新具体数据。</p></article>
    </section>
  </section>
</main>)HTML";
    return document("柳州市钢一中学2629班定制AI - 高中学习规划助手", body);
}

std::string renderLoginPage() {
    const std::string body = headerShell("login") + R"HTML(
<main class="min-h-screen bg-[radial-gradient(circle_at_top,rgba(219,234,254,0.9),rgba(248,251,255,0.98)_44%,rgba(255,255,255,1))] text-slate-950">
  <section class="mx-auto flex w-full max-w-md flex-col px-4 py-14 sm:px-6">
    <div class="text-center">
      <p class="text-sm font-semibold text-sky-700">钢一定制AI</p>
      <h1 class="mt-3 text-3xl font-bold tracking-tight sm:text-4xl">登录 钢一定制AI</h1>
      <p class="mt-3 text-sm leading-6 text-slate-600">输入账号和密码，进入你的课程、进度和问答页面。</p>
    </div>
    <form class="mt-8 space-y-5 rounded-3xl border border-slate-200 bg-white p-6 shadow-xl shadow-slate-900/8" action="/api/auth/login" method="post">
      <div>
        <label class="text-sm font-semibold text-slate-800" for="account">账号</label>
        <input id="account" name="account" type="text" autocomplete="username" required class="mt-2 w-full rounded-xl border border-slate-200 bg-slate-50 px-4 py-3 text-base text-slate-900 outline-none transition placeholder:text-slate-400 focus:border-sky-500 focus:bg-white focus:ring-4 focus:ring-sky-100" placeholder="学号 / 手机号 / 账号" />
      </div>
      <div>
        <label class="text-sm font-semibold text-slate-800" for="password">密码</label>
        <input id="password" name="password" type="password" autocomplete="current-password" required class="mt-2 w-full rounded-xl border border-slate-200 bg-slate-50 px-4 py-3 text-base text-slate-900 outline-none transition placeholder:text-slate-400 focus:border-sky-500 focus:bg-white focus:ring-4 focus:ring-sky-100" placeholder="请输入密码" />
      </div>
      <button class="inline-flex min-h-12 w-full items-center justify-center rounded-xl bg-sky-700 px-6 text-sm font-semibold text-white transition hover:bg-sky-800 focus:outline-none focus:ring-4 focus:ring-sky-200" type="submit">登录</button>
      <p class="text-center text-sm text-slate-500">登录后可查看学习进度、课程列表和问答记录。</p>
    </form>
  </section>
</main>)HTML";
    return document("柳州市钢一中学2629班定制AI - 高中学习规划助手", body);
}

std::string renderAskPage(const std::string& goal) {
    const std::string body = headerShell("ask") + R"HTML(
<main class="min-h-screen bg-[#f5f9ff] text-slate-950">
  <section class="mx-auto flex w-full max-w-5xl flex-col px-4 py-6 sm:px-6">
    <div class="rounded-3xl border border-sky-100 bg-white p-6 shadow-sm shadow-sky-900/5 sm:p-8">
      <p class="text-sm font-semibold text-sky-700">学习问答</p>
      <h1 class="mt-2 text-3xl font-bold tracking-tight sm:text-4xl">正在为你生成步骤化解答。</h1>
      <p class="mt-3 text-base leading-7 text-slate-600">在这里把目标、困惑或作业题直接发给系统，后续可以接上对话式 API。</p>
      <div class="mt-5 rounded-2xl border border-slate-200 bg-sky-50/60 p-4 text-sm leading-7 text-slate-700"><span class="font-semibold text-slate-900">当前目标：</span><span>)HTML" + htmlEscape(goal.empty() ? "未填写" : goal) + R"HTML(</span></div>
    </div>
    <section class="mt-6 grid gap-4 lg:grid-cols-[1.2fr_0.8fr]">
      <div class="rounded-3xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5 sm:p-6">
        <div class="space-y-4">
          <div class="max-w-[85%] rounded-2xl rounded-bl-md bg-sky-50 px-4 py-3 text-sm leading-7 text-slate-700">你可以直接问：<span class="font-semibold text-slate-900">这道题为什么这样做？</span> 或者 <span class="font-semibold text-slate-900">我该怎么开始学</span>。</div>
          <div class="ml-auto max-w-[85%] rounded-2xl rounded-br-md bg-slate-100 px-4 py-3 text-sm leading-7 text-slate-700">系统会在后续版本里返回分步骤解释、检查点和参考资料。</div>
        </div>
      </div>
      <aside class="rounded-3xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5 sm:p-6">
        <h2 class="text-lg font-semibold text-slate-950">不知道怎么问？可以先点一个</h2>
        <div class="mt-4 grid gap-2 text-sm text-slate-700">
          <div class="rounded-xl border border-slate-200 bg-slate-50 px-3 py-2">帮我把 <span class="font-semibold text-slate-900">这个目标</span> 拆成学习步骤</div>
          <div class="rounded-xl border border-slate-200 bg-slate-50 px-3 py-2">这部分知识点最容易错在哪里</div>
          <div class="rounded-xl border border-slate-200 bg-slate-50 px-3 py-2">给我一个适合高中生的练习题</div>
        </div>
      </aside>
    </section>
    <section class="sticky bottom-0 mt-6 rounded-3xl border border-sky-100 bg-white p-4 shadow-lg shadow-sky-900/10 sm:p-5">
      <form class="flex flex-col gap-3 sm:flex-row sm:items-end" action="/api/ask" method="post">
        <input type="hidden" name="goal" value=)HTML" + jsString(goal) + R"HTML( />
        <div class="flex-1">
          <label class="sr-only" for="question">输入你的问题</label>
          <textarea id="question" name="question" rows="3" class="w-full resize-y rounded-2xl border border-slate-200 bg-slate-50 px-4 py-3 text-base text-slate-900 outline-none transition placeholder:text-slate-400 focus:border-sky-500 focus:bg-white focus:ring-4 focus:ring-sky-100" placeholder="例如：请帮我解释 Python 列表和元组的区别"></textarea>
        </div>
        <button class="inline-flex min-h-12 items-center justify-center rounded-2xl bg-sky-700 px-6 text-sm font-semibold text-white transition hover:bg-sky-800 focus:outline-none focus:ring-4 focus:ring-sky-200" type="submit">发送</button>
      </form>
    </section>
  </section>
</main>)HTML";
    return document("柳州市钢一中学2629班定制AI - 高中学习规划助手", body);
}

std::string renderMyCoursesPage() {
    const std::string body = headerShell("my-courses") + R"HTML(
<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950">
  <section class="mx-auto w-full max-w-6xl px-4 py-6 sm:px-6">
    <section class="overflow-hidden rounded-3xl border border-sky-100 bg-white p-6 shadow-sm shadow-sky-900/5 sm:p-8">
      <p class="text-sm font-semibold text-sky-700">我的课程</p>
      <h1 class="mt-2 text-3xl font-bold tracking-tight sm:text-4xl">正在加载我的课堂</h1>
      <p class="mt-3 text-base leading-7 text-slate-600">这里会显示已生成的课程、最近学习记录和继续学习入口。</p>
    </section>
    <section class="mt-6 grid gap-4 lg:grid-cols-2">
      <article class="rounded-3xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5 sm:p-6">
        <div class="flex items-center justify-between gap-3"><div><p class="text-sm font-semibold text-sky-700">课程卡片</p><h2 class="mt-1 text-xl font-semibold text-slate-950">示例课程</h2></div><a class="rounded-full border border-sky-200 bg-sky-50 px-4 py-2 text-sm font-semibold text-sky-700" href="/learn?courseId=demo-course&phaseIndex=0&topicIndex=0">继续学习</a></div>
        <p class="mt-3 text-sm leading-7 text-slate-600">课程名称、阶段、目标与最近进度会在这里列出。</p>
        <div class="mt-4 h-2 overflow-hidden rounded-full bg-slate-200"><div class="h-full w-1/2 rounded-full bg-sky-600"></div></div>
        <p class="mt-2 text-xs text-slate-500">完成度 50%</p>
      </article>
      <article class="rounded-3xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5 sm:p-6">
        <div class="flex items-center justify-between gap-3"><div><p class="text-sm font-semibold text-sky-700">课程卡片</p><h2 class="mt-1 text-xl font-semibold text-slate-950">学习进度</h2></div><a class="rounded-full border border-sky-200 bg-sky-50 px-4 py-2 text-sm font-semibold text-sky-700" href="/progress?courseId=demo-course">查看进度</a></div>
        <p class="mt-3 text-sm leading-7 text-slate-600">可在此展示多个课程条目，后续接入接口后会自动填充真实数据。</p>
        <div class="mt-4 grid gap-2 text-sm text-slate-700"><div class="rounded-xl border border-slate-200 bg-slate-50 px-3 py-2">暂无更多课程</div><div class="rounded-xl border border-slate-200 bg-slate-50 px-3 py-2">可以先去首页生成新课程</div></div>
      </article>
    </section>
  </section>
</main>)HTML";
    return document("柳州市钢一中学2629班定制AI - 高中学习规划助手", body);
}

}  // namespace gangyi

#include "page_renderer.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace gangyi {
namespace {

using script = std::string;

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

std::string renderPlanPage(const std::string& goal, const std::string& mode,
                           const std::string& courseId, const std::string& anonymousId) {
    const std::string body = R"HTML(<link rel="stylesheet" href="/plan.css">)HTML" + headerShell("home") + R"HTML(
<main class="plan-page min-h-screen bg-[#f5f9ff] text-slate-950">
  <section class="mx-auto w-full max-w-7xl px-4 py-7 sm:px-6 lg:px-8 lg:py-10">
    <div class="plan-heading"><div><p class="text-sm font-semibold uppercase tracking-[0.16em] text-sky-700">学习工作台 · 课程规划</p><h1 id="plan-title" class="mt-2 text-3xl font-bold tracking-tight sm:text-5xl">正在准备你的课程</h1><p id="plan-goal" class="mt-3 max-w-3xl text-base leading-7 text-slate-600"></p></div><div id="save-status" class="save-status" role="status">正在连接</div></div>
    <section id="loading" class="mt-8">)HTML" + loadingSpinner("正在整理学习路线", "会先生成可执行的课程骨架，再补充课件、资源和项目。") + R"HTML(</section>
    <section id="failure" class="mt-8 hidden"></section>
    <section id="result" class="mt-8 hidden">
      <div class="plan-toolbar"><div><span class="plan-kicker">视图</span><strong id="view-label">深度课程</strong></div><div class="view-switch" role="tablist"><button type="button" data-view="deep" class="view-button active">深度课程</button><button type="button" data-view="lite" class="view-button">快速规划</button></div></div>
      <div id="deep-view" class="plan-view"></div><div id="lite-view" class="plan-view hidden"></div>
    </section>
  </section>
</main>)HTML";

    const std::string script = R"HTML(<script>(()=>{
const initialGoal=)HTML" + jsString(goal) + R"HTML(,initialMode=)HTML" + jsString(mode == "lite" ? "lite" : "deep") + R"HTML(,courseId=)HTML" + jsString(courseId) + R"HTML(,anonymousId=)HTML" + jsString(anonymousId) + R"HTML(;
const $=id=>document.getElementById(id), loading=$('loading'),failure=$('failure'),result=$('result'),status=$('save-status'); let plan=null,currentView=initialMode, mastered=new Set();
const esc=value=>{const d=document.createElement('div');d.textContent=value??'';return d.innerHTML}; const text=(v,f='暂无内容')=>esc(v||f); const arr=(v)=>Array.isArray(v)?v:[];
const list=(items,fn)=>arr(items).length?arr(items).map(fn).join(''):'<p class="empty-note">暂无内容</p>';
const card=(label,title,body,extra='')=>'<article class="plan-card '+extra+'"><p class="plan-kicker">'+text(label)+'</p><h2>'+text(title)+'</h2>'+body+'</article>';
const pill=v=>'<span class="plan-pill">'+text(v)+'</span>';
function mind(node,depth=0){return '<li class="mind-node depth-'+depth+'"><span>'+text(node.label||node.title)+'</span>'+((node.children||[]).length?'<ul>'+arr(node.children).map(x=>mind(x,depth+1)).join('')+'</ul>':'')+'</li>'}
function phase(p,i){const steps=arr(p.steps);return '<article class="phase-card"><div class="phase-number">'+String(i+1).padStart(2,'0')+'</div><div class="phase-main"><div class="phase-meta"><span>'+text(p.duration,'阶段周期')+'</span><span>'+text(i===0?'待开始':'后续阶段')+'</span></div><h3>'+text(p.name,'学习阶段')+'</h3><p>'+text(p.goal||p.description)+'</p><div class="phase-grid"><div><b>阶段产出</b><p>'+text(p.output)+'</p></div><div><b>检查点</b><p>'+text(p.checkpoint)+'</p></div></div><div class="step-list">'+list(steps.slice(0,5),(s,j)=>'<a href="/learn?courseId='+encodeURIComponent(courseId)+'&phaseIndex='+i+'&topicIndex='+j+'" class="step-row"><span>'+String(j+1).padStart(2,'0')+'</span><strong>'+text(s.title||s.name,'知识点')+'</strong><em>进入学习 →</em></a>')+'</div></div></article>'}
function deep(){const slides=arr(plan.slides), structure=arr(plan.courseStructure),resources=arr(plan.resources),projects=arr(plan.projects);return '<section class="plan-hero"> <div><span class="mode-badge">深度规划</span><h2>'+text(plan.title)+'</h2><p>'+text(plan.summary)+'</p></div><div class="hero-facts"><div><small>规划周期</small><b>'+text(plan.duration)+'</b></div><div><small>阶段数量</small><b>'+arr(plan.roadmap).length+' 个</b></div><div><small>学习目标</small><b>'+text(plan.outcome,'持续进步')+'</b></div></div></section><section class="next-action"><div><p class="plan-kicker">下一步</p><h2>从第一节开始，把计划变成进度</h2><p>每完成一个知识点，就会在本次会话中留下掌握标记。</p></div><a class="primary-action" href="/learn?courseId='+encodeURIComponent(courseId)+'&phaseIndex=0&topicIndex=0">开始第一节 <span>→</span></a></section><div class="progress-strip"><div><b>0%</b><span>课程进度 · 已完成 0 / '+structure.reduce((n,s)=>n+arr(s.topics).length,0)+'</span></div><div class="progress-track"><i></i></div></div>'+card('课件速览','把路线先看懂','<div class="slide-tabs">'+list(slides.slice(0,8),(s,i)=>'<button type="button" class="slide-tab '+(i?'':'selected')+'" data-slide="'+i+'">'+String(i+1).padStart(2,'0')+' · '+text(s.title)+'</button>')+'</div><div id="slide-content" class="slide-content"></div>','full-card')+card('知识结构',plan.mindMap?.title||'课程知识结构','<div class="mind-map"><ul>'+list(arr(plan.mindMap?.nodes),mind)+'</ul></div>','full-card')+'<section><div class="section-heading"><div><p class="plan-kicker">执行路线</p><h2>按阶段推进</h2></div><span class="muted-label">共 '+arr(plan.roadmap).length+' 个阶段</span></div><div class="phase-list">'+list(plan.roadmap,phase)+'</div></section>'+card('课程结构','知识点清单','<div class="structure-list">'+list(structure,(s,i)=>'<div class="structure-row"><div><b>'+text(s.stage)+'</b><p>'+arr(s.topics).map(text).join(' · ')+'</p></div><a href="/learn?courseId='+encodeURIComponent(courseId)+'&phaseIndex='+i+'&topicIndex=0">开始 →</a></div>')+'</div>','full-card')+card('精选资源','为你的目标准备的材料','<div class="resource-grid">'+list(resources.slice(0,8),r=>'<a class="resource-item" target="_blank" rel="noreferrer" href="'+esc(r.href||'#')+'"><div><span>'+text(r.type,'资源')+'</span>'+((r.free!==false)?'<b>免费</b>':'')+'</div><h3>'+text(r.name)+'</h3><p>'+text(r.description)+'</p><small>'+text(r.difficulty,'适合入门')+' ↗</small></a>')+'</div><p class="resource-source">'+text(plan.resourceSourceMessage,'以下为钢一定制AI推荐资源')+'</p>','full-card')+card('项目实战','用一个作品验收学习成果','<div class="project-grid">'+list(projects,(p)=>'<div class="project-item"><span>'+text(p.difficulty,'实践')+'</span><h3>'+text(p.name)+'</h3><p>'+text(p.output)+'</p><small>'+text(p.duration)+'</small></div>')+'</div>','full-card')}
function lite(){const roadmap=arr(plan.roadmap),steps=roadmap.flatMap(p=>arr(p.steps)).slice(0,5);return '<section class="lite-hero"><div><span class="mode-badge lite-badge">快速规划</span><h2>'+text(plan.title)+'</h2><p>'+text(plan.summary)+'</p></div><div class="lite-facts"><span><b>'+text(plan.duration)+'</b><small>周期</small></span><span><b>'+roadmap.length+'</b><small>阶段</small></span><span><b>先做会</b><small>再深入</small></span></div></section><section class="next-action lite-action"><div><p class="plan-kicker">立即开始</p><h2>先完成第一步，获得可见进展</h2></div><a class="primary-action lite-primary" href="/learn?courseId='+encodeURIComponent(courseId)+'&phaseIndex=0&topicIndex=0">开始第一节 <span>→</span></a></section>'+card('课程结构','只保留现在最需要的内容','<div class="lite-stage-list">'+list(roadmap.slice(0,3),(p,i)=>'<div><span>'+String(i+1).padStart(2,'0')+'</span><strong>'+text(p.name)+'</strong><em>'+text(p.duration)+'</em><p>'+text(p.goal||p.description)+'</p></div>')+'</div>','full-card')+card('核心步骤','怎么做，怎么验收','<div class="core-steps">'+list(steps,(s,i)=>'<div><span>'+String(i+1).padStart(2,'0')+'</span><div><h3>'+text(s.title)+'</h3><p>'+text(s.explanation)+'</p><b>验收：'+text(s.check)+'</b></div></div>')+'</div>','full-card')+card('提醒','少走弯路','<div class="mistake-grid">'+list(roadmap.slice(0,2),p=>'<div><b>'+text(p.name)+'</b><p>'+list(p.commonMistakes,m=>'<span>• '+text(m)+'</span>')+'</p></div>')+'</div>','full-card')+card('可参考资料','随用随查','<div class="resource-grid">'+list(arr(plan.resources).slice(0,5),r=>'<a class="resource-item" target="_blank" rel="noreferrer" href="'+esc(r.href||'#')+'"><h3>'+text(r.name)+'</h3><p>'+text(r.description)+'</p></a>')+'</div>','full-card')}
function render(){ $('plan-title').textContent=plan.title||'你的课程总览';$('plan-goal').textContent=(plan.courseIntro||plan.summary||'')+(initialGoal?' · 目标：'+initialGoal:'');$('deep-view').innerHTML=deep();$('lite-view').innerHTML=lite();setView(currentView);renderSlide(0)}
function renderSlide(i){const s=arr(plan.slides)[i]||{};const el=$('slide-content');if(el)el.innerHTML='<h3>'+text(s.title)+'</h3><p>'+text(s.content)+'</p><div class="bullet-list">'+list(s.bullets,x=>'<span>'+text(x)+'</span>')+'</div>'}
function setView(view){currentView=view==='lite'?'lite':'deep';$('deep-view').classList.toggle('hidden',currentView!=='deep');$('lite-view').classList.toggle('hidden',currentView!=='lite');$('view-label').textContent=currentView==='lite'?'快速规划':'深度课程';document.querySelectorAll('[data-view]').forEach(b=>b.classList.toggle('active',b.dataset.view===currentView))}
function failureState(error){const type=error.type||'';const messages={timeout:'生成超时，请重试。',auth_error:'当前模型接口认证失败，请检查服务配置后重试。',rate_limited:'AI 服务请求过于频繁，请稍后重试。',invalid_response:'生成内容未通过质量检查，请重新生成。',json_parse_error:'生成内容未通过质量检查，请重新生成。',quality_rejected:'生成内容未通过质量检查，请重新生成。',missing_config:'当前模型接口尚未配置，请检查服务配置。'};failure.innerHTML='<div class="failure-panel"><div class="failure-mark">!</div><div><p class="plan-kicker">生成没有完成</p><h2>'+text(messages[type]||'AI 服务暂时不可用，请稍后重试。')+'</h2><p>'+text(error.message,'服务暂时没有返回可用内容。')+'</p><div class="failure-actions"><button type="button" class="primary-action" id="retry">重试生成</button>'+(initialMode==='deep'?'<button type="button" class="secondary-action" id="fallback-lite">先生成快速规划</button>':'')+'<a class="secondary-action" href="/">返回首页</a></div></div></div>';failure.classList.remove('hidden');loading.classList.add('hidden');$('retry').onclick=()=>load(true);const fallback=$('fallback-lite');if(fallback)fallback.onclick=()=>{currentView='lite';load(true,'lite')}}
async function request(url,options={}){if(url.startsWith('/api/courses/')&&anonymousId&&!url.includes('?'))url+='?anonymousId='+encodeURIComponent(anonymousId);const controller=new AbortController();const timer=setTimeout(()=>controller.abort(),12000);try{const r=await fetch(url,{...options,signal:controller.signal});const data=await r.json();if(!r.ok)throw Object.assign(new Error(data.message||data.error||'请求失败'),{type:data.type});return data}finally{clearTimeout(timer)}}
async function save(){if(!plan||courseId)return;status.textContent='正在保存课程…';try{const anon=anonymousId||localStorage.getItem('gangyi-anonymous-id')||crypto.randomUUID();localStorage.setItem('gangyi-anonymous-id',anon);const saved=await request('/api/courses',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({goal:initialGoal||plan.outcome,title:plan.title,summary:plan.summary,mode:initialMode,source:'ai',anonymousId:anon,payload:plan})});status.textContent='已自动保存';localStorage.setItem('gangyi-last-course',JSON.stringify({id:saved.courseId,title:plan.title,href:saved.href,updatedAt:new Date().toISOString()}));history.replaceState({},'',saved.href);document.dispatchEvent(new CustomEvent('course-saved',{detail:saved}))}catch(e){status.textContent='未保存';status.classList.add('warning');localStorage.setItem('gangyi-pending-plan',JSON.stringify({goal:initialGoal,mode:initialMode,payload:plan}));const note=document.createElement('div');note.className='save-warning';note.innerHTML='<b>课程快照保存失败。</b> 当前页面内容仍可查看，已暂存到本机。<button type="button" id="save-again">重新保存</button>';result.prepend(note);$('save-again').onclick=()=>{note.remove();save()}}}
async function load(force=false,requestedMode=currentView){loading.classList.remove('hidden');failure.classList.add('hidden');status.textContent='正在加载';try{let data;if(courseId&&!force){data=await request('/api/courses/'+encodeURIComponent(courseId));plan=data.snapshot.payload;currentView=data.course.mode||currentView}else{data=await request('/api/generate-plan',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({goal:initialGoal,mode:requestedMode,bypassCache:force,forcePlan:force,retry:force?Date.now():undefined})});plan=data}if(!plan||!Array.isArray(plan.roadmap)||!plan.roadmap.length)throw Object.assign(new Error('课程结构不完整'),{type:'quality_rejected'});loading.classList.add('hidden');result.classList.remove('hidden');render();status.textContent=courseId?'已恢复课程':'生成完成';if(!courseId)save()}catch(e){failureState(e)}}
document.querySelectorAll('[data-view]').forEach(b=>b.onclick=()=>setView(b.dataset.view));document.addEventListener('click',e=>{const b=e.target.closest('[data-slide]');if(b){document.querySelectorAll('[data-slide]').forEach(x=>x.classList.remove('selected'));b.classList.add('selected');renderSlide(Number(b.dataset.slide))}});document.addEventListener('course-saved',e=>{if(e.detail?.href&&!courseId)location.replace(e.detail.href)});load();
})();</script>)HTML";
    return document("课程规划 - 钢一定制AI", body, script);
}

std::string renderLearnPageLegacy(const std::string& courseId, const std::string& phaseIndex, const std::string& topicIndex) {
    const std::string body = headerShell("learn") + R"HTML(
<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950">
  <section class="mx-auto flex w-full max-w-6xl flex-col px-4 py-6 sm:px-6">
    <div class="flex flex-col gap-4 lg:flex-row lg:items-start lg:justify-between"><div class="max-w-3xl"><p class="text-sm font-semibold text-sky-700">微课程 · 学习中</p><h1 id="learn-title" class="mt-2 text-3xl font-bold tracking-tight sm:text-4xl">正在加载学习内容</h1><p id="learn-summary" class="mt-3 max-w-3xl text-base leading-7 text-slate-600">一节课只解决一个具体问题，完成理解、练习和检查后再进入下一节。</p></div><div class="grid gap-2 rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:min-w-72"><div class="flex items-center justify-between gap-3 text-sm"><span class="text-slate-500">阶段</span><span id="learn-phase-name" class="font-semibold text-slate-800">加载中</span></div><div class="flex items-center justify-between gap-3 text-sm"><span class="text-slate-500">主题</span><span id="learn-topic-name" class="font-semibold text-slate-800">加载中</span></div><div class="mt-2 h-2 overflow-hidden rounded-full bg-slate-200"><div id="learn-bar" class="soft-progress-fill h-full w-0 rounded-full bg-sky-600"></div></div><p id="learn-save-status" class="text-xs text-slate-500">准备开始</p></div></div>
    <section class="mt-6 rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-6">
      <div id="learn-loading" class="rounded-3xl border border-sky-100 bg-sky-50/60 p-8 text-center">
        <div class="mx-auto h-10 w-10 animate-spin rounded-full border-4 border-sky-100 border-t-sky-600"></div>
        <p class="mt-5 font-semibold text-slate-800">正在加载学习内容</p>
        <p class="mt-2 text-sm text-slate-600">页面骨架已就绪，课程步骤会在数据返回后自动展开。</p>
      </div>
      <div id="learn-content" class="hidden space-y-6"><section class="grid gap-4 lg:grid-cols-[1.3fr_0.9fr]"><article class="rounded-3xl border border-sky-100 bg-sky-50/70 p-5"><p class="text-sm font-semibold text-sky-700">本课概览</p><h2 id="learn-content-title" class="mt-2 text-2xl font-bold tracking-tight text-slate-950"></h2><p id="learn-content-summary" class="mt-3 leading-7 text-slate-600"></p></article><article class="rounded-3xl border border-slate-200 bg-white p-5"><p class="text-sm font-semibold text-slate-700">关键概念</p><div id="learn-key-concepts" class="mt-3 flex flex-wrap gap-2"></div></article></section><section class="grid gap-4 lg:grid-cols-2"><article class="rounded-3xl border border-sky-100 bg-white p-5"><div class="flex items-center justify-between gap-3"><p class="text-sm font-semibold text-sky-700">理解步骤</p><span id="step-count" class="text-xs text-slate-500"></span></div><div id="learn-steps" class="mt-4 space-y-3"></div></article><article class="rounded-3xl border border-sky-100 bg-white p-5"><p class="text-sm font-semibold text-sky-700">示例与练习</p><div id="learn-examples" class="mt-4 space-y-3"></div><div id="learn-practice" class="mt-4 space-y-3"></div></article></section><section class="grid gap-4 lg:grid-cols-2"><article class="rounded-3xl border border-sky-100 bg-white p-5"><div class="flex items-center justify-between gap-3"><p class="text-sm font-semibold text-sky-700">小测验</p><span class="text-xs text-slate-500">答对 70% 即通过</span></div><div id="learn-quiz" class="mt-4 space-y-3"></div><button type="button" id="quiz-submit" class="mt-4 inline-flex min-h-11 w-full items-center justify-center rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white hover:bg-sky-800">提交测验</button><p id="quiz-result" class="mt-3 text-sm font-semibold"></p></article><article class="rounded-3xl border border-sky-100 bg-white p-5"><p class="text-sm font-semibold text-sky-700">完成检查</p><div id="learn-checkpoint" class="mt-3 space-y-2 text-sm leading-7 text-slate-700"></div><div id="learn-mistakes" class="mt-4 space-y-2 text-sm leading-7 text-slate-700"></div><button type="button" id="complete-lesson" class="mt-5 inline-flex min-h-11 w-full items-center justify-center rounded-xl bg-emerald-600 px-4 text-sm font-semibold text-white hover:bg-emerald-700">完成本节并保存进度</button><a id="next-lesson" class="mt-3 hidden text-center text-sm font-semibold text-sky-700 hover:text-sky-900" href="#">进入下一节 →</a></article></section><section class="rounded-3xl border border-slate-200 bg-slate-50 p-5"><div class="flex flex-col gap-3 sm:flex-row sm:items-center sm:justify-between"><div><p class="text-sm font-semibold text-slate-700">参考资料</p><div id="learn-references" class="mt-2 space-y-2"></div></div><button type="button" id="regenerate" class="inline-flex min-h-10 items-center justify-center rounded-xl border border-slate-300 bg-white px-4 text-sm font-semibold text-slate-700 hover:border-sky-400 hover:text-sky-700">换一版讲解</button></div></section>
      </div>
    </section>
  </section>
</main>)HTML";

    const std::string script = R"HTML(<script>(()=>{const courseId=)HTML" + jsString(courseId) + R"HTML(;const phaseIndex=)HTML" + jsString(phaseIndex) + R"HTML(;const topicIndex=)HTML" + jsString(topicIndex) + R"HTML(;const q=new URLSearchParams({courseId,phaseIndex,topicIndex});document.getElementById('learn-course-id').textContent=courseId||'未提供';document.getElementById('learn-phase-index').textContent=phaseIndex||'0';document.getElementById('learn-topic-index').textContent=topicIndex||'0';const loading=document.getElementById('learn-loading');const content=document.getElementById('learn-content');const title=document.getElementById('learn-content-title');const summary=document.getElementById('learn-content-summary');const concepts=document.getElementById('learn-key-concepts');const steps=document.getElementById('learn-steps');const examples=document.getElementById('learn-examples');const practice=document.getElementById('learn-practice');const quiz=document.getElementById('learn-quiz');const mistakes=document.getElementById('learn-mistakes');const references=document.getElementById('learn-references');const esc=(s)=>{const d=document.createElement('div');d.textContent=s??'';return d.innerHTML;};const pill=(text)=>'<span class="inline-flex rounded-full border border-sky-200 bg-sky-50 px-3 py-1 text-sm font-medium text-sky-800">'+esc(text)+'</span>';const list=(items,renderItem)=>Array.isArray(items)&&items.length?items.map(renderItem).join(''):'<p class="text-sm text-slate-500">暂无内容</p>';const renderStep=(step,index)=>'<div class="rounded-2xl border border-sky-100 bg-sky-50/50 p-4"><p class="text-xs font-semibold uppercase tracking-[0.16em] text-sky-700">Step '+(index+1)+'</p><h3 class="mt-2 font-semibold text-slate-950">'+esc(step.title||step.name||('步骤 '+(index+1)))+'</h3><p class="mt-2 text-sm leading-7 text-slate-600">'+esc(step.explanation||step.content||'')+'</p><div class="mt-3 grid gap-2 text-sm text-slate-700"><p><span class="font-semibold text-slate-900">示例：</span>'+esc(step.example||'')+'</p><p><span class="font-semibold text-slate-900">行动：</span>'+esc(step.action||'')+'</p><p><span class="font-semibold text-slate-900">检查：</span>'+esc(step.check||'')+'</p></div></div>';const renderExample=(item,index)=>'<div class="rounded-2xl border border-slate-200 bg-slate-50 p-4"><p class="text-sm font-semibold text-slate-900">'+esc(item.title||('示例 '+(index+1)))+'</p><pre class="mt-2 whitespace-pre-wrap rounded-xl bg-white p-3 text-sm leading-7 text-slate-700">'+esc(item.content||item.example||'')+'</pre></div>';const renderPractice=(item,index)=>'<div class="rounded-2xl border border-slate-200 bg-white p-4"><div class="flex flex-wrap items-center gap-2"><p class="font-semibold text-slate-900">'+esc(item.title||('练习 '+(index+1)))+'</p><span class="rounded-full bg-sky-50 px-3 py-1 text-xs font-semibold text-sky-700">'+esc(item.difficulty||'')+'</span></div><p class="mt-2 text-sm leading-7 text-slate-600">'+esc(item.task||item.content||'')+'</p><p class="mt-3 text-sm text-slate-700"><span class="font-semibold text-slate-900">检查：</span>'+esc(item.check||'')+'</p></div>';const renderQuiz=(item,index)=>'<div class="rounded-2xl border border-slate-200 bg-white p-4"><p class="text-sm font-semibold text-slate-900">题目 '+(index+1)+'</p><p class="mt-2 text-sm leading-7 text-slate-700">'+esc(item.question||'')+'</p><div class="mt-3 grid gap-2">'+(Array.isArray(item.options)?item.options.map((option,optionIndex)=>'<div class="rounded-xl border border-slate-200 bg-slate-50 px-3 py-2 text-sm text-slate-700">'+String.fromCharCode(65+optionIndex)+'. '+esc(option)+'</div>').join(''):'')+'</div><p class="mt-3 text-sm text-slate-600"><span class="font-semibold text-slate-900">解析：</span>'+esc(item.explanation||'')+'</p></div>';const renderReference=(item,index)=>'<a class="block rounded-2xl border border-sky-100 bg-sky-50/50 p-4 transition hover:border-sky-300 hover:bg-sky-50" href="'+esc(item.url||'#')+'" target="_blank" rel="noreferrer"><p class="text-sm font-semibold text-slate-900">'+esc(item.title||('参考资料 '+(index+1)))+'</p><p class="mt-1 text-sm text-slate-600">'+esc(item.source||item.type||'')+'</p><p class="mt-2 break-all text-xs text-sky-700">'+esc(item.url||'')+'</p></a>';fetch('/api/learn?'+q.toString()).then(async response=>{const data=await response.json();if(!response.ok) throw new Error(data.error||'加载失败');return data;}).then(data=>{title.textContent=data.title||data.courseTitle||'微课程';summary.textContent=data.summary||data.description||'';concepts.innerHTML=list(data.keyConcepts||data.concepts||[],pill);steps.innerHTML=list(data.lessonSteps||data.steps||[],renderStep);examples.innerHTML=list(data.examples||[],renderExample);practice.innerHTML=list(data.practice||[],renderPractice);quiz.innerHTML=list(data.quiz||[],renderQuiz);mistakes.innerHTML=(data.commonMistakes||[]).length?(data.commonMistakes||[]).map(item=>'<div class="rounded-xl border border-amber-100 bg-amber-50 px-3 py-2">'+esc(item)+'</div>').join(''):'<p class="text-sm text-slate-500">暂无内容</p>';references.innerHTML=list(data.references||[],renderReference);loading.classList.add('hidden');content.classList.remove('hidden');}).catch(error=>{loading.innerHTML='<div class="rounded-3xl border border-rose-100 bg-rose-50 p-6 text-center"><p class="font-semibold text-rose-700">学习内容暂时无法加载</p><p class="mt-2 text-sm text-slate-600">'+esc(error.message)+'</p><p class="mt-4 text-sm text-slate-500">请稍后重试，或先保留当前页面骨架等待数据接口接通。</p></div>';});})();</script>)HTML";
    return document("柳州市钢一中学2629班定制AI - 高中学习规划助手", body, script);
}

std::string renderLearnPage(const std::string& courseId, const std::string& goal, const std::string& mode,
                            const std::string& phaseIndex, const std::string& phaseName,
                            const std::string& topicIndex, const std::string& topic,
                            const std::string& anonymousId, const std::string& regenerate,
                            const std::string& forceLearn, const std::string& retry) {
#define script std::string script
    const std::string body = headerShell("learn") + R"HTML(
<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950"><section class="mx-auto w-full max-w-6xl px-4 py-6 sm:px-6"><div class="flex flex-col gap-4 lg:flex-row lg:items-start lg:justify-between"><div><p class="text-sm font-semibold text-sky-700">微课程 · 学习中</p><h1 id="learn-title" class="mt-2 text-3xl font-bold tracking-tight sm:text-4xl">正在加载学习内容</h1><p id="learn-summary" class="mt-3 max-w-3xl leading-7 text-slate-600">完成理解、练习和检查，让每一节课都留下真实进度。</p></div><aside class="grid gap-2 rounded-3xl border border-sky-100 bg-white p-4 shadow-sm sm:min-w-72"><div class="flex justify-between gap-3 text-sm"><span>阶段</span><b id="learn-phase-name">加载中</b></div><div class="flex justify-between gap-3 text-sm"><span>主题</span><b id="learn-topic-name">加载中</b></div><div class="h-2 overflow-hidden rounded-full bg-slate-200"><i id="learn-bar" class="soft-progress-fill block h-full w-0 rounded-full bg-sky-600"></i></div><small id="learn-save-status">准备开始</small></aside></div><section class="mt-6 rounded-3xl border border-sky-100 bg-white p-4 shadow-sm sm:p-6"><div id="learn-loading" class="p-8 text-center"><div class="mx-auto h-10 w-10 animate-spin rounded-full border-4 border-sky-100 border-t-sky-600"></div><p class="mt-4 font-semibold">正在准备这节课</p></div><div id="learn-content" class="hidden space-y-6"><section class="grid gap-4 lg:grid-cols-[1.3fr_0.9fr]"><article class="rounded-2xl border border-sky-100 bg-sky-50/70 p-5"><p class="text-sm font-semibold text-sky-700">本课概览</p><h2 id="learn-content-title" class="mt-2 text-2xl font-bold"></h2><p id="learn-content-summary" class="mt-3 leading-7 text-slate-600"></p></article><article class="rounded-2xl border border-slate-200 p-5"><p class="text-sm font-semibold">关键概念</p><div id="learn-key-concepts" class="mt-3 flex flex-wrap gap-2"></div></article></section><section class="grid gap-4 lg:grid-cols-2"><article class="rounded-2xl border border-sky-100 p-5"><div class="flex justify-between"><p class="text-sm font-semibold text-sky-700">理解步骤</p><small id="step-count"></small></div><div id="learn-steps" class="mt-4 space-y-3"></div></article><article class="rounded-2xl border border-sky-100 p-5"><p class="text-sm font-semibold text-sky-700">示例与练习</p><div id="learn-examples" class="mt-4 space-y-3"></div><div id="learn-practice" class="mt-4 space-y-3"></div></article></section><section class="grid gap-4 lg:grid-cols-2"><article class="rounded-2xl border border-sky-100 p-5"><div class="flex justify-between"><p class="text-sm font-semibold text-sky-700">小测验</p><small>答对 70% 即通过</small></div><div id="learn-quiz" class="mt-4 space-y-3"></div><button id="quiz-submit" type="button" class="mt-4 min-h-11 w-full rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white">提交测验</button><p id="quiz-result" class="mt-3 text-sm font-semibold"></p></article><article class="rounded-2xl border border-sky-100 p-5"><p class="text-sm font-semibold text-sky-700">完成检查</p><div id="learn-checkpoint" class="mt-3 space-y-2 text-sm leading-7"></div><div id="learn-mistakes" class="mt-4 space-y-2 text-sm leading-7"></div><button id="complete-lesson" type="button" class="mt-5 min-h-11 w-full rounded-xl bg-emerald-600 px-4 text-sm font-semibold text-white">完成本节并保存进度</button></article></section><section class="rounded-2xl border border-slate-200 bg-slate-50 p-5"><div class="flex items-center justify-between gap-3"><p class="text-sm font-semibold">参考资料</p><button id="regenerate" type="button" class="rounded-xl border border-slate-300 bg-white px-4 py-2 text-sm font-semibold">换一版讲解</button></div><div id="learn-references" class="mt-3 space-y-2"></div></section></div></section></section></main>)HTML";
    const script = R"HTML(<script>(()=>{const C=)HTML"+jsString(courseId)+R"HTML(,P=)HTML"+jsString(phaseIndex)+R"HTML(,T=)HTML"+jsString(topicIndex)+R"HTML(,$=id=>document.getElementById(id),E=v=>{const d=document.createElement('div');d.textContent=v??'';return d.innerHTML},A=v=>Array.isArray(v)?v:[],K='gy:'+C+':'+P+':'+T,S=JSON.parse(localStorage.getItem(K)||'{}');let L;const list=(v,f)=>A(v).length?A(v).map(f).join(''):'<p class="text-sm text-slate-500">暂无内容</p>',post=(u,b)=>fetch(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(b)}).then(r=>r.json());function show(x){L=x;$('learn-title').textContent=x.title||'本节学习';$('learn-summary').textContent=x.summary||'';$('learn-content-title').textContent=x.title||'';$('learn-content-summary').textContent=x.summary||'';$('learn-phase-name').textContent=x.phaseName||'当前阶段';$('learn-topic-name').textContent=x.topicTitle||x.title||'当前主题';$('learn-key-concepts').innerHTML=list(x.keyConcepts,v=>'<span class="plan-pill">'+E(v)+'</span>');$('learn-steps').innerHTML=list(x.lessonSteps,(v,i)=>'<article class="rounded-2xl border border-sky-100 bg-sky-50/50 p-4"><button type="button" class="step-toggle mr-3 h-6 w-6 rounded-full border-2 border-sky-300" data-i="'+i+'">'+(S.steps?.[i]?'✓':'')+'</button><b>'+E(v.title)+'</b><p class="mt-2 text-sm leading-7">'+E(v.explanation)+'</p><p class="mt-1 text-sm">行动：'+E(v.action)+'</p><p class="mt-1 text-sm">检查：'+E(v.check)+'</p></article>');$('step-count').textContent=A(x.lessonSteps).filter((_,i)=>S.steps?.[i]).length+' / '+A(x.lessonSteps).length+' 已理解';$('learn-examples').innerHTML=list(x.examples,v=>'<details class="rounded-2xl border border-slate-200 p-4"><summary class="cursor-pointer font-semibold">'+E(v.title)+'</summary><p class="mt-3 whitespace-pre-wrap text-sm leading-7">'+E(v.content)+'</p></details>');$('learn-practice').innerHTML=list(x.practice,(v,i)=>'<label class="flex gap-3 rounded-2xl border border-slate-200 p-4"><input class="practice-toggle accent-sky-600" type="checkbox" data-i="'+i+'" '+(S.practice?.[i]?'checked':'')+'><span><b>'+E(v.title)+'</b><span class="mt-1 block text-sm">'+E(v.task)+'</span></span></label>');$('learn-quiz').innerHTML=list(x.quiz,(v,i)=>'<fieldset class="rounded-2xl border border-slate-200 p-4"><legend class="text-sm font-semibold">'+(i+1)+'. '+E(v.question)+'</legend>'+list(v.options,(o,j)=>'<label class="mt-2 flex gap-2 text-sm"><input type="radio" name="q'+i+'" value="'+j+'">'+E(o)+'</label>')+'</fieldset>');$('learn-checkpoint').innerHTML=list(x.checkpoint,v=>'<p>✓ '+E(v)+'</p>');$('learn-mistakes').innerHTML=list(x.commonMistakes,v=>'<p>• '+E(v)+'</p>');$('learn-references').innerHTML=list(x.references,v=>'<a class="block text-sm text-sky-700" target="_blank" rel="noreferrer" href="'+(/^https?:\/\//i.test(v.url||'')?E(v.url):'#')+'">'+E(v.title||v.source)+'</a>');$('learn-loading').classList.add('hidden');$('learn-content').classList.remove('hidden')}document.addEventListener('click',e=>{const b=e.target.closest('.step-toggle');if(!b)return;const i=+b.dataset.i;S.steps=S.steps||{};S.steps[i]=!S.steps[i];b.textContent=S.steps[i]?'✓':'';$('step-count').textContent=A(L.lessonSteps).filter((_,j)=>S.steps?.[j]).length+' / '+A(L.lessonSteps).length+' 已理解';$('learn-bar').style.width=Math.min(90,Math.round(A(L.lessonSteps).filter((_,j)=>S.steps?.[j]).length/A(L.lessonSteps).length*100))+'%';localStorage.setItem(K,JSON.stringify(S));post('/api/learning-step-progress',{courseId:C,phaseIndex:+P,stepIndex:i,stepTitle:L.lessonSteps[i].title,status:S.steps[i]?'understood':'unset'})});document.addEventListener('change',e=>{if(!e.target.matches('.practice-toggle'))return;S.practice=S.practice||{};S.practice[e.target.dataset.i]=e.target.checked;localStorage.setItem(K,JSON.stringify(S))});$('quiz-submit').onclick=()=>{let n=0;A(L.quiz).forEach((v,i)=>{const q=document.querySelector('input[name="q'+i+'"]:checked');if(q&&+q.value===v.answerIndex)n++});$('quiz-result').textContent=L.quiz.length?'得分 '+n+' / '+L.quiz.length+(n/L.quiz.length>=.7?'，已通过':'，再看一遍讲解后重试'):''};$('complete-lesson').onclick=()=>post('/api/learn/progress',{courseId:C,phaseIndex:+P,topicIndex:+T,phaseName:L.phaseName||'',topicTitle:L.topicTitle||L.title,status:'completed',lastVisitedUrl:location.pathname+location.search,lastPageType:'learn',lastPhaseIndex:+P,lastTopicIndex:+T}).then(r=>{$('learn-save-status').textContent=r.ok?'本节已完成，进度已保存':'保存失败，请稍后重试';$('complete-lesson').disabled=!!r.ok});$('regenerate').onclick=()=>location.search=new URLSearchParams({courseId:C,phaseIndex:P,topicIndex:T,regenerate:'1'});fetch('/api/learn?courseId='+encodeURIComponent(C)+'&phaseIndex='+P+'&topicIndex='+T).then(r=>r.json()).then(show).catch(()=>{$('learn-loading').innerHTML='<p class="text-rose-700">学习内容加载失败，请稍后重试。</p>'})})()</script>)HTML";
    #undef script
    return document("学习中 - 钢一定制AI", body, script);
}

std::string renderProgressPageLegacy(const std::string& courseId) {
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

std::string renderProgressPage(const std::string& courseId, const std::string& anonymousId,
                               const nlohmann::json& data) {
    const auto str = [](const nlohmann::json& v) { return v.is_string() ? v.get<std::string>() : std::string(); };
    const auto num = [](const nlohmann::json& v) { return v.is_number() ? v.get<int>() : 0; };
    const bool ready = data.value("ready", false);
    std::string body;
    if (!ready) {
        body = headerShell("progress") + R"HTML(<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950"><section class="mx-auto flex min-h-[60vh] w-full max-w-2xl items-center justify-center px-4 py-12 sm:px-6"><div class="w-full rounded-3xl border border-slate-200 bg-white p-8 text-center shadow-sm shadow-sky-900/5"><h1 class="text-2xl font-semibold text-slate-950">学习进度</h1><p class="mt-3 text-sm leading-6 text-slate-600">请从课程或计划页进入，系统会按阶段汇总你的学习进度。</p><a class="mt-5 inline-flex min-h-11 items-center justify-center rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white" href="/">去首页生成课程</a></div></section></main>)HTML";
        return document("学习进度 - 钢一定制AI", body);
    }
    const int overall = num(data.value("overallPercent", 0));
    const int completed = num(data.value("completedCount", 0));
    const int total = num(data.value("totalCount", 0));
    const std::string updated = str(data.value("updatedAt", ""));
    const bool hasBp = data.value("hasBreakpoint", false);
    const std::string bpUrl = str(data.value("lastVisitedUrl", ""));
    const std::string bpText = str(data.value("breakpointText", ""));
    std::string contLink;
    if (hasBp && !bpUrl.empty()) contLink = R"HTML(<a class="inline-flex min-h-11 items-center justify-center rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white transition hover:bg-sky-800" href=")HTML" + htmlEscape(bpUrl) + R"HTML(">继续学习 →</a>)HTML";

    std::string phaseCards;
    for (const auto& ph : data.value("phases", nlohmann::json::array())) {
        const std::string name = str(ph.value("name", ""));
        const int pindex = num(ph.value("index", 0));
        const int ptotal = num(ph.value("total", 0));
        const int pdone = num(ph.value("completed", 0));
        const int pct = num(ph.value("percent", 0));
        const std::string href = str(ph.value("href", ""));
        const std::string status = str(ph.value("status", "not_started"));
        const std::string statusLabel = status == "completed" ? "已完成" : status == "in_progress" ? "学习中" : "未开始";
        const std::string pillCls = status == "completed" ? "bg-emerald-100 text-emerald-700" : status == "in_progress" ? "bg-sky-100 text-sky-800" : "bg-slate-100 text-slate-600";
        const std::string barCls = status == "completed" ? "bg-emerald-500" : "bg-sky-600";
        phaseCards += R"HTML(<article class="rounded-3xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5">
  <div class="flex flex-wrap items-center justify-between gap-2"><div><p class="text-xs font-semibold text-sky-700">第 )HTML" + std::to_string(pindex) + R"HTML( 阶段</p><h2 class="mt-1 break-words text-xl font-semibold text-slate-950">)HTML" + htmlEscape(name) + R"HTML(</h2></div><span class="shrink-0 rounded-full px-2.5 py-1 text-xs font-semibold )HTML" + pillCls + R"HTML(">)HTML" + statusLabel + R"HTML(</span></div>
  <div class="mt-4 flex items-center justify-between text-sm text-slate-600"><span>完成进度</span><span class="font-semibold text-slate-900">)HTML" + std::to_string(pct) + R"HTML(% · )HTML" + std::to_string(pdone) + R"HTML(/ )HTML" + std::to_string(ptotal) + R"HTML( 节</span></div>
  <div class="mt-2 h-2.5 overflow-hidden rounded-full bg-slate-100"><div class="h-full rounded-full )HTML" + barCls + R"HTML(" style="width: )HTML" + std::to_string(pct) + R"HTML(%"></div></div>
  <a class="mt-4 inline-flex min-h-10 items-center justify-center rounded-xl bg-sky-700 px-3 text-sm font-semibold text-white transition hover:bg-sky-800" href=")HTML" + htmlEscape(href) + R"HTML(">进入本阶段 →</a>
</article>)HTML";
    }
    body = headerShell("progress") + R"HTML(<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950"><section class="mx-auto w-full max-w-6xl px-4 py-6 sm:px-6">
  <div class="flex flex-wrap items-end justify-between gap-3"><div><p class="text-sm font-semibold text-sky-700">学习进度</p><h1 class="mt-2 break-words text-3xl font-bold tracking-tight sm:text-4xl">)HTML" + htmlEscape(data.value("courseTitle", courseId)) + R"HTML(</h1><p class="mt-2 text-sm text-slate-600">)HTML" + (updated.empty() ? std::string("每一次理解、练习和完成都会汇总到这里。") : "最近同步：" + htmlEscape(updated)) + R"HTML(</p></div>)HTML" + contLink + R"HTML(</div>
  <section class="mt-6 grid gap-4 md:grid-cols-3">
    <article class="rounded-2xl border border-sky-100 bg-white p-5"><p class="text-sm text-slate-500">总体完成度</p><b class="mt-2 block text-4xl text-sky-700">)HTML" + std::to_string(overall) + R"HTML(%</b><div class="mt-4 h-3 overflow-hidden rounded-full bg-slate-200"><i class="block h-full rounded-full bg-sky-600" style="width: )HTML" + std::to_string(overall) + R"HTML(%"></i></div></article>
    <article class="rounded-2xl border border-sky-100 bg-white p-5"><p class="text-sm text-slate-500">已完成项目</p><b class="mt-2 block text-4xl">)HTML" + std::to_string(completed) + R"HTML(</b><p class="mt-2 text-sm text-slate-500">理解、卡片与任务</p></article>
    <article class="rounded-2xl border border-sky-100 bg-white p-5"><p class="text-sm text-slate-500">课程项目总数</p><b class="mt-2 block text-4xl">)HTML" + std::to_string(total) + R"HTML(</b><p class="mt-2 text-sm text-slate-500">按最新快照估算</p></article>
  </section>
  )HTML" + (hasBp ? R"HTML(<section class="mt-6 rounded-2xl border border-sky-100 bg-white p-5"><p class="text-sm font-semibold text-slate-700">学习断点</p><p class="mt-2 rounded-xl bg-slate-50 p-4 text-sm text-slate-600">)HTML" + htmlEscape(bpText) + R"HTML(</p></section>)HTML" : std::string()) + R"HTML(
  <section class="mt-6"><div class="mb-3"><h2 class="text-xl font-semibold">分阶段进度</h2></div><div class="grid gap-4 md:grid-cols-2">)HTML" + (phaseCards.empty() ? R"HTML(<p class="text-sm text-slate-500 md:col-span-2">暂无可展示的阶段，先到课程里生成学习计划。</p>)HTML" : phaseCards) + R"HTML(</div></section>
</section></main>)HTML";
    return document("学习进度 - 钢一定制AI", body);
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

std::string renderMyCoursesPage(const nlohmann::json& data) {
    const auto str = [](const nlohmann::json& v) { return v.is_string() ? v.get<std::string>() : std::string(); };
    const auto num = [](const nlohmann::json& v) { return v.is_number() ? v.get<int>() : 0; };
    std::string cardsHtml;
    for (const auto& c : data.value("courses", nlohmann::json::array())) {
        const std::string title = str(c.value("title", ""));
        const std::string goal = str(c.value("goal", ""));
        const std::string source = str(c.value("source", "ai"));
        const std::string createdAt = str(c.value("createdAt", ""));
        const int pct = num(c.value("overallPercent", 0));
        const std::string status = str(c.value("status", "not_started"));
        const std::string statusLabel = status == "completed" ? "已完成" : status == "in_progress" ? "学习中" : "未开始";
        const std::string statusCls = status == "completed" ? "bg-emerald-100 text-emerald-700" : status == "in_progress" ? "bg-sky-100 text-sky-800" : "bg-slate-100 text-slate-600";
        const std::string learnHref = str(c.value("learnHref", "#"));
        const std::string progressHref = str(c.value("progressHref", "#"));
        const std::string sourceLabel = source == "fallback" ? "应急内容" : (source == "mock" || source == "template" ? "示例" : "AI 生成");
        cardsHtml += R"HTML(<article class="rounded-3xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5 sm:p-6">
  <div class="flex flex-wrap items-center justify-between gap-2"><div class="min-w-0"><p class="text-xs font-semibold text-sky-700">)HTML" + htmlEscape(sourceLabel) + R"HTML(</p><h2 class="mt-1 break-words text-xl font-semibold text-slate-950">)HTML" + htmlEscape(title) + R"HTML(</h2></div><span class="shrink-0 rounded-full px-2.5 py-1 text-xs font-semibold )HTML" + statusCls + R"HTML(">)HTML" + statusLabel + R"HTML(</span></div>
  <p class="mt-3 break-words text-sm leading-7 text-slate-600">)HTML" + htmlEscape(goal) + R"HTML(</p>
  <div class="mt-4 flex items-center justify-between text-xs text-slate-500"><span>完成度</span><span class="font-semibold text-slate-700">)HTML" + std::to_string(pct) + R"HTML(%</span></div>
  <div class="mt-1 h-2 overflow-hidden rounded-full bg-slate-200"><div class="h-full rounded-full bg-sky-600" style="width: )HTML" + std::to_string(pct) + R"HTML(%"></div></div>
  <div class="mt-4 flex flex-wrap gap-2"><a class="inline-flex min-h-10 items-center justify-center rounded-xl bg-sky-700 px-3 text-sm font-semibold text-white transition hover:bg-sky-800" href=")HTML" + htmlEscape(learnHref) + R"HTML(">继续学习</a><a class="inline-flex min-h-10 items-center justify-center rounded-xl border border-sky-200 bg-sky-50 px-3 text-sm font-semibold text-sky-800 transition hover:bg-sky-100" href=")HTML" + htmlEscape(progressHref) + R"HTML(">查看进度</a></div>
  )HTML" + (createdAt.empty() ? "" : R"HTML(<p class="mt-3 text-xs text-slate-400">创建于 )HTML" + htmlEscape(createdAt) + "</p>") + R"HTML(
</article>)HTML";
    }
    const std::string body = headerShell("my-courses") + R"HTML(
<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950">
  <section class="mx-auto w-full max-w-6xl px-4 py-6 sm:px-6">
    <div class="flex flex-wrap items-end justify-between gap-3">
      <div><p class="text-sm font-semibold text-sky-700">我的课程</p><h1 class="mt-2 text-3xl font-bold tracking-tight sm:text-4xl">我的课堂</h1><p class="mt-3 text-base leading-7 text-slate-600">继续学习已生成的课程，或生成一个新的学习计划。</p></div>
      <a class="inline-flex min-h-11 items-center justify-center rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white transition hover:bg-sky-800" href="/">+ 生成新课程</a>
    </div>
    <section class="mt-6 grid gap-4 lg:grid-cols-2">)HTML"
        + (cardsHtml.empty() ? R"HTML(<div class="lg:col-span-2 rounded-3xl border border-dashed border-slate-200 bg-white p-8 text-center"><p class="text-slate-600">还没有课程。去首页输入你的学习目标，先生成一份课程计划。</p><a class="mt-4 inline-flex min-h-11 items-center justify-center rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white" href="/">去首页生成课程</a></div>)HTML" : cardsHtml)
        + R"HTML(</section>
  </section>
</main>)HTML";
    return document("我的课程 - 钢一定制AI", body);
}

std::string renderPhasePage(const std::string& courseId, const std::string& anonymousId,
                            const std::string& goal, const std::string& mode,
                            const std::string& phaseIndex, const std::string& phaseName,
                            const nlohmann::json& plan, const nlohmann::json& cardStatus) {
    const auto str = [](const nlohmann::json& v) {
        if (v.is_string()) return v.get<std::string>();
        return std::string();
    };
    const auto arr = [](const nlohmann::json& v) { return v.is_array(); };
    const auto q = [&](const std::string& key) {
        return std::string("&") + key + "=";
    };
    const std::string anonQ = anonymousId.empty() ? "" : q("anonymousId") + urlEncode(anonymousId);

    // 解析阶段与主题（MockPlan：roadmap[] / courseStructure[]）
    int index = 1;
    try { if (!phaseIndex.empty()) index = std::max(1, std::stoi(phaseIndex)); } catch (...) {}
    const bool hasPlan = plan.is_object() && arr(plan.value("roadmap", nlohmann::json())) &&
        arr(plan.value("courseStructure", nlohmann::json()));
    std::string stageTitle = phaseName.empty() ? "阶段" + std::to_string(index) : phaseName;
    std::string stageGoal, stageWhy, stageOutput, stageDuration, suitable;
    std::vector<std::pair<std::string, int>> topics;  // title, topicIndex
    if (hasPlan) {
        const auto roadmap = plan["roadmap"];
        const auto courseStructure = plan["courseStructure"];
        const nlohmann::json* stage = nullptr;
        if (index <= static_cast<int>(roadmap.size())) stage = &roadmap[index - 1];
        else for (const auto& s : roadmap) if (str(s.value("name", "")) == stageTitle) { stage = &s; break; }
        if (stage) {
            if (stage->contains("name") && (*stage)["name"].is_string()) stageTitle = (*stage)["name"].get<std::string>();
            stageGoal = str(stage->value("goal", stage->value("description", "")));
            stageWhy = str(stage->value("why", ""));
            stageOutput = str(stage->value("output", ""));
            stageDuration = str(stage->value("duration", ""));
        }
        const nlohmann::json* topicsSrc = nullptr;
        if (index <= static_cast<int>(courseStructure.size())) {
            const auto& cs = courseStructure[index - 1];
            if (arr(cs.value("topics", nlohmann::json()))) topicsSrc = &cs["topics"];
        }
        if (!topicsSrc && stage && arr(stage->value("tasks", nlohmann::json()))) topicsSrc = &(*stage)["tasks"];
        if (topicsSrc) {
            int n = 0;
            for (const auto& t : *topicsSrc) { ++n; topics.emplace_back(str(t), n); }
        }
        if (topics.empty() && stage) { stageTitle = str(stage->value("name", stageTitle)); }
    }
    if (!hasPlan) {
        // 无课程快照：给出“未找到/未生成”降级态
        const std::string back = courseId.empty()
            ? "/plan?goal=" + urlEncode(goal) + "&mode=" + urlEncode(mode) + anonQ
            : "/plan?courseId=" + urlEncode(courseId) + anonQ;
        const std::string body = headerShell("learn") + R"HTML(
<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950">
  <section class="mx-auto flex min-h-[70vh] w-full max-w-3xl items-center justify-center px-4 py-12 sm:px-6">
    <div class="rounded-3xl border border-amber-100 bg-white p-8 text-center shadow-sm shadow-sky-900/5">
      <h1 class="text-3xl font-semibold tracking-tight text-slate-950">阶段内容暂未生成完成</h1>
      <p class="mt-3 text-base leading-7 text-slate-600">当前课程结构还不完整，暂时无法生成阶段讲解、任务、课件和知识结构。请回到课程页重新生成或刷新后重试。</p>
      <div class="mt-6 flex flex-col gap-3 sm:flex-row sm:justify-center">
        <a class="inline-flex min-h-12 items-center justify-center rounded-xl bg-sky-700 px-5 text-sm font-semibold text-white transition hover:bg-sky-800" href=")HTML" + back + R"HTML(">返回课程大纲</a>
      </div>
    </div>
  </section>
</main>)HTML";
        return document("阶段 - 钢一定制AI", body);
    }

    // 主题状态
    int completed = 0;
    for (const auto& [title, no] : topics) {
        if (cardStatus.value(std::to_string(no), "") == "completed") ++completed;
    }
    const int total = static_cast<int>(topics.size());
    const int percent = total > 0 ? static_cast<int>(std::lround(completed * 100.0 / total)) : 0;
    const bool done = total > 0 && completed >= total;
    const std::string modeLabel = mode == "lite" ? "快速规划" : "深度 钢一定制AI 规划";
    const std::string modeDesc = mode == "lite" ? "轻量学习课程：阶段内容更聚焦，保留关键讲解和练习。" : "系统学习课程：阶段讲解、任务、课件和资料更完整。";
    if (suitable.empty()) suitable = "适合正在学习「" + goal + "」并准备完成「" + stageTitle + "」阶段任务的学习者。";
    const std::string backHref = courseId.empty()
        ? "/plan?goal=" + urlEncode(goal) + "&mode=" + urlEncode(mode) + anonQ
        : "/plan?courseId=" + urlEncode(courseId) + anonQ;
    const std::string firstHref = courseId.empty()
        ? "/learn?goal=" + urlEncode(goal) + "&mode=" + urlEncode(mode)
            + "&phaseIndex=" + std::to_string(index) + "&phaseName=" + urlEncode(stageTitle) + anonQ
        : "/learn?courseId=" + urlEncode(courseId)
            + "&phaseIndex=" + std::to_string(index) + "&phaseName=" + urlEncode(stageTitle)
            + "&topicIndex=1&goal=" + urlEncode(goal) + "&mode=" + urlEncode(mode) + anonQ;
    const std::string askHref = "/ask?goal=" + urlEncode(goal) + "&mode=" + urlEncode(mode) + anonQ;

    std::string topicsHtml;
    for (const auto& [title, no] : topics) {
        const std::string status = cardStatus.value(std::to_string(no), "not_started");
        const std::string statusLabel = status == "completed" ? "已完成" : status == "in_progress" ? "学习中" : "未开始";
        const std::string statusCls = status == "completed" ? "border-emerald-100 bg-emerald-50" : status == "in_progress" ? "border-sky-100 bg-sky-50" : "border-slate-200 bg-slate-50";
        const std::string badgeCls = status == "completed" ? "bg-emerald-100 text-emerald-700" : status == "in_progress" ? "bg-sky-100 text-sky-800" : "bg-white text-slate-600";
        const std::string hint = status == "completed" ? "已完成，可以复习巩固。" : status == "in_progress" ? "当前学习中，继续完成本节。" : "还未开始，从这里进入微课程。";
        std::string href = courseId.empty()
            ? "/learn?goal=" + urlEncode(goal) + "&mode=" + urlEncode(mode)
                + "&phaseIndex=" + std::to_string(index) + "&phaseName=" + urlEncode(stageTitle)
                + "&topicIndex=" + std::to_string(no) + "&topic=" + urlEncode(title) + anonQ
            : "/learn?courseId=" + urlEncode(courseId)
                + "&phaseIndex=" + std::to_string(index) + "&phaseName=" + urlEncode(stageTitle)
                + "&topicIndex=" + std::to_string(no) + "&topic=" + urlEncode(title)
                + "&goal=" + urlEncode(goal) + "&mode=" + urlEncode(mode) + anonQ;
        topicsHtml += R"HTML(<article data-topic=")HTML" + htmlEscape(title) + R"HTML(" class="interactive-card rounded-2xl border p-4 )HTML" + statusCls + R"HTML(">
          <div class="flex items-start justify-between gap-3">
            <div><p class="text-xs font-semibold text-sky-700">第 )HTML" + std::to_string(no) + R"HTML( 节</p><h3 class="mt-1 break-words font-semibold text-slate-950">)HTML" + htmlEscape(title) + R"HTML(</h3><p class="mt-2 text-sm text-slate-600">)HTML" + hint + R"HTML(</p></div>
            <span class="shrink-0 rounded-full px-2.5 py-1 text-xs font-semibold )HTML" + badgeCls + R"HTML(">)HTML" + statusLabel + R"HTML(</span>
          </div>
          <a class="mt-4 inline-flex min-h-10 items-center justify-center gap-2 rounded-xl bg-white px-3 text-sm font-semibold text-sky-800 ring-1 ring-sky-100 transition hover:bg-sky-100" href=")HTML" + href + R"HTML(">学习这一节</a>
        </article>)HTML";
    }

    const std::string progressState = done ? "本阶段已完成" : (completed > 0 ? "当前阶段进度" : "本阶段未开始");
    const std::string primaryLabel = done ? "复习本阶段" : (completed > 0 ? "继续本阶段" : "开始本阶段学习");
    const std::string body = headerShell("learn") + R"HTML(
<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950">
  <div class="mx-auto w-full max-w-6xl space-y-6 px-4 py-8 sm:px-6 lg:px-8 lg:py-10 xl:max-w-7xl">
    <section class="rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-8">
      <a class="inline-flex items-center gap-2 rounded-full border border-slate-200 bg-white px-3 py-2 text-sm font-semibold text-slate-700 transition hover:border-sky-200 hover:bg-sky-50 hover:text-sky-800" href=")HTML" + backHref + R"HTML(">← 返回学习方案</a>
      <div class="mt-8 grid min-w-0 gap-6 lg:grid-cols-[minmax(0,1fr)_300px] lg:items-end">
        <div>
          <div class="mb-4 inline-flex items-center gap-2 rounded-full bg-sky-50 px-3 py-2 text-sm font-medium text-sky-800">第 )HTML" + std::to_string(index) + R"HTML( 阶段详情</div>
          <h1 class="break-words text-2xl font-semibold tracking-tight text-slate-950 sm:text-4xl lg:text-5xl">)HTML" + htmlEscape(stageTitle) + R"HTML(</h1>
          <p class="mt-4 max-w-3xl break-words text-base leading-8 text-slate-600 sm:text-lg">针对「)HTML" + htmlEscape(goal.empty() ? "你的目标" : goal) + R"HTML(」的阶段学习计划</p>
          <div class="mt-4 rounded-2xl border border-sky-100 bg-white/80 p-4">
            <div class="flex flex-wrap items-center justify-between gap-3 text-sm font-semibold text-slate-700"><span>)HTML" + progressState + R"HTML(</span><span>)HTML" + std::to_string(percent) + R"HTML(% · )HTML" + std::to_string(completed) + R"HTML(/ )HTML" + std::to_string(total) + R"HTML( 节</span></div>
            <div class="mt-3 h-2.5 overflow-hidden rounded-full bg-slate-100"><div class="h-full rounded-full bg-sky-700" style="width: )HTML" + std::to_string(percent) + R"HTML(%"></div></div>
          </div>
          <div class="mt-5 max-w-full rounded-2xl border border-sky-100 bg-sky-50 px-4 py-3 text-sm shadow-sm sm:w-fit">
            <p class="font-semibold text-sky-800">当前模式：)HTML" + modeLabel + R"HTML(</p>
            <p class="mt-1 leading-6 text-slate-600">)HTML" + modeDesc + R"HTML(</p>
          </div>
        </div>
        <div class="grid min-w-0 gap-3 md:grid-cols-2 lg:grid-cols-1">
          <a class="inline-flex min-h-12 items-center justify-center gap-2 rounded-2xl bg-sky-700 px-4 text-sm font-semibold text-white transition hover:bg-sky-800" href=")HTML" + firstHref + R"HTML(">)HTML" + primaryLabel + R"HTML(</a>
          <a class="inline-flex min-h-12 items-center justify-center gap-2 rounded-2xl border border-sky-200 bg-sky-50 px-4 text-sm font-semibold text-sky-800 transition hover:bg-sky-100" href=")HTML" + askHref + R"HTML(">问 钢一定制AI</a>
        </div>
      </div>
    </section>
    <section class="grid gap-3 sm:grid-cols-2 lg:grid-cols-4">
      <div class="rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-5"><p class="text-sm font-semibold text-sky-700">阶段名称</p><p class="mt-2 break-words text-lg font-semibold text-slate-950">)HTML" + htmlEscape(stageTitle) + R"HTML(</p></div>
      <div class="rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-5"><p class="text-sm font-semibold text-sky-700">当前学习目标</p><p class="mt-2 break-words text-lg font-semibold text-slate-950">)HTML" + htmlEscape(stageGoal) + R"HTML(</p></div>
      <div class="rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-5"><p class="text-sm font-semibold text-sky-700">推荐学习周期</p><p class="mt-2 break-words text-lg font-semibold text-slate-950">)HTML" + (stageDuration.empty() ? std::string("按课程安排") : htmlEscape(stageDuration)) + R"HTML(</p></div>
      <div class="rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-5"><p class="text-sm font-semibold text-sky-700">适合人群</p><p class="mt-2 break-words text-sm leading-6 text-slate-600">)HTML" + htmlEscape(suitable) + R"HTML(</p></div>
    </section>
    <section class="rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-8">
      <div class="mb-6"><p class="text-sm font-semibold text-sky-700">阶段概览</p><h2 class="mt-2 text-2xl font-semibold tracking-tight text-slate-950">阶段目标</h2>
        <p class="mt-3 break-words leading-7 text-slate-600">)HTML" + htmlEscape(stageGoal) + R"HTML(</p>
        <p class="mt-4 break-words rounded-2xl bg-sky-50 p-4 text-sm leading-6 text-sky-900">为什么先学：)HTML" + htmlEscape(stageWhy) + R"HTML(</p>
        <p class="mt-3 break-words rounded-2xl bg-slate-50 p-4 text-sm leading-6 text-slate-700">阶段产出：)HTML" + htmlEscape(stageOutput) + R"HTML(</p>
      </div>
      <div class="grid gap-3 md:grid-cols-2">
)HTML" + topicsHtml + R"HTML(
      </div>
    </section>
    <section class="rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-8">
      <div class="mb-6"><p class="text-sm font-semibold text-sky-700">阶段展开</p><h2 class="mt-2 text-2xl font-semibold tracking-tight text-slate-950">分步讲解与任务</h2><p class="mt-3 break-words text-sm leading-6 text-slate-600">系统会为这个阶段生成讲解步骤、任务练习与常见错误提醒。</p></div>
      <div id="phase-expansion-body">)HTML" + loadingSpinner("正在整理阶段讲解", "正在生成步骤、任务与检查点…") + R"HTML(</div>
    </section>
  </div>
</main>)HTML";
    // 阶段展开：POST /api/phase-expansion 生成讲解/任务/检查点；失败展示降级态（可重试）
    const std::string script = R"HTML(<script>(()=>{
const C=)HTML"+jsString(courseId)+R"HTML(,A=)HTML"+jsString(anonymousId)+R"HTML(,G=)HTML"+jsString(goal)+R"HTML(,M=)HTML"+jsString(mode)+R"HTML(,P=)HTML"+std::to_string(index)+R"HTML(,N=)HTML"+jsString(stageTitle)+R"HTML(;
const esc=v=>{const d=document.createElement('div');d.textContent=v??'';return d.innerHTML};
const topics=Array.from(document.querySelectorAll('[data-topic]')).map(e=>e.getAttribute('data-topic')||'');
const box=document.getElementById('phase-expansion-body');
const arr=v=>Array.isArray(v)?v:[];
const block=(label,rows)=>rows.length?('<div class="mt-5"><p class="text-xs font-semibold uppercase tracking-[0.16em] text-sky-700">'+label+'</p><div class="mt-2 grid gap-2">'+rows.join('')+'</div></div>'):'';
const pills=items=>items.length?('<div class="mt-2 flex flex-wrap gap-2">'+items.map(x=>'<span class="inline-flex rounded-full border border-amber-100 bg-amber-50 px-3 py-1 text-sm text-amber-800">'+esc(x)+'</span>').join('')+'</div>'):'';
function render(x){
 const parts=[];
 if(x.objective)parts.push('<p class="mt-1 break-words leading-7 text-slate-600"><b>阶段目标：</b>'+esc(x.objective)+'</p>');
 if(x.overview)parts.push('<div class="rounded-2xl bg-sky-50 p-4 text-sm leading-6 text-sky-900">'+esc(x.overview)+'</div>');
 parts.push(block('分步讲解',arr(x.steps).map((s,i)=>'<div class="rounded-2xl border border-sky-100 bg-sky-50/50 p-4"><p class="text-xs font-semibold text-sky-700">Step '+(i+1)+'</p><h3 class="mt-1 font-semibold text-slate-950">'+esc(s.title||('第 '+(i+1)+' 步'))+'</h3>'+(s.explanation?'<p class="mt-2 text-sm leading-7 text-slate-600">'+esc(s.explanation)+'</p>':'')+(s.example?'<p class="mt-2 text-sm text-slate-700"><b>示例：</b>'+esc(s.example)+'</p>':'')+(s.action?'<p class="mt-2 text-sm text-slate-700"><b>行动：</b>'+esc(s.action)+'</p>':'')+(s.check?'<p class="mt-2 text-sm text-slate-700"><b>检查：</b>'+esc(s.check)+'</p>':'')+'</div>'));
 parts.push(block('阶段任务',arr(x.tasks).map((t,i)=>'<div class="rounded-2xl border border-slate-200 bg-white p-4"><p class="font-semibold text-slate-950">'+esc(t.title||('任务 '+(i+1)))+'</p>'+(t.duration?'<p class="mt-1 text-sm text-slate-500">预计 '+esc(t.duration)+'</p>':'')+(t.description?'<p class="mt-2 text-sm leading-7 text-slate-600">'+esc(t.description)+'</p>':'')+(t.output?'<p class="mt-2 rounded-xl bg-slate-50 p-3 text-sm text-slate-700"><b>产出：</b>'+esc(t.output)+'</p>':'')+'</div>'));
 if(arr(x.checklist).length)parts.push('<div class="mt-5 rounded-2xl border border-emerald-100 bg-emerald-50 p-4"><p class="text-xs font-semibold uppercase tracking-[0.16em] text-emerald-700">阶段验收 checklist</p><div class="mt-2 grid gap-2">'+arr(x.checklist).map(s=>'<p class="text-sm text-emerald-900">✓ '+esc(s)+'</p>').join('')+'</div></div>');
 if(arr(x.commonMistakes).length)parts.push('<div class="mt-5"><p class="text-xs font-semibold uppercase tracking-[0.16em] text-rose-700">常见错误</p>'+pills(arr(x.commonMistakes))+'</div>');
 box.innerHTML=parts.join('')||'<p class="text-sm text-slate-500">暂无展开内容，可在阶段页重新生成。</p>';
}
function failed(){
 box.innerHTML='<div class="rounded-2xl border border-amber-100 bg-amber-50 p-5 text-center"><p class="font-semibold text-amber-800">阶段内容暂未生成完成</p><p class="mt-2 text-sm text-slate-600">AI 暂时无法生成讲解与任务，请稍后重试或返回课程页重新生成。</p><button type="button" onclick="location.reload()" class="mt-4 inline-flex min-h-11 items-center justify-center rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white">重新生成阶段</button></div>';
}
fetch('/api/phase-expansion',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({courseId:C||undefined,anonymousId:A||undefined,goal:G,mode:M,phaseIndex:Number(P)||1,stage:N,topics})})
 .then(r=>r.json()).then(d=>{if(d.ok&&d.phase)render(d.phase);else failed();}).catch(failed);
})()</script>)HTML";
    return document("阶段 - 钢一定制AI", body, script);
}

}  // namespace gangyi

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
        htmlEscape(title) + "</title><link rel=\"icon\" type=\"image/png\" href=\"/school-logo.png\"><link rel=\"stylesheet\" href=\"/styles.css\"></head><body class=\"antialiased page-transition\">" + body + script + "</body></html>";
}

std::string navLink(const std::string& href, const std::string& label, bool active) {
    return std::string("<a class=\"rounded-lg px-3 py-2 transition ") +
        (active ? "text-sky-700 hover:bg-sky-50" : "text-slate-600 hover:bg-slate-50") +
        "\" href=\"" + href + "\">" + label + "</a>";
}

std::string headerShell(const std::string& active) {
    const auto is = [&](const std::string& key) { return active == key; };
    return std::string("<header class=\"site-header ") +
        (is("home") ? "home-site-header " : "") +
        R"HTML(sticky top-0 z-30 w-full max-w-full border-b border-slate-200/70 bg-white/92 backdrop-blur-xl"><div class="mx-auto flex min-h-13 w-full max-w-7xl items-center justify-between gap-2 px-3 py-2 sm:min-h-16 md:px-6 lg:px-8"><a class="shrink-0 whitespace-nowrap text-base font-semibold tracking-tight text-sky-900 md:text-lg" href="/">钢一定制AI</a><nav class="hidden min-w-0 flex-1 items-center justify-end gap-2 text-sm font-medium text-slate-600 md:flex lg:gap-4">)HTML" +
        navLink("/", "首页", is("home")) +
        navLink("/my-courses", "我的课程", is("my-courses")) +
        navLink("/ask", "AI 对话", is("ask")) +
        R"HTML(</nav><details class="group relative shrink-0 md:hidden"><summary class="list-none rounded-full border border-slate-200 bg-white px-3 py-2 text-sm font-semibold text-slate-700 shadow-sm marker:hidden focus:outline-none focus:ring-2 focus:ring-sky-300 [&::-webkit-details-marker]:hidden"><span class="inline-flex items-center gap-1.5"><span class="text-base leading-none">≡</span>菜单</span></summary><nav class="fixed left-3 right-3 top-14 mt-2 flex max-h-[calc(100dvh-4rem)] flex-col gap-1 overflow-y-auto rounded-2xl border border-slate-200 bg-white p-2 text-sm font-medium text-slate-700 shadow-xl shadow-slate-900/10 sm:left-auto sm:right-4 sm:w-[min(18rem,calc(100vw-1.5rem))] md:absolute md:right-0 md:top-full">)HTML" +
        navLink("/", "首页", is("home")) +
        navLink("/my-courses", "我的课程", is("my-courses")) +
        navLink("/ask", "AI 对话", is("ask")) +
        R"HTML(</nav></details></div></header>)HTML";
}

std::string loadingSpinner(const std::string& title, const std::string& subtitle) {
    return R"HTML(<div class="rounded-3xl border border-sky-100 bg-white p-8 text-center shadow-sm shadow-sky-900/5"><div class="mx-auto h-10 w-10 animate-spin rounded-full border-4 border-sky-100 border-t-sky-600"></div><p class="mt-5 font-semibold text-slate-800">)HTML" +
        htmlEscape(title) +
        R"HTML(</p><p class="mt-2 text-sm text-slate-500">)HTML" +
        htmlEscape(subtitle) +
        R"HTML(</p></div>)HTML";
}

}  // namespace

std::string renderHomePage() {
    const std::string body = headerShell("home") + R"HTML(
<main class="home-page min-h-screen text-slate-950">
  <section class="home-hero mx-auto flex min-h-[calc(100vh-3.25rem)] w-full max-w-6xl flex-col items-center px-4 pb-8 pt-3 text-center sm:px-6 sm:pt-5">
    <img src="/school-logo.png" alt="柳州市钢一中学校徽" width="680" height="680" fetchpriority="high" class="home-school-logo shrink-0 object-contain"><div class="home-intro"><p class="text-xs font-semibold uppercase tracking-[0.14em] text-sky-800">柳州市钢一中学</p><h1 class="mt-1 max-w-3xl text-2xl font-bold tracking-tight text-sky-900 sm:text-4xl">柳州市钢一中学2629班定制AI</h1><p class="mt-1 max-w-xl text-sm leading-6 text-slate-700 sm:text-base">把学习目标变成可以一步步完成的课程。</p></div>
    <form id="goal-form" class="home-goal-form mt-4 w-full max-w-3xl rounded-2xl border border-slate-200 bg-white p-2 text-left shadow-lg shadow-sky-900/10" action="/plan" method="get"><input id="goal-image" type="file" accept="image/*" class="hidden"><input id="goal-mode" type="hidden" name="mode" value="deep">
      <div class="home-goal-surface overflow-hidden rounded-[22px] border border-slate-100 bg-white"><div class="px-4 pb-4 pt-4 sm:px-5"><label class="hidden text-sm font-semibold text-sky-900 sm:block" for="goal">今天想提升哪一门高中课程？</label><textarea id="goal" name="goal" required rows="3" class="mt-1 block min-h-24 w-full resize-none border-0 bg-transparent text-base leading-7 text-slate-950 outline-none placeholder:text-slate-500 sm:text-lg" placeholder="例如：高一数学函数与图像"></textarea><div class="mt-3 flex flex-wrap items-center gap-2"><button id="pick-image" type="button" class="home-secondary-button inline-flex min-h-10 items-center justify-center rounded-full border border-slate-200 bg-white px-3 text-sm font-semibold text-sky-800 hover:bg-sky-50">＋ 上传参考图</button><span class="flex-1"></span><button id="goal-submit" type="submit" class="inline-flex h-11 min-w-11 items-center justify-center rounded-full bg-sky-700 px-4 text-sm font-semibold text-white hover:bg-sky-800">生成 →</button></div></div><div class="home-goal-modes grid border-t border-slate-100 bg-slate-50/70 sm:grid-cols-2"><button type="button" data-mode="lite" class="home-mode-panel mode-panel p-4 text-left text-sm text-slate-700"><b class="block text-slate-950">快速规划</b><span class="mt-1 block">快速生成学习路线</span></button><button type="button" data-mode="deep" class="home-mode-panel mode-panel border-t border-slate-100 p-4 text-left text-sm text-slate-700 sm:border-l sm:border-t-0"><b class="block text-sky-900">深度课程 · 推荐</b><span class="mt-1 block">搜索资料并生成完整课程</span></button></div></div>
      <div id="image-preview" class="hidden mt-3 items-center gap-3 rounded-2xl border border-sky-100 bg-sky-50 p-3"><img class="h-16 w-16 rounded-xl border border-white object-cover" alt="图片预览"><div class="min-w-0 flex-1"><p id="image-name" class="truncate text-sm font-semibold"></p><p id="image-meta" class="mt-1 text-xs text-slate-500"></p></div><button id="remove-image" type="button" class="rounded-xl border border-slate-200 bg-white px-3 py-2 text-sm text-slate-500">移除</button></div><p id="goal-message" class="hidden mt-3 rounded-xl bg-amber-50 px-3 py-2 text-sm text-amber-700"></p>
    </form>
    <div id="goal-examples" class="mt-4 flex max-w-4xl flex-wrap justify-center gap-2"></div>
  </section>
  <section class="home-features mx-auto grid w-full max-w-6xl gap-4 px-4 pb-20 sm:grid-cols-2 sm:px-6 lg:grid-cols-4">
    <article class="home-feature-card rounded-2xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><div class="text-2xl text-sky-700">01</div><h2 class="mt-4 font-semibold">阶段任务</h2><p class="mt-2 text-sm leading-6 text-slate-700">把大目标拆成每个阶段都能完成的行动。</p></article>
    <article class="home-feature-card rounded-2xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><div class="text-2xl text-sky-700">02</div><h2 class="mt-4 font-semibold">微课程讲解</h2><p class="mt-2 text-sm leading-6 text-slate-700">围绕你的目标组织重点知识与学习顺序。</p></article>
    <article class="home-feature-card rounded-2xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><div class="text-2xl text-sky-700">03</div><h2 class="mt-4 font-semibold">练习测验</h2><p class="mt-2 text-sm leading-6 text-slate-700">用练习和检查点确认每一步真正掌握。</p></article>
    <article class="home-feature-card rounded-2xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5"><div class="text-2xl text-sky-700">04</div><h2 class="mt-4 font-semibold">进度跟踪</h2><p class="mt-2 text-sm leading-6 text-slate-700">持续记录进展，及时调整接下来的路线。</p></article>
  </section>
</main>)HTML";
    const std::string script = R"HTML(<script>(()=>{const $=id=>document.getElementById(id),form=$('goal-form'),goal=$('goal'),file=$('goal-image'),mode=$('goal-mode'),preview=$('image-preview'),message=$('goal-message'),examples={lite:['三天梳理高一函数概念','快速复习英语时态','二次函数图像与性质','化学物质的量基础','高一语文文言文实词'],deep:['系统复习高中数学函数与导数','高考物理力学专题训练','高中英语阅读与写作提升','高中化学氧化还原反应','高中生物遗传与变异','高中语文现代文阅读','高考历史中国古代史复习','高中地理大气运动专题']};let selectedMode='deep',imageFile=null,imageUrl='';function setMode(value){selectedMode=value==='lite'?'lite':'deep';mode.value=selectedMode;document.querySelectorAll('[data-mode]').forEach(button=>{const active=button.dataset.mode===selectedMode;button.classList.toggle('bg-sky-700',active);button.classList.toggle('text-white',active);button.classList.toggle('border-sky-700',active);button.classList.toggle('bg-white',!active);button.classList.toggle('text-slate-600',!active)});renderExamples()}function renderExamples(){const items=[...examples[selectedMode]].sort(()=>Math.random()-.5).slice(0,5);$('goal-examples').innerHTML=items.map(item=>'<button type="button" class="goal-example rounded-full border border-sky-100 bg-white/80 px-3 py-2 text-sm font-medium text-sky-900 hover:bg-sky-50">'+item+'</button>').join('')}function clearImage(){if(imageUrl)URL.revokeObjectURL(imageUrl);imageFile=null;imageUrl='';file.value='';preview.classList.add('hidden');preview.classList.remove('flex')}function showMessage(text){message.textContent=text;message.classList.remove('hidden')}function createAnonymousId(){const id=typeof crypto!=='undefined'&&typeof crypto.randomUUID==='function'?crypto.randomUUID():Date.now().toString(36)+'-'+Math.random().toString(36).slice(2);return 'anon_'+id}function anonymousId(){try{let id=localStorage.getItem('gangyi-anonymous-id');if(!id){id=createAnonymousId();localStorage.setItem('gangyi-anonymous-id',id)}return id}catch(_){return createAnonymousId()}}file.addEventListener('change',()=>{const picked=file.files?.[0];message.classList.add('hidden');if(!picked)return;if(!picked.type.startsWith('image/')){clearImage();showMessage('请上传图片文件');return}if(picked.size>5*1024*1024){clearImage();showMessage('图片过大，请上传 5MB 以内的图片');return}clearImage();imageFile=picked;imageUrl=URL.createObjectURL(picked);preview.querySelector('img').src=imageUrl;$('image-name').textContent=picked.name;$('image-meta').textContent=(picked.size/1024/1024).toFixed(1)+' MB · 将结合图片生成学习目标';preview.classList.remove('hidden');preview.classList.add('flex')});$('pick-image').onclick=()=>file.click();$('remove-image').onclick=clearImage;document.addEventListener('click',event=>{const modeButton=event.target.closest('[data-mode]');if(modeButton)setMode(modeButton.dataset.mode);const example=event.target.closest('.goal-example');if(example){goal.value=example.textContent;goal.focus()}});goal.addEventListener('keydown',event=>{if(event.key==='Enter'&&!event.shiftKey&&!event.isComposing){event.preventDefault();form.requestSubmit()}});form.addEventListener('submit',async event=>{event.preventDefault();const text=goal.value.trim();message.classList.add('hidden');if(!text&&!imageFile){showMessage('请输入学习需求，或上传一张相关图片');return}const submit=$('goal-submit');submit.disabled=true;submit.textContent=imageFile?'识别中…':'准备生成…';let target=text;if(imageFile){try{const data=new FormData();data.append('image',imageFile);data.append('prompt',text);data.append('mode',selectedMode);const response=await fetch('/api/analyze-image-goal',{method:'POST',body:data});const result=await response.json();if(result.success&&result.goal)target=result.goal;else if(!text){showMessage(result.message||'图片识别未完成，请补充文字描述后重试');submit.disabled=false;submit.textContent='生成 →';return}else showMessage(result.message||'图片识别未完成，已按文字描述生成课程')}catch(_){if(!text){showMessage('图片识别未完成，请补充文字描述后重试');submit.disabled=false;submit.textContent='生成 →';return}showMessage('图片识别未完成，已按文字描述生成课程')}}location.href='/plan?'+new URLSearchParams({goal:target,mode:selectedMode,anonymousId:anonymousId()})});setMode('deep')})()</script>)HTML";
    return document("柳州市钢一定制AI - 把学习目标变成可执行课程", body, script);
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
      <div id="plan-view" class="plan-view"></div>
    </section>
  </section>
</main>)HTML";

    const std::string script = R"HTML(<script>(()=>{
const initialGoal=)HTML" + jsString(goal) + R"HTML(,initialMode=)HTML" + jsString(mode == "lite" ? "lite" : "deep") + R"HTML(,courseId=)HTML" + jsString(courseId) + R"HTML(,anonymousId=)HTML" + jsString(anonymousId) + R"HTML(;
const $=id=>document.getElementById(id), loading=$('loading'),failure=$('failure'),result=$('result'),status=$('save-status'); let plan=null,renderMode=initialMode;
const esc=value=>{const d=document.createElement('div');d.textContent=value??'';return d.innerHTML}; const text=(v,f='暂无内容')=>esc(v||f); const arr=(v)=>Array.isArray(v)?v:[];
const list=(items,fn)=>arr(items).length?arr(items).map(fn).join(''):'<p class="empty-note">暂无内容</p>';
const card=(label,title,body,extra='')=>'<article class="plan-card '+extra+'"><p class="plan-kicker">'+text(label)+'</p><h2>'+text(title)+'</h2>'+body+'</article>';
const pill=v=>'<span class="plan-pill">'+text(v)+'</span>';
function mind(node,depth=0){return '<li class="mind-node depth-'+depth+'"><span>'+text(node.label||node.title)+'</span>'+((node.children||[]).length?'<ul>'+arr(node.children).map(x=>mind(x,depth+1)).join('')+'</ul>':'')+'</li>'}
function phase(p,i){const steps=arr(p.steps);return '<article class="phase-card"><div class="phase-number">'+String(i+1).padStart(2,'0')+'</div><div class="phase-main"><div class="phase-meta"><span>'+text(p.duration,'阶段周期')+'</span><span>'+text(i===0?'待开始':'后续阶段')+'</span></div><h3>'+text(p.name,'学习阶段')+'</h3><p>'+text(p.goal||p.description)+'</p><div class="phase-grid"><div><b>阶段产出</b><p>'+text(p.output)+'</p></div><div><b>检查点</b><p>'+text(p.checkpoint)+'</p></div></div><div class="step-list">'+list(steps.slice(0,5),(s,j)=>'<a href="/learn?courseId='+encodeURIComponent(courseId)+'&phaseIndex='+i+'&topicIndex='+j+'" class="step-row"><span>'+String(j+1).padStart(2,'0')+'</span><strong>'+text(s.title||s.name,'知识点')+'</strong><em>进入学习 →</em></a>')+'</div></div></article>'}
function deep(){const slides=arr(plan.slides), structure=arr(plan.courseStructure),resources=arr(plan.resources),projects=arr(plan.projects);return '<section class="plan-hero"> <div><span class="mode-badge">深度规划</span><h2>'+text(plan.title)+'</h2><p>'+text(plan.summary)+'</p></div><div class="hero-facts"><div><small>规划周期</small><b>'+text(plan.duration)+'</b></div><div><small>阶段数量</small><b>'+arr(plan.roadmap).length+' 个</b></div><div><small>学习目标</small><b>'+text(plan.outcome,'持续进步')+'</b></div></div></section><section class="next-action"><div><p class="plan-kicker">下一步</p><h2>从第一节开始，把计划变成进度</h2><p>每完成一个知识点，就会在本次会话中留下掌握标记。</p></div><a class="primary-action" href="/learn?courseId='+encodeURIComponent(courseId)+'&phaseIndex=0&topicIndex=0">开始第一节 <span>→</span></a></section><div class="progress-strip"><div><b>0%</b><span>课程进度 · 已完成 0 / '+structure.reduce((n,s)=>n+arr(s.topics).length,0)+'</span></div><div class="progress-track"><i></i></div></div>'+card('课件速览','把路线先看懂','<div class="slide-tabs">'+list(slides.slice(0,8),(s,i)=>'<button type="button" class="slide-tab '+(i?'':'selected')+'" data-slide="'+i+'">'+String(i+1).padStart(2,'0')+' · '+text(s.title)+'</button>')+'</div><div id="slide-content" class="slide-content"></div>','full-card')+card('知识结构',plan.mindMap?.title||'课程知识结构','<div class="mind-map"><ul>'+list(arr(plan.mindMap?.nodes),mind)+'</ul></div>','full-card')+'<section><div class="section-heading"><div><p class="plan-kicker">执行路线</p><h2>按阶段推进</h2></div><span class="muted-label">共 '+arr(plan.roadmap).length+' 个阶段</span></div><div class="phase-list">'+list(plan.roadmap,phase)+'</div></section>'+card('课程结构','知识点清单','<div class="structure-list">'+list(structure,(s,i)=>'<div class="structure-row"><div><b>'+text(s.stage)+'</b><p>'+arr(s.topics).map(text).join(' · ')+'</p></div><a href="/learn?courseId='+encodeURIComponent(courseId)+'&phaseIndex='+i+'&topicIndex=0">开始 →</a></div>')+'</div>','full-card')+card('精选资源','为你的目标准备的材料','<div class="resource-grid">'+list(resources.slice(0,8),r=>'<a class="resource-item" target="_blank" rel="noreferrer" href="'+esc(r.href||'#')+'"><div><span>'+text(r.type,'资源')+'</span>'+((r.free!==false)?'<b>免费</b>':'')+'</div><h3>'+text(r.name)+'</h3><p>'+text(r.description)+'</p><small>'+text(r.difficulty,'适合入门')+' ↗</small></a>')+'</div><p class="resource-source">'+text(plan.resourceSourceMessage,'以下为钢一定制AI推荐资源')+'</p>','full-card')+card('项目实战','用一个作品验收学习成果','<div class="project-grid">'+list(projects,(p)=>'<div class="project-item"><span>'+text(p.difficulty,'实践')+'</span><h3>'+text(p.name)+'</h3><p>'+text(p.output)+'</p><small>'+text(p.duration)+'</small></div>')+'</div>','full-card')}
function lite(){const roadmap=arr(plan.roadmap),steps=roadmap.flatMap(p=>arr(p.steps)).slice(0,5);return '<section class="lite-hero"><div><span class="mode-badge lite-badge">快速规划</span><h2>'+text(plan.title)+'</h2><p>'+text(plan.summary)+'</p></div><div class="lite-facts"><span><b>'+text(plan.duration)+'</b><small>周期</small></span><span><b>'+roadmap.length+'</b><small>阶段</small></span><span><b>先做会</b><small>再深入</small></span></div></section><section class="next-action lite-action"><div><p class="plan-kicker">立即开始</p><h2>先完成第一步，获得可见进展</h2></div><a class="primary-action lite-primary" href="/learn?courseId='+encodeURIComponent(courseId)+'&phaseIndex=0&topicIndex=0">开始第一节 <span>→</span></a></section>'+card('课程结构','只保留现在最需要的内容','<div class="lite-stage-list">'+list(roadmap.slice(0,3),(p,i)=>'<div><span>'+String(i+1).padStart(2,'0')+'</span><strong>'+text(p.name)+'</strong><em>'+text(p.duration)+'</em><p>'+text(p.goal||p.description)+'</p></div>')+'</div>','full-card')+card('核心步骤','怎么做，怎么验收','<div class="core-steps">'+list(steps,(s,i)=>'<div><span>'+String(i+1).padStart(2,'0')+'</span><div><h3>'+text(s.title)+'</h3><p>'+text(s.explanation)+'</p><b>验收：'+text(s.check)+'</b></div></div>')+'</div>','full-card')+card('提醒','少走弯路','<div class="mistake-grid">'+list(roadmap.slice(0,2),p=>'<div><b>'+text(p.name)+'</b><p>'+list(p.commonMistakes,m=>'<span>• '+text(m)+'</span>')+'</p></div>')+'</div>','full-card')+card('可参考资料','随用随查','<div class="resource-grid">'+list(arr(plan.resources).slice(0,5),r=>'<a class="resource-item" target="_blank" rel="noreferrer" href="'+esc(r.href||'#')+'"><h3>'+text(r.name)+'</h3><p>'+text(r.description)+'</p></a>')+'</div>','full-card')}
function preserveAnonymousIdInLearningLinks(){document.querySelectorAll('#plan-view a[href^="/learn?courseId="]').forEach(link=>{const url=new URL(link.href,location.origin);const rawPhase=Number(url.searchParams.get('phaseIndex')||'0'),rawTopic=Number(url.searchParams.get('topicIndex')||'0');const phaseNo=Number.isFinite(rawPhase)?rawPhase+1:1,topicNo=Number.isFinite(rawTopic)?rawTopic+1:1;url.searchParams.set('phaseIndex',String(phaseNo));url.searchParams.set('topicIndex',String(topicNo));if(initialGoal&&!url.searchParams.get('goal'))url.searchParams.set('goal',initialGoal);if(initialMode&&!url.searchParams.get('mode'))url.searchParams.set('mode',initialMode);const stage=arr(plan.courseStructure)[phaseNo-1]||arr(plan.roadmap)[phaseNo-1]||{};const phaseTitle=stage.stage||stage.name||'';if(phaseTitle&&!url.searchParams.get('phaseName'))url.searchParams.set('phaseName',phaseTitle);const topics=arr(stage.topics);const steps=arr(stage.steps);const topicTitle=topics[topicNo-1]||steps[topicNo-1]?.title||steps[topicNo-1]?.name||'';if(topicTitle&&!url.searchParams.get('topic'))url.searchParams.set('topic',topicTitle);if(anonymousId)url.searchParams.set('anonymousId',anonymousId);link.href=url.pathname+'?'+url.searchParams.toString()})}
function render(){ $('plan-title').textContent=plan.title||'你的课程总览';$('plan-goal').textContent=(plan.courseIntro||plan.summary||'')+(initialGoal?' · 目标：'+initialGoal:'');$('plan-view').innerHTML=renderMode==='lite'?lite():deep();preserveAnonymousIdInLearningLinks();renderSlide(0)}
function renderSlide(i){const s=arr(plan.slides)[i]||{};const el=$('slide-content');if(el)el.innerHTML='<h3>'+text(s.title)+'</h3><p>'+text(s.content)+'</p><div class="bullet-list">'+list(s.bullets,x=>'<span>'+text(x)+'</span>')+'</div>'}
function failureState(error){const type=error.type||'';const messages={timeout:'生成超时，请重试。',auth_error:'当前模型接口认证失败，请检查服务配置后重试。',rate_limited:'AI 服务请求过于频繁，请稍后重试。',invalid_response:'生成内容未通过质量检查，请重新生成。',json_parse_error:'生成内容未通过质量检查，请重新生成。',quality_rejected:'生成内容未通过质量检查，请重新生成。',missing_config:'当前模型接口尚未配置，请检查服务配置。'};failure.innerHTML='<div class="failure-panel"><div class="failure-mark">!</div><div><p class="plan-kicker">生成没有完成</p><h2>'+text(messages[type]||'AI 服务暂时不可用，请稍后重试。')+'</h2><p>'+text(error.message,'服务暂时没有返回可用内容。')+'</p><div class="failure-actions"><button type="button" class="primary-action" id="retry">重试生成</button><a class="secondary-action" href="/">返回首页</a></div></div></div>';failure.classList.remove('hidden');loading.classList.add('hidden');$('retry').onclick=()=>load(true)}
async function request(url,options={}){if(url.startsWith('/api/courses/')&&anonymousId&&!url.includes('?'))url+='?anonymousId='+encodeURIComponent(anonymousId);const controller=new AbortController();const timeoutMs=url==='/api/generate-plan'?250000:12000;const timer=setTimeout(()=>controller.abort(),timeoutMs);try{const r=await fetch(url,{...options,signal:controller.signal});const data=await r.json();if(!r.ok)throw Object.assign(new Error(data.message||data.error||'请求失败'),{type:data.type});return data}catch(error){if(controller.signal.aborted)throw Object.assign(new Error('AI 响应超时，请重试。'),{type:'timeout'});throw error}finally{clearTimeout(timer)}}
async function save(){if(!plan||courseId)return;status.textContent='正在保存课程…';try{const createAnonymousId=()=>"anon_"+(typeof crypto!=="undefined"&&typeof crypto.randomUUID==="function"?crypto.randomUUID():Date.now().toString(36)+"-"+Math.random().toString(36).slice(2));let anon=anonymousId;try{anon=anon||localStorage.getItem('gangyi-anonymous-id')||createAnonymousId();localStorage.setItem('gangyi-anonymous-id',anon)}catch(_){anon=anon||createAnonymousId()}const saved=await request('/api/courses',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({goal:initialGoal||plan.outcome,title:plan.title,summary:plan.summary,mode:initialMode,source:'ai',anonymousId:anon,payload:plan})});status.textContent='已自动保存';localStorage.setItem('gangyi-last-course',JSON.stringify({id:saved.courseId,title:plan.title,href:saved.href,updatedAt:new Date().toISOString()}));history.replaceState({},'',saved.href);document.dispatchEvent(new CustomEvent('course-saved',{detail:saved}))}catch(e){status.textContent='未保存';status.classList.add('warning');localStorage.setItem('gangyi-pending-plan',JSON.stringify({goal:initialGoal,mode:initialMode,payload:plan}));const note=document.createElement('div');note.className='save-warning';note.innerHTML='<b>课程快照保存失败。</b> 当前页面内容仍可查看，已暂存到本机。<button type="button" id="save-again">重新保存</button>';result.prepend(note);$('save-again').onclick=()=>{note.remove();save()}}}
async function load(force=false){loading.classList.remove('hidden');failure.classList.add('hidden');status.textContent='正在加载';try{let data;if(courseId&&!force){data=await request('/api/courses/'+encodeURIComponent(courseId));plan=data.snapshot.payload;renderMode=data.course.mode==='lite'?'lite':'deep';if(plan?.generation?.source!=='ai'||plan?.generation?.promptVersion!=='ai-plan-v1')throw Object.assign(new Error('该课程来自旧生成链路，请返回首页重新生成。'),{type:'course_regeneration_required'})}else{data=await request('/api/generate-plan',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({goal:initialGoal,mode:initialMode,retry:force?Date.now():undefined})});plan=data}if(!plan||!Array.isArray(plan.roadmap)||!plan.roadmap.length)throw Object.assign(new Error('课程结构不完整'),{type:'quality_rejected'});loading.classList.add('hidden');result.classList.remove('hidden');render();status.textContent=courseId?'已恢复课程':'生成完成';if(!courseId)save()}catch(e){failureState(e)}}
document.addEventListener('click',e=>{const b=e.target.closest('[data-slide]');if(b){document.querySelectorAll('[data-slide]').forEach(x=>x.classList.remove('selected'));b.classList.add('selected');renderSlide(Number(b.dataset.slide))}});document.addEventListener('course-saved',e=>{if(e.detail?.href&&!courseId)location.replace(e.detail.href)});load();
})();</script>)HTML";
    return document("课程规划 - 钢一定制AI", body, script);
}

std::string renderLearnPage(const std::string& courseId, [[maybe_unused]] const std::string& goal,
                            [[maybe_unused]] const std::string& mode,
                            const std::string& phaseIndex, const std::string& phaseName,
                            const std::string& topicIndex, const std::string& topic,
                            [[maybe_unused]] const std::string& anonymousId,
                            [[maybe_unused]] const std::string& regenerate,
                            [[maybe_unused]] const std::string& forceLearn,
                            [[maybe_unused]] const std::string& retry) {
    const std::string body = headerShell("learn") + R"HTML(
<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950"><section class="mx-auto w-full max-w-6xl px-4 py-6 sm:px-6"><div class="flex flex-col gap-4 lg:flex-row lg:items-start lg:justify-between"><div><p class="text-sm font-semibold text-sky-700">微课程 · 学习中</p><h1 id="learn-title" class="mt-2 text-3xl font-bold tracking-tight sm:text-4xl">正在生成本节微课程</h1><p id="learn-summary" class="mt-3 max-w-3xl leading-7 text-slate-600">AI 正在根据课程目标和当前分支生成专属讲解、练习与测验。</p></div><aside class="grid gap-2 rounded-3xl border border-sky-100 bg-white p-4 shadow-sm sm:min-w-72 sm:max-w-sm"><div class="flex items-start justify-between gap-3 text-sm"><span class="shrink-0">阶段</span><b id="learn-phase-name" class="max-w-56 break-words text-right">)HTML" + htmlEscape(phaseName.empty() ? "正在确认阶段" : phaseName) + R"HTML(</b></div><div class="flex items-start justify-between gap-3 text-sm"><span class="shrink-0">主题</span><b id="learn-topic-name" class="max-w-56 break-words text-right">)HTML" + htmlEscape(topic.empty() ? "正在确认主题" : topic) + R"HTML(</b></div><div class="h-2 overflow-hidden rounded-full bg-slate-200"><i id="learn-bar" class="soft-progress-fill block h-full w-[12%] rounded-full bg-sky-600"></i></div><small id="learn-save-status">正在生成课程内容</small></aside></div><section class="mt-6 rounded-3xl border border-sky-100 bg-white p-4 shadow-sm sm:p-6"><div id="learn-loading" class="p-8 text-center"><div class="mx-auto h-10 w-10 animate-spin rounded-full border-4 border-sky-100 border-t-sky-600"></div><p class="mt-4 font-semibold">正在生成与当前主题对应的讲解</p><p class="mt-2 text-sm text-slate-500">系统会自动检查内容质量；未通过时会根据原因重新生成。</p></div><div id="learn-content" class="hidden space-y-6"><section class="grid gap-4 lg:grid-cols-[1.3fr_0.9fr]"><article class="rounded-2xl border border-sky-100 bg-sky-50/70 p-5"><p class="text-sm font-semibold text-sky-700">本课概览</p><h2 id="learn-content-title" class="mt-2 text-2xl font-bold"></h2><p id="learn-content-summary" class="mt-3 leading-7 text-slate-600"></p></article><article class="rounded-2xl border border-slate-200 p-5"><p class="text-sm font-semibold">关键概念</p><div id="learn-key-concepts" class="mt-3 flex flex-wrap gap-2"></div></article></section><section class="grid gap-4 lg:grid-cols-2"><article class="rounded-2xl border border-sky-100 p-5"><div class="flex justify-between"><p class="text-sm font-semibold text-sky-700">理解步骤</p><small id="step-count"></small></div><div id="learn-steps" class="mt-4 space-y-3"></div></article><article class="rounded-2xl border border-sky-100 p-5"><p class="text-sm font-semibold text-sky-700">示例与练习</p><div id="learn-examples" class="mt-4 space-y-3"></div><div id="learn-practice" class="mt-4 space-y-3"></div></article></section><section class="grid gap-4 lg:grid-cols-2"><article class="rounded-2xl border border-sky-100 p-5"><div class="flex justify-between"><p class="text-sm font-semibold text-sky-700">小测验</p><small>答对 70% 即通过</small></div><div id="learn-quiz" class="mt-4 space-y-3"></div><button id="quiz-submit" type="button" class="mt-4 min-h-11 w-full rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white">提交测验</button><p id="quiz-result" class="mt-3 text-sm font-semibold"></p></article><article class="rounded-2xl border border-sky-100 p-5"><p class="text-sm font-semibold text-sky-700">完成检查</p><div id="learn-checkpoint" class="mt-3 space-y-2 text-sm leading-7"></div><div id="learn-mistakes" class="mt-4 space-y-2 text-sm leading-7"></div><button id="complete-lesson" type="button" class="mt-5 min-h-11 w-full rounded-xl bg-emerald-600 px-4 text-sm font-semibold text-white">完成本节并保存进度</button></article></section><section class="rounded-2xl border border-slate-200 bg-slate-50 p-5"><div class="flex items-center justify-between gap-3"><div><p class="text-sm font-semibold">参考资料</p><p id="learn-resource-summary" class="mt-1 text-xs leading-5 text-slate-500"></p></div><button id="regenerate" type="button" class="rounded-xl border border-slate-300 bg-white px-4 py-2 text-sm font-semibold">换一版讲解</button></div><div id="learn-references" class="mt-3 grid gap-3 md:grid-cols-2"></div></section></div></section></section></main>)HTML";
    const std::string script = R"HTML(<script>(()=>{
const C=)HTML"+jsString(courseId)+R"HTML(,P=)HTML"+jsString(phaseIndex)+R"HTML(,T=)HTML"+jsString(topicIndex)+R"HTML(,$=id=>document.getElementById(id),E=value=>{const node=document.createElement('div');node.textContent=value??'';return node.innerHTML},A=value=>Array.isArray(value)?value:[],params=new URLSearchParams(location.search),order=['overview','steps','examples','practice','quiz','assessment'],blocks={},state=JSON.parse(localStorage.getItem('gy:'+C+':'+P+':'+T)||'{}'),loadingMarkup=$('learn-loading').innerHTML;let lesson={lessonSteps:[],practice:[],quiz:[]};
const list=(value,render)=>A(value).length?A(value).map(render).join(''):'',base=()=>{const query=new URLSearchParams({courseId:C,phaseIndex:P||'1',topicIndex:T||'1'});['anonymousId','goal','mode','phaseName','topic'].forEach(key=>{const value=params.get(key);if(value)query.set(key,value)});return '/api/learn?'+query.toString()},waiting=id=>{$(id).innerHTML='<p class="animate-pulse text-sm text-sky-700">AI 正在生成…</p>'};
function references(items){const box=$('learn-references');if(!box)return;box.innerHTML=A(items).length?A(items).map(item=>'<a class="block rounded-2xl border border-sky-100 bg-white p-4" target="_blank" rel="noreferrer" href="'+(/^https?:\/\//i.test(item.url||'')?E(item.url):'#')+'"><b class="text-sky-800">'+E(item.title||item.source)+'</b><p class="mt-2 text-sm text-slate-600">'+E(item.description||'')+'</p></a>').join(''):'<p class="text-sm text-slate-500">暂未检索到公开资料。</p>'}
function apply(block,data){blocks[block]=data;Object.assign(lesson,data);if(block==='overview'){$('learn-title').textContent=data.title;$('learn-summary').textContent=data.summary;$('learn-content-title').textContent=data.title;$('learn-content-summary').textContent=data.summary;$('learn-key-concepts').innerHTML=list(data.keyConcepts,value=>'<span class="plan-pill">'+E(value)+'</span>')}else if(block==='steps'){$('learn-steps').innerHTML=list(data.lessonSteps,(item,index)=>'<article class="rounded-2xl border border-sky-100 bg-sky-50/50 p-4"><button type="button" class="step-toggle mr-3 h-6 w-6 rounded-full border-2 border-sky-300" data-i="'+index+'">'+(state.steps?.[index]?'✓':'')+'</button><b>'+E(item.title)+'</b><p class="mt-2 text-sm leading-7">'+E(item.explanation)+'</p><p class="mt-2 text-sm"><b>示例：</b>'+E(item.example)+'</p><p class="mt-1 text-sm"><b>行动：</b>'+E(item.action)+'</p><p class="mt-1 text-sm"><b>检查：</b>'+E(item.check)+'</p></article>');updateStepCount()}else if(block==='examples'){$('learn-examples').innerHTML=list(data.examples,item=>'<details class="rounded-2xl border border-slate-200 p-4"><summary class="cursor-pointer font-semibold">'+E(item.title)+'</summary><p class="mt-3 whitespace-pre-wrap text-sm leading-7">'+E(item.content)+'</p><p class="mt-3 whitespace-pre-wrap rounded-xl bg-sky-50 p-3 text-sm leading-7"><b>解析：</b>'+E(item.solution)+'</p></details>')}else if(block==='practice'){$('learn-practice').innerHTML=list(data.practice,(item,index)=>'<label class="flex gap-3 rounded-2xl border border-slate-200 p-4"><input class="practice-toggle accent-sky-600" type="checkbox" data-i="'+index+'" '+(state.practice?.[index]?'checked':'')+'><span><b>'+E(item.title)+'</b><span class="mt-1 block text-sm">'+E(item.task)+'</span><span class="mt-2 block text-xs text-slate-500">检查：'+E(item.check)+'</span></span></label>')}else if(block==='quiz'){$('learn-quiz').innerHTML=list(data.quiz,(item,index)=>'<fieldset class="rounded-2xl border border-slate-200 p-4"><legend class="text-sm font-semibold">'+(index+1)+'. '+E(item.question)+'</legend>'+list(item.options,(option,choice)=>'<label class="mt-2 flex gap-2 text-sm"><input type="radio" name="q'+index+'" value="'+choice+'">'+E(option)+'</label>')+'</fieldset>');$('quiz-submit').disabled=false}else if(block==='assessment'){$('learn-checkpoint').innerHTML=list(data.checkpoint,item=>'<p>✓ '+E(item)+'</p>');$('learn-mistakes').innerHTML=list(data.commonMistakes,item=>'<p>• '+E(item)+'</p>');$('learn-resource-summary').textContent=data.resourceSummary||''}updateStatus()}
function failed(message,block=''){const label=labels[block]||'本节课程';$('learn-content').classList.add('hidden');$('learn-loading').classList.remove('hidden');$('learn-loading').innerHTML='<div class="mx-auto max-w-xl rounded-2xl border border-rose-100 bg-rose-50 p-6 text-rose-700"><p class="font-semibold">'+E(label)+'经过 3 次真实 AI 尝试仍未生成完整课程。</p><p class="mt-2 text-sm">'+E(message)+'</p><button type="button" data-retry-lesson class="mt-4 rounded-xl bg-sky-700 px-4 py-2 font-semibold text-white">重新生成整节课程</button></div>'}
function updateStepCount(){const items=A(lesson.lessonSteps);$('step-count').textContent=items.filter((_,index)=>state.steps?.[index]).length+' / '+items.length+' 已理解'}function updateStatus(){const done=order.filter(block=>blocks[block]).length;$('learn-bar').style.width=Math.round(done/order.length*100)+'%';$('learn-save-status').textContent='AI 板块已生成 '+done+' / '+order.length;$('complete-lesson').disabled=done!==order.length}
async function requestAll(retry=false){const url=base()+'&block=all'+(params.get('regenerate')==='1'?'&regenerate=1':'')+(retry?'&retry=1':'');const response=await fetch(url);let data={};try{data=await response.json()}catch(_){}if(!response.ok||data.ok===false){const error=new Error(data.error||'AI 生成失败');error.failedBlock=data.failedBlock||'';throw error}return data}
async function pollProgress(){try{const response=await fetch(base()),data=await response.json();if(!response.ok)return;const done=order.filter(block=>data.generations?.[block]?.source==='ai').length;$('learn-bar').style.width=Math.round(done/order.length*100)+'%';$('learn-save-status').textContent='真实 AI 已生成 '+done+' / '+order.length+'，完成后统一展示'}catch(_){}}
async function start(retry=false){$('learn-loading').innerHTML=loadingMarkup;$('learn-loading').classList.remove('hidden');$('learn-content').classList.add('hidden');$('quiz-submit').disabled=true;$('complete-lesson').disabled=true;const track=params.get('regenerate')!=='1',timer=track?setInterval(pollProgress,2500):0;if(track)pollProgress();else $('learn-save-status').textContent='真实 AI 正在重新生成整节课程';try{const saved=await requestAll(retry);for(const key of Object.keys(blocks))delete blocks[key];lesson={lessonSteps:[],practice:[],quiz:[]};$('learn-phase-name').textContent=saved.phaseName||params.get('phaseName')||'当前阶段';$('learn-topic-name').textContent=saved.topicTitle||params.get('topic')||'当前主题';Object.entries(saved.blocks||{}).forEach(([block,data])=>apply(block,data));if(order.some(block=>!blocks[block]))throw new Error('AI 尚未生成完整的六个课程板块');references(saved.references);$('learn-loading').classList.add('hidden');$('learn-content').classList.remove('hidden');if(params.has('regenerate')){params.delete('regenerate');const query=params.toString();history.replaceState({},'',location.pathname+(query?'?'+query:'')+location.hash)}}catch(error){failed(error.message,error.failedBlock)}finally{if(timer)clearInterval(timer)}}
document.addEventListener('click',event=>{const retry=event.target.closest('[data-retry-lesson]');if(retry){retry.disabled=true;retry.textContent='AI 正在重新生成整节课程…';start(true);return}const toggle=event.target.closest('.step-toggle');if(!toggle)return;const index=+toggle.dataset.i;state.steps=state.steps||{};state.steps[index]=!state.steps[index];toggle.textContent=state.steps[index]?'✓':'';updateStepCount();localStorage.setItem('gy:'+C+':'+P+':'+T,JSON.stringify(state));fetch('/api/learning-step-progress',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({courseId:C,phaseIndex:+P,stepIndex:index,stepTitle:lesson.lessonSteps[index].title,status:state.steps[index]?'understood':'unset'})})});
document.addEventListener('change',event=>{if(!event.target.matches('.practice-toggle'))return;state.practice=state.practice||{};state.practice[event.target.dataset.i]=event.target.checked;localStorage.setItem('gy:'+C+':'+P+':'+T,JSON.stringify(state))});
$('quiz-submit').onclick=()=>{let score=0;A(lesson.quiz).forEach((item,index)=>{const answer=document.querySelector('input[name="q'+index+'"]:checked');if(answer&&+answer.value===item.answerIndex)score++});$('quiz-result').textContent=lesson.quiz.length?'得分 '+score+' / '+lesson.quiz.length+(score/lesson.quiz.length>=.7?'，已通过':'，请复习后重试'):''};
$('complete-lesson').onclick=()=>fetch('/api/learn/progress',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({courseId:C,phaseIndex:+P,topicIndex:+T,phaseName:params.get('phaseName')||'',topicTitle:params.get('topic')||'',status:'completed',lastVisitedUrl:location.pathname+location.search,lastPageType:'learn',lastPhaseIndex:+P,lastTopicIndex:+T})});
$('regenerate').onclick=()=>{params.set('regenerate','1');location.search=params.toString()};start()})()</script>)HTML";
    const std::string identityBridge = R"HTML(<script>(()=>{const id=new URLSearchParams(location.search).get('anonymousId');if(id)document.cookie='gangyi_anonymous_id='+encodeURIComponent(id)+'; Path=/; SameSite=Lax'})()</script>)HTML";
    const std::string progressFeedbackEnhancement = R"HTML(<script>(()=>{const button=document.getElementById('complete-lesson');if(!button)return;const params=new URLSearchParams(location.search),key='gy:'+(params.get('courseId')||'')+':'+(params.get('phaseIndex')||'0')+':'+(params.get('topicIndex')||'0'),feedback=document.createElement('p'),topStatus=document.getElementById('learn-save-status'),bar=document.getElementById('learn-bar');feedback.id='complete-feedback';feedback.setAttribute('role','status');feedback.setAttribute('aria-live','polite');feedback.className='mt-3 text-center text-sm font-semibold';button.insertAdjacentElement('afterend',feedback);const setState=(state,message='')=>{if(state==='saving'){button.disabled=true;button.textContent='正在保存进度…';button.classList.add('opacity-75');feedback.className='mt-3 text-center text-sm font-semibold text-sky-700';feedback.textContent='正在保存本节完成状态，请稍候。'}else if(state==='success'){button.disabled=true;button.textContent='✓ 本节已完成，进度已保存';button.classList.remove('opacity-75');button.classList.add('bg-emerald-700');feedback.className='mt-3 text-center text-sm font-semibold text-emerald-700';feedback.textContent='保存成功，可以返回课程继续下一节。';if(topStatus)topStatus.textContent='本节已完成，进度已保存';if(bar)bar.style.width='100%';try{const saved=JSON.parse(localStorage.getItem(key)||'{}');saved.completed=true;localStorage.setItem(key,JSON.stringify(saved))}catch(_){}}else{button.disabled=false;button.textContent='重新保存本节进度';button.classList.remove('opacity-75');feedback.className='mt-3 text-center text-sm font-semibold text-rose-700';feedback.textContent=message||'保存失败，请检查网络后重试。'}};try{if(JSON.parse(localStorage.getItem(key)||'{}').completed)setState('success')}catch(_){}const nativeFetch=window.fetch.bind(window);window.fetch=(input,init={})=>{const url=typeof input==='string'?input:(input?.url||'');if(url!=='/api/learn/progress')return nativeFetch(input,init);setState('saving');return nativeFetch(input,init).then(async response=>{let data={};try{data=await response.clone().json()}catch(_){}if(response.ok&&data.ok!==false)setState('success');else setState('error',data.error?'保存失败：'+data.error:'保存失败，请稍后重试。');return response},error=>{setState('error','保存失败，请检查网络后重试。');throw error})}})()</script>)HTML";
    const std::string lessonNavigationEnhancement = R"HTML(<script>(()=>{const complete=document.getElementById('complete-lesson');if(!complete||document.getElementById('lesson-navigation'))return;const params=new URLSearchParams(location.search),courseId=params.get('courseId')||'',anonymousId=params.get('anonymousId')||'',currentPhase=Math.max(1,Number(params.get('phaseIndex'))||1),currentTopic=Math.max(1,Number(params.get('topicIndex'))||1),navigation=document.createElement('div'),back=document.createElement('a');navigation.id='lesson-navigation';navigation.className='mt-4 grid gap-3';back.id='lesson-return';back.className='inline-flex min-h-11 items-center justify-center rounded-xl border border-sky-200 bg-white px-4 text-sm font-semibold text-sky-700 transition hover:border-sky-400 hover:bg-sky-50';back.textContent='← 返回流程';const planParams=new URLSearchParams({courseId});if(anonymousId)planParams.set('anonymousId',anonymousId);back.href='/plan?'+planParams.toString();navigation.appendChild(back);(document.getElementById('complete-feedback')||complete).insertAdjacentElement('afterend',navigation);if(!courseId)return;const courseQuery=anonymousId?'?anonymousId='+encodeURIComponent(anonymousId):'';fetch('/api/courses/'+encodeURIComponent(courseId)+courseQuery).then(async response=>{const data=await response.json();if(!response.ok)throw new Error(data.error||'课程流程读取失败');const plan=data.snapshot?.payload||{},structure=Array.isArray(plan.courseStructure)?plan.courseStructure:[],roadmap=Array.isArray(plan.roadmap)?plan.roadmap:[],count=Math.max(structure.length,roadmap.length),phases=[];for(let i=0;i<count;i++){const courseStage=structure[i]||{},roadStage=roadmap[i]||{},rawTopics=Array.isArray(courseStage.topics)&&courseStage.topics.length?courseStage.topics:Array.isArray(roadStage.topics)&&roadStage.topics.length?roadStage.topics:Array.isArray(roadStage.steps)&&roadStage.steps.length?roadStage.steps:Array.isArray(roadStage.tasks)?roadStage.tasks:[],topics=rawTopics.map(item=>typeof item==='string'?item:(item?.title||item?.name||item?.topic||'')).filter(Boolean);phases.push({name:courseStage.stage||courseStage.name||roadStage.name||roadStage.stage||('阶段 '+(i+1)),topics})}let nextPhase=currentPhase-1,nextTopic=currentTopic;if(nextPhase>=phases.length)return;if(nextTopic>=phases[nextPhase].topics.length){nextPhase++;nextTopic=0;while(nextPhase<phases.length&&!phases[nextPhase].topics.length)nextPhase++}if(nextPhase>=phases.length||!phases[nextPhase].topics[nextTopic])return;const nextParams=new URLSearchParams({courseId,phaseIndex:String(nextPhase+1),topicIndex:String(nextTopic+1),goal:data.course?.goal||params.get('goal')||'',mode:data.course?.mode||params.get('mode')||'deep',phaseName:phases[nextPhase].name,topic:phases[nextPhase].topics[nextTopic]});if(anonymousId)nextParams.set('anonymousId',anonymousId);const next=document.createElement('a');next.id='lesson-next';next.className='inline-flex min-h-11 items-center justify-center rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white transition hover:bg-sky-800';next.textContent='继续下一节 →';next.href='/learn?'+nextParams.toString();next.setAttribute('aria-label','继续下一节：'+phases[nextPhase].topics[nextTopic]);navigation.classList.add('sm:grid-cols-2');navigation.appendChild(next)}).catch(()=>{})})()</script>)HTML";
    const std::string backNavigation = R"HTML(<script>(()=>{const page=document.querySelector('.learn-app-page'),heading=page?.querySelector('h1');const container=heading?.parentElement;if(!container||document.getElementById('learn-back'))return;const button=document.createElement('button');button.id='learn-back';button.type='button';button.className='mb-4 inline-flex min-h-10 items-center rounded-xl border border-sky-200 bg-white px-4 text-sm font-semibold text-sky-700 shadow-sm transition hover:border-sky-400 hover:bg-sky-50';button.textContent='← 返回上一页';button.addEventListener('click',()=>{if(window.history.length>1){window.history.back();return}const params=new URLSearchParams(location.search);const courseId=params.get('courseId');location.href=courseId?'/plan?courseId='+encodeURIComponent(courseId):'/'});container.insertBefore(button,container.firstChild)})()</script>)HTML";
    const std::string chatEnhancement = R"HTML(<script>(()=>{const p=new URLSearchParams(location.search),esc=v=>{const d=document.createElement('div');d.textContent=v??'';return d.innerHTML},messages=[];const wrap=document.createElement('div');wrap.innerHTML='<button id="learning-chat-toggle" type="button" class="fixed bottom-5 right-5 z-40 rounded-full bg-sky-700 px-5 py-3 text-sm font-semibold text-white shadow-lg hover:bg-sky-800">问问课堂 AI</button><section id="learning-chat-panel" class="fixed bottom-20 right-5 z-40 hidden w-[min(92vw,380px)] overflow-hidden rounded-3xl border border-sky-100 bg-white shadow-2xl"><header class="flex items-center justify-between bg-sky-700 px-4 py-3 text-white"><b>课堂 AI 助手</b><button id="learning-chat-close" type="button" class="text-xl">×</button></header><div id="learning-chat-messages" class="max-h-80 space-y-3 overflow-y-auto p-4"><p class="text-sm leading-6 text-slate-500">可以针对当前主题提问，AI 会结合本节内容回答。</p></div><form id="learning-chat-form" class="border-t border-slate-100 p-3"><textarea id="learning-chat-input" rows="2" required class="w-full resize-none rounded-xl border border-slate-200 p-3 text-sm outline-none focus:border-sky-500" placeholder="例如：这一步为什么适用于当前主题？"></textarea><button id="learning-chat-send" type="submit" class="mt-2 min-h-10 w-full rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white">发送</button></form></section>';document.body.appendChild(wrap);const panel=document.getElementById('learning-chat-panel'),toggle=document.getElementById('learning-chat-toggle'),close=document.getElementById('learning-chat-close'),box=document.getElementById('learning-chat-messages'),input=document.getElementById('learning-chat-input'),send=document.getElementById('learning-chat-send');const add=(text,right=false)=>{const node=document.createElement('p');node.className='rounded-2xl px-3 py-2 text-sm leading-6 whitespace-pre-wrap '+(right?'ml-8 bg-sky-100 text-slate-800':'mr-8 bg-slate-100 text-slate-700');node.innerHTML=esc(text);box.appendChild(node);box.scrollTop=box.scrollHeight;return node};toggle.onclick=()=>{panel.classList.toggle('hidden');if(!panel.classList.contains('hidden'))input.focus()};close.onclick=()=>panel.classList.add('hidden');document.getElementById('learning-chat-form').onsubmit=async e=>{e.preventDefault();const question=input.value.trim();if(!question)return;add(question,true);messages.push({role:'user',content:question});input.value='';send.disabled=true;send.textContent='思考中…';const pending=add('课堂 AI 正在思考…');try{const r=await fetch('/api/learning-chat',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({question,pageType:'learn',goal:p.get('goal')||'',mode:p.get('mode')||'deep',phaseName:p.get('phaseName')||document.getElementById('learn-phase-name')?.textContent||'',topic:p.get('topic')||document.getElementById('learn-topic-name')?.textContent||'',contextTitle:document.getElementById('learn-content-title')?.textContent||'',contextSummary:document.getElementById('learn-content-summary')?.textContent||'',messages})});const data=await r.json();if(!r.ok)throw new Error(data.error||'课堂 AI 暂时不可用，请重试');pending.textContent=data.answer||'AI 没有返回内容';messages.push({role:'assistant',content:data.answer||''})}catch(error){pending.textContent=error.message||'课堂 AI 暂时不可用，请重试'}finally{send.disabled=false;send.textContent='发送';input.focus()}}})()</script>)HTML";
    return document("学习中 - 钢一定制AI", body, identityBridge + script + progressFeedbackEnhancement + lessonNavigationEnhancement + backNavigation + chatEnhancement);
}

#if 0  // 顶部进度页面已删除。
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
    const std::string resetControl = courseId.empty() ? "" : R"HTML(<button id="reset-course-progress" type="button" class="inline-flex min-h-11 items-center justify-center rounded-xl border border-rose-200 bg-white px-4 text-sm font-semibold text-rose-700 transition hover:bg-rose-50">重置本课程进度</button>)HTML";

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
  <div class="mt-4 flex items-center justify-between text-sm text-slate-600"><span>完成进度</span><span class="font-semibold text-slate-900">)HTML" + std::to_string(pct) + R"HTML(% · )HTML" + std::to_string(pdone) + R"HTML(/)HTML" + std::to_string(ptotal) + R"HTML( 节</span></div>
  <div class="mt-2 h-2.5 overflow-hidden rounded-full bg-slate-100"><div class="h-full rounded-full )HTML" + barCls + R"HTML(" style="width: )HTML" + std::to_string(pct) + R"HTML(%"></div></div>
  <a class="mt-4 inline-flex min-h-10 items-center justify-center rounded-xl bg-sky-700 px-3 text-sm font-semibold text-white transition hover:bg-sky-800" href=")HTML" + htmlEscape(href) + R"HTML(">进入本阶段 →</a>
</article>)HTML";
    }
    body = headerShell("progress") + R"HTML(<main class="learn-app-page min-h-screen bg-[#f5f9ff] text-slate-950"><section class="mx-auto w-full max-w-6xl px-4 py-6 sm:px-6">
  <div class="flex flex-wrap items-end justify-between gap-3"><div><p class="text-sm font-semibold text-sky-700">学习进度</p><h1 class="mt-2 break-words text-3xl font-bold tracking-tight sm:text-4xl">)HTML" + htmlEscape(data.value("courseTitle", courseId)) + R"HTML(</h1><p class="mt-2 text-sm text-slate-600">)HTML" + (updated.empty() ? std::string("每一次理解、练习和完成都会汇总到这里。") : "最近同步：" + htmlEscape(updated)) + R"HTML(</p></div><div class="flex flex-wrap gap-2">)HTML" + contLink + resetControl + R"HTML(</div></div>
  <section class="mt-6 grid gap-4 md:grid-cols-3">
    <article class="rounded-2xl border border-sky-100 bg-white p-5"><p class="text-sm text-slate-500">总体完成度</p><b class="mt-2 block text-4xl text-sky-700">)HTML" + std::to_string(overall) + R"HTML(%</b><div class="mt-4 h-3 overflow-hidden rounded-full bg-slate-200"><i class="block h-full rounded-full bg-sky-600" style="width: )HTML" + std::to_string(overall) + R"HTML(%"></i></div></article>
    <article class="rounded-2xl border border-sky-100 bg-white p-5"><p class="text-sm text-slate-500">已完成项目</p><b class="mt-2 block text-4xl">)HTML" + std::to_string(completed) + R"HTML(</b><p class="mt-2 text-sm text-slate-500">理解、卡片与任务</p></article>
    <article class="rounded-2xl border border-sky-100 bg-white p-5"><p class="text-sm text-slate-500">课程项目总数</p><b class="mt-2 block text-4xl">)HTML" + std::to_string(total) + R"HTML(</b><p class="mt-2 text-sm text-slate-500">按最新快照估算</p></article>
  </section>
  )HTML" + (hasBp ? R"HTML(<section class="mt-6 rounded-2xl border border-sky-100 bg-white p-5"><p class="text-sm font-semibold text-slate-700">学习断点</p><p class="mt-2 rounded-xl bg-slate-50 p-4 text-sm text-slate-600">)HTML" + htmlEscape(bpText) + R"HTML(</p></section>)HTML" : std::string()) + R"HTML(
  <section class="mt-6"><div class="mb-3"><h2 class="text-xl font-semibold">分阶段进度</h2></div><div class="grid gap-4 md:grid-cols-2">)HTML" + (phaseCards.empty() ? R"HTML(<p class="text-sm text-slate-500 md:col-span-2">暂无可展示的阶段，先到课程里生成学习计划。</p>)HTML" : phaseCards) + R"HTML(</div></section>
</section></main>)HTML";
    const std::string resetScript = courseId.empty() ? "" : R"HTML(<script>(function(){const button=document.getElementById('reset-course-progress');if(!button)return;button.addEventListener('click',async()=>{if(!confirm('确定要清空这门课程的学习进度吗？课程内容不会删除。'))return;button.disabled=true;button.textContent='正在重置…';try{const response=await fetch('/api/course-progress',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'reset',courseId:)HTML" + jsString(courseId) + R"HTML(,anonymousId:)HTML" + jsString(anonymousId) + R"HTML(})});if(!response.ok)throw new Error();location.reload()}catch(_){button.disabled=false;button.textContent='重置失败，请重试'}})})();</script>)HTML";
    return document("学习进度 - 钢一定制AI", body, resetScript);
}
#endif

std::string renderAskPage(const std::string& question) {
    const std::string body = headerShell("ask") + R"HTML(
<main class="min-h-screen bg-[#f5f9ff] text-slate-950"><section class="mx-auto w-full max-w-6xl px-4 py-6 sm:px-6 lg:py-10">
  <header class="rounded-3xl border border-sky-100 bg-white p-6 shadow-sm shadow-sky-900/5 sm:p-8"><p class="text-sm font-semibold text-sky-700">AI 对话</p><h1 class="mt-2 text-3xl font-bold tracking-tight sm:text-4xl">有什么问题，直接问我</h1><p class="mt-3 text-base leading-7 text-slate-600">由当前配置的 AI 模型直接回答，不经过课程生成链路。</p></header>
  <section id="ask-messages" class="mt-6 space-y-4 rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-6"><p id="ask-empty" class="rounded-2xl border border-dashed border-sky-200 bg-sky-50/60 p-8 text-center text-sm leading-6 text-slate-600">还没有问题。你可以点击示例，或输入学习中遇到的具体困难。</p></section>
  <section class="sticky bottom-0 mt-6 rounded-3xl border border-sky-100 bg-white p-4 shadow-lg shadow-sky-900/10 sm:p-5"><form id="ask-form" class="flex flex-col gap-3 sm:flex-row sm:items-end"><div class="flex-1"><label class="sr-only" for="ask-question">输入你的问题</label><textarea id="ask-question" required rows="3" class="min-h-24 w-full resize-y rounded-2xl border border-slate-200 bg-slate-50 px-4 py-3 text-base leading-7 text-slate-900 outline-none transition focus:border-sky-500 focus:bg-white focus:ring-4 focus:ring-sky-100" placeholder="请输入你的问题"></textarea></div><button id="ask-submit" class="inline-flex min-h-12 items-center justify-center rounded-xl bg-sky-700 px-6 text-sm font-semibold text-white transition hover:bg-sky-800" type="submit">发送</button></form></section>
</section></main>)HTML";
    const std::string script = R"HTML(<script>(()=>{const I=)HTML" + jsString(question) + R"HTML(,$=id=>document.getElementById(id),esc=value=>{const node=document.createElement('div');node.textContent=value??'';return node.innerHTML},messages=$('ask-messages'),input=$('ask-question'),submit=$('ask-submit');function add(html,right=false){const item=document.createElement('article');item.className=right?'flex justify-end':'flex justify-start';item.innerHTML='<div class="max-w-[92%] whitespace-pre-wrap rounded-3xl px-5 py-4 text-sm leading-7 '+(right?'bg-sky-700 text-white':'border border-slate-200 bg-slate-50 text-slate-700')+'">'+html+'</div>';messages.append(item);item.scrollIntoView({behavior:'smooth',block:'end'});return item}async function ask(question){question=question.trim();if(!question)return;$('ask-empty')?.remove();add(esc(question),true);input.value='';submit.disabled=true;submit.textContent='正在思考…';const pending=add('钢一定制AI 正在思考…');try{const response=await fetch('/api/ask',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({question})});const data=await response.json();if(!response.ok)throw new Error(data.error||'回答暂未生成完成');pending.querySelector('div').textContent=data.answer;history.replaceState({},'', '/ask?question='+encodeURIComponent(question))}catch(error){pending.querySelector('div').textContent=error.message||'回答暂未生成完成，请稍后重试。'}finally{submit.disabled=false;submit.textContent='发送';input.focus()}}$('ask-form').addEventListener('submit',event=>{event.preventDefault();ask(input.value)});input.addEventListener('keydown',event=>{if(event.key==='Enter'&&!event.shiftKey){event.preventDefault();$('ask-form').requestSubmit()}});if(I){input.value=I;ask(I)}})()</script>)HTML";
    return document("AI 对话 - 钢一定制AI", body, script);
}

std::string renderMyCoursesPage(const nlohmann::json& data) {
    const auto str = [](const nlohmann::json& v) { return v.is_string() ? v.get<std::string>() : std::string(); };
    const auto num = [](const nlohmann::json& v) { return v.is_number() ? v.get<int>() : 0; };
    const std::string anonymousId = str(data.value("anonymousId", ""));
    const bool authenticated = data.value("authenticated", false);
    const auto stats = data.value("stats", nlohmann::json::object());
    const int total = num(stats.value("total", 0));
    std::string cardsHtml;
    for (const auto& c : data.value("courses", nlohmann::json::array())) {
        const std::string title = str(c.value("title", ""));
        const std::string goal = str(c.value("goal", ""));
        const std::string source = str(c.value("source", "ai"));
        const std::string createdAt = str(c.value("createdAt", ""));
        const std::string courseId = str(c.value("courseId", ""));
        const std::string sourceLabel = source == "fallback" ? "应急内容" : (source == "mock" || source == "template" ? "示例" : "AI 生成");
        cardsHtml += R"HTML(<article class="rounded-3xl border border-sky-100 bg-white p-5 shadow-sm shadow-sky-900/5 sm:p-6">
  <div class="flex flex-wrap items-center justify-between gap-2"><div class="min-w-0"><p class="text-xs font-semibold text-sky-700">)HTML" + htmlEscape(sourceLabel) + R"HTML(</p><h2 class="mt-1 break-words text-xl font-semibold text-slate-950">)HTML" + htmlEscape(title) + R"HTML(</h2></div></div>
  <p class="mt-3 break-words text-sm leading-7 text-slate-600">)HTML" + htmlEscape(goal) + R"HTML(</p>
  <div class="mt-4 flex flex-wrap gap-2"><button type="button" class="delete-course inline-flex min-h-10 items-center justify-center rounded-xl px-3 text-sm font-semibold text-slate-500 transition hover:bg-rose-50 hover:text-rose-700" data-course-id=")HTML" + htmlEscape(courseId) + R"HTML(">删除</button></div>
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
    <section class="mt-6 grid gap-4 sm:grid-cols-3"><article class="rounded-2xl border border-sky-100 bg-white p-5"><p class="text-sm text-slate-500">全部课程</p><b class="mt-2 block text-3xl text-sky-700">)HTML" + std::to_string(total) + R"HTML(</b></article></section>
    <p id="course-message" class="mt-4 text-sm text-slate-600" aria-live="polite"></p>
    <section class="mt-6 grid gap-4 lg:grid-cols-2">)HTML"
        + (cardsHtml.empty() ? R"HTML(<div class="lg:col-span-2 rounded-3xl border border-dashed border-slate-200 bg-white p-8 text-center"><p class="text-slate-600">还没有课程。去首页输入你的学习目标，先生成一份课程计划。</p><a class="mt-4 inline-flex min-h-11 items-center justify-center rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white" href="/">去首页生成课程</a></div>)HTML" : cardsHtml)
        + R"HTML(</section>
  </section>
</main>)HTML";
    const std::string script = R"HTML(<script>(function(){let anonymousId=)HTML" + jsString(anonymousId) + R"HTML(,authenticated=)HTML" + (authenticated ? "true" : "false") + R"HTML(;if(!authenticated&&!anonymousId){try{anonymousId=localStorage.getItem('gangyi-anonymous-id')||'anon_'+(typeof crypto!=='undefined'&&typeof crypto.randomUUID==='function'?crypto.randomUUID():Date.now().toString(36)+'-'+Math.random().toString(36).slice(2));localStorage.setItem('gangyi-anonymous-id',anonymousId)}catch(_){anonymousId='anon_'+Date.now().toString(36)+'-'+Math.random().toString(36).slice(2)}location.replace('/my-courses?anonymousId='+encodeURIComponent(anonymousId));return}const message=document.getElementById('course-message');document.querySelectorAll('.delete-course').forEach(button=>button.addEventListener('click',async()=>{if(!confirm('确定删除这门课程吗？学习进度和课程内容将一并删除。'))return;button.disabled=true;try{const query=anonymousId?'?anonymousId='+encodeURIComponent(anonymousId):'';const response=await fetch('/api/my-courses/'+encodeURIComponent(button.dataset.courseId)+query,{method:'DELETE'});if(!response.ok)throw new Error();location.reload()}catch(_){button.disabled=false;message.textContent='删除失败，请稍后重试。'}}))})();</script>)HTML";
    return document("我的课程 - 钢一定制AI", body, script);
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
    const std::string modeLabel = mode == "lite" ? "快速规划" : "深度课程规划";
    const std::string modeDesc = mode == "lite" ? "轻量学习课程：阶段内容更聚焦，保留关键讲解和练习。" : "系统学习课程：阶段讲解、任务、课件和资料更完整。";
    if (suitable.empty()) suitable = "适合正在学习「" + goal + "」并准备完成「" + stageTitle + "」阶段任务的学习者。";
    const std::string backHref = courseId.empty()
        ? "/plan?goal=" + urlEncode(goal) + "&mode=" + urlEncode(mode) + anonQ
        : "/plan?courseId=" + urlEncode(courseId) + anonQ;
    int nextTopicNo = 1;
    std::string nextTopicTitle;
    for (const auto& [title, no] : topics) {
        if (cardStatus.value(std::to_string(no), "not_started") != "completed") {
            nextTopicNo = no;
            nextTopicTitle = title;
            break;
        }
    }
    if (nextTopicTitle.empty() && !topics.empty()) {
        nextTopicNo = topics.front().second;
        nextTopicTitle = topics.front().first;
    }
    const std::string firstHref = courseId.empty()
        ? "/learn?goal=" + urlEncode(goal) + "&mode=" + urlEncode(mode)
            + "&phaseIndex=" + std::to_string(index) + "&phaseName=" + urlEncode(stageTitle)
            + "&topicIndex=" + std::to_string(nextTopicNo) + "&topic=" + urlEncode(nextTopicTitle) + anonQ
        : "/learn?courseId=" + urlEncode(courseId)
            + "&phaseIndex=" + std::to_string(index) + "&phaseName=" + urlEncode(stageTitle)
            + "&topicIndex=" + std::to_string(nextTopicNo) + "&topic=" + urlEncode(nextTopicTitle)
            + "&goal=" + urlEncode(goal) + "&mode=" + urlEncode(mode) + anonQ;
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
            <div class="flex flex-wrap items-center justify-between gap-3 text-sm font-semibold text-slate-700"><span>)HTML" + progressState + R"HTML(</span><span>)HTML" + std::to_string(percent) + R"HTML(% · )HTML" + std::to_string(completed) + R"HTML(/)HTML" + std::to_string(total) + R"HTML( 节</span></div>
            <div class="mt-3 h-2.5 overflow-hidden rounded-full bg-slate-100"><div class="h-full rounded-full bg-sky-700" style="width: )HTML" + std::to_string(percent) + R"HTML(%"></div></div>
          </div>
          <div class="mt-5 max-w-full rounded-2xl border border-sky-100 bg-sky-50 px-4 py-3 text-sm shadow-sm sm:w-fit">
            <p class="font-semibold text-sky-800">当前模式：)HTML" + modeLabel + R"HTML(</p>
            <p class="mt-1 leading-6 text-slate-600">)HTML" + modeDesc + R"HTML(</p>
          </div>
        </div>
        <div class="grid min-w-0 gap-3 md:grid-cols-2 lg:grid-cols-1">
          <a class="inline-flex min-h-12 items-center justify-center gap-2 rounded-2xl bg-sky-700 px-4 text-sm font-semibold text-white transition hover:bg-sky-800" href=")HTML" + firstHref + R"HTML(">)HTML" + primaryLabel + R"HTML(</a>
          <a class="inline-flex min-h-12 items-center justify-center gap-2 rounded-2xl border border-sky-200 bg-sky-50 px-4 text-sm font-semibold text-sky-800 transition hover:bg-sky-100" href=")HTML" + askHref + R"HTML(">问问钢一定制AI</a>
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
 parts.push(block('分步讲解',arr(x.steps).map((s,i)=>'<div class="rounded-2xl border border-sky-100 bg-sky-50/50 p-4"><p class="text-xs font-semibold text-sky-700">第 '+(i+1)+' 步</p><h3 class="mt-1 font-semibold text-slate-950">'+esc(s.title||('第 '+(i+1)+' 步'))+'</h3>'+(s.explanation?'<p class="mt-2 text-sm leading-7 text-slate-600">'+esc(s.explanation)+'</p>':'')+(s.example?'<p class="mt-2 text-sm text-slate-700"><b>示例：</b>'+esc(s.example)+'</p>':'')+(s.action?'<p class="mt-2 text-sm text-slate-700"><b>行动：</b>'+esc(s.action)+'</p>':'')+(s.check?'<p class="mt-2 text-sm text-slate-700"><b>检查：</b>'+esc(s.check)+'</p>':'')+'</div>')));
 parts.push(block('阶段任务',arr(x.tasks).map((t,i)=>'<div class="rounded-2xl border border-slate-200 bg-white p-4"><p class="font-semibold text-slate-950">'+esc(t.title||('任务 '+(i+1)))+'</p>'+(t.duration?'<p class="mt-1 text-sm text-slate-500">预计 '+esc(t.duration)+'</p>':'')+(t.description?'<p class="mt-2 text-sm leading-7 text-slate-600">'+esc(t.description)+'</p>':'')+(t.output?'<p class="mt-2 rounded-xl bg-slate-50 p-3 text-sm text-slate-700"><b>产出：</b>'+esc(t.output)+'</p>':'')+'</div>')));
 if(arr(x.checklist).length)parts.push('<div class="mt-5 rounded-2xl border border-emerald-100 bg-emerald-50 p-4"><p class="text-xs font-semibold uppercase tracking-[0.16em] text-emerald-700">阶段验收清单</p><div class="mt-2 grid gap-2">'+arr(x.checklist).map(s=>'<p class="text-sm text-emerald-900">✓ '+esc(s)+'</p>').join('')+'</div></div>');
 if(arr(x.commonMistakes).length)parts.push('<div class="mt-5"><p class="text-xs font-semibold uppercase tracking-[0.16em] text-rose-700">常见错误</p>'+pills(arr(x.commonMistakes))+'</div>');
 box.innerHTML=parts.join('')||'<p class="text-sm text-slate-500">暂无展开内容，可在阶段页重新生成。</p>';
}
function failed(){
 box.innerHTML='<div class="rounded-2xl border border-amber-100 bg-amber-50 p-5 text-center"><p class="font-semibold text-amber-800">阶段内容暂未生成完成</p><p class="mt-2 text-sm text-slate-600">AI 暂时无法生成讲解与任务，请稍后重试或返回课程页重新生成。</p><button type="button" onclick="location.reload()" class="mt-4 inline-flex min-h-11 items-center justify-center rounded-xl bg-sky-700 px-4 text-sm font-semibold text-white">重新生成阶段</button></div>';
}
fetch('/api/phase-expansion',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({courseId:C||undefined,anonymousId:A||undefined,goal:G,mode:M,phaseIndex:Number(P)||1,stage:N,topics})})
 .then(r=>r.json()).then(d=>{if(d.ok&&d.phase){render(d.phase);window.dispatchEvent(new CustomEvent('gangyi-phase-loaded',{detail:d}));}else failed();}).catch(failed);
})()</script>)HTML";
    const std::string interactionScript = R"HTML(<script>(()=>{
const C=)HTML" + jsString(courseId) + R"HTML(,A=)HTML" + jsString(anonymousId) + R"HTML(,G=)HTML" + jsString(goal) + R"HTML(,M=)HTML" + jsString(mode) + R"HTML(,P=)HTML" + std::to_string(index) + R"HTML(,N=)HTML" + jsString(stageTitle) + R"HTML(;
const box=document.getElementById('phase-expansion-body');
const key='gangyi-phase-progress:'+encodeURIComponent([C,A,G,M,P].join(':'));
const esc=value=>{const node=document.createElement('div');node.textContent=value??'';return node.innerHTML};
const arr=value=>Array.isArray(value)?value:[];
let saved={tasks:{},steps:{}};
try{saved=Object.assign(saved,JSON.parse(localStorage.getItem(key)||'{}'))}catch(_){ }
let detail=null;
function store(){localStorage.setItem(key,JSON.stringify(saved))}
function stateLabel(kind,value){return kind==='task'?({not_started:'未开始',in_progress:'进行中',completed:'已完成'}[value]||'未开始'):({unset:'未标记',understood:'已理解',review:'需要复习'}[value]||'未标记')}
function controlHtml(kind,index,value){const values=kind==='task'?['not_started','in_progress','completed']:['unset','understood','review'];return '<div class="mobile-button-stack mt-4 flex flex-wrap gap-2" data-phase-controls="'+kind+'-'+index+'">'+values.map(option=>'<button type="button" data-phase-kind="'+kind+'" data-phase-index="'+index+'" data-phase-status="'+option+'" class="min-h-10 rounded-full border px-3 text-sm font-semibold '+(option===value?'border-sky-700 bg-sky-700 text-white':'border-slate-200 bg-white text-slate-700 hover:bg-sky-50')+'">'+stateLabel(kind,option)+'</button>').join('')+'</div>'}
function paint(kind,index,value){saved[kind==='task'?'tasks':'steps'][index]=value;store();const controls=box.querySelector('[data-phase-controls="'+kind+'-'+index+'"]');if(!controls)return;controls.querySelectorAll('button').forEach(button=>{const active=button.dataset.phaseStatus===value;button.className='min-h-10 rounded-full border px-3 text-sm font-semibold '+(active?'border-sky-700 bg-sky-700 text-white':'border-slate-200 bg-white text-slate-700 hover:bg-sky-50')})}
function resourceHtml(resources){if(!resources.length)return '';return '<section class="mt-7 border-t border-sky-100 pt-6"><p class="text-xs font-semibold uppercase tracking-[.16em] text-sky-700">推荐资源</p><div class="mt-3 grid gap-3 md:grid-cols-2">'+resources.map(resource=>{const url=String(resource.url||''),href=url.startsWith('http')?url:'#';return '<a target="_blank" rel="noreferrer" href="'+esc(href)+'" class="block rounded-2xl border border-sky-100 bg-sky-50/50 p-4"><p class="text-xs font-semibold text-sky-700">'+esc(resource.type||resource.source||'学习资源')+'</p><h4 class="mt-1 font-semibold text-slate-950">'+esc(resource.title||'参考资料')+'</h4><p class="mt-2 text-sm leading-6 text-slate-600">'+esc(resource.description||'点击查看相关学习内容。')+'</p></a>'}).join('')+'</div></section>'}
function enhance(){if(!detail||!detail.phase)return;const sections=Array.from(box.children).filter(node=>node.classList&&node.classList.contains('mt-5'));const steps=arr(detail.phase.steps),tasks=arr(detail.phase.tasks);const stepCards=sections[0]?Array.from(sections[0].querySelectorAll(':scope > div > div')):[];const taskCards=sections[1]?Array.from(sections[1].querySelectorAll(':scope > div > div')):[];stepCards.forEach((card,index)=>card.insertAdjacentHTML('beforeend',controlHtml('step',index,saved.steps[index]||'unset')));taskCards.forEach((card,index)=>card.insertAdjacentHTML('beforeend',controlHtml('task',index,saved.tasks[index]||'not_started')));box.insertAdjacentHTML('beforeend',resourceHtml(arr(detail.resources)));}
function load(kind){const params=new URLSearchParams({courseId:C,anonymousId:A,goal:G,mode:M,phaseIndex:String(P)});return fetch('/api/'+(kind==='task'?'task-progress':'learning-step-progress')+'?'+params).then(response=>response.ok?response.json():{items:[]}).catch(()=>({items:[]}))}
function persist(kind,index,status){const row=kind==='task'?arr(detail.phase.tasks)[index]:arr(detail.phase.steps)[index];const payload={courseId:C||undefined,anonymousId:A||undefined,goal:G,mode:M,phaseIndex:Number(P),phaseName:N,status};if(kind==='task'){payload.taskIndex=index;payload.taskTitle=row?.title||('任务 '+(index+1))}else{payload.stepIndex=index;payload.stepTitle=row?.title||('第 '+(index+1)+' 步')}fetch('/api/'+(kind==='task'?'task-progress':'learning-step-progress'),{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)}).catch(()=>null)}
document.addEventListener('click',event=>{const button=event.target.closest('[data-phase-kind]');if(!button||!detail)return;const kind=button.dataset.phaseKind,index=Number(button.dataset.phaseIndex),status=button.dataset.phaseStatus;paint(kind,index,status);persist(kind,index,status)});
window.addEventListener('gangyi-phase-loaded',event=>{detail=event.detail||null;if(!detail||!detail.phase)return;Promise.all([load('task'),load('step')]).then(([taskData,stepData])=>{arr(taskData.items).forEach(item=>{if(item.status)saved.tasks[item.taskIndex]=item.status});arr(stepData.items).forEach(item=>{if(item.status)saved.steps[item.stepIndex]=item.status});store();enhance()})});
})()</script>)HTML";
    return document("阶段 - 钢一定制AI", body, script + interactionScript);
}

}  // namespace gangyi

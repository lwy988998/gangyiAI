#include "page_renderer.hpp"

#include <map>
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

std::string document(const std::string& title, const std::string& body, const std::string& script = {},
                     const std::string& headExtra = {}, const std::string& bodyClassExtra = {},
                     const std::string& bodyAttributes = {}) {
    return "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><meta name=\"theme-color\" content=\"#06090d\"><title>" +
        htmlEscape(title) + "</title><link rel=\"icon\" type=\"image/png\" href=\"/school-logo.png\"><link rel=\"stylesheet\" href=\"/styles.css\"><link rel=\"stylesheet\" href=\"/plan.css\"><link rel=\"stylesheet\" href=\"/dark.css\"><link rel=\"stylesheet\" href=\"/app-layout.css\"><script defer src=\"/app-layout.js\"></script><link rel=\"stylesheet\" href=\"/classroom.css\"><link rel=\"stylesheet\" href=\"/learning-flow.css\"><link rel=\"stylesheet\" href=\"/profile-radar.css\"><link rel=\"stylesheet\" href=\"/vendor/katex/katex.min.css\"><script defer src=\"/vendor/katex/katex.min.js\"></script><script defer src=\"/chat-render.js\"></script><script defer src=\"/profile-radar.js\"></script><script defer src=\"/dark.js\"></script><script defer src=\"/learning-agent.js\"></script><script defer src=\"/agent-shell.js\"></script><script defer src=\"/learning-flow.js\"></script><script defer src=\"/classroom.js\"></script>" + headExtra + "</head><body class=\"antialiased page-transition dark-app gy-theme-lab" + bodyClassExtra + "\"" + bodyAttributes + ">" + body +
        "<footer class=\"dark-footer\"><span>钢一定制AI</span><span>让学习有路径，让进步看得见。</span><span>本机学习数据 · 私密可控</span></footer>" + script + "</body></html>";
}

std::string navLink(const std::string& href, const std::string& label, bool active) {
    return std::string("<a class=\"dark-nav-link") + (active ? " is-active" : "") +
        "\" href=\"" + href + "\">" + label + "</a>";
}

std::string headerShell(const std::string& active) {
    const auto is = [&](const std::string& key) { return active == key; };
    const std::string links = navLink("/", "首页", is("home")) +
        navLink("/my-courses", "我的课程", is("my-courses")) +
        std::string("<a data-current-course-link class=\"dark-nav-link") + (is("learn") ? " is-active" : "") +
        "\" aria-disabled=\"true\" tabindex=\"-1\" title=\"暂未选择课程\">当前课程</a>" + navLink("/ask", "AI 导师", is("ask"));
    return R"HTML(<header class="site-header dark-header"><div class="dark-nav-shell"><a class="dark-brand" href="/"><img class="dark-brand-mark" src="/school-logo.png" alt="柳州市钢一中学校徽"><span>钢一<b>定制AI</b></span></a><nav class="dark-nav-links" aria-label="主导航">)HTML" + links +
        R"HTML(</nav><a class="dark-nav-cta" href="/my-courses#profile">我的画像 ↗</a><details class="dark-mobile-nav"><summary aria-label="打开菜单">☰</summary><nav aria-label="移动导航">)HTML" + links +
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

namespace {

std::string renderHomePageContent(bool startup) {
    const std::string body = headerShell("home") + R"HTML(
<main class="home-page min-h-screen text-slate-950">
  <section class="home-hero mx-auto flex min-h-[calc(100vh-3.25rem)] w-full max-w-6xl flex-col items-center px-4 pb-8 pt-3 text-center sm:px-6 sm:pt-5">
    <img src="/school-logo.png" alt="柳州市钢一中学校徽" width="680" height="680" fetchpriority="high" class="home-school-logo shrink-0 object-contain"><div class="home-intro"><p class="text-xs font-semibold uppercase tracking-[0.14em] text-sky-800">柳州市钢一中学 · AI 学习平台</p><h1 class="mt-1 max-w-3xl text-2xl font-bold tracking-tight text-sky-900 sm:text-4xl">让每个学习目标，<em>都看见进步。</em></h1><p class="mt-1 max-w-xl text-sm leading-6 text-slate-700 sm:text-base">从真实学习记录建立本机专属画像，让 AI 为每个知识点找到更合适的讲解和练习。</p></div>
    <form id="goal-form" class="home-goal-form mt-4 w-full max-w-3xl rounded-2xl border border-slate-200 bg-white p-2 text-left shadow-lg shadow-sky-900/10" action="/plan" method="get"><input id="goal-image" type="file" accept="image/*" class="hidden"><input id="goal-mode" type="hidden" name="mode" value="deep">
      <div class="home-goal-surface overflow-hidden rounded-[22px] border border-slate-100 bg-white"><div class="px-4 pb-4 pt-4 sm:px-5"><label class="hidden text-sm font-semibold text-sky-900 sm:block" for="goal">今天想提升哪一门高中课程？</label><textarea id="goal" name="goal" required rows="3" class="mt-1 block min-h-24 w-full resize-none border-0 bg-transparent text-base leading-7 text-slate-950 outline-none placeholder:text-slate-500 sm:text-lg" placeholder="例如：高一数学函数与图像"></textarea><div class="mt-3 flex flex-wrap items-center gap-2"><button id="pick-image" type="button" class="home-secondary-button inline-flex min-h-10 items-center justify-center rounded-full border border-slate-200 bg-white px-3 text-sm font-semibold text-sky-800 hover:bg-sky-50">＋ 上传参考图</button><span class="flex-1"></span><button id="goal-submit" type="submit" class="inline-flex h-11 min-w-11 items-center justify-center rounded-full bg-sky-700 px-4 text-sm font-semibold text-white hover:bg-sky-800">生成 →</button></div></div><div class="home-goal-modes grid border-t border-slate-100 bg-slate-50/70 sm:grid-cols-2"><button type="button" data-mode="lite" class="home-mode-panel mode-panel p-4 text-left text-sm text-slate-700"><b class="block text-slate-950">快速规划</b><span class="mt-1 block">快速生成学习路线</span></button><button type="button" data-mode="deep" class="home-mode-panel mode-panel border-t border-slate-100 p-4 text-left text-sm text-slate-700 sm:border-l sm:border-t-0"><b class="block text-sky-900">深度课程 · 推荐</b><span class="mt-1 block">搜索资料并生成完整课程</span></button></div></div>
      <div id="image-preview" class="hidden mt-3 items-center gap-3 rounded-2xl border border-sky-100 bg-sky-50 p-3"><img class="h-16 w-16 rounded-xl border border-white object-cover" alt="图片预览"><div class="min-w-0 flex-1"><p id="image-name" class="truncate text-sm font-semibold"></p><p id="image-meta" class="mt-1 text-xs text-slate-500"></p></div><button id="remove-image" type="button" class="rounded-xl border border-slate-200 bg-white px-3 py-2 text-sm text-slate-500">移除</button></div><p id="goal-message" class="hidden mt-3 rounded-xl bg-amber-50 px-3 py-2 text-sm text-amber-700"></p>
    </form>
    <div id="goal-examples" class="mt-4 flex max-w-4xl flex-wrap justify-center gap-2"></div>
    <aside class="home-profile-compact home-visual"><div class="profile-heading"><div><p>来自真实学习记录</p><h2>学习画像</h2></div><a href="/my-courses#profile">查看画像 ↗</a></div><div id="home-radar" data-profile-radar></div><p class="home-radar-note">从每一次真实作答，看见自己的进步。</p></aside>
  </section>
  <section id="profile" class="profile-section" aria-labelledby="profile-title">
    <div class="profile-heading"><div><p>继续学习</p><h2 id="profile-title">最近课程</h2></div><a class="recent-courses-all" href="/my-courses">全部课程 ↗</a></div>
    <p id="recent-courses-message" role="status" aria-live="polite">正在读取最近课程…</p>
    <div id="recent-courses" class="recent-courses" aria-label="最近三门课程"></div>
  </section>
</main>)HTML";
    const std::string script = R"HTML(<script>(()=>{const $=id=>document.getElementById(id),form=$('goal-form'),goal=$('goal'),file=$('goal-image'),mode=$('goal-mode'),preview=$('image-preview'),message=$('goal-message'),examples={lite:[],deep:[]};let selectedMode='deep',imageFile=null,imageUrl='';function setMode(value){selectedMode=value==='lite'?'lite':'deep';mode.value=selectedMode;document.querySelectorAll('[data-mode]').forEach(button=>{const active=button.dataset.mode===selectedMode;button.classList.toggle('bg-sky-700',active);button.classList.toggle('text-white',active);button.classList.toggle('border-sky-700',active);button.classList.toggle('bg-white',!active);button.classList.toggle('text-slate-600',!active)});renderExamples()}function renderExamples(){const target=$('goal-examples');target.replaceChildren();for(const item of examples[selectedMode]){const button=document.createElement('button');button.type='button';button.className='goal-example rounded-full border border-sky-100 bg-white/80 px-3 py-2 text-sm font-medium text-sky-900 hover:bg-sky-50';button.textContent=item;target.append(button)}}async function loadRecommendations(){try{const response=await fetch('/api/home/recommendations',{cache:'no-store'});if(!response.ok)return;const data=await response.json();if(data.items&&['lite','deep'].every(key=>Array.isArray(data.items[key])&&data.items[key].length>0&&data.items[key].every(item=>typeof item==='string'))){examples.lite=data.items.lite;examples.deep=data.items.deep;renderExamples()}if(data.updating||['pending','running'].includes(data.status))setTimeout(loadRecommendations,1000)}catch(_){}}loadRecommendations();function clearImage(){if(imageUrl)URL.revokeObjectURL(imageUrl);imageFile=null;imageUrl='';file.value='';preview.classList.add('hidden');preview.classList.remove('flex')}function showMessage(text){message.textContent=text;message.classList.remove('hidden')}function createAnonymousId(){const id=typeof crypto!=='undefined'&&typeof crypto.randomUUID==='function'?crypto.randomUUID():Date.now().toString(36)+'-'+Math.random().toString(36).slice(2);return 'anon_'+id}function anonymousId(){try{let id=localStorage.getItem('gangyi-anonymous-id');if(!id){id=createAnonymousId();localStorage.setItem('gangyi-anonymous-id',id)}return id}catch(_){return createAnonymousId()}}file.addEventListener('change',()=>{const picked=file.files?.[0];message.classList.add('hidden');if(!picked)return;if(!picked.type.startsWith('image/')){clearImage();showMessage('请上传图片文件');return}if(picked.size>5*1024*1024){clearImage();showMessage('图片过大，请上传 5MB 以内的图片');return}clearImage();imageFile=picked;imageUrl=URL.createObjectURL(picked);preview.querySelector('img').src=imageUrl;$('image-name').textContent=picked.name;$('image-meta').textContent=(picked.size/1024/1024).toFixed(1)+' MB · 将结合图片生成学习目标';preview.classList.remove('hidden');preview.classList.add('flex')});$('pick-image').onclick=()=>file.click();$('remove-image').onclick=clearImage;document.addEventListener('click',event=>{const modeButton=event.target.closest('[data-mode]');if(modeButton)setMode(modeButton.dataset.mode);const example=event.target.closest('.goal-example');if(example){goal.value=example.textContent;goal.focus()}});goal.addEventListener('keydown',event=>{if(event.key==='Enter'&&!event.shiftKey&&!event.isComposing){event.preventDefault();form.requestSubmit()}});form.addEventListener('submit',async event=>{event.preventDefault();const text=goal.value.trim();message.classList.add('hidden');if(!text&&!imageFile){showMessage('请输入学习需求，或上传一张相关图片');return}const submit=$('goal-submit');submit.disabled=true;submit.textContent=imageFile?'识别中…':'准备生成…';let target=text;if(imageFile){try{const data=new FormData();data.append('image',imageFile);data.append('prompt',text);data.append('mode',selectedMode);const response=await fetch('/api/analyze-image-goal',{method:'POST',body:data});const result=await response.json();if(result.success&&result.goal)target=result.goal;else if(!text){showMessage(result.message||'图片识别未完成，请补充文字描述后重试');submit.disabled=false;submit.textContent='生成 →';return}else showMessage(result.message||'图片识别未完成，已按文字描述生成课程')}catch(_){if(!text){showMessage('图片识别未完成，请补充文字描述后重试');submit.disabled=false;submit.textContent='生成 →';return}showMessage('图片识别未完成，已按文字描述生成课程')}}location.href='/plan?'+new URLSearchParams({goal:target,mode:selectedMode,anonymousId:anonymousId()})});setMode('deep')})()</script>)HTML";
    const std::string startupOverlay = R"HTML(<div id="startup-overlay" class="startup-overlay" role="status" aria-label="钢一定制AI正在启动"><div class="startup-overlay-grid"></div><div class="startup-scene"></div><div class="startup-overlay-brand"><small>柳州市钢一中学 · AI 学习平台</small><h2>钢一定制AI</h2></div><div class="startup-progress"><span></span></div><div class="startup-transition-layer" aria-hidden="true"></div></div>)HTML";
    const std::string startupHead = "<link rel=\"stylesheet\" href=\"/startup.css\"><script defer src=\"/startup.js\"></script>";
    const std::string startupAttributes = startup ? " data-startup-phase=\"intro\"" : "";
    return document("柳州市钢一定制AI - 把学习目标变成可执行课程",
        (startup ? startupOverlay : "") + body, script, startup ? startupHead : "",
        startup ? " startup-playing" : "", startupAttributes);
}

}  // namespace

std::string renderHomePage() {
    return renderHomePageContent(false);
}

std::string renderStartupPage() {
    return renderHomePageContent(true);
}

std::string renderPlanPage(const std::string& goal, const std::string& mode,
                           const std::string& courseId, const std::string& anonymousId) {
    const std::string body = headerShell("home") + R"HTML(
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
let initialGoal=)HTML" + jsString(goal) + R"HTML(,initialMode=)HTML" + jsString(mode == "lite" ? "lite" : "deep") + R"HTML(,courseId=)HTML" + jsString(courseId) + R"HTML(,anonymousId=)HTML" + jsString(anonymousId) + R"HTML(;
const $=id=>document.getElementById(id), loading=$('loading'),failure=$('failure'),result=$('result'),status=$('save-status'); let plan=null,renderMode=initialMode;
const esc=value=>{const d=document.createElement('div');d.textContent=value??'';return d.innerHTML}; const text=(v,f='暂无内容')=>esc(v||f); const arr=(v)=>Array.isArray(v)?v:[];
const list=(items,fn)=>arr(items).length?arr(items).map(fn).join(''):'<p class="empty-note">暂无内容</p>';
const card=(label,title,body,extra='')=>'<article class="plan-card '+extra+'"><p class="plan-kicker">'+text(label)+'</p><h2>'+text(title)+'</h2>'+body+'</article>';
const pill=v=>'<span class="plan-pill">'+text(v)+'</span>';
function mind(node,depth=0){return '<li class="mind-node depth-'+depth+'"><span>'+text(node.label||node.title)+'</span>'+((node.children||[]).length?'<ul>'+arr(node.children).map(x=>mind(x,depth+1)).join('')+'</ul>':'')+'</li>'}
function overview(){const stages=arr(plan.courseStructure),resources=arr(plan.resources),projects=arr(plan.projects);return '<div class="progress-strip"><div><b>0%</b><span>正在读取课程进度</span></div><div class="progress-track"><i></i></div></div><section class="course-stages"><h2>学习阶段</h2><div data-paged-list data-page-size="8">'+list(stages,(s,i)=>{const road=arr(plan.roadmap)[i]||{},query=new URLSearchParams({courseId,phaseIndex:String(i+1),...(anonymousId?{anonymousId}:{} )});return '<a class="outline-row" href="/phase?'+query+'"><span class="outline-number">'+String(i+1).padStart(2,'0')+'</span><div><h3>'+text(s.stage||road.name)+'</h3><p>'+text(s.goal||road.goal||road.description)+'</p><small>'+arr(s.topics).length+' 节</small></div><span class="stage-state" data-stage="'+i+'">查看阶段 →</span></a>'})+'</div></section><details class="course-more"><summary>更多课程内容</summary>'+card('知识结构',plan.mindMap?.title||'','<div class="mind-map"><ul>'+list(arr(plan.mindMap?.nodes),mind)+'</ul></div>','full-card')+card('精选资源','','<div class="resource-grid">'+list(resources,r=>'<a class="resource-item" target="_blank" rel="noreferrer" href="'+esc(/^https?:\/\//i.test(r.href||r.url||'')?(r.href||r.url):'#')+'"><h3>'+text(r.name||r.title)+'</h3><p>'+text(r.description)+'</p></a>')+'</div>','full-card')+card('作品任务','','<div class="project-grid">'+list(projects,p=>'<div><h3>'+text(p.name)+'</h3><p>'+text(p.output||p.description)+'</p></div>')+'</div>','full-card')+card('AI 课程路线预览','','<div id="course-preview" role="status">正在读取已保存路线…</div>','full-card')+'</details>'}
function preserveAnonymousIdInLearningLinks(){document.querySelectorAll('#plan-view a[href^="/learn?courseId="]').forEach(link=>{const url=new URL(link.href,location.origin);const rawPhase=Number(url.searchParams.get('phaseIndex')||'1'),rawTopic=Number(url.searchParams.get('topicIndex')||'1');const phaseNo=Number.isInteger(rawPhase)&&rawPhase>0?rawPhase:1,topicNo=Number.isInteger(rawTopic)&&rawTopic>0?rawTopic:1;url.searchParams.set('phaseIndex',String(phaseNo));url.searchParams.set('topicIndex',String(topicNo));if(initialGoal&&!url.searchParams.get('goal'))url.searchParams.set('goal',initialGoal);if(initialMode&&!url.searchParams.get('mode'))url.searchParams.set('mode',initialMode);const stage=arr(plan.courseStructure)[phaseNo-1]||arr(plan.roadmap)[phaseNo-1]||{};const phaseTitle=stage.stage||stage.name||'';if(phaseTitle&&!url.searchParams.get('phaseName'))url.searchParams.set('phaseName',phaseTitle);const topics=arr(stage.topics);const steps=arr(stage.steps);const item=topics[topicNo-1]||{};const topicTitle=(typeof item==='string'?item:item.title)||steps[topicNo-1]?.title||steps[topicNo-1]?.name||'';if(item.id)url.searchParams.set('topicId',item.id);if(item.legacyPhaseIndex)url.searchParams.set('phaseIndex',item.legacyPhaseIndex);if(item.legacyTopicIndex)url.searchParams.set('topicIndex',item.legacyTopicIndex);if(topicTitle&&!url.searchParams.get('topic'))url.searchParams.set('topic',topicTitle);if(anonymousId)url.searchParams.set('anonymousId',anonymousId);link.href=url.pathname+'?'+url.searchParams.toString()})}
async function restoreProgress(){if(!courseId)return;try{const data=await request('/api/courses/'+encodeURIComponent(courseId));const cards=arr(data.cards),stages=arr(plan.courseStructure),strip=document.querySelector('.progress-strip');const counts=stages.map((stage,phase)=>{const topics=arr(stage.topics);return {total:topics.length,done:topics.filter((topic,i)=>cards.some(c=>c.phaseIndex===(topic?.legacyPhaseIndex||phase+1)&&c.topicIndex===(topic?.legacyTopicIndex||i+1)&&c.status==='completed')).length}});const total=counts.reduce((sum,item)=>sum+item.total,0),done=counts.reduce((sum,item)=>sum+item.done,0),percent=total?Math.round(done*100/total):0;if(strip){strip.querySelector('b').textContent=percent+'%';strip.querySelector('span').textContent='课程进度 · 已完成 '+done+' / '+total;strip.querySelector('i').style.width=percent+'%';for(const node of document.querySelectorAll('[data-stage]')){const count=counts[Number(node.dataset.stage)];node.textContent=count.done+' / '+count.total+' 已完成 →';}}}catch(_){const strip=document.querySelector('.progress-strip');if(strip)strip.querySelector('span').textContent='课程进度暂时无法读取，原记录已保留';}}
function render(){ $('plan-title').textContent=plan.title||'你的课程总览';$('plan-goal').textContent=(plan.courseIntro||plan.summary||'')+(initialGoal?' · 目标：'+initialGoal:'');$('plan-view').innerHTML=overview();document.dispatchEvent(new Event('gangyi:layout'));window.GangyiNavigation?.setContext({courseId,phaseIndex:null});preserveAnonymousIdInLearningLinks();renderSlide(0);restoreProgress()}
function renderSlide(i){const s=arr(plan.slides)[i]||{};const el=$('slide-content');if(el)el.innerHTML='<h3>'+text(s.title)+'</h3><p>'+text(s.content)+'</p><div class="bullet-list">'+list(s.bullets,x=>'<span>'+text(x)+'</span>')+'</div>'}
function failureState(error){const type=error.type||'';const messages={timeout:'生成超时，请重试。',auth_error:'当前模型接口认证失败，请检查服务配置后重试。',rate_limited:'AI 服务请求过于频繁，请稍后重试。',invalid_response:'生成内容未通过质量检查，请重新生成。',json_parse_error:'生成内容未通过质量检查，请重新生成。',quality_rejected:'生成内容未通过质量检查，请重新生成。',missing_config:'当前模型接口尚未配置，请检查服务配置。'};failure.innerHTML='<div class="failure-panel"><div class="failure-mark">!</div><div><p class="plan-kicker">生成没有完成</p><h2>'+text(messages[type]||'AI 服务暂时不可用，请稍后重试。')+'</h2><p>'+text(error.message,'服务暂时没有返回可用内容。')+'</p><div class="failure-actions"><button type="button" class="primary-action" id="retry">重试生成</button><a class="secondary-action" href="/">返回首页</a></div></div></div>';failure.classList.remove('hidden');loading.classList.add('hidden');$('retry').onclick=()=>load(true)}
async function request(url,options={}){if(url.startsWith('/api/courses/')&&anonymousId&&!url.includes('?'))url+='?anonymousId='+encodeURIComponent(anonymousId);const controller=new AbortController();const timeoutMs=url==='/api/generate-plan'?250000:12000;const timer=setTimeout(()=>controller.abort(),timeoutMs);try{const r=await fetch(url,{...options,signal:controller.signal});const data=await r.json();if(!r.ok)throw Object.assign(new Error(data.message||data.error||'请求失败'),{type:data.type});return data}catch(error){if(controller.signal.aborted)throw Object.assign(new Error('AI 响应超时，请重试。'),{type:'timeout'});throw error}finally{clearTimeout(timer)}}
let planTaskId=new URLSearchParams(location.search).get('taskId')||'';
async function generatedPlan(force){
 if(force&&courseId)planTaskId='';
 const q=new URLSearchParams(location.search);let requestId=q.get('requestId');
 if(force&&planTaskId&&!courseId){const existing=await request('/api/learning-agent?taskId='+encodeURIComponent(planTaskId));if(['failed','paused','superseded'].includes(existing.status))await request('/api/learning-agent/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({command:'retry',taskId:planTaskId})});else planTaskId='';}
 if(!planTaskId){requestId=(force?'':requestId)||crypto.randomUUID();q.set('requestId',requestId);history.replaceState({},'','?'+q);const task=await request('/api/generate-plan',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({goal:initialGoal,mode:initialMode,requestId})});planTaskId=task.id;q.set('taskId',planTaskId);history.replaceState({},'','?'+q);}
 while(true){const task=await request('/api/learning-agent?taskId='+encodeURIComponent(planTaskId));status.textContent=task.message||'真实 AI 正在规划并保存课程…';if(task.status==='ready'&&task.course){courseId=task.course.id;const data=await request('/api/courses/'+encodeURIComponent(courseId));history.replaceState({},'',task.course.href);localStorage.setItem('gangyi-last-course',JSON.stringify({id:courseId,title:task.course.title,href:task.course.href}));return data;}if(['failed','paused','superseded','cancelled','waiting_student'].includes(task.status))throw new Error(task.error||task.message||'AI 已暂停，输入和有效步骤已保存，可重试。');await new Promise(resolve=>setTimeout(resolve,800));}
}
async function load(force=false){loading.classList.remove('hidden');failure.classList.add('hidden');status.textContent='正在加载';try{const data=courseId&&!force?await request('/api/courses/'+encodeURIComponent(courseId)):await generatedPlan(force);plan=data.snapshot.payload;initialGoal=data.course.goal;initialMode=data.course.mode;renderMode=data.course.mode==='lite'?'lite':'deep';if(!plan||!Array.isArray(plan.courseStructure)||!plan.courseStructure.length)throw new Error('AI 课程结构尚未完整保存');loading.classList.add('hidden');result.classList.remove('hidden');render();status.textContent='已恢复已保存课程';document.dispatchEvent(new CustomEvent('course-saved',{detail:{courseId,href:'/plan?courseId='+encodeURIComponent(courseId)}}));}catch(e){failureState(e)}}
document.addEventListener('click',e=>{const b=e.target.closest('[data-slide]');if(b){document.querySelectorAll('[data-slide]').forEach(x=>x.classList.remove('selected'));b.classList.add('selected');renderSlide(Number(b.dataset.slide))}});document.addEventListener('course-saved',e=>{if(e.detail?.href&&!courseId)location.replace(e.detail.href)});load();
})();</script>)HTML";
    return document("课程规划 - 钢一定制AI", body, script);
}

std::string renderClassroomPage(const std::string& view) {
    const std::string label = view == "practice" ? "练习" : view == "summary" ? "小结" : "讲解";
    const std::string body = headerShell("learn") + R"HTML(
<main class="lesson-page" data-lesson-view=")HTML" + htmlEscape(view) + R"HTML(">
  <div class="lesson-heading"><div><p class="uc-eyebrow">知识点 · 聊天课堂</p><h1 id="lesson-title">正在读取课堂</h1><p id="lesson-purpose"></p></div><nav class="lesson-tabs" aria-label="定位课堂内容"><button type="button" data-lesson-link="learn">讲解</button><button type="button" data-lesson-link="practice">练习</button><button type="button" data-lesson-link="summary">小结</button></nav></div>
  <p id="page-status" role="status" aria-live="polite"></p>
  <div id="lesson-scroll" tabindex="0" role="region" aria-label="课堂消息">
    <section id="lesson-sections" aria-label="教学内容与对话"></section>
    <section id="lesson-completion" class="lesson-completion"><details id="lesson-summary"><summary>本节小结与完成</summary><div id="lesson-summary-content"></div><p id="completion-note"></p><button id="finish-lesson" type="button">请 AI 确认本节完成情况</button><button id="prepare-next" type="button">请 AI 准备下一课 →</button></details></section>
  </div>
  <button id="lesson-latest" type="button" hidden>↓ 回到最新</button>
  <section id="lesson-chat-panel" class="lesson-chat-panel" aria-label="课堂输入">
    <div class="composer-context"><span id="composer-target">和 AI 老师交流</span><button id="composer-chat" type="button" class="secondary" hidden>自由交流</button></div>
    <form id="lesson-chat"><label class="sr-only" for="lesson-input">你的答案、问题或学习反馈</label><textarea id="lesson-input" rows="2" placeholder="输入答案、你的想法，或向 AI 老师追问…"></textarea><button id="lesson-send" type="submit">发送 ↑</button></form>
    <div class="composer-toolbar"><button id="explain-again" type="button" class="secondary">换一种讲法</button><button id="continue-teaching" type="button" class="secondary">请 AI 继续</button><button id="lesson-stop" type="button" class="secondary" hidden>停止回答</button><button id="lesson-retry" type="button" class="secondary" hidden>重试同次回答</button><span id="lesson-task-status" role="status">正在读取课堂</span></div>
  </section>
</main>)HTML";
    return document(label + " - 钢一定制AI", body, "<script defer src=\"/agent-classroom.js\"></script>", "<link rel=\"stylesheet\" href=\"/agent-classroom.css\">");
}

std::string renderLearnPage([[maybe_unused]] const std::string& courseId, [[maybe_unused]] const std::string& goal, [[maybe_unused]] const std::string& mode,
    [[maybe_unused]] const std::string& phaseIndex, [[maybe_unused]] const std::string& phaseName, [[maybe_unused]] const std::string& topicIndex,
    [[maybe_unused]] const std::string& topic, [[maybe_unused]] const std::string& anonymousId, [[maybe_unused]] const std::string& regenerate,
    [[maybe_unused]] const std::string& forceLearn, [[maybe_unused]] const std::string& retry) { return renderClassroomPage("learn"); }

std::string renderAskPage(const std::string& question) {
    const std::string body = headerShell("ask") + R"HTML(
<main class="min-h-screen bg-[#f5f9ff] text-slate-950"><section class="mx-auto w-full max-w-6xl px-4 py-6 sm:px-6 lg:py-10">
  <header class="rounded-3xl border border-sky-100 bg-white p-6 shadow-sm shadow-sky-900/5 sm:p-8"><p class="text-sm font-semibold text-sky-700">AI 对话</p><h1 class="mt-2 text-3xl font-bold tracking-tight sm:text-4xl">有什么问题，直接问我</h1><p class="mt-3 text-base leading-7 text-slate-600">由当前配置的 AI 模型直接回答，不经过课程生成链路。</p></header>
  <section id="ask-messages" class="mt-6 space-y-4 rounded-3xl border border-sky-100 bg-white p-4 shadow-sm shadow-sky-900/5 sm:p-6"><p id="ask-empty" class="rounded-2xl border border-dashed border-sky-200 bg-sky-50/60 p-8 text-center text-sm leading-6 text-slate-600">还没有问题。你可以点击示例，或输入学习中遇到的具体困难。</p></section>
  <section class="sticky bottom-0 mt-6 rounded-3xl border border-sky-100 bg-white p-4 shadow-lg shadow-sky-900/10 sm:p-5"><form id="ask-form" class="flex flex-col gap-3 sm:flex-row sm:items-end"><div class="flex-1"><label class="sr-only" for="ask-question">输入你的问题</label><textarea id="ask-question" required rows="3" class="min-h-24 w-full resize-y rounded-2xl border border-slate-200 bg-slate-50 px-4 py-3 text-base leading-7 text-slate-900 outline-none transition focus:border-sky-500 focus:bg-white focus:ring-4 focus:ring-sky-100" placeholder="请输入你的问题"></textarea></div><button id="ask-submit" class="inline-flex min-h-12 items-center justify-center rounded-xl bg-sky-700 px-6 text-sm font-semibold text-white transition hover:bg-sky-800" type="submit">发送</button></form></section>
</section></main>)HTML";
    const std::string script = "<script>window.gangyiInitialQuestion=" + jsString(question) + ";</script><script defer src=\"/ask.js\"></script>";
    return document("AI 对话 - 钢一定制AI", body, script);
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
        const std::string courseId = str(c.value("courseId", ""));
        const int coursePercent = num(c.value("percent", 0));
        const int courseDone = num(c.value("doneTopics", 0));
        const int courseTotal = num(c.value("totalTopics", 0));
        const int courseDue = num(c.value("dueCount", 0));
        const std::string sourceLabel = source == "fallback" ? "应急内容" : (source == "mock" || source == "template" ? "示例" : "AI 生成");
        cardsHtml += R"HTML(<article class="uc-card uc-course" data-course-id=")HTML" + htmlEscape(courseId) + R"HTML(">
  <div class="uc-course-top"><div class="min-w-0"><p class="uc-eyebrow">)HTML" + htmlEscape(sourceLabel) + R"HTML(</p><h3 class="uc-course-title">)HTML" + htmlEscape(title) + R"HTML(</h3></div>)HTML" +
            (courseDue > 0 ? R"HTML(<span class="uc-badge uc-badge-warn">待复习 )HTML" + std::to_string(courseDue) + R"HTML(</span>)HTML" : std::string()) + R"HTML(</div>
  <p class="uc-course-goal">)HTML" + htmlEscape(goal) + R"HTML(</p>
  <div class="uc-bar"><i style="width: )HTML" + std::to_string(coursePercent) + R"HTML(%"></i></div>
  <p class="uc-course-meta">完成 )HTML" + std::to_string(courseDone) + R"HTML( / )HTML" + std::to_string(courseTotal) + R"HTML( 节 · )HTML" + std::to_string(coursePercent) + R"HTML( %</p>
  <div class="uc-course-actions"><a class="uc-primary" href="/plan?courseId=)HTML" + htmlEscape(courseId) + R"HTML(">继续学习</a><button type="button" class="delete-course uc-ghost-danger" data-course-id=")HTML" + htmlEscape(courseId) + R"HTML(">删除</button></div>
  )HTML" + (createdAt.empty() ? "" : R"HTML(<p class="uc-course-created">创建于 )HTML" + htmlEscape(createdAt) + "</p>") + R"HTML(
</article>)HTML";
    }
    std::string dueHtml;
    for (const auto& item : data.value("dueReviews", nlohmann::json::array())) {
        dueHtml += R"HTML(<li><a href=")HTML" + htmlEscape(str(item.value("href", ""))) + R"HTML("><strong>)HTML" +
            htmlEscape(str(item.value("title", ""))) + R"HTML(</strong><span>)HTML" +
            htmlEscape(str(item.value("course", ""))) + R"HTML( · 到期 )HTML" + htmlEscape(str(item.value("due", ""))) + R"HTML(</span></a></li>)HTML";
    }
    const std::string body = headerShell("my-courses") + R"HTML(
<main class="uc-main"><div class="uc-shell"><div class="uc-content">
  <header class="uc-head"><div><p class="uc-eyebrow">本机学习档案</p><h1>我的课程</h1></div><div class="uc-head-actions"><a class="uc-primary" href="/">＋ 新课程</a><button id="open-api-settings" type="button">AI 设置</button></div></header>
  <p id="api-settings-message" class="uc-message" role="status" aria-live="polite"></p><p id="course-message" class="uc-message" aria-live="polite"></p>
  <div class="uc-tabs" role="tablist" aria-label="我的学习"><button id="courses-tab" type="button" role="tab" aria-controls="courses-panel" aria-selected="true" data-uc-tab="courses">我的课程</button><button id="profile-tab" type="button" role="tab" aria-controls="profile-panel" aria-selected="false" data-uc-tab="profile">我的画像</button></div>
  <section id="courses-panel" role="tabpanel" aria-labelledby="courses-tab"><div class="uc-course-grid" data-paged-list data-page-size="4">)HTML" +
      (cardsHtml.empty() ? "<p>还没有课程，去首页输入学习目标开始吧。</p>" : cardsHtml) + R"HTML(</div>
    <div id="uc-study-plan-anchor"></div><details class="uc-review-details"><summary>待复习清单</summary>)HTML" + (dueHtml.empty() ? "<p>目前没有到期复习。</p>" : "<ul class=\"uc-due-list\">" + dueHtml + "</ul>") + R"HTML(</details>
  </section>
  <section id="profile-panel" role="tabpanel" aria-labelledby="profile-tab" hidden><section id="uc-profile"><h2>我的画像</h2><div id="uc-profile-radar" data-profile-radar></div><details><summary>学科与知识点详情</summary><div id="uc-profile-subjects"></div><div id="uc-profile-topics"></div></details></section><section id="uc-time"><h2>学习时间</h2><p id="uc-availability-summary" role="status"></p><fieldset class="uc-availability"><legend class="sr-only">每周可学习时间</legend><div id="uc-availability-grid" class="availability-grid"></div></fieldset></section></section>
</div></div></main>)HTML";
    const std::string script = R"HTML(<script defer src="/user-center.js"></script>)HTML";
    return document("我的课程 - 钢一定制AI", body, script);
}

std::string renderPhasePage(const std::string& courseId, const std::string& anonymousId,
    const std::string& goal, const std::string& mode, const std::string& phaseIndex, const std::string& phaseName,
    const nlohmann::json& plan, const nlohmann::json& cardStatus) {
    int selected = 1; try { selected = std::max(1, std::stoi(phaseIndex)); } catch (...) { }
    const auto stages = plan.value("courseStructure", nlohmann::json::array());
    const auto roadmap = plan.value("roadmap", nlohmann::json::array());
    const std::string back = "/plan?courseId=" + urlEncode(courseId) + (anonymousId.empty() ? "" : "&anonymousId=" + urlEncode(anonymousId));
    if (!stages.is_array() || selected > static_cast<int>(stages.size()))
        return document("阶段 - 钢一定制AI", headerShell("learn") + "<main class=\"phase-page\"><h1>阶段内容暂未生成完成</h1><a href=\"" + htmlEscape(back) + "\">返回课程</a></main>");
    const auto stage = stages.at(selected - 1);
    const auto road = roadmap.is_array() && selected <= static_cast<int>(roadmap.size()) ? roadmap.at(selected - 1) : nlohmann::json::object();
    const std::string title = stage.value("stage", road.value("name", phaseName));
    const std::string stageGoal = stage.value("goal", road.value("goal", road.value("description", goal)));
    const std::string why = stage.value("why", road.value("why", ""));
    std::string rows; int number = 0;
    for (const auto& topic : stage.value("topics", nlohmann::json::array())) {
        ++number; const std::string name = topic.is_string() ? topic.get<std::string>() : topic.value("title", "");
        const int phase = topic.is_object() ? topic.value("legacyPhaseIndex", selected) : selected;
        const int index = topic.is_object() ? topic.value("legacyTopicIndex", number) : number;
        const std::string id = topic.is_object() ? topic.value("id", "") : "";
        const std::string description = topic.is_object() ? topic.value("description", topic.value("goal", "")) : "";
        const auto state = cardStatus.value(std::to_string(number), "not_started");
        const std::string href = "/learn?courseId=" + urlEncode(courseId) + "&phaseIndex=" + std::to_string(phase) + "&topicIndex=" + std::to_string(index) + "&topicId=" + urlEncode(id) + "&mode=" + urlEncode(mode) + (anonymousId.empty() ? "" : "&anonymousId=" + urlEncode(anonymousId));
        rows += "<a class=\"outline-row\" href=\"" + htmlEscape(href) + "\"><span class=\"outline-number\">" + std::to_string(number) + "</span><div><h3>" + htmlEscape(name) + "</h3><p>" + htmlEscape(description) + "</p></div><span>" + (state == "completed" ? "已完成" : state == "in_progress" ? "学习中" : "开始 →") + "</span></a>";
    }
    const std::string body = headerShell("learn") + "<main class=\"phase-page\"><a href=\"" + htmlEscape(back) + "\">← 返回课程</a><p class=\"uc-eyebrow\">第 " + std::to_string(selected) + " 阶段 · " + (mode == "lite" ? "快速规划" : "深度课程规划") + "</p><h1>" + htmlEscape(title) + "</h1><p class=\"phase-goal\">" + htmlEscape(stageGoal) + "</p>" + (why.empty() ? "" : "<p class=\"phase-why\">" + htmlEscape(why) + "</p>") + "<section><h2>本阶段知识点</h2><div data-paged-list data-page-size=\"8\">" + rows + "</div></section></main>";
    return document("阶段 - 钢一定制AI", body, "<script>window.GangyiNavigation?.setContext({courseId:" + jsString(courseId) + ",phaseIndex:" + std::to_string(selected) + "});</script>");
}

}  // namespace gangyi

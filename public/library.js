/* 资料中心只在用户发起任务时联网；页面恢复与引用回查读取保存的快照。 */
(() => {
  'use strict';
  if(document.body.dataset.myAiPage!=='library')return;
  const root=document.getElementById('my-ai-content'),notice=document.getElementById('my-ai-status'),params=new URLSearchParams(location.search);
  const esc=value=>String(value??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  const note=(text,error=false)=>{notice.textContent=text;notice.classList.toggle('error',error);};
  const date=value=>value?String(value).replace('T',' ').replace('Z',' UTC'):'未知';
  const providerNames={deepseek:'DeepSeek',bocha:'博查'};
  const provenance=item=>(item.searchProviders||[]).map(name=>providerNames[name]||name).join('、')||'手动添加或历史记录';
  const searchStatus=value=>({completed:'正常',missing_key:'未配置密钥',http_401:'密钥无效',http_402:'余额不足',http_403:'访问受限',http_429:'请求过于频繁',network_error:'连接失败',invalid_response:'结果格式异常',query_too_long:'搜索词过长',cancelled:'已取消',not_used:'尚未检索'}[value]||'未完成');
  const readLabels={summary_only:'仅搜索摘要',read:'已提取正文',partial:'部分文字可读',unreadable:'原文暂不可读'};
  const states={pending:'等待处理',running:'正在检索与核验',ready:'已完成',waiting_student:'需要补充背景',failed:'未完成',paused:'已暂停',cancelled:'已取消',superseded:'学习情况已变化'};
  async function request(path,body) {
    const response=await fetch(path,body?{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}:{cache:'no-store'});
    const value=await response.json();if(!response.ok)throw new Error(value.error||'操作未完成');return value;
  }
  const api=(body,query={})=>request('/api/my-ai/library'+(Object.keys(query).length?'?'+new URLSearchParams(query):''),body);
  const action=fn=>async event=>{const button=event.currentTarget;button.disabled=true;try{await fn(event);}catch(error){note(error.message,true);}finally{if(button.isConnected)button.disabled=false;}};
  const button=(text,key)=>'<button type="button" class="ma-button secondary" data-operation="'+key+'">'+text+'</button>';
  const taskKey='gy:library-task',draftKey='gy:library-goal';
  let currentTask=localStorage.getItem(taskKey),polling=false,finishedTask='';
  function courseId(){const control=document.getElementById('library-course');return control?control.value:params.get('courseId')||'';}
  async function research(body) {
    const task=await request('/api/my-ai/library/research',{...body,courseId:courseId(),requestId:crypto.randomUUID()});
    currentTask=task.id;localStorage.setItem(taskKey,currentTask);await pollTask();note('资料任务已启动，已完成的检索和正文会保存。');
  }
  async function pollTask() {
    if(!currentTask||polling||document.hidden)return;
    polling=true;try {
      const task=await request('/api/learning-agent?taskId='+encodeURIComponent(currentTask));
      const host=document.getElementById('library-task');
      if(host){
        host.innerHTML='<h2>资料任务</h2><p>'+esc(states[task.status]||task.status)+(task.activity?.title?' · '+esc(task.activity.title):'')+'</p>'+
          (task.message?'<p class="library-task-message">'+esc(task.message)+'</p>':'')+
          '<div class="ma-actions">'+(task.status==='running'||task.status==='pending'?button('暂停','pause'):task.status==='paused'?button('继续','resume'):
          ['failed','superseded'].includes(task.status)?button('重试','retry'):'')+'</div>';
        host.querySelectorAll('[data-operation]').forEach(b=>b.onclick=action(async()=>{await request('/api/learning-agent/control',{taskId:currentTask,command:b.dataset.operation});await pollTask();}));
      }
      if(['ready','waiting_student','failed','cancelled','superseded'].includes(task.status)){
        const signature=task.id+':'+task.status;
        if(host&&finishedTask!==signature){finishedTask=signature;if(!params.get('id'))await list();else if(!params.get('snapshotId'))await detail();}
      }
    }catch(error){note(error.message,true);}finally{polling=false;}
  }
  async function list() {
    const data=await api(null,{courseId:courseId(),search:document.getElementById('library-search')?.value||'',filter:document.getElementById('library-filter')?.value||'all'});
    const host=document.getElementById('library-list');if(!host)return;
    host.innerHTML=data.items.length?data.items.map(item=>'<article class="ma-card library-source"><div class="library-source-heading"><h3><a href="/my-ai/library?'+new URLSearchParams({id:item.id,courseId:courseId()})+'">'+esc(item.title||item.url)+'</a></h3><span class="library-read-state">'+esc(readLabels[item.readStatus]||item.readStatus)+'</span></div>'+
      '<p class="ma-note">'+esc(item.source)+' · '+esc(item.language||'语言未确认')+' · 检索：'+esc(provenance(item))+' · 发布：'+esc(date(item.publishedAt))+'</p>'+
      '<p>'+esc(item.choice?.reason||item.description||'等待AI阅读并评估适合度')+'</p>'+
      (item.choice?.conflicts?'<p class="ma-note">资料差异：'+esc(item.choice.conflicts)+'</p>':'')+
      '<p class="ma-note">核验：'+esc(date(item.lastCheckedAt))+(item.excluded?' · 已排除':'')+(item.choice?.manual?' · 采用你的手动选择':'')+'</p>'+
      '<div class="ma-actions"><button class="ma-button secondary" data-favorite="'+esc(item.id)+'">'+(item.favorite?'取消收藏':'收藏')+'</button>'+
      '<a class="ma-button secondary" href="/my-ai/library?'+new URLSearchParams({id:item.id,courseId:courseId()})+'">查看依据</a></div></article>').join(''):
      '<p class="ma-empty">还没有符合筛选条件的资料。告诉AI你的学习目标，或添加公开资料网址。</p>';
    host.querySelectorAll('[data-favorite]').forEach(b=>b.onclick=action(async()=>{const item=data.items.find(x=>x.id===b.dataset.favorite);await api({id:item.id,version:item.version,favorite:!item.favorite});await list();}));
    const usage=document.getElementById('library-usage');if(usage)usage.textContent='两家实际搜索请求 '+data.usage.searchRequests+' 次 · 正文获取 '+data.usage.fetchRequests+' 次 · 缓存命中 '+data.usage.cacheHits+' 次 · 部分读取 '+(data.usage.partialReads||0)+' 条 · 失败记录 '+data.usage.failures+' 条';
    if(usage&&data.usage.providers)for(const name of ['deepseek','bocha']){const part=data.usage.providers[name];if(part)usage.textContent+=' · '+providerNames[name]+'：请求 '+part.requests+' 次，缓存 '+part.cacheHits+' 次，失败 '+part.failures+' 次（'+searchStatus(part.lastStatus)+'）';}
  }
  async function detail() {
    let data=await api(null,{id:params.get('id'),...(params.get('snapshotId')?{snapshotId:params.get('snapshotId')}:{}),courseId:params.get('courseId')||''});
    const snapshot=data.snapshot;
    root.innerHTML='<a class="ma-link" href="/my-ai/library?'+new URLSearchParams({courseId:params.get('courseId')||''})+'">← 返回资料列表</a>'+
      '<article class="ma-card"><h2>'+esc(data.title)+'</h2><p>'+esc(data.description||'')+'</p><p class="ma-note">'+esc(data.source)+' · '+esc(readLabels[data.readStatus]||data.readStatus)+' · 检索：'+esc(provenance(data))+'</p>'+
      '<p class="ma-note">发布：'+esc(date(snapshot?.publishedAt??data.publishedAt))+' · 原文更新：'+esc(date(snapshot?.sourceUpdatedAt??data.sourceUpdatedAt))+' · 本次核验：'+esc(date(snapshot?.checkedAt??data.lastCheckedAt))+'</p>'+
      '<p class="ma-note">'+esc(data.coverage||'尚未读取原文，搜索摘要不作为全文依据')+'</p>'+
      (data.readReason?'<p class="ma-note">上次读取：'+esc(data.readReason)+'</p>':'')+
      '<div class="ma-actions"><a class="ma-button secondary" href="'+esc(data.url)+'" target="_blank" rel="noreferrer">打开原网站 ↗</a>'+
      button(data.favorite?'取消收藏':'收藏','favorite')+button(data.excluded?'允许使用':'排除此资料','exclude')+
      button(snapshot?'更新原文':'读取原文','read')+button('选为教学资料','select')+'</div></article><article class="ma-card" id="library-task" aria-live="polite"></article>'+
      '<article class="ma-card"><h2>保存的原文依据</h2>'+
      (snapshot?'<p class="ma-note">本次引用固定在此快照；新资料不会改写已有课堂依据。'+(snapshot.automatic===true?' · 自动字幕，可能有识别错误':snapshot.automatic===false?' · 人工字幕':snapshot.kind==='captions'?' · 字幕生成方式未提供':'')+'</p>'+
        snapshot.segments.map(segment=>'<section class="library-segment" id="'+esc(segment.id)+'"><p class="ma-note">'+esc(segment.page?'第 '+segment.page+' 页':segment.start!=null?'字幕 '+segment.start+'–'+segment.end+' 秒':'段落 '+segment.id)+'</p><p class="library-source-text">'+esc(segment.text)+'</p></section>').join(''):
        '<p class="ma-empty">暂无可核对的原文。可发起读取，读取失败会保留链接与缺失说明。</p>')+'</article>';
    root.querySelectorAll('[data-operation]').forEach(b=>b.onclick=action(async()=>{
      const operation=b.dataset.operation;
      if(operation==='read'){await research({goal:data.title,sourceId:data.id,version:data.version,refresh:!!snapshot,text:'读取或更新指定资料正文，核验日期和覆盖范围。'});return;}
      const update=operation==='favorite'?{favorite:!data.favorite}:operation==='exclude'?{excluded:!data.excluded}:{state:'selected',courseId:params.get('courseId')||''};
      data=await api({id:data.id,version:data.version,...update});await detail();
    }));
    note('原文快照仅用于引用和教学，资料阅读不形成掌握分数。');await pollTask();
    if(location.hash)document.getElementById(decodeURIComponent(location.hash.slice(1)))?.scrollIntoView({block:'center'});
  }
  async function initialize(){
    const fromChat=params.get('fromChat')||sessionStorage.getItem('my-ai-return-chat');if(fromChat){const link=document.getElementById('return-chat');link.href='/my-ai/chat?id='+encodeURIComponent(fromChat);link.hidden=false;}
    if(params.get('id')){await detail();return;}
    const overview=await request('/api/my-ai/overview');
    root.innerHTML='<article class="ma-card"><h2>为当前学习寻找合适资料</h2><p class="ma-note">AI会结合学习目标、基础和教学偏好，同时用 DeepSeek 和博查从不同角度搜索，合并去重后阅读和核验；一家暂不可用时会显示状态并保留另一家的结果。</p>'+
      '<form id="library-form"><div class="ma-field"><label for="library-goal">你现在想学什么，或正在遇到什么问题？</label><textarea id="library-goal" rows="3" required>'+esc(localStorage.getItem(draftKey)||'')+'</textarea></div>'+
      '<div class="ma-field"><label for="library-course">关联课程</label><select id="library-course"><option value="">个人学习资料</option>'+overview.courses.map(c=>'<option value="'+esc(c.id)+'">'+esc(c.title)+'</option>').join('')+'</select></div>'+
      '<button class="ma-button" type="submit">让AI搜索与核验</button></form></article>'+
      '<article class="ma-card" id="library-task" aria-live="polite"></article>'+
      '<article class="ma-card"><h2>资料列表</h2><div class="library-filters"><input id="library-search" aria-label="搜索已保存资料" placeholder="搜索已保存的资料与正文"><select id="library-filter" aria-label="资料筛选"><option value="all">全部</option><option value="favorites">已收藏</option><option value="read">正文可读</option><option value="excluded">已排除</option></select></div><p class="ma-note" id="library-usage"></p>'+
      '<form id="library-url-form" class="library-add-url"><input id="library-url" type="url" required placeholder="添加公开资料网址" aria-label="公开资料网址"><button class="ma-button secondary">添加网址</button></form></article><div class="ma-grid" id="library-list"></div>';
    document.getElementById('library-course').value=params.get('courseId')||'';
    const goal=document.getElementById('library-goal');goal.oninput=()=>localStorage.setItem(draftKey,goal.value);
    document.getElementById('library-form').onsubmit=async event=>{event.preventDefault();const button=event.currentTarget.querySelector('button');button.disabled=true;try{await research({goal:goal.value.trim()});}catch(error){note(error.message,true);}finally{button.disabled=false;}};
    document.getElementById('library-url-form').onsubmit=async event=>{event.preventDefault();try{const source=await api({url:document.getElementById('library-url').value,courseId:courseId()});location.href='/my-ai/library?'+new URLSearchParams({id:source.id,courseId:courseId()});}catch(error){note(error.message,true);}};
    document.getElementById('library-course').onchange=list;document.getElementById('library-filter').onchange=list;
    let timer;document.getElementById('library-search').oninput=()=>{clearTimeout(timer);timer=setTimeout(()=>list().catch(e=>note(e.message,true)),250);};
    await list();note('已保存资料可离线回查；点击搜索或更新才会发起资料任务。');await pollTask();
  }
  initialize().catch(error=>note(error.message,true));setInterval(pollTask,2500);
})();

/* 资料中心只在用户发起任务时联网；页面恢复与引用回查读取保存的快照。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 执行当前前端表达式：if(document.body.dataset.myAiPage!=='library')return;。
  if(document.body.dataset.myAiPage!=='library')return;
  // 【逐行批注】 声明前端变量 root，保存本行计算出的数据或对象。
  const root=document.getElementById('my-ai-content'),notice=document.getElementById('my-ai-status'),params=new URLSearchParams(location.search);
  // 【逐行批注】 声明前端变量 esc，保存本行计算出的数据或对象。
  const esc=value=>String(value??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  // 【逐行批注】 声明前端变量 note，保存本行计算出的数据或对象。
  const note=(text,error=false)=>{notice.textContent=text;notice.classList.toggle('error',error);};
  // 【逐行批注】 声明前端变量 date，保存本行计算出的数据或对象。
  const date=value=>value?String(value).replace('T',' ').replace('Z',' UTC'):'未知';
  // 【逐行批注】 声明前端变量 providerNames，保存本行计算出的数据或对象。
  const providerNames={deepseek:'DeepSeek',bocha:'博查'};
  // 【逐行批注】 声明前端变量 provenance，保存本行计算出的数据或对象。
  const provenance=item=>(item.searchProviders||[]).map(name=>providerNames[name]||name).join('、')||'手动添加或历史记录';
  // 【逐行批注】 声明前端变量 searchStatus，保存本行计算出的数据或对象。
  const searchStatus=value=>({completed:'正常',missing_key:'未配置密钥',http_401:'密钥无效',http_402:'余额不足',http_403:'访问受限',http_429:'请求过于频繁',network_error:'连接失败',invalid_response:'结果格式异常',query_too_long:'搜索词过长',cancelled:'已取消',not_used:'尚未检索'}[value]||'未完成');
  // 【逐行批注】 声明前端变量 readLabels，保存本行计算出的数据或对象。
  const readLabels={summary_only:'仅搜索摘要',read:'已提取正文',partial:'部分文字可读',unreadable:'原文暂不可读'};
  // 【逐行批注】 声明前端变量 states，保存本行计算出的数据或对象。
  const states={pending:'等待处理',running:'正在检索与核验',ready:'已完成',waiting_student:'需要补充背景',failed:'未完成',paused:'已暂停',cancelled:'已取消',superseded:'学习情况已变化'};
  // 【逐行批注】 定义函数 request，封装可复用的前端行为。
  async function request(path,body) {
    // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
    const response=await fetch(path,body?{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}:{cache:'no-store'});
    // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
    const value=await response.json();if(!response.ok)throw new Error(value.error||'操作未完成');return value;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 api，保存本行计算出的数据或对象。
  const api=(body,query={})=>request('/api/my-ai/library'+(Object.keys(query).length?'?'+new URLSearchParams(query):''),body);
  // 【逐行批注】 声明前端变量 byId，保存本行计算出的数据或对象。
  const byId=id=>typeof document.getElementById==='function'?document.getElementById(id):null;
  // 【逐行批注】 声明前端变量 setAttribute，保存本行计算出的数据或对象。
  const setAttribute=(node,name,value)=>{if(node&&typeof node.setAttribute==='function')node.setAttribute(name,value);};
  // 【逐行批注】 声明前端变量 removeAttribute，保存本行计算出的数据或对象。
  const removeAttribute=(node,name)=>{if(node&&typeof node.removeAttribute==='function')node.removeAttribute(name);};
  // 【逐行批注】 声明前端变量 traceFactory，保存本行计算出的数据或对象。
  const traceFactory=()=>typeof window!=='undefined'?window.GangyiTrace:null;
  // 点击瞬间给出进行时反馈：禁用按钮、换文案、容器 aria-busy，结束后恢复原文案。
  // 【逐行批注】 声明前端变量 action，保存本行计算出的数据或对象。
  const action=fn=>async event=>{
    // 【逐行批注】 声明前端变量 pressed，保存本行计算出的数据或对象。
    const pressed=event.currentTarget;
    // 【逐行批注】 执行当前前端表达式：if(!pressed||pressed.disabled)return;。
    if(!pressed||pressed.disabled)return;
    // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
    const label=pressed.textContent,container=pressed.closest?.('form,.ma-card,.ma-row')||null;
    // 【逐行批注】 计算右侧结果并更新 pressed.disabled，改变页面或请求状态。
    pressed.disabled=true;pressed.textContent='处理中…';setAttribute(container,'aria-busy','true');
    // 【逐行批注】 调用 fn 执行页面交互或数据处理：try{await fn(event);}。
    try{await fn(event);}
    // 【逐行批注】 调用 note 执行页面交互或数据处理：catch(error){note(error.message,true);}。
    catch(error){note(error.message,true);}
    // 【逐行批注】 执行当前前端表达式：finally{。
    finally{
      // 【逐行批注】 执行当前前端表达式：if(pressed.isConnected!==false){pressed.disabled=false;pressed.textContent=label;}。
      if(pressed.isConnected!==false){pressed.disabled=false;pressed.textContent=label;}
      // 【逐行批注】 调用 removeAttribute 执行页面交互或数据处理：removeAttribute(container,'aria-busy');。
      removeAttribute(container,'aria-busy');
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 button，保存本行计算出的数据或对象。
  const button=(text,key)=>'<button type="button" class="ma-button secondary" data-operation="'+key+'">'+text+'</button>';
  // 【逐行批注】 声明前端变量 taskKey，保存本行计算出的数据或对象。
  const taskKey='gy:library-task',draftKey='gy:library-goal';
  // 【逐行批注】 声明前端变量 currentTask，保存本行计算出的数据或对象。
  let currentTask=localStorage.getItem(taskKey),polling=false,finishedTask='',libraryTrace=null,traceMount=null,shellHost=null,lastTask=null,lastRequest=null;
  // 【逐行批注】 定义函数 courseId，封装可复用的前端行为。
  function courseId(){const control=byId('library-course');return control?control.value:params.get('courseId')||'';}
  // 任务卡只在重建时渲染一次结构；轮询只更新状态文本与共享轨迹，不再整块替换。
  // 【逐行批注】 定义函数 renderTaskShell，封装可复用的前端行为。
  function renderTaskShell(){
    // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
    const host=byId('library-task');if(!host)return null;
    // 【逐行批注】 执行当前前端表达式：if(shellHost!==host){。
    if(shellHost!==host){
      // 【逐行批注】 计算右侧结果并更新 shellHost，改变页面或请求状态。
      shellHost=host;
      // 【逐行批注】 计算右侧结果并更新 host.innerHTML，改变页面或请求状态。
      host.innerHTML='<h2>资料任务</h2>'+
        // 【逐行批注】 执行当前前端表达式：'<p class="library-task-status" id="library-task-status" role="status" aria-live="polite"…。
        '<p class="library-task-status" id="library-task-status" role="status" aria-live="polite">正在准备资料任务。</p>'+
        // 【逐行批注】 执行当前前端表达式：'<div id="library-agent-trace"></div>'+。
        '<div id="library-agent-trace"></div>'+
        // 【逐行批注】 执行当前前端表达式：'<p class="library-task-message" id="library-task-message" hidden></p>'+。
        '<p class="library-task-message" id="library-task-message" hidden></p>'+
        // 【逐行批注】 执行当前前端表达式：'<div class="ma-actions" id="library-task-fallback" hidden></div>';。
        '<div class="ma-actions" id="library-task-fallback" hidden></div>';
      // 【逐行批注】 计算右侧结果并更新 traceMount，改变页面或请求状态。
      traceMount=null;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return host;。
    return host;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 setStatusLine，封装可复用的前端行为。
  function setStatusLine(text){
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node=byId('library-task-status');if(!node)return;
    // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
    const value=String(text??'');
    // 【逐行批注】 执行当前前端表达式：if(node.textContent!==value)node.textContent=value;。
    if(node.textContent!==value)node.textContent=value;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 discardTrace，封装可复用的前端行为。
  function discardTrace(){libraryTrace?.destroy?.();libraryTrace=null;traceMount=null;}
  // 【逐行批注】 定义函数 ensureTrace，封装可复用的前端行为。
  function ensureTrace(){
    // 【逐行批注】 声明前端变量 factory，保存本行计算出的数据或对象。
    const factory=traceFactory(),mount=byId('library-agent-trace');
    // 【逐行批注】 执行当前前端表达式：if(!factory||!mount)return null;。
    if(!factory||!mount)return null;
    // 【逐行批注】 执行当前前端表达式：if(traceMount!==mount){。
    if(traceMount!==mount){
      // 【逐行批注】 调用 discardTrace 执行页面交互或数据处理：discardTrace();。
      discardTrace();
      // 【逐行批注】 计算右侧结果并更新 libraryTrace，改变页面或请求状态。
      libraryTrace=factory.create(mount,{variant:'panel',eyebrow:'AI 资料执行过程',collapseWhenDone:true,
        // 【逐行批注】 计算右侧结果并更新 onPause:()，改变页面或请求状态。
        onPause:()=>controlTask('pause'),
        // 【逐行批注】 计算右侧结果并更新 onResume:()，改变页面或请求状态。
        onResume:()=>controlTask('resume'),
        // 【逐行批注】 计算右侧结果并更新 onRetry:task，改变页面或请求状态。
        onRetry:task=>String(task?.id||'').startsWith('library-local-')?retryResearch():controlTask('retry'),
        // 【逐行批注】 计算右侧结果并更新 onCancel:()，改变页面或请求状态。
        onCancel:()=>controlTask('cancel'),
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 计算右侧结果并更新 traceMount，改变页面或请求状态。
      traceMount=mount;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return libraryTrace;。
    return libraryTrace;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 controlTask，封装可复用的前端行为。
  async function controlTask(command){
    // 【逐行批注】 执行当前前端表达式：if(!currentTask)return;。
    if(!currentTask)return;
    // 【逐行批注】 执行当前前端表达式：try{。
    try{
      // 【逐行批注】 调用 request 执行页面交互或数据处理：await request('/api/learning-agent/control',{taskId:currentTask,comma…。
      await request('/api/learning-agent/control',{taskId:currentTask,command});
      // 【逐行批注】 调用 setStatusLine 执行页面交互或数据处理：setStatusLine(({pause:'资料任务已暂停，已完成资料保留。',resume:'已恢复资料任务。',retry:'已请求…。
      setStatusLine(({pause:'资料任务已暂停，已完成资料保留。',resume:'已恢复资料任务。',retry:'已请求从上次失败步骤重试。',cancel:'已停止资料任务，已保存资料保留。'})[command]||'已更新资料任务状态。');
      // 【逐行批注】 调用 pollTask 执行页面交互或数据处理：await pollTask();。
      await pollTask();
    // 【逐行批注】 调用 note 执行页面交互或数据处理：}catch(error){note('操作未完成：'+error.message,true);}。
    }catch(error){note('操作未完成：'+error.message,true);}
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 retryResearch，保存本行计算出的数据或对象。
  const retryResearch=()=>lastRequest?research(lastRequest).catch(error=>note(error.message,true)):Promise.resolve();
  // 变化才写 DOM：轨迹组件自身按值渲染任务编号、阶段、计时、步骤与失败原因。
  // 【逐行批注】 定义函数 updateTaskView，封装可复用的前端行为。
  function updateTaskView(task){
    // 【逐行批注】 执行当前前端表达式：if(!task)return;。
    if(!task)return;
    // 【逐行批注】 计算右侧结果并更新 lastTask，改变页面或请求状态。
    lastTask=task;
    // 【逐行批注】 调用 renderTaskShell 执行页面交互或数据处理：renderTaskShell();。
    renderTaskShell();
    // 【逐行批注】 声明前端变量 failure，保存本行计算出的数据或对象。
    const failure=task.failure?.message||task.error||'';
    // 【逐行批注】 调用 setStatusLine 执行页面交互或数据处理：setStatusLine((states[task.status]||task.status||'处理中')+(task.activit…。
    setStatusLine((states[task.status]||task.status||'处理中')+(task.activity?.title?' · '+task.activity.title:'')+(failure&&task.status==='failed'?' · '+failure:''));
    // 【逐行批注】 声明前端变量 message，保存本行计算出的数据或对象。
    const message=byId('library-task-message'),messageText=task.message||'';
    // 【逐行批注】 计算右侧结果并更新 if(message){message.textContent，改变页面或请求状态。
    if(message){message.textContent=messageText;message.hidden=!messageText;}
    // 【逐行批注】 声明前端变量 trace，保存本行计算出的数据或对象。
    const trace=ensureTrace();
    // 【逐行批注】 声明前端变量 fallback，保存本行计算出的数据或对象。
    const fallback=byId('library-task-fallback');
    // 【逐行批注】 计算右侧结果并更新 if(trace){if(fallback)fallback.hidden，改变页面或请求状态。
    if(trace){if(fallback)fallback.hidden=true;trace.update(task);return;}
    // 共享轨迹组件不可用时保留兜底控制，避免任务无法暂停或重试。
    // 【逐行批注】 执行当前前端表达式：if(!fallback)return;。
    if(!fallback)return;
    // 【逐行批注】 计算右侧结果并更新 fallback.hidden，改变页面或请求状态。
    fallback.hidden=false;
    // 【逐行批注】 计算右侧结果并更新 fallback.innerHTML，改变页面或请求状态。
    fallback.innerHTML=['running','pending'].includes(task.status)?button('暂停','pause'):task.status==='paused'?button('继续','resume'):['failed','superseded'].includes(task.status)?button('重试','retry'):'';
    // 【逐行批注】 计算右侧结果并更新 fallback.querySelectorAll('[data-operation]'…，改变页面或请求状态。
    fallback.querySelectorAll('[data-operation]').forEach(b=>b.onclick=action(async()=>{await controlTask(b.dataset.operation);}));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 paintLastTask，封装可复用的前端行为。
  function paintLastTask(){if(lastTask)updateTaskView(lastTask);}
  // 【逐行批注】 定义函数 research，封装可复用的前端行为。
  async function research(body) {
    // 【逐行批注】 计算右侧结果并更新 lastRequest，改变页面或请求状态。
    lastRequest=body;
    // 【逐行批注】 声明前端变量 local，保存本行计算出的数据或对象。
    const local={id:'library-local-'+Date.now(),status:'pending',purpose:'搜索并核验学习资料',calls:0,createdAt:new Date().toISOString(),updatedAt:new Date().toISOString(),
      // 【逐行批注】 调用 Date 执行页面交互或数据处理：activity:{title:'正在提交资料任务',detail:'已提交检索与核验请求，等待任务编号。',state:'pending…。
      activity:{title:'正在提交资料任务',detail:'已提交检索与核验请求，等待任务编号。',state:'pending',at:new Date().toISOString()},events:[]};
    // 【逐行批注】 调用 updateTaskView、note 执行页面交互或数据处理：updateTaskView(local);note('正在提交资料任务…');。
    updateTaskView(local);note('正在提交资料任务…');
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
      const task=await request('/api/my-ai/library/research',{...body,courseId:courseId(),requestId:crypto.randomUUID()});
      // 【逐行批注】 计算右侧结果并更新 currentTask，改变页面或请求状态。
      currentTask=task.id;localStorage.setItem(taskKey,currentTask);finishedTask='';
      // 【逐行批注】 调用 updateTaskView、Date 执行页面交互或数据处理：updateTaskView({...local,...task,purpose:task.purpose||local.purpose,…。
      updateTaskView({...local,...task,purpose:task.purpose||local.purpose,createdAt:task.createdAt||local.createdAt,updatedAt:task.updatedAt||new Date().toISOString()});
      // 【逐行批注】 调用 pollTask、note 执行页面交互或数据处理：await pollTask();note('资料任务已启动，已完成的检索和正文会保存。');。
      await pollTask();note('资料任务已启动，已完成的检索和正文会保存。');
    // 【逐行批注】 执行当前前端表达式：} catch(error) {。
    } catch(error) {
      // 提交阶段失败：本地构造失败任务喂给轨迹，页面提示与轨迹状态保持一致。
      // 【逐行批注】 声明前端变量 message，保存本行计算出的数据或对象。
      const message=error.message||'资料任务未提交成功。';
      // 【逐行批注】 调用 updateTaskView、Date 执行页面交互或数据处理：updateTaskView({...local,status:'failed',updatedAt:new Date().toISOSt…。
      updateTaskView({...local,status:'failed',updatedAt:new Date().toISOString(),failure:{message},
        // 【逐行批注】 调用 Date 执行页面交互或数据处理：activity:{title:'提交失败',detail:'检索请求没有成功发出；已保存的资料与原文不受影响。',state:'fail…。
        activity:{title:'提交失败',detail:'检索请求没有成功发出；已保存的资料与原文不受影响。',state:'failed',at:new Date().toISOString()}});
      // 【逐行批注】 在错误分支抛出异常：throw error;。
      throw error;
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 pollTask，封装可复用的前端行为。
  async function pollTask() {
    // 【逐行批注】 执行当前前端表达式：if(!currentTask||polling||document.hidden)return;。
    if(!currentTask||polling||document.hidden)return;
    // 【逐行批注】 计算右侧结果并更新 polling，改变页面或请求状态。
    polling=true;try {
      // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
      const task=await request('/api/learning-agent?taskId='+encodeURIComponent(currentTask));
      // 【逐行批注】 调用 updateTaskView 执行页面交互或数据处理：updateTaskView(task);。
      updateTaskView(task);
      // 【逐行批注】 执行当前前端表达式：if(['ready','waiting_student','failed','cancelled','superseded'].includes(task.status)){。
      if(['ready','waiting_student','failed','cancelled','superseded'].includes(task.status)){
        // 【逐行批注】 声明前端变量 signature，保存本行计算出的数据或对象。
        const signature=task.id+':'+task.status;
        // 【逐行批注】 调用 list、detail 执行页面交互或数据处理：if(finishedTask!==signature){finishedTask=signature;if(!params.get('i…。
        if(finishedTask!==signature){finishedTask=signature;if(!params.get('id'))await list();else if(!params.get('snapshotId'))await detail();}
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 执行当前前端表达式：}catch(error){。
    }catch(error){
      // 任务可能仍在服务端运行，读取失败只提示状态暂不可用，不伪造失败结论。
      // 【逐行批注】 调用 note、setStatusLine 执行页面交互或数据处理：note(error.message,true);setStatusLine('暂时无法读取资料任务状态，已有资料与原文不受影响。');。
      note(error.message,true);setStatusLine('暂时无法读取资料任务状态，已有资料与原文不受影响。');
    // 【逐行批注】 计算右侧结果并更新 }finally{polling，改变页面或请求状态。
    }finally{polling=false;}
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 list，封装可复用的前端行为。
  async function list() {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    const data=await api(null,{courseId:courseId(),search:document.getElementById('library-search')?.value||'',filter:document.getElementById('library-filter')?.value||'all'});
    // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
    const host=document.getElementById('library-list');if(!host)return;
    // 【逐行批注】 计算右侧结果并更新 host.innerHTML，改变页面或请求状态。
    host.innerHTML=data.items.length?data.items.map(item=>'<article class="ma-card library-source"><div class="library-source-heading"><h3><a href="/my-ai/library?'+new URLSearchParams({id:item.id,courseId:courseId()})+'">'+esc(item.title||item.url)+'</a></h3><span class="library-read-state">'+esc(readLabels[item.readStatus]||item.readStatus)+'</span></div>'+
      // 【逐行批注】 调用 esc、provenance、date 执行页面交互或数据处理：'<p class="ma-note">'+esc(item.source)+' · '+esc(item.language||'语言未确…。
      '<p class="ma-note">'+esc(item.source)+' · '+esc(item.language||'语言未确认')+' · 检索：'+esc(provenance(item))+' · 发布：'+esc(date(item.publishedAt))+'</p>'+
      // 【逐行批注】 调用 esc 执行页面交互或数据处理：'<p>'+esc(item.choice?.reason||item.description||'等待AI阅读并评估适合度')+'</p…。
      '<p>'+esc(item.choice?.reason||item.description||'等待AI阅读并评估适合度')+'</p>'+
      // 【逐行批注】 调用 esc 执行页面交互或数据处理：(item.choice?.conflicts?'<p class="ma-note">资料差异：'+esc(item.choice.co…。
      (item.choice?.conflicts?'<p class="ma-note">资料差异：'+esc(item.choice.conflicts)+'</p>':'')+
      // 【逐行批注】 调用 esc、date 执行页面交互或数据处理：'<p class="ma-note">核验：'+esc(date(item.lastCheckedAt))+(item.excluded…。
      '<p class="ma-note">核验：'+esc(date(item.lastCheckedAt))+(item.excluded?' · 已排除':'')+(item.choice?.manual?' · 采用你的手动选择':'')+'</p>'+
      // 【逐行批注】 调用 esc 执行页面交互或数据处理：'<div class="ma-actions"><button class="ma-button secondary" data-fav…。
      '<div class="ma-actions"><button class="ma-button secondary" data-favorite="'+esc(item.id)+'">'+(item.favorite?'取消收藏':'收藏')+'</button>'+
      // 【逐行批注】 调用 URLSearchParams、courseId 执行页面交互或数据处理：'<a class="ma-button secondary" href="/my-ai/library?'+new URLSearchP…。
      '<a class="ma-button secondary" href="/my-ai/library?'+new URLSearchParams({id:item.id,courseId:courseId()})+'">查看依据</a></div></article>').join(''):
      // 【逐行批注】 执行当前前端表达式：'<p class="ma-empty">还没有符合筛选条件的资料。告诉AI你的学习目标，或添加公开资料网址。</p>';。
      '<p class="ma-empty">还没有符合筛选条件的资料。告诉AI你的学习目标，或添加公开资料网址。</p>';
    // 【逐行批注】 计算右侧结果并更新 host.querySelectorAll('[data-favorite]').for…，改变页面或请求状态。
    host.querySelectorAll('[data-favorite]').forEach(b=>b.onclick=action(async()=>{const item=data.items.find(x=>x.id===b.dataset.favorite);await api({id:item.id,version:item.version,favorite:!item.favorite});await list();}));
    // 【逐行批注】 声明前端变量 usage，保存本行计算出的数据或对象。
    const usage=document.getElementById('library-usage');if(usage)usage.textContent='两家实际搜索请求 '+data.usage.searchRequests+' 次 · 正文获取 '+data.usage.fetchRequests+' 次 · 缓存命中 '+data.usage.cacheHits+' 次 · 部分读取 '+(data.usage.partialReads||0)+' 条 · 失败记录 '+data.usage.failures+' 条';
    // 【逐行批注】 计算右侧结果并更新 if(usage&&data.usage.providers)for(const nam…，改变页面或请求状态。
    if(usage&&data.usage.providers)for(const name of ['deepseek','bocha']){const part=data.usage.providers[name];if(part)usage.textContent+=' · '+providerNames[name]+'：请求 '+part.requests+' 次，缓存 '+part.cacheHits+' 次，失败 '+part.failures+' 次（'+searchStatus(part.lastStatus)+'）';}
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 detail，封装可复用的前端行为。
  async function detail() {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    let data=await api(null,{id:params.get('id'),...(params.get('snapshotId')?{snapshotId:params.get('snapshotId')}:{}),courseId:params.get('courseId')||''});
    // 【逐行批注】 声明前端变量 snapshot，保存本行计算出的数据或对象。
    const snapshot=data.snapshot;
    const basis=data.curriculumSource;
    const curriculumInfo=basis?'<p class="ma-note">'+esc(basis.title)+' · '+esc(basis.coverage)+'</p>':'';
    const studyLink=snapshot?.segments?.length && !data.excluded && data.choice?.state!=='excluded'
      ? '<a class="ma-button" href="/ask?'+new URLSearchParams({sourceId:data.id,snapshotId:snapshot.id,courseId:params.get('courseId')||''})+'">围绕这份资料学</a>' : '';
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML='<a class="ma-link" href="/my-ai/library?'+new URLSearchParams({
      courseId:params.get('courseId')||'', chapterId:params.get('chapterId')||''
    })+'">← 返回资料列表</a>'+
      // 【逐行批注】 调用 esc、provenance 执行页面交互或数据处理：'<article class="ma-card"><h2>'+esc(data.title)+'</h2><p>'+esc(data.d…。
      '<article class="ma-card"><h2>'+esc(data.title)+'</h2><p>'+esc(data.description||'')+'</p><p class="ma-note">'+esc(data.source)+' · '+esc(readLabels[data.readStatus]||data.readStatus)+' · 检索：'+esc(provenance(data))+'</p>'+
      // 【逐行批注】 调用 esc、date 执行页面交互或数据处理：'<p class="ma-note">发布：'+esc(date(snapshot?.publishedAt??data.publish…。
      '<p class="ma-note">发布：'+esc(date(snapshot?.publishedAt??data.publishedAt))+' · 原文更新：'+esc(date(snapshot?.sourceUpdatedAt??data.sourceUpdatedAt))+' · 本次核验：'+esc(date(snapshot?.checkedAt??data.lastCheckedAt))+'</p>'+
      // 【逐行批注】 调用 esc 执行页面交互或数据处理：'<p class="ma-note">'+esc(data.coverage||'尚未读取原文，搜索摘要不作为全文依据')+'</p>'+。
      '<p class="ma-note">'+esc(data.coverage||'尚未读取原文，搜索摘要不作为全文依据')+'</p>'+
      curriculumInfo+
      // 【逐行批注】 调用 esc 执行页面交互或数据处理：(data.readReason?'<p class="ma-note">上次读取：'+esc(data.readReason)+'</p…。
      (data.readReason?'<p class="ma-note">上次读取：'+esc(data.readReason)+'</p>':'')+
      // 【逐行批注】 调用 esc 执行页面交互或数据处理：'<div class="ma-actions"><a class="ma-button secondary" href="'+esc(d…。
      '<div class="ma-actions"><a class="ma-button secondary" href="'+esc(data.url)+'" target="_blank" rel="noreferrer">打开原网站 ↗</a>'+
      // 【逐行批注】 调用 button 执行页面交互或数据处理：button(data.favorite?'取消收藏':'收藏','favorite')+button(data.excluded?'允许…。
      button(data.favorite?'取消收藏':'收藏','favorite')+button(data.excluded?'允许使用':'排除此资料','exclude')+
      // 【逐行批注】 调用 button 执行页面交互或数据处理：button(snapshot?'更新原文':'读取原文','read')+button('选为教学资料','select')+'</di…。
      button(snapshot?'更新原文':'读取原文','read')+button('选为教学资料','select')+studyLink+'</div></article>'+
      // 【逐行批注】 执行当前前端表达式：'<article class="ma-card" id="library-task"></article>'+。
      '<article class="ma-card" id="library-task"></article>'+
      // 【逐行批注】 执行当前前端表达式：'<article class="ma-card"><h2>保存的原文依据</h2>'+。
      '<article class="ma-card"><h2>保存的原文依据</h2>'+
      // 【逐行批注】 执行当前前端表达式：(snapshot?'<p class="ma-note">本次引用固定在此快照；新资料不会改写已有课堂依据。'+(snapshot.automatic===true?' · 自…。
      (snapshot?'<p class="ma-note">本次引用固定在此快照；新资料不会改写已有课堂依据。'+(snapshot.automatic===true?' · 自动字幕，可能有识别错误':snapshot.automatic===false?' · 人工字幕':snapshot.kind==='captions'?' · 字幕生成方式未提供':'')+'</p>'+
        // 【逐行批注】 计算右侧结果并更新 snapshot.segments.map(segment，改变页面或请求状态。
        snapshot.segments.map(segment=>'<section class="library-segment" id="'+esc(segment.id)+'"><p class="ma-note">'+esc(segment.page?'第 '+segment.page+' 页':segment.start!=null?'字幕 '+segment.start+'–'+segment.end+' 秒':'段落 '+segment.id)+'</p><p class="library-source-text">'+esc(segment.text)+'</p></section>').join(''):
        // 【逐行批注】 执行当前前端表达式：'<p class="ma-empty">暂无可核对的原文。可发起读取，读取失败会保留链接与缺失说明。</p>')+'</article>';。
        '<p class="ma-empty">暂无可核对的原文。可发起读取，读取失败会保留链接与缺失说明。</p>')+'</article>';
    // 【逐行批注】 计算右侧结果并更新 shellHost，改变页面或请求状态。
    shellHost=null;traceMount=null;discardTrace();
    // 【逐行批注】 计算右侧结果并更新 root.querySelectorAll('[data-operation]').fo…，改变页面或请求状态。
    root.querySelectorAll('[data-operation]').forEach(b=>b.onclick=action(async()=>{
      // 【逐行批注】 声明前端变量 operation，保存本行计算出的数据或对象。
      const operation=b.dataset.operation;
      // 【逐行批注】 调用 research 执行页面交互或数据处理：if(operation==='read'){await research({goal:data.title,sourceId:data.…。
      if(operation==='read'){await research({goal:data.title,sourceId:data.id,version:data.version,refresh:!!snapshot,text:'读取或更新指定资料正文，核验日期和覆盖范围。'});return;}
      // 【逐行批注】 声明前端变量 update，保存本行计算出的数据或对象。
      const update=operation==='favorite'?{favorite:!data.favorite}:operation==='exclude'?{excluded:!data.excluded}:{state:'selected',courseId:params.get('courseId')||''};
      // 【逐行批注】 计算右侧结果并更新 data，改变页面或请求状态。
      data=await api({id:data.id,version:data.version,...update});await detail();
    // 【逐行批注】 执行当前前端表达式：}));。
    }));
    // 【逐行批注】 调用 note 执行页面交互或数据处理：note('原文快照仅用于引用和教学，资料阅读不形成掌握分数。');。
    note('原文快照仅用于引用和教学，资料阅读不形成掌握分数。');
    // 【逐行批注】 调用 paintLastTask 执行页面交互或数据处理：paintLastTask();。
    paintLastTask();
    // 【逐行批注】 调用 pollTask 执行页面交互或数据处理：await pollTask();。
    await pollTask();
    // 【逐行批注】 调用 decodeURIComponent 执行页面交互或数据处理：if(location.hash)document.getElementById(decodeURIComponent(location.…。
    if(location.hash)document.getElementById(decodeURIComponent(location.hash.slice(1)))?.scrollIntoView({block:'center'});
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 initialize，封装可复用的前端行为。
  let curriculumRequest = 0;
  async function showCurriculum(previewId = '') {
    const host=byId('library-curriculum');
    if(!host)return;
    const selectedCourse=courseId();
    const requestId=++curriculumRequest;
    const path='/api/my-ai/curriculum';
    let data;
    try{data=await request(path+'?'+new URLSearchParams({courseId:selectedCourse}));}
    catch(error){if(requestId===curriculumRequest)host.textContent=error.message;return;}
    if(requestId!==curriculumRequest || courseId()!==selectedCourse || !data.chapter || !Array.isArray(data.chapters))return;
    const choices=data.chapters, selection=data.selection;
    const active = choices.find(choice=>choice.chapter.id===previewId) || data;
    host.innerHTML='<h2>课程标准与教材</h2><div class="ma-field"><label for="library-curriculum-edition">教材版本</label>'+
      '<select id="library-curriculum-edition">'+choices.map(({chapter})=>
        '<option value="'+esc(chapter.id)+'">'+esc(chapter.edition)+'</option>').join('')+
      '</select></div><div data-curriculum-details></div>';
    const edition=byId('library-curriculum-edition');
    const details=host.querySelector('[data-curriculum-details]');
    edition.value=active.chapter.id;
    const render=({chapter,sources:entries}, draftGrade)=>{
      const labels={verified:'已核对',unverified:'待核对原文',missing:'待添加',excluded:'已排除'};
      const verified=entries.every(source=>source.status==='verified');
      const sources=entries.map((source,index)=>{
        const loc=source.page?'查看第 '+source.page+' 页':source.status==='verified'?'查看目录段落':'打开资料记录';
        const action=source.href?'<a class="ma-link" href="'+esc(source.href)+'">'+esc(loc)+'</a>':
          '<button type="button" class="ma-button secondary" data-add-curriculum="'+index+'">添加来源</button>';
        return '<li><div><strong>'+esc(source.title)+'</strong><p class="ma-note">'+esc(source.coverage)+'</p></div>'+
          '<div><span class="library-read-state">'+esc(labels[source.status])+'</span>'+action+'</div></li>';
      }).join('');
      const hasSelection=Boolean(selection.chapterId);
      const gradeLabel=selection.grade?' · '+esc(selection.grade):'';
      const selected=hasSelection?'<p class="library-curriculum-selection">'+
        (selection.status==='verified'?'本课已选用'+gradeLabel:'已选依据暂不可用，请核对资料状态')+
        ' · '+esc(selection.chapter?.edition||'')+'</p><ul class="library-curriculum-pinned">'+(selection.sources||[]).filter(source=>source.href).map(source=>
          '<li><a class="ma-link" href="'+esc(source.href)+'">'+esc(source.page?'本课固定的标准第 '+source.page+' 页':source.status==='verified'?'本课固定的出版社目录':'查看当前资料状态')+'</a></li>').join('')+'</ul>':'';
      const controls=selectedCourse?'<div class="ma-actions"><label>年级 <select id="library-curriculum-grade"><option value="">暂不指定</option>'+
        ['高一','高二','高三'].map(grade=>'<option'+(draftGrade===grade?' selected':'')+'>'+grade+'</option>').join('')+'</select></label>'+
        '<button type="button" class="ma-button" data-save-curriculum '+(verified?'':'disabled')+'>'+(hasSelection?'更新本课依据':'用作本课依据')+'</button>'+
        (hasSelection?'<button type="button" class="ma-button secondary" data-clear-curriculum>取消本课选用</button>':'')+
        '</div>':'<p class="ma-note">选择关联课程后，可以将已核对的版本和章节用于该课程。</p>';
      details.innerHTML='<p><strong>'+esc(chapter.stage+' · '+chapter.subject)+'</strong></p>'+
        '<p>'+esc(chapter.title)+'</p><p class="ma-note">'+esc(chapter.editionBasis)+'</p><ul class="library-curriculum-sources">'+sources+'</ul>'+selected+controls;
      details.querySelectorAll('[data-add-curriculum]').forEach(button=>button.onclick=action(async()=>{
        const source=entries[Number(button.dataset.addCurriculum)];
        const saved=await api({url:source.url,title:source.title,courseId:selectedCourse});
        location.href='/my-ai/library?'+new URLSearchParams({id:saved.id,courseId:selectedCourse,chapterId:chapter.id});
      }));
      const change=async(clear)=>{
        await request(path,{courseId:selectedCourse,version:selection.version,chapterId:clear?'':chapter.id,
          grade:clear?'':byId('library-curriculum-grade').value});
        if (requestId===curriculumRequest) await showCurriculum(chapter.id);
      };
      const save=details.querySelector('[data-save-curriculum]');
      const clear=details.querySelector('[data-clear-curriculum]');
      if(save)save.onclick=action(()=>change(false));
      if(clear)clear.onclick=action(()=>change(true));
    };
    edition.onchange=()=>render(choices.find(choice=>choice.chapter.id===edition.value), byId('library-curriculum-grade')?.value);
    render(active, selection.grade);
  }

  async function initialize(){
    // 【逐行批注】 声明前端变量 fromChat，保存本行计算出的数据或对象。
    const fromChat=params.get('fromChat')||sessionStorage.getItem('my-ai-return-chat');if(fromChat){const link=document.getElementById('return-chat');link.href='/my-ai/chat?id='+encodeURIComponent(fromChat);link.hidden=false;}
    // 【逐行批注】 调用 detail 执行页面交互或数据处理：if(params.get('id')){await detail();return;}。
    if(params.get('id')){await detail();return;}
    // 【逐行批注】 声明前端变量 overview，保存本行计算出的数据或对象。
    const overview=await request('/api/my-ai/overview');
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML='<article class="ma-card"><h2>为当前学习寻找合适资料</h2><p class="ma-note">AI会结合学习目标、基础和教学偏好，同时用 DeepSeek 和博查从不同角度搜索，合并去重后阅读和核验；一家暂不可用时会显示状态并保留另一家的结果。</p>'+
      // 【逐行批注】 调用 esc 执行页面交互或数据处理：'<form id="library-form"><div class="ma-field"><label for="library-go…。
      '<form id="library-form"><div class="ma-field"><label for="library-goal">你现在想学什么，或正在遇到什么问题？</label><textarea id="library-goal" rows="3" required>'+esc(localStorage.getItem(draftKey)||'')+'</textarea></div>'+
      // 【逐行批注】 调用 esc 执行页面交互或数据处理：'<div class="ma-field"><label for="library-course">关联课程</label><selec…。
      '<div class="ma-field"><label for="library-course">关联课程</label><select id="library-course"><option value="">个人学习资料</option>'+overview.courses.map(c=>'<option value="'+esc(c.id)+'">'+esc(c.title)+'</option>').join('')+'</select></div>'+
      // 【逐行批注】 执行当前前端表达式：'<button class="ma-button" type="submit">让AI搜索与核验</button></form></article>'+。
      '<button class="ma-button" type="submit">让AI搜索与核验</button></form></article>'+
      // 【逐行批注】 执行当前前端表达式：'<article class="ma-card" id="library-task"></article>'+。
      '<article class="ma-card" id="library-task"></article>'+
      '<article class="ma-card" id="library-curriculum"><h2>课程标准与教材</h2><p class="ma-note">正在读取本机教材依据…</p></article>'+
      // 【逐行批注】 执行当前前端表达式：'<article class="ma-card"><h2>资料列表</h2><div class="library-filters"><input id="library-se…。
      '<article class="ma-card"><h2>资料列表</h2><div class="library-filters"><input id="library-search" aria-label="搜索已保存资料" placeholder="搜索已保存的资料与正文"><select id="library-filter" aria-label="资料筛选"><option value="all">全部</option><option value="favorites">已收藏</option><option value="read">正文可读</option><option value="excluded">已排除</option></select></div><p class="ma-note" id="library-usage"></p>'+
      // 【逐行批注】 执行当前前端表达式：'<form id="library-url-form" class="library-add-url"><input id="library-url" type="url" r…。
      '<form id="library-url-form" class="library-add-url"><input id="library-url" type="url" required placeholder="添加公开资料网址" aria-label="公开资料网址"><button class="ma-button secondary">添加网址</button></form></article><div class="ma-grid" id="library-list"></div>';
    // 【逐行批注】 计算右侧结果并更新 shellHost，改变页面或请求状态。
    shellHost=null;traceMount=null;discardTrace();
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('library-course').va…，改变页面或请求状态。
    document.getElementById('library-course').value=params.get('courseId')||'';
    // 【逐行批注】 声明前端变量 goal，保存本行计算出的数据或对象。
    const goal=document.getElementById('library-goal');goal.oninput=()=>localStorage.setItem(draftKey,goal.value);
    // 【逐行批注】 声明前端变量 form，保存本行计算出的数据或对象。
    const form=document.getElementById('library-form');
    // 【逐行批注】 按循环规则遍历或重复处理：form.onsubmit=async event=>{。
    form.onsubmit=async event=>{
      // 【逐行批注】 执行当前前端表达式：event.preventDefault();。
      event.preventDefault();
      // 【逐行批注】 声明前端变量 pressed，保存本行计算出的数据或对象。
      const pressed=form.querySelector?.('button[type=submit],button')||null,label=pressed?.textContent;
      // 【逐行批注】 计算右侧结果并更新 if(pressed){pressed.disabled，改变页面或请求状态。
      if(pressed){pressed.disabled=true;pressed.textContent='正在提交…';}
      // 【逐行批注】 调用 setAttribute 执行页面交互或数据处理：setAttribute(form,'aria-busy','true');。
      setAttribute(form,'aria-busy','true');
      // 【逐行批注】 调用 research 执行页面交互或数据处理：try{await research({goal:goal.value.trim()});}。
      try{await research({goal:goal.value.trim()});}
      // 【逐行批注】 调用 note 执行页面交互或数据处理：catch(error){note(error.message,true);}。
      catch(error){note(error.message,true);}
      // 【逐行批注】 调用 removeAttribute 执行页面交互或数据处理：finally{if(pressed&&pressed.isConnected!==false){pressed.disabled=fal…。
      finally{if(pressed&&pressed.isConnected!==false){pressed.disabled=false;pressed.textContent=label;}removeAttribute(form,'aria-busy');}
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 声明前端变量 urlForm，保存本行计算出的数据或对象。
    const urlForm=document.getElementById('library-url-form');
    // 【逐行批注】 计算右侧结果并更新 urlForm.onsubmit，改变页面或请求状态。
    urlForm.onsubmit=async event=>{
      // 【逐行批注】 执行当前前端表达式：event.preventDefault();。
      event.preventDefault();
      // 【逐行批注】 声明前端变量 pressed，保存本行计算出的数据或对象。
      const pressed=urlForm.querySelector?.('button')||null,label=pressed?.textContent;
      // 【逐行批注】 计算右侧结果并更新 if(pressed){pressed.disabled，改变页面或请求状态。
      if(pressed){pressed.disabled=true;pressed.textContent='正在提交…';}
      // 【逐行批注】 调用 setAttribute 执行页面交互或数据处理：setAttribute(urlForm,'aria-busy','true');。
      setAttribute(urlForm,'aria-busy','true');
      // 【逐行批注】 计算右侧结果并更新 try{const source，改变页面或请求状态。
      try{const source=await api({url:document.getElementById('library-url').value,courseId:courseId()});location.href='/my-ai/library?'+new URLSearchParams({id:source.id,courseId:courseId()});}
      // 【逐行批注】 调用 note 执行页面交互或数据处理：catch(error){note(error.message,true);}。
      catch(error){note(error.message,true);}
      // 【逐行批注】 调用 removeAttribute 执行页面交互或数据处理：finally{if(pressed&&pressed.isConnected!==false){pressed.disabled=fal…。
      finally{if(pressed&&pressed.isConnected!==false){pressed.disabled=false;pressed.textContent=label;}removeAttribute(urlForm,'aria-busy');}
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('library-course').on…，改变页面或请求状态。
    document.getElementById('library-course').onchange=()=>Promise.all([list(),showCurriculum()]);
    document.getElementById('library-filter').onchange=list;
    void showCurriculum(params.get('chapterId')||'');
    // 【逐行批注】 声明前端变量 timer，保存本行计算出的数据或对象。
    let timer;document.getElementById('library-search').oninput=()=>{clearTimeout(timer);timer=setTimeout(()=>list().catch(e=>note(e.message,true)),250);};
    // 【逐行批注】 调用 paintLastTask 执行页面交互或数据处理：paintLastTask();。
    paintLastTask();
    // 【逐行批注】 调用 list、note、pollTask 执行页面交互或数据处理：await list();note('已保存资料可离线回查；点击搜索或更新才会发起资料任务。');await pollTask();。
    await list();note('已保存资料可离线回查；点击搜索或更新才会发起资料任务。');await pollTask();
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 计算右侧结果并更新 initialize().catch(error，改变页面或请求状态。
  initialize().catch(error=>note(error.message,true));setInterval(pollTask,2500);
// 【逐行批注】 执行当前前端表达式：})();。
})();

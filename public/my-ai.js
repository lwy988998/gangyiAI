/* 我的AI：每个功能使用独立路由，读取页面只展示已保存记录。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 声明前端变量 page，保存本行计算出的数据或对象。
  const page=document.body.dataset.myAiPage,root=document.getElementById('my-ai-content'),notice=document.getElementById('my-ai-status');
  // 【逐行批注】 执行当前前端表达式：if(page==='library')return;。
  if(page==='library')return;
  // 【逐行批注】 声明前端变量 escape，保存本行计算出的数据或对象。
  const escape=value=>String(value??'').replace(/[&<>"']/g,ch=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[ch]));
  // 【逐行批注】 声明前端变量 status，保存本行计算出的数据或对象。
  const status=(text,error=false)=>{notice.textContent=text;notice.classList.toggle('error',error);};
  // 【逐行批注】 定义函数 api，封装可复用的前端行为。
  async function api(section,body,query={}) {
    // 【逐行批注】 声明前端变量 url，保存本行计算出的数据或对象。
    const url='/api/my-ai/'+section+(Object.keys(query).length?'?'+new URLSearchParams(query):'');
    // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
    const response=await fetch(url,body?{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}:{cache:'no-store'});
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    const data=await response.json();if(!response.ok)throw new Error(data.error||'操作未完成，请稍后重试。');return data;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 existing，封装可复用的前端行为。
  async function existing(url,body,method='POST') {const response=await fetch(url,body?{method,headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}:{cache:'no-store'});const data=await response.json();if(!response.ok)throw new Error(data.error||'操作未完成');return data;}
  // 点击瞬间给出进行时反馈：禁用按钮、换文案、容器 aria-busy，结束后恢复原文案。
  // 【逐行批注】 声明前端变量 action，保存本行计算出的数据或对象。
  const action=handler=>async event=>{
    // 【逐行批注】 声明前端变量 button，保存本行计算出的数据或对象。
    const button=event.currentTarget;
    // 【逐行批注】 执行当前前端表达式：if(!button||button.disabled)return;。
    if(!button||button.disabled)return;
    // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
    const label=button.textContent,container=button.closest?.('form,.ma-card,.ma-row,.ma-chat-composer')||null;
    // 【逐行批注】 计算右侧结果并更新 button.disabled，改变页面或请求状态。
    button.disabled=true;button.textContent='处理中…';
    // 【逐行批注】 执行当前前端表达式：if(container&&typeof container.setAttribute==='function')container.setAttribute('aria-bus…。
    if(container&&typeof container.setAttribute==='function')container.setAttribute('aria-busy','true');
    // 【逐行批注】 调用 handler 执行页面交互或数据处理：try{await handler(event);}。
    try{await handler(event);}
    // 【逐行批注】 调用 status 执行页面交互或数据处理：catch(error){status(error.message,true);}。
    catch(error){status(error.message,true);}
    // 【逐行批注】 执行当前前端表达式：finally{。
    finally{
      // 【逐行批注】 执行当前前端表达式：if(button.isConnected!==false){button.disabled=false;button.textContent=label;}。
      if(button.isConnected!==false){button.disabled=false;button.textContent=label;}
      // 【逐行批注】 执行当前前端表达式：if(container&&typeof container.removeAttribute==='function')container.removeAttribute('ar…。
      if(container&&typeof container.removeAttribute==='function')container.removeAttribute('aria-busy');
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 empty，保存本行计算出的数据或对象。
  const empty=text=>'<p class="ma-empty">'+escape(text)+'</p>';
  // 【逐行批注】 声明前端变量 button，保存本行计算出的数据或对象。
  const button=(label,id,kind='')=>'<button type="button" class="ma-button '+kind+'" id="'+id+'">'+label+'</button>';
  // 【逐行批注】 声明前端变量 card，保存本行计算出的数据或对象。
  const card=(title,content)=>'<article class="ma-card"><h2>'+escape(title)+'</h2>'+content+'</article>';
  // 【逐行批注】 声明前端变量 metric，保存本行计算出的数据或对象。
  const metric=(value,label)=>'<div class="ma-metric"><strong>'+escape(value)+'</strong><span>'+escape(label)+'</span></div>';
  // 【逐行批注】 声明前端变量 field，保存本行计算出的数据或对象。
  const field=(id,label,value='',type='text')=>'<div class="ma-field"><label for="'+id+'">'+label+'</label><'+(type==='textarea'?'textarea':'input type="'+type+'"')+' id="'+id+'"'+(type==='textarea'?'>'+escape(value)+'</textarea>':' value="'+escape(value)+'">')+'</div>';
  // 【逐行批注】 声明前端变量 score，保存本行计算出的数据或对象。
  const score=value=>value===null||value===undefined?'数据不足':value+' 分';
  // 【逐行批注】 声明前端变量 time，保存本行计算出的数据或对象。
  const time=value=>value?new Date(value).toLocaleString('zh-CN',{hour12:false}):'暂无记录';
  // 【逐行批注】 声明前端变量 taskLabels，保存本行计算出的数据或对象。
  const taskLabels={pending:'等待处理',running:'正在处理',ready:'已完成',waiting_student:'等待你的回应',failed:'处理失败',paused:'已暂停',cancelled:'已取消',superseded:'已被新的请求替代',requesting:'请求中',completed:'已完成',receiving:'正在接收',interrupted:'已中断'};
  // 【逐行批注】 声明前端变量 params，保存本行计算出的数据或对象。
  const params=new URLSearchParams(location.search),chatReturn=params.get('fromChat')||sessionStorage.getItem('my-ai-return-chat');
  // 【逐行批注】 调用 encodeURIComponent 执行页面交互或数据处理：if(chatReturn&&page!=='chat'){const link=document.getElementById('ret…。
  if(chatReturn&&page!=='chat'){const link=document.getElementById('return-chat');link.href='/my-ai/chat?id='+encodeURIComponent(chatReturn);link.hidden=false;}
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('click',event=>{const a=event.target.closest('a[href^="/my…。
  document.addEventListener('click',event=>{const a=event.target.closest('a[href^="/my-ai/"]');if(a&&page==='chat'&&params.get('id'))sessionStorage.setItem('my-ai-return-chat',params.get('id'));});

  // 【逐行批注】 定义函数 overview，封装可复用的前端行为。
  async function overview() {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    const data=await api('overview'),p=data.profile;
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML=card('', '<div class="ma-profile">'+(p.avatar?'<img class="ma-avatar" src="'+escape(p.avatar)+'" alt="我的头像">':'<span class="ma-avatar" aria-hidden="true">'+escape((p.nickname||'同学').slice(0,1))+'</span>')+'<div><h2>'+escape(p.nickname||'同学')+'</h2><p class="ma-note">'+escape(p.goal||'为自己设定一个学习目标')+'</p><a class="ma-link" href="/my-ai/profile">编辑个人资料 →</a></div></div>')+
      // 【逐行批注】 调用 metric 执行页面交互或数据处理：'<div class="ma-metrics">'+metric(data.courses.length,'在学课程')+metric(…。
      '<div class="ma-metrics">'+metric(data.courses.length,'在学课程')+metric(data.completed,'已完成课时')+metric(data.time.length?(data.studySeconds<60?'不足1分钟':Math.round(data.studySeconds/60)+' 分钟'):'尚未记录','已记录学习时间')+metric(data.topics.filter(t=>t.score!==null).length,'已评估知识点')+'</div>'+
      // 【逐行批注】 调用 card、escape、empty 执行页面交互或数据处理：card('下一步建议',data.suggestions.length?'<p>'+escape(data.suggestions[0]…。
      card('下一步建议',data.suggestions.length?'<p>'+escape(data.suggestions[0])+'</p><a class="ma-link" href="/my-ai/reports">查看完整学习报告 →</a>':empty('完成学习后，AI会依据真实记录更新建议。'))+
      // 【逐行批注】 执行当前前端表达式：'<div class="ma-grid">'+[['chat','AI管家','告诉AI你想调整什么，继续历史聊天。'],['reports','学习报告','查看掌握情况、薄…。
      '<div class="ma-grid">'+[['chat','AI管家','告诉AI你想调整什么，继续历史聊天。'],['reports','学习报告','查看掌握情况、薄弱点和判断依据。'],['plan','学习计划','设置每周时间，查看统一学习安排。'],['preferences','教学偏好','让讲解方式和节奏更适合自己。'],['memories','AI记忆','查看AI记住了什么，随时修改。'],['tasks','AI任务','查看执行情况，暂停或继续任务。']].map(([path,title,desc])=>'<a class="ma-card" href="/my-ai/'+path+'"><h3>'+title+'</h3><p class="ma-note">'+desc+'</p><span class="ma-link">打开 →</span></a>').join('')+'</div>';
    // 【逐行批注】 调用 status 执行页面交互或数据处理：status('本机档案 · 依据已保存的真实学习记录');。
    status('本机档案 · 依据已保存的真实学习记录');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 profile，封装可复用的前端行为。
  async function profile() {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    let data=await api('profile');
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML=card('个人资料',field('nickname','昵称',data.nickname)+field('goal','学习目标',data.goal,'textarea')+field('avatar','头像（PNG或JPEG，最大200 KB）','','file')+'<p class="ma-note">个人资料保存于本机。头像可选，不影响学习。</p>'+button('保存资料','save-profile'));
    // 【逐行批注】 声明前端变量 file，保存本行计算出的数据或对象。
    const file=document.getElementById('avatar');file.accept='image/png,image/jpeg';
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('save-profile').oncl…，改变页面或请求状态。
    document.getElementById('save-profile').onclick=action(async()=>{
      // 【逐行批注】 声明前端变量 avatar，保存本行计算出的数据或对象。
      let avatar=data.avatar||'';if(file.files[0]){const item=file.files[0];if(item.size>200000||!['image/png','image/jpeg'].includes(item.type))throw new Error('请选择200 KB以内的PNG或JPEG图片。');avatar=await new Promise((resolve,reject)=>{const reader=new FileReader();reader.onload=()=>resolve(reader.result);reader.onerror=reject;reader.readAsDataURL(item);});}
      // 【逐行批注】 计算右侧结果并更新 data，改变页面或请求状态。
      data=await api('profile',{version:data.version,nickname:document.getElementById('nickname').value,goal:document.getElementById('goal').value,avatar});status('个人资料已保存。');
    // 【逐行批注】 调用 status 执行页面交互或数据处理：});status('设置你的昵称和学习目标');。
    });status('设置你的昵称和学习目标');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 preferences，封装可复用的前端行为。
  async function preferences() {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    let data=await api('preferences');
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML=card('教学方式',field('style','讲解方式',data.style)+field('tone','交流语气',data.tone)+field('pace','教学节奏',data.pace)+field('detail','详细程度',data.detail)+field('instructions','其他教学要求',data.instructions,'textarea')+'<label><input id="auto-memory" type="checkbox" '+(data.autoMemory?'checked':'')+'> 允许AI从真实互动中形成偏好记忆</label><p class="ma-note">手动设置和你明确表达的要求优先于推测。具体记忆可在独立页面查看和修改。</p><div class="ma-actions">'+button('保存偏好','save-preferences')+'<a class="ma-button secondary" href="/my-ai/memories">管理AI记忆</a></div>');
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('save-preferences').…，改变页面或请求状态。
    document.getElementById('save-preferences').onclick=action(async()=>{const body={version:data.version,autoMemory:document.getElementById('auto-memory').checked};for(const id of ['style','tone','pace','detail','instructions'])body[id]=document.getElementById(id).value;data=await api('preferences',body);status('教学偏好已保存，将用于后续教学。');});const changes=await api('changes');const latest=changes.items.find(c=>c.state==='applied');if(latest){const undo=document.createElement('button');undo.className='ma-button secondary';undo.textContent='撤回最近的管家偏好修改';undo.onclick=action(async()=>{await api('undo',{id:latest.id});await preferences();status('已撤回管家的偏好修改。');});root.append(undo);}status('可以填写自己的要求，也可以在AI管家中通过对话调整。');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 memories，封装可复用的前端行为。
  async function memories() {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    const data=await api('memories');
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML=card('添加记忆',field('new-memory','希望AI记住什么？','','textarea')+button('添加记忆','add-memory'))+card('已保存的记忆',data.items.length?data.items.map((m,i)=>'<div class="ma-row">'+field('memory-'+i,'记忆内容',m.content,'textarea')+'<p class="ma-note">来源：'+escape(({manual:'手动添加',explicit:'明确表达',inferred:'AI根据互动推测'}[m.source]||m.source))+' · '+escape(time(m.updatedAt))+'</p>'+(m.sourceChatId?'<a class="ma-link" href="/my-ai/chat?id='+encodeURIComponent(m.sourceChatId)+'">查看来源聊天 →</a>':m.sourceLessonId?'<a class="ma-link" href="/learn?lessonId='+encodeURIComponent(m.sourceLessonId)+'">查看来源课时 →</a>':'')+'<div class="ma-actions"><button class="ma-button secondary" data-save-memory="'+i+'">保存修改</button><button class="ma-button danger" data-delete-memory="'+i+'">删除</button></div></div>').join(''):empty('还没有长期记忆。'));
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('add-memory').onclick，改变页面或请求状态。
    document.getElementById('add-memory').onclick=action(async()=>{await api('memories',{content:document.getElementById('new-memory').value,source:'manual'});await memories();status('记忆已添加。');});
    // 【逐行批注】 计算右侧结果并更新 root.querySelectorAll('[data-save-memory]').…，改变页面或请求状态。
    root.querySelectorAll('[data-save-memory]').forEach(b=>b.onclick=action(async()=>{const i=Number(b.dataset.saveMemory),m=data.items[i];await api('memories',{id:m.id,version:m.version,content:document.getElementById('memory-'+i).value,source:'manual'});await memories();status('记忆已修改。');}));
    // 【逐行批注】 计算右侧结果并更新 root.querySelectorAll('[data-delete-memory]'…，改变页面或请求状态。
    root.querySelectorAll('[data-delete-memory]').forEach(b=>b.onclick=action(async()=>{const m=data.items[Number(b.dataset.deleteMemory)];await api('memories',{id:m.id,version:m.version,deleted:true});await memories();status('记忆已删除，旧来源不会再次自动建立它。');}));status('你可以决定AI记住什么。');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 chart，封装可复用的前端行为。
  function chart(host,series,type='line') {if(!host||!window.echarts)return;const view=window.echarts.init(host,null,{renderer:'svg'});view.setOption({backgroundColor:'transparent',textStyle:{color:'#a6b8c9'},tooltip:{trigger:'axis'},legend:{textStyle:{color:'#a6b8c9'}},grid:{left:45,right:20,bottom:35,top:40},xAxis:{type:'category',data:series.dates,axisLine:{lineStyle:{color:'#334858'}}},yAxis:{type:'value',splitLine:{lineStyle:{color:'#22313e'}}},series:series.values.map(item=>({name:item.name,type,data:item.data,connectNulls:false,smooth:false}))});window.addEventListener('resize',()=>view.resize());}
  // 【逐行批注】 定义函数 teachingEvidenceRow，封装可复用的前端行为。
  function teachingEvidenceRow(e) {
    // 【逐行批注】 声明前端变量 request，保存本行计算出的数据或对象。
    const request = e.inputKind === 'teaching_request' || Boolean(e.requestType), followup = e.inputKind === 'followup';
    // 【逐行批注】 声明前端变量 mode，保存本行计算出的数据或对象。
    const mode = request ? '教学请求 · '+({hint:'给一点提示',worked_example:'看示范',rephrase:'换一种讲法',retry_question:'再试一道'}[e.requestType] || '帮助') :
      // 【逐行批注】 执行当前前端表达式：followup ? '追问，未作为作答评分' : {independent:'独立作答',after_hint:'提示后作答',after_example:'示范后作答',he…。
      followup ? '追问，未作为作答评分' : {independent:'独立作答',after_hint:'提示后作答',after_example:'示范后作答',help_unknown:'帮助程度未确定'}[e.helpMode] || '';
    // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
    let body = (e.question?'<h3>'+escape(e.question)+'</h3>':'')+'<p>'+escape(e.text||'已保存的真实学习互动')+'</p>';
    // 【逐行批注】 计算右侧结果并更新 body +，改变页面或请求状态。
    body += '<p class="ma-note">'+escape(time(e.at))+(mode?' · '+mode:'')+(e.rePracticeOf?' · 关联新题复练':'')+'</p>';
    // 【逐行批注】 调用 escape 执行页面交互或数据处理：if(e.kind==='practice' && e.feedback && e.feedbackHelpType!=='none')b…。
    if(e.kind==='practice' && e.feedback && e.feedbackHelpType!=='none')body+='<p class="ma-note">'+escape({
      // 【逐行批注】 执行当前前端表达式：explanation:'反馈包含相关概念讲解',hint:'反馈包含本题提示',worked_example:'反馈包含本题完整示范',。
      explanation:'反馈包含相关概念讲解',hint:'反馈包含本题提示',worked_example:'反馈包含本题完整示范',
      // 【逐行批注】 执行当前前端表达式：clarification:'反馈用于澄清思路'}[e.feedbackHelpType]||'反馈帮助程度未确定')+'</p>';。
      clarification:'反馈用于澄清思路'}[e.feedbackHelpType]||'反馈帮助程度未确定')+'</p>';
    // 【逐行批注】 计算右侧结果并更新 body +，改变页面或请求状态。
    body += '<div'+(e.kind==='practice' && e.lessonId?' data-report-feedback="'+escape(e.id)+'"':'')+'>';
    // 【逐行批注】 判断条件 (e.diagnosis?.summary) body += '<p>卡点：'+escape(e.diagnosis.summa…，成立时执行对应逻辑。
    if (e.diagnosis?.summary) body += '<p>卡点：'+escape(e.diagnosis.summary)+'</p>';
    // 【逐行批注】 判断条件 (e.diagnosis?.evidence?.length) body += '<p class="ma-note">回答依据…，成立时执行对应逻辑。
    if (e.diagnosis?.evidence?.length) body += '<p class="ma-note">回答依据：'+escape(e.diagnosis.evidence.join('；'))+'</p>';
    // 【逐行批注】 判断条件 (e.feedback) body += '<p>'+escape(e.feedback)+'</p>';，成立时执行对应逻辑。
    if (e.feedback) body += '<p>'+escape(e.feedback)+'</p>';
    // 【逐行批注】 判断条件 (e.uncertainty) body += '<p class="ma-note">'+escape(e.uncertain…，成立时执行对应逻辑。
    if (e.uncertainty) body += '<p class="ma-note">'+escape(e.uncertainty)+'</p>';
    // 【逐行批注】 计算右侧结果并更新 body +，改变页面或请求状态。
    body += '</div>';
    // 【逐行批注】 按循环规则遍历或重复处理：for (const h of request || followup ? [] : e.helpHistory || []) body += '<…。
    for (const h of request || followup ? [] : e.helpHistory || []) body += '<details><summary>'+escape({hint:'实际获得的提示',worked_example:'实际看到的完整示范',unknown:'程度未确定的帮助'}[h.helpType]||'教学帮助')+'</summary><p>'+escape(h.text)+'</p></details>';
    // 【逐行批注】 判断条件 (e.relatedLearning?.length) body += '<p class="ma-note">曾学过 '+e.…，成立时执行对应逻辑。
    if (e.relatedLearning?.length) body += '<p class="ma-note">曾学过 '+e.relatedLearning.length+' 段相关讲解。</p>';
    // 【逐行批注】 判断条件 (e.lessonId) body += '<div class="ma-actions"><a class="ma-link"…，成立时执行对应逻辑。
    if (e.lessonId) body += '<div class="ma-actions"><a class="ma-link" href="/learn?lessonId='+encodeURIComponent(e.lessonId)+'">查看对应课时 →</a>'+(e.sectionId?'<a class="ma-link" href="/'+(e.view==='learn'?'learn':'practice')+'?lessonId='+encodeURIComponent(e.lessonId)+'&sectionId='+encodeURIComponent(e.sectionId)+'">继续巩固 →</a>':'')+'</div>';
    // 【逐行批注】 当前一条件不成立时继续判断：(e.courseId) body += '<a class="ma-link" href="/plan?courseId='+…。
    else if (e.courseId) body += '<a class="ma-link" href="/plan?courseId='+encodeURIComponent(e.courseId)+'">查看对应课程 →</a>';
    // 【逐行批注】 结束当前函数并返回结果：return '<div class="ma-row" id="'+escape('evidence-'+e.id)+'">'+body+…。
    return '<div class="ma-row" id="'+escape('evidence-'+e.id)+'">'+body+'</div>';
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 reports，封装可复用的前端行为。
  async function reports() {
    // 【逐行批注】 声明前端变量 all，保存本行计算出的数据或对象。
    const all=await api('overview');const query={};for(const key of ['from','to','subject','courseId'])if(params.get(key))query[key]=params.get(key);
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    const data=await api('reports',null,query);
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML='<form class="ma-filters" id="report-filter"><input type="date" name="from" aria-label="开始日期" value="'+escape(query.from)+'"><input type="date" name="to" aria-label="结束日期" value="'+escape(query.to)+'"><select name="courseId" aria-label="课程"><option value="">全部课程</option>'+all.courses.map(c=>'<option value="'+escape(c.id)+'" '+(query.courseId===c.id?'selected':'')+'>'+escape(c.title)+'</option>').join('')+'</select><select name="subject" aria-label="学科"><option value="">全部学科</option>'+all.subjects.map(s=>'<option '+(query.subject===s.subject?'selected':'')+'>'+escape(s.subject)+'</option>').join('')+'</select><button class="ma-button">查看</button></form>'+
      // 【逐行批注】 调用 metric 执行页面交互或数据处理：'<div class="ma-metrics">'+metric(data.courses.length,'课程')+metric(da…。
      '<div class="ma-metrics">'+metric(data.courses.length,'课程')+metric(data.completed,'已完成课时')+metric(data.time.length?(data.studySeconds<60?'不足1分钟':Math.round(data.studySeconds/60)+' 分钟'):'尚未记录','有效学习时间')+metric(data.evidence.length,'当前范围学习记录')+'</div>'+
      // 【逐行批注】 调用 card 执行页面交互或数据处理：card('全局学习画像（跨课程）','<div data-profile-radar id="my-ai-radar"></div>')+。
      card('全局学习画像（跨课程）','<div data-profile-radar id="my-ai-radar"></div>')+
      // 【逐行批注】 计算右侧结果并更新 card('全局学科与薄弱点',data.subjects.length?data.su…，改变页面或请求状态。
      card('全局学科与薄弱点',data.subjects.length?data.subjects.map(s=>'<div class="ma-row"><h3>'+escape(s.subject)+' · '+score(s.score)+'</h3><p>'+escape(s.rationale)+'</p><p class="ma-note">薄弱点：'+escape((s.weakPoints||[]).join('、')||'暂无足够证据')+'</p><p>'+escape(s.recommendation)+'</p></div>').join(''):empty('完成真实作答后才会形成学习画像。'))+
      // 【逐行批注】 计算右侧结果并更新 card('课程进度',data.courses.length?data.courses…，改变页面或请求状态。
      card('课程进度',data.courses.length?data.courses.map(c=>'<div class="ma-row"><h3>'+escape(c.title)+'</h3><div class="ma-bar"><i style="width:'+Math.max(0,Math.min(100,c.percent))+'%"></i></div><p class="ma-note">'+c.completed+' / '+c.total+' 节 · '+c.percent+'%</p><a class="ma-link" href="/plan?courseId='+encodeURIComponent(c.id)+'">进入课程 →</a></div>').join(''):empty('当前没有课程。'))+
      // 【逐行批注】 计算右侧结果并更新 card('知识点掌握度',data.topics.length?data.topics…，改变页面或请求状态。
      card('知识点掌握度',data.topics.length?data.topics.map(t=>'<div class="ma-row"><h3>'+escape(t.topic)+' · '+score(t.score)+'</h3><p>'+escape(t.rationale)+'</p><p class="ma-note">依据 '+t.evidenceCount+' 条记录</p><p>'+escape(t.recommendation)+'</p><a class="ma-link" href="/my-ai/reports?courseId='+encodeURIComponent(t.courseId)+(t.evidenceIds?.[0]?'&evidenceId='+encodeURIComponent(t.evidenceIds[0]):'')+'#evidence">查看对应记录 →</a></div>').join(''):empty('当前范围的数据不足。'))+
      card('已完成复习',(data.completedReviews||[]).length?data.completedReviews.map(item=>{
        const details=escape(item.courseTitle)+' · '+escape(time(item.completedAt))+' · 完成 '+item.questionCount+' 题'+
          (item.changedCount?' · 评价依据已更新':'');
        const evidence=item.evidenceIds.map((id,index)=>'<a class="ma-link" href="/my-ai/reports?'+
          new URLSearchParams({courseId:item.courseId,evidenceId:id})+'#evidence">第 '+(index+1)+' 条评价</a>').join('');
        const timing=item.timing||{},duration=seconds=>seconds>=86400?(seconds/86400).toFixed(1)+' 天':seconds>=60?(seconds/60).toFixed(1)+' 分钟':seconds+' 秒';
        const interval=timing.status==='delay_met'?'各题提交均达到约定间隔，最短 '+duration(timing.minimumElapsedSeconds)+'。':
          timing.status==='too_early'?'本次提前复习，尚未达到约定间隔 '+duration(timing.minimumDelaySeconds)+'。':
          timing.status==='unavailable'?'提交时间记录不足，无法核对复习间隔。':'本次未约定最短间隔。';
        return '<div class="ma-row"><h3>'+escape(item.title)+'</h3><p class="ma-note">'+details+'</p>'+
          '<p class="ma-note">'+escape(interval)+' 完成复习不代表已验证长期记忆。</p>'+
          '<div class="ma-actions"><a class="ma-link" href="/practice?lessonId='+encodeURIComponent(item.lessonId)+
          '">回看复习</a>'+evidence+'</div></div>';
      }).join(''):empty('复习完成后，这里会显示作答依据。'))+
      // 【逐行批注】 调用 card、escape、empty 执行页面交互或数据处理：card('变化趋势','<p class="ma-note">'+escape(data.historyNote)+'</p>'+(da…。
      card('变化趋势','<p class="ma-note">'+escape(data.historyNote)+'</p>'+(data.history.some(h=>Object.values(h.scores).some(v=>typeof v==='number'))?'<div id="profile-chart" class="ma-chart"></div>':empty('完成有效学习并更新画像后，这里会展示变化趋势。')))+
      // 【逐行批注】 调用 card、empty 执行页面交互或数据处理：card('有效学习时间',(data.time.length?'<div id="time-chart" class="ma-chart…。
      card('有效学习时间',(data.time.length?'<div id="time-chart" class="ma-chart"></div>':empty('尚无升级后的有效学习时间记录。'))+'<p class="ma-note">只累计学习页面在前台、且最近五分钟有交互的时间。多窗口不会重复累计。</p>')+
      // 【逐行批注】 调用 empty 执行页面交互或数据处理：'<article class="ma-card" id="evidence"><h2>学习记录与依据</h2>'+(data.evide…。
      '<article class="ma-card" id="evidence"><h2>学习记录与依据</h2>'+(data.evidence.length?(params.get('evidenceId')?data.evidence.filter(e=>e.id===params.get('evidenceId')):data.evidence.slice(-100)).reverse().map(teachingEvidenceRow).join(''):empty('当前范围没有可展示的记录。'))+'</article>';
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('report-filter').ons…，改变页面或请求状态。
    document.getElementById('report-filter').onsubmit=event=>{event.preventDefault();const query=new URLSearchParams(new FormData(event.currentTarget));location.href='/my-ai/reports?'+query;};
    // 报告里看到的本题反馈也属于实际帮助；继续巩固前先保存展示，不触发模型。
    // 【逐行批注】 声明前端变量 evidenceById，保存本行计算出的数据或对象。
    const evidenceById=new Map(data.evidence.map(record=>[record.id,record])); let exposureFrame;
    // 【逐行批注】 声明前端变量 recordFeedback，保存本行计算出的数据或对象。
    const recordFeedback=()=>{for(const node of root.querySelectorAll('[data-report-feedback]')) {
      // 【逐行批注】 声明前端变量 record，保存本行计算出的数据或对象。
      const record=evidenceById.get(node.dataset.reportFeedback);
      // 【逐行批注】 执行当前前端表达式：if(record)window.GangyiAgent?.recordEvaluation(record.lessonId,record,node);。
      if(record)window.GangyiAgent?.recordEvaluation(record.lessonId,record,node);
    // 【逐行批注】 执行当前前端表达式：}};。
    }};
    // 【逐行批注】 声明前端变量 scheduleFeedback，保存本行计算出的数据或对象。
    const scheduleFeedback=()=>{cancelAnimationFrame(exposureFrame);exposureFrame=requestAnimationFrame(recordFeedback);};
    // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('scroll',scheduleFeedback,true);window.addEventListener('r…。
    document.addEventListener('scroll',scheduleFeedback,true);window.addEventListener('resize',scheduleFeedback);
    // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('visibilitychange',scheduleFeedback);root.addEventListener…。
    document.addEventListener('visibilitychange',scheduleFeedback);root.addEventListener('toggle',scheduleFeedback,true);
    // 【逐行批注】 为页面事件绑定处理逻辑：root.addEventListener('click',event=>{const link=event.target.closest('a');。
    root.addEventListener('click',event=>{const link=event.target.closest('a');
      // 【逐行批注】 执行当前前端表达式：if(!link||event.button!==0||event.ctrlKey||event.metaKey||event.shiftKey||!/^\/(learn|pra…。
      if(!link||event.button!==0||event.ctrlKey||event.metaKey||event.shiftKey||!/^\/(learn|practice)\?/.test(link.getAttribute('href')||''))return;
      // 【逐行批注】 调用 recordFeedback 执行页面交互或数据处理：event.preventDefault();recordFeedback();。
      event.preventDefault();recordFeedback();
      // 【逐行批注】 计算右侧结果并更新 (window.GangyiAgent?.flushExposure()||Promis…，改变页面或请求状态。
      (window.GangyiAgent?.flushExposure()||Promise.resolve()).then(()=>{location.href=link.href;}).catch(error=>status(error.message,true));
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 【逐行批注】 调用 scheduleFeedback 执行页面交互或数据处理：scheduleFeedback();。
    scheduleFeedback();
    // 【逐行批注】 执行当前前端表达式：if(window.GangyiProfileRadar)window.GangyiProfileRadar.mount(document.getElementById('my-…。
    if(window.GangyiProfileRadar)window.GangyiProfileRadar.mount(document.getElementById('my-ai-radar'));
    // 【逐行批注】 声明前端变量 history，保存本行计算出的数据或对象。
    const history=[...data.history].reverse(),subjects=[...new Set(history.flatMap(h=>Object.keys(h.scores)))];chart(document.getElementById('profile-chart'),{dates:history.map(h=>time(h.updatedAt)),values:subjects.map(name=>({name,data:history.map(h=>h.scores[name]??null)}))});
    // 【逐行批注】 声明前端变量 daily，保存本行计算出的数据或对象。
    const daily={};for(const t of data.time)daily[t.day]=(daily[t.day]||0)+t.seconds/60;const days=Object.keys(daily).sort();chart(document.getElementById('time-chart'),{dates:days,values:[{name:'学习分钟',data:days.map(day=>Math.round(daily[day]))}]},'bar');status('课程进度为当前状态；时间和记录按筛选范围显示。画像依据跨课程的真实作答。');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 plan，封装可复用的前端行为。
  async function plan() {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    let data=await existing('/api/study-plan');
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML=card('每周可学习时间','<div class="ma-plan-grid">'+['周一','周二','周三','周四','周五','周六','周日'].map((name,i)=>{const slot=data.availability.find(s=>s.weekday===i+1);return '<label class="ma-plan-day"><input type="checkbox" data-day="'+(i+1)+'" '+(slot?'checked':'')+'> '+name+'<input type="number" min="1" max="1440" aria-label="'+name+'分钟数" value="'+(slot?.minutes||30)+'">分钟</label>';}).join('')+'</div><p class="ma-note">这是所有课程共用的每周时间预算。重新排课会先生成候选方案，确认后应用。</p>'+button('保存学习时间','save-time'))+'<div id="uc-study-plan-anchor"></div>';
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('save-time').onclick，改变页面或请求状态。
    document.getElementById('save-time').onclick=action(async()=>{const availability=[...root.querySelectorAll('.ma-plan-day')].filter(l=>l.querySelector('[type=checkbox]').checked).map(l=>({weekday:Number(l.querySelector('[type=checkbox]').dataset.day),minutes:Number(l.querySelector('[type=number]').value)}));if(!availability.length)throw new Error('请至少选择一个学习日。');data=await existing('/api/study-plan',{version:data.version,availability},'PUT');status('学习时间已保存。');});
    // 【逐行批注】 声明前端变量 style，保存本行计算出的数据或对象。
    const style=document.createElement('link');style.rel='stylesheet';style.href='/learning-flow.css';document.head.append(style);const script=document.createElement('script');script.src='/learning-flow.js';document.body.append(script);status('在这里管理全部课程的学习时间和安排。');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 tasks，封装可复用的前端行为。
  async function tasks() {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    const data=await api('tasks'),activity=await existing('/api/ai-activity');
    // 【逐行批注】 声明前端变量 rows，保存本行计算出的数据或对象。
    const rows=data.items||[];root.innerHTML=card('全软件AI任务','<p>当前状态：'+(activity.paused?'已暂停':'可执行')+'</p><div class="ma-actions">'+button(activity.paused?'恢复AI请求':'暂停全部AI请求','toggle-ai')+'</div>')+card('任务记录',rows.length?rows.map((t,i)=>'<div class="ma-row"><h3>'+escape(t.purpose||'学习任务')+'</h3><p>'+escape(taskLabels[t.status]||'处理中')+'</p><p class="ma-note">'+escape(t.message||t.error||t.failure?.message||'')+'</p><div class="ma-actions"><button class="ma-button secondary" data-task="'+i+'" data-command="'+(['failed','superseded'].includes(t.status)?'retry':'resume')+'">'+(['failed','superseded'].includes(t.status)?'重试':'继续')+'</button><button class="ma-button secondary" data-task="'+i+'" data-command="pause">暂停</button></div></div>').join(''):empty('还没有任务记录。'));
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('toggle-ai').onclick，改变页面或请求状态。
    document.getElementById('toggle-ai').onclick=action(async()=>{await existing('/api/ai-activity/control',{command:activity.paused?'resume':'pause'});await tasks();});
    // 【逐行批注】 计算右侧结果并更新 root.querySelectorAll('[data-task]').forEach…，改变页面或请求状态。
    root.querySelectorAll('[data-task]').forEach(b=>b.onclick=action(async()=>{await existing('/api/learning-agent/control',{taskId:rows[Number(b.dataset.task)].id,command:b.dataset.command});await tasks();}));status('任务与实际模型请求分别记录，可在使用情况页查看调用明细。');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 usage，封装可复用的前端行为。
  async function usage() {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    const data=await api('usage');
    // 【逐行批注】 声明前端变量 fields，保存本行计算出的数据或对象。
    const fields=[['input','输入'],['output','输出'],['cachedInput','缓存命中（输入的一部分）'],['uncachedInput','缓存未命中'],['reasoning','思考（输出的一部分）']];
    // 【逐行批注】 声明前端变量 breakdown，保存本行计算出的数据或对象。
    const breakdown=fields.map(([key,label])=>{const item=data.tokenBreakdown?.[key];return '<div class="ma-row"><strong>'+escape(label)+'：'+escape(item?.tokens??'未提供')+'</strong><p class="ma-note">'+escape(item?.knownCalls??0)+' / '+escape(data.calls)+' 次请求提供'+(item?.overflow?'；合计超出统计范围':'')+'</p></div>';}).join('');
    // 【逐行批注】 声明前端变量 details，保存本行计算出的数据或对象。
    const details=tokens=>'<details><summary>展开明细</summary>'+[
      // 【逐行批注】 执行当前前端表达式：['输入',tokens?.prompt_tokens],['输出',tokens?.completion_tokens],。
      ['输入',tokens?.prompt_tokens],['输出',tokens?.completion_tokens],
      // 【逐行批注】 执行当前前端表达式：['缓存命中',tokens?.prompt_cache_hit_tokens??tokens?.prompt_tokens_details?.cached_tokens],。
      ['缓存命中',tokens?.prompt_cache_hit_tokens??tokens?.prompt_tokens_details?.cached_tokens],
      // 【逐行批注】 执行当前前端表达式：['缓存未命中',tokens?.prompt_cache_miss_tokens],['思考',tokens?.completion_tokens_details?.reaso…。
      ['缓存未命中',tokens?.prompt_cache_miss_tokens],['思考',tokens?.completion_tokens_details?.reasoning_tokens]
    // 【逐行批注】 计算右侧结果并更新 ].map(([label,value])，改变页面或请求状态。
    ].map(([label,value])=>'<p class="ma-note">'+escape(label)+'：'+escape(value??'未提供')+'</p>').join('')+'</details>';
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML='<div class="ma-metrics">'+metric(data.calls,'实际模型请求')+metric(data.tasks,'关联学习任务')+metric(data.success,'成功请求')+metric(data.tokens??'未提供','已知Token合计')+'</div>'+card('Token构成','<p class="ma-note">总量由 '+escape(data.knownTokenCalls)+' / '+escape(data.calls)+' 次请求提供。各项独立统计，不能相互相加推算。</p><div class="ma-grid">'+breakdown+'</div>')+card('调用明细','<p class="ma-note">'+escape(data.note)+'</p><div class="ma-table-wrap"><table class="ma-table"><thead><tr><th>时间</th><th>来源与用途</th><th>模型</th><th>状态</th><th>HTTP耗时 / 任务耗时</th><th>Token</th></tr></thead><tbody>'+data.items.map(r=>'<tr><td>'+escape(time(r.at))+'</td><td>'+escape(r.source)+'<br><small>'+escape(r.purpose)+'</small></td><td>'+escape(r.model)+'</td><td>'+escape(taskLabels[r.status]||r.status)+'</td><td>'+(Number.isFinite(r.durationMs)?(r.durationMs/1000).toFixed(1)+'秒':'未知')+'<br><small>'+[['queueMs','排队'],['firstContentMs','首有效内容'],['completionMs','完整完成']].map(([key,label])=>label+'：'+(Number.isFinite(r[key])?(r[key]/1000).toFixed(1)+'秒':'未知')).join('<br>')+'</small></td><td>'+escape(r.tokens?.total_tokens??'未提供')+details(r.tokens)+'</td></tr>').join('')+'</tbody></table></div>'+(data.items.length?'':empty('还没有升级后的实际模型请求。')));status('Token仅使用服务商返回值；缓存与思考不重复计入总量。');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 libraryUsageView，封装可复用的前端行为。
  async function libraryUsageView() {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    const data=await existing('/api/my-ai/library/usage');
    // 【逐行批注】 调用 card、metric 执行页面交互或数据处理：root.insertAdjacentHTML('afterbegin',card('联网资料使用情况','<div class="ma-…。
    root.insertAdjacentHTML('afterbegin',card('联网资料使用情况','<div class="ma-metrics">'+metric(data.searchRequests,'两家搜索实际请求')+metric(data.providers?.deepseek?.requests??0,'DeepSeek 搜索请求')+metric(data.providers?.bocha?.requests??0,'博查搜索请求')+metric(data.fetchRequests,'正文获取请求')+metric(data.cacheHits,'缓存命中')+metric(data.failures,'失败记录')+'</div><p class="ma-note">检索与正文读取单独统计，Token仅显示模型服务商实际返回值。</p>'));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 settings，封装可复用的前端行为。
  async function settings() {
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML=card('AI服务与模型','<p>使用桌面设置配置服务商、接口地址、模型和密钥，并测试连接。</p><p class="ma-note">密钥保存在Windows凭据管理器。调整配置后按桌面设置提示应用到服务。</p>'+button('打开AI设置','open-settings'))+card('桌面设置','<p class="ma-note">开机启动及现有桌面选项在同一设置窗口管理。</p>');
    // 【逐行批注】 计算右侧结果并更新 if(window.gangyiDesktopInfo){const info，改变页面或请求状态。
    if(window.gangyiDesktopInfo){const info=await window.gangyiDesktopInfo();const summary=document.createElement('article');summary.className='ma-card';summary.innerHTML='<h2>当前AI配置</h2><p>服务商：'+escape(info.provider)+'</p><p>模型：'+escape(info.model)+'</p><p class="ma-note">接口：'+escape(info.baseUrl)+'</p><p>'+ (info.configured?'已配置密钥':'尚未配置密钥')+'</p>';root.prepend(summary);}
    // 【逐行批注】 声明前端变量 suggestions，保存本行计算出的数据或对象。
    const suggestions=(await api('actions')).items.filter(a=>a.kind==='settings'&&a.state==='awaiting_native'&&a.target);
    // 【逐行批注】 按循环规则遍历或重复处理：for(const item of suggestions){const view=document.createElement('article'…。
    for(const item of suggestions){const view=document.createElement('article');view.className='ma-card';view.innerHTML='<h2>待检查的设置建议</h2><p>'+escape(item.impact)+'</p><p class="ma-note">打开后会预填建议模型；检查配置并点击桌面窗口的保存按钮后才生效。</p>';const open=document.createElement('button');open.className='ma-button';open.textContent='打开并检查建议配置';open.onclick=action(async()=>{if(!window.gangyiOpenApiSettings)throw new Error('请在Windows桌面应用中检查配置。');await window.gangyiOpenApiSettings({model:item.target,before:item.before?.model||''});status('已打开建议配置，等待你检查并保存。');});view.append(open);root.append(view);}
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('open-settings').onc…，改变页面或请求状态。
    document.getElementById('open-settings').onclick=action(async()=>{if(!window.gangyiOpenApiSettings)throw new Error('请在桌面应用中打开此页面，或从托盘进入设置。');await window.gangyiOpenApiSettings();status('已打开桌面设置。');});status('配置操作由桌面设置窗口确认保存。');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 showActions，封装可复用的前端行为。
  async function showActions(host) {
    // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
    const data=await api('actions');const pending=data.items.filter(a=>a.state==='pending');
    // 【逐行批注】 计算右侧结果并更新 host.innerHTML，改变页面或请求状态。
    host.innerHTML=pending.map(a=>'<article class="ma-card ma-confirm"><h3>请确认操作</h3><p>'+escape(a.impact)+'</p><p class="ma-note">'+escape(a.before?.title||'')+' '+escape(a.reason)+'</p><div class="ma-actions"><button class="ma-button danger" data-action="'+escape(a.id)+'" data-decision="confirm">确认执行</button><button class="ma-button secondary" data-action="'+escape(a.id)+'" data-decision="cancel">取消</button></div></article>').join('');
    // 【逐行批注】 计算右侧结果并更新 host.querySelectorAll('[data-action]').forEa…，改变页面或请求状态。
    host.querySelectorAll('[data-action]').forEach(b=>b.onclick=action(async()=>{const item=pending.find(a=>a.id===b.dataset.action);const result=await api('confirm',{id:item.id,version:item.version,decision:b.dataset.decision});await showActions(host);status(result.state==='cancelled'?'操作已取消。':result.state==='awaiting_native'?'请在对应独立页面完成操作。':'操作已完成。');if(result.href){const a=document.createElement('a');a.className='ma-button';a.href=result.href;a.textContent='打开对应页面 →';host.append(a);}}));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 dataPage，封装可复用的前端行为。
  async function dataPage() {
    // 【逐行批注】 声明前端变量 overview，保存本行计算出的数据或对象。
    const overview=await api('overview');root.innerHTML='<div id="pending-actions"></div>'+card('档案备份与恢复','<p>备份包含课程、学习记录、偏好、记忆和管家聊天。</p><div class="ma-actions">'+button('备份本机档案','backup')+button('从备份恢复','restore','secondary')+'</div><p class="ma-note">恢复前会自动保存当前数据库的回滚备份，随后由桌面窗口确认。</p>')+card('清理课程','<div class="ma-field"><label for="delete-course">选择课程</label><select id="delete-course"><option value="">请选择</option>'+overview.courses.map(c=>'<option value="'+escape(c.id)+'">'+escape(c.title)+'</option>').join('')+'</select></div>'+button('预览删除影响','delete-course-preview','danger'))+card('清理管家记录','<p class="ma-note">先检查影响范围，再确认执行。</p><div class="ma-actions">'+button('清空管家聊天','clear-chats','danger')+button('清空长期记忆','clear-memories','danger')+'</div>');
    // 【逐行批注】 调用 showActions 执行页面交互或数据处理：await showActions(document.getElementById('pending-actions'));。
    await showActions(document.getElementById('pending-actions'));
    // 【逐行批注】 按循环规则遍历或重复处理：for(const [id,method] of [['backup','gangyiBackupData'],['restore','gangyi…。
    for(const [id,method] of [['backup','gangyiBackupData'],['restore','gangyiRestoreData']])document.getElementById(id).onclick=action(async()=>{if(!window[method])throw new Error('此操作需要在Windows桌面应用中进行。');await window[method]();status('已打开桌面操作，结果以系统提示为准。');});
    // 【逐行批注】 按循环规则遍历或重复处理：for(const [id,kind] of [['delete-course-preview','delete_course'],['clear-…。
    for(const [id,kind] of [['delete-course-preview','delete_course'],['clear-chats','clear_chats'],['clear-memories','clear_memories']])document.getElementById(id).onclick=action(async()=>{const target=kind==='delete_course'?document.getElementById('delete-course').value:'';if(kind==='delete_course'&&!target)throw new Error('请先选择课程。');await api('proposal',{kind,target,reason:'你在数据管理页发起了此操作',requestId:crypto.randomUUID()});await showActions(document.getElementById('pending-actions'));window.scrollTo({top:0,behavior:'smooth'});});status('重要操作先预览，再确认。');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 chat，封装可复用的前端行为。
  async function chat() {
    // 【逐行批注】 声明前端变量 archived，保存本行计算出的数据或对象。
    const archived=params.get('archived')==='true',list=await api('chats',null,{archived}),chatId=params.get('id');let state=chatId?await api('chat',null,{id:chatId}):null;
    // 【逐行批注】 计算右侧结果并更新 root.innerHTML，改变页面或请求状态。
    root.innerHTML='<div class="ma-chat-layout"><aside class="ma-chat-list">'+button('＋ 新聊天','new-chat')+'<input class="ma-search" id="chat-search" aria-label="搜索聊天" placeholder="搜索聊天"><a class="ma-link" href="/my-ai/chat'+(archived?'':'?archived=true')+'">'+(archived?'查看进行中的聊天':'查看已归档聊天')+'</a><div id="chat-list">'+list.items.map(c=>'<a class="ma-chat-item '+(c.id===chatId?'is-active':'')+'" href="/my-ai/chat?id='+encodeURIComponent(c.id)+(archived?'&archived=true':'')+'">'+escape(c.title)+'</a>').join('')+'</div></aside><div class="ma-chat-room"><header class="ma-chat-title"><h2>'+escape(state?.title||'AI管家')+'</h2>'+(state?'<div class="ma-actions">'+button('重命名','rename-chat','secondary')+button(state.archived?'取消归档':'归档','archive-chat','secondary')+'</div>':'')+'</header><div id="chat-agent-trace"></div><div class="ma-chat-scroll" id="chat-scroll"><div id="chat-messages"></div><div id="chat-actions"></div></div><form class="ma-chat-composer" id="chat-form"><label class="sr-only" for="chat-input">告诉AI你想做什么</label><textarea id="chat-input" placeholder="例如：以后讲解先举生活中的例子；看看我最近哪些知识点需要巩固。"></textarea><div class="ma-actions"><span class="ma-task-status" id="chat-task-status">输入你的要求，AI会通过实际工具处理。</span><div>'+button('停止','stop-chat','secondary')+' '+button('继续','resume-chat','secondary')+' '+button('重试','retry-chat','secondary')+' <button class="ma-button" id="send-chat">发送</button></div></div></form></div></div>';
    // 【逐行批注】 声明前端变量 input，保存本行计算出的数据或对象。
    const input=document.getElementById('chat-input'),scroll=document.getElementById('chat-scroll'),messages=document.getElementById('chat-messages'),taskStatus=document.getElementById('chat-task-status');
    // 【逐行批注】 声明前端变量 form，保存本行计算出的数据或对象。
    const form=document.getElementById('chat-form'),sendButton=document.getElementById('send-chat');
    // 【逐行批注】 声明前端变量 stopButton，保存本行计算出的数据或对象。
    const stopButton=document.getElementById('stop-chat'),resumeButton=document.getElementById('resume-chat'),retryButton=document.getElementById('retry-chat');
    // 【逐行批注】 声明前端变量 lastStatusText，保存本行计算出的数据或对象。
    let lastStatusText='';
    // 【逐行批注】 声明前端变量 setTaskStatus，保存本行计算出的数据或对象。
    const setTaskStatus=text=>{if(lastStatusText!==text){lastStatusText=text;taskStatus.textContent=text;}};
    // 新建聊天发送失败时 state 已存在但 URL 还没有 id，后续草稿、轮询和重试都按实际聊天编号工作。
    // 【逐行批注】 声明前端变量 sessionId，保存本行计算出的数据或对象。
    const sessionId=()=>state?.id||chatId;
    const agent = window.GangyiAgent;
    const liveHost = document.createElement('div'); liveHost.id = 'streaming-reply';
    const liveView = agent.createStreamView(liveHost, {filter:record => record.field === 'message' &&
      !(record.state === 'confirmed' && (state?.messages || []).some(message => message.role === 'assistant' && message.text === record.text))});
    let watchedTask = '', stopLiveWatch, liveRecords = [];
    function watchReply(taskId) {
      if (!taskId || watchedTask === taskId) return;
      stopLiveWatch?.(); watchedTask = taskId; liveRecords = []; liveView.clear();
      stopLiveWatch = agent.watch(taskId, {
        onContent(records) {
          if (watchedTask !== taskId) return;
          liveRecords = records; liveView.update(records);
          if (liveHost.parentNode !== messages) messages.append(liveHost);
        },
        onEvent(event) { if (event.type === 'action') pollOnce(); return false; },
        onState(task) {
          if (watchedTask !== taskId) return;
          chatTrace?.update(task);
          setTaskStatus((taskLabels[task.status] || '正在处理') + (task.activity?.title ? ' · ' + task.activity.title : ''));
          if (!['pending','running'].includes(task.status)) { watchedTask = ''; pollOnce(); }
        },
        onConnectionError(message) { setTaskStatus(message); },
      });
    }
    window.addEventListener('pagehide', () => stopLiveWatch?.(), {once:true});

    // 共享轨迹统一展示任务编号、阶段、持续计时、已保存步骤和失败原因；页面不再另造一套步骤/重试界面。
    // 【逐行批注】 声明前端变量 control，保存本行计算出的数据或对象。
    const control=async(command,taskId)=>{
      // 【逐行批注】 执行当前前端表达式：if(!taskId)return;。
      if(!taskId)return;
      // 【逐行批注】 执行当前前端表达式：try{。
      try{
        // 【逐行批注】 调用 existing 执行页面交互或数据处理：await existing('/api/learning-agent/control',{taskId,command});。
        await existing('/api/learning-agent/control',{taskId,command});
        // 【逐行批注】 调用 setTaskStatus 执行页面交互或数据处理：setTaskStatus(({pause:'任务已暂停，输入和有效结果已保留。',resume:'已恢复 AI 处理。',retry:'…。
        setTaskStatus(({pause:'任务已暂停，输入和有效结果已保留。',resume:'已恢复 AI 处理。',retry:'已请求从失败步骤重试。',cancel:'已停止本次 AI 处理，已保存内容保留。'})[command]||'已更新任务状态。');
        // 【逐行批注】 调用 pollOnce 执行页面交互或数据处理：await pollOnce();。
        await pollOnce();
      // 【逐行批注】 调用 status 执行页面交互或数据处理：}catch(error){status('操作未完成：'+error.message,true);}。
      }catch(error){status('操作未完成：'+error.message,true);}
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 声明前端变量 resubmit，保存本行计算出的数据或对象。
    const resubmit=()=>{if(typeof form?.requestSubmit==='function')form.requestSubmit();};
    // 【逐行批注】 声明前端变量 chatTrace，保存本行计算出的数据或对象。
    const chatTrace=window.GangyiTrace?.create(document.getElementById('chat-agent-trace'),{variant:'inline',eyebrow:'AI 管家执行过程',collapseWhenDone:true,
      // 【逐行批注】 计算右侧结果并更新 onPause:task，改变页面或请求状态。
      onPause:task=>control('pause',state?.taskId||task?.id),
      // 【逐行批注】 计算右侧结果并更新 onResume:task，改变页面或请求状态。
      onResume:task=>control('resume',state?.taskId||task?.id),
      // 【逐行批注】 计算右侧结果并更新 onRetry:task，改变页面或请求状态。
      onRetry:task=>String(task?.id||'').startsWith('chat-local-')?Promise.resolve(resubmit()):control('retry',state?.taskId||task?.id),
      // 【逐行批注】 计算右侧结果并更新 onCancel:task，改变页面或请求状态。
      onCancel:task=>control('cancel',state?.taskId||task?.id),
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 有轨迹时由轨迹承载暂停/停止/恢复/重试，旧按钮只在没有轨迹组件时兜底。
    // 【逐行批注】 计算右侧结果并更新 if(chatTrace)for(const controlButton of [sto…，改变页面或请求状态。
    if(chatTrace)for(const controlButton of [stopButton,resumeButton,retryButton])controlButton.hidden=true;
    // 提交瞬间的本地任务：拿到服务端真实任务编号后立即替换，避免轨迹与页面状态矛盾。
    // 【逐行批注】 声明前端变量 localTask，保存本行计算出的数据或对象。
    const localTask=()=>({id:'chat-local-'+Date.now(),status:'pending',purpose:'AI 管家处理你的要求',calls:0,createdAt:new Date().toISOString(),updatedAt:new Date().toISOString(),
      // 【逐行批注】 调用 Date 执行页面交互或数据处理：activity:{title:'正在提交给 AI',detail:'要求已发出，等待服务端返回任务编号。',state:'pending…。
      activity:{title:'正在提交给 AI',detail:'要求已发出，等待服务端返回任务编号。',state:'pending',at:new Date().toISOString()},events:[]});
    // 【逐行批注】 声明前端变量 renderMessages，保存本行计算出的数据或对象。
    const messageNodes = new Map();
    const renderMessages = items => {
      const near = scroll.scrollHeight - scroll.scrollTop - scroll.clientHeight < 100;
      const used = new Set(), ordered = [];
      for (const [index, message] of items.entries()) {
        const key = message.id || (message.requestId ? message.requestId + ':' + message.role : String(index));
        used.add(key);
        const signature = JSON.stringify([message.role, message.text, message.revisions]);
        let entry = messageNodes.get(key);
        if (!entry || entry.signature !== signature) {
          const node = document.createElement('article'); node.className = 'ma-message ' + message.role;
          const label = document.createElement('div'); label.className = 'ma-message-label'; label.textContent = message.role === 'user' ? '我' : 'AI管家';
          const content = document.createElement('div'); content.className = 'ma-message-content';
          if (message.role === 'assistant' && window.GangyiChat) content.innerHTML = window.GangyiChat.renderMarkdown(message.text, {math:true});
          else content.textContent = message.text;
          node.append(label, content);
          if (message.revisions?.length) agent.streamNotice(node, {state:'confirmed', revisions:message.revisions});
          entry = {node, signature}; messageNodes.set(key, entry);
        }
        ordered.push(entry.node);
      }
      let cursor = messages.firstChild;
      for (const node of ordered) { if (node === cursor) cursor = cursor.nextSibling; else messages.insertBefore(node, cursor); }
      for (const [key, entry] of messageNodes) if (!used.has(key)) { entry.node.remove(); messageNodes.delete(key); }
      for (const node of [...messages.children]) if (!ordered.includes(node) && node !== liveHost) node.remove();
      liveView.update(liveRecords);
      if (liveHost.parentNode !== messages || liveHost !== messages.lastElementChild) messages.append(liveHost);
      if (near) scroll.scrollTop = scroll.scrollHeight;
    };
    // 【逐行批注】 计算右侧结果并更新 if(state){renderMessages(state.messages);inp…，改变页面或请求状态。
    if(state){renderMessages(state.messages);input.value=localStorage.getItem('my-ai-draft:'+chatId)??state.draft;scroll.scrollTop=state.scroll||0;sessionStorage.setItem('my-ai-return-chat',chatId);}else messages.innerHTML=empty('新建一个聊天，或直接输入要求开始。');
    agent.followLatest(scroll);
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('new-chat').onclick，改变页面或请求状态。
    document.getElementById('new-chat').onclick=action(async()=>{const item=await api('chats',{title:'新聊天'});location.href='/my-ai/chat?id='+encodeURIComponent(item.id);});
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('chat-search').oninp…，改变页面或请求状态。
    document.getElementById('chat-search').oninput=async event=>{try{const data=await api('chats',null,{archived,search:event.target.value});document.getElementById('chat-list').innerHTML=data.items.map(c=>'<a class="ma-chat-item" href="/my-ai/chat?id='+encodeURIComponent(c.id)+'">'+escape(c.title)+'</a>').join('');}catch(error){status(error.message,true);}};
    // 【逐行批注】 计算右侧结果并更新 if(state){document.getElementById('rename-ch…，改变页面或请求状态。
    if(state){document.getElementById('rename-chat').onclick=action(async()=>{const title=prompt('聊天名称',state.title);if(title===null)return;state=await api('chat',{id:sessionId(),version:state.version,title});location.reload();});document.getElementById('archive-chat').onclick=action(async()=>{await api('chat',{id:sessionId(),version:state.version,archived:!state.archived});location.href='/my-ai/chat';});}
    // 【逐行批注】 计算右侧结果并更新 resumeButton.hidden，改变页面或请求状态。
    resumeButton.hidden=true;retryButton.hidden=true;
    // 【逐行批注】 按循环规则遍历或重复处理：for(const [controlButton,command] of [[resumeButton,'resume'],[retryButton…。
    for(const [controlButton,command] of [[resumeButton,'resume'],[retryButton,'retry']])controlButton.onclick=action(async()=>{await control(command,state?.taskId);});
    // 【逐行批注】 声明前端变量 draftTimer，保存本行计算出的数据或对象。
    let draftTimer,dirty=false,submitting=false,request=null;
    // 【逐行批注】 计算右侧结果并更新 input.oninput，改变页面或请求状态。
    input.oninput=()=>{dirty=true;if(chatId)localStorage.setItem('my-ai-draft:'+chatId,input.value);clearTimeout(draftTimer);draftTimer=setTimeout(async()=>{if(!state||submitting)return;try{state=await api('chat',{id:sessionId(),version:state.version,draft:input.value,scroll:scroll.scrollTop});dirty=false;}catch(error){status('草稿已保存在此电脑浏览器中；'+error.message,true);}},800);};
    // 【逐行批注】 按循环规则遍历或重复处理：form.onsubmit=async event=>{event.preventDefault();if(submitting||!input.v…。
    form.onsubmit=async event=>{event.preventDefault();if(submitting||!input.value.trim())return;submitting=true;
      // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
      const label=sendButton.textContent;sendButton.disabled=true;sendButton.textContent='正在提交…';
      // 【逐行批注】 执行当前前端表达式：if(typeof form.setAttribute==='function')form.setAttribute('aria-busy','true');。
      if(typeof form.setAttribute==='function')form.setAttribute('aria-busy','true');
      // 提交瞬间就显示等待态：本地 pending 任务包含编号、用途与起始时间，供轨迹持续计时。
      // 【逐行批注】 声明前端变量 pendingTask，保存本行计算出的数据或对象。
      const pendingTask=localTask();chatTrace?.update(pendingTask);setTaskStatus('正在提交给 AI…');
      // 【逐行批注】 调用 api、encodeURIComponent 执行页面交互或数据处理：try{if(!state){state=await api('chats',{title:input.value.slice(0,24)…。
      try{if(!state){state=await api('chats',{title:input.value.slice(0,24)||'新聊天'});await api('chat/send',{id:state.id,text:input.value,requestId:crypto.randomUUID()});location.href='/my-ai/chat?id='+encodeURIComponent(state.id);return;}
        // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
        const value=input.value;request=request&&request.text===value?request:{text:value,requestId:crypto.randomUUID()};const task=await api('chat/send',{id:sessionId(),...request});
        // 服务端提交响应就是真实任务对象，立即替换本地等待态。
        // 【逐行批注】 调用 Date 执行页面交互或数据处理：chatTrace?.update({...pendingTask,...task,purpose:task.purpose||pendi…。
        chatTrace?.update({...pendingTask,...task,purpose:task.purpose||pendingTask.purpose,createdAt:task.createdAt||pendingTask.createdAt,updatedAt:task.updatedAt||new Date().toISOString()});
        // 【逐行批注】 计算右侧结果并更新 state.taskId，改变页面或请求状态。
        state.taskId=task.id;watchReply(task.id);input.value='';localStorage.removeItem('my-ai-draft:'+sessionId());dirty=false;request=null;state=await api('chat',null,{id:sessionId()});renderMessages(state.messages);setTaskStatus('AI正在处理，可随时停止。');scroll.scrollTop=scroll.scrollHeight;
      // 【逐行批注】 执行当前前端表达式：}catch(error){。
      }catch(error){
        // 提交阶段失败：保留已输入内容和已有记录，并在轨迹里就地显示失败原因与已耗时。
        // 【逐行批注】 调用 Date 执行页面交互或数据处理：chatTrace?.update({...pendingTask,status:'failed',updatedAt:new Date(…。
        chatTrace?.update({...pendingTask,status:'failed',updatedAt:new Date().toISOString(),failure:{message:error.message||'提交未成功，请重试。'},
          // 【逐行批注】 调用 Date 执行页面交互或数据处理：activity:{title:'提交未成功',detail:'要求没有成功送达；已输入的内容和已有学习记录都保留。',state:'fa…。
          activity:{title:'提交未成功',detail:'要求没有成功送达；已输入的内容和已有学习记录都保留。',state:'failed',at:new Date().toISOString()}});
        // 【逐行批注】 调用 setTaskStatus、status 执行页面交互或数据处理：setTaskStatus('提交未成功：'+(error.message||'请重试。'));status(error.message,…。
        setTaskStatus('提交未成功：'+(error.message||'请重试。'));status(error.message,true);
      // 【逐行批注】 计算右侧结果并更新 }finally{submitting，改变页面或请求状态。
      }finally{submitting=false;sendButton.disabled=false;sendButton.textContent=label;if(typeof form.removeAttribute==='function')form.removeAttribute('aria-busy');}
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 计算右侧结果并更新 input.onkeydown，改变页面或请求状态。
    input.onkeydown=event=>{if(event.key==='Enter'&&!event.shiftKey&&!event.isComposing){event.preventDefault();document.getElementById('chat-form').requestSubmit();}};
    // 【逐行批注】 计算右侧结果并更新 stopButton.onclick，改变页面或请求状态。
    stopButton.onclick=action(async()=>{await control('pause',state?.taskId);});
    // 【逐行批注】 调用 showActions 执行页面交互或数据处理：await showActions(document.getElementById('chat-actions'));。
    await showActions(document.getElementById('chat-actions'));
    // 【逐行批注】 声明前端变量 polling，保存本行计算出的数据或对象。
    let polling=false,lastMessages='';
    // 轮询只更新既有节点：轨迹、状态文本和流式草稿都按值比较后写入，不重建整块界面。
    // 【逐行批注】 定义函数 pollOnce，封装可复用的前端行为。
    async function pollOnce(){
      // 【逐行批注】 执行当前前端表达式：if(!state||document.hidden||polling||submitting)return;。
      if(!state||document.hidden||polling||submitting)return;
      // 【逐行批注】 计算右侧结果并更新 polling，改变页面或请求状态。
      polling=true;
      // 【逐行批注】 执行当前前端表达式：try{。
      try{
        // 【逐行批注】 声明前端变量 latest，保存本行计算出的数据或对象。
        const latest=await api('chat',null,{id:sessionId()});
        // 【逐行批注】 执行当前前端表达式：if(!dirty)state=latest;else state.taskId=latest.taskId;。
        if(!dirty)state=latest;else state.taskId=latest.taskId;
        // 【逐行批注】 声明前端变量 signature，保存本行计算出的数据或对象。
        const signature=JSON.stringify(latest.messages);
        // 【逐行批注】 调用 renderMessages 执行页面交互或数据处理：if(signature!==lastMessages){renderMessages(latest.messages);lastMess…。
        if(signature!==lastMessages){renderMessages(latest.messages);lastMessages=signature;}
        // 【逐行批注】 执行当前前端表达式：if(state.taskId){。
        if(state.taskId){
          // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
          const task=await existing('/api/learning-agent?taskId='+encodeURIComponent(state.taskId));
          // 【逐行批注】 执行当前前端表达式：chatTrace?.update(task);。
          chatTrace?.update(task);
          // 没有轨迹组件时才显示手写控制按钮，避免与轨迹操作重复。
          // 【逐行批注】 执行当前前端表达式：if(!chatTrace){stopButton.hidden=!['running','pending'].includes(task.status);resumeButto…。
          if(!chatTrace){stopButton.hidden=!['running','pending'].includes(task.status);resumeButton.hidden=task.status!=='paused';retryButton.hidden=!['failed','superseded'].includes(task.status);}
          // 【逐行批注】 调用 setTaskStatus 执行页面交互或数据处理：setTaskStatus((taskLabels[task.status]||'正在处理')+(task.activity?.title…。
          setTaskStatus((taskLabels[task.status]||'正在处理')+(task.activity?.title?' · '+task.activity.title:''));
          // 【逐行批注】 声明前端变量 old，保存本行计算出的数据或对象。
          if (['pending','running'].includes(task.status)) watchReply(task.id);
          liveView.update(liveRecords);
          if (liveHost.parentNode !== messages) messages.append(liveHost);

        }
        // 【逐行批注】 调用 showActions 执行页面交互或数据处理：await showActions(document.getElementById('chat-actions'));。
        await showActions(document.getElementById('chat-actions'));
      // 【逐行批注】 调用 status 执行页面交互或数据处理：}catch(error){status(error.message,true);}。
      }catch(error){status(error.message,true);}
      // 【逐行批注】 计算右侧结果并更新 finally{polling，改变页面或请求状态。
      finally{polling=false;}
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 调用 setInterval 执行页面交互或数据处理：setInterval(pollOnce,2000);。
    const refreshTimer = setInterval(() => { if (!watchedTask && !document.hidden) pollOnce(); }, 5000);
    window.addEventListener('pagehide', () => clearInterval(refreshTimer), {once:true});
    // 【逐行批注】 调用 pollOnce 执行页面交互或数据处理：if(state?.taskId)pollOnce();。
    if(state?.taskId)pollOnce();
    // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('pagehide',()=>{if(!state)return;localStorage.setItem('my-ai…。
    window.addEventListener('pagehide',()=>{if(!state)return;localStorage.setItem('my-ai-draft:'+sessionId(),input.value);fetch('/api/my-ai/chat',{method:'POST',keepalive:true,headers:{'Content-Type':'application/json'},body:JSON.stringify({id:sessionId(),version:state.version,draft:input.value,scroll:scroll.scrollTop})}).catch(()=>{});});status('多个聊天共享个人偏好和长期记忆，各自保留对话与草稿。');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 handlers，保存本行计算出的数据或对象。
  const handlers={overview,profile,preferences,memories,reports,plan,tasks,usage,settings,data:dataPage,chat};
  // 【逐行批注】 计算右侧结果并更新 (handlers[page]||overview)().then(()，改变页面或请求状态。
  (handlers[page]||overview)().then(()=>page==='usage'?libraryUsageView():null).catch(error=>status(error.message,true));
// 【逐行批注】 执行当前前端表达式：})();。
})();

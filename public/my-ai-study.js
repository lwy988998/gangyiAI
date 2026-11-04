/* 仅在实际课堂页面记录前台学习时间，多窗口由服务端租约去重。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 执行当前前端表达式：if(!['/learn','/practice','/summary'].includes(location.pathname))return;。
  if(!['/learn','/practice','/summary'].includes(location.pathname))return;
  // 【逐行批注】 声明前端变量 params，保存本行计算出的数据或对象。
  const params=new URLSearchParams(location.search),owner=crypto.randomUUID();let courseId=params.get('courseId')||'',lastInput=Date.now(),busy=false,wasActive=false;
  // 【逐行批注】 为页面事件绑定处理逻辑：for(const name of ['pointerdown','keydown','input','scroll'])document.addEventListen…。
  for(const name of ['pointerdown','keydown','input','scroll'])document.addEventListener(name,()=>{lastInput=Date.now();},{capture:true,passive:true});
  // 【逐行批注】 定义函数 resolveCourse，封装可复用的前端行为。
  async function resolveCourse(){if(courseId)return;const lessonId=params.get('lessonId');if(!lessonId)return;const response=await fetch('/api/learning-agent/lesson?lessonId='+encodeURIComponent(lessonId));if(response.ok){const lesson=await response.json();courseId=lesson.courseId||lesson.lesson?.courseId||'';}}
  // 【逐行批注】 定义函数 tick，封装可复用的前端行为。
  async function tick(final=false){if(!courseId||busy)return;busy=true;try{const day=new Date().toLocaleDateString('sv-SE'),active=!final&&!document.hidden&&document.hasFocus(),ending=wasActive&&!active;wasActive=active;await fetch('/api/my-ai/heartbeat',{method:'POST',keepalive:final,headers:{'Content-Type':'application/json'},body:JSON.stringify({owner,courseId,day,visible:active||ending,ending,recent:Date.now()-lastInput<=300000})});}catch(_){}finally{busy=false;}}
  // 【逐行批注】 为页面事件绑定处理逻辑：resolveCourse().then(()=>tick()).catch(()=>{});setInterval(()=>tick(),10000);documen…。
  resolveCourse().then(()=>tick()).catch(()=>{});setInterval(()=>tick(),10000);document.addEventListener('visibilitychange',()=>tick());window.addEventListener('blur',()=>tick());window.addEventListener('focus',()=>tick());window.addEventListener('pagehide',()=>tick(true));
// 【逐行批注】 执行当前前端表达式：})();。
})();

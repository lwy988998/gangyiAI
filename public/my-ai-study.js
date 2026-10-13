/* 仅在实际课堂页面记录前台学习时间，多窗口由服务端租约去重。 */
(() => {
  'use strict';
  if(!['/learn','/practice','/summary'].includes(location.pathname))return;
  const params=new URLSearchParams(location.search),owner=crypto.randomUUID();let courseId=params.get('courseId')||'',lastInput=Date.now(),busy=false,wasActive=false;
  for(const name of ['pointerdown','keydown','input','scroll'])document.addEventListener(name,()=>{lastInput=Date.now();},{capture:true,passive:true});
  async function resolveCourse(){if(courseId)return;const lessonId=params.get('lessonId');if(!lessonId)return;const response=await fetch('/api/learning-agent/lesson?lessonId='+encodeURIComponent(lessonId));if(response.ok){const lesson=await response.json();courseId=lesson.courseId||lesson.lesson?.courseId||'';}}
  async function tick(final=false){if(!courseId||busy)return;busy=true;try{const day=new Date().toLocaleDateString('sv-SE'),active=!final&&!document.hidden&&document.hasFocus(),ending=wasActive&&!active;wasActive=active;await fetch('/api/my-ai/heartbeat',{method:'POST',keepalive:final,headers:{'Content-Type':'application/json'},body:JSON.stringify({owner,courseId,day,visible:active||ending,ending,recent:Date.now()-lastInput<=300000})});}catch(_){}finally{busy=false;}}
  resolveCourse().then(()=>tick()).catch(()=>{});setInterval(()=>tick(),10000);document.addEventListener('visibilitychange',()=>tick());window.addEventListener('blur',()=>tick());window.addEventListener('focus',()=>tick());window.addEventListener('pagehide',()=>tick(true));
})();

/* 免费执行真实客户端，验证正文视区覆盖、流式增长、图片载入和刷新后的上报语义。 */
const assert=require('node:assert/strict'), fs=require('node:fs'), vm=require('node:vm'), path=require('node:path');
const sent=[],beacons=[],listeners={};
const context={window:{addEventListener:(name,fn)=>listeners[name]=fn},document:{hidden:false},
  fetch:async(url,options)=>{if(options)sent.push(JSON.parse(options.body));return {ok:true,json:async()=>({status:'running',events:[]})}},
  navigator:{sendBeacon:(_,blob)=>beacons.push(JSON.parse(blob.parts[0]))},Blob:class{constructor(parts){this.parts=parts}},
  setTimeout:()=>0,clearTimeout:()=>{},innerHeight:100,innerWidth:400,location:{protocol:'http:',host:'fixture'},WebSocket:class{addEventListener(){}close(){}},
  localStorage:{getItem:()=>null,setItem(){},removeItem(){}}};
vm.createContext(context);vm.runInContext(fs.readFileSync(path.join(__dirname,'../public/learning-agent.js'),'utf8'),context);
const api=context.window.GangyiAgent, viewport={getBoundingClientRect:()=>({top:0,bottom:100,left:0,right:400})};
const node={isConnected:true,top:0,height:200,images:[],closest:()=>null,querySelectorAll(){return this.images},
  getBoundingClientRect(){return {top:this.top,bottom:this.top+this.height,height:this.height,left:0,right:400}}};
const section={id:'demo',version:1,body:'完整示范正文'},message={taskId:'task',step:1,text:'完整教学消息',sequences:[1,3]};
const lastSection=()=>sent.flatMap(v=>v.sections || []).at(-1);
(async()=>{
  node.top=150;api.recordVisibleSection('lesson',section,node,viewport);await api.flushExposure();assert.equal(sent.length,0,'仅标题可见时不能记正文已展示');
  node.top=0;api.recordVisibleSection('lesson',section,node,viewport);await api.flushExposure();assert.equal(lastSection().complete,false);
  node.top=-100;api.recordVisibleSection('lesson',section,node,viewport);await api.flushExposure();assert.equal(lastSection().complete,true);
  const gap={...section,id:'gap'};node.height=300;node.top=0;api.recordVisibleSection('lesson',gap,node,viewport);
  node.top=-200;api.recordVisibleSection('lesson',gap,node,viewport);await api.flushExposure();assert.equal(lastSection().complete,false,'跳过中段不能算完整');
  node.top=-100;api.recordVisibleSection('lesson',gap,node,viewport);await api.flushExposure();assert.equal(lastSection().complete,true);
  node.height=200;node.top=0;api.recordVisibleMessage(message,node,viewport);await api.flushExposure();
  assert.deepEqual(sent.filter(v=>v.taskId==='task').at(-1).completeSequences,[]);
  node.top=-100;api.recordVisibleMessage(message,node,viewport);await api.flushExposure();
  assert.deepEqual(sent.filter(v=>v.taskId==='task').at(-1).completeSequences,[1,3]);
  node.top=0;node.height=50;const growing={...message,taskId:'growing',text:'首段',sequences:[7]};
  api.recordVisibleMessage(growing,node,viewport);await api.flushExposure();node.height=200;
  api.recordVisibleMessage({...growing,text:'首段加上尚未看完的长段',sequences:[7,9]},node,viewport);await api.flushExposure();
  assert.equal(sent.filter(v=>v.taskId==='growing').at(-1).completeSequences.includes(9),false,'追加正文不能沿用此前完整状态');
  const image={complete:false,naturalWidth:0};node.images=[image];node.top=0;
  api.recordVisibleSection('lesson',{...section,id:'image'},node,viewport);await api.flushExposure();assert.equal(lastSection().complete,false);
  image.complete=true;image.naturalWidth=100;node.top=-100;
  api.recordVisibleSection('lesson',{...section,id:'image'},node,viewport);await api.flushExposure();assert.equal(lastSection().complete,false,'占位区域不能当作已看过图片');
  node.top=0;api.recordVisibleSection('lesson',{...section,id:'image'},node,viewport);await api.flushExposure();assert.equal(lastSection().complete,true);
  node.images=[];node.height=50;api.recordVisibleMessage({...message,taskId:'beacon'},node,viewport);listeners.pagehide();
  assert.deepEqual(beacons.find(v=>v.taskId==='beacon').completeSequences,[1,3]);
  console.log('教学展示客户端回归通过：标题边缘、部分正文、完整覆盖、缺口、流式增长、图片和离开页面回执。');
})().catch(error=>{console.error(error);process.exitCode=1});

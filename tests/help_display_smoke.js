/* 免费执行真实客户端，验证正文视区覆盖、流式增长、图片载入和刷新后的上报语义。 */
// 【逐行批注】 声明前端变量 assert，保存本行计算出的数据或对象。
const assert=require('node:assert/strict'), fs=require('node:fs'), vm=require('node:vm'), path=require('node:path');
// 【逐行批注】 声明前端变量 sent，保存本行计算出的数据或对象。
const sent=[],beacons=[],listeners={};
// 【逐行批注】 声明前端变量 context，保存本行计算出的数据或对象。
const context={window:{addEventListener:(name,fn)=>listeners[name]=fn},document:{hidden:false},
  // 【逐行批注】 计算右侧结果并更新 fetch:async(url,options)，改变页面或请求状态。
  fetch:async(url,options)=>{if(options)sent.push(JSON.parse(options.body));return {ok:true,json:async()=>({status:'running',events:[]})}},
  // 【逐行批注】 计算右侧结果并更新 navigator:{sendBeacon:(_,blob)，改变页面或请求状态。
  navigator:{sendBeacon:(_,blob)=>beacons.push(JSON.parse(blob.parts[0]))},Blob:class{constructor(parts){this.parts=parts}},AbortController,
  // 【逐行批注】 为页面事件绑定处理逻辑：setTimeout:()=>0,clearTimeout:()=>{},innerHeight:100,innerWidth:400,location:{protoc…。
  setTimeout:()=>0,clearTimeout:()=>{},innerHeight:100,innerWidth:400,location:{protocol:'http:',host:'fixture'},WebSocket:class{addEventListener(){}close(){}},
  // 【逐行批注】 计算右侧结果并更新 localStorage:{getItem:()，改变页面或请求状态。
  localStorage:{getItem:()=>null,setItem(){},removeItem(){}}};
// 【逐行批注】 执行当前前端表达式：vm.createContext(context);vm.runInContext(fs.readFileSync(path.join(__dirname,'../public/…。
vm.createContext(context);vm.runInContext(fs.readFileSync(path.join(__dirname,'../public/learning-agent.js'),'utf8'),context);
// 【逐行批注】 声明前端变量 api，保存本行计算出的数据或对象。
const api=context.window.GangyiAgent, viewport={getBoundingClientRect:()=>({top:0,bottom:100,left:0,right:400})};
// 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
const node={isConnected:true,top:0,height:200,images:[],closest:()=>null,querySelectorAll(){return this.images},
  // 【逐行批注】 调用 getBoundingClientRect 执行页面交互或数据处理：getBoundingClientRect(){return {top:this.top,bottom:this.top+this.hei…。
  getBoundingClientRect(){return {top:this.top,bottom:this.top+this.height,height:this.height,left:0,right:400}}};
// 【逐行批注】 声明前端变量 section，保存本行计算出的数据或对象。
const section={id:'demo',version:1,body:'完整示范正文'},message={taskId:'task',step:1,text:'完整教学消息',sequences:[1,3]};
// 【逐行批注】 声明前端变量 lastSection，保存本行计算出的数据或对象。
const lastSection=()=>sent.flatMap(v=>v.sections || []).at(-1);
// 【逐行批注】 计算右侧结果并更新 (async()，改变页面或请求状态。
(async()=>{
  // 【逐行批注】 计算右侧结果并更新 node.top，改变页面或请求状态。
  node.top=150;api.recordVisibleSection('lesson',section,node,viewport);await api.flushExposure();assert.equal(sent.length,0,'仅标题可见时不能记正文已展示');
  // 【逐行批注】 计算右侧结果并更新 node.top，改变页面或请求状态。
  node.top=0;api.recordVisibleSection('lesson',section,node,viewport);await api.flushExposure();assert.equal(lastSection().complete,false);
  // 【逐行批注】 计算右侧结果并更新 node.top，改变页面或请求状态。
  node.top=-100;api.recordVisibleSection('lesson',section,node,viewport);await api.flushExposure();assert.equal(lastSection().complete,true);
  // 【逐行批注】 声明前端变量 gap，保存本行计算出的数据或对象。
  const gap={...section,id:'gap'};node.height=300;node.top=0;api.recordVisibleSection('lesson',gap,node,viewport);
  // 【逐行批注】 计算右侧结果并更新 node.top，改变页面或请求状态。
  node.top=-200;api.recordVisibleSection('lesson',gap,node,viewport);await api.flushExposure();assert.equal(lastSection().complete,false,'跳过中段不能算完整');
  // 【逐行批注】 计算右侧结果并更新 node.top，改变页面或请求状态。
  node.top=-100;api.recordVisibleSection('lesson',gap,node,viewport);await api.flushExposure();assert.equal(lastSection().complete,true);
  // 【逐行批注】 计算右侧结果并更新 node.height，改变页面或请求状态。
  node.height=200;node.top=0;api.recordVisibleMessage(message,node,viewport);await api.flushExposure();
  // 【逐行批注】 计算右侧结果并更新 assert.deepEqual(sent.filter(v，改变页面或请求状态。
  assert.deepEqual(sent.filter(v=>v.taskId==='task').at(-1).completeSequences,[]);
  // 【逐行批注】 计算右侧结果并更新 node.top，改变页面或请求状态。
  node.top=-100;api.recordVisibleMessage(message,node,viewport);await api.flushExposure();
  // 【逐行批注】 计算右侧结果并更新 assert.deepEqual(sent.filter(v，改变页面或请求状态。
  assert.deepEqual(sent.filter(v=>v.taskId==='task').at(-1).completeSequences,[1,3]);
  // 【逐行批注】 计算右侧结果并更新 node.top，改变页面或请求状态。
  node.top=0;node.height=50;const growing={...message,taskId:'growing',text:'首段',sequences:[7]};
  // 【逐行批注】 计算右侧结果并更新 api.recordVisibleMessage(growing,node,viewpo…，改变页面或请求状态。
  api.recordVisibleMessage(growing,node,viewport);await api.flushExposure();node.height=200;
  // 【逐行批注】 执行当前前端表达式：api.recordVisibleMessage({...growing,text:'首段加上尚未看完的长段',sequences:[7,9]},node,viewport);a…。
  api.recordVisibleMessage({...growing,text:'首段加上尚未看完的长段',sequences:[7,9]},node,viewport);await api.flushExposure();
  // 【逐行批注】 计算右侧结果并更新 assert.equal(sent.filter(v，改变页面或请求状态。
  assert.equal(sent.filter(v=>v.taskId==='growing').at(-1).completeSequences.includes(9),false,'追加正文不能沿用此前完整状态');
  // 【逐行批注】 声明前端变量 image，保存本行计算出的数据或对象。
  const image={complete:false,naturalWidth:0};node.images=[image];node.top=0;
  // 【逐行批注】 调用 lastSection 执行页面交互或数据处理：api.recordVisibleSection('lesson',{...section,id:'image'},node,viewpo…。
  api.recordVisibleSection('lesson',{...section,id:'image'},node,viewport);await api.flushExposure();assert.equal(lastSection().complete,false);
  // 【逐行批注】 计算右侧结果并更新 image.complete，改变页面或请求状态。
  image.complete=true;image.naturalWidth=100;node.top=-100;
  // 【逐行批注】 调用 lastSection 执行页面交互或数据处理：api.recordVisibleSection('lesson',{...section,id:'image'},node,viewpo…。
  api.recordVisibleSection('lesson',{...section,id:'image'},node,viewport);await api.flushExposure();assert.equal(lastSection().complete,false,'占位区域不能当作已看过图片');
  // 【逐行批注】 计算右侧结果并更新 node.top，改变页面或请求状态。
  node.top=0;api.recordVisibleSection('lesson',{...section,id:'image'},node,viewport);await api.flushExposure();assert.equal(lastSection().complete,true);
  // 【逐行批注】 计算右侧结果并更新 node.images，改变页面或请求状态。
  node.images=[];node.height=50;api.recordVisibleMessage({...message,taskId:'beacon'},node,viewport);listeners.pagehide();
  // 【逐行批注】 计算右侧结果并更新 assert.deepEqual(beacons.find(v，改变页面或请求状态。
  assert.deepEqual(beacons.find(v=>v.taskId==='beacon').completeSequences,[1,3]);
  // 【逐行批注】 执行当前前端表达式：console.log('教学展示客户端回归通过：标题边缘、部分正文、完整覆盖、缺口、流式增长、图片和离开页面回执。');。
  console.log('教学展示客户端回归通过：标题边缘、部分正文、完整覆盖、缺口、流式增长、图片和离开页面回执。');
// 【逐行批注】 计算右侧结果并更新 })().catch(error，改变页面或请求状态。
})().catch(error=>{console.error(error);process.exitCode=1});

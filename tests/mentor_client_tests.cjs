/* 验证成功回答后的迟到错误不会覆盖正文，活动连接的错误仍正常提示。 */
// 【逐行批注】 声明前端变量 assert，保存本行计算出的数据或对象。
const assert = require('node:assert/strict');
// 【逐行批注】 声明前端变量 fs，保存本行计算出的数据或对象。
const fs = require('node:fs');
// 【逐行批注】 声明前端变量 path，保存本行计算出的数据或对象。
const path = require('node:path');
// 【逐行批注】 声明前端变量 vm，保存本行计算出的数据或对象。
const vm = require('node:vm');
// 【逐行批注】 声明前端变量 source，保存本行计算出的数据或对象。
const source = fs.readFileSync(path.join(__dirname, '../public/ask.js'), 'utf8');
// 【逐行批注】 声明前端变量 drain，保存本行计算出的数据或对象。
const drain = () => new Promise(resolve => setImmediate(resolve));

// 【逐行批注】 定义函数 fixture，封装可复用的前端行为。
async function fixture() {
  // 【逐行批注】 声明前端变量 nodes，保存本行计算出的数据或对象。
  const nodes = new Map();
  // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
  const node = id => {
    // 【逐行批注】 判断条件 (!nodes.has(id)) nodes.set(id, {value:'', listeners:{}, children…，成立时执行对应逻辑。
    if (!nodes.has(id)) nodes.set(id, {value:'', listeners:{}, children:[],
      // 【逐行批注】 调用 append、before、remove、replaceChildren 执行页面交互或数据处理：append(...children) { this.children.push(...children); }, before(){},…。
      append(...children) { this.children.push(...children); }, before(){}, remove(){}, replaceChildren(){}, focus(){},
      // 【逐行批注】 为页面事件绑定处理逻辑：addEventListener(name, callback) { this.listeners[name] = callback; },。
      addEventListener(name, callback) { this.listeners[name] = callback; },
      // 【逐行批注】 调用 querySelector、node 执行页面交互或数据处理：querySelector(name) { return node(name); }});。
      querySelector(name) { return node(name); }});
    // 【逐行批注】 结束当前函数并返回结果：return nodes.get(id);。
    return nodes.get(id);
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 result，保存本行计算出的数据或对象。
  const result = {failed:0,completed:0,sockets:[]};
  // 【逐行批注】 执行当前前端表达式：class Socket {。
  class Socket {
    // 【逐行批注】 计算右侧结果并更新 OPEN，改变页面或请求状态。
    static OPEN = 1; static CONNECTING = 0;
    // 【逐行批注】 计算右侧结果并更新 constructor(){this.readyState，改变页面或请求状态。
    constructor(){this.readyState=1;result.sockets.push(this);}
    // 【逐行批注】 计算右侧结果并更新 close(){this.closed，改变页面或请求状态。
    close(){this.closed=true;}
    // 【逐行批注】 调用 send 执行页面交互或数据处理：send(){}。
    send(){}
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 window，保存本行计算出的数据或对象。
  const window = {GangyiChat:{renderMarkdown:x=>x,
    // 【逐行批注】 计算右侧结果并更新 createAssistantView:()，改变页面或请求状态。
    createAssistantView:()=>({push(){},text:()=> '真实完成的回答',bubble:node('bubble'),
      // 【逐行批注】 调用 complete、callback、fail 执行页面交互或数据处理：complete(callback){result.completed++;callback();},fail(){result.fail…。
      complete(callback){result.completed++;callback();},fail(){result.failed++;},
      // 【逐行批注】 计算右侧结果并更新 addActions:()，改变页面或请求状态。
      addActions:()=>node('actions')}),copyText:async()=>{}}};
  // 【逐行批注】 计算右侧结果并更新 vm.runInNewContext(source, {window,document:…，改变页面或请求状态。
  vm.runInNewContext(source, {window,document:{getElementById:node,createElement:()=>node(Math.random())},
    // 【逐行批注】 计算右侧结果并更新 WebSocket:Socket,fetch:async()，改变页面或请求状态。
    WebSocket:Socket,fetch:async()=>({ok:true,json:async()=>({messages:[],suggestions:[]})}),
    // 【逐行批注】 调用 replaceState 执行页面交互或数据处理：location:{protocol:'http:',host:'fixture.invalid'},history:{replaceSt…。
    location:{protocol:'http:',host:'fixture.invalid'},history:{replaceState(){}},
    // 【逐行批注】 计算右侧结果并更新 setTimeout(){},confirm:()，改变页面或请求状态。
    setTimeout(){},confirm:()=>true});
  // 【逐行批注】 调用 drain 执行页面交互或数据处理：await drain();。
  await drain();
  // 【逐行批注】 计算右侧结果并更新 node('ask-question').value，改变页面或请求状态。
  node('ask-question').value='实际提问';
  // 【逐行批注】 调用 node、preventDefault 执行页面交互或数据处理：node('ask-form').listeners.submit({preventDefault(){}});。
  node('ask-form').listeners.submit({preventDefault(){}});
  // 【逐行批注】 结束当前函数并返回结果：return {node,result,socket:result.sockets[0]};。
  return {node,result,socket:result.sockets[0]};
// 【逐行批注】 结束当前前端代码块。
}

// 【逐行批注】 定义函数 main，封装可复用的前端行为。
async function main() {
  // 【逐行批注】 声明前端变量 successful，保存本行计算出的数据或对象。
  const successful = await fixture();
  // 【逐行批注】 执行当前前端表达式：assert.ok(successful.node('ask-form').children.includes(successful.node('ask-stop')), '停止…。
  assert.ok(successful.node('ask-form').children.includes(successful.node('ask-stop')), '停止按钮应与输入框相邻');
  // 【逐行批注】 执行当前前端表达式：successful.socket.onmessage({data:JSON.stringify({type:'done',cancelled:false})});。
  successful.socket.onmessage({data:JSON.stringify({type:'done',cancelled:false})});
  // 【逐行批注】 执行当前前端表达式：successful.socket.onerror();。
  successful.socket.onerror();
  // 【逐行批注】 执行当前前端表达式：successful.socket.onclose();。
  successful.socket.onclose();
  // 【逐行批注】 执行当前前端表达式：assert.equal(successful.result.completed,1);。
  assert.equal(successful.result.completed,1);
  // 【逐行批注】 执行当前前端表达式：assert.equal(successful.result.failed,0,'成功回答不能被关闭后的迟到错误标成失败');。
  assert.equal(successful.result.failed,0,'成功回答不能被关闭后的迟到错误标成失败');
  // 【逐行批注】 执行当前前端表达式：assert.equal(successful.node('ask-submit').disabled,false,'完成后恢复发送入口');。
  assert.equal(successful.node('ask-submit').disabled,false,'完成后恢复发送入口');
  // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
  const active = await fixture();
  // 【逐行批注】 执行当前前端表达式：active.socket.onerror();active.socket.onclose();。
  active.socket.onerror();active.socket.onclose();
  // 【逐行批注】 执行当前前端表达式：assert.equal(active.result.failed,1,'真实中断仍需要提示，并且只提示一次');。
  assert.equal(active.result.failed,1,'真实中断仍需要提示，并且只提示一次');
  // 【逐行批注】 执行当前前端表达式：assert.equal(active.node('ask-submit').disabled,false,'真实中断后仍可再次发送');。
  assert.equal(active.node('ask-submit').disabled,false,'真实中断后仍可再次发送');
  // 【逐行批注】 执行当前前端表达式：console.log('导师客户端：成功后的迟到错误、活动故障及停止入口验证通过。');。
  console.log('导师客户端：成功后的迟到错误、活动故障及停止入口验证通过。');
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 计算右侧结果并更新 main().catch(error，改变页面或请求状态。
main().catch(error=>{console.error(error);process.exitCode=1;});

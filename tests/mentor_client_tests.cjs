/* 验证成功回答后的迟到错误不会覆盖正文，活动连接的错误仍正常提示。 */
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const source = fs.readFileSync(path.join(__dirname, '../public/ask.js'), 'utf8');
const drain = () => new Promise(resolve => setImmediate(resolve));

async function fixture() {
  const nodes = new Map();
  const node = id => {
    if (!nodes.has(id)) nodes.set(id, {value:'', listeners:{}, children:[],
      append(...children) { this.children.push(...children); }, before(){}, remove(){}, replaceChildren(){}, focus(){},
      addEventListener(name, callback) { this.listeners[name] = callback; },
      querySelector(name) { return node(name); }});
    return nodes.get(id);
  };
  const result = {failed:0,completed:0,sockets:[]};
  class Socket {
    static OPEN = 1; static CONNECTING = 0;
    constructor(){this.readyState=1;result.sockets.push(this);}
    close(){this.closed=true;}
    send(){}
  }
  const window = {GangyiChat:{renderMarkdown:x=>x,
    createAssistantView:()=>({push(){},text:()=> '真实完成的回答',bubble:node('bubble'),
      complete(callback){result.completed++;callback();},fail(){result.failed++;},
      addActions:()=>node('actions')}),copyText:async()=>{}}};
  vm.runInNewContext(source, {window,document:{getElementById:node,createElement:()=>node(Math.random())},
    WebSocket:Socket,fetch:async()=>({ok:true,json:async()=>({messages:[],suggestions:[]})}),
    location:{protocol:'http:',host:'fixture.invalid'},history:{replaceState(){}},
    setTimeout(){},confirm:()=>true});
  await drain();
  node('ask-question').value='实际提问';
  node('ask-form').listeners.submit({preventDefault(){}});
  return {node,result,socket:result.sockets[0]};
}

async function main() {
  const successful = await fixture();
  assert.ok(successful.node('ask-form').children.includes(successful.node('ask-stop')), '停止按钮应与输入框相邻');
  successful.socket.onmessage({data:JSON.stringify({type:'done',cancelled:false})});
  successful.socket.onerror();
  successful.socket.onclose();
  assert.equal(successful.result.completed,1);
  assert.equal(successful.result.failed,0,'成功回答不能被关闭后的迟到错误标成失败');
  assert.equal(successful.node('ask-submit').disabled,false,'完成后恢复发送入口');
  const active = await fixture();
  active.socket.onerror();active.socket.onclose();
  assert.equal(active.result.failed,1,'真实中断仍需要提示，并且只提示一次');
  assert.equal(active.node('ask-submit').disabled,false,'真实中断后仍可再次发送');
  console.log('导师客户端：成功后的迟到错误、活动故障及停止入口验证通过。');
}
main().catch(error=>{console.error(error);process.exitCode=1;});

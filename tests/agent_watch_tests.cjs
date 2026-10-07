/* 模拟迟到的真实请求时序，取消订阅后不得更新界面、记录曝光或启动重试。 */
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const vm=require('node:vm');
const source=fs.readFileSync(path.join(__dirname,'../public/learning-agent.js'),'utf8');
const drain=()=>new Promise(resolve=>setImmediate(resolve));
function fixture(){
  let resolve,reject;const pending=new Promise((yes,no)=>{resolve=yes;reject=no});
  const result={states:0,events:0,errors:0,requests:0,timers:0,sockets:[]};
  class Socket{constructor(){this.listeners={};result.sockets.push(this)}addEventListener(name,callback){this.listeners[name]=callback}send(){}close(){this.closed=true}}
  const window={addEventListener(){}};
  vm.runInNewContext(source,{window,fetch:()=>{result.requests++;return pending},WebSocket:Socket,
    setTimeout:()=>++result.timers,clearTimeout(){},location:{protocol:'http:',host:'fixture.invalid'}});
  const close=window.GangyiAgent.watch('fixture-task',{onState(){result.states++},onEvent(){result.events++;return true},onConnectionError(){result.errors++}});
  return {resolve,reject,result,close};
}
async function main(){
  {
    const f=fixture();f.close();
    f.resolve({ok:true,json:async()=>({status:'running',events:[{seq:1,type:'delta',field:'message',text:'迟到的旧文字'}]})});
    await drain();await drain();
    assert.equal(f.result.states,0,'取消后迟到状态不能覆盖恢复后的状态');
    assert.equal(f.result.events,0,'取消后迟到文字不再呈现');
    assert.equal(f.result.requests,1,'未呈现文字不记录曝光');
    assert.equal(f.result.sockets.length,0,'取消后不新建连接');
  }
  {
    const f=fixture();f.close();f.reject(new Error('迟到的连接错误'));
    await drain();await drain();
    assert.equal(f.result.errors,0,'取消后旧错误不能覆盖当前课堂');
    assert.equal(f.result.timers,0,'取消后不安排重试轮询');
  }
  {
    const f=fixture();f.resolve({ok:true,json:async()=>({status:'running',events:[]})});
    await drain();const socket=f.result.sockets[0];assert.ok(socket);
    f.close();socket.listeners.message({data:JSON.stringify({seq:1,type:'delta',field:'message',text:'已取消连接的旧事件'})});
    socket.listeners.message({data:'无效的旧事件'});await drain();
    assert.equal(f.result.states,1,'活动订阅仍正常报告初始状态');
    assert.equal(f.result.events,0,'取消后的连接事件不再投递');
    assert.equal(f.result.errors,0,'已取消连接的异常不再投递');
  }
  {
    const f=fixture();f.resolve({ok:true,json:async()=>({status:'waiting_student',events:[{seq:1,type:'delta',field:'message',text:'完整反馈'}]})});
    await drain();await drain();
    assert.equal(f.result.states,1,'有效完成状态仍正常报告');
    assert.equal(f.result.events,1,'最后的有效反馈不会被取消保护丢弃');
    assert.equal(f.result.requests,2,'实际展示的有效文字仍保存曝光');
    f.close();
  }
  {
    const f=fixture();f.reject(new Error('活动连接错误'));await drain();await drain();
    assert.equal(f.result.errors,1,'活动订阅的真实故障仍上报');
    assert.equal(f.result.timers,1,'活动订阅的故障仍安排恢复轮询');
    f.close();
  }
  console.log('订阅取消时序：迟到状态、文字、错误、曝光和连接事件均被隔离。');
}
main().catch(error=>{console.error(error);process.exitCode=1});

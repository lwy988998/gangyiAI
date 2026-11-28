/* 不启动浏览器或模型，验证资料列表和任务始终使用当前课程选择。 */
// 【逐行批注】 声明前端变量 assert，保存本行计算出的数据或对象。
const assert = require('node:assert/strict');
// 【逐行批注】 声明前端变量 fs，保存本行计算出的数据或对象。
const fs = require('node:fs');
// 【逐行批注】 声明前端变量 path，保存本行计算出的数据或对象。
const path = require('node:path');
// 【逐行批注】 声明前端变量 vm，保存本行计算出的数据或对象。
const vm = require('node:vm');
// 【逐行批注】 声明前端变量 source，保存本行计算出的数据或对象。
const source = fs.readFileSync(path.join(__dirname, '../public/library.js'), 'utf8');
// 【逐行批注】 声明前端变量 drain，保存本行计算出的数据或对象。
const drain = () => new Promise(resolve => setImmediate(resolve));

// 【逐行批注】 定义函数 fixture，封装可复用的前端行为。
async function fixture(search) {
  // 【逐行批注】 声明前端变量 nodes，保存本行计算出的数据或对象。
  const nodes = new Map(), calls = [], storage = new Map();
  // 【逐行批注】 声明前端变量 courseControlAvailable，保存本行计算出的数据或对象。
  let courseControlAvailable = true;
  // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
  const node = id => {
    // 【逐行批注】 判断条件 (!nodes.has(id)) nodes.set(id, {value:'', innerHTML:'', isConnec…，成立时执行对应逻辑。
    if (!nodes.has(id)) nodes.set(id, {value:'', innerHTML:'', isConnected:true,
      // 【逐行批注】 计算右侧结果并更新 classList:{toggle(){}}, querySelectorAll:()，改变页面或请求状态。
      classList:{toggle(){}}, querySelectorAll:()=>[], querySelector:key=>node(id+':'+key)});
    // 【逐行批注】 结束当前函数并返回结果：return nodes.get(id);。
    return nodes.get(id);
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 store，保存本行计算出的数据或对象。
  const store = {getItem:key=>storage.get(key)??null, setItem:(key,value)=>storage.set(key,value)};
  // 【逐行批注】 声明前端变量 sequence，保存本行计算出的数据或对象。
  let sequence = 0;
  // 【逐行批注】 执行当前前端表达式：vm.runInNewContext(source, {。
  vm.runInNewContext(source, {
    // 【逐行批注】 计算右侧结果并更新 document:{body:{dataset:{myAiPage:'library'}…，改变页面或请求状态。
    document:{body:{dataset:{myAiPage:'library'}}, getElementById:id=>id==='library-course'&&!courseControlAvailable?null:node(id), hidden:false},
    // 【逐行批注】 执行当前前端表达式：location:{search, hash:'', href:''}, URLSearchParams,。
    location:{search, hash:'', href:''}, URLSearchParams,
    // 【逐行批注】 计算右侧结果并更新 localStorage:store, sessionStorage:{getItem:…，改变页面或请求状态。
    localStorage:store, sessionStorage:{getItem:()=>null}, crypto:{randomUUID:()=>String(++sequence)},
    // 【逐行批注】 调用 setInterval、setTimeout、callback、clearTimeout 执行页面交互或数据处理：setInterval(){}, setTimeout(callback){callback();return 1;}, clearTim…。
    setInterval(){}, setTimeout(callback){callback();return 1;}, clearTimeout(){},
    // 【逐行批注】 计算右侧结果并更新 fetch:async (url, options，改变页面或请求状态。
    fetch:async (url, options={}) => {
      // 【逐行批注】 声明前端变量 request，保存本行计算出的数据或对象。
      const request = {url:new URL(url, 'https://fixture.invalid'), body:options.body?JSON.parse(options.body):null};
      // 【逐行批注】 执行当前前端表达式：calls.push(request);。
      calls.push(request);
      // 【逐行批注】 声明前端变量 endpoint，保存本行计算出的数据或对象。
      const endpoint=request.url.pathname;
      // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
      let value;
      // 【逐行批注】 判断条件 (endpoint==='/api/my-ai/overview') value={courses:[{id:'course-a…，成立时执行对应逻辑。
      if (endpoint==='/api/my-ai/overview') value={courses:[{id:'course-a',title:'数学'},{id:'course-b',title:'编程'}]};
      // 【逐行批注】 当前一条件不成立时继续判断：(endpoint==='/api/my-ai/library/research') value={id:'task-'+seq…。
      else if (endpoint==='/api/my-ai/library/research') value={id:'task-'+sequence};
      // 【逐行批注】 当前一条件不成立时继续判断：(endpoint==='/api/learning-agent') value={id:'task-'+sequence,st…。
      else if (endpoint==='/api/learning-agent') value={id:'task-'+sequence,status:'waiting_student'};
      // 【逐行批注】 当前一条件不成立时继续判断：(endpoint==='/api/my-ai/library' && request.body) value={id:'man…。
      else if (endpoint==='/api/my-ai/library' && request.body) value={id:'manual-source'};
      // 【逐行批注】 当前一条件不成立时继续判断：(endpoint==='/api/my-ai/library') value={items:[],usage:{searchR…。
      else if (endpoint==='/api/my-ai/library') value={items:[],usage:{searchRequests:0,fetchRequests:0,cacheHits:0,failures:0}};
      // 【逐行批注】 执行前一条件不成立时的分支。
      else throw new Error('未预期的接口：'+endpoint);
      // 【逐行批注】 结束当前函数并返回结果：return {ok:true,json:async()=>value};。
      return {ok:true,json:async()=>value};
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 调用 drain 执行页面交互或数据处理：await drain();。
  await drain();
  // 【逐行批注】 结束当前函数并返回结果：return {node,calls,setCourseControlAvailable:value=>{courseControlAva…。
  return {node,calls,setCourseControlAvailable:value=>{courseControlAvailable=value;}};
// 【逐行批注】 结束当前前端代码块。
}

// 【逐行批注】 计算右侧结果并更新 (async()，改变页面或请求状态。
(async()=>{
  // 【逐行批注】 计算右侧结果并更新 {node,calls,setCourseControlAvailable}，改变页面或请求状态。
  const {node,calls,setCourseControlAvailable}=await fixture('?courseId=course-a');
  // 【逐行批注】 声明前端变量 listCalls，保存本行计算出的数据或对象。
  const listCalls=()=>calls.filter(c=>c.url.pathname==='/api/my-ai/library'&&!c.body);
  // 【逐行批注】 声明前端变量 latestList，保存本行计算出的数据或对象。
  const latestList=()=>listCalls().at(-1).url.searchParams;
  // 【逐行批注】 调用 latestList 执行页面交互或数据处理：assert.equal(latestList().get('courseId'),'course-a','初始列表使用 URL 中的课程…。
  assert.equal(latestList().get('courseId'),'course-a','初始列表使用 URL 中的课程');
  // 【逐行批注】 计算右侧结果并更新 assert.equal(calls.filter(c，改变页面或请求状态。
  assert.equal(calls.filter(c=>c.body).length,0,'初始化只读已保存资料');

  // 【逐行批注】 计算右侧结果并更新 node('library-course').value，改变页面或请求状态。
  node('library-course').value='course-b'; await node('library-course').onchange();
  // 【逐行批注】 调用 latestList 执行页面交互或数据处理：assert.equal(latestList().get('courseId'),'course-b','切换课程后列表使用新选择');。
  assert.equal(latestList().get('courseId'),'course-b','切换课程后列表使用新选择');
  // 【逐行批注】 计算右侧结果并更新 node('library-course').value，改变页面或请求状态。
  node('library-course').value=''; await node('library-course').onchange();
  // 【逐行批注】 调用 latestList 执行页面交互或数据处理：assert.equal(latestList().get('courseId'),'','清空选择后列表不能回退到旧课程');。
  assert.equal(latestList().get('courseId'),'','清空选择后列表不能回退到旧课程');

  // 【逐行批注】 计算右侧结果并更新 node('library-search').value，改变页面或请求状态。
  node('library-search').value='线性代数'; node('library-search').oninput(); await drain();
  // 【逐行批注】 调用 latestList 执行页面交互或数据处理：assert.equal(latestList().get('courseId'),'','检索保持个人资料范围');。
  assert.equal(latestList().get('courseId'),'','检索保持个人资料范围');
  // 【逐行批注】 调用 latestList 执行页面交互或数据处理：assert.equal(latestList().get('search'),'线性代数','检索词进入列表请求');。
  assert.equal(latestList().get('search'),'线性代数','检索词进入列表请求');

  // 【逐行批注】 计算右侧结果并更新 node('library-goal').value，改变页面或请求状态。
  node('library-goal').value='了解适合我基础的资料';
  // 【逐行批注】 调用 node、preventDefault 执行页面交互或数据处理：await node('library-form').onsubmit({preventDefault(){},currentTarget…。
  await node('library-form').onsubmit({preventDefault(){},currentTarget:node('library-form')});
  // 【逐行批注】 计算右侧结果并更新 assert.equal(calls.find(c，改变页面或请求状态。
  assert.equal(calls.find(c=>c.url.pathname==='/api/my-ai/library/research').body.courseId,'','新检索任务遵守当前选择');
  // 【逐行批注】 计算右侧结果并更新 node('library-url').value，改变页面或请求状态。
  node('library-url').value='https://example.org/document';
  // 【逐行批注】 调用 node、preventDefault 执行页面交互或数据处理：await node('library-url-form').onsubmit({preventDefault(){}});。
  await node('library-url-form').onsubmit({preventDefault(){}});
  // 【逐行批注】 计算右侧结果并更新 assert.equal(calls.filter(c，改变页面或请求状态。
  assert.equal(calls.filter(c=>c.url.pathname==='/api/my-ai/library'&&c.body).at(-1).body.courseId,'','手动添加遵守当前选择');

  // 【逐行批注】 调用 setCourseControlAvailable 执行页面交互或数据处理：setCourseControlAvailable(false);。
  setCourseControlAvailable(false);
  // 【逐行批注】 调用 node 执行页面交互或数据处理：await node('library-filter').onchange();。
  await node('library-filter').onchange();
  // 【逐行批注】 调用 latestList 执行页面交互或数据处理：assert.equal(latestList().get('courseId'),'course-a','课程控件不存在时回退到 URL…。
  assert.equal(latestList().get('courseId'),'course-a','课程控件不存在时回退到 URL');
  // 【逐行批注】 调用 node、preventDefault 执行页面交互或数据处理：await node('library-form').onsubmit({preventDefault(){},currentTarget…。
  await node('library-form').onsubmit({preventDefault(){},currentTarget:node('library-form')});
  // 【逐行批注】 计算右侧结果并更新 assert.equal(calls.filter(c，改变页面或请求状态。
  assert.equal(calls.filter(c=>c.url.pathname==='/api/my-ai/library/research').at(-1).body.courseId,'course-a','缺少课程控件时检索任务回退到 URL');
  // 【逐行批注】 调用 node、preventDefault 执行页面交互或数据处理：await node('library-url-form').onsubmit({preventDefault(){}});。
  await node('library-url-form').onsubmit({preventDefault(){}});
  // 【逐行批注】 计算右侧结果并更新 assert.equal(calls.filter(c，改变页面或请求状态。
  assert.equal(calls.filter(c=>c.url.pathname==='/api/my-ai/library'&&c.body).at(-1).body.courseId,'course-a','缺少课程控件时手动添加回退到 URL');
  // 【逐行批注】 声明前端变量 personal，保存本行计算出的数据或对象。
  const personal=await fixture('');
  // 【逐行批注】 计算右侧结果并更新 assert.equal(personal.calls.find(c，改变页面或请求状态。
  assert.equal(personal.calls.find(c=>c.url.pathname==='/api/my-ai/library').url.searchParams.get('courseId'),'','URL 没有课程时使用个人资料范围');
  // 【逐行批注】 执行当前前端表达式：personal.setCourseControlAvailable(false);。
  personal.setCourseControlAvailable(false);
  // 【逐行批注】 执行当前前端表达式：await personal.node('library-filter').onchange();。
  await personal.node('library-filter').onchange();
  // 【逐行批注】 计算右侧结果并更新 assert.equal(personal.calls.filter(c，改变页面或请求状态。
  assert.equal(personal.calls.filter(c=>c.url.pathname==='/api/my-ai/library'&&!c.body).at(-1).url.searchParams.get('courseId'),'','控件和URL都没有课程时列表使用个人范围');
  // 【逐行批注】 计算右侧结果并更新 personal.node('library-goal').value，改变页面或请求状态。
  personal.node('library-goal').value='个人资料';
  // 【逐行批注】 调用 preventDefault 执行页面交互或数据处理：await personal.node('library-form').onsubmit({preventDefault(){},curr…。
  await personal.node('library-form').onsubmit({preventDefault(){},currentTarget:personal.node('library-form')});
  // 【逐行批注】 计算右侧结果并更新 assert.equal(personal.calls.filter(c，改变页面或请求状态。
  assert.equal(personal.calls.filter(c=>c.url.pathname==='/api/my-ai/library/research').at(-1).body.courseId,'','控件和URL都没有课程时任务使用个人范围');
  // 【逐行批注】 计算右侧结果并更新 personal.node('library-url').value，改变页面或请求状态。
  personal.node('library-url').value='https://example.org/personal';
  // 【逐行批注】 调用 preventDefault 执行页面交互或数据处理：await personal.node('library-url-form').onsubmit({preventDefault(){}}…。
  await personal.node('library-url-form').onsubmit({preventDefault(){}});
  // 【逐行批注】 计算右侧结果并更新 assert.equal(personal.calls.filter(c，改变页面或请求状态。
  assert.equal(personal.calls.filter(c=>c.url.pathname==='/api/my-ai/library'&&c.body).at(-1).body.courseId,'','控件和URL都没有课程时添加使用个人范围');
  // 【逐行批注】 执行当前前端表达式：console.log('资料课程初始关联、切换、清空、列表检索、任务、手动添加和缺控件回退检查通过');。
  console.log('资料课程初始关联、切换、清空、列表检索、任务、手动添加和缺控件回退检查通过');
// 【逐行批注】 计算右侧结果并更新 })().catch(error，改变页面或请求状态。
})().catch(error=>{console.error(error);process.exitCode=1;});

/* 不启动浏览器或模型，验证资料列表和任务始终使用当前课程选择。 */
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const source = fs.readFileSync(path.join(__dirname, '../public/library.js'), 'utf8');
const drain = () => new Promise(resolve => setImmediate(resolve));

async function fixture(search) {
  const nodes = new Map(), calls = [], storage = new Map();
  let courseControlAvailable = true;
  const node = id => {
    if (!nodes.has(id)) nodes.set(id, {value:'', innerHTML:'', isConnected:true,
      classList:{toggle(){}}, querySelectorAll:()=>[], querySelector:key=>node(id+':'+key)});
    return nodes.get(id);
  };
  const store = {getItem:key=>storage.get(key)??null, setItem:(key,value)=>storage.set(key,value)};
  let sequence = 0;
  vm.runInNewContext(source, {
    document:{body:{dataset:{myAiPage:'library'}}, getElementById:id=>id==='library-course'&&!courseControlAvailable?null:node(id), hidden:false},
    location:{search, hash:'', href:''}, URLSearchParams,
    localStorage:store, sessionStorage:{getItem:()=>null}, crypto:{randomUUID:()=>String(++sequence)},
    setInterval(){}, setTimeout(callback){callback();return 1;}, clearTimeout(){},
    fetch:async (url, options={}) => {
      const request = {url:new URL(url, 'https://fixture.invalid'), body:options.body?JSON.parse(options.body):null};
      calls.push(request);
      const endpoint=request.url.pathname;
      let value;
      if (endpoint==='/api/my-ai/overview') value={courses:[{id:'course-a',title:'数学'},{id:'course-b',title:'编程'}]};
      else if (endpoint==='/api/my-ai/library/research') value={id:'task-'+sequence};
      else if (endpoint==='/api/learning-agent') value={id:'task-'+sequence,status:'waiting_student'};
      else if (endpoint==='/api/my-ai/library' && request.body) value={id:'manual-source'};
      else if (endpoint==='/api/my-ai/library') value={items:[],usage:{searchRequests:0,fetchRequests:0,cacheHits:0,failures:0}};
      else throw new Error('未预期的接口：'+endpoint);
      return {ok:true,json:async()=>value};
    }
  });
  await drain();
  return {node,calls,setCourseControlAvailable:value=>{courseControlAvailable=value;}};
}

(async()=>{
  const {node,calls,setCourseControlAvailable}=await fixture('?courseId=course-a');
  const listCalls=()=>calls.filter(c=>c.url.pathname==='/api/my-ai/library'&&!c.body);
  const latestList=()=>listCalls().at(-1).url.searchParams;
  assert.equal(latestList().get('courseId'),'course-a','初始列表使用 URL 中的课程');
  assert.equal(calls.filter(c=>c.body).length,0,'初始化只读已保存资料');

  node('library-course').value='course-b'; await node('library-course').onchange();
  assert.equal(latestList().get('courseId'),'course-b','切换课程后列表使用新选择');
  node('library-course').value=''; await node('library-course').onchange();
  assert.equal(latestList().get('courseId'),'','清空选择后列表不能回退到旧课程');

  node('library-search').value='线性代数'; node('library-search').oninput(); await drain();
  assert.equal(latestList().get('courseId'),'','检索保持个人资料范围');
  assert.equal(latestList().get('search'),'线性代数','检索词进入列表请求');

  node('library-goal').value='了解适合我基础的资料';
  await node('library-form').onsubmit({preventDefault(){},currentTarget:node('library-form')});
  assert.equal(calls.find(c=>c.url.pathname==='/api/my-ai/library/research').body.courseId,'','新检索任务遵守当前选择');
  node('library-url').value='https://example.org/document';
  await node('library-url-form').onsubmit({preventDefault(){}});
  assert.equal(calls.filter(c=>c.url.pathname==='/api/my-ai/library'&&c.body).at(-1).body.courseId,'','手动添加遵守当前选择');

  setCourseControlAvailable(false);
  await node('library-filter').onchange();
  assert.equal(latestList().get('courseId'),'course-a','课程控件不存在时回退到 URL');
  const personal=await fixture('');
  assert.equal(personal.calls.find(c=>c.url.pathname==='/api/my-ai/library').url.searchParams.get('courseId'),'','URL 没有课程时使用个人资料范围');
  console.log('资料课程初始关联、切换、清空、列表检索、任务、手动添加和缺控件回退检查通过');
})().catch(error=>{console.error(error);process.exitCode=1;});

/* 使用旧客户端的真实缓存格式，验证迁移后不会重复带回已提交的草稿。 */
// 【逐行批注】 声明前端变量 assert，保存本行计算出的数据或对象。
const assert = require('node:assert/strict');
// 【逐行批注】 声明前端变量 fs，保存本行计算出的数据或对象。
const fs = require('node:fs');
// 【逐行批注】 声明前端变量 path，保存本行计算出的数据或对象。
const path = require('node:path');
// 【逐行批注】 声明前端变量 vm，保存本行计算出的数据或对象。
const vm = require('node:vm');
// 【逐行批注】 声明前端变量 source，保存本行计算出的数据或对象。
const source = fs.readFileSync(path.join(__dirname, '../public/agent-classroom.js'), 'utf8');
// 【逐行批注】 声明前端变量 declaration，保存本行计算出的数据或对象。
const declaration = source.slice(source.indexOf('  function readDraft('), source.indexOf('\n  function material('));

// 【逐行批注】 定义函数 fixture，封装可复用的前端行为。
function fixture(lessonId = 'legacy-session', query = new URLSearchParams()) {
  // 【逐行批注】 声明前端变量 cache，保存本行计算出的数据或对象。
  const cache = new Map();
  // 【逐行批注】 声明前端变量 api，保存本行计算出的数据或对象。
  const api = {storage:{get:key=>cache.get('gangyi-agent:'+key)||'',set:(key,value)=>cache.set('gangyi-agent:'+key,value)}};
  // 【逐行批注】 声明前端变量 localStorage，保存本行计算出的数据或对象。
  const localStorage = {getItem:key=>cache.has(key)?cache.get(key):null};
  // 【逐行批注】 声明前端变量 lesson，保存本行计算出的数据或对象。
  const lesson = {courseId:'original-course',phaseIndex:1,topicIndex:2};
  // 【逐行批注】 声明前端变量 reviewContext，保存本行计算出的数据或对象。
  const reviewContext = query.has('review') ? {day:Number(query.get('review')),reviewId:query.get('reviewId')||''} : {};
  // 【逐行批注】 声明前端变量 read，保存本行计算出的数据或对象。
  const read = vm.runInNewContext(declaration+'\nreadDraft', {api,localStorage,lessonId,query,lesson,reviewContext});
  // 【逐行批注】 声明前端变量 section，保存本行计算出的数据或对象。
  const section = {id:'original-practice',legacyKind:'practice',legacyIndex:0,questionId:'q-stable',version:1};
  // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
  const key = `draft:${lessonId}:${section.id}`;
  // 【逐行批注】 声明前端变量 oldKey，保存本行计算出的数据或对象。
  const oldKey = identity=>'gy:question-draft:'+JSON.stringify(['original-course',1,2,'','','','practice',identity,1]);
  // 【逐行批注】 结束当前函数并返回结果：return {cache,api,read,section,key,oldKey};。
  return {cache,api,read,section,key,oldKey};
// 【逐行批注】 结束当前前端代码块。
}

// 【逐行批注】 开始当前前端代码块。
{
  // 【逐行批注】 声明前端变量 f，保存本行计算出的数据或对象。
  const f=fixture();f.cache.set(f.oldKey('q-stable'),'尚未提交的旧思路');
  // 【逐行批注】 执行当前前端表达式：assert.equal(f.read(f.key,f.section),'尚未提交的旧思路');。
  assert.equal(f.read(f.key,f.section),'尚未提交的旧思路');
  // 【逐行批注】 执行当前前端表达式：assert.equal(f.api.storage.get(f.key),'尚未提交的旧思路');。
  assert.equal(f.api.storage.get(f.key),'尚未提交的旧思路');
  // 【逐行批注】 执行当前前端表达式：f.cache.delete('gangyi-agent:'+f.key);。
  f.cache.delete('gangyi-agent:'+f.key);
  // 【逐行批注】 执行当前前端表达式：assert.equal(f.read(f.key,f.section),'','提交后刷新不能再次带回旧草稿');。
  assert.equal(f.read(f.key,f.section),'','提交后刷新不能再次带回旧草稿');
  // 【逐行批注】 执行当前前端表达式：assert.equal(f.cache.get(f.oldKey('q-stable')),'尚未提交的旧思路','保留旧缓存供恢复，不依靠删除旧记录修复');。
  assert.equal(f.cache.get(f.oldKey('q-stable')),'尚未提交的旧思路','保留旧缓存供恢复，不依靠删除旧记录修复');
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 开始当前前端代码块。
{
  // 【逐行批注】 声明前端变量 f，保存本行计算出的数据或对象。
  const f=fixture();f.cache.set(f.oldKey(0),'早期客户端按题号保存的草稿');
  // 【逐行批注】 执行当前前端表达式：assert.equal(f.read(f.key,f.section),'早期客户端按题号保存的草稿','没有旧题目 ID 时兼容零号题索引');。
  assert.equal(f.read(f.key,f.section),'早期客户端按题号保存的草稿','没有旧题目 ID 时兼容零号题索引');
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 按循环规则遍历或重复处理：for(const current of ['新窗口的思路','']) {。
for(const current of ['新窗口的思路','']) {
  // 【逐行批注】 声明前端变量 f，保存本行计算出的数据或对象。
  const f=fixture();f.cache.set(f.oldKey('q-stable'),'更早的输入');f.api.storage.set(f.key,current);
  // 【逐行批注】 执行当前前端表达式：assert.equal(f.read(f.key,f.section),current,'新输入与明确清空都优先于旧草稿');。
  assert.equal(f.read(f.key,f.section),current,'新输入与明确清空都优先于旧草稿');
  // 【逐行批注】 执行当前前端表达式：f.cache.delete('gangyi-agent:'+f.key);。
  f.cache.delete('gangyi-agent:'+f.key);
  // 【逐行批注】 执行当前前端表达式：assert.equal(f.read(f.key,f.section),'','已有新缓存使用后不重复迁移');。
  assert.equal(f.read(f.key,f.section),'','已有新缓存使用后不重复迁移');
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 开始当前前端代码块。
{
  // 【逐行批注】 声明前端变量 f，保存本行计算出的数据或对象。
  const f=fixture('lesson-new');f.cache.set('gy:lesson-chat-draft:'+JSON.stringify({courseId:'original-course',phaseIndex:1,topicIndex:2}),'旧课时的追问');
  // 【逐行批注】 执行当前前端表达式：assert.equal(f.read('chat-draft:lesson-new'),'','新的 AI 课时不继承另一个旧课时的追问');。
  assert.equal(f.read('chat-draft:lesson-new'),'','新的 AI 课时不继承另一个旧课时的追问');
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 开始当前前端代码块。
{
  // 【逐行批注】 声明前端变量 f，保存本行计算出的数据或对象。
  const f=fixture('legacy-prepared-old-task',new URLSearchParams('lessonTaskId=old-task&review=0&reviewId=review-old'));
  // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
  const key='gy:question-draft:'+JSON.stringify(['original-course',1,2,'old-task','review-old',0,'practice','q-stable',1]);
  // 【逐行批注】 执行当前前端表达式：f.cache.set(key,'独立复习课的草稿');assert.equal(f.read(f.key,f.section),'独立复习课的草稿');。
  f.cache.set(key,'独立复习课的草稿');assert.equal(f.read(f.key,f.section),'独立复习课的草稿');
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 执行当前前端表达式：console.log('旧草稿迁移：首次恢复、提交后刷新、旧题号、新输入、清空与独立复习作用域均通过。');。
console.log('旧草稿迁移：首次恢复、提交后刷新、旧题号、新输入、清空与独立复习作用域均通过。');

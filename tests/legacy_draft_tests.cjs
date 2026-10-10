/* 使用旧客户端的真实缓存格式，验证迁移后不会重复带回已提交的草稿。 */
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const source = fs.readFileSync(path.join(__dirname, '../public/agent-classroom.js'), 'utf8');
const declaration = source.slice(source.indexOf('  function readDraft('), source.indexOf('\n  function material('));

function fixture(lessonId = 'legacy-session', query = new URLSearchParams()) {
  const cache = new Map();
  const api = {storage:{get:key=>cache.get('gangyi-agent:'+key)||'',set:(key,value)=>cache.set('gangyi-agent:'+key,value)}};
  const localStorage = {getItem:key=>cache.has(key)?cache.get(key):null};
  const lesson = {courseId:'original-course',phaseIndex:1,topicIndex:2};
  const reviewContext = query.has('review') ? {day:Number(query.get('review')),reviewId:query.get('reviewId')||''} : {};
  const read = vm.runInNewContext(declaration+'\nreadDraft', {api,localStorage,lessonId,query,lesson,reviewContext});
  const section = {id:'original-practice',legacyKind:'practice',legacyIndex:0,questionId:'q-stable',version:1};
  const key = `draft:${lessonId}:${section.id}`;
  const oldKey = identity=>'gy:question-draft:'+JSON.stringify(['original-course',1,2,'','','','practice',identity,1]);
  return {cache,api,read,section,key,oldKey};
}

{
  const f=fixture();f.cache.set(f.oldKey('q-stable'),'尚未提交的旧思路');
  assert.equal(f.read(f.key,f.section),'尚未提交的旧思路');
  assert.equal(f.api.storage.get(f.key),'尚未提交的旧思路');
  f.cache.delete('gangyi-agent:'+f.key);
  assert.equal(f.read(f.key,f.section),'','提交后刷新不能再次带回旧草稿');
  assert.equal(f.cache.get(f.oldKey('q-stable')),'尚未提交的旧思路','保留旧缓存供恢复，不依靠删除旧记录修复');
}
{
  const f=fixture();f.cache.set(f.oldKey(0),'早期客户端按题号保存的草稿');
  assert.equal(f.read(f.key,f.section),'早期客户端按题号保存的草稿','没有旧题目 ID 时兼容零号题索引');
}
for(const current of ['新窗口的思路','']) {
  const f=fixture();f.cache.set(f.oldKey('q-stable'),'更早的输入');f.api.storage.set(f.key,current);
  assert.equal(f.read(f.key,f.section),current,'新输入与明确清空都优先于旧草稿');
  f.cache.delete('gangyi-agent:'+f.key);
  assert.equal(f.read(f.key,f.section),'','已有新缓存使用后不重复迁移');
}
{
  const f=fixture('lesson-new');f.cache.set('gy:lesson-chat-draft:'+JSON.stringify({courseId:'original-course',phaseIndex:1,topicIndex:2}),'旧课时的追问');
  assert.equal(f.read('chat-draft:lesson-new'),'','新的 AI 课时不继承另一个旧课时的追问');
}
{
  const f=fixture('legacy-prepared-old-task',new URLSearchParams('lessonTaskId=old-task&review=0&reviewId=review-old'));
  const key='gy:question-draft:'+JSON.stringify(['original-course',1,2,'old-task','review-old',0,'practice','q-stable',1]);
  f.cache.set(key,'独立复习课的草稿');assert.equal(f.read(f.key,f.section),'独立复习课的草稿');
}
console.log('旧草稿迁移：首次恢复、提交后刷新、旧题号、新输入、清空与独立复习作用域均通过。');

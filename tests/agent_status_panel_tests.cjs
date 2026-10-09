/* 模拟任务更新与控制时序，验证折叠状态、真实操作和旧任务兼容。 */
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const drain = () => new Promise(resolve => setImmediate(resolve));
class Element {
  constructor(tag) { this.tagName = tag; this.children = []; this.attributes = {}; this.listeners = {}; this.open = false; this.hidden = false; this.disabled = false; this._text = ''; }
  set textContent(value) { this._text = String(value); this.children = []; }
  get textContent() { return this._text + this.children.map(child => child.textContent).join(''); }
  get firstChild() { return this.children[0]; }
  append(...nodes) { this.children.push(...nodes); }
  prepend(...nodes) { this.children.unshift(...nodes); }
  replaceChildren(...nodes) { this._text = ''; this.children = nodes; }
  setAttribute(key, value) { this.attributes[key] = value; }
  removeAttribute(key) { delete this.attributes[key]; }
  addEventListener(name, fn) { this.listeners[name] = fn; }
  querySelectorAll(tag) { return this.children.flatMap(child => [...(child.tagName === tag ? [child] : []), ...child.querySelectorAll(tag)]); }
  focus() { this.focused = true; }
  get dataset() { return this._dataset || (this._dataset = {}); }
}
async function main() {
  const body = new Element('body'), main = new Element('main'), intervals = [], controls = [], requests = [];
  const find = test => { function visit(node) { if (test(node)) return node; for (const child of node.children) { const value = visit(child); if (value) return value; } } return visit(body); };
  let task = { id: 'one', status: 'running', calls: 2, purpose: '准备下一课',
    target: { courseTitle: '化学课程', lessonTitle: '化合价与电子变化' },
    activity: { phase: 'tool', title: '校验并保存选择题', detail: '正在校验本次操作的数据。' },
    events: [{ seq: 3, type: 'action', at: '2026-10-08T08:10:00Z', activity: { title: '创建本次课时', detail: '课时已创建。' }, message: 'PRIVATE-DO-NOT-RENDER' }] };
  let defer = null, globalPaused = false;
  const snapshot = () => ({ paused: globalPaused, todayCalls: 5, tasks: [
    { ...task, taskId: task.id, source: 'AI 课堂', tracked: true },
    { id: 'picture', source: '目标图片', purpose: '识别图片并提取学习目标', status: globalPaused ? 'paused' : 'requesting', calls: 3, tracked: true }
  ] });
  const document = { body, hidden: false, createElement: tag => new Element(tag), getElementById: id => find(node => node.id === id),
    querySelector: selector => selector === 'main' ? main : null, querySelectorAll: () => [], addEventListener() {}, dispatchEvent() {} };
  const api = { storage: { get: () => null }, failureMessage: value => value.failure?.message || value.error || '',
    request: async (url, data) => { requests.push(url); if (url === '/api/courses') return {};
      if (url === '/api/ai-activity/control') { controls.push({ ...data, all: true }); globalPaused = data.command === 'pause'; task = { ...task, paused: globalPaused, status: globalPaused ? 'paused' : 'running' }; return snapshot(); }
      if (url === '/api/ai-activity') return snapshot();
      if (defer) { const pending = defer; defer = null; return pending; } return task; },
    control: async value => { controls.push(value); task = { ...task, paused: true, status: 'paused' }; return task; } };
  const window = { GangyiAgent: api, addEventListener() {} };
  vm.runInNewContext(fs.readFileSync(path.join(__dirname, '../public/agent-shell.js'), 'utf8'), {
    document, window, location: { pathname: '/learn', search: '?taskId=one' }, URLSearchParams,
    CustomEvent: class { constructor(name, value) { this.type = name; this.detail = value.detail; } },
    setInterval: fn => { intervals.push(fn); return 1; }, clearInterval() {}
  });
  await drain(); await drain();
  const byClass = name => find(node => (node.className || '').split(' ').includes(name));
  const navigation = document.getElementById('learning-navigation'), history = byClass('ai-control-history'), details = byClass('ai-task-details');
  assert.equal(history.open, false); assert.equal(details.open, false);
  assert.equal(byClass('ai-operation-title').textContent, '校验并保存选择题');
  assert.match(byClass('ai-operation-target').textContent, /化学课程.*化合价与电子变化/);
  assert.ok(!body.textContent.includes('PRIVATE-'), '操作面板不呈现工具输出和教师正文');
  assert.match(document.getElementById('ai-live-calls').textContent, /AI 课堂/);
  assert.match(document.getElementById('ai-live-calls').textContent, /目标图片/);
  assert.match(document.getElementById('ai-live-calls').textContent, /实际请求 3 次/);
  assert.equal(byClass('ai-call-history').open, false, '调用历史默认折叠');
  assert.match(byClass('ai-task-facts').textContent, /主控处理轮次2 次/);
  assert.ok(!byClass('ai-task-facts').textContent.includes('AI 请求'), '主控轮次不能冒充实际请求次数');
  history.open = true; details.open = true;
  const historyNode = byClass('ai-step-list').firstChild, button = find(node => node.tagName === 'button' && node.textContent === '暂停全部 AI');
  intervals[0](); await drain();
  assert.equal(history.open, true); assert.equal(details.open, true);
  assert.equal(byClass('ai-step-list').firstChild, historyNode, '轮询不重建未改变的步骤');
  assert.equal(find(node => node.tagName === 'button' && node.textContent === '暂停全部 AI'), button, '轮询保留控制按钮与键盘焦点');
  await button.onclick();
  assert.equal(controls.length, 1); assert.equal(controls[0].command, 'pause'); assert.equal(controls[0].all, true);
  assert.equal(navigation.firstChild.textContent, '全部 AI 已暂停');
  assert.equal(byClass('ai-operation-title').textContent, '已暂停：校验并保存选择题');
  task = { ...task, status: 'running', paused: true }; intervals[0](); await drain();
  assert.equal(navigation.firstChild.textContent, '全部 AI 已暂停', '暂停状态不能被处理中遮盖');
  await find(node => node.tagName === 'button' && node.textContent === '恢复全部 AI').onclick();
  assert.equal(controls[1].command, 'resume'); assert.equal(controls[1].all, true);
  task = { id: 'legacy', status: 'ready', calls: 3, events: [{ type: 'action', message: 'PRIVATE-OLD-MESSAGE' }] };
  intervals[0](); await drain();
  assert.match(byClass('ai-operation-detail').textContent, /历史任务未记录具体执行步骤/);
  assert.ok(!body.textContent.includes('PRIVATE-OLD-MESSAGE'));
  task = { id: 'failed', status: 'failed', failure: { operation: '校验并保存选择题', message: '题目选项未完整保存。', attempt: 3 }, events: [] };
  intervals[0](); await drain();
  assert.equal(byClass('ai-operation-title').textContent, '校验并保存选择题');
  assert.equal(byClass('ai-operation-detail').textContent, '题目选项未完整保存。');
  // 切换课时后，上一任务的迟到错误也不能覆盖当前面板。
  let reject;
  defer = new Promise((resolve, no) => { reject = no; });
  intervals[0](); await drain();
  task = { id: 'new', status: 'running', purpose: '回应你的交流', activity: { title: '接收 AI 的教学回复', detail: '正在接收公开内容。' }, events: [] };
  await window.GangyiNavigation.setContext({ taskId: 'new' }); await drain();
  reject(new Error('上一任务的迟到错误')); await drain();
  assert.equal(byClass('ai-operation-title').textContent, '接收 AI 的教学回复');
  assert.ok(!body.textContent.includes('迟到错误'));
  assert.ok(requests.some(url => url.endsWith('taskId=new')));
  console.log('AI 状态面板：全软件实时列表、实际次数、全局暂停恢复、折叠保持、历史兼容及迟到请求回归通过。');
}
main().catch(error => { console.error(error); process.exitCode = 1; });

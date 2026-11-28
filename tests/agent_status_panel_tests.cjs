/* 模拟任务更新与控制时序，验证折叠状态、真实操作和旧任务兼容。 */
// 【逐行批注】 声明前端变量 assert，保存本行计算出的数据或对象。
const assert = require('node:assert/strict');
// 【逐行批注】 声明前端变量 fs，保存本行计算出的数据或对象。
const fs = require('node:fs');
// 【逐行批注】 声明前端变量 path，保存本行计算出的数据或对象。
const path = require('node:path');
// 【逐行批注】 声明前端变量 vm，保存本行计算出的数据或对象。
const vm = require('node:vm');
// 【逐行批注】 声明前端变量 drain，保存本行计算出的数据或对象。
const drain = () => new Promise(resolve => setImmediate(resolve));
// 【逐行批注】 执行当前前端表达式：class Element {。
class Element {
  // 【逐行批注】 计算右侧结果并更新 constructor(tag) { this.tagName，改变页面或请求状态。
  constructor(tag) { this.tagName = tag; this.children = []; this.attributes = {}; this.listeners = {}; this.open = false; this.hidden = false; this.disabled = false; this._text = ''; }
  // 【逐行批注】 计算右侧结果并更新 set textContent(value) { this._text，改变页面或请求状态。
  set textContent(value) { this._text = String(value); this.children = []; }
  // 【逐行批注】 计算右侧结果并更新 get textContent() { return this._text + this…，改变页面或请求状态。
  get textContent() { return this._text + this.children.map(child => child.textContent).join(''); }
  // 【逐行批注】 调用 firstChild 执行页面交互或数据处理：get firstChild() { return this.children[0]; }。
  get firstChild() { return this.children[0]; }
  // 【逐行批注】 调用 append 执行页面交互或数据处理：append(...nodes) { this.children.push(...nodes); }。
  append(...nodes) { this.children.push(...nodes); }
  // 【逐行批注】 调用 prepend 执行页面交互或数据处理：prepend(...nodes) { this.children.unshift(...nodes); }。
  prepend(...nodes) { this.children.unshift(...nodes); }
  // 【逐行批注】 计算右侧结果并更新 replaceChildren(...nodes) { this._text，改变页面或请求状态。
  replaceChildren(...nodes) { this._text = ''; this.children = nodes; }
  // 【逐行批注】 计算右侧结果并更新 setAttribute(key, value) { this.attributes[k…，改变页面或请求状态。
  setAttribute(key, value) { this.attributes[key] = value; }
  // 【逐行批注】 调用 removeAttribute 执行页面交互或数据处理：removeAttribute(key) { delete this.attributes[key]; }。
  removeAttribute(key) { delete this.attributes[key]; }
  // 【逐行批注】 为页面事件绑定处理逻辑：addEventListener(name, fn) { this.listeners[name] = fn; }。
  addEventListener(name, fn) { this.listeners[name] = fn; }
  // 【逐行批注】 计算右侧结果并更新 querySelectorAll(tag) { return this.children…，改变页面或请求状态。
  querySelectorAll(tag) { return this.children.flatMap(child => [...(child.tagName === tag ? [child] : []), ...child.querySelectorAll(tag)]); }
  // 【逐行批注】 计算右侧结果并更新 focus() { this.focused，改变页面或请求状态。
  focus() { this.focused = true; }
  // 【逐行批注】 计算右侧结果并更新 get dataset() { return this._dataset || (thi…，改变页面或请求状态。
  get dataset() { return this._dataset || (this._dataset = {}); }
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 定义函数 main，封装可复用的前端行为。
async function main() {
  // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
  const body = new Element('body'), main = new Element('main'), intervals = [], controls = [], requests = [];
  // 【逐行批注】 声明前端变量 find，保存本行计算出的数据或对象。
  const find = test => { function visit(node) { if (test(node)) return node; for (const child of node.children) { const value = visit(child); if (value) return value; } } return visit(body); };
  // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
  let task = { id: 'one', status: 'running', calls: 2, purpose: '准备下一课',
    // 【逐行批注】 执行当前前端表达式：target: { courseTitle: '化学课程', lessonTitle: '化合价与电子变化' },。
    target: { courseTitle: '化学课程', lessonTitle: '化合价与电子变化' },
    // 【逐行批注】 执行当前前端表达式：activity: { phase: 'tool', title: '校验并保存选择题', detail: '正在校验本次操作的数据。' },。
    activity: { phase: 'tool', title: '校验并保存选择题', detail: '正在校验本次操作的数据。' },
    // 【逐行批注】 执行当前前端表达式：events: [{ seq: 3, type: 'action', at: '2026-10-08T08:10:00Z', activity: { title: '创建本次课时…。
    events: [{ seq: 3, type: 'action', at: '2026-10-08T08:10:00Z', activity: { title: '创建本次课时', detail: '课时已创建。' }, message: 'PRIVATE-DO-NOT-RENDER' }] };
  // 【逐行批注】 声明前端变量 defer，保存本行计算出的数据或对象。
  let defer = null, globalPaused = false;
  // 【逐行批注】 声明前端变量 snapshot，保存本行计算出的数据或对象。
  const snapshot = () => ({ paused: globalPaused, todayCalls: 5, tasks: [
    // 【逐行批注】 执行当前前端表达式：{ ...task, taskId: task.id, source: 'AI 课堂', tracked: true },。
    { ...task, taskId: task.id, source: 'AI 课堂', tracked: true },
    // 【逐行批注】 执行当前前端表达式：{ id: 'picture', source: '目标图片', purpose: '识别图片并提取学习目标', status: globalPaused ? 'paused' …。
    { id: 'picture', source: '目标图片', purpose: '识别图片并提取学习目标', status: globalPaused ? 'paused' : 'requesting', calls: 3, tracked: true }
  // 【逐行批注】 执行当前前端表达式：] });。
  ] });
  // 【逐行批注】 声明前端变量 document，保存本行计算出的数据或对象。
  const document = { body, hidden: false, createElement: tag => new Element(tag), getElementById: id => find(node => node.id === id),
    // 【逐行批注】 为页面事件绑定处理逻辑：querySelector: selector => selector === 'main' ? main : null, querySelectorAll: () =…。
    querySelector: selector => selector === 'main' ? main : null, querySelectorAll: () => [], addEventListener() {}, dispatchEvent() {} };
  // 【逐行批注】 声明前端变量 api，保存本行计算出的数据或对象。
  const api = { storage: { get: () => null }, failureMessage: value => value.failure?.message || value.error || '',
    // 【逐行批注】 计算右侧结果并更新 request: async (url, data)，改变页面或请求状态。
    request: async (url, data) => { requests.push(url); if (url === '/api/courses') return {};
      // 【逐行批注】 判断条件 (url === '/api/ai-activity/control') { controls.push({ ...data, …，成立时执行对应逻辑。
      if (url === '/api/ai-activity/control') { controls.push({ ...data, all: true }); globalPaused = data.command === 'pause'; task = { ...task, paused: globalPaused, status: globalPaused ? 'paused' : 'running' }; return snapshot(); }
      // 【逐行批注】 判断条件 (url === '/api/ai-activity') return snapshot();，成立时执行对应逻辑。
      if (url === '/api/ai-activity') return snapshot();
      // 【逐行批注】 判断条件 (defer) { const pending = defer; defer = null; return pending; }…，成立时执行对应逻辑。
      if (defer) { const pending = defer; defer = null; return pending; } return task; },
    // 【逐行批注】 计算右侧结果并更新 control: async value，改变页面或请求状态。
    control: async value => { controls.push(value); task = { ...task, paused: true, status: 'paused' }; return task; } };
  // 【逐行批注】 声明前端变量 window，保存本行计算出的数据或对象。
  const window = { GangyiAgent: api, addEventListener() {} };
  // 【逐行批注】 执行当前前端表达式：vm.runInNewContext(fs.readFileSync(path.join(__dirname, '../public/agent-shell.js'), 'utf…。
  vm.runInNewContext(fs.readFileSync(path.join(__dirname, '../public/agent-shell.js'), 'utf8'), {
    // 【逐行批注】 计算右侧结果并更新 document, window, location: { pathname: '/le…，改变页面或请求状态。
    document, window, location: { pathname: '/learn', search: '?taskId=one' }, URLSearchParams,
    // 【逐行批注】 计算右侧结果并更新 CustomEvent: class { constructor(name, value…，改变页面或请求状态。
    CustomEvent: class { constructor(name, value) { this.type = name; this.detail = value.detail; } },
    // 【逐行批注】 计算右侧结果并更新 setInterval: fn，改变页面或请求状态。
    setInterval: fn => { intervals.push(fn); return 1; }, clearInterval() {}
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 调用 drain 执行页面交互或数据处理：await drain(); await drain();。
  await drain(); await drain();
  // 【逐行批注】 声明前端变量 byClass，保存本行计算出的数据或对象。
  const byClass = name => find(node => (node.className || '').split(' ').includes(name));
  // 【逐行批注】 声明前端变量 navigation，保存本行计算出的数据或对象。
  const navigation = document.getElementById('learning-navigation'), history = byClass('ai-control-history'), details = byClass('ai-task-details');
  // 【逐行批注】 执行当前前端表达式：assert.equal(history.open, false); assert.equal(details.open, false);。
  assert.equal(history.open, false); assert.equal(details.open, false);
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.equal(byClass('ai-operation-title').textContent, '校验并保存选择题');。
  assert.equal(byClass('ai-operation-title').textContent, '校验并保存选择题');
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.match(byClass('ai-operation-target').textContent, /化学课程.*化合价与电…。
  assert.match(byClass('ai-operation-target').textContent, /化学课程.*化合价与电子变化/);
  // 【逐行批注】 执行当前前端表达式：assert.ok(!body.textContent.includes('PRIVATE-'), '操作面板不呈现工具输出和教师正文');。
  assert.ok(!body.textContent.includes('PRIVATE-'), '操作面板不呈现工具输出和教师正文');
  // 【逐行批注】 执行当前前端表达式：assert.match(document.getElementById('ai-live-calls').textContent, /AI 课堂/);。
  assert.match(document.getElementById('ai-live-calls').textContent, /AI 课堂/);
  // 【逐行批注】 执行当前前端表达式：assert.match(document.getElementById('ai-live-calls').textContent, /目标图片/);。
  assert.match(document.getElementById('ai-live-calls').textContent, /目标图片/);
  // 【逐行批注】 执行当前前端表达式：assert.match(document.getElementById('ai-live-calls').textContent, /实际请求 3 次/);。
  assert.match(document.getElementById('ai-live-calls').textContent, /实际请求 3 次/);
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.equal(byClass('ai-call-history').open, false, '调用历史默认折叠');。
  assert.equal(byClass('ai-call-history').open, false, '调用历史默认折叠');
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.match(byClass('ai-task-facts').textContent, /主控处理轮次2 次/);。
  assert.match(byClass('ai-task-facts').textContent, /主控处理轮次2 次/);
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.ok(!byClass('ai-task-facts').textContent.includes('AI 请求'), '主…。
  assert.ok(!byClass('ai-task-facts').textContent.includes('AI 请求'), '主控轮次不能冒充实际请求次数');
  // 【逐行批注】 计算右侧结果并更新 history.open，改变页面或请求状态。
  history.open = true; details.open = true;
  // 【逐行批注】 声明前端变量 historyNode，保存本行计算出的数据或对象。
  const historyNode = byClass('ai-step-list').firstChild, button = find(node => node.tagName === 'button' && node.textContent === '暂停全部 AI');
  // 【逐行批注】 调用 drain 执行页面交互或数据处理：intervals[0](); await drain();。
  intervals[0](); await drain();
  // 【逐行批注】 执行当前前端表达式：assert.equal(history.open, true); assert.equal(details.open, true);。
  assert.equal(history.open, true); assert.equal(details.open, true);
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.equal(byClass('ai-step-list').firstChild, historyNode, '轮询不重建未…。
  assert.equal(byClass('ai-step-list').firstChild, historyNode, '轮询不重建未改变的步骤');
  // 【逐行批注】 计算右侧结果并更新 assert.equal(find(node，改变页面或请求状态。
  assert.equal(find(node => node.tagName === 'button' && node.textContent === '暂停全部 AI'), button, '轮询保留控制按钮与键盘焦点');
  // 【逐行批注】 执行当前前端表达式：await button.onclick();。
  await button.onclick();
  // 【逐行批注】 执行当前前端表达式：assert.equal(controls.length, 1); assert.equal(controls[0].command, 'pause'); assert.equa…。
  assert.equal(controls.length, 1); assert.equal(controls[0].command, 'pause'); assert.equal(controls[0].all, true);
  // 【逐行批注】 执行当前前端表达式：assert.equal(navigation.firstChild.textContent, '全部 AI 已暂停');。
  assert.equal(navigation.firstChild.textContent, '全部 AI 已暂停');
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.equal(byClass('ai-operation-title').textContent, '已暂停：校验并保存选择题…。
  assert.equal(byClass('ai-operation-title').textContent, '已暂停：校验并保存选择题');
  // 【逐行批注】 计算右侧结果并更新 task，改变页面或请求状态。
  task = { ...task, status: 'running', paused: true }; intervals[0](); await drain();
  // 【逐行批注】 执行当前前端表达式：assert.equal(navigation.firstChild.textContent, '全部 AI 已暂停', '暂停状态不能被处理中遮盖');。
  assert.equal(navigation.firstChild.textContent, '全部 AI 已暂停', '暂停状态不能被处理中遮盖');
  // 【逐行批注】 计算右侧结果并更新 await find(node，改变页面或请求状态。
  await find(node => node.tagName === 'button' && node.textContent === '恢复全部 AI').onclick();
  // 【逐行批注】 执行当前前端表达式：assert.equal(controls[1].command, 'resume'); assert.equal(controls[1].all, true);。
  assert.equal(controls[1].command, 'resume'); assert.equal(controls[1].all, true);
  // 【逐行批注】 计算右侧结果并更新 task，改变页面或请求状态。
  task = { id: 'legacy', status: 'ready', calls: 3, events: [{ type: 'action', message: 'PRIVATE-OLD-MESSAGE' }] };
  // 【逐行批注】 调用 drain 执行页面交互或数据处理：intervals[0](); await drain();。
  intervals[0](); await drain();
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.match(byClass('ai-operation-detail').textContent, /历史任务未记录具体执行…。
  assert.match(byClass('ai-operation-detail').textContent, /历史任务未记录具体执行步骤/);
  // 【逐行批注】 执行当前前端表达式：assert.ok(!body.textContent.includes('PRIVATE-OLD-MESSAGE'));。
  assert.ok(!body.textContent.includes('PRIVATE-OLD-MESSAGE'));
  // 【逐行批注】 计算右侧结果并更新 task，改变页面或请求状态。
  task = { id: 'failed', status: 'failed', failure: { operation: '校验并保存选择题', message: '题目选项未完整保存。', attempt: 3 }, events: [] };
  // 【逐行批注】 调用 drain 执行页面交互或数据处理：intervals[0](); await drain();。
  intervals[0](); await drain();
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.equal(byClass('ai-operation-title').textContent, '校验并保存选择题');。
  assert.equal(byClass('ai-operation-title').textContent, '校验并保存选择题');
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.equal(byClass('ai-operation-detail').textContent, '题目选项未完整保存。'…。
  assert.equal(byClass('ai-operation-detail').textContent, '题目选项未完整保存。');
  // 切换课时后，上一任务的迟到错误也不能覆盖当前面板。
  // 【逐行批注】 声明前端变量 reject，保存本行计算出的数据或对象。
  let reject;
  // 【逐行批注】 计算右侧结果并更新 defer，改变页面或请求状态。
  defer = new Promise((resolve, no) => { reject = no; });
  // 【逐行批注】 调用 drain 执行页面交互或数据处理：intervals[0](); await drain();。
  intervals[0](); await drain();
  // 【逐行批注】 计算右侧结果并更新 task，改变页面或请求状态。
  task = { id: 'new', status: 'running', purpose: '回应你的交流', activity: { title: '接收 AI 的教学回复', detail: '正在接收公开内容。' }, events: [] };
  // 【逐行批注】 调用 drain 执行页面交互或数据处理：await window.GangyiNavigation.setContext({ taskId: 'new' }); await dr…。
  await window.GangyiNavigation.setContext({ taskId: 'new' }); await drain();
  // 【逐行批注】 调用 reject、Error、drain 执行页面交互或数据处理：reject(new Error('上一任务的迟到错误')); await drain();。
  reject(new Error('上一任务的迟到错误')); await drain();
  // 【逐行批注】 调用 byClass 执行页面交互或数据处理：assert.equal(byClass('ai-operation-title').textContent, '接收 AI 的教学回复'…。
  assert.equal(byClass('ai-operation-title').textContent, '接收 AI 的教学回复');
  // 【逐行批注】 执行当前前端表达式：assert.ok(!body.textContent.includes('迟到错误'));。
  assert.ok(!body.textContent.includes('迟到错误'));
  // 【逐行批注】 计算右侧结果并更新 assert.ok(requests.some(url，改变页面或请求状态。
  assert.ok(requests.some(url => url.endsWith('taskId=new')));
  // 【逐行批注】 执行当前前端表达式：console.log('AI 状态面板：全软件实时列表、实际次数、全局暂停恢复、折叠保持、历史兼容及迟到请求回归通过。');。
  console.log('AI 状态面板：全软件实时列表、实际次数、全局暂停恢复、折叠保持、历史兼容及迟到请求回归通过。');
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 计算右侧结果并更新 main().catch(error，改变页面或请求状态。
main().catch(error => { console.error(error); process.exitCode = 1; });

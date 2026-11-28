// 【逐行批注】 声明前端变量 assert，保存本行计算出的数据或对象。
const assert = require('node:assert/strict');
// 【逐行批注】 声明前端变量 fs，保存本行计算出的数据或对象。
const fs = require('node:fs');
// 【逐行批注】 声明前端变量 path，保存本行计算出的数据或对象。
const path = require('node:path');
// 【逐行批注】 声明前端变量 vm，保存本行计算出的数据或对象。
const vm = require('node:vm');

// 【逐行批注】 执行当前前端表达式：class Element {。
class Element {
  // 【逐行批注】 调用 constructor 执行页面交互或数据处理：constructor(tag) {。
  constructor(tag) {
    // 【逐行批注】 计算右侧结果并更新 this.tagName，改变页面或请求状态。
    this.tagName = tag;
    // 【逐行批注】 计算右侧结果并更新 this.children，改变页面或请求状态。
    this.children = [];
    // 【逐行批注】 计算右侧结果并更新 this.attributes，改变页面或请求状态。
    this.attributes = {};
    // 【逐行批注】 计算右侧结果并更新 this.listeners，改变页面或请求状态。
    this.listeners = {};
    // 【逐行批注】 计算右侧结果并更新 this.hidden，改变页面或请求状态。
    this.hidden = false;
    // 【逐行批注】 计算右侧结果并更新 this.disabled，改变页面或请求状态。
    this.disabled = false;
    // 【逐行批注】 计算右侧结果并更新 this.open，改变页面或请求状态。
    this.open = false;
    // 【逐行批注】 计算右侧结果并更新 this._text，改变页面或请求状态。
    this._text = '';
    // 【逐行批注】 计算右侧结果并更新 this.className，改变页面或请求状态。
    this.className = '';
    // 【逐行批注】 计算右侧结果并更新 this.classList，改变页面或请求状态。
    this.classList = {add: (...names) => { this.className = [...new Set((this.className + ' ' + names.join(' ')).trim().split(/\s+/))].join(' '); }};
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 计算右侧结果并更新 set textContent(value) { this._text，改变页面或请求状态。
  set textContent(value) { this._text = String(value ?? ''); this.children = []; }
  // 【逐行批注】 计算右侧结果并更新 get textContent() { return this._text + this…，改变页面或请求状态。
  get textContent() { return this._text + this.children.map(child => child.textContent).join(''); }
  // 【逐行批注】 调用 append 执行页面交互或数据处理：append(...nodes) { this.children.push(...nodes); }。
  append(...nodes) { this.children.push(...nodes); }
  // 【逐行批注】 调用 prepend 执行页面交互或数据处理：prepend(...nodes) { this.children.unshift(...nodes); }。
  prepend(...nodes) { this.children.unshift(...nodes); }
  // 【逐行批注】 计算右侧结果并更新 replaceChildren(...nodes) { this._text，改变页面或请求状态。
  replaceChildren(...nodes) { this._text = ''; this.children = nodes; }
  // 【逐行批注】 计算右侧结果并更新 setAttribute(name, value) { this.attributes[…，改变页面或请求状态。
  setAttribute(name, value) { this.attributes[name] = String(value); }
  // 【逐行批注】 调用 getAttribute 执行页面交互或数据处理：getAttribute(name) { return Object.hasOwn(this.attributes, name) ? th…。
  getAttribute(name) { return Object.hasOwn(this.attributes, name) ? this.attributes[name] : null; }
  // 【逐行批注】 调用 removeAttribute 执行页面交互或数据处理：removeAttribute(name) { delete this.attributes[name]; }。
  removeAttribute(name) { delete this.attributes[name]; }
  // 【逐行批注】 为页面事件绑定处理逻辑：addEventListener(name, callback) { this.listeners[name] = callback; }。
  addEventListener(name, callback) { this.listeners[name] = callback; }
  // 【逐行批注】 计算右侧结果并更新 get dataset() { return this._dataset || (thi…，改变页面或请求状态。
  get dataset() { return this._dataset || (this._dataset = {}); }
// 【逐行批注】 结束当前前端代码块。
}

// 【逐行批注】 定义函数 main，封装可复用的前端行为。
async function main() {
// 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
const host = new Element('div');
// 【逐行批注】 声明前端变量 document，保存本行计算出的数据或对象。
const document = {
  // 【逐行批注】 计算右侧结果并更新 createElement: tag，改变页面或请求状态。
  createElement: tag => new Element(tag),
  // 【逐行批注】 计算右侧结果并更新 createTextNode: value，改变页面或请求状态。
  createTextNode: value => ({textContent: String(value ?? '')}),
// 【逐行批注】 执行当前前端表达式：};。
};
// 【逐行批注】 声明前端变量 window，保存本行计算出的数据或对象。
const window = {};
// 【逐行批注】 声明前端变量 context，保存本行计算出的数据或对象。
const context = vm.createContext({
  // 【逐行批注】 执行当前前端表达式：window, document, Date, Number, String, Array, Map, Set, console, setTimeout, clearTimeou…。
  window, document, Date, Number, String, Array, Map, Set, console, setTimeout, clearTimeout,
// 【逐行批注】 执行当前前端表达式：});。
});
// 【逐行批注】 执行当前前端表达式：vm.runInContext(fs.readFileSync(path.join(__dirname, '../public/agent-trace.js'), 'utf8')…。
vm.runInContext(fs.readFileSync(path.join(__dirname, '../public/agent-trace.js'), 'utf8'), context);

// 【逐行批注】 声明前端变量 finishRetry，保存本行计算出的数据或对象。
let finishRetry;
// 【逐行批注】 声明前端变量 trace，保存本行计算出的数据或对象。
const trace = window.GangyiTrace.create(host, {
  // 【逐行批注】 执行当前前端表达式：variant: 'inline',。
  variant: 'inline',
  // 【逐行批注】 执行当前前端表达式：collapseWhenDone: true,。
  collapseWhenDone: true,
  // 【逐行批注】 计算右侧结果并更新 onRetry: ()，改变页面或请求状态。
  onRetry: () => new Promise(resolve => { finishRetry = resolve; }),
// 【逐行批注】 执行当前前端表达式：});。
});
// 【逐行批注】 声明前端变量 started，保存本行计算出的数据或对象。
const started = new Date(Date.now() - 95000).toISOString();
// 【逐行批注】 执行当前前端表达式：trace.update({。
trace.update({
  // 【逐行批注】 执行当前前端表达式：id: 'trace-running', status: 'running', purpose: '准备下一课', calls: 3, createdAt: started,。
  id: 'trace-running', status: 'running', purpose: '准备下一课', calls: 3, createdAt: started,
  // 【逐行批注】 执行当前前端表达式：activity: {title: '生成讲义', detail: '正在核对并保存已完成的讲解。', state: 'started', at: started},。
  activity: {title: '生成讲义', detail: '正在核对并保存已完成的讲解。', state: 'started', at: started},
  // 【逐行批注】 执行当前前端表达式：events: [。
  events: [
    // 【逐行批注】 执行当前前端表达式：{seq: 1, type: 'activity', at: started, activity: {title: '读取学习记录', detail: '已读取本课记录。', s…。
    {seq: 1, type: 'activity', at: started, activity: {title: '读取学习记录', detail: '已读取本课记录。', state: 'completed'}},
    // 【逐行批注】 调用 Date 执行页面交互或数据处理：{seq: 2, type: 'action', at: new Date().toISOString(), activity: {tit…。
    {seq: 2, type: 'action', at: new Date().toISOString(), activity: {title: '保存讲解', detail: '讲解已保存。', state: 'completed'}},
  // 【逐行批注】 执行当前前端表达式：],。
  ],
// 【逐行批注】 执行当前前端表达式：});。
});
// 【逐行批注】 执行当前前端表达式：assert.equal(host.hidden, false);。
assert.equal(host.hidden, false);
// 【逐行批注】 执行当前前端表达式：assert.match(host.textContent, /生成讲义/);。
assert.match(host.textContent, /生成讲义/);
// 【逐行批注】 执行当前前端表达式：assert.match(host.textContent, /已用/);。
assert.match(host.textContent, /已用/);
// 【逐行批注】 执行当前前端表达式：assert.match(host.textContent, /90 秒没有新的有效进展/);。
assert.match(host.textContent, /90 秒没有新的有效进展/);
// 【逐行批注】 执行当前前端表达式：assert.ok(!host.textContent.includes('privateToolArguments'));。
assert.ok(!host.textContent.includes('privateToolArguments'));
// 【逐行批注】 声明前端变量 root，保存本行计算出的数据或对象。
const root = host.children[0];
// 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
const body = root.children.find(node => node.className === 'gy-trace-body');
// 【逐行批注】 声明前端变量 toggle，保存本行计算出的数据或对象。
const toggle = root.children[0].children.find(node => node.className === 'gy-trace-toggle');
// 【逐行批注】 执行当前前端表达式：assert.equal(toggle.getAttribute('aria-expanded'), 'true');。
assert.equal(toggle.getAttribute('aria-expanded'), 'true');
// 【逐行批注】 执行当前前端表达式：assert.equal(body.hidden, false);。
assert.equal(body.hidden, false);
// 【逐行批注】 执行当前前端表达式：toggle.onclick();。
toggle.onclick();
// 【逐行批注】 执行当前前端表达式：assert.equal(toggle.getAttribute('aria-expanded'), 'false');。
assert.equal(toggle.getAttribute('aria-expanded'), 'false');
// 【逐行批注】 执行当前前端表达式：assert.equal(toggle.getAttribute('aria-label'), '展开 AI 执行过程');。
assert.equal(toggle.getAttribute('aria-label'), '展开 AI 执行过程');
// 【逐行批注】 执行当前前端表达式：assert.equal(body.hidden, true);。
assert.equal(body.hidden, true);
// 【逐行批注】 执行当前前端表达式：toggle.onclick();。
toggle.onclick();
// 【逐行批注】 执行当前前端表达式：assert.equal(body.hidden, false);。
assert.equal(body.hidden, false);

// 【逐行批注】 执行当前前端表达式：trace.update({。
trace.update({
  // 【逐行批注】 执行当前前端表达式：id: 'trace-running', status: 'failed', purpose: '准备下一课', calls: 4, createdAt: started,。
  id: 'trace-running', status: 'failed', purpose: '准备下一课', calls: 4, createdAt: started,
  // 【逐行批注】 调用 Date 执行页面交互或数据处理：activity: {title: '生成正文', detail: '请求失败。', state: 'failed', at: new D…。
  activity: {title: '生成正文', detail: '请求失败。', state: 'failed', at: new Date().toISOString()},
  // 【逐行批注】 执行当前前端表达式：failure: {message: 'AI 服务已连接，但生成正文超时；已有有效步骤已保留。'},。
  failure: {message: 'AI 服务已连接，但生成正文超时；已有有效步骤已保留。'},
  // 【逐行批注】 执行当前前端表达式：events: [],。
  events: [],
// 【逐行批注】 执行当前前端表达式：});。
});
// 【逐行批注】 执行当前前端表达式：assert.match(host.textContent, /已有有效步骤已保留/);。
assert.match(host.textContent, /已有有效步骤已保留/);
// 【逐行批注】 计算右侧结果并更新 assert.ok(body.children.some(node，改变页面或请求状态。
assert.ok(body.children.some(node => node.className === 'gy-trace-actions' && node.hidden === false));
// 【逐行批注】 声明前端变量 actions，保存本行计算出的数据或对象。
const actions = body.children.find(node => node.className === 'gy-trace-actions');
// 【逐行批注】 声明前端变量 retryButton，保存本行计算出的数据或对象。
const retryButton = actions.children.find(node => node.tagName === 'button');
// 【逐行批注】 执行当前前端表达式：trace.update({。
trace.update({
  // 【逐行批注】 执行当前前端表达式：id: 'trace-running', status: 'failed', purpose: '准备下一课', createdAt: started,。
  id: 'trace-running', status: 'failed', purpose: '准备下一课', createdAt: started,
  // 【逐行批注】 调用 Date 执行页面交互或数据处理：activity: {title: '生成正文', detail: '请求失败。', state: 'failed', at: new D…。
  activity: {title: '生成正文', detail: '请求失败。', state: 'failed', at: new Date().toISOString()},
  // 【逐行批注】 执行当前前端表达式：failure: {message: 'AI 服务已连接，但生成正文超时；已有有效步骤已保留。'},。
  failure: {message: 'AI 服务已连接，但生成正文超时；已有有效步骤已保留。'},
// 【逐行批注】 执行当前前端表达式：});。
});
// 【逐行批注】 声明前端变量 retrying，保存本行计算出的数据或对象。
const retrying = retryButton.onclick();
// 【逐行批注】 执行当前前端表达式：assert.equal(retryButton.disabled, false);。
assert.equal(retryButton.disabled, false);
// 【逐行批注】 执行当前前端表达式：assert.equal(retryButton.getAttribute('aria-busy'), 'true');。
assert.equal(retryButton.getAttribute('aria-busy'), 'true');
// 【逐行批注】 执行当前前端表达式：assert.equal(retryButton.getAttribute('aria-disabled'), 'true');。
assert.equal(retryButton.getAttribute('aria-disabled'), 'true');
// 【逐行批注】 执行当前前端表达式：assert.match(retryButton.textContent, /…/);。
assert.match(retryButton.textContent, /…/);
// 【逐行批注】 执行当前前端表达式：await retryButton.onclick();。
await retryButton.onclick();
// 【逐行批注】 执行当前前端表达式：assert.equal(retryButton.getAttribute('aria-busy'), 'true', '重复点击必须被重入保护拦住');。
assert.equal(retryButton.getAttribute('aria-busy'), 'true', '重复点击必须被重入保护拦住');
// 【逐行批注】 调用 finishRetry 执行页面交互或数据处理：finishRetry();。
finishRetry();
// 【逐行批注】 执行当前前端表达式：await retrying;。
await retrying;
// 【逐行批注】 执行当前前端表达式：assert.equal(retryButton.getAttribute('aria-busy'), 'false');。
assert.equal(retryButton.getAttribute('aria-busy'), 'false');
// 【逐行批注】 执行当前前端表达式：assert.equal(retryButton.getAttribute('aria-disabled'), 'false');。
assert.equal(retryButton.getAttribute('aria-disabled'), 'false');
// 【逐行批注】 执行当前前端表达式：assert.ok(!retryButton.textContent.includes('…'));。
assert.ok(!retryButton.textContent.includes('…'));

// 【逐行批注】 执行当前前端表达式：trace.update({。
trace.update({
  // 【逐行批注】 执行当前前端表达式：id: 'trace-running', status: 'ready', purpose: '准备下一课', calls: 5, createdAt: started,。
  id: 'trace-running', status: 'ready', purpose: '准备下一课', calls: 5, createdAt: started,
  // 【逐行批注】 调用 Date 执行页面交互或数据处理：activity: {title: '本次处理已完成', detail: '完整内容已保存。', state: 'completed', …。
  activity: {title: '本次处理已完成', detail: '完整内容已保存。', state: 'completed', at: new Date().toISOString()},
  // 【逐行批注】 执行当前前端表达式：events: [],。
  events: [],
// 【逐行批注】 执行当前前端表达式：});。
});
// 【逐行批注】 声明前端变量 details，保存本行计算出的数据或对象。
const details = body.children.find(node => node.tagName === 'details');
// 【逐行批注】 执行当前前端表达式：assert.equal(details.open, false);。
assert.equal(details.open, false);
// 【逐行批注】 执行当前前端表达式：assert.match(host.textContent, /已完成/);。
assert.match(host.textContent, /已完成/);

// 【逐行批注】 声明前端变量 compactHost，保存本行计算出的数据或对象。
const compactHost = new Element('div');
// 【逐行批注】 声明前端变量 compactTrace，保存本行计算出的数据或对象。
const compactTrace = window.GangyiTrace.create(compactHost, {defaultCollapsed: true, collapseWhenDone: true});
// 【逐行批注】 执行当前前端表达式：compactTrace.update({。
compactTrace.update({
  // 【逐行批注】 执行当前前端表达式：id: 'trace-compact', status: 'running', purpose: '准备下一课', createdAt: started,。
  id: 'trace-compact', status: 'running', purpose: '准备下一课', createdAt: started,
  // 【逐行批注】 调用 Date 执行页面交互或数据处理：activity: {title: '生成讲义', state: 'started', at: new Date().toISOStrin…。
  activity: {title: '生成讲义', state: 'started', at: new Date().toISOString()}, events: [],
// 【逐行批注】 执行当前前端表达式：});。
});
// 【逐行批注】 声明前端变量 compactRoot，保存本行计算出的数据或对象。
const compactRoot = compactHost.children[0];
// 【逐行批注】 声明前端变量 compactBody，保存本行计算出的数据或对象。
const compactBody = compactRoot.children.find(node => node.className === 'gy-trace-body');
// 【逐行批注】 声明前端变量 compactToggle，保存本行计算出的数据或对象。
const compactToggle = compactRoot.children[0].children.find(node => node.className === 'gy-trace-toggle');
// 【逐行批注】 执行当前前端表达式：assert.equal(compactToggle.getAttribute('aria-expanded'), 'false');。
assert.equal(compactToggle.getAttribute('aria-expanded'), 'false');
// 【逐行批注】 执行当前前端表达式：assert.equal(compactBody.hidden, true);。
assert.equal(compactBody.hidden, true);
// 【逐行批注】 执行当前前端表达式：compactTrace.update({。
compactTrace.update({
  // 【逐行批注】 执行当前前端表达式：id: 'trace-compact', status: 'failed', purpose: '准备下一课', createdAt: started,。
  id: 'trace-compact', status: 'failed', purpose: '准备下一课', createdAt: started,
  // 【逐行批注】 调用 Date 执行页面交互或数据处理：activity: {title: '生成正文', state: 'failed', at: new Date().toISOString…。
  activity: {title: '生成正文', state: 'failed', at: new Date().toISOString()},
  // 【逐行批注】 执行当前前端表达式：failure: {message: '生成失败，已有步骤已保留。'}, events: [],。
  failure: {message: '生成失败，已有步骤已保留。'}, events: [],
// 【逐行批注】 执行当前前端表达式：});。
});
// 【逐行批注】 执行当前前端表达式：assert.equal(compactToggle.getAttribute('aria-expanded'), 'true');。
assert.equal(compactToggle.getAttribute('aria-expanded'), 'true');
// 【逐行批注】 执行当前前端表达式：assert.equal(compactBody.hidden, false);。
assert.equal(compactBody.hidden, false);
// 【逐行批注】 执行当前前端表达式：console.log('AI 执行轨迹：可折叠、默认紧凑、失败自动展开、焦点与脱敏检查通过。');。
console.log('AI 执行轨迹：可折叠、默认紧凑、失败自动展开、焦点与脱敏检查通过。');
// 【逐行批注】 结束当前前端代码块。
}

// 【逐行批注】 计算右侧结果并更新 main().catch(error，改变页面或请求状态。
main().catch(error => {
  // 【逐行批注】 执行当前前端表达式：console.error(error);。
  console.error(error);
  // 【逐行批注】 计算右侧结果并更新 process.exitCode，改变页面或请求状态。
  process.exitCode = 1;
// 【逐行批注】 执行当前前端表达式：});。
});

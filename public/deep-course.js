/* 深度课堂只呈现已保存的教学内容，图示交互在本地完成。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 声明前端变量 mount，保存本行计算出的数据或对象。
  const mount = document.getElementById('deep-course-app');
  // 【逐行批注】 判断条件 (!mount) return;，成立时执行对应逻辑。
  if (!mount) return;
  // 【逐行批注】 声明前端变量 api，保存本行计算出的数据或对象。
  const api = window.GangyiAgent;
  // 【逐行批注】 声明前端变量 query，保存本行计算出的数据或对象。
  const query = new URLSearchParams(location.search);
  // 【逐行批注】 声明前端变量 allowedScope，保存本行计算出的数据或对象。
  const allowedScope = ['lessonId', 'courseId', 'phaseIndex', 'topicIndex', 'topicId'];
  // 【逐行批注】 声明前端变量 scope，保存本行计算出的数据或对象。
  const scope = Object.fromEntries(allowedScope.filter(key => query.has(key)).map(key => [key, query.get(key)]));
  // 【逐行批注】 声明前端变量 scopeError，保存本行计算出的数据或对象。
  let scopeError = '';
  // 【逐行批注】 按循环规则遍历或重复处理：for (const key of ['phaseIndex', 'topicIndex']) if (Object.hasOwn(scope, k…。
  for (const key of ['phaseIndex', 'topicIndex']) if (Object.hasOwn(scope, key)) {
    // 【逐行批注】 声明前端变量 raw，保存本行计算出的数据或对象。
    const raw = scope[key]; const value = /^\d+$/.test(raw) ? Number(raw) : NaN;
    // 【逐行批注】 判断条件 (!Number.isSafeInteger(value) || value < 1 || value > 100000) sc…，成立时执行对应逻辑。
    if (!Number.isSafeInteger(value) || value < 1 || value > 100000) scopeError = '课程位置无效，请从课程目录重新选择知识点。';
    // 【逐行批注】 执行前一条件不成立时的分支。
    else scope[key] = value;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 saved，保存本行计算出的数据或对象。
  let saved = null, lesson = null, currentView = 'lecture', currentSlide = 0, busy = false, sending = false;
  // 【逐行批注】 声明前端变量 watchingTask，保存本行计算出的数据或对象。
  let watchingTask = '', stopWatching = null, refreshPromise = null, refreshAgain = false, activeEpoch = 0;
  // 【逐行批注】 声明前端变量 taskMessage，保存本行计算出的数据或对象。
  let taskMessage = '', lastTask = null, pendingMessages = [], enteringLesson = '', entryFailure = '', focusedTeachingTask = '';
  let visitedLessonId = '';
  let workspaceContext, learningTools;
  // 【逐行批注】 声明前端变量 tracePanel，保存本行计算出的数据或对象。
  let tracePanel = null;
  // 【逐行批注】 声明前端变量 localFailureRetry，保存本行计算出的数据或对象。
  let localFailureRetry = null;
  // 【逐行批注】 声明前端变量 fullscreenMode，保存本行计算出的数据或对象。
  let fullscreenMode = false;
  // 【逐行批注】 声明前端变量 chatOpenBeforeFullscreen，保存本行计算出的数据或对象。
  let chatOpenBeforeFullscreen = 'false';
  // 【逐行批注】 声明前端变量 lastFullscreenCompact，保存本行计算出的数据或对象。
  let lastFullscreenCompact = null;
  // 【逐行批注】 声明前端变量 independentChat，保存本行计算出的数据或对象。
  let independentChat = {lessonId:'', webEnabled:false, messages:[], activeTask:null};
  // 【逐行批注】 声明前端变量 independentChatLesson，保存本行计算出的数据或对象。
  let independentChatLesson = '';
  // 【逐行批注】 声明前端变量 independentChatEpoch，保存本行计算出的数据或对象。
  let independentChatEpoch = 0;
  // 【逐行批注】 声明前端变量 independentChatSubmitting，保存本行计算出的数据或对象。
  const independentChatSubmitting = new Set();
  // 【逐行批注】 声明前端变量 independentChatStreaming，保存本行计算出的数据或对象。
  const independentChatStreaming = new Set();
  // 【逐行批注】 声明前端变量 independentChatStream，保存本行计算出的数据或对象。
  let independentChatStream = '';
  let chatRenderFrame = 0, chatNodeScope = '';
  const chatNodeCache = new Map();
  // 【逐行批注】 声明前端变量 independentChatSeen，保存本行计算出的数据或对象。
  let independentChatSeen = new Set();
  // 【逐行批注】 声明前端变量 stopIndependentWatch，保存本行计算出的数据或对象。
  let stopIndependentWatch = null;
  // 【逐行批注】 声明前端变量 readingPositions，保存本行计算出的数据或对象。
  const readingPositions = new Map();
  // 【逐行批注】 声明前端变量 mediaValues，保存本行计算出的数据或对象。
  const mediaValues = new Map();
  const componentDraftSyncs = new Map();
  const componentTraces = new Map();
  const interactiveInstances = [];
  let readingSignature = '';
  // 【逐行批注】 声明前端变量 fullscreenSessionKey，保存本行计算出的数据或对象。
  const fullscreenSessionKey = 'gy-classroom-fullscreen';
  // 【逐行批注】 声明前端变量 svgNS，保存本行计算出的数据或对象。
  const svgNS = 'http://www.w3.org/2000/svg';
  // 【逐行批注】 声明前端变量 element，保存本行计算出的数据或对象。
  const element = (tag, text = '', className = '') => {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = document.createElement(tag); node.textContent = String(text ?? ''); node.className = className; return node;
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 svg，保存本行计算出的数据或对象。
  const svg = (tag, attributes = {}) => {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = document.createElementNS(svgNS, tag);
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [name, value] of Object.entries(attributes)) node.setAttribute(…。
    for (const [name, value] of Object.entries(attributes)) node.setAttribute(name, String(value));
    // 【逐行批注】 结束当前函数并返回结果：return node;。
    return node;
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 paths，保存本行计算出的数据或对象。
  const paths = {
    // 【逐行批注】 执行当前前端表达式：menu: '<path d="M4 6h16M4 12h16M4 18h16"/>',。
    menu: '<path d="M4 6h16M4 12h16M4 18h16"/>',
    // 【逐行批注】 执行当前前端表达式：close: '<path d="m6 6 12 12M18 6 6 18"/>',。
    close: '<path d="m6 6 12 12M18 6 6 18"/>',
    // 【逐行批注】 执行当前前端表达式：chat: '<path d="M4 4h16v12H9l-5 5Z"/><path d="M8 8h8M8 12h5"/>',。
    chat: '<path d="M4 4h16v12H9l-5 5Z"/><path d="M8 8h8M8 12h5"/>',
    // 【逐行批注】 执行当前前端表达式：book: '<path d="M12 5C8 3 5 3 2 4v15c4-1 7-1 10 1 3-2 6-2 10-1V4c-3-1-6-1-10 1Zm0 0v15"/>…。
    book: '<path d="M12 5C8 3 5 3 2 4v15c4-1 7-1 10 1 3-2 6-2 10-1V4c-3-1-6-1-10 1Zm0 0v15"/>',
    // 【逐行批注】 执行当前前端表达式：slides: '<rect x="3" y="3" width="18" height="13" rx="1"/><path d="M12 16v5m-4 0h8M7 8h10…。
    slides: '<rect x="3" y="3" width="18" height="13" rx="1"/><path d="M12 16v5m-4 0h8M7 8h10M7 11h7"/>',
    // 【逐行批注】 执行当前前端表达式：arrow: '<path d="M5 12h14m-5-5 5 5-5 5"/>',。
    arrow: '<path d="M5 12h14m-5-5 5 5-5 5"/>',
    // 【逐行批注】 执行当前前端表达式：left: '<path d="m14 5-7 7 7 7"/>',。
    left: '<path d="m14 5-7 7 7 7"/>',
    // 【逐行批注】 执行当前前端表达式：right: '<path d="m10 5 7 7-7 7"/>',。
    right: '<path d="m10 5 7 7-7 7"/>',
    // 【逐行批注】 执行当前前端表达式：send: '<path d="M12 20V4m-6 6 6-6 6 6"/>',。
    send: '<path d="M12 20V4m-6 6 6-6 6 6"/>',
    // 【逐行批注】 执行当前前端表达式：check: '<path d="m5 12 4 4L19 6"/>',。
    check: '<path d="m5 12 4 4L19 6"/>',
    // 【逐行批注】 执行当前前端表达式：more: '<circle cx="5" cy="12" r="1"/><circle cx="12" cy="12" r="1"/><circle cx="19" cy="1…。
    more: '<circle cx="5" cy="12" r="1"/><circle cx="12" cy="12" r="1"/><circle cx="19" cy="12" r="1"/>',
    // 【逐行批注】 执行当前前端表达式：expand: '<path d="M8 3H3v5M16 3h5v5M21 16v5h-5M3 16v5h5"/>',。
    expand: '<path d="M8 3H3v5M16 3h5v5M21 16v5h-5M3 16v5h5"/>',
    // 【逐行批注】 执行当前前端表达式：collapse: '<path d="M8 3v5H3M16 3v5h5M21 16h-5v5M3 16h5v5"/>',。
    collapse: '<path d="M8 3v5H3M16 3v5h5M21 16h-5v5M3 16h5v5"/>',
    // 【逐行批注】 执行当前前端表达式：globe: '<circle cx="12" cy="12" r="9"/><path d="M3 12h18M12 3c3 3 3 15 0 18M12 3c-3 3-3 1…。
    globe: '<circle cx="12" cy="12" r="9"/><path d="M3 12h18M12 3c3 3 3 15 0 18M12 3c-3 3-3 15 0 18"/>',
    // 【逐行批注】 执行当前前端表达式：trash: '<path d="M4 7h16M9 7V4h6v3m-9 0 1 14h10l1-14M10 11v6m4-6v6"/>',。
    trash: '<path d="M4 7h16M9 7V4h6v3m-9 0 1 14h10l1-14M10 11v6m4-6v6"/>',
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 定义函数 icon，封装可复用的前端行为。
  function icon(name) {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = svg('svg', {width:16, height:16, viewBox:'0 0 24 24', fill:'none', stroke:'currentColor', 'stroke-width':1.6, 'stroke-linecap':'round', 'stroke-linejoin':'round', 'aria-hidden':'true'});
    // 【逐行批注】 计算右侧结果并更新 node.innerHTML，改变页面或请求状态。
    node.innerHTML = paths[name] || paths.book; return node;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 button，封装可复用的前端行为。
  function button(text, action, primary = false, iconName = '') {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = element('button', '', 'dc-button' + (primary ? ' dc-primary' : ''));
    // 【逐行批注】 计算右侧结果并更新 node.type，改变页面或请求状态。
    node.type = 'button'; node.dataset.action = action;
    // 【逐行批注】 判断条件 (iconName) node.append(icon(iconName));，成立时执行对应逻辑。
    if (iconName) node.append(icon(iconName));
    // 【逐行批注】 判断条件 (text) node.append(element('span', text, iconName ? 'dc-action-l…，成立时执行对应逻辑。
    if (text) node.append(element('span', text, iconName ? 'dc-action-label' : ''));
    // 【逐行批注】 结束当前函数并返回结果：return node;。
    return node;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 rich，封装可复用的前端行为。
  function rich(value, className = 'dc-rich') {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = element('div', '', className);
    // 【逐行批注】 判断条件 (api?.richText) api.richText(node, value ?? '');，成立时执行对应逻辑。
    if (api?.richText) api.richText(node, value ?? '');
    // 【逐行批注】 当前一条件不成立时继续判断：(window.GangyiChat) node.innerHTML = window.GangyiChat.renderMar…。
    else if (window.GangyiChat) node.innerHTML = window.GangyiChat.renderMarkdown(String(value ?? ''), {math:true});
    // 【逐行批注】 执行前一条件不成立时的分支。
    else node.textContent = String(value ?? '');
    // 【逐行批注】 结束当前函数并返回结果：return node;。
    return node;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 items，保存本行计算出的数据或对象。
  const items = value => Array.isArray(value) ? value : [];
  // 【逐行批注】 声明前端变量 finite，保存本行计算出的数据或对象。
  const finite = (value, fallback = 0) => Number.isFinite(Number(value)) ? Number(value) : fallback;
  // 【逐行批注】 声明前端变量 number，保存本行计算出的数据或对象。
  const number = value => Number(finite(value).toFixed(3)).toLocaleString('zh-CN', {maximumFractionDigits:3});
  // 【逐行批注】 定义函数 chatTime，封装可复用的前端行为。
  function chatTime(value) {
    // 【逐行批注】 声明前端变量 at，保存本行计算出的数据或对象。
    const at = new Date(value);
    // 【逐行批注】 判断条件 (Number.isNaN(at.getTime())) return '';，成立时执行对应逻辑。
    if (Number.isNaN(at.getTime())) return '';
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 结束当前函数并返回结果：return new Intl.DateTimeFormat('zh-CN', {。
      return new Intl.DateTimeFormat('zh-CN', {
        // 【逐行批注】 执行当前前端表达式：month:'2-digit', day:'2-digit', hour:'2-digit', minute:'2-digit', hour12:false,。
        month:'2-digit', day:'2-digit', hour:'2-digit', minute:'2-digit', hour12:false,
      // 【逐行批注】 执行当前前端表达式：}).format(at);。
      }).format(at);
    // 【逐行批注】 执行当前前端表达式：} catch (_) { return ''; }。
    } catch (_) { return ''; }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 safeLink，封装可复用的前端行为。
  function safeLink(value, local = false) {
    // 【逐行批注】 判断条件 (typeof value !== 'string' || /[\\\u0000-\u0020\u007f]/.test(val…，成立时执行对应逻辑。
    if (typeof value !== 'string' || /[\\\u0000-\u0020\u007f]/.test(value)) return '';
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 url，保存本行计算出的数据或对象。
      const url = new URL(value, location.origin);
      // 【逐行批注】 判断条件 (!['http:', 'https:'].includes(url.protocol) || url.username || …，成立时执行对应逻辑。
      if (!['http:', 'https:'].includes(url.protocol) || url.username || url.password) return '';
      // 【逐行批注】 结束当前函数并返回结果：return local && url.origin !== location.origin ? '' : url.href;。
      return local && url.origin !== location.origin ? '' : url.href;
    // 【逐行批注】 执行当前前端表达式：} catch (_) { return ''; }。
    } catch (_) { return ''; }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 lessonId，保存本行计算出的数据或对象。
  const lessonId = () => lesson?.id || scope.lessonId || '';
  // 【逐行批注】 声明前端变量 storageKey，保存本行计算出的数据或对象。
  const storageKey = name => `deep:${lessonId() || scope.courseId || ''}:${name}`;
  // 【逐行批注】 声明前端变量 store，保存本行计算出的数据或对象。
  const store = (name, value) => api?.storage.set(storageKey(name), String(value));
  // 【逐行批注】 声明前端变量 restore，保存本行计算出的数据或对象。
  const restore = name => api?.storage.get(storageKey(name)) || '';
  // 【逐行批注】 声明前端变量 preparationTaskKey，保存本行计算出的数据或对象。
  const preparationTaskKey = () => {
    // 【逐行批注】 声明前端变量 course，保存本行计算出的数据或对象。
    const course = scope.courseId || lesson?.courseId || saved?.course?.id || '';
    // 【逐行批注】 声明前端变量 topic，保存本行计算出的数据或对象。
    const topic = scope.topicId || lesson?.topicId || '';
    // 【逐行批注】 声明前端变量 phase，保存本行计算出的数据或对象。
    const phase = scope.phaseIndex || lesson?.phaseIndex || '';
    // 【逐行批注】 声明前端变量 topicIndex，保存本行计算出的数据或对象。
    const topicIndex = scope.topicIndex || lesson?.topicIndex || '';
    // 【逐行批注】 结束当前函数并返回结果：return `deep-preparation:${course}:${topic || 'position'}:${phase}:${…。
    return `deep-preparation:${course}:${topic || 'position'}:${phase}:${topicIndex}`;
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 rememberPreparationTask，保存本行计算出的数据或对象。
  const rememberPreparationTask = task => {
    // 【逐行批注】 判断条件 (/^agent-[0-9a-f]+$/i.test(String(task?.id || ''))) api?.storage…，成立时执行对应逻辑。
    if (/^agent-[0-9a-f]+$/i.test(String(task?.id || ''))) api?.storage.set(preparationTaskKey(), task.id);
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 rememberedPreparationTask，保存本行计算出的数据或对象。
  const rememberedPreparationTask = () => {
    // 【逐行批注】 声明前端变量 id，保存本行计算出的数据或对象。
    const id = api?.storage.get(preparationTaskKey()) || '';
    // 【逐行批注】 结束当前函数并返回结果：return /^agent-[0-9a-f]+$/i.test(id) ? id : '';。
    return /^agent-[0-9a-f]+$/i.test(id) ? id : '';
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 status，保存本行计算出的数据或对象。
  const status = message => {
    // 【逐行批注】 计算右侧结果并更新 taskMessage，改变页面或请求状态。
    taskMessage = message;
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = document.getElementById('dc-status');
    // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
    const text = node?.querySelector('[data-status-text]');
    // 【逐行批注】 判断条件 (text) text.textContent = message;，成立时执行对应逻辑。
    if (text) text.textContent = message;
    // 【逐行批注】 当前一条件不成立时继续判断：(node) node.textContent = message;。
    else if (node) node.textContent = message;
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 计算右侧结果并更新 try { fullscreenMode，改变页面或请求状态。
  try { fullscreenMode = sessionStorage.getItem(fullscreenSessionKey) === '1'; } catch (_) { /* 私密浏览环境仍可在当前页面使用全屏。 */ }
  // 【逐行批注】 定义函数 storeFullscreenMode，封装可复用的前端行为。
  function storeFullscreenMode(enabled) {
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 判断条件 (enabled) sessionStorage.setItem(fullscreenSessionKey, '1');，成立时执行对应逻辑。
      if (enabled) sessionStorage.setItem(fullscreenSessionKey, '1');
      // 【逐行批注】 执行前一条件不成立时的分支。
      else sessionStorage.removeItem(fullscreenSessionKey);
    // 【逐行批注】 执行当前前端表达式：} catch (_) { /* 仅影响跨页面保持，不影响当前页面全屏。 * / }。
    } catch (_) { /* 仅影响跨页面保持，不影响当前页面全屏。 */ }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 applyFullscreenUi，封装可复用的前端行为。
  function applyFullscreenUi() {
    // 【逐行批注】 执行当前前端表达式：document.documentElement.classList.toggle('gy-classroom-fullscreen', fullscreenMode);。
    document.documentElement.classList.toggle('gy-classroom-fullscreen', fullscreenMode);
    // 【逐行批注】 计算右侧结果并更新 mount.dataset.fullscreen，改变页面或请求状态。
    mount.dataset.fullscreen = String(fullscreenMode);
    // 【逐行批注】 声明前端变量 bar，保存本行计算出的数据或对象。
    const bar = document.getElementById('dc-fullscreen-bar');
    // 【逐行批注】 判断条件 (bar) bar.hidden = !fullscreenMode;，成立时执行对应逻辑。
    if (bar) bar.hidden = !fullscreenMode;
    // 【逐行批注】 声明前端变量 control，保存本行计算出的数据或对象。
    const control = document.getElementById('dc-fullscreen');
    // 【逐行批注】 判断条件 (control) {，成立时执行对应逻辑。
    if (control) {
      // 【逐行批注】 调用 String 执行页面交互或数据处理：control.setAttribute('aria-pressed', String(fullscreenMode));。
      control.setAttribute('aria-pressed', String(fullscreenMode));
      // 【逐行批注】 执行当前前端表达式：control.setAttribute('aria-label', fullscreenMode ? '退出全屏课堂' : '全屏显示课堂');。
      control.setAttribute('aria-label', fullscreenMode ? '退出全屏课堂' : '全屏显示课堂');
      // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
      const label = control.querySelector('span');
      // 【逐行批注】 判断条件 (label) label.textContent = fullscreenMode ? '退出全屏' : '全屏课堂';，成立时执行对应逻辑。
      if (label) label.textContent = fullscreenMode ? '退出全屏' : '全屏课堂';
      if (learningTools) control.querySelector('svg')?.replaceWith(icon(fullscreenMode ? 'collapse' : 'expand'));
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 调用 updatePanels 执行页面交互或数据处理：updatePanels();。
    updatePanels();
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 setSystemFullscreen，封装可复用的前端行为。
  async function setSystemFullscreen(enabled) {
    // 【逐行批注】 判断条件 (typeof window.gangyiSetClassroomFullscreen === 'function') {，成立时执行对应逻辑。
    if (typeof window.gangyiSetClassroomFullscreen === 'function') {
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 调用 Boolean 执行页面交互或数据处理：await window.gangyiSetClassroomFullscreen(Boolean(enabled));。
        await window.gangyiSetClassroomFullscreen(Boolean(enabled));
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
      // 【逐行批注】 执行当前前端表达式：} catch (_) { /* 浏览器打开时继续使用标准 Fullscreen API。 * / }。
      } catch (_) { /* 浏览器打开时继续使用标准 Fullscreen API。 */ }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 判断条件 (enabled && !document.fullscreenElement) await document.document…，成立时执行对应逻辑。
      if (enabled && !document.fullscreenElement) await document.documentElement.requestFullscreen?.();
      // 【逐行批注】 当前一条件不成立时继续判断：(!enabled && document.fullscreenElement) await document.exitFull…。
      else if (!enabled && document.fullscreenElement) await document.exitFullscreen?.();
    // 【逐行批注】 执行当前前端表达式：} catch (_) { /* 页面仍进入无干扰布局，退出按钮始终可用。 * / }。
    } catch (_) { /* 页面仍进入无干扰布局，退出按钮始终可用。 */ }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 enterClassroomFullscreen，封装可复用的前端行为。
  async function enterClassroomFullscreen() {
    workspaceContext?.close(false);
    // 【逐行批注】 判断条件 (fullscreenMode) return;，成立时执行对应逻辑。
    if (fullscreenMode) return;
    // 【逐行批注】 计算右侧结果并更新 chatOpenBeforeFullscreen，改变页面或请求状态。
    chatOpenBeforeFullscreen = mount.dataset.chatOpen || 'false';
    // 【逐行批注】 计算右侧结果并更新 fullscreenMode，改变页面或请求状态。
    fullscreenMode = true;
    // 【逐行批注】 计算右侧结果并更新 lastFullscreenCompact，改变页面或请求状态。
    lastFullscreenCompact = matchMedia('(max-width:1000px)').matches;
    // 【逐行批注】 调用 storeFullscreenMode 执行页面交互或数据处理：storeFullscreenMode(true);。
    storeFullscreenMode(true);
    // 【逐行批注】 调用 applyFullscreenUi 执行页面交互或数据处理：applyFullscreenUi();。
    applyFullscreenUi();
    // 【逐行批注】 调用 toggleChat、matchMedia 执行页面交互或数据处理：toggleChat(!matchMedia('(max-width:1000px)').matches, false);。
    toggleChat(!matchMedia('(max-width:1000px)').matches, false);
    // 【逐行批注】 调用 setSystemFullscreen 执行页面交互或数据处理：await setSystemFullscreen(true);。
    await setSystemFullscreen(true);
    // 【逐行批注】 执行当前前端表达式：document.getElementById('dc-fullscreen-exit')?.focus({preventScroll:true});。
    document.getElementById(learningTools ? 'dc-fullscreen' : 'dc-fullscreen-exit')?.focus({preventScroll:true});
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 exitClassroomFullscreen，封装可复用的前端行为。
  async function exitClassroomFullscreen() {
    // 【逐行批注】 判断条件 (!fullscreenMode) return;，成立时执行对应逻辑。
    if (!fullscreenMode) return;
    // 【逐行批注】 计算右侧结果并更新 fullscreenMode，改变页面或请求状态。
    fullscreenMode = false;
    // 【逐行批注】 计算右侧结果并更新 lastFullscreenCompact，改变页面或请求状态。
    lastFullscreenCompact = null;
    // 【逐行批注】 调用 storeFullscreenMode 执行页面交互或数据处理：storeFullscreenMode(false);。
    storeFullscreenMode(false);
    // 【逐行批注】 调用 applyFullscreenUi 执行页面交互或数据处理：applyFullscreenUi();。
    applyFullscreenUi();
    // 【逐行批注】 调用 setSystemFullscreen 执行页面交互或数据处理：await setSystemFullscreen(false);。
    await setSystemFullscreen(false);
    // 【逐行批注】 调用 toggleChat 执行页面交互或数据处理：toggleChat(chatOpenBeforeFullscreen === 'true', false);。
    toggleChat(chatOpenBeforeFullscreen === 'true', false);
    // 【逐行批注】 执行当前前端表达式：document.getElementById('dc-fullscreen')?.focus({preventScroll:true});。
    document.getElementById('dc-fullscreen')?.focus({preventScroll:true});
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 reduceMotion，保存本行计算出的数据或对象。
  const reduceMotion = () => Boolean(window.matchMedia?.('(prefers-reduced-motion: reduce)').matches);
  // 【逐行批注】 声明前端变量 scrollBehavior，保存本行计算出的数据或对象。
  const scrollBehavior = () => reduceMotion() ? 'auto' : 'smooth';
  // 【逐行批注】 定义函数 failureTask，封装可复用的前端行为。
  function failureTask(purpose, message, startedAt) {
    // 【逐行批注】 声明前端变量 at，保存本行计算出的数据或对象。
    const at = new Date().toISOString();
    // 【逐行批注】 结束当前函数并返回结果：return {id:'local-' + Date.now(), status:'failed', purpose, calls:0,。
    return {id:'local-' + Date.now(), status:'failed', purpose, calls:0,
      // 【逐行批注】 调用 Date 执行页面交互或数据处理：createdAt:new Date(startedAt || Date.now()).toISOString(), updatedAt:…。
      createdAt:new Date(startedAt || Date.now()).toISOString(), updatedAt:at,
      // 【逐行批注】 执行当前前端表达式：activity:{title:'本次请求未完成', detail:message, state:'failed', at},。
      activity:{title:'本次请求未完成', detail:message, state:'failed', at},
      // 【逐行批注】 执行当前前端表达式：failure:{message}, error:message, events:[]};。
      failure:{message}, error:message, events:[]};
  // 【逐行批注】 结束当前前端代码块。
  }
  // 提交请求本身失败时还没有服务端任务，构造同形失败任务让轨迹与页面状态一致。
  // 【逐行批注】 定义函数 feedLocalFailure，封装可复用的前端行为。
  function feedLocalFailure(purpose, message, startedAt, retry) {
    // 【逐行批注】 计算右侧结果并更新 localFailureRetry，改变页面或请求状态。
    localFailureRetry = typeof retry === 'function' ? retry : null;
    // 【逐行批注】 调用 failureTask 执行页面交互或数据处理：tracePanel?.update(failureTask(purpose, message, startedAt));。
    tracePanel?.update(failureTask(purpose, message, startedAt));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 retryTraceTask，封装可复用的前端行为。
  async function retryTraceTask() {
    // 【逐行批注】 判断条件 (String(tracePanel?.task?.id || '').startsWith('local-')) {，成立时执行对应逻辑。
    if (String(tracePanel?.task?.id || '').startsWith('local-')) {
      // 【逐行批注】 声明前端变量 retry，保存本行计算出的数据或对象。
      const retry = localFailureRetry; localFailureRetry = null;
      // 【逐行批注】 判断条件 (retry) return retry();，成立时执行对应逻辑。
      if (retry) return retry();
      // 【逐行批注】 调用 status 执行页面交互或数据处理：status('请求没有提交成功，请重新发起。');。
      status('请求没有提交成功，请重新发起。');
      // 【逐行批注】 结束当前函数并返回结果：return;。
      return;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 判断条件 (!lastTask?.id) return;，成立时执行对应逻辑。
    if (!lastTask?.id) return;
    // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
    const task = await api.control({command:'retry',taskId:lastTask.id});
    // 【逐行批注】 计算右侧结果并更新 watchingTask，改变页面或请求状态。
    watchingTask = ''; paintTask(task); observe(task.id);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 request，封装可复用的前端行为。
  async function request(path, data) {
    // 【逐行批注】 判断条件 (api?.request) return api.request(path, data);，成立时执行对应逻辑。
    if (api?.request) return api.request(path, data);
    // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
    const response = await fetch(path, data === undefined ? {cache:'no-store'} : {method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify(data)});
    // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
    const value = await response.json(); if (!response.ok) throw new Error(value.error || '请求未完成，请重试。'); return value;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 loadIndependentChat，封装可复用的前端行为。
  async function loadIndependentChat(force = false) {
    // 【逐行批注】 声明前端变量 currentLesson，保存本行计算出的数据或对象。
    const currentLesson = lesson?.id || scope.lessonId || '';
    // 【逐行批注】 判断条件 (!currentLesson) { independentChat = {lessonId:'', webEnabled:fa…，成立时执行对应逻辑。
    if (!currentLesson) { independentChat = {lessonId:'', webEnabled:false, messages:[], activeTask:null}; renderChat(); return; }
    // 【逐行批注】 判断条件 (!force && independentChatLesson === currentLesson && independen…，成立时执行对应逻辑。
    if (!force && independentChatLesson === currentLesson && independentChat.lessonId === currentLesson) return;
    // 【逐行批注】 声明前端变量 epoch，保存本行计算出的数据或对象。
    const epoch = ++independentChatEpoch;
    // 【逐行批注】 判断条件 (independentChatLesson && independentChatLesson !== currentLesso…，成立时执行对应逻辑。
    if (independentChatLesson && independentChatLesson !== currentLesson) {
      // 【逐行批注】 计算右侧结果并更新 stopIndependentWatch?.(); stopIndependentWat…，改变页面或请求状态。
      stopIndependentWatch?.(); stopIndependentWatch = null;
      // 【逐行批注】 计算右侧结果并更新 independentChatSeen，改变页面或请求状态。
      independentChatSeen = new Set(); independentChatStream = '';
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
      const value = await request('/api/independent-chat?' + new URLSearchParams({lessonId:currentLesson}));
      // 【逐行批注】 判断条件 (epoch !== independentChatEpoch || currentLesson !== (lesson?.id…，成立时执行对应逻辑。
      if (epoch !== independentChatEpoch || currentLesson !== (lesson?.id || scope.lessonId || '')) return;
      // 【逐行批注】 计算右侧结果并更新 stopIndependentWatch?.(); stopIndependentWat…，改变页面或请求状态。
      stopIndependentWatch?.(); stopIndependentWatch = null; independentChatSeen = new Set(); independentChatStream = '';
      // 【逐行批注】 计算右侧结果并更新 independentChatLesson，改变页面或请求状态。
      independentChatLesson = currentLesson; independentChat = value;
      // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
      const active = ['pending','running'].includes(value.activeTask?.status) && !value.activeTask?.paused;
      // 【逐行批注】 判断条件 (active) independentChatStreaming.add(currentLesson); else indep…，成立时执行对应逻辑。
      if (active) independentChatStreaming.add(currentLesson); else independentChatStreaming.delete(currentLesson);
      // 【逐行批注】 调用 renderChat 执行页面交互或数据处理：renderChat();。
      renderChat();
      // 【逐行批注】 判断条件 (value.activeTask?.id && ['pending','running','paused'].includes…，成立时执行对应逻辑。
      if (value.activeTask?.id && ['pending','running','paused'].includes(value.activeTask.status))
        // 【逐行批注】 调用 observeIndependentTask 执行页面交互或数据处理：observeIndependentTask(value.activeTask.id);。
        observeIndependentTask(value.activeTask.id);
    // 【逐行批注】 执行当前前端表达式：} catch (error) {。
    } catch (error) {
      // 【逐行批注】 判断条件 (epoch !== independentChatEpoch) return;，成立时执行对应逻辑。
      if (epoch !== independentChatEpoch) return;
      // 【逐行批注】 计算右侧结果并更新 independentChat，改变页面或请求状态。
      independentChat = {...independentChat, lessonId:currentLesson, error:error.message, messages:[], activeTask:null};
      // 【逐行批注】 执行当前前端表达式：independentChatStreaming.delete(currentLesson);。
      independentChatStreaming.delete(currentLesson);
      // 【逐行批注】 调用 renderChat 执行页面交互或数据处理：renderChat();。
      renderChat();
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 observeIndependentTask，封装可复用的前端行为。
  function observeIndependentTask(taskId) {
    // 【逐行批注】 计算右侧结果并更新 stopIndependentWatch?.(); independentChatSeen，改变页面或请求状态。
    stopIndependentWatch?.(); independentChatSeen = new Set(); independentChatStream = '';
    // 【逐行批注】 判断条件 (!api?.watch) return;，成立时执行对应逻辑。
    if (!api?.watch) return;
    // 【逐行批注】 声明前端变量 epoch，保存本行计算出的数据或对象。
    const epoch = independentChatEpoch, currentLesson = lesson?.id || scope.lessonId || '';
    // 【逐行批注】 计算右侧结果并更新 stopIndependentWatch，改变页面或请求状态。
    stopIndependentWatch = api.watch(taskId, {
      // 【逐行批注】 调用 onEvent 执行页面交互或数据处理：onEvent(event) {。
      onEvent(event) {
        // 【逐行批注】 判断条件 (epoch !== independentChatEpoch) return false;，成立时执行对应逻辑。
        if (epoch !== independentChatEpoch) return false;
        // 【逐行批注】 判断条件 (event.type === 'delta' && event.field === 'message') {，成立时执行对应逻辑。
        if (event.type === 'delta' && event.field === 'message') {
          // 【逐行批注】 判断条件 (!independentChatSeen.has(event.seq)) { independentChatSeen.add(…，成立时执行对应逻辑。
          if (!independentChatSeen.has(event.seq)) { independentChatSeen.add(event.seq); independentChatStream += event.text || ''; scheduleChatRender(); }
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
      // 【逐行批注】 执行当前前端表达式：},。
      },
      // 【逐行批注】 调用 onState 执行页面交互或数据处理：onState(task) {。
      onState(task) {
        // 【逐行批注】 判断条件 (epoch !== independentChatEpoch) return;，成立时执行对应逻辑。
        if (epoch !== independentChatEpoch) return;
        // 【逐行批注】 计算右侧结果并更新 independentChat.activeTask，改变页面或请求状态。
        independentChat.activeTask = task;
        // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
        const active = ['pending','running'].includes(task.status) && !task.paused;
        // 【逐行批注】 判断条件 (active) independentChatStreaming.add(currentLesson); else indep…，成立时执行对应逻辑。
        if (active) independentChatStreaming.add(currentLesson); else independentChatStreaming.delete(currentLesson);
        // 【逐行批注】 判断条件 (['pending','paused'].includes(task.status)) {，成立时执行对应逻辑。
        if (['pending','paused'].includes(task.status)) {
          // 【逐行批注】 声明前端变量 chunks，保存本行计算出的数据或对象。
          const chunks = (task.events || []).filter(item => item.type === 'delta' && item.field === 'message');
          // 【逐行批注】 计算右侧结果并更新 independentChatSeen，改变页面或请求状态。
          independentChatSeen = new Set(chunks.map(item => item.seq));
          // 【逐行批注】 计算右侧结果并更新 independentChatStream，改变页面或请求状态。
          independentChatStream = chunks.map(item => item.text || '').join('');
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 调用 syncChatAvailability、renderChat 执行页面交互或数据处理：syncChatAvailability(); renderChat();。
        syncChatAvailability(); renderChat();
        // 【逐行批注】 判断条件 (!['pending','running'].includes(task.status) && task.status !==…，成立时执行对应逻辑。
        if (!['pending','running'].includes(task.status) && task.status !== 'paused') loadIndependentChat(true);
      // 【逐行批注】 执行当前前端表达式：},。
      },
      // 【逐行批注】 调用 onConnectionError、renderChat 执行页面交互或数据处理：onConnectionError(message) { if (epoch === independentChatEpoch) { in…。
      onConnectionError(message) { if (epoch === independentChatEpoch) { independentChat.error = message; renderChat(); } },
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 sendIndependentChat，封装可复用的前端行为。
  async function sendIndependentChat(text, draft) {
    // 【逐行批注】 声明前端变量 currentLesson，保存本行计算出的数据或对象。
    const currentLesson = lesson?.id || scope.lessonId || '';
    // 【逐行批注】 判断条件 (!currentLesson || !text.trim() || independentChatSubmitting.has…，成立时执行对应逻辑。
    if (!currentLesson || !text.trim() || independentChatSubmitting.has(currentLesson) ||
        // 【逐行批注】 执行当前前端表达式：independentChatStreaming.has(currentLesson)) return;。
        independentChatStreaming.has(currentLesson)) return;
    // 【逐行批注】 声明前端变量 epoch，保存本行计算出的数据或对象。
    const epoch = independentChatEpoch;
    // 【逐行批注】 声明前端变量 requestId，保存本行计算出的数据或对象。
    const requestId = api?.id?.() || crypto.randomUUID(), startedAt = Date.now();
    // 【逐行批注】 执行当前前端表达式：independentChatSubmitting.add(currentLesson);。
    independentChatSubmitting.add(currentLesson);
    // 【逐行批注】 计算右侧结果并更新 independentChatStream，改变页面或请求状态。
    independentChatStream = ''; independentChatSeen = new Set(); independentChat.error = '';
    // 【逐行批注】 计算右侧结果并更新 independentChat.messages，改变页面或请求状态。
    independentChat.messages = [...(independentChat.messages || []), {id:'local-user:' + requestId, role:'user', content:text.trim(), status:'sending', sources:[]}];
    // 【逐行批注】 判断条件 (draft) { store(draft.name, ''); draft.node.value = ''; }，成立时执行对应逻辑。
    if (draft) { store(draft.name, ''); draft.node.value = ''; }
    // 【逐行批注】 调用 syncChatAvailability、renderChat、status 执行页面交互或数据处理：syncChatAvailability(); renderChat(); status('AI 聊天正在提交…');。
    syncChatAvailability(); renderChat(); status('AI 聊天正在提交…');
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
      const value = await request('/api/independent-chat/send', {lessonId:currentLesson, text:text.trim(), requestId, webEnabled:Boolean(independentChat.webEnabled)});
      // 【逐行批注】 判断条件 (epoch !== independentChatEpoch || currentLesson !== (lesson?.id…，成立时执行对应逻辑。
      if (epoch !== independentChatEpoch || currentLesson !== (lesson?.id || scope.lessonId || '')) return;
      // 【逐行批注】 计算右侧结果并更新 independentChat，改变页面或请求状态。
      independentChat = value.chat || independentChat;
      // 【逐行批注】 计算右侧结果并更新 independentChatLesson，改变页面或请求状态。
      independentChatLesson = currentLesson;
      // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
      const active = ['pending','running'].includes(value.task?.status);
      // 【逐行批注】 判断条件 (active) independentChatStreaming.add(currentLesson); else indep…，成立时执行对应逻辑。
      if (active) independentChatStreaming.add(currentLesson); else independentChatStreaming.delete(currentLesson);
      // 【逐行批注】 调用 renderChat 执行页面交互或数据处理：renderChat();。
      renderChat();
      // 【逐行批注】 判断条件 (value.task?.id) observeIndependentTask(value.task.id);，成立时执行对应逻辑。
      if (value.task?.id) observeIndependentTask(value.task.id);
      // 【逐行批注】 调用 status 执行页面交互或数据处理：status(value.task?.status === 'failed' ? '独立聊天未完成；消息已保留，可以重试。' : '独立聊…。
      status(value.task?.status === 'failed' ? '独立聊天未完成；消息已保留，可以重试。' : '独立聊天已在后台处理。');
    // 【逐行批注】 执行当前前端表达式：} catch (error) {。
    } catch (error) {
      // 【逐行批注】 判断条件 (epoch !== independentChatEpoch || currentLesson !== (lesson?.id…，成立时执行对应逻辑。
      if (epoch !== independentChatEpoch || currentLesson !== (lesson?.id || scope.lessonId || '')) return;
      // 【逐行批注】 执行当前前端表达式：independentChatStreaming.delete(currentLesson);。
      independentChatStreaming.delete(currentLesson);
      // 【逐行批注】 计算右侧结果并更新 independentChat.messages，改变页面或请求状态。
      independentChat.messages = independentChat.messages.map(item => item.id === 'local-user:' + requestId ? {...item, status:'failed'} : item);
      // 【逐行批注】 计算右侧结果并更新 independentChat.error，改变页面或请求状态。
      independentChat.error = error.message;
      // 【逐行批注】 调用 status 执行页面交互或数据处理：status(error.message);。
      status(error.message);
      // 【逐行批注】 调用 renderChat 执行页面交互或数据处理：renderChat();。
      renderChat();
    // 【逐行批注】 执行当前前端表达式：} finally {。
    } finally {
      // 【逐行批注】 执行当前前端表达式：independentChatSubmitting.delete(currentLesson);。
      independentChatSubmitting.delete(currentLesson);
      // 【逐行批注】 判断条件 (epoch === independentChatEpoch && currentLesson === (lesson?.id…，成立时执行对应逻辑。
      if (epoch === independentChatEpoch && currentLesson === (lesson?.id || scope.lessonId || '')) {
        // 【逐行批注】 调用 syncChatAvailability 执行页面交互或数据处理：syncChatAvailability();。
        syncChatAvailability();
        // 【逐行批注】 计算右侧结果并更新 refresh().catch(()，改变页面或请求状态。
        refresh().catch(()=>{});
        // 【逐行批注】 判断条件 (Date.now() - startedAt > 0) setDisabled();，成立时执行对应逻辑。
        if (Date.now() - startedAt > 0) setDisabled();
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 toggleIndependentWeb，封装可复用的前端行为。
  async function toggleIndependentWeb() {
    // 【逐行批注】 声明前端变量 currentLesson，保存本行计算出的数据或对象。
    const currentLesson = lesson?.id || scope.lessonId || '';
    // 【逐行批注】 判断条件 (!currentLesson || independentChatSubmitting.has(currentLesson) …，成立时执行对应逻辑。
    if (!currentLesson || independentChatSubmitting.has(currentLesson) || independentChatStreaming.has(currentLesson)) return;
    // 【逐行批注】 声明前端变量 webEnabled，保存本行计算出的数据或对象。
    const webEnabled = !independentChat.webEnabled;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
      const value = await request('/api/independent-chat/settings', {lessonId:currentLesson, webEnabled});
      // 【逐行批注】 计算右侧结果并更新 independentChat.webEnabled，改变页面或请求状态。
      independentChat.webEnabled = value.webEnabled;
      // 【逐行批注】 调用 renderChat 执行页面交互或数据处理：renderChat();。
      renderChat();
    // 【逐行批注】 计算右侧结果并更新 } catch (error) { independentChat.error，改变页面或请求状态。
    } catch (error) { independentChat.error = error.message; renderChat(); }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 clearIndependentChat，封装可复用的前端行为。
  async function clearIndependentChat() {
    // 【逐行批注】 声明前端变量 currentLesson，保存本行计算出的数据或对象。
    const currentLesson = lesson?.id || scope.lessonId || '';
    // 【逐行批注】 判断条件 (!currentLesson) return;，成立时执行对应逻辑。
    if (!currentLesson) return;
    // 【逐行批注】 判断条件 (!window.confirm('清空当前课时的独立聊天？此操作不会删除课程内容，但无法恢复聊天记录。')) return;，成立时执行对应逻辑。
    if (!window.confirm('清空当前课时的独立聊天？此操作不会删除课程内容，但无法恢复聊天记录。')) return;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 计算右侧结果并更新 stopIndependentWatch?.(); stopIndependentWat…，改变页面或请求状态。
      stopIndependentWatch?.(); stopIndependentWatch = null;
      // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
      const value = await request('/api/independent-chat/clear', {lessonId:currentLesson});
      // 【逐行批注】 计算右侧结果并更新 independentChat，改变页面或请求状态。
      independentChat = {...value, messages:value.messages || [], activeTask:null};
      // 【逐行批注】 执行当前前端表达式：independentChatSubmitting.delete(currentLesson); independentChatStreaming.delete(currentL…。
      independentChatSubmitting.delete(currentLesson); independentChatStreaming.delete(currentLesson);
      // 【逐行批注】 计算右侧结果并更新 independentChatStream，改变页面或请求状态。
      independentChatStream = ''; independentChatSeen = new Set();
      // 【逐行批注】 调用 syncChatAvailability、renderChat、status 执行页面交互或数据处理：syncChatAvailability(); renderChat(); status('当前课时的独立聊天已清空。');。
      syncChatAvailability(); renderChat(); status('当前课时的独立聊天已清空。');
    // 【逐行批注】 计算右侧结果并更新 } catch (error) { independentChat.error，改变页面或请求状态。
    } catch (error) { independentChat.error = error.message; renderChat(); }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 retryIndependentChat，封装可复用的前端行为。
  async function retryIndependentChat() {
    // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
    const task = independentChat.activeTask;
    // 【逐行批注】 判断条件 (!task?.id) return;，成立时执行对应逻辑。
    if (!task?.id) return;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
      const value = await api.control({command:'retry', taskId:task.id});
      // 【逐行批注】 计算右侧结果并更新 independentChat.activeTask，改变页面或请求状态。
      independentChat.activeTask = value;
      // 【逐行批注】 判断条件 (['pending','running'].includes(value.status)) independentChatSt…，成立时执行对应逻辑。
      if (['pending','running'].includes(value.status)) independentChatStreaming.add(lesson?.id || scope.lessonId || '');
      // 【逐行批注】 执行前一条件不成立时的分支。
      else independentChatStreaming.delete(lesson?.id || scope.lessonId || '');
      // 【逐行批注】 调用 syncChatAvailability、renderChat、observeIndependentTask 执行页面交互或数据处理：syncChatAvailability(); renderChat(); observeIndependentTask(value.id…。
      syncChatAvailability(); renderChat(); observeIndependentTask(value.id);
    // 【逐行批注】 计算右侧结果并更新 } catch (error) { independentChat.error，改变页面或请求状态。
    } catch (error) { independentChat.error = error.message; renderChat(); }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 controlIndependentChat，封装可复用的前端行为。
  async function controlIndependentChat(command) {
    // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
    const task = independentChat.activeTask;
    // 【逐行批注】 判断条件 (!task?.id || !api?.control) return;，成立时执行对应逻辑。
    if (!task?.id || !api?.control) return;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
      const value = await api.control({command, taskId:task.id});
      // 【逐行批注】 计算右侧结果并更新 independentChat.activeTask，改变页面或请求状态。
      independentChat.activeTask = value;
      // 【逐行批注】 声明前端变量 currentLesson，保存本行计算出的数据或对象。
      const currentLesson = lesson?.id || scope.lessonId || '';
      // 【逐行批注】 判断条件 (['pending','running'].includes(value.status) && !value.paused) …，成立时执行对应逻辑。
      if (['pending','running'].includes(value.status) && !value.paused) independentChatStreaming.add(currentLesson);
      // 【逐行批注】 执行前一条件不成立时的分支。
      else independentChatStreaming.delete(currentLesson);
      // 【逐行批注】 判断条件 (command === 'resume') observeIndependentTask(value.id);，成立时执行对应逻辑。
      if (command === 'resume') observeIndependentTask(value.id);
      // 【逐行批注】 调用 syncChatAvailability、renderChat 执行页面交互或数据处理：syncChatAvailability(); renderChat();。
      syncChatAvailability(); renderChat();
    // 【逐行批注】 计算右侧结果并更新 } catch (error) { independentChat.error，改变页面或请求状态。
    } catch (error) { independentChat.error = error.message; renderChat(); }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 skeleton，封装可复用的前端行为。
  function skeleton() {
    // 【逐行批注】 声明前端变量 box，保存本行计算出的数据或对象。
    const box = element('div', '', 'dc-empty'); box.setAttribute('role', 'status');
    // 【逐行批注】 调用 element 执行页面交互或数据处理：box.append(element('h2', '正在读取本节内容'), element('p', '恢复已保存的讲义、课件和学习位置。…。
    box.append(element('h2', '正在读取本节内容'), element('p', '恢复已保存的讲义、课件和学习位置。'));
    // 【逐行批注】 按循环规则遍历或重复处理：for (let i = 0; i < 3; i++) box.append(element('div', '', 'dc-skeleton'));。
    for (let i = 0; i < 3; i++) box.append(element('div', '', 'dc-skeleton'));
    // 【逐行批注】 结束当前函数并返回结果：return box;。
    return box;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 buildShell，封装可复用的前端行为。
  function buildShell() {
    // 【逐行批注】 执行当前前端表达式：mount.replaceChildren();。
    mount.replaceChildren();
    // 【逐行批注】 计算右侧结果并更新 mount.className，改变页面或请求状态。
    mount.className = 'deep-course-shell'; mount.dataset.directoryOpen = 'true'; mount.dataset.chatOpen = 'false';
    // 【逐行批注】 执行当前前端表达式：document.body.classList.add('deep-course-body');。
    document.body.classList.add('deep-course-body');
    // 【逐行批注】 声明前端变量 head，保存本行计算出的数据或对象。
    const head = element('header', '', 'dc-head');
    // 【逐行批注】 声明前端变量 heading，保存本行计算出的数据或对象。
    const heading = element('div'); heading.append(element('h1', '正在读取课堂'));
    // 【逐行批注】 计算右侧结果并更新 heading.querySelector('h1').id，改变页面或请求状态。
    heading.querySelector('h1').id = 'dc-title';
    // 【逐行批注】 声明前端变量 context，保存本行计算出的数据或对象。
    const context = element('p', '深度课程', 'dc-head-context'); context.id = 'dc-context'; heading.append(context);
    const mastery = element('details', '', 'dc-mastery'); mastery.id = 'dc-mastery'; mastery.hidden = true;
    const masterySummary = element('summary'); masterySummary.id = 'dc-mastery-summary';
    const masteryReason = element('p'); masteryReason.id = 'dc-mastery-reason';
    const masteryNext = element('p'); masteryNext.id = 'dc-mastery-next';
    mastery.append(masterySummary, masteryReason, masteryNext); heading.append(mastery);
    const topicEvidence = element('details', '', 'agent-topic-evidence');
    topicEvidence.id = 'dc-topic-evidence'; topicEvidence.hidden = true; heading.append(topicEvidence);
    const teachingPreferences = element('p', '', 'dc-teaching-preferences');
    teachingPreferences.id = 'dc-teaching-preferences'; teachingPreferences.hidden = true; heading.append(teachingPreferences);
    const teachingRound = element('p', '', 'dc-teaching-preferences');
    teachingRound.id = 'dc-teaching-round'; teachingRound.hidden = true; teachingRound.setAttribute('role', 'status');
    heading.append(teachingRound);
    // 【逐行批注】 声明前端变量 controls，保存本行计算出的数据或对象。
    const controls = element('div', '', 'dc-head-actions');
    // 【逐行批注】 声明前端变量 fullscreen，保存本行计算出的数据或对象。
    const fullscreen = button('全屏课堂', 'fullscreen', false, 'expand'); fullscreen.id = 'dc-fullscreen';
    // 【逐行批注】 执行当前前端表达式：fullscreen.setAttribute('aria-pressed', 'false'); fullscreen.setAttribute('aria-label', '…。
    fullscreen.setAttribute('aria-pressed', 'false'); fullscreen.setAttribute('aria-label', '全屏显示课堂');
    // 【逐行批注】 声明前端变量 directory，保存本行计算出的数据或对象。
    const directory = button('课程目录', 'directory', false, 'menu'); directory.id = 'dc-directory-toggle'; directory.setAttribute('aria-controls', 'dc-directory'); directory.setAttribute('aria-expanded', 'true');
    // 【逐行批注】 声明前端变量 answer，保存本行计算出的数据或对象。
    const answer = button('AI 聊天', 'chat', false, 'chat'); answer.id = 'dc-chat-toggle'; answer.setAttribute('aria-controls', 'dc-answer-panel'); answer.setAttribute('aria-expanded', 'false');
    // 【逐行批注】 执行当前前端表达式：controls.append(fullscreen, directory, answer); head.append(heading, controls);。
    const labsLink=element('a','互动实验','dc-button dc-labs-link');labsLink.href='/interactive-labs.html';labsLink.title='打开可自由操作的互动实验';
    const courseContextToggle = element('button', '本课上下文', 'dc-button gy-context-toggle');
    courseContextToggle.type = 'button';
    controls.append(fullscreen, directory, courseContextToggle, answer, labsLink); head.append(heading, controls);
    // 【逐行批注】 声明前端变量 fullscreenBar，保存本行计算出的数据或对象。
    const fullscreenBar = element('header', '', 'dc-fullscreen-bar'); fullscreenBar.id = 'dc-fullscreen-bar'; fullscreenBar.hidden = true;
    // 【逐行批注】 声明前端变量 fullscreenTitle，保存本行计算出的数据或对象。
    const fullscreenTitle = element('span', '深度课堂', 'dc-fullscreen-title'); fullscreenTitle.id = 'dc-fullscreen-title';
    // 【逐行批注】 声明前端变量 fullscreenTabs，保存本行计算出的数据或对象。
    const fullscreenTabs = element('nav', '', 'dc-fullscreen-tabs'); fullscreenTabs.setAttribute('role', 'tablist'); fullscreenTabs.setAttribute('aria-label', '全屏教学内容视图');
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [view, text, name] of [['lecture','讲义','book'], ['slides','课件',…。
    for (const [view, text, name] of [['lecture','讲义','book'], ['slides','课件','slides']]) {
      // 【逐行批注】 声明前端变量 tab，保存本行计算出的数据或对象。
      const tab = button(text, `view:${view}`, false, name); tab.className = 'dc-fs-tab'; tab.dataset.viewMode = view;
      // 【逐行批注】 计算右侧结果并更新 tab.id，改变页面或请求状态。
      tab.id = `dc-fs-tab-${view}`; tab.setAttribute('role', 'tab'); tab.setAttribute('aria-controls', 'dc-reading');
      // 【逐行批注】 调用 String 执行页面交互或数据处理：tab.setAttribute('aria-selected', String(view === currentView)); full…。
      tab.setAttribute('aria-selected', String(view === currentView)); fullscreenTabs.append(tab);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 声明前端变量 fullscreenActions，保存本行计算出的数据或对象。
    const fullscreenActions = element('div', '', 'dc-fullscreen-actions');
    // 【逐行批注】 声明前端变量 fullscreenChat，保存本行计算出的数据或对象。
    const fullscreenChat = button('聊天', 'fullscreen-chat', false, 'chat'); fullscreenChat.id = 'dc-fullscreen-chat';
    // 【逐行批注】 执行当前前端表达式：fullscreenChat.setAttribute('aria-controls', 'dc-answer-panel'); fullscreenChat.setAttrib…。
    fullscreenChat.setAttribute('aria-controls', 'dc-answer-panel'); fullscreenChat.setAttribute('aria-label', '展开 AI 聊天');
    // 【逐行批注】 声明前端变量 fullscreenExit，保存本行计算出的数据或对象。
    const fullscreenExit = button('退出全屏', 'fullscreen-exit', false, 'collapse'); fullscreenExit.id = 'dc-fullscreen-exit';
    // 【逐行批注】 执行当前前端表达式：fullscreenActions.append(fullscreenChat, fullscreenExit);。
    fullscreenActions.append(fullscreenChat, fullscreenExit);
    // 【逐行批注】 执行当前前端表达式：fullscreenBar.append(fullscreenTitle, fullscreenTabs, fullscreenActions);。
    fullscreenBar.append(fullscreenTitle, fullscreenTabs, fullscreenActions);
    // 【逐行批注】 声明前端变量 layout，保存本行计算出的数据或对象。
    const layout = element('div', '', 'dc-layout');
    // 【逐行批注】 声明前端变量 outline，保存本行计算出的数据或对象。
    const outline = element('aside', '', 'dc-directory'); outline.id = 'dc-directory'; outline.setAttribute('aria-label', '课程目录');
    // 【逐行批注】 声明前端变量 reader，保存本行计算出的数据或对象。
    const reader = element('section', '', 'dc-reader');
    // 【逐行批注】 声明前端变量 toolbar，保存本行计算出的数据或对象。
    const toolbar = element('div', '', 'dc-view-toolbar');
    // 【逐行批注】 声明前端变量 tabs，保存本行计算出的数据或对象。
    const tabs = element('nav', '', 'dc-tabs'); tabs.setAttribute('role', 'tablist'); tabs.setAttribute('aria-label', '教学内容视图');
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [view, text, name] of [['lecture','讲义','book'], ['slides','课件',…。
    for (const [view, text, name] of [['lecture','讲义','book'], ['slides','课件','slides']]) {
      // 【逐行批注】 声明前端变量 tab，保存本行计算出的数据或对象。
      const tab = button(text, `view:${view}`, false, name); tab.className = 'dc-tab'; tab.id = `dc-tab-${view}`;
      // 【逐行批注】 计算右侧结果并更新 tab.dataset.viewMode，改变页面或请求状态。
      tab.dataset.viewMode = view;
      // 【逐行批注】 调用 String 执行页面交互或数据处理：tab.setAttribute('role', 'tab'); tab.setAttribute('aria-selected', St…。
      tab.setAttribute('role', 'tab'); tab.setAttribute('aria-selected', String(view === currentView)); tab.setAttribute('aria-controls', 'dc-reading'); tab.tabIndex = view === currentView ? 0 : -1; tabs.append(tab);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 声明前端变量 savedLabel，保存本行计算出的数据或对象。
    const savedLabel = element('span', '已保存内容优先复用', 'dc-saved'); savedLabel.prepend(icon('check')); toolbar.append(tabs, savedLabel);
    // 【逐行批注】 声明前端变量 slideToolbar，保存本行计算出的数据或对象。
    const slideToolbar = element('nav', '', 'dc-slide-navigation'); slideToolbar.id = 'dc-slide-navigation'; slideToolbar.hidden = true; slideToolbar.setAttribute('aria-label', '课件翻页');
    // 【逐行批注】 声明前端变量 previousSlide，保存本行计算出的数据或对象。
    const previousSlide = button('上一页', 'previous', false, 'left'), nextSlide = button('下一页', 'next', false, 'right');
    // 【逐行批注】 计算右侧结果并更新 previousSlide.id，改变页面或请求状态。
    previousSlide.id = 'dc-slide-previous'; nextSlide.id = 'dc-slide-next';
    // 【逐行批注】 按循环规则遍历或重复处理：for (const control of [previousSlide, nextSlide]) control.setAttribute('ar…。
    for (const control of [previousSlide, nextSlide]) control.setAttribute('aria-controls', 'dc-reading');
    // 【逐行批注】 声明前端变量 slideCount，保存本行计算出的数据或对象。
    const slideCount = element('span', '', 'dc-slide-count'); slideCount.id = 'dc-slide-count'; slideCount.setAttribute('role', 'status'); slideCount.setAttribute('aria-live', 'polite'); slideCount.setAttribute('aria-atomic', 'true');
    // 【逐行批注】 执行当前前端表达式：slideToolbar.append(previousSlide, slideCount, nextSlide);。
    slideToolbar.append(previousSlide, slideCount, nextSlide);
    // 【逐行批注】 声明前端变量 traceHost，保存本行计算出的数据或对象。
    const traceHost = element('div'); traceHost.id = 'dc-trace-host';
    // 【逐行批注】 声明前端变量 reading，保存本行计算出的数据或对象。
    const reading = element('div', '', 'dc-reading'); reading.id = 'dc-reading'; reading.tabIndex = 0; reading.setAttribute('role', 'tabpanel'); reading.setAttribute('aria-labelledby', 'dc-tab-lecture'); reading.append(skeleton());
    // 【逐行批注】 声明前端变量 footer，保存本行计算出的数据或对象。
    const footer = element('footer', '', 'dc-footer'); const feedback = element('p', '', 'dc-footer-message'); feedback.id = 'dc-status'; feedback.setAttribute('role', 'status'); feedback.setAttribute('aria-live', 'polite');
    // 【逐行批注】 声明前端变量 spinner，保存本行计算出的数据或对象。
    const spinner = element('span', '', 'dc-activity-spinner'); spinner.id = 'dc-activity-spinner'; spinner.hidden = true; spinner.setAttribute('aria-hidden', 'true');
    // 【逐行批注】 声明前端变量 feedbackText，保存本行计算出的数据或对象。
    const feedbackText = element('span', '正在读取课堂…', 'dc-status-text'); feedbackText.setAttribute('data-status-text', '');
    // 【逐行批注】 执行当前前端表达式：feedback.append(spinner, feedbackText);。
    feedback.append(spinner, feedbackText);
    // 【逐行批注】 声明前端变量 footerControls，保存本行计算出的数据或对象。
    const footerControls = element('div', '', 'dc-footer-controls');
    // 【逐行批注】 声明前端变量 more，保存本行计算出的数据或对象。
    const more = element('details', '', 'dc-more'); const summary = element('summary', '更多', 'dc-button'); summary.prepend(icon('more'));
    // 【逐行批注】 声明前端变量 menu，保存本行计算出的数据或对象。
    const menu = element('div', '', 'dc-more-list');
    const reviseRound = button('调整本轮', 'revise-round');
    reviseRound.hidden = true; menu.append(reviseRound);
    menu.append(button('检查本节完成情况', 'finish'), button('重试失败素材', 'retry-media'));
    // 【逐行批注】 执行当前前端表达式：more.append(summary, menu);。
    more.append(summary, menu);
    // 【逐行批注】 声明前端变量 stop，保存本行计算出的数据或对象。
    const stop = button('停止', 'stop'); stop.id = 'dc-stop'; stop.hidden = true;
    // 【逐行批注】 声明前端变量 retry，保存本行计算出的数据或对象。
    const retry = button('重试', 'retry'); retry.id = 'dc-retry'; retry.hidden = true;
    // 【逐行批注】 声明前端变量 enter，保存本行计算出的数据或对象。
    const enter = button('重新进入', 'enter'); enter.id = 'dc-enter'; enter.hidden = true;
    // 【逐行批注】 声明前端变量 next，保存本行计算出的数据或对象。
    const next = button('继续学习', 'continue', true, 'arrow'); next.id = 'dc-continue';
    // 【逐行批注】 执行当前前端表达式：footerControls.append(stop, retry, enter, more, next); footer.append(feedback, footerCont…。
    footerControls.append(stop, retry, enter, more, next); footer.append(feedback, footerControls);
    // 【逐行批注】 声明前端变量 courseReply，保存本行计算出的数据或对象。
    const courseReply = element('form', '', 'dc-course-reply'); courseReply.id = 'dc-course-reply-form';
    // 【逐行批注】 声明前端变量 courseReplyLabel，保存本行计算出的数据或对象。
    const courseReplyLabel = element('label', '课程回答', 'dc-course-reply-label'); courseReplyLabel.htmlFor = 'dc-course-reply-input';
    // 【逐行批注】 声明前端变量 courseReplyField，保存本行计算出的数据或对象。
    const courseReplyField = element('div', '', 'dc-course-reply-field');
    // 【逐行批注】 声明前端变量 courseReplyInput，保存本行计算出的数据或对象。
    const courseReplyInput = element('textarea'); courseReplyInput.id = 'dc-course-reply-input'; courseReplyInput.rows = 1;
    // 【逐行批注】 计算右侧结果并更新 courseReplyInput.placeholder，改变页面或请求状态。
    courseReplyInput.placeholder = '回复 AI 导师，或输入你的想法'; courseReplyInput.setAttribute('aria-describedby', 'dc-course-reply-hint');
    // 【逐行批注】 为页面事件绑定处理逻辑：courseReplyInput.addEventListener('input', () => { store('course-reply-draft', cours…。
    courseReplyInput.addEventListener('input', () => { store('course-reply-draft', courseReplyInput.value); syncCourseReplyAvailability(); });
    // 【逐行批注】 声明前端变量 courseReplySend，保存本行计算出的数据或对象。
    const courseReplySend = button('', 'course-reply-send', true, 'send'); courseReplySend.type = 'submit'; courseReplySend.id = 'dc-course-reply-send'; courseReplySend.setAttribute('aria-label', '发送课程回答');
    // 【逐行批注】 执行当前前端表达式：courseReplyField.append(courseReplyInput); courseReply.append(courseReplyLabel, courseRep…。
    courseReplyField.append(courseReplyInput); courseReply.append(courseReplyLabel, courseReplyField, courseReplySend);
    // 【逐行批注】 声明前端变量 courseReplyHint，保存本行计算出的数据或对象。
    const courseReplyHint = element('span', '发送到当前课程 AI', 'dc-course-reply-hint'); courseReplyHint.id = 'dc-course-reply-hint';
    // 【逐行批注】 执行当前前端表达式：reader.append(toolbar, slideToolbar, traceHost, reading, courseReply, footer);。
    reader.append(toolbar, slideToolbar, traceHost, reading, courseReply, footer);
    // 【逐行批注】 声明前端变量 answerPanel，保存本行计算出的数据或对象。
    const answerPanel = element('aside', '', 'dc-answer-panel'); answerPanel.id = 'dc-answer-panel'; answerPanel.setAttribute('aria-label', 'AI 聊天'); answerPanel.append(buildChat());
    // 【逐行批注】 声明前端变量 chatDock，保存本行计算出的数据或对象。
    const chatDock = button('AI 聊天', 'fullscreen-chat', true, 'chat'); chatDock.id = 'dc-chat-dock'; chatDock.className += ' dc-chat-dock'; chatDock.hidden = true;
    // 【逐行批注】 声明前端变量 chatScrim，保存本行计算出的数据或对象。
    const chatScrim = element('button', '', 'dc-chat-scrim'); chatScrim.type = 'button'; chatScrim.dataset.action = 'close-fullscreen-chat'; chatScrim.setAttribute('aria-label', '关闭 AI 聊天');
    // 【逐行批注】 执行当前前端表达式：layout.append(outline, reader, answerPanel);。
    const courseContextPanel = element('aside');
    courseContextPanel.id = 'dc-course-context';
    courseContextPanel.hidden = true;
    layout.append(outline, reader, answerPanel, courseContextPanel);
    workspaceContext = window.GangyiWorkspace?.contextPanel(courseContextToggle, courseContextPanel,
      [mastery, topicEvidence, teachingPreferences, traceHost], open => {
        if (open && mount.dataset.chatOpen === 'true') toggleChat(false, false);
        mount.dataset.contextOpen = String(open);
      });
    // 【逐行批注】 执行当前前端表达式：mount.append(head, fullscreenBar, layout, chatDock, chatScrim);。
    mount.append(head, fullscreenBar, layout, chatDock, chatScrim);
    // 【逐行批注】 声明前端变量 outlineDialog，保存本行计算出的数据或对象。
    const outlineDialog = element('dialog', '', 'dc-dialog'); outlineDialog.id = 'dc-outline-dialog'; outlineDialog.setAttribute('aria-label', '课程目录');
    // 【逐行批注】 声明前端变量 dialogHead，保存本行计算出的数据或对象。
    const dialogHead = element('div', '', 'dc-dialog-head'); dialogHead.append(element('span', '课程目录'), button('关闭', 'close-directory')); outlineDialog.append(dialogHead, element('div', '', 'dc-directory'));
    // 【逐行批注】 声明前端变量 chatDialog，保存本行计算出的数据或对象。
    const chatDialog = element('dialog', '', 'dc-dialog dc-chat-dialog'); chatDialog.id = 'dc-chat-dialog'; chatDialog.setAttribute('aria-label', 'AI 聊天');
    // 【逐行批注】 执行当前前端表达式：mount.append(outlineDialog, chatDialog);。
    mount.append(outlineDialog, chatDialog);
    learningTools = window.GangyiWorkspace?.compactToolbar(heading.querySelector('h1'));
    if (learningTools) {
      tabs.dataset.menuKeepOpen = '';
      slideToolbar.dataset.menuKeepOpen = '';
      menu.className = 'gy-learning-actions';
      learningTools.panel.classList.add('gy-deep-menu');
      learningTools.toggle.before(fullscreen);
      fullscreen.addEventListener('click', handleClick);
      learningTools.append(tabs, slideToolbar, directory, courseContextToggle, answer, next,
        learningTools.disclose('课堂操作', menu, labsLink),
        learningTools.disclose('本课说明与教学过程', context, mastery, topicEvidence, teachingPreferences, traceHost));
      reader.insertBefore(teachingRound, courseReply);
      head.remove();
      toolbar.remove();
      more.remove();
      learningTools.panel.addEventListener('click', handleClick);
      learningTools.panel.addEventListener('keydown', handleKey);
      outlineDialog.addEventListener('close', () => {
        updatePanels();
        learningTools.toggle.focus({preventScroll:true});
      });
    }
    // 【逐行批注】 为页面事件绑定处理逻辑：chatDialog.addEventListener('close', () => { const room = chatDialog.querySelector('…。
    chatDialog.addEventListener('close', () => { const room = chatDialog.querySelector('.dc-chat-room'); if (room) answerPanel.append(room); mount.dataset.chatOpen = 'false'; updatePanels(); });
    // 【逐行批注】 为页面事件绑定处理逻辑：reading.addEventListener('scroll', () => {。
    reading.addEventListener('scroll', () => {
      // 【逐行批注】 调用 store、exposeVisible 执行页面交互或数据处理：readingPositions.set(currentView, reading.scrollTop); store(`scroll:$…。
      readingPositions.set(currentView, reading.scrollTop); store(`scroll:${currentView}`, reading.scrollTop); exposeVisible();
    // 【逐行批注】 执行当前前端表达式：}, {passive:true});。
    }, {passive:true});
    // 【逐行批注】 调用 ResizeObserver 执行页面交互或数据处理：new ResizeObserver(exposeVisible).observe(reading);。
    new ResizeObserver(exposeVisible).observe(reading);
    // 【逐行批注】 为页面事件绑定处理逻辑：mount.addEventListener('click', handleClick);。
    mount.addEventListener('click', handleClick);
    // 【逐行批注】 为页面事件绑定处理逻辑：mount.addEventListener('submit', handleSubmit);。
    mount.addEventListener('submit', handleSubmit);
    // 【逐行批注】 为页面事件绑定处理逻辑：mount.addEventListener('keydown', handleKey);。
    mount.addEventListener('keydown', handleKey);
    // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('keydown', event => {。
    document.addEventListener('keydown', event => {
      // 【逐行批注】 执行当前前端表达式：if(event.key!=='Escape'||!fullscreenMode)return;。
      if(event.key!=='Escape'||!fullscreenMode)return;
      // 【逐行批注】 执行当前前端表达式：if(document.querySelector('dialog[open]'))return;。
      if(document.querySelector('dialog[open], #learning-workspace-menu:popover-open'))return;
      // 【逐行批注】 执行当前前端表达式：event.preventDefault();。
      event.preventDefault();
      // 【逐行批注】 调用 exitClassroomFullscreen 执行页面交互或数据处理：exitClassroomFullscreen();。
      exitClassroomFullscreen();
    // 【逐行批注】 执行当前前端表达式：}, true);。
    }, true);
    // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('resize', () => {。
    window.addEventListener('resize', () => {
      // 【逐行批注】 声明前端变量 compact，保存本行计算出的数据或对象。
      const compact = fullscreenMode && matchMedia('(max-width:1000px)').matches;
      // 【逐行批注】 判断条件 (fullscreenMode && lastFullscreenCompact !== null && compact !==…，成立时执行对应逻辑。
      if (fullscreenMode && lastFullscreenCompact !== null && compact !== lastFullscreenCompact)
        // 【逐行批注】 调用 toggleChat 执行页面交互或数据处理：toggleChat(!compact, false);。
        toggleChat(!compact, false);
      // 【逐行批注】 判断条件 (fullscreenMode) lastFullscreenCompact = compact;，成立时执行对应逻辑。
      if (fullscreenMode) lastFullscreenCompact = compact;
      // 【逐行批注】 调用 updatePanels 执行页面交互或数据处理：updatePanels();。
      updatePanels();
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 buildChat，封装可复用的前端行为。
  function buildChat() {
    // 【逐行批注】 声明前端变量 room，保存本行计算出的数据或对象。
    const room = element('section', '', 'dc-chat-room'); room.id = 'dc-chat-room';
    // 【逐行批注】 声明前端变量 head，保存本行计算出的数据或对象。
    const head = element('header', '', 'dc-chat-header'); const heading = element('div'); heading.append(element('h2', 'AI 聊天'), element('p', '独立对话，不使用课程内容或学习记录。'));
    // 【逐行批注】 声明前端变量 headerActions，保存本行计算出的数据或对象。
    const headerActions = element('div', '', 'dc-chat-header-actions');
    // 【逐行批注】 声明前端变量 clear，保存本行计算出的数据或对象。
    const clear = button('', 'clear-chat', false, 'trash'); clear.classList.add('dc-icon-button'); clear.setAttribute('aria-label', '清空当前会话');
    // 【逐行批注】 声明前端变量 close，保存本行计算出的数据或对象。
    const close = button('', 'close-chat', false, 'close'); close.setAttribute('aria-label', '收起 AI 聊天'); close.classList.add('dc-icon-button');
    // 【逐行批注】 执行当前前端表达式：headerActions.append(clear, close); head.append(heading, headerActions);。
    headerActions.append(clear, close); head.append(heading, headerActions);
    // 【逐行批注】 声明前端变量 scroll，保存本行计算出的数据或对象。
    const scroll = element('div', '', 'dc-chat-scroll'); scroll.id = 'dc-chat-scroll'; scroll.setAttribute('role', 'log'); scroll.setAttribute('aria-label', '当前课时的独立聊天记录'); scroll.append(element('p', '可以询问任何问题。聊天只保存在当前课时，不会进入课程、画像或学习记录。', 'dc-chat-empty'));
    // 【逐行批注】 声明前端变量 form，保存本行计算出的数据或对象。
    const form = element('form', '', 'dc-chat-composer'); form.id = 'dc-chat-form'; const label = element('label', '向 AI 提问'); label.htmlFor = 'dc-chat-input';
    // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
    const row = element('div', '', 'dc-chat-input-row'); const input = element('textarea'); input.id = 'dc-chat-input'; input.rows = 2; input.placeholder = '输入你想问的任意问题';
    // 【逐行批注】 为页面事件绑定处理逻辑：input.addEventListener('input', () => { store('chat-draft', input.value); syncChatAv…。
    input.addEventListener('input', () => { store('chat-draft', input.value); syncChatAvailability(); });
    // 【逐行批注】 声明前端变量 web，保存本行计算出的数据或对象。
    const web = button('联网', 'toggle-web', false, 'globe'); web.id = 'dc-web-toggle'; web.setAttribute('aria-pressed', 'false'); web.setAttribute('aria-label', '开启联网搜索');
    // 【逐行批注】 声明前端变量 send，保存本行计算出的数据或对象。
    const send = button('', 'send', true, 'send'); send.type = 'submit'; send.setAttribute('aria-label', '发送问题'); row.append(input, web, send);
    // 【逐行批注】 按循环规则遍历或重复处理：form.append(label, row, element('p', '这里不会读取课程、作答、画像或长期记忆；联网开启后由 AI 判断是否需要…。
    form.append(label, row, element('p', '这里不会读取课程、作答、画像或长期记忆；联网开启后由 AI 判断是否需要搜索。', 'dc-chat-note'));
    // 【逐行批注】 执行当前前端表达式：room.append(head, scroll, form); return room;。
    room.append(head, scroll, form); return room;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 updatePanels，封装可复用的前端行为。
  function updatePanels() {
    // 【逐行批注】 声明前端变量 small，保存本行计算出的数据或对象。
    const small = matchMedia('(max-width:700px)').matches;
    // 【逐行批注】 声明前端变量 compactFullscreen，保存本行计算出的数据或对象。
    const compactFullscreen = fullscreenMode && matchMedia('(max-width:1000px)').matches;
    // 【逐行批注】 声明前端变量 chatDialog，保存本行计算出的数据或对象。
    const chatDialog = document.getElementById('dc-chat-dialog');
    // 【逐行批注】 判断条件 ((!small || compactFullscreen) && chatDialog.open) {，成立时执行对应逻辑。
    if ((!small || compactFullscreen) && chatDialog.open) {
      // 【逐行批注】 执行当前前端表达式：chatDialog.close();。
      chatDialog.close();
      // 【逐行批注】 判断条件 (compactFullscreen) mount.dataset.chatOpen = 'true';，成立时执行对应逻辑。
      if (compactFullscreen) mount.dataset.chatOpen = 'true';
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 声明前端变量 directoryShown，保存本行计算出的数据或对象。
    const directoryShown = !fullscreenMode && !small && mount.dataset.directoryOpen === 'true';
    // 【逐行批注】 调用 String 执行页面交互或数据处理：document.getElementById('dc-directory-toggle').setAttribute('aria-exp…。
    document.getElementById('dc-directory-toggle').setAttribute('aria-expanded', String(directoryShown || document.getElementById('dc-outline-dialog').open));
    // 【逐行批注】 声明前端变量 chatExpanded，保存本行计算出的数据或对象。
    const chatExpanded = mount.dataset.chatOpen === 'true' || chatDialog.open;
    // 【逐行批注】 调用 String 执行页面交互或数据处理：document.getElementById('dc-chat-toggle').setAttribute('aria-expanded…。
    document.getElementById('dc-chat-toggle').setAttribute('aria-expanded', String(chatExpanded));
    // 【逐行批注】 声明前端变量 fullscreenChat，保存本行计算出的数据或对象。
    const fullscreenChat = document.getElementById('dc-fullscreen-chat');
    // 【逐行批注】 判断条件 (fullscreenChat) {，成立时执行对应逻辑。
    if (fullscreenChat) {
      // 【逐行批注】 调用 String 执行页面交互或数据处理：fullscreenChat.setAttribute('aria-expanded', String(chatExpanded));。
      fullscreenChat.setAttribute('aria-expanded', String(chatExpanded));
      // 【逐行批注】 执行当前前端表达式：fullscreenChat.setAttribute('aria-label', chatExpanded ? '收起 AI 聊天' : '展开 AI 聊天');。
      fullscreenChat.setAttribute('aria-label', chatExpanded ? '收起 AI 聊天' : '展开 AI 聊天');
      // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
      const text = fullscreenChat.querySelector('span');
      // 【逐行批注】 判断条件 (text) text.textContent = chatExpanded ? '收起聊天' : '聊天';，成立时执行对应逻辑。
      if (text) text.textContent = chatExpanded ? '收起聊天' : '聊天';
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 声明前端变量 dock，保存本行计算出的数据或对象。
    const dock = document.getElementById('dc-chat-dock'); if (dock) dock.hidden = !fullscreenMode || chatExpanded;
    // 【逐行批注】 声明前端变量 scrim，保存本行计算出的数据或对象。
    const scrim = document.querySelector('.dc-chat-scrim'); if (scrim) scrim.hidden = !(fullscreenMode && compactFullscreen && chatExpanded);
    // 【逐行批注】 计算右侧结果并更新 mount.dataset.overlayChat，改变页面或请求状态。
    mount.dataset.overlayChat = String(fullscreenMode && compactFullscreen && chatExpanded);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 toggleChat，封装可复用的前端行为。
  function toggleChat(open, focus = true) {
    if (open) workspaceContext?.close(false);
    // 【逐行批注】 声明前端变量 dialog，保存本行计算出的数据或对象。
    const dialog = document.getElementById('dc-chat-dialog');
    // 【逐行批注】 声明前端变量 compactFullscreen，保存本行计算出的数据或对象。
    const compactFullscreen = fullscreenMode && matchMedia('(max-width:1000px)').matches;
    // 【逐行批注】 判断条件 (!compactFullscreen && matchMedia('(max-width:700px)').matches) {，成立时执行对应逻辑。
    if (!compactFullscreen && matchMedia('(max-width:700px)').matches) {
      // 【逐行批注】 判断条件 (open && !dialog.open) { dialog.append(document.getElementById('…，成立时执行对应逻辑。
      if (open && !dialog.open) { dialog.append(document.getElementById('dc-chat-room')); dialog.showModal(); }
      // 【逐行批注】 当前一条件不成立时继续判断：(!open && dialog.open) dialog.close();。
      else if (!open && dialog.open) dialog.close();
    // 【逐行批注】 执行当前前端表达式：} else {。
    } else {
      // 【逐行批注】 判断条件 (dialog.open) dialog.close();，成立时执行对应逻辑。
      if (dialog.open) dialog.close();
      // 【逐行批注】 声明前端变量 room，保存本行计算出的数据或对象。
      const room = document.getElementById('dc-chat-room'), panel = document.getElementById('dc-answer-panel');
      // 【逐行批注】 判断条件 (room && panel && room.parentElement !== panel) panel.append(roo…，成立时执行对应逻辑。
      if (room && panel && room.parentElement !== panel) panel.append(room);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 mount.dataset.chatOpen，改变页面或请求状态。
    mount.dataset.chatOpen = String(open); store('chat-open', open ? '1' : '0'); updatePanels();
    // 【逐行批注】 判断条件 (open && focus) document.getElementById('dc-chat-input').focus({…，成立时执行对应逻辑。
    if (open && focus) document.getElementById('dc-chat-input').focus({preventScroll:true});
    // 【逐行批注】 调用 requestAnimationFrame 执行页面交互或数据处理：requestAnimationFrame(exposeVisible);。
    requestAnimationFrame(exposeVisible);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderOutline，封装可复用的前端行为。
  function renderOutline() {
    // 【逐行批注】 声明前端变量 container，保存本行计算出的数据或对象。
    const container = document.getElementById('dc-directory'); container.replaceChildren();
    // 【逐行批注】 调用 element 执行页面交互或数据处理：container.append(element('h2', saved?.course?.title || '课程目录'), eleme…。
    container.append(element('h2', saved?.course?.title || '课程目录'), element('p', '沿课程主线，逐课探索。'));
    // 【逐行批注】 声明前端变量 stages，保存本行计算出的数据或对象。
    const stages = items(saved?.outline?.courseStructure);
    // 【逐行批注】 判断条件 (!stages.length) container.append(element('p', '课程目录暂未读取，请重试。'));，成立时执行对应逻辑。
    if (!stages.length) container.append(element('p', '课程目录暂未读取，请重试。'));
    // 【逐行批注】 计算右侧结果并更新 stages.forEach((stage, phaseIndex)，改变页面或请求状态。
    stages.forEach((stage, phaseIndex) => {
      // 【逐行批注】 声明前端变量 group，保存本行计算出的数据或对象。
      const group = element('details', '', 'dc-outline-stage'); group.open = phaseIndex + 1 === Number(lesson?.phaseIndex || scope.phaseIndex || 1);
      // 【逐行批注】 调用 element 执行页面交互或数据处理：group.append(element('summary', stage.stage || stage.name || '学习阶段'));。
      group.append(element('summary', stage.stage || stage.name || '学习阶段'));
      // 【逐行批注】 计算右侧结果并更新 items(stage.topics).forEach((topic, topicInd…，改变页面或请求状态。
      items(stage.topics).forEach((topic, topicIndex) => {
        // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
        const value = typeof topic === 'string' ? {title:topic} : topic;
        // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
        const active = lesson?.topicId || scope.topicId ? value.id === (lesson?.topicId || scope.topicId) : Number(lesson?.phaseIndex || scope.phaseIndex || 1) === Number(value.legacyPhaseIndex) && Number(lesson?.topicIndex || scope.topicIndex || 1) === Number(value.legacyTopicIndex);
        // 【逐行批注】 声明前端变量 control，保存本行计算出的数据或对象。
        const control = button('', 'topic'); control.className = 'dc-outline-topic';
        // 【逐行批注】 调用 Number、String 执行页面交互或数据处理：if(Number.isSafeInteger(Number(value.legacyPhaseIndex))&&Number(value…。
        if(Number.isSafeInteger(Number(value.legacyPhaseIndex))&&Number(value.legacyPhaseIndex)>0)control.dataset.phaseIndex=String(value.legacyPhaseIndex);
        // 【逐行批注】 调用 Number、String 执行页面交互或数据处理：if(Number.isSafeInteger(Number(value.legacyTopicIndex))&&Number(value…。
        if(Number.isSafeInteger(Number(value.legacyTopicIndex))&&Number(value.legacyTopicIndex)>0)control.dataset.topicIndex=String(value.legacyTopicIndex);
        // 【逐行批注】 判断条件 (value.id) control.dataset.topicId = value.id;，成立时执行对应逻辑。
        if (value.id) control.dataset.topicId = value.id;
        // 【逐行批注】 判断条件 (active) {control.setAttribute('aria-current', 'page');group.ope…，成立时执行对应逻辑。
        if (active) {control.setAttribute('aria-current', 'page');group.open=true;}
        // 【逐行批注】 调用 element 执行页面交互或数据处理：control.append(element('i', '', 'dc-topic-mark'), element('span', val…。
        control.append(element('i', '', 'dc-topic-mark'), element('span', value.title || value.name || '知识点')); group.append(control);
      // 【逐行批注】 执行当前前端表达式：}); container.append(group);。
      }); container.append(group);
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 【逐行批注】 声明前端变量 dialogBody，保存本行计算出的数据或对象。
    const dialogBody = document.querySelector('#dc-outline-dialog>.dc-directory');
    // 【逐行批注】 计算右侧结果并更新 dialogBody.replaceChildren(...[...container.…，改变页面或请求状态。
    dialogBody.replaceChildren(...[...container.children].map(node => node.cloneNode(true)));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 lessonQuestions，保存本行计算出的数据或对象。
  const lessonQuestions = () => items(lesson?.sections).filter(section => section.kind === 'question' && section.legacyKind !== 'example');
  // 【逐行批注】 定义函数 evaluationsFor，封装可复用的前端行为。
  function evaluationsFor(section) {
    // 仅识别同题已发布的真实评价；低分或需要澄清仍交由主控判断，不在前端设置通过线。
    // 【逐行批注】 结束当前函数并返回结果：return items(lesson?.evaluations).filter(value => value.sectionId ===…。
    return items(lesson?.evaluations).filter(value => value.sectionId === section.id && value.id && value.interactionId &&
      // 【逐行批注】 执行当前前端表达式：typeof value.credible === 'boolean' && (!value.status || value.status === 'valid') &&。
      typeof value.credible === 'boolean' && (!value.status || value.status === 'valid') &&
      // 【逐行批注】 执行当前前端表达式：(!value.questionId || !section.questionId || value.questionId === section.questionId));。
      (!value.questionId || !section.questionId || value.questionId === section.questionId));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 teachingRecords，封装可复用的前端行为。
  function teachingRecords() {
    // 【逐行批注】 声明前端变量 timeline，保存本行计算出的数据或对象。
    const timeline=items(lesson?.timeline).filter(item=>item.type==='message');
    // 【逐行批注】 声明前端变量 combined，保存本行计算出的数据或对象。
    const combined=new Map();
    // 【逐行批注】 按循环规则遍历或重复处理：for(const [index,record] of [...timeline,...pendingMessages].entries()){。
    for(const [index,record] of [...timeline,...pendingMessages].entries()){
      // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
      const text=String(record.text||record.message||'');if(!text.trim())continue;
      // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
      const key=record.taskId?`${record.taskId}:${record.role}:${record.step??0}`:record.id||`saved:${index}`;
      // 【逐行批注】 声明前端变量 previous，保存本行计算出的数据或对象。
      const previous=combined.get(key);
      // 【逐行批注】 调用 String 执行页面交互或数据处理：if(previous&&(!record.partial||String(previous.text||'').length>=text…。
      if(previous&&(!record.partial||String(previous.text||'').length>=text.length))continue;
      // 【逐行批注】 执行当前前端表达式：combined.set(key,{...record,text,displayKey:key});。
      combined.set(key,{...record,text,displayKey:key});
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return combined;。
    return combined;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 teachingThreads，封装可复用的前端行为。
  function teachingThreads() {
    // 【逐行批注】 声明前端变量 timeline，保存本行计算出的数据或对象。
    const timeline=items(lesson?.timeline),records=teachingRecords(),groups=new Map();
    // 【逐行批注】 声明前端变量 sectionForTask，保存本行计算出的数据或对象。
    const sectionForTask=taskId=>{
      // 【逐行批注】 声明前端变量 direct，保存本行计算出的数据或对象。
      const direct=timeline.find(item=>item.taskId===taskId&&item.sectionId);
      // 【逐行批注】 执行当前前端表达式：if(direct)return direct.sectionId;。
      if(direct)return direct.sectionId;
      // 【逐行批注】 声明前端变量 evaluationItem，保存本行计算出的数据或对象。
      const evaluationItem=timeline.find(item=>item.taskId===taskId&&item.type==='evaluation');
      // 【逐行批注】 结束当前函数并返回结果：return evaluationItem?items(lesson?.evaluations).find(item=>item.id==…。
      return evaluationItem?items(lesson?.evaluations).find(item=>item.id===evaluationItem.id)?.sectionId||'':'';
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 按循环规则遍历或重复处理：for(const [key,record] of records){。
    for(const [key,record] of records){
      // 【逐行批注】 声明前端变量 sectionId，保存本行计算出的数据或对象。
      const sectionId=record.sectionId||sectionForTask(record.taskId);
      // 【逐行批注】 声明前端变量 group，保存本行计算出的数据或对象。
      const group=groups.get(sectionId)||[];group.push({...record,displayKey:key});groups.set(sectionId,group);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return groups;。
    return groups;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderTeachingThread，封装可复用的前端行为。
  function renderTeachingThread(records) {
    // 【逐行批注】 调用 items 执行页面交互或数据处理：if(!items(records).length)return null;。
    if(!items(records).length)return null;
    // 【逐行批注】 声明前端变量 thread，保存本行计算出的数据或对象。
    const thread=element('section','','dc-teaching-thread');thread.setAttribute('aria-label','AI 教学回复');
    // 【逐行批注】 按循环规则遍历或重复处理：for(const record of records){。
    for(const record of records){
      // 【逐行批注】 声明前端变量 message，保存本行计算出的数据或对象。
      const message=element('article','',`dc-teaching-message ${record.role==='user'?'user':'assistant'}`);
      // 【逐行批注】 计算右侧结果并更新 message.dataset.teachingKey，改变页面或请求状态。
      message.dataset.teachingKey=record.displayKey;
      // 【逐行批注】 计算右侧结果并更新 message.dataset.teachingTaskId，改变页面或请求状态。
      message.dataset.teachingTaskId=record.taskId||'';
      // 【逐行批注】 调用 element 执行页面交互或数据处理：message.append(element('p',record.role==='user'?'你的回复':'AI 导师','dc-te…。
      message.append(element('p',record.role==='user'?'你的回复':'AI 导师','dc-teaching-label'));
      // 【逐行批注】 调用 element、rich 执行页面交互或数据处理：message.append(record.role==='user'?element('div',record.text,'dc-ric…。
      message.append(record.role==='user'?element('div',record.text,'dc-rich'):rich(record.text));
      // 【逐行批注】 调用 element 执行页面交互或数据处理：if(record.partial)message.append(element('p','这段回复尚未完整完成。','dc-partia…。
      if(record.partial)message.append(element('p','这段回复尚未完整完成。','dc-partial'));
      // 【逐行批注】 执行当前前端表达式：thread.append(message);。
      thread.append(message);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return thread;。
    return thread;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 nextCourseTopic，封装可复用的前端行为。
  function nextCourseTopic() {
    // 【逐行批注】 声明前端变量 topics，保存本行计算出的数据或对象。
    const topics = items(saved?.outline?.courseStructure).flatMap(stage => items(stage.topics)).filter(topic => topic && typeof topic === 'object');
    // 【逐行批注】 声明前端变量 currentId，保存本行计算出的数据或对象。
    const currentId = lesson?.topicId || scope.topicId;
    // 【逐行批注】 声明前端变量 index，保存本行计算出的数据或对象。
    const index = topics.findIndex(topic => currentId ? topic.id === currentId :
      // 【逐行批注】 调用 Number 执行页面交互或数据处理：Number(topic.legacyPhaseIndex) === Number(lesson?.phaseIndex || scope…。
      Number(topic.legacyPhaseIndex) === Number(lesson?.phaseIndex || scope.phaseIndex) &&
      // 【逐行批注】 调用 Number 执行页面交互或数据处理：Number(topic.legacyTopicIndex) === Number(lesson?.topicIndex || scope…。
      Number(topic.legacyTopicIndex) === Number(lesson?.topicIndex || scope.topicIndex));
    // 【逐行批注】 声明前端变量 next，保存本行计算出的数据或对象。
    const next = index >= 0 ? topics[index + 1] : null;
    // 【逐行批注】 声明前端变量 hasIndices，保存本行计算出的数据或对象。
    const hasIndices = next && ['legacyPhaseIndex','legacyTopicIndex'].every(key => Number.isSafeInteger(Number(next[key])) && Number(next[key]) > 0 && Number(next[key]) <= 100000);
    // 【逐行批注】 结束当前函数并返回结果：return next && (next.id || hasIndices) ? next : null;。
    return next && (next.id || hasIndices) ? next : null;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 openTopic，封装可复用的前端行为。
  function openTopic(topic) {
    // 【逐行批注】 声明前端变量 params，保存本行计算出的数据或对象。
    const params = new URLSearchParams({courseId:saved?.course?.id || lesson?.courseId || scope.courseId});
    // 【逐行批注】 判断条件 (topic.id) params.set('topicId', topic.id);，成立时执行对应逻辑。
    if (topic.id) params.set('topicId', topic.id);
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [field, key] of [['legacyPhaseIndex','phaseIndex'], ['legacyTop…。
    for (const [field, key] of [['legacyPhaseIndex','phaseIndex'], ['legacyTopicIndex','topicIndex']])
      // 【逐行批注】 判断条件 (Number.isSafeInteger(Number(topic[field])) && Number(topic[fiel…，成立时执行对应逻辑。
      if (Number.isSafeInteger(Number(topic[field])) && Number(topic[field]) > 0 && Number(topic[field]) <= 100000) params.set(key, topic[field]);
    // 新知识点只打开只读页面，尚未准备时由学生显式开始备课。
    // 【逐行批注】 计算右侧结果并更新 location.href，改变页面或请求状态。
    location.href = location.pathname + '?' + params;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 showDirectory，封装可复用的前端行为。
  function showDirectory() {
    // 【逐行批注】 判断条件 (matchMedia('(max-width:700px)').matches || matchMedia('(max-wid…，成立时执行对应逻辑。
    if (matchMedia('(max-width:700px)').matches || matchMedia('(max-width:900px)').matches && mount.dataset.chatOpen === 'true') document.getElementById('dc-outline-dialog').showModal();
    // 【逐行批注】 执行前一条件不成立时的分支。
    else { mount.dataset.directoryOpen = 'true'; store('directory-open', '1'); }
    // 【逐行批注】 调用 updatePanels 执行页面交互或数据处理：updatePanels();。
    updatePanels();
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderMedia，封装可复用的前端行为。
  function renderMedia(media) {
    // 【逐行批注】 声明前端变量 figure，保存本行计算出的数据或对象。
    const figure = element('figure', '', 'dc-media'); figure.dataset.mediaId = media.id;
    // 【逐行批注】 判断条件 (media.title) figure.append(element('h3', media.title));，成立时执行对应逻辑。
    if (media.title) figure.append(element('h3', media.title));
    // 【逐行批注】 判断条件 (media.status !== 'ready') {，成立时执行对应逻辑。
    if (media.status !== 'ready') {
      // 【逐行批注】 调用 element 执行页面交互或数据处理：figure.append(element('p', media.error || (media.status === 'pending'…。
      figure.append(element('p', media.error || (media.status === 'pending' ? '这部分素材正在准备，已生成的讲义可继续学习。' : '这部分素材暂未完成，已生成的讲义可继续学习。'), 'dc-media-state'));
      // 【逐行批注】 判断条件 (media.status !== 'pending') { const retry = button('重试此素材', 'me…，成立时执行对应逻辑。
      if (media.status !== 'pending') { const retry = button('重试此素材', 'media'); retry.dataset.mediaId = media.id; figure.append(retry); }
    // 【逐行批注】 执行当前前端表达式：} else if (media.type === 'image') {。
    } else if (media.type === 'image') {
      // 【逐行批注】 声明前端变量 url，保存本行计算出的数据或对象。
      const url = safeLink(media.url, true); const source = safeLink(media.sourceUrl);
      // 【逐行批注】 判断条件 (url) {，成立时执行对应逻辑。
      if (url) {
        // 【逐行批注】 声明前端变量 image，保存本行计算出的数据或对象。
        const image = element('img', '', 'dc-media-image'); image.src = url; image.alt = media.alt || media.title || '课堂参考图片'; image.loading = 'lazy'; image.referrerPolicy = 'no-referrer';
        // 【逐行批注】 为页面事件绑定处理逻辑：image.addEventListener('error', () => { image.hidden = true; const message = element…。
        image.addEventListener('error', () => { image.hidden = true; const message = element('p', '图片暂时无法显示，可继续学习或重试此图片。', 'dc-media-state'); const retry = button('重试图片', 'media'); retry.dataset.mediaId = media.id; retry.dataset.repair='true'; figure.append(message, retry); }, {once:true}); figure.append(image);
      // 【逐行批注】 调用 element 执行页面交互或数据处理：} else figure.append(element('p', '图片地址暂不可用。', 'dc-media-state'));。
      } else figure.append(element('p', '图片地址暂不可用。', 'dc-media-state'));
      // 【逐行批注】 判断条件 (source) { const link = element('a', media.sourceTitle || '查看图片来…，成立时执行对应逻辑。
      if (source) { const link = element('a', media.sourceTitle || '查看图片来源', 'dc-media-source'); link.href = source; link.target = '_blank'; link.rel = 'noopener noreferrer'; figure.append(link); }
    // 【逐行批注】 执行当前前端表达式：} else if (media.type === 'timeline') {。
    } else if (media.type === 'timeline') {
      // 【逐行批注】 声明前端变量 list，保存本行计算出的数据或对象。
      const list = element('ol', '', 'dc-timeline'); for (const event of items(media.events)) { const row = element('li'); row.append(element('strong', event.label || ''), element('div', event.text || '')); list.append(row); } figure.append(list);
    // 【逐行批注】 执行当前前端表达式：} else if (media.type === 'flowchart') {。
    } else if (media.type === 'flowchart') {
      // 【逐行批注】 声明前端变量 list，保存本行计算出的数据或对象。
      const list = element('ul', '', 'dc-flow'); for (const node of items(media.nodes)) { const entry = element('li'); entry.append(element('span', node.label || node.id || '', 'dc-flow-node')); list.append(entry); } figure.append(list);
      // 【逐行批注】 声明前端变量 names，保存本行计算出的数据或对象。
      const names = new Map(items(media.nodes).map(node => [node.id, node.label || node.id]));
      // 【逐行批注】 声明前端变量 relations，保存本行计算出的数据或对象。
      const relations = items(media.edges).map(edge => `${names.get(edge.from) || edge.from} → ${names.get(edge.to) || edge.to}${edge.label ? `：${edge.label}` : ''}`);
      // 【逐行批注】 判断条件 (relations.length) figure.append(element('p', relations.join('\n…，成立时执行对应逻辑。
      if (relations.length) figure.append(element('p', relations.join('\n'), 'dc-flow-edge-list'));
    // 【逐行批注】 调用 renderFunction 执行页面交互或数据处理：} else if (media.type === 'function') renderFunction(media, figure);。
    } else if (media.type === 'function') renderFunction(media, figure);
    // 【逐行批注】 当前一条件不成立时继续判断：(media.type === 'forces') renderForces(media, figure);。
    else if (media.type === 'forces') renderForces(media, figure);
    // 渲染入口以同一受控注册表为准，避免新学科已保存却仍被旧类型清单挡住。
    else if (window.GangyiInteractive?.types.includes(media.type)) renderInteractive(media, figure);
    // 【逐行批注】 执行前一条件不成立时的分支。
    else figure.append(element('p', '当前图示格式暂不支持展示，讲义内容仍可阅读。', 'dc-media-state'));
    // 【逐行批注】 判断条件 (media.caption) figure.append(element('figcaption', media.captio…，成立时执行对应逻辑。
    if (media.caption) figure.append(element('figcaption', media.caption));
    // 【逐行批注】 结束当前函数并返回结果：return figure;。
    return figure;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 mediaFor，封装可复用的前端行为。
  function mediaFor(ids) {
    // 【逐行批注】 声明前端变量 media，保存本行计算出的数据或对象。
    const media = items(saved?.content?.media); return items(ids).map(id => media.find(value => value.id === id)).filter(Boolean);
  // 【逐行批注】 结束当前前端代码块。
  }

  function renderInteractive(media, figure) {
    try {
      if (!window.GangyiInteractive) throw new Error('互动没有加载成功，请刷新后重试；讲义仍可阅读。');
      const version = saved?.content?.version || 1;
      const signature = JSON.stringify([version, media]);
      const componentLessonId = lessonId();
      const draftKey = name => `deep:${componentLessonId}:interactive${name}:${media.id}`;
      const loadDraft = name => {
        const key = draftKey(name);
        try { return sessionStorage.getItem(`gangyi-agent:${key}`) ?? api?.storage.get(key); }
        catch (_) { return api?.storage.get(key); }
      };
      const storeDraft = (name, data) => {
        const key = draftKey(name), value = JSON.stringify(data);
        // 每个窗口保留自己的草稿和确认版本，浏览器持久存储仍可用于关页后恢复。
        try { sessionStorage.setItem(`gangyi-agent:${key}`, value); } catch (_) { /* 沿用浏览器持久存储。 */ }
        api?.storage.set(key, value);
      };
      const serverDraft = saved?.componentDrafts?.[media.id];
      let activity;
      try {
        const storedActivity = JSON.parse(loadDraft('-activity') || 'null');
        if (storedActivity?.signature === signature && storedActivity.trace?.contentVersion === version) activity = storedActivity.trace;
      } catch (_) { /* 轨迹损坏时重新开始，不能补造已发生动作。 */ }
      if (!activity || !Array.isArray(activity.events) || !Number.isSafeInteger(activity.totalChanges))
        activity = {schemaVersion:1, contentVersion:version, sessionId:crypto.randomUUID(), totalChanges:0, truncated:false, events:[]};
      let previous = mediaValues.get(media.id);
      if (!previous || typeof previous !== 'object') {
        try {
          const stored = JSON.parse(loadDraft('') || 'null');
          if (stored?.signature === signature) previous = stored.state;
        } catch (_) { /* 损坏的探索草稿不能冒充已恢复状态。 */ }
      }
      let syncMeta;
      try { syncMeta = JSON.parse(loadDraft('-sync') || 'null'); } catch (_) { /* 使用版本冲突保护。 */ }
      const localSync = syncMeta?.signature === signature && syncMeta.sessionId === activity.sessionId ? syncMeta : null;
      const localDirty = activity.totalChanges > (localSync?.totalChanges ?? 0);
      const matchingDraft = serverDraft?.contentVersion === version && serverDraft?.trace?.contentVersion === version;
      const useServerDraft = matchingDraft && (!previous ||
        (!localDirty && serverDraft.revision >= (localSync?.revision ?? 0)));
      if (useServerDraft) {
        previous = serverDraft.state;
        // 本地操作只追加到本地轨迹，服务端快照的状态和序号须始终对应。
        activity = {...serverDraft.trace, events:[...serverDraft.trace.events]};
        storeDraft('', {signature, state:previous});
        storeDraft('-activity', {signature, trace:activity});
        syncMeta = {signature, revision:serverDraft.revision, sessionId:activity.sessionId, totalChanges:activity.totalChanges};
        storeDraft('-sync', syncMeta);
      } else {
        syncMeta = localSync;
      }
      componentTraces.set(media.id, {signature, trace:activity});
      if (!previous || typeof previous !== 'object') {
        const recovered = saved?.componentRecovery?.[media.id];
        if (recovered?.contentVersion === version && recovered.source === 'submitted_component_activity') previous = recovered.state;
      }
      const syncKey = `${componentLessonId}:${media.id}:${version}`;
      let sync = componentDraftSyncs.get(syncKey);
      if (!sync) {
        sync = {revision:syncMeta?.revision ?? 0, pending:null, running:false, blocked:false, timer:null};
        componentDraftSyncs.set(syncKey, sync);
      } else if (useServerDraft && !sync.running && !sync.pending) {
        sync.revision = serverDraft.revision;
      }
      const queueDraft = value => {
        const blocks = items(saved?.content?.[currentView]);
        const page = blocks.find(block => items(block.mediaIds).includes(media.id));
        if (!componentLessonId || !page) return;
        sync.pending = {lessonId:componentLessonId, componentId:media.id, deepView:currentView,
          contentId:page.id, state:JSON.parse(JSON.stringify(value)), trace:JSON.parse(JSON.stringify(activity))};
        clearTimeout(sync.timer);
        const flush = async () => {
          if (sync.running || sync.blocked || !sync.pending) return;
          sync.running = true;
          const pending = sync.pending; sync.pending = null;
          try {
            const result = await request('/api/deep-course/component-draft', {...pending, expectedRevision:sync.revision});
            sync.revision = result.revision;
            storeDraft('-sync', {signature, revision:sync.revision, sessionId:pending.trace.sessionId,
              totalChanges:pending.trace.totalChanges});
          } catch (error) {
            sync.pending = sync.pending || pending;
            sync.blocked = true;
            status(`探索暂未同步：${error.message} 本地草稿仍保留。`);
          } finally {
            sync.running = false;
            if (sync.pending && !sync.blocked) sync.timer = setTimeout(flush, 250);
          }
        };
        sync.timer = setTimeout(flush, 250);
      };
      const saveState = value => {
        // 草稿按课时、组件和内容签名隔离；探索不会调用正式作答接口。
        mediaValues.set(media.id, value);
        storeDraft('', {signature, state:value});
      };
      const recordActivity = (kind, value) => {
        // 只保存完成的受控状态变化；鼠标移动不产生事件，也不会逐次调用模型。
        const current = componentTraces.get(media.id);
        if (current?.signature === signature) activity = current.trace;
        const snapshot = JSON.parse(JSON.stringify(value));
        activity.totalChanges++;
        activity.events.push({seq:activity.totalChanges, kind, state:snapshot});
        if (activity.events.length > 200) activity.events.splice(0, activity.events.length - 200);
        // 标注理由的长度变化较大，保留最近轨迹时同时控制字节量。
        if (media.type === 'annotation_lab')
          while (activity.events.length > 1 && new TextEncoder().encode(JSON.stringify(activity.events)).length > 64000)
            activity.events.shift();
        activity.truncated = activity.totalChanges > activity.events.length;
        storeDraft('-activity', {signature, trace:activity});
      };
      const instance = window.GangyiInteractive.mount(figure, media, {
        contentVersion: version, state: previous,
        onChange: (value, detail) => { saveState(value); recordActivity(detail?.kind === 'reset' ? 'reset' : 'change', value); queueDraft(value); },
        onExplain: async value => {
          saveState(value);
          recordActivity('explain', value);
          queueDraft(value);
          const prompt = {sets:'请根据当前集合探索的操作和结果，解释当前交并补或集合关系，留意全集和空集。',function_lab:'请根据当前函数系数和图像解释变化规律。',
            circuit_lab:'请根据当前电路的连接方式、电源和电阻，解释电流与电压的关系。',motion_lab:'请结合我设置的初位置、初速度、加速度和时刻，解释位移与路程、速度与速率的区别，引导我观察运动方向变化。',chemical_balance:'请根据当前配平系数和原子计数，解释哪些元素已经守恒、哪些尚未守恒，先不要直接给出配平答案。',
            cell_lab:'请解释我当前选择的细胞结构与功能，留意典型细胞示意的适用范围。',history_lab:'请按当前事件排列和已核对日期讨论时间顺序，不能仅凭先后断言因果。',
            climate_lab:'请比较当前选择月份的气温和降水。这是教学示例数据，不是真实城市观测。',annotation_lab:'请结合我对原文的当前标注讨论阅读理由，允许合理多解，不要把标注当成正式作答。',
            sentence_lab:'请结合我对当前句子片段的成分标注讨论句法和语境，不把标签选择直接当成掌握证据。',algorithm_lab:'请根据当前算法、数组、目标和步数解释刚发生的比较，引导我预测下一步。'}[media.type] || '请根据程序核对后的当前探索状态解释结果。';
          await sendEvent({type: 'chat', text: prompt + '这是探索提问，不是正式作答。'}, undefined, media.id);
        }
      });
      mediaValues.set(media.id, instance.getState());
      if (activity.events.length && (!matchingDraft || (!useServerDraft && localDirty))) queueDraft(instance.getState());
      const finish = element('button', '完成探索，继续教学', 'dc-button'); finish.type = 'button';
      finish.onclick = async () => {
        finish.disabled = true;
        try {
          const value = instance.getState(); saveState(value); recordActivity('finish', value);
          queueDraft(value);
          await sendEvent({type:'chat', text:'我已完成当前组件探索，请读取我的真实操作，结合学习目标安排下一步教学。探索不是正式作答，请先通过问题检查理解。'}, undefined, media.id);
        } finally { finish.disabled = false; }
      };
      figure.append(finish);
      interactiveInstances.push({destroy() { finish.onclick = null; finish.remove(); instance.destroy(); }});
    } catch (error) {
      figure.append(element('p', error.message || '互动暂不可用，可以继续阅读讲义。', 'dc-media-state'));
    }
  }
  // 【逐行批注】 定义函数 svgText，封装可复用的前端行为。
  function svgText(parent, text, x, y, attributes = {}) { const node = svg('text', {x,y,...attributes}); node.textContent = text; parent.append(node); return node; }
  // 【逐行批注】 定义函数 renderFunction，封装可复用的前端行为。
  function renderFunction(media, figure) {
    // 【逐行批注】 声明前端变量 plot，保存本行计算出的数据或对象。
    const plot = svg('svg', {viewBox:'0 0 360 230', class:'dc-plot', role:'img', 'aria-label':media.title || '函数图像'}); figure.append(plot);
    // 【逐行批注】 声明前端变量 coefficients，保存本行计算出的数据或对象。
    const coefficients = items(media.coefficients).slice(0,3).map(value => finite(value)); while (coefficients.length < 3) coefficients.push(0);
    // 【逐行批注】 声明前端变量 parameter，保存本行计算出的数据或对象。
    const parameter = media.parameter && typeof media.parameter === 'object' ? media.parameter : null;
    // 【逐行批注】 声明前端变量 index，保存本行计算出的数据或对象。
    const index = Math.max(0, Math.min(2, Math.trunc(finite(parameter?.coefficientIndex))));
    // 【逐行批注】 声明前端变量 initial，保存本行计算出的数据或对象。
    const initial = finite(parameter?.value, coefficients[index]);
    // 【逐行批注】 声明前端变量 update，保存本行计算出的数据或对象。
    const update = value => {
      // 【逐行批注】 判断条件 (parameter) coefficients[index] = value;，成立时执行对应逻辑。
      if (parameter) coefficients[index] = value;
      // 【逐行批注】 计算右侧结果并更新 plot.replaceChildren(); const left，改变页面或请求状态。
      plot.replaceChildren(); const left=43,right=343,top=17,bottom=196;
      // 【逐行批注】 声明前端变量 xMin，保存本行计算出的数据或对象。
      const xMin=finite(media.xMin,-5), xMax=Math.max(xMin+.1,finite(media.xMax,5));
      // 【逐行批注】 声明前端变量 points，保存本行计算出的数据或对象。
      const points = Array.from({length:181},(_,i) => { const x=xMin+(xMax-xMin)*i/180; return [x, coefficients[0]*x*x+coefficients[1]*x+coefficients[2]]; });
      // 【逐行批注】 声明前端变量 yMin，保存本行计算出的数据或对象。
      let yMin=Math.min(0,...points.map(point=>point[1])), yMax=Math.max(0,...points.map(point=>point[1]));
      // 【逐行批注】 声明前端变量 padding，保存本行计算出的数据或对象。
      const padding=Math.max((yMax-yMin)*.12,.5); yMin-=padding; yMax+=padding;
      // 【逐行批注】 声明前端变量 px，保存本行计算出的数据或对象。
      const px=x=>left+(x-xMin)/(xMax-xMin)*(right-left), py=y=>bottom-(y-yMin)/(yMax-yMin)*(bottom-top);
      // 【逐行批注】 按循环规则遍历或重复处理：for(let i=0;i<=4;i++){const y=top+(bottom-top)*i/4;plot.append(svg('line',…。
      for(let i=0;i<=4;i++){const y=top+(bottom-top)*i/4;plot.append(svg('line',{x1:left,y1:y,x2:right,y2:y,class:'dc-tick'}));}
      // 【逐行批注】 声明前端变量 zeroY，保存本行计算出的数据或对象。
      const zeroY=py(0),zeroX=px(Math.max(xMin,Math.min(xMax,0)));
      // 【逐行批注】 调用 svg 执行页面交互或数据处理：plot.append(svg('line',{x1:left,y1:zeroY,x2:right,y2:zeroY,class:'dc-…。
      plot.append(svg('line',{x1:left,y1:zeroY,x2:right,y2:zeroY,class:'dc-axis'}),svg('line',{x1:zeroX,y1:top,x2:zeroX,y2:bottom,class:'dc-axis'}));
      // 【逐行批注】 计算右侧结果并更新 plot.append(svg('path',{d:points.map(([x,y],…，改变页面或请求状态。
      plot.append(svg('path',{d:points.map(([x,y],i)=>`${i?'L':'M'}${px(x).toFixed(2)},${py(y).toFixed(2)}`).join(' '),class:'dc-curve'}));
      // 【逐行批注】 调用 svgText、number 执行页面交互或数据处理：svgText(plot,number(xMin),left,bottom+21,{'text-anchor':'middle'});sv…。
      svgText(plot,number(xMin),left,bottom+21,{'text-anchor':'middle'});svgText(plot,number(xMax),right,bottom+21,{'text-anchor':'middle'});
      // 【逐行批注】 调用 svgText、number 执行页面交互或数据处理：svgText(plot,'x',right-4,zeroY-7);svgText(plot,'y',zeroX+7,top+7);svg…。
      svgText(plot,'x',right-4,zeroY-7);svgText(plot,'y',zeroX+7,top+7);svgText(plot,number(yMax),left-5,top+5,{'text-anchor':'end'});svgText(plot,number(yMin),left-5,bottom+3,{'text-anchor':'end'});
      // 【逐行批注】 计算右侧结果并更新 plot.setAttribute('aria-label',`${media.titl…，改变页面或请求状态。
      plot.setAttribute('aria-label',`${media.title || '函数图像'}，y = ${number(coefficients[0])} x² + ${number(coefficients[1])} x + ${number(coefficients[2])}，横轴范围 ${number(xMin)} 到 ${number(xMax)}`);
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 判断条件 (parameter) renderParameter(figure,media.id,parameter.name || '参…，成立时执行对应逻辑。
    if (parameter) renderParameter(figure,media.id,parameter.name || '参数',parameter.min,parameter.max,parameter.step,initial,value=>{mediaValues.set(media.id,value);update(value);});
    // 【逐行批注】 调用 update、finite 执行页面交互或数据处理：update(finite(mediaValues.get(media.id),initial));。
    update(finite(mediaValues.get(media.id),initial));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderParameter，封装可复用的前端行为。
  function renderParameter(figure, id, name, minimum, maximum, step, initial, onChange, unit = '') {
    // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
    const row=element('div','','dc-parameter'), label=element('label',name), input=element('input'), output=element('output');
    // 【逐行批注】 计算右侧结果并更新 input.type，改变页面或请求状态。
    input.type='range';input.id=`dc-parameter-${id}-${currentView}`;label.htmlFor=input.id;
    // 【逐行批注】 计算右侧结果并更新 input.min，改变页面或请求状态。
    input.min=String(finite(minimum,0));input.max=String(Math.max(Number(input.min)+.1,finite(maximum,10)));input.step=String(Math.max(.001,finite(step,.1)));
    // 【逐行批注】 计算右侧结果并更新 input.value，改变页面或请求状态。
    input.value=String(finite(mediaValues.get(id),initial));output.htmlFor=input.id;output.textContent=number(input.value)+unit;
    // 【逐行批注】 执行当前前端表达式：input.setAttribute('aria-label',`${name}${unit ? `（${unit}）` : ''}`);。
    input.setAttribute('aria-label',`${name}${unit ? `（${unit}）` : ''}`);
    // 【逐行批注】 声明前端变量 change，保存本行计算出的数据或对象。
    const change=()=>{output.textContent=number(input.value)+unit;onChange(Number(input.value));};input.addEventListener('input',change);
    // 【逐行批注】 声明前端变量 reset，保存本行计算出的数据或对象。
    const reset=button('重置','reset');reset.className='dc-reset';reset.onclick=()=>{input.value=String(initial);change();};
    // 【逐行批注】 执行当前前端表达式：row.append(label,input,output,reset);figure.append(row);。
    row.append(label,input,output,reset);figure.append(row);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderForces，封装可复用的前端行为。
  function renderForces(media, figure) {
    // 【逐行批注】 声明前端变量 mass，保存本行计算出的数据或对象。
    const mass=Math.max(.001,finite(media.mass,1)),friction=Math.max(0,finite(media.friction)),initial=finite(media.force);
    // 【逐行批注】 声明前端变量 plot，保存本行计算出的数据或对象。
    const plot=svg('svg',{viewBox:'0 0 360 230',class:'dc-plot',role:'img','aria-label':media.title || '水平方向受力演示'});figure.append(plot);
    // 【逐行批注】 声明前端变量 readout，保存本行计算出的数据或对象。
    const readout=element('div','','dc-force-readout');figure.append(readout);
    // 【逐行批注】 声明前端变量 update，保存本行计算出的数据或对象。
    const update=force=>{
      // 【逐行批注】 计算右侧结果并更新 mediaValues.set(media.id,force);plot.replace…，改变页面或请求状态。
      mediaValues.set(media.id,force);plot.replaceChildren();const direction=Math.sign(force),opposition=Math.min(Math.abs(force),friction),net=direction*Math.max(0,Math.abs(force)-friction),acceleration=net/mass;
      // 【逐行批注】 声明前端变量 defs，保存本行计算出的数据或对象。
      const defs=svg('defs'),marker=svg('marker',{id:`dc-arrow-${media.id}-${currentView}`,viewBox:'0 0 10 10',refX:9,refY:5,markerWidth:5,markerHeight:5,orient:'auto-start-reverse'});marker.append(svg('path',{d:'M0 0 10 5 0 10Z',fill:'currentColor'}));defs.append(marker);plot.append(defs);
      // 【逐行批注】 调用 svg 执行页面交互或数据处理：plot.append(svg('line',{x1:25,y1:156,x2:335,y2:156,class:'dc-axis'}),…。
      plot.append(svg('line',{x1:25,y1:156,x2:335,y2:156,class:'dc-axis'}),svg('rect',{x:145,y:92,width:70,height:64,rx:5,class:'dc-object'}));
      // 【逐行批注】 声明前端变量 scale，保存本行计算出的数据或对象。
      const scale=110/Math.max(1,Math.abs(force),friction),arrow=`url(#dc-arrow-${media.id}-${currentView})`;
      // 【逐行批注】 调用 svg 执行页面交互或数据处理：if(Math.abs(force)>.001)plot.append(svg('line',{x1:180,y1:111,x2:180+…。
      if(Math.abs(force)>.001)plot.append(svg('line',{x1:180,y1:111,x2:180+force*scale,y2:111,class:'dc-force','marker-end':arrow}));
      // 【逐行批注】 调用 svg 执行页面交互或数据处理：if(opposition>.001)plot.append(svg('line',{x1:180,y1:137,x2:180-direc…。
      if(opposition>.001)plot.append(svg('line',{x1:180,y1:137,x2:180-direction*opposition*scale,y2:137,class:'dc-friction','marker-end':arrow}));
      // 【逐行批注】 计算右侧结果并更新 svgText(plot,`F，改变页面或请求状态。
      svgText(plot,`F = ${number(force)} N`,Math.min(285,Math.max(75,180+force*scale*.6)),82,{'text-anchor':'middle'});svgText(plot,`f = ${number(opposition)} N`,95,184,{'text-anchor':'middle'});svgText(plot,`${number(mass)} kg`,180,210,{'text-anchor':'middle'});
      // 【逐行批注】 调用 element、number 执行页面交互或数据处理：readout.replaceChildren(element('span',`合力 ${number(net)} N`),element…。
      readout.replaceChildren(element('span',`合力 ${number(net)} N`),element('span',`加速度 ${number(acceleration)} m/s²`),element('span',`阻力上限 ${number(friction)} N`));
      // 【逐行批注】 调用 number 执行页面交互或数据处理：plot.setAttribute('aria-label',`${media.title || '水平方向受力演示'}，静止起步模型，质…。
      plot.setAttribute('aria-label',`${media.title || '水平方向受力演示'}，静止起步模型，质量 ${number(mass)} 千克，施加力 ${number(force)} 牛，实际反向阻力 ${number(opposition)} 牛，合力 ${number(net)} 牛，加速度 ${number(acceleration)} 米每二次方秒`);
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 声明前端变量 forceRange，保存本行计算出的数据或对象。
    const forceRange=Math.max(20,Math.abs(initial)*2,friction*2);
    // 【逐行批注】 调用 renderParameter 执行页面交互或数据处理：renderParameter(figure,media.id,'施加力 F',-forceRange,forceRange,.5,ini…。
    renderParameter(figure,media.id,'施加力 F',-forceRange,forceRange,.5,initial,update,' N');
    // 【逐行批注】 调用 element 执行页面交互或数据处理：figure.append(element('p','静止起步的简化模型：向右为正，阻力反向且有上限；仅比较水平受力，不模拟速度或位移。'…。
    figure.append(element('p','静止起步的简化模型：向右为正，阻力反向且有上限；仅比较水平受力，不模拟速度或位移。','dc-media-state'));
    // 【逐行批注】 调用 update、finite 执行页面交互或数据处理：update(finite(mediaValues.get(media.id),initial));。
    update(finite(mediaValues.get(media.id),initial));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderQuestion，封装可复用的前端行为。
  function choiceText(value, index) {
    const label=String.fromCharCode(65+index), text=typeof value==='string'?value:(JSON.stringify(value) || '');
    const prefix=text.match(/^\s*([A-Z])[.、:：)）]\s*/i);
    return prefix && prefix[1].toUpperCase()===label ? text.trimStart() : `${label}. ${text}`;
  }
  function questionStem(question) {
    const source=String(question?.question || ''), options=items(question?.options);
    if (!options.length || options.some(value=>typeof value!=='string')) return source;
    const lines=source.trimEnd().split(/\r?\n/), start=lines.length-options.length;
    if (start<=0 || !options.every((value,index)=>lines[start+index].trim()===choiceText(value,index).trim())) return source;
    const stem=lines.slice(0,start).join('\n').trimEnd();
    return stem.trim() ? stem : source;
  }
  function renderQuestion(section,teaching=[]) {
    // 【逐行批注】 声明前端变量 card，保存本行计算出的数据或对象。
    const card=element('section','','dc-question');card.dataset.sectionId=section.id;card.dataset.sectionVersion=String(section.version || 1);
    // 【逐行批注】 声明前端变量 evaluations，保存本行计算出的数据或对象。
    const evaluations=evaluationsFor(section);card.dataset.evaluated=String(evaluations.length>0);
    // 【逐行批注】 调用 element、rich 执行页面交互或数据处理：card.append(element('h2',section.title || '试着想一想'),rich(section.quest…。
    card.append(element('h2',section.title || '试着想一想'),rich(questionStem(section.question)));
    // 【逐行批注】 调用 renderMaterials 执行页面交互或数据处理：if(section.question?.materials)card.append(renderMaterials(section.qu…。
    if(section.question?.materials)card.append(renderMaterials(section.question.materials));
    // 【逐行批注】 声明前端变量 form，保存本行计算出的数据或对象。
    const form=element('form');form.dataset.questionId=section.id;const options=element('div','','dc-question-options');
    // 【逐行批注】 声明前端变量 selected，保存本行计算出的数据或对象。
    let selected=[];try{selected=JSON.parse(restore(`choice:${section.id}`)||'[]');}catch(_){ /* 损坏的本地草稿不改变正式作答。 */ }
    // 【逐行批注】 计算右侧结果并更新 items(section.question?.options).forEach((te…，改变页面或请求状态。
    items(section.question?.options).forEach((text,index)=>{const label=element('label','','dc-question-option'),choice=element('input');choice.type=section.question.type==='multi_choice'?'checkbox':'radio';choice.name=`dc-question-${section.id}`;choice.value=String(index);choice.checked=items(selected).includes(String(index));label.append(choice,element('span',choiceText(text,index)));options.append(label);});
    // 【逐行批注】 为页面事件绑定处理逻辑：options.addEventListener('change',()=>store(`choice:${section.id}`,JSON.stringify([.…。
    options.addEventListener('change',()=>store(`choice:${section.id}`,JSON.stringify([...options.querySelectorAll('input:checked')].map(input=>input.value))));
    // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
    const label=element('label','写下答案或思路'),answer=element('textarea');answer.id=`dc-answer-${section.id}`;label.htmlFor=answer.id;answer.value=restore(`answer:${section.id}`);answer.placeholder='先独立尝试，再和 AI 一起检查。';answer.addEventListener('input',()=>store(`answer:${section.id}`,answer.value));
    // 【逐行批注】 声明前端变量 submit，保存本行计算出的数据或对象。
    const submit=button('提交答案','answer',true);submit.type='submit';form.append(options,label,answer,submit);card.append(form);
    // 【逐行批注】 按循环规则遍历或重复处理：for(const evaluation of evaluations){const feedback=element('div','','dc-e…。
    for(const evaluation of evaluations){const feedback=element('div','','dc-evaluation');feedback.dataset.evaluationId=evaluation.id;feedback.append(element('strong','本次作答反馈'),rich(evaluation.feedback || ''));card.append(feedback);}
    // 【逐行批注】 声明前端变量 thread，保存本行计算出的数据或对象。
    const thread=renderTeachingThread(teaching);if(thread)card.append(thread);
    // 【逐行批注】 结束当前函数并返回结果：return card;。
    return card;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderMaterials，封装可复用的前端行为。
  function renderMaterials(value) {
    // 【逐行批注】 声明前端变量 root，保存本行计算出的数据或对象。
    const root=element('div','','dc-question-materials');
    // 【逐行批注】 调用 renderMaterials 执行页面交互或数据处理：if(Array.isArray(value)){for(const item of value)root.append(renderMa…。
    if(Array.isArray(value)){for(const item of value)root.append(renderMaterials(item));return root;}
    // 【逐行批注】 调用 rich 执行页面交互或数据处理：if(value===null||typeof value!=='object'){root.append(rich(value??'')…。
    if(value===null||typeof value!=='object'){root.append(rich(value??''));return root;}
    // 【逐行批注】 按循环规则遍历或重复处理：for(const key of ['title','text','body','latex','formula'])if(typeof value…。
    for(const key of ['title','text','body','latex','formula'])if(typeof value[key]==='string')root.append(rich(['latex','formula'].includes(key)?`$$${value[key]}$$`:value[key]));
    // 【逐行批注】 声明前端变量 imageUrl，保存本行计算出的数据或对象。
    const imageUrl=safeLink(value.imageUrl);if(imageUrl){const image=element('img','','dc-media-image');image.src=imageUrl;image.alt=value.alt||'题目材料';image.loading='lazy';image.referrerPolicy='no-referrer';root.append(image);}
    // 【逐行批注】 声明前端变量 rows，保存本行计算出的数据或对象。
    const rows=value.rows||value.values||value.cells;if(Array.isArray(rows)){const table=element('table'),wrap=element('div','','dc-rich');if(Array.isArray(value.headers)){const row=element('tr');for(const text of value.headers)row.append(element('th',text));table.append(row);}for(const items of rows){const row=element('tr');for(const text of Array.isArray(items)?items:[items]){const cell=element('td');cell.append(rich(text));row.append(cell);}table.append(row);}wrap.append(table);root.append(wrap);}
    // 【逐行批注】 调用 renderMaterials 执行页面交互或数据处理：if(value.table)root.append(renderMaterials(value.table));return root;。
    if(value.table)root.append(renderMaterials(value.table));return root;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderReading，封装可复用的前端行为。
  function renderReading(preserve = true) {
    const reading = document.getElementById('dc-reading');
    const threads = teachingThreads();
    const lecture = items(saved?.content?.lecture), slides = items(saved?.content?.slides), questions = lessonQuestions();
    // 终态订阅与刷新可能重复返回同一内容。只比较阅读区的事实输入，保留焦点、选区和组件实例。
    // 理解状态展示在标题区，更新判断无需销毁学生正在操作的实验；真实内容/题目/反馈变化仍重绘。
    const signature = JSON.stringify([lesson?.id, lesson?.title, lesson?.entered, lesson?.teachingTaskId, lesson?.teachingRound,
      saved?.content, lesson?.sections, lesson?.evaluations, [...threads], currentView, currentSlide,
      !lecture.length && !slides.length ? [busy, sending] : null]);
    if (readingSignature === signature && reading.firstChild) {
      if (!preserve) reading.scrollTop = finite(restore(`scroll:${currentView}`));
      setDisabled();
      return;
    }
    // 更换讲义或课件之前释放旧组件的监听器，状态由独立草稿继续保存。
    for (const instance of interactiveInstances.splice(0)) instance.destroy();
    // 【逐行批注】 声明前端变量 reading，保存本行计算出的数据或对象。
    const oldTop=preserve?reading.scrollTop:finite(restore(`scroll:${currentView}`));
    // 【逐行批注】 声明前端变量 wasAtBottom，保存本行计算出的数据或对象。
    const wasAtBottom=oldTop+reading.clientHeight>=reading.scrollHeight-28;
    // 【逐行批注】 执行当前前端表达式：reading.replaceChildren();。
    reading.replaceChildren();
    // 【逐行批注】 声明前端变量 threads，保存本行计算出的数据或对象。
    // 【逐行批注】 声明前端变量 lecture，保存本行计算出的数据或对象。
    // 【逐行批注】 执行当前前端表达式：if(!lecture.length&&!slides.length){。
    if(!lecture.length&&!slides.length){
      // 【逐行批注】 声明前端变量 draft，保存本行计算出的数据或对象。
      const draft=items(lesson?.sections).filter(section=>['explanation','summary'].includes(section.kind)&&String(section.body||'').trim());
      // 【逐行批注】 执行当前前端表达式：if(draft.length){。
      if(draft.length){
        // 【逐行批注】 声明前端变量 article，保存本行计算出的数据或对象。
        const article=element('article','','dc-lecture dc-draft-lecture');
        // 【逐行批注】 声明前端变量 note，保存本行计算出的数据或对象。
        const note=element('aside','','dc-draft-note');note.setAttribute('role','status');
        // 【逐行批注】 调用 element 执行页面交互或数据处理：note.append(element('strong','已保存内容，可以先阅读'),element('p',busy?'AI 正在继续…。
        note.append(element('strong','已保存内容，可以先阅读'),element('p',busy?'AI 正在继续补充其余内容；这里的讲解已经保存，刷新或离开页面不会丢失。':'本节内容尚未完整保存，下方仅显示已经确认可用的讲解。'));
        // 【逐行批注】 执行当前前端表达式：article.append(note);。
        article.append(note);
        // 【逐行批注】 按循环规则遍历或重复处理：for(const section of draft){const block=element('section','','dc-lecture-s…。
        for(const section of draft){const block=element('section','','dc-lecture-section');block.dataset.sectionId=section.id||'';if(section.title)block.append(element('h2',section.title));block.append(rich(section.body));article.append(block);}
        // 【逐行批注】 执行当前前端表达式：reading.append(article);。
        reading.append(article);
      // 【逐行批注】 执行当前前端表达式：} else {。
      } else {
        // 【逐行批注】 声明前端变量 empty，保存本行计算出的数据或对象。
        const empty=element('section','','dc-empty');empty.append(element('h2',lesson?'本节内容正在准备':'从这一课开始'),element('p',lesson?'AI 会结合课程目标与学习记录准备内容。已成功保存的部分会优先展示。':'进入课时后生成本节讲义、课件和需要的图示，之后可以随时回来继续。'));
        // 【逐行批注】 调用 button 执行页面交互或数据处理：if(!busy&&!sending)empty.append(button(lesson?'准备本节内容':'开始这一课','start…。
        if (!busy && !sending) {
          if (learningTools) empty.append(element('p', '从操作菜单选择「准备本节内容」。'));
          else empty.append(button(lesson ? '准备本节内容' : '开始这一课', 'start', true, 'arrow'));
        }
        reading.append(empty);
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 执行当前前端表达式：} else if(currentView==='slides') {。
    } else if(currentView==='slides') {
      // 【逐行批注】 调用 element、button 执行页面交互或数据处理：if(!slides.length){const empty=element('div','','dc-empty');empty.app…。
      if(!slides.length){const empty=element('div','','dc-empty');empty.append(element('h2','本节暂未生成课件'),element('p','可以先阅读已保存的讲义，或在右侧独立聊天中询问。'),button('打开 AI 聊天','chat'));reading.append(empty);}
      // 【逐行批注】 执行前一条件不成立时的分支。
      else {
        // 【逐行批注】 计算右侧结果并更新 currentSlide，改变页面或请求状态。
        currentSlide=Math.min(Math.max(0,currentSlide),slides.length-1);const slide=slides[currentSlide];const card=element('article','','dc-slide');card.append(element('h2',slide.title || lesson?.title || '本节课件'));
        // 【逐行批注】 声明前端变量 sources，保存本行计算出的数据或对象。
        const sources=items(slide.sourceSectionIds).map(id=>items(lesson?.sections).find(section=>section.id===id&&['explanation','summary'].includes(section.kind))).filter(Boolean);
        // 【逐行批注】 计算右侧结果并更新 if(sources.length){for(const source of sourc…，改变页面或请求状态。
        if(sources.length){for(const source of sources){const block=element('div','','dc-slide-source');block.dataset.sectionId=source.id;block.append(rich(source.body||''));card.append(block);}}
        // 【逐行批注】 执行前一条件不成立时的分支。
        else card.append(rich(slide.body || ''));
        // 【逐行批注】 按循环规则遍历或重复处理：for(const media of mediaFor(slide.mediaIds))card.append(renderMedia(media)…。
        for(const media of mediaFor(slide.mediaIds))card.append(renderMedia(media));reading.append(card);
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 执行当前前端表达式：} else {。
    } else {
      // 【逐行批注】 声明前端变量 article，保存本行计算出的数据或对象。
      const article=element('article','','dc-lecture'),attached=new Set();
      // 【逐行批注】 按循环规则遍历或重复处理：for(const section of lecture){const block=element('section','','dc-lecture…。
      for(const section of lecture){const block=element('section','','dc-lecture-section');block.dataset.sectionId=section.sectionId||section.id;if(section.title)block.append(element('h2',section.title));block.append(rich(section.body || ''));for(const media of mediaFor(section.mediaIds)){block.append(renderMedia(media));attached.add(media.id);}article.append(block);}
      // 【逐行批注】 按循环规则遍历或重复处理：for(const media of items(saved?.content?.media))if(!attached.has(media.id)…。
      for(const media of items(saved?.content?.media))if(!attached.has(media.id))article.append(renderMedia(media));
      // 【逐行批注】 声明前端变量 questionIds，保存本行计算出的数据或对象。
      const questionIds=new Set(questions.map(section=>section.id));
      // 【逐行批注】 按循环规则遍历或重复处理：for(const section of questions)article.append(renderQuestion(section,threa…。
      for(const section of questions)article.append(renderQuestion(section,threads.get(section.id)));
      // 【逐行批注】 按循环规则遍历或重复处理：for(const [sectionId,records] of threads)if(!questionIds.has(sectionId)){c…。
      for(const [sectionId,records] of threads)if(!questionIds.has(sectionId)){const thread=renderTeachingThread(records);if(thread)article.append(thread);}
      // 【逐行批注】 执行当前前端表达式：reading.append(article);。
      reading.append(article);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 声明前端变量 slideToolbar，保存本行计算出的数据或对象。
    const slideToolbar=document.getElementById('dc-slide-navigation');slideToolbar.hidden=currentView!=='slides'||!slides.length;
    // 【逐行批注】 执行当前前端表达式：if(!slideToolbar.hidden){document.getElementById('dc-slide-previous').disabled=currentSli…。
    if(!slideToolbar.hidden){document.getElementById('dc-slide-previous').disabled=currentSlide===0;document.getElementById('dc-slide-next').disabled=currentSlide===slides.length-1;document.getElementById('dc-slide-count').textContent=`${currentSlide+1} / ${slides.length}`;}
    // 【逐行批注】 计算右侧结果并更新 reading.scrollTop，改变页面或请求状态。
    reading.scrollTop=wasAtBottom?reading.scrollHeight:oldTop;
    // 【逐行批注】 按循环规则遍历或重复处理：for(const tab of mount.querySelectorAll('[data-view-mode]')){。
    for(const tab of document.querySelectorAll('[data-view-mode]')){
      // 【逐行批注】 声明前端变量 selected，保存本行计算出的数据或对象。
      const selected=tab.dataset.viewMode===currentView;
      // 【逐行批注】 计算右侧结果并更新 tab.setAttribute('aria-selected',String(sele…，改变页面或请求状态。
      tab.setAttribute('aria-selected',String(selected));tab.tabIndex=selected?0:-1;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 调用 setDisabled 执行页面交互或数据处理：reading.setAttribute('aria-labelledby',`dc-tab-${currentView}`);setDi…。
    reading.setAttribute('aria-labelledby',`dc-tab-${currentView}`);setDisabled();
    // 仅在渲染成功后记住签名，异常不能把未完成的视图误认为已经显示。
    readingSignature = signature;
    // 【逐行批注】 计算右侧结果并更新 requestAnimationFrame(()，改变页面或请求状态。
    requestAnimationFrame(()=>{
      // 【逐行批注】 声明前端变量 current，保存本行计算出的数据或对象。
      const current=lesson?.teachingTaskId;
      // 【逐行批注】 执行当前前端表达式：if(!current||focusedTeachingTask===current)return;。
      if(!current||focusedTeachingTask===current)return;
      // 【逐行批注】 声明前端变量 reply，保存本行计算出的数据或对象。
      const reply=[...reading.querySelectorAll('.dc-teaching-message.assistant')]
        // 【逐行批注】 计算右侧结果并更新 .find(node，改变页面或请求状态。
        .find(node=>node.dataset.teachingTaskId===current);
      // 【逐行批注】 执行当前前端表达式：if(!reply)return;。
      if(!reply)return;
      // 【逐行批注】 计算右侧结果并更新 focusedTeachingTask，改变页面或请求状态。
      focusedTeachingTask=current;
      // 【逐行批注】 执行当前前端表达式：reply.scrollIntoView({block:'center',behavior:'auto'});。
      reply.scrollIntoView({block:'center',behavior:'auto'});
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 【逐行批注】 调用 requestAnimationFrame 执行页面交互或数据处理：if(lesson?.entered===true)requestAnimationFrame(exposeVisible);。
    if(lesson?.entered===true)requestAnimationFrame(exposeVisible);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderChat，封装可复用的前端行为。
  // 流式片段只合并绘制，原始文本和序列仍逐段接收。
  function scheduleChatRender() {
    if (chatRenderFrame) return;
    chatRenderFrame = requestAnimationFrame(() => { chatRenderFrame = 0; renderChat(); });
  }
  function renderChat() {
    if (chatRenderFrame) { cancelAnimationFrame(chatRenderFrame); chatRenderFrame = 0; }
    const scroll = document.getElementById('dc-chat-scroll');
    if (!scroll) return;
    const oldTop = scroll.scrollTop, atBottom = scroll.scrollHeight - scroll.clientHeight - oldTop < 55;
    const currentScope = independentChat.lessonId || lesson?.id || scope.lessonId || '';
    if (chatNodeScope !== currentScope) { chatNodeCache.clear(); chatNodeScope = currentScope; }
    const nodes = [], used = new Set();
    const cached = (key, value, create) => {
      const signature = JSON.stringify(value), previous = chatNodeCache.get(key);
      used.add(key);
      if (previous?.signature === signature) return previous.node;
      const node = create(); chatNodeCache.set(key, {signature, node}); return node;
    };
    for (const [index, record] of items(independentChat.messages).entries()) {
      const text = record.content || ''; if (!text) continue;
      const key = 'message:' + (record.id || index);
      const message = cached(key, [text,record.role,record.createdAt,record.sources,record.status,record.error], () => {
        const node = element('article','',`dc-chat-message ${record.role === 'user' ? 'user' : 'assistant'}`);
        node.dataset.messageKey = record.id;
        const label = element('h3',record.role === 'user' ? '你' : 'AI 聊天');
        const at = record.createdAt ? chatTime(record.createdAt) : '';
        if (at) label.append(element('time',at)); node.append(label);
        node.append(record.role === 'user' ? element('div',text,'dc-rich') : rich(text));
        const sources = items(record.sources).map(value => safeLink(value)).filter(Boolean);
        if (sources.length) {
          const list = element('ol','','dc-chat-sources'); list.append(element('strong','参考来源'));
          sources.forEach((url,sourceIndex) => {
            const row = element('li'), link = element('a',(sourceIndex + 1) + '. ' + url);
            link.href = url; link.target = '_blank'; link.rel = 'noopener noreferrer'; row.append(link); list.append(row);
          }); node.append(list);
        }
        if (['failed','paused','cancelled','sending'].includes(record.status))
          node.append(element('p',record.error || ({failed:'回复未完成。',paused:'回复已暂停。',cancelled:'回复已停止。',sending:'正在发送…'})[record.status],'dc-partial'));
        return node;
      });
      nodes.push(message);
    }
    if (independentChatStream) {
      const draft = cached('stream','stream',() => {
        const node = element('article','','dc-chat-message assistant');
        node.append(element('h3','AI 聊天 · 正在回复'),rich('')); return node;
      });
      const entry = chatNodeCache.get('stream');
      if (entry.text !== independentChatStream) {
        draft.lastElementChild.replaceWith(rich(independentChatStream)); entry.text = independentChatStream;
      }
      nodes.push(draft);
    }
    const task = independentChat.activeTask;
    if (task && ['pending','running','paused','failed','cancelled'].includes(task.status)) {
      nodes.push(cached('task',[task.id,task.status,task.activity?.title,task.activity?.detail],() => {
        const state = element('div','','dc-chat-task-state'); state.dataset.status = task.status;
        state.append(element('strong',task.activity?.title || ({pending:'等待开始',running:'正在回复',paused:'已暂停',failed:'回复未完成',cancelled:'已停止'})[task.status] || '处理中'));
        if (task.activity?.detail) state.append(element('p',task.activity.detail));
        if (task.status === 'failed' && task.id) { const retry = button('重试这次回复','retry-independent',false); retry.onclick = retryIndependentChat; state.append(retry); }
        if (task.status === 'paused') { const resume = button('继续回复','',false); resume.onclick = () => controlIndependentChat('resume'); state.append(resume); }
        if (['pending','running'].includes(task.status)) { const stop = button('停止回复','',false); stop.onclick = () => controlIndependentChat('cancel'); state.append(stop); }
        return state;
      }));
    }
    if (independentChat.error) nodes.push(cached('error',independentChat.error,() => element('p',independentChat.error,'dc-chat-error')));
    if (!nodes.length) nodes.push(cached('empty','empty',() => element('p','可以询问任何问题。这里不会读取课程内容、作答、画像、计划或长期记忆。','dc-chat-empty')));
    for (const key of chatNodeCache.keys()) if (!used.has(key)) chatNodeCache.delete(key);
    // 仅插入、移动或移除变化节点；逐段输出不再拆掉历史、选择或停止按钮。
    let cursor = scroll.firstChild;
    for (const node of nodes) {
      if (node === cursor) cursor = cursor.nextSibling;
      else scroll.insertBefore(node,cursor);
    }
    while (cursor) { const next = cursor.nextSibling; cursor.remove(); cursor = next; }
    scroll.scrollTop = atBottom ? scroll.scrollHeight : oldTop;
    syncChatAvailability();
  }
  // 【逐行批注】 定义函数 exposeVisible，封装可复用的前端行为。
  function exposeVisible() {
    // 【逐行批注】 执行当前前端表达式：if(!lesson||lesson.entered!==true||!api||document.hidden)return;const viewport=document.g…。
    if(!lesson||lesson.entered!==true||!api||document.hidden)return;const viewport=document.getElementById('dc-reading');
    // 【逐行批注】 声明前端变量 byId，保存本行计算出的数据或对象。
    const byId=new Map(items(lesson.sections).map(section=>[section.id,section]));
    // 【逐行批注】 按循环规则遍历或重复处理：for(const node of viewport.querySelectorAll('[data-section-id]')){const se…。
    for(const node of viewport.querySelectorAll('[data-section-id]')){const section=byId.get(node.dataset.sectionId);if(section)api.recordVisibleSection?.(lesson.id,section,node,viewport);}
    // 【逐行批注】 按循环规则遍历或重复处理：for(const node of viewport.querySelectorAll('[data-evaluation-id]')){const…。
    for(const node of viewport.querySelectorAll('[data-evaluation-id]')){const evaluation=items(lesson.evaluations).find(value=>value.id===node.dataset.evaluationId);if(evaluation)api.recordEvaluation?.(lesson.id,evaluation,node,viewport);}
      // 【逐行批注】 声明前端变量 records，保存本行计算出的数据或对象。
      const records=teachingRecords();
      // 【逐行批注】 按循环规则遍历或重复处理：for(const node of viewport.querySelectorAll('[data-teaching-key]')){const …。
      for(const node of viewport.querySelectorAll('[data-teaching-key]')){const record=records.get(node.dataset.teachingKey);if(record)api.recordVisibleMessage?.(record,node.querySelector('.dc-rich'),viewport);}
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 setDisabled，封装可复用的前端行为。
  function setDisabled() {
    const revise = document.querySelector('[data-action=revise-round]');
    revise.hidden = lesson?.teachingRound?.status !== 'active';
    revise.disabled = sending || lesson?.entered !== true || Boolean(scopeError);
    // 【逐行批注】 声明前端变量 locked，保存本行计算出的数据或对象。
    const locked=busy||sending;
    const round = lesson?.teachingRound;
    const unavailable = locked || lesson?.entered !== true || Boolean(scopeError);
    for (const form of mount.querySelectorAll('.dc-question form')) {
      const historical = Boolean(round && round.pendingSectionId !== form.dataset.questionId);
      const options = form.querySelector('.dc-question-options');
      form.hidden = historical && !options.childElementCount;
      for (const node of form.children) if (node !== options) node.hidden = historical;
      for (const control of form.elements) control.disabled = unavailable || historical;
    }
    for (const control of document.querySelectorAll('[data-action=finish],[data-action=retry-media],[data-action=media]')) {
      control.disabled = unavailable;
    }
    // 课程备课在后台进行时仍允许使用独立聊天；聊天不读取课程上下文。
    // 已保存的草稿内容同样可以发送，不把“正式进入课堂”当作聊天前置条件。
    // 【逐行批注】 调用 syncChatAvailability 执行页面交互或数据处理：syncChatAvailability();。
    syncChatAvailability();
    // 【逐行批注】 声明前端变量 next，保存本行计算出的数据或对象。
    const next=document.getElementById('dc-continue'),topic=lesson?.completed?nextCourseTopic():null;
    // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
    const label=locked?'查看生成进度':!lesson?'准备本节内容':lesson.completed?(topic?'下一知识点':'查看课程目录'):lesson.entered!==true?'查看生成进度':'继续学习';
    // 【逐行批注】 计算右侧结果并更新 next.querySelector('span').textContent，改变页面或请求状态。
    next.querySelector('span').textContent=label;next.setAttribute('aria-label',topic?`下一知识点：${topic.title||topic.name||'继续课程'}`:label);
    // 【逐行批注】 计算右侧结果并更新 next.disabled，改变页面或请求状态。
    next.disabled = Boolean(scopeError);
    next.setAttribute('aria-disabled', String(locked || Boolean(lesson && !lesson.completed && lesson.entered !== true)));
    // 【逐行批注】 计算右侧结果并更新 mount.dataset.ready，改变页面或请求状态。
    mount.dataset.ready=String(!locked&&Boolean(lesson)&&lesson.entered===true&&!scopeError);
    // 【逐行批注】 按循环规则遍历或重复处理：for(const control of mount.querySelectorAll('[data-action=start]'))control…。
    for(const control of mount.querySelectorAll('[data-action=start]'))control.disabled=locked;
    // 【逐行批注】 声明前端变量 stop，保存本行计算出的数据或对象。
    const stop=document.getElementById('dc-stop'),retry=document.getElementById('dc-retry');stop.hidden=!busy;retry.hidden=!lastTask||!['failed','cancelled','superseded'].includes(lastTask.status);stop.disabled=!watchingTask;retry.disabled=sending;
    // 【逐行批注】 声明前端变量 activitySpinner，保存本行计算出的数据或对象。
    const activitySpinner=document.getElementById('dc-activity-spinner');if(activitySpinner)activitySpinner.hidden=!(busy||sending);
    // 【逐行批注】 声明前端变量 enter，保存本行计算出的数据或对象。
    const enter=document.getElementById('dc-enter');enter.hidden=!entryFailure;enter.disabled=Boolean(enteringLesson)||sending;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 syncChatAvailability，封装可复用的前端行为。
  function syncChatAvailability() {
    // 【逐行批注】 声明前端变量 input，保存本行计算出的数据或对象。
    const input=document.getElementById('dc-chat-input');
    // 【逐行批注】 声明前端变量 send，保存本行计算出的数据或对象。
    const send=document.querySelector('#dc-chat-form button[type=submit]');
    // 【逐行批注】 执行当前前端表达式：if(!input||!send)return;。
    if(!input||!send)return;
    // 【逐行批注】 声明前端变量 hasLesson，保存本行计算出的数据或对象。
    const hasLesson=Boolean(lesson?.id||scope.lessonId);
    // 【逐行批注】 声明前端变量 taskActive，保存本行计算出的数据或对象。
    const taskActive=['pending','running'].includes(independentChat.activeTask?.status)&&!independentChat.activeTask?.paused;
    // 【逐行批注】 声明前端变量 currentLesson，保存本行计算出的数据或对象。
    const currentLesson=lesson?.id||scope.lessonId||'';
    // 【逐行批注】 声明前端变量 submitting，保存本行计算出的数据或对象。
    const submitting=independentChatSubmitting.has(currentLesson);
    // 【逐行批注】 声明前端变量 streaming，保存本行计算出的数据或对象。
    const streaming=independentChatStreaming.has(currentLesson)||taskActive;
    // 【逐行批注】 声明前端变量 disabled，保存本行计算出的数据或对象。
    const disabled=!hasLesson||Boolean(scopeError)||submitting||streaming||!input.value.trim();
    // 【逐行批注】 计算右侧结果并更新 send.disabled，改变页面或请求状态。
    send.disabled=disabled;send.setAttribute('aria-disabled',String(disabled));
    // 【逐行批注】 调用 String 执行页面交互或数据处理：send.setAttribute('aria-busy',String(submitting||streaming));。
    send.setAttribute('aria-busy',String(submitting||streaming));
    // 【逐行批注】 执行当前前端表达式：send.setAttribute('aria-label',streaming?'AI 正在回复':submitting?'正在发送问题':'发送问题');。
    send.setAttribute('aria-label',streaming?'AI 正在回复':submitting?'正在发送问题':'发送问题');
    // 【逐行批注】 声明前端变量 web，保存本行计算出的数据或对象。
    const web=document.getElementById('dc-web-toggle');
    // 【逐行批注】 计算右侧结果并更新 if(web){const enabled，改变页面或请求状态。
    if(web){const enabled=Boolean(independentChat.webEnabled);web.setAttribute('aria-pressed',String(enabled));web.setAttribute('aria-label',enabled?'关闭联网搜索':'开启联网搜索');web.dataset.enabled=String(enabled);web.disabled=submitting||streaming;}
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 syncCourseReplyAvailability，封装可复用的前端行为。
  function syncCourseReplyAvailability() {
    // 【逐行批注】 声明前端变量 input，保存本行计算出的数据或对象。
    const input=document.getElementById('dc-course-reply-input');
    // 【逐行批注】 声明前端变量 send，保存本行计算出的数据或对象。
    const send=document.getElementById('dc-course-reply-send');
    // 【逐行批注】 执行当前前端表达式：if(!input||!send)return;。
    if(!input||!send)return;
    // 【逐行批注】 声明前端变量 hasLesson，保存本行计算出的数据或对象。
    const hasLesson=Boolean(lesson?.id||scope.lessonId);
    // 【逐行批注】 声明前端变量 unavailable，保存本行计算出的数据或对象。
    const unavailable=!hasLesson||Boolean(scopeError)||lesson?.entered!==true;
    // 【逐行批注】 计算右侧结果并更新 input.disabled，改变页面或请求状态。
    input.disabled=unavailable;
    // 【逐行批注】 声明前端变量 disabled，保存本行计算出的数据或对象。
    const disabled=unavailable||busy||sending||!input.value.trim();
    // 【逐行批注】 计算右侧结果并更新 send.disabled，改变页面或请求状态。
    send.disabled=disabled;send.setAttribute('aria-disabled',String(disabled));
    // 【逐行批注】 调用 String 执行页面交互或数据处理：send.setAttribute('aria-busy',String(sending));。
    send.setAttribute('aria-busy',String(sending));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderAll，封装可复用的前端行为。
  function renderAll() {
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('dc-title').textCont…，改变页面或请求状态。
    document.getElementById('dc-title').textContent=lesson?.title || saved?.course?.title || '深度课程';
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('dc-fullscreen-title…，改变页面或请求状态。
    document.getElementById('dc-fullscreen-title').textContent=lesson?.title || saved?.course?.title || '深度课程';
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('dc-context').textCo…，改变页面或请求状态。
    document.getElementById('dc-context').textContent=lesson?.purpose || saved?.course?.goal || '讲义与课件，按需要深入学习。';
    const round = lesson?.teachingRound;
    const roundNode = document.getElementById('dc-teaching-round');
    roundNode.hidden = !round;
    const roundText = round ? `本轮目标：${round.goal} · 已反馈 ${round.feedbackCount}/${round.questionCount} 题` : '';
    if (roundNode.textContent !== roundText) roundNode.textContent = roundText;
    // 只展示服务端当前有效的理解判断；无评估不推测，旧证据失效时说明正在重新确认。
    const mastery = lesson?.learningState?.mastery;
    api?.paintTopicEvidence(document.getElementById('dc-topic-evidence'), lesson);
    const preferences = lesson?.teachingPreferences;
    const preferenceNode = document.getElementById('dc-teaching-preferences');
    const preferenceLabels = [preferences?.explanation === 'simple' ? '简单讲解' : '',
      ({slow:'慢一点',fast:'快一点'})[preferences?.pace] || '', preferences?.guidance === 'independent' ? '自己尝试' : ''].filter(Boolean);
    preferenceNode.hidden = !preferences?.version;
    preferenceNode.textContent = preferenceLabels.length ? `本课：${preferenceLabels.join(' · ')}` : '本课：按默认方式讲解';
    const labels = {UNKNOWN:'还需了解', NOT_STARTED:'尚未开始', LEARNING:'正在学习', PARTIAL:'部分理解',
      MISUNDERSTOOD:'需要澄清一个概念', PRACTICING:'正在巩固', PROVISIONAL_MASTERY:'初步掌握', VERIFIED_MASTERY:'已通过本次独立验证'};
    const masteryHost = document.getElementById('dc-mastery');
    masteryHost.hidden = !(mastery?.assessed || mastery?.stale);
    document.getElementById('dc-mastery-summary').textContent = mastery?.stale ? '学习情况有更新，需要重新确认' : `当前理解：${labels[mastery?.status] || '还需了解'}`;
    document.getElementById('dc-mastery-reason').textContent = [mastery?.reason, mastery?.uncertainty].filter(Boolean).join(' ');
    const actions = {explain:'换一种讲法', ask:'再做一次理解检查', hint:'先看一点提示', practice:'再练一道', review:'安排复习', advance:'进入下一知识点', pause:'暂停学习'};
    const nextSuggestion = lesson?.learningState?.nextTeachingAction;
    const nextNode = document.getElementById('dc-mastery-next');
    nextNode.hidden = !nextSuggestion || !actions[nextSuggestion.type];
    nextNode.textContent = nextSuggestion && actions[nextSuggestion.type] ? `下一步建议：${actions[nextSuggestion.type]}` : '';
    workspaceContext?.update(lesson?.courseId || scope.courseId);
    // 【逐行批注】 调用 renderOutline、renderReading、renderChat 执行页面交互或数据处理：renderOutline();renderReading();renderChat();。
    renderOutline();renderReading();renderChat();
    // 【逐行批注】 声明前端变量 chatInput，保存本行计算出的数据或对象。
    const chatInput=document.getElementById('dc-chat-input');chatInput.value=restore('chat-draft');
    // 【逐行批注】 声明前端变量 courseReplyInput，保存本行计算出的数据或对象。
    const courseReplyInput=document.getElementById('dc-course-reply-input');
    // 【逐行批注】 调用 restore 执行页面交互或数据处理：if(courseReplyInput&&document.activeElement!==courseReplyInput)course…。
    if(courseReplyInput&&document.activeElement!==courseReplyInput)courseReplyInput.value=restore('course-reply-draft');
    // 【逐行批注】 调用 syncChatAvailability、syncCourseReplyAvailability 执行页面交互或数据处理：syncChatAvailability();syncCourseReplyAvailability();。
    syncChatAvailability();syncCourseReplyAvailability();
    // 【逐行批注】 调用 loadIndependentChat 执行页面交互或数据处理：if(lesson?.id||scope.lessonId)loadIndependentChat();。
    if(lesson?.id||scope.lessonId)loadIndependentChat();
    // 【逐行批注】 声明前端变量 next，保存本行计算出的数据或对象。
    const next=lesson?.completed?nextCourseTopic():null;
    // 【逐行批注】 调用 status、updatePanels 执行页面交互或数据处理：status(lesson?.completed&&!busy?(next?`本节已完成，可前往「${next.title||next.n…。
    status(lesson?.completed&&!busy?(next?`本节已完成，可前往「${next.title||next.name||'下一知识点'}」。`:'本节已完成，可从目录回顾或选择其他知识点。'):taskMessage||'讲义与课件已保存，学习操作会继续记录。');updatePanels();
    // 【逐行批注】 调用 CustomEvent 执行页面交互或数据处理：if(lesson){window.GangyiNavigation?.setContext?.({courseId:lesson.cou…。
    if(lesson){window.GangyiNavigation?.setContext?.({courseId:lesson.courseId,lessonId:lesson.id,phaseIndex:lesson.phaseIndex,topicIndex:lesson.topicIndex,topicId:lesson.topicId,teachingTaskId:lesson.teachingTaskId});document.dispatchEvent(new CustomEvent('gangyi:lesson-context',{detail:{courseId:lesson.courseId,lessonId:lesson.id,phaseIndex:lesson.phaseIndex,topicIndex:lesson.topicIndex}}));}
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 refresh，封装可复用的前端行为。
  async function refresh() {
    // 【逐行批注】 执行当前前端表达式：if(scopeError)return;。
    if(scopeError)return;
    // 【逐行批注】 计算右侧结果并更新 if(refreshPromise){refreshAgain，改变页面或请求状态。
    if(refreshPromise){refreshAgain=true;return refreshPromise;}
    // 【逐行批注】 声明前端变量 epoch，保存本行计算出的数据或对象。
    const epoch=activeEpoch;
    // 【逐行批注】 计算右侧结果并更新 refreshPromise，改变页面或请求状态。
    refreshPromise=(async()=>{
      // 【逐行批注】 计算右侧结果并更新 try{const result，改变页面或请求状态。
      try{const result=await request('/api/deep-course?'+new URLSearchParams(scope));if(epoch!==activeEpoch)return;saved=result;lesson=result.lesson || null;
        // 【逐行批注】 声明前端变量 activePreparation，保存本行计算出的数据或对象。
        let activePreparation=result.activeTask && typeof result.activeTask==='object'?result.activeTask:null;
        // 【逐行批注】 计算右侧结果并更新 if(lesson?.id){scope.lessonId，改变页面或请求状态。
        if(lesson?.id){scope.lessonId=lesson.id;query.set('lessonId',lesson.id);history.replaceState(null,'',location.pathname+'?'+query);}
        // 【逐行批注】 执行当前前端表达式：if(lesson?.status==='ready'&&lesson.entered!==true&&!enteringLesson&&!entryFailure){。
        if(lesson?.status==='ready'&&lesson.entered!==true&&!enteringLesson&&!entryFailure){
          // 【逐行批注】 计算右侧结果并更新 enteringLesson，改变页面或请求状态。
          enteringLesson=lesson.id;setDisabled();status('正在恢复本课学习位置…');
          // 【逐行批注】 调用 request 执行页面交互或数据处理：try{if(api?.control)await api.control({command:'enter_lesson',lessonI…。
          try{if(api?.control)await api.control({command:'enter_lesson',lessonId:lesson.id});else await request('/api/learning-agent/control',{command:'enter_lesson',lessonId:lesson.id});
            // 【逐行批注】 调用 request、URLSearchParams 执行页面交互或数据处理：if(epoch===activeEpoch){const entered=await request('/api/deep-course…。
            if(epoch===activeEpoch){const entered=await request('/api/deep-course?'+new URLSearchParams(scope));if(epoch===activeEpoch){saved=entered;lesson=entered.lesson||null;activePreparation=entered.activeTask&&typeof entered.activeTask==='object'?entered.activeTask:null;}}
          // 【逐行批注】 计算右侧结果并更新 }catch(error){entryFailure，改变页面或请求状态。
          }catch(error){entryFailure=error.message;status(error.message);}
          // 【逐行批注】 计算右侧结果并更新 finally{enteringLesson，改变页面或请求状态。
          finally{enteringLesson='';setDisabled();}
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 调用 renderAll 执行页面交互或数据处理：if(epoch===activeEpoch)renderAll();。
        if (epoch === activeEpoch && lesson?.entered === true && visitedLessonId !== lesson.id) {
          visitedLessonId = lesson.id;
          const visit = {command:'enter_lesson', lessonId:lesson.id};
          (api?.control ? api.control(visit) : request('/api/learning-agent/control', visit)).catch(() => {});
        }
        if(epoch===activeEpoch)renderAll();
        // 【逐行批注】 声明前端变量 preparationTaskId，保存本行计算出的数据或对象。
        const preparationTaskId=lesson?.teachingTaskId||activePreparation?.id||rememberedPreparationTask();
        // 【逐行批注】 执行当前前端表达式：if(preparationTaskId&&preparationTaskId!==watchingTask){。
        if(preparationTaskId&&preparationTaskId!==watchingTask){
          // 【逐行批注】 调用 paintTask、renderReading 执行页面交互或数据处理：if(activePreparation?.id===preparationTaskId){paintTask(activePrepara…。
          if(activePreparation?.id===preparationTaskId){paintTask(activePreparation);renderReading(true);}
          // 【逐行批注】 调用 observe 执行页面交互或数据处理：observe(preparationTaskId);。
          observe(preparationTaskId);
        // 【逐行批注】 结束当前前端代码块。
        }
      // 【逐行批注】 调用 status、element、button 执行页面交互或数据处理：}catch(error){status(error.message);if(!saved){const container=docume…。
      }catch(error){status(error.message);if(!saved){const container=document.getElementById('dc-reading');container.replaceChildren();const empty=element('div','','dc-empty');empty.append(element('h2','课堂暂时无法读取'),element('p',error.message),button('重新读取','reload'));container.append(empty);}}
    // 【逐行批注】 执行当前前端表达式：})();。
    })();
    // 【逐行批注】 计算右侧结果并更新 try{await refreshPromise;}finally{refreshPro…，改变页面或请求状态。
    try{await refreshPromise;}finally{refreshPromise=null;if(refreshAgain){refreshAgain=false;refresh();}}
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 paintTask，封装可复用的前端行为。
  function paintTask(task) {
    // 【逐行批注】 声明前端变量 previousBusy，保存本行计算出的数据或对象。
    const previousBusy=busy;localFailureRetry=null;lastTask=task;busy=!task.paused&&['pending','running'].includes(task.status);
    // 【逐行批注】 声明前端变量 labels，保存本行计算出的数据或对象。
    const labels={pending:'任务已进入后台队列，可以离开页面并在顶部“AI 状态”查看。',running:'AI 正在处理，已有内容可以继续阅读。',waiting_student:'准备好了，可以继续作答或提问。',ready:'本次内容已保存。',paused:'任务已暂停，已有内容与输入保留。',cancelled:'已停止，已有内容与输入保留。',superseded:'已有更新的学习记录，旧结果未应用。'};
    // 【逐行批注】 调用 status、setDisabled 执行页面交互或数据处理：status(task.status==='failed'?(api?.failureMessage?.(task)||task.erro…。
    status(task.status==='failed'?(api?.failureMessage?.(task)||task.error||'本次生成未完成，可重试。'):
      task.localAction === 'independent_attempt' ? task.message : labels[task.status]||'已更新课堂状态。');setDisabled();
    // 【逐行批注】 执行当前前端表达式：tracePanel?.update(task);。
    tracePanel?.update(task);
    // 【逐行批注】 调用 renderReading 执行页面交互或数据处理：if(previousBusy!==busy)renderReading(true);。
    if(previousBusy!==busy)renderReading(true);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 observe，封装可复用的前端行为。
  function observe(taskId) {
    // 【逐行批注】 执行当前前端表达式：if(!taskId||watchingTask===taskId)return;stopWatching?.();watchingTask=taskId;const epoch…。
    if(!taskId||watchingTask===taskId)return;stopWatching?.();watchingTask=taskId;const epoch=activeEpoch;
    // 【逐行批注】 调用 status 执行页面交互或数据处理：if(!api?.watch){status('实时状态暂不可用，请重新读取已保存的课堂。');return;}。
    if(!api?.watch){status('实时状态暂不可用，请重新读取已保存的课堂。');return;}
    // 【逐行批注】 计算右侧结果并更新 stopWatching，改变页面或请求状态。
    stopWatching=api.watch(taskId,{onEvent(event){if(epoch!==activeEpoch)return false;tracePanel?.event(event);if(event.type==='action'||event.type==='round_feedback')refresh();
      // 【逐行批注】 调用 renderChat 执行页面交互或数据处理：if(event.type==='delta'&&event.field==='message'){let record=pendingM…。
      if(event.type==='delta'&&event.field==='message'){let record=pendingMessages.find(value=>value.taskId===taskId&&value.step===event.step);if(!record){record={id:`${taskId}:${event.step}`,taskId,step:event.step,role:'assistant',text:'',sequences:[],partial:true};pendingMessages.push(record);}record.text+=event.text;record.sequences.push(event.seq);renderChat();}return false;},
      // 【逐行批注】 调用 onState、paintTask、refresh 执行页面交互或数据处理：onState(task){if(epoch!==activeEpoch)return;if(task.lesson?.id&&!scop…。
      onState(task){if(epoch!==activeEpoch)return;if(task.lesson?.id&&!scope.lessonId)scope.lessonId=task.lesson.id;paintTask(task);if(!['pending','running'].includes(task.status)){for(const record of pendingMessages)if(record.taskId===taskId)record.partial=!['ready','waiting_student'].includes(task.status);refresh();}},
      // 【逐行批注】 调用 onConnectionError、status 执行页面交互或数据处理：onConnectionError(message){if(epoch===activeEpoch)status(message);}});。
      onConnectionError(message){if(epoch===activeEpoch)status(message);}});
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 start，封装可复用的前端行为。
  async function start() {
    // 【逐行批注】 计算右侧结果并更新 if(busy||sending||scopeError)return;const st…，改变页面或请求状态。
    if(busy||sending||scopeError)return;const startedAt=Date.now();sending=true;entryFailure='';setDisabled();status('正在准备这一课…');
    // 【逐行批注】 计算右侧结果并更新 try{const value，改变页面或请求状态。
    try{const value=await request('/api/deep-course/start',{...scope,requestId:api?.id?.()||crypto.randomUUID()});if(value.lessonId)scope.lessonId=value.lessonId;if(value.task?.lesson?.id)scope.lessonId=value.task.lesson.id;if(value.task){rememberPreparationTask(value.task);paintTask(value.task);observe(value.task.id);}await refresh();}
    // 【逐行批注】 计算右侧结果并更新 catch(error){status(error.message);feedLocal…，改变页面或请求状态。
    catch(error){status(error.message);feedLocalFailure('开始或恢复本课',error.message,startedAt,()=>start());}finally{sending=false;setDisabled();}
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 sendEvent，封装可复用的前端行为。
  async function sendEvent(event, draft, interactionMediaId) {
    // 【逐行批注】 声明前端变量 courseId，保存本行计算出的数据或对象。
    const courseId=lesson?.courseId||saved?.course?.id||scope.courseId||'';
    // 【逐行批注】 声明前端变量 activeLessonId，保存本行计算出的数据或对象。
    const activeLessonId=lesson?.id||scope.lessonId||'';
    // 【逐行批注】 执行当前前端表达式：if(!courseId||sending||scopeError){。
    if(!courseId||sending||scopeError){
      // 【逐行批注】 调用 status 执行页面交互或数据处理：if(sending)status('上一条内容仍在提交，请稍候。');。
      if(sending)status('上一条内容仍在提交，请稍候。');
      // 【逐行批注】 当前一条件不成立时继续判断：(!courseId)status('课程信息尚未读取，暂不能发送问题。');。
      else if(!courseId)status('课程信息尚未读取，暂不能发送问题。');
      // 【逐行批注】 结束当前函数并返回结果：return;。
      return;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 声明前端变量 startedAt，保存本行计算出的数据或对象。
    const startedAt=Date.now();
    // 点击后到服务端返回之间必须有可见文案，不能只把按钮灰掉。
    // 【逐行批注】 声明前端变量 busyHost，保存本行计算出的数据或对象。
    const busyHost=draft?.node?.closest?.('form')||document.getElementById('dc-chat-form');
    // 【逐行批注】 计算右侧结果并更新 sending，改变页面或请求状态。
    sending=true;setDisabled();status('正在提交给 AI…');busyHost?.setAttribute('aria-busy','true');
    // 【逐行批注】 声明前端变量 viewport，保存本行计算出的数据或对象。
    const viewport=document.getElementById('dc-reading').getBoundingClientRect();
    // 【逐行批注】 声明前端变量 visibleSection，保存本行计算出的数据或对象。
    const visibleSection=[...mount.querySelectorAll('.dc-lecture-section')].find(node=>{const box=node.getBoundingClientRect();return box.bottom>viewport.top+12&&box.top<viewport.bottom;});
    // 【逐行批注】 声明前端变量 contentId，保存本行计算出的数据或对象。
    let deepView=currentView;
    let contentId=currentView==='slides'?items(saved?.content?.slides)[currentSlide]?.id:visibleSection?.dataset.sectionId||items(saved?.content?.lecture)[0]?.id;
    if(interactionMediaId){
      // 解释当前组件必须绑定该组件所在的保存板块，不能按视口顶端猜测页面。
      const candidates=items(saved?.content?.[deepView]);
      const block=(deepView==='slides'?candidates[currentSlide]:null)||candidates.find(item=>items(item.mediaIds).includes(interactionMediaId));
      if(!block||!items(block.mediaIds).includes(interactionMediaId)){
        sending=false;setDisabled();busyHost?.removeAttribute('aria-busy');
        throw new Error('当前互动与课堂板块已变化，请重新打开这一内容后再请求解释。');
      }
      contentId=block.id;
    }
    // 【逐行批注】 声明前端变量 mediaState，保存本行计算出的数据或对象。
    const mediaState=Object.fromEntries([...mediaValues].filter(([id])=>!interactionMediaId||id===interactionMediaId).filter(([,value])=>
      (typeof value==='number'&&Number.isFinite(value)) ||
      (value&&typeof value==='object'&&value.schemaVersion===1)).slice(0,32));
    const componentActivities = {};
    const attached = items(saved?.content?.[deepView]).find(block => block.id === contentId)?.mediaIds || [];
    for (const id of attached) if (mediaState[id] && (!interactionMediaId || id === interactionMediaId)) {
      try {
        const data = componentTraces.get(id) || JSON.parse(restore(`interactive-activity:${id}`) || 'null');
        const media = items(saved?.content?.media).find(item => item.id === id);
        if (data?.signature === JSON.stringify([saved?.content?.version || 1, media]) && data.trace.events.length)
          componentActivities[id] = data.trace;
      } catch (_) { /* 损坏草稿不发送为操作证据。 */ }
    }
    // 【逐行批注】 声明前端变量 payload，保存本行计算出的数据或对象。
    const payload={...event,courseId,...(activeLessonId?{lessonId:activeLessonId}:{}),view:event.type==='question_answer'?'practice':'learn',deepView,...(contentId?{contentId}:{}),mediaState,componentActivities,requestId:api?.id?.()||crypto.randomUUID()};
    // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
    const key=storageKey(`pending:${event.type}:${event.sectionId || event.roundRevision?.roundId || 'chat'}`);let pending;
    // 【逐行批注】 计算右侧结果并更新 try{pending，改变页面或请求状态。
    try{pending=JSON.parse(api?.storage.get(key)||'null');}catch(_){ /* 本地损坏的请求记录重新建立。 */ }
    // 【逐行批注】 执行当前前端表达式：if(pending&&pending.text===payload.text&&pending.type===payload.type&&pending.sectionVers…。
    if(pending&&pending.text===payload.text&&pending.type===payload.type&&pending.sectionVersion===payload.sectionVersion&&
      pending.requestType===payload.requestType&&pending.teachingPreferenceVersion===payload.teachingPreferenceVersion)payload.requestId=pending.requestId;
    // 【逐行批注】 执行当前前端表达式：api?.storage.set(key,JSON.stringify(payload));。
    api?.storage.set(key,JSON.stringify(payload));
    // 【逐行批注】 声明前端变量 accepted，保存本行计算出的数据或对象。
    let accepted=false;
    // 【逐行批注】 计算右侧结果并更新 try{exposeVisible();const task，改变页面或请求状态。
    try{exposeVisible();const task=api?.submit?await api.submit(payload):await request('/api/learning-agent/events',payload);api?.storage.remove(key);
      // 【逐行批注】 执行当前前端表达式：if(event.text&&event.type!=='lesson_finish_request')pendingMessages.push({id:`${task.id}:…。
      if(event.text&&event.type!=='lesson_finish_request')pendingMessages.push({id:`${task.id}:user`,taskId:task.id,role:'user',text:event.text,sectionId:event.sectionId||''});
      // 【逐行批注】 调用 store 执行页面交互或数据处理：if(draft){store(draft.name,'');if(draft.node?.value===draft.text)draf…。
      if(draft){store(draft.name,'');if(draft.node?.value===draft.text)draft.node.value='';}
      // 【逐行批注】 计算右侧结果并更新 paintTask(task);observe(task.id);renderReadi…，改变页面或请求状态。
      paintTask(task);observe(task.id);renderReading(true);accepted=true;
    // 【逐行批注】 计算右侧结果并更新 }catch(error){status(error.message);feedLoca…，改变页面或请求状态。
    }catch(error){status(error.message);if(event.requestType==='revise_round')return error.message;feedLocalFailure('提交课堂互动',error.message,startedAt,()=>sendEvent(event,draft,interactionMediaId));}
    // 【逐行批注】 计算右侧结果并更新 finally{sending，改变页面或请求状态。
    finally{sending=false;busyHost?.removeAttribute('aria-busy');setDisabled();}
    // 【逐行批注】 计算右侧结果并更新 if(accepted)refresh().catch(error，改变页面或请求状态。
    if(accepted)refresh().catch(error=>status('问题已发送，但页面同步失败：'+error.message));
    return accepted;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 handleSubmit，封装可复用的前端行为。
  function handleSubmit(event) {
    // 【逐行批注】 调用 sendEvent 执行页面交互或数据处理：if(event.target.id==='dc-course-reply-form'){event.preventDefault();c…。
    if(event.target.id==='dc-course-reply-form'){event.preventDefault();const input=document.getElementById('dc-course-reply-input');const text=input.value.trim();if(!text){input.focus();return;}sendEvent({type:'chat',text},{name:'course-reply-draft',node:input,text:input.value});return;}
    // 【逐行批注】 调用 sendIndependentChat 执行页面交互或数据处理：if(event.target.id==='dc-chat-form'){event.preventDefault();const inp…。
    if(event.target.id==='dc-chat-form'){event.preventDefault();const input=document.getElementById('dc-chat-input');if(!input.value.trim()){input.focus();return;}sendIndependentChat(input.value,{name:'chat-draft',node:input,text:input.value});return;}
    // 【逐行批注】 声明前端变量 id，保存本行计算出的数据或对象。
    const id=event.target.dataset.questionId;if(!id)return;event.preventDefault();const section=items(lesson?.sections).find(value=>value.id===id);if(!section)return;
    // 【逐行批注】 声明前端变量 form，保存本行计算出的数据或对象。
    const form=event.target,answer=form.querySelector('textarea'),choices=[...form.querySelectorAll('input:checked')].map(input=>Number(input.value));
    // 【逐行批注】 声明前端变量 selected，保存本行计算出的数据或对象。
    const selected=choices.map(index=>choiceText(section.question.options[index],index));const text=[...selected,answer.value.trim()].filter(Boolean).join('\n');
    // 【逐行批注】 调用 status 执行页面交互或数据处理：if(!text){status('先选择答案或写下你的思路，再提交。');answer.focus();return;}。
    if(!text){status('先选择答案或写下你的思路，再提交。');answer.focus();return;}
    // 【逐行批注】 调用 sendEvent 执行页面交互或数据处理：sendEvent({type:'question_answer',action:'answer',text,sectionId:sect…。
    sendEvent({type:'question_answer',action:'answer',text,sectionId:section.id,sectionVersion:section.version},{name:`answer:${section.id}`,node:answer,text:answer.value});
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 waitMediaTask，封装可复用的前端行为。
  async function waitMediaTask(task, epoch) {
    // 【逐行批注】 声明前端变量 current，保存本行计算出的数据或对象。
    let current=task;
    // 【逐行批注】 在条件成立期间循环处理：while(['pending','running'].includes(current.status)&&!current.paused){。
    while(['pending','running'].includes(current.status)&&!current.paused){
      // 【逐行批注】 计算右侧结果并更新 await new Promise(resolve，改变页面或请求状态。
      await new Promise(resolve=>setTimeout(resolve,400));
      // 【逐行批注】 调用 Error 执行页面交互或数据处理：if(epoch!==activeEpoch)throw new Error('页面已经离开，未继续重试其他素材。');。
      if(epoch!==activeEpoch)throw new Error('页面已经离开，未继续重试其他素材。');
      // 【逐行批注】 计算右侧结果并更新 current，改变页面或请求状态。
      current=await request('/api/learning-agent?taskId='+encodeURIComponent(current.id));
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 调用 Error 执行页面交互或数据处理：if(current.paused||!['ready','waiting_student'].includes(current.stat…。
    if(current.paused||!['ready','waiting_student'].includes(current.status))throw new Error(current.error||'素材重试已暂停或未完成，其余素材保留原状态。');
    // 【逐行批注】 结束当前函数并返回结果：return current;。
    return current;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 retryMedia，封装可复用的前端行为。
  async function retryMedia(id = '', repair = false) {
    // 【逐行批注】 调用 items 执行页面交互或数据处理：if(!lesson||lesson.entered!==true||busy||sending)return;const media=i…。
    if(!lesson||lesson.entered!==true||busy||sending)return;const media=id?[id]:items(saved?.content?.media).filter(value=>value.status==='failed').map(value=>value.id);
    // 【逐行批注】 调用 status、setDisabled 执行页面交互或数据处理：if(!media.length){status('当前没有需要重试的失败素材。');return;}sending=true;setDi…。
    if(!media.length){status('当前没有需要重试的失败素材。');return;}sending=true;setDisabled();
    // 【逐行批注】 声明前端变量 epoch，保存本行计算出的数据或对象。
    const epoch=activeEpoch;
    // 【逐行批注】 计算右侧结果并更新 try{for(const mediaId of media){const value，改变页面或请求状态。
    try{for(const mediaId of media){const value=await request('/api/deep-course/retry-media',{lessonId:lesson.id,mediaId,repair,requestId:api?.id?.()||crypto.randomUUID()});if(value.task){paintTask(value.task);observe(value.task.id);await waitMediaTask(value.task,epoch);}await refresh();}}
    // 【逐行批注】 计算右侧结果并更新 catch(error){status(error.message);}finally{…，改变页面或请求状态。
    catch(error){status(error.message);}finally{sending=false;setDisabled();}
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 continueLearning，封装可复用的前端行为。
  function continueLearning() {
    // 【逐行批注】 执行当前前端表达式：if(scopeError)return;。
    if(scopeError)return;
    // 【逐行批注】 调用 scrollBehavior、status 执行页面交互或数据处理：if(busy||sending){tracePanel?.root.scrollIntoView({block:'nearest',be…。
    if(busy||sending){tracePanel?.root.scrollIntoView({block:'nearest',behavior:scrollBehavior()});status('AI 正在生成，已有步骤会持续显示在这里；可以离开页面并在顶部“AI 状态”查看。');return;}
    // 【逐行批注】 调用 start 执行页面交互或数据处理：if(!lesson){start();return;}。
    if(!lesson){start();return;}
    // 【逐行批注】 计算右侧结果并更新 if(lesson.completed){const next，改变页面或请求状态。
    if(lesson.completed){const next=nextCourseTopic();if(next)openTopic(next);else showDirectory();return;}
    // 【逐行批注】 执行当前前端表达式：if(lesson.entered!==true)return;。
    if(lesson.entered!==true)return;
    if(lesson.teachingPreferences?.guidance==='independent'){
      sendEvent({type:'teaching_request',requestType:'resume_guidance',text:'请继续引导我学习当前内容。',
        teachingPreferenceVersion:lesson.teachingPreferences.version});return;
    }
    // 【逐行批注】 声明前端变量 questions，保存本行计算出的数据或对象。
    const questions=lessonQuestions(),round=lesson.teachingRound;
    const unanswered=round
      ? questions.find(section=>section.id===round.pendingSectionId)
      : questions.find(section=>!evaluationsFor(section).length);
    // 【逐行批注】 执行当前前端表达式：if(unanswered){。
    if(unanswered){
      // 【逐行批注】 调用 store、renderReading 执行页面交互或数据处理：if(currentView!=='lecture'){readingPositions.set(currentView,document…。
      if(currentView!=='lecture'){readingPositions.set(currentView,document.getElementById('dc-reading').scrollTop);currentView='lecture';store('view',currentView);renderReading(false);}
      // 【逐行批注】 声明前端变量 question，保存本行计算出的数据或对象。
      const question=[...document.querySelectorAll('#dc-reading .dc-question')].find(node=>node.dataset.sectionId===unanswered.id);
      // 【逐行批注】 调用 scrollBehavior 执行页面交互或数据处理：question?.scrollIntoView({block:'start',behavior:scrollBehavior()});q…。
      question?.scrollIntoView({block:'start',behavior:scrollBehavior()});question?.querySelector('input,textarea')?.focus({preventScroll:true});
      // 【逐行批注】 调用 status 执行页面交互或数据处理：status('先尝试这道尚未评价的练习，AI 会根据你的真实回答继续教学。');return;。
      status('先尝试这道尚未评价的练习，AI 会根据你的真实回答继续教学。');return;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 调用 toggleChat 执行页面交互或数据处理：toggleChat(true);。
    toggleChat(true);
    // 【逐行批注】 调用 sendEvent 执行页面交互或数据处理：sendEvent(questions.length?{type:'lesson_finish_request',text:'请根据本节真…。
    if(round?.status==='active'){
      sendEvent({type:'chat',text:'请从本轮已经保存的进度继续，按当前题数安排下一题或本轮总结。'});return;
    }
    sendEvent(questions.length?{type:'lesson_finish_request',text:'请根据本节真实作答和已有评价判断下一步：证据充分时判断本节是否完成；证据不足时继续教学或安排需要的练习，不因阅读或翻页认定掌握。'}:
      // 【逐行批注】 执行当前前端表达式：{type:'chat',text:'请结合本节已保存的学习内容与真实记录，判断适合的下一步教学；需要练习或澄清时继续安排，不因阅读认定掌握。'});。
      {type:'chat',text:'请结合本节已保存的学习内容与真实记录，判断适合的下一步教学；需要练习或澄清时继续安排，不因阅读认定掌握。'});
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 handleClick，封装可复用的前端行为。
  async function handleClick(event) {
    // 【逐行批注】 声明前端变量 control，保存本行计算出的数据或对象。
    const control=event.target.closest('[data-action]');if(!control)return;const action=control.dataset.action;
    // 【逐行批注】 调用 clearIndependentChat 执行页面交互或数据处理：if(action==='clear-chat'){clearIndependentChat();return;}。
    if(action==='clear-chat'){clearIndependentChat();return;}
    // 【逐行批注】 调用 toggleIndependentWeb 执行页面交互或数据处理：if(action==='toggle-web'){toggleIndependentWeb();return;}。
    if(action==='toggle-web'){toggleIndependentWeb();return;}
    // 【逐行批注】 调用 retryIndependentChat 执行页面交互或数据处理：if(action==='retry-independent'){retryIndependentChat();return;}。
    if(action==='retry-independent'){retryIndependentChat();return;}
    if (action === 'fullscreen') {
      if (fullscreenMode) exitClassroomFullscreen();
      else enterClassroomFullscreen();
      return;
    }
    // 【逐行批注】 调用 exitClassroomFullscreen 执行页面交互或数据处理：if(action==='fullscreen-exit'){exitClassroomFullscreen();return;}。
    if(action==='fullscreen-exit'){exitClassroomFullscreen();return;}
    // 【逐行批注】 调用 toggleChat 执行页面交互或数据处理：if(action==='fullscreen-chat'){toggleChat(mount.dataset.chatOpen!=='t…。
    if(action==='fullscreen-chat'){toggleChat(mount.dataset.chatOpen!=='true');return;}
    // 【逐行批注】 调用 toggleChat 执行页面交互或数据处理：if(action==='close-fullscreen-chat'){toggleChat(false);return;}。
    if(action==='close-fullscreen-chat'){toggleChat(false);return;}
    // 【逐行批注】 调用 matchMedia、String、store、updatePanels 执行页面交互或数据处理：if(action==='directory'){if(matchMedia('(max-width:700px)').matches||…。
    if (action === 'directory') {
      if (fullscreenMode || matchMedia('(max-width:700px)').matches ||
          matchMedia('(max-width:900px)').matches && mount.dataset.chatOpen === 'true') {
        learningTools?.close();
        document.getElementById('dc-outline-dialog').showModal();
      } else {
        mount.dataset.directoryOpen = String(mount.dataset.directoryOpen !== 'true');
        store('directory-open', mount.dataset.directoryOpen === 'true' ? '1' : '0');
      }
      updatePanels();
      return;
    }
    // 【逐行批注】 调用 updatePanels 执行页面交互或数据处理：if(action==='close-directory'){document.getElementById('dc-outline-di…。
    if(action==='close-directory'){document.getElementById('dc-outline-dialog').close();updatePanels();return;}
    // 【逐行批注】 调用 toggleChat 执行页面交互或数据处理：if(action==='chat'){toggleChat(mount.dataset.chatOpen!=='true');retur…。
    if(action==='chat'){toggleChat(mount.dataset.chatOpen!=='true');return;}if(action==='close-chat'){toggleChat(false);return;}
    // 【逐行批注】 计算右侧结果并更新 if(action.startsWith('view:')){readingPositi…，改变页面或请求状态。
    if (action.startsWith('view:')) {
      readingPositions.set(currentView, document.getElementById('dc-reading').scrollTop);
      currentView = action.slice(5);
      store('view', currentView);
      renderReading(false);
      if (event.isTrusted) learningTools?.close();
      return;
    }
    // 【逐行批注】 调用 store、renderReading 执行页面交互或数据处理：if(action==='previous'||action==='next'){currentSlide+=action==='next…。
    if(action==='previous'||action==='next'){currentSlide+=action==='next'?1:-1;store('slide',currentSlide);store('scroll:slides',0);readingPositions.set('slides',0);renderReading(false);return;}
    // 【逐行批注】 调用 openTopic 执行页面交互或数据处理：if(action==='topic'){openTopic({id:control.dataset.topicId,legacyPhas…。
    if(action==='topic'){openTopic({id:control.dataset.topicId,legacyPhaseIndex:control.dataset.phaseIndex,legacyTopicIndex:control.dataset.topicIndex});return;}
    // 【逐行批注】 调用 start、refresh、retryMedia 执行页面交互或数据处理：if(action==='start'){start();return;}if(action==='reload'||action==='…。
    if(action==='start'){start();return;}if(action==='reload'||action==='enter'){entryFailure='';refresh();return;}if(action==='media'){retryMedia(control.dataset.mediaId,control.dataset.repair==='true');return;}if(action==='retry-media'){retryMedia();return;}
    // 【逐行批注】 执行当前前端表达式：control.closest('.dc-more')?.removeAttribute('open');。
    control.closest('.dc-more')?.removeAttribute('open');
    // 【逐行批注】 调用 continueLearning 执行页面交互或数据处理：if(action==='continue'){continueLearning();return;}。
    if(action==='continue'){continueLearning();return;}
    if(action==='revise-round'){api.showRoundRevision(lesson?.teachingRound, value=>sendEvent(value));return;}
    // 【逐行批注】 调用 sendEvent 执行页面交互或数据处理：if(action==='finish'){sendEvent({type:'lesson_finish_request',text:'请…。
    if(action==='finish'){sendEvent({type:'lesson_finish_request',text:'请根据本节真实作答和已有评价判断完成情况，证据不足时明确说明。'});return;}
    // 【逐行批注】 执行当前前端表达式：if((action==='stop'||action==='retry')&&watchingTask){。
    if((action==='stop'||action==='retry')&&watchingTask){
      // 【逐行批注】 声明前端变量 control，保存本行计算出的数据或对象。
      const control=action==='stop'?document.getElementById('dc-stop'):document.getElementById('dc-retry');
      // 【逐行批注】 声明前端变量 previous，保存本行计算出的数据或对象。
      const previous=control?.textContent;if(control){control.disabled=true;control.textContent=action==='stop'?'正在停止…':'正在重试…';}
      // 【逐行批注】 计算右侧结果并更新 try{const task，改变页面或请求状态。
      try{const task=api?.control?await api.control({command:action==='stop'?'cancel':'retry',taskId:watchingTask}):await request('/api/learning-agent/control',{command:action==='stop'?'cancel':'retry',taskId:watchingTask});watchingTask='';paintTask(task);observe(task.id);}
      // 【逐行批注】 调用 status 执行页面交互或数据处理：catch(error){status(error.message);}。
      catch(error){status(error.message);}
      // 【逐行批注】 计算右侧结果并更新 finally{if(control)control.textContent，改变页面或请求状态。
      finally{if(control)control.textContent=previous||control.textContent;}
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 handleKey，封装可复用的前端行为。
  function handleKey(event) {
    // 【逐行批注】 执行当前前端表达式：if(event.target.id==='dc-chat-input'&&event.key==='Enter'&&!event.shiftKey&&!event.isComp…。
    if(event.target.id==='dc-chat-input'&&event.key==='Enter'&&!event.shiftKey&&!event.isComposing){event.preventDefault();event.target.closest('form').requestSubmit();}
    // 【逐行批注】 执行当前前端表达式：if(event.target.id==='dc-course-reply-input'&&event.key==='Enter'&&!event.shiftKey&&!even…。
    if(event.target.id==='dc-course-reply-input'&&event.key==='Enter'&&!event.shiftKey&&!event.isComposing){event.preventDefault();event.target.closest('form').requestSubmit();}
    // 【逐行批注】 计算右侧结果并更新 if(event.target.matches('.dc-tab,.dc-fs-tab'…，改变页面或请求状态。
    if (event.target.matches('.dc-tab,.dc-fs-tab') && ['ArrowLeft','ArrowRight','Home','End'].includes(event.key)) {
      event.preventDefault();
      const tabs = [...event.target.closest('[role=tablist]').querySelectorAll('[role=tab]')];
      const index = event.key === 'Home' ? 0 : event.key === 'End' ? tabs.length - 1 :
        (tabs.indexOf(event.target) + (event.key === 'ArrowRight' ? 1 : -1) + tabs.length) % tabs.length;
      tabs[index]?.click();
      tabs[index]?.focus();
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 调用 buildShell 执行页面交互或数据处理：buildShell();。
  buildShell();
  // 【逐行批注】 调用 applyFullscreenUi 执行页面交互或数据处理：applyFullscreenUi();。
  applyFullscreenUi();
  // 【逐行批注】 执行当前前端表达式：if(window.GangyiTrace){。
  if(window.GangyiTrace){
    // 【逐行批注】 计算右侧结果并更新 tracePanel，改变页面或请求状态。
    tracePanel=window.GangyiTrace.create(document.getElementById('dc-trace-host'),{
      // 【逐行批注】 执行当前前端表达式：variant:'inline',eyebrow:'AI 备课执行过程',collapseWhenDone:true,defaultCollapsed:true,。
      variant:'inline',eyebrow:'AI 备课执行过程',collapseWhenDone:true,defaultCollapsed:true,
      // 【逐行批注】 计算右侧结果并更新 onRetry:()，改变页面或请求状态。
      onRetry:()=>retryTraceTask(),
      // 【逐行批注】 计算右侧结果并更新 onResume:async()，改变页面或请求状态。
      onResume:async()=>{if(!lastTask?.id)return;const task=await api.control({command:'resume',taskId:lastTask.id});watchingTask='';paintTask(task);observe(task.id);},
      // 【逐行批注】 计算右侧结果并更新 onPause:async()，改变页面或请求状态。
      onPause:async()=>{if(!lastTask?.id)return;const task=await api.control({command:'pause',taskId:lastTask.id});paintTask(task);},
      // 【逐行批注】 计算右侧结果并更新 onCancel:async()，改变页面或请求状态。
      onCancel:async()=>{if(!lastTask?.id)return;const task=await api.control({command:'cancel',taskId:lastTask.id});paintTask(task);},
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 计算右侧结果并更新 if(scopeError){const reading，改变页面或请求状态。
  if(scopeError){const reading=document.getElementById('dc-reading');reading.replaceChildren(element('div',scopeError,'dc-empty'));status(scopeError);setDisabled();return;}
  // 【逐行批注】 计算右侧结果并更新 refresh().then(()，改变页面或请求状态。
  refresh().then(()=>{
    // 【逐行批注】 计算右侧结果并更新 currentView，改变页面或请求状态。
    currentView=restore('view')==='slides'?'slides':'lecture';currentSlide=Math.max(0,finite(restore('slide')));mount.dataset.directoryOpen=restore('directory-open')==='0'?'false':'true';renderReading(false);
    // 【逐行批注】 执行当前前端表达式：if(fullscreenMode){。
    if(fullscreenMode){
      // 【逐行批注】 计算右侧结果并更新 lastFullscreenCompact，改变页面或请求状态。
      lastFullscreenCompact=matchMedia('(max-width:1000px)').matches;
      // 【逐行批注】 调用 setSystemFullscreen 执行页面交互或数据处理：setSystemFullscreen(true);。
      setSystemFullscreen(true);
      // 【逐行批注】 调用 toggleChat、restore 执行页面交互或数据处理：toggleChat(lastFullscreenCompact?restore('chat-open')==='1':true,fals…。
      toggleChat(lastFullscreenCompact?restore('chat-open')==='1':true,false);
    // 【逐行批注】 调用 restore、matchMedia、toggleChat 执行页面交互或数据处理：}else if(restore('chat-open')==='1'&&!matchMedia('(max-width:700px)')…。
    }else if(restore('chat-open')==='1'&&!matchMedia('(max-width:700px)').matches)toggleChat(true,false);
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('visibilitychange',exposeVisible);。
  document.addEventListener('visibilitychange',exposeVisible);
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('fullscreenchange',()=>{。
  document.addEventListener('fullscreenchange',()=>{
    // 【逐行批注】 执行当前前端表达式：if(fullscreenMode&&!document.fullscreenElement&&typeof window.gangyiSetClassroomFullscree…。
    if(fullscreenMode&&!document.fullscreenElement&&typeof window.gangyiSetClassroomFullscreen!=='function'){
      // 【逐行批注】 计算右侧结果并更新 fullscreenMode，改变页面或请求状态。
      fullscreenMode=false;storeFullscreenMode(false);applyFullscreenUi();toggleChat(chatOpenBeforeFullscreen==='true',false);
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('pagehide',()=>{const chatInput=document.getElementById('dc-…。
  window.addEventListener('pagehide',()=>{const chatInput=document.getElementById('dc-chat-input');if(chatInput)store('chat-draft',chatInput.value);const courseReplyInput=document.getElementById('dc-course-reply-input');if(courseReplyInput)store('course-reply-draft',courseReplyInput.value);stopWatching?.();stopIndependentWatch?.();independentChatEpoch++;if(chatRenderFrame){cancelAnimationFrame(chatRenderFrame);chatRenderFrame=0;}chatNodeCache.clear();activeEpoch++;});
// 【逐行批注】 执行当前前端表达式：})();。
})();

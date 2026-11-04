/* 页面导航与 AI 状态各自集中呈现，共用稳定课程和课时上下文。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 声明前端变量 api，保存本行计算出的数据或对象。
  const api = window.GangyiAgent;
  // 【逐行批注】 判断条件 (!api || document.getElementById('learning-navigation')) return;，成立时执行对应逻辑。
  if (!api || document.getElementById('learning-navigation')) return;
  // 【逐行批注】 声明前端变量 element，保存本行计算出的数据或对象。
  const element = (tag, text = '', className = '') => { const node = document.createElement(tag); node.textContent = text; node.className = className; return node; };
  // 变值才写：轮询每秒执行，重复写入相同文本会让 aria-live 反复播报并丢失键盘焦点。
  // 【逐行批注】 声明前端变量 setText，保存本行计算出的数据或对象。
  const setText = (node, value) => {
    // 【逐行批注】 判断条件 (!node) return;，成立时执行对应逻辑。
    if (!node) return;
    // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
    const text = String(value ?? '');
    // 【逐行批注】 判断条件 (node.__gyText !== text) { node.__gyText = text; node.textConten…，成立时执行对应逻辑。
    if (node.__gyText !== text) { node.__gyText = text; node.textContent = text; }
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 context，保存本行计算出的数据或对象。
  let context = Object.fromEntries(new URLSearchParams(location.search)), current, course, outline = [], outlineCourse = '', courseVersion = 0, stateVersion = 0;
  // 【逐行批注】 声明前端变量 localLink，保存本行计算出的数据或对象。
  const localLink = value => typeof value === 'string' && value.startsWith('/') && !value.startsWith('//');
  // 【逐行批注】 定义函数 anchor，封装可复用的前端行为。
  function anchor(text, href, className = '') { const node = element('a', text, className); node.href = href; return node; }
  // 【逐行批注】 定义函数 paintCurrent，封装可复用的前端行为。
  function paintCurrent(value) {
    // 【逐行批注】 按循环规则遍历或重复处理：for (const node of document.querySelectorAll('[data-current-course-link]')…。
    for (const node of document.querySelectorAll('[data-current-course-link]')) {
      // 【逐行批注】 计算右侧结果并更新 node.href，改变页面或请求状态。
      node.href = '/current-course'; node.removeAttribute('aria-disabled'); node.removeAttribute('tabindex');
      // 【逐行批注】 计算右侧结果并更新 node.title，改变页面或请求状态。
      node.title = value && localLink(value.href) ? value.title || '当前课程' : '查看当前课程';
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 restoreCurrent，封装可复用的前端行为。
  async function restoreCurrent() {
    // 【逐行批注】 计算右侧结果并更新 try { const data，改变页面或请求状态。
    try { const data = await api.request('/api/courses'); paintCurrent(data.currentCourse); }
    // 【逐行批注】 执行当前前端表达式：catch (_) { /* 读取失败时保留已经核实的入口，不跳到课程列表冒充当前课程。 * / }。
    catch (_) { /* 读取失败时保留已经核实的入口，不跳到课程列表冒充当前课程。 */ }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 top，保存本行计算出的数据或对象。
  const top = element('nav', '', 'course-context-navigation'); top.setAttribute('aria-label', '课程内导航'); top.hidden = true;
  // 【逐行批注】 声明前端变量 crumbs，保存本行计算出的数据或对象。
  const crumbs = element('div', '', 'course-breadcrumbs'), moves = element('div', '', 'course-context-actions'); top.append(crumbs, moves);
  // 【逐行批注】 执行当前前端表达式：document.querySelector('main')?.prepend(top);。
  document.querySelector('main')?.prepend(top);
  // 【逐行批注】 定义函数 renderContext，封装可复用的前端行为。
  function renderContext() {
    // 【逐行批注】 计算右侧结果并更新 top.hidden，改变页面或请求状态。
    top.hidden = !course || !['/plan', '/phase', '/learn', '/practice', '/summary', '/agent-prepare.html'].includes(location.pathname);
    // 【逐行批注】 执行当前前端表达式：crumbs.replaceChildren(); moves.replaceChildren(); if (top.hidden) return;。
    crumbs.replaceChildren(); moves.replaceChildren(); if (top.hidden) return;
    // 【逐行批注】 计算右侧结果并更新 crumbs.append(anchor('我的课程', '/my-courses'),…，改变页面或请求状态。
    crumbs.append(anchor('我的课程', '/my-courses'), element('span', '／', 'breadcrumb-separator'), anchor(course.title || '当前课程', '/plan?courseId=' + encodeURIComponent(course.id)));
    // 【逐行批注】 声明前端变量 position，保存本行计算出的数据或对象。
    const position = outline.findIndex(item => context.topicId ? item.id === context.topicId : Number(item.legacyPhaseIndex) === Number(context.phaseIndex) && Number(item.legacyTopicIndex) === Number(context.topicIndex));
    // 【逐行批注】 声明前端变量 phase，保存本行计算出的数据或对象。
    const phase = position >= 0 ? outline[position].displayPhase : Number(context.phaseIndex || 0);
    // 【逐行批注】 判断条件 (phase) crumbs.append(element('span', '／', 'breadcrumb-separator…，成立时执行对应逻辑。
    if (phase) crumbs.append(element('span', '／', 'breadcrumb-separator'), anchor('阶段 ' + phase, '/phase?' + new URLSearchParams({ courseId: course.id, phaseIndex: phase })));
    // 【逐行批注】 判断条件 (position >= 0 && context.lessonId) {，成立时执行对应逻辑。
    if (position >= 0 && context.lessonId) {
      // 【逐行批注】 声明前端变量 href，保存本行计算出的数据或对象。
      const href = item => '/learn?' + new URLSearchParams({ courseId: course.id, topicId: item.id || '', phaseIndex: item.legacyPhaseIndex, topicIndex: item.legacyTopicIndex });
      // 【逐行批注】 判断条件 (position > 0) moves.append(anchor('← 上一节', href(outline[positio…，成立时执行对应逻辑。
      if (position > 0) moves.append(anchor('← 上一节', href(outline[position - 1])));
      // 【逐行批注】 判断条件 (position < outline.length - 1) moves.append(anchor('下一节 →', hre…，成立时执行对应逻辑。
      if (position < outline.length - 1) moves.append(anchor('下一节 →', href(outline[position + 1])));
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 try { const saved，改变页面或请求状态。
    try { const saved = JSON.parse(api.storage.get('current-lesson') || 'null'); if (saved && localLink(saved.href) && !context.lessonId) moves.append(anchor('回到当前课时 →', saved.href)); }
    // 【逐行批注】 执行当前前端表达式：catch (_) { /* 损坏缓存不产生跳转。 * / }。
    catch (_) { /* 损坏缓存不产生跳转。 */ }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 setContext，封装可复用的前端行为。
  async function setContext(value) {
    // 【逐行批注】 声明前端变量 previousTask，保存本行计算出的数据或对象。
    const previousTask = context.teachingTaskId || context.taskId || '';
    // 【逐行批注】 计算右侧结果并更新 context，改变页面或请求状态。
    context = { ...context, ...value };
    // 【逐行批注】 判断条件 (previousTask !== (context.teachingTaskId || context.taskId || '…，成立时执行对应逻辑。
    if (previousTask !== (context.teachingTaskId || context.taskId || '')) load();
    // 【逐行批注】 调用 CustomEvent 执行页面交互或数据处理：document.dispatchEvent(new CustomEvent('gangyi:navigation-context', {…。
    document.dispatchEvent(new CustomEvent('gangyi:navigation-context', { detail: context }));
    // 【逐行批注】 判断条件 (!context.courseId) { renderContext(); return; }，成立时执行对应逻辑。
    if (!context.courseId) { renderContext(); return; }
    // 【逐行批注】 判断条件 (outlineCourse === context.courseId && course) { renderContext()…，成立时执行对应逻辑。
    if (outlineCourse === context.courseId && course) { renderContext(); return; }
    // 【逐行批注】 声明前端变量 id，保存本行计算出的数据或对象。
    const id = context.courseId, version = ++courseVersion;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
      const data = await api.request('/api/courses/' + encodeURIComponent(id)); if (version !== courseVersion || id !== context.courseId) return;
      // 【逐行批注】 计算右侧结果并更新 course，改变页面或请求状态。
      course = data.course; outlineCourse = id;
      // 【逐行批注】 计算右侧结果并更新 outline，改变页面或请求状态。
      outline = (data.snapshot?.payload?.courseStructure || []).flatMap((stage, index) => (stage.topics || []).map((value, topicIndex) => {
        // 【逐行批注】 声明前端变量 topic，保存本行计算出的数据或对象。
        const topic = typeof value === 'object' && value ? value : { title: String(value || '') };
        // 【逐行批注】 结束当前函数并返回结果：return { ...topic, displayPhase: index + 1, legacyPhaseIndex: topic.l…。
        return { ...topic, displayPhase: index + 1, legacyPhaseIndex: topic.legacyPhaseIndex || index + 1, legacyTopicIndex: topic.legacyTopicIndex || topicIndex + 1 };
      // 【逐行批注】 执行当前前端表达式：}));。
      }));
      // 【逐行批注】 计算右侧结果并更新 paintCurrent({ ...course, href: '/plan?cours…，改变页面或请求状态。
      paintCurrent({ ...course, href: '/plan?courseId=' + encodeURIComponent(id) }); renderContext();
    // 【逐行批注】 调用 renderContext、restoreCurrent 执行页面交互或数据处理：} catch (_) { if (version === courseVersion) { course = null; outline…。
    } catch (_) { if (version === courseVersion) { course = null; outlineCourse = ''; renderContext(); restoreCurrent(); } }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 计算右侧结果并更新 window.GangyiNavigation，改变页面或请求状态。
  window.GangyiNavigation = { setContext, restoreCurrent };
  // 【逐行批注】 调用 paintCurrent、restoreCurrent、setContext 执行页面交互或数据处理：paintCurrent(null); restoreCurrent(); setContext(context);。
  paintCurrent(null); restoreCurrent(); setContext(context);
  // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('focus', restoreCurrent); window.addEventListener('storage',…。
  window.addEventListener('focus', restoreCurrent); window.addEventListener('storage', restoreCurrent);
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('visibilitychange', () => { if (!document.hidden) restoreC…。
  document.addEventListener('visibilitychange', () => { if (!document.hidden) restoreCurrent(); });
  // 【逐行批注】 声明前端变量 navigation，保存本行计算出的数据或对象。
  const navigation = element('details'); navigation.id = 'learning-navigation';
  // 【逐行批注】 声明前端变量 toggle，保存本行计算出的数据或对象。
  const toggle = element('summary', 'AI 状态'); toggle.setAttribute('aria-label', '打开 AI 状态与控制');
  // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
  const body = element('div', '', 'gy-navigation-body'), panel = element('section'); panel.id = 'agent-control-panel';
  // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
  const label = element('h2', 'AI 教学状态'), state = element('p', '', 'ai-control-notice'); state.setAttribute('role', 'status');
  // 【逐行批注】 声明前端变量 operation，保存本行计算出的数据或对象。
  const operation = element('div', '', 'ai-current-operation'); operation.setAttribute('aria-live', 'polite'); operation.setAttribute('aria-atomic', 'true');
  // 【逐行批注】 声明前端变量 operationLabel，保存本行计算出的数据或对象。
  const operationLabel = element('p', '当前操作', 'ai-operation-eyebrow'), operationTitle = element('h3', '正在读取任务状态', 'ai-operation-title');
  // 【逐行批注】 声明前端变量 target，保存本行计算出的数据或对象。
  const target = element('p', '', 'ai-operation-target'), operationDetail = element('p', '', 'ai-operation-detail');
  // 【逐行批注】 执行当前前端表达式：operation.append(operationLabel, operationTitle, target, operationDetail);。
  operation.append(operationLabel, operationTitle, target, operationDetail);
  // 【逐行批注】 声明前端变量 traceHost，保存本行计算出的数据或对象。
  const traceHost = element('div');
  // 【逐行批注】 声明前端变量 trace，保存本行计算出的数据或对象。
  const trace = window.GangyiTrace?.create(traceHost, {variant:'panel', eyebrow:'可核验执行过程', collapseWhenDone:true,
    // 【逐行批注】 计算右侧结果并更新 onRetry:async()，改变页面或请求状态。
    onRetry:async()=>{if(current?.id)await controlFromTrace('retry',current.id);},
    // 【逐行批注】 计算右侧结果并更新 onResume:async()，改变页面或请求状态。
    onResume:async()=>{if(current?.id)await controlFromTrace('resume',current.id);},
    // 【逐行批注】 计算右侧结果并更新 onPause:async()，改变页面或请求状态。
    onPause:async()=>{if(current?.id)await controlFromTrace('pause',current.id);},
    // 【逐行批注】 计算右侧结果并更新 onCancel:async()，改变页面或请求状态。
    onCancel:async()=>{if(current?.id)await controlFromTrace('cancel',current.id);},
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 声明前端变量 actions，保存本行计算出的数据或对象。
  const actions = element('div', '', 'ai-control-actions'), changes = element('div', '', 'ai-control-changes');
  // 【逐行批注】 定义函数 disclosure，封装可复用的前端行为。
  function disclosure(title, className) { const node = element('details', '', className), summary = element('summary', title); node.append(summary); return node; }
  // 【逐行批注】 声明前端变量 history，保存本行计算出的数据或对象。
  const history = disclosure('最近步骤', 'ai-control-history'), historyList = element('ol', '', 'ai-step-list'); history.append(historyList);
  // 【逐行批注】 声明前端变量 taskDetails，保存本行计算出的数据或对象。
  const taskDetails = disclosure('任务详情', 'ai-task-details'), facts = element('dl', '', 'ai-task-facts'); taskDetails.append(facts);
  // 【逐行批注】 声明前端变量 adjustments，保存本行计算出的数据或对象。
  const adjustments = disclosure('最近调整', 'ai-control-adjustments'); adjustments.append(changes); adjustments.hidden = true;
  // 【逐行批注】 声明前端变量 metrics，保存本行计算出的数据或对象。
  const metrics = element('p', '', 'ai-call-metrics'), globalActions = element('div', '', 'ai-control-actions');
  // 【逐行批注】 声明前端变量 callList，保存本行计算出的数据或对象。
  const callList = element('div', '', 'ai-live-calls'); callList.id = 'ai-live-calls';
  // 【逐行批注】 声明前端变量 callHistory，保存本行计算出的数据或对象。
  const callHistory = disclosure('调用历史', 'ai-call-history'), pastList = element('div'); callHistory.append(pastList);
  // 【逐行批注】 声明前端变量 currentDetails，保存本行计算出的数据或对象。
  const currentDetails = disclosure('当前课时的执行详情', 'ai-context-details');
  // 【逐行批注】 声明前端变量 callNotice，保存本行计算出的数据或对象。
  const callNotice = element('p', '', 'ai-control-notice'); callNotice.hidden = true; callNotice.setAttribute('role', 'status');
  // 【逐行批注】 执行当前前端表达式：currentDetails.append(operation, traceHost, state, actions, history, taskDetails, adjustm…。
  currentDetails.append(operation, traceHost, state, actions, history, taskDetails, adjustments);
  // 【逐行批注】 执行当前前端表达式：panel.append(label, metrics, globalActions, callNotice, callList, callHistory, currentDet…。
  panel.append(label, metrics, globalActions, callNotice, callList, callHistory, currentDetails);
  // 【逐行批注】 执行当前前端表达式：body.append(panel); navigation.append(toggle, body); document.body.append(navigation);。
  body.append(panel); navigation.append(toggle, body); document.body.append(navigation);
  // 【逐行批注】 声明前端变量 globalPaused，保存本行计算出的数据或对象。
  let globalPaused = false, globalBusy = false, globalKey = '', activityVersion = 0, historyLimit = 20, liveKey = '', pastKey = '', latestView;
  // 只在新任务出现或首次进入活跃态时自动展开，之后用户手动折叠不会被每秒轮询覆盖。
  // 【逐行批注】 声明前端变量 autoExpandedTask，保存本行计算出的数据或对象。
  let autoExpandedTask = '', wasActive = false;
  // 控制请求（POST）失败时，用本地失败任务在轨迹里就地说明原因，避免轨迹与页面状态矛盾。
  // 【逐行批注】 声明前端变量 localFailure，保存本行计算出的数据或对象。
  let localFailure = null;
  // 【逐行批注】 定义函数 noteControlFailure，封装可复用的前端行为。
  function noteControlFailure(message) {
    // 【逐行批注】 声明前端变量 base，保存本行计算出的数据或对象。
    const base = current || {};
    // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
    const text = message || '操作没有生效，请稍后重试。';
    // 【逐行批注】 计算右侧结果并更新 localFailure，改变页面或请求状态。
    localFailure = { at: Date.now(), task: {
      // 【逐行批注】 调用 Number 执行页面交互或数据处理：id: base.id || '', status: 'failed', purpose: base.purpose || 'AI 控制操…。
      id: base.id || '', status: 'failed', purpose: base.purpose || 'AI 控制操作', calls: Number(base.calls || 0),
      // 【逐行批注】 调用 Date 执行页面交互或数据处理：createdAt: base.createdAt || new Date().toISOString(), updatedAt: new…。
      createdAt: base.createdAt || new Date().toISOString(), updatedAt: new Date().toISOString(),
      // 【逐行批注】 调用 Date 执行页面交互或数据处理：activity: { title: '操作未生效', detail: '操作请求没有送达，AI 状态没有改变；可以重新操作或稍后重试。'…。
      activity: { title: '操作未生效', detail: '操作请求没有送达，AI 状态没有改变；可以重新操作或稍后重试。', state: 'failed', at: new Date().toISOString() },
      // 【逐行批注】 执行当前前端表达式：failure: { message: '操作未生效：' + text },。
      failure: { message: '操作未生效：' + text },
    // 【逐行批注】 执行当前前端表达式：} };。
    } };
    // 【逐行批注】 计算右侧结果并更新 setText(state, '操作未生效：' + text); state.hidden，改变页面或请求状态。
    setText(state, '操作未生效：' + text); state.hidden = false;
    // 【逐行批注】 计算右侧结果并更新 currentDetails.open，改变页面或请求状态。
    currentDetails.open = true;
    // 【逐行批注】 执行当前前端表达式：trace?.update(localFailure.task);。
    trace?.update(localFailure.task);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 rows，保存本行计算出的数据或对象。
  const rows = new Map();
  // 【逐行批注】 声明前端变量 statusText，保存本行计算出的数据或对象。
  const statusText = { pending: '排队等待', queued: '等待请求', running: '本地处理', requesting: '正在请求 AI', receiving: '正在接收 AI 回复',
    // 【逐行批注】 执行当前前端表达式：paused: '已暂停', ready: '任务已完成', completed: '请求已完成', waiting_student: '等待你的回应', failed: '请求…。
    paused: '已暂停', ready: '任务已完成', completed: '请求已完成', waiting_student: '等待你的回应', failed: '请求未完成',
    // 【逐行批注】 执行当前前端表达式：cancelled: '已停止', superseded: '已被新任务替代', interrupted: '软件退出，请求已中断' };。
    cancelled: '已停止', superseded: '已被新任务替代', interrupted: '软件退出，请求已中断' };
  // 【逐行批注】 声明前端变量 ongoing，保存本行计算出的数据或对象。
  const ongoing = task => ['pending', 'queued', 'running', 'requesting', 'receiving', 'paused'].includes(task.status);
  // 【逐行批注】 定义函数 callRow，封装可复用的前端行为。
  function callRow(task) {
    // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
    let value = rows.get(task.id);
    // 【逐行批注】 判断条件 (!value) {，成立时执行对应逻辑。
    if (!value) {
      // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
      const row = element('article', '', 'ai-call-row'), source = element('p', '', 'ai-call-source'), purpose = element('h3', '', 'ai-call-purpose');
      // 【逐行批注】 声明前端变量 status，保存本行计算出的数据或对象。
      const status = element('p', '', 'ai-call-status'), action = element('p', '', 'ai-call-operation'), target = element('p', '', 'ai-call-target');
      // 【逐行批注】 声明前端变量 count，保存本行计算出的数据或对象。
      const count = element('p', '', 'ai-call-count'), detail = disclosure('详情', 'ai-call-detail'), facts = element('p'), controls = element('div', '', 'ai-control-actions');
      // 【逐行批注】 执行当前前端表达式：detail.append(facts, controls); row.append(source, purpose, status, action, target, count…。
      detail.append(facts, controls); row.append(source, purpose, status, action, target, count, detail);
      // 【逐行批注】 计算右侧结果并更新 value，改变页面或请求状态。
      value = { row, source, purpose, status, action, target, count, facts, controls, key: '' }; rows.set(task.id, value);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 value.row.dataset.status，改变页面或请求状态。
    value.row.dataset.status = task.status;
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(value.source, task.source || 'AI 服务'); setText(value.purpose,…。
    setText(value.source, task.source || 'AI 服务'); setText(value.purpose, task.purpose || '处理学习请求');
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(value.status, statusText[task.status] || '已保存状态');。
    setText(value.status, statusText[task.status] || '已保存状态');
    // 【逐行批注】 声明前端变量 retryLeft，保存本行计算出的数据或对象。
    const retryLeft = task.retry?.delayMs ? Math.max(0, Math.ceil((Number(task.retry.delayMs) -
      // 【逐行批注】 调用 Date 执行页面交互或数据处理：Math.max(0, Date.now() - new Date(task.retry.scheduledAt || 0).getTim…。
      Math.max(0, Date.now() - new Date(task.retry.scheduledAt || 0).getTime())) / 1000)) : 0;
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(value.action, task.retry?.delayMs ? '将在 ' + retryLeft + ' 秒后自…。
    setText(value.action, task.retry?.delayMs ? '将在 ' + retryLeft + ' 秒后自动恢复（' +
      // 【逐行批注】 调用 Number 执行页面交互或数据处理：Math.max(1, Number(task.retry.attempt || 1) - 1) + '/2）；已保存步骤不会重复。' :。
      Math.max(1, Number(task.retry.attempt || 1) - 1) + '/2）；已保存步骤不会重复。' :
      // 【逐行批注】 执行当前前端表达式：task.failure?.message || task.activity?.title || ({ requesting: '等待模型开始返回内容。', receiving:…。
      task.failure?.message || task.activity?.title || ({ requesting: '等待模型开始返回内容。', receiving: '正在接收内容，完整返回后再校验保存。',
      // 【逐行批注】 执行当前前端表达式：pending: '任务已经提交，尚未发起模型请求。', queued: '请求已准备，等待发送。', paused: '输入和有效结果保留，恢复后继续未完成任务。' })[ta…。
      pending: '任务已经提交，尚未发起模型请求。', queued: '请求已准备，等待发送。', paused: '输入和有效结果保留，恢复后继续未完成任务。' })[task.status] || '');
    // 【逐行批注】 计算右侧结果并更新 value.action.hidden，改变页面或请求状态。
    value.action.hidden = !value.action.textContent;
    // 【逐行批注】 计算右侧结果并更新 setText(value.target, [task.target?.courseTi…，改变页面或请求状态。
    setText(value.target, [task.target?.courseTitle, task.target?.lessonTitle].filter(Boolean).join(' · ')); value.target.hidden = !value.target.textContent;
    // 【逐行批注】 调用 setText、Number 执行页面交互或数据处理：setText(value.count, task.tracked || !task.legacyCalls ? '实际请求 ' + Nu…。
    setText(value.count, task.tracked || !task.legacyCalls ? '实际请求 ' + Number(task.calls || 0) + ' 次' : '历史模型轮次 ' + Number(task.legacyCalls) + ' 次（旧版未记录实际请求）');
    // 【逐行批注】 判断条件 (task.tracked && task.legacyCalls) setText(value.count, value.co…，成立时执行对应逻辑。
    if (task.tracked && task.legacyCalls) setText(value.count, value.count.textContent + ' · 另有旧版模型轮次 ' + Number(task.legacyCalls) + ' 次');
    // 【逐行批注】 调用 setText、Date 执行页面交互或数据处理：setText(value.facts, [task.model && '模型：' + task.model, task.updatedA…。
    setText(value.facts, [task.model && '模型：' + task.model, task.updatedAt && '最近更新：' + new Date(task.updatedAt).toLocaleString('zh-CN'), task.activity?.detail].filter(Boolean).join('\n'));
    // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
    const key = JSON.stringify([task.taskId, task.status, globalPaused]);
    // 【逐行批注】 判断条件 (value.key !== key) {，成立时执行对应逻辑。
    if (value.key !== key) {
      // 【逐行批注】 计算右侧结果并更新 value.key，改变页面或请求状态。
      value.key = key; value.controls.replaceChildren();
      // 【逐行批注】 定义函数 control，封装可复用的前端行为。
      function control(text, command) {
        // 【逐行批注】 声明前端变量 button，保存本行计算出的数据或对象。
        const button = element('button', text); button.type = 'button';
        // 【逐行批注】 计算右侧结果并更新 button.onclick，改变页面或请求状态。
        button.onclick = async () => { button.disabled = true; setText(button, text + '…'); try {
          // 【逐行批注】 声明前端变量 updated，保存本行计算出的数据或对象。
          const updated = await api.control({ command, taskId: task.taskId });
          // 【逐行批注】 计算右侧结果并更新 localFailure，改变页面或请求状态。
          localFailure = null;
          // 【逐行批注】 调用 CustomEvent、load 执行页面交互或数据处理：document.dispatchEvent(new CustomEvent('gangyi:agent-control', { deta…。
          document.dispatchEvent(new CustomEvent('gangyi:agent-control', { detail: { command, task: updated } })); await load();
        // 【逐行批注】 计算右侧结果并更新 } catch (error) { noteControlFailure(error.m…，改变页面或请求状态。
        } catch (error) { noteControlFailure(error.message); } finally { button.disabled = false; setText(button, text); } };
        // 【逐行批注】 执行当前前端表达式：value.controls.append(button);。
        value.controls.append(button);
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (task.taskId) {，成立时执行对应逻辑。
      if (task.taskId) {
        // 【逐行批注】 判断条件 (task.status === 'paused' && !globalPaused) control('恢复此任务', 're…，成立时执行对应逻辑。
        if (task.status === 'paused' && !globalPaused) control('恢复此任务', 'resume');
        // 【逐行批注】 判断条件 (['failed', 'cancelled', 'superseded'].includes(task.status) && …，成立时执行对应逻辑。
        if (['failed', 'cancelled', 'superseded'].includes(task.status) && !globalPaused) control('重试此任务', 'retry');
        // 【逐行批注】 判断条件 (ongoing(task)) control('停止此任务', 'cancel');，成立时执行对应逻辑。
        if (ongoing(task)) control('停止此任务', 'cancel');
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return value.row;。
    return value.row;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderCalls，封装可复用的前端行为。
  function renderCalls(view) {
    // 【逐行批注】 计算右侧结果并更新 latestView，改变页面或请求状态。
    latestView = view; callNotice.hidden = true;
    // 【逐行批注】 计算右侧结果并更新 globalPaused，改变页面或请求状态。
    globalPaused = Boolean(view.paused);
    // 【逐行批注】 声明前端变量 tasks，保存本行计算出的数据或对象。
    const tasks = [...(view.tasks || [])].sort((a, b) => String(b.updatedAt || '').localeCompare(String(a.updatedAt || '')));
    // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
    const active = tasks.filter(ongoing), past = tasks.filter(task => !ongoing(task));
    // 【逐行批注】 声明前端变量 requesting，保存本行计算出的数据或对象。
    const requesting = active.filter(task => ['requesting', 'receiving'].includes(task.status)).length;
    // 【逐行批注】 声明前端变量 needsAttention，保存本行计算出的数据或对象。
    const needsAttention = tasks.filter(task => task.status === 'failed').slice(0, 9).length;
    // 【逐行批注】 计算右侧结果并更新 navigation.dataset.status，改变页面或请求状态。
    navigation.dataset.status = globalPaused ? 'paused' : requesting ? 'running' : active.length ? 'pending' : needsAttention ? 'failed' : 'idle';
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(toggle, globalPaused ? '全部 AI 已暂停' : requesting ? 'AI 正在调用 · …。
    setText(toggle, globalPaused ? '全部 AI 已暂停' : requesting ? 'AI 正在调用 · ' + requesting + ' 项' :
      // 【逐行批注】 执行当前前端表达式：active.length ? 'AI 待处理 · ' + active.length + ' 项' : needsAttention ? 'AI 需要处理 · ' + need…。
      active.length ? 'AI 待处理 · ' + active.length + ' 项' : needsAttention ? 'AI 需要处理 · ' + needsAttention + ' 项' : 'AI 状态');
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(label, '全软件 AI 调用');。
    setText(label, '全软件 AI 调用');
    // 【逐行批注】 调用 setText、Number 执行页面交互或数据处理：setText(metrics, '正在请求 ' + requesting + ' 项 · 今日实际请求 ' + Number(view.…。
    setText(metrics, '正在请求 ' + requesting + ' 项 · 今日实际请求 ' + Number(view.todayCalls || 0) + ' 次 · 实时更新');
    // 【逐行批注】 判断条件 (globalKey !== String(globalPaused)) {，成立时执行对应逻辑。
    if (globalKey !== String(globalPaused)) {
      // 【逐行批注】 计算右侧结果并更新 globalKey，改变页面或请求状态。
      globalKey = String(globalPaused); globalActions.replaceChildren();
      // 【逐行批注】 声明前端变量 control，保存本行计算出的数据或对象。
      const control = element('button', globalPaused ? '恢复全部 AI' : '暂停全部 AI'); control.type = 'button';
      // 【逐行批注】 计算右侧结果并更新 control.onclick，改变页面或请求状态。
      control.onclick = async () => {
        // 【逐行批注】 判断条件 (globalBusy) return; globalBusy = true; control.disabled = true;…，成立时执行对应逻辑。
        if (globalBusy) return; globalBusy = true; control.disabled = true; ++activityVersion;
        // 【逐行批注】 声明前端变量 command，保存本行计算出的数据或对象。
        const command = globalPaused ? 'resume' : 'pause';
        // 【逐行批注】 执行当前前端表达式：try {。
        try {
          // 【逐行批注】 声明前端变量 result，保存本行计算出的数据或对象。
          const result = await api.request('/api/ai-activity/control', { command }); renderCalls(result);
          // 【逐行批注】 调用 CustomEvent 执行页面交互或数据处理：document.dispatchEvent(new CustomEvent('gangyi:agent-control', { deta…。
          document.dispatchEvent(new CustomEvent('gangyi:agent-control', { detail: { command, all: true } }));
          // 【逐行批注】 调用 load 执行页面交互或数据处理：await load();。
          await load();
        // 【逐行批注】 计算右侧结果并更新 } catch (error) { callNotice.textContent，改变页面或请求状态。
        } catch (error) { callNotice.textContent = error.message; callNotice.hidden = false; }
        // 【逐行批注】 计算右侧结果并更新 finally { globalBusy，改变页面或请求状态。
        finally { globalBusy = false; for (const button of globalActions.querySelectorAll('button')) button.disabled = false; }
      // 【逐行批注】 执行当前前端表达式：};。
      };
      // 【逐行批注】 计算右侧结果并更新 control.disabled，改变页面或请求状态。
      control.disabled = globalBusy; globalActions.append(control);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 复用每行节点，实时更新不打断折叠开合和按钮焦点。
    // 【逐行批注】 计算右侧结果并更新 active.sort((a,b)，改变页面或请求状态。
    active.sort((a,b) => String(b.createdAt || b.id).localeCompare(String(a.createdAt || a.id)));
    // 【逐行批注】 声明前端变量 activeNodes，保存本行计算出的数据或对象。
    const activeNodes = active.map(callRow), pastNodes = past.slice(0, historyLimit).map(callRow);
    // 【逐行批注】 声明前端变量 nextLiveKey，保存本行计算出的数据或对象。
    const nextLiveKey = JSON.stringify([active.map(task => task.id), globalPaused]), nextPastKey = JSON.stringify([past.map(task => task.id), historyLimit]);
    // 【逐行批注】 判断条件 (liveKey !== nextLiveKey) {，成立时执行对应逻辑。
    if (liveKey !== nextLiveKey) {
      // 【逐行批注】 计算右侧结果并更新 liveKey，改变页面或请求状态。
      liveKey = nextLiveKey; callList.replaceChildren(...activeNodes);
      // 【逐行批注】 判断条件 (!active.length) callList.append(element('p', globalPaused ? '全部…，成立时执行对应逻辑。
      if (!active.length) callList.append(element('p', globalPaused ? '全部 AI 已暂停，新的请求也会等待恢复。' : '目前没有正在调用 AI 的任务。', 'ai-call-empty'));
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 判断条件 (pastKey !== nextPastKey) {，成立时执行对应逻辑。
    if (pastKey !== nextPastKey) {
      // 【逐行批注】 计算右侧结果并更新 pastKey，改变页面或请求状态。
      pastKey = nextPastKey; pastList.replaceChildren(...pastNodes);
      // 【逐行批注】 判断条件 (past.length > historyLimit) {，成立时执行对应逻辑。
      if (past.length > historyLimit) {
        // 【逐行批注】 声明前端变量 more，保存本行计算出的数据或对象。
        const more = element('button', '查看更多历史'); more.type = 'button'; more.onclick = () => { historyLimit += 20; renderCalls(latestView); }; pastList.append(more);
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(callHistory.firstChild, '调用历史 · ' + past.length + ' 项');。
    setText(callHistory.firstChild, '调用历史 · ' + past.length + ' 项');
    // 【逐行批注】 声明前端变量 present，保存本行计算出的数据或对象。
    const present = new Set(tasks.map(task => task.id)); for (const id of rows.keys()) if (!present.has(id)) rows.delete(id);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 actionsKey，保存本行计算出的数据或对象。
  let actionsKey = '', historyKey = '', changesKey = '', controlBusy = false;
  // 【逐行批注】 声明前端变量 clock，保存本行计算出的数据或对象。
  const clock = value => { const date = new Date(value); return Number.isNaN(date.getTime()) ? '' : date.toLocaleTimeString('zh-CN', { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false }); };
  // 【逐行批注】 定义函数 renderOperation，封装可复用的前端行为。
  function renderOperation(task, paused) {
    // 【逐行批注】 声明前端变量 activity，保存本行计算出的数据或对象。
    const activity = task.activity, finished = ['ready', 'waiting_student'].includes(task.status);
    // 【逐行批注】 声明前端变量 names，保存本行计算出的数据或对象。
    const names = [task.target?.courseTitle, task.target?.lessonTitle].filter(Boolean);
    // 【逐行批注】 计算右侧结果并更新 setText(target, names.join(' · ')); target.h…，改变页面或请求状态。
    setText(target, names.join(' · ')); target.hidden = !names.length;
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(operationLabel, paused ? '暂停位置' : task.status === 'failed' ? …。
    setText(operationLabel, paused ? '暂停位置' : task.status === 'failed' ? '未完成的操作' : finished ? '本次结果' : '当前操作');
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(operationTitle, activity?.title || ({ idle: '暂无正在执行的任务', pend…。
    setText(operationTitle, activity?.title || ({ idle: '暂无正在执行的任务', pending: '等待开始：' + (task.purpose || '本次学习请求'),
      // 【逐行批注】 执行当前前端表达式：running: '正在处理本次学习请求', paused: '本次处理已暂停', ready: '本次处理已完成', waiting_student: '等待你的回答或补充信息…。
      running: '正在处理本次学习请求', paused: '本次处理已暂停', ready: '本次处理已完成', waiting_student: '等待你的回答或补充信息',
      // 【逐行批注】 执行当前前端表达式：cancelled: '本次任务已停止', superseded: '本次任务已由新学习记录替代' })[task.status] || '任务状态已保存');。
      cancelled: '本次任务已停止', superseded: '本次任务已由新学习记录替代' })[task.status] || '任务状态已保存');
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(operationDetail, activity?.detail || (task.id ? '此历史任务未记录具体执行…。
    setText(operationDetail, activity?.detail || (task.id ? '此历史任务未记录具体执行步骤，已有内容和记录仍然保留。' : '提交学习目标或进入课堂后，会在这里显示真实执行操作。'));
    // 【逐行批注】 判断条件 (paused) {，成立时执行对应逻辑。
    if (paused) {
      // 【逐行批注】 判断条件 (finished) { setText(operationTitle, '后续 AI 处理已暂停'); setText(ope…，成立时执行对应逻辑。
      if (finished) { setText(operationTitle, '后续 AI 处理已暂停'); setText(operationDetail, '本次任务已经完成，已有结果保留。恢复后会继续响应新的学习操作。'); }
      // 【逐行批注】 执行前一条件不成立时的分支。
      else { setText(operationTitle, activity ? '已暂停：' + activity.title : '本次处理已暂停'); setText(operationDetail, task.pauseReason === 'shutdown' ? '软件关闭时暂停在这里，输入和有效结果已保存。' : '你已暂停本次处理，输入和有效结果已保存。恢复后继续未完成的步骤。'); }
    // 【逐行批注】 执行当前前端表达式：} else if (task.status === 'pending') {。
    } else if (task.status === 'pending') {
      // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(operationTitle, '等待开始：' + (task.purpose || '本次学习请求')); setTex…。
      setText(operationTitle, '等待开始：' + (task.purpose || '本次学习请求')); setText(operationDetail, '任务已提交；AI 尚未开始本次处理。');
    // 【逐行批注】 执行当前前端表达式：} else if (task.status === 'failed') {。
    } else if (task.status === 'failed') {
      // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(operationTitle, task.failure?.operation || activity?.title ||…。
      setText(operationTitle, task.failure?.operation || activity?.title || '本次处理未完成'); setText(operationDetail, api.failureMessage(task));
    // 【逐行批注】 执行当前前端表达式：} else if (task.status === 'cancelled' || task.status === 'superseded') {。
    } else if (task.status === 'cancelled' || task.status === 'superseded') {
      // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(operationTitle, task.status === 'cancelled' ? '本次任务已停止' : '旧任…。
      setText(operationTitle, task.status === 'cancelled' ? '本次任务已停止' : '旧任务已停止，避免覆盖新记录'); setText(operationDetail, '此前已保存的有效内容和学习记录保留。');
    // 【逐行批注】 执行当前前端表达式：} else if (task.status === 'ready') {。
    } else if (task.status === 'ready') {
      // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(operationTitle, '已完成：' + (task.purpose || '本次处理'));。
      setText(operationTitle, '已完成：' + (task.purpose || '本次处理'));
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(state, !paused && ['pending', 'running'].includes(task.status…。
    setText(state, !paused && ['pending', 'running'].includes(task.status) && task.failure ? '上一步未完成，正在重新处理：' + api.failureMessage(task) : '');
    // 【逐行批注】 计算右侧结果并更新 state.hidden，改变页面或请求状态。
    state.hidden = !state.textContent;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderHistory，封装可复用的前端行为。
  function renderHistory(task) {
    // 【逐行批注】 声明前端变量 completed，保存本行计算出的数据或对象。
    const completed = (task.events || []).filter(event => event.type === 'action');
    // 【逐行批注】 声明前端变量 recent，保存本行计算出的数据或对象。
    const recent = completed.slice(-6).reverse(), key = JSON.stringify([task.id, recent]);
    // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(history.firstChild, '最近步骤' + (completed.length ? ' · 已完成 ' + …。
    setText(history.firstChild, '最近步骤' + (completed.length ? ' · 已完成 ' + completed.length + ' 项' : ''));
    // 【逐行批注】 判断条件 (key === historyKey) return; historyKey = key; historyList.repla…，成立时执行对应逻辑。
    if (key === historyKey) return; historyKey = key; historyList.replaceChildren();
    // 【逐行批注】 判断条件 (!recent.length) { historyList.append(element('li', '尚无已完成的操作。',…，成立时执行对应逻辑。
    if (!recent.length) { historyList.append(element('li', '尚无已完成的操作。', 'ai-step-empty')); return; }
    // 【逐行批注】 按循环规则遍历或重复处理：for (const event of recent) {。
    for (const event of recent) {
      // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
      const row = element('li'), title = element('strong', event.activity?.title || '教学操作已完成');
      // 【逐行批注】 声明前端变量 time，保存本行计算出的数据或对象。
      const time = element('time', clock(event.at)); if (event.at) { time.dateTime = event.at; time.title = new Date(event.at).toLocaleString('zh-CN'); }
      // 【逐行批注】 调用 element 执行页面交互或数据处理：row.append(title, time, element('p', event.activity?.detail || '历史任务仅…。
      row.append(title, time, element('p', event.activity?.detail || '历史任务仅保存了执行记录，未记录具体操作。')); historyList.append(row);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 判断条件 (completed.length > recent.length) historyList.append(element('l…，成立时执行对应逻辑。
    if (completed.length > recent.length) historyList.append(element('li', '显示最近 ' + recent.length + ' 项操作。', 'ai-step-empty'));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 factsKey，保存本行计算出的数据或对象。
  let factsKey = '';
  // 【逐行批注】 定义函数 renderFacts，封装可复用的前端行为。
  function renderFacts(task) {
    // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
    const key = JSON.stringify([task.id, task.purpose, task.calls, task.model, task.updatedAt,
      // 【逐行批注】 执行当前前端表达式：task.failure?.operation, task.failure?.attempt, task.activity?.title]);。
      task.failure?.operation, task.failure?.attempt, task.activity?.title]);
    // 【逐行批注】 判断条件 (key !== factsKey) {，成立时执行对应逻辑。
    if (key !== factsKey) {
      // 【逐行批注】 计算右侧结果并更新 factsKey，改变页面或请求状态。
      factsKey = key;
      // 【逐行批注】 执行当前前端表达式：facts.replaceChildren();。
      facts.replaceChildren();
      // 【逐行批注】 定义函数 fact，封装可复用的前端行为。
      function fact(title, value) { if (value === undefined || value === null || value === '') return; facts.append(element('dt', title), element('dd', String(value))); }
      // 【逐行批注】 调用 fact、Number 执行页面交互或数据处理：fact('任务', task.purpose); fact('主控处理轮次', Number(task.calls || 0) + ' …。
      fact('任务', task.purpose); fact('主控处理轮次', Number(task.calls || 0) + ' 次'); fact('使用模型', task.model);
      // 【逐行批注】 调用 fact、Date 执行页面交互或数据处理：fact('最近记录', task.updatedAt ? new Date(task.updatedAt).toLocaleString…。
      fact('最近记录', task.updatedAt ? new Date(task.updatedAt).toLocaleString('zh-CN') : '');
      // 【逐行批注】 判断条件 (task.failure) { fact('未完成操作', task.failure.operation || task.ac…，成立时执行对应逻辑。
      if (task.failure) { fact('未完成操作', task.failure.operation || task.activity?.title); fact('连续失败', task.failure.attempt ? task.failure.attempt + ' 次' : ''); }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 taskDetails.hidden，改变页面或请求状态。
    taskDetails.hidden = !task.id; history.hidden = !task.id;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 button，封装可复用的前端行为。
  function button(title, command, taskId) {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = element('button', title); node.type = 'button'; node.onclick = async () => {
      // 【逐行批注】 判断条件 (controlBusy) return; controlBusy = true;，成立时执行对应逻辑。
      if (controlBusy) return; controlBusy = true;
      // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(node, title + '…');。
      setText(node, title + '…');
      // 【逐行批注】 按循环规则遍历或重复处理：for (const control of actions.querySelectorAll('button')) control.disabled…。
      for (const control of actions.querySelectorAll('button')) control.disabled = true;
      // 【逐行批注】 计算右侧结果并更新 try { const task，改变页面或请求状态。
      try { const task = await api.control({ command, taskId }); localFailure = null; document.dispatchEvent(new CustomEvent('gangyi:agent-control', { detail: { command, task } })); await load(); }
      // 【逐行批注】 调用 noteControlFailure 执行页面交互或数据处理：catch (error) { noteControlFailure(error.message); }。
      catch (error) { noteControlFailure(error.message); }
      // 【逐行批注】 计算右侧结果并更新 finally { controlBusy，改变页面或请求状态。
      finally { controlBusy = false; setText(node, title); for (const control of actions.querySelectorAll('button')) control.disabled = false; }
    // 【逐行批注】 执行当前前端表达式：}; actions.append(node);。
    }; actions.append(node);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 controlFromTrace，封装可复用的前端行为。
  async function controlFromTrace(command, taskId) {
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
      const task = await api.control({command, taskId});
      // 【逐行批注】 计算右侧结果并更新 localFailure，改变页面或请求状态。
      localFailure = null;
      // 【逐行批注】 调用 CustomEvent 执行页面交互或数据处理：document.dispatchEvent(new CustomEvent('gangyi:agent-control', {detai…。
      document.dispatchEvent(new CustomEvent('gangyi:agent-control', {detail:{command, task}}));
      // 【逐行批注】 调用 load 执行页面交互或数据处理：await load();。
      await load();
    // 【逐行批注】 调用 noteControlFailure 执行页面交互或数据处理：} catch (error) { noteControlFailure(error.message); }。
    } catch (error) { noteControlFailure(error.message); }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 load，封装可复用的前端行为。
  async function load() {
    // 【逐行批注】 声明前端变量 callVersion，保存本行计算出的数据或对象。
    const callVersion = ++activityVersion;
    // 【逐行批注】 计算右侧结果并更新 api.request('/api/ai-activity').then(view，改变页面或请求状态。
    api.request('/api/ai-activity').then(view => { if (callVersion === activityVersion) renderCalls(view); })
      // 【逐行批注】 计算右侧结果并更新 .catch(error，改变页面或请求状态。
      .catch(error => { if (callVersion === activityVersion) { callNotice.textContent = '实时调用状态暂时无法更新：' + error.message; callNotice.hidden = false; } });
    // 【逐行批注】 声明前端变量 version，保存本行计算出的数据或对象。
    const version = ++stateVersion;
    // 【逐行批注】 声明前端变量 requested，保存本行计算出的数据或对象。
    const requested = context.teachingTaskId || context.taskId || '';
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
      const task = await api.request('/api/learning-agent' + (requested ? '?taskId=' + encodeURIComponent(requested) : ''));
      // 【逐行批注】 判断条件 (version !== stateVersion || requested !== (context.teachingTask…，成立时执行对应逻辑。
      if (version !== stateVersion || requested !== (context.teachingTaskId || context.taskId || '')) return;
      // 【逐行批注】 计算右侧结果并更新 current，改变页面或请求状态。
      current = task;
      // 控制请求失败后的 8 秒内保留本地失败说明，避免每秒轮询立刻把失败原因覆盖掉。
      // 【逐行批注】 判断条件 (localFailure && Date.now() - localFailure.at < 8000) trace?.upd…，成立时执行对应逻辑。
      if (localFailure && Date.now() - localFailure.at < 8000) trace?.update(localFailure.task);
      // 【逐行批注】 执行前一条件不成立时的分支。
      else { localFailure = null; trace?.update(task); }
      // 【逐行批注】 声明前端变量 activeNow，保存本行计算出的数据或对象。
      const activeNow = ['pending', 'running'].includes(task.status);
      // 【逐行批注】 声明前端变量 attention，保存本行计算出的数据或对象。
      const attention = ['pending', 'running', 'failed', 'paused'].includes(task.status);
      // 【逐行批注】 判断条件 (attention && task.id && (task.id !== autoExpandedTask || (activ…，成立时执行对应逻辑。
      if (attention && task.id && (task.id !== autoExpandedTask || (activeNow && !wasActive))) {
        // 【逐行批注】 计算右侧结果并更新 autoExpandedTask，改变页面或请求状态。
        autoExpandedTask = task.id;
        // 【逐行批注】 计算右侧结果并更新 currentDetails.open，改变页面或请求状态。
        currentDetails.open = true;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 计算右侧结果并更新 wasActive，改变页面或请求状态。
      wasActive = activeNow;
      // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(currentDetails.firstChild, requested ? '本页任务的执行详情' : '最近任务的执行…。
      setText(currentDetails.firstChild, requested ? '本页任务的执行详情' : '最近任务的执行详情');
      // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
      const active = ['pending', 'running'].includes(task.status), paused = task.paused || task.status === 'paused';
      // 【逐行批注】 调用 renderOperation、renderHistory、renderFacts 执行页面交互或数据处理：renderOperation(task, paused); renderHistory(task); renderFacts(task);。
      renderOperation(task, paused); renderHistory(task); renderFacts(task);
      // 【逐行批注】 声明前端变量 nextActions，保存本行计算出的数据或对象。
      const nextActions = JSON.stringify([task.id, paused, task.status, task.lesson?.entered]);
      // 【逐行批注】 判断条件 (actionsKey !== nextActions) {，成立时执行对应逻辑。
      if (actionsKey !== nextActions) {
        // 【逐行批注】 计算右侧结果并更新 actionsKey，改变页面或请求状态。
        actionsKey = nextActions; actions.replaceChildren();
        // 【逐行批注】 判断条件 (task.id) {，成立时执行对应逻辑。
        if (task.id) {
          // 【逐行批注】 判断条件 (paused && !globalPaused) button('恢复此任务', 'resume', task.id);，成立时执行对应逻辑。
          if (paused && !globalPaused) button('恢复此任务', 'resume', task.id);
          // 【逐行批注】 判断条件 (['failed', 'superseded', 'cancelled'].includes(task.status)) bu…，成立时执行对应逻辑。
          if (['failed', 'superseded', 'cancelled'].includes(task.status)) button('重试', 'retry', task.id);
          // 【逐行批注】 判断条件 (active || task.status === 'paused') button('停止任务', 'cancel', ta…，成立时执行对应逻辑。
          if (active || task.status === 'paused') button('停止任务', 'cancel', task.id);
          // 【逐行批注】 判断条件 (task.lesson && !task.lesson.entered && ['ready', 'waiting_stude…，成立时执行对应逻辑。
          if (task.lesson && !task.lesson.entered && ['ready', 'waiting_student'].includes(task.status)) actions.append(anchor('进入已备好的课堂 →', '/agent-prepare.html?taskId=' + encodeURIComponent(task.id)));
          // 【逐行批注】 按循环规则遍历或重复处理：for (const control of actions.querySelectorAll('button')) control.disabled…。
          for (const control of actions.querySelectorAll('button')) control.disabled = controlBusy;
        // 【逐行批注】 结束当前前端代码块。
        }
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 声明前端变量 applied，保存本行计算出的数据或对象。
      const applied = (task.changeHistory || []).filter(item => item.status === 'applied').slice(-5).reverse(), nextChanges = JSON.stringify(applied);
      // 【逐行批注】 计算右侧结果并更新 adjustments.hidden，改变页面或请求状态。
      adjustments.hidden = !applied.length;
      // 【逐行批注】 判断条件 (changesKey === nextChanges) return; changesKey = nextChanges; c…，成立时执行对应逻辑。
      if (changesKey === nextChanges) return; changesKey = nextChanges; changes.replaceChildren();
      // 【逐行批注】 按循环规则遍历或重复处理：for (const change of applied) {。
      for (const change of applied) {
        // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
        const row = element('article'), reason = element('p', change.reason), undo = element('button', '撤回这次调整'); undo.type = 'button';
        // 【逐行批注】 计算右侧结果并更新 undo.onclick，改变页面或请求状态。
        undo.onclick = async () => { undo.disabled = true; try { await api.control({ command: 'undo', changeId: change.id }); await load(); } catch (error) { reason.textContent = error.message; } finally { undo.disabled = false; } };
        // 【逐行批注】 执行当前前端表达式：row.append(reason, undo); changes.append(row);。
        row.append(reason, undo); changes.append(row);
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 执行当前前端表达式：} catch (error) { if (version === stateVersion && requested === (context.teachingTaskId |…。
    } catch (error) { if (version === stateVersion && requested === (context.teachingTaskId || context.taskId || '')) { state.textContent = '无法更新任务状态：' + error.message; state.hidden = false; } }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 为页面事件绑定处理逻辑：navigation.addEventListener('toggle', () => { if (navigation.open) load(); });。
  navigation.addEventListener('toggle', () => { if (navigation.open) load(); });
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('keydown', event => { if (event.key === 'Escape' && naviga…。
  document.addEventListener('keydown', event => { if (event.key === 'Escape' && navigation.open) { navigation.open = false; toggle.focus(); } });
  // 【逐行批注】 为页面事件绑定处理逻辑：load(); const timer = setInterval(() => { if (!document.hidden) load(); }, 1000); wi…。
  load(); const timer = setInterval(() => { if (!document.hidden) load(); }, 1000); window.addEventListener('pagehide', () => clearInterval(timer));
// 【逐行批注】 执行当前前端表达式：})();。
})();

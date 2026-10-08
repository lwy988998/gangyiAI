/* 页面导航与 AI 状态各自集中呈现，共用稳定课程和课时上下文。 */
(() => {
  'use strict';
  const api = window.GangyiAgent;
  if (!api || document.getElementById('learning-navigation')) return;
  const element = (tag, text = '', className = '') => { const node = document.createElement(tag); node.textContent = text; node.className = className; return node; };
  let context = Object.fromEntries(new URLSearchParams(location.search)), current, course, outline = [], outlineCourse = '', courseVersion = 0, stateVersion = 0;
  const localLink = value => typeof value === 'string' && value.startsWith('/') && !value.startsWith('//');
  function anchor(text, href, className = '') { const node = element('a', text, className); node.href = href; return node; }
  function paintCurrent(value) {
    for (const node of document.querySelectorAll('[data-current-course-link]')) {
      if (value && localLink(value.href)) { node.href = value.href; node.removeAttribute('aria-disabled'); node.removeAttribute('tabindex'); node.title = value.title || '当前课程'; }
      else { node.removeAttribute('href'); node.setAttribute('aria-disabled', 'true'); node.tabIndex = -1; node.title = '暂未选择课程，请从我的课程选择'; }
    }
  }
  async function restoreCurrent() {
    try { const data = await api.request('/api/courses'); paintCurrent(data.currentCourse); }
    catch (_) { /* 读取失败时保留已经核实的入口，不跳到课程列表冒充当前课程。 */ }
  }
  const top = element('nav', '', 'course-context-navigation'); top.setAttribute('aria-label', '课程内导航'); top.hidden = true;
  const crumbs = element('div', '', 'course-breadcrumbs'), moves = element('div', '', 'course-context-actions'); top.append(crumbs, moves);
  document.querySelector('main')?.prepend(top);
  function renderContext() {
    top.hidden = !course || !['/plan', '/phase', '/learn', '/practice', '/summary', '/agent-prepare.html'].includes(location.pathname);
    crumbs.replaceChildren(); moves.replaceChildren(); if (top.hidden) return;
    crumbs.append(anchor('我的课程', '/my-courses'), element('span', '／', 'breadcrumb-separator'), anchor(course.title || '当前课程', '/plan?courseId=' + encodeURIComponent(course.id)));
    const position = outline.findIndex(item => context.topicId ? item.id === context.topicId : Number(item.legacyPhaseIndex) === Number(context.phaseIndex) && Number(item.legacyTopicIndex) === Number(context.topicIndex));
    const phase = position >= 0 ? outline[position].displayPhase : Number(context.phaseIndex || 0);
    if (phase) crumbs.append(element('span', '／', 'breadcrumb-separator'), anchor('阶段 ' + phase, '/phase?' + new URLSearchParams({ courseId: course.id, phaseIndex: phase })));
    if (position >= 0 && context.lessonId) {
      const href = item => '/learn?' + new URLSearchParams({ courseId: course.id, topicId: item.id || '', phaseIndex: item.legacyPhaseIndex, topicIndex: item.legacyTopicIndex });
      if (position > 0) moves.append(anchor('← 上一节', href(outline[position - 1])));
      if (position < outline.length - 1) moves.append(anchor('下一节 →', href(outline[position + 1])));
    }
    try { const saved = JSON.parse(api.storage.get('current-lesson') || 'null'); if (saved && localLink(saved.href) && !context.lessonId) moves.append(anchor('回到当前课时 →', saved.href)); }
    catch (_) { /* 损坏缓存不产生跳转。 */ }
  }
  async function setContext(value) {
    const previousTask = context.teachingTaskId || context.taskId || '';
    context = { ...context, ...value };
    if (previousTask !== (context.teachingTaskId || context.taskId || '')) load();
    document.dispatchEvent(new CustomEvent('gangyi:navigation-context', { detail: context }));
    if (!context.courseId) { renderContext(); return; }
    if (outlineCourse === context.courseId && course) { renderContext(); return; }
    const id = context.courseId, version = ++courseVersion;
    try {
      const data = await api.request('/api/courses/' + encodeURIComponent(id)); if (version !== courseVersion || id !== context.courseId) return;
      course = data.course; outlineCourse = id;
      outline = (data.snapshot?.payload?.courseStructure || []).flatMap((stage, index) => (stage.topics || []).map((value, topicIndex) => {
        const topic = typeof value === 'object' && value ? value : { title: String(value || '') };
        return { ...topic, displayPhase: index + 1, legacyPhaseIndex: topic.legacyPhaseIndex || index + 1, legacyTopicIndex: topic.legacyTopicIndex || topicIndex + 1 };
      }));
      paintCurrent({ ...course, href: '/plan?courseId=' + encodeURIComponent(id) }); renderContext();
    } catch (_) { if (version === courseVersion) { course = null; outlineCourse = ''; renderContext(); restoreCurrent(); } }
  }
  window.GangyiNavigation = { setContext, restoreCurrent };
  paintCurrent(null); restoreCurrent(); setContext(context);
  window.addEventListener('focus', restoreCurrent); window.addEventListener('storage', restoreCurrent);
  document.addEventListener('visibilitychange', () => { if (!document.hidden) restoreCurrent(); });
  const navigation = element('details'); navigation.id = 'learning-navigation';
  const toggle = element('summary', 'AI 状态'); toggle.setAttribute('aria-label', '打开 AI 状态与控制');
  const body = element('div', '', 'gy-navigation-body'), panel = element('section'); panel.id = 'agent-control-panel';
  const label = element('h2', 'AI 教学状态'), state = element('p', '', 'ai-control-notice'); state.setAttribute('role', 'status');
  const operation = element('div', '', 'ai-current-operation'); operation.setAttribute('aria-live', 'polite'); operation.setAttribute('aria-atomic', 'true');
  const operationLabel = element('p', '当前操作', 'ai-operation-eyebrow'), operationTitle = element('h3', '正在读取任务状态', 'ai-operation-title');
  const target = element('p', '', 'ai-operation-target'), operationDetail = element('p', '', 'ai-operation-detail');
  operation.append(operationLabel, operationTitle, target, operationDetail);
  const actions = element('div', '', 'ai-control-actions'), changes = element('div', '', 'ai-control-changes');
  function disclosure(title, className) { const node = element('details', '', className), summary = element('summary', title); node.append(summary); return node; }
  const history = disclosure('最近步骤', 'ai-control-history'), historyList = element('ol', '', 'ai-step-list'); history.append(historyList);
  const taskDetails = disclosure('任务详情', 'ai-task-details'), facts = element('dl', '', 'ai-task-facts'); taskDetails.append(facts);
  const adjustments = disclosure('最近调整', 'ai-control-adjustments'); adjustments.append(changes); adjustments.hidden = true;
  panel.append(label, operation, state, actions, history, taskDetails, adjustments); body.append(panel); navigation.append(toggle, body); document.body.append(navigation);
  let actionsKey = '', historyKey = '', changesKey = '', controlBusy = false;
  const clock = value => { const date = new Date(value); return Number.isNaN(date.getTime()) ? '' : date.toLocaleTimeString('zh-CN', { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false }); };
  function renderOperation(task, paused) {
    const activity = task.activity, finished = ['ready', 'waiting_student'].includes(task.status);
    const names = [task.target?.courseTitle, task.target?.lessonTitle].filter(Boolean);
    target.textContent = names.join(' · '); target.hidden = !names.length;
    operationLabel.textContent = paused ? '暂停位置' : task.status === 'failed' ? '未完成的操作' : finished ? '本次结果' : '当前操作';
    operationTitle.textContent = activity?.title || ({ idle: '暂无正在执行的任务', pending: '等待开始：' + (task.purpose || '本次学习请求'),
      running: '正在处理本次学习请求', paused: '本次处理已暂停', ready: '本次处理已完成', waiting_student: '等待你的回答或补充信息',
      cancelled: '本次任务已停止', superseded: '本次任务已由新学习记录替代' })[task.status] || '任务状态已保存';
    operationDetail.textContent = activity?.detail || (task.id ? '此历史任务未记录具体执行步骤，已有内容和记录仍然保留。' : '提交学习目标或进入课堂后，会在这里显示真实执行操作。');
    if (paused) {
      if (finished) { operationTitle.textContent = '后续 AI 处理已暂停'; operationDetail.textContent = '本次任务已经完成，已有结果保留。恢复后会继续响应新的学习操作。'; }
      else { operationTitle.textContent = activity ? '已暂停：' + activity.title : '本次处理已暂停'; operationDetail.textContent = task.pauseReason === 'shutdown' ? '软件关闭时暂停在这里，输入和有效结果已保存。' : '你已暂停本次处理，输入和有效结果已保存。恢复后继续未完成的步骤。'; }
    } else if (task.status === 'pending') {
      operationTitle.textContent = '等待开始：' + (task.purpose || '本次学习请求'); operationDetail.textContent = '任务已提交；AI 尚未开始本次处理。';
    } else if (task.status === 'failed') {
      operationTitle.textContent = task.failure?.operation || activity?.title || '本次处理未完成'; operationDetail.textContent = api.failureMessage(task);
    } else if (task.status === 'cancelled' || task.status === 'superseded') {
      operationTitle.textContent = task.status === 'cancelled' ? '本次任务已停止' : '旧任务已停止，避免覆盖新记录'; operationDetail.textContent = '此前已保存的有效内容和学习记录保留。';
    } else if (task.status === 'ready') {
      operationTitle.textContent = '已完成：' + (task.purpose || '本次处理');
    }
    state.textContent = !paused && ['pending', 'running'].includes(task.status) && task.failure ? '上一步未完成，正在重新处理：' + api.failureMessage(task) : '';
    state.hidden = !state.textContent;
  }
  function renderHistory(task) {
    const completed = (task.events || []).filter(event => event.type === 'action');
    const recent = completed.slice(-6).reverse(), key = JSON.stringify([task.id, recent]);
    history.firstChild.textContent = '最近步骤' + (completed.length ? ' · 已完成 ' + completed.length + ' 项' : '');
    if (key === historyKey) return; historyKey = key; historyList.replaceChildren();
    if (!recent.length) { historyList.append(element('li', '尚无已完成的操作。', 'ai-step-empty')); return; }
    for (const event of recent) {
      const row = element('li'), title = element('strong', event.activity?.title || '教学操作已完成');
      const time = element('time', clock(event.at)); if (event.at) { time.dateTime = event.at; time.title = new Date(event.at).toLocaleString('zh-CN'); }
      row.append(title, time, element('p', event.activity?.detail || '历史任务仅保存了执行记录，未记录具体操作。')); historyList.append(row);
    }
    if (completed.length > recent.length) historyList.append(element('li', '显示最近 ' + recent.length + ' 项操作。', 'ai-step-empty'));
  }
  function renderFacts(task) {
    facts.replaceChildren();
    function fact(title, value) { if (value === undefined || value === null || value === '') return; facts.append(element('dt', title), element('dd', String(value))); }
    fact('任务', task.purpose); fact('AI 请求', Number(task.calls || 0) + ' 次'); fact('使用模型', task.model);
    fact('最近记录', task.updatedAt ? new Date(task.updatedAt).toLocaleString('zh-CN') : '');
    if (task.failure) { fact('未完成操作', task.failure.operation || task.activity?.title); fact('连续失败', task.failure.attempt ? task.failure.attempt + ' 次' : ''); }
    taskDetails.hidden = !task.id; history.hidden = !task.id;
  }
  function button(title, command, taskId) {
    const node = element('button', title); node.type = 'button'; node.onclick = async () => {
      if (controlBusy) return; controlBusy = true; for (const control of actions.querySelectorAll('button')) control.disabled = true;
      try { const task = await api.control({ command, taskId }); document.dispatchEvent(new CustomEvent('gangyi:agent-control', { detail: { command, task } })); await load(); }
      catch (error) { state.textContent = error.message; state.hidden = false; }
      finally { controlBusy = false; for (const control of actions.querySelectorAll('button')) control.disabled = false; }
    }; actions.append(node);
  }
  async function load() {
    const version = ++stateVersion;
    const requested = context.teachingTaskId || context.taskId || '';
    try {
      const task = await api.request('/api/learning-agent' + (requested ? '?taskId=' + encodeURIComponent(requested) : ''));
      if (version !== stateVersion || requested !== (context.teachingTaskId || context.taskId || '')) return;
      current = task;
      const active = ['pending', 'running'].includes(task.status), paused = task.paused || task.status === 'paused';
      navigation.dataset.status = paused ? 'paused' : task.status;
      toggle.textContent = paused ? 'AI 已暂停' : active ? 'AI 处理中' : task.status === 'failed' ? 'AI 需处理' : 'AI 状态';
      label.textContent = toggle.textContent;
      renderOperation(task, paused); renderHistory(task); renderFacts(task);
      const nextActions = JSON.stringify([task.id, paused, task.status, task.lesson?.entered]);
      if (actionsKey !== nextActions) {
        actionsKey = nextActions; actions.replaceChildren();
        if (task.id) {
          if (paused) button('恢复 AI', 'resume', task.id); else button('暂停 AI', 'pause', task.id);
          if (['failed', 'superseded', 'cancelled'].includes(task.status)) button('重试', 'retry', task.id);
          if (active || task.status === 'paused') button('停止任务', 'cancel', task.id);
          if (task.lesson && !task.lesson.entered && ['ready', 'waiting_student'].includes(task.status)) actions.append(anchor('进入已备好的课堂 →', '/agent-prepare.html?taskId=' + encodeURIComponent(task.id)));
          for (const control of actions.querySelectorAll('button')) control.disabled = controlBusy;
        }
      }
      const applied = (task.changeHistory || []).filter(item => item.status === 'applied').slice(-5).reverse(), nextChanges = JSON.stringify(applied);
      adjustments.hidden = !applied.length;
      if (changesKey === nextChanges) return; changesKey = nextChanges; changes.replaceChildren();
      for (const change of applied) {
        const row = element('article'), reason = element('p', change.reason), undo = element('button', '撤回这次调整'); undo.type = 'button';
        undo.onclick = async () => { undo.disabled = true; try { await api.control({ command: 'undo', changeId: change.id }); await load(); } catch (error) { reason.textContent = error.message; } finally { undo.disabled = false; } };
        row.append(reason, undo); changes.append(row);
      }
    } catch (error) { if (version === stateVersion && requested === (context.teachingTaskId || context.taskId || '')) { state.textContent = '无法更新任务状态：' + error.message; state.hidden = false; } }
  }
  navigation.addEventListener('toggle', () => { if (navigation.open) load(); });
  document.addEventListener('keydown', event => { if (event.key === 'Escape' && navigation.open) { navigation.open = false; toggle.focus(); } });
  load(); const timer = setInterval(() => { if (!document.hidden) load(); }, 1800); window.addEventListener('pagehide', () => clearInterval(timer));
})();

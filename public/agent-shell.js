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
  const label = element('h2', 'AI 教学状态'), state = element('p'); state.setAttribute('role', 'status');
  const actions = element('div', '', 'ai-control-actions'), changes = element('div', '', 'ai-control-changes');
  panel.append(label, state, actions, changes); body.append(panel); navigation.append(toggle, body); document.body.append(navigation);
  function button(title, command) {
    const node = element('button', title); node.type = 'button'; node.onclick = async () => {
      node.disabled = true;
      try { const task = await api.control({ command, taskId: context.teachingTaskId || context.taskId || current.id }); document.dispatchEvent(new CustomEvent('gangyi:agent-control', { detail: { command, task } })); await load(); }
      catch (error) { state.textContent = error.message; } finally { node.disabled = false; }
    }; actions.append(node);
  }
  async function load() {
    const version = ++stateVersion;
    const requested = context.teachingTaskId || context.taskId || '';
    try {
      const task = await api.request('/api/learning-agent' + (requested ? '?taskId=' + encodeURIComponent(requested) : ''));
      if (version !== stateVersion || requested !== (context.teachingTaskId || context.taskId || '')) return;
      current = task; actions.replaceChildren(); changes.replaceChildren();
      const active = ['pending', 'running'].includes(task.status), paused = task.paused || task.status === 'paused';
      navigation.dataset.status = paused ? 'paused' : task.status;
      toggle.textContent = active ? 'AI 处理中' : paused ? 'AI 已暂停' : task.status === 'failed' ? 'AI 需处理' : 'AI 状态';
      label.textContent = toggle.textContent;
      state.textContent = task.status === 'failed' ? api.failureMessage(task) : paused ? '输入和有效结果已保存，可继续处理。' :
        ({ idle: '暂无正在执行的 AI 任务。', pending: '正在等待 AI 处理。', running: '正在结合最新学习情况处理。', ready: '本次处理已完成。', waiting_student: '等待你的回答或补充信息。', superseded: '学习记录已更新，旧输出没有应用。', cancelled: '本次任务已停止。' })[task.status] || 'AI 状态已保存。';
      if (!task.id) return;
      if (paused) button('恢复 AI', 'resume'); else button('暂停 AI', 'pause');
      if (['failed', 'superseded', 'cancelled'].includes(task.status)) button('重试', 'retry');
      if (task.lesson && !task.lesson.entered && ['ready', 'waiting_student'].includes(task.status)) actions.append(anchor('进入已备好的课堂 →', '/agent-prepare.html?taskId=' + encodeURIComponent(task.id)));
      for (const change of (task.changeHistory || []).filter(item => item.status === 'applied').slice(-5).reverse()) {
        const row = element('article'), reason = element('p', change.reason), undo = element('button', '撤回这次调整'); undo.type = 'button';
        undo.onclick = async () => { undo.disabled = true; try { await api.control({ command: 'undo', changeId: change.id }); await load(); } catch (error) { reason.textContent = error.message; } finally { undo.disabled = false; } };
        row.append(reason, undo); changes.append(row);
      }
    } catch (error) { state.textContent = error.message; }
  }
  document.addEventListener('keydown', event => { if (event.key === 'Escape' && navigation.open) { navigation.open = false; toggle.focus(); } });
  load(); const timer = setInterval(() => { if (!document.hidden) load(); }, 1800); window.addEventListener('pagehide', () => clearInterval(timer));
})();

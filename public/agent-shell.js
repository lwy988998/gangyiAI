/* 全站共用悬浮导航，原有 AI 控制保留在同一菜单。 */
(() => {
  'use strict'; const api = window.GangyiAgent; if (!api || document.getElementById('learning-navigation')) return;
  const navigation = document.createElement('details'); navigation.id = 'learning-navigation';
  const toggle = document.createElement('summary'); toggle.setAttribute('aria-label', '打开学习导航'); toggle.textContent = '导航';
  const body = document.createElement('div'); body.className = 'gy-navigation-body'; const links = document.createElement('nav'); links.className = 'gy-navigation-links'; links.setAttribute('aria-label', '学习导航');
  const panel = document.createElement('details'); panel.id = 'agent-control-panel'; const label = document.createElement('summary'); label.textContent = 'AI 教学状态';
  const state = document.createElement('p'); state.setAttribute('role', 'status'); const actions = document.createElement('div'), changes = document.createElement('div');
  panel.append(label, state, actions, changes); body.append(links, panel); navigation.append(toggle, body); document.body.append(navigation);
  let current, context = Object.fromEntries(new URLSearchParams(location.search)), outlineCourse = '', outline = [];
  const localLink = value => typeof value === 'string' && value.startsWith('/') && !value.startsWith('//');
  function link(title, href) { if (!localLink(href)) return; const node = document.createElement('a'); node.textContent = title; node.href = href; links.append(node); }
  function renderLinks() {
    links.replaceChildren(); if (context.lessonId) {
      for (const [path, title] of [['learn', '讲解'], ['practice', '练习'], ['summary', '小结']]) {
        const params = new URLSearchParams({lessonId:context.lessonId});
        for(const key of ['review','reviewId','day','lessonTaskId'])if(context[key]!==undefined)params.set(key,context[key]);
        link(title, `/${path}?${params}`);
      }
      const position = outline.findIndex(item => context.topicId ? item.id === context.topicId : Number(item.legacyPhaseIndex) === Number(context.phaseIndex) && Number(item.legacyTopicIndex) === Number(context.topicIndex));
      if (position >= 0) { const href = item => '/learn?' + new URLSearchParams({courseId: context.courseId, ...(item.id ? {topicId: item.id} : {}), phaseIndex: item.legacyPhaseIndex, topicIndex: item.legacyTopicIndex}); if (position > 0) link('← 上一节', href(outline[position - 1])); if (position < outline.length - 1) link('下一节 →', href(outline[position + 1])); link('返回阶段', '/phase?' + new URLSearchParams({courseId: context.courseId, phaseIndex: outline[position].displayPhase})); }
      else if (context.phaseIndex) link('返回阶段', '/phase?' + new URLSearchParams({courseId: context.courseId, phaseIndex: context.phaseIndex}));
    }
    if (context.courseId) link('返回课程', '/plan?courseId=' + encodeURIComponent(context.courseId));
    try { const saved = JSON.parse(api.storage.get('current-lesson') || 'null'); if (saved && localLink(saved.href)) link('回到当前这一节', saved.href); } catch (_) { /* 损坏缓存不产生跳转。 */ }
    link('我的课程', '/my-courses'); link('我的画像', '/my-courses#profile'); link('首页', '/');
    const name = {'/learn': '讲解', '/practice': '练习', '/summary': '小结', '/phase': '阶段', '/plan': '课程', '/my-courses': '我的课程'}[location.pathname]; toggle.textContent = name ? `${name} · 导航` : '导航';
  }
  async function setContext(value) {
    context = {...context, ...value};
    document.dispatchEvent(new CustomEvent('gangyi:navigation-context', {detail: context}));
    document.querySelectorAll('[data-current-course-link]').forEach(node => { node.href = context.courseId ? '/plan?courseId=' + encodeURIComponent(context.courseId) : '/my-courses'; });
    renderLinks(); if (!context.courseId || outlineCourse === context.courseId) return;
    const courseId = context.courseId; outlineCourse = courseId;
    try { const data = await api.request('/api/courses/' + encodeURIComponent(courseId)); if (courseId !== context.courseId) return; outline = (data.snapshot?.payload?.courseStructure || []).flatMap((stage, index) => (stage.topics || []).map((topic, topicIndex) => ({...(typeof topic === 'object' ? topic : {title: topic}), displayPhase: index + 1, legacyPhaseIndex: topic.legacyPhaseIndex || index + 1, legacyTopicIndex: topic.legacyTopicIndex || topicIndex + 1}))); renderLinks(); } catch (_) { outlineCourse = ''; }
  }
  window.GangyiNavigation = {setContext}; setContext(context);
  const button = (title, command) => { const node = document.createElement('button'); node.type = 'button'; node.textContent = title; node.onclick = async () => { node.disabled = true; try { const taskId=context.lessonId&&context.teachingTaskId?context.teachingTaskId:current.id; const task=await api.control({command, taskId}); document.dispatchEvent(new CustomEvent('gangyi:agent-control',{detail:{command,task}})); await load(); } catch (error) { state.textContent = error.message; } finally { node.disabled = false; } }; actions.append(node); };
  async function load() {
    try {
      const requestedTaskId=context.lessonId&&context.teachingTaskId?context.teachingTaskId:'';
      const task = await api.request('/api/learning-agent' + (requestedTaskId ? '?taskId=' + encodeURIComponent(requestedTaskId) : ''));
      if(requestedTaskId!==(context.lessonId&&context.teachingTaskId?context.teachingTaskId:''))return;
      current = task; panel.hidden = !task.id; if (!task.id) return;
      const active = ['pending', 'running'].includes(task.status), paused = task.paused || task.status === 'paused', prepared = ['ready', 'waiting_student'].includes(task.status) && task.lesson && !task.lesson.entered;
      state.textContent = paused ? 'AI 已暂停，输入和有效结果已保存。' : ({pending: 'AI 正在等待处理。', running: 'AI 正在处理最新学习情况。', ready: 'AI 已完成本次处理。', waiting_student: 'AI 等待你的回答。', failed: 'AI 请求失败，可重试。', superseded: '旧结果未应用。', cancelled: '已停止。'})[task.status] || 'AI 状态已保存。'; label.textContent = active ? 'AI 教学进行中' : paused ? 'AI 已暂停' : prepared ? 'AI 已备好下一课' : 'AI 教学状态'; actions.replaceChildren();
      if (paused) button('恢复 AI', 'resume'); else button('暂停 AI', 'pause'); if (['failed', 'superseded', 'cancelled'].includes(task.status)) button('重试', 'retry');
      if (prepared) { const node = document.createElement('a'); node.textContent = `已备好：${task.lesson.title} →`; node.href = '/agent-prepare.html?taskId=' + encodeURIComponent(task.id); actions.append(node); }
      changes.replaceChildren(); for (const change of (task.changeHistory || []).filter(item => item.status === 'applied').slice(-5).reverse()) { const row = document.createElement('article'), reason = document.createElement('p'), undo = document.createElement('button'); reason.textContent = change.reason; undo.type = 'button'; undo.textContent = '撤回这次调整'; undo.onclick = async () => { undo.disabled = true; try { await api.control({command: 'undo', changeId: change.id}); await load(); } catch (error) { reason.textContent = error.message; } finally { undo.disabled = false; } }; row.append(reason, undo); changes.append(row); }
    } catch (error) { state.textContent = error.message; }
  }
  document.addEventListener('keydown', event => { if (event.key === 'Escape' && navigation.open) { navigation.open = false; toggle.focus(); } }); load(); const timer = setInterval(() => { if (!document.hidden) load(); }, 1800); window.addEventListener('pagehide', () => clearInterval(timer));
})();

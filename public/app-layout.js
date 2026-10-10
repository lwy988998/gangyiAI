/* 分页与页签只改变呈现，不删除内容或触发 AI 生成。 */
(() => {
  'use strict'; const initialized = new WeakSet();
  function paginate() {
    document.querySelectorAll('[data-paged-list]').forEach(root => {
      if (initialized.has(root)) return; initialized.add(root);
      const rows = [...root.children], size = Number(root.dataset.pageSize || 8); if (rows.length <= size) return;
      const pages = Math.ceil(rows.length / size), controls = document.createElement('nav'); controls.className = 'gy-pagination'; controls.setAttribute('aria-label', '列表分页');
      const previous = document.createElement('button'), next = document.createElement('button'), label = document.createElement('span'); previous.type = next.type = 'button'; previous.textContent = '上一页'; next.textContent = '下一页'; label.setAttribute('aria-live', 'polite'); controls.append(previous, label, next); root.after(controls);
      let page = 0; function paint() { rows.forEach((row, index) => { row.hidden = index < page * size || index >= (page + 1) * size; }); label.textContent = `${page + 1} / ${pages}`; previous.disabled = page === 0; next.disabled = page === pages - 1; }
      previous.onclick = () => { page--; paint(); }; next.onclick = () => { page++; paint(); }; paint();
    });
  }
  paginate(); document.addEventListener('gangyi:layout', paginate);
  const tabs = [...document.querySelectorAll('[data-uc-tab]')];
  function activate(name) { tabs.forEach(tab => { const active = tab.dataset.ucTab === name; tab.setAttribute('aria-selected', String(active)); tab.tabIndex = active ? 0 : -1; document.getElementById(tab.getAttribute('aria-controls')).hidden = !active; }); window.dispatchEvent(new Event('resize')); }
  if (tabs.length) {
    const fromHash = () => activate(['#profile', '#uc-profile', '#uc-time'].includes(location.hash) ? 'profile' : 'courses');
    tabs.forEach((tab, index) => { tab.onclick = () => { history.replaceState(null, '', tab.dataset.ucTab === 'profile' ? '#profile' : location.pathname); activate(tab.dataset.ucTab); }; tab.onkeydown = event => { if (!['ArrowLeft', 'ArrowRight', 'Home', 'End'].includes(event.key)) return; event.preventDefault(); const selected = event.key === 'Home' ? 0 : event.key === 'End' ? tabs.length - 1 : (index + (event.key === 'ArrowLeft' ? -1 : 1) + tabs.length) % tabs.length; tabs[selected].click(); tabs[selected].focus(); }; }); fromHash(); window.addEventListener('hashchange', fromHash);
  }
})();

/* C 方向的课程目录只读取保存的大纲和进度，不触发备课或评分。 */
(() => {
  'use strict';
  if (!['/plan', '/phase', '/learn', '/practice', '/summary'].includes(location.pathname)) return;
  const main = document.querySelector('main'); if (!main) return;
  let context = Object.fromEntries(new URLSearchParams(location.search)), loadedCourse = '', saved, requestVersion = 0;
  const desktop = document.createElement('aside'); desktop.className = 'course-directory'; desktop.hidden = true; desktop.setAttribute('aria-label', '当前课程目录');
  const mobile = document.createElement('details'); mobile.className = 'course-directory-mobile'; mobile.hidden = true;
  document.body.append(desktop); main.prepend(mobile);
  const element = (tag, text = '', className = '') => { const node = document.createElement(tag); node.textContent = text; node.className = className; return node; };
  let collapsed = false; try { collapsed = localStorage.getItem('gy:directory-collapsed') === '1'; } catch (_) { /* 存储不可用时默认显示目录。 */ }
  const toggle = element('button', '', 'course-directory-toggle'); toggle.type = 'button'; toggle.hidden = true;
  toggle.setAttribute('aria-controls', 'course-directory'); desktop.id = 'course-directory'; document.body.append(toggle);
  function paintCollapse() {
    document.body.classList.toggle('gy-directory-collapsed', collapsed); desktop.hidden = collapsed || !saved;
    toggle.textContent = collapsed ? '☰ 展开目录' : '‹ 收起目录'; toggle.setAttribute('aria-expanded', String(!collapsed));
  }
  toggle.onclick = () => { collapsed = !collapsed; try { localStorage.setItem('gy:directory-collapsed', collapsed ? '1' : '0'); } catch (_) { /* 本次操作仍生效。 */ } paintCollapse(); };
  document.addEventListener('keydown', event => { if (event.key === 'Escape' && mobile.open) { mobile.open = false; mobile.querySelector('summary')?.focus(); } });
  function content() {
    const courseId = context.courseId, stages = saved.snapshot?.payload?.courseStructure || [], cards = saved.cards || [];
    const stageTopics = stages.map((stage, phase) => (stage.topics || []).map((value, index) => {
      const topic = typeof value === 'string' ? {title: value} : value;
      return {...topic, displayPhase: phase + 1, legacyPhaseIndex: topic.legacyPhaseIndex || phase + 1, legacyTopicIndex: topic.legacyTopicIndex || index + 1};
    }));
    const matches = topic => context.topicId ? topic.id === context.topicId : Number(context.phaseIndex) === Number(topic.legacyPhaseIndex) && Number(context.topicIndex) === Number(topic.legacyTopicIndex);
    const selectedPhase = location.pathname === '/phase' ? Number(context.phaseIndex || 1) - 1 : stageTopics.findIndex(topics => topics.some(matches));
    const root = element('div', '', 'course-directory-body'), heading = element('div', '', 'course-directory-heading');
    heading.append(element('p', '课程目录'), element('h2', saved.course.title || saved.course.goal));
    const back = element('a', '课程目标与进度 →'); back.href = '/plan?courseId=' + encodeURIComponent(courseId); heading.append(back); root.append(heading);
    const tree = element('nav', '', 'course-directory-tree'); tree.setAttribute('aria-label', '阶段与知识点');
    let done = 0, total = 0;
    stages.forEach((stage, index) => {
      const chapter = element('details'), summary = element('summary'), label = element('a', stage.stage || '阶段 ' + (index + 1));
      chapter.open = index === Math.max(0, selectedPhase); label.href = '/phase?' + new URLSearchParams({courseId, phaseIndex: index + 1});
      chapter.classList.toggle('is-active-stage', index === selectedPhase);
      const stageDone = stageTopics[index].filter(topic => cards.some(card => Number(card.phaseIndex) === Number(topic.legacyPhaseIndex) && Number(card.topicIndex) === Number(topic.legacyTopicIndex) && card.status === 'completed')).length;
      summary.append(element('span', String(index + 1).padStart(2, '0'), 'directory-stage-number'), element('strong', label.textContent), element('span', `${stageDone}/${stageTopics[index].length}`, 'directory-stage-progress'));
      label.textContent = '阶段目标与清单 →'; label.className = 'directory-stage-link'; if (index === selectedPhase && location.pathname === '/phase') label.setAttribute('aria-current', 'page');
      chapter.append(summary, label);
      stageTopics[index].forEach(topic => {
        const status = cards.find(card => Number(card.phaseIndex) === Number(topic.legacyPhaseIndex) && Number(card.topicIndex) === Number(topic.legacyTopicIndex))?.status;
        const completed = status === 'completed', active = matches(topic) && ['/learn', '/practice', '/summary'].includes(location.pathname);
        const link = element('a', '', 'course-directory-topic' + (completed ? ' is-complete' : ''));
        link.href = '/learn?' + new URLSearchParams({courseId, phaseIndex: topic.legacyPhaseIndex, topicIndex: topic.legacyTopicIndex, ...(topic.id ? {topicId: topic.id} : {})});
        if (active) link.setAttribute('aria-current', 'page');
        const mark = element('span', completed ? '✓' : active ? '●' : '○'); mark.setAttribute('aria-hidden', 'true');
        link.append(mark, element('span', topic.title));
        link.setAttribute('aria-label', topic.title + (completed ? '，已完成' : active ? '，当前知识点' : ''));
        chapter.append(link); total++; if (completed) done++;
      });
      tree.append(chapter);
    });
    const progress = element('progress'); progress.max = total || 1; progress.value = done; progress.setAttribute('aria-label', '课程已完成课时');
    const footer = element('div', '', 'course-directory-foot'); footer.append(element('p', `${done} / ${total} 节已完成`), progress, element('small', '阅读与追问不自动计为掌握')); root.append(tree, footer);
    return root;
  }
  function render() {
    desktop.replaceChildren(content());
    const label = element('summary', '课程目录 · ' + (saved.course.title || saved.course.goal));
    mobile.replaceChildren(label, content()); mobile.hidden = false; toggle.hidden = false; paintCollapse();
    document.body.classList.add('gy-course-view');
  }
  async function load(value, force = false) {
    context = {...context, ...value}; if (!context.courseId) return;
    if (loadedCourse === context.courseId && saved && !force) { render(); return; }
    const courseId = context.courseId, version = ++requestVersion;
    try {
      const response = await fetch('/api/courses/' + encodeURIComponent(courseId), {cache: 'no-store'});
      const data = await response.json(); if (!response.ok) throw new Error(data.error || '课程目录读取失败');
      if (version !== requestVersion || context.courseId !== courseId) return;
      saved = data; loadedCourse = courseId; render();
    } catch (error) {
      if (version !== requestVersion) return;
      desktop.replaceChildren(element('p', error.message, 'course-directory-foot')); desktop.hidden = false;
      document.body.classList.add('gy-course-view');
    }
  }
  document.addEventListener('gangyi:navigation-context', event => load(event.detail));
  document.addEventListener('gangyi:lesson-context', event => load(event.detail));
  document.addEventListener('gangyi:lesson-finished', () => load(context, true));
  load(context);
})();

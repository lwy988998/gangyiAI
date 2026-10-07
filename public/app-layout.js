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
    const back = element('a', '← 我的课程'); back.href = '/my-courses'; heading.append(back); root.append(heading);
    const tree = element('nav', '', 'course-directory-tree'); tree.setAttribute('aria-label', '阶段与知识点');
    let done = 0, total = 0;
    stages.forEach((stage, index) => {
      const chapter = element('details'), summary = element('summary'), label = element('a', stage.stage || '阶段 ' + (index + 1));
      chapter.open = index === Math.max(0, selectedPhase); label.href = '/phase?' + new URLSearchParams({courseId, phaseIndex: index + 1});
      summary.append(element('span', String(index + 1).padStart(2, '0')), label); chapter.append(summary);
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
    root.append(tree, element('p', `真实记录 · ${done} / ${total} 节已完成\n阅读与追问不自动计为掌握`, 'course-directory-foot'));
    return root;
  }
  function render() {
    desktop.replaceChildren(content());
    const label = element('summary', '课程目录 · ' + (saved.course.title || saved.course.goal));
    mobile.replaceChildren(label, content()); desktop.hidden = mobile.hidden = false;
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

/* 分页与页签只改变呈现，不删除内容或触发 AI 生成。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict'; const initialized = new WeakSet();
  // 【逐行批注】 定义函数 paginate，封装可复用的前端行为。
  function paginate() {
    // 【逐行批注】 计算右侧结果并更新 document.querySelectorAll('[data-paged-list]…，改变页面或请求状态。
    document.querySelectorAll('[data-paged-list]').forEach(root => {
      // 【逐行批注】 判断条件 (initialized.has(root)) return; initialized.add(root);，成立时执行对应逻辑。
      if (initialized.has(root)) return; initialized.add(root);
      // 【逐行批注】 声明前端变量 rows，保存本行计算出的数据或对象。
      const rows = [...root.children], size = Number(root.dataset.pageSize || 8); if (rows.length <= size) return;
      // 【逐行批注】 声明前端变量 pages，保存本行计算出的数据或对象。
      const pages = Math.ceil(rows.length / size), controls = document.createElement('nav'); controls.className = 'gy-pagination'; controls.setAttribute('aria-label', '列表分页');
      // 【逐行批注】 声明前端变量 previous，保存本行计算出的数据或对象。
      const previous = document.createElement('button'), next = document.createElement('button'), label = document.createElement('span'); previous.type = next.type = 'button'; previous.textContent = '上一页'; next.textContent = '下一页'; label.setAttribute('aria-live', 'polite'); controls.append(previous, label, next); root.after(controls);
      // 【逐行批注】 声明前端变量 page，保存本行计算出的数据或对象。
      let page = 0; function paint() { rows.forEach((row, index) => { row.hidden = index < page * size || index >= (page + 1) * size; }); label.textContent = `${page + 1} / ${pages}`; previous.disabled = page === 0; next.disabled = page === pages - 1; }
      // 【逐行批注】 计算右侧结果并更新 previous.onclick，改变页面或请求状态。
      previous.onclick = () => { page--; paint(); }; next.onclick = () => { page++; paint(); }; paint();
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 为页面事件绑定处理逻辑：paginate(); document.addEventListener('gangyi:layout', paginate);。
  paginate(); document.addEventListener('gangyi:layout', paginate);
  // 【逐行批注】 声明前端变量 tabs，保存本行计算出的数据或对象。
  const tabs = [...document.querySelectorAll('[data-uc-tab]')];
  // 【逐行批注】 定义函数 activate，封装可复用的前端行为。
  function activate(name) { tabs.forEach(tab => { const active = tab.dataset.ucTab === name; tab.setAttribute('aria-selected', String(active)); tab.tabIndex = active ? 0 : -1; document.getElementById(tab.getAttribute('aria-controls')).hidden = !active; }); window.dispatchEvent(new Event('resize')); }
  // 【逐行批注】 判断条件 (tabs.length) {，成立时执行对应逻辑。
  if (tabs.length) {
    // 【逐行批注】 声明前端变量 fromHash，保存本行计算出的数据或对象。
    const fromHash = () => activate(['#profile', '#uc-profile', '#uc-time'].includes(location.hash) ? 'profile' : 'courses');
    // 【逐行批注】 为页面事件绑定处理逻辑：tabs.forEach((tab, index) => { tab.onclick = () => { history.replaceState(null, '', …。
    tabs.forEach((tab, index) => { tab.onclick = () => { history.replaceState(null, '', tab.dataset.ucTab === 'profile' ? '#profile' : location.pathname); activate(tab.dataset.ucTab); }; tab.onkeydown = event => { if (!['ArrowLeft', 'ArrowRight', 'Home', 'End'].includes(event.key)) return; event.preventDefault(); const selected = event.key === 'Home' ? 0 : event.key === 'End' ? tabs.length - 1 : (index + (event.key === 'ArrowLeft' ? -1 : 1) + tabs.length) % tabs.length; tabs[selected].click(); tabs[selected].focus(); }; }); fromHash(); window.addEventListener('hashchange', fromHash);
  // 【逐行批注】 结束当前前端代码块。
  }
// 【逐行批注】 执行当前前端表达式：})();。
})();

/* C 方向的课程目录只读取保存的大纲和进度，不触发备课或评分。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 判断条件 (!['/plan', '/phase', '/learn', '/practice', '/summary'].include…，成立时执行对应逻辑。
  if (!['/plan', '/phase', '/learn', '/practice', '/summary'].includes(location.pathname)) return;
  // 【逐行批注】 声明前端变量 main，保存本行计算出的数据或对象。
  const main = document.querySelector('main'); if (!main) return;
  // 【逐行批注】 声明前端变量 context，保存本行计算出的数据或对象。
  let context = Object.fromEntries(new URLSearchParams(location.search)), loadedCourse = '', saved, requestVersion = 0;
  // 【逐行批注】 声明前端变量 desktop，保存本行计算出的数据或对象。
  const desktop = document.createElement('aside'); desktop.className = 'course-directory'; desktop.hidden = true; desktop.setAttribute('aria-label', '当前课程目录');
  // 【逐行批注】 声明前端变量 mobile，保存本行计算出的数据或对象。
  const mobile = document.createElement('details'); mobile.className = 'course-directory-mobile'; mobile.hidden = true;
  // 【逐行批注】 执行当前前端表达式：document.body.append(desktop); main.prepend(mobile);。
  document.body.append(desktop); main.prepend(mobile);
  // 【逐行批注】 声明前端变量 element，保存本行计算出的数据或对象。
  const element = (tag, text = '', className = '') => { const node = document.createElement(tag); node.textContent = text; node.className = className; return node; };
  // 【逐行批注】 声明前端变量 collapsed，保存本行计算出的数据或对象。
  let collapsed = false; try { collapsed = localStorage.getItem('gy:directory-collapsed') === '1'; } catch (_) { /* 存储不可用时默认显示目录。 */ }
  // 【逐行批注】 声明前端变量 toggle，保存本行计算出的数据或对象。
  const toggle = element('button', '', 'course-directory-toggle'); toggle.type = 'button'; toggle.hidden = true;
  // 【逐行批注】 计算右侧结果并更新 toggle.setAttribute('aria-controls', 'course…，改变页面或请求状态。
  toggle.setAttribute('aria-controls', 'course-directory'); desktop.id = 'course-directory'; document.body.append(toggle);
  // 【逐行批注】 定义函数 paintCollapse，封装可复用的前端行为。
  function paintCollapse() {
    // 【逐行批注】 计算右侧结果并更新 document.body.classList.toggle('gy-directory…，改变页面或请求状态。
    document.body.classList.toggle('gy-directory-collapsed', collapsed); desktop.hidden = collapsed || !saved;
    // 【逐行批注】 计算右侧结果并更新 toggle.textContent，改变页面或请求状态。
    toggle.textContent = collapsed ? '☰ 展开目录' : '‹ 收起目录'; toggle.setAttribute('aria-expanded', String(!collapsed));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 计算右侧结果并更新 toggle.onclick，改变页面或请求状态。
  toggle.onclick = () => { collapsed = !collapsed; try { localStorage.setItem('gy:directory-collapsed', collapsed ? '1' : '0'); } catch (_) { /* 本次操作仍生效。 */ } paintCollapse(); };
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('keydown', event => { if (event.key === 'Escape' && mobile…。
  document.addEventListener('keydown', event => { if (event.key === 'Escape' && mobile.open) { mobile.open = false; mobile.querySelector('summary')?.focus(); } });
  // 【逐行批注】 定义函数 content，封装可复用的前端行为。
  function content() {
    // 【逐行批注】 声明前端变量 courseId，保存本行计算出的数据或对象。
    const courseId = context.courseId, stages = saved.snapshot?.payload?.courseStructure || [], cards = saved.cards || [];
    // 【逐行批注】 声明前端变量 stageTopics，保存本行计算出的数据或对象。
    const stageTopics = stages.map((stage, phase) => (stage.topics || []).map((value, index) => {
      // 【逐行批注】 声明前端变量 topic，保存本行计算出的数据或对象。
      const topic = typeof value === 'string' ? {title: value} : value;
      // 【逐行批注】 结束当前函数并返回结果：return {...topic, displayPhase: phase + 1, legacyPhaseIndex: topic.le…。
      return {...topic, displayPhase: phase + 1, legacyPhaseIndex: topic.legacyPhaseIndex || phase + 1, legacyTopicIndex: topic.legacyTopicIndex || index + 1};
    // 【逐行批注】 执行当前前端表达式：}));。
    }));
    // 【逐行批注】 声明前端变量 matches，保存本行计算出的数据或对象。
    const matches = topic => context.topicId ? topic.id === context.topicId : Number(context.phaseIndex) === Number(topic.legacyPhaseIndex) && Number(context.topicIndex) === Number(topic.legacyTopicIndex);
    // 【逐行批注】 声明前端变量 selectedPhase，保存本行计算出的数据或对象。
    const selectedPhase = location.pathname === '/phase' ? Number(context.phaseIndex || 1) - 1 : stageTopics.findIndex(topics => topics.some(matches));
    // 【逐行批注】 声明前端变量 root，保存本行计算出的数据或对象。
    const root = element('div', '', 'course-directory-body'), heading = element('div', '', 'course-directory-heading');
    // 【逐行批注】 调用 element 执行页面交互或数据处理：heading.append(element('p', '课程目录'), element('h2', saved.course.title…。
    heading.append(element('p', '课程目录'), element('h2', saved.course.title || saved.course.goal));
    // 【逐行批注】 声明前端变量 back，保存本行计算出的数据或对象。
    const back = element('a', '课程目标与进度 →'); back.href = '/plan?courseId=' + encodeURIComponent(courseId); heading.append(back); root.append(heading);
    // 【逐行批注】 声明前端变量 tree，保存本行计算出的数据或对象。
    const tree = element('nav', '', 'course-directory-tree'); tree.setAttribute('aria-label', '阶段与知识点');
    // 【逐行批注】 声明前端变量 done，保存本行计算出的数据或对象。
    let done = 0, total = 0;
    // 【逐行批注】 计算右侧结果并更新 stages.forEach((stage, index)，改变页面或请求状态。
    stages.forEach((stage, index) => {
      // 【逐行批注】 声明前端变量 chapter，保存本行计算出的数据或对象。
      const chapter = element('details'), summary = element('summary'), label = element('a', stage.stage || '阶段 ' + (index + 1));
      // 【逐行批注】 计算右侧结果并更新 chapter.open，改变页面或请求状态。
      chapter.open = index === Math.max(0, selectedPhase); label.href = '/phase?' + new URLSearchParams({courseId, phaseIndex: index + 1});
      // 【逐行批注】 执行当前前端表达式：chapter.classList.toggle('is-active-stage', index === selectedPhase);。
      chapter.classList.toggle('is-active-stage', index === selectedPhase);
      // 【逐行批注】 声明前端变量 stageDone，保存本行计算出的数据或对象。
      const stageDone = stageTopics[index].filter(topic => cards.some(card => Number(card.phaseIndex) === Number(topic.legacyPhaseIndex) && Number(card.topicIndex) === Number(topic.legacyTopicIndex) && card.status === 'completed')).length;
      // 【逐行批注】 调用 element、String 执行页面交互或数据处理：summary.append(element('span', String(index + 1).padStart(2, '0'), 'd…。
      summary.append(element('span', String(index + 1).padStart(2, '0'), 'directory-stage-number'), element('strong', label.textContent), element('span', `${stageDone}/${stageTopics[index].length}`, 'directory-stage-progress'));
      // 【逐行批注】 计算右侧结果并更新 label.textContent，改变页面或请求状态。
      label.textContent = '阶段目标与清单 →'; label.className = 'directory-stage-link'; if (index === selectedPhase && location.pathname === '/phase') label.setAttribute('aria-current', 'page');
      // 【逐行批注】 执行当前前端表达式：chapter.append(summary, label);。
      chapter.append(summary, label);
      // 【逐行批注】 计算右侧结果并更新 stageTopics[index].forEach(topic，改变页面或请求状态。
      stageTopics[index].forEach(topic => {
        // 【逐行批注】 声明前端变量 status，保存本行计算出的数据或对象。
        const status = cards.find(card => Number(card.phaseIndex) === Number(topic.legacyPhaseIndex) && Number(card.topicIndex) === Number(topic.legacyTopicIndex))?.status;
        // 【逐行批注】 声明前端变量 completed，保存本行计算出的数据或对象。
        const completed = status === 'completed', active = matches(topic) && ['/learn', '/practice', '/summary'].includes(location.pathname);
        // 【逐行批注】 声明前端变量 link，保存本行计算出的数据或对象。
        const link = element('a', '', 'course-directory-topic' + (completed ? ' is-complete' : ''));
        // 【逐行批注】 计算右侧结果并更新 link.href，改变页面或请求状态。
        link.href = '/learn?' + new URLSearchParams({courseId, phaseIndex: topic.legacyPhaseIndex, topicIndex: topic.legacyTopicIndex, ...(topic.id ? {topicId: topic.id} : {})});
        // 【逐行批注】 判断条件 (active) link.setAttribute('aria-current', 'page');，成立时执行对应逻辑。
        if (active) link.setAttribute('aria-current', 'page');
        // 【逐行批注】 声明前端变量 mark，保存本行计算出的数据或对象。
        const mark = element('span', completed ? '✓' : active ? '●' : '○'); mark.setAttribute('aria-hidden', 'true');
        // 【逐行批注】 调用 element 执行页面交互或数据处理：link.append(mark, element('span', topic.title));。
        link.append(mark, element('span', topic.title));
        // 【逐行批注】 执行当前前端表达式：link.setAttribute('aria-label', topic.title + (completed ? '，已完成' : active ? '，当前知识点' : '…。
        link.setAttribute('aria-label', topic.title + (completed ? '，已完成' : active ? '，当前知识点' : ''));
        // 【逐行批注】 执行当前前端表达式：chapter.append(link); total++; if (completed) done++;。
        chapter.append(link); total++; if (completed) done++;
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 执行当前前端表达式：tree.append(chapter);。
      tree.append(chapter);
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 【逐行批注】 声明前端变量 progress，保存本行计算出的数据或对象。
    const progress = element('progress'); progress.max = total || 1; progress.value = done; progress.setAttribute('aria-label', '课程已完成课时');
    // 【逐行批注】 声明前端变量 footer，保存本行计算出的数据或对象。
    const footer = element('div', '', 'course-directory-foot'); footer.append(element('p', `${done} / ${total} 节已完成`), progress, element('small', '阅读与追问不自动计为掌握')); root.append(tree, footer);
    // 【逐行批注】 结束当前函数并返回结果：return root;。
    return root;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 render，封装可复用的前端行为。
  function render() {
    // 【逐行批注】 调用 content 执行页面交互或数据处理：desktop.replaceChildren(content());。
    desktop.replaceChildren(content());
    // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
    const label = element('summary', '课程目录 · ' + (saved.course.title || saved.course.goal));
    // 【逐行批注】 计算右侧结果并更新 mobile.replaceChildren(label, content()); mo…，改变页面或请求状态。
    mobile.replaceChildren(label, content()); mobile.hidden = false; toggle.hidden = false; paintCollapse();
    // 【逐行批注】 执行当前前端表达式：document.body.classList.add('gy-course-view');。
    document.body.classList.add('gy-course-view');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 load，封装可复用的前端行为。
  async function load(value, force = false) {
    // 【逐行批注】 计算右侧结果并更新 context，改变页面或请求状态。
    context = {...context, ...value}; if (!context.courseId) return;
    // 【逐行批注】 判断条件 (loadedCourse === context.courseId && saved && !force) { render(…，成立时执行对应逻辑。
    if (loadedCourse === context.courseId && saved && !force) { render(); return; }
    // 【逐行批注】 声明前端变量 courseId，保存本行计算出的数据或对象。
    const courseId = context.courseId, version = ++requestVersion;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch('/api/courses/' + encodeURIComponent(courseId), {cache: 'no-store'});
      // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
      const data = await response.json(); if (!response.ok) throw new Error(data.error || '课程目录读取失败');
      // 【逐行批注】 判断条件 (version !== requestVersion || context.courseId !== courseId) re…，成立时执行对应逻辑。
      if (version !== requestVersion || context.courseId !== courseId) return;
      // 【逐行批注】 计算右侧结果并更新 saved，改变页面或请求状态。
      saved = data; loadedCourse = courseId; render();
    // 【逐行批注】 执行当前前端表达式：} catch (error) {。
    } catch (error) {
      // 【逐行批注】 判断条件 (version !== requestVersion) return;，成立时执行对应逻辑。
      if (version !== requestVersion) return;
      // 【逐行批注】 计算右侧结果并更新 desktop.replaceChildren(element('p', error.m…，改变页面或请求状态。
      desktop.replaceChildren(element('p', error.message, 'course-directory-foot')); desktop.hidden = false;
      // 【逐行批注】 执行当前前端表达式：document.body.classList.add('gy-course-view');。
      document.body.classList.add('gy-course-view');
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('gangyi:navigation-context', event => load(event.detail));。
  document.addEventListener('gangyi:navigation-context', event => load(event.detail));
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('gangyi:lesson-context', event => load(event.detail));。
  document.addEventListener('gangyi:lesson-context', event => load(event.detail));
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('gangyi:lesson-finished', () => load(context, true));。
  document.addEventListener('gangyi:lesson-finished', () => load(context, true));
  // 【逐行批注】 调用 load 执行页面交互或数据处理：load(context);。
  load(context);
// 【逐行批注】 执行当前前端表达式：})();。
})();

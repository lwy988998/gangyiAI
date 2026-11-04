/* 同一知识点共享教学背景，三页分别呈现自己的真实内容、对话与草稿。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 声明前端变量 api，保存本行计算出的数据或对象。
  const api = window.GangyiAgent, query = new URLSearchParams(location.search), lessonId = query.get('lessonId');
  // 【逐行批注】 声明前端变量 reviewContext，保存本行计算出的数据或对象。
  const reviewContext = query.has('review') ? {day: Number(query.get('review')), reviewId: query.get('reviewId') || ''} : {};
  // 【逐行批注】 声明前端变量 lessonHref，保存本行计算出的数据或对象。
  const lessonHref = path => {
    // 【逐行批注】 声明前端变量 params，保存本行计算出的数据或对象。
    const params = new URLSearchParams({lessonId});
    // 【逐行批注】 按循环规则遍历或重复处理：for (const key of ['review', 'reviewId', 'day', 'lessonTaskId']) if (query…。
    for (const key of ['review', 'reviewId', 'day', 'lessonTaskId']) if (query.has(key)) params.set(key, query.get(key));
    // 【逐行批注】 结束当前函数并返回结果：return '/' + path + '?' + params;。
    return '/' + path + '?' + params;
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 定义函数 readDraft，封装可复用的前端行为。
  function readDraft(key, section) {
    // 【逐行批注】 声明前端变量 current，保存本行计算出的数据或对象。
    const current = api.storage.get(key), legacy = lessonId.startsWith('legacy-');
    // 【逐行批注】 声明前端变量 marker，保存本行计算出的数据或对象。
    const marker = `legacy-draft-migrated:${key}`;
    // 【逐行批注】 声明前端变量 hasCurrent，保存本行计算出的数据或对象。
    let hasCurrent = Boolean(current);
    // 【逐行批注】 计算右侧结果并更新 try { hasCurrent，改变页面或请求状态。
    try { hasCurrent = hasCurrent || localStorage.getItem(`gangyi-agent:${key}`) !== null; } catch (_) { /* 存储不可读时继续使用当前输入。 */ }
    // 【逐行批注】 判断条件 (hasCurrent) { if(legacy)api.storage.set(marker,'1'); return cur…，成立时执行对应逻辑。
    if (hasCurrent) { if(legacy)api.storage.set(marker,'1'); return current; }
    // 【逐行批注】 判断条件 (!legacy || api.storage.get(marker)) return '';，成立时执行对应逻辑。
    if (!legacy || api.storage.get(marker)) return '';
    // 【逐行批注】 声明前端变量 taskId，保存本行计算出的数据或对象。
    const taskId = query.get('lessonTaskId') || (lessonId.startsWith('legacy-prepared-') ? lessonId.slice('legacy-prepared-'.length) : '');
    // 【逐行批注】 声明前端变量 context，保存本行计算出的数据或对象。
    const context = {courseId: lesson.courseId, phaseIndex: lesson.phaseIndex, topicIndex: lesson.topicIndex,
      // 【逐行批注】 执行当前前端表达式：...(taskId ? {lessonTaskId: taskId} : {}), ...reviewContext};。
      ...(taskId ? {lessonTaskId: taskId} : {}), ...reviewContext};
    // 【逐行批注】 声明前端变量 identities，保存本行计算出的数据或对象。
    const identities = section ? [...new Set([section.questionId, section.legacyIndex].filter(value=>value!==undefined&&value!==null&&value!==''))] : [];
    // 【逐行批注】 声明前端变量 oldKeys，保存本行计算出的数据或对象。
    const oldKeys = section ? identities.map(identity=>'gy:question-draft:' + JSON.stringify([context.courseId, context.phaseIndex, context.topicIndex,
      // 【逐行批注】 执行当前前端表达式：taskId, context.reviewId || '', context.day ?? '', section.legacyKind, identity, section.…。
      taskId, context.reviewId || '', context.day ?? '', section.legacyKind, identity, section.version])) :
      // 【逐行批注】 执行当前前端表达式：['gy:lesson-chat-draft:' + JSON.stringify(context)];。
      ['gy:lesson-chat-draft:' + JSON.stringify(context)];
    // 【逐行批注】 计算右侧结果并更新 try { const saved，改变页面或请求状态。
    try { const saved = oldKeys.map(oldKey=>localStorage.getItem(oldKey)||'').find(Boolean)||''; if(saved)api.storage.set(key,saved);
      // 【逐行批注】 执行当前前端表达式：api.storage.set(marker,'1'); return saved; }。
      api.storage.set(marker,'1'); return saved; }
    // 【逐行批注】 执行当前前端表达式：catch (_) { return ''; }。
    catch (_) { return ''; }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 material，封装可复用的前端行为。
  function material(root, value) {
    // 【逐行批注】 判断条件 (Array.isArray(value)) { value.forEach(item => material(root, it…，成立时执行对应逻辑。
    if (Array.isArray(value)) { value.forEach(item => material(root, item)); return; }
    // 【逐行批注】 判断条件 (typeof value !== 'object' || value === null) { const node=docum…，成立时执行对应逻辑。
    if (typeof value !== 'object' || value === null) { const node=document.createElement('div'); api.richText(node, value ?? ''); root.append(node); return; }
    // 【逐行批注】 按循环规则遍历或重复处理：for (const key of ['title', 'text', 'body', 'latex', 'formula']) if (typeo…。
    for (const key of ['title', 'text', 'body', 'latex', 'formula']) if (typeof value[key] === 'string') {
      // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
      const node=document.createElement(key === 'title' ? 'h3' : 'div'); api.richText(node, ['latex', 'formula'].includes(key) ? `$$${value[key]}$$` : value[key]); root.append(node);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 判断条件 (typeof value.imageUrl === 'string' && /^https?:\/\//.test(value…，成立时执行对应逻辑。
    if (typeof value.imageUrl === 'string' && /^https?:\/\//.test(value.imageUrl)) {
      // 【逐行批注】 声明前端变量 image，保存本行计算出的数据或对象。
      const image=document.createElement('img'); image.src=value.imageUrl; image.alt=value.alt || '题目材料'; image.loading='lazy'; image.referrerPolicy='no-referrer'; image.style.maxWidth='100%'; root.append(image);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 声明前端变量 rows，保存本行计算出的数据或对象。
    const rows = value.rows || value.values || value.cells;
    // 【逐行批注】 判断条件 (Array.isArray(rows)) {，成立时执行对应逻辑。
    if (Array.isArray(rows)) {
      // 【逐行批注】 声明前端变量 table，保存本行计算出的数据或对象。
      const table=document.createElement('table'); table.className='agent-material-table';
      // 【逐行批注】 判断条件 (Array.isArray(value.headers)) { const row=document.createElemen…，成立时执行对应逻辑。
      if (Array.isArray(value.headers)) { const row=document.createElement('tr'); value.headers.forEach(text => { const cell=document.createElement('th'); cell.textContent=String(text); row.append(cell); }); table.append(row); }
      // 【逐行批注】 计算右侧结果并更新 rows.forEach(items，改变页面或请求状态。
      rows.forEach(items => { const row=document.createElement('tr'); (Array.isArray(items) ? items : [items]).forEach(text => { const cell=document.createElement('td'); api.richText(cell, text); row.append(cell); }); table.append(row); }); root.append(table);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 判断条件 (value.table) material(root, value.table);，成立时执行对应逻辑。
    if (value.table) material(root, value.table);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 $，保存本行计算出的数据或对象。
  const $ = id => document.getElementById(id), root = $('lesson-sections'), scroll = $('lesson-scroll'), input = $('lesson-input');
  // 【逐行批注】 声明前端变量 nodes，保存本行计算出的数据或对象。
  const nodes = new Map(), records = new Map();
  // 【逐行批注】 声明前端变量 lesson，保存本行计算出的数据或对象。
  let lesson, admissionError = '', selectedQuestion = '', watchingTask = '', watchingView = '', unsubscribe, watchEpoch = 0;
  // 【逐行批注】 声明前端变量 busy，保存本行计算出的数据或对象。
  let busy = false, sending = false, refreshing = false, refreshAgain = false, loaded = false, rendering = false, taskRevision = 0;
  // 【逐行批注】 声明前端变量 localFailureRetry，保存本行计算出的数据或对象。
  let localFailureRetry = null;
  // 【逐行批注】 声明前端变量 initialTeachingDeferred，保存本行计算出的数据或对象。
  let initialTeachingDeferred = false, startingInitialTeaching = false;
  // 【逐行批注】 声明前端变量 activeView，保存本行计算出的数据或对象。
  let activeView = document.querySelector('[data-lesson-view]').dataset.lessonView, initialPosition = true;
  // 【逐行批注】 声明前端变量 trace，保存本行计算出的数据或对象。
  const trace = window.GangyiTrace?.create($('classroom-agent-trace'), {
    // 【逐行批注】 执行当前前端表达式：variant:'inline', eyebrow:'AI 教学执行过程', collapseWhenDone:true, defaultCollapsed:true,。
    variant:'inline', eyebrow:'AI 教学执行过程', collapseWhenDone:true, defaultCollapsed:true,
    // 【逐行批注】 计算右侧结果并更新 onRetry:()，改变页面或请求状态。
    onRetry:()=>retryTraceTask(), onResume:()=>controlWatchingTask('resume'),
    // 【逐行批注】 计算右侧结果并更新 onPause:()，改变页面或请求状态。
    onPause:()=>controlWatchingTask('pause'), onCancel:()=>controlWatchingTask('cancel'),
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 课堂以阅读和输入区为先：轨迹只说明执行过程，宿主不参与伸缩，矮视口下进一步收紧高度。
  // 【逐行批注】 判断条件 (trace?.root) {，成立时执行对应逻辑。
  if (trace?.root) {
    // 【逐行批注】 计算右侧结果并更新 trace.root.style.maxHeight，改变页面或请求状态。
    trace.root.style.maxHeight = 'clamp(140px, 26vh, 240px)';
    // 【逐行批注】 计算右侧结果并更新 $('classroom-agent-trace').style.flex，改变页面或请求状态。
    $('classroom-agent-trace').style.flex = 'none';
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 failureTask，保存本行计算出的数据或对象。
  const failureTask = (purpose, message, startedAt) => {
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
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 提交请求本身失败时还没有服务端任务，构造同形失败任务让轨迹与页面状态一致。
  // 【逐行批注】 定义函数 feedLocalFailure，封装可复用的前端行为。
  function feedLocalFailure(purpose, message, startedAt, retry) {
    // 【逐行批注】 计算右侧结果并更新 localFailureRetry，改变页面或请求状态。
    localFailureRetry = typeof retry === 'function' ? retry : null;
    // 【逐行批注】 调用 failureTask 执行页面交互或数据处理：trace?.update(failureTask(purpose, message, startedAt));。
    trace?.update(failureTask(purpose, message, startedAt));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 retryTraceTask，封装可复用的前端行为。
  function retryTraceTask() {
    // 【逐行批注】 判断条件 (String(trace?.task?.id || '').startsWith('local-')) {，成立时执行对应逻辑。
    if (String(trace?.task?.id || '').startsWith('local-')) {
      // 【逐行批注】 声明前端变量 retry，保存本行计算出的数据或对象。
      const retry = localFailureRetry; localFailureRetry = null;
      // 【逐行批注】 判断条件 (retry) return retry();，成立时执行对应逻辑。
      if (retry) return retry();
      // 【逐行批注】 计算右侧结果并更新 $('lesson-task-status').textContent，改变页面或请求状态。
      $('lesson-task-status').textContent = '请求没有提交成功，请重新发送。';
      // 【逐行批注】 结束当前函数并返回结果：return;。
      return;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return controlWatchingTask('retry');。
    return controlWatchingTask('retry');
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 element，保存本行计算出的数据或对象。
  const element = (tag, text = '', className = '') => { const node = document.createElement(tag); node.textContent = text; node.className = className; return node; };
  // 【逐行批注】 声明前端变量 focus，保存本行计算出的数据或对象。
  const focus = () => lesson?.teachingFocus || {};
  // 【逐行批注】 声明前端变量 sectionFor，保存本行计算出的数据或对象。
  const sectionFor = id => lesson?.sections.find(section => section.id === id);
  // 【逐行批注】 声明前端变量 isQuestion，保存本行计算出的数据或对象。
  const isQuestion = section => section?.kind === 'question' && section.legacyKind !== 'example';
  // 【逐行批注】 声明前端变量 areaFor，保存本行计算出的数据或对象。
  const areaFor = section => section.view || (section.kind === 'summary' ? 'summary' : isQuestion(section) &&
    // 【逐行批注】 执行当前前端表达式：!['interaction', 'diagnostic'].includes(section.questionKind || section.legacyKind) ? 'pr…。
    !['interaction', 'diagnostic'].includes(section.questionKind || section.legacyKind) ? 'practice' : 'learn');
  // 【逐行批注】 声明前端变量 sectionNode，保存本行计算出的数据或对象。
  const sectionNode = id => nodes.get('section:' + id) || nodes.get('summary:' + id);
  // 【逐行批注】 定义函数 sectionHref，封装可复用的前端行为。
  function sectionHref(section) {
    // 【逐行批注】 声明前端变量 url，保存本行计算出的数据或对象。
    const url = new URL(section.href || '/' + areaFor(section), location.origin);
    // 【逐行批注】 执行当前前端表达式：url.searchParams.set('lessonId', lessonId); url.searchParams.set('sectionId', section.id);。
    url.searchParams.set('lessonId', lessonId); url.searchParams.set('sectionId', section.id);
    // 复习入口会固定打开练习页，讲解和小结的直达地址不能继承它。
    // 【逐行批注】 按循环规则遍历或重复处理：for (const key of ['day', 'lessonTaskId', ...(areaFor(section) === 'practi…。
    for (const key of ['day', 'lessonTaskId', ...(areaFor(section) === 'practice' ? ['review', 'reviewId'] : [])])
      // 【逐行批注】 判断条件 (query.has(key)) url.searchParams.set(key, query.get(key));，成立时执行对应逻辑。
      if (query.has(key)) url.searchParams.set(key, query.get(key));
    // 【逐行批注】 结束当前函数并返回结果：return url.pathname + url.search;。
    return url.pathname + url.search;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 sectionLink，封装可复用的前端行为。
  function sectionLink(section, text) {
    // 【逐行批注】 声明前端变量 link，保存本行计算出的数据或对象。
    const link = element('a', text, 'ma-link'); link.href = sectionHref(section); link.dataset.sectionLink = section.id;
    // 【逐行批注】 计算右侧结果并更新 link.onclick，改变页面或请求状态。
    link.onclick = event => {
      // 【逐行批注】 调用 saveDraft 执行页面交互或数据处理：saveDraft();。
      saveDraft();
      // 【逐行批注】 判断条件 (areaFor(section) !== activeView || event.button !== 0 || event.…，成立时执行对应逻辑。
      if (areaFor(section) !== activeView || event.button !== 0 || event.ctrlKey || event.metaKey || event.shiftKey || event.altKey) return;
      // 【逐行批注】 执行当前前端表达式：event.preventDefault();。
      event.preventDefault();
      // 【逐行批注】 判断条件 (isQuestion(section)) selectQuestion(section.id);，成立时执行对应逻辑。
      if (isQuestion(section)) selectQuestion(section.id);
      // 【逐行批注】 调用 jumpTo、sectionNode 执行页面交互或数据处理：query.set('sectionId', section.id); history.replaceState(null, '', li…。
      query.set('sectionId', section.id); history.replaceState(null, '', link.href); jumpTo(sectionNode(section.id));
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 结束当前函数并返回结果：return link;。
    return link;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 pageKey，保存本行计算出的数据或对象。
  const pageKey = prefix => prefix + ':' + lessonId + ':' + activeView;
  // 【逐行批注】 声明前端变量 draftKey，保存本行计算出的数据或对象。
  const draftKey = () => selectedQuestion ? 'draft:' + lessonId + ':' + selectedQuestion : pageKey('chat-draft');
  // 【逐行批注】 定义函数 saveDraft，封装可复用的前端行为。
  function saveDraft() {
    // 【逐行批注】 判断条件 (!loaded) return;，成立时执行对应逻辑。
    if (!loaded) return;
    // 同时保存输入目标，首次开课的 AI 焦点晚到时仍能恢复原草稿。
    // 【逐行批注】 调用 pageKey 执行页面交互或数据处理：api.storage.set(pageKey('composer'), selectedQuestion || 'chat');。
    api.storage.set(pageKey('composer'), selectedQuestion || 'chat');
    // 【逐行批注】 调用 draftKey 执行页面交互或数据处理：api.storage.set(draftKey(), input.value);。
    api.storage.set(draftKey(), input.value);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 selectQuestion，封装可复用的前端行为。
  function selectQuestion(id, focusInput = false) {
    // 【逐行批注】 判断条件 (id && (!isQuestion(sectionFor(id)) || areaFor(sectionFor(id)) !…，成立时执行对应逻辑。
    if (id && (!isQuestion(sectionFor(id)) || areaFor(sectionFor(id)) !== activeView)) return;
    // 【逐行批注】 计算右侧结果并更新 saveDraft(); selectedQuestion，改变页面或请求状态。
    saveDraft(); selectedQuestion = id || ''; api.storage.set(pageKey('composer'), selectedQuestion || 'chat');
    // 【逐行批注】 计算右侧结果并更新 input.value，改变页面或请求状态。
    input.value = readDraft(draftKey(), sectionFor(selectedQuestion)); resizeInput(); paintTarget();
    // 【逐行批注】 判断条件 (focusInput) input.focus({preventScroll:true});，成立时执行对应逻辑。
    if (focusInput) input.focus({preventScroll:true});
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 paintTarget，封装可复用的前端行为。
  function paintTarget() {
    // 【逐行批注】 计算右侧结果并更新 $('composer-target').textContent，改变页面或请求状态。
    $('composer-target').textContent = selectedQuestion ? '正在回答：' + sectionFor(selectedQuestion).title : '和 AI 老师交流';
    // 【逐行批注】 计算右侧结果并更新 $('composer-chat').hidden，改变页面或请求状态。
    $('composer-chat').hidden = !selectedQuestion;
    // 【逐行批注】 按循环规则遍历或重复处理：for (const card of scroll.querySelectorAll('[data-section-id]')) card.clas…。
    for (const card of scroll.querySelectorAll('[data-section-id]')) card.classList.toggle('is-current-question', card.dataset.sectionId === selectedQuestion);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 resizeInput，封装可复用的前端行为。
  function resizeInput() { const follow = nearBottom(); input.style.height = 'auto'; input.style.height = Math.min(input.scrollHeight, 136) + 'px';
    // 【逐行批注】 判断条件 (follow && !initialPosition) requestAnimationFrame(() => { scrol…，成立时执行对应逻辑。
    if (follow && !initialPosition) requestAnimationFrame(() => { scroll.scrollTop = scroll.scrollHeight; }); }
  // 【逐行批注】 声明前端变量 nearBottom，保存本行计算出的数据或对象。
  const nearBottom = () => scroll.scrollHeight - scroll.clientHeight - scroll.scrollTop < 72;
  // 【逐行批注】 定义函数 readingPosition，封装可复用的前端行为。
  function readingPosition() {
    // 【逐行批注】 声明前端变量 boundary，保存本行计算出的数据或对象。
    const boundary = scroll.getBoundingClientRect().top;
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = [...root.children].find(node => node.getBoundingClientRect().bottom > boundary + 8);
    // 【逐行批注】 结束当前函数并返回结果：return {key:node?.dataset.entryKey || '', offset:node ? node.getBound…。
    return {key:node?.dataset.entryKey || '', offset:node ? node.getBoundingClientRect().top - boundary : 0, top:scroll.scrollTop, bottom:nearBottom()};
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 restorePosition，封装可复用的前端行为。
  function restorePosition(position) {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = nodes.get(position.key);
    // 【逐行批注】 判断条件 (position.bottom) scroll.scrollTop = scroll.scrollHeight;，成立时执行对应逻辑。
    if (position.bottom) scroll.scrollTop = scroll.scrollHeight;
    // 【逐行批注】 当前一条件不成立时继续判断：(node?.isConnected) scroll.scrollTop += node.getBoundingClientRe…。
    else if (node?.isConnected) scroll.scrollTop += node.getBoundingClientRect().top - scroll.getBoundingClientRect().top - position.offset;
    // 【逐行批注】 执行前一条件不成立时的分支。
    else scroll.scrollTop = position.top || 0;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 mutate，封装可复用的前端行为。
  function mutate(callback) {
    // 【逐行批注】 声明前端变量 position，保存本行计算出的数据或对象。
    const position = readingPosition(); rendering = true; callback(); restorePosition(position); rendering = false;
    // 【逐行批注】 计算右侧结果并更新 $('lesson-latest').hidden，改变页面或请求状态。
    $('lesson-latest').hidden = nearBottom(); requestAnimationFrame(exposeVisible);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 jumpTo，封装可复用的前端行为。
  function jumpTo(node) {
    // 【逐行批注】 判断条件 (!node) return;，成立时执行对应逻辑。
    if (!node) return;
    // 【逐行批注】 按循环规则遍历或重复处理：for (let parent = node.parentElement; parent && parent !== scroll; parent …。
    for (let parent = node.parentElement; parent && parent !== scroll; parent = parent.parentElement) if (parent.tagName === 'DETAILS') parent.open = true;
    // 【逐行批注】 计算右侧结果并更新 scroll.scrollTop，改变页面或请求状态。
    scroll.scrollTop = Math.max(0, node.getBoundingClientRect().top - scroll.getBoundingClientRect().top + scroll.scrollTop - 12);
    // 【逐行批注】 调用 requestAnimationFrame 执行页面交互或数据处理：requestAnimationFrame(exposeVisible);。
    requestAnimationFrame(exposeVisible);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 locate，封装可复用的前端行为。
  function locate(view, updateUrl = true) {
    // 【逐行批注】 判断条件 (updateUrl) { saveDraft(); location.href = lessonHref(view); ret…，成立时执行对应逻辑。
    if (updateUrl) { saveDraft(); location.href = lessonHref(view); return; }
    // 【逐行批注】 声明前端变量 identity，保存本行计算出的数据或对象。
    const identity = view === 'learn' ? focus().sectionId : view === 'practice' ?
      // 【逐行批注】 调用 focus 执行页面交互或数据处理：(focus().responseView === 'practice' ? focus().responseSectionId : fo…。
      (focus().responseView === 'practice' ? focus().responseSectionId : focus().practiceSectionId) : '';
    // 【逐行批注】 声明前端变量 target，保存本行计算出的数据或对象。
    let target = [...scroll.querySelectorAll('[data-section-id]')].find(node => node.dataset.sectionId === identity);
    // 【逐行批注】 计算右侧结果并更新 target ||，改变页面或请求状态。
    target ||= [...scroll.querySelectorAll('[data-area]')].find(node => node.dataset.area === view);
    // 【逐行批注】 判断条件 (view === 'summary') { $('lesson-summary').open = true; target =…，成立时执行对应逻辑。
    if (view === 'summary') { $('lesson-summary').open = true; target = $('lesson-completion'); }
    // 【逐行批注】 判断条件 (target) jumpTo(target);，成立时执行对应逻辑。
    if (target) jumpTo(target);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 questionCard，封装可复用的前端行为。
  function questionCard(section, card) {
    // 【逐行批注】 声明前端变量 question，保存本行计算出的数据或对象。
    const question = element('div', '', 'agent-question'); api.richText(question, section.question.question); card.append(question);
    // 【逐行批注】 判断条件 (section.question.materials) { const materials = element('div', …，成立时执行对应逻辑。
    if (section.question.materials) { const materials = element('div', '', 'agent-materials'); material(materials, section.question.materials); card.append(materials); }
    // 【逐行批注】 声明前端变量 form，保存本行计算出的数据或对象。
    const form = element('form'), options = element('div', '', 'agent-options'); form.dataset.section = section.id;
    // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
    const key = 'draft:' + lessonId + ':' + section.id + ':choice';
    // 【逐行批注】 声明前端变量 selected，保存本行计算出的数据或对象。
    let selected; try { selected = JSON.parse(api.storage.get(key) || 'null'); } catch (_) { selected = api.storage.get(key); }
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [index, text] of (section.question.options || []).entries()) {。
    for (const [index, text] of (section.question.options || []).entries()) {
      // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
      const label = element('label'), choice = element('input'); choice.type = section.question.type === 'multi_choice' ? 'checkbox' : 'radio';
      // 【逐行批注】 计算右侧结果并更新 choice.name，改变页面或请求状态。
      choice.name = 'choice-' + section.id; choice.value = String(index); choice.checked = Array.isArray(selected) ? selected.includes(choice.value) : selected === choice.value;
      // 【逐行批注】 调用 element 执行页面交互或数据处理：label.append(choice, element('span', String.fromCharCode(65 + index) …。
      label.append(choice, element('span', String.fromCharCode(65 + index) + '. ' + text)); options.append(label);
      // 【逐行批注】 为页面事件绑定处理逻辑：choice.addEventListener('change', () => { api.storage.set(key, JSON.stringify([...op…。
      choice.addEventListener('change', () => { api.storage.set(key, JSON.stringify([...options.querySelectorAll('input:checked')].map(node => node.value)));
        // 【逐行批注】 判断条件 (selectedQuestion !== section.id) selectQuestion(section.id); });，成立时执行对应逻辑。
        if (selectedQuestion !== section.id) selectQuestion(section.id); });
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 按循环规则遍历或重复处理：form.append(options);。
    form.append(options);
    // 【逐行批注】 声明前端变量 toolbar，保存本行计算出的数据或对象。
    const toolbar = element('div', '', 'question-actions'), submit = element('button', options.children.length ? '提交答案' : '在下方回答');
    // 【逐行批注】 计算右侧结果并更新 submit.type，改变页面或请求状态。
    submit.type = options.children.length ? 'submit' : 'button';
    // 【逐行批注】 判断条件 (!options.children.length) submit.onclick = () => selectQuestion…，成立时执行对应逻辑。
    if (!options.children.length) submit.onclick = () => selectQuestion(section.id, true);
    // 【逐行批注】 声明前端变量 unknown，保存本行计算出的数据或对象。
    const unknown = element('button', '暂时不会', 'secondary'), skip = element('button', '跳过', 'secondary'); unknown.type = skip.type = 'button';
    // 【逐行批注】 计算右侧结果并更新 unknown.onclick，改变页面或请求状态。
    unknown.onclick = () => sendAnswer(section.id, 'unknown'); skip.onclick = () => sendAnswer(section.id, 'skip');
    // 【逐行批注】 声明前端变量 note，保存本行计算出的数据或对象。
    const note = element('p', '', 'question-note'); note.setAttribute('role', 'status');
    // 【逐行批注】 执行当前前端表达式：toolbar.append(submit, unknown, skip); form.append(toolbar, note); card.append(form);。
    toolbar.append(submit, unknown, skip); form.append(toolbar, note); card.append(form);
    // 【逐行批注】 按循环规则遍历或重复处理：form.onsubmit = event => { event.preventDefault(); sendAnswer(section.id, …。
    form.onsubmit = event => { event.preventDefault(); sendAnswer(section.id, 'answer'); };
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 makeSection，封装可复用的前端行为。
  function makeSection(section) {
    // 【逐行批注】 声明前端变量 card，保存本行计算出的数据或对象。
    const card = element('article', '', 'chat-section'); card.dataset.sectionId = section.id; card.dataset.sectionVersion = section.version; card.dataset.area = areaFor(section);
    // 【逐行批注】 调用 element、isQuestion、areaFor 执行页面交互或数据处理：card.append(element('p', section.kind === 'summary' ? 'AI · 小结' : isQ…。
    card.append(element('p', section.kind === 'summary' ? 'AI · 小结' : isQuestion(section) ? 'AI · ' + (areaFor(section) === 'practice' ? '练习' : '课堂提问') :
      // 【逐行批注】 调用 element 执行页面交互或数据处理：section.presentation === 'example' || section.legacyKind === 'example…。
      section.presentation === 'example' || section.legacyKind === 'example' ? 'AI · 示范例题' : 'AI · 讲解', 'chat-author'), element('h2', section.title));
    // 【逐行批注】 判断条件 (isQuestion(section)) questionCard(section, card);，成立时执行对应逻辑。
    if (isQuestion(section)) questionCard(section, card);
    // 【逐行批注】 执行前一条件不成立时的分支。
    else { const body = element('div', '', 'lesson-body'); api.richText(body, section.body || ''); card.append(body); }
    // 【逐行批注】 调用 element、isQuestion 执行页面交互或数据处理：if(section.materialOrigin)card.append(element('p',isQuestion(section)…。
    if(section.materialOrigin)card.append(element('p',isQuestion(section)?'AI编写或改编的练习':section.materialOrigin==='source_based'?'依据资料讲解':'AI补充讲解','library-origin'));
    // 【逐行批注】 计算右侧结果并更新 if(section.citations?.length){const details，改变页面或请求状态。
    if(section.citations?.length){const details=element('details','','library-citations');details.append(element('summary','查看本段资料依据（'+section.citations.length+'）'));
      // 【逐行批注】 按循环规则遍历或重复处理：for(const citation of section.citations){const entry=element('p');const li…。
      for(const citation of section.citations){const entry=element('p');const link=element('a',citation.title||'查看原文依据');link.href=citation.href;entry.append(link);
        // 【逐行批注】 调用 element 执行页面交互或数据处理：entry.append(element('span',citation.locator?.page?' · 第'+citation.lo…。
        entry.append(element('span',citation.locator?.page?' · 第'+citation.locator.page+'页':citation.locator?.start!=null?' · '+citation.locator.start+'秒':' · '+citation.segmentId));
        // 【逐行批注】 调用 element 执行页面交互或数据处理：if(citation.quote)entry.append(element('p',citation.quote));details.a…。
        if(citation.quote)entry.append(element('p',citation.quote));details.append(entry);}card.append(details);}
    // 【逐行批注】 调用 sectionLink 执行页面交互或数据处理：card.append(sectionLink(section, '直达此段'));。
    card.append(sectionLink(section, '直达此段'));
    // 【逐行批注】 结束当前函数并返回结果：return card;。
    return card;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 makeMessage，封装可复用的前端行为。
  function makeMessage(record) {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = element('article', '', 'chat-message ' + record.role);
    // 【逐行批注】 调用 element 执行页面交互或数据处理：node.append(element('p', record.role === 'user' ? '你' : 'AI 老师', 'cha…。
    node.append(element('p', record.role === 'user' ? '你' : 'AI 老师', 'chat-author'), element('div', '', 'chat-message-body'), element('p', '', 'chat-partial'));
    // 【逐行批注】 结束当前函数并返回结果：return node;。
    return node;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 updateMessage，封装可复用的前端行为。
  function updateMessage(node, record) {
    // 【逐行批注】 判断条件 (node.dataset.rawText !== record.text) { node.dataset.rawText = …，成立时执行对应逻辑。
    if (node.dataset.rawText !== record.text) { node.dataset.rawText = record.text;
      // 【逐行批注】 判断条件 (record.role === 'assistant') api.richText(node.querySelector('.…，成立时执行对应逻辑。
      if (record.role === 'assistant') api.richText(node.querySelector('.chat-message-body'), record.text);
      // 【逐行批注】 执行前一条件不成立时的分支。
      else node.querySelector('.chat-message-body').textContent = record.text; }
    // 【逐行批注】 声明前端变量 note，保存本行计算出的数据或对象。
    const note = node.querySelector('.chat-partial'); note.hidden = !record.partial || busy && record.taskId === watchingTask;
    // 【逐行批注】 计算右侧结果并更新 note.textContent，改变页面或请求状态。
    note.textContent = '这段回复未完整完成，已接收内容保留，不作为新的可靠评价。';
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 makeEvaluation，封装可复用的前端行为。
  function makeEvaluation(record) {
    // 【逐行批注】 声明前端变量 card，保存本行计算出的数据或对象。
    const card = element('article', '', 'chat-evaluation'); card.dataset.area = record.view || activeView;
    // 【逐行批注】 调用 element 执行页面交互或数据处理：card.append(element('p', 'AI · 本次作答评价', 'chat-author'), element('h2',…。
    card.append(element('p', 'AI · 本次作答评价', 'chat-author'), element('h2', record.credible && record.score !== null && record.score !== undefined ?
      // 【逐行批注】 执行当前前端表达式：'本次作答 · ' + record.score + ' 分' : '本次反馈 · 尚无可靠分数'));。
      '本次作答 · ' + record.score + ' 分' : '本次反馈 · 尚无可靠分数'));
    // 【逐行批注】 声明前端变量 feedbackContent，保存本行计算出的数据或对象。
    const feedbackContent = element('div'); feedbackContent.dataset.evaluationFeedback = record.id;
    // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
    const body = element('div'); api.richText(body, record.feedback || ''); feedbackContent.append(body); card.append(feedbackContent);
    // 【逐行批注】 判断条件 (record.feedbackHelpType && record.feedbackHelpType !== 'none') …，成立时执行对应逻辑。
    if (record.feedbackHelpType && record.feedbackHelpType !== 'none') card.append(element('p', {
      // 【逐行批注】 执行当前前端表达式：explanation:'反馈包含相关概念讲解',hint:'反馈包含本题提示',worked_example:'反馈包含本题完整示范',。
      explanation:'反馈包含相关概念讲解',hint:'反馈包含本题提示',worked_example:'反馈包含本题完整示范',
      // 【逐行批注】 执行当前前端表达式：clarification:'反馈用于澄清思路',unknown:'反馈帮助程度未确定'}[record.feedbackHelpType] || '', 'agent-note…。
      clarification:'反馈用于澄清思路',unknown:'反馈帮助程度未确定'}[record.feedbackHelpType] || '', 'agent-note'));
    // 【逐行批注】 判断条件 (record.uncertainty) feedbackContent.append(element('p', record.…，成立时执行对应逻辑。
    if (record.uncertainty) feedbackContent.append(element('p', record.uncertainty, 'agent-note'));
    // 【逐行批注】 判断条件 (record.diagnosis?.summary) feedbackContent.append(element('p', …，成立时执行对应逻辑。
    if (record.diagnosis?.summary) feedbackContent.append(element('p', '卡点：' + record.diagnosis.summary, 'agent-note'));
    // 【逐行批注】 判断条件 (record.diagnosis?.evidence?.length) feedbackContent.append(elem…，成立时执行对应逻辑。
    if (record.diagnosis?.evidence?.length) feedbackContent.append(element('p', '依据：' + record.diagnosis.evidence.join('；'), 'agent-note'));
    // 【逐行批注】 判断条件 (record.helpMode) card.append(element('p', {independent:'本题独立作答'…，成立时执行对应逻辑。
    if (record.helpMode) card.append(element('p', {independent:'本题独立作答',after_hint:'本题在提示后作答',after_example:'本题在完整示范后作答',help_unknown:'本题帮助程度未确定'}[record.helpMode] || '', 'agent-note'));
    // 【逐行批注】 判断条件 (record.rePracticeOf) card.append(element('p', '这是关联原问题的新题复练结果。'…，成立时执行对应逻辑。
    if (record.rePracticeOf) card.append(element('p', '这是关联原问题的新题复练结果。', 'agent-note'));
    // 【逐行批注】 结束当前函数并返回结果：return card;。
    return card;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 putRecord，封装可复用的前端行为。
  function putRecord(key, record) {
    // 【逐行批注】 计算右侧结果并更新 records.set(key, record); let node，改变页面或请求状态。
    records.set(key, record); let node = nodes.get(key);
    // 【逐行批注】 判断条件 (!node) { node = record.type === 'section' ? makeSection(record.…，成立时执行对应逻辑。
    if (!node) { node = record.type === 'section' ? makeSection(record.section) : record.type === 'evaluation' ? makeEvaluation(record) : makeMessage(record);
      // 【逐行批注】 计算右侧结果并更新 node.dataset.entryKey，改变页面或请求状态。
      node.dataset.entryKey = key; nodes.set(key, node); }
    // 【逐行批注】 判断条件 (record.type === 'message') updateMessage(node, record);，成立时执行对应逻辑。
    if (record.type === 'message') updateMessage(node, record);
    // 【逐行批注】 结束当前函数并返回结果：return node;。
    return node;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 render，封装可复用的前端行为。
  function render() {
    // 【逐行批注】 计算右侧结果并更新 mutate(()，改变页面或请求状态。
    mutate(() => {
      // 【逐行批注】 计算右侧结果并更新 $('lesson-title').textContent，改变页面或请求状态。
      $('lesson-title').textContent = lesson.title; $('lesson-purpose').textContent = lesson.purpose;
      // 【逐行批注】 声明前端变量 order，保存本行计算出的数据或对象。
      const order = [], used = new Set(), timeline = lesson.timeline || [];
      // 【逐行批注】 声明前端变量 referenced，保存本行计算出的数据或对象。
      const referenced = new Set(timeline.filter(item => item.type === 'section').map(item => item.sectionId));
      // 【逐行批注】 声明前端变量 archive，保存本行计算出的数据或对象。
      let archive = nodes.get('saved-materials');
      // 【逐行批注】 判断条件 (!archive) { archive = element('details', '', 'saved-materials')…，成立时执行对应逻辑。
      if (!archive) { archive = element('details', '', 'saved-materials'); archive.dataset.entryKey = 'saved-materials';
        // 【逐行批注】 调用 element 执行页面交互或数据处理：archive.append(element('summary', '已保存课件 · 随时回看')); nodes.set('saved-…。
        archive.append(element('summary', '已保存课件 · 随时回看')); nodes.set('saved-materials', archive); }
      // 【逐行批注】 声明前端变量 savedSections，保存本行计算出的数据或对象。
      const savedSections = lesson.sections.filter(section => areaFor(section) === activeView && !referenced.has(section.id) && section.kind !== 'summary');
      // 【逐行批注】 按循环规则遍历或重复处理：for (const section of savedSections) {。
      for (const section of savedSections) {
        // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
        const key = 'section:' + section.id;
        // 【逐行批注】 判断条件 (nodes.has(key) && nodes.get(key).dataset.sectionVersion !== Str…，成立时执行对应逻辑。
        if (nodes.has(key) && nodes.get(key).dataset.sectionVersion !== String(section.version)) { nodes.get(key).remove(); nodes.delete(key); }
        // 【逐行批注】 调用 putRecord 执行页面交互或数据处理：archive.append(putRecord(key, {type:'section', section})); used.add(k…。
        archive.append(putRecord(key, {type:'section', section})); used.add(key);
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 计算右侧结果并更新 archive.hidden，改变页面或请求状态。
      archive.hidden = !savedSections.length; order.push(archive);
      // 旧交流按原记录保留；相同文字可能来自不同轮次，不能按正文去重。
      // 【逐行批注】 按循环规则遍历或重复处理：for (const section of lesson.sections.filter(section => areaFor(section) =…。
      for (const section of lesson.sections.filter(section => areaFor(section) === activeView)) for (const [index, item] of (section.legacyDialog || []).entries()) {
        // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
        const key = 'legacy:' + section.id + ':' + index; order.push(putRecord(key, {...item, type:'message'})); used.add(key);
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 按循环规则遍历或重复处理：for (const item of timeline) {。
      for (const item of timeline) {
        // 【逐行批注】 判断条件 (item.type === 'section') {，成立时执行对应逻辑。
        if (item.type === 'section') {
          // 【逐行批注】 声明前端变量 section，保存本行计算出的数据或对象。
          const section = sectionFor(item.sectionId); if (!section || areaFor(section) !== activeView) continue;
          // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
          const key = 'section:' + section.id; if (used.has(key)) continue;
          // 【逐行批注】 判断条件 (nodes.has(key) && nodes.get(key).dataset.sectionVersion !== Str…，成立时执行对应逻辑。
          if (nodes.has(key) && nodes.get(key).dataset.sectionVersion !== String(section.version)) { nodes.get(key).remove(); nodes.delete(key); }
          // 【逐行批注】 调用 putRecord 执行页面交互或数据处理：order.push(putRecord(key, {type:'section', section})); used.add(key);。
          order.push(putRecord(key, {type:'section', section})); used.add(key);
        // 【逐行批注】 执行当前前端表达式：} else if (item.type === 'evaluation') {。
        } else if (item.type === 'evaluation') {
          // 【逐行批注】 判断条件 ((item.view || 'learn') !== activeView) continue;，成立时执行对应逻辑。
          if ((item.view || 'learn') !== activeView) continue;
          // 【逐行批注】 声明前端变量 evaluation，保存本行计算出的数据或对象。
          const evaluation = lesson.evaluations.find(value => value.id === item.id); if (!evaluation) continue;
          // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
          const key = 'evaluation:' + item.id; order.push(putRecord(key, {...evaluation, type:'evaluation'})); used.add(key);
        // 【逐行批注】 执行当前前端表达式：} else {。
        } else {
          // 【逐行批注】 判断条件 ((item.view || 'learn') !== activeView) continue;，成立时执行对应逻辑。
          if ((item.view || 'learn') !== activeView) continue;
          // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
          const key = item.role === 'user' ? item.taskId + ':user' : item.id;
          // 【逐行批注】 调用 putRecord 执行页面交互或数据处理：order.push(putRecord(key, item)); used.add(key);。
          order.push(putRecord(key, item)); used.add(key);
        // 【逐行批注】 结束当前前端代码块。
        }
      // 【逐行批注】 结束当前前端代码块。
      }
      // 流式文字和刚提交的输入在服务端刷新前保留；同一任务、同一步骤始终复用一个消息节点。
      // 【逐行批注】 按循环规则遍历或重复处理：for (const [key, record] of records) if (record.optimistic && (record.view…。
      for (const [key, record] of records) if (record.optimistic && (record.view || activeView) === activeView && !used.has(key)) {
        // 【逐行批注】 判断条件 (key.endsWith(':fallback') && timeline.some(item => item.type ==…，成立时执行对应逻辑。
        if (key.endsWith(':fallback') && timeline.some(item => item.type === 'message' && item.role === 'assistant' && item.taskId === record.taskId)) continue;
        // 【逐行批注】 执行当前前端表达式：order.push(nodes.get(key)); used.add(key);。
        order.push(nodes.get(key)); used.add(key);
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 按循环规则遍历或重复处理：for (const evaluation of lesson.evaluations || []) { const key = 'evaluati…。
      for (const evaluation of lesson.evaluations || []) { const key = 'evaluation:' + evaluation.id;
        // 【逐行批注】 判断条件 (timeline.some(item => item.type === 'evaluation' && item.id ===…，成立时执行对应逻辑。
        if (timeline.some(item => item.type === 'evaluation' && item.id === evaluation.id)) continue;
        // 【逐行批注】 声明前端变量 section，保存本行计算出的数据或对象。
        const section = sectionFor(evaluation.sectionId); if (section && areaFor(section) !== activeView) continue;
        // 【逐行批注】 判断条件 (!used.has(key)) { order.push(putRecord(key, {...evaluation, typ…，成立时执行对应逻辑。
        if (!used.has(key)) { order.push(putRecord(key, {...evaluation, type:'evaluation'})); used.add(key); } }
      // 【逐行批注】 声明前端变量 cursor，保存本行计算出的数据或对象。
      let cursor = root.firstChild;
      // 【逐行批注】 按循环规则遍历或重复处理：for (const node of order) { if (node !== cursor) root.insertBefore(node, c…。
      for (const node of order) { if (node !== cursor) root.insertBefore(node, cursor); cursor = node.nextSibling; }
      // 【逐行批注】 按循环规则遍历或重复处理：for (const [key, node] of nodes) if (key !== 'saved-materials' && !used.ha…。
      for (const [key, node] of nodes) if (key !== 'saved-materials' && !used.has(key) && !key.startsWith('summary:')) { node.remove(); nodes.delete(key); records.delete(key); }
      // 【逐行批注】 计算右侧结果并更新 $('lesson-completion').hidden，改变页面或请求状态。
      $('lesson-completion').hidden = activeView !== 'summary';
      // 【逐行批注】 判断条件 (activeView === 'summary') { $('lesson-summary').open = true; re…，成立时执行对应逻辑。
      if (activeView === 'summary') { $('lesson-summary').open = true; renderSummary(); }
      // 【逐行批注】 调用 paintNextStep、paintTarget、setDisabled 执行页面交互或数据处理：paintNextStep(); paintTarget(); setDisabled();。
      paintNextStep(); paintTarget(); setDisabled();
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 直接进入练习/小结时可能没有讲解焦点，也必须完成定位并开始累计滚动展示。
    // 【逐行批注】 声明前端变量 requestedSection，保存本行计算出的数据或对象。
    const requestedSection = sectionFor(query.get('sectionId'));
    // 【逐行批注】 判断条件 (initialPosition && (requestedSection || activeView !== 'learn' …，成立时执行对应逻辑。
    if (initialPosition && (requestedSection || activeView !== 'learn' || focus().sectionId || admissionError || !lesson.teachingTaskId)) {
      // 【逐行批注】 计算右侧结果并更新 initialPosition，改变页面或请求状态。
      initialPosition = false; let saved;
      // 【逐行批注】 计算右侧结果并更新 try { saved，改变页面或请求状态。
      try { saved = JSON.parse(api.storage.get(pageKey('scroll')) || 'null'); } catch (_) { /* 默认定位当前课堂。 */ }
      // 显式直达优先于上次滚动位置，也能打开归档内的非题目补讲卡片。
      // 【逐行批注】 判断条件 (requestedSection && areaFor(requestedSection) === activeView) j…，成立时执行对应逻辑。
      if (requestedSection && areaFor(requestedSection) === activeView) jumpTo(sectionNode(requestedSection.id));
      // 【逐行批注】 当前一条件不成立时继续判断：(saved) restorePosition(saved); else locate(activeView, false);。
      else if (saved) restorePosition(saved); else locate(activeView, false);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 判断条件 (lesson.teachingTaskId && !admissionError && lesson.teachingTask…，成立时执行对应逻辑。
    if (lesson.teachingTaskId && !admissionError && lesson.teachingTaskId !== watchingTask) observe(lesson.teachingTaskId);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 paintNextStep，封装可复用的前端行为。
  function paintNextStep() {
    // 【逐行批注】 声明前端变量 prompt，保存本行计算出的数据或对象。
    const prompt = $('lesson-next-step'), response = focus().responseTaskId === lesson.teachingTaskId ? sectionFor(focus().responseSectionId) : null;
    // 【逐行批注】 声明前端变量 latest，保存本行计算出的数据或对象。
    const latest = (lesson.timeline || []).filter(item => item.taskId === lesson.teachingTaskId && item.type === 'section').at(-1);
    // 【逐行批注】 声明前端变量 section，保存本行计算出的数据或对象。
    const section = response || (latest && sectionFor(latest.sectionId));
    // 【逐行批注】 声明前端变量 target，保存本行计算出的数据或对象。
    const target = section ? areaFor(section) : lesson.teachingTaskView;
    // 【逐行批注】 计算右侧结果并更新 prompt.replaceChildren(); prompt.hidden，改变页面或请求状态。
    prompt.replaceChildren(); prompt.hidden = !target || !section && target === activeView;
    // 【逐行批注】 判断条件 (prompt.hidden) return;，成立时执行对应逻辑。
    if (prompt.hidden) return;
    // 【逐行批注】 调用 element 执行页面交互或数据处理：prompt.append(element('span', 'AI 已保存下一步内容。'));。
    prompt.append(element('span', 'AI 已保存下一步内容。'));
    // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
    const text = '查看' + {learn:'讲解', practice:'练习', summary:'小结'}[target] + '原卡片 →';
    // 【逐行批注】 判断条件 (section) prompt.append(sectionLink(section, text));，成立时执行对应逻辑。
    if (section) prompt.append(sectionLink(section, text));
    // 【逐行批注】 执行前一条件不成立时的分支。
    else { const link = element('a', '进入' + {learn:'讲解', practice:'练习', summary:'小结'}[target] + ' →'); link.href = lessonHref(target); prompt.append(link); }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 renderSummary，封装可复用的前端行为。
  function renderSummary() {
    // 【逐行批注】 声明前端变量 parent，保存本行计算出的数据或对象。
    const parent = $('lesson-summary-content');
    // 【逐行批注】 按循环规则遍历或重复处理：for (const section of lesson.sections.filter(section => section.kind === '…。
    for (const section of lesson.sections.filter(section => section.kind === 'summary' && nodes.get('section:' + section.id)?.parentElement !== root)) {
      // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
      const key = 'summary:' + section.id;
      // 【逐行批注】 判断条件 (nodes.has(key) && nodes.get(key).dataset.sectionVersion !== Str…，成立时执行对应逻辑。
      if (nodes.has(key) && nodes.get(key).dataset.sectionVersion !== String(section.version)) { nodes.get(key).remove(); nodes.delete(key); }
      // 【逐行批注】 判断条件 (!nodes.has(key)) { const node = makeSection(section); node.data…，成立时执行对应逻辑。
      if (!nodes.has(key)) { const node = makeSection(section); node.dataset.entryKey = key; nodes.set(key, node); } parent.append(nodes.get(key));
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [key, node] of nodes) if (key.startsWith('summary:') && nodes.g…。
    for (const [key, node] of nodes) if (key.startsWith('summary:') && nodes.get('section:' + key.slice(8))?.parentElement === root) { node.remove(); nodes.delete(key); }
    // 【逐行批注】 判断条件 (!parent.dataset.references) {，成立时执行对应逻辑。
    if (!parent.dataset.references) {
      // 【逐行批注】 计算右侧结果并更新 parent.dataset.references，改变页面或请求状态。
      parent.dataset.references = '1';
      // 【逐行批注】 按循环规则遍历或重复处理：for (const reference of lesson.references || []) { const card = element('a…。
      for (const reference of lesson.references || []) { const card = element('article', '', 'chat-reference');
        // 【逐行批注】 调用 element 执行页面交互或数据处理：card.append(element('h3', reference.title || reference.name || refere…。
        card.append(element('h3', reference.title || reference.name || reference.source || '参考资料'), element('p', reference.description || ''));
        // 【逐行批注】 声明前端变量 href，保存本行计算出的数据或对象。
        const href = reference.url || reference.href; if (/^https?:\/\//i.test(href || '')) { const link = element('a', '查看资料 ↗'); link.href = href; link.target = '_blank'; link.rel = 'noreferrer'; card.append(link); } parent.append(card); }
      // 【逐行批注】 按循环规则遍历或重复处理：for (const record of lesson.historicalAssessments || []) { const card = el…。
      for (const record of lesson.historicalAssessments || []) { const card = element('article', '', 'chat-evaluation'); card.append(element('h3', record.kind === 'quiz' ? '历史测验记录' : '历史作答记录'));
        // 【逐行批注】 判断条件 (record.score !== undefined && record.score !== null) card.appen…，成立时执行对应逻辑。
        if (record.score !== undefined && record.score !== null) card.append(element('p', '原保存分数：' + record.score + (record.total !== undefined ? ' / ' + record.total : '')));
        // 【逐行批注】 判断条件 (record.response) card.append(element('p', record.response)); if…，成立时执行对应逻辑。
        if (record.response) card.append(element('p', record.response)); if (record.feedback) { const body = element('div'); api.richText(body, record.feedback); card.append(body); }
        // 【逐行批注】 按循环规则遍历或重复处理：for (const [index, answer] of (record.answers || []).entries()) card.appen…。
        for (const [index, answer] of (record.answers || []).entries()) card.append(element('p', '第 ' + (index + 1) + ' 题原提交：' + (answer === null ? '未作答' : answer === 'unknown' ? '暂时不会' : typeof answer === 'number' ? '选项 ' + String.fromCharCode(65 + answer) : String(answer))));
        // 【逐行批注】 执行当前前端表达式：parent.append(card); }。
        parent.append(card); }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 $('completion-note').textContent，改变页面或请求状态。
    $('completion-note').textContent = lesson.completed ? 'AI 已根据实际记录确认本节完成，原作答和交流继续保留。' :
      // 【逐行批注】 执行当前前端表达式：(lesson.evaluations || []).length ? '完成情况由 AI 结合实际作答和评价判断。' : '本节尚无足够的实际作答评价；阅读和追问不表示掌握。';。
      (lesson.evaluations || []).length ? '完成情况由 AI 结合实际作答和评价判断。' : '本节尚无足够的实际作答评价；阅读和追问不表示掌握。';
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 exposeVisible，封装可复用的前端行为。
  function exposeVisible() {
    // 【逐行批注】 判断条件 (!lesson || admissionError) return;，成立时执行对应逻辑。
    if (!lesson || admissionError) return;
    // 【逐行批注】 按循环规则遍历或重复处理：for (const node of scroll.querySelectorAll('[data-section-id]')) {。
    for (const node of scroll.querySelectorAll('[data-section-id]')) {
      // 【逐行批注】 声明前端变量 section，保存本行计算出的数据或对象。
      const section=sectionFor(node.dataset.sectionId);
      // 【逐行批注】 判断条件 (section) api.recordVisibleSection(lessonId,section,isQuestion(s…，成立时执行对应逻辑。
      if (section) api.recordVisibleSection(lessonId,section,isQuestion(section) ? node : node.querySelector('.lesson-body'),scroll);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [key, record] of records) if (record.role === 'assistant')。
    for (const [key, record] of records) if (record.role === 'assistant')
      // 【逐行批注】 执行当前前端表达式：api.recordVisibleMessage(record,nodes.get(key)?.querySelector('.chat-message-body'),scrol…。
      api.recordVisibleMessage(record,nodes.get(key)?.querySelector('.chat-message-body'),scroll);
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [key, record] of records) if (record.type === 'evaluation')。
    for (const [key, record] of records) if (record.type === 'evaluation')
      // 【逐行批注】 执行当前前端表达式：api.recordEvaluation(lessonId, record, nodes.get(key)?.querySelector('[data-evaluation-fe…。
      api.recordEvaluation(lessonId, record, nodes.get(key)?.querySelector('[data-evaluation-feedback]'), scroll);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 setDisabled，封装可复用的前端行为。
  function setDisabled() {
    // 【逐行批注】 声明前端变量 locked，保存本行计算出的数据或对象。
    const locked = !loaded || busy || sending || Boolean(admissionError);
    // 先恢复课时与草稿再开放输入；AI 处理中仍可继续编辑草稿。
    // 【逐行批注】 计算右侧结果并更新 input.disabled，改变页面或请求状态。
    input.disabled = !loaded;
    // 【逐行批注】 按循环规则遍历或重复处理：for (const control of scroll.querySelectorAll('form button, form input')) …。
    for (const control of scroll.querySelectorAll('form button, form input')) control.disabled = locked;
    // 【逐行批注】 按循环规则遍历或重复处理：for (const id of ['lesson-send', 'teaching-hint', 'teaching-example', 'tea…。
    for (const id of ['lesson-send', 'teaching-hint', 'teaching-example', 'teaching-retry', 'explain-again', 'continue-teaching', 'finish-lesson']) $(id).disabled = locked;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 paintState，封装可复用的前端行为。
  function paintState(task) {
    // 【逐行批注】 计算右侧结果并更新 localFailureRetry，改变页面或请求状态。
    localFailureRetry = null;
    // 【逐行批注】 计算右侧结果并更新 busy，改变页面或请求状态。
    busy = !task.paused && ['pending', 'running'].includes(task.status);
    // 【逐行批注】 声明前端变量 labels，保存本行计算出的数据或对象。
    const labels = {pending:'等待 AI 处理', running:'AI 正在结合你的回答互动', waiting_student:'你可以继续回答或追问', ready:'AI 已完成本次互动',
      // 【逐行批注】 执行当前前端表达式：paused:'AI 已暂停，输入和已有结果保留', cancelled:'已停止本次回答；中断输出不作为正式评价', superseded:'学习记录已有更新，旧输出未应用'};。
      paused:'AI 已暂停，输入和已有结果保留', cancelled:'已停止本次回答；中断输出不作为正式评价', superseded:'学习记录已有更新，旧输出未应用'};
    // 【逐行批注】 计算右侧结果并更新 $('lesson-task-status').textContent，改变页面或请求状态。
    $('lesson-task-status').textContent = task.paused ? labels.paused : task.status === 'failed' ? api.failureMessage(task) : labels[task.status] || task.status;
    // 【逐行批注】 计算右侧结果并更新 $('lesson-stop').hidden，改变页面或请求状态。
    $('lesson-stop').hidden = !busy; $('lesson-retry').hidden = !['failed', 'cancelled', 'superseded'].includes(task.status); setDisabled();
    // 【逐行批注】 执行当前前端表达式：trace?.update(task);。
    trace?.update(task);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 controlWatchingTask，封装可复用的前端行为。
  async function controlWatchingTask(command) {
    // 【逐行批注】 判断条件 (!watchingTask) return;，成立时执行对应逻辑。
    if (!watchingTask) return;
    // 【逐行批注】 声明前端变量 button，保存本行计算出的数据或对象。
    const button = command === 'cancel' ? $('lesson-stop') : command === 'retry' ? $('lesson-retry') : null;
    // 【逐行批注】 判断条件 (button) { button.disabled = true; button.dataset.label = button…，成立时执行对应逻辑。
    if (button) { button.disabled = true; button.dataset.label = button.textContent; button.textContent = command === 'cancel' ? '正在停止…' : '正在重试…'; }
    // 【逐行批注】 计算右侧结果并更新 try { const task，改变页面或请求状态。
    try { const task = await api.control({command, taskId:watchingTask}); paintState(task); observe(task.id, true); }
    // 【逐行批注】 计算右侧结果并更新 catch (error) { $('lesson-task-status').text…，改变页面或请求状态。
    catch (error) { $('lesson-task-status').textContent = error.message; }
    // 【逐行批注】 计算右侧结果并更新 finally { if (button) { button.disabled，改变页面或请求状态。
    finally { if (button) { button.disabled = false; button.textContent = button.dataset.label || button.textContent; } }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 observe，封装可复用的前端行为。
  function observe(taskId, restart = false) {
    // 【逐行批注】 判断条件 (watchingTask === taskId && !restart) return;，成立时执行对应逻辑。
    if (watchingTask === taskId && !restart) return;
    // 【逐行批注】 计算右侧结果并更新 unsubscribe?.(); watchingTask，改变页面或请求状态。
    unsubscribe?.(); watchingTask = taskId; const epoch = ++watchEpoch;
    // 【逐行批注】 计算右侧结果并更新 watchingView，改变页面或请求状态。
    watchingView = lesson.teachingTaskView || (lesson.timeline || []).find(item => item.taskId === taskId && item.view)?.view || activeView;
    // 【逐行批注】 执行当前前端表达式：window.GangyiNavigation?.setContext({teachingTaskId:taskId});。
    window.GangyiNavigation?.setContext({teachingTaskId:taskId});
    // 【逐行批注】 声明前端变量 afterSeq，保存本行计算出的数据或对象。
    const afterSeq = Math.max(0, ...[...records.values()].filter(record => record.taskId === taskId).flatMap(record => record.sequences || []));
    // 【逐行批注】 计算右侧结果并更新 unsubscribe，改变页面或请求状态。
    unsubscribe = api.watch(taskId, {afterSeq,
      // 【逐行批注】 调用 onEvent 执行页面交互或数据处理：onEvent(event) {。
      onEvent(event) {
        // 【逐行批注】 判断条件 (epoch !== watchEpoch) return false;，成立时执行对应逻辑。
        if (epoch !== watchEpoch) return false;
        // 【逐行批注】 执行当前前端表达式：trace?.event(event);。
        trace?.event(event);
        // 【逐行批注】 判断条件 (event.type === 'action') { refreshLesson(); return false; }，成立时执行对应逻辑。
        if (event.type === 'action') { refreshLesson(); return false; }
        // 【逐行批注】 判断条件 (event.type !== 'delta' || event.field !== 'message' || watching…，成立时执行对应逻辑。
        if (event.type !== 'delta' || event.field !== 'message' || watchingView !== activeView) return false;
        // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
        const key = taskId + ':step:' + event.step, previous = records.get(key);
        // 【逐行批注】 声明前端变量 record，保存本行计算出的数据或对象。
        const record = {...previous, type:'message', role:'assistant', taskId, view:watchingView, id:key, step:event.step,
          // 【逐行批注】 执行当前前端表达式：text:(previous?.text || '') + event.text, sequences:[...(previous?.sequences || []), even…。
          text:(previous?.text || '') + event.text, sequences:[...(previous?.sequences || []), event.seq], partial:true, optimistic:true};
        // 【逐行批注】 计算右侧结果并更新 mutate(()，改变页面或请求状态。
        mutate(() => { const node = putRecord(key, record); if (!node.isConnected) root.append(node); });
        // 【逐行批注】 结束当前函数并返回结果：return false; // 可见性检查单独记录已读，向上回看时不把新回复算作已看。。
        return false; // 可见性检查单独记录已读，向上回看时不把新回复算作已看。
      // 【逐行批注】 执行当前前端表达式：},。
      },
      // 【逐行批注】 调用 onState、paintState 执行页面交互或数据处理：onState(task) { if (epoch !== watchEpoch) return; paintState(task);。
      onState(task) { if (epoch !== watchEpoch) return; paintState(task);
        // 【逐行批注】 判断条件 (task.view) watchingView = task.view;，成立时执行对应逻辑。
        if (task.view) watchingView = task.view;
        // 【逐行批注】 判断条件 (['ready', 'waiting_student'].includes(task.status)) for (const …，成立时执行对应逻辑。
        if (['ready', 'waiting_student'].includes(task.status)) for (const scope of ['chat', 'finish', ...lesson.sections.filter(isQuestion).map(section => section.id)]) {
          // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
          const key = 'pending:' + lessonId + ':' + (scope === 'chat' ? scope + ':' + watchingView : scope);
          // 【逐行批注】 计算右侧结果并更新 try { const pending，改变页面或请求状态。
          try { const pending = JSON.parse(api.storage.get(key) || 'null'); if (pending?.requestId === task.requestId) api.storage.remove(key); }
          // 【逐行批注】 执行当前前端表达式：catch (_) { /* 其他窗口的输入缓存不能被旧任务清除。 * / }。
          catch (_) { /* 其他窗口的输入缓存不能被旧任务清除。 */ }
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 判断条件 (!['pending', 'running'].includes(task.status)) {，成立时执行对应逻辑。
        if (!['pending', 'running'].includes(task.status)) {
          // 【逐行批注】 判断条件 (watchingView === activeView && task.message && ![...records.val…，成立时执行对应逻辑。
          if (watchingView === activeView && task.message && ![...records.values()].some(record => record.taskId === taskId && record.role === 'assistant'))
            // 【逐行批注】 计算右侧结果并更新 mutate(()，改变页面或请求状态。
            mutate(() => root.append(putRecord(taskId + ':fallback', {type:'message', role:'assistant', taskId, text:task.message, optimistic:true})));
          // 【逐行批注】 按循环规则遍历或重复处理：for (const [key, record] of records) if (record.taskId === taskId && recor…。
          for (const [key, record] of records) if (record.taskId === taskId && record.role === 'assistant') updateMessage(nodes.get(key), record);
          // 【逐行批注】 调用 refreshLesson 执行页面交互或数据处理：refreshLesson();。
          refreshLesson();
          // 【逐行批注】 判断条件 (initialTeachingDeferred && ['ready', 'waiting_student'].include…，成立时执行对应逻辑。
          if (initialTeachingDeferred && ['ready', 'waiting_student'].includes(task.status)) startDeferredInitialTeaching();
        // 【逐行批注】 结束当前前端代码块。
        }
      // 【逐行批注】 执行当前前端表达式：},。
      },
      // 【逐行批注】 调用 onConnectionError 执行页面交互或数据处理：onConnectionError(error) { if (epoch === watchEpoch) $('lesson-task-s…。
      onConnectionError(error) { if (epoch === watchEpoch) $('lesson-task-status').textContent = error; }
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 startDeferredInitialTeaching，封装可复用的前端行为。
  async function startDeferredInitialTeaching() {
    // 【逐行批注】 判断条件 (!initialTeachingDeferred || startingInitialTeaching) return;，成立时执行对应逻辑。
    if (!initialTeachingDeferred || startingInitialTeaching) return;
    // 【逐行批注】 计算右侧结果并更新 startingInitialTeaching，改变页面或请求状态。
    startingInitialTeaching = true; initialTeachingDeferred = false;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 started，保存本行计算出的数据或对象。
      const started = await api.control({command:'start_lesson', lessonId});
      // 【逐行批注】 计算右侧结果并更新 initialTeachingDeferred，改变页面或请求状态。
      initialTeachingDeferred = Boolean(started.initialTeachingDeferred);
      // 新开课绑定后，先前仍在返回的课时读取不能把旧任务重新装回界面。
      // 【逐行批注】 调用 refreshLesson 执行页面交互或数据处理：taskRevision++; refreshLesson();。
      taskRevision++; refreshLesson();
    // 【逐行批注】 计算右侧结果并更新 } catch (error) { initialTeachingDeferred，改变页面或请求状态。
    } catch (error) { initialTeachingDeferred = true; $('lesson-task-status').textContent = error.message; }
    // 【逐行批注】 计算右侧结果并更新 finally { startingInitialTeaching，改变页面或请求状态。
    finally { startingInitialTeaching = false; }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 refreshLesson，封装可复用的前端行为。
  async function refreshLesson() {
    // 【逐行批注】 判断条件 (refreshing) { refreshAgain = true; return; } refreshing = true;，成立时执行对应逻辑。
    if (refreshing) { refreshAgain = true; return; } refreshing = true;
    // 【逐行批注】 声明前端变量 revision，保存本行计算出的数据或对象。
    const revision = taskRevision;
    // 【逐行批注】 计算右侧结果并更新 try { await api.flushExposure(); const saved，改变页面或请求状态。
    try { await api.flushExposure(); const saved = await api.request('/api/learning-agent/lesson?lessonId=' + encodeURIComponent(lessonId));
      // 【逐行批注】 判断条件 (revision !== taskRevision) { refreshAgain = true; return; }，成立时执行对应逻辑。
      if (revision !== taskRevision) { refreshAgain = true; return; }
      // 【逐行批注】 声明前端变量 previousFocus，保存本行计算出的数据或对象。
      const previousFocus = focus().responseSectionId || focus().interactionSectionId, completed = lesson.completed;
      // 【逐行批注】 计算右侧结果并更新 lesson，改变页面或请求状态。
      lesson = saved; render(); const nextFocus = focus().responseSectionId || focus().interactionSectionId;
      // 【逐行批注】 判断条件 (nextFocus && nextFocus !== previousFocus && !input.value) selec…，成立时执行对应逻辑。
      if (nextFocus && nextFocus !== previousFocus && !input.value) selectQuestion(nextFocus);
      // 【逐行批注】 判断条件 (lesson.completed && !completed) document.dispatchEvent(new Cust…，成立时执行对应逻辑。
      if (lesson.completed && !completed) document.dispatchEvent(new CustomEvent('gangyi:lesson-finished'));
    // 【逐行批注】 计算右侧结果并更新 } catch (error) { $('lesson-task-status').te…，改变页面或请求状态。
    } catch (error) { $('lesson-task-status').textContent = error.message; }
    // 【逐行批注】 计算右侧结果并更新 finally { refreshing，改变页面或请求状态。
    finally { refreshing = false; if (refreshAgain) { refreshAgain = false; refreshLesson(); } }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 sendEvent，封装可复用的前端行为。
  async function sendEvent(event, sourceDraft, choiceState) {
    // 【逐行批注】 判断条件 (admissionError || busy || sending) return;，成立时执行对应逻辑。
    if (admissionError || busy || sending) return;
    // 【逐行批注】 声明前端变量 startedAt，保存本行计算出的数据或对象。
    const startedAt = Date.now(), composer = $('lesson-chat-panel');
    // 点击后到服务端返回之间必须有可见文案，不能只把按钮灰掉。
    // 【逐行批注】 计算右侧结果并更新 sending，改变页面或请求状态。
    sending = true; $('lesson-task-status').textContent = '正在提交给 AI…';
    // 【逐行批注】 调用 setDisabled 执行页面交互或数据处理：composer.setAttribute('aria-busy', 'true'); setDisabled();。
    composer.setAttribute('aria-busy', 'true'); setDisabled();
    // 【逐行批注】 声明前端变量 scope，保存本行计算出的数据或对象。
    const scope = event.type === 'question_answer' ? event.sectionId : event.type === 'lesson_finish_request' ? 'finish' : 'chat';
    // 【逐行批注】 声明前端变量 pendingKey，保存本行计算出的数据或对象。
    const pendingKey = 'pending:' + lessonId + ':' + (scope === 'chat' ? scope + ':' + activeView : scope);
    // 【逐行批注】 计算右侧结果并更新 event，改变页面或请求状态。
    event = {...event, courseId:lesson.courseId, lessonId, view:activeView, ...reviewContext, requestId:api.id()};
    // 【逐行批注】 计算右侧结果并更新 try { const pending，改变页面或请求状态。
    try { const pending = JSON.parse(api.storage.get(pendingKey) || 'null');
      // 【逐行批注】 判断条件 (pending && pending.type === event.type && pending.text === even…，成立时执行对应逻辑。
      if (pending && pending.type === event.type && pending.text === event.text && pending.action === event.action &&
        // 【逐行批注】 执行当前前端表达式：pending.sectionId === event.sectionId && pending.sectionVersion === event.sectionVersion …。
        pending.sectionId === event.sectionId && pending.sectionVersion === event.sectionVersion && pending.view === activeView) event = pending;
    // 【逐行批注】 执行当前前端表达式：} catch (_) { /* 损坏缓存不改变本次输入。 * / }。
    } catch (_) { /* 损坏缓存不改变本次输入。 */ }
    // 【逐行批注】 执行当前前端表达式：api.storage.set(pendingKey, JSON.stringify(event));。
    api.storage.set(pendingKey, JSON.stringify(event));
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 计算右侧结果并更新 exposeVisible(); const task，改变页面或请求状态。
      exposeVisible(); const task = await api.submit(event);
      // 【逐行批注】 判断条件 (event.type !== 'lesson_finish_request')，成立时执行对应逻辑。
      if (event.type !== 'lesson_finish_request')
        // 【逐行批注】 计算右侧结果并更新 mutate(()，改变页面或请求状态。
        mutate(() => root.append(putRecord(task.id + ':user', {type:'message', role:'user', taskId:task.id, text:event.text, optimistic:true})));
      // 【逐行批注】 判断条件 (sourceDraft && api.storage.get(sourceDraft.key) === sourceDraft…，成立时执行对应逻辑。
      if (sourceDraft && api.storage.get(sourceDraft.key) === sourceDraft.text) {
        // 【逐行批注】 判断条件 (draftKey() === sourceDraft.key && input.value === sourceDraft.t…，成立时执行对应逻辑。
        if (draftKey() === sourceDraft.key && input.value === sourceDraft.text) { input.value = ''; resizeInput(); }
        // 【逐行批注】 执行当前前端表达式：api.storage.set(sourceDraft.key, '');。
        api.storage.set(sourceDraft.key, '');
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (choiceState && api.storage.get(choiceState.key) === choiceState…，成立时执行对应逻辑。
      if (choiceState && api.storage.get(choiceState.key) === choiceState.value) { api.storage.set(choiceState.key, '[]'); for (const choice of choiceState.form.querySelectorAll('input')) choice.checked = false; }
      // 【逐行批注】 计算右侧结果并更新 taskRevision++; lesson.teachingTaskId，改变页面或请求状态。
      taskRevision++; lesson.teachingTaskId = task.id; lesson.teachingTaskView = activeView; paintState(task); observe(task.id); refreshLesson();
    // 【逐行批注】 执行当前前端表达式：} catch (error) {。
    } catch (error) {
      // 【逐行批注】 计算右侧结果并更新 $('lesson-task-status').textContent，改变页面或请求状态。
      $('lesson-task-status').textContent = error.message;
      // 【逐行批注】 计算右侧结果并更新 feedLocalFailure('提交课堂互动', error.message, st…，改变页面或请求状态。
      feedLocalFailure('提交课堂互动', error.message, startedAt, () => sendEvent(event, sourceDraft, choiceState));
    // 【逐行批注】 计算右侧结果并更新 } finally { sending，改变页面或请求状态。
    } finally { sending = false; composer.removeAttribute('aria-busy'); setDisabled(); }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 sendAnswer，封装可复用的前端行为。
  function sendAnswer(id, action) {
    // 【逐行批注】 声明前端变量 section，保存本行计算出的数据或对象。
    const section = sectionFor(id), form = nodes.get('section:' + id)?.querySelector('form'); if (!isQuestion(section)) return;
    // 【逐行批注】 声明前端变量 choices，保存本行计算出的数据或对象。
    const choices = [...(form?.querySelectorAll('input:checked') || [])];
    // 【逐行批注】 声明前端变量 explanation，保存本行计算出的数据或对象。
    const explanation = selectedQuestion === id ? input.value.trim() : readDraft('draft:' + lessonId + ':' + id, section).trim();
    // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
    const text = action === 'unknown' ? '暂时不会' : action === 'skip' ? '我跳过这道题' :
      // 【逐行批注】 计算右侧结果并更新 (choices.length ? '我选' + choices.map(choice，改变页面或请求状态。
      (choices.length ? '我选' + choices.map(choice => (Number(choice.value) + 1) + '：' + section.question.options[Number(choice.value)]).join('、') + '\n' : '') + explanation;
    // 【逐行批注】 判断条件 (!text.trim()) { if (form) form.querySelector('.question-note').…，成立时执行对应逻辑。
    if (!text.trim()) { if (form) form.querySelector('.question-note').textContent = '请先选择选项，或在下方写出你的答案。'; selectQuestion(id, true); return; }
    // 【逐行批注】 调用 sendEvent 执行页面交互或数据处理：sendEvent({type:'question_answer', action, text, sectionId:id, sectio…。
    sendEvent({type:'question_answer', action, text, sectionId:id, sectionVersion:section.version},
      // 【逐行批注】 调用 readDraft 执行页面交互或数据处理：action === 'answer' ? {key:'draft:' + lessonId + ':' + id, text:selec…。
      action === 'answer' ? {key:'draft:' + lessonId + ':' + id, text:selectedQuestion === id ? input.value : readDraft('draft:' + lessonId + ':' + id, section)} : null,
      // 【逐行批注】 执行当前前端表达式：action === 'answer' && form ? {key:'draft:' + lessonId + ':' + id + ':choice', value:api.…。
      action === 'answer' && form ? {key:'draft:' + lessonId + ':' + id + ':choice', value:api.storage.get('draft:' + lessonId + ':' + id + ':choice'), form} : null);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 sendTeaching，封装可复用的前端行为。
  function sendTeaching(text, fromComposer = false) {
    // “正在回答”使用与题卡一致的完整作答；切到自由交流后仍只发送追问文字。
    // 【逐行批注】 判断条件 (fromComposer && selectedQuestion) { sendAnswer(selectedQuestion…，成立时执行对应逻辑。
    if (fromComposer && selectedQuestion) { sendAnswer(selectedQuestion, 'answer'); return; }
    // 【逐行批注】 判断条件 (!text.trim()) { input.focus(); return; }，成立时执行对应逻辑。
    if (!text.trim()) { input.focus(); return; }
    // 【逐行批注】 调用 sendEvent、focus、draftKey 执行页面交互或数据处理：sendEvent({type:'chat', text, ...(focus().sectionId ? {sectionId:focu…。
    sendEvent({type:'chat', text, ...(focus().sectionId ? {sectionId:focus().sectionId} : {})}, fromComposer ? {key:draftKey(), text:input.value} : null);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 requestTeaching，封装可复用的前端行为。
  function requestTeaching(requestType) {
    // 【逐行批注】 调用 saveDraft 执行页面交互或数据处理：saveDraft();。
    saveDraft();
    // 【逐行批注】 声明前端变量 section，保存本行计算出的数据或对象。
    const section = sectionFor(selectedQuestion || focus().responseSectionId || focus().practiceSectionId || focus().interactionSectionId);
    // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
    const text = {hint:'请只给一点提示，先提供一个方向，保留让我独立完成的空间。',worked_example:'请示范当前题的完整过程，之后根据我的回答判断是否需要独立复练。',retry_question:'请根据本题卡点安排一道关联的新题，让我独立再试。',rephrase:'请结合我的反馈和教学偏好，围绕当前内容换一种讲法。'}[requestType];
    // 【逐行批注】 判断条件 (requestType !== 'rephrase' && !isQuestion(section)) { $('lesson…，成立时执行对应逻辑。
    if (requestType !== 'rephrase' && !isQuestion(section)) { $('lesson-task-status').textContent = '请先选择要帮助或复练的题目。'; return; }
    // 【逐行批注】 调用 sendEvent、isQuestion 执行页面交互或数据处理：sendEvent({type:'teaching_request',requestType,text,...(isQuestion(se…。
    sendEvent({type:'teaching_request',requestType,text,...(isQuestion(section)?{sectionId:section.id,sectionVersion:section.version}:{})});
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 load，封装可复用的前端行为。
  async function load() {
    // 【逐行批注】 执行当前前端表达式：document.body.classList.add('gy-chat-classroom');。
    document.body.classList.add('gy-chat-classroom');
    // 【逐行批注】 判断条件 (!lessonId) throw new Error('缺少课时标识，请从课程入口进入。');，成立时执行对应逻辑。
    if (!lessonId) throw new Error('缺少课时标识，请从课程入口进入。');
    // 【逐行批注】 计算右侧结果并更新 lesson，改变页面或请求状态。
    lesson = await api.request('/api/learning-agent/lesson?lessonId=' + encodeURIComponent(lessonId));
    // 【逐行批注】 判断条件 (lesson.status !== 'ready') throw new Error('课时尚未完整保存，请等待 AI 备课后…，成立时执行对应逻辑。
    if (lesson.status !== 'ready') throw new Error('课时尚未完整保存，请等待 AI 备课后进入。');
    // 【逐行批注】 声明前端变量 requestedSection，保存本行计算出的数据或对象。
    const requestedSection = sectionFor(query.get('sectionId'));
    // 【逐行批注】 判断条件 (requestedSection && areaFor(requestedSection) !== activeView) {…，成立时执行对应逻辑。
    if (requestedSection && areaFor(requestedSection) !== activeView) { location.replace(sectionHref(requestedSection)); return; }
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 判断条件 (!lesson.entered) await api.control({command:'enter_lesson', les…，成立时执行对应逻辑。
      if (!lesson.entered) await api.control({command:'enter_lesson', lessonId});
      // 回看已保存卡片只读取原记录，普通讲解入口仍按原流程启动教学。
      // 【逐行批注】 判断条件 (activeView === 'learn' && !lesson.initialTeachingTaskId && !req…，成立时执行对应逻辑。
      if (activeView === 'learn' && !lesson.initialTeachingTaskId && !requestedSection) {
        // 【逐行批注】 声明前端变量 started，保存本行计算出的数据或对象。
        const started = await api.control({command:'start_lesson', lessonId}); initialTeachingDeferred = Boolean(started.initialTeachingDeferred);
        // 【逐行批注】 计算右侧结果并更新 lesson，改变页面或请求状态。
        lesson = await api.request('/api/learning-agent/lesson?lessonId=' + encodeURIComponent(lessonId)); }
    // 【逐行批注】 计算右侧结果并更新 } catch (error) { admissionError，改变页面或请求状态。
    } catch (error) { admissionError = error.message; }
    // 【逐行批注】 声明前端变量 context，保存本行计算出的数据或对象。
    const context = {courseId:lesson.courseId, lessonId, phaseIndex:lesson.phaseIndex, topicIndex:lesson.topicIndex, topicId:lesson.topicId, teachingTaskId:lesson.teachingTaskId};
    // 【逐行批注】 调用 CustomEvent 执行页面交互或数据处理：window.GangyiNavigation?.setContext(context); document.dispatchEvent(…。
    window.GangyiNavigation?.setContext(context); document.dispatchEvent(new CustomEvent('gangyi:lesson-context', {detail:context}));
    // 【逐行批注】 调用 element、URLSearchParams 执行页面交互或数据处理：if(!document.getElementById('lesson-library-link')){const link=elemen…。
    if(!document.getElementById('lesson-library-link')){const link=element('a','本课资料 →','ma-link');link.id='lesson-library-link';link.href='/my-ai/library?'+new URLSearchParams({courseId:lesson.courseId});$('page-status').insertAdjacentElement('afterend',link);}
    // 【逐行批注】 调用 lessonHref 执行页面交互或数据处理：api.storage.set('current-lesson', JSON.stringify({lessonId, courseId:…。
    api.storage.set('current-lesson', JSON.stringify({lessonId, courseId:lesson.courseId, href:lessonHref('learn')}));
    // 【逐行批注】 声明前端变量 savedTarget，保存本行计算出的数据或对象。
    const savedTarget = query.get('sectionId') || api.storage.get(pageKey('composer')) || (activeView === 'learn' ? api.storage.get('composer:' + lessonId) : '');
    // 【逐行批注】 计算右侧结果并更新 selectedQuestion，改变页面或请求状态。
    selectedQuestion = savedTarget === 'chat' ? '' : [savedTarget, focus().responseSectionId,
      // 【逐行批注】 调用 focus 执行页面交互或数据处理：activeView === 'practice' ? focus().practiceSectionId : focus().inter…。
      activeView === 'practice' ? focus().practiceSectionId : focus().interactionSectionId]
      // 【逐行批注】 计算右侧结果并更新 .find(id，改变页面或请求状态。
      .find(id => isQuestion(sectionFor(id)) && areaFor(sectionFor(id)) === activeView) || '';
    // 【逐行批注】 判断条件 (!isQuestion(sectionFor(selectedQuestion)) || areaFor(sectionFor…，成立时执行对应逻辑。
    if (!isQuestion(sectionFor(selectedQuestion)) || areaFor(sectionFor(selectedQuestion)) !== activeView) selectedQuestion = '';
    // 【逐行批注】 判断条件 (!selectedQuestion && activeView === 'learn' && !api.storage.get…，成立时执行对应逻辑。
    if (!selectedQuestion && activeView === 'learn' && !api.storage.get(pageKey('chat-draft')))
      // 【逐行批注】 调用 pageKey 执行页面交互或数据处理：api.storage.set(pageKey('chat-draft'), api.storage.get('chat-draft:' …。
      api.storage.set(pageKey('chat-draft'), api.storage.get('chat-draft:' + lessonId));
    // 【逐行批注】 计算右侧结果并更新 input.value，改变页面或请求状态。
    input.value = readDraft(draftKey(), sectionFor(selectedQuestion)); loaded = true; resizeInput();
    // 【逐行批注】 计算右侧结果并更新 $('page-status').textContent，改变页面或请求状态。
    $('page-status').textContent = admissionError ? admissionError + '。你可以回看已保存内容；继续学习请重新备课。' :
      // 【逐行批注】 执行当前前端表达式：{learn:'讲解与随堂互动', practice:'集中练习与作答交流', summary:'小结、检查点与完成判断'}[activeView] + ' · 本页独立保存交流…。
      {learn:'讲解与随堂互动', practice:'集中练习与作答交流', summary:'小结、检查点与完成判断'}[activeView] + ' · 本页独立保存交流和草稿，AI 共享本节学习记录。';
    // 【逐行批注】 判断条件 (admissionError) { $('page-status').classList.add('agent-error')…，成立时执行对应逻辑。
    if (admissionError) { $('page-status').classList.add('agent-error'); const link = element('a', '按最新学习情况重新备课 →');
      // 【逐行批注】 计算右侧结果并更新 link.href，改变页面或请求状态。
      link.href = '/agent-prepare.html?' + new URLSearchParams({courseId:lesson.courseId, ...(lesson.topicId ? {topicId:lesson.topicId} : {})}); $('page-status').append(link); }
    // 【逐行批注】 计算右侧结果并更新 render(); for (const link of document.queryS…，改变页面或请求状态。
    render(); for (const link of document.querySelectorAll('[data-lesson-link]')) { link.href = lessonHref(link.dataset.lessonLink);
      // 【逐行批注】 判断条件 (link.dataset.lessonLink === activeView) link.setAttribute('aria…，成立时执行对应逻辑。
      if (link.dataset.lessonLink === activeView) link.setAttribute('aria-current', 'page'); }
    // 【逐行批注】 判断条件 (!lesson.teachingTaskId) $('lesson-task-status').textContent = a…，成立时执行对应逻辑。
    if (!lesson.teachingTaskId) $('lesson-task-status').textContent = admissionError || '你可以继续回答或追问';
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 为页面事件绑定处理逻辑：input.addEventListener('input', () => { saveDraft(); resizeInput(); });。
  input.addEventListener('input', () => { saveDraft(); resizeInput(); });
  // 【逐行批注】 为页面事件绑定处理逻辑：input.addEventListener('keydown', event => { if (event.key === 'Enter' && !event.shi…。
  input.addEventListener('keydown', event => { if (event.key === 'Enter' && !event.shiftKey && !event.isComposing && !busy && !sending) { event.preventDefault(); sendTeaching(input.value, true); } });
  // 【逐行批注】 计算右侧结果并更新 $('lesson-chat').onsubmit，改变页面或请求状态。
  $('lesson-chat').onsubmit = event => { event.preventDefault(); sendTeaching(input.value, true); };
  // 【逐行批注】 计算右侧结果并更新 $('composer-chat').onclick，改变页面或请求状态。
  $('composer-chat').onclick = () => selectQuestion('', true);
  // 【逐行批注】 计算右侧结果并更新 $('explain-again').onclick，改变页面或请求状态。
  $('explain-again').onclick = () => requestTeaching('rephrase');
  // 【逐行批注】 计算右侧结果并更新 $('teaching-hint').onclick，改变页面或请求状态。
  $('teaching-hint').onclick = () => requestTeaching('hint');
  // 【逐行批注】 计算右侧结果并更新 $('teaching-example').onclick，改变页面或请求状态。
  $('teaching-example').onclick = () => requestTeaching('worked_example');
  // 【逐行批注】 计算右侧结果并更新 $('teaching-retry').onclick，改变页面或请求状态。
  $('teaching-retry').onclick = () => requestTeaching('retry_question');
  // 【逐行批注】 计算右侧结果并更新 $('continue-teaching').onclick，改变页面或请求状态。
  $('continue-teaching').onclick = () => sendTeaching('请结合本节实际反馈判断下一步；如果适合继续，请选择下一个教学焦点。');
  // 【逐行批注】 计算右侧结果并更新 $('lesson-stop').onclick，改变页面或请求状态。
  $('lesson-stop').onclick = () => controlWatchingTask('cancel');
  // 【逐行批注】 计算右侧结果并更新 $('lesson-retry').onclick，改变页面或请求状态。
  $('lesson-retry').onclick = () => controlWatchingTask('retry');
  // 【逐行批注】 计算右侧结果并更新 $('finish-lesson').onclick，改变页面或请求状态。
  $('finish-lesson').onclick = () => sendEvent({type:'lesson_finish_request', text:'请根据本节真实作答和已有评价判断完成情况，证据不足时明确说明，不使用固定通过率。'});
  // 【逐行批注】 计算右侧结果并更新 $('prepare-next').onclick，改变页面或请求状态。
  $('prepare-next').onclick = () => { location.href = '/agent-prepare.html?' + new URLSearchParams({courseId:lesson.courseId, requestId:api.id()}); };
  // 【逐行批注】 计算右侧结果并更新 $('lesson-latest').onclick，改变页面或请求状态。
  $('lesson-latest').onclick = () => { scroll.scrollTop = scroll.scrollHeight; exposeVisible(); };
  // 【逐行批注】 按循环规则遍历或重复处理：for (const button of document.querySelectorAll('[data-lesson-link]')) butt…。
  for (const button of document.querySelectorAll('[data-lesson-link]')) button.onclick = () => locate(button.dataset.lessonLink);
  // 【逐行批注】 声明前端变量 scrollFrame，保存本行计算出的数据或对象。
  let scrollFrame;
  // 【逐行批注】 为页面事件绑定处理逻辑：scroll.addEventListener('scroll', () => { if (rendering || !loaded || initialPositio…。
  scroll.addEventListener('scroll', () => { if (rendering || !loaded || initialPosition) return; cancelAnimationFrame(scrollFrame);
    // 【逐行批注】 计算右侧结果并更新 scrollFrame，改变页面或请求状态。
    scrollFrame = requestAnimationFrame(() => { api.storage.set(pageKey('scroll'), JSON.stringify(readingPosition())); $('lesson-latest').hidden = nearBottom(); exposeVisible(); }); });
  // 【逐行批注】 为页面事件绑定处理逻辑：scroll.addEventListener('toggle', () => requestAnimationFrame(exposeVisible), true);。
  scroll.addEventListener('toggle', () => requestAnimationFrame(exposeVisible), true);
  // 【逐行批注】 为页面事件绑定处理逻辑：scroll.addEventListener('load', () => requestAnimationFrame(exposeVisible), true);。
  scroll.addEventListener('load', () => requestAnimationFrame(exposeVisible), true);
  // 【逐行批注】 计算右侧结果并更新 new ResizeObserver(()，改变页面或请求状态。
  new ResizeObserver(() => { if (loaded) exposeVisible(); }).observe(scroll);
  // 【逐行批注】 计算右侧结果并更新 new ResizeObserver(()，改变页面或请求状态。
  new ResizeObserver(() => document.body.style.setProperty('--classroom-composer-height', $('lesson-chat-panel').offsetHeight + 'px')).observe($('lesson-chat-panel'));
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('gangyi:agent-control', event => { const {command, task, a…。
  document.addEventListener('gangyi:agent-control', event => { const {command, task, all} = event.detail || {};
    // 【逐行批注】 判断条件 (watchingTask && ['pause', 'resume', 'retry', 'cancel'].includes…，成立时执行对应逻辑。
    if (watchingTask && ['pause', 'resume', 'retry', 'cancel'].includes(command) && (all || task?.id === watchingTask)) observe(watchingTask, true); });
  // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('pagehide', () => { saveDraft(); if (loaded && !initialPosit…。
  window.addEventListener('pagehide', () => { saveDraft(); if (loaded && !initialPosition) api.storage.set(pageKey('scroll'), JSON.stringify(readingPosition())); unsubscribe?.(); watchEpoch++; });
  // 【逐行批注】 调用 setDisabled 执行页面交互或数据处理：setDisabled();。
  setDisabled();
  // 【逐行批注】 计算右侧结果并更新 load().catch(error，改变页面或请求状态。
  load().catch(error => { $('page-status').textContent = error.message; $('page-status').classList.add('agent-error'); $('lesson-task-status').textContent = '课堂暂未就绪'; });
// 【逐行批注】 执行当前前端表达式：})();。
})();

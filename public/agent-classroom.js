/* 同一知识点共享教学背景，三页分别呈现自己的真实内容、对话与草稿。 */
(() => {
  'use strict';
  const api = window.GangyiAgent, query = new URLSearchParams(location.search), lessonId = query.get('lessonId');
  const reviewContext = query.has('review') ? {day: Number(query.get('review')), reviewId: query.get('reviewId') || ''} : {};
  const lessonHref = path => {
    const params = new URLSearchParams({lessonId});
    for (const key of ['review', 'reviewId', 'day', 'lessonTaskId']) if (query.has(key)) params.set(key, query.get(key));
    return '/' + path + '?' + params;
  };
  function readDraft(key, section) {
    const current = api.storage.get(key), legacy = lessonId.startsWith('legacy-');
    const marker = `legacy-draft-migrated:${key}`;
    let hasCurrent = Boolean(current);
    try { hasCurrent = hasCurrent || localStorage.getItem(`gangyi-agent:${key}`) !== null; } catch (_) { /* 存储不可读时继续使用当前输入。 */ }
    if (hasCurrent) { if(legacy)api.storage.set(marker,'1'); return current; }
    if (!legacy || api.storage.get(marker)) return '';
    const taskId = query.get('lessonTaskId') || (lessonId.startsWith('legacy-prepared-') ? lessonId.slice('legacy-prepared-'.length) : '');
    const context = {courseId: lesson.courseId, phaseIndex: lesson.phaseIndex, topicIndex: lesson.topicIndex,
      ...(taskId ? {lessonTaskId: taskId} : {}), ...reviewContext};
    const identities = section ? [...new Set([section.questionId, section.legacyIndex].filter(value=>value!==undefined&&value!==null&&value!==''))] : [];
    const oldKeys = section ? identities.map(identity=>'gy:question-draft:' + JSON.stringify([context.courseId, context.phaseIndex, context.topicIndex,
      taskId, context.reviewId || '', context.day ?? '', section.legacyKind, identity, section.version])) :
      ['gy:lesson-chat-draft:' + JSON.stringify(context)];
    try { const saved = oldKeys.map(oldKey=>localStorage.getItem(oldKey)||'').find(Boolean)||''; if(saved)api.storage.set(key,saved);
      api.storage.set(marker,'1'); return saved; }
    catch (_) { return ''; }
  }
  function material(root, value) {
    if (Array.isArray(value)) { value.forEach(item => material(root, item)); return; }
    if (typeof value !== 'object' || value === null) { const node=document.createElement('div'); api.richText(node, value ?? ''); root.append(node); return; }
    for (const key of ['title', 'text', 'body', 'latex', 'formula']) if (typeof value[key] === 'string') {
      const node=document.createElement(key === 'title' ? 'h3' : 'div'); api.richText(node, ['latex', 'formula'].includes(key) ? `$$${value[key]}$$` : value[key]); root.append(node);
    }
    if (typeof value.imageUrl === 'string' && /^https?:\/\//.test(value.imageUrl)) {
      const image=document.createElement('img'); image.src=value.imageUrl; image.alt=value.alt || '题目材料'; image.loading='lazy'; image.referrerPolicy='no-referrer'; image.style.maxWidth='100%'; root.append(image);
    }
    const rows = value.rows || value.values || value.cells;
    if (Array.isArray(rows)) {
      const table=document.createElement('table'); table.className='agent-material-table';
      if (Array.isArray(value.headers)) { const row=document.createElement('tr'); value.headers.forEach(text => { const cell=document.createElement('th'); cell.textContent=String(text); row.append(cell); }); table.append(row); }
      rows.forEach(items => { const row=document.createElement('tr'); (Array.isArray(items) ? items : [items]).forEach(text => { const cell=document.createElement('td'); api.richText(cell, text); row.append(cell); }); table.append(row); }); root.append(table);
    }
    if (value.table) material(root, value.table);
  }
  const $ = id => document.getElementById(id), root = $('lesson-sections'), scroll = $('lesson-scroll'), input = $('lesson-input');
  const nodes = new Map(), records = new Map(), shownSections = new Set();
  let lesson, admissionError = '', selectedQuestion = '', watchingTask = '', watchingView = '', unsubscribe, watchEpoch = 0;
  let busy = false, sending = false, refreshing = false, refreshAgain = false, loaded = false, rendering = false, taskRevision = 0;
  let activeView = document.querySelector('[data-lesson-view]').dataset.lessonView, initialPosition = true;
  const element = (tag, text = '', className = '') => { const node = document.createElement(tag); node.textContent = text; node.className = className; return node; };
  const focus = () => lesson?.teachingFocus || {};
  const sectionFor = id => lesson?.sections.find(section => section.id === id);
  const isQuestion = section => section?.kind === 'question' && section.legacyKind !== 'example';
  const areaFor = section => section.view || (section.kind === 'summary' ? 'summary' : isQuestion(section) &&
    !['interaction', 'diagnostic'].includes(section.questionKind || section.legacyKind) ? 'practice' : 'learn');
  const pageKey = prefix => prefix + ':' + lessonId + ':' + activeView;
  const draftKey = () => selectedQuestion ? 'draft:' + lessonId + ':' + selectedQuestion : pageKey('chat-draft');
  function saveDraft() { if (loaded) api.storage.set(draftKey(), input.value); }
  function selectQuestion(id, focusInput = false) {
    if (id && (!isQuestion(sectionFor(id)) || areaFor(sectionFor(id)) !== activeView)) return;
    saveDraft(); selectedQuestion = id || ''; api.storage.set(pageKey('composer'), selectedQuestion || 'chat');
    input.value = readDraft(draftKey(), sectionFor(selectedQuestion)); resizeInput(); paintTarget();
    if (focusInput) input.focus({preventScroll:true});
  }
  function paintTarget() {
    $('composer-target').textContent = selectedQuestion ? '正在回答：' + sectionFor(selectedQuestion).title : '和 AI 老师交流';
    $('composer-chat').hidden = !selectedQuestion;
    for (const card of scroll.querySelectorAll('[data-section-id]')) card.classList.toggle('is-current-question', card.dataset.sectionId === selectedQuestion);
  }
  function resizeInput() { const follow = nearBottom(); input.style.height = 'auto'; input.style.height = Math.min(input.scrollHeight, 136) + 'px';
    if (follow && !initialPosition) requestAnimationFrame(() => { scroll.scrollTop = scroll.scrollHeight; }); }
  const nearBottom = () => scroll.scrollHeight - scroll.clientHeight - scroll.scrollTop < 72;
  function readingPosition() {
    const boundary = scroll.getBoundingClientRect().top;
    const node = [...root.children].find(node => node.getBoundingClientRect().bottom > boundary + 8);
    return {key:node?.dataset.entryKey || '', offset:node ? node.getBoundingClientRect().top - boundary : 0, top:scroll.scrollTop, bottom:nearBottom()};
  }
  function restorePosition(position) {
    const node = nodes.get(position.key);
    if (position.bottom) scroll.scrollTop = scroll.scrollHeight;
    else if (node?.isConnected) scroll.scrollTop += node.getBoundingClientRect().top - scroll.getBoundingClientRect().top - position.offset;
    else scroll.scrollTop = position.top || 0;
  }
  function mutate(callback) {
    const position = readingPosition(); rendering = true; callback(); restorePosition(position); rendering = false;
    $('lesson-latest').hidden = nearBottom(); requestAnimationFrame(exposeVisible);
  }
  function jumpTo(node) {
    if (!node) return;
    for (let parent = node.parentElement; parent && parent !== scroll; parent = parent.parentElement) if (parent.tagName === 'DETAILS') parent.open = true;
    scroll.scrollTop = Math.max(0, node.getBoundingClientRect().top - scroll.getBoundingClientRect().top + scroll.scrollTop - 12);
    requestAnimationFrame(exposeVisible);
  }
  function locate(view, updateUrl = true) {
    if (updateUrl) { saveDraft(); location.href = lessonHref(view); return; }
    const identity = view === 'learn' ? focus().sectionId : view === 'practice' ?
      (focus().responseView === 'practice' ? focus().responseSectionId : focus().practiceSectionId) : '';
    let target = [...scroll.querySelectorAll('[data-section-id]')].find(node => node.dataset.sectionId === identity);
    target ||= [...scroll.querySelectorAll('[data-area]')].find(node => node.dataset.area === view);
    if (view === 'summary') { $('lesson-summary').open = true; target = $('lesson-completion'); }
    if (target) jumpTo(target);
  }
  function questionCard(section, card) {
    const question = element('div', '', 'agent-question'); api.richText(question, section.question.question); card.append(question);
    if (section.question.materials) { const materials = element('div', '', 'agent-materials'); material(materials, section.question.materials); card.append(materials); }
    const form = element('form'), options = element('div', '', 'agent-options'); form.dataset.section = section.id;
    const key = 'draft:' + lessonId + ':' + section.id + ':choice';
    let selected; try { selected = JSON.parse(api.storage.get(key) || 'null'); } catch (_) { selected = api.storage.get(key); }
    for (const [index, text] of (section.question.options || []).entries()) {
      const label = element('label'), choice = element('input'); choice.type = section.question.type === 'multi_choice' ? 'checkbox' : 'radio';
      choice.name = 'choice-' + section.id; choice.value = String(index); choice.checked = Array.isArray(selected) ? selected.includes(choice.value) : selected === choice.value;
      label.append(choice, element('span', String.fromCharCode(65 + index) + '. ' + text)); options.append(label);
      choice.addEventListener('change', () => { api.storage.set(key, JSON.stringify([...options.querySelectorAll('input:checked')].map(node => node.value)));
        if (selectedQuestion !== section.id) selectQuestion(section.id); });
    }
    form.append(options);
    const toolbar = element('div', '', 'question-actions'), submit = element('button', options.children.length ? '提交答案' : '在下方回答');
    submit.type = options.children.length ? 'submit' : 'button';
    if (!options.children.length) submit.onclick = () => selectQuestion(section.id, true);
    const unknown = element('button', '暂时不会', 'secondary'), skip = element('button', '跳过', 'secondary'); unknown.type = skip.type = 'button';
    unknown.onclick = () => sendAnswer(section.id, 'unknown'); skip.onclick = () => sendAnswer(section.id, 'skip');
    const note = element('p', '', 'question-note'); note.setAttribute('role', 'status');
    toolbar.append(submit, unknown, skip); form.append(toolbar, note); card.append(form);
    form.onsubmit = event => { event.preventDefault(); sendAnswer(section.id, 'answer'); };
  }
  function makeSection(section) {
    const card = element('article', '', 'chat-section'); card.dataset.sectionId = section.id; card.dataset.sectionVersion = section.version; card.dataset.area = areaFor(section);
    card.append(element('p', section.kind === 'summary' ? 'AI · 小结' : isQuestion(section) ? 'AI · ' + (areaFor(section) === 'practice' ? '练习' : '课堂提问') :
      section.presentation === 'example' || section.legacyKind === 'example' ? 'AI · 示范例题' : 'AI · 讲解', 'chat-author'), element('h2', section.title));
    if (isQuestion(section)) questionCard(section, card);
    else { const body = element('div', '', 'lesson-body'); api.richText(body, section.body || ''); card.append(body); }
    return card;
  }
  function makeMessage(record) {
    const node = element('article', '', 'chat-message ' + record.role);
    node.append(element('p', record.role === 'user' ? '你' : 'AI 老师', 'chat-author'), element('div', '', 'chat-message-body'), element('p', '', 'chat-partial'));
    return node;
  }
  function updateMessage(node, record) {
    if (node.dataset.rawText !== record.text) { node.dataset.rawText = record.text;
      if (record.role === 'assistant') api.richText(node.querySelector('.chat-message-body'), record.text);
      else node.querySelector('.chat-message-body').textContent = record.text; }
    const note = node.querySelector('.chat-partial'); note.hidden = !record.partial || busy && record.taskId === watchingTask;
    note.textContent = '这段回复未完整完成，已接收内容保留，不作为新的可靠评价。';
  }
  function makeEvaluation(record) {
    const card = element('article', '', 'chat-evaluation'); card.dataset.area = record.view || activeView;
    card.append(element('p', 'AI · 本次作答评价', 'chat-author'), element('h2', record.credible && record.score !== null && record.score !== undefined ?
      '本次作答 · ' + record.score + ' 分' : '本次反馈 · 尚无可靠分数'));
    const body = element('div'); api.richText(body, record.feedback || ''); card.append(body);
    if (record.uncertainty) card.append(element('p', record.uncertainty, 'agent-note'));
    return card;
  }
  function putRecord(key, record) {
    records.set(key, record); let node = nodes.get(key);
    if (!node) { node = record.type === 'section' ? makeSection(record.section) : record.type === 'evaluation' ? makeEvaluation(record) : makeMessage(record);
      node.dataset.entryKey = key; nodes.set(key, node); }
    if (record.type === 'message') updateMessage(node, record);
    return node;
  }
  function render() {
    mutate(() => {
      $('lesson-title').textContent = lesson.title; $('lesson-purpose').textContent = lesson.purpose;
      const order = [], used = new Set(), timeline = lesson.timeline || [];
      const referenced = new Set(timeline.filter(item => item.type === 'section').map(item => item.sectionId));
      let archive = nodes.get('saved-materials');
      if (!archive) { archive = element('details', '', 'saved-materials'); archive.dataset.entryKey = 'saved-materials';
        archive.append(element('summary', '已保存课件 · 随时回看')); nodes.set('saved-materials', archive); }
      const savedSections = lesson.sections.filter(section => areaFor(section) === activeView && !referenced.has(section.id) && section.kind !== 'summary');
      for (const section of savedSections) {
        const key = 'section:' + section.id;
        if (nodes.has(key) && nodes.get(key).dataset.sectionVersion !== String(section.version)) { nodes.get(key).remove(); nodes.delete(key); }
        archive.append(putRecord(key, {type:'section', section})); used.add(key);
      }
      archive.hidden = !savedSections.length; order.push(archive);
      // 旧交流按原记录保留；相同文字可能来自不同轮次，不能按正文去重。
      for (const section of lesson.sections.filter(section => areaFor(section) === activeView)) for (const [index, item] of (section.legacyDialog || []).entries()) {
        const key = 'legacy:' + section.id + ':' + index; order.push(putRecord(key, {...item, type:'message'})); used.add(key);
      }
      for (const item of timeline) {
        if (item.type === 'section') {
          const section = sectionFor(item.sectionId); if (!section || areaFor(section) !== activeView) continue;
          const key = 'section:' + section.id; if (used.has(key)) continue;
          if (nodes.has(key) && nodes.get(key).dataset.sectionVersion !== String(section.version)) { nodes.get(key).remove(); nodes.delete(key); }
          order.push(putRecord(key, {type:'section', section})); used.add(key);
        } else if (item.type === 'evaluation') {
          if ((item.view || 'learn') !== activeView) continue;
          const evaluation = lesson.evaluations.find(value => value.id === item.id); if (!evaluation) continue;
          const key = 'evaluation:' + item.id; order.push(putRecord(key, {...evaluation, type:'evaluation'})); used.add(key);
        } else {
          if ((item.view || 'learn') !== activeView) continue;
          const key = item.role === 'user' ? item.taskId + ':user' : item.id;
          order.push(putRecord(key, item)); used.add(key);
        }
      }
      // 流式文字和刚提交的输入在服务端刷新前保留；同一任务、同一步骤始终复用一个消息节点。
      for (const [key, record] of records) if (record.optimistic && (record.view || activeView) === activeView && !used.has(key)) {
        if (key.endsWith(':fallback') && timeline.some(item => item.type === 'message' && item.role === 'assistant' && item.taskId === record.taskId)) continue;
        order.push(nodes.get(key)); used.add(key);
      }
      for (const evaluation of lesson.evaluations || []) { const key = 'evaluation:' + evaluation.id;
        if (timeline.some(item => item.type === 'evaluation' && item.id === evaluation.id)) continue;
        const section = sectionFor(evaluation.sectionId); if (section && areaFor(section) !== activeView) continue;
        if (!used.has(key)) { order.push(putRecord(key, {...evaluation, type:'evaluation'})); used.add(key); } }
      let cursor = root.firstChild;
      for (const node of order) { if (node !== cursor) root.insertBefore(node, cursor); cursor = node.nextSibling; }
      for (const [key, node] of nodes) if (key !== 'saved-materials' && !used.has(key) && !key.startsWith('summary:')) { node.remove(); nodes.delete(key); records.delete(key); }
      $('lesson-completion').hidden = activeView !== 'summary';
      if (activeView === 'summary') { $('lesson-summary').open = true; renderSummary(); }
      paintNextStep(); paintTarget(); setDisabled();
    });
    if (initialPosition && (focus().sectionId || admissionError || !lesson.teachingTaskId)) {
      initialPosition = false; let saved;
      try { saved = JSON.parse(api.storage.get(pageKey('scroll')) || 'null'); } catch (_) { /* 默认定位当前课堂。 */ }
      if (saved) restorePosition(saved); else locate(activeView, false);
    }
    if (lesson.teachingTaskId && !admissionError && lesson.teachingTaskId !== watchingTask) observe(lesson.teachingTaskId);
  }
  function paintNextStep() {
    const prompt = $('lesson-next-step'), response = focus().responseTaskId === lesson.teachingTaskId ? sectionFor(focus().responseSectionId) : null;
    const latest = (lesson.timeline || []).filter(item => item.taskId === lesson.teachingTaskId && item.type === 'section').at(-1);
    const target = response ? areaFor(response) : latest ? areaFor(sectionFor(latest.sectionId)) : lesson.teachingTaskView;
    prompt.replaceChildren(); prompt.hidden = !target || target === activeView;
    if (prompt.hidden) return;
    prompt.append(element('span', 'AI 已保存下一步内容。'));
    const link = element('a', '进入' + {learn:'讲解', practice:'练习', summary:'小结'}[target] + ' →'); link.href = lessonHref(target); prompt.append(link);
  }
  function renderSummary() {
    const parent = $('lesson-summary-content');
    for (const section of lesson.sections.filter(section => section.kind === 'summary' && nodes.get('section:' + section.id)?.parentElement !== root)) {
      const key = 'summary:' + section.id;
      if (nodes.has(key) && nodes.get(key).dataset.sectionVersion !== String(section.version)) { nodes.get(key).remove(); nodes.delete(key); }
      if (!nodes.has(key)) { const node = makeSection(section); node.dataset.entryKey = key; nodes.set(key, node); } parent.append(nodes.get(key));
    }
    for (const [key, node] of nodes) if (key.startsWith('summary:') && nodes.get('section:' + key.slice(8))?.parentElement === root) { node.remove(); nodes.delete(key); }
    if (!parent.dataset.references) {
      parent.dataset.references = '1';
      for (const reference of lesson.references || []) { const card = element('article', '', 'chat-reference');
        card.append(element('h3', reference.title || reference.name || reference.source || '参考资料'), element('p', reference.description || ''));
        const href = reference.url || reference.href; if (/^https?:\/\//i.test(href || '')) { const link = element('a', '查看资料 ↗'); link.href = href; link.target = '_blank'; link.rel = 'noreferrer'; card.append(link); } parent.append(card); }
      for (const record of lesson.historicalAssessments || []) { const card = element('article', '', 'chat-evaluation'); card.append(element('h3', record.kind === 'quiz' ? '历史测验记录' : '历史作答记录'));
        if (record.score !== undefined && record.score !== null) card.append(element('p', '原保存分数：' + record.score + (record.total !== undefined ? ' / ' + record.total : '')));
        if (record.response) card.append(element('p', record.response)); if (record.feedback) { const body = element('div'); api.richText(body, record.feedback); card.append(body); }
        for (const [index, answer] of (record.answers || []).entries()) card.append(element('p', '第 ' + (index + 1) + ' 题原提交：' + (answer === null ? '未作答' : answer === 'unknown' ? '暂时不会' : typeof answer === 'number' ? '选项 ' + String.fromCharCode(65 + answer) : String(answer))));
        parent.append(card); }
    }
    $('completion-note').textContent = lesson.completed ? 'AI 已根据实际记录确认本节完成，原作答和交流继续保留。' :
      (lesson.evaluations || []).length ? '完成情况由 AI 结合实际作答和评价判断。' : '本节尚无足够的实际作答评价；阅读和追问不表示掌握。';
  }
  function visible(node) {
    if (!node?.isConnected || node.closest('details:not([open])') || node.closest('[hidden]')) return false;
    const bounds = node.getBoundingClientRect(), viewport = scroll.getBoundingClientRect();
    return bounds.bottom > viewport.top && bounds.top < viewport.bottom && bounds.right > viewport.left && bounds.left < viewport.right && bounds.height > 0;
  }
  function exposeVisible() {
    if (!lesson || admissionError) return;
    const sections = [...scroll.querySelectorAll('[data-section-id]')].filter(visible).map(node => ({id:node.dataset.sectionId, version:Number(node.dataset.sectionVersion)}));
    const fresh = sections.filter(section => !shownSections.has(section.id + ':' + section.version));
    if (fresh.length) { fresh.forEach(section => shownSections.add(section.id + ':' + section.version)); api.recordSections(lessonId, fresh); }
    for (const [key, record] of records) if (record.role === 'assistant' && record.taskId && record.sequences?.length && visible(nodes.get(key)))
      api.recordMessages(record.taskId, Math.max(...record.sequences), record.sequences);
  }
  function setDisabled() {
    const locked = busy || sending || Boolean(admissionError);
    for (const control of scroll.querySelectorAll('form button, form input')) control.disabled = locked;
    for (const id of ['lesson-send', 'explain-again', 'continue-teaching', 'finish-lesson']) $(id).disabled = locked;
  }
  function paintState(task) {
    busy = !task.paused && ['pending', 'running'].includes(task.status);
    const labels = {pending:'等待 AI 处理', running:'AI 正在结合你的回答互动', waiting_student:'你可以继续回答或追问', ready:'AI 已完成本次互动',
      paused:'AI 已暂停，输入和已有结果保留', cancelled:'已停止本次回答；中断输出不作为正式评价', superseded:'学习记录已有更新，旧输出未应用'};
    $('lesson-task-status').textContent = task.paused ? labels.paused : task.status === 'failed' ? api.failureMessage(task) : labels[task.status] || task.status;
    $('lesson-stop').hidden = !busy; $('lesson-retry').hidden = !['failed', 'cancelled', 'superseded'].includes(task.status); setDisabled();
  }
  function observe(taskId, restart = false) {
    if (watchingTask === taskId && !restart) return;
    unsubscribe?.(); watchingTask = taskId; const epoch = ++watchEpoch;
    watchingView = lesson.teachingTaskView || (lesson.timeline || []).find(item => item.taskId === taskId && item.view)?.view || activeView;
    window.GangyiNavigation?.setContext({teachingTaskId:taskId});
    const afterSeq = Math.max(0, ...[...records.values()].filter(record => record.taskId === taskId).flatMap(record => record.sequences || []));
    unsubscribe = api.watch(taskId, {afterSeq,
      onEvent(event) {
        if (epoch !== watchEpoch) return false;
        if (event.type === 'action') { refreshLesson(); return false; }
        if (event.type !== 'delta' || event.field !== 'message' || watchingView !== activeView) return false;
        const key = taskId + ':step:' + event.step, previous = records.get(key);
        const record = {...previous, type:'message', role:'assistant', taskId, view:watchingView, id:key, step:event.step,
          text:(previous?.text || '') + event.text, sequences:[...(previous?.sequences || []), event.seq], partial:true, optimistic:true};
        mutate(() => { const node = putRecord(key, record); if (!node.isConnected) root.append(node); });
        return false; // 可见性检查单独记录已读，向上回看时不把新回复算作已看。
      },
      onState(task) { if (epoch !== watchEpoch) return; paintState(task);
        if (task.view) watchingView = task.view;
        if (['ready', 'waiting_student'].includes(task.status)) for (const scope of ['chat', 'finish', ...lesson.sections.filter(isQuestion).map(section => section.id)]) {
          const key = 'pending:' + lessonId + ':' + (scope === 'chat' ? scope + ':' + watchingView : scope);
          try { const pending = JSON.parse(api.storage.get(key) || 'null'); if (pending?.requestId === task.requestId) api.storage.remove(key); }
          catch (_) { /* 其他窗口的输入缓存不能被旧任务清除。 */ }
        }
        if (!['pending', 'running'].includes(task.status)) {
          if (watchingView === activeView && task.message && ![...records.values()].some(record => record.taskId === taskId && record.role === 'assistant'))
            mutate(() => root.append(putRecord(taskId + ':fallback', {type:'message', role:'assistant', taskId, text:task.message, optimistic:true})));
          for (const [key, record] of records) if (record.taskId === taskId && record.role === 'assistant') updateMessage(nodes.get(key), record);
          refreshLesson();
        }
      },
      onConnectionError(error) { if (epoch === watchEpoch) $('lesson-task-status').textContent = error; }
    });
  }
  async function refreshLesson() {
    if (refreshing) { refreshAgain = true; return; } refreshing = true;
    const revision = taskRevision;
    try { await api.flushExposure(); const saved = await api.request('/api/learning-agent/lesson?lessonId=' + encodeURIComponent(lessonId));
      if (revision !== taskRevision) { refreshAgain = true; return; }
      const previousFocus = focus().responseSectionId || focus().interactionSectionId, completed = lesson.completed;
      lesson = saved; render(); const nextFocus = focus().responseSectionId || focus().interactionSectionId;
      if (nextFocus && nextFocus !== previousFocus && !input.value) selectQuestion(nextFocus);
      if (lesson.completed && !completed) document.dispatchEvent(new CustomEvent('gangyi:lesson-finished'));
    } catch (error) { $('lesson-task-status').textContent = error.message; }
    finally { refreshing = false; if (refreshAgain) { refreshAgain = false; refreshLesson(); } }
  }
  async function sendEvent(event, sourceDraft, choiceState) {
    if (admissionError || busy || sending) return;
    sending = true; setDisabled();
    const scope = event.type === 'question_answer' ? event.sectionId : event.type === 'lesson_finish_request' ? 'finish' : 'chat';
    const pendingKey = 'pending:' + lessonId + ':' + (scope === 'chat' ? scope + ':' + activeView : scope);
    event = {...event, courseId:lesson.courseId, lessonId, view:activeView, ...reviewContext, requestId:api.id()};
    try { const pending = JSON.parse(api.storage.get(pendingKey) || 'null');
      if (pending && pending.type === event.type && pending.text === event.text && pending.action === event.action &&
        pending.sectionId === event.sectionId && pending.sectionVersion === event.sectionVersion && pending.view === activeView) event = pending;
    } catch (_) { /* 损坏缓存不改变本次输入。 */ }
    api.storage.set(pendingKey, JSON.stringify(event));
    try {
      exposeVisible(); const task = await api.submit(event);
      if (event.type !== 'lesson_finish_request')
        mutate(() => root.append(putRecord(task.id + ':user', {type:'message', role:'user', taskId:task.id, text:event.text, optimistic:true})));
      if (sourceDraft && api.storage.get(sourceDraft.key) === sourceDraft.text) {
        if (draftKey() === sourceDraft.key && input.value === sourceDraft.text) { input.value = ''; resizeInput(); }
        api.storage.set(sourceDraft.key, '');
      }
      if (choiceState && api.storage.get(choiceState.key) === choiceState.value) { api.storage.set(choiceState.key, '[]'); for (const choice of choiceState.form.querySelectorAll('input')) choice.checked = false; }
      taskRevision++; lesson.teachingTaskId = task.id; lesson.teachingTaskView = activeView; paintState(task); observe(task.id); refreshLesson();
    } catch (error) { $('lesson-task-status').textContent = error.message; }
    finally { sending = false; setDisabled(); }
  }
  function sendAnswer(id, action, fromComposer = false) {
    const section = sectionFor(id), form = nodes.get('section:' + id)?.querySelector('form'); if (!isQuestion(section)) return;
    const choices = [...(form?.querySelectorAll('input:checked') || [])];
    const explanation = selectedQuestion === id ? input.value.trim() : readDraft('draft:' + lessonId + ':' + id, section).trim();
    const text = action === 'unknown' ? '暂时不会' : action === 'skip' ? '我跳过这道题' :
      (!fromComposer && choices.length ? '我选' + choices.map(choice => (Number(choice.value) + 1) + '：' + section.question.options[Number(choice.value)]).join('、') + '\n' : '') + explanation;
    if (!text.trim()) { if (form) form.querySelector('.question-note').textContent = '请先选择选项，或在下方写出你的答案。'; selectQuestion(id, true); return; }
    sendEvent({type:'question_answer', action, text, sectionId:id, sectionVersion:section.version},
      action === 'answer' ? {key:'draft:' + lessonId + ':' + id, text:selectedQuestion === id ? input.value : readDraft('draft:' + lessonId + ':' + id, section)} : null,
      !fromComposer && action === 'answer' && form ? {key:'draft:' + lessonId + ':' + id + ':choice', value:api.storage.get('draft:' + lessonId + ':' + id + ':choice'), form} : null);
  }
  function sendTeaching(text, fromComposer = false) {
    if (!text.trim()) { input.focus(); return; }
    if (fromComposer && selectedQuestion) { sendAnswer(selectedQuestion, 'answer', true); return; }
    sendEvent({type:'chat', text, ...(focus().sectionId ? {sectionId:focus().sectionId} : {})}, fromComposer ? {key:draftKey(), text:input.value} : null);
  }
  async function load() {
    document.body.classList.add('gy-chat-classroom');
    if (!lessonId) throw new Error('缺少课时标识，请从课程入口进入。');
    lesson = await api.request('/api/learning-agent/lesson?lessonId=' + encodeURIComponent(lessonId));
    if (lesson.status !== 'ready') throw new Error('课时尚未完整保存，请等待 AI 备课后进入。');
    try {
      if (!lesson.entered) await api.control({command:'enter_lesson', lessonId});
      if (activeView === 'learn' && !lesson.initialTeachingTaskId) { await api.control({command:'start_lesson', lessonId});
        lesson = await api.request('/api/learning-agent/lesson?lessonId=' + encodeURIComponent(lessonId)); }
    } catch (error) { admissionError = error.message; }
    const context = {courseId:lesson.courseId, lessonId, phaseIndex:lesson.phaseIndex, topicIndex:lesson.topicIndex, topicId:lesson.topicId, teachingTaskId:lesson.teachingTaskId};
    window.GangyiNavigation?.setContext(context); document.dispatchEvent(new CustomEvent('gangyi:lesson-context', {detail:context}));
    api.storage.set('current-lesson', JSON.stringify({lessonId, courseId:lesson.courseId, href:lessonHref('learn')}));
    const savedTarget = api.storage.get(pageKey('composer')) || (activeView === 'learn' ? api.storage.get('composer:' + lessonId) : '');
    selectedQuestion = savedTarget === 'chat' ? '' : [savedTarget, focus().responseSectionId,
      activeView === 'practice' ? focus().practiceSectionId : focus().interactionSectionId]
      .find(id => isQuestion(sectionFor(id)) && areaFor(sectionFor(id)) === activeView) || '';
    if (!isQuestion(sectionFor(selectedQuestion)) || areaFor(sectionFor(selectedQuestion)) !== activeView) selectedQuestion = '';
    if (!selectedQuestion && activeView === 'learn' && !api.storage.get(pageKey('chat-draft')))
      api.storage.set(pageKey('chat-draft'), api.storage.get('chat-draft:' + lessonId));
    input.value = readDraft(draftKey(), sectionFor(selectedQuestion)); loaded = true; resizeInput();
    $('page-status').textContent = admissionError ? admissionError + '。你可以回看已保存内容；继续学习请重新备课。' :
      {learn:'讲解与随堂互动', practice:'集中练习与作答交流', summary:'小结、检查点与完成判断'}[activeView] + ' · 本页独立保存交流和草稿，AI 共享本节学习记录。';
    if (admissionError) { $('page-status').classList.add('agent-error'); const link = element('a', '按最新学习情况重新备课 →');
      link.href = '/agent-prepare.html?' + new URLSearchParams({courseId:lesson.courseId, ...(lesson.topicId ? {topicId:lesson.topicId} : {})}); $('page-status').append(link); }
    render(); for (const link of document.querySelectorAll('[data-lesson-link]')) { link.href = lessonHref(link.dataset.lessonLink);
      if (link.dataset.lessonLink === activeView) link.setAttribute('aria-current', 'page'); }
    if (!lesson.teachingTaskId) $('lesson-task-status').textContent = admissionError || '你可以继续回答或追问';
  }
  input.addEventListener('input', () => { saveDraft(); resizeInput(); });
  input.addEventListener('keydown', event => { if (event.key === 'Enter' && !event.shiftKey && !event.isComposing && !busy && !sending) { event.preventDefault(); sendTeaching(input.value, true); } });
  $('lesson-chat').onsubmit = event => { event.preventDefault(); sendTeaching(input.value, true); };
  $('composer-chat').onclick = () => selectQuestion('', true);
  $('explain-again').onclick = () => sendTeaching('请围绕当前这一段换一种讲法，结合我已有的反馈解释。');
  $('continue-teaching').onclick = () => sendTeaching('请结合本节实际反馈判断下一步；如果适合继续，请选择下一个教学焦点。');
  $('lesson-stop').onclick = async () => { try { const task = await api.control({command:'cancel', taskId:watchingTask}); paintState(task); observe(task.id, true); } catch (error) { $('lesson-task-status').textContent = error.message; } };
  $('lesson-retry').onclick = async () => { try { const task = await api.control({command:'retry', taskId:watchingTask}); paintState(task); observe(task.id, true); } catch (error) { $('lesson-task-status').textContent = error.message; } };
  $('finish-lesson').onclick = () => sendEvent({type:'lesson_finish_request', text:'请根据本节真实作答和已有评价判断完成情况，证据不足时明确说明，不使用固定通过率。'});
  $('prepare-next').onclick = () => { location.href = '/agent-prepare.html?' + new URLSearchParams({courseId:lesson.courseId, requestId:api.id()}); };
  $('lesson-latest').onclick = () => { scroll.scrollTop = scroll.scrollHeight; exposeVisible(); };
  for (const button of document.querySelectorAll('[data-lesson-link]')) button.onclick = () => locate(button.dataset.lessonLink);
  let scrollFrame;
  scroll.addEventListener('scroll', () => { if (rendering || !loaded || initialPosition) return; cancelAnimationFrame(scrollFrame);
    scrollFrame = requestAnimationFrame(() => { api.storage.set(pageKey('scroll'), JSON.stringify(readingPosition())); $('lesson-latest').hidden = nearBottom(); exposeVisible(); }); });
  scroll.addEventListener('toggle', () => requestAnimationFrame(exposeVisible), true);
  new ResizeObserver(() => { if (loaded) exposeVisible(); }).observe(scroll);
  new ResizeObserver(() => document.body.style.setProperty('--classroom-composer-height', $('lesson-chat-panel').offsetHeight + 'px')).observe($('lesson-chat-panel'));
  document.addEventListener('gangyi:agent-control', event => { const {command, task, all} = event.detail || {};
    if (watchingTask && ['pause', 'resume', 'retry', 'cancel'].includes(command) && (all || task?.id === watchingTask)) observe(watchingTask, true); });
  window.addEventListener('pagehide', () => { saveDraft(); if (loaded && !initialPosition) api.storage.set(pageKey('scroll'), JSON.stringify(readingPosition())); unsubscribe?.(); watchEpoch++; });
  load().catch(error => { $('page-status').textContent = error.message; $('page-status').classList.add('agent-error'); $('lesson-task-status').textContent = '课堂暂未就绪'; });
})();

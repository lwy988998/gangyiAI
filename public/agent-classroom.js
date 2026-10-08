/* 题目在上、作答在下；只有真实主控事件可产生教师回复。 */
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
  const status = document.getElementById('page-status');
  const sessions = new Map(); let lesson, revision = 0, refreshing = false, admissionError = '';
  const view = document.querySelector('[data-lesson-view]')?.dataset.lessonView || 'learn';
  let readingId = api.storage.get(`reading:${lessonId}`), practiceId = api.storage.get(`practice:${lessonId}`);
  function message(root, role, text) {
    const node = document.createElement('div'); node.className = `agent-message ${role}`;
    if (role === 'assistant') api.richText(node, text); else node.textContent = text;
    root.append(node); node.dataset.rawText = text; return node;
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
  function compactDialogue(root) {
    const messages=[...root.querySelectorAll(':scope > .agent-message')];
    const user=messages.findLast(node=>node.classList.contains('user'));
    const teacher=messages.findLast(node=>node.classList.contains('assistant'));
    const older=messages.filter(node=>node!==user&&node!==teacher);
    if(!older.length)return;
    const card=root.closest('.agent-card')||root.parentElement;
    let history=card.querySelector('.dialogue-history');
    if(!history){history=document.createElement('details');history.className='dialogue-history';const label=document.createElement('summary');label.textContent='历史交流';history.append(label);root.after(history)}
    for(const node of older)history.append(node);
  }
  function observe(task, root, note, stop, retry, form) {
    const stopPrevious = sessions.get(root); stopPrevious?.();
    const steps = new Map(); let currentTaskId = task.id;
    if (root.dataset.taskId === task.id) for (const node of root.querySelectorAll(':scope > [data-step]')) steps.set(Number(node.dataset.step), {text: node.dataset.rawText || '', node});
    window.GangyiNavigation?.setContext({teachingTaskId: task.id});
    const previousSequence = root.dataset.taskId === currentTaskId ? Number(root.dataset.sequence || 0) : 0;
    root.dataset.taskId = currentTaskId;
    const unsubscribe = api.watch(currentTaskId, {
      afterSeq: previousSequence,
      onEvent(event) {
        root.dataset.sequence = event.seq;
        if (!root.isConnected || event.type !== 'delta' || event.field !== 'message') return false;
        if (!steps.has(event.step)) steps.set(event.step, { text: '', node: message(root, 'assistant', '') });
        const step = steps.get(event.step); step.node.dataset.step = event.step; step.node.dataset.taskId = currentTaskId; step.text += event.text; step.node.dataset.rawText = step.text; api.richText(step.node, step.text); return true;
      },
      onState(current) {
        const paused = current.paused || current.status === 'paused';
        const active = !paused && ['pending', 'running'].includes(current.status);
        if(!active)compactDialogue(root);
        if (form) for (const control of form.querySelectorAll('button')) control.disabled = active;
        stop.hidden = !active; retry.hidden = !['failed', 'paused', 'superseded', 'cancelled'].includes(current.status);
        const labels = { pending: '等待 AI 处理', running: 'AI 正在结合你的回答互动', waiting_student: '你可以继续回答或追问',
          ready: 'AI 已完成本次互动', failed: '等待 AI 更新，输入和已有结果已保留', paused: 'AI 已暂停',
          cancelled: '已停止本次回答；中断输出不作为正式评价', superseded: '学习记录已有更新，这次旧输出未应用' };
        note.textContent = paused ? 'AI 已暂停' : current.status === 'failed' ? api.failureMessage(current) : labels[current.status] || current.status;
        if (['ready', 'waiting_student', 'failed'].includes(current.status)) {
          if (steps.size === 0 && current.message) message(root, 'assistant', current.message);
          if (current.status !== 'failed' && form) for(const scope of new Set([form.dataset.section || 'chat','chat'])) { const key=`pending:${lessonId}:${scope}`; try { const pending=JSON.parse(api.storage.get(key)||'null'); if(pending?.requestId===current.requestId)api.storage.remove(key); } catch (_) { /* 其他窗口的请求缓存不能被旧回复清掉。 */ } }
          refreshLesson().catch(error => { status.textContent = error.message; });
        }
      },
      onConnectionError(error) { note.textContent = error; },
    });
    const restart = event => {
      const {command,task:next}=event.detail||{};
      if(root.isConnected&&['pause','resume','retry','cancel'].includes(command)&&next?.id===currentTaskId)
        observe(next,root,note,stop,retry,form);
    };
    document.addEventListener('gangyi:agent-control',restart);
    sessions.set(root,()=>{unsubscribe();document.removeEventListener('gangyi:agent-control',restart)});
    stop.onclick = async () => { try { await api.control({ command: 'cancel', taskId: currentTaskId }); } catch (error) { note.textContent = error.message; } };
    retry.onclick = async () => {
      try { const next = await api.control({ command: 'retry', taskId: currentTaskId }); observe(next, root, note, stop, retry, form); }
      catch (error) { note.textContent = error.message; }
    };
  }
  function questionCard(section, card) {
    card.dataset.sectionId = section.id; card.dataset.sectionVersion = section.version;
    const question = document.createElement('div'); question.className='agent-question'; api.richText(question, section.question.question); card.append(question);
    if (section.question.materials) {
      const materials = document.createElement('div'); materials.className = 'agent-materials';
      material(materials, section.question.materials); card.append(materials);
    }
    const form = document.createElement('form'); form.dataset.section = section.id;
    const options = document.createElement('div'); options.className = 'agent-options';
    for (const [index, text] of (section.question.options || []).entries()) {
      const label = document.createElement('label'), radio = document.createElement('input'), caption = document.createElement('span');
      radio.type = section.question.type === 'multi_choice' ? 'checkbox' : 'radio'; radio.name = `choice-${section.id}`; radio.value = index; caption.textContent = text;
      label.append(radio, caption); options.append(label);
    }
    const label = document.createElement('label'), input = document.createElement('textarea');
    input.id = `answer-${section.id}`; input.rows = 3; label.htmlFor = input.id; label.textContent = '你的答案、解题过程或追问';
    const draftKey = `draft:${lessonId}:${section.id}`; input.value = readDraft(draftKey, section);
    const selected = api.storage.get(`${draftKey}:choice`);
    for (const radio of options.querySelectorAll('input')) {
      let chosen; try { chosen = JSON.parse(selected); } catch (_) { chosen = selected; }
      radio.checked = Array.isArray(chosen) ? chosen.includes(radio.value) : radio.value === chosen;
      radio.addEventListener('change', () => api.storage.set(`${draftKey}:choice`, JSON.stringify([...options.querySelectorAll('input:checked')].map(item => item.value))));
    }
    input.addEventListener('input', () => api.storage.set(draftKey, input.value));
    const submit = document.createElement('button'); submit.type = 'submit'; submit.textContent = '发送给 AI';
    const unknown = document.createElement('button'); unknown.type = 'button'; unknown.className = 'secondary'; unknown.textContent = '暂时不会';
    const skip = document.createElement('button'); skip.type = 'button'; skip.className = 'secondary'; skip.textContent = '跳过';
    form.append(options, label, input, submit, unknown, skip);
    const dialog = document.createElement('div'); dialog.className = 'agent-dialog';
    const rows = section.dialog?.length ? section.dialog : section.id === focus().responseSectionId ? (lesson.teachingDialog || []).filter(item => item.taskId === focus().responseTaskId) : [];
    for (const item of rows.slice(-2)) { const node = message(dialog, item.role, item.text); if (item.step !== undefined) node.dataset.step = item.step; if (item.partial) message(dialog, 'note', '这段讲解曾被中断，已展示内容保留，未形成新的可靠评价。'); }
    let history;
    if ((section.dialog || []).length > 2) {
      history = document.createElement('details'); history.className='dialogue-history'; const title = document.createElement('summary'); title.textContent = '本题历史交流'; history.append(title);
      for (const item of section.dialog.slice(0,-2)) message(history, item.role, item.text);
    }
    if (section.taskId) { dialog.dataset.taskId = section.taskId; dialog.dataset.sequence = section.afterSeq || 0; }
    const note = document.createElement('p'); note.className = 'agent-note'; note.setAttribute('role', 'status'); note.textContent = '你可以继续回答或追问';
    const stop = document.createElement('button'), retry = document.createElement('button');
    stop.type = retry.type = 'button'; stop.textContent = '停止回答'; retry.textContent = '重试同次回答'; stop.hidden = retry.hidden = true;
    card.append(form, note, dialog, stop, retry); if(history)card.append(history);
    async function send(action) {
      if (admissionError) return;
      const choices = [...options.querySelectorAll('input:checked')];
      const text = action === 'unknown' ? '暂时不会' : action === 'skip' ? '我跳过这道题' :
        `${choices.length ? '我选' + choices.map(choice => `${Number(choice.value) + 1}：${section.question.options[Number(choice.value)]}`).join('、') + '\n' : ''}${input.value.trim()}`;
      if (!text.trim()) { note.textContent = '请先输入答案或选择一个选项。'; input.focus(); return; }
      submit.disabled = unknown.disabled = skip.disabled = true;
      try {
        let event = { type: 'question_answer', action, text, courseId: lesson.courseId, lessonId, view, ...reviewContext,
          sectionId: section.id, sectionVersion: section.version, requestId: api.id() };
        try {
          const pending = JSON.parse(api.storage.get(`pending:${lessonId}:${section.id}`) || 'null');
          if (pending && pending.text === text && pending.action === action && pending.sectionVersion === section.version) event = pending;
        } catch (_) { /* 无效临时缓存不能改变实际输入。 */ }
        api.storage.set(`pending:${lessonId}:${section.id}`, JSON.stringify(event));
        const submittedDraft = input.value, submittedChoice = api.storage.get(`${draftKey}:choice`);
        const task = await api.submit(event); message(dialog, 'user', text);
        if (action === 'answer' && input.value === submittedDraft && api.storage.get(draftKey) === submittedDraft && api.storage.get(`${draftKey}:choice`) === submittedChoice) { input.value = ''; for(const choice of choices)choice.checked=false; api.storage.remove(draftKey); api.storage.remove(`${draftKey}:choice`); }
        observe(task, dialog, note, stop, retry, form);
      } catch (error) { note.textContent = error.message; submit.disabled = unknown.disabled = skip.disabled = false; }
      finally { if (!sessions.has(dialog)) submit.disabled = unknown.disabled = skip.disabled = false; }
    }
    form.addEventListener('submit', event => { event.preventDefault(); send('answer'); });
    unknown.addEventListener('click', () => send('unknown')); skip.addEventListener('click', () => send('skip'));
    if (section.taskId && !admissionError)
      observe({ id: section.taskId }, dialog, note, stop, retry, form);
  }
  const element = (tag, text = '', className = '') => { const node = document.createElement(tag); node.textContent = text; node.className = className; return node; };
  const focus = () => lesson.teachingFocus || {};
  const questions = () => lesson.sections.filter(section => section.kind === 'question' && section.legacyKind !== 'example' &&
    (focus().responseView === 'practice' && section.id === focus().responseSectionId ||
    !['interaction','diagnostic'].includes(section.questionKind) && !['interaction','diagnostic'].includes(section.legacyKind)));
  const explanations = () => lesson.sections.filter(section => section.kind === 'explanation' && !['overview','example'].includes(section.legacyKind) && section.presentation !== 'example');
  function addBody(root, section) { const body = element('div', '', 'lesson-body'); body.dataset.sectionId=section.id; body.dataset.sectionVersion=section.version; api.richText(body, section.body || ''); root.append(body); }
  function exposeSections() {
    if(!lesson)return;
    const sections = [...document.querySelectorAll('#lesson-sections [data-section-id]')].filter(node => !node.closest('details:not([open])') && !node.closest('[hidden]')).map(node => ({id:node.dataset.sectionId,version:Number(node.dataset.sectionVersion || lesson.sections.find(section=>section.id===node.dataset.sectionId)?.version)}));
    api.recordSections(lessonId, sections);
  }
  function renderSections() {
    for (const close of sessions.values()) close(); sessions.clear();
    document.getElementById('lesson-title').textContent = lesson.title;
    document.getElementById('lesson-purpose').textContent = lesson.purpose;
    const root = document.getElementById('lesson-sections'); root.replaceChildren();
    const chatPanel = document.getElementById('lesson-chat-panel'), chatDialog = document.getElementById('lesson-dialog');
    chatDialog.replaceChildren(); const chatInput = document.getElementById('lesson-input');
    const input = document.activeElement; if (input?.tagName === 'TEXTAREA' && input.id === 'lesson-input') api.storage.set(`chat-draft:${lessonId}`, input.value);
    document.getElementById('question-navigation').hidden = view !== 'practice';
    document.getElementById('lesson-completion').hidden = view !== 'summary';
    chatPanel.hidden = view === 'summary';
    if (view === 'learn') {
      const steps = explanations(), selected = lesson.sections.find(section => section.kind === 'explanation' && section.id === readingId) || lesson.sections.find(section => section.kind === 'explanation' && section.id === focus().sectionId) || steps[0];
      const overview = lesson.sections.filter(section => section.legacyKind === 'overview');
      if (overview.length) { const details = element('details', '', 'course-more'); details.append(element('summary','本节目标与关键概念')); overview.forEach(section => addBody(details,section)); root.append(details); }
      if (selected) {
        readingId = selected.id;
        const card = element('section', '', 'agent-card current-explanation'); card.dataset.sectionId = selected.id;
        const position = element('p', steps.includes(selected) ? `${focus().sectionId && selected.id!==focus().sectionId ? '回看知识段' : '当前知识段'} · ${steps.indexOf(selected)+1} / ${steps.length}` : 'AI 当前讲解', 'agent-note');
        card.append(position, element('h2', selected.title)); addBody(card, selected);
        const tools = element('div', '', 'agent-toolbar');
        const different = element('button','换一种讲法'), proceed = element('button','请 AI 继续 →');
        different.type = proceed.type = 'button'; different.className = 'secondary';
        different.onclick = () => sendTeaching('请围绕当前这一段换一种讲法，结合我已有的反馈解释。');
        proceed.onclick = () => sendTeaching('请结合本节实际反馈判断下一步；如果适合继续，请选择下一个教学焦点。');
        tools.append(different, proceed); card.append(tools); root.append(card);
      }
      let sampleDetails; const samples = lesson.sections.filter(section => (section.legacyKind === 'example' || section.presentation === 'example'));
      if (samples.length) { const details=element('details','', 'course-more');details.append(element('summary','AI 示范例题')); for(const section of samples){details.append(element('h3',section.title));addBody(details,section)}sampleDetails=details; }
      const question = lesson.sections.find(section => section.id === (focus().responseView === 'learn' ? focus().responseSectionId : focus().interactionSectionId) && section.kind === 'question') || lesson.sections.find(section => section.kind === 'question' && (['interaction','diagnostic'].includes(section.questionKind) || ['interaction','diagnostic'].includes(section.legacyKind)));
      if (question && (!selected || !focus().sectionId || selected.id===focus().sectionId)) { const card=element('section','','agent-card current-interaction');card.append(element('h2',question.title));
        const active = lesson.teachingTaskId ? {...question,taskId:lesson.teachingTaskId,taskStatus:lesson.teachingTaskStatus,afterSeq:lesson.teachingAfterSeq,dialog:(lesson.teachingDialog||[]).filter(item=>item.taskId===lesson.teachingTaskId)} : question;
        questionCard(active,card);root.append(card);chatPanel.hidden=true; }
      if(sampleDetails)root.append(sampleDetails);
      const history = element('details','','course-more'); history.append(element('summary','回看已保存段落与交流'));
      for(const section of steps){const card=element('article','','lesson-history-entry');card.append(element('h3',section.title));addBody(card,section);const read=element('button','回看这一段');read.type='button';read.className='secondary';read.onclick=()=>{readingId=section.id;api.storage.set(`reading:${lessonId}`,readingId);renderSections()};card.append(read);history.append(card)}
      for(const item of lesson.teachingDialog||[])message(history,item.role,item.text);
      const current=element('button','回到 AI 当前段');current.type='button';current.onclick=()=>{readingId=focus().sectionId||'';api.storage.remove(`reading:${lessonId}`);renderSections()};history.append(current);root.append(history);
    } else if (view === 'practice') {
      const list = questions(), section = list.find(item=>item.id===practiceId) || list.find(item=>item.id===(focus().responseView==='practice'?focus().responseSectionId:focus().practiceSectionId)) || list[0];
      if(section){practiceId=section.id;api.storage.set(`practice:${lessonId}`,practiceId);const card=element('section','','agent-card current-practice');card.dataset.sectionId=section.id;card.append(element('h2',section.title));questionCard(section,card);root.append(card);chatPanel.hidden=true;
        const index=list.indexOf(section),previous=document.getElementById('previous-question'),next=document.getElementById('next-question');document.getElementById('question-position').textContent=`${index+1} / ${list.length}`;previous.disabled=index===0;previous.onclick=()=>{practiceId=list[index-1].id;renderSections()};next.textContent=index===list.length-1?'进入小结 →':'下一题 →';next.onclick=()=>{if(index===list.length-1)location.href=lessonHref('summary');else{practiceId=list[index+1].id;renderSections()}};
      }else{root.append(element('p','本节暂无已保存的集中练习题，可以向 AI 请求安排。','agent-note'));document.getElementById('question-navigation').hidden=true;}
    } else {
      const summary = lesson.sections.filter(section=>section.kind==='summary');
      for(const section of summary){const folded=['commonMistakes','resourceSummary'].includes(section.legacyKind);const card=element(folded?'details':'section','',folded?'course-more':'agent-card');card.append(element(folded?'summary':'h2',section.title));addBody(card,section);root.append(card)}
      if((lesson.references||[]).length){const references=element('details','','course-more');references.append(element('summary','参考资料'));for(const item of lesson.references){const row=element('article','','lesson-history-entry');row.append(element('h3',item.title||item.name||item.source||'资料'),element('p',item.description||''));const href=item.url||item.href;if(/^https?:\/\//i.test(href||'')){const link=element('a','查看资料 ↗');link.href=href;link.target='_blank';link.rel='noreferrer';row.append(link)}references.append(row)}root.append(references)}
      const evaluations = lesson.evaluations || [];
      root.append(element('p',evaluations.length?'以下记录来自实际作答与已保存的 AI 评价。':(lesson.historicalAssessments||[]).length?'这个知识点已有历史作答记录，本轮未产生新的评价。':'本节尚无足够的实际作答评价；阅读和追问不表示掌握。','agent-note'));
      const previousEvaluations=element('details','','course-more');previousEvaluations.append(element('summary','回看本节此前的 AI 评价'));
      for(const [index,result] of evaluations.entries()){const card=element('article','','agent-card');card.append(element('h2',result.credible&&result.score!==null?`本次作答 · ${result.score} 分`:'本次反馈 · 尚无可靠分数'));message(card,'user',result.response||'');message(card,'assistant',result.feedback||'');card.append(element('p',result.uncertainty||'','agent-note'));(index===evaluations.length-1?root:previousEvaluations).append(card)}
      if(evaluations.length>1)root.append(previousEvaluations);
      if((lesson.historicalAssessments||[]).length){const history=element('details','','course-more');history.append(element('summary','此前保存的作答与测验'));for(const record of lesson.historicalAssessments){const card=element('article','','lesson-history-entry');card.append(element('h3',record.kind==='quiz'?'历史测验记录':'历史作答记录'));if(record.score!==null&&record.score!==undefined)card.append(element('p',`原保存分数：${record.score}${record.total!==undefined?' / '+record.total:''}`));if(record.response)message(card,'user',record.response);if(record.feedback)message(card,'assistant',record.feedback);for(const [index,answer] of (record.answers||[]).entries()){const text=answer===null?'未作答':answer==='unknown'?'暂时不会':typeof answer==='number'?`选项 ${String.fromCharCode(65+answer)}`:String(answer);card.append(element('p',`第 ${index+1} 题原提交：${text}`))}history.append(card)}root.append(history)}
    }
    chatInput.value = readDraft(`chat-draft:${lessonId}`);
    const latest = (lesson.teachingDialog || lesson.dialog || []).slice(-2);
    for(const item of latest){const node=message(chatDialog,item.role,item.text);if(item.step!==undefined)node.dataset.step=item.step;}
    const chatNote=element('p','','agent-note'),chatStop=element('button','停止'),chatRetry=element('button','重试');chatNote.setAttribute('role','status');chatStop.type=chatRetry.type='button';chatStop.hidden=chatRetry.hidden=true;chatDialog.append(chatNote,chatStop,chatRetry);
    if(!admissionError && !chatPanel.hidden && lesson.teachingTaskId){chatDialog.dataset.taskId=lesson.teachingTaskId;chatDialog.dataset.sequence=lesson.teachingAfterSeq||0;observe({id:lesson.teachingTaskId},chatDialog,chatNote,chatStop,chatRetry,document.getElementById('lesson-chat'));}
    if(admissionError){
      for(const input of document.querySelectorAll('#lesson-sections form input, #lesson-sections form textarea, #lesson-sections form button, #lesson-chat input, #lesson-chat textarea, #lesson-chat button, .current-explanation .agent-toolbar button'))input.disabled=true;
      document.getElementById('finish-lesson').disabled=true;
    }
    document.dispatchEvent(new CustomEvent('gangyi:layout')); exposeSections();
  }
  async function refreshLesson() {
    if(refreshing)return;refreshing=true;const requestRevision=++revision;
    try{await api.flushExposure();const saved=await api.request(`/api/learning-agent/lesson?lessonId=${encodeURIComponent(lessonId)}`);if(requestRevision!==revision)return;
      const changed=JSON.stringify(saved.teachingFocus)!==JSON.stringify(lesson.teachingFocus)||saved.sections.length!==lesson.sections.length||saved.completed!==lesson.completed;
      if(saved.teachingFocus?.sectionId!==lesson.teachingFocus?.sectionId){readingId='';api.storage.remove(`reading:${lessonId}`)}
      if(saved.teachingFocus?.responseSectionId!==lesson.teachingFocus?.responseSectionId || saved.teachingFocus?.practiceSectionId!==lesson.teachingFocus?.practiceSectionId){practiceId='';api.storage.remove(`practice:${lessonId}`)}
      lesson=saved;if(changed){renderSections();if(saved.completed)document.dispatchEvent(new CustomEvent('gangyi:lesson-finished'))}
    }finally{refreshing=false}
  }
  async function sendTeaching(text) {
    if(admissionError)return;
    const interactive=document.querySelector('.current-interaction'),form=interactive?.querySelector('form')||document.getElementById('lesson-chat'),input=form.querySelector('textarea'),dialog=interactive?.querySelector('.agent-dialog')||document.getElementById('lesson-dialog');
    const pendingKey=`pending:${lessonId}:chat`,draftKey=interactive?`draft:${lessonId}:${form.dataset.section}`:`chat-draft:${lessonId}`;
    if(!text.trim())return;const button=form.querySelector('button[type=submit]');button.disabled=true;
    try{let event={type:'chat',courseId:lesson.courseId,lessonId,view,text,requestId:api.id(),...reviewContext,...(view==='learn'&&readingId?{sectionId:readingId}:{})};
      try{const saved=JSON.parse(api.storage.get(pendingKey)||'null');if(saved&&saved.text===text&&saved.sectionId===event.sectionId)event=saved}catch(_){/* 损坏缓存不改变本次真实输入。 */}
      api.storage.set(pendingKey,JSON.stringify(event));const submittedDraft=input.value,task=await api.submit(event);
      if(input.value===submittedDraft&&api.storage.get(draftKey)===submittedDraft&&submittedDraft===text){input.value='';api.storage.remove(draftKey)}
      message(dialog,'user',text);if(!interactive)document.getElementById('lesson-chat-panel').hidden=false;
      const note=element('p','','agent-note'),stop=element('button','停止'),retry=element('button','重试');stop.type=retry.type='button';stop.hidden=retry.hidden=true;dialog.append(note,stop,retry);observe(task,dialog,note,stop,retry,form);
    }catch(error){status.textContent=error.message;button.disabled=false;}
  }
  async function load() {
    if(!lessonId)throw new Error('缺少课时标识，请从 AI 已备好的课程入口进入。');
    lesson=await api.request(`/api/learning-agent/lesson?lessonId=${encodeURIComponent(lessonId)}`);
    if(lesson.status!=='ready')throw new Error('课时尚未完成，请等待真实 AI 备课后进入。');
    // 入课失败时保留已保存课件供回看，教学操作仍须通过主控入课检查。
    try{
      if(!lesson.entered)await api.control({command:'enter_lesson',lessonId});
      if(view==='learn'&&!lesson.initialTeachingTaskId){await api.control({command:'start_lesson',lessonId});lesson=await api.request(`/api/learning-agent/lesson?lessonId=${encodeURIComponent(lessonId)}`)}
    }catch(error){admissionError=error.message;}
    for(const link of document.querySelectorAll('[data-lesson-link]')){link.href=lessonHref(link.dataset.lessonLink);if(link.dataset.lessonLink===view)link.setAttribute('aria-current','page')}
    const context={courseId:lesson.courseId,lessonId,phaseIndex:lesson.phaseIndex,topicIndex:lesson.topicIndex,topicId:lesson.topicId,teachingTaskId:lesson.teachingTaskId};
    window.GangyiNavigation?.setContext(context);document.dispatchEvent(new CustomEvent('gangyi:lesson-context',{detail:context}));
    api.storage.set('current-lesson',JSON.stringify({lessonId,courseId:lesson.courseId,href:lessonHref('learn')}));
    const form=document.getElementById('lesson-chat'),input=document.getElementById('lesson-input');input.addEventListener('input',()=>api.storage.set(`chat-draft:${lessonId}`,input.value));form.addEventListener('submit',event=>{event.preventDefault();sendTeaching(input.value)});
    document.getElementById('prepare-next').onclick=()=>{location.href='/agent-prepare.html?'+new URLSearchParams({courseId:lesson.courseId,requestId:api.id()})};
    document.getElementById('finish-lesson').onclick=async()=>{const button=document.getElementById('finish-lesson');button.disabled=true;try{const task=await api.submit({type:'lesson_finish_request',courseId:lesson.courseId,lessonId,...reviewContext,requestId:api.id(),text:'请根据本节真实作答和已有评价判断完成情况，证据不足时明确说明，不使用固定通过率。'});observe(task,document.getElementById('finish-dialog'),document.getElementById('finish-status'),document.getElementById('finish-stop'),document.getElementById('finish-retry'));}catch(error){document.getElementById('finish-status').textContent=error.message}finally{button.disabled=false}};
    renderSections();status.textContent=admissionError ? admissionError+'。你可以回看已保存内容；继续学习请重新备课。' : 'AI 根据真实反馈选择下一步；三页切换会保留课时和草稿。';
    if(admissionError){status.classList.add('agent-error');const link=element('a','请 AI 按最新情况重新备课 →');link.href='/agent-prepare.html?'+new URLSearchParams({courseId:lesson.courseId,...(lesson.topicId?{topicId:lesson.topicId}:{})});status.after(link);}
  }
  load().catch(error => { status.textContent = error.message; status.classList.add('agent-error'); if (lesson) { const link=document.createElement('a');link.textContent='请 AI 按最新情况重新备课 →';link.href='/agent-prepare.html?'+new URLSearchParams({courseId:lesson.courseId,...(lesson.topicId?{topicId:lesson.topicId}:{})});status.after(link); } });
  document.addEventListener('toggle', exposeSections, true);
  window.addEventListener('pagehide', () => { for (const close of sessions.values()) close(); });
})();

/* 题目在上、作答在下；只有真实主控事件可产生教师回复。 */
(() => {
  'use strict';
  const api = window.GangyiAgent, lessonId = new URLSearchParams(location.search).get('lessonId');
  const status = document.getElementById('page-status');
  const sessions = new Map(); let lesson;
  function message(root, role, text) {
    const node = document.createElement('div'); node.className = `agent-message ${role}`;
    if (role === 'assistant') api.richText(node, text); else node.textContent = text;
    root.append(node); return node;
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
  function observe(task, root, note, stop, retry, form) {
    const stopPrevious = sessions.get(root); stopPrevious?.();
    const steps = new Map(); let currentTaskId = task.id;
    const previousSequence = root.dataset.taskId === currentTaskId ? Number(root.dataset.sequence || 0) : 0;
    root.dataset.taskId = currentTaskId;
    const unsubscribe = api.watch(currentTaskId, {
      afterSeq: previousSequence,
      onEvent(event) {
        root.dataset.sequence = event.seq;
        if (event.type !== 'delta' || event.field !== 'message') return;
        if (!steps.has(event.step)) steps.set(event.step, { text: '', node: message(root, 'assistant', '') });
        const step = steps.get(event.step); step.text += event.text; api.richText(step.node, step.text);
      },
      onState(current) {
        const active = ['pending', 'running'].includes(current.status);
        if (form) for (const control of form.querySelectorAll('button')) control.disabled = active;
        stop.hidden = !active; retry.hidden = !['failed', 'paused', 'superseded', 'cancelled'].includes(current.status);
        const labels = { pending: '等待 AI 处理', running: 'AI 正在结合你的回答互动', waiting_student: '你可以继续回答或追问',
          ready: 'AI 已完成本次互动', failed: '等待 AI 更新，输入和已有结果已保留', paused: 'AI 已暂停',
          cancelled: '已停止本次回答；中断输出不作为正式评价', superseded: '学习记录已有更新，这次旧输出未应用' };
        note.textContent = labels[current.status] || current.status;
        if (current.status === 'ready' || current.status === 'waiting_student') {
          if (steps.size === 0 && current.message) message(root, 'assistant', current.message);
          if (form) api.storage.remove(`pending:${lessonId}:${form.dataset.section || 'chat'}`);
        }
      },
      onConnectionError(error) { note.textContent = error; },
    });
    sessions.set(root, unsubscribe);
    stop.onclick = async () => { try { await api.control({ command: 'cancel', taskId: currentTaskId }); } catch (error) { note.textContent = error.message; } };
    retry.onclick = async () => {
      try { const next = await api.control({ command: 'retry', taskId: currentTaskId }); observe(next, root, note, stop, retry, form); }
      catch (error) { note.textContent = error.message; }
    };
  }
  function questionCard(section, card) {
    const question = document.createElement('p'); api.richText(question, section.question.question); card.append(question);
    if (section.question.materials) {
      const materials = document.createElement('div'); materials.className = 'agent-materials';
      material(materials, section.question.materials); card.append(materials);
    }
    const form = document.createElement('form'); form.dataset.section = section.id;
    const options = document.createElement('div'); options.className = 'agent-options';
    for (const [index, text] of (section.question.options || []).entries()) {
      const label = document.createElement('label'), radio = document.createElement('input'), caption = document.createElement('span');
      radio.type = 'radio'; radio.name = `choice-${section.id}`; radio.value = index; caption.textContent = text;
      label.append(radio, caption); options.append(label);
    }
    const label = document.createElement('label'), input = document.createElement('textarea');
    input.id = `answer-${section.id}`; input.rows = 4; label.htmlFor = input.id; label.textContent = '你的答案、解题过程或追问';
    const draftKey = `draft:${lessonId}:${section.id}`; input.value = api.storage.get(draftKey);
    const selected = api.storage.get(`${draftKey}:choice`);
    for (const radio of options.querySelectorAll('input')) {
      radio.checked = radio.value === selected;
      radio.addEventListener('change', () => api.storage.set(`${draftKey}:choice`, radio.value));
    }
    input.addEventListener('input', () => api.storage.set(draftKey, input.value));
    const submit = document.createElement('button'); submit.type = 'submit'; submit.textContent = '发送给 AI';
    const unknown = document.createElement('button'); unknown.type = 'button'; unknown.className = 'secondary'; unknown.textContent = '暂时不会';
    const skip = document.createElement('button'); skip.type = 'button'; skip.className = 'secondary'; skip.textContent = '跳过';
    form.append(options, label, input, submit, unknown, skip);
    const dialog = document.createElement('div'); dialog.className = 'agent-dialog';
    for (const item of section.dialog || []) { message(dialog, item.role, item.text); if (item.partial) message(dialog, 'assistant', '这段讲解曾被中断，已展示内容保留，未形成新的可靠评价。'); }
    if (section.taskId) { dialog.dataset.taskId = section.taskId; dialog.dataset.sequence = section.afterSeq || 0; }
    const note = document.createElement('p'); note.className = 'agent-note'; note.setAttribute('role', 'status');
    const stop = document.createElement('button'), retry = document.createElement('button');
    stop.type = retry.type = 'button'; stop.textContent = '停止回答'; retry.textContent = '重试同次回答'; stop.hidden = retry.hidden = true;
    card.append(form, note, dialog, stop, retry);
    async function send(action) {
      const choice = form.querySelector('input[type="radio"]:checked');
      const text = action === 'unknown' ? '暂时不会' : action === 'skip' ? '我跳过这道题' :
        `${choice ? `我选${Number(choice.value) + 1}：${section.question.options[Number(choice.value)]}\n` : ''}${input.value.trim()}`;
      if (!text.trim()) { note.textContent = '请先输入答案或选择一个选项。'; input.focus(); return; }
      submit.disabled = unknown.disabled = skip.disabled = true;
      try {
        let event = { type: 'question_answer', action, text, courseId: lesson.courseId, lessonId,
          sectionId: section.id, sectionVersion: section.version, requestId: api.id() };
        try {
          const pending = JSON.parse(api.storage.get(`pending:${lessonId}:${section.id}`) || 'null');
          if (pending && pending.text === text && pending.action === action && pending.sectionVersion === section.version) event = pending;
        } catch (_) { /* 无效临时缓存不能改变实际输入。 */ }
        api.storage.set(`pending:${lessonId}:${section.id}`, JSON.stringify(event));
        const task = await api.submit(event); message(dialog, 'user', text); input.value = ''; api.storage.remove(draftKey); api.storage.remove(`${draftKey}:choice`);
        observe(task, dialog, note, stop, retry, form);
      } catch (error) { note.textContent = error.message; submit.disabled = unknown.disabled = skip.disabled = false; }
      finally { if (!sessions.has(dialog)) submit.disabled = unknown.disabled = skip.disabled = false; }
    }
    form.addEventListener('submit', event => { event.preventDefault(); send('answer'); });
    unknown.addEventListener('click', () => send('unknown')); skip.addEventListener('click', () => send('skip'));
    if (section.taskId && ['pending', 'running', 'failed', 'paused', 'superseded', 'cancelled'].includes(section.taskStatus))
      observe({ id: section.taskId }, dialog, note, stop, retry, form);
  }
  async function load() {
    if (!lessonId) throw new Error('缺少课时标识，请从 AI 已备好的课程入口进入。');
    lesson = await api.request(`/api/learning-agent/lesson?lessonId=${encodeURIComponent(lessonId)}`);
    if (!lesson.entered) await api.control({ command: 'enter_lesson', lessonId });
    if (lesson.status !== 'ready') throw new Error('课时尚未完成，请等待真实 AI 备课后进入。');
    document.getElementById('lesson-title').textContent = lesson.title;
    document.getElementById('lesson-purpose').textContent = lesson.purpose;
    const root = document.getElementById('lesson-sections');
    for (const section of lesson.sections) {
      const card = document.createElement('section'); card.className = 'agent-card';
      const title = document.createElement('h2'); title.textContent = section.title; card.append(title);
      if (section.kind === 'question') questionCard(section, card);
      else { const body = document.createElement('div'); api.richText(body, section.body); card.append(body); }
      root.append(card);
    }
    status.textContent = '课件由真实 AI 准备；题目先作答，再结合你的实际思路互动。';
    document.getElementById('prepare-next').onclick = () => {
      location.href = `/agent-prepare.html?courseId=${encodeURIComponent(lesson.courseId)}&requestId=${encodeURIComponent(api.id())}`;
    };
    const input = document.getElementById('lesson-input'), form = document.getElementById('lesson-chat');
    const chatDialog = document.getElementById('lesson-dialog');
    for (const item of lesson.dialog || []) message(chatDialog, item.role, item.text);
    const chatNote = document.createElement('p'), chatStop = document.createElement('button'), chatRetry = document.createElement('button');
    chatNote.setAttribute('role', 'status'); chatStop.type = chatRetry.type = 'button';
    chatStop.textContent = '停止回答'; chatRetry.textContent = '重试'; chatStop.hidden = chatRetry.hidden = true;
    chatDialog.append(chatNote, chatStop, chatRetry);
    if (lesson.taskId) {
      chatDialog.dataset.taskId = lesson.taskId; chatDialog.dataset.sequence = lesson.afterSeq || 0;
      if (['pending', 'running', 'failed', 'paused', 'superseded', 'cancelled'].includes(lesson.taskStatus))
        observe({ id: lesson.taskId }, chatDialog, chatNote, chatStop, chatRetry, form);
    }
    input.value = api.storage.get(`chat-draft:${lessonId}`);
    input.addEventListener('input', () => api.storage.set(`chat-draft:${lessonId}`, input.value));
    form.addEventListener('submit', async event => {
      event.preventDefault(); if (!input.value.trim()) return;
      const dialog = chatDialog, note = chatNote, stop = chatStop, retry = chatRetry;
      const submitButton = form.querySelector('button[type="submit"]'); if (submitButton) submitButton.disabled = true;
      try {
        const text = input.value; let pending = { type: 'chat', courseId: lesson.courseId, lessonId, text, requestId: api.id() };
        try { const saved = JSON.parse(api.storage.get(`pending:${lessonId}:chat`) || 'null'); if (saved && saved.text === text) pending = saved; } catch (_) { /* 保留真实输入，不信任损坏的临时缓存。 */ }
        api.storage.set(`pending:${lessonId}:chat`, JSON.stringify(pending));
        const task = await api.submit(pending);
        message(dialog, 'user', text); input.value = ''; api.storage.remove(`chat-draft:${lessonId}`);
        observe(task, dialog, note, stop, retry, form);
      } catch (error) { note.textContent = error.message; if (submitButton) submitButton.disabled = false; }
    });
  }
  load().catch(error => { status.textContent = error.message; status.classList.add('agent-error'); if (lesson) { const link=document.createElement('a');link.textContent='请 AI 按最新情况重新备课 →';link.href='/agent-prepare.html?'+new URLSearchParams({courseId:lesson.courseId,...(lesson.topicId?{topicId:lesson.topicId}:{})});status.after(link); } });
  window.addEventListener('pagehide', () => { for (const close of sessions.values()) close(); });
})();

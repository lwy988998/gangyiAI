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
        if (form) for (const control of form.querySelectorAll('button[type="submit"]')) control.disabled = active;
        stop.hidden = !active; retry.hidden = current.status !== 'failed' && current.status !== 'paused';
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
    input.addEventListener('input', () => api.storage.set(draftKey, input.value));
    const submit = document.createElement('button'); submit.type = 'submit'; submit.textContent = '发送给 AI';
    const unknown = document.createElement('button'); unknown.type = 'button'; unknown.className = 'secondary'; unknown.textContent = '暂时不会';
    const skip = document.createElement('button'); skip.type = 'button'; skip.className = 'secondary'; skip.textContent = '跳过';
    form.append(options, label, input, submit, unknown, skip);
    const dialog = document.createElement('div'); dialog.className = 'agent-dialog';
    for (const item of section.dialog || []) message(dialog, item.role, item.text);
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
        const task = await api.submit(event); message(dialog, 'user', text); input.value = ''; api.storage.remove(draftKey);
        observe(task, dialog, note, stop, retry, form);
      } catch (error) { note.textContent = error.message; }
      finally { unknown.disabled = skip.disabled = false; if (!sessions.has(dialog)) submit.disabled = false; }
    }
    form.addEventListener('submit', event => { event.preventDefault(); send('answer'); });
    unknown.addEventListener('click', () => send('unknown')); skip.addEventListener('click', () => send('skip'));
    if (section.taskId && ['pending', 'running', 'failed', 'paused'].includes(section.taskStatus))
      observe({ id: section.taskId }, dialog, note, stop, retry, form);
  }
  async function load() {
    if (!lessonId) throw new Error('缺少课时标识，请从 AI 已备好的课程入口进入。');
    lesson = await api.request(`/api/learning-agent/lesson?lessonId=${encodeURIComponent(lessonId)}`);
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
    input.value = api.storage.get(`chat-draft:${lessonId}`);
    input.addEventListener('input', () => api.storage.set(`chat-draft:${lessonId}`, input.value));
    form.addEventListener('submit', async event => {
      event.preventDefault(); if (!input.value.trim()) return;
      const dialog = document.getElementById('lesson-dialog'), note = document.createElement('p');
      const stop = document.createElement('button'), retry = document.createElement('button');
      stop.textContent = '停止回答'; retry.textContent = '重试'; retry.hidden = true; dialog.append(note, stop, retry);
      try {
        const text = input.value, task = await api.submit({ type: 'chat', courseId: lesson.courseId, text, requestId: api.id() });
        message(dialog, 'user', text); input.value = ''; api.storage.remove(`chat-draft:${lessonId}`);
        observe(task, dialog, note, stop, retry, form);
      } catch (error) { note.textContent = error.message; }
    });
  }
  load().catch(error => { status.textContent = error.message; status.classList.add('agent-error'); });
  window.addEventListener('pagehide', () => { for (const close of sessions.values()) close(); });
})();

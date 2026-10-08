(() => {
  'use strict';
  const params = new URLSearchParams(location.search);
  const context = {courseId: params.get('courseId') || '', phaseIndex: Number(params.get('phaseIndex') || 1), topicIndex: Number(params.get('topicIndex') || 1),
    ...(params.get('lessonTaskId') ? {lessonTaskId: params.get('lessonTaskId')} : {}),
    ...(params.has('review') ? {day: Number(params.get('review')), reviewId: params.get('reviewId') || ''} : {})};
  document.addEventListener('gangyi:lesson-context', event => Object.assign(context, event.detail));
  const clientId = crypto.randomUUID ? crypto.randomUUID() : Date.now() + '-' + Math.random().toString(36).slice(2);
  const byId = id => document.getElementById(id);
  const element = (tag, text, className = '') => { const node = document.createElement(tag); node.textContent = text; node.className = className; return node; };
  async function api(path, data, method = 'POST') {
    const response = await fetch(path, data === undefined ? undefined : {method, headers: {'Content-Type': 'application/json'}, body: JSON.stringify(data)});
    const result = await response.json();
    if (!response.ok) throw new Error(result.error || '请求未完成，请重试');
    return result;
  }
  const localTime = text => text ? new Date(text).toLocaleString('zh-CN') : '';
  const drafts = new Map();
  const activeSubmissions = new Set();
  const requestIdentity = () => crypto.randomUUID ? crypto.randomUUID() : clientId + '-' + Date.now() + '-' + Math.random().toString(36).slice(2);
  function readLocal(key, fallback = '') { try { return localStorage.getItem(key) || fallback; } catch (_) { return fallback; } }
  function saveLocal(key, value) { try { if (value) localStorage.setItem(key, value); else localStorage.removeItem(key); } catch (_) { /* 无法使用本地缓存时，输入仍保留在页面。 */ } }
  function formatPublic(node, text) {
    node.classList.add('chat-body');
    if (window.GangyiChat) node.innerHTML = window.GangyiChat.renderMarkdown(String(text || ''), {math: true});
    else node.textContent = String(text || '');
  }

  function renderDialogue(kind, item, index, base) {
    base = {...base, ...(item.lessonTaskId ? {lessonTaskId: item.lessonTaskId} : {})};
    const row = element('article', '', 'classroom-question ai-dialogue'); row.dataset.index = index; row.dataset.kind = kind;
    const title = element('h3', ''); formatPublic(title, item.question || item.task || item.content || item.title || '题目'); row.append(title);
    if (item.title && item.title !== item.question) row.prepend(element('p', item.title, 'question-title'));
    const materials = Array.isArray(item.materials) ? item.materials.join('\n\n') : item.materials;
    if (materials) { const material = element('div', '', 'question-materials'); formatPublic(material, typeof materials === 'string' ? materials : JSON.stringify(materials)); row.append(material); }
    const history = element('div', '', 'ai-dialogue-history'); history.setAttribute('aria-label', '本题对话记录');
    const choiceBox = element('div', '', 'classroom-options');
    (item.options || []).forEach((text, choice) => {
      const label = element('label', ''); const radio = document.createElement('input'); radio.type = 'radio';
      radio.name = 'q-' + kind + '-' + index + '-' + (base.lessonTaskId || 'original') + '-' + (item.questionId || 'question'); radio.value = choice;
      if (item.studentAnswer === choice) radio.checked = true;
      label.append(radio, element('span', text)); choiceBox.append(label);
    });
    const input = document.createElement('textarea'); input.rows = 3; input.setAttribute('aria-label', '你的回答或问题');
    input.placeholder = '输入答案和解题过程，提交后仍可追问或说明希望怎样讲解';
    const actions = element('div', '', 'classroom-actions');
    const send = element('button', '发送给 AI'), unknown = element('button', '暂时不会'), explain = element('button', '请求讲解'), stop = element('button', '停止回答'), skip = element('button', '跳过本题');
    send.dataset.submit = '1';
    [send, unknown, explain, skip, stop].forEach(button => { button.type = 'button'; }); stop.hidden = true;
    actions.append(send, unknown, explain, skip, stop);
    const feedback = element('p', '', 'classroom-feedback'); feedback.setAttribute('role', 'status');
    const retry = element('button', '重试本次回答'); retry.type = 'button'; retry.hidden = true;
    row.append(choiceBox, history, input, actions, feedback, retry);
    let version = item.dialogueVersion || 0, questionId = item.questionId || '', socket = null, lastRequest = '', lastText = '', lastIntent = 'answer', lastGiven = null, lastSelected = null, polling = null, turnCount = 0, stopRequested = false, choiceChanged = false;
    choiceBox.addEventListener('change', () => { choiceChanged = true; });
    const contentVersion = item.contentVersion || window.gangyiContentVersion || 1;
    const draftKey = 'gy:question-draft:' + JSON.stringify([base.courseId, base.phaseIndex, base.topicIndex, base.lessonTaskId || '', base.reviewId || '', base.day ?? '', kind, questionId || index, contentVersion]);
    input.value = readLocal(draftKey);
    const saveDraft = () => saveLocal(draftKey, input.value);
    input.addEventListener('input', saveDraft);
    const draftRecord = {row, input, saveDraft, questionId, kind, index, contentVersion}; drafts.set(draftKey, draftRecord);
    const identity = () => ({...base, kind, index, questionId, contentVersion, version});
    const query = () => new URLSearchParams(identity());
    function message(text, mine, failed = false) {
      const bubble = element('div', '', 'ai-dialogue-bubble ' + (mine ? 'is-user' : 'is-teacher') + (failed ? ' is-failed' : ''));
      if (mine || failed) bubble.textContent = text; else formatPublic(bubble, text);
      history.append(bubble); return bubble;
    }
    async function refresh(restore = false) {
      try {
        const data = await api('/api/classroom/dialogue?' + query()); version = data.version ?? data.dialogueVersion ?? version; questionId = data.questionId || questionId;
        row.dataset.questionId = questionId; draftRecord.questionId = questionId; turnCount = (data.turns || []).length;
        if (restore || !socket) {
          const focused = document.activeElement === input;
          if (!focused || restore) {
            history.replaceChildren();
            (data.turns || []).forEach(turn => {
              message(turn.user, true);
              if (turn.assistant) message(turn.assistant, false);
              else {
                const parts = [...(turn.previousPartialReplies || []).map(reply => reply.text).filter(Boolean), ...(turn.partialAssistant ? [turn.partialAssistant] : [])];
                parts.forEach(text => { message(text, false); message('本次讲解未完成，已展示内容保留；尚未形成新的可靠评价。', false, true); });
              }
              if (turn.error) message(turn.error, false, true);
            });
            const latest = data.turns?.at(-1);
            if (latest && ['failed', 'cancelled', 'interrupted'].includes(latest.status)) { if (!input.value) input.value = latest.user; lastRequest = latest.requestId; lastText = latest.user; lastIntent = latest.intent || 'answer'; lastGiven = latest.givenAnswer ?? latest.user; lastSelected = latest.selectedOption ?? null; saveDraft(); }
            if (!(data.turns || []).length && item.status === 'answered') {
              if (item.studentAnswer !== undefined) message(typeof item.studentAnswer === 'string' ? item.studentAnswer : item.options?.[item.studentAnswer] || '已作答', true);
              if (item.feedback) message(item.feedback, false);
            }
          }
        }
        retry.hidden = data.evaluationStatus !== 'waiting' && !['failed', 'cancelled', 'interrupted'].includes(data.turns?.at(-1)?.status);
        feedback.textContent = data.evaluationStatus === 'waiting' ? '等待 AI 评价，之前的可靠结果保留。' :
          data.evaluationStatus === 'pending' ? ((data.turns || []).length ? '回答已保存，真实 AI 正在评价…' : '可以开始回答，也可以直接向 AI 提问。') : (data.feedback || (turnCount ? '已保存本题对话，可以继续交流。' : '未作答不表示不会，可先作答、提问或跳过。'));
        if (data.mode) document.dispatchEvent(new CustomEvent('gangyi:diagnostic-mode', {detail: data.mode}));
      } catch (error) { feedback.textContent = error.message; }
    }
    function busy(value) { send.disabled = value; unknown.disabled = value; explain.disabled = value; skip.disabled = value; retry.disabled = value; stop.disabled = false; stop.hidden = !value; row.setAttribute('aria-busy', String(value)); }
    async function submit(text, intent = turnCount ? 'ask' : 'answer', retrying = false) {
      if (socket) return;
      if (!questionId) await refresh(); if (!questionId) return;
      const selected = choiceBox.querySelector('input:checked');
      const typed = input.value.trim();
      const useChoice = selected && (choiceChanged || intent === 'answer' || !typed && !text);
      // 帮助按钮同时发送学生当轮说明；失败重试继续使用原请求，不重复拼接。
      const prompted = text && !retrying && typed ? text + '\n本轮说明或讲解偏好：' + typed : text;
      const raw = prompted || (useChoice && typed ? '所选答案：' + item.options[Number(selected.value)] + '\n解题过程或问题：' + typed : typed || (selected ? item.options[Number(selected.value)] : ''));
      const choiceAnswer = !text && !typed && selected ? Number(selected.value) : null;
      if (!raw) { feedback.textContent = '请先输入回答或选择一个答案。'; input.focus(); return; }
      const requestId = retrying && lastRequest ? lastRequest : requestIdentity();
      const givenAnswer = retrying && lastGiven !== null ? lastGiven : choiceAnswer === null ? raw : choiceAnswer;
      const selectedOption = retrying ? lastSelected : useChoice ? Number(selected.value) : null;
      lastGiven = givenAnswer; lastSelected = selectedOption;
      choiceChanged = false;
      lastText = raw; lastRequest = requestId; lastIntent = intent; input.value = ''; saveDraft();
      message(raw, true); const view = window.GangyiChat.createAssistantView(history, {scrollRoot: history}); let terminal = false;
      busy(true); feedback.textContent = '正在与真实 AI 对话…';
      socket = new WebSocket((location.protocol === 'https:' ? 'wss:' : 'ws:') + '//' + location.host + '/ws/classroom');
      const connection = socket;
      let resolveSubmission;
      const submission = new Promise(resolve => { resolveSubmission = resolve; });
      activeSubmissions.add(submission);
      const settleSubmission = () => { activeSubmissions.delete(submission); resolveSubmission(); };
      stopRequested = false;
      connection.onopen = () => { connection.send(JSON.stringify({...identity(), type: 'ask', question: raw,
        answer: givenAnswer, selectedOption, intent, requestId}));
        if (stopRequested) connection.send(JSON.stringify({type: 'stop', requestId})); };
      connection.onmessage = event => {
        let data; try { data = JSON.parse(event.data); } catch (_) { return; }
        if (data.type === 'delta') view.push(data.text || '');
        if (data.type === 'done') {
          settleSubmission();
          terminal = true; if (data.version !== undefined) version = data.version;
          view.complete();
          if (data.cancelled) { message('已停止，本次未计入可靠评价。', false, true); if (!input.value) input.value = raw; saveDraft(); retry.hidden = false; }
          else { lastRequest = ''; lastText = ''; turnCount++; feedback.textContent = '完整评价和讲解已保存，可继续追问。'; }
          connection.close();
        }
        if (data.type === 'error') { settleSubmission(); terminal = true; view.fail(data.message || '回答失败，原有结果保留。'); if (!input.value) input.value = raw; saveDraft(); retry.hidden = false; connection.close(); }
      };
      connection.onerror = () => { if (terminal) return; terminal = true; settleSubmission(); view.fail('连接中断，输入已保留，可以重试。'); if (!input.value) input.value = raw; saveDraft(); retry.hidden = false; };
      connection.onclose = () => { settleSubmission(); if (!terminal) { view.fail('回答中断，本次未形成可靠评价。'); if (!input.value) input.value = raw; saveDraft(); retry.hidden = false; } socket = null; busy(false); refresh(); input.focus(); };
    }
    send.onclick = () => submit(); unknown.onclick = () => submit('我暂时不会这道题，请根据我的情况帮助我', 'unknown'); explain.onclick = () => submit('请讲解这道题；如果我的回答缺少过程，请明确说明判断依据不足', 'ask');
    input.addEventListener('keydown', event => { if (event.ctrlKey && event.key === 'Enter') submit(); });
    stop.onclick = () => { stopRequested = true; stop.disabled = true; if (socket?.readyState === WebSocket.OPEN) socket.send(JSON.stringify({type: 'stop', requestId: lastRequest})); };
    retry.onclick = async () => { if (lastText) { await refresh(); submit(lastText, lastIntent, true); } else { retry.disabled = true; try { await api('/api/classroom/evaluation/retry', {...identity(), requestId: lastRequest || requestIdentity()}); await refresh(); } catch (error) { feedback.textContent = error.message; } finally { retry.disabled = false; } } };
    skip.onclick = async () => { saveDraft(); try { await api('/api/classroom/question/skip', {...identity(), requestId: requestIdentity()}); feedback.textContent = '已跳过，可继续阅读或提问；跳过只表示缺少信息。'; } catch (error) { feedback.textContent = error.message; } };
    refresh(true); polling = setInterval(() => { if (!document.hidden && row.isConnected && !socket) refresh(); if (!row.isConnected) clearInterval(polling); }, 4000);
    return row;
  }

  function mountStudyPlan(before) {
    if (byId('weekly-plan')) return;
    const host = typeof before === 'string' ? document.querySelector(before) : before; if (!host) return;
    const box = element('section', '', 'classroom-card weekly-plan'); box.id = 'weekly-plan';
    const title = element('h2', 'AI 统一学习安排'); title.id = 'weekly-title';
    const message = element('p', '所有课程共用每天的学习时间。'); message.id = 'weekly-message'; message.setAttribute('role', 'status');
    const availability = element('div', ''), entries = element('div', ''); availability.id = 'weekly-availability'; entries.id = 'weekly-entries';
    const actions = element('div', '', 'classroom-actions'), save = element('button', '保存修改'), replan = element('button', '根据最新表现重排');
    save.id = 'weekly-save'; replan.id = 'weekly-replan'; save.type = replan.type = 'button'; actions.append(save, replan);
    const confirm = element('div', '', 'ai-plan-preview'); confirm.id = 'weekly-confirm'; confirm.hidden = true;
    const timeDetails = element('details', ''); timeDetails.append(element('summary', '调整每天可用时间'), availability); box.append(title, message, timeDetails, entries, actions, confirm); host.before(box);
    let plan = null, dirty = false, busy = false, editEpoch = 0;
    const names = ['周一', '周二', '周三', '周四', '周五', '周六', '周日'];
    const entryTitle = item => item.title || item.topic || item.courseTitle ||
      ({'agent-lesson':'已备课课时', lesson:'课程知识点', review:'复习任务'}[item.kind] || '学习任务');
    function publicReason(value) {
      let reason = value.proposal.reason || '查看后确认是否应用。';
      const known = [...(value.proposal.previousPlan?.entries || []), ...(value.proposal.plan.entries || [])];
      for (const item of known) {
        for (const id of [item.taskId, item.lessonId, item.reviewId]) if (id) reason = reason.split(id).join(entryTitle(item));
        if (item.courseId) reason = reason.split(item.courseId).join(item.courseTitle || '该课程');
      }
      return reason;
    }
    function gather() {
      return {availability: [...availability.querySelectorAll('label')].filter(label => label.querySelector('[type=checkbox]').checked)
        .map(label => ({weekday: Number(label.dataset.weekday), minutes: Number(label.querySelector('[type=number]').value)})),
        entries: [...entries.querySelectorAll('.weekly-row')].map(row => ({...plan.entries[Number(row.dataset.index)],
          date: row.querySelector('[type=date]').value, minutes: Number(row.querySelector('[type=number]').value)}))};
    }
    function announceDraft(active = dirty) {
      if (!plan) return Promise.resolve();
      return api('/api/study-plan/draft', {clientId, version: plan.version, active, ...(active ? gather() : {})}).catch(() => {});
    }
    box.addEventListener('input', () => { dirty = true; announceDraft(); });
    function setBusy(value) { if (busy !== value) ++editEpoch; busy = value; save.disabled = value; replan.disabled = value || plan?.status === 'pending'; replan.textContent = replan.disabled ? '正在生成候选…' : '重新排课'; }
    function showPreview(value) {
      confirm.replaceChildren(); confirm.hidden = !value.proposal;
      if (!value.proposal) return;
      confirm.append(element('h3', 'AI 新旧安排对比'), element('p', publicReason(value)));
      const comparison = element('div', '', 'ai-plan-comparison');
      for (const [label, list] of [['当前安排', dirty ? gather().entries : value.proposal.previousPlan?.entries || plan.entries || []], ['AI 新安排', value.proposal.plan.entries || []]]) {
        const column = element('div', ''); column.append(element('h4', label));
        const budget = label === 'AI 新安排' ? value.proposal.plan.availability : dirty ? gather().availability : value.proposal.previousPlan?.availability || plan.availability;
        column.append(element('p', '每天总预算：' + budget.map(slot => names[slot.weekday - 1] + ' ' + slot.minutes + ' 分钟').join('、')));
        list.forEach(item => column.append(element('p', item.date + ' · ' + entryTitle(item) + ' · ' + item.minutes + ' 分钟'))); comparison.append(column);
      }
      const accept = element('button', '确认应用'), cancel = element('button', '取消，保留原安排'); accept.type = cancel.type = 'button';
      accept.id = 'weekly-accept'; cancel.id = 'weekly-cancel';
      accept.onclick = () => decide('confirm', value.proposal.id); cancel.onclick = () => decide('cancel', value.proposal.id);
      confirm.append(comparison, accept, cancel);
    }
    function render(value) {
      plan = value; title.textContent = 'AI 统一学习安排 · ' + value.weekStart + ' 至 ' + (value.weekEnd || value.weekStart);
      message.textContent = value.message || '正在等待真实 AI 安排…'; availability.replaceChildren(); entries.replaceChildren();
      const grid = element('div', '', 'availability-grid');
      names.forEach((name, index) => {
        const slot = value.availability.find(item => item.weekday === index + 1), label = element('label', ''); label.dataset.weekday = index + 1;
        const check = document.createElement('input'); check.type = 'checkbox'; check.checked = !!slot;
        check.dataset.weekday = index + 1;
        const minutes = document.createElement('input'); minutes.type = 'number'; minutes.min = 1; minutes.max = 1440; minutes.step = 1;
        minutes.value = slot?.minutes || 30; minutes.setAttribute('aria-label', name + '总分钟数'); label.append(check, element('span', name), minutes, element('span', '分钟')); grid.append(label);
      }); availability.append(element('h3', '每天可用总时间'), grid);
      entries.append(element('h3', '全部课程安排'));
      (value.entries || []).forEach((item, index) => {
        const row = element('div', '', 'weekly-row'); row.dataset.index = index;
        const date = document.createElement('input'); date.type = 'date'; date.value = item.date; date.setAttribute('aria-label', '学习日期');
        const minutes = document.createElement('input'); minutes.type = 'number'; minutes.value = item.minutes; minutes.min = 1; minutes.max = 1440; minutes.setAttribute('aria-label', '学习分钟数');
        if (item.completed) date.disabled = minutes.disabled = true;
        const link = element('a', entryTitle(item) + (item.completed ? ' · 已完成' : ''));
        link.href = '/learn?' + new URLSearchParams(item.kind === 'agent-lesson' && item.lessonId ? {lessonId:item.lessonId} :
          {courseId:item.courseId, phaseIndex:item.phaseIndex, topicIndex:item.topicIndex, ...(item.kind === 'review' ? {reviewId:item.reviewId, review:item.day} : {})});
        row.append(date, link, minutes, element('span', '分钟'));
        for (const [symbol, offset] of [['↑', -1], ['↓', 1]]) {
          const move = element('button', symbol); move.type = 'button'; move.setAttribute('aria-label', offset < 0 ? '上移' : '下移');
          move.dataset[offset < 0 ? 'up' : 'down'] = '1';
          move.onclick = () => { const list = gather(); const to = index + offset; if (to < 0 || to >= list.entries.length) return; [list.entries[index], list.entries[to]] = [list.entries[to], list.entries[index]]; render({...plan, ...list}); dirty = true; announceDraft(); }; row.append(move);
        }
        entries.append(row);
      });
      if (!(value.entries || []).length) entries.append(element('p', '尚未安排课程。设置可用时间后，点击“重新排课”生成候选。'));
      showPreview(value);
      setBusy(busy);
    }
    async function load() { if (busy) return; const epoch = editEpoch; try { const value = await api('/api/study-plan'); if (busy || epoch !== editEpoch) return; if (!dirty) render(value); else { message.textContent = value.message; showPreview(value); } } catch (error) { if (epoch === editEpoch) message.textContent = error.message; } }
    async function decide(action, proposalId) {
      setBusy(true); try { const data = await api('/api/study-plan/' + action, {version: plan.version, proposalId}); dirty = false; await announceDraft(false); render(data.plan); } catch (error) { message.textContent = error.message; } finally { setBusy(false); }
    }
    save.onclick = async () => { setBusy(true); try { const saved = await api('/api/study-plan', {version: plan.version, ...gather()}, 'PUT'); dirty = false; await announceDraft(false); render(saved); } catch (error) { message.textContent = error.message; } finally { setBusy(false); } };
    replan.onclick = async () => { confirm.hidden = true; confirm.replaceChildren(); setBusy(true); try { const data = await api('/api/study-plan/replan', {version: plan.version, availability: gather().availability, ...(dirty ? {entries: gather().entries, preview: true} : {})}); plan.status = data.plan.status; message.textContent = '真实 AI 正在统筹全部课程，请稍候…'; } catch (error) { message.textContent = error.message; } finally { setBusy(false); } };
    load(); setInterval(() => { if (!document.hidden) load(); }, 4000); setInterval(() => { if (dirty) announceDraft(); }, 30000);
    window.addEventListener('pagehide', () => { if (dirty) navigator.sendBeacon('/api/study-plan/draft', new Blob([JSON.stringify({clientId, active: false})], {type: 'application/json'})); });
  }

  async function mountPreview() {
    if (!context.courseId || !byId('plan-view')) return;
    let host = byId('course-preview');
    if (!host) { const card = element('section', '', 'plan-card full-card'); card.append(element('h2', 'AI 课程路线预览')); host = element('div', ''); host.id = 'course-preview'; card.append(host); byId('plan-view').prepend(card); }
    if (host.dataset.mounted) return; host.dataset.mounted = '1';
    let active = 0, previewTaskId = readLocal('gy:preview-task:' + context.courseId);
    const tabs = element('div', '', 'slide-tabs'), content = element('article', '', 'slide-content'), status = element('p', '', 'ai-muted');
    const update = element('button', '更新 AI 预览'); update.type = 'button'; host.replaceChildren(status, tabs, content, update);
    function display(slides) {
      active = Math.min(active, slides.length - 1); tabs.replaceChildren(); content.replaceChildren();
      slides.forEach((slide, index) => { const button = element('button', String(index + 1).padStart(2, '0') + ' · ' + slide.title, 'slide-tab' + (index === active ? ' selected' : '')); button.type = 'button'; button.onclick = () => { active = index; display(slides); }; tabs.append(button); });
      const slide = slides[active]; if (!slide) return;
      content.append(element('h3', slide.title), element('p', slide.content));
      if (slide.bullets?.length) { const list = element('ul', '', 'ai-preview-bullets'); slide.bullets.forEach(text => list.append(element('li', text))); content.append(list); }
    }
    async function load() {
      try {
        const value = await api('/api/courses/' + encodeURIComponent(context.courseId) + '/preview');
        display(value.slides || []); status.textContent = (value.message || '正在等待真实 AI 生成课程路线预览…') + (value.updatedAt ? ' · ' + localTime(value.updatedAt) : '');
        update.disabled = false;
        if (previewTaskId) {
          const task = await api('/api/learning-agent?taskId=' + encodeURIComponent(previewTaskId));
          update.disabled = ['pending', 'running'].includes(task.status);
          if (update.disabled || ['failed', 'paused', 'superseded', 'waiting_student'].includes(task.status)) status.textContent = task.error || task.message || 'AI 正在准备路线说明。';
        } else if (value.status === 'pending') status.textContent = '原预览尚未完成，可以让 AI 按最新情况重新准备。';

      } catch (error) { status.textContent = error.message; }
    }
    update.onclick = async () => { update.disabled = true; try { const task = await api('/api/courses/' + encodeURIComponent(context.courseId) + '/preview', {retry: true, requestId: requestIdentity()}); previewTaskId = task.id; saveLocal('gy:preview-task:' + context.courseId, previewTaskId); await load(); } catch (error) { status.textContent = error.message; update.disabled = false; } };
    load(); setInterval(() => { if (!document.hidden) load(); }, 4000);
  }

  function mountNextStep() {
    const home = document.querySelector('.home-hero');
    const host = home; if (!host || byId('ai-next-step')) return;
    const box = element('section', '', 'classroom-card ai-next-step'); box.id = 'ai-next-step';
    const detail = element('p', '正在读取真实 AI 的下一步建议…'), link = element('a', '查看学习安排 →');
    const fullDetail=element('details','','course-more'), fullText=element('p','','ai-next-full');
    fullText.style.whiteSpace='pre-wrap';fullDetail.append(element('summary','展开 AI 完整说明'),fullText);fullDetail.hidden=true;
    link.href = '/my-courses#weekly-plan'; link.className = 'ai-next-action'; box.append(detail, fullDetail, link); home.querySelector('.home-intro').after(box);
    async function load() {
      try {
        const data = await api('/api/home/next-step');
        const text=data.lesson ? data.lesson.title + '：' + (data.message || 'AI 已完成备课') : data.message || '等待 AI 更新';
        const sentence=(text.match(/^[\s\S]*?[。！？\n]/)?.[0]||text).trim();
        detail.textContent=sentence;fullText.textContent=text;fullDetail.hidden=sentence===text.trim();
        link.href = data.lesson ? '/agent-prepare.html?taskId='+encodeURIComponent(data.id) : '/my-courses#weekly-plan'; link.hidden = !data.lesson; link.textContent = data.lesson ? '查看 AI 已备好的学习内容 →' : '查看学习安排 →';
      } catch (_) { detail.textContent = '等待 AI 更新，已保存的课程仍可继续阅读。'; }
    }
    load(); setInterval(() => { if (!document.hidden) load(); }, 4000);
  }
  function mountPreparation() {
    if (!byId('learn-content') || !context.courseId || byId('ai-preparation-note')) return;
    const box = element('section', '', 'classroom-card'); box.id = 'ai-preparation-note';
    const status = element('p', 'AI 会根据你的真实对话和作答调整讲解与练习。'), supplement = element('p', '', 'ai-supplement');
    const retry = element('button', '重试 AI 备课'); retry.type = 'button'; retry.hidden = true;
    box.append(element('h2', 'AI 正在跟着你备课'), status, supplement, retry); byId('learn-content').prepend(box);
    const exposed = new Set(), supplements = new Set(), selectors = {overview: '#learn-key-concepts', steps: '#learn-steps', examples: '#learn-examples', practice: '#learn-practice', quiz: '#learn-quiz', assessment: '#learn-checkpoint'};
    const observer = new IntersectionObserver(records => {
      const names = [];
      records.forEach(record => { if (record.isIntersecting && !exposed.has(record.target.dataset.aiBlock) && !byId('learn-content').classList.contains('hidden')) { exposed.add(record.target.dataset.aiBlock); names.push(record.target.dataset.aiBlock); } });
      if (names.length) api('/api/learn/exposure', {...context, blocks: names, contentVersion: window.gangyiContentVersion || 1}).catch(() => {});
    }, {threshold: 0.05});
    Object.entries(selectors).forEach(([block, selector]) => { const node = document.querySelector(selector); if (node) { node.dataset.aiBlock = block; observer.observe(node); } });
    async function load() {
      try {
        if (context.lessonTaskId) { status.textContent = '本课堂使用已经完整保存的课程；下一课会结合最新作答重新分析。'; return; }
        const value = await api('/api/classroom/preparation?courseId=' + encodeURIComponent(context.courseId));
        status.textContent = value.message; retry.hidden = value.status !== 'waiting';
        const instruction = (value.teaching || []).find(item => item.phaseIndex === context.phaseIndex && item.topicIndex === context.topicIndex);
        if (instruction?.supplement && !supplements.has(instruction.supplement)) { supplements.add(instruction.supplement); const addition = element('div', ''); formatPublic(addition, instruction.supplement); supplement.append(addition); }
      } catch (_) { /* 已展示的课堂和输入保持可用。 */ }
    }
    retry.onclick = async () => { retry.disabled = true; try { await api('/api/classroom/preparation/retry', context); await load(); } finally { retry.disabled = false; } };
    load(); if (!context.lessonTaskId) setInterval(() => { if (!document.hidden) load(); }, 4000);
  }
  function collectDrafts() {
    const list = [];
    for (const [key, draft] of drafts) {
      if (!draft.row.isConnected) { drafts.delete(key); continue; }
      draft.saveDraft();
      if (draft.input.value.trim()) list.push({questionId: draft.questionId, kind: draft.kind, index: draft.index, text: draft.input.value, contentVersion: draft.contentVersion});
    }
    const input = byId('learning-chat-input');
    if (input?.value.trim()) { saveLocal('gy:lesson-chat-draft:' + JSON.stringify(context), input.value); list.push({kind: 'lesson', text: input.value}); }
    return list;
  }
  async function mountQuestionKind(kind, target) {
    const host = typeof target === 'string' ? document.querySelector(target) : target; if (!host || host.dataset.questionsLoading) return;
    const contentVersion = window.gangyiContentVersion || 1;
    if (host.dataset.questionsVersion === String(contentVersion)) return;
    host.dataset.questionsLoading = '1';
    try {
      const data = await api('/api/classroom/questions?' + new URLSearchParams({...context, kind, contentVersion}));
      const questions = (data.questions || []).filter(item => item.kind === kind);
      // 内容已展示后保留原题和对话；追加题目不会覆盖学生正在编辑的输入。
      if (!host.querySelector('.ai-dialogue')) host.replaceChildren();
      for (const item of questions) if (![...host.querySelectorAll('.ai-dialogue')].some(row => row.dataset.questionId === item.questionId)) {
        const row = renderDialogue(kind, item, item.index, context); row.dataset.questionId = item.questionId; host.append(row);
      }
      if (!questions.length && !host.children.length) host.append(element('p', '本节没有这一类题目，可继续阅读或请 AI 准备下一课。', 'ai-muted'));
      host.dataset.questionsVersion = String(contentVersion);
    } catch (error) {
      if (!host.querySelector('.ai-dialogue')) { host.replaceChildren(element('p', error.message)); const retry = element('button', '重试读取题目'); retry.type = 'button'; retry.onclick = () => { delete host.dataset.questionsVersion; mountQuestionKind(kind, host); }; host.append(retry); }
    } finally { delete host.dataset.questionsLoading; }
  }
  function mountNextLessonAction() {
    const next = byId('lesson-next'); if (!next || next.dataset.preparationMounted) return;
    next.dataset.preparationMounted = '1'; next.removeAttribute('href'); next.textContent = '让 AI 准备下一课';
    next.onclick = async event => {
      event.preventDefault(); if (next.disabled) return;
      next.disabled = true; next.textContent = 'AI 正在读取学习情况…';
      const status = byId('next-lesson-action-status');
      try {
        const data = await api('/api/learn/next', {...context, requestId: requestIdentity(), drafts: collectDrafts(), contentVersion: window.gangyiContentVersion || 1});
        if (!data.id || !String(data.url || '').startsWith('/learn/next?')) throw new Error('备课任务未成功创建，请重试。');
        // 先让当前页已提交的回答到终态，避免导航断开连接而取消在途评价。
        if (activeSubmissions.size && status) status.textContent = '已提交回答仍在处理，完成后进入备课页。';
        while (activeSubmissions.size) await Promise.all([...activeSubmissions]);
        location.assign(data.url);
      } catch (error) { if (status) status.textContent = error.message; next.disabled = false; next.textContent = '让 AI 准备下一课'; }
    };
  }
  function showPublicBlock(host, stage, block) {
    if (!block || typeof block !== 'object') return;
    host.replaceChildren();
    const prose = value => { if (typeof value !== 'string' || !value.trim()) return; const node = element('div', '', 'preparation-prose'); formatPublic(node, value); host.append(node); };
    if (stage === 'overview') { prose(block.title); prose(block.summary); (block.keyConcepts || []).forEach(prose); }
    if (stage === 'steps') (block.lessonSteps || []).forEach(item => { prose(item.title); prose(item.explanation); prose(item.example); prose(item.action); });
    if (['examples', 'practice', 'quiz'].includes(stage)) {
      const items = stage === 'examples' ? block.examples : stage === 'practice' ? block.practice : block.quiz;
      (items || []).forEach(item => { const question = element('article', '', 'preparation-question'); const title = element('h3', item.title || item.question || item.task || '题目'); question.append(title);
        const text = item.question || item.task || item.content; if (text && text !== title.textContent) { const node = element('div', ''); formatPublic(node, text); question.append(node); }
        if (typeof item.materials === 'string') { const material = element('div', ''); formatPublic(material, item.materials); question.append(material); }
        if (Array.isArray(item.options)) { const options = element('ol', '', 'preparation-options'); item.options.forEach(text => options.append(element('li', text))); question.append(options); }
        host.append(question);
      });
    }
    if (stage === 'assessment') { (block.checkpoint || []).forEach(prose); prose(block.summary); prose(block.resourceSummary); }
    if (stage === 'decision') prose(block.reason || block.text);
  }
  function mountNextLessonPage() {
    const page = byId('next-lesson-page'); if (!page || page.dataset.mounted) return; page.dataset.mounted = '1';
    const id = params.get('id'), status = byId('next-lesson-status'), blocksHost = byId('next-lesson-blocks'), stop = byId('next-lesson-stop'), retry = byId('next-lesson-retry'), back = byId('next-lesson-return');
    const names = {decision: '调整说明', overview: '课程总览', steps: '讲解', examples: '例题', practice: '练习', quiz: '测验', assessment: '总结'};
    const stages = new Map(); let socket = null, lastSeq = 0, terminal = false, reconnectTimer = null, reconnectCount = 0;
    for (const [stage, name] of Object.entries(names)) { const section = element('section', '', 'classroom-card preparation-stage'); section.dataset.stage = stage; section.hidden = true;
      const text = element('div', '', 'preparation-stream'); const completed = element('small', '', 'ai-muted'); section.append(element('h2', name), text, completed); blocksHost.append(section); stages.set(stage, {section, text, completed, raw: ''}); }
    back.textContent = '返回课堂'; back.href = '/my-courses';
    function finish(type, message) { terminal = true; stop.hidden = true; retry.hidden = type === 'ready'; status.textContent = message; if (socket) socket.close(); }
    function ready(url) { if (!url || !String(url).startsWith('/learn?')) { finish('failed', '备课保存结果缺少有效课堂地址，请重新准备。'); return; } finish('ready', '课程已完整校验并保存，正在进入课堂…'); location.assign(url); }
    function event(data) {
      if (!Number.isInteger(data.seq) || data.seq <= lastSeq) return;
      if (data.seq !== lastSeq + 1) { if (socket) socket.close(); restore(); return; }
      lastSeq = data.seq;
      if (data.reset) { terminal = false; stop.hidden = false; retry.hidden = true; stages.forEach(part => { part.raw = ''; part.text.replaceChildren(); part.section.hidden = true; part.completed.textContent = ''; }); }
      const part = stages.get(data.stage);
      if (part) part.section.hidden = false;
      if (data.type === 'stage') { status.textContent = (names[data.stage] || '备课') + '：真实 AI 正在生成…'; if (part) part.completed.textContent = '生成中'; }
      if (data.type === 'delta' && part) { part.raw += data.text || ''; formatPublic(part.text, part.raw); }
      if (data.type === 'block_complete' && part) { const block = typeof data.block === 'object' ? data.block : data.data; if (block) showPublicBlock(part.text, data.stage, block); part.completed.textContent = '本板块已生成，等待整课校验和保存'; }
      if (data.type === 'ready') ready(data.classroomUrl);
      if (data.type === 'failed' || data.type === 'cancelled') finish(data.type, data.text || data.message || (data.type === 'failed' ? '备课未完成，原课堂已保留。可重试或返回。' : '备课已停止。可以重新准备或返回课堂。'));
    }
    function subscribe() {
      if (terminal || !id || socket) return;
      const connection = new WebSocket((location.protocol === 'https:' ? 'wss:' : 'ws:') + '//' + location.host + '/ws/learn/next'); socket = connection;
      connection.onopen = () => { reconnectCount = 0; connection.send(JSON.stringify({type: 'subscribe', id, afterSeq: lastSeq})); };
      connection.onmessage = payload => { try { const data = JSON.parse(payload.data); if (data.type === 'error') finish('failed', data.message || '无法订阅备课，请重新准备。'); else event(data); } catch (_) { /* 不完整事件不改变已展示内容。 */ } };
      connection.onclose = () => { if (socket === connection) socket = null; if (!terminal) { status.textContent = '连接中断，正在恢复已生成内容…'; clearTimeout(reconnectTimer); reconnectTimer = setTimeout(restore, Math.min(1000 * ++reconnectCount, 8000)); } };
      connection.onerror = () => { connection.close(); };
    }
    async function restore() {
      if (!id) { finish('failed', '缺少备课任务编号，请返回课堂重新准备。'); return; }
      try {
        const data = await api('/api/learn/next/' + encodeURIComponent(id));
        if (data.sourceUrl?.startsWith('/learn?')) back.href = data.sourceUrl;
        (data.events || []).sort((a, b) => a.seq - b.seq).forEach(event);
        if (terminal) return;
        // 服务端裁剪历史事件时，用完整公开快照恢复；不在刷新时重新创建任务。
        if (data.latestSeq > lastSeq && !(data.events || []).length) { for (const [stage, block] of Object.entries(data.blocks || {})) { const part = stages.get(stage); if (part) { part.section.hidden = false; showPublicBlock(part.text, stage, block); } } if (data.reason) { const part = stages.get('decision'); part.section.hidden = false; formatPublic(part.text, data.reason); } lastSeq = data.latestSeq; }
        if (data.status === 'ready') { ready(data.classroomUrl); return; }
        if (['failed', 'cancelled', 'stale'].includes(data.status)) { finish(data.status, data.message || '本次备课未保存，请重新准备或返回课堂。'); return; }
        status.textContent = data.message || '真实 AI 正在结合最新学习情况准备课程…'; stop.hidden = false; retry.hidden = true; subscribe();
      } catch (error) { status.textContent = error.message + '，可以重连或返回课堂。'; retry.hidden = false; }
    }
    stop.onclick = async () => { stop.disabled = true; try { await api('/api/learn/next/' + encodeURIComponent(id) + '/cancel', {}); await restore(); } catch (error) { status.textContent = error.message; } finally { stop.disabled = false; } };
    retry.onclick = async () => {
      retry.disabled = true;
      try { const result = await api('/api/learn/next/' + encodeURIComponent(id) + '/retry', {requestId: requestIdentity()});
        if (result.id && result.id !== id && result.url) { location.assign(result.url); return; }
        terminal = false; stages.forEach(part => { part.raw = ''; part.text.replaceChildren(); part.section.hidden = true; part.completed.textContent = ''; }); stop.hidden = false; retry.hidden = true; await restore();
      } catch (error) { status.textContent = error.message; } finally { retry.disabled = false; }
    };
    window.addEventListener('pagehide', () => { terminal = true; clearTimeout(reconnectTimer); if (socket) socket.close(); });
    restore();
  }
  window.GangyiLearning = {renderDialogue, mountStudyPlan, mountQuestionKind, collectDrafts};
  document.dispatchEvent(new CustomEvent('gangyi:learning-ready'));
  const initial = () => {
    if (document.querySelector('.home-hero')) mountNextStep(); if (byId('uc-study-plan-anchor')) mountStudyPlan('#uc-study-plan-anchor');
    mountPreview(); mountNextStep(); mountPreparation(); mountNextLessonAction(); mountNextLessonPage();
  };
  const monitor = new MutationObserver(initial); monitor.observe(document.body, {childList: true, subtree: true});
  initial();
  window.addEventListener('pagehide', collectDrafts);
})();

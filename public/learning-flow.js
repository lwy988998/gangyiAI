(() => {
  'use strict';
  const params = new URLSearchParams(location.search);
  const context = {courseId: params.get('courseId') || '', phaseIndex: Number(params.get('phaseIndex') || 1), topicIndex: Number(params.get('topicIndex') || 1)};
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
  const shortFormula = text => String(text || '').replace(/\b(H2|CO2|O2|Cl2|NH3)↑1\b/g, '$1↑');

  function renderDialogue(kind, item, index, base) {
    const row = element('article', '', 'classroom-question ai-dialogue'); row.dataset.index = index;
    row.append(element('h3', item.question));
    const history = element('div', '', 'ai-dialogue-history'); history.setAttribute('aria-label', '本题对话记录');
    const choiceBox = element('div', '', 'classroom-options');
    if (item.type !== 'open') (item.options || []).forEach((text, choice) => {
      const label = element('label', ''); const radio = document.createElement('input'); radio.type = 'radio';
      radio.name = kind + '-' + index; radio.value = choice;
      if (item.answer === choice) radio.checked = true;
      label.append(radio, element('span', text)); choiceBox.append(label);
    });
    const input = document.createElement('textarea'); input.rows = 3; input.setAttribute('aria-label', '你的回答或问题');
    input.placeholder = '回答这道题，或说“我不知道”，也可以继续问 AI';
    const actions = element('div', '', 'classroom-actions');
    const send = element('button', '发送给 AI'), unknown = element('button', '暂时不会'), explain = element('button', '请讲一点'), stop = element('button', '停止回答');
    send.dataset.submit = '1';
    [send, unknown, explain, stop].forEach(button => { button.type = 'button'; }); stop.hidden = true;
    actions.append(send, unknown, explain, stop);
    const feedback = element('p', '', 'classroom-feedback'); feedback.setAttribute('role', 'status');
    const retry = element('button', '重试 AI 评价'); retry.type = 'button'; retry.hidden = true;
    row.append(choiceBox, history, input, actions, feedback, retry);
    let version = 0, questionId = '', socket = null, lastRequest = '', lastText = '', polling = null;
    const query = () => new URLSearchParams({...base, kind, index});
    function message(text, mine, failed = false) {
      const bubble = element('div', text, 'ai-dialogue-bubble ' + (mine ? 'is-user' : 'is-teacher') + (failed ? ' is-failed' : ''));
      history.append(bubble); return bubble;
    }
    async function refresh(restore = false) {
      try {
        const data = await api('/api/classroom/dialogue?' + query()); version = data.version; questionId = data.questionId;
        if (restore || !socket) {
          const focused = document.activeElement === input;
          if (!focused || restore) {
            history.replaceChildren();
            (data.turns || []).forEach(turn => { message(turn.user, true); if (turn.assistant) message(turn.assistant, false); if (turn.error) message(turn.error, false, true); });
            const latest = data.turns?.at(-1);
            if (restore && latest?.status === 'failed') { input.value = latest.user; lastRequest = latest.requestId; lastText = latest.user; }
            if (!(data.turns || []).length && item.status === 'answered') {
              if (item.answer !== undefined) message(typeof item.answer === 'string' ? item.answer : item.options?.[item.answer] || '已作答', true);
              if (item.feedback) message(item.feedback, false);
            }
          }
        }
        retry.hidden = data.evaluationStatus !== 'waiting';
        feedback.textContent = data.evaluationStatus === 'waiting' ? '等待 AI 评价，之前的可靠结果保留。' :
          data.evaluationStatus === 'pending' ? ((data.turns || []).length ? '回答已保存，真实 AI 正在评价和调整备课…' : '可以开始回答，也可以直接向 AI 提问。') : (data.feedback || 'AI 已处理你的反馈，可以继续交流。');
        if (data.mode) document.dispatchEvent(new CustomEvent('gangyi:diagnostic-mode', {detail: data.mode}));
      } catch (error) { feedback.textContent = error.message; }
    }
    function busy(value) { send.disabled = value; unknown.disabled = value; explain.disabled = value; stop.hidden = !value; row.setAttribute('aria-busy', String(value)); }
    async function submit(text, intent = 'answer') {
      if (socket) return;
      if (!questionId) await refresh(); if (!questionId) return;
      const selected = choiceBox.querySelector('input:checked');
      const raw = text || input.value.trim() || (selected ? item.options[Number(selected.value)] : '');
      const choiceAnswer = !text && !input.value.trim() && selected ? Number(selected.value) : null;
      if (!raw) { feedback.textContent = '请先输入回答或选择一个答案。'; input.focus(); return; }
      const requestId = raw === lastText && lastRequest ? lastRequest : (crypto.randomUUID ? crypto.randomUUID() : clientId + '-' + Date.now());
      lastText = raw; lastRequest = requestId; input.value = '';
      message(raw, true); const bubble = message('AI 正在思考…', false); let answer = '', terminal = false;
      busy(true); feedback.textContent = '正在与真实 AI 对话…';
      socket = new WebSocket((location.protocol === 'https:' ? 'wss:' : 'ws:') + '//' + location.host + '/ws/classroom');
      const connection = socket;
      connection.onopen = () => connection.send(JSON.stringify({...base, kind, index, type: 'ask', question: raw,
        answer: choiceAnswer === null ? raw : choiceAnswer, intent, questionId, version, requestId}));
      connection.onmessage = event => {
        const data = JSON.parse(event.data);
        if (data.type === 'delta') { answer += data.text; bubble.textContent = answer; }
        if (data.type === 'done') {
          terminal = true; if (data.version !== undefined) version = data.version;
          if (data.cancelled) { bubble.textContent = (answer ? answer + '\n' : '') + '已停止，本次未计入可靠评价。'; input.value = raw; }
          else { lastRequest = ''; lastText = ''; feedback.textContent = '回答已保存，AI 正在根据表现调整备课。'; }
          connection.close();
        }
        if (data.type === 'error') { terminal = true; bubble.textContent = data.message; input.value = raw; connection.close(); }
      };
      connection.onerror = () => { bubble.textContent = '连接中断，输入已保留，可以重试。'; input.value = raw; };
      connection.onclose = () => { if (!terminal) input.value = raw; socket = null; busy(false); refresh(); input.focus(); };
    }
    send.onclick = () => submit(); unknown.onclick = () => submit('我暂时不会这道题'); explain.onclick = () => submit('请先用简单的话讲一点，再问我一个小问题', 'ask');
    input.addEventListener('keydown', event => { if (event.ctrlKey && event.key === 'Enter') submit(); });
    stop.onclick = () => { if (socket?.readyState === WebSocket.OPEN) socket.send(JSON.stringify({type: 'stop'})); };
    retry.onclick = async () => { retry.disabled = true; try { await api('/api/classroom/evaluation/retry', {...base, kind, index}); await refresh(); } catch (error) { feedback.textContent = error.message; } finally { retry.disabled = false; } };
    if (kind === 'interaction') {
      const skip = element('button', '跳过活动'); skip.type = 'button'; actions.append(skip);
      skip.onclick = async () => { try { await api('/api/classroom/activity/skip', {...base, index}); feedback.textContent = '已跳过，可继续阅读或提问；跳过不形成掌握证据。'; } catch (error) { feedback.textContent = error.message; } };
    }
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
    box.append(title, message, element('p', '每天的分钟数是全部课程的总预算。手动安排会先展示对比，经确认后更新。', 'ai-muted'), availability, entries, actions, confirm); host.before(box);
    let plan = null, dirty = false, busy = false, editEpoch = 0;
    const names = ['周一', '周二', '周三', '周四', '周五', '周六', '周日'];
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
    function setBusy(value) { if (busy !== value) ++editEpoch; busy = value; save.disabled = value; replan.disabled = value || plan?.status === 'pending'; replan.textContent = replan.disabled ? '正在重排…' : '根据最新表现重排'; }
    function showPreview(value) {
      confirm.replaceChildren(); confirm.hidden = !value.proposal;
      if (!value.proposal) return;
      confirm.append(element('h3', 'AI 新旧安排对比'), element('p', value.proposal.reason || '查看后确认是否应用。'));
      const comparison = element('div', '', 'ai-plan-comparison');
      for (const [label, list] of [['当前安排', dirty ? gather().entries : value.proposal.previousPlan?.entries || plan.entries || []], ['AI 新安排', value.proposal.plan.entries || []]]) {
        const column = element('div', ''); column.append(element('h4', label));
        const budget = label === 'AI 新安排' ? value.proposal.plan.availability : dirty ? gather().availability : value.proposal.previousPlan?.availability || plan.availability;
        column.append(element('p', '每天总预算：' + budget.map(slot => names[slot.weekday - 1] + ' ' + slot.minutes + ' 分钟').join('、')));
        list.forEach(item => column.append(element('p', item.date + ' · ' + item.title + ' · ' + item.minutes + ' 分钟'))); comparison.append(column);
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
        const minutes = document.createElement('input'); minutes.type = 'number'; minutes.min = 10; minutes.max = 240; minutes.step = 5;
        minutes.value = slot?.minutes || 30; minutes.setAttribute('aria-label', name + '总分钟数'); label.append(check, element('span', name), minutes, element('span', '分钟')); grid.append(label);
      }); availability.append(element('h3', '每天可用总时间'), grid);
      entries.append(element('h3', '全部课程安排'));
      (value.entries || []).forEach((item, index) => {
        const row = element('div', '', 'weekly-row'); row.dataset.index = index;
        const date = document.createElement('input'); date.type = 'date'; date.value = item.date; date.setAttribute('aria-label', '学习日期');
        const minutes = document.createElement('input'); minutes.type = 'number'; minutes.value = item.minutes; minutes.min = 5; minutes.max = 240; minutes.setAttribute('aria-label', '学习分钟数');
        if (item.completed) date.disabled = minutes.disabled = true;
        const link = element('a', item.title + (item.completed ? ' · 已完成' : '')); link.href = '/learn?' + new URLSearchParams({courseId: item.courseId, phaseIndex: item.phaseIndex, topicIndex: item.topicIndex, ...(item.kind === 'review' ? {reviewId: item.reviewId, review: item.day} : {})});
        row.append(date, link, minutes, element('span', '分钟'));
        for (const [symbol, offset] of [['↑', -1], ['↓', 1]]) {
          const move = element('button', symbol); move.type = 'button'; move.setAttribute('aria-label', offset < 0 ? '上移' : '下移');
          move.dataset[offset < 0 ? 'up' : 'down'] = '1';
          move.onclick = () => { const list = gather(); const to = index + offset; if (to < 0 || to >= list.entries.length) return; [list.entries[index], list.entries[to]] = [list.entries[to], list.entries[index]]; render({...plan, ...list}); dirty = true; announceDraft(); }; row.append(move);
        }
        entries.append(row);
      });
      if (!(value.entries || []).length) entries.append(element('p', value.status === 'waiting' ? '等待 AI 安排，可以保存总学习时间后重试。' : 'AI 正在结合学习情况准备安排。'));
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
    let active = 0, queued = false;
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
        update.disabled = value.status === 'pending';
        if (!queued && (value.status === 'missing' || value.stale && value.status === 'ready')) { queued = true; await api('/api/courses/' + encodeURIComponent(context.courseId) + '/preview', {}); }
      } catch (error) { status.textContent = error.message; }
    }
    update.onclick = async () => { update.disabled = true; try { await api('/api/courses/' + encodeURIComponent(context.courseId) + '/preview', {retry: true}); await load(); } catch (error) { status.textContent = error.message; update.disabled = false; } };
    load(); setInterval(() => { if (!document.hidden) load(); }, 4000);
  }

  function mountNextStep() {
    const home = document.querySelector('.home-hero'), lesson = byId('lesson-navigation') || byId('complete-lesson');
    const host = lesson || home; if (!host || byId('ai-next-step')) return;
    const box = element('section', '', 'classroom-card ai-next-step'); box.id = 'ai-next-step';
    const detail = element('p', '正在读取真实 AI 的下一步建议…'), link = element('a', '查看学习安排 →');
    link.href = '/#weekly-plan'; link.className = 'ai-next-action'; box.append(element('h2', 'AI 推荐的下一步'), detail, link); host.after(box);
    async function load() {
      try {
        const data = await api('/api/home/next-step' + (lesson && context.courseId ? '?courseId=' + encodeURIComponent(context.courseId) : ''));
        detail.textContent = data.title ? data.title + '：' + (data.reason || data.message) : data.message;
        link.href = data.href || '/#weekly-plan'; link.textContent = data.title ? '前往 AI 建议的学习内容 →' : '查看学习安排 →';
        const next = byId('lesson-next');
        if (next && data.href) {
          const current = data.courseId === context.courseId && data.phaseIndex === context.phaseIndex && data.topicIndex === context.topicIndex;
          next.href = current ? '#ai-preparation-note' : data.href; next.textContent = current ? '先按 AI 建议巩固本节 →' : '前往 AI 推荐的下一节 →';
        } else if (next) next.textContent = '按课程大纲继续 →';
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
    const exposed = new Set(), selectors = {overview: '#learn-key-concepts', steps: '#learn-steps', examples: '#learn-examples', practice: '#learn-practice', quiz: '#learn-quiz', assessment: '#learn-checkpoint'};
    const observer = new IntersectionObserver(records => {
      const names = [];
      records.forEach(record => { if (record.isIntersecting && !exposed.has(record.target.dataset.aiBlock) && !byId('learn-content').classList.contains('hidden')) { exposed.add(record.target.dataset.aiBlock); names.push(record.target.dataset.aiBlock); } });
      if (names.length) api('/api/learn/exposure', {...context, blocks: names, contentVersion: window.gangyiContentVersion || 1}).catch(() => {});
    }, {threshold: 0.05});
    Object.entries(selectors).forEach(([block, selector]) => { const node = document.querySelector(selector); if (node) { node.dataset.aiBlock = block; observer.observe(node); } });
    async function load() {
      try {
        const value = await api('/api/classroom/preparation?courseId=' + encodeURIComponent(context.courseId));
        status.textContent = value.message; retry.hidden = value.status !== 'waiting';
        const instruction = (value.teaching || []).find(item => item.phaseIndex === context.phaseIndex && item.topicIndex === context.topicIndex);
        supplement.textContent = instruction?.supplement || '';
        const cached = await api('/api/learn?' + new URLSearchParams(context));
        if ((cached.contentVersion || 1) !== (window.gangyiContentVersion || 1)) {
          for (const [block, data] of Object.entries(cached.blocks || {})) if (!exposed.has(block) && window.gangyiApplyBlock) window.gangyiApplyBlock(block, data);
          window.gangyiContentVersion = cached.contentVersion || 1;
        }
      } catch (_) { /* 已展示的课堂和输入保持可用。 */ }
    }
    retry.onclick = async () => { retry.disabled = true; try { await api('/api/classroom/preparation/retry', context); await load(); } finally { retry.disabled = false; } };
    document.addEventListener('gangyi:lesson-block', () => decorateExamples());
    decorateExamples(); load(); setInterval(() => { if (!document.hidden) load(); }, 4000);
  }
  function decorateExamples() {
    document.querySelectorAll('#learn-examples details').forEach(item => {
      if (item.querySelector('[data-ai-example]')) return;
      item.querySelectorAll('p').forEach(node => { node.textContent = shortFormula(node.textContent); });
      const button = element('button', '请 AI 带我理解这道例题'); button.type = 'button'; button.dataset.aiExample = '1';
      button.onclick = () => { const input = byId('learning-chat-input'), form = byId('learning-chat-form'); if (!input || !form) return; byId('learning-chat-panel')?.classList.remove('hidden'); input.value = '请一步步带我理解这道例题，先讲一点再问我一个小问题：' + item.textContent.replace(button.textContent, '').slice(0, 1800); form.requestSubmit(); };
      item.append(button);
    });
  }
  window.GangyiLearning = {renderDialogue, mountStudyPlan};
  const initial = () => {
    if (document.querySelector('.home-hero')) { mountNextStep(); mountStudyPlan(document.querySelector('.home-hero').nextElementSibling || document.querySelector('.dark-footer')); }
    mountPreview(); mountNextStep(); mountPreparation();
  };
  const monitor = new MutationObserver(initial); monitor.observe(document.body, {childList: true, subtree: true});
  initial();
})();

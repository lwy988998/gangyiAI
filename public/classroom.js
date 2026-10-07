(() => {
  'use strict';
  const params = new URLSearchParams(location.search);
  const courseId = params.get('courseId') || '';
  const phaseIndex = Number(params.get('phaseIndex') || 1);
  const topicIndex = Number(params.get('topicIndex') || 1);
  const $ = id => document.getElementById(id);
  const escape = value => { const node = document.createElement('span'); node.textContent = String(value ?? ''); return node.innerHTML; };
  const context = {courseId, phaseIndex, topicIndex, ...(params.get('lessonTaskId') ? {lessonTaskId: params.get('lessonTaskId')} : {}), ...(params.has('review') ? {day: Number(params.get('review')), reviewId: params.get('reviewId') || ''} : {})};
  document.addEventListener('gangyi:lesson-context', event => Object.assign(context, event.detail));
  async function api(path, data) {
    const response = await fetch(path, data ? {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(data)} : undefined);
    const result = await response.json();
    if (!response.ok) throw new Error(result.error || '请求失败，请重试');
    return result;
  }

  async function setupWeeklyPlan() {
    // 学习安排只在我的课程中显示。
  }

  async function setupReviews() {
    if (location.pathname !== '/') return;
    const main = document.querySelector('main');
    if (!main) return;
    const section = document.createElement('section');
    section.id = 'due-reviews'; section.className = 'classroom-card due-reviews';
    section.innerHTML = '<h2>待复习</h2><p role="status">正在检查到期复习…</p>';
    main.prepend(section);
    try {
      const courses = (await api('/api/courses')).courses || [];
      const results = await Promise.all(courses.map(async course => ({course, items: (await api('/api/classroom/reviews?courseId=' + encodeURIComponent(course.id))).items || []})));
      const items = results.flatMap(({course, items}) => items.map(item => ({course, item})));
      section.innerHTML = '<details><summary>待复习 · ' + items.length + ' 项</summary>' + (items.length ? '<ul>' + items.map(({course, item}) => '<li><a href="/learn?' + new URLSearchParams({courseId: course.id, phaseIndex: item.phaseIndex, topicIndex: item.topicIndex, review: item.day, reviewId: item.reviewId}).toString() + '">' + escape(course.title + ' · ' + item.title + ' · 到期 ' + item.due) + '</a></li>').join('') + '</ul>' : '<p>目前没有到期复习。</p>') + '</details>';
    } catch (_) { section.querySelector('p').textContent = '复习清单暂未更新，课程仍可继续。'; }
  }

  function setupLearn() {
    if (!courseId || !$('learn-loading')) return;
    if (params.has('review') || window.gangyiLessonKind === 'review') $('complete-lesson').textContent = '保存复习记录';
    document.addEventListener('gangyi:lesson-context', () => { if (window.gangyiLessonKind === 'review') $('complete-lesson').textContent = '保存复习记录'; });
    const lessonSection = $('learn-loading').parentElement;
    const diagnostic = document.createElement('section'); diagnostic.id = 'classroom-diagnostic'; diagnostic.className = 'classroom-card';
    diagnostic.innerHTML = '<h2>课前诊断</h2><p>可以先回答、请求提示，或暂时跳过；漏答只表示缺少信息。</p><div id="diagnostic-questions"></div>';
    lessonSection.prepend(diagnostic);
    const interaction = document.createElement('section'); interaction.id = 'classroom-interaction'; interaction.className = 'classroom-card';
    interaction.innerHTML = '<h2>边学边互动</h2><p>输入自己的理解或解题方法，AI 会结合实际回答帮助你。</p><div id="interaction-questions"></div>';
    $('learn-content').prepend(interaction);
    let diagnosticStarted = false;
    async function loadActivities() {
      if (diagnosticStarted) return; diagnosticStarted = true;
      await Promise.allSettled([
        window.GangyiLearning.mountQuestionKind('diagnostic', '#diagnostic-questions'),
        window.GangyiLearning.mountQuestionKind('interaction', '#interaction-questions')]);
    }
    document.addEventListener('gangyi:lesson-block', event => { if (event.detail.block === 'quiz') loadActivities(); });
    if (window.gangyiLessonBlocks?.quiz) loadActivities();
    const next = document.createElement('section'); next.id = 'classroom-next'; next.className = 'classroom-card'; next.hidden = true;
    $('complete-lesson').after(next);
    function showNext() { next.hidden = false; next.innerHTML = '<h2>本节记录已保存</h2><p>可以继续与 AI 交流，或让 AI 综合最新作答、课程目标和共享学习时间准备下一课。</p>'; }
    api('/api/classroom/state?' + new URLSearchParams(context)).then(state => { if (state.completed) showNext(); }).catch(() => {});
    const saveLessonProgress = $('complete-lesson').onclick;
    $('complete-lesson').onclick = async event => {
      try {
        if (params.has('review') || window.gangyiLessonKind === 'review') {
          const result = await api('/api/classroom/finish', {...context, contentVersion: window.gangyiContentVersion || 1});
          if (result.ok === false) throw new Error(result.message || '复习尚未保存');
          if (result.evaluationStatus === 'insufficient') { $('complete-feedback').textContent = result.message || '尚无可靠作答，未推测掌握情况。'; return; }
          $('complete-feedback').textContent = result.message || (result.passed ? '可靠复习评价已保存。' : '复习记录已保存，下一课会结合实际反馈安排。');
          $('complete-lesson').textContent = '复习记录已保存'; showNext(); return;
        }
        const response = await saveLessonProgress.call($('complete-lesson'), event);
        if (!response.ok) throw new Error('本节进度尚未保存');
        await api('/api/classroom/finish', context); showNext();
      } catch (error) { next.hidden = false; next.textContent = error.message + '。可以稍后重试，下一课仍可独立准备。'; }
    };
    setupChat(); setupIdlePrompt();
  }

  function setupChat() {
    const form = $('learning-chat-form'), input = $('learning-chat-input'), box = $('learning-chat-messages');
    if (!form || !input || !box) return;
    const chat = window.GangyiChat;
    const conversation = context.lessonTaskId ? 'lesson-task-' + context.lessonTaskId : 'lesson-' + courseId + '-' + phaseIndex + '-' + topicIndex;
    const draftKey = 'gy:lesson-chat-draft:' + JSON.stringify(context);
    try { input.value = localStorage.getItem(draftKey) || ''; } catch (_) {}
    input.addEventListener('input', () => { try { localStorage.setItem(draftKey, input.value); } catch (_) {} });
    function addHistory(text, mine) {
      const row = document.createElement('article');
      row.className = mine ? 'chat-row chat-row-mine' : 'chat-row chat-row-ai';
      const bubble = document.createElement('div');
      bubble.className = mine ? 'chat-bubble chat-bubble-mine' : 'chat-bubble chat-bubble-ai';
      if (mine) { bubble.textContent = text; }
      else {
        const body = document.createElement('div');
        body.className = 'chat-body';
        body.innerHTML = chat.renderMarkdown(text, {math: true});
        bubble.appendChild(body);
      }
      row.appendChild(bubble); box.appendChild(row);
    }
    function topicName() {
      return ($('learn-topic-name')?.textContent || '').trim();
    }
    function finish(view, question, data) {
      view.complete(() => {
        const answer = view.text();
        if (data && data.cancelled) {
          const note = document.createElement('p');
          note.className = 'chat-error';
          note.textContent = '已停止，本次回答未保存。';
          view.bubble.appendChild(note);
          return;
        }
        const actions = view.addActions('<button type="button" class="chat-copy">复制</button><span class="chat-chips"></span>');
        const copyButton = actions.querySelector('.chat-copy');
        copyButton.addEventListener('click', async () => {
          try { await chat.copyText(answer); copyButton.textContent = '已复制'; }
          catch (_) { copyButton.textContent = '复制失败'; }
          setTimeout(() => { copyButton.textContent = '复制'; }, 1600);
        });
        const host = actions.querySelector('.chat-chips');
        const fallback = ['这一段能再讲细一点吗？', '举个例子说明一下', '为什么这里要用这个方法？'];
        fetch('/api/ask/suggestions', {method: 'POST', headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({question, answer, topic: topicName()})})
          .then(response => response.ok ? response.json() : null)
          .then(payload => {
            const list = ((payload && payload.suggestions) || []).filter(item => typeof item === 'string' && item.trim()).slice(0, 3);
            return list.length ? list : fallback;
          })
          .catch(() => fallback)
          .then(items => {
            host.innerHTML = '';
            items.forEach(text => {
              const chip = document.createElement('button');
              chip.type = 'button'; chip.className = 'chat-chip'; chip.textContent = text;
              chip.addEventListener('click', () => { input.value = text; input.focus(); form.requestSubmit(); });
              host.appendChild(chip);
            });
          });
      });
    }
    api('/api/conversations/' + encodeURIComponent(conversation))
      .then(data => (data.messages || []).forEach(item => addHistory(item.text, item.role === 'user'))).catch(() => {});
    let selected = '', socket = null;
    const stop = document.createElement('button'); stop.type = 'button'; stop.id = 'learning-chat-stop'; stop.textContent = '停止回答'; stop.hidden = true;
    form.appendChild(stop);
    stop.onclick = () => {
      if (!socket) return;
      const connection = socket;
      stop.disabled = true;
      const request = () => { try { connection.send(JSON.stringify({type: 'stop'})); } catch (_) {} };
      if (connection.readyState === WebSocket.OPEN) request();
      else if (connection.readyState === WebSocket.CONNECTING) connection.addEventListener('open', request, {once: true});
    };
    document.addEventListener('mouseup', () => {
      const text = window.getSelection()?.toString().trim() || '';
      if (!text || text.length > 2000 || !window.getSelection()?.anchorNode?.parentElement?.closest('#learn-content')) return;
      selected = text;
      let ask = $('ask-selection');
      if (!ask) { ask = document.createElement('button'); ask.id = 'ask-selection'; ask.type = 'button'; ask.textContent = '选中文字提问'; document.body.appendChild(ask); }
      const rect = window.getSelection().getRangeAt(0).getBoundingClientRect();
      ask.style.left = Math.min(window.innerWidth - 160, Math.max(8, rect.left)) + 'px';
      ask.style.top = Math.max(8, rect.bottom + 8) + 'px';
      ask.hidden = false;
      ask.onclick = () => { $('learning-chat-panel').classList.remove('hidden'); input.value = '这段内容是什么意思？'; input.focus(); ask.hidden = true; };
    });
    form.onsubmit = event => {
      event.preventDefault();
      const question = input.value.trim(); if (!question || socket) return;
      const sentSelection = selected;
      addHistory(question, true);
      input.value = '';
      try { localStorage.removeItem(draftKey); } catch (_) {}
      const send = $('learning-chat-send'); send.disabled = true; stop.hidden = false; stop.disabled = false;
      const view = chat.createAssistantView(box, {scrollRoot: box});
      let finished = false;
      const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
      socket = new WebSocket(protocol + '//' + location.host + '/ws/classroom');
      socket.onopen = () => socket.send(JSON.stringify({...context, type: 'ask', question, selection: sentSelection}));
      socket.onmessage = event => {
        const data = JSON.parse(event.data);
        if (data.type === 'delta') view.push(data.text);
        if (data.type === 'done') { finished = true; finish(view, question, data); socket.close(); }
        if (data.type === 'error') { finished = true; view.fail(data.message); socket.close(); }
      };
      socket.onerror = () => { finished = true; view.fail('连接中断，请重试。'); };
      socket.onclose = () => {
        if (!finished) view.fail('回答中断，本次未完成。');
        socket = null; send.disabled = false; stop.hidden = true; stop.disabled = false; selected = ''; input.focus();
      };
    };
  }

  function setupIdlePrompt() {
    const prompt = document.createElement('aside');
    prompt.id = 'classroom-idle'; prompt.hidden = true;
    prompt.innerHTML = '<span>停了一会儿？可以试着用自己的话说出本节关键概念。</span><button type="button" aria-label="关闭提示">×</button>';
    document.body.appendChild(prompt);
    let lastActive = Date.now(), lastPrompt = 0, count = 0;
    const active = () => { lastActive = Date.now(); prompt.hidden = true; };
    ['pointerdown', 'keydown', 'scroll'].forEach(name => document.addEventListener(name, active, {passive: true}));
    document.addEventListener('focusout', active);
    window.addEventListener('focus', active);
    prompt.querySelector('button').onclick = active;
    setInterval(() => {
      const typing = ['INPUT', 'TEXTAREA'].includes(document.activeElement?.tagName);
      const now = Date.now();
      if (!prompt.hidden && now - lastPrompt >= 15000) { prompt.hidden = true; lastActive = now; }
      if (count >= 2 || !prompt.hidden || !document.hasFocus() || typing) return;
      if (now - lastActive >= 120000 && now - lastPrompt >= 120000) {
        prompt.hidden = false; lastPrompt = now; count++;
      }
    }, 5000);
  }

  setupWeeklyPlan(); setupReviews(); setupLearn();
})();

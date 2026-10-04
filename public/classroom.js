(() => {
  'use strict';
  const params = new URLSearchParams(location.search);
  const courseId = params.get('courseId') || '';
  const phaseIndex = Number(params.get('phaseIndex') || 1);
  const topicIndex = Number(params.get('topicIndex') || 1);
  const $ = id => document.getElementById(id);
  const escape = value => { const node = document.createElement('span'); node.textContent = String(value ?? ''); return node.innerHTML; };
  const context = {courseId, phaseIndex, topicIndex};
  async function api(path, data) {
    const response = await fetch(path, data ? {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(data)} : undefined);
    const result = await response.json();
    if (!response.ok) throw new Error(result.error || '请求失败，请重试');
    return result;
  }

  async function setupWeeklyPlan() {
    if (courseId && $('plan-view')) window.GangyiLearning.mountStudyPlan('#plan-view');
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
      section.innerHTML = '<h2>待复习</h2>' + (items.length ? '<ul>' + items.map(({course, item}) => '<li><a href="/learn?' + new URLSearchParams({courseId: course.id, phaseIndex: item.phaseIndex, topicIndex: item.topicIndex, review: item.day, reviewId: item.reviewId}).toString() + '">' + escape(course.title + ' · ' + item.title + ' · 到期 ' + item.due) + '</a></li>').join('') + '</ul>' : '<p>目前没有到期复习。</p>');
    } catch (_) { section.querySelector('p').textContent = '复习清单暂未更新，课程仍可继续。'; }
  }

  function setupLearn() {
    if (!courseId || !$('learn-loading')) return;
    const base = {courseId, phaseIndex, topicIndex};
    const lessonSection = $('learn-loading').parentElement;
    const diagnostic = document.createElement('section');
    diagnostic.id = 'classroom-diagnostic'; diagnostic.className = 'classroom-card';
    diagnostic.innerHTML = '<h2>课前诊断</h2><p role="status" id="diagnostic-status">正在准备两道短题；课堂讲解同时在后台生成。</p><div id="diagnostic-questions"></div><button type="button" id="diagnostic-skip">跳过诊断，阅读完整课堂</button>';
    lessonSection.prepend(diagnostic);
    const interaction = document.createElement('section');
    interaction.id = 'classroom-interaction'; interaction.className = 'classroom-card';
    interaction.innerHTML = '<h2>边学边互动</h2><p role="status" id="interaction-status">预测、讲给 AI 听、新情境应用正在准备；活动可跳过。</p><div id="interaction-questions"></div>';
    $('learn-content').prepend(interaction);
    const branch = document.createElement('section');
    branch.id = 'classroom-branch'; branch.className = 'classroom-card'; branch.hidden = true;
    diagnostic.after(branch);
    const guidance = document.createElement('aside');
    guidance.id = 'classroom-guidance'; guidance.className = 'classroom-card'; guidance.hidden = true;
    guidance.innerHTML = '<h2>想一想</h2><p></p><button type="button">稍后再说</button>';
    branch.after(guidance);
    guidance.querySelector('button').onclick = () => { guidance.hidden = true; };
    let guidedQuestion = '';
    function guide() { if (guidedQuestion) { guidance.querySelector('p').textContent = guidedQuestion; guidance.hidden = false; } }
    document.addEventListener('click', event => { if (event.target.closest('.step-toggle')) guide(); });
    const next = document.createElement('section');
    next.id = 'classroom-next'; next.className = 'classroom-card'; next.hidden = true;
    $('complete-lesson').after(next);
    function showNext(result) {
      next.hidden = false;
      next.innerHTML = '<h2>下一步</h2><p>' + (result.challengeRequired ? '先完成新情境挑战题，再形成掌握证据。' : result.nextStep === 'remedial' ? '建议先做短补弱并在明天复测。' : '明天开始短复习，后续第 3、7 天继续巩固。') + '</p><div id="next-due-reviews"></div><a href="/plan?courseId=' + encodeURIComponent(courseId) + '">查看课程路径与新课 →</a>';
      api('/api/classroom/reviews?courseId=' + encodeURIComponent(courseId)).then(data => {
        const due = data.items || [];
        if (due.length) $('next-due-reviews').textContent = '已到期复习：' + due.map(item => item.title).join('、');
      }).catch(() => {});
      if (result.nextStep === 'remedial' && document.body.dataset.classroomMode !== 'weak') {
        branch.hidden = false;
        branch.innerHTML = '<h2>短补弱</h2><p>正在根据本节表现准备针对性补讲…</p>';
        api('/api/classroom/remedial?' + new URLSearchParams(base)).then(data => {
          branch.innerHTML = '<h2>' + escape(data.title) + ' · 约 ' + data.minutes + ' 分钟</h2><p class="whitespace-pre-wrap">' + escape(data.content) + '</p><p><b>自检：</b>' + escape(data.check) + '</p>';
        }).catch(() => { branch.querySelector('p').textContent = '补讲暂未就绪，可以先回看原课堂。'; });
      }
    }
    let appliedMode = '', thirdLoading = false;
    function applyMode(mode) {
      if (mode === appliedMode) return;
      if (mode === 'third') {
        if (!thirdLoading && $('diagnostic-questions').children.length < 3) {
          thirdLoading = true;
          api('/api/classroom/start?' + new URLSearchParams({...base, kind: 'diagnostic'})).then(data => {
            if (data.questions[2] && $('diagnostic-questions').children.length < 3)
              $('diagnostic-questions').append(renderItem('diagnostic', data.questions[2], 2));
            appliedMode = 'third'; $('diagnostic-status').textContent = '前两题结果不明确，请继续第三题。';
          }).finally(() => { thirdLoading = false; });
        }
        return;
      }
      if (!['weak', 'full', 'familiar'].includes(mode)) return;
      appliedMode = mode;
      document.body.dataset.classroomMode = mode;
      diagnostic.querySelector('#diagnostic-status').textContent = mode === 'weak' ? '诊断显示需要先补弱。' : mode === 'familiar' ? '已熟悉：精简重复解释，请完成新情境挑战题。' : '展示完整课堂。';
      branch.hidden = false;
      if (mode === 'weak') {
        branch.innerHTML = '<h2>7 分钟针对性补讲</h2><p>正在结合诊断错误准备补讲…</p>';
        api('/api/classroom/remedial?' + new URLSearchParams(base)).then(data => {
          branch.innerHTML = '<h2>' + escape(data.title) + ' · 约 ' + data.minutes + ' 分钟</h2><p class="whitespace-pre-wrap">' + escape(data.content) + '</p><p><b>自检：</b>' + escape(data.check) + '</p>';
        }).catch(() => { branch.innerHTML = '<h2>先读完整课堂</h2><p>针对性补讲暂未就绪，原课程可以继续。</p>'; });
      } else if (mode === 'familiar') branch.innerHTML = '<h2>精简已熟悉内容</h2><p>重复解释已收起；新情境应用题是掌握证据的必要条件。</p><button type="button" id="show-all-steps">展开完整解释</button>';
      else branch.innerHTML = '<h2>完整课堂</h2><p>可按自己的节奏阅读、互动和测验。</p>';
      $('show-all-steps')?.addEventListener('click', () => { document.body.dataset.classroomMode = 'full'; branch.querySelector('p').textContent = '完整解释已展开。'; });
    }
    document.addEventListener('gangyi:diagnostic-mode', event => applyMode(event.detail));
    function renderItem(kind, item, index) {
      return window.GangyiLearning.renderDialogue(kind, item, index, base);
    }
    async function startKind(kind) {
      const status = $(kind === 'diagnostic' ? 'diagnostic-status' : 'interaction-status');
      const box = $(kind === 'diagnostic' ? 'diagnostic-questions' : 'interaction-questions');
      try {
        const data = await api('/api/classroom/start?' + new URLSearchParams({...base, kind}));
        box.replaceChildren(...data.questions.map((item, i) => renderItem(kind, item, i)));
        status.textContent = kind === 'diagnostic' ? '先完成两题；结果不明确时追加第三题。' : '可按任意顺序参与，也可以跳过。';
        if (kind === 'diagnostic') { $('diagnostic-skip').hidden = !!data.skipped; applyMode(data.mode); }
        else guidedQuestion = data.questions[0]?.question || '';
      } catch (error) { status.textContent = error.message + ' 可继续原课程。'; }
    }
    $('diagnostic-skip').onclick = async () => {
      try { const result = await api('/api/classroom/skip', base); $('diagnostic-questions').replaceChildren(); $('diagnostic-skip').hidden = true; applyMode(result.mode); }
      catch (error) { $('diagnostic-status').textContent = error.message; }
    };
    startKind('diagnostic'); startKind('interaction');
    api('/api/classroom/state?' + new URLSearchParams(base)).then(state => { if (state.completed) showNext({nextStep: state.quizPassed ? 'review' : 'remedial'}); }).catch(() => {});
    const saveLessonProgress = $('complete-lesson').onclick;
    $('complete-lesson').onclick = async event => {
      try {
        const response = await saveLessonProgress.call($('complete-lesson'), event);
        if (!response.ok) throw new Error('本节进度尚未保存');
        showNext(await api('/api/classroom/finish', base));
      }
      catch (_) { next.hidden = false; next.innerHTML = '<h2>下一步</h2><p>原课程与下一课入口仍可使用；稍后可重试保存复习安排。</p>'; }
    };
    const quizResult = $('quiz-result');
    new MutationObserver(() => { if (quizResult.textContent.includes('得分')) {
      const status = $('interaction-status'); if (status) status.textContent = '测验已完成：可以回看互动反馈或继续下一步。';
      guide();
    }}).observe(quizResult, {childList: true, characterData: true, subtree: true});
    setupChat(); setupIdlePrompt(); setupReviewMode();
  }

  function setupReviewMode() {
    if (!params.has('review') || !$('quiz-submit')) return;
    $('quiz-submit').onclick = async () => {
      const answers = [...$('learn-quiz').querySelectorAll('fieldset')].map((_, i) => {
        const selected = document.querySelector('input[name="q' + i + '"]:checked'); return selected ? (selected.value === "unknown" ? "unknown" : Number(selected.value)) : null;
      });
      try {
        const result = await api('/api/classroom/review/submit', {...context, day: Number(params.get('review')),
          reviewId: params.get('reviewId') || '', questions: window.gangyiLessonBlocks?.quiz?.quiz || [], answers});
        $('quiz-result').textContent = '复习得分 ' + result.score + ' / ' + result.total + (result.passed ? '，已通过' : '，明天再练');
        if (result.passed) {
          const next = $('classroom-next');
          next.hidden = false;
          next.innerHTML = '<h2>复习通过</h2><p>返回原课程路径，或直接继续新课。</p><a href="/plan?courseId=' + encodeURIComponent(courseId) + '">返回原课程路径 →</a>';
        }
      } catch (error) { $('quiz-result').textContent = error.message; }
    };
  }

  function setupChat() {
    const form = $('learning-chat-form'), input = $('learning-chat-input'), box = $('learning-chat-messages');
    if (!form || !input || !box) return;
    const chat = window.GangyiChat;
    const conversation = 'lesson-' + courseId + '-' + phaseIndex + '-' + topicIndex;
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

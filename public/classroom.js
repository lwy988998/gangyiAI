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

  function setupAvailability() {
    const form = $('goal-form');
    if (!form || $('home-availability')) return;
    const defaults = [{weekday: 1, minutes: 30}, {weekday: 3, minutes: 30}, {weekday: 5, minutes: 30}];
    let saved = defaults;
    try { saved = JSON.parse(localStorage.getItem('gangyi-week-availability') || 'null') || defaults; } catch (_) {}
    const names = ['周一', '周二', '周三', '周四', '周五', '周六', '周日'];
    const section = document.createElement('fieldset');
    section.id = 'home-availability';
    section.className = 'classroom-card availability-card';
    section.innerHTML = '<legend>每周可学习时间</legend><p>预填每周 3 天、每天 30 分钟；课程生成后仍可修改。</p><div class="availability-grid"></div>';
    const grid = section.querySelector('.availability-grid');
    names.forEach((name, i) => {
      const slot = saved.find(item => item.weekday === i + 1);
      const label = document.createElement('label');
      label.innerHTML = '<input type="checkbox" data-weekday="' + (i + 1) + '" ' + (slot ? 'checked' : '') + '><span>' + name + '</span><input type="number" min="10" max="240" step="5" value="' + (slot?.minutes || 30) + '" aria-label="' + name + '分钟数"><span>分钟</span>';
      grid.appendChild(label);
    });
    function save() {
      const availability = [...grid.querySelectorAll('label')].filter(label => label.querySelector('[type=checkbox]').checked)
        .map(label => ({weekday: Number(label.querySelector('[type=checkbox]').dataset.weekday), minutes: Math.max(10, Math.min(240, Number(label.querySelector('[type=number]').value) || 30))}));
      if (availability.length) {
        localStorage.setItem('gangyi-week-availability', JSON.stringify(availability));
        localStorage.setItem('gangyi-week-pending', '1');
      }
    }
    section.addEventListener('change', save);
    form.appendChild(section);
  }

  async function setupWeeklyPlan() {
    if (!courseId || !$('plan-view')) return;
    const container = document.createElement('section');
    container.id = 'weekly-plan';
    container.className = 'classroom-card weekly-plan';
    container.innerHTML = '<h2>本周学习安排</h2><p id="weekly-message" role="status">正在读取可编辑初稿…</p><div id="weekly-availability"></div><div id="weekly-entries"></div><div class="classroom-actions"><button type="button" id="weekly-save">保存修改</button><button type="button" id="weekly-replan">根据最新表现重排</button></div><div id="weekly-confirm" hidden></div>';
    $('plan-view').before(container);
    let plan;
    const names = ['周一', '周二', '周三', '周四', '周五', '周六', '周日'];
    function render() {
      $('weekly-message').textContent = '可修改日期、顺序和时长；手动安排不会被自动覆盖。';
      $('weekly-availability').innerHTML = '<h3>可用时间</h3><div class="availability-grid">' + names.map((name, i) => {
        const slot = plan.availability.find(item => item.weekday === i + 1);
        return '<label><input type="checkbox" data-weekday="' + (i + 1) + '" ' + (slot ? 'checked' : '') + '><span>' + name + '</span><input type="number" min="10" max="240" step="5" value="' + (slot?.minutes || 30) + '" aria-label="' + name + '分钟数"><span>分钟</span></label>';
      }).join('') + '</div>';
      $('weekly-entries').innerHTML = '<h3>每日安排</h3>' + (plan.entries.length ? plan.entries.map((entry, i) => '<div class="weekly-row" data-index="' + i + '"><input type="date" value="' + escape(entry.date) + '" aria-label="学习日期"><strong>' + escape(entry.title) + '</strong><input type="number" min="5" max="240" value="' + entry.minutes + '" aria-label="学习分钟数"><span>分钟</span><button type="button" data-up="' + i + '" aria-label="上移">↑</button><button type="button" data-down="' + i + '" aria-label="下移">↓</button></div>').join('') : '<p>本周暂无活动，请设置可用时间。</p>');
    }
    function gather() {
      const availability = [...$('weekly-availability').querySelectorAll('label')].filter(label => label.querySelector('[type=checkbox]').checked)
        .map(label => ({weekday: Number(label.querySelector('[type=checkbox]').dataset.weekday), minutes: Number(label.querySelector('[type=number]').value)}));
      const entries = [...$('weekly-entries').querySelectorAll('.weekly-row')].map((row, i) => ({...plan.entries[Number(row.dataset.index)], date: row.querySelector('[type=date]').value, minutes: Number(row.querySelector('[type=number]').value), order: i}));
      return {availability, entries};
    }
    async function save() {
      try { plan = await api('/api/classroom/week/edit', {courseId, version: plan.version, ...gather()}); render(); $('weekly-message').textContent = '周计划已保存。'; }
      catch (error) { $('weekly-message').textContent = error.message; }
    }
    $('weekly-save').onclick = save;
    $('weekly-replan').onclick = async () => {
      try {
        const result = await api('/api/classroom/week/replan', {courseId, confirm: false});
        if (!result.requiresConfirmation) { plan = result.plan; render(); return; }
        const box = $('weekly-confirm');
        box.hidden = false;
        box.innerHTML = '<h3>重排会覆盖手动安排</h3><p>当前安排：' + result.current.entries.map(entry => escape(entry.date + ' ' + entry.title + ' ' + entry.minutes + '分钟')).join('；') + '</p><p>建议安排：' + result.proposed.map(entry => escape(entry.date + ' ' + entry.title + ' ' + entry.minutes + '分钟')).join('；') + '</p><button type="button" id="weekly-accept">确认覆盖</button><button type="button" id="weekly-cancel">保留手动安排</button>';
        $('weekly-cancel').onclick = () => { box.hidden = true; };
        $('weekly-accept').onclick = async () => {
          try { const accepted = await api('/api/classroom/week/replan', {courseId, confirm: true}); plan = accepted.plan; box.hidden = true; render(); }
          catch (error) { $('weekly-message').textContent = error.message; }
        };
      } catch (error) { $('weekly-message').textContent = error.message; }
    };
    $('weekly-entries').onclick = event => {
      const up = event.target.closest('[data-up]'), down = event.target.closest('[data-down]');
      if (!up && !down) return;
      const gathered = gather(); plan.availability = gathered.availability; plan.entries = gathered.entries;
      const i = Number((up || down).dataset[up ? 'up' : 'down']);
      const j = i + (up ? -1 : 1);
      if (j < 0 || j >= plan.entries.length) return;
      [plan.entries[i], plan.entries[j]] = [plan.entries[j], plan.entries[i]];
      render();
    };
    try {
      plan = await api('/api/classroom/week?courseId=' + encodeURIComponent(courseId));
      if (localStorage.getItem('gangyi-week-pending') === '1') {
        const availability = JSON.parse(localStorage.getItem('gangyi-week-availability') || 'null');
        if (Array.isArray(availability) && availability.length) plan = await api('/api/classroom/week/edit', {courseId, version: plan.version, availability});
        localStorage.removeItem('gangyi-week-pending');
      }
      render();
    } catch (error) { $('weekly-message').textContent = error.message; }
  }

  async function setupReviews() {
    if (!['/', '/my-courses'].includes(location.pathname)) return;
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
    function applyMode(mode) {
      if (!['weak', 'full', 'familiar'].includes(mode)) return;
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
    function renderItem(kind, item, index) {
      const row = document.createElement('article');
      row.className = 'classroom-question'; row.dataset.index = index;
      const answered = item.status === 'answered' || item.status === 'skipped';
      const options = item.type === 'open' ? '<textarea rows="3" aria-label="你的回答" placeholder="用自己的话说一说"></textarea>' : (item.options || []).map((option, i) => '<label><input type="radio" name="' + kind + '-' + index + '" value="' + i + '"><span>' + escape(option) + '</span></label>').join('');
      row.innerHTML = '<h3>' + escape(item.question) + '</h3><div class="classroom-options">' + options + '</div><div class="classroom-actions"><button type="button" data-submit>提交回答</button>' + (kind === 'interaction' ? '<button type="button" data-skip>跳过活动</button>' : '') + '</div><p role="status" class="classroom-feedback"></p>';
      const feedback = row.querySelector('.classroom-feedback');
      if (answered) { row.querySelector('.classroom-actions').hidden = true; feedback.textContent = item.feedback || (item.status === 'skipped' ? '已跳过' : '已作答'); }
      row.querySelector('[data-submit]').onclick = async () => {
        const answer = item.type === 'open' ? row.querySelector('textarea').value.trim() : Number(row.querySelector('input:checked')?.value);
        if (item.type === 'open' && !answer || item.type !== 'open' && !row.querySelector('input:checked')) { feedback.textContent = '请先作答。'; return; }
        row.querySelector('[data-submit]').disabled = true; feedback.textContent = '正在校验答案…';
        try {
          const result = await api('/api/classroom/submit', {...base, kind, index, answer});
          feedback.textContent = result.feedback + (result.followUp ? ' 追问：' + result.followUp : '') + (!result.credible ? ' 此评价未计入掌握证据。' : '');
          row.querySelector('.classroom-actions').hidden = true;
          if (!result.correct && kind === 'interaction') {
            const hint = document.createElement('button'); hint.type = 'button'; hint.textContent = '逐级提示';
            feedback.after(hint);
            hint.onclick = async () => { try { const data = await api('/api/classroom/hint', {...base, kind, index}); feedback.textContent += ' 提示 ' + data.level + '：' + data.hint; if (data.level >= 3) hint.remove(); } catch (error) { feedback.textContent = error.message; } };
          }
          if (kind === 'diagnostic') {
            if (result.nextQuestion) $('diagnostic-questions').appendChild(renderItem('diagnostic', result.nextQuestion, 2));
            applyMode(result.mode);
          } else guide();
        } catch (error) { row.querySelector('[data-submit]').disabled = false; feedback.textContent = error.message; }
      };
      row.querySelector('[data-skip]')?.addEventListener('click', async () => {
        try { await api('/api/classroom/activity/skip', {...base, index}); row.querySelector('.classroom-actions').hidden = true; feedback.textContent = '已跳过；可以继续阅读。'; }
        catch (error) { feedback.textContent = error.message; }
      });
      return row;
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
        const selected = document.querySelector('input[name="q' + i + '"]:checked'); return selected ? Number(selected.value) : null;
      });
      try {
        const result = await api('/api/classroom/review/submit', {...context, day: Number(params.get('review')),
          reviewId: params.get('reviewId') || '', answers});
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
    api('/api/conversations/lesson-' + encodeURIComponent(courseId) + '-' + phaseIndex + '-' + topicIndex)
      .then(data => (data.messages || []).forEach(item => {
        const line = document.createElement('p');
        line.className = item.role === 'user' ? 'classroom-chat-user' : 'classroom-chat-reply';
        line.textContent = item.text; box.appendChild(line);
      })).catch(() => {});
    let selected = '', socket = null;
    const stop = document.createElement('button'); stop.type = 'button'; stop.id = 'learning-chat-stop'; stop.textContent = '停止回答'; stop.hidden = true;
    form.appendChild(stop);
    stop.onclick = () => { if (socket) { if (socket.readyState === WebSocket.OPEN) socket.send(JSON.stringify({type: 'stop'})); socket.close(); } stop.hidden = true; };
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
      const mine = document.createElement('p'); mine.className = 'classroom-chat-user'; mine.textContent = question; box.appendChild(mine);
      const reply = document.createElement('p'); reply.className = 'classroom-chat-reply'; box.appendChild(reply);
      box.scrollTop = box.scrollHeight; input.value = '';
      const send = $('learning-chat-send'); send.disabled = true; stop.hidden = false;
      const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
      socket = new WebSocket(protocol + '//' + location.host + '/ws/classroom');
      socket.onopen = () => socket.send(JSON.stringify({...context, type: 'ask', question, selection: selected}));
      socket.onmessage = event => {
        const data = JSON.parse(event.data);
        if (data.type === 'delta') reply.textContent += data.text;
        if (data.type === 'error' || data.cancelled) reply.textContent += '\n' + data.message;
        if (data.type === 'done' || data.type === 'error') socket.close();
        box.scrollTop = box.scrollHeight;
      };
      socket.onerror = () => { reply.textContent += '\n连接中断，请重试。'; };
      socket.onclose = () => { socket = null; send.disabled = false; stop.hidden = true; selected = ''; input.focus(); };
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

  setupAvailability(); setupWeeklyPlan(); setupReviews(); setupLearn();
})();

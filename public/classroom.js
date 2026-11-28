// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 声明前端变量 params，保存本行计算出的数据或对象。
  const params = new URLSearchParams(location.search);
  // 【逐行批注】 声明前端变量 courseId，保存本行计算出的数据或对象。
  const courseId = params.get('courseId') || '';
  // 【逐行批注】 声明前端变量 phaseIndex，保存本行计算出的数据或对象。
  const phaseIndex = Number(params.get('phaseIndex') || 1);
  // 【逐行批注】 声明前端变量 topicIndex，保存本行计算出的数据或对象。
  const topicIndex = Number(params.get('topicIndex') || 1);
  // 【逐行批注】 声明前端变量 $，保存本行计算出的数据或对象。
  const $ = id => document.getElementById(id);
  // 【逐行批注】 声明前端变量 escape，保存本行计算出的数据或对象。
  const escape = value => { const node = document.createElement('span'); node.textContent = String(value ?? ''); return node.innerHTML; };
  // 【逐行批注】 声明前端变量 context，保存本行计算出的数据或对象。
  const context = {courseId, phaseIndex, topicIndex, ...(params.get('lessonTaskId') ? {lessonTaskId: params.get('lessonTaskId')} : {}), ...(params.has('review') ? {day: Number(params.get('review')), reviewId: params.get('reviewId') || ''} : {})};
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('gangyi:lesson-context', event => Object.assign(context, e…。
  document.addEventListener('gangyi:lesson-context', event => Object.assign(context, event.detail));
  // 【逐行批注】 定义函数 api，封装可复用的前端行为。
  async function api(path, data) {
    // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
    const response = await fetch(path, data ? {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(data)} : undefined);
    // 【逐行批注】 声明前端变量 result，保存本行计算出的数据或对象。
    const result = await response.json();
    // 【逐行批注】 判断条件 (!response.ok) throw new Error(result.error || '请求失败，请重试');，成立时执行对应逻辑。
    if (!response.ok) throw new Error(result.error || '请求失败，请重试');
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 setupWeeklyPlan，封装可复用的前端行为。
  async function setupWeeklyPlan() {
    // 学习安排只在我的课程中显示。
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 setupReviews，封装可复用的前端行为。
  async function setupReviews() {
    // 【逐行批注】 判断条件 (location.pathname !== '/') return;，成立时执行对应逻辑。
    if (location.pathname !== '/') return;
    // 【逐行批注】 声明前端变量 main，保存本行计算出的数据或对象。
    const main = document.querySelector('main');
    // 【逐行批注】 判断条件 (!main) return;，成立时执行对应逻辑。
    if (!main) return;
    // 【逐行批注】 声明前端变量 section，保存本行计算出的数据或对象。
    const section = document.createElement('section');
    // 【逐行批注】 计算右侧结果并更新 section.id，改变页面或请求状态。
    section.id = 'due-reviews'; section.className = 'classroom-card due-reviews';
    // 【逐行批注】 计算右侧结果并更新 section.innerHTML，改变页面或请求状态。
    section.innerHTML = '<h2>待复习</h2><p role="status">正在检查到期复习…</p>';
    // 【逐行批注】 执行当前前端表达式：main.prepend(section);。
    main.prepend(section);
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 courses，保存本行计算出的数据或对象。
      const courses = (await api('/api/courses')).courses || [];
      // 【逐行批注】 声明前端变量 results，保存本行计算出的数据或对象。
      const results = await Promise.all(courses.map(async course => ({course, items: (await api('/api/classroom/reviews?courseId=' + encodeURIComponent(course.id))).items || []})));
      // 【逐行批注】 声明前端变量 items，保存本行计算出的数据或对象。
      const items = results.flatMap(({course, items}) => items.map(item => ({course, item})));
      // 【逐行批注】 计算右侧结果并更新 section.innerHTML，改变页面或请求状态。
      section.innerHTML = '<details><summary>待复习 · ' + items.length + ' 项</summary>' + (items.length ? '<ul>' + items.map(({course, item}) => '<li><a href="/learn?' + new URLSearchParams({courseId: course.id, phaseIndex: item.phaseIndex, topicIndex: item.topicIndex, review: item.day, reviewId: item.reviewId}).toString() + '">' + escape(course.title + ' · ' + item.title + ' · 到期 ' + item.due) + '</a></li>').join('') + '</ul>' : '<p>目前没有到期复习。</p>') + '</details>';
    // 【逐行批注】 计算右侧结果并更新 } catch (_) { section.querySelector('p').tex…，改变页面或请求状态。
    } catch (_) { section.querySelector('p').textContent = '复习清单暂未更新，课程仍可继续。'; }
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 setupLearn，封装可复用的前端行为。
  function setupLearn() {
    // 【逐行批注】 判断条件 (!courseId || !$('learn-loading')) return;，成立时执行对应逻辑。
    if (!courseId || !$('learn-loading')) return;
    // 【逐行批注】 判断条件 (params.has('review') || window.gangyiLessonKind === 'review') $…，成立时执行对应逻辑。
    if (params.has('review') || window.gangyiLessonKind === 'review') $('complete-lesson').textContent = '保存复习记录';
    // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('gangyi:lesson-context', () => { if (window.gangyiLessonKi…。
    document.addEventListener('gangyi:lesson-context', () => { if (window.gangyiLessonKind === 'review') $('complete-lesson').textContent = '保存复习记录'; });
    // 【逐行批注】 声明前端变量 lessonSection，保存本行计算出的数据或对象。
    const lessonSection = $('learn-loading').parentElement;
    // 【逐行批注】 声明前端变量 diagnostic，保存本行计算出的数据或对象。
    const diagnostic = document.createElement('section'); diagnostic.id = 'classroom-diagnostic'; diagnostic.className = 'classroom-card';
    // 【逐行批注】 计算右侧结果并更新 diagnostic.innerHTML，改变页面或请求状态。
    diagnostic.innerHTML = '<h2>课前诊断</h2><p>可以先回答、请求提示，或暂时跳过；漏答只表示缺少信息。</p><div id="diagnostic-questions"></div>';
    // 【逐行批注】 执行当前前端表达式：lessonSection.prepend(diagnostic);。
    lessonSection.prepend(diagnostic);
    // 【逐行批注】 声明前端变量 interaction，保存本行计算出的数据或对象。
    const interaction = document.createElement('section'); interaction.id = 'classroom-interaction'; interaction.className = 'classroom-card';
    // 【逐行批注】 计算右侧结果并更新 interaction.innerHTML，改变页面或请求状态。
    interaction.innerHTML = '<h2>边学边互动</h2><p>输入自己的理解或解题方法，AI 会结合实际回答帮助你。</p><div id="interaction-questions"></div>';
    // 【逐行批注】 执行当前前端表达式：$('learn-content').prepend(interaction);。
    $('learn-content').prepend(interaction);
    // 【逐行批注】 声明前端变量 diagnosticStarted，保存本行计算出的数据或对象。
    let diagnosticStarted = false;
    // 【逐行批注】 定义函数 loadActivities，封装可复用的前端行为。
    async function loadActivities() {
      // 【逐行批注】 判断条件 (diagnosticStarted) return; diagnosticStarted = true;，成立时执行对应逻辑。
      if (diagnosticStarted) return; diagnosticStarted = true;
      // 【逐行批注】 执行当前前端表达式：await Promise.allSettled([。
      await Promise.allSettled([
        // 【逐行批注】 执行当前前端表达式：window.GangyiLearning.mountQuestionKind('diagnostic', '#diagnostic-questions'),。
        window.GangyiLearning.mountQuestionKind('diagnostic', '#diagnostic-questions'),
        // 【逐行批注】 执行当前前端表达式：window.GangyiLearning.mountQuestionKind('interaction', '#interaction-questions')]);。
        window.GangyiLearning.mountQuestionKind('interaction', '#interaction-questions')]);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('gangyi:lesson-block', event => { if (event.detail.block =…。
    document.addEventListener('gangyi:lesson-block', event => { if (event.detail.block === 'quiz') loadActivities(); });
    // 【逐行批注】 判断条件 (window.gangyiLessonBlocks?.quiz) loadActivities();，成立时执行对应逻辑。
    if (window.gangyiLessonBlocks?.quiz) loadActivities();
    // 【逐行批注】 声明前端变量 next，保存本行计算出的数据或对象。
    const next = document.createElement('section'); next.id = 'classroom-next'; next.className = 'classroom-card'; next.hidden = true;
    // 【逐行批注】 执行当前前端表达式：$('complete-lesson').after(next);。
    $('complete-lesson').after(next);
    // 【逐行批注】 定义函数 showNext，封装可复用的前端行为。
    function showNext() { next.hidden = false; next.innerHTML = '<h2>本节记录已保存</h2><p>可以继续与 AI 交流，或让 AI 综合最新作答、课程目标和共享学习时间准备下一课。</p>'; }
    // 【逐行批注】 计算右侧结果并更新 api('/api/classroom/state?' + new URLSearchP…，改变页面或请求状态。
    api('/api/classroom/state?' + new URLSearchParams(context)).then(state => { if (state.completed) showNext(); }).catch(() => {});
    // 【逐行批注】 声明前端变量 saveLessonProgress，保存本行计算出的数据或对象。
    const saveLessonProgress = $('complete-lesson').onclick;
    // 【逐行批注】 计算右侧结果并更新 $('complete-lesson').onclick，改变页面或请求状态。
    $('complete-lesson').onclick = async event => {
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 判断条件 (params.has('review') || window.gangyiLessonKind === 'review') {，成立时执行对应逻辑。
        if (params.has('review') || window.gangyiLessonKind === 'review') {
          // 【逐行批注】 声明前端变量 result，保存本行计算出的数据或对象。
          const result = await api('/api/classroom/finish', {...context, contentVersion: window.gangyiContentVersion || 1});
          // 【逐行批注】 判断条件 (result.ok === false) throw new Error(result.message || '复习尚未保存'…，成立时执行对应逻辑。
          if (result.ok === false) throw new Error(result.message || '复习尚未保存');
          // 【逐行批注】 判断条件 (result.evaluationStatus === 'insufficient') { $('complete-feedb…，成立时执行对应逻辑。
          if (result.evaluationStatus === 'insufficient') { $('complete-feedback').textContent = result.message || '尚无可靠作答，未推测掌握情况。'; return; }
          // 【逐行批注】 计算右侧结果并更新 $('complete-feedback').textContent，改变页面或请求状态。
          $('complete-feedback').textContent = result.message || (result.passed ? '可靠复习评价已保存。' : '复习记录已保存，下一课会结合实际反馈安排。');
          // 【逐行批注】 计算右侧结果并更新 $('complete-lesson').textContent，改变页面或请求状态。
          $('complete-lesson').textContent = '复习记录已保存'; showNext(); return;
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
        const response = await saveLessonProgress.call($('complete-lesson'), event);
        // 【逐行批注】 判断条件 (!response.ok) throw new Error('本节进度尚未保存');，成立时执行对应逻辑。
        if (!response.ok) throw new Error('本节进度尚未保存');
        // 【逐行批注】 调用 api、showNext 执行页面交互或数据处理：await api('/api/classroom/finish', context); showNext();。
        await api('/api/classroom/finish', context); showNext();
      // 【逐行批注】 计算右侧结果并更新 } catch (error) { next.hidden，改变页面或请求状态。
      } catch (error) { next.hidden = false; next.textContent = error.message + '。可以稍后重试，下一课仍可独立准备。'; }
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 调用 setupChat、setupIdlePrompt 执行页面交互或数据处理：setupChat(); setupIdlePrompt();。
    setupChat(); setupIdlePrompt();
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 setupChat，封装可复用的前端行为。
  function setupChat() {
    // 【逐行批注】 声明前端变量 form，保存本行计算出的数据或对象。
    const form = $('learning-chat-form'), input = $('learning-chat-input'), box = $('learning-chat-messages');
    // 【逐行批注】 判断条件 (!form || !input || !box) return;，成立时执行对应逻辑。
    if (!form || !input || !box) return;
    // 【逐行批注】 声明前端变量 chat，保存本行计算出的数据或对象。
    const chat = window.GangyiChat;
    // 【逐行批注】 声明前端变量 conversation，保存本行计算出的数据或对象。
    const conversation = context.lessonTaskId ? 'lesson-task-' + context.lessonTaskId : 'lesson-' + courseId + '-' + phaseIndex + '-' + topicIndex;
    // 【逐行批注】 声明前端变量 draftKey，保存本行计算出的数据或对象。
    const draftKey = 'gy:lesson-chat-draft:' + JSON.stringify(context);
    // 【逐行批注】 计算右侧结果并更新 try { input.value，改变页面或请求状态。
    try { input.value = localStorage.getItem(draftKey) || ''; } catch (_) {}
    // 【逐行批注】 为页面事件绑定处理逻辑：input.addEventListener('input', () => { try { localStorage.setItem(draftKey, input.v…。
    input.addEventListener('input', () => { try { localStorage.setItem(draftKey, input.value); } catch (_) {} });
    // 【逐行批注】 定义函数 addHistory，封装可复用的前端行为。
    function addHistory(text, mine) {
      // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
      const row = document.createElement('article');
      // 【逐行批注】 计算右侧结果并更新 row.className，改变页面或请求状态。
      row.className = mine ? 'chat-row chat-row-mine' : 'chat-row chat-row-ai';
      // 【逐行批注】 声明前端变量 bubble，保存本行计算出的数据或对象。
      const bubble = document.createElement('div');
      // 【逐行批注】 计算右侧结果并更新 bubble.className，改变页面或请求状态。
      bubble.className = mine ? 'chat-bubble chat-bubble-mine' : 'chat-bubble chat-bubble-ai';
      // 【逐行批注】 判断条件 (mine) { bubble.textContent = text; }，成立时执行对应逻辑。
      if (mine) { bubble.textContent = text; }
      // 【逐行批注】 执行前一条件不成立时的分支。
      else {
        // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
        const body = document.createElement('div');
        // 【逐行批注】 计算右侧结果并更新 body.className，改变页面或请求状态。
        body.className = 'chat-body';
        // 【逐行批注】 计算右侧结果并更新 body.innerHTML，改变页面或请求状态。
        body.innerHTML = chat.renderMarkdown(text, {math: true});
        // 【逐行批注】 执行当前前端表达式：bubble.appendChild(body);。
        bubble.appendChild(body);
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 执行当前前端表达式：row.appendChild(bubble); box.appendChild(row);。
      row.appendChild(bubble); box.appendChild(row);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 topicName，封装可复用的前端行为。
    function topicName() {
      // 【逐行批注】 结束当前函数并返回结果：return ($('learn-topic-name')?.textContent || '').trim();。
      return ($('learn-topic-name')?.textContent || '').trim();
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 finish，封装可复用的前端行为。
    function finish(view, question, data) {
      // 【逐行批注】 计算右侧结果并更新 view.complete(()，改变页面或请求状态。
      view.complete(() => {
        // 【逐行批注】 声明前端变量 answer，保存本行计算出的数据或对象。
        const answer = view.text();
        // 【逐行批注】 判断条件 (data && data.cancelled) {，成立时执行对应逻辑。
        if (data && data.cancelled) {
          // 【逐行批注】 声明前端变量 note，保存本行计算出的数据或对象。
          const note = document.createElement('p');
          // 【逐行批注】 计算右侧结果并更新 note.className，改变页面或请求状态。
          note.className = 'chat-error';
          // 【逐行批注】 计算右侧结果并更新 note.textContent，改变页面或请求状态。
          note.textContent = '已停止，本次回答未保存。';
          // 【逐行批注】 执行当前前端表达式：view.bubble.appendChild(note);。
          view.bubble.appendChild(note);
          // 【逐行批注】 结束当前函数并返回结果：return;。
          return;
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 声明前端变量 actions，保存本行计算出的数据或对象。
        const actions = view.addActions('<button type="button" class="chat-copy">复制</button><span class="chat-chips"></span>');
        // 【逐行批注】 声明前端变量 copyButton，保存本行计算出的数据或对象。
        const copyButton = actions.querySelector('.chat-copy');
        // 【逐行批注】 为页面事件绑定处理逻辑：copyButton.addEventListener('click', async () => {。
        copyButton.addEventListener('click', async () => {
          // 【逐行批注】 计算右侧结果并更新 try { await chat.copyText(answer); copyButto…，改变页面或请求状态。
          try { await chat.copyText(answer); copyButton.textContent = '已复制'; }
          // 【逐行批注】 计算右侧结果并更新 catch (_) { copyButton.textContent，改变页面或请求状态。
          catch (_) { copyButton.textContent = '复制失败'; }
          // 【逐行批注】 计算右侧结果并更新 setTimeout(()，改变页面或请求状态。
          setTimeout(() => { copyButton.textContent = '复制'; }, 1600);
        // 【逐行批注】 执行当前前端表达式：});。
        });
        // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
        const host = actions.querySelector('.chat-chips');
        // 【逐行批注】 声明前端变量 fallback，保存本行计算出的数据或对象。
        const fallback = ['这一段能再讲细一点吗？', '举个例子说明一下', '为什么这里要用这个方法？'];
        // 【逐行批注】 调用 fetch 执行页面交互或数据处理：fetch('/api/ask/suggestions', {method: 'POST', headers: {'Content-Typ…。
        fetch('/api/ask/suggestions', {method: 'POST', headers: {'Content-Type': 'application/json'},
          // 【逐行批注】 调用 topicName 执行页面交互或数据处理：body: JSON.stringify({question, answer, topic: topicName()})})。
          body: JSON.stringify({question, answer, topic: topicName()})})
          // 【逐行批注】 计算右侧结果并更新 .then(response，改变页面或请求状态。
          .then(response => response.ok ? response.json() : null)
          // 【逐行批注】 计算右侧结果并更新 .then(payload，改变页面或请求状态。
          .then(payload => {
            // 【逐行批注】 声明前端变量 list，保存本行计算出的数据或对象。
            const list = ((payload && payload.suggestions) || []).filter(item => typeof item === 'string' && item.trim()).slice(0, 3);
            // 【逐行批注】 结束当前函数并返回结果：return list.length ? list : fallback;。
            return list.length ? list : fallback;
          // 【逐行批注】 执行当前前端表达式：})。
          })
          // 【逐行批注】 计算右侧结果并更新 .catch(()，改变页面或请求状态。
          .catch(() => fallback)
          // 【逐行批注】 计算右侧结果并更新 .then(items，改变页面或请求状态。
          .then(items => {
            // 【逐行批注】 计算右侧结果并更新 host.innerHTML，改变页面或请求状态。
            host.innerHTML = '';
            // 【逐行批注】 计算右侧结果并更新 items.forEach(text，改变页面或请求状态。
            items.forEach(text => {
              // 【逐行批注】 声明前端变量 chip，保存本行计算出的数据或对象。
              const chip = document.createElement('button');
              // 【逐行批注】 计算右侧结果并更新 chip.type，改变页面或请求状态。
              chip.type = 'button'; chip.className = 'chat-chip'; chip.textContent = text;
              // 【逐行批注】 为页面事件绑定处理逻辑：chip.addEventListener('click', () => { input.value = text; input.focus(); form.reque…。
              chip.addEventListener('click', () => { input.value = text; input.focus(); form.requestSubmit(); });
              // 【逐行批注】 执行当前前端表达式：host.appendChild(chip);。
              host.appendChild(chip);
            // 【逐行批注】 执行当前前端表达式：});。
            });
          // 【逐行批注】 执行当前前端表达式：});。
          });
      // 【逐行批注】 执行当前前端表达式：});。
      });
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 调用 api、encodeURIComponent 执行页面交互或数据处理：api('/api/conversations/' + encodeURIComponent(conversation))。
    api('/api/conversations/' + encodeURIComponent(conversation))
      // 【逐行批注】 计算右侧结果并更新 .then(data，改变页面或请求状态。
      .then(data => (data.messages || []).forEach(item => addHistory(item.text, item.role === 'user'))).catch(() => {});
    // 【逐行批注】 声明前端变量 selected，保存本行计算出的数据或对象。
    let selected = '', socket = null;
    // 【逐行批注】 声明前端变量 stop，保存本行计算出的数据或对象。
    const stop = document.createElement('button'); stop.type = 'button'; stop.id = 'learning-chat-stop'; stop.textContent = '停止回答'; stop.hidden = true;
    // 【逐行批注】 按循环规则遍历或重复处理：form.appendChild(stop);。
    form.appendChild(stop);
    // 【逐行批注】 计算右侧结果并更新 stop.onclick，改变页面或请求状态。
    stop.onclick = () => {
      // 【逐行批注】 判断条件 (!socket) return;，成立时执行对应逻辑。
      if (!socket) return;
      // 【逐行批注】 声明前端变量 connection，保存本行计算出的数据或对象。
      const connection = socket;
      // 【逐行批注】 计算右侧结果并更新 stop.disabled，改变页面或请求状态。
      stop.disabled = true;
      // 【逐行批注】 声明前端变量 request，保存本行计算出的数据或对象。
      const request = () => { try { connection.send(JSON.stringify({type: 'stop'})); } catch (_) {} };
      // 【逐行批注】 判断条件 (connection.readyState === WebSocket.OPEN) request();，成立时执行对应逻辑。
      if (connection.readyState === WebSocket.OPEN) request();
      // 【逐行批注】 为页面事件绑定处理逻辑：else if (connection.readyState === WebSocket.CONNECTING) connection.addEventListener…。
      else if (connection.readyState === WebSocket.CONNECTING) connection.addEventListener('open', request, {once: true});
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('mouseup', () => {。
    document.addEventListener('mouseup', () => {
      // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
      const text = window.getSelection()?.toString().trim() || '';
      // 【逐行批注】 判断条件 (!text || text.length > 2000 || !window.getSelection()?.anchorNo…，成立时执行对应逻辑。
      if (!text || text.length > 2000 || !window.getSelection()?.anchorNode?.parentElement?.closest('#learn-content')) return;
      // 【逐行批注】 计算右侧结果并更新 selected，改变页面或请求状态。
      selected = text;
      // 【逐行批注】 声明前端变量 ask，保存本行计算出的数据或对象。
      let ask = $('ask-selection');
      // 【逐行批注】 判断条件 (!ask) { ask = document.createElement('button'); ask.id = 'ask-s…，成立时执行对应逻辑。
      if (!ask) { ask = document.createElement('button'); ask.id = 'ask-selection'; ask.type = 'button'; ask.textContent = '选中文字提问'; document.body.appendChild(ask); }
      // 【逐行批注】 声明前端变量 rect，保存本行计算出的数据或对象。
      const rect = window.getSelection().getRangeAt(0).getBoundingClientRect();
      // 【逐行批注】 计算右侧结果并更新 ask.style.left，改变页面或请求状态。
      ask.style.left = Math.min(window.innerWidth - 160, Math.max(8, rect.left)) + 'px';
      // 【逐行批注】 计算右侧结果并更新 ask.style.top，改变页面或请求状态。
      ask.style.top = Math.max(8, rect.bottom + 8) + 'px';
      // 【逐行批注】 计算右侧结果并更新 ask.hidden，改变页面或请求状态。
      ask.hidden = false;
      // 【逐行批注】 计算右侧结果并更新 ask.onclick，改变页面或请求状态。
      ask.onclick = () => { $('learning-chat-panel').classList.remove('hidden'); input.value = '这段内容是什么意思？'; input.focus(); ask.hidden = true; };
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 【逐行批注】 按循环规则遍历或重复处理：form.onsubmit = event => {。
    form.onsubmit = event => {
      // 【逐行批注】 执行当前前端表达式：event.preventDefault();。
      event.preventDefault();
      // 【逐行批注】 声明前端变量 question，保存本行计算出的数据或对象。
      const question = input.value.trim(); if (!question || socket) return;
      // 【逐行批注】 声明前端变量 sentSelection，保存本行计算出的数据或对象。
      const sentSelection = selected;
      // 【逐行批注】 调用 addHistory 执行页面交互或数据处理：addHistory(question, true);。
      addHistory(question, true);
      // 【逐行批注】 计算右侧结果并更新 input.value，改变页面或请求状态。
      input.value = '';
      // 【逐行批注】 执行当前前端表达式：try { localStorage.removeItem(draftKey); } catch (_) {}。
      try { localStorage.removeItem(draftKey); } catch (_) {}
      // 【逐行批注】 声明前端变量 send，保存本行计算出的数据或对象。
      const send = $('learning-chat-send'); send.disabled = true; stop.hidden = false; stop.disabled = false;
      // 【逐行批注】 声明前端变量 view，保存本行计算出的数据或对象。
      const view = chat.createAssistantView(box, {scrollRoot: box});
      // 【逐行批注】 声明前端变量 finished，保存本行计算出的数据或对象。
      let finished = false;
      // 【逐行批注】 声明前端变量 protocol，保存本行计算出的数据或对象。
      const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
      // 【逐行批注】 计算右侧结果并更新 socket，改变页面或请求状态。
      socket = new WebSocket(protocol + '//' + location.host + '/ws/classroom');
      // 【逐行批注】 计算右侧结果并更新 socket.onopen，改变页面或请求状态。
      socket.onopen = () => socket.send(JSON.stringify({...context, type: 'ask', question, selection: sentSelection}));
      // 【逐行批注】 计算右侧结果并更新 socket.onmessage，改变页面或请求状态。
      socket.onmessage = event => {
        // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
        const data = JSON.parse(event.data);
        // 【逐行批注】 判断条件 (data.type === 'delta') view.receive(data);，成立时执行对应逻辑。
        if (data.type === 'delta') view.receive(data);
        // 【逐行批注】 判断条件 (data.type === 'done') { finished = true; finish(view, question,…，成立时执行对应逻辑。
        if (data.type === 'stream_status') view.receive(data);
        if (data.type === 'done') { finished = true; finish(view, question, data); socket.close(); }
        // 【逐行批注】 判断条件 (data.type === 'error') { finished = true; view.fail(data.messag…，成立时执行对应逻辑。
        if (data.type === 'error') { finished = true; view.fail(data.message); socket.close(); }
      // 【逐行批注】 执行当前前端表达式：};。
      };
      // 【逐行批注】 计算右侧结果并更新 socket.onerror，改变页面或请求状态。
      socket.onerror = () => { finished = true; view.fail('连接中断，请重试。'); };
      // 【逐行批注】 计算右侧结果并更新 socket.onclose，改变页面或请求状态。
      socket.onclose = () => {
        // 【逐行批注】 判断条件 (!finished) view.fail('回答中断，本次未完成。');，成立时执行对应逻辑。
        if (!finished) view.fail('回答中断，本次未完成。');
        // 【逐行批注】 计算右侧结果并更新 socket，改变页面或请求状态。
        socket = null; send.disabled = false; stop.hidden = true; stop.disabled = false; selected = ''; input.focus();
      // 【逐行批注】 执行当前前端表达式：};。
      };
    // 【逐行批注】 执行当前前端表达式：};。
    };
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 setupIdlePrompt，封装可复用的前端行为。
  function setupIdlePrompt() {
    // 【逐行批注】 声明前端变量 prompt，保存本行计算出的数据或对象。
    const prompt = document.createElement('aside');
    // 【逐行批注】 计算右侧结果并更新 prompt.id，改变页面或请求状态。
    prompt.id = 'classroom-idle'; prompt.hidden = true;
    // 【逐行批注】 计算右侧结果并更新 prompt.innerHTML，改变页面或请求状态。
    prompt.innerHTML = '<span>停了一会儿？可以试着用自己的话说出本节关键概念。</span><button type="button" aria-label="关闭提示">×</button>';
    // 【逐行批注】 执行当前前端表达式：document.body.appendChild(prompt);。
    document.body.appendChild(prompt);
    // 【逐行批注】 声明前端变量 lastActive，保存本行计算出的数据或对象。
    let lastActive = Date.now(), lastPrompt = 0, count = 0;
    // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
    const active = () => { lastActive = Date.now(); prompt.hidden = true; };
    // 【逐行批注】 为页面事件绑定处理逻辑：['pointerdown', 'keydown', 'scroll'].forEach(name => document.addEventListener(name,…。
    ['pointerdown', 'keydown', 'scroll'].forEach(name => document.addEventListener(name, active, {passive: true}));
    // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('focusout', active);。
    document.addEventListener('focusout', active);
    // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('focus', active);。
    window.addEventListener('focus', active);
    // 【逐行批注】 计算右侧结果并更新 prompt.querySelector('button').onclick，改变页面或请求状态。
    prompt.querySelector('button').onclick = active;
    // 【逐行批注】 计算右侧结果并更新 setInterval(()，改变页面或请求状态。
    setInterval(() => {
      // 【逐行批注】 声明前端变量 typing，保存本行计算出的数据或对象。
      const typing = ['INPUT', 'TEXTAREA'].includes(document.activeElement?.tagName);
      // 【逐行批注】 声明前端变量 now，保存本行计算出的数据或对象。
      const now = Date.now();
      // 【逐行批注】 判断条件 (!prompt.hidden && now - lastPrompt >= 15000) { prompt.hidden = …，成立时执行对应逻辑。
      if (!prompt.hidden && now - lastPrompt >= 15000) { prompt.hidden = true; lastActive = now; }
      // 【逐行批注】 判断条件 (count >= 2 || !prompt.hidden || !document.hasFocus() || typing)…，成立时执行对应逻辑。
      if (count >= 2 || !prompt.hidden || !document.hasFocus() || typing) return;
      // 【逐行批注】 判断条件 (now - lastActive >= 120000 && now - lastPrompt >= 120000) {，成立时执行对应逻辑。
      if (now - lastActive >= 120000 && now - lastPrompt >= 120000) {
        // 【逐行批注】 计算右侧结果并更新 prompt.hidden，改变页面或请求状态。
        prompt.hidden = false; lastPrompt = now; count++;
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 执行当前前端表达式：}, 5000);。
    }, 5000);
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 调用 setupWeeklyPlan、setupReviews、setupLearn 执行页面交互或数据处理：setupWeeklyPlan(); setupReviews(); setupLearn();。
  setupWeeklyPlan(); setupReviews(); setupLearn();
// 【逐行批注】 执行当前前端表达式：})();。
})();

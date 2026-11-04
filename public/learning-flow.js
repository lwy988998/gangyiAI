// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 声明前端变量 params，保存本行计算出的数据或对象。
  const params = new URLSearchParams(location.search);
  // 【逐行批注】 声明前端变量 context，保存本行计算出的数据或对象。
  const context = {courseId: params.get('courseId') || '', phaseIndex: Number(params.get('phaseIndex') || 1), topicIndex: Number(params.get('topicIndex') || 1),
    // 【逐行批注】 执行当前前端表达式：...(params.get('lessonTaskId') ? {lessonTaskId: params.get('lessonTaskId')} : {}),。
    ...(params.get('lessonTaskId') ? {lessonTaskId: params.get('lessonTaskId')} : {}),
    // 【逐行批注】 调用 Number 执行页面交互或数据处理：...(params.has('review') ? {day: Number(params.get('review')), review…。
    ...(params.has('review') ? {day: Number(params.get('review')), reviewId: params.get('reviewId') || ''} : {})};
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('gangyi:lesson-context', event => Object.assign(context, e…。
  document.addEventListener('gangyi:lesson-context', event => Object.assign(context, event.detail));
  // 【逐行批注】 声明前端变量 clientId，保存本行计算出的数据或对象。
  const clientId = crypto.randomUUID ? crypto.randomUUID() : Date.now() + '-' + Math.random().toString(36).slice(2);
  // 【逐行批注】 声明前端变量 byId，保存本行计算出的数据或对象。
  const byId = id => document.getElementById(id);
  // 【逐行批注】 声明前端变量 element，保存本行计算出的数据或对象。
  const element = (tag, text, className = '') => { const node = document.createElement(tag); node.textContent = text; node.className = className; return node; };
  // 【逐行批注】 定义函数 api，封装可复用的前端行为。
  async function api(path, data, method = 'POST') {
    // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
    const response = await fetch(path, data === undefined ? undefined : {method, headers: {'Content-Type': 'application/json'}, body: JSON.stringify(data)});
    // 【逐行批注】 声明前端变量 result，保存本行计算出的数据或对象。
    const result = await response.json();
    // 【逐行批注】 判断条件 (!response.ok) throw new Error(result.error || '请求未完成，请重试');，成立时执行对应逻辑。
    if (!response.ok) throw new Error(result.error || '请求未完成，请重试');
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 localTime，保存本行计算出的数据或对象。
  const localTime = text => text ? new Date(text).toLocaleString('zh-CN') : '';
  // 【逐行批注】 声明前端变量 drafts，保存本行计算出的数据或对象。
  const drafts = new Map();
  // 【逐行批注】 声明前端变量 activeSubmissions，保存本行计算出的数据或对象。
  const activeSubmissions = new Set();
  // 【逐行批注】 声明前端变量 requestIdentity，保存本行计算出的数据或对象。
  const requestIdentity = () => crypto.randomUUID ? crypto.randomUUID() : clientId + '-' + Date.now() + '-' + Math.random().toString(36).slice(2);
  // 【逐行批注】 定义函数 readLocal，封装可复用的前端行为。
  function readLocal(key, fallback = '') { try { return localStorage.getItem(key) || fallback; } catch (_) { return fallback; } }
  // 【逐行批注】 定义函数 saveLocal，封装可复用的前端行为。
  function saveLocal(key, value) { try { if (value) localStorage.setItem(key, value); else localStorage.removeItem(key); } catch (_) { /* 无法使用本地缓存时，输入仍保留在页面。 */ } }
  // 【逐行批注】 定义函数 formatPublic，封装可复用的前端行为。
  function formatPublic(node, text) {
    // 【逐行批注】 执行当前前端表达式：node.classList.add('chat-body');。
    node.classList.add('chat-body');
    // 【逐行批注】 判断条件 (window.GangyiChat) node.innerHTML = window.GangyiChat.renderMar…，成立时执行对应逻辑。
    if (window.GangyiChat) node.innerHTML = window.GangyiChat.renderMarkdown(String(text || ''), {math: true});
    // 【逐行批注】 执行前一条件不成立时的分支。
    else node.textContent = String(text || '');
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 renderDialogue，封装可复用的前端行为。
  function renderDialogue(kind, item, index, base) {
    // 【逐行批注】 计算右侧结果并更新 base，改变页面或请求状态。
    base = {...base, ...(item.lessonTaskId ? {lessonTaskId: item.lessonTaskId} : {})};
    // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
    const row = element('article', '', 'classroom-question ai-dialogue'); row.dataset.index = index; row.dataset.kind = kind;
    // 【逐行批注】 声明前端变量 title，保存本行计算出的数据或对象。
    const title = element('h3', ''); formatPublic(title, item.question || item.task || item.content || item.title || '题目'); row.append(title);
    // 【逐行批注】 判断条件 (item.title && item.title !== item.question) row.prepend(element…，成立时执行对应逻辑。
    if (item.title && item.title !== item.question) row.prepend(element('p', item.title, 'question-title'));
    // 【逐行批注】 声明前端变量 materials，保存本行计算出的数据或对象。
    const materials = Array.isArray(item.materials) ? item.materials.join('\n\n') : item.materials;
    // 【逐行批注】 判断条件 (materials) { const material = element('div', '', 'question-mate…，成立时执行对应逻辑。
    if (materials) { const material = element('div', '', 'question-materials'); formatPublic(material, typeof materials === 'string' ? materials : JSON.stringify(materials)); row.append(material); }
    // 【逐行批注】 声明前端变量 history，保存本行计算出的数据或对象。
    const history = element('div', '', 'ai-dialogue-history'); history.setAttribute('aria-label', '本题对话记录');
    // 【逐行批注】 声明前端变量 choiceBox，保存本行计算出的数据或对象。
    const choiceBox = element('div', '', 'classroom-options');
    // 【逐行批注】 计算右侧结果并更新 (item.options || []).forEach((text, choice)，改变页面或请求状态。
    (item.options || []).forEach((text, choice) => {
      // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
      const label = element('label', ''); const radio = document.createElement('input'); radio.type = 'radio';
      // 【逐行批注】 计算右侧结果并更新 radio.name，改变页面或请求状态。
      radio.name = 'q-' + kind + '-' + index + '-' + (base.lessonTaskId || 'original') + '-' + (item.questionId || 'question'); radio.value = choice;
      // 【逐行批注】 判断条件 (item.studentAnswer === choice) radio.checked = true;，成立时执行对应逻辑。
      if (item.studentAnswer === choice) radio.checked = true;
      // 【逐行批注】 调用 element 执行页面交互或数据处理：label.append(radio, element('span', text)); choiceBox.append(label);。
      label.append(radio, element('span', text)); choiceBox.append(label);
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 【逐行批注】 声明前端变量 input，保存本行计算出的数据或对象。
    const input = document.createElement('textarea'); input.rows = 3; input.setAttribute('aria-label', '你的回答或问题');
    // 【逐行批注】 计算右侧结果并更新 input.placeholder，改变页面或请求状态。
    input.placeholder = '输入答案和解题过程，提交后仍可追问或说明希望怎样讲解';
    // 【逐行批注】 声明前端变量 actions，保存本行计算出的数据或对象。
    const actions = element('div', '', 'classroom-actions');
    // 【逐行批注】 声明前端变量 send，保存本行计算出的数据或对象。
    const send = element('button', '发送给 AI'), unknown = element('button', '暂时不会'), explain = element('button', '请求讲解'), stop = element('button', '停止回答'), skip = element('button', '跳过本题');
    // 【逐行批注】 计算右侧结果并更新 send.dataset.submit，改变页面或请求状态。
    send.dataset.submit = '1';
    // 【逐行批注】 计算右侧结果并更新 [send, unknown, explain, skip, stop].forEach…，改变页面或请求状态。
    [send, unknown, explain, skip, stop].forEach(button => { button.type = 'button'; }); stop.hidden = true;
    // 【逐行批注】 执行当前前端表达式：actions.append(send, unknown, explain, skip, stop);。
    actions.append(send, unknown, explain, skip, stop);
    // 【逐行批注】 声明前端变量 feedback，保存本行计算出的数据或对象。
    const feedback = element('p', '', 'classroom-feedback'); feedback.setAttribute('role', 'status');
    // 【逐行批注】 声明前端变量 retry，保存本行计算出的数据或对象。
    const retry = element('button', '重试本次回答'); retry.type = 'button'; retry.hidden = true;
    // 【逐行批注】 执行当前前端表达式：row.append(choiceBox, history, input, actions, feedback, retry);。
    row.append(choiceBox, history, input, actions, feedback, retry);
    // 【逐行批注】 声明前端变量 version，保存本行计算出的数据或对象。
    let version = item.dialogueVersion || 0, questionId = item.questionId || '', socket = null, lastRequest = '', lastText = '', lastIntent = 'answer', lastGiven = null, lastSelected = null, polling = null, turnCount = 0, stopRequested = false, choiceChanged = false;
    // 【逐行批注】 为页面事件绑定处理逻辑：choiceBox.addEventListener('change', () => { choiceChanged = true; });。
    choiceBox.addEventListener('change', () => { choiceChanged = true; });
    // 【逐行批注】 声明前端变量 contentVersion，保存本行计算出的数据或对象。
    const contentVersion = item.contentVersion || window.gangyiContentVersion || 1;
    // 【逐行批注】 声明前端变量 draftKey，保存本行计算出的数据或对象。
    const draftKey = 'gy:question-draft:' + JSON.stringify([base.courseId, base.phaseIndex, base.topicIndex, base.lessonTaskId || '', base.reviewId || '', base.day ?? '', kind, questionId || index, contentVersion]);
    // 【逐行批注】 计算右侧结果并更新 input.value，改变页面或请求状态。
    input.value = readLocal(draftKey);
    // 【逐行批注】 声明前端变量 saveDraft，保存本行计算出的数据或对象。
    const saveDraft = () => saveLocal(draftKey, input.value);
    // 【逐行批注】 为页面事件绑定处理逻辑：input.addEventListener('input', saveDraft);。
    input.addEventListener('input', saveDraft);
    // 【逐行批注】 声明前端变量 draftRecord，保存本行计算出的数据或对象。
    const draftRecord = {row, input, saveDraft, questionId, kind, index, contentVersion}; drafts.set(draftKey, draftRecord);
    // 【逐行批注】 声明前端变量 identity，保存本行计算出的数据或对象。
    const identity = () => ({...base, kind, index, questionId, contentVersion, version});
    // 【逐行批注】 声明前端变量 query，保存本行计算出的数据或对象。
    const query = () => new URLSearchParams(identity());
    // 【逐行批注】 定义函数 message，封装可复用的前端行为。
    function message(text, mine, failed = false) {
      // 【逐行批注】 声明前端变量 bubble，保存本行计算出的数据或对象。
      const bubble = element('div', '', 'ai-dialogue-bubble ' + (mine ? 'is-user' : 'is-teacher') + (failed ? ' is-failed' : ''));
      // 【逐行批注】 判断条件 (mine || failed) bubble.textContent = text; else formatPublic(bu…，成立时执行对应逻辑。
      if (mine || failed) bubble.textContent = text; else formatPublic(bubble, text);
      // 【逐行批注】 执行当前前端表达式：history.append(bubble); return bubble;。
      history.append(bubble); return bubble;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 refresh，封装可复用的前端行为。
    async function refresh(restore = false) {
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
        const data = await api('/api/classroom/dialogue?' + query()); version = data.version ?? data.dialogueVersion ?? version; questionId = data.questionId || questionId;
        // 【逐行批注】 计算右侧结果并更新 row.dataset.questionId，改变页面或请求状态。
        row.dataset.questionId = questionId; draftRecord.questionId = questionId; turnCount = (data.turns || []).length;
        // 【逐行批注】 判断条件 (restore || !socket) {，成立时执行对应逻辑。
        if (restore || !socket) {
          // 【逐行批注】 声明前端变量 focused，保存本行计算出的数据或对象。
          const focused = document.activeElement === input;
          // 【逐行批注】 判断条件 (!focused || restore) {，成立时执行对应逻辑。
          if (!focused || restore) {
            // 【逐行批注】 执行当前前端表达式：history.replaceChildren();。
            history.replaceChildren();
            // 【逐行批注】 计算右侧结果并更新 (data.turns || []).forEach(turn，改变页面或请求状态。
            (data.turns || []).forEach(turn => {
              // 【逐行批注】 调用 message 执行页面交互或数据处理：message(turn.user, true);。
              message(turn.user, true);
              // 【逐行批注】 判断条件 (turn.assistant) message(turn.assistant, false);，成立时执行对应逻辑。
              if (turn.assistant) message(turn.assistant, false);
              // 【逐行批注】 执行前一条件不成立时的分支。
              else {
                // 【逐行批注】 声明前端变量 parts，保存本行计算出的数据或对象。
                const parts = [...(turn.previousPartialReplies || []).map(reply => reply.text).filter(Boolean), ...(turn.partialAssistant ? [turn.partialAssistant] : [])];
                // 【逐行批注】 计算右侧结果并更新 parts.forEach(text，改变页面或请求状态。
                parts.forEach(text => { message(text, false); message('本次讲解未完成，已展示内容保留；尚未形成新的可靠评价。', false, true); });
              // 【逐行批注】 结束当前前端代码块。
              }
              // 【逐行批注】 判断条件 (turn.error) message(turn.error, false, true);，成立时执行对应逻辑。
              if (turn.error) message(turn.error, false, true);
            // 【逐行批注】 执行当前前端表达式：});。
            });
            // 【逐行批注】 声明前端变量 latest，保存本行计算出的数据或对象。
            const latest = data.turns?.at(-1);
            // 【逐行批注】 判断条件 (latest && ['failed', 'cancelled', 'interrupted'].includes(lates…，成立时执行对应逻辑。
            if (latest && ['failed', 'cancelled', 'interrupted'].includes(latest.status)) { if (!input.value) input.value = latest.user; lastRequest = latest.requestId; lastText = latest.user; lastIntent = latest.intent || 'answer'; lastGiven = latest.givenAnswer ?? latest.user; lastSelected = latest.selectedOption ?? null; saveDraft(); }
            // 【逐行批注】 判断条件 (!(data.turns || []).length && item.status === 'answered') {，成立时执行对应逻辑。
            if (!(data.turns || []).length && item.status === 'answered') {
              // 【逐行批注】 判断条件 (item.studentAnswer !== undefined) message(typeof item.studentAn…，成立时执行对应逻辑。
              if (item.studentAnswer !== undefined) message(typeof item.studentAnswer === 'string' ? item.studentAnswer : item.options?.[item.studentAnswer] || '已作答', true);
              // 【逐行批注】 判断条件 (item.feedback) message(item.feedback, false);，成立时执行对应逻辑。
              if (item.feedback) message(item.feedback, false);
            // 【逐行批注】 结束当前前端代码块。
            }
          // 【逐行批注】 结束当前前端代码块。
          }
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 计算右侧结果并更新 retry.hidden，改变页面或请求状态。
        retry.hidden = data.evaluationStatus !== 'waiting' && !['failed', 'cancelled', 'interrupted'].includes(data.turns?.at(-1)?.status);
        // 【逐行批注】 计算右侧结果并更新 feedback.textContent，改变页面或请求状态。
        feedback.textContent = data.evaluationStatus === 'waiting' ? '等待 AI 评价，之前的可靠结果保留。' :
          // 【逐行批注】 执行当前前端表达式：data.evaluationStatus === 'pending' ? ((data.turns || []).length ? '回答已保存，真实 AI 正在评价…' : …。
          data.evaluationStatus === 'pending' ? ((data.turns || []).length ? '回答已保存，真实 AI 正在评价…' : '可以开始回答，也可以直接向 AI 提问。') : (data.feedback || (turnCount ? '已保存本题对话，可以继续交流。' : '未作答不表示不会，可先作答、提问或跳过。'));
        // 【逐行批注】 判断条件 (data.mode) document.dispatchEvent(new CustomEvent('gangyi:diagn…，成立时执行对应逻辑。
        if (data.mode) document.dispatchEvent(new CustomEvent('gangyi:diagnostic-mode', {detail: data.mode}));
      // 【逐行批注】 计算右侧结果并更新 } catch (error) { feedback.textContent，改变页面或请求状态。
      } catch (error) { feedback.textContent = error.message; }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 busy，封装可复用的前端行为。
    function busy(value) { send.disabled = value; unknown.disabled = value; explain.disabled = value; skip.disabled = value; retry.disabled = value; stop.disabled = false; stop.hidden = !value; row.setAttribute('aria-busy', String(value)); }
    // 【逐行批注】 定义函数 submit，封装可复用的前端行为。
    async function submit(text, intent = turnCount ? 'ask' : 'answer', retrying = false) {
      // 【逐行批注】 判断条件 (socket) return;，成立时执行对应逻辑。
      if (socket) return;
      // 【逐行批注】 判断条件 (!questionId) await refresh(); if (!questionId) return;，成立时执行对应逻辑。
      if (!questionId) await refresh(); if (!questionId) return;
      // 【逐行批注】 声明前端变量 selected，保存本行计算出的数据或对象。
      const selected = choiceBox.querySelector('input:checked');
      // 【逐行批注】 声明前端变量 typed，保存本行计算出的数据或对象。
      const typed = input.value.trim();
      // 【逐行批注】 声明前端变量 useChoice，保存本行计算出的数据或对象。
      const useChoice = selected && (choiceChanged || intent === 'answer' || !typed && !text);
      // 帮助按钮同时发送学生当轮说明；失败重试继续使用原请求，不重复拼接。
      // 【逐行批注】 声明前端变量 prompted，保存本行计算出的数据或对象。
      const prompted = text && !retrying && typed ? text + '\n本轮说明或讲解偏好：' + typed : text;
      // 【逐行批注】 声明前端变量 raw，保存本行计算出的数据或对象。
      const raw = prompted || (useChoice && typed ? '所选答案：' + item.options[Number(selected.value)] + '\n解题过程或问题：' + typed : typed || (selected ? item.options[Number(selected.value)] : ''));
      // 【逐行批注】 声明前端变量 choiceAnswer，保存本行计算出的数据或对象。
      const choiceAnswer = !text && !typed && selected ? Number(selected.value) : null;
      // 【逐行批注】 判断条件 (!raw) { feedback.textContent = '请先输入回答或选择一个答案。'; input.focus();…，成立时执行对应逻辑。
      if (!raw) { feedback.textContent = '请先输入回答或选择一个答案。'; input.focus(); return; }
      // 【逐行批注】 声明前端变量 requestId，保存本行计算出的数据或对象。
      const requestId = retrying && lastRequest ? lastRequest : requestIdentity();
      // 【逐行批注】 声明前端变量 givenAnswer，保存本行计算出的数据或对象。
      const givenAnswer = retrying && lastGiven !== null ? lastGiven : choiceAnswer === null ? raw : choiceAnswer;
      // 【逐行批注】 声明前端变量 selectedOption，保存本行计算出的数据或对象。
      const selectedOption = retrying ? lastSelected : useChoice ? Number(selected.value) : null;
      // 【逐行批注】 计算右侧结果并更新 lastGiven，改变页面或请求状态。
      lastGiven = givenAnswer; lastSelected = selectedOption;
      // 【逐行批注】 计算右侧结果并更新 choiceChanged，改变页面或请求状态。
      choiceChanged = false;
      // 【逐行批注】 计算右侧结果并更新 lastText，改变页面或请求状态。
      lastText = raw; lastRequest = requestId; lastIntent = intent; input.value = ''; saveDraft();
      // 【逐行批注】 计算右侧结果并更新 message(raw, true); const view，改变页面或请求状态。
      message(raw, true); const view = window.GangyiChat.createAssistantView(history, {scrollRoot: history}); let terminal = false;
      // 【逐行批注】 计算右侧结果并更新 busy(true); feedback.textContent，改变页面或请求状态。
      busy(true); feedback.textContent = '正在与真实 AI 对话…';
      // 【逐行批注】 计算右侧结果并更新 socket，改变页面或请求状态。
      socket = new WebSocket((location.protocol === 'https:' ? 'wss:' : 'ws:') + '//' + location.host + '/ws/classroom');
      // 【逐行批注】 声明前端变量 connection，保存本行计算出的数据或对象。
      const connection = socket;
      // 【逐行批注】 声明前端变量 resolveSubmission，保存本行计算出的数据或对象。
      let resolveSubmission;
      // 【逐行批注】 声明前端变量 submission，保存本行计算出的数据或对象。
      const submission = new Promise(resolve => { resolveSubmission = resolve; });
      // 【逐行批注】 执行当前前端表达式：activeSubmissions.add(submission);。
      activeSubmissions.add(submission);
      // 【逐行批注】 声明前端变量 settleSubmission，保存本行计算出的数据或对象。
      const settleSubmission = () => { activeSubmissions.delete(submission); resolveSubmission(); };
      // 【逐行批注】 计算右侧结果并更新 stopRequested，改变页面或请求状态。
      stopRequested = false;
      // 【逐行批注】 计算右侧结果并更新 connection.onopen，改变页面或请求状态。
      connection.onopen = () => { connection.send(JSON.stringify({...identity(), type: 'ask', question: raw,
        // 【逐行批注】 执行当前前端表达式：answer: givenAnswer, selectedOption, intent, requestId}));。
        answer: givenAnswer, selectedOption, intent, requestId}));
        // 【逐行批注】 判断条件 (stopRequested) connection.send(JSON.stringify({type: 'stop', re…，成立时执行对应逻辑。
        if (stopRequested) connection.send(JSON.stringify({type: 'stop', requestId})); };
      // 【逐行批注】 计算右侧结果并更新 connection.onmessage，改变页面或请求状态。
      connection.onmessage = event => {
        // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
        let data; try { data = JSON.parse(event.data); } catch (_) { return; }
        // 【逐行批注】 判断条件 (data.type === 'delta') view.push(data.text || '');，成立时执行对应逻辑。
        if (data.type === 'delta') view.push(data.text || '');
        // 【逐行批注】 判断条件 (data.type === 'done') {，成立时执行对应逻辑。
        if (data.type === 'done') {
          // 【逐行批注】 调用 settleSubmission 执行页面交互或数据处理：settleSubmission();。
          settleSubmission();
          // 【逐行批注】 计算右侧结果并更新 terminal，改变页面或请求状态。
          terminal = true; if (data.version !== undefined) version = data.version;
          // 【逐行批注】 执行当前前端表达式：view.complete();。
          view.complete();
          // 【逐行批注】 判断条件 (data.cancelled) { message('已停止，本次未计入可靠评价。', false, true); if (!…，成立时执行对应逻辑。
          if (data.cancelled) { message('已停止，本次未计入可靠评价。', false, true); if (!input.value) input.value = raw; saveDraft(); retry.hidden = false; }
          // 【逐行批注】 执行前一条件不成立时的分支。
          else { lastRequest = ''; lastText = ''; turnCount++; feedback.textContent = '完整评价和讲解已保存，可继续追问。'; }
          // 【逐行批注】 执行当前前端表达式：connection.close();。
          connection.close();
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 判断条件 (data.type === 'error') { settleSubmission(); terminal = true; v…，成立时执行对应逻辑。
        if (data.type === 'error') { settleSubmission(); terminal = true; view.fail(data.message || '回答失败，原有结果保留。'); if (!input.value) input.value = raw; saveDraft(); retry.hidden = false; connection.close(); }
      // 【逐行批注】 执行当前前端表达式：};。
      };
      // 【逐行批注】 计算右侧结果并更新 connection.onerror，改变页面或请求状态。
      connection.onerror = () => { if (terminal) return; terminal = true; settleSubmission(); view.fail('连接中断，输入已保留，可以重试。'); if (!input.value) input.value = raw; saveDraft(); retry.hidden = false; };
      // 【逐行批注】 计算右侧结果并更新 connection.onclose，改变页面或请求状态。
      connection.onclose = () => { settleSubmission(); if (!terminal) { view.fail('回答中断，本次未形成可靠评价。'); if (!input.value) input.value = raw; saveDraft(); retry.hidden = false; } socket = null; busy(false); refresh(); input.focus(); };
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 send.onclick，改变页面或请求状态。
    send.onclick = () => submit(); unknown.onclick = () => submit('我暂时不会这道题，请根据我的情况帮助我', 'unknown'); explain.onclick = () => submit('请讲解这道题；如果我的回答缺少过程，请明确说明判断依据不足', 'ask');
    // 【逐行批注】 为页面事件绑定处理逻辑：input.addEventListener('keydown', event => { if (event.ctrlKey && event.key === 'Ent…。
    input.addEventListener('keydown', event => { if (event.ctrlKey && event.key === 'Enter') submit(); });
    // 【逐行批注】 计算右侧结果并更新 stop.onclick，改变页面或请求状态。
    stop.onclick = () => { stopRequested = true; stop.disabled = true; if (socket?.readyState === WebSocket.OPEN) socket.send(JSON.stringify({type: 'stop', requestId: lastRequest})); };
    // 【逐行批注】 计算右侧结果并更新 retry.onclick，改变页面或请求状态。
    retry.onclick = async () => { if (lastText) { await refresh(); submit(lastText, lastIntent, true); } else { retry.disabled = true; try { await api('/api/classroom/evaluation/retry', {...identity(), requestId: lastRequest || requestIdentity()}); await refresh(); } catch (error) { feedback.textContent = error.message; } finally { retry.disabled = false; } } };
    // 【逐行批注】 计算右侧结果并更新 skip.onclick，改变页面或请求状态。
    skip.onclick = async () => { saveDraft(); try { await api('/api/classroom/question/skip', {...identity(), requestId: requestIdentity()}); feedback.textContent = '已跳过，可继续阅读或提问；跳过只表示缺少信息。'; } catch (error) { feedback.textContent = error.message; } };
    // 【逐行批注】 计算右侧结果并更新 refresh(true); polling，改变页面或请求状态。
    refresh(true); polling = setInterval(() => { if (!document.hidden && row.isConnected && !socket) refresh(); if (!row.isConnected) clearInterval(polling); }, 4000);
    // 【逐行批注】 结束当前函数并返回结果：return row;。
    return row;
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 mountStudyPlan，封装可复用的前端行为。
  function mountStudyPlan(before) {
    // 【逐行批注】 判断条件 (byId('weekly-plan')) return;，成立时执行对应逻辑。
    if (byId('weekly-plan')) return;
    // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
    const host = typeof before === 'string' ? document.querySelector(before) : before; if (!host) return;
    // 【逐行批注】 声明前端变量 box，保存本行计算出的数据或对象。
    const box = element('section', '', 'classroom-card weekly-plan'); box.id = 'weekly-plan';
    // 【逐行批注】 声明前端变量 title，保存本行计算出的数据或对象。
    const title = element('h2', 'AI 统一学习安排'); title.id = 'weekly-title';
    // 【逐行批注】 声明前端变量 message，保存本行计算出的数据或对象。
    const message = element('p', '所有课程共用每天的学习时间。'); message.id = 'weekly-message'; message.setAttribute('role', 'status');
    // 【逐行批注】 声明前端变量 availability，保存本行计算出的数据或对象。
    const availability = element('div', ''), entries = element('div', ''); availability.id = 'weekly-availability'; entries.id = 'weekly-entries';
    // 【逐行批注】 声明前端变量 actions，保存本行计算出的数据或对象。
    const actions = element('div', '', 'classroom-actions'), save = element('button', '保存修改'), replan = element('button', '根据最新表现重排');
    // 【逐行批注】 计算右侧结果并更新 save.id，改变页面或请求状态。
    save.id = 'weekly-save'; replan.id = 'weekly-replan'; save.type = replan.type = 'button'; actions.append(save, replan);
    // 【逐行批注】 声明前端变量 confirm，保存本行计算出的数据或对象。
    const confirm = element('div', '', 'ai-plan-preview'); confirm.id = 'weekly-confirm'; confirm.hidden = true;
    // 【逐行批注】 声明前端变量 timeDetails，保存本行计算出的数据或对象。
    const timeDetails = element('details', ''); timeDetails.append(element('summary', '调整每天可用时间'), availability); box.append(title, message, timeDetails, entries, actions, confirm); host.before(box);
    // 【逐行批注】 声明前端变量 plan，保存本行计算出的数据或对象。
    let plan = null, dirty = false, busy = false, editEpoch = 0;
    // 【逐行批注】 声明前端变量 names，保存本行计算出的数据或对象。
    const names = ['周一', '周二', '周三', '周四', '周五', '周六', '周日'];
    // 【逐行批注】 声明前端变量 entryTitle，保存本行计算出的数据或对象。
    const entryTitle = item => item.title || item.topic || item.courseTitle ||
      // 【逐行批注】 执行当前前端表达式：({'agent-lesson':'已备课课时', lesson:'课程知识点', review:'复习任务'}[item.kind] || '学习任务');。
      ({'agent-lesson':'已备课课时', lesson:'课程知识点', review:'复习任务'}[item.kind] || '学习任务');
    // 【逐行批注】 定义函数 publicReason，封装可复用的前端行为。
    function publicReason(value) {
      // 【逐行批注】 声明前端变量 reason，保存本行计算出的数据或对象。
      let reason = value.proposal.reason || '查看后确认是否应用。';
      // 【逐行批注】 声明前端变量 known，保存本行计算出的数据或对象。
      const known = [...(value.proposal.previousPlan?.entries || []), ...(value.proposal.plan.entries || [])];
      // 【逐行批注】 按循环规则遍历或重复处理：for (const item of known) {。
      for (const item of known) {
        // 【逐行批注】 按循环规则遍历或重复处理：for (const id of [item.taskId, item.lessonId, item.reviewId]) if (id) reas…。
        for (const id of [item.taskId, item.lessonId, item.reviewId]) if (id) reason = reason.split(id).join(entryTitle(item));
        // 【逐行批注】 判断条件 (item.courseId) reason = reason.split(item.courseId).join(item.c…，成立时执行对应逻辑。
        if (item.courseId) reason = reason.split(item.courseId).join(item.courseTitle || '该课程');
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 结束当前函数并返回结果：return reason;。
      return reason;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 gather，封装可复用的前端行为。
    function gather() {
      // 【逐行批注】 结束当前函数并返回结果：return {availability: [...availability.querySelectorAll('label')].fil…。
      return {availability: [...availability.querySelectorAll('label')].filter(label => label.querySelector('[type=checkbox]').checked)
        // 【逐行批注】 计算右侧结果并更新 .map(label，改变页面或请求状态。
        .map(label => ({weekday: Number(label.dataset.weekday), minutes: Number(label.querySelector('[type=number]').value)})),
        // 【逐行批注】 计算右侧结果并更新 entries: [...entries.querySelectorAll('.week…，改变页面或请求状态。
        entries: [...entries.querySelectorAll('.weekly-row')].map(row => ({...plan.entries[Number(row.dataset.index)],
          // 【逐行批注】 计算右侧结果并更新 date: row.querySelector('[type，改变页面或请求状态。
          date: row.querySelector('[type=date]').value, minutes: Number(row.querySelector('[type=number]').value)}))};
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 announceDraft，封装可复用的前端行为。
    function announceDraft(active = dirty) {
      // 【逐行批注】 判断条件 (!plan) return Promise.resolve();，成立时执行对应逻辑。
      if (!plan) return Promise.resolve();
      // 【逐行批注】 结束当前函数并返回结果：return api('/api/study-plan/draft', {clientId, version: plan.version,…。
      return api('/api/study-plan/draft', {clientId, version: plan.version, active, ...(active ? gather() : {})}).catch(() => {});
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 为页面事件绑定处理逻辑：box.addEventListener('input', () => { dirty = true; announceDraft(); });。
    box.addEventListener('input', () => { dirty = true; announceDraft(); });
    // 【逐行批注】 定义函数 setBusy，封装可复用的前端行为。
    function setBusy(value) { if (busy !== value) ++editEpoch; busy = value; save.disabled = value; replan.disabled = value || plan?.status === 'pending'; replan.textContent = replan.disabled ? '正在生成候选…' : '重新排课'; }
    // 【逐行批注】 定义函数 showPreview，封装可复用的前端行为。
    function showPreview(value) {
      // 【逐行批注】 计算右侧结果并更新 confirm.replaceChildren(); confirm.hidden，改变页面或请求状态。
      confirm.replaceChildren(); confirm.hidden = !value.proposal;
      // 【逐行批注】 判断条件 (!value.proposal) return;，成立时执行对应逻辑。
      if (!value.proposal) return;
      // 【逐行批注】 调用 element、publicReason 执行页面交互或数据处理：confirm.append(element('h3', 'AI 新旧安排对比'), element('p', publicReason(…。
      confirm.append(element('h3', 'AI 新旧安排对比'), element('p', publicReason(value)));
      // 【逐行批注】 声明前端变量 comparison，保存本行计算出的数据或对象。
      const comparison = element('div', '', 'ai-plan-comparison');
      // 【逐行批注】 按循环规则遍历或重复处理：for (const [label, list] of [['当前安排', dirty ? gather().entries : value.pro…。
      for (const [label, list] of [['当前安排', dirty ? gather().entries : value.proposal.previousPlan?.entries || plan.entries || []], ['AI 新安排', value.proposal.plan.entries || []]]) {
        // 【逐行批注】 声明前端变量 column，保存本行计算出的数据或对象。
        const column = element('div', ''); column.append(element('h4', label));
        // 【逐行批注】 声明前端变量 budget，保存本行计算出的数据或对象。
        const budget = label === 'AI 新安排' ? value.proposal.plan.availability : dirty ? gather().availability : value.proposal.previousPlan?.availability || plan.availability;
        // 【逐行批注】 计算右侧结果并更新 column.append(element('p', '每天总预算：' + budget…，改变页面或请求状态。
        column.append(element('p', '每天总预算：' + budget.map(slot => names[slot.weekday - 1] + ' ' + slot.minutes + ' 分钟').join('、')));
        // 【逐行批注】 计算右侧结果并更新 list.forEach(item，改变页面或请求状态。
        list.forEach(item => column.append(element('p', item.date + ' · ' + entryTitle(item) + ' · ' + item.minutes + ' 分钟'))); comparison.append(column);
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 声明前端变量 accept，保存本行计算出的数据或对象。
      const accept = element('button', '确认应用'), cancel = element('button', '取消，保留原安排'); accept.type = cancel.type = 'button';
      // 【逐行批注】 计算右侧结果并更新 accept.id，改变页面或请求状态。
      accept.id = 'weekly-accept'; cancel.id = 'weekly-cancel';
      // 【逐行批注】 计算右侧结果并更新 accept.onclick，改变页面或请求状态。
      accept.onclick = () => decide('confirm', value.proposal.id); cancel.onclick = () => decide('cancel', value.proposal.id);
      // 【逐行批注】 执行当前前端表达式：confirm.append(comparison, accept, cancel);。
      confirm.append(comparison, accept, cancel);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 render，封装可复用的前端行为。
    function render(value) {
      // 【逐行批注】 计算右侧结果并更新 plan，改变页面或请求状态。
      plan = value; title.textContent = 'AI 统一学习安排 · ' + value.weekStart + ' 至 ' + (value.weekEnd || value.weekStart);
      // 【逐行批注】 计算右侧结果并更新 message.textContent，改变页面或请求状态。
      message.textContent = value.message || '正在等待真实 AI 安排…'; availability.replaceChildren(); entries.replaceChildren();
      // 【逐行批注】 声明前端变量 grid，保存本行计算出的数据或对象。
      const grid = element('div', '', 'availability-grid');
      // 【逐行批注】 计算右侧结果并更新 names.forEach((name, index)，改变页面或请求状态。
      names.forEach((name, index) => {
        // 【逐行批注】 声明前端变量 slot，保存本行计算出的数据或对象。
        const slot = value.availability.find(item => item.weekday === index + 1), label = element('label', ''); label.dataset.weekday = index + 1;
        // 【逐行批注】 声明前端变量 check，保存本行计算出的数据或对象。
        const check = document.createElement('input'); check.type = 'checkbox'; check.checked = !!slot;
        // 【逐行批注】 计算右侧结果并更新 check.dataset.weekday，改变页面或请求状态。
        check.dataset.weekday = index + 1;
        // 【逐行批注】 声明前端变量 minutes，保存本行计算出的数据或对象。
        const minutes = document.createElement('input'); minutes.type = 'number'; minutes.min = 1; minutes.max = 1440; minutes.step = 1;
        // 【逐行批注】 计算右侧结果并更新 minutes.value，改变页面或请求状态。
        minutes.value = slot?.minutes || 30; minutes.setAttribute('aria-label', name + '总分钟数'); label.append(check, element('span', name), minutes, element('span', '分钟')); grid.append(label);
      // 【逐行批注】 调用 element 执行页面交互或数据处理：}); availability.append(element('h3', '每天可用总时间'), grid);。
      }); availability.append(element('h3', '每天可用总时间'), grid);
      // 【逐行批注】 调用 element 执行页面交互或数据处理：entries.append(element('h3', '全部课程安排'));。
      entries.append(element('h3', '全部课程安排'));
      // 【逐行批注】 计算右侧结果并更新 (value.entries || []).forEach((item, index)，改变页面或请求状态。
      (value.entries || []).forEach((item, index) => {
        // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
        const row = element('div', '', 'weekly-row'); row.dataset.index = index;
        // 【逐行批注】 声明前端变量 date，保存本行计算出的数据或对象。
        const date = document.createElement('input'); date.type = 'date'; date.value = item.date; date.setAttribute('aria-label', '学习日期');
        // 【逐行批注】 声明前端变量 minutes，保存本行计算出的数据或对象。
        const minutes = document.createElement('input'); minutes.type = 'number'; minutes.value = item.minutes; minutes.min = 1; minutes.max = 1440; minutes.setAttribute('aria-label', '学习分钟数');
        // 【逐行批注】 判断条件 (item.completed) date.disabled = minutes.disabled = true;，成立时执行对应逻辑。
        if (item.completed) date.disabled = minutes.disabled = true;
        // 【逐行批注】 声明前端变量 link，保存本行计算出的数据或对象。
        const link = element('a', entryTitle(item) + (item.completed ? ' · 已完成' : ''));
        // 【逐行批注】 计算右侧结果并更新 link.href，改变页面或请求状态。
        link.href = '/learn?' + new URLSearchParams(item.kind === 'agent-lesson' && item.lessonId ? {lessonId:item.lessonId} :
          // 【逐行批注】 执行当前前端表达式：{courseId:item.courseId, phaseIndex:item.phaseIndex, topicIndex:item.topicIndex, ...(item…。
          {courseId:item.courseId, phaseIndex:item.phaseIndex, topicIndex:item.topicIndex, ...(item.kind === 'review' ? {reviewId:item.reviewId, review:item.day} : {})});
        // 【逐行批注】 调用 element 执行页面交互或数据处理：row.append(date, link, minutes, element('span', '分钟'));。
        row.append(date, link, minutes, element('span', '分钟'));
        // 【逐行批注】 按循环规则遍历或重复处理：for (const [symbol, offset] of [['↑', -1], ['↓', 1]]) {。
        for (const [symbol, offset] of [['↑', -1], ['↓', 1]]) {
          // 【逐行批注】 声明前端变量 move，保存本行计算出的数据或对象。
          const move = element('button', symbol); move.type = 'button'; move.setAttribute('aria-label', offset < 0 ? '上移' : '下移');
          // 【逐行批注】 执行当前前端表达式：move.dataset[offset < 0 ? 'up' : 'down'] = '1';。
          move.dataset[offset < 0 ? 'up' : 'down'] = '1';
          // 【逐行批注】 计算右侧结果并更新 move.onclick，改变页面或请求状态。
          move.onclick = () => { const list = gather(); const to = index + offset; if (to < 0 || to >= list.entries.length) return; [list.entries[index], list.entries[to]] = [list.entries[to], list.entries[index]]; render({...plan, ...list}); dirty = true; announceDraft(); }; row.append(move);
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 执行当前前端表达式：entries.append(row);。
        entries.append(row);
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 判断条件 (!(value.entries || []).length) entries.append(element('p', '尚未安…，成立时执行对应逻辑。
      if (!(value.entries || []).length) entries.append(element('p', '尚未安排课程。设置可用时间后，点击“重新排课”生成候选。'));
      // 【逐行批注】 调用 showPreview 执行页面交互或数据处理：showPreview(value);。
      showPreview(value);
      // 【逐行批注】 调用 setBusy 执行页面交互或数据处理：setBusy(busy);。
      setBusy(busy);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 load，封装可复用的前端行为。
    async function load() { if (busy) return; const epoch = editEpoch; try { const value = await api('/api/study-plan'); if (busy || epoch !== editEpoch) return; if (!dirty) render(value); else { message.textContent = value.message; showPreview(value); } } catch (error) { if (epoch === editEpoch) message.textContent = error.message; } }
    // 【逐行批注】 定义函数 decide，封装可复用的前端行为。
    async function decide(action, proposalId) {
      // 【逐行批注】 计算右侧结果并更新 setBusy(true); try { const data，改变页面或请求状态。
      setBusy(true); try { const data = await api('/api/study-plan/' + action, {version: plan.version, proposalId}); dirty = false; await announceDraft(false); render(data.plan); } catch (error) { message.textContent = error.message; } finally { setBusy(false); }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 save.onclick，改变页面或请求状态。
    save.onclick = async () => { setBusy(true); try { const saved = await api('/api/study-plan', {version: plan.version, ...gather()}, 'PUT'); dirty = false; await announceDraft(false); render(saved); } catch (error) { message.textContent = error.message; } finally { setBusy(false); } };
    // 【逐行批注】 计算右侧结果并更新 replan.onclick，改变页面或请求状态。
    replan.onclick = async () => { confirm.hidden = true; confirm.replaceChildren(); setBusy(true); try { const data = await api('/api/study-plan/replan', {version: plan.version, availability: gather().availability, ...(dirty ? {entries: gather().entries, preview: true} : {})}); plan.status = data.plan.status; message.textContent = '真实 AI 正在统筹全部课程，请稍候…'; } catch (error) { message.textContent = error.message; } finally { setBusy(false); } };
    // 【逐行批注】 计算右侧结果并更新 load(); setInterval(()，改变页面或请求状态。
    load(); setInterval(() => { if (!document.hidden) load(); }, 4000); setInterval(() => { if (dirty) announceDraft(); }, 30000);
    // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('pagehide', () => { if (dirty) navigator.sendBeacon('/api/st…。
    window.addEventListener('pagehide', () => { if (dirty) navigator.sendBeacon('/api/study-plan/draft', new Blob([JSON.stringify({clientId, active: false})], {type: 'application/json'})); });
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 mountPreview，封装可复用的前端行为。
  async function mountPreview() {
    // 【逐行批注】 判断条件 (!context.courseId || !byId('plan-view')) return;，成立时执行对应逻辑。
    if (!context.courseId || !byId('plan-view')) return;
    // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
    let host = byId('course-preview');
    // 【逐行批注】 判断条件 (!host) { const card = element('section', '', 'plan-card full-ca…，成立时执行对应逻辑。
    if (!host) { const card = element('section', '', 'plan-card full-card'); card.append(element('h2', 'AI 课程路线预览')); host = element('div', ''); host.id = 'course-preview'; card.append(host); byId('plan-view').prepend(card); }
    // 【逐行批注】 判断条件 (host.dataset.mounted) return; host.dataset.mounted = '1';，成立时执行对应逻辑。
    if (host.dataset.mounted) return; host.dataset.mounted = '1';
    // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
    let active = 0, previewTaskId = readLocal('gy:preview-task:' + context.courseId);
    // 【逐行批注】 声明前端变量 tabs，保存本行计算出的数据或对象。
    const tabs = element('div', '', 'slide-tabs'), content = element('article', '', 'slide-content'), status = element('p', '', 'ai-muted');
    // 【逐行批注】 声明前端变量 update，保存本行计算出的数据或对象。
    const update = element('button', '更新 AI 预览'); update.type = 'button'; host.replaceChildren(status, tabs, content, update);
    // 【逐行批注】 定义函数 display，封装可复用的前端行为。
    function display(slides) {
      // 【逐行批注】 计算右侧结果并更新 active，改变页面或请求状态。
      active = Math.min(active, slides.length - 1); tabs.replaceChildren(); content.replaceChildren();
      // 【逐行批注】 计算右侧结果并更新 slides.forEach((slide, index)，改变页面或请求状态。
      slides.forEach((slide, index) => { const button = element('button', String(index + 1).padStart(2, '0') + ' · ' + slide.title, 'slide-tab' + (index === active ? ' selected' : '')); button.type = 'button'; button.onclick = () => { active = index; display(slides); }; tabs.append(button); });
      // 【逐行批注】 声明前端变量 slide，保存本行计算出的数据或对象。
      const slide = slides[active]; if (!slide) return;
      // 【逐行批注】 调用 element 执行页面交互或数据处理：content.append(element('h3', slide.title), element('p', slide.content…。
      content.append(element('h3', slide.title), element('p', slide.content));
      // 【逐行批注】 判断条件 (slide.bullets?.length) { const list = element('ul', '', 'ai-pre…，成立时执行对应逻辑。
      if (slide.bullets?.length) { const list = element('ul', '', 'ai-preview-bullets'); slide.bullets.forEach(text => list.append(element('li', text))); content.append(list); }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 load，封装可复用的前端行为。
    async function load() {
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
        const value = await api('/api/courses/' + encodeURIComponent(context.courseId) + '/preview');
        // 【逐行批注】 计算右侧结果并更新 display(value.slides || []); status.textCont…，改变页面或请求状态。
        display(value.slides || []); status.textContent = (value.message || '正在等待真实 AI 生成课程路线预览…') + (value.updatedAt ? ' · ' + localTime(value.updatedAt) : '');
        // 【逐行批注】 计算右侧结果并更新 update.disabled，改变页面或请求状态。
        update.disabled = false;
        // 【逐行批注】 判断条件 (previewTaskId) {，成立时执行对应逻辑。
        if (previewTaskId) {
          // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
          const task = await api('/api/learning-agent?taskId=' + encodeURIComponent(previewTaskId));
          // 【逐行批注】 计算右侧结果并更新 update.disabled，改变页面或请求状态。
          update.disabled = ['pending', 'running'].includes(task.status);
          // 【逐行批注】 判断条件 (update.disabled || ['failed', 'paused', 'superseded', 'waiting_…，成立时执行对应逻辑。
          if (update.disabled || ['failed', 'paused', 'superseded', 'waiting_student'].includes(task.status)) status.textContent = task.error || task.message || 'AI 正在准备路线说明。';
        // 【逐行批注】 执行当前前端表达式：} else if (value.status === 'pending') status.textContent = '原预览尚未完成，可以让 AI 按最新情况重新准备。';。
        } else if (value.status === 'pending') status.textContent = '原预览尚未完成，可以让 AI 按最新情况重新准备。';

      // 【逐行批注】 计算右侧结果并更新 } catch (error) { status.textContent，改变页面或请求状态。
      } catch (error) { status.textContent = error.message; }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 update.onclick，改变页面或请求状态。
    update.onclick = async () => { update.disabled = true; try { const task = await api('/api/courses/' + encodeURIComponent(context.courseId) + '/preview', {retry: true, requestId: requestIdentity()}); previewTaskId = task.id; saveLocal('gy:preview-task:' + context.courseId, previewTaskId); await load(); } catch (error) { status.textContent = error.message; update.disabled = false; } };
    // 【逐行批注】 计算右侧结果并更新 load(); setInterval(()，改变页面或请求状态。
    load(); setInterval(() => { if (!document.hidden) load(); }, 4000);
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 mountNextStep，封装可复用的前端行为。
  function mountNextStep() {
    // 【逐行批注】 声明前端变量 home，保存本行计算出的数据或对象。
    const home = document.querySelector('.home-hero');
    // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
    const host = home; if (!host || byId('ai-next-step')) return;
    // 【逐行批注】 声明前端变量 box，保存本行计算出的数据或对象。
    const box = element('section', '', 'classroom-card ai-next-step'); box.id = 'ai-next-step';
    // 【逐行批注】 声明前端变量 detail，保存本行计算出的数据或对象。
    const detail = element('p', '正在读取真实 AI 的下一步建议…'), link = element('a', '查看学习安排 →');
    // 【逐行批注】 声明前端变量 fullDetail，保存本行计算出的数据或对象。
    const fullDetail=element('details','','course-more'), fullText=element('p','','ai-next-full');
    // 【逐行批注】 计算右侧结果并更新 fullText.style.whiteSpace，改变页面或请求状态。
    fullText.style.whiteSpace='pre-wrap';fullDetail.append(element('summary','展开 AI 完整说明'),fullText);fullDetail.hidden=true;
    // 【逐行批注】 计算右侧结果并更新 link.href，改变页面或请求状态。
    link.href = '/my-courses#weekly-plan'; link.className = 'ai-next-action'; box.append(detail, fullDetail, link); home.querySelector('.home-intro').after(box);
    // 【逐行批注】 定义函数 load，封装可复用的前端行为。
    async function load() {
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
        const data = await api('/api/home/next-step');
        // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
        const text=data.lesson ? data.lesson.title + '：' + (data.message || 'AI 已完成备课') : data.message || '等待 AI 更新';
        // 【逐行批注】 声明前端变量 sentence，保存本行计算出的数据或对象。
        const sentence=(text.match(/^[\s\S]*?[。！？\n]/)?.[0]||text).trim();
        // 【逐行批注】 计算右侧结果并更新 detail.textContent，改变页面或请求状态。
        detail.textContent=sentence;fullText.textContent=text;fullDetail.hidden=sentence===text.trim();
        // 【逐行批注】 计算右侧结果并更新 link.href，改变页面或请求状态。
        link.href = data.lesson ? '/agent-prepare.html?taskId='+encodeURIComponent(data.id) : '/my-courses#weekly-plan'; link.hidden = !data.lesson; link.textContent = data.lesson ? '查看 AI 已备好的学习内容 →' : '查看学习安排 →';
      // 【逐行批注】 计算右侧结果并更新 } catch (_) { detail.textContent，改变页面或请求状态。
      } catch (_) { detail.textContent = '等待 AI 更新，已保存的课程仍可继续阅读。'; }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 load(); setInterval(()，改变页面或请求状态。
    load(); setInterval(() => { if (!document.hidden) load(); }, 4000);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 mountPreparation，封装可复用的前端行为。
  function mountPreparation() {
    // 【逐行批注】 判断条件 (!byId('learn-content') || !context.courseId || byId('ai-prepara…，成立时执行对应逻辑。
    if (!byId('learn-content') || !context.courseId || byId('ai-preparation-note')) return;
    // 【逐行批注】 声明前端变量 box，保存本行计算出的数据或对象。
    const box = element('section', '', 'classroom-card'); box.id = 'ai-preparation-note';
    // 【逐行批注】 声明前端变量 status，保存本行计算出的数据或对象。
    const status = element('p', 'AI 会根据你的真实对话和作答调整讲解与练习。'), supplement = element('p', '', 'ai-supplement');
    // 【逐行批注】 声明前端变量 retry，保存本行计算出的数据或对象。
    const retry = element('button', '重试 AI 备课'); retry.type = 'button'; retry.hidden = true;
    // 【逐行批注】 调用 element、byId 执行页面交互或数据处理：box.append(element('h2', 'AI 正在跟着你备课'), status, supplement, retry); b…。
    box.append(element('h2', 'AI 正在跟着你备课'), status, supplement, retry); byId('learn-content').prepend(box);
    // 【逐行批注】 声明前端变量 exposed，保存本行计算出的数据或对象。
    const exposed = new Set(), supplements = new Set(), selectors = {overview: '#learn-key-concepts', steps: '#learn-steps', examples: '#learn-examples', practice: '#learn-practice', quiz: '#learn-quiz', assessment: '#learn-checkpoint'};
    // 【逐行批注】 声明前端变量 observer，保存本行计算出的数据或对象。
    const observer = new IntersectionObserver(records => {
      // 【逐行批注】 声明前端变量 names，保存本行计算出的数据或对象。
      const names = [];
      // 【逐行批注】 计算右侧结果并更新 records.forEach(record，改变页面或请求状态。
      records.forEach(record => { if (record.isIntersecting && !exposed.has(record.target.dataset.aiBlock) && !byId('learn-content').classList.contains('hidden')) { exposed.add(record.target.dataset.aiBlock); names.push(record.target.dataset.aiBlock); } });
      // 【逐行批注】 判断条件 (names.length) api('/api/learn/exposure', {...context, blocks: n…，成立时执行对应逻辑。
      if (names.length) api('/api/learn/exposure', {...context, blocks: names, contentVersion: window.gangyiContentVersion || 1}).catch(() => {});
    // 【逐行批注】 执行当前前端表达式：}, {threshold: 0.05});。
    }, {threshold: 0.05});
    // 【逐行批注】 计算右侧结果并更新 Object.entries(selectors).forEach(([block, s…，改变页面或请求状态。
    Object.entries(selectors).forEach(([block, selector]) => { const node = document.querySelector(selector); if (node) { node.dataset.aiBlock = block; observer.observe(node); } });
    // 【逐行批注】 定义函数 load，封装可复用的前端行为。
    async function load() {
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 判断条件 (context.lessonTaskId) { status.textContent = '本课堂使用已经完整保存的课程；下一…，成立时执行对应逻辑。
        if (context.lessonTaskId) { status.textContent = '本课堂使用已经完整保存的课程；下一课会结合最新作答重新分析。'; return; }
        // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
        const value = await api('/api/classroom/preparation?courseId=' + encodeURIComponent(context.courseId));
        // 【逐行批注】 计算右侧结果并更新 status.textContent，改变页面或请求状态。
        status.textContent = value.message; retry.hidden = value.status !== 'waiting';
        // 【逐行批注】 声明前端变量 instruction，保存本行计算出的数据或对象。
        const instruction = (value.teaching || []).find(item => item.phaseIndex === context.phaseIndex && item.topicIndex === context.topicIndex);
        // 【逐行批注】 判断条件 (instruction?.supplement && !supplements.has(instruction.supplem…，成立时执行对应逻辑。
        if (instruction?.supplement && !supplements.has(instruction.supplement)) { supplements.add(instruction.supplement); const addition = element('div', ''); formatPublic(addition, instruction.supplement); supplement.append(addition); }
      // 【逐行批注】 执行当前前端表达式：} catch (_) { /* 已展示的课堂和输入保持可用。 * / }。
      } catch (_) { /* 已展示的课堂和输入保持可用。 */ }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 retry.onclick，改变页面或请求状态。
    retry.onclick = async () => { retry.disabled = true; try { await api('/api/classroom/preparation/retry', context); await load(); } finally { retry.disabled = false; } };
    // 【逐行批注】 调用 load、setInterval 执行页面交互或数据处理：load(); if (!context.lessonTaskId) setInterval(() => { if (!document.…。
    load(); if (!context.lessonTaskId) setInterval(() => { if (!document.hidden) load(); }, 4000);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 collectDrafts，封装可复用的前端行为。
  function collectDrafts() {
    // 【逐行批注】 声明前端变量 list，保存本行计算出的数据或对象。
    const list = [];
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [key, draft] of drafts) {。
    for (const [key, draft] of drafts) {
      // 【逐行批注】 判断条件 (!draft.row.isConnected) { drafts.delete(key); continue; }，成立时执行对应逻辑。
      if (!draft.row.isConnected) { drafts.delete(key); continue; }
      // 【逐行批注】 执行当前前端表达式：draft.saveDraft();。
      draft.saveDraft();
      // 【逐行批注】 判断条件 (draft.input.value.trim()) list.push({questionId: draft.question…，成立时执行对应逻辑。
      if (draft.input.value.trim()) list.push({questionId: draft.questionId, kind: draft.kind, index: draft.index, text: draft.input.value, contentVersion: draft.contentVersion});
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 声明前端变量 input，保存本行计算出的数据或对象。
    const input = byId('learning-chat-input');
    // 【逐行批注】 判断条件 (input?.value.trim()) { saveLocal('gy:lesson-chat-draft:' + JSON…，成立时执行对应逻辑。
    if (input?.value.trim()) { saveLocal('gy:lesson-chat-draft:' + JSON.stringify(context), input.value); list.push({kind: 'lesson', text: input.value}); }
    // 【逐行批注】 结束当前函数并返回结果：return list;。
    return list;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 mountQuestionKind，封装可复用的前端行为。
  async function mountQuestionKind(kind, target) {
    // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
    const host = typeof target === 'string' ? document.querySelector(target) : target; if (!host || host.dataset.questionsLoading) return;
    // 【逐行批注】 声明前端变量 contentVersion，保存本行计算出的数据或对象。
    const contentVersion = window.gangyiContentVersion || 1;
    // 【逐行批注】 判断条件 (host.dataset.questionsVersion === String(contentVersion)) retur…，成立时执行对应逻辑。
    if (host.dataset.questionsVersion === String(contentVersion)) return;
    // 【逐行批注】 计算右侧结果并更新 host.dataset.questionsLoading，改变页面或请求状态。
    host.dataset.questionsLoading = '1';
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
      const data = await api('/api/classroom/questions?' + new URLSearchParams({...context, kind, contentVersion}));
      // 【逐行批注】 声明前端变量 questions，保存本行计算出的数据或对象。
      const questions = (data.questions || []).filter(item => item.kind === kind);
      // 内容已展示后保留原题和对话；追加题目不会覆盖学生正在编辑的输入。
      // 【逐行批注】 判断条件 (!host.querySelector('.ai-dialogue')) host.replaceChildren();，成立时执行对应逻辑。
      if (!host.querySelector('.ai-dialogue')) host.replaceChildren();
      // 【逐行批注】 按循环规则遍历或重复处理：for (const item of questions) if (![...host.querySelectorAll('.ai-dialogue…。
      for (const item of questions) if (![...host.querySelectorAll('.ai-dialogue')].some(row => row.dataset.questionId === item.questionId)) {
        // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
        const row = renderDialogue(kind, item, item.index, context); row.dataset.questionId = item.questionId; host.append(row);
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (!questions.length && !host.children.length) host.append(element…，成立时执行对应逻辑。
      if (!questions.length && !host.children.length) host.append(element('p', '本节没有这一类题目，可继续阅读或请 AI 准备下一课。', 'ai-muted'));
      // 【逐行批注】 计算右侧结果并更新 host.dataset.questionsVersion，改变页面或请求状态。
      host.dataset.questionsVersion = String(contentVersion);
    // 【逐行批注】 执行当前前端表达式：} catch (error) {。
    } catch (error) {
      // 【逐行批注】 判断条件 (!host.querySelector('.ai-dialogue')) { host.replaceChildren(ele…，成立时执行对应逻辑。
      if (!host.querySelector('.ai-dialogue')) { host.replaceChildren(element('p', error.message)); const retry = element('button', '重试读取题目'); retry.type = 'button'; retry.onclick = () => { delete host.dataset.questionsVersion; mountQuestionKind(kind, host); }; host.append(retry); }
    // 【逐行批注】 执行当前前端表达式：} finally { delete host.dataset.questionsLoading; }。
    } finally { delete host.dataset.questionsLoading; }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 mountNextLessonAction，封装可复用的前端行为。
  function mountNextLessonAction() {
    // 【逐行批注】 声明前端变量 next，保存本行计算出的数据或对象。
    const next = byId('lesson-next'); if (!next || next.dataset.preparationMounted) return;
    // 【逐行批注】 计算右侧结果并更新 next.dataset.preparationMounted，改变页面或请求状态。
    next.dataset.preparationMounted = '1'; next.removeAttribute('href'); next.textContent = '让 AI 准备下一课';
    // 【逐行批注】 计算右侧结果并更新 next.onclick，改变页面或请求状态。
    next.onclick = async event => {
      // 【逐行批注】 执行当前前端表达式：event.preventDefault(); if (next.disabled) return;。
      event.preventDefault(); if (next.disabled) return;
      // 【逐行批注】 计算右侧结果并更新 next.disabled，改变页面或请求状态。
      next.disabled = true; next.textContent = 'AI 正在读取学习情况…';
      // 【逐行批注】 声明前端变量 status，保存本行计算出的数据或对象。
      const status = byId('next-lesson-action-status');
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
        const data = await api('/api/learn/next', {...context, requestId: requestIdentity(), drafts: collectDrafts(), contentVersion: window.gangyiContentVersion || 1});
        // 【逐行批注】 判断条件 (!data.id || !String(data.url || '').startsWith('/learn/next?'))…，成立时执行对应逻辑。
        if (!data.id || !String(data.url || '').startsWith('/learn/next?')) throw new Error('备课任务未成功创建，请重试。');
        // 先让当前页已提交的回答到终态，避免导航断开连接而取消在途评价。
        // 【逐行批注】 判断条件 (activeSubmissions.size && status) status.textContent = '已提交回答仍在…，成立时执行对应逻辑。
        if (activeSubmissions.size && status) status.textContent = '已提交回答仍在处理，完成后进入备课页。';
        // 【逐行批注】 在条件成立期间循环处理：while (activeSubmissions.size) await Promise.all([...activeSubmissions]);。
        while (activeSubmissions.size) await Promise.all([...activeSubmissions]);
        // 【逐行批注】 执行当前前端表达式：location.assign(data.url);。
        location.assign(data.url);
      // 【逐行批注】 计算右侧结果并更新 } catch (error) { if (status) status.textCon…，改变页面或请求状态。
      } catch (error) { if (status) status.textContent = error.message; next.disabled = false; next.textContent = '让 AI 准备下一课'; }
    // 【逐行批注】 执行当前前端表达式：};。
    };
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 showPublicBlock，封装可复用的前端行为。
  function showPublicBlock(host, stage, block) {
    // 【逐行批注】 判断条件 (!block || typeof block !== 'object') return;，成立时执行对应逻辑。
    if (!block || typeof block !== 'object') return;
    // 【逐行批注】 执行当前前端表达式：host.replaceChildren();。
    host.replaceChildren();
    // 【逐行批注】 声明前端变量 prose，保存本行计算出的数据或对象。
    const prose = value => { if (typeof value !== 'string' || !value.trim()) return; const node = element('div', '', 'preparation-prose'); formatPublic(node, value); host.append(node); };
    // 【逐行批注】 判断条件 (stage === 'overview') { prose(block.title); prose(block.summary…，成立时执行对应逻辑。
    if (stage === 'overview') { prose(block.title); prose(block.summary); (block.keyConcepts || []).forEach(prose); }
    // 【逐行批注】 判断条件 (stage === 'steps') (block.lessonSteps || []).forEach(item => { …，成立时执行对应逻辑。
    if (stage === 'steps') (block.lessonSteps || []).forEach(item => { prose(item.title); prose(item.explanation); prose(item.example); prose(item.action); });
    // 【逐行批注】 判断条件 (['examples', 'practice', 'quiz'].includes(stage)) {，成立时执行对应逻辑。
    if (['examples', 'practice', 'quiz'].includes(stage)) {
      // 【逐行批注】 声明前端变量 items，保存本行计算出的数据或对象。
      const items = stage === 'examples' ? block.examples : stage === 'practice' ? block.practice : block.quiz;
      // 【逐行批注】 计算右侧结果并更新 (items || []).forEach(item，改变页面或请求状态。
      (items || []).forEach(item => { const question = element('article', '', 'preparation-question'); const title = element('h3', item.title || item.question || item.task || '题目'); question.append(title);
        // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
        const text = item.question || item.task || item.content; if (text && text !== title.textContent) { const node = element('div', ''); formatPublic(node, text); question.append(node); }
        // 【逐行批注】 判断条件 (typeof item.materials === 'string') { const material = element(…，成立时执行对应逻辑。
        if (typeof item.materials === 'string') { const material = element('div', ''); formatPublic(material, item.materials); question.append(material); }
        // 【逐行批注】 判断条件 (Array.isArray(item.options)) { const options = element('ol', ''…，成立时执行对应逻辑。
        if (Array.isArray(item.options)) { const options = element('ol', '', 'preparation-options'); item.options.forEach(text => options.append(element('li', text))); question.append(options); }
        // 【逐行批注】 执行当前前端表达式：host.append(question);。
        host.append(question);
      // 【逐行批注】 执行当前前端表达式：});。
      });
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 判断条件 (stage === 'assessment') { (block.checkpoint || []).forEach(pros…，成立时执行对应逻辑。
    if (stage === 'assessment') { (block.checkpoint || []).forEach(prose); prose(block.summary); prose(block.resourceSummary); }
    // 【逐行批注】 判断条件 (stage === 'decision') prose(block.reason || block.text);，成立时执行对应逻辑。
    if (stage === 'decision') prose(block.reason || block.text);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 mountNextLessonPage，封装可复用的前端行为。
  function mountNextLessonPage() {
    // 【逐行批注】 声明前端变量 page，保存本行计算出的数据或对象。
    const page = byId('next-lesson-page'); if (!page || page.dataset.mounted) return; page.dataset.mounted = '1';
    // 【逐行批注】 声明前端变量 id，保存本行计算出的数据或对象。
    const id = params.get('id'), status = byId('next-lesson-status'), blocksHost = byId('next-lesson-blocks'), stop = byId('next-lesson-stop'), retry = byId('next-lesson-retry'), back = byId('next-lesson-return');
    // 【逐行批注】 声明前端变量 names，保存本行计算出的数据或对象。
    const names = {decision: '调整说明', overview: '课程总览', steps: '讲解', examples: '例题', practice: '练习', quiz: '测验', assessment: '总结'};
    // 【逐行批注】 声明前端变量 stages，保存本行计算出的数据或对象。
    const stages = new Map(); let socket = null, lastSeq = 0, terminal = false, reconnectTimer = null, reconnectCount = 0;
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [stage, name] of Object.entries(names)) { const section = eleme…。
    for (const [stage, name] of Object.entries(names)) { const section = element('section', '', 'classroom-card preparation-stage'); section.dataset.stage = stage; section.hidden = true;
      // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
      const text = element('div', '', 'preparation-stream'); const completed = element('small', '', 'ai-muted'); section.append(element('h2', name), text, completed); blocksHost.append(section); stages.set(stage, {section, text, completed, raw: ''}); }
    // 【逐行批注】 计算右侧结果并更新 back.textContent，改变页面或请求状态。
    back.textContent = '返回课堂'; back.href = '/my-courses';
    // 【逐行批注】 定义函数 finish，封装可复用的前端行为。
    function finish(type, message) { terminal = true; stop.hidden = true; retry.hidden = type === 'ready'; status.textContent = message; if (socket) socket.close(); }
    // 【逐行批注】 定义函数 ready，封装可复用的前端行为。
    function ready(url) { if (!url || !String(url).startsWith('/learn?')) { finish('failed', '备课保存结果缺少有效课堂地址，请重新准备。'); return; } finish('ready', '课程已完整校验并保存，正在进入课堂…'); location.assign(url); }
    // 【逐行批注】 定义函数 event，封装可复用的前端行为。
    function event(data) {
      // 【逐行批注】 判断条件 (!Number.isInteger(data.seq) || data.seq <= lastSeq) return;，成立时执行对应逻辑。
      if (!Number.isInteger(data.seq) || data.seq <= lastSeq) return;
      // 【逐行批注】 判断条件 (data.seq !== lastSeq + 1) { if (socket) socket.close(); restore…，成立时执行对应逻辑。
      if (data.seq !== lastSeq + 1) { if (socket) socket.close(); restore(); return; }
      // 【逐行批注】 计算右侧结果并更新 lastSeq，改变页面或请求状态。
      lastSeq = data.seq;
      // 【逐行批注】 判断条件 (data.reset) { terminal = false; stop.hidden = false; retry.hidd…，成立时执行对应逻辑。
      if (data.reset) { terminal = false; stop.hidden = false; retry.hidden = true; stages.forEach(part => { part.raw = ''; part.text.replaceChildren(); part.section.hidden = true; part.completed.textContent = ''; }); }
      // 【逐行批注】 声明前端变量 part，保存本行计算出的数据或对象。
      const part = stages.get(data.stage);
      // 【逐行批注】 判断条件 (part) part.section.hidden = false;，成立时执行对应逻辑。
      if (part) part.section.hidden = false;
      // 【逐行批注】 判断条件 (data.type === 'stage') { status.textContent = (names[data.stage…，成立时执行对应逻辑。
      if (data.type === 'stage') { status.textContent = (names[data.stage] || '备课') + '：真实 AI 正在生成…'; if (part) part.completed.textContent = '生成中'; }
      // 【逐行批注】 判断条件 (data.type === 'delta' && part) { part.raw += data.text || ''; f…，成立时执行对应逻辑。
      if (data.type === 'delta' && part) { part.raw += data.text || ''; formatPublic(part.text, part.raw); }
      // 【逐行批注】 判断条件 (data.type === 'block_complete' && part) { const block = typeof …，成立时执行对应逻辑。
      if (data.type === 'block_complete' && part) { const block = typeof data.block === 'object' ? data.block : data.data; if (block) showPublicBlock(part.text, data.stage, block); part.completed.textContent = '本板块已生成，等待整课校验和保存'; }
      // 【逐行批注】 判断条件 (data.type === 'ready') ready(data.classroomUrl);，成立时执行对应逻辑。
      if (data.type === 'ready') ready(data.classroomUrl);
      // 【逐行批注】 判断条件 (data.type === 'failed' || data.type === 'cancelled') finish(dat…，成立时执行对应逻辑。
      if (data.type === 'failed' || data.type === 'cancelled') finish(data.type, data.text || data.message || (data.type === 'failed' ? '备课未完成，原课堂已保留。可重试或返回。' : '备课已停止。可以重新准备或返回课堂。'));
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 subscribe，封装可复用的前端行为。
    function subscribe() {
      // 【逐行批注】 判断条件 (terminal || !id || socket) return;，成立时执行对应逻辑。
      if (terminal || !id || socket) return;
      // 【逐行批注】 声明前端变量 connection，保存本行计算出的数据或对象。
      const connection = new WebSocket((location.protocol === 'https:' ? 'wss:' : 'ws:') + '//' + location.host + '/ws/learn/next'); socket = connection;
      // 【逐行批注】 计算右侧结果并更新 connection.onopen，改变页面或请求状态。
      connection.onopen = () => { reconnectCount = 0; connection.send(JSON.stringify({type: 'subscribe', id, afterSeq: lastSeq})); };
      // 【逐行批注】 计算右侧结果并更新 connection.onmessage，改变页面或请求状态。
      connection.onmessage = payload => { try { const data = JSON.parse(payload.data); if (data.type === 'error') finish('failed', data.message || '无法订阅备课，请重新准备。'); else event(data); } catch (_) { /* 不完整事件不改变已展示内容。 */ } };
      // 【逐行批注】 计算右侧结果并更新 connection.onclose，改变页面或请求状态。
      connection.onclose = () => { if (socket === connection) socket = null; if (!terminal) { status.textContent = '连接中断，正在恢复已生成内容…'; clearTimeout(reconnectTimer); reconnectTimer = setTimeout(restore, Math.min(1000 * ++reconnectCount, 8000)); } };
      // 【逐行批注】 计算右侧结果并更新 connection.onerror，改变页面或请求状态。
      connection.onerror = () => { connection.close(); };
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 restore，封装可复用的前端行为。
    async function restore() {
      // 【逐行批注】 判断条件 (!id) { finish('failed', '缺少备课任务编号，请返回课堂重新准备。'); return; }，成立时执行对应逻辑。
      if (!id) { finish('failed', '缺少备课任务编号，请返回课堂重新准备。'); return; }
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
        const data = await api('/api/learn/next/' + encodeURIComponent(id));
        // 【逐行批注】 判断条件 (data.sourceUrl?.startsWith('/learn?')) back.href = data.sourceU…，成立时执行对应逻辑。
        if (data.sourceUrl?.startsWith('/learn?')) back.href = data.sourceUrl;
        // 【逐行批注】 计算右侧结果并更新 (data.events || []).sort((a, b)，改变页面或请求状态。
        (data.events || []).sort((a, b) => a.seq - b.seq).forEach(event);
        // 【逐行批注】 判断条件 (terminal) return;，成立时执行对应逻辑。
        if (terminal) return;
        // 服务端裁剪历史事件时，用完整公开快照恢复；不在刷新时重新创建任务。
        // 【逐行批注】 判断条件 (data.latestSeq > lastSeq && !(data.events || []).length) { for …，成立时执行对应逻辑。
        if (data.latestSeq > lastSeq && !(data.events || []).length) { for (const [stage, block] of Object.entries(data.blocks || {})) { const part = stages.get(stage); if (part) { part.section.hidden = false; showPublicBlock(part.text, stage, block); } } if (data.reason) { const part = stages.get('decision'); part.section.hidden = false; formatPublic(part.text, data.reason); } lastSeq = data.latestSeq; }
        // 【逐行批注】 判断条件 (data.status === 'ready') { ready(data.classroomUrl); return; }，成立时执行对应逻辑。
        if (data.status === 'ready') { ready(data.classroomUrl); return; }
        // 【逐行批注】 判断条件 (['failed', 'cancelled', 'stale'].includes(data.status)) { finis…，成立时执行对应逻辑。
        if (['failed', 'cancelled', 'stale'].includes(data.status)) { finish(data.status, data.message || '本次备课未保存，请重新准备或返回课堂。'); return; }
        // 【逐行批注】 计算右侧结果并更新 status.textContent，改变页面或请求状态。
        status.textContent = data.message || '真实 AI 正在结合最新学习情况准备课程…'; stop.hidden = false; retry.hidden = true; subscribe();
      // 【逐行批注】 计算右侧结果并更新 } catch (error) { status.textContent，改变页面或请求状态。
      } catch (error) { status.textContent = error.message + '，可以重连或返回课堂。'; retry.hidden = false; }
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 stop.onclick，改变页面或请求状态。
    stop.onclick = async () => { stop.disabled = true; try { await api('/api/learn/next/' + encodeURIComponent(id) + '/cancel', {}); await restore(); } catch (error) { status.textContent = error.message; } finally { stop.disabled = false; } };
    // 【逐行批注】 计算右侧结果并更新 retry.onclick，改变页面或请求状态。
    retry.onclick = async () => {
      // 【逐行批注】 计算右侧结果并更新 retry.disabled，改变页面或请求状态。
      retry.disabled = true;
      // 【逐行批注】 计算右侧结果并更新 try { const result，改变页面或请求状态。
      try { const result = await api('/api/learn/next/' + encodeURIComponent(id) + '/retry', {requestId: requestIdentity()});
        // 【逐行批注】 判断条件 (result.id && result.id !== id && result.url) { location.assign(…，成立时执行对应逻辑。
        if (result.id && result.id !== id && result.url) { location.assign(result.url); return; }
        // 【逐行批注】 计算右侧结果并更新 terminal，改变页面或请求状态。
        terminal = false; stages.forEach(part => { part.raw = ''; part.text.replaceChildren(); part.section.hidden = true; part.completed.textContent = ''; }); stop.hidden = false; retry.hidden = true; await restore();
      // 【逐行批注】 计算右侧结果并更新 } catch (error) { status.textContent，改变页面或请求状态。
      } catch (error) { status.textContent = error.message; } finally { retry.disabled = false; }
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('pagehide', () => { terminal = true; clearTimeout(reconnectT…。
    window.addEventListener('pagehide', () => { terminal = true; clearTimeout(reconnectTimer); if (socket) socket.close(); });
    // 【逐行批注】 调用 restore 执行页面交互或数据处理：restore();。
    restore();
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 计算右侧结果并更新 window.GangyiLearning，改变页面或请求状态。
  window.GangyiLearning = {renderDialogue, mountStudyPlan, mountQuestionKind, collectDrafts};
  // 【逐行批注】 调用 CustomEvent 执行页面交互或数据处理：document.dispatchEvent(new CustomEvent('gangyi:learning-ready'));。
  document.dispatchEvent(new CustomEvent('gangyi:learning-ready'));
  // 【逐行批注】 声明前端变量 initial，保存本行计算出的数据或对象。
  const initial = () => {
    // 【逐行批注】 判断条件 (document.querySelector('.home-hero')) mountNextStep(); if (byId…，成立时执行对应逻辑。
    if (document.querySelector('.home-hero')) mountNextStep(); if (byId('uc-study-plan-anchor')) mountStudyPlan('#uc-study-plan-anchor');
    // 【逐行批注】 调用 mountPreview、mountNextStep、mountPreparation、mountNextLessonAction 执行页面交互或数据处理：mountPreview(); mountNextStep(); mountPreparation(); mountNextLessonA…。
    mountPreview(); mountNextStep(); mountPreparation(); mountNextLessonAction(); mountNextLessonPage();
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 monitor，保存本行计算出的数据或对象。
  const monitor = new MutationObserver(initial); monitor.observe(document.body, {childList: true, subtree: true});
  // 【逐行批注】 调用 initial 执行页面交互或数据处理：initial();。
  initial();
  // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('pagehide', collectDrafts);。
  window.addEventListener('pagehide', collectDrafts);
// 【逐行批注】 执行当前前端表达式：})();。
})();

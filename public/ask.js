/* AI 导师页：流式回答、Markdown 排版、复制与追问建议。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 声明前端变量 $，保存本行计算出的数据或对象。
  const $ = id => document.getElementById(id);
  // 【逐行批注】 声明前端变量 messages，保存本行计算出的数据或对象。
  const messages = $('ask-messages'), input = $('ask-question'), submit = $('ask-submit');
  // 【逐行批注】 声明前端变量 chat，保存本行计算出的数据或对象。
  const chat = window.GangyiChat;
  const attached = $('ask-material')?.dataset;
  const conversationId = attached?.conversationId || 'general';
  const conversationUrl = '/api/conversations/' + encodeURIComponent(conversationId);
  const materialContext = attached?.examQuestion ? {
    courseId: attached.courseId, examQuestion: JSON.parse(attached.examQuestion)
  } : attached?.sourceId ? {
    courseId: attached.courseId, material: { sourceId: attached.sourceId, snapshotId: attached.snapshotId }
  } : {};
  // 【逐行批注】 声明前端变量 socket，保存本行计算出的数据或对象。
  let socket = null, ready = false, knownHistory = false, historyMessages = [], lastQuestion = '';
  // 【逐行批注】 声明前端变量 trace，保存本行计算出的数据或对象。
  const trace = window.GangyiTrace?.create($('ask-agent-trace'), {variant:'inline', eyebrow:'AI 导师执行过程', collapseWhenDone:true,
    // 【逐行批注】 计算右侧结果并更新 onCancel: ()，改变页面或请求状态。
    onCancel: () => requestStop(),
    // 【逐行批注】 计算右侧结果并更新 onRetry: ()，改变页面或请求状态。
    onRetry: () => retryLast(),
  // 【逐行批注】 执行当前前端表达式：});。
  });

  // 【逐行批注】 声明前端变量 toolbar，保存本行计算出的数据或对象。
  const toolbar = document.createElement('div');
  // 【逐行批注】 计算右侧结果并更新 toolbar.className，改变页面或请求状态。
  toolbar.className = 'chat-toolbar';
  // 【逐行批注】 计算右侧结果并更新 toolbar.innerHTML，改变页面或请求状态。
  toolbar.innerHTML = '<button type="button" id="ask-clear" class="chat-copy">清除本机对话</button>' +
    // 【逐行批注】 执行当前前端表达式：'<button type="button" id="ask-stop" class="chat-copy" hidden>停止回答</button>';。
    '<button type="button" id="ask-stop" class="chat-copy" hidden>停止回答</button>';
  // 【逐行批注】 执行当前前端表达式：messages.before(toolbar);。
  messages.before(toolbar);
  // 【逐行批注】 声明前端变量 clear，保存本行计算出的数据或对象。
  const clear = $('ask-clear'), stop = $('ask-stop');
  if (attached) clear.textContent = attached.examQuestion ? '清除本题对话' : '清除本资料对话';
  // 自动滚动到最新回答时，停止入口仍与输入框相邻。
  // 【逐行批注】 执行当前前端表达式：$('ask-form').append(stop);。
  $('ask-form').append(stop);
  let retryButton;
  const learningTools = window.GangyiWorkspace?.compactToolbar(document.querySelector('main header h1'));
  if (learningTools) {
    const main = document.querySelector('main');
    const intro = main.querySelector('header');
    const materialName = $('ask-material')?.querySelector('p span')?.textContent;
    document.querySelector('.gy-learning-title').textContent = materialName ? 'AI 导师 · ' + materialName : 'AI 导师';
    $('ask-empty').textContent = '可以贴上题干、写出你的思路，或说说你卡在哪一步。';
    main.classList.add('gy-ask-workspace');
    messages.tabIndex = 0;
    messages.setAttribute('aria-label', '导师对话');
    if (attached && attached.unavailable !== 'true') {
      learningTools.append(learningTools.disclose('当前学习材料', $('ask-material')));
    }
    learningTools.append(clear, learningTools.disclose('导师说明与执行过程', intro.lastElementChild, $('ask-agent-trace')));
    intro.remove();
    toolbar.remove();
    retryButton = document.createElement('button');
    retryButton.type = 'button';
    retryButton.className = 'chat-copy';
    retryButton.textContent = '重试回答';
    retryButton.hidden = true;
    retryButton.addEventListener('click', () => retryLast());
    $('ask-form').append(retryButton);
  }
  // 【逐行批注】 计算右侧结果并更新 submit.disabled，改变页面或请求状态。
  submit.disabled = true; clear.disabled = true;

  // 【逐行批注】 声明前端变量 FALLBACK_SUGGESTIONS，保存本行计算出的数据或对象。
  const FALLBACK_SUGGESTIONS = [];

  // 【逐行批注】 定义函数 addMine，封装可复用的前端行为。
  function addMine(text) {
    // 【逐行批注】 执行当前前端表达式：$('ask-empty')?.remove();。
    $('ask-empty')?.remove();
    // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
    const row = document.createElement('article');
    // 【逐行批注】 计算右侧结果并更新 row.className，改变页面或请求状态。
    row.className = 'chat-row chat-row-mine';
    // 【逐行批注】 声明前端变量 bubble，保存本行计算出的数据或对象。
    const bubble = document.createElement('div');
    // 【逐行批注】 计算右侧结果并更新 bubble.className，改变页面或请求状态。
    bubble.className = 'chat-bubble chat-bubble-mine';
    // 【逐行批注】 计算右侧结果并更新 bubble.textContent，改变页面或请求状态。
    bubble.textContent = text;
    // 【逐行批注】 执行当前前端表达式：row.append(bubble);。
    row.append(bubble);
    // 【逐行批注】 执行当前前端表达式：messages.append(row);。
    messages.append(row);
    // 【逐行批注】 结束当前函数并返回结果：return row;。
    return row;
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 addStatic，封装可复用的前端行为。
  function addStatic(text, mine) {
    // 【逐行批注】 判断条件 (mine) return addMine(text);，成立时执行对应逻辑。
    if (mine) return addMine(text);
    // 【逐行批注】 执行当前前端表达式：$('ask-empty')?.remove();。
    $('ask-empty')?.remove();
    // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
    const row = document.createElement('article');
    // 【逐行批注】 计算右侧结果并更新 row.className，改变页面或请求状态。
    row.className = 'chat-row chat-row-ai';
    // 【逐行批注】 声明前端变量 bubble，保存本行计算出的数据或对象。
    const bubble = document.createElement('div');
    // 【逐行批注】 计算右侧结果并更新 bubble.className，改变页面或请求状态。
    bubble.className = 'chat-bubble chat-bubble-ai';
    // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
    const body = document.createElement('div');
    // 【逐行批注】 计算右侧结果并更新 body.className，改变页面或请求状态。
    body.className = 'chat-body';
    // 【逐行批注】 计算右侧结果并更新 body.innerHTML，改变页面或请求状态。
    body.innerHTML = chat.renderMarkdown(text, { math: true });
    // 【逐行批注】 执行当前前端表达式：bubble.append(body);。
    bubble.append(body);
    // 【逐行批注】 执行当前前端表达式：row.append(bubble);。
    row.append(bubble);
    // 【逐行批注】 执行当前前端表达式：messages.append(row);。
    messages.append(row);
    // 【逐行批注】 结束当前函数并返回结果：return row;。
    return row;
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 busy，封装可复用的前端行为。
  function busy(state) {
    // 【逐行批注】 计算右侧结果并更新 submit.disabled，改变页面或请求状态。
    submit.disabled = state || !ready;
    // 【逐行批注】 计算右侧结果并更新 clear.disabled，改变页面或请求状态。
    clear.disabled = state || !ready;
    // 【逐行批注】 计算右侧结果并更新 submit.textContent，改变页面或请求状态。
    submit.textContent = state ? '正在回答…' : '发送';
    // 【逐行批注】 计算右侧结果并更新 stop.hidden，改变页面或请求状态。
    stop.hidden = !state;
    // 【逐行批注】 计算右侧结果并更新 stop.disabled，改变页面或请求状态。
    stop.disabled = false;
    // 容器整体标记忙碌，读屏与样式都能看到提交中的状态。
    // 【逐行批注】 声明前端变量 form，保存本行计算出的数据或对象。
    const form = $('ask-form');
    // 【逐行批注】 判断条件 (state && typeof form?.setAttribute === 'function') form.setAttr…，成立时执行对应逻辑。
    if (state && typeof form?.setAttribute === 'function') form.setAttribute('aria-busy', 'true');
    // 【逐行批注】 当前一条件不成立时继续判断：(!state && typeof form?.removeAttribute === 'function') form.rem…。
    else if (!state && typeof form?.removeAttribute === 'function') form.removeAttribute('aria-busy');
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 loadSuggestions，封装可复用的前端行为。
  async function loadSuggestions(actions, question, answer) {
    // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
    const host = actions.querySelector('.chat-chips');
    // 【逐行批注】 声明前端变量 items，保存本行计算出的数据或对象。
    let items = FALLBACK_SUGGESTIONS;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch('/api/ask/suggestions', { method: 'POST', headers: { 'Content-Type': 'application/json' },
        // 【逐行批注】 执行当前前端表达式：body: JSON.stringify({ question, answer }) });。
        body: JSON.stringify({ question, answer }) });
      // 【逐行批注】 判断条件 (response.ok) {，成立时执行对应逻辑。
      if (response.ok) {
        // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
        const data = await response.json();
        // 【逐行批注】 声明前端变量 list，保存本行计算出的数据或对象。
        const list = (data.suggestions || []).filter(item => typeof item === 'string' && item.trim()).slice(0, 3);
        // 【逐行批注】 判断条件 (list.length) items = list;，成立时执行对应逻辑。
        if (list.length) items = list;
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 执行当前前端表达式：} catch (_) {}。
    } catch (_) {}
    // 【逐行批注】 计算右侧结果并更新 host.innerHTML，改变页面或请求状态。
    host.innerHTML = '';
    // 【逐行批注】 按循环规则遍历或重复处理：for (const text of items) {。
    for (const text of items) {
      // 【逐行批注】 声明前端变量 chip，保存本行计算出的数据或对象。
      const chip = document.createElement('button');
      // 【逐行批注】 计算右侧结果并更新 chip.type，改变页面或请求状态。
      chip.type = 'button';
      // 【逐行批注】 计算右侧结果并更新 chip.className，改变页面或请求状态。
      chip.className = 'chat-chip';
      // 【逐行批注】 计算右侧结果并更新 chip.textContent，改变页面或请求状态。
      chip.textContent = text;
      // 【逐行批注】 为页面事件绑定处理逻辑：chip.addEventListener('click', () => { input.value = text; input.focus(); ask(text);…。
      chip.addEventListener('click', () => { input.value = text; input.focus(); ask(text); });
      // 【逐行批注】 执行当前前端表达式：host.append(chip);。
      host.append(chip);
    // 【逐行批注】 结束当前前端代码块。
    }
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
        // 【逐行批注】 执行当前前端表达式：view.bubble.append(note);。
        view.bubble.append(note);
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
      // 【逐行批注】 调用 loadSuggestions 执行页面交互或数据处理：loadSuggestions(actions, question, answer);。
      loadSuggestions(actions, question, answer);
      // 【逐行批注】 执行当前前端表达式：historyMessages.push({ role: 'user', content: question }, { role: 'assistant', content: a…。
      historyMessages.push({ role: 'user', content: question }, { role: 'assistant', content: answer });
      // 【逐行批注】 执行当前前端表达式：history.replaceState({}, '', '/ask');。
      if (!attached) history.replaceState({}, '', '/ask');
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 ask，封装可复用的前端行为。
  function ask(question, options = {}) {
    // 【逐行批注】 计算右侧结果并更新 question，改变页面或请求状态。
    question = String(question || '').trim();
    // 已在生成时保持当前请求，等结束后再接受新的意图。
    if (!question || !ready || socket) return;
    if (!attached && window.GangyiStudyIntent?.yesterday(question)) {
      location.assign('/review-yesterday');
      return;
    }
    // 【逐行批注】 计算右侧结果并更新 lastQuestion，改变页面或请求状态。
    lastQuestion = question;
    // 从失败步骤重试时复用同一条提问气泡，不重复追加历史。
    // 【逐行批注】 判断条件 (!options.repeat) addMine(question);，成立时执行对应逻辑。
    if (!options.repeat) addMine(question);
    // 【逐行批注】 计算右侧结果并更新 input.value，改变页面或请求状态。
    input.value = '';
    // 【逐行批注】 调用 busy 执行页面交互或数据处理：busy(true);。
    busy(true);
    // 【逐行批注】 声明前端变量 view，保存本行计算出的数据或对象。
    const view = chat.createAssistantView(messages, { scrollRoot: learningTools ? messages : window });
    // 【逐行批注】 声明前端变量 finished，保存本行计算出的数据或对象。
    let finished = false, received = 0, lastProgressPaint = 0;
    // 【逐行批注】 声明前端变量 live，保存本行计算出的数据或对象。
    const live = {id:'ask-live-' + Date.now(), status:'pending', purpose:'AI 导师回答', createdAt:new Date().toISOString(),
      // 【逐行批注】 调用 Date 执行页面交互或数据处理：calls:1, activity:{title:'连接 AI 导师',detail:'问题已提交，正在建立流式连接。',state:'s…。
      calls:1, activity:{title:'连接 AI 导师',detail:'问题已提交，正在建立流式连接。',state:'started',at:new Date().toISOString()}, events:[]};
    // 变值才写：状态与阶段变化立即更新，计时和接收进度由轨迹组件自行刷新。
    // 【逐行批注】 声明前端变量 paint，保存本行计算出的数据或对象。
    const paint = () => {
      live.updatedAt = new Date().toISOString();
      trace?.update(live);
      if (retryButton) retryButton.hidden = live.status !== 'failed';
    };
    // 【逐行批注】 调用 paint 执行页面交互或数据处理：paint();。
    paint();
    // 【逐行批注】 声明前端变量 connection，保存本行计算出的数据或对象。
    const connection = new WebSocket((location.protocol === 'https:' ? 'wss:' : 'ws:') + '//' + location.host + '/ws/ask');
    // 【逐行批注】 计算右侧结果并更新 socket，改变页面或请求状态。
    socket = connection;
    // 【逐行批注】 计算右侧结果并更新 connection.onopen，改变页面或请求状态。
    connection.onopen = () => { live.status='running';live.activity={title:'接收 AI 导师回复',detail:'回答会逐字显示；连接中断时会明确提示。',state:'started',at:new Date().toISOString()};paint();connection.send(JSON.stringify({ type: 'ask', question,
      // 【逐行批注】 执行当前前端表达式：...(knownHistory ? { messages: historyMessages.slice(-8) } : {}) }));。
      ...materialContext,
      ...(knownHistory ? { messages: historyMessages.slice(-8) } : {}) }));
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 计算右侧结果并更新 connection.onmessage，改变页面或请求状态。
    connection.onmessage = event => {
      // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
      const data = JSON.parse(event.data);
      // 【逐行批注】 判断条件 (data.type === 'delta') {，成立时执行对应逻辑。
      if (data.type === 'delta') {
        // 【逐行批注】 执行当前前端表达式：view.receive(data);。
        view.receive(data);
        // 只记录接收进度，不把模型正文写进轨迹历史。
        // 【逐行批注】 计算右侧结果并更新 received +，改变页面或请求状态。
        received += 1;
        // 【逐行批注】 计算右侧结果并更新 live.activity，改变页面或请求状态。
        live.activity = {title:'接收 AI 导师回复',detail:'已接收 ' + received + ' 段内容，文字正在显示。',state:'started',at:new Date().toISOString()};
        // 【逐行批注】 声明前端变量 now，保存本行计算出的数据或对象。
        const now = Date.now();
        // 【逐行批注】 判断条件 (!lastProgressPaint || now - lastProgressPaint > 400) { lastProg…，成立时执行对应逻辑。
        if (!lastProgressPaint || now - lastProgressPaint > 400) { lastProgressPaint = now; paint(); }
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (data.type === 'done') { finished = true; live.status=data.cance…，成立时执行对应逻辑。
        if (data.type === 'stream_status') view.receive(data);
      if (data.type === 'done') { finished = true; live.status=data.cancelled?'cancelled':'ready';live.activity={title:data.cancelled?'已停止回答':'回答已完成',detail:data.cancelled?'本次回答未保存。':'回答已完整接收'+(received?'（共 '+received+' 段）':'')+'。',state:'completed',at:new Date().toISOString()};paint();finish(view, question, data); connection.close(); }
      // 【逐行批注】 判断条件 (data.type === 'error') { finished = true; live.status='failed';…，成立时执行对应逻辑。
      if (data.type === 'error') { finished = true; live.status='failed';live.failure={message:data.message||'回答未完成'};live.activity={title:'回答未完成',detail:data.message||'连接中断，已有文字仍可查看。',state:'failed',at:new Date().toISOString()};paint();view.fail(data.message); connection.close(); }
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 计算右侧结果并更新 connection.onerror，改变页面或请求状态。
    connection.onerror = () => {
      // 完成后关闭连接可能产生迟到事件，不覆盖已成功保存的回答。
      // 【逐行批注】 判断条件 (finished) return;，成立时执行对应逻辑。
      if (finished) return;
      // 【逐行批注】 计算右侧结果并更新 finished，改变页面或请求状态。
      finished = true; live.status='failed';live.failure={message:'连接中断，请重试。'};live.activity={title:'连接中断',detail:'已收到的文字仍可查看，可以重新提问。',state:'failed',at:new Date().toISOString()};paint();view.fail('连接中断，请重试。'); connection.close();
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 计算右侧结果并更新 connection.onclose，改变页面或请求状态。
    connection.onclose = () => {
      // 未收到 done 就断开时，轨迹必须一并结束，否则会永久显示“运行中”。
      // 【逐行批注】 判断条件 (!finished) {，成立时执行对应逻辑。
      if (!finished) {
        // 【逐行批注】 计算右侧结果并更新 finished，改变页面或请求状态。
        finished = true;
        // 【逐行批注】 计算右侧结果并更新 live.status，改变页面或请求状态。
        live.status='failed';
        // 【逐行批注】 计算右侧结果并更新 live.failure，改变页面或请求状态。
        live.failure={message:'回答中断，本次未完成。'};
        // 【逐行批注】 计算右侧结果并更新 live.activity，改变页面或请求状态。
        live.activity={title:'回答中断',detail:received?'已接收 '+received+' 段内容仍可查看，可以重新提问。':'连接在回答完成前中断，可以重新提问。',state:'failed',at:new Date().toISOString()};
        // 【逐行批注】 调用 paint 执行页面交互或数据处理：paint();。
        paint();
        // 【逐行批注】 执行当前前端表达式：view.fail('回答中断，本次未完成。');。
        view.fail('回答中断，本次未完成。');
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (socket !== connection) return;，成立时执行对应逻辑。
      if (socket !== connection) return;
      // 【逐行批注】 计算右侧结果并更新 socket，改变页面或请求状态。
      socket = null;
      // 【逐行批注】 调用 busy 执行页面交互或数据处理：busy(false);。
      busy(false);
      // 【逐行批注】 执行当前前端表达式：input.focus();。
      input.focus();
    // 【逐行批注】 执行当前前端表达式：};。
    };
  // 【逐行批注】 结束当前前端代码块。
  }

  // 连接尚未就绪时先排队，等 open 后再发送；结束状态由服务端确认，避免丢事件。
  // 【逐行批注】 定义函数 requestStop，封装可复用的前端行为。
  function requestStop() {
    // 【逐行批注】 判断条件 (!socket) return;，成立时执行对应逻辑。
    if (!socket) return;
    // 【逐行批注】 声明前端变量 connection，保存本行计算出的数据或对象。
    const connection = socket;
    // 【逐行批注】 计算右侧结果并更新 stop.disabled，改变页面或请求状态。
    stop.disabled = true;
    // 【逐行批注】 声明前端变量 request，保存本行计算出的数据或对象。
    const request = () => { try { connection.send(JSON.stringify({ type: 'stop' })); } catch (_) {} };
    // 【逐行批注】 判断条件 (connection.readyState === WebSocket.OPEN) request();，成立时执行对应逻辑。
    if (connection.readyState === WebSocket.OPEN) request();
    // 【逐行批注】 为页面事件绑定处理逻辑：else if (connection.readyState === WebSocket.CONNECTING) connection.addEventListener…。
    else if (connection.readyState === WebSocket.CONNECTING) connection.addEventListener('open', request, { once: true });
  // 【逐行批注】 结束当前前端代码块。
  }
  // 轨迹里的“从失败步骤重试”复用同一问题，不重复追加提问气泡。
  // 【逐行批注】 定义函数 retryLast，封装可复用的前端行为。
  function retryLast() {
    // 【逐行批注】 判断条件 (!lastQuestion || socket) return;，成立时执行对应逻辑。
    if (!lastQuestion || socket) return;
    // 【逐行批注】 结束当前函数并返回结果：return ask(lastQuestion, { repeat: true });。
    return ask(lastQuestion, { repeat: true });
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 为页面事件绑定处理逻辑：stop.addEventListener('click', () => requestStop());。
  stop.addEventListener('click', () => requestStop());

  // 【逐行批注】 为页面事件绑定处理逻辑：$('ask-form').addEventListener('submit', event => { event.preventDefault(); ask(inpu…。
  $('ask-form').addEventListener('submit', event => { event.preventDefault(); ask(input.value); });
  // 【逐行批注】 为页面事件绑定处理逻辑：input.addEventListener('keydown', event => {。
  input.addEventListener('keydown', event => {
    // 【逐行批注】 判断条件 (event.key === 'Enter' && !event.shiftKey && !event.isComposing)…，成立时执行对应逻辑。
    if (event.key === 'Enter' && !event.shiftKey && !event.isComposing) { event.preventDefault(); $('ask-form').requestSubmit(); }
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 为页面事件绑定处理逻辑：clear.addEventListener('click', async () => {。
  clear.addEventListener('click', async () => {
    // 【逐行批注】 判断条件 (socket || !ready || !confirm('确定清除此电脑上的 AI 对话记录吗？')) return;，成立时执行对应逻辑。
    if (socket || !ready || !confirm(attached ? '确定清除当前学习对话吗？' : '确定清除此电脑上的 AI 对话记录吗？')) return;
    // 【逐行批注】 计算右侧结果并更新 ready，改变页面或请求状态。
    ready = false; busy(false); clear.disabled = true;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch(conversationUrl, { method: 'DELETE' });
      // 【逐行批注】 判断条件 (!response.ok) throw new Error('清除失败，请重试。');，成立时执行对应逻辑。
      if (!response.ok) throw new Error('清除失败，请重试。');
      // 【逐行批注】 计算右侧结果并更新 historyMessages，改变页面或请求状态。
      historyMessages = []; knownHistory = true; messages.replaceChildren();
      // 【逐行批注】 执行当前前端表达式：messages.append(Object.assign(document.createElement('p'), { id: 'ask-empty', className: …。
      messages.append(Object.assign(document.createElement('p'), { id: 'ask-empty', className: 'chat-empty', textContent: '对话已清除。可以继续提问。' }));
    // 【逐行批注】 调用 addStatic 执行页面交互或数据处理：} catch (error) { addStatic(error.message, false); }。
    } catch (error) { addStatic(error.message, false); }
    // 【逐行批注】 计算右侧结果并更新 finally { ready，改变页面或请求状态。
    finally { ready = true; busy(false); }
  // 【逐行批注】 执行当前前端表达式：});。
  });

  // 【逐行批注】 定义函数 load，封装可复用的前端行为。
  async function load() {
    if (attached?.unavailable) {
      input.disabled = true;
      return;
    }
    if (attached) input.placeholder = attached.examQuestion ? '例如：我看不懂题目的条件，请给我一点提示' : '例如：先讲清这份资料的重点，再解释我看不懂的地方';
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch(conversationUrl);
      // 【逐行批注】 判断条件 (!response.ok) throw new Error('历史记录读取失败，提问时将由服务端读取。');，成立时执行对应逻辑。
      if (!response.ok) throw new Error('历史记录读取失败，提问时将由服务端读取。');
      // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
      const data = await response.json();
      // 【逐行批注】 按循环规则遍历或重复处理：for (const item of data.messages || []) {。
      for (const item of data.messages || []) {
        // 【逐行批注】 调用 addStatic 执行页面交互或数据处理：addStatic(item.text, item.role === 'user');。
        addStatic(item.text, item.role === 'user');
        // 【逐行批注】 执行当前前端表达式：historyMessages.push({ role: item.role, content: item.text });。
        historyMessages.push({ role: item.role, content: item.text });
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 计算右侧结果并更新 knownHistory，改变页面或请求状态。
      knownHistory = true;
    // 【逐行批注】 调用 addStatic 执行页面交互或数据处理：} catch (error) { addStatic(error.message, false); }。
    } catch (error) { addStatic(error.message, false); }
    // 【逐行批注】 计算右侧结果并更新 ready，改变页面或请求状态。
    ready = true;
    // 【逐行批注】 调用 busy 执行页面交互或数据处理：busy(false);。
    busy(false);
    // 【逐行批注】 声明前端变量 initial，保存本行计算出的数据或对象。
    const initial = window.gangyiInitialQuestion?.trim();
    if (!initial && !attached) {
      const draft = new URLSearchParams(location.search).get('draft');
      if (draft) input.value = draft;
    }
    // 【逐行批注】 判断条件 (initial && historyMessages.at(-2)?.content !== initial) ask(ini…，成立时执行对应逻辑。
    if (initial && historyMessages.at(-2)?.content !== initial) ask(initial);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 调用 load 执行页面交互或数据处理：load();。
  load();
// 【逐行批注】 执行当前前端表达式：})();。
})();

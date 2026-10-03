/* AI 导师页：流式回答、Markdown 排版、复制与追问建议。 */
(() => {
  const $ = id => document.getElementById(id);
  const messages = $('ask-messages'), input = $('ask-question'), submit = $('ask-submit');
  const chat = window.GangyiChat;
  let socket = null, ready = false, knownHistory = false, historyMessages = [];

  const toolbar = document.createElement('div');
  toolbar.className = 'chat-toolbar';
  toolbar.innerHTML = '<button type="button" id="ask-clear" class="chat-copy">清除本机对话</button>' +
    '<button type="button" id="ask-stop" class="chat-copy" hidden>停止回答</button>';
  messages.before(toolbar);
  const clear = $('ask-clear'), stop = $('ask-stop');
  submit.disabled = true; clear.disabled = true;

  const FALLBACK_SUGGESTIONS = ['能用更简单的说法再讲一遍吗？', '举个例子说明一下', '出一道类似的练习题'];

  function addMine(text) {
    $('ask-empty')?.remove();
    const row = document.createElement('article');
    row.className = 'chat-row chat-row-mine';
    const bubble = document.createElement('div');
    bubble.className = 'chat-bubble chat-bubble-mine';
    bubble.textContent = text;
    row.append(bubble);
    messages.append(row);
    return row;
  }

  function addStatic(text, mine) {
    if (mine) return addMine(text);
    $('ask-empty')?.remove();
    const row = document.createElement('article');
    row.className = 'chat-row chat-row-ai';
    const bubble = document.createElement('div');
    bubble.className = 'chat-bubble chat-bubble-ai';
    const body = document.createElement('div');
    body.className = 'chat-body';
    body.innerHTML = chat.renderMarkdown(text, { math: true });
    bubble.append(body);
    row.append(bubble);
    messages.append(row);
    return row;
  }

  function busy(state) {
    submit.disabled = state || !ready;
    clear.disabled = state || !ready;
    submit.textContent = state ? '正在回答…' : '发送';
    stop.hidden = !state;
    stop.disabled = false;
  }

  async function loadSuggestions(actions, question, answer) {
    const host = actions.querySelector('.chat-chips');
    let items = FALLBACK_SUGGESTIONS;
    try {
      const response = await fetch('/api/ask/suggestions', { method: 'POST', headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ question, answer }) });
      if (response.ok) {
        const data = await response.json();
        const list = (data.suggestions || []).filter(item => typeof item === 'string' && item.trim()).slice(0, 3);
        if (list.length) items = list;
      }
    } catch (_) {}
    host.innerHTML = '';
    for (const text of items) {
      const chip = document.createElement('button');
      chip.type = 'button';
      chip.className = 'chat-chip';
      chip.textContent = text;
      chip.addEventListener('click', () => { input.value = text; input.focus(); ask(text); });
      host.append(chip);
    }
  }

  function finish(view, question, data) {
    view.complete(() => {
      const answer = view.text();
      if (data && data.cancelled) {
        const note = document.createElement('p');
        note.className = 'chat-error';
        note.textContent = '已停止，本次回答未保存。';
        view.bubble.append(note);
        return;
      }
      const actions = view.addActions('<button type="button" class="chat-copy">复制</button><span class="chat-chips"></span>');
      const copyButton = actions.querySelector('.chat-copy');
      copyButton.addEventListener('click', async () => {
        try { await chat.copyText(answer); copyButton.textContent = '已复制'; }
        catch (_) { copyButton.textContent = '复制失败'; }
        setTimeout(() => { copyButton.textContent = '复制'; }, 1600);
      });
      loadSuggestions(actions, question, answer);
      historyMessages.push({ role: 'user', content: question }, { role: 'assistant', content: answer });
      history.replaceState({}, '', '/ask');
    });
  }

  function ask(question) {
    question = String(question || '').trim();
    if (!question || !ready || socket) return;
    addMine(question);
    input.value = '';
    busy(true);
    const view = chat.createAssistantView(messages, { scrollRoot: window });
    let finished = false;
    const connection = new WebSocket((location.protocol === 'https:' ? 'wss:' : 'ws:') + '//' + location.host + '/ws/ask');
    socket = connection;
    connection.onopen = () => connection.send(JSON.stringify({ type: 'ask', question,
      ...(knownHistory ? { messages: historyMessages.slice(-8) } : {}) }));
    connection.onmessage = event => {
      const data = JSON.parse(event.data);
      if (data.type === 'delta') view.push(data.text);
      if (data.type === 'done') { finished = true; finish(view, question, data); connection.close(); }
      if (data.type === 'error') { finished = true; view.fail(data.message); connection.close(); }
    };
    connection.onerror = () => { finished = true; view.fail('连接中断，请重试。'); connection.close(); };
    connection.onclose = () => {
      if (!finished) view.fail('回答中断，本次未完成。');
      if (socket !== connection) return;
      socket = null;
      busy(false);
      input.focus();
    };
  }

  // 连接尚未就绪时先排队，等 open 后再发送；结束状态由服务端确认，避免丢事件。
  stop.addEventListener('click', () => {
    if (!socket) return;
    const connection = socket;
    stop.disabled = true;
    const request = () => { try { connection.send(JSON.stringify({ type: 'stop' })); } catch (_) {} };
    if (connection.readyState === WebSocket.OPEN) request();
    else if (connection.readyState === WebSocket.CONNECTING) connection.addEventListener('open', request, { once: true });
  });

  $('ask-form').addEventListener('submit', event => { event.preventDefault(); ask(input.value); });
  input.addEventListener('keydown', event => {
    if (event.key === 'Enter' && !event.shiftKey && !event.isComposing) { event.preventDefault(); $('ask-form').requestSubmit(); }
  });
  clear.addEventListener('click', async () => {
    if (socket || !ready || !confirm('确定清除此电脑上的 AI 对话记录吗？')) return;
    ready = false; busy(false); clear.disabled = true;
    try {
      const response = await fetch('/api/conversations/general', { method: 'DELETE' });
      if (!response.ok) throw new Error('清除失败，请重试。');
      historyMessages = []; knownHistory = true; messages.replaceChildren();
      messages.append(Object.assign(document.createElement('p'), { id: 'ask-empty', className: 'chat-empty', textContent: '对话已清除。可以继续提问。' }));
    } catch (error) { addStatic(error.message, false); }
    finally { ready = true; busy(false); }
  });

  async function load() {
    try {
      const response = await fetch('/api/conversations/general');
      if (!response.ok) throw new Error('历史记录读取失败，提问时将由服务端读取。');
      const data = await response.json();
      for (const item of data.messages || []) {
        addStatic(item.text, item.role === 'user');
        historyMessages.push({ role: item.role, content: item.text });
      }
      knownHistory = true;
    } catch (error) { addStatic(error.message, false); }
    ready = true;
    busy(false);
    const initial = window.gangyiInitialQuestion?.trim();
    if (initial && historyMessages.at(-2)?.content !== initial) ask(initial);
  }
  load();
})();

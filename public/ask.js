(() => {
  const $ = id => document.getElementById(id);
  const messages = $('ask-messages'), input = $('ask-question'), submit = $('ask-submit');
  let socket = null, ready = false, knownHistory = false, historyMessages = [], pending = null;
  const clear = document.createElement('button');
  clear.type = 'button'; clear.className = 'ask-clear'; clear.textContent = '清除本机对话';
  messages.before(clear);
  const stop = document.createElement('button');
  stop.type = 'button'; stop.id = 'ask-stop'; stop.className = 'ask-clear'; stop.textContent = '停止回答'; stop.hidden = true;
  submit.after(stop);
  submit.disabled = true; clear.disabled = true;

  function add(text, mine = false) {
    $('ask-empty')?.remove();
    const article = document.createElement('article');
    article.className = mine ? 'flex justify-end' : 'flex justify-start';
    const bubble = document.createElement('div');
    bubble.className = 'ask-bubble max-w-[92%] whitespace-pre-wrap rounded-3xl px-5 py-4 text-sm leading-7 ' +
      (mine ? 'bg-sky-700 text-white' : 'border border-slate-200 bg-slate-50 text-slate-700');
    bubble.textContent = text; article.append(bubble); messages.append(article);
    article.scrollIntoView({behavior: matchMedia('(prefers-reduced-motion: reduce)').matches ? 'instant' : 'smooth', block: 'end'});
    return bubble;
  }

  function ask(question) {
    question = question.trim();
    if (!question || !ready || socket) return;
    add(question, true); input.value = ''; submit.disabled = true; clear.disabled = true;
    submit.textContent = '正在回答…'; stop.hidden = false;
    const reply = add('钢一定制AI 正在思考…'); pending = reply;
    let content = '', finished = false;
    const connection = new WebSocket((location.protocol === 'https:' ? 'wss:' : 'ws:') + '//' + location.host + '/ws/ask');
    socket = connection;
    connection.onopen = () => connection.send(JSON.stringify({type: 'ask', question,
      ...(knownHistory ? {messages: historyMessages.slice(-8)} : {})}));
    connection.onmessage = event => {
      const data = JSON.parse(event.data);
      if (data.type === 'delta') { content += data.text; reply.textContent = content; }
      if (data.type === 'done') {
        finished = true;
        if (data.cancelled) reply.textContent = content + '\n已停止，本次回答未保存。';
        else { historyMessages.push({role: 'user', content: question}, {role: 'assistant', content}); history.replaceState({}, '', '/ask'); }
        connection.close();
      }
      if (data.type === 'error') { finished = true; reply.textContent = content + '\n' + data.message; connection.close(); }
    };
    connection.onerror = () => { finished = true; reply.textContent = content + '\n连接中断，请重试。'; connection.close(); };
    connection.onclose = () => {
      if (!finished && socket === connection) reply.textContent = content + '\n回答中断，本次未完成。';
      if (socket !== connection) return;
      socket = null; pending = null; submit.disabled = false; clear.disabled = false;
      submit.textContent = '发送'; stop.hidden = true; input.focus();
    };
  }

  stop.onclick = () => {
    if (!socket) return;
    if (socket.readyState === WebSocket.OPEN) socket.send(JSON.stringify({type: 'stop'}));
    if (pending) pending.textContent += '\n已停止，本次回答未保存。';
    const connection = socket; socket = null; pending = null; connection.close();
    submit.disabled = false; clear.disabled = false; submit.textContent = '发送'; stop.hidden = true; input.focus();
  };
  $('ask-form').addEventListener('submit', event => { event.preventDefault(); ask(input.value); });
  input.addEventListener('keydown', event => {
    if (event.key === 'Enter' && !event.shiftKey && !event.isComposing) { event.preventDefault(); $('ask-form').requestSubmit(); }
  });
  clear.addEventListener('click', async () => {
    if (socket || !ready || !confirm('确定清除此电脑上的 AI 对话记录吗？')) return;
    ready = false; clear.disabled = true; submit.disabled = true;
    try {
      const response = await fetch('/api/conversations/general', {method: 'DELETE'});
      if (!response.ok) throw new Error('清除失败，请重试。');
      historyMessages = []; knownHistory = true; messages.replaceChildren();
      messages.append(Object.assign(document.createElement('p'), {id: 'ask-empty', textContent: '对话已清除。可以继续提问。'}));
    } catch (error) { add(error.message); }
    finally { ready = true; clear.disabled = false; submit.disabled = false; }
  });

  async function load() {
    try {
      const response = await fetch('/api/conversations/general');
      if (!response.ok) throw new Error('历史记录读取失败，提问时将由服务端读取。');
      const data = await response.json();
      for (const item of data.messages || []) {
        add(item.text, item.role === 'user'); historyMessages.push({role: item.role, content: item.text});
      }
      knownHistory = true;
    } catch (error) { add(error.message); }
    ready = true; submit.disabled = false; clear.disabled = false;
    const initial = window.gangyiInitialQuestion?.trim();
    if (initial && historyMessages.at(-2)?.content !== initial) ask(initial);
  }
  load();
})();

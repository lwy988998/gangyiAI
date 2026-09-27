(() => {
  const $ = id => document.getElementById(id);
  const messages = $('ask-messages');
  const input = $('ask-question');
  const submit = $('ask-submit');
  const clear = document.createElement('button');
  clear.type = 'button';
  clear.className = 'ask-clear';
  clear.textContent = '清除本机对话';
  messages.before(clear);

  function add(text, mine = false) {
    $('ask-empty')?.remove();
    const article = document.createElement('article');
    article.className = mine ? 'flex justify-end' : 'flex justify-start';
    const bubble = document.createElement('div');
    bubble.className = 'ask-bubble max-w-[92%] whitespace-pre-wrap rounded-3xl px-5 py-4 text-sm leading-7 ' +
      (mine ? 'bg-sky-700 text-white' : 'border border-slate-200 bg-slate-50 text-slate-700');
    bubble.textContent = text;
    article.append(bubble);
    messages.append(article);
    article.scrollIntoView({behavior: matchMedia('(prefers-reduced-motion: reduce)').matches ? 'instant' : 'smooth', block: 'end'});
    return bubble;
  }

  async function ask(question) {
    question = question.trim();
    if (!question) return;
    add(question, true);
    input.value = '';
    submit.disabled = true;
    submit.textContent = '正在思考…';
    const pending = add('钢一定制AI 正在思考…');
    try {
      const response = await fetch('/api/ask', {method: 'POST', headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({question, conversationId: 'general'})});
      const data = await response.json();
      if (!response.ok) throw new Error(data.error || '回答暂未生成完成');
      pending.textContent = data.answer;
      history.replaceState({}, '', '/ask');
    } catch (error) { pending.textContent = error.message || '回答暂未生成完成，请稍后重试。'; }
    finally { submit.disabled = false; submit.textContent = '发送'; input.focus(); }
  }

  $('ask-form').addEventListener('submit', event => { event.preventDefault(); ask(input.value); });
  input.addEventListener('keydown', event => {
    if (event.key === 'Enter' && !event.shiftKey) { event.preventDefault(); $('ask-form').requestSubmit(); }
  });
  clear.addEventListener('click', async () => {
    if (!confirm('确定清除此电脑上的 AI 对话记录吗？')) return;
    clear.disabled = true;
    try {
      const response = await fetch('/api/conversations/general', {method: 'DELETE'});
      if (!response.ok) throw new Error('清除失败，请重试。');
      messages.replaceChildren();
      messages.append(Object.assign(document.createElement('p'), {id: 'ask-empty', textContent: '对话已清除。可以继续提问。'}));
    } catch (error) { add(error.message); }
    finally { clear.disabled = false; }
  });

  async function load() {
    try {
      const response = await fetch('/api/conversations/general');
      if (!response.ok) throw new Error('历史记录读取失败');
      const data = await response.json();
      for (const item of data.messages || []) add(item.text, item.role === 'user');
      const initial = window.gangyiInitialQuestion?.trim();
      if (initial && (data.messages || []).at(-2)?.text !== initial) await ask(initial);
    } catch (error) { add(error.message); }
  }
  load();
})();

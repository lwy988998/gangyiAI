/* 统一主控客户端：订阅真实事件，普通状态读取和恢复不会生成新内容。 */
(() => {
  'use strict';
  const request = async (path, data) => {
    const response = await fetch(path, data === undefined ? { cache: 'no-store' } : {
      method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(data),
    });
    const value = await response.json();
    if (!response.ok) throw new Error(value.error || '请求未完成，请稍后重试。');
    return value;
  };
  const storage = {
    get(key) { try { return localStorage.getItem(`gangyi-agent:${key}`) || ''; } catch (_) { return ''; } },
    set(key, value) { try { localStorage.setItem(`gangyi-agent:${key}`, value); } catch (_) { /* 存储不可用时保留当前输入。 */ } },
    remove(key) { try { localStorage.removeItem(`gangyi-agent:${key}`); } catch (_) { /* 不影响已保存记录。 */ } },
  };
  const id = () => globalThis.crypto?.randomUUID?.() || `${Date.now()}-${Math.random().toString(16).slice(2)}`;
  function watch(taskId, handlers = {}) {
    let sequence = handlers.afterSeq || 0, socket, timer, stopped = false;
    const events = new Set();
    const receive = event => {
      if (!Number.isInteger(event.seq) || event.seq <= sequence || events.has(event.seq)) return;
      events.add(event.seq); sequence = event.seq; handlers.onEvent?.(event);
    };
    const state = task => {
      for (const event of task.events || []) receive(event);
      handlers.onState?.(task);
      if (!['pending', 'running'].includes(task.status)) { stopped = true; clearTimeout(timer); socket?.close(); }
    };
    async function poll() {
      if (stopped) return;
      try { state(await request(`/api/learning-agent?taskId=${encodeURIComponent(taskId)}`)); }
      catch (error) { handlers.onConnectionError?.(error.message); }
      if (!stopped) timer = setTimeout(poll, 1200);
    }
    request(`/api/learning-agent?taskId=${encodeURIComponent(taskId)}`).then(task => {
      state(task);
      if (stopped) return;
      socket = new WebSocket(`${location.protocol === 'https:' ? 'wss:' : 'ws:'}//${location.host}/ws/learning-agent`);
      socket.addEventListener('open', () => socket.send(JSON.stringify({ taskId, afterSeq: sequence })));
      socket.addEventListener('message', message => {
        try {
          const event = JSON.parse(message.data);
          if (event.type === 'state') state(event.task); else receive(event);
        } catch (_) { handlers.onConnectionError?.('实时连接暂时中断，已保存的内容仍可恢复。'); }
      });
      socket.addEventListener('close', () => { if (!stopped) timer = setTimeout(poll, 400); });
      socket.addEventListener('error', () => socket.close());
    }).catch(error => { handlers.onConnectionError?.(error.message); timer = setTimeout(poll, 1200); });
    return () => { stopped = true; clearTimeout(timer); socket?.close(); };
  }
  const richText = (element, value) => {
    if (window.GangyiChat) element.innerHTML = window.GangyiChat.renderMarkdown(String(value), { math: true });
    else element.textContent = String(value);
  };
  window.GangyiAgent = { request, watch, storage, id, richText,
    submit: event => request('/api/learning-agent/events', event),
    control: value => request('/api/learning-agent/control', value),
  };
})();

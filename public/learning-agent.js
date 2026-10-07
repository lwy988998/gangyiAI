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
  const exposures = new Map();
  const sectionExposures = new Map();
  const flushExposure = async () => {
    await Promise.all([...sectionExposures].map(async ([lessonId, sections]) => {
      await request('/api/learn/exposure', {lessonId, sections: [...sections.values()]});
      if (sectionExposures.get(lessonId) === sections) sectionExposures.delete(lessonId);
    }));
    await Promise.all([...exposures].map(async ([taskId, value]) => {
      await request('/api/learn/exposure', { taskId, seq: value.seq, displayedSequences: [...value.displayedSequences] });
      if (exposures.get(taskId) === value) exposures.delete(taskId);
    }));
  };
  function watch(taskId, handlers = {}) {
    let sequence = handlers.afterSeq || 0, socket, timer, stopped = false;
    const events = new Set();
    const receive = event => {
      if (stopped || !Number.isInteger(event.seq) || event.seq <= sequence || events.has(event.seq)) return;
      events.add(event.seq); sequence = event.seq;
      const shown = handlers.onEvent ? handlers.onEvent(event) !== false : false;
      const previous = exposures.get(taskId), displayedSequences = new Set(previous?.displayedSequences || []);
      if (shown && event.type === 'delta' && event.field === 'message') displayedSequences.add(event.seq);
      exposures.set(taskId, { seq: Math.max(sequence, previous?.seq || 0), displayedSequences });
    };
    const state = task => {
      if (stopped) return;
      for (const event of task.events || []) receive(event);
      handlers.onState?.(task);
      flushExposure().catch(() => { /* 下一次学生提交前再次保存已展示序号。 */ });
      if (!['pending', 'running'].includes(task.status)) { stopped = true; clearTimeout(timer); socket?.close(); }
    };
    async function poll() {
      if (stopped) return;
      try { state(await request(`/api/learning-agent?taskId=${encodeURIComponent(taskId)}`)); }
      catch (error) { if(!stopped)handlers.onConnectionError?.(error.message); }
      if (!stopped) timer = setTimeout(poll, 1200);
    }
    request(`/api/learning-agent?taskId=${encodeURIComponent(taskId)}`).then(task => {
      state(task);
      if (stopped) return;
      socket = new WebSocket(`${location.protocol === 'https:' ? 'wss:' : 'ws:'}//${location.host}/ws/learning-agent`);
      socket.addEventListener('open', () => { if(stopped){socket.close();return} socket.send(JSON.stringify({ taskId, afterSeq: sequence })); });
      socket.addEventListener('message', message => {
        if(stopped)return;
        try {
          const event = JSON.parse(message.data);
          if (event.type === 'state') state(event.task); else receive(event);
        } catch (_) { handlers.onConnectionError?.('实时连接暂时中断，已保存的内容仍可恢复。'); }
      });
      socket.addEventListener('close', () => { if (!stopped) timer = setTimeout(poll, 400); });
      socket.addEventListener('error', () => socket.close());
    }).catch(error => { if(stopped)return; handlers.onConnectionError?.(error.message); timer = setTimeout(poll, 1200); });
    return () => { stopped = true; clearTimeout(timer); socket?.close(); };
  }
  const richText = (element, value) => {
    if (window.GangyiChat) element.innerHTML = window.GangyiChat.renderMarkdown(String(value), { math: true });
    else element.textContent = String(value);
  };
  window.GangyiAgent = { request, watch, storage, id, richText, flushExposure,
    recordSections(lessonId, sections) {
      if (!sections.length) return;
      const saved = new Map(sectionExposures.get(lessonId) || []);
      for (const section of sections) saved.set(section.id, section);
      sectionExposures.set(lessonId, saved);
      flushExposure().catch(() => { /* 学生提交前再次保存已展示板块。 */ });
    },
    submit: async event => { await flushExposure(); return request('/api/learning-agent/events', event); },
    control: value => request('/api/learning-agent/control', value),
  };
  window.addEventListener('pagehide', () => {
    for (const [lessonId, sections] of sectionExposures) navigator.sendBeacon('/api/learn/exposure',
      new Blob([JSON.stringify({lessonId, sections: [...sections.values()]})], {type:'application/json'}));
    for (const [taskId, value] of exposures) navigator.sendBeacon('/api/learn/exposure',
      new Blob([JSON.stringify({ taskId, seq: value.seq, displayedSequences: [...value.displayedSequences] })], { type: 'application/json' }));
  });
})();

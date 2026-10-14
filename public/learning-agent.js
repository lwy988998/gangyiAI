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
  const evaluationExposures = new Map(), evaluationBodies = new Map();
  const shownMessages = new Map(); let exposureTimer;
  const flushExposure = async () => {
    await Promise.all([...evaluationExposures].map(async ([lessonId, evaluations]) => {
      await request('/api/learn/exposure', {lessonId, evaluations: [...evaluations.values()]});
      if (evaluationExposures.get(lessonId) === evaluations) evaluationExposures.delete(lessonId);
    }));
    await Promise.all([...sectionExposures].map(async ([lessonId, sections]) => {
      await request('/api/learn/exposure', {lessonId, sections: [...sections.values()]});
      if (sectionExposures.get(lessonId) === sections) sectionExposures.delete(lessonId);
    }));
    await Promise.all([...exposures].map(async ([taskId, value]) => {
      await request('/api/learn/exposure', { taskId, seq: value.seq, displayedSequences: [...value.displayedSequences] });
      if (exposures.get(taskId) === value) exposures.delete(taskId);
    }));
  };
  // 反馈正文实际进入视区才记录；长反馈逐段合并可见范围，未看完整时保留不确定性。
  function recordEvaluation(lessonId, record, node, viewportNode) {
    if (document.hidden || !record.feedback || !node?.isConnected || node.closest('details:not([open])') || node.closest('[hidden]')) return;
    const bounds = node.getBoundingClientRect(), viewport = viewportNode?.getBoundingClientRect() ||
      {top:0, bottom:innerHeight, left:0, right:innerWidth};
    if (bounds.height <= 0 || bounds.bottom <= viewport.top || bounds.top >= viewport.bottom || bounds.right <= viewport.left || bounds.left >= viewport.right) return;
    const key = lessonId + ':' + record.id;
    let state = evaluationBodies.get(key);
    if (state?.feedback === record.feedback && state.complete) return;
    if (!state || state.feedback !== record.feedback || Math.abs(state.height-bounds.height)>1)
      state = {feedback:record.feedback, height:bounds.height, ranges:[], complete:false, reported:false};
    state.ranges.push([Math.max(0, viewport.top-bounds.top), Math.min(bounds.height, viewport.bottom-bounds.top)]);
    state.ranges.sort((a,b)=>a[0]-b[0]);
    const merged = [];
    for (const range of state.ranges) {
      const last=merged.at(-1);
      if (last && range[0] <= last[1]+1) last[1]=Math.max(last[1],range[1]); else merged.push([...range]);
    }
    state.ranges=merged; state.complete=merged.length===1 && merged[0][0]<=1 && merged[0][1]>=bounds.height-1;
    const fresh=!state.reported || state.complete; state.reported=true; evaluationBodies.set(key,state);
    if (!fresh) return;
    const pending = new Map(evaluationExposures.get(lessonId) || []);
    pending.set(record.id,{id:record.id, feedback:record.feedback, complete:state.complete});
    evaluationExposures.set(lessonId,pending);
    flushExposure().catch(()=>{ /* 下次提交或页面离开前保留未确认的可见反馈。 */ });
  }
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
      if (!['pending', 'running'].includes(task.status) && !(task.status === 'paused' && task.pauseReason === 'global')) { stopped = true; clearTimeout(timer); socket?.close(); }
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
  const failureMessage = task => task?.failure?.message || task?.error || 'AI 请求未完成，输入和已有有效结果已保留，可重试。';
  window.GangyiAgent = { request, watch, storage, id, richText, flushExposure, failureMessage, recordEvaluation,
    recordMessages(taskId, seq, sequences) {
      const shown = shownMessages.get(taskId) || new Set();
      const fresh = sequences.filter(sequence => !shown.has(sequence));
      if (!fresh.length) return;
      fresh.forEach(sequence => shown.add(sequence)); shownMessages.set(taskId, shown);
      const previous = exposures.get(taskId), displayedSequences = new Set(previous?.displayedSequences || []);
      for (const sequence of sequences) displayedSequences.add(sequence);
      exposures.set(taskId, {seq: Math.max(seq, previous?.seq || 0), displayedSequences});
      clearTimeout(exposureTimer); exposureTimer = setTimeout(() => flushExposure().catch(() => {
        /* 网络恢复或下次提交时继续保存，未确认的已读队列仍保留。 */
      }), 400);
    },
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
    for (const [lessonId, evaluations] of evaluationExposures) navigator.sendBeacon('/api/learn/exposure',
      new Blob([JSON.stringify({lessonId, evaluations:[...evaluations.values()]})], {type:'application/json'}));
    for (const [lessonId, sections] of sectionExposures) navigator.sendBeacon('/api/learn/exposure',
      new Blob([JSON.stringify({lessonId, sections: [...sections.values()]})], {type:'application/json'}));
    for (const [taskId, value] of exposures) navigator.sendBeacon('/api/learn/exposure',
      new Blob([JSON.stringify({ taskId, seq: value.seq, displayedSequences: [...value.displayedSequences] })], { type: 'application/json' }));
  });
})();

/* 统一主控客户端：订阅真实事件，普通状态读取和恢复不会生成新内容。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 声明前端变量 request，保存本行计算出的数据或对象。
  const request = async (path, data, options = {}) => {
    // 【逐行批注】 声明前端变量 controller，保存本行计算出的数据或对象。
    const controller = new AbortController();
    // 【逐行批注】 声明前端变量 timeoutMs，保存本行计算出的数据或对象。
    const timeoutMs = Number(options.timeoutMs || 20000);
    // 【逐行批注】 声明前端变量 timer，保存本行计算出的数据或对象。
    const timer = setTimeout(() => controller.abort(), timeoutMs);
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch(path, data === undefined
        // 【逐行批注】 执行当前前端表达式：? { cache: 'no-store', signal: controller.signal }。
        ? { cache: 'no-store', signal: controller.signal }
        // 【逐行批注】 执行当前前端表达式：: { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify…。
        : { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(data), signal: controller.signal });
      // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
      const value = await response.json();
      // 【逐行批注】 判断条件 (!response.ok) throw new Error(value.error || '请求未完成，请稍后重试。');，成立时执行对应逻辑。
      if (!response.ok) throw new Error(value.error || '请求未完成，请稍后重试。');
      // 【逐行批注】 结束当前函数并返回结果：return value;。
      return value;
    // 【逐行批注】 执行当前前端表达式：} catch (error) {。
    } catch (error) {
      // 【逐行批注】 判断条件 (controller.signal.aborted)，成立时执行对应逻辑。
      if (controller.signal.aborted)
        // 【逐行批注】 在错误分支抛出异常：throw new Error('请求超过 20 秒未返回；任务可能仍在后台，请打开顶部“AI 状态”查看。');。
        throw new Error('请求超过 20 秒未返回；任务可能仍在后台，请打开顶部“AI 状态”查看。');
      // 【逐行批注】 在错误分支抛出异常：throw error;。
      throw error;
    // 【逐行批注】 执行当前前端表达式：} finally {。
    } finally {
      // 【逐行批注】 调用 clearTimeout 执行页面交互或数据处理：clearTimeout(timer);。
      clearTimeout(timer);
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 storage，保存本行计算出的数据或对象。
  const storage = {
    // 【逐行批注】 调用 get 执行页面交互或数据处理：get(key) { try { return localStorage.getItem(`gangyi-agent:${key}`) |…。
    get(key) { try { return localStorage.getItem(`gangyi-agent:${key}`) || ''; } catch (_) { return ''; } },
    // 【逐行批注】 调用 set 执行页面交互或数据处理：set(key, value) { try { localStorage.setItem(`gangyi-agent:${key}`, v…。
    set(key, value) { try { localStorage.setItem(`gangyi-agent:${key}`, value); } catch (_) { /* 存储不可用时保留当前输入。 */ } },
    // 【逐行批注】 调用 remove 执行页面交互或数据处理：remove(key) { try { localStorage.removeItem(`gangyi-agent:${key}`); }…。
    remove(key) { try { localStorage.removeItem(`gangyi-agent:${key}`); } catch (_) { /* 不影响已保存记录。 */ } },
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 id，保存本行计算出的数据或对象。
  const id = () => globalThis.crypto?.randomUUID?.() || `${Date.now()}-${Math.random().toString(16).slice(2)}`;
  // 【逐行批注】 声明前端变量 exposures，保存本行计算出的数据或对象。
  const exposures = new Map();
  // 【逐行批注】 声明前端变量 sectionExposures，保存本行计算出的数据或对象。
  const sectionExposures = new Map();
  // 【逐行批注】 声明前端变量 evaluationExposures，保存本行计算出的数据或对象。
  const evaluationExposures = new Map(), evaluationBodies = new Map();
  // 【逐行批注】 声明前端变量 shownMessages，保存本行计算出的数据或对象。
  const shownMessages = new Map(); let exposureTimer;
  // 【逐行批注】 声明前端变量 completeMessages，保存本行计算出的数据或对象。
  const completeMessages = new Map(), displayBodies = new Map();
  // 【逐行批注】 声明前端变量 flushExposure，保存本行计算出的数据或对象。
  const flushExposure = async () => {
    // 【逐行批注】 计算右侧结果并更新 await Promise.all([...evaluationExposures].m…，改变页面或请求状态。
    await Promise.all([...evaluationExposures].map(async ([lessonId, evaluations]) => {
      // 【逐行批注】 调用 request 执行页面交互或数据处理：await request('/api/learn/exposure', {lessonId, evaluations: [...eval…。
      await request('/api/learn/exposure', {lessonId, evaluations: [...evaluations.values()]});
      // 【逐行批注】 判断条件 (evaluationExposures.get(lessonId) === evaluations) evaluationEx…，成立时执行对应逻辑。
      if (evaluationExposures.get(lessonId) === evaluations) evaluationExposures.delete(lessonId);
    // 【逐行批注】 执行当前前端表达式：}));。
    }));
    // 【逐行批注】 计算右侧结果并更新 await Promise.all([...sectionExposures].map(…，改变页面或请求状态。
    await Promise.all([...sectionExposures].map(async ([lessonId, sections]) => {
      // 【逐行批注】 调用 request 执行页面交互或数据处理：await request('/api/learn/exposure', {lessonId, sections: [...section…。
      await request('/api/learn/exposure', {lessonId, sections: [...sections.values()]});
      // 【逐行批注】 判断条件 (sectionExposures.get(lessonId) === sections) sectionExposures.d…，成立时执行对应逻辑。
      if (sectionExposures.get(lessonId) === sections) sectionExposures.delete(lessonId);
    // 【逐行批注】 执行当前前端表达式：}));。
    }));
    // 【逐行批注】 计算右侧结果并更新 await Promise.all([...exposures].map(async (…，改变页面或请求状态。
    await Promise.all([...exposures].map(async ([taskId, value]) => {
      // 【逐行批注】 调用 request 执行页面交互或数据处理：await request('/api/learn/exposure', { taskId, seq: value.seq, displa…。
      await request('/api/learn/exposure', { taskId, seq: value.seq, displayedSequences: [...value.displayedSequences], completeSequences:[...(value.completeSequences || [])] });
      // 【逐行批注】 判断条件 (exposures.get(taskId) === value) exposures.delete(taskId);，成立时执行对应逻辑。
      if (exposures.get(taskId) === value) exposures.delete(taskId);
    // 【逐行批注】 执行当前前端表达式：}));。
    }));
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 正文逐段进入视区才累计，不能因标题或卡片边缘可见就确认整段示范。
  // 【逐行批注】 定义函数 displayCoverage，封装可复用的前端行为。
  function displayCoverage(key, signature, node, viewportNode) {
    // 【逐行批注】 判断条件 (document.hidden || !node?.isConnected || node.closest('details:…，成立时执行对应逻辑。
    if (document.hidden || !node?.isConnected || node.closest('details:not([open])') || node.closest('[hidden]')) return null;
    // 【逐行批注】 声明前端变量 bounds，保存本行计算出的数据或对象。
    const bounds=node.getBoundingClientRect(), viewport=viewportNode?.getBoundingClientRect() || {top:0,bottom:innerHeight,left:0,right:innerWidth};
    // 【逐行批注】 判断条件 (bounds.height<=0 || bounds.bottom<=viewport.top || bounds.top>=…，成立时执行对应逻辑。
    if (bounds.height<=0 || bounds.bottom<=viewport.top || bounds.top>=viewport.bottom || bounds.right<=viewport.left || bounds.left>=viewport.right) return null;
    // 【逐行批注】 声明前端变量 state，保存本行计算出的数据或对象。
    let state=displayBodies.get(key);
    // 【逐行批注】 判断条件 (!state || state.signature!==signature || Math.abs(state.height-…，成立时执行对应逻辑。
    if (!state || state.signature!==signature || Math.abs(state.height-bounds.height)>1)
      // 【逐行批注】 计算右侧结果并更新 state，改变页面或请求状态。
      state={signature,height:bounds.height,ranges:[],complete:false};
    // 【逐行批注】 声明前端变量 pendingImage，保存本行计算出的数据或对象。
    const pendingImage=[...(node.querySelectorAll?.('img') || [])].some(image=>!image.complete || image.naturalWidth===0);
    // 【逐行批注】 判断条件 (!state.complete && !pendingImage) {，成立时执行对应逻辑。
    if (!state.complete && !pendingImage) {
      // 【逐行批注】 执行当前前端表达式：state.ranges.push([Math.max(0,viewport.top-bounds.top),Math.min(bounds.height,viewport.bo…。
      state.ranges.push([Math.max(0,viewport.top-bounds.top),Math.min(bounds.height,viewport.bottom-bounds.top)]);
      // 【逐行批注】 计算右侧结果并更新 state.ranges.sort((a,b)，改变页面或请求状态。
      state.ranges.sort((a,b)=>a[0]-b[0]); const merged=[];
      // 【逐行批注】 按循环规则遍历或重复处理：for (const range of state.ranges) {。
      for (const range of state.ranges) {
        // 【逐行批注】 声明前端变量 last，保存本行计算出的数据或对象。
        const last=merged.at(-1);
        // 【逐行批注】 判断条件 (last && range[0]<=last[1]+1) last[1]=Math.max(last[1],range[1])…，成立时执行对应逻辑。
        if (last && range[0]<=last[1]+1) last[1]=Math.max(last[1],range[1]); else merged.push([...range]);
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 计算右侧结果并更新 state.ranges，改变页面或请求状态。
      state.ranges=merged; state.complete=merged.length===1 && merged[0][0]<=1 && merged[0][1]>=bounds.height-1;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 执行当前前端表达式：displayBodies.set(key,state); return state;。
    displayBodies.set(key,state); return state;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 反馈正文实际进入视区才记录；长反馈逐段合并可见范围，未看完整时保留不确定性。
  // 【逐行批注】 定义函数 recordEvaluation，封装可复用的前端行为。
  function recordEvaluation(lessonId, record, node, viewportNode) {
    // 【逐行批注】 判断条件 (document.hidden || !record.feedback || !node?.isConnected || no…，成立时执行对应逻辑。
    if (document.hidden || !record.feedback || !node?.isConnected || node.closest('details:not([open])') || node.closest('[hidden]')) return;
    // 【逐行批注】 声明前端变量 bounds，保存本行计算出的数据或对象。
    const bounds = node.getBoundingClientRect(), viewport = viewportNode?.getBoundingClientRect() ||
      // 【逐行批注】 执行当前前端表达式：{top:0, bottom:innerHeight, left:0, right:innerWidth};。
      {top:0, bottom:innerHeight, left:0, right:innerWidth};
    // 【逐行批注】 判断条件 (bounds.height <= 0 || bounds.bottom <= viewport.top || bounds.t…，成立时执行对应逻辑。
    if (bounds.height <= 0 || bounds.bottom <= viewport.top || bounds.top >= viewport.bottom || bounds.right <= viewport.left || bounds.left >= viewport.right) return;
    // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
    const key = lessonId + ':' + record.id;
    // 【逐行批注】 声明前端变量 state，保存本行计算出的数据或对象。
    let state = evaluationBodies.get(key);
    // 【逐行批注】 判断条件 (state?.feedback === record.feedback && state.complete) return;，成立时执行对应逻辑。
    if (state?.feedback === record.feedback && state.complete) return;
    // 【逐行批注】 判断条件 (!state || state.feedback !== record.feedback || Math.abs(state.…，成立时执行对应逻辑。
    if (!state || state.feedback !== record.feedback || Math.abs(state.height-bounds.height)>1)
      // 【逐行批注】 计算右侧结果并更新 state，改变页面或请求状态。
      state = {feedback:record.feedback, height:bounds.height, ranges:[], complete:false, reported:false};
    // 【逐行批注】 执行当前前端表达式：state.ranges.push([Math.max(0, viewport.top-bounds.top), Math.min(bounds.height, viewport…。
    state.ranges.push([Math.max(0, viewport.top-bounds.top), Math.min(bounds.height, viewport.bottom-bounds.top)]);
    // 【逐行批注】 计算右侧结果并更新 state.ranges.sort((a,b)，改变页面或请求状态。
    state.ranges.sort((a,b)=>a[0]-b[0]);
    // 【逐行批注】 声明前端变量 merged，保存本行计算出的数据或对象。
    const merged = [];
    // 【逐行批注】 按循环规则遍历或重复处理：for (const range of state.ranges) {。
    for (const range of state.ranges) {
      // 【逐行批注】 声明前端变量 last，保存本行计算出的数据或对象。
      const last=merged.at(-1);
      // 【逐行批注】 判断条件 (last && range[0] <= last[1]+1) last[1]=Math.max(last[1],range[1…，成立时执行对应逻辑。
      if (last && range[0] <= last[1]+1) last[1]=Math.max(last[1],range[1]); else merged.push([...range]);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 state.ranges，改变页面或请求状态。
    state.ranges=merged; state.complete=merged.length===1 && merged[0][0]<=1 && merged[0][1]>=bounds.height-1;
    // 【逐行批注】 声明前端变量 fresh，保存本行计算出的数据或对象。
    const fresh=!state.reported || state.complete; state.reported=true; evaluationBodies.set(key,state);
    // 【逐行批注】 判断条件 (!fresh) return;，成立时执行对应逻辑。
    if (!fresh) return;
    // 【逐行批注】 声明前端变量 pending，保存本行计算出的数据或对象。
    const pending = new Map(evaluationExposures.get(lessonId) || []);
    // 【逐行批注】 执行当前前端表达式：pending.set(record.id,{id:record.id, feedback:record.feedback, complete:state.complete});。
    pending.set(record.id,{id:record.id, feedback:record.feedback, complete:state.complete});
    // 【逐行批注】 执行当前前端表达式：evaluationExposures.set(lessonId,pending);。
    evaluationExposures.set(lessonId,pending);
    // 【逐行批注】 计算右侧结果并更新 flushExposure().catch(()，改变页面或请求状态。
    flushExposure().catch(()=>{ /* 下次提交或页面离开前保留未确认的可见反馈。 */ });
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 watch，封装可复用的前端行为。
  function watch(taskId, handlers = {}) {
    // 【逐行批注】 声明前端变量 sequence，保存本行计算出的数据或对象。
    let sequence = handlers.afterSeq || 0, socket, timer, socketTimer, stopped = false;
    // 【逐行批注】 声明前端变量 events，保存本行计算出的数据或对象。
    const events = new Set();
    // 【逐行批注】 声明前端变量 receive，保存本行计算出的数据或对象。
    const receive = event => {
      // 【逐行批注】 判断条件 (stopped || !Number.isInteger(event.seq) || event.seq <= sequenc…，成立时执行对应逻辑。
      if (stopped || !Number.isInteger(event.seq) || event.seq <= sequence || events.has(event.seq)) return;
      // 【逐行批注】 计算右侧结果并更新 events.add(event.seq); sequence，改变页面或请求状态。
      events.add(event.seq); sequence = event.seq;
      // 【逐行批注】 声明前端变量 shown，保存本行计算出的数据或对象。
      const shown = handlers.onEvent ? handlers.onEvent(event) !== false : false;
      // 【逐行批注】 声明前端变量 previous，保存本行计算出的数据或对象。
      const previous = exposures.get(taskId), displayedSequences = new Set(previous?.displayedSequences || []);
      // 【逐行批注】 判断条件 (shown && event.type === 'delta' && event.field === 'message') d…，成立时执行对应逻辑。
      if (shown && event.type === 'delta' && event.field === 'message') displayedSequences.add(event.seq);
      // 【逐行批注】 执行当前前端表达式：exposures.set(taskId, { ...previous, seq: Math.max(sequence, previous?.seq || 0), display…。
      exposures.set(taskId, { ...previous, seq: Math.max(sequence, previous?.seq || 0), displayedSequences });
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 声明前端变量 state，保存本行计算出的数据或对象。
    const state = task => {
      // 【逐行批注】 判断条件 (stopped) return;，成立时执行对应逻辑。
      if (stopped) return;
      // 【逐行批注】 按循环规则遍历或重复处理：for (const event of task.events || []) receive(event);。
      for (const event of task.events || []) receive(event);
      // 【逐行批注】 执行当前前端表达式：handlers.onState?.(task);。
      handlers.onState?.(task);
      // 【逐行批注】 计算右侧结果并更新 flushExposure().catch(()，改变页面或请求状态。
      flushExposure().catch(() => { /* 下一次学生提交前再次保存已展示序号。 */ });
      // 【逐行批注】 判断条件 (!['pending', 'running'].includes(task.status) && !(task.status …，成立时执行对应逻辑。
      if (!['pending', 'running'].includes(task.status) && !(task.status === 'paused' && task.pauseReason === 'global')) { stopped = true; clearTimeout(timer); clearTimeout(socketTimer); socket?.close(); }
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 定义函数 poll，封装可复用的前端行为。
    async function poll() {
      // 【逐行批注】 判断条件 (stopped) return;，成立时执行对应逻辑。
      if (stopped) return;
      // 【逐行批注】 计算右侧结果并更新 try { state(await request(`/api/learning-age…，改变页面或请求状态。
      try { state(await request(`/api/learning-agent?taskId=${encodeURIComponent(taskId)}`)); }
      // 【逐行批注】 执行当前前端表达式：catch (error) { if(!stopped)handlers.onConnectionError?.(error.message); }。
      catch (error) { if(!stopped)handlers.onConnectionError?.(error.message); }
      // 【逐行批注】 判断条件 (!stopped) timer = setTimeout(poll, 1200);，成立时执行对应逻辑。
      if (!stopped) timer = setTimeout(poll, 1200);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 request(`/api/learning-agent?taskId，改变页面或请求状态。
    request(`/api/learning-agent?taskId=${encodeURIComponent(taskId)}`).then(task => {
      // 【逐行批注】 调用 state 执行页面交互或数据处理：state(task);。
      state(task);
      // 【逐行批注】 判断条件 (stopped) return;，成立时执行对应逻辑。
      if (stopped) return;
      // 【逐行批注】 计算右侧结果并更新 socket，改变页面或请求状态。
      socket = new WebSocket(`${location.protocol === 'https:' ? 'wss:' : 'ws:'}//${location.host}/ws/learning-agent`);
      // 【逐行批注】 计算右侧结果并更新 socketTimer，改变页面或请求状态。
      socketTimer = setTimeout(() => {
        // 【逐行批注】 判断条件 (stopped || socket?.readyState === WebSocket.OPEN) return;，成立时执行对应逻辑。
        if (stopped || socket?.readyState === WebSocket.OPEN) return;
        // 【逐行批注】 执行当前前端表达式：socket?.close();。
        socket?.close();
        // 【逐行批注】 判断条件 (!timer) poll();，成立时执行对应逻辑。
        if (!timer) poll();
      // 【逐行批注】 执行当前前端表达式：}, 15000);。
      }, 15000);
      // 【逐行批注】 为页面事件绑定处理逻辑：socket.addEventListener('open', () => { clearTimeout(socketTimer); if(stopped){socke…。
      socket.addEventListener('open', () => { clearTimeout(socketTimer); if(stopped){socket.close();return} socket.send(JSON.stringify({ taskId, afterSeq: sequence })); });
      // 【逐行批注】 为页面事件绑定处理逻辑：socket.addEventListener('message', message => {。
      socket.addEventListener('message', message => {
        // 【逐行批注】 执行当前前端表达式：if(stopped)return;。
        if(stopped)return;
        // 【逐行批注】 执行当前前端表达式：try {。
        try {
          // 【逐行批注】 声明前端变量 event，保存本行计算出的数据或对象。
          const event = JSON.parse(message.data);
          // 【逐行批注】 判断条件 (event.type === 'state') state(event.task); else receive(event);，成立时执行对应逻辑。
          if (event.type === 'state') state(event.task); else receive(event);
        // 【逐行批注】 执行当前前端表达式：} catch (_) { handlers.onConnectionError?.('实时连接暂时中断，已保存的内容仍可恢复。'); }。
        } catch (_) { handlers.onConnectionError?.('实时连接暂时中断，已保存的内容仍可恢复。'); }
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 为页面事件绑定处理逻辑：socket.addEventListener('close', () => { if (!stopped) timer = setTimeout(poll, 400)…。
      socket.addEventListener('close', () => { if (!stopped) timer = setTimeout(poll, 400); });
      // 【逐行批注】 为页面事件绑定处理逻辑：socket.addEventListener('error', () => socket.close());。
      socket.addEventListener('error', () => socket.close());
    // 【逐行批注】 计算右侧结果并更新 }).catch(error，改变页面或请求状态。
    }).catch(error => { if(stopped)return; handlers.onConnectionError?.(error.message); timer = setTimeout(poll, 1200); });
    // 【逐行批注】 结束当前函数并返回结果：return () => { stopped = true; clearTimeout(timer); clearTimeout(sock…。
    return () => { stopped = true; clearTimeout(timer); clearTimeout(socketTimer); socket?.close(); };
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 richText，保存本行计算出的数据或对象。
  const richText = (element, value) => {
    // 【逐行批注】 判断条件 (window.GangyiChat) element.innerHTML = window.GangyiChat.render…，成立时执行对应逻辑。
    if (window.GangyiChat) element.innerHTML = window.GangyiChat.renderMarkdown(String(value), { math: true });
    // 【逐行批注】 执行前一条件不成立时的分支。
    else element.textContent = String(value);
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 failureMessage，保存本行计算出的数据或对象。
  const failureMessage = task => task?.failure?.message || task?.error || 'AI 请求未完成，输入和已有有效结果已保留，可重试。';
  // 【逐行批注】 计算右侧结果并更新 window.GangyiAgent，改变页面或请求状态。
  window.GangyiAgent = { request, watch, storage, id, richText, flushExposure, failureMessage, recordEvaluation,
    // 【逐行批注】 调用 recordVisibleSection 执行页面交互或数据处理：recordVisibleSection(lessonId, section, node, viewport) {。
    recordVisibleSection(lessonId, section, node, viewport) {
      // 【逐行批注】 声明前端变量 state，保存本行计算出的数据或对象。
      const state=displayCoverage('section:'+lessonId+':'+section.id+':'+section.version, section.body || JSON.stringify(section.question || {}),node,viewport);
      // 【逐行批注】 判断条件 (!state || state.sent===state.complete) return;，成立时执行对应逻辑。
      if (!state || state.sent===state.complete) return;
      // 【逐行批注】 计算右侧结果并更新 state.sent，改变页面或请求状态。
      state.sent=state.complete; this.recordSections(lessonId,[{id:section.id,version:Number(section.version),complete:state.complete}]);
    // 【逐行批注】 执行当前前端表达式：},。
    },
    // 【逐行批注】 调用 recordVisibleMessage 执行页面交互或数据处理：recordVisibleMessage(record, node, viewport) {。
    recordVisibleMessage(record, node, viewport) {
      // 【逐行批注】 判断条件 (!record.taskId || !record.sequences?.length) return;，成立时执行对应逻辑。
      if (!record.taskId || !record.sequences?.length) return;
      // 【逐行批注】 声明前端变量 state，保存本行计算出的数据或对象。
      const state=displayCoverage('message:'+record.taskId+':'+(record.step ?? record.sequences[0]),record.text,node,viewport);
      // 【逐行批注】 判断条件 (state) this.recordMessages(record.taskId,Math.max(...record.seq…，成立时执行对应逻辑。
      if (state) this.recordMessages(record.taskId,Math.max(...record.sequences),record.sequences,state.complete);
    // 【逐行批注】 执行当前前端表达式：},。
    },
    // 【逐行批注】 计算右侧结果并更新 recordMessages(taskId, seq, sequences, compl…，改变页面或请求状态。
    recordMessages(taskId, seq, sequences, complete=false) {
      // 【逐行批注】 声明前端变量 shown，保存本行计算出的数据或对象。
      const shown = shownMessages.get(taskId) || new Set();
      // 【逐行批注】 声明前端变量 fresh，保存本行计算出的数据或对象。
      const fresh = sequences.filter(sequence => !shown.has(sequence));
      // 【逐行批注】 声明前端变量 fullyShown，保存本行计算出的数据或对象。
      const fullyShown=completeMessages.get(taskId) || new Set(), freshComplete=complete ? sequences.filter(sequence=>!fullyShown.has(sequence)) : [];
      // 【逐行批注】 判断条件 (!fresh.length && !freshComplete.length) return;，成立时执行对应逻辑。
      if (!fresh.length && !freshComplete.length) return;
      // 【逐行批注】 计算右侧结果并更新 fresh.forEach(sequence，改变页面或请求状态。
      fresh.forEach(sequence => shown.add(sequence)); shownMessages.set(taskId, shown);
      // 【逐行批注】 声明前端变量 previous，保存本行计算出的数据或对象。
      const previous = exposures.get(taskId), displayedSequences = new Set(previous?.displayedSequences || []);
      // 【逐行批注】 按循环规则遍历或重复处理：for (const sequence of sequences) displayedSequences.add(sequence);。
      for (const sequence of sequences) displayedSequences.add(sequence);
      // 【逐行批注】 计算右侧结果并更新 freshComplete.forEach(sequence，改变页面或请求状态。
      freshComplete.forEach(sequence=>fullyShown.add(sequence)); completeMessages.set(taskId,fullyShown);
      // 【逐行批注】 声明前端变量 completeSequences，保存本行计算出的数据或对象。
      const completeSequences=new Set(previous?.completeSequences || []); freshComplete.forEach(sequence=>completeSequences.add(sequence));
      // 【逐行批注】 执行当前前端表达式：exposures.set(taskId, {...previous,seq: Math.max(seq, previous?.seq || 0), displayedSeque…。
      exposures.set(taskId, {...previous,seq: Math.max(seq, previous?.seq || 0), displayedSequences,completeSequences});
      // 【逐行批注】 计算右侧结果并更新 clearTimeout(exposureTimer); exposureTimer，改变页面或请求状态。
      clearTimeout(exposureTimer); exposureTimer = setTimeout(() => flushExposure().catch(() => {
        /* 网络恢复或下次提交时继续保存，未确认的已读队列仍保留。 */
      // 【逐行批注】 执行当前前端表达式：}), 400);。
      }), 400);
    // 【逐行批注】 执行当前前端表达式：},。
    },
    // 【逐行批注】 调用 recordSections 执行页面交互或数据处理：recordSections(lessonId, sections) {。
    recordSections(lessonId, sections) {
      // 【逐行批注】 判断条件 (!sections.length) return;，成立时执行对应逻辑。
      if (!sections.length) return;
      // 【逐行批注】 声明前端变量 saved，保存本行计算出的数据或对象。
      const saved = new Map(sectionExposures.get(lessonId) || []);
      // 【逐行批注】 按循环规则遍历或重复处理：for (const section of sections) {。
      for (const section of sections) {
        // 【逐行批注】 声明前端变量 previous，保存本行计算出的数据或对象。
        const previous=saved.get(section.id);
        // 【逐行批注】 执行当前前端表达式：saved.set(section.id,{...section,complete:section.complete===true || (previous?.version==…。
        saved.set(section.id,{...section,complete:section.complete===true || (previous?.version===section.version && previous.complete===true)});
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 执行当前前端表达式：sectionExposures.set(lessonId, saved);。
      sectionExposures.set(lessonId, saved);
      // 【逐行批注】 计算右侧结果并更新 flushExposure().catch(()，改变页面或请求状态。
      flushExposure().catch(() => { /* 学生提交前再次保存已展示板块。 */ });
    // 【逐行批注】 执行当前前端表达式：},。
    },
    // 【逐行批注】 计算右侧结果并更新 submit: async event，改变页面或请求状态。
    submit: async event => { await flushExposure(); return request('/api/learning-agent/events', event); },
    // 【逐行批注】 计算右侧结果并更新 control: value，改变页面或请求状态。
    control: value => request('/api/learning-agent/control', value),
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('pagehide', () => {。
  window.addEventListener('pagehide', () => {
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [lessonId, evaluations] of evaluationExposures) navigator.sendB…。
    for (const [lessonId, evaluations] of evaluationExposures) navigator.sendBeacon('/api/learn/exposure',
      // 【逐行批注】 调用 Blob 执行页面交互或数据处理：new Blob([JSON.stringify({lessonId, evaluations:[...evaluations.value…。
      new Blob([JSON.stringify({lessonId, evaluations:[...evaluations.values()]})], {type:'application/json'}));
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [lessonId, sections] of sectionExposures) navigator.sendBeacon(…。
    for (const [lessonId, sections] of sectionExposures) navigator.sendBeacon('/api/learn/exposure',
      // 【逐行批注】 调用 Blob 执行页面交互或数据处理：new Blob([JSON.stringify({lessonId, sections: [...sections.values()]}…。
      new Blob([JSON.stringify({lessonId, sections: [...sections.values()]})], {type:'application/json'}));
    // 【逐行批注】 按循环规则遍历或重复处理：for (const [taskId, value] of exposures) navigator.sendBeacon('/api/learn/…。
    for (const [taskId, value] of exposures) navigator.sendBeacon('/api/learn/exposure',
      // 【逐行批注】 调用 Blob 执行页面交互或数据处理：new Blob([JSON.stringify({ taskId, seq: value.seq, displayedSequences…。
      new Blob([JSON.stringify({ taskId, seq: value.seq, displayedSequences: [...value.displayedSequences],completeSequences:[...(value.completeSequences || [])] })], { type: 'application/json' }));
  // 【逐行批注】 执行当前前端表达式：});。
  });
// 【逐行批注】 执行当前前端表达式：})();。
})();

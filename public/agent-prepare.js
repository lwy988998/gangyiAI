/* 每个入口请求有稳定标识；刷新复用任务，完整保存后才进入同一份课堂。 */
(() => {
  'use strict';
  const api = window.GangyiAgent, query = new URLSearchParams(location.search);
  const courseId = query.get('courseId'), taskId = query.get('taskId');
  let requestId = query.get('requestId');
  if (!requestId) { requestId = api.id(); query.set('requestId', requestId); history.replaceState(null, '', `?${query}`); }
  const status = document.getElementById('prepare-status'), root = document.getElementById('prepare-stream');
  const stop = document.getElementById('prepare-stop'), retry = document.getElementById('prepare-retry'), enter = document.getElementById('prepare-enter');
  const streams = new Map(); let current, close, entered = false, sequence = 0;
  const fieldNode = event => {
    const key = `${event.step}:${event.actionIndex}:${event.field}:${event.optionIndex}`;
    if (!streams.has(key)) {
      const node = document.createElement(event.field === 'title' ? 'h2' : 'div');
      node.className = 'agent-card agent-preview'; root.append(node); streams.set(key, node);
    }
    return streams.get(key);
  };
  function follow(task) {
    close?.(); current = task;
    close = api.watch(task.id, {
      afterSeq: sequence,
      onEvent(event) { sequence = event.seq; if (event.type === 'delta') fieldNode(event).textContent += event.text; },
      async onState(state) {
        current = state; stop.hidden = !['pending', 'running'].includes(state.status);
        retry.hidden = !['failed', 'paused', 'superseded'].includes(state.status);
        status.textContent = ({ pending: '等待 AI 读取最新学习记录', running: '真实 AI 正在逐段准备课程', ready: '完整课件已保存',
          failed: '等待 AI 更新，已有有效步骤已保留', paused: '备课已暂停，可从有效步骤继续', superseded: '学习情况已有变化，旧输出没有应用',
          waiting_student: 'AI 需要你补充信息后再继续备课' })[state.status] || state.status;
        if (state.status === 'ready' && state.lesson && !entered) {
          entered = true; enter.hidden = false; enter.href = state.lesson.href;
          if (state.navigate) {
            try { const saved = await api.control({ command: 'enter_lesson', lessonId: state.lesson.id }); location.assign(saved.href); }
            catch (error) { entered = false; status.textContent = error.message; retry.hidden = false; }
          }
        }
      }, onConnectionError(error) { status.textContent = error; },
    });
  }
  stop.onclick = async () => {
    try { await api.control({ command: 'pause', taskId: current.id }); } catch (error) { status.textContent = error.message; }
  };
  retry.onclick = async () => {
    try { follow(await api.control({ command: 'retry', taskId: current.id })); } catch (error) { status.textContent = error.message; }
  };
  const opening = taskId ? api.request(`/api/learning-agent?taskId=${encodeURIComponent(taskId)}`) :
    api.submit({ type: 'prepare_next', courseId, requestId, navigate: true,
      text: '先分析全部活跃课程中的真实作答、提示和最新反馈，自主选择推进、巩固、复习或跨课程学习。准备完整新课，并流式展示公开教学内容；保存后进入同一份课堂。' });
  opening.then(follow).catch(error => { status.textContent = error.message; stop.hidden = true; });
  window.addEventListener('pagehide', () => close?.());
})();

/* 每个入口请求有稳定标识；刷新复用任务，完整保存后才进入同一份课堂。 */
(() => {
  'use strict';
  const api = window.GangyiAgent, query = new URLSearchParams(location.search);
  const courseId = query.get('courseId'), taskId = query.get('taskId');
  const reviewContext = query.has('review') ? {day:Number(query.get('review')),reviewId:query.get('reviewId')||''} : {};
  const classroomHref = lessonId => {
    const params = new URLSearchParams({lessonId});
    for(const key of ['review','reviewId','day'])if(query.has(key))params.set(key,query.get(key));
    const target = query.has('review') ? 'practice' : ['learn','practice','summary'].includes(query.get('view')) ? query.get('view') : 'learn';
    return '/' + target + '?' + params;
  };
  let requestId = query.get('requestId');
  if (!requestId) { requestId = api.id(); query.set('requestId', requestId); history.replaceState(null, '', `?${query}`); }
  const status = document.getElementById('prepare-status'), root = document.getElementById('prepare-stream');
  const stop = document.getElementById('prepare-stop'), retry = document.getElementById('prepare-retry'), enter = document.getElementById('prepare-enter');
  const feedback = document.getElementById('prepare-feedback'), input = document.getElementById('prepare-input');
  const badge = document.getElementById('prepare-badge'), errorBox = document.getElementById('prepare-error'), empty = document.getElementById('prepare-empty');
  const submitted = document.getElementById('prepare-submitted'), generating = document.getElementById('prepare-generating'), saved = document.getElementById('prepare-saved');
  input.value = api.storage.get(`prepare-draft:${requestId}`);
  input.addEventListener('input', () => api.storage.set(`prepare-draft:${requestId}`, input.value));
  const streams = new Map(), cards = new Map(); let current, close, entered = false, sequence = 0;
  const fieldNode = event => {
    const group = `${current.id}:${event.step}:${event.actionIndex}`, key = `${group}:${event.field}:${event.optionIndex}`;
    if (!cards.has(group)) {
      const card = document.createElement('article'); card.className = 'agent-preview';
      const label = document.createElement('p'); label.className = 'prepare-preview-label'; label.textContent = 'AI 实时生成'; card.append(label); root.append(card); cards.set(group, card);
    }
    if (!streams.has(key)) {
      const isOption = event.field === 'option' || event.field === 'options';
      const node = document.createElement(event.field === 'title' ? 'h2' : isOption ? 'li' : 'div');
      const card = cards.get(group);
      if (isOption) { let options = card.querySelector('ol'); if (!options) { options = document.createElement('ol'); options.className = 'prepare-preview-options'; card.append(options); } options.append(node); }
      else card.append(node);
      streams.set(key, node);
    }
    return streams.get(key);
  };
  function follow(task) {
    close?.(); if (current?.id !== task.id) sequence = 0; current = task;
    window.GangyiNavigation?.setContext({taskId: task.id});
    close = api.watch(task.id, {
      afterSeq: sequence,
      onEvent(event) { sequence = event.seq; if (event.type === 'delta') { empty.hidden = true; fieldNode(event).textContent += event.text; } return false; },
      async onState(state) {
        current = state; stop.hidden = !['pending', 'running'].includes(state.status);
        feedback.hidden = state.status !== 'waiting_student' || Boolean(state.lesson);
        retry.hidden = !['failed', 'paused', 'superseded'].includes(state.status);
        status.textContent = state.status === 'failed' ? api.failureMessage(state) : ({ pending: '等待 AI 读取最新学习记录', running: '真实 AI 正在逐段准备课程', ready: '完整课件已保存',
          paused: '备课已暂停，可从有效步骤继续', superseded: '学习情况已有变化，旧输出没有应用',
          waiting_student: 'AI 需要你补充信息后再继续备课' })[state.status] || state.status;
        badge.textContent = ({pending:'已提交',running:'正在备课',ready:'已保存',failed:'需要处理',paused:'已暂停',superseded:'记录已更新',waiting_student:'等待补充'})[state.status] || '任务已保存';
        submitted.className = 'is-complete'; generating.className = state.status === 'ready' && state.lesson ? 'is-complete' : state.status === 'failed' ? 'is-interrupted' : ['paused','waiting_student','superseded'].includes(state.status) ? 'is-paused' : state.calls > 0 ? 'is-active' : '';
        saved.className = state.status === 'ready' && state.lesson ? 'is-complete' : '';
        document.getElementById('prepare-preview-status').textContent = ['pending','running'].includes(state.status) ? '生成中内容' : cards.size ? '已保留的内容' : '暂无课件预览';
        empty.textContent = ({failed:'本次尚无有效课件，可查看左侧原因后重试。',paused:'任务已暂停，恢复后继续展示生成内容。',waiting_student:'等待你补充信息后继续备课。',superseded:'学习情况已变化，请重试并读取最新记录。'})[state.status] || 'AI 正在读取学习情况，生成的内容会逐段显示在这里。';
        errorBox.hidden = state.status !== 'failed'; errorBox.textContent = state.status === 'failed' ? api.failureMessage(state) : '';
        retry.textContent = state.status === 'paused' ? '恢复备课' : '重试备课';
        if (state.lesson?.title) document.getElementById('prepare-course').textContent = state.lesson.title;
        window.GangyiNavigation?.setContext({taskId: state.id, ...(state.lesson?.courseId || courseId ? {courseId: state.lesson?.courseId || courseId} : {})});
        if (state.status === 'ready' && state.lesson && !entered) {
          entered = true; enter.hidden = false; enter.href = classroomHref(state.lesson.id);
          enter.onclick = async event => {
            event.preventDefault();
            try { const saved = await api.control({ command: 'enter_lesson', lessonId: state.lesson.id }); location.assign(classroomHref(saved.id)); }
            catch (error) { status.textContent = error.message; retry.hidden = false; }
          };
          if (state.navigate) {
            try { const saved = await api.control({ command: 'enter_lesson', lessonId: state.lesson.id }); location.assign(classroomHref(saved.id)); }
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
    retry.disabled = true;
    try { follow(current ? await api.control({ command: current.status === 'paused' ? 'resume' : 'retry', taskId: current.id }) : await openTask()); }
    catch (error) { showOpeningError(error); } finally { retry.disabled = false; }
  };
  feedback.addEventListener('submit', async event => {
    event.preventDefault(); if (!input.value.trim()) return;
    const button = feedback.querySelector('button'); button.disabled = true;
    try {
      const nextId = api.id(), next = await api.submit({ type: 'prepare_next', ...(courseId ? { courseId } : {}),
        requestId: nextId, previousTaskId: current.id, navigate: true, text: input.value.trim(), ...reviewContext,
        ...(query.has('topicId') ? { topicId: query.get('topicId') } : {}) });
      query.set('taskId', next.id); query.set('requestId', nextId); history.replaceState(null, '', `?${query}`);
      api.storage.remove(`prepare-draft:${requestId}`); requestId = nextId; input.value = ''; follow(next);
    } catch (error) { status.textContent = error.message; } finally { button.disabled = false; }
  });
  const openTask = () => taskId ? api.request(`/api/learning-agent?taskId=${encodeURIComponent(taskId)}`) :
    api.submit({ type: 'prepare_next', ...(courseId ? { courseId } : {}), requestId, navigate: true, ...reviewContext,
      ...(query.has('topicId') ? { topicId: query.get('topicId') } : {}),
      ...(query.has('phaseIndex') ? { phaseIndex: Number(query.get('phaseIndex')), topicIndex: Number(query.get('topicIndex') || 1) } : {}),
      text: '先分析全部活跃课程中的真实作答、提示和最新反馈，自主选择推进、巩固、复习或跨课程学习。准备完整新课，并流式展示公开教学内容；保存后进入同一份课堂。' });
  function showOpeningError(error) { status.textContent = error.message; errorBox.textContent = error.message; errorBox.hidden = false; badge.textContent = current ? '需要处理' : '请求未提交'; stop.hidden = true; retry.hidden = false; }
  openTask().then(follow).catch(showOpeningError);
  document.addEventListener('gangyi:agent-control', event => {
    if (event.detail?.all && current && ['pause', 'resume'].includes(event.detail.command)) {
      api.request('/api/learning-agent?taskId=' + encodeURIComponent(current.id)).then(follow).catch(showOpeningError);
    } else if (['resume','retry'].includes(event.detail?.command) && event.detail.task?.id === current?.id) follow(event.detail.task);
  });
  if (courseId) api.request('/api/courses/' + encodeURIComponent(courseId)).then(data => { document.getElementById('prepare-course').textContent = data.course.title; }).catch(() => {});
  window.addEventListener('pagehide', () => close?.());
})();

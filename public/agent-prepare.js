/* 每个入口请求有稳定标识；刷新复用任务，完整保存后才进入同一份课堂。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 声明前端变量 api，保存本行计算出的数据或对象。
  const api = window.GangyiAgent, query = new URLSearchParams(location.search);
  // 【逐行批注】 声明前端变量 courseId，保存本行计算出的数据或对象。
  const courseId = query.get('courseId'), taskId = query.get('taskId');
  // 【逐行批注】 声明前端变量 reviewContext，保存本行计算出的数据或对象。
  const reviewContext = query.has('review') ? {day:Number(query.get('review')),reviewId:query.get('reviewId')||''} : {};
  // 【逐行批注】 声明前端变量 classroomHref，保存本行计算出的数据或对象。
  const classroomHref = lessonId => {
    // 【逐行批注】 声明前端变量 params，保存本行计算出的数据或对象。
    const params = new URLSearchParams({lessonId});
    // 【逐行批注】 按循环规则遍历或重复处理：for(const key of ['review','reviewId','day'])if(query.has(key))params.set(…。
    for(const key of ['review','reviewId','day'])if(query.has(key))params.set(key,query.get(key));
    // 【逐行批注】 声明前端变量 target，保存本行计算出的数据或对象。
    const target = query.has('review') ? 'practice' : ['learn','practice','summary'].includes(query.get('view')) ? query.get('view') : 'learn';
    // 【逐行批注】 结束当前函数并返回结果：return '/' + target + '?' + params;。
    return '/' + target + '?' + params;
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 requestId，保存本行计算出的数据或对象。
  let requestId = query.get('requestId');
  // 【逐行批注】 判断条件 (!requestId) { requestId = api.id(); query.set('requestId', requ…，成立时执行对应逻辑。
  if (!requestId) { requestId = api.id(); query.set('requestId', requestId); history.replaceState(null, '', `?${query}`); }
  // 【逐行批注】 声明前端变量 status，保存本行计算出的数据或对象。
  const status = document.getElementById('prepare-status'), root = document.getElementById('prepare-stream');
  // 【逐行批注】 声明前端变量 stop，保存本行计算出的数据或对象。
  const stop = document.getElementById('prepare-stop'), retry = document.getElementById('prepare-retry'), enter = document.getElementById('prepare-enter');
  // 【逐行批注】 声明前端变量 feedback，保存本行计算出的数据或对象。
  const feedback = document.getElementById('prepare-feedback'), input = document.getElementById('prepare-input');
  // 【逐行批注】 声明前端变量 badge，保存本行计算出的数据或对象。
  const badge = document.getElementById('prepare-badge'), errorBox = document.getElementById('prepare-error'), empty = document.getElementById('prepare-empty');
  // 【逐行批注】 声明前端变量 submitted，保存本行计算出的数据或对象。
  const submitted = document.getElementById('prepare-submitted'), generating = document.getElementById('prepare-generating'), saved = document.getElementById('prepare-saved');
  // 【逐行批注】 声明前端变量 trace，保存本行计算出的数据或对象。
  const trace = window.GangyiTrace?.create(document.getElementById('prepare-trace'), {
    // 【逐行批注】 执行当前前端表达式：variant: 'panel', eyebrow: 'AI 备课执行过程', collapseWhenDone: true,。
    variant: 'panel', eyebrow: 'AI 备课执行过程', collapseWhenDone: true,
    // 【逐行批注】 计算右侧结果并更新 onRetry: ()，改变页面或请求状态。
    onRetry: () => retryTraceTask(),
    // 【逐行批注】 计算右侧结果并更新 onResume: async ()，改变页面或请求状态。
    onResume: async () => { if (!current?.id) return; follow(await api.control({ command: 'resume', taskId: current.id })); },
    // 【逐行批注】 计算右侧结果并更新 onPause: async ()，改变页面或请求状态。
    onPause: async () => { if (!current?.id) return; trace?.update(await api.control({ command: 'pause', taskId: current.id })); },
    // 【逐行批注】 计算右侧结果并更新 onCancel: async ()，改变页面或请求状态。
    onCancel: async () => { if (!current?.id) return; trace?.update(await api.control({ command: 'cancel', taskId: current.id })); },
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 计算右侧结果并更新 input.value，改变页面或请求状态。
  input.value = api.storage.get(`prepare-draft:${requestId}`);
  // 【逐行批注】 为页面事件绑定处理逻辑：input.addEventListener('input', () => api.storage.set(`prepare-draft:${requestId}`, …。
  input.addEventListener('input', () => api.storage.set(`prepare-draft:${requestId}`, input.value));
  // 【逐行批注】 声明前端变量 streams，保存本行计算出的数据或对象。
  const streams = new Map(), cards = new Map(); let current, close, entered = false, sequence = 0;
  // 【逐行批注】 声明前端变量 openedAt，保存本行计算出的数据或对象。
  const openedAt = Date.now();
  // 【逐行批注】 声明前端变量 localFailureRetry，保存本行计算出的数据或对象。
  let localFailureRetry = null;
  // 【逐行批注】 声明前端变量 failureTask，保存本行计算出的数据或对象。
  const failureTask = (purpose, message, startedAt) => {
    // 【逐行批注】 声明前端变量 at，保存本行计算出的数据或对象。
    const at = new Date().toISOString();
    // 【逐行批注】 结束当前函数并返回结果：return {id:'local-' + Date.now(), status:'failed', purpose, calls:0,。
    return {id:'local-' + Date.now(), status:'failed', purpose, calls:0,
      // 【逐行批注】 调用 Date 执行页面交互或数据处理：createdAt:new Date(startedAt || Date.now()).toISOString(), updatedAt:…。
      createdAt:new Date(startedAt || Date.now()).toISOString(), updatedAt:at,
      // 【逐行批注】 执行当前前端表达式：activity:{title:'本次请求未完成', detail:message, state:'failed', at},。
      activity:{title:'本次请求未完成', detail:message, state:'failed', at},
      // 【逐行批注】 执行当前前端表达式：failure:{message}, error:message, events:[]};。
      failure:{message}, error:message, events:[]};
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 提交请求本身失败时还没有服务端任务，构造同形失败任务让轨迹与页面状态一致。
  // 【逐行批注】 定义函数 feedLocalFailure，封装可复用的前端行为。
  function feedLocalFailure(purpose, message, startedAt, retry) {
    // 【逐行批注】 计算右侧结果并更新 localFailureRetry，改变页面或请求状态。
    localFailureRetry = typeof retry === 'function' ? retry : null;
    // 【逐行批注】 调用 failureTask 执行页面交互或数据处理：trace?.update(failureTask(purpose, message, startedAt));。
    trace?.update(failureTask(purpose, message, startedAt));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 retryTraceTask，封装可复用的前端行为。
  async function retryTraceTask() {
    // 【逐行批注】 判断条件 (String(trace?.task?.id || '').startsWith('local-')) {，成立时执行对应逻辑。
    if (String(trace?.task?.id || '').startsWith('local-')) {
      // 【逐行批注】 声明前端变量 retry，保存本行计算出的数据或对象。
      const retry = localFailureRetry; localFailureRetry = null;
      // 【逐行批注】 判断条件 (retry) return retry();，成立时执行对应逻辑。
      if (retry) return retry();
      // 【逐行批注】 计算右侧结果并更新 status.textContent，改变页面或请求状态。
      status.textContent = '请求没有提交成功，请重新发起备课。';
      // 【逐行批注】 结束当前函数并返回结果：return;。
      return;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 判断条件 (!current?.id) return;，成立时执行对应逻辑。
    if (!current?.id) return;
    // 【逐行批注】 调用 follow 执行页面交互或数据处理：follow(await api.control({ command: 'retry', taskId: current.id }));。
    follow(await api.control({ command: 'retry', taskId: current.id }));
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 fieldNode，保存本行计算出的数据或对象。
  const fieldNode = event => {
    // 【逐行批注】 声明前端变量 group，保存本行计算出的数据或对象。
    const group = `${current.id}:${event.step}:${event.actionIndex}`, key = `${group}:${event.field}:${event.optionIndex}`;
    // 【逐行批注】 判断条件 (!cards.has(group)) {，成立时执行对应逻辑。
    if (!cards.has(group)) {
      // 【逐行批注】 声明前端变量 card，保存本行计算出的数据或对象。
      const card = document.createElement('article'); card.className = 'agent-preview';
      // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
      const label = document.createElement('p'); label.className = 'prepare-preview-label'; label.textContent = 'AI 实时生成'; card.append(label); root.append(card); cards.set(group, card);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 判断条件 (!streams.has(key)) {，成立时执行对应逻辑。
    if (!streams.has(key)) {
      // 【逐行批注】 声明前端变量 isOption，保存本行计算出的数据或对象。
      const isOption = event.field === 'option' || event.field === 'options';
      // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
      const node = document.createElement(event.field === 'title' ? 'h2' : isOption ? 'li' : 'div');
      // 【逐行批注】 声明前端变量 card，保存本行计算出的数据或对象。
      const card = cards.get(group);
      // 【逐行批注】 判断条件 (isOption) { let options = card.querySelector('ol'); if (!option…，成立时执行对应逻辑。
      if (isOption) { let options = card.querySelector('ol'); if (!options) { options = document.createElement('ol'); options.className = 'prepare-preview-options'; card.append(options); } options.append(node); }
      // 【逐行批注】 执行前一条件不成立时的分支。
      else card.append(node);
      // 【逐行批注】 执行当前前端表达式：streams.set(key, node);。
      streams.set(key, node);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return streams.get(key);。
    return streams.get(key);
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 定义函数 follow，封装可复用的前端行为。
  function follow(task) {
    // 【逐行批注】 执行当前前端表达式：close?.(); if (current?.id !== task.id) sequence = 0; current = task;。
    close?.(); if (current?.id !== task.id) sequence = 0; current = task;
    // 【逐行批注】 执行当前前端表达式：window.GangyiNavigation?.setContext({taskId: task.id});。
    window.GangyiNavigation?.setContext({taskId: task.id});
    // 【逐行批注】 计算右侧结果并更新 close，改变页面或请求状态。
    close = api.watch(task.id, {
      // 【逐行批注】 执行当前前端表达式：afterSeq: sequence,。
      afterSeq: sequence,
      // 【逐行批注】 计算右侧结果并更新 onEvent(event) { sequence，改变页面或请求状态。
      onEvent(event) { sequence = event.seq; trace?.event(event); if (event.type === 'delta') { empty.hidden = true; fieldNode(event).textContent += event.text; } return false; },
      // 【逐行批注】 调用 onState 执行页面交互或数据处理：async onState(state) {。
      async onState(state) {
        // 【逐行批注】 计算右侧结果并更新 localFailureRetry，改变页面或请求状态。
        localFailureRetry = null; current = state; stop.hidden = !['pending', 'running'].includes(state.status);
        // 【逐行批注】 计算右侧结果并更新 feedback.hidden，改变页面或请求状态。
        feedback.hidden = state.status !== 'waiting_student' || Boolean(state.lesson);
        // 【逐行批注】 计算右侧结果并更新 retry.hidden，改变页面或请求状态。
        retry.hidden = !['failed', 'paused', 'superseded'].includes(state.status);
        // 【逐行批注】 计算右侧结果并更新 status.textContent，改变页面或请求状态。
        status.textContent = state.status === 'failed' ? api.failureMessage(state) : ({ pending: '等待 AI 读取最新学习记录', running: '真实 AI 正在逐段准备课程', ready: '完整课件已保存',
          // 【逐行批注】 执行当前前端表达式：paused: '备课已暂停，可从有效步骤继续', superseded: '学习情况已有变化，旧输出没有应用',。
          paused: '备课已暂停，可从有效步骤继续', superseded: '学习情况已有变化，旧输出没有应用',
          // 【逐行批注】 执行当前前端表达式：waiting_student: 'AI 需要你补充信息后再继续备课' })[state.status] || state.status;。
          waiting_student: 'AI 需要你补充信息后再继续备课' })[state.status] || state.status;
        // 【逐行批注】 计算右侧结果并更新 badge.textContent，改变页面或请求状态。
        badge.textContent = ({pending:'已提交',running:'正在备课',ready:'已保存',failed:'需要处理',paused:'已暂停',superseded:'记录已更新',waiting_student:'等待补充'})[state.status] || '任务已保存';
        // 【逐行批注】 计算右侧结果并更新 submitted.className，改变页面或请求状态。
        submitted.className = 'is-complete'; generating.className = state.status === 'ready' && state.lesson ? 'is-complete' : state.status === 'failed' ? 'is-interrupted' : ['paused','waiting_student','superseded'].includes(state.status) ? 'is-paused' : state.calls > 0 ? 'is-active' : '';
        // 【逐行批注】 计算右侧结果并更新 saved.className，改变页面或请求状态。
        saved.className = state.status === 'ready' && state.lesson ? 'is-complete' : '';
        // 【逐行批注】 计算右侧结果并更新 document.getElementById('prepare-preview-sta…，改变页面或请求状态。
        document.getElementById('prepare-preview-status').textContent = ['pending','running'].includes(state.status) ? '生成中内容' : cards.size ? '已保留的内容' : '暂无课件预览';
        // 【逐行批注】 计算右侧结果并更新 empty.textContent，改变页面或请求状态。
        empty.textContent = ({failed:'本次尚无有效课件，可查看左侧原因后重试。',paused:'任务已暂停，恢复后继续展示生成内容。',waiting_student:'等待你补充信息后继续备课。',superseded:'学习情况已变化，请重试并读取最新记录。'})[state.status] || 'AI 正在读取学习情况，生成的内容会逐段显示在这里。';
        // 【逐行批注】 计算右侧结果并更新 errorBox.hidden，改变页面或请求状态。
        errorBox.hidden = state.status !== 'failed'; errorBox.textContent = state.status === 'failed' ? api.failureMessage(state) : '';
        // 【逐行批注】 计算右侧结果并更新 retry.textContent，改变页面或请求状态。
        retry.textContent = state.status === 'paused' ? '恢复备课' : '重试备课';
        // 【逐行批注】 执行当前前端表达式：trace?.update(state);。
        trace?.update(state);
        // 【逐行批注】 判断条件 (state.lesson?.title) document.getElementById('prepare-course').…，成立时执行对应逻辑。
        if (state.lesson?.title) document.getElementById('prepare-course').textContent = state.lesson.title;
        // 【逐行批注】 执行当前前端表达式：window.GangyiNavigation?.setContext({taskId: state.id, ...(state.lesson?.courseId || cour…。
        window.GangyiNavigation?.setContext({taskId: state.id, ...(state.lesson?.courseId || courseId ? {courseId: state.lesson?.courseId || courseId} : {})});
        // 【逐行批注】 判断条件 (state.status === 'ready' && state.lesson && !entered) {，成立时执行对应逻辑。
        if (state.status === 'ready' && state.lesson && !entered) {
          // 【逐行批注】 计算右侧结果并更新 entered，改变页面或请求状态。
          entered = true; enter.hidden = false; enter.href = classroomHref(state.lesson.id);
          // 【逐行批注】 计算右侧结果并更新 enter.onclick，改变页面或请求状态。
          enter.onclick = async event => {
            // 【逐行批注】 执行当前前端表达式：event.preventDefault();。
            event.preventDefault();
            // 【逐行批注】 计算右侧结果并更新 try { const saved，改变页面或请求状态。
            try { const saved = await api.control({ command: 'enter_lesson', lessonId: state.lesson.id }); location.assign(classroomHref(saved.id)); }
            // 【逐行批注】 计算右侧结果并更新 catch (error) { status.textContent，改变页面或请求状态。
            catch (error) { status.textContent = error.message; retry.hidden = false; }
          // 【逐行批注】 执行当前前端表达式：};。
          };
          // 【逐行批注】 判断条件 (state.navigate) {，成立时执行对应逻辑。
          if (state.navigate) {
            // 【逐行批注】 计算右侧结果并更新 try { const saved，改变页面或请求状态。
            try { const saved = await api.control({ command: 'enter_lesson', lessonId: state.lesson.id }); location.assign(classroomHref(saved.id)); }
            // 【逐行批注】 计算右侧结果并更新 catch (error) { entered，改变页面或请求状态。
            catch (error) { entered = false; status.textContent = error.message; retry.hidden = false; }
          // 【逐行批注】 结束当前前端代码块。
          }
        // 【逐行批注】 结束当前前端代码块。
        }
      // 【逐行批注】 计算右侧结果并更新 }, onConnectionError(error) { status.textCon…，改变页面或请求状态。
      }, onConnectionError(error) { status.textContent = error; },
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 计算右侧结果并更新 stop.onclick，改变页面或请求状态。
  stop.onclick = async () => {
    // 【逐行批注】 判断条件 (!current?.id) return;，成立时执行对应逻辑。
    if (!current?.id) return;
    // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
    const label = stop.textContent; stop.disabled = true; stop.textContent = '正在暂停…';
    // 【逐行批注】 执行当前前端表达式：try { trace?.update(await api.control({ command: 'pause', taskId: current.id })); }。
    try { trace?.update(await api.control({ command: 'pause', taskId: current.id })); }
    // 【逐行批注】 计算右侧结果并更新 catch (error) { status.textContent，改变页面或请求状态。
    catch (error) { status.textContent = error.message; }
    // 【逐行批注】 计算右侧结果并更新 finally { stop.disabled，改变页面或请求状态。
    finally { stop.disabled = false; stop.textContent = label; }
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 计算右侧结果并更新 retry.onclick，改变页面或请求状态。
  retry.onclick = async () => {
    // 【逐行批注】 计算右侧结果并更新 retry.disabled，改变页面或请求状态。
    retry.disabled = true; retry.textContent = current?.status === 'paused' ? '正在恢复…' : '正在重试…';
    // 【逐行批注】 调用 follow、openTask 执行页面交互或数据处理：try { follow(current ? await api.control({ command: current.status ==…。
    try { follow(current ? await api.control({ command: current.status === 'paused' ? 'resume' : 'retry', taskId: current.id }) : await openTask()); }
    // 【逐行批注】 计算右侧结果并更新 catch (error) { showOpeningError(error); } f…，改变页面或请求状态。
    catch (error) { showOpeningError(error); } finally { retry.disabled = false; }
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 为页面事件绑定处理逻辑：feedback.addEventListener('submit', async event => {。
  feedback.addEventListener('submit', async event => {
    // 【逐行批注】 执行当前前端表达式：event.preventDefault(); if (!input.value.trim()) return;。
    event.preventDefault(); if (!input.value.trim()) return;
    // 【逐行批注】 声明前端变量 button，保存本行计算出的数据或对象。
    const button = feedback.querySelector('button'), label = button.textContent, startedAt = Date.now();
    // 点击后到服务端返回之间必须有可见文案，不能只把按钮灰掉。
    // 【逐行批注】 计算右侧结果并更新 button.disabled，改变页面或请求状态。
    button.disabled = true; button.textContent = '正在提交…';
    // 【逐行批注】 计算右侧结果并更新 feedback.setAttribute('aria-busy', 'true'); …，改变页面或请求状态。
    feedback.setAttribute('aria-busy', 'true'); status.textContent = '正在提交你的补充信息…'; errorBox.hidden = true;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 nextId，保存本行计算出的数据或对象。
      const nextId = api.id(), next = await api.submit({ type: 'prepare_next', ...(courseId ? { courseId } : {}),
        // 【逐行批注】 执行当前前端表达式：requestId: nextId, previousTaskId: current.id, navigate: true, text: input.value.trim(), …。
        requestId: nextId, previousTaskId: current.id, navigate: true, text: input.value.trim(), ...reviewContext,
        // 【逐行批注】 执行当前前端表达式：...(query.has('topicId') ? { topicId: query.get('topicId') } : {}) });。
        ...(query.has('topicId') ? { topicId: query.get('topicId') } : {}) });
      // 【逐行批注】 执行当前前端表达式：query.set('taskId', next.id); query.set('requestId', nextId); history.replaceState(null, …。
      query.set('taskId', next.id); query.set('requestId', nextId); history.replaceState(null, '', `?${query}`);
      // 【逐行批注】 计算右侧结果并更新 api.storage.remove(`prepare-draft:${requestI…，改变页面或请求状态。
      api.storage.remove(`prepare-draft:${requestId}`); requestId = nextId; input.value = ''; follow(next);
    // 【逐行批注】 执行当前前端表达式：} catch (error) {。
    } catch (error) {
      // 【逐行批注】 计算右侧结果并更新 status.textContent，改变页面或请求状态。
      status.textContent = error.message; errorBox.textContent = error.message; errorBox.hidden = false;
      // 【逐行批注】 计算右侧结果并更新 feedLocalFailure('提交备课补充信息', error.message, …，改变页面或请求状态。
      feedLocalFailure('提交备课补充信息', error.message, startedAt, () => feedback.requestSubmit());
    // 【逐行批注】 计算右侧结果并更新 } finally { button.disabled，改变页面或请求状态。
    } finally { button.disabled = false; button.textContent = label; feedback.removeAttribute('aria-busy'); }
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 声明前端变量 openTask，保存本行计算出的数据或对象。
  const openTask = () => taskId ? api.request(`/api/learning-agent?taskId=${encodeURIComponent(taskId)}`) :
    // 【逐行批注】 执行当前前端表达式：api.submit({ type: 'prepare_next', ...(courseId ? { courseId } : {}), requestId, navigate…。
    api.submit({ type: 'prepare_next', ...(courseId ? { courseId } : {}), requestId, navigate: true, ...reviewContext,
      // 【逐行批注】 执行当前前端表达式：...(query.has('topicId') ? { topicId: query.get('topicId') } : {}),。
      ...(query.has('topicId') ? { topicId: query.get('topicId') } : {}),
      // 【逐行批注】 调用 Number 执行页面交互或数据处理：...(query.has('phaseIndex') ? { phaseIndex: Number(query.get('phaseIn…。
      ...(query.has('phaseIndex') ? { phaseIndex: Number(query.get('phaseIndex')), topicIndex: Number(query.get('topicIndex') || 1) } : {}),
      // 【逐行批注】 执行当前前端表达式：text: '先分析全部活跃课程中的真实作答、提示和最新反馈，自主选择推进、巩固、复习或跨课程学习。准备完整新课，并流式展示公开教学内容；保存后进入同一份课堂。' });。
      text: '先分析全部活跃课程中的真实作答、提示和最新反馈，自主选择推进、巩固、复习或跨课程学习。准备完整新课，并流式展示公开教学内容；保存后进入同一份课堂。' });
  // 【逐行批注】 定义函数 showOpeningError，封装可复用的前端行为。
  function showOpeningError(error) {
    // 【逐行批注】 计算右侧结果并更新 status.textContent，改变页面或请求状态。
    status.textContent = error.message; errorBox.textContent = error.message; errorBox.hidden = false;
    // 【逐行批注】 计算右侧结果并更新 badge.textContent，改变页面或请求状态。
    badge.textContent = current ? '需要处理' : '请求未提交'; stop.hidden = true; retry.hidden = false;
    // 【逐行批注】 计算右侧结果并更新 feedLocalFailure(current ? '恢复本次备课' : '提交本次备…，改变页面或请求状态。
    feedLocalFailure(current ? '恢复本次备课' : '提交本次备课请求', error.message, openedAt, () => retry.click());
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 注册异步成功回调：openTask().then(follow).catch(showOpeningError);。
  openTask().then(follow).catch(showOpeningError);
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('gangyi:agent-control', event => {。
  document.addEventListener('gangyi:agent-control', event => {
    // 【逐行批注】 判断条件 (event.detail?.all && current && ['pause', 'resume'].includes(ev…，成立时执行对应逻辑。
    if (event.detail?.all && current && ['pause', 'resume'].includes(event.detail.command)) {
      // 【逐行批注】 计算右侧结果并更新 api.request('/api/learning-agent?taskId，改变页面或请求状态。
      api.request('/api/learning-agent?taskId=' + encodeURIComponent(current.id)).then(follow).catch(showOpeningError);
    // 【逐行批注】 调用 follow 执行页面交互或数据处理：} else if (['resume','retry'].includes(event.detail?.command) && even…。
    } else if (['resume','retry'].includes(event.detail?.command) && event.detail.task?.id === current?.id) follow(event.detail.task);
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 判断条件 (courseId) api.request('/api/courses/' + encodeURIComponent(cour…，成立时执行对应逻辑。
  if (courseId) api.request('/api/courses/' + encodeURIComponent(courseId)).then(data => { document.getElementById('prepare-course').textContent = data.course.title; }).catch(() => {});
  // 【逐行批注】 为页面事件绑定处理逻辑：window.addEventListener('pagehide', () => close?.());。
  window.addEventListener('pagehide', () => close?.());
// 【逐行批注】 执行当前前端表达式：})();。
})();

/* 共享 AI 执行轨迹。只展示脱敏后的阶段、动作和耗时，不读取隐藏推理或工具私有参数。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 判断条件 (window.GangyiTrace) return;，成立时执行对应逻辑。
  if (window.GangyiTrace) return;
  // 【逐行批注】 声明前端变量 softLimitMs，保存本行计算出的数据或对象。
  const softLimitMs = 90000;
  // 【逐行批注】 声明前端变量 hardLimit，保存本行计算出的数据或对象。
  const hardLimit = 240000;
  // 【逐行批注】 声明前端变量 labels，保存本行计算出的数据或对象。
  const labels = {
    // 【逐行批注】 执行当前前端表达式：pending: '任务已提交，等待开始',。
    pending: '任务已提交，等待开始',
    // 【逐行批注】 执行当前前端表达式：running: '正在执行',。
    running: '正在执行',
    // 【逐行批注】 执行当前前端表达式：paused: '已暂停',。
    paused: '已暂停',
    // 【逐行批注】 执行当前前端表达式：ready: '已完成',。
    ready: '已完成',
    // 【逐行批注】 执行当前前端表达式：waiting_student: '等待你的回应',。
    waiting_student: '等待你的回应',
    // 【逐行批注】 执行当前前端表达式：failed: '本次处理未完成',。
    failed: '本次处理未完成',
    // 【逐行批注】 执行当前前端表达式：cancelled: '已停止',。
    cancelled: '已停止',
    // 【逐行批注】 执行当前前端表达式：superseded: '已由更新的记录接替',。
    superseded: '已由更新的记录接替',
    // 【逐行批注】 执行当前前端表达式：interrupted: '软件退出，任务已中断',。
    interrupted: '软件退出，任务已中断',
    // 【逐行批注】 执行当前前端表达式：action: '已保存一步操作',。
    action: '已保存一步操作',
    // 【逐行批注】 执行当前前端表达式：activity: '阶段更新',。
    activity: '阶段更新',
    // 【逐行批注】 执行当前前端表达式：failed_event: '本次处理未完成',。
    failed_event: '本次处理未完成',
    // 【逐行批注】 执行当前前端表达式：retry_scheduled: '已安排自动恢复',。
    retry_scheduled: '已安排自动恢复',
    // 【逐行批注】 执行当前前端表达式：delta: '内容接收中',。
    delta: '内容接收中',
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
  const active = task => ['pending', 'running'].includes(task?.status);
  // 【逐行批注】 声明前端变量 time，保存本行计算出的数据或对象。
  const time = value => {
    // 【逐行批注】 声明前端变量 date，保存本行计算出的数据或对象。
    const date = new Date(value);
    // 【逐行批注】 结束当前函数并返回结果：return Number.isNaN(date.getTime()) ? '' : date.toLocaleTimeString('z…。
    return Number.isNaN(date.getTime()) ? '' : date.toLocaleTimeString('zh-CN', {hour:'2-digit',minute:'2-digit',second:'2-digit',hour12:false});
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 duration，保存本行计算出的数据或对象。
  const duration = ms => {
    // 【逐行批注】 声明前端变量 seconds，保存本行计算出的数据或对象。
    const seconds = Math.max(0, Math.floor(Number(ms || 0) / 1000));
    // 【逐行批注】 判断条件 (seconds < 60) return seconds + ' 秒';，成立时执行对应逻辑。
    if (seconds < 60) return seconds + ' 秒';
    // 【逐行批注】 声明前端变量 minutes，保存本行计算出的数据或对象。
    const minutes = Math.floor(seconds / 60), rest = seconds % 60;
    // 【逐行批注】 结束当前函数并返回结果：return minutes + ' 分' + (rest ? ' ' + rest + ' 秒' : '');。
    return minutes + ' 分' + (rest ? ' ' + rest + ' 秒' : '');
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 shortId，保存本行计算出的数据或对象。
  const shortId = value => {
    // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
    const text = String(value || '').replace(/[^0-9a-zA-Z_-]/g, '');
    // 【逐行批注】 判断条件 (!text) return '';，成立时执行对应逻辑。
    if (!text) return '';
    // 【逐行批注】 结束当前函数并返回结果：return text.length > 8 ? text.slice(-8) : text;。
    return text.length > 8 ? text.slice(-8) : text;
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
  const node = (tag, text = '', className = '') => {
    // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
    const value = document.createElement(tag);
    // 【逐行批注】 计算右侧结果并更新 value.textContent，改变页面或请求状态。
    value.textContent = String(text ?? '');
    // 【逐行批注】 计算右侧结果并更新 value.className，改变页面或请求状态。
    value.className = className;
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 变值才写，避免每秒重建文本节点导致读屏重复播报和键盘焦点丢失。
  // 【逐行批注】 声明前端变量 setText，保存本行计算出的数据或对象。
  const setText = (target, value) => {
    // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
    const text = String(value ?? '');
    // 【逐行批注】 判断条件 (target.__gyText !== text) { target.__gyText = text; target.text…，成立时执行对应逻辑。
    if (target.__gyText !== text) { target.__gyText = text; target.textContent = text; }
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 traceSequence，保存本行计算出的数据或对象。
  let traceSequence = 0;
  // 【逐行批注】 定义函数 create，封装可复用的前端行为。
  function create(host, options = {}) {
    // 【逐行批注】 判断条件 (!host) return null;，成立时执行对应逻辑。
    if (!host) return null;
    // 【逐行批注】 执行当前前端表达式：host.classList.add('gy-trace-host', 'gy-trace-' + (options.variant || 'inline'));。
    host.classList.add('gy-trace-host', 'gy-trace-' + (options.variant || 'inline'));
    // 【逐行批注】 计算右侧结果并更新 host.hidden，改变页面或请求状态。
    host.hidden = true;
    // 【逐行批注】 声明前端变量 root，保存本行计算出的数据或对象。
    const root = node('section', '', 'gy-agent-trace');
    // 【逐行批注】 计算右侧结果并更新 root.dataset.status，改变页面或请求状态。
    root.dataset.status = 'idle';
    // 【逐行批注】 执行当前前端表达式：root.setAttribute('role', 'region');。
    root.setAttribute('role', 'region');
    // 【逐行批注】 执行当前前端表达式：root.setAttribute('aria-label', options.label || 'AI 执行过程');。
    root.setAttribute('aria-label', options.label || 'AI 执行过程');
    // 【逐行批注】 声明前端变量 head，保存本行计算出的数据或对象。
    const head = node('div', '', 'gy-trace-head');
    // 【逐行批注】 声明前端变量 mark，保存本行计算出的数据或对象。
    const mark = node('span', '', 'gy-trace-mark');
    // 【逐行批注】 声明前端变量 wrap，保存本行计算出的数据或对象。
    const wrap = node('div', '', 'gy-trace-title-wrap');
    // 【逐行批注】 声明前端变量 eyebrow，保存本行计算出的数据或对象。
    const eyebrow = node('p', options.eyebrow || 'AI 执行过程', 'gy-trace-eyebrow');
    // 【逐行批注】 声明前端变量 title，保存本行计算出的数据或对象。
    const title = node('h3', '正在准备', 'gy-trace-title');
    // 【逐行批注】 声明前端变量 meta，保存本行计算出的数据或对象。
    const meta = node('p', '', 'gy-trace-meta');
    // 【逐行批注】 执行当前前端表达式：wrap.append(eyebrow, title, meta);。
    wrap.append(eyebrow, title, meta);
    // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
    const body = node('div', '', 'gy-trace-body');
    // 【逐行批注】 计算右侧结果并更新 body.id，改变页面或请求状态。
    body.id = 'gy-trace-body-' + (++traceSequence);
    // 【逐行批注】 声明前端变量 toggle，保存本行计算出的数据或对象。
    const toggle = node('button', '收起过程', 'gy-trace-toggle');
    // 【逐行批注】 计算右侧结果并更新 toggle.type，改变页面或请求状态。
    toggle.type = 'button';
    // 【逐行批注】 执行当前前端表达式：toggle.setAttribute('aria-controls', body.id);。
    toggle.setAttribute('aria-controls', body.id);
    // 【逐行批注】 执行当前前端表达式：head.append(mark, wrap, toggle);。
    head.append(mark, wrap, toggle);
    // 单一活区：只有阶段变化或失败原因才播报，计时和倒计时留在 meta。
    // 【逐行批注】 声明前端变量 notice，保存本行计算出的数据或对象。
    const notice = node('p', '', 'gy-trace-notice');
    // 【逐行批注】 执行当前前端表达式：notice.setAttribute('role', 'status');。
    notice.setAttribute('role', 'status');
    // 【逐行批注】 执行当前前端表达式：notice.setAttribute('aria-live', 'polite');。
    notice.setAttribute('aria-live', 'polite');
    // 【逐行批注】 执行当前前端表达式：notice.setAttribute('aria-atomic', 'true');。
    notice.setAttribute('aria-atomic', 'true');
    // 【逐行批注】 声明前端变量 steps，保存本行计算出的数据或对象。
    const steps = node('ol', '', 'gy-trace-steps');
    // 【逐行批注】 声明前端变量 details，保存本行计算出的数据或对象。
    const details = node('details', '', 'gy-trace-details');
    // 【逐行批注】 声明前端变量 detailsSummary，保存本行计算出的数据或对象。
    const detailsSummary = node('summary', '查看完整执行过程');
    // 【逐行批注】 声明前端变量 history，保存本行计算出的数据或对象。
    const history = node('ol', '', 'gy-trace-history');
    // 【逐行批注】 执行当前前端表达式：details.append(detailsSummary, history);。
    details.append(detailsSummary, history);
    // 【逐行批注】 声明前端变量 actions，保存本行计算出的数据或对象。
    const actions = node('div', '', 'gy-trace-actions');
    // 错误与等待说明始终可见；步骤、历史和控制按钮收进可折叠正文。
    // 【逐行批注】 执行当前前端表达式：body.append(steps, details, actions);。
    body.append(steps, details, actions);
    // 【逐行批注】 执行当前前端表达式：root.append(head, notice, body);。
    root.append(head, notice, body);
    // 【逐行批注】 执行当前前端表达式：host.replaceChildren(root);。
    host.replaceChildren(root);

    // 【逐行批注】 声明前端变量 task，保存本行计算出的数据或对象。
    let task = null;
    // 【逐行批注】 声明前端变量 taskKey，保存本行计算出的数据或对象。
    let taskKey = '';
    // 【逐行批注】 声明前端变量 eventMap，保存本行计算出的数据或对象。
    let eventMap = new Map();
    // 【逐行批注】 声明前端变量 timer，保存本行计算出的数据或对象。
    let timer = 0;
    // 【逐行批注】 声明前端变量 destroyed，保存本行计算出的数据或对象。
    let destroyed = false;
    // 【逐行批注】 声明前端变量 userToggled，保存本行计算出的数据或对象。
    let userToggled = false;
    // 【逐行批注】 声明前端变量 collapseTouched，保存本行计算出的数据或对象。
    let collapseTouched = false;
    // 【逐行批注】 声明前端变量 collapsed，保存本行计算出的数据或对象。
    let collapsed = options.defaultCollapsed === true;
    // 【逐行批注】 声明前端变量 lastSteps，保存本行计算出的数据或对象。
    let lastSteps = '', lastHistory = '', lastActions = '', lastStatus = '';
    // 【逐行批注】 声明前端变量 eventKey，保存本行计算出的数据或对象。
    const eventKey = item => Number.isInteger(item?.seq) ? 'seq:' + item.seq :
      // 【逐行批注】 执行当前前端表达式：[item?.type, item?.step, item?.field, item?.actionIndex].join(':');。
      [item?.type, item?.step, item?.field, item?.actionIndex].join(':');
    // 流式正文片段只用于服务端状态，不进入轨迹，避免历史被成百条 delta 灌满。
    // 【逐行批注】 声明前端变量 keepEvent，保存本行计算出的数据或对象。
    const keepEvent = item => item && typeof item === 'object' && item.type !== 'delta';
    // 【逐行批注】 声明前端变量 addEvents，保存本行计算出的数据或对象。
    const addEvents = values => {
      // 【逐行批注】 按循环规则遍历或重复处理：for (const item of Array.isArray(values) ? values : []) if (keepEvent(item…。
      for (const item of Array.isArray(values) ? values : []) if (keepEvent(item)) eventMap.set(eventKey(item), item);
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 声明前端变量 sortedEvents，保存本行计算出的数据或对象。
    const sortedEvents = () => [...eventMap.values()].sort((a, b) =>
      // 【逐行批注】 调用 Number、String 执行页面交互或数据处理：Number(a.seq || 0) - Number(b.seq || 0) || String(a.at || '').localeC…。
      Number(a.seq || 0) - Number(b.seq || 0) || String(a.at || '').localeCompare(String(b.at || '')));
    // 【逐行批注】 定义函数 renderCollapsed，封装可复用的前端行为。
    function renderCollapsed() {
      // 【逐行批注】 计算右侧结果并更新 root.dataset.collapsed，改变页面或请求状态。
      root.dataset.collapsed = String(collapsed);
      // 【逐行批注】 计算右侧结果并更新 body.hidden，改变页面或请求状态。
      body.hidden = collapsed;
      // 【逐行批注】 调用 String 执行页面交互或数据处理：toggle.setAttribute('aria-expanded', String(!collapsed));。
      toggle.setAttribute('aria-expanded', String(!collapsed));
      // 【逐行批注】 执行当前前端表达式：toggle.setAttribute('aria-label', collapsed ? '展开 AI 执行过程' : '收起 AI 执行过程');。
      toggle.setAttribute('aria-label', collapsed ? '展开 AI 执行过程' : '收起 AI 执行过程');
      // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(toggle, collapsed ? '展开过程' : '收起过程');。
      setText(toggle, collapsed ? '展开过程' : '收起过程');
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 toggle.onclick，改变页面或请求状态。
    toggle.onclick = () => {
      // 【逐行批注】 计算右侧结果并更新 collapseTouched，改变页面或请求状态。
      collapseTouched = true;
      // 【逐行批注】 计算右侧结果并更新 collapsed，改变页面或请求状态。
      collapsed = !collapsed;
      // 【逐行批注】 调用 renderCollapsed 执行页面交互或数据处理：renderCollapsed();。
      renderCollapsed();
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 定义函数 button，封装可复用的前端行为。
    function button(text, kind, callback) {
      // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
      const value = node('button', text, kind);
      // 【逐行批注】 计算右侧结果并更新 value.type，改变页面或请求状态。
      value.type = 'button';
      // 【逐行批注】 声明前端变量 busy，保存本行计算出的数据或对象。
      let busy = false;
      // 【逐行批注】 计算右侧结果并更新 value.onclick，改变页面或请求状态。
      value.onclick = async () => {
        // 【逐行批注】 判断条件 (busy || value.getAttribute('aria-disabled') === 'true') return;，成立时执行对应逻辑。
        if (busy || value.getAttribute('aria-disabled') === 'true') return;
        // 【逐行批注】 计算右侧结果并更新 busy，改变页面或请求状态。
        busy = true;
        // 原生 disabled 会让正在聚焦的按钮立刻失去键盘焦点；aria 状态保留焦点，
        // busy 标记同时阻止重复点击。
        // 【逐行批注】 执行当前前端表达式：value.setAttribute('aria-busy', 'true');。
        value.setAttribute('aria-busy', 'true');
        // 【逐行批注】 执行当前前端表达式：value.setAttribute('aria-disabled', 'true');。
        value.setAttribute('aria-disabled', 'true');
        // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(value, text + '…');。
        setText(value, text + '…');
        // 【逐行批注】 执行当前前端表达式：try { await callback?.(task); }。
        try { await callback?.(task); }
        // 【逐行批注】 执行当前前端表达式：finally {。
        finally {
          // 【逐行批注】 判断条件 (!destroyed) {，成立时执行对应逻辑。
          if (!destroyed) {
            // 【逐行批注】 计算右侧结果并更新 busy，改变页面或请求状态。
            busy = false;
            // 【逐行批注】 执行当前前端表达式：value.setAttribute('aria-busy', 'false');。
            value.setAttribute('aria-busy', 'false');
            // 【逐行批注】 执行当前前端表达式：value.setAttribute('aria-disabled', 'false');。
            value.setAttribute('aria-disabled', 'false');
            // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(value, text);。
            setText(value, text);
          // 【逐行批注】 结束当前前端代码块。
          }
        // 【逐行批注】 结束当前前端代码块。
        }
      // 【逐行批注】 执行当前前端表达式：};。
      };
      // 【逐行批注】 执行当前前端表达式：actions.append(value);。
      actions.append(value);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 actionSignature，封装可复用的前端行为。
    function actionSignature() {
      // 【逐行批注】 判断条件 (!task?.id) return '';，成立时执行对应逻辑。
      if (!task?.id) return '';
      // 【逐行批注】 结束当前函数并返回结果：return [task.id, task.status,。
      return [task.id, task.status,
        // 【逐行批注】 执行当前前端表达式：task.status === 'paused' && options.onResume ? 'resume' : '',。
        task.status === 'paused' && options.onResume ? 'resume' : '',
        // 【逐行批注】 执行当前前端表达式：['failed', 'cancelled', 'superseded'].includes(task.status) && options.onRetry ? 'retry' …。
        ['failed', 'cancelled', 'superseded'].includes(task.status) && options.onRetry ? 'retry' : '',
        // 【逐行批注】 调用 active 执行页面交互或数据处理：active(task) && options.onPause ? 'pause' : '',。
        active(task) && options.onPause ? 'pause' : '',
        // 【逐行批注】 调用 active 执行页面交互或数据处理：active(task) && options.onCancel ? 'cancel' : ''].join('|');。
        active(task) && options.onCancel ? 'cancel' : ''].join('|');
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 renderActions，封装可复用的前端行为。
    function renderActions() {
      // 【逐行批注】 声明前端变量 signature，保存本行计算出的数据或对象。
      const signature = actionSignature();
      // 【逐行批注】 判断条件 (signature === lastActions) return;，成立时执行对应逻辑。
      if (signature === lastActions) return;
      // 【逐行批注】 计算右侧结果并更新 lastActions，改变页面或请求状态。
      lastActions = signature;
      // 【逐行批注】 执行当前前端表达式：actions.replaceChildren();。
      actions.replaceChildren();
      // 【逐行批注】 判断条件 (!task?.id) { actions.hidden = true; return; }，成立时执行对应逻辑。
      if (!task?.id) { actions.hidden = true; return; }
      // 【逐行批注】 判断条件 (task.status === 'paused' && options.onResume) button('恢复任务', 'p…，成立时执行对应逻辑。
      if (task.status === 'paused' && options.onResume) button('恢复任务', 'primary', options.onResume);
      // 【逐行批注】 判断条件 (['failed', 'cancelled', 'superseded'].includes(task.status) && …，成立时执行对应逻辑。
      if (['failed', 'cancelled', 'superseded'].includes(task.status) && options.onRetry) button('从失败步骤重试', 'primary', options.onRetry);
      // 【逐行批注】 判断条件 (active(task) && options.onPause) button('暂停', '', options.onPau…，成立时执行对应逻辑。
      if (active(task) && options.onPause) button('暂停', '', options.onPause);
      // 【逐行批注】 判断条件 (active(task) && options.onCancel) button('停止任务', '', options.on…，成立时执行对应逻辑。
      if (active(task) && options.onCancel) button('停止任务', '', options.onCancel);
      // 【逐行批注】 计算右侧结果并更新 actions.hidden，改变页面或请求状态。
      actions.hidden = !actions.children.length;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 声明前端变量 elapsedOf，保存本行计算出的数据或对象。
    const elapsedOf = (value, now) => {
      // 【逐行批注】 判断条件 (!value?.createdAt) return 0;，成立时执行对应逻辑。
      if (!value?.createdAt) return 0;
      // 【逐行批注】 声明前端变量 start，保存本行计算出的数据或对象。
      const start = new Date(value.createdAt).getTime();
      // 【逐行批注】 判断条件 (Number.isNaN(start)) return 0;，成立时执行对应逻辑。
      if (Number.isNaN(start)) return 0;
      // 【逐行批注】 声明前端变量 end，保存本行计算出的数据或对象。
      const end = active(value) ? now : new Date(value.updatedAt || value.activity?.at || 0).getTime();
      // 【逐行批注】 结束当前函数并返回结果：return Math.max(0, (Number.isNaN(end) || !end ? now : end) - start);。
      return Math.max(0, (Number.isNaN(end) || !end ? now : end) - start);
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 定义函数 render，封装可复用的前端行为。
    function render() {
      // 【逐行批注】 判断条件 (!task || destroyed) return;，成立时执行对应逻辑。
      if (!task || destroyed) return;
      // 【逐行批注】 计算右侧结果并更新 host.hidden，改变页面或请求状态。
      host.hidden = false;
      // 【逐行批注】 声明前端变量 now，保存本行计算出的数据或对象。
      const now = Date.now();
      // 【逐行批注】 声明前端变量 status，保存本行计算出的数据或对象。
      const status = task.status || '';
      // 【逐行批注】 声明前端变量 activity，保存本行计算出的数据或对象。
      const activity = task.activity || {};
      // 【逐行批注】 计算右侧结果并更新 root.dataset.status，改变页面或请求状态。
      root.dataset.status = status || 'running';
      // 【逐行批注】 声明前端变量 currentTitle，保存本行计算出的数据或对象。
      const currentTitle = activity.title || labels[status] || '处理中';
      // 【逐行批注】 声明前端变量 elapsed，保存本行计算出的数据或对象。
      const elapsed = elapsedOf(task, now);
      // 【逐行批注】 声明前端变量 calls，保存本行计算出的数据或对象。
      const calls = Number(task.calls || 0);
      // 【逐行批注】 声明前端变量 stepEvents，保存本行计算出的数据或对象。
      const stepEvents = sortedEvents().filter(item => item.type === 'activity' || item.type === 'action');
      // 【逐行批注】 声明前端变量 completed，保存本行计算出的数据或对象。
      const completed = stepEvents.filter(item => item.type === 'action').length;
      // 【逐行批注】 声明前端变量 retryRemain，保存本行计算出的数据或对象。
      const retryRemain = task.retry?.delayMs
        // 【逐行批注】 调用 Number、Date 执行页面交互或数据处理：? Math.max(0, Math.ceil((Number(task.retry.delayMs) - Math.max(0, now…。
        ? Math.max(0, Math.ceil((Number(task.retry.delayMs) - Math.max(0, now - new Date(task.retry.scheduledAt || 0).getTime())) / 1000)) : 0;
      // 【逐行批注】 声明前端变量 taskNumber，保存本行计算出的数据或对象。
      const taskNumber = shortId(task.id);
      // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(title, currentTitle);。
      setText(title, currentTitle);
      // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(meta, [。
      setText(meta, [
        // 【逐行批注】 执行当前前端表达式：task.purpose || 'AI 任务',。
        task.purpose || 'AI 任务',
        // 【逐行批注】 执行当前前端表达式：taskNumber ? '任务 #' + taskNumber : '',。
        taskNumber ? '任务 #' + taskNumber : '',
        // 【逐行批注】 调用 duration 执行页面交互或数据处理：elapsed ? '已用 ' + duration(elapsed) : '',。
        elapsed ? '已用 ' + duration(elapsed) : '',
        // 【逐行批注】 执行当前前端表达式：completed ? '已保存 ' + completed + ' 步' : '',。
        completed ? '已保存 ' + completed + ' 步' : '',
        // 【逐行批注】 执行当前前端表达式：calls ? '第 ' + calls + ' 轮' : '',。
        calls ? '第 ' + calls + ' 轮' : '',
        // 【逐行批注】 调用 active、Number 执行页面交互或数据处理：task.retry?.delayMs && active(task) ? '自动恢复 ' + Math.max(1, Number(ta…。
        task.retry?.delayMs && active(task) ? '自动恢复 ' + Math.max(1, Number(task.retry.attempt || 1) - 1) + '/2，' + retryRemain + ' 秒后继续' : '',
      // 【逐行批注】 执行当前前端表达式：].filter(Boolean).join(' · '));。
      ].filter(Boolean).join(' · '));

      // 【逐行批注】 声明前端变量 stepsKey，保存本行计算出的数据或对象。
      const stepsKey = stepEvents.map(item => [item.seq, item.type, item.activity?.state, item.activity?.title, item.message].join('~')).join('|');
      // 【逐行批注】 判断条件 (stepsKey !== lastSteps) {，成立时执行对应逻辑。
      if (stepsKey !== lastSteps) {
        // 【逐行批注】 计算右侧结果并更新 lastSteps，改变页面或请求状态。
        lastSteps = stepsKey;
        // 【逐行批注】 声明前端变量 recent，保存本行计算出的数据或对象。
        const recent = stepEvents.slice(-5).reverse();
        // 【逐行批注】 计算右侧结果并更新 steps.replaceChildren(...recent.map(item，改变页面或请求状态。
        steps.replaceChildren(...recent.map(item => {
          // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
          const row = node('li', '', 'gy-trace-step');
          // 【逐行批注】 计算右侧结果并更新 row.dataset.state，改变页面或请求状态。
          row.dataset.state = item.type === 'action' ? 'completed' : item.activity?.state || '';
          // 【逐行批注】 声明前端变量 heading，保存本行计算出的数据或对象。
          const heading = node('strong', item.activity?.title || labels[item.type] || '执行操作');
          // 【逐行批注】 声明前端变量 at，保存本行计算出的数据或对象。
          const at = time(item.at);
          // 【逐行批注】 判断条件 (at) {，成立时执行对应逻辑。
          if (at) {
            // 【逐行批注】 声明前端变量 stamp，保存本行计算出的数据或对象。
            const stamp = node('time', at);
            // 【逐行批注】 计算右侧结果并更新 stamp.dateTime，改变页面或请求状态。
            stamp.dateTime = String(item.at || '');
            // 【逐行批注】 执行当前前端表达式：heading.append(stamp);。
            heading.append(stamp);
          // 【逐行批注】 结束当前前端代码块。
          }
          // 【逐行批注】 执行当前前端表达式：row.append(heading);。
          row.append(heading);
          // 【逐行批注】 声明前端变量 detail，保存本行计算出的数据或对象。
          const detail = item.activity?.detail || '';
          // 【逐行批注】 判断条件 (detail) row.append(node('p', detail));，成立时执行对应逻辑。
          if (detail) row.append(node('p', detail));
          // 【逐行批注】 结束当前函数并返回结果：return row;。
          return row;
        // 【逐行批注】 执行当前前端表达式：}));。
        }));
        // 【逐行批注】 计算右侧结果并更新 steps.hidden，改变页面或请求状态。
        steps.hidden = !recent.length;
      // 【逐行批注】 结束当前前端代码块。
      }

      // 【逐行批注】 声明前端变量 historyKey，保存本行计算出的数据或对象。
      const historyKey = sortedEvents().map(item => item.seq ?? eventKey(item)).join('|');
      // 【逐行批注】 判断条件 (historyKey !== lastHistory) {，成立时执行对应逻辑。
      if (historyKey !== lastHistory) {
        // 【逐行批注】 计算右侧结果并更新 lastHistory，改变页面或请求状态。
        lastHistory = historyKey;
        // 【逐行批注】 声明前端变量 historyEvents，保存本行计算出的数据或对象。
        const historyEvents = sortedEvents();
        // 【逐行批注】 计算右侧结果并更新 history.replaceChildren(...historyEvents.sli…，改变页面或请求状态。
        history.replaceChildren(...historyEvents.slice(-80).reverse().map(item => {
          // 【逐行批注】 声明前端变量 row，保存本行计算出的数据或对象。
          const row = node('li');
          // 【逐行批注】 声明前端变量 at，保存本行计算出的数据或对象。
          const at = time(item.at);
          // 【逐行批注】 判断条件 (at) {，成立时执行对应逻辑。
          if (at) {
            // 【逐行批注】 声明前端变量 stamp，保存本行计算出的数据或对象。
            const stamp = node('time', at);
            // 【逐行批注】 计算右侧结果并更新 stamp.dateTime，改变页面或请求状态。
            stamp.dateTime = String(item.at || '');
            // 【逐行批注】 执行当前前端表达式：row.append(stamp);。
            row.append(stamp);
          // 【逐行批注】 结束当前前端代码块。
          }
          // 只写结构化状态，不回放模型正文内容。
          // 【逐行批注】 执行当前前端表达式：row.append(document.createTextNode(item.activity?.title || labels[item.type] || '状态更新'));。
          row.append(document.createTextNode(item.activity?.title || labels[item.type] || '状态更新'));
          // 【逐行批注】 结束当前函数并返回结果：return row;。
          return row;
        // 【逐行批注】 执行当前前端表达式：}));。
        }));
        // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(detailsSummary, '查看完整执行过程' + (historyEvents.length ? '（' + hi…。
        setText(detailsSummary, '查看完整执行过程' + (historyEvents.length ? '（' + historyEvents.length + ' 条）' : ''));
      // 【逐行批注】 结束当前前端代码块。
      }

      // 【逐行批注】 声明前端变量 noticeText，保存本行计算出的数据或对象。
      let noticeText = '';
      // 【逐行批注】 声明前端变量 noticeKind，保存本行计算出的数据或对象。
      let noticeKind = '';
      // 【逐行批注】 判断条件 (task.failure?.message || task.error) {，成立时执行对应逻辑。
      if (task.failure?.message || task.error) {
        // 【逐行批注】 计算右侧结果并更新 noticeText，改变页面或请求状态。
        noticeText = (task.failure?.message || task.error) + (elapsed ? '（本次已用 ' + duration(elapsed) + '）' : '');
        // 【逐行批注】 计算右侧结果并更新 noticeKind，改变页面或请求状态。
        noticeKind = 'error';
      // 【逐行批注】 执行当前前端表达式：} else if (status === 'paused') {。
      } else if (status === 'paused') {
        // 【逐行批注】 计算右侧结果并更新 noticeText，改变页面或请求状态。
        noticeText = '输入和已经保存的内容仍然保留，恢复后从当前步骤继续。';
        // 【逐行批注】 计算右侧结果并更新 noticeKind，改变页面或请求状态。
        noticeKind = 'warning';
      // 【逐行批注】 调用 active 执行页面交互或数据处理：} else if (active(task) && task.retry?.delayMs) {。
      } else if (active(task) && task.retry?.delayMs) {
        // 【逐行批注】 声明前端变量 activityAt，保存本行计算出的数据或对象。
        const activityAt = new Date(activity.at || task.updatedAt || 0).getTime();
        // 【逐行批注】 声明前端变量 idle，保存本行计算出的数据或对象。
        const idle = activityAt ? now - activityAt : elapsed;
        // 【逐行批注】 计算右侧结果并更新 noticeText，改变页面或请求状态。
        noticeText = '本次请求未成功，已有步骤不会重复执行；正在自动恢复' +
          // 【逐行批注】 执行当前前端表达式：(idle >= softLimitMs ? '。已经超过 90 秒没有新的有效进展。' : '。');。
          (idle >= softLimitMs ? '。已经超过 90 秒没有新的有效进展。' : '。');
        // 【逐行批注】 计算右侧结果并更新 noticeKind，改变页面或请求状态。
        noticeKind = 'warning';
      // 【逐行批注】 调用 active 执行页面交互或数据处理：} else if (active(task)) {。
      } else if (active(task)) {
        // 【逐行批注】 声明前端变量 activityAt，保存本行计算出的数据或对象。
        const activityAt = new Date(activity.at || task.updatedAt || 0).getTime();
        // 【逐行批注】 声明前端变量 idle，保存本行计算出的数据或对象。
        const idle = activityAt ? now - activityAt : elapsed;
        // 【逐行批注】 判断条件 (idle >= softLimitMs) {，成立时执行对应逻辑。
        if (idle >= softLimitMs) {
          // 【逐行批注】 计算右侧结果并更新 noticeText，改变页面或请求状态。
          noticeText = '已经超过 90 秒没有新的有效进展。任务仍在受控等待，单次请求最多持续 ' +
            // 【逐行批注】 执行当前前端表达式：Math.round(hardLimit / 60000) + ' 分钟，也可以现在暂停或停止。';。
            Math.round(hardLimit / 60000) + ' 分钟，也可以现在暂停或停止。';
          // 【逐行批注】 计算右侧结果并更新 noticeKind，改变页面或请求状态。
          noticeKind = 'warning';
        // 【逐行批注】 执行当前前端表达式：} else if (status === 'pending') {。
        } else if (status === 'pending') {
          // 【逐行批注】 计算右侧结果并更新 noticeText，改变页面或请求状态。
          noticeText = '任务已进入后台队列；可以离开当前页面，在顶部“AI 状态”继续查看。';
        // 【逐行批注】 结束当前前端代码块。
        }
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 调用 setText 执行页面交互或数据处理：setText(notice, noticeText);。
      setText(notice, noticeText);
      // 【逐行批注】 计算右侧结果并更新 notice.dataset.kind，改变页面或请求状态。
      notice.dataset.kind = noticeKind;
      // 【逐行批注】 计算右侧结果并更新 notice.hidden，改变页面或请求状态。
      notice.hidden = !noticeText;

      // 【逐行批注】 判断条件 (taskKey !== String(task.id || '')) userToggled = false;，成立时执行对应逻辑。
      if (taskKey !== String(task.id || '')) userToggled = false;
      // 【逐行批注】 判断条件 (status !== lastStatus) {，成立时执行对应逻辑。
      if (status !== lastStatus) {
        // 【逐行批注】 计算右侧结果并更新 lastStatus，改变页面或请求状态。
        lastStatus = status;
        // 【逐行批注】 判断条件 (!collapseTouched) {，成立时执行对应逻辑。
        if (!collapseTouched) {
          // 【逐行批注】 判断条件 (['failed', 'paused'].includes(status)) collapsed = false;，成立时执行对应逻辑。
          if (['failed', 'paused'].includes(status)) collapsed = false;
          // 【逐行批注】 当前一条件不成立时继续判断：(active(task)) collapsed = options.defaultCollapsed === true;。
          else if (active(task)) collapsed = options.defaultCollapsed === true;
          // 【逐行批注】 当前一条件不成立时继续判断：(options.collapseWhenDone) collapsed = true;。
          else if (options.collapseWhenDone) collapsed = true;
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 判断条件 (!userToggled) {，成立时执行对应逻辑。
        if (!userToggled) {
          // 【逐行批注】 判断条件 (['failed', 'paused'].includes(status)) details.open = true;，成立时执行对应逻辑。
          if (['failed', 'paused'].includes(status)) details.open = true;
          // 【逐行批注】 当前一条件不成立时继续判断：(options.collapseWhenDone && !active(task)) details.open = false;。
          else if (options.collapseWhenDone && !active(task)) details.open = false;
        // 【逐行批注】 结束当前前端代码块。
        }
      // 【逐行批注】 执行当前前端表达式：} else if (!userToggled) {。
      } else if (!userToggled) {
        // 【逐行批注】 判断条件 (options.collapseWhenDone && !active(task) && !['failed', 'pause…，成立时执行对应逻辑。
        if (options.collapseWhenDone && !active(task) && !['failed', 'paused'].includes(status)) details.open = false;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 调用 renderActions 执行页面交互或数据处理：renderActions();。
      renderActions();
      // 【逐行批注】 调用 renderCollapsed 执行页面交互或数据处理：renderCollapsed();。
      renderCollapsed();
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 tick，封装可复用的前端行为。
    function tick() {
      // 【逐行批注】 判断条件 (destroyed) return;，成立时执行对应逻辑。
      if (destroyed) return;
      // 【逐行批注】 调用 render 执行页面交互或数据处理：render();。
      render();
      // 【逐行批注】 判断条件 (task && active(task)) timer = setTimeout(tick, 1000);，成立时执行对应逻辑。
      if (task && active(task)) timer = setTimeout(tick, 1000);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 update，封装可复用的前端行为。
    function update(value) {
      // 【逐行批注】 判断条件 (!value || typeof value !== 'object') return;，成立时执行对应逻辑。
      if (!value || typeof value !== 'object') return;
      // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
      const key = String(value.id || '');
      // 【逐行批注】 判断条件 (taskKey !== key) {，成立时执行对应逻辑。
      if (taskKey !== key) {
        // 【逐行批注】 计算右侧结果并更新 taskKey，改变页面或请求状态。
        taskKey = key;
        // 【逐行批注】 计算右侧结果并更新 eventMap，改变页面或请求状态。
        eventMap = new Map();
        // 【逐行批注】 计算右侧结果并更新 lastSteps，改变页面或请求状态。
        lastSteps = ''; lastHistory = ''; lastActions = ''; lastStatus = '';
        // 【逐行批注】 计算右侧结果并更新 userToggled，改变页面或请求状态。
        userToggled = false;
        // 【逐行批注】 计算右侧结果并更新 collapseTouched，改变页面或请求状态。
        collapseTouched = false;
        // 【逐行批注】 计算右侧结果并更新 collapsed，改变页面或请求状态。
        collapsed = options.defaultCollapsed === true;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 计算右侧结果并更新 task，改变页面或请求状态。
      task = value;
      // 【逐行批注】 调用 addEvents 执行页面交互或数据处理：addEvents(value.events);。
      addEvents(value.events);
      // 【逐行批注】 调用 clearTimeout 执行页面交互或数据处理：clearTimeout(timer);。
      clearTimeout(timer);
      // 【逐行批注】 调用 render 执行页面交互或数据处理：render();。
      render();
      // 【逐行批注】 判断条件 (active(task)) timer = setTimeout(tick, 1000);，成立时执行对应逻辑。
      if (active(task)) timer = setTimeout(tick, 1000);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 event，封装可复用的前端行为。
    function event(value) {
      // 【逐行批注】 判断条件 (!keepEvent(value)) return;，成立时执行对应逻辑。
      if (!keepEvent(value)) return;
      // 【逐行批注】 调用 eventKey 执行页面交互或数据处理：eventMap.set(eventKey(value), value);。
      eventMap.set(eventKey(value), value);
      // 【逐行批注】 判断条件 (task) render();，成立时执行对应逻辑。
      if (task) render();
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 destroy，封装可复用的前端行为。
    function destroy() {
      // 【逐行批注】 计算右侧结果并更新 destroyed，改变页面或请求状态。
      destroyed = true;
      // 【逐行批注】 调用 clearTimeout 执行页面交互或数据处理：clearTimeout(timer);。
      clearTimeout(timer);
      // 【逐行批注】 执行当前前端表达式：host.replaceChildren();。
      host.replaceChildren();
      // 【逐行批注】 计算右侧结果并更新 host.hidden，改变页面或请求状态。
      host.hidden = true;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 为页面事件绑定处理逻辑：details.addEventListener('toggle', () => { userToggled = true; });。
    details.addEventListener('toggle', () => { userToggled = true; });
    // 【逐行批注】 调用 renderCollapsed 执行页面交互或数据处理：renderCollapsed();。
    renderCollapsed();
    // 【逐行批注】 结束当前函数并返回结果：return {root, update, event, destroy, get task() { return task; }};。
    return {root, update, event, destroy, get task() { return task; }};
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 计算右侧结果并更新 window.GangyiTrace，改变页面或请求状态。
  window.GangyiTrace = {create, labels, hardLimit, softLimitMs};
// 【逐行批注】 执行当前前端表达式：})();。
})();

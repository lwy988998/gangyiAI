/* 首页与用户中心共用六维画像；读取与切换仅展示本机已保存的 AI 结果。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 声明前端变量 SUBJECTS，保存本行计算出的数据或对象。
  const SUBJECTS = ['语文', '数学', '英语', '物理', '化学', '生物', '政治', '历史', '地理'];
  // 【逐行批注】 声明前端变量 ABILITIES，保存本行计算出的数据或对象。
  const ABILITIES = ['知识记忆', '概念理解', '方法应用', '逻辑推理', '表达说明', '综合迁移'];
  // 【逐行批注】 声明前端变量 DEFINITIONS，保存本行计算出的数据或对象。
  const DEFINITIONS = ['事实与知识的回忆', '概念含义的解释', '已知方法的运用', '有依据的分析与推导', '解题过程与观点的说明', '新情境或跨知识点的运用'];
  // 【逐行批注】 声明前端变量 DEFAULTS，保存本行计算出的数据或对象。
  const DEFAULTS = {mode: 'subjects', subjects: SUBJECTS.slice(0, 6)};
  // 【逐行批注】 声明前端变量 NS，保存本行计算出的数据或对象。
  const NS = 'http://www.w3.org/2000/svg';
  // 【逐行批注】 声明前端变量 serial，保存本行计算出的数据或对象。
  let serial = 0;
  // 【逐行批注】 声明前端变量 element，保存本行计算出的数据或对象。
  const element = (tag, text = '', className = '') => {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = document.createElement(tag);
    // 【逐行批注】 计算右侧结果并更新 node.textContent，改变页面或请求状态。
    node.textContent = text;
    // 【逐行批注】 判断条件 (className) node.className = className;，成立时执行对应逻辑。
    if (className) node.className = className;
    // 【逐行批注】 结束当前函数并返回结果：return node;。
    return node;
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 svgElement，保存本行计算出的数据或对象。
  const svgElement = (tag, attributes) => {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = document.createElementNS(NS, tag);
    // 【逐行批注】 计算右侧结果并更新 Object.entries(attributes || {}).forEach(([k…，改变页面或请求状态。
    Object.entries(attributes || {}).forEach(([key, value]) => node.setAttribute(key, value));
    // 【逐行批注】 结束当前函数并返回结果：return node;。
    return node;
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 point，保存本行计算出的数据或对象。
  const point = (index, radius) => {
    // 【逐行批注】 声明前端变量 angle，保存本行计算出的数据或对象。
    const angle = -Math.PI / 2 + index * Math.PI / 3;
    // 【逐行批注】 结束当前函数并返回结果：return [220 + Math.cos(angle) * radius, 177 + Math.sin(angle) * radiu…。
    return [220 + Math.cos(angle) * radius, 177 + Math.sin(angle) * radius];
  // 【逐行批注】 执行当前前端表达式：};。
  };
  // 【逐行批注】 声明前端变量 clean，保存本行计算出的数据或对象。
  const clean = text => String(text || '').replace(/\b(?:q-[0-9a-f]{16}|[0-9a-f]{32})\b/gi, '')
    // 【逐行批注】 计算右侧结果并更新 .replace(/unknown，改变页面或请求状态。
    .replace(/unknown=true/g, '明确反馈暂时不会').replace(/topicStates/g, '已评估学习记录').replace(/\[\[\s*[" ,]*\]\]/g, '').trim();
  // 【逐行批注】 声明前端变量 localTime，保存本行计算出的数据或对象。
  const localTime = value => {
    // 【逐行批注】 声明前端变量 date，保存本行计算出的数据或对象。
    const date = new Date(value);
    // 【逐行批注】 结束当前函数并返回结果：return !value || Number.isNaN(date.getTime()) ? '尚未评估' : date.toLocal…。
    return !value || Number.isNaN(date.getTime()) ? '尚未评估' : date.toLocaleString('zh-CN', {hour12: false});
  // 【逐行批注】 执行当前前端表达式：};。
  };

  // 【逐行批注】 执行当前前端表达式：class Radar {。
  class Radar {
    // 【逐行批注】 调用 constructor 执行页面交互或数据处理：constructor(root) {。
    constructor(root) {
      // 【逐行批注】 计算右侧结果并更新 this.root，改变页面或请求状态。
      this.root = root;
      // 【逐行批注】 计算右侧结果并更新 this.preferences，改变页面或请求状态。
      this.preferences = {...DEFAULTS, subjects: [...DEFAULTS.subjects]};
      // 【逐行批注】 计算右侧结果并更新 this.active，改变页面或请求状态。
      this.active = 0;
      // 【逐行批注】 计算右侧结果并更新 this.busy，改变页面或请求状态。
      this.busy = false;
      // 【逐行批注】 计算右侧结果并更新 this.signature，改变页面或请求状态。
      this.signature = '';
      // 【逐行批注】 计算右侧结果并更新 this.preferencesEpoch，改变页面或请求状态。
      this.preferencesEpoch = 0;
      // 【逐行批注】 执行当前前端表达式：root.classList.add('profile-radar');。
      root.classList.add('profile-radar');
      // 【逐行批注】 执行当前前端表达式：root.replaceChildren();。
      root.replaceChildren();
      // 【逐行批注】 声明前端变量 header，保存本行计算出的数据或对象。
      const header = element('div', '', 'radar-header');
      // 【逐行批注】 声明前端变量 heading，保存本行计算出的数据或对象。
      const heading = element('div');
      // 【逐行批注】 调用 element 执行页面交互或数据处理：heading.append(element('span', '本机专属 · AI 评估', 'radar-eyebrow'), elem…。
      heading.append(element('span', '本机专属 · AI 评估', 'radar-eyebrow'), element('h3', '你的六维学习画像'));
      // 【逐行批注】 计算右侧结果并更新 this.refresh，改变页面或请求状态。
      this.refresh = element('button', '更新画像 ↗', 'radar-refresh');
      // 【逐行批注】 计算右侧结果并更新 this.refresh.type，改变页面或请求状态。
      this.refresh.type = 'button';
      // 【逐行批注】 为页面事件绑定处理逻辑：this.refresh.addEventListener('click', () => this.retry());。
      this.refresh.addEventListener('click', () => this.retry());
      // 【逐行批注】 执行当前前端表达式：header.append(heading, this.refresh);。
      header.append(heading, this.refresh);
      // 【逐行批注】 声明前端变量 toolbar，保存本行计算出的数据或对象。
      const toolbar = element('div', '', 'radar-toolbar');
      // 【逐行批注】 声明前端变量 modes，保存本行计算出的数据或对象。
      const modes = element('div', '', 'radar-modes');
      // 【逐行批注】 执行当前前端表达式：modes.setAttribute('role', 'group');。
      modes.setAttribute('role', 'group');
      // 【逐行批注】 执行当前前端表达式：modes.setAttribute('aria-label', '画像类型');。
      modes.setAttribute('aria-label', '画像类型');
      // 【逐行批注】 计算右侧结果并更新 this.modes，改变页面或请求状态。
      this.modes = ['subjects', 'abilities'].map((mode, index) => {
        // 【逐行批注】 声明前端变量 button，保存本行计算出的数据或对象。
        const button = element('button', ['学科画像', '学习能力'][index]);
        // 【逐行批注】 计算右侧结果并更新 button.type，改变页面或请求状态。
        button.type = 'button';
        // 【逐行批注】 计算右侧结果并更新 button.dataset.mode，改变页面或请求状态。
        button.dataset.mode = mode;
        // 【逐行批注】 为页面事件绑定处理逻辑：button.addEventListener('click', () => this.save({...this.preferences, mode}));。
        button.addEventListener('click', () => this.save({...this.preferences, mode}));
        // 【逐行批注】 执行当前前端表达式：modes.append(button);。
        modes.append(button);
        // 【逐行批注】 结束当前函数并返回结果：return button;。
        return button;
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 计算右侧结果并更新 this.select，改变页面或请求状态。
      this.select = element('button', '选择学科', 'radar-select');
      // 【逐行批注】 计算右侧结果并更新 this.select.type，改变页面或请求状态。
      this.select.type = 'button';
      // 【逐行批注】 为页面事件绑定处理逻辑：this.select.addEventListener('click', () => this.openSelection());。
      this.select.addEventListener('click', () => this.openSelection());
      // 【逐行批注】 执行当前前端表达式：toolbar.append(modes, this.select);。
      toolbar.append(modes, this.select);
      // 【逐行批注】 计算右侧结果并更新 this.selection，改变页面或请求状态。
      this.selection = element('form', '', 'radar-selection');
      // 【逐行批注】 计算右侧结果并更新 this.selection.hidden，改变页面或请求状态。
      this.selection.hidden = true;
      // 【逐行批注】 执行当前前端表达式：this.selection.setAttribute('aria-label', '选择六门学科');。
      this.selection.setAttribute('aria-label', '选择六门学科');
      // 【逐行批注】 为页面事件绑定处理逻辑：this.selection.addEventListener('submit', event => {。
      this.selection.addEventListener('submit', event => {
        // 【逐行批注】 执行当前前端表达式：event.preventDefault();。
        event.preventDefault();
        // 【逐行批注】 声明前端变量 subjects，保存本行计算出的数据或对象。
        const subjects = [...this.selection.querySelectorAll('input:checked')].map(input => input.value);
        // 【逐行批注】 判断条件 (subjects.length === 6) this.save({...this.preferences, subjects…，成立时执行对应逻辑。
        if (subjects.length === 6) this.save({...this.preferences, subjects}, true);
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 计算右侧结果并更新 this.chart，改变页面或请求状态。
      this.chart = element('div', '', 'radar-chart');
      // 【逐行批注】 计算右侧结果并更新 this.svg，改变页面或请求状态。
      this.svg = svgElement('svg', {viewBox: '0 0 440 354', 'aria-hidden': 'true', focusable: 'false'});
      // 【逐行批注】 声明前端变量 gradientId，保存本行计算出的数据或对象。
      const gradientId = `radar-fill-${++serial}`;
      // 【逐行批注】 声明前端变量 defs，保存本行计算出的数据或对象。
      const defs = svgElement('defs');
      // 【逐行批注】 声明前端变量 gradient，保存本行计算出的数据或对象。
      const gradient = svgElement('linearGradient', {id: gradientId, x1: '0%', y1: '0%', x2: '35%', y2: '100%'});
      // 【逐行批注】 调用 svgElement 执行页面交互或数据处理：gradient.append(svgElement('stop', {offset: '0%', 'stop-color': '#a4e…。
      gradient.append(svgElement('stop', {offset: '0%', 'stop-color': '#a4e5d2', 'stop-opacity': '.38'}),
        // 【逐行批注】 调用 svgElement 执行页面交互或数据处理：svgElement('stop', {offset: '100%', 'stop-color': '#63c7b5', 'stop-op…。
        svgElement('stop', {offset: '100%', 'stop-color': '#63c7b5', 'stop-opacity': '.05'}));
      // 【逐行批注】 执行当前前端表达式：defs.append(gradient);。
      defs.append(gradient);
      // 【逐行批注】 执行当前前端表达式：this.svg.append(defs);。
      this.svg.append(defs);
      // 【逐行批注】 按循环规则遍历或重复处理：for (const score of [20, 40, 60, 80, 100]) {。
      for (const score of [20, 40, 60, 80, 100]) {
        // 【逐行批注】 计算右侧结果并更新 this.svg.append(svgElement('polygon', {point…，改变页面或请求状态。
        this.svg.append(svgElement('polygon', {points: Array.from({length: 6}, (_, i) => point(i, 110 * score / 100).join(',')).join(' '), class: 'radar-grid'}));
        // 【逐行批注】 声明前端变量 tick，保存本行计算出的数据或对象。
        const tick = svgElement('text', {x: 226, y: 177 - 110 * score / 100 + 4, class: 'radar-tick'});
        // 【逐行批注】 计算右侧结果并更新 tick.textContent，改变页面或请求状态。
        tick.textContent = score;
        // 【逐行批注】 执行当前前端表达式：this.svg.append(tick);。
        this.svg.append(tick);
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 按循环规则遍历或重复处理：for (let i = 0; i < 6; i++) {。
      for (let i = 0; i < 6; i++) {
        // 【逐行批注】 计算右侧结果并更新 [x, y]，改变页面或请求状态。
        const [x, y] = point(i, 110);
        // 【逐行批注】 调用 svgElement 执行页面交互或数据处理：this.svg.append(svgElement('line', {x1: 220, y1: 177, x2: x, y2: y, c…。
        this.svg.append(svgElement('line', {x1: 220, y1: 177, x2: x, y2: y, class: 'radar-guide'}));
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 计算右侧结果并更新 this.shape，改变页面或请求状态。
      this.shape = svgElement('polygon', {class: 'radar-area', fill: `url(#${gradientId})`});
      // 【逐行批注】 计算右侧结果并更新 this.shape.hidden，改变页面或请求状态。
      this.shape.hidden = true;
      // 【逐行批注】 执行当前前端表达式：this.svg.append(this.shape);。
      this.svg.append(this.shape);
      // 【逐行批注】 计算右侧结果并更新 this.dataPoints，改变页面或请求状态。
      this.dataPoints = svgElement('g');
      // 【逐行批注】 执行当前前端表达式：this.svg.append(this.dataPoints);。
      this.svg.append(this.dataPoints);
      // 【逐行批注】 执行当前前端表达式：this.chart.append(this.svg);。
      this.chart.append(this.svg);
      // 【逐行批注】 计算右侧结果并更新 this.axes，改变页面或请求状态。
      this.axes = Array.from({length: 6}, (_, i) => {
        // 【逐行批注】 声明前端变量 button，保存本行计算出的数据或对象。
        const button = element('button', '', 'radar-axis');
        // 【逐行批注】 计算右侧结果并更新 button.type，改变页面或请求状态。
        button.type = 'button';
        // 【逐行批注】 计算右侧结果并更新 button.dataset.axis，改变页面或请求状态。
        button.dataset.axis = i;
        // 【逐行批注】 计算右侧结果并更新 [x, y]，改变页面或请求状态。
        const [x, y] = point(i, 155);
        // 【逐行批注】 计算右侧结果并更新 button.style.left，改变页面或请求状态。
        button.style.left = `${x / 440 * 100}%`;
        // 【逐行批注】 计算右侧结果并更新 button.style.top，改变页面或请求状态。
        button.style.top = `${y / 354 * 100}%`;
        // 【逐行批注】 调用 element 执行页面交互或数据处理：button.append(element('span', '', 'radar-axis-name'), element('strong…。
        button.append(element('span', '', 'radar-axis-name'), element('strong', '待评估'));
        // 【逐行批注】 声明前端变量 activate，保存本行计算出的数据或对象。
        const activate = () => { this.active = i; this.showDetail(); };
        // 【逐行批注】 为页面事件绑定处理逻辑：button.addEventListener('pointerenter', event => { if (event.pointerType !== 'touch'…。
        button.addEventListener('pointerenter', event => { if (event.pointerType !== 'touch') activate(); });
        // 【逐行批注】 为页面事件绑定处理逻辑：button.addEventListener('focus', activate);。
        button.addEventListener('focus', activate);
        // 【逐行批注】 为页面事件绑定处理逻辑：button.addEventListener('click', activate);。
        button.addEventListener('click', activate);
        // 【逐行批注】 执行当前前端表达式：this.chart.append(button);。
        this.chart.append(button);
        // 【逐行批注】 结束当前函数并返回结果：return button;。
        return button;
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 计算右侧结果并更新 this.notice，改变页面或请求状态。
      this.notice = element('p', '正在读取本机学习画像…', 'radar-notice');
      // 【逐行批注】 执行当前前端表达式：this.notice.setAttribute('role', 'status');。
      this.notice.setAttribute('role', 'status');
      // 【逐行批注】 计算右侧结果并更新 this.detail，改变页面或请求状态。
      this.detail = element('section', '', 'radar-detail');
      // 【逐行批注】 执行当前前端表达式：this.detail.setAttribute('aria-label', '当前维度评估详情');。
      this.detail.setAttribute('aria-label', '当前维度评估详情');
      // 【逐行批注】 计算右侧结果并更新 this.detailTitle，改变页面或请求状态。
      this.detailTitle = element('h4');
      // 【逐行批注】 计算右侧结果并更新 this.detailScope，改变页面或请求状态。
      this.detailScope = element('p', '', 'radar-detail-scope');
      // 【逐行批注】 计算右侧结果并更新 this.detailReason，改变页面或请求状态。
      this.detailReason = element('p', '', 'radar-detail-reason');
      // 【逐行批注】 计算右侧结果并更新 this.detailAdvice，改变页面或请求状态。
      this.detailAdvice = element('p', '', 'radar-detail-advice');
      // 【逐行批注】 计算右侧结果并更新 this.detailTime，改变页面或请求状态。
      this.detailTime = element('small');
      // 【逐行批注】 执行当前前端表达式：this.detail.append(this.detailTitle, this.detailScope, this.detailReason, this.detailAdvi…。
      this.detail.append(this.detailTitle, this.detailScope, this.detailReason, this.detailAdvice, this.detailTime);
      // 【逐行批注】 执行当前前端表达式：root.append(header, toolbar, this.selection, this.chart, this.notice, this.detail);。
      root.append(header, toolbar, this.selection, this.chart, this.notice, this.detail);
      // 【逐行批注】 执行当前前端表达式：this.update({subjects: [], abilities: []});。
      this.update({subjects: [], abilities: []});
      // 【逐行批注】 执行当前前端表达式：this.load();。
      this.load();
      // 【逐行批注】 计算右侧结果并更新 this.timer，改变页面或请求状态。
      this.timer = setInterval(() => { if (!document.hidden) this.load(); }, 5000);
      // 【逐行批注】 为页面事件绑定处理逻辑：addEventListener('focus', () => this.load());。
      addEventListener('focus', () => this.load());
      // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('visibilitychange', () => { if (!document.hidden) this.loa…。
      document.addEventListener('visibilitychange', () => { if (!document.hidden) this.load(); });
    // 【逐行批注】 结束当前前端代码块。
    }

    // 【逐行批注】 调用 load 执行页面交互或数据处理：async load() {。
    async load() {
      // 【逐行批注】 判断条件 (this.loading) return;，成立时执行对应逻辑。
      if (this.loading) return;
      // 【逐行批注】 计算右侧结果并更新 this.loading，改变页面或请求状态。
      this.loading = true;
      // 【逐行批注】 声明前端变量 epoch，保存本行计算出的数据或对象。
      const epoch = this.preferencesEpoch;
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
        const response = await fetch('/api/profile', {cache: 'no-store'});
        // 【逐行批注】 判断条件 (!response.ok) throw new Error();，成立时执行对应逻辑。
        if (!response.ok) throw new Error();
        // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
        const data = await response.json();
        // 【逐行批注】 判断条件 (epoch === this.preferencesEpoch) this.update(data);，成立时执行对应逻辑。
        if (epoch === this.preferencesEpoch) this.update(data);
      // 【逐行批注】 计算右侧结果并更新 } catch (_) { this.notice.textContent，改变页面或请求状态。
      } catch (_) { this.notice.textContent = '画像暂时无法读取，保留当前显示结果。'; }
      // 【逐行批注】 计算右侧结果并更新 finally { this.loading，改变页面或请求状态。
      finally { this.loading = false; }
    // 【逐行批注】 结束当前前端代码块。
    }

    // 【逐行批注】 调用 update 执行页面交互或数据处理：update(data) {。
    update(data) {
      // 【逐行批注】 计算右侧结果并更新 this.data，改变页面或请求状态。
      this.data = data;
      // 【逐行批注】 判断条件 (!this.busy && data.radarPreferences) this.preferences = data.ra…，成立时执行对应逻辑。
      if (!this.busy && data.radarPreferences) this.preferences = data.radarPreferences;
      // 【逐行批注】 声明前端变量 abilities，保存本行计算出的数据或对象。
      const abilities = this.preferences.mode === 'abilities';
      // 【逐行批注】 声明前端变量 source，保存本行计算出的数据或对象。
      const source = abilities ? (data.abilities || []) : (data.subjects || []);
      // 【逐行批注】 声明前端变量 names，保存本行计算出的数据或对象。
      const names = abilities ? ABILITIES : this.preferences.subjects;
      // 【逐行批注】 计算右侧结果并更新 this.dimensions，改变页面或请求状态。
      this.dimensions = names.map((name, index) => {
        // 【逐行批注】 声明前端变量 found，保存本行计算出的数据或对象。
        const found = source.find(item => (abilities ? item.name : item.subject) === name) || {};
        // 【逐行批注】 结束当前函数并返回结果：return {...found, name, definition: abilities ? DEFINITIONS[index] : …。
        return {...found, name, definition: abilities ? DEFINITIONS[index] : '',
          // 【逐行批注】 执行当前前端表达式：score: Number.isFinite(found.score) && found.score >= 0 && found.score <= 100 ? found.sco…。
          score: Number.isFinite(found.score) && found.score >= 0 && found.score <= 100 ? found.score : null};
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 计算右侧结果并更新 this.modes.forEach(button，改变页面或请求状态。
      this.modes.forEach(button => {
        // 【逐行批注】 声明前端变量 selected，保存本行计算出的数据或对象。
        const selected = button.dataset.mode === this.preferences.mode;
        // 【逐行批注】 调用 String 执行页面交互或数据处理：button.setAttribute('aria-pressed', String(selected));。
        button.setAttribute('aria-pressed', String(selected));
        // 【逐行批注】 执行当前前端表达式：button.classList.toggle('is-active', selected);。
        button.classList.toggle('is-active', selected);
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 计算右侧结果并更新 this.select.hidden，改变页面或请求状态。
      this.select.hidden = abilities;
      // 【逐行批注】 声明前端变量 valid，保存本行计算出的数据或对象。
      const valid = this.dimensions.filter(item => item.score !== null).length;
      // 【逐行批注】 声明前端变量 state，保存本行计算出的数据或对象。
      const state = abilities ? (data.abilityStatus || {}) : data;
      // 【逐行批注】 计算右侧结果并更新 this.notice.textContent，改变页面或请求状态。
      this.notice.textContent = state.updating ? 'AI 正在处理最新学习记录 · 当前结果仍可查看。' : state.error ?
        // 【逐行批注】 执行当前前端表达式：'等待 AI 更新 · 当前保留上次真实评估。' : valid ? `已评估 ${valid} / 6 维 · 评分仅覆盖已测内容` :。
        '等待 AI 更新 · 当前保留上次真实评估。' : valid ? `已评估 ${valid} / 6 维 · 评分仅覆盖已测内容` :
          // 【逐行批注】 执行当前前端表达式：'待评估 · AI 会根据真实作答判断证据是否充分';。
          '待评估 · AI 会根据真实作答判断证据是否充分';
      // 【逐行批注】 声明前端变量 signature，保存本行计算出的数据或对象。
      const signature = JSON.stringify([this.preferences, this.dimensions.map(item => [item.name, item.score])]);
      // 【逐行批注】 判断条件 (signature !== this.signature) {，成立时执行对应逻辑。
      if (signature !== this.signature) {
        // 【逐行批注】 计算右侧结果并更新 this.signature，改变页面或请求状态。
        this.signature = signature;
        // 【逐行批注】 计算右侧结果并更新 this.axes.forEach((button, i)，改变页面或请求状态。
        this.axes.forEach((button, i) => {
          // 【逐行批注】 声明前端变量 item，保存本行计算出的数据或对象。
          const item = this.dimensions[i];
          // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
          const label = item.score === null ? '待评估' : item.score === 0 ? '入门起点' : `${item.score} 分`;
          // 【逐行批注】 计算右侧结果并更新 button.firstChild.textContent，改变页面或请求状态。
          button.firstChild.textContent = item.name;
          // 【逐行批注】 计算右侧结果并更新 button.lastChild.textContent，改变页面或请求状态。
          button.lastChild.textContent = label;
          // 【逐行批注】 执行当前前端表达式：button.setAttribute('aria-label', `${item.name}，${label}，查看依据和建议`);。
          button.setAttribute('aria-label', `${item.name}，${label}，查看依据和建议`);
          // 【逐行批注】 执行当前前端表达式：button.classList.toggle('is-pending', item.score === null);。
          button.classList.toggle('is-pending', item.score === null);
        // 【逐行批注】 执行当前前端表达式：});。
        });
        // 【逐行批注】 计算右侧结果并更新 this.shape.style.display，改变页面或请求状态。
        this.shape.style.display = valid === 6 ? '' : 'none';
        // 【逐行批注】 判断条件 (valid === 6) this.shape.setAttribute('points', this.dimensions.…，成立时执行对应逻辑。
        if (valid === 6) this.shape.setAttribute('points', this.dimensions.map((item, i) => point(i, 110 * item.score / 100).join(',')).join(' '));
        // 【逐行批注】 执行当前前端表达式：this.dataPoints.replaceChildren();。
        this.dataPoints.replaceChildren();
        // 【逐行批注】 计算右侧结果并更新 this.dimensions.forEach((item, i)，改变页面或请求状态。
        this.dimensions.forEach((item, i) => {
          // 【逐行批注】 判断条件 (item.score === null) return;，成立时执行对应逻辑。
          if (item.score === null) return;
          // 【逐行批注】 计算右侧结果并更新 [x, y]，改变页面或请求状态。
          const [x, y] = point(i, 110 * item.score / 100);
          // 【逐行批注】 调用 svgElement 执行页面交互或数据处理：this.dataPoints.append(svgElement('line', {x1: 220, y1: 177, x2: x, y…。
          this.dataPoints.append(svgElement('line', {x1: 220, y1: 177, x2: x, y2: y, class: 'radar-value-line'}),
            // 【逐行批注】 调用 svgElement 执行页面交互或数据处理：svgElement('circle', {cx: x, cy: y, r: 3.5, class: 'radar-value-point…。
            svgElement('circle', {cx: x, cy: y, r: 3.5, class: 'radar-value-point', 'data-score': item.score}));
        // 【逐行批注】 执行当前前端表达式：});。
        });
        // 【逐行批注】 执行当前前端表达式：this.svg.classList.remove('radar-changed');。
        this.svg.classList.remove('radar-changed');
        // 【逐行批注】 计算右侧结果并更新 requestAnimationFrame(()，改变页面或请求状态。
        requestAnimationFrame(() => this.svg.classList.add('radar-changed'));
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 执行当前前端表达式：this.showDetail();。
      this.showDetail();
    // 【逐行批注】 结束当前前端代码块。
    }

    // 【逐行批注】 调用 showDetail 执行页面交互或数据处理：showDetail() {。
    showDetail() {
      // 【逐行批注】 声明前端变量 item，保存本行计算出的数据或对象。
      const item = this.dimensions[this.active];
      // 【逐行批注】 判断条件 (!item) return;，成立时执行对应逻辑。
      if (!item) return;
      // 【逐行批注】 计算右侧结果并更新 this.axes.forEach((button, i)，改变页面或请求状态。
      this.axes.forEach((button, i) => button.classList.toggle('is-selected', i === this.active));
      // 【逐行批注】 声明前端变量 score，保存本行计算出的数据或对象。
      const score = item.score === null ? '待评估' : item.score === 0 ? '入门起点 · 0 分' : `${item.score} / 100`;
      // 【逐行批注】 计算右侧结果并更新 this.detailTitle.textContent，改变页面或请求状态。
      this.detailTitle.textContent = `${item.name} · ${score}`;
      // 【逐行批注】 计算右侧结果并更新 this.detailScope.textContent，改变页面或请求状态。
      this.detailScope.textContent = this.preferences.mode === 'abilities' ? `评估范围：全部课程的已测任务 · ${item.definition}` :
        // 【逐行批注】 执行当前前端表达式：'评估范围：本学科已测知识点，随更多真实作答更新';。
        '评估范围：本学科已测知识点，随更多真实作答更新';
      // 【逐行批注】 计算右侧结果并更新 this.detailReason.textContent，改变页面或请求状态。
      this.detailReason.textContent = item.rationale ? `依据：${clean(item.rationale)}` :
        // 【逐行批注】 执行当前前端表达式：item.score === null ? '尚无足够的对应题目证据，暂不推测你的水平。' : '依据：根据真实作答由 AI 评估。';。
        item.score === null ? '尚无足够的对应题目证据，暂不推测你的水平。' : '依据：根据真实作答由 AI 评估。';
      // 【逐行批注】 计算右侧结果并更新 this.detailAdvice.textContent，改变页面或请求状态。
      this.detailAdvice.textContent = item.recommendation ? `建议：${clean(item.recommendation)}` :
        // 【逐行批注】 执行当前前端表达式：'继续完成课程中的测验或课堂问答，积累真实反馈。';。
        '继续完成课程中的测验或课堂问答，积累真实反馈。';
      // 【逐行批注】 计算右侧结果并更新 this.detailTime.textContent，改变页面或请求状态。
      this.detailTime.textContent = item.score === null ? item.updatedAt ? `对应证据不足 · ${localTime(item.updatedAt)}` : '等待可靠证据与 AI 评估' :
        // 【逐行批注】 调用 localTime 执行页面交互或数据处理：`${item.evidenceCount || 0} 份有效反馈 · ${localTime(item.updatedAt)}`;。
        `${item.evidenceCount || 0} 份有效反馈 · ${localTime(item.updatedAt)}`;
    // 【逐行批注】 结束当前前端代码块。
    }

    // 【逐行批注】 调用 openSelection 执行页面交互或数据处理：openSelection() {。
    openSelection() {
      // 【逐行批注】 判断条件 (this.busy) return;，成立时执行对应逻辑。
      if (this.busy) return;
      // 【逐行批注】 执行当前前端表达式：this.selection.replaceChildren();。
      this.selection.replaceChildren();
      // 【逐行批注】 调用 element 执行页面交互或数据处理：this.selection.append(element('p', '选择 6 门学科；已有画像会保留，选择只改变展示。'));。
      this.selection.append(element('p', '选择 6 门学科；已有画像会保留，选择只改变展示。'));
      // 【逐行批注】 声明前端变量 grid，保存本行计算出的数据或对象。
      const grid = element('div', '', 'radar-subject-grid');
      // 【逐行批注】 计算右侧结果并更新 SUBJECTS.forEach(name，改变页面或请求状态。
      SUBJECTS.forEach(name => {
        // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
        const label = element('label');
        // 【逐行批注】 声明前端变量 input，保存本行计算出的数据或对象。
        const input = element('input');
        // 【逐行批注】 计算右侧结果并更新 input.type，改变页面或请求状态。
        input.type = 'checkbox'; input.value = name; input.checked = this.preferences.subjects.includes(name);
        // 【逐行批注】 调用 element 执行页面交互或数据处理：label.append(input, element('span', name));。
        label.append(input, element('span', name));
        // 【逐行批注】 执行当前前端表达式：grid.append(label);。
        grid.append(label);
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 声明前端变量 actions，保存本行计算出的数据或对象。
      const actions = element('div', '', 'radar-selection-actions');
      // 【逐行批注】 计算右侧结果并更新 this.counter，改变页面或请求状态。
      this.counter = element('span');
      // 【逐行批注】 计算右侧结果并更新 this.confirm，改变页面或请求状态。
      this.confirm = element('button', '保存选择');
      // 【逐行批注】 计算右侧结果并更新 this.confirm.type，改变页面或请求状态。
      this.confirm.type = 'submit';
      // 【逐行批注】 声明前端变量 cancel，保存本行计算出的数据或对象。
      const cancel = element('button', '取消');
      // 【逐行批注】 计算右侧结果并更新 cancel.type，改变页面或请求状态。
      cancel.type = 'button';
      // 【逐行批注】 为页面事件绑定处理逻辑：cancel.addEventListener('click', () => { this.selection.hidden = true; this.select.f…。
      cancel.addEventListener('click', () => { this.selection.hidden = true; this.select.focus(); });
      // 【逐行批注】 执行当前前端表达式：actions.append(this.counter, cancel, this.confirm);。
      actions.append(this.counter, cancel, this.confirm);
      // 【逐行批注】 执行当前前端表达式：this.selection.append(grid, actions);。
      this.selection.append(grid, actions);
      // 【逐行批注】 声明前端变量 count，保存本行计算出的数据或对象。
      const count = () => {
        // 【逐行批注】 声明前端变量 number，保存本行计算出的数据或对象。
        const number = this.selection.querySelectorAll('input:checked').length;
        // 【逐行批注】 计算右侧结果并更新 this.counter.textContent，改变页面或请求状态。
        this.counter.textContent = `已选 ${number} / 6`;
        // 【逐行批注】 计算右侧结果并更新 this.confirm.disabled，改变页面或请求状态。
        this.confirm.disabled = number !== 6;
      // 【逐行批注】 执行当前前端表达式：};。
      };
      // 【逐行批注】 计算右侧结果并更新 this.selection.onchange，改变页面或请求状态。
      this.selection.onchange = count;
      // 【逐行批注】 计算右侧结果并更新 count(); this.selection.hidden，改变页面或请求状态。
      count(); this.selection.hidden = false;
      // 【逐行批注】 执行当前前端表达式：grid.querySelector('input').focus();。
      grid.querySelector('input').focus();
    // 【逐行批注】 结束当前前端代码块。
    }

    // 【逐行批注】 计算右侧结果并更新 async save(preferences, closeSelection，改变页面或请求状态。
    async save(preferences, closeSelection = false) {
      // 【逐行批注】 判断条件 (this.busy) return;，成立时执行对应逻辑。
      if (this.busy) return;
      // 【逐行批注】 计算右侧结果并更新 this.busy，改变页面或请求状态。
      this.busy = true;
      // 【逐行批注】 执行当前前端表达式：this.preferencesEpoch++;。
      this.preferencesEpoch++;
      // 【逐行批注】 计算右侧结果并更新 this.modes.forEach(button，改变页面或请求状态。
      this.modes.forEach(button => button.disabled = true);
      // 【逐行批注】 计算右侧结果并更新 this.selection.querySelectorAll('button, inp…，改变页面或请求状态。
      this.selection.querySelectorAll('button, input').forEach(input => input.disabled = true);
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
        const response = await fetch('/api/profile/radar-preferences', {method: 'PUT', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(preferences)});
        // 【逐行批注】 判断条件 (!response.ok) throw new Error();，成立时执行对应逻辑。
        if (!response.ok) throw new Error();
        // 【逐行批注】 计算右侧结果并更新 this.preferences，改变页面或请求状态。
        this.preferences = await response.json();
        // 【逐行批注】 判断条件 (closeSelection) this.selection.hidden = true;，成立时执行对应逻辑。
        if (closeSelection) this.selection.hidden = true;
        // 【逐行批注】 执行当前前端表达式：this.update(this.data);。
        this.update(this.data);
      // 【逐行批注】 计算右侧结果并更新 } catch (_) { this.notice.textContent，改变页面或请求状态。
      } catch (_) { this.notice.textContent = '选择未能保存，请稍后重试。'; }
      // 【逐行批注】 执行当前前端表达式：finally {。
      finally {
        // 【逐行批注】 计算右侧结果并更新 this.busy，改变页面或请求状态。
        this.busy = false;
        // 【逐行批注】 计算右侧结果并更新 this.modes.forEach(button，改变页面或请求状态。
        this.modes.forEach(button => button.disabled = false);
        // 【逐行批注】 计算右侧结果并更新 this.selection.querySelectorAll('button, inp…，改变页面或请求状态。
        this.selection.querySelectorAll('button, input').forEach(input => input.disabled = false);
        // 【逐行批注】 判断条件 (closeSelection) this.select.focus();，成立时执行对应逻辑。
        if (closeSelection) this.select.focus();
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 结束当前前端代码块。
    }

    // 【逐行批注】 调用 retry 执行页面交互或数据处理：async retry() {。
    async retry() {
      // 【逐行批注】 计算右侧结果并更新 this.refresh.disabled，改变页面或请求状态。
      this.refresh.disabled = true;
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
        const response = await fetch('/api/profile/refresh', {method: 'POST'});
        // 【逐行批注】 判断条件 (!response.ok) throw new Error();，成立时执行对应逻辑。
        if (!response.ok) throw new Error();
        // 【逐行批注】 计算右侧结果并更新 this.notice.textContent，改变页面或请求状态。
        this.notice.textContent = '已请求 AI 更新，当前结果仍可查看。';
        // 【逐行批注】 计算右侧结果并更新 setTimeout(()，改变页面或请求状态。
        setTimeout(() => this.load(), 2500);
      // 【逐行批注】 计算右侧结果并更新 } catch (_) { this.notice.textContent，改变页面或请求状态。
      } catch (_) { this.notice.textContent = '更新请求未能提交，请稍后重试。'; }
      // 【逐行批注】 计算右侧结果并更新 finally { this.refresh.disabled，改变页面或请求状态。
      finally { this.refresh.disabled = false; }
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 计算右侧结果并更新 window.GangyiProfileRadar，改变页面或请求状态。
  window.GangyiProfileRadar = {mount: root => new Radar(root)};
  // 【逐行批注】 计算右侧结果并更新 document.querySelectorAll('[data-profile-rad…，改变页面或请求状态。
  document.querySelectorAll('[data-profile-radar]').forEach(root => new Radar(root));
// 【逐行批注】 执行当前前端表达式：})();。
})();

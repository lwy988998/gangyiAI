/* 首页与用户中心共用六维画像；读取与切换仅展示本机已保存的 AI 结果。 */
(() => {
  'use strict';
  const SUBJECTS = ['语文', '数学', '英语', '物理', '化学', '生物', '政治', '历史', '地理'];
  const ABILITIES = ['知识记忆', '概念理解', '方法应用', '逻辑推理', '表达说明', '综合迁移'];
  const DEFINITIONS = ['事实与知识的回忆', '概念含义的解释', '已知方法的运用', '有依据的分析与推导', '解题过程与观点的说明', '新情境或跨知识点的运用'];
  const DEFAULTS = {mode: 'subjects', subjects: SUBJECTS.slice(0, 6)};
  const NS = 'http://www.w3.org/2000/svg';
  let serial = 0;
  const element = (tag, text = '', className = '') => {
    const node = document.createElement(tag);
    node.textContent = text;
    if (className) node.className = className;
    return node;
  };
  const svgElement = (tag, attributes) => {
    const node = document.createElementNS(NS, tag);
    Object.entries(attributes || {}).forEach(([key, value]) => node.setAttribute(key, value));
    return node;
  };
  const point = (index, radius) => {
    const angle = -Math.PI / 2 + index * Math.PI / 3;
    return [220 + Math.cos(angle) * radius, 177 + Math.sin(angle) * radius];
  };
  const clean = text => String(text || '').replace(/\b(?:q-[0-9a-f]{16}|[0-9a-f]{32})\b/gi, '')
    .replace(/unknown=true/g, '明确反馈暂时不会').replace(/topicStates/g, '已评估学习记录').replace(/\[\[\s*[" ,]*\]\]/g, '').trim();
  const localTime = value => {
    const date = new Date(value);
    return !value || Number.isNaN(date.getTime()) ? '尚未评估' : date.toLocaleString('zh-CN', {hour12: false});
  };

  class Radar {
    constructor(root) {
      this.root = root;
      this.preferences = {...DEFAULTS, subjects: [...DEFAULTS.subjects]};
      this.active = 0;
      this.busy = false;
      this.signature = '';
      this.preferencesEpoch = 0;
      root.classList.add('profile-radar');
      root.replaceChildren();
      const header = element('div', '', 'radar-header');
      const heading = element('div');
      heading.append(element('span', '本机专属 · AI 评估', 'radar-eyebrow'), element('h3', '你的六维学习画像'));
      this.refresh = element('button', '更新画像 ↗', 'radar-refresh');
      this.refresh.type = 'button';
      this.refresh.addEventListener('click', () => this.retry());
      header.append(heading, this.refresh);
      const toolbar = element('div', '', 'radar-toolbar');
      const modes = element('div', '', 'radar-modes');
      modes.setAttribute('role', 'group');
      modes.setAttribute('aria-label', '画像类型');
      this.modes = ['subjects', 'abilities'].map((mode, index) => {
        const button = element('button', ['学科画像', '学习能力'][index]);
        button.type = 'button';
        button.dataset.mode = mode;
        button.addEventListener('click', () => this.save({...this.preferences, mode}));
        modes.append(button);
        return button;
      });
      this.select = element('button', '选择学科', 'radar-select');
      this.select.type = 'button';
      this.select.addEventListener('click', () => this.openSelection());
      toolbar.append(modes, this.select);
      this.selection = element('form', '', 'radar-selection');
      this.selection.hidden = true;
      this.selection.setAttribute('aria-label', '选择六门学科');
      this.selection.addEventListener('submit', event => {
        event.preventDefault();
        const subjects = [...this.selection.querySelectorAll('input:checked')].map(input => input.value);
        if (subjects.length === 6) this.save({...this.preferences, subjects}, true);
      });
      this.chart = element('div', '', 'radar-chart');
      this.svg = svgElement('svg', {viewBox: '0 0 440 354', 'aria-hidden': 'true', focusable: 'false'});
      const gradientId = `radar-fill-${++serial}`;
      const defs = svgElement('defs');
      const gradient = svgElement('linearGradient', {id: gradientId, x1: '0%', y1: '0%', x2: '35%', y2: '100%'});
      gradient.append(svgElement('stop', {offset: '0%', 'stop-color': '#a4e5d2', 'stop-opacity': '.38'}),
        svgElement('stop', {offset: '100%', 'stop-color': '#63c7b5', 'stop-opacity': '.05'}));
      defs.append(gradient);
      this.svg.append(defs);
      for (const score of [20, 40, 60, 80, 100]) {
        this.svg.append(svgElement('polygon', {points: Array.from({length: 6}, (_, i) => point(i, 110 * score / 100).join(',')).join(' '), class: 'radar-grid'}));
        const tick = svgElement('text', {x: 226, y: 177 - 110 * score / 100 + 4, class: 'radar-tick'});
        tick.textContent = score;
        this.svg.append(tick);
      }
      for (let i = 0; i < 6; i++) {
        const [x, y] = point(i, 110);
        this.svg.append(svgElement('line', {x1: 220, y1: 177, x2: x, y2: y, class: 'radar-guide'}));
      }
      this.shape = svgElement('polygon', {class: 'radar-area', fill: `url(#${gradientId})`});
      this.shape.hidden = true;
      this.svg.append(this.shape);
      this.dataPoints = svgElement('g');
      this.svg.append(this.dataPoints);
      this.chart.append(this.svg);
      this.axes = Array.from({length: 6}, (_, i) => {
        const button = element('button', '', 'radar-axis');
        button.type = 'button';
        button.dataset.axis = i;
        const [x, y] = point(i, 155);
        button.style.left = `${x / 440 * 100}%`;
        button.style.top = `${y / 354 * 100}%`;
        button.append(element('span', '', 'radar-axis-name'), element('strong', '待评估'));
        const activate = () => { this.active = i; this.showDetail(); };
        button.addEventListener('pointerenter', event => { if (event.pointerType !== 'touch') activate(); });
        button.addEventListener('focus', activate);
        button.addEventListener('click', activate);
        this.chart.append(button);
        return button;
      });
      this.notice = element('p', '正在读取本机学习画像…', 'radar-notice');
      this.notice.setAttribute('role', 'status');
      this.detail = element('section', '', 'radar-detail');
      this.detail.setAttribute('aria-label', '当前维度评估详情');
      this.detailTitle = element('h4');
      this.detailScope = element('p', '', 'radar-detail-scope');
      this.detailReason = element('p', '', 'radar-detail-reason');
      this.detailAdvice = element('p', '', 'radar-detail-advice');
      this.detailTime = element('small');
      this.detail.append(this.detailTitle, this.detailScope, this.detailReason, this.detailAdvice, this.detailTime);
      root.append(header, toolbar, this.selection, this.chart, this.notice, this.detail);
      this.update({subjects: [], abilities: []});
      this.load();
      this.timer = setInterval(() => { if (!document.hidden) this.load(); }, 5000);
      addEventListener('focus', () => this.load());
      document.addEventListener('visibilitychange', () => { if (!document.hidden) this.load(); });
    }

    async load() {
      if (this.loading) return;
      this.loading = true;
      const epoch = this.preferencesEpoch;
      try {
        const response = await fetch('/api/profile', {cache: 'no-store'});
        if (!response.ok) throw new Error();
        const data = await response.json();
        if (epoch === this.preferencesEpoch) this.update(data);
      } catch (_) { this.notice.textContent = '画像暂时无法读取，保留当前显示结果。'; }
      finally { this.loading = false; }
    }

    update(data) {
      this.data = data;
      if (!this.busy && data.radarPreferences) this.preferences = data.radarPreferences;
      const abilities = this.preferences.mode === 'abilities';
      const source = abilities ? (data.abilities || []) : (data.subjects || []);
      const names = abilities ? ABILITIES : this.preferences.subjects;
      this.dimensions = names.map((name, index) => {
        const found = source.find(item => (abilities ? item.name : item.subject) === name) || {};
        return {...found, name, definition: abilities ? DEFINITIONS[index] : '',
          score: Number.isFinite(found.score) && found.score >= 0 && found.score <= 100 ? found.score : null};
      });
      this.modes.forEach(button => {
        const selected = button.dataset.mode === this.preferences.mode;
        button.setAttribute('aria-pressed', String(selected));
        button.classList.toggle('is-active', selected);
      });
      this.select.hidden = abilities;
      const valid = this.dimensions.filter(item => item.score !== null).length;
      const state = abilities ? (data.abilityStatus || {}) : data;
      this.notice.textContent = state.updating ? 'AI 正在评估 · 当前结果仍可查看。' : state.error ?
        '等待 AI 更新 · 当前保留上次真实评估。' : valid ? `已评估 ${valid} / 6 维 · 评分仅覆盖已测内容` :
          '待评估 · 每个维度需要至少 3 道不同题目的可靠反馈';
      const signature = JSON.stringify([this.preferences, this.dimensions.map(item => [item.name, item.score])]);
      if (signature !== this.signature) {
        this.signature = signature;
        this.axes.forEach((button, i) => {
          const item = this.dimensions[i];
          const label = item.score === null ? '待评估' : item.score === 0 ? '入门起点' : `${item.score} 分`;
          button.firstChild.textContent = item.name;
          button.lastChild.textContent = label;
          button.setAttribute('aria-label', `${item.name}，${label}，查看依据和建议`);
          button.classList.toggle('is-pending', item.score === null);
        });
        this.shape.style.display = valid === 6 ? '' : 'none';
        if (valid === 6) this.shape.setAttribute('points', this.dimensions.map((item, i) => point(i, 110 * item.score / 100).join(',')).join(' '));
        this.dataPoints.replaceChildren();
        this.dimensions.forEach((item, i) => {
          if (item.score === null) return;
          const [x, y] = point(i, 110 * item.score / 100);
          this.dataPoints.append(svgElement('line', {x1: 220, y1: 177, x2: x, y2: y, class: 'radar-value-line'}),
            svgElement('circle', {cx: x, cy: y, r: 3.5, class: 'radar-value-point', 'data-score': item.score}));
        });
        this.svg.classList.remove('radar-changed');
        requestAnimationFrame(() => this.svg.classList.add('radar-changed'));
      }
      this.showDetail();
    }

    showDetail() {
      const item = this.dimensions[this.active];
      if (!item) return;
      this.axes.forEach((button, i) => button.classList.toggle('is-selected', i === this.active));
      const score = item.score === null ? '待评估' : item.score === 0 ? '入门起点 · 0 分' : `${item.score} / 100`;
      this.detailTitle.textContent = `${item.name} · ${score}`;
      this.detailScope.textContent = this.preferences.mode === 'abilities' ? `评估范围：全部课程的已测任务 · ${item.definition}` :
        '评估范围：本学科已测知识点，随更多真实作答更新';
      this.detailReason.textContent = item.rationale ? `依据：${clean(item.rationale)}` :
        item.score === null ? '尚无足够的对应题目证据，暂不推测你的水平。' : '依据：根据真实作答由 AI 评估。';
      this.detailAdvice.textContent = item.recommendation ? `建议：${clean(item.recommendation)}` :
        '继续完成课程中的测验或课堂问答，积累真实反馈。';
      this.detailTime.textContent = item.score === null ? item.updatedAt ? `对应证据不足 · ${localTime(item.updatedAt)}` : '等待可靠证据与 AI 评估' :
        `${item.evidenceCount || 0} 份有效反馈 · ${localTime(item.updatedAt)}`;
    }

    openSelection() {
      if (this.busy) return;
      this.selection.replaceChildren();
      this.selection.append(element('p', '选择 6 门学科；已有画像会保留，选择只改变展示。'));
      const grid = element('div', '', 'radar-subject-grid');
      SUBJECTS.forEach(name => {
        const label = element('label');
        const input = element('input');
        input.type = 'checkbox'; input.value = name; input.checked = this.preferences.subjects.includes(name);
        label.append(input, element('span', name));
        grid.append(label);
      });
      const actions = element('div', '', 'radar-selection-actions');
      this.counter = element('span');
      this.confirm = element('button', '保存选择');
      this.confirm.type = 'submit';
      const cancel = element('button', '取消');
      cancel.type = 'button';
      cancel.addEventListener('click', () => { this.selection.hidden = true; this.select.focus(); });
      actions.append(this.counter, cancel, this.confirm);
      this.selection.append(grid, actions);
      const count = () => {
        const number = this.selection.querySelectorAll('input:checked').length;
        this.counter.textContent = `已选 ${number} / 6`;
        this.confirm.disabled = number !== 6;
      };
      this.selection.onchange = count;
      count(); this.selection.hidden = false;
      grid.querySelector('input').focus();
    }

    async save(preferences, closeSelection = false) {
      if (this.busy) return;
      this.busy = true;
      this.preferencesEpoch++;
      this.modes.forEach(button => button.disabled = true);
      this.selection.querySelectorAll('button, input').forEach(input => input.disabled = true);
      try {
        const response = await fetch('/api/profile/radar-preferences', {method: 'PUT', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(preferences)});
        if (!response.ok) throw new Error();
        this.preferences = await response.json();
        if (closeSelection) this.selection.hidden = true;
        this.update(this.data);
      } catch (_) { this.notice.textContent = '选择未能保存，请稍后重试。'; }
      finally {
        this.busy = false;
        this.modes.forEach(button => button.disabled = false);
        this.selection.querySelectorAll('button, input').forEach(input => input.disabled = false);
        if (closeSelection) this.select.focus();
      }
    }

    async retry() {
      this.refresh.disabled = true;
      try {
        const response = await fetch('/api/profile/refresh', {method: 'POST'});
        if (!response.ok) throw new Error();
        this.notice.textContent = '已请求 AI 更新，当前结果仍可查看。';
        setTimeout(() => this.load(), 2500);
      } catch (_) { this.notice.textContent = '更新请求未能提交，请稍后重试。'; }
      finally { this.refresh.disabled = false; }
    }
  }
  document.querySelectorAll('[data-profile-radar]').forEach(root => new Radar(root));
})();

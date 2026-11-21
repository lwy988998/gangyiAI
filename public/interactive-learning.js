/* 受控教学组件：只渲染注册类型，探索不提交成绩。 */
(function (root, factory) {
  const api = factory(root);
  if (typeof module === 'object' && module.exports) module.exports = api;
  else root.GangyiInteractive = api;
})(typeof globalThis === 'object' ? globalThis : this, function (root) {
  'use strict';
  const labs = typeof module === 'object' && module.exports ? require('./interactive-labs.js') : root.GangyiInteractiveLabs;
  const setOperations = Object.freeze(['intersection', 'union', 'complement_a', 'complement_b', 'relations']);

  function members(values, universe) {
    // 与 C++ 的字节长度限制一致，中文和表情不能绕过参数上限。
    if (!Array.isArray(values) || values.length > 16) throw new Error('集合最多包含 16 个文字元素。');
    const seen = new Set();
    for (const value of values) {
      if (typeof value !== 'string' || !value.trim() || new TextEncoder().encode(value).length > 64 || seen.has(value) || (universe && !universe.includes(value)))
        throw new Error('集合中有重复、过长或不属于当前全集的元素。');
      seen.add(value);
    }
    return [...values];
  }

  function validate(config) {
    if (labs?.types.includes(config?.type)) return labs.validate(config);
    if (!config || config.type !== 'sets' || (config.schemaVersion !== undefined && config.schemaVersion !== 1))
      throw new Error('暂不支持这个互动，请先阅读讲解。');
    const universe = members(config.universe);
    if (!universe.length) throw new Error('这个互动还没有可探索的元素。');
    const operation = config.operation ?? 'intersection';
    if (!setOperations.includes(operation)) throw new Error('当前集合探索不支持这个操作。');
    return {type: 'sets', schemaVersion: 1, universe, operation, setA: members(config.setA, universe), setB: members(config.setB, universe)};
  }

  function checkedState(config, input, contentVersion) {
    if (labs?.types.includes(config?.type)) return labs.checkedState(config, input, contentVersion);
    // 只接收原始选择，交集由程序重算；旧课件的状态不得覆盖新配置。
    if (!input || input.schemaVersion !== 1 || !Number.isSafeInteger(input.contentVersion) || input.contentVersion !== contentVersion ||
        Object.keys(input).some(key => !['schemaVersion', 'contentVersion', 'setA', 'setB'].includes(key)))
      throw new Error('互动内容已更新，请从当前内容重新探索。');
    return {schemaVersion: 1, contentVersion, setA: members(input.setA, config.universe), setB: members(input.setB, config.universe)};
  }

  function intersection(config, state) {
    return config.universe.filter(value => state.setA.includes(value) && state.setB.includes(value));
  }

  function calculateSets(config, state) {
    const universe = members(config.universe), a = members(state.setA, universe), b = members(state.setB, universe);
    const common = intersection({universe}, {setA: a, setB: b});
    const aSubsetB = a.every(value => b.includes(value)), bSubsetA = b.every(value => a.includes(value));
    return {intersection: common, union: universe.filter(value => a.includes(value) || b.includes(value)),
      complementA: universe.filter(value => !a.includes(value)), complementB: universe.filter(value => !b.includes(value)),
      relations: {aSubsetB, bSubsetA, equal: aSubsetB && bSubsetA,
        aProperSubsetB: aSubsetB && !bSubsetA, bProperSubsetA: bSubsetA && !aSubsetB}};
  }

  function mountSets(host, rawConfig, options = {}) {
    const config = validate(rawConfig), contentVersion = options.contentVersion;
    if (!Number.isSafeInteger(contentVersion) || contentVersion < 1) throw new Error('互动版本无效，请重新打开当前课时。');
    let state = {schemaVersion: 1, contentVersion, setA: [...config.setA], setB: [...config.setB]};
    let restored = false, invalidRestore = false;
    if (options.state) {
      try { state = checkedState(config, options.state, contentVersion); restored = true; }
      catch (_) { invalidRestore = true; }
    }
    // 课件切换时集中解除事件，避免重复挂载后一次操作触发多次回传。
    const controller = new AbortController();
    const listen = (node, type, action) => node.addEventListener(type, action, {signal: controller.signal});
    const doc = host.ownerDocument;
    // 元素标签和学生可见文字始终使用 textContent，不执行模型提供的标记。
    const node = (tag, text, className) => {
      const value = doc.createElement(tag);
      if (text) value.textContent = text;
      if (className) value.className = className;
      return value;
    };
    const frame = node('div', '', 'il-sets');
    const instructions = {intersection: '勾选元素的归属，观察同时属于 A 和 B 的元素。',
      union: '勾选元素的归属，观察至少属于 A 或 B 之一的元素。',
      complement_a: '先确认全集 U，再观察其中不属于 A 的元素。',
      complement_b: '先确认全集 U，再观察其中不属于 B 的元素。',
      relations: '改变集合成员，比较子集、真子集与集合相等；空集也可以参与比较。'};
    frame.append(node('p', instructions[config.operation], 'il-instruction'));
    frame.append(node('p', `全集 U = { ${config.universe.join(', ')} }`, 'il-sets-universe'));
    const grid = node('div', '', 'il-sets-layout'), diagram = node('div', '', 'il-sets-diagram');
    const svg = doc.createElementNS('http://www.w3.org/2000/svg', 'svg');
    svg.setAttribute('viewBox', '0 0 320 165'); svg.setAttribute('aria-hidden', 'true');
    const circles = [];
    for (const [index, cx] of [119, 201].entries()) {
      const circle = doc.createElementNS('http://www.w3.org/2000/svg', 'circle');
      circle.setAttribute('cx', cx); circle.setAttribute('cy', '85'); circle.setAttribute('r', '69');
      circle.dataset.set = index === 0 ? 'A' : 'B'; circles.push(circle); svg.append(circle);
    }
    const labels = [];
    for (const [label, x] of [['A', 85], ['B', 235]]) {
      const text = doc.createElementNS('http://www.w3.org/2000/svg', 'text');
      text.setAttribute('x', x); text.setAttribute('y', '86'); text.setAttribute('text-anchor', 'middle');
      text.textContent = label; labels.push(text); svg.append(text);
    }
    const result = node('p', '', 'il-sets-result'); result.setAttribute('role', 'status'); result.setAttribute('aria-live', 'polite');
    const description = node('p', '', 'il-sets-description'); diagram.append(svg, result, description);
    const membership = node('div', '', 'il-sets-membership'); diagram.append(membership);
    const table = node('table', '', 'il-sets-table'); table.append(node('caption', '选择元素的归属'));
    const head = node('tr');
    for (const label of ['元素', '属于 A', '属于 B']) { const th = node('th', label); th.scope = 'col'; head.append(th); }
    const thead = node('thead'); thead.append(head); table.append(thead);
    const body = node('tbody'); table.append(body);
    const controls = [];
    for (const value of config.universe) {
      const row = node('tr'), heading = node('th', value); heading.scope = 'row'; row.append(heading);
      for (const key of ['setA', 'setB']) {
        const cell = node('td'), label = node('label'), input = node('input'); input.type = 'checkbox';
        input.setAttribute('aria-label', `${value} 属于集合 ${key === 'setA' ? 'A' : 'B'}`);
        input.dataset.set = key; input.dataset.element = value; label.append(input); cell.append(label); row.append(cell);
        controls.push({input, key, value});
        listen(input, 'change', () => {
          const selected = new Set(state[key]); input.checked ? selected.add(value) : selected.delete(value);
          state[key] = config.universe.filter(item => selected.has(item)); update(); options.onChange?.(getState());
        });
      }
      body.append(row);
    }
    const getState = () => checkedState(config, state, contentVersion);
    const update = () => {
      for (const {input, key, value} of controls) input.checked = state[key].includes(value);
      const calculated = calculateSets(config, state);
      const setCircle = (index, x, radius, visible = true) => {
        circles[index].setAttribute('cx', String(x)); circles[index].setAttribute('r', String(radius));
        circles[index].setAttribute('visibility', visible ? 'visible' : 'hidden');
      };
      const setLabel = (index, text, x, visible = true) => {
        labels[index].textContent = text; labels[index].setAttribute('x', String(x));
        labels[index].setAttribute('visibility', visible ? 'visible' : 'hidden');
      };
      if (!state.setA.length && !state.setB.length) {
        svg.dataset.relation = 'both-empty';
        setCircle(0, 160, 69, false); setCircle(1, 160, 69, false);
        setLabel(0, 'A = B = ∅', 160); setLabel(1, '', 160, false);
      } else if (!state.setA.length) {
        svg.dataset.relation = 'a-empty';
        setCircle(0, 70, 60, false); setCircle(1, 215, 60);
        setLabel(0, 'A = ∅', 70); setLabel(1, 'B', 215);
      } else if (!state.setB.length) {
        svg.dataset.relation = 'b-empty';
        setCircle(0, 105, 60); setCircle(1, 250, 60, false);
        setLabel(0, 'A', 105); setLabel(1, 'B = ∅', 250);
      } else if (calculated.relations.equal) {
        svg.dataset.relation = 'equal';
        setCircle(0, 160, 69); setCircle(1, 160, 69, false);
        setLabel(0, 'A = B', 160); setLabel(1, '', 160, false);
      } else if (calculated.relations.aProperSubsetB) {
        svg.dataset.relation = 'a-subset';
        setCircle(0, 145, 36); setCircle(1, 160, 69);
        setLabel(0, 'A', 145); setLabel(1, 'B', 205);
      } else if (calculated.relations.bProperSubsetA) {
        svg.dataset.relation = 'b-subset';
        setCircle(0, 160, 69); setCircle(1, 175, 36);
        setLabel(0, 'A', 115); setLabel(1, 'B', 175);
      } else if (!calculated.intersection.length) {
        svg.dataset.relation = 'disjoint';
        setCircle(0, 85, 58); setCircle(1, 235, 58);
        setLabel(0, 'A', 85); setLabel(1, 'B', 235);
      } else {
        svg.dataset.relation = 'overlap';
        setCircle(0, 119, 69); setCircle(1, 201, 69);
        setLabel(0, 'A', 85); setLabel(1, 'B', 235);
      }
      const format = values => values.length ? `{ ${values.join(', ')} }` : '∅';
      const expressions = {intersection: ['A ∩ B', calculated.intersection], union: ['A ∪ B', calculated.union],
        complement_a: ['U \\ A', calculated.complementA], complement_b: ['U \\ B', calculated.complementB]};
      if (config.operation === 'relations') {
        const relation = calculated.relations;
        result.textContent = relation.equal ? 'A = B' : relation.aProperSubsetB ? 'A ⊊ B' : relation.bProperSubsetA ? 'B ⊊ A' : 'A、B 互不包含';
        description.textContent = relation.equal ? '两个集合的元素完全相同；相等时互为子集，但不是真子集。' :
          relation.aProperSubsetB ? 'A 的每个元素都属于 B，并且 B 至少有一个元素不属于 A。' :
          relation.bProperSubsetA ? 'B 的每个元素都属于 A，并且 A 至少有一个元素不属于 B。' : '两个集合各有不属于对方的元素。';
      } else {
        const [label, values] = expressions[config.operation];
        result.textContent = `${label} = ${format(values)}`;
        description.textContent = config.operation === 'intersection' ? `${values.length} 个元素同时属于 A 和 B。` :
          config.operation === 'union' ? `${values.length} 个元素至少属于 A、B 中的一个；重复元素只计算一次。` :
            `补集相对于当前全集 U，共有 ${values.length} 个元素。`;
      }
      membership.replaceChildren(node('p', `A = ${format(state.setA)}`), node('p', `B = ${format(state.setB)}`),
        node('p', '图形示意集合关系；元素个数以选择结果为准。', 'il-sets-boundary'));
    };
    grid.append(diagram, table); frame.append(grid);
    const note = node('p', invalidRestore ? '内容已更新，上次探索无法恢复，已载入当前示例。' : restored ? '已恢复上次探索。' : '可以自由尝试；这里的操作不会计入成绩。', 'il-note');
    const actions = node('div', '', 'il-actions'), reset = node('button', '重置探索', 'dc-button'); reset.type = 'button';
    listen(reset, 'click', () => {
      state = {schemaVersion: 1, contentVersion, setA: [...config.setA], setB: [...config.setB]};
      update(); note.textContent = '已恢复本次示例，可以重新探索。'; options.onChange?.(getState(), {kind:'reset'});
    });
    actions.append(reset);
    if (options.onExplain) {
      const explain = node('button', '请老师解释当前结果', 'dc-button'); explain.type = 'button';
      listen(explain, 'click', async () => {
        explain.disabled = true;
        try { await options.onExplain(getState()); }
        finally { explain.disabled = false; }
      });
      actions.append(explain);
    }
    frame.append(note, actions); host.append(frame); update();
    return {getState, destroy() { controller.abort(); frame.remove(); }};
  }

    // 注册表只包含经审核的渲染器，模型不能注册或执行任意组件代码。
    const registry = new Map([['sets', mountSets]]);
    for (const type of labs?.types || []) registry.set(type, labs.mount);
  function mount(host, config, options) {
    const render = registry.get(config?.type);
    if (!render) throw new Error('暂不支持这个互动，请先阅读讲解。');
    return render(host, config, options);
  }
  return Object.freeze({mount, validate, checkedState, intersection, calculateSets, setOperations, types: Object.freeze([...registry.keys()])});
});

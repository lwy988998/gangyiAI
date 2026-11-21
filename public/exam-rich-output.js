/* 试卷讲解专用富内容增强：沿用共用 Markdown 渲染器，安全绘制结构化学习图示。 */
(() => {
  'use strict';

  const SVG_NS = 'http://www.w3.org/2000/svg';
  const MAX_SOURCE = 60000;
  const MAX_NODES = 120;
  const MAX_DEPTH = 8;
  const activeStates = new WeakMap();

  const escapeText = value => String(value ?? '');
  const finite = value => typeof value === 'number' && Number.isFinite(value);

  function parseJson(source) {
    if (source.length > MAX_SOURCE) throw new Error('图示内容过长，暂时无法展示。');
    const value = JSON.parse(source);
    if (!value || Array.isArray(value) || typeof value !== 'object') throw new Error('图示数据格式无效。');
    return value;
  }

  function validateChart(value) {
    const type = ['line', 'bar', 'scatter'].includes(value.type) ? value.type : '';
    const categories = value.categories;
    const series = value.series;
    if (!type || !Array.isArray(categories) || !categories.length || categories.length > 100 ||
        !categories.every(item => ['string', 'number'].includes(typeof item)) ||
        !Array.isArray(series) || !series.length || series.length > 8) {
      throw new Error('图表仅支持 1 至 100 个类别和最多 8 组数据。');
    }
    const cleanSeries = series.map(item => {
      if (!item || typeof item.name !== 'string' || !Array.isArray(item.data) || item.data.length !== categories.length ||
          !item.data.every(finite) || item.data.some(number => Math.abs(number) > 1e12)) {
        throw new Error('每组图表数据必须是与类别等长的有限数值。');
      }
      return {name: item.name.slice(0, 80), data: item.data.slice()};
    });
    return {
      type,
      title: typeof value.title === 'string' ? value.title.slice(0, 120) : '',
      xLabel: typeof value.xLabel === 'string' ? value.xLabel.slice(0, 80) : '',
      yLabel: typeof value.yLabel === 'string' ? value.yLabel.slice(0, 80) : '',
      categories: categories.map(item => String(item).slice(0, 100)),
      series: cleanSeries,
    };
  }

  function validateFunction(value) {
    const coefficients = value.coefficients;
    const xMin = value.xMin;
    const xMax = value.xMax;
    if (!Array.isArray(coefficients) || coefficients.length < 1 || coefficients.length > 4 ||
        !coefficients.every(finite) || coefficients.some(number => Math.abs(number) > 1e6) ||
        !finite(xMin) || !finite(xMax) || xMin >= xMax || xMax - xMin > 1e5) {
      throw new Error('函数图仅支持有限系数及有效的横轴范围。');
    }
    let parameter = null;
    if (value.parameter != null) {
      const item = value.parameter;
      if (!item || !Number.isInteger(item.coefficientIndex) || item.coefficientIndex < 0 ||
          item.coefficientIndex >= coefficients.length || !finite(item.min) || !finite(item.max) ||
          !finite(item.step) || !finite(item.value) || item.min >= item.max || item.step <= 0 ||
          item.value < item.min || item.value > item.max || item.max - item.min > 1e6) {
        throw new Error('函数参数滑块范围无效。');
      }
      parameter = {
        name: typeof item.name === 'string' ? item.name.slice(0, 40) : '参数',
        coefficientIndex: item.coefficientIndex,
        min: item.min,
        max: item.max,
        step: item.step,
        value: item.value,
      };
    }
    return {
      title: typeof value.title === 'string' ? value.title.slice(0, 120) : '',
      coefficients: coefficients.slice(),
      xMin,
      xMax,
      parameter,
    };
  }

  function validateFlow(value) {
    const direction = ['TD', 'TB', 'BT', 'LR', 'RL'].includes(value.direction) ? value.direction : 'TD';
    if (!Array.isArray(value.nodes) || !value.nodes.length || value.nodes.length > MAX_NODES ||
        !Array.isArray(value.edges) || value.edges.length > 300) throw new Error('流程图节点或连线数量无效。');
    const nodes = [];
    const ids = new Set();
    for (const node of value.nodes) {
      if (!node || typeof node.id !== 'string' || !node.id || node.id.length > 64 || ids.has(node.id) ||
          typeof node.label !== 'string' || node.label.length > 180) throw new Error('流程图节点数据无效。');
      ids.add(node.id);
      nodes.push({id: node.id, label: node.label});
    }
    const edges = value.edges.map(edge => {
      if (!edge || !ids.has(edge.from) || !ids.has(edge.to) || edge.from === edge.to ||
          (edge.label != null && (typeof edge.label !== 'string' || edge.label.length > 100))) {
        throw new Error('流程图连线引用了无效节点。');
      }
      return {from: edge.from, to: edge.to, label: edge.label || ''};
    });
    return {title: typeof value.title === 'string' ? value.title.slice(0, 120) : '', direction, nodes, edges};
  }

  function parseFlowMermaid(source) {
    const lines = source.split(/\r?\n/).map(line => line.trim()).filter(line => line && !line.startsWith('%%'));
    const header = /^(?:flowchart|graph)\s+(TD|TB|BT|LR|RL)$/i.exec(lines[0] || '');
    if (!header || lines.length > MAX_NODES * 3 + 1) throw new Error('流程图语法暂不支持。');
    const nodes = new Map();
    const edges = [];
    const endpoint = token => {
      const match = /^([A-Za-z0-9_-]{1,64})(?:\[([^\]]{0,180})\]|\{([^}]{0,180})\}|\(([^)]{0,180})\)|\(\(([^)]{0,180})\)\))?$/.exec(token.trim());
      if (!match) throw new Error('流程图节点语法暂不支持。');
      const id = match[1];
      const label = match[2] ?? match[3] ?? match[4] ?? match[5] ?? id;
      if (!nodes.has(id)) nodes.set(id, {id, label});
      return id;
    };
    for (const line of lines.slice(1)) {
      const match = /^(.+?)\s*(-->|---|==>|-.->)\s*(?:\|([^|]{1,100})\|\s*)?(.+)$/.exec(line);
      if (match) edges.push({from: endpoint(match[1]), to: endpoint(match[4]), label: match[3] || ''});
      else endpoint(line);
      if (nodes.size > MAX_NODES || edges.length > 300) throw new Error('流程图节点或连线数量过多。');
    }
    return validateFlow({direction: header[1].toUpperCase(), nodes: [...nodes.values()], edges});
  }

  function validateMindmap(value) {
    const root = value.root;
    let count = 0;
    function visit(node, depth) {
      const label = node && (typeof node.label === 'string' ? node.label : node.text);
      if (typeof label !== 'string' || !label.trim() || label.length > 180 ||
          depth > MAX_DEPTH || ++count > MAX_NODES) throw new Error('知识导图层级或节点数据无效。');
      const children = node.children == null ? [] : node.children;
      if (!Array.isArray(children)) throw new Error('知识导图子节点格式无效。');
      return {label: label.trim(), children: children.map(child => visit(child, depth + 1))};
    }
    return {title: typeof value.title === 'string' ? value.title.slice(0, 120) : '', root: visit(root, 0)};
  }

  function parseMindmapMermaid(source) {
    const lines = source.split(/\r?\n/).filter(line => line.trim());
    if (!/^\s*mindmap\s*$/.test(lines[0] || '') || lines.length > MAX_NODES + 1) throw new Error('知识导图语法暂不支持。');
    const root = {label: '', children: []};
    const stack = [{indent: -1, node: root}];
    for (const line of lines.slice(1)) {
      const raw = line.match(/^(\s*)(.*)$/);
      const indent = raw[1].replace(/\t/g, '  ').length;
      const shaped = /^(?:[A-Za-z0-9_-]+)?\(\((.*)\)\)$|^(?:[A-Za-z0-9_-]+)?\((.*)\)$|^(?:[A-Za-z0-9_-]+)?\{(.*)\}$|^(?:[A-Za-z0-9_-]+)?\[([^\]]*)\]$/.exec(raw[2].trim());
      const label = (shaped ? shaped.slice(1).find(value => value !== undefined) : raw[2]).trim();
      if (!label || label.length > 180) throw new Error('知识导图节点内容无效。');
      while (stack.length > 1 && stack[stack.length - 1].indent >= indent) stack.pop();
      const node = {label, children: []};
      stack[stack.length - 1].node.children.push(node);
      stack.push({indent, node});
    }
    return validateMindmap({root: root.children[0]});
  }

  function create(tag, className, text) {
    const node = document.createElement(tag);
    if (className) node.className = className;
    if (text != null) node.textContent = escapeText(text);
    return node;
  }

  function svg(tag, attrs, text) {
    const node = document.createElementNS(SVG_NS, tag);
    for (const [key, value] of Object.entries(attrs || {})) node.setAttribute(key, String(value));
    if (text != null) node.textContent = escapeText(text);
    return node;
  }

  function makeButton(label, title, action) {
    const button = create('button', 'exam-rich-button', label);
    button.type = 'button';
    button.title = title;
    button.setAttribute('aria-label', title);
    button.addEventListener('click', action);
    return button;
  }

  function makeToolbar() {
    const toolbar = create('div', 'exam-rich-toolbar');
    return toolbar;
  }

  function fitViewport(viewport, content, track) {
    let scale = 1;
    let x = 0;
    let y = 0;
    let dragging = false;
    let pointerX = 0;
    let pointerY = 0;
    const apply = () => { content.style.transform = `translate(${x}px, ${y}px) scale(${scale})`; };
    const controls = makeToolbar();
    const zoomIn = makeButton('+', '放大图示', () => { scale = Math.min(2.5, scale + 0.15); apply(); });
    const zoomOut = makeButton('-', '缩小图示', () => { scale = Math.max(0.5, scale - 0.15); apply(); });
    const reset = makeButton('重置视图', '恢复图示位置和比例', () => { scale = 1; x = 0; y = 0; apply(); });
    controls.append(zoomOut, zoomIn, reset);
    viewport.append(controls);
    const onWheel = event => {
      if (event.target.closest('button, summary, input')) return;
      event.preventDefault();
      scale = Math.max(0.5, Math.min(2.5, scale + (event.deltaY < 0 ? 0.08 : -0.08)));
      apply();
    };
    const onPointerDown = event => {
      if (event.target.closest('button, summary, input, a')) return;
      dragging = true;
      pointerX = event.clientX - x;
      pointerY = event.clientY - y;
      viewport.classList.add('is-dragging');
      viewport.setPointerCapture(event.pointerId);
    };
    const onPointerMove = event => {
      if (!dragging) return;
      x = event.clientX - pointerX;
      y = event.clientY - pointerY;
      apply();
    };
    const stop = () => { dragging = false; viewport.classList.remove('is-dragging'); };
    viewport.addEventListener('wheel', onWheel, {passive: false});
    viewport.addEventListener('pointerdown', onPointerDown);
    viewport.addEventListener('pointermove', onPointerMove);
    viewport.addEventListener('pointerup', stop);
    viewport.addEventListener('pointercancel', stop);
    track?.push(() => {
      viewport.removeEventListener('wheel', onWheel);
      viewport.removeEventListener('pointerdown', onPointerDown);
      viewport.removeEventListener('pointermove', onPointerMove);
      viewport.removeEventListener('pointerup', stop);
      viewport.removeEventListener('pointercancel', stop);
    });
    apply();
  }

  function makeFigure(state, title, source, kind) {
    const figure = create('figure', 'exam-rich-figure');
    const head = create('div', 'exam-rich-head');
    if (title) head.append(create('figcaption', 'exam-rich-title', title));
    const toolbar = makeToolbar();
    const details = create('details', 'exam-rich-source');
    details.append(create('summary', '', '查看图示数据'));
    const pre = create('pre', 'exam-rich-source-code');
    pre.textContent = source;
    details.append(pre);
    toolbar.append(details);
    const open = makeButton('大画布', '在大画布中查看图示', () => openCanvas(state, title, source, kind));
    toolbar.append(open);
    head.append(toolbar);
    figure.append(head);
    return figure;
  }

  function openCanvas(state, title, source, kind) {
    if (!document.createElement('dialog').showModal) return;
    const dialog = create('dialog', 'exam-rich-dialog');
    state.dialogs.add(dialog);
    const head = create('header', 'exam-rich-dialog-head');
    head.append(create('strong', '', title || '学习图示'));
    head.append(makeButton('关闭', '关闭大画布', () => dialog.close()));
    const body = create('div', 'exam-rich-dialog-body');
    dialog.append(head, body);
    document.body.append(dialog);
    dialog.addEventListener('close', () => {
      for (const chart of state.charts) {
        if (body.contains(chart.getDom())) {
          chart.dispose();
          state.charts.delete(chart);
        }
      }
      state.dialogs.delete(dialog);
      dialog.remove();
    }, {once: true});
    try {
      if (kind === 'chart') renderChart(body, state, validateChart(parseJson(source)), source, true);
      else if (kind === 'function') renderFunction(body, state, validateFunction(parseJson(source)), source, true);
      else if (kind === 'flow') {
        const graph = /^\s*(?:flowchart|graph)\s+/i.test(source) ? parseFlowMermaid(source) : validateFlow(parseJson(source));
        renderFlow(body, state, graph, source, true);
      } else {
        const map = /^\s*mindmap\s*(?:\r?\n|$)/.test(source) ? parseMindmapMermaid(source) : validateMindmap(parseJson(source));
        renderMindmap(body, state, map, source, true);
      }
    } catch (error) {
      body.append(create('p', 'exam-rich-error', error.message || '图示暂时无法打开。'));
    }
    dialog.showModal();
  }

  function renderChart(target, state, data, source, large) {
    const figure = makeFigure(state, data.title, source, 'chart');
    const chartHost = create('div', 'exam-rich-chart');
    if (large) chartHost.classList.add('is-large');
    figure.append(chartHost);
    target.append(figure);
    if (!window.echarts || typeof window.echarts.init !== 'function') {
      chartHost.append(create('p', 'exam-rich-error', '图表组件未加载；原始数据仍可查看。'));
      return;
    }
    const chart = window.echarts.init(chartHost, null, {renderer: 'canvas'});
    state.charts.add(chart);
    const setOption = () => chart.setOption({
      animation: !window.matchMedia('(prefers-reduced-motion: reduce)').matches,
      color: ['#3d61ce', '#17836d', '#c27618', '#b34255', '#6763ad', '#4e7488', '#8a6b4b', '#5e7185'],
      title: data.title ? {text: data.title, left: 12, top: 4, textStyle: {fontSize: 15, fontWeight: 600, color: '#21324b'}} : undefined,
      grid: {left: 58, right: 22, top: data.title ? 48 : 22, bottom: data.categories.length > 12 ? 82 : 54, containLabel: true},
      legend: {type: 'scroll', top: 8, right: 16, data: data.series.map(item => item.name)},
      tooltip: {trigger: 'axis', axisPointer: {type: 'cross'}},
      xAxis: {type: 'category', name: data.xLabel, data: data.categories, axisLabel: {hideOverlap: true}},
      yAxis: {type: 'value', name: data.yLabel, scale: true},
      dataZoom: [{type: 'inside', filterMode: 'none'}, {type: 'slider', height: 18, bottom: 10, show: data.categories.length > 12}],
      series: data.series.map(item => ({name: item.name, type: data.type, data: item.data, smooth: data.type === 'line', showSymbol: data.categories.length < 40, emphasis: {focus: 'series'}})),
    }, true);
    setOption();
    const observer = new ResizeObserver(() => chart.resize());
    observer.observe(chartHost);
    state.cleanups.push(() => observer.disconnect());
  }

  function renderFunction(target, state, data, source, large) {
    const figure = makeFigure(state, data.title, source, 'function');
    let coefficients = data.coefficients.slice();
    const chartHost = create('div', 'exam-rich-chart');
    if (large) chartHost.classList.add('is-large');
    figure.append(chartHost);
    target.append(figure);
    let slider = null;
    let readout = null;
    if (data.parameter) {
      const row = create('label', 'exam-rich-slider');
      row.append(create('span', '', data.parameter.name));
      slider = document.createElement('input');
      slider.type = 'range';
      slider.min = String(data.parameter.min);
      slider.max = String(data.parameter.max);
      slider.step = String(data.parameter.step);
      slider.value = String(data.parameter.value);
      slider.setAttribute('aria-label', data.parameter.name);
      readout = create('output', '', slider.value);
      row.append(slider, readout);
      figure.insertBefore(row, chartHost);
      slider.addEventListener('input', () => {
        const value = Number(slider.value);
        coefficients[data.parameter.coefficientIndex] = value;
        readout.value = slider.value;
        readout.textContent = slider.value;
        update();
      });
    }
    if (!window.echarts || typeof window.echarts.init !== 'function') {
      chartHost.append(create('p', 'exam-rich-error', '图表组件未加载；函数表达式仍可查看。'));
      return;
    }
    const chart = window.echarts.init(chartHost, null, {renderer: 'canvas'});
    state.charts.add(chart);
    const update = () => {
      const values = Array.from({length: 401}, (_, index) => {
        const x = data.xMin + (data.xMax - data.xMin) * index / 400;
        const y = coefficients.reduce((sum, coefficient, power) => sum + coefficient * Math.pow(x, coefficients.length - power - 1), 0);
        return [Number(x.toPrecision(8)), Number(y.toPrecision(8))];
      });
      chart.setOption({
        animation: !window.matchMedia('(prefers-reduced-motion: reduce)').matches,
        color: ['#3d61ce'],
        grid: {left: 60, right: 22, top: 18, bottom: 52, containLabel: true},
        tooltip: {trigger: 'axis', axisPointer: {type: 'cross'}, valueFormatter: value => Number(value).toPrecision(5)},
        xAxis: {type: 'value', name: 'x', min: data.xMin, max: data.xMax, splitLine: {show: true}},
        yAxis: {type: 'value', name: 'y', scale: true, splitLine: {show: true}},
        dataZoom: [{type: 'inside', filterMode: 'none'}, {type: 'slider', height: 18, bottom: 8}],
        series: [{type: 'line', showSymbol: false, data: values, smooth: false, lineStyle: {width: 3}}],
      }, true);
    };
    update();
    const observer = new ResizeObserver(() => chart.resize());
    observer.observe(chartHost);
    state.cleanups.push(() => observer.disconnect());
  }

  function wrapFlowLabel(value) {
    const lines = [];
    for (const paragraph of String(value).split('\n')) {
      let line = '';
      let width = 0;
      for (const char of paragraph) {
        const units = char.codePointAt(0) > 0x2ff ? 2 : 1;
        if (line && width + units > 16) { lines.push(line); line = ''; width = 0; }
        line += char;
        width += units;
      }
      lines.push(line);
    }
    return lines.length ? lines : [''];
  }

  function flowPositions(graph) {
    const nodes = graph.nodes;
    const ids = nodes.map(node => node.id);
    const index = new Map(ids.map((id, i) => [id, i]));
    const adjacency = Array.from({length: nodes.length}, () => []);
    graph.edges.forEach((edge, edgeIndex) => adjacency[index.get(edge.from)].push({to: index.get(edge.to), edgeIndex}));
    const state = new Array(nodes.length).fill(0);
    const backEdges = new Set();
    const visit = node => {
      state[node] = 1;
      for (const next of adjacency[node]) {
        if (state[next.to] === 1) backEdges.add(next.edgeIndex);
        else if (state[next.to] === 0) visit(next.to);
      }
      state[node] = 2;
    };
    nodes.forEach((_, i) => { if (state[i] === 0) visit(i); });
    const indegree = new Array(nodes.length).fill(0);
    const layers = new Array(nodes.length).fill(0);
    const outgoing = Array.from({length: nodes.length}, () => []);
    for (const [edgeIndex, edge] of graph.edges.entries()) {
      if (backEdges.has(edgeIndex)) continue;
      const from = index.get(edge.from);
      const to = index.get(edge.to);
      outgoing[from].push(to);
      indegree[to]++;
    }
    const queue = indegree.flatMap((value, i) => value === 0 ? [i] : []);
    while (queue.length) {
      const current = queue.shift();
      for (const next of outgoing[current]) {
        layers[next] = Math.max(layers[next], layers[current] + 1);
        if (--indegree[next] === 0) queue.push(next);
      }
    }
    const ranks = new Map();
    layers.forEach((layer, i) => ranks.set(layer, [...(ranks.get(layer) || []), i]));
    const coords = new Map();
    const horizontal = graph.direction === 'LR' || graph.direction === 'RL';
    const reverse = graph.direction === 'BT' || graph.direction === 'RL';
    const maxLayer = Math.max(0, ...layers);
    const maxPerLayer = Math.max(1, ...[...ranks.values()].map(items => items.length));
    const spacingX = 250;
    const lines = new Map(nodes.map(node => [node.id, wrapFlowLabel(node.label)]));
    const heights = new Map(nodes.map(node => [node.id, Math.max(48, lines.get(node.id).length * 18 + 16)]));
    const maxHeight = Math.max(...heights.values());
    const horizontalSpacingY = Math.max(112, maxHeight + 40);
    const verticalSpacingY = Math.max(120, maxHeight + 60);
    nodes.forEach((node, i) => {
      const layer = reverse ? maxLayer - layers[i] : layers[i];
      const place = ranks.get(layers[i]).indexOf(i);
      const x = (horizontal ? layer : place) * (horizontal ? spacingX : 220) + 100;
      const y = (horizontal ? place : layer) * (horizontal ? horizontalSpacingY : verticalSpacingY) + maxHeight / 2 + 32;
      coords.set(node.id, {x, y});
    });
    return {coords, width: (horizontal ? maxLayer + 1 : maxPerLayer) * (horizontal ? spacingX : 220) + 200,
      height: (horizontal ? maxPerLayer : maxLayer + 1) * (horizontal ? horizontalSpacingY : verticalSpacingY) + maxHeight + 64,
      lines, heights, backEdges};
  }

  function renderFlow(target, state, graph, source, large) {
    const figure = makeFigure(state, graph.title, source, 'flow');
    const viewport = create('div', 'exam-rich-viewport exam-rich-flow');
    if (large) viewport.classList.add('is-large');
    const canvas = create('div', 'exam-rich-pan-canvas');
    viewport.append(canvas);
    figure.append(viewport);
    target.append(figure);
    try {
      const layout = flowPositions(graph);
      const {coords, lines, heights, backEdges} = layout;
      const horizontal = graph.direction === 'LR' || graph.direction === 'RL';
      const reverse = graph.direction === 'BT' || graph.direction === 'RL';
      const laneOffsets = new Map();
      let extra = 30;
      for (const edgeIndex of backEdges) {
        laneOffsets.set(edgeIndex, extra);
        extra += Math.max(34, wrapFlowLabel(graph.edges[edgeIndex].label).length * 18 + 12);
      }
      const width = layout.width + (!horizontal && backEdges.size ? extra + 150 : 0);
      const height = layout.height + (horizontal && backEdges.size ? extra : 0);
      const plot = svg('svg', {viewBox: `0 0 ${width} ${height}`, role: 'img', 'aria-label': graph.title || '流程图'});
      plot.style.width = `${width}px`;
      plot.style.height = `${height}px`;
      const defs = svg('defs');
      const marker = svg('marker', {id: 'exam-rich-arrow-' + Math.random().toString(36).slice(2), viewBox: '0 0 10 10', refX: 9, refY: 5, markerWidth: 6, markerHeight: 6, orient: 'auto-start-reverse'});
      marker.append(svg('path', {d: 'M 0 0 L 10 5 L 0 10 z', fill: '#5e6d82'}));
      defs.append(marker);
      plot.append(defs);
      const markerId = marker.id;
      for (const [edgeIndex, edge] of graph.edges.entries()) {
        const from = coords.get(edge.from);
        const to = coords.get(edge.to);
        const start = horizontal ? {x: from.x + (reverse ? -70 : 70), y: from.y} : {x: from.x, y: from.y + (reverse ? -heights.get(edge.from) / 2 : heights.get(edge.from) / 2)};
        const end = horizontal ? {x: to.x + (reverse ? 70 : -70), y: to.y} : {x: to.x, y: to.y + (reverse ? heights.get(edge.to) / 2 : -heights.get(edge.to) / 2)};
        const middle = horizontal ? (start.x + end.x) / 2 : (start.y + end.y) / 2;
        const isBack = backEdges.has(edgeIndex);
        const lane = (horizontal ? layout.height : layout.width) + (laneOffsets.get(edgeIndex) || 0);
        const path = isBack
          ? horizontal
            ? `M${from.x},${from.y + heights.get(edge.from) / 2} L${from.x},${lane} L${to.x},${lane} L${to.x},${to.y + heights.get(edge.to) / 2}`
            : `M${from.x + 70},${from.y} L${lane},${from.y} L${lane},${to.y} L${to.x + 70},${to.y}`
          : horizontal ? `M${start.x},${start.y} C${middle},${start.y} ${middle},${end.y} ${end.x},${end.y}` : `M${start.x},${start.y} C${start.x},${middle} ${end.x},${middle} ${end.x},${end.y}`;
        plot.append(svg('path', {d: path, class: isBack ? 'exam-rich-edge is-back-edge' : 'exam-rich-edge', 'data-from': edge.from, 'data-to': edge.to, 'marker-end': `url(#${markerId})`}));
        if (edge.label) {
          const labelLines = wrapFlowLabel(edge.label);
          const x = isBack ? (horizontal ? (from.x + to.x) / 2 : lane + 8) : (horizontal ? middle : start.x + 12);
          const y = isBack ? (horizontal ? lane - 7 - (labelLines.length - 1) * 18 : (from.y + to.y) / 2) : (horizontal ? start.y - 7 - (labelLines.length - 1) * 18 : middle);
          const label = svg('text', {x, y, class: 'exam-rich-edge-label', 'text-anchor': isBack && horizontal ? 'middle' : 'start'});
          labelLines.forEach((line, i) => label.append(svg('tspan', {x, dy: i ? 18 : 0}, line)));
          plot.append(label);
        }
      }
      for (const node of graph.nodes) {
        const point = coords.get(node.id);
        const widthBox = 140;
        const heightBox = heights.get(node.id);
        const group = svg('g', {class: 'exam-rich-node'});
        group.append(svg('rect', {x: point.x - widthBox / 2, y: point.y - heightBox / 2, width: widthBox, height: heightBox, rx: 7}));
        const text = svg('text', {x: point.x, y: point.y - (lines.get(node.id).length - 1) * 9 + 5, 'text-anchor': 'middle'});
        lines.get(node.id).forEach((line, i) => text.append(svg('tspan', {x: point.x, dy: i ? 18 : 0}, line)));
        group.append(text);
        plot.append(group);
      }
      canvas.append(plot);
      fitViewport(viewport, canvas, state.cleanups);
    } catch (error) {
      viewport.append(create('p', 'exam-rich-error', error.message || '流程图暂时无法展示。'));
    }
  }

  function renderMindmap(target, state, map, source, large) {
    const figure = makeFigure(state, map.title, source, 'mindmap');
    const viewport = create('div', 'exam-rich-viewport exam-rich-mindmap');
    if (large) viewport.classList.add('is-large');
    const canvas = create('div', 'exam-rich-pan-canvas');
    const tree = create('ul', 'exam-rich-tree');
    canvas.append(tree);
    function appendNode(parent, node, depth, isRoot) {
      const item = create('li', depth === 0 ? 'is-root' : '');
      const label = node.children.length ? document.createElement('details') : document.createElement('div');
      if (label.tagName === 'DETAILS' && isRoot) label.open = true;
      const summary = label.tagName === 'DETAILS' ? create('summary', '', node.label) : create('span', 'exam-rich-tree-label', node.label);
      label.append(summary);
      if (node.children.length) {
        const children = create('ul', 'exam-rich-tree-children');
        for (const child of node.children) appendNode(children, child, depth + 1, false);
        label.append(children);
      }
      item.append(label);
      parent.append(item);
    }
    appendNode(tree, map.root, 0, true);
    viewport.append(canvas);
    figure.append(viewport);
    target.append(figure);
    fitViewport(viewport, canvas, state.cleanups);
  }

  function codeLanguage(block) {
    return block.querySelector('.chat-code-lang')?.textContent.trim().toLowerCase() || '';
  }

  function addCodeTools(block, code, language, options) {
    const head = block.querySelector('.chat-code-lang')?.parentElement || block;
    const tools = create('div', 'exam-rich-code-tools');
    const copy = makeButton('复制', '复制代码', async () => {
      try {
        if (window.GangyiChat?.copyText) await window.GangyiChat.copyText(code.textContent);
        else await navigator.clipboard.writeText(code.textContent);
        copy.textContent = '已复制';
        window.setTimeout(() => { copy.textContent = '复制'; }, 1200);
      } catch (_) {
        copy.textContent = '复制失败';
      }
    });
    tools.append(copy);
    if (typeof options.onRunCode === 'function') {
      // 运行能力只能由集成方提供的受限执行器实现；本模块不会执行本机命令或任意脚本。
      const run = makeButton('运行', '在受限的学习代码环境中运行', async () => {
        run.disabled = true;
        try {
          const result = await options.onRunCode({language, code: code.textContent});
          const output = create('pre', 'exam-rich-code-result');
          output.textContent = String(result?.output ?? result ?? '运行完成。');
          block.append(output);
        } catch (error) {
          const output = create('pre', 'exam-rich-code-result is-error');
          output.textContent = String(error?.message || '运行失败。');
          block.append(output);
        } finally {
          run.disabled = false;
        }
      });
      tools.append(run);
    }
    head.append(tools);
  }

  function enhance(root, state, options) {
    for (const block of [...root.querySelectorAll('.chat-code')]) {
      if (block.dataset.examEnhanced === 'true') continue;
      block.dataset.examEnhanced = 'true';
      const code = block.querySelector('pre code');
      if (!code) continue;
      const language = codeLanguage(block);
      const source = code.textContent;
      const aliases = {
        chart: 'chart', echarts: 'chart', 'chart-json': 'chart',
        plot: 'function', function: 'function', 'function-json': 'function',
        flowchart: 'flow', 'flowchart-json': 'flow', mermaid: 'flow',
        mindmap: 'mindmap', 'mindmap-json': 'mindmap',
      };
      const kind = aliases[language];
      if (!kind) {
        addCodeTools(block, code, language, options);
        continue;
      }
      try {
        let value;
        if (kind === 'flow' && language === 'mermaid') value = parseFlowMermaid(source);
        else if (kind === 'mindmap' && language === 'mindmap') value = parseMindmapMermaid(source);
        else value = parseJson(source);
        if (kind === 'chart') value = validateChart(value);
        else if (kind === 'function') value = validateFunction(value);
        else if (kind === 'flow' && language !== 'mermaid') value = validateFlow(value);
        else if (kind === 'mindmap' && language !== 'mindmap') value = validateMindmap(value);
        const replacement = document.createElement('div');
        if (kind === 'chart') renderChart(replacement, state, value, source, false);
        else if (kind === 'function') renderFunction(replacement, state, value, source, false);
        else if (kind === 'flow') renderFlow(replacement, state, value, source, false);
        else renderMindmap(replacement, state, value, source, false);
        block.replaceWith(...replacement.childNodes);
      } catch (error) {
        const message = create('p', 'exam-rich-error', error.message || '这段可视化内容暂时无法展示。');
        block.after(message);
      }
    }
  }

  function destroy(root) {
    const state = activeStates.get(root);
    if (!state) return;
    for (const cleanup of state.cleanups) cleanup();
    for (const chart of state.charts) chart.dispose();
    for (const dialog of state.dialogs) dialog.close?.();
    activeStates.delete(root);
  }

  function renderArtifact(root, artifact) {
    if (!root || typeof root.replaceChildren !== 'function') throw new TypeError('需要有效的图示容器。');
    destroy(root);
    root.replaceChildren();
    root.classList.add('exam-rich-output');
    const state = {charts: new Set(), dialogs: new Set(), cleanups: []};
    activeStates.set(root, state);
    const source = typeof artifact?.content === 'string' ? artifact.content : '';
    try {
      const value = parseJson(source);
      if (artifact.type === 'chart') renderChart(root, state, validateChart(value), source, false);
      else if (artifact.type === 'function') renderFunction(root, state, validateFunction(value), source, false);
      else if (artifact.type === 'flowchart') renderFlow(root, state, validateFlow(value), source, false);
      else if (artifact.type === 'mindmap') renderMindmap(root, state, validateMindmap(value), source, false);
      else throw new Error('暂不支持这种图示类型。');
    } catch (error) {
      root.append(create('p', 'exam-rich-error', error.message || '图示暂时无法展示。'));
      const details = create('details', 'exam-rich-source');
      details.append(create('summary', '', '查看图示数据'));
      details.append(create('pre', 'exam-rich-source-code', source));
      root.append(details);
    }
    return {destroy: () => destroy(root)};
  }

  function render(root, markdown, options = {}) {
    if (!root || typeof root.replaceChildren !== 'function') throw new TypeError('需要有效的聊天内容容器。');
    destroy(root);
    const state = {charts: new Set(), dialogs: new Set(), cleanups: []};
    activeStates.set(root, state);
    if (!window.GangyiChat || typeof window.GangyiChat.renderMarkdown !== 'function') {
      root.replaceChildren(create('p', 'exam-rich-error', '聊天内容渲染组件尚未加载。'));
      return {destroy: () => destroy(root)};
    }
    root.classList.add('exam-rich-output');
    root.innerHTML = window.GangyiChat.renderMarkdown(String(markdown ?? ''), {math: true});
    enhance(root, state, options);
    return {destroy: () => destroy(root), update: value => render(root, value, options)};
  }

  window.ExamRichOutput = {
    render,
    renderArtifact,
    destroy,
    parseJson,
    parseFlowMermaid,
    parseMindmapMermaid,
    validateChart,
    validateFunction,
    validateFlow,
    validateMindmap,
  };
})();

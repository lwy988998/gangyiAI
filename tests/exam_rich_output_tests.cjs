// 试卷讲解富内容模块的结构化数据回归测试。
const assert = require('assert');
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const window = {};
const context = {
  window,
  console,
  document: {},
  setTimeout,
  clearTimeout,
};
vm.runInNewContext(fs.readFileSync(path.join(__dirname, '../public/exam-rich-output.js'), 'utf8'), context);
const api = window.ExamRichOutput;
assert(api, '富内容模块应暴露 ExamRichOutput');

const chart = api.validateChart({
  type: 'line',
  categories: ['第1题', '第2题'],
  series: [{name: '掌握度', data: [70, 85]}],
});
assert.deepStrictEqual(chart.categories, ['第1题', '第2题']);
assert.throws(() => api.validateChart({
  type: 'bar', categories: ['A'], series: [{name: '错误', data: [Infinity]}],
}), /有限数值/);
assert.throws(() => api.validateChart({
  type: 'line', categories: ['A'], series: [{name: '错误', data: [1, 2]}],
}), /等长/);

const fn = api.validateFunction({title: '二次函数', coefficients: [1, 0, -1], xMin: -2, xMax: 2,
  parameter: {name: 'a', coefficientIndex: 0, min: -2, max: 2, step: 0.5, value: 1}});
assert.strictEqual(fn.parameter.coefficientIndex, 0);
assert.throws(() => api.validateFunction({coefficients: [1], xMin: 2, xMax: 1}), /有效的横轴/);

const flow = api.validateFlow({
  direction: 'LR',
  nodes: [{id: 'start', label: '读题'}, {id: 'end', label: '作答'}],
  edges: [{from: 'start', to: 'end', label: '理解后'}],
});
assert.strictEqual(flow.edges[0].from, 'start');
const mermaidFlow = api.parseFlowMermaid('flowchart LR\nA[读题] -->|理解| B{作答}');
assert.strictEqual(mermaidFlow.nodes[1].label, '作答');
assert.throws(() => api.validateFlow({
  nodes: [{id: 'same', label: '节点'}], edges: [{from: 'same', to: 'missing'}],
}), /无效节点/);

const mindmap = api.validateMindmap({root: {label: '函数', children: [{label: '定义', children: []}]}});
assert.strictEqual(mindmap.root.children[0].label, '定义');
const textMindmap = api.validateMindmap({root: {text: '历史', children: [{text: '背景'}]}});
assert.strictEqual(textMindmap.root.children[0].label, '背景');
const mermaidMindmap = api.parseMindmapMermaid('mindmap\n  根节点\n    分支 <img>');
assert.strictEqual(mermaidMindmap.root.children[0].label, '分支 <img>');
assert.throws(() => api.validateMindmap({root: {label: 'x', children: [{label: ''}]}}), /节点数据/);

assert.throws(() => api.parseJson('<img src=x onerror=alert(1)>'), /Unexpected token|图示数据格式/);
console.log('exam rich output tests passed');

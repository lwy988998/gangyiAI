/* 同一探索协议在浏览器与后端之外的纯数据边界回归。 */
const test = require('node:test');
const assert = require('node:assert/strict');
const engine = require('../public/interactive-learning.js');
const config = {type: 'sets', schemaVersion: 1, universe: ['1', '2', '3'], setA: ['1', '2'], setB: ['2', '3']};
const state = {schemaVersion: 1, contentVersion: 4, setA: ['1', '2'], setB: ['1', '2', '3']};

test('交集符合同时属于两集合的定义，含空集和全集', () => {
  const universe = ['a', 'b', 'c', 'd'];
  for (let a = 0; a < 16; a++) for (let b = 0; b < 16; b++) {
    const setA = universe.filter((_, i) => a & (1 << i));
    const setB = universe.filter((_, i) => b & (1 << i));
    const result = engine.intersection({universe}, {setA, setB});
    assert.deepEqual(result, universe.filter((_, i) => (a & b) & (1 << i)));
  }
});

test('拒绝未知组件与无效配置，不执行任意代码', () => {
  assert.deepEqual(engine.types, ['sets','function_lab','circuit_lab','chemical_balance','motion_lab','number_line','vector_lab','equation_lab','statistics_lab','triangle_lab','wave_lab','periodic_lab','cell_lab','history_lab','climate_lab','annotation_lab','sentence_lab','algorithm_lab']);
  for (const patch of [{type: 'javascript'}, {schemaVersion: true}, {universe: []}, {setA: ['9']}, {setA: ['1', '1']}, {setA: [1]}, {universe: ['中'.repeat(22)]}])
    assert.throws(() => engine.validate({...config, ...patch}));
});

test('拒绝陈旧状态和自报成绩，状态独立于调用者可变数组', () => {
  assert.throws(() => engine.checkedState(config, state, 5));
  assert.throws(() => engine.checkedState(config, {...state, score: 100}, 4));
  assert.throws(() => engine.checkedState(config, {...state, intersection: ['fake']}, 4));
  const checked = engine.checkedState(config, state, 4);
  checked.setA.push('3');
  assert.deepEqual(state.setA, ['1', '2']);
});

test('恶意标记只是文字数据，缺少渲染器不得静默执行', () => {
  const text = '<img src=x onerror=alert(1)>';
  assert.equal(engine.validate({...config, universe: [text], setA: [text], setB: []}).universe[0], text);
  assert.throws(() => engine.mount(null, {type: 'unregistered'}));
});

test('并交容斥、补集与子集关系在所有小集合组合上成立', () => {
  const universe = ['1', '2', '3', '4'];
  for (let a = 0; a < 16; a++) for (let b = 0; b < 16; b++) {
    const setA = universe.filter((_, index) => a & (1 << index));
    const setB = universe.filter((_, index) => b & (1 << index));
    const result = engine.calculateSets({universe}, {setA, setB});
    assert.equal(result.union.length + result.intersection.length, setA.length + setB.length);
    assert.equal(result.complementA.length + setA.length, universe.length);
    assert.equal(result.complementB.length + setB.length, universe.length);
    assert.equal(result.relations.equal, a === b);
    assert.equal(result.relations.aSubsetB, (a & b) === a);
    assert.equal(result.relations.aProperSubsetB, (a & b) === a && a !== b);
  }
  assert.equal(engine.validate(config).operation, 'intersection');
  for (const operation of engine.setOperations) assert.equal(engine.validate({...config, operation}).operation, operation);
  assert.throws(() => engine.validate({...config, operation: 'generated_script'}));
});

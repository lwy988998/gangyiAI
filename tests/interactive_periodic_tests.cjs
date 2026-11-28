'use strict';
const test=require('node:test');
const assert=require('node:assert/strict');
const labs=require('../public/interactive-labs.js');
const data=require('../public/periodic-elements-data.js');
const config={type:'periodic_lab'};
const state={schemaVersion:1,contentVersion:1,datasetId:data.id,selectedZ:11,comparisonZ:17,highlight:'none'};

test('118种元素连续且显示位置唯一，前20号电子层总数与原子序数一致',()=>{
  assert.equal(data.elements.length,118);assert.equal(new Set(data.elements.map(item=>item.symbol)).size,118);
  const positions=new Set(),counts=Array(7).fill(0);
  const expectedShells=[[1],[2],[2,1],[2,2],[2,3],[2,4],[2,5],[2,6],[2,7],[2,8],
    [2,8,1],[2,8,2],[2,8,3],[2,8,4],[2,8,5],[2,8,6],[2,8,7],[2,8,8],[2,8,8,1],[2,8,8,2]];
  for(let index=0;index<118;index++){
    const item=data.elements[index];assert.equal(item.atomicNumber,index+1);counts[item.period-1]++;
    const position=`${item.gridRow}:${item.gridColumn}`;assert.ok(!positions.has(position));positions.add(position);
    if(index<20)assert.deepEqual(item.neutralShells,expectedShells[index]);else assert.equal(item.neutralShells,null);
    const actual=labs.calculate(config,{...state,selectedZ:index+1},1);
    assert.equal(actual.selected.protons,index+1);assert.equal(actual.selected.electrons,index+1);
    assert.equal(actual.selected.neutrons,null);assert.equal(actual.scoringEligible,false);
  }
  assert.deepEqual(counts,[2,8,8,18,18,32,32]);
  for(const [z,symbol,period,group] of [[1,'H',1,1],[2,'He',1,18],[26,'Fe',4,8],[79,'Au',6,11],[118,'Og',7,18]]){
    const item=data.elements[z-1];assert.equal(item.symbol,symbol);assert.equal(item.period,period);assert.equal(item.group,group);
  }
});

test('周期和族来自实际元素选择，单列系列不被强行判断成同族',()=>{
  const sodiumChlorine=labs.calculate(config,state,1);assert.equal(sodiumChlorine.samePeriod,true);assert.equal(sodiumChlorine.sameGroup,false);
  const sodiumPotassium=labs.calculate(config,{...state,comparisonZ:19,highlight:'group'},1);
  assert.equal(sodiumPotassium.samePeriod,false);assert.equal(sodiumPotassium.sameGroup,true);
  assert.deepEqual(sodiumPotassium.highlightedAtomicNumbers,[1,3,11,19,37,55,87]);
  const sixth=labs.calculate(config,{...state,selectedZ:58,comparisonZ:71,highlight:'period'},1);
  assert.equal(sixth.highlightedAtomicNumbers.length,32);assert.equal(sixth.samePeriod,true);assert.equal(sixth.sameGroup,null);
  for(const z of [57,58,71,89,92,103]){
    const actual=labs.calculate(config,{...state,selectedZ:z,highlight:'group'},1);
    assert.equal(actual.selected.group,null);assert.equal(actual.sameGroup,null);assert.deepEqual(actual.highlightedAtomicNumbers,[z]);
  }
  const copy=labs.calculate(config,state,1);copy.selected.neutralShells[0]=99;assert.deepEqual(data.elements[10].neutralShells,[2,8,1]);
});

test('元素状态拒绝越界、旧资料、伪造名称及原子组成',()=>{
  for(const patch of [{selectedZ:0},{selectedZ:119},{selectedZ:11.5},{selectedZ:true},{comparisonZ:null},
    {highlight:'reactivity'},{datasetId:'old-data'}]){
    assert.throws(()=>labs.validate({...config,...patch}));assert.throws(()=>labs.calculate(config,{...state,...patch},1));
  }
  for(const patch of [{protons:100},{symbol:'Au'},{neutralShells:[2,8,8]},{score:100},{contentVersion:2},{schemaVersion:true}])
    assert.throws(()=>labs.calculate(config,{...state,...patch},1));
  for(const key of Object.keys(state)){const missing={...state};delete missing[key];assert.throws(()=>labs.calculate(config,missing,1));}
  const copied=labs.checkedState(config,state,1);copied.selectedZ=1;assert.equal(state.selectedZ,11);
});

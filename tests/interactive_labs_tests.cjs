'use strict';
const test = require('node:test');
const assert = require('node:assert/strict');
const labs = require('../public/interactive-labs.js');
const fixtures = require('./fixtures/interactive-labs-v1.json');

test('统计量保留频数、平移缩放与排列不变性，拒绝非法数据和自报结果', () => {
  const config={type:'statistics_lab'},state={schemaVersion:1,contentVersion:1,values:[2,3,3,4,8],varianceBasis:'population'};
  for(const values of [[],Array(33).fill(0),[true],[null],['3'],[Infinity],[NaN],[100.5],[-101],[.1]]) {
    assert.throws(()=>labs.validate({...config,values}));assert.throws(()=>labs.calculate(config,{...state,values},1));
  }
  for(const patch of [{varianceBasis:'infer'},{varianceBasis:null},{score:100},{mean:4},{schemaVersion:true}])assert.throws(()=>labs.calculate(config,{...state,...patch},1));
  const copied=labs.checkedState(config,state,1);copied.values[0]=10;assert.equal(state.values[0],2);
  const checked=labs.validate({...config,values:state.values});checked.values[0]=11;assert.equal(state.values[0],2);
  for(let a=-3;a<=3;a++)for(let b=-2;b<=2;b++)for(const values of [[a,b,a+.5],[a,b,0,b+.5]]) {
    const original=labs.calculate(config,{...state,values},1);
    const reverse=labs.calculate(config,{...state,values:[...values].reverse()},1);
    for(const key of ['mean','median','modeValues','frequencies','exact'])assert.deepEqual(reverse[key],original[key]);
    const shifted=labs.calculate(config,{...state,values:values.map(v=>v+2)},1);
    assert.ok(Math.abs(shifted.mean-original.mean-2)<1e-12);assert.equal(shifted.median,original.median+2);assert.equal(shifted.populationVariance,original.populationVariance);
    const scaled=labs.calculate(config,{...state,values:values.map(v=>v*-2)},1);
    assert.equal(scaled.populationVariance,original.populationVariance*4);
    const duplicated=labs.calculate(config,{...state,values:values.flatMap(v=>[v,v])},1);
    assert.deepEqual(duplicated.exact.populationVariance,original.exact.populationVariance);assert.equal(duplicated.count,2*original.count);
    assert.equal(original.frequencies.reduce((n,item)=>n+item.count,0),values.length);
    assert.equal(original.scoringEligible,false);
  }
});

test('方程变形保留精确分数与解集，拒绝零乘除、伪造原题和越界步骤', () => {
  const config={type:'equation_lab',left:[3,1],right:[0,2]},state={schemaVersion:1,contentVersion:1,steps:[]};
  for(const steps of [[{op:'multiply',value:0}],[{op:'divide',value:0}],[{op:'add_x',value:true}],
    [{op:'add_constant',value:.5}],[{op:'add_x',value:11}],[{op:'add_x',value:1,score:100}],
    [{op:'left_only',value:1}],Array(13).fill({op:'add_constant',value:1}),Array(6).fill({op:'multiply',value:10})])
    assert.throws(()=>labs.calculate(config,{...state,steps},1));
  for(const patch of [{left:[1,2]},{solution:'1/3'},{score:100}])assert.throws(()=>labs.calculate(config,{...state,...patch},1));
  const copied=labs.checkedState(config,{...state,steps:[{op:'add_constant',value:-1}]},1);
  const next=labs.checkedState(config,copied,1);copied.steps[0].value=10;assert.equal(next.steps[0].value,-1);
  for(let a=-3;a<=3;a++)for(let b=-2;b<=2;b++)for(let c=-2;c<=2;c++){
    const cfg={type:'equation_lab',left:[a,b],right:[c,1]};
    const original=labs.calculate(cfg,state,1);
    const changed=labs.calculate(cfg,{...state,steps:[{op:'add_x',value:2},{op:'add_constant',value:-3},
      {op:'divide',value:-3},{op:'multiply',value:-3}]},1);
    assert.equal(changed.solutionKind,original.solutionKind);assert.equal(changed.solutionExact,original.solutionExact);
    if(changed.solution){
      const {numerator:n,denominator:d}=changed.solution;
      assert.ok((a-c)*n===(1-b)*d);
    }
  }
});

test('向量的交换、平行四边形与实数倍性质，零向量边界和草稿独立', () => {
  const config={type:'vector_lab'};
  for(const vectorA of [[0,0],[1,2],[-3,4],[.5,-.5]])for(const vectorB of [[0,0],[2,4],[4,3],[-.5,.5]]) {
    const state={schemaVersion:1,contentVersion:1,vectorA,vectorB,scalar:-2,operation:'add'};
    const sum=labs.calculate(config,state,1),difference=labs.calculate(config,{...state,operation:'subtract'},1);
    const exchanged=labs.calculate(config,{...state,vectorA:vectorB,vectorB:vectorA},1);
    assert.deepEqual(sum.vectorResult,exchanged.vectorResult);
    assert.equal(sum.dotProduct,exchanged.dotProduct);
    assert.equal(sum.determinant===0?0:sum.determinant,exchanged.determinant===0?0:-exchanged.determinant);
    assert.ok(Math.abs(sum.resultNorm**2+difference.resultNorm**2-2*(sum.normA**2+sum.normB**2))<1e-9);
    const scaled=labs.calculate(config,{...state,operation:'scale_a'},1);
    assert.ok(Math.abs(scaled.resultNorm-2*sum.normA)<1e-9);
    if(!sum.normA||!sum.normB) {
      assert.equal(sum.angleDegrees,null);assert.equal(sum.parallelDirections,null);assert.equal(sum.perpendicularDirections,null);
    }
  }
  for(const bad of [{vectorA:[1]},{vectorA:[true,2]},{vectorA:[.1,2]},{vectorB:null},
    {vectorB:[11,0]},{vectorA:Array(2)},{scalar:3.5},{operation:'cross_3d'}])assert.throws(()=>labs.validate({...config,...bad}));
  const state={schemaVersion:1,contentVersion:1,vectorA:[3,0],vectorB:[0,4],scalar:2,operation:'add'};
  const checked=labs.checkedState(config,state,1);state.vectorA[0]=9;assert.equal(checked.vectorA[0],3);
});

test('选区绑定原文码点，不能替换引文、理由越界或共享可变草稿', () => {
  const config={type:'annotation_lab',mode:'selection',fragments:['甲🌱','乙，丙。']};
  const entry={start:1,end:3,label:1,reason:'文字理由'};
  const state={schemaVersion:1,contentVersion:1,selections:[entry]};
  const checked=labs.checkedState(config,state,1);
  entry.reason='后来更改';assert.equal(checked.selections[0].reason,'文字理由');
  assert.equal(labs.calculate(config,checked,1).annotations[0].text,'🌱乙');
  for(const invalid of [{...entry,start:-1},{...entry,end:7},{...entry,end:1},{...entry,label:0},
    {...entry,text:'替换引文'},{...entry,reason:'汉'.repeat(161)}])
    assert.throws(()=>labs.calculate(config,{...state,selections:[invalid]},1));
  assert.throws(()=>labs.calculate(config,{...state,selections:Array(17).fill(entry)},1));
  assert.throws(()=>labs.calculate(config,{...state,fragments:['替换','原文']},1));
});

test('恒加速度运动满足速度位移关系，转向时路程仍递增', () => {
  for(const velocity of [-6,0,6])for(const acceleration of [-2,0,2]) {
    let previousDistance=0;
    for(let tick=0;tick<=12;tick++) {
      const state={schemaVersion:1,contentVersion:1,initialPositionM:5,initialVelocityMps:velocity,
        accelerationMps2:acceleration,timeSeconds:tick*.5};
      const result=labs.calculate({type:'motion_lab'},state,1),v=result.velocityMps;
      assert.ok(Math.abs(v*v-velocity*velocity-2*acceleration*result.displacementM)<1e-9);
      assert.ok(result.travelDistanceM+1e-9>=Math.abs(result.displacementM));
      assert.ok(result.travelDistanceM+1e-9>=previousDistance);previousDistance=result.travelDistanceM;
    }
  }
  for(const bad of [{timeSeconds:-1},{accelerationMps2:11},{initialVelocityMps:true}])
    assert.throws(()=>labs.validate({type:'motion_lab',...bad}));
});

test('二分查找不改变数组，目标存在时每步仍在候选区间，重复值和缺失目标可结束', () => {
  for(let length=2;length<=8;length++)for(let target=0;target<=12;target++) {
    const values=Array.from({length},(_,index)=>Math.floor(index/2)*2);
    const config={type:'algorithm_lab',algorithm:'binary_search',values,target};
    let total=length;
    for(let step=0;step<=total;step++) {
      const result=labs.calculate(config,{schemaVersion:1,contentVersion:1,step},1);
      total=result.totalSteps;assert.deepEqual(result.array,values);
      if(result.finished) {
        assert.equal(result.foundIndex!==null,values.includes(target));
        if(result.foundIndex!==null)assert.equal(values[result.foundIndex],target);
      } else if(values.includes(target))assert.ok(values.slice(result.low,result.high+1).includes(target));
    }
  }
  for(const bad of [{values:[3,1],target:1},{},{values:[1,2],target:3,step:3},{values:[1,2],target:true}])
    assert.throws(()=>labs.validate({type:'algorithm_lab',algorithm:'binary_search',...bad}));
  const config={type:'algorithm_lab',algorithm:'binary_search',values:[1,2],target:2};
  assert.throws(()=>labs.checkedState(config,{schemaVersion:1,contentVersion:1,step:0,target:1},1));
});

test('前后端共用标准案例，探索不伪造正式成绩', () => {
  for (const item of fixtures.cases) {
    const result = labs.calculate(item.config,item.state,1);
    for (const [key,value] of Object.entries(item.expected)) {
      if (typeof value === 'number') assert.ok(Math.abs(result[key]-value)<1e-9);
      else assert.deepEqual(result[key],value);
    }
    assert.equal(result.scoringEligible,false);
    assert.throws(()=>labs.checkedState(item.config,{...item.state,score:100},1));
    assert.throws(()=>labs.checkedState(item.config,item.state,2));
  }
});
test('数轴拒绝逆序、闭合无穷端、伪造归属及非法数值，镜像保持归属和长度', () => {
  for(const bad of [{lower:3,upper:2},{lower:null,lowerClosed:true},{upper:null,upperClosed:true},
    {lowerClosed:1},{point:null},{point:20.5},{lower:.1}])assert.throws(()=>labs.validate({type:'number_line',...bad}));
  for(const lower of [null,-2,0])for(const upper of [0,2,null])
    for(const lowerClosed of [false,true])for(const upperClosed of [false,true]) {
      if((lower===null&&lowerClosed)||(upper===null&&upperClosed))continue;
      for(const point of [-3,-2,-.5,0,.5,2,3]) {
        const state={schemaVersion:1,contentVersion:1,lower,upper,lowerClosed,upperClosed,point};
        const result=labs.calculate({type:'number_line'},state,1);
        const reflected=labs.calculate({type:'number_line'},{...state,lower:upper===null?null:-upper,
          upper:lower===null?null:-lower,lowerClosed:upperClosed,upperClosed:lowerClosed,point:-point},1);
        assert.equal(result.containsPoint,reflected.containsPoint);
        assert.equal(result.length,reflected.length);
        assert.throws(()=>labs.checkedState({type:'number_line'},{...state,containsPoint:true},1));
      }
    }
});

test('零电阻、未知公式及超范围状态必须拒绝', () => {
  for(const config of [{type:'circuit_lab',r1:0},{type:'circuit_lab',voltage:true},{type:'circuit_lab',mode:'script'},
    {type:'function_lab',coefficients:[1,.1,0]},{type:'chemical_balance',reactionId:'__proto__'},{type:'chemical_balance',coefficients:[0,1,1]}])
    assert.throws(()=>labs.validate(config));
});

test('定义域不能由学生状态替换，视窗不裁剪数学值域', () => {
  const config={type:'function_lab',domain:{min:2,max:4},xMin:-5,xMax:5};
  for(const domain of [null,[],{min:1,max:1},{min:3,max:2},{min:true,max:2},{min:-21,max:1},{min:0,max:2,open:true}])
    assert.throws(()=>labs.validate({...config,domain}));
  const state={schemaVersion:1,contentVersion:1,coefficients:[1,0,0]};
  assert.throws(()=>labs.calculate(config,{...state,domain:{min:0,max:4}},1));
  assert.throws(()=>labs.calculate(config,{...state,functionAnalysis:{range:{min:0,max:1}}},1));
  for(const [xMin,xMax] of [[5,10],[-5,2],[4,5]])assert.throws(()=>labs.validate({...config,xMin,xMax}));
  assert.doesNotThrow(()=>labs.validate({...config,xMin:3,xMax:5}));
  assert.deepEqual(labs.calculate({type:'function_lab',xMin:-1,xMax:1},state,1).functionAnalysis.range,{min:0,max:null});
});

test('闭区间上的每个抽样值均在值域内，零点符合原函数', () => {
  for(const coefficients of [[1,-2,0],[-2,3,1],[0,-1,2],[0,0,3],[.5,-1,-2]]){
    const result=labs.calculate({type:'function_lab',domain:{min:-3,max:4}},
      {schemaVersion:1,contentVersion:1,coefficients},1).functionAnalysis;
    const [a,b,c]=coefficients;
    for(let i=0;i<=140;i++){
      const x=-3+i*.05,y=a*x*x+b*x+c;
      assert.ok(y>=result.range.min-1e-9&&y<=result.range.max+1e-9);
    }
    for(const x of result.zeros.values){assert.ok(x>=-3&&x<=4);assert.ok(Math.abs(a*x*x+b*x+c)<1e-9);}
  }
});
test('六科拒绝伪造资料、隐藏结构、原文替换及越界步数', () => {
  for(const config of [{type:'cell_lab',cellType:'animal',selectedPart:'chloroplast'},
    {type:'history_lab',datasetId:'invented'},{type:'history_lab',order:['launch','launch','return']},
    {type:'climate_lab',datasetId:'Beijing'},{type:'climate_lab',month:0},
    {type:'annotation_lab',fragments:['a','b'],marks:[1,4]},
    {type:'sentence_lab',fragments:['a','b'],roles:[1]},
    {type:'algorithm_lab',values:[2,1],step:2},{type:'algorithm_lab',values:[true,1]}]) assert.throws(()=>labs.validate(config));
  const config={type:'annotation_lab',fragments:['原文','片段']};
  assert.throws(()=>labs.calculate(config,{schemaVersion:1,contentVersion:1,marks:[1,0],fragments:['替换','原文']},1));
  const state={schemaVersion:1,contentVersion:1,marks:[1,0]},checked=labs.checkedState(config,state,1);
  state.marks[0]=3;assert.equal(checked.marks[0],1);
});
test('冒泡排序的全步重放保留多重集，每步只可能交换相邻两项，最终有序', () => {
  // 穷举含重复值的四项数组，验证整体性质，而非复制排序公式作为期望。
  for(let encoded=0;encoded<256;encoded++) {
    const values=Array.from({length:4},(_,index)=>(encoded>>(index*2))&3),config={type:'algorithm_lab',values};let previous=values;
    for(let step=0;step<=6;step++) {
      const result=labs.calculate(config,{schemaVersion:1,contentVersion:1,step},1),changed=result.array.flatMap((value,index)=>value!==previous[index]?[index]:[]);
      assert.deepEqual([...result.array].sort((a,b)=>a-b),[...values].sort((a,b)=>a-b));
      assert.ok(changed.length===0||(changed.length===2&&changed[1]===changed[0]+1));
      previous=result.array;
      if(step===6)assert.deepEqual(result.array,[...values].sort((a,b)=>a-b));
    }
  }
});

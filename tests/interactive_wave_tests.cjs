'use strict';
const test=require('node:test');
const assert=require('node:assert/strict');
const labs=require('../public/interactive-labs.js');
const config={type:'wave_lab'};
const state={schemaVersion:1,contentVersion:1,amplitudeM:1,waveSpeedMps:2,periodSeconds:2,timeSeconds:0,probeXM:1};
const near=(a,b)=>assert.ok(Math.abs(a-b)<1e-10,`${a} != ${b}`);

test('固定质点每四分之一周期的位移和振动速度区别于波速',()=>{
  const expected=[[0,1,0],[.5,0,-Math.PI],[1,-1,0],[1.5,0,Math.PI],[2,1,0]];
  for(const [timeSeconds,position,velocity] of expected){
    const actual=labs.calculate(config,{...state,timeSeconds},1);
    near(actual.probeDisplacementM,position);near(actual.particleVelocityMps,velocity);
    assert.equal(actual.particleFixedXM,1);assert.equal(actual.wavelengthM,4);assert.equal(actual.frequencyHz,.5);
    assert.equal(actual.scoringEligible,false);assert.equal(actual.propagationDirection,'positive_x');
  }
});

test('波形的时空周期和向右传播恒等式成立，振幅不改变设定波速',()=>{
  for(const speed of [2,4])for(const period of [1,1.5,2])for(const time of [0,.5,1])for(const x of [.5,1,2]){
    const input={...state,waveSpeedMps:speed,periodSeconds:period,timeSeconds:time,probeXM:x};
    const original=labs.calculate(config,input,1),wavelength=speed*period;
    const later=labs.calculate(config,{...input,timeSeconds:time+period},1);
    const shifted=labs.calculate(config,{...input,probeXM:x+wavelength},1);
    const traveled=labs.calculate(config,{...input,probeXM:x+speed*.25,timeSeconds:time+.25},1);
    for(const actual of [later,shifted,traveled])near(actual.probeDisplacementM,original.probeDisplacementM);
    const doubled=labs.calculate(config,{...input,amplitudeM:2},1);
    near(doubled.probeDisplacementM,2*original.probeDisplacementM);
    assert.equal(doubled.wavelengthM,original.wavelengthM);assert.equal(doubled.waveSpeedMps,speed);
  }
  const flat=labs.calculate(config,{...state,amplitudeM:0},1);
  assert.equal(flat.visibleOscillation,false);assert.equal(flat.probeDisplacementM,0);assert.equal(flat.particleVelocityMps,0);
});

test('波动拒绝越界、错误步长、伪造结果和陈旧版本',()=>{
  for(const patch of [{amplitudeM:-.5},{amplitudeM:2.5},{amplitudeM:.1},{waveSpeedMps:0},{waveSpeedMps:1.5},
    {periodSeconds:0},{periodSeconds:4.5},{timeSeconds:8.25},{timeSeconds:.1},{probeXM:12.5},
    {probeXM:true},{amplitudeM:null},{periodSeconds:NaN},{timeSeconds:'1'}]){
    assert.throws(()=>labs.validate({...config,...patch}));assert.throws(()=>labs.calculate(config,{...state,...patch},1));
  }
  for(const patch of [{score:100},{wavelengthM:2},{particleVelocityMps:2},{schemaVersion:true},{contentVersion:2}])
    assert.throws(()=>labs.calculate(config,{...state,...patch},1));
  const missing={...state};delete missing.probeXM;assert.throws(()=>labs.calculate(config,missing,1));
  assert.throws(()=>labs.calculate(config,state,0));
  const copied=labs.checkedState(config,state,1);copied.amplitudeM=2;assert.equal(state.amplitudeM,1);
});

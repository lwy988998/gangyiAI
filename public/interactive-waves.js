/* 理想简谐横波：波形向右传播，固定质点只沿竖直方向振动。 */
(function(root,factory){
  const api=factory(root);
  if(typeof module==='object'&&module.exports)module.exports=api;
  else root.GangyiInteractiveWaves=api;
})(typeof globalThis==='object'?globalThis:this,function(root){
  'use strict';
  const fields={amplitudeM:[0,2,.5,1],waveSpeedMps:[1,5,1,2],periodSeconds:[1,4,.5,2],timeSeconds:[0,8,.25,0],probeXM:[0,12,.5,1]};
  function parameter(value,limits){
    const [low,high,step]=limits;
    if(typeof value!=='number'||!Number.isFinite(value)||value<low||value>high||!Number.isInteger(value/step))
      throw new Error('波动参数超出当前范围或步长。');
    return value;
  }
  function validate(source){
    if(!source||source.type!=='wave_lab'||(source.schemaVersion!==undefined&&source.schemaVersion!==1))
      throw new Error('波动组件类型或版本无效。');
    const config={type:'wave_lab',schemaVersion:1};
    for(const [name,limits] of Object.entries(fields))config[name]=parameter(Object.hasOwn(source,name)?source[name]:limits[3],limits);
    return config;
  }
  function checkedState(source,state,contentVersion){
    const config=validate(source),names=['schemaVersion','contentVersion',...Object.keys(fields)];
    if(!Number.isSafeInteger(contentVersion)||contentVersion<1||!state||Array.isArray(state)||
      state.schemaVersion!==1||state.contentVersion!==contentVersion||Object.keys(state).length!==names.length||
      Object.keys(state).some(key=>!names.includes(key)))throw new Error('波动探索状态已变化或含未经确认的结果。');
    const result={schemaVersion:1,contentVersion};
    for(const [name,limits] of Object.entries(fields))result[name]=parameter(state[name],limits);
    return result;
  }
  const clean=value=>Math.abs(value)<1e-12?0:value;
  function displacement(state,x,time){return clean(state.amplitudeM*Math.sin(2*Math.PI*(x/(state.waveSpeedMps*state.periodSeconds)-time/state.periodSeconds)));}
  function calculate(source,state,contentVersion){
    const value=checkedState(source,state,contentVersion),period=value.periodSeconds;
    const phase=2*Math.PI*(value.probeXM/(value.waveSpeedMps*period)-value.timeSeconds/period);
    return {...value,kind:'exploration',scoringEligible:false,inputSource:'browser_report',resultSource:'program_calculation',
      model:'ideal_right_traveling_sinusoidal_transverse_wave',wavelengthM:value.waveSpeedMps*period,frequencyHz:1/period,
      propagationDirection:'positive_x',probeDisplacementM:displacement(value,value.probeXM,value.timeSeconds),
      particleVelocityMps:clean(-value.amplitudeM*2*Math.PI/period*Math.cos(phase)),particleFixedXM:value.probeXM,visibleOscillation:value.amplitudeM>0};
  }
  function mount(host,source,options={}){
    const config=validate(source),contentVersion=options.contentVersion;
    // 配置中的 type 不是学生状态的一部分。
    const newState=()=>{const {type,...value}=config;return checkedState(config,{...value,contentVersion},contentVersion);};
    let state=newState(),restored=false,invalid=false,disposed=false,running=false,raf=0,previousStamp=null,displayTime=state.timeSeconds;
    if(options.state){try{state=checkedState(config,options.state,contentVersion);displayTime=state.timeSeconds;restored=true;}catch(_){invalid=true;}}
    const doc=host.ownerDocument,controller=new AbortController();
    const node=(tag,text='',className='')=>{const item=doc.createElement(tag);item.textContent=text;item.className=className;return item;};
    const svg=(tag,attrs={})=>{const item=doc.createElementNS('http://www.w3.org/2000/svg',tag);for(const [key,value] of Object.entries(attrs))item.setAttribute(key,value);return item;};
    const listen=(target,type,callback,capture=false)=>target.addEventListener(type,callback,{signal:controller.signal,capture});
    const frame=node('section','','il-lab');frame.dataset.labType='wave_lab';
    frame.append(node('p','播放后观察 P 点：波形向右移动，P 点的横坐标保持不变。暂停或调整时刻后，可以对照同一瞬间。','il-instruction'));
    const playback=node('div','','il-actions'),play=node('button','播放','dc-button');play.type='button';play.setAttribute('aria-pressed','false');playback.append(play);frame.append(playback);
    const diagram=node('div','','il-lab-diagram'),plot=svg('svg',{viewBox:'0 0 600 260',role:'img','aria-label':'向右传播的简谐横波与固定质点 P'});
    const axes=svg('g'),wave=svg('path',{fill:'none',stroke:'var(--dc-accent,#3d61ce)','stroke-width':2.5}),guide=svg('path',{fill:'none',stroke:'currentColor','stroke-dasharray':'4 4',opacity:.5});
    const probe=svg('circle',{r:6,fill:'currentColor',stroke:'var(--dc-surface,#fff)','stroke-width':2}),probeLabel=svg('text');probeLabel.textContent='P';
    plot.append(axes,guide,wave,probe,probeLabel);diagram.append(plot);frame.append(diagram);
    const readout=node('p','','il-lab-result');readout.setAttribute('role','status');readout.setAttribute('aria-live','polite');frame.append(readout);
    const controls=node('div','','il-lab-controls'),bindings=[];
    const labels={amplitudeM:'振幅 A（m）',waveSpeedMps:'设定波速 v（m/s）',periodSeconds:'周期 T（s）',timeSeconds:'时刻 t（s）',probeXM:'质点 P 的固定位置（m）'};
    for(const [name,limits] of Object.entries(fields)){
      const row=node('label','','il-lab-control'),input=node('input'),output=node('output');input.type='range';
      [input.min,input.max,input.step]=limits.slice(0,3).map(String);input.setAttribute('aria-label',labels[name]);
      row.append(node('span',labels[name]),input,output);controls.append(row);
      bindings.push(()=>{const value=name==='timeSeconds'?Math.round(displayTime*4)/4:state[name];input.value=String(value);output.textContent=String(value);});
      listen(input,'input',()=>{const next=parameter(input.valueAsNumber,limits);pause(false);state[name]=next;displayTime=state.timeSeconds;paint();notify();});
    }
    frame.append(controls,node('p','理想模型：y=A sin[2π(x/(vT)−t/T)]，波长 λ=vT，频率 f=1/T。设定波速保持不变时，改变周期会改变波长；改变振幅不会改变设定波速。','il-note'));
    const note=node('p',invalid?'课件已更新，已载入当前参数。':restored?'已恢复上次探索，播放保持暂停。':'播放不自动开始；暂停后保存当前时刻，时刻按 0.25 s 保存。','il-note');frame.append(note);
    const actions=node('div','','il-actions'),reset=node('button','重置探索','dc-button');reset.type='button';
    listen(reset,'click',()=>{pause(false);state=newState();displayTime=state.timeSeconds;paint();notify();note.textContent='已恢复本次示例。';});actions.append(reset);
    if(options.onExplain){const explain=node('button','请老师解释当前结果','dc-button');explain.type='button';
      listen(explain,'click',async()=>{pause();explain.disabled=true;try{await options.onExplain(getState());}catch(_){note.textContent='解释请求未完成，可以稍后重试。';}finally{if(!disposed)explain.disabled=false;}});actions.append(explain);}
    frame.append(actions,node('p','当前显示 0—12 m 范围内的理想简谐横波，不模拟波源启动、反射、干涉、衰减或能量。','il-note'));
    const link=node('a','模型依据：OpenStax 16.2');link.href='https://openstax.org/books/university-physics-volume-1/pages/16-2-mathematics-of-waves';link.target='_blank';link.rel='noopener noreferrer';frame.append(link);host.append(frame);
    let width=600;
    const x=value=>42+value*(width-76)/12,y=value=>135-value*45,format=value=>Number(value.toPrecision(5)).toString();
    function setupAxes(){
      width=Math.max(320,Math.min(600,diagram.clientWidth||600));plot.setAttribute('viewBox',`0 0 ${width} 260`);axes.replaceChildren();
      const text=(left,top,value,anchor='middle')=>{const item=svg('text',{x:left,y:top,'text-anchor':anchor});item.textContent=value;axes.append(item);};
      for(let i=0;i<=12;i+=2){axes.append(svg('path',{d:`M${x(i)} ${y(-2)}V${y(2)}`,stroke:'currentColor',opacity:.12}));text(x(i),y(0)+21,String(i));}
      for(const i of [-2,-1,0,1,2]){axes.append(svg('path',{d:`M${x(0)} ${y(i)}H${x(12)}`,stroke:'currentColor',opacity:i===0?.45:.12}));if(i)text(x(0)-14,y(i)+4,String(i));}
      text(x(0)-4,25,'y / m');text(x(12),y(-2)+24,'x / m','end');text(width/2,23,'波形向右传播');paint();
    }
    function paint(){
      bindings.forEach(bind=>bind());const points=[];
      for(let i=0;i<=480;i++)points.push(`${i?'L':'M'}${x(i/40).toFixed(2)},${y(displacement(state,i/40,displayTime)).toFixed(2)}`);
      wave.setAttribute('d',points.join(' '));const px=x(state.probeXM),py=y(displacement(state,state.probeXM,displayTime));
      guide.setAttribute('d',`M${px} ${y(-2)}V${y(2)}`);probe.setAttribute('cx',px);probe.setAttribute('cy',py);
      probeLabel.setAttribute('x',state.probeXM>11?px-10:px+10);probeLabel.setAttribute('y',py-12);probeLabel.setAttribute('text-anchor',state.probeXM>11?'end':'start');
      const result=calculate(config,getState(),contentVersion);
      readout.textContent=`t=${format(displayTime)} s，λ=${format(result.wavelengthM)} m，f=${format(result.frequencyHz)} Hz。P 的横坐标始终为 ${state.probeXM} m，当前竖直位移约 ${format(displacement(state,state.probeXM,displayTime))} m。`+(state.amplitudeM===0?'振幅为 0，图像为平衡直线，不能从图像辨认波长。':'');
      play.textContent=running?'暂停':displayTime>=8?'重播':'播放';play.setAttribute('aria-pressed',String(running));
    }
    function getState(){return checkedState(config,{...state,timeSeconds:Math.round(displayTime*4)/4},contentVersion);}
    function notify(){options.onChange?.(getState());}
    function pause(save=true){
      if(!running)return;running=false;root.cancelAnimationFrame(raf);raf=0;previousStamp=null;
      state.timeSeconds=Math.round(displayTime*4)/4;displayTime=state.timeSeconds;readout.setAttribute('aria-live','polite');paint();if(save)notify();
    }
    function tick(stamp){
      if(disposed||!running)return;
      if(previousStamp!==null)displayTime=Math.min(8,displayTime+Math.min(.1,(stamp-previousStamp)/1000));
      previousStamp=stamp;if(displayTime>=8){pause();return;}paint();raf=root.requestAnimationFrame(tick);
    }
    listen(play,'click',()=>{if(running){pause();return;}if(displayTime>=8){displayTime=0;state.timeSeconds=0;}running=true;readout.setAttribute('aria-live','off');previousStamp=null;paint();raf=root.requestAnimationFrame(tick);});
    listen(doc,'visibilitychange',()=>{if(doc.hidden)pause();});listen(root,'pagehide',()=>pause(),true);
    setupAxes();let previousWidth=diagram.clientWidth;
    const observer=root.ResizeObserver?new root.ResizeObserver(()=>{if(!disposed&&diagram.clientWidth&&diagram.clientWidth!==previousWidth){previousWidth=diagram.clientWidth;setupAxes();}}):null;observer?.observe(diagram);
    return {getState,destroy(){disposed=true;running=false;root.cancelAnimationFrame(raf);controller.abort();observer?.disconnect();frame.remove();}};
  }
  return Object.freeze({validate,checkedState,calculate,mount});
});

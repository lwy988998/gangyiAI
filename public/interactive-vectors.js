/* 平面向量的受控计算与探索，结果和方向边界与服务端一致。 */
(function(root,factory){
  const api=factory(root);
  if(typeof module==='object'&&module.exports)module.exports=api;
  else root.GangyiInteractiveVectors=api;
})(typeof globalThis==='object'?globalThis:this,function(root){
  'use strict';
  const fields=['vectorA','vectorB','scalar','operation'];
  function number(value,bound){
    if(typeof value!=='number'||!Number.isFinite(value)||Math.abs(value)>bound||!Number.isInteger(value*2))
      throw new Error('向量参数超出范围，数值步长为0.5。');
    return value;
  }
  function validate(source){
    if(!source||source.type!=='vector_lab'||(source.schemaVersion!==undefined&&source.schemaVersion!==1))
      throw new Error('向量组件类型或版本无效。');
    const config={type:'vector_lab',schemaVersion:1};
    for(const [key,fallback] of [['vectorA',[3,0]],['vectorB',[0,4]]]){
      const value=Object.hasOwn(source,key)?source[key]:fallback;
      if(!Array.isArray(value)||value.length!==2)throw new Error('平面向量须有两个坐标。');
      config[key]=Array.from(value,item=>number(item,10));
    }
    config.scalar=number(Object.hasOwn(source,'scalar')?source.scalar:2,3);
    config.operation=Object.hasOwn(source,'operation')?source.operation:'add';
    if(!['add','subtract','scale_a'].includes(config.operation))throw new Error('向量只支持加、减与实数倍。');
    return config;
  }
  function checkedState(source,state,contentVersion){
    const config=validate(source);
    if(!Number.isSafeInteger(contentVersion)||contentVersion<1||!state||Array.isArray(state)||
      state.schemaVersion!==1||state.contentVersion!==contentVersion||
      Object.keys(state).length!==fields.length+2||fields.some(key=>!Object.hasOwn(state,key))||
      Object.keys(state).some(key=>!['schemaVersion','contentVersion',...fields].includes(key)))
      throw new Error('向量探索状态已变化或含有未经确认的结果，请重新探索。');
    const checked=validate({...config,...Object.fromEntries(fields.map(key=>[key,state[key]]))});
    return {schemaVersion:1,contentVersion,...Object.fromEntries(fields.map(key=>[key,checked[key]]))};
  }
  function calculate(source,state,contentVersion){
    const value=checkedState(source,state,contentVersion),[ax,ay]=value.vectorA,[bx,by]=value.vectorB;
    const vectorResult=value.operation==='scale_a'?[value.scalar*ax,value.scalar*ay]:
      [ax+(value.operation==='subtract'?-bx:bx),ay+(value.operation==='subtract'?-by:by)];
    const normA=Math.hypot(ax,ay),normB=Math.hypot(bx,by),dotProduct=ax*bx+ay*by,determinant=ax*by-ay*bx;
    const directionsDefined=normA>0&&normB>0;
    const angle=determinant===0?(dotProduct<0?180:0):Math.acos(Math.max(-1,Math.min(1,dotProduct/(normA*normB))))*180/Math.PI;
    return {...value,kind:'exploration',scoringEligible:false,inputSource:'browser_report',resultSource:'program_calculation',
      model:'euclidean_plane_vectors',vectorResult,normA,normB,resultNorm:Math.hypot(...vectorResult),
      dotProduct,determinant,directionsDefined,angleDegrees:directionsDefined?angle:null,
      parallelDirections:directionsDefined?determinant===0:null,perpendicularDirections:directionsDefined?dotProduct===0:null};
  }
  function mount(host,source,options={}){
    const config=validate(source),contentVersion=options.contentVersion;
    const initial=()=>checkedState(config,{schemaVersion:1,contentVersion,...Object.fromEntries(fields.map(key=>[key,config[key]]))},contentVersion);
    let state=initial(),restored=false,invalid=false,disposed=false;
    if(options.state){try{state=checkedState(config,options.state,contentVersion);restored=true;}catch(_){invalid=true;}}
    const doc=host.ownerDocument,controller=new AbortController();
    const node=(tag,text='',className='')=>{const item=doc.createElement(tag);item.textContent=text;item.className=className;return item;};
    const svg=(tag,attrs={})=>{const item=doc.createElementNS('http://www.w3.org/2000/svg',tag);Object.entries(attrs).forEach(([key,value])=>item.setAttribute(key,value));return item;};
    const listen=(target,type,callback)=>target.addEventListener(type,callback,{signal:controller.signal});
    const frame=node('section','','il-lab');frame.dataset.labType='vector_lab';
    frame.append(node('p','调整 A、B 的坐标，比较向量的加减、实数倍与数量积。箭头表示方向，长度表示模。','il-instruction'));
    const diagram=node('div','','il-lab-diagram'),plot=svg('svg',{viewBox:'0 0 600 280',role:'img','aria-label':'平面向量及运算结果'});
    diagram.append(plot);
    const readout=node('p','','il-lab-result');readout.setAttribute('role','status');readout.setAttribute('aria-live','polite');
    const controls=node('div','','il-lab-controls'),bindings=[];
    const choice=node('label','','il-lab-control'),select=node('select');select.setAttribute('aria-label','向量运算');
    for(const [value,text] of [['add','A + B'],['subtract','A − B'],['scale_a','kA']]){
      const option=node('option',text);option.value=value;select.append(option);
    }
    choice.append(node('span','运算'),select);controls.append(choice);bindings.push(()=>select.value=state.operation);
    const getState=()=>checkedState(config,state,contentVersion);
    const changed=()=>{update();options.onChange?.(getState());};
    listen(select,'change',()=>{state.operation=select.value;changed();});
    for(const [key,label,bound] of [['vectorA','A',10],['vectorB','B',10],['scalar','倍数 k',3]]){
      for(let index=0;index<(key==='scalar'?1:2);index++){
        const name=key==='scalar'?label:`${label} 的 ${index===0?'x':'y'} 坐标`;
        const row=node('label','','il-lab-control'),input=node('input'),output=node('output');
        input.type='range';input.min=String(-bound);input.max=String(bound);input.step='.5';input.setAttribute('aria-label',name);
        row.append(node('span',name),input,output);controls.append(row);
        bindings.push(()=>{const value=key==='scalar'?state.scalar:state[key][index];input.value=String(value);output.textContent=String(value);if(key==='scalar')row.hidden=state.operation!=='scale_a';});
        listen(input,'input',()=>{
          try{const value=number(input.valueAsNumber,bound);if(key==='scalar')state.scalar=value;else state[key][index]=value;changed();}
          catch(_){update();}
        });
      }
    }
    const note=node('p',invalid?'课件已更新，已载入当前向量。':restored?'已恢复上次探索。':'自由探索不会计入成绩。','il-note');
    frame.append(diagram,readout,controls,node('p','二维直角坐标，坐标 −10—10，倍数 −3—3，步长 0.5。图像等比例缩放；含零向量时不判断方向关系或夹角。','il-note'),note);
    const actions=node('div','','il-actions'),reset=node('button','重置探索','dc-button');reset.type='button';
    listen(reset,'click',()=>{state=initial();changed();note.textContent='已恢复本次示例。';});actions.append(reset);
    if(options.onExplain){const explain=node('button','请老师解释当前结果','dc-button');explain.type='button';
      listen(explain,'click',async()=>{explain.disabled=true;try{await options.onExplain(getState());}catch(_){note.textContent='解释请求未完成，可以稍后重试。';}finally{explain.disabled=false;}});actions.append(explain);}
    frame.append(actions);host.append(frame);
    const format=value=>Number(value.toPrecision(5)).toString();
    function update(){
      bindings.forEach(bind=>bind());const result=calculate(config,state,contentVersion);
      const expression=state.operation==='add'?'A + B':state.operation==='subtract'?'A − B':`${format(state.scalar)}A`;
      const relation=!result.directionsDefined?'含零向量，夹角和方向关系未定义':
        `夹角 ${format(result.angleDegrees)}°；`+(result.parallelDirections?(result.dotProduct>0?'同向平行':'反向平行'):result.perpendicularDirections?'垂直':'不平行也不垂直');
      readout.textContent=`A=(${state.vectorA.map(format).join(', ')})，B=(${state.vectorB.map(format).join(', ')})；`+
        `R=${expression}=(${result.vectorResult.map(format).join(', ')})。|A|=${format(result.normA)}，|B|=${format(result.normB)}，|R|=${format(result.resultNorm)}；A·B=${format(result.dotProduct)}；${relation}。`;
      plot.setAttribute('aria-label',readout.textContent);plot.replaceChildren();
      const width=Math.max(320,Math.min(600,diagram.clientWidth||600)),cx=width/2,cy=140;
      plot.setAttribute('viewBox',`0 0 ${width} 280`);
      const extent=Math.max(1,Math.ceil(Math.max(...state.vectorA.map(Math.abs),...state.vectorB.map(Math.abs),...result.vectorResult.map(Math.abs))));
      const unit=Math.min((width-110)/2,90)/extent,x=value=>cx+value*unit,y=value=>cy-value*unit;
      const text=(left,top,value,color='currentColor')=>{const item=svg('text',{x:left,y:top,fill:color,'text-anchor':'middle'});item.textContent=value;plot.append(item);};
      plot.append(svg('path',{d:`M${cx-110} ${cy}h220m-7 -4l7 4 -7 4M${cx} 255V25m-4 7l4 -7 4 7`,fill:'none',stroke:'currentColor',opacity:.55}));
      text(cx+118,cy+5,'x');text(cx+12,25,'y');text(cx-10,cy+19,'0');
      for(const value of [-extent,extent]){text(x(value),cy+20,String(value));text(cx-21,y(value)+4,String(value));}
      if(state.operation==='add')plot.append(svg('path',{d:`M${x(state.vectorA[0])} ${y(state.vectorA[1])}L${x(result.vectorResult[0])} ${y(result.vectorResult[1])}L${x(state.vectorB[0])} ${y(state.vectorB[1])}`,fill:'none',stroke:'currentColor','stroke-dasharray':'3 4',opacity:.5}));
      for(const [value,label,color,dash,offset] of [[state.vectorA,'A','var(--dc-accent,#3d61ce)','',-15],
        [state.vectorB,'B','currentColor','6 4',20],[result.vectorResult,'R','currentColor','',-29]]){
        const [vx,vy]=value,px=x(vx),py=y(vy),norm=Math.hypot(vx,vy);
        if(norm){
          const ux=vx/norm,uy=-vy/norm;
          plot.append(svg('path',{d:`M${cx} ${cy}L${px} ${py}`,fill:'none',stroke:color,'stroke-width':label==='R'?3:2,'stroke-dasharray':dash}));
          plot.append(svg('path',{d:`M${px-ux*8-uy*4} ${py-uy*8+ux*4}L${px} ${py}L${px-ux*8+uy*4} ${py-uy*8-ux*4}`,fill:'none',stroke:color,'stroke-width':2}));
        }else plot.append(svg('circle',{cx,cy,r:3,fill:color}));
        text(px,py+offset,label,color);
      }
    }
    update();
    let previousWidth=diagram.clientWidth;
    const observer=root.ResizeObserver?new root.ResizeObserver(()=>{if(!disposed&&diagram.clientWidth&&diagram.clientWidth!==previousWidth){previousWidth=diagram.clientWidth;update();}}):null;
    observer?.observe(diagram);
    return {getState,destroy(){disposed=true;controller.abort();observer?.disconnect();frame.remove();}};
  }
  return Object.freeze({validate,checkedState,calculate,mount});
});

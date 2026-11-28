/* 受控平面三角形探索：顶点可拖动，也可用键盘调整。 */
(function(root,factory){
  const api=factory(root);
  if(typeof module==='object'&&module.exports)module.exports=api;
  else root.GangyiInteractiveGeometry=api;
})(typeof globalThis==='object'?globalThis:this,function(root){
  'use strict';
  function coordinate(value){
    if(typeof value!=='number'||!Number.isFinite(value)||Math.abs(value)>10||!Number.isInteger(value*2))
      throw new Error('顶点坐标须在 −10 至 10 内，步长为 0.5。');
    return value;
  }
  function validate(source){
    if(!source||source.type!=='triangle_lab'||(source.schemaVersion!==undefined&&source.schemaVersion!==1))
      throw new Error('三角形组件类型或版本无效。');
    const points=Object.hasOwn(source,'points')?source.points:[[0,0],[4,0],[0,3]];
    if(!Array.isArray(points)||points.length!==3)throw new Error('三角形须提供三个平面顶点。');
    return {type:'triangle_lab',schemaVersion:1,points:Array.from(points,point=>{
      if(!Array.isArray(point)||point.length!==2)throw new Error('每个顶点须有两个坐标。');
      return Array.from(point,coordinate);
    })};
  }
  function checkedState(source,state,contentVersion){
    const config=validate(source);
    if(!Number.isSafeInteger(contentVersion)||contentVersion<1||!state||Array.isArray(state)||
      state.schemaVersion!==1||state.contentVersion!==contentVersion||
      Object.keys(state).length!==3||!Object.hasOwn(state,'points')||
      Object.keys(state).some(key=>!['schemaVersion','contentVersion','points'].includes(key)))
      throw new Error('三角形探索状态已变化或含未经确认的结果，请重新探索。');
    const checked=validate({...config,points:state.points});
    return {schemaVersion:1,contentVersion,points:checked.points};
  }
  function calculate(source,state,contentVersion){
    const value=checkedState(source,state,contentVersion),p=value.points;
    const distance2=(a,b)=>(p[a][0]-p[b][0])**2+(p[a][1]-p[b][1])**2;
    const ab=distance2(0,1),bc=distance2(1,2),ca=distance2(2,0);
    const cross=(p[1][0]-p[0][0])*(p[2][1]-p[0][1])-(p[1][1]-p[0][1])*(p[2][0]-p[0][0]);
    const validTriangle=cross!==0;
    let angleDegrees=null,sideKind=null,angleKind=null;
    if(validTriangle){
      const angle=(at,a,b)=>{
        const ux=p[a][0]-p[at][0],uy=p[a][1]-p[at][1],vx=p[b][0]-p[at][0],vy=p[b][1]-p[at][1];
        return Math.acos(Math.max(-1,Math.min(1,(ux*vx+uy*vy)/(Math.hypot(ux,uy)*Math.hypot(vx,vy)))))*180/Math.PI;
      };
      angleDegrees={A:angle(0,1,2),B:angle(1,0,2),C:angle(2,0,1)};
      sideKind=ab===bc&&bc===ca?'equilateral':ab===bc||bc===ca||ca===ab?'isosceles':'scalene';
      const ordered=[ab,bc,ca].sort((a,b)=>a-b),difference=ordered[0]+ordered[1]-ordered[2];
      angleKind=difference===0?'right':difference>0?'acute':'obtuse';
    }
    return {...value,kind:'exploration',scoringEligible:false,inputSource:'browser_report',resultSource:'program_calculation',
      model:'euclidean_plane_triangle',validTriangle,orientation:cross>0?'counterclockwise':cross<0?'clockwise':'collinear',
      area:Math.abs(cross)/2,sideLengths:{AB:Math.sqrt(ab),BC:Math.sqrt(bc),CA:Math.sqrt(ca)},
      perimeter:validTriangle?Math.sqrt(ab)+Math.sqrt(bc)+Math.sqrt(ca):null,angleDegrees,sideKind,angleKind};
  }
  function mount(host,source,options={}){
    const config=validate(source),contentVersion=options.contentVersion;
    const initial=()=>checkedState(config,{schemaVersion:1,contentVersion,points:config.points},contentVersion);
    let state=initial(),restored=false,invalid=false,disposed=false,drag=null,plotMapping=null;
    if(options.state){try{state=checkedState(config,options.state,contentVersion);restored=true;}catch(_){invalid=true;}}
    const doc=host.ownerDocument,controller=new AbortController();
    const node=(tag,text='',className='')=>{const item=doc.createElement(tag);item.textContent=text;item.className=className;return item;};
    const svg=(tag,attrs={})=>{const item=doc.createElementNS('http://www.w3.org/2000/svg',tag);Object.entries(attrs).forEach(([key,value])=>item.setAttribute(key,value));return item;};
    const listen=(target,type,callback)=>target.addEventListener(type,callback,{signal:controller.signal});
    const frame=node('section','','il-lab');frame.dataset.labType='triangle_lab';
    frame.append(node('p','拖动 A、B、C，或调整坐标，观察边长、角度和面积。三个点共线时不能组成三角形。','il-instruction'));
    const diagram=node('div','','il-lab-diagram'),plot=svg('svg',{viewBox:'0 0 600 340',role:'img','aria-label':'平面三角形及三个顶点'});
    plot.style.touchAction='none';diagram.append(plot);
    const readout=node('p','','il-lab-result');readout.setAttribute('role','status');readout.setAttribute('aria-live','polite');
    const controls=node('div','','il-lab-controls'),bindings=[];
    const getState=()=>checkedState(config,state,contentVersion);
    const changed=()=>{update();options.onChange?.(getState());};
    for(let point=0;point<3;point++)for(let axis=0;axis<2;axis++){
      const name=`${'ABC'[point]} 的 ${axis===0?'x':'y'} 坐标`,row=node('label','','il-lab-control'),input=node('input'),output=node('output');
      input.type='range';input.min='-10';input.max='10';input.step='.5';input.setAttribute('aria-label',name);
      row.append(node('span',name),input,output);controls.append(row);
      bindings.push(()=>{input.value=String(state.points[point][axis]);output.textContent=String(state.points[point][axis]);});
      listen(input,'input',()=>{try{state.points[point][axis]=coordinate(input.valueAsNumber);changed();}catch(_){update();}});
    }
    const note=node('p',invalid?'课件已更新，已载入当前顶点。':restored?'已恢复上次探索。':'自由探索不会计入成绩。','il-note');
    frame.append(diagram,readout,controls,node('p','平面直角坐标；各坐标范围 −10—10，步长 0.5。长度和角度按需要近似显示；共线或顶点重合时不判断三角形类型。此坐标网格不能画出精确的等边三角形。','il-note'),note);
    const actions=node('div','','il-actions'),reset=node('button','重置探索','dc-button');reset.type='button';
    listen(reset,'click',()=>{state=initial();changed();note.textContent='已恢复本次示例。';});actions.append(reset);
    if(options.onExplain){const explain=node('button','请老师解释当前结果','dc-button');explain.type='button';
      listen(explain,'click',async()=>{explain.disabled=true;try{await options.onExplain(getState());}catch(_){note.textContent='解释请求未完成，可以稍后重试。';}finally{explain.disabled=false;}});actions.append(explain);}
    frame.append(actions);host.append(frame);
    const format=value=>Number(value.toPrecision(5)).toString();
    function update(){
      bindings.forEach(bind=>bind());const result=calculate(config,state,contentVersion);
      const points=state.points.map((point,index)=>`${'ABC'[index]}=(${point.map(format).join(', ')})`).join('，');
      if(!result.validTriangle)readout.textContent=`${points}。三个顶点共线或重合，面积为 0；不能判断三角形边角类型。`;
      else {
        const side={equilateral:'等边',isosceles:'等腰',scalene:'不等边'}[result.sideKind];
        const angle={right:'直角',acute:'锐角',obtuse:'钝角'}[result.angleKind];
        readout.textContent=`${points}。${side}、${angle}三角形；面积 ${format(result.area)}，周长约 ${format(result.perimeter)}。`+
          `AB≈${format(result.sideLengths.AB)}，BC≈${format(result.sideLengths.BC)}，CA≈${format(result.sideLengths.CA)}；`+
          `∠A≈${format(result.angleDegrees.A)}°，∠B≈${format(result.angleDegrees.B)}°，∠C≈${format(result.angleDegrees.C)}°。`;
      }
      plot.setAttribute('aria-label',readout.textContent);plot.replaceChildren();
      const width=Math.max(320,Math.min(600,diagram.clientWidth||600)),cx=width/2,cy=170,unit=Math.min((width-80)/20,13);
      const x=value=>cx+value*unit,y=value=>cy-value*unit;
      plotMapping={cx,cy,unit};plot.setAttribute('viewBox',`0 0 ${width} 340`);
      const label=(left,top,text)=>{const item=svg('text',{x:left,y:top,fill:'currentColor','text-anchor':'middle'});item.textContent=text;plot.append(item);};
      for(let i=-10;i<=10;i+=2){
        plot.append(svg('path',{d:`M${x(i)} ${y(-10)}V${y(10)}M${x(-10)} ${y(i)}H${x(10)}`,fill:'none',stroke:'currentColor',opacity:i===0?0.45:0.12}));
        if(i!==0){label(x(i),cy+18,String(i));label(cx-16,y(i)+4,String(i));}
      }
      label(cx-12,cy+18,'0');label(x(10)+14,cy+4,'x');label(cx+12,y(10)-8,'y');
      plot.append(svg('polygon',{points:state.points.map(point=>`${x(point[0])},${y(point[1])}`).join(' '),
        fill:'var(--dc-accent,#3d61ce)','fill-opacity':result.validTriangle?0.13:0,stroke:'var(--dc-accent,#3d61ce)','stroke-width':2}));
      state.points.forEach((point,index)=>{
        const px=x(point[0]),py=y(point[1]);
        plot.append(svg('circle',{cx:px,cy:py,r:8,fill:'var(--dc-accent,#3d61ce)',stroke:'white','stroke-width':2}));
        // 透明命中区扩大触摸目标，不改变图形大小。
        plot.append(svg('circle',{cx:px,cy:py,r:20,fill:'transparent','data-point':index}));
        label(px+14,py-11,'ABC'[index]);
      });
    }
    function movePoint(event){
      if(!plotMapping)return;
      const matrix=plot.getScreenCTM();if(!matrix)return;
      const point=plot.createSVGPoint();point.x=event.clientX;point.y=event.clientY;
      const local=point.matrixTransform(matrix.inverse()),{cx,cy,unit}=plotMapping;
      const clamp=value=>Math.max(-10,Math.min(10,Math.round(value*2)/2));
      const next=[clamp((local.x-cx)/unit),clamp((cy-local.y)/unit)];
      if(next.some((value,index)=>value!==state.points[drag.index][index])){state.points[drag.index]=next;changed();}
    }
    listen(plot,'pointerdown',event=>{
      const target=event.target.closest?.('[data-point]');if(!target)return;
      drag={id:event.pointerId,index:Number(target.getAttribute('data-point'))};
      plot.setPointerCapture(event.pointerId);event.preventDefault();movePoint(event);
    });
    listen(plot,'pointermove',event=>{if(drag&&drag.id===event.pointerId)movePoint(event);});
    const finish=event=>{if(drag&&drag.id===event.pointerId)drag=null;};
    listen(plot,'pointerup',finish);listen(plot,'pointercancel',finish);listen(plot,'lostpointercapture',finish);
    update();
    let previousWidth=diagram.clientWidth;
    const observer=root.ResizeObserver?new root.ResizeObserver(()=>{if(!disposed&&diagram.clientWidth&&diagram.clientWidth!==previousWidth){previousWidth=diagram.clientWidth;update();}}):null;
    observer?.observe(diagram);
    return {getState,destroy(){disposed=true;controller.abort();observer?.disconnect();frame.remove();}};
  }
  return Object.freeze({validate,checkedState,calculate,mount});
});

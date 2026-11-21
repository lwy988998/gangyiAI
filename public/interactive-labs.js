/* 受控学科实验：JSXGraph函数图、理想直流电路、原子守恒配平。 */
(function (root, factory) {
  const api = factory(root);
  if (typeof module === 'object' && module.exports) module.exports = api;
  else root.GangyiInteractiveLabs = api;
})(typeof globalThis === 'object' ? globalThis : this, function (root) {
  'use strict';
  const subjects = typeof module === 'object' && module.exports ? require('./interactive-subjects.js') : root.GangyiInteractiveSubjects;
  const vectors = typeof module === 'object' && module.exports ? require('./interactive-vectors.js') : root.GangyiInteractiveVectors;
  const equations = typeof module === 'object' && module.exports ? require('./interactive-equations.js') : root.GangyiInteractiveEquations;
  const statistics = typeof module === 'object' && module.exports ? require('./interactive-statistics.js') : root.GangyiInteractiveStatistics;
  const types = ['function_lab', 'circuit_lab', 'chemical_balance', 'motion_lab', 'number_line', ...(vectors?['vector_lab']:[]), ...(equations?['equation_lab']:[]), ...(statistics?['statistics_lab']:[]), ...(subjects?.types || [])];
  const motionFields = ['initialPositionM','initialVelocityMps','accelerationMps2','timeSeconds'];
  const numberLineFields = ['lower','upper','lowerClosed','upperClosed','point'];
  const reactions = {
    water: {formulas:['H₂','O₂','H₂O'], atoms:[{H:2},{O:2},{H:2,O:1}], leftCount:2},
    methane: {formulas:['CH₄','O₂','CO₂','H₂O'], atoms:[{C:1,H:4},{O:2},{C:1,O:2},{H:2,O:1}], leftCount:2},
    iron_oxide: {formulas:['Fe','O₂','Fe₂O₃'], atoms:[{Fe:1},{O:2},{Fe:2,O:3}], leftCount:2}
  };
  function number(value, low, high, integer = false) {
    if (typeof value !== 'number' || !Number.isFinite(value) || value < low || value > high || (integer && !Number.isInteger(value)))
      throw new Error('实验参数超出当前模型的范围。');
    return value;
  }
  function validate(source) {
    if(source?.type==='statistics_lab'&&statistics)return statistics.validate(source);
    if(source?.type==='equation_lab'&&equations)return equations.validate(source);
    if(source?.type==='vector_lab'&&vectors)return vectors.validate(source);
    if (subjects?.types.includes(source?.type)) return subjects.validate(source);
    if (!source || !types.includes(source.type) || (source.schemaVersion !== undefined && source.schemaVersion !== 1))
      throw new Error('暂不支持这个互动实验。');
    const config = {type:source.type,schemaVersion:1};
    if (source.type === 'function_lab') {
      const values = source.coefficients ?? [1,0,0];
      if (!Array.isArray(values) || values.length !== 3) throw new Error('函数需要三个系数。');
      config.coefficients = values.map(value => { number(value,-10,10); if (!Number.isInteger(value*2)) throw new Error('系数步长为0.5。'); return value; });
      if (Object.hasOwn(source,'domain')) {
        const domain=source.domain;
        if(!domain || Array.isArray(domain) || typeof domain!=='object' || Object.keys(domain).length!==2 || !Object.hasOwn(domain,'min') || !Object.hasOwn(domain,'max'))
          throw new Error('函数定义域须为含 min、max 的闭区间。');
        config.domain={min:number(domain.min,-20,20),max:number(domain.max,-20,20)};
        if(config.domain.min>=config.domain.max)throw new Error('函数定义域下界须小于上界。');
      }
      config.xMin = number(source.xMin ?? config.domain?.min ?? -5,-20,20); config.xMax = number(source.xMax ?? config.domain?.max ?? 5,-20,20);
      if (config.xMin >= config.xMax) throw new Error('函数横轴范围无效。');
      if(config.domain&&(config.xMax<=config.domain.min||config.xMin>=config.domain.max))
        throw new Error('当前视窗无法显示定义域内的曲线，请调整横轴范围。');
    } else if (source.type === 'number_line') {
      for(const [key,fallback] of [['lower',-2],['upper',3],['point',0]]) {
        const value=Object.hasOwn(source,key)?source[key]:fallback;
        if(key!=='point'&&value===null){config[key]=null;continue;}
        config[key]=number(value,-20,20);
        if(!Number.isInteger(value*2))throw new Error('数轴数值步长为0.5。');
      }
      for(const key of ['lowerClosed','upperClosed']) {
        const value=Object.hasOwn(source,key)?source[key]:key==='upperClosed'&&config.upper!==null;
        if(typeof value!=='boolean')throw new Error('数轴端点的开闭须为布尔值。');
        config[key]=value;
      }
      if((config.lower!==null&&config.upper!==null&&config.lower>config.upper)||
        (config.lower===null&&config.lowerClosed)||(config.upper===null&&config.upperClosed))
        throw new Error('左端点不能大于右端点，无穷端不能闭合。');
    } else if (source.type === 'motion_lab') {
      for(const [key,fallback,min,max] of [['initialPositionM',0,-100,100],['initialVelocityMps',6,-20,20],
        ['accelerationMps2',-2,-10,10],['timeSeconds',0,0,20]])
        config[key]=number(Object.hasOwn(source,key)?source[key]:fallback,min,max);
    } else if (source.type === 'circuit_lab') {
      config.mode = source.mode ?? 'series';
      if (!['series','parallel'].includes(config.mode)) throw new Error('只支持两电阻串联或并联。');
      config.voltage = number(source.voltage ?? 6,1,24,true);
      config.r1 = number(source.r1 ?? 10,1,100,true); config.r2 = number(source.r2 ?? 20,1,100,true);
    } else {
      config.reactionId = source.reactionId ?? 'water';
      if (!Object.hasOwn(reactions,config.reactionId)) throw new Error('配平示例尚未注册。');
      const reaction = reactions[config.reactionId], values = source.coefficients ?? reaction.formulas.map(()=>1);
      if (!Array.isArray(values) || values.length !== reaction.formulas.length) throw new Error('配平系数数量不符。');
      config.coefficients = values.map(value=>number(value,1,8,true));
    }
    return config;
  }
  function initial(config, contentVersion) {
    if(config.type==='number_line')return {schemaVersion:1,contentVersion,...Object.fromEntries(numberLineFields.map(key=>[key,config[key]]))};
    if(config.type==='motion_lab')return {schemaVersion:1,contentVersion,...Object.fromEntries(motionFields.map(key=>[key,config[key]]))};
    return {schemaVersion:1,contentVersion,...(config.type==='circuit_lab' ?
      {mode:config.mode,voltage:config.voltage,r1:config.r1,r2:config.r2} : {coefficients:[...config.coefficients]})};
  }
  function checkedState(source, state, contentVersion) {
    if(source?.type==='statistics_lab'&&statistics)return statistics.checkedState(source,state,contentVersion);
    if(source?.type==='equation_lab'&&equations)return equations.checkedState(source,state,contentVersion);
    if(source?.type==='vector_lab'&&vectors)return vectors.checkedState(source,state,contentVersion);
    if (subjects?.types.includes(source?.type)) return subjects.checkedState(source,state,contentVersion);
    const config = validate(source), fields = config.type==='number_line'?numberLineFields:config.type==='motion_lab'?motionFields:config.type==='circuit_lab' ? ['mode','voltage','r1','r2'] : ['coefficients'];
    if (!Number.isSafeInteger(contentVersion) || contentVersion<1 || !state || state.schemaVersion!==1 || state.contentVersion!==contentVersion ||
      Object.keys(state).some(key=>!['schemaVersion','contentVersion',...fields].includes(key)) || fields.some(key=>!Object.hasOwn(state,key)))
      throw new Error('实验状态已过期或含有未经确认的结果，请重新探索。');
    return initial(validate({...config,...Object.fromEntries(fields.map(key=>[key,state[key]]))}),contentVersion);
  }
  // 数学定义域与可平移缩放的绘图视窗分别保存，值域由当前合法系数计算。
  function functionAnalysis(config) {
    const [a,b,c]=config.coefficients,domain=config.domain;
    const inside=x=>!domain||(x>=domain.min&&x<=domain.max),value=x=>(a*x+b)*x+c;
    const result={kind:a!==0?'quadratic':b!==0?'linear':'constant',
      domain:domain?{kind:'closed_interval',...domain}:{kind:'real'},range:{min:null,max:null},turningPoint:null,
      zeros:{kind:a===0&&b===0&&c===0?'all':'finite',values:[]}};
    if(a!==0){
      const x=-b/(2*a)||0,y=value(x);result.turningPoint={x,y,inDomain:inside(x)};
      if(!domain)result.range[a>0?'min':'max']=y;
      const discriminant=b*b-4*a*c;
      if(discriminant>=0){
        const root=Math.sqrt(discriminant);
        result.zeros.values=[...new Set([(-b-root)/(2*a),(-b+root)/(2*a)].filter(inside).map(x=>x||0))].sort((x,y)=>x-y);
      }
    } else if(b!==0){const x=-c/b||0;if(inside(x))result.zeros.values.push(x);}
    else result.range={min:c,max:c};
    if(domain){
      const values=[value(domain.min),value(domain.max)];
      if(result.turningPoint?.inDomain)values.push(result.turningPoint.y);
      result.range={min:Math.min(...values),max:Math.max(...values)};
    }
    return result;
  }
  function calculate(source, state, contentVersion) {
    if(source?.type==='statistics_lab'&&statistics)return statistics.calculate(source,state,contentVersion);
    if(source?.type==='equation_lab'&&equations)return equations.calculate(source,state,contentVersion);
    if(source?.type==='vector_lab'&&vectors)return vectors.calculate(source,state,contentVersion);
    if (subjects?.types.includes(source?.type)) return subjects.calculate(source,state,contentVersion);
    const config=validate(source), value=checkedState(config,state,contentVersion);
    const result={...value,kind:'exploration',scoringEligible:false,inputSource:'browser_report',resultSource:'program_calculation'};
    if(config.type==='function_lab') {
      const inDomain=x=>!config.domain||(x>=config.domain.min&&x<=config.domain.max);
      result.yAtZero=inDomain(0)?value.coefficients[2]:null; result.yAtOne=inDomain(1)?value.coefficients.reduce((sum,x)=>sum+x,0):null;
      result.functionAnalysis=functionAnalysis({...config,coefficients:value.coefficients});
    } else if(config.type==='number_line') {
      const {lower,upper,lowerClosed,upperClosed,point}=value;
      const equal=lower!==null&&upper!==null&&lower===upper,empty=equal&&!(lowerClosed&&upperClosed);
      Object.assign(result,{intervalKind:empty?'empty':equal?'singleton':lower===null&&upper===null?'real_line':
        lower===null?'left_ray':upper===null?'right_ray':'interval',
        containsPoint:!empty&&(lower===null||point>lower||(point===lower&&lowerClosed))&&
          (upper===null||point<upper||(point===upper&&upperClosed)),
        length:lower!==null&&upper!==null?upper-lower:null});
    } else if(config.type==='motion_lab') {
      const {initialPositionM:x,initialVelocityMps:v,accelerationMps2:a,timeSeconds:t}=value;
      const displacement=v*t+a*t*t/2,turningTime=a!==0?-v/a:-1;
      let distance=Math.abs(displacement);
      if(turningTime>0&&turningTime<t) {
        const turningDisplacement=v*turningTime+a*turningTime*turningTime/2;
        distance=Math.abs(turningDisplacement)+Math.abs(displacement-turningDisplacement);
      }
      Object.assign(result,{positionM:x+displacement,displacementM:displacement,velocityMps:v+a*t,
        speedMps:Math.abs(v+a*t),travelDistanceM:distance,model:'one_dimensional_constant_acceleration'});
    } else if(config.type==='circuit_lab') {
      const {voltage,r1,r2,mode}=value, series=mode==='series';
      result.equivalentResistance=series?r1+r2:r1*r2/(r1+r2);
      result.totalCurrent=voltage/result.equivalentResistance;
      result.current1=series?result.totalCurrent:voltage/r1; result.current2=series?result.totalCurrent:voltage/r2;
      result.voltage1=series?result.totalCurrent*r1:voltage; result.voltage2=series?result.totalCurrent*r2:voltage;
      result.model='ideal_dc_two_resistors';
    } else {
      const reaction=reactions[config.reactionId]; result.leftAtoms={}; result.rightAtoms={};
      reaction.atoms.forEach((atoms,index)=>Object.entries(atoms).forEach(([element,count])=>{
        const side=index<reaction.leftCount?result.leftAtoms:result.rightAtoms;
        side[element]=(side[element]||0)+count*value.coefficients[index];
      }));
      result.balanced=[...new Set([...Object.keys(result.leftAtoms),...Object.keys(result.rightAtoms)])].every(key=>result.leftAtoms[key]===result.rightAtoms[key]);
      result.reactionId=config.reactionId;
    }
    return result;
  }
  let boardCounter=0;
  function mount(host, source, options={}) {
    if(source?.type==='statistics_lab'&&statistics)return statistics.mount(host,source,options);
    if(source?.type==='equation_lab'&&equations)return equations.mount(host,source,options);
    if(source?.type==='vector_lab'&&vectors)return vectors.mount(host,source,options);
    if (subjects?.types.includes(source?.type)) return subjects.mount(host,source,options);
    const config=validate(source), contentVersion=options.contentVersion;
    let state=checkedState(config,initial(config,contentVersion),contentVersion), restored=false, invalid=false;
    if(options.state) { try { state=checkedState(config,options.state,contentVersion); restored=true; } catch(_){invalid=true;} }
    const doc=host.ownerDocument, controller=new AbortController();
    const node=(tag,text='',className='')=>{const value=doc.createElement(tag);value.textContent=text;value.className=className;return value;};
    const svg=(tag,attrs={})=>{const value=doc.createElementNS('http://www.w3.org/2000/svg',tag);Object.entries(attrs).forEach(([k,v])=>value.setAttribute(k,v));return value;};
    const listen=(target,type,callback)=>target.addEventListener(type,callback,{signal:controller.signal});
    const frame=node('section','','il-lab');frame.dataset.labType=config.type;
    const instructions={function_lab:'调整 a、b、c，观察 y = ax² + bx + c 的图像。图像可缩放和平移。',
      circuit_lab:'切换串联、并联，调整电源和电阻，比较总电流与支路电流。',chemical_balance:'调整各物质前的整数系数，使反应两侧每种元素的原子总数相等。'};
    instructions.motion_lab='调整初速度、加速度和时刻，比较位置、位移和路程。正方向沿位置轴增大的方向。';
    instructions.number_line='调整区间端点和开闭，移动观察点，比较它是否属于区间。空心圆不含端点，实心圆含端点。';
    frame.append(node('p',instructions[config.type],'il-instruction'));
    const diagram=node('div','','il-lab-diagram'),controls=node('div','','il-lab-controls'),readout=node('p','','il-lab-result');
    readout.setAttribute('role','status');readout.setAttribute('aria-live','polite');
    frame.append(diagram,readout,controls);
    const bindings=[]; let board=null,resize=null,disposed=false;
    const getState=()=>checkedState(config,state,contentVersion);
    const format=value=>Number(value.toPrecision(5)).toString();
    const parameter=(label,min,max,step,get,set,type='range')=>{
      const row=node('label','','il-lab-control'),name=node('span',label),input=node('input'),output=node('output');
      input.type=type;input.min=String(min);input.max=String(max);input.step=String(step);input.setAttribute('aria-label',label);
      row.append(name,input,output);controls.append(row);bindings.push(()=>{input.value=String(get());output.textContent=String(get());});
      listen(input,'input',()=>{try{set(number(input.valueAsNumber,min,max,type==='number'));update();options.onChange?.(getState());}catch(_){update();}});
    };
    let circuitSvg=null,atomTable=null,motionSvg=null,numberLineSvg=null;
    if(config.type==='function_lab') {
      diagram.id=`il-board-${++boardCounter}`;diagram.classList.add('il-function-board');diagram.setAttribute('aria-label','多项式函数图像');
      ['a','b','c'].forEach((label,index)=>parameter(label,-10,10,.5,()=>state.coefficients[index],value=>state.coefficients[index]=value));
    } else if(config.type==='number_line') {
      numberLineSvg=svg('svg',{viewBox:'0 0 600 170',role:'img','aria-label':'区间和观察点的数轴'});diagram.append(numberLineSvg);
      const change=(patch)=>{
        try {state=checkedState(config,{...state,...patch},contentVersion);update();note.textContent='自由探索不会计入成绩。';options.onChange?.(getState());}
        catch(error){note.textContent=error.message;update();}
      };
      for(const [key,title,closedKey] of [['lower','左端点','lowerClosed'],['upper','右端点','upperClosed']]) {
        const row=node('label','','il-lab-control'),select=node('select');select.setAttribute('aria-label',title+'类型');
        for(const [value,text] of [['open','不含端点'],['closed','含端点'],['infinite',key==='lower'?'负无穷':'正无穷']]) {
          const option=node('option',text);option.value=value;select.append(option);
        }
        row.append(node('span',title+'类型'),select);controls.append(row);
        bindings.push(()=>select.value=state[key]===null?'infinite':state[closedKey]?'closed':'open');
        listen(select,'change',()=>{
          const fallback=key==='lower'?Math.min(-2,state.upper??3):Math.max(3,state.lower??-2);
          change({[key]:select.value==='infinite'?null:state[key]??fallback,[closedKey]:select.value==='closed'});
        });
        const valueRow=node('label','','il-lab-control'),input=node('input');input.type='number';input.min='-20';input.max='20';input.step='.5';
        input.setAttribute('aria-label',title+'数值');valueRow.append(node('span',title+'数值'),input);controls.append(valueRow);
        bindings.push(()=>{input.disabled=state[key]===null;input.value=state[key]===null?'':String(state[key]);});
        listen(input,'change',()=>change({[key]:input.valueAsNumber}));
      }
      parameter('观察点',-20,20,.5,()=>state.point,value=>state.point=value);
    } else if(config.type==='motion_lab') {
      motionSvg=svg('svg',{viewBox:'0 0 600 280',role:'img','aria-label':'匀变速直线运动的位置与时间图像'});diagram.append(motionSvg);
      for(const [label,key,min,max] of [['初位置 / m','initialPositionM',-100,100],['初速度 / m/s','initialVelocityMps',-20,20],
        ['加速度 / m/s²','accelerationMps2',-10,10],['时刻 / s','timeSeconds',0,20]])
        parameter(label,min,max,.5,()=>state[key],value=>state[key]=value);
    } else if(config.type==='circuit_lab') {
      const row=node('label','','il-lab-control'),label=node('span','连接方式'),select=node('select');select.setAttribute('aria-label','连接方式');
      for(const [value,text] of [['series','串联'],['parallel','并联']]){const option=node('option',text);option.value=value;select.append(option);}
      row.append(label,select);controls.append(row);bindings.push(()=>select.value=state.mode);
      listen(select,'change',()=>{state.mode=select.value;update();options.onChange?.(getState());});
      circuitSvg=svg('svg',{viewBox:'0 0 420 230',role:'img','aria-label':'两电阻理想直流电路'});diagram.append(circuitSvg);
      parameter('电源电压 / V',1,24,1,()=>state.voltage,value=>state.voltage=value);
      parameter('电阻 R₁ / Ω',1,100,1,()=>state.r1,value=>state.r1=value);
      parameter('电阻 R₂ / Ω',1,100,1,()=>state.r2,value=>state.r2=value);
    } else {
      const reaction=reactions[config.reactionId];atomTable=node('table','','il-atom-table');atomTable.append(node('caption','两侧各元素的原子总数'));diagram.append(atomTable);
      reaction.formulas.forEach((label,index)=>parameter(label+' 的系数',1,8,1,()=>state.coefficients[index],value=>state.coefficients[index]=value,'number'));
    }
    const note=node('p',invalid?'实验内容已更新，已载入当前示例。':restored?'已恢复上次探索。':'自由探索不会计入成绩。','il-note');
    const boundary={function_lab:'支持常数、一次和二次多项式。横轴视窗可缩放和平移，不改变数学定义域；观察图像不能替代证明。',circuit_lab:'理想电源与纯电阻模型，不包含导线电阻、温度、短路、交流或元件损耗。',
      chemical_balance:'这里只比较原子总数，不表示化学键、反应条件或反应速率；配平成功不等于已经提交作答。'};
    boundary.motion_lab='一维恒定加速度模型。位置图显示 0—20 秒，观察时刻内的路程按运动方向分别累计。参数变化代表另一组运动条件。';
    boundary.number_line='数值范围 −20—20，步长 0.5；无穷端用箭头表示。两端相等时，仅同时闭合才保留一个点，其他情况为空集。';
    frame.append(node('p',boundary[config.type],'il-note'),note);
    const actions=node('div','','il-actions'),reset=node('button','重置探索','dc-button');reset.type='button';
    listen(reset,'click',()=>{state=initial(config,contentVersion);update();note.textContent='已恢复本次示例。';options.onChange?.(getState(),{kind:'reset'});});actions.append(reset);
    if(options.onExplain){const explain=node('button','请老师解释当前结果','dc-button');explain.type='button';
      listen(explain,'click',async()=>{explain.disabled=true;try{await options.onExplain(getState());}catch(_){note.textContent='解释请求未完成，可以稍后重试。';}finally{explain.disabled=false;}});actions.append(explain);}
    frame.append(actions);host.append(frame);
    function update(){
      bindings.forEach(bind=>bind());const result=calculate(config,state,contentVersion);
      if(config.type==='function_lab'){
        const [a,b,c]=state.coefficients;
        const analysis=result.functionAnalysis,range=analysis.range;
        const domainText=config.domain?`[${format(config.domain.min)}, ${format(config.domain.max)}]`:'全体实数';
        const rangeText=`${range.min===null?'(−∞':`[${format(range.min)}`}, ${range.max===null?'+∞)':`${format(range.max)}]`}`;
        const zerosText=analysis.zeros.kind==='all'?'定义域内任意 x':analysis.zeros.values.length?analysis.zeros.values.map(format).join('、'):'无';
        const samples=[0,1].map(x=>result[x===0?'yAtZero':'yAtOne']===null?`x=${x} 不在定义域内`:`x=${x} 时 y=${format(result[x===0?'yAtZero':'yAtOne'])}`).join('，');
        readout.textContent=`y = ${a}x² ${b<0?'−':'+'} ${Math.abs(b)}x ${c<0?'−':'+'} ${Math.abs(c)}；定义域：${domainText}；值域：${rangeText}；零点：${zerosText}。${samples}`;
        board?.update();
      } else if(config.type==='number_line') {
        const {lower,upper,lowerClosed,upperClosed,point}=state;
        const expression=(lowerClosed?'[':'(')+(lower===null?'−∞':format(lower))+', '+(upper===null?'+∞':format(upper))+(upperClosed?']':')');
        const interval=result.intervalKind==='empty'?'∅（空集）':result.intervalKind==='singleton'?`{${format(lower)}}`:expression;
        readout.textContent=`区间 ${interval}；观察点 ${format(point)} ${result.containsPoint?'属于':'不属于'}该区间；长度 ${result.length===null?'无穷':format(result.length)}。`;
        numberLineSvg.setAttribute('aria-label',readout.textContent);numberLineSvg.replaceChildren();
        // 窄屏重排刻度坐标，避免把字体随整张宽图缩成难以辨认的小字。
        const width=Math.max(320,Math.min(600,diagram.clientWidth||600)),axisRight=width-20;
        numberLineSvg.setAttribute('viewBox',`0 0 ${width} 170`);
        const x=value=>40+(value+20)*(width-80)/40;
        numberLineSvg.append(svg('path',{d:`M20 75H${axisRight}M20 75l8 -5M20 75l8 5M${axisRight} 75l-8 -5M${axisRight} 75l-8 5`,fill:'none',stroke:'currentColor'}));
        for(const value of [-20,-10,0,10,20]) {
          numberLineSvg.append(svg('path',{d:`M${x(value)} 75v6`,stroke:'currentColor'}));
          const label=svg('text',{x:x(value),y:103,'text-anchor':'middle'});label.textContent=String(value);numberLineSvg.append(label);
        }
        if(result.intervalKind!=='empty') {
          const left=lower===null?20:x(lower),right=upper===null?axisRight:x(upper);
          numberLineSvg.append(svg('path',{d:`M${left} 60H${right}`,stroke:'currentColor','stroke-width':4}));
          for(const [value,closed,isLower] of [[lower,lowerClosed,true],[upper,upperClosed,false]]) {
            if(value===null) {
              const at=isLower?20:axisRight,sign=isLower?1:-1;
              numberLineSvg.append(svg('path',{d:`M${at+sign*9} 53L${at} 60l${sign*9} 7`,fill:'none',stroke:'currentColor','stroke-width':3}));
            } else {
              numberLineSvg.append(svg('circle',{cx:x(value),cy:60,r:5,fill:closed?'currentColor':'var(--dc-surface,#fff)',stroke:'currentColor','stroke-width':2}));
              const label=svg('text',{x:x(value),y:isLower?30:45,'text-anchor':'middle'});label.textContent=format(value);numberLineSvg.append(label);
            }
          }
        }
        numberLineSvg.append(svg('path',{d:`M${x(point)} 112v18m-5 -5l5 5 5 -5`,fill:'none',stroke:'currentColor','stroke-width':2}));
        const label=svg('text',{x:x(point),y:153,'text-anchor':'middle'});label.textContent=`观察点 ${format(point)}`;numberLineSvg.append(label);
      } else if(config.type==='motion_lab') {
        readout.textContent=`t=${format(state.timeSeconds)} s；位置 ${format(result.positionM)} m；位移 ${format(result.displacementM)} m；路程 ${format(result.travelDistanceM)} m；速度 ${format(result.velocityMps)} m/s；速率 ${format(result.speedMps)} m/s。`;
        motionSvg.replaceChildren();
        const position=t=>state.initialPositionM+state.initialVelocityMps*t+state.accelerationMps2*t*t/2;
        const extrema=[position(0),position(20)],turn=state.accelerationMps2!==0?-state.initialVelocityMps/state.accelerationMps2:-1;
        if(turn>0&&turn<20)extrema.push(position(turn));
        const minimum=Math.min(...extrema),maximum=Math.max(...extrema),padding=Math.max((maximum-minimum)*.1,1);
        const y=value=>240-(value-minimum+padding)/(maximum-minimum+2*padding)*210;
        const text=(x,y,value)=>{const label=svg('text',{x,y});label.textContent=value;motionSvg.append(label);};
        motionSvg.append(svg('path',{d:'M55 25V240H570',fill:'none',class:'il-chart-grid'}));
        text(8,20,'位置 / m');text(505,273,'时间 / s');
        for(const t of [0,5,10,15,20])text(55+t*25,260,String(t));
        for(const value of new Set([minimum,(minimum+maximum)/2,maximum]))text(2,y(value),format(value));
        const points=Array.from({length:101},(_,i)=>{const t=i/5;return (55+t*25)+','+y(position(t));}).join(' ');
        motionSvg.append(svg('polyline',{points,fill:'none',stroke:'currentColor','stroke-width':2}));
        motionSvg.append(svg('circle',{cx:55+state.timeSeconds*25,cy:y(result.positionM),r:6,fill:'currentColor'}));
      } else if(config.type==='circuit_lab'){
        readout.textContent=`等效电阻 ${format(result.equivalentResistance)} Ω；总电流 ${format(result.totalCurrent)} A；I₁=${format(result.current1)} A，I₂=${format(result.current2)} A；U₁=${format(result.voltage1)} V，U₂=${format(result.voltage2)} V`;
        circuitSvg.replaceChildren();
        const path=svg('path',{d:state.mode==='series'?'M65 155V65H140M200 65H250M310 65H365V190H65V155':'M65 155V65H140M200 65H365V190H65V155M65 155H140M200 155H365',class:'il-circuit-wire'});circuitSvg.append(path);
        const positions=state.mode==='series'?[[140,53],[250,53]]:[[140,53],[140,143]];
        positions.forEach(([x,y],index)=>{circuitSvg.append(svg('rect',{x,y,width:60,height:24,class:'il-circuit-resistor'}));const text=svg('text',{x:x+30,y:y-8,'text-anchor':'middle'});text.textContent=`R${index+1}=${state[index?'r2':'r1']}Ω`;circuitSvg.append(text);});
        circuitSvg.append(svg('circle',{cx:65,cy:133,r:22,class:'il-circuit-source'}));const voltage=svg('text',{x:65,y:138,'text-anchor':'middle'});voltage.textContent=state.voltage+'V';circuitSvg.append(voltage);
      } else {
        const reaction=reactions[config.reactionId];
        readout.textContent=reaction.formulas.map((formula,index)=>(index===reaction.leftCount?'→ ':'')+state.coefficients[index]+formula).join(' + ').replace('+ →','→')+
          (result.balanced?'：原子数相等。':'：原子数还不相等。');
        atomTable.replaceChildren(node('caption','两侧各元素的原子总数'));const heading=node('tr');['元素','反应物侧','生成物侧','比较'].forEach(value=>heading.append(node('th',value)));atomTable.append(heading);
        [...new Set([...Object.keys(result.leftAtoms),...Object.keys(result.rightAtoms)])].forEach(element=>{const row=node('tr');
          [element,result.leftAtoms[element]||0,result.rightAtoms[element]||0,result.leftAtoms[element]===result.rightAtoms[element]?'相等':'不相等'].forEach(value=>row.append(node('td',String(value))));atomTable.append(row);});
      }
    }
    update();
    if(config.type==='number_line'&&root.ResizeObserver) {
      let previousWidth=diagram.clientWidth;
      resize=new root.ResizeObserver(()=>{
        if(!disposed&&diagram.clientWidth&&diagram.clientWidth!==previousWidth) {
          previousWidth=diagram.clientWidth;update();
        }
      });
      resize.observe(diagram);
    }
    // 课件先挂入阅读区，再初始化本地图形库；销毁时释放画板和尺寸监听。
    const frameRequest=root.requestAnimationFrame?.(()=>{
      if(disposed||config.type!=='function_lab')return;
      if(!root.JXG?.JSXGraph){note.textContent='函数图像暂未加载，仍可调整系数并查看数值。';return;}
      board=root.JXG.JSXGraph.initBoard(diagram.id,{boundingbox:[config.xMin,8,config.xMax,-8],axis:true,showCopyright:false,showNavigation:false,
        pan:{enabled:true},zoom:{enabled:true,wheel:true,needShift:true},resize:{enabled:false},keyboard:{enabled:true}});
      const graphParents=[x=>state.coefficients[0]*x*x+state.coefficients[1]*x+state.coefficients[2]];
      if(config.domain)graphParents.push(config.domain.min,config.domain.max);
      board.create('functiongraph',graphParents,{strokeColor:'#3d61ce',strokeWidth:3});
      diagram.dataset.plotReady='true';
      if(root.ResizeObserver){resize=new root.ResizeObserver(()=>{if(!disposed&&diagram.clientWidth)board.resizeContainer(diagram.clientWidth,diagram.clientHeight,true,true);});resize.observe(diagram);}
    });
    return {getState,destroy(){disposed=true;controller.abort();if(frameRequest)root.cancelAnimationFrame(frameRequest);resize?.disconnect();if(board)root.JXG.JSXGraph.freeBoard(board);frame.remove();}};
  }
  return Object.freeze({types:Object.freeze(types),validate,checkedState,calculate,mount});
});

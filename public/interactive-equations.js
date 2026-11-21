/* 一元一次方程的等价变形；只接受受限步骤，不解析或执行表达式。 */
(function(root,factory){
  const api=factory();
  if(typeof module==='object'&&module.exports)module.exports=api;
  else root.GangyiInteractiveEquations=api;
})(typeof globalThis==='object'?globalThis:this,function(){
  'use strict';
  const limit=1000000000000n;
  const integer=(value,bound)=>{
    if(!Number.isSafeInteger(value)||Math.abs(value)>bound)throw new Error('方程参数须为范围内整数。');
    return value;
  };
  const absolute=value=>value<0n?-value:value;
  function rational(n,d=1n){
    if(d===0n)throw new Error('分母不能为零。');
    if(d<0n){n=-n;d=-d;}
    let a=absolute(n),b=d;while(b){const r=a%b;a=b;b=r;}
    return {numerator:n/a,denominator:d/a};
  }
  function calculateRational(op,a,b){
    const x=a.numerator,y=a.denominator,u=b.numerator,v=b.denominator;
    let result;
    if(op==='add'||op==='subtract'){
      let q=y,r=v;while(r){const rest=q%r;q=r;r=rest;}
      const left=x*(v/q),right=u*(y/q),denominator=y*(v/q);
      if(absolute(left)>limit||absolute(right)>limit||denominator>limit)throw new Error('精确计算中间值超过10^12。');
      const numerator=left+(op==='add'?right:-right);
      if(absolute(numerator)>limit)throw new Error('精确计算中间值超过10^12。');
      result=rational(numerator,denominator);
    } else {
      const other=op==='divide'?rational(v,u):b;
      let p=absolute(x),q=other.denominator;while(q){const rest=p%q;p=q;q=rest;}
      let r=absolute(other.numerator),s=y;while(s){const rest=r%s;r=s;s=rest;}
      const numerator=(x/p)*(other.numerator/r),denominator=(y/r)*(other.denominator/p);
      if(absolute(numerator)>limit||denominator>limit)throw new Error('精确计算中间值超过10^12。');
      result=rational(numerator,denominator);
    }
    return result;
  }
  const serial=value=>({numerator:Number(value.numerator),denominator:Number(value.denominator)});
  const exact=value=>String(value.numerator)+(value.denominator===1n?'':'/'+value.denominator);
  function validate(source){
    if(!source||source.type!=='equation_lab'||(source.schemaVersion!==undefined&&source.schemaVersion!==1))throw new Error('方程组件类型或协议无效。');
    const result={type:'equation_lab',schemaVersion:1};
    for(const [key,fallback] of [['left',[2,3]],['right',[1,7]]]){
      const side=Object.hasOwn(source,key)?source[key]:fallback;
      if(!Array.isArray(side)||side.length!==2)throw new Error('方程每边须为[一次项系数,常数]。');
      result[key]=Array.from(side,value=>integer(value,20));
    }
    return result;
  }
  function checkedState(source,state,contentVersion){
    validate(source);
    if(!Number.isSafeInteger(contentVersion)||contentVersion<1||!state||Array.isArray(state)||Object.keys(state).length!==3||
      state.schemaVersion!==1||state.contentVersion!==contentVersion||!Array.isArray(state.steps)||state.steps.length>12)
      throw new Error('方程状态只接受当前版本的变形步骤，最多12步。');
    const steps=Array.from(state.steps,step=>{
      if(!step||Array.isArray(step)||Object.keys(step).length!==2||!Object.hasOwn(step,'op')||!Object.hasOwn(step,'value')||
        !['add_x','add_constant','multiply','divide'].includes(step.op))throw new Error('每步只接受两边相同的合法操作和整数参数。');
      const value=integer(step.value,10);
      if(['multiply','divide'].includes(step.op)&&value===0)throw new Error('保持同解的变形不能同乘零，也不能除以零。');
      return {op:step.op,value};
    });
    return {schemaVersion:1,contentVersion,steps};
  }
  function calculate(source,state,contentVersion){
    const config=validate(source),value=checkedState(config,state,contentVersion);
    const sides={left:config.left.map(n=>rational(BigInt(n))),right:config.right.map(n=>rational(BigInt(n)))};
    const history=[];
    for(const step of value.steps){
      for(const key of ['left','right'])for(let i=0;i<2;i++){
        if((step.op==='add_x'&&i!==0)||(step.op==='add_constant'&&i!==1))continue;
        const result=calculateRational(['add_x','add_constant'].includes(step.op)?'add':step.op,sides[key][i],rational(BigInt(step.value)));
        if(absolute(result.numerator)>1000000n||result.denominator>1000000n)throw new Error('变形后的分子或分母超过10^6，请撤销或使用更小的数。');
        sides[key][i]=result;
      }
      history.push({step:{...step},left:sides.left.map(serial),right:sides.right.map(serial),equivalent:true});
    }
    const coefficient=calculateRational('subtract',sides.left[0],sides.right[0]);
    const constant=calculateRational('subtract',sides.right[1],sides.left[1]);
    const solutionKind=coefficient.numerator!==0n?'unique':constant.numerator===0n?'all_real':'none';
    const solution=solutionKind==='unique'?calculateRational('divide',constant,coefficient):null;
    const isolated=(a,b)=>sides[a][0].numerator===1n&&sides[a][0].denominator===1n&&sides[a][1].numerator===0n&&sides[b][0].numerator===0n;
    return {...value,kind:'exploration',scoringEligible:false,inputSource:'browser_report',resultSource:'program_calculation',
      model:'linear_equation_equivalent_steps',left:sides.left.map(serial),right:sides.right.map(serial),history,
      solutionKind,solution:solution?serial(solution):null,solutionExact:solution?exact(solution):null,
      isolated:solutionKind==='unique'&&(isolated('left','right')||isolated('right','left')),equivalencePreserved:true,stepCount:value.steps.length};
  }
  function mount(host,source,options={}){
    const config=validate(source),version=options.contentVersion,initial=()=>({schemaVersion:1,contentVersion:version,steps:[]});
    let state=checkedState(config,initial(),version),restored=false,invalid=false;
    if(options.state){try{calculate(config,options.state,version);state=checkedState(config,options.state,version);restored=true;}catch(_){invalid=true;}}
    const doc=host.ownerDocument,controller=new AbortController();
    const node=(tag,text='',className='')=>{const item=doc.createElement(tag);item.textContent=text;item.className=className;return item;};
    const listen=(target,event,callback)=>target.addEventListener(event,callback,{signal:controller.signal});
    const frame=node('section','','il-lab');frame.dataset.labType='equation_lab';
    frame.append(node('p','选择两边相同的变形，观察等式怎样改变。先尝试把未知数留在一边，再把常数移到另一边。','il-instruction'));
    const original=node('p','','il-note'),equation=node('p','','il-equation-current'),status=node('p','','il-lab-result');
    equation.setAttribute('aria-label','当前方程');status.setAttribute('role','status');status.setAttribute('aria-live','polite');
    const controls=node('div','','il-lab-controls'),operation=node('select'),input=node('input');
    operation.setAttribute('aria-label','等式两边的操作');input.setAttribute('aria-label','操作数');input.type='number';input.min='-10';input.max='10';input.step='1';input.value='-1';
    for(const [value,label] of [['add_x','两边同加 kx'],['add_constant','两边同加 k'],['multiply','两边同乘 k'],['divide','两边同除以 k']]){
      const option=node('option',label);option.value=value;operation.append(option);
    }
    for(const [label,control] of [['操作',operation],['操作数 k',input]]){const row=node('label','','il-lab-control');row.append(node('span',label),control);controls.append(row);}
    const actions=node('div','','il-actions'),apply=node('button','应用到两边','dc-button'),undo=node('button','撤销上一步','dc-button'),reset=node('button','重置探索','dc-button');
    for(const button of [apply,undo,reset])button.type='button';actions.append(apply,undo,reset);
    const note=node('p',invalid?'方程内容已变化，已载入当前示例。':restored?'已恢复上次变形。':'自由探索不会计入成绩。','il-note'),history=node('ol','','il-equation-history');
    history.setAttribute('aria-label','等价变形记录');
    const display=r=>String(r.numerator)+(r.denominator===1?'':'/'+r.denominator);
    const side=values=>{
      const [a,b]=values;
      let expression=a.numerator===0?'':a.numerator===a.denominator?'x':a.numerator===-a.denominator?'−x':`(${display(a)})x`;
      if(b.numerator!==0)expression+=(expression?(b.numerator<0?' − ':' + '):b.numerator<0?'−':'')+display({numerator:Math.abs(b.numerator),denominator:b.denominator});
      return expression||'0';
    };
    const equationText=result=>`${side(result.left)} = ${side(result.right)}`;
    original.textContent='原方程：'+equationText(calculate(config,initial(),version));
    frame.append(original,equation,status,controls,actions,note,history,node('p','仅支持一元一次方程与受限等价变形。操作数是 −10—10 的整数，最多12步；同乘或同除必须使用非零数。计算保留精确分数。','il-note'));
    const changed=()=>{update();options.onChange?.(checkedState(config,state,version));};
    listen(apply,'click',()=>{
      try{
        const next={...state,steps:[...state.steps,{op:operation.value,value:input.valueAsNumber}]};
        calculate(config,next,version);state=checkedState(config,next,version);changed();note.textContent='两边应用了相同操作，解集保持。';
      }catch(error){note.textContent=error.message;}
    });
    listen(undo,'click',()=>{if(!state.steps.length)return;state={...state,steps:state.steps.slice(0,-1)};changed();note.textContent='已撤销最后一步。';});
    listen(reset,'click',()=>{state=initial();changed();note.textContent='已恢复原方程。';});
    if(options.onExplain){const explain=node('button','请老师解释当前结果','dc-button');explain.type='button';actions.append(explain);
      listen(explain,'click',async()=>{explain.disabled=true;try{await options.onExplain(checkedState(config,state,version));}catch(_){note.textContent='解释请求未完成，可以稍后重试。';}finally{explain.disabled=false;}});}
    function update(){
      const result=calculate(config,state,version);equation.textContent=equationText(result);
      status.textContent=`已变形 ${result.stepCount} / 12 步。`+(result.solutionKind==='all_real'?'左右相同，任意实数都满足。':
        result.solutionKind==='none'?'未知数项抵消后常数不同，当前方程无解。':result.isolated?`未知数已单独留在一边：x = ${result.solutionExact}。请用原方程核对。`:'解集保持，继续尝试把 x 单独留在一边。');
      undo.disabled=!state.steps.length;apply.disabled=state.steps.length>=12;history.replaceChildren();
      for(const record of result.history){
        const labels={add_x:`两边同加 ${record.step.value}x`,add_constant:`两边同加 ${record.step.value}`,multiply:`两边同乘 ${record.step.value}`,divide:`两边同除以 ${record.step.value}`};
        history.append(node('li',labels[record.step.op]+'；'+equationText(record)));
      }
    }
    host.append(frame);update();
    return {getState:()=>checkedState(config,state,version),destroy(){controller.abort();frame.remove();}};
  }
  return Object.freeze({validate,checkedState,calculate,mount});
});

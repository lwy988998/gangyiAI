/* 有限单变量数据探索；半整数先放大两倍计算，保留精确分数。 */
(function(root,factory){
  const api=factory();
  if(typeof module==='object'&&module.exports)module.exports=api;
  else root.GangyiInteractiveStatistics=api;
})(typeof globalThis==='object'?globalThis:this,function(){
  'use strict';
  function validate(source){
    if(!source||source.type!=='statistics_lab'||(source.schemaVersion!==undefined&&source.schemaVersion!==1))throw new Error('统计组件类型或协议无效。');
    const values=Object.hasOwn(source,'values')?source.values:[2,3,3,4,8];
    if(!Array.isArray(values)||values.length<1||values.length>32)throw new Error('需要1至32项数据，重复项也算一项。');
    for(const value of values)if(typeof value!=='number'||!Number.isFinite(value)||Math.abs(value)>100||!Number.isInteger(value*2))throw new Error('数据须为 −100 至 100 的数，步长0.5。');
    const varianceBasis=Object.hasOwn(source,'varianceBasis')?source.varianceBasis:'population';
    if(!['population','sample'].includes(varianceBasis))throw new Error('方差口径只支持总体或样本。');
    return {type:'statistics_lab',schemaVersion:1,values:[...values],varianceBasis};
  }
  function checkedState(source,state,contentVersion){
    const config=validate(source);
    if(!Number.isSafeInteger(contentVersion)||contentVersion<1||!state||Array.isArray(state)||Object.keys(state).length!==4||
      state.schemaVersion!==1||state.contentVersion!==contentVersion||!Object.hasOwn(state,'values')||!Object.hasOwn(state,'varianceBasis'))
      throw new Error('统计状态只接受当前版本的数据与方差口径，不能自报结果或成绩。');
    const checked=validate({...config,values:state.values,varianceBasis:state.varianceBasis});
    return {schemaVersion:1,contentVersion,values:checked.values,varianceBasis:checked.varianceBasis};
  }
  function fraction(numerator,denominator){
    let a=Math.abs(numerator),b=denominator;while(b){const next=a%b;a=b;b=next;}
    return {numerator:numerator/a,denominator:denominator/a};
  }
  const approximate=value=>value===null?null:value.numerator/value.denominator;
  const display=value=>value===null?'无定义':String(value.numerator)+(value.denominator===1?'':'/'+value.denominator);
  function calculate(source,input,contentVersion){
    const state=checkedState(source,input,contentVersion),doubled=state.values.map(value=>value*2).sort((a,b)=>a-b),n=doubled.length;
    const sum=doubled.reduce((a,b)=>a+b,0),sumSquares=doubled.reduce((a,b)=>a+b*b,0),counts=new Map();
    for(const value of doubled)counts.set(value,(counts.get(value)||0)+1);
    const mean=fraction(sum,2*n),median=n%2?fraction(doubled[(n-1)/2],2):fraction(doubled[n/2-1]+doubled[n/2],4);
    const centered=n*sumSquares-sum*sum,populationVariance=fraction(centered,4*n*n),sampleVariance=n>1?fraction(centered,4*n*(n-1)):null;
    const maxFrequency=Math.max(...counts.values()),frequencies=Array.from(counts,([value,count])=>({value:value/2,count}));
    return {...state,kind:'exploration',scoringEligible:false,inputSource:'browser_report',resultSource:'program_calculation',model:'finite_univariate_descriptive_statistics',
      count:n,sum:sum/2,minimum:doubled[0]/2,maximum:doubled[n-1]/2,range:(doubled[n-1]-doubled[0])/2,
      mean:approximate(mean),median:approximate(median),modeValues:frequencies.filter(item=>maxFrequency>1&&item.count===maxFrequency).map(item=>item.value),maxFrequency,frequencies,
      populationVariance:approximate(populationVariance),sampleVariance:approximate(sampleVariance),variance:approximate(state.varianceBasis==='sample'?sampleVariance:populationVariance),
      varianceDenominator:state.varianceBasis==='sample'?n-1:n,exact:{mean,median,populationVariance,sampleVariance}};
  }
  function mount(host,source,options={}){
    const config=validate(source),version=options.contentVersion,initial=()=>({schemaVersion:1,contentVersion:version,values:[...config.values],varianceBasis:config.varianceBasis});
    let state=checkedState(config,initial(),version),restored=false,invalid=false;
    if(options.state){try{state=checkedState(config,options.state,version);restored=true;}catch(_){invalid=true;}}
    const doc=host.ownerDocument,controller=new AbortController();let rowController=null;
    const node=(tag,text='',className='')=>{const item=doc.createElement(tag);item.textContent=text;item.className=className;return item;};
    const listen=(target,event,callback,signal=controller.signal)=>target.addEventListener(event,callback,{signal});
    const frame=node('section','','il-lab');frame.dataset.labType='statistics_lab';
    frame.append(node('p','改动一项数据，比较平均数、中位数、众数和方差怎样变化。图中每个圆点代表一项数据，重复项向上叠放。','il-instruction'));
    const chart=doc.createElementNS('http://www.w3.org/2000/svg','svg');chart.setAttribute('viewBox','0 0 400 230');chart.setAttribute('role','img');chart.setAttribute('aria-label','数据点图；精确数值与频数见下方频数表');chart.classList.add('il-statistics-chart');
    const result=node('div','','il-lab-result');result.setAttribute('role','status');result.setAttribute('aria-live','polite');
    const values=node('div','','il-statistics-values');values.setAttribute('aria-label','当前数据');
    const controls=node('div','','il-lab-controls'),basis=node('select'),newValue=node('input');
    basis.setAttribute('aria-label','方差口径');
    for(const [value,label] of [['population','总体方差（除以 n）'],['sample','校正样本方差（除以 n−1）']]){const option=node('option',label);option.value=value;basis.append(option);}
    newValue.type='number';newValue.min='-100';newValue.max='100';newValue.step='0.5';newValue.value='0';newValue.setAttribute('aria-label','新增数据');
    for(const [label,input] of [['方差口径',basis],['新增数据',newValue]]){const row=node('label','','il-lab-control');row.append(node('span',label),input);controls.append(row);}
    const actions=node('div','','il-actions'),add=node('button','添加数据','dc-button'),reset=node('button','重置探索','dc-button');
    for(const button of [add,reset])button.type='button';actions.append(add,reset);
    const note=node('p',invalid?'统计内容已变化，已载入当前示例。':restored?'已恢复上次数据与方差口径。':'自由探索不会计入成绩。','il-note');
    const details=node('details'),table=node('table');table.setAttribute('aria-label','数据频数表');details.append(node('summary','查看数据频数表'),table);
    frame.append(chart,result,values,controls,actions,note,details,node('p','总体方差将这组数据视为完整总体，除以 n；校正样本方差除以 n−1，只有一项数据时无定义。部分教材把除以 n 的描述统计量称为样本方差，这里按分母明确区分。仅作描述统计，不据此推断实际总体。每项限 −100—100，步长0.5，最多32项。所有项都只出现一次时，此处记为无众数。','il-note'));
    const changed=(next,rebuild=false)=>{
      state=checkedState(config,next,version);update(rebuild);options.onChange?.(checkedState(config,state,version));note.textContent='已按当前数据重新计算。';
    };
    listen(basis,'change',()=>changed({...state,varianceBasis:basis.value}));
    listen(add,'click',()=>{try{changed({...state,values:[...state.values,newValue.valueAsNumber]},true);}catch(error){note.textContent=error.message;}});
    listen(reset,'click',()=>{changed(initial(),true);note.textContent='已恢复本课原始数据。';});
    if(options.onExplain){const explain=node('button','请老师解释当前结果','dc-button');explain.type='button';actions.append(explain);
      listen(explain,'click',async()=>{explain.disabled=true;try{await options.onExplain(checkedState(config,state,version));}catch(_){note.textContent='解释请求未完成，可以稍后重试。';}finally{explain.disabled=false;}});}
    function rebuildRows(){
      rowController?.abort();rowController=new AbortController();values.replaceChildren();
      state.values.forEach((value,index)=>{
        const row=node('div','','il-statistics-row'),label=node('label',`第 ${index+1} 项`),input=node('input'),remove=node('button','删除','dc-button');
        input.type='number';input.min='-100';input.max='100';input.step='0.5';input.value=String(value);input.setAttribute('aria-label',`第 ${index+1} 项数据`);label.append(input);
        remove.type='button';remove.setAttribute('aria-label',`删除第 ${index+1} 项数据`);remove.disabled=state.values.length===1;
        listen(input,'change',()=>{try{const next=[...state.values];next[index]=input.valueAsNumber;changed({...state,values:next});}catch(error){input.value=String(state.values[index]);note.textContent=error.message;}},rowController.signal);
        listen(remove,'click',()=>{const next=state.values.filter((_,i)=>i!==index);changed({...state,values:next},true);values.querySelectorAll('input')[Math.min(index,next.length-1)]?.focus();},rowController.signal);
        row.append(label,remove);values.append(row);
      });
    }
    function draw(observation){
      chart.replaceChildren();
      const svg=(tag,attributes,text)=>{const item=doc.createElementNS(chart.namespaceURI,tag);for(const [key,value] of Object.entries(attributes))item.setAttribute(key,String(value));if(text!==undefined)item.textContent=text;chart.append(item);};
      const lo=observation.minimum,hi=observation.maximum,padding=lo===hi?1:Math.max(.5,(hi-lo)*.05),min=lo-padding,max=hi+padding;
      const x=value=>30+340*(value-min)/(max-min),gap=Math.min(13,145/observation.maxFrequency);
      // 点图高度随实际叠放层数变化，为课堂正文保留空间。
      const height=Math.max(105,70+gap*(observation.maxFrequency-1)),baseline=height-45;
      chart.setAttribute('viewBox',`0 0 400 ${height}`);
      svg('line',{x1:24,y1:baseline,x2:376,y2:baseline,stroke:'currentColor'});
      for(const value of [...new Set([lo,(lo+hi)/2,hi])]){svg('line',{x1:x(value),y1:baseline,x2:x(value),y2:baseline+6,stroke:'currentColor'});svg('text',{x:x(value),y:baseline+26,'text-anchor':'middle',fill:'currentColor','font-size':13},String(value));}
      for(const item of observation.frequencies)for(let i=0;i<item.count;i++)svg('circle',{cx:x(item.value),cy:baseline-10-i*gap,r:Math.min(4,gap*.4),fill:'currentColor'});
    }
    function update(rebuild=false){
      const observation=calculate(config,state,version);basis.value=state.varianceBasis;add.disabled=state.values.length===32;
      if(rebuild)rebuildRows();draw(observation);result.replaceChildren();
      for(const text of [`数据 ${observation.count} 项；平均数 ${display(observation.exact.mean)}；中位数 ${display(observation.exact.median)}。`,
        `众数：${observation.modeValues.length?observation.modeValues.join('、'):'无'}；最小值 ${observation.minimum}，最大值 ${observation.maximum}，极差 ${observation.range}。`,
        state.varianceBasis==='sample'?`校正样本方差（除以 n−1）：${display(observation.exact.sampleVariance)}${observation.count===1?'（n=1，不能除以零）':''}。`:`总体方差（除以 n）：${display(observation.exact.populationVariance)}。`])result.append(node('p',text));
      table.replaceChildren();const head=node('thead'),header=node('tr');for(const text of ['数值','频数']){const th=node('th',text);th.scope='col';header.append(th);}head.append(header);const body=node('tbody');
      for(const item of observation.frequencies){const row=node('tr');row.append(node('td',String(item.value)),node('td',String(item.count)));body.append(row);}table.append(head,body);
    }
    host.append(frame);update(true);
    return {getState:()=>checkedState(config,state,version),destroy(){rowController?.abort();controller.abort();frame.remove();}};
  }
  return Object.freeze({validate,checkedState,calculate,mount});
});

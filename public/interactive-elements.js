/* 元素选择、周期与族比较；原子组成只采用已注册的中性原子资料。 */
(function(root,factory){
  const api=factory(root);if(typeof module==='object'&&module.exports)module.exports=api;else root.GangyiInteractiveElements=api;
})(typeof globalThis==='object'?globalThis:this,function(root){
  'use strict';
  const data=typeof module==='object'&&module.exports?require('./periodic-elements-data.js'):root.GangyiPeriodicElements;
  function atomicNumber(value){if(!Number.isInteger(value)||value<1||value>118)throw new Error('原子序数须为 1 至 118 的整数。');return value;}
  function validate(source){
    if(!data)throw new Error('元素资料未加载，请重新打开当前页面。');
    if(!source||source.type!=='periodic_lab'||(source.schemaVersion!==undefined&&source.schemaVersion!==1)||
      (source.datasetId!==undefined&&source.datasetId!==data.id))throw new Error('元素组件类型、协议或资料版本无效。');
    const highlight=Object.hasOwn(source,'highlight')?source.highlight:'none';
    if(!['none','period','group'].includes(highlight))throw new Error('元素高亮只支持当前元素、同周期或同族。');
    return {type:'periodic_lab',schemaVersion:1,datasetId:data.id,
      selectedZ:atomicNumber(Object.hasOwn(source,'selectedZ')?source.selectedZ:11),
      comparisonZ:atomicNumber(Object.hasOwn(source,'comparisonZ')?source.comparisonZ:17),highlight};
  }
  function checkedState(source,state,contentVersion){
    validate(source);const fields=['schemaVersion','contentVersion','datasetId','selectedZ','comparisonZ','highlight'];
    if(!Number.isSafeInteger(contentVersion)||contentVersion<1||!state||Array.isArray(state)||state.schemaVersion!==1||
      state.contentVersion!==contentVersion||Object.keys(state).length!==fields.length||Object.keys(state).some(key=>!fields.includes(key)))
      throw new Error('元素探索状态已变化或含未经确认的原子组成。');
    const {type,...value}=validate({type:'periodic_lab',...state});return {...value,contentVersion};
  }
  function facts(z){
    const item=data.elements[atomicNumber(z)-1];
    return {atomicNumber:z,symbol:item.symbol,name:item.name,period:item.period,group:item.group,series:item.series,
      neutralShells:item.neutralShells?[...item.neutralShells]:null,protons:z,electrons:z,neutrons:null,atomKind:'neutral_ground_state'};
  }
  function calculate(source,state,contentVersion){
    const value=checkedState(source,state,contentVersion),selected=facts(value.selectedZ),comparison=facts(value.comparisonZ);
    const highlightedAtomicNumbers=data.elements.filter(item=>item.atomicNumber===value.selectedZ||
      (value.highlight==='period'&&item.period===selected.period)||
      (value.highlight==='group'&&selected.group!==null&&item.group===selected.group)).map(item=>item.atomicNumber);
    return {...value,kind:'exploration',scoringEligible:false,inputSource:'browser_report',resultSource:'program_calculation',
      factSource:'registered_pubchem_snapshot',model:'registered_periodic_table_neutral_atoms',selected,comparison,
      samePeriod:selected.period===comparison.period,sameGroup:selected.group===null||comparison.group===null?null:selected.group===comparison.group,
      highlightedAtomicNumbers,sourceUrl:data.sourceUrl,sourceSha256:data.sourceSha256};
  }
  function mount(host,source,options={}){
    const config=validate(source),version=options.contentVersion;
    const initial=()=>{const {type,...value}=config;return checkedState(config,{...value,contentVersion:version},version);};
    let state=initial(),restored=false,invalid=false,disposed=false;
    if(options.state){try{state=checkedState(config,options.state,version);restored=true;}catch(_){invalid=true;}}
    const doc=host.ownerDocument,controller=new AbortController();
    const node=(tag,text='',className='')=>{const item=doc.createElement(tag);item.textContent=text;item.className=className;return item;};
    const svg=(tag,attrs={})=>{const item=doc.createElementNS('http://www.w3.org/2000/svg',tag);for(const [name,value] of Object.entries(attrs))item.setAttribute(name,value);return item;};
    const listen=(target,event,callback)=>target.addEventListener(event,callback,{signal:controller.signal});
    const frame=node('section','','il-lab il-periodic');frame.dataset.labType='periodic_lab';
    frame.append(node('p','选择两个元素，比较周期、族和中性原子的组成。A 是当前元素，B 是比较元素；探索不会计入成绩。','il-instruction'));
    const controls=node('div','','il-lab-controls'),selected=node('select'),comparison=node('select'),highlight=node('select');
    const control=(title,input)=>{input.setAttribute('aria-label',title);const label=node('label','','il-lab-control');label.append(node('span',title),input);controls.append(label);};
    for(const select of [selected,comparison])for(const element of data.elements){const option=node('option',`${element.atomicNumber} · ${element.symbol} · ${element.name}`);option.value=String(element.atomicNumber);select.append(option);}
    for(const [value,title] of [['none','只看当前元素'],['period','高亮同周期'],['group','高亮同族']]){const option=node('option',title);option.value=value;highlight.append(option);}
    control('当前元素 A',selected);control('比较元素 B',comparison);control('周期表高亮',highlight);frame.append(controls);
    const readout=node('p','','il-lab-result');readout.setAttribute('role','status');readout.setAttribute('aria-live','polite');frame.append(readout);
    const table=node('table','','il-element-comparison'),caption=node('caption','中性原子比较'),head=node('thead'),headrow=node('tr');
    for(const title of ['项目','当前 A','比较 B']){const cell=node('th',title);cell.scope='col';headrow.append(cell);}head.append(headrow);table.append(caption,head);
    const body=node('tbody'),rows=[];
    for(const title of ['元素','原子序数 / 质子数','中性原子的电子数','周期','族或系列','电子层分布（前20号）','中子数']){
      const row=node('tr'),name=node('th',title),a=node('td'),b=node('td');name.scope='row';row.append(name,a,b);body.append(row);rows.push([a,b]);
    }
    table.append(body);frame.append(table,node('p','这里展示中性原子。中子数需要具体同位素，不能把相对原子质量直接当作质量数。','il-note'));
    const details=node('details','','il-atom-details'),summary=node('summary','查看当前元素的电子层示意'),atom=node('div','','il-atom-diagram');details.append(summary,atom);frame.append(details);
    frame.append(node('p','全表可横向滚动；也可用上方选择框定位。表内左右键按原子序数移动，上下键寻找同列元素。','il-note'));
    const scroll=node('div','','il-periodic-scroll');scroll.tabIndex=0;scroll.setAttribute('role','region');scroll.setAttribute('aria-label','元素周期表，可横向滚动');
    const grid=node('div','','il-periodic-grid');grid.setAttribute('role','group');grid.setAttribute('aria-label','元素选择');scroll.append(grid);frame.append(scroll);
    const cell=(title,row,column,span=1)=>{const item=node('span',title,'il-periodic-heading');item.style.gridRow=row;item.style.gridColumn=`${column} / span ${span}`;grid.append(item);return item;};
    cell('族',1,1);for(let group=1;group<=18;group++)cell(String(group),1,group+1);
    for(let period=1;period<=7;period++)cell(String(period),period+1,1);
    cell('57—71',7,4).title='镧系另列';cell('89—103',8,4).title='锕系另列';
    cell('镧系 · 第6周期',10,1,3);cell('锕系 · 第7周期',11,1,3);
    const buttons=new Map();
    for(const item of data.elements){const button=node('button','','il-element-button');button.type='button';button.dataset.atomicNumber=String(item.atomicNumber);
      button.style.gridRow=item.gridRow;button.style.gridColumn=item.gridColumn;button.append(node('span',String(item.atomicNumber),'il-element-number'),node('strong',item.symbol),node('span','','il-element-mark'));
      button.title=`${item.name} · 原子序数 ${item.atomicNumber}`;grid.append(button);buttons.set(item.atomicNumber,button);
      listen(button,'click',()=>{state.selectedZ=item.atomicNumber;update();notify();});
      listen(button,'keydown',event=>{
        let next;
        if(event.key==='ArrowRight')next=Math.min(118,item.atomicNumber+1);
        if(event.key==='ArrowLeft')next=Math.max(1,item.atomicNumber-1);
        if(['ArrowUp','ArrowDown'].includes(event.key)){
          const step=event.key==='ArrowUp'?-1:1;
          const candidates=data.elements.filter(other=>other.gridColumn===item.gridColumn&&(other.gridRow-item.gridRow)*step>0).sort((a,b)=>(a.gridRow-b.gridRow)*step);
          next=candidates[0]?.atomicNumber;
        }
        if(['Home','End'].includes(event.key)){
          const candidates=data.elements.filter(other=>other.gridRow===item.gridRow);
          next=event.ctrlKey?(event.key==='Home'?1:118):candidates[event.key==='Home'?0:candidates.length-1].atomicNumber;
        }
        if(next!==undefined){event.preventDefault();state.selectedZ=next;update();buttons.get(next).focus({preventScroll:true});notify();}
      });
    }
    const note=node('p',invalid?'课件或资料版本已更新，已载入当前选择。':restored?'已恢复上次元素比较。':'先比较 Na 与 Cl，再试着选择同周期或同族。','il-note');frame.append(note);
    const actions=node('div','','il-actions'),reset=node('button','重置探索','dc-button');reset.type='button';actions.append(reset);
    listen(reset,'click',()=>{state=initial();update();notify();note.textContent='已恢复本次示例。';});
    if(options.onExplain){const explain=node('button','请老师解释当前结果','dc-button');explain.type='button';actions.append(explain);
      listen(explain,'click',async()=>{explain.disabled=true;try{await options.onExplain(getState());}catch(_){note.textContent='解释请求未完成，可以稍后重试。';}finally{if(!disposed)explain.disabled=false;}});}
    frame.append(actions,node('p','镧系与锕系在本表单列，不在该系列成员上填写统一族号。不提供离子、同位素、化学键或反应预测。','il-note'));
    const link=node('a','元素事实来源：PubChem');link.href='https://pubchem.ncbi.nlm.nih.gov/ptable/';link.target='_blank';link.rel='noopener noreferrer';frame.append(link);host.append(frame);
    const groupText=item=>item.group===null?(item.series==='lanthanoid'?'镧系（族号未列）':'锕系（族号未列）'):`第 ${item.group} 族`;
    function update(){
      selected.value=String(state.selectedZ);comparison.value=String(state.comparisonZ);highlight.value=state.highlight;
      const result=calculate(config,state,version),a=result.selected,b=result.comparison;
      readout.textContent=`A：${a.symbol}（${a.name}），B：${b.symbol}（${b.name}）。`+(result.samePeriod?'在同一周期。':'不在同一周期。')+
        (result.sameGroup===null?'含单列系列元素，不作同族判断。':result.sameGroup?'在同一族。':'不在同一族。');
      for(const [column,item] of [[0,a],[1,b]]){
        const values=[`${item.symbol} · ${item.name}`,String(item.protons),String(item.electrons),`第 ${item.period} 周期`,groupText(item),item.neutralShells?item.neutralShells.join('，'):'尚未提供','未知：需指定同位素'];
        values.forEach((value,index)=>{rows[index][column].textContent=value;});
      }
      for(const item of data.elements){const button=buttons.get(item.atomicNumber),isSelected=item.atomicNumber===state.selectedZ,isComparison=item.atomicNumber===state.comparisonZ;
        button.classList.toggle('is-selected',isSelected);button.classList.toggle('is-comparison',isComparison);button.classList.toggle('is-related',result.highlightedAtomicNumbers.includes(item.atomicNumber));
        button.setAttribute('aria-pressed',String(isSelected));button.tabIndex=isSelected?0:-1;button.lastChild.textContent=isSelected&&isComparison?'A/B':isSelected?'A':isComparison?'B':'';
        button.setAttribute('aria-label',`${item.atomicNumber} ${item.symbol} ${item.name}${isSelected?'，当前元素 A':''}${isComparison?'，比较元素 B':''}`);
      }
      const selectedButton=buttons.get(state.selectedZ),left=selectedButton.offsetLeft,right=left+selectedButton.offsetWidth;
      if(left<scroll.scrollLeft)scroll.scrollLeft=Math.max(0,left-10);else if(right>scroll.scrollLeft+scroll.clientWidth)scroll.scrollLeft=right-scroll.clientWidth+10;
      atom.replaceChildren();
      if(!a.neutralShells){atom.append(node('p','此处电子层示意仅覆盖前 20 号中性原子，不把简化填层规则外推到所有元素。','il-note'));return;}
      const picture=svg('svg',{viewBox:'0 0 280 220',role:'img','aria-label':`${a.symbol} 中性原子电子层分布 ${a.neutralShells.join('、')}；非真实电子轨道`});
      a.neutralShells.forEach((count,index)=>{const radius=24+index*24;picture.append(svg('circle',{cx:140,cy:110,r:radius,fill:'none',stroke:'currentColor',opacity:.5}));
        for(let i=0;i<count;i++){const angle=-Math.PI/2+2*Math.PI*i/count;picture.append(svg('circle',{cx:140+radius*Math.cos(angle),cy:110+radius*Math.sin(angle),r:4,fill:'var(--dc-accent,#3d61ce)'}));}});
      const nucleus=svg('text',{x:140,y:115,'text-anchor':'middle'});nucleus.textContent=`+${a.protons}`;picture.append(nucleus);atom.append(picture,node('p','只示意各电子层的电子数；圆圈不是电子的真实运动轨道，也不表示真实尺寸。','il-note'));
    }
    function getState(){return checkedState(config,state,version);}function notify(){options.onChange?.(getState());}
    listen(selected,'change',()=>{state.selectedZ=atomicNumber(Number(selected.value));update();notify();});
    listen(comparison,'change',()=>{state.comparisonZ=atomicNumber(Number(comparison.value));update();notify();});
    listen(highlight,'change',()=>{state.highlight=highlight.value;update();notify();});update();
    return {getState,destroy(){disposed=true;controller.abort();frame.remove();}};
  }
  return Object.freeze({validate,checkedState,calculate,mount,data});
});

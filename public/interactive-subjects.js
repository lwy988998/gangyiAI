/* 六科学科互动：只处理受控参数，原文、事实和算法结果均从当前配置重新取得。 */
(function (root, factory) {
  const api = factory();
  if (typeof module === 'object' && module.exports) module.exports = api;
  else root.GangyiInteractiveSubjects = api;
})(typeof globalThis === 'object' ? globalThis : this, function () {
  'use strict';
  const types = ['cell_lab','history_lab','climate_lab','annotation_lab','sentence_lab','algorithm_lab'];
  const facts = {
    membrane:['细胞膜','选择性地调节物质进出；植物细胞也有细胞膜。'],
    cytoplasm:['细胞质','细胞膜以内、细胞核以外的区域，包含细胞质基质与多种细胞器。'],
    nucleus:['细胞核','含有大部分遗传物质；此处展示有细胞核的典型真核细胞。'],
    mitochondrion:['线粒体','参与细胞呼吸和ATP生成；植物细胞也有线粒体。'],
    wall:['细胞壁','位于细胞膜外，提供支持与保护；典型植物细胞具有细胞壁。'],
    chloroplast:['叶绿体','在绿色植物的光合细胞中参与光合作用；不是所有植物细胞都有叶绿体。'],
    vacuole:['中央液泡','成熟植物细胞中常见的大型液泡参与储存及维持膨压。']
  };
  const events = {
    launch:{label:'阿波罗11号发射',date:'1969-07-16'},
    landing:{label:'登月舱着陆月面',date:'1969-07-20'},
    return:{label:'指令舱返回地球溅落',date:'1969-07-24'}
  };
  // 自编教学数据，不能据此推断某个真实地区的气候类型。
  const temperatures = [2,4,8,14,19,23,26,25,21,15,9,4];
  const rainfall = [30,35,50,75,110,150,180,160,100,65,40,25];
  const labels = {annotation_lab:['未标注','意象','表达','疑问'],sentence_lab:['未标注','主语','谓语动词','宾语','修饰或状语']};
  const fields = type => ({cell_lab:['cellType','selectedPart'],history_lab:['order','selectedEvent'],climate_lab:['month','compareMonth'],
    annotation_lab:['marks'],sentence_lab:['roles'],algorithm_lab:['step']})[type];
  const stateFields = config => config.mode==='selection'?['selections']:fields(config.type);
  const parts = type => ['membrane','cytoplasm','nucleus','mitochondrion',...(type==='plant'?['wall','chloroplast','vacuole']:[])];
  function integer(value,low,high) {
    if (!Number.isSafeInteger(value) || value<low || value>high) throw new Error('互动参数须为范围内整数。');
    return value;
  }
  function fragments(value) {
    if (!Array.isArray(value) || value.length<2 || value.length>16) throw new Error('需要2至16个原文片段。');
    let total=0;
    for (const text of value) {
      if (typeof text!=='string' || !text.trim() || new TextEncoder().encode(text).length>240) throw new Error('文本片段为空或超过240字节。');
      total+=new TextEncoder().encode(text).length;
    }
    if (total>2400) throw new Error('文本总长度超过2400字节。');
    return [...value];
  }
  function annotationSelections(passage,selections) {
    const length=Array.from(passage).length;
    if(!Array.isArray(selections)||selections.length>16)throw new Error('原文选区最多16条。');
    return selections.map(item=>{
      if(!item||typeof item!=='object'||Object.keys(item).length!==4||
        !['start','end','label','reason'].every(key=>Object.hasOwn(item,key))||typeof item.reason!=='string')
        throw new Error('选区只接受起止位置、类别和阅读理由。');
      const start=integer(item.start,0,length-1),end=integer(item.end,start+1,length),label=integer(item.label,1,3);
      if(new TextEncoder().encode(item.reason).length>480)throw new Error('标注理由超过480字节。');
      return {start,end,label,reason:item.reason};
    });
  }
  function binarySearchSteps(values,target) {
    let low=0,high=values.length-1;
    const steps=[{low,high,comparedIndices:[],foundIndex:null,finished:false}];
    while(low<=high) {
      const middle=low+Math.floor((high-low)/2),found=values[middle]===target;
      if(!found) {
        if(values[middle]<target)low=middle+1;
        else high=middle-1;
      }
      steps.push({low,high,comparedIndices:[middle],foundIndex:found?middle:null,finished:found||low>high});
      if(found)break;
    }
    return steps;
  }
  function validate(source) {
    if (!source || !types.includes(source.type) || (source.schemaVersion!==undefined && source.schemaVersion!==1)) throw new Error('学科互动类型或版本无效。');
    const type=source.type, config={type,schemaVersion:1};
    if(type==='cell_lab') {
      config.cellType=source.cellType??'plant';config.selectedPart=source.selectedPart??'nucleus';
      if(!['plant','animal'].includes(config.cellType)||!parts(config.cellType).includes(config.selectedPart)) throw new Error('当前细胞示意中没有这个结构。');
    } else if(type==='history_lab') {
      config.datasetId=source.datasetId??'apollo11';
      if(config.datasetId!=='apollo11') throw new Error('历史资料集尚未核对。');
      const order=source.order??['landing','return','launch'];
      if(!Array.isArray(order)||order.length!==3||new Set(order).size!==3||order.some(id=>!Object.hasOwn(events,id))) throw new Error('历史事件重复或未知。');
      config.order=[...order];config.selectedEvent=source.selectedEvent??'landing';
      if(!order.includes(config.selectedEvent)) throw new Error('所选事件不在资料中。');
    } else if(type==='climate_lab') {
      config.datasetId=source.datasetId??'illustrative_north';
      if(config.datasetId!=='illustrative_north') throw new Error('气候数据集尚未注册。');
      config.month=integer(source.month??1,1,12);config.compareMonth=integer(source.compareMonth??7,1,12);
    } else if(type==='annotation_lab'||type==='sentence_lab') {
      config.fragments=fragments(source.fragments);
      const key=type==='annotation_lab'?'marks':'roles', values=source[key]??config.fragments.map(()=>0);
      if(!Array.isArray(values)||values.length!==config.fragments.length) throw new Error('标注数量须与原文片段一致。');
      config[key]=values.map(value=>integer(value,0,labels[type].length-1));
      if(type==='annotation_lab') {
        const mode=Object.hasOwn(source,'mode')?source.mode:'fragments';
        if(mode==='selection') {
          config.mode=mode;
          config.selections=annotationSelections(config.fragments.join(''),Object.hasOwn(source,'selections')?source.selections:[]);
        } else if(mode!=='fragments')throw new Error('阅读标注模式无效。');
      }
    } else {
      const values=source.values??[5,2,4,1];
      if(!Array.isArray(values)||values.length<2||values.length>8) throw new Error('算法数组需要2至8项。');
      config.values=values.map(value=>integer(value,0,99));
      const algorithm=source.algorithm??'bubble_sort';
      let maximum=values.length*(values.length-1)/2;
      if(algorithm==='binary_search') {
        if(values.some((value,index)=>index>0&&value<values[index-1]))throw new Error('二分查找需要升序排列的数组。');
        config.algorithm=algorithm;config.target=integer(source.target,0,99);
        maximum=binarySearchSteps(values,config.target).length-1;
      } else if(algorithm!=='bubble_sort')throw new Error('算法尚未注册。');
      config.step=integer(source.step??0,0,maximum);
    }
    return config;
  }
  function initial(config,version) {
    return {schemaVersion:1,contentVersion:version,...Object.fromEntries(stateFields(config).map(key=>[key,Array.isArray(config[key])?structuredClone(config[key]):config[key]]))};
  }
  function checkedState(source,state,version) {
    const config=validate(source), keys=stateFields(config);
    if(!Number.isSafeInteger(version)||version<1||!state||state.schemaVersion!==1||state.contentVersion!==version||
      Object.keys(state).some(key=>!['schemaVersion','contentVersion',...keys].includes(key))||keys.some(key=>!Object.hasOwn(state,key)))
      throw new Error('互动状态已过期或含有额外结果，请重新探索。');
    return initial(validate({...config,...Object.fromEntries(keys.map(key=>[key,state[key]]))}),version);
  }
  function calculate(source,input,version) {
    const config=validate(source), state=checkedState(config,input,version), type=config.type;
    const result={...state,kind:'exploration',scoringEligible:false,inputSource:'browser_report',resultSource:'program_calculation'};
    if(type==='cell_lab') {
      result.part=[...facts[state.selectedPart]];result.availableParts=parts(state.cellType);
    } else if(type==='history_lab') {
      result.dates=state.order.map(id=>events[id].date);result.chronological=result.dates.every((date,i)=>i===0||date>result.dates[i-1]);
      result.selectedFact={...events[state.selectedEvent]};result.sourceUrl='https://www.nasa.gov/mission/apollo-11/';
    } else if(type==='climate_lab') {
      const index=state.month-1, compare=state.compareMonth-1;
      result.temperatureC=temperatures[index];result.precipitationMm=rainfall[index];
      result.temperatureDifferenceC=temperatures[index]-temperatures[compare];result.precipitationDifferenceMm=rainfall[index]-rainfall[compare];
      result.dataKind='illustrative_not_observed';
    } else if(labels[type]) {
      if(config.mode==='selection') {
        const passage=Array.from(config.fragments.join(''));
        result.annotations=state.selections.map(item=>({start:item.start,end:item.end,text:passage.slice(item.start,item.end).join(''),label:labels[type][item.label],reason:item.reason}));
        result.interpretation='student_annotation_not_correctness';return result;
      }
      const key=type==='annotation_lab'?'marks':'roles';
      result.annotations=state[key].flatMap((value,index)=>value?[{index,text:config.fragments[index],label:labels[type][value]}]:[]);
      result.interpretation='student_annotation_not_correctness';
    } else if(config.algorithm==='binary_search') {
      const steps=binarySearchSteps(config.values,config.target);
      Object.assign(result,steps[state.step],{array:[...config.values],target:config.target,
        totalSteps:steps.length-1,algorithm:'binary_search_ascending'});
    } else {
      const values=[...config.values];let count=0;result.comparedIndices=[];result.swapped=false;
      for(let end=values.length-1;end>0;end--) for(let index=0;index<end;index++) {
        if(count++>=state.step)continue;
        const swap=values[index]>values[index+1];
        if(swap)[values[index],values[index+1]]=[values[index+1],values[index]];
        result.comparedIndices=[index,index+1];result.swapped=swap;
      }
      result.array=values;result.totalSteps=count;result.finished=state.step===count;result.algorithm='bubble_sort_ascending';
    }
    return result;
  }
  function mount(host,source,options={}) {
    const config=validate(source), version=options.contentVersion, type=config.type;
    let state=checkedState(config,initial(config,version),version), restored=false, invalid=false;
    if(options.state)try{state=checkedState(config,options.state,version);restored=true;}catch(_){invalid=true;}
    const doc=host.ownerDocument, controller=new AbortController();
    const node=(tag,text='',className='')=>{const value=doc.createElement(tag);value.textContent=text;value.className=className;return value;};
    const svg=(tag,attrs={})=>{const value=doc.createElementNS('http://www.w3.org/2000/svg',tag);Object.entries(attrs).forEach(([key,val])=>value.setAttribute(key,val));return value;};
    const listen=(target,event,callback)=>target.addEventListener(event,callback,{signal:controller.signal});
    const frame=node('section','','il-lab il-subject');frame.dataset.labType=type;
    const instructions={cell_lab:'切换典型植物、动物细胞，再选择一个结构，比较结构与功能。',history_lab:'选择事件查看资料，用“提前”“推后”调整顺序，再核对日期。',
      climate_lab:'选择两个不同月份，比较气温、降水及季节变化。',annotation_lab:'给原文片段做标注：意象、表达或疑问。点击片段也可依次切换标注。',
      sentence_lab:'为句子片段选择成分，观察句子结构。遇到多种合理分析时，可把理由交给老师讨论。',algorithm_lab:'点击“下一步”，观察相邻两个数如何比较和交换；也可以返回上一步。'};
    if(config.algorithm==='binary_search')instructions.algorithm_lab='预测下一次比较的中点，逐步缩小查找范围，也可以回到上一步。';
    if(config.mode==='selection')instructions.annotation_lab='在原文中选中词句，选择标注类别并写下理由，再添加标注。也可用 Shift 与方向键选择文字。';
    frame.append(node('p',instructions[type],'il-instruction'));
    const diagram=node('div','','il-subject-diagram'), controls=node('div','','il-subject-controls'), readout=node('p','','il-lab-result');
    readout.setAttribute('role','status');readout.setAttribute('aria-live','polite');frame.append(diagram,readout,controls);
    const bindings=[];
    const getState=()=>checkedState(config,state,version);
    const changed=(kind='change')=>{update();options.onChange?.(getState(),{kind});};
    const button=(text,action,parent=controls)=>{const value=node('button',text,'dc-button');value.type='button';listen(value,'click',action);parent.append(value);return value;};
    const select=(label,choices,get,set)=>{
      const row=node('label','','il-subject-control'),input=node('select');input.setAttribute('aria-label',label);
      for(const [value,text] of choices){const option=node('option',text);option.value=value;input.append(option);}
      row.append(node('span',label),input);controls.append(row);bindings.push(()=>input.value=String(get()));
      listen(input,'change',()=>{set(input.value);changed();});return input;
    };
    let cellSvg,partControls,historyRows,climateSvg,marked,algorithmSvg,previous,next,selectionList,addSelection;
    if(type==='cell_lab') {
      select('细胞类型',[['plant','典型光合植物细胞'],['animal','典型动物细胞']],()=>state.cellType,value=>{
        state.cellType=value;if(!parts(value).includes(state.selectedPart))state.selectedPart='nucleus';
      });
      cellSvg=svg('svg',{viewBox:'0 0 440 270',role:'img','aria-label':'典型真核细胞简化结构示意，非比例图'});diagram.append(cellSvg);
      partControls=node('div','','il-part-controls');controls.append(partControls);
      for(const [id,[label]] of Object.entries(facts)) {
        const value=button(label,()=>{state.selectedPart=id;changed();},partControls);value.dataset.part=id;
        bindings.push(()=>{value.hidden=!parts(state.cellType).includes(id);value.setAttribute('aria-pressed',String(state.selectedPart===id));});
      }
      // SVG只是同步图示，选择操作同时提供原生按钮，支持键盘和辅助技术。
      listen(cellSvg,'click',event=>{const id=event.target.closest('[data-part]')?.dataset.part;if(parts(state.cellType).includes(id)){state.selectedPart=id;changed();}});
    } else if(type==='history_lab') {
      historyRows=new Map();const list=node('ol','','il-history-list');diagram.append(list);
      for(const [id,event] of Object.entries(events)) {
        const row=node('li'),name=button(event.label,()=>{state.selectedEvent=id;changed();},row),date=node('time',event.date);date.dateTime=event.date;
        const move=delta=>{const index=state.order.indexOf(id),target=index+delta;if(target<0||target>=3)return;[state.order[index],state.order[target]]=[state.order[target],state.order[index]];changed();
          // 到达边界后方向按钮会禁用，把焦点交给同一事件，避免落回页面BODY。
          const control=delta<0?up:down;(control.disabled?name:control).focus();};
        const up=button('提前',()=>move(-1),row),down=button('推后',()=>move(1),row);
        up.setAttribute('aria-label',event.label+'提前');down.setAttribute('aria-label',event.label+'推后');row.append(date);list.append(row);
        historyRows.set(id,{row,name,up,down,list});
      }
    } else if(type==='climate_lab') {
      const months=Array.from({length:12},(_,i)=>[i+1,`${i+1}月`]);
      select('观察月份',months,()=>state.month,value=>state.month=Number(value));select('比较月份',months,()=>state.compareMonth,value=>state.compareMonth=Number(value));
      diagram.append(node('p','教学示例数据 · 非真实城市观测记录','il-data-source'));
      climateSvg=svg('svg',{viewBox:'0 0 600 330',role:'img','aria-label':'十二个月气温与降水，气温和降水分别使用标明单位的坐标'});diagram.append(climateSvg);
      const details=node('details'),summary=node('summary','查看完整示例数据表'),table=node('table','','il-atom-table'),head=node('tr');
      ['月份','气温 / °C','降水 / mm'].forEach(text=>head.append(node('th',text)));table.append(node('caption','自编的十二个月教学数据'),head);
      temperatures.forEach((value,index)=>{const row=node('tr');[`${index+1}月`,value,rainfall[index]].forEach(text=>row.append(node('td',String(text))));table.append(row);});
      details.append(summary,table);diagram.append(details);
    } else if(config.mode==='selection') {
      const passage=node('textarea','','il-selection-text');passage.readOnly=true;passage.rows=4;
      passage.value=config.fragments.join('');passage.setAttribute('aria-label','选择要标注的原文');diagram.append(passage);
      // textarea把CRLF显示为LF，选区仍须回到原文的码点位置；代理对内部没有合法边界。
      const codePoints=Array.from(config.fragments.join('')),selectionOffsets=[0];
      for(let index=0;index<codePoints.length;index++) {
        const character=codePoints[index];
        if(character==='\r'&&codePoints[index+1]==='\n')index++;
        if(character.length===2)selectionOffsets.push(null);
        selectionOffsets.push(index+1);
      }
      const category=node('select');category.setAttribute('aria-label','标注类别');
      labels[type].slice(1).forEach((text,index)=>{const option=node('option',text);option.value=String(index+1);category.append(option);});
      const reason=node('textarea','','il-selection-reason');reason.rows=2;reason.setAttribute('aria-label','标注理由');
      const categoryLabel=node('label','','il-subject-control');categoryLabel.append(node('span','标注类别'),category);
      const reasonLabel=node('label','','il-selection-field');reasonLabel.append(node('span','标注理由'),reason);
      const notice=node('p','','il-note');notice.setAttribute('role','status');
      controls.append(categoryLabel,reasonLabel);
      const selectionActions=node('div','','il-actions');controls.append(selectionActions);
      addSelection=button('添加标注',()=>{
        try {
          const start=selectionOffsets[passage.selectionStart],end=selectionOffsets[passage.selectionEnd];
          const entry={start,end,label:Number(category.value),reason:reason.value};
          state=checkedState(config,{...state,selections:[...state.selections,entry]},version);
          reason.value='';notice.textContent='';changed();
        } catch(error){notice.textContent=error.message;}
      },selectionActions);
      selectionList=node('ol','','il-selection-list');controls.append(notice,selectionList);
    } else if(labels[type]) {
      marked=[];const key=type==='annotation_lab'?'marks':'roles', passage=node('div','','il-marked-passage');diagram.append(passage);
      config.fragments.forEach((text,index)=>{
        const item=button(text,()=>{state[key][index]=(state[key][index]+1)%labels[type].length;changed();},passage);
        const choice=select(`片段${index+1}：${text}`,labels[type].map((label,value)=>[value,label]),()=>state[key][index],value=>state[key][index]=Number(value));
        marked.push({item,choice});
      });
    } else {
      algorithmSvg=svg('svg',{viewBox:'0 0 600 235',role:'img','aria-label':config.algorithm==='binary_search'?'二分查找当前数组及刚比较的中点':'冒泡排序当前数组及刚比较的相邻项'});diagram.append(algorithmSvg);
      previous=button('上一步',()=>{if(state.step>0){state.step--;changed();}});
      next=button('下一步',()=>{if(!calculate(config,state,version).finished){state.step++;changed();}});
    }
    const boundaries={cell_lab:'典型真核细胞简图，结构位置和大小不按比例，不包含全部细胞器。植物细胞也进行细胞呼吸；并非所有植物细胞都含叶绿体。',
      history_lab:'仅核对事件与日期。时间先后本身不能证明因果；此处没有人物动机或历史解释评分。',
      climate_lab:'这是为比较练习自编的北半球季节型数据，没有真实站点、观测年份或气候分类结论。月份差值不表示因果。',
      annotation_lab:'标注表达你的阅读理解，没有程序预设的唯一答案；原文片段应来自本课材料，选择标注不会自动评分。',
      sentence_lab:'本组件记录你的句法分析，不自动判定唯一正确结构，不提供听力或发音评价。',
      algorithm_lab:config.algorithm==='binary_search'?'在升序数组中每次比较一个中点，再缩小查找区间。重复值返回本次命中的索引，找到或区间为空时结束。':'只重放固定的升序冒泡排序；每步比较一对相邻项。不执行任意代码，数组长度最多8项，完成排序不等于已掌握算法。'};
    frame.append(node('p',boundaries[type],'il-note'));
    if(type==='cell_lab'||type==='history_lab') {
      const source=node('p','','il-note'),link=node('a',type==='cell_lab'?'结构依据：OpenStax Biology 2e 4.3':'日期依据：NASA Apollo 11');
      link.href=type==='cell_lab'?'https://openstax.org/books/biology-2e/pages/4-3-eukaryotic-cells':'https://www.nasa.gov/mission/apollo-11/';
      link.target='_blank';link.rel='noopener noreferrer';source.append(link);frame.append(source);
    }
    const note=node('p',invalid?'上次探索与当前内容不符，已载入当前示例。':restored?'已恢复上次探索。':'自由探索不会计入成绩。','il-note');
    const actions=node('div','','il-actions');frame.append(note,actions);
    button('重置探索',()=>{state=initial(config,version);changed('reset');note.textContent='已恢复本次示例。';},actions);
    if(options.onExplain){const explain=button('请老师解释当前结果',async()=>{explain.disabled=true;try{await options.onExplain(getState());}catch(_){note.textContent='解释请求未完成，可以稍后重试。';}finally{explain.disabled=false;}},actions);}
    host.append(frame);
    function update() {
      bindings.forEach(bind=>bind());const result=calculate(config,state,version);
      if(type==='cell_lab') {
        cellSvg.replaceChildren();
        const shape=(id,tag,attrs)=>{const element=svg(tag,{...attrs,class:`il-cell-part${state.selectedPart===id?' is-selected':''}`,'data-part':id});const title=svg('title');title.textContent=facts[id][0];element.append(title);cellSvg.append(element);};
        if(state.cellType==='plant')shape('wall','rect',{x:30,y:15,width:380,height:240,rx:25});
        // 细胞质填充细胞膜以内的区域，避免把它画成另一层独立的膜或细胞器。
        shape('cytoplasm','rect',{x:43,y:28,width:354,height:214,rx:state.cellType==='plant'?19:99});
        shape('membrane','rect',{x:42,y:27,width:356,height:216,rx:state.cellType==='plant'?20:100});
        if(state.cellType==='plant'){shape('vacuole','rect',{x:170,y:66,width:135,height:135,rx:28});shape('chloroplast','ellipse',{cx:115,cy:76,rx:34,ry:18});}
        shape('nucleus','ellipse',{cx:state.cellType==='plant'?112:205,cy:145,rx:38,ry:32});
        shape('mitochondrion','ellipse',{cx:330,cy:202,rx:32,ry:15});
        readout.textContent=`${result.part[0]}：${result.part[1]}`;
      } else if(type==='history_lab') {
        state.order.forEach((id,index)=>{const row=historyRows.get(id);row.list.append(row.row);row.name.setAttribute('aria-pressed',String(state.selectedEvent===id));row.up.disabled=index===0;row.down.disabled=index===2;});
        readout.textContent=`${result.selectedFact.label}：${result.selectedFact.date}。${result.chronological?'当前顺序符合日期先后。':'当前顺序与日期先后不一致，可以继续调整。'}`;
      } else if(type==='climate_lab') {
        climateSvg.replaceChildren();const text=(x,y,value)=>{const element=svg('text',{x,y});element.textContent=value;climateSvg.append(element);};
        text(12,18,'气温 / °C');text(12,182,'降水 / mm');
        [0,10,20,30].forEach(value=>{text(12,145-value*3,String(value));climateSvg.append(svg('path',{d:`M45 ${141-value*3}H580`,class:'il-chart-grid'}));});
        [0,100,200].forEach(value=>{text(5,300-value*.5,String(value));climateSvg.append(svg('path',{d:`M45 ${296-value*.5}H580`,class:'il-chart-grid'}));});
        climateSvg.append(svg('polyline',{points:temperatures.map((value,index)=>`${60+index*46},${141-value*3}`).join(' '),class:'il-temperature-line'}));
        temperatures.forEach((value,index)=>{const selected=index===state.month-1,compare=index===state.compareMonth-1;
          climateSvg.append(svg('circle',{cx:60+index*46,cy:141-value*3,r:selected?6:3,class:selected?'il-chart-current':'il-temperature-point'}));
          climateSvg.append(svg('rect',{x:49+index*46,y:296-rainfall[index]*.5,width:22,height:rainfall[index]*.5,class:selected?'il-chart-current':compare?'il-chart-compare':'il-rainfall-bar'}));
          text(53+index*46,322,String(index+1));});
        text(548,162,'月份');readout.textContent=`${state.month}月：${result.temperatureC} °C、${result.precipitationMm} mm；相对${state.compareMonth}月，气温差 ${result.temperatureDifferenceC} °C，降水差 ${result.precipitationDifferenceMm} mm。`;
      } else if(config.mode==='selection') {
        selectionList.replaceChildren();
        result.annotations.forEach((entry,index)=>{
          const row=node('li'),quote=node('blockquote',entry.text),label=node('p',entry.label),reason=node('p',entry.reason);
          row.append(quote,label,reason);
          button('删除标注',()=>{state.selections.splice(index,1);changed();addSelection.focus();},row).setAttribute('aria-label',`删除标注 ${index+1}`);
          selectionList.append(row);
        });
        readout.textContent=result.annotations.length?`已标注 ${result.annotations.length} 处，可继续选择词句。`:'尚未添加选区标注。';
      } else if(labels[type]) {
        const key=type==='annotation_lab'?'marks':'roles';
        marked.forEach(({item},index)=>{const value=state[key][index];item.dataset.mark=String(value);item.setAttribute('aria-label',`${config.fragments[index]}，${labels[type][value]}，点击切换标注`);});
        readout.textContent=result.annotations.length?result.annotations.map(item=>`${item.text} → ${item.label}`).join('；'):'尚未标注。可以从你最有把握的一个片段开始。';
      } else {
        algorithmSvg.replaceChildren();const width=540/result.array.length;
        result.array.forEach((value,index)=>{const height=value*1.4+8;algorithmSvg.append(svg('rect',{x:32+index*width,y:195-height,width:width-12,height,class:result.comparedIndices.includes(index)?'il-chart-current':'il-rainfall-bar',opacity:config.algorithm==='binary_search'&&(index<result.low||index>result.high)?0.35:1}));
          const text=svg('text',{x:32+index*width+(width-12)/2,y:182-height,'text-anchor':'middle'});text.textContent=String(value);algorithmSvg.append(text);
          const label=svg('text',{x:32+index*width+(width-12)/2,y:220,'text-anchor':'middle'});label.textContent=`[${index}]`;algorithmSvg.append(label);});
        previous.disabled=state.step===0;next.disabled=result.finished;
        if(config.algorithm==='binary_search') {
          readout.textContent=`目标 ${result.target}；第 ${state.step} 步。`+
            (state.step?`刚比较索引 ${result.comparedIndices[0]}，值为 ${result.array[result.comparedIndices[0]]}。`:'尚未比较。')+
            (result.finished?(result.foundIndex===null?'查找区间为空，没有找到目标。':`找到目标，索引为 ${result.foundIndex}。`):`下一次查找索引区间 [${result.low}, ${result.high}]。`);
        } else readout.textContent=`第 ${state.step} / ${result.totalSteps} 步；数组 [${result.array.join(', ')}]。`+(state.step?`刚比较索引 ${result.comparedIndices.join(' 与 ')}，${result.swapped?'交换了两项':'顺序合适，未交换'}。`:'还没有比较。')+(result.finished?'本轮排序完成。':'');
      }
    }
    update();return {getState,destroy(){controller.abort();frame.remove();}};
  }
  return Object.freeze({types:Object.freeze(types),validate,checkedState,calculate,mount});
});

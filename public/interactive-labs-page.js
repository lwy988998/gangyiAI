/* 离线可用的实验入口；示例不产生课程、画像或正式成绩。 */
(() => {
  'use strict';
  const examples={
    periodic:{title:'从元素位置到原子组成',question:'比较 Na 与 Cl 的周期和族，再把当前元素改为 K、比较元素改为 Na，观察两者的联系。展开电子层示意，注意中性原子的电子数。',config:{type:'periodic_lab'}},
    wave:{title:'波形向前，质点怎样运动',question:'播放并观察 P 点。暂停后改变周期，保持波速不变，比较波长；再改变振幅，波速会变化吗？',config:{type:'wave_lab'}},
    triangle:{title:'移动顶点，观察三角形怎样变化',question:'从直角三角形开始，移动 C 点比较边长、角度与面积；再让三个点共线，观察哪些结论不再适用。',config:{type:'triangle_lab'}},
    statistics:{title:'一项数据会怎样影响整组统计量',question:'原数据为 2、3、3、4、8。把 8 改成 3，再比较平均数与中位数；切换两种方差，说明分母为什么不同。',config:{type:'statistics_lab'}},
    equation:{title:'等式两边怎样变，解才不变',question:'从 2x+3=x+7 开始。先把一次项留在一边，再把常数留在另一边。每一步都说明两边做了什么。',config:{type:'equation_lab'}},
    vector:{title:'向量相加，长度也相加吗',question:'从 A=(3,0)、B=(0,4) 开始，比较 |A+B| 和 |A|+|B|；再把 B 调到与 A 同方向，观察有什么不同。',config:{type:'vector_lab'}},
    numberline:{title:'端点属于区间吗',question:'把观察点移到左端点，再切换左端点的开闭。区间长度有没有变化？把两端调到同一个数，再比较空集与单点集。',config:{type:'number_line'}},
    function:{title:'探索函数图像',question:'当二次项系数从正变负，图像会怎样变化？试着把 a 调到 0，再比较一次函数。',config:{type:'function_lab',coefficients:[1,0,0]}},
    circuit:{title:'比较串联和并联',question:'保持电源和电阻不变，切换连接方式。总电流有什么变化？支路电流一定相等吗？',config:{type:'circuit_lab',mode:'series',voltage:6,r1:10,r2:20}},
    motion:{title:'位移和路程为什么不同',question:'初速度为 6 m/s，加速度为 −2 m/s²。先观察第 3 秒，再观察第 6 秒，位置、速度和路程分别怎样变化？',config:{type:'motion_lab',initialVelocityMps:6,accelerationMps2:-2}},
    chemical:{title:'用原子守恒探索配平',question:'调整 H₂、O₂ 和 H₂O 前的系数，让两侧的氢、氧原子总数分别相等。',config:{type:'chemical_balance',reactionId:'water'}},
    sets:{title:'哪些元素同时属于两个集合',question:'选择元素属于 A 或 B，找出“同时属于”的元素，再试着让交集为空。',config:{type:'sets',universe:['1','2','3','4'],setA:['1','2'],setB:['2','3']}},
    cell:{title:'细胞结构怎样分工',question:'植物细胞和动物细胞都有哪些结构？选择线粒体，再切换细胞类型，比较它是否仍然存在。',config:{type:'cell_lab'}},
    history:{title:'把事件放回时间中',question:'阿波罗11号经历了发射、月面着陆、返回地球。调整排列，再用资料中的日期核对。',config:{type:'history_lab'}},
    climate:{title:'比较月份，读懂气候数据',question:'先比较1月与7月，再找出气温最高、降水最多的月份。这里使用自编教学数据。',config:{type:'climate_lab'}},
    annotation:{title:'把你的阅读发现标出来',question:'阅读这段自编短文，标出你注意到的意象、表达或疑问，再想一想选择的理由。',config:{type:'annotation_lab',fragments:['雨停了，','屋檐还在滴水。','一束光落在书页上，','像替未读完的故事','留下一枚书签。']}},
    selection:{title:'选择词句，写下阅读理由',question:'阅读这段自编短文，选中你注意到的词句，说明它怎样表达人物的感受。',config:{type:'annotation_lab',mode:'selection',fragments:['窗外的雨停了，','屋檐还在滴水。','他把旧信折好，','久久没有放回抽屉。']}},
    sentence:{title:'动手分析句子结构',question:'在这个自编句子中，谁做了什么？尝试标记主语、谓语动词、宾语和修饰成分。',config:{type:'sentence_lab',fragments:['The students','read','a book','in the library.']}},
    algorithm:{title:'一步一步看排序',question:'每次只比较相邻两项。如果左边更大，就交换。预测下一步，再点按钮核对。',config:{type:'algorithm_lab',values:[5,2,4,1]}},
    binary:{title:'一步一步缩小查找范围',question:'在有序数组中找 11。预测中点比较后能排除哪一半，再点下一步核对。',config:{type:'algorithm_lab',algorithm:'binary_search',values:[1,3,5,7,9,11,13,15],target:11}}
  };
  let instance;
  const host=document.getElementById('lab-host'),status=document.getElementById('lab-status');
  function select(id){
    const example=examples[id];if(!example)return;
    instance?.destroy();host.replaceChildren();
    document.getElementById('lab-title').textContent=example.title;document.getElementById('lab-question').textContent=example.question;
    for(const button of document.querySelectorAll('[data-lab]'))button.setAttribute('aria-pressed',String(button.dataset.lab===id));
    let previous;try{previous=JSON.parse(localStorage.getItem('gangyi-lab-demo:'+id)||'null');}catch(_){/* 损坏草稿只影响本次恢复。 */}
    try{
      instance=window.GangyiInteractive.mount(host,example.config,{contentVersion:1,state:previous,onChange(state){
        try{localStorage.setItem('gangyi-lab-demo:'+id,JSON.stringify(state));}catch(_){status.textContent='当前可以探索；本机存储不可用，关闭后需要重新开始。';}
      }});
      status.textContent='';
    }catch(error){status.textContent=error.message;}
  }
  document.querySelector('.labs-topics').addEventListener('click',event=>{const button=event.target.closest('[data-lab]');if(button)select(button.dataset.lab);});
  window.addEventListener('pagehide',()=>instance?.destroy());
  select('function');
})();

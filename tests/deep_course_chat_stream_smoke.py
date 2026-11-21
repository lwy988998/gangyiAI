"""实际课堂前端的长会话流式渲染测量；历史和流事件是明确的合成输入。"""
import hashlib
import json
import math
from pathlib import Path
import sys
import time
from urllib.parse import urlencode

from playwright.sync_api import sync_playwright

repo=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(repo/'tests'))
import classroom_v560_fixture as fixture
from deep_course_integration_smoke import DeepFixture
fixture.FixtureAI=DeepFixture
out=Path(sys.argv[2]).resolve()
out.mkdir(parents=True,exist_ok=False)

def percentile(values,p):return sorted(values)[max(0,math.ceil(len(values)*p)-1)]

with fixture.start_session(sys.argv[1]) as (info,api,_),sync_playwright() as runner:
    def wait(task_id):
        deadline=time.monotonic()+90
        while time.monotonic()<deadline:
            task=api('/api/learning-agent?taskId='+task_id)
            if task['status'] not in ('pending','running'):
                assert task['status'] in ('ready','waiting_student'),task.get('error')
                return task
            time.sleep(.1)
        raise TimeoutError(task_id)
    course=wait(api('/api/generate-plan',{'goal':'长会话渲染的隔离课堂','mode':'deep','requestId':'stream-performance-plan'})['id'])['course']['id']
    lesson=wait(api('/api/deep-course/start',{'courseId':course,'phaseIndex':1,'topicIndex':1,'requestId':'stream-performance-prepare'})['task']['id'])['lesson']['id']
    history=[{'id':f'synthetic-history-{i}','role':'assistant' if i%2 else 'user',
        'content':f'合成历史消息 {i}。保留当时的思路与说明。'+('**比较函数** $y=x^2$ 的变化。\n\n请先观察，再用自己的话解释。' if i%2 else '我想先理解当前问题，保留自己的判断。'),
        'sources':['https://example.invalid/synthetic-source'] if i==1 else [],'createdAt':'2026-11-21T00:00:00Z','status':'ready'} for i in range(150)]
    chat={'lessonId':lesson,'webEnabled':False,'messages':history,'activeTask':{'id':'performance-chat-task','status':'running'}}
    browser=runner.chromium.launch(headless=True,channel='msedge')
    page=browser.new_page(viewport={'width':1280,'height':900},reduced_motion='reduce')
    errors=[]
    page.on('pageerror',lambda error:errors.append(str(error)))
    page.add_init_script('''Object.defineProperty(window,'GangyiAgent',{configurable:true,set(value){
      Object.defineProperty(window,'GangyiAgent',{configurable:true,writable:true,value});
      const original=value.watch;
      value.watch=(id,handlers)=>{if(id==='performance-chat-task'){window.syntheticStream=handlers;return ()=>{window.syntheticStreamStopped=true;};}return original(id,handlers);};
    }});''')
    page.route('**/api/independent-chat?*',lambda route:route.fulfill(json=chat))
    page.goto(info['base']+'/deep-course?'+urlencode({'lessonId':lesson}))
    page.wait_for_function("document.getElementById('deep-course-app').dataset.ready==='true' && window.syntheticStream")
    if len(sys.argv)>3 and sys.argv[3]=='--mobile-only':
        page.set_viewport_size({'width':390,'height':844})
        fixture.open_learning_menu(page).locator('#dc-chat-toggle').click()
        page.locator('#dc-chat-dialog').wait_for(state='visible')
        page.evaluate("syntheticStream.onEvent({type:'delta',field:'message',seq:1,text:'窄屏合成回复，历史仍保持。'})")
        page.evaluate('()=>new Promise(resolve=>requestAnimationFrame(()=>requestAnimationFrame(resolve)))')
        assert page.locator('#dc-chat-dialog [data-message-key]').count()==150
        assert page.locator('#dc-chat-dialog').get_by_role('button',name='停止回复',exact=True).is_visible()
        assert page.evaluate('document.documentElement.scrollWidth<=innerWidth+1')
        page.locator('#dc-chat-dialog').screenshot(path=str(out/'chat-mobile.png'))
        (out/'report.json').write_text(json.dumps({'passed':True,'scope':'仅窄屏聊天显示','viewport':390,
            'historyMessages':150,'stopVisible':True,'pageErrors':errors,'paidModelRequests':0},ensure_ascii=False,indent=2),encoding='utf-8')
        browser.close()
        raise SystemExit(0)
    fixture.open_learning_menu(page).locator('#dc-chat-toggle').click()
    page.wait_for_function("document.querySelectorAll('#dc-chat-scroll [data-message-key]').length===150")
    page.evaluate('''()=>{
      window.originalHistory=[...document.querySelectorAll('#dc-chat-scroll [data-message-key]')];
      window.originalStop=[...document.querySelectorAll('#dc-chat-scroll button')].find(button=>button.textContent==='停止回复');
      if(!originalStop)throw new Error('停止入口不存在');originalStop.focus();
      const range=document.createRange();range.selectNodeContents(originalHistory[2].querySelector('.dc-rich'));
      getSelection().removeAllRanges();getSelection().addRange(range);window.selectedHistory=getSelection().toString();
      window.expectedText='';window.streamSequence=0;
    }''')
    samples=[]
    for burst in range(6):
        values=page.evaluate('''async()=>{
          const durations=[];
          for(let i=0;i<10;i++){
            const text=`第${++streamSequence}段合成说明。`;
            expectedText+=text;
            const before=performance.now();syntheticStream.onEvent({type:'delta',field:'message',seq:streamSequence,text});
            durations.push(performance.now()-before);
          }
          await new Promise(resolve=>requestAnimationFrame(()=>requestAnimationFrame(resolve)));
          return durations;
        }''')
        samples.extend(values)
    observed=page.evaluate('''()=>{
      const nodes=[...document.querySelectorAll('#dc-chat-scroll [data-message-key]')];
      const stop=[...document.querySelectorAll('#dc-chat-scroll button')].find(button=>button.textContent==='停止回复');
      const stream=[...document.querySelectorAll('#dc-chat-scroll article')].find(node=>node.querySelector('h3')?.textContent==='AI 聊天 · 正在回复');
      return {historyRetained:nodes.filter((node,index)=>node===originalHistory[index]).length,
        historyCount:nodes.length,stopRetained:stop===originalStop,stopFocusRetained:document.activeElement===originalStop,
        selectedHistoryRetained:getSelection().toString()===selectedHistory&&getSelection().anchorNode?.isConnected===true,
        exactStreamText:stream?.querySelector('.dc-rich')?.textContent===expectedText,
        expectedText,actualText:stream?.querySelector('.dc-rich')?.textContent||'',scrollHeight:document.getElementById('dc-chat-scroll').scrollHeight};
    }''')
    # 重放相同序列不应重复展示文字。
    page.evaluate("syntheticStream.onEvent({type:'delta',field:'message',seq:60,text:'不能重复'})")
    page.evaluate('()=>new Promise(resolve=>requestAnimationFrame(()=>requestAnimationFrame(resolve)))')
    assert '不能重复' not in page.locator('#dc-chat-scroll').inner_text()
    page.screenshot(path=str(out/'chat-stream-desktop.png'))
    page.evaluate("syntheticStream.onState({id:'performance-chat-task',status:'paused',events:[{type:'delta',field:'message',seq:60,text:expectedText}]})")
    assert page.locator('#dc-chat-scroll').get_by_role('button',name='继续回复',exact=True).count()==1
    page.evaluate("syntheticStream.onState({id:'performance-chat-task',status:'running'})")
    assert page.locator('#dc-chat-scroll').get_by_role('button',name='停止回复',exact=True).count()==1
    history[1].update(content='已更正的合成历史说明。',sources=['https://example.invalid/corrected-source'])
    removed=history.pop()['id']
    history.append({'id':'synthetic-final-reply','role':'assistant','content':observed['expectedText'],'status':'ready','sources':[]})
    chat['activeTask']=None
    page.evaluate("syntheticStream.onState({id:'performance-chat-task',status:'ready'})")
    page.wait_for_function("document.querySelector('[data-message-key=synthetic-final-reply]')")
    assert page.locator(f'[data-message-key={removed}]').count()==0
    assert page.locator('[data-message-key=synthetic-history-1]').get_by_role('link').get_attribute('href')=='https://example.invalid/corrected-source'
    assert '已更正的合成历史说明。' in page.locator('[data-message-key=synthetic-history-1]').inner_text()
    assert page.locator('[data-message-key=synthetic-final-reply] .dc-rich').inner_text()==observed['expectedText']
    assert page.locator('#dc-chat-scroll .dc-chat-task-state').count()==0
    page.set_viewport_size({'width':390,'height':844})
    assert page.evaluate('document.documentElement.scrollWidth<=innerWidth+1')
    page.screenshot(path=str(out/'chat-completed-mobile.png'))
    report={'passed':observed['historyRetained']==150 and observed['stopRetained'] and observed['stopFocusRetained'] and observed['exactStreamText'] and observed['selectedHistoryRetained'] and not errors,
        'scope':'实际产品课堂前端；通过测试watch回调送入60个合成delta，150条合成历史；非真实模型首响或网络SSE性能',
        'mockAIOnly':True,'paidModelRequests':0,'browser':browser.version,'historyMessages':150,'streamChunks':60,
        'deltaHandlerMs':{'p50':percentile(samples,.5),'p95':percentile(samples,.95),'max':max(samples),'samples':samples},
        'observed':observed,'pageErrors':errors,'sourceSha256':hashlib.sha256((repo/'public/deep-course.js').read_bytes()).hexdigest()}
    report.update(pausedAndRunningControls=True,historyCorrectionAndRemoval=True,finalReplyExact=True,completedTaskControlsCleared=True)
    (out/'report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    browser.close()
    print(json.dumps({'passed':report['passed'],'historyRetained':observed['historyRetained'],'p95':report['deltaHandlerMs']['p95']},ensure_ascii=False))
    if not report['passed']:raise SystemExit(1)

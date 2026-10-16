"""免费真实页面回归：板块、消息的部分展示、完整展示与刷新，使用隔离夹具。"""
import json
import sqlite3
import sys
import time
from contextlib import closing
from urllib.request import Request, urlopen
from playwright.sync_api import expect, sync_playwright
from classroom_v560_fixture import start_session


def main(executable):
    with start_session(executable) as (info, api, _):
        lesson_id = info['lessonId']
        def lesson():
            return api('/api/learning-agent/lesson?lessonId='+lesson_id)
        practices = [s for s in lesson()['sections'] if s.get('questionKind') == 'practice']
        serial = 0
        def wait(task):
            end = time.monotonic()+60
            while time.monotonic()<end:
                task = api('/api/learning-agent?taskId='+task['id'])
                if task['status'] not in ('pending', 'running'):
                    assert task['status']=='waiting_student', task
                    return task
                time.sleep(.05)
            raise AssertionError('隔离教学任务超时')
        def answer(section):
            nonlocal serial
            serial += 1
            request_id='display-answer-'+str(serial)
            wait(api('/api/learning-agent/events',dict(type='question_answer',text='我认为化合价发生变化',courseId=info['courseId'],
                lessonId=lesson_id,sectionId=section['id'],sectionVersion=section['version'],view='practice',requestId=request_id)))
            with closing(sqlite3.connect(info['database'])) as db:
                values=[json.loads(v) for v, in db.execute("SELECT payload FROM LearningInteraction WHERE kind='chat-user'")]
            return next(v for v in values if v.get('requestId')==request_id)
        def metadata(key):
            with closing(sqlite3.connect(info['database'])) as db:
                row=db.execute('SELECT value FROM ProfileMeta WHERE key=?',(key,)).fetchone()
            return json.loads(row[0]) if row else {}
        def until(check):
            end=time.monotonic()+10
            while time.monotonic()<end:
                if check(): return
                time.sleep(.05)
            raise AssertionError('展示回执未按预期保存')
        def cover(page,node):
            height=node.evaluate('(n)=>n.getBoundingClientRect().height')
            step=max(30,int(page.locator('#lesson-scroll').bounding_box()['height']*.7))
            for offset in range(0,int(height)+step,step):
                node.evaluate('(n,y)=>{const s=document.getElementById("lesson-scroll");s.scrollTop+=n.getBoundingClientRect().top-s.getBoundingClientRect().top+y}',offset)
                # 滚动回调会再排一帧；等待它和布局完成后再保存，不能把刚赋scrollTop当作回执已到。
                page.evaluate('new Promise(resolve=>requestAnimationFrame(()=>requestAnimationFrame(resolve)))')
            page.evaluate('GangyiAgent.flushExposure()')
        with closing(sqlite3.connect(info['database'])) as db:
            row=db.execute('SELECT payload FROM ClassroomActivity WHERE id=?',(lesson_id,)).fetchone()
            raw=json.loads(row[0]); section=raw['sections'][0]
            section.update(helpType='worked_example',targetQuestionId=practices[0]['questionId'],
                body='\n\n'.join(['完整示范：Fe由0升到+2，失去两个电子。']*60),
                demonstratedQuestions=[{'question':practices[0]['question']['question'],'options':practices[0]['question'].get('options',[])}])
            section_id=section['id'];db.execute('UPDATE ClassroomActivity SET payload=? WHERE id=?',(json.dumps(raw,ensure_ascii=False),lesson_id));db.commit()
        with sync_playwright() as p:
            browser=p.chromium.launch(headless=True); page=browser.new_page(viewport={'width':390,'height':650},reduced_motion='reduce')
            errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
            page.goto(info['base']+'/learn?lessonId='+lesson_id);expect(page.locator('#lesson-input')).to_be_enabled()
            node=page.locator('[data-section-id="'+section_id+'"] .lesson-body')
            node.evaluate('(n)=>n.scrollIntoView({block:"start"})');page.evaluate('GangyiAgent.flushExposure()')
            until(lambda:section_id in metadata('agent-section-display:'+lesson_id))
            assert metadata('agent-section-display:'+lesson_id)[section_id]['complete'] is False
            assert answer(practices[0])['helpMode']=='help_unknown'
            assert answer(practices[1])['helpMode']=='independent'
            page.reload();expect(page.locator('#lesson-input')).to_be_enabled()
            cover(page,node);until(lambda:metadata('agent-section-display:'+lesson_id).get(section_id,{}).get('complete') is True)
            assert answer(practices[0])['helpMode']=='after_example'
            page.reload();expect(page.locator('#lesson-input')).to_be_enabled();page.evaluate('GangyiAgent.flushExposure()')
            assert metadata('agent-section-display:'+lesson_id)[section_id]['complete'] is True
            assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()
            browser.close();assert not errors,errors
        # 新开隔离档案，避免已经完整展示的板块掩盖消息自己的部分展示状态。
    with start_session(executable) as (info, api, _):
        lesson_id=info['lessonId'];practices=[s for s in lesson()['sections'] if s.get('questionKind')=='practice']
        with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"long_teaching":true}',headers={'Content-Type':'application/json'})):pass
        task=wait(api('/api/learning-agent/events',dict(type='teaching_request',requestType='worked_example',text='请完整示范',
            courseId=info['courseId'],lessonId=lesson_id,sectionId=practices[0]['id'],sectionVersion=practices[0]['version'],view='practice',requestId='long-display-message')))
        with sync_playwright() as p:
            browser=p.chromium.launch(headless=True);page=browser.new_page(viewport={'width':390,'height':650},reduced_motion='reduce')
            errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
            page.goto(info['base']+'/practice?lessonId='+lesson_id);expect(page.locator('#lesson-input')).to_be_enabled()
            node=page.locator('.chat-message.assistant .chat-message-body').filter(has_text='示范：Fe').first
            node.evaluate('(n)=>n.scrollIntoView({block:"start"})');page.evaluate('GangyiAgent.flushExposure()')
            until(lambda:bool(metadata('agent-exposure:'+task['id']).get('displayedSequences')))
            initial_answer=answer(practices[0])
            assert initial_answer['helpMode']=='help_unknown', (initial_answer['helpMode'],metadata('agent-exposure:'+task['id']))
            assert answer(practices[1])['helpMode']=='independent'
            cover(page,node)
            message_sequences=[e['seq'] for e in task['events'] if e.get('type')=='delta' and e.get('field')=='message']
            until(lambda:all(s in metadata('agent-exposure:'+task['id']).get('completeSequences',[]) for s in message_sequences))
            completed=answer(practices[0])
            assert completed['helpMode']=='after_example', (completed['helpMode'],
                [{k:h.get(k) for k in ('helpType','partial','taskId','step')} for h in completed.get('helpHistory',[])],
                metadata('agent-exposure:'+task['id']),node.evaluate('(n)=>({height:n.getBoundingClientRect().height,top:n.getBoundingClientRect().top})'))
            page.reload();expect(page.locator('#lesson-input')).to_be_enabled();page.evaluate('GangyiAgent.flushExposure()')
            assert answer(practices[0])['helpMode']=='after_example'
            assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()
            browser.close();assert not errors,errors
        print('长板块/消息部分→完整展示、刷新恢复、同题关联、其他题隔离和私有字段回归通过；付费调用0次。')


if __name__=='__main__':
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])

# 【逐行批注】 执行当前 Python 语句："""免费真实页面回归：板块、消息的部分展示、完整展示与刷新，使用隔离夹具。"""。
"""免费真实页面回归：板块、消息的部分展示、完整展示与刷新，使用隔离夹具。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sqlite3。
import sqlite3
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextlib import closing。
from contextlib import closing
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen
# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import expect, sync_playwright。
from playwright.sync_api import expect, sync_playwright
# 【逐行批注】 导入后续逻辑需要的模块或符号：from classroom_v560_fixture import start_session。
from classroom_v560_fixture import start_session


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable):
    # 【逐行批注】 在受控上下文中执行资源操作：with start_session(executable) as (info, api, _):。
    with start_session(executable) as (info, api, _):
        # 【逐行批注】 计算右侧结果并赋给 lesson_id，更新脚本状态。
        lesson_id = info['lessonId']
        # 【逐行批注】 定义函数 lesson，封装可复用的处理步骤。
        def lesson():
            # 【逐行批注】 结束当前函数并返回结果：return api('/api/learning-agent/lesson?lessonId='+lesson_id)。
            return api('/api/learning-agent/lesson?lessonId='+lesson_id)
        # 【逐行批注】 计算右侧结果并赋给 practices，更新脚本状态。
        practices = [s for s in lesson()['sections'] if s.get('questionKind') == 'practice']
        # 【逐行批注】 计算右侧结果并赋给 serial，更新脚本状态。
        serial = 0
        # 【逐行批注】 定义函数 wait，封装可复用的处理步骤。
        def wait(task):
            # 【逐行批注】 计算右侧结果并赋给 end，更新脚本状态。
            end = time.monotonic()+60
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic()<end:。
            while time.monotonic()<end:
                # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
                task = api('/api/learning-agent?taskId='+task['id'])
                # 【逐行批注】 判断条件 task['status'] not in ('pending', 'running'):，成立时执行对应分支。
                if task['status'] not in ('pending', 'running'):
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert task['status']=='waiting_student', task。
                    assert task['status']=='waiting_student', task
                    # 【逐行批注】 结束当前函数并返回结果：return task。
                    return task
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.05)。
                time.sleep(.05)
            # 【逐行批注】 在错误情形抛出异常：raise AssertionError('隔离教学任务超时')。
            raise AssertionError('隔离教学任务超时')
        # 【逐行批注】 定义函数 answer，封装可复用的处理步骤。
        def answer(section):
            # 【逐行批注】 执行当前 Python 语句：nonlocal serial。
            nonlocal serial
            # 【逐行批注】 计算右侧结果并赋给 serial +，更新脚本状态。
            serial += 1
            # 【逐行批注】 计算右侧结果并赋给 request_id，更新脚本状态。
            request_id='display-answer-'+str(serial)
            # 【逐行批注】 计算右侧结果并赋给 wait(api('/api/learning-agent/events',dict(t…，更新脚本状态。
            wait(api('/api/learning-agent/events',dict(type='question_answer',text='我认为化合价发生变化',courseId=info['courseId'],
                # 【逐行批注】 计算右侧结果并赋给 lessonId，更新脚本状态。
                lessonId=lesson_id,sectionId=section['id'],sectionVersion=section['version'],view='practice',requestId=request_id)))
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as db:。
            with closing(sqlite3.connect(info['database'])) as db:
                # 【逐行批注】 计算右侧结果并赋给 values，更新脚本状态。
                values=[json.loads(v) for v, in db.execute("SELECT payload FROM LearningInteraction WHERE kind='chat-user'")]
            # 【逐行批注】 结束当前函数并返回结果：return next(v for v in values if v.get('requestId')==request_id)。
            return next(v for v in values if v.get('requestId')==request_id)
        # 【逐行批注】 定义函数 metadata，封装可复用的处理步骤。
        def metadata(key):
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as db:。
            with closing(sqlite3.connect(info['database'])) as db:
                # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
                row=db.execute('SELECT value FROM ProfileMeta WHERE key=?',(key,)).fetchone()
            # 【逐行批注】 结束当前函数并返回结果：return json.loads(row[0]) if row else {}。
            return json.loads(row[0]) if row else {}
        # 【逐行批注】 定义函数 until，封装可复用的处理步骤。
        def until(check):
            # 【逐行批注】 计算右侧结果并赋给 end，更新脚本状态。
            end=time.monotonic()+10
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic()<end:。
            while time.monotonic()<end:
                # 【逐行批注】 判断条件 check(): return，成立时执行对应分支。
                if check(): return
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.05)。
                time.sleep(.05)
            # 【逐行批注】 在错误情形抛出异常：raise AssertionError('展示回执未按预期保存')。
            raise AssertionError('展示回执未按预期保存')
        # 【逐行批注】 定义函数 cover，封装可复用的处理步骤。
        def cover(page,node):
            # 【逐行批注】 计算右侧结果并赋给 height，更新脚本状态。
            height=node.evaluate('(n)=>n.getBoundingClientRect().height')
            # 【逐行批注】 计算右侧结果并赋给 step，更新脚本状态。
            step=max(30,int(page.locator('#lesson-scroll').bounding_box()['height']*.7))
            # 【逐行批注】 遍历数据并重复执行循环体：for offset in range(0,int(height)+step,step):。
            for offset in range(0,int(height)+step,step):
                # 【逐行批注】 计算右侧结果并赋给 node.evaluate('(n,y)，更新脚本状态。
                node.evaluate('(n,y)=>{const s=document.getElementById("lesson-scroll");s.scrollTop+=n.getBoundingClientRect().top-s.getBoundingClientRect().top+y}',offset)
                # 滚动回调会再排一帧；等待它和布局完成后再保存，不能把刚赋scrollTop当作回执已到。
                # 【逐行批注】 计算右侧结果并赋给 page.evaluate('new Promise(resolve，更新脚本状态。
                page.evaluate('new Promise(resolve=>requestAnimationFrame(()=>requestAnimationFrame(resolve)))')
            # 【逐行批注】 执行当前 Python 语句：page.evaluate('GangyiAgent.flushExposure()')。
            page.evaluate('GangyiAgent.flushExposure()')
        # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as db:。
        with closing(sqlite3.connect(info['database'])) as db:
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row=db.execute('SELECT payload FROM ClassroomActivity WHERE id=?',(lesson_id,)).fetchone()
            # 【逐行批注】 计算右侧结果并赋给 raw，更新脚本状态。
            raw=json.loads(row[0]); section=raw['sections'][0]
            # 【逐行批注】 计算右侧结果并赋给 section.update(helpType，更新脚本状态。
            section.update(helpType='worked_example',targetQuestionId=practices[0]['questionId'],
                # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
                body='\n\n'.join(['完整示范：Fe由0升到+2，失去两个电子。']*60),
                # 【逐行批注】 计算右侧结果并赋给 demonstratedQuestions，更新脚本状态。
                demonstratedQuestions=[{'question':practices[0]['question']['question'],'options':practices[0]['question'].get('options',[])}])
            # 【逐行批注】 计算右侧结果并赋给 section_id，更新脚本状态。
            section_id=section['id'];db.execute('UPDATE ClassroomActivity SET payload=? WHERE id=?',(json.dumps(raw,ensure_ascii=False),lesson_id));db.commit()
        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as p:。
        with sync_playwright() as p:
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser=p.chromium.launch(headless=True); page=browser.new_page(viewport={'width':390,'height':650},reduced_motion='reduce')
            # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
            errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/learn?lessonId，更新脚本状态。
            page.goto(info['base']+'/learn?lessonId='+lesson_id);expect(page.locator('#lesson-input')).to_be_enabled()
            # 【逐行批注】 计算右侧结果并赋给 node，更新脚本状态。
            node=page.locator('[data-section-id="'+section_id+'"] .lesson-body')
            # 【逐行批注】 计算右侧结果并赋给 node.evaluate('(n)，更新脚本状态。
            node.evaluate('(n)=>n.scrollIntoView({block:"start"})');page.evaluate('GangyiAgent.flushExposure()')
            # 【逐行批注】 调用 until、metadata 完成当前脚本或测试步骤：until(lambda:section_id in metadata('agent-section-display:'+lesson_i…。
            until(lambda:section_id in metadata('agent-section-display:'+lesson_id))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert metadata('agent-section-display:'+lesson_id)[section_id]['complete'] is …。
            assert metadata('agent-section-display:'+lesson_id)[section_id]['complete'] is False
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer(practices[0])['helpMode']=='help_unknown'。
            assert answer(practices[0])['helpMode']=='help_unknown'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer(practices[1])['helpMode']=='independent'。
            assert answer(practices[1])['helpMode']=='independent'
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：page.reload();expect(page.locator('#lesson-input')).to_be_enabled()。
            page.reload();expect(page.locator('#lesson-input')).to_be_enabled()
            # 【逐行批注】 调用 cover、until、metadata 完成当前脚本或测试步骤：cover(page,node);until(lambda:metadata('agent-section-display:'+lesso…。
            cover(page,node);until(lambda:metadata('agent-section-display:'+lesson_id).get(section_id,{}).get('complete') is True)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer(practices[0])['helpMode']=='after_example'。
            assert answer(practices[0])['helpMode']=='after_example'
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：page.reload();expect(page.locator('#lesson-input')).to_be_enabled();p…。
            page.reload();expect(page.locator('#lesson-input')).to_be_enabled();page.evaluate('GangyiAgent.flushExposure()')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert metadata('agent-section-display:'+lesson_id)[section_id]['complete'] is …。
            assert metadata('agent-section-display:'+lesson_id)[section_id]['complete'] is True
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()。
            assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()
            # 【逐行批注】 执行当前 Python 语句：browser.close();assert not errors,errors。
            browser.close();assert not errors,errors
        # 新开隔离档案，避免已经完整展示的板块掩盖消息自己的部分展示状态。
    # 【逐行批注】 在受控上下文中执行资源操作：with start_session(executable) as (info, api, _):。
    with start_session(executable) as (info, api, _):
        # 【逐行批注】 计算右侧结果并赋给 lesson_id，更新脚本状态。
        lesson_id=info['lessonId'];practices=[s for s in lesson()['sections'] if s.get('questionKind')=='practice']
        # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"long_teaching"…。
        with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"long_teaching":true}',headers={'Content-Type':'application/json'})):pass
        # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
        task=wait(api('/api/learning-agent/events',dict(type='teaching_request',requestType='worked_example',text='请完整示范',
            # 【逐行批注】 计算右侧结果并赋给 courseId，更新脚本状态。
            courseId=info['courseId'],lessonId=lesson_id,sectionId=practices[0]['id'],sectionVersion=practices[0]['version'],view='practice',requestId='long-display-message')))
        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as p:。
        with sync_playwright() as p:
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser=p.chromium.launch(headless=True);page=browser.new_page(viewport={'width':390,'height':650},reduced_motion='reduce')
            # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
            errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/practice?lessonId，更新脚本状态。
            page.goto(info['base']+'/practice?lessonId='+lesson_id);expect(page.locator('#lesson-input')).to_be_enabled()
            # 【逐行批注】 计算右侧结果并赋给 node，更新脚本状态。
            node=page.locator('.chat-message.assistant .chat-message-body').filter(has_text='示范：Fe').first
            # 【逐行批注】 计算右侧结果并赋给 node.evaluate('(n)，更新脚本状态。
            node.evaluate('(n)=>n.scrollIntoView({block:"start"})');page.evaluate('GangyiAgent.flushExposure()')
            # 【逐行批注】 调用 until、bool、metadata 完成当前脚本或测试步骤：until(lambda:bool(metadata('agent-exposure:'+task['id']).get('display…。
            until(lambda:bool(metadata('agent-exposure:'+task['id']).get('displayedSequences')))
            # 【逐行批注】 计算右侧结果并赋给 initial_answer，更新脚本状态。
            initial_answer=answer(practices[0])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert initial_answer['helpMode']=='help_unknown', (initial_answer['helpMode'],…。
            assert initial_answer['helpMode']=='help_unknown', (initial_answer['helpMode'],metadata('agent-exposure:'+task['id']))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer(practices[1])['helpMode']=='independent'。
            assert answer(practices[1])['helpMode']=='independent'
            # 【逐行批注】 调用 cover 完成当前脚本或测试步骤：cover(page,node)。
            cover(page,node)
            # 【逐行批注】 计算右侧结果并赋给 message_sequences，更新脚本状态。
            message_sequences=[e['seq'] for e in task['events'] if e.get('type')=='delta' and e.get('field')=='message']
            # 【逐行批注】 调用 until、all、metadata 完成当前脚本或测试步骤：until(lambda:all(s in metadata('agent-exposure:'+task['id']).get('com…。
            until(lambda:all(s in metadata('agent-exposure:'+task['id']).get('completeSequences',[]) for s in message_sequences))
            # 【逐行批注】 计算右侧结果并赋给 completed，更新脚本状态。
            completed=answer(practices[0])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert completed['helpMode']=='after_example', (completed['helpMode'],。
            assert completed['helpMode']=='after_example', (completed['helpMode'],
                # 【逐行批注】 调用 in 完成当前脚本或测试步骤：[{k:h.get(k) for k in ('helpType','partial','taskId','step')} for h i…。
                [{k:h.get(k) for k in ('helpType','partial','taskId','step')} for h in completed.get('helpHistory',[])],
                # 【逐行批注】 计算右侧结果并赋给 metadata('agent-exposure:'+task['id']),node.…，更新脚本状态。
                metadata('agent-exposure:'+task['id']),node.evaluate('(n)=>({height:n.getBoundingClientRect().height,top:n.getBoundingClientRect().top})'))
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：page.reload();expect(page.locator('#lesson-input')).to_be_enabled();p…。
            page.reload();expect(page.locator('#lesson-input')).to_be_enabled();page.evaluate('GangyiAgent.flushExposure()')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer(practices[0])['helpMode']=='after_example'。
            assert answer(practices[0])['helpMode']=='after_example'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()。
            assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()
            # 【逐行批注】 执行当前 Python 语句：browser.close();assert not errors,errors。
            browser.close();assert not errors,errors
        # 【逐行批注】 输出当前运行结果：print('长板块/消息部分→完整展示、刷新恢复、同题关联、其他题隔离和私有字段回归通过；付费调用0次。')。
        print('长板块/消息部分→完整展示、刷新恢复、同题关联、其他题隔离和私有字段回归通过；付费调用0次。')


# 【逐行批注】 判断条件 __name__=='__main__':，成立时执行对应分支。
if __name__=='__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encodin…，成立时执行对应分支。
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(sys.argv[1])。
    main(sys.argv[1])

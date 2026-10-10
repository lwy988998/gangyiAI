"""统一课堂的持续集成浏览器回归；使用隔离协议档案，不读取真实用户记录。"""
import json
import sqlite3
import sys
import time
from contextlib import closing
from urllib.request import Request, urlopen
from playwright.sync_api import expect, sync_playwright
from classroom_v560_fixture import start_session


def main(executable):
    with start_session(executable) as (info, api, process):
        # 旧课程同时保存作品任务，不能将旧总任务数当作课堂课时数。
        with closing(sqlite3.connect(info['database'])) as database:
            for identity, raw in database.execute('SELECT id,payload FROM CourseSnapshot').fetchall():
                payload=json.loads(raw)
                payload['generation']['promptVersion']='legacy-v5.5'
                payload['roadmap']=[dict(tasks=['作品任务'+str(i) for i in range(8)])]
                database.execute('UPDATE CourseSnapshot SET payload=? WHERE id=?',(json.dumps(payload,ensure_ascii=False),identity))
            database.commit()
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True)
            page = browser.new_page(viewport={'width':1440,'height':900},reduced_motion='reduce')
            errors=[]
            page.on('pageerror',lambda error:errors.append(str(error)))
            lesson=info['lessonId']
            page.goto(info['base']+'/plan?courseId='+info['courseId'])
            page.get_by_text('课程进度 · 已完成 0 / 4',exact=True).wait_for()
            page.goto(info['base']+'/learn?lessonId='+lesson)
            page.get_by_role('heading',name='先比较同一种元素',exact=True).wait_for()
            page.get_by_role('heading',name='说说你的观察',exact=True).wait_for()
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            expect(page.locator('.course-directory .course-directory-topic[aria-current=page]')).to_have_count(1)
            expect(page.locator('.course-directory-mobile .course-directory-topic[aria-current=page]')).to_have_count(1)
            before=api('/api/learning-agent/lesson?lessonId='+lesson)
            assert 'expectedAnswer' not in json.dumps(before) and 'rubric' not in json.dumps(before)
            assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()
            def fits():
                assert page.evaluate('document.documentElement.scrollWidth<=innerWidth && window.scrollY===0')
                composer=page.locator('#lesson-chat-panel').bounding_box()
                field_box=page.locator('#lesson-input').bounding_box()
                region=page.locator('#lesson-scroll').bounding_box()
                monitor=page.locator('#learning-navigation > summary').bounding_box()
                title=page.locator('#lesson-title').bounding_box()
                height=page.evaluate('innerHeight')
                assert composer and field_box and region and monitor and title
                assert title['width']>140 and title['height']<90
                assert region['height']>100 and field_box['y']>=0 and composer['y']+composer['height']<=height+1
                assert region['y']+region['height']<=composer['y']+1
                assert monitor['y']+monitor['height']<=composer['y']+1
            fits()
            field=page.locator('#lesson-input')
            field.fill('本段草稿，尚未提交。')
            page.reload()
            expect(page.locator('#lesson-input')).to_have_value('本段草稿，尚未提交。')
            calls_before=json.load(urlopen(info['fixture']))['calls']
            original_task=api('/api/learning-agent/lesson?lessonId='+lesson)['initialTeachingTaskId']
            for area in ('practice','summary','learn'):
                page.locator('[data-lesson-link='+area+']').click()
                expect(page.locator('[data-lesson-view]')).to_have_attribute('data-lesson-view', area)
                expect(field).to_have_value('本段草稿，尚未提交。' if area=='learn' else '')
                assert all(value==area for value in page.locator('#lesson-scroll [data-area]').evaluate_all('(nodes)=>nodes.map(node=>node.dataset.area)'))
                fits()
            assert json.load(urlopen(info['fixture']))['calls']==calls_before
            assert api('/api/learning-agent/lesson?lessonId='+lesson)['initialTeachingTaskId']==original_task
            page.locator('[data-lesson-link=practice]').click()
            practice=page.locator('.chat-section').filter(has=page.get_by_role('heading',name='练习一：判断变化',exact=True))
            expect(practice).to_have_count(1)
            practice.locator('input[type=radio]').first.check()
            field.fill('练习草稿，与讲解分开。')
            page.reload()
            expect(page.locator('#lesson-input')).to_have_value('练习草稿，与讲解分开。')
            practice=page.locator('.chat-section').filter(has=page.get_by_role('heading',name='练习一：判断变化',exact=True))
            expect(practice.locator('input[type=radio]').first).to_be_checked()
            page.locator('[data-lesson-link=summary]').click()
            page.get_by_text('本节尚无足够的实际作答评价；阅读和追问不表示掌握。',exact=True).wait_for()
            # 课时读取返回前不能提前输入，恢复完成后再写实际作答说明。
            loading_routes = []
            loading_pattern = '**/api/learning-agent/lesson?lessonId='+lesson
            def hold_loading(route):
                if not loading_routes and '/practice?' in route.request.frame.url:
                    assert field.is_disabled(), '课时读取尚未返回时必须锁定输入'
                    loading_routes.append(route.request.url)
                route.continue_()
            page.route(loading_pattern, hold_loading)
            page.locator('[data-lesson-link=practice]').click()
            expect(page).to_have_url(info['base']+'/practice?lessonId='+lesson)
            expect(page.locator('#page-status')).to_contain_text('本页独立保存交流和草稿')
            expect(field).to_be_enabled()
            assert loading_routes, '延迟加载场景必须实际拦截课时读取'
            page.unroute(loading_pattern, hold_loading)
            field.fill('我认为从0升高到+2。')
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',headers={'Content-Type':'application/json'})): pass
            practice.get_by_role('button',name='提交答案',exact=True).click()
            expect(page.locator('.chat-message.user .chat-message-body').last).to_contain_text('我选1')
            expect(page.locator('.chat-message.user .chat-message-body').last).to_contain_text('我认为从0升高到+2。')
            page.locator('#learning-navigation > summary').click()
            expect(page.locator('#agent-control-panel .ai-control-history')).not_to_have_attribute('open', '')
            expect(page.locator('#agent-control-panel .ai-task-details')).not_to_have_attribute('open', '')
            with page.expect_response(lambda response:response.url.endswith('/api/ai-activity/control') and response.request.post_data_json.get('command')=='pause') as paused_response:
                page.get_by_role('button',name='暂停全部 AI',exact=True).click()
            assert paused_response.value.status==200 and paused_response.value.json()['paused']
            expect(page.locator('#lesson-task-status')).to_contain_text('AI 已暂停')
            page.locator('.ai-context-details > summary').click()
            expect(page.locator('.ai-operation-title')).to_contain_text('已暂停')
            assert page.locator('.ai-operation-target').inner_text()
            assert 'PRIVATE-' not in page.locator('#agent-control-panel').inner_text()
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',headers={'Content-Type':'application/json'})): pass
            page.get_by_role('button',name='恢复全部 AI',exact=True).click()
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            page.get_by_role('heading',name='本次作答 · 80 分',exact=True).wait_for()
            page.locator('.chat-message.assistant').get_by_text('你已经给出了观察，接下来可以补充电子变化的理由。',exact=True).wait_for()
            # 评价必须先于后续讲解，展示顺序与服务端实际动作顺序一致。
            sequence=page.locator('#lesson-sections').evaluate("(root)=>[...root.children].map(node=>node.textContent)")
            grade_index=next(i for i,text in enumerate(sequence) if '本次作答 · 80 分' in text)
            explanation_index=next(i for i,text in enumerate(sequence) if '你已经给出了观察' in text)
            assert grade_index<explanation_index
            page.locator('.ai-control-history > summary').click()
            expect(page.locator('.ai-step-list')).to_contain_text('本次回答的评价')
            page.locator('#learning-navigation > summary').click()
            field.fill('为什么要先比较同一种元素？')
            page.locator('#lesson-send').click()
            page.locator('.chat-message.assistant').get_by_text('这是对思路的追问。我们可以先比较前后状态，再联系电子变化。',exact=True).wait_for()
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            assert page.locator('.chat-message.user').count()==2
            assert page.locator('.dialogue-history').count()==0
            page.reload()
            expect(page.locator('.chat-message.user')).to_have_count(2)
            page.locator('.chat-message.assistant').get_by_text('这是对思路的追问。我们可以先比较前后状态，再联系电子变化。',exact=True).wait_for()
            page.locator('[data-lesson-link=practice]').click()
            field.fill('请根据我的回答出一道选择题。')
            page.locator('#lesson-send').click()
            page.get_by_role('heading',name='根据你的回答：比较电子得失',exact=True).wait_for()
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            followup=page.locator('.chat-section').filter(has=page.get_by_role('heading',name='根据你的回答：比较电子得失',exact=True))
            expect(followup.locator('input[type=radio]')).to_have_count(2)
            followup.locator('input[type=radio]').last.check()
            field.fill('新题尚未提交的解释。')
            page.reload()
            expect(page.locator('#lesson-input')).to_have_value('新题尚未提交的解释。')
            expect(followup.locator('input[type=radio]').last).to_be_checked()
            assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()
            # 已看历史时，新流式回复不强行拖回底部；正文增长也不移动输入栏。
            field.fill('')
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',headers={'Content-Type':'application/json'})): pass
            page.locator('#continue-teaching').click()
            expect(page.locator('#lesson-stop')).to_be_visible()
            page.locator('#lesson-scroll').evaluate('(node)=>node.scrollTop=0')
            top_before=page.locator('#lesson-scroll').evaluate('(node)=>node.scrollTop')
            field_before=field.bounding_box()
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问',timeout=30000)
            assert page.get_by_role('heading',name='把化合价变化联系到电子',exact=True).count()==0
            assert abs(page.locator('#lesson-scroll').evaluate('(node)=>node.scrollTop')-top_before)<2
            assert abs(field.bounding_box()['y']-field_before['y'])<2
            expect(page.locator('#lesson-latest')).to_be_visible()
            page.locator('#lesson-latest').click()
            assert page.locator('#lesson-scroll').evaluate('(node)=>node.scrollHeight-node.scrollTop-node.clientHeight')<2
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',headers={'Content-Type':'application/json'})): pass
            from pathlib import Path
            screenshots=Path(__file__).resolve().parents[1]/'verification'
            page.screenshot(path=str(screenshots/'chat-classroom-1440.png'))
            for width,height in ((960,600),(390,844)):
                page.set_viewport_size({'width':width,'height':height})
                fits()
                if width==390:
                    assert page.locator('.course-directory-mobile').is_visible() and not page.locator('.course-directory').is_visible()
                page.screenshot(path=str(screenshots/('chat-classroom-'+str(width)+'.png')))
            page.set_viewport_size({'width':1440,'height':900})
            page.locator('#composer-chat').click()
            field.fill('为什么停止后还能继续追问？')
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',headers={'Content-Type':'application/json'})): pass
            page.locator('#lesson-send').click()
            page.wait_for_function("document.querySelector('#lesson-stop:not([hidden])') && [...document.querySelectorAll('.chat-message.assistant')].at(-1).textContent.length>25")
            interrupted_task=api('/api/learning-agent/lesson?lessonId='+lesson)['teachingTaskId']
            page.locator('#lesson-stop').click()
            expect(page.locator('#lesson-task-status')).to_contain_text('已停止本次回答')
            expect(page.locator('#lesson-send')).to_be_enabled()
            expect(page.locator('#lesson-retry')).to_be_visible()
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',headers={'Content-Type':'application/json'})): pass
            page.locator('#lesson-retry').click()
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问',timeout=30000)
            assert api('/api/learning-agent/lesson?lessonId='+lesson)['teachingTaskId']==interrupted_task
            assert page.locator('.chat-message.user').filter(has_text='为什么停止后还能继续追问？').count()==1
            # 不可自动恢复的服务商错误保留同次输入，修正连接后重试同一个任务。
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"http_error":402}',headers={'Content-Type':'application/json'})): pass
            field.fill('为什么请求失败时要保留输入？')
            page.locator('#lesson-send').click()
            expect(page.locator('#lesson-retry')).to_be_visible(timeout=30000)
            failed_task=api('/api/learning-agent/lesson?lessonId='+lesson)['teachingTaskId']
            assert api('/api/learning-agent?taskId='+failed_task)['failure']['httpStatus']==402
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{}',headers={'Content-Type':'application/json'})): pass
            page.locator('#lesson-retry').click()
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问',timeout=30000)
            assert api('/api/learning-agent/lesson?lessonId='+lesson)['teachingTaskId']==failed_task
            assert page.locator('.chat-message.user').filter(has_text='为什么请求失败时要保留输入？').count()==1
            page.locator('[data-lesson-link=learn]').click()
            page.locator('#composer-chat').click()
            expect(page.locator('.chat-message.user')).to_have_count(0)
            field.fill('原知识点独立保存的草稿。')
            second_task = api('/api/learning-agent/events', dict(type='prepare_next', courseId=info['courseId'],
                requestId='browser-second-prepared-lesson', navigate=False))
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                second = api('/api/learning-agent?taskId='+second_task['id'])
                if second['status'] == 'ready': break
                time.sleep(.05)
            assert second['status'] == 'ready'
            other_lesson = second['lesson']['id']
            assert other_lesson != lesson
            # 模拟首次开课的焦点晚到：先写聊天草稿，再读取实际题目焦点。
            focus_route = '**/api/learning-agent/lesson?lessonId='+other_lesson
            def delay_focus(route):
                response = route.fetch()
                data = response.json()
                data['teachingFocus'] = {}
                route.fulfill(response=response, json=data)
            page.route(focus_route, delay_focus)
            page.goto(info['base']+'/learn?lessonId='+other_lesson)
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            expect(page.locator('#composer-target')).to_have_text('和 AI 老师交流')
            expect(page.locator('#lesson-input')).to_have_value('')
            page.locator('#lesson-input').fill('另一知识点的草稿。')
            page.unroute(focus_route, delay_focus)
            page.goto(info['base']+'/learn?lessonId='+lesson)
            expect(page.locator('#lesson-input')).to_have_value('原知识点独立保存的草稿。')
            page.goto(info['base']+'/learn?lessonId='+other_lesson)
            expect(page.locator('#composer-target')).to_have_text('和 AI 老师交流')
            expect(page.locator('#lesson-input')).to_have_value('另一知识点的草稿。')
            # 完成按钮使用真实主控判断；刷新后回复保留，不能伪造一条没有保存的学生消息。
            page.locator('[data-lesson-link=summary]').click()
            users_before = page.locator('.chat-message.user').count()
            page.locator('#finish-lesson').click()
            finish_reply = page.locator('.chat-message.assistant').get_by_text('请结合本节检查点回顾，已有记录会保留，缺少证据的部分继续练习。', exact=True)
            finish_reply.wait_for()
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            expect(page.locator('.chat-message.user')).to_have_count(users_before)
            page.reload()
            finish_reply.wait_for()
            expect(page.locator('.chat-message.user')).to_have_count(users_before)
            # 相同文字的旧交流与新交流都是实际记录，不能按正文去重丢掉旧轮次。
            with closing(sqlite3.connect(info['database'])) as database:
                saved_lesson = json.loads(database.execute('SELECT payload FROM ClassroomActivity WHERE id=?', (lesson,)).fetchone()[0])
                next(section for section in saved_lesson['sections'] if section.get('questionKind')=='practice')['legacyDialog'] = [dict(role='user', text='为什么停止后还能继续追问？', at=1)]
                database.execute('UPDATE ClassroomActivity SET payload=? WHERE id=?', (json.dumps(saved_lesson), lesson))
                database.commit()
            page.goto(info['base']+'/practice?lessonId='+lesson)
            expect(page.locator('.chat-message.user').filter(has_text='为什么停止后还能继续追问？')).to_have_count(2)
            user_keys = page.locator('.chat-message.user').filter(has_text='为什么停止后还能继续追问？').evaluate_all('(nodes)=>nodes.map(node=>node.dataset.entryKey)')
            assert user_keys[0].startswith('legacy:') and not user_keys[1].startswith('legacy:')
            page.goto(info['base']+'/learn?lessonId='+other_lesson)
            # 展示夹具直接改写课表前暂停后台，避免人工注入的计划触发独立画像任务。
            api('/api/learning-agent/control',dict(command='pause',taskId=second_task['id']))
            initial_plan = api('/api/study-plan')
            with closing(sqlite3.connect(info['database'])) as database:
                saved = database.execute("SELECT value FROM ProfileMeta WHERE key='learning-flow'").fetchone()
                raw = json.loads(saved[0]) if saved else dict(plan=initial_plan)
                scheduled = [dict(taskId='agent-lesson:'+identity, kind='agent-lesson', lessonId=identity,
                    courseId=info['courseId'], phaseIndex=1, topicIndex=1, title=title, date='2099-01-05', minutes=10, order=index)
                    for index,(identity,title) in enumerate([(lesson,'已保存课堂甲'),(other_lesson,'已保存课堂乙')])]
                raw['plan']['entries'] = scheduled
                candidate = json.loads(json.dumps(raw['plan']))
                del candidate['entries'][0]['title']
                raw.update(agentControlled=True,status='ready',proposal=dict(id='display-only-proposal',plan=candidate,
                    previousPlan=raw['plan'],reason='保留 '+scheduled[0]['taskId']+' 与 '+scheduled[1]['taskId']))
                database.execute("INSERT INTO ProfileMeta(key,value) VALUES('learning-flow',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value",(json.dumps(raw),))
                database.commit()
            page.set_viewport_size({'width':1440,'height':900})
            page.goto(info['base']+'/my-courses')
            links = page.locator('#weekly-entries .weekly-row a')
            expect(links).to_have_count(2)
            assert links.nth(0).get_attribute('href') == '/learn?lessonId='+lesson
            assert links.nth(1).get_attribute('href') == '/learn?lessonId='+other_lesson
            preview = page.locator('#weekly-confirm')
            expect(preview).to_be_visible()
            assert 'undefined' not in preview.inner_text()
            assert all(item['taskId'] not in preview.inner_text() for item in scheduled)
            # 过期但完整保存的课件仍可回看，入课保护继续限制新的教学操作。
            with closing(sqlite3.connect(info['database'])) as database:
                raw = json.loads(database.execute('SELECT payload FROM ClassroomActivity WHERE id=?',(other_lesson,)).fetchone()[0])
                raw['entered'] = False
                raw['sourceLearningVersion'] = -1
                database.execute('UPDATE ClassroomActivity SET payload=? WHERE id=?',(json.dumps(raw),other_lesson))
                database.commit()
            calls_before = json.load(urlopen(info['fixture']))['calls']
            for view in ('learn','practice','summary'):
                page.goto(info['base']+'/'+view+'?lessonId='+other_lesson)
                expect(page.locator('#lesson-title')).to_have_text(raw['title'])
                expect(page.locator('#page-status')).to_contain_text('你可以回看已保存内容')
                assert page.locator('#lesson-sections').inner_text()
                assert [page.locator('[data-lesson-link]').nth(index).get_attribute('data-lesson-link') for index in range(3)] == ['learn','practice','summary']
                for button in page.locator('#lesson-sections form button').all(): expect(button).to_be_disabled()
            assert json.load(urlopen(info['fixture']))['calls'] == calls_before
            # 开启动画且正文增长时，悬浮控制仍固定在可见窗口内。
            page.emulate_media(reduced_motion='no-preference')
            page.goto(info['base']+'/practice?lessonId='+lesson)
            page.locator('[data-lesson-link=practice]').click()
            page.locator('.chat-section[data-area=practice]:visible').first.wait_for()
            page.wait_for_function("getComputedStyle(document.body).opacity==='1'")
            toggle = page.locator('#learning-navigation > summary')
            before = toggle.bounding_box()
            page.evaluate("""() => {
                const paragraph=document.createElement('p');
                paragraph.textContent='这是一段持续增长的课堂讲解，正文完整保留。'.repeat(600);
                document.querySelector('#lesson-sections').append(paragraph);
            }""")
            after = toggle.bounding_box()
            assert before and after and abs(before['y']-after['y']) < 1
            assert after['y'] >= 0 and after['y']+after['height'] <= 900
            toggle.click()
            expect(page.locator('#agent-control-panel')).to_be_visible()
            assert not errors,errors
            browser.close()
        with closing(sqlite3.connect(info['database'])) as database:
            values=[json.loads(row[0]) for row in database.execute("SELECT payload FROM LearningInteraction WHERE kind='practice'")]
            assert len(values)==1 and values[0]['response'].startswith('我选1') and values[0]['response'].endswith('我认为从0升高到+2。'), values
            assert values[0]['assisted'] and values[0]['model']=='fixture-v560'
        print('聊天课堂、固定输入、内容定位、卡片作答、分题草稿、全局暂停恢复、评价顺序、历史回看与窄屏回归通过（隔离协议夹具）')


if __name__=='__main__':
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])

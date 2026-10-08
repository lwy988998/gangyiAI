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
            page.locator('#lesson-sections').get_by_text('你可以继续回答或追问',exact=True).wait_for()
            # 桌面目录和窄屏目录各保留一份当前项，CSS 决定显示哪一份。
            expect(page.locator('.course-directory .course-directory-topic[aria-current=page]')).to_have_count(1)
            expect(page.locator('.course-directory-mobile .course-directory-topic[aria-current=page]')).to_have_count(1)
            assert page.locator('.current-explanation').count()==1
            assert page.locator('#lesson-sections').inner_text().find('PRIVATE-')<0
            before=api('/api/learning-agent/lesson?lessonId='+lesson)
            assert 'expectedAnswer' not in json.dumps(before) and 'rubric' not in json.dumps(before)
            field=page.get_by_role('textbox',name='你的答案、解题过程或追问')
            field.fill('本段草稿，尚未提交。')
            page.reload()
            field=page.get_by_role('textbox',name='你的答案、解题过程或追问')
            assert field.input_value()=='本段草稿，尚未提交。'
            page.get_by_role('link',name='练习',exact=True).click()
            page.get_by_role('heading',name='练习一：判断变化',exact=True).wait_for()
            field=page.get_by_role('textbox',name='你的答案、解题过程或追问')
            field.fill('练习草稿，不同于讲解。')
            page.get_by_role('button',name='下一题 →',exact=True).click()
            assert page.locator('.current-practice').count()==1
            page.get_by_role('button',name='← 上一题',exact=True).click()
            assert field.input_value()=='练习草稿，不同于讲解。'
            page.get_by_role('link',name='小结',exact=True).click()
            page.get_by_text('本节尚无足够的实际作答评价；阅读和追问不表示掌握。',exact=True).wait_for()
            page.get_by_role('link',name='练习',exact=True).click()
            field=page.get_by_role('textbox',name='你的答案、解题过程或追问')
            field.fill('我认为从0升高到+2。')
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',headers={'Content-Type':'application/json'})): pass
            page.get_by_role('button',name='发送给 AI',exact=True).click()
            page.locator('#learning-navigation > summary').click()
            with page.expect_response(lambda response:response.url.endswith('/api/learning-agent/control') and response.request.post_data_json.get('command')=='pause') as paused_response:
                page.get_by_role('button',name='暂停 AI',exact=True).click()
            assert paused_response.value.status == 200
            paused_task = paused_response.value.json()
            assert api('/api/learning-agent?taskId='+paused_task['id'])['paused']
            page.locator('.current-practice').get_by_text('AI 已暂停',exact=True).wait_for()
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',headers={'Content-Type':'application/json'})): pass
            page.get_by_role('button',name='恢复 AI',exact=True).click()
            page.locator('#lesson-sections').get_by_text('你已经给出了观察，接下来可以补充电子变化的理由。',exact=True).wait_for()
            page.locator('#lesson-sections').get_by_text('你可以继续回答或追问',exact=True).wait_for()
            page.locator('#learning-navigation > summary').click()
            field.fill('为什么要先比较同一种元素？')
            page.get_by_role('button',name='发送给 AI',exact=True).click()
            page.locator('#lesson-sections').get_by_text('这是对思路的追问。我们可以先比较前后状态，再联系电子变化。',exact=True).wait_for()
            page.locator('#lesson-sections').get_by_text('你可以继续回答或追问',exact=True).wait_for()
            assert page.locator('.current-practice .agent-dialog > .agent-message.user').count()==1
            assert page.locator('.dialogue-history').count()==1
            page.reload()
            page.locator('#lesson-sections').get_by_text('这是对思路的追问。我们可以先比较前后状态，再联系电子变化。',exact=True).wait_for()
            page.get_by_role('link',name='小结',exact=True).click()
            page.get_by_role('heading',name='本次作答 · 80 分',exact=True).wait_for()
            page.get_by_role('link',name='练习',exact=True).click()
            field=page.get_by_role('textbox',name='你的答案、解题过程或追问')
            field.fill('请根据我的回答出一道选择题。')
            page.get_by_role('button',name='发送给 AI',exact=True).click()
            page.get_by_role('heading',name='根据你的回答：比较电子得失',exact=True).wait_for()
            expect(page.locator('.current-practice input[type=radio]')).to_have_count(2)
            page.reload()
            page.get_by_role('heading',name='根据你的回答：比较电子得失',exact=True).wait_for()
            expect(page.locator('.current-practice input[type=radio]')).to_have_count(2)
            assert 'PRIVATE-' not in page.locator('#lesson-sections').inner_text()
            page.get_by_role('link',name='讲解',exact=True).click()
            page.get_by_role('button',name='请 AI 继续 →',exact=True).click()
            page.get_by_role('heading',name='把化合价变化联系到电子',exact=True).wait_for()
            page.set_viewport_size({'width':390,'height':844})
            assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
            assert page.locator('.course-directory-mobile').is_visible()
            assert not page.locator('.course-directory').is_visible()
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
                assert all(page.locator('[data-lesson-link]').nth(index).get_attribute('href').endswith('lessonId='+other_lesson) for index in range(3))
                for button in page.locator('#lesson-sections form button').all(): expect(button).to_be_disabled()
            assert json.load(urlopen(info['fixture']))['calls'] == calls_before
            # 开启动画且正文增长时，悬浮控制仍固定在可见窗口内。
            page.emulate_media(reduced_motion='no-preference')
            page.goto(info['base']+'/practice?lessonId='+lesson)
            page.locator('.current-practice').wait_for()
            page.wait_for_function("getComputedStyle(document.body).opacity==='1'")
            toggle = page.locator('#learning-navigation > summary')
            before = toggle.bounding_box()
            page.evaluate("""() => {
                const paragraph=document.createElement('p');
                paragraph.textContent='这是一段持续增长的课堂讲解，正文完整保留。'.repeat(600);
                document.querySelector('.current-practice .agent-dialog').append(paragraph);
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
            assert len(values)==1 and values[0]['response']=='我认为从0升高到+2。'
            assert values[0]['assisted'] and values[0]['model']=='fixture-v560'
        print('C 课堂、目录、三页切换、一题一屏、分题草稿、连续交流、纯追问不评分与窄屏回归通过（隔离协议夹具）')


if __name__=='__main__':
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])

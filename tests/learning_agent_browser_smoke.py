# 【逐行批注】 执行当前 Python 语句："""统一课堂的持续集成浏览器回归；使用隔离协议档案，不读取真实用户记录。"""。
"""统一课堂的持续集成浏览器回归；使用隔离协议档案，不读取真实用户记录。"""
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
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import parse_qs, urlsplit。
from urllib.parse import parse_qs, urlsplit
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen
# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import expect, sync_playwright。
from playwright.sync_api import expect, sync_playwright
# 【逐行批注】 导入后续逻辑需要的模块或符号：from classroom_v560_fixture import start_session。
from classroom_v560_fixture import launch_test_browser, open_learning_menu, start_session


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable):
    # 【逐行批注】 在受控上下文中执行资源操作：with start_session(executable) as (info, api, process):。
    with start_session(executable) as (info, api, process):
        # 旧课程同时保存作品任务，不能将旧总任务数当作课堂课时数。
        # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as database:。
        with closing(sqlite3.connect(info['database'])) as database:
            # 【逐行批注】 遍历数据并重复执行循环体：for identity, raw in database.execute('SELECT id,payload FROM CourseSnapsh…。
            for identity, raw in database.execute('SELECT id,payload FROM CourseSnapshot').fetchall():
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload=json.loads(raw)
                # 【逐行批注】 计算右侧结果并赋给 payload['generation']['promptVersion']，更新脚本状态。
                payload['generation']['promptVersion']='legacy-v5.5'
                # 【逐行批注】 计算右侧结果并赋给 payload['roadmap']，更新脚本状态。
                payload['roadmap']=[dict(tasks=['作品任务'+str(i) for i in range(8)])]
                # 【逐行批注】 计算右侧结果并赋给 database.execute('UPDATE CourseSnapshot SET …，更新脚本状态。
                database.execute('UPDATE CourseSnapshot SET payload=? WHERE id=?',(json.dumps(payload,ensure_ascii=False),identity))
            # 【逐行批注】 执行当前 Python 语句：database.commit()。
            database.commit()
        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as playwright:。
        with sync_playwright() as playwright:
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser = launch_test_browser(playwright)
            # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
            page = browser.new_page(viewport={'width':1440,'height':900},reduced_motion='reduce')
            # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
            errors=[]
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：page.on('pageerror',lambda error:errors.append(str(error)))。
            page.on('pageerror',lambda error:errors.append(str(error)))
            # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
            lesson=info['lessonId']
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/plan?courseId，更新脚本状态。
            page.goto(info['base']+'/plan?courseId='+info['courseId'])
            # 【逐行批注】 计算右侧结果并赋给 page.get_by_text('课程进度 · 已完成 0 / 4',exact，更新脚本状态。
            page.get_by_text('课程进度 · 已完成 0 / 4',exact=True).wait_for()
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/learn?lessonId，更新脚本状态。
            page.goto(info['base']+'/learn?lessonId='+lesson)
            # 【逐行批注】 计算右侧结果并赋给 page.get_by_role('heading',name，更新脚本状态。
            page.locator('.chat-section').get_by_role('heading',name='先比较同一种元素',exact=True).wait_for()
            # 【逐行批注】 计算右侧结果并赋给 page.get_by_role('heading',name，更新脚本状态。
            page.locator('.chat-section').get_by_role('heading',name='说说你的观察',exact=True).wait_for()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            # 【逐行批注】 计算右侧结果并赋给 expect(page.locator('.course-directory .cour…，更新脚本状态。
            expect(page.locator('.course-directory .course-directory-topic[aria-current=page]')).to_have_count(1)
            # 【逐行批注】 计算右侧结果并赋给 expect(page.locator('.course-directory-mobil…，更新脚本状态。
            expect(page.locator('.course-directory-mobile .course-directory-topic[aria-current=page]')).to_have_count(1)
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before=api('/api/learning-agent/lesson?lessonId='+lesson)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'expectedAnswer' not in json.dumps(before) and 'rubric' not in json.dump…。
            assert 'expectedAnswer' not in json.dumps(before) and 'rubric' not in json.dumps(before)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()。
            assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()
            # 【逐行批注】 定义函数 fits，封装可复用的处理步骤。
            def fits():
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.evaluate('document.documentElement.scrollWidth<=innerWidth && windo…。
                assert page.evaluate('document.documentElement.scrollWidth<=innerWidth && window.scrollY===0')
                # 【逐行批注】 计算右侧结果并赋给 composer，更新脚本状态。
                composer=page.locator('#lesson-chat-panel').bounding_box()
                # 【逐行批注】 计算右侧结果并赋给 field_box，更新脚本状态。
                field_box=page.locator('#lesson-input').bounding_box()
                # 【逐行批注】 计算右侧结果并赋给 region，更新脚本状态。
                region=page.locator('#lesson-scroll').bounding_box()
                # 【逐行批注】 计算右侧结果并赋给 monitor，更新脚本状态。
                monitor=page.get_by_role('button', name='操作菜单', exact=True).bounding_box()
                # 【逐行批注】 计算右侧结果并赋给 title，更新脚本状态。
                title=page.locator('#lesson-title').bounding_box()
                # 【逐行批注】 计算右侧结果并赋给 height，更新脚本状态。
                height=page.evaluate('innerHeight')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert composer and field_box and region and monitor and title。
                assert composer and field_box and region and monitor and title
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert title['width']>140 and title['height']<90。
                assert title['width']>0 and title['height']<=25
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert region['height']>100 and field_box['y']>=0 and composer['y']+composer['h…。
                assert region['height']>100 and field_box['y']>=0 and composer['y']+composer['height']<=height+1
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert region['y']+region['height']<=composer['y']+1。
                assert region['y']+region['height']<=composer['y']+1
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert monitor['y']+monitor['height']<=composer['y']+1。
                assert monitor['y']+monitor['height']<=composer['y']+1
            # 【逐行批注】 调用 fits 完成当前脚本或测试步骤：fits()。
            fits()
            # 【逐行批注】 计算右侧结果并赋给 field，更新脚本状态。
            field=page.locator('#lesson-input')
            # 【逐行批注】 执行当前 Python 语句：field.fill('本段草稿，尚未提交。')。
            field.fill('本段草稿，尚未提交。')
            # 【逐行批注】 执行当前 Python 语句：page.reload()。
            page.reload()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_have_value('本段草稿，尚未提交。')。
            expect(page.locator('#lesson-input')).to_have_value('本段草稿，尚未提交。')
            # 【逐行批注】 计算右侧结果并赋给 original_task，更新脚本状态。
            original_task=api('/api/learning-agent/lesson?lessonId='+lesson)['initialTeachingTaskId']
            # 先等实际教学任务终态，再比较导航前后服务端真实接收的模型请求数。
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline=time.monotonic()+30
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic()<deadline:。
            while time.monotonic()<deadline:
                # 【逐行批注】 计算右侧结果并赋给 settled，更新脚本状态。
                settled=api('/api/learning-agent?taskId='+original_task)
                # 【逐行批注】 判断条件 settled['status'] not in ('pending','running'): break，成立时执行对应分支。
                if settled['status'] not in ('pending','running'): break
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.05)。
                time.sleep(.05)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert settled['status'] in ('ready','waiting_student'), settled.get('error')。
            assert settled['status'] in ('ready','waiting_student'), settled.get('error')
            # 【逐行批注】 计算右侧结果并赋给 calls_before，更新脚本状态。
            calls_before=json.load(urlopen(info['fixture']))['calls']
            # 【逐行批注】 遍历数据并重复执行循环体：for area in ('practice','summary','learn'):。
            for area in ('practice','summary','learn'):
                # 【逐行批注】 计算右侧结果并赋给 page.locator('[data-lesson-link，更新脚本状态。
                open_learning_menu(page).locator('[data-lesson-link='+area+']').click()
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('[data-lesson-view]')).to_have_attribute('data-le…。
                expect(page.locator('[data-lesson-view]')).to_have_attribute('data-lesson-view', area)
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(field).to_have_value('本段草稿，尚未提交。' if area=='learn' else '')。
                expect(field).to_have_value('本段草稿，尚未提交。' if area=='learn' else '')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(value==area for value in page.locator('#lesson-scroll [data-area]').…。
                assert all(value==area for value in page.locator('#lesson-scroll [data-area]').evaluate_all('(nodes)=>nodes.map(node=>node.dataset.area)'))
                # 【逐行批注】 调用 fits 完成当前脚本或测试步骤：fits()。
                fits()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert json.load(urlopen(info['fixture']))['calls']==calls_before。
            assert json.load(urlopen(info['fixture']))['calls']==calls_before
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learning-agent/lesson?lessonId='+lesson)['initialTeachingTaskI…。
            assert api('/api/learning-agent/lesson?lessonId='+lesson)['initialTeachingTaskId']==original_task
            # 使用实际服务公开的地址，跨过服务权限校验和真实前端，而非替换 GangyiAgent。
            # 【逐行批注】 计算右侧结果并赋给 public_lesson，更新脚本状态。
            public_lesson=api('/api/learning-agent/lesson?lessonId='+lesson)
            # 【逐行批注】 计算右侧结果并赋给 saved_section，更新脚本状态。
            saved_section=next(section for section in public_lesson['sections'] if section['title']=='把化合价变化联系到电子')
            # 【逐行批注】 计算右侧结果并赋给 href，更新脚本状态。
            href=saved_section['href']; destination=urlsplit(href); identity=saved_section['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert destination.path=='/'+saved_section['view']=='/learn'。
            assert destination.path=='/'+saved_section['view']=='/learn'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert parse_qs(destination.query)['lessonId']==[lesson] and parse_qs(destinati…。
            assert parse_qs(destination.query)['lessonId']==[lesson] and parse_qs(destination.query)['sectionId']==[identity]
            # 【逐行批注】 计算右侧结果并赋给 target，更新脚本状态。
            target=page.locator('[data-section-id="'+identity+'"]')
            # 【逐行批注】 定义函数 located_saved_section，封装可复用的处理步骤。
            def located_saved_section():
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(target).to_have_count(1)。
                expect(target).to_have_count(1)
                # 【逐行批注】 计算右侧结果并赋给 expect(target.get_by_role('heading',name，更新脚本状态。
                expect(target.get_by_role('heading',name=saved_section['title'],exact=True)).to_be_visible()
                # 【逐行批注】 计算右侧结果并赋给 page.wait_for_function("""identity，更新脚本状态。
                page.wait_for_function("""identity=>{const target=[...document.querySelectorAll('[data-section-id]')].find(node=>node.dataset.sectionId===identity);
                    if(!target)return false;const box=target.getBoundingClientRect(),viewport=document.getElementById('lesson-scroll').getBoundingClientRect();
                    return box.top>=viewport.top-2&&box.top<viewport.bottom-20;}""",arg=identity)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert target.locator('xpath=ancestor::details').evaluate_all('(nodes)=>nodes.e…。
                assert target.locator('xpath=ancestor::details').evaluate_all('(nodes)=>nodes.every(node=>node.open)')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('#lesson-scroll').evaluate('node=>node.scrollTop')>0。
                assert page.locator('#lesson-scroll').evaluate('node=>node.scrollTop')>0
            # 【逐行批注】 执行当前 Python 语句：page.goto(info['base']+href)。
            page.goto(info['base']+href)
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('[data-lesson-view]')).to_have_attribute('data-le…。
            expect(page.locator('[data-lesson-view]')).to_have_attribute('data-lesson-view',saved_section['view'])
            # 【逐行批注】 调用 located_saved_section 完成当前脚本或测试步骤：located_saved_section()。
            located_saved_section()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(field).to_have_value('本段草稿，尚未提交。')。
            expect(field).to_have_value('本段草稿，尚未提交。')
            # 【逐行批注】 计算右侧结果并赋给 permalink，更新脚本状态。
            permalink=target.get_by_role('link',name='直达此段',exact=True)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert permalink.get_attribute('href')==href。
            assert permalink.get_attribute('href')==href
            # 在远离目标的滚动位置激活实际链接，避免 Playwright 先自动滚到链接而掩盖定位缺陷。
            # 【逐行批注】 计算右侧结果并赋给 page.locator('#lesson-scroll').evaluate('node，更新脚本状态。
            page.locator('#lesson-scroll').evaluate('node=>node.scrollTop=0')
            # 【逐行批注】 计算右侧结果并赋给 permalink.evaluate('link，更新脚本状态。
            permalink.evaluate('link=>link.click()')
            # 【逐行批注】 调用 located_saved_section 完成当前脚本或测试步骤：located_saved_section()。
            located_saved_section()
            # 【逐行批注】 调用 located_saved_section 完成当前脚本或测试步骤：page.reload(); located_saved_section()。
            page.reload(); located_saved_section()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(field).to_have_value('本段草稿，尚未提交。')。
            expect(field).to_have_value('本段草稿，尚未提交。')
            # 【逐行批注】 遍历数据并重复执行循环体：for area in ('practice','summary','learn'):。
            for area in ('practice','summary','learn'):
                # 【逐行批注】 计算右侧结果并赋给 page.locator('[data-lesson-link，更新脚本状态。
                open_learning_menu(page).locator('[data-lesson-link='+area+']').click()
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('[data-lesson-view]')).to_have_attribute('data-le…。
                expect(page.locator('[data-lesson-view]')).to_have_attribute('data-lesson-view',area)
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(field).to_be_enabled()。
                expect(field).to_be_enabled()
            # 【逐行批注】 调用 located_saved_section 完成当前脚本或测试步骤：page.goto(info['base']+href); located_saved_section()。
            page.goto(info['base']+href); located_saved_section()
            # 【逐行批注】 计算右侧结果并赋给 after_navigation，更新脚本状态。
            after_navigation=api('/api/learning-agent/lesson?lessonId='+lesson)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert after_navigation['teachingTaskId']==public_lesson['teachingTaskId']。
            assert after_navigation['teachingTaskId']==public_lesson['teachingTaskId']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert after_navigation['initialTeachingTaskId']==original_task。
            assert after_navigation['initialTeachingTaskId']==original_task
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learning-agent?taskId='+original_task)['status'] in ('ready','…。
            assert api('/api/learning-agent?taskId='+original_task)['status'] in ('ready','waiting_student')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert json.load(urlopen(info['fixture']))['calls']==calls_before。
            assert json.load(urlopen(info['fixture']))['calls']==calls_before
            # 【逐行批注】 计算右侧结果并赋给 page.locator('[data-lesson-link，更新脚本状态。
            open_learning_menu(page).locator('[data-lesson-link=practice]').click()
            # 【逐行批注】 计算右侧结果并赋给 practice，更新脚本状态。
            practice=page.locator('.chat-section').filter(has=page.get_by_role('heading',name='练习一：判断变化',exact=True))
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(practice).to_have_count(1)。
            expect(practice).to_have_count(1)
            # 【逐行批注】 计算右侧结果并赋给 practice.locator('input[type，更新脚本状态。
            practice.locator('input[type=radio]').first.check()
            # 【逐行批注】 执行当前 Python 语句：field.fill('练习草稿，与讲解分开。')。
            field.fill('练习草稿，与讲解分开。')
            # 【逐行批注】 执行当前 Python 语句：page.reload()。
            page.reload()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_have_value('练习草稿，与讲解分开。')。
            expect(page.locator('#lesson-input')).to_have_value('练习草稿，与讲解分开。')
            # 【逐行批注】 计算右侧结果并赋给 practice，更新脚本状态。
            practice=page.locator('.chat-section').filter(has=page.get_by_role('heading',name='练习一：判断变化',exact=True))
            # 【逐行批注】 计算右侧结果并赋给 expect(practice.locator('input[type，更新脚本状态。
            expect(practice.locator('input[type=radio]').first).to_be_checked()
            # 【逐行批注】 计算右侧结果并赋给 page.locator('[data-lesson-link，更新脚本状态。
            open_learning_menu(page).locator('[data-lesson-link=summary]').click()
            # 【逐行批注】 计算右侧结果并赋给 page.get_by_text('本节尚无足够的实际作答评价；阅读和追问不表示掌握。'…，更新脚本状态。
            page.get_by_text('本节尚无足够的实际作答评价；阅读和追问不表示掌握。',exact=True).wait_for()
            # 课时读取返回前不能提前输入，恢复完成后再写实际作答说明。
            # 【逐行批注】 计算右侧结果并赋给 loading_routes，更新脚本状态。
            loading_routes = []
            # 【逐行批注】 计算右侧结果并赋给 loading_pattern，更新脚本状态。
            loading_pattern = '**/api/learning-agent/lesson?lessonId='+lesson
            # 【逐行批注】 定义函数 hold_loading，封装可复用的处理步骤。
            def hold_loading(route):
                # 【逐行批注】 判断条件 not loading_routes and '/practice?' in route.request.frame.url:，成立时执行对应分支。
                if not loading_routes and '/practice?' in route.request.frame.url:
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert field.is_disabled(), '课时读取尚未返回时必须锁定输入'。
                    assert field.is_disabled(), '课时读取尚未返回时必须锁定输入'
                    # 【逐行批注】 执行当前 Python 语句：loading_routes.append(route.request.url)。
                    loading_routes.append(route.request.url)
                # 【逐行批注】 执行当前 Python 语句：route.continue_()。
                route.continue_()
            # 【逐行批注】 执行当前 Python 语句：page.route(loading_pattern, hold_loading)。
            page.route(loading_pattern, hold_loading)
            # 【逐行批注】 计算右侧结果并赋给 page.locator('[data-lesson-link，更新脚本状态。
            open_learning_menu(page).locator('[data-lesson-link=practice]').click()
            # 【逐行批注】 计算右侧结果并赋给 expect(page).to_have_url(info['base']+'/prac…，更新脚本状态。
            expect(page).to_have_url(info['base']+'/practice?lessonId='+lesson)
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#page-status')).to_contain_text('本页独立保存交流和草稿')。
            expect(page.locator('#page-status')).to_contain_text('本页独立保存交流和草稿')
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(field).to_be_enabled()。
            expect(field).to_be_enabled()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert loading_routes, '延迟加载场景必须实际拦截课时读取'。
            assert loading_routes, '延迟加载场景必须实际拦截课时读取'
            # 【逐行批注】 执行当前 Python 语句：page.unroute(loading_pattern, hold_loading)。
            page.unroute(loading_pattern, hold_loading)
            # 【逐行批注】 执行当前 Python 语句：field.fill('我认为从0升高到+2。')。
            field.fill('我认为从0升高到+2。')
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',h…。
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',headers={'Content-Type':'application/json'})): pass
            # 【逐行批注】 计算右侧结果并赋给 practice.get_by_role('button',name，更新脚本状态。
            practice.get_by_role('button',name='提交答案',exact=True).click()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('.chat-message.user .chat-message-body').last).to…。
            expect(page.locator('.chat-message.user .chat-message-body').last).to_contain_text('我选1')
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('.chat-message.user .chat-message-body').last).to…。
            expect(page.locator('.chat-message.user .chat-message-body').last).to_contain_text('我认为从0升高到+2。')
            # 【逐行批注】 执行当前 Python 语句：page.locator('#learning-navigation > summary').click()。
            open_learning_menu(page, '全站导航与设置').locator('#learning-navigation > summary').click()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#agent-control-panel .ai-control-history')).not_…。
            expect(page.locator('#agent-control-panel .ai-control-history')).not_to_have_attribute('open', '')
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#agent-control-panel .ai-task-details')).not_to_…。
            expect(page.locator('#agent-control-panel .ai-task-details')).not_to_have_attribute('open', '')
            # 【逐行批注】 在受控上下文中执行资源操作：with page.expect_response(lambda response:response.url.endswith('/api/ai-activi…。
            with page.expect_response(lambda response:response.url.endswith('/api/ai-activity/control') and response.request.post_data_json.get('command')=='pause') as paused_response:
                # 【逐行批注】 计算右侧结果并赋给 page.get_by_role('button',name，更新脚本状态。
                page.get_by_role('button',name='暂停全部 AI',exact=True).click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert paused_response.value.status==200 and paused_response.value.json()['paus…。
            assert paused_response.value.status==200 and paused_response.value.json()['paused']
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_contain_text('AI 已暂停')。
            expect(page.locator('#lesson-task-status')).to_contain_text('AI 已暂停')
            # 【逐行批注】 执行当前 Python 语句：page.locator('.ai-context-details > summary').click()。
            detail = page.locator('.ai-context-details')
            if detail.get_attribute('open') is None:
                detail.locator('summary').first.click()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('.ai-operation-title')).to_contain_text('已暂停')。
            expect(page.locator('.ai-operation-title')).to_contain_text('已暂停')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('.ai-operation-target').inner_text()。
            expect(page.locator('.ai-operation-target')).to_contain_text('化合价与电子变化')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'PRIVATE-' not in page.locator('#agent-control-panel').inner_text()。
            assert 'PRIVATE-' not in page.locator('#agent-control-panel').inner_text()
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',…。
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',headers={'Content-Type':'application/json'})): pass
            # 【逐行批注】 计算右侧结果并赋给 page.get_by_role('button',name，更新脚本状态。
            page.get_by_role('button',name='恢复全部 AI',exact=True).click()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            # 【逐行批注】 计算右侧结果并赋给 page.get_by_role('heading',name，更新脚本状态。
            page.get_by_role('heading',name='本次作答 · 80 分',exact=True).wait_for()
            # 【逐行批注】 计算右侧结果并赋给 page.locator('.chat-message.assistant').get_…，更新脚本状态。
            page.locator('.chat-message.assistant').get_by_text('AI补充：\n你已经给出了观察，接下来可以补充电子变化的理由。',exact=True).wait_for()
            # 评价必须先于后续讲解，展示顺序与服务端实际动作顺序一致。
            # 【逐行批注】 计算右侧结果并赋给 sequence，更新脚本状态。
            sequence=page.locator('#lesson-sections').evaluate("(root)=>[...root.children].map(node=>node.textContent)")
            # 【逐行批注】 计算右侧结果并赋给 grade_index，更新脚本状态。
            grade_index=next(i for i,text in enumerate(sequence) if '本次作答 · 80 分' in text)
            # 【逐行批注】 计算右侧结果并赋给 explanation_index，更新脚本状态。
            explanation_index=next(i for i,text in enumerate(sequence) if '你已经给出了观察' in text)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert grade_index<explanation_index。
            assert grade_index<explanation_index
            # 【逐行批注】 执行当前 Python 语句：page.locator('.ai-control-history > summary').click()。
            page.locator('.ai-control-history > summary').click()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('.ai-step-list')).to_contain_text('本次回答的评价')。
            expect(page.locator('.ai-step-list')).to_contain_text('本次回答的评价')
            # 【逐行批注】 执行当前 Python 语句：page.locator('#learning-navigation > summary').click()。
            page.locator('#learning-navigation > summary').click()
            page.keyboard.press('Escape')
            # 【逐行批注】 执行当前 Python 语句：field.fill('为什么要先比较同一种元素？')。
            field.fill('为什么要先比较同一种元素？')
            # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-send').click()。
            page.locator('#lesson-send').click()
            # 【逐行批注】 计算右侧结果并赋给 page.locator('.chat-message.assistant').get_…，更新脚本状态。
            page.locator('.chat-message.assistant').get_by_text('AI补充：\n这是对思路的追问。我们可以先比较前后状态，再联系电子变化。',exact=True).wait_for()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('.chat-message.user').count()==2。
            assert page.locator('.chat-message.user').count()==2
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('.dialogue-history').count()==0。
            assert page.locator('.dialogue-history').count()==0
            # 【逐行批注】 执行当前 Python 语句：page.reload()。
            page.reload()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('.chat-message.user')).to_have_count(2)。
            expect(page.locator('.chat-message.user')).to_have_count(2)
            # 【逐行批注】 计算右侧结果并赋给 page.locator('.chat-message.assistant').get_…，更新脚本状态。
            page.locator('.chat-message.assistant').get_by_text('AI补充：\n这是对思路的追问。我们可以先比较前后状态，再联系电子变化。',exact=True).wait_for()
            # 【逐行批注】 计算右侧结果并赋给 page.locator('[data-lesson-link，更新脚本状态。
            open_learning_menu(page).locator('[data-lesson-link=practice]').click()
            # 【逐行批注】 执行当前 Python 语句：field.fill('请根据我的回答出一道选择题。')。
            field.fill('请根据我的回答出一道选择题。')
            # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-send').click()。
            page.locator('#lesson-send').click()
            # 【逐行批注】 计算右侧结果并赋给 page.get_by_role('heading',name，更新脚本状态。
            page.locator('.chat-section').get_by_role('heading',name='根据你的回答：比较电子得失',exact=True).wait_for()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            # 正式题卡接管后不再保留同一内容的独立流式草稿。
            expect(page.locator('.agent-stream-sections')).to_be_hidden()
            # 【逐行批注】 计算右侧结果并赋给 followup，更新脚本状态。
            followup=page.locator('.chat-section').filter(has=page.get_by_role('heading',name='根据你的回答：比较电子得失',exact=True))
            # 【逐行批注】 计算右侧结果并赋给 expect(followup.locator('input[type，更新脚本状态。
            expect(followup.locator('input[type=radio]')).to_have_count(2)
            # 【逐行批注】 计算右侧结果并赋给 followup.locator('input[type，更新脚本状态。
            followup.locator('input[type=radio]').last.check()
            # 【逐行批注】 执行当前 Python 语句：field.fill('新题尚未提交的解释。')。
            field.fill('新题尚未提交的解释。')
            # 【逐行批注】 执行当前 Python 语句：page.reload()。
            page.reload()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_have_value('新题尚未提交的解释。')。
            expect(page.locator('#lesson-input')).to_have_value('新题尚未提交的解释。')
            # 【逐行批注】 计算右侧结果并赋给 expect(followup.locator('input[type，更新脚本状态。
            expect(followup.locator('input[type=radio]').last).to_be_checked()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()。
            assert 'PRIVATE-' not in page.locator('#lesson-scroll').inner_text()
            # 已看历史时，新流式回复不强行拖回底部；正文增长也不移动输入栏。
            # 【逐行批注】 执行当前 Python 语句：field.fill('')。
            field.fill('')
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',h…。
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',headers={'Content-Type':'application/json'})): pass
            # 【逐行批注】 执行当前 Python 语句：page.locator('#continue-teaching').click()。
            open_learning_menu(page).locator('#continue-teaching').click()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-stop')).to_be_visible()。
            expect(page.locator('#lesson-stop')).to_be_visible()
            # 【逐行批注】 计算右侧结果并赋给 page.locator('#lesson-scroll').evaluate('(no…，更新脚本状态。
            page.locator('#lesson-scroll').evaluate('(node)=>node.scrollTop=0')
            # 【逐行批注】 计算右侧结果并赋给 top_before，更新脚本状态。
            top_before=page.locator('#lesson-scroll').evaluate('(node)=>node.scrollTop')
            # 【逐行批注】 计算右侧结果并赋给 field_before，更新脚本状态。
            field_before=field.bounding_box()
            # 【逐行批注】 计算右侧结果并赋给 expect(page.locator('#lesson-task-status')).…，更新脚本状态。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问',timeout=30000)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.get_by_role('heading',name='把化合价变化联系到电子',exact=True).count()==0。
            assert page.get_by_role('heading',name='把化合价变化联系到电子',exact=True).count()==0
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert abs(page.locator('#lesson-scroll').evaluate('(node)=>node.scrollTop')-to…。
            assert abs(page.locator('#lesson-scroll').evaluate('(node)=>node.scrollTop')-top_before)<2
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert abs(field.bounding_box()['y']-field_before['y'])<2。
            assert abs(field.bounding_box()['y']-field_before['y'])<2
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-latest')).to_be_visible()。
            expect(page.locator('#lesson-latest')).to_be_visible()
            # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-latest').click()。
            page.locator('#lesson-latest').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('#lesson-scroll').evaluate('(node)=>node.scrollHeight-node.…。
            assert page.locator('#lesson-scroll').evaluate('(node)=>node.scrollHeight-node.scrollTop-node.clientHeight')<2
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',…。
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',headers={'Content-Type':'application/json'})): pass
            # 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
            from pathlib import Path
            # 【逐行批注】 计算右侧结果并赋给 screenshots，更新脚本状态。
            screenshots=Path(__file__).resolve().parents[1]/'verification'
            # 【逐行批注】 计算右侧结果并赋给 page.screenshot(path，更新脚本状态。
            page.screenshot(path=str(screenshots/'chat-classroom-1440.png'))
            # 【逐行批注】 遍历数据并重复执行循环体：for width,height in ((960,600),(390,844)):。
            for width,height in ((960,600),(390,844)):
                # 【逐行批注】 执行当前 Python 语句：page.set_viewport_size({'width':width,'height':height})。
                page.set_viewport_size({'width':width,'height':height})
                # 【逐行批注】 调用 fits 完成当前脚本或测试步骤：fits()。
                fits()
                # 【逐行批注】 判断条件 width==390:，成立时执行对应分支。
                if width==390:
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('.course-directory-mobile').is_visible() and not page.locat…。
                    assert not page.locator('.course-directory').is_visible()
                    assert open_learning_menu(page).locator('.course-directory-mobile').is_visible()
                    page.keyboard.press('Escape')
                # 【逐行批注】 计算右侧结果并赋给 page.screenshot(path，更新脚本状态。
                page.screenshot(path=str(screenshots/('chat-classroom-'+str(width)+'.png')))
            # 【逐行批注】 执行当前 Python 语句：page.set_viewport_size({'width':1440,'height':900})。
            page.set_viewport_size({'width':1440,'height':900})
            # 【逐行批注】 执行当前 Python 语句：page.locator('#composer-chat').click()。
            open_learning_menu(page).locator('#composer-chat').click()
            # 【逐行批注】 执行当前 Python 语句：field.fill('为什么停止后还能继续追问？')。
            field.fill('为什么停止后还能继续追问？')
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',h…。
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',headers={'Content-Type':'application/json'})): pass
            # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-send').click()。
            page.locator('#lesson-send').click()
            # 【逐行批注】 调用 not 完成当前脚本或测试步骤：page.wait_for_function("document.querySelector('#lesson-stop:not([hid…。
            page.wait_for_function("document.querySelector('#lesson-stop:not([hidden])') && [...document.querySelectorAll('.chat-message.assistant')].at(-1).textContent.length>25")
            # 【逐行批注】 计算右侧结果并赋给 interrupted_task，更新脚本状态。
            interrupted_task=api('/api/learning-agent/lesson?lessonId='+lesson)['teachingTaskId']
            # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-stop').click()。
            page.locator('#lesson-stop').click()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_contain_text('已停止本次回答')。
            expect(page.locator('#lesson-task-status')).to_contain_text('已停止本次回答')
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-send')).to_be_enabled()。
            expect(page.locator('#lesson-send')).to_be_enabled()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-retry')).to_be_visible()。
            expect(page.locator('#lesson-retry')).to_be_visible()
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',…。
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',headers={'Content-Type':'application/json'})): pass
            # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-retry').click()。
            page.locator('#lesson-retry').click()
            # 【逐行批注】 计算右侧结果并赋给 expect(page.locator('#lesson-task-status')).…，更新脚本状态。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问',timeout=30000)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learning-agent/lesson?lessonId='+lesson)['teachingTaskId']==in…。
            assert api('/api/learning-agent/lesson?lessonId='+lesson)['teachingTaskId']==interrupted_task
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('.chat-message.user').filter(has_text='为什么停止后还能继续追问？').coun…。
            assert page.locator('.chat-message.user').filter(has_text='为什么停止后还能继续追问？').count()==1
            # 不可自动恢复的服务商错误保留同次输入，修正连接后重试同一个任务。
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"http_error":40…。
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"http_error":402}',headers={'Content-Type':'application/json'})): pass
            # 【逐行批注】 执行当前 Python 语句：field.fill('为什么请求失败时要保留输入？')。
            field.fill('为什么请求失败时要保留输入？')
            # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-send').click()。
            page.locator('#lesson-send').click()
            # 【逐行批注】 计算右侧结果并赋给 expect(page.locator('#lesson-retry')).to_be_…，更新脚本状态。
            expect(page.locator('#lesson-retry')).to_be_visible(timeout=30000)
            # 【逐行批注】 计算右侧结果并赋给 failed_task，更新脚本状态。
            failed_task=api('/api/learning-agent/lesson?lessonId='+lesson)['teachingTaskId']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learning-agent?taskId='+failed_task)['failure']['httpStatus']=…。
            assert api('/api/learning-agent?taskId='+failed_task)['failure']['httpStatus']==402
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control',data=b'{}',headers={'Co…。
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{}',headers={'Content-Type':'application/json'})): pass
            # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-retry').click()。
            page.locator('#lesson-retry').click()
            # 【逐行批注】 计算右侧结果并赋给 expect(page.locator('#lesson-task-status')).…，更新脚本状态。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问',timeout=30000)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learning-agent/lesson?lessonId='+lesson)['teachingTaskId']==fa…。
            assert api('/api/learning-agent/lesson?lessonId='+lesson)['teachingTaskId']==failed_task
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('.chat-message.user').filter(has_text='为什么请求失败时要保留输入？').cou…。
            assert page.locator('.chat-message.user').filter(has_text='为什么请求失败时要保留输入？').count()==1
            # 【逐行批注】 计算右侧结果并赋给 page.locator('[data-lesson-link，更新脚本状态。
            open_learning_menu(page).locator('[data-lesson-link=learn]').click()
            # 【逐行批注】 执行当前 Python 语句：page.locator('#composer-chat').click()。
            open_learning_menu(page).locator('#composer-chat').click()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('.chat-message.user')).to_have_count(0)。
            expect(page.locator('.chat-message.user')).to_have_count(0)
            # 【逐行批注】 执行当前 Python 语句：field.fill('原知识点独立保存的草稿。')。
            field.fill('原知识点独立保存的草稿。')
            # 【逐行批注】 计算右侧结果并赋给 second_task，更新脚本状态。
            second_task = api('/api/learning-agent/events', dict(type='prepare_next', courseId=info['courseId'],
                # 【逐行批注】 计算右侧结果并赋给 requestId，更新脚本状态。
                requestId='browser-second-prepared-lesson', navigate=False))
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = time.monotonic() + 30
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
            while time.monotonic() < deadline:
                # 【逐行批注】 计算右侧结果并赋给 second，更新脚本状态。
                second = api('/api/learning-agent?taskId='+second_task['id'])
                # 【逐行批注】 判断条件 second['status'] == 'ready': break，成立时执行对应分支。
                if second['status'] == 'ready': break
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.05)。
                time.sleep(.05)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert second['status'] == 'ready'。
            assert second['status'] == 'ready'
            # 【逐行批注】 计算右侧结果并赋给 other_lesson，更新脚本状态。
            other_lesson = second['lesson']['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert other_lesson != lesson。
            assert other_lesson != lesson
            # 模拟首次开课的焦点晚到：先写聊天草稿，再读取实际题目焦点。
            # 【逐行批注】 计算右侧结果并赋给 focus_route，更新脚本状态。
            focus_route = '**/api/learning-agent/lesson?lessonId='+other_lesson
            # 【逐行批注】 定义函数 delay_focus，封装可复用的处理步骤。
            def delay_focus(route):
                # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
                response = route.fetch()
                # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
                data = response.json()
                # 【逐行批注】 计算右侧结果并赋给 data['teachingFocus']，更新脚本状态。
                data['teachingFocus'] = {}
                # 【逐行批注】 计算右侧结果并赋给 route.fulfill(response，更新脚本状态。
                route.fulfill(response=response, json=data)
            # 【逐行批注】 执行当前 Python 语句：page.route(focus_route, delay_focus)。
            page.route(focus_route, delay_focus)
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/learn?lessonId，更新脚本状态。
            page.goto(info['base']+'/learn?lessonId='+other_lesson)
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问', timeout=25000)
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#composer-target')).to_have_text('和 AI 老师交流')。
            expect(page.locator('#composer-target')).to_have_text('和 AI 老师交流')
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_have_value('')。
            expect(page.locator('#lesson-input')).to_have_value('')
            # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-input').fill('另一知识点的草稿。')。
            page.locator('#lesson-input').fill('另一知识点的草稿。')
            # 等待仍在处理的拦截请求，避免切页时重复完成同一路由。
            # 【逐行批注】 计算右侧结果并赋给 page.unroute_all(behavior，更新脚本状态。
            page.unroute_all(behavior='wait')
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/learn?lessonId，更新脚本状态。
            page.goto(info['base']+'/learn?lessonId='+lesson)
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_have_value('原知识点独立保存的草稿。')。
            expect(page.locator('#lesson-input')).to_have_value('原知识点独立保存的草稿。')
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/learn?lessonId，更新脚本状态。
            page.goto(info['base']+'/learn?lessonId='+other_lesson)
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#composer-target')).to_have_text('和 AI 老师交流')。
            expect(page.locator('#composer-target')).to_have_text('和 AI 老师交流')
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_have_value('另一知识点的草稿。')。
            expect(page.locator('#lesson-input')).to_have_value('另一知识点的草稿。')
            # 完成按钮使用真实主控判断；刷新后回复保留，不能伪造一条没有保存的学生消息。
            # 【逐行批注】 计算右侧结果并赋给 page.locator('[data-lesson-link，更新脚本状态。
            open_learning_menu(page).locator('[data-lesson-link=summary]').click()
            # 【逐行批注】 计算右侧结果并赋给 users_before，更新脚本状态。
            users_before = page.locator('.chat-message.user').count()
            # 【逐行批注】 执行当前 Python 语句：page.locator('#finish-lesson').click()。
            open_learning_menu(page, '课堂操作').locator('#finish-lesson').click()
            # 【逐行批注】 计算右侧结果并赋给 finish_reply，更新脚本状态。
            finish_reply = page.locator('.chat-message.assistant').get_by_text('AI补充：\n请结合本节检查点回顾，已有记录会保留，缺少证据的部分继续练习。', exact=True)
            # 【逐行批注】 执行当前 Python 语句：finish_reply.wait_for()。
            finish_reply.wait_for()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('.chat-message.user')).to_have_count(users_before)。
            expect(page.locator('.chat-message.user')).to_have_count(users_before)
            # 【逐行批注】 执行当前 Python 语句：page.reload()。
            page.reload()
            # 【逐行批注】 执行当前 Python 语句：finish_reply.wait_for()。
            finish_reply.wait_for()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('.chat-message.user')).to_have_count(users_before)。
            expect(page.locator('.chat-message.user')).to_have_count(users_before)
            # 相同文字的旧交流与新交流都是实际记录，不能按正文去重丢掉旧轮次。
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as database:。
            with closing(sqlite3.connect(info['database'])) as database:
                # 【逐行批注】 计算右侧结果并赋给 saved_lesson，更新脚本状态。
                saved_lesson = json.loads(database.execute('SELECT payload FROM ClassroomActivity WHERE id=?', (lesson,)).fetchone()[0])
                # 【逐行批注】 调用 next、dict 完成当前脚本或测试步骤：next(section for section in saved_lesson['sections'] if section.get('…。
                next(section for section in saved_lesson['sections'] if section.get('questionKind')=='practice')['legacyDialog'] = [dict(role='user', text='为什么停止后还能继续追问？', at=1)]
                # 【逐行批注】 计算右侧结果并赋给 database.execute('UPDATE ClassroomActivity S…，更新脚本状态。
                database.execute('UPDATE ClassroomActivity SET payload=? WHERE id=?', (json.dumps(saved_lesson), lesson))
                # 【逐行批注】 执行当前 Python 语句：database.commit()。
                database.commit()
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/practice?lessonId，更新脚本状态。
            page.goto(info['base']+'/practice?lessonId='+lesson)
            # 【逐行批注】 计算右侧结果并赋给 expect(page.locator('.chat-message.user').fi…，更新脚本状态。
            expect(page.locator('.chat-message.user').filter(has_text='为什么停止后还能继续追问？')).to_have_count(2)
            # 【逐行批注】 计算右侧结果并赋给 user_keys，更新脚本状态。
            user_keys = page.locator('.chat-message.user').filter(has_text='为什么停止后还能继续追问？').evaluate_all('(nodes)=>nodes.map(node=>node.dataset.entryKey)')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert user_keys[0].startswith('legacy:') and not user_keys[1].startswith('lega…。
            assert user_keys[0].startswith('legacy:') and not user_keys[1].startswith('legacy:')
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/learn?lessonId，更新脚本状态。
            page.goto(info['base']+'/learn?lessonId='+other_lesson)
            # 展示夹具直接改写课表前暂停后台，避免人工注入的计划触发独立画像任务。
            # 【逐行批注】 计算右侧结果并赋给 api('/api/learning-agent/control',dict(comma…，更新脚本状态。
            api('/api/learning-agent/control',dict(command='pause',taskId=second_task['id']))
            # 【逐行批注】 计算右侧结果并赋给 initial_plan，更新脚本状态。
            initial_plan = api('/api/study-plan')
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as database:。
            with closing(sqlite3.connect(info['database'])) as database:
                # 【逐行批注】 计算右侧结果并赋给 saved，更新脚本状态。
                saved = database.execute("SELECT value FROM ProfileMeta WHERE key='learning-flow'").fetchone()
                # 【逐行批注】 计算右侧结果并赋给 raw，更新脚本状态。
                raw = json.loads(saved[0]) if saved else dict(plan=initial_plan)
                # 【逐行批注】 计算右侧结果并赋给 scheduled，更新脚本状态。
                scheduled = [dict(taskId='agent-lesson:'+identity, kind='agent-lesson', lessonId=identity,
                    # 【逐行批注】 计算右侧结果并赋给 courseId，更新脚本状态。
                    courseId=info['courseId'], phaseIndex=1, topicIndex=1, title=title, date='2099-01-05', minutes=10, order=index)
                    # 【逐行批注】 遍历数据并重复执行循环体：for index,(identity,title) in enumerate([(lesson,'已保存课堂甲'),(other_lesson,'…。
                    for index,(identity,title) in enumerate([(lesson,'已保存课堂甲'),(other_lesson,'已保存课堂乙')])]
                # 【逐行批注】 计算右侧结果并赋给 raw['plan']['entries']，更新脚本状态。
                raw['plan']['entries'] = scheduled
                # 【逐行批注】 计算右侧结果并赋给 candidate，更新脚本状态。
                candidate = json.loads(json.dumps(raw['plan']))
                # 【逐行批注】 执行当前 Python 语句：del candidate['entries'][0]['title']。
                del candidate['entries'][0]['title']
                # 【逐行批注】 计算右侧结果并赋给 raw.update(agentControlled，更新脚本状态。
                raw.update(agentControlled=True,status='ready',proposal=dict(id='display-only-proposal',plan=candidate,
                    # 【逐行批注】 计算右侧结果并赋给 previousPlan，更新脚本状态。
                    previousPlan=raw['plan'],reason='保留 '+scheduled[0]['taskId']+' 与 '+scheduled[1]['taskId']))
                # 【逐行批注】 计算右侧结果并赋给 database.execute("INSERT INTO ProfileMeta(ke…，更新脚本状态。
                database.execute("INSERT INTO ProfileMeta(key,value) VALUES('learning-flow',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value",(json.dumps(raw),))
                # 【逐行批注】 执行当前 Python 语句：database.commit()。
                database.commit()
            # 【逐行批注】 执行当前 Python 语句：page.set_viewport_size({'width':1440,'height':900})。
            page.set_viewport_size({'width':1440,'height':900})
            # 【逐行批注】 执行当前 Python 语句：page.goto(info['base']+'/my-ai/plan')。
            page.goto(info['base']+'/my-ai/plan')
            # 【逐行批注】 计算右侧结果并赋给 links，更新脚本状态。
            links = page.locator('#weekly-entries .weekly-row a')
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(links).to_have_count(2)。
            expect(links).to_have_count(2)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert links.nth(0).get_attribute('href') == '/learn?lessonId='+lesson。
            assert links.nth(0).get_attribute('href') == '/learn?lessonId='+lesson
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert links.nth(1).get_attribute('href') == '/learn?lessonId='+other_lesson。
            assert links.nth(1).get_attribute('href') == '/learn?lessonId='+other_lesson
            # 【逐行批注】 计算右侧结果并赋给 preview，更新脚本状态。
            preview = page.locator('#weekly-confirm')
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(preview).to_be_visible()。
            expect(preview).to_be_visible()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'undefined' not in preview.inner_text()。
            assert 'undefined' not in preview.inner_text()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(item['taskId'] not in preview.inner_text() for item in scheduled)。
            assert all(item['taskId'] not in preview.inner_text() for item in scheduled)
            # 过期但完整保存的课件仍可回看，入课保护继续限制新的教学操作。
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as database:。
            with closing(sqlite3.connect(info['database'])) as database:
                # 【逐行批注】 计算右侧结果并赋给 raw，更新脚本状态。
                raw = json.loads(database.execute('SELECT payload FROM ClassroomActivity WHERE id=?',(other_lesson,)).fetchone()[0])
                # 【逐行批注】 计算右侧结果并赋给 raw['entered']，更新脚本状态。
                raw['entered'] = False
                # 【逐行批注】 计算右侧结果并赋给 raw['sourceLearningVersion']，更新脚本状态。
                raw['sourceLearningVersion'] = -1
                # 【逐行批注】 计算右侧结果并赋给 database.execute('UPDATE ClassroomActivity S…，更新脚本状态。
                database.execute('UPDATE ClassroomActivity SET payload=? WHERE id=?',(json.dumps(raw),other_lesson))
                # 【逐行批注】 执行当前 Python 语句：database.commit()。
                database.commit()
            # 【逐行批注】 计算右侧结果并赋给 calls_before，更新脚本状态。
            calls_before = json.load(urlopen(info['fixture']))['calls']
            # 【逐行批注】 遍历数据并重复执行循环体：for view in ('learn','practice','summary'):。
            for view in ('learn','practice','summary'):
                # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/'+view+'?lessonId，更新脚本状态。
                page.goto(info['base']+'/'+view+'?lessonId='+other_lesson)
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-title')).to_have_text(raw['title'])。
                expect(page.locator('#lesson-title')).to_have_text(raw['title'])
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#page-status')).to_contain_text('你可以回看已保存内容')。
                expect(page.locator('#page-status')).to_contain_text('你可以回看已保存内容')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('#lesson-sections').inner_text()。
                assert page.locator('#lesson-sections').inner_text()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert [page.locator('[data-lesson-link]').nth(index).get_attribute('data-lesso…。
                assert [page.locator('[data-lesson-link]').nth(index).get_attribute('data-lesson-link') for index in range(3)] == ['learn','practice','summary']
                # 【逐行批注】 遍历数据并重复执行循环体：for button in page.locator('#lesson-sections form button').all(): expect(b…。
                for button in page.locator('#lesson-sections form button').all(): expect(button).to_be_disabled()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert json.load(urlopen(info['fixture']))['calls'] == calls_before。
            assert json.load(urlopen(info['fixture']))['calls'] == calls_before
            # 开启动画且正文增长时，页头操作仍固定在可见窗口内。
            # 【逐行批注】 计算右侧结果并赋给 page.emulate_media(reduced_motion，更新脚本状态。
            page.emulate_media(reduced_motion='no-preference')
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/practice?lessonId，更新脚本状态。
            page.goto(info['base']+'/practice?lessonId='+lesson)
            # 【逐行批注】 计算右侧结果并赋给 page.locator('[data-lesson-link，更新脚本状态。
            open_learning_menu(page).locator('[data-lesson-link=practice]').click()
            # 【逐行批注】 计算右侧结果并赋给 page.locator('.chat-section[data-area，更新脚本状态。
            page.locator('.chat-section[data-area=practice]:visible').first.wait_for()
            # 【逐行批注】 调用 getComputedStyle 完成当前脚本或测试步骤：page.wait_for_function("getComputedStyle(document.body).opacity==='1'…。
            page.wait_for_function("getComputedStyle(document.body).opacity==='1'")
            # 【逐行批注】 计算右侧结果并赋给 toggle，更新脚本状态。
            toggle = page.get_by_role('button', name='操作菜单', exact=True)
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = toggle.bounding_box()
            # 【逐行批注】 计算右侧结果并赋给 page.evaluate("""()，更新脚本状态。
            page.evaluate("""() => {
                const paragraph=document.createElement('p');
                paragraph.textContent='这是一段持续增长的课堂讲解，正文完整保留。'.repeat(600);
                document.querySelector('#lesson-sections').append(paragraph);
            }""")
            # 【逐行批注】 计算右侧结果并赋给 after，更新脚本状态。
            after = toggle.bounding_box()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert before and after and abs(before['y']-after['y']) < 1。
            assert before and after and abs(before['y']-after['y']) < 1
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert after['y'] >= 0 and after['y']+after['height'] <= 900。
            assert after['y'] >= 0 and after['y']+after['height'] <= 900
            # 【逐行批注】 执行当前 Python 语句：toggle.click()。
            activity = open_learning_menu(page, '全站导航与设置').locator('#learning-navigation')
            if activity.get_attribute('open') is None:
                activity.locator(':scope > summary').click()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#agent-control-panel')).to_be_visible()。
            expect(page.locator('#agent-control-panel')).to_be_visible()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors,errors。
            assert not errors,errors
            # 【逐行批注】 执行当前 Python 语句：browser.close()。
            browser.close()
        # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as database:。
        with closing(sqlite3.connect(info['database'])) as database:
            # 【逐行批注】 计算右侧结果并赋给 values，更新脚本状态。
            values=[json.loads(row[0]) for row in database.execute("SELECT payload FROM LearningInteraction WHERE kind='practice'")]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(values)==1 and values[0]['response'].startswith('我选1') and values[0]…。
            assert len(values)==1 and values[0]['response'].startswith('我选1') and values[0]['response'].endswith('我认为从0升高到+2。'), values
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not values[0]['assisted'] and values[0]['model']=='fixture-v560'。
            assert not values[0]['assisted'] and values[0]['model']=='fixture-v560'
        # 【逐行批注】 输出当前运行结果：print('聊天课堂、固定输入、内容定位、卡片作答、分题草稿、全局暂停恢复、评价顺序、历史回看与窄屏回归通过（隔离协议夹具）')。
        print('聊天课堂、固定输入、内容定位、卡片作答、分题草稿、全局暂停恢复、评价顺序、历史回看与窄屏回归通过（隔离协议夹具）')


# 【逐行批注】 判断条件 __name__=='__main__':，成立时执行对应分支。
if __name__=='__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encodin…，成立时执行对应分支。
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(sys.argv[1])。
    main(sys.argv[1])

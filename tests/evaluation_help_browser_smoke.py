# 【逐行批注】 执行当前 Python 语句："""验证课堂和报告实际显示的反馈关联后续作答；仅连接本地确定性模型夹具。"""。
"""验证课堂和报告实际显示的反馈关联后续作答；仅连接本地确定性模型夹具。"""
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
        # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control', data=b'{"feedback_help…。
        with urlopen(Request(info['fixture']+'/fixture-control', data=b'{"feedback_help_type":"worked_example"}',
                             # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                             headers={'Content-Type': 'application/json'})):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass
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
        # 【逐行批注】 定义函数 answer，封装可复用的处理步骤。
        def answer(section):
            # 【逐行批注】 执行当前 Python 语句：nonlocal serial。
            nonlocal serial
            # 【逐行批注】 计算右侧结果并赋给 serial +，更新脚本状态。
            serial += 1
            # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
            task = api('/api/learning-agent/events', dict(type='question_answer', text='我认为化合价发生变化',
                       # 【逐行批注】 计算右侧结果并赋给 courseId，更新脚本状态。
                       courseId=info['courseId'], lessonId=lesson_id, sectionId=section['id'], sectionVersion=section['version'],
                       # 【逐行批注】 计算右侧结果并赋给 view，更新脚本状态。
                       view='practice', requestId='feedback-browser-'+str(serial)))
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = time.monotonic()+30
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
            while time.monotonic() < deadline:
                # 【逐行批注】 计算右侧结果并赋给 shown，更新脚本状态。
                shown = api('/api/learning-agent?taskId='+task['id'])
                # 【逐行批注】 判断条件 shown['status'] not in ('pending', 'running'):，成立时执行对应分支。
                if shown['status'] not in ('pending', 'running'):
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert shown['status'] == 'waiting_student', shown。
                    assert shown['status'] == 'waiting_student', shown
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                time.sleep(.1)
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 在错误情形抛出异常：raise AssertionError('评价夹具超时')。
                raise AssertionError('评价夹具超时')
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as db:。
            with closing(sqlite3.connect(info['database'])) as db:
                # 【逐行批注】 计算右侧结果并赋给 records，更新脚本状态。
                records = [json.loads(r[0]) for r in db.execute("SELECT payload FROM LearningInteraction WHERE kind='chat-user'")]
            # 【逐行批注】 结束当前函数并返回结果：return next(r for r in records if r.get('requestId') == 'feedback-bro…。
            return next(r for r in records if r.get('requestId') == 'feedback-browser-'+str(serial))
        # 【逐行批注】 定义函数 exposure，封装可复用的处理步骤。
        def exposure(evaluation_id):
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as db:。
            with closing(sqlite3.connect(info['database'])) as db:
                # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
                row = db.execute('SELECT value FROM ProfileMeta WHERE key=?', ('agent-evaluation-exposure:'+lesson_id,)).fetchone()
            # 【逐行批注】 结束当前函数并返回结果：return json.loads(row[0]).get(evaluation_id) if row else None。
            return json.loads(row[0]).get(evaluation_id) if row else None
        # 【逐行批注】 定义函数 wait_exposure，封装可复用的处理步骤。
        def wait_exposure(evaluation_id, partial):
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = time.monotonic()+10
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
            while time.monotonic() < deadline:
                # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
                result = exposure(evaluation_id)
                # 【逐行批注】 判断条件 result and result['partial'] == partial:，成立时执行对应分支。
                if result and result['partial'] == partial:
                    # 【逐行批注】 结束当前函数并返回结果：return result。
                    return result
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.05)。
                time.sleep(.05)
            # 【逐行批注】 在错误情形抛出异常：raise AssertionError('评价展示未保存：'+evaluation_id+'，现有记录：'+str(exposure(e…。
            raise AssertionError('评价展示未保存：'+evaluation_id+'，现有记录：'+str(exposure(evaluation_id)))
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer(practices[0])['helpMode'] == 'independent'。
        assert answer(practices[0])['helpMode'] == 'independent'
        # 【逐行批注】 计算右侧结果并赋给 first，更新脚本状态。
        first = lesson()['evaluations'][-1]
        # 模型输出已保存，但尚未打开页面，不能当作学生已经获得帮助。
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert exposure(first['id']) is None。
        assert exposure(first['id']) is None
        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as playwright:。
        with sync_playwright() as playwright:
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser = playwright.chromium.launch(headless=True)
            # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
            page = browser.new_page(viewport={'width':1440, 'height':900}, reduced_motion='reduce')
            # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
            errors = []
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：page.on('pageerror', lambda error: errors.append(str(error)))。
            page.on('pageerror', lambda error: errors.append(str(error)))
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/practice?lessonId，更新脚本状态。
            page.goto(info['base']+'/practice?lessonId='+lesson_id+'&sectionId='+practices[0]['id'])
            # 【逐行批注】 计算右侧结果并赋给 node，更新脚本状态。
            node = page.locator('[data-evaluation-feedback="'+first['id']+'"]')
            # 【逐行批注】 执行当前 Python 语句：node.scroll_into_view_if_needed()。
            node.scroll_into_view_if_needed()
            # 【逐行批注】 执行当前 Python 语句：page.evaluate('GangyiAgent.flushExposure()')。
            page.evaluate('GangyiAgent.flushExposure()')
            # 【逐行批注】 调用 wait_exposure 完成当前脚本或测试步骤：wait_exposure(first['id'], False)。
            wait_exposure(first['id'], False)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer(practices[0])['helpMode'] == 'after_example'。
            assert answer(practices[0])['helpMode'] == 'after_example'
            # 另一题的反馈尚未展示，不能沿用第一题的帮助。
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer(practices[1])['helpMode'] == 'independent'。
            assert answer(practices[1])['helpMode'] == 'independent'
            # 【逐行批注】 计算右侧结果并赋给 second，更新脚本状态。
            second = next(e for e in lesson()['evaluations'] if e['sectionId'] == practices[1]['id'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert exposure(second['id']) is None。
            assert exposure(second['id']) is None
            # 【逐行批注】 计算右侧结果并赋给 calls_before，更新脚本状态。
            calls_before = json.load(urlopen(info['fixture']))['calls']
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/my-ai/reports?cours…，更新脚本状态。
            page.goto(info['base']+'/my-ai/reports?courseId='+info['courseId']+'&evidenceId='+second['id'])
            # 【逐行批注】 计算右侧结果并赋给 report_node，更新脚本状态。
            report_node = page.locator('[data-report-feedback="'+second['id']+'"]')
            # 【逐行批注】 执行当前 Python 语句：report_node.scroll_into_view_if_needed()。
            report_node.scroll_into_view_if_needed()
            # 【逐行批注】 执行当前 Python 语句：page.evaluate('GangyiAgent.flushExposure()')。
            page.evaluate('GangyiAgent.flushExposure()')
            # 【逐行批注】 调用 wait_exposure 完成当前脚本或测试步骤：wait_exposure(second['id'], False)。
            wait_exposure(second['id'], False)
            # 继续巩固入口须把反馈展示保存完毕再进入原题；浏览报告自身不调用模型。
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert json.load(urlopen(info['fixture']))['calls'] == calls_before。
            assert json.load(urlopen(info['fixture']))['calls'] == calls_before
            # 【逐行批注】 计算右侧结果并赋给 page.get_by_role('link', name，更新脚本状态。
            page.get_by_role('link', name='继续巩固 →', exact=True).click()
            # 【逐行批注】 计算右侧结果并赋给 expect(page).to_have_url(info['base']+'/prac…，更新脚本状态。
            expect(page).to_have_url(info['base']+'/practice?lessonId='+lesson_id+'&sectionId='+practices[1]['id'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer(practices[1])['helpMode'] == 'after_example'。
            assert answer(practices[1])['helpMode'] == 'after_example'
            # 新评价长于视区时，部分展示标 unknown；滚动覆盖全文后升级为 worked_example。
            # 【逐行批注】 执行当前 Python 语句：page.goto('about:blank')。
            page.goto('about:blank')
            # 【逐行批注】 计算右侧结果并赋给 answer(practices[1]); long_record，更新脚本状态。
            answer(practices[1]); long_record = lesson()['evaluations'][-1]
            # 【逐行批注】 执行当前 Python 语句：page.set_viewport_size({'width':390, 'height':320})。
            page.set_viewport_size({'width':390, 'height':320})
            # 【逐行批注】 计算右侧结果并赋给 page.add_init_script("document.addEventListe…，更新脚本状态。
            page.add_init_script("document.addEventListener('DOMContentLoaded',()=>{const style=document.createElement('style');style.textContent='[data-report-feedback]{min-height:1800px}';document.head.append(style)});")
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/my-ai/reports?cours…，更新脚本状态。
            page.goto(info['base']+'/my-ai/reports?courseId='+info['courseId']+'&evidenceId='+long_record['id'])
            # 【逐行批注】 计算右侧结果并赋给 report_node，更新脚本状态。
            report_node = page.locator('[data-report-feedback="'+long_record['id']+'"]')
            # 【逐行批注】 计算右侧结果并赋给 report_node.evaluate('(node)，更新脚本状态。
            report_node.evaluate('(node)=>node.scrollIntoView({block:"start"})')
            # 【逐行批注】 调用 wait_exposure 完成当前脚本或测试步骤：wait_exposure(long_record['id'], True)。
            wait_exposure(long_record['id'], True)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert exposure(long_record['id'])['helpType'] == 'unknown'。
            assert exposure(long_record['id'])['helpType'] == 'unknown'
            # 【逐行批注】 遍历数据并重复执行循环体：for offset in range(0, 1801, 180):。
            for offset in range(0, 1801, 180):
                # 【逐行批注】 计算右侧结果并赋给 report_node.evaluate('(node,offset)，更新脚本状态。
                report_node.evaluate('(node,offset)=>window.scrollTo(0,node.getBoundingClientRect().top+scrollY+offset)', offset)
                # 【逐行批注】 调用 Promise 完成当前脚本或测试步骤：page.evaluate('new Promise(requestAnimationFrame)')。
                page.evaluate('new Promise(requestAnimationFrame)')
            # 【逐行批注】 执行当前 Python 语句：page.evaluate('GangyiAgent.flushExposure()')。
            page.evaluate('GangyiAgent.flushExposure()')
            # 【逐行批注】 调用 wait_exposure 完成当前脚本或测试步骤：wait_exposure(long_record['id'], False)。
            wait_exposure(long_record['id'], False)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert exposure(long_record['id'])['helpType'] == 'worked_example'。
            assert exposure(long_record['id'])['helpType'] == 'worked_example'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, errors。
            assert not errors, errors
            # 【逐行批注】 执行当前 Python 语句：browser.close()。
            browser.close()
        # 【逐行批注】 输出当前运行结果：print('课堂/报告反馈实际展示、同题关联、别题隔离、继续巩固与长反馈部分→完整展示回归通过；外部付费调用0次。')。
        print('课堂/报告反馈实际展示、同题关联、别题隔离、继续巩固与长反馈部分→完整展示回归通过；外部付费调用0次。')


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'):，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'):
        # 【逐行批注】 计算右侧结果并赋给 sys.stdout.reconfigure(encoding，更新脚本状态。
        sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(sys.argv[1])。
    main(sys.argv[1])

"""验证课堂和报告实际显示的反馈关联后续作答；仅连接本地确定性模型夹具。"""
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
        with urlopen(Request(info['fixture']+'/fixture-control', data=b'{"feedback_help_type":"worked_example"}',
                             headers={'Content-Type': 'application/json'})):
            pass
        lesson_id = info['lessonId']
        def lesson():
            return api('/api/learning-agent/lesson?lessonId='+lesson_id)
        practices = [s for s in lesson()['sections'] if s.get('questionKind') == 'practice']
        serial = 0
        def answer(section):
            nonlocal serial
            serial += 1
            task = api('/api/learning-agent/events', dict(type='question_answer', text='我认为化合价发生变化',
                       courseId=info['courseId'], lessonId=lesson_id, sectionId=section['id'], sectionVersion=section['version'],
                       view='practice', requestId='feedback-browser-'+str(serial)))
            deadline = time.monotonic()+30
            while time.monotonic() < deadline:
                shown = api('/api/learning-agent?taskId='+task['id'])
                if shown['status'] not in ('pending', 'running'):
                    assert shown['status'] == 'waiting_student', shown
                    break
                time.sleep(.1)
            else:
                raise AssertionError('评价夹具超时')
            with closing(sqlite3.connect(info['database'])) as db:
                records = [json.loads(r[0]) for r in db.execute("SELECT payload FROM LearningInteraction WHERE kind='chat-user'")]
            return next(r for r in records if r.get('requestId') == 'feedback-browser-'+str(serial))
        def exposure(evaluation_id):
            with closing(sqlite3.connect(info['database'])) as db:
                row = db.execute('SELECT value FROM ProfileMeta WHERE key=?', ('agent-evaluation-exposure:'+lesson_id,)).fetchone()
            return json.loads(row[0]).get(evaluation_id) if row else None
        def wait_exposure(evaluation_id, partial):
            deadline = time.monotonic()+10
            while time.monotonic() < deadline:
                result = exposure(evaluation_id)
                if result and result['partial'] == partial:
                    return result
                time.sleep(.05)
            raise AssertionError('评价展示未保存：'+evaluation_id+'，现有记录：'+str(exposure(evaluation_id)))
        assert answer(practices[0])['helpMode'] == 'independent'
        first = lesson()['evaluations'][-1]
        # 模型输出已保存，但尚未打开页面，不能当作学生已经获得帮助。
        assert exposure(first['id']) is None
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True)
            page = browser.new_page(viewport={'width':1440, 'height':900}, reduced_motion='reduce')
            errors = []
            page.on('pageerror', lambda error: errors.append(str(error)))
            page.goto(info['base']+'/practice?lessonId='+lesson_id+'&sectionId='+practices[0]['id'])
            node = page.locator('[data-evaluation-feedback="'+first['id']+'"]')
            node.scroll_into_view_if_needed()
            page.evaluate('GangyiAgent.flushExposure()')
            wait_exposure(first['id'], False)
            assert answer(practices[0])['helpMode'] == 'after_example'
            # 另一题的反馈尚未展示，不能沿用第一题的帮助。
            assert answer(practices[1])['helpMode'] == 'independent'
            second = next(e for e in lesson()['evaluations'] if e['sectionId'] == practices[1]['id'])
            assert exposure(second['id']) is None
            calls_before = json.load(urlopen(info['fixture']))['calls']
            page.goto(info['base']+'/my-ai/reports?courseId='+info['courseId']+'&evidenceId='+second['id'])
            report_node = page.locator('[data-report-feedback="'+second['id']+'"]')
            report_node.scroll_into_view_if_needed()
            page.evaluate('GangyiAgent.flushExposure()')
            wait_exposure(second['id'], False)
            # 继续巩固入口须把反馈展示保存完毕再进入原题；浏览报告自身不调用模型。
            assert json.load(urlopen(info['fixture']))['calls'] == calls_before
            page.get_by_role('link', name='继续巩固 →', exact=True).click()
            expect(page).to_have_url(info['base']+'/practice?lessonId='+lesson_id+'&sectionId='+practices[1]['id'])
            assert answer(practices[1])['helpMode'] == 'after_example'
            # 新评价长于视区时，部分展示标 unknown；滚动覆盖全文后升级为 worked_example。
            page.goto('about:blank')
            answer(practices[1]); long_record = lesson()['evaluations'][-1]
            page.set_viewport_size({'width':390, 'height':320})
            page.add_init_script("document.addEventListener('DOMContentLoaded',()=>{const style=document.createElement('style');style.textContent='[data-report-feedback]{min-height:1800px}';document.head.append(style)});")
            page.goto(info['base']+'/my-ai/reports?courseId='+info['courseId']+'&evidenceId='+long_record['id'])
            report_node = page.locator('[data-report-feedback="'+long_record['id']+'"]')
            report_node.evaluate('(node)=>node.scrollIntoView({block:"start"})')
            wait_exposure(long_record['id'], True)
            assert exposure(long_record['id'])['helpType'] == 'unknown'
            for offset in range(0, 1801, 180):
                report_node.evaluate('(node,offset)=>window.scrollTo(0,node.getBoundingClientRect().top+scrollY+offset)', offset)
                page.evaluate('new Promise(requestAnimationFrame)')
            page.evaluate('GangyiAgent.flushExposure()')
            wait_exposure(long_record['id'], False)
            assert exposure(long_record['id'])['helpType'] == 'worked_example'
            assert not errors, errors
            browser.close()
        print('课堂/报告反馈实际展示、同题关联、别题隔离、继续巩固与长反馈部分→完整展示回归通过；外部付费调用0次。')


if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])

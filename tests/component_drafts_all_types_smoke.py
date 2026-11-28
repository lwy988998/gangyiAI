"""验证所有已注册互动组件的未提交探索可从服务端恢复。"""

import json
from pathlib import Path
import sys
import time
from urllib.parse import urlencode

from playwright.sync_api import sync_playwright

import classroom_v560_fixture as fixture
from deep_course_integration_smoke import DeepFixture


def wait_for(check, message, timeout=60, page=None):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        result = check()
        if result:
            return result
        if page:
            page.wait_for_timeout(80)
        else:
            time.sleep(.08)
    raise AssertionError(message)


def main():
    executable, output = sys.argv[1:3]
    output = Path(output).resolve()
    output.mkdir(parents=True, exist_ok=False)
    for name in ('include_sets', 'include_labs', 'motion_lab', 'number_line', 'vector_lab',
                 'statistics_lab', 'equation_lab', 'triangle_lab', 'wave_lab', 'periodic_lab'):
        setattr(DeepFixture, name, True)
    fixture.FixtureAI = DeepFixture
    with fixture.start_session(executable) as (info, api, _), sync_playwright() as runner:
        def wait_task(task_id):
            def completed():
                task = api('/api/learning-agent?taskId=' + task_id)
                if task['status'] in ('pending', 'running'):
                    return None
                assert task['status'] in ('ready', 'waiting_student'), task.get('error')
                return task
            return wait_for(completed, '模拟教学任务未完成')

        course = wait_task(api('/api/generate-plan', {'goal': '互动组件草稿恢复检查', 'mode': 'deep',
                                                     'requestId': 'all-drafts-plan'})['id'])['course']['id']
        task = api('/api/deep-course/start', {'courseId': course, 'phaseIndex': 1, 'topicIndex': 1,
                                               'requestId': 'all-drafts-lesson'})['task']['id']
        lesson_id = wait_task(task)['lesson']['id']
        query = urlencode({'lessonId': lesson_id})
        path = '/api/deep-course?' + query
        before = api(path)
        errors = []
        browser = fixture.launch_test_browser(runner)
        context = browser.new_context(viewport={'width': 1280, 'height': 900}, reduced_motion='reduce')
        page = context.new_page()
        page.on('pageerror', lambda error: errors.append(str(error)))
        ready = "document.getElementById('deep-course-app').dataset.ready === 'true'"
        page.goto(info['base'] + '/deep-course?' + query)
        page.wait_for_function(ready)
        calls_before = len(DeepFixture.calls)

        def lab(name):
            return page.locator(f'[data-media-id={name}]').first

        lab('set-lab').get_by_role('checkbox', name='1 属于集合 B', exact=True).set_checked(True)
        lab('function-lab').get_by_role('slider', name='a', exact=True).press('ArrowRight')
        lab('circuit-lab').get_by_role('combobox', name='连接方式', exact=True).select_option('parallel')
        lab('chemical-lab').get_by_role('spinbutton', name='H₂ 的系数', exact=True).fill('2')
        lab('cell-lab').get_by_role('button', name='叶绿体', exact=True).click()
        lab('history-lab').get_by_role('button', name='阿波罗11号发射提前', exact=True).click()
        lab('climate-lab').get_by_role('combobox', name='观察月份', exact=True).select_option('7')
        lab('annotation-lab').get_by_role('combobox', name='片段1：雨停了。', exact=True).select_option('3')
        lab('sentence-lab').get_by_role('combobox', name='片段1：The students', exact=True).select_option('1')
        lab('algorithm-lab').get_by_role('button', name='下一步', exact=True).click()
        lab('motion-lab').get_by_role('slider', name='时刻 / s', exact=True).press('ArrowRight')
        lab('number-line').get_by_role('combobox', name='左端点类型', exact=True).select_option('closed')
        lab('vector-lab').get_by_role('combobox', name='向量运算', exact=True).select_option('subtract')
        lab('statistics-lab').get_by_role('combobox', name='方差口径', exact=True).select_option('sample')
        lab('triangle-lab').get_by_role('slider', name='C 的 y 坐标', exact=True).press('ArrowRight')
        lab('wave-lab').get_by_role('slider', name='周期 T（s）', exact=True).press('ArrowRight')
        lab('periodic-lab').get_by_role('combobox', name='当前元素 A', exact=True).select_option('19')
        equation = lab('equation-lab')
        equation.get_by_role('combobox', name='等式两边的操作', exact=True).select_option('add_constant')
        equation.get_by_role('spinbutton', name='操作数', exact=True).fill('-1')
        equation.get_by_role('button', name='应用到两边', exact=True).click()

        names = ['set-lab', 'function-lab', 'circuit-lab', 'chemical-lab', 'cell-lab', 'history-lab',
                 'climate-lab', 'annotation-lab', 'sentence-lab', 'algorithm-lab', 'motion-lab',
                 'number-line', 'vector-lab', 'statistics-lab', 'equation-lab', 'triangle-lab', 'wave-lab', 'periodic-lab']
        def saved_all():
            drafts = api(path).get('componentDrafts', {})
            return drafts if all(name in drafts for name in names) else None
        drafts = wait_for(saved_all, '未保存全部组件的探索草稿', timeout=30, page=page)
        for name in names:
            assert drafts[name]['trace']['events'][-1]['state'] == drafts[name]['state'], name
            assert drafts[name]['trace']['totalChanges'] >= 1, name

        def result_text(name):
            selector = '.il-sets-result' if name == 'set-lab' else '.il-equation-current' if name == 'equation-lab' else '.il-lab-result'
            return lab(name).locator(selector).inner_text()

        displayed = {name: result_text(name) for name in names}
        context.close()
        context = browser.new_context(viewport={'width': 1280, 'height': 900}, reduced_motion='reduce')
        page = context.new_page()
        page.on('pageerror', lambda error: errors.append(str(error)))
        page.add_init_script('window.initialStorageSize = localStorage.length')
        page.goto(info['base'] + '/deep-course?' + query)
        page.wait_for_function(ready)
        assert page.evaluate('window.initialStorageSize') == 0
        for name in names:
            assert result_text(name) == displayed[name], name
        assert api(path)['lesson']['evaluations'] == before['lesson']['evaluations']
        assert len(DeepFixture.calls) == calls_before and not errors, errors
        report = {'componentCount': len(names), 'components': names, 'newBrowserStorage': True,
                  'unsubmittedDraftsRestored': True, 'formalEvaluationsUnchanged': True,
                  'autosaveDidNotCallModel': True, 'paidModelRequests': 0, 'pageErrors': errors}
        context.close()
        browser.close()
    (output / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(report, ensure_ascii=False))


if __name__ == '__main__':
    main()

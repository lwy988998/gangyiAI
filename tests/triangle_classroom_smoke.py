"""用隔离模型验证三角形的实际课堂、轨迹读取和草稿恢复。"""

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
    DeepFixture.include_labs = True
    DeepFixture.triangle_lab = True
    fixture.FixtureAI = DeepFixture
    errors = []
    with fixture.start_session(executable) as (info, api, _), sync_playwright() as runner:
        def wait_task(task_id):
            def completed():
                task = api('/api/learning-agent?taskId=' + task_id)
                if task['status'] in ('pending', 'running'):
                    return None
                assert task['status'] in ('ready', 'waiting_student'), task.get('error')
                return task
            return wait_for(completed, '隔离教学任务未完成')

        course = wait_task(api('/api/generate-plan', {'goal': '观察三角形顶点与面积', 'mode': 'deep',
                                                     'requestId': 'triangle-plan'})['id'])['course']['id']
        task = api('/api/deep-course/start', {'courseId': course, 'phaseIndex': 1, 'topicIndex': 1,
                                               'requestId': 'triangle-lesson'})['task']['id']
        lesson_id = wait_task(task)['lesson']['id']
        query = urlencode({'lessonId': lesson_id})
        path = '/api/deep-course?' + query
        original = api(path)
        browser = fixture.launch_test_browser(runner)
        context = browser.new_context(viewport={'width': 1280, 'height': 900}, reduced_motion='reduce')
        page = context.new_page()
        page.on('pageerror', lambda error: errors.append(str(error)))
        ready = "document.getElementById('deep-course-app').dataset.ready === 'true'"
        page.goto(info['base'] + '/deep-course?' + query)
        page.wait_for_function(ready)
        lab = page.locator('[data-media-id=triangle-lab]').first
        result = lab.locator('.il-lab-result')
        assert '直角三角形' in result.inner_text() and '面积 6' in result.inner_text()

        plot = lab.locator('svg')
        point = plot.locator('[data-point="2"]')
        point.scroll_into_view_if_needed()
        box = point.bounding_box()
        target = plot.evaluate('''svg=>{
          const view=svg.viewBox.baseVal,unit=Math.min((view.width-80)/20,13);
          const p=svg.createSVGPoint();p.x=view.width/2+2*unit;p.y=170-2*unit;
          const screen=p.matrixTransform(svg.getScreenCTM());return {x:screen.x,y:screen.y};
        }''')
        page.mouse.move(box['x'] + box['width']/2, box['y'] + box['height']/2)
        page.mouse.down()
        page.mouse.move(target['x'], target['y'], steps=4)
        page.mouse.up()
        assert lab.get_by_role('slider', name='C 的 x 坐标', exact=True).input_value() == '2'
        assert lab.get_by_role('slider', name='C 的 y 坐标', exact=True).input_value() == '2'
        assert '等腰、直角三角形' in result.inner_text() and '面积 4' in result.inner_text()

        x_control = lab.get_by_role('slider', name='C 的 x 坐标', exact=True)
        x_control.focus(); x_control.press('ArrowRight')
        assert x_control.input_value() == '2.5'
        y_control = lab.get_by_role('slider', name='C 的 y 坐标', exact=True)
        y_control.evaluate("node=>{node.value='0';node.dispatchEvent(new Event('input',{bubbles:true}));}")
        assert '共线或重合' in result.inner_text()
        for width, height in [(1280, 900), (390, 844)]:
            page.set_viewport_size({'width': width, 'height': height})
            lab.scroll_into_view_if_needed()
            assert page.evaluate('document.documentElement.scrollWidth<=innerWidth+1')
            assert lab.evaluate('node=>node.scrollWidth<=node.clientWidth+1')
            lab.screenshot(path=str(output / f'triangle-{width}.png'))

        with page.expect_response(lambda response: response.url.endswith('/api/learning-agent/events') and
                                  response.request.method == 'POST') as submitted:
            lab.get_by_role('button', name='请老师解释当前结果', exact=True).click()
        assert submitted.value.status == 202, submitted.value.text()
        wait_task(submitted.value.json()['id'])
        call = next(item for item in reversed(DeepFixture.calls) if isinstance(item, dict) and
                    item.get('event', {}).get('mediaState', {}).get('triangle-lab'))
        observed = call['event']['mediaState']['triangle-lab']
        assert observed['points'] == [[0, 0], [4, 0], [2.5, 0]]
        assert observed['validTriangle'] is False and observed['angleDegrees'] is None and observed['perimeter'] is None
        assert observed['resultSource'] == 'program_calculation' and observed['scoringEligible'] is False
        trace = next(item['traceRead'] for item in reversed(DeepFixture.calls)
                     if isinstance(item, dict) and item.get('traceRead', {}).get('componentId') == 'triangle-lab')
        assert trace['events'] and trace['events'][0]['observation']['scoringEligible'] is False
        assert api(path)['lesson']['evaluations'] == original['lesson']['evaluations']
        wait_for(lambda: api(path).get('componentDrafts', {}).get('triangle-lab'), '未保存三角形探索', page=page)
        context.close()
        context = browser.new_context(viewport={'width': 1280, 'height': 900})
        page = context.new_page()
        page.on('pageerror', lambda error: errors.append(str(error)))
        page.add_init_script('window.initialStorageSize=localStorage.length')
        page.goto(info['base'] + '/deep-course?' + query)
        page.wait_for_function(ready)
        assert page.evaluate('window.initialStorageSize') == 0
        lab = page.locator('[data-media-id=triangle-lab]').first
        assert lab.get_by_role('slider', name='C 的 x 坐标', exact=True).input_value() == '2.5'
        assert lab.get_by_role('slider', name='C 的 y 坐标', exact=True).input_value() == '0'
        lab.get_by_role('button', name='重置探索', exact=True).click()
        assert '面积 6' in lab.locator('.il-lab-result').inner_text()
        assert api(path)['lesson']['evaluations'] == original['lesson']['evaluations']
        assert not errors, errors
        report = {'passed': True, 'mockAIOnly': True, 'paidModelRequests': 0, 'drag': True,
                  'keyboard': True, 'degenerateTriangle': True, 'serverRecalculated': True,
                  'traceToolRead': True, 'newBrowserRestored': True, 'reset': True,
                  'formalEvaluationsUnchanged': True, 'viewports': [1280, 390], 'pageErrors': errors}
        context.close(); browser.close()
    (output / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(report, ensure_ascii=False))


if __name__ == '__main__':
    main()

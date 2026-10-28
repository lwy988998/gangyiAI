"""真实 C++ 服务的深度课堂闭环验收；只使用临时档案与模拟模型。"""
import io
import json
from pathlib import Path
import sys
import time
from urllib.parse import urlencode

from playwright.sync_api import sync_playwright
import classroom_v560_fixture as fixture


class DeepFixture(fixture.FixtureAI):
    calls = []
    deep_courses = set()

    def do_POST(self):
        raw = self.rfile.read(int(self.headers['Content-Length']))
        request = json.loads(raw)
        context = None
        for entry in request['messages']:
            try:
                candidate = json.loads(entry.get('content', ''))
                if isinstance(candidate, dict) and 'event' in candidate:
                    context = candidate
                    break
            except (ValueError, TypeError):
                pass
        if context is None:
            self.send_error(400, 'Missing fixture context')
            return
        event = context['event']
        deep = event.get('type') == 'plan_course' and event.get('mode') == 'deep'
        deep |= event.get('courseId') in type(self).deep_courses
        deep |= event.get('type') == 'prepare_next' and '完整深度讲义' in event.get('text', '')
        if not deep:
            self.rfile = io.BytesIO(raw)
            return super().do_POST()
        if event.get('courseId'):
            type(self).deep_courses.add(event['courseId'])
        type(self).calls.append({'event': event, 'currentDeepPage': context.get('lessonContext', {}).get('currentDeepPage')})
        results = []
        for message in request['messages']:
            if message['role'] == 'user':
                try:
                    results.extend(json.loads(message['content']).get('toolResults', []))
                except ValueError:
                    pass
        actions, state, message = [], 'completed', '隔离协议内容已保存。'
        if event['type'] == 'plan_course':
            actions = [{'id': 'deep-plan', 'tool': 'create_course', 'args': {
                'goal': '理解函数图像与参数变化', 'title': '深度隔离函数课程', 'mode': 'deep',
                'summary': '用于真实 C++ 路由与浏览器协议的隔离课程',
                'payload': {'courseStructure': [{'stage': '函数与图像', 'goal': '比较变化并解释参数作用',
                                                'topics': ['单调性的观察', '系数与曲线']}]}}}]
        elif event['type'] == 'prepare_next':
            lesson = next((item['result']['lessonId'] for item in results if item.get('result', {}).get('lessonId')), None)
            section_ids = [item['result']['sectionId'] for item in results if item.get('result', {}).get('sectionId')]
            if not lesson:
                actions = [{'id': 'deep-create', 'tool': 'create_lesson', 'args': {
                    'courseId': event['courseId'], 'title': '函数的单调性', 'purpose': '从函数值的变化理解单调性。',
                    'intent': 'advance', 'topicId': event['topicId']}}]
                state = 'continue'
            elif not section_ids:
                sections = [
                    {'kind': 'explanation', 'title': '观察函数的变化',
                     'body': '对于 $f(x)=2x+1$，取 x 为 1、2、3，函数值依次为 3、5、7。\n\n观察自变量增大时，函数值如何变化，再比较任意两个自变量对应的函数值。',
                     'helpType': 'explanation', 'demonstratedQuestions': []},
                    {'kind': 'question', 'questionKind': 'practice', 'title': '用自己的话说明',
                     'question': {'type': 'open', 'question': '你怎样判断函数 f(x)=2x+1 的变化趋势？',
                                  'expectedAnswer': 'PRIVATE-DEEP-EXPECTED', 'rubric': 'PRIVATE-DEEP-RUBRIC'}},
                    {'kind': 'summary', 'title': '从变化走向定义', 'body': '先观察变化，再用任意两个自变量说明同一区间内的大小关系。',
                     'helpType': 'explanation', 'demonstratedQuestions': []}]
                actions = [{'id': 'deep-section-' + str(index), 'tool': 'append_section', 'args': {'lessonId': lesson, 'section': section}}
                           for index, section in enumerate(sections)]
                state = 'continue'
            elif not any('contentVersion' in item.get('result', {}) for item in results):
                actions = [{'id': 'deep-content', 'tool': 'set_deep_content', 'args': {'lessonId': lesson, 'content': {
                    'lecture': [{'sectionId': section_ids[0], 'mediaIds': ['curve', 'timeline']}],
                    'slides': [{'id': 'slide-observe', 'title': '先观察，再判断', 'sourceSectionIds': [section_ids[0]], 'mediaIds': ['curve']},
                               {'id': 'slide-summary', 'title': '用定义说明', 'sourceSectionIds': [section_ids[2]], 'mediaIds': []}],
                    'media': [{'id': 'curve', 'type': 'function', 'title': '观察系数改变', 'coefficients': [1, 0, 0], 'xMin': -3, 'xMax': 3,
                               'parameter': {'name': '系数 a', 'min': -3, 'max': 3, 'step': 0.5, 'value': 1, 'coefficientIndex': 0}},
                              {'id': 'timeline', 'type': 'timeline', 'title': '思考过程', 'events': [{'label': '观察', 'text': '比较函数值'}, {'label': '说明', 'text': '解释变化规律'}]}]}}}]
                state = 'continue'
            else:
                actions = [{'id': 'deep-finish', 'tool': 'finish_lesson', 'args': {'lessonId': lesson}}]
        elif event['type'] == 'question_answer':
            actual = next(item for item in context['currentInputs'] if item['requestId'] == event['requestId'])
            actions = [{'id': 'deep-evaluate', 'tool': 'evaluate_answer', 'args': {
                'interactionId': actual['interactionId'], 'score': 80, 'credible': True,
                'feedback': '隔离回归反馈：已引用本次实际输入，接下来可以补充判断理由。',
                'feedbackHelpType': 'none', 'reason': '仅验证真实输入到评价保存的协议', 'uncertainty': '模拟模型，不用于证明教学质量',
                'diagnosis': {'summary': '已说明变化趋势，可以继续解释任意两点的关系。', 'evidence': [actual['text']],
                              'teachingDirection': '从具体数值比较过渡到函数定义', 'needsClarification': False}}}]
            state, message = 'waiting_student', '本次作答已保存。'
        else:
            state, message = 'waiting_student', '已结合当前课件与图示参数读取本课内容，可以继续说明你的困惑。'
        value = json.dumps({'teaching': {'helpType': 'explanation', 'demonstratedQuestions': []},
                            'message': message, 'actions': actions, 'state': state}, ensure_ascii=False)
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream')
        self.end_headers()
        try:
            for index in range(0, len(value), 55):
                chunk = {'model': 'fixture-deep-integration', 'choices': [{'delta': {'content': value[index:index + 55]}, 'finish_reason': None}]}
                self.wfile.write(('data: ' + json.dumps(chunk, ensure_ascii=False) + '\n\n').encode())
                self.wfile.flush()
            self.wfile.write(b'data: [DONE]\n\n')
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            pass


def main(executable, output):
    output.mkdir(parents=True, exist_ok=True)
    fixture.FixtureAI = DeepFixture
    errors, views = [], []
    with fixture.start_session(executable) as (info, api, _process):
        def wait(task_id):
            deadline = time.monotonic() + 90
            while time.monotonic() < deadline:
                value = api('/api/learning-agent?taskId=' + task_id)
                if value['status'] not in ('pending', 'running'):
                    assert value['status'] in ('ready', 'waiting_student'), value
                    return value
                time.sleep(0.1)
            raise AssertionError('深度课程隔离任务超时')

        planned = api('/api/generate-plan', {'goal': '理解函数与图像', 'mode': 'deep', 'requestId': 'deep-integration-plan'})
        course_id = wait(planned['id'])['course']['id']
        initial = api('/api/deep-course?' + urlencode({'courseId': course_id, 'phaseIndex': 1, 'topicIndex': 1}))
        assert initial['enabled'] and initial['lesson'] is None
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True, channel='msedge')
            context = browser.new_context(viewport={'width': 1280, 'height': 720}, reduced_motion='reduce')
            tab = context.new_page()
            tab.on('pageerror', lambda error: errors.append(str(error)))
            tab.goto(info['base'] + '/deep-course?' + urlencode({'courseId': course_id, 'phaseIndex': 1, 'topicIndex': 1}))
            tab.locator('[data-action=start]').click()
            tab.wait_for_function("!document.getElementById('dc-continue').disabled", timeout=90000)
            prepared = api('/api/deep-course?' + urlencode({'courseId': course_id, 'phaseIndex': 1, 'topicIndex': 1}))
            lesson_id = prepared['lesson']['id']
            assert prepared['lesson']['entered'] is True and prepared['content']['lecture']
            assert '正在读取已保存的课堂' not in tab.locator('#deep-course-app').inner_text()
            assert 'PRIVATE-DEEP' not in tab.content()
            before = len(DeepFixture.calls)
            tab.locator('[data-action="view:slides"]').click()
            assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == prepared['content']['lecture'][0]['sectionId']
            slider = tab.locator('.dc-slide input[type=range]')
            slider.focus()
            slider.press('ArrowRight')
            tab.locator('[data-action=next]').click()
            tab.locator('[data-action=previous]').click()
            assert len(DeepFixture.calls) == before
            tab.locator('#dc-chat-toggle').click()
            tab.locator('#dc-chat-input').fill('为什么改变参数后图像会变化？')
            tab.locator('#dc-chat-input').press('Enter')
            tab.wait_for_function("document.getElementById('dc-chat-input').value === ''")
            tab.wait_for_function("!document.getElementById('dc-continue').disabled", timeout=90000)
            chats = [call for call in DeepFixture.calls if isinstance(call, dict) and call['event']['type'] == 'chat' and call['event'].get('courseId') == course_id]
            assert chats and chats[-1]['event']['deepView'] == 'slides' and chats[-1]['event']['contentId'] == 'slide-observe'
            assert chats[-1]['event']['mediaState']['curve'] == 1.5
            assert chats[-1]['currentDeepPage']['id'] == 'slide-observe'
            assert chats[-1]['currentDeepPage']['media'][0]['currentParameterValue'] == 1.5
            tab.locator('[data-action=close-chat]').click()
            tab.locator('[data-action="view:lecture"]').click()
            tab.locator('.dc-question textarea').fill('x 增大时，函数值也增大；斜率为正。')
            tab.locator('.dc-question button[type=submit]').click()
            tab.wait_for_function("!document.getElementById('dc-continue').disabled", timeout=90000)
            assert api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['evaluations']
            before_continue = len(DeepFixture.calls)
            tab.locator('#dc-continue').click()
            tab.wait_for_function("!document.getElementById('dc-continue').disabled", timeout=90000)
            assert any(call['event']['type'] == 'lesson_finish_request' for call in DeepFixture.calls[before_continue:] if isinstance(call, dict))
            assert not api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson'].get('completed', False)
            assert tab.locator('#dc-continue').inner_text() == '继续学习'
            tab.locator('[data-action=close-chat]').click()
            before = len(DeepFixture.calls)
            tab.reload()
            tab.wait_for_function("!document.getElementById('dc-continue').disabled")
            assert len(DeepFixture.calls) == before
            for width, height in [(1280, 720), (1440, 900), (390, 844)]:
                tab.set_viewport_size({'width': width, 'height': height})
                tab.locator('#dc-reading').evaluate('node => { node.scrollTop = 0; }')
                check = tab.evaluate("() => ({width:innerWidth,contentWidth:document.documentElement.scrollWidth,footerBottom:document.querySelector('.dc-footer').getBoundingClientRect().bottom,viewportHeight:innerHeight})")
                assert check['contentWidth'] <= width + 1 and check['footerBottom'] <= height + 1, check
                views.append(check)
                tab.screenshot(path=str(output / f'deep-real-service-{width}.png'), full_page=True)
            tab.locator('#dc-directory-toggle').click()
            assert tab.locator('#dc-outline-dialog').is_visible()
            tab.keyboard.press('Escape')
            tab.locator('#dc-chat-toggle').click()
            assert tab.locator('#dc-chat-dialog').is_visible() and tab.locator('#dc-chat-input').is_visible()
            tab.screenshot(path=str(output / 'deep-real-service-390-chat.png'), full_page=True)
            tab.keyboard.press('Escape')
            assert not errors, errors
            pending = api('/api/deep-course/start', {'courseId': course_id, 'phaseIndex': 1, 'topicIndex': 2, 'requestId': 'second-deep-lesson'})
            second_task = wait(pending['task']['id'])
            second_id = second_task['lesson']['id']
            assert api('/api/deep-course?' + urlencode({'lessonId': second_id}))['lesson']['entered'] is False
            before = len(DeepFixture.calls)
            tab.goto(info['base'] + '/deep-course?' + urlencode({'lessonId': second_id}))
            tab.wait_for_function("!document.getElementById('dc-continue').disabled")
            assert api('/api/deep-course?' + urlencode({'lessonId': second_id}))['lesson']['entered'] is True
            assert len(DeepFixture.calls) == before
            browser.close()
        report = {'realCppService': True, 'mockAIOnly': True, 'courseId': course_id, 'lessonId': lesson_id,
                  'views': views, 'pageErrors': errors, 'modelHttpRequests': len(DeepFixture.calls),
                  'deepEvents': [call['event'] for call in DeepFixture.calls if isinstance(call, dict)],
                  'currentPageContexts': [call['currentDeepPage'] for call in DeepFixture.calls if isinstance(call, dict) and call['currentDeepPage']],
                  'cachedEntryWithoutGeneration': True, 'evaluatedContinueReachesController': True, 'passed': True}
        (output / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
        print(json.dumps({'passed': True, 'realCppService': True, 'mockAIOnly': True, 'views': len(views), 'pageErrors': errors}, ensure_ascii=False))


if __name__ == '__main__':
    main(sys.argv[1], Path(sys.argv[2] if len(sys.argv) > 2 else 'verification/deep-course-integration').resolve())

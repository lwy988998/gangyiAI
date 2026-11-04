# 【逐行批注】 执行当前 Python 语句："""真实 C++ 服务的深度课堂闭环验收；只使用临时档案与模拟模型。"""。
"""真实 C++ 服务的深度课堂闭环验收；只使用临时档案与模拟模型。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import io。
import io
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import urlencode。
from urllib.parse import urlencode

# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import sync_playwright。
from playwright.sync_api import sync_playwright
# 【逐行批注】 导入后续逻辑需要的模块或符号：import classroom_v560_fixture as fixture。
import classroom_v560_fixture as fixture


# 【逐行批注】 定义类 DeepFixture，组织相关数据与行为。
class DeepFixture(fixture.FixtureAI):
    # 【逐行批注】 计算右侧结果并赋给 calls，更新脚本状态。
    calls = []
    # 【逐行批注】 计算右侧结果并赋给 deep_courses，更新脚本状态。
    deep_courses = set()

    # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
    def do_POST(self):
        # 【逐行批注】 计算右侧结果并赋给 raw，更新脚本状态。
        raw = self.rfile.read(int(self.headers['Content-Length']))
        # 【逐行批注】 计算右侧结果并赋给 request，更新脚本状态。
        request = json.loads(raw)
        # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
        context = None
        # 【逐行批注】 遍历数据并重复执行循环体：for entry in request['messages']:。
        for entry in request['messages']:
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 计算右侧结果并赋给 candidate，更新脚本状态。
                candidate = json.loads(entry.get('content', ''))
                # 【逐行批注】 判断条件 isinstance(candidate, dict) and 'event' in candidate:，成立时执行对应分支。
                if isinstance(candidate, dict) and 'event' in candidate:
                    # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
                    context = candidate
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
            # 【逐行批注】 捕获并处理异常：except (ValueError, TypeError):。
            except (ValueError, TypeError):
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
        # 【逐行批注】 判断条件 context is None:，成立时执行对应分支。
        if context is None:
            # 【逐行批注】 执行当前 Python 语句：self.send_error(400, 'Missing fixture context')。
            self.send_error(400, 'Missing fixture context')
            # 【逐行批注】 结束当前函数并返回结果：return。
            return
        # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
        event = context['event']
        # 【逐行批注】 计算右侧结果并赋给 deep，更新脚本状态。
        deep = event.get('type') == 'plan_course' and event.get('mode') == 'deep'
        # 【逐行批注】 计算右侧结果并赋给 deep |，更新脚本状态。
        deep |= event.get('courseId') in type(self).deep_courses
        # 【逐行批注】 计算右侧结果并赋给 deep |，更新脚本状态。
        deep |= event.get('type') == 'prepare_next' and '完整深度讲义' in event.get('text', '')
        # 【逐行批注】 判断条件 not deep:，成立时执行对应分支。
        if not deep:
            # 【逐行批注】 计算右侧结果并赋给 self.rfile，更新脚本状态。
            self.rfile = io.BytesIO(raw)
            # 【逐行批注】 结束当前函数并返回结果：return super().do_POST()。
            return super().do_POST()
        # 【逐行批注】 判断条件 event.get('courseId'):，成立时执行对应分支。
        if event.get('courseId'):
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).deep_courses.add(event['courseId'])。
            type(self).deep_courses.add(event['courseId'])
        # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).calls.append({'event': event, 'currentDeepPage': context.g…。
        type(self).calls.append({'event': event, 'currentDeepPage': context.get('lessonContext', {}).get('currentDeepPage')})
        # 【逐行批注】 计算右侧结果并赋给 results，更新脚本状态。
        results = []
        # 【逐行批注】 遍历数据并重复执行循环体：for message in request['messages']:。
        for message in request['messages']:
            # 【逐行批注】 判断条件 message['role'] == 'user':，成立时执行对应分支。
            if message['role'] == 'user':
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 执行当前 Python 语句：results.extend(json.loads(message['content']).get('toolResults', []))。
                    results.extend(json.loads(message['content']).get('toolResults', []))
                # 【逐行批注】 捕获并处理异常：except ValueError:。
                except ValueError:
                    # 【逐行批注】 执行当前 Python 语句：pass。
                    pass
        # 【逐行批注】 计算右侧结果并赋给 actions, state, message，更新脚本状态。
        actions, state, message = [], 'completed', '隔离协议内容已保存。'
        # 【逐行批注】 判断条件 event['type'] == 'plan_course':，成立时执行对应分支。
        if event['type'] == 'plan_course':
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [{'id': 'deep-plan', 'tool': 'create_course', 'args': {
                # 【逐行批注】 执行当前 Python 语句：'goal': '理解函数图像与参数变化', 'title': '深度隔离函数课程', 'mode': 'deep',。
                'goal': '理解函数图像与参数变化', 'title': '深度隔离函数课程', 'mode': 'deep',
                # 【逐行批注】 执行当前 Python 语句：'summary': '用于真实 C++ 路由与浏览器协议的隔离课程',。
                'summary': '用于真实 C++ 路由与浏览器协议的隔离课程',
                # 【逐行批注】 执行当前 Python 语句：'payload': {'courseStructure': [{'stage': '函数与图像', 'goal': '比较变化并解释参数作用',。
                'payload': {'courseStructure': [{'stage': '函数与图像', 'goal': '比较变化并解释参数作用',
                                                # 【逐行批注】 执行当前 Python 语句：'topics': ['单调性的观察', '系数与曲线']}]}}}]。
                                                'topics': ['单调性的观察', '系数与曲线']}]}}}]
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'prepare_next':。
        elif event['type'] == 'prepare_next':
            # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
            lesson = next((item['result']['lessonId'] for item in results if item.get('result', {}).get('lessonId')), None)
            # 【逐行批注】 计算右侧结果并赋给 section_ids，更新脚本状态。
            section_ids = [item['result']['sectionId'] for item in results if item.get('result', {}).get('sectionId')]
            # 【逐行批注】 判断条件 not lesson:，成立时执行对应分支。
            if not lesson:
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [{'id': 'deep-create', 'tool': 'create_lesson', 'args': {
                    # 【逐行批注】 执行当前 Python 语句：'courseId': event['courseId'], 'title': '函数的单调性', 'purpose': '从函数值的变化理解单调性。',。
                    'courseId': event['courseId'], 'title': '函数的单调性', 'purpose': '从函数值的变化理解单调性。',
                    # 【逐行批注】 执行当前 Python 语句：'intent': 'advance', 'topicId': event['topicId']}}]。
                    'intent': 'advance', 'topicId': event['topicId']}}]
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'continue'
            # 【逐行批注】 当前一条件不成立时继续判断：not section_ids:。
            elif not section_ids:
                # 【逐行批注】 计算右侧结果并赋给 sections，更新脚本状态。
                sections = [
                    # 【逐行批注】 执行当前 Python 语句：{'kind': 'explanation', 'title': '观察函数的变化',。
                    {'kind': 'explanation', 'title': '观察函数的变化',
                     # 【逐行批注】 计算右侧结果并赋给 'body': '对于 $f(x)，更新脚本状态。
                     'body': '对于 $f(x)=2x+1$，取 x 为 1、2、3，函数值依次为 3、5、7。\n\n观察自变量增大时，函数值如何变化，再比较任意两个自变量对应的函数值。',
                     # 【逐行批注】 执行当前 Python 语句：'helpType': 'explanation', 'demonstratedQuestions': []},。
                     'helpType': 'explanation', 'demonstratedQuestions': []},
                    # 【逐行批注】 执行当前 Python 语句：{'kind': 'question', 'questionKind': 'practice', 'title': '用自己的话说明',。
                    {'kind': 'question', 'questionKind': 'practice', 'title': '用自己的话说明',
                     # 【逐行批注】 计算右侧结果并赋给 'question': {'type': 'open', 'question': '你怎…，更新脚本状态。
                     'question': {'type': 'open', 'question': '你怎样判断函数 f(x)=2x+1 的变化趋势？',
                                  # 【逐行批注】 执行当前 Python 语句：'expectedAnswer': 'PRIVATE-DEEP-EXPECTED', 'rubric': 'PRIVATE-DEEP-RUBRIC'}},。
                                  'expectedAnswer': 'PRIVATE-DEEP-EXPECTED', 'rubric': 'PRIVATE-DEEP-RUBRIC'}},
                    # 【逐行批注】 执行当前 Python 语句：{'kind': 'summary', 'title': '从变化走向定义', 'body': '先观察变化，再用任意两个自变量说明同一区间内的大小关系。',。
                    {'kind': 'summary', 'title': '从变化走向定义', 'body': '先观察变化，再用任意两个自变量说明同一区间内的大小关系。',
                     # 【逐行批注】 执行当前 Python 语句：'helpType': 'explanation', 'demonstratedQuestions': []}]。
                     'helpType': 'explanation', 'demonstratedQuestions': []}]
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [{'id': 'deep-section-' + str(index), 'tool': 'append_section', 'args': {'lessonId': lesson, 'section': section}}
                           # 【逐行批注】 遍历数据并重复执行循环体：for index, section in enumerate(sections)]。
                           for index, section in enumerate(sections)]
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'continue'
            # 【逐行批注】 当前一条件不成立时继续判断：not any('contentVersion' in item.get('result', {}) for item in r…。
            elif not any('contentVersion' in item.get('result', {}) for item in results):
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [{'id': 'deep-content', 'tool': 'set_deep_content', 'args': {'lessonId': lesson, 'content': {
                    # 【逐行批注】 执行当前 Python 语句：'lecture': [{'sectionId': section_ids[0], 'mediaIds': ['curve', 'timeline']}],。
                    'lecture': [{'sectionId': section_ids[0], 'mediaIds': ['curve', 'timeline']}],
                    # 【逐行批注】 执行当前 Python 语句：'slides': [{'id': 'slide-observe', 'title': '先观察，再判断', 'sourceSectionIds': [section_ids[0…。
                    'slides': [{'id': 'slide-observe', 'title': '先观察，再判断', 'sourceSectionIds': [section_ids[0]], 'mediaIds': ['curve']},
                               # 【逐行批注】 执行当前 Python 语句：{'id': 'slide-summary', 'title': '用定义说明', 'sourceSectionIds': [section_ids[2]], 'mediaIds…。
                               {'id': 'slide-summary', 'title': '用定义说明', 'sourceSectionIds': [section_ids[2]], 'mediaIds': []}],
                    # 【逐行批注】 执行当前 Python 语句：'media': [{'id': 'curve', 'type': 'function', 'title': '观察系数改变', 'coefficients': [1, 0, 0…。
                    'media': [{'id': 'curve', 'type': 'function', 'title': '观察系数改变', 'coefficients': [1, 0, 0], 'xMin': -3, 'xMax': 3,
                               # 【逐行批注】 执行当前 Python 语句：'parameter': {'name': '系数 a', 'min': -3, 'max': 3, 'step': 0.5, 'value': 1, 'coefficientI…。
                               'parameter': {'name': '系数 a', 'min': -3, 'max': 3, 'step': 0.5, 'value': 1, 'coefficientIndex': 0}},
                              # 【逐行批注】 执行当前 Python 语句：{'id': 'timeline', 'type': 'timeline', 'title': '思考过程', 'events': [{'label': '观察', 'text'…。
                              {'id': 'timeline', 'type': 'timeline', 'title': '思考过程', 'events': [{'label': '观察', 'text': '比较函数值'}, {'label': '说明', 'text': '解释变化规律'}]}]}}}]
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'continue'
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [{'id': 'deep-finish', 'tool': 'finish_lesson', 'args': {'lessonId': lesson}}]
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'question_answer':。
        elif event['type'] == 'question_answer':
            # 【逐行批注】 计算右侧结果并赋给 actual，更新脚本状态。
            actual = next(item for item in context['currentInputs'] if item['requestId'] == event['requestId'])
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [{'id': 'deep-evaluate', 'tool': 'evaluate_answer', 'args': {
                # 【逐行批注】 执行当前 Python 语句：'interactionId': actual['interactionId'], 'score': 80, 'credible': True,。
                'interactionId': actual['interactionId'], 'score': 80, 'credible': True,
                # 【逐行批注】 执行当前 Python 语句：'feedback': '隔离回归反馈：已引用本次实际输入，接下来可以补充判断理由。',。
                'feedback': '隔离回归反馈：已引用本次实际输入，接下来可以补充判断理由。',
                # 【逐行批注】 执行当前 Python 语句：'feedbackHelpType': 'none', 'reason': '仅验证真实输入到评价保存的协议', 'uncertainty': '模拟模型，不用于证明教学质量',。
                'feedbackHelpType': 'none', 'reason': '仅验证真实输入到评价保存的协议', 'uncertainty': '模拟模型，不用于证明教学质量',
                # 【逐行批注】 执行当前 Python 语句：'diagnosis': {'summary': '已说明变化趋势，可以继续解释任意两点的关系。', 'evidence': [actual['text']],。
                'diagnosis': {'summary': '已说明变化趋势，可以继续解释任意两点的关系。', 'evidence': [actual['text']],
                              # 【逐行批注】 执行当前 Python 语句：'teachingDirection': '从具体数值比较过渡到函数定义', 'needsClarification': False}}}]。
                              'teachingDirection': '从具体数值比较过渡到函数定义', 'needsClarification': False}}}]
            # 【逐行批注】 计算右侧结果并赋给 state, message，更新脚本状态。
            state, message = 'waiting_student', '本次作答已保存。'
        # 【逐行批注】 执行前一条件不成立时的分支。
        else:
            # 【逐行批注】 计算右侧结果并赋给 state, message，更新脚本状态。
            state, message = 'waiting_student', '已结合当前课件与图示参数读取本课内容，可以继续说明你的困惑。'
        # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
        value = json.dumps({'teaching': {'helpType': 'explanation', 'demonstratedQuestions': []},
                            # 【逐行批注】 计算右侧结果并赋给 'message': message, 'actions': actions, 'sta…，更新脚本状态。
                            'message': message, 'actions': actions, 'state': state}, ensure_ascii=False)
        # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
        self.send_response(200)
        # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'text/event-stream')。
        self.send_header('Content-Type', 'text/event-stream')
        # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
        self.end_headers()
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 遍历数据并重复执行循环体：for index in range(0, len(value), 55):。
            for index in range(0, len(value), 55):
                # 【逐行批注】 计算右侧结果并赋给 chunk，更新脚本状态。
                chunk = {'model': 'fixture-deep-integration', 'choices': [{'delta': {'content': value[index:index + 55]}, 'finish_reason': None}]}
                # 【逐行批注】 计算右侧结果并赋给 self.wfile.write(('data: ' + json.dumps(chun…，更新脚本状态。
                self.wfile.write(('data: ' + json.dumps(chunk, ensure_ascii=False) + '\n\n').encode())
                # 【逐行批注】 执行当前 Python 语句：self.wfile.flush()。
                self.wfile.flush()
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(b'data: [DONE]\n\n')。
            self.wfile.write(b'data: [DONE]\n\n')
        # 【逐行批注】 捕获并处理异常：except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):。
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable, output):
    # 【逐行批注】 计算右侧结果并赋给 output.mkdir(parents，更新脚本状态。
    output.mkdir(parents=True, exist_ok=True)
    # 【逐行批注】 计算右侧结果并赋给 fixture.FixtureAI，更新脚本状态。
    fixture.FixtureAI = DeepFixture
    # 【逐行批注】 计算右侧结果并赋给 errors, views，更新脚本状态。
    errors, views = [], []
    # 【逐行批注】 在受控上下文中执行资源操作：with fixture.start_session(executable) as (info, api, _process):。
    with fixture.start_session(executable) as (info, api, _process):
        # 【逐行批注】 定义函数 wait，封装可复用的处理步骤。
        def wait(task_id):
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = time.monotonic() + 90
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
            while time.monotonic() < deadline:
                # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
                value = api('/api/learning-agent?taskId=' + task_id)
                # 【逐行批注】 判断条件 value['status'] not in ('pending', 'running'):，成立时执行对应分支。
                if value['status'] not in ('pending', 'running'):
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert value['status'] in ('ready', 'waiting_student'), value。
                    assert value['status'] in ('ready', 'waiting_student'), value
                    # 【逐行批注】 结束当前函数并返回结果：return value。
                    return value
                # 【逐行批注】 执行当前 Python 语句：time.sleep(0.1)。
                time.sleep(0.1)
            # 【逐行批注】 在错误情形抛出异常：raise AssertionError('深度课程隔离任务超时')。
            raise AssertionError('深度课程隔离任务超时')

        # 【逐行批注】 计算右侧结果并赋给 planned，更新脚本状态。
        planned = api('/api/generate-plan', {'goal': '理解函数与图像', 'mode': 'deep', 'requestId': 'deep-integration-plan'})
        # 【逐行批注】 计算右侧结果并赋给 course_id，更新脚本状态。
        course_id = wait(planned['id'])['course']['id']
        # 【逐行批注】 计算右侧结果并赋给 initial，更新脚本状态。
        initial = api('/api/deep-course?' + urlencode({'courseId': course_id, 'phaseIndex': 1, 'topicIndex': 1}))
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert initial['enabled'] and initial['lesson'] is None。
        assert initial['enabled'] and initial['lesson'] is None
        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as playwright:。
        with sync_playwright() as playwright:
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser = playwright.chromium.launch(headless=True, channel='msedge')
            # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
            context = browser.new_context(viewport={'width': 1280, 'height': 720}, reduced_motion='reduce')
            # 【逐行批注】 计算右侧结果并赋给 tab，更新脚本状态。
            tab = context.new_page()
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：tab.on('pageerror', lambda error: errors.append(str(error)))。
            tab.on('pageerror', lambda error: errors.append(str(error)))
            # 【逐行批注】 调用 urlencode 完成当前脚本或测试步骤：tab.goto(info['base'] + '/deep-course?' + urlencode({'courseId': cour…。
            tab.goto(info['base'] + '/deep-course?' + urlencode({'courseId': course_id, 'phaseIndex': 1, 'topicIndex': 1}))
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=start]').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'", timeout=90000)
            # 【逐行批注】 计算右侧结果并赋给 prepared，更新脚本状态。
            prepared = api('/api/deep-course?' + urlencode({'courseId': course_id, 'phaseIndex': 1, 'topicIndex': 1}))
            # 【逐行批注】 计算右侧结果并赋给 lesson_id，更新脚本状态。
            lesson_id = prepared['lesson']['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert prepared['lesson']['entered'] is True and prepared['content']['lecture']。
            assert prepared['lesson']['entered'] is True and prepared['content']['lecture']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '正在读取已保存的课堂' not in tab.locator('#deep-course-app').inner_text()。
            assert '正在读取已保存的课堂' not in tab.locator('#deep-course-app').inner_text()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'PRIVATE-DEEP' not in tab.content()。
            assert 'PRIVATE-DEEP' not in tab.content()
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(DeepFixture.calls)
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-tab-slides').click()。
            tab.locator('#dc-tab-slides').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == prep…。
            assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == prepared['content']['lecture'][0]['sectionId']
            # 【逐行批注】 计算右侧结果并赋给 slider，更新脚本状态。
            slider = tab.locator('.dc-slide input[type=range]')
            # 【逐行批注】 执行当前 Python 语句：slider.focus()。
            slider.focus()
            # 【逐行批注】 执行当前 Python 语句：slider.press('ArrowRight')。
            slider.press('ArrowRight')
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=next]').click()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=previous]').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(DeepFixture.calls) == before。
            assert len(DeepFixture.calls) == before
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-toggle').click()。
            tab.locator('#dc-chat-toggle').click()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-input').fill('为什么改变参数后图像会变化？')。
            tab.locator('#dc-chat-input').fill('为什么改变参数后图像会变化？')
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-input').press('Enter')。
            tab.locator('#dc-chat-input').press('Enter')
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('dc-chat-input').value === ''")。
            tab.wait_for_function("document.getElementById('dc-chat-input').value === ''")
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'", timeout=90000)
            # 【逐行批注】 计算右侧结果并赋给 chats，更新脚本状态。
            chats = [call for call in DeepFixture.calls if isinstance(call, dict) and call['event']['type'] == 'chat' and call['event'].get('courseId') == course_id]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert chats and chats[-1]['event']['deepView'] == 'slides' and chats[-1]['even…。
            assert chats and chats[-1]['event']['deepView'] == 'slides' and chats[-1]['event']['contentId'] == 'slide-observe'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert chats[-1]['event']['mediaState']['curve'] == 1.5。
            assert chats[-1]['event']['mediaState']['curve'] == 1.5
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert chats[-1]['currentDeepPage']['id'] == 'slide-observe'。
            assert chats[-1]['currentDeepPage']['id'] == 'slide-observe'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert chats[-1]['currentDeepPage']['media'][0]['currentParameterValue'] == 1.5。
            assert chats[-1]['currentDeepPage']['media'][0]['currentParameterValue'] == 1.5
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=close-chat]').click()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-tab-lecture').click()。
            tab.locator('#dc-tab-lecture').click()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('.dc-question textarea').fill('x 增大时，函数值也增大；斜率为正。')。
            tab.locator('.dc-question textarea').fill('x 增大时，函数值也增大；斜率为正。')
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('.dc-question button[type，更新脚本状态。
            tab.locator('.dc-question button[type=submit]').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'", timeout=90000)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson'][…。
            assert api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['evaluations']
            # 【逐行批注】 计算右侧结果并赋给 before_continue，更新脚本状态。
            before_continue = len(DeepFixture.calls)
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-continue').click()。
            tab.locator('#dc-continue').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'", timeout=90000)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert any(call['event']['type'] == 'lesson_finish_request' for call in DeepFix…。
            assert any(call['event']['type'] == 'lesson_finish_request' for call in DeepFixture.calls[before_continue:] if isinstance(call, dict))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesso…。
            assert not api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson'].get('completed', False)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-continue').inner_text() == '继续学习'。
            assert tab.locator('#dc-continue').inner_text() == '继续学习'
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=close-chat]').click()
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(DeepFixture.calls)
            # 【逐行批注】 执行当前 Python 语句：tab.reload()。
            tab.reload()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(DeepFixture.calls) == before。
            assert len(DeepFixture.calls) == before
            # 【逐行批注】 遍历数据并重复执行循环体：for width, height in [(1280, 720), (1440, 900), (390, 844)]:。
            for width, height in [(1280, 720), (1440, 900), (390, 844)]:
                # 【逐行批注】 执行当前 Python 语句：tab.set_viewport_size({'width': width, 'height': height})。
                tab.set_viewport_size({'width': width, 'height': height})
                # 【逐行批注】 计算右侧结果并赋给 tab.locator('#dc-reading').evaluate('node，更新脚本状态。
                tab.locator('#dc-reading').evaluate('node => { node.scrollTop = 0; }')
                # 【逐行批注】 计算右侧结果并赋给 check，更新脚本状态。
                check = tab.evaluate("() => ({width:innerWidth,contentWidth:document.documentElement.scrollWidth,footerBottom:document.querySelector('.dc-footer').getBoundingClientRect().bottom,viewportHeight:innerHeight})")
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert check['contentWidth'] <= width + 1 and check['footerBottom'] <= height +…。
                assert check['contentWidth'] <= width + 1 and check['footerBottom'] <= height + 1, check
                # 【逐行批注】 执行当前 Python 语句：views.append(check)。
                views.append(check)
                # 【逐行批注】 计算右侧结果并赋给 tab.screenshot(path，更新脚本状态。
                tab.screenshot(path=str(output / f'deep-real-service-{width}.png'), full_page=True)
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-directory-toggle').click()。
            tab.locator('#dc-directory-toggle').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-outline-dialog').is_visible()。
            assert tab.locator('#dc-outline-dialog').is_visible()
            # 【逐行批注】 执行当前 Python 语句：tab.keyboard.press('Escape')。
            tab.keyboard.press('Escape')
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-toggle').click()。
            tab.locator('#dc-chat-toggle').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-chat-dialog').is_visible() and tab.locator('#dc-chat-in…。
            assert tab.locator('#dc-chat-dialog').is_visible() and tab.locator('#dc-chat-input').is_visible()
            # 【逐行批注】 计算右侧结果并赋给 tab.screenshot(path，更新脚本状态。
            tab.screenshot(path=str(output / 'deep-real-service-390-chat.png'), full_page=True)
            # 【逐行批注】 执行当前 Python 语句：tab.keyboard.press('Escape')。
            tab.keyboard.press('Escape')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, errors。
            assert not errors, errors
            # 【逐行批注】 计算右侧结果并赋给 pending，更新脚本状态。
            pending = api('/api/deep-course/start', {'courseId': course_id, 'phaseIndex': 1, 'topicIndex': 2, 'requestId': 'second-deep-lesson'})
            # 【逐行批注】 计算右侧结果并赋给 second_task，更新脚本状态。
            second_task = wait(pending['task']['id'])
            # 【逐行批注】 计算右侧结果并赋给 second_id，更新脚本状态。
            second_id = second_task['lesson']['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/deep-course?' + urlencode({'lessonId': second_id}))['lesson'][…。
            assert api('/api/deep-course?' + urlencode({'lessonId': second_id}))['lesson']['entered'] is False
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(DeepFixture.calls)
            # 【逐行批注】 调用 urlencode 完成当前脚本或测试步骤：tab.goto(info['base'] + '/deep-course?' + urlencode({'lessonId': seco…。
            tab.goto(info['base'] + '/deep-course?' + urlencode({'lessonId': second_id}))
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/deep-course?' + urlencode({'lessonId': second_id}))['lesson'][…。
            assert api('/api/deep-course?' + urlencode({'lessonId': second_id}))['lesson']['entered'] is True
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(DeepFixture.calls) == before。
            assert len(DeepFixture.calls) == before
            # 【逐行批注】 执行当前 Python 语句：browser.close()。
            browser.close()
        # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
        report = {'realCppService': True, 'mockAIOnly': True, 'courseId': course_id, 'lessonId': lesson_id,
                  # 【逐行批注】 调用 len 完成当前脚本或测试步骤：'views': views, 'pageErrors': errors, 'modelHttpRequests': len(DeepFi…。
                  'views': views, 'pageErrors': errors, 'modelHttpRequests': len(DeepFixture.calls),
                  # 【逐行批注】 调用 isinstance 完成当前脚本或测试步骤：'deepEvents': [call['event'] for call in DeepFixture.calls if isinsta…。
                  'deepEvents': [call['event'] for call in DeepFixture.calls if isinstance(call, dict)],
                  # 【逐行批注】 调用 isinstance 完成当前脚本或测试步骤：'currentPageContexts': [call['currentDeepPage'] for call in DeepFixtu…。
                  'currentPageContexts': [call['currentDeepPage'] for call in DeepFixture.calls if isinstance(call, dict) and call['currentDeepPage']],
                  # 【逐行批注】 执行当前 Python 语句：'cachedEntryWithoutGeneration': True, 'evaluatedContinueReachesController': True, 'passed…。
                  'cachedEntryWithoutGeneration': True, 'evaluatedContinueReachesController': True, 'passed': True}
        # 【逐行批注】 计算右侧结果并赋给 (output / 'report.json').write_text(json.dum…，更新脚本状态。
        (output / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
        # 【逐行批注】 输出当前运行结果：print(json.dumps({'passed': True, 'realCppService': True, 'mockAIOnly': Tr…。
        print(json.dumps({'passed': True, 'realCppService': True, 'mockAIOnly': True, 'views': len(views), 'pageErrors': errors}, ensure_ascii=False))


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 调用 main、Path、len 完成当前脚本或测试步骤：main(sys.argv[1], Path(sys.argv[2] if len(sys.argv) > 2 else 'verific…。
    main(sys.argv[1], Path(sys.argv[2] if len(sys.argv) > 2 else 'verification/deep-course-integration').resolve())

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
from threading import Event
from urllib.error import HTTPError
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
    independent_calls = []
    include_sets = False
    include_mastery = False
    include_intents = False
    include_labs = False
    include_rounds = False
    include_topic_evidence = False
    release_round_question = Event()
    attempted_early_question = False
    attempted_early_finish = False
    saw_feedback_stage_prompt = False
    saw_early_finish_error = False
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
            # 独立聊天只含通用会话，不能为满足旧测试而注入课程上下文。
            if '独立通用聊天助手' in request['messages'][0].get('content', ''):
                type(self).independent_calls.append(request)
                self.send_response(200)
                self.send_header('Content-Type', 'text/event-stream')
                self.end_headers()
                chunk = {'model': 'fixture-independent-chat', 'choices': [{'delta': {'content': '这是独立聊天的隔离回复。'}, 'finish_reason': 'stop'}]}
                self.wfile.write(('data: ' + json.dumps(chunk, ensure_ascii=False) + '\n\ndata: [DONE]\n\n').encode())
                return
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
        type(self).calls.append({'event': event, 'currentDeepPage': context.get('lessonContext', {}).get('currentDeepPage'),
                                 'currentInputs': context.get('currentInputs', []),
                                 'teachingPreferences': context.get('lessonContext', {}).get('teachingPreferences')})
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
            review = type(self).include_topic_evidence and event.get('text') == '请准备同一知识点的复习课'
            # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
            lesson = next((item['result']['lessonId'] for item in results if item.get('result', {}).get('lessonId')), None)
            initial_round = next((item['result']['roundId'] for item in results
                                  if item.get('id') == 'deep-round' and item.get('result', {}).get('roundId')), None)
            # 【逐行批注】 计算右侧结果并赋给 section_ids，更新脚本状态。
            section_ids = [item['result']['sectionId'] for item in results if item.get('result', {}).get('sectionId')]
            # 【逐行批注】 判断条件 not lesson:，成立时执行对应分支。
            if not lesson:
                if review:
                    assert context['topicEvidence']['status'] == 'available'
                    assert context['topicEvidence']['topicId'] == event['topicId']
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [{'id': 'deep-create', 'tool': 'create_lesson', 'args': {
                    # 【逐行批注】 执行当前 Python 语句：'courseId': event['courseId'], 'title': '函数的单调性', 'purpose': '从函数值的变化理解单调性。',。
                    'courseId': event['courseId'], 'title': '函数的单调性', 'purpose': '从函数值的变化理解单调性。',
                    # 【逐行批注】 执行当前 Python 语句：'intent': 'advance', 'topicId': event['topicId']}}]。
                    'intent': 'review' if review else 'advance', 'topicId': event['topicId']}}]
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'continue'
            # 【逐行批注】 当前一条件不成立时继续判断：not section_ids:。
            elif not initial_round:
                actions = [{'id': 'deep-round', 'tool': 'start_teaching_round', 'args': {
                    'lessonId': lesson, 'goal': '说明函数变化趋势', 'questionCount': 1}}]
                state = 'continue'
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
                if type(self).include_labs:
                    sections.append({'kind':'explanation','title':'隔离组件协议验证区','body':'只验证互动身份与回传，不作为学科教学质量样本。',
                                     'helpType':'explanation','demonstratedQuestions':[]})
                actions = [{'id': 'deep-section-' + str(index), 'tool': 'append_section', 'args': {'lessonId': lesson, 'section': section}}
                           # 【逐行批注】 遍历数据并重复执行循环体：for index, section in enumerate(sections)]。
                           for index, section in enumerate(sections)]
                for action in actions:
                    if action['args']['section']['kind'] == 'question':
                        action['args']['roundId'] = initial_round
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
        elif event['type'] == 'chat' and event.get('text') == '请记录本课理解状态':
            read = next((item['result'] for item in results if 'publishedEvaluations' in item.get('result', {})), None)
            if not read:
                actions = [{'id': 'read-state-basis', 'tool': 'read_lesson', 'args': {'lessonId': event['lessonId']}}]
                state, message = 'continue', ''
            elif not any(item.get('result', {}).get('learningStateSaved') for item in results):
                actions = [{'id': 'record-understanding', 'tool': 'save_learning_state', 'args': {
                    'lessonId': event['lessonId'], 'status': 'PARTIAL',
                    'reason': '已说明变化趋势，尚需补充任意两点的理由。<img src=x onerror=alert(1)>',
                    'uncertainty': '本次只有一道题，不代表已经长期掌握。',
                    'evidenceIds': [item['id'] for item in read['publishedEvaluations']], 'nextAction': 'practice'}}]
                state, message = 'continue', ''
            else:
                state, message = 'waiting_student', '理解情况已根据本次回答记录，可以换一道题进一步确认。'
        elif event['type'] == 'chat' and any(item.get('componentActivities') for item in context.get('currentInputs', [])):
            actual = next(item for item in context['currentInputs'] if item.get('componentActivities'))
            component_id = next(iter(actual['componentActivities']))
            trace = next((item.get('result') for item in results if item.get('result', {}).get('componentId') == component_id), None)
            if trace is None:
                actions = [{'id': 'fixture-read-trace', 'tool': 'read_component_activity', 'args': {
                    'lessonId': event['lessonId'], 'interactionId': actual['interactionId'],
                    'componentId': component_id, 'offset': 0, 'limit': 1}}]
                state, message = 'continue', ''
            else:
                assert len(trace['events']) == 1 and trace['offset'] == 0
                assert trace['events'][0]['observation']['scoringEligible'] is False
                type(self).calls.append({'traceRead': trace, 'requestId': event['requestId']})
                state, message = 'waiting_student', '隔离夹具已通过实际工具读取本课活动；这不证明真实模型教学质量。'
        elif type(self).include_rounds and event['type'] == 'chat' and event.get('text') in (
            '带我做一轮集合关系练习', '再开始一轮集合关系练习'
        ):
            round_state = context.get('lessonContext', {}).get('teachingRound')
            if not round_state or round_state['status'] != 'active':
                actions = [{'id': 'open-round', 'tool': 'start_teaching_round', 'args': {
                    'lessonId': event['lessonId'], 'goal': '区分子集和交集', 'questionCount': 2}}]
                state, message = 'continue', '本轮目标是区分子集和交集，共 2 题。'
            else:
                question = ('若 A={1}、B={1,2}，A 是 B 的子集吗？' if event['text'] == '带我做一轮集合关系练习'
                            else '若 C={2}、D={2,3}，C 是 D 的子集吗？')
                actions = [self.round_question(event, round_state, 'subset', question, '是')]
                state, message = 'waiting_student', '请先判断子集关系。'
        elif type(self).include_rounds and event.get('requestType') == 'revise_round':
            round_state = context['lessonContext']['teachingRound']
            revision = event['roundRevision']
            assert round_state['supersedesRoundId'] == revision['roundId']
            assert round_state['goal'] == revision['goal'] and round_state['questionCount'] == revision['questionCount']
            actions = [self.round_question(event, round_state, 'complement', '全集 U={1,2,3}，A={1} 的补集是什么？', '{2,3}')]
            state, message = 'waiting_student', '本轮先从给定全集的补集开始。'
        elif type(self).include_rounds and event['type'] == 'question_answer' and any(
            item.get('roundId') and '你怎样判断函数' not in item.get('questionSnapshot', {}).get('question', '')
            for item in context.get('currentInputs', [])
        ):
            actual = next(item for item in context['currentInputs'] if item['requestId'] == event['requestId'])
            round_state = context.get('lessonContext', {}).get('teachingRound', {})
            evaluated = round_state.get('evaluatedQuestionIds', [])
            if actual['questionId'] in evaluated:
                assert not any(entry.get('role') == 'system' and
                               '本题评价已经暂存，尚未向学生发布' in entry.get('content', '')
                               for entry in request['messages'])
                if len(evaluated) == 1:
                    assert '交集' in actual['text']
                    assert type(self).release_round_question.wait(25), '测试未恢复暂停的下一题生成'
                    actions = [self.round_question(event, round_state, 'intersection', 'A={1}、B={1,2}，A 与 B 的交集是什么？', '{1}')]
                    state, message = 'waiting_student', '你刚才提到了交集，我们用同一组集合比较两种概念。'
                else:
                    actions = [{'id': 'close-round', 'tool': 'complete_teaching_round', 'args': {
                        'lessonId': event['lessonId'], 'roundId': round_state['roundId']}}]
                    state, message = 'waiting_student', '两题均已反馈，本轮结束。'
            elif any(item.get('result', {}).get('evaluationId') for item in results):
                if len(evaluated) == 1 and actual['text'] == '{1}' and not type(self).attempted_early_finish:
                    type(self).saw_feedback_stage_prompt = any(
                        entry.get('role') == 'system' and '本题评价已经暂存，尚未向学生发布' in entry.get('content', '')
                        for entry in request['messages'])
                    assert type(self).saw_feedback_stage_prompt
                    type(self).attempted_early_finish = True
                    actions = [{'id': 'premature-close', 'tool': 'complete_teaching_round', 'args': {
                        'lessonId': event['lessonId'], 'roundId': round_state['roundId']}}]
                    state, message = 'continue', ''
                elif len(evaluated) == 1 and actual['text'] == '{1}' and not type(self).saw_early_finish_error:
                    errors = []
                    for entry in request['messages']:
                        try:
                            errors.append(json.loads(entry.get('content', '')).get('toolError', ''))
                        except (ValueError, TypeError, AttributeError):
                            pass
                    assert any('评价已暂存' in error and 'message' in error for error in errors)
                    type(self).saw_early_finish_error = True
                    state, message = 'continue', '交集是 {1}，你的判断正确。'
                elif not type(self).attempted_early_question:
                    type(self).attempted_early_question = True
                    actions = [self.round_question(event, round_state, 'premature', '这道题不应在反馈前保存。', '无')]
                    state, message = 'continue', ''
                else:
                    state, message = 'waiting_student', '请对照反馈检查你的判断。'
            else:
                actions = [{'id': 'grade-round', 'tool': 'evaluate_answer', 'args': {
                    'interactionId': actual['interactionId'], 'score': 100 if actual['text'] == '{1}' else 0, 'credible': True,
                    'feedback': '交集是 {1}。' if actual['text'] == '{1}' else '子集检查 A 的每个元素是否都在 B 中；交集给出共有元素。',
                    'feedbackHelpType': 'explanation', 'reason': '依据实际提交', 'uncertainty': '只检验当前题',
                    'diagnosis': {'summary': '已核对所用概念', 'evidence': [actual['text']],
                                  'teachingDirection': '区分包含和共有', 'needsClarification': False}}}]
                state, message = 'continue', ''
        elif event['type'] == 'question_answer':
            # 【逐行批注】 计算右侧结果并赋给 actual，更新脚本状态。
            actual = next(item for item in context['currentInputs'] if item['requestId'] == event['requestId'])
            round_state = context.get('lessonContext', {}).get('teachingRound', {})
            evaluated = round_state.get('evaluatedQuestionIds', [])
            if actual['questionId'] in evaluated:
                actions = [{'id': 'deep-close-round', 'tool': 'complete_teaching_round', 'args': {
                    'lessonId': event['lessonId'], 'roundId': round_state['roundId']}}]
                state, message = 'waiting_student', '本轮作答和反馈已完成。'
            elif any(item.get('result', {}).get('evaluationId') for item in results):
                actions, state, message = [], 'waiting_student', '本次作答已保存。'
            else:
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
        if type(self).include_sets:
            for action in actions:
                if action['tool'] == 'set_deep_content':
                    content = action['args']['content']
                    content['media'].append({'id': 'set-lab', 'type': 'sets', 'title': '探索集合的交集',
                                             'universe': ['1', '2', '3', '4'], 'setA': ['1', '2'], 'setB': ['2', '3']})
                    content['lecture'][0]['mediaIds'].append('set-lab')
                    content['slides'][0]['mediaIds'].append('set-lab')
        if type(self).include_labs:
            for action in actions:
                if action['tool'] == 'set_deep_content':
                    content = action['args']['content']
                    labs = [{'id':'function-lab','type':'function_lab','title':'探索函数图像','coefficients':[1,0,0],
                             'domain':{'min':0,'max':2},'xMin':-3,'xMax':3},
                            {'id':'circuit-lab','type':'circuit_lab','title':'比较串联和并联','voltage':6,'r1':10,'r2':20},
                            {'id':'chemical-lab','type':'chemical_balance','title':'探索原子守恒','reactionId':'water'},
                            {'id':'cell-lab','type':'cell_lab','title':'细胞结构'},
                            {'id':'history-lab','type':'history_lab','title':'事件排序'},
                            {'id':'climate-lab','type':'climate_lab','title':'教学气候数据比较'},
                            {'id':'annotation-lab','type':'annotation_lab','title':'阅读标注','fragments':['雨停了。','书页上留下光。']},
                            {'id':'sentence-lab','type':'sentence_lab','title':'句子结构','fragments':['The students','read','a book','in the library.']},
                            {'id':'algorithm-lab','type':'algorithm_lab','title':'排序步进','values':[5,2,4,1]}]
                    if getattr(type(self), 'binary_search', False):
                        labs[-1].update(title='二分查找步进', algorithm='binary_search',
                                        values=[1,3,5,7,9,11,13,15], target=11)
                    if getattr(type(self), 'motion_lab', False):
                        labs.append({'id':'motion-lab','type':'motion_lab','title':'直线运动',
                                     'initialPositionM':0,'initialVelocityMps':6,'accelerationMps2':-2})
                    if getattr(type(self), 'number_line', False):
                        labs.append({'id':'number-line','type':'number_line','title':'数轴与区间'})
                    if getattr(type(self), 'vector_lab', False):
                        labs.append({'id':'vector-lab','type':'vector_lab','title':'平面向量'})
                    if getattr(type(self), 'statistics_lab', False):
                        labs.append({'id':'statistics-lab','type':'statistics_lab','title':'数据与统计量','values':[2,3,3,4,8]})
                    if getattr(type(self), 'equation_lab', False):
                        labs.append({'id':'equation-lab','type':'equation_lab','title':'等式两边的变形',
                                     'left':[3,1],'right':[0,2]})
                    if getattr(type(self), 'annotation_selection', False):
                        labs[6].update(mode='selection',fragments=['甲🌱乙，','丙丁。'])
                    content['media'].extend(labs)
                    content['lecture'][0]['mediaIds'].extend(item['id'] for item in labs[:3])
                    # 六科放在另一真实板块，避免所有组件在同一页掩盖解释请求绑定错误。
                    content['lecture'].append({'sectionId':section_ids[3],
                                               'mediaIds':[item['id'] for item in labs[3:]]})
                    content['slides'][0]['mediaIds'].extend(item['id'] for item in labs)
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
    @staticmethod
    def round_question(event, round_state, name, question, answer):
        return {'id': name, 'tool': 'append_section', 'args': {
            'lessonId': event['lessonId'], 'roundId': round_state['roundId'],
            'section': {'kind': 'question', 'questionKind': 'interaction', 'title': '集合关系',
                        'question': {'question': question, 'type': 'open', 'expectedAnswer': answer, 'rubric': '按真实回答核对'}}}}


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
            if DeepFixture.include_mastery:
                # 仅在测试页捕获真实订阅回调，用于重现晚到终态的重复读取。
                tab.add_init_script("""(() => {
                    let value;
                    Object.defineProperty(window, 'GangyiAgent', {configurable: true,
                        get: () => value, set: api => {
                            value = api;
                            const watch = api.watch;
                            api.watch = (id, handlers) => watch(id, {...handlers, onState(task) {
                                window.__masteryWatch = {handlers, task};
                                handlers.onState?.(task);
                            }});
                        }});
                })();""")
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：tab.on('pageerror', lambda error: errors.append(str(error)))。
            tab.on('pageerror', lambda error: errors.append(str(error)))
            # 【逐行批注】 调用 urlencode 完成当前脚本或测试步骤：tab.goto(info['base'] + '/deep-course?' + urlencode({'courseId': cour…。
            tab.goto(info['base'] + '/deep-course?' + urlencode({'courseId': course_id, 'phaseIndex': 1, 'topicIndex': 1}))
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            fixture.open_learning_menu(tab).locator('#dc-continue').click()
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
            fixture.open_learning_menu(tab).locator('#dc-tab-slides').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == prep…。
            assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == prepared['content']['lecture'][0]['sectionId']
            # 【逐行批注】 计算右侧结果并赋给 slider，更新脚本状态。
            slider = tab.locator('.dc-slide [data-media-id=curve] input[type=range]')
            # 【逐行批注】 执行当前 Python 语句：slider.focus()。
            slider.focus()
            # 【逐行批注】 执行当前 Python 语句：slider.press('ArrowRight')。
            slider.press('ArrowRight')
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            fixture.open_learning_menu(tab).locator('[data-action=next]').click()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            fixture.open_learning_menu(tab).locator('[data-action=previous]').click()
            tab.keyboard.press('Escape')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(DeepFixture.calls) == before。
            assert len(DeepFixture.calls) == before
            if DeepFixture.include_sets:
                lab = tab.locator('[data-media-id=set-lab]')
                assert lab.locator('.il-sets-result').inner_text() == 'A ∩ B = { 2 }'
                assert lab.locator('.il-sets-diagram svg').get_attribute('data-relation') == 'overlap'
                checkbox = lab.get_by_role('checkbox', name='1 属于集合 B', exact=True)
                checkbox.focus()
                checkbox.press('Space')
                assert lab.locator('.il-sets-result').inner_text() == 'A ∩ B = { 1, 2 }'
                assert len(DeepFixture.calls) == before
                deadline = time.monotonic() + 20
                while time.monotonic() < deadline:
                    drafts = api('/api/deep-course?' + urlencode({'lessonId': lesson_id})).get('componentDrafts', {})
                    if drafts.get('set-lab', {}).get('state', {}).get('setB') == ['1', '2', '3']:
                        break
                    time.sleep(.1)
                else:
                    raise AssertionError('未提交的组件探索没有保存为草稿')
                assert drafts['set-lab']['trace']['totalChanges'] == 1
                draft_context = browser.new_context(viewport={'width': 1280, 'height': 720}, reduced_motion='reduce')
                draft_tab = draft_context.new_page()
                draft_tab.on('pageerror', lambda error: errors.append(str(error)))
                draft_tab.goto(info['base'] + '/deep-course?' + urlencode({'lessonId': lesson_id}))
                draft_tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                draft_lab = draft_tab.locator('[data-media-id=set-lab]').first
                assert draft_lab.get_by_role('checkbox', name='1 属于集合 B', exact=True).is_checked()
                assert draft_lab.locator('.il-sets-result').inner_text() == 'A ∩ B = { 1, 2 }'
                assert len(DeepFixture.calls) == before
                draft_context.close()
                tab.reload()
                tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                lab = tab.locator('[data-media-id=set-lab]')
                assert lab.get_by_role('checkbox', name='1 属于集合 B', exact=True).is_checked()
                assert lab.locator('.il-note').inner_text() == '已恢复上次探索。'
                assert len(DeepFixture.calls) == before
                lab.get_by_role('button', name='请老师解释当前结果').click()
                tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                deadline = time.monotonic() + 30
                while time.monotonic() < deadline:
                    explore_calls = [call for call in DeepFixture.calls if isinstance(call, dict) and call.get('event', {}).get('type') == 'chat' and 'set-lab' in call['event'].get('mediaState', {})]
                    if explore_calls:
                        break
                    time.sleep(0.1)
                assert explore_calls
                observed = explore_calls[-1]['event']['mediaState']['set-lab']
                assert observed['intersection'] == ['1', '2']
                assert observed['kind'] == 'exploration' and observed['scoringEligible'] is False
                assert observed['resultSource'] == 'program_calculation'
                assert 'componentActivities' not in explore_calls[-1]['event']
                actual = next(item for item in explore_calls[-1]['currentInputs'] if item.get('componentActivities'))
                activity = actual['componentActivities']['set-lab']
                assert activity['totalChanges'] == 2 and activity['storedEvents'] == 2
                assert 'events' not in activity and activity['scoringEligible'] is False
                assert [item['kind'] for item in activity['recentEvents']] == ['change', 'explain']
                assert activity['recentEvents'][0]['observation']['intersection'] == ['1', '2']
                deadline = time.monotonic() + 30
                while time.monotonic() < deadline:
                    read_back = [call['traceRead'] for call in DeepFixture.calls if isinstance(call, dict) and call.get('requestId') == explore_calls[-1]['event']['requestId'] and 'traceRead' in call]
                    if read_back: break
                    time.sleep(.1)
                assert read_back and read_back[0]['traceId'] == activity['traceId']
                assert not api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['evaluations']
                wait(api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['teachingTaskId'])
                # 新浏览器不共享 localStorage，已提交的真实探索仍须从服务端记录恢复。
                recovered = api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['componentRecovery']['set-lab']
                assert recovered['source'] == 'submitted_component_activity'
                assert recovered['state']['setB'] == ['1', '2', '3']
                recovery_context = browser.new_context(viewport={'width': 1280, 'height': 720}, reduced_motion='reduce')
                recovery_tab = recovery_context.new_page()
                recovery_tab.on('pageerror', lambda error: errors.append(str(error)))
                recovery_tab.goto(info['base'] + '/deep-course?' + urlencode({'lessonId': lesson_id}))
                recovery_tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                recovered_lab = recovery_tab.locator('[data-media-id=set-lab]').first
                assert recovered_lab.get_by_role('checkbox', name='1 属于集合 B', exact=True).is_checked()
                assert recovered_lab.locator('.il-sets-result').inner_text() == 'A ∩ B = { 1, 2 }'
                diagram = recovered_lab.locator('.il-sets-diagram svg')
                def geometry():
                    return diagram.evaluate("""svg => ({relation: svg.dataset.relation,
                        circles: [...svg.querySelectorAll('circle')].map(circle => ({
                            x: Number(circle.getAttribute('cx')), y: Number(circle.getAttribute('cy')),
                            r: Number(circle.getAttribute('r')), visible: circle.getAttribute('visibility') !== 'hidden'}))})""")
                subset = geometry()
                assert subset['relation'] == 'a-subset'
                a, b = subset['circles']
                assert a['visible'] and b['visible'] and ((a['x'] - b['x']) ** 2 + (a['y'] - b['y']) ** 2) ** .5 + a['r'] <= b['r']
                recovered_lab.get_by_role('checkbox', name='3 属于集合 B', exact=True).uncheck()
                assert geometry()['relation'] == 'equal' and sum(c['visible'] for c in geometry()['circles']) == 1
                for width, height in [(1280, 720), (1440, 900), (390, 844)]:
                    recovery_tab.set_viewport_size({'width': width, 'height': height})
                    assert recovery_tab.evaluate('document.documentElement.scrollWidth <= innerWidth + 1')
                    recovered_lab.screenshot(path=str(output / f'sets-equal-{width}.png'))
                recovery_tab.set_viewport_size({'width': 1280, 'height': 720})
                for value in ['1', '2']:
                    recovered_lab.get_by_role('checkbox', name=f'{value} 属于集合 A', exact=True).uncheck()
                    recovered_lab.get_by_role('checkbox', name=f'{value} 属于集合 B', exact=True).uncheck()
                assert geometry()['relation'] == 'both-empty' and not any(c['visible'] for c in geometry()['circles'])
                recovered_lab.get_by_role('checkbox', name='3 属于集合 B', exact=True).check()
                assert geometry()['relation'] == 'a-empty' and not geometry()['circles'][0]['visible']
                recovered_lab.get_by_role('checkbox', name='1 属于集合 A', exact=True).check()
                separate = geometry()
                a, b = separate['circles']
                assert separate['relation'] == 'disjoint' and abs(a['x'] - b['x']) > a['r'] + b['r']
                recovered_lab.get_by_role('checkbox', name='1 属于集合 B', exact=True).check()
                assert geometry()['relation'] == 'a-subset'
                recovered_lab.get_by_role('checkbox', name='3 属于集合 B', exact=True).uncheck()
                recovered_lab.get_by_role('checkbox', name='2 属于集合 A', exact=True).check()
                inverse = geometry()
                a, b = inverse['circles']
                assert inverse['relation'] == 'b-subset' and abs(a['x'] - b['x']) + b['r'] <= a['r']
                recovered_lab.get_by_role('checkbox', name='1 属于集合 B', exact=True).uncheck()
                assert geometry()['relation'] == 'b-empty' and not geometry()['circles'][1]['visible']
                recovery_context.close()
                lab = tab.locator('[data-media-id=set-lab]')
                lab.get_by_role('button', name='重置探索').click()
                assert lab.locator('.il-sets-result').inner_text() == 'A ∩ B = { 2 }'
                lab.get_by_role('button', name='完成探索，继续教学').click()
                tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                deadline = time.monotonic() + 30
                while time.monotonic() < deadline:
                    finished = [item for item in DeepFixture.calls if isinstance(item, dict) and
                        item.get('event', {}).get('text', '').startswith('我已完成当前组件探索')]
                    if finished and any(call.get('requestId') == finished[-1]['event']['requestId'] and 'traceRead' in call for call in DeepFixture.calls if isinstance(call, dict)): break
                    time.sleep(.1)
                assert finished and any(call.get('requestId') == finished[-1]['event']['requestId'] and 'traceRead' in call for call in DeepFixture.calls if isinstance(call, dict))
                final_input = next(item for item in finished[-1]['currentInputs'] if item.get('componentActivities'))
                assert [item['kind'] for item in final_input['componentActivities']['set-lab']['recentEvents']] == ['change', 'explain', 'reset', 'finish']
                for content_id, request_id in [('slide-observe', 'sets-stale-state'), ('nonexistent-page', 'sets-invalid-page')]:
                    stale = {'schemaVersion': 1, 'contentVersion': -1, 'setA': ['1'], 'setB': ['1'], 'score': 100}
                    task = api('/api/learning-agent/events', {'type': 'chat', 'courseId': course_id, 'lessonId': lesson_id,
                               'view': 'learn', 'deepView': 'slides', 'contentId': content_id,
                               'text': '核对这份操作状态。', 'mediaState': {'set-lab': stale}, 'requestId': request_id})
                    wait(task['id'])
                    checked_call = [call for call in DeepFixture.calls if isinstance(call, dict) and call.get('event', {}).get('requestId') == request_id][-1]
                    assert checked_call['event']['mediaState'] == {}
                lifecycle = tab.evaluate("""() => {
                  const host = document.createElement('div'); document.body.append(host);
                  let changes = 0;
                  const component = GangyiInteractive.mount(host, {type:'sets', universe:['x'], setA:[], setB:[]},
                    {contentVersion:1, onChange:()=>changes++});
                  const checkbox = host.querySelector('input');
                  component.destroy(); checkbox.click(); host.remove();
                  return {changes, detached: !checkbox.isConnected};
                }""")
                assert lifecycle == {'changes': 0, 'detached': True}
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-toggle').click()。
            fixture.open_learning_menu(tab).locator('#dc-chat-toggle').click()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-input').fill('为什么改变参数后图像会变化？')。
            tab.locator('#dc-chat-input').fill('为什么改变参数后图像会变化？')
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-input').press('Enter')。
            tab.locator('#dc-chat-input').press('Enter')
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('dc-chat-input').value === ''")。
            tab.wait_for_function("document.getElementById('dc-chat-input').value === ''")
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'", timeout=90000)
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                chat_state = api('/api/independent-chat?' + urlencode({'lessonId': lesson_id}))
                if any(item.get('role') == 'assistant' and item.get('status') == 'ready' for item in chat_state.get('messages', [])):
                    break
                time.sleep(0.1)
            else:
                raise AssertionError('独立聊天没有保存完整回复')
            assert DeepFixture.independent_calls
            encoded_chat = json.dumps(DeepFixture.independent_calls[-1], ensure_ascii=False)
            for private_context in ['PRIVATE-DEEP', course_id, lesson_id, 'currentDeepPage', 'currentParameterValue', '单调性的观察']:
                assert private_context not in encoded_chat, private_context
            assert not api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['evaluations']
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=close-chat]').click()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-tab-lecture').click()。
            fixture.open_learning_menu(tab).locator('#dc-tab-lecture').click()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('.dc-question textarea').fill('x 增大时，函数值也增大；斜率为正。')。
            tab.locator('.dc-question textarea').fill('x 增大时，函数值也增大；斜率为正。')
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('.dc-question button[type，更新脚本状态。
            with tab.expect_response(lambda response: response.request.method == 'POST' and
                                     response.url.endswith('/api/learning-agent/events')) as submitted:
                tab.locator('.dc-question button[type=submit]').click()
            payload = submitted.value.request.post_data_json
            for component_id, trace in payload.get('componentActivities', {}).items():
                assert trace['events'][-1]['state'] == payload['mediaState'][component_id], (
                    component_id, trace['events'][-1], payload['mediaState'][component_id])
            assert submitted.value.status == 202, submitted.value.text()
            wait(submitted.value.json()['id'])
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'", timeout=90000)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson'][…。
            answered_lesson = api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']
            assert answered_lesson['evaluations']
            learning_state = answered_lesson['learningState']
            assert learning_state['lessonId'] == lesson_id and learning_state['courseId'] == course_id
            assert learning_state['evidence']['distinctQuestionCount'] == 1
            assert learning_state['mastery']['status'] == 'UNKNOWN'
            assert 'PRIVATE-DEEP' not in json.dumps(learning_state)
            if DeepFixture.include_labs:
                initial_evaluations = answered_lesson['evaluations']
                def assert_recovered_result(host, expected):
                    assert host.locator('.il-lab-result').inner_text() == expected
                def fresh_component(media_id, inspect_component):
                    recovery_context = browser.new_context(viewport={'width': 1280, 'height': 720}, reduced_motion='reduce')
                    recovery_tab = recovery_context.new_page()
                    recovery_tab.on('pageerror', lambda error: errors.append(str(error)))
                    try:
                        recovery_tab.goto(info['base'] + '/deep-course?' + urlencode({'lessonId': lesson_id}))
                        recovery_tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                        inspect_component(recovery_tab.locator(f'[data-media-id={media_id}]').first)
                    finally:
                        recovery_context.close()
                tab.wait_for_function("document.querySelector('[data-media-id=function-lab] [data-plot-ready=true]')")
                for media_id, label, value in [('function-lab','a','2'),('circuit-lab','电源电压 / V','12'),('chemical-lab','H₂ 的系数','2')]:
                    host = tab.locator(f'[data-media-id={media_id}]')
                    control = host.get_by_role('slider', name=label) if media_id != 'chemical-lab' else host.get_by_role('spinbutton', name=label)
                    if media_id == 'chemical-lab':
                        control.fill(value)
                        host.get_by_role('spinbutton', name='H₂O 的系数').fill('2')
                    else:
                        before_key = float(control.input_value())
                        control.focus(); control.press('ArrowRight')
                        assert float(control.input_value()) > before_key
                        control.evaluate('(node,value)=>{node.value=value;node.dispatchEvent(new Event("input",{bubbles:true}));}', value)
                    if media_id == 'circuit-lab':
                        host.get_by_role('combobox',name='连接方式').select_option('parallel')
                    before = len(DeepFixture.calls)
                    host.get_by_role('button',name='请老师解释当前结果').click()
                    tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                    tab.wait_for_load_state('networkidle')
                    call = next(call for call in DeepFixture.calls[before:] if isinstance(call,dict) and media_id in call['event'].get('mediaState',{}))
                    observed = call['event']['mediaState'][media_id]
                    assert observed['resultSource'] == 'program_calculation' and observed['scoringEligible'] is False
                    if media_id == 'function-lab':
                        assert observed['yAtOne'] == 2
                        analysis = observed['functionAnalysis']
                        assert analysis['domain'] == {'kind':'closed_interval','min':0,'max':2}
                        assert analysis['range'] == {'min':0,'max':8}
                        assert analysis['zeros'] == {'kind':'finite','values':[0]}
                        assert '定义域：[0, 2]' in host.locator('.il-lab-result').inner_text()
                        assert '值域：[0, 8]' in host.locator('.il-lab-result').inner_text()
                        actual_input = next(item for item in call['currentInputs'] if media_id in item.get('componentActivities',{}))
                        activity = actual_input['componentActivities'][media_id]
                        assert activity['lastObserved']['functionAnalysis'] == analysis
                        ranges = [item['observation']['functionAnalysis']['range']['max'] for item in activity['recentEvents']]
                        assert 6 in ranges and ranges[-1] == 8, ranges
                    if media_id == 'circuit-lab':
                        assert abs(observed['totalCurrent']-1.8) < 1e-9
                    if media_id == 'chemical-lab':
                        assert observed['balanced'] is True and observed['leftAtoms'] == {'H':4,'O':2}
                    assert api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['evaluations'] == initial_evaluations
                    wait(api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['teachingTaskId'])
                    def check_recovered(host):
                        recovered_control = host.get_by_role('slider', name=label) if media_id != 'chemical-lab' else host.get_by_role('spinbutton', name=label)
                        assert recovered_control.input_value() == value
                    fresh_component(media_id, check_recovered)
                    tab.reload()
                    tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                    host = tab.locator(f'[data-media-id={media_id}]')
                    control = host.get_by_role('slider',name=label) if media_id != 'chemical-lab' else host.get_by_role('spinbutton',name=label)
                    assert control.input_value() == value
                    if media_id == 'chemical-lab':
                        assert '原子数相等' in host.locator('.il-lab-result').inner_text()
                    if media_id == 'function-lab':
                        tab.wait_for_function("document.querySelector('[data-media-id=function-lab] [data-plot-ready=true]')")
                        assert '值域：[0, 8]' in host.locator('.il-lab-result').inner_text()
                    for width,height in [(1280,720),(1440,900),(390,844)]:
                        tab.set_viewport_size({'width':width,'height':height})
                        host.scroll_into_view_if_needed()
                        assert tab.evaluate('document.documentElement.scrollWidth<=innerWidth+1')
                        tab.screenshot(path=str(output/f'{media_id}-{width}.png'),full_page=True)
                    tab.set_viewport_size({'width':1280,'height':720})
                    host.get_by_role('button',name='重置探索').click()
                    assert control.input_value() == ('6' if media_id == 'circuit-lab' else '1')
                # 六科均从真实课堂操作回传，服务端按当前保存配置重算；不得生成正式评价。
                for media_id in ['cell-lab','history-lab','climate-lab','annotation-lab','sentence-lab','algorithm-lab']:
                    host=tab.locator(f'[data-media-id={media_id}]')
                    if media_id=='cell-lab':
                        host.get_by_role('button',name='叶绿体',exact=True).focus();tab.keyboard.press('Enter')
                    elif media_id=='history-lab':
                        host.get_by_role('button',name='阿波罗11号发射提前',exact=True).click()
                        host.get_by_role('button',name='阿波罗11号发射提前',exact=True).click()
                    elif media_id=='climate-lab':
                        host.get_by_role('combobox',name='观察月份').select_option('7')
                        host.get_by_role('combobox',name='比较月份').select_option('1')
                    elif media_id=='annotation-lab':
                        host.get_by_role('combobox',name='片段1：雨停了。',exact=True).select_option('3')
                    elif media_id=='sentence-lab':
                        host.get_by_role('combobox',name='片段1：The students',exact=True).select_option('1')
                    else:
                        host.get_by_role('button',name='下一步',exact=True).focus();tab.keyboard.press('Enter')
                    explored=host.locator('.il-lab-result').inner_text()
                    before=len(DeepFixture.calls)
                    if media_id=='cell-lab':
                        # 相邻两个板块同时可见，解释必须绑定下方组件所在板块。
                        tab.locator('#dc-reading').evaluate("node=>{const blocks=node.querySelectorAll('.dc-lecture-section');node.scrollTop=blocks[1].offsetTop-node.offsetTop-160;}")
                        assert tab.locator('.dc-lecture-section').first.evaluate("node=>{const box=node.getBoundingClientRect(),reader=document.getElementById('dc-reading').getBoundingClientRect();return box.bottom>reader.top+12;}")
                        failed_payloads=[]
                        def fail_submission(route):
                            failed_payloads.append(route.request.post_data_json)
                            route.abort('failed')
                        # 首次请求明确未送达服务端；用户点击现有失败重试入口后再次提交。
                        tab.route('**/api/learning-agent/events',fail_submission)
                        host.get_by_role('button',name='请老师解释当前结果').evaluate('node=>node.click()')
                        retry=tab.locator('button').filter(has_text='从失败步骤重试').first
                        retry.wait_for(state='attached')
                        tab.unroute('**/api/learning-agent/events',fail_submission)
                        assert len(failed_payloads)==1 and len(DeepFixture.calls)==before
                        retry.evaluate('node=>node.click()')
                    else:
                        host.get_by_role('button',name='请老师解释当前结果').click()
                    tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                    tab.wait_for_load_state('networkidle')
                    call=next(call for call in DeepFixture.calls[before:] if isinstance(call,dict) and media_id in call['event'].get('mediaState',{}))
                    observed=call['event']['mediaState'][media_id]
                    assert list(call['event']['mediaState'])==[media_id]
                    assert call['event']['contentId']==prepared['content']['lecture'][1]['id']
                    assert media_id in [item['id'] for item in call['currentDeepPage']['media']]
                    if media_id=='cell-lab':
                        assert call['event']['requestId']==failed_payloads[0]['requestId']
                        assert call['event']['contentId']==failed_payloads[0]['contentId']
                        assert set(failed_payloads[0]['mediaState'])=={media_id}
                    assert observed['resultSource']=='program_calculation' and observed['scoringEligible'] is False
                    if media_id=='cell-lab': assert observed['part'][0]=='叶绿体'
                    elif media_id=='history-lab': assert observed['chronological'] is True and observed['dates']==['1969-07-16','1969-07-20','1969-07-24']
                    elif media_id=='climate-lab': assert observed['temperatureDifferenceC']==24 and observed['dataKind']=='illustrative_not_observed'
                    elif media_id=='annotation-lab': assert observed['annotations']==[{'index':0,'text':'雨停了。','label':'疑问'}]
                    elif media_id=='sentence-lab': assert observed['annotations'][0]['label']=='主语'
                    else: assert observed['array']==[2,5,4,1] and observed['step']==1
                    assert api('/api/deep-course?'+urlencode({'lessonId':lesson_id}))['lesson']['evaluations']==initial_evaluations
                    wait(api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['teachingTaskId'])
                    fresh_component(media_id, lambda recovered_host: assert_recovered_result(recovered_host, explored))
                    tab.reload();tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                    host=tab.locator(f'[data-media-id={media_id}]')
                    assert host.locator('.il-lab-result').inner_text()==explored
                    host.get_by_role('button',name='重置探索').click()
                    assert host.locator('.il-lab-result').inner_text()!=explored
                # 卸载后不保留JSXGraph画板；切换回讲义仍能恢复草稿。
                fixture.open_learning_menu(tab).locator('#dc-tab-slides').click()
                fixture.open_learning_menu(tab).locator('#dc-slide-next').click()
                tab.keyboard.press('Escape')
                assert tab.evaluate('Object.keys(JXG.boards).length') == 0
                fixture.open_learning_menu(tab).locator('#dc-tab-lecture').click()
                tab.wait_for_function("document.querySelector('[data-media-id=function-lab] [data-plot-ready=true]')")
                # 直接体验入口不依赖模型，四种组件可立即操作；仅保存本地示例草稿。
                preview = context.new_page()
                preview.on('pageerror', lambda error: errors.append(str(error)))
                model_calls_before = len(DeepFixture.calls)
                preview.goto(info['base'] + '/interactive-labs.html')
                preview.wait_for_function("document.querySelector('[data-plot-ready=true]')")
                from interactive_subjects_browser_checks import verify_subjects
                verify_subjects(preview)
                for kind in ['function','circuit','chemical','sets','cell','history','climate','annotation','sentence','algorithm']:
                    preview.locator(f'[data-lab={kind}]').click()
                    if kind == 'function': preview.wait_for_function("document.querySelector('[data-plot-ready=true]')")
                    else: assert preview.evaluate('Object.keys(JXG.boards).length') == 0
                    for width,height in [(1280,720),(1440,900),(390,844)]:
                        preview.set_viewport_size({'width':width,'height':height})
                        assert preview.evaluate('document.documentElement.scrollWidth <= innerWidth + 1')
                        preview.screenshot(path=str(output/f'gallery-{kind}-{width}.png'),full_page=True)
                assert len(DeepFixture.calls) == model_calls_before
                preview.close()
            if DeepFixture.include_intents:
                before_evaluations = answered_lesson['evaluations']
                for intent, field, value in [('simplify', 'explanation', 'simple'), ('slow_down', 'pace', 'slow'),
                                              ('speed_up', 'pace', 'fast'), ('give_example', 'pace', 'fast'),
                                              ('independent_attempt', 'guidance', 'independent'),
                                              ('resume_guidance', 'guidance', 'guided'), ('reset_preferences', 'pace', 'normal')]:
                    previous = api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['teachingPreferences']['version']
                    calls_before = len(DeepFixture.calls)
                    if intent == 'resume_guidance':
                        # 自己尝试后的主按钮必须恢复讲解，而非误触完成判定。
                        fixture.open_learning_menu(tab).locator('#dc-continue').click()
                    else:
                        task = api('/api/learning-agent/events', {'type': 'teaching_request', 'requestType': intent,
                            'lessonId': lesson_id, 'courseId': course_id, 'teachingPreferenceVersion': previous,
                            'requestId': f'深度课堂偏好-{intent}', 'text': '请按我指定的方式讲解。'})
                        wait(task['id'])
                        tab.reload()
                    tab.wait_for_function("([field,value]) => document.querySelector('#dc-teaching-preferences')?.textContent && document.getElementById('deep-course-app').dataset.ready === 'true'", arg=[field,value])
                    # 等待服务器已保存的新版本，而非仅依赖按钮解除禁用。
                    deadline = time.monotonic() + 20
                    while time.monotonic() < deadline:
                        current = api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']
                        if current['teachingPreferences']['version'] == previous + 1:
                            break
                        time.sleep(.05)
                    assert current['teachingPreferences']['version'] == previous + 1
                    assert current['teachingPreferences'][field] == value
                    task_state = wait(current['teachingTaskId'])
                    assert current['evaluations'] == before_evaluations
                    if intent == 'independent_attempt':
                        assert len(DeepFixture.calls) == calls_before and task_state['calls'] == 0
                    else:
                        call = next(call for call in reversed(DeepFixture.calls) if isinstance(call, dict) and call['event'].get('requestType') == intent)
                        assert call['teachingPreferences'][field] == value
                    tab.reload()
                    tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                    assert api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['teachingPreferences'][field] == value
                assert '默认方式' in tab.locator('#dc-teaching-preferences').text_content()
            if DeepFixture.include_mastery:
                before_evaluations = answered_lesson['evaluations']
                state_task = api('/api/learning-agent/events', {'type': 'chat', 'courseId': course_id,
                                 'lessonId': lesson_id, 'requestId': 'mastery-browser', 'text': '请记录本课理解状态'})
                wait(state_task['id'])
                assert api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['evaluations'] == before_evaluations
                tab.reload()
                tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                assert tab.locator('#dc-mastery-summary').inner_text() == '当前理解：部分理解'
                fixture.open_learning_menu(tab, '本课说明与教学过程')
                tab.locator('#dc-mastery-summary').click()
                assert '只有一道题' in tab.locator('#dc-mastery-reason').inner_text()
                assert tab.locator('#dc-mastery-reason img').count() == 0
                assert tab.locator('#dc-mastery-next').inner_text() == '下一步建议：再练一道'
                for width, height in [(1280, 720), (1440, 900), (390, 844)]:
                    tab.set_viewport_size({'width': width, 'height': height})
                    panel = tab.locator('#dc-mastery').bounding_box()
                    assert panel and panel['height'] <= min(200, height * .25) + 2, panel
                    assert tab.evaluate('document.documentElement.scrollWidth <= innerWidth + 1')
                    tab.screenshot(path=str(output / f'mastery-expanded-{width}.png'), full_page=True)
                tab.set_viewport_size({'width': 1280, 'height': 720})
                tab.locator('#dc-mastery-summary').click()
                tab.keyboard.press('Escape')
                # 重新投递同一终态回调，模拟恢复订阅与晚到状态交叠；不生成模型请求。
                tab.wait_for_load_state('networkidle')
                focus_node = tab.locator('[data-media-id=set-lab] input').first
                focus_node.focus()
                tab.evaluate("""() => {
                    window.__readingFocus = document.activeElement;
                    window.__readingNode = document.querySelector('[data-media-id=set-lab]');
                    window.__readingRefreshSeen = false;
                    const original = window.fetch;
                    window.fetch = async (...args) => {
                        const response = await original(...args);
                        if (String(args[0]).startsWith('/api/deep-course?')) {
                            const copy = response.clone();
                            await copy.json();
                            requestAnimationFrame(() => requestAnimationFrame(() => window.__readingRefreshSeen = true));
                        }
                        return response;
                    };
                    window.__masteryWatch.handlers.onState(window.__masteryWatch.task);
                }""")
                tab.wait_for_function('window.__readingRefreshSeen === true')
                assert tab.evaluate("window.__readingNode === document.querySelector('[data-media-id=set-lab]')"), '相同内容刷新替换了交互节点'
                assert tab.evaluate('window.__readingFocus === document.activeElement'), '相同内容刷新丢失键盘焦点'
            # 【逐行批注】 计算右侧结果并赋给 before_continue，更新脚本状态。
            before_continue = len(DeepFixture.calls)
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-continue').click()。
            fixture.open_learning_menu(tab).locator('#dc-continue').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'", timeout=90000)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert any(call['event']['type'] == 'lesson_finish_request' for call in DeepFix…。
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                if any(call.get('event', {}).get('type') == 'lesson_finish_request'
                       for call in DeepFixture.calls[before_continue:] if isinstance(call, dict)):
                    break
                time.sleep(.1)
            else:
                raise AssertionError('未收到完成课时的教学请求')
            finish_task = api('/api/deep-course?' + urlencode({'lessonId': lesson_id}))['lesson']['teachingTaskId']
            wait(finish_task)
            tab.wait_for_function("document.querySelector('#dc-continue')?.innerText.trim() === '继续学习'", timeout=30000)
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
            if DeepFixture.include_sets:
                lesson_query = urlencode({'lessonId': lesson_id})
                draft_url = '/api/deep-course/component-draft'
                ready = "document.getElementById('deep-course-app').dataset.ready === 'true'"
                initial_draft = api('/api/deep-course?' + lesson_query)['componentDrafts']['set-lab']
                initial_members = set(initial_draft['state']['setB'])
                with browser.new_context() as shared_profile:
                    writer_tab, stale_tab = shared_profile.new_page(), shared_profile.new_page()
                    for page in (writer_tab, stale_tab):
                        page.on('pageerror', lambda error: errors.append(str(error)))
                        page.goto(info['base'] + '/deep-course?' + lesson_query)
                        page.wait_for_function(ready)
                    writer_check = writer_tab.locator('[data-media-id=set-lab]').get_by_role('checkbox', name='1 属于集合 B', exact=True)
                    stale_check = stale_tab.locator('[data-media-id=set-lab]').get_by_role('checkbox', name='4 属于集合 B', exact=True)
                    writer_selected = '1' not in initial_members
                    stale_selected = '4' not in initial_members
                    assert writer_check.is_checked() != writer_selected
                    assert stale_check.is_checked() != stale_selected
                    with writer_tab.expect_response(lambda res: res.url.endswith(draft_url)) as accepted:
                        writer_check.set_checked(writer_selected)
                    assert accepted.value.ok, accepted.value.text()
                    accepted_revision = accepted.value.json()['revision']
                    with stale_tab.expect_response(lambda res: res.url.endswith(draft_url)) as conflict:
                        stale_check.set_checked(stale_selected)
                    assert conflict.value.status == 409, conflict.value.text()
                    stale_tab.reload()
                    stale_tab.wait_for_function(ready)
                    assert stale_check.is_checked() == stale_selected
                    writer_tab.reload()
                    writer_tab.wait_for_function(ready)
                    assert writer_check.is_checked() == writer_selected
                    saved_draft = api('/api/deep-course?' + lesson_query)['componentDrafts']['set-lab']
                    assert set(saved_draft['state']['setB']) == initial_members ^ {'1'}
                    assert saved_draft['revision'] == accepted_revision
                    stale_tab.locator('[data-media-id=set-lab]').screenshot(path=str(output / 'sets-local-conflicting-draft.png'))
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
                if DeepFixture.include_sets:
                    lab = tab.locator('[data-media-id=set-lab]')
                    lab.scroll_into_view_if_needed()
                    lab.screenshot(path=str(output / f'sets-{width}.png'))
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-directory-toggle').click()。
            fixture.open_learning_menu(tab).locator('#dc-directory-toggle').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-outline-dialog').is_visible()。
            assert tab.locator('#dc-outline-dialog').is_visible()
            # 【逐行批注】 执行当前 Python 语句：tab.keyboard.press('Escape')。
            tab.keyboard.press('Escape')
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-toggle').click()。
            fixture.open_learning_menu(tab).locator('#dc-chat-toggle').click()
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
            if DeepFixture.include_mastery:
                assert not tab.locator('#dc-mastery').is_visible()
            if DeepFixture.include_intents:
                # 同一服务的轻量课堂也使用同一课时偏好，不能只验证深度课堂。
                quick_id = info['lessonId']
                tab.goto(info['base'] + '/learn?' + urlencode({'lessonId': quick_id}))
                tab.wait_for_function("document.querySelector('#lesson-input') && !document.querySelector('#lesson-input').disabled")
                quick = api('/api/learning-agent/lesson?' + urlencode({'lessonId': quick_id}))
                if quick.get('teachingTaskId'):
                    wait(quick['teachingTaskId'])
                quick_evaluations = quick['evaluations']
                for intent in ['simplify', 'independent_attempt', 'reset_preferences']:
                    quick = api('/api/learning-agent/lesson?' + urlencode({'lessonId': quick_id}))
                    previous = quick['teachingPreferences']['version']
                    calls_before = len(DeepFixture.calls)
                    fixture.open_learning_menu(tab, '课堂操作')
                    tab.locator(f'[data-teaching-preference={intent}]').click()
                    deadline = time.monotonic() + 20
                    while time.monotonic() < deadline:
                        quick = api('/api/learning-agent/lesson?' + urlencode({'lessonId': quick_id}))
                        if quick['teachingPreferences']['version'] == previous + 1:
                            break
                        time.sleep(.05)
                    assert quick['teachingPreferences']['version'] == previous + 1
                    quick_task = wait(quick['teachingTaskId'])
                    assert quick['evaluations'] == quick_evaluations
                    if intent == 'independent_attempt':
                        assert quick_task['calls'] == 0 and len(DeepFixture.calls) == calls_before
                        tab.reload()
                        tab.wait_for_function("document.querySelector('#classroom-teaching-preferences')?.textContent.includes('自己尝试')")
                        assert '老师会等你' not in tab.locator('#lesson-scroll').inner_text()
                        for width, height in [(1280, 720), (1440, 900), (390, 844)]:
                            tab.set_viewport_size({'width': width, 'height': height})
                            assert tab.evaluate('document.documentElement.scrollWidth <= innerWidth + 1')
                            tab.screenshot(path=str(output / f'quick-preferences-{width}.png'), full_page=True)
            if DeepFixture.include_topic_evidence:
                source_url = '/api/deep-course?' + urlencode({'lessonId': lesson_id})
                source = api(source_url)['lesson']
                prior = source['topicEvidence']
                assert prior['lessonCount'] == 1 and prior['evaluationCount'] == 1
                task = api('/api/learning-agent/events', {'type':'prepare_next', 'courseId':course_id, 'topicId':source['topicId'],
                           'requestId':'deep-linked-review', 'text':'请准备同一知识点的复习课'})
                review_id = wait(task['id'])['lesson']['id']
                assert review_id != lesson_id
                review_url = '/deep-course?' + urlencode({'lessonId': review_id})
                tab.goto(info['base'] + review_url)
                fixture.open_learning_menu(tab, '本课说明与教学过程')
                evidence = tab.locator('#dc-topic-evidence')
                summary = evidence.get_by_text('本知识点的历史作答', exact=True)
                summary.wait_for()
                summary.press('Space')
                assert '其他课时' in evidence.inner_text()
                tab.keyboard.press('Escape')
                question = tab.locator('.dc-question form').first
                question.locator('textarea').fill('我还是比较计算出来的函数值')
                question.locator('button[type=submit]').click()
                tab.wait_for_function("document.querySelector('#dc-topic-evidence')?.textContent.includes('同题复做')")
                reviewed = api('/api' + review_url)['lesson']
                evidence_data = reviewed['topicEvidence']
                assert evidence_data['lessonCount'] == evidence_data['evaluationCount'] == 2
                assert evidence_data['distinctSupportedQuestionCount'] == 1
                assert evidence_data['recentAttempts'][-1]['repeat'] is True
                assert evidence_data['recentAttempts'][-1]['independentFirstAttempt'] is False
                for width, height in [(1280, 720), (1440, 900), (390, 844)]:
                    tab.set_viewport_size({'width':width,'height':height})
                    assert tab.evaluate('document.documentElement.scrollWidth <= innerWidth + 1')
                    fixture.open_learning_menu(tab, '本课说明与教学过程')
                    evidence.screenshot(path=str(output / f'topic-evidence-{width}.png'))
                    tab.keyboard.press('Escape')
                assert not errors, errors
            if DeepFixture.include_rounds:
                lesson_url = '/api/deep-course?' + urlencode({'lessonId': lesson_id})
                tab.goto(info['base'] + '/deep-course?' + urlencode({'lessonId': lesson_id}))
                tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
                composer = tab.locator('#dc-course-reply-input')
                prior_task_id = api(lesson_url)['lesson']['teachingTaskId']
                composer.fill('带我做一轮集合关系练习')
                tab.locator('#dc-course-reply-send').click()
                tab.wait_for_function("!document.querySelector('#dc-course-reply-input').value")
                round_task_id = api(lesson_url)['lesson']['teachingTaskId']
                assert round_task_id != prior_task_id
                wait(round_task_id)
                round_node = tab.locator('#dc-teaching-round')
                tab.wait_for_function("document.querySelector('#dc-teaching-round')?.textContent.includes('0/2')")
                current_question = tab.locator('.dc-question form:has(button[type=submit]:visible)')
                current_question.locator('button[type=submit]').wait_for()
                assert current_question.count() == 1
                tab.reload()
                current_question.locator('textarea').fill('我把子集和交集搞混了')
                current_question.locator('button[type=submit]').click()
                tab.wait_for_function("document.querySelector('#dc-teaching-round')?.textContent.includes('1/2')")
                during_feedback = api(lesson_url)['lesson']
                first_answer_id = during_feedback['teachingTaskId']
                assert during_feedback['teachingRound']['questionNumber'] == 1
                assert '这道题不应在反馈前保存。' not in json.dumps(during_feedback['sections'], ensure_ascii=False)
                paused = api('/api/learning-agent/control', {'command': 'pause', 'taskId': first_answer_id})
                assert paused['status'] == 'paused'
                tab.reload()
                round_node.wait_for()
                assert '1/2' in round_node.inner_text() and current_question.count() == 0
                DeepFixture.release_round_question.set()
                api('/api/learning-agent/control', {'command': 'resume', 'taskId': first_answer_id})
                wait(first_answer_id)
                tab.reload()
                tab.get_by_text('A={1}、B={1,2}，A 与 B 的交集是什么？', exact=True).wait_for()
                assert current_question.count() == 1
                assert tab.get_by_text('子集检查 A 的每个元素是否都在 B 中；交集给出共有元素。', exact=True).is_visible()
                for width, height in [(1280, 720), (1440, 900), (390, 844)]:
                    tab.set_viewport_size({'width': width, 'height': height})
                    assert tab.evaluate('document.documentElement.scrollWidth <= innerWidth + 1')
                    tab.screenshot(path=str(output / f'teaching-round-{width}.png'), full_page=True)
                current_question.locator('textarea').fill('{1}')
                current_question.locator('button[type=submit]').click()
                tab.wait_for_function("document.querySelector('#dc-teaching-round')?.textContent.includes('2/2')")
                completed = wait(api(lesson_url)['lesson']['teachingTaskId'])
                round_state = api(lesson_url)['lesson']['teachingRound']
                assert completed['status'] == 'waiting_student'
                assert round_state['status'] == 'completed' and round_state['feedbackCount'] == round_state['questionCount'] == 2
                assert DeepFixture.saw_feedback_stage_prompt and DeepFixture.saw_early_finish_error
                tab.reload()
                round_node.wait_for()
                assert '2/2' in round_node.inner_text() and current_question.count() == 0
                composer.fill('再开始一轮集合关系练习')
                tab.locator('#dc-course-reply-send').click()
                tab.get_by_text('若 C={2}、D={2,3}，C 是 D 的子集吗？', exact=True).wait_for()
                current_question.locator('textarea').fill('我仍把子集和交集搞混了')
                current_question.locator('button[type=submit]').click()
                tab.wait_for_function("document.querySelector('#dc-teaching-round')?.textContent.includes('1/2')")
                before_revision = api(lesson_url)['lesson']
                wait(before_revision['teachingTaskId'])
                before_revision = api(lesson_url)['lesson']
                fixture.open_learning_menu(tab, '课堂操作')
                tab.get_by_role('button', name='调整本轮').click()
                dialog = tab.get_by_role('dialog', name='调整本轮')
                assert dialog.get_by_label('学习目标').input_value() == '区分子集和交集'
                dialog.get_by_label('学习目标').fill('理解相对于给定全集的补集')
                dialog.get_by_label('题数').fill('3')
                tab.screenshot(path=str(output / 'revise-teaching-round-390.png'), full_page=True)
                blocked_submissions = []
                def fail_revision(route):
                    blocked_submissions.append(route.request.post_data_json)
                    route.abort('failed')
                tab.route('**/api/learning-agent/events', fail_revision)
                dialog.get_by_role('button', name='保存并开始新轮次').click()
                dialog.get_by_role('alert').wait_for()
                assert dialog.get_by_label('学习目标').input_value() == '理解相对于给定全集的补集'
                assert dialog.get_by_label('题数').input_value() == '3'
                assert api(lesson_url)['lesson']['teachingRound'] == before_revision['teachingRound']
                tab.unroute('**/api/learning-agent/events', fail_revision)
                dialog.get_by_role('button', name='保存并开始新轮次').click()
                dialog.wait_for(state='detached')
                tab.get_by_text('全集 U={1,2,3}，A={1} 的补集是什么？', exact=True).wait_for()
                revised = api(lesson_url)['lesson']
                previous = revised['teachingRoundHistory'][-1]
                assert previous['roundId'] == before_revision['teachingRound']['roundId']
                assert previous['status'] == 'revised' and previous['questionCount'] == 2 and previous['feedbackCount'] == 1
                assert revised['teachingRound']['supersedesRoundId'] == previous['roundId']
                assert revised['teachingRound']['questionCount'] == 3 and revised['teachingRound']['feedbackCount'] == 0
                assert revised['evaluations'] == before_revision['evaluations']
                assert current_question.count() == 1 and len(blocked_submissions) == 1
                try:
                    api('/api/learning-agent/events', dict(blocked_submissions[0], requestId='stale-round-revision'))
                except HTTPError as error:
                    assert error.code == 409 and '当前教学轮次已变化' in error.read().decode()
                else:
                    raise AssertionError('旧轮次的调整请求未被拒绝')
                tab.reload()
                round_node.wait_for()
                assert '0/3' in round_node.inner_text() and current_question.count() == 1
                assert not errors, errors
            browser.close()
        # 保存真实进入模拟模型请求的活动序列，便于复核每步变化而非只看最终参数。
        function_activities = {}
        if DeepFixture.include_labs:
            for call in DeepFixture.calls:
                if not isinstance(call, dict):
                    continue
                for actual_input in call.get('currentInputs', []):
                    activity = actual_input.get('componentActivities', {}).get('function-lab')
                    if activity:
                        function_activities[activity['traceId']] = {
                            'traceId': activity['traceId'], 'interactionId': actual_input['interactionId'],
                            'contentVersion': activity['contentVersion'], 'scoringEligible': activity['scoringEligible'],
                            'events': [{'seq': event['seq'], 'kind': event['kind'],
                                        'coefficients': event['observation']['coefficients'],
                                        'functionAnalysis': event['observation']['functionAnalysis']}
                                       for event in activity['recentEvents']]}
        # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
        report = {'realCppService': True, 'mockAIOnly': True, 'courseId': course_id, 'lessonId': lesson_id,
                  # 【逐行批注】 调用 len 完成当前脚本或测试步骤：'views': views, 'pageErrors': errors, 'modelHttpRequests': len(DeepFi…。
                  'views': views, 'pageErrors': errors, 'modelHttpRequests': len(DeepFixture.calls),
                  # 【逐行批注】 调用 isinstance 完成当前脚本或测试步骤：'deepEvents': [call['event'] for call in DeepFixture.calls if isinsta…。
                  'deepEvents': [call['event'] for call in DeepFixture.calls if isinstance(call, dict) and 'event' in call],
                  # 【逐行批注】 调用 isinstance 完成当前脚本或测试步骤：'currentPageContexts': [call['currentDeepPage'] for call in DeepFixtu…。
                  'currentPageContexts': [call['currentDeepPage'] for call in DeepFixture.calls if isinstance(call, dict) and call.get('currentDeepPage')],
                  # 【逐行批注】 执行当前 Python 语句：'cachedEntryWithoutGeneration': True, 'evaluatedContinueReachesController': True, 'passed…。
                  'cachedEntryWithoutGeneration': True, 'evaluatedContinueReachesController': True,
                  'independentChatIsolationVerified': True, 'setsExplorationVerified': DeepFixture.include_sets,
                  'componentDraftConflictVerified': DeepFixture.include_sets,
                  'learningMasteryVerified': DeepFixture.include_mastery, 'teachingIntentsVerified': DeepFixture.include_intents,
                  'quickTeachingIntentsVerified': DeepFixture.include_intents, 'interactiveLabsVerified': DeepFixture.include_labs,
                  'functionDomainActivityVerified': DeepFixture.include_labs, 'paidModelRequests':0,
                  'functionActivities':list(function_activities.values()),
                  'teachingRoundVerified': DeepFixture.include_rounds, 'topicEvidenceVerified': DeepFixture.include_topic_evidence,
                  'passed': True}
        # 【逐行批注】 计算右侧结果并赋给 (output / 'report.json').write_text(json.dum…，更新脚本状态。
        (output / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
        # 【逐行批注】 输出当前运行结果：print(json.dumps({'passed': True, 'realCppService': True, 'mockAIOnly': Tr…。
        print(json.dumps({'passed': True, 'realCppService': True, 'mockAIOnly': True, 'views': len(views), 'pageErrors': errors}, ensure_ascii=False))


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    DeepFixture.include_rounds = '--teaching-round' in sys.argv[3:]
    DeepFixture.include_topic_evidence = '--topic-evidence' in sys.argv[3:]
    # 【逐行批注】 调用 main、Path、len 完成当前脚本或测试步骤：main(sys.argv[1], Path(sys.argv[2] if len(sys.argv) > 2 else 'verific…。
    main(sys.argv[1], Path(sys.argv[2] if len(sys.argv) > 2 else 'verification/deep-course-integration').resolve())

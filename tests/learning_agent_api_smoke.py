# 【逐行批注】 执行当前 Python 语句："""统一主控的隔离接口验收；模型协议夹具只验证真实 HTTP 调用与工具执行。"""。
"""统一主控的隔离接口验收；模型协议夹具只验证真实 HTTP 调用与工具执行。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：import socket。
import socket
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sqlite3。
import sqlite3
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextlib import closing。
from contextlib import closing
from datetime import date
# 【逐行批注】 导入后续逻辑需要的模块或符号：from concurrent.futures import ThreadPoolExecutor。
from concurrent.futures import ThreadPoolExecutor
# 【逐行批注】 导入后续逻辑需要的模块或符号：from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer。
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from threading import Thread。
from threading import Thread
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.error import HTTPError。
from urllib.error import HTTPError
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import quote。
from urllib.parse import quote
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen


# 【逐行批注】 定义函数 port，封装可复用的处理步骤。
def port():
    # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as connection:。
    with socket.socket() as connection:
        # 【逐行批注】 执行当前 Python 语句：connection.bind(('127.0.0.1', 0))。
        connection.bind(('127.0.0.1', 0))
        # 【逐行批注】 结束当前函数并返回结果：return connection.getsockname()[1]。
        return connection.getsockname()[1]


# 【逐行批注】 定义类 ProtocolAI，组织相关数据与行为。
class ProtocolAI(BaseHTTPRequestHandler):
    # 【逐行批注】 计算右侧结果并赋给 calls，更新脚本状态。
    calls = []
    # 【逐行批注】 计算右侧结果并赋给 repaired，更新脚本状态。
    repaired = set()

    # 【逐行批注】 定义函数 log_message，封装可复用的处理步骤。
    def log_message(self, *_):
        # 【逐行批注】 执行当前 Python 语句：pass。
        pass

    # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
    def do_POST(self):
        # 【逐行批注】 计算右侧结果并赋给 request，更新脚本状态。
        request = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert request.get('response_format') == {'type': 'json_object'}, '主控应请求有效的 JSO…。
        assert request.get('response_format') == {'type': 'json_object'}, '主控应请求有效的 JSON 对象输出'
        # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
        context = json.loads(request['messages'][1]['content'])
        # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
        event = context['event']
        expected_guide = '课程大纲规划助手' if event['type'] == 'plan_course' else '教学主控'
        assert expected_guide in request['messages'][0]['content'], '请求应使用本任务对应的协议'
        type(self).calls.append(event.get('requestId', event['type']))
        # 【逐行批注】 计算右侧结果并赋给 results，更新脚本状态。
        results = []
        # 【逐行批注】 遍历数据并重复执行循环体：for item in request['messages'][2:]:。
        for item in request['messages'][2:]:
            # 【逐行批注】 判断条件 item['role'] == 'user':，成立时执行对应分支。
            if item['role'] == 'user':
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 执行当前 Python 语句：results.extend(json.loads(item['content']).get('toolResults', []))。
                    results.extend(json.loads(item['content']).get('toolResults', []))
                # 【逐行批注】 捕获并处理异常：except ValueError:。
                except ValueError:
                    # 【逐行批注】 执行当前 Python 语句：pass。
                    pass
        # 【逐行批注】 计算右侧结果并赋给 actions, state, message，更新脚本状态。
        actions, state, message = [], 'completed', '模型依据已保存的真实记录完成本次处理。'
        # 【逐行批注】 判断条件 event['type'] == 'startup':，成立时执行对应分支。
        if event['type'] == 'startup':
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [dict(id='recommend', tool='update_recommendations', args=dict(
                # 【逐行批注】 计算右侧结果并赋给 lite，更新脚本状态。
                lite=['认识化合价', '理解函数'], deep=['氧化还原', '函数概念'], reason='无作答时仅提出学习方向'))]
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'plan_course':。
        elif event['type'] == 'plan_course':
            outline = dict(title='自主两主题课程', summary='按学生实际反馈调整',
                           courseStructure=[dict(stage='概念', topics=['化合价', '氧化还原'])])
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'prepare_next':。
        elif event['type'] == 'prepare_next':
            # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
            lesson = next((item['result']['lessonId'] for item in results if 'lessonId' in item['result']), None)
            # 【逐行批注】 判断条件 not lesson:，成立时执行对应分支。
            if not lesson:
                review_args = {}
                if review := context.get('scheduledReview'):
                    assert review['available'] and review['courseId'] == event['courseId']
                    assert review['basisStatus'] in ('verified', 'changed')
                    assert bool(review['evidence']) == (review['basisStatus'] == 'verified')
                    review_args = {'intent': 'review'}
                    if review['topicId']:
                        review_args['topicId'] = review['topicId']
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='create', tool='create_lesson', args=dict(courseId=event['courseId'], title='自主课堂',
                    purpose='对比概念', **review_args))]
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'continue'
            # 【逐行批注】 当前一条件不成立时继续判断：not any('sectionId' in item['result'] for item in results):。
            elif not any('sectionId' in item['result'] for item in results):
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='question-' + kind, tool='append_section', args=dict(lessonId=lesson, questionKind=kind,
                    # 【逐行批注】 计算右侧结果并赋给 section，更新脚本状态。
                    section=dict(kind='question', **({'questionKind':kind} if kind != 'interaction' else {}), title=kind + '题', question=dict(
                        # 【逐行批注】 计算右侧结果并赋给 question，更新脚本状态。
                        question=kind + '：铁变为亚铁离子时化合价如何变化？', type='choice', options=['升高', '降低'],
                        # 【逐行批注】 计算右侧结果并赋给 expectedAnswer，更新脚本状态。
                        expectedAnswer='PRIVATE-EXPECTED', rubric='PRIVATE-RUBRIC'))))
                    # 【逐行批注】 遍历数据并重复执行循环体：for kind in ('diagnostic', 'interaction', 'practice', 'quiz', 'review')]。
                    for kind in ('diagnostic', 'interaction', 'practice', 'quiz', 'review')]
                # 【逐行批注】 计算右侧结果并赋给 actions.append(dict(id，更新脚本状态。
                actions.append(dict(id='example', tool='append_section', args=dict(lessonId=lesson, section=dict(kind='explanation',presentation='example',title='示范讲解',body='先对比前后的化合价，再解释变化。'))))
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'continue'
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='finish', tool='finish_lesson', args=dict(lessonId=lesson))]
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'waiting_student'
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'lesson_start':。
        elif event['type'] == 'lesson_start':
            # 【逐行批注】 计算右侧结果并赋给 saved，更新脚本状态。
            saved = next((item['result'] for item in results if 'sections' in item['result']), None)
            # 【逐行批注】 判断条件 saved is None:，成立时执行对应分支。
            if saved is None:
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='read-start', tool='read_lesson', args=dict(lessonId=event['lessonId']))]
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'continue'
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 explanation，更新脚本状态。
                explanation = next(section for section in saved['sections'] if section['kind'] == 'explanation')
                # 【逐行批注】 计算右侧结果并赋给 interaction，更新脚本状态。
                interaction = next(section for section in saved['sections'] if section.get('questionKind') == 'interaction')
                # 【逐行批注】 计算右侧结果并赋给 practice，更新脚本状态。
                practice = next(section for section in saved['sections'] if section.get('questionKind') == 'practice')
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='start-focus', tool='select_teaching_focus', args=dict(lessonId=event['lessonId'],sectionId=explanation['id'],interactionSectionId=interaction['id'],practiceSectionId=practice['id']))]
                # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
                message = '先比较当前段，再回答一个相关问题。'
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'waiting_student'
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'course_preview':。
        elif event['type'] == 'course_preview':
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [dict(id='preview', tool='update_course_preview', args=dict(courseId=event['courseId'],
                # 【逐行批注】 计算右侧结果并赋给 slides，更新脚本状态。
                slides=[dict(title='概念路线', content='先比较变化'), dict(title='巩固路线', content='根据实际回答调整')], reason='模型自主选择两页路线说明'))]
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'question_answer':。
        elif event['type'] == 'lesson_finish_request':
            assert context.get('lessonContext', {}).get('scheduledReviewId')
            if any('每道复习题须先发布有效评价' in item['content'] for item in request['messages'][2:]):
                message, state = '请完成尚未发布评价的复习题。', 'waiting_student'
            else:
                actions = [dict(id='complete-review', tool='complete_lesson',
                                args=dict(lessonId=event['lessonId'], reason='根据本次复习作答和正式评价检查'))]
        elif event['type'] == 'question_answer':
            if event['requestId'] == 'review-ungraded-revision':
                self.send_response(401)
                self.end_headers()
                return
            # 【逐行批注】 计算右侧结果并赋给 inputs，更新脚本状态。
            inputs = event.get('batchAnswers', [event])
            # 【逐行批注】 遍历数据并重复执行循环体：for index, item in enumerate(inputs):。
            for index, item in enumerate(inputs):
                # 【逐行批注】 判断条件 item.get('action') in ('hint', 'skip', 'omitted'):，成立时执行对应分支。
                if item.get('action') in ('hint', 'skip', 'omitted'):
                    # 【逐行批注】 执行当前 Python 语句：continue。
                    continue
                # 【逐行批注】 计算右侧结果并赋给 actual_input，更新脚本状态。
                actual_input = next(value for value in context['currentInputs'] if value['requestId'] == item['requestId'])
                # 【逐行批注】 计算右侧结果并赋给 actual，更新脚本状态。
                actual = {'id': actual_input['interactionId'], 'payload': actual_input}
                # 【逐行批注】 判断条件 item.get('text') == '为什么这样判断？':，成立时执行对应分支。
                if item.get('text') == '为什么这样判断？':
                    # 【逐行批注】 计算右侧结果并赋给 actions.append(dict(id，更新脚本状态。
                    actions.append(dict(id='classify-' + str(index), tool='classify_input', args=dict(
                        # 【逐行批注】 计算右侧结果并赋给 interactionId，更新脚本状态。
                        interactionId=actual['id'], isAnswer=False, reason='这是对已给出反馈的追问')))
                # 【逐行批注】 执行前一条件不成立时的分支。
                else:
                    # 【逐行批注】 计算右侧结果并赋给 actions.append(dict(id，更新脚本状态。
                    actions.append(dict(id='evaluate-' + str(index), tool='evaluate_answer', args=dict(interactionId=actual['id'],
                        # 【逐行批注】 计算右侧结果并赋给 score，更新脚本状态。
                        score=70, credible=True, feedback='依据实际作答给出评价', feedbackHelpType='none', reason='学生指出化合价升高', uncertainty='仅反映本题表现',
                        # 【逐行批注】 计算右侧结果并赋给 diagnosis，更新脚本状态。
                        diagnosis=dict(summary='需要补充电子变化的联系', evidence=[actual['payload']['text']], teachingDirection='联系失电子与化合价升高', needsClarification=False))))
            # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
            message = '先评价实际输入，再解释电子变化。'
            # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
            state = 'waiting_student'
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'acceptance_tool':。
        elif event['type'] == 'acceptance_tool':
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [dict(id='action', tool=event['tool'], args=event['args'])]
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'acceptance_fail':。
        elif event['type'] == 'acceptance_fail':
            # 【逐行批注】 执行当前 Python 语句：self.send_response(503)。
            self.send_response(503)
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 结束当前函数并返回结果：return。
            return
        # 【逐行批注】 遍历数据并重复执行循环体：for action in actions:。
        for action in actions:
            # 【逐行批注】 计算右侧结果并赋给 section，更新脚本状态。
            section = action.get('args', {}).get('section')
            # 【逐行批注】 判断条件 action.get('tool') == 'append_section' and isinstance(section, d…，成立时执行对应分支。
            if action.get('tool') == 'append_section' and isinstance(section, dict) and section.get('kind') != 'question':
                # 【逐行批注】 执行当前 Python 语句：section.setdefault('helpType', 'explanation')。
                section.setdefault('helpType', 'explanation')
        decision = outline if event['type'] == 'plan_course' else dict(
            teaching=dict(helpType='hint' if event.get('action') == 'hint' else 'explanation'),
            message=message, actions=actions, state=state)
        value = json.dumps(decision, ensure_ascii=False)
        # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
        self.send_response(200)
        # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'text/event-stream')。
        self.send_header('Content-Type', 'text/event-stream')
        # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
        self.end_headers()
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 遍历数据并重复执行循环体：for offset in range(0, len(value), 13):。
            for offset in range(0, len(value), 13):
                # 【逐行批注】 计算右侧结果并赋给 chunk，更新脚本状态。
                chunk = dict(model='fixture-agent-http', choices=[dict(delta=dict(content=value[offset:offset + 13]), finish_reason=None)])
                # 【逐行批注】 计算右侧结果并赋给 self.wfile.write(('data: ' + json.dumps(chun…，更新脚本状态。
                self.wfile.write(('data: ' + json.dumps(chunk, ensure_ascii=False) + '\n\n').encode())
                # 【逐行批注】 执行当前 Python 语句：self.wfile.flush()。
                self.wfile.flush()
            # 【逐行批注】 计算右侧结果并赋给 interrupted，更新脚本状态。
            interrupted = event.get('failureAfterEvaluation') and event['requestId'] not in type(self).repaired and any('evaluationId' in item.get('result', {}) for item in results)
            # 【逐行批注】 计算右侧结果并赋给 self.wfile.write(('data: ' + json.dumps(dict…，更新脚本状态。
            self.wfile.write(('data: ' + json.dumps(dict(model='fixture-agent-http', choices=[dict(delta={}, finish_reason='length' if interrupted else 'stop')])) + '\n\ndata: [DONE]\n\n').encode())
        # 【逐行批注】 捕获并处理异常：except (BrokenPipeError, ConnectionResetError):。
        except (BrokenPipeError, ConnectionResetError):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable):
    # 【逐行批注】 计算右侧结果并赋给 repo，更新脚本状态。
    repo = Path(__file__).resolve().parents[1]
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-agent-api-') as directory:。
    with tempfile.TemporaryDirectory(prefix='gangyi-agent-api-') as directory:
        # 【逐行批注】 计算右侧结果并赋给 database，更新脚本状态。
        database = Path(directory) / 'fictional.db'
        # 【逐行批注】 计算右侧结果并赋给 ai，更新脚本状态。
        ai = ThreadingHTTPServer(('127.0.0.1', port()), ProtocolAI)
        # 【逐行批注】 计算右侧结果并赋给 Thread(target，更新脚本状态。
        Thread(target=ai.serve_forever, daemon=True).start()
        # 【逐行批注】 计算右侧结果并赋给 base，更新脚本状态。
        base = f'http://127.0.0.1:{port()}'
        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
        env = dict(os.environ, DATABASE_PATH=str(database), PORT=base.rsplit(':', 1)[1], HOST='127.0.0.1',
            # 【逐行批注】 计算右侧结果并赋给 AI_BASE_URL，更新脚本状态。
            AI_BASE_URL=f'http://127.0.0.1:{ai.server_port}/v1', AI_API_KEY='fictional-key', AI_MODEL='fixture-agent-http',
            # 【逐行批注】 计算右侧结果并赋给 LOCAL_CONTROL_TOKEN，更新脚本状态。
            LOCAL_CONTROL_TOKEN='fictional-control', GANGYI_LAUNCH_SESSION_ID='fixture-api-startup')
        # 【逐行批注】 计算右侧结果并赋给 log，更新脚本状态。
        log = open(Path(directory) / 'service.log', 'w+', encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
        process = subprocess.Popen([str(Path(executable).resolve())], cwd=repo, env=env, stdout=log, stderr=log,
            # 【逐行批注】 计算右侧结果并赋给 creationflags，更新脚本状态。
            creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        # 【逐行批注】 计算右侧结果并赋给 serial，更新脚本状态。
        serial = 0
        # 【逐行批注】 定义函数 api，封装可复用的处理步骤。
        def api(path, value=None, status=200):
            # 【逐行批注】 计算右侧结果并赋给 request，更新脚本状态。
            request = Request(base + path, data=None if value is None else json.dumps(value).encode(), headers={'Content-Type': 'application/json'})
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(request, timeout=15) as response:。
                with urlopen(request, timeout=15) as response:
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert response.status == status, (path, response.status)。
                    assert response.status == status, (path, response.status)
                    # 【逐行批注】 结束当前函数并返回结果：return json.load(response)。
                    return json.load(response)
            # 【逐行批注】 捕获并处理异常：except HTTPError as error:。
            except HTTPError as error:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert error.code == status, (path, error.code, error.read().decode())。
                assert error.code == status, (path, error.code, error.read().decode())
                # 【逐行批注】 结束当前函数并返回结果：return json.load(error)。
                return json.load(error)
        # 【逐行批注】 定义函数 wait，封装可复用的处理步骤。
        def wait(task_id='', expected='ready'):
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(500):。
            for _ in range(500):
                # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
                value = api('/api/learning-agent' + ('?taskId=' + quote(task_id) if task_id else ''))
                # 【逐行批注】 判断条件 value['status'] not in ('idle', 'pending', 'running'):，成立时执行对应分支。
                if value['status'] not in ('idle', 'pending', 'running'):
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert value['status'] == expected, {key: value.get(key) for key in ('status', …。
                    assert value['status'] == expected, {key: value.get(key) for key in ('status', 'error', 'calls')}
                    # 【逐行批注】 结束当前函数并返回结果：return value。
                    return value
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.03)。
                time.sleep(.03)
            # 【逐行批注】 在错误情形抛出异常：raise AssertionError('主控任务未结束')。
            raise AssertionError(('主控任务未结束', {key: value.get(key) for key in
                                  ('status', 'error', 'failure', 'retry', 'activity')}))
        # 【逐行批注】 定义函数 submit，封装可复用的处理步骤。
        def submit(event, expected='ready'):
            # 【逐行批注】 执行当前 Python 语句：nonlocal serial。
            nonlocal serial
            # 【逐行批注】 计算右侧结果并赋给 serial +，更新脚本状态。
            serial += 1
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：event.setdefault('requestId', 'fixture-' + str(serial))。
            event.setdefault('requestId', 'fixture-' + str(serial))
            # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
            task = api('/api/learning-agent/events', event, 202)
            # 【逐行批注】 结束当前函数并返回结果：return wait(task['id'], expected)。
            return wait(task['id'], expected)
        # 【逐行批注】 定义函数 tool，封装可复用的处理步骤。
        def tool(name, args):
            # 【逐行批注】 结束当前函数并返回结果：return submit(dict(type='acceptance_tool', tool=name, args=args))。
            return submit(dict(type='acceptance_tool', tool=name, args=args))
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(200):。
            for _ in range(200):
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 调用 api 完成当前脚本或测试步骤：api('/health')。
                    api('/health')
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
                # 【逐行批注】 捕获并处理异常：except OSError:。
                except OSError:
                    # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                    time.sleep(.1)
            # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait()。
            wait()
            # 【逐行批注】 计算右侧结果并赋给 recommendation，更新脚本状态。
            recommendation = api('/api/home/recommendations')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert recommendation['items']['lite'] == ['认识化合价', '理解函数']。
            assert recommendation['items']['lite'] == ['认识化合价', '理解函数']
            # 【逐行批注】 计算右侧结果并赋给 calls，更新脚本状态。
            calls = len(ProtocolAI.calls)
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(3):。
            for _ in range(3):
                # 【逐行批注】 调用 api 完成当前脚本或测试步骤：api('/api/home/recommendations'); api('/api/profile'); api('/api/lear…。
                api('/api/home/recommendations'); api('/api/profile'); api('/api/learning-agent')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(ProtocolAI.calls) == calls。
            assert len(ProtocolAI.calls) == calls
            # 【逐行批注】 计算右侧结果并赋给 planned，更新脚本状态。
            planned = api('/api/generate-plan', dict(goal='化合价', mode='lite', requestId='plan'), 202)
            # 课程与任务在同一事务保存；并发轮询不能把新任务与旧访问列表混合。
            # 【逐行批注】 在受控上下文中执行资源操作：with ThreadPoolExecutor(max_workers=4) as readers:。
            with ThreadPoolExecutor(max_workers=4) as readers:
                # 【逐行批注】 计算右侧结果并赋给 polls，更新脚本状态。
                polls = [readers.submit(wait, planned['id']) for _ in range(4)]
                # 【逐行批注】 计算右侧结果并赋给 planned，更新脚本状态。
                planned = polls[0].result()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(result.result()['course']['id'] == planned['course']['id'] for resul…。
                assert all(result.result()['course']['id'] == planned['course']['id'] for result in polls)
            # 【逐行批注】 计算右侧结果并赋给 course，更新脚本状态。
            course = planned['course']['id']
            # 【逐行批注】 计算右侧结果并赋给 outline，更新脚本状态。
            outline = api('/api/courses/' + course)['snapshot']['payload']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(outline['courseStructure']) == 1 and len(outline['courseStructure'][…。
            assert len(outline['courseStructure']) == 1 and len(outline['courseStructure'][0]['topics']) == 2
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert outline['generation']['model'] == 'fixture-agent-http'。
            assert outline['generation']['model'] == 'fixture-agent-http'
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(base + '/phase?courseId=' + course + '&phaseIndex=1') as response:。
            with urlopen(base + '/phase?courseId=' + course + '&phaseIndex=1') as response:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '快速规划' in response.read().decode('utf-8'), '目录入口应读取课程实际模式'。
                assert '快速规划' in response.read().decode('utf-8'), '目录入口应读取课程实际模式'
            # 【逐行批注】 计算右侧结果并赋给 calls，更新脚本状态。
            calls = len(ProtocolAI.calls)
            # 【逐行批注】 计算右侧结果并赋给 preview_before，更新脚本状态。
            preview_before = api('/api/courses/' + course + '/preview')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert preview_before['status'] in ('missing', 'waiting'), preview_before。
            assert preview_before['status'] in ('missing', 'waiting'), preview_before
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(ProtocolAI.calls) == calls, '读取预览不能启动独立教学模型'。
            assert len(ProtocolAI.calls) == calls, '读取预览不能启动独立教学模型'
            # 【逐行批注】 计算右侧结果并赋给 preview，更新脚本状态。
            preview = api('/api/courses/' + course + '/preview', dict(requestId='preview'), 202); wait(preview['id'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(api('/api/courses/' + course + '/preview')['slides']) == 2。
            assert len(api('/api/courses/' + course + '/preview')['slides']) == 2
            # 【逐行批注】 计算右侧结果并赋给 prepared，更新脚本状态。
            prepared = submit(dict(type='prepare_next', courseId=course, navigate=True))
            # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
            lesson = prepared['lesson']['id']
            # 【逐行批注】 计算右侧结果并赋给 shown，更新脚本状态。
            shown = api('/api/learning-agent/lesson?lessonId=' + lesson)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(shown['sections']) == 6 and 'PRIVATE-' not in json.dumps(shown)。
            assert len(shown['sections']) == 6 and 'PRIVATE-' not in json.dumps(shown)
            # 【逐行批注】 计算右侧结果并赋给 api('/api/learning-agent/control', dict(comm…，更新脚本状态。
            api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=lesson))
            # 【逐行批注】 计算右侧结果并赋给 same，更新脚本状态。
            same = api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=lesson))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert same['id'] == lesson。
            assert same['id'] == lesson
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learning-agent?taskId=' + prepared['id'])['lesson']['entered']。
            assert api('/api/learning-agent?taskId=' + prepared['id'])['lesson']['entered']
            # 【逐行批注】 计算右侧结果并赋给 question_count，更新脚本状态。
            question_count = sum(section['kind'] == 'question' for section in shown['sections'])
            # 【逐行批注】 计算右侧结果并赋给 started，更新脚本状态。
            started = api('/api/learning-agent/control', dict(command='start_lesson', lessonId=lesson)); wait(started['id'], 'waiting_student')
            # 【逐行批注】 计算右侧结果并赋给 calls_before_refresh，更新脚本状态。
            calls_before_refresh = len(ProtocolAI.calls)
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(3):。
            for _ in range(3):
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learning-agent/control', dict(command='start_lesson', lessonId…。
                assert api('/api/learning-agent/control', dict(command='start_lesson', lessonId=lesson))['id'] == started['id']
                # 【逐行批注】 计算右侧结果并赋给 focus，更新脚本状态。
                focus = api('/api/learning-agent/lesson?lessonId=' + lesson)['teachingFocus']
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert focus['interactionSectionId'] and focus['practiceSectionId']。
                assert focus['interactionSectionId'] and focus['practiceSectionId']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(ProtocolAI.calls) == calls_before_refresh, '刷新开课不能重复调用模型'。
            assert len(ProtocolAI.calls) == calls_before_refresh, '刷新开课不能重复调用模型'
            # 【逐行批注】 遍历数据并重复执行循环体：for section in (item for item in shown['sections'] if item['kind'] == 'que…。
            for section in (item for item in shown['sections'] if item['kind'] == 'question'):
                # 【逐行批注】 计算右侧结果并赋给 answered，更新脚本状态。
                answered = submit(dict(type='question_answer', courseId=course, lessonId=lesson, sectionId=section['id'],
                    # 【逐行批注】 计算右侧结果并赋给 sectionVersion，更新脚本状态。
                    sectionVersion=section['version'], text='化合价升高'), 'waiting_student')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answered['calls'] == 2, str(answered) + '全部题型必须先评价，再发起讲解请求'。
                assert answered['calls'] == 2, str(answered) +  '全部题型必须先评价，再发起讲解请求'
                # 【逐行批注】 计算右侧结果并赋给 api('/api/learn/exposure', dict(taskId，更新脚本状态。
                api('/api/learn/exposure', dict(taskId=answered['id'], seq=answered['seq']))
            # 【逐行批注】 计算右侧结果并赋给 first，更新脚本状态。
            first = shown['sections'][0]
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = api('/api/learning-agent/lesson?lessonId=' + lesson)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(before['sections'][0]['dialog']) == 2。
            assert len(before['sections'][0]['dialog']) == 2
            # 【逐行批注】 计算右侧结果并赋给 followup，更新脚本状态。
            followup = submit(dict(type='question_answer', courseId=course, lessonId=lesson, sectionId=first['id'],
                # 【逐行批注】 计算右侧结果并赋给 sectionVersion，更新脚本状态。
                sectionVersion=first['version'], text='为什么这样判断？'), 'waiting_student')
            # 【逐行批注】 计算右侧结果并赋给 api('/api/learn/exposure', dict(taskId，更新脚本状态。
            api('/api/learn/exposure', dict(taskId=followup['id'], seq=followup['seq']))
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 计算右侧结果并赋给 grades，更新脚本状态。
                grades = [json.loads(row[0]) | {'id': row[1]} for row in db.execute("SELECT payload,id FROM LearningInteraction WHERE kind='practice'")]
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(grades) == question_count, '追问不能新增一道已评分题'。
                assert len(grades) == question_count, '追问不能新增一道已评分题'
            # 【逐行批注】 计算右侧结果并赋给 interrupted，更新脚本状态。
            interrupted = submit(dict(type='question_answer', courseId=course, lessonId=lesson, sectionId=first['id'],
                # 【逐行批注】 计算右侧结果并赋给 sectionVersion，更新脚本状态。
                sectionVersion=first['version'], text='化合价升高', requestId='interrupted-answer', failureAfterEvaluation=True), 'failed')
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practic…。
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practice'").fetchone()[0] == question_count, '讲解中断不能提交暂存评价'
            # 【逐行批注】 计算右侧结果并赋给 api('/api/learn/exposure', dict(taskId，更新脚本状态。
            api('/api/learn/exposure', dict(taskId=interrupted['id'], seq=interrupted['seq']))
            # 【逐行批注】 计算右侧结果并赋给 partial，更新脚本状态。
            partial = api('/api/learning-agent/lesson?lessonId=' + lesson)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not any(item.get('taskId') == interrupted['id'] for item in partial['sec…。
            assert not any(item.get('taskId') == interrupted['id'] for item in partial['sections'][0]['dialog']), '未通过完整校验的中断回复不能登记为学生看过的讲解'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert interrupted['activity']['phase'] in ('receiving', 'failed'), '保留真实接收或失败进…。
            assert interrupted['activity']['phase'] in ('receiving', 'failed'), '保留真实接收或失败进度，不公开未验证文字'
            # 【逐行批注】 执行当前 Python 语句：ProtocolAI.repaired.add('interrupted-answer')。
            ProtocolAI.repaired.add('interrupted-answer')
            # 【逐行批注】 计算右侧结果并赋给 api('/api/learning-agent/control', dict(comm…，更新脚本状态。
            api('/api/learning-agent/control', dict(command='retry', taskId=interrupted['id']))
            # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(interrupted['id'], 'waiting_student')。
            wait(interrupted['id'], 'waiting_student')
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 计算右侧结果并赋给 new_grades，更新脚本状态。
                new_grades = [json.loads(row[0]) | {'id': row[1]} for row in db.execute("SELECT payload,id FROM LearningInteraction WHERE kind='practice'")]
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(new_grades) == question_count + 1。
                assert len(new_grades) == question_count + 1
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(not item.get('assisted') for item in new_grades), '普通概念讲解不能作为本题提示'。
                assert all(not item.get('assisted') for item in new_grades), '普通概念讲解不能作为本题提示'
            # 【逐行批注】 计算右侧结果并赋给 grade，更新脚本状态。
            grade = grades[0]
            # 【逐行批注】 计算右侧结果并赋给 rating，更新脚本状态。
            rating = dict(score=71, sufficient=True, evidenceIds=[grade['id']], rationale='引用一道适用的真实回答', recommendation='继续概念辨析', uncertainty='仅反映化合价本题')
            # 【逐行批注】 计算右侧结果并赋给 tool('update_profiles', dict(subjects，更新脚本状态。
            tool('update_profiles', dict(subjects=[dict(rating, subject='化学')], abilities=[dict(rating, id='understanding')]))
            # 【逐行批注】 计算右侧结果并赋给 profile，更新脚本状态。
            profile = api('/api/profile')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert next(item for item in profile['subjects'] if item['subject'] == '化学')['e…。
            assert next(item for item in profile['subjects'] if item['subject'] == '化学')['evidenceCount'] == 1
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert next(item for item in profile['abilities'] if item['id'] == 'understandi…。
            assert next(item for item in profile['abilities'] if item['id'] == 'understanding')['score'] == 71
            # 同一道题多轮仍只有一道证据；充分性由模型判断。
            # 【逐行批注】 计算右侧结果并赋给 repeated，更新脚本状态。
            repeated = [item['id'] for item in new_grades if item['questionId'] == grade['questionId']]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(repeated) == 2。
            assert len(repeated) == 2
            # 【逐行批注】 计算右侧结果并赋给 rating['evidenceIds']，更新脚本状态。
            rating['evidenceIds'] = repeated
            # 【逐行批注】 计算右侧结果并赋给 tool('update_profiles', dict(abilities，更新脚本状态。
            tool('update_profiles', dict(abilities=[dict(rating, id='understanding')]))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert next(item for item in api('/api/profile')['abilities'] if item['id'] == …。
            assert next(item for item in api('/api/profile')['abilities'] if item['id'] == 'understanding')['evidenceCount'] == 1
            # 未进入的未来主题可以重排，实际进入后撤回不能删除该内容。
            # 【逐行批注】 计算右侧结果并赋给 adjusted，更新脚本状态。
            adjusted = json.loads(json.dumps(outline))
            # 【逐行批注】 执行当前 Python 语句：adjusted['courseStructure'][0]['topics'].reverse()。
            adjusted['courseStructure'][0]['topics'].reverse()
            # 【逐行批注】 计算右侧结果并赋给 reordered，更新脚本状态。
            reordered = tool('revise_outline', dict(courseId=course, payload=adjusted, reason='模型选择先比较电子，再梳理概念'))
            # 【逐行批注】 计算右侧结果并赋给 undo_outline，更新脚本状态。
            undo_outline = api('/api/learning-agent/control', dict(command='undo', changeId=reordered['changes'][0]['id']))
            # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(undo_outline['id'])。
            wait(undo_outline['id'])
            # 【逐行批注】 计算右侧结果并赋给 restored_outline，更新脚本状态。
            restored_outline = api('/api/courses/' + course)['snapshot']['payload']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert restored_outline['courseStructure'][0]['topics'][0]['id'] == outline['co…。
            assert restored_outline['courseStructure'][0]['topics'][0]['id'] == outline['courseStructure'][0]['topics'][0]['id']
            # 【逐行批注】 计算右侧结果并赋给 changed，更新脚本状态。
            changed = tool('adjust_goal', dict(courseId=course, goal='先理解电子变化', reason='真实反馈显示需补齐概念'))
            # 【逐行批注】 计算右侧结果并赋给 change，更新脚本状态。
            change = changed['changes'][0]['id']
            # 【逐行批注】 计算右侧结果并赋给 undo，更新脚本状态。
            undo = api('/api/learning-agent/control', dict(command='undo', changeId=change))
            # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(undo['id'])。
            wait(undo['id'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/courses/' + course)['course']['goal'] == '化合价'。
            assert api('/api/courses/' + course)['course']['goal'] == '化合价'
            # 【逐行批注】 计算右侧结果并赋给 schedule，更新脚本状态。
            schedule = tool('schedule_review', dict(courseId=course, title='根据真实作答复习', due='2099-01-05', reason='模型自主选择日期', lessonId=lesson))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert schedule['status'] == 'ready'。
            assert schedule['status'] == 'ready'
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 计算右侧结果并赋给 reviews，更新脚本状态。
                reviews = [json.loads(row[0]) for row in db.execute("SELECT payload FROM ClassroomActivity WHERE kind='review'")]
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(reviews) == 1 and reviews[0]['due'] == '2099-01-05'。
                assert len(reviews) == 1 and reviews[0]['due'] == '2099-01-05'
            future_review = dict(type='prepare_next', courseId=course, reviewId=reviews[0]['id'], day=0,
                                 requestId='early-scheduled-review')
            assert api('/api/learning-agent/events', future_review, 409)
            origin_lesson = api('/api/learning-agent/lesson?lessonId=' + lesson)
            latest_evaluation = origin_lesson['learningState']['evidence']['evaluationIds'][-1]
            review_args = dict(courseId=course, title='回查化合价作答', due=date.today().isoformat(),
                               reason='核对最近的正式回答', lessonId=lesson, minimumDelaySeconds=86400)
            for bad_delay in [True, 0, 4294967297]:
                submit(dict(type='acceptance_tool', tool='schedule_review',
                            args=dict(review_args, minimumDelaySeconds=bad_delay)), 'failed')
            submit(dict(type='acceptance_tool', tool='schedule_review',
                        args=dict(review_args, due='2000-01-01')), 'failed')
            submit(dict(type='acceptance_tool', tool='schedule_review',
                        args=dict(review_args, evidenceIds=['未保存的评价'])), 'failed')
            tool('schedule_review', dict(review_args, evidenceIds=[latest_evaluation]))
            items = api('/api/classroom/reviews?courseId=' + course)['items']
            scheduled = next(item for item in items if item['title'] == review_args['title'])
            assert scheduled['evidenceIds'] == [latest_evaluation]
            review_id = scheduled['reviewId']
            review_path = f'/learn?courseId={course}&review=0&reviewId={review_id}'
            with urlopen(base + review_path) as response:
                assert '/agent-prepare.html?' in response.url and 'prepare-' + review_id + '-verified' in response.url
            browser_output = os.environ.get('GANGYI_REVIEW_BROWSER_REPORT')
            if browser_output:
                from playwright.sync_api import expect, sync_playwright

                report = Path(browser_output)
                snapshots = []
                destinations = []
                with sync_playwright() as driver:
                    browser = driver.chromium.launch(channel='msedge', headless=True)
                    for width, height in [(1280, 720), (390, 844)]:
                        page = browser.new_page(viewport=dict(width=width, height=height))
                        errors = []
                        page.on('pageerror', lambda error: errors.append(str(error)))
                        page.goto(base + '/my-courses')
                        page.get_by_text('待复习清单', exact=True).click()
                        link = page.get_by_role('link', name='回查化合价作答', exact=False)
                        expect(link).to_contain_text('依据 1 条历史作答')
                        panel = page.locator('.uc-review-details')
                        expect(panel.get_by_role('link', name='回看原课')).to_have_attribute('href', '/learn?lessonId=' + lesson)
                        panel.screenshot(path=report.with_name(f'scheduled-review-{width}.png'))
                        link.focus()
                        page.keyboard.press('Enter')
                        page.wait_for_url('**/practice?lessonId=**', timeout=30000)
                        assert f'reviewId={review_id}' in page.url and not errors, errors
                        destinations.append(page.url)
                        snapshots.append(dict(width=width, height=height, classroom=page.url.split('?')[0].split('/')[-1]))
                        page.close()
                    browser.close()
                assert len(set(destinations)) == 1
                report.write_text(json.dumps(dict(views=snapshots, reusedClassroom=True, paidModelRequests=0)), encoding='utf-8')
            opening = dict(type='prepare_next', courseId=course, reviewId=review_id, day=0,
                           requestId='open-scheduled-review', navigate=True)
            prepared_review = submit(opening)
            assert api('/api/learning-agent/events', opening, 202)['id'] == prepared_review['id']
            with closing(sqlite3.connect(database)) as db:
                row = json.loads(db.execute('SELECT payload FROM ClassroomActivity WHERE id=?',
                    (prepared_review['lesson']['id'],)).fetchone()[0])
                assert row['intent'] == 'review' and row['scheduledReviewId'] == review_id
                prior_score, = db.execute('SELECT payload FROM LearningInteraction WHERE id=?',
                    (latest_evaluation,)).fetchone()
                corrected = json.loads(prior_score)
                corrected['score'] = (corrected['score'] + 1) % 101
                db.execute('UPDATE LearningInteraction SET payload=? WHERE id=?',
                    (json.dumps(corrected), latest_evaluation))
                db.commit()
            try:
                with urlopen(base + review_path) as response:
                    assert 'prepare-' + review_id + '-changed' in response.url
                api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=prepared_review['lesson']['id']), 409)
                submit(dict(opening, requestId='open-scheduled-review-after-correction'))
            finally:
                with closing(sqlite3.connect(database)) as db:
                    db.execute('UPDATE LearningInteraction SET payload=? WHERE id=?', (prior_score, latest_evaluation))
                    db.commit()
            # 【逐行批注】 计算右侧结果并赋给 chat，更新脚本状态。
            chat = submit(dict(type='chat', courseId=course, lessonId=lesson, text='请用日常例子解释'), 'ready')
            # 【逐行批注】 计算右侧结果并赋给 api('/api/learn/exposure', dict(taskId，更新脚本状态。
            api('/api/learn/exposure', dict(taskId=chat['id'], seq=chat['seq']))
            # 【逐行批注】 计算右侧结果并赋给 restored，更新脚本状态。
            restored = api('/api/learning-agent/lesson?lessonId=' + lesson)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert [item['role'] for item in restored['dialog']] == ['user', 'assistant']。
            assert [item['role'] for item in restored['dialog']] == ['user', 'assistant']
            # 普通交流会更新全局学习版本，复习依据未变时仍可进入已备课时。
            review_lesson = prepared_review['lesson']['id']
            api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=review_lesson))
            # 批量兼容入口区分漏答和不会；同一请求重放不会新增输入或评价。
            # 【逐行批注】 计算右侧结果并赋给 batch，更新脚本状态。
            batch = dict(courseId=course, lessonId=lesson, kind='quiz', answers=[None], requestId='batch-omitted')
            # 【逐行批注】 计算右侧结果并赋给 omitted，更新脚本状态。
            omitted = api('/api/quiz-attempts', batch, 202); wait(omitted['id'], 'waiting_student')
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practic…。
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practice'").fetchone()[0] == question_count + 1
            # 【逐行批注】 计算右侧结果并赋给 batch.update(answers，更新脚本状态。
            batch.update(answers=['unknown'], requestId='batch-unknown')
            # 【逐行批注】 计算右侧结果并赋给 unknown，更新脚本状态。
            unknown = api('/api/quiz-attempts', batch, 202); wait(unknown['id'], 'waiting_student')
            # 【逐行批注】 计算右侧结果并赋给 calls，更新脚本状态。
            calls = len(ProtocolAI.calls)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/quiz-attempts', batch, 202)['id'] == unknown['id']。
            assert api('/api/quiz-attempts', batch, 202)['id'] == unknown['id']
            # 【逐行批注】 调用 len 完成当前脚本或测试步骤：time.sleep(.2); assert len(ProtocolAI.calls) == calls。
            time.sleep(.2); assert len(ProtocolAI.calls) == calls
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practic…。
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practice'").fetchone()[0] == question_count + 2
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [json.loads(row[0]).get('action') for row in db.execute("SELECT payload FROM LearningInteraction WHERE kind='chat-user'")]
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'omitted' in actions and 'unknown' in actions。
                assert 'omitted' in actions and 'unknown' in actions
            # v5.3/v5.4 的原课堂与连续对话保持身份，公开内容不泄露旧答案。
            # 【逐行批注】 计算右侧结果并赋给 old_content，更新脚本状态。
            old_content = dict(blocks=dict(overview=dict(title='原概览',summary='已保存的概念概览',keyConcepts=['已保存的关键概念']),
                # 【逐行批注】 计算右侧结果并赋给 examples，更新脚本状态。
                examples=dict(examples=[dict(title='原示范',content='原示范题干',solution='原 AI 示范解法')]),
                # 【逐行批注】 计算右侧结果并赋给 assessment，更新脚本状态。
                assessment=dict(checkpoint=['原检查点'],commonMistakes=['原错误提醒'],resourceSummary='原资料说明'),
                # 【逐行批注】 计算右侧结果并赋给 steps，更新脚本状态。
                steps=dict(lessonSteps=[dict(title='原讲解', explanation='旧课件说明')]),
                # 【逐行批注】 计算右侧结果并赋给 practice，更新脚本状态。
                practice=dict(practice=[dict(question='旧题：比较化合价', expectedAnswer='PRIVATE-OLD')]),
                # 【逐行批注】 计算右侧结果并赋给 quiz，更新脚本状态。
                quiz=dict(quiz=[dict(question='旧测验', options=['甲', '乙'], answerIndex=0)])), references=[dict(title='原参考资料',url='https://example.com/learning')], contentVersion=3)
            # 【逐行批注】 计算右侧结果并赋给 old_dialog，更新脚本状态。
            old_dialog = dict(turns=[dict(user='我原来写过的回答', assistant='我实际看过的旧讲解', status='ready')])
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 在受控上下文中执行资源操作：with db:。
                with db:
                    # 【逐行批注】 调用 LearningSession、VALUES 完成当前脚本或测试步骤：db.execute('INSERT INTO LearningSession(id,courseId,goal,mode,phaseIn…。
                    db.execute('INSERT INTO LearningSession(id,courseId,goal,mode,phaseIndex,phaseName,topicIndex,topicTitle,title,content) VALUES(?,?,?,?,?,?,?,?,?,?)',
                        # 【逐行批注】 执行当前 Python 语句：('old-session', course, '化合价', 'lite', 1, '概念', 1, '化合价', '原有课堂', json.dumps(old_content)…。
                        ('old-session', course, '化合价', 'lite', 1, '概念', 1, '化合价', '原有课堂', json.dumps(old_content)))
                    # 【逐行批注】 调用 ClassroomActivity、VALUES 完成当前脚本或测试步骤：db.execute('INSERT INTO ClassroomActivity(id,courseId,kind,payload,up…。
                    db.execute('INSERT INTO ClassroomActivity(id,courseId,kind,payload,updatedAt,phaseIndex,topicIndex) VALUES(?,?,?,?,?,?,?)',
                        # 【逐行批注】 执行当前 Python 语句：('classroom:' + course + ':1:1:practice:0:dialogue', course, 'dialogue', json.dumps(old_d…。
                        ('classroom:' + course + ':1:1:practice:0:dialogue', course, 'dialogue', json.dumps(old_dialog), '2026-01-01', 1, 1))
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 在受控上下文中执行资源操作：with db:。
                with db:
                    # 【逐行批注】 调用 ClassroomActivity、VALUES 完成当前脚本或测试步骤：db.execute('INSERT INTO ClassroomActivity(id,courseId,kind,payload,up…。
                    db.execute('INSERT INTO ClassroomActivity(id,courseId,kind,payload,updatedAt,phaseIndex,topicIndex) VALUES(?,?,?,?,?,?,?)',
                        # 【逐行批注】 计算右侧结果并赋给 ('old-practice-evidence', course, 'practice'…，更新脚本状态。
                        ('old-practice-evidence', course, 'practice', json.dumps(dict(status='answered', credible=True, questionSnapshot=dict(question='旧题：比较化合价'), answer='我原来写过的回答')), '2026-01-01', 1, 1))
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 在受控上下文中执行资源操作：with db:。
                with db:
                    # 【逐行批注】 调用 LearningInteraction、VALUES 完成当前脚本或测试步骤：db.execute('INSERT INTO LearningInteraction(id,kind,payload,createdAt…。
                    db.execute('INSERT INTO LearningInteraction(id,kind,payload,createdAt,courseId) VALUES(?,?,?,?,?)',
                        # 【逐行批注】 计算右侧结果并赋给 ('old-quiz-evidence','quiz',json.dumps(dict(…，更新脚本状态。
                        ('old-quiz-evidence','quiz',json.dumps(dict(phaseIndex=1,topicIndex=1,score=1,total=2,answers=[0,'unknown'],results=[dict(answerIndex=0,expectedAnswer='PRIVATE-HISTORICAL')])),'2026-01-01',course))
            # 【逐行批注】 计算右侧结果并赋给 old，更新脚本状态。
            old = api('/api/learn?courseId=' + course + '&phaseIndex=1&topicIndex=1')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert old['id'] == 'legacy-old-session' and 'PRIVATE-' not in json.dumps(old)。
            assert old['id'] == 'legacy-old-session' and 'PRIVATE-' not in json.dumps(old)
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(base + '/learn?courseId=' + course + '&phaseIndex=1&topicIndex=1&r…。
            with urlopen(base + '/learn?courseId=' + course + '&phaseIndex=1&topicIndex=1&review=2&reviewId=old-review') as response:
                # 【逐行批注】 计算右侧结果并赋给 destination，更新脚本状态。
                destination = response.geturl()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '/practice?' in destination and 'review=2' in destination and 'reviewId=…。
                assert '/practice?' in destination and 'review=2' in destination and 'reviewId=old-review' in destination, '旧复习入口保留作用域并进入练习页'
            # 【逐行批注】 计算右侧结果并赋给 sample，更新脚本状态。
            sample = next(item for item in old['sections'] if item.get('legacyKind') == 'example')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert sample['kind'] == 'explanation' and '原 AI 示范解法' in sample['body'], '原示范必…。
            assert sample['kind'] == 'explanation' and '原 AI 示范解法' in sample['body'], '原示范必须保留正文且不再作为新评分题'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert any(item.get('legacyKind') == 'overview' and '已保存的关键概念' in item['body'] …。
            assert any(item.get('legacyKind') == 'overview' and '已保存的关键概念' in item['body'] for item in old['sections'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert any(item.get('legacyKind') == 'checkpoint' and '原检查点' in item['body'] fo…。
            assert any(item.get('legacyKind') == 'checkpoint' and '原检查点' in item['body'] for item in old['sections'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert old['references'][0]['title'] == '原参考资料'。
            assert old['references'][0]['title'] == '原参考资料'
            # 【逐行批注】 计算右侧结果并赋给 historical，更新脚本状态。
            historical = next(item for item in old['historicalAssessments'] if item['id']=='old-quiz-evidence')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert historical['score']==1 and historical['total']==2 and historical['answer…。
            assert historical['score']==1 and historical['total']==2 and historical['answers']==[0,'unknown']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'PRIVATE-HISTORICAL' not in json.dumps(old), '历史测验只返回原提交与保存分数，不暴露评分资料'。
            assert 'PRIVATE-HISTORICAL' not in json.dumps(old), '历史测验只返回原提交与保存分数，不暴露评分资料'
            # 【逐行批注】 计算右侧结果并赋给 old_question，更新脚本状态。
            old_question = next(item for item in old['sections'] if item.get('legacyKind') == 'practice')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert [item['text'] for item in old_question['dialog']] == ['我原来写过的回答', '我实际看过…。
            assert [item['text'] for item in old_question['dialog']] == ['我原来写过的回答', '我实际看过的旧讲解'], old_question['dialog']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learn?courseId=' + course + '&phaseIndex=1&topicIndex=1')['id'…。
            assert api('/api/learn?courseId=' + course + '&phaseIndex=1&topicIndex=1')['id'] == old['id']
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert json.loads(db.execute("SELECT content FROM LearningSession WHERE id='old…。
                assert json.loads(db.execute("SELECT content FROM LearningSession WHERE id='old-session'").fetchone()[0]) == old_content
            # 【逐行批注】 计算右侧结果并赋给 prepared_content，更新脚本状态。
            prepared_content = json.loads(json.dumps(old_content)); prepared_content['blocks']['practice']['practice'][0]['question'] = '原已备巩固题，与推进课不同'
            # 【逐行批注】 计算右侧结果并赋给 legacy_prepared，更新脚本状态。
            legacy_prepared = dict(lessonTaskId='old-prepare', courseId=course, phaseIndex=1, topicIndex=1, topicTitle='原独立巩固课',
                # 【逐行批注】 计算右侧结果并赋给 kind，更新脚本状态。
                kind='consolidation', content=prepared_content, progress=dict(status='completed'))
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 在受控上下文中执行资源操作：with db:。
                with db:
                    # 【逐行批注】 调用 ClassroomActivity、VALUES 完成当前脚本或测试步骤：db.execute('INSERT INTO ClassroomActivity(id,courseId,kind,payload,up…。
                    db.execute('INSERT INTO ClassroomActivity(id,courseId,kind,payload,updatedAt,phaseIndex,topicIndex) VALUES(?,?,?,?,?,?,?)',
                        # 【逐行批注】 执行当前 Python 语句：('prepared-lesson:old-prepare', course, 'prepared-lesson', json.dumps(legacy_prepared), '…。
                        ('prepared-lesson:old-prepare', course, 'prepared-lesson', json.dumps(legacy_prepared), '2026-01-01', 1, 1))
                    # 【逐行批注】 调用 ClassroomActivity、VALUES 完成当前脚本或测试步骤：db.execute('INSERT INTO ClassroomActivity(id,courseId,kind,payload,up…。
                    db.execute('INSERT INTO ClassroomActivity(id,courseId,kind,payload,updatedAt,phaseIndex,topicIndex) VALUES(?,?,?,?,?,?,?)',
                        # 【逐行批注】 计算右侧结果并赋给 ('next-preparation:old-prepare', course, 'ne…，更新脚本状态。
                        ('next-preparation:old-prepare', course, 'next-preparation', json.dumps(dict(status='ready', classroomUrl='/learn?courseId='+course+'&lessonTaskId=old-prepare')), '2026-01-01', 1, 1))
                    # 【逐行批注】 调用 ClassroomActivity、VALUES 完成当前脚本或测试步骤：db.execute('INSERT INTO ClassroomActivity(id,courseId,kind,payload,up…。
                    db.execute('INSERT INTO ClassroomActivity(id,courseId,kind,payload,updatedAt,phaseIndex,topicIndex) VALUES(?,?,?,?,?,?,?)',
                        # 【逐行批注】 执行当前 Python 语句：('classroom:' + course + ':1:1:task:old-prepare:practice:0:dialogue', course, 'dialogue',…。
                        ('classroom:' + course + ':1:1:task:old-prepare:practice:0:dialogue', course, 'dialogue', json.dumps(old_dialog), '2026-01-01', 1, 1))
            # 【逐行批注】 计算右侧结果并赋给 kept，更新脚本状态。
            kept = api('/api/learn?courseId=' + course + '&lessonTaskId=old-prepare')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert kept['id'] != old['id'] and kept['completed']。
            assert kept['id'] != old['id'] and kept['completed']
            # 【逐行批注】 计算右侧结果并赋给 kept_question，更新脚本状态。
            kept_question = next(item for item in kept['sections'] if item.get('legacyKind') == 'practice')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert kept_question['question']['question'] == '原已备巩固题，与推进课不同'。
            assert kept_question['question']['question'] == '原已备巩固题，与推进课不同'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(kept_question['dialog']) == 2 and 'PRIVATE-' not in json.dumps(kept)。
            assert len(kept_question['dialog']) == 2 and 'PRIVATE-' not in json.dumps(kept)
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(base + '/learn/next?id=old-prepare', timeout=10) as response:。
            with urlopen(base + '/learn/next?id=old-prepare', timeout=10) as response:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'lessonId=' + kept['id'] in response.url。
                assert 'lessonId=' + kept['id'] in response.url
            # 【逐行批注】 计算右侧结果并赋给 legacy_rating，更新脚本状态。
            legacy_rating = dict(score=74, sufficient=True, evidenceIds=['old-practice-evidence'], rationale='引用保留的旧课堂真实回答', recommendation='继续说明比较过程', uncertainty='仅反映此道历史题')
            # 【逐行批注】 计算右侧结果并赋给 tool('update_profiles', dict(abilities，更新脚本状态。
            tool('update_profiles', dict(abilities=[dict(legacy_rating, id='expression')]))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert next(item for item in api('/api/profile')['abilities'] if item['id'] == …。
            assert next(item for item in api('/api/profile')['abilities'] if item['id'] == 'expression')['score'] == 74
            # 【逐行批注】 计算右侧结果并赋给 failed，更新脚本状态。
            review_page = '/api/learning-agent/lesson?lessonId=' + review_lesson
            close_review = dict(type='lesson_finish_request', courseId=course, lessonId=review_lesson,
                                text='检查本次复习是否完成')
            submit(dict(close_review), 'waiting_student')
            assert not api(review_page).get('completed', False)
            assert any(item['reviewId'] == review_id for item in api('/api/classroom/reviews?courseId=' + course)['items'])
            report_path = '/api/my-ai/reports?courseId=' + course
            assert api(report_path)['completedReviews'] == []
            review_questions = [section for section in api(review_page)['sections'] if section['kind'] == 'question']
            original_progress = api(report_path)['completed']
            for index, question in enumerate(review_questions):
                submit(dict(type='question_answer', courseId=course, lessonId=review_lesson,
                            sectionId=question['id'], sectionVersion=question['version'], text='化合价升高'), 'waiting_student')
                if index == 0:
                    submit(dict(close_review), 'waiting_student')
            latest_answer = dict(type='question_answer', courseId=course, lessonId=review_lesson,
                                 sectionId=review_questions[-1]['id'], sectionVersion=review_questions[-1]['version'])
            submit(dict(latest_answer, requestId='review-ungraded-revision', text='我想重新核对化合价变化'), 'failed')
            submit(dict(close_review), 'waiting_student')
            submit(dict(latest_answer, text='我重新确认化合价升高'), 'waiting_student')
            submit(dict(close_review))
            submit(dict(close_review))
            assert api(review_page)['completed']
            assert all(item['reviewId'] != review_id for item in api('/api/classroom/reviews?courseId=' + course)['items'])
            with urlopen(base + review_path) as response:
                assert '/practice?lessonId=' + review_lesson in response.url
            report = api(report_path)
            recorded, = report['completedReviews']
            assert report['completed'] == original_progress
            assert recorded['id'] == review_id and recorded['lessonId'] == review_lesson
            assert recorded['questionCount'] == len(review_questions) == len(recorded['evidenceIds'])
            assert recorded['changedCount'] == 0
            assert recorded['timing']['status'] == 'too_early'
            assert recorded['timing']['minimumDelaySeconds'] == 86400
            assert recorded['timing']['minimumElapsedSeconds'] < 86400
            assert recorded['timing']['learningEffectVerified'] is False
            assert api(report_path + '&from=2099-01-01')['completedReviews'] == []
            evidence_id = recorded['evidenceIds'][0]
            assert evidence_id in {item['id'] for item in report['evidence']}
            if browser_output:
                with sync_playwright() as driver:
                    browser = driver.chromium.launch(channel='msedge', headless=True)
                    page = browser.new_page(viewport=dict(width=390, height=844))
                    errors = []
                    page.on('pageerror', lambda error: errors.append(str(error)))
                    page.goto(base + '/my-ai/reports?courseId=' + course)
                    card = page.locator('.ma-card').filter(has=page.get_by_role('heading', name='已完成复习'))
                    expect(card.get_by_role('link', name='回看复习')).to_be_visible()
                    expect(card).to_contain_text('本次提前复习')
                    expect(card).to_contain_text('完成复习不代表已验证长期记忆')
                    link = card.get_by_role('link', name='第 1 条评价')
                    expect(link).to_have_attribute('href', f'/my-ai/reports?courseId={course}&evidenceId={evidence_id}#evidence')
                    card.screenshot(path=Path(browser_output).with_name(Path(browser_output).stem + '-completed-390.png'))
                    link.focus()
                    page.keyboard.press('Enter')
                    expect(page.locator('#evidence')).to_contain_text('化合价升高')
                    assert not errors, errors
                    browser.close()
            with closing(sqlite3.connect(database)) as db:
                db.execute('DELETE FROM LearningInteraction WHERE id=?', (evidence_id,))
                db.commit()
            revised, = api(report_path)['completedReviews']
            assert revised['changedCount'] == 1 and evidence_id not in revised['evidenceIds']
            assert revised['timing']['evidenceCurrent'] is False
            # 秒级间隔只验真实服务时间的协议；不会修改历史时间，也不作为长期效果样本。
            anchor = api('/api/learning-agent/lesson?lessonId=' + lesson)['learningState']['evidence']['evaluationIds'][-1]
            tool('schedule_review', dict(review_args, title='秒级协议验证',
                minimumDelaySeconds=2, evidenceIds=[anchor]))
            short_id = next(item['reviewId'] for item in api('/api/classroom/reviews?courseId=' + course)['items'] if item['title'] == '秒级协议验证')
            second_review = submit(dict(type='prepare_next', courseId=course, reviewId=short_id, day=0, navigate=True))
            second_lesson = second_review['lesson']['id']
            api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=second_lesson))
            time.sleep(2.1)
            for question in api('/api/learning-agent/lesson?lessonId=' + second_lesson)['sections']:
                if question['kind'] == 'question':
                    submit(dict(type='question_answer', courseId=course, lessonId=second_lesson,
                        sectionId=question['id'], sectionVersion=question['version'], text='化合价升高'), 'waiting_student')
            submit(dict(type='lesson_finish_request', courseId=course, lessonId=second_lesson, text='检查本次复习是否完成'))
            timed = next(item for item in api(report_path)['completedReviews'] if item['id'] == short_id)
            assert timed['timing']['status'] == 'delay_met' and timed['timing']['minimumElapsedSeconds'] >= 2
            assert timed['timing']['learningEffectVerified'] is False
            delay_checks = dict(earlyReview=recorded['timing'], afterEvaluationDeletion=revised['timing'],
                actualShortInterval=timed['timing'], historicalTimesModified=False, paidModelRequests=0,
                realLearningEffectEvaluated=False)
            if browser_output:
                saved_report = json.loads(Path(browser_output).read_text(encoding='utf-8'))
                saved_report['delayChecks'] = delay_checks
                Path(browser_output).write_text(json.dumps(saved_report, ensure_ascii=False, indent=2), encoding='utf-8')
            print(json.dumps(delay_checks, ensure_ascii=False), flush=True)
            failed = submit(dict(type='acceptance_fail'), 'failed')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert failed['calls'] == 3。
            assert failed['calls'] == 3
            # 【逐行批注】 计算右侧结果并赋给 count，更新脚本状态。
            count = len(ProtocolAI.calls); time.sleep(.3); api('/api/learning-agent?taskId=' + failed['id'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(ProtocolAI.calls) == count。
            assert len(ProtocolAI.calls) == count
            # 【逐行批注】 计算右侧结果并赋给 paused，更新脚本状态。
            paused = api('/api/learning-agent/control', dict(command='pause', taskId=failed['id']))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert paused['status'] == 'paused'。
            assert paused['status'] == 'paused'
            # 拒绝不存在的题目标识，原输入和可靠评价不能被错题覆盖。
            # 【逐行批注】 计算右侧结果并赋给 api('/api/classroom/submit', dict(courseId，更新脚本状态。
            api('/api/classroom/submit', dict(courseId=course, lessonId=lesson, sectionId='does-not-exist', kind='diagnostic', index=0, answer='升高'), 409)
            # 【逐行批注】 计算右侧结果并赋给 api('/api/learning-agent/lesson?lessonId，更新脚本状态。
            api('/api/learning-agent/lesson?lessonId=' + lesson)
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert db.execute('PRAGMA quick_check').fetchone()[0] == 'ok'。
                assert db.execute('PRAGMA quick_check').fetchone()[0] == 'ok'
            # 【逐行批注】 输出当前运行结果：print('主控 HTTP、多步自主规划、六类题型、先评价后讲解、追问不评分、一题画像、撤回、自由复习日期、对话恢复、缓存与三次失败停止通过（协议…。
            print('主控 HTTP、多步自主规划、六类题型、先评价后讲解、追问不评分、一题画像、撤回、自由复习日期、对话恢复、缓存与三次失败停止通过（协议夹具）')
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 判断条件 process.poll() is None:，成立时执行对应分支。
            if process.poll() is None:
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(base + '/internal/shutdown', data=b'', headers={'X-Gangyi-…。
                    with urlopen(Request(base + '/internal/shutdown', data=b'', headers={'X-Gangyi-Control-Token': 'fictional-control'}), timeout=10):
                        # 【逐行批注】 执行当前 Python 语句：pass。
                        pass
                    # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                    process.wait(timeout=15)
                # 【逐行批注】 捕获并处理异常：except Exception:。
                except Exception:
                    # 【逐行批注】 计算右侧结果并赋给 process.terminate(); process.wait(timeout，更新脚本状态。
                    process.terminate(); process.wait(timeout=15)
            # 【逐行批注】 执行当前 Python 语句：ai.shutdown(); ai.server_close(); log.close()。
            ai.shutdown(); ai.server_close(); log.close()


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encod…，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(sys.argv[1])。
    main(sys.argv[1])

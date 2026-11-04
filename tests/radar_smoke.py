# 【逐行批注】 执行当前 Python 语句："""用隔离虚构档案验证能力评估、逐维证据、失败保留和版本保护。"""。
"""用隔离虚构档案验证能力评估、逐维证据、失败保留和版本保护。"""
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
# 【逐行批注】 导入后续逻辑需要的模块或符号：import threading。
import threading
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：import urllib.error。
import urllib.error
# 【逐行批注】 导入后续逻辑需要的模块或符号：import urllib.request。
import urllib.request
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextlib import contextmanager。
from contextlib import contextmanager
# 【逐行批注】 导入后续逻辑需要的模块或符号：from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer。
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path

# 【逐行批注】 计算右侧结果并赋给 IDS，更新脚本状态。
IDS = ['memory', 'understanding', 'application', 'reasoning', 'expression', 'transfer']
# 【逐行批注】 计算右侧结果并赋给 NAMES，更新脚本状态。
NAMES = ['知识记忆', '概念理解', '方法应用', '逻辑推理', '表达说明', '综合迁移']
# 【逐行批注】 计算右侧结果并赋给 SUBJECTS，更新脚本状态。
SUBJECTS = ['语文', '数学', '英语', '物理', '化学', '生物']

# 【逐行批注】 定义函数 wait_for，封装可复用的处理步骤。
def wait_for(predicate, seconds=20):
    # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
    deadline = time.monotonic() + seconds
    # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
    while time.monotonic() < deadline:
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
            value = predicate()
            # 【逐行批注】 判断条件 value:，成立时执行对应分支。
            if value:
                # 【逐行批注】 结束当前函数并返回结果：return value。
                return value
        # 【逐行批注】 捕获并处理异常：except (OSError, ValueError):。
        except (OSError, ValueError):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass
        # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
        time.sleep(.1)
    # 【逐行批注】 在错误情形抛出异常：raise AssertionError('画像未在期限内达到预期状态')。
    raise AssertionError('画像未在期限内达到预期状态')

# 【逐行批注】 定义类 Harness，组织相关数据与行为。
class Harness:
    # 【逐行批注】 定义函数 __init__，封装可复用的处理步骤。
    def __init__(self, executable, root, unified=False):
        # 【逐行批注】 计算右侧结果并赋给 self.executable, self.root，更新脚本状态。
        self.executable, self.root = Path(executable).resolve(), Path(root)
        # 【逐行批注】 计算右侧结果并赋给 self.calls，更新脚本状态。
        self.calls = []
        # 【逐行批注】 计算右侧结果并赋给 self.mode，更新脚本状态。
        self.mode = 'valid'
        # 【逐行批注】 计算右侧结果并赋给 self.score，更新脚本状态。
        self.score = 67
        # 【逐行批注】 计算右侧结果并赋给 self.entered，更新脚本状态。
        self.entered = threading.Event()
        # 【逐行批注】 计算右侧结果并赋给 self.release，更新脚本状态。
        self.release = threading.Event()
        # 【逐行批注】 计算右侧结果并赋给 harness，更新脚本状态。
        harness = self
        # 【逐行批注】 定义类 Model，组织相关数据与行为。
        class Model(BaseHTTPRequestHandler):
            # 【逐行批注】 定义函数 log_message，封装可复用的处理步骤。
            def log_message(self, *_):
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
            # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
            def do_POST(self):
                # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
                body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
                # 【逐行批注】 计算右侧结果并赋给 system，更新脚本状态。
                system = body['messages'][0]['content']
                # 【逐行批注】 计算右侧结果并赋给 text，更新脚本状态。
                text = body['messages'][-1]['content']
                # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
                data = json.loads(text) if text.startswith('{') else {}
                # 【逐行批注】 判断条件 '课堂作答评价教师' in system:，成立时执行对应分支。
                if '课堂作答评价教师' in system:
                    # 【逐行批注】 计算右侧结果并赋给 given，更新脚本状态。
                    given = data['givenAnswer']
                    # 【逐行批注】 计算右侧结果并赋给 unknown，更新脚本状态。
                    unknown = '不会' in data.get('studentAnswer', '')
                    # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
                    result = {'isAnswer': True, 'correct': isinstance(given, int) and given == data['question'].get('answerIndex'),
                        # 【逐行批注】 执行当前 Python 语句：'unknown': unknown, 'confidence': .95, 'feedback': '根据本轮实际回答核对概念；未写过程时依据不足。',。
                        'unknown': unknown, 'confidence': .95, 'feedback': '根据本轮实际回答核对概念；未写过程时依据不足。',
                        # 【逐行批注】 执行当前 Python 语句：'misconception': '', 'methodAnalysis': '学生未提供推导过程，不能推测具体方法。'}。
                        'misconception': '', 'methodAnalysis': '学生未提供推导过程，不能推测具体方法。'}
                # 【逐行批注】 当前一条件不成立时继续判断：'课堂教师' in system:。
                elif '课堂教师' in system:
                    # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
                    result = '依据你的本轮回答说明概念，并结合原题继续讲解。'
                # 【逐行批注】 当前一条件不成立时继续判断：'六维学习能力评估教师' in system:。
                elif '六维学习能力评估教师' in system:
                    # 【逐行批注】 计算右侧结果并赋给 mode, score，更新脚本状态。
                    mode, score = harness.mode, harness.score
                    # 【逐行批注】 执行当前 Python 语句：harness.calls.append(data)。
                    harness.calls.append(data)
                    # 【逐行批注】 计算右侧结果并赋给 tasks，更新脚本状态。
                    tasks = data['tasks']
                    # 【逐行批注】 计算右侧结果并赋给 selected，更新脚本状态。
                    selected = [task for task in tasks if task['question'].get('type') == 'open'] if mode == 'complete' else tasks
                    # 【逐行批注】 计算右侧结果并赋给 references，更新脚本状态。
                    references = [task['id'] for task in selected[:3]]
                    # 【逐行批注】 判断条件 mode == 'blocked':，成立时执行对应分支。
                    if mode == 'blocked':
                        # 【逐行批注】 执行当前 Python 语句：harness.entered.set()。
                        harness.entered.set()
                        # 【逐行批注】 执行当前 Python 语句：harness.release.wait(12)。
                        harness.release.wait(12)
                    # 【逐行批注】 判断条件 mode == 'missing': references = ['不存在的题目'] * 3，成立时执行对应分支。
                    if mode == 'missing': references = ['不存在的题目'] * 3
                    # 【逐行批注】 判断条件 mode == 'few': references = references[:2]，成立时执行对应分支。
                    if mode == 'few': references = references[:2]
                    # 【逐行批注】 判断条件 mode == 'duplicate': references = [references[0]] * 3，成立时执行对应分支。
                    if mode == 'duplicate': references = [references[0]] * 3
                    # 【逐行批注】 判断条件 mode == 'out-of-range': score = 101，成立时执行对应分支。
                    if mode == 'out-of-range': score = 101
                    # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
                    result = {'abilities': [{'id': key,
                        # 【逐行批注】 执行当前 Python 语句：'score': score if key == 'memory' else 0 if key == 'understanding' else None,。
                        'score': score if key == 'memory' else 0 if key == 'understanding' else None,
                        # 【逐行批注】 执行当前 Python 语句：'rationale': '依据不同题目的真实作答判断已测任务表现。',。
                        'rationale': '依据不同题目的真实作答判断已测任务表现。',
                        # 【逐行批注】 调用 alert 完成当前脚本或测试步骤：'recommendation': '<img src=x onerror=alert(1)>先复习概念，再解释一次。',。
                        'recommendation': '<img src=x onerror=alert(1)>先复习概念，再解释一次。',
                        # 【逐行批注】 调用 in 完成当前脚本或测试步骤：'evidenceIds': references if key in ('memory', 'understanding') else …。
                        'evidenceIds': references if key in ('memory', 'understanding') else []} for key in IDS]}
                    # 【逐行批注】 判断条件 mode == 'expression':，成立时执行对应分支。
                    if mode == 'expression':
                        # 【逐行批注】 计算右侧结果并赋给 result['abilities'][4].update(score，更新脚本状态。
                        result['abilities'][4].update(score=70, evidenceIds=references)
                    # 【逐行批注】 判断条件 mode == 'complete':，成立时执行对应分支。
                    if mode == 'complete':
                        # 【逐行批注】 遍历数据并重复执行循环体：for item, score in zip(result['abilities'], [55, 0, 72, 44, 63, 41]):。
                        for item, score in zip(result['abilities'], [55, 0, 72, 44, 63, 41]):
                            # 【逐行批注】 计算右侧结果并赋给 item.update(score，更新脚本状态。
                            item.update(score=score, evidenceIds=references)
                    # 【逐行批注】 判断条件 mode == 'fail':，成立时执行对应分支。
                    if mode == 'fail':
                        # 【逐行批注】 执行当前 Python 语句：self.send_response(503); self.end_headers(); return。
                        self.send_response(503); self.end_headers(); return
                # 【逐行批注】 当前一条件不成立时继续判断：'学习诊断教师' in system:。
                elif '学习诊断教师' in system:
                    # 【逐行批注】 计算右侧结果并赋给 events，更新脚本状态。
                    events = [event for event in data['events'] if event['kind'] in ('quiz', 'question-evaluation')]
                    # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
                    result = {'score': 73, 'rationale': '三道虚构题目反馈', 'weakPoints': [],
                              # 【逐行批注】 执行当前 Python 语句：'recommendation': '先复习基础', 'evidenceIds': [events[-1]['id']], 'nextReviewAt': ''}。
                              'recommendation': '先复习基础', 'evidenceIds': [events[-1]['id']], 'nextReviewAt': ''}
                # 【逐行批注】 当前一条件不成立时继续判断：'画像评估 AI' in system:。
                elif '画像评估 AI' in system:
                    # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
                    result = {'subjects': [{'subject': '数学', 'score': 73, 'rationale': '已测题目表现',
                        # 【逐行批注】 执行当前 Python 语句：'weakPoints': [], 'recommendation': '继续练习',。
                        'weakPoints': [], 'recommendation': '继续练习',
                        # 【逐行批注】 执行当前 Python 语句：'evidenceIds': data['topicStates'][0]['evidenceIds']}]}。
                        'evidenceIds': data['topicStates'][0]['evidenceIds']}]}
                # 【逐行批注】 执行前一条件不成立时的分支。
                else:
                    # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
                    result = {mode: {'continue': [f'{mode} 基础目标{i}' for i in range(3)],
                        # 【逐行批注】 调用 range、in 完成当前脚本或测试步骤：'explore': [f'{mode} 探索目标{i}' for i in range(2)]} for mode in ('lite'…。
                        'explore': [f'{mode} 探索目标{i}' for i in range(2)]} for mode in ('lite', 'deep')}
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload = json.dumps({'model': 'isolated-mock', 'choices': [{'message': {
                    # 【逐行批注】 计算右侧结果并赋给 'content': result if isinstance(result, str)…，更新脚本状态。
                    'content': result if isinstance(result, str) else json.dumps(result, ensure_ascii=False)}}]}, ensure_ascii=False).encode()
                # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
                self.send_response(200)
                # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'application/json')。
                self.send_header('Content-Type', 'application/json')
                # 【逐行批注】 调用 str、len 完成当前脚本或测试步骤：self.send_header('Content-Length', str(len(payload)))。
                self.send_header('Content-Length', str(len(payload)))
                # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
                self.end_headers()
                # 【逐行批注】 执行当前 Python 语句：self.wfile.write(payload)。
                self.wfile.write(payload)
        # 【逐行批注】 判断条件 unified:，成立时执行对应分支。
        if unified:
            # 【逐行批注】 导入后续逻辑需要的模块或符号：from learning_agent_api_smoke import ProtocolAI。
            from learning_agent_api_smoke import ProtocolAI
            # 【逐行批注】 执行当前 Python 语句：ProtocolAI.calls.clear()。
            ProtocolAI.calls.clear()
            # 【逐行批注】 计算右侧结果并赋给 Model，更新脚本状态。
            Model = ProtocolAI
            # 【逐行批注】 计算右侧结果并赋给 self.calls，更新脚本状态。
            self.calls = ProtocolAI.calls
        # 【逐行批注】 计算右侧结果并赋给 self.model，更新脚本状态。
        self.model = ThreadingHTTPServer(('127.0.0.1', 0), Model)
        # 【逐行批注】 计算右侧结果并赋给 threading.Thread(target，更新脚本状态。
        threading.Thread(target=self.model.serve_forever, daemon=True).start()
        # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as sock:。
        with socket.socket() as sock:
            # 【逐行批注】 计算右侧结果并赋给 sock.bind(('127.0.0.1', 0)); port，更新脚本状态。
            sock.bind(('127.0.0.1', 0)); port = sock.getsockname()[1]
        # 【逐行批注】 计算右侧结果并赋给 self.base，更新脚本状态。
        self.base = f'http://127.0.0.1:{port}'
        # 【逐行批注】 计算右侧结果并赋给 self.database，更新脚本状态。
        self.database = self.root / 'fictional.db'
        # 【逐行批注】 计算右侧结果并赋给 self.env，更新脚本状态。
        self.env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(self.database),
            # 【逐行批注】 计算右侧结果并赋给 AI_BASE_URL，更新脚本状态。
            AI_BASE_URL=f'http://127.0.0.1:{self.model.server_port}/v1', AI_API_KEY='fictional-test-key',
            # 【逐行批注】 计算右侧结果并赋给 AI_MODEL，更新脚本状态。
            AI_MODEL='isolated-mock', BOCHA_API_KEY='', SEARCH_FALLBACK_PROVIDER='',
            # 【逐行批注】 计算右侧结果并赋给 RESOURCE_SEARCH_CACHE_DIR，更新脚本状态。
            RESOURCE_SEARCH_CACHE_DIR=str(self.root / 'search-cache'), LOCAL_CONTROL_TOKEN='radar-test',
            # 【逐行批注】 计算右侧结果并赋给 GANGYI_LAUNCH_SESSION_ID，更新脚本状态。
            GANGYI_LAUNCH_SESSION_ID='radar-test-launch')
        # 【逐行批注】 执行当前 Python 语句：self.start()。
        self.start()

    # 【逐行批注】 定义函数 request，封装可复用的处理步骤。
    def request(self, path, method='GET', body=None, expected=200):
        # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
        payload = None if body is None else json.dumps(body, ensure_ascii=False).encode()
        # 【逐行批注】 计算右侧结果并赋给 req，更新脚本状态。
        req = urllib.request.Request(self.base + path, data=payload, method=method,
            # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
            headers={'Content-Type': 'application/json', 'X-Gangyi-Control-Token': 'radar-test'})
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
            response = urllib.request.urlopen(req, timeout=8)
        # 【逐行批注】 捕获并处理异常：except urllib.error.HTTPError as error:。
        except urllib.error.HTTPError as error:
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert error.code == expected, (error.code, error.read())。
            assert error.code == expected, (error.code, error.read())
            # 【逐行批注】 结束当前函数并返回结果：return json.loads(error.read())。
            return json.loads(error.read())
        # 【逐行批注】 在受控上下文中执行资源操作：with response:。
        with response:
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert response.status == expected。
            assert response.status == expected
            # 【逐行批注】 结束当前函数并返回结果：return json.loads(response.read())。
            return json.loads(response.read())

    # 【逐行批注】 应用装饰器 @contextmanager，扩展下一函数或测试的行为。
    @contextmanager
    # 【逐行批注】 定义函数 db，封装可复用的处理步骤。
    def db(self):
        # 【逐行批注】 计算右侧结果并赋给 connection，更新脚本状态。
        connection = sqlite3.connect(self.database, timeout=10)
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 在受控上下文中执行资源操作：with connection: yield connection。
            with connection: yield connection
        # 【逐行批注】 执行当前 Python 语句：finally: connection.close()。
        finally: connection.close()

    # 【逐行批注】 定义函数 start，封装可复用的处理步骤。
    def start(self):
        # 【逐行批注】 计算右侧结果并赋给 self.log，更新脚本状态。
        self.log = (self.root / 'service.log').open('ab')
        # 【逐行批注】 计算右侧结果并赋给 self.process，更新脚本状态。
        self.process = subprocess.Popen([str(self.executable)], cwd=self.executable.parent, env=self.env,
            # 【逐行批注】 计算右侧结果并赋给 stdout，更新脚本状态。
            stdout=self.log, stderr=self.log, creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        # 【逐行批注】 调用 wait_for 完成当前脚本或测试步骤：wait_for(lambda: self.request('/api/profile'))。
        wait_for(lambda: self.request('/api/profile'))

    # 【逐行批注】 定义函数 stop，封装可复用的处理步骤。
    def stop(self):
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 执行当前 Python 语句：self.request('/internal/shutdown', 'POST', {})。
            self.request('/internal/shutdown', 'POST', {})
            # 【逐行批注】 计算右侧结果并赋给 self.process.wait(timeout，更新脚本状态。
            self.process.wait(timeout=15)
        # 【逐行批注】 捕获并处理异常：except (OSError, subprocess.TimeoutExpired):。
        except (OSError, subprocess.TimeoutExpired):
            # 【逐行批注】 执行当前 Python 语句：self.process.kill(); self.process.wait()。
            self.process.kill(); self.process.wait()
        # 【逐行批注】 执行当前 Python 语句：self.log.close()。
        self.log.close()

    # 【逐行批注】 定义函数 close，封装可复用的处理步骤。
    def close(self):
        # 【逐行批注】 执行当前 Python 语句：self.release.set(); self.stop(); self.model.shutdown(); self.model.server_close()。
        self.release.set(); self.stop(); self.model.shutdown(); self.model.server_close()

    # 【逐行批注】 定义函数 seed_quiz，封装可复用的处理步骤。
    def seed_quiz(self, repeated=False):
        # 【逐行批注】 在受控上下文中执行资源操作：with self.db() as db:。
        with self.db() as db:
            # 【逐行批注】 调用 VALUES 完成当前脚本或测试步骤：db.execute('INSERT OR IGNORE INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?…。
            db.execute('INSERT OR IGNORE INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?)',
                # 【逐行批注】 执行当前 Python 语句：('fictional-course', None, None, '数学概念与方法', 'deep', '虚构数学课程', None,。
                ('fictional-course', None, None, '数学概念与方法', 'deep', '虚构数学课程', None,
                 # 【逐行批注】 执行当前 Python 语句：'ai', 'active', '2026-01-01', '2026-01-01'))。
                 'ai', 'active', '2026-01-01', '2026-01-01'))
            # 【逐行批注】 计算右侧结果并赋给 questions，更新脚本状态。
            questions = [{'question': '请判断虚构概念题' + str(0 if repeated else i),
                # 【逐行批注】 调用 range 完成当前脚本或测试步骤：'options': ['甲', '乙', '丙', '丁'], 'answerIndex': 0} for i in range(3)]。
                'options': ['甲', '乙', '丙', '丁'], 'answerIndex': 0} for i in range(3)]
            # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
            payload = {'promptVersion': 'ai-block-v1', 'blocks': {'quiz': {'quiz': questions}}}
            # 更换虚构题组只用于证据门槛测试；独立的课堂验收覆盖已展示题的稳定性。
            # 【逐行批注】 执行当前 Python 语句：db.execute("DELETE FROM ClassroomActivity WHERE id LIKE 'classroom:fictional-course:1:1:q…。
            db.execute("DELETE FROM ClassroomActivity WHERE id LIKE 'classroom:fictional-course:1:1:quiz:%'")
            # 【逐行批注】 调用 VALUES 完成当前脚本或测试步骤：db.execute('INSERT OR REPLACE INTO LearningSession VALUES(?,?,?,?,?,?…。
            db.execute('INSERT OR REPLACE INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)',
                # 【逐行批注】 执行当前 Python 语句：('fictional-session', 'fictional-course', None, '数学概念与方法', 'deep', 1, '基础', 1,。
                ('fictional-session', 'fictional-course', None, '数学概念与方法', 'deep', 1, '基础', 1,
                 # 【逐行批注】 计算右侧结果并赋给 '基础概念', '虚构题目', None, None, json.dumps(paylo…，更新脚本状态。
                 '基础概念', '虚构题目', None, None, json.dumps(payload, ensure_ascii=False), None, 0, 'ai'))

    # 【逐行批注】 定义函数 answer，封装可复用的处理步骤。
    def answer(self, answers):
        # 【逐行批注】 结束当前函数并返回结果：return self.request('/api/quiz-attempts', 'POST', {'courseId': 'ficti…。
        return self.request('/api/quiz-attempts', 'POST', {'courseId': 'fictional-course',
            # 【逐行批注】 执行当前 Python 语句：'phaseIndex': 1, 'topicIndex': 1, 'answers': answers})。
            'phaseIndex': 1, 'topicIndex': 1, 'answers': answers})

    # 【逐行批注】 定义函数 settled，封装可复用的处理步骤。
    def settled(self):
        # 【逐行批注】 结束当前函数并返回结果：return wait_for(lambda: (value if not value['abilityStatus']['updatin…。
        return wait_for(lambda: (value if not value['abilityStatus']['updating'] else None)
            # 【逐行批注】 判断条件 (value := self.request('/api/profile')) else None)，成立时执行对应分支。
            if (value := self.request('/api/profile')) else None)

# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable):
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-radar-') as directory:。
    with tempfile.TemporaryDirectory(prefix='gangyi-radar-') as directory:
        # 【逐行批注】 计算右侧结果并赋给 h，更新脚本状态。
        h = Harness(executable, directory)
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 empty，更新脚本状态。
            empty = h.settled()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(empty['abilities']) == 6 and all(x['score'] is None for x in empty['…。
            assert len(empty['abilities']) == 6 and all(x['score'] is None for x in empty['abilities'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert h.calls == []。
            assert h.calls == []
            # 【逐行批注】 计算右侧结果并赋给 preferences，更新脚本状态。
            preferences = {'mode': 'abilities', 'subjects': ['政治', '历史', '地理', '语文', '数学', '英语']}
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert h.request('/api/profile/radar-preferences', 'PUT', preferences) == prefe…。
            assert h.request('/api/profile/radar-preferences', 'PUT', preferences) == preferences
            # 【逐行批注】 遍历数据并重复执行循环体：for bad in [dict(preferences, subjects=SUBJECTS[:5]), dict(preferences, su…。
            for bad in [dict(preferences, subjects=SUBJECTS[:5]), dict(preferences, subjects=['数学'] * 6),
                        # 【逐行批注】 计算右侧结果并赋给 dict(preferences, subjects，更新脚本状态。
                        dict(preferences, subjects=SUBJECTS[:5] + ['大学数学']), dict(preferences, mode='invalid')]:
                # 【逐行批注】 执行当前 Python 语句：h.request('/api/profile/radar-preferences', 'PUT', bad, 400)。
                h.request('/api/profile/radar-preferences', 'PUT', bad, 400)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert h.request('/api/profile/radar-preferences') == preferences。
            assert h.request('/api/profile/radar-preferences') == preferences
            # 数据库暂时被占用超过忙等待时间，后台线程仍应恢复处理后续作答。
            # 【逐行批注】 在受控上下文中执行资源操作：with h.db() as locked:。
            with h.db() as locked:
                # 【逐行批注】 执行当前 Python 语句：locked.execute('BEGIN EXCLUSIVE')。
                locked.execute('BEGIN EXCLUSIVE')
                # 【逐行批注】 执行当前 Python 语句：time.sleep(8.2)。
                time.sleep(8.2)
                # 【逐行批注】 执行当前 Python 语句：locked.commit()。
                locked.commit()
            # 【逐行批注】 计算右侧结果并赋给 h.seed_quiz(repeated，更新脚本状态。
            h.seed_quiz(repeated=True)
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(3): h.answer(['unknown', 'unknown', 'unknown'])。
            for _ in range(3): h.answer(['unknown', 'unknown', 'unknown'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(x['score'] is None for x in h.settled()['abilities']) and not h.calls。
            assert all(x['score'] is None for x in h.settled()['abilities']) and not h.calls
            # 【逐行批注】 执行当前 Python 语句：h.seed_quiz()。
            h.seed_quiz()
            # 【逐行批注】 执行当前 Python 语句：h.answer([None, None, None])。
            h.answer([None, None, None])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(x['score'] is None for x in h.settled()['abilities']) and not h.calls。
            assert all(x['score'] is None for x in h.settled()['abilities']) and not h.calls
            # 【逐行批注】 执行当前 Python 语句：h.answer(['unknown', 'unknown', 'unknown'])。
            h.answer(['unknown', 'unknown', 'unknown'])
            # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
            value = h.settled()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert value['abilities'][0]['score'] == 67 and value['abilities'][1]['score'] …。
            assert value['abilities'][0]['score'] == 67 and value['abilities'][1]['score'] == 0
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert value['abilityStatus']['source'] == 'ai'。
            assert value['abilityStatus']['source'] == 'ai'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(task['unknown'] for task in h.calls[-1]['tasks'])。
            assert all(task['unknown'] for task in h.calls[-1]['tasks'])
            # 【逐行批注】 在受控上下文中执行资源操作：with h.db() as db:。
            with h.db() as db:
                # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
                result = json.loads(db.execute("SELECT payload FROM LearningInteraction WHERE kind='question-evaluation' ORDER BY rowid DESC LIMIT 1").fetchone()[0])
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(x['questionSnapshot']['question'] and x['questionId'] for x in resul…。
                assert all(x['questionSnapshot']['question'] and x['questionId'] for x in result['results'])
            # 【逐行批注】 计算右侧结果并赋给 count，更新脚本状态。
            count = len(h.calls)
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(5): h.request('/api/profile')。
            for _ in range(5): h.request('/api/profile')
            # 【逐行批注】 计算右侧结果并赋给 h.request('/api/profile/radar-preferences', …，更新脚本状态。
            h.request('/api/profile/radar-preferences', 'PUT', dict(preferences, mode='subjects'))
            # 【逐行批注】 执行当前 Python 语句：time.sleep(2.2)。
            time.sleep(2.2)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(h.calls) == count。
            assert len(h.calls) == count
            # 【逐行批注】 计算右侧结果并赋给 old，更新脚本状态。
            old = value['abilities']
            # 【逐行批注】 遍历数据并重复执行循环体：for mode in ['missing', 'few', 'duplicate', 'out-of-range', 'expression', …。
            for mode in ['missing', 'few', 'duplicate', 'out-of-range', 'expression', 'fail']:
                # 【逐行批注】 计算右侧结果并赋给 h.mode，更新脚本状态。
                h.mode = mode
                # 【逐行批注】 执行当前 Python 语句：h.request('/api/profile/refresh', 'POST', {}, 202)。
                h.request('/api/profile/refresh', 'POST', {}, 202)
                # 【逐行批注】 计算右侧结果并赋给 failed，更新脚本状态。
                failed = h.settled()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert failed['abilities'] == old and failed['abilityStatus']['status'] == 'wai…。
                assert failed['abilities'] == old and failed['abilityStatus']['status'] == 'waiting', mode
                # 【逐行批注】 计算右侧结果并赋给 count，更新脚本状态。
                count = len(h.calls)
                # 【逐行批注】 执行当前 Python 语句：time.sleep(2.2)。
                time.sleep(2.2)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(h.calls) == count, '相同失败版本不得自动重试'。
                assert len(h.calls) == count, '相同失败版本不得自动重试'
            # 【逐行批注】 计算右侧结果并赋给 h.mode, h.score，更新脚本状态。
            h.mode, h.score = 'blocked', 31
            # 【逐行批注】 执行当前 Python 语句：h.request('/api/profile/refresh', 'POST', {}, 202)。
            h.request('/api/profile/refresh', 'POST', {}, 202)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert h.entered.wait(8)。
            assert h.entered.wait(8)
            # 【逐行批注】 执行当前 Python 语句：h.answer([0, 0, 0])。
            h.answer([0, 0, 0])
            # 【逐行批注】 计算右侧结果并赋给 h.mode, h.score，更新脚本状态。
            h.mode, h.score = 'valid', 84
            # 【逐行批注】 执行当前 Python 语句：h.release.set()。
            h.release.set()
            # 【逐行批注】 计算右侧结果并赋给 final，更新脚本状态。
            final = h.settled()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert final['abilities'][0]['score'] == 84, '旧请求不得覆盖新证据版本'。
            assert final['abilities'][0]['score'] == 84, '旧请求不得覆盖新证据版本'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(task['answer'] == 0 and not task['unknown'] for task in h.calls[-1][…。
            assert all(task['answer'] == 0 and not task['unknown'] for task in h.calls[-1]['tasks'])
            # 【逐行批注】 计算右侧结果并赋给 count，更新脚本状态。
            count = len(h.calls)
            # 【逐行批注】 执行当前 Python 语句：h.stop(); h.start()。
            h.stop(); h.start()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert h.settled()['abilities'] == final['abilities'] and len(h.calls) == count。
            assert h.settled()['abilities'] == final['abilities'] and len(h.calls) == count
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert h.request('/api/profile/radar-preferences')['subjects'] == preferences['…。
            assert h.request('/api/profile/radar-preferences')['subjects'] == preferences['subjects']
            # 课堂记录含原题与真实开放答案；失败评价和跳过仍不能形成能力证据。
            # 【逐行批注】 计算右侧结果并赋给 h.mode，更新脚本状态。
            h.mode = 'complete'
            # 【逐行批注】 在受控上下文中执行资源操作：with h.db() as db:。
            with h.db() as db:
                # 【逐行批注】 遍历数据并重复执行循环体：for i in range(9):。
                for i in range(9):
                    # 【逐行批注】 计算右侧结果并赋给 item，更新脚本状态。
                    item = dict(status='answered' if i < 6 else 'skipped', credible=i < 3 or i >= 6,
                        # 【逐行批注】 计算右侧结果并赋给 type，更新脚本状态。
                        type='open', question=f'虚构新情境综合任务{i}：解释概念并写出方法和推导过程。',
                        # 【逐行批注】 计算右侧结果并赋给 rubric，更新脚本状态。
                        rubric='写出概念、理由和应用过程', answer='先说明概念，再根据条件推导并迁移到新情境。',
                        # 【逐行批注】 计算右侧结果并赋给 correct，更新脚本状态。
                        correct=True, unknown=False)
                    # 【逐行批注】 调用 VALUES 完成当前脚本或测试步骤：db.execute('INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)',。
                    db.execute('INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)',
                        # 【逐行批注】 执行当前 Python 语句：(f'open-task-{i}', 'fictional-course', 1, 1, 'interaction', json.dumps(item), '2099-01-01…。
                        (f'open-task-{i}', 'fictional-course', 1, 1, 'interaction', json.dumps(item), '2099-01-01T00:00:00Z'))
            # 【逐行批注】 执行当前 Python 语句：h.request('/api/profile/refresh', 'POST', {}, 202)。
            h.request('/api/profile/refresh', 'POST', {}, 202)
            # 【逐行批注】 计算右侧结果并赋给 complete，更新脚本状态。
            complete = h.settled()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert [x['score'] for x in complete['abilities']] == [55, 0, 72, 44, 63, 41]。
            assert [x['score'] for x in complete['abilities']] == [55, 0, 72, 44, 63, 41]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(h.calls[-1]['tasks']) == 6, '失败评价和跳过不得作为证据'。
            assert len(h.calls[-1]['tasks']) == 6, '失败评价和跳过不得作为证据'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(x['evidenceCount'] == 3 for x in complete['abilities'])。
            assert all(x['evidenceCount'] == 3 for x in complete['abilities'])
            # 恢复已中断的在途评估：保留旧画像，同一版本不再调用模型。
            # 【逐行批注】 计算右侧结果并赋给 count，更新脚本状态。
            count = len(h.calls); h.stop()
            # 【逐行批注】 在受控上下文中执行资源操作：with h.db() as db:。
            with h.db() as db:
                # 【逐行批注】 计算右侧结果并赋给 revision，更新脚本状态。
                revision = int(db.execute("SELECT value FROM ProfileMeta WHERE key='revision'").fetchone()[0]) + 1
                # 【逐行批注】 计算右侧结果并赋给 db.execute("UPDATE ProfileMeta SET value，更新脚本状态。
                db.execute("UPDATE ProfileMeta SET value=? WHERE key='revision'", (str(revision),))
                # 【逐行批注】 计算右侧结果并赋给 stored，更新脚本状态。
                stored = json.loads(db.execute("SELECT value FROM ProfileMeta WHERE key='ability-profile'").fetchone()[0])
                # 【逐行批注】 计算右侧结果并赋给 stored['pendingVersion']，更新脚本状态。
                stored['pendingVersion'] = revision
                # 【逐行批注】 计算右侧结果并赋给 db.execute("UPDATE ProfileMeta SET value，更新脚本状态。
                db.execute("UPDATE ProfileMeta SET value=? WHERE key='ability-profile'", (json.dumps(stored),))
                # 【逐行批注】 计算右侧结果并赋给 db.execute("INSERT INTO ProfileMeta VALUES('…，更新脚本状态。
                db.execute("INSERT INTO ProfileMeta VALUES('profile-attempt-revision',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value", (str(revision),))
            # 【逐行批注】 计算右侧结果并赋给 h.start(); interrupted，更新脚本状态。
            h.start(); interrupted = h.settled()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert interrupted['abilities'] == complete['abilities'] and interrupted['abili…。
            assert interrupted['abilities'] == complete['abilities'] and interrupted['abilityStatus']['status'] == 'waiting'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(h.calls) == count, '恢复时不得重复请求已中断版本'。
            assert len(h.calls) == count, '恢复时不得重复请求已中断版本'
            # 【逐行批注】 执行当前 Python 语句：h.request('/api/profile/refresh', 'POST', {}, 202)。
            h.request('/api/profile/refresh', 'POST', {}, 202)
            # 【逐行批注】 计算右侧结果并赋给 retried，更新脚本状态。
            retried = h.settled()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert [x['score'] for x in retried['abilities']] == [x['score'] for x in compl…。
            assert [x['score'] for x in retried['abilities']] == [x['score'] for x in complete['abilities']] and len(h.calls) == count + 1
            # 【逐行批注】 输出当前运行结果：print('六维画像：可靠证据、独立 AI、门槛校验、失败保留、版本冲突与重启保留通过')。
            print('六维画像：可靠证据、独立 AI、门槛校验、失败保留、版本冲突与重启保留通过')
        # 【逐行批注】 执行当前 Python 语句：finally: h.close()。
        finally: h.close()

# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encod…，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main、Path 完成当前脚本或测试步骤：main(Path(sys.argv[1]))。
    main(Path(sys.argv[1]))

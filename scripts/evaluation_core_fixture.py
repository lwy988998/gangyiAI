# 【逐行批注】 执行当前 Python 语句："""通过两版完整产品的同一原生API运行三组公开模拟流程。"""。
"""通过两版完整产品的同一原生API运行三组公开模拟流程。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import io。
import io
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import socket。
import socket
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sqlite3。
import sqlite3
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：from http.server import ThreadingHTTPServer。
from http.server import ThreadingHTTPServer
# 【逐行批注】 导入后续逻辑需要的模块或符号：from threading import Thread。
from threading import Thread
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen

# 【逐行批注】 导入后续逻辑需要的模块或符号：from evaluation_guard import digest, file_hash, isolated_output, summarize。
from evaluation_guard import digest, file_hash, isolated_output, summarize

# 【逐行批注】 调用 str、Path 完成当前脚本或测试步骤：sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tests'))。
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tests'))
# 【逐行批注】 导入后续逻辑需要的模块或符号：from classroom_v560_fixture import FixtureAI。
from classroom_v560_fixture import FixtureAI

# 【逐行批注】 计算右侧结果并赋给 MATERIAL，更新脚本状态。
MATERIAL = {'chat': '以后请先举例，再讲定义', 'goal': '比较化合价与电子变化',
            # 【逐行批注】 执行当前 Python 语句：'source': {'url': 'https://example.org/evaluation-snapshot', 'title': '固定离线原文'},。
            'source': {'url': 'https://example.org/evaluation-snapshot', 'title': '固定离线原文'},
            # 【逐行批注】 执行当前 Python 语句：'snapshot': {'checkedAt': '2026-10-19T00:00:00Z', 'publishedAt': None,。
            'snapshot': {'checkedAt': '2026-10-19T00:00:00Z', 'publishedAt': None,
                         # 【逐行批注】 执行当前 Python 语句：'segments': [{'id': 's1', 'text': '化合价升高对应失去电子，比较同一种元素的前后状态。'}]}}。
                         'segments': [{'id': 's1', 'text': '化合价升高对应失去电子，比较同一种元素的前后状态。'}]}}


# 【逐行批注】 定义函数 run，封装可复用的处理步骤。
def run(executable, output, source_commit):
    # 【逐行批注】 计算右侧结果并赋给 output，更新脚本状态。
    output = isolated_output(output)
    # 【逐行批注】 计算右侧结果并赋给 output.mkdir(parents，更新脚本状态。
    output.mkdir(parents=True, exist_ok=False)
    # 【逐行批注】 计算右侧结果并赋给 requests，更新脚本状态。
    requests = []

    # 【逐行批注】 定义类 CoreFixture，组织相关数据与行为。
    class CoreFixture(FixtureAI):
        # 【逐行批注】 计算右侧结果并赋给 source_id，更新脚本状态。
        source_id = ''

        # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
        def do_POST(self):
            # 【逐行批注】 计算右侧结果并赋给 raw，更新脚本状态。
            raw = self.rfile.read(int(self.headers['Content-Length']))
            # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
            body = json.loads(raw)
            # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
            context = None
            # 【逐行批注】 遍历数据并重复执行循环体：for message in body['messages']:。
            for message in body['messages']:
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 计算右侧结果并赋给 candidate，更新脚本状态。
                    candidate = json.loads(message.get('content', ''))
                # 【逐行批注】 捕获并处理异常：except (ValueError, TypeError):。
                except (ValueError, TypeError):
                    # 【逐行批注】 执行当前 Python 语句：continue。
                    continue
                # 【逐行批注】 判断条件 isinstance(candidate, dict) and isinstance(candidate.get('event'…，成立时执行对应分支。
                if isinstance(candidate, dict) and isinstance(candidate.get('event'), dict):
                    # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
                    context = candidate
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
            # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
            event = context['event'] if context else {'type': 'unknown'}
            # 【逐行批注】 执行当前 Python 语句：requests.append({'eventType': event['type'], 'request': body,。
            requests.append({'eventType': event['type'], 'request': body,
                             # 【逐行批注】 调用 sum、len 完成当前脚本或测试步骤：'inputBytes': sum(len(m.get('content', '').encode('utf-8')) for m in …。
                             'inputBytes': sum(len(m.get('content', '').encode('utf-8')) for m in body['messages'])})
            # 【逐行批注】 计算右侧结果并赋给 actions, message, state，更新脚本状态。
            actions, message, state = [], '', 'completed'
            # 【逐行批注】 判断条件 event['type'] == 'assistant_chat':，成立时执行对应分支。
            if event['type'] == 'assistant_chat':
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [{'id': 'fixed-style', 'tool': 'set_teaching_preferences',
                            # 【逐行批注】 执行当前 Python 语句：'args': {'style': '先举生活中的例子，再说明概念'}}]。
                            'args': {'style': '先举生活中的例子，再说明概念'}}]
                # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
                message = '已保存先举例再讲定义的偏好。'
            # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'library_research':。
            elif event['type'] == 'library_research':
                # 【逐行批注】 判断条件 not any('toolResults' in m.get('content', '') for m in body['mes…，成立时执行对应分支。
                if not any('toolResults' in m.get('content', '') for m in body['messages'][2:]):
                    # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                    actions = [{'id': 'fixed-read', 'tool': 'read_resource',
                                # 【逐行批注】 执行当前 Python 语句：'args': {'sourceId': self.source_id, 'snapshotId': 'evaluation-fixed-snapshot'}}]。
                                'args': {'sourceId': self.source_id, 'snapshotId': 'evaluation-fixed-snapshot'}}]
                    # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                    state = 'continue'
                # 【逐行批注】 执行前一条件不成立时的分支。
                else:
                    # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
                    message = '已读取同份固定原文，缺少外部资料的部分保持未知。'
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 self.rfile，更新脚本状态。
                self.rfile = io.BytesIO(raw)
                # 【逐行批注】 结束当前函数并返回结果：return super().do_POST()。
                return super().do_POST()
            # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
            value = json.dumps({'message': message, 'actions': actions, 'state': state}, ensure_ascii=False)
            # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
            self.send_response(200)
            # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'text/event-stream')。
            self.send_header('Content-Type', 'text/event-stream')
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 计算右侧结果并赋给 frame，更新脚本状态。
            frame = {'model': 'evaluation-core-fixture', 'choices': [{'delta': {'content': value}, 'finish_reason': None}]}
            # 【逐行批注】 计算右侧结果并赋给 self.wfile.write(('data: '+json.dumps(frame,…，更新脚本状态。
            self.wfile.write(('data: '+json.dumps(frame, ensure_ascii=False)+'\n\ndata: [DONE]\n\n').encode('utf-8'))

    # 【逐行批注】 计算右侧结果并赋给 server，更新脚本状态。
    server = ThreadingHTTPServer(('127.0.0.1', 0), CoreFixture)
    # 【逐行批注】 计算右侧结果并赋给 Thread(target，更新脚本状态。
    Thread(target=server.serve_forever, daemon=True).start()
    # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as sock:。
    with socket.socket() as sock:
        # 【逐行批注】 执行当前 Python 语句：sock.bind(('127.0.0.1', 0))。
        sock.bind(('127.0.0.1', 0))
        # 【逐行批注】 计算右侧结果并赋给 port，更新脚本状态。
        port = sock.getsockname()[1]
    # 【逐行批注】 计算右侧结果并赋给 base，更新脚本状态。
    base = f'http://127.0.0.1:{port}'
    # 【逐行批注】 计算右侧结果并赋给 database，更新脚本状态。
    database = output / 'isolated.db'
    # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
    env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(database),
               # 【逐行批注】 计算右侧结果并赋给 AI_BASE_URL，更新脚本状态。
               AI_BASE_URL=f'http://127.0.0.1:{server.server_port}/v1', AI_MODEL='evaluation-core-fixture',
               # 【逐行批注】 计算右侧结果并赋给 AI_API_KEY，更新脚本状态。
               AI_API_KEY='isolated-fixture-key', BOCHA_API_KEY='', LOCAL_CONTROL_TOKEN='',
               # 【逐行批注】 计算右侧结果并赋给 GANGYI_LAUNCH_SESSION_ID，更新脚本状态。
               GANGYI_LAUNCH_SESSION_ID='evaluation-isolated')
    # 【逐行批注】 计算右侧结果并赋给 turns, complete, failure，更新脚本状态。
    turns, complete, failure = [], False, None
    # 【逐行批注】 计算右侧结果并赋给 started，更新脚本状态。
    started = time.monotonic()
    # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
    process = None

    # 【逐行批注】 定义函数 api，封装可复用的处理步骤。
    def api(path, body=None):
        # 【逐行批注】 计算右侧结果并赋给 request，更新脚本状态。
        request = Request(base+path, data=None if body is None else json.dumps(body, ensure_ascii=False).encode('utf-8'),
                          # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                          headers={'Content-Type': 'application/json'})
        # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(request, timeout=10) as response:。
        with urlopen(request, timeout=10) as response:
            # 【逐行批注】 结束当前函数并返回结果：return json.load(response)。
            return json.load(response)

    # 【逐行批注】 定义函数 checkpoint，封装可复用的处理步骤。
    def checkpoint():
        # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
        result = {'schema': 'core-api-fixture-v564-r1', 'sourceCommit': source_commit,
                  # 【逐行批注】 调用 file_hash、digest 完成当前脚本或测试步骤：'executableSha256': file_hash(executable), 'materialSha256': digest(M…。
                  'executableSha256': file_hash(executable), 'materialSha256': digest(MATERIAL),
                  # 【逐行批注】 执行当前 Python 语句：'entryProtocol': 'native-product-http-api', 'fixtureOnly': True, 'paidModelRequests': 0,。
                  'entryProtocol': 'native-product-http-api', 'fixtureOnly': True, 'paidModelRequests': 0,
                  # 【逐行批注】 执行当前 Python 语句：'turns': turns, 'complete': complete, 'failure': failure,。
                  'turns': turns, 'complete': complete, 'failure': failure,
                  # 【逐行批注】 调用 round 完成当前脚本或测试步骤：'externalElapsedMs': round((time.monotonic()-started)*1000, 3),。
                  'externalElapsedMs': round((time.monotonic()-started)*1000, 3),
                  # 【逐行批注】 执行当前 Python 语句：'externalTimingBoundary': '同一外部API提交至终态轮询返回，包含轮询间隔；并非产品内queue/firstContent/completion',。
                  'externalTimingBoundary': '同一外部API提交至终态轮询返回，包含轮询间隔；并非产品内queue/firstContent/completion',
                  # 【逐行批注】 调用 len 完成当前脚本或测试步骤：'capturedMockRequests': len(requests)}。
                  'capturedMockRequests': len(requests)}
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 result['httpRequests']，更新脚本状态。
            result['httpRequests'] = api('/api/my-ai/usage')['items']
        # 【逐行批注】 捕获并处理异常：except (OSError, ValueError):。
        except (OSError, ValueError):
            # 【逐行批注】 计算右侧结果并赋给 result['usageCollectionUnknown']，更新脚本状态。
            result['usageCollectionUnknown'] = True
        # 【逐行批注】 计算右侧结果并赋给 result['summary']，更新脚本状态。
        result['summary'] = summarize(result)
        # 【逐行批注】 计算右侧结果并赋给 temporary，更新脚本状态。
        temporary = output/'result.json.tmp'
        # 【逐行批注】 计算右侧结果并赋给 temporary.write_text(json.dumps(result, ensu…，更新脚本状态。
        temporary.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
        # 【逐行批注】 执行当前 Python 语句：temporary.replace(output/'result.json')。
        temporary.replace(output/'result.json')
        # 【逐行批注】 计算右侧结果并赋给 (output/'prompts.json').write_text(json.dump…，更新脚本状态。
        (output/'prompts.json').write_text(json.dumps(requests, ensure_ascii=False, indent=2), encoding='utf-8')
        # 【逐行批注】 结束当前函数并返回结果：return result。
        return result

    # 【逐行批注】 定义函数 wait，封装可复用的处理步骤。
    def wait(task_id):
        # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
        deadline = time.monotonic()+60
        # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
        while time.monotonic() < deadline:
            # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
            task = api('/api/learning-agent?taskId='+task_id)
            # 【逐行批注】 判断条件 task['status'] in ('ready', 'waiting_student', 'completed'):，成立时执行对应分支。
            if task['status'] in ('ready', 'waiting_student', 'completed'):
                # 【逐行批注】 结束当前函数并返回结果：return task。
                return task
            # 【逐行批注】 判断条件 task['status'] in ('failed', 'superseded', 'paused'):，成立时执行对应分支。
            if task['status'] in ('failed', 'superseded', 'paused'):
                # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('模拟流程任务失败：'+task['status'])。
                raise RuntimeError('模拟流程任务失败：'+task['status'])
            # 【逐行批注】 执行当前 Python 语句：time.sleep(.05)。
            time.sleep(.05)
        # 【逐行批注】 在错误情形抛出异常：raise TimeoutError('模拟流程任务超时')。
        raise TimeoutError('模拟流程任务超时')

    # 【逐行批注】 定义函数 execute，封装可复用的处理步骤。
    def execute(name, path, body):
        # 【逐行批注】 计算右侧结果并赋给 clock，更新脚本状态。
        clock = time.monotonic()
        # 【逐行批注】 计算右侧结果并赋给 sent，更新脚本状态。
        sent = api(path, body)
        # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
        task = wait(sent['id'])
        # 【逐行批注】 执行当前 Python 语句：turns.append({'flow': name, 'event': body, 'task': task,。
        turns.append({'flow': name, 'event': body, 'task': task,
                      # 【逐行批注】 调用 round 完成当前脚本或测试步骤：'externalApiMs': round((time.monotonic()-clock)*1000, 3)})。
                      'externalApiMs': round((time.monotonic()-clock)*1000, 3)})
        # 【逐行批注】 调用 checkpoint 完成当前脚本或测试步骤：checkpoint()。
        checkpoint()
        # 【逐行批注】 结束当前函数并返回结果：return task。
        return task

    # 【逐行批注】 在受控上下文中执行资源操作：with (output/'service.log').open('w', encoding='utf-8') as log:。
    with (output/'service.log').open('w', encoding='utf-8') as log:
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
            process = subprocess.Popen([str(Path(executable).resolve())], cwd=Path(executable).resolve().parent,
                                       # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                                       env=env, stdout=log, stderr=log,
                                       # 【逐行批注】 计算右侧结果并赋给 creationflags，更新脚本状态。
                                       creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = time.monotonic()+25
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
            while time.monotonic() < deadline:
                # 【逐行批注】 判断条件 process.poll() is not None:，成立时执行对应分支。
                if process.poll() is not None:
                    # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('隔离产品启动失败')。
                    raise RuntimeError('隔离产品启动失败')
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 计算右侧结果并赋给 initial，更新脚本状态。
                    initial = api('/api/learning-agent')
                    # 【逐行批注】 判断条件 initial.get('id'):，成立时执行对应分支。
                    if initial.get('id'):
                        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(initial['id'])。
                        wait(initial['id'])
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
                # 【逐行批注】 捕获并处理异常：except OSError:。
                except OSError:
                    # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                    time.sleep(.1)
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 在错误情形抛出异常：raise TimeoutError('隔离产品启动超时')。
                raise TimeoutError('隔离产品启动超时')
            # 【逐行批注】 计算右侧结果并赋给 chat，更新脚本状态。
            chat = api('/api/my-ai/chats', {'title': '固定管家对照'})
            # 【逐行批注】 调用 execute 完成当前脚本或测试步骤：execute('assistant', '/api/my-ai/chat/send', {'id': chat['id'], 'text…。
            execute('assistant', '/api/my-ai/chat/send', {'id': chat['id'], 'text': MATERIAL['chat'], 'requestId': 'fixed-chat'})
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '先举生活' in api('/api/my-ai/preferences')['style']。
            assert '先举生活' in api('/api/my-ai/preferences')['style']
            # 【逐行批注】 计算右侧结果并赋给 course，更新脚本状态。
            course = execute('plan', '/api/generate-plan', {'goal': MATERIAL['goal'], 'mode': 'lite', 'requestId': 'fixed-plan'})['course']['id']
            # 【逐行批注】 计算右侧结果并赋给 prepared，更新脚本状态。
            prepared = execute('prepare', '/api/learning-agent/events', {'type': 'prepare_next', 'courseId': course, 'navigate': False, 'requestId': 'fixed-prepare'})
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert prepared.get('lesson', {}).get('id'), '备课未保存课程板块'。
            assert prepared.get('lesson', {}).get('id'), '备课未保存课程板块'
            # 【逐行批注】 计算右侧结果并赋给 source，更新脚本状态。
            source = api('/api/my-ai/library', MATERIAL['source'])
            # 【逐行批注】 计算右侧结果并赋给 CoreFixture.source_id，更新脚本状态。
            CoreFixture.source_id = source['id']
            # 只写隔离产品数据库；两版接收同一已保存原文快照，绝不访问外部资料。
            # 【逐行批注】 在受控上下文中执行资源操作：with sqlite3.connect(database) as db:。
            with sqlite3.connect(database) as db:
                # 【逐行批注】 计算右侧结果并赋给 snapshot，更新脚本状态。
                snapshot = dict(MATERIAL['snapshot'], sourceId=source['id'])
                # 【逐行批注】 计算右侧结果并赋给 db.execute('INSERT INTO MyAIRecord VALUES(?,…，更新脚本状态。
                db.execute('INSERT INTO MyAIRecord VALUES(?,?,?,?,?)', ('evaluation-fixed-snapshot', 'library-snapshot', json.dumps(snapshot, ensure_ascii=False), '2026-10-19', 1))
                # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
                row = db.execute('SELECT payload FROM MyAIRecord WHERE id=?', (source['id'],)).fetchone()
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload = dict(json.loads(row[0]), snapshotId='evaluation-fixed-snapshot', readStatus='read')
                # 【逐行批注】 计算右侧结果并赋给 db.execute('UPDATE MyAIRecord SET payload，更新脚本状态。
                db.execute('UPDATE MyAIRecord SET payload=? WHERE id=?', (json.dumps(payload, ensure_ascii=False), source['id']))
            # 【逐行批注】 计算右侧结果并赋给 research，更新脚本状态。
            research = execute('research', '/api/my-ai/library/research', {'goal': '读取固定化合价原文并检查依据', 'courseId': '', 'requestId': 'fixed-research'})
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert any(event.get('type') == 'action' and event.get('activity', {}).get('too…。
            assert any(event.get('type') == 'action' and event.get('activity', {}).get('tool') == 'read_resource'
                       # 【逐行批注】 执行当前 Python 语句：and event['activity'].get('state') == 'completed' for event in research.get('events', [])…。
                       and event['activity'].get('state') == 'completed' for event in research.get('events', [])), '资料流程未保存产品read_resource工具回执'
            # 【逐行批注】 计算右侧结果并赋给 library_usage，更新脚本状态。
            library_usage = api('/api/my-ai/library/usage')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert library_usage['searchRequests'] == library_usage['fetchRequests'] == 0。
            assert library_usage['searchRequests'] == library_usage['fetchRequests'] == 0
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(api('/api/my-ai/usage')['items']) == len(requests), '模拟服务请求与逐笔HTTP账目…。
            assert len(api('/api/my-ai/usage')['items']) == len(requests), '模拟服务请求与逐笔HTTP账目不一致'
            # 【逐行批注】 计算右侧结果并赋给 complete，更新脚本状态。
            complete = True
        # 【逐行批注】 捕获并处理异常：except Exception as error:。
        except Exception as error:
            # 【逐行批注】 计算右侧结果并赋给 failure，更新脚本状态。
            failure = type(error).__name__+': '+str(error)
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
            result = checkpoint()
            # 【逐行批注】 判断条件 process and process.poll() is None:，成立时执行对应分支。
            if process and process.poll() is None:
                # 【逐行批注】 执行当前 Python 语句：process.terminate()。
                process.terminate()
                # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                process.wait(timeout=20)
            # 【逐行批注】 执行当前 Python 语句：server.shutdown()。
            server.shutdown()
            # 【逐行批注】 执行当前 Python 语句：server.server_close()。
            server.server_close()
    # 【逐行批注】 结束当前函数并返回结果：return result。
    return result

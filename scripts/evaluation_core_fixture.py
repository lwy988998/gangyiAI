"""通过两版完整产品的同一原生API运行三组公开模拟流程。"""
import io
import json
import os
from pathlib import Path
import socket
import sqlite3
import subprocess
import sys
import time
from http.server import ThreadingHTTPServer
from threading import Thread
from urllib.request import Request, urlopen

from evaluation_guard import digest, file_hash, isolated_output, summarize

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tests'))
from classroom_v560_fixture import FixtureAI

MATERIAL = {'chat': '以后请先举例，再讲定义', 'goal': '比较化合价与电子变化',
            'source': {'url': 'https://example.org/evaluation-snapshot', 'title': '固定离线原文'},
            'snapshot': {'checkedAt': '2026-10-19T00:00:00Z', 'publishedAt': None,
                         'segments': [{'id': 's1', 'text': '化合价升高对应失去电子，比较同一种元素的前后状态。'}]}}


def run(executable, output, source_commit):
    output = isolated_output(output)
    output.mkdir(parents=True, exist_ok=False)
    requests = []

    class CoreFixture(FixtureAI):
        source_id = ''

        def do_POST(self):
            raw = self.rfile.read(int(self.headers['Content-Length']))
            body = json.loads(raw)
            context = None
            for message in body['messages']:
                try:
                    candidate = json.loads(message.get('content', ''))
                except (ValueError, TypeError):
                    continue
                if isinstance(candidate, dict) and isinstance(candidate.get('event'), dict):
                    context = candidate
                    break
            event = context['event'] if context else {'type': 'unknown'}
            requests.append({'eventType': event['type'], 'request': body,
                             'inputBytes': sum(len(m.get('content', '').encode('utf-8')) for m in body['messages'])})
            actions, message, state = [], '', 'completed'
            if event['type'] == 'assistant_chat':
                actions = [{'id': 'fixed-style', 'tool': 'set_teaching_preferences',
                            'args': {'style': '先举生活中的例子，再说明概念'}}]
                message = '已保存先举例再讲定义的偏好。'
            elif event['type'] == 'library_research':
                if not any('toolResults' in m.get('content', '') for m in body['messages'][2:]):
                    actions = [{'id': 'fixed-read', 'tool': 'read_resource',
                                'args': {'sourceId': self.source_id, 'snapshotId': 'evaluation-fixed-snapshot'}}]
                    state = 'continue'
                else:
                    message = '已读取同份固定原文，缺少外部资料的部分保持未知。'
            else:
                self.rfile = io.BytesIO(raw)
                return super().do_POST()
            value = json.dumps({'message': message, 'actions': actions, 'state': state}, ensure_ascii=False)
            self.send_response(200)
            self.send_header('Content-Type', 'text/event-stream')
            self.end_headers()
            frame = {'model': 'evaluation-core-fixture', 'choices': [{'delta': {'content': value}, 'finish_reason': None}]}
            self.wfile.write(('data: '+json.dumps(frame, ensure_ascii=False)+'\n\ndata: [DONE]\n\n').encode('utf-8'))

    server = ThreadingHTTPServer(('127.0.0.1', 0), CoreFixture)
    Thread(target=server.serve_forever, daemon=True).start()
    with socket.socket() as sock:
        sock.bind(('127.0.0.1', 0))
        port = sock.getsockname()[1]
    base = f'http://127.0.0.1:{port}'
    database = output / 'isolated.db'
    env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(database),
               AI_BASE_URL=f'http://127.0.0.1:{server.server_port}/v1', AI_MODEL='evaluation-core-fixture',
               AI_API_KEY='isolated-fixture-key', BOCHA_API_KEY='', LOCAL_CONTROL_TOKEN='',
               GANGYI_LAUNCH_SESSION_ID='evaluation-isolated')
    turns, complete, failure = [], False, None
    started = time.monotonic()
    process = None

    def api(path, body=None):
        request = Request(base+path, data=None if body is None else json.dumps(body, ensure_ascii=False).encode('utf-8'),
                          headers={'Content-Type': 'application/json'})
        with urlopen(request, timeout=10) as response:
            return json.load(response)

    def checkpoint():
        result = {'schema': 'core-api-fixture-v564-r1', 'sourceCommit': source_commit,
                  'executableSha256': file_hash(executable), 'materialSha256': digest(MATERIAL),
                  'entryProtocol': 'native-product-http-api', 'fixtureOnly': True, 'paidModelRequests': 0,
                  'turns': turns, 'complete': complete, 'failure': failure,
                  'externalElapsedMs': round((time.monotonic()-started)*1000, 3),
                  'externalTimingBoundary': '同一外部API提交至终态轮询返回，包含轮询间隔；并非产品内queue/firstContent/completion',
                  'capturedMockRequests': len(requests)}
        try:
            result['httpRequests'] = api('/api/my-ai/usage')['items']
        except (OSError, ValueError):
            result['usageCollectionUnknown'] = True
        result['summary'] = summarize(result)
        temporary = output/'result.json.tmp'
        temporary.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
        temporary.replace(output/'result.json')
        (output/'prompts.json').write_text(json.dumps(requests, ensure_ascii=False, indent=2), encoding='utf-8')
        return result

    def wait(task_id):
        deadline = time.monotonic()+60
        while time.monotonic() < deadline:
            task = api('/api/learning-agent?taskId='+task_id)
            if task['status'] in ('ready', 'waiting_student', 'completed'):
                return task
            if task['status'] in ('failed', 'superseded', 'paused'):
                raise RuntimeError('模拟流程任务失败：'+task['status'])
            time.sleep(.05)
        raise TimeoutError('模拟流程任务超时')

    def execute(name, path, body):
        clock = time.monotonic()
        sent = api(path, body)
        task = wait(sent['id'])
        turns.append({'flow': name, 'event': body, 'task': task,
                      'externalApiMs': round((time.monotonic()-clock)*1000, 3)})
        checkpoint()
        return task

    with (output/'service.log').open('w', encoding='utf-8') as log:
        try:
            process = subprocess.Popen([str(Path(executable).resolve())], cwd=Path(executable).resolve().parent,
                                       env=env, stdout=log, stderr=log,
                                       creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
            deadline = time.monotonic()+25
            while time.monotonic() < deadline:
                if process.poll() is not None:
                    raise RuntimeError('隔离产品启动失败')
                try:
                    initial = api('/api/learning-agent')
                    if initial.get('id'):
                        wait(initial['id'])
                    break
                except OSError:
                    time.sleep(.1)
            else:
                raise TimeoutError('隔离产品启动超时')
            chat = api('/api/my-ai/chats', {'title': '固定管家对照'})
            execute('assistant', '/api/my-ai/chat/send', {'id': chat['id'], 'text': MATERIAL['chat'], 'requestId': 'fixed-chat'})
            assert '先举生活' in api('/api/my-ai/preferences')['style']
            course = execute('plan', '/api/generate-plan', {'goal': MATERIAL['goal'], 'mode': 'lite', 'requestId': 'fixed-plan'})['course']['id']
            prepared = execute('prepare', '/api/learning-agent/events', {'type': 'prepare_next', 'courseId': course, 'navigate': False, 'requestId': 'fixed-prepare'})
            assert prepared.get('lesson', {}).get('id'), '备课未保存课程板块'
            source = api('/api/my-ai/library', MATERIAL['source'])
            CoreFixture.source_id = source['id']
            # 只写隔离产品数据库；两版接收同一已保存原文快照，绝不访问外部资料。
            with sqlite3.connect(database) as db:
                snapshot = dict(MATERIAL['snapshot'], sourceId=source['id'])
                db.execute('INSERT INTO MyAIRecord VALUES(?,?,?,?,?)', ('evaluation-fixed-snapshot', 'library-snapshot', json.dumps(snapshot, ensure_ascii=False), '2026-10-19', 1))
                row = db.execute('SELECT payload FROM MyAIRecord WHERE id=?', (source['id'],)).fetchone()
                payload = dict(json.loads(row[0]), snapshotId='evaluation-fixed-snapshot', readStatus='read')
                db.execute('UPDATE MyAIRecord SET payload=? WHERE id=?', (json.dumps(payload, ensure_ascii=False), source['id']))
            research = execute('research', '/api/my-ai/library/research', {'goal': '读取固定化合价原文并检查依据', 'courseId': '', 'requestId': 'fixed-research'})
            assert any(event.get('type') == 'action' and event.get('activity', {}).get('tool') == 'read_resource'
                       and event['activity'].get('state') == 'completed' for event in research.get('events', [])), '资料流程未保存产品read_resource工具回执'
            library_usage = api('/api/my-ai/library/usage')
            assert library_usage['searchRequests'] == library_usage['fetchRequests'] == 0
            assert len(api('/api/my-ai/usage')['items']) == len(requests), '模拟服务请求与逐笔HTTP账目不一致'
            complete = True
        except Exception as error:
            failure = type(error).__name__+': '+str(error)
        finally:
            result = checkpoint()
            if process and process.poll() is None:
                process.terminate()
                process.wait(timeout=20)
            server.shutdown()
            server.server_close()
    return result

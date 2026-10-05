"""用隔离虚构档案验证能力评估、逐维证据、失败保留和版本保护。"""
import json
import os
import socket
import sqlite3
import subprocess
import sys
import tempfile
import threading
import time
import urllib.error
import urllib.request
from contextlib import contextmanager
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

IDS = ['memory', 'understanding', 'application', 'reasoning', 'expression', 'transfer']
NAMES = ['知识记忆', '概念理解', '方法应用', '逻辑推理', '表达说明', '综合迁移']
SUBJECTS = ['语文', '数学', '英语', '物理', '化学', '生物']

def wait_for(predicate, seconds=20):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        try:
            value = predicate()
            if value:
                return value
        except (OSError, ValueError):
            pass
        time.sleep(.1)
    raise AssertionError('画像未在期限内达到预期状态')

class Harness:
    def __init__(self, executable, root, unified=False):
        self.executable, self.root = Path(executable).resolve(), Path(root)
        self.calls = []
        self.mode = 'valid'
        self.score = 67
        self.entered = threading.Event()
        self.release = threading.Event()
        harness = self
        class Model(BaseHTTPRequestHandler):
            def log_message(self, *_):
                pass
            def do_POST(self):
                body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
                system = body['messages'][0]['content']
                text = body['messages'][-1]['content']
                data = json.loads(text) if text.startswith('{') else {}
                if '课堂作答评价教师' in system:
                    given = data['givenAnswer']
                    unknown = '不会' in data.get('studentAnswer', '')
                    result = {'isAnswer': True, 'correct': isinstance(given, int) and given == data['question'].get('answerIndex'),
                        'unknown': unknown, 'confidence': .95, 'feedback': '根据本轮实际回答核对概念；未写过程时依据不足。',
                        'misconception': '', 'methodAnalysis': '学生未提供推导过程，不能推测具体方法。'}
                elif '课堂教师' in system:
                    result = '依据你的本轮回答说明概念，并结合原题继续讲解。'
                elif '六维学习能力评估教师' in system:
                    mode, score = harness.mode, harness.score
                    harness.calls.append(data)
                    tasks = data['tasks']
                    selected = [task for task in tasks if task['question'].get('type') == 'open'] if mode == 'complete' else tasks
                    references = [task['id'] for task in selected[:3]]
                    if mode == 'blocked':
                        harness.entered.set()
                        harness.release.wait(12)
                    if mode == 'missing': references = ['不存在的题目'] * 3
                    if mode == 'few': references = references[:2]
                    if mode == 'duplicate': references = [references[0]] * 3
                    if mode == 'out-of-range': score = 101
                    result = {'abilities': [{'id': key,
                        'score': score if key == 'memory' else 0 if key == 'understanding' else None,
                        'rationale': '依据不同题目的真实作答判断已测任务表现。',
                        'recommendation': '<img src=x onerror=alert(1)>先复习概念，再解释一次。',
                        'evidenceIds': references if key in ('memory', 'understanding') else []} for key in IDS]}
                    if mode == 'expression':
                        result['abilities'][4].update(score=70, evidenceIds=references)
                    if mode == 'complete':
                        for item, score in zip(result['abilities'], [55, 0, 72, 44, 63, 41]):
                            item.update(score=score, evidenceIds=references)
                    if mode == 'fail':
                        self.send_response(503); self.end_headers(); return
                elif '学习诊断教师' in system:
                    events = [event for event in data['events'] if event['kind'] in ('quiz', 'question-evaluation')]
                    result = {'score': 73, 'rationale': '三道虚构题目反馈', 'weakPoints': [],
                              'recommendation': '先复习基础', 'evidenceIds': [events[-1]['id']], 'nextReviewAt': ''}
                elif '画像评估 AI' in system:
                    result = {'subjects': [{'subject': '数学', 'score': 73, 'rationale': '已测题目表现',
                        'weakPoints': [], 'recommendation': '继续练习',
                        'evidenceIds': data['topicStates'][0]['evidenceIds']}]}
                else:
                    result = {mode: {'continue': [f'{mode} 基础目标{i}' for i in range(3)],
                        'explore': [f'{mode} 探索目标{i}' for i in range(2)]} for mode in ('lite', 'deep')}
                payload = json.dumps({'model': 'isolated-mock', 'choices': [{'message': {
                    'content': result if isinstance(result, str) else json.dumps(result, ensure_ascii=False)}}]}, ensure_ascii=False).encode()
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Content-Length', str(len(payload)))
                self.end_headers()
                self.wfile.write(payload)
        if unified:
            from learning_agent_api_smoke import ProtocolAI
            ProtocolAI.calls.clear()
            Model = ProtocolAI
            self.calls = ProtocolAI.calls
        self.model = ThreadingHTTPServer(('127.0.0.1', 0), Model)
        threading.Thread(target=self.model.serve_forever, daemon=True).start()
        with socket.socket() as sock:
            sock.bind(('127.0.0.1', 0)); port = sock.getsockname()[1]
        self.base = f'http://127.0.0.1:{port}'
        self.database = self.root / 'fictional.db'
        self.env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(self.database),
            AI_BASE_URL=f'http://127.0.0.1:{self.model.server_port}/v1', AI_API_KEY='fictional-test-key',
            AI_MODEL='isolated-mock', BOCHA_API_KEY='', SEARCH_FALLBACK_PROVIDER='',
            RESOURCE_SEARCH_CACHE_DIR=str(self.root / 'search-cache'), LOCAL_CONTROL_TOKEN='radar-test',
            GANGYI_LAUNCH_SESSION_ID='radar-test-launch')
        self.start()

    def request(self, path, method='GET', body=None, expected=200):
        payload = None if body is None else json.dumps(body, ensure_ascii=False).encode()
        req = urllib.request.Request(self.base + path, data=payload, method=method,
            headers={'Content-Type': 'application/json', 'X-Gangyi-Control-Token': 'radar-test'})
        try:
            response = urllib.request.urlopen(req, timeout=8)
        except urllib.error.HTTPError as error:
            assert error.code == expected, (error.code, error.read())
            return json.loads(error.read())
        with response:
            assert response.status == expected
            return json.loads(response.read())

    @contextmanager
    def db(self):
        connection = sqlite3.connect(self.database, timeout=10)
        try:
            with connection: yield connection
        finally: connection.close()

    def start(self):
        self.log = (self.root / 'service.log').open('ab')
        self.process = subprocess.Popen([str(self.executable)], cwd=self.executable.parent, env=self.env,
            stdout=self.log, stderr=self.log, creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        wait_for(lambda: self.request('/api/profile'))

    def stop(self):
        try:
            self.request('/internal/shutdown', 'POST', {})
            self.process.wait(timeout=15)
        except (OSError, subprocess.TimeoutExpired):
            self.process.kill(); self.process.wait()
        self.log.close()

    def close(self):
        self.release.set(); self.stop(); self.model.shutdown(); self.model.server_close()

    def seed_quiz(self, repeated=False):
        with self.db() as db:
            db.execute('INSERT OR IGNORE INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?)',
                ('fictional-course', None, None, '数学概念与方法', 'deep', '虚构数学课程', None,
                 'ai', 'active', '2026-01-01', '2026-01-01'))
            questions = [{'question': '请判断虚构概念题' + str(0 if repeated else i),
                'options': ['甲', '乙', '丙', '丁'], 'answerIndex': 0} for i in range(3)]
            payload = {'promptVersion': 'ai-block-v1', 'blocks': {'quiz': {'quiz': questions}}}
            # 更换虚构题组只用于证据门槛测试；独立的课堂验收覆盖已展示题的稳定性。
            db.execute("DELETE FROM ClassroomActivity WHERE id LIKE 'classroom:fictional-course:1:1:quiz:%'")
            db.execute('INSERT OR REPLACE INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)',
                ('fictional-session', 'fictional-course', None, '数学概念与方法', 'deep', 1, '基础', 1,
                 '基础概念', '虚构题目', None, None, json.dumps(payload, ensure_ascii=False), None, 0, 'ai'))

    def answer(self, answers):
        return self.request('/api/quiz-attempts', 'POST', {'courseId': 'fictional-course',
            'phaseIndex': 1, 'topicIndex': 1, 'answers': answers})

    def settled(self):
        return wait_for(lambda: (value if not value['abilityStatus']['updating'] else None)
            if (value := self.request('/api/profile')) else None)

def main(executable):
    with tempfile.TemporaryDirectory(prefix='gangyi-radar-') as directory:
        h = Harness(executable, directory)
        try:
            empty = h.settled()
            assert len(empty['abilities']) == 6 and all(x['score'] is None for x in empty['abilities'])
            assert h.calls == []
            preferences = {'mode': 'abilities', 'subjects': ['政治', '历史', '地理', '语文', '数学', '英语']}
            assert h.request('/api/profile/radar-preferences', 'PUT', preferences) == preferences
            for bad in [dict(preferences, subjects=SUBJECTS[:5]), dict(preferences, subjects=['数学'] * 6),
                        dict(preferences, subjects=SUBJECTS[:5] + ['大学数学']), dict(preferences, mode='invalid')]:
                h.request('/api/profile/radar-preferences', 'PUT', bad, 400)
            assert h.request('/api/profile/radar-preferences') == preferences
            # 数据库暂时被占用超过忙等待时间，后台线程仍应恢复处理后续作答。
            with h.db() as locked:
                locked.execute('BEGIN EXCLUSIVE')
                time.sleep(8.2)
                locked.commit()
            h.seed_quiz(repeated=True)
            for _ in range(3): h.answer(['unknown', 'unknown', 'unknown'])
            assert all(x['score'] is None for x in h.settled()['abilities']) and not h.calls
            h.seed_quiz()
            h.answer([None, None, None])
            assert all(x['score'] is None for x in h.settled()['abilities']) and not h.calls
            h.answer(['unknown', 'unknown', 'unknown'])
            value = h.settled()
            assert value['abilities'][0]['score'] == 67 and value['abilities'][1]['score'] == 0
            assert value['abilityStatus']['source'] == 'ai'
            assert all(task['unknown'] for task in h.calls[-1]['tasks'])
            with h.db() as db:
                result = json.loads(db.execute("SELECT payload FROM LearningInteraction WHERE kind='question-evaluation' ORDER BY rowid DESC LIMIT 1").fetchone()[0])
                assert all(x['questionSnapshot']['question'] and x['questionId'] for x in result['results'])
            count = len(h.calls)
            for _ in range(5): h.request('/api/profile')
            h.request('/api/profile/radar-preferences', 'PUT', dict(preferences, mode='subjects'))
            time.sleep(2.2)
            assert len(h.calls) == count
            old = value['abilities']
            for mode in ['missing', 'few', 'duplicate', 'out-of-range', 'expression', 'fail']:
                h.mode = mode
                h.request('/api/profile/refresh', 'POST', {}, 202)
                failed = h.settled()
                assert failed['abilities'] == old and failed['abilityStatus']['status'] == 'waiting', mode
                count = len(h.calls)
                time.sleep(2.2)
                assert len(h.calls) == count, '相同失败版本不得自动重试'
            h.mode, h.score = 'blocked', 31
            h.request('/api/profile/refresh', 'POST', {}, 202)
            assert h.entered.wait(8)
            h.answer([0, 0, 0])
            h.mode, h.score = 'valid', 84
            h.release.set()
            final = h.settled()
            assert final['abilities'][0]['score'] == 84, '旧请求不得覆盖新证据版本'
            assert all(task['answer'] == 0 and not task['unknown'] for task in h.calls[-1]['tasks'])
            count = len(h.calls)
            h.stop(); h.start()
            assert h.settled()['abilities'] == final['abilities'] and len(h.calls) == count
            assert h.request('/api/profile/radar-preferences')['subjects'] == preferences['subjects']
            # 课堂记录含原题与真实开放答案；失败评价和跳过仍不能形成能力证据。
            h.mode = 'complete'
            with h.db() as db:
                for i in range(9):
                    item = dict(status='answered' if i < 6 else 'skipped', credible=i < 3 or i >= 6,
                        type='open', question=f'虚构新情境综合任务{i}：解释概念并写出方法和推导过程。',
                        rubric='写出概念、理由和应用过程', answer='先说明概念，再根据条件推导并迁移到新情境。',
                        correct=True, unknown=False)
                    db.execute('INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)',
                        (f'open-task-{i}', 'fictional-course', 1, 1, 'interaction', json.dumps(item), '2099-01-01T00:00:00Z'))
            h.request('/api/profile/refresh', 'POST', {}, 202)
            complete = h.settled()
            assert [x['score'] for x in complete['abilities']] == [55, 0, 72, 44, 63, 41]
            assert len(h.calls[-1]['tasks']) == 6, '失败评价和跳过不得作为证据'
            assert all(x['evidenceCount'] == 3 for x in complete['abilities'])
            # 恢复已中断的在途评估：保留旧画像，同一版本不再调用模型。
            count = len(h.calls); h.stop()
            with h.db() as db:
                revision = int(db.execute("SELECT value FROM ProfileMeta WHERE key='revision'").fetchone()[0]) + 1
                db.execute("UPDATE ProfileMeta SET value=? WHERE key='revision'", (str(revision),))
                stored = json.loads(db.execute("SELECT value FROM ProfileMeta WHERE key='ability-profile'").fetchone()[0])
                stored['pendingVersion'] = revision
                db.execute("UPDATE ProfileMeta SET value=? WHERE key='ability-profile'", (json.dumps(stored),))
                db.execute("INSERT INTO ProfileMeta VALUES('profile-attempt-revision',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value", (str(revision),))
            h.start(); interrupted = h.settled()
            assert interrupted['abilities'] == complete['abilities'] and interrupted['abilityStatus']['status'] == 'waiting'
            assert len(h.calls) == count, '恢复时不得重复请求已中断版本'
            h.request('/api/profile/refresh', 'POST', {}, 202)
            retried = h.settled()
            assert [x['score'] for x in retried['abilities']] == [x['score'] for x in complete['abilities']] and len(h.calls) == count + 1
            print('六维画像：可靠证据、独立 AI、门槛校验、失败保留、版本冲突与重启保留通过')
        finally: h.close()

if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encoding='utf-8')
    main(Path(sys.argv[1]))

"""用真实服务进程验证作答反馈流中断后的接续和去重。"""

import asyncio
from concurrent.futures import ThreadPoolExecutor
from contextlib import closing, ExitStack
from io import BytesIO
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile
from threading import Event, Thread
from http.server import ThreadingHTTPServer
import time
from urllib.error import URLError
from urllib.request import build_opener, ProxyHandler, Request

import websockets

from learning_agent_api_smoke import ProtocolAI, port


class RecoveryAI(ProtocolAI):
    calls = []
    entered = {}
    release = {}
    resumed = {}

    def do_POST(self):
        raw = self.rfile.read(int(self.headers['Content-Length']))
        messages = json.loads(raw)['messages']
        context = json.loads(messages[1]['content'])
        # 大纲规划上下文可省略外部请求编号，与基础模拟协议保持一致。
        request_id = context['event'].get('requestId', context['event']['type'])
        if request_id in self.entered:
            if self.entered[request_id].is_set():
                self.resumed.setdefault(request_id, []).append(context)
            elif self.has_evaluation(messages[2:]):
                self.calls.append(request_id)
                self.send_response(200)
                self.send_header('Content-Type', 'text/event-stream')
                self.end_headers()
                partial = {'model': 'fixture-agent-http', 'choices': [
                    {'delta': {'content': '{"message":"未完成的模拟反馈'}, 'finish_reason': None}]}
                self.wfile.write(('data: ' + json.dumps(partial, ensure_ascii=False) + '\n\n').encode())
                self.wfile.flush()
                self.entered[request_id].set()
                self.release[request_id].wait(timeout=45)
                return
        self.rfile = BytesIO(raw)
        super().do_POST()

    @staticmethod
    def has_evaluation(messages):
        for message in messages:
            if message['role'] != 'user':
                continue
            try:
                payload = json.loads(message['content'])
            except ValueError:
                continue
            if isinstance(payload, dict) and any('evaluationId' in item.get('result', {})
                                                for item in payload.get('toolResults', [])):
                return True
        return False


async def subscribe(base, task_id, after=0, until_terminal=False):
    async with websockets.connect(base.replace('http:', 'ws:') + '/ws/learn/next', origin=base) as socket:
        await socket.send(json.dumps(dict(taskId=task_id, afterSeq=after)))
        events = []
        while True:
            frame = json.loads(await asyncio.wait_for(socket.recv(), timeout=12))
            if frame['type'] == 'state':
                task = frame['task']
                if not until_terminal or task['status'] not in ('running', 'pending'):
                    return events, task
            else:
                events.append(frame)


async def subscribe_two(base, task_id, after):
    return await asyncio.gather(*(subscribe(base, task_id, after, until_terminal=True) for _ in range(2)))


def main(executable, browser_check=False):
    repo = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='gangyi-recovery-') as directory, \
         ThreadingHTTPServer(('127.0.0.1', 0), RecoveryAI) as model, ExitStack() as resources:
        socket_worker = resources.enter_context(ThreadPoolExecutor(max_workers=1))
        Thread(target=model.serve_forever, daemon=True).start()
        database = Path(directory) / 'fixture.db'
        env = dict(os.environ, DATABASE_PATH=str(database), PORT=str(port()), HOST='127.0.0.1',
                   AI_BASE_URL=f'http://127.0.0.1:{model.server_port}/v1', AI_API_KEY='fixture-only',
                   AI_MODEL='fixture-agent-http', LOCAL_CONTROL_TOKEN='fixture-control',
                   GANGYI_LAUNCH_SESSION_ID='recovery-fixture', BOCHA_API_KEY='', DEEPSEEK_API_KEY='',
                   DEEPSEEK_SEARCH_API_KEY='')
        env.update(BOCHA_BASE_URL=env['AI_BASE_URL'], DEEPSEEK_SEARCH_BASE_URL=env['AI_BASE_URL'])
        base = 'http://127.0.0.1:' + env['PORT']
        processes = []
        log = Path(directory) / 'service.log'

        def api(path, data=None, *, headers=None, expected=200):
            payload = None if data is None else json.dumps(data, ensure_ascii=False).encode()
            request = Request(base + path, data=payload, headers={'Content-Type': 'application/json', **(headers or {})})
            with build_opener(ProxyHandler({})).open(request, timeout=10) as response:
                assert response.status == expected, (path, response.status)
                return json.load(response)

        def launch():
            with log.open('a', encoding='utf-8') as output:
                process = subprocess.Popen([str(Path(executable).resolve())], cwd=repo, env=env,
                                           stdout=output, stderr=output,
                                           creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
            processes.append(process)
            for _ in range(150):
                assert process.poll() is None, '隔离服务启动失败'
                try:
                    api('/health')
                    return process
                except URLError:
                    time.sleep(.10)
            raise AssertionError('隔离服务启动超时')

        def task_view(task_id=''):
            return api('/api/learning-agent' + ('?taskId=' + task_id if task_id else ''))

        def wait_task(task_id='', expected='ready'):
            deadline = time.monotonic() + 45
            while time.monotonic() < deadline:
                task = task_view(task_id)
                if task['status'] == expected:
                    return task
                if task['status'] not in ('idle', 'pending', 'running'):
                    raise AssertionError((task_id, task['status'], task.get('error')))
                time.sleep(.08)
            raise AssertionError(f'等待任务超时 {task_id}')

        def submit(event):
            return api('/api/learning-agent/events', event, expected=202)['id']

        def records(kind):
            with closing(sqlite3.connect(database)) as db:
                return {row[0]: json.loads(row[1]) for row in db.execute(
                    'SELECT id,payload FROM LearningInteraction WHERE kind=?', (kind,))}

        def receive(coroutine):
            # Playwright 的同步接口拥有自己的事件循环，订阅使用独立线程。
            return socket_worker.submit(asyncio.run, coroutine).result()

        checks = []
        try:
            process = launch()
            wait_task()
            plan = api('/api/generate-plan', dict(goal='学习化合价', mode='lite', requestId='recovery-plan'), expected=202)
            course = wait_task(plan['id'])['course']['id']
            prepared = submit(dict(type='prepare_next', courseId=course, requestId='recovery-lesson', navigate=True))
            lesson = wait_task(prepared)['lesson']['id']
            lesson_url = '/api/learning-agent/lesson?lessonId=' + lesson
            api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=lesson))
            lesson_state = api(lesson_url)
            questions = [section for section in lesson_state['sections'] if section['kind'] == 'question']

            browser = None
            browser_errors = []
            if browser_check:
                from playwright.sync_api import expect, sync_playwright
                from classroom_v560_fixture import launch_test_browser
                browser = launch_test_browser(resources.enter_context(sync_playwright()))
                resources.callback(browser.close)
                questions.sort(key=lambda section: section.get('questionKind') != 'practice')

            for number, method in enumerate(('graceful', 'interrupted', 'manual_pause')):
                request_id = 'recovery-' + method
                RecoveryAI.entered[request_id] = Event()
                RecoveryAI.release[request_id] = Event()
                previous_feedback = records('practice')
                before_inputs = records('chat-user')
                event = dict(type='question_answer', courseId=course, lessonId=lesson, requestId=request_id,
                             sectionId=questions[number]['id'], sectionVersion=questions[number]['version'],
                             text=f'隔离作答 {number + 1}：铁的化合价升高。')
                task_id = submit(event)
                assert RecoveryAI.entered[request_id].wait(timeout=30), '模型未到达评价后的反馈流'
                assert records('practice') == previous_feedback, '未完成反馈时不应发布评分'
                assert len(records('chat-user')) == len(before_inputs) + 1
                pages = []
                if browser and method == 'graceful':
                    for width, height in ((1280, 720), (390, 844)):
                        tab = browser.new_page(viewport=dict(width=width, height=height))
                        tab.on('pageerror', lambda error: browser_errors.append(str(error)))
                        tab.goto(base + '/practice?lessonId=' + lesson)
                        expect(tab.locator('#lesson-stop')).to_be_visible(timeout=12000)
                        tab.evaluate('window.recoveryDocument = true')
                        pages.append(tab)
                before_events, current = receive(subscribe(base, task_id))
                assert current['status'] == 'running'
                assert before_events
                cursor = before_events[-1]['seq']
                old_pid = process.pid

                if method == 'manual_pause':
                    paused = api('/api/learning-agent/control', dict(command='pause', taskId=task_id))
                    assert paused['status'] == 'paused'
                if method != 'interrupted':
                    assert api('/internal/shutdown', {}, headers={'X-Gangyi-Control-Token': 'fixture-control'})['ok']
                else:
                    process.kill()
                exit_code = process.wait(timeout=25)
                if method != 'interrupted':
                    assert exit_code == 0
                RecoveryAI.release[request_id].set()
                assert records('practice') == previous_feedback

                process = launch()
                assert process.pid != old_pid
                if method == 'manual_pause':
                    paused_events, paused = receive(subscribe(base, task_id, cursor))
                    assert paused['status'] == 'paused' and paused.get('pauseReason') == 'user'
                    assert paused_events and max(item['seq'] for item in paused_events) > cursor
                    assert not RecoveryAI.resumed.get(request_id)
                    assert records('practice') == previous_feedback
                    api('/api/learning-agent/control', dict(command='resume', taskId=task_id))
                copies = receive(subscribe_two(base, task_id, cursor))
                assert copies[0][0] == copies[1][0] and copies[0][0]
                replayed = [item['seq'] for item in copies[0][0]]
                assert replayed == sorted(set(replayed)) and replayed[0] > cursor
                assert all(task['id'] == task_id and task['status'] == 'waiting_student' for _, task in copies), copies
                assert RecoveryAI.resumed.get(request_id)
                assert all(any(row['requestId'] == request_id for row in ctx['currentInputs'])
                           for ctx in RecoveryAI.resumed[request_id])
                current_lesson = api(lesson_url)
                assert current_lesson['id'] == lesson and current_lesson['entered']
                assert [item['id'] for item in current_lesson['sections']] == [item['id'] for item in lesson_state['sections']]
                assert '未完成的模拟反馈' not in json.dumps(current_lesson, ensure_ascii=False)
                for tab in pages:
                    feedback = tab.locator('.chat-evaluation').get_by_text('依据实际作答给出评价', exact=True)
                    expect(feedback).to_have_count(1, timeout=20000)
                    expect(tab.locator('#lesson-stop')).to_be_hidden()
                    assert tab.evaluate('window.recoveryDocument === true'), '续课不应要求刷新页面'
                    tab.close()
                assert not browser_errors, browser_errors

                after_feedback = records('practice')
                assert len(after_feedback) == len(previous_feedback) + 1
                assert all(after_feedback[key] == value for key, value in previous_feedback.items())
                calls = RecoveryAI.calls.count(request_id)
                with ThreadPoolExecutor(max_workers=2) as clients:
                    assert list(clients.map(submit, [event, event])) == [task_id, task_id]
                assert RecoveryAI.calls.count(request_id) == calls
                assert records('practice') == after_feedback and len(records('chat-user')) == len(before_inputs) + 1
                checks.append(dict(mode=method, previousPid=old_pid, resumedPid=process.pid, resumeCursor=cursor,
                                   replayedEvents=len(replayed), evaluationCount=len(after_feedback),
                                   browserTabs=len(pages)))

            stale_id = 'recovery-stale-answer'
            RecoveryAI.entered[stale_id] = Event()
            RecoveryAI.release[stale_id] = Event()
            previous_feedback = records('practice')
            stale = submit(dict(type='question_answer', courseId=course, lessonId=lesson, requestId=stale_id,
                                sectionId=questions[3]['id'], sectionVersion=questions[3]['version'],
                                text='这一份较早的作答已经被另一个窗口的输入替代。'))
            assert RecoveryAI.entered[stale_id].wait(timeout=30)
            newer = submit(dict(type='question_answer', courseId=course, lessonId=lesson, requestId='recovery-newer-answer',
                                sectionId=questions[4]['id'], sectionVersion=questions[4]['version'],
                                text='这是另一个窗口提交的最新作答。'))
            old_pid = process.pid
            process.kill()
            process.wait(timeout=10)
            RecoveryAI.release[stale_id].set()
            with closing(sqlite3.connect(database)) as db:
                durable = {row[0]: json.loads(row[1]) for row in db.execute(
                    'SELECT id,payload FROM ClassroomActivity WHERE id IN (?, ?)', (stale, newer))}
                assert durable[stale]['status'] == 'running', '旧任务必须在中断时仍持久化为运行中'
                assert durable[stale]['learningVersion'] < durable[newer]['learningVersion']
            process = launch()
            wait_task(stale, 'superseded')
            wait_task(newer, 'waiting_student')
            assert not RecoveryAI.resumed.get(stale_id), '已有更新作答时不得再次向模型发送旧任务'
            after_feedback = records('practice')
            assert len(after_feedback) == len(previous_feedback) + 1
            assert all(after_feedback[key] == value for key, value in previous_feedback.items())
            assert api(lesson_url)['teachingTaskId'] == newer
            checks.append(dict(mode='superseded', previousPid=old_pid, resumedPid=process.pid,
                               staleLearningVersion=durable[stale]['learningVersion'],
                               newerLearningVersion=durable[newer]['learningVersion'],
                               evaluationCount=len(after_feedback)))
        except Exception:
            print(log.read_text(encoding='utf-8', errors='replace')[-10000:], file=sys.stderr)
            raise
        finally:
            for release in RecoveryAI.release.values():
                release.set()
            for child in processes:
                if child.poll() is None:
                    child.kill()
                    child.wait(timeout=10)
            model.shutdown()
        print(json.dumps(dict(passed=True, realCppService=True, paidModelRequests=0, checks=checks), ensure_ascii=False))


if __name__ == '__main__':
    main(sys.argv[1], browser_check='--browser' in sys.argv[2:])

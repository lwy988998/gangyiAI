"""虚构档案与协议模拟服务验收主控页面；不读取用户数据库、截图或凭据。"""
import json
import os
import socket
import sqlite3
import subprocess
import sys
import tempfile
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Thread
from urllib.request import Request, urlopen
from playwright.sync_api import sync_playwright


def port():
    with socket.socket() as sock:
        sock.bind(('127.0.0.1', 0))
        return sock.getsockname()[1]


class FixtureAI(BaseHTTPRequestHandler):
    calls = 0

    def log_message(self, *_):
        pass

    def do_POST(self):
        body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        messages = body['messages']
        if '教学主控' not in messages[0]['content']:
            value = dict(order=[], teaching=[], nextTaskId='', reason='虚构档案没有旧大纲任务')
        else:
            type(self).calls += 1
            context = json.loads(messages[1]['content'])
            event = context['event']
            results = []
            for item in messages[2:]:
                if item['role'] == 'user':
                    try:
                        results.extend(json.loads(item['content']).get('toolResults', []))
                    except json.JSONDecodeError:
                        pass
            actions, state = [], 'completed'
            if event['type'] == 'startup':
                actions = [dict(id='startup-recommendations', tool='update_recommendations', args=dict(
                    lite=['认识函数', '梳理化合价', '理解英语时态', '阅读历史材料', '观察天气变化'],
                    deep=['数学函数课程', '化学基础课程', '英语语法课程', '历史阅读课程', '地理气候课程'],
                    reason='没有实际作答时仅提出通用方向'))]
            elif event['type'] == 'prepare_next':
                lesson = next((item['result']['lessonId'] for item in results if 'lessonId' in item['result']), None)
                if not lesson:
                    actions = [dict(id='create', tool='create_lesson', args=dict(courseId='fixture', title='化合价入门', purpose='先理解真实作答所需的概念'))]
                    state = 'continue'
                elif not any('sectionId' in item['result'] for item in results):
                    actions = [dict(id='explain', tool='append_section', args=dict(lessonId=lesson,
                        section=dict(kind='explanation', title='比较元素的前后状态', body='先比较反应前后同一种元素。'))),
                        dict(id='question', tool='append_section', args=dict(lessonId=lesson,
                        section=dict(kind='question', title='你来尝试', question=dict(
                            question='Fe 变为 Fe²⁺ 时，化合价怎样变化？', type='open', expectedAnswer='由0升高到+2', rubric='指出升高'))))]
                    state = 'continue'
                else:
                    actions = [dict(id='finish', tool='finish_lesson', args=dict(lessonId=lesson))]
            elif event['type'] == 'question_answer':
                actual = next(item for item in context['feedback'] if item['payload'].get('requestId') == event['requestId'])
                actions = [dict(id='evaluate', tool='evaluate_answer', args=dict(interactionId=actual['id'], score=80,
                    credible=True, feedback='你指出化合价升高，可以再说明电子的变化。', reason='引用真实输入中的0和+2',
                    uncertainty='仅可确认本题，不能推测其他能力'))]
                state = 'waiting_student'
            value = dict(message='根据你实际写出的思路，我们继续比较化合价与电子变化。', actions=actions, state=state)
        text = json.dumps(value, ensure_ascii=False)
        if body.get('stream'):
            self.send_response(200)
            self.send_header('Content-Type', 'text/event-stream')
            self.end_headers()
            try:
                for index in range(0, len(text), 9):
                    chunk = dict(model='fixture-agent-protocol', choices=[dict(delta=dict(content=text[index:index + 9]), finish_reason=None)])
                    self.wfile.write(('data: ' + json.dumps(chunk, ensure_ascii=False) + '\n\n').encode())
                    self.wfile.flush()
                    time.sleep(.004)
                final = dict(model='fixture-agent-protocol', choices=[dict(delta={}, finish_reason='stop')])
                self.wfile.write(('data: ' + json.dumps(final) + '\n\ndata: [DONE]\n\n').encode())
                self.wfile.flush()
            except (BrokenPipeError, ConnectionResetError):
                pass
        else:
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(dict(model='fixture-agent-protocol', choices=[dict(message=dict(content=text), finish_reason='stop')])).encode())


def main(executable):
    repository = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='gangyi-agent-browser-') as directory:
        database = Path(directory) / 'fictional.db'
        ai = ThreadingHTTPServer(('127.0.0.1', port()), FixtureAI)
        Thread(target=ai.serve_forever, daemon=True).start()
        base = f'http://127.0.0.1:{port()}'
        env = dict(os.environ, DATABASE_PATH=str(database), PORT=base.rsplit(':', 1)[1], HOST='127.0.0.1',
            AI_BASE_URL=f'http://127.0.0.1:{ai.server_port}/v1', AI_API_KEY='fictional-test-key',
            AI_MODEL='fixture-agent-protocol', GANGYI_LAUNCH_SESSION_ID='isolated-browser-launch')
        process = subprocess.Popen([str(Path(executable).resolve())], cwd=repository, env=env,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
            creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        def get(path):
            with urlopen(base + path, timeout=10) as response:
                return json.load(response)
        def post(path, value):
            with urlopen(Request(base + path, data=json.dumps(value).encode(), headers={'Content-Type': 'application/json'}), timeout=10) as response:
                return json.load(response)
        try:
            last_state = {}
            for _ in range(300):
                if process.poll() is not None:
                    raise AssertionError('隔离服务未正常启动')
                try:
                    last_state = get('/api/learning-agent')
                    if last_state['status'] == 'ready':
                        break
                except OSError:
                    pass
                time.sleep(.1)
            else:
                diagnostic = {key: last_state.get(key) for key in ('status', 'calls', 'error')}
                raise AssertionError('启动推荐未完成：' + json.dumps(diagnostic, ensure_ascii=False))
            with sqlite3.connect(database) as db:
                db.execute('INSERT INTO Course(id,anonymousId,goal,mode,title,source,status,createdAt,updatedAt) VALUES(?,?,?,?,?,?,?,?,?)',
                    ('fixture', 'fictional', '化合价', 'deep', '虚构化学课程', 'ai', 'active', '2026-10-04', '2026-10-04'))
            with sync_playwright() as playwright:
                browser = playwright.chromium.launch(headless=True)
                page = browser.new_page(viewport={'width': 1100, 'height': 850}, reduced_motion='reduce')
                errors = []
                page.on('pageerror', lambda error: errors.append(str(error)))
                page.goto(base + '/agent-prepare.html?courseId=fixture&requestId=browser-preparation')
                page.wait_for_url('**/agent-classroom.html?lessonId=*', timeout=60000)
                page.get_by_role('heading', name='化合价入门', exact=True).wait_for()
                assert page.locator('#lesson-sections .agent-card').count() == 2
                assert '由0升高到+2' not in page.locator('#lesson-sections').inner_text()
                lesson_id = page.url.split('lessonId=', 1)[1]
                saved = get('/api/learning-agent/lesson?lessonId=' + lesson_id)
                assert 'expectedAnswer' not in json.dumps(saved) and 'rubric' not in json.dumps(saved)
                calls = FixtureAI.calls
                page.reload()
                page.get_by_role('heading', name='你来尝试', exact=True).wait_for()
                assert FixtureAI.calls == calls
                answer = page.locator('#lesson-sections textarea')
                answer.fill('我认为从0升高到+2。')
                page.reload()
                assert page.locator('#lesson-sections textarea').input_value() == '我认为从0升高到+2。'
                page.locator('#lesson-sections button[type=submit]').click()
                page.locator('#lesson-sections .agent-message.assistant').wait_for()
                page.wait_for_function("document.querySelector('#lesson-sections .agent-note')?.textContent.includes('继续回答')", timeout=30000)
                assert '比较化合价' in page.locator('#lesson-sections .agent-message.assistant').inner_text()
                calls = FixtureAI.calls
                page.reload()
                page.locator('#lesson-sections .agent-message.assistant').wait_for()
                assert page.locator('#lesson-sections .agent-message.user').count() == 1
                assert FixtureAI.calls == calls
                page.set_viewport_size({'width': 390, 'height': 844})
                assert page.evaluate('document.documentElement.scrollWidth <= innerWidth')
                page.locator('#lesson-sections textarea').focus()
                assert page.locator('#lesson-sections textarea').evaluate('(element) => element === document.activeElement')
                assert not errors, errors
                browser.close()
            with sqlite3.connect(database) as db:
                rows = db.execute("SELECT payload FROM LearningInteraction WHERE kind='practice'").fetchall()
                actual = [json.loads(row[0]) for row in rows if 'questionId' in json.loads(row[0])]
                assert len(actual) == 1 and actual[0]['response'] == '我认为从0升高到+2。'
                assert actual[0]['model'] == 'fixture-agent-protocol' and actual[0]['score'] == 80
            print('主控协议流、两块自主课件、先题后答、答案隐藏、真实输入评价、刷新恢复、草稿和窄屏验收通过（虚构模型协议）')
        finally:
            if process.poll() is None:
                process.terminate()
                process.wait(timeout=20)
            ai.shutdown()
            ai.server_close()


if __name__ == '__main__':
    main(sys.argv[1])

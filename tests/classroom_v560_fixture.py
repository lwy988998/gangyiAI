"""为统一课堂的浏览器验收提供隔离档案与协议夹具，不读取真实用户档案。"""
import json
import os
import socket
import sqlite3
import subprocess
import sys
import tempfile
import time
from contextlib import closing, contextmanager
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Thread
from urllib.request import Request, urlopen


class FixtureAI(BaseHTTPRequestHandler):
    calls = []
    slow = False
    fail_next = False
    fail = False

    def log_message(self, *_):
        pass

    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps({'calls': len(self.calls), 'slow': self.slow, 'fail_next': self.fail_next, 'fail': self.fail}).encode())

    def do_POST(self):
        request = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        if self.path == '/fixture-control':
            type(self).slow = request.get('slow', False)
            type(self).fail_next = request.get('fail_next', False)
            type(self).fail = request.get('fail', False)
            self.send_response(200)
            self.end_headers()
            self.wfile.write(b'{}')
            return
        if type(self).fail_next or type(self).fail:
            type(self).fail_next = False
            self.send_response(503)
            self.end_headers()
            return
        context = json.loads(request['messages'][1]['content'])
        event = context['event']
        type(self).calls.append(event['requestId'])
        results = []
        for item in request['messages'][2:]:
            if item['role'] == 'user':
                try:
                    results.extend(json.loads(item['content']).get('toolResults', []))
                except ValueError:
                    pass
        actions, state, message = [], 'completed', '已保存本次处理。'
        if event['type'] == 'startup':
            actions = [dict(id='recommend', tool='update_recommendations', args=dict(
                lite=['认识化合价', '比较电子变化', '练习配平'], deep=['氧化还原概念课', '化合价与电子转移'], reason='虚构档案仅提供学习方向'))]
        elif event['type'] == 'plan_course':
            actions = [dict(id='plan', tool='create_course', args=dict(goal='理解化合价与电子变化', title='隔离化学课程', mode='lite', summary='用于界面验收的虚构课程',
                payload=dict(courseStructure=[dict(stage='建立概念', goal='比较化合价变化', why='先比较变化，再联系电子', topics=['化合价比较', '电子得失']),
                    dict(stage='应用与巩固', goal='应用比较方法', why='结合实际回答再安排练习', topics=['反应判断', '配平练习'])])))]
        elif event['type'] == 'prepare_next':
            lesson = next((item['result']['lessonId'] for item in results if 'lessonId' in item['result']), None)
            if not lesson:
                outline = context['courses'][0]['outline']
                actions = [dict(id='create', tool='create_lesson', args=dict(courseId=event['courseId'], title='化合价与电子变化', purpose='从比较前后状态开始理解', intent='advance', topicId=outline['courseStructure'][0]['topics'][0]['id']))]
                state = 'continue'
            elif not any('sectionId' in item['result'] for item in results):
                sections = [dict(kind='explanation', title='先比较同一种元素', body='比较反应前后同一种元素的状态。先标出化合价，再观察升高还是降低。\n\n把观察过程说清楚，比只记结论更有帮助。'),
                    dict(kind='explanation', title='把化合价变化联系到电子', body='接着联系电子得失，解释这种变化为什么发生。\n\n请先给出判断理由，再用另一种情况检查自己的理解。'),
                    dict(kind='explanation', presentation='example', title='示范：比较一个简单变化', body='这是示范讲解。先写出前后状态，再比较变化方向。示范不产生学生评分。'),
                    dict(kind='question', questionKind='interaction', title='说说你的观察', question=dict(question='Fe 变为 Fe²⁺ 时，你观察到怎样的变化？', type='open', expectedAnswer='PRIVATE-EXPECTED', rubric='PRIVATE-RUBRIC')),
                    dict(kind='question', questionKind='interaction', title='联系电子说一说', question=dict(question='你怎样用电子得失解释刚才的观察？', type='open', expectedAnswer='PRIVATE-EXPECTED', rubric='PRIVATE-RUBRIC')),
                    dict(kind='question', questionKind='practice', title='练习一：判断变化', question=dict(question='请选择化合价升高的情况，并写出理由。', type='choice', options=['Fe → Fe²⁺', 'Cu²⁺ → Cu'], expectedAnswer='PRIVATE-EXPECTED', rubric='PRIVATE-RUBRIC')),
                    dict(kind='question', questionKind='practice', title='练习二：解释理由', question=dict(question='解释化合价变化与电子得失的联系。', type='open', expectedAnswer='PRIVATE-EXPECTED', rubric='PRIVATE-RUBRIC')),
                    dict(kind='summary', title='本节检查点', body='- 能比较同一元素的前后状态。\n- 能说明自己的判断理由。\n- 能用电子变化解释观察。')]
                actions = [dict(id=f'section-{index}', tool='append_section', args=dict(lessonId=lesson, section=section)) for index, section in enumerate(sections)]
                state = 'continue'
            else:
                actions = [dict(id='finish', tool='finish_lesson', args=dict(lessonId=lesson))]
        elif event['type'] in ('lesson_start', 'chat'):
            lesson = next((item['result'] for item in results if 'sections' in item['result']), None)
            if not lesson:
                actions = [dict(id='read', tool='read_lesson', args=dict(lessonId=event['lessonId']))]
                state = 'continue'
            else:
                explanations = [section for section in lesson['sections'] if section['kind'] == 'explanation' and section.get('presentation') != 'example']
                interactions = [section for section in lesson['sections'] if section.get('questionKind') == 'interaction']
                practices = [section for section in lesson['sections'] if section.get('questionKind') == 'practice']
                index = 1 if event['type'] == 'chat' and '继续' in event.get('text', '') else 0
                actions = [dict(id='focus', tool='select_teaching_focus', args=dict(lessonId=lesson['id'], sectionId=explanations[index]['id'], interactionSectionId=interactions[index]['id'], practiceSectionId=practices[0]['id']))]
                message = '请围绕当前段说说你的思路；我会根据你的反馈再决定下一步。'
                state = 'waiting_student'
        elif event['type'] == 'question_answer':
            actual = next(item for item in context['feedback'] if item['payload'].get('requestId') == event['requestId'])
            if '为什么' in event['text'] or '提示' in event['text']:
                actions = [dict(id='classify', tool='classify_input', args=dict(interactionId=actual['id'], isAnswer=False, reason='本次输入只有追问'))]
                message = '这是对思路的追问。我们可以先比较前后状态，再联系电子变化。'
            elif event.get('action') == 'skip':
                message = '这道题已跳过，不把跳过当成可靠作答。'
            else:
                actions = [dict(id='evaluate', tool='evaluate_answer', args=dict(interactionId=actual['id'], score=80, credible=True, feedback='依据这次实际输入给出反馈，请继续说明判断理由。', reason='引用本次学生输入', uncertainty='只反映这道题'))]
                message = '你已经给出了观察，接下来可以补充电子变化的理由。'
            state = 'waiting_student'
        elif event['type'] == 'lesson_finish_request':
            message = '请结合本节检查点回顾，已有记录会保留，缺少证据的部分继续练习。'
            state = 'waiting_student'
        elif event['type'] == 'acceptance_tool':
            actions = [dict(id='control-check', tool=event['tool'], args=event['args'])]
            message = '测试调整已保存。你可以从学习导航撤回这次调整，已有作答记录仍保留。'
        value = json.dumps(dict(message=message, actions=actions, state=state), ensure_ascii=False)
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream')
        self.end_headers()
        try:
            for index in range(0, len(value), 23):
                chunk = dict(model='fixture-v560', choices=[dict(delta=dict(content=value[index:index+23]), finish_reason=None)])
                self.wfile.write(('data: ' + json.dumps(chunk, ensure_ascii=False) + '\n\n').encode())
                self.wfile.flush()
                if type(self).slow:
                    time.sleep(.12)
            self.wfile.write(('data: ' + json.dumps(dict(model='fixture-v560', choices=[dict(delta={}, finish_reason='stop')])) + '\n\ndata: [DONE]\n\n').encode())
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            pass


@contextmanager
def start_session(executable=None):
    repo = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='gangyi-v560-ui-') as directory:
        database = Path(directory) / 'fictional.db'
        ai = ThreadingHTTPServer(('127.0.0.1', 0), FixtureAI)
        Thread(target=ai.serve_forever, daemon=True).start()
        with socket.socket() as sock:
            sock.bind(('127.0.0.1', 0))
            port = sock.getsockname()[1]
        base = f'http://127.0.0.1:{port}'
        log = open(Path(directory) / 'service.log', 'w+', encoding='utf-8')
        env = dict(os.environ, DATABASE_PATH=str(database), PORT=str(port), HOST='127.0.0.1', AI_BASE_URL=f'http://127.0.0.1:{ai.server_port}/v1', AI_API_KEY='fictional-fixture-key', AI_MODEL='fixture-v560', GANGYI_LAUNCH_SESSION_ID='v560-ui-isolated')
        process = subprocess.Popen([str(Path(executable).resolve() if executable else repo / 'build-windows/gangyiAI.exe')], cwd=repo, env=env, stdout=log, stderr=log, creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        def api(path, value=None):
            with urlopen(Request(base+path, data=None if value is None else json.dumps(value, ensure_ascii=False).encode(), headers={'Content-Type':'application/json'}), timeout=15) as response:
                return json.load(response)
        def wait(task):
            deadline = time.monotonic()+90
            while time.monotonic()<deadline:
                value = api('/api/learning-agent?taskId='+task)
                if value['status'] not in ('pending','running','idle'):
                    assert value['status'] in ('ready','waiting_student'), value.get('error')
                    return value
                time.sleep(.1)
            raise RuntimeError('隔离任务超时')
        try:
            for _ in range(300):
                if process.poll() is not None:
                    raise RuntimeError('隔离服务启动失败')
                try:
                    startup = api('/api/learning-agent')
                    if startup.get('id'):
                        wait(startup['id'])
                        break
                except OSError:
                    pass
                time.sleep(.1)
            plan = api('/api/generate-plan', dict(goal='虚构学生的化学学习', mode='lite', requestId='v560-ui-plan'))
            course = wait(plan['id'])['course']['id']
            prepare = api('/api/learning-agent/events', dict(type='prepare_next', courseId=course, requestId='v560-ui-prepare', navigate=False))
            lesson = wait(prepare['id'])['lesson']['id']
            api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=lesson))
            info = dict(base=base, fixture=f'http://127.0.0.1:{ai.server_port}', database=str(database), pid=process.pid, lessonId=lesson, courseId=course, status='ready')
            (repo/'verification/v560-ui-session.json').write_text(json.dumps(info, ensure_ascii=False, indent=2), encoding='utf-8')
            yield info, api, process
        finally:
            if process.poll() is None:
                process.terminate()
                process.wait(timeout=20)
            ai.shutdown()
            ai.server_close()
            log.close()


def main():
    try:
        with start_session() as (info, api, process):
            print(json.dumps(info, ensure_ascii=False), flush=True)
            while process.poll() is None:
                time.sleep(.5)
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    main()

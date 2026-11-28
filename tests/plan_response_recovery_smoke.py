"""使用真实课程接口与隔离模型验证回复异常、自动恢复和手动继续。"""

from contextlib import closing
import io
import json
import os
import sqlite3
import sys
from threading import Event
import time
from urllib.error import HTTPError

from playwright.sync_api import expect, sync_playwright
import classroom_v560_fixture as fixture


class Model(fixture.FixtureAI):
    plan_calls = {}
    allow_manual_recovery = False
    manual_started = Event()
    manual_release = Event()

    def log_message(self, *_):
        pass

    def do_POST(self):
        raw = self.rfile.read(int(self.headers['Content-Length']))
        request = json.loads(raw)
        context = {}
        for item in request['messages']:
            if item['role'] != 'user':
                continue
            try:
                candidate = json.loads(item['content'])
            except ValueError:
                continue
            if isinstance(candidate, dict) and 'event' in candidate:
                context = candidate
                break
        event = context.get('event', {})
        if event.get('type') != 'plan_course' or event.get('goal') not in {
            '完整围栏', '长度后格式错误', '后续动作越权', '大纲字段类型错误', '超出恢复预算'
        }:
            self.rfile = io.BytesIO(raw)
            return super().do_POST()
        goal = event.get('goal', '')
        outline = {
            'title': '隔离课程大纲',
            'summary': '先理解概念再独立应用',
            'courseStructure': [{'stage': '概念', 'topics': ['定义', '应用']}],
        }
        finish = 'stop'
        guide = request['messages'][0]['content']
        attempts = self.plan_calls.setdefault(goal, [])
        attempts.append({'planning': '课程大纲规划助手' in guide, 'repair': '上次回复未通过校验' in guide,
                         'goal': goal, 'mode': event.get('mode'), 'tokenLimit': request['max_tokens'],
                         'messageCount': len(request['messages']), 'contextKeys': set(context),
                         'format': request['response_format'], 'catalog': '可用工具：' in guide})
        body = json.dumps(outline, ensure_ascii=False)
        if goal == '完整围栏':
            body = '```json\r\n' + body + '\r\n```'
        elif goal == '长度后格式错误':
            if len(attempts) == 1:
                body, finish = '{"title":"隔离课程大纲","courseStructure":[', 'length'
            elif len(attempts) == 2:
                body += ']'
        elif goal == '后续动作越权' and len(attempts) == 1:
            body = json.dumps({**outline, 'actions': [{'id': 'late', 'tool': 'unknown_tool', 'args': {}}]}, ensure_ascii=False)
        elif goal == '大纲字段类型错误' and len(attempts) == 1:
            body = json.dumps({**outline, 'courseStructure': [{'stage': '概念', 'topics': [{'title': '格式正确但类型错误'}]}]}, ensure_ascii=False)
        elif goal == '超出恢复预算':
            if not self.allow_manual_recovery:
                body += '\n{"extra":true}'
            else:
                self.manual_started.set()
                assert self.manual_release.wait(10)
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream')
        self.end_headers()
        frame = {'model': 'fixture', 'choices': [{'delta': {'content': body}, 'finish_reason': finish}]}
        self.wfile.write(('data: ' + json.dumps(frame, ensure_ascii=False) + '\n\ndata: [DONE]\n\n').encode())


def task_result(api, task_id, expected):
    deadline = time.monotonic() + 30
    while time.monotonic() < deadline:
        task = api('/api/learning-agent?taskId=' + task_id)
        if task['status'] not in ('running', 'pending', 'idle'):
            assert task['status'] == expected, (task.get('failure'), task['status'])
            return task
        time.sleep(.1)
    raise TimeoutError(task_id)


os.environ['GANGYI_AGENT_RETRY_BASE_MS'] = '1'
fixture.FixtureAI = Model
with fixture.start_session(sys.argv[1]) as (info, api, _):
    try:
        api('/api/generate-plan', {'goal': '完整围栏', 'mode': 'unsupported', 'requestId': 'invalid-mode'})
    except HTTPError as error:
        assert error.code == 409 and '课程模式' in json.load(error)['error']
    else:
        raise AssertionError('非法模式在发送模型请求前应被拒绝')
    assert not Model.plan_calls
    courses = {}
    for goal, mode, expected_attempts in [('完整围栏', 'deep', 1), ('长度后格式错误', 'deep', 3),
                                           ('后续动作越权', 'lite', 2), ('大纲字段类型错误', 'lite', 2)]:
        request = {'goal': goal, 'requestId': 'recovery-' + goal}
        if goal != '完整围栏':
            request['mode'] = mode
        task = api('/api/generate-plan', request)
        task = task_result(api, task['id'], 'ready')
        assert len(Model.plan_calls[goal]) == expected_attempts, Model.plan_calls
        calls = Model.plan_calls[goal]
        assert [c['repair'] for c in calls] == [False] + [True] * (expected_attempts - 1)
        assert all(c['goal'] == goal and c['mode'] == mode for c in calls)
        assert all(c['planning'] and not c['catalog'] and c['messageCount'] == 2 and
                   c['format'] == {'type': 'json_object'} and
                   c['contextKeys'] <= {'event', 'planningContext'} for c in calls), calls
        if goal == '长度后格式错误':
            assert [c['tokenLimit'] for c in Model.plan_calls[goal]] == [8192, 16384, 16384]
        courses[goal] = (task['course']['id'], mode)

    # 用隔离档案模拟简明大纲已提交、任务状态尚未收尾的恢复窗口。
    saved_task = task['id']
    saved_course = task['course']['id']
    calls_before_resume = len(Model.plan_calls['大纲字段类型错误'])
    with closing(sqlite3.connect(info['database'])) as db, db:
        row, = db.execute('SELECT payload FROM ClassroomActivity WHERE id=?', (saved_task,)).fetchone()
        checkpoint = json.loads(row)
        assert checkpoint['event']['mode'] == 'lite' and checkpoint['course']['id'] == saved_course
        checkpoint.update(status='paused', pauseReason='user')
        db.execute('UPDATE ClassroomActivity SET payload=? WHERE id=?',
                   (json.dumps(checkpoint, ensure_ascii=False), saved_task))
    api('/api/learning-agent/control', {'command': 'resume', 'taskId': saved_task})
    recovered = task_result(api, saved_task, 'ready')
    assert recovered['course']['id'] == saved_course
    assert len(Model.plan_calls['大纲字段类型错误']) == calls_before_resume

    # 持续错误必须有限停止，页面说明真实原因。使用原任务的重试按钮恢复。
    goal = '超出恢复预算'
    preexisting = api('/api/generate-plan', {'goal': goal, 'mode': 'deep', 'requestId': 'manual-recovery'})
    failure = task_result(api, preexisting['id'], 'failed')
    assert failure['failure']['kind'] == 'structure'
    assert len(Model.plan_calls[goal]) == 3 and 'course' not in failure
    with sync_playwright() as runner:
        browser = fixture.launch_test_browser(runner)
        page = browser.new_page(viewport={'width': 1280, 'height': 800})
        page.goto(info['base'] + '/plan?taskId=' + preexisting['id'])
        alert = page.locator('.failure-panel[role=alert]')
        alert.get_by_role('heading', name='AI 回复格式有误，课程尚未生成').wait_for()
        assert alert.get_by_text('原学习目标已保留。', exact=False).is_visible()
        page.set_viewport_size({'width': 390, 'height': 844})
        assert page.evaluate('document.documentElement.scrollWidth <= innerWidth + 1')
        Model.allow_manual_recovery = True
        with page.expect_request(lambda req: req.url.endswith('/api/learning-agent/control')) as retried:
            alert.get_by_role('button', name='继续规划').click()
        assert retried.value.post_data_json == {'command': 'retry', 'taskId': preexisting['id']}
        assert Model.manual_started.wait(10)
        try:
            duplicate = api('/api/learning-agent/control', {'command': 'retry', 'taskId': preexisting['id']})
            assert duplicate['id'] == preexisting['id'] and duplicate['status'] == 'running'
        finally:
            Model.manual_release.set()
        # 重试期间各轮草稿会保留同名标题；只有已保存总览才代表恢复完成。
        expect(page.locator('#plan-title')).to_have_text('隔离课程大纲')
        expect(page.locator('#result')).to_be_visible()
        expect(page.locator('#plan-view .outline-row h3')).to_have_text(['概念'])
        expect(page.locator('#plan-live-content')).to_be_hidden()
        expect(alert).to_be_hidden()
        browser.close()
    task = task_result(api, preexisting['id'], 'ready')
    assert len(Model.plan_calls[goal]) == 4 and Model.plan_calls[goal][-1]['repair']
    courses[goal] = (task['course']['id'], 'deep')
    with closing(sqlite3.connect(info['database'])) as db:
        for goal, (course_id, mode) in courses.items():
            assert db.execute('SELECT id, mode FROM Course WHERE goal=?', (goal,)).fetchall() == [(course_id, mode)]
    print('通过：首次专用大纲协议、截断、畸形 JSON、越权字段、类型错误、有界失败及原任务恢复。')

"""我的AI隔离验收：仅使用模拟模型，保存页面预览与机器可读结果。"""
import io
import json
import os
import sqlite3
import subprocess
from urllib.parse import urlsplit
import sys
import time
from pathlib import Path
from urllib.error import HTTPError
from playwright.sync_api import sync_playwright
import classroom_v560_fixture as fixture


class PersonalFixture(fixture.FixtureAI):
    def do_POST(self):
        raw = self.rfile.read(int(self.headers['Content-Length']))
        body = json.loads(raw)
        context = None
        for message in body.get('messages', []):
            try:
                value = json.loads(message.get('content', ''))
                if value.get('event', {}).get('type') == 'assistant_chat':
                    context = value
                    break
            except (ValueError, AttributeError):
                pass
        if context is None:
            self.rfile = io.BytesIO(raw)
            return super().do_POST()
        type(self).calls.append(body)
        text = context['event']['text']
        if '兼容选项' in text and body.get('stream_options'):
            type(self).calls.append(body)
            self.send_response(400)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"error":{"message":"stream_options is unsupported"}}')
            return
        actions = []
        if '先举例' in text:
            actions = [{'id': 'style-one', 'tool': 'set_teaching_preferences', 'args': {'style': '先举生活中的例子，再说明概念'}}]
            message = '已将讲解方式调整为先举生活中的例子，再说明概念。你可以在教学偏好页修改或撤回。'
        elif '记住' in text:
            actions = [{'id': 'memory-one', 'tool': 'remember_preference', 'args': {'content': '喜欢用生活中的例子理解概念', 'source': 'explicit'}}]
            message = '已记住你的偏好，可在AI记忆页查看和修改。'
        elif '删除课程' in text:
            target = context['courses'][0]['id']
            actions = [{'id': 'delete-one', 'tool': 'propose_personal_action', 'args': {'kind': 'delete_course', 'target': target, 'reason': '用户明确要求删除课程'}}]
            message = '已准备删除预览，请先检查影响，再确认执行。'
        else:
            message = '你的个人偏好和学习记录已经读取。可以继续查看学习报告，或告诉我需要调整什么。'
        decision = json.dumps({'message': message, 'actions': actions, 'state': 'completed'}, ensure_ascii=False)
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream')
        self.end_headers()
        try:
            for offset in range(0, len(decision), 12):
                event = {'model': 'fixture-personal', 'choices': [{'delta': {'content': decision[offset:offset+12]}, 'finish_reason': None}]}
                self.wfile.write(('data: '+json.dumps(event, ensure_ascii=False)+'\n\n').encode())
                self.wfile.flush()
                time.sleep(.006)
            self.wfile.write(('data: '+json.dumps({'choices': [{'delta': {}, 'finish_reason': 'stop'}]})+'\n\n').encode())
            self.wfile.write(('data: '+json.dumps({'choices': [], 'usage': {'prompt_tokens': 120, 'completion_tokens': 35, 'total_tokens': 155}})+'\n\n').encode())
            self.wfile.write(b'data: [DONE]\n\n')
        except (BrokenPipeError, ConnectionResetError):
            pass


def main():
    executable = sys.argv[1]
    output = Path(sys.argv[2] if len(sys.argv) > 2 else 'verification/my-ai').resolve()
    output.mkdir(parents=True, exist_ok=True)
    fixture.FixtureAI = PersonalFixture
    checks = []
    with fixture.start_session(executable) as (info, api, process):
        base = info['base']
        def wait(task_id):
            until = time.monotonic()+45
            while time.monotonic() < until:
                task = api('/api/learning-agent?taskId='+task_id)
                if task['status'] in ('ready', 'waiting_student'):
                    return task
                if task['status'] in ('failed', 'superseded'):
                    raise AssertionError(task)
                time.sleep(.15)
            raise AssertionError('AI任务未完成')
        profile = api('/api/my-ai/profile')
        api('/api/my-ai/profile', {'version': profile['version'], 'nickname': '测试同学', 'goal': '这是用于验收的隔离学习档案'})
        first = api('/api/my-ai/chats', {'title': '教学风格与学习安排'})
        second = api('/api/my-ai/chats', {'title': '另一段独立聊天'})
        body = {'id': first['id'], 'requestId': 'preference-one', 'text': '以后请先举例，再讲定义'}
        task = api('/api/my-ai/chat/send', body)
        wait(task['id'])
        assert api('/api/my-ai/chat/send', body)['id'] == task['id']
        prefs = api('/api/my-ai/preferences')
        assert '先举生活' in prefs['style']
        assert not api('/api/my-ai/chat?id='+second['id'])['messages']
        checks.append('管家真实工具执行、请求幂等和聊天隔离')
        task = api('/api/my-ai/chat/send', {'id': first['id'], 'requestId': 'memory-one', 'text': '记住我喜欢生活中的例子'})
        wait(task['id'])
        memories = api('/api/my-ai/memories')['items']
        assert memories and memories[0]['source'] == 'explicit'
        memory = memories[0]
        api('/api/my-ai/memories', {'id': memory['id'], 'version': memory['version'], 'deleted': True})
        assert not api('/api/my-ai/memories')['items']
        checks.append('记忆保存、来源与删除')
        usage = api('/api/my-ai/usage')
        assert usage['knownTokenCalls'] >= 2 and usage['tokens'] >= 310
        assert any(row['tokens'] is None for row in usage['items'])
        checks.append('流式空choices用量帧与缺失Token区分')
        task = api('/api/my-ai/chat/send', {'id': first['id'], 'requestId': 'compatibility', 'text': '测试兼容选项'})
        wait(task['id'])
        compatibility = api('/api/my-ai/usage')['items']
        assert any(r['status'] == 'failed' for r in compatibility)
        checks.append('服务商拒绝用量选项时兼容重试与实际请求计数')
        with sync_playwright() as p:
            browser = p.chromium.launch()
            page = browser.new_page(viewport={'width': 1440, 'height': 1000}, reduced_motion='reduce')
            errors = []
            page.on('pageerror', lambda error: errors.append(str(error)))
            for route in ['', 'reports', 'plan', 'preferences', 'memories', 'tasks', 'usage', 'profile', 'settings', 'data', 'chat?id='+first['id']]:
                page.goto(base+'/my-ai'+('/'+route if route else ''), wait_until='networkidle')
                page.wait_for_function("!document.querySelector('#my-ai-status').textContent.includes('正在读取')")
                assert not page.locator('#my-ai-status').evaluate('(e)=>e.classList.contains("error")'), page.locator('#my-ai-status').inner_text()
                assert page.locator('.my-ai-sidebar [aria-current="page"]').count() == 1
                assert page.evaluate('document.documentElement.scrollWidth <= innerWidth+1')
            assert not errors, errors
            checks.append('11个页面直接打开、资源加载、目录与横向布局')
            page.set_viewport_size({'width': 1100, 'height': 768})
            page.goto(base+'/my-ai/chat?id='+first['id'], wait_until='networkidle')
            box = page.locator('#send-chat').bounding_box()
            assert box['y']+box['height'] <= 768
            page.set_viewport_size({'width': 1440, 'height': 1000})
            checks.append('较矮电脑窗口聊天输入和发送按钮保持可见')
            page.locator('#chat-input').fill('这一页的独立草稿')
            page.wait_for_timeout(1100)
            page.reload(wait_until='networkidle')
            assert page.locator('#chat-input').input_value() == '这一页的独立草稿'
            page.goto(base+'/my-ai/chat?id='+second['id'], wait_until='networkidle')
            assert page.locator('#chat-input').input_value() == ''
            checks.append('浏览器刷新恢复与独立草稿')
            for route, name in [('', 'overview'), ('reports', 'reports'), ('chat?id='+first['id'], 'chat'), ('preferences', 'preferences')]:
                page.goto(base+'/my-ai'+('/'+route if route else ''), wait_until='networkidle')
                page.screenshot(path=str(output/(name+'.png')), full_page=route not in ('', 'chat?id='+first['id']))
            page.goto(base+'/my-courses#profile', wait_until='networkidle')
            page.wait_for_url('**/my-ai/reports')
            page.go_back(wait_until='networkidle')
            checks.append('旧画像入口迁移')
            browser.close()
        process.terminate();process.wait(timeout=20)
        env=dict(os.environ,DATABASE_PATH=info['database'],HOST='127.0.0.1',PORT=str(urlsplit(base).port),AI_BASE_URL=info['fixture']+'/v1',AI_API_KEY='fictional-fixture-key',AI_MODEL='fixture-v560',GANGYI_LAUNCH_SESSION_ID='v560-ui-isolated')
        restarted=subprocess.Popen([str(Path(executable).resolve())],cwd=Path(__file__).resolve().parents[1],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        try:
            deadline=time.monotonic()+20
            while time.monotonic()<deadline:
                try:
                    restored=api('/api/my-ai/chat?id='+first['id'])
                    break
                except Exception:time.sleep(.1)
            else:raise AssertionError('重启后个人中心未恢复')
            assert restored['messages'] and restored['draft']=='这一页的独立草稿'
            assert api('/api/my-ai/profile')['nickname']=='测试同学'
            checks.append('服务进程重启后聊天、资料和草稿持久恢复')
        finally:
            restarted.terminate();restarted.wait(timeout=20)
        result = {'checks': checks, 'mockRequests': len(PersonalFixture.calls), 'paidRequests': 0, 'realModelVerified': False}
        (output/'result.json').write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
        print(json.dumps(result, ensure_ascii=False))


if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    main()

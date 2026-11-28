# 【逐行批注】 执行当前 Python 语句："""我的AI隔离验收：仅使用模拟模型，保存页面预览与机器可读结果。"""。
"""我的AI隔离验收：仅使用模拟模型，保存页面预览与机器可读结果。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import io。
import io
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sqlite3。
import sqlite3
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import urlsplit。
from urllib.parse import urlsplit
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.error import HTTPError。
from urllib.error import HTTPError
# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import sync_playwright。
from playwright.sync_api import sync_playwright
# 【逐行批注】 导入后续逻辑需要的模块或符号：import classroom_v560_fixture as fixture。
import classroom_v560_fixture as fixture


# 【逐行批注】 定义类 PersonalFixture，组织相关数据与行为。
class PersonalFixture(fixture.FixtureAI):
    # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
    def do_POST(self):
        # 【逐行批注】 计算右侧结果并赋给 raw，更新脚本状态。
        raw = self.rfile.read(int(self.headers['Content-Length']))
        # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
        body = json.loads(raw)
        # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
        context = None
        # 【逐行批注】 遍历数据并重复执行循环体：for message in body.get('messages', []):。
        for message in body.get('messages', []):
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
                value = json.loads(message.get('content', ''))
                # 【逐行批注】 判断条件 value.get('event', {}).get('type') == 'assistant_chat':，成立时执行对应分支。
                if value.get('event', {}).get('type') == 'assistant_chat':
                    # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
                    context = value
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
            # 【逐行批注】 捕获并处理异常：except (ValueError, AttributeError):。
            except (ValueError, AttributeError):
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
        # 【逐行批注】 判断条件 context is None:，成立时执行对应分支。
        if context is None:
            # 【逐行批注】 计算右侧结果并赋给 self.rfile，更新脚本状态。
            self.rfile = io.BytesIO(raw)
            # 【逐行批注】 结束当前函数并返回结果：return super().do_POST()。
            return super().do_POST()
        # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).calls.append(body)。
        type(self).calls.append(body)
        # 【逐行批注】 计算右侧结果并赋给 text，更新脚本状态。
        text = context['event']['text']
        # 【逐行批注】 判断条件 '兼容选项' in text and body.get('stream_options'):，成立时执行对应分支。
        if '兼容选项' in text and body.get('stream_options'):
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).calls.append(body)。
            type(self).calls.append(body)
            # 【逐行批注】 执行当前 Python 语句：self.send_response(400)。
            self.send_response(400)
            # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'application/json')。
            self.send_header('Content-Type', 'application/json')
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(b'{"error":{"message":"stream_options is unsupported"}}')。
            self.wfile.write(b'{"error":{"message":"stream_options is unsupported"}}')
            # 【逐行批注】 结束当前函数并返回结果：return。
            return
        # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
        actions = []
        # 【逐行批注】 判断条件 '先举例' in text:，成立时执行对应分支。
        if '先举例' in text:
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [{'id': 'style-one', 'tool': 'set_teaching_preferences', 'args': {'style': '先举生活中的例子，再说明概念'}}]
            # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
            message = '已将讲解方式调整为先举生活中的例子，再说明概念。你可以在教学偏好页修改或撤回。'
        # 【逐行批注】 当前一条件不成立时继续判断：'记住' in text:。
        elif '记住' in text:
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [{'id': 'memory-one', 'tool': 'remember_preference', 'args': {'content': '喜欢用生活中的例子理解概念', 'source': 'explicit'}}]
            # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
            message = '已记住你的偏好，可在AI记忆页查看和修改。'
        # 【逐行批注】 当前一条件不成立时继续判断：'删除课程' in text:。
        elif '删除课程' in text:
            # 【逐行批注】 计算右侧结果并赋给 target，更新脚本状态。
            target = context['courses'][0]['id']
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [{'id': 'delete-one', 'tool': 'propose_personal_action', 'args': {'kind': 'delete_course', 'target': target, 'reason': '用户明确要求删除课程'}}]
            # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
            message = '已准备删除预览，请先检查影响，再确认执行。'
        # 【逐行批注】 执行前一条件不成立时的分支。
        else:
            # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
            message = '你的个人偏好和学习记录已经读取。可以继续查看学习报告，或告诉我需要调整什么。'
        # 【逐行批注】 计算右侧结果并赋给 decision，更新脚本状态。
        decision = json.dumps({'message': message, 'actions': actions, 'state': 'completed'}, ensure_ascii=False)
        # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
        self.send_response(200)
        # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'text/event-stream')。
        self.send_header('Content-Type', 'text/event-stream')
        # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
        self.end_headers()
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 遍历数据并重复执行循环体：for offset in range(0, len(decision), 12):。
            for offset in range(0, len(decision), 12):
                # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
                event = {'model': 'fixture-personal', 'choices': [{'delta': {'content': decision[offset:offset+12]}, 'finish_reason': None}]}
                # 【逐行批注】 计算右侧结果并赋给 self.wfile.write(('data: '+json.dumps(event,…，更新脚本状态。
                self.wfile.write(('data: '+json.dumps(event, ensure_ascii=False)+'\n\n').encode())
                # 【逐行批注】 执行当前 Python 语句：self.wfile.flush()。
                self.wfile.flush()
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.006)。
                time.sleep(.006)
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(('data: '+json.dumps({'choices': [{'delta': {}, 'finish_reason': 'stop'}…。
            self.wfile.write(('data: '+json.dumps({'choices': [{'delta': {}, 'finish_reason': 'stop'}]})+'\n\n').encode())
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(('data: '+json.dumps({'choices': [], 'usage': {'prompt_tokens': 120, 'co…。
            self.wfile.write(('data: '+json.dumps({'choices': [], 'usage': {'prompt_tokens': 120, 'completion_tokens': 35, 'total_tokens': 155}})+'\n\n').encode())
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(b'data: [DONE]\n\n')。
            self.wfile.write(b'data: [DONE]\n\n')
        # 【逐行批注】 捕获并处理异常：except (BrokenPipeError, ConnectionResetError):。
        except (BrokenPipeError, ConnectionResetError):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 计算右侧结果并赋给 executable，更新脚本状态。
    executable = sys.argv[1]
    # 【逐行批注】 计算右侧结果并赋给 output，更新脚本状态。
    output = Path(sys.argv[2] if len(sys.argv) > 2 else 'verification/my-ai').resolve()
    # 【逐行批注】 计算右侧结果并赋给 output.mkdir(parents，更新脚本状态。
    output.mkdir(parents=True, exist_ok=True)
    # 【逐行批注】 计算右侧结果并赋给 fixture.FixtureAI，更新脚本状态。
    fixture.FixtureAI = PersonalFixture
    # 【逐行批注】 计算右侧结果并赋给 checks，更新脚本状态。
    checks = []
    # 【逐行批注】 在受控上下文中执行资源操作：with fixture.start_session(executable) as (info, api, process):。
    with fixture.start_session(executable) as (info, api, process):
        # 【逐行批注】 计算右侧结果并赋给 base，更新脚本状态。
        base = info['base']
        # 【逐行批注】 定义函数 wait，封装可复用的处理步骤。
        def wait(task_id):
            # 【逐行批注】 计算右侧结果并赋给 until，更新脚本状态。
            until = time.monotonic()+45
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < until:。
            while time.monotonic() < until:
                # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
                task = api('/api/learning-agent?taskId='+task_id)
                # 【逐行批注】 判断条件 task['status'] in ('ready', 'waiting_student'):，成立时执行对应分支。
                if task['status'] in ('ready', 'waiting_student'):
                    # 【逐行批注】 结束当前函数并返回结果：return task。
                    return task
                # 【逐行批注】 判断条件 task['status'] in ('failed', 'superseded'):，成立时执行对应分支。
                if task['status'] in ('failed', 'superseded'):
                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError(task)。
                    raise AssertionError(task)
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.15)。
                time.sleep(.15)
            # 【逐行批注】 在错误情形抛出异常：raise AssertionError('AI任务未完成')。
            raise AssertionError('AI任务未完成')
        # 【逐行批注】 计算右侧结果并赋给 profile，更新脚本状态。
        profile = api('/api/my-ai/profile')
        # 【逐行批注】 调用 api 完成当前脚本或测试步骤：api('/api/my-ai/profile', {'version': profile['version'], 'nickname':…。
        api('/api/my-ai/profile', {'version': profile['version'], 'nickname': '测试同学', 'goal': '这是用于验收的隔离学习档案'})
        # 【逐行批注】 计算右侧结果并赋给 first，更新脚本状态。
        first = api('/api/my-ai/chats', {'title': '教学风格与学习安排'})
        # 【逐行批注】 计算右侧结果并赋给 second，更新脚本状态。
        second = api('/api/my-ai/chats', {'title': '另一段独立聊天'})
        # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
        body = {'id': first['id'], 'requestId': 'preference-one', 'text': '以后请先举例，再讲定义'}
        # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
        task = api('/api/my-ai/chat/send', body)
        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(task['id'])。
        wait(task['id'])
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/my-ai/chat/send', body)['id'] == task['id']。
        assert api('/api/my-ai/chat/send', body)['id'] == task['id']
        # 【逐行批注】 计算右侧结果并赋给 prefs，更新脚本状态。
        prefs = api('/api/my-ai/preferences')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '先举生活' in prefs['style']。
        assert '先举生活' in prefs['style']
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not api('/api/my-ai/chat?id='+second['id'])['messages']。
        assert not api('/api/my-ai/chat?id='+second['id'])['messages']
        # 【逐行批注】 执行当前 Python 语句：checks.append('管家真实工具执行、请求幂等和聊天隔离')。
        checks.append('管家真实工具执行、请求幂等和聊天隔离')
        # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
        task = api('/api/my-ai/chat/send', {'id': first['id'], 'requestId': 'memory-one', 'text': '记住我喜欢生活中的例子'})
        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(task['id'])。
        wait(task['id'])
        # 【逐行批注】 计算右侧结果并赋给 memories，更新脚本状态。
        memories = api('/api/my-ai/memories')['items']
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert memories and memories[0]['source'] == 'explicit'。
        assert memories and memories[0]['source'] == 'explicit'
        # 【逐行批注】 计算右侧结果并赋给 memory，更新脚本状态。
        memory = memories[0]
        # 【逐行批注】 调用 api 完成当前脚本或测试步骤：api('/api/my-ai/memories', {'id': memory['id'], 'version': memory['ve…。
        api('/api/my-ai/memories', {'id': memory['id'], 'version': memory['version'], 'deleted': True})
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not api('/api/my-ai/memories')['items']。
        assert not api('/api/my-ai/memories')['items']
        # 【逐行批注】 执行当前 Python 语句：checks.append('记忆保存、来源与删除')。
        checks.append('记忆保存、来源与删除')
        # 【逐行批注】 计算右侧结果并赋给 usage，更新脚本状态。
        usage = api('/api/my-ai/usage')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert usage['knownTokenCalls'] >= 2 and usage['tokens'] >= 310。
        assert usage['knownTokenCalls'] >= 2 and usage['tokens'] >= 310
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert any(row['tokens'] is None for row in usage['items'])。
        assert any(row['tokens'] is None for row in usage['items'])
        # 【逐行批注】 执行当前 Python 语句：checks.append('流式空choices用量帧与缺失Token区分')。
        checks.append('流式空choices用量帧与缺失Token区分')
        # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
        task = api('/api/my-ai/chat/send', {'id': first['id'], 'requestId': 'compatibility', 'text': '测试兼容选项'})
        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(task['id'])。
        wait(task['id'])
        # 【逐行批注】 计算右侧结果并赋给 compatibility，更新脚本状态。
        compatibility = api('/api/my-ai/usage')['items']
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert any(r['status'] == 'failed' for r in compatibility)。
        assert any(r['status'] == 'failed' for r in compatibility)
        # 【逐行批注】 执行当前 Python 语句：checks.append('服务商拒绝用量选项时兼容重试与实际请求计数')。
        checks.append('服务商拒绝用量选项时兼容重试与实际请求计数')
        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as p:。
        with sync_playwright() as p:
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser = p.chromium.launch()
            # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
            page = browser.new_page(viewport={'width': 1440, 'height': 1000}, reduced_motion='reduce')
            # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
            errors = []
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：page.on('pageerror', lambda error: errors.append(str(error)))。
            page.on('pageerror', lambda error: errors.append(str(error)))
            # 【逐行批注】 遍历数据并重复执行循环体：for route in ['', 'reports', 'plan', 'preferences', 'memories', 'tasks', '…。
            for route in ['', 'reports', 'plan', 'preferences', 'memories', 'tasks', 'usage', 'profile', 'settings', 'data', 'chat?id='+first['id']]:
                # 【逐行批注】 计算右侧结果并赋给 page.goto(base+'/my-ai'+('/'+route if route …，更新脚本状态。
                page.goto(base+'/my-ai'+('/'+route if route else ''), wait_until='networkidle')
                # 【逐行批注】 执行当前 Python 语句：page.wait_for_function("!document.querySelector('#my-ai-status').textContent.includes('正在…。
                page.wait_for_function("!document.querySelector('#my-ai-status').textContent.includes('正在读取')")
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not page.locator('#my-ai-status').evaluate('(e)=>e.classList.contains("e…。
                assert not page.locator('#my-ai-status').evaluate('(e)=>e.classList.contains("error")'), page.locator('#my-ai-status').inner_text()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('.my-ai-sidebar [aria-current="page"]').count() == 1。
                assert page.locator('.my-ai-sidebar [aria-current="page"]').count() == 1
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.evaluate('document.documentElement.scrollWidth <= innerWidth+1')。
                assert page.evaluate('document.documentElement.scrollWidth <= innerWidth+1')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, errors。
            assert not errors, errors
            # 【逐行批注】 执行当前 Python 语句：checks.append('11个页面直接打开、资源加载、目录与横向布局')。
            checks.append('11个页面直接打开、资源加载、目录与横向布局')
            # 【逐行批注】 执行当前 Python 语句：page.set_viewport_size({'width': 1100, 'height': 768})。
            page.set_viewport_size({'width': 1100, 'height': 768})
            # 【逐行批注】 计算右侧结果并赋给 page.goto(base+'/my-ai/chat?id，更新脚本状态。
            page.goto(base+'/my-ai/chat?id='+first['id'], wait_until='networkidle')
            # 【逐行批注】 计算右侧结果并赋给 box，更新脚本状态。
            box = page.locator('#send-chat').bounding_box()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert box['y']+box['height'] <= 768。
            assert box['y']+box['height'] <= 768
            # 【逐行批注】 执行当前 Python 语句：page.set_viewport_size({'width': 1440, 'height': 1000})。
            page.set_viewport_size({'width': 1440, 'height': 1000})
            # 【逐行批注】 执行当前 Python 语句：checks.append('较矮电脑窗口聊天输入和发送按钮保持可见')。
            checks.append('较矮电脑窗口聊天输入和发送按钮保持可见')
            # 【逐行批注】 执行当前 Python 语句：page.locator('#chat-input').fill('这一页的独立草稿')。
            page.locator('#chat-input').fill('这一页的独立草稿')
            # 【逐行批注】 执行当前 Python 语句：page.wait_for_timeout(1100)。
            page.wait_for_timeout(1100)
            # 【逐行批注】 计算右侧结果并赋给 page.reload(wait_until，更新脚本状态。
            page.reload(wait_until='networkidle')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('#chat-input').input_value() == '这一页的独立草稿'。
            assert page.locator('#chat-input').input_value() == '这一页的独立草稿'
            # 【逐行批注】 计算右侧结果并赋给 page.goto(base+'/my-ai/chat?id，更新脚本状态。
            page.goto(base+'/my-ai/chat?id='+second['id'], wait_until='networkidle')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('#chat-input').input_value() == ''。
            assert page.locator('#chat-input').input_value() == ''
            # 【逐行批注】 执行当前 Python 语句：checks.append('浏览器刷新恢复与独立草稿')。
            checks.append('浏览器刷新恢复与独立草稿')
            # 【逐行批注】 遍历数据并重复执行循环体：for route, name in [('', 'overview'), ('reports', 'reports'), ('chat?id='+…。
            for route, name in [('', 'overview'), ('reports', 'reports'), ('chat?id='+first['id'], 'chat'), ('preferences', 'preferences')]:
                # 【逐行批注】 计算右侧结果并赋给 page.goto(base+'/my-ai'+('/'+route if route …，更新脚本状态。
                page.goto(base+'/my-ai'+('/'+route if route else ''), wait_until='networkidle')
                # 【逐行批注】 计算右侧结果并赋给 page.screenshot(path，更新脚本状态。
                page.screenshot(path=str(output/(name+'.png')), full_page=route not in ('', 'chat?id='+first['id']))
            # 【逐行批注】 计算右侧结果并赋给 page.goto(base+'/my-courses#profile', wait_u…，更新脚本状态。
            page.goto(base+'/my-courses#profile', wait_until='networkidle')
            # 【逐行批注】 执行当前 Python 语句：page.wait_for_url('** /my-ai/reports')。
            page.wait_for_url('**/my-ai/reports')
            # 【逐行批注】 计算右侧结果并赋给 page.go_back(wait_until，更新脚本状态。
            page.go_back(wait_until='networkidle')
            # 【逐行批注】 执行当前 Python 语句：checks.append('旧画像入口迁移')。
            checks.append('旧画像入口迁移')
            # 【逐行批注】 执行当前 Python 语句：browser.close()。
            browser.close()
        # 【逐行批注】 计算右侧结果并赋给 process.terminate();process.wait(timeout，更新脚本状态。
        process.terminate();process.wait(timeout=20)
        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
        env=dict(os.environ,DATABASE_PATH=info['database'],HOST='127.0.0.1',PORT=str(urlsplit(base).port),AI_BASE_URL=info['fixture']+'/v1',AI_API_KEY='fictional-fixture-key',AI_MODEL='fixture-v560',GANGYI_LAUNCH_SESSION_ID='v560-ui-isolated')
        # 【逐行批注】 计算右侧结果并赋给 restarted，更新脚本状态。
        restarted=subprocess.Popen([str(Path(executable).resolve())],cwd=Path(__file__).resolve().parents[1],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline=time.monotonic()+20
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic()<deadline:。
            while time.monotonic()<deadline:
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 计算右侧结果并赋给 restored，更新脚本状态。
                    restored=api('/api/my-ai/chat?id='+first['id'])
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
                # 【逐行批注】 捕获并处理异常：except Exception:time.sleep(.1)。
                except Exception:time.sleep(.1)
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:raise AssertionError('重启后个人中心未恢复')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert restored['messages'] and restored['draft']=='这一页的独立草稿'。
            assert restored['messages'] and restored['draft']=='这一页的独立草稿'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/my-ai/profile')['nickname']=='测试同学'。
            assert api('/api/my-ai/profile')['nickname']=='测试同学'
            # 【逐行批注】 执行当前 Python 语句：checks.append('服务进程重启后聊天、资料和草稿持久恢复')。
            checks.append('服务进程重启后聊天、资料和草稿持久恢复')
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 计算右侧结果并赋给 restarted.terminate();restarted.wait(timeout，更新脚本状态。
            restarted.terminate();restarted.wait(timeout=20)
        # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
        result = {'checks': checks, 'mockRequests': len(PersonalFixture.calls), 'paidRequests': 0, 'realModelVerified': False}
        # 【逐行批注】 计算右侧结果并赋给 (output/'result.json').write_text(json.dumps…，更新脚本状态。
        (output/'result.json').write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
        # 【逐行批注】 输出当前运行结果：print(json.dumps(result, ensure_ascii=False))。
        print(json.dumps(result, ensure_ascii=False))


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'):，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'):
        # 【逐行批注】 计算右侧结果并赋给 sys.stdout.reconfigure(encoding，更新脚本状态。
        sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()

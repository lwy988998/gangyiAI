# 【逐行批注】 执行当前 Python 语句："""隔离档案内验证双搜索工具、来源落库、分引擎统计和页面呈现，全程使用本机夹具。"""。
"""隔离档案内验证双搜索工具、来源落库、分引擎统计和页面呈现，全程使用本机夹具。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import socket。
import socket
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
# 【逐行批注】 导入后续逻辑需要的模块或符号：from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer。
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 计算右侧结果并赋给 executable，更新脚本状态。
    executable = Path(sys.argv[1]).resolve()
    # 【逐行批注】 计算右侧结果并赋给 counts，更新脚本状态。
    counts = {'deepseek': 0, 'bocha': 0}

    # 【逐行批注】 定义类 Fixture，组织相关数据与行为。
    class Fixture(BaseHTTPRequestHandler):
        # 【逐行批注】 定义函数 log_message，封装可复用的处理步骤。
        def log_message(self, *_args):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass

        # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
        def do_POST(self):
            # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
            body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
            # 【逐行批注】 判断条件 self.path.endswith('/chat/completions'):，成立时执行对应分支。
            if self.path.endswith('/chat/completions'):
                # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
                context = '\n'.join(str(message.get('content', '')) for message in body['messages'])
                # 【逐行批注】 计算右侧结果并赋给 decision，更新脚本状态。
                decision = {'message': '', 'actions': [], 'state': 'completed'}
                # 【逐行批注】 判断条件 '双搜索接口验收' in context and 'search-both' not in context:，成立时执行对应分支。
                if '双搜索接口验收' in context and 'search-both' not in context:
                    # 【逐行批注】 计算右侧结果并赋给 decision['actions']，更新脚本状态。
                    decision['actions'] = [{'id': 'search-both', 'tool': 'search_resources',
                                            # 【逐行批注】 执行当前 Python 语句：'args': {'queries': [{'query': 'fixture dual resources', 'angle': '权威资料', 'count': 3}]}}]。
                                            'args': {'queries': [{'query': 'fixture dual resources', 'angle': '权威资料', 'count': 3}]}}]
                    # 【逐行批注】 计算右侧结果并赋给 decision['state']，更新脚本状态。
                    decision['state'] = 'continue'
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload = {'model': 'offline-fixture', 'choices': [{'message': {'content': json.dumps(decision, ensure_ascii=False)}, 'finish_reason': 'stop'}]}
            # 【逐行批注】 当前一条件不成立时继续判断：self.path == '/v1/tools/search':。
            elif self.path == '/v1/tools/search':
                # 【逐行批注】 计算右侧结果并赋给 counts['deepseek'] +，更新脚本状态。
                counts['deepseek'] += 1
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload = {'results': [{'title': '共同教学资料', 'url': 'https://example.org/study?utm_source=ds',
                                       # 【逐行批注】 执行当前 Python 语句：'summary': '已核对的资料摘要', 'site_name': '示例资料', 'date': None, 'rerank_score': 2.5}]}。
                                       'summary': '已核对的资料摘要', 'site_name': '示例资料', 'date': None, 'rerank_score': 2.5}]}
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 counts['bocha'] +，更新脚本状态。
                counts['bocha'] += 1
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload = {'code': 200, 'data': {'webPages': {'value': [
                    # 【逐行批注】 执行当前 Python 语句：{'name': '共同教学资料', 'url': 'https://example.org/study#reference', 'summary': '同一网页资料'},。
                    {'name': '共同教学资料', 'url': 'https://example.org/study#reference', 'summary': '同一网页资料'},
                    # 【逐行批注】 执行当前 Python 语句：{'name': '博查补充资料', 'url': 'https://example.org/extra', 'summary': '第二个原始来源'}]}}}。
                    {'name': '博查补充资料', 'url': 'https://example.org/extra', 'summary': '第二个原始来源'}]}}}
            # 【逐行批注】 计算右侧结果并赋给 streaming，更新脚本状态。
            streaming = self.path.endswith('/chat/completions') and body.get('stream')
            # 【逐行批注】 判断条件 streaming:，成立时执行对应分支。
            if streaming:
                # 【逐行批注】 计算右侧结果并赋给 delta，更新脚本状态。
                delta = {'model': 'offline-fixture', 'choices': [{'delta': {'content': json.dumps(decision, ensure_ascii=False)}, 'finish_reason': None}]}
                # 【逐行批注】 计算右侧结果并赋给 finish，更新脚本状态。
                finish = {'choices': [{'delta': {}, 'finish_reason': 'stop'}]}
                # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
                data = ('data: ' + json.dumps(delta, ensure_ascii=False) + '\n\ndata: ' + json.dumps(finish) + '\n\ndata: [DONE]\n\n').encode()
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
                data = json.dumps(payload, ensure_ascii=False).encode()
            # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
            self.send_response(200)
            # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'text/event-stream' if streaming else 'application/json')。
            self.send_header('Content-Type', 'text/event-stream' if streaming else 'application/json')
            # 【逐行批注】 调用 str、len 完成当前脚本或测试步骤：self.send_header('Content-Length', str(len(data)))。
            self.send_header('Content-Length', str(len(data)))
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(data)。
            self.wfile.write(data)

    # 【逐行批注】 计算右侧结果并赋给 fixture，更新脚本状态。
    fixture = ThreadingHTTPServer(('127.0.0.1', 0), Fixture)
    # 【逐行批注】 计算右侧结果并赋给 threading.Thread(target，更新脚本状态。
    threading.Thread(target=fixture.serve_forever, daemon=True).start()
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-dual-library-') as folder:。
        with tempfile.TemporaryDirectory(prefix='gangyi-dual-library-') as folder:
            # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as listener:。
            with socket.socket() as listener:
                # 【逐行批注】 执行当前 Python 语句：listener.bind(('127.0.0.1', 0))。
                listener.bind(('127.0.0.1', 0))
                # 【逐行批注】 计算右侧结果并赋给 port，更新脚本状态。
                port = listener.getsockname()[1]
            # 【逐行批注】 计算右侧结果并赋给 base，更新脚本状态。
            base = f'http://127.0.0.1:{port}'
            # 【逐行批注】 计算右侧结果并赋给 mock，更新脚本状态。
            mock = f'http://127.0.0.1:{fixture.server_port}'
            # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
            env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(Path(folder) / 'profile.db'),
                       # 【逐行批注】 计算右侧结果并赋给 AI_API_KEY，更新脚本状态。
                       AI_API_KEY='offline-model', AI_BASE_URL=mock, AI_MODEL='offline-fixture',
                       # 【逐行批注】 计算右侧结果并赋给 DEEPSEEK_SEARCH_API_KEY，更新脚本状态。
                       DEEPSEEK_SEARCH_API_KEY='offline-deepseek', BOCHA_API_KEY='offline-bocha',
                       # 【逐行批注】 计算右侧结果并赋给 DEEPSEEK_SEARCH_BASE_URL，更新脚本状态。
                       DEEPSEEK_SEARCH_BASE_URL=mock, BOCHA_BASE_URL=mock,
                       # 【逐行批注】 计算右侧结果并赋给 RESOURCE_SEARCH_CACHE_DIR，更新脚本状态。
                       RESOURCE_SEARCH_CACHE_DIR=str(Path(folder) / 'cache'), LOCAL_CONTROL_TOKEN='dual-library', NO_PROXY='*')
            # 【逐行批注】 定义函数 api，封装可复用的处理步骤。
            def api(path, body=None):
                # 【逐行批注】 计算右侧结果并赋给 request，更新脚本状态。
                request = Request(base + path, data=None if body is None else json.dumps(body, ensure_ascii=False).encode(),
                                  # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                                  headers={'Content-Type': 'application/json'})
                # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(request, timeout=10) as response:。
                with urlopen(request, timeout=10) as response:
                    # 【逐行批注】 结束当前函数并返回结果：return json.load(response)。
                    return json.load(response)
            # 【逐行批注】 在受控上下文中执行资源操作：with (Path(folder) / 'service.log').open('w', encoding='utf-8') as log:。
            with (Path(folder) / 'service.log').open('w', encoding='utf-8') as log:
                # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
                process = subprocess.Popen([str(executable)], cwd=executable.parent, env=env, stdout=log, stderr=log,
                                           # 【逐行批注】 计算右侧结果并赋给 creationflags，更新脚本状态。
                                           creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(150):。
                    for _ in range(150):
                        # 【逐行批注】 开始可能抛出异常的保护代码块。
                        try:
                            # 【逐行批注】 调用 api 完成当前脚本或测试步骤：api('/health')。
                            api('/health')
                            # 【逐行批注】 执行当前 Python 语句：break。
                            break
                        # 【逐行批注】 捕获并处理异常：except OSError:。
                        except OSError:
                            # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                            time.sleep(.1)
                    # 【逐行批注】 执行前一条件不成立时的分支。
                    else:
                        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('隔离服务未能启动')。
                        raise RuntimeError('隔离服务未能启动')
                    # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
                    task = api('/api/my-ai/library/research', {'goal': '双搜索接口验收', 'courseId': '', 'requestId': 'dual-library'})
                    # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(120):。
                    for _ in range(120):
                        # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                        state = api('/api/learning-agent?taskId=' + task['id'])
                        # 【逐行批注】 判断条件 state['status'] in ('ready', 'failed', 'waiting_student'):，成立时执行对应分支。
                        if state['status'] in ('ready', 'failed', 'waiting_student'):
                            # 【逐行批注】 执行当前 Python 语句：break。
                            break
                        # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                        time.sleep(.1)
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['status'] == 'ready', state。
                    assert state['status'] == 'ready', state
                    # 【逐行批注】 计算右侧结果并赋给 items，更新脚本状态。
                    items = api('/api/my-ai/library')['items']
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(items) == 2, items。
                    assert len(items) == 2, items
                    # 【逐行批注】 计算右侧结果并赋给 shared，更新脚本状态。
                    shared = next(item for item in items if item['url'] == 'https://example.org/study')
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert shared['searchProviders'] == ['deepseek', 'bocha']。
                    assert shared['searchProviders'] == ['deepseek', 'bocha']
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert shared['readStatus'] == 'summary_only' and shared['snapshotId'] == ''。
                    assert shared['readStatus'] == 'summary_only' and shared['snapshotId'] == ''
                    # 【逐行批注】 计算右侧结果并赋给 usage，更新脚本状态。
                    usage = api('/api/my-ai/library/usage')
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert usage['searchRequests'] == 2 and usage['providers']['deepseek']['request…。
                    assert usage['searchRequests'] == 2 and usage['providers']['deepseek']['requests'] == 1
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert usage['providers']['bocha']['requests'] == 1 and counts == {'deepseek': …。
                    assert usage['providers']['bocha']['requests'] == 1 and counts == {'deepseek': 1, 'bocha': 1}
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert usage['tokens'] is None and all(row['provider'] in counts for row in usa…。
                    assert usage['tokens'] is None and all(row['provider'] in counts for row in usage['items'])
                    # 页面普通读取不会额外搜索；截图只在本机手工验收入口启用。
                    # 【逐行批注】 判断条件 len(sys.argv) > 2:，成立时执行对应分支。
                    if len(sys.argv) > 2:
                        # 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import sync_playwright。
                        from playwright.sync_api import sync_playwright
                        # 【逐行批注】 计算右侧结果并赋给 output，更新脚本状态。
                        output = Path(sys.argv[2]).resolve()
                        # 【逐行批注】 计算右侧结果并赋给 output.mkdir(parents，更新脚本状态。
                        output.mkdir(parents=True, exist_ok=True)
                        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as playwright:。
                        with sync_playwright() as playwright:
                            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
                            browser = playwright.chromium.launch()
                            # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
                            page = browser.new_page(viewport={'width': 1440, 'height': 1000})
                            # 【逐行批注】 执行当前 Python 语句：page.goto(base + '/my-ai/library')。
                            page.goto(base + '/my-ai/library')
                            # 【逐行批注】 计算右侧结果并赋给 page.get_by_text('检索：DeepSeek、博查', exact，更新脚本状态。
                            page.get_by_text('检索：DeepSeek、博查', exact=False).wait_for()
                            # 【逐行批注】 计算右侧结果并赋给 page.screenshot(path，更新脚本状态。
                            page.screenshot(path=str(output / 'library-desktop.png'), full_page=True)
                            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'DeepSeek：请求 1 次' in page.locator('#library-usage').inner_text()。
                            assert 'DeepSeek：请求 1 次' in page.locator('#library-usage').inner_text()
                            # 【逐行批注】 执行当前 Python 语句：page.set_viewport_size({'width': 390, 'height': 844})。
                            page.set_viewport_size({'width': 390, 'height': 844})
                            # 【逐行批注】 计算右侧结果并赋给 page.screenshot(path，更新脚本状态。
                            page.screenshot(path=str(output / 'library-narrow.png'), full_page=True)
                            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.evaluate('document.documentElement.scrollWidth <= innerWidth')。
                            assert page.evaluate('document.documentElement.scrollWidth <= innerWidth')
                            # 【逐行批注】 执行当前 Python 语句：page.goto(base + '/my-ai/usage')。
                            page.goto(base + '/my-ai/usage')
                            # 【逐行批注】 计算右侧结果并赋给 page.get_by_text('DeepSeek 搜索请求', exact，更新脚本状态。
                            page.get_by_text('DeepSeek 搜索请求', exact=True).wait_for()
                            # 【逐行批注】 计算右侧结果并赋给 page.screenshot(path，更新脚本状态。
                            page.screenshot(path=str(output / 'usage.png'), full_page=True)
                            # 【逐行批注】 执行当前 Python 语句：browser.close()。
                            browser.close()
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert counts == {'deepseek': 1, 'bocha': 1}。
                    assert counts == {'deepseek': 1, 'bocha': 1}
                    # 【逐行批注】 输出当前运行结果：print('双搜索工具落库、共同来源去重、独立请求统计、摘要与原文区分、页面读取零搜索通过；外网请求0。')。
                    print('双搜索工具落库、共同来源去重、独立请求统计、摘要与原文区分、页面读取零搜索通过；外网请求0。')
                # 【逐行批注】 执行当前 Python 语句：finally:。
                finally:
                    # 【逐行批注】 开始可能抛出异常的保护代码块。
                    try:
                        # 【逐行批注】 计算右侧结果并赋给 request，更新脚本状态。
                        request = Request(base + '/internal/shutdown', data=b'', headers={'X-Gangyi-Control-Token': 'dual-library'})
                        # 【逐行批注】 计算右侧结果并赋给 urlopen(request, timeout，更新脚本状态。
                        urlopen(request, timeout=5).close()
                        # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                        process.wait(timeout=15)
                    # 【逐行批注】 执行当前 Python 语句：finally:。
                    finally:
                        # 【逐行批注】 判断条件 process.poll() is None:，成立时执行对应分支。
                        if process.poll() is None:
                            # 【逐行批注】 执行当前 Python 语句：process.kill()。
                            process.kill()
                            # 【逐行批注】 执行当前 Python 语句：process.wait()。
                            process.wait()
    # 【逐行批注】 执行当前 Python 语句：finally:。
    finally:
        # 【逐行批注】 执行当前 Python 语句：fixture.shutdown()。
        fixture.shutdown()
        # 【逐行批注】 执行当前 Python 语句：fixture.server_close()。
        fixture.server_close()


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()

"""隔离档案内验证双搜索工具、来源落库、分引擎统计和页面呈现，全程使用本机夹具。"""
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.request import Request, urlopen


def main():
    executable = Path(sys.argv[1]).resolve()
    counts = {'deepseek': 0, 'bocha': 0}

    class Fixture(BaseHTTPRequestHandler):
        def log_message(self, *_args):
            pass

        def do_POST(self):
            body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
            if self.path.endswith('/chat/completions'):
                context = '\n'.join(str(message.get('content', '')) for message in body['messages'])
                decision = {'message': '', 'actions': [], 'state': 'completed'}
                if '双搜索接口验收' in context and 'search-both' not in context:
                    decision['actions'] = [{'id': 'search-both', 'tool': 'search_resources',
                                            'args': {'queries': [{'query': 'fixture dual resources', 'angle': '权威资料', 'count': 3}]}}]
                    decision['state'] = 'continue'
                payload = {'model': 'offline-fixture', 'choices': [{'message': {'content': json.dumps(decision, ensure_ascii=False)}, 'finish_reason': 'stop'}]}
            elif self.path == '/v1/tools/search':
                counts['deepseek'] += 1
                payload = {'results': [{'title': '共同教学资料', 'url': 'https://example.org/study?utm_source=ds',
                                       'summary': '已核对的资料摘要', 'site_name': '示例资料', 'date': None, 'rerank_score': 2.5}]}
            else:
                counts['bocha'] += 1
                payload = {'code': 200, 'data': {'webPages': {'value': [
                    {'name': '共同教学资料', 'url': 'https://example.org/study#reference', 'summary': '同一网页资料'},
                    {'name': '博查补充资料', 'url': 'https://example.org/extra', 'summary': '第二个原始来源'}]}}}
            streaming = self.path.endswith('/chat/completions') and body.get('stream')
            if streaming:
                delta = {'model': 'offline-fixture', 'choices': [{'delta': {'content': json.dumps(decision, ensure_ascii=False)}, 'finish_reason': None}]}
                finish = {'choices': [{'delta': {}, 'finish_reason': 'stop'}]}
                data = ('data: ' + json.dumps(delta, ensure_ascii=False) + '\n\ndata: ' + json.dumps(finish) + '\n\ndata: [DONE]\n\n').encode()
            else:
                data = json.dumps(payload, ensure_ascii=False).encode()
            self.send_response(200)
            self.send_header('Content-Type', 'text/event-stream' if streaming else 'application/json')
            self.send_header('Content-Length', str(len(data)))
            self.end_headers()
            self.wfile.write(data)

    fixture = ThreadingHTTPServer(('127.0.0.1', 0), Fixture)
    threading.Thread(target=fixture.serve_forever, daemon=True).start()
    try:
        with tempfile.TemporaryDirectory(prefix='gangyi-dual-library-') as folder:
            with socket.socket() as listener:
                listener.bind(('127.0.0.1', 0))
                port = listener.getsockname()[1]
            base = f'http://127.0.0.1:{port}'
            mock = f'http://127.0.0.1:{fixture.server_port}'
            env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(Path(folder) / 'profile.db'),
                       AI_API_KEY='offline-model', AI_BASE_URL=mock, AI_MODEL='offline-fixture',
                       DEEPSEEK_SEARCH_API_KEY='offline-deepseek', BOCHA_API_KEY='offline-bocha',
                       DEEPSEEK_SEARCH_BASE_URL=mock, BOCHA_BASE_URL=mock,
                       RESOURCE_SEARCH_CACHE_DIR=str(Path(folder) / 'cache'), LOCAL_CONTROL_TOKEN='dual-library', NO_PROXY='*')
            def api(path, body=None):
                request = Request(base + path, data=None if body is None else json.dumps(body, ensure_ascii=False).encode(),
                                  headers={'Content-Type': 'application/json'})
                with urlopen(request, timeout=10) as response:
                    return json.load(response)
            with (Path(folder) / 'service.log').open('w', encoding='utf-8') as log:
                process = subprocess.Popen([str(executable)], cwd=executable.parent, env=env, stdout=log, stderr=log,
                                           creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
                try:
                    for _ in range(150):
                        try:
                            api('/health')
                            break
                        except OSError:
                            time.sleep(.1)
                    else:
                        raise RuntimeError('隔离服务未能启动')
                    task = api('/api/my-ai/library/research', {'goal': '双搜索接口验收', 'courseId': '', 'requestId': 'dual-library'})
                    for _ in range(120):
                        state = api('/api/learning-agent?taskId=' + task['id'])
                        if state['status'] in ('ready', 'failed', 'waiting_student'):
                            break
                        time.sleep(.1)
                    assert state['status'] == 'ready', state
                    items = api('/api/my-ai/library')['items']
                    assert len(items) == 2, items
                    shared = next(item for item in items if item['url'] == 'https://example.org/study')
                    assert shared['searchProviders'] == ['deepseek', 'bocha']
                    assert shared['readStatus'] == 'summary_only' and shared['snapshotId'] == ''
                    usage = api('/api/my-ai/library/usage')
                    assert usage['searchRequests'] == 2 and usage['providers']['deepseek']['requests'] == 1
                    assert usage['providers']['bocha']['requests'] == 1 and counts == {'deepseek': 1, 'bocha': 1}
                    assert usage['tokens'] is None and all(row['provider'] in counts for row in usage['items'])
                    # 页面普通读取不会额外搜索；截图只在本机手工验收入口启用。
                    if len(sys.argv) > 2:
                        from playwright.sync_api import sync_playwright
                        output = Path(sys.argv[2]).resolve()
                        output.mkdir(parents=True, exist_ok=True)
                        with sync_playwright() as playwright:
                            browser = playwright.chromium.launch()
                            page = browser.new_page(viewport={'width': 1440, 'height': 1000})
                            page.goto(base + '/my-ai/library')
                            page.get_by_text('检索：DeepSeek、博查', exact=False).wait_for()
                            page.screenshot(path=str(output / 'library-desktop.png'), full_page=True)
                            assert 'DeepSeek：请求 1 次' in page.locator('#library-usage').inner_text()
                            page.set_viewport_size({'width': 390, 'height': 844})
                            page.screenshot(path=str(output / 'library-narrow.png'), full_page=True)
                            assert page.evaluate('document.documentElement.scrollWidth <= innerWidth')
                            page.goto(base + '/my-ai/usage')
                            page.get_by_text('DeepSeek 搜索请求', exact=True).wait_for()
                            page.screenshot(path=str(output / 'usage.png'), full_page=True)
                            browser.close()
                    assert counts == {'deepseek': 1, 'bocha': 1}
                    print('双搜索工具落库、共同来源去重、独立请求统计、摘要与原文区分、页面读取零搜索通过；外网请求0。')
                finally:
                    try:
                        request = Request(base + '/internal/shutdown', data=b'', headers={'X-Gangyi-Control-Token': 'dual-library'})
                        urlopen(request, timeout=5).close()
                        process.wait(timeout=15)
                    finally:
                        if process.poll() is None:
                            process.kill()
                            process.wait()
    finally:
        fixture.shutdown()
        fixture.server_close()


if __name__ == '__main__':
    main()

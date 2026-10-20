"""使用本机 HTTP 服务验证真实 C++ 双搜索链路，不读取凭据或访问外网。"""
import datetime
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer


def main():
    driver = str(Path(sys.argv[1]).resolve())
    lock = threading.Lock()
    received = []
    barrier = None
    mode = 'normal'

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_args):
            pass

        def do_POST(self):
            body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
            provider = 'deepseek' if self.path == '/v1/tools/search' else 'bocha'
            with lock:
                received.append((provider, body, self.headers.get('Authorization')))
            if barrier:
                try:
                    barrier.wait(4)
                except threading.BrokenBarrierError:
                    self.send_error(500)
                    return
            if mode == 'cancel':
                time.sleep(2)
            status = 401 if mode == 'auth' and provider == 'deepseek' else 200
            if provider == 'deepseek':
                payload = {'results': [
                    {'title': '共同来源', 'url': 'https://EXAMPLE.org:443/shared?utm_source=ds#one',
                     'summary': 'DeepSeek 提供的摘要', 'date': None, 'site_name': None, 'rerank_score': 2.9},
                    {'title': 'DeepSeek 独有', 'url': 'https://example.org/ds', 'snippet': '原始摘要',
                     'date': '2026-10-17T00:00:00Z', 'site_name': '原网站', 'rerank_score': 2.2}]}
            else:
                payload = {'code': 200, 'data': {'webPages': {'value': [
                    {'name': '共同来源', 'url': 'https://example.org/shared?utm_medium=bocha#two', 'summary': '博查摘要'},
                    {'name': '博查独有', 'url': 'https://example.org/bocha', 'summary': '第三条资料'}]}}}
            if mode == 'invalid' and provider == 'deepseek':
                payload = {'unexpected': True}
            if mode == 'empty':
                payload = {'results': []} if provider == 'deepseek' else {'code': 200, 'data': {'webPages': {'value': []}}}
            data = json.dumps(payload, ensure_ascii=False).encode()
            self.send_response(status)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(data)))
            self.end_headers()
            try:
                self.wfile.write(data)
            except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
                pass

    server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        with tempfile.TemporaryDirectory(prefix='gangyi-dual-') as folder:
            env = dict(os.environ, DEEPSEEK_SEARCH_API_KEY='offline-deepseek', BOCHA_API_KEY='offline-bocha',
                       DEEPSEEK_SEARCH_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                       BOCHA_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                       RESOURCE_SEARCH_CACHE_DIR=folder, NO_PROXY='*')
            def run(*requests, **overrides):
                effective = dict(env, **overrides)
                proc = subprocess.run([driver], input=''.join(json.dumps(row, ensure_ascii=False) + '\n' for row in requests),
                                      text=True, encoding='utf-8', capture_output=True, env=effective, timeout=30)
                assert proc.returncode == 0, proc.stderr
                rows = [json.loads(row) for row in proc.stdout.splitlines()]
                assert len(rows) == len(requests), proc.stdout
                return rows

            # 屏障只有在两个 HTTP 请求均已到达后放行，串行实现无法通过。
            barrier = threading.Barrier(2)
            first, cached, refreshed = run({'query': '分数概念'}, {'query': '分数概念'}, {'query': '分数概念', 'refresh': True})
            assert first['status'] == 'completed' and first['attempts'] == 2
            assert len(first['resources']) == 3 and first['resources'][0]['url'] == 'https://example.org/shared'
            assert first['resources'][0]['providers'] == ['deepseek', 'bocha']
            assert first['resources'][0]['source'] != 'deepseek' and first['resources'][0]['publishedAt'] == ''
            assert cached['cached'] and cached['attempts'] == 0 and refreshed['attempts'] == 2
            assert cached['resources'] == first['resources'] and len(received) == 4
            assert {row[2] for row in received} == {'Bearer offline-deepseek', 'Bearer offline-bocha'}
            assert all(row[1]['need_content'] is False for row in received if row[0] == 'deepseek')
            barrier = None
            for failure in ('auth', 'invalid'):
                mode = failure
                row = run({'query': failure})[0]
                assert row['status'] == 'partial' and len(row['resources']) == 2
                assert row['providers'][0]['status'] == ('http_401' if failure == 'auth' else 'invalid_response')
                mode = 'normal'
                row = run({'query': failure})[0]
                assert row['status'] == 'completed' and row['attempts'] == 1 and row['providers'][1]['cached']
            mode = 'empty'
            row = run({'query': '空结果'})[0]
            assert row['status'] == 'completed' and row['resources'] == []
            mode = 'normal'
            row = run({'query': '日期', 'freshness': '2026-01-01..2026-10-20'})[0]
            assert row['status'] == 'completed'
            assert any(body['query'] == '日期 after:2026-01-01 before:2026-10-20' for provider, body, _ in received if provider == 'deepseek')
            assert any(body.get('freshness') == '2026-01-01..2026-10-20' for provider, body, _ in received if provider == 'bocha')
            row = run({'query': '中' * 128})[0]
            assert row['status'] == 'completed'
            row = run({'query': '中' * 129})[0]
            assert row['status'] == 'partial' and row['providers'][0]['status'] == 'query_too_long'
            before = len(received)
            row = run({'query': '预先取消', 'cancelAfterMs': 0})[0]
            assert 'error' in row and len(received) == before
            row = run({'query': '缺密钥'}, DEEPSEEK_SEARCH_API_KEY='', DEEPSEEK_API_KEY='',
                      AI_BASE_URL='https://api.other-provider.invalid', AI_API_KEY='never-send-this-key')[0]
            assert row['status'] == 'partial' and row['providers'][0]['status'] == 'missing_key'
            assert not any(auth == 'Bearer never-send-this-key' for _, _, auth in received)
            # 旧建课入口的每个搜索角度也会进入同一双引擎实现。
            before = len(received)
            row = run({'query': 'Python', 'legacy': True})[0]
            assert len(received) - before == 10 and len(row['resources']) == 3
            assert row['resources'][0]['providers'] == ['deepseek', 'bocha']
            mode = 'cancel'
            row = run({'query': '请求中取消', 'cancelAfterMs': 100})[0]
            assert row['status'] == 'cancelled' and row['resources'] == [] and row['attempts'] == 2
            print('双搜索并行、去重、来源、缓存、刷新、恢复、日期、中文长度、缺密钥、旧入口与取消均通过；外网请求0。')
    finally:
        server.shutdown()
        server.server_close()


if __name__ == '__main__':
    main()

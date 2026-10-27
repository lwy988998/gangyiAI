"""在本机服务器核对共享请求的实际次数、取消隔离和刷新语义。"""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from search_refresh_checks import run_refresh_checks


def main():
    received = []
    lock = threading.Lock()
    mode = 'normal'

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_args):
            pass

        def do_POST(self):
            body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
            with lock:
                received.append((self.path, body))
            time.sleep(0.35)
            deepseek = self.path == '/v1/tools/search'
            payload = {'results': [{'title': '相同原始资料', 'url': 'https://example.org/source', 'summary': '保留完整摘要'}]}
            self.send_response(401 if mode == 'failure' and deepseek else 200)
            data = json.dumps(payload).encode()
            self.send_header('Content-Length', str(len(data)))
            self.end_headers()
            try:
                self.wfile.write(data)
            except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
                pass

    class FixtureServer(ThreadingHTTPServer):
        # Python 3.12 默认队列仅五个连接，不能承接关闭共享时的八路并发。
        request_queue_size = 64

    server = FixtureServer(('127.0.0.1', 0), Handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    try:
        with tempfile.TemporaryDirectory(prefix='search-reuse-') as folder:
            env = dict(os.environ, DEEPSEEK_SEARCH_API_KEY='fixture-ds', BOCHA_API_KEY='fixture-bocha',
                       DEEPSEEK_SEARCH_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                       BOCHA_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                       RESOURCE_SEARCH_CACHE_DIR=folder, NO_PROXY='*')
            env.pop('GANGYI_EVAL_SEARCH_BUDGET_FILE', None)

            def run(items, **settings):
                before = len(received)
                process = subprocess.run([str(Path(sys.argv[1]).resolve())],
                    input=json.dumps({'parallel': items})+'\n', text=True, encoding='utf-8',
                    capture_output=True, env=dict(env, **settings), timeout=15)
                assert process.returncode == 0, process.stderr
                rows = json.loads(process.stdout)['parallel']
                assert all('error' not in row for row in rows), rows
                assert sum(row['attempts'] for row in rows) == len(received)-before, rows
                return rows, len(received)-before

            rows, count = run([{'query': '同一操作'} for _ in range(6)])
            assert count == 2 and all(row['status'] == 'completed' for row in rows)
            assert all(row['resources'] == rows[0]['resources'] for row in rows)
            assert any(row['shared'] for row in rows)
            rows, count = run([{'query': '同一操作'}])
            assert count == 0 and rows[0]['cached']
            rows, count = run([{'query': '关闭去重'} for _ in range(4)], GANGYI_SEARCH_SINGLEFLIGHT='0')
            assert count == 8
            for label, cancellations in [('发起者取消', [100, -1]), ('等待者取消', [-1, 100])]:
                rows, count = run([{'query': label, 'cancelAfterMs': limit, 'startDelayMs': index*30}
                                   for index, limit in enumerate(cancellations)])
                assert count == 2
                for row, limit in zip(rows, cancellations):
                    assert row['status'] == ('cancelled' if limit >= 0 else 'completed'), rows
                    if limit >= 0:
                        assert row['elapsedMs'] < 300 and not row['resources'], rows
            rows, count = run([{'query': '全部取消', 'cancelAfterMs': 80},
                               {'query': '全部取消', 'cancelAfterMs': 100, 'startDelayMs': 20}])
            assert all(row['status'] == 'cancelled' and not row['resources'] for row in rows)
            rows, count = run([{'query': '全部取消'}])
            assert count == 2 and rows[0]['status'] == 'completed' and not rows[0]['cached']
            rows, count = run([{'query': '强制刷新'}, {'query': '强制刷新', 'refresh': True, 'startDelayMs': 30}])
            assert count == 4
            rows, count = run([{'query': '不同范围', 'scopeId': 'one'}, {'query': '不同范围', 'scopeId': 'two'}])
            assert count == 4
            mode = 'failure'
            rows, count = run([{'query': '失败恢复'} for _ in range(3)])
            assert count == 2 and all(row['status'] == 'partial' for row in rows)
            mode = 'normal'
            rows, count = run([{'query': '失败恢复'}])
            assert count == 1 and rows[0]['status'] == 'completed'
            if os.name == 'nt':
                ledger = Path(folder)/'evaluation-budget.json'
                ledger.write_text(json.dumps({'limit': 20, 'attempts': 18, 'reserved': 0}), encoding='utf-8')
                rows, count = run([{'query': '预算内最后两次'}], GANGYI_EVAL_SEARCH_BUDGET_FILE=str(ledger))
                assert count == 2 and json.loads(ledger.read_text(encoding='utf-8'))['attempts'] == 20
                rows, count = run([{'query': '预算不足不发送'}], GANGYI_EVAL_SEARCH_BUDGET_FILE=str(ledger))
                assert count == 0 and rows[0]['status'] == 'failed'
                assert json.loads(ledger.read_text(encoding='utf-8'))['attempts'] == 20
            run_refresh_checks(sys.argv[1])
            print('并发六次双搜索仅2次HTTP；取消隔离、刷新世代、跨进程、缓存失效边界、范围隔离、失败恢复和关闭开关通过；外网请求0。')
    finally:
        server.shutdown()
        server.server_close()


if __name__ == '__main__':
    main()

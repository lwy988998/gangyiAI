# 【逐行批注】 执行当前 Python 语句："""在本机服务器核对共享请求的实际次数、取消隔离和刷新语义。"""。
"""在本机服务器核对共享请求的实际次数、取消隔离和刷新语义。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
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
# 【逐行批注】 导入后续逻辑需要的模块或符号：from search_refresh_checks import run_refresh_checks。
from search_refresh_checks import run_refresh_checks


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 计算右侧结果并赋给 received，更新脚本状态。
    received = []
    # 【逐行批注】 计算右侧结果并赋给 lock，更新脚本状态。
    lock = threading.Lock()
    # 【逐行批注】 计算右侧结果并赋给 mode，更新脚本状态。
    mode = 'normal'

    # 【逐行批注】 定义类 Handler，组织相关数据与行为。
    class Handler(BaseHTTPRequestHandler):
        # 【逐行批注】 定义函数 log_message，封装可复用的处理步骤。
        def log_message(self, *_args):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass

        # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
        def do_POST(self):
            # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
            body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
            # 【逐行批注】 在受控上下文中执行资源操作：with lock:。
            with lock:
                # 【逐行批注】 执行当前 Python 语句：received.append((self.path, body))。
                received.append((self.path, body))
            # 【逐行批注】 执行当前 Python 语句：time.sleep(0.35)。
            time.sleep(0.35)
            # 【逐行批注】 计算右侧结果并赋给 deepseek，更新脚本状态。
            deepseek = self.path == '/v1/tools/search'
            # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
            payload = {'results': [{'title': '相同原始资料', 'url': 'https://example.org/source', 'summary': '保留完整摘要'}]}
            # 【逐行批注】 执行当前 Python 语句：self.send_response(401 if mode == 'failure' and deepseek else 200)。
            self.send_response(401 if mode == 'failure' and deepseek else 200)
            # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
            data = json.dumps(payload).encode()
            # 【逐行批注】 调用 str、len 完成当前脚本或测试步骤：self.send_header('Content-Length', str(len(data)))。
            self.send_header('Content-Length', str(len(data)))
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 执行当前 Python 语句：self.wfile.write(data)。
                self.wfile.write(data)
            # 【逐行批注】 捕获并处理异常：except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):。
            except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass

    # 【逐行批注】 定义类 FixtureServer，组织相关数据与行为。
    class FixtureServer(ThreadingHTTPServer):
        # Python 3.12 默认队列仅五个连接，不能承接关闭共享时的八路并发。
        # 【逐行批注】 计算右侧结果并赋给 request_queue_size，更新脚本状态。
        request_queue_size = 64

    # 【逐行批注】 计算右侧结果并赋给 server，更新脚本状态。
    server = FixtureServer(('127.0.0.1', 0), Handler)
    # 【逐行批注】 计算右侧结果并赋给 threading.Thread(target，更新脚本状态。
    threading.Thread(target=server.serve_forever, daemon=True).start()
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='search-reuse-') as folder:。
        with tempfile.TemporaryDirectory(prefix='search-reuse-') as folder:
            # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
            env = dict(os.environ, DEEPSEEK_SEARCH_API_KEY='fixture-ds', BOCHA_API_KEY='fixture-bocha',
                       # 【逐行批注】 计算右侧结果并赋给 DEEPSEEK_SEARCH_BASE_URL，更新脚本状态。
                       DEEPSEEK_SEARCH_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                       # 【逐行批注】 计算右侧结果并赋给 BOCHA_BASE_URL，更新脚本状态。
                       BOCHA_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                       # 【逐行批注】 计算右侧结果并赋给 RESOURCE_SEARCH_CACHE_DIR，更新脚本状态。
                       RESOURCE_SEARCH_CACHE_DIR=folder, NO_PROXY='*')
            # 【逐行批注】 执行当前 Python 语句：env.pop('GANGYI_EVAL_SEARCH_BUDGET_FILE', None)。
            env.pop('GANGYI_EVAL_SEARCH_BUDGET_FILE', None)

            # 【逐行批注】 定义函数 run，封装可复用的处理步骤。
            def run(items, **settings):
                # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
                before = len(received)
                # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
                process = subprocess.run([str(Path(sys.argv[1]).resolve())],
                    # 【逐行批注】 计算右侧结果并赋给 input，更新脚本状态。
                    input=json.dumps({'parallel': items})+'\n', text=True, encoding='utf-8',
                    # 【逐行批注】 计算右侧结果并赋给 capture_output，更新脚本状态。
                    capture_output=True, env=dict(env, **settings), timeout=15)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert process.returncode == 0, process.stderr。
                assert process.returncode == 0, process.stderr
                # 【逐行批注】 计算右侧结果并赋给 rows，更新脚本状态。
                rows = json.loads(process.stdout)['parallel']
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all('error' not in row for row in rows), rows。
                assert all('error' not in row for row in rows), rows
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert sum(row['attempts'] for row in rows) == len(received)-before, rows。
                assert sum(row['attempts'] for row in rows) == len(received)-before, rows
                # 【逐行批注】 结束当前函数并返回结果：return rows, len(received)-before。
                return rows, len(received)-before

            # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
            rows, count = run([{'query': '同一操作'} for _ in range(6)])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 2 and all(row['status'] == 'completed' for row in rows)。
            assert count == 2 and all(row['status'] == 'completed' for row in rows)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(row['resources'] == rows[0]['resources'] for row in rows)。
            assert all(row['resources'] == rows[0]['resources'] for row in rows)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert any(row['shared'] for row in rows)。
            assert any(row['shared'] for row in rows)
            # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
            rows, count = run([{'query': '同一操作'}])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 0 and rows[0]['cached']。
            assert count == 0 and rows[0]['cached']
            # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
            rows, count = run([{'query': '关闭去重'} for _ in range(4)], GANGYI_SEARCH_SINGLEFLIGHT='0')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 8。
            assert count == 8
            # 【逐行批注】 遍历数据并重复执行循环体：for label, cancellations in [('发起者取消', [100, -1]), ('等待者取消', [-1, 100])]:。
            for label, cancellations in [('发起者取消', [100, -1]), ('等待者取消', [-1, 100])]:
                # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
                rows, count = run([{'query': label, 'cancelAfterMs': limit, 'startDelayMs': index*30}
                                   # 【逐行批注】 遍历数据并重复执行循环体：for index, limit in enumerate(cancellations)])。
                                   for index, limit in enumerate(cancellations)])
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 2。
                assert count == 2
                # 【逐行批注】 遍历数据并重复执行循环体：for row, limit in zip(rows, cancellations):。
                for row, limit in zip(rows, cancellations):
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == ('cancelled' if limit >= 0 else 'completed'), rows。
                    assert row['status'] == ('cancelled' if limit >= 0 else 'completed'), rows
                    # 【逐行批注】 判断条件 limit >= 0:，成立时执行对应分支。
                    if limit >= 0:
                        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['elapsedMs'] < 300 and not row['resources'], rows。
                        assert row['elapsedMs'] < 300 and not row['resources'], rows
            # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
            rows, count = run([{'query': '全部取消', 'cancelAfterMs': 80},
                               # 【逐行批注】 执行当前 Python 语句：{'query': '全部取消', 'cancelAfterMs': 100, 'startDelayMs': 20}])。
                               {'query': '全部取消', 'cancelAfterMs': 100, 'startDelayMs': 20}])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(row['status'] == 'cancelled' and not row['resources'] for row in row…。
            assert all(row['status'] == 'cancelled' and not row['resources'] for row in rows)
            # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
            rows, count = run([{'query': '全部取消'}])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 2 and rows[0]['status'] == 'completed' and not rows[0]['cached']。
            assert count == 2 and rows[0]['status'] == 'completed' and not rows[0]['cached']
            # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
            rows, count = run([{'query': '强制刷新'}, {'query': '强制刷新', 'refresh': True, 'startDelayMs': 30}])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 4。
            assert count == 4
            # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
            rows, count = run([{'query': '不同范围', 'scopeId': 'one'}, {'query': '不同范围', 'scopeId': 'two'}])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 4。
            assert count == 4
            # 【逐行批注】 计算右侧结果并赋给 mode，更新脚本状态。
            mode = 'failure'
            # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
            rows, count = run([{'query': '失败恢复'} for _ in range(3)])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 2 and all(row['status'] == 'partial' for row in rows)。
            assert count == 2 and all(row['status'] == 'partial' for row in rows)
            # 【逐行批注】 计算右侧结果并赋给 mode，更新脚本状态。
            mode = 'normal'
            # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
            rows, count = run([{'query': '失败恢复'}])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 1 and rows[0]['status'] == 'completed'。
            assert count == 1 and rows[0]['status'] == 'completed'
            # 【逐行批注】 判断条件 os.name == 'nt':，成立时执行对应分支。
            if os.name == 'nt':
                # 【逐行批注】 计算右侧结果并赋给 ledger，更新脚本状态。
                ledger = Path(folder)/'evaluation-budget.json'
                # 【逐行批注】 计算右侧结果并赋给 ledger.write_text(json.dumps({'limit': 20, '…，更新脚本状态。
                ledger.write_text(json.dumps({'limit': 20, 'attempts': 18, 'reserved': 0}), encoding='utf-8')
                # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
                rows, count = run([{'query': '预算内最后两次'}], GANGYI_EVAL_SEARCH_BUDGET_FILE=str(ledger))
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 2 and json.loads(ledger.read_text(encoding='utf-8'))['attempts'…。
                assert count == 2 and json.loads(ledger.read_text(encoding='utf-8'))['attempts'] == 20
                # 【逐行批注】 计算右侧结果并赋给 rows, count，更新脚本状态。
                rows, count = run([{'query': '预算不足不发送'}], GANGYI_EVAL_SEARCH_BUDGET_FILE=str(ledger))
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert count == 0 and rows[0]['status'] == 'failed'。
                assert count == 0 and rows[0]['status'] == 'failed'
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert json.loads(ledger.read_text(encoding='utf-8'))['attempts'] == 20。
                assert json.loads(ledger.read_text(encoding='utf-8'))['attempts'] == 20
            # 【逐行批注】 调用 run_refresh_checks 完成当前脚本或测试步骤：run_refresh_checks(sys.argv[1])。
            run_refresh_checks(sys.argv[1])
            # 【逐行批注】 输出当前运行结果：print('并发六次双搜索仅2次HTTP；取消隔离、刷新世代、跨进程、缓存失效边界、范围隔离、失败恢复和关闭开关通过；外网请求0。')。
            print('并发六次双搜索仅2次HTTP；取消隔离、刷新世代、跨进程、缓存失效边界、范围隔离、失败恢复和关闭开关通过；外网请求0。')
    # 【逐行批注】 执行当前 Python 语句：finally:。
    finally:
        # 【逐行批注】 执行当前 Python 语句：server.shutdown()。
        server.shutdown()
        # 【逐行批注】 执行当前 Python 语句：server.server_close()。
        server.server_close()


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()

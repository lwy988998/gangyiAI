# 【逐行批注】 执行当前 Python 语句："""使用本机 HTTP 服务验证真实 C++ 双搜索链路，不读取凭据或访问外网。"""。
"""使用本机 HTTP 服务验证真实 C++ 双搜索链路，不读取凭据或访问外网。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import datetime。
import datetime
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


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 计算右侧结果并赋给 driver，更新脚本状态。
    driver = str(Path(sys.argv[1]).resolve())
    # 【逐行批注】 计算右侧结果并赋给 lock，更新脚本状态。
    lock = threading.Lock()
    # 【逐行批注】 计算右侧结果并赋给 received，更新脚本状态。
    received = []
    # 【逐行批注】 计算右侧结果并赋给 barrier，更新脚本状态。
    barrier = None
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
            # 【逐行批注】 计算右侧结果并赋给 provider，更新脚本状态。
            provider = 'deepseek' if self.path == '/v1/tools/search' else 'bocha'
            # 【逐行批注】 在受控上下文中执行资源操作：with lock:。
            with lock:
                # 【逐行批注】 执行当前 Python 语句：received.append((provider, body, self.headers.get('Authorization')))。
                received.append((provider, body, self.headers.get('Authorization')))
            # 【逐行批注】 判断条件 barrier:，成立时执行对应分支。
            if barrier:
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 执行当前 Python 语句：barrier.wait(4)。
                    barrier.wait(4)
                # 【逐行批注】 捕获并处理异常：except threading.BrokenBarrierError:。
                except threading.BrokenBarrierError:
                    # 【逐行批注】 执行当前 Python 语句：self.send_error(500)。
                    self.send_error(500)
                    # 【逐行批注】 结束当前函数并返回结果：return。
                    return
            # 【逐行批注】 判断条件 mode == 'cancel':，成立时执行对应分支。
            if mode == 'cancel':
                # 【逐行批注】 执行当前 Python 语句：time.sleep(2)。
                time.sleep(2)
            # 【逐行批注】 计算右侧结果并赋给 status，更新脚本状态。
            status = 401 if mode == 'auth' and provider == 'deepseek' else 200
            # 【逐行批注】 判断条件 provider == 'deepseek':，成立时执行对应分支。
            if provider == 'deepseek':
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload = {'results': [
                    # 【逐行批注】 计算右侧结果并赋给 {'title': '共同来源', 'url': 'https://EXAMPLE.or…，更新脚本状态。
                    {'title': '共同来源', 'url': 'https://EXAMPLE.org:443/shared?utm_source=ds#one',
                     # 【逐行批注】 执行当前 Python 语句：'summary': 'DeepSeek 提供的摘要', 'date': None, 'site_name': None, 'rerank_score': 2.9},。
                     'summary': 'DeepSeek 提供的摘要', 'date': None, 'site_name': None, 'rerank_score': 2.9},
                    # 【逐行批注】 执行当前 Python 语句：{'title': 'DeepSeek 独有', 'url': 'https://example.org/ds', 'snippet': '原始摘要',。
                    {'title': 'DeepSeek 独有', 'url': 'https://example.org/ds', 'snippet': '原始摘要',
                     # 【逐行批注】 执行当前 Python 语句：'date': '2026-10-17T00:00:00Z', 'site_name': '原网站', 'rerank_score': 2.2}]}。
                     'date': '2026-10-17T00:00:00Z', 'site_name': '原网站', 'rerank_score': 2.2}]}
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload = {'code': 200, 'data': {'webPages': {'value': [
                    # 【逐行批注】 计算右侧结果并赋给 {'name': '共同来源', 'url': 'https://example.org…，更新脚本状态。
                    {'name': '共同来源', 'url': 'https://example.org/shared?utm_medium=bocha#two', 'summary': '博查摘要'},
                    # 【逐行批注】 执行当前 Python 语句：{'name': '博查独有', 'url': 'https://example.org/bocha', 'summary': '第三条资料'}]}}}。
                    {'name': '博查独有', 'url': 'https://example.org/bocha', 'summary': '第三条资料'}]}}}
            # 【逐行批注】 判断条件 mode == 'invalid' and provider == 'deepseek':，成立时执行对应分支。
            if mode == 'invalid' and provider == 'deepseek':
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload = {'unexpected': True}
            # 【逐行批注】 判断条件 mode == 'empty':，成立时执行对应分支。
            if mode == 'empty':
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload = {'results': []} if provider == 'deepseek' else {'code': 200, 'data': {'webPages': {'value': []}}}
            # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
            data = json.dumps(payload, ensure_ascii=False).encode()
            # 【逐行批注】 执行当前 Python 语句：self.send_response(status)。
            self.send_response(status)
            # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'application/json')。
            self.send_header('Content-Type', 'application/json')
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

    # 【逐行批注】 计算右侧结果并赋给 server，更新脚本状态。
    server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    # 【逐行批注】 计算右侧结果并赋给 thread，更新脚本状态。
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    # 【逐行批注】 执行当前 Python 语句：thread.start()。
    thread.start()
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-dual-') as folder:。
        with tempfile.TemporaryDirectory(prefix='gangyi-dual-') as folder:
            # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
            env = dict(os.environ, DEEPSEEK_SEARCH_API_KEY='offline-deepseek', BOCHA_API_KEY='offline-bocha',
                       # 【逐行批注】 计算右侧结果并赋给 DEEPSEEK_SEARCH_BASE_URL，更新脚本状态。
                       DEEPSEEK_SEARCH_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                       # 【逐行批注】 计算右侧结果并赋给 BOCHA_BASE_URL，更新脚本状态。
                       BOCHA_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                       # 【逐行批注】 计算右侧结果并赋给 RESOURCE_SEARCH_CACHE_DIR，更新脚本状态。
                       RESOURCE_SEARCH_CACHE_DIR=folder, NO_PROXY='*')
            # 【逐行批注】 定义函数 run，封装可复用的处理步骤。
            def run(*requests, **overrides):
                # 【逐行批注】 计算右侧结果并赋给 effective，更新脚本状态。
                effective = dict(env, **overrides)
                # 【逐行批注】 计算右侧结果并赋给 proc，更新脚本状态。
                proc = subprocess.run([driver], input=''.join(json.dumps(row, ensure_ascii=False) + '\n' for row in requests),
                                      # 【逐行批注】 计算右侧结果并赋给 text，更新脚本状态。
                                      text=True, encoding='utf-8', capture_output=True, env=effective, timeout=30)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert proc.returncode == 0, proc.stderr。
                assert proc.returncode == 0, proc.stderr
                # 【逐行批注】 计算右侧结果并赋给 rows，更新脚本状态。
                rows = [json.loads(row) for row in proc.stdout.splitlines()]
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(rows) == len(requests), proc.stdout。
                assert len(rows) == len(requests), proc.stdout
                # 【逐行批注】 结束当前函数并返回结果：return rows。
                return rows

            # 屏障只有在两个 HTTP 请求均已到达后放行，串行实现无法通过。
            # 【逐行批注】 计算右侧结果并赋给 barrier，更新脚本状态。
            barrier = threading.Barrier(2)
            # 【逐行批注】 计算右侧结果并赋给 first, cached, refreshed，更新脚本状态。
            first, cached, refreshed = run({'query': '分数概念'}, {'query': '分数概念'}, {'query': '分数概念', 'refresh': True})
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert first['status'] == 'completed' and first['attempts'] == 2。
            assert first['status'] == 'completed' and first['attempts'] == 2
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(first['resources']) == 3 and first['resources'][0]['url'] == 'https:…。
            assert len(first['resources']) == 3 and first['resources'][0]['url'] == 'https://example.org/shared'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert first['resources'][0]['providers'] == ['deepseek', 'bocha']。
            assert first['resources'][0]['providers'] == ['deepseek', 'bocha']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert first['resources'][0]['source'] != 'deepseek' and first['resources'][0][…。
            assert first['resources'][0]['source'] != 'deepseek' and first['resources'][0]['publishedAt'] == ''
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert cached['cached'] and cached['attempts'] == 0 and refreshed['attempts'] =…。
            assert cached['cached'] and cached['attempts'] == 0 and refreshed['attempts'] == 2
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert cached['resources'] == first['resources'] and len(received) == 4。
            assert cached['resources'] == first['resources'] and len(received) == 4
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert {row[2] for row in received} == {'Bearer offline-deepseek', 'Bearer offl…。
            assert {row[2] for row in received} == {'Bearer offline-deepseek', 'Bearer offline-bocha'}
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(row[1]['need_content'] is False for row in received if row[0] == 'de…。
            assert all(row[1]['need_content'] is False for row in received if row[0] == 'deepseek')
            # 【逐行批注】 计算右侧结果并赋给 barrier，更新脚本状态。
            barrier = None
            # 【逐行批注】 遍历数据并重复执行循环体：for failure in ('auth', 'invalid'):。
            for failure in ('auth', 'invalid'):
                # 【逐行批注】 计算右侧结果并赋给 mode，更新脚本状态。
                mode = failure
                # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
                row = run({'query': failure})[0]
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'partial' and len(row['resources']) == 2。
                assert row['status'] == 'partial' and len(row['resources']) == 2
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['providers'][0]['status'] == ('http_401' if failure == 'auth' else '…。
                assert row['providers'][0]['status'] == ('http_401' if failure == 'auth' else 'invalid_response')
                # 【逐行批注】 计算右侧结果并赋给 mode，更新脚本状态。
                mode = 'normal'
                # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
                row = run({'query': failure})[0]
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'completed' and row['attempts'] == 1 and row['providers…。
                assert row['status'] == 'completed' and row['attempts'] == 1 and row['providers'][1]['cached']
            # 【逐行批注】 计算右侧结果并赋给 mode，更新脚本状态。
            mode = 'empty'
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = run({'query': '空结果'})[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'completed' and row['resources'] == []。
            assert row['status'] == 'completed' and row['resources'] == []
            # 【逐行批注】 计算右侧结果并赋给 mode，更新脚本状态。
            mode = 'normal'
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = run({'query': '日期', 'freshness': '2026-01-01..2026-10-20'})[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'completed'。
            assert row['status'] == 'completed'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert any(body['query'] == '日期 after:2026-01-01 before:2026-10-20' for provide…。
            assert any(body['query'] == '日期 after:2026-01-01 before:2026-10-20' for provider, body, _ in received if provider == 'deepseek')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert any(body.get('freshness') == '2026-01-01..2026-10-20' for provider, body…。
            assert any(body.get('freshness') == '2026-01-01..2026-10-20' for provider, body, _ in received if provider == 'bocha')
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = run({'query': '中' * 128})[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'completed'。
            assert row['status'] == 'completed'
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = run({'query': '中' * 129})[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'partial' and row['providers'][0]['status'] == 'query_t…。
            assert row['status'] == 'partial' and row['providers'][0]['status'] == 'query_too_long'
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(received)
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = run({'query': '预先取消', 'cancelAfterMs': 0})[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'error' in row and len(received) == before。
            assert 'error' in row and len(received) == before
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = run({'query': '缺密钥'}, DEEPSEEK_SEARCH_API_KEY='', DEEPSEEK_API_KEY='',
                      # 【逐行批注】 计算右侧结果并赋给 AI_BASE_URL，更新脚本状态。
                      AI_BASE_URL='https://api.other-provider.invalid', AI_API_KEY='never-send-this-key')[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'partial' and row['providers'][0]['status'] == 'missing…。
            assert row['status'] == 'partial' and row['providers'][0]['status'] == 'missing_key'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not any(auth == 'Bearer never-send-this-key' for _, _, auth in received)。
            assert not any(auth == 'Bearer never-send-this-key' for _, _, auth in received)
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = run({'query': '复用官方凭据'}, DEEPSEEK_SEARCH_API_KEY='', DEEPSEEK_API_KEY='',
                      # 【逐行批注】 计算右侧结果并赋给 AI_BASE_URL，更新脚本状态。
                      AI_BASE_URL='https://api.deepseek.com/v1', AI_API_KEY='reuse-deepseek')[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'completed' and any(auth == 'Bearer reuse-deepseek' for…。
            assert row['status'] == 'completed' and any(auth == 'Bearer reuse-deepseek' for _, _, auth in received)
            # 旧建课入口的每个搜索角度也会进入同一双引擎实现。
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(received)
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = run({'query': 'Python', 'legacy': True})[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(received) - before == 10 and len(row['resources']) == 3。
            assert len(received) - before == 10 and len(row['resources']) == 3
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['resources'][0]['providers'] == ['deepseek', 'bocha']。
            assert row['resources'][0]['providers'] == ['deepseek', 'bocha']
            # 【逐行批注】 计算右侧结果并赋给 mode，更新脚本状态。
            mode = 'cancel'
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = run({'query': '请求中取消', 'cancelAfterMs': 100})[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'cancelled' and row['resources'] == [] and row['attempt…。
            assert row['status'] == 'cancelled' and row['resources'] == [] and row['attempts'] == 2
            # 【逐行批注】 输出当前运行结果：print('双搜索并行、去重、来源、缓存、刷新、恢复、日期、中文长度、缺密钥、旧入口与取消均通过；外网请求0。')。
            print('双搜索并行、去重、来源、缓存、刷新、恢复、日期、中文长度、缺密钥、旧入口与取消均通过；外网请求0。')
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

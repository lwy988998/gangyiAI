# 【逐行批注】 执行当前 Python 语句："""用本机信号精确控制搜索完成顺序，核对刷新世代和跨进程缓存提交。"""。
"""用本机信号精确控制搜索完成顺序，核对刷新世代和跨进程缓存提交。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import hashlib。
import hashlib
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


# 【逐行批注】 定义函数 hold_cache_locks，封装可复用的处理步骤。
def hold_cache_locks(directory, ready, release):
    # 【逐行批注】 执行当前 Python 语句："""独立进程持同一系统字节锁，主测试解锁前缓存提交必须能够取消。"""。
    """独立进程持同一系统字节锁，主测试解锁前缓存提交必须能够取消。"""
    # 【逐行批注】 计算右侧结果并赋给 handles，更新脚本状态。
    handles = []
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 计算右侧结果并赋给 files，更新脚本状态。
        files = sorted(Path(directory).glob('*.lock'))
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(files) == 2, files。
        assert len(files) == 2, files
        # 【逐行批注】 遍历数据并重复执行循环体：for path in files:。
        for path in files:
            # 【逐行批注】 计算右侧结果并赋给 stream，更新脚本状态。
            stream = path.open('a+b')
            # 【逐行批注】 执行当前 Python 语句：handles.append(stream)。
            handles.append(stream)
            # 【逐行批注】 判断条件 os.name == 'nt':，成立时执行对应分支。
            if os.name == 'nt':
                # 【逐行批注】 导入后续逻辑需要的模块或符号：import msvcrt。
                import msvcrt
                # 【逐行批注】 执行当前 Python 语句：stream.write(b'\0')。
                stream.write(b'\0')
                # 【逐行批注】 执行当前 Python 语句：stream.flush()。
                stream.flush()
                # 【逐行批注】 执行当前 Python 语句：stream.seek(0)。
                stream.seek(0)
                # 【逐行批注】 执行当前 Python 语句：msvcrt.locking(stream.fileno(), msvcrt.LK_LOCK, 1)。
                msvcrt.locking(stream.fileno(), msvcrt.LK_LOCK, 1)
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 导入后续逻辑需要的模块或符号：import fcntl。
                import fcntl
                # 【逐行批注】 执行当前 Python 语句：fcntl.flock(stream.fileno(), fcntl.LOCK_EX)。
                fcntl.flock(stream.fileno(), fcntl.LOCK_EX)
        # 【逐行批注】 调用 Path 完成当前脚本或测试步骤：Path(ready).touch()。
        Path(ready).touch()
        # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
        deadline = time.monotonic() + 15
        # 【逐行批注】 在条件成立期间重复执行：while not Path(release).exists():。
        while not Path(release).exists():
            # 【逐行批注】 判断条件 time.monotonic() >= deadline:，成立时执行对应分支。
            if time.monotonic() >= deadline:
                # 【逐行批注】 在错误情形抛出异常：raise AssertionError('缓存锁夹具等待解锁超时')。
                raise AssertionError('缓存锁夹具等待解锁超时')
            # 【逐行批注】 执行当前 Python 语句：time.sleep(.01)。
            time.sleep(.01)
    # 【逐行批注】 执行当前 Python 语句：finally:。
    finally:
        # 【逐行批注】 遍历数据并重复执行循环体：for stream in handles:。
        for stream in handles:
            # 【逐行批注】 判断条件 os.name == 'nt':，成立时执行对应分支。
            if os.name == 'nt':
                # 【逐行批注】 执行当前 Python 语句：stream.seek(0)。
                stream.seek(0)
                # 【逐行批注】 执行当前 Python 语句：msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)。
                msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 执行当前 Python 语句：fcntl.flock(stream.fileno(), fcntl.LOCK_UN)。
                fcntl.flock(stream.fileno(), fcntl.LOCK_UN)
            # 【逐行批注】 执行当前 Python 语句：stream.close()。
            stream.close()


# 【逐行批注】 定义函数 run_refresh_checks，封装可复用的处理步骤。
def run_refresh_checks(driver):
    # 【逐行批注】 计算右侧结果并赋给 driver，更新脚本状态。
    driver = str(Path(driver).resolve())
    # 【逐行批注】 计算右侧结果并赋给 received, counts, events, releases, delivered，更新脚本状态。
    received, counts, events, releases, delivered = [], {}, {}, {}, {}
    # 【逐行批注】 计算右侧结果并赋给 lock，更新脚本状态。
    lock = threading.Lock()
    # 【逐行批注】 计算右侧结果并赋给 fixture，更新脚本状态。
    fixture = {}

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
            # 【逐行批注】 计算右侧结果并赋给 query，更新脚本状态。
            query = body['query']
            # 【逐行批注】 在受控上下文中执行资源操作：with lock:。
            with lock:
                # 【逐行批注】 计算右侧结果并赋给 case_root，更新脚本状态。
                case_root = fixture['root']
                # 【逐行批注】 计算右侧结果并赋给 number，更新脚本状态。
                number = counts.get((query, self.path), 0) + 1
                # 【逐行批注】 计算右侧结果并赋给 counts[query, self.path]，更新脚本状态。
                counts[query, self.path] = number
                # 【逐行批注】 执行当前 Python 语句：received.append((query, self.path, number))。
                received.append((query, self.path, number))
                # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
                event = events.setdefault((query, number), threading.Event())
                # 【逐行批注】 判断条件 sum(counts.get((query, path), 0) >= number for path in ('/v1/too…，成立时执行对应分支。
                if sum(counts.get((query, path), 0) >= number for path in ('/v1/tools/search', '/v1/web-search')) == 2:
                    # 【逐行批注】 执行当前 Python 语句：(case_root / f'{number}.signal').touch()。
                    (case_root / f'{number}.signal').touch()
                    # 【逐行批注】 执行当前 Python 语句：event.set()。
                    event.set()
                # 【逐行批注】 计算右侧结果并赋给 gate，更新脚本状态。
                gate = releases.get((query, number))
                # 【逐行批注】 计算右侧结果并赋给 failure，更新脚本状态。
                failure = fixture.get('failureAt') == number
            # 【逐行批注】 判断条件 gate is not None and not gate.wait(10):，成立时执行对应分支。
            if gate is not None and not gate.wait(10):
                # 【逐行批注】 结束当前函数并返回结果：return。
                return
            # 【逐行批注】 计算右侧结果并赋给 label，更新脚本状态。
            label = f'version-{number}'
            # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
            data = json.dumps({'results': [{'title': label, 'url': 'https://example.org/' + label,
                                           # 【逐行批注】 执行当前 Python 语句：'summary': label}]}).encode()。
                                           'summary': label}]}).encode()
            # 【逐行批注】 执行当前 Python 语句：self.send_response(401 if failure else 200)。
            self.send_response(401 if failure else 200)
            # 【逐行批注】 调用 str、len 完成当前脚本或测试步骤：self.send_header('Content-Length', str(len(data)))。
            self.send_header('Content-Length', str(len(data)))
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 执行当前 Python 语句：self.wfile.write(data)。
                self.wfile.write(data)
                # 【逐行批注】 执行当前 Python 语句：self.wfile.flush()。
                self.wfile.flush()
                # 【逐行批注】 在受控上下文中执行资源操作：with lock:。
                with lock:
                    # 【逐行批注】 计算右侧结果并赋给 sent，更新脚本状态。
                    sent = delivered.setdefault((query, number), set())
                    # 【逐行批注】 执行当前 Python 语句：sent.add(self.path)。
                    sent.add(self.path)
                    # 【逐行批注】 判断条件 len(sent) == 2:，成立时执行对应分支。
                    if len(sent) == 2:
                        # 【逐行批注】 执行当前 Python 语句：(case_root / f'{number}.sent').touch()。
                        (case_root / f'{number}.sent').touch()
            # 【逐行批注】 捕获并处理异常：except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):。
            except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass

    # 【逐行批注】 定义类 FixtureServer，组织相关数据与行为。
    class FixtureServer(ThreadingHTTPServer):
        # 【逐行批注】 计算右侧结果并赋给 request_queue_size，更新脚本状态。
        request_queue_size = 64

    # 【逐行批注】 计算右侧结果并赋给 server，更新脚本状态。
    server = FixtureServer(('127.0.0.1', 0), Handler)
    # 【逐行批注】 计算右侧结果并赋给 threading.Thread(target，更新脚本状态。
    threading.Thread(target=server.serve_forever, daemon=True).start()

    # 【逐行批注】 定义函数 environment，封装可复用的处理步骤。
    def environment(cache):
        # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
        value = dict(os.environ, DEEPSEEK_SEARCH_API_KEY='fixture-ds', BOCHA_API_KEY='fixture-bocha',
                     # 【逐行批注】 计算右侧结果并赋给 DEEPSEEK_SEARCH_BASE_URL，更新脚本状态。
                     DEEPSEEK_SEARCH_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                     # 【逐行批注】 计算右侧结果并赋给 BOCHA_BASE_URL，更新脚本状态。
                     BOCHA_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                     # 【逐行批注】 计算右侧结果并赋给 RESOURCE_SEARCH_CACHE_DIR，更新脚本状态。
                     RESOURCE_SEARCH_CACHE_DIR=str(cache), RESOURCE_SEARCH_TIMEOUT_MS='15000',
                     # 【逐行批注】 计算右侧结果并赋给 NO_PROXY，更新脚本状态。
                     NO_PROXY='*', no_proxy='*', GANGYI_SEARCH_SINGLEFLIGHT='1')
        # 【逐行批注】 执行当前 Python 语句：value.pop('GANGYI_EVAL_SEARCH_BUDGET_FILE', None)。
        value.pop('GANGYI_EVAL_SEARCH_BUDGET_FILE', None)
        # 【逐行批注】 结束当前函数并返回结果：return value。
        return value

    # 【逐行批注】 定义函数 start，封装可复用的处理步骤。
    def start(value, env):
        # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
        process = subprocess.Popen([driver], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                   # 【逐行批注】 计算右侧结果并赋给 text，更新脚本状态。
                                   text=True, encoding='utf-8', env=env)
        # 【逐行批注】 执行当前 Python 语句：process.stdin.write(json.dumps(value) + '\n')。
        process.stdin.write(json.dumps(value) + '\n')
        # 【逐行批注】 执行当前 Python 语句：process.stdin.flush()。
        process.stdin.flush()
        # 【逐行批注】 结束当前函数并返回结果：return process。
        return process

    # 【逐行批注】 定义函数 finish，封装可复用的处理步骤。
    def finish(process, timeout=15):
        # 【逐行批注】 计算右侧结果并赋给 stdout, stderr，更新脚本状态。
        stdout, stderr = process.communicate(timeout=timeout)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert process.returncode == 0, stderr。
        assert process.returncode == 0, stderr
        # 【逐行批注】 计算右侧结果并赋给 rows，更新脚本状态。
        rows = [json.loads(line) for line in stdout.splitlines()]
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all('error' not in row for row in rows), rows。
        assert all('error' not in row for row in rows), rows
        # 【逐行批注】 结束当前函数并返回结果：return rows。
        return rows

    # 【逐行批注】 定义函数 wait_started，封装可复用的处理步骤。
    def wait_started(query, number):
        # 【逐行批注】 在受控上下文中执行资源操作：with lock:。
        with lock:
            # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
            event = events.setdefault((query, number), threading.Event())
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert event.wait(8), (query, number, received)。
        assert event.wait(8), (query, number, received)

    # 【逐行批注】 定义函数 wait_cached，封装可复用的处理步骤。
    def wait_cached(cache, number):
        # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
        deadline = time.monotonic() + 8
        # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
        while time.monotonic() < deadline:
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 计算右侧结果并赋给 rows，更新脚本状态。
                rows = [json.loads(path.read_text(encoding='utf-8')) for path in Path(cache).glob('*.json')]
                # 【逐行批注】 判断条件 len(rows) == 2 and all(row['resources'][0]['title'] == f'version…，成立时执行对应分支。
                if len(rows) == 2 and all(row['resources'][0]['title'] == f'version-{number}' for row in rows):
                    # 【逐行批注】 结束当前函数并返回结果：return。
                    return
            # 【逐行批注】 捕获并处理异常：except (OSError, ValueError, KeyError, IndexError):。
            except (OSError, ValueError, KeyError, IndexError):
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
            # 【逐行批注】 执行当前 Python 语句：time.sleep(.01)。
            time.sleep(.01)
        # 【逐行批注】 在错误情形抛出异常：raise AssertionError(('刷新结果未保存', number, cache))。
        raise AssertionError(('刷新结果未保存', number, cache))

    # 【逐行批注】 定义函数 read_cached，封装可复用的处理步骤。
    def read_cached(query, env, number):
        # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
        before = len(received)
        # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
        row = finish(start({'query': query}, env))[0]
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['cached'] and row['attempts'] == 0 and len(received) == before, row。
        assert row['cached'] and row['attempts'] == 0 and len(received) == before, row
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['resources'][0]['title'] == f'version-{number}', row。
        assert row['resources'][0]['title'] == f'version-{number}', row

    # 【逐行批注】 定义函数 wait_until，封装可复用的处理步骤。
    def wait_until(predicate, description, timeout=8):
        # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
        deadline = time.monotonic() + timeout
        # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
        while time.monotonic() < deadline:
            # 【逐行批注】 判断条件 predicate():，成立时执行对应分支。
            if predicate():
                # 【逐行批注】 结束当前函数并返回结果：return。
                return
            # 【逐行批注】 执行当前 Python 语句：time.sleep(.01)。
            time.sleep(.01)
        # 【逐行批注】 在错误情形抛出异常：raise AssertionError(description)。
        raise AssertionError(description)

    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='search-refresh-') as folder:。
        with tempfile.TemporaryDirectory(prefix='search-refresh-') as folder:
            # 【逐行批注】 计算右侧结果并赋给 root，更新脚本状态。
            root = Path(folder)
            # 【逐行批注】 遍历数据并重复执行循环体：for variant in ('success', 'failure', 'cancelled', 'two-refreshes', 'norma…。
            for variant in ('success', 'failure', 'cancelled', 'two-refreshes', 'normal-during-refresh', 'cross-process'):
                # 【逐行批注】 计算右侧结果并赋给 case，更新脚本状态。
                case = root / variant
                # 【逐行批注】 执行当前 Python 语句：case.mkdir()。
                case.mkdir()
                # 【逐行批注】 计算右侧结果并赋给 cache，更新脚本状态。
                cache = case / 'cache'
                # 【逐行批注】 执行当前 Python 语句：fixture.clear()。
                fixture.clear()
                # 【逐行批注】 计算右侧结果并赋给 fixture.update(root，更新脚本状态。
                fixture.update(root=case, failureAt=1 if variant == 'failure' else 0)
                # 【逐行批注】 计算右侧结果并赋给 query，更新脚本状态。
                query = 'refresh-' + variant
                # 【逐行批注】 计算右侧结果并赋给 count，更新脚本状态。
                count = 3 if variant in ('two-refreshes', 'normal-during-refresh') else 2
                # 【逐行批注】 遍历数据并重复执行循环体：for number in range(1, count + 1):。
                for number in range(1, count + 1):
                    # 【逐行批注】 计算右侧结果并赋给 releases[query, number]，更新脚本状态。
                    releases[query, number] = threading.Event()
                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                env = environment(cache)
                # 【逐行批注】 计算右侧结果并赋给 old，更新脚本状态。
                old = {'query': query}
                # 【逐行批注】 判断条件 variant == 'cancelled':，成立时执行对应分支。
                if variant == 'cancelled':
                    # 【逐行批注】 计算右侧结果并赋给 old['cancelOnFile']，更新脚本状态。
                    old['cancelOnFile'] = str(case / 'cancel.signal')
                # 【逐行批注】 计算右侧结果并赋给 newer，更新脚本状态。
                newer = {'query': query, 'refresh': True, 'startAfterFile': str(case / '1.signal')}
                # 【逐行批注】 判断条件 variant == 'cross-process':，成立时执行对应分支。
                if variant == 'cross-process':
                    # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
                    process = start(old, env)
                    # 【逐行批注】 调用 wait_started 完成当前脚本或测试步骤：wait_started(query, 1)。
                    wait_started(query, 1)
                    # 【逐行批注】 计算右侧结果并赋给 refresh，更新脚本状态。
                    refresh = start({'query': query, 'refresh': True}, env)
                # 【逐行批注】 执行前一条件不成立时的分支。
                else:
                    # 【逐行批注】 计算右侧结果并赋给 inputs，更新脚本状态。
                    inputs = [old, newer]
                    # 【逐行批注】 判断条件 count == 3:，成立时执行对应分支。
                    if count == 3:
                        # 【逐行批注】 执行当前 Python 语句：inputs.append({'query': query, 'refresh': variant == 'two-refreshes',。
                        inputs.append({'query': query, 'refresh': variant == 'two-refreshes',
                                       # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'startAfterFile': str(case / '2.signal')})。
                                       'startAfterFile': str(case / '2.signal')})
                    # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
                    process = start({'parallel': inputs}, env)
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 调用 wait_started 完成当前脚本或测试步骤：wait_started(query, count)。
                    wait_started(query, count)
                    # 【逐行批注】 计算右侧结果并赋给 winner，更新脚本状态。
                    winner = 3 if variant == 'two-refreshes' else 2
                    # 【逐行批注】 执行当前 Python 语句：releases[query, winner].set()。
                    releases[query, winner].set()
                    # 【逐行批注】 调用 wait_cached 完成当前脚本或测试步骤：wait_cached(cache, winner)。
                    wait_cached(cache, winner)
                    # 【逐行批注】 判断条件 variant == 'cancelled':，成立时执行对应分支。
                    if variant == 'cancelled':
                        # 【逐行批注】 执行当前 Python 语句：(case / 'cancel.signal').touch()。
                        (case / 'cancel.signal').touch()
                        # 【逐行批注】 执行当前 Python 语句：time.sleep(.25)。
                        time.sleep(.25)
                    # 【逐行批注】 遍历数据并重复执行循环体：for number in range(1, count + 1):。
                    for number in range(1, count + 1):
                        # 【逐行批注】 执行当前 Python 语句：releases[query, number].set()。
                        releases[query, number].set()
                    # 【逐行批注】 计算右侧结果并赋给 rows，更新脚本状态。
                    rows = finish(process)
                    # 【逐行批注】 判断条件 variant == 'cross-process':，成立时执行对应分支。
                    if variant == 'cross-process':
                        # 【逐行批注】 计算右侧结果并赋给 refresh_rows，更新脚本状态。
                        refresh_rows = finish(refresh)
                        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert rows[0]['attempts'] + refresh_rows[0]['attempts'] == 4。
                        assert rows[0]['attempts'] + refresh_rows[0]['attempts'] == 4
                    # 【逐行批注】 执行前一条件不成立时的分支。
                    else:
                        # 【逐行批注】 计算右侧结果并赋给 parts，更新脚本状态。
                        parts = rows[0]['parallel']
                        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert sum(row['attempts'] for row in parts) == count * 2, parts。
                        assert sum(row['attempts'] for row in parts) == count * 2, parts
                        # 【逐行批注】 计算右侧结果并赋给 expected，更新脚本状态。
                        expected = 'failed' if variant == 'failure' else 'cancelled' if variant == 'cancelled' else 'completed'
                        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert parts[0]['status'] == expected, parts。
                        assert parts[0]['status'] == expected, parts
                        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert parts[1]['resources'][0]['title'] == 'version-2', parts。
                        assert parts[1]['resources'][0]['title'] == 'version-2', parts
                    # 【逐行批注】 调用 read_cached 完成当前脚本或测试步骤：read_cached(query, env, winner)。
                    read_cached(query, env, winner)
                # 【逐行批注】 执行当前 Python 语句：finally:。
                finally:
                    # 【逐行批注】 遍历数据并重复执行循环体：for number in range(1, count + 1):。
                    for number in range(1, count + 1):
                        # 【逐行批注】 执行当前 Python 语句：releases[query, number].set()。
                        releases[query, number].set()
                    # 【逐行批注】 遍历数据并重复执行循环体：for child in (process, refresh) if variant == 'cross-process' else (proces…。
                    for child in (process, refresh) if variant == 'cross-process' else (process,):
                        # 【逐行批注】 判断条件 child.poll() is None:，成立时执行对应分支。
                        if child.poll() is None:
                            # 【逐行批注】 执行当前 Python 语句：child.kill()。
                            child.kill()
                            # 【逐行批注】 计算右侧结果并赋给 child.communicate(timeout，更新脚本状态。
                            child.communicate(timeout=5)

            # 无磁盘缓存的刷新仍联网；配置了缓存但世代无法提交时不得发送。
            # 【逐行批注】 计算右侧结果并赋给 case，更新脚本状态。
            case = root / 'cache-availability'
            # 【逐行批注】 执行当前 Python 语句：case.mkdir()。
            case.mkdir()
            # 【逐行批注】 执行当前 Python 语句：fixture.clear()。
            fixture.clear()
            # 【逐行批注】 计算右侧结果并赋给 fixture['root']，更新脚本状态。
            fixture['root'] = case
            # 【逐行批注】 计算右侧结果并赋给 disabled，更新脚本状态。
            disabled = environment('')
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(received)
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = finish(start({'query': 'disabled-cache', 'refresh': True}, disabled))[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'completed' and not row['cached'] and len(received) - b…。
            assert row['status'] == 'completed' and not row['cached'] and len(received) - before == 2, row
            # 【逐行批注】 计算右侧结果并赋给 blocked，更新脚本状态。
            blocked = case / 'not-a-directory'
            # 【逐行批注】 计算右侧结果并赋给 blocked.write_text('占用缓存目录路径', encoding，更新脚本状态。
            blocked.write_text('占用缓存目录路径', encoding='utf-8')
            # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
            env = environment(blocked)
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(received)
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = finish(start({'query': 'unavailable-cache', 'refresh': True}, env))[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'failed' and row['attempts'] == 0 and len(received) == …。
            assert row['status'] == 'failed' and row['attempts'] == 0 and len(received) == before, row
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(part['status'] == 'cache_unavailable' for part in row['providers']),…。
            assert all(part['status'] == 'cache_unavailable' for part in row['providers']), row
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = finish(start({'query': 'unavailable-cache'}, env))[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'completed' and not row['cached'] and row['attempts'] =…。
            assert row['status'] == 'completed' and not row['cached'] and row['attempts'] == 2, row
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert blocked.read_text(encoding='utf-8') == '占用缓存目录路径'。
            assert blocked.read_text(encoding='utf-8') == '占用缓存目录路径'

            # 【逐行批注】 计算右侧结果并赋给 cache，更新脚本状态。
            cache = case / 'metadata-blocked'
            # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
            env = environment(cache)
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = finish(start({'query': 'metadata-blocked'}, env))[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'completed', row。
            assert row['status'] == 'completed', row
            # 【逐行批注】 遍历数据并重复执行循环体：for state in cache.glob('*.state'):。
            for state in cache.glob('*.state'):
                # 【逐行批注】 调用 Path、str 完成当前脚本或测试步骤：Path(str(state) + '.tmp').mkdir()。
                Path(str(state) + '.tmp').mkdir()
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(received)
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = finish(start({'query': 'metadata-blocked', 'refresh': True}, env))[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['attempts'] == 0 and len(received) == before, row。
            assert row['attempts'] == 0 and len(received) == before, row
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(part['status'] == 'cache_unavailable' for part in row['providers']),…。
            assert all(part['status'] == 'cache_unavailable' for part in row['providers']), row
            # 【逐行批注】 调用 read_cached 完成当前脚本或测试步骤：read_cached('metadata-blocked', env, 1)。
            read_cached('metadata-blocked', env, 1)

            # 新协议不读取旧文件，旧进程随后写回旧文件也不能覆盖新缓存。
            # 【逐行批注】 计算右侧结果并赋给 cache，更新脚本状态。
            cache = case / 'old-protocol'
            # 【逐行批注】 执行当前 Python 语句：cache.mkdir()。
            cache.mkdir()
            # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
            env = environment(cache)
            # 【逐行批注】 计算右侧结果并赋给 query，更新脚本状态。
            query = 'old-protocol-isolation'
            # 【逐行批注】 计算右侧结果并赋给 old_files，更新脚本状态。
            old_files = []
            # 【逐行批注】 遍历数据并重复执行循环体：for provider, credential in (('deepseek', 'fixture-ds'), ('bocha', 'fixtur…。
            for provider, credential in (('deepseek', 'fixture-ds'), ('bocha', 'fixture-bocha')):
                # 【逐行批注】 计算右侧结果并赋给 identity，更新脚本状态。
                identity = ('dual-v1:' + provider + ':' + f'http://127.0.0.1:{server.server_port}' + ':' +
                            # 【逐行批注】 执行当前 Python 语句：hashlib.sha256(credential.encode()).hexdigest() + ':' + query + ':noLimit:10')。
                            hashlib.sha256(credential.encode()).hexdigest() + ':' + query + ':noLimit:10')
                # 【逐行批注】 计算右侧结果并赋给 old，更新脚本状态。
                old = cache / (hashlib.sha256(identity.encode()).hexdigest() + '.json')
                # 【逐行批注】 执行当前 Python 语句：old_files.append(old)。
                old_files.append(old)
                # 【逐行批注】 调用 int 完成当前脚本或测试步骤：old.write_text(json.dumps({'createdAt': int(time.time()), 'resources'…。
                old.write_text(json.dumps({'createdAt': int(time.time()), 'resources': [
                    # 【逐行批注】 计算右侧结果并赋给 {'title': 'old-v1-cache', 'url': 'https://ex…，更新脚本状态。
                    {'title': 'old-v1-cache', 'url': 'https://example.org/old-v1-cache'}]}), encoding='utf-8')
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(received)
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = finish(start({'query': query}, env))[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not row['cached'] and row['attempts'] == 2 and len(received) - before ==…。
            assert not row['cached'] and row['attempts'] == 2 and len(received) - before == 2, row
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['resources'][0]['title'] == 'version-1', row。
            assert row['resources'][0]['title'] == 'version-1', row
            # 【逐行批注】 遍历数据并重复执行循环体：for old in old_files:。
            for old in old_files:
                # 【逐行批注】 调用 int 完成当前脚本或测试步骤：old.write_text(json.dumps({'createdAt': int(time.time()), 'resources'…。
                old.write_text(json.dumps({'createdAt': int(time.time()), 'resources': [
                    # 【逐行批注】 计算右侧结果并赋给 {'title': 'late-v1-write', 'url': 'https://e…，更新脚本状态。
                    {'title': 'late-v1-write', 'url': 'https://example.org/late-v1-write'}]}), encoding='utf-8')
            # 【逐行批注】 调用 read_cached 完成当前脚本或测试步骤：read_cached(query, env, 1)。
            read_cached(query, env, 1)

            # 网络完成后由另一进程锁住提交；等待期间取消不能生成成功缓存。
            # 【逐行批注】 遍历数据并重复执行循环体：for refresh in (False, True):。
            for refresh in (False, True):
                # 【逐行批注】 计算右侧结果并赋给 case，更新脚本状态。
                case = root / ('commit-cancel-refresh' if refresh else 'commit-cancel-shared')
                # 【逐行批注】 执行当前 Python 语句：case.mkdir()。
                case.mkdir()
                # 【逐行批注】 计算右侧结果并赋给 cache，更新脚本状态。
                cache = case / 'cache'
                # 【逐行批注】 执行当前 Python 语句：fixture.clear()。
                fixture.clear()
                # 【逐行批注】 计算右侧结果并赋给 fixture['root']，更新脚本状态。
                fixture['root'] = case
                # 【逐行批注】 计算右侧结果并赋给 query，更新脚本状态。
                query = case.name
                # 【逐行批注】 计算右侧结果并赋给 releases[query, 1]，更新脚本状态。
                releases[query, 1] = threading.Event()
                # 【逐行批注】 计算右侧结果并赋给 signal, observed, checks，更新脚本状态。
                signal, observed, checks = case / 'cancel.signal', case / 'cancel.observed', case / 'cancel.checks'
                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                env = environment(cache)
                # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
                process = start({'query': query, 'refresh': refresh, 'cancelOnFile': str(signal),
                                 # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'cancelObservedFile': str(observed), 'cancelCheckAfterFile': str(case…。
                                 'cancelObservedFile': str(observed), 'cancelCheckAfterFile': str(case / '1.sent'),
                                 # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'cancelChecksFile': str(checks)}, env)。
                                 'cancelChecksFile': str(checks)}, env)
                # 【逐行批注】 计算右侧结果并赋给 holder，更新脚本状态。
                holder = None
                # 【逐行批注】 计算右侧结果并赋给 unlock，更新脚本状态。
                unlock = case / 'unlock.signal'
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 调用 wait_started 完成当前脚本或测试步骤：wait_started(query, 1)。
                    wait_started(query, 1)
                    # 【逐行批注】 计算右侧结果并赋给 ready，更新脚本状态。
                    ready = case / 'locks.ready'
                    # 【逐行批注】 计算右侧结果并赋给 holder，更新脚本状态。
                    holder = subprocess.Popen([sys.executable, str(Path(__file__).resolve()), '--hold-cache-locks',
                                               # 【逐行批注】 调用 str 完成当前脚本或测试步骤：str(cache), str(ready), str(unlock)])。
                                               str(cache), str(ready), str(unlock)])
                    # 【逐行批注】 调用 wait_until 完成当前脚本或测试步骤：wait_until(ready.exists, '独立进程未取得缓存锁')。
                    wait_until(ready.exists, '独立进程未取得缓存锁')
                    # 【逐行批注】 执行当前 Python 语句：releases[query, 1].set()。
                    releases[query, 1].set()
                    # 【逐行批注】 调用 wait_until 完成当前脚本或测试步骤：wait_until((case / '1.sent').exists, 'HTTP响应未完成')。
                    wait_until((case / '1.sent').exists, 'HTTP响应未完成')
                    # 刷新没有共享等待者轮询；持续读取取消状态证明提交锁等待可中断。
                    # 【逐行批注】 调用 wait_until 完成当前脚本或测试步骤：wait_until(lambda: checks.exists() and checks.read_bytes().count(b'\n…。
                    wait_until(lambda: checks.exists() and checks.read_bytes().count(b'\n') >= 32,
                               # 【逐行批注】 执行当前 Python 语句：'HTTP完成后等待提交锁时未持续轮询取消')。
                               'HTTP完成后等待提交锁时未持续轮询取消')
                    # 【逐行批注】 执行当前 Python 语句：signal.touch()。
                    signal.touch()
                    # 【逐行批注】 计算右侧结果并赋给 wait_until(observed.exists, '提交锁等待期间未观察到取消',…，更新脚本状态。
                    wait_until(observed.exists, '提交锁等待期间未观察到取消', timeout=.5)
                    # 【逐行批注】 计算右侧结果并赋给 rows，更新脚本状态。
                    rows = finish(process, timeout=.5)
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert rows[0]['status'] == 'cancelled', rows。
                    assert rows[0]['status'] == 'cancelled', rows
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not list(cache.glob('*.json')), list(cache.iterdir())。
                    assert not list(cache.glob('*.json')), list(cache.iterdir())
                    # 【逐行批注】 执行当前 Python 语句：unlock.touch()。
                    unlock.touch()
                    # 【逐行批注】 计算右侧结果并赋给 holder.wait(timeout，更新脚本状态。
                    holder.wait(timeout=5)
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert holder.returncode == 0。
                    assert holder.returncode == 0
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not list(cache.glob('*.json')), list(cache.iterdir())。
                    assert not list(cache.glob('*.json')), list(cache.iterdir())
                    # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
                    row = finish(start({'query': query}, env))[0]
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'completed' and row['attempts'] == 2 and not row['cache…。
                    assert row['status'] == 'completed' and row['attempts'] == 2 and not row['cached'], row
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['resources'][0]['title'] == 'version-2', row。
                    assert row['resources'][0]['title'] == 'version-2', row
                # 【逐行批注】 执行当前 Python 语句：finally:。
                finally:
                    # 【逐行批注】 执行当前 Python 语句：releases[query, 1].set()。
                    releases[query, 1].set()
                    # 【逐行批注】 执行当前 Python 语句：unlock.touch()。
                    unlock.touch()
                    # 【逐行批注】 判断条件 holder is not None and holder.poll() is None:，成立时执行对应分支。
                    if holder is not None and holder.poll() is None:
                        # 【逐行批注】 计算右侧结果并赋给 holder.wait(timeout，更新脚本状态。
                        holder.wait(timeout=5)
                    # 【逐行批注】 判断条件 process.poll() is None:，成立时执行对应分支。
                    if process.poll() is None:
                        # 【逐行批注】 执行当前 Python 语句：process.kill()。
                        process.kill()
                        # 【逐行批注】 计算右侧结果并赋给 process.communicate(timeout，更新脚本状态。
                        process.communicate(timeout=5)

            # 较新的刷新失败或取消，不删除此前已成功保存的资料。
            # 【逐行批注】 计算右侧结果并赋给 case，更新脚本状态。
            case = root / 'cache-availability'
            # 【逐行批注】 计算右侧结果并赋给 fixture['root']，更新脚本状态。
            fixture['root'] = case
            # 【逐行批注】 计算右侧结果并赋给 cache，更新脚本状态。
            cache = case / 'old-protocol'
            # 【逐行批注】 计算右侧结果并赋给 query，更新脚本状态。
            query = 'old-protocol-isolation'
            # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
            env = environment(cache)
            # 【逐行批注】 计算右侧结果并赋给 fixture['failureAt']，更新脚本状态。
            fixture['failureAt'] = 2
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = finish(start({'query': query, 'refresh': True}, env))[0]
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'failed' and row['attempts'] == 2, row。
            assert row['status'] == 'failed' and row['attempts'] == 2, row
            # 【逐行批注】 调用 read_cached 完成当前脚本或测试步骤：read_cached(query, env, 1)。
            read_cached(query, env, 1)
            # 【逐行批注】 计算右侧结果并赋给 fixture['failureAt']，更新脚本状态。
            fixture['failureAt'] = 0
            # 【逐行批注】 计算右侧结果并赋给 releases[query, 3]，更新脚本状态。
            releases[query, 3] = threading.Event()
            # 【逐行批注】 计算右侧结果并赋给 signal，更新脚本状态。
            signal = case / 'refresh-cancel.signal'
            # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
            process = start({'query': query, 'refresh': True, 'cancelOnFile': str(signal)}, env)
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 调用 wait_started 完成当前脚本或测试步骤：wait_started(query, 3)。
                wait_started(query, 3)
                # 【逐行批注】 执行当前 Python 语句：signal.touch()。
                signal.touch()
                # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
                row = finish(process)[0]
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert row['status'] == 'cancelled', row。
                assert row['status'] == 'cancelled', row
                # 【逐行批注】 调用 read_cached 完成当前脚本或测试步骤：read_cached(query, env, 1)。
                read_cached(query, env, 1)
            # 【逐行批注】 执行当前 Python 语句：finally:。
            finally:
                # 【逐行批注】 执行当前 Python 语句：releases[query, 3].set()。
                releases[query, 3].set()
                # 【逐行批注】 判断条件 process.poll() is None:，成立时执行对应分支。
                if process.poll() is None:
                    # 【逐行批注】 执行当前 Python 语句：process.kill()。
                    process.kill()
                    # 【逐行批注】 计算右侧结果并赋给 process.communicate(timeout，更新脚本状态。
                    process.communicate(timeout=5)
    # 【逐行批注】 执行当前 Python 语句：finally:。
    finally:
        # 【逐行批注】 遍历数据并重复执行循环体：for gate in releases.values():。
        for gate in releases.values():
            # 【逐行批注】 执行当前 Python 语句：gate.set()。
            gate.set()
        # 【逐行批注】 执行当前 Python 语句：server.shutdown()。
        server.shutdown()
        # 【逐行批注】 执行当前 Python 语句：server.server_close()。
        server.server_close()


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 sys.argv[1] == '--hold-cache-locks':，成立时执行对应分支。
    if sys.argv[1] == '--hold-cache-locks':
        # 【逐行批注】 调用 hold_cache_locks 完成当前脚本或测试步骤：hold_cache_locks(*sys.argv[2:])。
        hold_cache_locks(*sys.argv[2:])
    # 【逐行批注】 执行前一条件不成立时的分支。
    else:
        # 【逐行批注】 调用 run_refresh_checks 完成当前脚本或测试步骤：run_refresh_checks(sys.argv[1])。
        run_refresh_checks(sys.argv[1])
        # 【逐行批注】 输出当前运行结果：print('搜索刷新先后顺序、普通请求交错、失败、提交前取消、跨进程与缓存不可用边界通过；外网请求0。')。
        print('搜索刷新先后顺序、普通请求交错、失败、提交前取消、跨进程与缓存不可用边界通过；外网请求0。')

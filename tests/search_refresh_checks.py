"""用本机信号精确控制搜索完成顺序，核对刷新世代和跨进程缓存提交。"""
import json
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer


def hold_cache_locks(directory, ready, release):
    """独立进程持同一系统字节锁，主测试解锁前缓存提交必须能够取消。"""
    handles = []
    try:
        files = sorted(Path(directory).glob('*.lock'))
        assert len(files) == 2, files
        for path in files:
            stream = path.open('a+b')
            handles.append(stream)
            if os.name == 'nt':
                import msvcrt
                stream.write(b'\0')
                stream.flush()
                stream.seek(0)
                msvcrt.locking(stream.fileno(), msvcrt.LK_LOCK, 1)
            else:
                import fcntl
                fcntl.flock(stream.fileno(), fcntl.LOCK_EX)
        Path(ready).touch()
        deadline = time.monotonic() + 15
        while not Path(release).exists():
            if time.monotonic() >= deadline:
                raise AssertionError('缓存锁夹具等待解锁超时')
            time.sleep(.01)
    finally:
        for stream in handles:
            if os.name == 'nt':
                stream.seek(0)
                msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
            else:
                fcntl.flock(stream.fileno(), fcntl.LOCK_UN)
            stream.close()


def run_refresh_checks(driver):
    driver = str(Path(driver).resolve())
    received, counts, events, releases, delivered = [], {}, {}, {}, {}
    lock = threading.Lock()
    fixture = {}

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_args):
            pass

        def do_POST(self):
            body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
            query = body['query']
            with lock:
                case_root = fixture['root']
                number = counts.get((query, self.path), 0) + 1
                counts[query, self.path] = number
                received.append((query, self.path, number))
                event = events.setdefault((query, number), threading.Event())
                if sum(counts.get((query, path), 0) >= number for path in ('/v1/tools/search', '/v1/web-search')) == 2:
                    (case_root / f'{number}.signal').touch()
                    event.set()
                gate = releases.get((query, number))
                failure = fixture.get('failureAt') == number
            if gate is not None and not gate.wait(10):
                return
            label = f'version-{number}'
            data = json.dumps({'results': [{'title': label, 'url': 'https://example.org/' + label,
                                           'summary': label}]}).encode()
            self.send_response(401 if failure else 200)
            self.send_header('Content-Length', str(len(data)))
            self.end_headers()
            try:
                self.wfile.write(data)
                self.wfile.flush()
                with lock:
                    sent = delivered.setdefault((query, number), set())
                    sent.add(self.path)
                    if len(sent) == 2:
                        (case_root / f'{number}.sent').touch()
            except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
                pass

    class FixtureServer(ThreadingHTTPServer):
        request_queue_size = 64

    server = FixtureServer(('127.0.0.1', 0), Handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()

    def environment(cache):
        value = dict(os.environ, DEEPSEEK_SEARCH_API_KEY='fixture-ds', BOCHA_API_KEY='fixture-bocha',
                     DEEPSEEK_SEARCH_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                     BOCHA_BASE_URL=f'http://127.0.0.1:{server.server_port}',
                     RESOURCE_SEARCH_CACHE_DIR=str(cache), RESOURCE_SEARCH_TIMEOUT_MS='15000',
                     NO_PROXY='*', no_proxy='*', GANGYI_SEARCH_SINGLEFLIGHT='1')
        value.pop('GANGYI_EVAL_SEARCH_BUDGET_FILE', None)
        return value

    def start(value, env):
        process = subprocess.Popen([driver], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                   text=True, encoding='utf-8', env=env)
        process.stdin.write(json.dumps(value) + '\n')
        process.stdin.flush()
        return process

    def finish(process, timeout=15):
        stdout, stderr = process.communicate(timeout=timeout)
        assert process.returncode == 0, stderr
        rows = [json.loads(line) for line in stdout.splitlines()]
        assert all('error' not in row for row in rows), rows
        return rows

    def wait_started(query, number):
        with lock:
            event = events.setdefault((query, number), threading.Event())
        assert event.wait(8), (query, number, received)

    def wait_cached(cache, number):
        deadline = time.monotonic() + 8
        while time.monotonic() < deadline:
            try:
                rows = [json.loads(path.read_text(encoding='utf-8')) for path in Path(cache).glob('*.json')]
                if len(rows) == 2 and all(row['resources'][0]['title'] == f'version-{number}' for row in rows):
                    return
            except (OSError, ValueError, KeyError, IndexError):
                pass
            time.sleep(.01)
        raise AssertionError(('刷新结果未保存', number, cache))

    def read_cached(query, env, number):
        before = len(received)
        row = finish(start({'query': query}, env))[0]
        assert row['cached'] and row['attempts'] == 0 and len(received) == before, row
        assert row['resources'][0]['title'] == f'version-{number}', row

    def wait_until(predicate, description, timeout=8):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            if predicate():
                return
            time.sleep(.01)
        raise AssertionError(description)

    try:
        with tempfile.TemporaryDirectory(prefix='search-refresh-') as folder:
            root = Path(folder)
            for variant in ('success', 'failure', 'cancelled', 'two-refreshes', 'normal-during-refresh', 'cross-process'):
                case = root / variant
                case.mkdir()
                cache = case / 'cache'
                fixture.clear()
                fixture.update(root=case, failureAt=1 if variant == 'failure' else 0)
                query = 'refresh-' + variant
                count = 3 if variant in ('two-refreshes', 'normal-during-refresh') else 2
                for number in range(1, count + 1):
                    releases[query, number] = threading.Event()
                env = environment(cache)
                old = {'query': query}
                if variant == 'cancelled':
                    old['cancelOnFile'] = str(case / 'cancel.signal')
                newer = {'query': query, 'refresh': True, 'startAfterFile': str(case / '1.signal')}
                if variant == 'cross-process':
                    process = start(old, env)
                    wait_started(query, 1)
                    refresh = start({'query': query, 'refresh': True}, env)
                else:
                    inputs = [old, newer]
                    if count == 3:
                        inputs.append({'query': query, 'refresh': variant == 'two-refreshes',
                                       'startAfterFile': str(case / '2.signal')})
                    process = start({'parallel': inputs}, env)
                try:
                    wait_started(query, count)
                    winner = 3 if variant == 'two-refreshes' else 2
                    releases[query, winner].set()
                    wait_cached(cache, winner)
                    if variant == 'cancelled':
                        (case / 'cancel.signal').touch()
                        time.sleep(.25)
                    for number in range(1, count + 1):
                        releases[query, number].set()
                    rows = finish(process)
                    if variant == 'cross-process':
                        refresh_rows = finish(refresh)
                        assert rows[0]['attempts'] + refresh_rows[0]['attempts'] == 4
                    else:
                        parts = rows[0]['parallel']
                        assert sum(row['attempts'] for row in parts) == count * 2, parts
                        expected = 'failed' if variant == 'failure' else 'cancelled' if variant == 'cancelled' else 'completed'
                        assert parts[0]['status'] == expected, parts
                        assert parts[1]['resources'][0]['title'] == 'version-2', parts
                    read_cached(query, env, winner)
                finally:
                    for number in range(1, count + 1):
                        releases[query, number].set()
                    for child in (process, refresh) if variant == 'cross-process' else (process,):
                        if child.poll() is None:
                            child.kill()
                            child.communicate(timeout=5)

            # 无磁盘缓存的刷新仍联网；配置了缓存但世代无法提交时不得发送。
            case = root / 'cache-availability'
            case.mkdir()
            fixture.clear()
            fixture['root'] = case
            disabled = environment('')
            before = len(received)
            row = finish(start({'query': 'disabled-cache', 'refresh': True}, disabled))[0]
            assert row['status'] == 'completed' and not row['cached'] and len(received) - before == 2, row
            blocked = case / 'not-a-directory'
            blocked.write_text('占用缓存目录路径', encoding='utf-8')
            env = environment(blocked)
            before = len(received)
            row = finish(start({'query': 'unavailable-cache', 'refresh': True}, env))[0]
            assert row['status'] == 'failed' and row['attempts'] == 0 and len(received) == before, row
            assert all(part['status'] == 'cache_unavailable' for part in row['providers']), row
            row = finish(start({'query': 'unavailable-cache'}, env))[0]
            assert row['status'] == 'completed' and not row['cached'] and row['attempts'] == 2, row
            assert blocked.read_text(encoding='utf-8') == '占用缓存目录路径'

            cache = case / 'metadata-blocked'
            env = environment(cache)
            row = finish(start({'query': 'metadata-blocked'}, env))[0]
            assert row['status'] == 'completed', row
            for state in cache.glob('*.state'):
                Path(str(state) + '.tmp').mkdir()
            before = len(received)
            row = finish(start({'query': 'metadata-blocked', 'refresh': True}, env))[0]
            assert row['attempts'] == 0 and len(received) == before, row
            assert all(part['status'] == 'cache_unavailable' for part in row['providers']), row
            read_cached('metadata-blocked', env, 1)

            # 新协议不读取旧文件，旧进程随后写回旧文件也不能覆盖新缓存。
            cache = case / 'old-protocol'
            cache.mkdir()
            env = environment(cache)
            query = 'old-protocol-isolation'
            old_files = []
            for provider, credential in (('deepseek', 'fixture-ds'), ('bocha', 'fixture-bocha')):
                identity = ('dual-v1:' + provider + ':' + f'http://127.0.0.1:{server.server_port}' + ':' +
                            hashlib.sha256(credential.encode()).hexdigest() + ':' + query + ':noLimit:10')
                old = cache / (hashlib.sha256(identity.encode()).hexdigest() + '.json')
                old_files.append(old)
                old.write_text(json.dumps({'createdAt': int(time.time()), 'resources': [
                    {'title': 'old-v1-cache', 'url': 'https://example.org/old-v1-cache'}]}), encoding='utf-8')
            before = len(received)
            row = finish(start({'query': query}, env))[0]
            assert not row['cached'] and row['attempts'] == 2 and len(received) - before == 2, row
            assert row['resources'][0]['title'] == 'version-1', row
            for old in old_files:
                old.write_text(json.dumps({'createdAt': int(time.time()), 'resources': [
                    {'title': 'late-v1-write', 'url': 'https://example.org/late-v1-write'}]}), encoding='utf-8')
            read_cached(query, env, 1)

            # 网络完成后由另一进程锁住提交；等待期间取消不能生成成功缓存。
            for refresh in (False, True):
                case = root / ('commit-cancel-refresh' if refresh else 'commit-cancel-shared')
                case.mkdir()
                cache = case / 'cache'
                fixture.clear()
                fixture['root'] = case
                query = case.name
                releases[query, 1] = threading.Event()
                signal, observed, checks = case / 'cancel.signal', case / 'cancel.observed', case / 'cancel.checks'
                env = environment(cache)
                process = start({'query': query, 'refresh': refresh, 'cancelOnFile': str(signal),
                                 'cancelObservedFile': str(observed), 'cancelCheckAfterFile': str(case / '1.sent'),
                                 'cancelChecksFile': str(checks)}, env)
                holder = None
                unlock = case / 'unlock.signal'
                try:
                    wait_started(query, 1)
                    ready = case / 'locks.ready'
                    holder = subprocess.Popen([sys.executable, str(Path(__file__).resolve()), '--hold-cache-locks',
                                               str(cache), str(ready), str(unlock)])
                    wait_until(ready.exists, '独立进程未取得缓存锁')
                    releases[query, 1].set()
                    wait_until((case / '1.sent').exists, 'HTTP响应未完成')
                    # 刷新没有共享等待者轮询；持续读取取消状态证明提交锁等待可中断。
                    wait_until(lambda: checks.exists() and checks.read_bytes().count(b'\n') >= 32,
                               'HTTP完成后等待提交锁时未持续轮询取消')
                    signal.touch()
                    wait_until(observed.exists, '提交锁等待期间未观察到取消', timeout=.5)
                    rows = finish(process, timeout=.5)
                    assert rows[0]['status'] == 'cancelled', rows
                    assert not list(cache.glob('*.json')), list(cache.iterdir())
                    unlock.touch()
                    holder.wait(timeout=5)
                    assert holder.returncode == 0
                    assert not list(cache.glob('*.json')), list(cache.iterdir())
                    row = finish(start({'query': query}, env))[0]
                    assert row['status'] == 'completed' and row['attempts'] == 2 and not row['cached'], row
                    assert row['resources'][0]['title'] == 'version-2', row
                finally:
                    releases[query, 1].set()
                    unlock.touch()
                    if holder is not None and holder.poll() is None:
                        holder.wait(timeout=5)
                    if process.poll() is None:
                        process.kill()
                        process.communicate(timeout=5)

            # 较新的刷新失败或取消，不删除此前已成功保存的资料。
            case = root / 'cache-availability'
            fixture['root'] = case
            cache = case / 'old-protocol'
            query = 'old-protocol-isolation'
            env = environment(cache)
            fixture['failureAt'] = 2
            row = finish(start({'query': query, 'refresh': True}, env))[0]
            assert row['status'] == 'failed' and row['attempts'] == 2, row
            read_cached(query, env, 1)
            fixture['failureAt'] = 0
            releases[query, 3] = threading.Event()
            signal = case / 'refresh-cancel.signal'
            process = start({'query': query, 'refresh': True, 'cancelOnFile': str(signal)}, env)
            try:
                wait_started(query, 3)
                signal.touch()
                row = finish(process)[0]
                assert row['status'] == 'cancelled', row
                read_cached(query, env, 1)
            finally:
                releases[query, 3].set()
                if process.poll() is None:
                    process.kill()
                    process.communicate(timeout=5)
    finally:
        for gate in releases.values():
            gate.set()
        server.shutdown()
        server.server_close()


if __name__ == '__main__':
    if sys.argv[1] == '--hold-cache-locks':
        hold_cache_locks(*sys.argv[2:])
    else:
        run_refresh_checks(sys.argv[1])
        print('搜索刷新先后顺序、普通请求交错、失败、提交前取消、跨进程与缓存不可用边界通过；外网请求0。')

"""跨进程调用监控验收：只连接本地夹具，不读取真实凭据或真实用户档案。"""
import json
import os
import select
import subprocess
import sys
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Event, Thread
from urllib.request import Request, urlopen
from classroom_v560_fixture import start_session


class ConnectionProbe(BaseHTTPRequestHandler):
    started = Event()
    disconnected = Event()
    count = 0

    def log_message(self, *_):
        pass

    def do_POST(self):
        self.rfile.read(int(self.headers['Content-Length']))
        type(self).count += 1
        if type(self).count == 1:
            type(self).started.set()
            readable, _, _ = select.select([self.connection], [], [], 3)
            if readable and self.connection.recv(1) == b'':
                type(self).disconnected.set()
                return
        data = json.dumps(dict(model='fixture', choices=[dict(message=dict(content='OK'))])).encode()
        try:
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(data)))
            self.end_headers()
            self.wfile.write(data)
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            pass


def eventually(check, seconds=10):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        result = check()
        if result:
            return result
        time.sleep(.05)
    raise AssertionError('状态未在限定时间内到达')


def main(executable, probe_executable):
    with start_session(executable) as (info, api, process):
        provider = ThreadingHTTPServer(('127.0.0.1', 0), ConnectionProbe)
        Thread(target=provider.serve_forever, daemon=True).start()
        record = str(Path(info['database']).parent / 'ai-activity.json')
        child = subprocess.Popen([probe_executable, '--monitor-probe', record, f'http://127.0.0.1:{provider.server_port}/v1'],
                                 stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                                 creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        try:
            assert ConnectionProbe.started.wait(5), '独立进程没有发出连接测试请求'
            with urlopen(Request(info['fixture']+'/fixture-control', data=b'{"slow":true}', headers={'Content-Type':'application/json'})):
                pass
            task = api('/api/learning-agent/events', dict(type='acceptance_tool', requestId='monitor-first', courseId=info['courseId'], tool='read_context', args={}))
            def both_requesting():
                view = api('/api/ai-activity')
                active = [item for item in view['tasks'] if item['status'] in ('requesting', 'receiving')]
                return view if any(item.get('taskId') == task['id'] for item in active) and any(item['source'] == 'AI 设置' for item in active) else None
            running = eventually(both_requesting, 5)
            assert 'PRIVATE-' not in json.dumps(running), '监控不能包含标准答案、工具正文或模型提示词'
            paused_at = time.monotonic()
            paused = api('/api/ai-activity/control', dict(command='pause'))
            assert paused['paused']
            assert ConnectionProbe.disconnected.wait(1.5), '暂停必须中断正在等待首字的独立进程请求'
            interrupted_seconds = time.monotonic() - paused_at
            eventually(lambda: api('/api/learning-agent?taskId='+task['id'])['status'] == 'paused')
            queued = [api('/api/learning-agent/events', dict(type='acceptance_tool', requestId='monitor-queued-'+str(index), courseId=info['courseId'], tool='read_context', args={})) for index in range(2)]
            before = api('/api/ai-activity')['todayCalls']
            for _ in range(5):
                view = api('/api/ai-activity')
                assert view['todayCalls'] == before, '暂停期间的状态读取和新任务不得发出模型请求'
            assert all(any(item['id'] == item_task['id'] and item['calls'] == 0 and item['status'] == 'paused' for item in view['tasks']) for item_task in queued)
            with urlopen(Request(info['fixture']+'/fixture-control', data=b'{"slow":false}', headers={'Content-Type':'application/json'})):
                pass
            api('/api/ai-activity/control', dict(command='resume'))
            for item in [task, *queued]:
                eventually(lambda: api('/api/learning-agent?taskId='+item['id'])['status'] == 'ready', 15)
            assert child.wait(timeout=10) == 0, '恢复全部必须继续独立进程中的连接测试'
            final = api('/api/ai-activity')
            connection = next(item for item in final['tasks'] if item['source'] == 'AI 设置')
            assert connection['calls'] == 2 and connection['status'] == 'completed'
            assert ConnectionProbe.count == 2, '连接测试仅恢复未完成的那一次请求'
            assert not final['paused'] and 'PRIVATE-' not in json.dumps(final)
            print(json.dumps(dict(passed=True, allSoftware=True, crossProcess=True, interruptedSeconds=round(interrupted_seconds, 3), queuedWithoutCalls=True,
                                  allTasksResumed=True, directRequestAttempts=connection['calls']), ensure_ascii=False))
        finally:
            if child.poll() is None:
                child.terminate()
                child.wait(timeout=10)
            provider.shutdown()
            provider.server_close()


if __name__ == '__main__':
    main(*sys.argv[1:])

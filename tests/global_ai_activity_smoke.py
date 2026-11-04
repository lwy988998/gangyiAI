# 【逐行批注】 执行当前 Python 语句："""跨进程调用监控验收：只连接本地夹具，不读取真实凭据或真实用户档案。"""。
"""跨进程调用监控验收：只连接本地夹具，不读取真实凭据或真实用户档案。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：import select。
import select
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer。
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from threading import Event, Thread。
from threading import Event, Thread
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen
# 【逐行批注】 导入后续逻辑需要的模块或符号：from classroom_v560_fixture import start_session。
from classroom_v560_fixture import start_session


# 【逐行批注】 定义类 ConnectionProbe，组织相关数据与行为。
class ConnectionProbe(BaseHTTPRequestHandler):
    # 【逐行批注】 计算右侧结果并赋给 started，更新脚本状态。
    started = Event()
    # 【逐行批注】 计算右侧结果并赋给 disconnected，更新脚本状态。
    disconnected = Event()
    # 【逐行批注】 计算右侧结果并赋给 count，更新脚本状态。
    count = 0

    # 【逐行批注】 定义函数 log_message，封装可复用的处理步骤。
    def log_message(self, *_):
        # 【逐行批注】 执行当前 Python 语句：pass。
        pass

    # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
    def do_POST(self):
        # 【逐行批注】 调用 int 完成当前脚本或测试步骤：self.rfile.read(int(self.headers['Content-Length']))。
        self.rfile.read(int(self.headers['Content-Length']))
        # 【逐行批注】 计算右侧结果并赋给 type(self).count +，更新脚本状态。
        type(self).count += 1
        # 【逐行批注】 判断条件 type(self).count == 1:，成立时执行对应分支。
        if type(self).count == 1:
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).started.set()。
            type(self).started.set()
            # 【逐行批注】 计算右侧结果并赋给 readable, _, _，更新脚本状态。
            readable, _, _ = select.select([self.connection], [], [], 3)
            # 【逐行批注】 判断条件 readable and self.connection.recv(1) == b'':，成立时执行对应分支。
            if readable and self.connection.recv(1) == b'':
                # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).disconnected.set()。
                type(self).disconnected.set()
                # 【逐行批注】 结束当前函数并返回结果：return。
                return
        # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
        data = json.dumps(dict(model='fixture', choices=[dict(message=dict(content='OK'))])).encode()
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
            self.send_response(200)
            # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'application/json')。
            self.send_header('Content-Type', 'application/json')
            # 【逐行批注】 调用 str、len 完成当前脚本或测试步骤：self.send_header('Content-Length', str(len(data)))。
            self.send_header('Content-Length', str(len(data)))
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(data)。
            self.wfile.write(data)
        # 【逐行批注】 捕获并处理异常：except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):。
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass


# 【逐行批注】 定义函数 eventually，封装可复用的处理步骤。
def eventually(check, seconds=10):
    # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
    deadline = time.monotonic() + seconds
    # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
    while time.monotonic() < deadline:
        # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
        result = check()
        # 【逐行批注】 判断条件 result:，成立时执行对应分支。
        if result:
            # 【逐行批注】 结束当前函数并返回结果：return result。
            return result
        # 【逐行批注】 执行当前 Python 语句：time.sleep(.05)。
        time.sleep(.05)
    # 【逐行批注】 在错误情形抛出异常：raise AssertionError('状态未在限定时间内到达')。
    raise AssertionError('状态未在限定时间内到达')


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable, probe_executable):
    # 【逐行批注】 在受控上下文中执行资源操作：with start_session(executable) as (info, api, process):。
    with start_session(executable) as (info, api, process):
        # 【逐行批注】 计算右侧结果并赋给 provider，更新脚本状态。
        provider = ThreadingHTTPServer(('127.0.0.1', 0), ConnectionProbe)
        # 【逐行批注】 计算右侧结果并赋给 Thread(target，更新脚本状态。
        Thread(target=provider.serve_forever, daemon=True).start()
        # 【逐行批注】 计算右侧结果并赋给 record，更新脚本状态。
        record = str(Path(info['database']).parent / 'ai-activity.json')
        # 【逐行批注】 计算右侧结果并赋给 child，更新脚本状态。
        child = subprocess.Popen([probe_executable, '--monitor-probe', record, f'http://127.0.0.1:{provider.server_port}/v1'],
                                 # 【逐行批注】 计算右侧结果并赋给 stdout，更新脚本状态。
                                 stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                                 # 【逐行批注】 计算右侧结果并赋给 creationflags，更新脚本状态。
                                 creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert ConnectionProbe.started.wait(5), '独立进程没有发出连接测试请求'。
            assert ConnectionProbe.started.wait(5), '独立进程没有发出连接测试请求'
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control', data=b'{"slow":true}',…。
            with urlopen(Request(info['fixture']+'/fixture-control', data=b'{"slow":true}', headers={'Content-Type':'application/json'})):
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
            # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
            task = api('/api/learning-agent/events', dict(type='acceptance_tool', requestId='monitor-first', courseId=info['courseId'], tool='read_context', args={}))
            # 【逐行批注】 定义函数 both_requesting，封装可复用的处理步骤。
            def both_requesting():
                # 【逐行批注】 计算右侧结果并赋给 view，更新脚本状态。
                view = api('/api/ai-activity')
                # 【逐行批注】 计算右侧结果并赋给 active，更新脚本状态。
                active = [item for item in view['tasks'] if item['status'] in ('requesting', 'receiving')]
                # 【逐行批注】 结束当前函数并返回结果：return view if any(item.get('taskId') == task['id'] for item in activ…。
                return view if any(item.get('taskId') == task['id'] for item in active) and any(item['source'] == 'AI 设置' for item in active) else None
            # 【逐行批注】 计算右侧结果并赋给 running，更新脚本状态。
            running = eventually(both_requesting, 5)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'PRIVATE-' not in json.dumps(running), '监控不能包含标准答案、工具正文或模型提示词'。
            assert 'PRIVATE-' not in json.dumps(running), '监控不能包含标准答案、工具正文或模型提示词'
            # 【逐行批注】 计算右侧结果并赋给 paused_at，更新脚本状态。
            paused_at = time.monotonic()
            # 【逐行批注】 计算右侧结果并赋给 paused，更新脚本状态。
            paused = api('/api/ai-activity/control', dict(command='pause'))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert paused['paused']。
            assert paused['paused']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert ConnectionProbe.disconnected.wait(1.5), '暂停必须中断正在等待首字的独立进程请求'。
            assert ConnectionProbe.disconnected.wait(1.5), '暂停必须中断正在等待首字的独立进程请求'
            # 【逐行批注】 计算右侧结果并赋给 interrupted_seconds，更新脚本状态。
            interrupted_seconds = time.monotonic() - paused_at
            # 【逐行批注】 计算右侧结果并赋给 eventually(lambda: api('/api/learning-agent?…，更新脚本状态。
            eventually(lambda: api('/api/learning-agent?taskId='+task['id'])['status'] == 'paused')
            # 【逐行批注】 计算右侧结果并赋给 queued，更新脚本状态。
            queued = [api('/api/learning-agent/events', dict(type='acceptance_tool', requestId='monitor-queued-'+str(index), courseId=info['courseId'], tool='read_context', args={})) for index in range(2)]
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = api('/api/ai-activity')['todayCalls']
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(5):。
            for _ in range(5):
                # 【逐行批注】 计算右侧结果并赋给 view，更新脚本状态。
                view = api('/api/ai-activity')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert view['todayCalls'] == before, '暂停期间的状态读取和新任务不得发出模型请求'。
                assert view['todayCalls'] == before, '暂停期间的状态读取和新任务不得发出模型请求'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(any(item['id'] == item_task['id'] and item['calls'] == 0 and item['s…。
            assert all(any(item['id'] == item_task['id'] and item['calls'] == 0 and item['status'] == 'paused' for item in view['tasks']) for item_task in queued)
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control', data=b'{"slow":false}'…。
            with urlopen(Request(info['fixture']+'/fixture-control', data=b'{"slow":false}', headers={'Content-Type':'application/json'})):
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
            # 【逐行批注】 计算右侧结果并赋给 api('/api/ai-activity/control', dict(command，更新脚本状态。
            api('/api/ai-activity/control', dict(command='resume'))
            # 【逐行批注】 遍历数据并重复执行循环体：for item in [task, *queued]:。
            for item in [task, *queued]:
                # 【逐行批注】 计算右侧结果并赋给 eventually(lambda: api('/api/learning-agent?…，更新脚本状态。
                eventually(lambda: api('/api/learning-agent?taskId='+item['id'])['status'] == 'ready', 15)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert child.wait(timeout=10) == 0, '恢复全部必须继续独立进程中的连接测试'。
            assert child.wait(timeout=10) == 0, '恢复全部必须继续独立进程中的连接测试'
            # 【逐行批注】 计算右侧结果并赋给 final，更新脚本状态。
            final = api('/api/ai-activity')
            # 【逐行批注】 计算右侧结果并赋给 connection，更新脚本状态。
            connection = next(item for item in final['tasks'] if item['source'] == 'AI 设置')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert connection['calls'] == 2 and connection['status'] == 'completed'。
            assert connection['calls'] == 2 and connection['status'] == 'completed'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert ConnectionProbe.count == 2, '连接测试仅恢复未完成的那一次请求'。
            assert ConnectionProbe.count == 2, '连接测试仅恢复未完成的那一次请求'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not final['paused'] and 'PRIVATE-' not in json.dumps(final)。
            assert not final['paused'] and 'PRIVATE-' not in json.dumps(final)
            # 【逐行批注】 输出当前运行结果：print(json.dumps(dict(passed=True, allSoftware=True, crossProcess=True, in…。
            print(json.dumps(dict(passed=True, allSoftware=True, crossProcess=True, interruptedSeconds=round(interrupted_seconds, 3), queuedWithoutCalls=True,
                                  # 【逐行批注】 计算右侧结果并赋给 allTasksResumed，更新脚本状态。
                                  allTasksResumed=True, directRequestAttempts=connection['calls']), ensure_ascii=False))
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 判断条件 child.poll() is None:，成立时执行对应分支。
            if child.poll() is None:
                # 【逐行批注】 执行当前 Python 语句：child.terminate()。
                child.terminate()
                # 【逐行批注】 计算右侧结果并赋给 child.wait(timeout，更新脚本状态。
                child.wait(timeout=10)
            # 【逐行批注】 执行当前 Python 语句：provider.shutdown()。
            provider.shutdown()
            # 【逐行批注】 执行当前 Python 语句：provider.server_close()。
            provider.server_close()


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(*sys.argv[1:])。
    main(*sys.argv[1:])

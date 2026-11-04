# 【逐行批注】 执行当前 Python 语句："""隔离验证本机 HTTP 与 WebSocket 的来源边界，全程不连接付费模型。"""。
"""隔离验证本机 HTTP 与 WebSocket 的来源边界，全程不连接付费模型。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import asyncio。
import asyncio
# 【逐行批注】 导入后续逻辑需要的模块或符号：import http.client。
import http.client
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
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time

# 【逐行批注】 导入后续逻辑需要的模块或符号：import websockets。
import websockets


# 【逐行批注】 定义函数 exercise，封装可复用的处理步骤。
def exercise(executable):
    # 【逐行批注】 计算右侧结果并赋给 executable，更新脚本状态。
    executable = Path(executable).resolve()
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-http-guard-') as directory:。
    with tempfile.TemporaryDirectory(prefix='gangyi-http-guard-') as directory:
        # 【逐行批注】 计算右侧结果并赋给 root，更新脚本状态。
        root = Path(directory)
        # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as probe:。
        with socket.socket() as probe:
            # 【逐行批注】 执行当前 Python 语句：probe.bind(('127.0.0.1', 0))。
            probe.bind(('127.0.0.1', 0))
            # 【逐行批注】 计算右侧结果并赋给 port，更新脚本状态。
            port = probe.getsockname()[1]
        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
        env = {key: os.environ[key] for key in ('SYSTEMROOT', 'WINDIR', 'PATH', 'COMSPEC', 'PATHEXT') if key in os.environ}
        # 【逐行批注】 计算右侧结果并赋给 env.update(HOST，更新脚本状态。
        env.update(HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(root/'isolated.db'),
                   # 【逐行批注】 计算右侧结果并赋给 AI_BASE_URL，更新脚本状态。
                   AI_BASE_URL='http://127.0.0.1:1/v1', AI_API_KEY='', AI_MODEL='offline-fixture',
                   # 【逐行批注】 计算右侧结果并赋给 LOCAL_CONTROL_TOKEN，更新脚本状态。
                   LOCAL_CONTROL_TOKEN='guard-fixture', LOCALAPPDATA=str(root/'local'),
                   # 【逐行批注】 计算右侧结果并赋给 USERPROFILE，更新脚本状态。
                   USERPROFILE=str(root/'profile'), APPDATA=str(root/'roaming'))
        # 【逐行批注】 在受控上下文中执行资源操作：with (root/'service.log').open('wb') as log:。
        with (root/'service.log').open('wb') as log:
            # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
            process = subprocess.Popen([str(executable)], cwd=executable.parent, env=env, stdout=log, stderr=log)

            # 【逐行批注】 定义函数 request，封装可复用的处理步骤。
            def request(path, body=None, headers=None, method=None, http10=False):
                # 【逐行批注】 计算右侧结果并赋给 connection，更新脚本状态。
                connection = http.client.HTTPConnection('127.0.0.1', port, timeout=5)
                # 【逐行批注】 判断条件 http10:，成立时执行对应分支。
                if http10:
                    # 【逐行批注】 计算右侧结果并赋给 connection._http_vsn, connection._http_vsn_s…，更新脚本状态。
                    connection._http_vsn, connection._http_vsn_str = 10, 'HTTP/1.0'
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
                    data = json.dumps(body).encode() if body is not None else None
                    # 【逐行批注】 计算右侧结果并赋给 connection.request(method or ('POST' if data…，更新脚本状态。
                    connection.request(method or ('POST' if data else 'GET'), path, body=data,
                                       # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                                       headers={'Content-Type': 'application/json', **(headers or {})})
                    # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
                    response = connection.getresponse()
                    # 【逐行批注】 结束当前函数并返回结果：return response.status, response.read()。
                    return response.status, response.read()
                # 【逐行批注】 执行当前 Python 语句：finally:。
                finally:
                    # 【逐行批注】 执行当前 Python 语句：connection.close()。
                    connection.close()

            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(100):。
                for _ in range(100):
                    # 【逐行批注】 开始可能抛出异常的保护代码块。
                    try:
                        # 【逐行批注】 判断条件 request('/health')[0] == 200:，成立时执行对应分支。
                        if request('/health')[0] == 200:
                            # 【逐行批注】 执行当前 Python 语句：break。
                            break
                    # 【逐行批注】 捕获并处理异常：except OSError:。
                    except OSError:
                        # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                        time.sleep(.1)
                # 【逐行批注】 执行前一条件不成立时的分支。
                else:
                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError('隔离服务未就绪')。
                    raise AssertionError('隔离服务未就绪')
                # 【逐行批注】 计算右侧结果并赋给 base，更新脚本状态。
                base = f'http://127.0.0.1:{port}'
                # 【逐行批注】 计算右侧结果并赋给 status, body，更新脚本状态。
                status, body = request('/api/ai-activity/control', {'command': 'pause'}, {'Origin': base, 'Sec-Fetch-Site': 'same-origin'})
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert status == 200 and json.loads(body)['paused']。
                assert status == 200 and json.loads(body)['paused']
                # 缺 Origin 的本机原生管理调用继续兼容；浏览器的伪造来源不能恢复全局 AI。
                # 【逐行批注】 计算右侧结果并赋给 bad，更新脚本状态。
                bad = [
                    # 【逐行批注】 执行当前 Python 语句：{'Origin': 'https://untrusted.example.invalid', 'Content-Type': 'text/plain'},。
                    {'Origin': 'https://untrusted.example.invalid', 'Content-Type': 'text/plain'},
                    # 【逐行批注】 执行当前 Python 语句：{'Origin': 'null'}, {'Origin': ''}, {'Origin': base+'/'},。
                    {'Origin': 'null'}, {'Origin': ''}, {'Origin': base+'/'},
                    # 【逐行批注】 执行当前 Python 语句：{'Origin': f'http://localhost:{port}'}, {'Origin': f'http://127.0.0.1:{port+1}'},。
                    {'Origin': f'http://localhost:{port}'}, {'Origin': f'http://127.0.0.1:{port+1}'},
                    # 【逐行批注】 执行当前 Python 语句：{'Host': f'untrusted.example.invalid:{port}'}, {'Host': f'127.0.0.1:{port+1}'},。
                    {'Host': f'untrusted.example.invalid:{port}'}, {'Host': f'127.0.0.1:{port+1}'},
                    # 【逐行批注】 执行当前 Python 语句：{'Host': f'127.0.0.1:{port}.untrusted.invalid'},。
                    {'Host': f'127.0.0.1:{port}.untrusted.invalid'},
                    # 【逐行批注】 执行当前 Python 语句：{'Sec-Fetch-Site': 'cross-site'}, {'Sec-Fetch-Site': 'same-site'},。
                    {'Sec-Fetch-Site': 'cross-site'}, {'Sec-Fetch-Site': 'same-site'},
                    # 【逐行批注】 执行当前 Python 语句：{'Connection': 'Upgrade', 'Upgrade': 'h2c', 'Origin': 'https://untrusted.invalid'},。
                    {'Connection': 'Upgrade', 'Upgrade': 'h2c', 'Origin': 'https://untrusted.invalid'},
                    # 【逐行批注】 执行当前 Python 语句：{'Connection': 'Upgrade', 'Upgrade': 'h2', 'Host': f'untrusted.invalid:{port}'},。
                    {'Connection': 'Upgrade', 'Upgrade': 'h2', 'Host': f'untrusted.invalid:{port}'},
                # 【逐行批注】 执行当前 Python 语句：]。
                ]
                # 【逐行批注】 遍历数据并重复执行循环体：for headers in bad:。
                for headers in bad:
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert request('/api/ai-activity/control', {'command': 'resume'}, headers)[0] =…。
                    assert request('/api/ai-activity/control', {'command': 'resume'}, headers)[0] == 403, headers
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert json.loads(request('/api/ai-activity')[1])['paused'], headers。
                    assert json.loads(request('/api/ai-activity')[1])['paused'], headers
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert request('/api/my-ai/preferences', headers={'Host': 'untrusted.example.in…。
                assert request('/api/my-ai/preferences', headers={'Host': 'untrusted.example.invalid'})[0] == 403
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 计算右侧结果并赋给 legacy_upgrade，更新脚本状态。
                    legacy_upgrade = request('/api/ai-activity/control', {'command': 'resume'}, {
                        # 【逐行批注】 执行当前 Python 语句：'Host': f'127.0.0.1:{port}', 'Connection': 'Upgrade', 'Upgrade': 'websocket',。
                        'Host': f'127.0.0.1:{port}', 'Connection': 'Upgrade', 'Upgrade': 'websocket',
                        # 【逐行批注】 计算右侧结果并赋给 'Origin': 'https://untrusted.invalid'}, http…，更新脚本状态。
                        'Origin': 'https://untrusted.invalid'}, http10=True)[0]
                # 【逐行批注】 捕获并处理异常：except (ConnectionResetError, http.client.RemoteDisconnected):。
                except (ConnectionResetError, http.client.RemoteDisconnected):
                    # 部分 Crow/HTTP 解析器直接关闭不支持的 HTTP/1.0 升级，拒绝且不执行写入即可。
                    # 【逐行批注】 计算右侧结果并赋给 legacy_upgrade，更新脚本状态。
                    legacy_upgrade = 0
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert legacy_upgrade in (0, 400, 403)。
                assert legacy_upgrade in (0, 400, 403)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert json.loads(request('/api/ai-activity')[1])['paused']。
                assert json.loads(request('/api/ai-activity')[1])['paused']
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert request('/health', headers={'Host': f'localhost:{port}'})[0] == 200。
                assert request('/health', headers={'Host': f'localhost:{port}'})[0] == 200
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert request('/health', headers={'Origin': base, 'Sec-Fetch-Site': 'none'})[0…。
                assert request('/health', headers={'Origin': base, 'Sec-Fetch-Site': 'none'})[0] == 200
                # 两份冲突的 Host/Origin 也不能依赖库选取其中一份来通过校验。
                # 【逐行批注】 遍历数据并重复执行循环体：for name, first, second in [('Host', f'127.0.0.1:{port}', 'untrusted.inval…。
                for name, first, second in [('Host', f'127.0.0.1:{port}', 'untrusted.invalid'),
                                            # 【逐行批注】 执行当前 Python 语句：('Origin', base, 'https://untrusted.invalid')]:。
                                            ('Origin', base, 'https://untrusted.invalid')]:
                    # 【逐行批注】 计算右侧结果并赋给 connection，更新脚本状态。
                    connection = http.client.HTTPConnection('127.0.0.1', port, timeout=5)
                    # 【逐行批注】 计算右侧结果并赋给 connection.putrequest('GET', '/health', skip…，更新脚本状态。
                    connection.putrequest('GET', '/health', skip_host=name == 'Host')
                    # 【逐行批注】 执行当前 Python 语句：connection.putheader(name, first)。
                    connection.putheader(name, first)
                    # 【逐行批注】 执行当前 Python 语句：connection.putheader(name, second)。
                    connection.putheader(name, second)
                    # 【逐行批注】 执行当前 Python 语句：connection.endheaders()。
                    connection.endheaders()
                    # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
                    response = connection.getresponse()
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert response.status in (400, 403)。
                    assert response.status in (400, 403)
                    # 【逐行批注】 执行当前 Python 语句：response.read()。
                    response.read()
                    # 【逐行批注】 执行当前 Python 语句：connection.close()。
                    connection.close()

                # 【逐行批注】 定义函数 sockets，封装可复用的处理步骤。
                async def sockets():
                    # 【逐行批注】 遍历数据并重复执行循环体：for route in ('/ws/ask', '/ws/classroom', '/ws/learn/next', '/ws/learning-…。
                    for route in ('/ws/ask', '/ws/classroom', '/ws/learn/next', '/ws/learning-agent'):
                        # 【逐行批注】 遍历数据并重复执行循环体：for origin in (None, base):。
                        for origin in (None, base):
                            # 【逐行批注】 计算右侧结果并赋给 async with websockets.connect(base.replace('…，更新脚本状态。
                            async with websockets.connect(base.replace('http:', 'ws:')+route, origin=origin, close_timeout=1):
                                # 【逐行批注】 执行当前 Python 语句：pass。
                                pass
                        # 【逐行批注】 遍历数据并重复执行循环体：for origin in ('null', 'https://untrusted.invalid', f'http://localhost:{po…。
                        for origin in ('null', 'https://untrusted.invalid', f'http://localhost:{port}'):
                            # 【逐行批注】 开始可能抛出异常的保护代码块。
                            try:
                                # 【逐行批注】 计算右侧结果并赋给 async with websockets.connect(base.replace('…，更新脚本状态。
                                async with websockets.connect(base.replace('http:', 'ws:')+route, origin=origin, open_timeout=5):
                                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError('不可信 WebSocket 来源被接受：'+route)。
                                    raise AssertionError('不可信 WebSocket 来源被接受：'+route)
                            # 【逐行批注】 捕获并处理异常：except websockets.exceptions.InvalidStatus as error:。
                            except websockets.exceptions.InvalidStatus as error:
                                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert error.response.status_code in (400, 403)。
                                assert error.response.status_code in (400, 403)
                # 【逐行批注】 调用 sockets 完成当前脚本或测试步骤：asyncio.run(sockets())。
                asyncio.run(sockets())
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert request('/api/ai-activity/control', {'command': 'resume'})[0] == 200。
                assert request('/api/ai-activity/control', {'command': 'resume'})[0] == 200
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not json.loads(request('/api/ai-activity')[1])['paused']。
                assert not json.loads(request('/api/ai-activity')[1])['paused']
            # 【逐行批注】 执行当前 Python 语句：finally:。
            finally:
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 调用 request 完成当前脚本或测试步骤：request('/internal/shutdown', {}, {'X-Gangyi-Control-Token': 'guard-f…。
                    request('/internal/shutdown', {}, {'X-Gangyi-Control-Token': 'guard-fixture'})
                    # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                    process.wait(timeout=10)
                # 【逐行批注】 捕获并处理异常：except (OSError, subprocess.TimeoutExpired):。
                except (OSError, subprocess.TimeoutExpired):
                    # 【逐行批注】 执行当前 Python 语句：process.terminate()。
                    process.terminate()
                    # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                    process.wait(timeout=10)
    # 【逐行批注】 输出当前运行结果：print('本机HTTP/四个WebSocket来源、Host、重复头及原生兼容回归通过；付费调用0。')。
    print('本机HTTP/四个WebSocket来源、Host、重复头及原生兼容回归通过；付费调用0。')


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 调用 exercise 完成当前脚本或测试步骤：exercise(sys.argv[1])。
    exercise(sys.argv[1])

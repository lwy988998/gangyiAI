"""隔离验证本机 HTTP 与 WebSocket 的来源边界，全程不连接付费模型。"""
import asyncio
import http.client
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time

import websockets


def exercise(executable):
    executable = Path(executable).resolve()
    with tempfile.TemporaryDirectory(prefix='gangyi-http-guard-') as directory:
        root = Path(directory)
        with socket.socket() as probe:
            probe.bind(('127.0.0.1', 0))
            port = probe.getsockname()[1]
        env = {key: os.environ[key] for key in ('SYSTEMROOT', 'WINDIR', 'PATH', 'COMSPEC', 'PATHEXT') if key in os.environ}
        env.update(HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(root/'isolated.db'),
                   AI_BASE_URL='http://127.0.0.1:1/v1', AI_API_KEY='', AI_MODEL='offline-fixture',
                   LOCAL_CONTROL_TOKEN='guard-fixture', LOCALAPPDATA=str(root/'local'),
                   USERPROFILE=str(root/'profile'), APPDATA=str(root/'roaming'))
        with (root/'service.log').open('wb') as log:
            process = subprocess.Popen([str(executable)], cwd=executable.parent, env=env, stdout=log, stderr=log)

            def request(path, body=None, headers=None, method=None, http10=False):
                connection = http.client.HTTPConnection('127.0.0.1', port, timeout=5)
                if http10:
                    connection._http_vsn, connection._http_vsn_str = 10, 'HTTP/1.0'
                try:
                    data = json.dumps(body).encode() if body is not None else None
                    connection.request(method or ('POST' if data else 'GET'), path, body=data,
                                       headers={'Content-Type': 'application/json', **(headers or {})})
                    response = connection.getresponse()
                    return response.status, response.read()
                finally:
                    connection.close()

            try:
                for _ in range(100):
                    try:
                        if request('/health')[0] == 200:
                            break
                    except OSError:
                        time.sleep(.1)
                else:
                    raise AssertionError('隔离服务未就绪')
                base = f'http://127.0.0.1:{port}'
                status, body = request('/api/ai-activity/control', {'command': 'pause'}, {'Origin': base, 'Sec-Fetch-Site': 'same-origin'})
                assert status == 200 and json.loads(body)['paused']
                # 缺 Origin 的本机原生管理调用继续兼容；浏览器的伪造来源不能恢复全局 AI。
                bad = [
                    {'Origin': 'https://untrusted.example.invalid', 'Content-Type': 'text/plain'},
                    {'Origin': 'null'}, {'Origin': ''}, {'Origin': base+'/'},
                    {'Origin': f'http://localhost:{port}'}, {'Origin': f'http://127.0.0.1:{port+1}'},
                    {'Host': f'untrusted.example.invalid:{port}'}, {'Host': f'127.0.0.1:{port+1}'},
                    {'Host': f'127.0.0.1:{port}.untrusted.invalid'},
                    {'Sec-Fetch-Site': 'cross-site'}, {'Sec-Fetch-Site': 'same-site'},
                    {'Connection': 'Upgrade', 'Upgrade': 'h2c', 'Origin': 'https://untrusted.invalid'},
                    {'Connection': 'Upgrade', 'Upgrade': 'h2', 'Host': f'untrusted.invalid:{port}'},
                ]
                for headers in bad:
                    assert request('/api/ai-activity/control', {'command': 'resume'}, headers)[0] == 403, headers
                    assert json.loads(request('/api/ai-activity')[1])['paused'], headers
                assert request('/api/my-ai/preferences', headers={'Host': 'untrusted.example.invalid'})[0] == 403
                try:
                    legacy_upgrade = request('/api/ai-activity/control', {'command': 'resume'}, {
                        'Host': f'127.0.0.1:{port}', 'Connection': 'Upgrade', 'Upgrade': 'websocket',
                        'Origin': 'https://untrusted.invalid'}, http10=True)[0]
                except (ConnectionResetError, http.client.RemoteDisconnected):
                    # 部分 Crow/HTTP 解析器直接关闭不支持的 HTTP/1.0 升级，拒绝且不执行写入即可。
                    legacy_upgrade = 0
                assert legacy_upgrade in (0, 400, 403)
                assert json.loads(request('/api/ai-activity')[1])['paused']
                assert request('/health', headers={'Host': f'localhost:{port}'})[0] == 200
                assert request('/health', headers={'Origin': base, 'Sec-Fetch-Site': 'none'})[0] == 200
                # 两份冲突的 Host/Origin 也不能依赖库选取其中一份来通过校验。
                for name, first, second in [('Host', f'127.0.0.1:{port}', 'untrusted.invalid'),
                                            ('Origin', base, 'https://untrusted.invalid')]:
                    connection = http.client.HTTPConnection('127.0.0.1', port, timeout=5)
                    connection.putrequest('GET', '/health', skip_host=name == 'Host')
                    connection.putheader(name, first)
                    connection.putheader(name, second)
                    connection.endheaders()
                    response = connection.getresponse()
                    assert response.status in (400, 403)
                    response.read()
                    connection.close()

                async def sockets():
                    for route in ('/ws/ask', '/ws/classroom', '/ws/learn/next', '/ws/learning-agent'):
                        for origin in (None, base):
                            async with websockets.connect(base.replace('http:', 'ws:')+route, origin=origin, close_timeout=1):
                                pass
                        for origin in ('null', 'https://untrusted.invalid', f'http://localhost:{port}'):
                            try:
                                async with websockets.connect(base.replace('http:', 'ws:')+route, origin=origin, open_timeout=5):
                                    raise AssertionError('不可信 WebSocket 来源被接受：'+route)
                            except websockets.exceptions.InvalidStatus as error:
                                assert error.response.status_code in (400, 403)
                asyncio.run(sockets())
                assert request('/api/ai-activity/control', {'command': 'resume'})[0] == 200
                assert not json.loads(request('/api/ai-activity')[1])['paused']
            finally:
                try:
                    request('/internal/shutdown', {}, {'X-Gangyi-Control-Token': 'guard-fixture'})
                    process.wait(timeout=10)
                except (OSError, subprocess.TimeoutExpired):
                    process.terminate()
                    process.wait(timeout=10)
    print('本机HTTP/四个WebSocket来源、Host、重复头及原生兼容回归通过；付费调用0。')


if __name__ == '__main__':
    exercise(sys.argv[1])

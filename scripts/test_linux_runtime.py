"""Linux 发布服务的静态资源、隔离档案、输入边界与优雅关闭验收。"""
import http.client
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time


def main(executable):
    repo = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='gangyi-linux-runtime-') as directory:
        with socket.socket() as sock:
            sock.bind(('127.0.0.1', 0)); port = sock.getsockname()[1]
        token = 'fictional-linux-control'
        env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(Path(directory) / 'isolated.db'),
            LOCAL_CONTROL_TOKEN=token, AI_BASE_URL='http://127.0.0.1:9/v1', AI_API_KEY='', AI_MODEL='fictional-unconfigured')
        log_path = Path(directory) / 'service.log'
        with log_path.open('wb') as log:
            process = subprocess.Popen([str(Path(executable).resolve())], cwd=repo, env=env, stdout=log, stderr=log)
            def request(path, method='GET', body=None, status=200, headers=None):
                connection = http.client.HTTPConnection('127.0.0.1', port, timeout=10)
                actual_headers = headers or {}
                if isinstance(body, dict):
                    body = json.dumps(body).encode(); actual_headers = dict(actual_headers, **{'Content-Type': 'application/json'})
                connection.request(method, path, body, actual_headers)
                response = connection.getresponse(); data = response.read(); result_headers = dict(response.getheaders()); connection.close()
                assert response.status == status, (path, response.status, data[:300])
                return data, result_headers
            try:
                for _ in range(200):
                    try:
                        data, _ = request('/health'); assert json.loads(data)['status'] == 'healthy'; break
                    except OSError:
                        time.sleep(.1)
                else: raise AssertionError('隔离服务没有启动')
                for path in ('/', '/ask', '/my-courses', '/agent-prepare.html', '/agent-classroom.html'):
                    data, _ = request(path); assert '钢一定制AI'.encode() in data
                for path, mime in (('/styles.css', 'text/css'), ('/learning-agent.js', 'javascript'), ('/agent-shell.js', 'javascript'),
                    ('/agent-classroom.css', 'text/css'), ('/school-logo.png', 'image/png'), ('/campus-background.jpg', 'image/jpeg')):
                    data, headers = request(path); assert len(data) > 100 and mime in headers['Content-Type']
                request('/../CMakeLists.txt', status=404)
                request('/api/ask', 'POST', b'{', status=409)
                request('/api/generate-plan', 'POST', dict(mode='lite'), status=400)
                request('/api/classroom/submit', 'POST', dict(courseId='missing', kind='quiz', index=0, answer=0), status=409)
                request('/internal/shutdown', 'POST', headers={'X-Gangyi-Control-Token': 'wrong'}, status=403)
                for _ in range(200):
                    data, _ = request('/api/learning-agent'); state = json.loads(data)
                    if state['status'] == 'failed': break
                    time.sleep(.03)
                assert state['status'] == 'failed' and state['calls'] == 3
                data, _ = request('/api/home/recommendations'); recommendations = json.loads(data)
                assert recommendations['items']['lite'] == [] and recommendations['items']['deep'] == [], '缺少真实 AI 时不能编造推荐'
                request('/internal/shutdown', 'POST', headers={'X-Gangyi-Control-Token': token})
                assert process.wait(timeout=15) == 0
                log.flush(); assert token not in log_path.read_text('utf-8', errors='replace')
            finally:
                if process.poll() is None: process.terminate(); process.wait(timeout=15)
    print('Linux 运行时资源、参数边界、三次失败无模板回退与优雅退出通过')


if __name__ == '__main__':
    main(sys.argv[1])

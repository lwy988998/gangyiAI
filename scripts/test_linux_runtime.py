# 【逐行批注】 执行当前 Python 语句："""Linux 发布服务的静态资源、隔离档案、输入边界与优雅关闭验收。"""。
"""Linux 发布服务的静态资源、隔离档案、输入边界与优雅关闭验收。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import http.client。
import http.client
# 【逐行批注】 导入后续逻辑需要的模块或符号：import argparse。
import argparse
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


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable, working_directory=None):
    # 【逐行批注】 计算右侧结果并赋给 repo，更新脚本状态。
    repo = Path(working_directory).resolve() if working_directory else Path(__file__).resolve().parents[1]
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-linux-runtime-') as directory:。
    with tempfile.TemporaryDirectory(prefix='gangyi-linux-runtime-') as directory:
        # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as sock:。
        with socket.socket() as sock:
            # 【逐行批注】 计算右侧结果并赋给 sock.bind(('127.0.0.1', 0)); port，更新脚本状态。
            sock.bind(('127.0.0.1', 0)); port = sock.getsockname()[1]
        # 【逐行批注】 计算右侧结果并赋给 token，更新脚本状态。
        token = 'fictional-linux-control'
        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
        env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(Path(directory) / 'isolated.db'),
            # 【逐行批注】 计算右侧结果并赋给 LOCAL_CONTROL_TOKEN，更新脚本状态。
            LOCAL_CONTROL_TOKEN=token, AI_BASE_URL='http://127.0.0.1:9/v1', AI_API_KEY='', AI_MODEL='fictional-unconfigured')
        # 【逐行批注】 计算右侧结果并赋给 log_path，更新脚本状态。
        log_path = Path(directory) / 'service.log'
        # 【逐行批注】 在受控上下文中执行资源操作：with log_path.open('wb') as log:。
        with log_path.open('wb') as log:
            # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
            process = subprocess.Popen([str(Path(executable).resolve())], cwd=repo, env=env, stdout=log, stderr=log,
                # 【逐行批注】 计算右侧结果并赋给 creationflags，更新脚本状态。
                creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
            # 【逐行批注】 定义函数 request，封装可复用的处理步骤。
            def request(path, method='GET', body=None, status=200, headers=None):
                # 【逐行批注】 计算右侧结果并赋给 connection，更新脚本状态。
                connection = http.client.HTTPConnection('127.0.0.1', port, timeout=10)
                # 【逐行批注】 计算右侧结果并赋给 actual_headers，更新脚本状态。
                actual_headers = headers or {}
                # 【逐行批注】 判断条件 isinstance(body, dict):，成立时执行对应分支。
                if isinstance(body, dict):
                    # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
                    body = json.dumps(body).encode(); actual_headers = dict(actual_headers, **{'Content-Type': 'application/json'})
                # 【逐行批注】 执行当前 Python 语句：connection.request(method, path, body, actual_headers)。
                connection.request(method, path, body, actual_headers)
                # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
                response = connection.getresponse(); data = response.read(); result_headers = dict(response.getheaders()); connection.close()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert response.status == status, (path, response.status, data[:300])。
                assert response.status == status, (path, response.status, data[:300])
                # 【逐行批注】 结束当前函数并返回结果：return data, result_headers。
                return data, result_headers
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(200):。
                for _ in range(200):
                    # 【逐行批注】 开始可能抛出异常的保护代码块。
                    try:
                        # 【逐行批注】 计算右侧结果并赋给 data, _，更新脚本状态。
                        data, _ = request('/health'); assert json.loads(data)['status'] == 'healthy'; break
                    # 【逐行批注】 捕获并处理异常：except OSError:。
                    except OSError:
                        # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                        time.sleep(.1)
                # 【逐行批注】 执行前一条件不成立时的分支。
                else: raise AssertionError('隔离服务没有启动')
                # 【逐行批注】 遍历数据并重复执行循环体：for path in ('/', '/ask', '/my-courses', '/agent-prepare.html'):。
                for path in ('/', '/ask', '/my-courses', '/agent-prepare.html'):
                    # 【逐行批注】 计算右侧结果并赋给 data, _，更新脚本状态。
                    data, _ = request(path); assert '钢一定制AI'.encode() in data, path
                # 旧课堂链接负责转入统一课堂，并保留课时和复习参数。
                # 【逐行批注】 计算右侧结果并赋给 data, _，更新脚本状态。
                data, _ = request('/agent-classroom.html?lessonId=legacy&review=1')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert "location.replace('/learn'+location.search)".encode() in data。
                assert "location.replace('/learn'+location.search)".encode() in data
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '正在恢复已保存的课时'.encode() in data。
                assert '正在恢复已保存的课时'.encode() in data
                # 【逐行批注】 遍历数据并重复执行循环体：for path, mime in (('/styles.css', 'text/css'), ('/learning-agent.js', 'ja…。
                for path, mime in (('/styles.css', 'text/css'), ('/learning-agent.js', 'javascript'), ('/agent-shell.js', 'javascript'),
                    # 【逐行批注】 执行当前 Python 语句：('/agent-classroom.css', 'text/css'), ('/app-layout.css', 'text/css'), ('/app-layout.js',…。
                    ('/agent-classroom.css', 'text/css'), ('/app-layout.css', 'text/css'), ('/app-layout.js', 'javascript'),
                    # 【逐行批注】 执行当前 Python 语句：('/school-logo.png', 'image/png'), ('/campus-background.jpg', 'image/jpeg')):。
                    ('/school-logo.png', 'image/png'), ('/campus-background.jpg', 'image/jpeg')):
                    # 【逐行批注】 计算右侧结果并赋给 data, headers，更新脚本状态。
                    data, headers = request(path); assert len(data) > 100 and mime in headers['Content-Type']
                # 【逐行批注】 计算右侧结果并赋给 request('/../CMakeLists.txt', status，更新脚本状态。
                request('/../CMakeLists.txt', status=404)
                # 【逐行批注】 计算右侧结果并赋给 request('/api/ask', 'POST', b'{', status，更新脚本状态。
                request('/api/ask', 'POST', b'{', status=409)
                # 【逐行批注】 计算右侧结果并赋给 request('/api/generate-plan', 'POST', dict(m…，更新脚本状态。
                request('/api/generate-plan', 'POST', dict(mode='lite'), status=400)
                # 【逐行批注】 计算右侧结果并赋给 request('/api/classroom/submit', 'POST', dic…，更新脚本状态。
                request('/api/classroom/submit', 'POST', dict(courseId='missing', kind='quiz', index=0, answer=0), status=409)
                # 【逐行批注】 计算右侧结果并赋给 request('/internal/shutdown', 'POST', headers，更新脚本状态。
                request('/internal/shutdown', 'POST', headers={'X-Gangyi-Control-Token': 'wrong'}, status=403)
                # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(200):。
                for _ in range(200):
                    # 【逐行批注】 计算右侧结果并赋给 data, _，更新脚本状态。
                    data, _ = request('/api/learning-agent'); state = json.loads(data)
                    # 【逐行批注】 判断条件 state['status'] == 'failed': break，成立时执行对应分支。
                    if state['status'] == 'failed': break
                    # 【逐行批注】 执行当前 Python 语句：time.sleep(.03)。
                    time.sleep(.03)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['status'] == 'failed' and state['calls'] == 1。
                assert state['status'] == 'failed' and state['calls'] == 1
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['failure']['kind'] == 'missing_config' and not state['failure']['r…。
                assert state['failure']['kind'] == 'missing_config' and not state['failure']['retryable']
                # 【逐行批注】 计算右侧结果并赋给 data, _，更新脚本状态。
                data, _ = request('/api/home/recommendations'); recommendations = json.loads(data)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert recommendations['items']['lite'] == [] and recommendations['items']['dee…。
                assert recommendations['items']['lite'] == [] and recommendations['items']['deep'] == [], '缺少真实 AI 时不能编造推荐'
                # 【逐行批注】 计算右侧结果并赋给 request('/internal/shutdown', 'POST', headers，更新脚本状态。
                request('/internal/shutdown', 'POST', headers={'X-Gangyi-Control-Token': token})
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert process.wait(timeout=15) == 0。
                assert process.wait(timeout=15) == 0
                # 【逐行批注】 计算右侧结果并赋给 log.flush(); assert token not in log_path.re…，更新脚本状态。
                log.flush(); assert token not in log_path.read_text('utf-8', errors='replace')
            # 【逐行批注】 执行当前 Python 语句：finally:。
            finally:
                # 【逐行批注】 判断条件 process.poll() is None: process.terminate(); process.wait(timeou…，成立时执行对应分支。
                if process.poll() is None: process.terminate(); process.wait(timeout=15)
    # 【逐行批注】 输出当前运行结果：print('运行时资源、参数边界、未配置鉴权立即停止、无模板回退与优雅退出通过；工作目录：'+str(repo))。
    print('运行时资源、参数边界、未配置鉴权立即停止、无模板回退与优雅退出通过；工作目录：'+str(repo))


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encod…，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 计算右侧结果并赋给 parser，更新脚本状态。
    parser = argparse.ArgumentParser(description=__doc__)
    # 【逐行批注】 执行当前 Python 语句：parser.add_argument('executable')。
    parser.add_argument('executable')
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('--working-directory', h…，更新脚本状态。
    parser.add_argument('--working-directory', help='候选包目录；从此目录读取包内public，避免误用仓库资源')
    # 【逐行批注】 计算右侧结果并赋给 arguments，更新脚本状态。
    arguments = parser.parse_args()
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(arguments.executable, arguments.working_directory)。
    main(arguments.executable, arguments.working_directory)

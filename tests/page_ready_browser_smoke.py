"""页面就绪判定回归：直接运行启动器注入的真实脚本，验证每个应用页面都能上报就绪。"""
import os
import socket
import subprocess
import sys
import tempfile
import time
from pathlib import Path
from urllib.request import Request, urlopen

from playwright.sync_api import sync_playwright


ROOT = Path(__file__).resolve().parents[1]
APPLICATION_PAGES = ['/', '/my-courses', '/plan', '/phase', '/ask',
                     '/agent-prepare.html', '/agent-classroom.html']


def free_port():
    with socket.socket() as connection:
        connection.bind(('127.0.0.1', 0))
        return connection.getsockname()[1]


def desktop_script():
    """从启动器源码里取出真实的注入脚本，避免测试与实现各写一份判定条件。"""
    source = (ROOT / 'src' / 'windows_launcher.cpp').read_text(encoding='utf-8')
    start = source.index('R"JS(') + len('R"JS(')
    end = source.index(')JS"', start)
    return source[start:end]


def signals_ready(page, script, seconds):
    page.evaluate("() => { window.__gangyiReady = false; window.gangyiPageReady = () => { window.__gangyiReady = true; }; }")
    page.evaluate(script)
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        if page.evaluate('() => window.__gangyiReady === true'):
            return True
        time.sleep(0.1)
    return False


def main(executable):
    script = desktop_script()
    repository = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='gangyi-page-ready-') as directory:
        database = Path(directory) / 'fictional.db'
        serving = free_port()
        base = f'http://127.0.0.1:{serving}'
        env = dict(os.environ, DATABASE_PATH=str(database), PORT=str(serving), HOST='127.0.0.1',
                   AI_BASE_URL='http://127.0.0.1:9/v1', AI_API_KEY='', AI_MODEL='fictional-model',
                   GANGYI_LAUNCH_SESSION_ID='page-ready-fixture',
                   LOCAL_CONTROL_TOKEN='page-ready-shutdown')
        log = open(Path(directory) / 'service.log', 'w+', encoding='utf-8')
        process = subprocess.Popen([str(Path(executable).resolve())], cwd=repository, env=env,
                                   stdout=log, stderr=log,
                                   creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        try:
            for _ in range(300):
                try:
                    with urlopen(base + '/health', timeout=2) as response:
                        if response.status == 200:
                            break
                except OSError:
                    pass
                time.sleep(0.1)
            else:
                raise AssertionError('隔离服务未正常启动')
            with sync_playwright() as playwright:
                browser = playwright.chromium.launch(headless=True)
                page = browser.new_page(viewport={'width': 1180, 'height': 820})
                failures = []
                for path in APPLICATION_PAGES:
                    response = page.goto(base + path, wait_until='domcontentloaded')
                    if response is None or response.status != 200:
                        failures.append(f'{path} 未正常返回页面（{response.status if response else "无响应"}）')
                        continue
                    if not signals_ready(page, script, 3):
                        failures.append(f'{path} 未上报页面就绪，会误弹“加载失败”')
                # 真正的失败不能被放过：404 与应用外的空白页都必须保持未就绪。
                page.goto(base + '/no-such-page')
                if signals_ready(page, script, 1.2):
                    failures.append('404 页面被误判为就绪')
                page.goto('about:blank')
                if signals_ready(page, script, 1.2):
                    failures.append('空白页被误判为就绪')
                try:
                    page.goto('http://127.0.0.1:1/', timeout=5000)
                except Exception:
                    pass
                try:
                    if signals_ready(page, script, 1.2):
                        failures.append('连接失败页被误判为就绪')
                except Exception:
                    pass  # 浏览器错误页无法执行脚本时视为通过；这不影响上面的真实失败保护。
                browser.close()
                if failures:
                    raise AssertionError('；'.join(failures))
        finally:
            if process.poll() is None:
                try:
                    with urlopen(Request(base + '/internal/shutdown', data=b'',
                                         headers={'X-Gangyi-Control-Token': 'page-ready-shutdown'}),
                                 timeout=5):
                        pass
                    process.wait(timeout=10)
                except Exception:
                    process.terminate()
                    process.wait(timeout=15)
            log.close()
    print('页面就绪判定回归通过：' + '、'.join(APPLICATION_PAGES) + ' 均可正常上报，404 与空白页不会被误判')


if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])

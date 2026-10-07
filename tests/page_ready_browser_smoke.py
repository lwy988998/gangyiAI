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
from classroom_v560_fixture import start_session


ROOT = Path(__file__).resolve().parents[1]



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
    with start_session(executable) as (info, api, process):
        lesson = info['lessonId']
        paths = ['/', '/startup', '/my-courses', '/my-courses#profile',
                 '/plan?courseId=' + info['courseId'],
                 '/phase?courseId=' + info['courseId'] + '&phaseIndex=1', '/ask',
                 '/agent-prepare.html?taskId=' + api('/api/learning-agent')['id'],
                 '/learn?lessonId=' + lesson, '/practice?lessonId=' + lesson,
                 '/summary?lessonId=' + lesson, '/agent-classroom.html?lessonId=' + lesson]
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True)
            page = browser.new_page(viewport={'width':1180,'height':820}, reduced_motion='reduce')
            failures = []
            for path in paths:
                target = info['base'] + path
                response = page.goto(target, wait_until='domcontentloaded')
                # 页内画像页签只改变锚点，没有新的 HTTP 响应；仍核对目标与实际就绪状态。
                hash_navigation = response is None and '#' in path and page.url == target
                if (response is None and not hash_navigation) or (response is not None and response.status != 200):
                    failures.append(path + ' 未正常返回页面')
                    continue
                if not signals_ready(page, script, 3):
                    failures.append(path + ' 未上报页面就绪')
            page.goto(info['base'] + '/no-such-page')
            if signals_ready(page, script, 1.2): failures.append('404 被误判为就绪')
            page.goto('about:blank')
            if signals_ready(page, script, 1.2): failures.append('空白页被误判为就绪')
            try:
                page.goto('http://127.0.0.1:1/', timeout=5000)
            except Exception:
                pass
            try:
                if signals_ready(page, script, 1.2): failures.append('连接失败被误判为就绪')
            except Exception:
                pass
            browser.close()
            assert not failures, '；'.join(failures)
    print('页面就绪覆盖首页、启动、课程、阶段、导师、备课、课堂三页和旧链接，保留真实失败保护。')


if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])

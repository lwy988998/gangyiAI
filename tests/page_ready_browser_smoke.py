# 【逐行批注】 执行当前 Python 语句："""页面就绪判定回归：直接运行启动器注入的真实脚本，验证每个应用页面都能上报就绪。"""。
"""页面就绪判定回归：直接运行启动器注入的真实脚本，验证每个应用页面都能上报就绪。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
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
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen

# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import sync_playwright。
from playwright.sync_api import sync_playwright
# 【逐行批注】 导入后续逻辑需要的模块或符号：from classroom_v560_fixture import start_session。
from classroom_v560_fixture import start_session


# 【逐行批注】 计算右侧结果并赋给 ROOT，更新脚本状态。
ROOT = Path(__file__).resolve().parents[1]



# 【逐行批注】 定义函数 free_port，封装可复用的处理步骤。
def free_port():
    # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as connection:。
    with socket.socket() as connection:
        # 【逐行批注】 执行当前 Python 语句：connection.bind(('127.0.0.1', 0))。
        connection.bind(('127.0.0.1', 0))
        # 【逐行批注】 结束当前函数并返回结果：return connection.getsockname()[1]。
        return connection.getsockname()[1]


# 【逐行批注】 定义函数 desktop_script，封装可复用的处理步骤。
def desktop_script():
    # 【逐行批注】 执行当前 Python 语句："""从启动器源码里取出真实的注入脚本，避免测试与实现各写一份判定条件。"""。
    """从启动器源码里取出真实的注入脚本，避免测试与实现各写一份判定条件。"""
    # 【逐行批注】 计算右侧结果并赋给 source，更新脚本状态。
    source = (ROOT / 'src' / 'windows_launcher.cpp').read_text(encoding='utf-8')
    # 【逐行批注】 计算右侧结果并赋给 start，更新脚本状态。
    start = source.index('R"JS(') + len('R"JS(')
    # 【逐行批注】 计算右侧结果并赋给 end，更新脚本状态。
    end = source.index(')JS"', start)
    # 【逐行批注】 结束当前函数并返回结果：return source[start:end]。
    return source[start:end]


# 【逐行批注】 定义函数 signals_ready，封装可复用的处理步骤。
def signals_ready(page, script, seconds):
    # 【逐行批注】 计算右侧结果并赋给 page.evaluate("()，更新脚本状态。
    page.evaluate("() => { window.__gangyiReady = false; window.gangyiPageReady = () => { window.__gangyiReady = true; }; }")
    # 【逐行批注】 执行当前 Python 语句：page.evaluate(script)。
    page.evaluate(script)
    # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
    deadline = time.monotonic() + seconds
    # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
    while time.monotonic() < deadline:
        # 【逐行批注】 判断条件 page.evaluate('() => window.__gangyiReady === true'):，成立时执行对应分支。
        if page.evaluate('() => window.__gangyiReady === true'):
            # 【逐行批注】 结束当前函数并返回结果：return True。
            return True
        # 【逐行批注】 执行当前 Python 语句：time.sleep(0.1)。
        time.sleep(0.1)
    # 【逐行批注】 结束当前函数并返回结果：return False。
    return False


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable):
    # 【逐行批注】 计算右侧结果并赋给 script，更新脚本状态。
    script = desktop_script()
    # 【逐行批注】 在受控上下文中执行资源操作：with start_session(executable) as (info, api, process):。
    with start_session(executable) as (info, api, process):
        # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
        lesson = info['lessonId']
        # 【逐行批注】 计算右侧结果并赋给 paths，更新脚本状态。
        paths = ['/', '/startup', '/current-course', '/my-courses', '/my-courses#profile',
                 # 【逐行批注】 计算右侧结果并赋给 '/plan?courseId，更新脚本状态。
                 '/plan?courseId=' + info['courseId'],
                 # 【逐行批注】 计算右侧结果并赋给 '/phase?courseId，更新脚本状态。
                 '/phase?courseId=' + info['courseId'] + '&phaseIndex=1', '/ask',
                 # 【逐行批注】 计算右侧结果并赋给 '/agent-prepare.html?taskId，更新脚本状态。
                 '/agent-prepare.html?taskId=' + api('/api/learning-agent')['id'],
                 # 【逐行批注】 计算右侧结果并赋给 '/learn?lessonId，更新脚本状态。
                 '/learn?lessonId=' + lesson, '/practice?lessonId=' + lesson,
                 # 【逐行批注】 计算右侧结果并赋给 '/summary?lessonId，更新脚本状态。
                 '/summary?lessonId=' + lesson, '/agent-classroom.html?lessonId=' + lesson]
        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as playwright:。
        with sync_playwright() as playwright:
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser = playwright.chromium.launch(headless=True)
            # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
            page = browser.new_page(viewport={'width':1180,'height':820}, reduced_motion='reduce')
            # 【逐行批注】 计算右侧结果并赋给 failures，更新脚本状态。
            failures = []
            # 【逐行批注】 遍历数据并重复执行循环体：for path in paths:。
            for path in paths:
                # 【逐行批注】 计算右侧结果并赋给 target，更新脚本状态。
                target = info['base'] + path
                # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
                response = page.goto(target, wait_until='domcontentloaded')
                # 页内画像页签只改变锚点，没有新的 HTTP 响应；仍核对目标与实际就绪状态。
                # 【逐行批注】 计算右侧结果并赋给 hash_navigation，更新脚本状态。
                hash_navigation = response is None and '#' in path and page.url == target
                # 【逐行批注】 判断条件 (response is None and not hash_navigation) or (response is not N…，成立时执行对应分支。
                if (response is None and not hash_navigation) or (response is not None and response.status != 200):
                    # 【逐行批注】 执行当前 Python 语句：failures.append(path + ' 未正常返回页面')。
                    failures.append(path + ' 未正常返回页面')
                    # 【逐行批注】 执行当前 Python 语句：continue。
                    continue
                # 【逐行批注】 判断条件 not signals_ready(page, script, 3):，成立时执行对应分支。
                if not signals_ready(page, script, 3):
                    # 【逐行批注】 执行当前 Python 语句：failures.append(path + ' 未上报页面就绪')。
                    failures.append(path + ' 未上报页面就绪')
            # 【逐行批注】 执行当前 Python 语句：page.goto(info['base'] + '/no-such-page')。
            page.goto(info['base'] + '/no-such-page')
            # 【逐行批注】 判断条件 signals_ready(page, script, 1.2): failures.append('404 被误判为就绪')，成立时执行对应分支。
            if signals_ready(page, script, 1.2): failures.append('404 被误判为就绪')
            # 【逐行批注】 执行当前 Python 语句：page.goto('about:blank')。
            page.goto('about:blank')
            # 【逐行批注】 判断条件 signals_ready(page, script, 1.2): failures.append('空白页被误判为就绪')，成立时执行对应分支。
            if signals_ready(page, script, 1.2): failures.append('空白页被误判为就绪')
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 计算右侧结果并赋给 page.goto('http://127.0.0.1:1/', timeout，更新脚本状态。
                page.goto('http://127.0.0.1:1/', timeout=5000)
            # 【逐行批注】 捕获并处理异常：except Exception:。
            except Exception:
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 判断条件 signals_ready(page, script, 1.2): failures.append('连接失败被误判为就绪')，成立时执行对应分支。
                if signals_ready(page, script, 1.2): failures.append('连接失败被误判为就绪')
            # 【逐行批注】 捕获并处理异常：except Exception:。
            except Exception:
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
            # 【逐行批注】 执行当前 Python 语句：browser.close()。
            browser.close()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not failures, '；'.join(failures)。
            assert not failures, '；'.join(failures)
    # 【逐行批注】 输出当前运行结果：print('页面就绪覆盖首页、启动、课程、阶段、导师、备课、课堂三页和旧链接，保留真实失败保护。')。
    print('页面就绪覆盖首页、启动、课程、阶段、导师、备课、课堂三页和旧链接，保留真实失败保护。')


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'):，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'):
        # 【逐行批注】 计算右侧结果并赋给 sys.stdout.reconfigure(encoding，更新脚本状态。
        sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(sys.argv[1])。
    main(sys.argv[1])

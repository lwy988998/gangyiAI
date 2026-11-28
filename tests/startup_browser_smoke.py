# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from tempfile import TemporaryDirectory。
from tempfile import TemporaryDirectory
# 【逐行批注】 导入后续逻辑需要的模块或符号：from time import monotonic, sleep。
from time import monotonic, sleep
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：import socket。
import socket
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess

# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import sync_playwright。
from playwright.sync_api import sync_playwright


# 【逐行批注】 计算右侧结果并赋给 ROOT，更新脚本状态。
ROOT = Path(__file__).resolve().parents[1]
# 【逐行批注】 计算右侧结果并赋给 SERVICE，更新脚本状态。
SERVICE = Path(os.environ.get("GANGYI_SERVICE_EXE", str(ROOT / "build-ascii" / "gangyiAI.exe"))).resolve()
# 【逐行批注】 计算右侧结果并赋给 SHOTS，更新脚本状态。
SHOTS = ROOT / "verification" / "startup-scan-v410" / "browser-shots"


# 【逐行批注】 定义函数 free_port，封装可复用的处理步骤。
def free_port():
    # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as connection:。
    with socket.socket() as connection:
        # 【逐行批注】 执行当前 Python 语句：connection.bind(("127.0.0.1", 0))。
        connection.bind(("127.0.0.1", 0))
        # 【逐行批注】 结束当前函数并返回结果：return connection.getsockname()[1]。
        return connection.getsockname()[1]


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 计算右侧结果并赋给 SHOTS.mkdir(parents，更新脚本状态。
    SHOTS.mkdir(parents=True, exist_ok=True)
    # 【逐行批注】 计算右侧结果并赋给 port，更新脚本状态。
    port = free_port()
    # 【逐行批注】 计算右侧结果并赋给 base，更新脚本状态。
    base = f"http://127.0.0.1:{port}"
    # 【逐行批注】 在受控上下文中执行资源操作：with TemporaryDirectory(prefix="gangyi-startup-") as temporary:。
    with TemporaryDirectory(prefix="gangyi-startup-") as temporary:
        # 【逐行批注】 计算右侧结果并赋给 environment，更新脚本状态。
        environment = os.environ.copy()
        # 【逐行批注】 计算右侧结果并赋给 environment.update(HOST，更新脚本状态。
        environment.update(HOST="127.0.0.1", PORT=str(port), DATABASE_PATH=str(Path(temporary) / "test.db"),
                           # 【逐行批注】 计算右侧结果并赋给 AI_API_KEY，更新脚本状态。
                           AI_API_KEY="", LOCAL_CONTROL_TOKEN="startup-test-token")
        # 【逐行批注】 计算右侧结果并赋给 service，更新脚本状态。
        service = subprocess.Popen([str(SERVICE)], cwd=SERVICE.parent, env=environment,
                                   # 【逐行批注】 计算右侧结果并赋给 stdout，更新脚本状态。
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                   # 【逐行批注】 计算右侧结果并赋给 creationflags，更新脚本状态。
                                   creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = monotonic() + 20
            # 【逐行批注】 在条件成立期间重复执行：while monotonic() < deadline:。
            while monotonic() < deadline:
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 判断条件 urlopen(base + "/health", timeout=1).status == 200:，成立时执行对应分支。
                    if urlopen(base + "/health", timeout=1).status == 200:
                        # 【逐行批注】 执行当前 Python 语句：break。
                        break
                # 【逐行批注】 捕获并处理异常：except Exception:。
                except Exception:
                    # 【逐行批注】 调用 sleep 完成当前脚本或测试步骤：sleep(.15)。
                    sleep(.15)
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 在错误情形抛出异常：raise AssertionError("本地服务未就绪")。
                raise AssertionError("本地服务未就绪")
            # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as playwright:。
            with sync_playwright() as playwright:
                # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
                browser = playwright.chromium.launch(headless=True)
                # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
                page = browser.new_page(viewport={"width": 1120, "height": 760})
                # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
                errors = []
                # 【逐行批注】 调用 str 完成当前脚本或测试步骤：page.on("pageerror", lambda error: errors.append(str(error)))。
                page.on("pageerror", lambda error: errors.append(str(error)))
                # 【逐行批注】 执行当前 Python 语句：page.on("response", lambda response: errors.append(f"HTTP {response.status}: {response.ur…。
                page.on("response", lambda response: errors.append(f"HTTP {response.status}: {response.url}")
                        # 【逐行批注】 判断条件 response.status >= 400 else None)，成立时执行对应分支。
                        if response.status >= 400 else None)
                # 【逐行批注】 计算右侧结果并赋给 page.goto(base + "/", wait_until，更新脚本状态。
                page.goto(base + "/", wait_until="networkidle")
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator("#startup-overlay").count() == 0。
                assert page.locator("#startup-overlay").count() == 0
                # 【逐行批注】 输出当前运行结果：print("NORMAL_HOME overlay=NONE")。
                print("NORMAL_HOME overlay=NONE")
                # 【逐行批注】 计算右侧结果并赋给 page.goto(base + "/startup", wait_until，更新脚本状态。
                page.goto(base + "/startup", wait_until="networkidle")
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator("#startup-overlay").is_visible()。
                assert page.locator("#startup-overlay").is_visible()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator(".scan-art").count() == 1。
                assert page.locator(".scan-art").count() == 1
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator(".aurora-field, .orbit-art, .particle-art, .panel-art").cou…。
                assert page.locator(".aurora-field, .orbit-art, .particle-art, .panel-art").count() == 0
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator("#goal-form").count() == 1。
                assert page.locator("#goal-form").count() == 1
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator("body").get_attribute("data-startup-phase") == "intro"。
                assert page.locator("body").get_attribute("data-startup-phase") == "intro"
                # 【逐行批注】 执行当前 Python 语句：page.wait_for_timeout(800)。
                page.wait_for_timeout(800)
                # 【逐行批注】 计算右侧结果并赋给 page.screenshot(path，更新脚本状态。
                page.screenshot(path=str(SHOTS / "scan-intro.png"))
                # 【逐行批注】 执行当前 Python 语句：page.wait_for_function("document.body.dataset.startupPhase === 'outro'", timeout=4500)。
                page.wait_for_function("document.body.dataset.startupPhase === 'outro'", timeout=4500)
                # 【逐行批注】 计算右侧结果并赋给 page.screenshot(path，更新脚本状态。
                page.screenshot(path=str(SHOTS / "scan-transition.png"))
                # 【逐行批注】 执行当前 Python 语句：page.wait_for_function("document.body.dataset.startupPhase === 'home'", timeout=4500)。
                page.wait_for_function("document.body.dataset.startupPhase === 'home'", timeout=4500)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator("#startup-overlay").count() == 0。
                assert page.locator("#startup-overlay").count() == 0
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.evaluate("location.pathname") == "/"。
                assert page.evaluate("location.pathname") == "/"
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator("#goal-form").is_visible()。
                assert page.locator("#goal-form").is_visible()
                # 【逐行批注】 输出当前运行结果：print("STARTUP_SCAN intro=PASS transition=PASS real_home=PASS")。
                print("STARTUP_SCAN intro=PASS transition=PASS real_home=PASS")
                # 【逐行批注】 计算右侧结果并赋给 reduced，更新脚本状态。
                reduced = browser.new_page(viewport={"width": 390, "height": 844}, reduced_motion="reduce")
                # 【逐行批注】 计算右侧结果并赋给 reduced.goto(base + "/startup", wait_until，更新脚本状态。
                reduced.goto(base + "/startup", wait_until="networkidle")
                # 【逐行批注】 执行当前 Python 语句：reduced.wait_for_timeout(500)。
                reduced.wait_for_timeout(500)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert reduced.locator("#startup-overlay").count() == 0。
                assert reduced.locator("#startup-overlay").count() == 0
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert reduced.evaluate("document.documentElement.scrollWidth <= innerWidth")。
                assert reduced.evaluate("document.documentElement.scrollWidth <= innerWidth")
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert reduced.evaluate("location.pathname") == "/"。
                assert reduced.evaluate("location.pathname") == "/"
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, errors。
                assert not errors, errors
                # 【逐行批注】 输出当前运行结果：print("REDUCED_MOTION skip=PASS MOBILE_OVERFLOW=NONE JS_HTTP_ERRORS=0")。
                print("REDUCED_MOTION skip=PASS MOBILE_OVERFLOW=NONE JS_HTTP_ERRORS=0")
                # 【逐行批注】 执行当前 Python 语句：browser.close()。
                browser.close()
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 计算右侧结果并赋给 urlopen(Request(base + "/internal/shutdown",…，更新脚本状态。
                urlopen(Request(base + "/internal/shutdown", method="POST",
                                # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                                headers={"X-Gangyi-Control-Token": "startup-test-token"}), timeout=2)
            # 【逐行批注】 捕获并处理异常：except Exception:。
            except Exception:
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 计算右侧结果并赋给 service.wait(timeout，更新脚本状态。
                service.wait(timeout=5)
            # 【逐行批注】 捕获并处理异常：except subprocess.TimeoutExpired:。
            except subprocess.TimeoutExpired:
                # 【逐行批注】 执行当前 Python 语句：service.terminate()。
                service.terminate()
                # 【逐行批注】 计算右侧结果并赋给 service.wait(timeout，更新脚本状态。
                service.wait(timeout=5)


# 【逐行批注】 判断条件 __name__ == "__main__":，成立时执行对应分支。
if __name__ == "__main__":
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()

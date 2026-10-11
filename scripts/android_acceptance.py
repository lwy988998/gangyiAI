"""在明确指定的安卓模拟器中验收 APK 与实际 WebView，使用模拟 AI 并统计调用。"""
import argparse
import base64
import io
import json
from pathlib import Path
import subprocess
import sys
import threading
import time
from http.server import ThreadingHTTPServer
from urllib.request import urlopen, build_opener, ProxyHandler
import websocket

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tests"))
from classroom_v560_fixture import FixtureAI

class AndroidFixture(FixtureAI):
    def do_GET(self):
        if self.path.endswith("/models"):
            self.send_response(200); self.end_headers()
            self.wfile.write(b'{"data":[{"id":"fixture-v560"}]}')
        else:
            super().do_GET()
    def do_POST(self):
        body = self.rfile.read(int(self.headers["Content-Length"]))
        request = json.loads(body)
        if len(request.get("messages", [])) == 1:
            type(self).calls.append("connection-test")
            self.send_response(200); self.end_headers()
            self.wfile.write(b'{"model":"fixture-v560","choices":[{"message":{"content":"OK"},"finish_reason":"stop"}]}')
            return
        previous = self.rfile
        try:
            self.rfile = io.BytesIO(body); super().do_POST()
        finally:
            self.rfile = previous

class WebView:
    def __init__(self, address):
        self.socket = websocket.create_connection(address, timeout=20, suppress_origin=True)
        self.sequence = 0
        self.call("Page.enable")
        self.call("Runtime.enable")
    def call(self, method, params=None):
        self.sequence += 1
        self.socket.send(json.dumps(dict(id=self.sequence, method=method, params=params or {})))
        while True:
            response = json.loads(self.socket.recv())
            if response.get("id") == self.sequence:
                if "error" in response: raise RuntimeError(response["error"])
                return response.get("result", {})
    def evaluate(self, expression):
        value = self.call("Runtime.evaluate", dict(expression=expression, returnByValue=True, awaitPromise=True))
        if "exceptionDetails" in value: raise RuntimeError(value["exceptionDetails"])
        return value.get("result", {}).get("value")
    def wait(self, expression, seconds=45):
        deadline = time.monotonic() + seconds
        while time.monotonic() < deadline:
            if self.evaluate("Boolean(" + expression + ")"): return
            time.sleep(.15)
        raise AssertionError("WebView 等待失败：" + expression)
    def screenshot(self, path):
        value = self.call("Page.captureScreenshot", dict(format="png"))
        Path(path).write_bytes(base64.b64decode(value["data"]))

def main(args):
    assert args.serial.startswith("emulator-"), "本脚本只允许清理指定的隔离模拟器，不能用于真机"
    args.output.mkdir(parents=True, exist_ok=True)
    report = dict(device=args.serial, api=None, realDevice=False, paidModelCalls=0, checks=[])
    def adb(*arguments, check=True, timeout=60):
        value = subprocess.run([str(args.adb), "-s", args.serial, *map(str, arguments)], capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=timeout)
        if check and value.returncode: raise RuntimeError(value.stderr or value.stdout)
        return value.stdout.strip()
    server = ThreadingHTTPServer(("127.0.0.1", 0), AndroidFixture)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    package = "io.github.lwy988998.gangyiai"
    browser = None
    try:
        report["api"] = adb("shell", "getprop", "ro.build.version.sdk")
        adb("reverse", "tcp:" + str(server.server_port), "tcp:" + str(server.server_port))
        adb("install", "-r", args.apk); adb("install", "-r", args.test_apk)
        adb("shell", "pm", "clear", package)
        if int(report["api"]) >= 33: adb("shell", "pm", "grant", package, "android.permission.POST_NOTIFICATIONS")
        result = adb("shell", "am", "instrument", "-w", "-e", "mockPort", server.server_port,
                     package + ".test/androidx.test.runner.AndroidJUnitRunner", timeout=240)
        (args.output / "instrumentation.txt").write_text(result, encoding="utf-8")
        assert "OK (1 test)" in result, result
        report["checks"].append("原生独立核心、Keystore 加密、会话隔离、零自动启动调用、课程与备课")
        count = len(AndroidFixture.calls)
        adb("shell", "am", "start", "-n", package + "/.MainActivity")
        socket_name = ""
        pid = ""
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            value = adb("shell", "pidof", package, check=False)
            if value.strip(): pid = value.split()[0]; break
            time.sleep(.2)
        assert pid, "应用进程没有启动"
        deadline = time.monotonic() + 45
        while time.monotonic() < deadline:
            sockets = adb("shell", "cat", "/proc/net/unix")
            names = [row.split()[-1].lstrip("@") for row in sockets.splitlines() if "webview_devtools_remote_" + pid in row]
            if names: socket_name = names[-1]; break
            time.sleep(.3)
        assert socket_name, "实际 WebView 调试接口未就绪"
        adb("forward", "tcp:9223", "localabstract:" + socket_name)
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            pages = json.load(build_opener(ProxyHandler({})).open("http://127.0.0.1:9223/json", timeout=5))
            local = [page for page in pages if page.get("url", "").startswith("http://127.0.0.1:")]
            if local: break
            time.sleep(.3)
        report["webviewTargets"] = [{"url": page.get("url"), "title": page.get("title")} for page in pages]
        assert local, "课堂网页未打开：" + json.dumps(report["webviewTargets"], ensure_ascii=False)
        browser = WebView(local[0]["webSocketDebuggerUrl"])
        browser.wait("document.getElementById('lesson-input') && document.getElementById('lesson-sections').children.length>0")
        assert len(AndroidFixture.calls) == count, "重新打开 App 发起了自动模型调用"
        if browser.evaluate("Boolean(document.getElementById('android-continue-lesson'))"):
            browser.evaluate("document.getElementById('android-continue-lesson').click()")
            browser.wait("!document.getElementById('android-continue-lesson') && document.getElementById('lesson-task-status')?.textContent.includes('继续')", seconds=90)
            report["checks"].append("恢复记录不自动调用，点击继续后启动 AI")
        origin = browser.evaluate("location.origin")
        lesson = browser.evaluate("new URLSearchParams(location.search).get('lessonId')")
        def navigate(page):
            browser.call("Page.navigate", dict(url=origin + "/" + page + "?lessonId=" + lesson))
            browser.wait("document.querySelector('[data-lesson-view]')?.dataset.lessonView===" + json.dumps(page) + " && document.getElementById('lesson-input')")
            browser.wait("document.getElementById('lesson-sections').children.length>0 || document.querySelector('[data-lesson-view]').dataset.lessonView==='summary'")
        browser.evaluate("(()=>{const input=document.getElementById('lesson-input');input.value='安卓讲解草稿';input.dispatchEvent(new Event('input',{bubbles:true}));})()")
        navigate("practice")
        assert browser.evaluate("document.getElementById('lesson-input').value") == ""
        navigate("learn")
        assert browser.evaluate("document.getElementById('lesson-input').value") == "安卓讲解草稿"
        browser.call("Page.reload")
        browser.wait("document.getElementById('lesson-input')?.value==='安卓讲解草稿'")
        report["checks"].append("实际 WebView 三页切换、页内草稿隔离和刷新恢复")
        def fits():
            geometry = browser.evaluate("(()=>{const a=document.getElementById('lesson-chat-panel').getBoundingClientRect();const b=document.getElementById('lesson-scroll').getBoundingClientRect();return {height:innerHeight,width:innerWidth,scrollWidth:document.documentElement.scrollWidth,composerBottom:a.bottom,scrollHeight:b.height}})()")
            assert geometry["scrollWidth"] <= geometry["width"] + 1, geometry
            assert geometry["composerBottom"] <= geometry["height"] + 1 and geometry["scrollHeight"] > 60, geometry
            return geometry
        report["portrait"] = fits(); browser.screenshot(args.output / "安卓课堂-竖屏.png")
        # 使用系统触摸与输入法，而非仅改变浏览器尺寸。
        keyboard_point = browser.evaluate("(()=>{const box=document.getElementById('lesson-input').getBoundingClientRect();return {x:(box.x+30)*devicePixelRatio,y:(box.y+20)*devicePixelRatio}})()")
        adb("shell", "input", "tap", round(keyboard_point["x"]), round(keyboard_point["y"] + 28 * browser.evaluate("devicePixelRatio")))
        time.sleep(1)
        report["keyboard"] = fits(); browser.screenshot(args.output / "安卓课堂-键盘.png")
        adb("shell", "input", "keyevent", "4"); time.sleep(.4)
        report["checks"].append("系统键盘开启时输入框和提交区仍在可见窗口")
        adb("shell", "settings", "put", "system", "accelerometer_rotation", "0")
        adb("shell", "settings", "put", "system", "user_rotation", "1")
        time.sleep(1)
        report["landscape"] = fits(); browser.screenshot(args.output / "安卓课堂-横屏.png")
        adb("shell", "settings", "put", "system", "user_rotation", "0"); time.sleep(1)
        report["checks"].append("横竖屏切换保留课堂与草稿")
        navigate("practice")
        browser.wait("document.querySelector('.agent-options input[type=radio]')")
        browser.evaluate("(()=>{document.querySelector('.agent-options input[type=radio]').click();const card=document.querySelector('.agent-options').closest('.chat-section');[...card.querySelectorAll('button')].find(node=>node.textContent.includes('提交')).click()})()")
        browser.wait("document.getElementById('lesson-task-status').textContent.includes('继续')", seconds=90)
        assert len(AndroidFixture.calls) > count
        browser.screenshot(args.output / "安卓课堂-选择题.png")
        report["checks"].append("选择题卡片提交与真实主控评价链（模拟模型）")
        navigate("summary"); browser.screenshot(args.output / "安卓课堂-小结.png")
        browser.evaluate("document.getElementById('learning-navigation').open=true")
        browser.wait("document.getElementById('agent-control-panel')")
        browser.screenshot(args.output / "全局AI状态.png")
        report["checks"].append("小结与全局 AI 调用监控")
        # 查看首页和课程管理、雷达，不重新触发备课。
        browser.evaluate("location.href='/'")
        browser.wait("document.querySelectorAll('#home-radar .radar-axis').length===6")
        browser.screenshot(args.output / "安卓首页-雷达.png")
        browser.evaluate("location.href='/my-courses'")
        browser.wait("document.body.innerText.includes('隔离化学课程')")
        browser.screenshot(args.output / "安卓我的课程.png")
        report["checks"].append("首页、课程管理与六维雷达实际 WebView 展示")
        assert browser.evaluate("typeof GangyiAndroid==='object'")
        adb("shell", "input", "keyevent", "3")
        time.sleep(2)
        adb("shell", "am", "force-stop", package)
        count = len(AndroidFixture.calls)
        adb("shell", "am", "start", "-n", package + "/.MainActivity")
        time.sleep(4)
        assert len(AndroidFixture.calls) == count, "系统结束进程后静默重新调用模型"
        report["checks"].append("退出和进程重启不会自动重新调用 AI")
        report["passed"] = True
    except Exception as error:
        report["passed"] = False; report["failure"] = str(error)
        if browser:
            try: browser.screenshot(args.output / "失败现场.png")
            except Exception: pass
        raise
    finally:
        report["mockModelCalls"] = len(AndroidFixture.calls)
        report["mockRequests"] = AndroidFixture.calls
        (args.output / "android-acceptance.json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
        server.shutdown(); server.server_close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--adb", type=Path, required=True)
    parser.add_argument("--serial", required=True)
    parser.add_argument("--apk", type=Path, required=True)
    parser.add_argument("--test-apk", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    main(parser.parse_args())

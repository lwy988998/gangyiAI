from pathlib import Path
from tempfile import TemporaryDirectory
from time import monotonic, sleep
from urllib.request import Request, urlopen
import os
import socket
import subprocess

from playwright.sync_api import sync_playwright


ROOT = Path(__file__).resolve().parents[1]
SERVICE = Path(os.environ.get("GANGYI_SERVICE_EXE", str(ROOT / "build-ascii" / "gangyiAI.exe"))).resolve()
SHOTS = ROOT / "verification" / "startup-scan-v410" / "browser-shots"


def free_port():
    with socket.socket() as connection:
        connection.bind(("127.0.0.1", 0))
        return connection.getsockname()[1]


def main():
    SHOTS.mkdir(parents=True, exist_ok=True)
    port = free_port()
    base = f"http://127.0.0.1:{port}"
    with TemporaryDirectory(prefix="gangyi-startup-") as temporary:
        environment = os.environ.copy()
        environment.update(HOST="127.0.0.1", PORT=str(port), DATABASE_PATH=str(Path(temporary) / "test.db"),
                           AI_API_KEY="", LOCAL_CONTROL_TOKEN="startup-test-token")
        service = subprocess.Popen([str(SERVICE)], cwd=SERVICE.parent, env=environment,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                   creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        try:
            deadline = monotonic() + 20
            while monotonic() < deadline:
                try:
                    if urlopen(base + "/health", timeout=1).status == 200:
                        break
                except Exception:
                    sleep(.15)
            else:
                raise AssertionError("本地服务未就绪")
            with sync_playwright() as playwright:
                browser = playwright.chromium.launch(headless=True)
                page = browser.new_page(viewport={"width": 1120, "height": 760})
                errors = []
                page.on("pageerror", lambda error: errors.append(str(error)))
                page.on("response", lambda response: errors.append(f"HTTP {response.status}: {response.url}")
                        if response.status >= 400 else None)
                page.goto(base + "/", wait_until="networkidle")
                assert page.locator("#startup-overlay").count() == 0
                print("NORMAL_HOME overlay=NONE")
                page.goto(base + "/startup", wait_until="networkidle")
                assert page.locator("#startup-overlay").is_visible()
                assert page.locator(".scan-art").count() == 1
                assert page.locator(".aurora-field, .orbit-art, .particle-art, .panel-art").count() == 0
                assert page.locator("#goal-form").count() == 1
                assert page.locator("body").get_attribute("data-startup-phase") == "intro"
                page.wait_for_timeout(800)
                page.screenshot(path=str(SHOTS / "scan-intro.png"))
                page.wait_for_function("document.body.dataset.startupPhase === 'outro'", timeout=4500)
                page.screenshot(path=str(SHOTS / "scan-transition.png"))
                page.wait_for_function("document.body.dataset.startupPhase === 'home'", timeout=4500)
                assert page.locator("#startup-overlay").count() == 0
                assert page.evaluate("location.pathname") == "/"
                assert page.locator("#goal-form").is_visible()
                print("STARTUP_SCAN intro=PASS transition=PASS real_home=PASS")
                reduced = browser.new_page(viewport={"width": 390, "height": 844}, reduced_motion="reduce")
                reduced.goto(base + "/startup", wait_until="networkidle")
                reduced.wait_for_timeout(500)
                assert reduced.locator("#startup-overlay").count() == 0
                assert reduced.evaluate("document.documentElement.scrollWidth <= innerWidth")
                assert reduced.evaluate("location.pathname") == "/"
                assert not errors, errors
                print("REDUCED_MOTION skip=PASS MOBILE_OVERFLOW=NONE JS_HTTP_ERRORS=0")
                browser.close()
        finally:
            try:
                urlopen(Request(base + "/internal/shutdown", method="POST",
                                headers={"X-Gangyi-Control-Token": "startup-test-token"}), timeout=2)
            except Exception:
                pass
            try:
                service.wait(timeout=5)
            except subprocess.TimeoutExpired:
                service.terminate()
                service.wait(timeout=5)


if __name__ == "__main__":
    main()

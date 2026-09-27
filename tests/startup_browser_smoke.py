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
SHOTS = ROOT / "verification" / "startup-random-v034" / "browser-shots"


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
                for invalid in ("99", "2x", "-1"):
                    page.goto(base + f"/startup?variant={invalid}", wait_until="networkidle")
                    assert page.locator("body").get_attribute("data-startup-variant") == "aurora"
                print("INVALID_VARIANT fallback=aurora")
                for variant in range(5):
                    page.goto(base + f"/startup?variant={variant}", wait_until="networkidle")
                    assert page.locator("#startup-overlay").is_visible()
                    assert page.locator(".startup-overlay-index, .startup-overlay-brand p").count() == 0
                    assert not any(name in page.locator("#startup-overlay").inner_text() for name in
                                   ("极光折射", "精密轨道", "点阵成形", "光栅扫描", "层叠空间",
                                    "AURORA / REFRACTION", "ORBIT / ALIGNMENT", "PARTICLE / ASSEMBLY",
                                    "SCAN / REVEAL", "PANELS / ARRIVAL"))
                    assert page.locator("#goal-form").count() == 1
                    assert page.locator("body").get_attribute("data-startup-phase") == "intro"
                    page.wait_for_timeout(1700)
                    page.screenshot(path=str(SHOTS / f"variant-{variant}-intro.png"))
                    page.wait_for_timeout(2050)
                    assert page.locator("body").get_attribute("data-startup-phase") == "outro"
                    page.screenshot(path=str(SHOTS / f"variant-{variant}-transition.png"))
                    page.wait_for_timeout(1500)
                    assert page.locator("#startup-overlay").count() == 0
                    assert page.locator("body").get_attribute("data-startup-phase") == "home"
                    assert page.evaluate("location.pathname") == "/"
                    assert page.locator("#goal-form").is_visible()
                    print(f"VARIANT_{variant} name=HIDDEN intro=PASS transition=PASS real_home=PASS")
                reduced = browser.new_page(viewport={"width": 390, "height": 844}, reduced_motion="reduce")
                reduced.goto(base + "/startup?variant=4", wait_until="networkidle")
                reduced.wait_for_timeout(500)
                assert reduced.locator("#startup-overlay").is_visible()
                assert reduced.locator("body").get_attribute("data-startup-phase") == "intro"
                assert reduced.evaluate("document.documentElement.scrollWidth <= innerWidth")
                reduced.wait_for_timeout(4800)
                assert reduced.locator("#startup-overlay").count() == 0
                assert reduced.evaluate("location.pathname") == "/"
                assert not errors, errors
                print("REDUCED_MOTION full_play=PASS MOBILE_OVERFLOW=NONE JS_HTTP_ERRORS=0")
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

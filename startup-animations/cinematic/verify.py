from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Thread
import argparse

from playwright.sync_api import sync_playwright


ROOT = Path(__file__).resolve().parents[2]
BASE = Path(__file__).resolve().parent
FILES = ["01-aurora.html", "02-orbit.html", "03-particles.html", "04-scan.html", "05-panels.html"]
OLD = ["startup-06-liquid-glass.html", "startup-07-particle-lattice.html", "startup-08-gooey-rings.html", "startup-09-circuit-awakening.html", "startup-10-prism-gateway.html"]


class QuietHandler(SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        pass


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--baseline", action="store_true")
    args = parser.parse_args()
    server = ThreadingHTTPServer(("127.0.0.1", 0), partial(QuietHandler, directory=str(ROOT)))
    Thread(target=server.serve_forever, daemon=True).start()
    base = f"http://127.0.0.1:{server.server_port}/startup-animations/"
    try:
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True)
            page = browser.new_page(viewport={"width": 1440, "height": 900})
            errors = []
            page.on("pageerror", lambda error: errors.append(str(error)))
            if args.baseline:
                for name in OLD:
                    response = page.goto(base + name, wait_until="domcontentloaded")
                    assert response and response.status == 200
                    assert page.locator(".scene").count() == 1
                print("BASELINE old_startup_pages=5/5 status=PASS")
            else:
                page.goto(base + "cinematic/gallery.html", wait_until="networkidle")
                assert page.locator(".card").count() == 5
                for index, name in enumerate(FILES, 1):
                    missing = []
                    page.on("response", lambda response: missing.append(response.url) if response.status >= 400 else None)
                    response = page.goto(base + "cinematic/" + name, wait_until="networkidle")
                    assert response and response.status == 200
                    assert page.locator(".demo").get_attribute("data-phase") == "intro"
                    assert page.locator(".splash-index, .splash-brand p, .preview-caption").count() == 0
                    assert not any(label in page.locator("body").inner_text() for label in
                                   ("极光折射", "精密轨道", "点阵成形", "光栅扫描", "层叠空间"))
                    assert page.locator(".home").get_attribute("inert") is not None
                    page.wait_for_timeout(2100)
                    assert page.evaluate("document.getAnimations({subtree:true}).length") > 0
                    page.screenshot(path=str(BASE / f"shot-{index:02}-intro.png"))
                    page.wait_for_timeout(1600)
                    assert page.locator(".demo").get_attribute("data-phase") == "outro"
                    page.screenshot(path=str(BASE / f"shot-{index:02}-transition.png"))
                    page.wait_for_timeout(1500)
                    assert page.locator(".demo").get_attribute("data-phase") == "home"
                    assert page.locator(".home").get_attribute("inert") is None
                    assert page.locator(".splash").is_hidden()
                    page.screenshot(path=str(BASE / f"shot-{index:02}-home.png"))
                    page.get_by_role("button", name="↻ 重播").click()
                    assert page.locator(".demo").get_attribute("data-phase") == "intro"
                    page.get_by_role("button", name="跳过动画 →").click()
                    assert page.locator(".demo").get_attribute("data-phase") == "home"
                    assert not missing, f"{name}: HTTP 错误 {missing}"
                    print(f"OPTION_{index} name=HIDDEN intro=PASS transition=PASS home=PASS replay=PASS skip=PASS")
                mobile = browser.new_page(viewport={"width": 390, "height": 844}, reduced_motion="reduce")
                mobile.goto(base + "cinematic/01-aurora.html", wait_until="networkidle")
                mobile.wait_for_timeout(350)
                assert mobile.locator(".demo").get_attribute("data-phase") == "home"
                assert mobile.evaluate("document.documentElement.scrollWidth <= innerWidth")
                mobile.close()
                assert not errors, f"JavaScript 错误：{errors}"
                print("GALLERY=5 REDUCED_MOTION=PASS MOBILE_OVERFLOW=NONE JS_ERRORS=0")
            browser.close()
    finally:
        server.shutdown()
        server.server_close()


if __name__ == "__main__":
    main()

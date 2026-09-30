"""检查课堂页面交互和主要页面文字对比度。"""

import json
import os
import sqlite3
import subprocess
import tempfile
import time
from contextlib import closing
from pathlib import Path
from urllib.request import urlopen

from playwright.sync_api import sync_playwright

from classroom_api_smoke import MockAI, free_port
from http.server import ThreadingHTTPServer
from threading import Thread


AUDIT = """() => {
  const rgb = text => (text.match(/[\\d.]+/g) || []).map(Number);
  const luminance = c => {
    const x = c.slice(0, 3).map(v => v / 255).map(v => v <= .04045 ? v / 12.92 : ((v + .055) / 1.055) ** 2.4);
    return x[0] * .2126 + x[1] * .7152 + x[2] * .0722;
  };
  const failures = [];
  for (const element of document.body.querySelectorAll('*')) {
    if (!Array.from(element.childNodes).some(node => node.nodeType === 3 && node.textContent.trim())) continue;
    const style = getComputedStyle(element), rect = element.getBoundingClientRect();
    if (style.display === 'none' || style.visibility === 'hidden' || !rect.width || !rect.height) continue;
    let parent = element, bg = [6, 9, 13, 1];
    while (parent) {
      const candidate = rgb(getComputedStyle(parent).backgroundColor);
      if (candidate.length >= 3 && (candidate[3] === undefined || candidate[3] >= .98)) { bg = candidate; break; }
      parent = parent.parentElement;
    }
    const fg = rgb(style.color);
    if (fg.length < 3) continue;
    const a = luminance(fg), b = luminance(bg), ratio = (Math.max(a, b) + .05) / (Math.min(a, b) + .05);
    const font = parseFloat(style.fontSize), large = font >= 24 || (font >= 18.66 && Number(style.fontWeight) >= 700);
    if (ratio < (large ? 3 : 4.5)) failures.push({text: element.textContent.trim().slice(0, 38),
      ratio: Number(ratio.toFixed(2)), fg: style.color, bg: getComputedStyle(parent || document.body).backgroundColor,
      tag: element.tagName, className: String(element.className).slice(0, 55)});
  }
  return failures.slice(0, 30);
}"""


def main(executable):
    with tempfile.TemporaryDirectory(prefix="classroom-browser-") as directory:
        database = Path(directory) / "ui.db"
        ai = ThreadingHTTPServer(("127.0.0.1", free_port()), MockAI)
        worker = Thread(target=ai.serve_forever, daemon=True)
        worker.start()
        port = free_port()
        env = dict(os.environ, HOST="127.0.0.1", PORT=str(port), DATABASE_PATH=str(database),
                   AI_BASE_URL=f"http://127.0.0.1:{ai.server_port}/v1", AI_API_KEY="mock-key",
                   LOCAL_CONTROL_TOKEN="ui-test")
        service = subprocess.Popen([str(executable)], cwd=executable.parent, env=env,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                   creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        base = f"http://127.0.0.1:{port}"
        try:
            for _ in range(100):
                try:
                    if urlopen(base + "/health", timeout=1).status == 200:
                        break
                except Exception:
                    time.sleep(.1)
            else:
                raise AssertionError("页面服务未就绪")
            topics = ["函数单调性", "函数图像"]
            plan = {"title": "函数课", "summary": "学习函数性质", "duration": "一周", "outcome": "掌握函数",
                    "generation": {"source": "ai", "promptVersion": "ai-plan-v1"},
                    "courseStructure": [{"stage": "函数", "topics": topics}],
                    "roadmap": [{"name": "函数", "topics": topics, "steps": [{"title": "理解定义"}]}]}
            quiz = [{"question": f"测验{i}", "options": ["正确", "错误", "其他", "不确定"], "answerIndex": 0} for i in range(3)]
            blocks = {"overview": {"title": "函数单调性", "summary": "理解区间上的变化", "keyConcepts": topics},
                      "steps": {"lessonSteps": [{"title": "定义", "explanation": "观察区间内函数值的变化。", "example": "y=x", "action": "判断", "check": "是否单调"}]},
                      "examples": {"examples": [{"title": "例题", "content": "判断函数", "solution": "递增"}]},
                      "practice": {"practice": [{"title": "练习", "task": "判断区间", "check": "核对"}]},
                      "quiz": {"quiz": quiz},
                      "assessment": {"checkpoint": ["能判断单调性"], "commonMistakes": ["忽略区间"], "resourceSummary": "复习定义"}}
            content = {"promptVersion": "ai-block-v1", "blocks": blocks,
                       "generations": {name: {"source": "ai", "promptVersion": "ai-block-v1", "model": "mock"} for name in blocks},
                       "references": []}
            with closing(sqlite3.connect(database)) as db, db:
                db.execute("INSERT INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?)",
                           ("c1", "anon", None, "函数", "deep", "函数课", "学习函数性质", "ai", "active", "2026-09-28", "2026-09-28"))
                db.execute("INSERT INTO CourseSnapshot VALUES(?,?,?,?,?)",
                           ("s1", "c1", 1, json.dumps(plan, ensure_ascii=False), "2026-09-28"))
                db.execute("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                           ("l1", "c1", "anon", "函数", "deep", 1, "函数", 1, "函数单调性", "函数单调性",
                            "理解区间上的变化", "", json.dumps(content, ensure_ascii=False), "[]", 0, "ai"))
                for kind in ("diagnostic", "interaction"):
                    for index in range(3):
                        item = {"question": f"{kind}{index}", "type": "open" if kind == "interaction" and index == 1 else "choice",
                                "options": ["正确", "错误", "其他", "不确定"], "answerIndex": 0,
                                "rubric": "说明定义、区间和判断过程", "explanation": "先检查区间。", "status": "pending"}
                        db.execute("INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)",
                                   (f"classroom:c1:1:1:{kind}:{index}", "c1", 1, 1, kind,
                                    json.dumps(item, ensure_ascii=False), "2026-09-28"))
                review = {"title": "函数单调性到期复习", "due": "2026-09-28", "status": "pending",
                          "phaseIndex": 1, "topicIndex": 1, "day": 1}
                db.execute("INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)",
                           ("classroom:c1:1:1:review:1", "c1", 1, 1, "review",
                            json.dumps(review, ensure_ascii=False), "2026-09-28"))
            with sync_playwright() as playwright:
                browser = playwright.chromium.launch(headless=True)
                page = browser.new_page(viewport={"width": 1280, "height": 900})
                errors = []
                page.on("pageerror", lambda error: errors.append(str(error)))
                pages = ["/", "/startup", "/ask", "/my-courses", "/plan?courseId=c1",
                         "/phase?courseId=c1&phaseIndex=1",
                         "/learn?courseId=c1&phaseIndex=1&topicIndex=1"]
                audit = {}
                for path in pages:
                    page.goto(base + path, wait_until="networkidle")
                    if path == "/":
                        assert page.locator("#home-availability input[type=checkbox]:checked").count() == 3
                        monday = page.locator("#home-availability input[data-weekday='1']")
                        tuesday = page.locator("#home-availability input[data-weekday='2']")
                        monday.uncheck()
                        tuesday.check()
                        tuesday.locator("xpath=..//input[@type='number']").fill("45")
                        tuesday.dispatch_event("change")
                        page.locator("#due-reviews a").get_by_text("函数单调性", exact=False).wait_for()
                        page.locator("#goal-submit").focus()
                        assert page.locator("#goal-submit").evaluate("el => parseFloat(getComputedStyle(el).outlineWidth) >= 2")
                        page.locator("#goal-submit").evaluate("el => el.disabled = true")
                        assert not page.evaluate(AUDIT), "首页禁用态对比度不足"
                    if path == "/ask":
                        page.locator("#ask-question").fill("导师界面上下文")
                        page.locator("#ask-submit").click()
                        page.locator("#ask-messages").get_by_text("片段一片段二", exact=True).wait_for()
                        page.locator("#ask-submit:not([disabled])").wait_for()
                        page.locator("#ask-question").fill("第二问")
                        page.locator("#ask-submit").click()
                        page.locator("#ask-messages").get_by_text("片段一片段二", exact=True).nth(1).wait_for()
                        page.locator("#ask-submit:not([disabled])").wait_for()
                        assert "导师界面上下文" in json.dumps(MockAI.stream_messages, ensure_ascii=False)
                        page.locator("#ask-question").fill("停止界面测试")
                        page.locator("#ask-submit").click()
                        page.locator("#ask-stop").click()
                        page.locator("#ask-messages").get_by_text("已停止", exact=False).wait_for()
                        page.locator("#ask-submit:not([disabled])").wait_for()
                        page.reload(wait_until="networkidle")
                        assert "停止界面测试" not in page.locator("#ask-messages").inner_text()
                    if path.startswith("/plan"):
                        page.locator("#weekly-entries .weekly-row").first.wait_for()
                        assert page.locator("#weekly-availability input[type=checkbox]:checked").count() == 3
                        assert not page.locator("#weekly-availability input[data-weekday='1']").is_checked()
                        assert page.locator("#weekly-availability input[data-weekday='2']").is_checked()
                        assert page.locator("#weekly-availability input[data-weekday='2']").locator(
                            "xpath=..//input[@type='number']").input_value() == "45"
                        first_link = page.locator(".next-action a.primary-action").first.get_attribute("href")
                        assert "phaseIndex=1&topicIndex=1" in first_link, first_link
                        first_title = page.locator("#weekly-entries .weekly-row strong").first.inner_text()
                        page.locator("#weekly-entries .weekly-row [data-down]").first.click()
                        assert page.locator("#weekly-entries .weekly-row strong").first.inner_text() != first_title
                        page.locator("#weekly-entries .weekly-row input[type=date]").first.fill("2026-10-01")
                        minutes = page.locator("#weekly-entries .weekly-row input[type=number]").first
                        minutes.fill("25")
                        page.locator("#weekly-save").click()
                        page.locator("#weekly-message").get_by_text("周计划已保存").wait_for()
                        page.locator("#weekly-replan").click()
                        page.locator("#weekly-confirm").get_by_text("重排会覆盖手动安排").wait_for()
                        page.locator("#weekly-cancel").click()
                        assert page.locator("#weekly-confirm").is_hidden()
                        page.locator(".primary-action").first.hover()
                        assert not page.evaluate(AUDIT), "计划页悬停态对比度不足"
                    if path.startswith("/learn"):
                        page.locator("#diagnostic-questions .classroom-question").first.wait_for()
                        assert page.locator("#diagnostic-questions .classroom-question").count() == 2
                        page.locator("#learn-content").wait_for(state="visible")
                        page.evaluate("""() => {
                          const node = document.querySelector('#learn-content-summary').firstChild;
                          const range = document.createRange(); range.selectNodeContents(node);
                          const selection = window.getSelection(); selection.removeAllRanges(); selection.addRange(range);
                          document.dispatchEvent(new MouseEvent('mouseup'));
                        }""")
                        page.locator("#ask-selection").wait_for(state="visible")
                        page.locator("#ask-selection").click()
                        assert page.locator("#learning-chat-panel").is_visible()
                        page.locator("#learning-chat-form").evaluate("el => el.requestSubmit()")
                        page.locator("#learning-chat-messages").get_by_text("片段一片段二", exact=True).wait_for()
                        assert "导师界面上下文" not in page.locator("#learning-chat-messages").inner_text()
                        assert page.locator("#learning-chat-stop").count() == 1
                        page.locator("#diagnostic-skip").click()
                        page.locator("#classroom-branch").wait_for(state="visible")
                        assert "完整课堂" in page.locator("#classroom-branch").inner_text()
                        page.locator("#interaction-questions .classroom-question").first.wait_for()
                        page.locator("#interaction-questions .classroom-question").first.locator("input[value='1']").check()
                        page.locator("#interaction-questions .classroom-question").first.locator("[data-submit]").click()
                        page.locator("#interaction-questions .classroom-question").first.get_by_text("追问", exact=False).wait_for()
                    audit[path] = page.evaluate(AUDIT)
                assert not errors, errors
                assert all(not failures for failures in audit.values()), audit
                def reset_diagnostic():
                    with closing(sqlite3.connect(database)) as db, db:
                        db.execute("DELETE FROM ClassroomActivity WHERE id='classroom:c1:1:1:state'")
                        for index in range(3):
                            key = f"classroom:c1:1:1:diagnostic:{index}"
                            item = json.loads(db.execute("SELECT payload FROM ClassroomActivity WHERE id=?", (key,)).fetchone()[0])
                            item.update(status="pending")
                            for field in ("answer", "correct", "credible", "feedback", "followUp", "hintLevel"):
                                item.pop(field, None)
                            db.execute("UPDATE ClassroomActivity SET payload=? WHERE id=?", (json.dumps(item, ensure_ascii=False), key))
                reset_diagnostic()
                page.goto(base + "/learn?courseId=c1&phaseIndex=1&topicIndex=1", wait_until="networkidle")
                for index in range(2):
                    row = page.locator("#diagnostic-questions .classroom-question").nth(index)
                    row.locator("input[value='0']").check()
                    row.locator("[data-submit]").click()
                    row.locator(".classroom-feedback").get_by_text("回答正确", exact=False).wait_for()
                page.locator("#classroom-branch").get_by_text("精简已熟悉内容").wait_for()
                page.reload(wait_until="networkidle")
                assert page.locator("#diagnostic-questions .classroom-question").count() == 2
                reset_diagnostic()
                page.goto(base + "/learn?courseId=c1&phaseIndex=1&topicIndex=1", wait_until="networkidle")
                for index in range(2):
                    row = page.locator("#diagnostic-questions .classroom-question").nth(index)
                    row.locator("input[value='1']").check()
                    row.locator("[data-submit]").click()
                    row.locator(".classroom-feedback").get_by_text("先检查", exact=False).wait_for()
                page.locator("#classroom-branch").get_by_text("分钟").wait_for()
                failure = browser.new_page(viewport={"width": 1280, "height": 900})
                failure.route("**/api/classroom/start?*", lambda route: route.fulfill(
                    status=503, content_type="application/json", body='{"error":"数据库暂不可用"}'))
                failure.goto(base + "/learn?courseId=c1&phaseIndex=1&topicIndex=1", wait_until="networkidle")
                failure.locator("#learn-content").wait_for(state="visible")
                assert "可继续原课程" in failure.locator("#diagnostic-status").inner_text()
                failure.close()
                review_page = browser.new_page(viewport={"width": 1280, "height": 900})
                review_page.goto(base + "/learn?courseId=c1&phaseIndex=1&topicIndex=1&review=1",
                                 wait_until="networkidle")
                review_page.locator("#learn-quiz fieldset").first.wait_for()
                for fieldset in review_page.locator("#learn-quiz fieldset").all():
                    fieldset.locator("input[value='0']").check()
                review_page.locator("#quiz-submit").click()
                review_page.locator("#quiz-result").get_by_text("已通过", exact=False).wait_for()
                review_page.locator("#classroom-next a").get_by_text("返回原课程路径", exact=False).wait_for()
                review_page.close()
                with closing(sqlite3.connect(database)) as db:
                    pending_reviews = db.execute(
                        "SELECT COUNT(*) FROM ClassroomActivity WHERE courseId='c1' AND kind='review' AND payload LIKE '%\"status\":\"pending\"%'"
                    ).fetchone()[0]
                    assert pending_reviews == 3
                idle = browser.new_page(viewport={"width": 1280, "height": 900})
                idle.clock.install()
                idle.goto(base + "/learn?courseId=c1&phaseIndex=1&topicIndex=1", wait_until="networkidle")
                idle.bring_to_front()
                idle.clock.fast_forward(120001)
                assert idle.locator("#classroom-idle").is_visible()
                idle.clock.fast_forward(16000)
                assert not idle.locator("#classroom-idle").is_visible()
                idle.clock.fast_forward(119000)
                assert not idle.locator("#classroom-idle").is_visible()
                idle.clock.fast_forward(7000)
                assert idle.locator("#classroom-idle").is_visible()
                idle.clock.fast_forward(16000)
                assert not idle.locator("#classroom-idle").is_visible()
                idle.clock.fast_forward(121000)
                assert not idle.locator("#classroom-idle").is_visible()
                idle.close()
                print("BROWSER_UI pages=7 diagnostic=2 skip=full interaction=feedback weekly_edit_confirm=PASS review_return=PASS JS_ERRORS=0")
                print("BROWSER_BRANCH familiar=PASS weak=PASS failure_continue=PASS idle_2min=PASS")
                for path, failures in audit.items():
                    print("CONTRAST", path, json.dumps(failures, ensure_ascii=False))
                browser.close()
        finally:
            service.terminate()
            service.wait(timeout=10)
            ai.shutdown(); ai.server_close(); worker.join(timeout=2)


if __name__ == "__main__":
    import sys
    main(Path(sys.argv[1]).resolve())

"""检查课堂页面交互和主要页面文字对比度。"""

import json
import io
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


class BrowserAI(MockAI):
    """新备课任务真实走本地 HTTP/SSE，测试模型仅返回虚构课程。"""
    def do_POST(self):
        raw = self.rfile.read(int(self.headers['Content-Length']))
        body = json.loads(raw)
        system = body['messages'][0]['content']
        if not body.get('stream') or not ('全课程教学统筹教师' in system or '钢一定制AI的备课教师' in system):
            self.rfile = io.BytesIO(raw)
            return super().do_POST()
        MockAI.requests.append(body)
        data = json.loads(body['messages'][-1]['content'])
        if '全课程教学统筹教师' in system:
            candidates = data['candidates']
            item = next(candidate for candidate in candidates if candidate['kind'] == 'lesson' and candidate['topicIndex'] == 1)
            result = {'reason': '根据本次实际回答，先巩固区间与定义。', 'taskId': item['taskId'],
                      'action': 'consolidate', 'minutes': min(10, item['availableMinutes']),
                      'instruction': '结合已写出的解题过程补讲', 'knowledgePoints': ['区间', '定义']}
        else:
            block, topic = data['block'], data['topic']
            if block == 'overview':
                result = {'title': topic + '巩固课堂', 'summary': topic + '通过具体判断理解变化。',
                          'inferredDomain': '数学', 'keyConcepts': ['定义', '区间', '方向']}
            elif block == 'steps':
                result = {'lessonSteps': [{'title': topic + '步骤' + str(i), 'explanation': '先确定区间，再比较函数值。',
                    'example': '观察 y=x 的变化', 'action': '写出判断依据', 'check': '私有自检秘密'} for i in range(4)]}
            elif block == 'examples':
                result = {'examples': [{'title': topic + '例题' + str(i), 'content': '请判断区间中的变化' + str(i),
                    'solution': '私有例题答案秘密'} for i in range(2)]}
            elif block == 'practice':
                result = {'practice': [{'title': topic + '练习' + str(i), 'task': '说明区间与变化依据' + str(i),
                    'check': '私有练习检查秘密'} for i in range(3)]}
            elif block == 'quiz':
                result = {'quiz': [{'question': topic + '判断题' + str(i), 'options': ['甲', '乙', '丙', '丁'],
                    'answerIndex': 0, 'explanation': '私有测验解析秘密'} for i in range(3)]}
            else:
                result = {'checkpoint': [topic + '说明定义', '能判断变化方向'], 'commonMistakes': ['遗漏条件', '忽略区间'],
                          'resourceSummary': topic + '参考教材'}
        encoded = json.dumps(result, ensure_ascii=False)
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream')
        self.end_headers()
        try:
            for offset in range(0, len(encoded), 25):
                event = {'model': 'mock-browser-v550', 'choices': [{'delta': {'content': encoded[offset:offset+25]}}]}
                self.wfile.write(('data: ' + json.dumps(event, ensure_ascii=False) + '\n\n').encode())
                self.wfile.flush(); time.sleep(.025)
            self.wfile.write(b'data: [DONE]\n\n'); self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            pass


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
        ai = ThreadingHTTPServer(("127.0.0.1", free_port()), BrowserAI)
        worker = Thread(target=ai.serve_forever, daemon=True)
        worker.start()
        port = free_port()
        env = dict(os.environ, HOST="127.0.0.1", PORT=str(port), DATABASE_PATH=str(database),
                   AI_BASE_URL=f"http://127.0.0.1:{ai.server_port}/v1", AI_API_KEY="mock-key",
                   BOCHA_API_KEY="", SEARCH_FALLBACK_PROVIDER="", RESOURCE_SEARCH_CACHE_DIR=str(Path(directory) / 'search'),
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
                        assert page.locator("#home-availability").count() == 0
                        page.locator("#goal-examples .goal-example").first.wait_for()
                        assert page.locator("#goal-examples .goal-example").count() == 5
                        assert page.locator("#goal-examples img").count() == 0
                        assert not page.evaluate("window.homeInjected || false")
                        recommendation = page.locator("#goal-examples .goal-example").first.inner_text()
                        page.locator("#goal-examples .goal-example").first.click()
                        assert page.locator("#goal").input_value() == recommendation
                        page.locator("[data-mode='lite']").click()
                        assert page.locator("#goal-examples .goal-example").count() == 5
                        assert "lite" in page.locator("#goal-examples .goal-example").first.inner_text()
                        page.locator("[data-mode='deep']").click()
                        page.locator("#recent-courses a").first.wait_for()
                        assert "courseId=c1" in page.locator("#recent-courses a").first.get_attribute("href")
                        home_calls = [call for call in MockAI.requests if "首页学习目标推荐助手" in call["messages"][0]["content"]]
                        assert len(home_calls) == 1
                        page.reload(wait_until="networkidle")
                        assert len([call for call in MockAI.requests if "首页学习目标推荐助手" in call["messages"][0]["content"]]) == 1
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
                    if path == "/ask":
                        page.locator("#ask-question").fill("思考测试：**重点** $x^2+y^2=r^2$")
                        page.locator("#ask-submit").click()
                        page.locator("#ask-messages .chat-thinking").wait_for(state="visible", timeout=8000)
                        page.locator("#ask-messages .chat-body strong").first.wait_for(timeout=20000)
                        page.locator("#ask-messages .chat-body .katex").first.wait_for(timeout=20000)
                        page.locator("#ask-messages .chat-body ul li").first.wait_for(timeout=20000)
                        page.locator("#ask-submit:not([disabled])").wait_for(timeout=20000)
                        page.locator("#ask-messages .chat-actions .chat-copy").last.wait_for(timeout=10000)
                        assert page.locator("#ask-messages .chat-chips .chat-chip").count() >= 1
                        page.locator("#ask-messages .chat-chips .chat-chip").first.click()
                        page.locator("#ask-submit:not([disabled])").wait_for(timeout=20000)
                    if path == "/my-courses":
                        for name in ("uc-overview", "uc-courses", "uc-time", "uc-profile", "uc-settings"):
                            assert page.locator("#" + name).count() == 1, name
                        assert page.locator(".uc-nav a").count() == 5
                        assert page.locator(".uc-course").count() >= 1
                        assert page.locator(".uc-course a[href^='/plan?courseId=']").first.is_visible()
                        assert page.locator("#uc-availability-grid input[type=checkbox]:checked").count() == 3
                        monday = page.locator("#uc-availability-grid input[data-weekday='1']")
                        tuesday = page.locator("#uc-availability-grid input[data-weekday='2']")
                        monday.uncheck()
                        tuesday.check()
                        tuesday.locator("xpath=..//input[@type='number']").fill("45")
                        tuesday.dispatch_event("change")
                        page.locator("#uc-availability-summary").get_by_text("已保存为全部课程共享预算", exact=False).wait_for()
                        assert "周二" in page.locator("#uc-availability-summary").inner_text()
                        page.wait_for_function("() => document.getElementById('uc-profile-subjects').textContent.trim().length > 0")
                        page.wait_for_function("() => document.getElementById('uc-profile-topics').textContent.trim().length > 0")
                        page.locator("#uc-settings").scroll_into_view_if_needed()
                        assert page.locator("#open-api-settings").is_visible()
                        page.set_viewport_size({"width": 390, "height": 844})
                        assert page.evaluate("document.documentElement.scrollWidth <= window.innerWidth + 1")
                        assert page.locator(".uc-nav a").first.is_visible()
                        page.set_viewport_size({"width": 1280, "height": 900})
                    if path.startswith("/plan"):
                        page.locator("#weekly-entries .weekly-row").first.wait_for()
                        assert page.locator("#weekly-availability input[type=checkbox]:checked").count() == 3
                        assert not page.locator("#weekly-availability input[data-weekday='1']").is_checked()
                        assert page.locator("#weekly-availability input[data-weekday='2']").is_checked()
                        assert page.locator("#weekly-availability input[data-weekday='2']").locator(
                            "xpath=..//input[@type='number']").input_value() == "45"
                        first_link = page.locator(".next-action a.primary-action").first.get_attribute("href")
                        assert "phaseIndex=1&topicIndex=1" in first_link, first_link
                        page.locator("#weekly-replan:not([disabled])").wait_for()
                        if page.locator("#weekly-accept").is_visible():
                            page.locator("#weekly-accept").click()
                            page.locator("#weekly-confirm").wait_for(state="hidden")
                        first_title = page.locator("#weekly-entries .weekly-row a").first.inner_text()
                        review = page.locator("#weekly-entries .weekly-row").filter(has_text="到期复习")
                        review.locator("[data-up]").click()
                        review.locator("[data-up]").click()
                        assert page.locator("#weekly-entries .weekly-row a").first.inner_text() != first_title
                        page.locator("#weekly-entries .weekly-row input[type=date]").first.fill("2026-10-01")
                        minutes = page.locator("#weekly-entries .weekly-row input[type=number]").first
                        minutes.fill("25")
                        page.locator("#weekly-save").click()
                        page.locator("#weekly-message").get_by_text("修改已保存").wait_for()
                        page.locator("#weekly-replan").click()
                        page.locator("#weekly-confirm").get_by_text("AI 新旧安排对比").wait_for()
                        page.locator("#weekly-cancel").click()
                        page.locator("#weekly-confirm").wait_for(state="hidden")
                        page.locator("#weekly-replan").click()
                        page.locator("#weekly-accept").wait_for()
                        calls_before = len([call for call in MockAI.requests if "学习统筹教师" in call["messages"][0]["content"]])
                        page.locator("#weekly-accept").click()
                        page.locator("#weekly-confirm").wait_for(state="hidden")
                        assert "学习安排 · " in page.locator("#weekly-title").inner_text()
                        assert "已" in page.locator("#weekly-message").inner_text()
                        assert len([call for call in MockAI.requests if "学习统筹教师" in call["messages"][0]["content"]]) == calls_before
                        page.locator(".primary-action").first.hover()
                        assert not page.evaluate(AUDIT), "计划页悬停态对比度不足"
                    if path.startswith("/learn"):
                        page.locator("#diagnostic-questions .classroom-question").first.wait_for()
                        assert page.locator("#diagnostic-questions .classroom-question").count() == 3
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
                        for target in ('diagnostic-questions', 'interaction-questions', 'learn-examples', 'learn-practice', 'learn-quiz'):
                            page.locator('#' + target + ' .ai-dialogue').first.wait_for()
                            row = page.locator('#' + target + ' .ai-dialogue').first
                            assert row.locator('textarea').count() == 1
                            # 同一浏览器帧取两者位置，避免流式自动滚动夹在两次读取之间。
                            assert row.evaluate("el=>el.querySelector('h3').getBoundingClientRect().top<el.querySelector('textarea').getBoundingClientRect().top"), target
                            assert row.get_by_role('button', name='跳过本题').count() == 1
                        assert page.locator('#quiz-submit').count() == 0
                        assert '解析：' not in page.locator('#learn-examples').inner_text()
                        row = page.locator('#interaction-questions .classroom-question').first
                        row.locator("input[value='1']").check()
                        row.locator('textarea').fill('我比较区间两端的函数值，所以认为递增。')
                        row.locator('[data-submit]').click()
                        row.locator('.ai-dialogue-history').get_by_text('片段一片段二', exact=True).wait_for()
                        row.locator('[data-submit]:not([disabled])').wait_for()
                        assert row.locator('textarea').is_visible()
                    audit[path] = page.evaluate(AUDIT)
                assert not errors, errors
                assert all(not failures for failures in audit.values()), audit
                page.goto(base + '/learn?courseId=c1&phaseIndex=1&topicIndex=1', wait_until='networkidle')
                row = page.locator('#diagnostic-questions .classroom-question').first
                row.locator("input[value='0']").check(); row.locator('[data-submit]').click()
                row.locator('.ai-dialogue-history').get_by_text('片段一片段二', exact=True).wait_for()
                row.locator('[data-submit]:not([disabled])').wait_for()
                row.locator('textarea').fill('请解释比较过程'); row.locator('[data-submit]').click()
                row.locator('.ai-dialogue-history').get_by_text('片段一片段二', exact=True).nth(1).wait_for()
                row.locator('[data-submit]:not([disabled])').wait_for()
                page.reload(wait_until='networkidle')
                row = page.locator('#diagnostic-questions .classroom-question').first
                assert row.locator('.ai-dialogue-history').get_by_text('片段一片段二', exact=True).count() == 2
                failure = browser.new_page(viewport={'width': 1280, 'height': 900})
                failure.route('**/api/classroom/questions?*kind=diagnostic*', lambda route: route.fulfill(
                    status=503, content_type='application/json', body=json.dumps({'error':'数据库暂不可用'})))
                failure.goto(base + '/learn?courseId=c1&phaseIndex=1&topicIndex=1', wait_until='networkidle')
                failure.locator('#learn-content').wait_for(state='visible')
                assert '数据库暂不可用' in failure.locator('#diagnostic-questions').inner_text()
                assert failure.locator('#diagnostic-questions').get_by_role('button', name='重试读取题目').count() == 1
                failure.close()
                review_page = browser.new_page(viewport={'width': 1280, 'height': 900})
                review_page.goto(base + '/learn?courseId=c1&phaseIndex=1&topicIndex=1&review=1&reviewId=classroom%3Ac1%3A1%3A1%3Areview%3A1', wait_until='networkidle')
                review_page.locator('#learn-quiz .ai-dialogue').first.wait_for()
                assert review_page.locator('#learn-quiz .ai-dialogue').first.get_attribute('data-kind') == 'review'
                for row in review_page.locator('#learn-quiz .ai-dialogue').all():
                    row.locator("input[value='0']").check(); row.locator('[data-submit]').click()
                    row.locator('.ai-dialogue-history').get_by_text('片段一片段二', exact=True).wait_for()
                    row.locator('[data-submit]:not([disabled])').wait_for()
                review_page.locator('#complete-lesson').click()
                review_page.locator('#complete-feedback').get_by_text('已保存', exact=False).wait_for()
                review_page.close()
                # 主动点击只创建一次任务，刷新与实际 WebSocket 订阅不重复发模型请求。
                page.goto(base + '/learn?courseId=c1&phaseIndex=1&topicIndex=1', wait_until='networkidle')
                # 这一项检查稳定版本的正常保存；画像异步更新导致的过期另有留页验收。
                page.wait_for_function("""async () => {
                  const profile = await fetch('/api/profile').then(r=>r.json());
                  const plan = await fetch('/api/study-plan').then(r=>r.json());
                  return !profile.updating && !profile.abilityStatus.updating && plan.status === 'ready';
                }""", timeout=30000)
                before = len([call for call in MockAI.requests if call.get('stream') and ('全课程教学统筹教师' in call['messages'][0]['content'] or '钢一定制AI的备课教师' in call['messages'][0]['content'])])
                prep_events = []
                page.on('websocket', lambda connection: connection.on('framereceived', lambda payload: prep_events.append(payload)))
                page.locator('#lesson-next').click()
                page.wait_for_url('**/learn/next?id=*')
                page.locator('#next-lesson-blocks [data-stage=decision]').wait_for(state='visible')
                page.reload()
                page.wait_for_url('**/learn?**lessonTaskId=*', timeout=30000)
                page.locator('#learn-content').wait_for(state='visible')
                page.wait_for_function("""() => document.querySelectorAll('#learn-examples .ai-dialogue').length === 2 &&
                  document.querySelectorAll('#learn-practice .ai-dialogue').length === 3 &&
                  document.querySelectorAll('#learn-quiz .ai-dialogue').length === 3""")
                assert page.locator('#learn-examples .ai-dialogue').count() == 2
                assert page.locator('#learn-practice .ai-dialogue').count() == 3
                assert page.locator('#learn-quiz .ai-dialogue').count() == 3
                assert page.locator('#lesson-next').inner_text() == '让 AI 准备下一课'
                assert '秘密' not in json.dumps(prep_events, ensure_ascii=False)
                after = len([call for call in MockAI.requests if call.get('stream') and ('全课程教学统筹教师' in call['messages'][0]['content'] or '钢一定制AI的备课教师' in call['messages'][0]['content'])])
                assert after - before == 7, (before, after)
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
                print("BROWSER_UI pages=7 all_question_types=PASS dialogue_restore=PASS weekly_edit_confirm=PASS review_save=PASS JS_ERRORS=0")
                print("BROWSER_PREPARATION real_http_sse=PASS reload_subscribe=PASS hidden_answers=PASS requests=7 idle_2min=PASS")
                for path, failures in audit.items():
                    print("CONTRAST", path, json.dumps(failures, ensure_ascii=False))
                browser.close()
        except Exception:
            try:
                print("PAGE_URL", page.url, flush=True)
                if '/learn/next?id=' in page.url:
                    task_id = page.url.split('id=', 1)[1].split('&', 1)[0]
                    task = json.loads(urlopen(base + '/api/learn/next/' + task_id).read())
                    print("PREPARATION_DEBUG", {key: task.get(key) for key in ('id', 'status', 'stage', 'message', 'latestSeq')}, flush=True)
                    with closing(sqlite3.connect(database)) as check:
                        stored = json.loads(check.execute('SELECT payload FROM ClassroomActivity WHERE id=?', ('next-preparation:' + task_id,)).fetchone()[0])
                        versions = stored.get('versions', {})
                        meta = dict(check.execute('SELECT key,value FROM ProfileMeta'))
                        changed = []
                        if versions.get('learningVersion') != int(meta.get('learning-revision', '1')):
                            changed.append(('learningVersion', versions.get('learningVersion'), meta.get('learning-revision')))
                        for row_id, saved in versions.get('sessions', {}).items():
                            current = check.execute('SELECT content FROM LearningSession WHERE id=?', (row_id,)).fetchone()
                            if current and current[0] != saved: changed.append(('session', row_id))
                        for key, saved in versions.get('exposures', {}).items():
                            if meta.get(key, '') != saved: changed.append(('exposure', key))
                        if json.loads(meta.get('learning-flow', '{}')).get('plan') != versions.get('studyPlan'): changed.append(('studyPlan',))
                        if meta.get('study-drafts', '') != versions.get('drafts'): changed.append(('drafts',))
                        print('FINGERPRINT_CHANGED', changed, flush=True)
                print("FLOW_DEBUG", urlopen(base + "/api/study-plan").read().decode(), flush=True)
                print("FLOW_CALLS", [call["messages"][0]["content"][:32] for call in MockAI.requests], flush=True)
                print("JS_ERRORS", errors, flush=True)
            except Exception: pass
            raise
        finally:
            service.terminate()
            service.wait(timeout=10)
            ai.shutdown(); ai.server_close(); worker.join(timeout=2)


if __name__ == "__main__":
    import sys
    main(Path(sys.argv[1]).resolve())

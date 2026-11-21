#!/usr/bin/env python3
"""使用隔离的本地假接口检查试卷页面交互，不访问用户档案或模型。"""

import base64
import json
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit

from playwright.sync_api import expect, sync_playwright
from classroom_v560_fixture import open_learning_menu

ROOT = Path(__file__).resolve().parents[1]
PUBLIC = ROOT / "public"
SESSION = {
    "id": "fixture", "title": "测试试卷", "status": "ready", "activeTaskId": "", "progress": {"current": 1, "total": 1},
    "pages": [{"name": "试卷第一页.png"}], "files": [], "sources": [],
    "questions": [{"id": "q1", "number": "1", "subject": "历史", "stem": "测试题干",
                   "handwrittenAnswer": "这是识别出的手写内容", "handwrittenAnswerConfidence": 0.99,
                   "materials": ["印刷材料", {"label": "材料一", "text": "材料原文", "referenceAnswer": "不应展示的材料答案"}],
                   "subquestions": [{"label": "（1）", "text": "请概括特点", "referenceAnswer": "不应展示的参考答案", "rubric": "不应展示的评分标准", "score": 6},
                                    "请说明影响", {"label": "（3）", "stem": "仅供查看的题目", "rubric": "隐藏评分"}],
                   "recognitionStatus": "ready"},
                  {"id": "q2", "number": "2", "subject": "历史", "stem": "没有手写答案的题目",
                   "materials": [], "subquestions": [], "recognitionStatus": "ready"}],
    "messages": [
        {"id": "legacy", "role": "assistant", "content": "```mindmap-json\n{\"root\":{\"label\":\"旧导图\",\"children\":[]}}\n```"},
        {"id": "message-map", "role": "assistant", "content": "两张学习导图\n图示之间的说明", "artifactIds": ["map-a", "map-b", "chart-a"],
         "artifactParts": [{"type": "text", "text": "两张学习导图"}, {"type": "artifact", "id": "map-a"},
                           {"type": "text", "text": "图示之间的说明"}, {"type": "artifact", "id": "map-b"},
                           {"type": "artifact", "id": "chart-a"}]},
    ],
    "artifacts": [
        {"id": "map-a", "type": "mindmap", "originMessageId": "message-map", "revision": 1,
         "content": json.dumps({"root": {"label": "第一张", "children": [{"label": "原分支"}]}}, ensure_ascii=False)},
        {"id": "map-b", "type": "mindmap", "originMessageId": "message-map", "revision": 1,
         "content": json.dumps({"root": {"label": "第二张", "children": [{"label": "保持状态"}]}}, ensure_ascii=False)},
        {"id": "chart-a", "type": "chart", "originMessageId": "message-map", "revision": 1,
         "content": json.dumps({"type": "bar", "categories": ["甲", "乙"], "series": [{"name": "示例数据", "data": [3, 5]}]}, ensure_ascii=False)},
    ],
}
CONFIRMATIONS = []
CHAT_POSTS = []
ALLOW_CHAT = False
HTML = """<!doctype html><html lang="zh-CN"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<link rel="stylesheet" href="/styles.css"><link rel="stylesheet" href="/clarity.css">
<link rel="stylesheet" href="/exam-page.css"><link rel="stylesheet" href="/exam-rich-output.css">
<link rel="stylesheet" href="/workspace.css"><script src="/workspace.js"></script>
<script>window.GangyiAgent={watch:(id,handlers)=>{window.lastWatchedTask=id;window.watchedHandlers=handlers;return ()=>{}}};</script>
<script defer src="/vendor/echarts/echarts.min.js"></script><script defer src="/chat-render.js"></script><script defer src="/exam-rich-output.js"></script>
<script defer src="/exam-page.js"></script></head><body class="clarity-app">
<header class="site-header dark-header"><div class="dark-nav-shell"></div></header>
<main class="exam-page" id="exam-page"></main></body></html>"""


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *_):
        pass

    def json_response(self, status, body):
        data = json.dumps(body, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        path = urlsplit(self.path).path
        if path == "/exam-explanations":
            data, mime = HTML.encode(), "text/html; charset=utf-8"
        elif path == "/api/exam-explanations/fixture":
            return self.json_response(200, SESSION)
        elif path == "/api/exam-explanations/fixture/sources":
            return self.json_response(200, {"sources": SESSION["sources"]})
        elif path == "/api/exam-explanations/fixture/source-confirmations":
            return self.json_response(200, {"confirmations": CONFIRMATIONS})
        elif path == "/api/exam-explanations/fixture/pages/0":
            data = base64.b64decode("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+/8fkAAAAASUVORK5CYII=")
            mime = "image/png"
        elif path == "/vendor/echarts/echarts.min.js":
            data = (PUBLIC / "vendor/echarts/echarts.min.js").read_bytes()
            mime = "application/javascript"
        else:
            name = path.lstrip("/")
            if "/" in name or not (PUBLIC / name).is_file():
                self.send_error(404)
                return
            data = (PUBLIC / name).read_bytes()
            mime = "text/css" if name.endswith(".css") else "application/javascript"
        self.send_response(200)
        self.send_header("Content-Type", mime)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_PATCH(self):
        payload = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
        question = next(question for question in SESSION["questions"] if question["id"] == payload["questionId"])
        if "subquestionIndex" in payload:
            assert set(payload) == {"questionId", "subquestionIndex", "subquestionText"}
            subquestions = question["subquestions"]
            index = payload["subquestionIndex"]
            if isinstance(subquestions[index], dict):
                subquestions[index]["text"] = payload["subquestionText"]
            else:
                subquestions[index] = payload["subquestionText"]
        elif "subquestions" in payload:
            assert set(payload) == {"questionId", "subquestions"}
            question["subquestions"] = payload["subquestions"]
        else:
            question["stem"] = payload.get("text", question["stem"])
        self.json_response(200, SESSION)

    def do_POST(self):
        path = urlsplit(self.path).path
        if path == "/api/exam-explanations/fixture/source-snapshots":
            payload = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
            assert payload["url"] == "https://example.org/exam"
            return self.json_response(200, {"id": "snapshot-1", "sourceUrl": payload["url"], "segments": [
                {"id": "s1", "text": "原文题目：测试题干。"}, {"id": "s2", "text": "答案与评分依据：A。"}]})
        if path == "/api/exam-explanations/fixture/source-confirmations":
            payload = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
            assert payload == {"questionId": "q1", "snapshotId": "snapshot-1", "questionSegmentId": "s1",
                               "questionQuote": "测试题干", "answerSegmentId": "s2", "answerQuote": "A",
                               "confirmed": True}
            CONFIRMATIONS[:] = [{"questionId": "q1", "sourceUrl": "https://example.org/exam", "revoked": False,
                                 "gradingEligible": True, "label": "用户确认，非官方"}]
            return self.json_response(200, {"ok": True})
        if path == "/api/exam-explanations/fixture/source-revocations":
            payload = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
            assert payload == {"questionId": "q1", "sourceUrl": "https://example.org/exam"}
            CONFIRMATIONS[0]["revoked"] = True
            CONFIRMATIONS[0]["gradingEligible"] = False
            return self.json_response(200, {"ok": True})
        if path == "/api/exam-explanations/fixture/messages" and ALLOW_CHAT:
            payload = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
            CHAT_POSTS.append(payload)
            SESSION["messages"].append({"id": "sent-" + str(len(CHAT_POSTS)), "role": "user", **payload, "content": payload["text"]})
            SESSION["activeRequestId"] = payload["requestId"]
            SESSION["activeTaskId"] = "chat-" + str(len(CHAT_POSTS))
            return self.json_response(202, {"chat": SESSION, "task": {"id": SESSION["activeTaskId"]}})
        if path == "/api/exam-explanations/fixture/recognition-retries":
            payload = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
            assert payload.get("requestId", "").startswith("exam-retry-")
            SESSION["status"] = "recognizing"
            SESSION["activeTaskId"] = "retry-fixture"
            return self.json_response(202, {"session": SESSION, "task": {"id": "retry-fixture"}})
        self.json_response(503, {"error": "模拟网络失败"})


def main():
    global ALLOW_CHAT
    server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    try:
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True, channel="msedge")
            page = browser.new_page(viewport={"width": 1280, "height": 720})
            page.goto(f"http://127.0.0.1:{server.server_port}/exam-explanations?id=fixture")
            expect(page.get_by_role("heading", name="测试试卷")).to_be_visible()
            expect(page.get_by_text("旧导图", exact=True)).to_be_visible()
            first_map = page.locator('[data-artifact-id="map-a"]')
            second_map = page.locator('[data-artifact-id="map-b"]')
            expect(first_map.get_by_text("原分支", exact=True)).to_be_visible()
            expect(second_map.get_by_text("保持状态", exact=True)).to_be_visible()
            chart = page.locator('[data-artifact-id="chart-a"] canvas')
            expect(chart).to_be_visible()
            page.wait_for_function("""() => {
                const canvas = document.querySelector('[data-artifact-id="chart-a"] canvas');
                if (!canvas || canvas.width < 100 || canvas.height < 100) return false;
                const pixels = canvas.getContext('2d').getImageData(0, 0, canvas.width, canvas.height).data;
                let painted = 0;
                for (let i = 3; i < pixels.length; i += 4) if (pixels[i] > 0 && ++painted > 100) return true;
                return false;
            }""")
            assert page.evaluate("""() => {
                const a = document.querySelector('[data-artifact-id="map-a"]');
                const b = document.querySelector('[data-artifact-id="map-b"]');
                const text = [...a.parentElement.children].find(node => node.textContent === '图示之间的说明');
                return a.nextElementSibling === text && text.nextElementSibling === b;
            }"""), "图文顺序与回复不一致"
            second_map.get_by_role("button", name="放大图示").click()
            old_transform = second_map.locator('.exam-rich-pan-canvas').get_attribute('style')
            assert 'scale(1.15)' in old_transform
            source = page.locator(".exam-source-body")
            expect(source).to_be_hidden()
            open_learning_menu(page).get_by_role("button", name="查看原卷和核对题目").click()
            expect(source).to_be_visible()
            page.get_by_role("button", name="收起原卷").click()
            expect(source).to_be_hidden()
            open_learning_menu(page).get_by_role("button", name="查看原卷和核对题目").click()
            expect(source).to_be_visible()
            first_question = page.locator('.exam-question').nth(0)
            expect(first_question.get_by_text('手写识别可能有误，请核对原图与识别文字。')).to_be_visible()
            expect(first_question.get_by_text('已识别手写答案：这是识别出的手写内容')).to_be_visible()
            expect(page.locator('.exam-question').nth(1).get_by_text('手写识别可能有误，请核对原图与识别文字。')).to_have_count(0)
            first_question.get_by_text('校正材料、小问和手写答案').click()
            expect(first_question.get_by_role('textbox', name='修改第1题手写答案')).to_have_value('这是识别出的手写内容')
            expect(first_question.get_by_role('textbox', name='查看第1题材料')).to_have_value('印刷材料\n\n材料一 材料原文')
            expect(first_question.get_by_role('textbox', name='查看第1题材料')).to_have_attribute('readonly', '')
            expect(first_question.get_by_role('textbox', name='修改第1题第1小问')).to_have_value('请概括特点')
            expect(first_question.get_by_role('textbox', name='修改第1题第2小问')).to_have_value('请说明影响')
            expect(first_question.get_by_role('textbox', name='查看第1题第3小问')).to_have_value('仅供查看的题目')
            expect(first_question.get_by_role('textbox', name='查看第1题第3小问')).to_have_attribute('readonly', '')
            assert '不应展示的参考答案' not in first_question.inner_text()
            assert '不应展示的评分标准' not in first_question.inner_text()
            with page.expect_response(lambda response: response.request.method == 'PATCH'):
                first_question.get_by_role('textbox', name='修改第1题第1小问').fill('修正后的小问题干')
                page.get_by_role('heading', name='测试试卷').click()
            assert SESSION['questions'][0]['subquestions'][0] == {"label": "（1）", "text": "修正后的小问题干", "referenceAnswer": "不应展示的参考答案", "rubric": "不应展示的评分标准", "score": 6}
            assert SESSION['questions'][0]['materials'][1]['referenceAnswer'] == '不应展示的材料答案'
            first_question.get_by_text('校正材料、小问和手写答案').click()
            with page.expect_response(lambda response: response.request.method == 'PATCH'):
                first_question.get_by_role('textbox', name='修改第1题第2小问').fill('修正后的字符串小问')
                page.get_by_role('heading', name='测试试卷').click()
            assert SESSION['questions'][0]['subquestions'][1] == '修正后的字符串小问'
            second_question = page.locator('.exam-question').nth(1)
            second_question.get_by_text('校正材料、小问和手写答案').click()
            with page.expect_response(lambda response: response.request.method == 'PATCH'):
                second_question.get_by_role('textbox', name='修改第2题小问列表').fill('新增小问一\n新增小问二')
                page.get_by_role('heading', name='测试试卷').click()
            assert SESSION['questions'][1]['subquestions'] == ['新增小问一', '新增小问二']
            link = page.get_by_role("link", name="打开试卷第一页.png原图")
            expect(link).to_have_attribute("href", "/api/exam-explanations/fixture/pages/0")
            expect(link).to_have_attribute("target", "_blank")
            screenshot_dir = ROOT / "verification"
            screenshot_dir.mkdir(exist_ok=True)
            page.screenshot(path=str(screenshot_dir / "exam-page-fixes-desktop.png"), full_page=True)

            draft = page.get_by_role("textbox", name="输入学习消息")
            draft.fill("这是尚未发送的学习问题")
            SESSION["sources"] = [{"title": "待核实的候选资料", "url": "https://example.org/exam", "match": "unverified"}]
            expect(page.get_by_role("link", name="待核实的候选资料")).to_be_visible(timeout=9000)
            expect(page.get_by_text("联网候选来源（未核实，不作为正式评分依据）")).to_be_visible()
            expect(draft).to_have_value("这是尚未发送的学习问题")
            page.get_by_role("button", name="核对原文").click()
            review = page.locator("dialog.exam-source-review[open]")
            expect(review.get_by_text("请对照原卷，分别选择题目和答案的原文证据；确认由用户作出，非官方。")).to_be_visible()
            review.get_by_role("button", name="确认对应关系").click()
            expect(review.get_by_role("status")).to_contain_text("请选择题目")
            review.get_by_label("对应试卷题目").select_option("q1")
            review.get_by_label("题目原文证据").select_option("s1")
            review.get_by_label("答案或评分依据原文证据").select_option("s2")
            question_quote = review.get_by_label("题目引用（至少 12 字节，最多 8000 字节）")
            answer_quote = review.get_by_label("答案引用（最多 8000 字节）")
            question_quote.fill("伪造的引用")
            answer_quote.fill("A")
            review.get_by_role("checkbox").check()
            review.get_by_role("button", name="确认对应关系").click()
            expect(review.get_by_role("status")).to_contain_text("必须逐字来自")
            assert not CONFIRMATIONS
            question_quote.fill("测试")
            review.get_by_role("button", name="确认对应关系").click()
            expect(review.get_by_role("status")).to_contain_text("12 至 8000 字节")
            question_quote.fill("测试题干")
            page.screenshot(path=str(screenshot_dir / "exam-source-review-desktop.png"), full_page=True)
            page.set_viewport_size({"width": 390, "height": 844})
            assert review.evaluate("el => el.getBoundingClientRect().width <= window.innerWidth"), "核验对话框在窄屏溢出"
            page.screenshot(path=str(screenshot_dir / "exam-source-review-mobile.png"), full_page=True)
            page.set_viewport_size({"width": 1280, "height": 720})
            review.get_by_role("button", name="确认对应关系").click()
            expect(page.get_by_text("第 1 题：用户确认，非官方")).to_be_visible()
            CONFIRMATIONS[0]["gradingEligible"] = False
            expect(page.get_by_text("第 1 题：已失效，请重新核对")).to_be_visible(timeout=9000)
            SESSION["sources"] = []
            expect(page.get_by_text("历史来源（不在当前候选中）")).to_be_visible(timeout=9000)
            page.get_by_role("button", name="撤销确认").click()
            expect(page.get_by_text("第 1 题：已撤销")).to_be_visible()
            expect(draft).to_have_value("这是尚未发送的学习问题")
            page.get_by_role("textbox", name="修改第1题题干").fill("修正后的题干")
            SESSION["artifacts"][0]["revision"] = 2
            SESSION["artifacts"][0]["content"] = json.dumps({"root": {"label": "第一张", "children": [{"label": "更新分支"}]}}, ensure_ascii=False)
            page.get_by_role("heading", name="测试试卷").click()
            expect(page.get_by_role("textbox", name="修改第1题题干")).to_have_value("修正后的题干")
            expect(first_map.get_by_text("更新分支", exact=True)).to_be_visible()
            expect(first_map.get_by_text("原分支", exact=True)).to_have_count(0)
            assert second_map.locator('.exam-rich-pan-canvas').get_attribute('style') == old_transform, "更新另一张图后缩放状态丢失"
            expect(page.get_by_role("textbox", name="输入学习消息")).to_have_value("这是尚未发送的学习问题")
            page.get_by_role("button", name="发送", exact=True).click()
            expect(page.get_by_role("status")).to_contain_text("输入内容已保留")
            expect(page.get_by_role("textbox", name="输入学习消息")).to_have_value("这是尚未发送的学习问题")
            page.reload()
            expect(page.get_by_role("textbox", name="输入学习消息")).to_have_value("这是尚未发送的学习问题")
            expect(page.locator('[data-artifact-id="map-a"]').get_by_text("更新分支", exact=True)).to_be_visible()
            expect(page.get_by_text("旧导图", exact=True)).to_be_visible()

            for language, body, marker in [
                ("mermaid", "flowchart LR\nA[读题] --> B[思考]", "读题"),
                ("mindmap", "mindmap\n  根节点\n    分支", "根节点"),
            ]:
                page.evaluate("""([lang, source]) => {
                    const target = document.createElement('div'); target.id = 'fixture-rich';
                    document.body.append(target);
                    window.ExamRichOutput.render(target, '```' + lang + '\\n' + source + '\\n```');
                }""", [language, body])
                page.locator("#fixture-rich").get_by_role("button", name="大画布").click()
                dialog = page.locator("dialog[open]")
                expect(dialog).to_contain_text(marker)
                expect(dialog.locator(".exam-rich-error")).to_have_count(0)
                dialog.get_by_role("button", name="关闭").click()
                page.locator("#fixture-rich").evaluate("el => { window.ExamRichOutput.destroy(el); el.remove(); }")

            cyclic_flow = {"direction": "LR", "title": "三省六部制",
                           "nodes": [{"id": "emperor", "label": "皇帝提出救灾措施"},
                                     {"id": "draft", "label": "中书省负责草拟并起草正式诏令"},
                                     {"id": "review", "label": "门下省负责审议并可以封驳退回"},
                                     {"id": "execute", "label": "尚书省负责执行政令"},
                                     {"id": "ministry", "label": "户部负责具体办理"}],
                           "edges": [{"from": "emperor", "to": "draft"}, {"from": "draft", "to": "review"},
                                     {"from": "review", "to": "execute", "label": "通过"},
                                     {"from": "execute", "to": "ministry"},
                                     {"from": "review", "to": "draft", "label": "封驳退回"}]}
            page.evaluate("""source => {
                const target = document.createElement('div'); target.id = 'fixture-flow'; document.body.append(target);
                window.ExamRichOutput.renderArtifact(target, {type: 'flowchart', content: JSON.stringify(source)});
            }""", cyclic_flow)
            flow = page.locator('#fixture-flow')
            expect(flow.locator('.exam-rich-error')).to_have_count(0)
            expect(flow.locator('.exam-rich-node')).to_have_count(5)
            expect(flow.locator('.exam-rich-edge')).to_have_count(5)
            expect(flow.locator('.is-back-edge[data-from="review"][data-to="draft"]')).to_have_count(1)
            expect(flow.get_by_text('封驳退回', exact=True)).to_be_visible()
            assert flow.locator('.exam-rich-node').nth(1).locator('tspan').count() > 1, '长中文节点没有换行'
            assert flow.evaluate("""el => [...el.querySelectorAll('.exam-rich-node')].every(node => {
                const rect = node.querySelector('rect').getBBox(), text = node.querySelector('text').getBBox();
                return text.x >= rect.x - 1 && text.y >= rect.y - 1 &&
                    text.x + text.width <= rect.x + rect.width + 1 && text.y + text.height <= rect.y + rect.height + 1;
            })"""), '流程图节点文字超出边框'
            assert flow.locator('.is-back-edge').get_attribute('marker-end').startswith('url(#'), '回边没有箭头'
            flow.get_by_role('button', name='大画布').click()
            expect(page.locator('dialog[open] .is-back-edge')).to_have_count(1)
            page.locator('dialog[open]').get_by_role('button', name='关闭').click()
            flow.screenshot(path=str(screenshot_dir / 'exam-cyclic-flow-desktop.png'))

            page.set_viewport_size({"width": 390, "height": 844})
            assert page.evaluate("document.documentElement.scrollWidth <= window.innerWidth"), '回路流程图使窄屏页面横向溢出'
            expect(flow.locator('.exam-rich-node')).to_have_count(5)
            flow.screenshot(path=str(screenshot_dir / 'exam-cyclic-flow-mobile.png'))
            flow.evaluate("el => { window.ExamRichOutput.destroy(el); el.remove(); }")
            page.reload()
            expect(page.locator(".exam-source-body")).to_be_hidden()
            open_learning_menu(page).get_by_role("button", name="查看原卷和核对题目").click()
            expect(page.get_by_role("dialog", name="原卷与题目核对")).to_be_visible()
            page.keyboard.press("Escape")
            expect(page.get_by_role("dialog", name="原卷与题目核对")).to_be_hidden()
            expect(page.get_by_role("textbox", name="输入学习消息")).to_be_visible()
            assert page.evaluate("document.documentElement.scrollWidth <= window.innerWidth"), "窄屏页面发生横向溢出"
            page.screenshot(path=str(screenshot_dir / "exam-page-fixes-mobile.png"), full_page=True)

            SESSION["status"] = "failed"
            SESSION["progress"]["current"] = 0
            page.reload()
            expect(page.get_by_text("试卷识别未完成，原卷和已识别的部分已保存。", exact=False)).to_be_visible()
            expect(page.get_by_role("button", name="发送", exact=True)).to_be_disabled()
            page.get_by_role("button", name="继续识别").click()
            expect(page.get_by_text("正在识别试卷，可以先输入问题；识别完成后即可发送。", exact=True)).to_be_visible()
            assert page.evaluate("window.lastWatchedTask") == "retry-fixture", "重试后未监听识别任务"
            expect(page.get_by_role("textbox", name="输入学习消息")).to_have_value("这是尚未发送的学习问题")
            expect(page.get_by_role("button", name="发送", exact=True)).to_be_disabled()
            SESSION["status"] = "ready"
            SESSION["progress"]["current"] = 1
            SESSION["activeTaskId"] = ""
            page.evaluate("window.watchedHandlers.onState({status:'ready'})")
            expect(page.get_by_role("button", name="发送", exact=True)).to_be_enabled()
            expect(page.get_by_role("textbox", name="输入学习消息")).to_have_value("这是尚未发送的学习问题")
            ALLOW_CHAT = True
            page.get_by_role("textbox", name="输入学习消息").fill("请修改第一个导图")
            page.get_by_role("button", name="发送", exact=True).click()
            expect(page.get_by_text("请修改第一个导图", exact=True)).to_be_visible()
            assert page.evaluate("window.lastWatchedTask") == "chat-1"
            page.get_by_role("textbox", name="输入学习消息").fill("其他尚未发送的草稿")
            SESSION["status"] = "failed"
            SESSION["activeTaskId"] = ""
            SESSION["activeRequestId"] = ""
            page.evaluate("""window.watchedHandlers.onState({status:'failed', failure:{message:'无效图示 <img src=x onerror=alert(1)>'}})""")
            failure = page.get_by_role("alert")
            expect(failure).to_contain_text("无效图示 <img src=x onerror=alert(1)>")
            assert failure.locator('img').count() == 0, "失败信息不应被解析为 HTML"
            page.get_by_role("button", name="重新编辑原问题").click()
            expect(failure).to_contain_text("输入框已有草稿")
            expect(failure).to_contain_text("无效图示 <img src=x onerror=alert(1)>")
            expect(page.get_by_role("textbox", name="输入学习消息")).to_have_value("其他尚未发送的草稿")
            page.reload()
            expect(page.get_by_role("alert")).to_contain_text("无效图示 <img src=x onerror=alert(1)>")
            page.get_by_role("textbox", name="输入学习消息").fill("")
            page.get_by_role("button", name="重新编辑原问题").click()
            expect(page.get_by_role("textbox", name="输入学习消息")).to_have_value("请修改第一个导图")
            assert len(CHAT_POSTS) == 1, "恢复输入不应自动提交或重复写入"
            page.get_by_role("button", name="发送", exact=True).click()
            expect(page.get_by_role("alert")).to_have_count(0)
            assert len(CHAT_POSTS) == 2 and CHAT_POSTS[0]["requestId"] != CHAT_POSTS[1]["requestId"]
            browser.close()
        print("试卷页面浏览器交互通过：折叠、原页、草稿恢复、失败保护、图示大画布与窄屏。")
    finally:
        server.shutdown()
        server.server_close()


if __name__ == "__main__":
    main()

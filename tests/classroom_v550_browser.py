"""v5.5.0 浏览器协议验收：使用虚构题目，不访问私人档案或真实 AI。"""
import json
import pathlib
import re
import unittest
from urllib.parse import parse_qs, urlparse
from playwright.sync_api import sync_playwright

ROOT = pathlib.Path(__file__).resolve().parents[1]


class ClassroomBrowserTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.runtime = sync_playwright().start()
        cls.browser = cls.runtime.chromium.launch(headless=True)

    @classmethod
    def tearDownClass(cls):
        cls.browser.close()
        cls.runtime.stop()

    def setUp(self):
        self.page = self.browser.new_page()
        self.requests = []
        self.errors = []
        self.dialogue = None
        self.task_view = None
        self.page.on('pageerror', lambda error: self.errors.append(str(error)))
        self.page.route("**/*", self.route)
        self.page.add_init_script("""
          window.wires=[];
          class Wire {
            static OPEN=1; static CONNECTING=0;
            constructor(url){this.url=url; this.readyState=0; window.wires.push(this);
              setTimeout(()=>{this.readyState=1; if(this.onopen)this.onopen({});},10);}
            send(raw){this.sent=(this.sent||[]);this.sent.push(JSON.parse(raw));}
            close(){this.readyState=3;if(this.onclose)this.onclose({});}
            addEventListener(name,fn){if(name==='open')this.onopen=fn;}
            event(data){if(this.onmessage)this.onmessage({data:JSON.stringify(data)});}
          }
          window.WebSocket=Wire;
        """)

    def tearDown(self):
        self.page.close()

    def route(self, route):
        url = route.request.url
        if "/api/" in url:
            self.requests.append((url, route.request.post_data))
            payload = {"questionId": "fictional-q", "version": 2, "contentVersion": 7,
                       "turns": [], "evaluationStatus": "ready"}
            if self.dialogue and '/api/classroom/dialogue?' in url:
                payload = self.dialogue
            if '/api/classroom/questions?' in url:
                kind = parse_qs(urlparse(url).query).get('kind', ['quiz'])[0]
                payload = {"contentVersion": 7, "questions": [{"questionId": "fictional-" + kind,
                    "kind": kind, "index": 0, "question": "比较 2 与 3", "contentVersion": 7,
                    "type": "choice", "options": ["2 更大", "3 更大"], "status": "pending"}]}
            if '/api/learn?' in url:
                payload = {"ok": True, "contentVersion": 7, "kind": "consolidation", "blocks": {
                    "overview": {"title": "数量差", "summary": "比较大小", "keyConcepts": ["正数"]},
                    "steps": {"lessonSteps": [{"title": "理解差值", "explanation": "**观察两个数**"}]},
                    "examples": {"examples": [{"title": "例题", "content": "比较 2 与 3"}]},
                    "practice": {"practice": [{"title": "练习", "task": "比较 4 与 5"}]},
                    "quiz": {"quiz": [{"question": "比较 2 与 3", "options": ["2", "3"]}]},
                    "assessment": {"checkpoint": ["能比较大小"], "resourceSummary": "教材"}}}
            if "/api/learn/next" in url:
                payload = {"id": "fictional-task", "status": "pending", "events": [],
                           "latestSeq": 0, "blocks": {}, "url": "/learn/next?id=fictional-task"}
                if self.task_view and route.request.method == 'GET':
                    payload = self.task_view
            route.fulfill(content_type="application/json", body=json.dumps(payload))
        elif url.endswith(".js"):
            route.fulfill(content_type="application/javascript", body=(ROOT / "public" / url.rsplit("/", 1)[1]).read_text(encoding="utf-8"))
        else:
            route.fulfill(content_type="text/html", body=self.fixture)

    def open(self, body="<main id='fixture'></main>", path="/learn?courseId=fictional&phaseIndex=1&topicIndex=1&lessonTaskId=prepared"):
        self.fixture = "<html><head><meta charset='utf-8'></head><body>" + body + "<script src='/chat-render.js'></script><script src='/learning-flow.js'></script></body></html>"
        self.page.goto("http://fictional.local" + path)

    def question(self, kind="quiz"):
        self.page.evaluate("""kind=>document.getElementById('fixture').append(
          GangyiLearning.renderDialogue(kind,{question:'比较 2 与 3',questionId:'fictional-q',
          options:['2 更大','3 更大'],materials:'两数都是正数',contentVersion:7,index:0},0,
          {courseId:'fictional',phaseIndex:1,topicIndex:1,lessonTaskId:'prepared'}))""", kind)

    def test_all_kinds_keep_composer_and_true_stream_renderer(self):
        self.open()
        for kind in ["diagnostic", "interaction", "example", "practice", "quiz", "review"]:
            self.page.locator("#fixture").evaluate("node=>node.replaceChildren()")
            self.question(kind)
            row = self.page.locator(".ai-dialogue")
            self.assertIn("两数都是正数", row.inner_text())
            self.assertEqual(row.locator("textarea").count(), 1)
            row.locator("input[value='1']").check()
            row.locator("textarea").fill("因为 3 比 2 多 1")
            row.get_by_role("button", name="发送给 AI").click()
            self.page.wait_for_function("wires.at(-1)?.sent?.length")
            packet = self.page.evaluate("wires.at(-1).sent[0]")
            self.assertEqual(packet["contentVersion"], 7)
            self.assertEqual(packet["lessonTaskId"], "prepared")
            self.assertEqual(packet["questionId"], "fictional-q")
            self.assertIn("因为", packet["question"])
            self.page.evaluate("wires.at(-1).event({type:'delta',text:'**你写出了数量差。** $3-2=1$'})")
            self.page.wait_for_function("document.querySelector('.ai-dialogue-history strong')")
            self.page.evaluate("wires.at(-1).event({type:'done',version:3})")
            self.assertEqual(row.locator("textarea").count(), 1)
            self.assertEqual(row.get_by_role("button", name="请求讲解").count(), 1)
            self.assertEqual(row.get_by_role("button", name="跳过本题").count(), 1)

    def test_drafts_are_isolated_by_prepared_lesson(self):
        self.open()
        self.question()
        self.page.locator("textarea").fill("还没发送的推导")
        self.page.reload()
        self.question()
        self.assertEqual(self.page.locator("textarea").input_value(), "还没发送的推导")
        self.page.goto("http://fictional.local/learn?courseId=fictional&lessonTaskId=another")
        self.page.evaluate("document.getElementById('fixture').append(GangyiLearning.renderDialogue('quiz',{question:'比较',questionId:'fictional-q',contentVersion:7},0,{courseId:'fictional',phaseIndex:1,topicIndex:1,lessonTaskId:'another'}))")
        self.assertEqual(self.page.locator("textarea").input_value(), "")

    def test_review_drafts_are_isolated_by_review_task_and_day(self):
        self.open()
        add = """scope => { const host=document.getElementById('fixture'); host.replaceChildren();
          host.append(GangyiLearning.renderDialogue('review', {question:'比较 2 与 3',
            questionId:'same-review-question',contentVersion:7,index:0}, 0,
            {courseId:'fictional',phaseIndex:1,topicIndex:1,...scope})); }"""
        self.page.evaluate(add, {'reviewId': 'review-a', 'day': 1})
        self.page.locator('textarea').fill('本轮复习未发送的过程')
        self.page.evaluate(add, {'reviewId': 'review-b', 'day': 1})
        self.assertEqual('', self.page.locator('textarea').input_value())
        self.page.evaluate(add, {'reviewId': 'review-a', 'day': 3})
        self.assertEqual('', self.page.locator('textarea').input_value())
        self.page.evaluate(add, {'reviewId': 'review-a', 'day': 1})
        self.assertEqual('本轮复习未发送的过程', self.page.locator('textarea').input_value())

    def test_retry_keeps_request_identity_and_multiround_remains_open(self):
        self.open()
        self.question()
        self.page.locator('textarea').fill('我认为 3 更大')
        self.page.get_by_role('button', name='发送给 AI').click()
        self.page.wait_for_function('wires.at(-1)?.sent?.length')
        original = self.page.evaluate('wires.at(-1).sent[0]')
        self.dialogue = {"questionId": "fictional-q", "version": 3, "contentVersion": 7,
                        "evaluationStatus": "waiting", "turns": [{"user": "我认为 3 更大",
                        "status": "failed", "requestId": original['requestId'], "intent": "answer"}]}
        self.page.evaluate("wires.at(-1).event({type:'error',message:'模型请求失败'})")
        self.page.get_by_role('button', name='重试本次回答').click()
        self.page.wait_for_function('wires.length===2 && wires.at(-1)?.sent?.length')
        retry = self.page.evaluate('wires.at(-1).sent[0]')
        self.assertEqual(retry['requestId'], original['requestId'])
        self.assertEqual(retry['version'], 3)
        self.dialogue = {"questionId": "fictional-q", "version": 4, "evaluationStatus": "ready",
                        "turns": [{"user": "我认为 3 更大", "assistant": "判断正确，但依据只有结论。", "status": "complete"}]}
        self.page.evaluate("wires.at(-1).event({type:'delta',text:'判断正确，但依据只有结论。'});wires.at(-1).event({type:'done',version:4})")
        self.page.locator('textarea').fill('能再解释差值吗')
        self.page.get_by_role('button', name='发送给 AI').click()
        self.page.wait_for_function('wires.length===3 && wires.at(-1)?.sent?.length')
        followup = self.page.evaluate('wires.at(-1).sent[0]')
        self.assertEqual(followup['intent'], 'ask')
        self.assertNotEqual(followup['requestId'], original['requestId'])

    def test_stop_before_socket_open_preserves_answer(self):
        self.open()
        self.question()
        self.page.locator('textarea').fill('尚未完成的回答')
        self.page.get_by_role('button', name='发送给 AI').click()
        self.page.get_by_role('button', name='停止回答').click()
        self.page.wait_for_function("wires.at(-1)?.sent?.some(p=>p.type==='stop')")
        self.page.evaluate("wires.at(-1).event({type:'done',cancelled:true,version:2})")
        self.assertIn('尚未完成', self.page.locator('textarea').input_value())
        self.assertTrue(self.page.get_by_role('button', name='发送给 AI').is_enabled())

    def test_help_buttons_send_current_teaching_preference(self):
        self.open()
        for label, preference in [('请求讲解', '只给提示，不要公布答案'), ('暂时不会', '请从定义开始，语速慢一些')]:
            self.page.locator('#fixture').evaluate('node=>node.replaceChildren()')
            self.question()
            row = self.page.locator('.ai-dialogue')
            row.locator('textarea').fill(preference)
            row.get_by_role('button', name=label, exact=True).click()
            self.page.wait_for_function("wires.at(-1)?.sent?.[0]?.type==='ask'")
            request = self.page.evaluate('wires.at(-1).sent[0]')
            self.assertIn(preference, request['question'])
            self.assertIn(preference, request['answer'])

    def test_partial_teaching_restores_without_reliable_success(self):
        self.dialogue = {'questionId': 'fictional-q', 'version': 3, 'contentVersion': 7,
            'evaluationStatus': 'waiting', 'assisted': True, 'turns': [{'user': '我的答案是 2',
                'partialAssistant': '**先比较差值**，这段提示已经展示过。<script>window.partialInjected=true</script>',
                'status': 'cancelled', 'requestId': 'stopped-attempt', 'intent': 'answer'}]}
        self.open(); self.question()
        row = self.page.locator('.ai-dialogue')
        row.locator('.ai-dialogue-history strong').wait_for()
        self.assertIn('先比较差值', row.locator('.ai-dialogue-history').inner_text())
        self.assertIn('未完成', row.locator('.ai-dialogue-history').inner_text())
        self.assertFalse(self.page.evaluate('window.partialInjected || false'))
        self.assertTrue(row.get_by_role('button', name='重试本次回答').is_visible())
        self.assertIn('之前的可靠结果保留', row.locator('.classroom-feedback').inner_text())
        self.assertNotIn('完整评价和讲解已保存', row.inner_text())

    def test_retry_restores_previous_partial_replies_without_duplicating_ready(self):
        self.dialogue = {'questionId': 'fictional-q', 'version': 4, 'contentVersion': 7,
            'evaluationStatus': 'pending', 'turns': [{'user': '我的答案是 2', 'status': 'pending',
                'requestId': 'same-attempt', 'previousPartialReplies': [{'text': '**此前提示**：先检查符号', 'incomplete': True}],
                'partialAssistant': '这一轮继续比较差值', 'incomplete': True}]}
        self.open(); self.question()
        row = self.page.locator('.ai-dialogue')
        row.locator('.ai-dialogue-history strong').wait_for(timeout=2000)
        self.assertIn('此前提示', row.locator('.ai-dialogue-history').inner_text())
        self.assertIn('这一轮继续比较', row.locator('.ai-dialogue-history').inner_text())
        self.assertIn('未完成', row.locator('.ai-dialogue-history').inner_text())
        self.dialogue['evaluationStatus'] = 'ready'
        self.dialogue['turns'][0]['status'] = 'ready'
        self.dialogue['turns'][0]['assistant'] = '已经完成的完整回复'
        self.page.reload(); self.question()
        row = self.page.locator('.ai-dialogue')
        row.locator('.ai-dialogue-history').get_by_text('已经完成的完整回复', exact=True).wait_for()
        self.assertNotIn('此前提示', row.locator('.ai-dialogue-history').inner_text())

    def test_choice_only_retry_preserves_numeric_answer(self):
        self.open()
        self.question()
        self.page.locator('input[value="1"]').check()
        self.page.get_by_role('button', name='发送给 AI').click()
        self.page.wait_for_function('wires.at(-1)?.sent?.length')
        original = self.page.evaluate('wires.at(-1).sent[0]')
        self.assertEqual(original['answer'], 1)
        self.dialogue = {'questionId': 'fictional-q', 'version': 3, 'evaluationStatus': 'waiting',
            'turns': [{'user': '3 更大', 'givenAnswer': 1, 'selectedOption': 1, 'requestId': original['requestId'], 'status': 'failed'}]}
        self.page.evaluate("wires.at(-1).event({type:'error',message:'失败'})")
        self.page.get_by_role('button', name='重试本次回答').click()
        self.page.wait_for_function('wires.length===2 && wires.at(-1)?.sent?.length')
        self.assertEqual(self.page.evaluate('wires.at(-1).sent[0].answer'), 1)

    def test_reused_question_radio_groups_are_independent(self):
        self.open()
        self.question('diagnostic')
        self.question('quiz')
        rows = self.page.locator('.ai-dialogue')
        rows.nth(0).locator('input[value="0"]').check()
        rows.nth(1).locator('input[value="1"]').check()
        self.assertTrue(rows.nth(0).locator('input[value="0"]').is_checked())
        self.assertTrue(rows.nth(1).locator('input[value="1"]').is_checked())

    def test_preparation_sequence_cancel_restore_and_ready(self):
        self.open("<main id='next-lesson-page'><p id='next-lesson-status'></p><div id='next-lesson-blocks'></div><button id='next-lesson-stop'>停止备课</button><button id='next-lesson-retry'>重新准备</button><a id='next-lesson-return'></a></main>", "/learn/next?id=fictional-task")
        self.page.wait_for_function("wires.at(-1)?.sent?.[0]?.type==='subscribe'")
        self.page.evaluate("wires.at(-1).event({seq:1,type:'stage',stage:'decision'})")
        self.page.evaluate("wires.at(-1).event({seq:2,type:'delta',stage:'decision',text:'根据本次回答补讲数量差'})")
        self.page.evaluate("wires.at(-1).event({seq:2,type:'delta',stage:'decision',text:'重复文字不应出现'})")
        self.assertNotIn("重复文字", self.page.locator("#next-lesson-blocks").inner_text())
        self.assertIn("数量差", self.page.locator("#next-lesson-blocks").inner_text())
        self.page.evaluate("wires.at(-1).event({seq:3,type:'cancelled',stage:'decision',text:'已停止'})")
        self.assertIn("/learn/next", self.page.url)
        self.assertTrue(self.page.locator("#next-lesson-retry").is_visible())

    def test_next_waits_for_submitted_question_socket_before_navigation(self):
        self.open("<main id='fixture'></main><button id='lesson-next'>让 AI 准备下一课</button><p id='next-lesson-action-status'></p>")
        self.question()
        row = self.page.locator('.ai-dialogue')
        row.locator('textarea').fill('已经提交的解题过程')
        row.locator('[data-submit]').click()
        self.page.wait_for_function("wires.at(-1)?.sent?.[0]?.type==='ask'")
        self.page.locator('#lesson-next').click()
        self.page.wait_for_function("document.getElementById('lesson-next').disabled")
        self.page.wait_for_timeout(100)
        self.assertNotIn('/learn/next', self.page.url)
        self.assertEqual(1, self.page.evaluate('wires.at(-1).readyState'))
        self.assertEqual(1, sum('/api/learn/next' in url for url, _ in self.requests))
        self.page.evaluate("wires.at(-1).event({type:'done',version:3})")
        self.page.wait_for_url('**/learn/next?id=fictional-task')

    def test_ready_is_the_only_preparation_navigation(self):
        self.open("<main id='next-lesson-page'><p id='next-lesson-status'></p><div id='next-lesson-blocks'></div><button id='next-lesson-stop'></button><button id='next-lesson-retry'></button><a id='next-lesson-return'></a></main>", "/learn/next?id=fictional-task")
        self.page.wait_for_function('wires.at(-1)?.sent?.length')
        self.page.evaluate("wires.at(-1).event({seq:1,type:'block_complete',stage:'quiz',block:{quiz:[{question:'比较',options:['2','3'],answerIndex:1,explanation:'隐藏答案'}]}})")
        self.assertIn('/learn/next', self.page.url)
        self.assertNotIn('隐藏答案', self.page.locator('#next-lesson-blocks').inner_text())
        self.page.evaluate("wires.at(-1).event({seq:2,type:'ready',stage:'assessment',classroomUrl:'/learn?courseId=fictional&lessonTaskId=prepared'})")
        self.page.wait_for_url('**/learn?courseId=fictional&lessonTaskId=prepared')

    def test_stale_snapshot_stays_on_page_and_retry_continues_sequence(self):
        self.task_view = {'id': 'fictional-task', 'status': 'stale', 'latestSeq': 1, 'blocks': {},
            'message': '学习版本已变化，请重新准备', 'sourceUrl': '/learn?courseId=fictional',
            'events': [{'seq': 1, 'type': 'failed', 'stage': 'assessment', 'stale': True,
                        'message': '学习版本已变化，请重新准备'}]}
        self.open("<main id='next-lesson-page'><p id='next-lesson-status'></p><div id='next-lesson-blocks'></div><button id='next-lesson-stop'></button><button id='next-lesson-retry'>重新准备</button><a id='next-lesson-return'></a></main>", "/learn/next?id=fictional-task")
        self.page.locator('#next-lesson-retry').wait_for(state='visible')
        self.assertIn('/learn/next', self.page.url)
        self.assertIn('版本已变化', self.page.locator('#next-lesson-status').inner_text())
        self.assertEqual('/learn?courseId=fictional', self.page.locator('#next-lesson-return').get_attribute('href'))
        self.task_view = {'id': 'fictional-task', 'status': 'running', 'latestSeq': 2, 'blocks': {},
            'events': [{'seq': 2, 'type': 'stage', 'stage': 'decision', 'reset': True}]}
        self.page.locator('#next-lesson-retry').click()
        self.page.wait_for_function("wires.at(-1)?.sent?.[0]?.afterSeq===2")
        self.assertEqual(1, sum('/retry' in url for url, _ in self.requests))
        self.page.evaluate("wires.at(-1).event({seq:3,type:'delta',stage:'decision',text:'依据最新回答重新备课'})")
        self.assertIn('最新回答', self.page.locator('#next-lesson-blocks').inner_text())
        self.assertIn('/learn/next', self.page.url)

    def test_saved_lesson_renderer_loads_public_questions_without_regeneration(self):
        source = (ROOT / 'src/page_renderer.cpp').read_text(encoding='utf-8').split('std::string renderLearnPage', 1)[1].split('std::string renderNextLessonPage', 1)[0]
        body_code, scripts_code = source.split('    const std::string script = ', 1)
        body = ''.join(re.findall(r'R"HTML\((.*?)\)HTML"', body_code, re.S))
        script_code, enhancements = scripts_code.split('    const std::string identityBridge', 1)
        values = {'courseId': 'fictional', 'phaseIndex': '1', 'topicIndex': '1'}
        script_code = re.sub(r'\)HTML"\+jsString\((courseId|phaseIndex|topicIndex)\)\+R"HTML\(', lambda m: json.dumps(values[m[1]]), script_code)
        scripts = ''.join(re.findall(r'R"HTML\((.*?)\)HTML"', script_code + enhancements, re.S))
        # 真实页面以 defer 按 learning-flow、classroom 顺序加载，避免验收夹具提前运行课堂脚本。
        self.open(body + scripts + "<script defer src='/classroom.js'></script>")
        self.page.wait_for_function("document.querySelectorAll('.ai-dialogue').length===5")
        self.assertEqual(self.errors, [])
        self.assertEqual(self.page.locator('#quiz-submit').count(), 0)
        self.assertEqual(self.page.get_by_role('button', name='让 AI 准备下一课').count(), 1)
        self.assertEqual(self.page.locator('#learn-examples .ai-dialogue textarea').count(), 1)
        self.assertTrue(self.page.locator('#regenerate').is_hidden())
        learn_requests = [url for url, _ in self.requests if '/api/learn?' in url]
        self.assertTrue(all('lessonTaskId=prepared' in url and 'regenerate=' not in url for url in learn_requests))

    def test_rendered_page_has_no_answer_or_direct_next_jump(self):
        source = (ROOT / "src/page_renderer.cpp").read_text(encoding="utf-8")
        lesson = source.split("std::string renderLearnPage", 1)[1].split("#if 0", 1)[0]
        self.assertNotIn("item.solution", lesson)
        self.assertNotIn("item.check", lesson)
        self.assertNotIn("继续下一节 →", lesson)
        self.assertIn("lessonTaskId", lesson)
        self.assertIn("renderNextLessonPage", source)


if __name__ == "__main__":
    unittest.main()

"""深度课堂隔离浏览器验收；夹具只验证呈现和协议，不代表真实 AI 教学能力。"""
import argparse
import copy
import json
import mimetypes
import os
from pathlib import Path
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlsplit

from playwright.sync_api import sync_playwright


PUBLIC = Path(__file__).resolve().parents[1] / 'public'
FIXTURE = {
    'enabled': True,
    'course': {'id': 'fixture-course', 'title': '函数与变化', 'goal': '理解函数图像与参数变化'},
    'outline': {'courseStructure': [{'stage': '函数与图像', 'topics': [
        {'id': 'topic-one', 'title': '函数的单调性', 'legacyPhaseIndex': 1, 'legacyTopicIndex': 1},
        {'id': 'topic-two', 'title': '参数与曲线', 'legacyPhaseIndex': 3, 'legacyTopicIndex': 7}]}]},
    'lesson': {
        'id': 'fixture-lesson', 'courseId': 'fixture-course', 'title': '函数的单调性',
        'purpose': '从自变量与函数值的变化理解单调性。', 'phaseIndex': 1, 'topicIndex': 1,
        'topicId': 'topic-one', 'status': 'ready', 'entered': False, 'initialTeachingTaskId': 'saved-initial-task',
        'sections': [{'id': 'lecture-one', 'kind': 'explanation', 'version': 1, 'body': '对于 $f(x)=2x+1$，自变量增大时，函数值也会增大。\n\n<script>window.fixtureUnsafe=true</script>'},
                     {'id': 'summary-one', 'kind': 'summary', 'version': 1, 'body': '比较任意两个自变量对应的函数值。'},
                     {'id': 'question-one', 'questionId': 'stable-question-one', 'kind': 'question', 'version': 2, 'title': '一起判断',
                      'question': {'type': 'single_choice', 'question': '当 x 从 1 增至 2，f(x)=2x+1 如何变化？',
                                   'options': ['从 3 变成 5', '从 5 变成 3']}}],
        'timeline': [], 'evaluations': [], 'teachingFocus': {'practiceSectionId': 'question-one'}
    },
    'content': {
        'version': 1,
        'lecture': [{'id': 'lecture-one', 'sectionId': 'lecture-one', 'title': '观察函数的变化',
                     'body': '对于 $f(x)=2x+1$，自变量增大时，函数值也会增大。\n\n<script>window.fixtureUnsafe=true</script>',
                     'mediaIds': ['curve', 'timeline', 'forces', 'failed-image']}],
        'slides': [{'id': 'slide-one', 'title': '先观察，再判断', 'sourceSectionIds': ['lecture-one'], 'body': '对于 $f(x)=2x+1$，自变量增大时，函数值也会增大。\n\n<script>window.fixtureUnsafe=true</script>', 'mediaIds': ['curve']},
                   {'id': 'slide-two', 'title': '用定义说明', 'sourceSectionIds': ['summary-one'], 'body': '比较任意两个自变量对应的函数值。', 'mediaIds': []}],
        'media': [
            {'id': 'curve', 'type': 'function', 'title': '改变斜率，观察曲线', 'status': 'ready',
             'coefficients': [0, 2, 1], 'xMin': -3, 'xMax': 3,
             'parameter': {'name': '斜率 k', 'min': -4, 'max': 4, 'step': 0.5, 'value': 2, 'coefficientIndex': 1}},
            {'id': 'timeline', 'type': 'timeline', 'title': '理解的三个步骤', 'status': 'ready',
             'events': [{'label': '观察', 'text': '比较函数值'}, {'label': '归纳', 'text': '说明变化规律'}]},
            {'id': 'forces', 'type': 'forces', 'title': '给定力的一维演示', 'status': 'ready', 'mass': 2, 'force': 10, 'friction': 2},
            {'id': 'failed-image', 'type': 'image', 'title': '参考图片', 'status': 'failed', 'error': '图片暂未读取完成。'}]
    }
}


class FixtureServer(BaseHTTPRequestHandler):
    data = copy.deepcopy(FIXTURE)
    requests = []
    reads = []
    retry_mode = 'immediate'
    media_tasks = {}
    overlapping_retry = False
    premature_exposure = False
    unprepared_topics = set()

    @classmethod
    def complete_media(cls, media_id):
        media = next(item for item in cls.data['content']['media'] if item['id'] == media_id)
        media.update({'type': 'timeline', 'status': 'ready', 'events': [{'label': '已重试', 'text': '独立恢复素材'}]})

    def log_message(self, *_):
        pass

    def reply(self, value, status=200):
        data = json.dumps(value, ensure_ascii=False).encode('utf-8')
        self.send_response(status)
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        path = urlsplit(self.path).path
        if path == '/api/deep-course':
            type(self).reads.append(self.path)
            topic_id = parse_qs(urlsplit(self.path).query).get('topicId', [''])[0]
            if topic_id in type(self).unprepared_topics:
                value = copy.deepcopy(type(self).data)
                value.update({'lesson': None, 'content': None, 'status': 'unprepared'})
                return self.reply(value)
            return self.reply(type(self).data)
        if path == '/api/learning-agent':
            task_id = parse_qs(urlsplit(self.path).query).get('taskId', [''])[0]
            if task_id in type(self).media_tasks:
                task = type(self).media_tasks[task_id]
                task['polls'] -= 1
                if task['polls'] <= 0 and task['status'] == 'pending':
                    task['status'] = 'ready'
                    type(self).complete_media(task['mediaId'])
                return self.reply(task)
            return self.reply({'id': 'fixture-task', 'status': 'ready', 'events': []})
        if path == '/deep':
            data = '''<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><link rel="stylesheet" href="/deep-course.css"><style>html,body{margin:0}.site-header{height:72px;display:flex;align-items:center;padding:0 24px;border-bottom:1px solid #dfe6ef;font:15px 'Microsoft YaHei',sans-serif}.site-header img{width:34px;height:34px;margin-right:11px}:root{--gy-header-height:72px}@media(max-width:700px){.site-header{height:64px}:root{--gy-header-height:64px}}</style></head><body class="clarity-app"><header class="site-header"><img src="/school-logo.png" alt="柳州市钢一中学校徽">钢一定制 AI</header><main id="deep-course-app"></main><script src="/chat-render.js"></script><script src="/learning-agent.js"></script><script src="/deep-course.js"></script></body></html>'''.encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.end_headers()
            self.wfile.write(data)
            return
        target = (PUBLIC / path.lstrip('/')).resolve()
        if target.is_relative_to(PUBLIC.resolve()) and target.is_file():
            self.send_response(200)
            self.send_header('Content-Type', mimetypes.guess_type(target.name)[0] or 'application/octet-stream')
            self.end_headers()
            self.wfile.write(target.read_bytes())
            return
        self.send_error(404)

    def do_POST(self):
        path = urlsplit(self.path).path
        value = json.loads(self.rfile.read(int(self.headers.get('Content-Length', 0))))
        type(self).requests.append({'path': path, 'body': value})
        if path == '/api/learn/exposure' and not type(self).data['lesson']['entered']:
            type(self).premature_exposure = True
        if path == '/api/learning-agent/control' and value.get('command') == 'enter_lesson':
            type(self).data['lesson']['entered'] = True
        if path == '/api/deep-course/start':
            type(self).data = copy.deepcopy(FIXTURE)
            return self.reply({'lessonId': 'fixture-lesson', 'cached': True})
        if path == '/api/deep-course/retry-media':
            if type(self).retry_mode != 'immediate':
                type(self).overlapping_retry |= any(task['status'] == 'pending' for task in type(self).media_tasks.values())
                task_id = 'media-task-' + str(len(type(self).media_tasks) + 1)
                task = {'id': task_id, 'status': 'paused' if type(self).retry_mode == 'paused' else 'pending', 'events': [], 'polls': 4, 'mediaId': value['mediaId']}
                type(self).media_tasks[task_id] = task
                return self.reply({'task': task})
            type(self).complete_media(value['mediaId'])
        if path == '/api/learning-agent/events':
            type(self).data['lesson']['timeline'].append({'type': 'message', 'role': 'user', 'taskId': 'fixture-task', 'text': value['text']})
            if value['type'] == 'question_answer':
                section = next(item for item in type(self).data['lesson']['sections'] if item['id'] == value['sectionId'])
                type(self).data['lesson']['evaluations'].append({
                    'id': 'evaluation-' + value['requestId'], 'interactionId': 'input-' + value['requestId'],
                    'sectionId': section['id'], 'questionId': section.get('questionId', ''),
                    'credible': True, 'score': 80, 'feedback': '已保存这次实际作答的评价。'})
        return self.reply({'task': {'id': 'fixture-task', 'status': 'ready', 'events': []}} if path.startswith('/api/deep-course/') else {'id': 'fixture-task', 'status': 'ready', 'events': []})


def run(output):
    output.mkdir(parents=True, exist_ok=True)
    server = ThreadingHTTPServer(('127.0.0.1', 0), FixtureServer)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    errors = []
    try:
        with sync_playwright() as playwright:
            executable = os.environ.get('BROWSER_EXECUTABLE')
            edge = Path('C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe')
            if not executable and edge.is_file():
                executable = str(edge)
            browser = playwright.chromium.launch(headless=True, **({'executable_path': executable} if executable else {}))
            context = browser.new_context(viewport={'width': 1280, 'height': 720}, reduced_motion='reduce')
            tab = context.new_page()
            tab.on('pageerror', lambda error: errors.append(str(error)))
            tab.goto(f'http://127.0.0.1:{server.server_port}/deep?lessonId=fixture-lesson')
            tab.add_style_tag(url='/clarity.css')
            tab.locator('.dc-lecture-section h2').wait_for()
            tab.wait_for_function("!document.getElementById('dc-continue').disabled")
            assert [entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/control' and entry['body']['command'] == 'enter_lesson']
            assert not [entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/start']
            assert tab.locator('#dc-title').inner_text() == '函数的单调性'
            assert tab.evaluate('window.fixtureUnsafe') is None
            tab.locator('[data-action="view:slides"]').click()
            assert tab.locator('.dc-slide h2').inner_text() == '先观察，再判断'
            assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == 'lecture-one'
            tab.locator('[data-action="next"]').click()
            assert tab.locator('.dc-slide h2').inner_text() == '用定义说明'
            assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == 'summary-one'
            tab.locator('[data-action="view:lecture"]').click()
            curve = tab.locator('[data-media-id="curve"] input[type=range]')
            curve.focus()
            curve.press('ArrowRight')
            assert '2.5 x' in tab.locator('[data-media-id="curve"] svg').get_attribute('aria-label')
            assert '合力 8 N' in tab.locator('[data-media-id="forces"]').inner_text()
            force = tab.locator('[data-media-id="forces"] input[type=range]')
            force.evaluate("node => { node.value = '1'; node.dispatchEvent(new Event('input', {bubbles:true})); }")
            assert '合力 0 N' in tab.locator('[data-media-id="forces"]').inner_text()
            force.evaluate("node => { node.value = '-4'; node.dispatchEvent(new Event('input', {bubbles:true})); }")
            assert '合力 -2 N' in tab.locator('[data-media-id="forces"]').inner_text()
            force.evaluate("node => { node.value = '10'; node.dispatchEvent(new Event('input', {bubbles:true})); }")
            assert not [request for request in FixtureServer.requests if request['path'] == '/api/learning-agent/events']
            tab.locator('[data-media-id="failed-image"] [data-action=media]').click()
            tab.locator('[data-media-id="failed-image"] .dc-timeline').wait_for()
            assert len([request for request in FixtureServer.requests if request['path'] == '/api/deep-course/retry-media']) == 1
            tab.locator('#dc-chat-toggle').click()
            tab.locator('#dc-chat-input').fill('请用 <script> 举例，但不要执行。')
            tab.locator('#dc-chat-input').press('Enter')
            tab.wait_for_function("document.getElementById('dc-chat-input').value === ''")
            assert [request for request in FixtureServer.requests if request['path'] == '/api/learning-agent/events'][-1]['body']['type'] == 'chat'
            assert [request for request in FixtureServer.requests if request['path'] == '/api/learning-agent/events'][-1]['body']['deepView'] == 'lecture'
            tab.locator('[data-action=close-chat]').click()
            tab.locator('[data-question-id="question-one"] input[value="0"]').check()
            tab.locator('[data-question-id="question-one"] textarea').fill('函数值增加了 2。')
            tab.locator('[data-question-id="question-one"] button[type=submit]').click()
            tab.wait_for_function("document.querySelector('[data-question-id=question-one] textarea').value === ''")
            answer = [request for request in FixtureServer.requests if request['path'] == '/api/learning-agent/events'][-1]['body']
            assert answer['type'] == 'question_answer' and answer['sectionVersion'] == 2 and answer['view'] == 'practice'
            assert '从 3 变成 5' in answer['text'] and '函数值增加了 2' in answer['text']
            checks = []
            for width, height in [(1440, 900), (1280, 720), (390, 844)]:
                tab.set_viewport_size({'width': width, 'height': height})
                tab.locator('#dc-reading').evaluate('node => { node.scrollTop = 0; }')
                state = tab.evaluate('''() => ({width:innerWidth, scrollWidth:document.documentElement.scrollWidth, footerBottom:document.querySelector('.dc-footer').getBoundingClientRect().bottom, viewportHeight:innerHeight, logo:document.querySelector('.site-header img').getBoundingClientRect().width})''')
                assert state['scrollWidth'] <= width + 1, state
                assert state['footerBottom'] <= height + 1, state
                assert state['logo'] > 0, state
                checks.append(state)
                tab.screenshot(path=str(output / f'deep-course-{width}.png'), full_page=True)
            tab.locator('#dc-directory-toggle').click()
            assert tab.locator('#dc-outline-dialog').is_visible()
            tab.keyboard.press('Escape')
            assert not tab.locator('#dc-outline-dialog').is_visible()
            tab.locator('#dc-chat-toggle').click()
            assert tab.locator('#dc-chat-dialog').is_visible()
            assert tab.locator('#dc-chat-input').is_visible()
            tab.screenshot(path=str(output / 'deep-course-390-chat.png'), full_page=True)
            tab.keyboard.press('Escape')
            assert not tab.locator('#dc-chat-dialog').is_visible()
            before_reads = len(FixtureServer.reads)
            tab.goto(f'http://127.0.0.1:{server.server_port}/deep?courseId=fixture-course&phaseIndex=1.5&topicIndex=2')
            assert '课程位置无效' in tab.locator('#dc-status').inner_text()
            assert len(FixtureServer.reads) == before_reads
            FixtureServer.data = copy.deepcopy(FIXTURE)
            FixtureServer.data['lesson'] = None
            FixtureServer.data['content'] = None
            tab.goto(f'http://127.0.0.1:{server.server_port}/deep?courseId=fixture-course&phaseIndex=3&topicIndex=7&topicId=topic-one')
            tab.locator('[data-action=start]').click()
            tab.wait_for_function("!document.getElementById('dc-continue').disabled")
            started = [entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/start'][-1]['body']
            assert started['phaseIndex'] == 3 and started['topicIndex'] == 7 and started['topicId'] == 'topic-one'
            assert not FixtureServer.premature_exposure
            FixtureServer.data['outline']['courseStructure'][0]['topics'][0].update({'legacyPhaseIndex': 3, 'legacyTopicIndex': 7})
            tab.reload()
            tab.locator('#dc-directory-toggle').click()
            topic = tab.locator('#dc-outline-dialog [data-topic-id="topic-one"]')
            assert topic.get_attribute('data-phase-index') == '3' and topic.get_attribute('data-topic-index') == '7'
            tab.keyboard.press('Escape')
            FixtureServer.data['content']['media'] = [
                {'id': 'failed-one', 'type': 'timeline', 'title': '素材一', 'status': 'failed'},
                {'id': 'failed-two', 'type': 'timeline', 'title': '素材二', 'status': 'failed'}]
            FixtureServer.data['content']['lecture'][0]['mediaIds'] = ['failed-one', 'failed-two']
            FixtureServer.retry_mode = 'delayed'
            before = len(FixtureServer.requests)
            tab.reload()
            tab.locator('.dc-more>summary').click()
            tab.locator('[data-action=retry-media]').click()
            tab.wait_for_function("document.querySelectorAll('.dc-timeline').length === 2")
            assert not FixtureServer.overlapping_retry
            retries = [entry for entry in FixtureServer.requests[before:] if entry['path'] == '/api/deep-course/retry-media']
            assert [entry['body']['mediaId'] for entry in retries] == ['failed-one', 'failed-two']
            for media in FixtureServer.data['content']['media']:
                media['status'] = 'failed'
            FixtureServer.retry_mode = 'paused'
            before = len(FixtureServer.requests)
            tab.reload()
            tab.locator('.dc-more>summary').click()
            tab.locator('[data-action=retry-media]').click()
            tab.wait_for_function("document.getElementById('dc-status').textContent.includes('暂停')")
            tab.wait_for_timeout(500)
            retries = [entry for entry in FixtureServer.requests[before:] if entry['path'] == '/api/deep-course/retry-media']
            assert len(retries) == 1
            FixtureServer.retry_mode = 'immediate'
            FixtureServer.data['content']['media'] = [{'id': 'broken-ready', 'type': 'image', 'title': '已失效缓存图片', 'status': 'ready', 'url': '/missing-image.png'}]
            FixtureServer.data['content']['lecture'][0]['mediaIds'] = ['broken-ready']
            tab.reload()
            tab.get_by_role('button', name='重试图片', exact=True).click()
            tab.locator('.dc-timeline').wait_for()
            repaired = [entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/retry-media'][-1]['body']
            assert repaired['mediaId'] == 'broken-ready' and repaired['repair'] is True

            # 真实评价改变继续路径；前端不能把阅读、点击或分数直接变成完成记录。
            FixtureServer.data = copy.deepcopy(FIXTURE)
            FixtureServer.data['lesson']['entered'] = True
            FixtureServer.data['content']['media'] = FixtureServer.data['content']['media'][:1]
            FixtureServer.data['content']['lecture'][0]['mediaIds'] = ['curve']
            tab.set_viewport_size({'width': 1280, 'height': 720})
            tab.goto(f'http://127.0.0.1:{server.server_port}/deep?lessonId=fixture-lesson')
            tab.wait_for_function("!document.getElementById('dc-continue').disabled")
            before_events = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'])
            tab.locator('[data-action="view:slides"]').click()
            tab.locator('#dc-continue').click()
            assert tab.locator('#dc-tab-lecture').get_attribute('aria-selected') == 'true'
            assert tab.evaluate("document.activeElement.closest('[data-question-id]')?.dataset.questionId") == 'question-one'
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events']) == before_events
            tab.locator('[data-question-id="question-one"] input[value="0"]').check()
            tab.locator('[data-question-id="question-one"] textarea').fill('我比较了两个函数值。')
            tab.locator('[data-question-id="question-one"] button[type=submit]').click()
            tab.locator('.dc-question[data-evaluated="true"] .dc-evaluation').wait_for()
            tab.wait_for_function("!document.getElementById('dc-continue').disabled")
            tab.locator('#dc-continue').click()
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.chatOpen === 'true' && !document.getElementById('dc-continue').disabled")
            assert [entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'][-1]['body']['type'] == 'lesson_finish_request'
            assert not FixtureServer.data['lesson'].get('completed', False)
            assert tab.locator('#dc-continue').inner_text() == '继续学习'

            second_question = copy.deepcopy(FixtureServer.data['lesson']['sections'][-1])
            second_question.update({'id': 'question-two', 'questionId': 'stable-question-two', 'title': '关联新题'})
            FixtureServer.data['lesson']['sections'].append(second_question)
            FixtureServer.data['lesson']['evaluations'].append({
                'id': 'discarded-evaluation', 'interactionId': 'discarded-input', 'sectionId': 'question-two',
                'questionId': 'stable-question-two', 'status': 'discarded_followup', 'credible': True, 'score': 100,
                'feedback': '已撤销的追问不作为作答评价。'})
            tab.reload()
            tab.wait_for_function("!document.getElementById('dc-continue').disabled")
            before_events = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'])
            tab.locator('#dc-continue').click()
            assert tab.evaluate("document.activeElement.closest('[data-question-id]')?.dataset.questionId") == 'question-two'
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events']) == before_events
            assert tab.locator('.dc-question').first.get_attribute('data-evaluated') == 'true'

            # 需要澄清的有效评价也交回主控，不能在前端用通过线拦住或强行通过。
            FixtureServer.data['lesson']['evaluations'][-1].update({'status': 'valid', 'credible': False, 'score': None})
            tab.reload()
            tab.wait_for_function("!document.getElementById('dc-continue').disabled")
            tab.locator('#dc-continue').click()
            tab.wait_for_function("!document.getElementById('dc-continue').disabled")
            assert [entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'][-1]['body']['type'] == 'lesson_finish_request'
            assert not FixtureServer.data['lesson'].get('completed', False)

            # 模拟既有主控随后保存的完成结果；导航不触发下一课付费生成。
            FixtureServer.data['lesson']['completed'] = True
            FixtureServer.unprepared_topics = {'topic-two'}
            tab.reload()
            tab.get_by_role('button', name='下一知识点：参数与曲线', exact=True).wait_for()
            for width, height in [(1280, 720), (390, 844)]:
                tab.set_viewport_size({'width': width, 'height': height})
                state = tab.evaluate("() => ({width:innerWidth,contentWidth:document.documentElement.scrollWidth,footerBottom:document.querySelector('.dc-footer').getBoundingClientRect().bottom,viewportHeight:innerHeight})")
                assert state['contentWidth'] <= width + 1 and state['footerBottom'] <= height + 1, state
                tab.screenshot(path=str(output / f'deep-course-completed-{width}.png'), full_page=True)
            starts_before = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/start'])
            events_before = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'])
            tab.locator('#dc-continue').click()
            tab.locator('[data-action=start]').wait_for()
            target = parse_qs(urlsplit(tab.url).query)
            assert target['topicId'] == ['topic-two'] and target['phaseIndex'] == ['3'] and target['topicIndex'] == ['7']
            assert 'lessonId' not in target
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/start']) == starts_before
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events']) == events_before

            FixtureServer.unprepared_topics.clear()
            FixtureServer.data['lesson'].update({'topicId': 'topic-two', 'phaseIndex': 3, 'topicIndex': 7})
            tab.reload()
            tab.get_by_role('button', name='查看课程目录', exact=True).wait_for()
            tab.locator('#dc-continue').click()
            assert tab.locator('#dc-directory-toggle').get_attribute('aria-expanded') == 'true'
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events']) == events_before
            assert not errors, errors
            result = {'fixtureOnly': True, 'checks': checks, 'errors': errors,
                      'continuation': {'unansweredLocated': True, 'evaluatedRequestsController': True,
                                       'newQuestionLocated': True, 'completionUsesSavedState': True,
                                       'nextTopicRequiresExplicitPreparation': True},
                      'events': [request for request in FixtureServer.requests if request['path'] == '/api/learning-agent/events'],
                      'result': 'passed'}
            (output / 'deep-course-verification.json').write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
            print(json.dumps(result, ensure_ascii=False))
            browser.close()
    finally:
        server.shutdown()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('output', nargs='?', type=Path, default=Path('verification/deep-course-browser'))
    run(parser.parse_args().output)

# 【逐行批注】 执行当前 Python 语句："""深度课堂隔离浏览器验收；夹具只验证呈现和协议，不代表真实 AI 教学能力。"""。
"""深度课堂隔离浏览器验收；夹具只验证呈现和协议，不代表真实 AI 教学能力。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import argparse。
import argparse
# 【逐行批注】 导入后续逻辑需要的模块或符号：import copy。
import copy
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import mimetypes。
import mimetypes
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import threading。
import threading
# 【逐行批注】 导入后续逻辑需要的模块或符号：from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer。
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import parse_qs, urlsplit。
from urllib.parse import parse_qs, urlsplit

# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import sync_playwright。
from playwright.sync_api import sync_playwright


# 【逐行批注】 计算右侧结果并赋给 PUBLIC，更新脚本状态。
PUBLIC = Path(__file__).resolve().parents[1] / 'public'
# 【逐行批注】 计算右侧结果并赋给 FIXTURE，更新脚本状态。
FIXTURE = {
    # 【逐行批注】 执行当前 Python 语句：'enabled': True,。
    'enabled': True,
    # 【逐行批注】 执行当前 Python 语句：'course': {'id': 'fixture-course', 'title': '函数与变化', 'goal': '理解函数图像与参数变化'},。
    'course': {'id': 'fixture-course', 'title': '函数与变化', 'goal': '理解函数图像与参数变化'},
    # 【逐行批注】 执行当前 Python 语句：'outline': {'courseStructure': [{'stage': '函数与图像', 'topics': [。
    'outline': {'courseStructure': [{'stage': '函数与图像', 'topics': [
        # 【逐行批注】 执行当前 Python 语句：{'id': 'topic-one', 'title': '函数的单调性', 'legacyPhaseIndex': 1, 'legacyTopicIndex': 1},。
        {'id': 'topic-one', 'title': '函数的单调性', 'legacyPhaseIndex': 1, 'legacyTopicIndex': 1},
        # 【逐行批注】 执行当前 Python 语句：{'id': 'topic-two', 'title': '参数与曲线', 'legacyPhaseIndex': 3, 'legacyTopicIndex': 7}]}]},。
        {'id': 'topic-two', 'title': '参数与曲线', 'legacyPhaseIndex': 3, 'legacyTopicIndex': 7}]}]},
    # 【逐行批注】 执行当前 Python 语句：'lesson': {。
    'lesson': {
        # 【逐行批注】 执行当前 Python 语句：'id': 'fixture-lesson', 'courseId': 'fixture-course', 'title': '函数的单调性',。
        'id': 'fixture-lesson', 'courseId': 'fixture-course', 'title': '函数的单调性',
        # 【逐行批注】 执行当前 Python 语句：'purpose': '从自变量与函数值的变化理解单调性。', 'phaseIndex': 1, 'topicIndex': 1,。
        'purpose': '从自变量与函数值的变化理解单调性。', 'phaseIndex': 1, 'topicIndex': 1,
        # 【逐行批注】 执行当前 Python 语句：'topicId': 'topic-one', 'status': 'ready', 'entered': False, 'initialTeachingTaskId': 'sa…。
        'topicId': 'topic-one', 'status': 'ready', 'entered': False, 'initialTeachingTaskId': 'saved-initial-task',
        # 【逐行批注】 计算右侧结果并赋给 'sections': [{'id': 'lecture-one', 'kind': '…，更新脚本状态。
        'sections': [{'id': 'lecture-one', 'kind': 'explanation', 'version': 1, 'body': '对于 $f(x)=2x+1$，自变量增大时，函数值也会增大。\n\n<script>window.fixtureUnsafe=true</script>'},
                     # 【逐行批注】 执行当前 Python 语句：{'id': 'summary-one', 'kind': 'summary', 'version': 1, 'body': '比较任意两个自变量对应的函数值。'},。
                     {'id': 'summary-one', 'kind': 'summary', 'version': 1, 'body': '比较任意两个自变量对应的函数值。'},
                     # 【逐行批注】 执行当前 Python 语句：{'id': 'question-one', 'questionId': 'stable-question-one', 'kind': 'question', 'version'…。
                     {'id': 'question-one', 'questionId': 'stable-question-one', 'kind': 'question', 'version': 2, 'title': '一起判断',
                      # 【逐行批注】 计算右侧结果并赋给 'question': {'type': 'single_choice', 'quest…，更新脚本状态。
                      'question': {'type': 'single_choice', 'question': '当 x 从 1 增至 2，f(x)=2x+1 如何变化？',
                                   # 【逐行批注】 执行当前 Python 语句：'options': ['从 3 变成 5', '从 5 变成 3']}}],。
                                   'options': ['从 3 变成 5', '从 5 变成 3']}}],
        # 【逐行批注】 执行当前 Python 语句：'timeline': [], 'evaluations': [], 'teachingFocus': {'practiceSectionId': 'question-one'}。
        'timeline': [], 'evaluations': [], 'teachingFocus': {'practiceSectionId': 'question-one'}
    # 【逐行批注】 执行当前 Python 语句：},。
    },
    # 【逐行批注】 执行当前 Python 语句：'content': {。
    'content': {
        # 【逐行批注】 执行当前 Python 语句：'version': 1,。
        'version': 1,
        # 【逐行批注】 执行当前 Python 语句：'lecture': [{'id': 'lecture-one', 'sectionId': 'lecture-one', 'title': '观察函数的变化',。
        'lecture': [{'id': 'lecture-one', 'sectionId': 'lecture-one', 'title': '观察函数的变化',
                     # 【逐行批注】 计算右侧结果并赋给 'body': '对于 $f(x)，更新脚本状态。
                     'body': '对于 $f(x)=2x+1$，自变量增大时，函数值也会增大。\n\n<script>window.fixtureUnsafe=true</script>',
                     # 【逐行批注】 执行当前 Python 语句：'mediaIds': ['curve', 'timeline', 'forces', 'failed-image']}],。
                     'mediaIds': ['curve', 'timeline', 'forces', 'failed-image']}],
        # 【逐行批注】 计算右侧结果并赋给 'slides': [{'id': 'slide-one', 'title': '先观察…，更新脚本状态。
        'slides': [{'id': 'slide-one', 'title': '先观察，再判断', 'sourceSectionIds': ['lecture-one'], 'body': '对于 $f(x)=2x+1$，自变量增大时，函数值也会增大。\n\n<script>window.fixtureUnsafe=true</script>', 'mediaIds': ['curve']},
                   # 【逐行批注】 执行当前 Python 语句：{'id': 'slide-two', 'title': '用定义说明', 'sourceSectionIds': ['summary-one'], 'body': '比较任意两…。
                   {'id': 'slide-two', 'title': '用定义说明', 'sourceSectionIds': ['summary-one'], 'body': '比较任意两个自变量对应的函数值。', 'mediaIds': []}],
        # 【逐行批注】 执行当前 Python 语句：'media': [。
        'media': [
            # 【逐行批注】 执行当前 Python 语句：{'id': 'curve', 'type': 'function', 'title': '改变斜率，观察曲线', 'status': 'ready',。
            {'id': 'curve', 'type': 'function', 'title': '改变斜率，观察曲线', 'status': 'ready',
             # 【逐行批注】 执行当前 Python 语句：'coefficients': [0, 2, 1], 'xMin': -3, 'xMax': 3,。
             'coefficients': [0, 2, 1], 'xMin': -3, 'xMax': 3,
             # 【逐行批注】 执行当前 Python 语句：'parameter': {'name': '斜率 k', 'min': -4, 'max': 4, 'step': 0.5, 'value': 2, 'coefficientI…。
             'parameter': {'name': '斜率 k', 'min': -4, 'max': 4, 'step': 0.5, 'value': 2, 'coefficientIndex': 1}},
            # 【逐行批注】 执行当前 Python 语句：{'id': 'timeline', 'type': 'timeline', 'title': '理解的三个步骤', 'status': 'ready',。
            {'id': 'timeline', 'type': 'timeline', 'title': '理解的三个步骤', 'status': 'ready',
             # 【逐行批注】 执行当前 Python 语句：'events': [{'label': '观察', 'text': '比较函数值'}, {'label': '归纳', 'text': '说明变化规律'}]},。
             'events': [{'label': '观察', 'text': '比较函数值'}, {'label': '归纳', 'text': '说明变化规律'}]},
            # 【逐行批注】 执行当前 Python 语句：{'id': 'forces', 'type': 'forces', 'title': '给定力的一维演示', 'status': 'ready', 'mass': 2, 'fo…。
            {'id': 'forces', 'type': 'forces', 'title': '给定力的一维演示', 'status': 'ready', 'mass': 2, 'force': 10, 'friction': 2},
            # 【逐行批注】 执行当前 Python 语句：{'id': 'failed-image', 'type': 'image', 'title': '参考图片', 'status': 'failed', 'error': '图片…。
            {'id': 'failed-image', 'type': 'image', 'title': '参考图片', 'status': 'failed', 'error': '图片暂未读取完成。'}]
    # 【逐行批注】 执行当前 Python 语句：}。
    }
# 【逐行批注】 执行当前 Python 语句：}。
}


# 【逐行批注】 定义类 FixtureServer，组织相关数据与行为。
class FixtureServer(BaseHTTPRequestHandler):
    # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
    data = copy.deepcopy(FIXTURE)
    # 【逐行批注】 计算右侧结果并赋给 requests，更新脚本状态。
    requests = []
    # 【逐行批注】 计算右侧结果并赋给 reads，更新脚本状态。
    reads = []
    # 【逐行批注】 计算右侧结果并赋给 retry_mode，更新脚本状态。
    retry_mode = 'immediate'
    # 【逐行批注】 计算右侧结果并赋给 media_tasks，更新脚本状态。
    media_tasks = {}
    # 【逐行批注】 计算右侧结果并赋给 overlapping_retry，更新脚本状态。
    overlapping_retry = False
    # 【逐行批注】 计算右侧结果并赋给 premature_exposure，更新脚本状态。
    premature_exposure = False
    # 【逐行批注】 计算右侧结果并赋给 premature_requests，更新脚本状态。
    premature_requests = []
    # 【逐行批注】 计算右侧结果并赋给 unprepared_topics，更新脚本状态。
    unprepared_topics = set()
    # 【逐行批注】 计算右侧结果并赋给 delayed_topics，更新脚本状态。
    delayed_topics = set()
    # 【逐行批注】 计算右侧结果并赋给 active_preparation，更新脚本状态。
    active_preparation = None
    # 【逐行批注】 计算右侧结果并赋给 task_reads，更新脚本状态。
    task_reads = []
    # 【逐行批注】 计算右侧结果并赋给 independent_by_lesson，更新脚本状态。
    independent_by_lesson = {}

    # 【逐行批注】 应用装饰器 @classmethod，扩展下一函数或测试的行为。
    @classmethod
    # 【逐行批注】 定义函数 complete_media，封装可复用的处理步骤。
    def complete_media(cls, media_id):
        # 【逐行批注】 计算右侧结果并赋给 media，更新脚本状态。
        media = next(item for item in cls.data['content']['media'] if item['id'] == media_id)
        # 【逐行批注】 执行当前 Python 语句：media.update({'type': 'timeline', 'status': 'ready', 'events': [{'label': '已重试', 'text': …。
        media.update({'type': 'timeline', 'status': 'ready', 'events': [{'label': '已重试', 'text': '独立恢复素材'}]})

    # 【逐行批注】 定义函数 log_message，封装可复用的处理步骤。
    def log_message(self, *_):
        # 【逐行批注】 执行当前 Python 语句：pass。
        pass

    # 【逐行批注】 定义函数 reply，封装可复用的处理步骤。
    def reply(self, value, status=200):
        # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
        data = json.dumps(value, ensure_ascii=False).encode('utf-8')
        # 【逐行批注】 执行当前 Python 语句：self.send_response(status)。
        self.send_response(status)
        # 【逐行批注】 计算右侧结果并赋给 self.send_header('Content-Type', 'applicatio…，更新脚本状态。
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        # 【逐行批注】 调用 str、len 完成当前脚本或测试步骤：self.send_header('Content-Length', str(len(data)))。
        self.send_header('Content-Length', str(len(data)))
        # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
        self.end_headers()
        # 【逐行批注】 执行当前 Python 语句：self.wfile.write(data)。
        self.wfile.write(data)

    # 【逐行批注】 定义函数 do_GET，封装可复用的处理步骤。
    def do_GET(self):
        # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
        path = urlsplit(self.path).path
        # 【逐行批注】 判断条件 path == '/api/deep-course':，成立时执行对应分支。
        if path == '/api/deep-course':
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).reads.append(self.path)。
            type(self).reads.append(self.path)
            # 【逐行批注】 计算右侧结果并赋给 topic_id，更新脚本状态。
            topic_id = parse_qs(urlsplit(self.path).query).get('topicId', [''])[0]
            # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
            value = copy.deepcopy(type(self).data)
            # 【逐行批注】 判断条件 type(self).active_preparation:，成立时执行对应分支。
            if type(self).active_preparation:
                # 【逐行批注】 计算右侧结果并赋给 value['activeTask']，更新脚本状态。
                value['activeTask'] = copy.deepcopy(type(self).active_preparation)
            # 【逐行批注】 判断条件 topic_id in type(self).unprepared_topics:，成立时执行对应分支。
            if topic_id in type(self).unprepared_topics:
                # 【逐行批注】 执行当前 Python 语句：value.update({'lesson': None, 'content': None, 'status': 'unprepared'})。
                value.update({'lesson': None, 'content': None, 'status': 'unprepared'})
                # 【逐行批注】 结束当前函数并返回结果：return self.reply(value)。
                return self.reply(value)
            # 【逐行批注】 结束当前函数并返回结果：return self.reply(value)。
            return self.reply(value)
        # 【逐行批注】 判断条件 path == '/api/learning-agent':，成立时执行对应分支。
        if path == '/api/learning-agent':
            # 【逐行批注】 计算右侧结果并赋给 task_id，更新脚本状态。
            task_id = parse_qs(urlsplit(self.path).query).get('taskId', [''])[0]
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).task_reads.append(task_id)。
            type(self).task_reads.append(task_id)
            # 【逐行批注】 判断条件 type(self).active_preparation and task_id == type(self).active_p…，成立时执行对应分支。
            if type(self).active_preparation and task_id == type(self).active_preparation.get('id'):
                # 【逐行批注】 结束当前函数并返回结果：return self.reply(copy.deepcopy(type(self).active_preparation))。
                return self.reply(copy.deepcopy(type(self).active_preparation))
            # 【逐行批注】 判断条件 task_id in type(self).media_tasks:，成立时执行对应分支。
            if task_id in type(self).media_tasks:
                # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
                task = type(self).media_tasks[task_id]
                # 【逐行批注】 计算右侧结果并赋给 task['polls'] -，更新脚本状态。
                task['polls'] -= 1
                # 【逐行批注】 判断条件 task['polls'] <= 0 and task['status'] == 'pending':，成立时执行对应分支。
                if task['polls'] <= 0 and task['status'] == 'pending':
                    # 【逐行批注】 计算右侧结果并赋给 task['status']，更新脚本状态。
                    task['status'] = 'ready'
                    # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).complete_media(task['mediaId'])。
                    type(self).complete_media(task['mediaId'])
                # 【逐行批注】 结束当前函数并返回结果：return self.reply(task)。
                return self.reply(task)
            # 【逐行批注】 结束当前函数并返回结果：return self.reply({'id': 'fixture-task', 'status': 'ready', 'events':…。
            return self.reply({'id': 'fixture-task', 'status': 'ready', 'events': []})
        # 【逐行批注】 判断条件 path == '/api/independent-chat':，成立时执行对应分支。
        if path == '/api/independent-chat':
            # 【逐行批注】 计算右侧结果并赋给 lesson_id，更新脚本状态。
            lesson_id = parse_qs(urlsplit(self.path).query).get('lessonId', [''])[0]
            # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
            state = type(self).independent_by_lesson.setdefault(lesson_id, {'webEnabled': False, 'messages': []})
            # 【逐行批注】 结束当前函数并返回结果：return self.reply({'lessonId': lesson_id, 'webEnabled': state['webEna…。
            return self.reply({'lessonId': lesson_id, 'webEnabled': state['webEnabled'],
                               # 【逐行批注】 执行当前 Python 语句：'messages': state['messages'], 'activeTask': None})。
                               'messages': state['messages'], 'activeTask': None})
        # 【逐行批注】 判断条件 path == '/deep':，成立时执行对应分支。
        if path == '/deep':
            # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
            data = '''<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><link rel="stylesheet" href="/deep-course.css"><link rel="stylesheet" href="/agent-trace.css"><style>html,body{margin:0}.site-header{height:72px;display:flex;align-items:center;padding:0 24px;border-bottom:1px solid #dfe6ef;font:15px 'Microsoft YaHei',sans-serif}.site-header img{width:34px;height:34px;margin-right:11px}:root{--gy-header-height:72px}@media(max-width:700px){.site-header{height:64px}:root{--gy-header-height:64px}}</style></head><body class="clarity-app"><header class="site-header"><img src="/school-logo.png" alt="柳州市钢一中学校徽">钢一定制 AI</header><main id="deep-course-app"></main><script src="/chat-render.js"></script><script src="/learning-agent.js"></script><script src="/agent-trace.js"></script><script src="/deep-course.js"></script></body></html>'''.encode('utf-8')
            # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
            self.send_response(200)
            # 【逐行批注】 计算右侧结果并赋给 self.send_header('Content-Type', 'text/html;…，更新脚本状态。
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(data)。
            self.wfile.write(data)
            # 【逐行批注】 结束当前函数并返回结果：return。
            return
        # 【逐行批注】 计算右侧结果并赋给 target，更新脚本状态。
        target = (PUBLIC / path.lstrip('/')).resolve()
        # 【逐行批注】 判断条件 target.is_relative_to(PUBLIC.resolve()) and target.is_file():，成立时执行对应分支。
        if target.is_relative_to(PUBLIC.resolve()) and target.is_file():
            # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
            self.send_response(200)
            # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', mimetypes.guess_type(target.name)[0] or 'application/oct…。
            self.send_header('Content-Type', mimetypes.guess_type(target.name)[0] or 'application/octet-stream')
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(target.read_bytes())。
            self.wfile.write(target.read_bytes())
            # 【逐行批注】 结束当前函数并返回结果：return。
            return
        # 【逐行批注】 执行当前 Python 语句：self.send_error(404)。
        self.send_error(404)

    # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
    def do_POST(self):
        # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
        path = urlsplit(self.path).path
        # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
        value = json.loads(self.rfile.read(int(self.headers.get('Content-Length', 0))))
        # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).requests.append({'path': path, 'body': value})。
        type(self).requests.append({'path': path, 'body': value})
        # 【逐行批注】 判断条件 path == '/api/learn/exposure' and (not type(self).data.get('less…，成立时执行对应分支。
        if path == '/api/learn/exposure' and (not type(self).data.get('lesson') or not type(self).data['lesson'].get('entered')):
            # 【逐行批注】 计算右侧结果并赋给 type(self).premature_exposure，更新脚本状态。
            type(self).premature_exposure = True
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).premature_requests.append({'path': path, 'body': value, 'u…。
            type(self).premature_requests.append({'path': path, 'body': value, 'url': self.headers.get('Referer')})
        # 【逐行批注】 判断条件 path == '/api/learning-agent/control' and value.get('command') =…，成立时执行对应分支。
        if path == '/api/learning-agent/control' and value.get('command') == 'enter_lesson':
            # 【逐行批注】 计算右侧结果并赋给 type(self).data['lesson']['entered']，更新脚本状态。
            type(self).data['lesson']['entered'] = True
        # 【逐行批注】 判断条件 path == '/api/independent-chat/settings':，成立时执行对应分支。
        if path == '/api/independent-chat/settings':
            # 【逐行批注】 计算右侧结果并赋给 lesson_id，更新脚本状态。
            lesson_id = value['lessonId']
            # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
            state = type(self).independent_by_lesson.setdefault(lesson_id, {'webEnabled': False, 'messages': []})
            # 【逐行批注】 计算右侧结果并赋给 state['webEnabled']，更新脚本状态。
            state['webEnabled'] = value['webEnabled']
            # 【逐行批注】 结束当前函数并返回结果：return self.reply({'lessonId': lesson_id, 'webEnabled': state['webEna…。
            return self.reply({'lessonId': lesson_id, 'webEnabled': state['webEnabled']})
        # 【逐行批注】 判断条件 path == '/api/independent-chat/send':，成立时执行对应分支。
        if path == '/api/independent-chat/send':
            # 【逐行批注】 计算右侧结果并赋给 lesson_id，更新脚本状态。
            lesson_id = value['lessonId']
            # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
            state = type(self).independent_by_lesson.setdefault(lesson_id, {'webEnabled': False, 'messages': []})
            # 【逐行批注】 执行当前 Python 语句：state['messages'].append({'id': 'user-' + value['requestId'], 'role': 'user',。
            state['messages'].append({'id': 'user-' + value['requestId'], 'role': 'user',
                                      # 【逐行批注】 执行当前 Python 语句：'content': value['text'], 'status': 'ready', 'sources': [],。
                                      'content': value['text'], 'status': 'ready', 'sources': [],
                                      # 【逐行批注】 执行当前 Python 语句：'createdAt': '2026-10-30T13:31:59Z'})。
                                      'createdAt': '2026-10-30T13:31:59Z'})
            # 【逐行批注】 执行当前 Python 语句：state['messages'].append({'id': 'assistant-' + value['requestId'], 'role': 'assistant',。
            state['messages'].append({'id': 'assistant-' + value['requestId'], 'role': 'assistant',
                                      # 【逐行批注】 执行当前 Python 语句：'content': '这是独立的测试回复。', 'status': 'ready',。
                                      'content': '这是独立的测试回复。', 'status': 'ready',
                                      # 【逐行批注】 执行当前 Python 语句：'sources': ['https://example.com/source'],。
                                      'sources': ['https://example.com/source'],
                                      # 【逐行批注】 执行当前 Python 语句：'createdAt': '2026-10-30T13:32:02Z'})。
                                      'createdAt': '2026-10-30T13:32:02Z'})
            # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
            task = {'id': 'fixture-independent-task', 'status': 'ready', 'events': []}
            # 【逐行批注】 结束当前函数并返回结果：return self.reply({'chat': {'lessonId': lesson_id, 'webEnabled': stat…。
            return self.reply({'chat': {'lessonId': lesson_id, 'webEnabled': state['webEnabled'],
                                        # 【逐行批注】 执行当前 Python 语句：'messages': state['messages'], 'activeTask': task}, 'task': task})。
                                        'messages': state['messages'], 'activeTask': task}, 'task': task})
        # 【逐行批注】 判断条件 path == '/api/independent-chat/clear':，成立时执行对应分支。
        if path == '/api/independent-chat/clear':
            # 【逐行批注】 计算右侧结果并赋给 lesson_id，更新脚本状态。
            lesson_id = value['lessonId']
            # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
            state = type(self).independent_by_lesson.setdefault(lesson_id, {'webEnabled': False, 'messages': []})
            # 【逐行批注】 计算右侧结果并赋给 state['messages']，更新脚本状态。
            state['messages'] = []
            # 【逐行批注】 结束当前函数并返回结果：return self.reply({'lessonId': lesson_id, 'messages': [], 'webEnabled…。
            return self.reply({'lessonId': lesson_id, 'messages': [], 'webEnabled': state['webEnabled']})
        # 【逐行批注】 判断条件 path == '/api/deep-course/start':，成立时执行对应分支。
        if path == '/api/deep-course/start':
            # 【逐行批注】 判断条件 value.get('topicId') in type(self).delayed_topics:，成立时执行对应分支。
            if value.get('topicId') in type(self).delayed_topics:
                # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
                task = {
                    # 【逐行批注】 执行当前 Python 语句：'id': 'agent-deadbeef1234', 'status': 'running', 'purpose': '准备下一课',。
                    'id': 'agent-deadbeef1234', 'status': 'running', 'purpose': '准备下一课',
                    # 【逐行批注】 执行当前 Python 语句：'createdAt': '2026-10-30T04:00:00Z', 'updatedAt': '2026-10-30T04:00:01Z',。
                    'createdAt': '2026-10-30T04:00:00Z', 'updatedAt': '2026-10-30T04:00:01Z',
                    # 【逐行批注】 执行当前 Python 语句：'activity': {'title': '准备本课讲义', 'state': 'started', 'at': '2026-10-30T04:00:01Z'},。
                    'activity': {'title': '准备本课讲义', 'state': 'started', 'at': '2026-10-30T04:00:01Z'},
                    # 【逐行批注】 执行当前 Python 语句：'events': []}。
                    'events': []}
                # 【逐行批注】 计算右侧结果并赋给 type(self).data，更新脚本状态。
                type(self).data = copy.deepcopy(FIXTURE)
                # 【逐行批注】 计算右侧结果并赋给 type(self).data['lesson']['entered']，更新脚本状态。
                type(self).data['lesson']['entered'] = False
                # 【逐行批注】 计算右侧结果并赋给 type(self).data['lesson']['status']，更新脚本状态。
                type(self).data['lesson']['status'] = 'draft'
                # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).unprepared_topics.discard(value['topicId'])。
                type(self).unprepared_topics.discard(value['topicId'])
                # 【逐行批注】 计算右侧结果并赋给 type(self).active_preparation，更新脚本状态。
                type(self).active_preparation = task
                # 【逐行批注】 结束当前函数并返回结果：return self.reply({'task': copy.deepcopy(task)})。
                return self.reply({'task': copy.deepcopy(task)})
            # 【逐行批注】 计算右侧结果并赋给 type(self).data，更新脚本状态。
            type(self).data = copy.deepcopy(FIXTURE)
            # 【逐行批注】 结束当前函数并返回结果：return self.reply({'lessonId': 'fixture-lesson', 'cached': True})。
            return self.reply({'lessonId': 'fixture-lesson', 'cached': True})
        # 【逐行批注】 判断条件 path == '/api/deep-course/retry-media':，成立时执行对应分支。
        if path == '/api/deep-course/retry-media':
            # 【逐行批注】 判断条件 type(self).retry_mode != 'immediate':，成立时执行对应分支。
            if type(self).retry_mode != 'immediate':
                # 【逐行批注】 计算右侧结果并赋给 type(self).overlapping_retry |，更新脚本状态。
                type(self).overlapping_retry |= any(task['status'] == 'pending' for task in type(self).media_tasks.values())
                # 【逐行批注】 计算右侧结果并赋给 task_id，更新脚本状态。
                task_id = 'media-task-' + str(len(type(self).media_tasks) + 1)
                # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
                task = {'id': task_id, 'status': 'paused' if type(self).retry_mode == 'paused' else 'pending', 'events': [], 'polls': 4, 'mediaId': value['mediaId']}
                # 【逐行批注】 计算右侧结果并赋给 type(self).media_tasks[task_id]，更新脚本状态。
                type(self).media_tasks[task_id] = task
                # 【逐行批注】 结束当前函数并返回结果：return self.reply({'task': task})。
                return self.reply({'task': task})
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).complete_media(value['mediaId'])。
            type(self).complete_media(value['mediaId'])
        # 【逐行批注】 判断条件 path == '/api/learning-agent/events':，成立时执行对应分支。
        if path == '/api/learning-agent/events':
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).data['lesson']['timeline'].append({'type': 'message', 'rol…。
            type(self).data['lesson']['timeline'].append({'type': 'message', 'role': 'user', 'taskId': 'fixture-task', 'text': value['text']})
            # 【逐行批注】 判断条件 value['type'] == 'question_answer':，成立时执行对应分支。
            if value['type'] == 'question_answer':
                # 【逐行批注】 计算右侧结果并赋给 section，更新脚本状态。
                section = next(item for item in type(self).data['lesson']['sections'] if item['id'] == value['sectionId'])
                # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).data['lesson']['evaluations'].append({。
                type(self).data['lesson']['evaluations'].append({
                    # 【逐行批注】 执行当前 Python 语句：'id': 'evaluation-' + value['requestId'], 'interactionId': 'input-' + value['requestId'],。
                    'id': 'evaluation-' + value['requestId'], 'interactionId': 'input-' + value['requestId'],
                    # 【逐行批注】 执行当前 Python 语句：'sectionId': section['id'], 'questionId': section.get('questionId', ''),。
                    'sectionId': section['id'], 'questionId': section.get('questionId', ''),
                    # 【逐行批注】 执行当前 Python 语句：'credible': True, 'score': 80, 'feedback': '已保存这次实际作答的评价。'})。
                    'credible': True, 'score': 80, 'feedback': '已保存这次实际作答的评价。'})
        # 【逐行批注】 结束当前函数并返回结果：return self.reply({'task': {'id': 'fixture-task', 'status': 'ready', …。
        return self.reply({'task': {'id': 'fixture-task', 'status': 'ready', 'events': []}} if path.startswith('/api/deep-course/') else {'id': 'fixture-task', 'status': 'ready', 'events': []})


# 【逐行批注】 定义函数 run，封装可复用的处理步骤。
def run(output):
    # 【逐行批注】 计算右侧结果并赋给 output.mkdir(parents，更新脚本状态。
    output.mkdir(parents=True, exist_ok=True)
    # 【逐行批注】 计算右侧结果并赋给 server，更新脚本状态。
    server = ThreadingHTTPServer(('127.0.0.1', 0), FixtureServer)
    # 【逐行批注】 计算右侧结果并赋给 threading.Thread(target，更新脚本状态。
    threading.Thread(target=server.serve_forever, daemon=True).start()
    # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
    errors = []
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as playwright:。
        with sync_playwright() as playwright:
            # 【逐行批注】 计算右侧结果并赋给 executable，更新脚本状态。
            executable = os.environ.get('BROWSER_EXECUTABLE')
            # 【逐行批注】 计算右侧结果并赋给 edge，更新脚本状态。
            edge = Path('C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe')
            # 【逐行批注】 判断条件 not executable and edge.is_file():，成立时执行对应分支。
            if not executable and edge.is_file():
                # 【逐行批注】 计算右侧结果并赋给 executable，更新脚本状态。
                executable = str(edge)
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser = playwright.chromium.launch(headless=True, **({'executable_path': executable} if executable else {}))
            # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
            context = browser.new_context(viewport={'width': 1280, 'height': 720}, reduced_motion='reduce')
            # 【逐行批注】 计算右侧结果并赋给 tab，更新脚本状态。
            tab = context.new_page()
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：tab.on('pageerror', lambda error: errors.append(str(error)))。
            tab.on('pageerror', lambda error: errors.append(str(error)))
            # 【逐行批注】 计算右侧结果并赋给 tab.goto(f'http://127.0.0.1:{server.server_p…，更新脚本状态。
            tab.goto(f'http://127.0.0.1:{server.server_port}/deep?lessonId=fixture-lesson')
            # 【逐行批注】 计算右侧结果并赋给 tab.add_style_tag(url，更新脚本状态。
            tab.add_style_tag(url='/clarity.css')
            # 【逐行批注】 执行当前 Python 语句：tab.locator('.dc-lecture-section h2').wait_for()。
            tab.locator('.dc-lecture-section h2').wait_for()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert [entry for entry in FixtureServer.requests if entry['path'] == '/api/lea…。
            assert [entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/control' and entry['body']['command'] == 'enter_lesson']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not [entry for entry in FixtureServer.requests if entry['path'] == '/api…。
            assert not [entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/start']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-title').inner_text() == '函数的单调性'。
            assert tab.locator('#dc-title').inner_text() == '函数的单调性'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.evaluate('window.fixtureUnsafe') is None。
            assert tab.evaluate('window.fixtureUnsafe') is None

            # 全屏课堂：紧凑顶栏、固定答疑、收起入口与窄窗口抽屉。
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-fullscreen').click()。
            tab.locator('#dc-fullscreen').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.documentElement.classList.contains('gy-classroom-fullscre…。
            tab.wait_for_function("document.documentElement.classList.contains('gy-classroom-fullscreen')")
            # 【逐行批注】 计算右侧结果并赋给 fullscreen_layout，更新脚本状态。
            fullscreen_layout = tab.evaluate('''() => ({
              barHeight: document.getElementById('dc-fullscreen-bar').getBoundingClientRect().height,
              allowed: innerHeight / 17 + 1,
              headerHidden: getComputedStyle(document.querySelector('.site-header')).display === 'none',
              directoryHidden: getComputedStyle(document.getElementById('dc-directory')).display === 'none',
              chatVisible: getComputedStyle(document.getElementById('dc-answer-panel')).display !== 'none'
            })''')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert fullscreen_layout['barHeight'] <= fullscreen_layout['allowed'], fullscre…。
            assert fullscreen_layout['barHeight'] <= fullscreen_layout['allowed'], fullscreen_layout
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert fullscreen_layout['headerHidden'] and fullscreen_layout['directoryHidden…。
            assert fullscreen_layout['headerHidden'] and fullscreen_layout['directoryHidden'] and fullscreen_layout['chatVisible']
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-fullscreen-chat').click()。
            tab.locator('#dc-fullscreen-chat').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not tab.locator('#dc-answer-panel').is_visible()。
            assert not tab.locator('#dc-answer-panel').is_visible()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-chat-dock').is_visible()。
            assert tab.locator('#dc-chat-dock').is_visible()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-dock').click()。
            tab.locator('#dc-chat-dock').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-answer-panel').is_visible()。
            assert tab.locator('#dc-answer-panel').is_visible()
            # 【逐行批注】 执行当前 Python 语句：tab.keyboard.press('Escape')。
            tab.keyboard.press('Escape')
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("!document.documentElement.classList.contains('gy-classroom-fullscr…。
            tab.wait_for_function("!document.documentElement.classList.contains('gy-classroom-fullscreen')")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('.site-header').is_visible()。
            assert tab.locator('.site-header').is_visible()

            # 【逐行批注】 执行当前 Python 语句：tab.set_viewport_size({'width': 900, 'height': 700})。
            tab.set_viewport_size({'width': 900, 'height': 700})
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-fullscreen').click()。
            tab.locator('#dc-fullscreen').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.documentElement.classList.contains('gy-classroom-fullscre…。
            tab.wait_for_function("document.documentElement.classList.contains('gy-classroom-fullscreen')")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-chat-dock').is_visible()。
            assert tab.locator('#dc-chat-dock').is_visible()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not tab.locator('#dc-answer-panel').is_visible()。
            assert not tab.locator('#dc-answer-panel').is_visible()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-dock').click()。
            tab.locator('#dc-chat-dock').click()
            # 【逐行批注】 计算右侧结果并赋给 drawer，更新脚本状态。
            drawer = tab.evaluate('''() => { const box=document.getElementById('dc-answer-panel').getBoundingClientRect();
              return {left:box.left,right:box.right,width:box.width,viewport:innerWidth}; }''')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert drawer['right'] <= drawer['viewport'] + 1 and drawer['width'] <= 380。
            assert drawer['right'] <= drawer['viewport'] + 1 and drawer['width'] <= 380
            # 【逐行批注】 执行当前 Python 语句：tab.keyboard.press('Escape')。
            tab.keyboard.press('Escape')
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("!document.documentElement.classList.contains('gy-classroom-fullscr…。
            tab.wait_for_function("!document.documentElement.classList.contains('gy-classroom-fullscreen')")
            # 【逐行批注】 执行当前 Python 语句：tab.set_viewport_size({'width': 1280, 'height': 720})。
            tab.set_viewport_size({'width': 1280, 'height': 720})

            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-tab-slides').click()。
            tab.locator('#dc-tab-slides').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('.dc-slide h2').inner_text() == '先观察，再判断'。
            assert tab.locator('.dc-slide h2').inner_text() == '先观察，再判断'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == 'lec…。
            assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == 'lecture-one'
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action="next"]').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('.dc-slide h2').inner_text() == '用定义说明'。
            assert tab.locator('.dc-slide h2').inner_text() == '用定义说明'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == 'sum…。
            assert tab.locator('.dc-slide-source').get_attribute('data-section-id') == 'summary-one'
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-tab-lecture').click()。
            tab.locator('#dc-tab-lecture').click()
            # 【逐行批注】 计算右侧结果并赋给 curve，更新脚本状态。
            curve = tab.locator('[data-media-id="curve"] input[type=range]')
            # 【逐行批注】 执行当前 Python 语句：curve.focus()。
            curve.focus()
            # 【逐行批注】 执行当前 Python 语句：curve.press('ArrowRight')。
            curve.press('ArrowRight')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '2.5 x' in tab.locator('[data-media-id="curve"] svg').get_attribute('ari…。
            assert '2.5 x' in tab.locator('[data-media-id="curve"] svg').get_attribute('aria-label')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '合力 8 N' in tab.locator('[data-media-id="forces"]').inner_text()。
            assert '合力 8 N' in tab.locator('[data-media-id="forces"]').inner_text()
            # 【逐行批注】 计算右侧结果并赋给 force，更新脚本状态。
            force = tab.locator('[data-media-id="forces"] input[type=range]')
            # 【逐行批注】 计算右侧结果并赋给 force.evaluate("node，更新脚本状态。
            force.evaluate("node => { node.value = '1'; node.dispatchEvent(new Event('input', {bubbles:true})); }")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '合力 0 N' in tab.locator('[data-media-id="forces"]').inner_text()。
            assert '合力 0 N' in tab.locator('[data-media-id="forces"]').inner_text()
            # 【逐行批注】 计算右侧结果并赋给 force.evaluate("node，更新脚本状态。
            force.evaluate("node => { node.value = '-4'; node.dispatchEvent(new Event('input', {bubbles:true})); }")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '合力 -2 N' in tab.locator('[data-media-id="forces"]').inner_text()。
            assert '合力 -2 N' in tab.locator('[data-media-id="forces"]').inner_text()
            # 【逐行批注】 计算右侧结果并赋给 force.evaluate("node，更新脚本状态。
            force.evaluate("node => { node.value = '10'; node.dispatchEvent(new Event('input', {bubbles:true})); }")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not [request for request in FixtureServer.requests if request['path'] ==…。
            assert not [request for request in FixtureServer.requests if request['path'] == '/api/learning-agent/events']
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-media-id，更新脚本状态。
            tab.locator('[data-media-id="failed-image"] [data-action=media]').click()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-media-id，更新脚本状态。
            tab.locator('[data-media-id="failed-image"] .dc-timeline').wait_for()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len([request for request in FixtureServer.requests if request['path'] ==…。
            assert len([request for request in FixtureServer.requests if request['path'] == '/api/deep-course/retry-media']) == 1
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-toggle').click()。
            tab.locator('#dc-chat-toggle').click()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-input').fill('请用 <script> 举例，但不要执行。')。
            tab.locator('#dc-chat-input').fill('请用 <script> 举例，但不要执行。')
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-input').press('Enter')。
            tab.locator('#dc-chat-input').press('Enter')
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('dc-chat-input').value === ''")。
            tab.wait_for_function("document.getElementById('dc-chat-input').value === ''")
            # 【逐行批注】 计算右侧结果并赋给 sent_chat，更新脚本状态。
            sent_chat = [request for request in FixtureServer.requests if request['path'] == '/api/independent-chat/send'][-1]['body']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert sent_chat['lessonId'] == 'fixture-lesson' and sent_chat['text'] == '请用 <…。
            assert sent_chat['lessonId'] == 'fixture-lesson' and sent_chat['text'] == '请用 <script> 举例，但不要执行。'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not [request for request in FixtureServer.requests。
            assert not [request for request in FixtureServer.requests
                        # 【逐行批注】 判断条件 request['path'] == '/api/learning-agent/events' and request['bod…，成立时执行对应分支。
                        if request['path'] == '/api/learning-agent/events' and request['body'].get('type') == 'chat']
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('#dc-chat-scroll a[href，更新脚本状态。
            tab.locator('#dc-chat-scroll a[href="https://example.com/source"]').wait_for()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=close-chat]').click()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-question-id，更新脚本状态。
            tab.locator('[data-question-id="question-one"] input[value="0"]').check()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-question-id，更新脚本状态。
            tab.locator('[data-question-id="question-one"] textarea').fill('函数值增加了 2。')
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-question-id，更新脚本状态。
            tab.locator('[data-question-id="question-one"] button[type=submit]').click()
            # 【逐行批注】 计算右侧结果并赋给 tab.wait_for_function("document.querySelecto…，更新脚本状态。
            tab.wait_for_function("document.querySelector('[data-question-id=question-one] textarea').value === ''")
            # 【逐行批注】 计算右侧结果并赋给 answer，更新脚本状态。
            answer = [request for request in FixtureServer.requests if request['path'] == '/api/learning-agent/events'][-1]['body']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer['type'] == 'question_answer' and answer['sectionVersion'] == 2 an…。
            assert answer['type'] == 'question_answer' and answer['sectionVersion'] == 2 and answer['view'] == 'practice'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '从 3 变成 5' in answer['text'] and '函数值增加了 2' in answer['text']。
            assert '从 3 变成 5' in answer['text'] and '函数值增加了 2' in answer['text']
            # 【逐行批注】 计算右侧结果并赋给 checks，更新脚本状态。
            checks = []
            # 【逐行批注】 遍历数据并重复执行循环体：for width, height in [(1440, 900), (1280, 720), (390, 844)]:。
            for width, height in [(1440, 900), (1280, 720), (390, 844)]:
                # 【逐行批注】 执行当前 Python 语句：tab.set_viewport_size({'width': width, 'height': height})。
                tab.set_viewport_size({'width': width, 'height': height})
                # 【逐行批注】 计算右侧结果并赋给 tab.locator('#dc-reading').evaluate('node，更新脚本状态。
                tab.locator('#dc-reading').evaluate('node => { node.scrollTop = 0; }')
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = tab.evaluate('''() => ({width:innerWidth, scrollWidth:document.documentElement.scrollWidth, footerBottom:document.querySelector('.dc-footer').getBoundingClientRect().bottom, viewportHeight:innerHeight, logo:document.querySelector('.site-header img').getBoundingClientRect().width})''')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['scrollWidth'] <= width + 1, state。
                assert state['scrollWidth'] <= width + 1, state
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['footerBottom'] <= height + 1, state。
                assert state['footerBottom'] <= height + 1, state
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['logo'] > 0, state。
                assert state['logo'] > 0, state
                # 【逐行批注】 执行当前 Python 语句：checks.append(state)。
                checks.append(state)
                # 【逐行批注】 计算右侧结果并赋给 tab.screenshot(path，更新脚本状态。
                tab.screenshot(path=str(output / f'deep-course-{width}.png'), full_page=True)
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-directory-toggle').click()。
            tab.locator('#dc-directory-toggle').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-outline-dialog').is_visible()。
            assert tab.locator('#dc-outline-dialog').is_visible()
            # 【逐行批注】 执行当前 Python 语句：tab.keyboard.press('Escape')。
            tab.keyboard.press('Escape')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not tab.locator('#dc-outline-dialog').is_visible()。
            assert not tab.locator('#dc-outline-dialog').is_visible()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-toggle').click()。
            tab.locator('#dc-chat-toggle').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-chat-dialog').is_visible()。
            assert tab.locator('#dc-chat-dialog').is_visible()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-chat-input').is_visible()。
            assert tab.locator('#dc-chat-input').is_visible()
            # 【逐行批注】 计算右侧结果并赋给 tab.screenshot(path，更新脚本状态。
            tab.screenshot(path=str(output / 'deep-course-390-chat.png'), full_page=True)
            # 【逐行批注】 执行当前 Python 语句：tab.keyboard.press('Escape')。
            tab.keyboard.press('Escape')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not tab.locator('#dc-chat-dialog').is_visible()。
            assert not tab.locator('#dc-chat-dialog').is_visible()

            # 备课任务尚未创建课时时切换到其它知识点，再返回必须重新绑定同一任务。
            # 【逐行批注】 执行当前 Python 语句：tab.set_viewport_size({'width': 1280, 'height': 720})。
            tab.set_viewport_size({'width': 1280, 'height': 720})
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data，更新脚本状态。
            FixtureServer.data = copy.deepcopy(FIXTURE)
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['lesson']，更新脚本状态。
            FixtureServer.data['lesson'] = None
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['content']，更新脚本状态。
            FixtureServer.data['content'] = None
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.unprepared_topics，更新脚本状态。
            FixtureServer.unprepared_topics = {'topic-two'}
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.delayed_topics，更新脚本状态。
            FixtureServer.delayed_topics = {'topic-two'}
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.active_preparation，更新脚本状态。
            FixtureServer.active_preparation = None
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.task_reads.clear()。
            FixtureServer.task_reads.clear()
            # 【逐行批注】 计算右侧结果并赋给 task_url，更新脚本状态。
            task_url = f'http://127.0.0.1:{server.server_port}/deep?courseId=fixture-course&phaseIndex=3&topicIndex=7&topicId=topic-two'
            # 【逐行批注】 执行当前 Python 语句：tab.goto(task_url)。
            tab.goto(task_url)
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=start]').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.querySelector('#dc-status').textContent.includes('AI 正在处理…。
            tab.wait_for_function("document.querySelector('#dc-status').textContent.includes('AI 正在处理')")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-trace-host .gy-agent-trace').is_visible()。
            assert tab.locator('#dc-trace-host .gy-agent-trace').is_visible()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-stop').is_visible()。
            assert tab.locator('#dc-stop').is_visible()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-activity-spinner').is_visible()。
            assert tab.locator('#dc-activity-spinner').is_visible()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-toggle').click()。
            tab.locator('#dc-chat-toggle').click()
            # 【逐行批注】 计算右侧结果并赋给 chat_draft，更新脚本状态。
            chat_draft = '课程还在准备，我也想先问一个相关问题。'
            # 【逐行批注】 计算右侧结果并赋给 chat_send，更新脚本状态。
            chat_send = tab.locator('#dc-chat-form button[type=submit]')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert chat_send.is_disabled(), '空输入时发送按钮应禁用'。
            assert chat_send.is_disabled(), '空输入时发送按钮应禁用'
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-chat-input').fill(chat_draft)。
            tab.locator('#dc-chat-input').fill(chat_draft)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not chat_send.is_disabled(), '备课进行中仍应允许独立 AI 聊天'。
            assert not chat_send.is_disabled(), '备课进行中仍应允许独立 AI 聊天'
            # 【逐行批注】 计算右侧结果并赋给 chats_before，更新脚本状态。
            chats_before = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/independent-chat/send'])
            # 【逐行批注】 执行当前 Python 语句：chat_send.click()。
            chat_send.click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('dc-chat-input').value === ''")。
            tab.wait_for_function("document.getElementById('dc-chat-input').value === ''")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api…。
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/independent-chat/send']) == chats_before + 1
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-chat-scroll').get_by_text('这是独立的测试回复。').last.is_visible…。
            assert tab.locator('#dc-chat-scroll').get_by_text('这是独立的测试回复。').last.is_visible()
            # 【逐行批注】 计算右侧结果并赋给 starts_before，更新脚本状态。
            starts_before = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/start'])
            # 【逐行批注】 计算右侧结果并赋给 tab.goto(f'http://127.0.0.1:{server.server_p…，更新脚本状态。
            tab.goto(f'http://127.0.0.1:{server.server_port}/deep?courseId=fixture-course&phaseIndex=1&topicIndex=1&topicId=topic-one')
            # 【逐行批注】 计算右侧结果并赋给 tab.wait_for_function("location.search.inclu…，更新脚本状态。
            tab.wait_for_function("location.search.includes('topicId=topic-one')")
            # 【逐行批注】 执行当前 Python 语句：tab.evaluate("localStorage.clear()")。
            tab.evaluate("localStorage.clear()")
            # 【逐行批注】 执行当前 Python 语句：tab.goto(task_url)。
            tab.goto(task_url)
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.querySelector('#dc-status').textContent.includes('AI 正在处理…。
            tab.wait_for_function("document.querySelector('#dc-status').textContent.includes('AI 正在处理')")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-trace-host .gy-agent-trace').is_visible()。
            assert tab.locator('#dc-trace-host .gy-agent-trace').is_visible()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-stop').is_visible()。
            assert tab.locator('#dc-stop').is_visible()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-activity-spinner').is_visible()。
            assert tab.locator('#dc-activity-spinner').is_visible()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'agent-deadbeef1234' in FixtureServer.task_reads。
            assert 'agent-deadbeef1234' in FixtureServer.task_reads
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api…。
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/start']) == starts_before
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.delayed_topics.clear()。
            FixtureServer.delayed_topics.clear()
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.active_preparation，更新脚本状态。
            FixtureServer.active_preparation = None
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.unprepared_topics.clear()。
            FixtureServer.unprepared_topics.clear()

            # 【逐行批注】 计算右侧结果并赋给 before_reads，更新脚本状态。
            before_reads = len(FixtureServer.reads)
            # 【逐行批注】 计算右侧结果并赋给 tab.goto(f'http://127.0.0.1:{server.server_p…，更新脚本状态。
            tab.goto(f'http://127.0.0.1:{server.server_port}/deep?courseId=fixture-course&phaseIndex=1.5&topicIndex=2')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '课程位置无效' in tab.locator('#dc-status').inner_text()。
            assert '课程位置无效' in tab.locator('#dc-status').inner_text()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(FixtureServer.reads) == before_reads。
            assert len(FixtureServer.reads) == before_reads
            # 等待离开页面的上一跳可见性请求结束，避免把旧页面请求归到下一场景。
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_timeout(250)。
            tab.wait_for_timeout(250)
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.premature_exposure，更新脚本状态。
            FixtureServer.premature_exposure = False
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.premature_requests.clear()。
            FixtureServer.premature_requests.clear()
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data，更新脚本状态。
            FixtureServer.data = copy.deepcopy(FIXTURE)
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['lesson']，更新脚本状态。
            FixtureServer.data['lesson'] = None
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['content']，更新脚本状态。
            FixtureServer.data['content'] = None
            # 【逐行批注】 计算右侧结果并赋给 tab.goto(f'http://127.0.0.1:{server.server_p…，更新脚本状态。
            tab.goto(f'http://127.0.0.1:{server.server_port}/deep?courseId=fixture-course&phaseIndex=3&topicIndex=7&topicId=topic-one')
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=start]').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
            # 【逐行批注】 计算右侧结果并赋给 started，更新脚本状态。
            started = [entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/start'][-1]['body']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert started['phaseIndex'] == 3 and started['topicIndex'] == 7 and started['t…。
            assert started['phaseIndex'] == 3 and started['topicIndex'] == 7 and started['topicId'] == 'topic-one'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not FixtureServer.premature_exposure, FixtureServer.premature_requests。
            assert not FixtureServer.premature_exposure, FixtureServer.premature_requests
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.data['outline']['courseStructure'][0]['topics'][0].update({'legacyPhaseInde…。
            FixtureServer.data['outline']['courseStructure'][0]['topics'][0].update({'legacyPhaseIndex': 3, 'legacyTopicIndex': 7})
            # 【逐行批注】 执行当前 Python 语句：tab.reload()。
            tab.reload()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-directory-toggle').click()。
            tab.locator('#dc-directory-toggle').click()
            # 【逐行批注】 计算右侧结果并赋给 topic，更新脚本状态。
            topic = tab.locator('#dc-outline-dialog [data-topic-id="topic-one"]')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert topic.get_attribute('data-phase-index') == '3' and topic.get_attribute('…。
            assert topic.get_attribute('data-phase-index') == '3' and topic.get_attribute('data-topic-index') == '7'
            # 【逐行批注】 执行当前 Python 语句：tab.keyboard.press('Escape')。
            tab.keyboard.press('Escape')
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['content']['media']，更新脚本状态。
            FixtureServer.data['content']['media'] = [
                # 【逐行批注】 执行当前 Python 语句：{'id': 'failed-one', 'type': 'timeline', 'title': '素材一', 'status': 'failed'},。
                {'id': 'failed-one', 'type': 'timeline', 'title': '素材一', 'status': 'failed'},
                # 【逐行批注】 执行当前 Python 语句：{'id': 'failed-two', 'type': 'timeline', 'title': '素材二', 'status': 'failed'}]。
                {'id': 'failed-two', 'type': 'timeline', 'title': '素材二', 'status': 'failed'}]
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['content']['lecture'][0][…，更新脚本状态。
            FixtureServer.data['content']['lecture'][0]['mediaIds'] = ['failed-one', 'failed-two']
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.retry_mode，更新脚本状态。
            FixtureServer.retry_mode = 'delayed'
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(FixtureServer.requests)
            # 【逐行批注】 执行当前 Python 语句：tab.reload()。
            tab.reload()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('.dc-more>summary').click()。
            tab.locator('.dc-more>summary').click()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=retry-media]').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.querySelectorAll('.dc-timeline').length === 2")。
            tab.wait_for_function("document.querySelectorAll('.dc-timeline').length === 2")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not FixtureServer.overlapping_retry。
            assert not FixtureServer.overlapping_retry
            # 【逐行批注】 计算右侧结果并赋给 retries，更新脚本状态。
            retries = [entry for entry in FixtureServer.requests[before:] if entry['path'] == '/api/deep-course/retry-media']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert [entry['body']['mediaId'] for entry in retries] == ['failed-one', 'faile…。
            assert [entry['body']['mediaId'] for entry in retries] == ['failed-one', 'failed-two']
            # 【逐行批注】 遍历数据并重复执行循环体：for media in FixtureServer.data['content']['media']:。
            for media in FixtureServer.data['content']['media']:
                # 【逐行批注】 计算右侧结果并赋给 media['status']，更新脚本状态。
                media['status'] = 'failed'
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.retry_mode，更新脚本状态。
            FixtureServer.retry_mode = 'paused'
            # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
            before = len(FixtureServer.requests)
            # 【逐行批注】 执行当前 Python 语句：tab.reload()。
            tab.reload()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('.dc-more>summary').click()。
            tab.locator('.dc-more>summary').click()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=retry-media]').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('dc-status').textContent.includes('暂停')")。
            tab.wait_for_function("document.getElementById('dc-status').textContent.includes('暂停')")
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_timeout(500)。
            tab.wait_for_timeout(500)
            # 【逐行批注】 计算右侧结果并赋给 retries，更新脚本状态。
            retries = [entry for entry in FixtureServer.requests[before:] if entry['path'] == '/api/deep-course/retry-media']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(retries) == 1。
            assert len(retries) == 1
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.retry_mode，更新脚本状态。
            FixtureServer.retry_mode = 'immediate'
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['content']['media']，更新脚本状态。
            FixtureServer.data['content']['media'] = [{'id': 'broken-ready', 'type': 'image', 'title': '已失效缓存图片', 'status': 'ready', 'url': '/missing-image.png'}]
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['content']['lecture'][0][…，更新脚本状态。
            FixtureServer.data['content']['lecture'][0]['mediaIds'] = ['broken-ready']
            # 【逐行批注】 执行当前 Python 语句：tab.reload()。
            tab.reload()
            # 【逐行批注】 计算右侧结果并赋给 tab.get_by_role('button', name，更新脚本状态。
            tab.get_by_role('button', name='重试图片', exact=True).click()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('.dc-timeline').wait_for()。
            tab.locator('.dc-timeline').wait_for()
            # 【逐行批注】 计算右侧结果并赋给 repaired，更新脚本状态。
            repaired = [entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/retry-media'][-1]['body']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert repaired['mediaId'] == 'broken-ready' and repaired['repair'] is True。
            assert repaired['mediaId'] == 'broken-ready' and repaired['repair'] is True

            # 真实评价改变继续路径；前端不能把阅读、点击或分数直接变成完成记录。
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data，更新脚本状态。
            FixtureServer.data = copy.deepcopy(FIXTURE)
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['lesson']['entered']，更新脚本状态。
            FixtureServer.data['lesson']['entered'] = True
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['content']['media']，更新脚本状态。
            FixtureServer.data['content']['media'] = FixtureServer.data['content']['media'][:1]
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['content']['lecture'][0][…，更新脚本状态。
            FixtureServer.data['content']['lecture'][0]['mediaIds'] = ['curve']
            # 【逐行批注】 执行当前 Python 语句：tab.set_viewport_size({'width': 1280, 'height': 720})。
            tab.set_viewport_size({'width': 1280, 'height': 720})
            # 【逐行批注】 计算右侧结果并赋给 tab.goto(f'http://127.0.0.1:{server.server_p…，更新脚本状态。
            tab.goto(f'http://127.0.0.1:{server.server_port}/deep?lessonId=fixture-lesson')
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
            # 【逐行批注】 计算右侧结果并赋给 before_events，更新脚本状态。
            before_events = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'])
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-tab-slides').click()。
            tab.locator('#dc-tab-slides').click()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-continue').click()。
            tab.locator('#dc-continue').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-tab-lecture').get_attribute('aria-selected') == 'true'。
            assert tab.locator('#dc-tab-lecture').get_attribute('aria-selected') == 'true'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.evaluate("document.activeElement.closest('[data-question-id]')?.data…。
            assert tab.evaluate("document.activeElement.closest('[data-question-id]')?.dataset.questionId") == 'question-one'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api…。
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events']) == before_events
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-question-id，更新脚本状态。
            tab.locator('[data-question-id="question-one"] input[value="0"]').check()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-question-id，更新脚本状态。
            tab.locator('[data-question-id="question-one"] textarea').fill('我比较了两个函数值。')
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-question-id，更新脚本状态。
            tab.locator('[data-question-id="question-one"] button[type=submit]').click()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('.dc-question[data-evaluated，更新脚本状态。
            tab.locator('.dc-question[data-evaluated="true"] .dc-evaluation').wait_for()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
            # 【逐行批注】 计算右侧结果并赋给 finish_events_before，更新脚本状态。
            finish_events_before = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'])
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-continue').click()。
            tab.locator('#dc-continue').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.chatOpen === 't…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.chatOpen === 'true' && document.getElementById('deep-course-app').dataset.ready === 'true'")
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(40):。
            for _ in range(40):
                # 【逐行批注】 判断条件 len([entry for entry in FixtureServer.requests if entry['path'] …，成立时执行对应分支。
                if len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events']) > finish_events_before:
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
                # 【逐行批注】 执行当前 Python 语句：tab.wait_for_timeout(50)。
                tab.wait_for_timeout(50)
            # 【逐行批注】 计算右侧结果并赋给 recent_event_types，更新脚本状态。
            recent_event_types = [entry['body'].get('type') for entry in FixtureServer.requests
                                  # 【逐行批注】 判断条件 entry['path'] == '/api/learning-agent/events'][-5:]，成立时执行对应分支。
                                  if entry['path'] == '/api/learning-agent/events'][-5:]
            # 【逐行批注】 计算右侧结果并赋给 continue_state，更新脚本状态。
            continue_state = {
                # 【逐行批注】 执行当前 Python 语句：'events': recent_event_types,。
                'events': recent_event_types,
                # 【逐行批注】 执行当前 Python 语句：'status': tab.locator('#dc-status').inner_text(),。
                'status': tab.locator('#dc-status').inner_text(),
                # 【逐行批注】 执行当前 Python 语句：'aria_disabled': tab.locator('#dc-continue').get_attribute('aria-disabled'),。
                'aria_disabled': tab.locator('#dc-continue').get_attribute('aria-disabled'),
                # 【逐行批注】 执行当前 Python 语句：'chat_open': tab.locator('#deep-course-app').get_attribute('data-chat-open'),。
                'chat_open': tab.locator('#deep-course-app').get_attribute('data-chat-open'),
                # 【逐行批注】 执行当前 Python 语句：'recent_paths': [entry['path'] for entry in FixtureServer.requests[-8:]],。
                'recent_paths': [entry['path'] for entry in FixtureServer.requests[-8:]],
            # 【逐行批注】 执行当前 Python 语句：}。
            }
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert [entry for entry in FixtureServer.requests if entry['path'] == '/api/lea…。
            assert [entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'][-1]['body']['type'] == 'lesson_finish_request', continue_state
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not FixtureServer.data['lesson'].get('completed', False)。
            assert not FixtureServer.data['lesson'].get('completed', False)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-continue').inner_text() == '继续学习'。
            assert tab.locator('#dc-continue').inner_text() == '继续学习'

            # 【逐行批注】 计算右侧结果并赋给 second_question，更新脚本状态。
            second_question = copy.deepcopy(FixtureServer.data['lesson']['sections'][-1])
            # 【逐行批注】 执行当前 Python 语句：second_question.update({'id': 'question-two', 'questionId': 'stable-question-two', 'title…。
            second_question.update({'id': 'question-two', 'questionId': 'stable-question-two', 'title': '关联新题'})
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.data['lesson']['sections'].append(second_question)。
            FixtureServer.data['lesson']['sections'].append(second_question)
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.data['lesson']['evaluations'].append({。
            FixtureServer.data['lesson']['evaluations'].append({
                # 【逐行批注】 执行当前 Python 语句：'id': 'discarded-evaluation', 'interactionId': 'discarded-input', 'sectionId': 'question-…。
                'id': 'discarded-evaluation', 'interactionId': 'discarded-input', 'sectionId': 'question-two',
                # 【逐行批注】 执行当前 Python 语句：'questionId': 'stable-question-two', 'status': 'discarded_followup', 'credible': True, 's…。
                'questionId': 'stable-question-two', 'status': 'discarded_followup', 'credible': True, 'score': 100,
                # 【逐行批注】 执行当前 Python 语句：'feedback': '已撤销的追问不作为作答评价。'})。
                'feedback': '已撤销的追问不作为作答评价。'})
            # 【逐行批注】 执行当前 Python 语句：tab.reload()。
            tab.reload()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
            # 【逐行批注】 计算右侧结果并赋给 before_events，更新脚本状态。
            before_events = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'])
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-continue').click()。
            tab.locator('#dc-continue').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.evaluate("document.activeElement.closest('[data-question-id]')?.data…。
            assert tab.evaluate("document.activeElement.closest('[data-question-id]')?.dataset.questionId") == 'question-two'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api…。
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events']) == before_events
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('.dc-question').first.get_attribute('data-evaluated') == 'tr…。
            assert tab.locator('.dc-question').first.get_attribute('data-evaluated') == 'true'

            # 需要澄清的有效评价也交回主控，不能在前端用通过线拦住或强行通过。
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.data['lesson']['evaluations'][-1].update({'status': 'valid', 'credible': Fa…。
            FixtureServer.data['lesson']['evaluations'][-1].update({'status': 'valid', 'credible': False, 'score': None})
            # 【逐行批注】 执行当前 Python 语句：tab.reload()。
            tab.reload()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-continue').click()。
            tab.locator('#dc-continue').click()
            # 【逐行批注】 执行当前 Python 语句：tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true…。
            tab.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert [entry for entry in FixtureServer.requests if entry['path'] == '/api/lea…。
            assert [entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'][-1]['body']['type'] == 'lesson_finish_request'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not FixtureServer.data['lesson'].get('completed', False)。
            assert not FixtureServer.data['lesson'].get('completed', False)

            # 模拟既有主控随后保存的完成结果；导航不触发下一课付费生成。
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['lesson']['completed']，更新脚本状态。
            FixtureServer.data['lesson']['completed'] = True
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.unprepared_topics，更新脚本状态。
            FixtureServer.unprepared_topics = {'topic-two'}
            # 【逐行批注】 执行当前 Python 语句：tab.reload()。
            tab.reload()
            # 【逐行批注】 计算右侧结果并赋给 tab.get_by_role('button', name，更新脚本状态。
            tab.get_by_role('button', name='下一知识点：参数与曲线', exact=True).wait_for()
            # 【逐行批注】 遍历数据并重复执行循环体：for width, height in [(1280, 720), (390, 844)]:。
            for width, height in [(1280, 720), (390, 844)]:
                # 【逐行批注】 执行当前 Python 语句：tab.set_viewport_size({'width': width, 'height': height})。
                tab.set_viewport_size({'width': width, 'height': height})
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = tab.evaluate("() => ({width:innerWidth,contentWidth:document.documentElement.scrollWidth,footerBottom:document.querySelector('.dc-footer').getBoundingClientRect().bottom,viewportHeight:innerHeight})")
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['contentWidth'] <= width + 1 and state['footerBottom'] <= height +…。
                assert state['contentWidth'] <= width + 1 and state['footerBottom'] <= height + 1, state
                # 【逐行批注】 计算右侧结果并赋给 tab.screenshot(path，更新脚本状态。
                tab.screenshot(path=str(output / f'deep-course-completed-{width}.png'), full_page=True)
            # 【逐行批注】 计算右侧结果并赋给 starts_before，更新脚本状态。
            starts_before = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/start'])
            # 【逐行批注】 计算右侧结果并赋给 events_before，更新脚本状态。
            events_before = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'])
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-continue').click()。
            tab.locator('#dc-continue').click()
            # 【逐行批注】 计算右侧结果并赋给 tab.locator('[data-action，更新脚本状态。
            tab.locator('[data-action=start]').wait_for()
            # 【逐行批注】 计算右侧结果并赋给 target，更新脚本状态。
            target = parse_qs(urlsplit(tab.url).query)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert target['topicId'] == ['topic-two'] and target['phaseIndex'] == ['3'] and…。
            assert target['topicId'] == ['topic-two'] and target['phaseIndex'] == ['3'] and target['topicIndex'] == ['7']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'lessonId' not in target。
            assert 'lessonId' not in target
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api…。
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/deep-course/start']) == starts_before
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api…。
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events']) == events_before

            # 【逐行批注】 执行当前 Python 语句：FixtureServer.unprepared_topics.clear()。
            FixtureServer.unprepared_topics.clear()
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.data['lesson'].update({'topicId': 'topic-two', 'phaseIndex': 3, 'topicIndex…。
            FixtureServer.data['lesson'].update({'topicId': 'topic-two', 'phaseIndex': 3, 'topicIndex': 7})
            # 【逐行批注】 执行当前 Python 语句：tab.reload()。
            tab.reload()
            # 【逐行批注】 计算右侧结果并赋给 tab.get_by_role('button', name，更新脚本状态。
            tab.get_by_role('button', name='查看课程目录', exact=True).wait_for()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-continue').click()。
            tab.locator('#dc-continue').click()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-directory-toggle').get_attribute('aria-expanded') == 't…。
            assert tab.locator('#dc-directory-toggle').get_attribute('aria-expanded') == 'true'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api…。
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events']) == events_before

            # 长课件的翻页栏始终在滚动区外，正文末段仍可完整阅读。
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data，更新脚本状态。
            FixtureServer.data = copy.deepcopy(FIXTURE)
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.data['lesson'].update({'id': 'long-fixture-lesson', 'entered': True})。
            FixtureServer.data['lesson'].update({'id': 'long-fixture-lesson', 'entered': True})
            # 【逐行批注】 计算右侧结果并赋给 long_body，更新脚本状态。
            long_body = '\n\n'.join(['长页夹具正文：比较自变量与函数值的变化，并说明判断所依据的关系。'] * 35 + ['长页完整正文的末段标记。'])
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['lesson']['sections'][0][…，更新脚本状态。
            FixtureServer.data['lesson']['sections'][0]['body'] = long_body
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.data['content']['lecture'][0].update({'body': long_body, 'mediaIds': []})。
            FixtureServer.data['content']['lecture'][0].update({'body': long_body, 'mediaIds': []})
            # 【逐行批注】 计算右侧结果并赋给 FixtureServer.data['content']['media']，更新脚本状态。
            FixtureServer.data['content']['media'] = []
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.data['content']['slides'][0].update({'body': long_body, 'mediaIds': []})。
            FixtureServer.data['content']['slides'][0].update({'body': long_body, 'mediaIds': []})
            # 【逐行批注】 执行当前 Python 语句：FixtureServer.data['content']['slides'].append({'id': 'slide-three', 'title': '回看判断过程', '…。
            FixtureServer.data['content']['slides'].append({'id': 'slide-three', 'title': '回看判断过程', 'sourceSectionIds': ['summary-one'], 'mediaIds': []})
            # 【逐行批注】 计算右侧结果并赋给 tab.goto(f'http://127.0.0.1:{server.server_p…，更新脚本状态。
            tab.goto(f'http://127.0.0.1:{server.server_port}/deep?lessonId=long-fixture-lesson')
            # 【逐行批注】 计算右侧结果并赋给 tab.add_style_tag(url，更新脚本状态。
            tab.add_style_tag(url='/clarity.css')
            # 【逐行批注】 执行当前 Python 语句：tab.locator('.dc-lecture-section').wait_for()。
            tab.locator('.dc-lecture-section').wait_for()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not tab.locator('#dc-slide-navigation').is_visible()。
            assert not tab.locator('#dc-slide-navigation').is_visible()
            # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-tab-slides').click()。
            tab.locator('#dc-tab-slides').click()
            # 【逐行批注】 计算右侧结果并赋给 paging_checks，更新脚本状态。
            paging_checks = []
            # 【逐行批注】 计算右侧结果并赋给 model_events_before，更新脚本状态。
            model_events_before = len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events'])
            # 【逐行批注】 遍历数据并重复执行循环体：for width, height in [(1280, 720), (1440, 900), (390, 844)]:。
            for width, height in [(1280, 720), (1440, 900), (390, 844)]:
                # 【逐行批注】 执行当前 Python 语句：tab.set_viewport_size({'width': width, 'height': height})。
                tab.set_viewport_size({'width': width, 'height': height})
                # 【逐行批注】 计算右侧结果并赋给 tab.locator('#dc-reading').evaluate('node，更新脚本状态。
                tab.locator('#dc-reading').evaluate('node => { node.scrollTop = 0; }')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-slide-count').inner_text() == '1 / 3'。
                assert tab.locator('#dc-slide-count').inner_text() == '1 / 3'
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-slide-previous').is_disabled() and tab.locator('#dc-sli…。
                assert tab.locator('#dc-slide-previous').is_disabled() and tab.locator('#dc-slide-next').is_enabled()
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = tab.evaluate('''() => {
                    const pager = document.getElementById('dc-slide-navigation').getBoundingClientRect();
                    const reading = document.getElementById('dc-reading');
                    const viewport = reading.getBoundingClientRect();
                    const controls = [...document.querySelectorAll('#dc-slide-navigation button,#dc-slide-count')].map(node => { const box = node.getBoundingClientRect(); return {top:box.top,bottom:box.bottom,left:box.left,right:box.right}; });
                    return {width:innerWidth,contentWidth:document.documentElement.scrollWidth,pagerTop:pager.top,pagerBottom:pager.bottom,readingTop:viewport.top,readingBottom:viewport.bottom,footerTop:document.querySelector('.dc-footer').getBoundingClientRect().top,scrollHeight:reading.scrollHeight,readingHeight:reading.clientHeight,controls};
                }''')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['contentWidth'] <= width + 1 and state['pagerBottom'] <= state['re…。
                assert state['contentWidth'] <= width + 1 and state['pagerBottom'] <= state['readingTop'] + 1, state
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['readingBottom'] <= state['footerTop'] + 1 and state['scrollHeight…。
                assert state['readingBottom'] <= state['footerTop'] + 1 and state['scrollHeight'] > state['readingHeight'] * 2, state
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(box['top'] >= 0 and box['bottom'] <= height and box['left'] >= 0 and…。
                assert all(box['top'] >= 0 and box['bottom'] <= height and box['left'] >= 0 and box['right'] <= width for box in state['controls']), state
                # 【逐行批注】 计算右侧结果并赋给 tab.screenshot(path，更新脚本状态。
                tab.screenshot(path=str(output / f'deep-course-long-slides-{width}.png'), full_page=True)
                # 【逐行批注】 计算右侧结果并赋给 tab.locator('#dc-reading').evaluate('node，更新脚本状态。
                tab.locator('#dc-reading').evaluate('node => { node.scrollTop = node.scrollHeight; }')
                # 【逐行批注】 计算右侧结果并赋给 bottom，更新脚本状态。
                bottom = tab.evaluate('''() => { const pager = document.getElementById('dc-slide-navigation').getBoundingClientRect(); const viewport = document.getElementById('dc-reading').getBoundingClientRect(); const last = document.querySelector('.dc-slide-source .dc-rich').lastElementChild.getBoundingClientRect(); return {pagerTop:pager.top,lastTop:last.top,lastBottom:last.bottom,readingTop:viewport.top,readingBottom:viewport.bottom}; }''')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert abs(bottom['pagerTop'] - state['pagerTop']) < 1 and bottom['lastTop'] < …。
                assert abs(bottom['pagerTop'] - state['pagerTop']) < 1 and bottom['lastTop'] < bottom['readingBottom'] and bottom['lastBottom'] > bottom['readingTop'], bottom
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '长页完整正文的末段标记。' in tab.locator('.dc-slide-source').inner_text()。
                assert '长页完整正文的末段标记。' in tab.locator('.dc-slide-source').inner_text()
                # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-slide-next').focus()。
                tab.locator('#dc-slide-next').focus()
                # 【逐行批注】 执行当前 Python 语句：tab.keyboard.press('Enter')。
                tab.keyboard.press('Enter')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-slide-count').inner_text() == '2 / 3'。
                assert tab.locator('#dc-slide-count').inner_text() == '2 / 3'
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('.dc-slide h2').inner_text() == '用定义说明'。
                assert tab.locator('.dc-slide h2').inner_text() == '用定义说明'
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.evaluate("document.activeElement.id === 'dc-slide-next' && document.…。
                assert tab.evaluate("document.activeElement.id === 'dc-slide-next' && document.getElementById('dc-reading').scrollTop === 0")
                # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-slide-next').click()。
                tab.locator('#dc-slide-next').click()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-slide-count').inner_text() == '3 / 3' and tab.locator('…。
                assert tab.locator('#dc-slide-count').inner_text() == '3 / 3' and tab.locator('#dc-slide-next').is_disabled()
                # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-slide-previous').click()。
                tab.locator('#dc-slide-previous').click()
                # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-slide-previous').click()。
                tab.locator('#dc-slide-previous').click()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-slide-count').inner_text() == '1 / 3'。
                assert tab.locator('#dc-slide-count').inner_text() == '1 / 3'
                # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-tab-lecture').click()。
                tab.locator('#dc-tab-lecture').click()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not tab.locator('#dc-slide-navigation').is_visible()。
                assert not tab.locator('#dc-slide-navigation').is_visible()
                # 【逐行批注】 执行当前 Python 语句：tab.locator('#dc-tab-slides').click()。
                tab.locator('#dc-tab-slides').click()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert tab.locator('#dc-slide-navigation').is_visible() and tab.locator('#dc-sl…。
                assert tab.locator('#dc-slide-navigation').is_visible() and tab.locator('#dc-slide-count').inner_text() == '1 / 3'
                # 【逐行批注】 执行当前 Python 语句：paging_checks.append(state)。
                paging_checks.append(state)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api…。
            assert len([entry for entry in FixtureServer.requests if entry['path'] == '/api/learning-agent/events']) == model_events_before
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, errors。
            assert not errors, errors
            # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
            result = {'fixtureOnly': True, 'checks': checks, 'errors': errors,
                      # 【逐行批注】 执行当前 Python 语句：'longSlidePaging': paging_checks,。
                      'longSlidePaging': paging_checks,
                      # 【逐行批注】 执行当前 Python 语句：'continuation': {'unansweredLocated': True, 'evaluatedRequestsController': True,。
                      'continuation': {'unansweredLocated': True, 'evaluatedRequestsController': True,
                                       # 【逐行批注】 执行当前 Python 语句：'newQuestionLocated': True, 'completionUsesSavedState': True,。
                                       'newQuestionLocated': True, 'completionUsesSavedState': True,
                                       # 【逐行批注】 执行当前 Python 语句：'nextTopicRequiresExplicitPreparation': True},。
                                       'nextTopicRequiresExplicitPreparation': True},
                      # 【逐行批注】 执行当前 Python 语句：'events': [request for request in FixtureServer.requests if request['path'] == '/api/lear…。
                      'events': [request for request in FixtureServer.requests if request['path'] == '/api/learning-agent/events'],
                      # 【逐行批注】 执行当前 Python 语句：'result': 'passed'}。
                      'result': 'passed'}
            # 【逐行批注】 计算右侧结果并赋给 (output / 'deep-course-verification.json').w…，更新脚本状态。
            (output / 'deep-course-verification.json').write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
            # 【逐行批注】 输出当前运行结果：print(json.dumps(result, ensure_ascii=False))。
            print(json.dumps(result, ensure_ascii=False))
            # 【逐行批注】 执行当前 Python 语句：browser.close()。
            browser.close()
    # 【逐行批注】 执行当前 Python 语句：finally:。
    finally:
        # 【逐行批注】 执行当前 Python 语句：server.shutdown()。
        server.shutdown()


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 计算右侧结果并赋给 parser，更新脚本状态。
    parser = argparse.ArgumentParser()
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('output', nargs，更新脚本状态。
    parser.add_argument('output', nargs='?', type=Path, default=Path('verification/deep-course-browser'))
    # 【逐行批注】 调用 run 完成当前脚本或测试步骤：run(parser.parse_args().output)。
    run(parser.parse_args().output)

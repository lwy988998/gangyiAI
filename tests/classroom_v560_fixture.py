# 【逐行批注】 执行当前 Python 语句："""为统一课堂的浏览器验收提供隔离档案与协议夹具，不读取真实用户档案。"""。
"""为统一课堂的浏览器验收提供隔离档案与协议夹具，不读取真实用户档案。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：import socket。
import socket
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sqlite3。
import sqlite3
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextlib import closing, contextmanager。
from contextlib import closing, contextmanager
# 【逐行批注】 导入后续逻辑需要的模块或符号：from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer。
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from threading import Thread。
from threading import Thread
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen


def open_learning_menu(page, section=None):
    """从正式页头打开学习操作，并按需展开其中的分组。"""
    panel = page.locator('#learning-workspace-menu')
    if not panel.is_visible():
        page.get_by_role('button', name='操作菜单', exact=True).click()
    if section:
        summary = panel.get_by_text(section, exact=True)
        if not summary.evaluate('(element) => element.parentElement.open'):
            summary.click()
    return panel


def launch_test_browser(playwright):
    """Windows 可直接复用已安装的 Edge。"""
    chromium = playwright.chromium
    if os.name == 'nt' and not Path(chromium.executable_path).is_file():
        return chromium.launch(channel='msedge', headless=True)
    return chromium.launch(headless=True)


# 【逐行批注】 定义类 FixtureAI，组织相关数据与行为。
class FixtureAI(BaseHTTPRequestHandler):
    # 【逐行批注】 计算右侧结果并赋给 calls，更新脚本状态。
    calls = []
    # 【逐行批注】 计算右侧结果并赋给 slow，更新脚本状态。
    slow = False
    # 【逐行批注】 计算右侧结果并赋给 fail_next，更新脚本状态。
    fail_next = False
    # 【逐行批注】 计算右侧结果并赋给 fail，更新脚本状态。
    fail = False
    # 【逐行批注】 计算右侧结果并赋给 http_error，更新脚本状态。
    http_error = 0
    # 【逐行批注】 计算右侧结果并赋给 feedback_help_type，更新脚本状态。
    feedback_help_type = 'none'
    # 【逐行批注】 计算右侧结果并赋给 long_teaching，更新脚本状态。
    long_teaching = False

    # 【逐行批注】 定义函数 log_message，封装可复用的处理步骤。
    def log_message(self, *_):
        # 【逐行批注】 执行当前 Python 语句：pass。
        pass

    # 【逐行批注】 定义函数 do_GET，封装可复用的处理步骤。
    def do_GET(self):
        # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
        self.send_response(200)
        # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'application/json')。
        self.send_header('Content-Type', 'application/json')
        # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
        self.end_headers()
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.wfile.write(json.dumps({'calls': len(self.calls), 'slow': self.s…。
        self.wfile.write(json.dumps({'calls': len(self.calls), 'slow': self.slow, 'fail_next': self.fail_next, 'fail': self.fail}).encode())

    # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
    def do_POST(self):
        # 【逐行批注】 计算右侧结果并赋给 request，更新脚本状态。
        request = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        # 【逐行批注】 判断条件 self.path == '/fixture-control':，成立时执行对应分支。
        if self.path == '/fixture-control':
            # 【逐行批注】 计算右侧结果并赋给 type(self).slow，更新脚本状态。
            type(self).slow = request.get('slow', False)
            # 【逐行批注】 计算右侧结果并赋给 type(self).fail_next，更新脚本状态。
            type(self).fail_next = request.get('fail_next', False)
            # 【逐行批注】 计算右侧结果并赋给 type(self).fail，更新脚本状态。
            type(self).fail = request.get('fail', False)
            # 【逐行批注】 计算右侧结果并赋给 type(self).http_error，更新脚本状态。
            type(self).http_error = request.get('http_error', 0)
            # 【逐行批注】 计算右侧结果并赋给 type(self).feedback_help_type，更新脚本状态。
            type(self).feedback_help_type = request.get('feedback_help_type', 'none')
            # 【逐行批注】 计算右侧结果并赋给 type(self).long_teaching，更新脚本状态。
            type(self).long_teaching = request.get('long_teaching', False)
            # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
            self.send_response(200)
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(b'{}')。
            self.wfile.write(b'{}')
            # 【逐行批注】 结束当前函数并返回结果：return。
            return
        # 【逐行批注】 判断条件 type(self).fail_next or type(self).fail or type(self).http_error:，成立时执行对应分支。
        if type(self).fail_next or type(self).fail or type(self).http_error:
            # 【逐行批注】 计算右侧结果并赋给 type(self).fail_next，更新脚本状态。
            type(self).fail_next = False
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).calls.append('failed-http')。
            type(self).calls.append('failed-http')
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：self.send_response(type(self).http_error or 503)。
            self.send_response(type(self).http_error or 503)
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 结束当前函数并返回结果：return。
            return
        # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
        system_prompt = request['messages'][0]['content']
        if '试卷识别器' in system_prompt:
            assert 'pageIndexes' in system_prompt
            if len(request['messages']) == 2:
                questions = [dict(id='q-practice', number='3', stem='另一个批次的题目',
                                  pageIndex=0, recognitionConfidence=.9)]
            else:
                questions = [
                    dict(id='q-practice', number='1', stem='识别原题',
                         materials=['共同材料'], pageIndex=0, pageIndexes=[0, 1],
                         subquestions=['第一个小问', dict(text='未校正的小问', referenceAnswer='PRIVATE-EXAM-KEY')],
                         referenceAnswer='PRIVATE-EXAM-KEY', handwrittenAnswer='PRIVATE-HANDWRITING',
                         recognitionConfidence=.9),
                    dict(id='q-uncertain', number='2', stem='可能有误的题干', pageIndex=2, recognitionConfidence=.4),
                ]
            self.send_decision(dict(title='隔离试卷', questions=questions))
            return
        if '试卷讲解老师' in system_prompt:
            self.send_decision(dict(message='试卷已识别，请先核对原卷页和小问。',
                                    artifacts=[], currentQuestionId='q-practice', state='continue'))
            return
        context = json.loads(request['messages'][1]['content'])
        # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
        event = context['event']
        expected_curriculum = {'核对课程教材来源': 'pep-a-sets-logic',
                               '核对课程人教 B 版教材来源': 'pep-b-sets-logic'}.get(event.get('text'))
        # 【逐行批注】 调用 type 完成当前脚本或测试步骤：type(self).calls.append(event['requestId'])。
        type(self).calls.append(event['requestId'])
        # 【逐行批注】 计算右侧结果并赋给 results，更新脚本状态。
        results = []
        # 【逐行批注】 遍历数据并重复执行循环体：for item in request['messages'][2:]:。
        for item in request['messages'][2:]:
            # 【逐行批注】 判断条件 item['role'] == 'user':，成立时执行对应分支。
            if item['role'] == 'user':
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 执行当前 Python 语句：results.extend(json.loads(item['content']).get('toolResults', []))。
                    results.extend(json.loads(item['content']).get('toolResults', []))
                # 【逐行批注】 捕获并处理异常：except ValueError:。
                except ValueError:
                    # 【逐行批注】 执行当前 Python 语句：pass。
                    pass
        # 【逐行批注】 计算右侧结果并赋给 actions, state, message，更新脚本状态。
        actions, state, message = [], 'completed', '已保存本次处理。'
        # 【逐行批注】 判断条件 event.get('attemptSchedule'):，成立时执行对应分支。
        if event.get('attemptSchedule'):
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [dict(id='unauthorized-schedule', tool='adjust_schedule', args=event['attemptSchedule'])]
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'startup':。
        elif event['type'] == 'startup':
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [dict(id='recommend', tool='update_recommendations', args=dict(
                # 【逐行批注】 计算右侧结果并赋给 lite，更新脚本状态。
                lite=['认识化合价', '比较电子变化', '练习配平'], deep=['氧化还原概念课', '化合价与电子转移'], reason='虚构档案仅提供学习方向'))]
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'plan_course':。
        elif event['type'] == 'plan_course':
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [dict(id='plan', tool='create_course', args=dict(goal='理解化合价与电子变化', title='隔离化学课程', mode='lite', summary='用于界面验收的虚构课程',
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload=dict(courseStructure=[dict(stage='建立概念', goal='比较化合价变化', why='先比较变化，再联系电子', topics=['化合价比较', '电子得失']),
                    # 【逐行批注】 计算右侧结果并赋给 dict(stage，更新脚本状态。
                    dict(stage='应用与巩固', goal='应用比较方法', why='结合实际回答再安排练习', topics=['反应判断', '配平练习'])])))]
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'prepare_next':。
        elif event['type'] == 'prepare_next':
            # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
            lesson = next((item['result']['lessonId'] for item in results if 'lessonId' in item['result']), None)
            # 【逐行批注】 判断条件 not lesson:，成立时执行对应分支。
            if not lesson:
                # 【逐行批注】 计算右侧结果并赋给 outline，更新脚本状态。
                outline = context['courses'][0]['outline']
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='create', tool='create_lesson', args=dict(courseId=event['courseId'], title='化合价与电子变化', purpose='从比较前后状态开始理解', intent='advance', topicId=outline['courseStructure'][0]['topics'][0]['id']))]
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'continue'
            # 【逐行批注】 当前一条件不成立时继续判断：not any('sectionId' in item['result'] for item in results):。
            elif not any('sectionId' in item['result'] for item in results):
                # 【逐行批注】 计算右侧结果并赋给 sections，更新脚本状态。
                sections = [dict(kind='explanation', title='先比较同一种元素', body='比较反应前后同一种元素的状态。先标出化合价，再观察升高还是降低。\n\n把观察过程说清楚，比只记结论更有帮助。'),
                    # 【逐行批注】 计算右侧结果并赋给 dict(kind，更新脚本状态。
                    dict(kind='explanation', title='把化合价变化联系到电子', body='接着联系电子得失，解释这种变化为什么发生。\n\n请先给出判断理由，再用另一种情况检查自己的理解。'),
                    # 【逐行批注】 计算右侧结果并赋给 dict(kind，更新脚本状态。
                    dict(kind='explanation', presentation='example', title='示范：比较一个简单变化', body='这是示范讲解。先写出前后状态，再比较变化方向。示范不产生学生评分。'),
                    # 【逐行批注】 计算右侧结果并赋给 dict(kind，更新脚本状态。
                    dict(kind='question', questionKind='interaction', title='说说你的观察', question=dict(question='Fe 变为 Fe²⁺ 时，你观察到怎样的变化？', type='open', expectedAnswer='PRIVATE-EXPECTED', rubric='PRIVATE-RUBRIC')),
                    # 【逐行批注】 计算右侧结果并赋给 dict(kind，更新脚本状态。
                    dict(kind='question', questionKind='interaction', title='联系电子说一说', question=dict(question='你怎样用电子得失解释刚才的观察？', type='open', expectedAnswer='PRIVATE-EXPECTED', rubric='PRIVATE-RUBRIC')),
                    # 【逐行批注】 计算右侧结果并赋给 dict(kind，更新脚本状态。
                    dict(kind='question', questionKind='practice', title='练习一：判断变化', question=dict(question='请选择化合价升高的情况，并写出理由。', type='choice', options=['Fe → Fe²⁺', 'Cu²⁺ → Cu'], expectedAnswer='PRIVATE-EXPECTED', rubric='PRIVATE-RUBRIC')),
                    # 【逐行批注】 计算右侧结果并赋给 dict(kind，更新脚本状态。
                    dict(kind='question', questionKind='practice', title='练习二：解释理由', question=dict(question='解释化合价变化与电子得失的联系。', type='open', expectedAnswer='PRIVATE-EXPECTED', rubric='PRIVATE-RUBRIC')),
                    # 【逐行批注】 计算右侧结果并赋给 dict(kind，更新脚本状态。
                    dict(kind='summary', title='本节检查点', body='- 能比较同一元素的前后状态。\n- 能说明自己的判断理由。\n- 能用电子变化解释观察。')]
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id=f'section-{index}', tool='append_section', args=dict(lessonId=lesson, section=section)) for index, section in enumerate(sections)]
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'continue'
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='finish', tool='finish_lesson', args=dict(lessonId=lesson))]
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] in ('lesson_start', 'chat'):。
        elif event['type'] == 'library_research' and event.get('sourceId'):
            if not results:
                actions = [dict(id='verify-official-original', tool='read_resource', args=dict(sourceId=event['sourceId']))]
                state, message = 'continue', ''
            else:
                message = '原文读取步骤已回传。'
        elif event['type'] == 'chat' and expected_curriculum:
            curriculum = context['savedLibrary']['curriculum']
            assert curriculum['chapter']['id'] == expected_curriculum and curriculum['grade'] == '高一'
            assert {source['role'] for source in curriculum['sources']} == {'standard', 'publisherCatalog'}
            assert all(source['status'] == 'verified' for source in curriculum['sources'])
            publisher = next(source for source in curriculum['sources'] if source['role'] == 'publisherCatalog')
            assert publisher['url'] == ('https://www.pep.com.cn/gzsxb/' if expected_curriculum == 'pep-b-sets-logic'
                                        else 'https://www.pep.com.cn/xw/zt/px/2019/shuxue/')
            if not results:
                actions = [dict(id='read-pinned-publisher', tool='read_resource', args=dict(sourceId=publisher['sourceId']))]
                state, message, curriculum_decision = 'continue', '', {}
            else:
                saved = next(item['result']['snapshot'] for item in results if 'snapshot' in item['result'])
                assert saved['id'] == publisher['snapshotId']
                message = '以课程标准说明范围，并按出版社目录找到相应章节。'
                curriculum_decision = dict(materialOrigin='source_based', citations=[
                    {key: source[key] for key in ('sourceId', 'snapshotId', 'segmentId')} for source in curriculum['sources']])
        elif event['type'] == 'chat' and 'examQuestion' in event:
            exam = context['examQuestion']
            assert not any(secret in json.dumps(context) for secret in ['PRIVATE-EXAM-KEY', 'PRIVATE-HANDWRITING'])
            assert 'savedLibrary' not in context and 'dataUrl' not in json.dumps(exam)
            guide = request['messages'][0]['content']
            assert '"name":"calculate_exact"' in guide and '"name":"create_course"' not in guide and '"name":"read_resource"' not in guide
            points = ', '.join(part['text'] for part in exam['subquestions'])
            history = next((item['text'] for item in context['materialConversation'] if item['role'] == 'user'), '')
            message = f'{exam["stem"]}｜{points}'
            if history:
                message += '｜前次提问：' + history
        elif event['type'] == 'chat' and 'material' in event:
            system_prompt = request['messages'][0]['content']
            assert '"name":"read_resource"' in system_prompt and 'nextOffset' in system_prompt
            assert '"name":"create_course"' not in system_prompt and '"name":"search_resources"' not in system_prompt
            selected, = context['savedLibrary']['sources']
            original = selected['snapshot']['segments'][0]
            assert selected['snapshotId'] == event['material']['snapshotId']
            assert selected['id'] == event['material']['sourceId']
            history = context['materialConversation']
            message = original['text']
            if history:
                message += ' 前次提问：' + next(item['text'] for item in history if item['role'] == 'user')
            read = next((item['result'] for item in results if 'snapshot' in item['result']), None)
            if '读取后半部分' in event['text'] and read is None:
                actions = [dict(id='read-fixed-version', tool='read_resource', args=dict(sourceId=selected['id'], offset=1))]
                state, message = 'continue', ''
            elif read is not None:
                assert read['snapshot']['id'] == selected['snapshotId']
                original = read['snapshot']['segments'][0]
                message = original['text']
            ref = dict(event['material'], segmentId=original['id'])
            if '伪造新版引用' in event['text']:
                ref.update(snapshotId='snap-latest', segmentId='latest-one')
            material_decision = dict(materialOrigin='source_based', citations=[ref]) if message else {}
        elif event['type'] in ('lesson_start', 'chat'):
            if 'yesterdayReview' in event:
                source = context['yesterdayReview']
                assert source['date'] == event['yesterdayReview']['date']
                assert source['visits'] == 1 and source['exchanges'] == 2
                assert context['lessonContext']['id'] == event['lessonId']
                assert '"name":"create_course"' not in system_prompt and '"name":"evaluate_answer"' not in system_prompt
            # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
            lesson = next((item['result'] for item in results if 'sections' in item['result']), None)
            # 【逐行批注】 判断条件 not lesson:，成立时执行对应分支。
            if not lesson:
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='read', tool='read_lesson', args=dict(lessonId=event['lessonId']))]
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'continue'
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 explanations，更新脚本状态。
                explanations = [section for section in lesson['sections'] if section['kind'] == 'explanation' and section.get('presentation') != 'example']
                # 【逐行批注】 计算右侧结果并赋给 interactions，更新脚本状态。
                interactions = [section for section in lesson['sections'] if section.get('questionKind') == 'interaction']
                # 【逐行批注】 计算右侧结果并赋给 practices，更新脚本状态。
                practices = [section for section in lesson['sections'] if section.get('questionKind') == 'practice']
                # 【逐行批注】 计算右侧结果并赋给 index，更新脚本状态。
                index = 1 if event['type'] == 'chat' and '继续' in event.get('text', '') else 0
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='focus', tool='select_teaching_focus', args=dict(lessonId=lesson['id'], sectionId=explanations[index]['id'], interactionSectionId=interactions[index]['id'], practiceSectionId=practices[0]['id']))]
                # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
                message = '请围绕当前段说说你的思路；我会根据你的反馈再决定下一步。'
                if 'yesterdayReview' in event:
                    message = '复习昨天这节课，先独立回忆怎么比较同一种元素的化合价。'
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = 'waiting_student'
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'question_answer':。
        elif event['type'] == 'question_answer':
            # 【逐行批注】 计算右侧结果并赋给 actual_input，更新脚本状态。
            actual_input = next(item for item in context['currentInputs'] if item['requestId'] == event['requestId'])
            # 【逐行批注】 计算右侧结果并赋给 actual，更新脚本状态。
            actual = {'id': actual_input['interactionId'], 'payload': actual_input}
            # 【逐行批注】 判断条件 '选择题' in event['text']:，成立时执行对应分支。
            if '选择题' in event['text']:
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='classify', tool='classify_input', args=dict(interactionId=actual['id'], isAnswer=False, reason='学生希望改用选择题练习')),
                    # 【逐行批注】 计算右侧结果并赋给 dict(id，更新脚本状态。
                    dict(id='followup-choice', tool='append_section', args=dict(lessonId=event['lessonId'], section=dict(kind='question', questionKind='interaction', title='根据你的回答：比较电子得失',
                        # 【逐行批注】 计算右侧结果并赋给 question，更新脚本状态。
                        question=dict(question='Fe 变为 Fe²⁺ 时，下列哪种描述正确？', type='choice', options=['失去两个电子', '得到两个电子'], expectedAnswer='PRIVATE-FOLLOWUP', rubric='PRIVATE-RUBRIC'))))]
                # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
                message = '根据你的回答换一道选择题，请选择电子变化的正确描述。'
            # 【逐行批注】 当前一条件不成立时继续判断：'为什么' in event['text'] or '提示' in event['text']:。
            elif '为什么' in event['text'] or '提示' in event['text']:
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='classify', tool='classify_input', args=dict(interactionId=actual['id'], isAnswer=False, reason='本次输入只有追问'))]
                # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
                message = '这是对思路的追问。我们可以先比较前后状态，再联系电子变化。'
            # 【逐行批注】 当前一条件不成立时继续判断：event.get('action') == 'skip':。
            elif event.get('action') == 'skip':
                # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
                message = '这道题已跳过，不把跳过当成可靠作答。'
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
                actions = [dict(id='evaluate', tool='evaluate_answer', args=dict(interactionId=actual['id'], score=80, credible=True,
                    # 【逐行批注】 计算右侧结果并赋给 feedback，更新脚本状态。
                    feedback='示范：Fe由0升到+2，失去两个电子。' if self.feedback_help_type=='worked_example' else '依据这次实际输入给出反馈，请继续说明判断理由。',
                    # 【逐行批注】 计算右侧结果并赋给 feedbackHelpType，更新脚本状态。
                    feedbackHelpType=self.feedback_help_type, reason='引用本次学生输入', uncertainty='只反映这道题',
                    # 【逐行批注】 计算右侧结果并赋给 diagnosis，更新脚本状态。
                    diagnosis=dict(summary='需要补充电子变化的理由', evidence=[actual['payload']['text']], teachingDirection='用电子得失解释化合价变化', needsClarification=False)))]
                # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
                message = '你已经给出了观察，接下来可以补充电子变化的理由。'
            # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
            state = 'waiting_student'
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'teaching_request':。
        elif event['type'] == 'teaching_request':
            # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
            message = {'hint':'先标出同一种元素的前后化合价，比较变化方向。',
                       # 【逐行批注】 执行当前 Python 语句：'worked_example':'示范：Fe 的化合价由 0 升到 +2，对应失去两个电子。',。
                       'worked_example':'示范：Fe 的化合价由 0 升到 +2，对应失去两个电子。',
                       # 【逐行批注】 执行当前 Python 语句：'rephrase':'换一种讲法：先看前后状态，再解释电子变化。'}.get(event.get('requestType'), '我们继续独立复练。')。
                       'rephrase':'换一种讲法：先看前后状态，再解释电子变化。'}.get(event.get('requestType'), '我们继续独立复练。')
            # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
            state = 'waiting_student'
            # 【逐行批注】 判断条件 self.long_teaching:，成立时执行对应分支。
            if self.long_teaching:
                # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
                message = '\n\n'.join([message] * 60)
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'lesson_finish_request':。
        elif event['type'] == 'lesson_finish_request':
            # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
            message = '请结合本节检查点回顾，已有记录会保留，缺少证据的部分继续练习。'
            # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
            state = 'waiting_student'
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'schedule_replan':。
        elif event['type'] == 'schedule_replan':
            # 【逐行批注】 计算右侧结果并赋给 course，更新脚本状态。
            course = context['courses'][0]['id']
            # 【逐行批注】 计算右侧结果并赋给 availability，更新脚本状态。
            availability = context['studyPlan'].get('requestedAvailability', [dict(weekday=1, minutes=30)])
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [dict(id='candidate', tool='adjust_schedule', args=dict(availability=availability,
                # 【逐行批注】 计算右侧结果并赋给 entries，更新脚本状态。
                entries=[dict(taskId='lesson:'+course+':1:1', courseId=course, kind='lesson', date='2099-01-05', minutes=20, order=0, phaseIndex=1, topicIndex=1)],
                # 【逐行批注】 计算右侧结果并赋给 reason，更新脚本状态。
                reason='按手动请求在可用时间内安排概念课，原课表等待确认'))]
        # 【逐行批注】 当前一条件不成立时继续判断：event['type'] == 'acceptance_tool':。
        elif event['type'] == 'acceptance_tool':
            # 【逐行批注】 计算右侧结果并赋给 actions，更新脚本状态。
            actions = [dict(id='control-check', tool=event['tool'], args=event['args'])]
            # 【逐行批注】 计算右侧结果并赋给 message，更新脚本状态。
            message = '测试调整已保存。你可以从学习导航撤回这次调整，已有作答记录仍保留。'
        # 【逐行批注】 遍历数据并重复执行循环体：for action in actions:。
        for action in actions:
            # 【逐行批注】 计算右侧结果并赋给 section，更新脚本状态。
            section = action.get('args', {}).get('section')
            # 【逐行批注】 判断条件 action.get('tool') == 'append_section' and isinstance(section, d…，成立时执行对应分支。
            if action.get('tool') == 'append_section' and isinstance(section, dict) and section.get('kind') != 'question':
                # 【逐行批注】 执行当前 Python 语句：section.setdefault('helpType', 'explanation')。
                section.setdefault('helpType', 'explanation')
        # 【逐行批注】 计算右侧结果并赋给 help_type，更新脚本状态。
        help_type = event.get('requestType') if event.get('requestType') in ('hint','worked_example') else 'explanation'
        # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
        decision = dict(teaching=dict(helpType=help_type), message=message, actions=actions, state=state)
        if event['type'] == 'chat' and 'material' in event:
            decision.update(material_decision)
        if event['type'] == 'chat' and expected_curriculum:
            decision.update(curriculum_decision)
        self.send_decision(decision)

    def send_decision(self, decision):
        value = json.dumps(decision, ensure_ascii=False)
        # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
        self.send_response(200)
        # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', 'text/event-stream')。
        self.send_header('Content-Type', 'text/event-stream')
        # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
        self.end_headers()
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 遍历数据并重复执行循环体：for index in range(0, len(value), 23):。
            for index in range(0, len(value), 23):
                # 【逐行批注】 计算右侧结果并赋给 chunk，更新脚本状态。
                chunk = dict(model='fixture-v560', choices=[dict(delta=dict(content=value[index:index+23]), finish_reason=None)])
                # 【逐行批注】 计算右侧结果并赋给 self.wfile.write(('data: ' + json.dumps(chun…，更新脚本状态。
                self.wfile.write(('data: ' + json.dumps(chunk, ensure_ascii=False) + '\n\n').encode())
                # 【逐行批注】 执行当前 Python 语句：self.wfile.flush()。
                self.wfile.flush()
                # 【逐行批注】 判断条件 type(self).slow:，成立时执行对应分支。
                if type(self).slow:
                    # 【逐行批注】 执行当前 Python 语句：time.sleep(.12)。
                    time.sleep(.12)
            # 【逐行批注】 计算右侧结果并赋给 self.wfile.write(('data: ' + json.dumps(dict…，更新脚本状态。
            self.wfile.write(('data: ' + json.dumps(dict(model='fixture-v560', choices=[dict(delta={}, finish_reason='stop')])) + '\n\ndata: [DONE]\n\n').encode())
        # 【逐行批注】 捕获并处理异常：except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):。
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass


# 【逐行批注】 应用装饰器 @contextmanager，扩展下一函数或测试的行为。
@contextmanager
# 【逐行批注】 定义函数 start_session，封装可复用的处理步骤。
def start_session(executable=None):
    # 【逐行批注】 计算右侧结果并赋给 repo，更新脚本状态。
    repo = Path(__file__).resolve().parents[1]
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-v560-ui-') as directory:。
    with tempfile.TemporaryDirectory(prefix='gangyi-v560-ui-') as directory:
        # 【逐行批注】 计算右侧结果并赋给 database，更新脚本状态。
        database = Path(directory) / 'fictional.db'
        # 【逐行批注】 计算右侧结果并赋给 ai，更新脚本状态。
        ai = ThreadingHTTPServer(('127.0.0.1', 0), FixtureAI)
        # 【逐行批注】 计算右侧结果并赋给 Thread(target，更新脚本状态。
        Thread(target=ai.serve_forever, daemon=True).start()
        # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as sock:。
        with socket.socket() as sock:
            # 【逐行批注】 执行当前 Python 语句：sock.bind(('127.0.0.1', 0))。
            sock.bind(('127.0.0.1', 0))
            # 【逐行批注】 计算右侧结果并赋给 port，更新脚本状态。
            port = sock.getsockname()[1]
        # 【逐行批注】 计算右侧结果并赋给 base，更新脚本状态。
        base = f'http://127.0.0.1:{port}'
        # 【逐行批注】 计算右侧结果并赋给 log，更新脚本状态。
        log = open(Path(directory) / 'service.log', 'w+', encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
        env = dict(os.environ, DATABASE_PATH=str(database), PORT=str(port), HOST='127.0.0.1', AI_BASE_URL=f'http://127.0.0.1:{ai.server_port}/v1', AI_API_KEY='fictional-fixture-key', AI_MODEL='fixture-v560', GANGYI_LAUNCH_SESSION_ID='v560-ui-isolated')
        # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
        env.update(BOCHA_API_KEY='', DEEPSEEK_SEARCH_API_KEY='', DEEPSEEK_API_KEY='',
                   BOCHA_BASE_URL=env['AI_BASE_URL'], DEEPSEEK_SEARCH_BASE_URL=env['AI_BASE_URL'])
        process = subprocess.Popen([str(Path(executable).resolve() if executable else repo / 'build-windows/gangyiAI.exe')], cwd=repo, env=env, stdout=log, stderr=log, creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        # 【逐行批注】 定义函数 api，封装可复用的处理步骤。
        def api(path, value=None):
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(base+path, data=None if value is None else json.dumps(valu…。
            with urlopen(Request(base+path, data=None if value is None else json.dumps(value, ensure_ascii=False).encode(), headers={'Content-Type':'application/json'}), timeout=15) as response:
                # 【逐行批注】 结束当前函数并返回结果：return json.load(response)。
                return json.load(response)
        # 【逐行批注】 定义函数 wait，封装可复用的处理步骤。
        def wait(task):
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = time.monotonic()+90
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic()<deadline:。
            while time.monotonic()<deadline:
                # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
                value = api('/api/learning-agent?taskId='+task)
                # 【逐行批注】 判断条件 value['status'] not in ('pending','running','idle'):，成立时执行对应分支。
                if value['status'] not in ('pending','running','idle'):
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert value['status'] in ('ready','waiting_student'), value.get('error')。
                    assert value['status'] in ('ready','waiting_student'), value.get('error')
                    # 【逐行批注】 结束当前函数并返回结果：return value。
                    return value
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                time.sleep(.1)
            # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('隔离任务超时')。
            raise RuntimeError('隔离任务超时')
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(300):。
            for _ in range(300):
                # 【逐行批注】 判断条件 process.poll() is not None:，成立时执行对应分支。
                if process.poll() is not None:
                    # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('隔离服务启动失败')。
                    raise RuntimeError('隔离服务启动失败')
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 计算右侧结果并赋给 startup，更新脚本状态。
                    startup = api('/api/learning-agent')
                    # 【逐行批注】 判断条件 startup.get('id'):，成立时执行对应分支。
                    if startup.get('id'):
                        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(startup['id'])。
                        wait(startup['id'])
                        # 【逐行批注】 执行当前 Python 语句：break。
                        break
                # 【逐行批注】 捕获并处理异常：except OSError:。
                except OSError:
                    # 【逐行批注】 执行当前 Python 语句：pass。
                    pass
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                time.sleep(.1)
            # 【逐行批注】 计算右侧结果并赋给 plan，更新脚本状态。
            plan = api('/api/generate-plan', dict(goal='虚构学生的化学学习', mode='lite', requestId='v560-ui-plan'))
            # 【逐行批注】 计算右侧结果并赋给 course，更新脚本状态。
            course = wait(plan['id'])['course']['id']
            # 【逐行批注】 计算右侧结果并赋给 prepare，更新脚本状态。
            prepare = api('/api/learning-agent/events', dict(type='prepare_next', courseId=course, requestId='v560-ui-prepare', navigate=False))
            # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
            lesson = wait(prepare['id'])['lesson']['id']
            # 【逐行批注】 计算右侧结果并赋给 api('/api/learning-agent/control', dict(comm…，更新脚本状态。
            api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=lesson))
            # 【逐行批注】 计算右侧结果并赋给 info，更新脚本状态。
            info = dict(base=base, fixture=f'http://127.0.0.1:{ai.server_port}', database=str(database), pid=process.pid, lessonId=lesson, courseId=course, status='ready')
            # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
            report = repo / 'verification/v560-ui-session.json'
            # 【逐行批注】 计算右侧结果并赋给 report.parent.mkdir(parents，更新脚本状态。
            report.parent.mkdir(parents=True, exist_ok=True)
            # 【逐行批注】 计算右侧结果并赋给 report.write_text(json.dumps(info, ensure_as…，更新脚本状态。
            report.write_text(json.dumps(info, ensure_ascii=False, indent=2), encoding='utf-8')
            # 【逐行批注】 执行当前 Python 语句：yield info, api, process。
            yield info, api, process
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 计算右侧结果并赋给 failed，更新脚本状态。
            failed = sys.exc_info()[0] is not None
            # 【逐行批注】 判断条件 failed:，成立时执行对应分支。
            if failed:
                # 【逐行批注】 输出当前运行结果：print('隔离服务退出状态：' + str(process.poll()), file=sys.stderr)。
                print('隔离服务退出状态：' + str(process.poll()), file=sys.stderr)
            # 【逐行批注】 判断条件 process.poll() is None:，成立时执行对应分支。
            if process.poll() is None:
                # 【逐行批注】 执行当前 Python 语句：process.terminate()。
                process.terminate()
                # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                process.wait(timeout=20)
            # 【逐行批注】 判断条件 failed:，成立时执行对应分支。
            if failed:
                # 【逐行批注】 执行当前 Python 语句：log.flush()。
                log.flush()
                # 【逐行批注】 执行当前 Python 语句：log.seek(0)。
                log.seek(0)
                # 【逐行批注】 输出当前运行结果：print('隔离服务失败日志：\n' + ''.join(log.readlines()[-45:]), file=sys.stderr)。
                print('隔离服务失败日志：\n' + ''.join(log.readlines()[-45:]), file=sys.stderr)
            # 【逐行批注】 执行当前 Python 语句：ai.shutdown()。
            ai.shutdown()
            # 【逐行批注】 执行当前 Python 语句：ai.server_close()。
            ai.server_close()
            # 【逐行批注】 执行当前 Python 语句：log.close()。
            log.close()


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 在受控上下文中执行资源操作：with start_session() as (info, api, process):。
        with start_session() as (info, api, process):
            # 【逐行批注】 输出当前运行结果：print(json.dumps(info, ensure_ascii=False), flush=True)。
            print(json.dumps(info, ensure_ascii=False), flush=True)
            # 【逐行批注】 在条件成立期间重复执行：while process.poll() is None:。
            while process.poll() is None:
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.5)。
                time.sleep(.5)
    # 【逐行批注】 捕获并处理异常：except KeyboardInterrupt:。
    except KeyboardInterrupt:
        # 【逐行批注】 执行当前 Python 语句：pass。
        pass


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'):，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'):
        # 【逐行批注】 计算右侧结果并赋给 sys.stdout.reconfigure(encoding，更新脚本状态。
        sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()

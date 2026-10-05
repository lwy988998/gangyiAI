"""统一主控的隔离接口验收；模型协议夹具只验证真实 HTTP 调用与工具执行。"""
import json
import os
import socket
import sqlite3
import subprocess
import sys
import tempfile
import time
from contextlib import closing
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Thread
from urllib.error import HTTPError
from urllib.parse import quote
from urllib.request import Request, urlopen


def port():
    with socket.socket() as connection:
        connection.bind(('127.0.0.1', 0))
        return connection.getsockname()[1]


class ProtocolAI(BaseHTTPRequestHandler):
    calls = []
    repaired = set()

    def log_message(self, *_):
        pass

    def do_POST(self):
        request = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        assert '教学主控' in request['messages'][0]['content'], '运行时不能启动独立教学模型'
        context = json.loads(request['messages'][1]['content'])
        event = context['event']
        type(self).calls.append(event['requestId'])
        results = []
        for item in request['messages'][2:]:
            if item['role'] == 'user':
                try:
                    results.extend(json.loads(item['content']).get('toolResults', []))
                except ValueError:
                    pass
        actions, state, message = [], 'completed', '模型依据已保存的真实记录完成本次处理。'
        if event['type'] == 'startup':
            actions = [dict(id='recommend', tool='update_recommendations', args=dict(
                lite=['认识化合价', '理解函数'], deep=['氧化还原', '函数概念'], reason='无作答时仅提出学习方向'))]
        elif event['type'] == 'plan_course':
            actions = [dict(id='course', tool='create_course', args=dict(goal='化合价', title='自主两主题课程', mode='lite',
                summary='按学生实际反馈调整', payload=dict(courseStructure=[dict(stage='概念', topics=['化合价', '氧化还原'])])))]
        elif event['type'] == 'prepare_next':
            lesson = next((item['result']['lessonId'] for item in results if 'lessonId' in item['result']), None)
            if not lesson:
                actions = [dict(id='create', tool='create_lesson', args=dict(courseId=event['courseId'], title='自主课堂', purpose='对比概念'))]
                state = 'continue'
            elif not any('sectionId' in item['result'] for item in results):
                actions = [dict(id='question-' + kind, tool='append_section', args=dict(lessonId=lesson,
                    section=dict(kind='question', questionKind=kind, title=kind + '题', question=dict(
                        question=kind + '：铁变为亚铁离子时化合价如何变化？', type='choice', options=['升高', '降低'],
                        expectedAnswer='PRIVATE-EXPECTED', rubric='PRIVATE-RUBRIC'))))
                    for kind in ('diagnostic', 'interaction', 'example', 'practice', 'quiz', 'review')]
                state = 'continue'
            else:
                actions = [dict(id='finish', tool='finish_lesson', args=dict(lessonId=lesson))]
                state = 'waiting_student'
        elif event['type'] == 'question_answer':
            inputs = event.get('batchAnswers', [event])
            for index, item in enumerate(inputs):
                if item.get('action') in ('hint', 'skip', 'omitted'):
                    continue
                actual = next(value for value in context['feedback'] if value['payload'].get('requestId') == item['requestId'])
                if item.get('text') == '为什么这样判断？':
                    actions.append(dict(id='classify-' + str(index), tool='classify_input', args=dict(
                        interactionId=actual['id'], isAnswer=False, reason='这是对已给出反馈的追问')))
                else:
                    actions.append(dict(id='evaluate-' + str(index), tool='evaluate_answer', args=dict(interactionId=actual['id'],
                        score=70, credible=True, feedback='依据实际作答给出评价', reason='学生指出化合价升高', uncertainty='仅反映本题表现')))
            message = '先评价实际输入，再解释电子变化。'
            state = 'waiting_student'
        elif event['type'] == 'acceptance_tool':
            actions = [dict(id='action', tool=event['tool'], args=event['args'])]
        elif event['type'] == 'acceptance_fail':
            self.send_response(503)
            self.end_headers()
            return
        value = json.dumps(dict(message=message, actions=actions, state=state), ensure_ascii=False)
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream')
        self.end_headers()
        try:
            for offset in range(0, len(value), 13):
                chunk = dict(model='fixture-agent-http', choices=[dict(delta=dict(content=value[offset:offset + 13]), finish_reason=None)])
                self.wfile.write(('data: ' + json.dumps(chunk, ensure_ascii=False) + '\n\n').encode())
                self.wfile.flush()
            interrupted = event.get('failureAfterEvaluation') and event['requestId'] not in type(self).repaired and any('evaluationId' in item.get('result', {}) for item in results)
            self.wfile.write(('data: ' + json.dumps(dict(model='fixture-agent-http', choices=[dict(delta={}, finish_reason='length' if interrupted else 'stop')])) + '\n\ndata: [DONE]\n\n').encode())
        except (BrokenPipeError, ConnectionResetError):
            pass


def main(executable):
    repo = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='gangyi-agent-api-') as directory:
        database = Path(directory) / 'fictional.db'
        ai = ThreadingHTTPServer(('127.0.0.1', port()), ProtocolAI)
        Thread(target=ai.serve_forever, daemon=True).start()
        base = f'http://127.0.0.1:{port()}'
        env = dict(os.environ, DATABASE_PATH=str(database), PORT=base.rsplit(':', 1)[1], HOST='127.0.0.1',
            AI_BASE_URL=f'http://127.0.0.1:{ai.server_port}/v1', AI_API_KEY='fictional-key', AI_MODEL='fixture-agent-http',
            LOCAL_CONTROL_TOKEN='fictional-control', GANGYI_LAUNCH_SESSION_ID='fixture-api-startup')
        log = open(Path(directory) / 'service.log', 'w+', encoding='utf-8')
        process = subprocess.Popen([str(Path(executable).resolve())], cwd=repo, env=env, stdout=log, stderr=log,
            creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        serial = 0
        def api(path, value=None, status=200):
            request = Request(base + path, data=None if value is None else json.dumps(value).encode(), headers={'Content-Type': 'application/json'})
            try:
                with urlopen(request, timeout=15) as response:
                    assert response.status == status, (path, response.status)
                    return json.load(response)
            except HTTPError as error:
                assert error.code == status, (path, error.code, error.read().decode())
                return json.load(error)
        def wait(task_id='', expected='ready'):
            for _ in range(500):
                value = api('/api/learning-agent' + ('?taskId=' + quote(task_id) if task_id else ''))
                if value['status'] not in ('idle', 'pending', 'running'):
                    assert value['status'] == expected, {key: value.get(key) for key in ('status', 'error', 'calls')}
                    return value
                time.sleep(.03)
            raise AssertionError('主控任务未结束')
        def submit(event, expected='ready'):
            nonlocal serial
            serial += 1
            event.setdefault('requestId', 'fixture-' + str(serial))
            task = api('/api/learning-agent/events', event, 202)
            return wait(task['id'], expected)
        def tool(name, args):
            return submit(dict(type='acceptance_tool', tool=name, args=args))
        try:
            for _ in range(200):
                try:
                    api('/health')
                    break
                except OSError:
                    time.sleep(.1)
            wait()
            recommendation = api('/api/home/recommendations')
            assert recommendation['items']['lite'] == ['认识化合价', '理解函数']
            calls = len(ProtocolAI.calls)
            for _ in range(3):
                api('/api/home/recommendations'); api('/api/profile'); api('/api/learning-agent')
            assert len(ProtocolAI.calls) == calls
            planned = api('/api/generate-plan', dict(goal='化合价', mode='lite', requestId='plan'), 202)
            planned = wait(planned['id']); course = planned['course']['id']
            outline = api('/api/courses/' + course)['snapshot']['payload']
            assert len(outline['courseStructure']) == 1 and len(outline['courseStructure'][0]['topics']) == 2
            assert outline['generation']['model'] == 'fixture-agent-http'
            prepared = submit(dict(type='prepare_next', courseId=course, navigate=True))
            lesson = prepared['lesson']['id']
            shown = api('/api/learning-agent/lesson?lessonId=' + lesson)
            assert len(shown['sections']) == 6 and 'PRIVATE-' not in json.dumps(shown)
            api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=lesson))
            same = api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=lesson))
            assert same['id'] == lesson
            for section in shown['sections']:
                answered = submit(dict(type='question_answer', courseId=course, lessonId=lesson, sectionId=section['id'],
                    sectionVersion=section['version'], text='化合价升高'), 'waiting_student')
                assert answered['calls'] == 2, '全部题型必须先评价，再发起讲解请求'
                api('/api/learn/exposure', dict(taskId=answered['id'], seq=answered['seq']))
            first = shown['sections'][0]
            before = api('/api/learning-agent/lesson?lessonId=' + lesson)
            assert len(before['sections'][0]['dialog']) == 2
            followup = submit(dict(type='question_answer', courseId=course, lessonId=lesson, sectionId=first['id'],
                sectionVersion=first['version'], text='为什么这样判断？'), 'waiting_student')
            api('/api/learn/exposure', dict(taskId=followup['id'], seq=followup['seq']))
            with closing(sqlite3.connect(database)) as db:
                grades = [json.loads(row[0]) | {'id': row[1]} for row in db.execute("SELECT payload,id FROM LearningInteraction WHERE kind='practice'")]
                assert len(grades) == 6, '追问不能新增一道已评分题'
            interrupted = submit(dict(type='question_answer', courseId=course, lessonId=lesson, sectionId=first['id'],
                sectionVersion=first['version'], text='化合价升高', requestId='interrupted-answer', failureAfterEvaluation=True), 'failed')
            with closing(sqlite3.connect(database)) as db:
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practice'").fetchone()[0] == 6, '讲解中断不能提交暂存评价'
            api('/api/learn/exposure', dict(taskId=interrupted['id'], seq=interrupted['seq']))
            partial = api('/api/learning-agent/lesson?lessonId=' + lesson)
            assert any(item.get('partial') for item in partial['sections'][0]['dialog']), '实际看过的中断讲解应恢复'
            ProtocolAI.repaired.add('interrupted-answer')
            api('/api/learning-agent/control', dict(command='retry', taskId=interrupted['id']))
            wait(interrupted['id'], 'waiting_student')
            with closing(sqlite3.connect(database)) as db:
                new_grades = [json.loads(row[0]) | {'id': row[1]} for row in db.execute("SELECT payload,id FROM LearningInteraction WHERE kind='practice'")]
                assert len(new_grades) == 7 and any(item.get('assisted') for item in new_grades)
            grade = grades[0]
            rating = dict(score=71, sufficient=True, evidenceIds=[grade['id']], rationale='引用一道适用的真实回答', recommendation='继续概念辨析', uncertainty='仅反映化合价本题')
            tool('update_profiles', dict(subjects=[dict(rating, subject='化学')], abilities=[dict(rating, id='understanding')]))
            profile = api('/api/profile')
            assert next(item for item in profile['subjects'] if item['subject'] == '化学')['evidenceCount'] == 1
            assert next(item for item in profile['abilities'] if item['id'] == 'understanding')['score'] == 71
            # 同一道题多轮仍只有一道证据；充分性由模型判断。
            repeated = [item['id'] for item in new_grades if item['questionId'] == grade['questionId']]
            assert len(repeated) == 2
            rating['evidenceIds'] = repeated
            tool('update_profiles', dict(abilities=[dict(rating, id='understanding')]))
            assert next(item for item in api('/api/profile')['abilities'] if item['id'] == 'understanding')['evidenceCount'] == 1
            # 未进入的未来主题可以重排，实际进入后撤回不能删除该内容。
            adjusted = json.loads(json.dumps(outline))
            adjusted['courseStructure'][0]['topics'].reverse()
            reordered = tool('revise_outline', dict(courseId=course, payload=adjusted, reason='模型选择先比较电子，再梳理概念'))
            undo_outline = api('/api/learning-agent/control', dict(command='undo', changeId=reordered['changes'][0]['id']))
            wait(undo_outline['id'])
            restored_outline = api('/api/courses/' + course)['snapshot']['payload']
            assert restored_outline['courseStructure'][0]['topics'][0]['id'] == outline['courseStructure'][0]['topics'][0]['id']
            changed = tool('adjust_goal', dict(courseId=course, goal='先理解电子变化', reason='真实反馈显示需补齐概念'))
            change = changed['changes'][0]['id']
            undo = api('/api/learning-agent/control', dict(command='undo', changeId=change))
            wait(undo['id'])
            assert api('/api/courses/' + course)['course']['goal'] == '化合价'
            schedule = tool('schedule_review', dict(courseId=course, title='根据真实作答复习', due='2099-01-05', reason='模型自主选择日期', lessonId=lesson))
            assert schedule['status'] == 'ready'
            with closing(sqlite3.connect(database)) as db:
                reviews = [json.loads(row[0]) for row in db.execute("SELECT payload FROM ClassroomActivity WHERE kind='review'")]
                assert len(reviews) == 1 and reviews[0]['due'] == '2099-01-05'
            chat = submit(dict(type='chat', courseId=course, lessonId=lesson, text='请用日常例子解释'), 'ready')
            api('/api/learn/exposure', dict(taskId=chat['id'], seq=chat['seq']))
            restored = api('/api/learning-agent/lesson?lessonId=' + lesson)
            assert [item['role'] for item in restored['dialog']] == ['user', 'assistant']
            # 批量兼容入口区分漏答和不会；同一请求重放不会新增输入或评价。
            batch = dict(courseId=course, lessonId=lesson, kind='quiz', answers=[None], requestId='batch-omitted')
            omitted = api('/api/quiz-attempts', batch, 202); wait(omitted['id'], 'waiting_student')
            with closing(sqlite3.connect(database)) as db:
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practice'").fetchone()[0] == 7
            batch.update(answers=['unknown'], requestId='batch-unknown')
            unknown = api('/api/quiz-attempts', batch, 202); wait(unknown['id'], 'waiting_student')
            calls = len(ProtocolAI.calls)
            assert api('/api/quiz-attempts', batch, 202)['id'] == unknown['id']
            time.sleep(.2); assert len(ProtocolAI.calls) == calls
            with closing(sqlite3.connect(database)) as db:
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practice'").fetchone()[0] == 8
                actions = [json.loads(row[0]).get('action') for row in db.execute("SELECT payload FROM LearningInteraction WHERE kind='chat-user'")]
                assert 'omitted' in actions and 'unknown' in actions
            failed = submit(dict(type='acceptance_fail'), 'failed')
            assert failed['calls'] == 3
            count = len(ProtocolAI.calls); time.sleep(.3); api('/api/learning-agent?taskId=' + failed['id'])
            assert len(ProtocolAI.calls) == count
            paused = api('/api/learning-agent/control', dict(command='pause', taskId=failed['id']))
            assert paused['status'] == 'paused'
            # 拒绝不存在的题目标识，原输入和可靠评价不能被错题覆盖。
            api('/api/classroom/submit', dict(courseId=course, lessonId=lesson, sectionId='does-not-exist', kind='diagnostic', index=0, answer='升高'), 409)
            api('/api/learning-agent/lesson?lessonId=' + lesson)
            with closing(sqlite3.connect(database)) as db:
                assert db.execute('PRAGMA quick_check').fetchone()[0] == 'ok'
            # v5.3/v5.4 的原课堂与连续对话保持身份，公开内容不泄露旧答案。
            old_content = dict(blocks=dict(steps=dict(lessonSteps=[dict(title='原讲解', explanation='旧课件说明')]),
                practice=dict(practice=[dict(question='旧题：比较化合价', expectedAnswer='PRIVATE-OLD')]),
                quiz=dict(quiz=[dict(question='旧测验', options=['甲', '乙'], answerIndex=0)])), contentVersion=3)
            old_dialog = dict(turns=[dict(user='我原来写过的回答', assistant='我实际看过的旧讲解', status='ready')])
            with closing(sqlite3.connect(database)) as db:
                with db:
                    db.execute('INSERT INTO LearningSession(id,courseId,goal,mode,phaseIndex,phaseName,topicIndex,topicTitle,title,content) VALUES(?,?,?,?,?,?,?,?,?,?)',
                        ('old-session', course, '化合价', 'lite', 1, '概念', 1, '化合价', '原有课堂', json.dumps(old_content)))
                    db.execute('INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)',
                        ('classroom:' + course + ':1:1:practice:0:dialogue', course, 'dialogue', json.dumps(old_dialog), '2026-01-01', 1, 1))
            old = api('/api/learn?courseId=' + course + '&phaseIndex=1&topicIndex=1')
            assert old['id'] == 'legacy-old-session' and 'PRIVATE-' not in json.dumps(old)
            old_question = next(item for item in old['sections'] if item.get('legacyKind') == 'practice')
            assert [item['text'] for item in old_question['dialog']] == ['我原来写过的回答', '我实际看过的旧讲解']
            assert api('/api/learn?courseId=' + course + '&phaseIndex=1&topicIndex=1')['id'] == old['id']
            with closing(sqlite3.connect(database)) as db:
                assert json.loads(db.execute("SELECT content FROM LearningSession WHERE id='old-session'").fetchone()[0]) == old_content
            print('主控 HTTP、多步自主规划、六类题型、先评价后讲解、追问不评分、一题画像、撤回、自由复习日期、对话恢复、缓存与三次失败停止通过（协议夹具）')
        finally:
            if process.poll() is None:
                try:
                    with urlopen(Request(base + '/internal/shutdown', data=b'', headers={'X-Gangyi-Control-Token': 'fictional-control'}), timeout=10):
                        pass
                    process.wait(timeout=15)
                except Exception:
                    process.terminate(); process.wait(timeout=15)
            ai.shutdown(); ai.server_close(); log.close()


if __name__ == '__main__':
    main(sys.argv[1])

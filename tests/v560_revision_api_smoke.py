# 【逐行批注】 执行当前 Python 语句："""本轮入口、课表权限、候选确认和出题衔接的隔离 HTTP 回归。"""。
"""本轮入口、课表权限、候选确认和出题衔接的隔离 HTTP 回归。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.error import HTTPError。
from urllib.error import HTTPError
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen
# 【逐行批注】 导入后续逻辑需要的模块或符号：from classroom_v560_fixture import start_session。
from classroom_v560_fixture import start_session


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable):
    # 【逐行批注】 在受控上下文中执行资源操作：with start_session(executable) as (info, api, process):。
    with start_session(executable) as (info, api, process):
        # 【逐行批注】 定义函数 request，封装可复用的处理步骤。
        def request(path, value=None, method=None, expected=200, host=None):
            # 【逐行批注】 计算右侧结果并赋给 req，更新脚本状态。
            req = Request((host or info['base']) + path, data=None if value is None else json.dumps(value, ensure_ascii=False).encode(),
                          # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                          headers={'Content-Type': 'application/json'}, method=method)
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(req, timeout=15) as response:。
                with urlopen(req, timeout=15) as response:
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert response.status == expected, (path, response.status)。
                    assert response.status == expected, (path, response.status)
                    # 【逐行批注】 计算右侧结果并赋给 raw，更新脚本状态。
                    raw = response.read()
            # 【逐行批注】 捕获并处理异常：except HTTPError as error:。
            except HTTPError as error:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert error.code == expected, (path, error.code)。
                assert error.code == expected, (path, error.code)
                # 【逐行批注】 计算右侧结果并赋给 raw，更新脚本状态。
                raw = error.read()
            # 【逐行批注】 结束当前函数并返回结果：return json.loads(raw) if raw.startswith(b'{') else raw.decode('utf-8…。
            return json.loads(raw) if raw.startswith(b'{') else raw.decode('utf-8')

        # 【逐行批注】 定义函数 wait，封装可复用的处理步骤。
        def wait(task, expected='ready'):
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = time.monotonic() + 90
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
            while time.monotonic() < deadline:
                # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                state = api('/api/learning-agent?taskId=' + task)
                # 【逐行批注】 判断条件 state['status'] not in ('pending', 'running'):，成立时执行对应分支。
                if state['status'] not in ('pending', 'running'):
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['status'] == expected, (state['status'], state.get('error'))。
                    assert state['status'] == expected, (state['status'], state.get('error'))
                    # 【逐行批注】 结束当前函数并返回结果：return state。
                    return state
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.05)。
                time.sleep(.05)
            # 【逐行批注】 在错误情形抛出异常：raise AssertionError('隔离任务超时')。
            raise AssertionError('隔离任务超时')

        # 【逐行批注】 计算右侧结果并赋给 course，更新脚本状态。
        course = info['courseId']
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/courses')['currentCourse']['id'] == course。
        assert api('/api/courses')['currentCourse']['id'] == course
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert info['courseId'] in request('/current-course'), '有效当前课程入口应进入真实课程总览'。
        assert info['courseId'] in request('/current-course'), '有效当前课程入口应进入真实课程总览'
        # 【逐行批注】 计算右侧结果并赋给 other，更新脚本状态。
        other = api('/api/generate-plan', dict(goal='第二门虚构课程', mode='lite', requestId='revision-other-course'))
        # 【逐行批注】 计算右侧结果并赋给 other_course，更新脚本状态。
        other_course = wait(other['id'])['course']['id']
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/courses')['currentCourse']['id'] == course, '建课不能冒充访问记录'。
        assert api('/api/courses')['currentCourse']['id'] == course, '建课不能冒充访问记录'
        # 【逐行批注】 计算右侧结果并赋给 request('/plan?courseId，更新脚本状态。
        request('/plan?courseId=' + other_course)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/courses')['currentCourse']['id'] == other_course。
        assert api('/api/courses')['currentCourse']['id'] == other_course
        # 【逐行批注】 计算右侧结果并赋给 request('/plan?courseId，更新脚本状态。
        request('/plan?courseId=' + course)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/courses')['currentCourse']['id'] == course, '同一秒访问不同课程也须按实际顺序恢…。
        assert api('/api/courses')['currentCourse']['id'] == course, '同一秒访问不同课程也须按实际顺序恢复'

        # 【逐行批注】 计算右侧结果并赋给 availability，更新脚本状态。
        availability = [dict(weekday=1, minutes=30)]
        # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
        before = api('/api/study-plan')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert before['status'] == 'ready', '尚未手动排课时不能永久显示正在生成'。
        assert before['status'] == 'ready', '尚未手动排课时不能永久显示正在生成'
        # 【逐行批注】 计算右侧结果并赋给 args，更新脚本状态。
        args = dict(availability=availability, entries=[], reason='试图从普通事件重排')
        # 【逐行批注】 遍历数据并重复执行循环体：for event_type in ('startup', 'chat', 'prepare_next', 'question_answer'):。
        for event_type in ('startup', 'chat', 'prepare_next', 'question_answer'):
            # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
            event = dict(type=event_type, courseId=course, attemptSchedule=args, text='普通教学事件的权限回归',
                # 【逐行批注】 计算右侧结果并赋给 requestId，更新脚本状态。
                requestId='denied-'+event_type, manualScheduleReplan=True, origin='manualScheduleReplan')
            # 【逐行批注】 判断条件 event_type in ('chat', 'question_answer'):，成立时执行对应分支。
            if event_type in ('chat', 'question_answer'):
                # 【逐行批注】 计算右侧结果并赋给 event['lessonId']，更新脚本状态。
                event['lessonId'] = info['lessonId']
            # 【逐行批注】 判断条件 event_type == 'question_answer':，成立时执行对应分支。
            if event_type == 'question_answer':
                # 【逐行批注】 计算右侧结果并赋给 section，更新脚本状态。
                section = next(s for s in api('/api/learning-agent/lesson?lessonId='+info['lessonId'])['sections'] if s.get('questionKind') == 'practice')
                # 【逐行批注】 计算右侧结果并赋给 event.update(sectionId，更新脚本状态。
                event.update(sectionId=section['id'], sectionVersion=section['version'], view='practice')
            # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
            task = api('/api/learning-agent/events', event)
            # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(task['id'], 'failed')。
            wait(task['id'], 'failed')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/study-plan')['entries'] == before['entries']。
            assert api('/api/study-plan')['entries'] == before['entries']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/study-plan')['proposal'] is None, '客户端字段不能扩大排课权限'。
            assert api('/api/study-plan')['proposal'] is None, '客户端字段不能扩大排课权限'
        # 【逐行批注】 计算右侧结果并赋给 spoof，更新脚本状态。
        spoof = api('/api/learning-agent/events', dict(type='schedule_replan', requestId='spoofed-manual', manualScheduleReplan=True))
        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(spoof['id'], 'failed')。
        wait(spoof['id'], 'failed')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/study-plan')['proposal'] is None。
        assert api('/api/study-plan')['proposal'] is None

        # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
        body = dict(version=before['version'], availability=availability, requestId='revision-replan-cancel')
        # 【逐行批注】 计算右侧结果并赋给 first，更新脚本状态。
        first = api('/api/study-plan/replan', body)
        # 【逐行批注】 计算右侧结果并赋给 duplicate，更新脚本状态。
        duplicate = api('/api/study-plan/replan', body)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert duplicate['task']['id'] == first['task']['id']。
        assert duplicate['task']['id'] == first['task']['id']
        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(first['task']['id'])。
        wait(first['task']['id'])
        # 【逐行批注】 计算右侧结果并赋给 candidate，更新脚本状态。
        candidate = api('/api/study-plan')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert candidate['entries'] == before['entries'] and candidate['proposal']['pla…。
        assert candidate['entries'] == before['entries'] and candidate['proposal']['plan']['entries']
        # 【逐行批注】 计算右侧结果并赋给 candidate_dates，更新脚本状态。
        candidate_dates = [entry['date'] for entry in candidate['proposal']['plan']['entries']]
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(isinstance(entry.get('title'), str) and entry['title'] for entry in …。
        assert all(isinstance(entry.get('title'), str) and entry['title'] for entry in candidate['proposal']['plan']['entries'])
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert candidate['proposal']['plan']['weekStart'] == min(candidate_dates)。
        assert candidate['proposal']['plan']['weekStart'] == min(candidate_dates)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert candidate['proposal']['plan']['weekEnd'] == max(candidate_dates)。
        assert candidate['proposal']['plan']['weekEnd'] == max(candidate_dates)
        # 【逐行批注】 计算右侧结果并赋给 proposal，更新脚本状态。
        proposal = candidate['proposal']['id']
        # 【逐行批注】 计算右侧结果并赋给 calls_before，更新脚本状态。
        calls_before = request('/', host=info['fixture'])['calls']
        # 【逐行批注】 计算右侧结果并赋给 cancelled，更新脚本状态。
        cancelled = api('/api/study-plan/cancel', dict(version=candidate['version'], proposalId=proposal))['plan']
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert cancelled['entries'] == before['entries'] and cancelled['proposal'] is N…。
        assert cancelled['entries'] == before['entries'] and cancelled['proposal'] is None
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert request('/', host=info['fixture'])['calls'] == calls_before, '取消不能再调用模型'。
        assert request('/', host=info['fixture'])['calls'] == calls_before, '取消不能再调用模型'

        # 【逐行批注】 计算右侧结果并赋给 second，更新脚本状态。
        second = api('/api/study-plan/replan', dict(version=cancelled['version'], availability=availability, requestId='revision-replan-confirm'))
        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(second['task']['id'])。
        wait(second['task']['id'])
        # 【逐行批注】 计算右侧结果并赋给 candidate，更新脚本状态。
        candidate = api('/api/study-plan')
        # 【逐行批注】 计算右侧结果并赋给 planned_entries，更新脚本状态。
        planned_entries = candidate['proposal']['plan']['entries']
        # 【逐行批注】 计算右侧结果并赋给 confirm_body，更新脚本状态。
        confirm_body = dict(version=candidate['version'], proposalId=candidate['proposal']['id'])
        # 【逐行批注】 计算右侧结果并赋给 calls_before，更新脚本状态。
        calls_before = request('/', host=info['fixture'])['calls']
        # 【逐行批注】 计算右侧结果并赋给 confirmed，更新脚本状态。
        confirmed = api('/api/study-plan/confirm', confirm_body)['plan']
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert confirmed['entries'] == planned_entries and confirmed['version'] == cand…。
        assert confirmed['entries'] == planned_entries and confirmed['version'] == candidate['version'] + 1
        # 【逐行批注】 计算右侧结果并赋给 request('/api/study-plan/confirm', confirm_b…，更新脚本状态。
        request('/api/study-plan/confirm', confirm_body, expected=409)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/study-plan')['entries'] == planned_entries。
        assert api('/api/study-plan')['entries'] == planned_entries
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert request('/', host=info['fixture'])['calls'] == calls_before, '确认必须应用原候选，…。
        assert request('/', host=info['fixture'])['calls'] == calls_before, '确认必须应用原候选，不能再生成'
        # 【逐行批注】 计算右侧结果并赋给 stale，更新脚本状态。
        stale = api('/api/study-plan/replan', dict(version=confirmed['version'], availability=availability, requestId='revision-replan-stale'))
        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(stale['task']['id'])。
        wait(stale['task']['id'])
        # 【逐行批注】 计算右侧结果并赋给 candidate，更新脚本状态。
        candidate = api('/api/study-plan')
        # 【逐行批注】 计算右侧结果并赋给 edited，更新脚本状态。
        edited = request('/api/study-plan', dict(version=candidate['version'], availability=[dict(weekday=1, minutes=40)]), method='PUT')
        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(edited['task']['id'])。
        wait(edited['task']['id'])
        # 【逐行批注】 计算右侧结果并赋给 request('/api/study-plan/confirm', dict(vers…，更新脚本状态。
        request('/api/study-plan/confirm', dict(version=candidate['version'], proposalId=candidate['proposal']['id']), expected=409)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/study-plan')['availability'] == [dict(weekday=1, minutes=40)]。
        assert api('/api/study-plan')['availability'] == [dict(weekday=1, minutes=40)]

        # 【逐行批注】 计算右侧结果并赋给 before_fail，更新脚本状态。
        before_fail = api('/api/study-plan')
        # 【逐行批注】 计算右侧结果并赋给 request('/fixture-control', dict(http_error，更新脚本状态。
        request('/fixture-control', dict(http_error=402), host=info['fixture'])
        # 【逐行批注】 计算右侧结果并赋给 failed_task，更新脚本状态。
        failed_task = api('/api/study-plan/replan', dict(version=before_fail['version'], availability=availability, requestId='revision-replan-402'))
        # 【逐行批注】 计算右侧结果并赋给 failed，更新脚本状态。
        failed = wait(failed_task['task']['id'], 'failed')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert failed['calls'] == 1 and failed['failure']['httpStatus'] == 402 and not …。
        assert failed['calls'] == 1 and failed['failure']['httpStatus'] == 402 and not failed['failure']['retryable']
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/study-plan')['status'] == 'failed'。
        assert api('/api/study-plan')['status'] == 'failed'
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/study-plan')['entries'] == before_fail['entries']。
        assert api('/api/study-plan')['entries'] == before_fail['entries']
        # 【逐行批注】 计算右侧结果并赋给 request('/fixture-control', {}, host，更新脚本状态。
        request('/fixture-control', {}, host=info['fixture'])
        # 【逐行批注】 计算右侧结果并赋给 api('/api/learning-agent/control', dict(comm…，更新脚本状态。
        api('/api/learning-agent/control', dict(command='retry', taskId=failed['id']))
        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(failed['id'])。
        wait(failed['id'])
        # 【逐行批注】 计算右侧结果并赋给 recovered，更新脚本状态。
        recovered = api('/api/study-plan')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert recovered['proposal'] and recovered['entries'] == before_fail['entries']。
        assert recovered['proposal'] and recovered['entries'] == before_fail['entries']
        # 【逐行批注】 计算右侧结果并赋给 api('/api/study-plan/cancel', dict(version，更新脚本状态。
        api('/api/study-plan/cancel', dict(version=recovered['version'], proposalId=recovered['proposal']['id']))

        # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
        lesson = api('/api/learning-agent/lesson?lessonId=' + info['lessonId'])
        # 【逐行批注】 计算右侧结果并赋给 original，更新脚本状态。
        original = next(s for s in lesson['sections'] if s.get('questionKind') == 'practice')
        # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
        event = dict(type='question_answer', lessonId=lesson['id'], courseId=course, sectionId=original['id'],
                     # 【逐行批注】 计算右侧结果并赋给 sectionVersion，更新脚本状态。
                     sectionVersion=original['version'], view='practice', text='请根据我的回答出一道选择题。', requestId='revision-choice')
        # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
        task = api('/api/learning-agent/events', event)
        # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(task['id'], 'waiting_student')。
        wait(task['id'], 'waiting_student')
        # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(2):。
        for _ in range(2):
            # 【逐行批注】 计算右侧结果并赋给 view，更新脚本状态。
            view = api('/api/learning-agent/lesson?lessonId=' + lesson['id'])
            # 【逐行批注】 计算右侧结果并赋给 focus，更新脚本状态。
            focus = view['teachingFocus']
            # 【逐行批注】 计算右侧结果并赋给 new_question，更新脚本状态。
            new_question = next(s for s in view['sections'] if s['id'] == focus['responseSectionId'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert focus['responseView'] == 'practice' and new_question['questionKind'] == …。
            assert focus['responseView'] == 'practice' and new_question['questionKind'] == 'interaction'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(new_question['question']['options']) == 2 and 'expectedAnswer' not i…。
            assert len(new_question['question']['options']) == 2 and 'expectedAnswer' not in json.dumps(view)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learning-agent/events', event)['id'] == task['id'], '重复请求不能追加重…。
        assert api('/api/learning-agent/events', event)['id'] == task['id'], '重复请求不能追加重复题目'

        # 重启同一隔离档案，验证当前课程由服务端持久化恢复。
        # 【逐行批注】 执行当前 Python 语句：process.terminate()。
        process.terminate()
        # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
        process.wait(timeout=20)
        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
        env = dict(os.environ, DATABASE_PATH=info['database'], PORT=info['base'].rsplit(':', 1)[1], HOST='127.0.0.1',
                   # 【逐行批注】 计算右侧结果并赋给 AI_BASE_URL，更新脚本状态。
                   AI_BASE_URL=info['fixture']+'/v1', AI_API_KEY='fictional-fixture-key', AI_MODEL='fixture-v560', GANGYI_LAUNCH_SESSION_ID='revision-restart')
        # 【逐行批注】 计算右侧结果并赋给 restarted，更新脚本状态。
        restarted = subprocess.Popen([str(Path(executable).resolve())], cwd=Path(__file__).resolve().parents[1], env=env,
            # 【逐行批注】 计算右侧结果并赋给 stdout，更新脚本状态。
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(200):。
            for _ in range(200):
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 调用 api 完成当前脚本或测试步骤：api('/health')。
                    api('/health')
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
                # 【逐行批注】 捕获并处理异常：except OSError:。
                except OSError:
                    # 【逐行批注】 执行当前 Python 语句：time.sleep(.05)。
                    time.sleep(.05)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/courses')['currentCourse']['id'] == course。
            assert api('/api/courses')['currentCourse']['id'] == course
            # 【逐行批注】 计算右侧结果并赋给 startup，更新脚本状态。
            startup = api('/api/learning-agent')
            # 【逐行批注】 判断条件 startup['status'] in ('pending', 'running'):，成立时执行对应分支。
            if startup['status'] in ('pending', 'running'):
                # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(startup['id'])。
                wait(startup['id'])
            # 【逐行批注】 计算右侧结果并赋给 deletion_task，更新脚本状态。
            deletion_task = api('/api/learning-agent/events', dict(type='chat', courseId=course,
                # 【逐行批注】 计算右侧结果并赋给 lessonId，更新脚本状态。
                lessonId=info['lessonId'], requestId='revision-delete-latest-task', text='删除前检查当前课时'))
            # 【逐行批注】 调用 wait 完成当前脚本或测试步骤：wait(deletion_task['id'], 'waiting_student')。
            wait(deletion_task['id'], 'waiting_student')
            # 删除记录也会触发合法的后台学习更新；先暂停该作用域，单独验证失效最近标记。
            # 【逐行批注】 计算右侧结果并赋给 api('/api/learning-agent/control', dict(comm…，更新脚本状态。
            api('/api/learning-agent/control', dict(command='pause', taskId=deletion_task['id']))
            # 【逐行批注】 计算右侧结果并赋给 request('/api/my-courses/'+course, method，更新脚本状态。
            request('/api/my-courses/'+course, method='DELETE')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learning-agent')['status'] == 'idle', '删课后失效的最近任务应回到空闲状态'。
            assert api('/api/learning-agent')['status'] == 'idle', '删课后失效的最近任务应回到空闲状态'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/home/next-step')['status'] == 'idle', '首页不能因为已删任务返回服务器错误'。
            assert api('/api/home/next-step')['status'] == 'idle', '首页不能因为已删任务返回服务器错误'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/next-learning')['action'] == 'waiting'。
            assert api('/api/next-learning')['action'] == 'waiting'
            # 【逐行批注】 计算右侧结果并赋给 request('/api/learning-agent?taskId，更新脚本状态。
            request('/api/learning-agent?taskId='+deletion_task['id'], expected=409)
            # 【逐行批注】 计算右侧结果并赋给 current，更新脚本状态。
            current = api('/api/courses')['currentCourse']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert current and current['id'] == other_course。
            assert current and current['id'] == other_course
            # 【逐行批注】 计算右侧结果并赋给 request('/api/my-courses/'+other_course, met…，更新脚本状态。
            request('/api/my-courses/'+other_course, method='DELETE')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/courses')['currentCourse'] is None。
            assert api('/api/courses')['currentCourse'] is None
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '当前没有课程' in request('/current-course'), '空档案应显示独立的当前课程空状态'。
            assert '当前没有课程' in request('/current-course'), '空档案应显示独立的当前课程空状态'
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 执行当前 Python 语句：restarted.terminate()。
            restarted.terminate()
            # 【逐行批注】 计算右侧结果并赋给 restarted.wait(timeout，更新脚本状态。
            restarted.wait(timeout=20)
    # 【逐行批注】 输出当前运行结果：print('本轮 HTTP 回归通过：当前课程恢复、排课授权、候选确认/取消/幂等/过期、402立即停止与重试、新选择题衔接')。
    print('本轮 HTTP 回归通过：当前课程恢复、排课授权、候选确认/取消/幂等/过期、402立即停止与重试、新选择题衔接')


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'):，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'):
        # 【逐行批注】 计算右侧结果并赋给 sys.stdout.reconfigure(encoding，更新脚本状态。
        sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(sys.argv[1])。
    main(sys.argv[1])

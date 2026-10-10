"""本轮入口、课表权限、候选确认和出题衔接的隔离 HTTP 回归。"""
import json
import os
import subprocess
import sys
import time
from pathlib import Path
from urllib.error import HTTPError
from urllib.request import Request, urlopen
from classroom_v560_fixture import start_session


def main(executable):
    with start_session(executable) as (info, api, process):
        def request(path, value=None, method=None, expected=200, host=None):
            req = Request((host or info['base']) + path, data=None if value is None else json.dumps(value, ensure_ascii=False).encode(),
                          headers={'Content-Type': 'application/json'}, method=method)
            try:
                with urlopen(req, timeout=15) as response:
                    assert response.status == expected, (path, response.status)
                    raw = response.read()
            except HTTPError as error:
                assert error.code == expected, (path, error.code)
                raw = error.read()
            return json.loads(raw) if raw.startswith(b'{') else raw.decode('utf-8')

        def wait(task, expected='ready'):
            deadline = time.monotonic() + 90
            while time.monotonic() < deadline:
                state = api('/api/learning-agent?taskId=' + task)
                if state['status'] not in ('pending', 'running'):
                    assert state['status'] == expected, (state['status'], state.get('error'))
                    return state
                time.sleep(.05)
            raise AssertionError('隔离任务超时')

        course = info['courseId']
        assert api('/api/courses')['currentCourse']['id'] == course
        other = api('/api/generate-plan', dict(goal='第二门虚构课程', mode='lite', requestId='revision-other-course'))
        other_course = wait(other['id'])['course']['id']
        assert api('/api/courses')['currentCourse']['id'] == course, '建课不能冒充访问记录'
        request('/plan?courseId=' + other_course)
        assert api('/api/courses')['currentCourse']['id'] == other_course
        request('/plan?courseId=' + course)
        assert api('/api/courses')['currentCourse']['id'] == course, '同一秒访问不同课程也须按实际顺序恢复'

        availability = [dict(weekday=1, minutes=30)]
        before = api('/api/study-plan')
        assert before['status'] == 'ready', '尚未手动排课时不能永久显示正在生成'
        args = dict(availability=availability, entries=[], reason='试图从普通事件重排')
        for event_type in ('startup', 'chat', 'prepare_next', 'question_answer'):
            event = dict(type=event_type, courseId=course, attemptSchedule=args, text='普通教学事件的权限回归',
                requestId='denied-'+event_type, manualScheduleReplan=True, origin='manualScheduleReplan')
            if event_type in ('chat', 'question_answer'):
                event['lessonId'] = info['lessonId']
            if event_type == 'question_answer':
                section = next(s for s in api('/api/learning-agent/lesson?lessonId='+info['lessonId'])['sections'] if s.get('questionKind') == 'practice')
                event.update(sectionId=section['id'], sectionVersion=section['version'], view='practice')
            task = api('/api/learning-agent/events', event)
            wait(task['id'], 'failed')
            assert api('/api/study-plan')['entries'] == before['entries']
            assert api('/api/study-plan')['proposal'] is None, '客户端字段不能扩大排课权限'
        spoof = api('/api/learning-agent/events', dict(type='schedule_replan', requestId='spoofed-manual', manualScheduleReplan=True))
        wait(spoof['id'], 'failed')
        assert api('/api/study-plan')['proposal'] is None

        body = dict(version=before['version'], availability=availability, requestId='revision-replan-cancel')
        first = api('/api/study-plan/replan', body)
        duplicate = api('/api/study-plan/replan', body)
        assert duplicate['task']['id'] == first['task']['id']
        wait(first['task']['id'])
        candidate = api('/api/study-plan')
        assert candidate['entries'] == before['entries'] and candidate['proposal']['plan']['entries']
        candidate_dates = [entry['date'] for entry in candidate['proposal']['plan']['entries']]
        assert all(isinstance(entry.get('title'), str) and entry['title'] for entry in candidate['proposal']['plan']['entries'])
        assert candidate['proposal']['plan']['weekStart'] == min(candidate_dates)
        assert candidate['proposal']['plan']['weekEnd'] == max(candidate_dates)
        proposal = candidate['proposal']['id']
        calls_before = request('/', host=info['fixture'])['calls']
        cancelled = api('/api/study-plan/cancel', dict(version=candidate['version'], proposalId=proposal))['plan']
        assert cancelled['entries'] == before['entries'] and cancelled['proposal'] is None
        assert request('/', host=info['fixture'])['calls'] == calls_before, '取消不能再调用模型'

        second = api('/api/study-plan/replan', dict(version=cancelled['version'], availability=availability, requestId='revision-replan-confirm'))
        wait(second['task']['id'])
        candidate = api('/api/study-plan')
        planned_entries = candidate['proposal']['plan']['entries']
        confirm_body = dict(version=candidate['version'], proposalId=candidate['proposal']['id'])
        calls_before = request('/', host=info['fixture'])['calls']
        confirmed = api('/api/study-plan/confirm', confirm_body)['plan']
        assert confirmed['entries'] == planned_entries and confirmed['version'] == candidate['version'] + 1
        request('/api/study-plan/confirm', confirm_body, expected=409)
        assert api('/api/study-plan')['entries'] == planned_entries
        assert request('/', host=info['fixture'])['calls'] == calls_before, '确认必须应用原候选，不能再生成'
        stale = api('/api/study-plan/replan', dict(version=confirmed['version'], availability=availability, requestId='revision-replan-stale'))
        wait(stale['task']['id'])
        candidate = api('/api/study-plan')
        edited = request('/api/study-plan', dict(version=candidate['version'], availability=[dict(weekday=1, minutes=40)]), method='PUT')
        wait(edited['task']['id'])
        request('/api/study-plan/confirm', dict(version=candidate['version'], proposalId=candidate['proposal']['id']), expected=409)
        assert api('/api/study-plan')['availability'] == [dict(weekday=1, minutes=40)]

        before_fail = api('/api/study-plan')
        request('/fixture-control', dict(http_error=402), host=info['fixture'])
        failed_task = api('/api/study-plan/replan', dict(version=before_fail['version'], availability=availability, requestId='revision-replan-402'))
        failed = wait(failed_task['task']['id'], 'failed')
        assert failed['calls'] == 1 and failed['failure']['httpStatus'] == 402 and not failed['failure']['retryable']
        assert api('/api/study-plan')['status'] == 'failed'
        assert api('/api/study-plan')['entries'] == before_fail['entries']
        request('/fixture-control', {}, host=info['fixture'])
        api('/api/learning-agent/control', dict(command='retry', taskId=failed['id']))
        wait(failed['id'])
        recovered = api('/api/study-plan')
        assert recovered['proposal'] and recovered['entries'] == before_fail['entries']
        api('/api/study-plan/cancel', dict(version=recovered['version'], proposalId=recovered['proposal']['id']))

        lesson = api('/api/learning-agent/lesson?lessonId=' + info['lessonId'])
        original = next(s for s in lesson['sections'] if s.get('questionKind') == 'practice')
        event = dict(type='question_answer', lessonId=lesson['id'], courseId=course, sectionId=original['id'],
                     sectionVersion=original['version'], view='practice', text='请根据我的回答出一道选择题。', requestId='revision-choice')
        task = api('/api/learning-agent/events', event)
        wait(task['id'], 'waiting_student')
        for _ in range(2):
            view = api('/api/learning-agent/lesson?lessonId=' + lesson['id'])
            focus = view['teachingFocus']
            new_question = next(s for s in view['sections'] if s['id'] == focus['responseSectionId'])
            assert focus['responseView'] == 'practice' and new_question['questionKind'] == 'interaction'
            assert len(new_question['question']['options']) == 2 and 'expectedAnswer' not in json.dumps(view)
        assert api('/api/learning-agent/events', event)['id'] == task['id'], '重复请求不能追加重复题目'

        # 重启同一隔离档案，验证当前课程由服务端持久化恢复。
        process.terminate()
        process.wait(timeout=20)
        env = dict(os.environ, DATABASE_PATH=info['database'], PORT=info['base'].rsplit(':', 1)[1], HOST='127.0.0.1',
                   AI_BASE_URL=info['fixture']+'/v1', AI_API_KEY='fictional-fixture-key', AI_MODEL='fixture-v560', GANGYI_LAUNCH_SESSION_ID='revision-restart')
        restarted = subprocess.Popen([str(Path(executable).resolve())], cwd=Path(__file__).resolve().parents[1], env=env,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        try:
            for _ in range(200):
                try:
                    api('/health')
                    break
                except OSError:
                    time.sleep(.05)
            assert api('/api/courses')['currentCourse']['id'] == course
            startup = api('/api/learning-agent')
            if startup['status'] in ('pending', 'running'):
                wait(startup['id'])
            deletion_task = api('/api/learning-agent/events', dict(type='chat', courseId=course,
                lessonId=info['lessonId'], requestId='revision-delete-latest-task', text='删除前检查当前课时'))
            wait(deletion_task['id'], 'waiting_student')
            # 删除记录也会触发合法的后台学习更新；先暂停该作用域，单独验证失效最近标记。
            api('/api/learning-agent/control', dict(command='pause', taskId=deletion_task['id']))
            request('/api/my-courses/'+course, method='DELETE')
            assert api('/api/learning-agent')['status'] == 'idle', '删课后失效的最近任务应回到空闲状态'
            assert api('/api/home/next-step')['status'] == 'idle', '首页不能因为已删任务返回服务器错误'
            assert api('/api/next-learning')['action'] == 'waiting'
            request('/api/learning-agent?taskId='+deletion_task['id'], expected=409)
            current = api('/api/courses')['currentCourse']
            assert current and current['id'] == other_course
            request('/api/my-courses/'+other_course, method='DELETE')
            assert api('/api/courses')['currentCourse'] is None
        finally:
            restarted.terminate()
            restarted.wait(timeout=20)
    print('本轮 HTTP 回归通过：当前课程恢复、排课授权、候选确认/取消/幂等/过期、402立即停止与重试、新选择题衔接')


if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])

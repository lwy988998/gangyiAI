"""验证组件草稿在断网、回执丢失、刷新及多窗口冲突后的安全恢复。"""

import json
from contextlib import closing
from pathlib import Path
import sqlite3
import sys
import time
from urllib.parse import urlencode
from urllib.error import HTTPError
from urllib.request import Request, urlopen

from playwright.sync_api import sync_playwright

import classroom_v560_fixture as fixture
from deep_course_integration_smoke import DeepFixture


def wait_for(check, message, timeout=8, page=None):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        value = check()
        if value:
            return value
        if page:
            page.wait_for_timeout(80)
        else:
            time.sleep(.08)
    raise AssertionError(message)


def main():
    executable, output = sys.argv[1:3]
    output = Path(output).resolve()
    output.mkdir(parents=True, exist_ok=False)
    DeepFixture.include_sets = True
    fixture.FixtureAI = DeepFixture
    report = {'paidModelRequests': 0}
    with fixture.start_session(executable) as (info, api, _), sync_playwright() as runner:
        def wait_task(task_id):
            def completed():
                task = api('/api/learning-agent?taskId=' + task_id)
                if task['status'] in ('pending', 'running'):
                    return None
                assert task['status'] in ('ready', 'waiting_student'), task.get('error')
                return task
            return wait_for(completed, '模拟教学任务超时', 60)

        course = wait_task(api('/api/generate-plan', {'goal': '集合草稿恢复验证', 'mode': 'deep',
                                                     'requestId': 'draft-recovery-plan'})['id'])['course']['id']
        task = api('/api/deep-course/start', {'courseId': course, 'phaseIndex': 1, 'topicIndex': 1,
                                               'requestId': 'draft-recovery-lesson'})['task']['id']
        lesson_id = wait_task(task)['lesson']['id']
        lesson_path = '/api/deep-course?' + urlencode({'lessonId': lesson_id})
        before = api(lesson_path)
        try:
            urlopen(Request(info['base'] + '/api/deep-course/component-draft', data=b'{',
                            headers={'Content-Type': 'application/json'}), timeout=5)
            raise AssertionError('损坏的草稿请求不应被接受')
        except HTTPError as error:
            assert error.code == 400
        errors = []
        browser = fixture.launch_test_browser(runner)
        context = browser.new_context(viewport={'width': 1280, 'height': 720})
        page = context.new_page()
        page.on('pageerror', lambda error: errors.append(str(error)))
        page.goto(info['base'] + '/deep-course?' + urlencode({'lessonId': lesson_id}))
        page.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
        lab = page.locator('[data-media-id=set-lab]').first
        lab.wait_for()
        draft_url = '**/api/deep-course/component-draft'
        control = {'mode': 'offline', 'requests': [], 'modes': []}
        calls_before = len(DeepFixture.calls)

        def intercept(route):
            body = route.request.post_data_json
            control['requests'].append(body)
            control['modes'].append(control['mode'])
            if control['mode'] == 'offline':
                route.abort('failed')
            elif control['mode'] == 'unavailable':
                control['mode'] = 'normal'
                route.fulfill(status=503, content_type='text/html', body='暂时不可用')
            elif control['mode'] == 'lost-receipt':
                response = route.fetch()
                assert response.ok, response.text()
                control['mode'] = 'offline'
                route.abort('failed')
            else:
                route.continue_()

        page.route(draft_url, intercept)
        first = lab.get_by_role('checkbox', name='1 属于集合 B', exact=True)
        first.set_checked(True)
        wait_for(lambda: len(control['requests']) >= 1, '未发出首次草稿保存', page=page)
        assert not api(lesson_path).get('componentDrafts', {}).get('set-lab')
        page.reload()
        page.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
        lab = page.locator('[data-media-id=set-lab]').first
        first = lab.get_by_role('checkbox', name='1 属于集合 B', exact=True)
        assert first.is_checked()
        control['mode'] = 'normal'
        draft = wait_for(lambda: api(lesson_path).get('componentDrafts', {}).get('set-lab'),
                         '恢复连接后草稿未自动同步', page=page)
        assert '1' in draft['state']['setB']
        page.wait_for_timeout(750)
        assert control['modes'].count('normal') == 1, '相同快照无需重复保存'
        report['networkRecovery'] = True
        report['offlineReloadPreservedDraft'] = True

        control['mode'] = 'unavailable'
        second = lab.get_by_role('checkbox', name='4 属于集合 B', exact=True)
        second.set_checked(True)
        draft = wait_for(lambda: (item if '4' in (item := api(lesson_path)['componentDrafts']['set-lab'])['state']['setB']
                                 else None), '临时服务错误后草稿未自动同步', page=page)
        report['serverRecovery'] = True

        start_revision = draft['revision']
        control['requests'].clear()
        control['mode'] = 'lost-receipt'
        first.set_checked(False)
        wait_for(lambda: control['mode'] == 'offline', '未模拟保存成功但回执丢失', page=page)
        assert api(lesson_path)['componentDrafts']['set-lab']['revision'] == start_revision + 1
        second.set_checked(False)
        page.reload()
        page.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
        lab = page.locator('[data-media-id=set-lab]').first
        assert not lab.get_by_role('checkbox', name='1 属于集合 B', exact=True).is_checked()
        assert not lab.get_by_role('checkbox', name='4 属于集合 B', exact=True).is_checked()
        control['mode'] = 'normal'
        draft = wait_for(lambda: (item if (item := api(lesson_path)['componentDrafts']['set-lab'])['revision'] >= start_revision + 2
                                  else None), '刷新后未恢复未确认的保存与后续探索', page=page)
        assert '1' not in draft['state']['setB'] and '4' not in draft['state']['setB']
        assert draft['revision'] == start_revision + 2, '回执丢失造成了重复保存'
        assert any(item['expectedRevision'] == start_revision for item in control['requests'][1:]), '没有重放相同保存请求'
        report['lostReceiptAndReload'] = True

        # 在隔离数据库上制造真实写锁，而非只模拟 HTTP 状态码。
        with closing(sqlite3.connect(info['database'], timeout=1, isolation_level=None)) as lock:
            lock.execute('BEGIN IMMEDIATE')
            try:
                with page.expect_response(lambda response: response.url.endswith('/api/deep-course/component-draft'),
                                          timeout=12000) as busy:
                    lab.get_by_role('checkbox', name='1 属于集合 B', exact=True).set_checked(True)
                assert busy.value.status == 503, f'数据库暂时繁忙被错误分类为 {busy.value.status}'
            finally:
                lock.rollback()
        draft = wait_for(lambda: (item if '1' in (item := api(lesson_path)['componentDrafts']['set-lab'])['state']['setB']
                                 else None), '数据库解锁后草稿未恢复同步', page=page)
        report['realDatabaseBusyRecovery'] = True

        other = browser.new_context()
        stale = other.new_page()
        stale.on('pageerror', lambda error: errors.append(str(error)))
        stale.goto(info['base'] + '/deep-course?' + urlencode({'lessonId': lesson_id}))
        stale.wait_for_function("document.getElementById('deep-course-app').dataset.ready === 'true'")
        stale_lab = stale.locator('[data-media-id=set-lab]').first
        current = page.locator('[data-media-id=set-lab]').first
        current.get_by_role('checkbox', name='1 属于集合 B', exact=True).set_checked(False)
        winner = wait_for(lambda: (item if '1' not in (item := api(lesson_path)['componentDrafts']['set-lab'])['state']['setB']
                                  else None), '另一窗口没有保存', page=page)
        attempts = []
        def observe_conflict(route):
            attempts.append(route.request.post_data_json)
            route.continue_()
        stale.route(draft_url, observe_conflict)
        stale_lab.get_by_role('checkbox', name='4 属于集合 B', exact=True).set_checked(True)
        stale.get_by_text('组件探索已在其他窗口更新', exact=False).first.wait_for()
        stale.wait_for_timeout(1500)
        assert len(attempts) == 1, '版本冲突不应被当作断网反复重试'
        assert api(lesson_path)['componentDrafts']['set-lab'] == winner
        assert stale_lab.get_by_role('checkbox', name='4 属于集合 B', exact=True).is_checked()
        report['conflictPreservesBothWindows'] = True
        assert api(lesson_path)['lesson']['evaluations'] == before['lesson']['evaluations']
        assert len(DeepFixture.calls) == calls_before, '保存探索不应调用模型'
        assert not errors, errors
        report['formalEvaluationsUnchanged'] = True
        report['autosaveDidNotCallModel'] = True
        report['pageErrors'] = errors
        other.close()
        context.close()
        browser.close()
    (output / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(report, ensure_ascii=False))


if __name__ == '__main__':
    main()

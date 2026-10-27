"""以实际隔离服务验证先练后讲、运行中延后及首次讲解只接管一次。"""
import json
import os
import shutil
import sqlite3
import sys
import tempfile
import time
from contextlib import closing, contextmanager
from datetime import datetime
from pathlib import Path
from urllib.request import Request, urlopen

from playwright.sync_api import expect, sync_playwright
from classroom_v560_fixture import start_session


@contextmanager
def preserve_binding_failure(info, observations):
    try:
        yield
    except Exception as error:
        # 临时会话退出前保留一致数据库快照和最后观察值，不能只留下浏览器关闭后的连带异常。
        parent = Path(os.environ.get('GANGYI_BINDING_EVIDENCE_DIR', tempfile.gettempdir()))
        target = parent / ('binding-failure-' + datetime.now().strftime('%Y%m%d-%H%M%S-%f'))
        target.mkdir(parents=True)
        diagnostics = {'exception': type(error).__name__, 'message': str(error), 'observations': observations}
        try:
            with closing(sqlite3.connect(info['database'])) as source, closing(sqlite3.connect(target / 'fixture.db')) as saved:
                source.backup(saved)
            log = Path(info['database']).parent / 'service.log'
            if log.exists():
                shutil.copy2(log, target / 'service.log')
        except Exception as snapshot_error:
            diagnostics['snapshotError'] = str(snapshot_error)
        (target / 'last-responses.json').write_text(json.dumps(diagnostics, ensure_ascii=False, indent=2), encoding='utf-8')
        print('首次讲解失败证据已保存：' + str(target), file=sys.stderr)
        raise


def main(executable):
    with start_session(executable) as (info, api, _):
        observations = {'lessons': {}, 'tasks': {}}

        def wait_task(identity, browser_page):
            deadline = time.monotonic() + 40
            while time.monotonic() < deadline:
                task = api('/api/learning-agent?taskId=' + identity)
                observations['tasks'][identity] = {key: task.get(key) for key in ('id', 'status', 'calls', 'failure')}
                if task['status'] not in ('pending', 'running'):
                    assert task['status'] in ('ready', 'waiting_student'), task.get('failure')
                    return task
                # 同步 Playwright 必须持续派发路由与 WebSocket 事件，不能用 sleep 饿住浏览器消息泵。
                browser_page.wait_for_timeout(50)
            raise AssertionError('隔离教学任务未结束')

        def set_slow(value):
            with urlopen(Request(info['fixture'] + '/fixture-control',
                                 data=json.dumps({'slow': value}).encode(), headers={'Content-Type': 'application/json'})):
                pass

        def lesson_view(identity):
            lesson = api('/api/learning-agent/lesson?lessonId=' + identity)
            observations['lessons'][identity] = {key: lesson.get(key) for key in
                ('id', 'status', 'version', 'updatedAt', 'initialTeachingTaskId', 'teachingTaskId', 'teachingFocus')}
            return lesson

        with preserve_binding_failure(info, observations), sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True)
            page = browser.new_page(viewport={'width': 1440, 'height': 900})
            errors, external = [], []
            page.on('pageerror', lambda error: errors.append(str(error)))

            def local_only(route):
                if route.request.url.startswith(info['base'] + '/'):
                    route.continue_()
                else:
                    external.append(route.request.url)
                    route.abort()

            page.route('**/*', local_only)
            # 已完整结束的练习任务可以被首次讲解接管，新的焦点保存必须成功。
            first = info['lessonId']
            prior = api('/api/learning-agent/events', dict(type='chat', courseId=info['courseId'], lessonId=first,
                view='practice', requestId='binding-completed-first', text='先解释练习中的化合价。'))
            wait_task(prior['id'], page)
            assert not lesson_view(first).get('initialTeachingTaskId')
            page.goto(info['base'] + '/learn?lessonId=' + first)
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            current = lesson_view(first)
            initial = wait_task(current['initialTeachingTaskId'], page)
            assert initial['id'] != prior['id'] and current['teachingTaskId'] == initial['id']
            assert current['teachingFocus']['taskId'] == initial['id']
            calls_before = json.load(urlopen(info['fixture']))['calls']
            page.reload()
            expect(page.locator('#lesson-input')).to_be_enabled()
            assert lesson_view(first)['initialTeachingTaskId'] == initial['id']
            assert json.load(urlopen(info['fixture']))['calls'] == calls_before
            # 原任务运行中时不创建竞争的开课任务，随后由同一页面等待有效终态。
            prepared = api('/api/learning-agent/events', dict(type='prepare_next', courseId=info['courseId'],
                requestId='binding-running-preparation', navigate=False))
            second = wait_task(prepared['id'], page)['lesson']['id']
            api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=second))
            set_slow(True)
            active = api('/api/learning-agent/events', dict(type='chat', courseId=info['courseId'], lessonId=second,
                view='practice', requestId='binding-running-first', text='请继续解释练习中的电子变化。'))
            deadline = time.monotonic() + 15
            while time.monotonic() < deadline:
                if api('/api/learning-agent?taskId=' + active['id'])['status'] == 'running':
                    break
                page.wait_for_timeout(50)
            else:
                raise AssertionError('未实际覆盖正在执行的原任务')
            page.goto(info['base'] + '/learn?lessonId=' + second)
            expect(page.locator('#lesson-input')).to_be_enabled()
            while_active = lesson_view(second)
            assert while_active['teachingTaskId'] == active['id'] and not while_active.get('initialTeachingTaskId')
            reused = api('/api/learning-agent/control', dict(command='start_lesson', lessonId=second))
            assert reused['id'] == active['id'] and reused['initialTeachingDeferred'] is True
            set_slow(False)
            wait_task(active['id'], page)
            deadline = time.monotonic() + 20
            while time.monotonic() < deadline:
                after = lesson_view(second)
                if after.get('initialTeachingTaskId'):
                    break
                # 当前同步驱动不会等待异步谓词结果；显式读取并继续派发页面事件。
                page.wait_for_timeout(50)
            else:
                raise AssertionError('原任务结束后页面没有继续首次讲解；最后课时状态：' +
                                     json.dumps(observations['lessons'][second], ensure_ascii=False))
            started = wait_task(after['initialTeachingTaskId'], page)
            assert started['id'] != active['id']
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            final = lesson_view(second)
            assert final['teachingTaskId'] == final['initialTeachingTaskId'] == started['id']
            assert final['teachingFocus']['taskId'] == started['id']
            assert api('/api/learning-agent/control', dict(command='start_lesson', lessonId=second))['id'] == started['id']
            assert not errors, errors
            assert not external, external
            page.unroute_all(behavior='wait')
            browser.close()
    print('实际服务先练后讲、执行中延后、结束后自动接管、焦点保存及刷新幂等回归通过（仅本机协议夹具）。')


if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])

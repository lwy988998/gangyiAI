# 【逐行批注】 执行当前 Python 语句："""以实际隔离服务验证先练后讲、运行中延后及首次讲解只接管一次。"""。
"""以实际隔离服务验证先练后讲、运行中延后及首次讲解只接管一次。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：import shutil。
import shutil
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sqlite3。
import sqlite3
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextlib import closing, contextmanager。
from contextlib import closing, contextmanager
# 【逐行批注】 导入后续逻辑需要的模块或符号：from datetime import datetime。
from datetime import datetime
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen

# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import expect, sync_playwright。
from playwright.sync_api import expect, sync_playwright
# 【逐行批注】 导入后续逻辑需要的模块或符号：from classroom_v560_fixture import start_session。
from classroom_v560_fixture import start_session


# 【逐行批注】 应用装饰器 @contextmanager，扩展下一函数或测试的行为。
@contextmanager
# 【逐行批注】 定义函数 preserve_binding_failure，封装可复用的处理步骤。
def preserve_binding_failure(info, observations):
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 执行当前 Python 语句：yield。
        yield
    # 【逐行批注】 捕获并处理异常：except Exception as error:。
    except Exception as error:
        # 临时会话退出前保留一致数据库快照和最后观察值，不能只留下浏览器关闭后的连带异常。
        # 【逐行批注】 计算右侧结果并赋给 parent，更新脚本状态。
        parent = Path(os.environ.get('GANGYI_BINDING_EVIDENCE_DIR', tempfile.gettempdir()))
        # 【逐行批注】 计算右侧结果并赋给 target，更新脚本状态。
        target = parent / ('binding-failure-' + datetime.now().strftime('%Y%m%d-%H%M%S-%f'))
        # 【逐行批注】 计算右侧结果并赋给 target.mkdir(parents，更新脚本状态。
        target.mkdir(parents=True)
        # 【逐行批注】 计算右侧结果并赋给 diagnostics，更新脚本状态。
        diagnostics = {'exception': type(error).__name__, 'message': str(error), 'observations': observations}
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as source, closing(sqlite3.conn…。
            with closing(sqlite3.connect(info['database'])) as source, closing(sqlite3.connect(target / 'fixture.db')) as saved:
                # 【逐行批注】 执行当前 Python 语句：source.backup(saved)。
                source.backup(saved)
            # 【逐行批注】 计算右侧结果并赋给 log，更新脚本状态。
            log = Path(info['database']).parent / 'service.log'
            # 【逐行批注】 判断条件 log.exists():，成立时执行对应分支。
            if log.exists():
                # 【逐行批注】 执行当前 Python 语句：shutil.copy2(log, target / 'service.log')。
                shutil.copy2(log, target / 'service.log')
        # 【逐行批注】 捕获并处理异常：except Exception as snapshot_error:。
        except Exception as snapshot_error:
            # 【逐行批注】 计算右侧结果并赋给 diagnostics['snapshotError']，更新脚本状态。
            diagnostics['snapshotError'] = str(snapshot_error)
        # 【逐行批注】 计算右侧结果并赋给 (target / 'last-responses.json').write_text(…，更新脚本状态。
        (target / 'last-responses.json').write_text(json.dumps(diagnostics, ensure_ascii=False, indent=2), encoding='utf-8')
        # 【逐行批注】 输出当前运行结果：print('首次讲解失败证据已保存：' + str(target), file=sys.stderr)。
        print('首次讲解失败证据已保存：' + str(target), file=sys.stderr)
        # 【逐行批注】 在错误情形抛出异常：raise。
        raise


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable):
    # 【逐行批注】 在受控上下文中执行资源操作：with start_session(executable) as (info, api, _):。
    with start_session(executable) as (info, api, _):
        # 【逐行批注】 计算右侧结果并赋给 observations，更新脚本状态。
        observations = {'lessons': {}, 'tasks': {}}

        # 【逐行批注】 定义函数 wait_task，封装可复用的处理步骤。
        def wait_task(identity, browser_page):
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = time.monotonic() + 40
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
            while time.monotonic() < deadline:
                # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
                task = api('/api/learning-agent?taskId=' + identity)
                # 【逐行批注】 计算右侧结果并赋给 observations['tasks'][identity]，更新脚本状态。
                observations['tasks'][identity] = {key: task.get(key) for key in ('id', 'status', 'calls', 'failure')}
                # 【逐行批注】 判断条件 task['status'] not in ('pending', 'running'):，成立时执行对应分支。
                if task['status'] not in ('pending', 'running'):
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert task['status'] in ('ready', 'waiting_student'), task.get('failure')。
                    assert task['status'] in ('ready', 'waiting_student'), task.get('failure')
                    # 【逐行批注】 结束当前函数并返回结果：return task。
                    return task
                # 同步 Playwright 必须持续派发路由与 WebSocket 事件，不能用 sleep 饿住浏览器消息泵。
                # 【逐行批注】 执行当前 Python 语句：browser_page.wait_for_timeout(50)。
                browser_page.wait_for_timeout(50)
            # 【逐行批注】 在错误情形抛出异常：raise AssertionError('隔离教学任务未结束')。
            raise AssertionError('隔离教学任务未结束')

        # 【逐行批注】 定义函数 set_slow，封装可复用的处理步骤。
        def set_slow(value):
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture'] + '/fixture-control',。
            with urlopen(Request(info['fixture'] + '/fixture-control',
                                 # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
                                 data=json.dumps({'slow': value}).encode(), headers={'Content-Type': 'application/json'})):
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass

        # 【逐行批注】 定义函数 lesson_view，封装可复用的处理步骤。
        def lesson_view(identity):
            # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
            lesson = api('/api/learning-agent/lesson?lessonId=' + identity)
            # 【逐行批注】 计算右侧结果并赋给 observations['lessons'][identity]，更新脚本状态。
            observations['lessons'][identity] = {key: lesson.get(key) for key in
                # 【逐行批注】 执行当前 Python 语句：('id', 'status', 'version', 'updatedAt', 'initialTeachingTaskId', 'teachingTaskId', 'teac…。
                ('id', 'status', 'version', 'updatedAt', 'initialTeachingTaskId', 'teachingTaskId', 'teachingFocus')}
            # 【逐行批注】 结束当前函数并返回结果：return lesson。
            return lesson

        # 【逐行批注】 在受控上下文中执行资源操作：with preserve_binding_failure(info, observations), sync_playwright() as playwri…。
        with preserve_binding_failure(info, observations), sync_playwright() as playwright:
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser = playwright.chromium.launch(headless=True)
            # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
            page = browser.new_page(viewport={'width': 1440, 'height': 900})
            # 【逐行批注】 计算右侧结果并赋给 errors, external，更新脚本状态。
            errors, external = [], []
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：page.on('pageerror', lambda error: errors.append(str(error)))。
            page.on('pageerror', lambda error: errors.append(str(error)))

            # 【逐行批注】 定义函数 local_only，封装可复用的处理步骤。
            def local_only(route):
                # 【逐行批注】 判断条件 route.request.url.startswith(info['base'] + '/'):，成立时执行对应分支。
                if route.request.url.startswith(info['base'] + '/'):
                    # 【逐行批注】 执行当前 Python 语句：route.continue_()。
                    route.continue_()
                # 【逐行批注】 执行前一条件不成立时的分支。
                else:
                    # 【逐行批注】 执行当前 Python 语句：external.append(route.request.url)。
                    external.append(route.request.url)
                    # 【逐行批注】 执行当前 Python 语句：route.abort()。
                    route.abort()

            # 【逐行批注】 执行当前 Python 语句：page.route('** /*', local_only)。
            page.route('**/*', local_only)
            # 已完整结束的练习任务可以被首次讲解接管，新的焦点保存必须成功。
            # 【逐行批注】 计算右侧结果并赋给 first，更新脚本状态。
            first = info['lessonId']
            # 【逐行批注】 计算右侧结果并赋给 prior，更新脚本状态。
            prior = api('/api/learning-agent/events', dict(type='chat', courseId=info['courseId'], lessonId=first,
                # 【逐行批注】 计算右侧结果并赋给 view，更新脚本状态。
                view='practice', requestId='binding-completed-first', text='先解释练习中的化合价。'))
            # 【逐行批注】 调用 wait_task 完成当前脚本或测试步骤：wait_task(prior['id'], page)。
            wait_task(prior['id'], page)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not lesson_view(first).get('initialTeachingTaskId')。
            assert not lesson_view(first).get('initialTeachingTaskId')
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base'] + '/learn?lessonId，更新脚本状态。
            page.goto(info['base'] + '/learn?lessonId=' + first)
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            # 【逐行批注】 计算右侧结果并赋给 current，更新脚本状态。
            current = lesson_view(first)
            # 【逐行批注】 计算右侧结果并赋给 initial，更新脚本状态。
            initial = wait_task(current['initialTeachingTaskId'], page)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert initial['id'] != prior['id'] and current['teachingTaskId'] == initial['i…。
            assert initial['id'] != prior['id'] and current['teachingTaskId'] == initial['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert current['teachingFocus']['taskId'] == initial['id']。
            assert current['teachingFocus']['taskId'] == initial['id']
            # 【逐行批注】 计算右侧结果并赋给 calls_before，更新脚本状态。
            calls_before = json.load(urlopen(info['fixture']))['calls']
            # 【逐行批注】 执行当前 Python 语句：page.reload()。
            page.reload()
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_be_enabled()。
            expect(page.locator('#lesson-input')).to_be_enabled()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert lesson_view(first)['initialTeachingTaskId'] == initial['id']。
            assert lesson_view(first)['initialTeachingTaskId'] == initial['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert json.load(urlopen(info['fixture']))['calls'] == calls_before。
            assert json.load(urlopen(info['fixture']))['calls'] == calls_before
            # 原任务运行中时不创建竞争的开课任务，随后由同一页面等待有效终态。
            # 【逐行批注】 计算右侧结果并赋给 prepared，更新脚本状态。
            prepared = api('/api/learning-agent/events', dict(type='prepare_next', courseId=info['courseId'],
                # 【逐行批注】 计算右侧结果并赋给 requestId，更新脚本状态。
                requestId='binding-running-preparation', navigate=False))
            # 【逐行批注】 计算右侧结果并赋给 second，更新脚本状态。
            second = wait_task(prepared['id'], page)['lesson']['id']
            # 【逐行批注】 计算右侧结果并赋给 api('/api/learning-agent/control', dict(comm…，更新脚本状态。
            api('/api/learning-agent/control', dict(command='enter_lesson', lessonId=second))
            # 【逐行批注】 调用 set_slow 完成当前脚本或测试步骤：set_slow(True)。
            set_slow(True)
            # 【逐行批注】 计算右侧结果并赋给 active，更新脚本状态。
            active = api('/api/learning-agent/events', dict(type='chat', courseId=info['courseId'], lessonId=second,
                # 【逐行批注】 计算右侧结果并赋给 view，更新脚本状态。
                view='practice', requestId='binding-running-first', text='请继续解释练习中的电子变化。'))
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = time.monotonic() + 15
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
            while time.monotonic() < deadline:
                # 【逐行批注】 判断条件 api('/api/learning-agent?taskId=' + active['id'])['status'] == '…，成立时执行对应分支。
                if api('/api/learning-agent?taskId=' + active['id'])['status'] == 'running':
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
                # 【逐行批注】 执行当前 Python 语句：page.wait_for_timeout(50)。
                page.wait_for_timeout(50)
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 在错误情形抛出异常：raise AssertionError('未实际覆盖正在执行的原任务')。
                raise AssertionError('未实际覆盖正在执行的原任务')
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base'] + '/learn?lessonId，更新脚本状态。
            page.goto(info['base'] + '/learn?lessonId=' + second)
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_be_enabled()。
            expect(page.locator('#lesson-input')).to_be_enabled()
            # 【逐行批注】 计算右侧结果并赋给 while_active，更新脚本状态。
            while_active = lesson_view(second)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert while_active['teachingTaskId'] == active['id'] and not while_active.get(…。
            assert while_active['teachingTaskId'] == active['id'] and not while_active.get('initialTeachingTaskId')
            # 【逐行批注】 计算右侧结果并赋给 reused，更新脚本状态。
            reused = api('/api/learning-agent/control', dict(command='start_lesson', lessonId=second))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert reused['id'] == active['id'] and reused['initialTeachingDeferred'] is Tr…。
            assert reused['id'] == active['id'] and reused['initialTeachingDeferred'] is True
            # 【逐行批注】 调用 set_slow 完成当前脚本或测试步骤：set_slow(False)。
            set_slow(False)
            # 【逐行批注】 调用 wait_task 完成当前脚本或测试步骤：wait_task(active['id'], page)。
            wait_task(active['id'], page)
            # 【逐行批注】 计算右侧结果并赋给 deadline，更新脚本状态。
            deadline = time.monotonic() + 20
            # 【逐行批注】 在条件成立期间重复执行：while time.monotonic() < deadline:。
            while time.monotonic() < deadline:
                # 【逐行批注】 计算右侧结果并赋给 after，更新脚本状态。
                after = lesson_view(second)
                # 【逐行批注】 判断条件 after.get('initialTeachingTaskId'):，成立时执行对应分支。
                if after.get('initialTeachingTaskId'):
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
                # 当前同步驱动不会等待异步谓词结果；显式读取并继续派发页面事件。
                # 【逐行批注】 执行当前 Python 语句：page.wait_for_timeout(50)。
                page.wait_for_timeout(50)
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 在错误情形抛出异常：raise AssertionError('原任务结束后页面没有继续首次讲解；最后课时状态：' +。
                raise AssertionError('原任务结束后页面没有继续首次讲解；最后课时状态：' +
                                     # 【逐行批注】 计算右侧结果并赋给 json.dumps(observations['lessons'][second], …，更新脚本状态。
                                     json.dumps(observations['lessons'][second], ensure_ascii=False))
            # 【逐行批注】 计算右侧结果并赋给 started，更新脚本状态。
            started = wait_task(after['initialTeachingTaskId'], page)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert started['id'] != active['id']。
            assert started['id'] != active['id']
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')。
            expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
            # 【逐行批注】 计算右侧结果并赋给 final，更新脚本状态。
            final = lesson_view(second)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert final['teachingTaskId'] == final['initialTeachingTaskId'] == started['id…。
            assert final['teachingTaskId'] == final['initialTeachingTaskId'] == started['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert final['teachingFocus']['taskId'] == started['id']。
            assert final['teachingFocus']['taskId'] == started['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/api/learning-agent/control', dict(command='start_lesson', lessonId…。
            assert api('/api/learning-agent/control', dict(command='start_lesson', lessonId=second))['id'] == started['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, errors。
            assert not errors, errors
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not external, external。
            assert not external, external
            # 【逐行批注】 计算右侧结果并赋给 page.unroute_all(behavior，更新脚本状态。
            page.unroute_all(behavior='wait')
            # 【逐行批注】 执行当前 Python 语句：browser.close()。
            browser.close()
    # 【逐行批注】 输出当前运行结果：print('实际服务先练后讲、执行中延后、结束后自动接管、焦点保存及刷新幂等回归通过（仅本机协议夹具）。')。
    print('实际服务先练后讲、执行中延后、结束后自动接管、焦点保存及刷新幂等回归通过（仅本机协议夹具）。')


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'):，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'):
        # 【逐行批注】 计算右侧结果并赋给 sys.stdout.reconfigure(encoding，更新脚本状态。
        sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(sys.argv[1])。
    main(sys.argv[1])

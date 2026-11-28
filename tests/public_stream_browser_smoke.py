"""用可暂停的本机模型验证首片显示、修正、恢复及课堂导航，不调用付费接口。"""

import io
import json
from pathlib import Path
import sys
import time
from threading import Event
from urllib.parse import urlencode

from playwright.sync_api import sync_playwright

import classroom_v560_fixture as fixture
from deep_course_integration_smoke import DeepFixture


class PublicStreamFixture(DeepFixture):
    gate = Event()
    sent = Event()
    finished = Event()
    correction_calls = 0
    first_body = '流式正文首段，模型仍在继续生成。'
    first_plan = '流式大纲首段'

    def emit(self, text, prefix=None):
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream')
        self.end_headers()

        def frame(value):
            data = dict(model='public-stream-fixture', choices=[dict(delta=dict(content=value), finish_reason=None)])
            self.wfile.write(('data: ' + json.dumps(data, ensure_ascii=False) + '\n\n').encode())
            self.wfile.flush()

        try:
            split = text.index(prefix) + len(prefix) if prefix else 0
            if split:
                frame(text[:split])
                type(self).sent.set()
                if not type(self).gate.wait(25):
                    raise TimeoutError('首片没有在模型结束前显示')
            for index in range(split, len(text), 37):
                frame(text[index:index + 37])
                time.sleep(.01)
            self.wfile.write(b'data: [DONE]\n\n')
            self.wfile.flush()
            type(self).finished.set()
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            pass

    def do_POST(self):
        raw = self.rfile.read(int(self.headers['Content-Length']))
        request = json.loads(raw)
        event = {}
        for message in request.get('messages', []):
            try:
                context = json.loads(message.get('content', ''))
            except (ValueError, TypeError):
                continue
            if isinstance(context, dict) and isinstance(context.get('event'), dict):
                event = context['event']
                break

        if event.get('goal') == '流式大纲验收' and event.get('type') == 'plan_course':
            value = dict(title=self.first_plan + '课程', summary='核对生成中即可阅读的课程路线',
                         courseStructure=[dict(stage='第一阶段', topics=['第一个知识点', '第二个知识点'])])
            return self.emit(json.dumps(value, ensure_ascii=False), self.first_plan)

        text = event.get('text')
        if text in ('流式导师验收', '流式管家验收') and event.get('type') in ('chat', 'assistant_chat'):
            prefix = text + '的首段正文'
            value = dict(teaching=dict(helpType='explanation'), message=prefix + '，后续正文已完成。', actions=[], state='waiting_student')
            return self.emit(json.dumps(value, ensure_ascii=False), prefix)
        if text in ('流式正文验收', '自动修正验收') and event.get('type') == 'chat':
            if text == '自动修正验收':
                type(self).correction_calls += 1
                if type(self).correction_calls == 1:
                    return self.emit('{"message":"这是等待修正的首版草稿","actions":[}', '首版草稿')
                value = dict(teaching=dict(helpType='explanation'), message='这是自动核对后的正式回复。',
                             actions=[], state='waiting_student')
                return self.emit(json.dumps(value, ensure_ascii=False))

            section = dict(kind='explanation', helpType='explanation', title='实时补充讲义',
                           materialOrigin='ai_supplement',
                           body=self.first_body + '\n\n' + '\n\n'.join(f'第 {index} 段后续讲义，保留前文并继续补充。' for index in range(90)))
            value = dict(teaching=dict(helpType='explanation'), message='',
                         actions=[dict(id='live-explanation', tool='append_section',
                                       args=dict(lessonId=event['lessonId'], section=section))], state='waiting_student')
            return self.emit(json.dumps(value, ensure_ascii=False), self.first_body)

        self.rfile = io.BytesIO(raw)
        return super().do_POST()


def main():
    executable, output = sys.argv[1:3]
    output = Path(output).resolve()
    output.mkdir(parents=True, exist_ok=False)
    fixture.FixtureAI = PublicStreamFixture
    report = {'paidModelRequests': 0}
    with fixture.start_session(executable) as (info, api, _), sync_playwright() as runner:
        def wait_task(task_id):
            deadline = time.monotonic() + 60
            while time.monotonic() < deadline:
                task = api('/api/learning-agent?taskId=' + task_id)
                if task['status'] not in ('pending', 'running'):
                    assert task['status'] in ('ready', 'waiting_student'), task.get('error')
                    return task
                time.sleep(.08)
            raise TimeoutError(task_id)

        browser = runner.chromium.launch(headless=True, channel='msedge')
        page = browser.new_page(viewport=dict(width=1280, height=720), reduced_motion='reduce')
        errors = []
        page.on('pageerror', lambda error: errors.append(str(error)))
        page.add_init_script("window.streamTimings=[];addEventListener('gangyi:stream-delivery',event=>streamTimings.push(event.detail));")

        PublicStreamFixture.gate.clear()
        page.goto(info['base'] + '/plan?' + urlencode(dict(goal='流式大纲验收', mode='deep')))
        page.get_by_text(PublicStreamFixture.first_plan, exact=True).wait_for()
        assert PublicStreamFixture.sent.is_set() and not PublicStreamFixture.finished.is_set()
        report['outlineVisibleBeforeModelFinished'] = True
        page.screenshot(path=str(output / 'outline-streaming.png'))
        PublicStreamFixture.gate.set()
        page.locator('#result').wait_for(state='visible')

        course = wait_task(api('/api/generate-plan', dict(goal='独立的深度课堂验收', mode='deep', requestId='public-stream-deep'))['id'])['course']['id']
        lesson = wait_task(api('/api/deep-course/start', dict(courseId=course, phaseIndex=1, topicIndex=1, requestId='public-stream-lesson'))['task']['id'])['lesson']['id']
        page.goto(info['base'] + '/deep-course?' + urlencode(dict(lessonId=lesson)))
        page.wait_for_function("document.getElementById('deep-course-app').dataset.ready==='true'")
        page.locator('#dc-directory-collapse').click()
        assert page.locator('#dc-directory-reopen').is_visible()
        page.locator('#dc-directory-reopen').click()
        assert page.locator('#dc-directory').is_visible()
        assert page.locator('#dc-home').get_attribute('href') == '/'
        report['directoryAndHome'] = True

        PublicStreamFixture.gate.clear()
        PublicStreamFixture.sent.clear()
        PublicStreamFixture.finished.clear()
        page.locator('#dc-course-reply-input').fill('流式正文验收')
        page.locator('#dc-course-reply-send').click()
        page.locator('#dc-stream-content').get_by_text(PublicStreamFixture.first_body, exact=True).wait_for()
        assert PublicStreamFixture.sent.is_set() and not PublicStreamFixture.finished.is_set()
        report['lectureVisibleBeforeModelFinished'] = True
        page.screenshot(path=str(output / 'lecture-streaming.png'))
        page.reload()
        page.locator('#dc-stream-content').get_by_text(PublicStreamFixture.first_body, exact=True).wait_for()
        report['unfinishedStreamRestored'] = True

        field = page.locator('#dc-course-reply-input')
        field.fill('尚未发送的下一条草稿')
        field.focus()
        page.evaluate("const box=document.getElementById('dc-reading');box.scrollTop=0;box.dispatchEvent(new Event('scroll'));")
        before = page.locator('#dc-reading').evaluate('node=>({top:node.scrollTop,overflow:node.scrollHeight>node.clientHeight+56})')
        PublicStreamFixture.gate.set()
        page.wait_for_function("document.getElementById('dc-task-status')?.textContent.includes('准备好了') || document.getElementById('dc-reading').textContent.includes('第 89 段')")
        page.locator('#dc-stream-content .gy-stream-notice').filter(has_text='生成中').wait_for(state='hidden')
        assert field.input_value() == '尚未发送的下一条草稿'
        assert page.evaluate("document.activeElement.id==='dc-course-reply-input'")
        if before['overflow']:
            assert page.locator('#dc-reading').evaluate('node=>node.scrollTop') < before['top'] + 30
            page.locator('#dc-reading').locator('..').locator(':scope > .gy-return-latest').click()
            assert page.locator('#dc-reading').evaluate('node=>node.scrollHeight-node.clientHeight-node.scrollTop') < 60
        report['draftAndReadingPosition'] = True

        PublicStreamFixture.gate.clear()
        PublicStreamFixture.sent.clear()
        PublicStreamFixture.finished.clear()
        field.fill('自动修正验收')
        page.locator('#dc-course-reply-send').click()
        page.locator('#dc-stream-content').get_by_text('这是等待修正的首版草稿', exact=True).wait_for()
        PublicStreamFixture.gate.set()
        page.get_by_text('这是自动核对后的正式回复。', exact=True).first.wait_for()
        page.get_by_text('已修正 · 查看改动', exact=True).first.wait_for()
        assert PublicStreamFixture.correction_calls == 2
        report['automaticCorrection'] = True
        assert page.get_by_role('heading', name='实时补充讲义', exact=True).count() == 1
        page.reload()
        page.get_by_role('heading', name='实时补充讲义', exact=True).wait_for(state='attached')
        page.get_by_text('这是自动核对后的正式回复。', exact=True).first.wait_for(state='attached')
        report['publishedLectureRetainedAfterNextReplyAndReload'] = True


        for width, height in [(1280, 720), (390, 844)]:
            page.set_viewport_size(dict(width=width, height=height))
            assert page.evaluate('document.documentElement.scrollWidth<=innerWidth+1')
            assert page.locator('#dc-home').is_visible()
            page.screenshot(path=str(output / f'classroom-{width}.png'))
        for target, marker in [('tutor', '流式导师验收'), ('manager', '流式管家验收')]:
            PublicStreamFixture.gate.clear()
            PublicStreamFixture.sent.clear()
            PublicStreamFixture.finished.clear()
            prefix = marker + '的首段正文'
            if target == 'tutor':
                page.goto(info['base'] + '/ask')
                page.locator('#ask-question').fill(marker)
                page.locator('#ask-submit').click()
            else:
                chat = api('/api/my-ai/chats', dict(title='流式管家检查'))
                page.goto(info['base'] + '/my-ai/chat?' + urlencode(dict(id=chat['id'])))
                page.locator('#chat-input').fill(marker)
                page.locator('#send-chat').click()
            page.get_by_text(prefix, exact=True).first.wait_for()
            assert PublicStreamFixture.sent.is_set() and not PublicStreamFixture.finished.is_set()
            PublicStreamFixture.gate.set()
            complete = prefix + '，后续正文已完成。'
            page.get_by_text(complete, exact=True).first.wait_for()
            if target == 'tutor':
                page.wait_for_function("!document.getElementById('ask-submit').disabled")
            else:
                page.locator('.ma-message.assistant .ma-message-content').get_by_text(complete, exact=True).first.wait_for()
            report[target + 'VisibleBeforeModelFinished'] = True

        report['pageErrors'] = errors
        report['deliveryMeasurements'] = page.evaluate('streamTimings')[-15:]
        assert not errors, errors
        browser.close()
    (output / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(report, ensure_ascii=False))


if __name__ == '__main__':
    main()

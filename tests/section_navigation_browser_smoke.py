"""使用生产课堂 DOM、样式和脚本，离线验证原卡片链接的实际滚动与草稿恢复。"""
import json
import re
from contextlib import contextmanager
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Thread
from urllib.parse import urlsplit

from playwright.sync_api import expect, sync_playwright


REPO = Path(__file__).resolve().parents[1]


def saved_lesson():
    sections = [dict(id='learn-first', kind='explanation', title='讲解原卡片', body='把同一整体平均分成若干份。',
                     version=1, view='learn', href='/learn?lessonId=navigation-fixture&sectionId=learn-first')]
    for index in range(14):
        sections.append(dict(id=f'question-{index}', kind='question', title=f'原练习 {index + 1}', questionKind='practice',
                             questionId=f'qid-{index}', version=1, view='practice',
                             href=f'/practice?lessonId=navigation-fixture&sectionId=question-{index}',
                             question=dict(question='请说明分数单位与平均分的关系。' * 5, type='multi_choice' if index == 1 else 'choice', options=['先统一单位', '直接相加'])))
    sections.append(dict(id='repair', kind='explanation', title='练习中的原补讲', version=1, view='practice',
                         href='/practice?lessonId=navigation-fixture&sectionId=repair',
                         body='分母表示把同一整体平均分成的份数，每份相当于整体的1/分母。'))
    for index in range(8):
        sections.append(dict(id=f'summary-{index}', kind='summary', title=f'原小结 {index + 1}', version=1,
                             view='summary', href=f'/summary?lessonId=navigation-fixture&sectionId=summary-{index}',
                             body='回顾同一整体和相同分数单位，完成情况仍依据原来的真实作答。' * 4))
    return dict(id='navigation-fixture', courseId='navigation-course', status='ready', entered=True,
                title='原卡片导航隔离夹具', purpose='只回看已保存的内容', sections=sections,
                teachingTaskId='saved-task', teachingTaskView='practice', evaluations=[],
                timeline=[dict(type='section', sectionId='repair', taskId='saved-task', view='practice')])


def classroom_html(view):
    # 直接使用生产页面的课堂主体，避免测试自己编造的布局。
    source = (REPO / 'src/page_renderer.cpp').read_text(encoding='utf-8')
    function = source.split('std::string renderClassroomPage(', 1)[1]
    body = re.search(r'headerShell\("learn"\) \+ R"HTML\((.*?)\)HTML";', function, re.S).group(1)
    body = body.replace(')HTML" + htmlEscape(view) + R"HTML(', view)
    body = body.replace(')HTML" + label + R"HTML(', dict(learn='讲解', practice='练习', summary='小结')[view])
    assert 'R"HTML' not in body
    styles = ''.join(f'<link rel="stylesheet" href="/{name}">' for name in
                     ['styles.css', 'plan.css', 'dark.css', 'app-layout.css', 'classroom.css', 'agent-classroom.css', 'library.css'])
    fixture = json.dumps(saved_lesson(), ensure_ascii=False)
    script = """
    const fixture = FIXTURE, query = new URLSearchParams(location.search);
    if (query.has('legacy')) fixture.sections.forEach(section => delete section.href);
    if (query.has('archived')) fixture.timeline = [{type:'section',sectionId:'question-0',taskId:'saved-task',view:'practice'}];
    let ownerStatus = 'running';
    const fixtureWatches = new Map();
    window.fixtureRequests = {read:0,control:[],submit:[],visible:[]};
    window.fixtureOwnerState = status => { ownerStatus=status; fixtureWatches.get('saved-task')?.onState({id:'saved-task',status}); };
    window.fixtureFinishOwner = () => window.fixtureOwnerState('waiting_student');
    const storageKey = key => 'navigation-fixture:' + key;
    window.GangyiAgent = {
      request: async path => { if (!path.startsWith('/api/learning-agent/lesson?')) throw Error('非预期读取');
        window.fixtureRequests.read++; return structuredClone(fixture); },
      control: async event => { window.fixtureRequests.control.push(event);
        if(query.has('deferred') && event.command==='start_lesson') {
          if(!['ready','waiting_student','cancelled','superseded'].includes(ownerStatus))return {id:'saved-task',status:ownerStatus,initialTeachingDeferred:true};
          fixture.initialTeachingTaskId=fixture.teachingTaskId='initial-task'; fixture.teachingTaskView='learn';
          fixture.teachingFocus={sectionId:'learn-first'};
          return {id:'initial-task',status:'waiting_student'};
        }
        throw Error('回看不能启动教学'); },
      submit: async event => { window.fixtureRequests.submit.push(event);
        if(query.has('answer-test'))return {id:'submitted-task-'+window.fixtureRequests.submit.length,status:'waiting_student',requestId:event.requestId};
        throw Error('回看不能提交教学请求'); },
      storage: {get:key=>localStorage.getItem(storageKey(key))||'',set:(key,value)=>localStorage.setItem(storageKey(key),value||''),remove:key=>localStorage.removeItem(storageKey(key))},
      richText:(node,value)=>node.innerHTML=window.GangyiChat.renderMarkdown(String(value),{math:false}),
      watch:(id,handlers)=>{ if(query.has('deferred')) { fixtureWatches.set(id,handlers);
          queueMicrotask(()=>handlers.onState({id,status:id==='initial-task'?'waiting_student':ownerStatus})); }
        return ()=>fixtureWatches.delete(id); },
      flushExposure:async()=>{},id:()=>crypto.randomUUID(),
      failureMessage:()=>'',recordVisibleMessage:()=>{},recordEvaluation:()=>{},
      recordVisibleSection:(lesson,section,node,scroll)=>{ if(!node)return; const a=node.getBoundingClientRect(),b=scroll.getBoundingClientRect();
        if(a.width&&a.height&&a.bottom>b.top&&a.top<b.bottom)window.fixtureRequests.visible.push(section.id); }
    };
    """.replace('FIXTURE', fixture)
    return ('<!doctype html><html lang="zh-CN"><meta charset="utf-8">' + styles +
            '<body class="dark-app"><header style="height:76px">离线导航回归</header>' + body +
            '<script src="/chat-render.js"></script><script>' + script + '</script><script defer src="/agent-classroom.js"></script></body></html>').encode()


@contextmanager
def local_pages():
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def do_GET(self):
            path = urlsplit(self.path).path
            if path in ('/learn', '/practice', '/summary'):
                content, kind = classroom_html(path[1:]), 'text/html; charset=utf-8'
            elif path in ('/agent-classroom.js', '/chat-render.js') or path.endswith('.css') and '/' not in path[1:]:
                content = (REPO / 'public' / path[1:]).read_bytes()
                kind = 'text/javascript; charset=utf-8' if path.endswith('.js') else 'text/css; charset=utf-8'
            else:
                self.send_error(404)
                return
            self.send_response(200)
            self.send_header('Content-Type', kind)
            self.send_header('Cache-Control', 'no-store')
            self.end_headers()
            self.wfile.write(content)

    server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    Thread(target=server.serve_forever, daemon=True).start()
    try:
        yield f'http://127.0.0.1:{server.server_port}'
    finally:
        server.shutdown()
        server.server_close()


def assert_located(page, identity):
    target = page.locator(f'[data-section-id="{identity}"]')
    expect(target).to_be_visible()
    page.wait_for_function("""id => {const node=document.querySelector('[data-section-id="'+id+'"]'),scroll=document.getElementById('lesson-scroll');
      if(!node)return false;const a=node.getBoundingClientRect(),b=scroll.getBoundingClientRect();return a.top>=b.top-2&&a.top<b.bottom-20;}""", arg=identity)
    assert target.count() == 1
    assert not page.evaluate('fixtureRequests.control.length || fixtureRequests.submit.length')
    return page.locator('#lesson-scroll').evaluate('node=>node.scrollTop')


def main():
    with local_pages() as base, sync_playwright() as playwright:
        browser = playwright.chromium.launch(headless=True)
        page = browser.new_page(viewport=dict(width=1440, height=900))
        errors, external = [], []
        page.on('pageerror', lambda error: errors.append(str(error)))

        def route(request):
            if request.request.url.startswith(base + '/'):
                request.continue_()
            else:
                external.append(request.request.url)
                request.abort()

        page.route('**/*', route)
        entry = base + '/practice?lessonId=navigation-fixture&sectionId=repair&archived=1&legacy=1'
        page.goto(entry)
        expect(page.locator('#lesson-input')).to_be_enabled()
        first_top = assert_located(page, 'repair')
        assert first_top > 1000
        assert page.locator('.saved-materials').evaluate('node=>node.open')
        # 显式卡片地址优先旧滚动位置；不只改变答题目标。
        page.evaluate("localStorage.setItem('navigation-fixture:scroll:navigation-fixture:practice',JSON.stringify({top:0,bottom:false}))")
        page.reload()
        assert assert_located(page, 'repair') > 1000
        draft = '这是尚未提交的独立作答草稿。'
        page.locator('#lesson-input').fill(draft)
        # 同页“原卡片”入口定位真实题卡并保留原题草稿。
        page.locator('#lesson-next-step a').click()
        assert_located(page, 'question-0')
        assert page.evaluate("localStorage.getItem('navigation-fixture:chat-draft:navigation-fixture:practice')") == draft
        question_draft = '先判断分数单位，再说明理由；这份答案尚未提交。'
        page.locator('#lesson-input').fill(question_draft)
        page.locator('[data-section-id="question-0"] input[type=radio]').first.check()
        expect(page.locator('[data-section-id="question-0"]')).to_have_class(re.compile('is-current-question'))
        page.reload()
        assert_located(page, 'question-0')
        expect(page.locator('#lesson-input')).to_have_value(question_draft)
        expect(page.locator('[data-section-id="question-0"] input[type=radio]').first).to_be_checked()
        # 从讲解页可靠地跨页直达练习中的原补讲，不启动首次讲解任务。
        page.goto(base + '/learn?lessonId=navigation-fixture&sectionId=learn-first')
        assert_located(page, 'learn-first')
        page.locator('#lesson-next-step a').click()
        page.wait_for_url('**/practice?**sectionId=repair**')
        assert_located(page, 'repair')
        # 小结和窄屏也定位原卡片，折叠内容会打开。
        page.goto(base + '/summary?lessonId=navigation-fixture&sectionId=summary-7')
        assert assert_located(page, 'summary-7') > 100
        page.set_viewport_size(dict(width=390, height=780))
        page.reload()
        assert_located(page, 'summary-7')
        # 页名与真实归属不符时先更正地址；不能先触发讲解开课。
        page.goto(base + '/learn?lessonId=navigation-fixture&sectionId=repair')
        page.wait_for_url('**/practice?**sectionId=repair**')
        assert_located(page, 'repair')
        page.goto(base + '/practice?lessonId=navigation-fixture&sectionId=unknown')
        expect(page.locator('#lesson-input')).to_be_enabled()
        assert page.locator('#lesson-sections .chat-section').count() > 0
        assert not errors, errors
        assert not external, external
        assert not page.evaluate('fixtureRequests.control.length || fixtureRequests.submit.length')
        # 教师文字中的站内 Markdown 入口使用真实渲染器，也能进入实际原卡片。
        page.evaluate("""() => {const note=document.createElement('div');note.id='fixture-markdown-link';
          GangyiAgent.richText(note,'[查看原补讲](/practice?lessonId=navigation-fixture&sectionId=repair)');
          document.getElementById('page-status').after(note);}""")
        markdown_link = page.locator('#fixture-markdown-link a')
        assert markdown_link.get_attribute('target') is None
        markdown_link.click()
        page.wait_for_url('**/practice?**sectionId=repair**')
        assert_located(page, 'repair')
        # 首次讲解不能抢占正在处理的练习；收到有效终态后仅续开一次。
        page.goto(base + '/learn?lessonId=navigation-fixture&deferred=1')
        expect(page.locator('#lesson-task-status')).to_have_text('AI 正在结合你的回答互动')
        assert page.evaluate('fixtureRequests.control.length') == 1
        assert not page.evaluate('fixture.initialTeachingTaskId')
        page.evaluate('fixtureFinishOwner()')
        page.wait_for_function("fixture.initialTeachingTaskId==='initial-task'&&fixtureRequests.control.length===2")
        expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
        assert page.evaluate('fixture.teachingTaskId') == 'initial-task'
        assert page.evaluate('fixtureRequests.control.every(event=>event.command==="start_lesson")')
        assert not page.evaluate('fixtureRequests.submit.length')
        # 当前页刚取消原任务时，停止意图优先；不能因终态变成 cancelled 自动启动 B。
        page.goto(base + '/learn?lessonId=navigation-fixture&deferred=1')
        expect(page.locator('#lesson-task-status')).to_have_text('AI 正在结合你的回答互动')
        page.evaluate("fixtureOwnerState('cancelled')")
        expect(page.locator('#lesson-task-status')).to_contain_text('已停止本次回答')
        expect(page.locator('#lesson-retry')).to_be_visible()
        page.evaluate('()=>new Promise(resolve=>requestAnimationFrame(()=>requestAnimationFrame(resolve)))')
        assert page.evaluate('fixtureRequests.control.length') == 1
        assert not page.evaluate('fixture.initialTeachingTaskId')
        assert not errors, errors
        assert not external, external
        browser.close()
    print(f'生产课堂 DOM/脚本离线回归通过：归档原卡片实际滚动 {first_top:.0f}px、旧位置优先级、同页/跨页、小结、窄屏、草稿保持、运行任务结束后续开一次；外部请求与教学提交均为0。')


if __name__ == '__main__':
    main()

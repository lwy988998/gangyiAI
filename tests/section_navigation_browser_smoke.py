# 【逐行批注】 执行当前 Python 语句："""使用生产课堂 DOM、样式和脚本，离线验证原卡片链接的实际滚动与草稿恢复。"""。
"""使用生产课堂 DOM、样式和脚本，离线验证原卡片链接的实际滚动与草稿恢复。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import re。
import re
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextlib import contextmanager。
from contextlib import contextmanager
# 【逐行批注】 导入后续逻辑需要的模块或符号：from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer。
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from threading import Thread。
from threading import Thread
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import urlsplit。
from urllib.parse import urlsplit

# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import expect, sync_playwright。
from playwright.sync_api import expect, sync_playwright


# 【逐行批注】 计算右侧结果并赋给 REPO，更新脚本状态。
REPO = Path(__file__).resolve().parents[1]


# 【逐行批注】 定义函数 saved_lesson，封装可复用的处理步骤。
def saved_lesson():
    # 【逐行批注】 计算右侧结果并赋给 sections，更新脚本状态。
    sections = [dict(id='learn-first', kind='explanation', title='讲解原卡片', body='把同一整体平均分成若干份。',
                     # 【逐行批注】 计算右侧结果并赋给 version，更新脚本状态。
                     version=1, view='learn', href='/learn?lessonId=navigation-fixture&sectionId=learn-first')]
    # 【逐行批注】 遍历数据并重复执行循环体：for index in range(14):。
    for index in range(14):
        # 【逐行批注】 计算右侧结果并赋给 sections.append(dict(id，更新脚本状态。
        sections.append(dict(id=f'question-{index}', kind='question', title=f'原练习 {index + 1}', questionKind='practice',
                             # 【逐行批注】 计算右侧结果并赋给 questionId，更新脚本状态。
                             questionId=f'qid-{index}', version=1, view='practice',
                             # 【逐行批注】 计算右侧结果并赋给 href，更新脚本状态。
                             href=f'/practice?lessonId=navigation-fixture&sectionId=question-{index}',
                             # 【逐行批注】 计算右侧结果并赋给 question，更新脚本状态。
                             question=dict(question='请说明分数单位与平均分的关系。' * 5, type='multi_choice' if index == 1 else 'choice', options=['先统一单位', '直接相加'])))
    # 【逐行批注】 计算右侧结果并赋给 sections.append(dict(id，更新脚本状态。
    sections.append(dict(id='repair', kind='explanation', title='练习中的原补讲', version=1, view='practice',
                         # 【逐行批注】 计算右侧结果并赋给 href，更新脚本状态。
                         href='/practice?lessonId=navigation-fixture&sectionId=repair',
                         # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
                         body='分母表示把同一整体平均分成的份数，每份相当于整体的1/分母。'))
    # 【逐行批注】 遍历数据并重复执行循环体：for index in range(8):。
    for index in range(8):
        # 【逐行批注】 计算右侧结果并赋给 sections.append(dict(id，更新脚本状态。
        sections.append(dict(id=f'summary-{index}', kind='summary', title=f'原小结 {index + 1}', version=1,
                             # 【逐行批注】 计算右侧结果并赋给 view，更新脚本状态。
                             view='summary', href=f'/summary?lessonId=navigation-fixture&sectionId=summary-{index}',
                             # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
                             body='回顾同一整体和相同分数单位，完成情况仍依据原来的真实作答。' * 4))
    # 【逐行批注】 结束当前函数并返回结果：return dict(id='navigation-fixture', courseId='navigation-course', st…。
    return dict(id='navigation-fixture', courseId='navigation-course', status='ready', entered=True,
                # 【逐行批注】 计算右侧结果并赋给 title，更新脚本状态。
                title='原卡片导航隔离夹具', purpose='只回看已保存的内容', sections=sections,
                # 【逐行批注】 计算右侧结果并赋给 teachingTaskId，更新脚本状态。
                teachingTaskId='saved-task', teachingTaskView='practice', evaluations=[],
                # 【逐行批注】 计算右侧结果并赋给 timeline，更新脚本状态。
                timeline=[dict(type='section', sectionId='repair', taskId='saved-task', view='practice')])


# 【逐行批注】 定义函数 classroom_html，封装可复用的处理步骤。
def classroom_html(view):
    # 直接使用生产页面的课堂主体，避免测试自己编造的布局。
    # 【逐行批注】 计算右侧结果并赋给 source，更新脚本状态。
    source = (REPO / 'src/page_renderer.cpp').read_text(encoding='utf-8')
    # 【逐行批注】 计算右侧结果并赋给 function，更新脚本状态。
    function = source.split('std::string renderClassroomPage(', 1)[1]
    # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
    body = re.search(r'headerShell\("learn"\) \+ R"HTML\((.*?)\)HTML";', function, re.S).group(1)
    # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
    body = body.replace(')HTML" + htmlEscape(view) + R"HTML(', view)
    # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
    body = body.replace(')HTML" + label + R"HTML(', dict(learn='讲解', practice='练习', summary='小结')[view])
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'R"HTML' not in body。
    assert 'R"HTML' not in body
    # 【逐行批注】 计算右侧结果并赋给 styles，更新脚本状态。
    styles = ''.join(f'<link rel="stylesheet" href="/{name}">' for name in
                     # 【逐行批注】 执行当前 Python 语句：['styles.css', 'plan.css', 'dark.css', 'app-layout.css', 'classroom.css', 'agent-classroo…。
                     ['styles.css', 'plan.css', 'dark.css', 'app-layout.css', 'classroom.css', 'agent-classroom.css', 'library.css'])
    # 【逐行批注】 计算右侧结果并赋给 fixture，更新脚本状态。
    fixture = json.dumps(saved_lesson(), ensure_ascii=False)
    # 【逐行批注】 计算右侧结果并赋给 script，更新脚本状态。
    script = """
    const fixture = FIXTURE, query = new URLSearchParams(location.search);
    if (query.has('legacy')) fixture.sections.forEach(section => delete section.href);
    if (query.has('archived')) fixture.timeline = [{type:'section',sectionId:'question-0',taskId:'saved-task',view:'practice'}];
    let ownerStatus = 'running';
    const fixtureWatches = new Map();
    window.fixtureRequests = {read:0,visits:[],control:[],submit:[],visible:[]};
    window.fixtureOwnerState = status => { ownerStatus=status; fixtureWatches.get('saved-task')?.onState({id:'saved-task',status}); };
    window.fixtureFinishOwner = () => window.fixtureOwnerState('waiting_student');
    const storageKey = key => 'navigation-fixture:' + key;
    window.GangyiAgent = {
      // 展示和流式状态使用生产客户端，只替换隔离数据与请求边界。
      ...window.GangyiAgent,
      request: async path => { if (!path.startsWith('/api/learning-agent/lesson?')) throw Error('非预期读取');
        window.fixtureRequests.read++; return structuredClone(fixture); },
      control: async event => {
        // 入课只记访问；教学命令另行记录，避免把访问误报为新教学调用。
        if(event.command==='enter_lesson') { window.fixtureRequests.visits.push(event); return {lessonId:fixture.id}; }
        window.fixtureRequests.control.push(event);
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
    # 【逐行批注】 结束当前函数并返回结果：return ('<!doctype html><html lang="zh-CN"><meta charset="utf-8">' + …。
    return ('<!doctype html><html lang="zh-CN"><meta charset="utf-8">' + styles +
            # 【逐行批注】 执行当前 Python 语句：'<body class="dark-app"><header style="height:76px">离线导航回归</header>' + body +。
            '<body class="dark-app"><header style="height:76px">离线导航回归</header>' + body +
            # 【逐行批注】 执行当前 Python 语句：'<script src="/chat-render.js"></script><script>' + script + '</script><script defer src=…。
            '<script src="/chat-render.js"></script><script src="/learning-agent.js"></script><script>' + script + '</script><script defer src="/agent-classroom.js"></script></body></html>').encode()


# 【逐行批注】 应用装饰器 @contextmanager，扩展下一函数或测试的行为。
@contextmanager
# 【逐行批注】 定义函数 local_pages，封装可复用的处理步骤。
def local_pages():
    # 【逐行批注】 定义类 Handler，组织相关数据与行为。
    class Handler(BaseHTTPRequestHandler):
        # 【逐行批注】 定义函数 log_message，封装可复用的处理步骤。
        def log_message(self, *_):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass

        # 【逐行批注】 定义函数 do_GET，封装可复用的处理步骤。
        def do_GET(self):
            # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
            path = urlsplit(self.path).path
            # 【逐行批注】 判断条件 path in ('/learn', '/practice', '/summary'):，成立时执行对应分支。
            if path in ('/learn', '/practice', '/summary'):
                # 【逐行批注】 计算右侧结果并赋给 content, kind，更新脚本状态。
                content, kind = classroom_html(path[1:]), 'text/html; charset=utf-8'
            # 【逐行批注】 当前一条件不成立时继续判断：path in ('/agent-classroom.js', '/chat-render.js') or path.endsw…。
            elif path in ('/agent-classroom.js', '/chat-render.js', '/learning-agent.js') or path.endswith('.css') and '/' not in path[1:]:
                # 【逐行批注】 计算右侧结果并赋给 content，更新脚本状态。
                content = (REPO / 'public' / path[1:]).read_bytes()
                # 【逐行批注】 计算右侧结果并赋给 kind，更新脚本状态。
                kind = 'text/javascript; charset=utf-8' if path.endswith('.js') else 'text/css; charset=utf-8'
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 执行当前 Python 语句：self.send_error(404)。
                self.send_error(404)
                # 【逐行批注】 结束当前函数并返回结果：return。
                return
            # 【逐行批注】 执行当前 Python 语句：self.send_response(200)。
            self.send_response(200)
            # 【逐行批注】 执行当前 Python 语句：self.send_header('Content-Type', kind)。
            self.send_header('Content-Type', kind)
            # 【逐行批注】 执行当前 Python 语句：self.send_header('Cache-Control', 'no-store')。
            self.send_header('Cache-Control', 'no-store')
            # 【逐行批注】 执行当前 Python 语句：self.end_headers()。
            self.end_headers()
            # 【逐行批注】 执行当前 Python 语句：self.wfile.write(content)。
            self.wfile.write(content)

    # 【逐行批注】 计算右侧结果并赋给 server，更新脚本状态。
    server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    # 【逐行批注】 计算右侧结果并赋给 Thread(target，更新脚本状态。
    Thread(target=server.serve_forever, daemon=True).start()
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 执行当前 Python 语句：yield f'http://127.0.0.1:{server.server_port}'。
        yield f'http://127.0.0.1:{server.server_port}'
    # 【逐行批注】 执行当前 Python 语句：finally:。
    finally:
        # 【逐行批注】 执行当前 Python 语句：server.shutdown()。
        server.shutdown()
        # 【逐行批注】 执行当前 Python 语句：server.server_close()。
        server.server_close()


# 【逐行批注】 定义函数 assert_located，封装可复用的处理步骤。
def assert_located(page, identity):
    # 【逐行批注】 计算右侧结果并赋给 target，更新脚本状态。
    target = page.locator(f'[data-section-id="{identity}"]')
    # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(target).to_be_visible()。
    expect(target).to_be_visible()
    # 【逐行批注】 计算右侧结果并赋给 page.wait_for_function("""id，更新脚本状态。
    page.wait_for_function("""id => {const node=document.querySelector('[data-section-id="'+id+'"]'),scroll=document.getElementById('lesson-scroll');
      if(!node)return false;const a=node.getBoundingClientRect(),b=scroll.getBoundingClientRect();return a.top>=b.top-2&&a.top<b.bottom-20;}""", arg=identity)
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert target.count() == 1。
    assert target.count() == 1
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not page.evaluate('fixtureRequests.control.length || fixtureRequests.sub…。
    assert not page.evaluate('fixtureRequests.control.length || fixtureRequests.submit.length')
    assert page.evaluate('fixtureRequests.visits.length===1 && fixtureRequests.visits[0].lessonId===fixture.id')
    # 【逐行批注】 结束当前函数并返回结果：return page.locator('#lesson-scroll').evaluate('node=>node.scrollTop')。
    return page.locator('#lesson-scroll').evaluate('node=>node.scrollTop')


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 在受控上下文中执行资源操作：with local_pages() as base, sync_playwright() as playwright:。
    with local_pages() as base, sync_playwright() as playwright:
        # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
        browser = playwright.chromium.launch(headless=True)
        # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
        page = browser.new_page(viewport=dict(width=1440, height=900))
        # 【逐行批注】 计算右侧结果并赋给 errors, external，更新脚本状态。
        errors, external = [], []
        # 【逐行批注】 调用 str 完成当前脚本或测试步骤：page.on('pageerror', lambda error: errors.append(str(error)))。
        page.on('pageerror', lambda error: errors.append(str(error)))

        # 【逐行批注】 定义函数 route，封装可复用的处理步骤。
        def route(request):
            # 【逐行批注】 判断条件 request.request.url.startswith(base + '/'):，成立时执行对应分支。
            if request.request.url.startswith(base + '/'):
                # 【逐行批注】 执行当前 Python 语句：request.continue_()。
                request.continue_()
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 执行当前 Python 语句：external.append(request.request.url)。
                external.append(request.request.url)
                # 【逐行批注】 执行当前 Python 语句：request.abort()。
                request.abort()

        # 【逐行批注】 执行当前 Python 语句：page.route('** /*', route)。
        page.route('**/*', route)
        # 【逐行批注】 计算右侧结果并赋给 entry，更新脚本状态。
        entry = base + '/practice?lessonId=navigation-fixture&sectionId=repair&archived=1&legacy=1'
        # 【逐行批注】 执行当前 Python 语句：page.goto(entry)。
        page.goto(entry)
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_be_enabled()。
        expect(page.locator('#lesson-input')).to_be_enabled()
        # 【逐行批注】 计算右侧结果并赋给 first_top，更新脚本状态。
        first_top = assert_located(page, 'repair')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert first_top > 1000。
        assert first_top > 1000
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('.saved-materials').evaluate('node=>node.open')。
        assert page.locator('.saved-materials').evaluate('node=>node.open')
        # 显式卡片地址优先旧滚动位置；不只改变答题目标。
        # 【逐行批注】 执行当前 Python 语句：page.evaluate("localStorage.setItem('navigation-fixture:scroll:navigation-fixture:practic…。
        page.evaluate("localStorage.setItem('navigation-fixture:scroll:navigation-fixture:practice',JSON.stringify({top:0,bottom:false}))")
        # 【逐行批注】 执行当前 Python 语句：page.reload()。
        page.reload()
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert assert_located(page, 'repair') > 1000。
        assert assert_located(page, 'repair') > 1000
        # 【逐行批注】 计算右侧结果并赋给 draft，更新脚本状态。
        draft = '这是尚未提交的独立作答草稿。'
        # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-input').fill(draft)。
        page.locator('#lesson-input').fill(draft)
        # 同页“原卡片”入口定位真实题卡并保留原题草稿。
        # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-next-step a').click()。
        page.locator('#lesson-next-step a').click()
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert_located(page, 'question-0')。
        assert_located(page, 'question-0')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.evaluate("localStorage.getItem('navigation-fixture:chat-draft:navig…。
        assert page.evaluate("localStorage.getItem('navigation-fixture:chat-draft:navigation-fixture:practice')") == draft
        # 【逐行批注】 计算右侧结果并赋给 question_draft，更新脚本状态。
        question_draft = '先判断分数单位，再说明理由；这份答案尚未提交。'
        # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-input').fill(question_draft)。
        page.locator('#lesson-input').fill(question_draft)
        # 【逐行批注】 计算右侧结果并赋给 page.locator('[data-section-id，更新脚本状态。
        page.locator('[data-section-id="question-0"] input[type=radio]').first.check()
        # 【逐行批注】 计算右侧结果并赋给 expect(page.locator('[data-section-id，更新脚本状态。
        expect(page.locator('[data-section-id="question-0"]')).to_have_class(re.compile('is-current-question'))
        # 【逐行批注】 执行当前 Python 语句：page.reload()。
        page.reload()
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert_located(page, 'question-0')。
        assert_located(page, 'question-0')
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_have_value(question_draft)。
        expect(page.locator('#lesson-input')).to_have_value(question_draft)
        # 【逐行批注】 计算右侧结果并赋给 expect(page.locator('[data-section-id，更新脚本状态。
        expect(page.locator('[data-section-id="question-0"] input[type=radio]').first).to_be_checked()
        # 从讲解页可靠地跨页直达练习中的原补讲，不启动首次讲解任务。
        # 【逐行批注】 计算右侧结果并赋给 page.goto(base + '/learn?lessonId，更新脚本状态。
        page.goto(base + '/learn?lessonId=navigation-fixture&sectionId=learn-first')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert_located(page, 'learn-first')。
        assert_located(page, 'learn-first')
        # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-next-step a').click()。
        page.locator('#lesson-next-step a').click()
        # 【逐行批注】 计算右侧结果并赋给 page.wait_for_url('** /practice?**sectionId，更新脚本状态。
        page.wait_for_url('**/practice?**sectionId=repair**')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert_located(page, 'repair')。
        assert_located(page, 'repair')
        # 小结和窄屏也定位原卡片，折叠内容会打开。
        # 【逐行批注】 计算右侧结果并赋给 page.goto(base + '/summary?lessonId，更新脚本状态。
        page.goto(base + '/summary?lessonId=navigation-fixture&sectionId=summary-7')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert assert_located(page, 'summary-7') > 100。
        assert assert_located(page, 'summary-7') > 100
        # 【逐行批注】 计算右侧结果并赋给 page.set_viewport_size(dict(width，更新脚本状态。
        page.set_viewport_size(dict(width=390, height=780))
        # 【逐行批注】 执行当前 Python 语句：page.reload()。
        page.reload()
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert_located(page, 'summary-7')。
        assert_located(page, 'summary-7')
        # 页名与真实归属不符时先更正地址；不能先触发讲解开课。
        # 【逐行批注】 计算右侧结果并赋给 page.goto(base + '/learn?lessonId，更新脚本状态。
        page.goto(base + '/learn?lessonId=navigation-fixture&sectionId=repair')
        # 【逐行批注】 计算右侧结果并赋给 page.wait_for_url('** /practice?**sectionId，更新脚本状态。
        page.wait_for_url('**/practice?**sectionId=repair**')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert_located(page, 'repair')。
        assert_located(page, 'repair')
        # 【逐行批注】 计算右侧结果并赋给 page.goto(base + '/practice?lessonId，更新脚本状态。
        page.goto(base + '/practice?lessonId=navigation-fixture&sectionId=unknown')
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-input')).to_be_enabled()。
        expect(page.locator('#lesson-input')).to_be_enabled()
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('#lesson-sections .chat-section').count() > 0。
        assert page.locator('#lesson-sections .chat-section').count() > 0
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, errors。
        assert not errors, errors
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not external, external。
        assert not external, external
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not page.evaluate('fixtureRequests.control.length || fixtureRequests.sub…。
        assert not page.evaluate('fixtureRequests.control.length || fixtureRequests.submit.length')
        # 教师文字中的站内 Markdown 入口使用真实渲染器，也能进入实际原卡片。
        # 【逐行批注】 计算右侧结果并赋给 page.evaluate("""()，更新脚本状态。
        page.evaluate("""() => {const note=document.createElement('div');note.id='fixture-markdown-link';
          GangyiAgent.richText(note,'[查看原补讲](/practice?lessonId=navigation-fixture&sectionId=repair)');
          document.getElementById('page-status').after(note);}""")
        # 【逐行批注】 计算右侧结果并赋给 markdown_link，更新脚本状态。
        markdown_link = page.locator('#fixture-markdown-link a')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert markdown_link.get_attribute('target') is None。
        assert markdown_link.get_attribute('target') is None
        # 【逐行批注】 执行当前 Python 语句：markdown_link.click()。
        markdown_link.click()
        # 【逐行批注】 计算右侧结果并赋给 page.wait_for_url('** /practice?**sectionId，更新脚本状态。
        page.wait_for_url('**/practice?**sectionId=repair**')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert_located(page, 'repair')。
        assert_located(page, 'repair')
        # 首次讲解不能抢占正在处理的练习；收到有效终态后仅续开一次。
        # 【逐行批注】 计算右侧结果并赋给 page.goto(base + '/learn?lessonId，更新脚本状态。
        page.goto(base + '/learn?lessonId=navigation-fixture&deferred=1')
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('AI 正在结合你的回答…。
        expect(page.locator('#lesson-task-status')).to_have_text('AI 正在结合你的回答互动')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.evaluate('fixtureRequests.control.length') == 1。
        assert page.evaluate('fixtureRequests.control.length') == 1
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not page.evaluate('fixture.initialTeachingTaskId')。
        assert not page.evaluate('fixture.initialTeachingTaskId')
        # 【逐行批注】 调用 fixtureFinishOwner 完成当前脚本或测试步骤：page.evaluate('fixtureFinishOwner()')。
        page.evaluate('fixtureFinishOwner()')
        # 【逐行批注】 执行当前 Python 语句：page.wait_for_function("fixture.initialTeachingTaskId==='initial-task'&&fixtureRequests.c…。
        page.wait_for_function("fixture.initialTeachingTaskId==='initial-task'&&fixtureRequests.control.length===2")
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')。
        expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.evaluate('fixture.teachingTaskId') == 'initial-task'。
        assert page.evaluate('fixture.teachingTaskId') == 'initial-task'
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.evaluate('fixtureRequests.control.every(event=>event.command==="sta…。
        assert page.evaluate('fixtureRequests.control.every(event=>event.command==="start_lesson")')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not page.evaluate('fixtureRequests.submit.length')。
        assert not page.evaluate('fixtureRequests.submit.length')
        # 当前页刚取消原任务时，停止意图优先；不能因终态变成 cancelled 自动启动 B。
        # 【逐行批注】 计算右侧结果并赋给 page.goto(base + '/learn?lessonId，更新脚本状态。
        page.goto(base + '/learn?lessonId=navigation-fixture&deferred=1')
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('AI 正在结合你的回答…。
        expect(page.locator('#lesson-task-status')).to_have_text('AI 正在结合你的回答互动')
        # 【逐行批注】 调用 fixtureOwnerState 完成当前脚本或测试步骤：page.evaluate("fixtureOwnerState('cancelled')")。
        page.evaluate("fixtureOwnerState('cancelled')")
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_contain_text('已停止本次回答')。
        expect(page.locator('#lesson-task-status')).to_contain_text('已停止本次回答')
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-retry')).to_be_visible()。
        expect(page.locator('#lesson-retry')).to_be_visible()
        # 【逐行批注】 计算右侧结果并赋给 page.evaluate('()，更新脚本状态。
        page.evaluate('()=>new Promise(resolve=>requestAnimationFrame(()=>requestAnimationFrame(resolve)))')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.evaluate('fixtureRequests.control.length') == 1。
        assert page.evaluate('fixtureRequests.control.length') == 1
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not page.evaluate('fixture.initialTeachingTaskId')。
        assert not page.evaluate('fixture.initialTeachingTaskId')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, errors。
        assert not errors, errors
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not external, external。
        assert not external, external
        # 【逐行批注】 执行当前 Python 语句：browser.close()。
        browser.close()
    # 【逐行批注】 输出当前运行结果：print(f'生产课堂 DOM/脚本离线回归通过：归档原卡片实际滚动 {first_top:.0f}px、旧位置优先级、同页/跨页、小结、窄屏、草…。
    print(f'生产课堂 DOM/脚本离线回归通过：归档原卡片实际滚动 {first_top:.0f}px、旧位置优先级、同页/跨页、小结、窄屏、草稿保持、运行任务结束后续开一次；外部请求与教学提交均为0。')


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()

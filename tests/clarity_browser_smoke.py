# 【逐行批注】 执行当前 Python 语句："""使用隔离档案验证全局澄明布局、课堂可用区域与个人中心页面。"""。
"""使用隔离档案验证全局澄明布局、课堂可用区域与个人中心页面。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import urlencode。
from urllib.parse import urlencode

# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import sync_playwright。
from playwright.sync_api import sync_playwright
# 【逐行批注】 导入后续逻辑需要的模块或符号：import classroom_v560_fixture as fixture。
import classroom_v560_fixture as fixture


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 计算右侧结果并赋给 executable，更新脚本状态。
    executable = sys.argv[1]
    # 【逐行批注】 计算右侧结果并赋给 output，更新脚本状态。
    output = Path(sys.argv[2] if len(sys.argv)>2 else 'verification/clarity-ui').resolve()
    # 【逐行批注】 计算右侧结果并赋给 output.mkdir(parents，更新脚本状态。
    output.mkdir(parents=True, exist_ok=True)
    # 【逐行批注】 计算右侧结果并赋给 results，更新脚本状态。
    results = []
    # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
    errors = []
    # 【逐行批注】 在受控上下文中执行资源操作：with fixture.start_session(executable) as (info, api, process):。
    with fixture.start_session(executable) as (info, api, process):
        # 【逐行批注】 计算右侧结果并赋给 query，更新脚本状态。
        query = urlencode({'courseId':info['courseId'],'lessonId':info['lessonId'],'phaseIndex':1,'topicIndex':1})
        # 【逐行批注】 计算右侧结果并赋给 personal，更新脚本状态。
        personal = ['', 'chat', 'library', 'reports', 'plan', 'preferences', 'memories', 'tasks', 'usage', 'profile', 'settings', 'data']
        # 【逐行批注】 计算右侧结果并赋给 routes，更新脚本状态。
        routes = [('/', 'home', '/'),('/my-courses','courses','/my-courses'),
                  # 【逐行批注】 执行当前 Python 语句：('/current-course','current-course','/current-course'),。
                  ('/current-course','current-course','/current-course'),
                  # 【逐行批注】 执行当前 Python 语句：('/plan?'+query,'course','/current-course'),('/phase?'+query,'phase','/current-course'),。
                  ('/plan?'+query,'course','/current-course'),('/phase?'+query,'phase','/current-course'),
                  # 【逐行批注】 执行当前 Python 语句：('/learn?'+query,'quick-learn','/current-course'),('/practice?'+query,'quick-practice','/…。
                  ('/learn?'+query,'quick-learn','/current-course'),('/practice?'+query,'quick-practice','/current-course'),
                  # 【逐行批注】 执行当前 Python 语句：('/summary?'+query,'quick-summary','/current-course'),('/ask','tutor','/ask'),。
                  ('/summary?'+query,'quick-summary','/current-course'),('/ask','tutor','/ask'),
                  # 未知任务只显示备课错误态，不提交生成，确保检查静态备课页本身而非跳走后的课堂。
                  # 【逐行批注】 计算右侧结果并赋给 ('/agent-prepare.html?taskId，更新脚本状态。
                  ('/agent-prepare.html?taskId=navigation-only&courseId='+info['courseId'],'prepare-header','/current-course'),
                  # 【逐行批注】 执行当前 Python 语句：('/deep-course?'+query,'deep-header','/current-course')]。
                  ('/deep-course?'+query,'deep-header','/current-course')]
        # 【逐行批注】 计算右侧结果并赋给 routes +，更新脚本状态。
        routes += [('/my-ai'+('/'+item if item else ''),'my-ai-'+(item or 'overview'),'/my-ai') for item in personal]
        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as playwright:。
        with sync_playwright() as playwright:
            # 【逐行批注】 计算右侧结果并赋给 browser_options，更新脚本状态。
            browser_options={'headless':True}
            # 【逐行批注】 判断条件 os.name=='nt' and not Path(playwright.chromium.executable_path).…，成立时执行对应分支。
            if os.name=='nt' and not Path(playwright.chromium.executable_path).is_file():
                # 【逐行批注】 计算右侧结果并赋给 browser_options['channel']，更新脚本状态。
                browser_options['channel']='msedge'
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser = playwright.chromium.launch(**browser_options)
            # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
            context = browser.new_context(reduced_motion='reduce')
            # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
            page = context.new_page()
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：page.on('pageerror',lambda error:errors.append(str(error)))。
            page.on('pageerror',lambda error:errors.append(str(error)))
            # 【逐行批注】 遍历数据并重复执行循环体：for width,height in [(1280,720),(1440,900),(390,844)]:。
            for width,height in [(1280,720),(1440,900),(390,844)]:
                # 【逐行批注】 执行当前 Python 语句：page.set_viewport_size({'width':width,'height':height})。
                page.set_viewport_size({'width':width,'height':height})
                # 【逐行批注】 遍历数据并重复执行循环体：for route,name,expected_navigation in routes:。
                for route,name,expected_navigation in routes:
                    # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
                    response=page.goto(info['base']+route,wait_until='networkidle')
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert response and response.ok, f'页面打开失败：{route}'。
                    assert response and response.ok, f'页面打开失败：{route}'
                    # 【逐行批注】 执行当前 Python 语句：page.evaluate('document.fonts.ready')。
                    page.evaluate('document.fonts.ready')
                    # 【逐行批注】 执行当前 Python 语句：page.wait_for_timeout(200)。
                    page.wait_for_timeout(200)
                    compact = page.locator('.gy-learning-menu-toggle').count() == 1
                    if compact:
                        fixture.open_learning_menu(page, '全站导航与设置')
                    # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
                    state=page.evaluate('''() => {
                      const visible = el => {const r=el.getBoundingClientRect(),s=getComputedStyle(el);return r.width>0&&r.height>0&&s.visibility!=='hidden'&&s.display!=='none'};
                      const composer=document.querySelector('.lesson-chat-panel');
                      const logo=Array.from(document.images).filter(visible).some(img=>/school-logo/.test(img.src)&&img.complete&&img.naturalWidth>0);
                      const header=document.querySelector('.site-header,.my-ai-top');
                      const sections=new Set(['/','/my-courses','/current-course','/ask','/my-ai']);
                      const menus=header.querySelector('.gy-learning-menu-links') ? [header.querySelector('.gy-learning-menu-links')] : [...header.querySelectorAll('nav')];
                      const navigation=menus.map(nav=>{
                        const links=[...nav.querySelectorAll('a')].map(link=>({link,
                          section:link.hasAttribute('data-current-course-link')?'/current-course':
                            sections.has(link.getAttribute('href'))?link.getAttribute('href'):null})).filter(item=>item.section);
                        return {active:links.filter(item=>item.link.classList.contains('is-active')).map(item=>item.section),
                          current:links.filter(item=>item.link.getAttribute('aria-current')==='page').map(item=>item.section)};
                      });
                      return {title:document.title,overflow:document.documentElement.scrollWidth>innerWidth+1,
                        logo,navigation,background:getComputedStyle(document.body).backgroundColor,
                        composer:composer&&visible(composer)?{top:composer.getBoundingClientRect().top,bottom:composer.getBoundingClientRect().bottom}:null,
                        viewportHeight:innerHeight};
                    }''')
                    # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
                    row={'route':route,'name':name,'width':width,'height':height,**state}
                    # 【逐行批注】 执行当前 Python 语句：results.append(row)。
                    results.append(row)
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not state['overflow'],f'页面横向溢出：{name}@{width}'。
                    assert not state['overflow'],f'页面横向溢出：{name}@{width}'
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['logo'],f'校徽不可见：{name}@{width}'。
                    assert state['logo'],f'校徽不可见：{name}@{width}'
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['navigation'],f'顶部主导航缺失：{name}@{width}'。
                    assert state['navigation'],f'顶部主导航缺失：{name}@{width}'
                    # 【逐行批注】 遍历数据并重复执行循环体：for navigation in state['navigation']:。
                    for navigation in state['navigation']:
                        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert navigation['active'] == [expected_navigation],f'导航高亮错误或不唯一：{name}@{width…。
                        assert navigation['active'] == [expected_navigation],f'导航高亮错误或不唯一：{name}@{width} {navigation}'
                        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert navigation['current'] == [expected_navigation],f'导航当前页语义错误或不唯一：{name}@{w…。
                        assert navigation['current'] == [expected_navigation],f'导航当前页语义错误或不唯一：{name}@{width} {navigation}'
                    # 【逐行批注】 判断条件 state['composer']:，成立时执行对应分支。
                    if state['composer']:
                        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert state['composer']['bottom'] <= height+2,f'课堂输入区在视口外：{name}@{width}'。
                        assert state['composer']['bottom'] <= height+2,f'课堂输入区在视口外：{name}@{width}'
                    if compact:
                        page.keyboard.press('Escape')
                    # 【逐行批注】 计算右侧结果并赋给 awaitable_name，更新脚本状态。
                    awaitable_name=f'{name}-{width}.png'
                    # 【逐行批注】 计算右侧结果并赋给 page.screenshot(path，更新脚本状态。
                    page.screenshot(path=str(output/awaitable_name),full_page=True)
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/learn?'+query,wait_…，更新脚本状态。
            page.goto(info['base']+'/learn?'+query,wait_until='domcontentloaded')
            # 【逐行批注】 计算右侧结果并赋给 trigger，更新脚本状态。
            fixture.open_learning_menu(page, '课堂操作')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.locator('#classroom-more').is_visible(),'更多课堂操作未展开'。
            assert page.locator('#classroom-more').is_visible(),'更多课堂操作未展开'
            # 【逐行批注】 执行当前 Python 语句：page.keyboard.press('Escape')。
            page.keyboard.press('Escape')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not page.locator('#classroom-more').is_visible(),'Escape未关闭课堂操作菜单'。
            assert not page.locator('#classroom-more').is_visible(),'Escape未关闭课堂操作菜单'
            # 【逐行批注】 执行当前 Python 语句：browser.close()。
            browser.close()
    # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
    report={'checks':results,'pageErrors':errors,'passed':not errors}
    # 【逐行批注】 计算右侧结果并赋给 (output/'report.json').write_text(json.dumps…，更新脚本状态。
    (output/'report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, f'页面脚本错误：{errors}'。
    assert not errors, f'页面脚本错误：{errors}'
    # 【逐行批注】 输出当前运行结果：print(json.dumps({'passed':True,'views':len(results),'pageErrors':errors},…。
    print(json.dumps({'passed':True,'views':len(results),'pageErrors':errors},ensure_ascii=False))


# 【逐行批注】 判断条件 __name__=='__main__':，成立时执行对应分支。
if __name__=='__main__':
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()

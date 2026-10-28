"""使用隔离档案验证全局澄明布局、课堂可用区域与个人中心页面。"""
import json
import os
from pathlib import Path
import sys
from urllib.parse import urlencode

from playwright.sync_api import sync_playwright
import classroom_v560_fixture as fixture


def main():
    executable = sys.argv[1]
    output = Path(sys.argv[2] if len(sys.argv)>2 else 'verification/clarity-ui').resolve()
    output.mkdir(parents=True, exist_ok=True)
    results = []
    errors = []
    with fixture.start_session(executable) as (info, api, process):
        query = urlencode({'courseId':info['courseId'],'lessonId':info['lessonId'],'phaseIndex':1,'topicIndex':1})
        personal = ['', 'chat', 'library', 'reports', 'plan', 'preferences', 'memories', 'tasks', 'usage', 'profile', 'settings', 'data']
        routes = [('/', 'home'),('/my-courses','courses'),('/plan?'+query,'course'),('/phase?'+query,'phase'),
                  ('/learn?'+query,'quick-learn'),('/practice?'+query,'quick-practice'),('/summary?'+query,'quick-summary'),('/ask','tutor')]
        routes += [('/my-ai'+('/'+item if item else ''),'my-ai-'+(item or 'overview')) for item in personal]
        with sync_playwright() as playwright:
            browser_options={'headless':True}
            if os.name=='nt' and not Path(playwright.chromium.executable_path).is_file():
                browser_options['channel']='msedge'
            browser = playwright.chromium.launch(**browser_options)
            context = browser.new_context(reduced_motion='reduce')
            page = context.new_page()
            page.on('pageerror',lambda error:errors.append(str(error)))
            for width,height in [(1280,720),(1440,900),(390,844)]:
                page.set_viewport_size({'width':width,'height':height})
                for route,name in routes:
                    response=page.goto(info['base']+route,wait_until='networkidle')
                    assert response and response.ok, f'页面打开失败：{route}'
                    page.evaluate('document.fonts.ready')
                    page.wait_for_timeout(200)
                    state=page.evaluate('''() => {
                      const visible = el => {const r=el.getBoundingClientRect(),s=getComputedStyle(el);return r.width>0&&r.height>0&&s.visibility!=='hidden'&&s.display!=='none'};
                      const composer=document.querySelector('.lesson-chat-panel');
                      const logo=Array.from(document.images).filter(visible).some(img=>/school-logo/.test(img.src)&&img.complete&&img.naturalWidth>0);
                      return {title:document.title,overflow:document.documentElement.scrollWidth>innerWidth+1,
                        logo,background:getComputedStyle(document.body).backgroundColor,
                        composer:composer&&visible(composer)?{top:composer.getBoundingClientRect().top,bottom:composer.getBoundingClientRect().bottom}:null,
                        viewportHeight:innerHeight};
                    }''')
                    row={'route':route,'name':name,'width':width,'height':height,**state}
                    results.append(row)
                    assert not state['overflow'],f'页面横向溢出：{name}@{width}'
                    assert state['logo'],f'校徽不可见：{name}@{width}'
                    if state['composer']:
                        assert state['composer']['bottom'] <= height+2,f'课堂输入区在视口外：{name}@{width}'
                    awaitable_name=f'{name}-{width}.png'
                    page.screenshot(path=str(output/awaitable_name),full_page=True)
            page.goto(info['base']+'/learn?'+query,wait_until='domcontentloaded')
            trigger=page.locator('[popovertarget="classroom-more"]')
            trigger.click()
            assert page.locator('#classroom-more').is_visible(),'更多课堂操作未展开'
            page.keyboard.press('Escape')
            assert not page.locator('#classroom-more').is_visible(),'Escape未关闭课堂操作菜单'
            browser.close()
    report={'checks':results,'pageErrors':errors,'passed':not errors}
    (output/'report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    assert not errors, f'页面脚本错误：{errors}'
    print(json.dumps({'passed':True,'views':len(results),'pageErrors':errors},ensure_ascii=False))


if __name__=='__main__':
    main()

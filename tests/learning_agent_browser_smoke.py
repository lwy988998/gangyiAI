"""统一课堂的持续集成浏览器回归；使用隔离协议档案，不读取真实用户记录。"""
import json
import sqlite3
import sys
from contextlib import closing
from urllib.request import Request, urlopen
from playwright.sync_api import expect, sync_playwright
from classroom_v560_fixture import start_session


def main(executable):
    with start_session(executable) as (info, api, process):
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True)
            page = browser.new_page(viewport={'width':1440,'height':900},reduced_motion='reduce')
            errors=[]
            page.on('pageerror',lambda error:errors.append(str(error)))
            lesson=info['lessonId']
            page.goto(info['base']+'/learn?lessonId='+lesson)
            page.get_by_role('heading',name='先比较同一种元素',exact=True).wait_for()
            page.get_by_role('heading',name='说说你的观察',exact=True).wait_for()
            page.get_by_text('你可以继续回答或追问',exact=True).wait_for()
            # 桌面目录和窄屏目录各保留一份当前项，CSS 决定显示哪一份。
            expect(page.locator('.course-directory .course-directory-topic[aria-current=page]')).to_have_count(1)
            expect(page.locator('.course-directory-mobile .course-directory-topic[aria-current=page]')).to_have_count(1)
            assert page.locator('.current-explanation').count()==1
            assert page.locator('#lesson-sections').inner_text().find('PRIVATE-')<0
            before=api('/api/learning-agent/lesson?lessonId='+lesson)
            assert 'expectedAnswer' not in json.dumps(before) and 'rubric' not in json.dumps(before)
            field=page.get_by_role('textbox',name='你的答案、解题过程或追问')
            field.fill('本段草稿，尚未提交。')
            page.reload()
            field=page.get_by_role('textbox',name='你的答案、解题过程或追问')
            assert field.input_value()=='本段草稿，尚未提交。'
            page.get_by_role('link',name='练习',exact=True).click()
            page.get_by_role('heading',name='练习一：判断变化',exact=True).wait_for()
            field=page.get_by_role('textbox',name='你的答案、解题过程或追问')
            field.fill('练习草稿，不同于讲解。')
            page.get_by_role('button',name='下一题 →',exact=True).click()
            assert page.locator('.current-practice').count()==1
            page.get_by_role('button',name='← 上一题',exact=True).click()
            assert field.input_value()=='练习草稿，不同于讲解。'
            page.get_by_role('link',name='小结',exact=True).click()
            page.get_by_text('本节尚无足够的实际作答评价；阅读和追问不表示掌握。',exact=True).wait_for()
            page.get_by_role('link',name='练习',exact=True).click()
            field=page.get_by_role('textbox',name='你的答案、解题过程或追问')
            field.fill('我认为从0升高到+2。')
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":true}',headers={'Content-Type':'application/json'})): pass
            page.get_by_role('button',name='发送给 AI',exact=True).click()
            page.get_by_text('练习 · 导航',exact=True).click()
            page.get_by_text('AI 教学状态',exact=True).click()
            page.get_by_role('button',name='暂停 AI',exact=True).click()
            page.locator('.current-practice').get_by_text('AI 已暂停',exact=True).wait_for()
            with urlopen(Request(info['fixture']+'/fixture-control',data=b'{"slow":false}',headers={'Content-Type':'application/json'})): pass
            page.get_by_role('button',name='恢复 AI',exact=True).click()
            page.get_by_text('你已经给出了观察，接下来可以补充电子变化的理由。',exact=True).wait_for()
            page.get_by_text('你可以继续回答或追问',exact=True).wait_for()
            page.get_by_text('练习 · 导航',exact=True).click()
            field.fill('为什么要先比较同一种元素？')
            page.get_by_role('button',name='发送给 AI',exact=True).click()
            page.get_by_text('这是对思路的追问。我们可以先比较前后状态，再联系电子变化。',exact=True).wait_for()
            page.get_by_text('你可以继续回答或追问',exact=True).wait_for()
            assert page.locator('.current-practice .agent-dialog > .agent-message.user').count()==1
            assert page.locator('.dialogue-history').count()==1
            page.reload()
            page.get_by_text('这是对思路的追问。我们可以先比较前后状态，再联系电子变化。',exact=True).wait_for()
            page.get_by_role('link',name='小结',exact=True).click()
            page.get_by_role('heading',name='本次作答 · 80 分',exact=True).wait_for()
            page.get_by_role('link',name='讲解',exact=True).click()
            page.get_by_role('button',name='请 AI 继续 →',exact=True).click()
            page.get_by_role('heading',name='把化合价变化联系到电子',exact=True).wait_for()
            page.set_viewport_size({'width':390,'height':844})
            assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
            assert page.locator('.course-directory-mobile').is_visible()
            assert not page.locator('.course-directory').is_visible()
            assert not errors,errors
            browser.close()
        with closing(sqlite3.connect(info['database'])) as database:
            values=[json.loads(row[0]) for row in database.execute("SELECT payload FROM LearningInteraction WHERE kind='practice'")]
            assert len(values)==1 and values[0]['response']=='我认为从0升高到+2。'
            assert values[0]['assisted'] and values[0]['model']=='fixture-v560'
        print('C 课堂、目录、三页切换、一题一屏、分题草稿、连续交流、纯追问不评分与窄屏回归通过（隔离协议夹具）')


if __name__=='__main__':
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])

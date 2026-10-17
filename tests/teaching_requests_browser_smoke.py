"""验证真实页面发送教学请求、保留草稿与选项，且不生成作答评价。"""
import json
import sqlite3
import sys
from contextlib import closing
from playwright.sync_api import expect, sync_playwright
from classroom_v560_fixture import start_session


def main(executable):
    with start_session(executable) as (info, api, _):
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True)
            page = browser.new_page(viewport={'width':1440,'height':900})
            errors = []
            page.on('pageerror', lambda error:errors.append(str(error)))
            page.goto(info['base']+'/practice?lessonId='+info['lessonId'])
            field=page.locator('#lesson-input'); expect(field).to_be_enabled()
            radio=page.locator('.agent-options input[type=radio]').first
            radio.check()
            draft='这份作答草稿还没写完，请先保留。'
            field.fill(draft)
            target=page.locator('.chat-section.is-current-question').get_attribute('data-section-id')
            for button,kind in [('teaching-hint','hint'),('teaching-example','worked_example'),('explain-again','rephrase'),('teaching-retry','retry_question')]:
                expect(page.locator('#'+button)).to_be_enabled()
                with page.expect_request(lambda req:req.url.endswith('/api/learning-agent/events')) as sent:
                    page.locator('#'+button).click()
                event=sent.value.post_data_json
                assert event['type']=='teaching_request' and event['requestType']==kind
                assert event['sectionId']==target and event['sectionVersion']==1
                expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
                expect(field).to_have_value(draft); expect(radio).to_be_checked()
                page.reload(); expect(field).to_have_value(draft); expect(radio).to_be_checked()
            with closing(sqlite3.connect(info['database'])) as db:
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practice'").fetchone()[0]==0
                requests=[json.loads(row[0]) for row in db.execute("SELECT payload FROM LearningInteraction WHERE kind='chat-user'")]
                assert len(requests)==4 and all(r['type']=='teaching_request' for r in requests)
            for width,height in [(960,600),(390,780)]:
                page.set_viewport_size({'width':width,'height':height})
                expect(field).to_be_visible()
                assert page.evaluate('document.documentElement.scrollWidth <= innerWidth')
                box=field.bounding_box();assert box['y']+box['height']<=height
            assert not errors,errors
            browser.close()
        print('四种教学请求、题目关联、草稿及选项保持、刷新恢复、不评分与窄屏回归通过。')


if __name__=='__main__':
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])

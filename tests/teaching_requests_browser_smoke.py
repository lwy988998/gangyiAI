# 【逐行批注】 执行当前 Python 语句："""验证真实页面发送教学请求、保留草稿与选项，且不生成作答评价。"""。
"""验证真实页面发送教学请求、保留草稿与选项，且不生成作答评价。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sqlite3。
import sqlite3
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextlib import closing。
from contextlib import closing
# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import expect, sync_playwright。
from playwright.sync_api import expect, sync_playwright
# 【逐行批注】 导入后续逻辑需要的模块或符号：from classroom_v560_fixture import start_session。
from classroom_v560_fixture import launch_test_browser, open_learning_menu, start_session


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable):
    # 【逐行批注】 在受控上下文中执行资源操作：with start_session(executable) as (info, api, _):。
    with start_session(executable) as (info, api, _):
        # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as playwright:。
        with sync_playwright() as playwright:
            # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
            browser = launch_test_browser(playwright)
            # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
            page = browser.new_page(viewport={'width':1440,'height':900})
            # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
            errors = []
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：page.on('pageerror', lambda error:errors.append(str(error)))。
            page.on('pageerror', lambda error:errors.append(str(error)))
            # 【逐行批注】 计算右侧结果并赋给 page.goto(info['base']+'/practice?lessonId，更新脚本状态。
            page.goto(info['base']+'/practice?lessonId='+info['lessonId'])
            # 【逐行批注】 计算右侧结果并赋给 field，更新脚本状态。
            field=page.locator('#lesson-input'); expect(field).to_be_enabled()
            # 【逐行批注】 计算右侧结果并赋给 radio，更新脚本状态。
            radio=page.locator('.agent-options input[type=radio]').first
            # 【逐行批注】 执行当前 Python 语句：radio.check()。
            radio.check()
            # 【逐行批注】 计算右侧结果并赋给 draft，更新脚本状态。
            draft='这份作答草稿还没写完，请先保留。'
            # 【逐行批注】 执行当前 Python 语句：field.fill(draft)。
            field.fill(draft)
            # 【逐行批注】 计算右侧结果并赋给 target，更新脚本状态。
            target=page.locator('.chat-section.is-current-question').get_attribute('data-section-id')
            # 【逐行批注】 遍历数据并重复执行循环体：for button,kind in [('teaching-hint','hint'),('teaching-example','worked_e…。
            for button,kind in [('teaching-hint','hint'),('teaching-example','worked_example'),('explain-again','rephrase'),('teaching-retry','retry_question')]:
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#'+button)).to_be_enabled()。
                expect(page.locator('#'+button)).to_be_enabled()
                # 【逐行批注】 计算右侧结果并赋给 page.locator('[popovertarget，更新脚本状态。
                open_learning_menu(page, '课堂操作')
                # 【逐行批注】 在受控上下文中执行资源操作：with page.expect_request(lambda req:req.url.endswith('/api/learning-agent/event…。
                with page.expect_request(lambda req:req.url.endswith('/api/learning-agent/events')) as sent:
                    # 【逐行批注】 执行当前 Python 语句：page.locator('#'+button).click()。
                    page.locator('#'+button).click()
                # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
                event=sent.value.post_data_json
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert event['type']=='teaching_request' and event['requestType']==kind。
                assert event['type']=='teaching_request' and event['requestType']==kind
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert event['sectionId']==target and event['sectionVersion']==1。
                assert event['sectionId']==target and event['sectionVersion']==1
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')。
                expect(page.locator('#lesson-task-status')).to_have_text('你可以继续回答或追问')
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(field).to_have_value(draft); expect(radio).to_be_checked()。
                expect(field).to_have_value(draft); expect(radio).to_be_checked()
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：page.reload(); expect(field).to_have_value(draft); expect(radio).to_b…。
                page.reload(); expect(field).to_have_value(draft); expect(radio).to_be_checked()
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(info['database'])) as db:。
            with closing(sqlite3.connect(info['database'])) as db:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practic…。
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind='practice'").fetchone()[0]==0
                # 【逐行批注】 计算右侧结果并赋给 requests，更新脚本状态。
                requests=[json.loads(row[0]) for row in db.execute("SELECT payload FROM LearningInteraction WHERE kind='chat-user'")]
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(requests)==4 and all(r['type']=='teaching_request' for r in requests)。
                assert len(requests)==4 and all(r['type']=='teaching_request' for r in requests)
            # 【逐行批注】 遍历数据并重复执行循环体：for width,height in [(960,600),(390,780)]:。
            for width,height in [(960,600),(390,780)]:
                # 【逐行批注】 执行当前 Python 语句：page.set_viewport_size({'width':width,'height':height})。
                page.set_viewport_size({'width':width,'height':height})
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(field).to_be_visible()。
                expect(field).to_be_visible()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert page.evaluate('document.documentElement.scrollWidth <= innerWidth')。
                assert page.evaluate('document.documentElement.scrollWidth <= innerWidth')
                # 【逐行批注】 计算右侧结果并赋给 box，更新脚本状态。
                box=field.bounding_box();assert box['y']+box['height']<=height
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors,errors。
            assert not errors,errors
            # 【逐行批注】 执行当前 Python 语句：browser.close()。
            browser.close()
        # 【逐行批注】 输出当前运行结果：print('四种教学请求、题目关联、草稿及选项保持、刷新恢复、不评分与窄屏回归通过。')。
        print('四种教学请求、题目关联、草稿及选项保持、刷新恢复、不评分与窄屏回归通过。')


# 【逐行批注】 判断条件 __name__=='__main__':，成立时执行对应分支。
if __name__=='__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encodin…，成立时执行对应分支。
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(sys.argv[1])。
    main(sys.argv[1])

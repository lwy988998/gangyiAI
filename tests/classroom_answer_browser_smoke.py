# 【逐行批注】 执行当前 Python 语句："""用生产课堂脚本验证底部和题卡提交一致、选项单独提交以及自由交流不夹带答案。"""。
"""用生产课堂脚本验证底部和题卡提交一致、选项单独提交以及自由交流不夹带答案。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import expect, sync_playwright。
from playwright.sync_api import expect, sync_playwright
# 【逐行批注】 导入后续逻辑需要的模块或符号：from section_navigation_browser_smoke import local_pages。
from section_navigation_browser_smoke import local_pages


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 在受控上下文中执行资源操作：with local_pages() as base, sync_playwright() as playwright:。
    with local_pages() as base, sync_playwright() as playwright:
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
            # 【逐行批注】 判断条件 route.request.url.startswith(base + '/'):，成立时执行对应分支。
            if route.request.url.startswith(base + '/'):
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
        # 【逐行批注】 计算右侧结果并赋给 page.goto(base + '/practice?lessonId，更新脚本状态。
        page.goto(base + '/practice?lessonId=navigation-fixture&sectionId=question-0&answer-test=1')
        # 【逐行批注】 计算右侧结果并赋给 field，更新脚本状态。
        field = page.locator('#lesson-input')
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(field).to_be_enabled()。
        expect(field).to_be_enabled()
        # 【逐行批注】 计算右侧结果并赋给 card，更新脚本状态。
        card = page.locator('[data-section-id="question-0"]')

        # 【逐行批注】 定义函数 captured，封装可复用的处理步骤。
        def captured(count):
            # 【逐行批注】 计算右侧结果并赋给 page.wait_for_function('count，更新脚本状态。
            page.wait_for_function('count=>fixtureRequests.submit.length===count', arg=count)
            # 【逐行批注】 结束当前函数并返回结果：return page.evaluate('structuredClone(fixtureRequests.submit.at(-1))')。
            return page.evaluate('structuredClone(fixtureRequests.submit.at(-1))')

        # 【逐行批注】 计算右侧结果并赋给 reason，更新脚本状态。
        reason = '我是根据分数单位来判断的。'
        # 【逐行批注】 遍历数据并重复执行循环体：for index, mode in enumerate(['composer', 'card', 'enter'], 1):。
        for index, mode in enumerate(['composer', 'card', 'enter'], 1):
            # 【逐行批注】 计算右侧结果并赋给 card.locator('input[type，更新脚本状态。
            card.locator('input[type=radio]').last.check()
            # 【逐行批注】 执行当前 Python 语句：field.fill(reason)。
            field.fill(reason)
            # 【逐行批注】 判断条件 mode == 'composer':，成立时执行对应分支。
            if mode == 'composer':
                # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-send').click()。
                page.locator('#lesson-send').click()
            # 【逐行批注】 当前一条件不成立时继续判断：mode == 'card':。
            elif mode == 'card':
                # 【逐行批注】 计算右侧结果并赋给 card.get_by_role('button', name，更新脚本状态。
                card.get_by_role('button', name='提交答案', exact=True).click()
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 执行当前 Python 语句：field.press('Enter')。
                field.press('Enter')
            # 【逐行批注】 计算右侧结果并赋给 answer，更新脚本状态。
            answer = captured(index)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer['type'] == 'question_answer' and answer['action'] == 'answer'。
            assert answer['type'] == 'question_answer' and answer['action'] == 'answer'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer['text'] == '我选2：直接相加\n' + reason。
            assert answer['text'] == '我选2：直接相加\n' + reason
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(field).to_have_value('')。
            expect(field).to_have_value('')
            # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(card.locator('input:checked')).to_have_count(0)。
            expect(card.locator('input:checked')).to_have_count(0)
        # 只选选项也能使用底部发送，不能要求额外写一段文字。
        # 【逐行批注】 计算右侧结果并赋给 card.locator('input[type，更新脚本状态。
        card.locator('input[type=radio]').first.check()
        # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-send').click()。
        page.locator('#lesson-send').click()
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert captured(4)['text'] == '我选1：先统一单位\n'。
        assert captured(4)['text'] == '我选1：先统一单位\n'
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(card.locator('input:checked')).to_have_count(0)。
        expect(card.locator('input:checked')).to_have_count(0)
        # 明确切到自由交流后，仍保留原选项草稿，纯追问不会变成一份选项答案。
        # 【逐行批注】 计算右侧结果并赋给 card.locator('input[type，更新脚本状态。
        card.locator('input[type=radio]').last.check()
        # 【逐行批注】 执行当前 Python 语句：page.locator('#composer-chat').click()。
        page.locator('#composer-chat').click()
        # 【逐行批注】 执行当前 Python 语句：field.fill('为什么要先统一分数单位？')。
        field.fill('为什么要先统一分数单位？')
        # 【逐行批注】 执行当前 Python 语句：page.locator('#lesson-send').click()。
        page.locator('#lesson-send').click()
        # 【逐行批注】 计算右侧结果并赋给 question，更新脚本状态。
        question = captured(5)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert question['type'] == 'chat' and question['text'] == '为什么要先统一分数单位？'。
        assert question['type'] == 'chat' and question['text'] == '为什么要先统一分数单位？'
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '我选' not in question['text']。
        assert '我选' not in question['text']
        # 【逐行批注】 计算右侧结果并赋给 expect(card.locator('input[type，更新脚本状态。
        expect(card.locator('input[type=radio]').last).to_be_checked()
        # 多选同样携带全部选择；刷新后的选项不应重复提交已经清除的答案。
        # 【逐行批注】 计算右侧结果并赋给 page.goto(base + '/practice?lessonId，更新脚本状态。
        page.goto(base + '/practice?lessonId=navigation-fixture&sectionId=question-1&answer-test=1')
        # 【逐行批注】 计算右侧结果并赋给 multi，更新脚本状态。
        multi = page.locator('[data-section-id="question-1"]')
        # 【逐行批注】 计算右侧结果并赋给 expect(multi.locator('input[type，更新脚本状态。
        expect(multi.locator('input[type=checkbox]')).to_have_count(2)
        # 【逐行批注】 遍历数据并重复执行循环体：for option in multi.locator('input[type=checkbox]').all():。
        for option in multi.locator('input[type=checkbox]').all():
            # 【逐行批注】 执行当前 Python 语句：option.check()。
            option.check()
        # 【逐行批注】 执行当前 Python 语句：field.fill(reason)。
        field.fill(reason)
        # 【逐行批注】 执行当前 Python 语句：field.press('Enter')。
        field.press('Enter')
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert captured(1)['text'] == '我选1：先统一单位、2：直接相加\n' + reason。
        assert captured(1)['text'] == '我选1：先统一单位、2：直接相加\n' + reason
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(multi.locator('input:checked')).to_have_count(0)。
        expect(multi.locator('input:checked')).to_have_count(0)
        # 【逐行批注】 执行当前 Python 语句：page.reload()。
        page.reload()
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(field).to_have_value('')。
        expect(field).to_have_value('')
        # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(multi.locator('input:checked')).to_have_count(0)。
        expect(multi.locator('input:checked')).to_have_count(0)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, errors。
        assert not errors, errors
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not external, external。
        assert not external, external
        # 【逐行批注】 执行当前 Python 语句：browser.close()。
        browser.close()
    # 【逐行批注】 输出当前运行结果：print('底部/题卡/Enter、单选/多选、仅选项作答、提交清理、刷新及纯追问隔离回归通过；真实模型请求0。')。
    print('底部/题卡/Enter、单选/多选、仅选项作答、提交清理、刷新及纯追问隔离回归通过；真实模型请求0。')


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()

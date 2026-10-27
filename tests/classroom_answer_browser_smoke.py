"""用生产课堂脚本验证底部和题卡提交一致、选项单独提交以及自由交流不夹带答案。"""
from playwright.sync_api import expect, sync_playwright
from section_navigation_browser_smoke import local_pages


def main():
    with local_pages() as base, sync_playwright() as playwright:
        browser = playwright.chromium.launch(headless=True)
        page = browser.new_page(viewport={'width': 1440, 'height': 900})
        errors, external = [], []
        page.on('pageerror', lambda error: errors.append(str(error)))

        def local_only(route):
            if route.request.url.startswith(base + '/'):
                route.continue_()
            else:
                external.append(route.request.url)
                route.abort()

        page.route('**/*', local_only)
        page.goto(base + '/practice?lessonId=navigation-fixture&sectionId=question-0&answer-test=1')
        field = page.locator('#lesson-input')
        expect(field).to_be_enabled()
        card = page.locator('[data-section-id="question-0"]')

        def captured(count):
            page.wait_for_function('count=>fixtureRequests.submit.length===count', arg=count)
            return page.evaluate('structuredClone(fixtureRequests.submit.at(-1))')

        reason = '我是根据分数单位来判断的。'
        for index, mode in enumerate(['composer', 'card', 'enter'], 1):
            card.locator('input[type=radio]').last.check()
            field.fill(reason)
            if mode == 'composer':
                page.locator('#lesson-send').click()
            elif mode == 'card':
                card.get_by_role('button', name='提交答案', exact=True).click()
            else:
                field.press('Enter')
            answer = captured(index)
            assert answer['type'] == 'question_answer' and answer['action'] == 'answer'
            assert answer['text'] == '我选2：直接相加\n' + reason
            expect(field).to_have_value('')
            expect(card.locator('input:checked')).to_have_count(0)
        # 只选选项也能使用底部发送，不能要求额外写一段文字。
        card.locator('input[type=radio]').first.check()
        page.locator('#lesson-send').click()
        assert captured(4)['text'] == '我选1：先统一单位\n'
        expect(card.locator('input:checked')).to_have_count(0)
        # 明确切到自由交流后，仍保留原选项草稿，纯追问不会变成一份选项答案。
        card.locator('input[type=radio]').last.check()
        page.locator('#composer-chat').click()
        field.fill('为什么要先统一分数单位？')
        page.locator('#lesson-send').click()
        question = captured(5)
        assert question['type'] == 'chat' and question['text'] == '为什么要先统一分数单位？'
        assert '我选' not in question['text']
        expect(card.locator('input[type=radio]').last).to_be_checked()
        # 多选同样携带全部选择；刷新后的选项不应重复提交已经清除的答案。
        page.goto(base + '/practice?lessonId=navigation-fixture&sectionId=question-1&answer-test=1')
        multi = page.locator('[data-section-id="question-1"]')
        expect(multi.locator('input[type=checkbox]')).to_have_count(2)
        for option in multi.locator('input[type=checkbox]').all():
            option.check()
        field.fill(reason)
        field.press('Enter')
        assert captured(1)['text'] == '我选1：先统一单位、2：直接相加\n' + reason
        expect(multi.locator('input:checked')).to_have_count(0)
        page.reload()
        expect(field).to_have_value('')
        expect(multi.locator('input:checked')).to_have_count(0)
        assert not errors, errors
        assert not external, external
        browser.close()
    print('底部/题卡/Enter、单选/多选、仅选项作答、提交清理、刷新及纯追问隔离回归通过；真实模型请求0。')


if __name__ == '__main__':
    main()

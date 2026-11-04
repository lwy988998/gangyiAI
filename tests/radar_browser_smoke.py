# 【逐行批注】 执行当前 Python 语句："""在虚构档案中验收双模式雷达的桌面、键盘、触屏和纯文本展示。"""。
"""在虚构档案中验收双模式雷达的桌面、键盘、触屏和纯文本展示。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from playwright.sync_api import expect, sync_playwright。
from playwright.sync_api import expect, sync_playwright
# 【逐行批注】 导入后续逻辑需要的模块或符号：from radar_smoke import Harness, IDS, NAMES, SUBJECTS。
from radar_smoke import Harness, IDS, NAMES, SUBJECTS

# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable, screenshots=None):
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-radar-browser-') as directory:。
    with tempfile.TemporaryDirectory(prefix='gangyi-radar-browser-') as directory:
        # 【逐行批注】 计算右侧结果并赋给 h，更新脚本状态。
        h = Harness(executable, directory, unified=True)
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 执行当前 Python 语句：h.settled()。
            h.settled()
            # 【逐行批注】 计算右侧结果并赋给 calls，更新脚本状态。
            calls = len(h.calls)
            # 【逐行批注】 在受控上下文中执行资源操作：with sync_playwright() as playwright:。
            with sync_playwright() as playwright:
                # 【逐行批注】 计算右侧结果并赋给 browser，更新脚本状态。
                browser = playwright.chromium.launch(headless=True)
                # 【逐行批注】 计算右侧结果并赋给 context，更新脚本状态。
                context = browser.new_context(viewport={'width': 1366, 'height': 1000}, reduced_motion='reduce')
                # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
                page = context.new_page()
                # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
                errors = []
                # 【逐行批注】 调用 str 完成当前脚本或测试步骤：page.on('pageerror', lambda error: errors.append(str(error)))。
                page.on('pageerror', lambda error: errors.append(str(error)))
                # 【逐行批注】 执行当前 Python 语句：page.goto(h.base)。
                page.goto(h.base)
                # 【逐行批注】 计算右侧结果并赋给 widget，更新脚本状态。
                widget = page.locator('#home-radar')
                # 【逐行批注】 执行当前 Python 语句：widget.locator('.radar-axis').last.wait_for()。
                widget.locator('.radar-axis').last.wait_for()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert widget.locator('.radar-axis').count() == 6。
                assert widget.locator('.radar-axis').count() == 6
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert widget.locator('.radar-axis strong').all_text_contents() == ['待评估'] * 6。
                assert widget.locator('.radar-axis strong').all_text_contents() == ['待评估'] * 6
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not widget.locator('.radar-area').is_visible()。
                assert not widget.locator('.radar-area').is_visible()

                # 【逐行批注】 定义函数 seed，封装可复用的处理步骤。
                def seed(subject_scores, ability_scores):
                    # 【逐行批注】 在受控上下文中执行资源操作：with h.db() as db:。
                    with h.db() as db:
                        # 【逐行批注】 执行当前 Python 语句：db.execute('DELETE FROM SubjectMastery')。
                        db.execute('DELETE FROM SubjectMastery')
                        # 【逐行批注】 遍历数据并重复执行循环体：for subject, score in zip(SUBJECTS, subject_scores):。
                        for subject, score in zip(SUBJECTS, subject_scores):
                            # 【逐行批注】 判断条件 score is None: continue，成立时执行对应分支。
                            if score is None: continue
                            # 【逐行批注】 调用 VALUES 完成当前脚本或测试步骤：db.execute('INSERT INTO SubjectMastery VALUES(?,?,?,?,?,?,?,?,?)',。
                            db.execute('INSERT INTO SubjectMastery VALUES(?,?,?,?,?,?,?,?,?)',
                                # 【逐行批注】 执行当前 Python 语句：(subject, score, '<b>这是虚构测试依据</b>', '[]', '先复习，再练习。', 3,。
                                (subject, score, '<b>这是虚构测试依据</b>', '[]', '先复习，再练习。', 3,
                                 # 【逐行批注】 执行当前 Python 语句：'fictional-browser-fixture', 'ready', '2026-10-03T00:00:00Z'))。
                                 'fictional-browser-fixture', 'ready', '2026-10-03T00:00:00Z'))
                        # 【逐行批注】 计算右侧结果并赋给 revision，更新脚本状态。
                        revision = int(db.execute("SELECT value FROM ProfileMeta WHERE key='revision'").fetchone()[0])
                        # 【逐行批注】 计算右侧结果并赋给 abilities，更新脚本状态。
                        abilities = [{'id': key, 'name': name, 'score': score, 'rationale': '虚构的开放任务表现',
                            # 【逐行批注】 调用 alert 完成当前脚本或测试步骤：'recommendation': '<img src=x onerror=alert(1)>先说明过程。',。
                            'recommendation': '<img src=x onerror=alert(1)>先说明过程。',
                            # 【逐行批注】 执行当前 Python 语句：'evidenceCount': 3 if score is not None else 0, 'updatedAt': '2026-10-03T00:00:00Z'}。
                            'evidenceCount': 3 if score is not None else 0, 'updatedAt': '2026-10-03T00:00:00Z'}
                            # 【逐行批注】 遍历数据并重复执行循环体：for key, name, score in zip(IDS, NAMES, ability_scores)]。
                            for key, name, score in zip(IDS, NAMES, ability_scores)]
                        # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                        payload = dict(dimensions=abilities, version=revision, attemptVersion=revision,
                            # 【逐行批注】 计算右侧结果并赋给 updatedAt，更新脚本状态。
                            updatedAt='2026-10-03T00:00:00Z', error='', model='fictional-browser-fixture')
                        # 【逐行批注】 调用 VALUES 完成当前脚本或测试步骤：db.execute('INSERT OR REPLACE INTO ProfileMeta VALUES(?,?)', ('abilit…。
                        db.execute('INSERT OR REPLACE INTO ProfileMeta VALUES(?,?)', ('ability-profile', json.dumps(payload)))

                # 【逐行批注】 调用 seed 完成当前脚本或测试步骤：seed([0, 57, None, None, None, None], [None] * 6)。
                seed([0, 57, None, None, None, None], [None] * 6)
                # 【逐行批注】 执行当前 Python 语句：page.reload()。
                page.reload()
                # 【逐行批注】 执行当前 Python 语句：page.wait_for_function("document.querySelector('#home-radar .radar-axis strong')?.textCon…。
                page.wait_for_function("document.querySelector('#home-radar .radar-axis strong')?.textContent === '入门起点'")
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert widget.locator('.radar-value-point').count() == 2。
                assert widget.locator('.radar-value-point').count() == 2
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not widget.locator('.radar-area').is_visible()。
                assert not widget.locator('.radar-area').is_visible()
                # 【逐行批注】 计算右侧结果并赋给 zero_axis，更新脚本状态。
                zero_axis = widget.get_by_role('button', name='语文，入门起点，查看依据和建议', exact=True)
                # 【逐行批注】 计算右侧结果并赋给 detail_heading，更新脚本状态。
                detail_heading = widget.locator('.radar-detail h4')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not widget.locator('.radar-detail').is_visible(), '首页评估依据默认折叠'。
                assert not widget.locator('.radar-detail').is_visible(), '首页评估依据默认折叠'
                # 【逐行批注】 执行当前 Python 语句：zero_axis.focus()。
                zero_axis.focus()
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(detail_heading).to_be_visible()。
                expect(detail_heading).to_be_visible()
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(detail_heading).to_have_text('语文 · 入门起点 · 0 分')。
                expect(detail_heading).to_have_text('语文 · 入门起点 · 0 分')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '<b>这是虚构测试依据</b>' in widget.locator('.radar-detail-reason').inner_text()。
                assert '<b>这是虚构测试依据</b>' in widget.locator('.radar-detail-reason').inner_text()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert widget.locator('.radar-detail b').count() == 0。
                assert widget.locator('.radar-detail b').count() == 0
                # 【逐行批注】 计算右侧结果并赋给 widget.get_by_role('button', name，更新脚本状态。
                widget.get_by_role('button', name='英语，待评估，查看依据和建议', exact=True).click()
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(detail_heading).to_have_text('英语 · 待评估')。
                expect(detail_heading).to_have_text('英语 · 待评估')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '0 分' not in detail_heading.inner_text(), '待评估不能冒充零分'。
                assert '0 分' not in detail_heading.inner_text(), '待评估不能冒充零分'
                # 【逐行批注】 执行当前 Python 语句：zero_axis.click()。
                zero_axis.click()
                # 【逐行批注】 调用 expect 完成当前脚本或测试步骤：expect(detail_heading).to_have_text('语文 · 入门起点 · 0 分')。
                expect(detail_heading).to_have_text('语文 · 入门起点 · 0 分')
                # 【逐行批注】 调用 seed 完成当前脚本或测试步骤：seed([32, 57, 81, 61, 44, 70], [72, 81, 66, 59, 52, 38])。
                seed([32, 57, 81, 61, 44, 70], [72, 81, 66, 59, 52, 38])
                # 【逐行批注】 执行当前 Python 语句：page.reload()。
                page.reload()
                # 【逐行批注】 计算右侧结果并赋给 widget.locator('.radar-area').wait_for(state，更新脚本状态。
                widget.locator('.radar-area').wait_for(state='visible')
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert widget.locator('.radar-value-point').count() == 6。
                assert widget.locator('.radar-value-point').count() == 6
                # 【逐行批注】 判断条件 screenshots:，成立时执行对应分支。
                if screenshots:
                    # 【逐行批注】 计算右侧结果并赋给 widget.screenshot(path，更新脚本状态。
                    widget.screenshot(path=str(Path(screenshots) / '学科六维-虚构验收.png'))
                # 【逐行批注】 计算右侧结果并赋给 widget.get_by_role('button', name，更新脚本状态。
                widget.get_by_role('button', name='学习能力', exact=True).click()
                # 【逐行批注】 执行当前 Python 语句：page.wait_for_function("document.querySelector('#home-radar .radar-axis-name')?.textConte…。
                page.wait_for_function("document.querySelector('#home-radar .radar-axis-name')?.textContent === '知识记忆'")
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert widget.locator('.radar-axis-name').all_text_contents() == NAMES。
                assert widget.locator('.radar-axis-name').all_text_contents() == NAMES
                # 【逐行批注】 执行当前 Python 语句：widget.locator('.radar-axis').nth(4).focus()。
                widget.locator('.radar-axis').nth(4).focus()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '全部课程' in widget.locator('.radar-detail-scope').inner_text()。
                assert '全部课程' in widget.locator('.radar-detail-scope').inner_text()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '<img src=x' in widget.locator('.radar-detail-advice').inner_text()。
                assert '<img src=x' in widget.locator('.radar-detail-advice').inner_text()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert widget.locator('.radar-detail img').count() == 0。
                assert widget.locator('.radar-detail img').count() == 0
                # 【逐行批注】 判断条件 screenshots:，成立时执行对应分支。
                if screenshots:
                    # 【逐行批注】 计算右侧结果并赋给 widget.screenshot(path，更新脚本状态。
                    widget.screenshot(path=str(Path(screenshots) / '学习能力六维-虚构验收.png'))
                # 【逐行批注】 执行当前 Python 语句：page.reload()。
                page.reload()
                # 【逐行批注】 执行当前 Python 语句：page.wait_for_function("document.querySelector('#home-radar .radar-axis-name')?.textConte…。
                page.wait_for_function("document.querySelector('#home-radar .radar-axis-name')?.textContent === '知识记忆'")
                # 【逐行批注】 计算右侧结果并赋给 widget.get_by_role('button', name，更新脚本状态。
                widget.get_by_role('button', name='学科画像', exact=True).click()
                # 【逐行批注】 计算右侧结果并赋给 widget.get_by_role('button', name，更新脚本状态。
                widget.get_by_role('button', name='选择学科', exact=True).click()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert widget.locator('.radar-subject-grid input').count() == 9。
                assert widget.locator('.radar-subject-grid input').count() == 9
                # 【逐行批注】 计算右侧结果并赋给 widget.get_by_label('生物', exact，更新脚本状态。
                widget.get_by_label('生物', exact=True).uncheck()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert widget.get_by_role('button', name='保存选择', exact=True).is_disabled()。
                assert widget.get_by_role('button', name='保存选择', exact=True).is_disabled()
                # 【逐行批注】 计算右侧结果并赋给 widget.get_by_label('历史', exact，更新脚本状态。
                widget.get_by_label('历史', exact=True).check()
                # 【逐行批注】 计算右侧结果并赋给 widget.get_by_role('button', name，更新脚本状态。
                widget.get_by_role('button', name='保存选择', exact=True).click()
                # 【逐行批注】 计算右侧结果并赋给 page.wait_for_function("[...document.querySe…，更新脚本状态。
                page.wait_for_function("[...document.querySelectorAll('#home-radar .radar-axis-name')].some(n => n.textContent === '历史')")
                # 【逐行批注】 执行当前 Python 语句：page.reload()。
                page.reload()
                # 【逐行批注】 计算右侧结果并赋给 page.wait_for_function("[...document.querySe…，更新脚本状态。
                page.wait_for_function("[...document.querySelectorAll('#home-radar .radar-axis-name')].some(n => n.textContent === '历史')")
                # 【逐行批注】 执行当前 Python 语句：page.goto(h.base + '/my-ai/reports')。
                page.goto(h.base + '/my-ai/reports')
                # 【逐行批注】 计算右侧结果并赋给 user_widget，更新脚本状态。
                user_widget = page.locator('#my-ai-radar')
                # 【逐行批注】 执行当前 Python 语句：user_widget.locator('.radar-axis').last.wait_for()。
                user_widget.locator('.radar-axis').last.wait_for()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert user_widget.locator('.radar-axis').count() == 6。
                assert user_widget.locator('.radar-axis').count() == 6
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '历史' in user_widget.locator('.radar-axis-name').all_text_contents()。
                assert '历史' in user_widget.locator('.radar-axis-name').all_text_contents()
                # 【逐行批注】 计算右侧结果并赋给 user_widget.get_by_role('button', name，更新脚本状态。
                user_widget.get_by_role('button', name='学习能力', exact=True).focus()
                # 【逐行批注】 执行当前 Python 语句：page.keyboard.press('Enter')。
                page.keyboard.press('Enter')
                # 【逐行批注】 执行当前 Python 语句：page.wait_for_function("document.querySelector('#my-ai-radar .radar-axis-name')?.textCont…。
                page.wait_for_function("document.querySelector('#my-ai-radar .radar-axis-name')?.textContent === '知识记忆'")
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(h.calls) == calls, '显示操作不得触发 AI 评估'。
                assert len(h.calls) == calls, '显示操作不得触发 AI 评估'
                # 【逐行批注】 计算右侧结果并赋给 touch，更新脚本状态。
                touch = browser.new_context(viewport={'width': 375, 'height': 812}, is_mobile=True,
                    # 【逐行批注】 计算右侧结果并赋给 has_touch，更新脚本状态。
                    has_touch=True, reduced_motion='reduce')
                # 【逐行批注】 计算右侧结果并赋给 mobile，更新脚本状态。
                mobile = touch.new_page()
                # 【逐行批注】 执行当前 Python 语句：mobile.goto(h.base)。
                mobile.goto(h.base)
                # 【逐行批注】 计算右侧结果并赋给 mobile_widget，更新脚本状态。
                mobile_widget = mobile.locator('#home-radar')
                # 【逐行批注】 执行当前 Python 语句：mobile_widget.locator('.radar-axis').last.wait_for()。
                mobile_widget.locator('.radar-axis').last.wait_for()
                # 【逐行批注】 执行当前 Python 语句：mobile_widget.locator('.radar-axis').last.tap()。
                mobile_widget.locator('.radar-axis').last.tap()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '综合迁移' in mobile_widget.locator('.radar-detail h4').inner_text()。
                assert '综合迁移' in mobile_widget.locator('.radar-detail h4').inner_text()
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert mobile.evaluate('document.documentElement.scrollWidth <= innerWidth + 1'…。
                assert mobile.evaluate('document.documentElement.scrollWidth <= innerWidth + 1'), '小屏幕不得横向溢出'
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert mobile_widget.locator('.radar-axis').last.bounding_box()['height'] >= 44。
                assert mobile_widget.locator('.radar-axis').last.bounding_box()['height'] >= 44
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not errors, errors。
                assert not errors, errors
                # 【逐行批注】 执行当前 Python 语句：touch.close(); context.close(); browser.close()。
                touch.close(); context.close(); browser.close()
            # 【逐行批注】 输出当前运行结果：print('六维雷达页面：空画像、部分/完整六维、零分、九科选择、键盘/触屏、纯文本与双页复用通过')。
            print('六维雷达页面：空画像、部分/完整六维、零分、九科选择、键盘/触屏、纯文本与双页复用通过')
        # 【逐行批注】 执行当前 Python 语句：finally: h.close()。
        finally: h.close()

# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encod…，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 计算右侧结果并赋给 screenshots，更新脚本状态。
    screenshots = Path(sys.argv[2]) if len(sys.argv) > 2 else None
    # 【逐行批注】 判断条件 screenshots: screenshots.mkdir(parents=True, exist_ok=True)，成立时执行对应分支。
    if screenshots: screenshots.mkdir(parents=True, exist_ok=True)
    # 【逐行批注】 调用 main、Path 完成当前脚本或测试步骤：main(Path(sys.argv[1]), screenshots)。
    main(Path(sys.argv[1]), screenshots)

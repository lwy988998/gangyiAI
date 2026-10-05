"""在虚构档案中验收双模式雷达的桌面、键盘、触屏和纯文本展示。"""
import json
import sys
import tempfile
from pathlib import Path
from playwright.sync_api import sync_playwright
from radar_smoke import Harness, IDS, NAMES, SUBJECTS

def main(executable, screenshots=None):
    with tempfile.TemporaryDirectory(prefix='gangyi-radar-browser-') as directory:
        h = Harness(executable, directory, unified=True)
        try:
            h.settled()
            calls = len(h.calls)
            with sync_playwright() as playwright:
                browser = playwright.chromium.launch(headless=True)
                context = browser.new_context(viewport={'width': 1366, 'height': 1000}, reduced_motion='reduce')
                page = context.new_page()
                errors = []
                page.on('pageerror', lambda error: errors.append(str(error)))
                page.goto(h.base)
                widget = page.locator('#home-radar')
                widget.locator('.radar-axis').last.wait_for()
                assert widget.locator('.radar-axis').count() == 6
                assert widget.locator('.radar-axis strong').all_text_contents() == ['待评估'] * 6
                assert not widget.locator('.radar-area').is_visible()

                def seed(subject_scores, ability_scores):
                    with h.db() as db:
                        db.execute('DELETE FROM SubjectMastery')
                        for subject, score in zip(SUBJECTS, subject_scores):
                            if score is None: continue
                            db.execute('INSERT INTO SubjectMastery VALUES(?,?,?,?,?,?,?,?,?)',
                                (subject, score, '<b>这是虚构测试依据</b>', '[]', '先复习，再练习。', 3,
                                 'fictional-browser-fixture', 'ready', '2026-10-03T00:00:00Z'))
                        revision = int(db.execute("SELECT value FROM ProfileMeta WHERE key='revision'").fetchone()[0])
                        abilities = [{'id': key, 'name': name, 'score': score, 'rationale': '虚构的开放任务表现',
                            'recommendation': '<img src=x onerror=alert(1)>先说明过程。',
                            'evidenceCount': 3 if score is not None else 0, 'updatedAt': '2026-10-03T00:00:00Z'}
                            for key, name, score in zip(IDS, NAMES, ability_scores)]
                        payload = dict(dimensions=abilities, version=revision, attemptVersion=revision,
                            updatedAt='2026-10-03T00:00:00Z', error='', model='fictional-browser-fixture')
                        db.execute('INSERT OR REPLACE INTO ProfileMeta VALUES(?,?)', ('ability-profile', json.dumps(payload)))

                seed([0, 57, None, None, None, None], [None] * 6)
                page.reload()
                page.wait_for_function("document.querySelector('#home-radar .radar-axis strong')?.textContent === '入门起点'")
                assert widget.locator('.radar-value-point').count() == 2
                assert not widget.locator('.radar-area').is_visible()
                widget.locator('.radar-axis').first.focus()
                assert '入门起点 · 0 分' in widget.locator('.radar-detail h4').inner_text()
                assert '<b>这是虚构测试依据</b>' in widget.locator('.radar-detail-reason').inner_text()
                assert widget.locator('.radar-detail b').count() == 0
                seed([32, 57, 81, 61, 44, 70], [72, 81, 66, 59, 52, 38])
                page.reload()
                widget.locator('.radar-area').wait_for(state='visible')
                assert widget.locator('.radar-value-point').count() == 6
                if screenshots:
                    widget.screenshot(path=str(Path(screenshots) / '学科六维-虚构验收.png'))
                widget.get_by_role('button', name='学习能力', exact=True).click()
                page.wait_for_function("document.querySelector('#home-radar .radar-axis-name')?.textContent === '知识记忆'")
                assert widget.locator('.radar-axis-name').all_text_contents() == NAMES
                widget.locator('.radar-axis').nth(4).focus()
                assert '全部课程' in widget.locator('.radar-detail-scope').inner_text()
                assert '<img src=x' in widget.locator('.radar-detail-advice').inner_text()
                assert widget.locator('.radar-detail img').count() == 0
                if screenshots:
                    widget.screenshot(path=str(Path(screenshots) / '学习能力六维-虚构验收.png'))
                page.reload()
                page.wait_for_function("document.querySelector('#home-radar .radar-axis-name')?.textContent === '知识记忆'")
                widget.get_by_role('button', name='学科画像', exact=True).click()
                widget.get_by_role('button', name='选择学科', exact=True).click()
                assert widget.locator('.radar-subject-grid input').count() == 9
                widget.get_by_label('生物', exact=True).uncheck()
                assert widget.get_by_role('button', name='保存选择', exact=True).is_disabled()
                widget.get_by_label('历史', exact=True).check()
                widget.get_by_role('button', name='保存选择', exact=True).click()
                page.wait_for_function("[...document.querySelectorAll('#home-radar .radar-axis-name')].some(n => n.textContent === '历史')")
                page.reload()
                page.wait_for_function("[...document.querySelectorAll('#home-radar .radar-axis-name')].some(n => n.textContent === '历史')")
                page.goto(h.base + '/my-courses')
                user_widget = page.locator('#uc-profile-radar')
                user_widget.locator('.radar-axis').last.wait_for()
                assert user_widget.locator('.radar-axis').count() == 6
                assert '历史' in user_widget.locator('.radar-axis-name').all_text_contents()
                user_widget.get_by_role('button', name='学习能力', exact=True).focus()
                page.keyboard.press('Enter')
                page.wait_for_function("document.querySelector('#uc-profile-radar .radar-axis-name')?.textContent === '知识记忆'")
                assert len(h.calls) == calls, '显示操作不得触发 AI 评估'
                touch = browser.new_context(viewport={'width': 375, 'height': 812}, is_mobile=True,
                    has_touch=True, reduced_motion='reduce')
                mobile = touch.new_page()
                mobile.goto(h.base)
                mobile_widget = mobile.locator('#home-radar')
                mobile_widget.locator('.radar-axis').last.wait_for()
                mobile_widget.locator('.radar-axis').last.tap()
                assert '综合迁移' in mobile_widget.locator('.radar-detail h4').inner_text()
                assert mobile.evaluate('document.documentElement.scrollWidth <= innerWidth + 1'), '小屏幕不得横向溢出'
                assert mobile_widget.locator('.radar-axis').last.bounding_box()['height'] >= 44
                assert not errors, errors
                touch.close(); context.close(); browser.close()
            print('六维雷达页面：空画像、部分/完整六维、零分、九科选择、键盘/触屏、纯文本与双页复用通过')
        finally: h.close()

if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encoding='utf-8')
    screenshots = Path(sys.argv[2]) if len(sys.argv) > 2 else None
    if screenshots: screenshots.mkdir(parents=True, exist_ok=True)
    main(Path(sys.argv[1]), screenshots)

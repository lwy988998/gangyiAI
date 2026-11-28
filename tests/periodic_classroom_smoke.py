"""通过实际本机课堂核对元素选择、键盘、资料来源、恢复与服务端事实回传。"""

from hashlib import sha256
import json
from pathlib import Path
import sys
import time
from urllib.parse import urlencode

from playwright.sync_api import sync_playwright

import classroom_v560_fixture as fixture
from deep_course_integration_smoke import DeepFixture


def verify_visuals(page, output):
    lab = page.locator('[data-lab-type=periodic_lab]')
    results = []
    for theme in ('light', 'dark'):
        if page.evaluate('document.documentElement.dataset.workspaceTheme') != theme:
            page.get_by_role('button', name='深色主题', exact=True).click()
        for width in (1280, 390):
            page.set_viewport_size({'width': width, 'height': 900})
            page.wait_for_timeout(100)
            assert page.evaluate('document.documentElement.scrollWidth<=innerWidth+1')
            assert lab.locator('.il-element-button[tabindex="0"]').count() == 1
            assert lab.locator('.il-element-button').count() == 118
            if width == 390:
                assert lab.locator('.il-periodic-scroll').evaluate('node=>node.scrollWidth>node.clientWidth')
                lab.locator('.il-periodic-scroll').evaluate('node=>node.scrollLeft=node.scrollWidth')
                assert lab.locator('.il-periodic-scroll').evaluate('node=>node.scrollLeft>0')
            contrast = lab.evaluate("""lab=>{
              const rgb=value=>value.match(/[\\d.]+/g).slice(0,3).map(Number);
              const luminance=value=>rgb(value).map(v=>{v/=255;return v<=.04045?v/12.92:((v+.055)/1.055)**2.4;}).reduce((sum,v,i)=>sum+v*[.2126,.7152,.0722][i],0);
              const ratio=(a,b)=>{a=luminance(a);b=luminance(b);return (Math.max(a,b)+.05)/(Math.min(a,b)+.05);};
              const background=getComputedStyle(lab.closest('.labs-workspace')).backgroundColor;
              const button=getComputedStyle(lab.querySelector('.il-element-button.is-selected'));
              return {note:ratio(getComputedStyle(lab.querySelector('.il-note')).color,background),
                table:ratio(getComputedStyle(lab.querySelector('td')).color,background),selected:ratio(button.color,button.backgroundColor)};
            }""")
            assert all(value >= 4.5 for value in contrast.values()), (theme, contrast)
            lab.screenshot(path=str(output / f'periodic-{theme}-{width}.png'))
            results.append({'theme': theme, 'width': width, 'contrast': contrast})
    return results


def main():
    executable, directory = sys.argv[1:3]
    output = Path(directory).resolve(); output.mkdir(parents=True, exist_ok=False)
    DeepFixture.include_labs = True; DeepFixture.periodic_lab = True; fixture.FixtureAI = DeepFixture
    errors = []
    with fixture.start_session(executable) as (info, api, _), sync_playwright() as runner:
        def wait_task(task_id):
            deadline = time.monotonic() + 60
            while time.monotonic() < deadline:
                value = api('/api/learning-agent?taskId=' + task_id)
                if value['status'] not in ('pending', 'running'):
                    assert value['status'] in ('ready', 'waiting_student'), value.get('error')
                    return value
                time.sleep(.08)
            raise TimeoutError(task_id)

        course = wait_task(api('/api/generate-plan', {'goal': '元素周期表与原子组成', 'mode': 'deep',
                                                      'requestId': 'periodic-plan'})['id'])['course']['id']
        task = api('/api/deep-course/start', {'courseId': course, 'phaseIndex': 1, 'topicIndex': 1,
                                              'requestId': 'periodic-lesson'})['task']['id']
        lesson_id = wait_task(task)['lesson']['id']; query = urlencode({'lessonId': lesson_id}); path = '/api/deep-course?' + query
        original = api(path); browser = fixture.launch_test_browser(runner)
        context = browser.new_context(viewport={'width': 1280, 'height': 900}, reduced_motion='reduce')
        page = context.new_page(); page.on('pageerror', lambda error: errors.append(str(error)))
        ready = "document.getElementById('deep-course-app').dataset.ready === 'true'"
        page.goto(info['base'] + '/deep-course?' + query); page.wait_for_function(ready)
        lab = page.locator('[data-media-id=periodic-lab]').first
        assert 'Na（Sodium）' in lab.locator('.il-lab-result').inner_text()
        assert lab.locator('.il-element-comparison').get_by_text('未知：需指定同位素', exact=True).count() == 2
        calls = len(DeepFixture.calls)
        selected = lab.get_by_role('combobox', name='当前元素 A', exact=True)
        comparison = lab.get_by_role('combobox', name='比较元素 B', exact=True)
        highlight = lab.get_by_role('combobox', name='周期表高亮', exact=True)
        selected.select_option('19'); comparison.select_option('11'); highlight.select_option('group')
        assert lab.locator('.il-element-button.is-related').count() == 7
        potassium = lab.locator('[data-atomic-number="19"]'); potassium.focus(); potassium.press('ArrowRight')
        assert selected.input_value() == '20'
        lab.locator('[data-atomic-number="20"]').press('ArrowLeft')
        lab.locator('[data-atomic-number="19"]').press('ArrowUp')
        assert selected.input_value() == '11'
        selected.select_option('58'); comparison.select_option('71')
        assert '不作同族判断' in lab.locator('.il-lab-result').inner_text()
        assert lab.locator('.il-element-button.is-related').count() == 1
        lab.locator('summary').click()
        assert '仅覆盖前 20 号' in lab.locator('.il-atom-diagram').inner_text()
        selected.select_option('19'); comparison.select_option('11')
        assert lab.locator('.il-atom-diagram svg').get_attribute('aria-label').startswith('K 中性原子电子层分布 2、8、8、1')
        assert len(DeepFixture.calls) == calls, '选择和比较不应调用模型'
        with page.expect_response(lambda response: response.url.endswith('/api/learning-agent/events') and response.request.method == 'POST') as submitted:
            lab.get_by_role('button', name='请老师解释当前结果', exact=True).click()
        assert submitted.value.status == 202, submitted.value.text()
        wait_task(submitted.value.json()['id'])
        call = next(item for item in reversed(DeepFixture.calls) if isinstance(item, dict) and
                    item.get('event', {}).get('mediaState', {}).get('periodic-lab'))
        observed = call['event']['mediaState']['periodic-lab']
        assert observed['selected']['symbol'] == 'K' and observed['selected']['protons'] == 19 and observed['selected']['electrons'] == 19
        assert observed['selected']['neutralShells'] == [2, 8, 8, 1] and observed['selected']['neutrons'] is None
        assert observed['comparison']['symbol'] == 'Na' and observed['sameGroup'] is True and observed['samePeriod'] is False
        assert observed['resultSource'] == 'program_calculation' and observed['scoringEligible'] is False
        assert observed['sourceUrl'] == 'https://pubchem.ncbi.nlm.nih.gov/rest/pug/periodictable/JSON'
        trace = next(item['traceRead'] for item in reversed(DeepFixture.calls) if isinstance(item, dict) and item.get('traceRead', {}).get('componentId') == 'periodic-lab')
        assert trace['events'] and trace['events'][0]['observation']['scoringEligible'] is False
        page.wait_for_function("""async path=>{
          const data=await (await fetch(path)).json();return data.componentDrafts?.['periodic-lab']?.state?.selectedZ===19;
        }""", arg=path)
        assert api(path)['lesson']['evaluations'] == original['lesson']['evaluations']
        context.close(); context = browser.new_context(viewport={'width': 1280, 'height': 900})
        page = context.new_page(); page.on('pageerror', lambda error: errors.append(str(error)))
        page.add_init_script('window.initialStorageSize=localStorage.length')
        page.goto(info['base'] + '/deep-course?' + query); page.wait_for_function(ready)
        assert page.evaluate('window.initialStorageSize') == 0
        lab = page.locator('[data-media-id=periodic-lab]').first
        assert lab.get_by_role('combobox', name='当前元素 A', exact=True).input_value() == '19'
        assert lab.get_by_role('combobox', name='比较元素 B', exact=True).input_value() == '11'
        assert lab.get_by_role('combobox', name='周期表高亮', exact=True).input_value() == 'group'
        lab.get_by_role('button', name='重置探索', exact=True).click()
        assert lab.get_by_role('combobox', name='当前元素 A', exact=True).input_value() == '11'
        assert api(path)['lesson']['evaluations'] == original['lesson']['evaluations']

        page.goto(info['base'] + '/interactive-labs.html'); page.locator('[data-lab=periodic]').click()
        visual_checks = verify_visuals(page, output)
        page.evaluate("""()=>{
          const host=document.createElement('div');document.body.append(host);window.periodicChanges=0;
          const instance=GangyiInteractiveElements.mount(host,{type:'periodic_lab'},{contentVersion:1,onChange(){periodicChanges++;}});
          const input=host.querySelector('select');instance.destroy();input.value='19';input.dispatchEvent(new Event('change'));
        }""")
        assert page.evaluate('periodicChanges') == 0
        assert not errors, errors
        context.close(); browser.close()
    root = Path(__file__).resolve().parents[1]
    report = {'passed': True, 'mockAIOnly': True, 'paidModelRequests': 0, 'keyboard': True,
              'seriesGroupUnknown': True, 'serverFactsVerified': True, 'traceToolRead': True,
              'newBrowserRestored': True, 'reset': True, 'destroyStopsCallbacks': True,
              'formalEvaluationsUnchanged': True, 'visualChecks': visual_checks, 'pageErrors': errors,
              'artifactHashes': {name: sha256((root / name).read_bytes()).hexdigest() for name in (
                  'include/interactive_elements.hpp', 'public/periodic-elements-data.js', 'public/interactive-elements.js', 'public/interactive-elements.css')},
              'executableSha256': sha256(Path(executable).read_bytes()).hexdigest()}
    (output / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(report, ensure_ascii=False))


if __name__ == '__main__':
    main()

"""用隔离模型验证波动播放、暂停保存、服务端复算和原课堂恢复。"""

import json
import math
from hashlib import sha256
from pathlib import Path
import sys
import time
from urllib.parse import urlencode

from playwright.sync_api import sync_playwright

import classroom_v560_fixture as fixture
from deep_course_integration_smoke import DeepFixture


def verify_visuals(page, output):
    """读取实际渲染颜色，防止深色实验标签退回浅色页面的默认值。"""
    lab = page.locator('[data-lab-type=wave_lab]')
    results = []
    for theme in ('light', 'dark'):
        if page.evaluate('document.documentElement.dataset.workspaceTheme') != theme:
            page.get_by_role('button', name='深色主题', exact=True).click()
        for width, height in [(1280, 900), (390, 844)]:
            page.set_viewport_size({'width': width, 'height': height})
            page.wait_for_timeout(100)
            assert page.evaluate('document.documentElement.scrollWidth<=innerWidth+1')
            contrast = lab.evaluate("""lab=>{
              const rgb=value=>value.match(/[\\d.]+/g).slice(0,3).map(Number);
              const luminance=value=>rgb(value).map(v=>{v/=255;return v<=.04045?v/12.92:((v+.055)/1.055)**2.4;}).reduce((sum,v,i)=>sum+v*[.2126,.7152,.0722][i],0);
              const ratio=(a,b)=>{a=luminance(a);b=luminance(b);return (Math.max(a,b)+.05)/(Math.min(a,b)+.05);};
              const background=getComputedStyle(lab.closest('.labs-workspace')).backgroundColor;
              return {note:ratio(getComputedStyle(lab.querySelector('.il-note')).color,background),
                axis:ratio(getComputedStyle(lab.querySelector('svg text')).fill,background),
                button:ratio(getComputedStyle(lab.querySelector('button')).color,getComputedStyle(lab.querySelector('button')).backgroundColor)};
            }""")
            assert all(value >= 4.5 for value in contrast.values()), (theme, contrast)
            lab.screenshot(path=str(output / f'wave-{theme}-{width}.png'))
            results.append({'theme': theme, 'width': width, 'contrast': contrast})
    return results


def main():
    executable, destination = sys.argv[1:3]
    output = Path(destination).resolve()
    output.mkdir(parents=True, exist_ok=False)
    DeepFixture.include_labs = True
    DeepFixture.wave_lab = True
    fixture.FixtureAI = DeepFixture
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

        course = wait_task(api('/api/generate-plan', {'goal': '区分波形传播与质点振动', 'mode': 'deep',
                                                      'requestId': 'wave-plan'})['id'])['course']['id']
        task = api('/api/deep-course/start', {'courseId': course, 'phaseIndex': 1, 'topicIndex': 1,
                                              'requestId': 'wave-lesson'})['task']['id']
        lesson_id = wait_task(task)['lesson']['id']
        query = urlencode({'lessonId': lesson_id})
        path = '/api/deep-course?' + query
        original = api(path)
        browser = fixture.launch_test_browser(runner)
        context = browser.new_context(viewport={'width': 1280, 'height': 900}, reduced_motion='reduce')
        page = context.new_page()
        page.on('pageerror', lambda error: errors.append(str(error)))
        ready = "document.getElementById('deep-course-app').dataset.ready === 'true'"
        page.goto(info['base'] + '/deep-course?' + query)
        page.wait_for_function(ready)
        lab = page.locator('[data-media-id=wave-lab]').first
        assert lab.get_by_role('button', name='播放', exact=True).get_attribute('aria-pressed') == 'false'
        assert 'λ=4 m' in lab.locator('.il-lab-result').inner_text()
        model_calls = len(DeepFixture.calls)
        curve = lab.locator('svg > path').nth(1)
        before = curve.get_attribute('d')
        particle = lab.locator('svg > circle')
        fixed_x = particle.get_attribute('cx')
        saves = []
        page.on('request', lambda request: saves.append(request.url)
                if '/component-draft' in request.url and request.method == 'POST' else None)
        lab.get_by_role('button', name='播放', exact=True).click()
        page.wait_for_timeout(650)
        assert lab.get_by_role('button', name='暂停', exact=True).is_visible()
        assert curve.get_attribute('d') != before and particle.get_attribute('cx') == fixed_x
        assert not saves, '每帧播放不应写入组件草稿'
        amplitude = lab.get_by_role('slider', name='振幅 A（m）', exact=True)
        amplitude.focus(); amplitude.press('ArrowRight')
        assert amplitude.input_value() == '1.5'
        assert lab.get_by_role('button', name='播放', exact=True).is_visible()
        period = lab.get_by_role('slider', name='周期 T（s）', exact=True)
        period.focus(); period.press('ArrowRight')
        lab.get_by_role('slider', name='时刻 t（s）', exact=True).evaluate(
            "node=>{node.value='.5';node.dispatchEvent(new Event('input',{bubbles:true}));}")
        assert 'λ=5 m' in lab.locator('.il-lab-result').inner_text()
        assert len(DeepFixture.calls) == model_calls, '参数探索不得调用模型'

        with page.expect_response(lambda response: response.url.endswith('/api/learning-agent/events') and
                                  response.request.method == 'POST') as submitted:
            lab.get_by_role('button', name='请老师解释当前结果', exact=True).click()
        assert submitted.value.status == 202, submitted.value.text()
        wait_task(submitted.value.json()['id'])
        call = next(item for item in reversed(DeepFixture.calls) if isinstance(item, dict) and
                    item.get('event', {}).get('mediaState', {}).get('wave-lab'))
        observed = call['event']['mediaState']['wave-lab']
        assert observed['wavelengthM'] == 5 and observed['frequencyHz'] == .4
        assert observed['timeSeconds'] == .5 and observed['amplitudeM'] == 1.5
        assert observed['particleFixedXM'] == 1 and abs(observed['probeDisplacementM']) < 1e-10
        assert abs(observed['particleVelocityMps'] + 1.2 * math.pi) < 1e-10
        assert observed['resultSource'] == 'program_calculation' and observed['scoringEligible'] is False
        trace = next(item['traceRead'] for item in reversed(DeepFixture.calls)
                     if isinstance(item, dict) and item.get('traceRead', {}).get('componentId') == 'wave-lab')
        assert trace['events'] and trace['events'][0]['observation']['scoringEligible'] is False
        assert api(path)['lesson']['evaluations'] == original['lesson']['evaluations']
        page.wait_for_function("""async path=>{
          const value=await (await fetch(path)).json();return value.componentDrafts?.['wave-lab']?.state?.periodSeconds===2.5;
        }""", arg=path)
        context.close()
        context = browser.new_context(viewport={'width': 1280, 'height': 900})
        page = context.new_page(); page.on('pageerror', lambda error: errors.append(str(error)))
        page.add_init_script('window.initialStorageSize=localStorage.length')
        page.goto(info['base'] + '/deep-course?' + query); page.wait_for_function(ready)
        assert page.evaluate('window.initialStorageSize') == 0
        lab = page.locator('[data-media-id=wave-lab]').first
        assert lab.get_by_role('slider', name='周期 T（s）', exact=True).input_value() == '2.5'
        assert lab.get_by_role('slider', name='时刻 t（s）', exact=True).input_value() == '0.5'
        assert lab.get_by_role('button', name='播放', exact=True).get_attribute('aria-pressed') == 'false'
        lab.get_by_role('button', name='重置探索', exact=True).click()
        assert lab.get_by_role('slider', name='时刻 t（s）', exact=True).input_value() == '0'
        assert 'λ=4 m' in lab.locator('.il-lab-result').inner_text()
        assert api(path)['lesson']['evaluations'] == original['lesson']['evaluations']

        # 用生产离线入口单独检查窄屏完整显示和播放资源释放。
        page.goto(info['base'] + '/interactive-labs.html')
        page.locator('[data-lab=wave]').click()
        lab = page.locator('[data-lab-type=wave_lab]')
        visual_checks = verify_visuals(page, output)
        lab.get_by_role('slider', name='振幅 A（m）', exact=True).evaluate(
            "node=>{node.value='0';node.dispatchEvent(new Event('input',{bubbles:true}));}")
        assert '不能从图像辨认波长' in lab.locator('.il-lab-result').inner_text()
        page.evaluate("""()=>{
          const nativeRequest=window.requestAnimationFrame,nativeCancel=window.cancelAnimationFrame;
          const pending=new Set();window.waveLifetime={changes:0,pending,restore(){window.requestAnimationFrame=nativeRequest;window.cancelAnimationFrame=nativeCancel;}};
          window.requestAnimationFrame=callback=>{const id=nativeRequest(stamp=>{pending.delete(id);callback(stamp);});pending.add(id);return id;};
          window.cancelAnimationFrame=id=>{pending.delete(id);nativeCancel(id);};
          const host=document.createElement('div');document.body.append(host);
          window.waveLifetime.host=host;window.waveLifetime.instance=GangyiInteractiveWaves.mount(host,{type:'wave_lab',timeSeconds:7.75},{contentVersion:1,onChange(){waveLifetime.changes++;}});
          host.querySelector('button').click();
        }""")
        page.wait_for_function("waveLifetime.instance.getState().timeSeconds===8 && waveLifetime.changes===1")
        assert page.evaluate('waveLifetime.pending.size') == 0
        page.evaluate("""()=>{
          const host=waveLifetime.host, time=host.querySelector('[aria-label="时刻 t（s）"]');
          time.value='0';time.dispatchEvent(new Event('input',{bubbles:true}));host.querySelector('button').click();
          waveLifetime.instance.destroy();waveLifetime.before=waveLifetime.changes;
        }""")
        page.wait_for_timeout(180)
        assert page.evaluate('waveLifetime.pending.size===0 && waveLifetime.before===waveLifetime.changes')
        page.evaluate('waveLifetime.restore()')
        assert not errors, errors
        context.close(); browser.close()
    report = {'passed': True, 'mockAIOnly': True, 'paidModelRequests': 0, 'playPause': True,
              'noPerFrameSaves': True, 'fixedParticle': True, 'keyboardWhilePlaying': True,
              'serverRecalculated': True, 'traceToolRead': True, 'newBrowserRestoredPaused': True,
              'reset': True, 'zeroAmplitude': True, 'playbackEndAndDestroy': True,
              'formalEvaluationsUnchanged': True, 'viewports': [1280, 390], 'pageErrors': errors}
    report['visualChecks'] = visual_checks
    root = Path(__file__).resolve().parents[1]
    report['artifactHashes'] = {name: sha256((root / name).read_bytes()).hexdigest() for name in (
        'include/interactive_waves.hpp', 'public/interactive-waves.js', 'public/interactive-labs.js')}
    report['executableSha256'] = sha256(Path(executable).read_bytes()).hexdigest()
    (output / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(report, ensure_ascii=False))


if __name__ == '__main__':
    main()

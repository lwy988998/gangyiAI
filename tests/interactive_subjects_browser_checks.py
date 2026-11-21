"""六科交互的边界验证，只使用浏览器中的教学示例。"""


def verify_subjects(page):
    host = page.locator('#lab-host')
    page.locator('[data-lab=cell]').click()
    host.get_by_role('button', name='叶绿体', exact=True).click()
    host.get_by_role('combobox', name='细胞类型').select_option('animal')
    assert host.get_by_role('button', name='叶绿体', exact=True).count() == 0
    assert host.get_by_role('button', name='线粒体', exact=True).is_visible()
    assert '细胞核' in host.locator('.il-lab-result').inner_text()
    host.get_by_role('button', name='线粒体', exact=True).focus()
    page.keyboard.press('Enter')
    assert '植物细胞也有线粒体' in host.locator('.il-lab-result').inner_text()
    page.reload()
    page.locator('[data-lab=cell]').click()
    assert host.get_by_role('combobox', name='细胞类型').input_value() == 'animal'
    host.get_by_role('button', name='重置探索').click()
    assert host.get_by_role('combobox', name='细胞类型').input_value() == 'plant'
    host.get_by_role('button', name='细胞膜', exact=True).click()
    assert host.locator('svg [data-part=membrane]').evaluate("node=>getComputedStyle(node).fill") == 'none'
    host.get_by_role('button', name='重置探索').click()

    page.locator('[data-lab=history]').click()
    host.get_by_role('button', name='阿波罗11号发射提前', exact=True).focus()
    page.keyboard.press('Enter')
    assert page.evaluate("document.activeElement?.getAttribute('aria-label')") == '阿波罗11号发射提前'
    page.keyboard.press('Enter')
    assert '符合日期先后' in host.locator('.il-lab-result').inner_text()
    assert page.evaluate("document.activeElement?.textContent") == '阿波罗11号发射'
    host.get_by_role('button', name='阿波罗11号发射推后', exact=True).focus()
    page.keyboard.press('Enter')
    page.keyboard.press('Enter')
    assert page.evaluate("document.activeElement?.textContent") == '阿波罗11号发射'
    assert host.get_by_role('button', name='阿波罗11号发射推后', exact=True).is_disabled()
    host.get_by_role('button', name='重置探索').click()

    page.locator('[data-lab=climate]').click()
    control = host.get_by_role('combobox', name='观察月份')
    control.focus()
    control.press('ArrowDown')
    control.press('Enter')
    assert control.input_value() == '2'
    host.get_by_role('combobox', name='比较月份').select_option('2')
    assert '气温差 0 °C，降水差 0 mm' in host.locator('.il-lab-result').inner_text()
    host.get_by_role('button', name='重置探索').click()

    for kind in ['annotation', 'sentence']:
        page.locator(f'[data-lab={kind}]').click()
        choice = host.get_by_role('combobox').first
        choice.select_option('1')
        assert host.locator('.il-marked-passage button').first.get_attribute('data-mark') == '1'
        host.locator('.il-marked-passage button').first.focus()
        page.keyboard.press('Enter')
        assert choice.input_value() == '2'
        page.reload()
        # 页面刷新先展示函数，重新选择目标后必须读回同一标注。
        page.locator(f'[data-lab={kind}]').click()
        assert host.get_by_role('combobox').first.input_value() == '2'
        host.get_by_role('button', name='重置探索').click()
        assert host.get_by_role('combobox').first.input_value() == '0'

    page.locator('[data-lab=algorithm]').click()
    for _ in range(6):
        host.get_by_role('button', name='下一步', exact=True).click()
    assert '[1, 2, 4, 5]' in host.locator('.il-lab-result').inner_text()
    assert host.get_by_role('button', name='下一步', exact=True).is_disabled()
    host.get_by_role('button', name='上一步', exact=True).click()
    assert '第 5 / 6 步' in host.locator('.il-lab-result').inner_text()
    host.get_by_role('button', name='重置探索').click()
    assert host.get_by_role('button', name='上一步', exact=True).is_disabled()

    # 受信注册器也必须把原文中的标记当文字；销毁后保留的旧按钮不得再触发事件。
    result = page.evaluate("""() => {
      const holder = document.createElement('div'); document.body.append(holder);
      let changes=0;
      const instance=GangyiInteractive.mount(holder,{type:'annotation_lab',fragments:['<img src=x onerror=alert(1)>','原文']},
        {contentVersion:1,onChange(){changes++;}});
      const button=holder.querySelector('.il-marked-passage button');button.click();
      const literal=button.textContent,images=holder.querySelectorAll('img').length;
      instance.destroy();button.click();holder.remove();
      return {literal,images,changes};
    }""")
    assert result == {'literal': '<img src=x onerror=alert(1)>', 'images': 0, 'changes': 1}
    page.evaluate("localStorage.setItem('gangyi-lab-demo:cell', JSON.stringify({schemaVersion:1,contentVersion:1,cellType:'animal',selectedPart:'chloroplast',score:100}))")
    page.locator('[data-lab=cell]').click()
    assert '上次探索与当前内容不符' in host.inner_text()
    host.get_by_role('button', name='重置探索').click()

/* 主题和本课上下文只使用页面现有数据，切换外观不会触发教学任务。 */
(() => {
  'use strict';
  const themeKey = 'gy-workspace-theme';
  const root = document.documentElement;
  let themeButton;
  let learningToolbar;
  let initial;
  try { initial = localStorage.getItem(themeKey); } catch (_) {}
  const applyTheme = value => {
    const dark = value === 'dark';
    root.dataset.workspaceTheme = dark ? 'dark' : 'light';
    if (themeButton) {
      themeButton.textContent = dark ? '浅色' : '深色';
      themeButton.setAttribute('aria-pressed', String(dark));
    }
    const color = document.querySelector('meta[name=theme-color]');
    if (color) color.content = dark ? '#141d2c' : '#f6f8fc';
  };
  applyTheme(initial);
  document.addEventListener('DOMContentLoaded', () => {
    const header = document.querySelector('.dark-nav-shell, .my-ai-top, .labs-header');
    if (!header) return;
    themeButton = document.createElement('button');
    themeButton.type = 'button';
    themeButton.className = 'gy-theme-toggle';
    themeButton.setAttribute('aria-label', '深色主题');
    themeButton.title = '切换课堂外观';
    themeButton.addEventListener('click', () => {
      const next = root.dataset.workspaceTheme === 'dark' ? 'light' : 'dark';
      applyTheme(next);
      try { localStorage.setItem(themeKey, next); } catch (_) {}
    });
    const mobile = header.querySelector('.dark-mobile-nav');
    if (learningToolbar) learningToolbar.settings.append(themeButton);
    else if (mobile) mobile.before(themeButton);
    else header.append(themeButton);
    applyTheme(root.dataset.workspaceTheme);
  });
  window.addEventListener('storage', event => {
    if (event.key === themeKey) applyTheme(event.newValue);
  });

  // 复用当前页面的链接与操作节点，保留原有路由、状态和事件监听。
  function compactToolbar(title) {
    if (learningToolbar) return learningToolbar;
    const shell = document.querySelector('.site-header .dark-nav-shell');
    if (!shell || !title) return null;
    const panel = document.createElement('div');
    panel.className = 'gy-learning-menu';
    panel.id = 'learning-workspace-menu';
    panel.setAttribute('popover', 'auto');
    panel.setAttribute('aria-label', '学习工作台操作');
    const toggle = document.createElement('button');
    toggle.type = 'button';
    toggle.className = 'gy-learning-menu-toggle';
    toggle.textContent = '操作菜单';
    toggle.setAttribute('popovertarget', panel.id);
    toggle.setAttribute('aria-expanded', 'false');
    panel.addEventListener('toggle', () => {
      toggle.setAttribute('aria-expanded', String(panel.matches(':popover-open')));
    });
    const navigation = document.createElement('nav');
    navigation.className = 'gy-learning-menu-links';
    navigation.setAttribute('aria-label', '全站导航');
    const brand = shell.querySelector('.dark-brand');
    if (brand) {
      brand.setAttribute('aria-label', '首页');
      navigation.append(brand);
    }
    const paths = new Set(brand ? ['/'] : []);
    for (const link of shell.querySelectorAll('.dark-nav-links a, .dark-mobile-nav a, .dark-nav-cta')) {
      const path = link.getAttribute('href') || (link.hasAttribute('data-current-course-link') && '/current-course');
      if (!path || paths.has(path)) continue;
      paths.add(path);
      navigation.append(link);
    }
    if (!paths.has('/exam-explanations')) {
      const link = document.createElement('a');
      link.href = '/exam-explanations';
      link.textContent = '试卷讲解';
      navigation.append(link);
    }
    const site = document.createElement('details');
    const siteTitle = document.createElement('summary');
    siteTitle.textContent = '全站导航与设置';
    const settings = document.createElement('div');
    settings.className = 'gy-learning-menu-settings';
    const activity = document.getElementById('learning-navigation');
    if (activity) settings.append(activity);
    if (themeButton) settings.append(themeButton);
    site.append(siteTitle, navigation, settings);
    panel.append(site);
    title.classList.add('gy-learning-title');
    const labelTitle = () => { title.title = title.textContent; };
    labelTitle();
    new MutationObserver(labelTitle).observe(title, {childList: true, characterData: true, subtree: true});
    shell.replaceChildren(title, toggle, panel);
    document.body.classList.add('gy-compact-workspace');
    const close = () => {
      if (panel.matches(':popover-open')) panel.hidePopover();
    };
    panel.addEventListener('click', event => {
      const action = event.target.closest('a[href], button');
      if (action && !event.defaultPrevented && !action.closest('[data-menu-keep-open]')) close();
    });
    learningToolbar = {
      panel, toggle, settings, close,
      append(...nodes) { site.before(...nodes.filter(Boolean)); },
      disclose(label, ...nodes) {
        const details = document.createElement('details');
        const summary = document.createElement('summary');
        summary.textContent = label;
        details.append(summary, ...nodes.filter(Boolean));
        return details;
      }
    };
    settings.dataset.menuKeepOpen = '';
    return learningToolbar;
  }

  function contextPanel(trigger, aside, movable, onToggle) {
    const small = window.matchMedia('(max-width:1000px)');
    const originals = movable.filter(Boolean).map(node => ({node, mark: document.createComment('课堂上下文原位置')}));
    const titleId = aside.id + '-title';
    const title = document.createElement('h2');
    title.id = titleId;
    title.textContent = '本课上下文';
    const closeButton = document.createElement('button');
    closeButton.className = 'gy-context-close';
    closeButton.type = 'button';
    closeButton.textContent = '收起';
    closeButton.setAttribute('aria-label', '收起本课上下文');
    const header = document.createElement('div');
    header.className = 'gy-context-head';
    header.append(title, closeButton);
    const links = document.createElement('nav');
    links.setAttribute('aria-label', '本课相关入口');
    const resource = document.createElement('a');
    resource.textContent = '本课资料';
    resource.href = '/my-ai/library';
    const plan = document.createElement('a');
    plan.textContent = '学习计划';
    plan.href = '/my-ai/plan';
    links.append(resource, plan);
    const information = document.createElement('div');
    information.className = 'gy-context-information';
    const empty = document.createElement('p');
    empty.className = 'gy-context-empty';
    empty.textContent = '暂时没有可展示的教学证据。';
    information.append(empty);
    const content = document.createElement('div');
    content.id = aside.id + '-content';
    content.className = 'gy-context-content';
    content.append(header, information, links);
    aside.classList.add('gy-context-panel');
    aside.setAttribute('aria-labelledby', titleId);
    aside.append(content);
    const dialog = document.createElement('dialog');
    dialog.className = 'gy-context-panel gy-context-dialog';
    dialog.setAttribute('aria-labelledby', titleId);
    aside.after(dialog);
    aside.hidden = true;
    trigger.setAttribute('aria-controls', content.id);
    trigger.setAttribute('aria-expanded', 'false');
    let opened = false;
    const close = (restoreFocus = true) => {
      if (!opened) return;
      opened = false;
      if (dialog.open) dialog.close();
      aside.append(content);
      aside.hidden = true;
      for (const {node, mark} of originals) mark.replaceWith(node);
      trigger.setAttribute('aria-expanded', 'false');
      onToggle(false);
      if (restoreFocus) {
        const target = trigger.getClientRects().length ? trigger : learningToolbar?.toggle;
        target?.focus({preventScroll:true});
      }
    };
    const show = () => {
      learningToolbar?.close();
      opened = true;
      for (const {node, mark} of originals) {
        node.before(mark);
        information.append(node);
      }
      if (small.matches || root.classList.contains('gy-classroom-fullscreen')) {
        dialog.append(content);
        dialog.showModal();
      } else {
        aside.hidden = false;
      }
      trigger.setAttribute('aria-expanded', 'true');
      onToggle(true);
      closeButton.focus({preventScroll:true});
    };
    closeButton.addEventListener('click', () => close());
    trigger.addEventListener('click', () => { if (opened) close(); else show(); });
    dialog.addEventListener('close', () => { if (opened && !dialog.open) close(); });
    small.addEventListener('change', () => { if (opened) close(); });
    aside.addEventListener('keydown', event => { if (event.key === 'Escape') close(); });
    return {
      close,
      update(courseId) {
        resource.href = '/my-ai/library' + (courseId ? '?' + new URLSearchParams({courseId}) : '');
        empty.hidden = originals.some(({node}) => !node.hidden && node.textContent.trim());
      }
    };
  }
  window.GangyiWorkspace = {compactToolbar, contextPanel};
})();

/* 安卓独立档案与窗口适配；教学内容与接口继续由现有课堂管理。 */
(() => {
  'use strict';
  document.documentElement.classList.add('gangyi-android');
  const bridge = window.GangyiAndroid;
  if (bridge) {
    const saved = JSON.parse(bridge.snapshot());
    // 本机服务端口每次变化，草稿不能依赖临时 HTTP origin。
    const storage = {
      getItem(key) { key = String(key); return Object.prototype.hasOwnProperty.call(saved, key) ? saved[key] : null; },
      setItem(key, value) { key = String(key); value = String(value); bridge.put(key, value); saved[key] = value; },
      removeItem(key) { key = String(key); bridge.remove(key); delete saved[key]; },
      key(index) { return Object.keys(saved)[index] ?? null; },
      clear() { Object.keys(saved).forEach(key => this.removeItem(key)); },
      get length() { return Object.keys(saved).length; }
    };
    Object.defineProperty(window, 'localStorage', { value: storage });
  }
  let baselineHeight = 0, previousWidth = innerWidth;
  const viewport = () => {
    document.documentElement.classList.toggle('android-tablet', typeof bridge?.tablet === 'function' ? bridge.tablet() : innerWidth >= 900);
    const height = window.visualViewport?.height || innerHeight;
    if (Math.abs(innerWidth - previousWidth) > 30) baselineHeight = 0;
    previousWidth = innerWidth; baselineHeight = Math.max(baselineHeight, height);
    document.documentElement.style.setProperty('--android-height', `${Math.round(height)}px`);
    const typing = document.activeElement?.matches('input,textarea,[contenteditable=true]');
    const keyboard = typeof bridge?.keyboardActive === 'function' ? bridge.keyboardActive() : height < baselineHeight - 120;
    document.documentElement.classList.toggle('android-typing', Boolean(typing && keyboard));
    document.documentElement.classList.toggle('android-short', height < 570);
  };
  addEventListener('resize', viewport);
  window.visualViewport?.addEventListener('resize', viewport);
  addEventListener('focusin', viewport);
  addEventListener('focusout', () => setTimeout(viewport, 100));
  viewport();
  document.addEventListener('DOMContentLoaded', () => {
    const navigation = document.createElement('nav');
    navigation.className = 'android-navigation'; navigation.setAttribute('aria-label', '主导航');
    const routes = [['/', '首页'], ['/my-courses', '我的课程'], ['/current-course', '当前课程'], ['/ask', 'AI 导师']];
    const classroom = ['/learn', '/practice', '/summary', '/plan', '/phase'].includes(location.pathname);
    for (const [href, label] of routes) {
      const link = document.createElement('a'); link.href = href; link.textContent = label;
      if (location.pathname === href || (classroom && href === '/current-course')) link.setAttribute('aria-current', 'page');
      navigation.append(link);
    }
    document.body.append(navigation);
    if (bridge) {
      const settings = document.createElement('button'); settings.type = 'button'; settings.className = 'android-settings';
      settings.textContent = '设置'; settings.setAttribute('aria-label', '打开安卓 AI 设置');
      settings.addEventListener('click', () => bridge.settings());
      document.querySelector('.dark-nav-shell')?.append(settings);
    }
    // 清楚提示人工恢复；只读取现有记录，不在这里提交任务。
    fetch('/api/ai-activity').then(response => response.json()).then(view => {
      const interrupted = view.tasks?.filter(task => task.status === 'paused' && task.pauseReason === 'mobile_restart') || [];
      if (!interrupted.length) return;
      const notice = document.createElement('div'); notice.className = 'android-recovery';
      notice.append(document.createTextNode(`有 ${interrupted.length} 项上次未完成的任务，记录已恢复。`));
      const button = document.createElement('button'); button.type = 'button'; button.textContent = '查看并选择继续';
      button.onclick = () => { const panel = document.getElementById('learning-navigation'); if (panel) panel.open = true; };
      notice.append(button); document.querySelector('main')?.prepend(notice);
    }).catch(() => {});
    viewport();
  });
})();

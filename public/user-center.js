/* 用户中心：学习时间、学习画像、AI 设置与课程管理。 */
(() => {
  'use strict';
  const $ = id => document.getElementById(id);
  const NAMES = ['周一', '周二', '周三', '周四', '周五', '周六', '周日'];
  const DEFAULT_AVAILABILITY = [{ weekday: 1, minutes: 30 }, { weekday: 3, minutes: 30 }, { weekday: 5, minutes: 30 }];
  const AVAILABILITY_KEY = 'gangyi-week-availability';
  const PENDING_KEY = 'gangyi-week-pending';
  let studyVersion = 0, studyEntries = [], saveTimer = null, saving = false;
  const timeEditorId = 'time-' + (crypto.randomUUID ? crypto.randomUUID() : Date.now());
  function protectTimeDraft(active) {
    return fetch('/api/study-plan/draft', {method: 'POST', keepalive: true, headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({clientId: timeEditorId, version: studyVersion, active, availability: collectAvailability(), entries: studyEntries})}).catch(() => {});
  }

  function readAvailability() {
    try {
      const parsed = JSON.parse(localStorage.getItem(AVAILABILITY_KEY) || 'null');
      if (Array.isArray(parsed) && parsed.length) return parsed;
    } catch (_) {}
    return DEFAULT_AVAILABILITY;
  }

  function collectAvailability() {
    const grid = $('uc-availability-grid');
    if (!grid) return [];
    return [...grid.querySelectorAll('label')]
      .filter(label => label.querySelector('[type=checkbox]').checked)
      .map(label => ({
        weekday: Number(label.querySelector('[type=checkbox]').dataset.weekday),
        minutes: Math.max(1, Math.min(1440, Number(label.querySelector('[type=number]').value) || 30)),
      }));
  }

  function updateAvailabilitySummary() {
    const summary = $('uc-availability-summary');
    if (!summary) return;
    const items = collectAvailability();
    if (!items.length) { summary.textContent = '还没有设置学习时间，课程计划页会使用默认安排。'; return; }
    const total = items.reduce((sum, item) => sum + item.minutes, 0);
    summary.textContent = '每周 ' + items.length + ' 天 · 共 ' + total + ' 分钟 · ' +
      items.map(item => NAMES[item.weekday - 1]).join('、');
  }

  async function saveAvailability() {
    const items = collectAvailability();
    if (!items.length) { $('uc-availability-summary').textContent = '请至少选择一个学习日。'; return; }
    if (saving) { clearTimeout(saveTimer); saveTimer = setTimeout(saveAvailability, 600); return; }
    saving = true;
    try {
      const response = await fetch('/api/study-plan', {method: 'PUT', keepalive: true, headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({version: studyVersion, availability: items})});
      const result = await response.json();
      if (!response.ok) throw new Error(result.error || '保存失败');
      studyVersion = result.version;
      studyEntries = result.entries; protectTimeDraft(false);
      localStorage.removeItem(AVAILABILITY_KEY); localStorage.removeItem(PENDING_KEY);
      updateAvailabilitySummary(); $('uc-availability-summary').textContent += ' · 已保存为全部课程共享预算';
    } catch (error) {
      $('uc-availability-summary').textContent = error.message + '，你的输入已保留；重新修改可重试。';
      try { const current = await (await fetch('/api/study-plan')).json(); studyVersion = current.version; } catch (_) {}
    } finally { saving = false; }
  }

  async function setupAvailability() {
    const grid = $('uc-availability-grid');
    if (!grid) return;
    let saved = readAvailability();
    try {
      const response = await fetch('/api/study-plan'); if (!response.ok) throw new Error();
      const result = await response.json(); studyVersion = result.version; studyEntries = result.entries; saved = result.availability;
    } catch (_) {
      $('uc-availability-summary').textContent = '总学习时间读取失败，请刷新后设置。'; return;
    }
    grid.innerHTML = '';
    NAMES.forEach((name, index) => {
      const slot = saved.find(item => Number(item.weekday) === index + 1);
      const minutes = slot ? Math.max(1, Math.min(1440, Number(slot.minutes) || 30)) : 30;
      const label = document.createElement('label');
      label.innerHTML = '<input type="checkbox" data-weekday="' + (index + 1) + '"' + (slot ? ' checked' : '') +
        '><span>' + name + '</span><input type="number" min="1" max="1440" step="1" value="' + minutes +
        '" aria-label="' + name + '分钟数"><span>分钟</span>';
      grid.append(label);
    });
    grid.addEventListener('change', () => { protectTimeDraft(true); clearTimeout(saveTimer); saveTimer = setTimeout(saveAvailability, 600); });
    grid.addEventListener('input', updateAvailabilitySummary);
    updateAvailabilitySummary();
  }

  const scoreLabel = value => (value === null || value === undefined) ? '数据不足' : value === 0 ? '入门起点' : String(value) + ' 分';

  async function loadProfile() {
    const host = $('uc-profile-subjects');
    if (!host) return;
    try {
      const response = await fetch('/api/profile');
      if (!response.ok) throw new Error();
      const data = await response.json();
      const subjects = data.subjects || [];
      if (!subjects.length) {
        host.innerHTML = '<h3>学科画像</h3><p class="uc-empty">数据不足：完成一次测验或课堂互动后，这里会生成学科画像。</p>';
        return;
      }
      host.innerHTML = '<h3>学科画像</h3><div class="uc-profile-grid">' + subjects.map(subject => {
        const weak = (subject.weakPoints || []).slice(0, 3);
        return '<article class="uc-profile-item"><p class="uc-eyebrow">' + escape(subject.subject || '') + '</p>' +
          '<b>' + escape(scoreLabel(subject.score)) + '</b>' +
          '<p class="uc-note">' + escape((subject.rationale || '暂无说明').replace(/\b(?:q-[0-9a-f]{16}|[0-9a-f]{32})\b/gi, '').replace(/unknown=true/g, '明确反馈暂时不会').replace(/topicStates/g, '已评估学习记录')) + '</p>' +
          (weak.length ? '<p class="uc-note">薄弱点：' + escape(weak.join('、')) + '</p>' : '') +
          (subject.recommendation ? '<p class="uc-note">建议：' + escape(subject.recommendation) + '</p>' : '') +
          '<p class="uc-course-created">依据 ' + Number(subject.evidenceCount || 0) + ' 条记录</p></article>';
      }).join('') + '</div>';
    } catch (_) {
      host.innerHTML = '<h3>学科画像</h3><p class="uc-empty">学习画像暂时读取失败，稍后会自动更新。</p>';
    }
  }

  async function loadTopicMastery() {
    const host = $('uc-profile-topics');
    if (!host) return;
    const ids = [...document.querySelectorAll('.uc-course')].map(element => element.dataset.courseId).filter(Boolean);
    if (!ids.length) { host.innerHTML = '<h3>主题掌握度</h3><p class="uc-empty">还没有课程。</p>'; return; }
    const results = await Promise.all(ids.map(async courseId => {
      try {
        const response = await fetch('/api/topic-mastery?courseId=' + encodeURIComponent(courseId));
        if (!response.ok) return [];
        const data = await response.json();
        return (data.topics || []).map(topic => Object.assign({ courseId }, topic));
      } catch (_) { return []; }
    }));
    const topics = results.flat().filter(topic => topic.status !== 'legacy');
    if (!topics.length) {
      host.innerHTML = '<h3>主题掌握度</h3><p class="uc-empty">数据不足：完成课堂互动或测验后才会生成主题掌握度，不猜测分数。</p>';
      return;
    }
    host.innerHTML = '<h3>主题掌握度</h3><ul class="uc-topic-list">' + topics.map(topic => {
      const count = Number(topic.evidenceCount || 0);
      const detail = count >= 1 ? escape(scoreLabel(topic.score)) : '数据不足';
      const weak = (topic.weakPoints || []).slice(0, 2);
      return '<li><div><strong>' + escape(topic.topic || '') + '</strong><span>第 ' + Number(topic.phaseIndex || 0) +
        ' 阶段</span></div><div class="uc-topic-score">' + detail +
        (weak.length ? '<small>' + escape(weak.join('、')) + '</small>' : '') + '</div></li>';
    }).join('') + '</ul>';
  }

  function escape(value) {
    const node = document.createElement('span');
    node.textContent = String(value ?? '');
    return node.innerHTML;
  }

  function setupApiSettings() {
    const button = $('open-api-settings');
    const message = $('api-settings-message');
    if (!button) return;
    button.addEventListener('click', async () => {
      if (typeof window.gangyiOpenApiSettings !== 'function') {
        message.textContent = '请在钢一定制AI桌面应用托盘菜单中选择“设置”来配置 API 接口。';
        return;
      }
      button.disabled = true;
      message.textContent = '正在打开 API 接口设置…';
      try {
        await window.gangyiOpenApiSettings();
        message.textContent = '请在弹出的设置窗口中完成接口配置并保存。';
      } catch (error) {
        message.textContent = error?.message || '设置窗口打开失败，请从托盘菜单进入“设置”。';
      } finally {
        button.disabled = false;
      }
    });
  }

  function setupCourseActions() {
    const message = $('course-message');
    document.querySelectorAll('.delete-course').forEach(button => {
      button.addEventListener('click', async () => {
        if (!confirm('确定删除这门课程吗？学习进度和课程内容将一并删除。')) return;
        button.disabled = true;
        try {
          const response = await fetch('/api/my-courses/' + encodeURIComponent(button.dataset.courseId), { method: 'DELETE' });
          if (!response.ok) throw new Error('删除失败，请稍后重试。');
          location.reload();
        } catch (error) {
          button.disabled = false;
          if (message) message.textContent = error.message;
        }
      });
    });
  }

  function setupNav() {
    const links = [...document.querySelectorAll('.uc-nav a')];
    if (!links.length || !('IntersectionObserver' in window)) return;
    const sections = links.map(link => document.querySelector(link.getAttribute('href'))).filter(Boolean);
    const observer = new IntersectionObserver(entries => {
      entries.forEach(entry => {
        if (!entry.isIntersecting) return;
        links.forEach(link => link.classList.toggle('is-active', link.getAttribute('href') === '#' + entry.target.id));
      });
    }, { rootMargin: '-20% 0px -70% 0px' });
    sections.forEach(section => observer.observe(section));
  }

  setupAvailability();
  setupApiSettings();
  setupCourseActions();
  setupNav();
  loadProfile();
  loadTopicMastery();
  setInterval(() => { if (!document.hidden) loadProfile(); }, 15000);
})();

/* 用户中心：学习时间、学习画像、AI 设置与课程管理。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 执行当前前端表达式：if(location.hash === '#profile' || location.hash === '#uc-profile') {location.replace('/m…。
  if(location.hash === '#profile' || location.hash === '#uc-profile') {location.replace('/my-ai/reports');return;}
  // 【逐行批注】 执行当前前端表达式：if(location.hash === '#uc-time' || location.hash === '#uc-study-plan-anchor') {location.r…。
  if(location.hash === '#uc-time' || location.hash === '#uc-study-plan-anchor') {location.replace('/my-ai/plan');return;}
  // 【逐行批注】 声明前端变量 $，保存本行计算出的数据或对象。
  const $ = id => document.getElementById(id);
  // 【逐行批注】 声明前端变量 NAMES，保存本行计算出的数据或对象。
  const NAMES = ['周一', '周二', '周三', '周四', '周五', '周六', '周日'];
  // 【逐行批注】 声明前端变量 DEFAULT_AVAILABILITY，保存本行计算出的数据或对象。
  const DEFAULT_AVAILABILITY = [{ weekday: 1, minutes: 30 }, { weekday: 3, minutes: 30 }, { weekday: 5, minutes: 30 }];
  // 【逐行批注】 声明前端变量 AVAILABILITY_KEY，保存本行计算出的数据或对象。
  const AVAILABILITY_KEY = 'gangyi-week-availability';
  // 【逐行批注】 声明前端变量 PENDING_KEY，保存本行计算出的数据或对象。
  const PENDING_KEY = 'gangyi-week-pending';
  // 【逐行批注】 声明前端变量 studyVersion，保存本行计算出的数据或对象。
  let studyVersion = 0, studyEntries = [], saveTimer = null, saving = false;
  // 【逐行批注】 声明前端变量 timeEditorId，保存本行计算出的数据或对象。
  const timeEditorId = 'time-' + (crypto.randomUUID ? crypto.randomUUID() : Date.now());
  // 【逐行批注】 定义函数 protectTimeDraft，封装可复用的前端行为。
  function protectTimeDraft(active) {
    // 【逐行批注】 结束当前函数并返回结果：return fetch('/api/study-plan/draft', {method: 'POST', keepalive: tru…。
    return fetch('/api/study-plan/draft', {method: 'POST', keepalive: true, headers: {'Content-Type': 'application/json'},
      // 【逐行批注】 计算右侧结果并更新 body: JSON.stringify({clientId: timeEditorId…，改变页面或请求状态。
      body: JSON.stringify({clientId: timeEditorId, version: studyVersion, active, availability: collectAvailability(), entries: studyEntries})}).catch(() => {});
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 readAvailability，封装可复用的前端行为。
  function readAvailability() {
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 parsed，保存本行计算出的数据或对象。
      const parsed = JSON.parse(localStorage.getItem(AVAILABILITY_KEY) || 'null');
      // 【逐行批注】 判断条件 (Array.isArray(parsed) && parsed.length) return parsed;，成立时执行对应逻辑。
      if (Array.isArray(parsed) && parsed.length) return parsed;
    // 【逐行批注】 执行当前前端表达式：} catch (_) {}。
    } catch (_) {}
    // 【逐行批注】 结束当前函数并返回结果：return DEFAULT_AVAILABILITY;。
    return DEFAULT_AVAILABILITY;
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 collectAvailability，封装可复用的前端行为。
  function collectAvailability() {
    // 【逐行批注】 声明前端变量 grid，保存本行计算出的数据或对象。
    const grid = $('uc-availability-grid');
    // 【逐行批注】 判断条件 (!grid) return [];，成立时执行对应逻辑。
    if (!grid) return [];
    // 【逐行批注】 结束当前函数并返回结果：return [...grid.querySelectorAll('label')]。
    return [...grid.querySelectorAll('label')]
      // 【逐行批注】 计算右侧结果并更新 .filter(label，改变页面或请求状态。
      .filter(label => label.querySelector('[type=checkbox]').checked)
      // 【逐行批注】 计算右侧结果并更新 .map(label，改变页面或请求状态。
      .map(label => ({
        // 【逐行批注】 计算右侧结果并更新 weekday: Number(label.querySelector('[type，改变页面或请求状态。
        weekday: Number(label.querySelector('[type=checkbox]').dataset.weekday),
        // 【逐行批注】 计算右侧结果并更新 minutes: Math.max(1, Math.min(1440, Number(l…，改变页面或请求状态。
        minutes: Math.max(1, Math.min(1440, Number(label.querySelector('[type=number]').value) || 30)),
      // 【逐行批注】 执行当前前端表达式：}));。
      }));
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 updateAvailabilitySummary，封装可复用的前端行为。
  function updateAvailabilitySummary() {
    // 【逐行批注】 声明前端变量 summary，保存本行计算出的数据或对象。
    const summary = $('uc-availability-summary');
    // 【逐行批注】 判断条件 (!summary) return;，成立时执行对应逻辑。
    if (!summary) return;
    // 【逐行批注】 声明前端变量 items，保存本行计算出的数据或对象。
    const items = collectAvailability();
    // 【逐行批注】 判断条件 (!items.length) { summary.textContent = '还没有设置学习时间，课程计划页会使用默认安排。…，成立时执行对应逻辑。
    if (!items.length) { summary.textContent = '还没有设置学习时间，课程计划页会使用默认安排。'; return; }
    // 【逐行批注】 声明前端变量 total，保存本行计算出的数据或对象。
    const total = items.reduce((sum, item) => sum + item.minutes, 0);
    // 【逐行批注】 计算右侧结果并更新 summary.textContent，改变页面或请求状态。
    summary.textContent = '每周 ' + items.length + ' 天 · 共 ' + total + ' 分钟 · ' +
      // 【逐行批注】 计算右侧结果并更新 items.map(item，改变页面或请求状态。
      items.map(item => NAMES[item.weekday - 1]).join('、');
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 saveAvailability，封装可复用的前端行为。
  async function saveAvailability() {
    // 【逐行批注】 声明前端变量 items，保存本行计算出的数据或对象。
    const items = collectAvailability();
    // 【逐行批注】 判断条件 (!items.length) { $('uc-availability-summary').textContent = '请至…，成立时执行对应逻辑。
    if (!items.length) { $('uc-availability-summary').textContent = '请至少选择一个学习日。'; return; }
    // 【逐行批注】 判断条件 (saving) { clearTimeout(saveTimer); saveTimer = setTimeout(saveA…，成立时执行对应逻辑。
    if (saving) { clearTimeout(saveTimer); saveTimer = setTimeout(saveAvailability, 600); return; }
    // 【逐行批注】 计算右侧结果并更新 saving，改变页面或请求状态。
    saving = true;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch('/api/study-plan', {method: 'PUT', keepalive: true, headers: {'Content-Type': 'application/json'},
        // 【逐行批注】 执行当前前端表达式：body: JSON.stringify({version: studyVersion, availability: items})});。
        body: JSON.stringify({version: studyVersion, availability: items})});
      // 【逐行批注】 声明前端变量 result，保存本行计算出的数据或对象。
      const result = await response.json();
      // 【逐行批注】 判断条件 (!response.ok) throw new Error(result.error || '保存失败');，成立时执行对应逻辑。
      if (!response.ok) throw new Error(result.error || '保存失败');
      // 【逐行批注】 计算右侧结果并更新 studyVersion，改变页面或请求状态。
      studyVersion = result.version;
      // 【逐行批注】 计算右侧结果并更新 studyEntries，改变页面或请求状态。
      studyEntries = result.entries; protectTimeDraft(false);
      // 【逐行批注】 执行当前前端表达式：localStorage.removeItem(AVAILABILITY_KEY); localStorage.removeItem(PENDING_KEY);。
      localStorage.removeItem(AVAILABILITY_KEY); localStorage.removeItem(PENDING_KEY);
      // 【逐行批注】 计算右侧结果并更新 updateAvailabilitySummary(); $('uc-availabil…，改变页面或请求状态。
      updateAvailabilitySummary(); $('uc-availability-summary').textContent += ' · 已保存为全部课程共享预算';
    // 【逐行批注】 执行当前前端表达式：} catch (error) {。
    } catch (error) {
      // 【逐行批注】 计算右侧结果并更新 $('uc-availability-summary').textContent，改变页面或请求状态。
      $('uc-availability-summary').textContent = error.message + '，你的输入已保留；重新修改可重试。';
      // 【逐行批注】 计算右侧结果并更新 try { const current，改变页面或请求状态。
      try { const current = await (await fetch('/api/study-plan')).json(); studyVersion = current.version; } catch (_) {}
    // 【逐行批注】 计算右侧结果并更新 } finally { saving，改变页面或请求状态。
    } finally { saving = false; }
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 setupAvailability，封装可复用的前端行为。
  async function setupAvailability() {
    // 【逐行批注】 声明前端变量 grid，保存本行计算出的数据或对象。
    const grid = $('uc-availability-grid');
    // 【逐行批注】 判断条件 (!grid) return;，成立时执行对应逻辑。
    if (!grid) return;
    // 【逐行批注】 声明前端变量 saved，保存本行计算出的数据或对象。
    let saved = readAvailability();
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch('/api/study-plan'); if (!response.ok) throw new Error();
      // 【逐行批注】 声明前端变量 result，保存本行计算出的数据或对象。
      const result = await response.json(); studyVersion = result.version; studyEntries = result.entries; saved = result.availability;
    // 【逐行批注】 执行当前前端表达式：} catch (_) {。
    } catch (_) {
      // 【逐行批注】 计算右侧结果并更新 $('uc-availability-summary').textContent，改变页面或请求状态。
      $('uc-availability-summary').textContent = '总学习时间读取失败，请刷新后设置。'; return;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 grid.innerHTML，改变页面或请求状态。
    grid.innerHTML = '';
    // 【逐行批注】 计算右侧结果并更新 NAMES.forEach((name, index)，改变页面或请求状态。
    NAMES.forEach((name, index) => {
      // 【逐行批注】 声明前端变量 slot，保存本行计算出的数据或对象。
      const slot = saved.find(item => Number(item.weekday) === index + 1);
      // 【逐行批注】 声明前端变量 minutes，保存本行计算出的数据或对象。
      const minutes = slot ? Math.max(1, Math.min(1440, Number(slot.minutes) || 30)) : 30;
      // 【逐行批注】 声明前端变量 label，保存本行计算出的数据或对象。
      const label = document.createElement('label');
      // 【逐行批注】 计算右侧结果并更新 label.innerHTML，改变页面或请求状态。
      label.innerHTML = '<input type="checkbox" data-weekday="' + (index + 1) + '"' + (slot ? ' checked' : '') +
        // 【逐行批注】 执行当前前端表达式：'><span>' + name + '</span><input type="number" min="1" max="1440" step="1" value="' + mi…。
        '><span>' + name + '</span><input type="number" min="1" max="1440" step="1" value="' + minutes +
        // 【逐行批注】 计算右侧结果并更新 '" aria-label，改变页面或请求状态。
        '" aria-label="' + name + '分钟数"><span>分钟</span>';
      // 【逐行批注】 执行当前前端表达式：grid.append(label);。
      grid.append(label);
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 【逐行批注】 为页面事件绑定处理逻辑：grid.addEventListener('change', () => { protectTimeDraft(true); clearTimeout(saveTim…。
    grid.addEventListener('change', () => { protectTimeDraft(true); clearTimeout(saveTimer); saveTimer = setTimeout(saveAvailability, 600); });
    // 【逐行批注】 为页面事件绑定处理逻辑：grid.addEventListener('input', updateAvailabilitySummary);。
    grid.addEventListener('input', updateAvailabilitySummary);
    // 【逐行批注】 调用 updateAvailabilitySummary 执行页面交互或数据处理：updateAvailabilitySummary();。
    updateAvailabilitySummary();
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 声明前端变量 scoreLabel，保存本行计算出的数据或对象。
  const scoreLabel = value => (value === null || value === undefined) ? '数据不足' : value === 0 ? '入门起点' : String(value) + ' 分';

  // 【逐行批注】 定义函数 loadProfile，封装可复用的前端行为。
  async function loadProfile() {
    // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
    const host = $('uc-profile-subjects');
    // 【逐行批注】 判断条件 (!host) return;，成立时执行对应逻辑。
    if (!host) return;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch('/api/profile');
      // 【逐行批注】 判断条件 (!response.ok) throw new Error();，成立时执行对应逻辑。
      if (!response.ok) throw new Error();
      // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
      const data = await response.json();
      // 【逐行批注】 声明前端变量 subjects，保存本行计算出的数据或对象。
      const subjects = data.subjects || [];
      // 【逐行批注】 判断条件 (!subjects.length) {，成立时执行对应逻辑。
      if (!subjects.length) {
        // 【逐行批注】 计算右侧结果并更新 host.innerHTML，改变页面或请求状态。
        host.innerHTML = '<h3>学科画像</h3><p class="uc-empty">数据不足：完成一次测验或课堂互动后，这里会生成学科画像。</p>';
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 计算右侧结果并更新 host.innerHTML，改变页面或请求状态。
      host.innerHTML = '<h3>学科画像</h3><div class="uc-profile-grid">' + subjects.map(subject => {
        // 【逐行批注】 声明前端变量 weak，保存本行计算出的数据或对象。
        const weak = (subject.weakPoints || []).slice(0, 3);
        // 【逐行批注】 结束当前函数并返回结果：return '<article class="uc-profile-item"><p class="uc-eyebrow">' + es…。
        return '<article class="uc-profile-item"><p class="uc-eyebrow">' + escape(subject.subject || '') + '</p>' +
          // 【逐行批注】 调用 escape、scoreLabel 执行页面交互或数据处理：'<b>' + escape(scoreLabel(subject.score)) + '</b>' +。
          '<b>' + escape(scoreLabel(subject.score)) + '</b>' +
          // 【逐行批注】 调用 escape、b 执行页面交互或数据处理：'<p class="uc-note">' + escape((subject.rationale || '暂无说明').replace(…。
          '<p class="uc-note">' + escape((subject.rationale || '暂无说明').replace(/\b(?:q-[0-9a-f]{16}|[0-9a-f]{32})\b/gi, '').replace(/unknown=true/g, '明确反馈暂时不会').replace(/topicStates/g, '已评估学习记录')) + '</p>' +
          // 【逐行批注】 调用 escape 执行页面交互或数据处理：(weak.length ? '<p class="uc-note">薄弱点：' + escape(weak.join('、')) + '…。
          (weak.length ? '<p class="uc-note">薄弱点：' + escape(weak.join('、')) + '</p>' : '') +
          // 【逐行批注】 调用 escape 执行页面交互或数据处理：(subject.recommendation ? '<p class="uc-note">建议：' + escape(subject.r…。
          (subject.recommendation ? '<p class="uc-note">建议：' + escape(subject.recommendation) + '</p>' : '') +
          // 【逐行批注】 调用 Number 执行页面交互或数据处理：'<p class="uc-course-created">依据 ' + Number(subject.evidenceCount || …。
          '<p class="uc-course-created">依据 ' + Number(subject.evidenceCount || 0) + ' 条记录</p></article>';
      // 【逐行批注】 执行当前前端表达式：}).join('') + '</div>';。
      }).join('') + '</div>';
    // 【逐行批注】 执行当前前端表达式：} catch (_) {。
    } catch (_) {
      // 【逐行批注】 计算右侧结果并更新 host.innerHTML，改变页面或请求状态。
      host.innerHTML = '<h3>学科画像</h3><p class="uc-empty">学习画像暂时读取失败，稍后会自动更新。</p>';
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 loadTopicMastery，封装可复用的前端行为。
  async function loadTopicMastery() {
    // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
    const host = $('uc-profile-topics');
    // 【逐行批注】 判断条件 (!host) return;，成立时执行对应逻辑。
    if (!host) return;
    // 【逐行批注】 声明前端变量 ids，保存本行计算出的数据或对象。
    const ids = [...document.querySelectorAll('.uc-course')].map(element => element.dataset.courseId).filter(Boolean);
    // 【逐行批注】 判断条件 (!ids.length) { host.innerHTML = '<h3>主题掌握度</h3><p class="uc-emp…，成立时执行对应逻辑。
    if (!ids.length) { host.innerHTML = '<h3>主题掌握度</h3><p class="uc-empty">还没有课程。</p>'; return; }
    // 【逐行批注】 声明前端变量 results，保存本行计算出的数据或对象。
    const results = await Promise.all(ids.map(async courseId => {
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
        const response = await fetch('/api/topic-mastery?courseId=' + encodeURIComponent(courseId));
        // 【逐行批注】 判断条件 (!response.ok) return [];，成立时执行对应逻辑。
        if (!response.ok) return [];
        // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
        const data = await response.json();
        // 【逐行批注】 结束当前函数并返回结果：return (data.topics || []).map(topic => Object.assign({ courseId }, t…。
        return (data.topics || []).map(topic => Object.assign({ courseId }, topic));
      // 【逐行批注】 执行当前前端表达式：} catch (_) { return []; }。
      } catch (_) { return []; }
    // 【逐行批注】 执行当前前端表达式：}));。
    }));
    // 【逐行批注】 声明前端变量 topics，保存本行计算出的数据或对象。
    const topics = results.flat().filter(topic => topic.status !== 'legacy');
    // 【逐行批注】 判断条件 (!topics.length) {，成立时执行对应逻辑。
    if (!topics.length) {
      // 【逐行批注】 计算右侧结果并更新 host.innerHTML，改变页面或请求状态。
      host.innerHTML = '<h3>主题掌握度</h3><p class="uc-empty">数据不足：完成课堂互动或测验后才会生成主题掌握度，不猜测分数。</p>';
      // 【逐行批注】 结束当前函数并返回结果：return;。
      return;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 计算右侧结果并更新 host.innerHTML，改变页面或请求状态。
    host.innerHTML = '<h3>主题掌握度</h3><ul class="uc-topic-list">' + topics.map(topic => {
      // 【逐行批注】 声明前端变量 count，保存本行计算出的数据或对象。
      const count = Number(topic.evidenceCount || 0);
      // 【逐行批注】 声明前端变量 detail，保存本行计算出的数据或对象。
      const detail = count >= 1 ? escape(scoreLabel(topic.score)) : '数据不足';
      // 【逐行批注】 声明前端变量 weak，保存本行计算出的数据或对象。
      const weak = (topic.weakPoints || []).slice(0, 2);
      // 【逐行批注】 结束当前函数并返回结果：return '<li><div><strong>' + escape(topic.topic || '') + '</strong><s…。
      return '<li><div><strong>' + escape(topic.topic || '') + '</strong><span>第 ' + Number(topic.phaseIndex || 0) +
        // 【逐行批注】 执行当前前端表达式：' 阶段</span></div><div class="uc-topic-score">' + detail +。
        ' 阶段</span></div><div class="uc-topic-score">' + detail +
        // 【逐行批注】 调用 escape 执行页面交互或数据处理：(weak.length ? '<small>' + escape(weak.join('、')) + '</small>' : '') …。
        (weak.length ? '<small>' + escape(weak.join('、')) + '</small>' : '') + '</div></li>';
    // 【逐行批注】 执行当前前端表达式：}).join('') + '</ul>';。
    }).join('') + '</ul>';
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 escape，封装可复用的前端行为。
  function escape(value) {
    // 【逐行批注】 声明前端变量 node，保存本行计算出的数据或对象。
    const node = document.createElement('span');
    // 【逐行批注】 计算右侧结果并更新 node.textContent，改变页面或请求状态。
    node.textContent = String(value ?? '');
    // 【逐行批注】 结束当前函数并返回结果：return node.innerHTML;。
    return node.innerHTML;
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 setupApiSettings，封装可复用的前端行为。
  function setupApiSettings() {
    // 【逐行批注】 声明前端变量 button，保存本行计算出的数据或对象。
    const button = $('open-api-settings');
    // 【逐行批注】 声明前端变量 message，保存本行计算出的数据或对象。
    const message = $('api-settings-message');
    // 【逐行批注】 判断条件 (!button) return;，成立时执行对应逻辑。
    if (!button) return;
    // 【逐行批注】 为页面事件绑定处理逻辑：button.addEventListener('click', async () => {。
    button.addEventListener('click', async () => {
      // 【逐行批注】 判断条件 (typeof window.gangyiOpenApiSettings !== 'function') {，成立时执行对应逻辑。
      if (typeof window.gangyiOpenApiSettings !== 'function') {
        // 【逐行批注】 计算右侧结果并更新 message.textContent，改变页面或请求状态。
        message.textContent = '请在钢一定制AI桌面应用托盘菜单中选择“设置”来配置 API 接口。';
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 计算右侧结果并更新 button.disabled，改变页面或请求状态。
      button.disabled = true;
      // 【逐行批注】 计算右侧结果并更新 message.textContent，改变页面或请求状态。
      message.textContent = '正在打开 API 接口设置…';
      // 【逐行批注】 执行当前前端表达式：try {。
      try {
        // 【逐行批注】 执行当前前端表达式：await window.gangyiOpenApiSettings();。
        await window.gangyiOpenApiSettings();
        // 【逐行批注】 计算右侧结果并更新 message.textContent，改变页面或请求状态。
        message.textContent = '请在弹出的设置窗口中完成接口配置并保存。';
      // 【逐行批注】 执行当前前端表达式：} catch (error) {。
      } catch (error) {
        // 【逐行批注】 计算右侧结果并更新 message.textContent，改变页面或请求状态。
        message.textContent = error?.message || '设置窗口打开失败，请从托盘菜单进入“设置”。';
      // 【逐行批注】 执行当前前端表达式：} finally {。
      } finally {
        // 【逐行批注】 计算右侧结果并更新 button.disabled，改变页面或请求状态。
        button.disabled = false;
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 setupCourseActions，封装可复用的前端行为。
  function setupCourseActions() {
    // 【逐行批注】 声明前端变量 message，保存本行计算出的数据或对象。
    const message = $('course-message');
    // 【逐行批注】 计算右侧结果并更新 document.querySelectorAll('.delete-course').…，改变页面或请求状态。
    document.querySelectorAll('.delete-course').forEach(button => {
      // 【逐行批注】 为页面事件绑定处理逻辑：button.addEventListener('click', async () => {。
      button.addEventListener('click', async () => {
        // 【逐行批注】 判断条件 (!confirm('确定删除这门课程吗？学习进度和课程内容将一并删除。')) return;，成立时执行对应逻辑。
        if (!confirm('确定删除这门课程吗？学习进度和课程内容将一并删除。')) return;
        // 【逐行批注】 计算右侧结果并更新 button.disabled，改变页面或请求状态。
        button.disabled = true;
        // 【逐行批注】 执行当前前端表达式：try {。
        try {
          // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
          const response = await fetch('/api/my-courses/' + encodeURIComponent(button.dataset.courseId), { method: 'DELETE' });
          // 【逐行批注】 判断条件 (!response.ok) throw new Error('删除失败，请稍后重试。');，成立时执行对应逻辑。
          if (!response.ok) throw new Error('删除失败，请稍后重试。');
          // 【逐行批注】 执行当前前端表达式：location.reload();。
          location.reload();
        // 【逐行批注】 执行当前前端表达式：} catch (error) {。
        } catch (error) {
          // 【逐行批注】 计算右侧结果并更新 button.disabled，改变页面或请求状态。
          button.disabled = false;
          // 【逐行批注】 判断条件 (message) message.textContent = error.message;，成立时执行对应逻辑。
          if (message) message.textContent = error.message;
        // 【逐行批注】 结束当前前端代码块。
        }
      // 【逐行批注】 执行当前前端表达式：});。
      });
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 setupNav，封装可复用的前端行为。
  function setupNav() {
    // 【逐行批注】 声明前端变量 links，保存本行计算出的数据或对象。
    const links = [...document.querySelectorAll('.uc-nav a')];
    // 【逐行批注】 判断条件 (!links.length || !('IntersectionObserver' in window)) return;，成立时执行对应逻辑。
    if (!links.length || !('IntersectionObserver' in window)) return;
    // 【逐行批注】 声明前端变量 sections，保存本行计算出的数据或对象。
    const sections = links.map(link => document.querySelector(link.getAttribute('href'))).filter(Boolean);
    // 【逐行批注】 声明前端变量 observer，保存本行计算出的数据或对象。
    const observer = new IntersectionObserver(entries => {
      // 【逐行批注】 计算右侧结果并更新 entries.forEach(entry，改变页面或请求状态。
      entries.forEach(entry => {
        // 【逐行批注】 判断条件 (!entry.isIntersecting) return;，成立时执行对应逻辑。
        if (!entry.isIntersecting) return;
        // 【逐行批注】 计算右侧结果并更新 links.forEach(link，改变页面或请求状态。
        links.forEach(link => link.classList.toggle('is-active', link.getAttribute('href') === '#' + entry.target.id));
      // 【逐行批注】 执行当前前端表达式：});。
      });
    // 【逐行批注】 执行当前前端表达式：}, { rootMargin: '-20% 0px -70% 0px' });。
    }, { rootMargin: '-20% 0px -70% 0px' });
    // 【逐行批注】 计算右侧结果并更新 sections.forEach(section，改变页面或请求状态。
    sections.forEach(section => observer.observe(section));
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 调用 setupAvailability 执行页面交互或数据处理：setupAvailability();。
  setupAvailability();
  // 【逐行批注】 调用 setupApiSettings 执行页面交互或数据处理：setupApiSettings();。
  setupApiSettings();
  // 【逐行批注】 调用 setupCourseActions 执行页面交互或数据处理：setupCourseActions();。
  setupCourseActions();
  // 【逐行批注】 调用 setupNav 执行页面交互或数据处理：setupNav();。
  setupNav();
  // 【逐行批注】 调用 loadProfile 执行页面交互或数据处理：loadProfile();。
  loadProfile();
  // 【逐行批注】 调用 loadTopicMastery 执行页面交互或数据处理：loadTopicMastery();。
  loadTopicMastery();
  // 【逐行批注】 计算右侧结果并更新 setInterval(()，改变页面或请求状态。
  setInterval(() => { if (!document.hidden) loadProfile(); }, 15000);
// 【逐行批注】 执行当前前端表达式：})();。
})();

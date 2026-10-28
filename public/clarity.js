/* 展示层复用已保存的学习状态，不触发模型生成或修改教学记录。 */
(() => {
  'use strict';
  const body = document.body;
  if (!body.classList.contains('clarity-app')) return;
  const header = document.querySelector('.site-header, .my-ai-top');
  function measureHeader() {
    if (header) body.style.setProperty('--gy-header-height', `${header.getBoundingClientRect().height}px`);
  }
  if (header && 'ResizeObserver' in window) new ResizeObserver(measureHeader).observe(header);
  measureHeader();
  const activity = document.getElementById('learning-navigation');
  const navigationShell = header?.querySelector('.dark-nav-shell');
  if (activity && navigationShell) {
    const actions = document.createElement('div');
    actions.className = 'clarity-header-actions';
    actions.append(activity);
    const profileLink = navigationShell.querySelector('.dark-nav-cta');
    const mobileMenu = navigationShell.querySelector('.dark-mobile-nav');
    if (profileLink) actions.append(profileLink);
    if (mobileMenu) actions.append(mobileMenu);
    navigationShell.append(actions);
  }
  const recent = document.getElementById('recent-courses');
  const continuation = document.getElementById('clarity-continue-content');
  let lastCourseKey = '';
  function updateContinuation() {
    if (!recent || !continuation) return;
    const course = recent.querySelector('.recent-course-card');
    const key = course?.textContent || '';
    if (!course || key === lastCourseKey) return;
    lastCourseKey = key;
    const title = document.createElement('h3');
    title.textContent = course.querySelector('h3')?.textContent || '继续你的课程';
    const description = document.createElement('p');
    description.textContent = course.querySelector('.recent-course-topic')?.textContent || '从上次的学习位置继续。';
    const track = document.createElement('div');
    track.className = 'clarity-course-track';
    const fill = document.createElement('span');
    fill.style.width = course.querySelector('.recent-course-track>span')?.style.width || '0%';
    track.append(fill);
    continuation.replaceChildren(title, description, track);
    document.getElementById('clarity-continue-progress').textContent = course.querySelector('.recent-course-progress')?.textContent || '已保存学习进度';
    document.getElementById('clarity-continue-state').textContent = course.querySelector('.recent-course-state')?.textContent || '学习中';
    const href = course.getAttribute('href');
    if (href && href.startsWith('/') && !href.startsWith('//')) document.getElementById('clarity-continue-link').href = href;
  }
  if (recent) new MutationObserver(updateContinuation).observe(recent, {childList: true, subtree: true, characterData: true});
  updateContinuation();
  const home = document.querySelector('.home-page');
  if (home) {
    const compactRadar = document.getElementById('home-radar');
    const radarDetail = compactRadar?.querySelector('.radar-detail');
    if (radarDetail) {
      const disclosure = document.createElement('details');
      disclosure.className = 'clarity-radar-evidence';
      const summary = document.createElement('summary');
      summary.textContent = '查看画像依据与建议';
      radarDetail.replaceWith(disclosure);
      disclosure.append(summary, radarDetail);
      compactRadar.addEventListener('click', event => {
        if (event.target.closest('.radar-axis')) disclosure.open = true;
      });
    }
    function arrangeReviews() {
      const reviews = document.getElementById('due-reviews');
      if (!reviews) return;
      const summary = reviews.querySelector('summary');
      reviews.hidden = Boolean(summary && /待复习\s*[·・]\s*0\s*项/.test(summary.textContent));
      if (home.lastElementChild !== reviews) home.append(reviews);
    }
    new MutationObserver(arrangeReviews).observe(home, {childList: true, subtree: true, characterData: true});
    arrangeReviews();
  }
  const more = document.getElementById('classroom-more');
  const trigger = document.querySelector('[popovertarget=classroom-more]');
  if (more && trigger) {
    trigger.addEventListener('click', () => {
      const bounds = trigger.getBoundingClientRect();
      more.style.right = `${Math.max(12, innerWidth - bounds.right)}px`;
      more.style.bottom = `${Math.max(12, innerHeight - bounds.top + 8)}px`;
    });
    more.addEventListener('click', event => {
      if (event.target.closest('button') && more.matches(':popover-open')) more.hidePopover();
    });
  }
  document.addEventListener('keydown', event => {
    if (event.key !== 'Escape') return;
    for (const menu of document.querySelectorAll('.dark-mobile-nav[open]')) {
      menu.open = false;
      menu.querySelector('summary')?.focus();
    }
  });
  // 旧报表仍用随包 ECharts，仅统一文字与轴线，不改变数据口径。
  const themed = new WeakSet();
  let scheduled = false;
  function themeCharts() {
    scheduled = false;
    if (!window.echarts) return;
    for (const host of document.querySelectorAll('.ma-chart')) {
      const chart = window.echarts.getInstanceByDom(host);
      if (!chart || themed.has(chart)) continue;
      chart.setOption({
        color: ['#3d61ce', '#3e8b88', '#b98740', '#9a6bb3', '#60884e'],
        textStyle: {color: '#5e6d82', fontSize: 12},
        legend: {textStyle: {color: '#5e6d82', fontSize: 12}},
        xAxis: {axisLine: {lineStyle: {color: '#dfe6ef'}}, axisLabel: {color: '#5e6d82', fontSize: 12}},
        yAxis: {axisLabel: {color: '#5e6d82', fontSize: 12}, splitLine: {lineStyle: {color: '#e5ebf3'}}}
      });
      themed.add(chart);
    }
  }
  const content = document.getElementById('my-ai-content');
  if (content) new MutationObserver(() => {
    if (!scheduled) { scheduled = true; requestAnimationFrame(themeCharts); }
  }).observe(content, {childList: true, subtree: true});
  themeCharts();
})();

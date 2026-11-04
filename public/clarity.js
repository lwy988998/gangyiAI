/* 展示层复用已保存的学习状态，不触发模型生成或修改教学记录。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';
  // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
  const body = document.body;
  // 【逐行批注】 判断条件 (!body.classList.contains('clarity-app')) return;，成立时执行对应逻辑。
  if (!body.classList.contains('clarity-app')) return;
  // 【逐行批注】 声明前端变量 header，保存本行计算出的数据或对象。
  const header = document.querySelector('.site-header, .my-ai-top');
  // 顶部导航按实际页面归属标记；静态备课、窄屏和旧版页面也保持一致。
  // 【逐行批注】 声明前端变量 coursePages，保存本行计算出的数据或对象。
  const coursePages = new Set(['/current-course', '/plan', '/phase', '/learn', '/practice', '/summary',
    // 【逐行批注】 执行当前前端表达式：'/deep-course', '/agent-prepare.html', '/agent-classroom.html']);。
    '/deep-course', '/agent-prepare.html', '/agent-classroom.html']);
  // 【逐行批注】 声明前端变量 currentPage，保存本行计算出的数据或对象。
  const currentPage = location.pathname;
  // 【逐行批注】 声明前端变量 currentSection，保存本行计算出的数据或对象。
  const currentSection = coursePages.has(currentPage) ? '/current-course' :
    // 【逐行批注】 执行当前前端表达式：currentPage === '/' || currentPage === '/startup' ? '/' :。
    currentPage === '/' || currentPage === '/startup' ? '/' :
    // 【逐行批注】 执行当前前端表达式：currentPage === '/my-courses' ? '/my-courses' : currentPage === '/ask' ? '/ask' :。
    currentPage === '/my-courses' ? '/my-courses' : currentPage === '/ask' ? '/ask' :
    // 【逐行批注】 执行当前前端表达式：currentPage === '/my-ai' || currentPage.startsWith('/my-ai/') ? '/my-ai' : null;。
    currentPage === '/my-ai' || currentPage.startsWith('/my-ai/') ? '/my-ai' : null;
  // 【逐行批注】 声明前端变量 mainSections，保存本行计算出的数据或对象。
  const mainSections = new Set(['/', '/my-courses', '/current-course', '/ask', '/my-ai']);
  // 【逐行批注】 判断条件 (header && currentSection) {，成立时执行对应逻辑。
  if (header && currentSection) {
    // 【逐行批注】 按循环规则遍历或重复处理：for (const nav of header.querySelectorAll('nav')) {。
    for (const nav of header.querySelectorAll('nav')) {
      // 【逐行批注】 声明前端变量 links，保存本行计算出的数据或对象。
      const links = [...nav.querySelectorAll('a')].map(link => {
        // 【逐行批注】 判断条件 (link.hasAttribute('data-current-course-link')) return {link, se…，成立时执行对应逻辑。
        if (link.hasAttribute('data-current-course-link')) return {link, section: '/current-course'};
        // 【逐行批注】 声明前端变量 href，保存本行计算出的数据或对象。
        const href = link.getAttribute('href');
        // 【逐行批注】 结束当前函数并返回结果：return {link, section: mainSections.has(href) ? href : null};。
        return {link, section: mainSections.has(href) ? href : null};
      // 【逐行批注】 计算右侧结果并更新 }).filter(item，改变页面或请求状态。
      }).filter(item => item.section);
      // 【逐行批注】 判断条件 (!links.some(item => item.section === currentSection)) continue;，成立时执行对应逻辑。
      if (!links.some(item => item.section === currentSection)) continue;
      // 【逐行批注】 按循环规则遍历或重复处理：for (const {link, section} of links) {。
      for (const {link, section} of links) {
        // 【逐行批注】 声明前端变量 active，保存本行计算出的数据或对象。
        const active = section === currentSection;
        // 【逐行批注】 执行当前前端表达式：link.classList.toggle('is-active', active);。
        link.classList.toggle('is-active', active);
        // 【逐行批注】 判断条件 (active) link.setAttribute('aria-current', 'page');，成立时执行对应逻辑。
        if (active) link.setAttribute('aria-current', 'page');
        // 【逐行批注】 执行前一条件不成立时的分支。
        else link.removeAttribute('aria-current');
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 定义函数 measureHeader，封装可复用的前端行为。
  function measureHeader() {
    // 【逐行批注】 判断条件 (header) body.style.setProperty('--gy-header-height', `${header.…，成立时执行对应逻辑。
    if (header) body.style.setProperty('--gy-header-height', `${header.getBoundingClientRect().height}px`);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 判断条件 (header && 'ResizeObserver' in window) new ResizeObserver(measur…，成立时执行对应逻辑。
  if (header && 'ResizeObserver' in window) new ResizeObserver(measureHeader).observe(header);
  // 【逐行批注】 调用 measureHeader 执行页面交互或数据处理：measureHeader();。
  measureHeader();
  // 【逐行批注】 声明前端变量 activity，保存本行计算出的数据或对象。
  const activity = document.getElementById('learning-navigation');
  // 主站头部用 .dark-nav-shell 承载；「我的 AI」头部是 .my-ai-top（无 shell），
  // 必须同样把 AI 状态入口放进首屏头部，否则它会被静态定位推到 100dvh 之后。
  // 【逐行批注】 声明前端变量 navigationShell，保存本行计算出的数据或对象。
  const navigationShell = header?.querySelector('.dark-nav-shell');
  // 【逐行批注】 声明前端变量 headerSlot，保存本行计算出的数据或对象。
  const headerSlot = navigationShell || (header?.classList.contains('my-ai-top') ? header : null);
  // 【逐行批注】 判断条件 (activity && headerSlot) {，成立时执行对应逻辑。
  if (activity && headerSlot) {
    // 【逐行批注】 声明前端变量 actions，保存本行计算出的数据或对象。
    const actions = document.createElement('div');
    // 【逐行批注】 计算右侧结果并更新 actions.className，改变页面或请求状态。
    actions.className = 'clarity-header-actions';
    // 【逐行批注】 执行当前前端表达式：actions.append(activity);。
    actions.append(activity);
    // 【逐行批注】 判断条件 (navigationShell) {，成立时执行对应逻辑。
    if (navigationShell) {
      // 【逐行批注】 声明前端变量 profileLink，保存本行计算出的数据或对象。
      const profileLink = navigationShell.querySelector('.dark-nav-cta');
      // 【逐行批注】 声明前端变量 mobileMenu，保存本行计算出的数据或对象。
      const mobileMenu = navigationShell.querySelector('.dark-mobile-nav');
      // 【逐行批注】 判断条件 (profileLink) actions.append(profileLink);，成立时执行对应逻辑。
      if (profileLink) actions.append(profileLink);
      // 【逐行批注】 判断条件 (mobileMenu) actions.append(mobileMenu);，成立时执行对应逻辑。
      if (mobileMenu) actions.append(mobileMenu);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 执行当前前端表达式：headerSlot.append(actions);。
    headerSlot.append(actions);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 recent，保存本行计算出的数据或对象。
  const recent = document.getElementById('recent-courses');
  // 【逐行批注】 声明前端变量 continuation，保存本行计算出的数据或对象。
  const continuation = document.getElementById('clarity-continue-content');
  // 【逐行批注】 声明前端变量 lastCourseKey，保存本行计算出的数据或对象。
  let lastCourseKey = '';
  // 【逐行批注】 定义函数 updateContinuation，封装可复用的前端行为。
  function updateContinuation() {
    // 【逐行批注】 判断条件 (!recent || !continuation) return;，成立时执行对应逻辑。
    if (!recent || !continuation) return;
    // 【逐行批注】 声明前端变量 course，保存本行计算出的数据或对象。
    const course = recent.querySelector('.recent-course-card');
    // 【逐行批注】 声明前端变量 key，保存本行计算出的数据或对象。
    const key = course?.textContent || '';
    // 【逐行批注】 判断条件 (!course || key === lastCourseKey) return;，成立时执行对应逻辑。
    if (!course || key === lastCourseKey) return;
    // 【逐行批注】 计算右侧结果并更新 lastCourseKey，改变页面或请求状态。
    lastCourseKey = key;
    // 【逐行批注】 声明前端变量 title，保存本行计算出的数据或对象。
    const title = document.createElement('h3');
    // 【逐行批注】 计算右侧结果并更新 title.textContent，改变页面或请求状态。
    title.textContent = course.querySelector('h3')?.textContent || '继续你的课程';
    // 【逐行批注】 声明前端变量 description，保存本行计算出的数据或对象。
    const description = document.createElement('p');
    // 【逐行批注】 计算右侧结果并更新 description.textContent，改变页面或请求状态。
    description.textContent = course.querySelector('.recent-course-topic')?.textContent || '从上次的学习位置继续。';
    // 【逐行批注】 声明前端变量 track，保存本行计算出的数据或对象。
    const track = document.createElement('div');
    // 【逐行批注】 计算右侧结果并更新 track.className，改变页面或请求状态。
    track.className = 'clarity-course-track';
    // 【逐行批注】 声明前端变量 fill，保存本行计算出的数据或对象。
    const fill = document.createElement('span');
    // 【逐行批注】 计算右侧结果并更新 fill.style.width，改变页面或请求状态。
    fill.style.width = course.querySelector('.recent-course-track>span')?.style.width || '0%';
    // 【逐行批注】 执行当前前端表达式：track.append(fill);。
    track.append(fill);
    // 【逐行批注】 执行当前前端表达式：continuation.replaceChildren(title, description, track);。
    continuation.replaceChildren(title, description, track);
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('clarity-continue-pr…，改变页面或请求状态。
    document.getElementById('clarity-continue-progress').textContent = course.querySelector('.recent-course-progress')?.textContent || '已保存学习进度';
    // 【逐行批注】 计算右侧结果并更新 document.getElementById('clarity-continue-st…，改变页面或请求状态。
    document.getElementById('clarity-continue-state').textContent = course.querySelector('.recent-course-state')?.textContent || '学习中';
    // 【逐行批注】 声明前端变量 href，保存本行计算出的数据或对象。
    const href = course.getAttribute('href');
    // 【逐行批注】 判断条件 (href && href.startsWith('/') && !href.startsWith('//')) documen…，成立时执行对应逻辑。
    if (href && href.startsWith('/') && !href.startsWith('//')) document.getElementById('clarity-continue-link').href = href;
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 判断条件 (recent) new MutationObserver(updateContinuation).observe(recent…，成立时执行对应逻辑。
  if (recent) new MutationObserver(updateContinuation).observe(recent, {childList: true, subtree: true, characterData: true});
  // 【逐行批注】 调用 updateContinuation 执行页面交互或数据处理：updateContinuation();。
  updateContinuation();
  // 【逐行批注】 声明前端变量 home，保存本行计算出的数据或对象。
  const home = document.querySelector('.home-page');
  // 【逐行批注】 判断条件 (home) {，成立时执行对应逻辑。
  if (home) {
    // 【逐行批注】 声明前端变量 compactRadar，保存本行计算出的数据或对象。
    const compactRadar = document.getElementById('home-radar');
    // 【逐行批注】 声明前端变量 radarDetail，保存本行计算出的数据或对象。
    const radarDetail = compactRadar?.querySelector('.radar-detail');
    // 【逐行批注】 判断条件 (radarDetail) {，成立时执行对应逻辑。
    if (radarDetail) {
      // 【逐行批注】 声明前端变量 disclosure，保存本行计算出的数据或对象。
      const disclosure = document.createElement('details');
      // 【逐行批注】 计算右侧结果并更新 disclosure.className，改变页面或请求状态。
      disclosure.className = 'clarity-radar-evidence';
      // 【逐行批注】 声明前端变量 summary，保存本行计算出的数据或对象。
      const summary = document.createElement('summary');
      // 【逐行批注】 计算右侧结果并更新 summary.textContent，改变页面或请求状态。
      summary.textContent = '查看画像依据与建议';
      // 【逐行批注】 执行当前前端表达式：radarDetail.replaceWith(disclosure);。
      radarDetail.replaceWith(disclosure);
      // 【逐行批注】 执行当前前端表达式：disclosure.append(summary, radarDetail);。
      disclosure.append(summary, radarDetail);
      // 【逐行批注】 声明前端变量 revealEvidence，保存本行计算出的数据或对象。
      const revealEvidence = event => {
        // 【逐行批注】 判断条件 (event.target.closest('.radar-axis')) disclosure.open = true;，成立时执行对应逻辑。
        if (event.target.closest('.radar-axis')) disclosure.open = true;
      // 【逐行批注】 执行当前前端表达式：};。
      };
      // 【逐行批注】 为页面事件绑定处理逻辑：compactRadar.addEventListener('click', revealEvidence);。
      compactRadar.addEventListener('click', revealEvidence);
      // 维度本身支持键盘聚焦，折叠详情须同步展开才能读取当前真实评估。
      // 【逐行批注】 为页面事件绑定处理逻辑：compactRadar.addEventListener('focusin', revealEvidence);。
      compactRadar.addEventListener('focusin', revealEvidence);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 定义函数 arrangeReviews，封装可复用的前端行为。
    function arrangeReviews() {
      // 【逐行批注】 声明前端变量 reviews，保存本行计算出的数据或对象。
      const reviews = document.getElementById('due-reviews');
      // 【逐行批注】 判断条件 (!reviews) return;，成立时执行对应逻辑。
      if (!reviews) return;
      // 【逐行批注】 声明前端变量 summary，保存本行计算出的数据或对象。
      const summary = reviews.querySelector('summary');
      // 【逐行批注】 计算右侧结果并更新 reviews.hidden，改变页面或请求状态。
      reviews.hidden = Boolean(summary && /待复习\s*[·・]\s*0\s*项/.test(summary.textContent));
      // 【逐行批注】 判断条件 (home.lastElementChild !== reviews) home.append(reviews);，成立时执行对应逻辑。
      if (home.lastElementChild !== reviews) home.append(reviews);
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 调用 MutationObserver 执行页面交互或数据处理：new MutationObserver(arrangeReviews).observe(home, {childList: true, …。
    new MutationObserver(arrangeReviews).observe(home, {childList: true, subtree: true, characterData: true});
    // 【逐行批注】 调用 arrangeReviews 执行页面交互或数据处理：arrangeReviews();。
    arrangeReviews();
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 more，保存本行计算出的数据或对象。
  const more = document.getElementById('classroom-more');
  // 【逐行批注】 声明前端变量 trigger，保存本行计算出的数据或对象。
  const trigger = document.querySelector('[popovertarget=classroom-more]');
  // 【逐行批注】 判断条件 (more && trigger) {，成立时执行对应逻辑。
  if (more && trigger) {
    // 【逐行批注】 为页面事件绑定处理逻辑：trigger.addEventListener('click', () => {。
    trigger.addEventListener('click', () => {
      // 【逐行批注】 声明前端变量 bounds，保存本行计算出的数据或对象。
      const bounds = trigger.getBoundingClientRect();
      // 【逐行批注】 计算右侧结果并更新 more.style.right，改变页面或请求状态。
      more.style.right = `${Math.max(12, innerWidth - bounds.right)}px`;
      // 【逐行批注】 计算右侧结果并更新 more.style.bottom，改变页面或请求状态。
      more.style.bottom = `${Math.max(12, innerHeight - bounds.top + 8)}px`;
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 【逐行批注】 为页面事件绑定处理逻辑：more.addEventListener('click', event => {。
    more.addEventListener('click', event => {
      // 【逐行批注】 判断条件 (event.target.closest('button') && more.matches(':popover-open')…，成立时执行对应逻辑。
      if (event.target.closest('button') && more.matches(':popover-open')) more.hidePopover();
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('keydown', event => {。
  document.addEventListener('keydown', event => {
    // 【逐行批注】 判断条件 (event.key !== 'Escape') return;，成立时执行对应逻辑。
    if (event.key !== 'Escape') return;
    // 【逐行批注】 按循环规则遍历或重复处理：for (const menu of document.querySelectorAll('.dark-mobile-nav[open]')) {。
    for (const menu of document.querySelectorAll('.dark-mobile-nav[open]')) {
      // 【逐行批注】 计算右侧结果并更新 menu.open，改变页面或请求状态。
      menu.open = false;
      // 【逐行批注】 执行当前前端表达式：menu.querySelector('summary')?.focus();。
      menu.querySelector('summary')?.focus();
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 旧报表仍用随包 ECharts，仅统一文字与轴线，不改变数据口径。
  // 【逐行批注】 声明前端变量 themed，保存本行计算出的数据或对象。
  const themed = new WeakSet();
  // 【逐行批注】 声明前端变量 scheduled，保存本行计算出的数据或对象。
  let scheduled = false;
  // 【逐行批注】 定义函数 themeCharts，封装可复用的前端行为。
  function themeCharts() {
    // 【逐行批注】 计算右侧结果并更新 scheduled，改变页面或请求状态。
    scheduled = false;
    // 【逐行批注】 判断条件 (!window.echarts) return;，成立时执行对应逻辑。
    if (!window.echarts) return;
    // 【逐行批注】 按循环规则遍历或重复处理：for (const host of document.querySelectorAll('.ma-chart')) {。
    for (const host of document.querySelectorAll('.ma-chart')) {
      // 【逐行批注】 声明前端变量 chart，保存本行计算出的数据或对象。
      const chart = window.echarts.getInstanceByDom(host);
      // 【逐行批注】 判断条件 (!chart || themed.has(chart)) continue;，成立时执行对应逻辑。
      if (!chart || themed.has(chart)) continue;
      // 【逐行批注】 执行当前前端表达式：chart.setOption({。
      chart.setOption({
        // 【逐行批注】 执行当前前端表达式：color: ['#3d61ce', '#3e8b88', '#b98740', '#9a6bb3', '#60884e'],。
        color: ['#3d61ce', '#3e8b88', '#b98740', '#9a6bb3', '#60884e'],
        // 【逐行批注】 执行当前前端表达式：textStyle: {color: '#5e6d82', fontSize: 12},。
        textStyle: {color: '#5e6d82', fontSize: 12},
        // 【逐行批注】 执行当前前端表达式：legend: {textStyle: {color: '#5e6d82', fontSize: 12}},。
        legend: {textStyle: {color: '#5e6d82', fontSize: 12}},
        // 【逐行批注】 执行当前前端表达式：xAxis: {axisLine: {lineStyle: {color: '#dfe6ef'}}, axisLabel: {color: '#5e6d82', fontSize…。
        xAxis: {axisLine: {lineStyle: {color: '#dfe6ef'}}, axisLabel: {color: '#5e6d82', fontSize: 12}},
        // 【逐行批注】 执行当前前端表达式：yAxis: {axisLabel: {color: '#5e6d82', fontSize: 12}, splitLine: {lineStyle: {color: '#e5e…。
        yAxis: {axisLabel: {color: '#5e6d82', fontSize: 12}, splitLine: {lineStyle: {color: '#e5ebf3'}}}
      // 【逐行批注】 执行当前前端表达式：});。
      });
      // 【逐行批注】 执行当前前端表达式：themed.add(chart);。
      themed.add(chart);
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 声明前端变量 content，保存本行计算出的数据或对象。
  const content = document.getElementById('my-ai-content');
  // 【逐行批注】 判断条件 (content) new MutationObserver(() => {，成立时执行对应逻辑。
  if (content) new MutationObserver(() => {
    // 【逐行批注】 判断条件 (!scheduled) { scheduled = true; requestAnimationFrame(themeChar…，成立时执行对应逻辑。
    if (!scheduled) { scheduled = true; requestAnimationFrame(themeCharts); }
  // 【逐行批注】 执行当前前端表达式：}).observe(content, {childList: true, subtree: true});。
  }).observe(content, {childList: true, subtree: true});
  // 【逐行批注】 调用 themeCharts 执行页面交互或数据处理：themeCharts();。
  themeCharts();
// 【逐行批注】 执行当前前端表达式：})();。
})();

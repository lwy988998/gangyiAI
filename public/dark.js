(() => {
  const radar = document.getElementById('home-radar');
  const cards = document.getElementById('profile-subjects');
  const message = document.getElementById('profile-message');
  const status = document.getElementById('home-profile-status');
  const refresh = document.getElementById('profile-refresh');
  const recentCards = document.getElementById('recent-courses');
  const recentMessage = document.getElementById('recent-courses-message');
  const reducedMotion = matchMedia('(prefers-reduced-motion: reduce)');
  let poll = 0;

  if (!reducedMotion.matches && 'IntersectionObserver' in window) {
    const reveals = document.querySelectorAll('[data-reveal]');
    if (reveals.length) {
      document.body.classList.add('motion-ready');
      const observer = new IntersectionObserver(entries => {
        for (const entry of entries) {
          if (!entry.isIntersecting) continue;
          entry.target.classList.add('is-visible');
          observer.unobserve(entry.target);
        }
      }, {threshold: 0.12});
      reveals.forEach(element => observer.observe(element));
    }
  }
  if (!reducedMotion.matches && matchMedia('(pointer: fine)').matches) {
    document.body.classList.add('has-pointer');
    let pointerFrame = 0;
    addEventListener('pointermove', event => {
      if (pointerFrame) return;
      const x = event.clientX;
      const y = event.clientY;
      pointerFrame = requestAnimationFrame(() => {
        document.body.style.setProperty('--pointer-x', `${x}px`);
        document.body.style.setProperty('--pointer-y', `${y}px`);
        pointerFrame = 0;
      });
    }, {passive: true});
  }

  function node(tag, text, className) {
    const item = document.createElement(tag);
    item.textContent = text;
    if (className) item.className = className;
    return item;
  }

  function localTime(value) {
    const date = new Date(value);
    return Number.isNaN(date.getTime()) ? '时间待更新' : date.toLocaleString('zh-CN', {
      month: 'numeric', day: 'numeric', hour: '2-digit', minute: '2-digit', hour12: false
    });
  }

  let recentLoading = false;
  let recentSignature = '';
  async function loadRecentCourses() {
    if (!recentCards || recentLoading || document.hidden) return;
    recentLoading = true;
    try {
      const response = await fetch('/api/recent-courses', {cache: 'no-store'});
      if (!response.ok) throw new Error('最近课程读取失败，稍后会自动重试。');
      const data = await response.json();
      const courses = (data.courses || []).slice(0, 3);
      const signature = JSON.stringify(courses);
      recentMessage.textContent = courses.length ? `最近学习的${['零', '一', '两', '三'][courses.length]}门课程，点击卡片进入课程。` :
        '还没有课程。从上方输入学习目标，创建你的第一门课程。';
      // 数据未变化时保留节点，避免自动刷新打断键盘焦点。
      if (signature === recentSignature) return;
      recentSignature = signature;
      recentCards.replaceChildren();
      for (const course of courses) {
        const card = node('a', '', 'recent-course-card');
        card.href = `/plan?courseId=${encodeURIComponent(course.courseId)}`;
        const completed = course.totalTopics > 0 && course.doneTopics === course.totalTopics;
        const state = completed ? '已完成' : course.lastActivityAt ? '学习中' : '待开始';
        card.append(node('span', state, 'recent-course-state'));
        card.append(node('h3', course.title || course.goal || '未命名课程'));
        card.append(node('p', course.latestTopic ? `最近课时：${course.latestTopic}` :
          course.lastActivityAt ? '已查看课程，选择一个课时继续学习。' : '课程已生成，进入课程开始学习。', 'recent-course-topic'));
        const progress = node('div', '', 'recent-course-progress');
        progress.append(node('span', course.totalTopics ? `课时完成 ${course.doneTopics} / ${course.totalTopics} 节` : '课时规划待生成'));
        if (course.totalTopics) progress.append(node('strong', `${course.percent}%`));
        card.append(progress);
        const track = node('div', '', 'recent-course-track');
        const fill = node('span', '');
        fill.style.width = `${Math.max(0, Math.min(100, course.percent || 0))}%`;
        track.append(fill);
        card.append(track);
        card.append(node('small', course.lastActivityAt ? `最近学习 · ${localTime(course.lastActivityAt)}` :
          `创建于 · ${localTime(course.createdAt)}`));
        card.append(node('span', '进入课程 ↗', 'recent-course-open'));
        recentCards.append(card);
      }
    } catch (error) {
      recentMessage.textContent = error.message || '最近课程暂时不可用，稍后会自动重试。';
    } finally { recentLoading = false; }
  }

  function showProfile(data) {
    const subjects = (data.subjects || []).filter(item => item.score !== null && Number.isFinite(item.score));
    const time = data.subjects?.map(item => item.updatedAt).filter(Boolean).sort().at(-1);
    const text = data.error || (data.updating ? '画像正在更新；当前显示最近一次有效结果。' :
      data.hasEvidence ? subjects.length ? '画像由 AI 根据真实原题、作答与提示经历评估。' : '等待 AI 根据真实作答评估，证据是否充分由 AI 判断。' :
        '数据不足：暂无可靠测验记录。');
    message.textContent = text;
    if (status) status.textContent = time ? `最近更新：${localTime(time)}` : '数据积累中';
    cards.replaceChildren();
    for (const item of data.subjects || []) {
      const card = node('article', '', 'profile-subject-card');
      const starting = item.score === 0;
      const explanation = starting && (item.rationale || '').includes('unknown=true') ? '你在本次基础诊断中明确反馈“暂时不会”。当前画像只覆盖已测知识点，后续会随着你自己的作答更新。' :
        (item.rationale || '等待更多学习证据。').replace(/\s*\[\[\s*(?:"[0-9a-f]{32}"\s*,\s*)*"[0-9a-f]{32}"\s*\]\]\s*$/i, '').replaceAll('topicStates', '已评估学习记录');
      card.append(node('h3', item.subject || '未命名学科'));
      card.append(node('strong', starting ? '入门起点' : item.score === null ?
        item.historicalScore === null ? '积累中' : `历史画像 ${item.historicalScore}%（待新测验验证）` : `${item.score}%`));
      card.append(node('p', explanation));
      if (item.weakPoints?.length) card.append(node('p', `待加强：${item.weakPoints.join('、')}`));
      if (item.recommendation) card.append(node('p', `建议：${item.recommendation}`));
      card.append(node('small', `依据 ${item.evidenceCount || 0} 份学习记录 · ${item.updatedAt ? localTime(item.updatedAt) : '等待更新'}`));
      cards.append(card);
    }
  }

  async function loadProfile() {
    if (!radar || !cards || !message) return;
    try {
      const response = await fetch('/api/profile');
      if (!response.ok) throw new Error('画像读取失败');
      const data = await response.json();
      showProfile(data);
      if (data.updating && !data.error && !poll) poll = setInterval(loadProfile, 4000);
      if ((!data.updating || data.error) && poll) { clearInterval(poll); poll = 0; }
    } catch (error) {
      message.textContent = error.message;
      // 雷达组件保留上次读取的有效结果。
    }
  }
  refresh?.addEventListener('click', async () => {
    refresh.disabled = true;
    try {
      const response = await fetch('/api/profile/refresh', {method: 'POST'});
      if (!response.ok) throw new Error('更新请求失败');
      message.textContent = '画像正在后台更新，当前结果仍可查看。';
      if (!poll) poll = setInterval(loadProfile, 4000);
      setTimeout(loadProfile, 2500);
    } catch (error) { message.textContent = error.message; }
    finally { refresh.disabled = false; }
  });
  loadProfile();
  loadRecentCourses();
  if (recentCards) {
    setInterval(loadRecentCourses, 15000);
    setInterval(() => { if (!document.hidden) loadProfile(); }, 15000);
    addEventListener('focus', loadRecentCourses);
    addEventListener('focus', loadProfile);
    addEventListener('pageshow', loadRecentCourses);
    document.addEventListener('visibilitychange', loadRecentCourses);
  }
  document.addEventListener('click', event => {
    const link = event.target.closest('a[href]');
    if (!link || reducedMotion.matches || event.defaultPrevented || event.button || event.metaKey || event.ctrlKey || event.shiftKey || event.altKey || link.target || link.hasAttribute('download')) return;
    const target = new URL(link.href, location.href);
    if (target.origin !== location.origin || target.pathname === location.pathname && target.search === location.search) return;
    event.preventDefault();
    document.body.classList.add('is-leaving');
    setTimeout(() => { location.href = target.href; }, 200);
  });
  addEventListener('pageshow', () => document.body.classList.remove('is-leaving'));
})();

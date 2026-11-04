// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 声明前端变量 radar，保存本行计算出的数据或对象。
  const radar = document.getElementById('home-radar');
  // 【逐行批注】 声明前端变量 cards，保存本行计算出的数据或对象。
  const cards = document.getElementById('profile-subjects');
  // 【逐行批注】 声明前端变量 message，保存本行计算出的数据或对象。
  const message = document.getElementById('profile-message');
  // 【逐行批注】 声明前端变量 status，保存本行计算出的数据或对象。
  const status = document.getElementById('home-profile-status');
  // 【逐行批注】 声明前端变量 refresh，保存本行计算出的数据或对象。
  const refresh = document.getElementById('profile-refresh');
  // 【逐行批注】 声明前端变量 recentCards，保存本行计算出的数据或对象。
  const recentCards = document.getElementById('recent-courses');
  // 【逐行批注】 声明前端变量 recentMessage，保存本行计算出的数据或对象。
  const recentMessage = document.getElementById('recent-courses-message');
  // 【逐行批注】 声明前端变量 reducedMotion，保存本行计算出的数据或对象。
  const reducedMotion = matchMedia('(prefers-reduced-motion: reduce)');
  // 【逐行批注】 声明前端变量 poll，保存本行计算出的数据或对象。
  let poll = 0;

  // 【逐行批注】 判断条件 (!reducedMotion.matches && 'IntersectionObserver' in window) {，成立时执行对应逻辑。
  if (!reducedMotion.matches && 'IntersectionObserver' in window) {
    // 【逐行批注】 声明前端变量 reveals，保存本行计算出的数据或对象。
    const reveals = document.querySelectorAll('[data-reveal]');
    // 【逐行批注】 判断条件 (reveals.length) {，成立时执行对应逻辑。
    if (reveals.length) {
      // 【逐行批注】 执行当前前端表达式：document.body.classList.add('motion-ready');。
      document.body.classList.add('motion-ready');
      // 【逐行批注】 声明前端变量 observer，保存本行计算出的数据或对象。
      const observer = new IntersectionObserver(entries => {
        // 【逐行批注】 按循环规则遍历或重复处理：for (const entry of entries) {。
        for (const entry of entries) {
          // 【逐行批注】 判断条件 (!entry.isIntersecting) continue;，成立时执行对应逻辑。
          if (!entry.isIntersecting) continue;
          // 【逐行批注】 执行当前前端表达式：entry.target.classList.add('is-visible');。
          entry.target.classList.add('is-visible');
          // 【逐行批注】 执行当前前端表达式：observer.unobserve(entry.target);。
          observer.unobserve(entry.target);
        // 【逐行批注】 结束当前前端代码块。
        }
      // 【逐行批注】 执行当前前端表达式：}, {threshold: 0.12});。
      }, {threshold: 0.12});
      // 【逐行批注】 计算右侧结果并更新 reveals.forEach(element，改变页面或请求状态。
      reveals.forEach(element => observer.observe(element));
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 判断条件 (!reducedMotion.matches && matchMedia('(pointer: fine)').matches…，成立时执行对应逻辑。
  if (!reducedMotion.matches && matchMedia('(pointer: fine)').matches) {
    // 【逐行批注】 执行当前前端表达式：document.body.classList.add('has-pointer');。
    document.body.classList.add('has-pointer');
    // 【逐行批注】 声明前端变量 pointerFrame，保存本行计算出的数据或对象。
    let pointerFrame = 0;
    // 【逐行批注】 为页面事件绑定处理逻辑：addEventListener('pointermove', event => {。
    addEventListener('pointermove', event => {
      // 【逐行批注】 判断条件 (pointerFrame) return;，成立时执行对应逻辑。
      if (pointerFrame) return;
      // 【逐行批注】 声明前端变量 x，保存本行计算出的数据或对象。
      const x = event.clientX;
      // 【逐行批注】 声明前端变量 y，保存本行计算出的数据或对象。
      const y = event.clientY;
      // 【逐行批注】 计算右侧结果并更新 pointerFrame，改变页面或请求状态。
      pointerFrame = requestAnimationFrame(() => {
        // 【逐行批注】 执行当前前端表达式：document.body.style.setProperty('--pointer-x', `${x}px`);。
        document.body.style.setProperty('--pointer-x', `${x}px`);
        // 【逐行批注】 执行当前前端表达式：document.body.style.setProperty('--pointer-y', `${y}px`);。
        document.body.style.setProperty('--pointer-y', `${y}px`);
        // 【逐行批注】 计算右侧结果并更新 pointerFrame，改变页面或请求状态。
        pointerFrame = 0;
      // 【逐行批注】 执行当前前端表达式：});。
      });
    // 【逐行批注】 执行当前前端表达式：}, {passive: true});。
    }, {passive: true});
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 node，封装可复用的前端行为。
  function node(tag, text, className) {
    // 【逐行批注】 声明前端变量 item，保存本行计算出的数据或对象。
    const item = document.createElement(tag);
    // 【逐行批注】 计算右侧结果并更新 item.textContent，改变页面或请求状态。
    item.textContent = text;
    // 【逐行批注】 判断条件 (className) item.className = className;，成立时执行对应逻辑。
    if (className) item.className = className;
    // 【逐行批注】 结束当前函数并返回结果：return item;。
    return item;
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 localTime，封装可复用的前端行为。
  function localTime(value) {
    // 【逐行批注】 声明前端变量 date，保存本行计算出的数据或对象。
    const date = new Date(value);
    // 【逐行批注】 结束当前函数并返回结果：return Number.isNaN(date.getTime()) ? '时间待更新' : date.toLocaleString('…。
    return Number.isNaN(date.getTime()) ? '时间待更新' : date.toLocaleString('zh-CN', {
      // 【逐行批注】 执行当前前端表达式：month: 'numeric', day: 'numeric', hour: '2-digit', minute: '2-digit', hour12: false。
      month: 'numeric', day: 'numeric', hour: '2-digit', minute: '2-digit', hour12: false
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 声明前端变量 recentLoading，保存本行计算出的数据或对象。
  let recentLoading = false;
  // 【逐行批注】 声明前端变量 recentSignature，保存本行计算出的数据或对象。
  let recentSignature = '';
  // 【逐行批注】 定义函数 loadRecentCourses，封装可复用的前端行为。
  async function loadRecentCourses() {
    // 【逐行批注】 判断条件 (!recentCards || recentLoading || document.hidden) return;，成立时执行对应逻辑。
    if (!recentCards || recentLoading || document.hidden) return;
    // 【逐行批注】 计算右侧结果并更新 recentLoading，改变页面或请求状态。
    recentLoading = true;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch('/api/recent-courses', {cache: 'no-store'});
      // 【逐行批注】 判断条件 (!response.ok) throw new Error('最近课程读取失败，稍后会自动重试。');，成立时执行对应逻辑。
      if (!response.ok) throw new Error('最近课程读取失败，稍后会自动重试。');
      // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
      const data = await response.json();
      // 【逐行批注】 声明前端变量 courses，保存本行计算出的数据或对象。
      const courses = (data.courses || []).slice(0, 3);
      // 【逐行批注】 声明前端变量 signature，保存本行计算出的数据或对象。
      const signature = JSON.stringify(courses);
      // 【逐行批注】 计算右侧结果并更新 recentMessage.textContent，改变页面或请求状态。
      recentMessage.textContent = courses.length ? `最近学习的${['零', '一', '两', '三'][courses.length]}门课程，点击卡片进入课程。` :
        // 【逐行批注】 执行当前前端表达式：'还没有课程。从上方输入学习目标，创建你的第一门课程。';。
        '还没有课程。从上方输入学习目标，创建你的第一门课程。';
      // 数据未变化时保留节点，避免自动刷新打断键盘焦点。
      // 【逐行批注】 判断条件 (signature === recentSignature) return;，成立时执行对应逻辑。
      if (signature === recentSignature) return;
      // 【逐行批注】 计算右侧结果并更新 recentSignature，改变页面或请求状态。
      recentSignature = signature;
      // 【逐行批注】 执行当前前端表达式：recentCards.replaceChildren();。
      recentCards.replaceChildren();
      // 【逐行批注】 按循环规则遍历或重复处理：for (const course of courses) {。
      for (const course of courses) {
        // 【逐行批注】 声明前端变量 card，保存本行计算出的数据或对象。
        const card = node('a', '', 'recent-course-card');
        // 【逐行批注】 计算右侧结果并更新 card.href，改变页面或请求状态。
        card.href = `/plan?courseId=${encodeURIComponent(course.courseId)}`;
        // 【逐行批注】 声明前端变量 completed，保存本行计算出的数据或对象。
        const completed = course.totalTopics > 0 && course.doneTopics === course.totalTopics;
        // 【逐行批注】 声明前端变量 state，保存本行计算出的数据或对象。
        const state = completed ? '已完成' : course.lastActivityAt ? '学习中' : '待开始';
        // 【逐行批注】 调用 node 执行页面交互或数据处理：card.append(node('span', state, 'recent-course-state'));。
        card.append(node('span', state, 'recent-course-state'));
        // 【逐行批注】 调用 node 执行页面交互或数据处理：card.append(node('h3', course.title || course.goal || '未命名课程'));。
        card.append(node('h3', course.title || course.goal || '未命名课程'));
        // 【逐行批注】 调用 node 执行页面交互或数据处理：card.append(node('p', course.latestTopic ? `最近课时：${course.latestTopic…。
        card.append(node('p', course.latestTopic ? `最近课时：${course.latestTopic}` :
          // 【逐行批注】 执行当前前端表达式：course.lastActivityAt ? '已查看课程，选择一个课时继续学习。' : '课程已生成，进入课程开始学习。', 'recent-course-topic'));。
          course.lastActivityAt ? '已查看课程，选择一个课时继续学习。' : '课程已生成，进入课程开始学习。', 'recent-course-topic'));
        // 【逐行批注】 声明前端变量 progress，保存本行计算出的数据或对象。
        const progress = node('div', '', 'recent-course-progress');
        // 【逐行批注】 调用 node 执行页面交互或数据处理：progress.append(node('span', course.totalTopics ? `课时完成 ${course.done…。
        progress.append(node('span', course.totalTopics ? `课时完成 ${course.doneTopics} / ${course.totalTopics} 节` : '课时规划待生成'));
        // 【逐行批注】 判断条件 (course.totalTopics) progress.append(node('strong', `${course.pe…，成立时执行对应逻辑。
        if (course.totalTopics) progress.append(node('strong', `${course.percent}%`));
        // 【逐行批注】 执行当前前端表达式：card.append(progress);。
        card.append(progress);
        // 【逐行批注】 声明前端变量 track，保存本行计算出的数据或对象。
        const track = node('div', '', 'recent-course-track');
        // 【逐行批注】 声明前端变量 fill，保存本行计算出的数据或对象。
        const fill = node('span', '');
        // 【逐行批注】 计算右侧结果并更新 fill.style.width，改变页面或请求状态。
        fill.style.width = `${Math.max(0, Math.min(100, course.percent || 0))}%`;
        // 【逐行批注】 执行当前前端表达式：track.append(fill);。
        track.append(fill);
        // 【逐行批注】 执行当前前端表达式：card.append(track);。
        card.append(track);
        // 【逐行批注】 调用 node、localTime 执行页面交互或数据处理：card.append(node('small', course.lastActivityAt ? `最近学习 · ${localTime…。
        card.append(node('small', course.lastActivityAt ? `最近学习 · ${localTime(course.lastActivityAt)}` :
          // 【逐行批注】 调用 localTime 执行页面交互或数据处理：`创建于 · ${localTime(course.createdAt)}`));。
          `创建于 · ${localTime(course.createdAt)}`));
        // 【逐行批注】 调用 node 执行页面交互或数据处理：card.append(node('span', '进入课程 ↗', 'recent-course-open'));。
        card.append(node('span', '进入课程 ↗', 'recent-course-open'));
        // 【逐行批注】 执行当前前端表达式：recentCards.append(card);。
        recentCards.append(card);
      // 【逐行批注】 结束当前前端代码块。
      }
    // 【逐行批注】 执行当前前端表达式：} catch (error) {。
    } catch (error) {
      // 【逐行批注】 计算右侧结果并更新 recentMessage.textContent，改变页面或请求状态。
      recentMessage.textContent = error.message || '最近课程暂时不可用，稍后会自动重试。';
    // 【逐行批注】 计算右侧结果并更新 } finally { recentLoading，改变页面或请求状态。
    } finally { recentLoading = false; }
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 showProfile，封装可复用的前端行为。
  function showProfile(data) {
    // 【逐行批注】 声明前端变量 subjects，保存本行计算出的数据或对象。
    const subjects = (data.subjects || []).filter(item => item.score !== null && Number.isFinite(item.score));
    // 【逐行批注】 声明前端变量 time，保存本行计算出的数据或对象。
    const time = data.subjects?.map(item => item.updatedAt).filter(Boolean).sort().at(-1);
    // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
    const text = data.error || (data.updating ? '画像正在更新；当前显示最近一次有效结果。' :
      // 【逐行批注】 执行当前前端表达式：data.hasEvidence ? subjects.length ? '画像由 AI 根据真实原题、作答与提示经历评估。' : '等待 AI 根据真实作答评估，证据是否充分由…。
      data.hasEvidence ? subjects.length ? '画像由 AI 根据真实原题、作答与提示经历评估。' : '等待 AI 根据真实作答评估，证据是否充分由 AI 判断。' :
        // 【逐行批注】 执行当前前端表达式：'数据不足：暂无可靠测验记录。');。
        '数据不足：暂无可靠测验记录。');
    // 【逐行批注】 计算右侧结果并更新 message.textContent，改变页面或请求状态。
    message.textContent = text;
    // 【逐行批注】 判断条件 (status) status.textContent = time ? `最近更新：${localTime(time)}` :…，成立时执行对应逻辑。
    if (status) status.textContent = time ? `最近更新：${localTime(time)}` : '数据积累中';
    // 【逐行批注】 执行当前前端表达式：cards.replaceChildren();。
    cards.replaceChildren();
    // 【逐行批注】 按循环规则遍历或重复处理：for (const item of data.subjects || []) {。
    for (const item of data.subjects || []) {
      // 【逐行批注】 声明前端变量 card，保存本行计算出的数据或对象。
      const card = node('article', '', 'profile-subject-card');
      // 【逐行批注】 声明前端变量 starting，保存本行计算出的数据或对象。
      const starting = item.score === 0;
      // 【逐行批注】 声明前端变量 explanation，保存本行计算出的数据或对象。
      const explanation = starting && (item.rationale || '').includes('unknown=true') ? '你在本次基础诊断中明确反馈“暂时不会”。当前画像只覆盖已测知识点，后续会随着你自己的作答更新。' :
        // 【逐行批注】 执行当前前端表达式：(item.rationale || '等待更多学习证据。').replace(/\s*\[\[\s*(?:"[0-9a-f]{32}"\s*,\s*)*"[0-9a-f]{32…。
        (item.rationale || '等待更多学习证据。').replace(/\s*\[\[\s*(?:"[0-9a-f]{32}"\s*,\s*)*"[0-9a-f]{32}"\s*\]\]\s*$/i, '').replaceAll('topicStates', '已评估学习记录');
      // 【逐行批注】 调用 node 执行页面交互或数据处理：card.append(node('h3', item.subject || '未命名学科'));。
      card.append(node('h3', item.subject || '未命名学科'));
      // 【逐行批注】 调用 node 执行页面交互或数据处理：card.append(node('strong', starting ? '入门起点' : item.score === null ?。
      card.append(node('strong', starting ? '入门起点' : item.score === null ?
        // 【逐行批注】 执行当前前端表达式：item.historicalScore === null ? '积累中' : `历史画像 ${item.historicalScore}%（待新测验验证）` : `${item…。
        item.historicalScore === null ? '积累中' : `历史画像 ${item.historicalScore}%（待新测验验证）` : `${item.score}%`));
      // 【逐行批注】 调用 node 执行页面交互或数据处理：card.append(node('p', explanation));。
      card.append(node('p', explanation));
      // 【逐行批注】 判断条件 (item.weakPoints?.length) card.append(node('p', `待加强：${item.weak…，成立时执行对应逻辑。
      if (item.weakPoints?.length) card.append(node('p', `待加强：${item.weakPoints.join('、')}`));
      // 【逐行批注】 判断条件 (item.recommendation) card.append(node('p', `建议：${item.recommend…，成立时执行对应逻辑。
      if (item.recommendation) card.append(node('p', `建议：${item.recommendation}`));
      // 【逐行批注】 调用 node、localTime 执行页面交互或数据处理：card.append(node('small', `依据 ${item.evidenceCount || 0} 份学习记录 · ${it…。
      card.append(node('small', `依据 ${item.evidenceCount || 0} 份学习记录 · ${item.updatedAt ? localTime(item.updatedAt) : '等待更新'}`));
      // 【逐行批注】 执行当前前端表达式：cards.append(card);。
      cards.append(card);
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 loadProfile，封装可复用的前端行为。
  async function loadProfile() {
    // 【逐行批注】 判断条件 (!radar || !cards || !message) return;，成立时执行对应逻辑。
    if (!radar || !cards || !message) return;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch('/api/profile');
      // 【逐行批注】 判断条件 (!response.ok) throw new Error('画像读取失败');，成立时执行对应逻辑。
      if (!response.ok) throw new Error('画像读取失败');
      // 【逐行批注】 声明前端变量 data，保存本行计算出的数据或对象。
      const data = await response.json();
      // 【逐行批注】 调用 showProfile 执行页面交互或数据处理：showProfile(data);。
      showProfile(data);
      // 【逐行批注】 判断条件 (data.updating && !data.error && !poll) poll = setInterval(loadP…，成立时执行对应逻辑。
      if (data.updating && !data.error && !poll) poll = setInterval(loadProfile, 4000);
      // 【逐行批注】 判断条件 ((!data.updating || data.error) && poll) { clearInterval(poll); …，成立时执行对应逻辑。
      if ((!data.updating || data.error) && poll) { clearInterval(poll); poll = 0; }
    // 【逐行批注】 执行当前前端表达式：} catch (error) {。
    } catch (error) {
      // 【逐行批注】 计算右侧结果并更新 message.textContent，改变页面或请求状态。
      message.textContent = error.message;
      // 雷达组件保留上次读取的有效结果。
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 为页面事件绑定处理逻辑：refresh?.addEventListener('click', async () => {。
  refresh?.addEventListener('click', async () => {
    // 【逐行批注】 计算右侧结果并更新 refresh.disabled，改变页面或请求状态。
    refresh.disabled = true;
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // 【逐行批注】 声明前端变量 response，保存本行计算出的数据或对象。
      const response = await fetch('/api/profile/refresh', {method: 'POST'});
      // 【逐行批注】 判断条件 (!response.ok) throw new Error('更新请求失败');，成立时执行对应逻辑。
      if (!response.ok) throw new Error('更新请求失败');
      // 【逐行批注】 计算右侧结果并更新 message.textContent，改变页面或请求状态。
      message.textContent = '画像正在后台更新，当前结果仍可查看。';
      // 【逐行批注】 判断条件 (!poll) poll = setInterval(loadProfile, 4000);，成立时执行对应逻辑。
      if (!poll) poll = setInterval(loadProfile, 4000);
      // 【逐行批注】 调用 setTimeout 执行页面交互或数据处理：setTimeout(loadProfile, 2500);。
      setTimeout(loadProfile, 2500);
    // 【逐行批注】 计算右侧结果并更新 } catch (error) { message.textContent，改变页面或请求状态。
    } catch (error) { message.textContent = error.message; }
    // 【逐行批注】 计算右侧结果并更新 finally { refresh.disabled，改变页面或请求状态。
    finally { refresh.disabled = false; }
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 调用 loadProfile 执行页面交互或数据处理：loadProfile();。
  loadProfile();
  // 【逐行批注】 调用 loadRecentCourses 执行页面交互或数据处理：loadRecentCourses();。
  loadRecentCourses();
  // 【逐行批注】 判断条件 (recentCards) {，成立时执行对应逻辑。
  if (recentCards) {
    // 【逐行批注】 调用 setInterval 执行页面交互或数据处理：setInterval(loadRecentCourses, 15000);。
    setInterval(loadRecentCourses, 15000);
    // 【逐行批注】 计算右侧结果并更新 setInterval(()，改变页面或请求状态。
    setInterval(() => { if (!document.hidden) loadProfile(); }, 15000);
    // 【逐行批注】 为页面事件绑定处理逻辑：addEventListener('focus', loadRecentCourses);。
    addEventListener('focus', loadRecentCourses);
    // 【逐行批注】 为页面事件绑定处理逻辑：addEventListener('focus', loadProfile);。
    addEventListener('focus', loadProfile);
    // 【逐行批注】 为页面事件绑定处理逻辑：addEventListener('pageshow', loadRecentCourses);。
    addEventListener('pageshow', loadRecentCourses);
    // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('visibilitychange', loadRecentCourses);。
    document.addEventListener('visibilitychange', loadRecentCourses);
  // 【逐行批注】 结束当前前端代码块。
  }
  // 【逐行批注】 为页面事件绑定处理逻辑：document.addEventListener('click', event => {。
  document.addEventListener('click', event => {
    // 【逐行批注】 声明前端变量 link，保存本行计算出的数据或对象。
    const link = event.target.closest('a[href]');
    // 【逐行批注】 判断条件 (!link || reducedMotion.matches || event.defaultPrevented || eve…，成立时执行对应逻辑。
    if (!link || reducedMotion.matches || event.defaultPrevented || event.button || event.metaKey || event.ctrlKey || event.shiftKey || event.altKey || link.target || link.hasAttribute('download')) return;
    // 【逐行批注】 声明前端变量 target，保存本行计算出的数据或对象。
    const target = new URL(link.href, location.href);
    // 【逐行批注】 判断条件 (target.origin !== location.origin || target.pathname === locati…，成立时执行对应逻辑。
    if (target.origin !== location.origin || target.pathname === location.pathname && target.search === location.search) return;
    // 【逐行批注】 执行当前前端表达式：event.preventDefault();。
    event.preventDefault();
    // 【逐行批注】 执行当前前端表达式：document.body.classList.add('is-leaving');。
    document.body.classList.add('is-leaving');
    // 【逐行批注】 计算右侧结果并更新 setTimeout(()，改变页面或请求状态。
    setTimeout(() => { location.href = target.href; }, 200);
  // 【逐行批注】 执行当前前端表达式：});。
  });
  // 【逐行批注】 为页面事件绑定处理逻辑：addEventListener('pageshow', () => document.body.classList.remove('is-leaving'));。
  addEventListener('pageshow', () => document.body.classList.remove('is-leaving'));
// 【逐行批注】 执行当前前端表达式：})();。
})();

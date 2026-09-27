(() => {
  const radar = document.getElementById('home-radar');
  const cards = document.getElementById('profile-subjects');
  const message = document.getElementById('profile-message');
  const status = document.getElementById('home-profile-status');
  const refresh = document.getElementById('profile-refresh');
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

  function showProfile(data) {
    const subjects = (data.subjects || []).filter(item => item.score !== null && Number.isFinite(item.score));
    const time = data.subjects?.map(item => item.updatedAt).filter(Boolean).sort().at(-1);
    const text = data.error || (data.updating ? '画像正在更新；当前显示最近一次有效结果。' :
      data.hasEvidence ? subjects.length ? '画像依据本机课程和学习记录生成。' : '数据积累中，完成学习或测验后将逐步形成画像。' :
        '还没有学习记录。创建课程后，画像会在本机自动建立。');
    message.textContent = text;
    status.textContent = time ? `最近更新：${time.replace('T', ' ').slice(0, 16)}` : '数据积累中';
    radar.replaceChildren();
    cards.replaceChildren();
    if (subjects.length < 3) {
      const list = node('div', '', 'radar-list');
      if (!subjects.length) list.append(node('p', '暂无可展示的学科强度'));
      for (const item of subjects) list.append(node('p', `${item.subject} · ${item.score}%`));
      radar.append(list);
    } else {
      const svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
      svg.setAttribute('viewBox', '0 0 360 270');
      svg.setAttribute('aria-hidden', 'true');
      const center = [180, 135];
      const point = (index, radius) => {
        const angle = -Math.PI / 2 + index * Math.PI * 2 / subjects.length;
        return [center[0] + Math.cos(angle) * radius, center[1] + Math.sin(angle) * radius];
      };
      const polygon = (radius) => subjects.map((_, index) => point(index, radius).join(',')).join(' ');
      for (const radius of [35, 68, 98]) {
        const grid = document.createElementNS(svg.namespaceURI, 'polygon');
        grid.setAttribute('points', polygon(radius));
        grid.setAttribute('class', 'radar-grid');
        svg.append(grid);
      }
      const shape = document.createElementNS(svg.namespaceURI, 'polygon');
      shape.setAttribute('points', subjects.map((item, index) => point(index, 98 * item.score / 100).join(',')).join(' '));
      svg.append(shape);
      subjects.forEach((item, index) => {
        const [x, y] = point(index, 117);
        const label = document.createElementNS(svg.namespaceURI, 'text');
        label.setAttribute('x', x);
        label.setAttribute('y', y);
        label.setAttribute('text-anchor', 'middle');
        label.textContent = `${item.subject} ${item.score}%`;
        svg.append(label);
      });
      radar.append(svg);
      radar.setAttribute('aria-label', subjects.map(item => `${item.subject} ${item.score}%`).join('，'));
    }
    for (const item of data.subjects || []) {
      const card = node('article', '', 'profile-subject-card');
      card.append(node('h3', item.subject || '未命名学科'));
      card.append(node('strong', item.score === null ? '积累中' : `${item.score}%`));
      card.append(node('p', item.rationale || '等待更多学习证据。'));
      if (item.weakPoints?.length) card.append(node('p', `待加强：${item.weakPoints.join('、')}`));
      if (item.recommendation) card.append(node('p', `建议：${item.recommendation}`));
      card.append(node('small', `依据 ${item.evidenceCount || 0} 条 · ${item.updatedAt || '等待更新'}`));
      cards.append(card);
    }
  }

  async function loadProfile() {
    if (!radar) return;
    try {
      const response = await fetch('/api/profile');
      if (!response.ok) throw new Error('画像读取失败');
      const data = await response.json();
      showProfile(data);
      if (data.updating && !data.error && !poll) poll = setInterval(loadProfile, 4000);
      if ((!data.updating || data.error) && poll) { clearInterval(poll); poll = 0; }
    } catch (error) {
      message.textContent = error.message;
      radar.textContent = '画像暂时不可用';
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

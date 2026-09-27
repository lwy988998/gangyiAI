const art = {
  aurora: `<div class="aurora-field"><i></i><i></i><i></i></div><div class="aurora-orb"><div class="aurora-core">钢<span>一</span></div></div><div class="aurora-lines"><i></i><i></i><i></i></div>`,
  orbit: `<div class="orbit-art"><div class="orbit-ring ring-a"></div><div class="orbit-ring ring-b"></div><div class="orbit-ring ring-c"></div><div class="orbit-sweep"></div><div class="orbit-center"><small>GANGYI</small><b>AI</b><small>READY</small></div><span class="orbit-node n1"></span><span class="orbit-node n2"></span><span class="orbit-node n3"></span></div>`,
  particles: `<div class="particle-art"><div class="particle-cloud"></div><div class="particle-sigil">钢一</div><div class="particle-frame"><span>LEARNING</span><span>INTELLIGENCE</span></div></div>`,
  scan: `<div class="scan-art"><div class="scan-grid"></div><div class="scan-beam"></div><div class="scan-copy"><small>SYSTEM INITIALIZATION</small><strong>钢一定制AI</strong><span>把每一步学习，点亮成清晰的路径。</span></div><div class="scan-metrics"><span>PROFILE <b>READY</b></span><span>COURSES <b>READY</b></span><span>WORKSPACE <b>READY</b></span></div></div>`,
  panels: `<div class="panel-art"><div class="panel-card card-left"><small>01 / PATH</small><b>学习路径</b><i></i><i></i><i></i></div><div class="panel-card card-main"><small>GANGYI · AI</small><div class="panel-logo">钢一</div><strong>你的学习空间<br>正在就绪</strong><span>LEARNING, IN MOTION</span></div><div class="panel-card card-right"><small>02 / PROFILE</small><b>专属画像</b><div class="panel-chart"></div></div></div>`
};

const variant = document.body.dataset.variant;
if (!art[variant]) throw new Error('未知的动画方案');
document.title = '钢一定制AI 开机预览';
document.body.innerHTML = `
  <div class="preview-controls"><a href="gallery.html" aria-label="返回五款方案">← 五款方案</a><button type="button" id="replay">↻ 重播</button><button type="button" id="skip">跳过动画 →</button></div>
  <div class="demo" data-phase="intro">
    <main class="home" id="home" inert aria-label="钢一定制AI 首页预览">
      <header class="home-header"><a class="home-brand" href="gallery.html"><span class="home-brand-mark">钢</span>钢一<b>定制AI</b></a><nav aria-label="首页导航"><span class="active">学习路径</span><span>学习画像</span><span>AI 导师</span><span>我的课程</span></nav><span class="home-cta">我的画像 ↗</span></header>
      <section class="home-hero"><div class="home-content"><img class="school-logo" src="../../public/school-logo.png" alt="柳州市钢一中学校徽"><p class="eyebrow">柳州市钢一中学 · AI 学习平台</p><h1>让每个学习目标，<em>都看见进步。</em></h1><p class="lead">从真实学习记录建立本机专属画像，让 AI 为每个知识点找到更合适的讲解和练习。</p><div class="goal-box"><label for="goal">今天想提升哪一门高中课程？</label><textarea id="goal" placeholder="例如：高一数学函数与图像"></textarea><div class="goal-actions"><span>＋ 上传参考图</span><button type="button">生成 →</button></div><div class="goal-modes"><span>快速规划<small>快速生成学习路线</small></span><span>深度课程 · 推荐<small>搜索资料并生成完整课程</small></span></div></div><div class="examples"><span>高中数学函数与导数</span><span>高考物理力学训练</span><span>英语阅读与写作</span></div></div><aside class="home-visual"><div class="radar-card"><small>本机画像</small><strong>学科掌握强度</strong><div class="radar"><svg viewBox="0 0 320 260" aria-hidden="true"><polygon points="160,27 272,91 272,169 160,233 48,169 48,91"/><polygon points="160,58 242,106 242,154 160,202 78,154 78,106"/><polygon points="160,87 216,118 216,142 160,173 104,142 104,118"/><polygon class="radar-shape" points="160,62 235,111 224,148 160,194 93,146 98,103"/><text x="160" y="17" text-anchor="middle">数学</text><text x="282" y="91">物理</text><text x="282" y="181">英语</text><text x="160" y="253" text-anchor="middle">化学</text><text x="10" y="181">语文</text><text x="10" y="91">生物</text></svg></div><div class="radar-bottom"><span>本机学习记录</span><span>查看详情 ↓</span></div></div><div class="float-chip top">↗ <span>学习进展<small>持续记录</small></span></div><div class="float-chip bottom">◐ <span>专属画像<small>本机保存</small></span></div></aside></section>
      <div class="home-ticker">学习目标 <b>✦</b> 阶段规划 <b>✦</b> 微课程 <b>✦</b> 练习测验 <b>✦</b> 本机画像 <b>✦</b> AI 导师 <b>✦</b> 学习目标 <b>✦</b> 阶段规划</div>
    </main>
    <div class="splash" role="status" aria-live="polite"><div class="splash-grid"></div><div class="scene">${art[variant]}</div><div class="splash-brand"><small>柳州市钢一中学 · AI 学习平台</small><h2>钢一定制AI</h2></div><div class="startup-progress"><span></span></div><div class="transition-layer" aria-hidden="true"></div></div>
  </div>
  `;

const demo = document.querySelector('.demo');
demo.dataset.variant = variant;
const home = document.getElementById('home');
const reducedMotion = matchMedia('(prefers-reduced-motion: reduce)');
let timers = [];
function clearTimers() { timers.forEach(clearTimeout); timers = []; }
function finish() {
  demo.dataset.phase = 'home';
  home.inert = false;
  document.querySelector('.splash').setAttribute('aria-hidden', 'true');
}
function play() {
  clearTimers();
  home.inert = true;
  const splash = document.querySelector('.splash');
  splash.replaceWith(splash.cloneNode(true));
  demo.dataset.phase = 'reset';
  void demo.offsetWidth;
  demo.dataset.phase = 'intro';
  if (reducedMotion.matches) { timers.push(setTimeout(finish, 250)); return; }
  timers.push(setTimeout(() => { demo.dataset.phase = 'outro'; }, 3450));
  timers.push(setTimeout(finish, 5000));
}
document.getElementById('replay').addEventListener('click', play);
document.getElementById('skip').addEventListener('click', () => { clearTimers(); finish(); });
play();

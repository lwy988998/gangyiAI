const art = {
  aurora: `<div class="aurora-field"><i></i><i></i><i></i></div><div class="aurora-orb"><div class="aurora-core">钢<span>一</span></div></div><div class="aurora-lines"><i></i><i></i><i></i></div>`,
  orbit: `<div class="orbit-art"><div class="orbit-ring ring-a"></div><div class="orbit-ring ring-b"></div><div class="orbit-ring ring-c"></div><div class="orbit-sweep"></div><div class="orbit-center"><small>GANGYI</small><b>AI</b><small>READY</small></div><span class="orbit-node n1"></span><span class="orbit-node n2"></span><span class="orbit-node n3"></span></div>`,
  particles: `<div class="particle-art"><div class="particle-cloud"></div><div class="particle-sigil">钢一</div><div class="particle-frame"><span>LEARNING</span><span>INTELLIGENCE</span></div></div>`,
  scan: `<div class="scan-art"><div class="scan-grid"></div><div class="scan-beam"></div><div class="scan-copy"><small>SYSTEM INITIALIZATION</small><strong>钢一定制AI</strong><span>把每一步学习，点亮成清晰的路径。</span></div><div class="scan-metrics"><span>PROFILE <b>READY</b></span><span>COURSES <b>READY</b></span><span>WORKSPACE <b>READY</b></span></div></div>`,
  panels: `<div class="panel-art"><div class="panel-card card-left"><small>01 / PATH</small><b>学习路径</b><i></i><i></i><i></i></div><div class="panel-card card-main"><small>GANGYI · AI</small><div class="panel-logo">钢一</div><strong>你的学习空间<br>正在就绪</strong><span>LEARNING, IN MOTION</span></div><div class="panel-card card-right"><small>02 / PROFILE</small><b>专属画像</b><div class="panel-chart"></div></div></div>`
};

const variant = document.body.dataset.startupVariant;
const overlay = document.getElementById('startup-overlay');
const app = [document.querySelector('.site-header'), document.querySelector('main.home-page'), document.querySelector('.dark-footer')];
app.forEach(element => { if (element) element.inert = true; });
overlay.querySelector('.startup-scene').innerHTML = art[variant] || art.aurora;
setTimeout(() => { document.body.dataset.startupPhase = 'outro'; }, 3450);
setTimeout(() => {
  document.body.dataset.startupPhase = 'home';
  document.body.classList.remove('startup-playing');
  app.forEach(element => { if (element) element.inert = false; });
  overlay.remove();
  history.replaceState(null, '', '/');
}, 5000);

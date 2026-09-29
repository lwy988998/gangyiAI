const overlay = document.getElementById('startup-overlay');
const app = [document.querySelector('.site-header'), document.querySelector('main.home-page'), document.querySelector('.dark-footer')];

app.forEach(element => { if (element) element.inert = true; });
overlay.querySelector('.startup-scene').innerHTML = `
  <div class="scan-art">
    <div class="scan-grid"></div>
    <div class="scan-beam"></div>
    <div class="scan-copy">
      <small>SYSTEM INITIALIZATION</small>
      <strong>钢一定制AI</strong>
      <span>把每一步学习，点亮成清晰的路径。</span>
    </div>
    <div class="scan-metrics">
      <span>PROFILE <b>READY</b></span>
      <span>COURSES <b>READY</b></span>
      <span>WORKSPACE <b>READY</b></span>
    </div>
  </div>`;

function finish() {
  document.body.dataset.startupPhase = 'home';
  document.body.classList.remove('startup-playing');
  app.forEach(element => { if (element) element.inert = false; });
  overlay.remove();
  history.replaceState(null, '', '/');
}

if (matchMedia('(prefers-reduced-motion: reduce)').matches) {
  finish();
} else {
  setTimeout(() => { document.body.dataset.startupPhase = 'outro'; }, 3450);
  setTimeout(finish, 5000);
}

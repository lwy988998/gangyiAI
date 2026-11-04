// 【逐行批注】 声明前端变量 overlay，保存本行计算出的数据或对象。
const overlay = document.getElementById('startup-overlay');
// 【逐行批注】 声明前端变量 app，保存本行计算出的数据或对象。
const app = [document.querySelector('.site-header'), document.querySelector('main.home-page'), document.querySelector('.dark-footer')];

// 【逐行批注】 计算右侧结果并更新 app.forEach(element，改变页面或请求状态。
app.forEach(element => { if (element) element.inert = true; });
// 【逐行批注】 计算右侧结果并更新 overlay.querySelector('.startup-scene').inne…，改变页面或请求状态。
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

// 【逐行批注】 定义函数 finish，封装可复用的前端行为。
function finish() {
  // 【逐行批注】 计算右侧结果并更新 document.body.dataset.startupPhase，改变页面或请求状态。
  document.body.dataset.startupPhase = 'home';
  // 【逐行批注】 执行当前前端表达式：document.body.classList.remove('startup-playing');。
  document.body.classList.remove('startup-playing');
  // 【逐行批注】 计算右侧结果并更新 app.forEach(element，改变页面或请求状态。
  app.forEach(element => { if (element) element.inert = false; });
  // 【逐行批注】 执行当前前端表达式：overlay.remove();。
  overlay.remove();
  // 【逐行批注】 执行当前前端表达式：history.replaceState(null, '', '/');。
  history.replaceState(null, '', '/');
// 【逐行批注】 结束当前前端代码块。
}

// 【逐行批注】 判断条件 (matchMedia('(prefers-reduced-motion: reduce)').matches) {，成立时执行对应逻辑。
if (matchMedia('(prefers-reduced-motion: reduce)').matches) {
  // 【逐行批注】 调用 finish 执行页面交互或数据处理：finish();。
  finish();
// 【逐行批注】 执行当前前端表达式：} else {。
} else {
  // 【逐行批注】 计算右侧结果并更新 setTimeout(()，改变页面或请求状态。
  setTimeout(() => { document.body.dataset.startupPhase = 'outro'; }, 3450);
  // 【逐行批注】 调用 setTimeout 执行页面交互或数据处理：setTimeout(finish, 5000);。
  setTimeout(finish, 5000);
// 【逐行批注】 结束当前前端代码块。
}

/* 全站读取已保存的主控状态，提供暂停、恢复、撤回和自主备课通知。 */
(() => {
  'use strict';
  const api = window.GangyiAgent;
  if (!api || document.getElementById('agent-control-panel')) return;
  const panel = document.createElement('details'); panel.id = 'agent-control-panel';
  const label = document.createElement('summary'); label.textContent = 'AI 教学状态';
  const state = document.createElement('p'); state.setAttribute('role', 'status');
  const actions = document.createElement('div'), changes = document.createElement('div');
  panel.append(label, state, actions, changes); document.body.append(panel);
  const style = document.createElement('style');
  style.textContent = '#agent-control-panel{position:fixed;right:1rem;bottom:1rem;z-index:40;max-width:min(380px,calc(100vw - 2rem));max-height:65vh;overflow:auto;padding:.75rem 1rem;border:1px solid #354542;border-radius:16px;background:#101a20;color:#e9f2ef;box-shadow:0 10px 40px #0006;font:14px/1.6 system-ui,Microsoft YaHei}#agent-control-panel summary{cursor:pointer;font-weight:600}#agent-control-panel button,#agent-control-panel a{display:inline-block;margin:.25rem .5rem .25rem 0;padding:.35rem .6rem;border:1px solid #50655f;border-radius:9px;background:transparent;color:#a4e5d4;text-decoration:none;cursor:pointer}#agent-control-panel p{white-space:normal;overflow-wrap:anywhere}#agent-control-panel article{border-top:1px solid #354542;margin-top:.75rem;padding-top:.5rem}';
  document.head.append(style); let current;
  const button = (title, command) => {
    const node = document.createElement('button'); node.type = 'button'; node.textContent = title;
    node.onclick = async () => { node.disabled = true; try { await api.control({ command, taskId: current.id }); await load(); } catch (error) { state.textContent = error.message; } finally { node.disabled = false; } };
    actions.append(node);
  };
  async function load() {
    try {
      const task = await api.request('/api/learning-agent'); current = task;
      if (!task.id) { panel.hidden = true; return; } panel.hidden = false;
      const active = ['pending', 'running'].includes(task.status), paused = task.paused || task.status === 'paused';
      const text = paused ? 'AI 已暂停，输入和有效结果已保存。' : ({ pending: 'AI 正在等待处理最新学习情况。', running: 'AI 正在结合真实学习情况处理。', ready: 'AI 已完成本次处理。', waiting_student: 'AI 正在等待你的下一次回答。', failed: 'AI 连续请求失败，已停止并保留有效结果，可手动重试。', superseded: '学习情况已有更新，旧结果没有应用。', cancelled: '本次处理已停止。' })[task.status] || 'AI 状态已保存。';
      if (state.textContent !== text) state.textContent = text;
      label.textContent = active ? 'AI 教学进行中' : paused ? 'AI 已暂停' : task.status === 'ready' && task.lesson ? 'AI 已备好下一课，点击进入' : 'AI 教学状态'; actions.replaceChildren();
      if (paused) button('恢复 AI', 'resume'); else button('暂停 AI', 'pause');
      if (['failed', 'superseded', 'cancelled'].includes(task.status)) button('重试', 'retry');
      if (task.status === 'ready' && task.lesson) {
        const link = document.createElement('a'); link.textContent = `已备好：${task.lesson.title} →`;
        link.href = `/agent-prepare.html?taskId=${encodeURIComponent(task.id)}`; actions.append(link);
      }
      changes.replaceChildren();
      for (const change of (task.changeHistory || []).filter(item => item.status === 'applied').slice(-5).reverse()) {
        const row = document.createElement('article'), reason = document.createElement('p'), undo = document.createElement('button');
        reason.textContent = change.reason; undo.type = 'button'; undo.textContent = '撤回这次调整';
        undo.onclick = async () => { undo.disabled = true; try { await api.control({ command: 'undo', changeId: change.id }); await load(); } catch (error) { reason.textContent = error.message; } finally { undo.disabled = false; } };
        row.append(reason, undo); changes.append(row);
      }
    } catch (error) { state.textContent = error.message; }
  }
  load(); const timer = setInterval(() => { if (!document.hidden) load(); }, 1800);
  window.addEventListener('pagehide', () => clearInterval(timer));
})();

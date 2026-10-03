/* 钢一定制AI 共用对话渲染：Markdown 子集 + KaTeX 公式 + 流式打字机视图。 */
(() => {
  'use strict';

  // 私有区字符不会被 HTML 解析或转义破坏，可安全充当占位符。
  const OPEN = '\uE000';
  const CLOSE = '\uE001';
  const TOKEN = new RegExp(OPEN + '([MC])(\\d+)' + CLOSE, 'g');

  const escapeHtml = value => String(value ?? '')
    .replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;').replace(/'/g, '&#39;');

  const token = (kind, index) => OPEN + kind + index + CLOSE;
  const isToken = (text, kind) => new RegExp('^' + OPEN + kind + '\\d+' + CLOSE + '$').test(text);

  function restore(html, store) {
    return html.replace(TOKEN, (_match, kind, index) => {
      const list = kind === 'M' ? store.math : store.code;
      const value = list[Number(index)];
      return value === undefined ? '' : value;
    });
  }

  function renderMath(latex, displayMode) {
    const source = String(latex ?? '').trim();
    if (!source) return '';
    if (!window.katex || typeof window.katex.renderToString !== 'function')
      return escapeHtml(displayMode ? '$$' + source + '$$' : '$' + source + '$');
    try {
      // trust=false 关闭 \href 等外部指令；strict=ignore 让个别不支持的写法退回文本而不是报错。
      return window.katex.renderToString(source, {
        displayMode, throwOnError: false, strict: 'ignore', trust: false, output: 'htmlAndMathml',
      });
    } catch (_) {
      return escapeHtml(source);
    }
  }

  function extractCode(raw, store) {
    let text = raw.replace(/```([^\n`]*)\n?([\s\S]*?)```/g, (_match, lang, body) => {
      const label = String(lang).trim();
      const header = label ? '<span class="chat-code-lang">' + escapeHtml(label) + '</span>' : '';
      store.code.push('<div class="chat-code">' + header + '<pre><code>' +
        escapeHtml(String(body).replace(/\n+$/, '')) + '</code></pre></div>');
      return '\n' + token('C', store.code.length - 1) + '\n';
    });
    // 流式过程中代码块尚未闭合：先把剩余内容按代码展示，避免露出裸反引号。
    const open = text.indexOf('```');
    if (open !== -1) {
      const body = text.slice(open + 3).replace(/^[^\n`]*\n?/, '');
      store.code.push('<div class="chat-code"><pre><code>' + escapeHtml(body) + '</code></pre></div>');
      text = text.slice(0, open) + '\n' + token('C', store.code.length - 1) + '\n';
    }
    return text;
  }

  function extractMath(raw, store, withMath) {
    const keep = (latex, display) => {
      if (!withMath) {
        store.math.push(escapeHtml(display ? '$$' + String(latex).trim() + '$$' : '$' + String(latex).trim() + '$'));
      } else {
        store.math.push(renderMath(latex, display));
      }
      return token('M', store.math.length - 1);
    };
    let text = raw.replace(/\$\$([\s\S]+?)\$\$/g, (_m, body) => keep(body, true));
    text = text.replace(/\\\[([\s\S]+?)\\\]/g, (_m, body) => keep(body, true));
    text = text.replace(/\\\(([\s\S]+?)\\\)/g, (_m, body) => keep(body, false));
    text = text.replace(/\$([^\n$]{1,400}?)\$/g, (_m, body) => keep(body, false));
    return text;
  }

  function inline(text, store) {
    let out = text;
    out = out.replace(/`([^`\n]+)`/g, '<code>$1</code>');
    out = out.replace(/\[([^\]\n]+)\]\(([^)\s]+)\)/g, (match, label, url) => {
      const safe = /^https?:\/\//i.test(url) ? url.replace(/"/g, '%22') : '';
      return safe ? '<a href="' + safe + '" target="_blank" rel="noopener noreferrer">' + label + '</a>' : label;
    });
    out = out.replace(/\*\*([^*\n]+)\*\*/g, '<strong>$1</strong>');
    out = out.replace(/__([^_\n]+)__/g, '<strong>$1</strong>');
    out = out.replace(/(^|[^*\w])\*([^*\n]+)\*/g, '$1<em>$2</em>');
    out = out.replace(/(^|[^_\w])_([^_\n]+)_/g, '$1<em>$2</em>');
    return restore(out, store);
  }

  const LIST_ITEM = /^(\s*)([-*+]|\d+[.)])\s+(.*)$/;
  const BLOCK_START = /^\s{0,3}(#{1,6}\s|>|-{3,}\s*$|\*{3,}\s*$|_{3,}\s*$)/;

  function renderList(lines, start, store, depth) {
    const first = LIST_ITEM.exec(lines[start]);
    const ordered = /\d/.test(first[2]);
    const baseIndent = first[1].length;
    const items = [];
    let index = start;
    while (index < lines.length) {
      const match = LIST_ITEM.exec(lines[index]);
      if (!match) {
        const trimmed = lines[index].trim();
        if (items.length && trimmed && lines[index].search(/\S/) > baseIndent) {
          items[items.length - 1].text += '\n' + trimmed;
          index++;
          continue;
        }
        break;
      }
      if (match[1].length < baseIndent) break;
      if (match[1].length > baseIndent && items.length && depth < 3) {
        const nested = renderList(lines, index, store, depth + 1);
        items[items.length - 1].nested = nested.html;
        index = nested.next;
        continue;
      }
      items.push({ text: match[3], nested: '' });
      index++;
    }
    const tag = ordered ? 'ol' : 'ul';
    const body = items.map(item => '<li>' + inline(item.text, store) + item.nested + '</li>').join('');
    return { html: '<' + tag + '>' + body + '</' + tag + '>', next: index };
  }

  function renderBlocks(text, store, depth) {
    const lines = text.split('\n');
    const out = [];
    let index = 0;
    while (index < lines.length) {
      const line = lines[index];
      if (!line.trim()) { index++; continue; }
      if (isToken(line.trim(), 'C')) { out.push(restore(line.trim(), store)); index++; continue; }
      const heading = /^(#{1,6})\s+(.*)$/.exec(line);
      if (heading) {
        const level = Math.min(heading[1].length + 2, 6);
        out.push('<h' + level + '>' + inline(heading[2], store) + '</h' + level + '>');
        index++;
        continue;
      }
      if (/^\s{0,3}(-{3,}|\*{3,}|_{3,})\s*$/.test(line)) { out.push('<hr>'); index++; continue; }
      if (/^\s{0,3}>\s?/.test(line)) {
        const quote = [];
        while (index < lines.length && /^\s{0,3}>\s?/.test(lines[index])) {
          quote.push(lines[index].replace(/^\s{0,3}>\s?/, ''));
          index++;
        }
        out.push('<blockquote>' + (depth < 3 ? renderBlocks(quote.join('\n'), store, depth + 1)
          : '<p>' + inline(quote.join('\n'), store) + '</p>') + '</blockquote>');
        continue;
      }
      if (LIST_ITEM.test(line)) {
        const listed = renderList(lines, index, store, depth);
        out.push(listed.html);
        index = listed.next;
        continue;
      }
      const paragraph = [];
      while (index < lines.length && lines[index].trim() && !BLOCK_START.test(lines[index]) &&
             !LIST_ITEM.test(lines[index]) && !isToken(lines[index].trim(), 'C')) {
        paragraph.push(lines[index]);
        index++;
      }
      if (paragraph.length) out.push('<p>' + inline(paragraph.join('\n'), store) + '</p>');
      else index++;
    }
    return out.join('');
  }

  function renderMarkdown(text, options) {
    const settings = Object.assign({ math: true }, options);
    const store = { math: [], code: [] };
    const guarded = extractMath(extractCode(String(text ?? ''), store), store, settings.math);
    return renderBlocks(escapeHtml(guarded), store, 0);
  }

  const reduceMotion = () => window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  function scrollMetrics(root) {
    if (root === window || root === document) {
      const doc = document.documentElement;
      return { top: window.scrollY, height: doc.scrollHeight, view: window.innerHeight };
    }
    return { top: root.scrollTop, height: root.scrollHeight, view: root.clientHeight };
  }

  function scrollToEnd(root, smooth) {
    const behavior = smooth && !reduceMotion() ? 'smooth' : 'auto';
    if (root === window || root === document) window.scrollTo({ top: document.documentElement.scrollHeight, behavior });
    else root.scrollTo({ top: root.scrollHeight, behavior });
  }

  function copyText(text) {
    if (navigator.clipboard && navigator.clipboard.writeText) return navigator.clipboard.writeText(text);
    return new Promise((resolve, reject) => {
      const area = document.createElement('textarea');
      area.value = text;
      area.setAttribute('readonly', '');
      area.style.position = 'fixed';
      area.style.opacity = '0';
      document.body.append(area);
      area.select();
      const ok = document.execCommand && document.execCommand('copy');
      area.remove();
      ok ? resolve() : reject(new Error('复制失败'));
    });
  }

  // 逐字平滑显示：服务器分片到达后进入缓冲，按积压量自适应揭示速度。
  function createAssistantView(container, options) {
    const settings = Object.assign({ scrollRoot: null, dark: true }, options);
    const article = document.createElement('article');
    article.className = 'chat-row chat-row-ai';
    const bubble = document.createElement('div');
    bubble.className = 'chat-bubble chat-bubble-ai' + (settings.dark ? ' chat-bubble-dark' : '');
    const body = document.createElement('div');
    body.className = 'chat-body';
    const cursor = document.createElement('span');
    cursor.className = 'chat-cursor';
    cursor.setAttribute('aria-hidden', 'true');
    bubble.append(body, cursor);
    article.append(bubble);
    container.append(article);

    const scrollRoot = settings.scrollRoot || window;
    let jump = null;
    function jumpHost() {
      if (!jump) {
        jump = document.createElement('div');
        jump.className = 'chat-jump-wrap';
        jump.innerHTML = '<button type="button" class="chat-jump">↓ 回到最新</button>';
        jump.hidden = true;
        jump.querySelector('button').addEventListener('click', () => {
          stuck = false;
          jump.hidden = true;
          scrollToEnd(scrollRoot, true);
        });
      }
      if (container.lastElementChild !== jump) container.append(jump);
      return jump;
    }

    let stuck = false;
    const trackRoot = scrollRoot === window || scrollRoot === document ? window : scrollRoot;
    const onScroll = () => {
      const metrics = scrollMetrics(scrollRoot);
      stuck = metrics.height - metrics.top - metrics.view > 120;
      const host = jumpHost();
      host.hidden = !stuck;
    };
    trackRoot.addEventListener('scroll', onScroll, { passive: true });
    const keepVisible = () => {
      const host = jumpHost();
      if (stuck) { host.hidden = false; return; }
      host.hidden = true;
      scrollToEnd(scrollRoot, false);
    };

    body.innerHTML = '<span class="chat-thinking" role="status"><i></i><i></i><i></i>正在思考…</span>';
    cursor.hidden = true;
    keepVisible();

    let pending = '';
    let shown = '';
    let finished = false;
    let frame = null;
    let onDone = null;

    function paint() {
      body.innerHTML = renderMarkdown(shown, { math: false });
    }

    function schedule() {
      if (frame) return;
      frame = requestAnimationFrame(() => {
        frame = null;
        const backlog = pending.length - shown.length;
        if (backlog <= 0) {
          paint();
          keepVisible();
          if (finished) finalize();
          return;
        }
        const step = Math.max(3, Math.ceil(backlog / 10));
        shown = pending.slice(0, shown.length + step);
        paint();
        keepVisible();
        schedule();
      });
    }

    function finalize() {
      body.innerHTML = renderMarkdown(shown, { math: true });
      cursor.hidden = true;
      keepVisible();
      trackRoot.removeEventListener('scroll', onScroll);
      if (onDone) onDone();
    }

    return {
      element: article,
      bubble,
      push(chunk) {
        const text = String(chunk ?? '');
        if (!text) return;
        if (!shown && pending === '') cursor.hidden = false;
        pending += text;
        schedule();
      },
      complete(handler) {
        finished = true;
        onDone = handler;
        if (!frame && shown === pending) finalize();
        else schedule();
      },
      fail(message) {
        finished = true;
        trackRoot.removeEventListener('scroll', onScroll);
        cursor.hidden = true;
        const extra = document.createElement('p');
        extra.className = 'chat-error';
        extra.textContent = String(message ?? '');
        body.innerHTML = shown ? renderMarkdown(shown, { math: true }) : '';
        body.append(extra);
        keepVisible();
      },
      text: () => pending,
      addActions(html) {
        const actions = document.createElement('div');
        actions.className = 'chat-actions';
        actions.innerHTML = html;
        bubble.append(actions);
        return actions;
      },
      lock() { cursor.hidden = true; trackRoot.removeEventListener('scroll', onScroll); },
    };
  }

  window.GangyiChat = { renderMarkdown, createAssistantView, copyText, escapeHtml };
})();

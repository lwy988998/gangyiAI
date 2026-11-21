/* 钢一定制AI 共用对话渲染：Markdown 子集 + KaTeX 公式 + 流式打字机视图。 */
// 【逐行批注】 计算右侧结果并更新 (()，改变页面或请求状态。
(() => {
  // 【逐行批注】 启用 JavaScript 严格模式，减少隐式错误。
  'use strict';

  // 私有区字符不会被 HTML 解析或转义破坏，可安全充当占位符。
  // 【逐行批注】 声明前端变量 OPEN，保存本行计算出的数据或对象。
  const OPEN = '\uE000';
  // 【逐行批注】 声明前端变量 CLOSE，保存本行计算出的数据或对象。
  const CLOSE = '\uE001';
  // 【逐行批注】 声明前端变量 TOKEN，保存本行计算出的数据或对象。
  const TOKEN = new RegExp(OPEN + '([MC])(\\d+)' + CLOSE, 'g');

  // 【逐行批注】 声明前端变量 escapeHtml，保存本行计算出的数据或对象。
  const escapeHtml = value => String(value ?? '')
    // 【逐行批注】 执行当前前端表达式：.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')。
    .replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
    // 【逐行批注】 执行当前前端表达式：.replace(/"/g, '&quot;').replace(/'/g, '&#39;');。
    .replace(/"/g, '&quot;').replace(/'/g, '&#39;');

  // 【逐行批注】 声明前端变量 token，保存本行计算出的数据或对象。
  const token = (kind, index) => OPEN + kind + index + CLOSE;
  // 【逐行批注】 声明前端变量 isToken，保存本行计算出的数据或对象。
  const isToken = (text, kind) => new RegExp('^' + OPEN + kind + '\\d+' + CLOSE + '$').test(text);

  // 【逐行批注】 定义函数 restore，封装可复用的前端行为。
  function restore(html, store) {
    // 【逐行批注】 结束当前函数并返回结果：return html.replace(TOKEN, (_match, kind, index) => {。
    return html.replace(TOKEN, (_match, kind, index) => {
      // 【逐行批注】 声明前端变量 list，保存本行计算出的数据或对象。
      const list = kind === 'M' ? store.math : store.code;
      // 【逐行批注】 声明前端变量 value，保存本行计算出的数据或对象。
      const value = list[Number(index)];
      // 【逐行批注】 结束当前函数并返回结果：return value === undefined ? '' : value;。
      return value === undefined ? '' : value;
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 renderMath，封装可复用的前端行为。
  function renderMath(latex, displayMode) {
    // 【逐行批注】 声明前端变量 source，保存本行计算出的数据或对象。
    const source = String(latex ?? '').trim();
    // 【逐行批注】 判断条件 (!source) return '';，成立时执行对应逻辑。
    if (!source) return '';
    // 【逐行批注】 判断条件 (!window.katex || typeof window.katex.renderToString !== 'functi…，成立时执行对应逻辑。
    if (!window.katex || typeof window.katex.renderToString !== 'function')
      // 【逐行批注】 结束当前函数并返回结果：return escapeHtml(displayMode ? '$$' + source + '$$' : '$' + source +…。
      return escapeHtml(displayMode ? '$$' + source + '$$' : '$' + source + '$');
    // 【逐行批注】 执行当前前端表达式：try {。
    try {
      // trust=false 关闭 \href 等外部指令；strict=ignore 让个别不支持的写法退回文本而不是报错。
      // 【逐行批注】 结束当前函数并返回结果：return window.katex.renderToString(source, {。
      return window.katex.renderToString(source, {
        // 【逐行批注】 执行当前前端表达式：displayMode, throwOnError: false, strict: 'ignore', trust: false, output: 'htmlAndMathml',。
        displayMode, throwOnError: false, strict: 'ignore', trust: false, output: 'htmlAndMathml',
      // 【逐行批注】 执行当前前端表达式：});。
      });
    // 【逐行批注】 执行当前前端表达式：} catch (_) {。
    } catch (_) {
      // 【逐行批注】 结束当前函数并返回结果：return escapeHtml(source);。
      return escapeHtml(source);
    // 【逐行批注】 结束当前前端代码块。
    }
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 extractCode，封装可复用的前端行为。
  function extractCode(raw, store) {
    // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
    let text = raw.replace(/```([^\n`]*)\n?([\s\S]*?)```/g, (_match, lang, body) => {
      const label = String(lang).trim();
      const header = label ? '<span class="chat-code-lang">' + escapeHtml(label) + '</span>' : '';
      store.code.push('<div class="chat-code">' + header + '<pre><code>' +
        escapeHtml(String(body).replace(/\n+$/, '')) + '</code></pre></div>');
      return '\n' + token('C', store.code.length - 1) + '\n';
    });
    // 流式过程中代码块尚未闭合：先把剩余内容按代码展示，避免露出裸反引号。
    const open = text.indexOf('```');
    // 【逐行批注】 判断条件 (open !== -1) {，成立时执行对应逻辑。
    if (open !== -1) {
      // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
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
    // 【逐行批注】 计算右侧结果并更新 out，改变页面或请求状态。
    out = out.replace(/\[([^\]\n]+)\]\(([^)\s]+)\)/g, (match, label, url) => {
      // 站内入口只接受单斜杠根路径；禁止反斜杠及控制字符改变浏览器的地址解释。
      // 【逐行批注】 声明前端变量 safeCharacters，保存本行计算出的数据或对象。
      const safeCharacters = !/[\\\u0000-\u0020\u007f]/.test(url);
      // 【逐行批注】 声明前端变量 external，保存本行计算出的数据或对象。
      const external = safeCharacters && /^https?:\/\//i.test(url);
      // 【逐行批注】 声明前端变量 internal，保存本行计算出的数据或对象。
      let internal = safeCharacters && /^\/(?!\/)/.test(url);
      // 【逐行批注】 判断条件 (internal) {，成立时执行对应逻辑。
      if (internal) {
        // 【逐行批注】 计算右侧结果并更新 try { const path，改变页面或请求状态。
        try { const path = decodeURIComponent(url.split(/[?#]/, 1)[0]); internal = /^\/(?!\/)/.test(path) && !/[\\\u0000-\u0020\u007f]/.test(path); }
        // 【逐行批注】 计算右侧结果并更新 catch (_) { internal，改变页面或请求状态。
        catch (_) { internal = false; }
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (!external && !internal) return label;，成立时执行对应逻辑。
      if (!external && !internal) return label;
      // 【逐行批注】 声明前端变量 attributes，保存本行计算出的数据或对象。
      const attributes = external ? ' target="_blank" rel="noopener noreferrer"' : '';
      // 链接单独保护，查询参数中的下划线等字符不能再次被当成 Markdown 语法。
      // 【逐行批注】 调用 inline 执行页面交互或数据处理：store.code.push('<a href="' + url.replace(/"/g, '%22') + '"' + attrib…。
      store.code.push('<a href="' + url.replace(/"/g, '%22') + '"' + attributes + '>' + inline(label, store) + '</a>');
      // 【逐行批注】 结束当前函数并返回结果：return token('C', store.code.length - 1);。
      return token('C', store.code.length - 1);
    // 【逐行批注】 执行当前前端表达式：});。
    });
    // 【逐行批注】 计算右侧结果并更新 out，改变页面或请求状态。
    out = out.replace(/\*\*([^*\n]+)\*\*/g, '<strong>$1</strong>');
    // 【逐行批注】 计算右侧结果并更新 out，改变页面或请求状态。
    out = out.replace(/__([^_\n]+)__/g, '<strong>$1</strong>');
    // 【逐行批注】 计算右侧结果并更新 out，改变页面或请求状态。
    out = out.replace(/(^|[^*\w])\*([^*\n]+)\*/g, '$1<em>$2</em>');
    // 【逐行批注】 计算右侧结果并更新 out，改变页面或请求状态。
    out = out.replace(/(^|[^_\w])_([^_\n]+)_/g, '$1<em>$2</em>');
    // 【逐行批注】 结束当前函数并返回结果：return restore(out, store);。
    return restore(out, store);
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 声明前端变量 LIST_ITEM，保存本行计算出的数据或对象。
  const LIST_ITEM = /^(\s*)([-*+]|\d+[.)])\s+(.*)$/;
  // 【逐行批注】 声明前端变量 BLOCK_START，保存本行计算出的数据或对象。
  const BLOCK_START = /^\s{0,3}(#{1,6}\s|>|-{3,}\s*$|\*{3,}\s*$|_{3,}\s*$)/;

  // 【逐行批注】 定义函数 renderList，封装可复用的前端行为。
  function renderList(lines, start, store, depth) {
    // 【逐行批注】 声明前端变量 first，保存本行计算出的数据或对象。
    const first = LIST_ITEM.exec(lines[start]);
    // 【逐行批注】 声明前端变量 ordered，保存本行计算出的数据或对象。
    const ordered = /\d/.test(first[2]);
    // 【逐行批注】 声明前端变量 baseIndent，保存本行计算出的数据或对象。
    const baseIndent = first[1].length;
    // 【逐行批注】 声明前端变量 items，保存本行计算出的数据或对象。
    const items = [];
    // 【逐行批注】 声明前端变量 index，保存本行计算出的数据或对象。
    let index = start;
    // 【逐行批注】 在条件成立期间循环处理：while (index < lines.length) {。
    while (index < lines.length) {
      // 【逐行批注】 声明前端变量 match，保存本行计算出的数据或对象。
      const match = LIST_ITEM.exec(lines[index]);
      // 【逐行批注】 判断条件 (!match) {，成立时执行对应逻辑。
      if (!match) {
        // 【逐行批注】 声明前端变量 trimmed，保存本行计算出的数据或对象。
        const trimmed = lines[index].trim();
        // 【逐行批注】 判断条件 (items.length && trimmed && lines[index].search(/\S/) > baseInde…，成立时执行对应逻辑。
        if (items.length && trimmed && lines[index].search(/\S/) > baseIndent) {
          // 【逐行批注】 计算右侧结果并更新 items[items.length - 1].text +，改变页面或请求状态。
          items[items.length - 1].text += '\n' + trimmed;
          // 【逐行批注】 执行当前前端表达式：index++;。
          index++;
          // 【逐行批注】 执行当前前端表达式：continue;。
          continue;
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 执行当前前端表达式：break;。
        break;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (match[1].length < baseIndent) break;，成立时执行对应逻辑。
      if (match[1].length < baseIndent) break;
      // 【逐行批注】 判断条件 (match[1].length > baseIndent && items.length && depth < 3) {，成立时执行对应逻辑。
      if (match[1].length > baseIndent && items.length && depth < 3) {
        // 【逐行批注】 声明前端变量 nested，保存本行计算出的数据或对象。
        const nested = renderList(lines, index, store, depth + 1);
        // 【逐行批注】 计算右侧结果并更新 items[items.length - 1].nested，改变页面或请求状态。
        items[items.length - 1].nested = nested.html;
        // 【逐行批注】 计算右侧结果并更新 index，改变页面或请求状态。
        index = nested.next;
        // 【逐行批注】 执行当前前端表达式：continue;。
        continue;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 执行当前前端表达式：items.push({ text: match[3], nested: '' });。
      items.push({ text: match[3], nested: '' });
      // 【逐行批注】 执行当前前端表达式：index++;。
      index++;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 声明前端变量 tag，保存本行计算出的数据或对象。
    const tag = ordered ? 'ol' : 'ul';
    // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
    const body = items.map(item => '<li>' + inline(item.text, store) + item.nested + '</li>').join('');
    // 【逐行批注】 结束当前函数并返回结果：return { html: '<' + tag + '>' + body + '</' + tag + '>', next: index…。
    return { html: '<' + tag + '>' + body + '</' + tag + '>', next: index };
  // 【逐行批注】 结束当前前端代码块。
  }

  // 按未转义的竖线拆分表格，保留行内代码和链接里的竖线。
  function tableCells(line) {
    const text = String(line).trim();
    const cells = [];
    let cell = '';
    let codeTicks = 0;
    let bracketDepth = 0;
    let linkDepth = 0;
    let separators = 0;
    for (let index = 0; index < text.length; index++) {
      const ch = text[index];
      if (ch === '\\' && text[index + 1] === '|') { cell += '|'; index++; continue; }
      if (ch === '`') {
        let end = index + 1;
        while (text[end] === '`') end++;
        const count = end - index;
        if (!codeTicks) codeTicks = count;
        else if (codeTicks === count) codeTicks = 0;
        cell += text.slice(index, end);
        index = end - 1;
        continue;
      }
      if (!codeTicks) {
        if (ch === '[') bracketDepth++;
        else if (ch === ']' && bracketDepth) bracketDepth--;
        else if (ch === '(' && !bracketDepth && (linkDepth || text[index - 1] === ']')) linkDepth++;
        else if (ch === ')' && linkDepth) linkDepth--;
        if (ch === '|' && !bracketDepth && !linkDepth) {
          cells.push(cell.trim()); cell = ''; separators++; continue;
        }
      }
      cell += ch;
    }
    cells.push(cell.trim());
    if (text.startsWith('|')) cells.shift();
    if (text.endsWith('|') && !text.endsWith('\\|') && !codeTicks && !linkDepth) cells.pop();
    return {cells, separators};
  }

  // 表头和分隔线必须列数一致，避免把普通文字误判为表格。
  function tableHeader(lines, index) {
    if (index + 1 >= lines.length) return null;
    const header = tableCells(lines[index]);
    const separator = tableCells(lines[index + 1]);
    if (!header.separators || header.cells.length === 0 || header.cells.length !== separator.cells.length ||
        !separator.cells.every(cell => /^:?-{3,}:?$/.test(cell))) return null;
    return {header: header.cells, alignment: separator.cells.map(cell =>
      cell.startsWith(':') && cell.endsWith(':') ? 'center' : cell.endsWith(':') ? 'right' : 'left')};
  }

  // 【逐行批注】 定义函数 renderBlocks，封装可复用的前端行为。
  function renderBlocks(text, store, depth) {
    // 【逐行批注】 声明前端变量 lines，保存本行计算出的数据或对象。
    const lines = text.split('\n');
    // 【逐行批注】 声明前端变量 out，保存本行计算出的数据或对象。
    const out = [];
    // 【逐行批注】 声明前端变量 index，保存本行计算出的数据或对象。
    let index = 0;
    // 【逐行批注】 在条件成立期间循环处理：while (index < lines.length) {。
    while (index < lines.length) {
      // 【逐行批注】 声明前端变量 line，保存本行计算出的数据或对象。
      const line = lines[index];
      // 【逐行批注】 判断条件 (!line.trim()) { index++; continue; }，成立时执行对应逻辑。
      if (!line.trim()) { index++; continue; }
      // 【逐行批注】 判断条件 (isToken(line.trim(), 'C')) { out.push(restore(line.trim(), stor…，成立时执行对应逻辑。
      if (isToken(line.trim(), 'C')) { out.push(restore(line.trim(), store)); index++; continue; }
      const table = tableHeader(lines, index);
      if (table) {
        const cellHtml = (tag, cell, column) => '<' + tag + ' style="text-align:' + table.alignment[column] + '">' +
          inline(cell || '', store) + '</' + tag + '>';
        const headings = table.header.map((cell, column) => cellHtml('th', cell, column)).join('');
        const rows = [];
        index += 2;
        while (index < lines.length && lines[index].trim()) {
          const row = tableCells(lines[index]);
          if (!row.separators || BLOCK_START.test(lines[index]) || LIST_ITEM.test(lines[index])) break;
          rows.push('<tr>' + table.header.map((_cell, column) => cellHtml('td', row.cells[column], column)).join('') + '</tr>');
          index++;
        }
        out.push('<div class="chat-table" style="overflow-x:auto;max-width:100%"><table><thead><tr>' + headings +
          '</tr></thead><tbody>' + rows.join('') + '</tbody></table></div>');
        continue;
      }
      // 【逐行批注】 声明前端变量 heading，保存本行计算出的数据或对象。
      const heading = /^(#{1,6})\s+(.*)$/.exec(line);
      // 【逐行批注】 判断条件 (heading) {，成立时执行对应逻辑。
      if (heading) {
        // 【逐行批注】 声明前端变量 level，保存本行计算出的数据或对象。
        const level = Math.min(heading[1].length + 2, 6);
        // 【逐行批注】 调用 inline 执行页面交互或数据处理：out.push('<h' + level + '>' + inline(heading[2], store) + '</h' + lev…。
        out.push('<h' + level + '>' + inline(heading[2], store) + '</h' + level + '>');
        // 【逐行批注】 执行当前前端表达式：index++;。
        index++;
        // 【逐行批注】 执行当前前端表达式：continue;。
        continue;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (/^\s{0,3}(-{3,}|\*{3,}|_{3,})\s*$/.test(line)) { out.push('<hr>…，成立时执行对应逻辑。
      if (/^\s{0,3}(-{3,}|\*{3,}|_{3,})\s*$/.test(line)) { out.push('<hr>'); index++; continue; }
      // 【逐行批注】 判断条件 (/^\s{0,3}>\s?/.test(line)) {，成立时执行对应逻辑。
      if (/^\s{0,3}>\s?/.test(line)) {
        // 【逐行批注】 声明前端变量 quote，保存本行计算出的数据或对象。
        const quote = [];
        // 【逐行批注】 在条件成立期间循环处理：while (index < lines.length && /^\s{0,3}>\s?/.test(lines[index])) {。
        while (index < lines.length && /^\s{0,3}>\s?/.test(lines[index])) {
          // 【逐行批注】 执行当前前端表达式：quote.push(lines[index].replace(/^\s{0,3}>\s?/, ''));。
          quote.push(lines[index].replace(/^\s{0,3}>\s?/, ''));
          // 【逐行批注】 执行当前前端表达式：index++;。
          index++;
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 调用 renderBlocks 执行页面交互或数据处理：out.push('<blockquote>' + (depth < 3 ? renderBlocks(quote.join('\n'),…。
        out.push('<blockquote>' + (depth < 3 ? renderBlocks(quote.join('\n'), store, depth + 1)
          // 【逐行批注】 调用 inline 执行页面交互或数据处理：: '<p>' + inline(quote.join('\n'), store) + '</p>') + '</blockquote>'…。
          : '<p>' + inline(quote.join('\n'), store) + '</p>') + '</blockquote>');
        // 【逐行批注】 执行当前前端表达式：continue;。
        continue;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (LIST_ITEM.test(line)) {，成立时执行对应逻辑。
      if (LIST_ITEM.test(line)) {
        // 【逐行批注】 声明前端变量 listed，保存本行计算出的数据或对象。
        const listed = renderList(lines, index, store, depth);
        // 【逐行批注】 执行当前前端表达式：out.push(listed.html);。
        out.push(listed.html);
        // 【逐行批注】 计算右侧结果并更新 index，改变页面或请求状态。
        index = listed.next;
        // 【逐行批注】 执行当前前端表达式：continue;。
        continue;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 声明前端变量 paragraph，保存本行计算出的数据或对象。
      const paragraph = [];
      // 【逐行批注】 在条件成立期间循环处理：while (index < lines.length && lines[index].trim() && !BLOCK_START.test(li…。
      while (index < lines.length && lines[index].trim() && !BLOCK_START.test(lines[index]) && !tableHeader(lines, index) &&
             // 【逐行批注】 调用 isToken 执行页面交互或数据处理：!LIST_ITEM.test(lines[index]) && !isToken(lines[index].trim(), 'C')) {。
             !LIST_ITEM.test(lines[index]) && !isToken(lines[index].trim(), 'C')) {
        // 【逐行批注】 执行当前前端表达式：paragraph.push(lines[index]);。
        paragraph.push(lines[index]);
        // 【逐行批注】 执行当前前端表达式：index++;。
        index++;
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (paragraph.length) out.push('<p>' + inline(paragraph.join('\n'),…，成立时执行对应逻辑。
      if (paragraph.length) out.push('<p>' + inline(paragraph.join('\n'), store) + '</p>');
      // 【逐行批注】 执行前一条件不成立时的分支。
      else index++;
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return out.join('');。
    return out.join('');
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 renderMarkdown，封装可复用的前端行为。
  function renderMarkdown(text, options) {
    // 【逐行批注】 声明前端变量 settings，保存本行计算出的数据或对象。
    const settings = Object.assign({ math: true }, options);
    // 【逐行批注】 声明前端变量 store，保存本行计算出的数据或对象。
    const store = { math: [], code: [] };
    // 【逐行批注】 声明前端变量 guarded，保存本行计算出的数据或对象。
    const guarded = extractMath(extractCode(String(text ?? ''), store), store, settings.math);
    // 【逐行批注】 结束当前函数并返回结果：return renderBlocks(escapeHtml(guarded), store, 0);。
    return renderBlocks(escapeHtml(guarded), store, 0);
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 声明前端变量 reduceMotion，保存本行计算出的数据或对象。
  const reduceMotion = () => window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  // 【逐行批注】 定义函数 scrollMetrics，封装可复用的前端行为。
  function scrollMetrics(root) {
    // 【逐行批注】 判断条件 (root === window || root === document) {，成立时执行对应逻辑。
    if (root === window || root === document) {
      // 【逐行批注】 声明前端变量 doc，保存本行计算出的数据或对象。
      const doc = document.documentElement;
      // 【逐行批注】 结束当前函数并返回结果：return { top: window.scrollY, height: doc.scrollHeight, view: window.…。
      return { top: window.scrollY, height: doc.scrollHeight, view: window.innerHeight };
    // 【逐行批注】 结束当前前端代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return { top: root.scrollTop, height: root.scrollHeight, view: root.c…。
    return { top: root.scrollTop, height: root.scrollHeight, view: root.clientHeight };
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 scrollToEnd，封装可复用的前端行为。
  function scrollToEnd(root, smooth) {
    // 【逐行批注】 声明前端变量 behavior，保存本行计算出的数据或对象。
    const behavior = smooth && !reduceMotion() ? 'smooth' : 'auto';
    // 【逐行批注】 判断条件 (root === window || root === document) window.scrollTo({ top: do…，成立时执行对应逻辑。
    if (root === window || root === document) window.scrollTo({ top: document.documentElement.scrollHeight, behavior });
    // 【逐行批注】 执行前一条件不成立时的分支。
    else root.scrollTo({ top: root.scrollHeight, behavior });
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 定义函数 copyText，封装可复用的前端行为。
  function copyText(text) {
    // 【逐行批注】 判断条件 (navigator.clipboard && navigator.clipboard.writeText) return na…，成立时执行对应逻辑。
    if (navigator.clipboard && navigator.clipboard.writeText) return navigator.clipboard.writeText(text);
    // 【逐行批注】 结束当前函数并返回结果：return new Promise((resolve, reject) => {。
    return new Promise((resolve, reject) => {
      // 【逐行批注】 声明前端变量 area，保存本行计算出的数据或对象。
      const area = document.createElement('textarea');
      // 【逐行批注】 计算右侧结果并更新 area.value，改变页面或请求状态。
      area.value = text;
      // 【逐行批注】 执行当前前端表达式：area.setAttribute('readonly', '');。
      area.setAttribute('readonly', '');
      // 【逐行批注】 计算右侧结果并更新 area.style.position，改变页面或请求状态。
      area.style.position = 'fixed';
      // 【逐行批注】 计算右侧结果并更新 area.style.opacity，改变页面或请求状态。
      area.style.opacity = '0';
      // 【逐行批注】 执行当前前端表达式：document.body.append(area);。
      document.body.append(area);
      // 【逐行批注】 执行当前前端表达式：area.select();。
      area.select();
      // 【逐行批注】 声明前端变量 ok，保存本行计算出的数据或对象。
      const ok = document.execCommand && document.execCommand('copy');
      // 【逐行批注】 执行当前前端表达式：area.remove();。
      area.remove();
      // 【逐行批注】 调用 resolve、reject、Error 执行页面交互或数据处理：ok ? resolve() : reject(new Error('复制失败'));。
      ok ? resolve() : reject(new Error('复制失败'));
    // 【逐行批注】 执行当前前端表达式：});。
    });
  // 【逐行批注】 结束当前前端代码块。
  }

  // 逐字平滑显示：服务器分片到达后进入缓冲，按积压量自适应揭示速度。
  // 【逐行批注】 定义函数 createAssistantView，封装可复用的前端行为。
  function createAssistantView(container, options) {
    // 【逐行批注】 声明前端变量 settings，保存本行计算出的数据或对象。
    const settings = Object.assign({ scrollRoot: null, dark: true }, options);
    // 【逐行批注】 声明前端变量 article，保存本行计算出的数据或对象。
    const article = document.createElement('article');
    // 【逐行批注】 计算右侧结果并更新 article.className，改变页面或请求状态。
    article.className = 'chat-row chat-row-ai';
    // 【逐行批注】 声明前端变量 bubble，保存本行计算出的数据或对象。
    const bubble = document.createElement('div');
    // 【逐行批注】 计算右侧结果并更新 bubble.className，改变页面或请求状态。
    bubble.className = 'chat-bubble chat-bubble-ai' + (settings.dark ? ' chat-bubble-dark' : '');
    // 【逐行批注】 声明前端变量 body，保存本行计算出的数据或对象。
    const body = document.createElement('div');
    // 【逐行批注】 计算右侧结果并更新 body.className，改变页面或请求状态。
    body.className = 'chat-body';
    // 【逐行批注】 声明前端变量 cursor，保存本行计算出的数据或对象。
    const cursor = document.createElement('span');
    // 【逐行批注】 计算右侧结果并更新 cursor.className，改变页面或请求状态。
    cursor.className = 'chat-cursor';
    // 【逐行批注】 执行当前前端表达式：cursor.setAttribute('aria-hidden', 'true');。
    cursor.setAttribute('aria-hidden', 'true');
    // 【逐行批注】 执行当前前端表达式：bubble.append(body, cursor);。
    bubble.append(body, cursor);
    // 【逐行批注】 执行当前前端表达式：article.append(bubble);。
    article.append(bubble);
    // 【逐行批注】 执行当前前端表达式：container.append(article);。
    container.append(article);

    // 【逐行批注】 声明前端变量 scrollRoot，保存本行计算出的数据或对象。
    const scrollRoot = settings.scrollRoot || window;
    // 【逐行批注】 声明前端变量 jump，保存本行计算出的数据或对象。
    let jump = null;
    // 【逐行批注】 定义函数 jumpHost，封装可复用的前端行为。
    function jumpHost() {
      // 【逐行批注】 判断条件 (!jump) {，成立时执行对应逻辑。
      if (!jump) {
        // 【逐行批注】 计算右侧结果并更新 jump，改变页面或请求状态。
        jump = document.createElement('div');
        // 【逐行批注】 计算右侧结果并更新 jump.className，改变页面或请求状态。
        jump.className = 'chat-jump-wrap';
        // 【逐行批注】 计算右侧结果并更新 jump.innerHTML，改变页面或请求状态。
        jump.innerHTML = '<button type="button" class="chat-jump">↓ 回到最新</button>';
        // 【逐行批注】 计算右侧结果并更新 jump.hidden，改变页面或请求状态。
        jump.hidden = true;
        // 【逐行批注】 为页面事件绑定处理逻辑：jump.querySelector('button').addEventListener('click', () => {。
        jump.querySelector('button').addEventListener('click', () => {
          // 【逐行批注】 计算右侧结果并更新 stuck，改变页面或请求状态。
          stuck = false;
          // 【逐行批注】 计算右侧结果并更新 jump.hidden，改变页面或请求状态。
          jump.hidden = true;
          // 【逐行批注】 调用 scrollToEnd 执行页面交互或数据处理：scrollToEnd(scrollRoot, true);。
          scrollToEnd(scrollRoot, true);
        // 【逐行批注】 执行当前前端表达式：});。
        });
      // 【逐行批注】 结束当前前端代码块。
      }
      // 【逐行批注】 判断条件 (container.lastElementChild !== jump) container.append(jump);，成立时执行对应逻辑。
      if (container.lastElementChild !== jump) container.append(jump);
      // 【逐行批注】 结束当前函数并返回结果：return jump;。
      return jump;
    // 【逐行批注】 结束当前前端代码块。
    }

    // 【逐行批注】 声明前端变量 stuck，保存本行计算出的数据或对象。
    let stuck = false;
    // 【逐行批注】 声明前端变量 trackRoot，保存本行计算出的数据或对象。
    const trackRoot = scrollRoot === window || scrollRoot === document ? window : scrollRoot;
    // 【逐行批注】 声明前端变量 onScroll，保存本行计算出的数据或对象。
    const onScroll = () => {
      // 【逐行批注】 声明前端变量 metrics，保存本行计算出的数据或对象。
      const metrics = scrollMetrics(scrollRoot);
      // 【逐行批注】 计算右侧结果并更新 stuck，改变页面或请求状态。
      stuck = metrics.height - metrics.top - metrics.view > 120;
      // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
      const host = jumpHost();
      // 【逐行批注】 计算右侧结果并更新 host.hidden，改变页面或请求状态。
      host.hidden = !stuck;
    // 【逐行批注】 执行当前前端表达式：};。
    };
    // 【逐行批注】 为页面事件绑定处理逻辑：trackRoot.addEventListener('scroll', onScroll, { passive: true });。
    trackRoot.addEventListener('scroll', onScroll, { passive: true });
    // 【逐行批注】 声明前端变量 keepVisible，保存本行计算出的数据或对象。
    const keepVisible = () => {
      // 【逐行批注】 声明前端变量 host，保存本行计算出的数据或对象。
      const host = jumpHost();
      // 【逐行批注】 判断条件 (stuck) { host.hidden = false; return; }，成立时执行对应逻辑。
      if (stuck) { host.hidden = false; return; }
      // 【逐行批注】 计算右侧结果并更新 host.hidden，改变页面或请求状态。
      host.hidden = true;
      // 【逐行批注】 调用 scrollToEnd 执行页面交互或数据处理：scrollToEnd(scrollRoot, false);。
      scrollToEnd(scrollRoot, false);
    // 【逐行批注】 执行当前前端表达式：};。
    };

    // 【逐行批注】 计算右侧结果并更新 body.innerHTML，改变页面或请求状态。
    body.innerHTML = '<span class="chat-thinking" role="status"><i></i><i></i><i></i>等待 AI 返回内容…</span>';
    // 【逐行批注】 计算右侧结果并更新 cursor.hidden，改变页面或请求状态。
    cursor.hidden = true;
    // 【逐行批注】 调用 keepVisible 执行页面交互或数据处理：keepVisible();。
    keepVisible();

    // 【逐行批注】 声明前端变量 pending，保存本行计算出的数据或对象。
    let pending = '';
    // 【逐行批注】 声明前端变量 shown，保存本行计算出的数据或对象。
    let shown = '';
    // 【逐行批注】 声明前端变量 finished，保存本行计算出的数据或对象。
    let finished = false;
    // 【逐行批注】 声明前端变量 frame，保存本行计算出的数据或对象。
    let frame = null;
    // 【逐行批注】 声明前端变量 onDone，保存本行计算出的数据或对象。
    let onDone = null;

    // 【逐行批注】 定义函数 paint，封装可复用的前端行为。
    function paint() {
      // 【逐行批注】 计算右侧结果并更新 body.innerHTML，改变页面或请求状态。
      body.innerHTML = renderMarkdown(shown, { math: false });
    // 【逐行批注】 结束当前前端代码块。
    }

    // 【逐行批注】 定义函数 schedule，封装可复用的前端行为。
    function schedule() {
      // 【逐行批注】 判断条件 (frame) return;，成立时执行对应逻辑。
      if (frame) return;
      // 【逐行批注】 计算右侧结果并更新 frame，改变页面或请求状态。
      frame = requestAnimationFrame(() => {
        // 【逐行批注】 计算右侧结果并更新 frame，改变页面或请求状态。
        frame = null;
        // 【逐行批注】 声明前端变量 backlog，保存本行计算出的数据或对象。
        const backlog = pending.length - shown.length;
        // 【逐行批注】 判断条件 (backlog <= 0) {，成立时执行对应逻辑。
        if (backlog <= 0) {
          // 【逐行批注】 调用 paint 执行页面交互或数据处理：paint();。
          paint();
          // 【逐行批注】 调用 keepVisible 执行页面交互或数据处理：keepVisible();。
          keepVisible();
          // 【逐行批注】 判断条件 (finished) finalize();，成立时执行对应逻辑。
          if (finished) finalize();
          // 【逐行批注】 结束当前函数并返回结果：return;。
          return;
        // 【逐行批注】 结束当前前端代码块。
        }
        // 【逐行批注】 声明前端变量 step，保存本行计算出的数据或对象。
        const step = Math.max(3, Math.ceil(backlog / 10));
        // 【逐行批注】 计算右侧结果并更新 shown，改变页面或请求状态。
        shown = pending.slice(0, shown.length + step);
        // 【逐行批注】 调用 paint 执行页面交互或数据处理：paint();。
        paint();
        // 【逐行批注】 调用 keepVisible 执行页面交互或数据处理：keepVisible();。
        keepVisible();
        // 【逐行批注】 调用 schedule 执行页面交互或数据处理：schedule();。
        schedule();
      // 【逐行批注】 执行当前前端表达式：});。
      });
    // 【逐行批注】 结束当前前端代码块。
    }

    // 【逐行批注】 定义函数 finalize，封装可复用的前端行为。
    function finalize() {
      // 【逐行批注】 计算右侧结果并更新 body.innerHTML，改变页面或请求状态。
      body.innerHTML = renderMarkdown(shown, { math: true });
      // 【逐行批注】 计算右侧结果并更新 cursor.hidden，改变页面或请求状态。
      cursor.hidden = true;
      // 【逐行批注】 调用 keepVisible 执行页面交互或数据处理：keepVisible();。
      keepVisible();
      // 【逐行批注】 执行当前前端表达式：trackRoot.removeEventListener('scroll', onScroll);。
      trackRoot.removeEventListener('scroll', onScroll);
      // 【逐行批注】 判断条件 (onDone) onDone();，成立时执行对应逻辑。
      if (onDone) onDone();
    // 【逐行批注】 结束当前前端代码块。
    }

    // 【逐行批注】 结束当前函数并返回结果：return {。
    return {
      // 【逐行批注】 执行当前前端表达式：element: article,。
      element: article,
      // 【逐行批注】 执行当前前端表达式：bubble,。
      bubble,
      // 【逐行批注】 调用 push 执行页面交互或数据处理：push(chunk) {。
      push(chunk) {
        // 【逐行批注】 声明前端变量 text，保存本行计算出的数据或对象。
        const text = String(chunk ?? '');
        // 【逐行批注】 判断条件 (!text) return;，成立时执行对应逻辑。
        if (!text) return;
        // 【逐行批注】 判断条件 (!shown && pending === '') cursor.hidden = false;，成立时执行对应逻辑。
        if (!shown && pending === '') cursor.hidden = false;
        // 【逐行批注】 计算右侧结果并更新 pending +，改变页面或请求状态。
        pending += text;
        // 【逐行批注】 调用 schedule 执行页面交互或数据处理：schedule();。
        schedule();
      // 【逐行批注】 执行当前前端表达式：},。
      },
      // 【逐行批注】 调用 complete 执行页面交互或数据处理：complete(handler) {。
      complete(handler) {
        // 【逐行批注】 计算右侧结果并更新 finished，改变页面或请求状态。
        finished = true;
        // 【逐行批注】 计算右侧结果并更新 onDone，改变页面或请求状态。
        onDone = handler;
        // 【逐行批注】 判断条件 (!frame && shown === pending) finalize();，成立时执行对应逻辑。
        if (!frame && shown === pending) finalize();
        // 【逐行批注】 执行前一条件不成立时的分支。
        else schedule();
      // 【逐行批注】 执行当前前端表达式：},。
      },
      // 【逐行批注】 调用 fail 执行页面交互或数据处理：fail(message) {。
      fail(message) {
        // 【逐行批注】 计算右侧结果并更新 finished，改变页面或请求状态。
        finished = true;
        // 【逐行批注】 执行当前前端表达式：trackRoot.removeEventListener('scroll', onScroll);。
        trackRoot.removeEventListener('scroll', onScroll);
        // 【逐行批注】 计算右侧结果并更新 cursor.hidden，改变页面或请求状态。
        cursor.hidden = true;
        // 【逐行批注】 声明前端变量 extra，保存本行计算出的数据或对象。
        const extra = document.createElement('p');
        // 【逐行批注】 计算右侧结果并更新 extra.className，改变页面或请求状态。
        extra.className = 'chat-error';
        // 【逐行批注】 计算右侧结果并更新 extra.textContent，改变页面或请求状态。
        extra.textContent = String(message ?? '');
        // 【逐行批注】 计算右侧结果并更新 body.innerHTML，改变页面或请求状态。
        body.innerHTML = shown ? renderMarkdown(shown, { math: true }) : '';
        // 【逐行批注】 执行当前前端表达式：body.append(extra);。
        body.append(extra);
        // 【逐行批注】 调用 keepVisible 执行页面交互或数据处理：keepVisible();。
        keepVisible();
      // 【逐行批注】 执行当前前端表达式：},。
      },
      // 【逐行批注】 计算右侧结果并更新 text: ()，改变页面或请求状态。
      text: () => pending,
      // 【逐行批注】 调用 addActions 执行页面交互或数据处理：addActions(html) {。
      addActions(html) {
        // 【逐行批注】 声明前端变量 actions，保存本行计算出的数据或对象。
        const actions = document.createElement('div');
        // 【逐行批注】 计算右侧结果并更新 actions.className，改变页面或请求状态。
        actions.className = 'chat-actions';
        // 【逐行批注】 计算右侧结果并更新 actions.innerHTML，改变页面或请求状态。
        actions.innerHTML = html;
        // 【逐行批注】 执行当前前端表达式：bubble.append(actions);。
        bubble.append(actions);
        // 【逐行批注】 结束当前函数并返回结果：return actions;。
        return actions;
      // 【逐行批注】 执行当前前端表达式：},。
      },
      // 【逐行批注】 计算右侧结果并更新 lock() { cursor.hidden，改变页面或请求状态。
      lock() { cursor.hidden = true; trackRoot.removeEventListener('scroll', onScroll); },
    // 【逐行批注】 执行当前前端表达式：};。
    };
  // 【逐行批注】 结束当前前端代码块。
  }

  // 【逐行批注】 计算右侧结果并更新 window.GangyiChat，改变页面或请求状态。
  window.GangyiChat = { renderMarkdown, createAssistantView, copyText, escapeHtml };
// 【逐行批注】 执行当前前端表达式：})();。
})();

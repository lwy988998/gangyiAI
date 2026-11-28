// 共享聊天表格：覆盖真实学习回复及不可信单元格。
const assert = require('assert');
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const window = {katex: {renderToString: value => `<span class="test-math">${value}</span>`}};
vm.runInNewContext(fs.readFileSync(path.join(__dirname, '../public/chat-render.js'), 'utf8'), {window, document: {}, navigator: {}});
const render = text => window.GangyiChat.renderMarkdown(text, {math: true});

const message = '先比较两种情况：\n| 项目 | 表达式 | 资料 |\n| :--- | :---: | ---: |\n' +
  '| 第一种 | $x+1$ | [来源](https://example.com/a|b) |\n' +
  '| 第二种 | `a|b` | 左\\|右 |\n接下来继续分析。';
const html = render(message);
assert(html.startsWith('<p>先比较两种情况：</p><div class="chat-table"'));
assert(html.includes('<thead><tr>') && html.includes('<tbody><tr>'));
assert(html.includes('text-align:center') && html.includes('text-align:right'));
assert(html.includes('<span class="test-math">x+1</span>'));
assert(html.includes('<a href="https://example.com/a|b" target="_blank" rel="noopener noreferrer">来源</a>'));
assert(html.includes('<code>a|b</code>') && html.includes('左|右'));
assert(html.endsWith('</table></div><p>接下来继续分析。</p>'));
assert.strictEqual((html.match(/<td /g) || []).length, 6);

const malicious = render('| <img src=x onerror=alert(1)> | 链接 |\n|---|---|\n| **安全** | [坏链接](javascript:alert) |');
assert(malicious.includes('&lt;img src=x onerror=alert(1)&gt;'));
assert(!malicious.includes('<img') && !malicious.includes('href="javascript:'));
assert(malicious.includes('<strong>安全</strong>'));
const plain = render('普通 | 文字\n不是 | 表头分隔线');
assert(!plain.includes('<table>'));
const code = render('```text\n| a | b |\n|---|---|\n```');
assert(!code.includes('<table>') && code.includes('<pre><code>'));
console.log('聊天 Markdown 表格测试通过');

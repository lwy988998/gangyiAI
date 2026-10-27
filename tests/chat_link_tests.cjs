/* 站内工具入口可点击，危险地址不能形成链接，外部链接继续隔离打开。 */
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const window = {};
vm.runInNewContext(fs.readFileSync(path.join(__dirname, '../public/chat-render.js'), 'utf8'), {window});
const render = text => window.GangyiChat.renderMarkdown(text, {math:false});
for (const href of ['/learn?lessonId=lesson_one&sectionId=section_two', '/my-ai/library', '/']) {
  const html = render('[查看原卡片](' + href + ')');
  assert.ok(html.includes('<a href="' + href.replaceAll('&', '&amp;') + '">查看原卡片</a>'), html);
  assert.ok(!html.includes('target='), '站内入口应在本页打开');
}
for (const href of ['https://example.org/a_b_c?q=x_y_z&mode=read', 'http://example.org/docs']) {
  const html = render('[**查看资料**](' + href + ')');
  assert.ok(html.includes('href="' + href.replaceAll('&', '&amp;') + '" target="_blank" rel="noopener noreferrer"'), html);
  assert.ok(html.includes('<strong>查看资料</strong>'), html);
  assert.ok(!html.includes('href="https://example.org/a<em>'), '地址不能被二次强调语法破坏');
}
for (const href of ['//evil.example/path', '/\\evil.example/path', '\\evil.example/path', 'https://example.org\\unsafe',
  '/%2fevil.example/path', '/%5cevil.example/path', '/%0a/evil.example', 'javascript:alert%281%29', 'data:text/html,unsafe',
  'file:///C:/private', 'vbscript:msgbox%281%29', '?next=javascript:unsafe', 'relative/path']) {
  assert.ok(!render('[入口](' + href + ')').includes('<a '), '危险或非根路径地址不应生成链接：' + href);
}
const escaped = render('[<img src=x onerror=alert>](https://example.org/?q="bad"&x=<unsafe>)');
assert.ok(!escaped.includes('<img') && !escaped.includes('<unsafe>'), escaped);
assert.ok(escaped.includes('&lt;img'), escaped);
console.log('站内安全根路径、危险协议与编码边界、外部隔离链接、URL及标签转义检查通过。');

/* 站内工具入口可点击，危险地址不能形成链接，外部链接继续隔离打开。 */
// 【逐行批注】 声明前端变量 assert，保存本行计算出的数据或对象。
const assert = require('node:assert/strict');
// 【逐行批注】 声明前端变量 fs，保存本行计算出的数据或对象。
const fs = require('node:fs');
// 【逐行批注】 声明前端变量 path，保存本行计算出的数据或对象。
const path = require('node:path');
// 【逐行批注】 声明前端变量 vm，保存本行计算出的数据或对象。
const vm = require('node:vm');
// 【逐行批注】 声明前端变量 window，保存本行计算出的数据或对象。
const window = {};
// 【逐行批注】 执行当前前端表达式：vm.runInNewContext(fs.readFileSync(path.join(__dirname, '../public/chat-render.js'), 'utf…。
vm.runInNewContext(fs.readFileSync(path.join(__dirname, '../public/chat-render.js'), 'utf8'), {window});
// 【逐行批注】 声明前端变量 render，保存本行计算出的数据或对象。
const render = text => window.GangyiChat.renderMarkdown(text, {math:false});
// 【逐行批注】 按循环规则遍历或重复处理：for (const href of ['/learn?lessonId=lesson_one&sectionId=section_two', '/…。
for (const href of ['/learn?lessonId=lesson_one&sectionId=section_two', '/my-ai/library', '/']) {
  // 【逐行批注】 声明前端变量 html，保存本行计算出的数据或对象。
  const html = render('[查看原卡片](' + href + ')');
  // 【逐行批注】 执行当前前端表达式：assert.ok(html.includes('<a href="' + href.replaceAll('&', '&amp;') + '">查看原卡片</a>'), htm…。
  assert.ok(html.includes('<a href="' + href.replaceAll('&', '&amp;') + '">查看原卡片</a>'), html);
  // 【逐行批注】 执行当前前端表达式：assert.ok(!html.includes('target='), '站内入口应在本页打开');。
  assert.ok(!html.includes('target='), '站内入口应在本页打开');
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 按循环规则遍历或重复处理：for (const href of ['https://example.org/a_b_c?q=x_y_z&mode=read', 'http:/…。
for (const href of ['https://example.org/a_b_c?q=x_y_z&mode=read', 'http://example.org/docs']) {
  // 【逐行批注】 声明前端变量 html，保存本行计算出的数据或对象。
  const html = render('[**查看资料**](' + href + ')');
  // 【逐行批注】 计算右侧结果并更新 assert.ok(html.includes('href，改变页面或请求状态。
  assert.ok(html.includes('href="' + href.replaceAll('&', '&amp;') + '" target="_blank" rel="noopener noreferrer"'), html);
  // 【逐行批注】 执行当前前端表达式：assert.ok(html.includes('<strong>查看资料</strong>'), html);。
  assert.ok(html.includes('<strong>查看资料</strong>'), html);
  // 【逐行批注】 执行当前前端表达式：assert.ok(!html.includes('href="https://example.org/a<em>'), '地址不能被二次强调语法破坏');。
  assert.ok(!html.includes('href="https://example.org/a<em>'), '地址不能被二次强调语法破坏');
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 按循环规则遍历或重复处理：for (const href of ['//evil.example/path', '/\\evil.example/path', '\\evil…。
for (const href of ['//evil.example/path', '/\\evil.example/path', '\\evil.example/path', 'https://example.org\\unsafe',
  // 【逐行批注】 执行当前前端表达式：'/%2fevil.example/path', '/%5cevil.example/path', '/%0a/evil.example', 'javascript:alert%…。
  '/%2fevil.example/path', '/%5cevil.example/path', '/%0a/evil.example', 'javascript:alert%281%29', 'data:text/html,unsafe',
  // 【逐行批注】 计算右侧结果并更新 'file:///C:/private', 'vbscript:msgbox%281%2…，改变页面或请求状态。
  'file:///C:/private', 'vbscript:msgbox%281%29', '?next=javascript:unsafe', 'relative/path']) {
  // 【逐行批注】 调用 render 执行页面交互或数据处理：assert.ok(!render('[入口](' + href + ')').includes('<a '), '危险或非根路径地址不应…。
  assert.ok(!render('[入口](' + href + ')').includes('<a '), '危险或非根路径地址不应生成链接：' + href);
// 【逐行批注】 结束当前前端代码块。
}
// 【逐行批注】 声明前端变量 escaped，保存本行计算出的数据或对象。
const escaped = render('[<img src=x onerror=alert>](https://example.org/?q="bad"&x=<unsafe>)');
// 【逐行批注】 执行当前前端表达式：assert.ok(!escaped.includes('<img') && !escaped.includes('<unsafe>'), escaped);。
assert.ok(!escaped.includes('<img') && !escaped.includes('<unsafe>'), escaped);
// 【逐行批注】 执行当前前端表达式：assert.ok(escaped.includes('&lt;img'), escaped);。
assert.ok(escaped.includes('&lt;img'), escaped);
// 【逐行批注】 执行当前前端表达式：console.log('站内安全根路径、危险协议与编码边界、外部隔离链接、URL及标签转义检查通过。');。
console.log('站内安全根路径、危险协议与编码边界、外部隔离链接、URL及标签转义检查通过。');

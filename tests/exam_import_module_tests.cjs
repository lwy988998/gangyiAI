const fs = require('fs');
const vm = require('vm');
const source = fs.readFileSync(require('path').join(__dirname, '..', 'public', 'exam-import.js'), 'utf8');
const context = {module: {exports: {}}, URL: {createObjectURL: () => 'blob:test', revokeObjectURL: () => {}}};
vm.runInNewContext(source, context, {filename: 'exam-import.js'});
const api = context.module.exports;
async function main() {
  const image = {name: 'page-1.png', type: 'image/png', size: 1200};
  const pdf = {name: 'paper.pdf', type: 'application/pdf', size: 3000, async arrayBuffer() { return new ArrayBuffer(4); }};
  const state = api.createExamImportState();
  const added = api.addExamFiles(state, [image, {name: 'bad.exe', type: 'application/octet-stream', size: 5}]);
  if (added.added.length !== 1 || added.rejected.length !== 1) throw new Error('文件类型校验未按预期工作');
  if (state.pages[0].type !== 'image') throw new Error('图片类型未保存');
  const fakePdfJs = {getDocument() { return {promise: Promise.resolve({numPages: 3})}; }};
  const pdfResult = await api.addExamFilesAsync(state, [pdf], {pdfjsLib: fakePdfJs});
  if (pdfResult.added.length !== 3 || pdfResult.rejected.length) throw new Error('多页 PDF 未拆成可整理的独立页面');
  if (state.pages.slice(1).some((page, index) => page.pageNumber !== index + 1 || page.pageCount !== 3)) throw new Error('PDF 页码关联错误');
  if (state.pages[1].sourceId !== state.pages[2].sourceId || state.pages[1].sourceFile !== pdf) throw new Error('PDF 页面与原始文件关联丢失');
  const firstId = state.pages[0].id;
  if (!api.moveExamPage(state, firstId, 1) || state.pages[1].id !== firstId) throw new Error('页面顺序调整失败');
  if (!api.setRecognitionText(state, firstId, '手写答案：B', 0.64)) throw new Error('识别文本编辑失败');
  if (state.pages[1].recognitionText !== '手写答案：B' || state.pages[1].recognitionStatus !== 'edited') throw new Error('识别层未更新');
  api.setRecognitionText(state, firstId, '答案修订', null);
  if (state.pages[1].recognitionConfidence !== null) throw new Error('未知置信度被错误转成零');
  if (api.validateExamFile({name: 'too-big.png', type: 'image/png', size: api.DEFAULT_MAX_FILE_BYTES + 1}).code !== 'too_large') throw new Error('大小校验失败');
  console.log('exam import module smoke tests passed');
}

main().catch(error => { console.error(error); process.exitCode = 1; });

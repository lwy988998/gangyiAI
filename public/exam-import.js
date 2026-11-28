/* 试卷讲解导入模块：只负责本地文件、页面顺序、预览与识别层编辑。 */
(() => {
  'use strict';

  const IMAGE_TYPES = new Set(['image/jpeg', 'image/png', 'image/webp', 'image/gif', 'image/bmp', 'image/tiff']);
  const PDF_TYPE = 'application/pdf';
  const DEFAULT_MAX_FILE_BYTES = 50 * 1024 * 1024;
  const DEFAULT_MAX_PAGES = 120;
  let nextPageId = 1;

  function fileType(file) {
    const type = String(file?.type || '').toLowerCase();
    if (IMAGE_TYPES.has(type)) return 'image';
    if (type === PDF_TYPE || /\.pdf$/i.test(String(file?.name || ''))) return 'pdf';
    return '';
  }

  function validateExamFile(file, options = {}) {
    const maxBytes = Number.isFinite(options.maxBytes) ? options.maxBytes : DEFAULT_MAX_FILE_BYTES;
    if (!file || typeof file !== 'object') return {ok: false, code: 'invalid', message: '文件对象无效。'};
    const type = fileType(file);
    if (!type) return {ok: false, code: 'unsupported', message: '仅支持常见图片和 PDF 文件。'};
    if (Number(file.size || 0) <= 0) return {ok: false, code: 'empty', message: '文件为空，无法读取。'};
    if (Number(file.size || 0) > maxBytes) return {ok: false, code: 'too_large', message: `文件不能超过 ${Math.round(maxBytes / 1024 / 1024)} MB。`};
    return {ok: true, type};
  }

  function makePage(file, type, previewUrl) {
    const page = {
      id: `exam-page-${nextPageId++}`,
      sourceId: `exam-source-${nextPageId}`,
      sourceFile: file,
      file,
      type,
      name: String(file.name || '未命名文件'),
      previewUrl: '',
      pageNumber: 1,
      pageCount: type === 'pdf' ? null : 1,
      recognitionText: '',
      recognitionConfidence: null,
      recognitionStatus: 'pending',
      error: ''
    };
    if (previewUrl !== undefined) {
      page.previewUrl = previewUrl;
    } else if (typeof URL !== 'undefined' && typeof URL.createObjectURL === 'function') {
      page.previewUrl = URL.createObjectURL(file);
    }
    return page;
  }

  const pdfDocumentCache = new WeakMap();

  async function openPdf(file, pdfjs) {
    if (pdfDocumentCache.has(file)) return pdfDocumentCache.get(file);
    if (!pdfjs || typeof pdfjs.getDocument !== 'function') throw new Error('未加载本地 PDF.js');
    const pending = pdfjs.getDocument({data: await file.arrayBuffer(), cMapUrl: '/vendor/pdfjs/cmaps/', cMapPacked: true, standardFontDataUrl: '/vendor/pdfjs/standard_fonts/'}).promise;
    pdfDocumentCache.set(file, pending);
    return pending;
  }

  async function addExamFilesAsync(state, files, options = {}) {
    const added = [];
    const rejected = [];
    for (const file of Array.from(files || [])) {
      const result = validateExamFile(file, state);
      if (!result.ok) {
        rejected.push({name: String(file?.name || '未命名文件'), ...result});
        continue;
      }
      if (result.type !== 'pdf') {
        if (state.pages.length >= state.maxPages) {
          rejected.push({name: String(file.name || '未命名文件'), code: 'page_limit', message: `单套试卷最多支持 ${state.maxPages} 页。`});
          continue;
        }
        const page = makePage(file, result.type);
        state.pages.push(page);
        if (!state.sources.includes(file)) state.sources.push(file);
        added.push(page);
        continue;
      }
      try {
        const pdf = await openPdf(file, options.pdfjsLib || (typeof window !== 'undefined' ? window.pdfjsLib : null));
        if (pdf.numPages < 1 || state.pages.length + pdf.numPages > state.maxPages) {
          rejected.push({name: String(file.name || '未命名文件'), code: 'page_limit', message: `单套试卷最多支持 ${state.maxPages} 页。`});
          continue;
        }
        const sourceUrl = typeof URL !== 'undefined' && URL.createObjectURL ? URL.createObjectURL(file) : '';
        const sourceId = `exam-source-${nextPageId++}`;
        if (!state.sources.includes(file)) state.sources.push(file);
        for (let number = 1; number <= pdf.numPages; number += 1) {
          const page = makePage(file, 'pdf', sourceUrl);
          page.sourceId = sourceId;
          page.pageNumber = number;
          page.pageCount = pdf.numPages;
          page.pdfDocument = pdf;
          state.pages.push(page);
          added.push(page);
        }
      } catch (error) {
        rejected.push({name: String(file.name || '未命名文件'), code: 'pdf_unavailable', message: '无法读取 PDF 分页；请确认本机已加载 PDF.js 或改用清晰的页面图片。'});
      }
    }
    if (!state.selectedPageId && state.pages[0]) state.selectedPageId = state.pages[0].id;
    state.status = added.length ? 'ready' : state.status;
    state.message = rejected.length ? `已加入 ${added.length} 页，${rejected.length} 项未加入。` : '';
    return {added, rejected};
  }

  function createExamImportState(options = {}) {
    return {
      maxBytes: Number.isFinite(options.maxBytes) ? options.maxBytes : DEFAULT_MAX_FILE_BYTES,
      maxPages: Number.isFinite(options.maxPages) ? options.maxPages : DEFAULT_MAX_PAGES,
      pages: [],
      sources: [],
      selectedPageId: '',
      status: 'idle',
      message: ''
    };
  }

  function revokePage(page) {
    if (page?.previewUrl && typeof URL !== 'undefined' && typeof URL.revokeObjectURL === 'function') {
      URL.revokeObjectURL(page.previewUrl);
    }
  }

  function addExamFiles(state, files) {
    const added = [];
    const rejected = [];
    for (const file of Array.from(files || [])) {
      const result = validateExamFile(file, state);
      if (!result.ok) {
        rejected.push({name: String(file?.name || '未命名文件'), ...result});
        continue;
      }
      if (result.type === 'pdf') {
        rejected.push({name: String(file.name || '未命名文件'), code: 'async_pdf', message: 'PDF 需要逐页读取；请使用 addFiles 接口。'});
        continue;
      }
      if (state.pages.length >= state.maxPages) {
        rejected.push({name: String(file.name || '未命名文件'), code: 'page_limit', message: `单套试卷最多支持 ${state.maxPages} 页。`});
        continue;
      }
      const page = makePage(file, result.type);
      state.pages.push(page);
      if (!state.sources.includes(file)) state.sources.push(file);
      added.push(page);
    }
    if (!state.selectedPageId && state.pages[0]) state.selectedPageId = state.pages[0].id;
    state.status = added.length ? 'ready' : state.status;
    state.message = rejected.length ? `已加入 ${added.length} 个文件，${rejected.length} 个文件未加入。` : '';
    return {added, rejected};
  }

  function moveExamPage(state, pageId, targetIndex) {
    const from = state.pages.findIndex(page => page.id === pageId);
    const to = Math.max(0, Math.min(state.pages.length - 1, Number(targetIndex)));
    if (from < 0 || !Number.isInteger(to) || from === to) return false;
    const [page] = state.pages.splice(from, 1);
    state.pages.splice(to, 0, page);
    return true;
  }

  function removeExamPage(state, pageId) {
    const index = state.pages.findIndex(page => page.id === pageId);
    if (index < 0) return false;
    const [page] = state.pages.splice(index, 1);
    const stillUsed = state.pages.some(item => item.previewUrl && item.previewUrl === page.previewUrl);
    if (!stillUsed) revokePage(page);
    if (state.selectedPageId === pageId) state.selectedPageId = state.pages[index]?.id || state.pages[index - 1]?.id || '';
    state.status = state.pages.length ? 'ready' : 'idle';
    return true;
  }

  function setRecognitionText(state, pageId, text, confidence = null) {
    const page = state.pages.find(item => item.id === pageId);
    if (!page) return false;
    page.recognitionText = String(text ?? '');
    page.recognitionConfidence = confidence !== null && confidence !== undefined && Number.isFinite(Number(confidence)) ? Number(confidence) : null;
    page.recognitionStatus = 'edited';
    page.error = '';
    return true;
  }

  async function pageImageDataUrl(page, options = {}) {
    if (page.type === 'image') {
      const file = page.sourceFile || page.file;
      const maxEdge = Number.isFinite(options.maxEdge) ? options.maxEdge : 1800;
      if (typeof createImageBitmap === 'function') {
        const bitmap = await createImageBitmap(file);
        const scale = Math.min(1, maxEdge / Math.max(bitmap.width, bitmap.height));
        const canvas = document.createElement('canvas');
        canvas.width = Math.max(1, Math.round(bitmap.width * scale));
        canvas.height = Math.max(1, Math.round(bitmap.height * scale));
        const context = canvas.getContext('2d');
        context.fillStyle = '#fff';
        context.fillRect(0, 0, canvas.width, canvas.height);
        context.drawImage(bitmap, 0, 0, canvas.width, canvas.height);
        bitmap.close?.();
        return canvas.toDataURL('image/jpeg', 0.9);
      }
      return await new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onload = () => resolve(String(reader.result || ''));
        reader.onerror = () => reject(new Error('图片读取失败'));
        reader.readAsDataURL(file);
      });
    }
    const pdf = page.pdfDocument || await openPdf(page.sourceFile || page.file, options.pdfjsLib || (typeof window !== 'undefined' ? window.pdfjsLib : null));
    const pdfPage = await pdf.getPage(page.pageNumber);
    const base = pdfPage.getViewport({scale: 1});
    const maxEdge = Number.isFinite(options.maxEdge) ? options.maxEdge : 1800;
    const scale = Math.min(1.8, maxEdge / Math.max(base.width, base.height));
    const viewport = pdfPage.getViewport({scale});
    const canvas = document.createElement('canvas');
    canvas.width = Math.ceil(viewport.width);
    canvas.height = Math.ceil(viewport.height);
    await pdfPage.render({canvasContext: canvas.getContext('2d'), viewport}).promise;
    return canvas.toDataURL('image/jpeg', 0.9);
  }

  function pageLabel(page, index) {
    return page.type === 'pdf' ? `${page.name} · 第 ${page.pageNumber} / ${page.pageCount} 页` : `${page.name} · 第 ${index + 1} 页`;
  }

  function createElement(tag, className, text) {
    const element = document.createElement(tag);
    if (className) element.className = className;
    if (text !== undefined) element.textContent = text;
    return element;
  }

  function mountExamImporter(root, options = {}) {
    if (!root || typeof document === 'undefined') throw new Error('试卷导入需要有效的页面容器。');
    const state = createExamImportState(options);
    const pdfjs = options.pdfjsLib || window.pdfjsLib || null;
    const callbacks = {
      onSubmit: typeof options.onSubmit === 'function' ? options.onSubmit : () => {},
      onRecognitionEdit: typeof options.onRecognitionEdit === 'function' ? options.onRecognitionEdit : () => {},
      onPageSelect: typeof options.onPageSelect === 'function' ? options.onPageSelect : () => {}
    };
    let destroyed = false;
    let submitting = false;

    root.classList.add('exam-import');
    root.replaceChildren();
    const form = createElement('div', 'exam-import__shell');
    const heading = createElement('div', 'exam-import__heading');
    heading.append(createElement('div', 'exam-import__eyebrow', '高中全学科 · 试卷讲解'), createElement('h2', '', options.title || '上传试卷'));
    const help = createElement('p', 'exam-import__help', '支持多页图片和 PDF。原始文件保留不变，识别文字可以随时校正。');
    heading.append(help);

    const dropzone = createElement('label', 'exam-import__dropzone');
    dropzone.setAttribute('for', 'exam-import-file-input');
    dropzone.append(createElement('strong', '', '选择试卷文件'), createElement('span', '', '也可以把图片或 PDF 拖到这里'));
    const input = createElement('input');
    input.type = 'file';
    input.id = 'exam-import-file-input';
    input.multiple = true;
    input.accept = 'image/*,.pdf,application/pdf';
    input.setAttribute('capture', 'environment');
    input.className = 'exam-import__input';
    dropzone.append(input);
    const status = createElement('p', 'exam-import__status');
    const submit = createElement('button', 'exam-import__submit', '开始识别并学习');
    submit.type = 'button';
    submit.disabled = true;
    submit.addEventListener('click', () => {
      if (!state.pages.length || submit.disabled) return;
      submitting = true;
      submit.disabled = true;
      setStatus('正在准备原始页面，完成后会自动开始第一题。');
      Promise.resolve(callbacks.onSubmit(state.pages.slice(), state)).catch(error => {
        submitting = false;
        renderList();
        setStatus(error?.message || '试卷提交失败，请重试。', 'error');
      });
    });

    const layout = createElement('div', 'exam-import__layout');
    const list = createElement('ol', 'exam-import__list');
    const preview = createElement('div', 'exam-import__preview');
    const previewEmpty = createElement('div', 'exam-import__empty', '加入试卷后，这里会显示原始页面预览。');
    preview.append(previewEmpty);
    layout.append(list, preview);
    form.append(heading, dropzone, status, submit, layout);
    root.append(form);

    function setStatus(text, kind = '') {
      status.textContent = text || '';
      status.dataset.kind = kind;
    }

    async function renderPdf(page, container) {
      try {
        const pdf = page.pdfDocument || await openPdf(page.sourceFile || page.file, pdfjs);
        const pdfPage = await pdf.getPage(page.pageNumber);
        const base = pdfPage.getViewport({scale: 1});
        const viewport = pdfPage.getViewport({scale: Math.min(1.15, 1400 / Math.max(base.width, base.height))});
        const canvas = createElement('canvas', 'exam-import__pdf-page');
        canvas.width = Math.ceil(viewport.width);
        canvas.height = Math.ceil(viewport.height);
        canvas.setAttribute('aria-label', `${page.name} 第 ${page.pageNumber} 页，共 ${page.pageCount} 页`);
        container.append(canvas);
        await pdfPage.render({canvasContext: canvas.getContext('2d'), viewport}).promise;
      } catch (error) {
        page.error = 'PDF 分页预览失败';
        container.append(createElement('p', 'exam-import__notice exam-import__notice--error', 'PDF 暂时无法分页预览；请稍后重试或改用清晰的页面图片。'));
      }
    }

    function renderList() {
      list.replaceChildren();
      state.pages.forEach((page, index) => {
        const item = createElement('li', `exam-import__item${page.id === state.selectedPageId ? ' is-selected' : ''}`);
        const button = createElement('button', 'exam-import__item-main');
        button.type = 'button';
        button.append(createElement('span', 'exam-import__item-index', String(index + 1)), createElement('span', 'exam-import__item-name', pageLabel(page, index)));
        button.addEventListener('click', () => { state.selectedPageId = page.id; callbacks.onPageSelect(page); render(); });
        const controls = createElement('span', 'exam-import__item-actions');
        const up = createElement('button', 'exam-import__icon-button', '上移');
        up.type = 'button'; up.disabled = index === 0; up.setAttribute('aria-label', `将第 ${index + 1} 页上移`); up.addEventListener('click', event => { event.stopPropagation(); moveExamPage(state, page.id, index - 1); render(); });
        const down = createElement('button', 'exam-import__icon-button', '下移');
        down.type = 'button'; down.disabled = index === state.pages.length - 1; down.setAttribute('aria-label', `将第 ${index + 1} 页下移`); down.addEventListener('click', event => { event.stopPropagation(); moveExamPage(state, page.id, index + 1); render(); });
        const remove = createElement('button', 'exam-import__icon-button exam-import__icon-button--danger', '移除');
        remove.type = 'button'; remove.setAttribute('aria-label', `移除第 ${index + 1} 页`); remove.addEventListener('click', event => { event.stopPropagation(); removeExamPage(state, page.id); render(); });
        controls.append(up, down, remove); item.append(button, controls); list.append(item);
      });
      if (!state.pages.length) list.append(createElement('li', 'exam-import__list-empty', '尚未加入文件'));
      submit.disabled = !state.pages.length || submitting;
    }

    function renderRecognition(page, container) {
      if (!page) return;
      const section = createElement('section', 'exam-import__recognition');
      section.append(createElement('div', 'exam-import__section-label', '识别文字（可编辑）'));
      const textarea = createElement('textarea', 'exam-import__textarea');
      textarea.rows = 6; textarea.value = page.recognitionText;
      textarea.placeholder = '识别完成后，题干、材料和手写答案会出现在这里。你可以直接修改。';
      textarea.addEventListener('input', () => { setRecognitionText(state, page.id, textarea.value, page.recognitionConfidence); callbacks.onRecognitionEdit(page, textarea.value); });
      section.append(textarea);
      const note = page.recognitionConfidence === null ? '等待识别；原图不受编辑影响。' : `识别置信度 ${Math.round(page.recognitionConfidence * 100)}%，低置信内容请核对。`;
      section.append(createElement('p', 'exam-import__recognition-note', note));
      container.append(section);
    }

    async function renderPreview(page) {
      preview.replaceChildren();
      if (!page) { preview.append(previewEmpty); return; }
      const header = createElement('div', 'exam-import__preview-header');
      header.append(createElement('strong', '', page.name), createElement('span', 'exam-import__preview-meta', page.type === 'pdf' ? 'PDF 原件' : '原始图片'));
      const canvas = createElement('div', 'exam-import__preview-canvas');
      if (page.type === 'pdf') await renderPdf(page, canvas);
      else { const image = createElement('img', 'exam-import__image'); image.src = page.previewUrl; image.alt = `${page.name} 原始试卷页面`; canvas.append(image); }
      preview.append(header, canvas);
      renderRecognition(page, preview);
    }

    function render() {
      if (destroyed) return;
      renderList();
      const page = state.pages.find(item => item.id === state.selectedPageId);
      renderPreview(page);
      if (state.message) setStatus(state.message, 'warning');
    }

    async function handleFiles(files) {
      setStatus('正在读取文件…');
      const result = await addExamFilesAsync(state, files, {pdfjsLib: pdfjs});
      render();
      if (result.rejected.length && !result.added.length) setStatus(result.rejected.map(item => `${item.name}：${item.message}`).join('；'), 'error');
      else if (result.rejected.length) setStatus(result.rejected.map(item => `${item.name}：${item.message}`).join('；'), 'warning');
    }

    input.addEventListener('change', () => { void handleFiles(input.files); input.value = ''; });
    ['dragenter', 'dragover'].forEach(eventName => dropzone.addEventListener(eventName, event => { event.preventDefault(); dropzone.classList.add('is-dragging'); }));
    ['dragleave', 'drop'].forEach(eventName => dropzone.addEventListener(eventName, event => { event.preventDefault(); dropzone.classList.remove('is-dragging'); }));
    dropzone.addEventListener('drop', event => { void handleFiles(event.dataTransfer?.files || []); });
    render();

    return {
      state,
      addFiles: handleFiles,
      selectPage(pageId) { if (state.pages.some(page => page.id === pageId)) { state.selectedPageId = pageId; render(); } },
      setRecognition(pageId, text, confidence) { setRecognitionText(state, pageId, text, confidence); render(); },
      getPages() { return state.pages.slice(); },
      getOriginalFiles() { return state.sources.slice(); },
      getPageImageDataUrl(pageId) {
        const page = state.pages.find(item => item.id === pageId);
        return page ? pageImageDataUrl(page, {pdfjsLib: pdfjs}) : Promise.reject(new Error('试卷页面不存在'));
      },
      submit() {
        if (!submit.disabled) submit.click();
      },
      destroy() {
        destroyed = true;
        const urls = new Set(state.pages.map(page => page.previewUrl).filter(Boolean));
        urls.forEach(url => URL.revokeObjectURL(url));
        root.replaceChildren();
      }
    };
  }

  const api = {DEFAULT_MAX_FILE_BYTES, DEFAULT_MAX_PAGES, validateExamFile, createExamImportState, addExamFiles, addExamFilesAsync, moveExamPage, removeExamPage, setRecognitionText, pageImageDataUrl, mountExamImporter};
  if (typeof window !== 'undefined') window.GangyiExamImport = api;
  if (typeof module !== 'undefined' && module.exports) module.exports = api;
})();

(() => {
  'use strict';
  const $ = (id) => document.getElementById(id);
  const root = $('exam-page');
  const request = async (url, options = {}) => {
    const response = await fetch(url, {cache: 'no-store', ...options, headers: {'Content-Type': 'application/json', ...(options.headers || {})}});
    const data = await response.json().catch(() => ({}));
    if (!response.ok) throw new Error(data.error || '请求未完成');
    return data;
  };
  const parameters = new URLSearchParams(location.search);
  const id = parameters.get('id');
  const pageTitle = document.createElement('h1');
  pageTitle.textContent = '试卷讲解';
  const learningTools = window.GangyiWorkspace?.compactToolbar(pageTitle);
  const sourceToggle = document.createElement('button');
  sourceToggle.type = 'button';
  const narrow = window.matchMedia('(max-width: 900px)');
  let session = null;
  let stopWatch = null;
  let watchingTaskId = '';
  let sourceCollapsed = true;
  let syncSource;
  narrow.addEventListener('change', () => syncSource?.());
  let sending = false;
  let sourceConfirmations = [];
  const renderedMessages = new Map();
  const draftKey = `gangyi:exam-draft:${id || ''}`;
  const failureKey = `gangyi:exam-failure:${id || ''}`;
  let draft = '';
  let failureInfo = null;
  try { draft = localStorage.getItem(draftKey) || ''; } catch (_) { /* 存储不可用时仍保留当前页面草稿。 */ }
  try {
    const saved = JSON.parse(localStorage.getItem(failureKey) || 'null');
    if (saved && typeof saved.message === 'string' && typeof saved.taskId === 'string' && typeof saved.requestId === 'string') failureInfo = saved;
  } catch (_) { /* 存储不可用或旧数据损坏时仅使用本页状态。 */ }
  const saveDraft = value => {
    draft = value;
    try {
      if (value) localStorage.setItem(draftKey, value);
      else localStorage.removeItem(draftKey);
    } catch (_) { /* 私密模式或存储空间不足时继续使用内存草稿。 */ }
  };
  const saveFailure = value => {
    failureInfo = value;
    try {
      if (value) localStorage.setItem(failureKey, JSON.stringify(value));
      else localStorage.removeItem(failureKey);
    } catch (_) { /* 存储不可用时仍在当前页面显示失败信息。 */ }
  };
  const statusLabel = status => ({recognizing: '正在识别', ready: '可以继续学习', failed: '处理未完成'})[status] || '正在处理';
  const recognitionIncomplete = () => {
    const current = session?.progress?.current;
    return Number.isInteger(current) && current < (session?.pages?.length || 0);
  };
  const requestId = prefix => `${prefix}-${crypto.randomUUID?.() || Date.now()}`;
  const disposeMessage = record => {
    for (const item of record.texts.values()) window.ExamRichOutput?.destroy(item.node);
    for (const item of record.artifacts.values()) window.ExamRichOutput?.destroy(item.node);
    record.article.remove();
  };
  const renderMessages = container => {
    const active = new Set();
    const artifacts = new Map((session.artifacts || []).filter(item => item && typeof item.id === 'string').map(item => [item.id, item]));
    for (const [index, message] of (session.messages || []).entries()) {
      const key = typeof message.id === 'string' && message.id ? message.id : `legacy:${index}:${message.role}:${message.content || ''}`;
      if (active.has(key)) continue;
      active.add(key);
      let record = renderedMessages.get(key);
      const role = message.role === 'user' ? 'user' : 'assistant';
      const content = typeof message.content === 'string' ? message.content : '';
      if (!record) {
        const article = document.createElement('article');
        record = {article, texts: new Map(), artifacts: new Map()};
        renderedMessages.set(key, record);
      }
      container.append(record.article);
      record.article.className = `exam-chat-message exam-chat-message--${role}`;
      const visible = new Set();
      const visibleText = new Set();
      const allowed = role === 'assistant' && Array.isArray(message.artifactIds) ? message.artifactIds : [];
      const parts = role === 'assistant' && Array.isArray(message.artifactParts) ? message.artifactParts : [{type: 'text', text: content}];
      const appendArtifact = artifactId => {
        const artifact = artifacts.get(artifactId);
        if (!allowed.includes(artifactId) || !artifact || artifact.originMessageId !== message.id || visible.has(artifactId)) return;
        visible.add(artifactId);
        const signature = JSON.stringify([artifact.type, artifact.revision, artifact.content]);
        let item = record.artifacts.get(artifactId);
        if (!item) {
          const node = document.createElement('div');
          node.className = 'exam-message-artifact';
          node.dataset.artifactId = artifactId;
          item = {node, signature: null};
          record.artifacts.set(artifactId, item);
        }
        record.article.append(item.node);
        if (item.signature !== signature) {
          window.ExamRichOutput?.renderArtifact(item.node, artifact);
          item.signature = signature;
        }
      };
      for (const [partIndex, part] of parts.entries()) {
        if (part?.type === 'artifact') { appendArtifact(part.id); continue; }
        if (part?.type !== 'text' || typeof part.text !== 'string') continue;
        const textKey = String(partIndex);
        visibleText.add(textKey);
        let item = record.texts.get(textKey);
        if (!item) { item = {node: document.createElement('div'), signature: null}; record.texts.set(textKey, item); }
        record.article.append(item.node);
        const signature = JSON.stringify([role, part.text]);
        if (item.signature !== signature) {
          window.ExamRichOutput?.destroy(item.node);
          if (role === 'assistant' && window.ExamRichOutput) window.ExamRichOutput.render(item.node, part.text);
          else item.node.textContent = part.text;
          item.signature = signature;
        }
      }
      for (const artifactId of allowed) appendArtifact(artifactId);
      for (const [textKey, item] of record.texts) if (!visibleText.has(textKey)) {
        window.ExamRichOutput?.destroy(item.node);
        item.node.remove();
        record.texts.delete(textKey);
      }
      for (const [artifactId, item] of record.artifacts) if (!visible.has(artifactId)) {
        window.ExamRichOutput?.destroy(item.node);
        item.node.remove();
        record.artifacts.delete(artifactId);
      }
    }
    for (const [key, record] of renderedMessages) if (!active.has(key)) {
      disposeMessage(record);
      renderedMessages.delete(key);
    }
  };
  let questionEdits = 0;
  const saveQuestionPatch = async (questionId, patch) => {
    ++questionEdits;
    try {
      session = await request(`/api/exam-explanations/${encodeURIComponent(session.id)}`, {
        method: 'PATCH', body: JSON.stringify({questionId, ...patch})
      });
      render();
    } finally { --questionEdits; }
  };
  const renderQuestions = (container) => {
    container.replaceChildren();
    for (const question of session.questions || []) {
      const card = document.createElement('article');
      card.className = `exam-question${question.recognitionStatus === 'uncertain' ? ' is-uncertain' : ''}`;
      const strong = document.createElement('strong'); strong.textContent = `${question.number || '题目'} · ${question.subject || '高中'}`;
      const text = document.createElement('div'); text.textContent = question.stem || '题干识别中…';
      card.append(strong, text);
      const addEditor = (host, field, label, initialValue, toPayload = value => value) => {
        const editor = document.createElement('textarea'); editor.setAttribute('aria-label', `修改第${question.number || ''}题${label}`);
        editor.value = initialValue; editor.placeholder = `校正识别出的${label}`;
        editor.addEventListener('blur', async () => {
          if (editor.value === initialValue) return;
          try { await saveQuestionPatch(question.id, {[field]: toPayload(editor.value)}); }
          catch (error) { alert(error.message); }
        });
        host.append(editor);
      };
      addEditor(card, 'text', '题干', question.stem || '');
      if (typeof question.handwrittenAnswer === 'string' && question.handwrittenAnswer.trim()) {
        const answer = document.createElement('p'); answer.className = 'exam-handwriting';
        answer.textContent = `已识别手写答案：${question.handwrittenAnswer}`;
        const note = document.createElement('p'); note.className = 'exam-handwriting-note';
        note.textContent = '手写识别可能有误，请核对原图与识别文字。';
        card.append(answer, note);
      }
      const correction = document.createElement('details'); correction.className = 'exam-question-correction';
      const correctionLabel = document.createElement('summary'); correctionLabel.textContent = '校正材料、小问和手写答案'; correction.append(correctionLabel);
      const correctionHost = document.createElement('div'); correctionHost.className = 'exam-question-correction__fields';
      card.append(correction);
      for (const [field, label] of [['subject', '学科'], ['type', '题型']]) {
        const editor = document.createElement('textarea'); editor.setAttribute('aria-label', `修改第${question.number || ''}题${label}`); editor.value = question[field] || ''; editor.placeholder = `校正${label}`;
        editor.addEventListener('blur', async () => { if (editor.value === (question[field] || '')) return; try { await saveQuestionPatch(question.id, {[field]: editor.value}); } catch (error) { alert(error.message); } });
        correctionHost.append(editor);
      }
      const materials = Array.isArray(question.materials) ? question.materials : [];
      if (materials.every(item => typeof item === 'string')) {
        addEditor(correctionHost, 'materials', '材料', materials.join('\n'), value => value.split('\n').map(item => item.trim()).filter(Boolean));
      } else {
        const materialText = materials.map(item => {
          if (typeof item === 'string') return item;
          if (!item || typeof item !== 'object' || Array.isArray(item)) return '此材料格式暂不支持显示，请查看原卷。';
          const label = typeof item.label === 'string' ? item.label : '';
          const text = ['text', 'content', 'stem'].map(key => item[key]).find(value => typeof value === 'string') || '';
          return [label, text].filter(Boolean).join(' ') || '此材料格式暂不支持显示，请查看原卷。';
        }).join('\n\n');
        const editor = document.createElement('textarea'); editor.setAttribute('aria-label', `查看第${question.number || ''}题材料`);
        editor.value = materialText; editor.readOnly = true;
        const note = document.createElement('small'); note.textContent = '该材料包含结构化信息，暂不支持在此编辑，请对照原卷。';
        correctionHost.append(editor, note);
      }
      const subquestions = Array.isArray(question.subquestions) ? question.subquestions : [];
      if (subquestions.every(item => typeof item === 'string')) {
        addEditor(correctionHost, 'subquestions', '小问列表', subquestions.join('\n'), value => value.split('\n').map(item => item.trim()).filter(Boolean));
        const note = document.createElement('small'); note.textContent = '每行一个小问，可增删或调整顺序。'; correctionHost.append(note);
      }
      for (const [index, subquestion] of subquestions.entries()) {
        const isObject = subquestion && typeof subquestion === 'object' && !Array.isArray(subquestion);
        const label = isObject && typeof subquestion.label === 'string' ? subquestion.label : `小问 ${index + 1}`;
        const canEdit = typeof subquestion === 'string' || (isObject && typeof subquestion.text === 'string');
        const content = typeof subquestion === 'string' ? subquestion :
          isObject ? ['text', 'stem'].map(key => subquestion[key]).find(value => typeof value === 'string') || '' : '';
        const field = document.createElement('label'); field.textContent = label;
        const editor = document.createElement('textarea'); editor.setAttribute('aria-label', `${canEdit ? '修改' : '查看'}第${question.number || ''}题第${index + 1}小问`);
        editor.value = content; editor.readOnly = !canEdit;
        if (!canEdit) editor.placeholder = '此小问格式暂不支持编辑，请查看原卷。';
        if (canEdit) editor.addEventListener('blur', async () => {
          if (editor.value === content) return;
          try {
            await saveQuestionPatch(question.id, {subquestionIndex: index, subquestionText: editor.value});
          } catch (error) { alert(error.message); }
        });
        field.append(editor); correctionHost.append(field);
      }
      addEditor(correctionHost, 'handwrittenAnswer', '手写答案', question.handwrittenAnswer || '');
      correction.append(correctionHost);
      const study = document.createElement('div'); study.className = 'exam-question-study';
      const currentPages = (question.pageIndexes || [question.pageIndex]).filter(index => Number.isInteger(index) && index >= 0 && index < session.pages.length);
      const pageForm = document.createElement('form'); pageForm.className = 'exam-question-study__pages';
      const pageLabel = document.createElement('label'); pageLabel.textContent = '原卷页码';
      const pageInput = document.createElement('input'); pageInput.value = currentPages.map(index => index + 1).join(', ');
      pageInput.required = true; pageInput.maxLength = 400; pageInput.placeholder = '例如 1, 2'; pageInput.setAttribute('aria-label', `第${question.number || ''}题原卷页码`);
      pageLabel.append(pageInput);
      const savePages = document.createElement('button'); savePages.type = 'submit'; savePages.textContent = '保存页码';
      const status = document.createElement('small'); status.setAttribute('aria-live', 'polite');
      pageForm.append(pageLabel, savePages);
      pageForm.addEventListener('submit', async event => {
        event.preventDefault();
        if (questionEdits) { status.textContent = '正在保存校正，请稍候。'; return; }
        const entered = pageInput.value.trim().split(/[,，\s]+/);
        const indexes = entered.map(value => Number(value) - 1);
        if (entered.some(value => !/^\d+$/.test(value)) || indexes.some(index => index < 0 || index >= session.pages.length)) {
          status.textContent = `请输入 1 到 ${session.pages.length} 的页码，多个页码用逗号分开。`;
          return;
        }
        savePages.disabled = true;
        try { await saveQuestionPatch(question.id, {pageIndexes: indexes}); }
        catch (error) { status.textContent = error.message; }
        finally { savePages.disabled = false; }
      });
      correctionHost.append(pageForm);
      const ready = ['ready', 'edited'].includes(question.recognitionStatus) && Boolean(question.stem?.trim()) &&
        currentPages.length > 0 && session.progress?.current === session.pages.length;
      if (ready) {
        const scope = document.createElement('select'); scope.setAttribute('aria-label', '选择要问导师的小问');
        scope.add(new Option('整道题', '-1'));
        for (const [index, point] of subquestions.entries()) {
          const option = new Option(`小问 ${index + 1}`, String(index));
          const pointText = typeof point === 'string' ? point : point?.text || point?.stem || point?.content;
          option.disabled = typeof pointText !== 'string' || !pointText.trim();
          scope.add(option);
        }
        const ask = document.createElement('a'); ask.textContent = '核对后问导师';
        const updateTarget = () => {
          const link = new URLSearchParams({examQuestion: JSON.stringify({
            examId: session.id, questionId: question.id, revision: question.sourceRevision ?? 0, part: Number(scope.value)
          })});
          if (parameters.get('courseId')) link.set('courseId', parameters.get('courseId'));
          ask.href = '/ask?' + link;
        };
        scope.addEventListener('change', updateTarget);
        ask.addEventListener('click', event => {
          if (questionEdits || pageInput.value !== currentPages.map(index => index + 1).join(', ')) {
            event.preventDefault(); status.textContent = '请先保存校正后的页码和题目。';
          }
        });
        updateTarget();
        study.append(scope, ask);
      } else status.textContent = '请核对题干和原卷页码后交给导师。';
      study.append(status);
      card.append(study);
      if (question.recognitionStatus === 'uncertain') { const note = document.createElement('small'); note.textContent = '识别存在不确定，后续可直接编辑题干。'; card.append(note); }
      container.append(card);
    }
  };
  const openSourceReview = async source => {
    const dialog = document.createElement('dialog'); dialog.className = 'exam-source-review';
    const heading = document.createElement('h2'); heading.textContent = '核对来源原文';
    const close = document.createElement('button'); close.type = 'button'; close.className = 'exam-source-review__close'; close.textContent = '关闭';
    close.addEventListener('click', () => dialog.close());
    const header = document.createElement('header'); header.append(heading, close);
    const status = document.createElement('p'); status.setAttribute('role', 'status'); status.textContent = '正在读取候选来源原文…';
    const content = document.createElement('div'); content.className = 'exam-source-review__content';
    dialog.append(header, status, content); document.body.append(dialog);
    dialog.addEventListener('close', () => dialog.remove(), {once: true});
    dialog.showModal();
    try {
      const snapshot = await request(`/api/exam-explanations/${encodeURIComponent(session.id)}/source-snapshots`, {
        method: 'POST', body: JSON.stringify({url: source.url}),
      });
      if (!dialog.isConnected) return;
      if (!Array.isArray(snapshot.segments) || !snapshot.segments.length || !snapshot.id) throw new Error('未获取到可核对的来源原文。');
      status.textContent = '请对照原卷，分别选择题目和答案的原文证据；确认由用户作出，非官方。';
      const origin = document.createElement('a'); origin.href = snapshot.finalUrl || source.url;
      if (!/^https?:\/\//i.test(origin.href)) origin.href = source.url;
      origin.target = '_blank'; origin.rel = 'noopener noreferrer'; origin.textContent = '打开原文网页';
      content.append(origin);
      const questionLabel = document.createElement('label'); questionLabel.textContent = '对应试卷题目';
      const question = document.createElement('select');
      question.append(new Option('请选择题目', ''));
      for (const item of session.questions || []) question.append(new Option(`第 ${item.number || item.id} 题`, item.id));
      questionLabel.append(question);
      content.append(questionLabel);
      const fields = [];
      for (const [fieldIndex, label] of ['题目原文证据', '答案或评分依据原文证据'].entries()) {
        const group = document.createElement('div'); group.className = 'exam-source-review__evidence';
        const segmentLabel = document.createElement('label'); segmentLabel.textContent = label;
        const segment = document.createElement('select'); segment.append(new Option('请选择原文段落', ''));
        for (const item of snapshot.segments) if (typeof item.id === 'string' && typeof item.text === 'string') {
          segment.append(new Option(item.text.replace(/\s+/g, ' ').slice(0, 120), item.id));
        }
        segmentLabel.append(segment);
        const quoteLabel = document.createElement('label'); quoteLabel.textContent = fieldIndex === 0 ? '题目引用（至少 12 字节，最多 8000 字节）' : '答案引用（最多 8000 字节）';
        const quote = document.createElement('textarea'); quote.disabled = true; quote.rows = 5;
        segment.addEventListener('change', () => {
          const selected = snapshot.segments.find(item => item.id === segment.value);
          quote.value = selected?.text || ''; quote.disabled = !selected;
        });
        quoteLabel.append(quote); group.append(segmentLabel, quoteLabel); content.append(group);
        fields.push({segment, quote});
      }
      const verifyLabel = document.createElement('label'); verifyLabel.className = 'exam-source-review__confirm';
      const verify = document.createElement('input'); verify.type = 'checkbox';
      verifyLabel.append(verify, document.createTextNode('我已对照原卷和原文，确认这两段对应所选题目和答案。'));
      const submit = document.createElement('button'); submit.type = 'button'; submit.className = 'exam-button'; submit.textContent = '确认对应关系';
      content.append(verifyLabel, submit);
      submit.addEventListener('click', async () => {
        if (!question.value || !verify.checked || fields.some(field => !field.segment.value || !field.quote.value.trim())) {
          status.textContent = '请选择题目及两段证据，并勾选确认。'; return;
        }
        for (const [index, field] of fields.entries()) {
          const segment = snapshot.segments.find(item => item.id === field.segment.value);
          if (!segment?.text.includes(field.quote.value)) { status.textContent = '引用内容必须逐字来自所选原文段落。'; return; }
          const bytes = new TextEncoder().encode(field.quote.value).length;
          if ((index === 0 && bytes < 12) || bytes > 8000) { status.textContent = index === 0 ? '题目引用须为 12 至 8000 字节，请截取合适长度的原文。' : '答案引用不能超过 8000 字节。'; return; }
        }
        submit.disabled = true;
        try {
          await request(`/api/exam-explanations/${encodeURIComponent(session.id)}/source-confirmations`, {method: 'POST', body: JSON.stringify({
            questionId: question.value, snapshotId: snapshot.id, questionSegmentId: fields[0].segment.value,
            questionQuote: fields[0].quote.value, answerSegmentId: fields[1].segment.value, answerQuote: fields[1].quote.value, confirmed: true,
          })});
          dialog.close(); await refreshSources();
        } catch (error) { status.textContent = error.message; submit.disabled = false; }
      });
    } catch (error) { if (dialog.isConnected) status.textContent = `${error.message}，请关闭后重试。`; }
  };
  const renderSources = (host) => {
    host.replaceChildren();
    const candidates = Array.isArray(session.sources) ? session.sources : [];
    const sources = [...candidates];
    for (const confirmation of sourceConfirmations) {
      if (confirmation.sourceUrl && !sources.some(source => source.url === confirmation.sourceUrl))
        sources.push({url: confirmation.sourceUrl, title: '历史来源（不在当前候选中）', historical: true});
    }
    if (!sources.length) return;
    const section = document.createElement('div'); section.className = 'exam-source-links';
    const heading = document.createElement('span'); heading.textContent = '联网候选来源（未核实，不作为正式评分依据）'; section.append(heading);
    for (const source of sources) {
      if (!source.url || !/^https?:\/\//i.test(source.url)) continue;
      const row = document.createElement('div'); row.className = 'exam-source-result';
      const link = document.createElement('a'); link.className = 'exam-source-link'; link.href = source.url; link.target = '_blank'; link.rel = 'noopener noreferrer';
      link.textContent = source.title || source.url; row.append(link);
      if (!source.historical) {
        const review = document.createElement('button'); review.type = 'button'; review.textContent = '核对原文';
        review.addEventListener('click', () => void openSourceReview(source)); row.append(review);
      }
      for (const confirmation of sourceConfirmations.filter(item => item.sourceUrl === source.url)) {
        const line = document.createElement('div'); line.className = 'exam-source-confirmation';
        const number = (session.questions || []).find(item => item.id === confirmation.questionId)?.number || confirmation.questionId;
        const label = document.createElement('span');
        label.textContent = `第 ${number} 题：${confirmation.revoked ? '已撤销' : confirmation.gradingEligible ? '用户确认，非官方' : '已失效，请重新核对'}`;
        line.append(label);
        if (!confirmation.revoked) {
          const revoke = document.createElement('button'); revoke.type = 'button'; revoke.textContent = '撤销确认';
          revoke.addEventListener('click', async () => {
            revoke.disabled = true;
            try {
              await request(`/api/exam-explanations/${encodeURIComponent(session.id)}/source-revocations`, {method: 'POST', body: JSON.stringify({questionId: confirmation.questionId, sourceUrl: source.url})});
              await refreshSources();
            } catch (error) { label.textContent = error.message; revoke.disabled = false; }
          });
          line.append(revoke);
        }
        row.append(line);
      }
      section.append(row);
    }
    host.append(section);
  };
  const renderSource = () => {
    const pages = $('exam-source-pages'); pages.replaceChildren();
    for (const [index, page] of (session.pages || []).entries()) {
      const button = document.createElement('a'); button.className = 'exam-source-page';
      button.href = `/api/exam-explanations/${encodeURIComponent(session.id)}/pages/${index}`;
      button.target = '_blank'; button.rel = 'noopener noreferrer';
      button.setAttribute('aria-label', `打开${page.name || `第${index + 1}页`}原图`);
      const image = document.createElement('img'); image.alt = page.name || `第${index + 1}页`; image.src = `/api/exam-explanations/${encodeURIComponent(session.id)}/pages/${index}`;
      const name = document.createElement('span'); name.textContent = `${index + 1}. ${page.name || '试卷页'}`; button.append(image, name); pages.append(button);
    }
    $('exam-source-status').textContent = `${session.pages?.length || 0} 页 · ${statusLabel(session.status)}`;
  };
  const nearLatest = region => region.scrollHeight - region.scrollTop - region.clientHeight < 80;
  const render = () => {
    syncSource = null;
    const previousMessages = root.querySelector('.exam-chat-messages');
    const previousTop = previousMessages && !nearLatest(previousMessages) ? previousMessages.scrollTop : null;
    root.replaceChildren();
    if (!session) { root.innerHTML = '<div class="exam-page__loading">正在读取试卷讲解…</div>'; return; }
    if (session.error) { root.innerHTML = `<p class="exam-error"></p>`; root.querySelector('p').textContent = session.error; return; }
    const layout = document.createElement('div'); layout.className = 'exam-workspace';
    const chat = document.createElement('section'); chat.className = 'exam-chat-panel';
    chat.innerHTML = '<header class="exam-chat-head"><div><h1></h1><p>围绕当前试卷自然对话，随时可以跳题、追问或直接看答案。</p></div><span class="exam-status"></span></header><div class="exam-chat-messages"></div><div class="exam-input-feedback" role="status" aria-live="polite"></div><form class="exam-chat-input"><textarea aria-label="输入学习消息" placeholder="回答当前题目，或说“给提示”“下一题”“直接讲答案”…"></textarea><button type="submit">发送</button></form>';
    layout.append(chat); root.append(layout);
    const title = session.title || '试卷讲解';
    pageTitle.textContent = title;
    chat.querySelector('h1').textContent = title;
    chat.querySelector('.exam-status').textContent = statusLabel(session.status);
    if (learningTools) chat.querySelector('.exam-chat-head').remove();
    const sourcesHost = document.createElement('div'); sourcesHost.className = 'exam-source-results';
    chat.insertBefore(sourcesHost, chat.querySelector('.exam-chat-messages'));
    renderSources(sourcesHost);
    const messages = chat.querySelector('.exam-chat-messages'); renderMessages(messages);
    if (learningTools) {
      messages.tabIndex = 0;
      messages.setAttribute('role', 'region');
      messages.setAttribute('aria-label', '试卷讲解对话');
    }
    const form = chat.querySelector('form');
    const textarea = form.querySelector('textarea'); textarea.value = draft;
    textarea.addEventListener('input', () => saveDraft(textarea.value));
    const recognizing = session.status === 'recognizing';
    const recognitionFailed = session.status === 'failed' && recognitionIncomplete();
    const feedback = chat.querySelector('.exam-input-feedback');
    if (failureInfo) {
      const failure = document.createElement('div'); failure.className = 'exam-task-failure'; failure.setAttribute('role', 'alert');
      const reason = document.createElement('span'); reason.textContent = `本次请求未完成：${failureInfo.message}`;
      failure.append(reason);
      const recoveryStatus = document.createElement('span'); recoveryStatus.className = 'exam-task-failure__status'; recoveryStatus.setAttribute('aria-live', 'polite');
      const failedMessage = (session.messages || []).find(message => message.role === 'user' && message.requestId === failureInfo.requestId);
      if (failedMessage && typeof failedMessage.content === 'string') {
        const recover = document.createElement('button'); recover.type = 'button'; recover.textContent = '重新编辑原问题';
        recover.addEventListener('click', () => {
          if (draft && draft !== failedMessage.content) { recoveryStatus.textContent = '输入框已有草稿，请先处理草稿后再重新编辑原问题。'; return; }
          saveDraft(failedMessage.content); textarea.value = failedMessage.content; textarea.focus();
          recoveryStatus.textContent = '原问题已放入输入框，编辑后可手动发送。';
        });
        failure.append(recover);
      }
      failure.append(recoveryStatus);
      chat.insertBefore(failure, feedback);
    }
    if (recognizing) feedback.textContent = '正在识别试卷，可以先输入问题；识别完成后即可发送。';
    if (recognitionFailed) {
      feedback.classList.add('is-error');
      feedback.textContent = '试卷识别未完成，原卷和已识别的部分已保存。';
      const retry = document.createElement('button'); retry.type = 'button'; retry.className = 'exam-retry'; retry.textContent = '继续识别';
      retry.addEventListener('click', async () => {
        retry.disabled = true;
        try {
          const result = await request(`/api/exam-explanations/${encodeURIComponent(session.id)}/recognition-retries`, {method: 'POST', body: JSON.stringify({requestId: requestId('exam-retry')})});
          session = result.session || result.chat || result;
          render();
          watchTask(result.task?.id || session.activeTaskId);
        } catch (error) { retry.disabled = false; feedback.firstChild.textContent = `${error.message}，可重试识别。`; }
      });
      feedback.append(' ', retry);
    }
    form.querySelector('button').disabled = sending || recognizing || recognitionFailed;
    form.addEventListener('submit', async event => {
      event.preventDefault();
      const text = textarea.value.trim();
      if (!text || sending || recognizing || recognitionFailed) return;
      sending = true;
      form.querySelector('button').disabled = true;
      try {
        const result = await request(`/api/exam-explanations/${encodeURIComponent(session.id)}/messages`, {method: 'POST', body: JSON.stringify({requestId: requestId('exam-msg'), text})});
        saveFailure(null);
        if (draft.trim() === text) saveDraft('');
        session = result.chat || await request(`/api/exam-explanations/${encodeURIComponent(session.id)}`);
        sending = false;
        render();
        watchTask(result.task?.id || result.taskId);
      } catch (error) {
        sending = false;
        const currentForm = root.querySelector('.exam-chat-input');
        if (currentForm) currentForm.querySelector('button').disabled = false;
        const feedback = root.querySelector('.exam-input-feedback');
        if (feedback) { feedback.classList.add('is-error'); feedback.textContent = `${error.message}，输入内容已保留，可重试。`; }
      }
    });
    const source = document.createElement('aside'); source.className = 'exam-source-panel'; source.innerHTML = '<header class="exam-source-head"><strong>原始试卷</strong><span id="exam-source-status" class="exam-status"></span></header><div class="exam-source-body"><div id="exam-source-pages" class="exam-source-pages"></div><div id="exam-original-files" class="exam-original-files"></div><div class="exam-question-list" id="exam-question-list"></div></div>';
    source.id = 'exam-source-panel';
    source.querySelector('.exam-source-body').id = 'exam-source-content';
    sourceToggle.setAttribute('aria-controls', source.id);
    sourceToggle.hidden = false;
    const closeSource = document.createElement('button');
    closeSource.type = 'button';
    closeSource.textContent = '收起原卷';
    source.querySelector('.exam-source-head').append(closeSource);
    const sourceDialog = document.createElement('dialog');
    sourceDialog.className = 'exam-source-dialog';
    sourceDialog.setAttribute('aria-label', '原卷与题目核对');
    layout.append(source, sourceDialog);
    const updateSource = () => {
      const open = !sourceCollapsed;
      source.hidden = !open;
      layout.classList.toggle('is-source-open', open && !narrow.matches);
      sourceToggle.textContent = open ? '收起原卷' : '查看原卷和核对题目';
      sourceToggle.setAttribute('aria-expanded', String(open));
      if (open && narrow.matches) {
        sourceDialog.append(source);
        if (!sourceDialog.open) sourceDialog.showModal();
      } else {
        if (sourceDialog.open) sourceDialog.close();
        layout.insertBefore(source, sourceDialog);
      }
    };
    closeSource.onclick = () => { sourceCollapsed = true; updateSource(); learningTools?.toggle.focus(); };
    sourceToggle.onclick = () => {
      learningTools?.close();
      sourceCollapsed = !sourceCollapsed;
      updateSource();
      if (!sourceCollapsed) closeSource.focus({preventScroll: true});
    };
    sourceDialog.addEventListener('cancel', event => { event.preventDefault(); closeSource.click(); });
    if (!learningTools) layout.prepend(sourceToggle);
    syncSource = updateSource;
    updateSource();
    renderSource(); renderQuestions($('exam-question-list')); renderOriginalFiles();
    if (learningTools) messages.scrollTop = previousTop ?? messages.scrollHeight;
  };
  const load = async () => { try { session = id ? await request(`/api/exam-explanations/${encodeURIComponent(id)}`) : null; render(); void refreshSources(); if (session?.activeTaskId && session.activeTaskId !== watchingTaskId) watchTask(session.activeTaskId); } catch (error) { session = {error: error.message}; render(); } };
  const refreshSources = async () => {
    if (!session || session.error || document.hidden) return;
    const currentId = session.id;
    try {
      const [freshResult, confirmationsResult] = await Promise.allSettled([
        request(`/api/exam-explanations/${encodeURIComponent(currentId)}/sources`),
        request(`/api/exam-explanations/${encodeURIComponent(currentId)}/source-confirmations`),
      ]);
      if (!session || session.id !== currentId) return;
      const fresh = freshResult.status === 'fulfilled' ? freshResult.value : {sources: session.sources};
      const confirmations = confirmationsResult.status === 'fulfilled' ? confirmationsResult.value : {confirmations: sourceConfirmations};
      const next = Array.isArray(fresh.sources) ? fresh.sources : [];
      const nextConfirmations = Array.isArray(confirmations.confirmations) ? confirmations.confirmations : [];
      if (JSON.stringify(next) === JSON.stringify(session.sources || []) && JSON.stringify(nextConfirmations) === JSON.stringify(sourceConfirmations)) return;
      session.sources = next;
      sourceConfirmations = nextConfirmations;
      const host = root.querySelector('.exam-source-results');
      if (host) renderSources(host);
    } catch (_) { /* 后台检索不可用时保留已有内容和输入。 */ }
  };
  const renderOriginalFiles = () => { const host = $('exam-original-files'); if (!host) return; host.replaceChildren(); for (const [index, file] of (session.files || []).entries()) { const link = document.createElement('a'); link.href = `/api/exam-explanations/${encodeURIComponent(session.id)}/files/${index}`; link.target = '_blank'; link.rel = 'noopener'; link.textContent = `打开原文件：${file.name || `文件 ${index + 1}`}`; host.append(link); } };
  const watchTask = taskId => {
    if (!taskId || !window.GangyiAgent?.watch) return;
    if (stopWatch) stopWatch();
    watchingTaskId = taskId;
    const watchedRequestId = typeof session?.activeRequestId === 'string' ? session.activeRequestId : '';
    const messages = document.querySelector('.exam-chat-messages');
    const following = messages && nearLatest(messages);
    const bubble = document.createElement('article'); bubble.className = 'exam-chat-message exam-chat-message--assistant is-streaming'; bubble.textContent = '正在读取试卷内容…'; messages?.append(bubble);
    if (following && learningTools) messages.scrollTop = messages.scrollHeight;
    let text = '';
    stopWatch = window.GangyiAgent.watch(taskId, {
      onEvent(event) {
        if (event.type !== 'delta' || event.field !== 'message') return false;
        const follow = messages && nearLatest(messages);
        text += event.text || '';
        bubble.textContent = text;
        bubble.dataset.hasContent = 'true';
        if (follow && learningTools) messages.scrollTop = messages.scrollHeight;
        return true;
      },
      onState(task) {
        if (watchingTaskId !== taskId || ['pending', 'running'].includes(task.status)) return;
        if (task.status === 'failed') {
          const reason = typeof task.failure?.message === 'string' && task.failure.message ? task.failure.message :
            typeof task.error === 'string' && task.error ? task.error : 'AI 请求未完成。';
          saveFailure({taskId, requestId: watchedRequestId, message: reason.slice(0, 4000)});
        } else if (task.status === 'ready') saveFailure(null);
        watchingTaskId = ''; void load();
      },
      onConnectionError(message) { bubble.textContent = text || message; },
    });
  };
  if (!id) {
    root.innerHTML = '<div class="exam-upload-shell"><section aria-label="最近的试卷讲解"><h2>最近的试卷</h2><div id="exam-session-list" class="exam-session-list"></div></section><section id="exam-upload" aria-labelledby="exam-upload-title"><h2 id="exam-upload-title" tabindex="-1">导入试卷</h2><div id="exam-import-root"></div></section></div>';
    const upload = $('exam-upload');
    if (learningTools) {
      upload.hidden = parameters.get('import') !== '1';
      const entry = document.createElement('button');
      entry.type = 'button';
      entry.textContent = '导入试卷';
      entry.setAttribute('aria-controls', upload.id);
      entry.onclick = () => {
        learningTools.close();
        upload.hidden = false;
        $('exam-upload-title').focus();
      };
      learningTools.append(entry);
    }
    request('/api/exam-explanations').then(result => { const list = $('exam-session-list'); for (const item of result.sessions || []) { const link = document.createElement('a'); link.className = 'exam-session-link'; link.href = `/exam-explanations?id=${encodeURIComponent(item.id)}`; const title = document.createElement('strong'); title.textContent = item.title || '试卷讲解'; const state = document.createElement('span'); state.textContent = statusLabel(item.status); link.append(title, state); list.append(link); } if (!list.childElementCount) list.textContent = '还没有试卷，可从操作菜单导入。'; }).catch(() => {});
    import('/vendor/pdfjs/pdf.min.mjs').then(pdfjs => {
      pdfjs.GlobalWorkerOptions.workerSrc = '/vendor/pdfjs/pdf.worker.min.mjs';
      let controller;
      controller = window.GangyiExamImport.mountExamImporter($('exam-import-root'), {pdfjsLib: pdfjs, maxPages: 80, onSubmit: async pages => {
        if (!pages.length) return;
        const sourceFiles = controller.getOriginalFiles();
        const originals = [];
        for (const file of sourceFiles) originals.push({name: file.name, mimeType: file.type,
          dataUrl: await new Promise((resolve, reject) => { const reader = new FileReader(); reader.onload = () => resolve(String(reader.result || '')); reader.onerror = reject; reader.readAsDataURL(file); })});
        const imagePages = [];
        for (const [index, page] of pages.entries()) imagePages.push({name: page.name, mimeType: 'image/jpeg', order: index,
          originalFileIndex: sourceFiles.findIndex(file => file === page.sourceFile), pageNumber: page.pageNumber,
          dataUrl: await controller.getPageImageDataUrl(page.id)});
        try {
          const title = (sourceFiles[0]?.name || '高中试卷').replace(/\.[^.]+$/, '');
          const created = await request('/api/exam-explanations', {method: 'POST', body: JSON.stringify({requestId: requestId('exam-upload'),
            title, pages: imagePages, originalFiles: originals})});
          location.href = `/exam-explanations?id=${encodeURIComponent(created.id)}`;
        } catch (error) { const status = document.querySelector('.exam-import__status'); if (status) { status.textContent = error.message; status.dataset.kind = 'error'; } throw error; }
      }});
    }).catch(error => { root.textContent = `本地 PDF 页面组件未加载：${error.message}`; });
  } else {
    if (learningTools) {
      const sessions = document.createElement('a');
      sessions.href = '/exam-explanations';
      sessions.textContent = '返回试卷列表';
      const upload = document.createElement('a');
      upload.href = '/exam-explanations?import=1';
      upload.textContent = '导入新试卷';
      sourceToggle.hidden = true;
      learningTools.append(sourceToggle, sessions, upload);
    }
    void load();
    const sourceTimer = window.setInterval(refreshSources, 5000);
    window.addEventListener('pagehide', () => window.clearInterval(sourceTimer), {once: true});
  }
})();

/* 深度课堂只呈现已保存的教学内容，图示交互在本地完成。 */
(() => {
  'use strict';
  const mount = document.getElementById('deep-course-app');
  if (!mount) return;
  const api = window.GangyiAgent;
  const query = new URLSearchParams(location.search);
  const allowedScope = ['lessonId', 'courseId', 'phaseIndex', 'topicIndex', 'topicId'];
  const scope = Object.fromEntries(allowedScope.filter(key => query.has(key)).map(key => [key, query.get(key)]));
  let scopeError = '';
  for (const key of ['phaseIndex', 'topicIndex']) if (Object.hasOwn(scope, key)) {
    const raw = scope[key]; const value = /^\d+$/.test(raw) ? Number(raw) : NaN;
    if (!Number.isSafeInteger(value) || value < 1 || value > 100000) scopeError = '课程位置无效，请从课程目录重新选择知识点。';
    else scope[key] = value;
  }
  let saved = null, lesson = null, currentView = 'lecture', currentSlide = 0, busy = false, sending = false;
  let watchingTask = '', stopWatching = null, refreshPromise = null, refreshAgain = false, activeEpoch = 0;
  let taskMessage = '', lastTask = null, pendingMessages = [], enteringLesson = '', entryFailure = '';
  const readingPositions = new Map();
  const mediaValues = new Map();
  const svgNS = 'http://www.w3.org/2000/svg';
  const element = (tag, text = '', className = '') => {
    const node = document.createElement(tag); node.textContent = String(text ?? ''); node.className = className; return node;
  };
  const svg = (tag, attributes = {}) => {
    const node = document.createElementNS(svgNS, tag);
    for (const [name, value] of Object.entries(attributes)) node.setAttribute(name, String(value));
    return node;
  };
  const paths = {
    menu: '<path d="M4 6h16M4 12h16M4 18h16"/>',
    close: '<path d="m6 6 12 12M18 6 6 18"/>',
    chat: '<path d="M4 4h16v12H9l-5 5Z"/><path d="M8 8h8M8 12h5"/>',
    book: '<path d="M12 5C8 3 5 3 2 4v15c4-1 7-1 10 1 3-2 6-2 10-1V4c-3-1-6-1-10 1Zm0 0v15"/>',
    slides: '<rect x="3" y="3" width="18" height="13" rx="1"/><path d="M12 16v5m-4 0h8M7 8h10M7 11h7"/>',
    arrow: '<path d="M5 12h14m-5-5 5 5-5 5"/>',
    left: '<path d="m14 5-7 7 7 7"/>',
    right: '<path d="m10 5 7 7-7 7"/>',
    send: '<path d="M12 20V4m-6 6 6-6 6 6"/>',
    check: '<path d="m5 12 4 4L19 6"/>',
    more: '<circle cx="5" cy="12" r="1"/><circle cx="12" cy="12" r="1"/><circle cx="19" cy="12" r="1"/>',
  };
  function icon(name) {
    const node = svg('svg', {width:16, height:16, viewBox:'0 0 24 24', fill:'none', stroke:'currentColor', 'stroke-width':1.6, 'stroke-linecap':'round', 'stroke-linejoin':'round', 'aria-hidden':'true'});
    node.innerHTML = paths[name] || paths.book; return node;
  }
  function button(text, action, primary = false, iconName = '') {
    const node = element('button', '', 'dc-button' + (primary ? ' dc-primary' : ''));
    node.type = 'button'; node.dataset.action = action;
    if (iconName) node.append(icon(iconName));
    if (text) node.append(element('span', text));
    return node;
  }
  function rich(value, className = 'dc-rich') {
    const node = element('div', '', className);
    if (api?.richText) api.richText(node, value ?? '');
    else if (window.GangyiChat) node.innerHTML = window.GangyiChat.renderMarkdown(String(value ?? ''), {math:true});
    else node.textContent = String(value ?? '');
    return node;
  }
  const items = value => Array.isArray(value) ? value : [];
  const finite = (value, fallback = 0) => Number.isFinite(Number(value)) ? Number(value) : fallback;
  const number = value => Number(finite(value).toFixed(3)).toLocaleString('zh-CN', {maximumFractionDigits:3});
  function safeLink(value, local = false) {
    if (typeof value !== 'string' || /[\\\u0000-\u0020\u007f]/.test(value)) return '';
    try {
      const url = new URL(value, location.origin);
      if (!['http:', 'https:'].includes(url.protocol) || url.username || url.password) return '';
      return local && url.origin !== location.origin ? '' : url.href;
    } catch (_) { return ''; }
  }
  const lessonId = () => lesson?.id || scope.lessonId || '';
  const storageKey = name => `deep:${lessonId() || scope.courseId || ''}:${name}`;
  const store = (name, value) => api?.storage.set(storageKey(name), String(value));
  const restore = name => api?.storage.get(storageKey(name)) || '';
  const status = message => { taskMessage = message; const node = document.getElementById('dc-status'); if (node) node.textContent = message; };
  async function request(path, data) {
    if (api?.request) return api.request(path, data);
    const response = await fetch(path, data === undefined ? {cache:'no-store'} : {method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify(data)});
    const value = await response.json(); if (!response.ok) throw new Error(value.error || '请求未完成，请重试。'); return value;
  }
  function skeleton() {
    const box = element('div', '', 'dc-empty'); box.setAttribute('role', 'status');
    box.append(element('h2', '正在读取本节内容'), element('p', '恢复已保存的讲义、课件和学习位置。'));
    for (let i = 0; i < 3; i++) box.append(element('div', '', 'dc-skeleton'));
    return box;
  }
  function buildShell() {
    mount.replaceChildren();
    mount.className = 'deep-course-shell'; mount.dataset.directoryOpen = 'true'; mount.dataset.chatOpen = 'false';
    document.body.classList.add('deep-course-body');
    const head = element('header', '', 'dc-head');
    const heading = element('div'); heading.append(element('h1', '正在读取课堂'));
    heading.querySelector('h1').id = 'dc-title';
    const context = element('p', '深度课程', 'dc-head-context'); context.id = 'dc-context'; heading.append(context);
    const controls = element('div', '', 'dc-head-actions');
    const directory = button('课程目录', 'directory', false, 'menu'); directory.id = 'dc-directory-toggle'; directory.setAttribute('aria-controls', 'dc-directory'); directory.setAttribute('aria-expanded', 'true');
    const answer = button('AI 答疑', 'chat', false, 'chat'); answer.id = 'dc-chat-toggle'; answer.setAttribute('aria-controls', 'dc-answer-panel'); answer.setAttribute('aria-expanded', 'false');
    controls.append(directory, answer); head.append(heading, controls);
    const layout = element('div', '', 'dc-layout');
    const outline = element('aside', '', 'dc-directory'); outline.id = 'dc-directory'; outline.setAttribute('aria-label', '课程目录');
    const reader = element('section', '', 'dc-reader');
    const toolbar = element('div', '', 'dc-view-toolbar');
    const tabs = element('nav', '', 'dc-tabs'); tabs.setAttribute('role', 'tablist'); tabs.setAttribute('aria-label', '教学内容视图');
    for (const [view, text, name] of [['lecture','讲义','book'], ['slides','课件','slides']]) {
      const tab = button(text, `view:${view}`, false, name); tab.className = 'dc-tab'; tab.id = `dc-tab-${view}`;
      tab.setAttribute('role', 'tab'); tab.setAttribute('aria-selected', String(view === currentView)); tab.setAttribute('aria-controls', 'dc-reading'); tab.tabIndex = view === currentView ? 0 : -1; tabs.append(tab);
    }
    const savedLabel = element('span', '已保存内容优先复用', 'dc-saved'); savedLabel.prepend(icon('check')); toolbar.append(tabs, savedLabel);
    const slideToolbar = element('nav', '', 'dc-slide-navigation'); slideToolbar.id = 'dc-slide-navigation'; slideToolbar.hidden = true; slideToolbar.setAttribute('aria-label', '课件翻页');
    const previousSlide = button('上一页', 'previous', false, 'left'), nextSlide = button('下一页', 'next', false, 'right');
    previousSlide.id = 'dc-slide-previous'; nextSlide.id = 'dc-slide-next';
    for (const control of [previousSlide, nextSlide]) control.setAttribute('aria-controls', 'dc-reading');
    const slideCount = element('span', '', 'dc-slide-count'); slideCount.id = 'dc-slide-count'; slideCount.setAttribute('role', 'status'); slideCount.setAttribute('aria-live', 'polite'); slideCount.setAttribute('aria-atomic', 'true');
    slideToolbar.append(previousSlide, slideCount, nextSlide);
    const reading = element('div', '', 'dc-reading'); reading.id = 'dc-reading'; reading.tabIndex = 0; reading.setAttribute('role', 'tabpanel'); reading.setAttribute('aria-labelledby', 'dc-tab-lecture'); reading.append(skeleton());
    const footer = element('footer', '', 'dc-footer'); const feedback = element('p', '正在读取课堂…', 'dc-footer-message'); feedback.id = 'dc-status'; feedback.setAttribute('role', 'status'); feedback.setAttribute('aria-live', 'polite');
    const footerControls = element('div', '', 'dc-footer-controls');
    const more = element('details', '', 'dc-more'); const summary = element('summary', '更多', 'dc-button'); summary.prepend(icon('more'));
    const menu = element('div', '', 'dc-more-list');
    for (const [action, text] of [['hint','给一点提示'], ['rephrase','换一种讲法'], ['worked_example','看示范'], ['retry_question','再试一道'], ['finish','检查本节完成情况'], ['retry-media','重试失败素材']]) menu.append(button(text, action));
    more.append(summary, menu);
    const stop = button('停止', 'stop'); stop.id = 'dc-stop'; stop.hidden = true;
    const retry = button('重试', 'retry'); retry.id = 'dc-retry'; retry.hidden = true;
    const enter = button('重新进入', 'enter'); enter.id = 'dc-enter'; enter.hidden = true;
    const next = button('继续学习', 'continue', true, 'arrow'); next.id = 'dc-continue';
    footerControls.append(stop, retry, enter, more, next); footer.append(feedback, footerControls); reader.append(toolbar, slideToolbar, reading, footer);
    const answerPanel = element('aside', '', 'dc-answer-panel'); answerPanel.id = 'dc-answer-panel'; answerPanel.setAttribute('aria-label', 'AI 答疑'); answerPanel.append(buildChat());
    layout.append(outline, reader, answerPanel); mount.append(head, layout);
    const outlineDialog = element('dialog', '', 'dc-dialog'); outlineDialog.id = 'dc-outline-dialog'; outlineDialog.setAttribute('aria-label', '课程目录');
    const dialogHead = element('div', '', 'dc-dialog-head'); dialogHead.append(element('span', '课程目录'), button('关闭', 'close-directory')); outlineDialog.append(dialogHead, element('div', '', 'dc-directory'));
    const chatDialog = element('dialog', '', 'dc-dialog dc-chat-dialog'); chatDialog.id = 'dc-chat-dialog'; chatDialog.setAttribute('aria-label', 'AI 答疑');
    mount.append(outlineDialog, chatDialog);
    chatDialog.addEventListener('close', () => { const room = chatDialog.querySelector('.dc-chat-room'); if (room) answerPanel.append(room); mount.dataset.chatOpen = 'false'; updatePanels(); });
    reading.addEventListener('scroll', () => {
      readingPositions.set(currentView, reading.scrollTop); store(`scroll:${currentView}`, reading.scrollTop); exposeVisible();
    }, {passive:true});
    new ResizeObserver(exposeVisible).observe(reading);
    mount.addEventListener('click', handleClick);
    mount.addEventListener('submit', handleSubmit);
    mount.addEventListener('keydown', handleKey);
    window.addEventListener('resize', updatePanels);
  }
  function buildChat() {
    const room = element('section', '', 'dc-chat-room'); room.id = 'dc-chat-room';
    const head = element('header', '', 'dc-chat-header'); const heading = element('div'); heading.append(element('h2', 'AI 答疑'), element('p', '围绕本课内容，随时提问。'));
    const close = button('', 'close-chat', false, 'close'); close.setAttribute('aria-label', '收起 AI 答疑'); close.classList.add('dc-icon-button'); head.append(heading, close);
    const scroll = element('div', '', 'dc-chat-scroll'); scroll.id = 'dc-chat-scroll'; scroll.setAttribute('role', 'log'); scroll.setAttribute('aria-label', '本节答疑记录'); scroll.append(element('p', '可以请 AI 解释当前讲义、课件或图示，也可以写下你的困惑。', 'dc-chat-empty'));
    const form = element('form', '', 'dc-chat-composer'); form.id = 'dc-chat-form'; const label = element('label', '向 AI 提问'); label.htmlFor = 'dc-chat-input';
    const row = element('div', '', 'dc-chat-input-row'); const input = element('textarea'); input.id = 'dc-chat-input'; input.rows = 2; input.placeholder = '哪一步还不理解？';
    input.addEventListener('input', () => store('chat-draft', input.value));
    const send = button('', 'send', true, 'send'); send.type = 'submit'; send.setAttribute('aria-label', '发送问题'); row.append(input, send);
    form.append(label, row, element('p', '讲义与答疑共享本节背景，记录按课时保存。', 'dc-chat-note')); room.append(head, scroll, form); return room;
  }
  function updatePanels() {
    const small = matchMedia('(max-width:700px)').matches;
    const chatDialog = document.getElementById('dc-chat-dialog');
    if (!small && chatDialog.open) { chatDialog.close(); mount.dataset.chatOpen = 'true'; }
    const directoryShown = !small && mount.dataset.directoryOpen === 'true';
    document.getElementById('dc-directory-toggle').setAttribute('aria-expanded', String(directoryShown || document.getElementById('dc-outline-dialog').open));
    document.getElementById('dc-chat-toggle').setAttribute('aria-expanded', String(mount.dataset.chatOpen === 'true' || chatDialog.open));
  }
  function toggleChat(open) {
    const dialog = document.getElementById('dc-chat-dialog');
    if (matchMedia('(max-width:700px)').matches) {
      if (open && !dialog.open) { dialog.append(document.getElementById('dc-chat-room')); dialog.showModal(); }
      else if (!open && dialog.open) dialog.close();
    }
    mount.dataset.chatOpen = String(open); store('chat-open', open ? '1' : '0'); updatePanels();
    if (open) document.getElementById('dc-chat-input').focus({preventScroll:true});
    requestAnimationFrame(exposeVisible);
  }
  function renderOutline() {
    const container = document.getElementById('dc-directory'); container.replaceChildren();
    container.append(element('h2', saved?.course?.title || '课程目录'), element('p', '沿课程主线，逐课探索。'));
    const stages = items(saved?.outline?.courseStructure);
    if (!stages.length) container.append(element('p', '课程目录暂未读取，请重试。'));
    stages.forEach((stage, phaseIndex) => {
      const group = element('details', '', 'dc-outline-stage'); group.open = phaseIndex + 1 === Number(lesson?.phaseIndex || scope.phaseIndex || 1);
      group.append(element('summary', stage.stage || stage.name || '学习阶段'));
      items(stage.topics).forEach((topic, topicIndex) => {
        const value = typeof topic === 'string' ? {title:topic} : topic;
        const active = lesson?.topicId || scope.topicId ? value.id === (lesson?.topicId || scope.topicId) : Number(lesson?.phaseIndex || scope.phaseIndex || 1) === Number(value.legacyPhaseIndex) && Number(lesson?.topicIndex || scope.topicIndex || 1) === Number(value.legacyTopicIndex);
        const control = button('', 'topic'); control.className = 'dc-outline-topic';
        if(Number.isSafeInteger(Number(value.legacyPhaseIndex))&&Number(value.legacyPhaseIndex)>0)control.dataset.phaseIndex=String(value.legacyPhaseIndex);
        if(Number.isSafeInteger(Number(value.legacyTopicIndex))&&Number(value.legacyTopicIndex)>0)control.dataset.topicIndex=String(value.legacyTopicIndex);
        if (value.id) control.dataset.topicId = value.id;
        if (active) {control.setAttribute('aria-current', 'page');group.open=true;}
        control.append(element('i', '', 'dc-topic-mark'), element('span', value.title || value.name || '知识点')); group.append(control);
      }); container.append(group);
    });
    const dialogBody = document.querySelector('#dc-outline-dialog>.dc-directory');
    dialogBody.replaceChildren(...[...container.children].map(node => node.cloneNode(true)));
  }
  const lessonQuestions = () => items(lesson?.sections).filter(section => section.kind === 'question' && section.legacyKind !== 'example');
  function evaluationsFor(section) {
    // 仅识别同题已发布的真实评价；低分或需要澄清仍交由主控判断，不在前端设置通过线。
    return items(lesson?.evaluations).filter(value => value.sectionId === section.id && value.id && value.interactionId &&
      typeof value.credible === 'boolean' && (!value.status || value.status === 'valid') &&
      (!value.questionId || !section.questionId || value.questionId === section.questionId));
  }
  function nextCourseTopic() {
    const topics = items(saved?.outline?.courseStructure).flatMap(stage => items(stage.topics)).filter(topic => topic && typeof topic === 'object');
    const currentId = lesson?.topicId || scope.topicId;
    const index = topics.findIndex(topic => currentId ? topic.id === currentId :
      Number(topic.legacyPhaseIndex) === Number(lesson?.phaseIndex || scope.phaseIndex) &&
      Number(topic.legacyTopicIndex) === Number(lesson?.topicIndex || scope.topicIndex));
    const next = index >= 0 ? topics[index + 1] : null;
    const hasIndices = next && ['legacyPhaseIndex','legacyTopicIndex'].every(key => Number.isSafeInteger(Number(next[key])) && Number(next[key]) > 0 && Number(next[key]) <= 100000);
    return next && (next.id || hasIndices) ? next : null;
  }
  function openTopic(topic) {
    const params = new URLSearchParams({courseId:saved?.course?.id || lesson?.courseId || scope.courseId});
    if (topic.id) params.set('topicId', topic.id);
    for (const [field, key] of [['legacyPhaseIndex','phaseIndex'], ['legacyTopicIndex','topicIndex']])
      if (Number.isSafeInteger(Number(topic[field])) && Number(topic[field]) > 0 && Number(topic[field]) <= 100000) params.set(key, topic[field]);
    // 新知识点只打开只读页面，尚未准备时由学生显式开始备课。
    location.href = location.pathname + '?' + params;
  }
  function showDirectory() {
    if (matchMedia('(max-width:700px)').matches || matchMedia('(max-width:900px)').matches && mount.dataset.chatOpen === 'true') document.getElementById('dc-outline-dialog').showModal();
    else { mount.dataset.directoryOpen = 'true'; store('directory-open', '1'); }
    updatePanels();
  }
  function renderMedia(media) {
    const figure = element('figure', '', 'dc-media'); figure.dataset.mediaId = media.id;
    if (media.title) figure.append(element('h3', media.title));
    if (media.status !== 'ready') {
      figure.append(element('p', media.error || (media.status === 'pending' ? '这部分素材正在准备，已生成的讲义可继续学习。' : '这部分素材暂未完成，已生成的讲义可继续学习。'), 'dc-media-state'));
      if (media.status !== 'pending') { const retry = button('重试此素材', 'media'); retry.dataset.mediaId = media.id; figure.append(retry); }
    } else if (media.type === 'image') {
      const url = safeLink(media.url, true); const source = safeLink(media.sourceUrl);
      if (url) {
        const image = element('img', '', 'dc-media-image'); image.src = url; image.alt = media.alt || media.title || '课堂参考图片'; image.loading = 'lazy'; image.referrerPolicy = 'no-referrer';
        image.addEventListener('error', () => { image.hidden = true; const message = element('p', '图片暂时无法显示，可继续学习或重试此图片。', 'dc-media-state'); const retry = button('重试图片', 'media'); retry.dataset.mediaId = media.id; retry.dataset.repair='true'; figure.append(message, retry); }, {once:true}); figure.append(image);
      } else figure.append(element('p', '图片地址暂不可用。', 'dc-media-state'));
      if (source) { const link = element('a', media.sourceTitle || '查看图片来源', 'dc-media-source'); link.href = source; link.target = '_blank'; link.rel = 'noopener noreferrer'; figure.append(link); }
    } else if (media.type === 'timeline') {
      const list = element('ol', '', 'dc-timeline'); for (const event of items(media.events)) { const row = element('li'); row.append(element('strong', event.label || ''), element('div', event.text || '')); list.append(row); } figure.append(list);
    } else if (media.type === 'flowchart') {
      const list = element('ul', '', 'dc-flow'); for (const node of items(media.nodes)) { const entry = element('li'); entry.append(element('span', node.label || node.id || '', 'dc-flow-node')); list.append(entry); } figure.append(list);
      const names = new Map(items(media.nodes).map(node => [node.id, node.label || node.id]));
      const relations = items(media.edges).map(edge => `${names.get(edge.from) || edge.from} → ${names.get(edge.to) || edge.to}${edge.label ? `：${edge.label}` : ''}`);
      if (relations.length) figure.append(element('p', relations.join('\n'), 'dc-flow-edge-list'));
    } else if (media.type === 'function') renderFunction(media, figure);
    else if (media.type === 'forces') renderForces(media, figure);
    else figure.append(element('p', '当前图示格式暂不支持展示，讲义内容仍可阅读。', 'dc-media-state'));
    if (media.caption) figure.append(element('figcaption', media.caption));
    return figure;
  }
  function mediaFor(ids) {
    const media = items(saved?.content?.media); return items(ids).map(id => media.find(value => value.id === id)).filter(Boolean);
  }
  function svgText(parent, text, x, y, attributes = {}) { const node = svg('text', {x,y,...attributes}); node.textContent = text; parent.append(node); return node; }
  function renderFunction(media, figure) {
    const plot = svg('svg', {viewBox:'0 0 360 230', class:'dc-plot', role:'img', 'aria-label':media.title || '函数图像'}); figure.append(plot);
    const coefficients = items(media.coefficients).slice(0,3).map(value => finite(value)); while (coefficients.length < 3) coefficients.push(0);
    const parameter = media.parameter && typeof media.parameter === 'object' ? media.parameter : null;
    const index = Math.max(0, Math.min(2, Math.trunc(finite(parameter?.coefficientIndex))));
    const initial = finite(parameter?.value, coefficients[index]);
    const update = value => {
      if (parameter) coefficients[index] = value;
      plot.replaceChildren(); const left=43,right=343,top=17,bottom=196;
      const xMin=finite(media.xMin,-5), xMax=Math.max(xMin+.1,finite(media.xMax,5));
      const points = Array.from({length:181},(_,i) => { const x=xMin+(xMax-xMin)*i/180; return [x, coefficients[0]*x*x+coefficients[1]*x+coefficients[2]]; });
      let yMin=Math.min(0,...points.map(point=>point[1])), yMax=Math.max(0,...points.map(point=>point[1]));
      const padding=Math.max((yMax-yMin)*.12,.5); yMin-=padding; yMax+=padding;
      const px=x=>left+(x-xMin)/(xMax-xMin)*(right-left), py=y=>bottom-(y-yMin)/(yMax-yMin)*(bottom-top);
      for(let i=0;i<=4;i++){const y=top+(bottom-top)*i/4;plot.append(svg('line',{x1:left,y1:y,x2:right,y2:y,class:'dc-tick'}));}
      const zeroY=py(0),zeroX=px(Math.max(xMin,Math.min(xMax,0)));
      plot.append(svg('line',{x1:left,y1:zeroY,x2:right,y2:zeroY,class:'dc-axis'}),svg('line',{x1:zeroX,y1:top,x2:zeroX,y2:bottom,class:'dc-axis'}));
      plot.append(svg('path',{d:points.map(([x,y],i)=>`${i?'L':'M'}${px(x).toFixed(2)},${py(y).toFixed(2)}`).join(' '),class:'dc-curve'}));
      svgText(plot,number(xMin),left,bottom+21,{'text-anchor':'middle'});svgText(plot,number(xMax),right,bottom+21,{'text-anchor':'middle'});
      svgText(plot,'x',right-4,zeroY-7);svgText(plot,'y',zeroX+7,top+7);svgText(plot,number(yMax),left-5,top+5,{'text-anchor':'end'});svgText(plot,number(yMin),left-5,bottom+3,{'text-anchor':'end'});
      plot.setAttribute('aria-label',`${media.title || '函数图像'}，y = ${number(coefficients[0])} x² + ${number(coefficients[1])} x + ${number(coefficients[2])}，横轴范围 ${number(xMin)} 到 ${number(xMax)}`);
    };
    if (parameter) renderParameter(figure,media.id,parameter.name || '参数',parameter.min,parameter.max,parameter.step,initial,value=>{mediaValues.set(media.id,value);update(value);});
    update(finite(mediaValues.get(media.id),initial));
  }
  function renderParameter(figure, id, name, minimum, maximum, step, initial, onChange, unit = '') {
    const row=element('div','','dc-parameter'), label=element('label',name), input=element('input'), output=element('output');
    input.type='range';input.id=`dc-parameter-${id}-${currentView}`;label.htmlFor=input.id;
    input.min=String(finite(minimum,0));input.max=String(Math.max(Number(input.min)+.1,finite(maximum,10)));input.step=String(Math.max(.001,finite(step,.1)));
    input.value=String(finite(mediaValues.get(id),initial));output.htmlFor=input.id;output.textContent=number(input.value)+unit;
    input.setAttribute('aria-label',`${name}${unit ? `（${unit}）` : ''}`);
    const change=()=>{output.textContent=number(input.value)+unit;onChange(Number(input.value));};input.addEventListener('input',change);
    const reset=button('重置','reset');reset.className='dc-reset';reset.onclick=()=>{input.value=String(initial);change();};
    row.append(label,input,output,reset);figure.append(row);
  }
  function renderForces(media, figure) {
    const mass=Math.max(.001,finite(media.mass,1)),friction=Math.max(0,finite(media.friction)),initial=finite(media.force);
    const plot=svg('svg',{viewBox:'0 0 360 230',class:'dc-plot',role:'img','aria-label':media.title || '水平方向受力演示'});figure.append(plot);
    const readout=element('div','','dc-force-readout');figure.append(readout);
    const update=force=>{
      mediaValues.set(media.id,force);plot.replaceChildren();const direction=Math.sign(force),opposition=Math.min(Math.abs(force),friction),net=direction*Math.max(0,Math.abs(force)-friction),acceleration=net/mass;
      const defs=svg('defs'),marker=svg('marker',{id:`dc-arrow-${media.id}-${currentView}`,viewBox:'0 0 10 10',refX:9,refY:5,markerWidth:5,markerHeight:5,orient:'auto-start-reverse'});marker.append(svg('path',{d:'M0 0 10 5 0 10Z',fill:'currentColor'}));defs.append(marker);plot.append(defs);
      plot.append(svg('line',{x1:25,y1:156,x2:335,y2:156,class:'dc-axis'}),svg('rect',{x:145,y:92,width:70,height:64,rx:5,class:'dc-object'}));
      const scale=110/Math.max(1,Math.abs(force),friction),arrow=`url(#dc-arrow-${media.id}-${currentView})`;
      if(Math.abs(force)>.001)plot.append(svg('line',{x1:180,y1:111,x2:180+force*scale,y2:111,class:'dc-force','marker-end':arrow}));
      if(opposition>.001)plot.append(svg('line',{x1:180,y1:137,x2:180-direction*opposition*scale,y2:137,class:'dc-friction','marker-end':arrow}));
      svgText(plot,`F = ${number(force)} N`,Math.min(285,Math.max(75,180+force*scale*.6)),82,{'text-anchor':'middle'});svgText(plot,`f = ${number(opposition)} N`,95,184,{'text-anchor':'middle'});svgText(plot,`${number(mass)} kg`,180,210,{'text-anchor':'middle'});
      readout.replaceChildren(element('span',`合力 ${number(net)} N`),element('span',`加速度 ${number(acceleration)} m/s²`),element('span',`阻力上限 ${number(friction)} N`));
      plot.setAttribute('aria-label',`${media.title || '水平方向受力演示'}，静止起步模型，质量 ${number(mass)} 千克，施加力 ${number(force)} 牛，实际反向阻力 ${number(opposition)} 牛，合力 ${number(net)} 牛，加速度 ${number(acceleration)} 米每二次方秒`);
    };
    const forceRange=Math.max(20,Math.abs(initial)*2,friction*2);
    renderParameter(figure,media.id,'施加力 F',-forceRange,forceRange,.5,initial,update,' N');
    figure.append(element('p','静止起步的简化模型：向右为正，阻力反向且有上限；仅比较水平受力，不模拟速度或位移。','dc-media-state'));
    update(finite(mediaValues.get(media.id),initial));
  }
  function renderQuestion(section) {
    const card=element('section','','dc-question');card.dataset.sectionId=section.id;card.dataset.sectionVersion=String(section.version || 1);
    const evaluations=evaluationsFor(section);card.dataset.evaluated=String(evaluations.length>0);
    card.append(element('h2',section.title || '试着想一想'),rich(section.question?.question || ''));
    if(section.question?.materials)card.append(renderMaterials(section.question.materials));
    const form=element('form');form.dataset.questionId=section.id;const options=element('div','','dc-question-options');
    let selected=[];try{selected=JSON.parse(restore(`choice:${section.id}`)||'[]');}catch(_){ /* 损坏的本地草稿不改变正式作答。 */ }
    items(section.question?.options).forEach((text,index)=>{const label=element('label','','dc-question-option'),choice=element('input');choice.type=section.question.type==='multi_choice'?'checkbox':'radio';choice.name=`dc-question-${section.id}`;choice.value=String(index);choice.checked=items(selected).includes(String(index));label.append(choice,element('span',`${String.fromCharCode(65+index)}. ${typeof text==='string'?text:JSON.stringify(text)}`));options.append(label);});
    options.addEventListener('change',()=>store(`choice:${section.id}`,JSON.stringify([...options.querySelectorAll('input:checked')].map(input=>input.value))));
    const label=element('label','写下答案或思路'),answer=element('textarea');answer.id=`dc-answer-${section.id}`;label.htmlFor=answer.id;answer.value=restore(`answer:${section.id}`);answer.placeholder='先独立尝试，再和 AI 一起检查。';answer.addEventListener('input',()=>store(`answer:${section.id}`,answer.value));
    const submit=button('提交答案','answer',true);submit.type='submit';form.append(options,label,answer,submit);card.append(form);
    for(const evaluation of evaluations){const feedback=element('div','','dc-evaluation');feedback.dataset.evaluationId=evaluation.id;feedback.append(element('strong','本次作答反馈'),rich(evaluation.feedback || ''));card.append(feedback);}
    return card;
  }
  function renderMaterials(value) {
    const root=element('div','','dc-question-materials');
    if(Array.isArray(value)){for(const item of value)root.append(renderMaterials(item));return root;}
    if(value===null||typeof value!=='object'){root.append(rich(value??''));return root;}
    for(const key of ['title','text','body','latex','formula'])if(typeof value[key]==='string')root.append(rich(['latex','formula'].includes(key)?`$$${value[key]}$$`:value[key]));
    const imageUrl=safeLink(value.imageUrl);if(imageUrl){const image=element('img','','dc-media-image');image.src=imageUrl;image.alt=value.alt||'题目材料';image.loading='lazy';image.referrerPolicy='no-referrer';root.append(image);}
    const rows=value.rows||value.values||value.cells;if(Array.isArray(rows)){const table=element('table'),wrap=element('div','','dc-rich');if(Array.isArray(value.headers)){const row=element('tr');for(const text of value.headers)row.append(element('th',text));table.append(row);}for(const items of rows){const row=element('tr');for(const text of Array.isArray(items)?items:[items]){const cell=element('td');cell.append(rich(text));row.append(cell);}table.append(row);}wrap.append(table);root.append(wrap);}
    if(value.table)root.append(renderMaterials(value.table));return root;
  }
  function renderReading(preserve = true) {
    const reading=document.getElementById('dc-reading');const oldTop=preserve?reading.scrollTop:finite(restore(`scroll:${currentView}`));reading.replaceChildren();
    const lecture=items(saved?.content?.lecture),slides=items(saved?.content?.slides),questions=lessonQuestions();
    if(!lecture.length&&!slides.length){
      const empty=element('section','','dc-empty');empty.append(element('h2',lesson?'本节内容正在准备':'从这一课开始'),element('p',lesson?'AI 会结合课程目标与学习记录准备内容。已成功保存的部分会优先展示。':'进入课时后生成本节讲义、课件和需要的图示，之后可以随时回来继续。'));
      if(!busy&&!sending)empty.append(button(lesson?'准备本节内容':'开始这一课','start',true,'arrow'));reading.append(empty);
    } else if(currentView==='slides') {
      if(!slides.length){const empty=element('div','','dc-empty');empty.append(element('h2','本节暂未生成课件'),element('p','可以先阅读已保存的讲义，或在 AI 答疑中提出制作课件的需要。'),button('打开 AI 答疑','chat'));reading.append(empty);}
      else {
        currentSlide=Math.min(Math.max(0,currentSlide),slides.length-1);const slide=slides[currentSlide];const card=element('article','','dc-slide');card.append(element('h2',slide.title || lesson?.title || '本节课件'));
        const sources=items(slide.sourceSectionIds).map(id=>items(lesson?.sections).find(section=>section.id===id&&['explanation','summary'].includes(section.kind))).filter(Boolean);
        if(sources.length){for(const source of sources){const block=element('div','','dc-slide-source');block.dataset.sectionId=source.id;block.append(rich(source.body||''));card.append(block);}}
        else card.append(rich(slide.body || ''));
        for(const media of mediaFor(slide.mediaIds))card.append(renderMedia(media));reading.append(card);
      }
    } else {
      const article=element('article','','dc-lecture'),attached=new Set();
      for(const section of lecture){const block=element('section','','dc-lecture-section');block.dataset.sectionId=section.sectionId||section.id;if(section.title)block.append(element('h2',section.title));block.append(rich(section.body || ''));for(const media of mediaFor(section.mediaIds)){block.append(renderMedia(media));attached.add(media.id);}article.append(block);}
      for(const media of items(saved?.content?.media))if(!attached.has(media.id))article.append(renderMedia(media));
      for(const section of questions)article.append(renderQuestion(section));
      reading.append(article);
    }
    const slideToolbar=document.getElementById('dc-slide-navigation');slideToolbar.hidden=currentView!=='slides'||!slides.length;
    if(!slideToolbar.hidden){document.getElementById('dc-slide-previous').disabled=currentSlide===0;document.getElementById('dc-slide-next').disabled=currentSlide===slides.length-1;document.getElementById('dc-slide-count').textContent=`${currentSlide+1} / ${slides.length}`;}
    reading.scrollTop=oldTop;document.getElementById('dc-tab-lecture').setAttribute('aria-selected',String(currentView==='lecture'));document.getElementById('dc-tab-slides').setAttribute('aria-selected',String(currentView==='slides'));
    for(const tab of mount.querySelectorAll('.dc-tab'))tab.tabIndex=tab.getAttribute('aria-selected')==='true'?0:-1;
    reading.setAttribute('aria-labelledby',`dc-tab-${currentView}`);setDisabled();requestAnimationFrame(exposeVisible);
  }
  function chatRecords() {
    const timeline=items(lesson?.timeline).filter(item=>item.type==='message');
    const records=timeline.length?timeline:items(lesson?.teachingDialog).length?items(lesson.teachingDialog):items(lesson?.dialog);
    const combined=new Map();
    for(const [index,record] of [...records,...pendingMessages].entries()){
      const key=record.taskId?`${record.taskId}:${record.role}:${record.step??0}`:record.id||`saved:${index}`;
      const previous=combined.get(key);if(previous&&!record.partial)continue;
      if(previous&&String(previous.text||'').length>=String(record.text||'').length)continue;
      combined.set(key,{...record,displayKey:key});
    }
    return [...combined.values()];
  }
  function renderChat() {
    const scroll=document.getElementById('dc-chat-scroll'),oldTop=scroll.scrollTop,atBottom=scroll.scrollHeight-scroll.clientHeight-oldTop<55;
    const records=chatRecords();scroll.replaceChildren();
    for(const record of records){const text=record.text || record.message || '';if(!text)continue;const key=record.displayKey;
      const message=element('article','',`dc-chat-message ${record.role==='user'?'user':'assistant'}`);message.dataset.messageKey=key;message.append(element('h3',record.role==='user'?'你':'AI 导师'));const body=record.role==='user'?element('div',text,'dc-rich'):rich(text);message.append(body);if(record.partial)message.append(element('p','这段内容尚未完整完成。','dc-partial'));scroll.append(message);}
    if(!scroll.children.length)scroll.append(element('p','可以围绕当前讲义、课件和图示提问，也可以请 AI 换一种讲法。','dc-chat-empty'));
    scroll.scrollTop=atBottom?scroll.scrollHeight:oldTop;
  }
  function exposeVisible() {
    if(!lesson||lesson.entered!==true||!api||document.hidden)return;const viewport=document.getElementById('dc-reading');
    const byId=new Map(items(lesson.sections).map(section=>[section.id,section]));
    for(const node of viewport.querySelectorAll('[data-section-id]')){const section=byId.get(node.dataset.sectionId);if(section)api.recordVisibleSection?.(lesson.id,section,node,viewport);}
    for(const node of viewport.querySelectorAll('[data-evaluation-id]')){const evaluation=items(lesson.evaluations).find(value=>value.id===node.dataset.evaluationId);if(evaluation)api.recordEvaluation?.(lesson.id,evaluation,node,viewport);}
    const chat=document.getElementById('dc-chat-scroll');if(!chat?.getBoundingClientRect().width)return;
    for(const record of chatRecords()){const node=[...chat.querySelectorAll('[data-message-key]')].find(value=>value.dataset.messageKey===record.displayKey);if(node)api.recordVisibleMessage?.(record,node.querySelector('.dc-rich'),chat);}
  }
  function setDisabled() {
    const locked=busy||sending;for(const control of mount.querySelectorAll('.dc-question form button,.dc-question form input,.dc-chat-composer button,#dc-continue,[data-action=hint],[data-action=rephrase],[data-action=worked_example],[data-action=retry_question],[data-action=finish],[data-action=retry-media],[data-action=media]'))control.disabled=locked||!lesson||lesson.entered!==true||Boolean(scopeError);
    const next=document.getElementById('dc-continue'),topic=lesson?.completed?nextCourseTopic():null;
    const label=lesson?.completed?(topic?'下一知识点':'查看课程目录'):'继续学习';next.querySelector('span').textContent=label;next.setAttribute('aria-label',topic?`下一知识点：${topic.title||topic.name||'继续课程'}`:label);
    next.disabled=locked||!lesson||Boolean(scopeError)||(!lesson.completed&&lesson.entered!==true);
    for(const control of mount.querySelectorAll('[data-action=start]'))control.disabled=locked;
    const stop=document.getElementById('dc-stop'),retry=document.getElementById('dc-retry');stop.hidden=!busy;retry.hidden=!lastTask||!['failed','cancelled','superseded'].includes(lastTask.status);stop.disabled=!watchingTask;retry.disabled=sending;
    const enter=document.getElementById('dc-enter');enter.hidden=!entryFailure;enter.disabled=Boolean(enteringLesson)||sending;
  }
  function renderAll() {
    document.getElementById('dc-title').textContent=lesson?.title || saved?.course?.title || '深度课程';
    document.getElementById('dc-context').textContent=lesson?.purpose || saved?.course?.goal || '讲义与课件，按需要深入学习。';
    renderOutline();renderReading();renderChat();document.getElementById('dc-chat-input').value=restore('chat-draft');
    const next=lesson?.completed?nextCourseTopic():null;
    status(lesson?.completed&&!busy?(next?`本节已完成，可前往「${next.title||next.name||'下一知识点'}」。`:'本节已完成，可从目录回顾或选择其他知识点。'):taskMessage||'讲义与课件已保存，学习操作会继续记录。');updatePanels();
    if(lesson){window.GangyiNavigation?.setContext?.({courseId:lesson.courseId,lessonId:lesson.id,phaseIndex:lesson.phaseIndex,topicIndex:lesson.topicIndex,topicId:lesson.topicId,teachingTaskId:lesson.teachingTaskId});document.dispatchEvent(new CustomEvent('gangyi:lesson-context',{detail:{courseId:lesson.courseId,lessonId:lesson.id,phaseIndex:lesson.phaseIndex,topicIndex:lesson.topicIndex}}));}
  }
  async function refresh() {
    if(scopeError)return;
    if(refreshPromise){refreshAgain=true;return refreshPromise;}
    const epoch=activeEpoch;
    refreshPromise=(async()=>{
      try{const result=await request('/api/deep-course?'+new URLSearchParams(scope));if(epoch!==activeEpoch)return;saved=result;lesson=result.lesson || null;
        if(lesson?.id){scope.lessonId=lesson.id;query.set('lessonId',lesson.id);history.replaceState(null,'',location.pathname+'?'+query);}
        renderAll();
        if(lesson?.status==='ready'&&lesson.entered!==true&&!enteringLesson&&!entryFailure){
          enteringLesson=lesson.id;setDisabled();status('正在恢复本课学习位置…');
          try{if(api?.control)await api.control({command:'enter_lesson',lessonId:lesson.id});else await request('/api/learning-agent/control',{command:'enter_lesson',lessonId:lesson.id});
            if(epoch===activeEpoch){const entered=await request('/api/deep-course?'+new URLSearchParams(scope));if(epoch===activeEpoch){saved=entered;lesson=entered.lesson||null;renderAll();}}
          }catch(error){entryFailure=error.message;status(error.message);}
          finally{enteringLesson='';setDisabled();}
        }
        if(lesson?.teachingTaskId&&lesson.teachingTaskId!==watchingTask)observe(lesson.teachingTaskId);
      }catch(error){status(error.message);if(!saved){const container=document.getElementById('dc-reading');container.replaceChildren();const empty=element('div','','dc-empty');empty.append(element('h2','课堂暂时无法读取'),element('p',error.message),button('重新读取','reload'));container.append(empty);}}
    })();
    try{await refreshPromise;}finally{refreshPromise=null;if(refreshAgain){refreshAgain=false;refresh();}}
  }
  function paintTask(task) {
    lastTask=task;busy=!task.paused&&['pending','running'].includes(task.status);
    const labels={pending:'正在准备本次学习内容…',running:'AI 正在处理，已有内容可以继续阅读。',waiting_student:'准备好了，可以继续作答或提问。',ready:'本次内容已保存。',paused:'任务已暂停，已有内容与输入保留。',cancelled:'已停止，已有内容与输入保留。',superseded:'已有更新的学习记录，旧结果未应用。'};
    status(task.status==='failed'?(api?.failureMessage?.(task)||task.error||'本次生成未完成，可重试。'):labels[task.status]||'已更新课堂状态。');setDisabled();
  }
  function observe(taskId) {
    if(!taskId||watchingTask===taskId)return;stopWatching?.();watchingTask=taskId;const epoch=activeEpoch;
    if(!api?.watch){status('实时状态暂不可用，请重新读取已保存的课堂。');return;}
    stopWatching=api.watch(taskId,{onEvent(event){if(epoch!==activeEpoch)return false;if(event.type==='action')refresh();
      if(event.type==='delta'&&event.field==='message'){let record=pendingMessages.find(value=>value.taskId===taskId&&value.step===event.step);if(!record){record={id:`${taskId}:${event.step}`,taskId,step:event.step,role:'assistant',text:'',sequences:[],partial:true};pendingMessages.push(record);}record.text+=event.text;record.sequences.push(event.seq);renderChat();}return false;},
      onState(task){if(epoch!==activeEpoch)return;if(task.lesson?.id&&!scope.lessonId)scope.lessonId=task.lesson.id;paintTask(task);if(!['pending','running'].includes(task.status)){for(const record of pendingMessages)if(record.taskId===taskId)record.partial=!['ready','waiting_student'].includes(task.status);refresh();}},
      onConnectionError(message){if(epoch===activeEpoch)status(message);}});
  }
  async function start() {
    if(busy||sending||scopeError)return;sending=true;entryFailure='';setDisabled();status('正在准备这一课…');
    try{const value=await request('/api/deep-course/start',{...scope,requestId:api?.id?.()||crypto.randomUUID()});if(value.lessonId)scope.lessonId=value.lessonId;if(value.task?.lesson?.id)scope.lessonId=value.task.lesson.id;if(value.task){paintTask(value.task);observe(value.task.id);}await refresh();}
    catch(error){status(error.message);}finally{sending=false;setDisabled();}
  }
  async function sendEvent(event, draft) {
    if(!lesson||lesson.entered!==true||busy||sending||scopeError)return;sending=true;setDisabled();
    const viewport=document.getElementById('dc-reading').getBoundingClientRect();
    const visibleSection=[...mount.querySelectorAll('.dc-lecture-section')].find(node=>{const box=node.getBoundingClientRect();return box.bottom>viewport.top+12&&box.top<viewport.bottom;});
    const contentId=currentView==='slides'?items(saved?.content?.slides)[currentSlide]?.id:visibleSection?.dataset.sectionId||items(saved?.content?.lecture)[0]?.id;
    const mediaState=Object.fromEntries([...mediaValues].filter(([,value])=>typeof value==='number'&&Number.isFinite(value)).slice(0,32));
    const payload={...event,courseId:lesson.courseId,lessonId:lesson.id,view:event.type==='question_answer'?'practice':'learn',deepView:currentView,...(contentId?{contentId}:{}),mediaState,requestId:api?.id?.()||crypto.randomUUID()};
    const key=storageKey(`pending:${event.type}:${event.sectionId || 'chat'}`);let pending;
    try{pending=JSON.parse(api?.storage.get(key)||'null');}catch(_){ /* 本地损坏的请求记录重新建立。 */ }
    if(pending&&pending.text===payload.text&&pending.type===payload.type&&pending.sectionVersion===payload.sectionVersion)payload.requestId=pending.requestId;
    api?.storage.set(key,JSON.stringify(payload));
    try{exposeVisible();const task=api?.submit?await api.submit(payload):await request('/api/learning-agent/events',payload);api?.storage.remove(key);
      if(event.text&&event.type!=='lesson_finish_request')pendingMessages.push({id:`${task.id}:user`,taskId:task.id,role:'user',text:event.text});
      if(draft){store(draft.name,'');if(draft.node?.value===draft.text)draft.node.value='';}
      paintTask(task);observe(task.id);renderChat();await refresh();
    }catch(error){status(error.message);}finally{sending=false;setDisabled();}
  }
  function handleSubmit(event) {
    if(event.target.id==='dc-chat-form'){event.preventDefault();const input=document.getElementById('dc-chat-input');if(!input.value.trim()){input.focus();return;}sendEvent({type:'chat',text:input.value.trim(),...(lesson?.teachingFocus?.sectionId?{sectionId:lesson.teachingFocus.sectionId}:{})},{name:'chat-draft',node:input,text:input.value});return;}
    const id=event.target.dataset.questionId;if(!id)return;event.preventDefault();const section=items(lesson?.sections).find(value=>value.id===id);if(!section)return;
    const form=event.target,answer=form.querySelector('textarea'),choices=[...form.querySelectorAll('input:checked')].map(input=>Number(input.value));
    const selected=choices.map(index=>`${String.fromCharCode(65+index)}. ${section.question.options[index]}`);const text=[...selected,answer.value.trim()].filter(Boolean).join('\n');
    if(!text){status('先选择答案或写下你的思路，再提交。');answer.focus();return;}
    sendEvent({type:'question_answer',action:'answer',text,sectionId:section.id,sectionVersion:section.version},{name:`answer:${section.id}`,node:answer,text:answer.value});
  }
  async function waitMediaTask(task, epoch) {
    let current=task;
    while(['pending','running'].includes(current.status)&&!current.paused){
      await new Promise(resolve=>setTimeout(resolve,400));
      if(epoch!==activeEpoch)throw new Error('页面已经离开，未继续重试其他素材。');
      current=await request('/api/learning-agent?taskId='+encodeURIComponent(current.id));
    }
    if(current.paused||!['ready','waiting_student'].includes(current.status))throw new Error(current.error||'素材重试已暂停或未完成，其余素材保留原状态。');
    return current;
  }
  async function retryMedia(id = '', repair = false) {
    if(!lesson||lesson.entered!==true||busy||sending)return;const media=id?[id]:items(saved?.content?.media).filter(value=>value.status==='failed').map(value=>value.id);
    if(!media.length){status('当前没有需要重试的失败素材。');return;}sending=true;setDisabled();
    const epoch=activeEpoch;
    try{for(const mediaId of media){const value=await request('/api/deep-course/retry-media',{lessonId:lesson.id,mediaId,repair,requestId:api?.id?.()||crypto.randomUUID()});if(value.task){paintTask(value.task);observe(value.task.id);await waitMediaTask(value.task,epoch);}await refresh();}}
    catch(error){status(error.message);}finally{sending=false;setDisabled();}
  }
  function continueLearning() {
    if(busy||sending||scopeError)return;
    if(!lesson){status('点击「开始这一课」准备本节内容。');return;}
    if(lesson.completed){const next=nextCourseTopic();if(next)openTopic(next);else showDirectory();return;}
    if(lesson.entered!==true)return;
    const questions=lessonQuestions(),unanswered=questions.find(section=>!evaluationsFor(section).length);
    if(unanswered){
      if(currentView!=='lecture'){readingPositions.set(currentView,document.getElementById('dc-reading').scrollTop);currentView='lecture';store('view',currentView);renderReading(false);}
      const question=[...document.querySelectorAll('#dc-reading .dc-question')].find(node=>node.dataset.sectionId===unanswered.id);
      question?.scrollIntoView({block:'start',behavior:'smooth'});question?.querySelector('input,textarea')?.focus({preventScroll:true});
      status('先尝试这道尚未评价的练习，AI 会根据你的真实回答继续教学。');return;
    }
    toggleChat(true);
    sendEvent(questions.length?{type:'lesson_finish_request',text:'请根据本节真实作答和已有评价判断下一步：证据充分时判断本节是否完成；证据不足时继续教学或安排需要的练习，不因阅读或翻页认定掌握。'}:
      {type:'chat',text:'请结合本节已保存的学习内容与真实记录，判断适合的下一步教学；需要练习或澄清时继续安排，不因阅读认定掌握。'});
  }
  async function handleClick(event) {
    const control=event.target.closest('[data-action]');if(!control)return;const action=control.dataset.action;
    if(action==='directory'){if(matchMedia('(max-width:700px)').matches||matchMedia('(max-width:900px)').matches&&mount.dataset.chatOpen==='true'){document.getElementById('dc-outline-dialog').showModal();}else{mount.dataset.directoryOpen=String(mount.dataset.directoryOpen!=='true');store('directory-open',mount.dataset.directoryOpen==='true'?'1':'0');}updatePanels();return;}
    if(action==='close-directory'){document.getElementById('dc-outline-dialog').close();updatePanels();return;}
    if(action==='chat'){toggleChat(mount.dataset.chatOpen!=='true');return;}if(action==='close-chat'){toggleChat(false);return;}
    if(action.startsWith('view:')){readingPositions.set(currentView,document.getElementById('dc-reading').scrollTop);currentView=action.slice(5);store('view',currentView);renderReading(false);return;}
    if(action==='previous'||action==='next'){currentSlide+=action==='next'?1:-1;store('slide',currentSlide);store('scroll:slides',0);readingPositions.set('slides',0);renderReading(false);return;}
    if(action==='topic'){openTopic({id:control.dataset.topicId,legacyPhaseIndex:control.dataset.phaseIndex,legacyTopicIndex:control.dataset.topicIndex});return;}
    if(action==='start'){start();return;}if(action==='reload'||action==='enter'){entryFailure='';refresh();return;}if(action==='media'){retryMedia(control.dataset.mediaId,control.dataset.repair==='true');return;}if(action==='retry-media'){retryMedia();return;}
    control.closest('.dc-more')?.removeAttribute('open');
    if(action==='continue'){continueLearning();return;}
    if(action==='finish'){sendEvent({type:'lesson_finish_request',text:'请根据本节真实作答和已有评价判断完成情况，证据不足时明确说明。'});return;}
    if(['hint','rephrase','worked_example','retry_question'].includes(action)){
      const section=items(lesson?.sections).find(value=>value.id===(lesson?.teachingFocus?.responseSectionId||lesson?.teachingFocus?.interactionSectionId||lesson?.teachingFocus?.practiceSectionId));
      if(action!=='rephrase'&&section?.kind!=='question'){status('请先进入本课已有题目，再请求提示或示范。');return;}
      const text={hint:'请给一点提示，保留让我独立完成的空间。',rephrase:'请结合当前讲义、课件及我的学习偏好，换一种讲法。',worked_example:'请示范当前题目的完整过程，再判断是否需要独立复练。',retry_question:'请根据当前题目卡点安排一道关联新题。'}[action];
      toggleChat(true);sendEvent({type:'teaching_request',requestType:action,text,...(section?.kind==='question'?{sectionId:section.id,sectionVersion:section.version}:{})});return;
    }
    if((action==='stop'||action==='retry')&&watchingTask){try{const task=api?.control?await api.control({command:action==='stop'?'cancel':'retry',taskId:watchingTask}):await request('/api/learning-agent/control',{command:action==='stop'?'cancel':'retry',taskId:watchingTask});watchingTask='';paintTask(task);observe(task.id);}catch(error){status(error.message);}}
  }
  function handleKey(event) {
    if(event.target.id==='dc-chat-input'&&event.key==='Enter'&&!event.shiftKey&&!event.isComposing){event.preventDefault();event.target.closest('form').requestSubmit();}
    if(event.target.classList.contains('dc-tab')&&['ArrowLeft','ArrowRight','Home','End'].includes(event.key)){event.preventDefault();const tabs=[...mount.querySelectorAll('.dc-tab')];const index=event.key==='Home'?0:event.key==='End'?tabs.length-1:(tabs.indexOf(event.target)+(event.key==='ArrowRight'?1:-1)+tabs.length)%tabs.length;tabs[index].click();tabs[index].focus();}
  }
  buildShell();
  if(scopeError){const reading=document.getElementById('dc-reading');reading.replaceChildren(element('div',scopeError,'dc-empty'));status(scopeError);setDisabled();return;}
  refresh().then(()=>{
    currentView=restore('view')==='slides'?'slides':'lecture';currentSlide=Math.max(0,finite(restore('slide')));mount.dataset.directoryOpen=restore('directory-open')==='0'?'false':'true';renderReading(false);
    if(restore('chat-open')==='1'&&!matchMedia('(max-width:700px)').matches)toggleChat(true);
  });
  document.addEventListener('visibilitychange',exposeVisible);
  window.addEventListener('pagehide',()=>{const input=document.getElementById('dc-chat-input');if(input)store('chat-draft',input.value);stopWatching?.();activeEpoch++;});
})();

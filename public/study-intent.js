/* 首页和导师共用明确学习意图。试卷或资料对话保持当前题目上下文。 */
(() => {
  const yesterday = text => {
    const phrase = String(text || '').normalize('NFKC').trim().replace(/[\s，、,]/g, '');
    return /^(?:(?:我想|我要|今天想|想|请|能不能|可以|麻烦|帮我|带我|教我|陪我|继续|先)){0,6}(?:复习|回顾)(?:一下)?(?:昨天|昨晚|昨日)/.test(phrase)
      || /^(?:昨天|昨晚|昨日)(?:学的|学过的)(?:内容|知识点|课)?(?:想|帮我|带我)?(?:再)?(?:复习|回顾)/.test(phrase);
  };
  const fromHome = text => {
    if (yesterday(text)) return '/review-yesterday';
    if (/^这(?:道)?题(?:我)?不会/.test(text))
      return '/ask?' + new URLSearchParams({draft: text});
    return '';
  };
  window.GangyiStudyIntent = {yesterday, fromHome};
})();

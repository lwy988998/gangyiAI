# 资料与教学整合验收

核验日期：2026-10-14。基线：Windows v5.6.1（651e741）。工作区：D:\gangyiAI-library。当前结果可供继续试用，尚不能宣布完整目标验收通过或正式发布。

## 已合并的教学修正

教学质量分支 6126184 已合入 feat/learning-library-desktop，合并提交 b63bd1d 的父提交为 6baedba 与 6126184。包括具体诊断、实际帮助记录、关联复练、评价反馈展示，以及完整示范题身份。合并时保留资料库、原文引用验证和教学板块的来源字段，补齐报告页所需的 learning-agent.js。

教学专项任务自己的 6/18 案例多轮目标仍未完成；其 100,000 Token 预算记录继续保留在原任务工作区，本报告的单轮资料教学测试不能替代该验收。

## 功能验证

| 项目 | 证据与结论 |
|---|---|
| 分角度真实搜索 | 初学者Python目标实际执行官方概念、常见误区、实例三个查询，并追加别名与引用两个缺口查询。17个候选来源，10次博查HTTP尝试（包含兼容重试），没有因第一批非空就停止。 |
| 阅读与个性化 | 两份Python官方原文完成读取，初学者保存两份推荐；进阶档案复用同一候选与缓存，选择API规范页，理由关联直接定义、就地修改返回值的要求。可变序列与浅拷贝尚未读到的部分明确说明。不是长期学习效果证明。 |
| 原文解析 | 实际公开HTML、原生隔离Readability、PDFium文字页、空/扫描PDF、字幕时间点和无时间戳文字稿分别验证。未知日期和自动字幕状态保持未知；局部读取独立标记。 |
| 数学公式 | 真实OpenStax原文暴露出MathML扁平化重复的问题；已保留分数、上下标及LaTeX注解，未完整支持的复杂公式注明局部读取。原快照保留，修正生成新快照。 |
| 英语来源覆盖 | Cambridge页面只提取到形式表，不能作为完整语法依据。补充读取EnglishClub完整公开内容；正文过短的HTML增加局部覆盖提醒，不把抓取时间冒充发布/更新日期。 |
| 引用安全 | sourceId、snapshotId、segmentId和逐字摘录核对真实数据库；假位置、假摘录、已排除来源拒绝用于新教学。错误内容不提前流给学生；旧课堂仍可回查固定快照。语义是否完整支持结论仍须人工核对。 |
| 摘录与完整原文 | 上下文明确标记摘录范围与完整段落数，read_resource 支持关键词和后续段落读取。实际Python官方快照的copy与None两次关键词读取均命中缓存，0模型请求、0正文获取；不能把前三段未提到说成整份快照没有。此项是原生工具验证，不是模型自动采用工具的效果证明。 |
| 明确刷新 | 用户事件明确刷新指定来源时，即使模型工具参数漏写刷新也执行获取。实际Python官方原文验证新增1次获取、0缓存命中、0模型请求，旧快照逐字保留。 |
| 空档案与中断 | 回归先复现空资料档案编造引用的文字提前展示，再修正为完整校验后展示。接收期间提供核验状态；中断的未校验文字不进入学生对话，不作为帮助或评分证据。 |
| 用户控制 | API专项验证收藏、排除、手动选用、旧版本写入拒绝和独立快照；重启后选择、原文仍保留。没有将阅读或收藏计入掌握度。 |
| 暂停与重试 | 主控专项验证读过的工具步骤暂停后恢复与重复动作复用；真实初学者任务重试1次模型请求后完成，无新增博查或正文请求。 |
| 备份恢复 | 原生启动器专项对完整SQLite备份/恢复检查来源、快照、页码与选择版本；资料使用现有schema 7记录，不另建备份系统。 |
| 用量 | 模型、搜索尝试、正文请求、缓存和局部读取分开；Token只用服务商返回值。发现控制状态丢失后用量键可能覆盖，已改为持久化的独立请求标识并追加回归，历史验收原件保留。 |
| 页面与截图 | `/my-ai/library`、课程与课堂入口和相关API已实现。旧截图为早期界面；最终合并版的桌面、窄屏与截图尚未验证，浏览器工具当前拒绝访问该本地页面，不使用其他通道绕过。 |

## 当前模型的五领域记录

使用当前配置 deepseek-flash。两版使用相同任务、教学偏好、私有标准；基线编译于v5.6.1对象，新版本读取实际保存的原文。每例是一次教学请求和一道新练习，不包含真实学生的长期作答。模型调用最多3次，完整输出上限8,192；达到调用上限算失败，不算有效教学。

| 领域 | 基线结果 / 请求 / Token | 新版结果 / 请求 / Token | 核对结论 |
|---|---|---|---|
| 数学 | 等待作答 / 3 / 13,863 | 等待作答 / 1 / 9,079 | 同单位与通分说明、1/4+1/6=5/12核对正确，三个引用位置真实；生活补充标明。 |
| 理科 | 等待作答 / 2 / 8,298 | 等待作答 / 1 / 8,766 | 经典力学惯性系下合力零不等于速度零，题目私有答案正确，原文引用真实。回复把保存的资料称为“你收藏的”，这个个人行为说法缺乏证据，应避免。 |
| 语言 | 等待作答 / 2 / 12,680 | 等待作答 / 2 / 15,521 | 首次来源只有形式表，使用AI补充；“when已说清先后”表述过宽，未算原文教学通过。换全文来源后收尾重试成功。最新共用规则下另行生成讲解和两空新题，关键时态、私有标准答案和引用核对正确；仍属于接近示范的迁移练习，不能证明独立理解，评分宽松边界需实际作答验证。 |
| 编程 | 失败 / 3 / 11,859 | 等待作答 / 1 / 9,881 | append、extend、None、别名和浅拷贝核对正确；新题a/b为[1,2,3,4]，c为[1]，r为None。引用位置存在，但解释的改写应与逐字原文区分。基线未完成，不能比较完整效果。 |
| 时效 | 失败 / 3 / 16,843 | 等待作答 / 2 / 26,548 | 根据实际Python 3.15官方原文核对2026-10-09发布、UTF-8默认编码和lazy imports；区分3.14旧页面表述。引用及选择题答案C正确。基线未完成。 |

这一轮共20次实际模型HTTP请求、133,338已知Token；全部返回用量。新版合计7次请求/69,795 Token，基线13次/63,543 Token。请求减少不代表总Token下降，不能宣称总体教学效果已经改善。

补充验证与失败探索独立保存：

- 最初数学搜索：7次模型HTTP尝试，4次返回101,523已知Token，3次未提供；4次正文读取未完成。后续修正解析后重新验证，不把这次失败删掉。
- 初学者Python搜索：6次模型请求，104,692 Token；工具步骤已保存但最终协议失败。重试新增1次请求、22,346 Token后完成。重试前的统计记录保留在原DB，不能依靠当时覆盖后的页面合计。
- 进阶排序：3次模型请求、28,278 Token；2次缓存读取、0新增检索/抓取；完成不同推荐与理由的有限验证。
- 五领域早期输出上限误设为2,500，保存6份失败探索，共12次实际请求、71,452已知Token。调整完整输出后执行上表，不将早期失败当作通过。
- 英语换EnglishClub全文：3次模型请求、22,714 Token。有效讲解及新题保存，随后缺少来源标记的收尾回复被拒绝；需要完成该修正的复核。
- 英语收尾复核：追加1次实际模型请求、6,374 Token，任务等待作答，0新增题目、0新增检索或抓取，原3条用量记录完整保留。解决了收尾与幂等问题，不能将示范时间线重复当作独立复练通过。
- 最新合并版英语：1次实际模型请求、8,732 Token，正常等待作答。原文引用对应结构表s7；火车讲解示范标明AI补充；新题为Lucy到站与北京火车开走的两空题，答案reached / had left。没有直接复制示范的时间数据，但情境与关键时态仍相近；没有学生新回答，不作为掌握度或整体效果证据。
- 编程基线原任务重试：使用只链接v5.6.1对象的验收入口，新增3次请求、16,351 Token，仍因两次结构错误及超时未完成。保留失败与原记录，不将重试算作基线通过。
- 版本时效基线原任务重试：新增3次请求、13,477已知Token，1次未提供用量；两次结构错误及超时后仍未完成。未完成的公开文字不作为合格教学证据。
- 进阶编程讲解：2次请求、16,198 Token，正常等待作答；讲解采用API术语、短代码并补充嵌套对象共享限制。与早期初学者讲解不是严格同版本对照；把短摘录未包含知识说成完整快照没有的问题已据此补齐读取工具与范围提示，尚未做该新工具的实际模型采用验证。

以上Token均来自实际回执，缺失部分不估算为零。各阶段使用隔离档案，与用户正式学习数据分开。

## 交付门槛

- 最新整合包26项完整CTest全部通过（1293.31秒），包括新增关键词缓存、明确刷新、空档案假引用和中断回复回归。此前旧断言仍要求显示未校验的部分文字，已随修正更新并完整重跑，失败记录保留。
- 最新学习增强试用版已在本机独立安装，版本5.6.1；桌面快捷方式“钢一定制AI 学习增强试用版”指向独立安装目录。安装退出码0、安装版启动器自检退出码0，程序、解析器与界面文件的SHA256逐项匹配打包文件。上轮安装/卸载保留数据验收原件继续保留；本次安装后不卸载用户正在试用的版本。
- 新增用量恢复专项确认：旧请求用量保留，重复完成回执不另算请求，缺失Token保持未知。真实英语重试也保留原3条记录。
- 英语换源后的收尾协议已复核；新规则下真实讲解、原文结构与新题答案已核对。近似题的迁移强度、评分宽松边界以及实际教学帮助类型仍须多轮作答验证。
- 最终页面预览、桌面与窄屏交互：待验证。
- 教学专项6/18多轮案例与“整体教学改善”结论：未完成。

## 可核对的原件

- [五领域逐项记录](D:/gangyiAI-library/verification/five-domains-final/progress.json)
- [真实初学者任务与重试](D:/gangyiAI-library/verification/live-library-retry/result.json)
- [进阶用户实际推荐](D:/gangyiAI-library/verification/library-advanced/result.json)
- [英语全文来源复核](D:/gangyiAI-library/verification/five-domains-final/language-candidate-reviewed/result.json)
- [英语任务收尾、幂等及用量保留](D:/gangyiAI-library/verification/five-domains-final/language-final-retry/summary.json)
- [最终英语讲解与练习](D:/gangyiAI-library/verification/five-domains-final/language-current-final/result.json)
- [编程基线原任务重试](D:/gangyiAI-library/verification/five-domains-final/programming-baseline-retry/result.json)
- [时效基线原任务重试](D:/gangyiAI-library/verification/five-domains-final/timeliness-baseline-retry/result.json)
- [进阶编程实际讲解](D:/gangyiAI-library/verification/personalized-teaching-advanced/result.json)
- [实际原文关键词缓存验证](D:/gangyiAI-library/verification/actual-keyword-cache/summary.json)
- [实际明确刷新及旧快照保留](D:/gangyiAI-library/verification/actual-explicit-refresh/summary.json)
- [原生解析、缓存及暂停回归](D:/gangyiAI-library/verification/library-bounded-source-tests.log)
- [安装脚本](D:/gangyiAI-library/scripts/test_windows_release.ps1)
- [最新构建、26项回归与打包记录](D:/gangyiAI-library/verification/library-final-package.log)
- [本机安装与安装文件校验](D:/gangyiAI-library/verification/local-library-install-result.json)
- [上轮整合包原生安装验收](D:/gangyiAI-library/verification/merged-native-install.log)
- [最终打包文件核对](D:/gangyiAI-library/verification/merged-artifact-identity.json)
- [真实讲解与题目的阅读报告](D:/gangyiAI-library/verification/five-domains-final/review.html)
- [独立试用安装包](D:/gangyiAI-library/dist/installer/gangyiAI-learning-preview1-v5.6.1-x64.exe)
- [独立免安装ZIP](D:/gangyiAI-library/dist/installer/gangyiAI-learning-preview1-v5.6.1-x64.zip)
- [校验值](D:/gangyiAI-library/dist/installer/SHA256SUMS.txt)
- [教学专项验收](D:/gangyiAI-teaching/docs/teaching-acceptance.md)

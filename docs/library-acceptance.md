# 资料与教学整合验收

核验记录更新：2026-10-16；本次完整回归与本机安装执行于2026-10-14。基线：Windows v5.6.1（651e741）。工作区：D:\gangyiAI-library。当前结果可供继续试用，尚不能宣布完整目标验收通过或正式发布。

## 已合并的教学修正

教学质量分支 6126184 经 b63bd1d 合入；最新 53fa8a0 经 c8385c7 合入 feat/learning-library-desktop。包括具体诊断、实际帮助记录、关联复练、评价反馈展示、完整示范题身份与实际展示过的其他题索引。最新工具说明补齐 helpType 和原题关联，避免模型漏填后重复报错。合并保留资料引用验证、私有评分标准、输出截断恢复、唯一请求用量标识与资料任务收尾规则。

教学专项任务自己的 6/18 案例多轮目标仍未完成。其最新累计上限为 1,000,000 Token，已知已用 839,528，未知请求预留 72,075，可分配 88,397；旧100,000记录不代表当前上限。历史仅数学有完整双版本配对，最新修正后的6案例及扩展12案例均未验收。本报告的单轮资料教学测试不能替代该验收。

## 功能验证

| 项目 | 证据与结论 |
|---|---|
| 分角度真实搜索 | 初学者Python目标实际执行官方概念、常见误区、实例三个查询，并追加别名与引用两个缺口查询。17个候选来源，10次博查HTTP尝试（包含兼容重试），没有因第一批非空就停止。 |
| 阅读与个性化 | 两份Python官方原文完成读取，初学者保存两份推荐；进阶档案复用同一候选与缓存，选择API规范页，理由关联直接定义、就地修改返回值的要求。可变序列与浅拷贝尚未读到的部分明确说明。不是长期学习效果证明。 |
| 原文解析 | 实际公开HTML、原生隔离Readability、PDFium文字页、空/扫描PDF、字幕时间点和无时间戳文字稿分别验证。未知日期和自动字幕状态保持未知；局部读取独立标记。 |
| 数学公式 | 真实OpenStax原文暴露出MathML扁平化重复的问题；已保留分数、上下标及LaTeX注解，未完整支持的复杂公式注明局部读取。原快照保留，修正生成新快照。 |
| 英语来源覆盖 | Cambridge页面只提取到形式表，不能作为完整语法依据。补充读取EnglishClub完整公开内容；正文过短的HTML增加局部覆盖提醒，不把抓取时间冒充发布/更新日期。 |
| 引用安全 | sourceId、snapshotId、segmentId和逐字摘录核对真实数据库；假位置、假摘录、已排除来源拒绝用于新教学。错误内容不提前流给学生；旧课堂仍可回查固定快照。语义是否完整支持结论仍须人工核对。 |
| 摘录与完整原文 | 上下文明确标记摘录范围与完整段落数，read_resource 支持关键词和后续段落读取。实际Python官方快照的copy与None两次关键词读取均命中缓存，0模型请求、0正文获取；不能把前三段未提到说成整份快照没有。另有实际模型调用关键词和后续段落工具的记录；不能把原生夹具等同于模型采用。 |
| 明确刷新 | 用户事件明确刷新指定来源时，即使模型工具参数漏写刷新也执行获取。实际Python官方原文验证新增1次获取、0缓存命中、0模型请求，旧快照逐字保留。 |
| 空档案与中断 | 回归先复现空资料档案编造引用的文字提前展示，再修正为完整校验后展示。接收期间提供核验状态；中断的未校验文字不进入学生对话，不作为帮助或评分证据。 |
| 用户控制与排序 | API专项验证收藏、排除、手动选用、旧版本写入拒绝和独立快照。a062288保存模型items推荐顺序，资料列表与教学上下文一致；原生夹具验证两份都选用时可交换顺序，手动选用优先、排除不能被模型撤销。旧记录没有rank时保持原有次序，没有将资料阅读计入掌握度。 |
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
- 进阶编程讲解：2次请求、16,198 Token，正常等待作答；讲解采用API术语、短代码并补充嵌套对象共享限制。与早期初学者讲解不是严格同版本对照；把短摘录未包含知识说成完整快照没有的问题已据此补齐读取工具与范围提示，后续实际模型采用验证见下文；不把不同版本、不同任务的讲解当作严格配对。

## 关键词查阅与复杂任务收尾的后续核验

- 实际模型使用关键词和翻页工具：前三阶段6次HTTP共111,854已知Token、12次缓存读取；最后一笔达到8,192输出上限，任务没有完成。原件保留。
- SDK现区分`finish_reason=length`与其他失败，不重复以同一上限重试。主控保留已执行动作，最多自动恢复一次到16,384；仍超限时停止并说明缩小范围。完整格式及引用验证前不展示部分教学文字。免费回归验证普通响应、流式响应、一次恢复及再次超限停止。
- 恢复阶段2次HTTP共58,276已知Token、4次缓存读取仍未收尾；收尾规则阶段2次HTTP超时，服务商未返回Token，保持未知。
- 资料任务首笔限时调整为120秒，收尾规则要求保存当前已核对推荐并标明未覆盖范围。后续同一原任务新增1次HTTP，服务商耗时71,030毫秒、37,443实际Token，状态`ready`；没有新增搜索、正文获取或缓存读取。两份进阶推荐保存，已有快照与用量原件逐字保留。
- 上述后续阶段共11次HTTP，9次提供207,573已知Token、2次未提供；失败和超时保留。实际成功没有触发16K自动恢复，不能以此替代截断恢复专项。
- 成功回执记录的HEAD为8a64b4f，当时尚未提交的源码哈希已与d75dc446的内容核对一致；不将这次记录改称最新c8385c7整合版模型验证。
- 五个引用位置均对应原快照，浅拷贝定义和原地修改返回None的原则可核对。s164是重复序列的别名例子，不能单独证明浅拷贝的嵌套对象语义；完整copy/deepcopy依据仍待补查，不算完整原文支持。
- 模型真实推荐顺序是规范页在先、教程在后；该次原始记录尚未保存排序。a062288补齐持久化，专项验证排序契约，不篡改模型原件。读取与推荐后的学科、知识点掌握记录仍为零。

以上Token均来自实际回执，缺失部分不估算为零。各阶段使用隔离档案，与用户正式学习数据分开。

## 交付门槛

- c8385c7整合包26项完整CTest全部通过（105.40秒），日志library-teaching-combined-package.log；包括资料排序、教学示范索引、帮助字段、唯一用量和输出截断回归。此前1293.31秒与103.75秒的构建记录保留，不冒充本次结果。
- 历史安装：c8385c7学习增强试用版于10月14日在本机独立安装，版本5.6.1；桌面快捷方式“钢一定制AI 学习增强试用版”指向独立安装目录。安装退出码0、安装版启动器自检退出码0，8项程序与界面SHA256逐项匹配；安装前4个试用档案文件逐字保留。此前安装/卸载验收原件保留。10月16日用户明确要求先删除所有本机版本，再在两端完成及合并后更新。
- 安装包SHA256：840906db7acf60c8d1ad41130e5d87114f267355d81b7cb715c3e774d18bed02；ZIP：a1f9e15179d2f4f5f449d3a29da371b481c3e11ba5114d4d92b5010cba4f65b2。
- 10月16日已按用户要求卸载正式版、教学优化试用版、学习增强试用版，三次退出码均为0；三个卸载登记及主程序移除，13个档案文件逐字保留，接口凭据与配置经本机DPAPI加密备份和恢复核对。最终整合安装尚待教学专项修复交付。
- 教学专项后续发现“长板块只露出一部分却按完整展示”的缺陷；该修复尚未提交、未包含在c8385c7安装中，交由教学专项完成后再整合。
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
- [实际关键词采用阶段及失败记录](D:/gangyiAI-library/verification/actual-keyword-model/summary.json)
- [复杂资料任务完成、源码和引用核对](D:/gangyiAI-library/verification/library-research-recovery-live/summary.json)
- [三版卸载与档案保留](D:/gangyiAI-library/verification/local-all-versions-uninstall-result.json)
- [整合版本机安装与档案保留](D:/gangyiAI-library/verification/local-library-combined-install-result.json)
- [整合版完整回归](D:/gangyiAI-library/verification/library-teaching-combined-package.log)
- [教学后续配对与预算清单](D:/gangyiAI-teaching/verification/remaining-acceptance-1m.json)
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

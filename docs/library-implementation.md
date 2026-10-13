# 个性化联网资料实施记录

基线：Windows v5.6.1，独立分支 feat/learning-library-desktop，工作区 D:\gangyiAI-library。

## 当前实现

- 博查按主控指定的查询角度、日期范围和结果数量搜索，记录实际请求与缓存命中；旧搜索路径继续收集各角度结果。
- 资料、原文快照、查询记录、推荐选择与使用情况保存到现有版本化 SQLite 记录。
- 新页面 /my-ai/library 支持列表、课程筛选、收藏、排除、手动选用、原文回查和主动检索。
- 新接口：GET/POST /api/my-ai/library、POST /api/my-ai/library/research、GET /api/my-ai/library/usage。
- 主控资料工具：read_library、search_resources、read_resource、select_resources；资料任务限制在资料工具范围。
- 教学板块保存内容来源与可验证引用；不存在的原文位置、虚构摘录和已排除资料拒绝提交。
- 网页读取使用 Readability 0.6.0 隔离解析，PDF使用 PDFium 8100 非V8动态库，字幕支持公开VTT、SRT及时间戳JSON。

## 需要继续验证的交付门槛

以下未完成项不能据此认定目标已经完成：

- 编译与专项回归、现有核心回归。
- 原生网页与PDF解析、字幕读取和公开网络访问。
- 多角度检索、日期、去重、资料矛盾、背景适配、任务暂停与重试的实际行为。
- 浏览器与桌面、窄屏预览；课堂和AI导师共享资料。
- 备份、恢复、跨窗口修改、历史快照及离线读取。
- 当前模型的数学、理科、语言、编程、时效知识真实教学对照。
- 引用的教学优化任务成果复核与最终试用包的兼容整合。
- 独立试用安装包、校验文件、页面预览和逐项验收报告。

## 依赖锁定

- Readability：提交 04fd32f72b448c12b02ba6c40928b67e510bac49，下载校验 Git 内容摘要，保留 Apache 2.0 许可证。
- PDFium：chromium/8100，pdfium-win-x64.tgz 的 SHA256 为 51621bcc1d9be9ea448f4b84b0516862b928e3968d3fca899af1a6d6e2848d6d；打包保留主许可证与第三方许可证。
- 资料内容只作为工具结果，不提升为系统指令。来源读取限制为公开HTTP/HTTPS，最终连接地址与各次重定向均校验。

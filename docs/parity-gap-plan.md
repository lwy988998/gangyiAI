# 钢一定制AI（C++ 版）与 AILINES 校园版 对齐差距清单与分阶段方案

> 依据仓库：
> - C++ 重写版：`D:\钢一定制AI`
> - 校园版（对照基准）：`D:\AILINES-CAMPUS`（Next.js App Router，品牌「钢一定制AI」）
> 用途：本文件是 C++ 版用户体验与校园版对齐的验收依据。每个阶段完成时按「验收标准」逐条对照。
> 生成方式：只读对比（未修改任何源代码）。

---

## 1. C++ 版当前执行阶段（按自身 git 历史）

| 阶段 | 内容 | 状态 |
|---|---|---|
| 阶段0 | 项目骨架（Crow HTTP + 配置 + 健康检查 + third_party 本地化 + 部署脚本） | ✅ |
| 阶段1 | 数据层（10 表 CRUD + 索引 + 全表测试） | ✅ |
| 阶段2a | AI 客户端（curl + 重试/退避/熔断/fallback + JSON 容错解析）+ AI 行为规格文档 | ✅ |
| 阶段2b | AI 生成逻辑：计划生成 ✅、微课程生成 ✅、问答 ❌（桩）、阶段展开 ❌、计划缓存 ❌ | ⚠️ ~40% |
| 阶段3 | 认证 / 搜索 / 会话等业务链路 | ❌ 未开始 |
| 阶段4 | 前端 SSR 页面（home/plan/learn/progress/login/ask/my-courses） | ✅ 骨架版（功能深度不足） |

**整体进度估计：40–50%。** 地基（HTTP/数据/AI 客户端）扎实，用户体验层差距大。

---

## 2. 功能差距矩阵

### 2.1 逐页面

| 页面 | 校园版 | C++ 版 | 差距 |
|---|---|---|---|
| 首页 `/` | 聊天式输入（Enter 发送）、图片上传识别目标（≤5MB）、随模式切换的动态示例词（高中词库 26 条取 5）、注册入口、移动端输入框吸底 | textarea + 快速/深度 radio + 4 张静态能力卡 | 大 |
| 课程规划 `/plan` | SSR 生成 + AilinesGeneratingState 动画（进度条封顶 94%）；lite/deep 双视图（LitePlanView / CoursePlanView：slides 课件、mindMap、roadmap、courseStructure、resources、projects）；联网搜索补充资源（8s 超时降级）；生成即保存课程+快照并 replaceState 为 courseId 版 URL；forcePlan/bypassCache/retry 重生成；7 天计划缓存；富失败态（重试/降级快速规划/返回首页） | CSR fetch /api/generate-plan + 简单卡片（title/summary/phases）；无资源、无保存、无缓存、无双视图；失败态仅"重试"链接 | 巨大 |
| 阶段页 `/phase` | 阶段展开（导学/步骤/任务/课件/检查清单）、思维导图、任务三态勾选、步骤理解三态、学习卡进度、悬浮助手 | 路由不存在 | 缺失 |
| 学习页 `/learn` | SSR；会话恢复（同课同主题读库，兜底内容不恢复）；联网搜索（8 条资源评分排序）；分步练习勾选、练习题（details 提示）、小测验本地即时判分（≥70% 通过）；完成按钮（localStorage+API 双写）；进度三表上报 + 异步全量重算；换一版讲解；断点续学记录；悬浮助手 | 页面壳 + /api/learn 一次拉取；无互动/判分/完成按钮/会话恢复/进度上报；phaseName/topic 退化为「阶段 X / 主题 Y」 | 巨大 |
| 进度页 `/progress` | 实时进度（三表 completed/total 比例 + 快照估算）、三态任务卡 + 重置、断点续学入口、快照质量校验 | 静态占位（40%、示例阶段写死） | 缺失（占位） |
| 我的课程 `/my-courses` | 真实列表（状态徽章 未开始/学习中/已完成、进度条、继续学习/查看课程/删除）、统计四卡、未登录引导、骨架屏 | 静态占位（示例课程卡写死） | 缺失（占位） |
| 问答 `/ask` | SSR 真实问答（generateAskAnswerWithAI，1500/2000 tokens，25s 超时）+ 聊天 UI（气泡/步骤/命令块复制）+ 示例问题 | 静态占位 + /api/ask 桩 | 大 |
| 登录 `/login` | 完整登录（scrypt、session cookie ailines_session、匿名数据绑定） | 表单提交到桩接口 | 大 |
| 注册 `/register` | 完整注册（邮箱校验、密码≥6、注册即登录+匿名绑定） | 路由不存在 | 缺失 |
| 学习路线 `/routes` | RoutesClient（localStorage 路线列表） | 路由不存在 | 缺失（孤儿页，可取舍） |
| 管理后台 `/admin` | overview 统计、用户列表（搜索/分页/筛选）、课程列表、tier 调整（ADMIN_EMAILS 白名单） | 不存在 | 缺失 |
| 悬浮助手 | FloatingAilinesChat：页面语境 + /api/learning-chat（8s 搜索超时、历史≤8、引用≤3 折叠）+ 按语境键 localStorage 持久化 | 不存在 | 缺失 |
| 图片识别目标 | /api/analyze-image-goal（视觉模型，失败降级为文字目标） | 不存在 | 缺失（可选） |

### 2.2 能力层

| 能力 | 校园版 | C++ 版 |
|---|---|---|
| 计划生成（Level 1 骨架） | ✅ | ✅ 提示词/参数几乎一致 |
| 微课程生成 | ✅ | ✅（resources 为空时行为一致） |
| 计划缓存（7 天 sha256） | ✅ | ❌ |
| 联网搜索（Tavily 主+Bocha 备、领域查询、归一化/评分/去重、7 天缓存） | ✅ | ❌ |
| 问答生成 | ✅ | ❌ 桩 |
| 阶段展开生成（失败返回 null 降级） | ✅ | ❌ |
| 上下文学习聊天（25s 超时、资源≤5、历史≤6） | ✅ | ❌ |
| 多 provider 容错 | ✅ 4 槽位 + /models 健康探测 + 400 去 response_format 重试 | ⚠️ 2 槽位 + 熔断（无探测、无 400 重试） |
| JSON 容错解析 | ✅ | ✅ 几乎逐行复刻 |
| 质量门禁（泛化黑名单/结构/关键词/重复/动作产出/得分≥55/来源标记） | ✅ 展示与保存前强校验 | ⚠️ 仅简版 validate |
| 课程/快照持久化（12s 超时、脱敏、900KB 裁剪） | ✅ | ⚠️ 表在（db.cpp），路由未接线 |
| 学习会话恢复 | ✅ | ⚠️ 表在，未接线 |
| 进度三表 + 异步重算 | ✅ | ⚠️ 表在，未接线 |
| 认证 | ✅ | ❌ 桩 |
| 会员/额度 | ✅ **但校园版测试模式全放行** | 无需实现闸门 |
| 管理员 | ✅ | ❌ |

---

## 3. 前后端架构差异

| 维度 | 校园版（Next.js） | C++ 版（Crow） |
|---|---|---|
| 渲染模型 | SSR 直出（生成发生在页面渲染时），Suspense + AilinesGeneratingState 动画兜底 | 页面壳 + CSR fetch（内联 script 填充） |
| 数据流 | 页面直接调 repository/AI 函数 | 独立 REST API + 静态文件 |
| 身份模型 | cookie session + anonymousId 双轨，服务端读 cookie | 无 cookie/匿名概念（登录桩） |
| 前端交互 | React 客户端组件（判分/勾选/上报/聊天均为客户端状态） | 内联 `<script>` |
| 移动端适配 | 专项 CSS（按钮单列全宽、字号阶梯降级、禁缩放、长文本换行） | 复用校园版编译 CSS（104KB），缺交互组件 |
| 静态资源 | Next.js 按需打包 | 直接复制编译产物 |

**本质差异**：校园版是「SSR + React 客户端交互」，C++ 版是「静态模板 + 少量内联 JS」。UX 对齐的最大工作量在把交互组件（判分测验、勾选、进度上报、聊天助手）用原生 JS 重写。

---

## 4. 关键结论

1. **会员/额度无需实现**：校园版 `lib/membership/permissions.ts`（canUseFeature 恒 true）与 `usage.ts`（checkUsageLimit 恒 allowed）均为「校园版测试模式：暂时开放全部功能」，C++ 版对齐 UX 不需要任何会员闸门；`UsageCounter` 表保留记账即可。
2. **数据层已对齐**：C++ db.cpp 的 10 表与校园版 prisma schema 一一对应（Course/User/UserSession/UsageCounter/CourseProgress/CourseSnapshot/TaskProgress/LearningStepProgress/LearningCardProgress/LearningSession），缺的是「路由接线」与 repository 级行为（trim/裁剪/脱敏/唯一键 upsert/异步重算）。
3. **AI 核心已对齐大半**：计划骨架 + 微课程生成与校园版提示词/参数几乎一致；缺缓存、搜索、问答、阶段展开、上下文聊天。
4. **质量门禁是 UX 的隐性支柱**：校园版「AI 输出不完整或模板化 → 不展示假内容 → 三选一状态页」的体验，依赖门禁逻辑，C++ 版需补齐到同等强度。

---

## 5. 分阶段对齐方案（每阶段含验收标准）

> 顺序按「用户主旅程」优先；每阶段保持可构建可运行；对照验收建议打开校园版同屏比对。

### 阶段 A：课程生成主链路对齐
1. 计划缓存：7 天文件缓存（sha256(goal+mode) + 结构校验），`bypassCache/forcePlan/retry` 跳过
2. 课程持久化接线：`POST /api/courses` + `GET /api/courses/[id]`（Course + CourseSnapshot，脱敏/裁剪/12s 超时），生成成功即保存，/plan 成功后把 URL 替换为 courseId 版
3. /plan 双视图：lite → LitePlanView 等效；deep → CoursePlanView 等效（roadmap/courseStructure/slides/mindMap/resources/projects 全区块）
4. 联网搜索资源：先接 Tavily（7 天缓存），Bocha 作 fallback；8s 超时静默降级
5. 富失败态：错误类型映射文案 + 重试/降级快速规划/返回首页
- **验收**：首页输入目标 → /plan 生成 → 保存 → 「我的课程/历史课堂」恢复；双模式逐屏与校园版一致

### 阶段 B：学习体验对齐
6. /phase 阶段页：阶段展开生成 + 任务三态 + 步骤理解三态 + 卡片进度上报
7. /learn 会话恢复：LearningSession 唯一键 upsert；fallbackUsed/source=fallback 的会话不恢复（提示重试）
8. 互动组件：分步练习勾选、练习题 details 提示、小测验本地判分（≥70% 通过）、完成按钮（localStorage+API 双写）
9. 进度上报：卡片/步骤/任务三表 + recomputeCourseProgress（异步、percent clamp 0-100、状态白名单归一化）
10. 断点续学：lastVisited 记录（相对路径 + courseId 匹配安全校验）
- **验收**：学完一节 → 标记完成 → /progress 百分比真实变化；同课同主题刷新恢复上次内容

### 阶段 C：进度与我的课程对齐
11. /progress 真实数据：快照估算 total（多候选键）、三表统计 completed、任务三态卡 + 重置
12. /my-courses 真实列表：状态徽章/进度条/继续学习/删除 + 统计四卡 + 未登录引导
- **验收**：课程全生命周期（生成→学习→完成→删除）闭环，数字与校园版一致

### 阶段 D：问答与助手
13. /api/ask 真实生成（generateAskAnswer 移植：1500/2000 tokens、25s 超时、命令块展示）
14. 悬浮助手 + /api/learning-chat（contextual chat + 页面语境 + 引用折叠 + 历史消息）
15. 图片识别目标（可选）：/api/analyze-image-goal（视觉模型，失败降级文字目标）
- **验收**：问答题与悬浮助手回答质量、引用展示与校园版一致

### 阶段 E：账号与后台
16. 登录/注册真实化：scrypt + session cookie + 匿名数据绑定（Course/CourseProgress 归属迁移）+ 隐性领养
17. /admin：overview 统计 + 用户列表 + tier 调整（ADMIN_EMAILS 白名单）
18. /routes 学习路线页（localStorage，可取舍）
- **验收**：注册 → 匿名数据合并 → 登录态全站生效；管理后台可查用户/课程/调 tier

---

## 6. 附录：关键证据位置

- 会员放行：`D:\AILINES-CAMPUS\lib\membership\permissions.ts:29-34`、`usage.ts:54-59`
- 会员矩阵：`D:\AILINES-CAMPUS\lib\membership\tiers.ts:5-39`
- 质量门禁：`D:\AILINES-CAMPUS\lib\courseContentQuality.ts:406-488`（valid = 无 fatal 且 score≥55）
- 计划生成：`D:\AILINES-CAMPUS\lib\ai\generatePlan.ts:9-120`、`generatePlanPrompt.ts`（Level 1 骨架，C++ 已复刻）
- 搜索：`D:\AILINES-CAMPUS\lib\search\searchResources.ts:91-150`、`learningDomain.ts:3-59`
- 进度重算：`D:\AILINES-CAMPUS\lib\course\courseProgressRepository.ts:89-127,157-217`
- C++ 端对照：`D:\钢一定制AI\src\main.cpp`（路由全景）、`src\db.cpp`（10 表）、`src\plan_generator.cpp`、`src\learning_generator.cpp`、`src\page_renderer.cpp`（SSR 页面）

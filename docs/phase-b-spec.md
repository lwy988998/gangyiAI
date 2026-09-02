# 阶段 B 实现规格：学习体验对齐

> 依据：`docs/parity-gap-plan.md` 阶段 B；对照基准 `D:\AILINES-CAMPUS`。
> 本文件是阶段 B 的编码契约：后端端点契约、数据行为、前端组件规格。实施时无需再读校园版源码。
> 数据表已就绪（db.cpp 10 表：CourseProgress/TaskProgress/LearningStepProgress/LearningCardProgress/LearningSession）。

---

## B1. 后端 API 契约（C++ Crow 路由）

### B1.1 `POST /api/learning-card-progress`（学习卡状态）
**请求**：
```json
{ "courseId": "?", "anonymousId": "?", "goal": "...", "mode": "lite|deep",
  "phaseIndex": 1, "phaseName": "...", "topicIndex": 1, "topicTitle": "...", "status": "not_started|in_progress|completed" }
```
**行为**：
- status 白名单：`not_started / in_progress / completed`，非法值归一化为 `not_started`
- 唯一键：`(courseId, phaseIndex, topicIndex)` 存在则 update，否则 insert；无 courseId 时按 `(anonymousId + goal + mode + phaseIndex + topicIndex)` 查找兜底
- topicTitle/phaseName trim 后截 500；phaseName 缺省 `阶段{phaseIndex}`
- 写入成功后**异步**触发 `recomputeCourseProgress`（fire-and-forget，失败仅记日志）
**响应**：`{ok:true, item:{...}}`（保存后的完整记录）

### B1.2 `POST /api/learning-step-progress`（步骤理解状态）
- status 白名单：`unset / understood / review`，非法→`unset`
- 唯一键：`(courseId, phaseIndex, stepIndex)`；无 courseId 兜底 `(anonymousId+goal+mode+phaseIndex+stepIndex)`
- 写后异步 recompute
**请求/响应结构同 B1.1**（stepIndex/stepTitle 替代 topicIndex/topicTitle）

### B1.3 `POST /api/task-progress`（阶段任务状态）
- status 白名单：`not_started / in_progress / completed`，非法→`not_started`
- 唯一键：`(courseId, phaseIndex, taskIndex)`；写后异步 recompute
**请求/响应结构同 B1.1**（taskIndex/taskTitle 替代 topicIndex/topicTitle）

### B1.4 `POST /api/learn/progress`（三合一：学习完成上报）
**请求**：
```json
{ "courseId": "?", "anonymousId": "?", "goal": "...", "mode": "lite|deep",
  "phaseIndex": 1, "phaseName": "...", "topicIndex": 1, "topicTitle": "...",
  "status": "completed",
  "lastVisitedUrl": "/learn?courseId=...", "lastPageType": "learn" }
```
**行为**：
1. upsert learning-card-progress（topicIndex 为 0 时归一化为 1；phaseIndex 必须 ≥1）
2. updateLastVisited（pageType=learn）：lastVisitedUrl 截 1200、lastPageType 截 40、lastPhaseIndex/Name/lastTopicIndex/Title 更新（名称截 500）
3. recomputeCourseProgress（异步）
**响应**：`{ok:true, progress:{overallPercent, completedCount, totalCount}}`

### B1.5 `POST /api/course-progress`（进度重算 / 断点记录）
**请求**（action 二选一）：
```json
{ "action": "recompute", "courseId": "..." }
{ "action": "lastVisited", "courseId": "...", "goal": "...", "mode": "...",
  "lastVisitedUrl": "/learn?...", "lastPageType": "learn", "lastPhaseIndex": 1, "lastPhaseName": "...", "lastTopicIndex": 1, "lastTopicTitle": "..." }
```
**行为**：
- `recompute`：全量重算并 upsert CourseProgress（见 B2）
- `lastVisited`：更新断点字段（lastVisitedUrl 必须是站内相对路径且以 `/` 开头且不含 `//`，否则忽略；pageType 截 40；名称截 500）
**响应**：`{ok:true, progress:{...}}`

### B1.6 `GET /api/learning-sessions` + `POST /api/learning-sessions`（微课会话存取）
**GET** `?courseId&anonymousId&goal&mode&phaseIndex&phaseName&topicIndex&topicTitle`：
- 优先 `(courseId, phaseIndex, topicIndex)`；无 courseId 时按 `(anonymousId+goal+mode+phaseIndex+topicIndex)` 最新一条
- 命中返回 `{ok:true, session:{id, title, summary, content, references, fallbackUsed, source, updatedAt}}`；未命中 `{ok:true, session:null}`
**POST**：
```json
{ "courseId": "?", "anonymousId": "?", "goal": "...", "mode": "lite|deep",
  "phaseIndex": 1, "phaseName": "...", "topicIndex": 1, "topicTitle": "...",
  "title": "...", "summary": "...", "searchQuery": "...", "content": {...微课完整JSON...},
  "references": [...], "fallbackUsed": false, "source": "ai|fallback" }
```
- 唯一键 `(courseId, phaseIndex, topicIndex)` upsert；content 存 JSON 字符串（≤900KB，超限裁剪 lessonSteps≤8/examples≤6/practice≤8/references≤8）
**响应**：`{ok:true}`

### B1.7 `POST /api/phase-expansion`（阶段展开生成）
**请求**：
```json
{ "goal": "...", "mode": "lite|deep", "phaseIndex": 1, "stage": "阶段名", "topics": ["..."], "resources": [...] }
```
**响应**（成功）：
```json
{ "ok": true, "phase": { "objective": "...", "overview": "...", "steps": [{"title","explanation","example","action","check"}],
  "tasks": [{"title","description","duration","output","actionSteps":[],"checklist":[]}],
  "checklist": ["..."], "commonMistakes": ["..."] } }
```
**失败**：AI 生成失败或质量不过 → `{ok:false, message:"阶段内容暂未生成完成，请稍后重试。"}`（HTTP 200，前端展示降级状态）
**参数**：maxTokens lite 6000 / deep 8000（v4-flash 防截断）；单次超时 lite 90s / deep 120s；response_format=json_object；temperature 0.3；失败重试一次（带"完整未截断"后缀）；仍失败返回 null（调用方降级）

### B1.8 `/learn` 页面服务端逻辑（会话恢复优先）
- 参数：goal/mode/phaseIndex/phaseName/topicIndex/topic/courseId/anonymousId/regenerate/forceLearn/retry
- 有 courseId 且非强制重新生成（无 regenerate/forceLearn/retry）时：先查 LearningSession → 命中且 `fallbackUsed=false` 且 `source!="fallback"` 时**直接返回已存内容**（标记"已恢复上次生成的学习内容"）；命中但 fallback → 不恢复（提示重新生成）
- 未命中 → 生成新微课 → upsert 会话
- 生成前可选联网搜索（resources 传入，≤8 条）

---

## B2. recomputeCourseProgress 逻辑（对齐校园版 courseProgressRepository.ts）

1. 读课程最新快照 payload（MockPlan 形状）
2. **估算总量**：
   - tasksTotal = 每个 roadmap 阶段取 tasks/checklist/practices/projects 任一非空数组的长度之和
   - cardsTotal = courseStructure 各阶段 topics 长度之和（courseStructure 缺失时用 roadmap 的 topics/learningCards/cards）
   - stepsTotal = roadmap 各阶段 steps 长度之和
   - totalCount = tasksTotal + cardsTotal + stepsTotal
3. **统计完成量**：completed = TaskProgress.completed 数 + LearningStepProgress.understood 数 + LearningCardProgress.completed 数（courseId 过滤）
4. **百分比**：total==0 → 0；否则 `round(completed/total*100)` clamp 0-100
5. upsert CourseProgress（courseId 唯一）

## B3. 前端组件规格（SSR 模板 + 原生 JS）

### B3.1 `/learn` 微课页（改造现有 renderLearnPage）
在现有渲染基础上增加互动区（数据来自 /api/learn 返回的微课 JSON：lessonSteps/practice/quiz/checkpoint/commonMistakes/references）：
- **分步练习**：每 step 可点击勾选（页面会话态），显示 x/y 步已完成计数
- **练习题**：难度徽标 + 任务 + `<details>` 查看提示/参考答案
- **小测验**：单选 4 选项，点选**本地即时判分**（正确绿/错误红 + 解析）；全答完显示结论（≥70% 通过"小测通过，可以继续下一节"，否则"得分偏低，建议先复习"）；重新作答按钮；AI 未产出合法测验时显示 amber 提示（绝不用模板补齐）
- **完成按钮**：读取 localStorage `ailines-progress-topic:{goal}:{phaseName}:{topic}` 合并状态；点击"标记本节已完成"→ 乐观更新 + 写本地 + `POST /api/learn/progress`；同步状态文案（正在同步/已保存/已保留本地）
- **换一版讲解**：链接带 `regenerate=1&forceLearn=1&retry=<ts>`
- **继续下一节**：基于 courseStructure 计算下一个 topic/阶段
- localStorage 键：`ailines-progress-topic:{goal}:{phaseName}:{topic}` + 完成 ID 列表 `ailines-completed-topics`（或类似）

### B3.2 `/phase` 阶段页（新页面 renderPhasePage）
- 参数：goal/mode/phaseIndex/phaseName/courseId/anonymousId
- 结构：返回课程大纲按钮 + "第 N 阶段" 徽标 + 阶段标题 + 进度卡（已完成/总节数 + 进度条）+ 主按钮（开始本阶段学习/继续）+ 问钢一定制AI
- **主题（学习点）网格**：从课程快照 courseStructure[phaseIndex] 的 topics 生成卡片（带状态徽标：已完成/学习中/未开始），点击跳 `/learn?courseId&phaseIndex&topicIndex&topic`
- **阶段展开区**：`POST /api/phase-expansion` 拉取 → 渲染 objective/overview/steps（讲解+示例+行动+检查）/tasks（任务卡：标题/描述/时长/产出 + 三态切换 未开始/进行中/已完成 + 展开详情）/checklist（绿勾列表）/commonMistakes（amber 列表）；失败显示降级文案
- **步骤理解三态**：每 step 三按钮 已理解/需要复习/取消标记 → localStorage `ailines-phase-step-understanding:{goal}:{mode}:{phaseIndex}:{phaseName}` + `POST /api/learning-step-progress`
- **任务三态**：`POST /api/task-progress` + localStorage `ailines-phase-tasks:{goal}:{mode}:{phaseIndex}:{phaseName}`
- 无 courseId 时：可从 URL goal+mode+phaseIndex 生成（阶段名取自 URL）

### B3.3 `/progress` 进度页（改造现有 renderProgressPage）
- 参数：goal/mode/courseId/anonymousId
- 数据：`GET /api/courses/{courseId}` 快照 → 估算进度阶段；`POST /api/course-progress {action:recompute}` 或直接读 CourseProgress
- **总进度区**：大号百分比 + 已完成 x/y + 最近学习位置 + 进度条
- **任务卡**：从快照 roadmap 生成每阶段一卡（标题 + 完成徽标 + 任务行：状态图标/徽标 + 动作链接 开始学习/继续学习/已学完 → /learn 或 /phase；点击自动置 in_progress；行内"标记完成/取消完成"按钮 → `POST /api/task-progress` + localStorage）
- **重置进度**：confirm 后清 localStorage + 重置本地状态（不删服务端）
- localStorage 键：`ailines-progress-topic:{goal}:{phaseName}:{topic}`（与 learn 共用）

### B3.4 前端共用规则
- 所有跨页链接携带 goal/mode/courseId/anonymousId
- 互动状态"乐观更新 → 写 localStorage → 异步 POST → 失败降级本地并改同步文案"
- 按钮/卡片沿用现有 Tailwind class 风格；移动端按钮组单列全宽

---

## B4. C++ 模块划分

| 模块 | 职责 | 归属 |
|---|---|---|
| `include/progress_service.hpp` + `src/progress_service.cpp` | 进度三表/会话的 upsert+查询+状态白名单、recomputeCourseProgress、lastVisited 更新（repository 层，db.cpp 之上） | 新文件（Hermes） |
| `include/phase_generator.hpp` + `src/phase_generator.cpp` | generatePhaseExpansion 移植（AI 生成阶段展开，复用 ai_client/parseAIJson） | 新文件（Hermes） |
| `src/main.cpp` | B1 全部路由接线 + /learn /phase /progress 页面参数传递 | 修改（主 agent） |
| `src/page_renderer.cpp` | B3 三个页面的 SSR 模板 + 内联 JS | 修改（opencode） |
| `public/` | 新增 plan 之外的前端资源（如 progress.css/phase.css 或合并进现有） | 新增（opencode） |
| `CMakeLists.txt` | 追加新源文件 | 修改（主 agent） |

## B5. 验收标准（对应 parity-gap-plan 阶段 B）

1. /learn 互动：勾选分步、做练习、测测验（≥70% 通过判定）、标记完成 → 刷新 /progress 看到百分比变化
2. 会话恢复：同一课程同一主题重新打开 /learn 恢复上次内容（非 fallback）；fallback 内容不恢复
3. /phase：阶段展开生成 + 任务/步骤三态持久化（localStorage+API）
4. /progress：真实百分比 + 三态任务卡 + 重置；lastVisited 断点可回到上次位置
5. 进度三表写入后 CourseProgress 自动重算（数字与校园版一致）

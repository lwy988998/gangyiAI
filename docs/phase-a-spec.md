# 阶段 A 实现规格：课程生成主链路对齐

> 依据：`docs/parity-gap-plan.md` 阶段 A；对照基准 `D:\AILINES-CAMPUS`。
> 本文件给出可直接编码的实现规格（端点契约、数据形状、缓存设计、搜索客户端、C++ 模块划分），实施时无需再读校园版源码。
> 覆盖范围：计划缓存、课程/快照持久化、/plan 双视图、联网搜索、富失败态。

---

## A1. 计划缓存（对照 `lib/ai/planCache.ts`）

**目录**：`data/ai-plan-cache/`（C++ 运行时相对工作目录，可用 `AI_PLAN_CACHE_DIR` 覆盖）

**Key**：`sha256("{mode}:{normalizeGoal(goal)}") + ".json"`
- `normalizeGoal`：trim → 连续空白折叠为单空格 → 转小写
- `mode`：归一化为 `lite` | `deep`（默认 deep）

**文件内容**：
```json
{ "goal": "<原输入 trim 后>", "mode": "lite|deep", "createdAt": "<ISO8601>", "plan": { ...GeneratedPlan } }
```

**TTL**：`AI_PLAN_CACHE_TTL_SECONDS` 默认 604800（7 天）；`createdAt` 过期即视为 miss

**命中校验 `isValidGeneratedPlan`（全过才算 hit）**：
- `title`/`goal`/`durationWeeks`(number)/`summary` 为 string
- `phases` 非空数组，每项：`name`/`objective`/`description` string、`durationWeeks` number、`topics` 为 string 数组、`steps`（可选）每项含 string 的 title/explanation/action/check
- `resources` 数组每项：name/type/difficulty/description/url string + free boolean
- `projects` **非空**数组，每项：name/difficulty/output string + estimatedHours number + acceptanceCriteria string 数组

**行为**：
- `generate-plan` 流程：命中 → 直接返回（标记来源 `legacy-ai`，响应可带 `cache:'hit'`）；miss → 生成成功后写缓存（写失败仅 warn 不阻断）
- `bypassCache=true` / `forcePlan=1` / `retry` 任一存在 → 跳过读缓存（但仍写缓存）

## A2. 课程/快照持久化（对照 `app/api/courses/route.ts` + `lib/course/courseRepository.ts`）

### 端点：`POST /api/courses`
**请求体**：
```json
{ "anonymousId": "anon_xxx", "goal": "…", "mode": "lite|deep", "title": "…", "summary": "…", "source": "ai", "payload": { ...MockPlan } }
```
**校验与响应（顺序即优先级）**：
1. `goal`/`title`/`payload` 缺失 → `400 {ok:false, error:"COURSE_SAVE_INVALID_INPUT", message:"课程信息不完整。", canRetry:true}`
2. payload 质量门禁不过 → `422 {ok:false, error:"COURSE_SNAPSHOT_INVALID", message:"课程内容暂未生成完成，请重新生成后再保存。", canRetry:true}`
3. 保存超时 12s → `504 COURSE_SAVE_TIMEOUT`；其他 → `500 COURSE_SAVE_FAILED`
4. 成功 → `200 {ok:true, courseId, href:"/plan?courseId=<id>&anonymousId=<anon>"}`（仅无登录用户时带 anonymousId）

**payload 入库前处理（`sanitizeCoursePayload` + `stripUnsafeKeys`）**：
- 递归深度 ≤8；剔除键名含 apikey/api_key/secret/token/authorization/base64/image/rawerror/stack 的字段；字符串截 10,000
- 白名单重排：`title,duration,summary,courseIntro,overview,audience,prerequisites,outcome,learningOutcomes,slides,mindMap,roadmap,courseStructure,resources,projects`
- resources 每项截断：name≤300 / description≤500 / href≤1000，最多 12 条，过滤无 name 或 href 者
- 总 JSON >900,000 时裁剪：slides≤20 / roadmap≤12 / resources≤8 / projects≤12

**upsert 逻辑（`createOrUpdateCourseSnapshot`）**：
- 以 `(userId 或 anonymousId) + goal + status='active'` 找现有 Course：命中 → 更新 title/summary/source/mode/updatedAt 并新建一条 CourseSnapshot（version = 旧 max+1）；未命中 → 新建 Course + 第一条快照（version=1）
- 响应中的 `courseId` 即 Course.id

### 端点：`GET /api/courses/[courseId]`
- 归属校验：courseId 对应 Course 的 `userId` 或 `anonymousId` 与请求身份匹配，否则 404
- 成功 → `{ course: {...Course}, snapshot: { payload } }`（最新快照）

### 端点：`GET /api/courses`（历史课堂下拉用）
- `?anonymousId&limit(默认50, clamp 1-100)&offset`；返回课程列表（id/goal/mode/title/summary/createdAt/updatedAt/href/progress 摘要）；失败降级 `200 {courses:[]}`

### 前端保存流（`CourseHistoryRecorder` 等效，原生 JS）
- `/plan` 生成成功渲染后，客户端自动 `POST /api/courses`（12s AbortController 超时）
- 成功：`history.replaceState` 把 URL 替换为 `/plan?courseId=…&anonymousId=…` 并刷新页面数据；localStorage 同步存一条课程历史；派发自定义事件刷新顶栏历史课堂
- 失败：显示 amber 提示条「课程快照保存失败。当前页面内容仍可查看…」+ localStorage 兜底，不阻断浏览

## A3. /plan 双视图（对照 `CoursePlanView` / `LitePlanView` + `adaptGeneratedPlan`）

**关键前提**：C++ `/api/generate-plan` 当前返回原始 GeneratedPlan 形状（phases），校园版页面消费的是 **adaptGeneratedPlan 后的 MockPlan 形状**。C++ 需实现等价适配器（或直接让页面按 MockPlan 形状渲染）。MockPlan 形状：
```json
{
  "title": "…", "duration": "6 周", "summary": "…", "courseIntro": "…", "overview": "…",
  "audience": "…", "prerequisites": ["…"], "outcome": "…", "learningOutcomes": ["…"],
  "slides": [{ "title": "…", "subtitle": "…", "content": "…", "bullets": ["…"], "speakerNote": "…", "relatedPhase": "…" }],
  "mindMap": { "title": "课程知识结构", "nodes": [{ "id": "root", "label": "…", "children": [{ "id": "…", "label": "…", "children": [...] }] }] },
  "roadmap": [{ "name": "…", "duration": "…", "goal": "…", "description": "…", "why": "…", "output": "…", "practice": "…", "checkpoint": "…", "commonMistakes": ["…"], "tasks": ["…"], "steps": [{ "title": "…", "explanation": "…", "example": "…", "action": "…", "check": "…" }] }],
  "courseStructure": [{ "stage": "…", "topics": ["…"] }],
  "resources": [{ "name": "…", "type": "…", "difficulty": "…", "free": true, "description": "…", "href": "…" }],
  "projects": [{ "name": "…", "difficulty": "…", "duration": "4 小时", "output": "…", "acceptance": "…" }]
}
```
**适配要点**（`adaptGeneratedPlan` 移植）：
- roadmap.steps 为空时用 `skeletonStepsFromPhase`（title=topics[i]，action=`进入微课程学习「topic」`，check=checkpoint，explanation=topicDescriptions[i] 或兜底文案）
- slides 为空时从 phases 推导（首张总览卡 + 每阶段 1 卡 + 每阶段前 2 步各 1 卡，截 12 张）；lite 只取前 1、deep 前 2
- mindMap 缺失时从 phases 推导（root=goal，child=阶段名，孙=steps.title 或 topics，每阶段≤5）
- `isRenderablePlan`：title+summary+roadmap 非空+courseStructure 非空，否则视为生成失败走失败态
- lite 的 durationWeeks clamp 1-2；title 兜底 `{goal}快速学习方案` / `钢一定制AI 学习方案`

**deep 视图区块顺序**（CoursePlanView）：计划头(标题+summary+目标/周期信息卡) → 下一步 CTA 横幅（有进度显「继续学习/上次学到」、无进度显「开始学习」）→ 课程进度条（% + 已完成 x/y）→ 课件卡（横向页签 + 知识点卡展开 + 标记掌握[仅会话内]）→ 思维导图（CSS 递归树）→ 学习路线阶段卡（序号/时长/状态徽标/目标/产出/检查点 + 查看阶段/开始本阶段/进入第一个知识点）→ 课程结构（阶段分卡 + 知识点状态 + 复习/继续/开始按钮）→ 资源卡（type/难度/免费徽标 + 查看资源外链）→ 项目实战卡 → 底部操作（保存路线/问钢一定制AI/开始执行）
**lite 视图区块**（LitePlanView）：绿色系 hero（快速规划徽标 + 模式/周期/「先做会，再深入」三格）→ 直接开始第一节 CTA → 课程结构 → 核心步骤（roadmap 前 5 步 怎么做/怎么验收）→ 必备材料/工具 + 常见错误双栏 → 练习清单 4 格 → 下一步建议 → 可参考资料（5 条）

## A4. 联网搜索资源（对照 `lib/search/`）

- **Provider**：Tavily（主，`TAVILY_API_KEY`）+ Bocha（备，`BOCHA_API_KEY`），`SEARCH_PROVIDER`/`SEARCH_FALLBACK_PROVIDER` 可切换
- **Tavily**：`POST https://api.tavily.com/search`，body `{query, search_depth:"basic", max_results:5, include_answer:false, include_raw_content:false}`，Bearer key
- **Bocha**：`POST {base}/v1/web-search`，body `{query, count:5, summary:true, freshness:"noLimit"}`，15s 超时
- **查询构造**：`detectLearningDomain(goal)`（13 领域含每领域 4-5 条查询，如数学→Khan Academy/3Blue1Brown/B站；编程→官方文档/GitHub）；无领域命中用通用查询
- **归一化**：来源按 hostname 推断（Khan/3B1B/B站/YouTube/GitHub/MDN 等）；类型 9 类（开源项目/官方文档/视频教程/在线课程/练习题库/项目实战/工具环境/社区资源/图文教程）；难度/语言（中文≥4 汉字）/免费（Udemy/Coursera 无 free 字样判付费）；评分（≤1 转百分比，缺省 70）；按领域生成推荐理由
- **去重排序**：url 去斜杠小写去重，score 降序，截 20 条
- **缓存**：文件缓存 `data/resource-search-cache/`，key=sha256(`{primary}:{fallback}|{domain}:{goal}`)，TTL 7 天，命中返回 `cache:'hit'`
- **消费**：/plan 深/浅模式生成后 `searchResources(goal)` 8s 超时竞速；命中取前 8 条注入 plan.resources（转换为 MockPlan 资源形状），提示条切换为「已为你补充全网真实学习资源」；失败静默降级为「以下为钢一定制AI推荐资源」

## A5. 富失败态（对照 `CourseGenerationPendingState`）

错误类型 → 文案映射：
| 类型 | 文案 |
|---|---|
| timeout | 生成超时，请重试。 |
| auth_error | 当前模型接口认证失败，请检查服务配置后重试。 |
| rate_limited | AI 服务请求过于频繁，请稍后重试。 |
| invalid_response / json_parse_error / quality_rejected | 生成内容未通过质量检查，请重新生成。 |
| missing_config | 当前模型接口尚未配置，请检查服务配置。 |
| 其他 | AI 服务暂时不可用，请稍后重试。 |

状态页三按钮：**重试生成**（带 `forcePlan=1&bypassCache=true&retry=<ts>`）/ **先生成快速规划**（仅 deep 失败时显示，切 lite 重试）/ **返回首页**。

## A6. C++ 模块划分建议

| 模块 | 职责 | 复用/新增 |
|---|---|---|
| `src/plan_cache.{hpp,cpp}` | 缓存读写 + sha256（需新增 sha256 实现或链接 OpenSSL）| 新增 |
| `src/plan_adapter.{hpp,cpp}` | GeneratedPlan → MockPlan 形状（adaptGeneratedPlan 移植）| 新增 |
| `src/search_client.{hpp,cpp}` | Tavily/Bocha HTTP 客户端 + 归一化 + 缓存 | 新增（复用 ai_client 的 curl 封装模式）|
| `src/quality_gate.{hpp,cpp}` | 保存前质量门禁（courseContentQuality 移植）| 新增 |
| `src/course_service.{hpp,cpp}` | Course/CourseSnapshot 的 repository 级 upsert/查询/脱敏 | 新增（db.cpp 之上）|
| `src/main.cpp` | 新增路由：POST /api/courses、GET /api/courses、GET /api/courses/[id]；/plan 页面改为按 MockPlan 渲染 | 修改 |
| `public/` | /plan 页前端：双视图渲染 + 自动保存 + 失败态 | 修改 |

**依赖新增**：sha256（OpenSSL 或自实现）；Tavily/Bocha 仅需 curl（已有）。

## A7. 验收标准（对应 parity-gap-plan 阶段 A）

1. 同一 goal+mode 二次生成命中缓存（`cache:'hit'`，≤秒级返回），`bypassCache=true` 绕过
2. /plan 生成成功 → 自动保存 → URL 变为 `?courseId=…` → 刷新恢复同内容；`GET /api/courses/[id]` 返回快照
3. deep/lite 双视图区块完整，与校园版逐屏一致（含 slides/mindMap/roadmap/courseStructure/resources/projects）
4. 有 TA 配置时资源卡为真实搜索资源（≤8 条）；无 TA 配置时静默降级不报错
5. 失败态文案按类型正确、三按钮可用；422/504 保存失败不阻断页面浏览

# 钢一定制AI · AI 行为规格（对应校园版 lib/ai/）

> 依据：`D:\AILINES-CAMPUS\lib\ai\`（12 个 TS 文件，约 1780 行）
> 用途：C++ AI 生成逻辑（阶段2b）的验收标准

## 1. 通用约定

- **API 端点**：`POST {AI_BASE_URL}/chat/completions`（默认 `https://api.deepseek.com`，模型 `deepseek-chat`）
- **超时**：AI_TIMEOUT_MS 默认 35000；计划生成 lite 75s / deep 80s（attempt），总超时 lite 200s / deep 220s
- **重试**：默认 2 次，退避 800ms 指数增长
- **熔断**：连续 3 次失败 → 熔断 60s
- **responseFormat**：json_object（DeepSeek 支持）
- **错误类型**：missing_config / auth_error / rate_limited / timeout / provider_5xx / network_error / invalid_response / json_parse_error / quality_rejected / unknown
- **错误 → HTTP**：missing_config→503，auth_error→503，timeout→504，其余→502

## 2. 课程生成（generatePlan.ts + generatePlanPrompt.ts）

### 输入
- `goal`（学习目标，trim 后非空校验）
- `mode`: 'lite' | 'deep'
- `bypassCache`: 是否跳过缓存

### 流程
1. 校验 goal 非空 → 400 invalid_request
2. 读缓存（goal+mode 哈希 key）→ 命中返回（标记 legacy-ai）
3. 构建提示词（createGeneratePlanMessages）→ AI 请求（maxTokens: lite 4800 / deep 7600）
4. parseAIJson 解析 → 适配 GeneratedPlan（adaptGeneratedPlan）
5. 质量门禁（validateCourseContent）→ 失败带修复后缀重试一次
6. 写缓存 → 返回

### 输出 JSON schema（GeneratedPlan）
```json
{
  "title": "string",
  "summary": "string",
  "courseIntro": "string",
  "overview": "string",
  "outcome": "string",
  "learningOutcomes": ["string"],
  "roadmap": [{
    "name": "string", "duration": "string", "goal": "string",
    "description": "string", "why": "string", "output": "string",
    "practice": "string", "checkpoint": "string",
    "commonMistakes": ["string"],
    "tasks": ["string"],
    "steps": [{"title": "string", "explanation": "string", "example": "string", "action": "string", "check": "string"}]
  }],
  "courseStructure": [{"stage": "string", "topics": ["string"]}],
  "slides": [{"title": "string", "subtitle": "string", "content": "string", "bullets": ["string"], "speakerNote": "string", "relatedPhase": "string"}]
}
```

### 提示词要点
- system：你是 AILINES AI 学习规划助手，必须输出严格 JSON
- user：包含 goal、mode（lite/deep）、输出 schema 要求、避免泛化短语
- JSON 解析失败时附加 `createJsonParseRetrySuffix` 重试

## 3. 微课程生成（generateLearningAnswer.ts）

### 输入
- goal, phaseName, topic, mode, resources（SearchResource[]，最多 8 条）

### 输出 JSON schema
```json
{
  "inferredDomain": "string",
  "title": "string",
  "summary": "string",
  "keyConcepts": ["string"],
  "lessonSteps": [{"title": "string", "explanation": "string", "example": "string", "action": "string", "check": "string"}],
  "examples": [{"title": "string", "content": "string", "solution": ["string"]}],
  "practice": [{"title": "string", "difficulty": "string", "task": "string", "check": "string"}],
  "quiz": [{"question": "string", "options": ["A","B","C","D"], "answerIndex": 0, "explanation": "string"}],
  "commonMistakes": ["string"],
  "checkpoint": ["string"],
  "resourceSummary": "string",
  "references": [{"title": "string", "source": "string", "url": "string", "type": "string"}]
}
```

### 约束
- lessonSteps 数量：lite 3-4 / deep 5-6；每步 explanation 长度 lite 90-140 / deep 160-260
- practice 数量：lite 2-3 / deep 3-5
- quiz 3-5 题，每题 4 个非空选项，answerIndex 唯一
- references **只能引用输入 resources 的 url**（filterReferences 校验）
- 质量门禁失败 → 带修复后缀重试一次 → 仍失败返回兜底（notice 标记，不扣额度）

## 4. 问答生成（generateAskAnswer.ts + askPrompt.ts）

- 输入：goal, question, mode
- maxTokens：lite 1500 / deep 2000（已上调）
- 输出：JSON {answer: string}
- 失败返回兜底答案

## 5. 阶段展开（generatePhaseExpansion.ts）

- 输入：goal, mode, phaseIndex, stage, topics, resources
- maxTokens：lite 3600 / deep 5200（已上调）
- 输出：{objective, overview, steps[], tasks[], checklist[], commonMistakes[]}
- 质量门禁失败 → 返回 null

## 6. 缓存（planCache.ts）

- key：sha256(goal 归一化 + mode)
- TTL：AI_PLAN_CACHE_TTL_SECONDS 默认 604800（7 天）
- 文件缓存：data/ai-plan-cache/

## 7. 质量门禁（courseContentQuality.ts 移植要点）

- 泛化短语黑名单：基础课程/关键抓手/不要只背名词/掌握基本概念/提升综合能力 等
- 结构校验：phases 数量（lite 3-5 / deep 4-6）、lessonSteps/practice/checkpoint 非空
- 关键词相关性：goal 关键词在内容中命中数
- 重复内容检测：唯一文本比例
- 动作/产出检测：特定动词/产出词正则
- **references 字段豁免**泛化判定（避免真实标题误伤——已修复）
- 评分：100 - 泛化命中×25 - 关键词缺失×15 - ...，≥55 且无 fatal 才通过

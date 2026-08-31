# AILINES 校园版数据行为规格

> 依据仓库：`D:\AILINES-CAMPUS`。
> 目标：为 C++ 版 `D:\钢一定制AI` 数据层提供验收规格。
> 范围：只描述校园版真实代码中的数据库访问行为，不扩展产品设计。

## 0. Prisma Client 与通用约定

### Prisma Client 封装

- `lib/db/prisma.ts` 只导出单例 `prisma`。
- 非生产环境会把实例挂到 `globalThis.prisma`，避免 Next.js 热重载重复创建 client；生产环境不缓存到 global。
- Prisma 日志：`NODE_ENV === 'development'` 时记录 `error,warn`，否则只记录 `error`。
- 证据：`D:/AILINES-CAMPUS/lib/db/prisma.ts:1-13`。

### Schema 与数据库

- `prisma/schema.prisma` 使用 SQLite：`provider = "sqlite"`，连接串来自 `DATABASE_URL`。
- 所有主要表的 `id` 字段由 Prisma `@default(cuid())` 生成，除非代码另行使用本地 localStorage 伪 ID。
- DateTime 字段由 Prisma `now()` / `@updatedAt` 或代码 `new Date()` 写入。
- API 返回给前端时，Date 通常转成 ISO 8601 字符串：`toISOString()`。
- 证据：`D:/AILINES-CAMPUS/prisma/schema.prisma:1-8`、`D:/AILINES-CAMPUS/prisma/schema.prisma:10-220`、`D:/AILINES-CAMPUS/app/api/courses/route.ts:87-99`、`D:/AILINES-CAMPUS/app/api/course-progress/route.ts:16-29`。

### 字符串标准化

- 多数写入入口会 `trim()` 字符串，空字符串转成 `undefined` / `null` 语义。
- 课程、学习内容、进度标题等会裁剪长度：课程 JSON 文本字段最大 10,000 字符，课程 JSON 最大 900,000 字符；学习会话 JSON 文本字段最大 10,000 字符，JSON 最大 900,000 字符，标题最大 500 字符。
- 证据：`D:/AILINES-CAMPUS/lib/course/courseRepository.ts:10-12`、`D:/AILINES-CAMPUS/lib/course/courseRepository.ts:41-44`、`D:/AILINES-CAMPUS/lib/course/courseRepository.ts:46-76`、`D:/AILINES-CAMPUS/lib/course/learningSessionRepository.ts:7-9`、`D:/AILINES-CAMPUS/lib/course/learningSessionRepository.ts:39-47`、`D:/AILINES-CAMPUS/lib/course/learningSessionRepository.ts:60-117`。

## 1. Course

### Schema

- 字段：`id, anonymousId, userId, goal, mode, title, summary, source, status, createdAt, updatedAt`。
- `source` 默认 `
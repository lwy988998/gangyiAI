# 深度课程协议（deep-v1）

本协议记录 v5.7.0 已实现的课程识别、课时生成、缓存、展示与媒体行为。实现位于 `src/deep_course.cpp`、`src/agent_curriculum.cpp`、`src/agent_lessons.cpp`、`src/learning_agent.cpp`、`src/main.cpp` 与 `public/deep-course.js`，继续使用 C++ 主控、SQLite 和原生前端。

产品支持任意领域的学习目标，由 AI 按领域任务组织课程大纲；互动图的受控类型有明确边界，不把“任意领域”解释为任何图形、代码或学科内容都已实现或完成验收。本版不提供 PPT、PDF 或图片导出，也不新增付费图像生成服务。

## 课程模式与旧课兼容

| 项目 | 规则 |
| --- | --- |
| 首页选择 | 快速规划发送 `mode=lite`；深度课程发送 `mode=deep`。 |
| 新课程建立 | `plan_course` 事件的明确选择决定模式，不能被模型的 `create_course.args.mode` 覆盖。 |
| 新深度标记 | 只有新建规划事件明确为 `mode=deep`，程序才在课程快照的 `payload` 保存 `pedagogy: "deep-v1"`。 |
| 课堂识别 | 读取版本号最大的课程快照，按其 `payload.pedagogy` 判断；课程列表接口也输出对应 `pedagogy`。 |
| 旧课程 | 原有 `course.mode=deep` 不能证明是新深度课；没有版本标记的课程继续快速规划。 |
| 大纲调整 | 保留原课程的协议标记，稳定知识点标识与原索引映射继续有效；不因重排把旧课转换成新课。 |

旧链接与现有课程继续走兼容入口；新深度课访问相应旧学习入口时由服务选择新课堂。无需批量迁移或重新生成旧课程。

## 生成与复用流程

1. `POST /api/generate-plan` 提交目标、模式和请求标识，转入原 `plan_course` 主控任务。
2. AI 通过 `create_course` 保存阶段、知识点、目标与主线。此阶段不一次生成整门讲义，也不设置固定课件页数。
3. 深度页面读取课程与已保存课时。定位优先使用稳定 `topicId`，兼容从 1 开始的 `phaseIndex`／`topicIndex`；`lessonId` 可直接定位课时。
4. 对尚未备课的知识点调用 `POST /api/deep-course/start`，提交同一主控的 `prepare_next`。程序按课程与稳定知识点构造固定备课请求身份，不同窗口重复点击复用同一任务。
5. AI 建立课时，先保存真实公开讲解和题目，再通过 `set_deep_content` 保存讲义、按需课件与媒体。`finish_lesson` 要求深度课已有非空讲义，失败媒体可以保留为可重试状态。
6. 已完成课时再次开始时返回 `cached=true` 和同一课时入口，并记录进入课时；不会重新备课。重复读取、切换视图、翻页和拖动本地图示不发起模型生成。

课程主线保持稳定，AI 依据真实作答调整讲解、练习和后续建议。原有主控的版本检查、任务归属、暂停、恢复、停止及失败重试继续生效。

## 页面与接口

页面入口为 `/deep-course`，支持 `lessonId`，或 `courseId` 加 `topicId`／阶段知识点索引。初次读取没有可用课时时展示准备入口；已有内容先开放阅读。

| 方法与路径 | 参数与返回 |
| --- | --- |
| `GET /api/deep-course` | 查询参数为 `lessonId`，或 `courseId` 配合 `topicId`／`phaseIndex`／`topicIndex`。返回 `enabled`、`status`、`course`、`outline`、公开 `lesson` 与 `content`。只读查询不触发 AI。 |
| `POST /api/deep-course/start` | JSON 定位参数同上，可传 `requestId`；返回备课 `task`，或已有 `lessonId`、`href`、`cached=true`。 |
| `POST /api/deep-course/retry-media` | JSON 含 `lessonId`、`mediaId`、`requestId`；图片缓存修复额外传 `repair=true`。返回主控 `task`；已就绪且无需修复时返回 `cached=true`。 |
| `GET /api/deep-course/image` | `lessonId` 与 `mediaId` 定位已保存图片，返回实际图片二进制与 MIME。支持内容版本查询参数用于刷新显示缓存。 |
| `POST /api/learning-agent/events` | 作答、答疑和教学请求使用原统一事件入口。 |
| `/api/learning-agent`、`/api/learning-agent/control`、`/ws/learning-agent` | 沿用原任务状态读取、控制和实时流。 |

深度 JSON 接口在请求无效、对象不匹配或访问范围不符时返回 `409` 与 `error`；图片不存在、未就绪或缓存无效时返回 `404`。按课程查询没有深度标记的旧课程时返回 `enabled=false,status=legacy`；直接读取不属于新协议的旧课时会被拒绝。访问范围继续要求当前本机档案与有效课程。

主区切换“讲义”与“课件”，左目录按课程主线选课，右侧答疑可收起。课件翻页栏独立于内容滚动区，长页仍保持翻页入口可见。窄屏目录和答疑以独立对话框展开，主阅读区和底部行动保持可用。当前视图、页码、阅读位置、草稿与互动参数由前端在对应课时范围恢复。

## 内容模型与来源绑定

`deepContent` 保存于现有 `agent-lesson` 课时 payload，包含：

```json
{
  "version": 1,
  "lecture": [],
  "slides": [],
  "media": []
}
```

保存后还记录生成模型和时间，内容更新递增 `version`。讲义与课件各最多 80 个板块，媒体最多 32 项；引用标识、类型、参数范围和关联对象均由后端校验。

### 讲义与课件

- 讲义条目必须通过 `sectionId` 引用**本课已保存的 `explanation` 板块**，标题和正文由程序读取原板块；模型不能另塞一段文字冒充已保存讲解。
- 课件条目包含稳定 `id`、标题、`sourceSectionIds` 和可选 `mediaIds`。来源限本课公开讲解或小结，正文由来源板块组合；不能引用其它课时、未保存板块或题目私有标准。
- `mediaIds` 必须对应本份内容里已存在的媒体标识。讲义不能为空；课件及媒体按教学需要组织，不要求每课固定数量。
- 题目仍由原 `append_section` 保存。公开课时只返回题干、选项和材料等学生所需内容，过滤未公开答案、评分标准及内部参数。

以下为工具参数结构示意，标识需替换为本课 `append_section` 实际返回的值：

```json
{
  "lessonId": "本课标识",
  "content": {
    "lecture": [
      {"sectionId": "本课讲解板块标识", "mediaIds": ["curve-a"]}
    ],
    "slides": [
      {
        "id": "slide-a",
        "title": "观察系数变化",
        "sourceSectionIds": ["本课讲解板块标识"],
        "mediaIds": ["curve-a"]
      }
    ],
    "media": [
      {
        "id": "curve-a",
        "type": "function",
        "title": "二次函数参数演示",
        "caption": "拖动参数观察函数图像。",
        "coefficients": [1, 0, 0],
        "xMin": -5,
        "xMax": 5,
        "parameter": {
          "name": "a",
          "min": -3,
          "max": 3,
          "step": 0.1,
          "value": 1,
          "coefficientIndex": 0
        }
      }
    ]
  }
}
```

### 按需媒体

| `type` | 已实现的数据与行为 |
| --- | --- |
| `timeline` | `events:[{label,text}]`，显示时间列、竖线与节点。 |
| `flowchart` | `nodes:[{id,label}]` 与 `edges:[{from,to,label}]`；节点标识唯一，连线必须指向已有节点。 |
| `function` | `coefficients:[a,b,c]` 对应 `a*x²+b*x+c`，`xMin`／`xMax` 指定横轴；可选 `parameter` 调节一个系数，`coefficientIndex` 为 0、1 或 2。 |
| `forces` | `mass`、`force`、`friction`、可选 `gravity`；本机调整受力参数并显示读数。摩擦参数为最大可抵消阻力，属于静止起步的简化模型，不模拟速度或位移。 |
| `image` | 只接受 `search_lesson_images` 已检索并缓存的同一媒体标识，不信任模型自造地址。公开视图输出本机图片接口 URL 和来源说明。 |

媒体通用字段包括 `id`、`type`、`title`、`caption` 和 `status`。有效媒体为 `ready`；未完成可以为 `pending` 或 `failed`，附简短 `error`。前端使用固定图形组件绘制结构化数据，不执行模型返回的任意 HTML 或 JavaScript。

普通图文讲义覆盖不适用现有互动类型的领域。学生可以要求补图或调整，但工具类型不会因此自动扩展为通用动画、仿真或任意代码运行环境。

## 联网图片与局部重试

`search_lesson_images` 参数为 `lessonId`、`query`、`title`、稳定 `mediaId`，可选 `refresh=true`。工具复用现有双搜索、来源读取和网络访问边界，提取网页图片候选，按查询、标题、替代文字及图注进行关联排序，并过滤明显 logo、头像等装饰项；不接受模型猜测地址。来源关联与实际图像适配性分别核对，文本匹配不能单独证明图片内容正确。

实际图片通过 PNG、JPEG、WebP 或 GIF 文件头识别，单图限制为 3 MiB。图片二进制与来源保存到现有记录存储，公开课时不返回内部 `blobId`；显示时走本机图片接口。图片二进制不作为普通教学上下文发送给模型，删除课程时清理所属图片记录，保留其它课程缓存。

图片抓取失败只保存该项失败状态，讲义与其它已成功媒体继续使用。请求 `retry-media` 后：

1. 校验素材确属当前课时；已就绪且没有修复要求的项目直接复用。
2. 创建原主控的辅助 `chat` 任务，携带 `deepMediaRetry`；图片修复额外携带 `deepImageRepair`。
3. `set_deep_content` 在重试任务中只替换指定媒体，保留已保存讲义、课件和其它媒体；图片工具也只允许操作当前指定媒体标识。
4. 多项失败通过前端逐项等待处理。暂停、失败或页面离开后停止继续后续重试，保留每项已成功结果。

若媒体元数据为 `ready`，但实际图片缓存无法加载，前端提供图片修复入口并发送 `repair=true`，重新获取该项实际图片。局部重试属于教学辅助，不作为学生作答评分。

## 答疑上下文与真实评价

答疑和题卡提交沿用原主控事件，深度课堂补充：

- `deepView`：当前为 `lecture` 或 `slides`。
- `contentId`：当前可见讲义板块或课件页标识。
- `mediaState`：相关本机互动的有限数值状态。

服务从已保存内容重建当前教学页面，只取受支持媒体的有效参数。模型可据此回答“这一页”或“这个参数”的问题，不能借由客户端字段覆盖保存内容或私有评分资料。

题卡使用 `question_answer`、实际输入、`sectionId` 和 `sectionVersion` 关联当前问题，先评价实际作答，再发布相应讲解。`evaluate_answer`、`classify_input`、提示曝光、暂存评价、正式评价、同题去重和画像证据规则均沿用原实现。真实评分不由课件页码、阅读时长、滑块拖动或打开答疑推断。

纯追问、请求示范、素材重试不伪造分数。学生明确不会或实际回答按原判断路径处理；评分不完整、讲解失败或输出截断时保留原失败与恢复机制。是否完成当前课、如何补弱及下一步安排由已有真实作答和评价决定。

## 验证边界

已完成的新增 C++ 专项使用内存数据库和注入模型回复，验证模式、结构、内容绑定、缓存与公开边界；浏览器夹具验证讲义、课件、互动、草稿、答疑上下文、作答事件、目录和局部重试。它们不代表真实模型的领域知识和教学质量。

真实 C++ 服务与模拟模型的端到端脚本 `tests/deep_course_integration_smoke.py` 已通过，覆盖大纲、逐课准备、课件来源、实际作答评价、当前页与互动参数上下文、缓存进入及三视口布局；“继续学习”和长课件翻页栏也通过模拟回归。该结果属于模拟模型的接口与持久化验证，真实模型教学结论限定于下文已核对的数学与历史案例。

真实数学、历史时间轴、图片工具链、冻结回归与旧档案副本的限定证据见 [v5.7.0 实际范围](releases/v5.7.0.md#实际范围)。公开版本以标签流水线和公开附件为准，私人验收结果在本机留存；没有验收过的领域、媒体案例和长期效果不据此推定通过。

相关既有语义见 [AI 教学主控](ai-controller.md) 与 [原课堂与页面协议](v5.6.0-ai-classroom.md)。

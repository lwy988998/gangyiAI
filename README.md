# 钢一定制AI

钢一定制AI 是面向柳州市钢一中学的 Windows 本机 AI 学习平台，当前版本为 **v5.6.7**。本版修复教学作答、课时任务、备份恢复、调用复用及本机接口问题，并明确数学术语与等价解评分边界；保留原模型设置与教学算法。使用 C++17、Crow、SQLite、WebView2 和原生 HTML/CSS/JavaScript，课程与学习记录保存在本机，无需在线账号。

需要联网时两家同时搜索，合并去重并保留来源；分别显示请求、缓存和失败状态。已有资料、原文快照和课堂引用继续保留。详见 [v5.6.7 版本说明](docs/releases/v5.6.7.md)。本轮只完成短案例对照与等价解专项，不代表全部学科、六教学案例及三类核心流程已完成真实验收，也不外推整体 Token 或耗时改善。

## 学习与 AI

- 首页输入学习目标或上传参考图，选择快速规划或深度课程；我的课程管理课程，“我的AI”管理统一学习安排、画像、教学偏好和学习时间。
- 当前课程恢复最近访问的有效课程；没有课程时显示“当前没有课程”，提供创建入口。
- 每个知识点有讲解、练习、小结三个独立聊天页面，固定底部输入框。各页保存自己的交流、草稿和滚动位置，AI 共享完整学习背景。
- 真实 AI 主控决定教学内容、题量、难度、评价和下一课。实际作答先评价再讲解，纯追问与示范不新增评分；切换、刷新和读取已有内容不重新备课。
- 学科画像与学习能力六维图依据真实作答评估。充分性由 AI 判断，无固定三题门槛；证据不足显示待评估，六维都有有效评分后显示完整区域。
- 全软件 AI 悬浮监控显示正在调用的任务、来源、用途、具体操作和请求次数，支持暂停全部请求、恢复未完成任务及失败重试。

详细行为见 [AI 教学主控](docs/ai-controller.md)、[课堂与页面协议](docs/v5.6.0-ai-classroom.md)、[AI 调用监控](docs/AI调用监控.md) 和 [版本说明](docs/releases/v5.6.7.md)。

## Windows 使用

在 [GitHub Releases](https://github.com/lwy988998/gangyiAI/releases/latest) 下载 `gangyiAI-learning-preview1-v5.6.7-x64.exe`，或完整解压 `gangyiAI-learning-preview1-v5.6.7-x64.zip` 后运行 `gangyiAI-launcher.exe`。版本标签的 Windows/Linux 检查通过并校验附件后，由流水线正式发布。

已安装“学习增强试用版”时，使用同页 `gangyiAI-learning-preview1-v5.6.7-x64.exe` 原位升级，继续使用原试用档案；本次发布沿用既有安装和档案标识。升级前备份程序、学习档案和设置，并保留回退副本。正式版与试用版使用独立档案，安装其中一种不会自动合并另一种的数据。

独立窗口恢复上次大小与位置，每次冷启动播放一次光栅扫描动画。首次启动可暂不配置 AI，后续在设置中填写服务商、接口地址、模型和 API Key。密钥保存在 Windows 凭据管理器，配置与课程数据位于 `%LOCALAPPDATA%/GangyiAI`；卸载默认保留课程数据。AI 不可用时保留有效内容并提供重试。

托盘提供日志、数据目录、诊断报告、数据库备份恢复及服务重启。服务异常后按限次退避策略恢复。免安装包不携带本机档案；程序需要 WebView2 Runtime，目前未进行代码签名。

下载页提供 `SHA256SUMS.txt`，校验命令：

```powershell
Get-FileHash ./gangyiAI-learning-preview1-v5.6.7-x64.exe -Algorithm SHA256
```

## 构建与验收

Windows 发布环境需要 CMake、MinGW-w64、PowerShell 和 Inno Setup 6：

```powershell
powershell -ExecutionPolicy Bypass -File ./scripts/build_windows.ps1
```

脚本下载固定版本依赖，编译、运行核心测试、自检并生成 `dist/installer` 中的安装包、免安装包及 SHA256 文件。仅编译和生成免安装包可加 `-SkipInstaller`。

源码保留核心回归和当前课堂、雷达、启动及页面就绪浏览器验收；共用夹具采用隔离档案和模拟服务。GitHub Actions 执行 Windows/Linux 编译、测试、Sanitizer 和隔离安装卸载验收，版本标签触发正式发布。人工检查见 [Windows 发布检查](docs/windows-release-checklist.md)。

本机 `verification`、构建缓存、测试数据和预览不进入 Git 或发布包。`third_party` 为构建依赖，不能当作演示目录删除。

## 服务端运行

Linux 或直接运行服务时设置以下环境变量，默认只监听本机：

```text
HOST=127.0.0.1
PORT=39002
DATABASE_PATH=gangyiAI.db
AI_BASE_URL=https://api.deepseek.com/v1
AI_API_KEY=你的密钥
AI_MODEL=deepseek-chat
```

运行服务程序 `gangyiAI`。课程、画像及记录保存在指定数据库；模型请求发送至你配置的服务，页面只读查询与显示操作不属于模型推理请求。

## 我的AI电脑版

首页导航的“我的AI”提供个人概览、AI管家、学习报告、学习计划、教学偏好、AI记忆、AI任务、使用情况、个人资料、软件设置和数据管理，均使用独立页面。

正式构建升级已有安装，保留原学习档案与密钥，数据库迁移前自动备份。需要隔离体验时可执行 `scripts/build_windows.ps1 -Preview`，使用独立安装目录、档案、注册表、凭据与启动互斥锁。

行为、接口和统计口径见 [我的AI说明](docs/MY_AI.md)。

# gangyiAI

gangyiAI（钢一定制AI）是面向柳州市钢一中学的本机 AI 学习平台，使用 C++17、Crow、SQLite 和原生 HTML/CSS/JavaScript 构建。它支持学习目标输入与图片识别、快速或深度课程规划、阶段学习、微课程、测验、AI 对话和课程管理。深色动态界面展示依据本机学习记录生成的专属学习画像；课程和对话记录保存在本机，不需要在线账号。

当前仓库源码版本为 **v0.3.4**；GitHub Releases 目前提供的最新版安装包仍为 **v0.2.1**。

## Windows 安装包

普通用户可前往 [GitHub Releases](https://github.com/lwy988998/gangyiAI/releases/latest) 下载
`gangyiAI-setup-v0.2.1-x64.exe`，也可下载 `gangyiAI-portable-v0.2.1-x64.zip` 解压免安装使用。
Windows v0.3.4 使用可缩放的独立窗口展示现有动态页面，不再默认打开浏览器。每次冷启动从五款开机动画中随机选择一款，避免连续重复，动画完成后平滑呈现真实首页。“我的课程”提供 API 接口快捷入口，可配置服务商、API 地址、API Key 和模型；密钥保存在 Windows 凭据管理器。首次启动可选择“暂不配置，直接使用”；需要 AI 生成或问答时，也可从托盘“设置”中配置。可用系统按钮最小化、最大化或关闭；关闭主窗口会退出程序。
首次发布版本尚未进行代码签名，Windows SmartScreen 可能显示未知发布者提示。

安装包同页提供 `SHA256SUMS.txt`，可使用以下命令校验下载文件：

```powershell
Get-FileHash .\gangyiAI-setup-v0.2.1-x64.exe -Algorithm SHA256
```

### 发布构建

发布环境需要 Windows 10/11 x64、CMake、MinGW-w64、PowerShell 5.1+ 和 Inno Setup 6。执行：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
```

脚本会下载固定版本的构建依赖、编译服务和原生 Win32 托盘启动器，并生成：

```text
dist\installer\gangyiAI-setup-v0.3.4-x64.exe
dist\installer\gangyiAI-portable-v0.3.4-x64.zip
dist\installer\SHA256SUMS.txt
```

安装程序按当前用户安装，不需要管理员权限。首次启动可跳过 API 配置并直接使用本地功能；AI 生成功能需要稍后在托盘“设置”中配置。配置 API 时可先点击“测试连接”，确认有效后保存启动。API Key 保存在 Windows 凭据管理器中，课程数据库保存在 `%LOCALAPPDATA%\GangyiAI\data`，卸载程序默认保留课程数据。

托盘菜单提供日志查看、数据目录、诊断报告、课程数据库备份恢复和服务重启。服务异常退出后会按限次退避策略自动恢复。免安装版同样把配置和数据保存在 `%LOCALAPPDATA%\GangyiAI`，复制 ZIP 不会自动携带课程数据。

GitHub Actions 会在全新 Windows Runner 中执行编译、CTest、健康检查、静默安装、卸载和免安装版验收。真实 Windows 10/11 图形界面发布检查见 [人工验收清单](docs/windows-release-checklist.md)。

只编译、不生成安装包：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1 -SkipInstaller
```

## 服务器运行

Linux 或已有部署方式仍可通过环境变量启动 `gangyiAI`：

```text
AI_BASE_URL=https://api.deepseek.com/v1
AI_API_KEY=你的密钥
AI_MODEL=deepseek-chat
DATABASE_PATH=gangyiAI.db
PORT=39002
```

默认仅监听 `127.0.0.1`。学习画像的依据保存在本机；评估时只向已配置的 AI 接口发送必要摘要，更新失败会保留最近一次有效结果。

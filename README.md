# gangyiAI

gangyiAI 是一个使用 C++17、Crow 和 SQLite 构建的本地 AI 学习平台。

## Windows 安装包

普通用户可前往 [GitHub Releases](https://github.com/lwy988998/gangyiAI/releases/latest) 下载
`gangyiAI-setup-v0.2.1-x64.exe`，也可下载 `gangyiAI-portable-v0.2.1-x64.zip` 解压免安装使用。
首次启动可选择 DeepSeek、OpenAI 或自定义兼容接口，填写 API Key 后自动获取模型或手工输入模型名称。
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
dist\installer\gangyiAI-setup-v0.2.1-x64.exe
dist\installer\gangyiAI-portable-v0.2.1-x64.zip
dist\installer\SHA256SUMS.txt
```

安装程序按当前用户安装，不需要管理员权限。首次启动时可先点击“测试连接”，确认配置有效后再保存启动。API Key 保存在 Windows 凭据管理器中，课程数据库保存在 `%LOCALAPPDATA%\GangyiAI\data`，卸载程序默认保留课程数据。

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

默认监听 `0.0.0.0`；Windows 启动器会改为仅监听 `127.0.0.1`。

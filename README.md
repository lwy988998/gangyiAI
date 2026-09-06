# gangyiAI

gangyiAI 是一个使用 C++17、Crow 和 SQLite 构建的本地 AI 学习平台。

## Windows 安装包

发布环境需要 Windows 10/11 x64、CMake、MinGW-w64、PowerShell 5.1+ 和 Inno Setup 6。执行：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
```

脚本会下载固定版本的构建依赖、编译服务和原生 Win32 托盘启动器，并生成：

```text
dist\installer\gangyiAI-setup-v0.1.0-x64.exe
```

安装程序按当前用户安装，不需要管理员权限。首次启动时填写 OpenAI Chat Completions 兼容接口地址、API Key 和模型名称。API Key 保存在 Windows 凭据管理器中，课程数据库保存在 `%LOCALAPPDATA%\GangyiAI\data`。

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

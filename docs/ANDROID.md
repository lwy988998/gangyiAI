# 安卓独立运行试用版

目标平台为 Android 10 及以上的 ARM64 手机和平板。安卓版本独立标识为 `1.0.0-preview.1`，电脑版保持 v5.6.0。HarmonyOS 6.1 不作为本版安卓验收设备。

## 运行结构

Kotlin 管理启动、原生设置、Keystore 与前台服务；WebView 使用随 APK 打包的网页；JNI 启动 NDK 编译的现有 C++ 教学主控。`src/server_runtime.cpp` 从桌面入口抽出，桌面保持原有默认策略，不新增教学流程或数据库表。

学习记录在应用私有 `files/data/gangyiAI.db`，网页在 `files/web`，资源缓存在应用私有缓存目录。草稿通过原生桥接保存到私有偏好设置，本机 HTTP 端口改变后仍能恢复。AI 配置由 Keystore 的 AES-GCM 密钥加密，禁用应用数据备份。

内置服务只监听 127.0.0.1，网页、接口及 WebSocket 验证随机会话 Cookie；存在 Origin 时必须与本机服务相同。外部网页交给系统浏览器，WebView 阻止外部网络子资源和本地文件。发布 APK 不包含测试档案、真实 API Key 或签名材料。

## AI 执行与恢复

- 安卓启动只恢复已有内容，不自动提交首页推荐或重新执行未完成任务。上次中断任务提示暂停，用户点击继续后才调用 AI。
- 前台提交的任务仍使用现有主控、工具调用、真实评价和失败重试，讲解、练习、小结独立呈现，各页恢复自己的交流和草稿。
- 进入后台时冻结已有活动任务集合，不新增后台推荐。任务完成后撤销前台服务通知并停止服务。
- 通知提供停止全部 AI 和返回课堂入口，停止与 App 全局暂停共用请求中断机制。
- 拒绝通知权限或系统关闭通知时，进入后台立即暂停 AI。重新打开后由用户选择恢复。
- 服务超时中止请求；系统结束进程后不静默重新提交。省电设置仍可能中止服务，已有有效记录可在下次打开恢复。

## 构建

固定 JDK 17、Gradle 9.4.1、AGP 9.2.1、SDK/目标 API 36、NDK 28.2.13676358、CMake 3.22.1。最低 API 29。工具链与大缓存可放 G 盘，源码不用移动。

```powershell
cd D:\gangyiAI-v551
.\scripts\setup_android.ps1 -ToolchainRoot G:\GangyiAI-Android-Toolchain -WithEmulator
python .\scripts\prepare_android_native.py --root G:\GangyiAI-Android-Toolchain\native --sdk G:\GangyiAI-Android-Toolchain\sdk --abi arm64-v8a
.\scripts\build_android.ps1 -Sign
```

签名材料在仓库外 `D:\codex\home\secrets\gangyiAI-android`；签名密码由当前 Windows 用户的 DPAPI 加密保护。后续沿用同一签名，并提高 `versionCode`，覆盖安装可保留数据。请单独安全备份签名库和可恢复的密码；DPAPI 文件只能在对应 Windows 用户环境解密，签名材料不上传 GitHub。

模拟器构建使用 `-Abi x86_64 -Debug`。自动验收脚本只接受 `emulator-*` 设备，并清理该模拟器的测试应用数据，不能用于真机。模拟器、实际安卓设备、模拟 AI 和真实 AI 分别报告，未验证的项目不计通过。

## 安装与使用

将 ARM64 签名 APK 发送到安卓手机或平板，在文件管理器里打开，按系统提示允许该来源安装应用。首次可以配置 AI，也可以暂不配置。读取已有内容可离线；生成课程、评价和交流需要有效 AI 配置及网络。

设置页填写服务商、API 地址、API Key 与模型。“获取模型”不生成教学内容；“测试连接”会发出一次模型请求。博查搜索配置可选。通知权限用于完成已经启动的后台任务，拒绝时只允许前台执行。

升级使用相同签名与应用标识；卸载会删除手机的学习数据和加密配置。HarmonyOS 6.1 的安装结果不代表安卓设备兼容性。

## 依赖与许可

固定使用 Crow、Asio、nlohmann/json、SQLite 3.53.4、curl 8.22.0 与指定提交的 BoringSSL，逐项校验 SHA256。TLS 库静态链接，依赖许可随 APK 打包。证书包来自 curl 提供的 Mozilla 根证书集合，按 MPL 2.0 使用，HTTPS 证书校验保持开启。

原生库与 APK 检查 16 KB 页面及 ZIP 对齐。实际设备兼容性需在对应安卓设备验收。

// 【逐行批注】 处理条件编译指令：#ifndef WIN32_LEAN_AND_MEAN。
#ifndef WIN32_LEAN_AND_MEAN
// 【逐行批注】 定义编译期常量或宏：#define WIN32_LEAN_AND_MEAN。
#define WIN32_LEAN_AND_MEAN
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 处理条件编译指令：#ifndef NOMINMAX。
#ifndef NOMINMAX
// 【逐行批注】 定义编译期常量或宏：#define NOMINMAX。
#define NOMINMAX
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 <winsock2.h>，为后续实现提供类型或接口声明。
#include <winsock2.h>
// 【逐行批注】 引入头文件 <windows.h>，为后续实现提供类型或接口声明。
#include <windows.h>
// 【逐行批注】 引入头文件 <bcrypt.h>，为后续实现提供类型或接口声明。
#include <bcrypt.h>
// 【逐行批注】 引入头文件 <commdlg.h>，为后续实现提供类型或接口声明。
#include <commdlg.h>
// 【逐行批注】 引入头文件 <shellapi.h>，为后续实现提供类型或接口声明。
#include <shellapi.h>
// 【逐行批注】 引入头文件 <shlobj.h>，为后续实现提供类型或接口声明。
#include <shlobj.h>
// 【逐行批注】 引入头文件 <wincred.h>，为后续实现提供类型或接口声明。
#include <wincred.h>
// 【逐行批注】 引入头文件 <webview/webview.h>，为后续实现提供类型或接口声明。
#include <webview/webview.h>

// 【逐行批注】 引入头文件 "ai_client.hpp"，为后续实现提供类型或接口声明。
#include "ai_client.hpp"
// 【逐行批注】 引入头文件 "search_client.hpp"，为后续实现提供类型或接口声明。
#include "search_client.hpp"
// 【逐行批注】 引入头文件 "launcher_support.hpp"，为后续实现提供类型或接口声明。
#include "launcher_support.hpp"
// 【逐行批注】 引入头文件 "version.hpp"，为后续实现提供类型或接口声明。
#include "version.hpp"
// 【逐行批注】 引入头文件 "windows_resource.h"，为后续实现提供类型或接口声明。
#include "windows_resource.h"

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <array>，为后续实现提供类型或接口声明。
#include <array>
// 【逐行批注】 引入头文件 <atomic>，为后续实现提供类型或接口声明。
#include <atomic>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <cwctype>，为后续实现提供类型或接口声明。
#include <cwctype>
// 【逐行批注】 引入头文件 <ctime>，为后续实现提供类型或接口声明。
#include <ctime>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <mutex>，为后续实现提供类型或接口声明。
#include <mutex>
// 【逐行批注】 引入头文件 <optional>，为后续实现提供类型或接口声明。
#include <optional>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>
// 【逐行批注】 引入头文件 <utility>，为后续实现提供类型或接口声明。
#include <utility>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kWindowClass[]，更新当前运行状态。
constexpr wchar_t kWindowClass[] = L"GangyiAILauncherWindow";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kDesktopWindowClass[]，更新当前运行状态。
constexpr wchar_t kDesktopWindowClass[] = L"GangyiAIDesktopWindow";
// 【逐行批注】 处理条件编译指令：#ifdef GANGYI_TEACHING_PREVIEW。
#ifdef GANGYI_TEACHING_PREVIEW
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kMutexName[]，更新当前运行状态。
constexpr wchar_t kMutexName[] = L"GangyiAI-Teaching-Preview.Launcher.v1";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kRegistryKey[]，更新当前运行状态。
constexpr wchar_t kRegistryKey[] = L"Software\\GangyiAI-Teaching-Preview";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kCredentialTarget[]，更新当前运行状态。
constexpr wchar_t kCredentialTarget[] = L"GangyiAI-Teaching-Preview/APIKey";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kBochaCredentialTarget[]，更新当前运行状态。
constexpr wchar_t kBochaCredentialTarget[] = L"GangyiAI-Teaching-Preview/BochaKey";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kDataName[]，更新当前运行状态。
constexpr wchar_t kDataName[] = L"GangyiAI-Teaching-Preview";
// 【逐行批注】 计算右侧结果并写入 constexpr int kFirstPort，更新当前运行状态。
constexpr int kFirstPort = 59002;
// 【逐行批注】 处理条件编译指令：#elif defined(GANGYI_MY_AI_PREVIEW)。
#elif defined(GANGYI_MY_AI_PREVIEW)
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kMutexName[]，更新当前运行状态。
constexpr wchar_t kMutexName[] = L"GangyiAI-Library-Preview.Launcher.v1";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kRegistryKey[]，更新当前运行状态。
constexpr wchar_t kRegistryKey[] = L"Software\\GangyiAI-Library-Preview";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kCredentialTarget[]，更新当前运行状态。
constexpr wchar_t kCredentialTarget[] = L"GangyiAI-Library-Preview/APIKey";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kBochaCredentialTarget[]，更新当前运行状态。
constexpr wchar_t kBochaCredentialTarget[] = L"GangyiAI-Library-Preview/BochaKey";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kDataName[]，更新当前运行状态。
constexpr wchar_t kDataName[]=L"GangyiAI-Library-Preview";
// 【逐行批注】 计算右侧结果并写入 constexpr int kFirstPort，更新当前运行状态。
constexpr int kFirstPort=49102;
// 【逐行批注】 处理条件编译指令：#else。
#else
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kMutexName[]，更新当前运行状态。
constexpr wchar_t kMutexName[] = L"GangyiAI.Launcher.v1";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kRegistryKey[]，更新当前运行状态。
constexpr wchar_t kRegistryKey[] = L"Software\\GangyiAI";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kCredentialTarget[]，更新当前运行状态。
constexpr wchar_t kCredentialTarget[] = L"GangyiAI/APIKey";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kBochaCredentialTarget[]，更新当前运行状态。
constexpr wchar_t kBochaCredentialTarget[] = L"GangyiAI/BochaKey";
// 【逐行批注】 计算右侧结果并写入 constexpr wchar_t kDataName[]，更新当前运行状态。
constexpr wchar_t kDataName[]=L"GangyiAI";
// 【逐行批注】 计算右侧结果并写入 constexpr int kFirstPort，更新当前运行状态。
constexpr int kFirstPort=39002;
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kTrayMessage，更新当前运行状态。
constexpr UINT kTrayMessage = WM_APP + 1;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kServiceReady，更新当前运行状态。
constexpr UINT kServiceReady = WM_APP + 2;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kClassroomFullscreen，更新当前运行状态。
constexpr UINT kClassroomFullscreen = WM_APP + 63;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kServiceFailed，更新当前运行状态。
constexpr UINT kServiceFailed = WM_APP + 3;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kOpenDesktop，更新当前运行状态。
constexpr UINT kOpenDesktop = WM_APP + 4;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kOpenApiSettings，更新当前运行状态。
constexpr UINT kOpenApiSettings = WM_APP + 8;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kConnectionTestComplete，更新当前运行状态。
constexpr UINT kConnectionTestComplete = WM_APP + 5;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kModelListComplete，更新当前运行状态。
constexpr UINT kModelListComplete = WM_APP + 6;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kRestartTimer，更新当前运行状态。
constexpr UINT kRestartTimer = 1;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kStableTimer，更新当前运行状态。
constexpr UINT kStableTimer = 2;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kDesktopLoadTimer，更新当前运行状态。
constexpr UINT kDesktopLoadTimer = 3;

// 【逐行批注】 计算右侧结果并写入 constexpr int kBaseUrlEdit，更新当前运行状态。
constexpr int kBaseUrlEdit = 101;
// 【逐行批注】 计算右侧结果并写入 constexpr int kApiKeyEdit，更新当前运行状态。
constexpr int kApiKeyEdit = 102;
// 【逐行批注】 计算右侧结果并写入 constexpr int kModelEdit，更新当前运行状态。
constexpr int kModelEdit = 103;
// 【逐行批注】 计算右侧结果并写入 constexpr int kAutoStartCheck，更新当前运行状态。
constexpr int kAutoStartCheck = 104;
// 【逐行批注】 计算右侧结果并写入 constexpr int kSaveButton，更新当前运行状态。
constexpr int kSaveButton = 105;
// 【逐行批注】 计算右侧结果并写入 constexpr int kCancelButton，更新当前运行状态。
constexpr int kCancelButton = 106;
// 【逐行批注】 计算右侧结果并写入 constexpr int kTestButton，更新当前运行状态。
constexpr int kTestButton = 107;
// 【逐行批注】 计算右侧结果并写入 constexpr int kProviderCombo，更新当前运行状态。
constexpr int kProviderCombo = 108;
// 【逐行批注】 计算右侧结果并写入 constexpr int kFetchModelsButton，更新当前运行状态。
constexpr int kFetchModelsButton = 109;
// 【逐行批注】 计算右侧结果并写入 constexpr int kSkipAiButton，更新当前运行状态。
constexpr int kSkipAiButton = 110;
// 【逐行批注】 计算右侧结果并写入 constexpr int kBochaKeyEdit，更新当前运行状态。
constexpr int kBochaKeyEdit = 111;
// 【逐行批注】 计算右侧结果并写入 constexpr int kBochaTestButton，更新当前运行状态。
constexpr int kBochaTestButton = 112;
// 【逐行批注】 计算右侧结果并写入 constexpr int kMenuOpen，更新当前运行状态。
constexpr int kMenuOpen = 201;
// 【逐行批注】 计算右侧结果并写入 constexpr int kMenuSettings，更新当前运行状态。
constexpr int kMenuSettings = 202;
// 【逐行批注】 计算右侧结果并写入 constexpr int kMenuRestart，更新当前运行状态。
constexpr int kMenuRestart = 203;
// 【逐行批注】 计算右侧结果并写入 constexpr int kMenuExit，更新当前运行状态。
constexpr int kMenuExit = 204;
// 【逐行批注】 计算右侧结果并写入 constexpr int kMenuViewLog，更新当前运行状态。
constexpr int kMenuViewLog = 205;
// 【逐行批注】 计算右侧结果并写入 constexpr int kMenuOpenData，更新当前运行状态。
constexpr int kMenuOpenData = 206;
// 【逐行批注】 计算右侧结果并写入 constexpr int kMenuBackup，更新当前运行状态。
constexpr int kMenuBackup = 207;
// 【逐行批注】 计算右侧结果并写入 constexpr int kMenuRestore，更新当前运行状态。
constexpr int kMenuRestore = 208;
// 【逐行批注】 计算右侧结果并写入 constexpr int kMenuDiagnostic，更新当前运行状态。
constexpr int kMenuDiagnostic = 209;

// 【逐行批注】 声明数据结构或类型 Settings 及其成员接口。
struct Settings {
    // 【逐行批注】 计算右侧结果并写入 std::wstring baseUrl，更新当前运行状态。
    std::wstring baseUrl = L"https://api.deepseek.com";
    // 【逐行批注】 计算右侧结果并写入 std::wstring model，更新当前运行状态。
    std::wstring model = L"deepseek-v4-flash";
    // 【逐行批注】 计算右侧结果并写入 gangyi::launcher::AIProvider provider，更新当前运行状态。
    gangyi::launcher::AIProvider provider = gangyi::launcher::AIProvider::DeepSeek;
    // 【逐行批注】 计算右侧结果并写入 bool autoStart，更新当前运行状态。
    bool autoStart = false;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 AppState 及其成员接口。
struct AppState {
    // 【逐行批注】 计算右侧结果并写入 HWND window，更新当前运行状态。
    HWND window = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND desktopWindow，更新当前运行状态。
    HWND desktopWindow = nullptr;
    // 【逐行批注】 计算右侧结果并写入 webview_t desktopView，更新当前运行状态。
    webview_t desktopView = nullptr;
    // 【逐行批注】 计算右侧结果并写入 ICoreWebView2* browser，更新当前运行状态。
    ICoreWebView2* browser = nullptr;
    // 【逐行批注】 执行当前声明或表达式：EventRegistrationToken navigationToken{};。
    EventRegistrationToken navigationToken{};
    // 【逐行批注】 执行当前声明或表达式：EventRegistrationToken newWindowToken{};。
    EventRegistrationToken newWindowToken{};
    // 【逐行批注】 计算右侧结果并写入 bool pageReady，更新当前运行状态。
    bool pageReady = false;
    // 【逐行批注】 计算右侧结果并写入 bool classroomFullscreen，更新当前运行状态。
    bool classroomFullscreen = false;
    // 【逐行批注】 执行当前声明或表达式：WINDOWPLACEMENT beforeClassroomFullscreen{};。
    WINDOWPLACEMENT beforeClassroomFullscreen{};
    // 【逐行批注】 计算右侧结果并写入 LONG_PTR classroomFullscreenStyle，更新当前运行状态。
    LONG_PTR classroomFullscreenStyle = 0;
    // 【逐行批注】 执行当前声明或表达式：std::wstring desktopOrigin;。
    std::wstring desktopOrigin;
    // 启动时先播放一次品牌动画，动画结束后由页面优雅回到首页。
    // 【逐行批注】 计算右侧结果并写入 std::wstring desktopRoute，更新当前运行状态。
    std::wstring desktopRoute = L"/startup";
    // 【逐行批注】 计算右侧结果并写入 int displayPort，更新当前运行状态。
    int displayPort = 0;
    // 【逐行批注】 计算右侧结果并写入 HWND baseUrlEdit，更新当前运行状态。
    HWND baseUrlEdit = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND apiKeyEdit，更新当前运行状态。
    HWND apiKeyEdit = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND bochaKeyEdit，更新当前运行状态。
    HWND bochaKeyEdit = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND bochaTestButton，更新当前运行状态。
    HWND bochaTestButton = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND modelEdit，更新当前运行状态。
    HWND modelEdit = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND providerCombo，更新当前运行状态。
    HWND providerCombo = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND fetchModelsButton，更新当前运行状态。
    HWND fetchModelsButton = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND autoStartCheck，更新当前运行状态。
    HWND autoStartCheck = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND testButton，更新当前运行状态。
    HWND testButton = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND saveButton，更新当前运行状态。
    HWND saveButton = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HWND statusLabel，更新当前运行状态。
    HWND statusLabel = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HANDLE process，更新当前运行状态。
    HANDLE process = nullptr;
    // 【逐行批注】 计算右侧结果并写入 HANDLE job，更新当前运行状态。
    HANDLE job = nullptr;
    // 【逐行批注】 执行当前声明或表达式：std::thread healthThread;。
    std::thread healthThread;
    // 【逐行批注】 执行当前声明或表达式：std::thread connectionTestThread;。
    std::thread connectionTestThread;
    // 【逐行批注】 执行当前声明或表达式：std::thread modelListThread;。
    std::thread modelListThread;
    // 【逐行批注】 执行当前声明或表达式：std::atomic<bool> cancelHealth{false};。
    std::atomic<bool> cancelHealth{false};
    // 【逐行批注】 执行当前声明或表达式：std::atomic<bool> connectionTestRunning{false};。
    std::atomic<bool> connectionTestRunning{false};
    // 【逐行批注】 执行当前声明或表达式：std::atomic<bool> modelListRunning{false};。
    std::atomic<bool> modelListRunning{false};
    // 【逐行批注】 执行当前声明或表达式：std::atomic<bool> shuttingDown{false};。
    std::atomic<bool> shuttingDown{false};
    // 【逐行批注】 执行当前声明或表达式：std::mutex connectionTestMutex;。
    std::mutex connectionTestMutex;
    // 【逐行批注】 执行当前声明或表达式：std::wstring connectionTestMessage;。
    std::wstring connectionTestMessage;
    // 【逐行批注】 计算右侧结果并写入 bool connectionTestSucceeded，更新当前运行状态。
    bool connectionTestSucceeded = false;
    // 【逐行批注】 执行当前声明或表达式：std::mutex modelListMutex;。
    std::mutex modelListMutex;
    // 【逐行批注】 执行当前声明或表达式：std::wstring modelListMessage;。
    std::wstring modelListMessage;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::wstring> availableModels;。
    std::vector<std::wstring> availableModels;
    // 【逐行批注】 计算右侧结果并写入 bool modelListSucceeded，更新当前运行状态。
    bool modelListSucceeded = false;
    // 【逐行批注】 执行当前声明或表达式：Settings settings;。
    Settings settings;
    // 【逐行批注】 执行当前声明或表达式：std::wstring installDir;。
    std::wstring installDir;
    // 【逐行批注】 执行当前声明或表达式：std::wstring dataDir;。
    std::wstring dataDir;
    // 【逐行批注】 执行当前声明或表达式：std::wstring controlToken;。
    std::wstring controlToken;
    // 【逐行批注】 执行当前声明或表达式：std::wstring launchSessionId;。
    std::wstring launchSessionId;
    // 【逐行批注】 计算右侧结果并写入 int port，更新当前运行状态。
    int port = 0;
    // 【逐行批注】 计算右侧结果并写入 bool hasConfig，更新当前运行状态。
    bool hasConfig = false;
    // 【逐行批注】 计算右侧结果并写入 bool skipAiSetup，更新当前运行状态。
    bool skipAiSetup = false;
    // 【逐行批注】 计算右侧结果并写入 bool smokeMode，更新当前运行状态。
    bool smokeMode = false;
    // 【逐行批注】 计算右侧结果并写入 bool background，更新当前运行状态。
    bool background = false;
    // 【逐行批注】 计算右侧结果并写入 bool openWhenReady，更新当前运行状态。
    bool openWhenReady = true;
    // 【逐行批注】 计算右侧结果并写入 DWORD lastExitCode，更新当前运行状态。
    DWORD lastExitCode = 0;
    // 【逐行批注】 执行当前声明或表达式：gangyi::launcher::RestartPolicy restartPolicy;。
    gangyi::launcher::RestartPolicy restartPolicy;
    // 【逐行批注】 执行当前声明或表达式：std::filesystem::path pendingRestoreRollback;。
    std::filesystem::path pendingRestoreRollback;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 执行当前声明或表达式：AppState g;。
AppState g;

// 【逐行批注】 定义函数 enablePerMonitorDpi，执行该函数声明的职责。
void enablePerMonitorDpi() {
    // 高 DPI 屏幕由系统按每个显示器缩放，避免 WebView2 被 DPI 虚拟化后变模糊。
    // 【逐行批注】 计算右侧结果并写入 using SetProcessDpiAwarenessContextFn，更新当前运行状态。
    using SetProcessDpiAwarenessContextFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
    // 【逐行批注】 计算右侧结果并写入 auto context，更新当前运行状态。
    const auto context = reinterpret_cast<DPI_AWARENESS_CONTEXT>(static_cast<INT_PTR>(-4));
    // 【逐行批注】 计算右侧结果并写入 auto user32，更新当前运行状态。
    const auto user32 = GetModuleHandleW(L"user32.dll");
    // 【逐行批注】 计算右侧结果并写入 auto setContext，更新当前运行状态。
    const auto setContext = user32 ? reinterpret_cast<SetProcessDpiAwarenessContextFn>(
        // 【逐行批注】 调用 GetProcAddress 完成本行所需操作：GetProcAddress(user32, "SetProcessDpiAwarenessContext")) : nullptr;。
        GetProcAddress(user32, "SetProcessDpiAwarenessContext")) : nullptr;
    // 【逐行批注】 判断条件 (!setContext || !setContext(context)) SetProcessDPIAware();，成立时执行对应分支。
    if (!setContext || !setContext(context)) SetProcessDPIAware();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 readRegistryString，并通过构造函数初始化其状态。
std::wstring readRegistryString(const wchar_t* name, const std::wstring& fallback = {}) {
    // 【逐行批注】 计算右侧结果并写入 auto read，更新当前运行状态。
    const auto read = [&](const wchar_t* registry) -> std::optional<std::wstring> {
        // 【逐行批注】 计算右侧结果并写入 HKEY key，更新当前运行状态。
        HKEY key = nullptr;
        // 【逐行批注】 判断条件 (RegOpenKeyExW(HKEY_CURRENT_USER, registry, 0, KEY_READ, &key) !…，成立时执行对应分支。
        if (RegOpenKeyExW(HKEY_CURRENT_USER, registry, 0, KEY_READ, &key) != ERROR_SUCCESS) return {};
        // 【逐行批注】 计算右侧结果并写入 DWORD type，更新当前运行状态。
        DWORD type = 0, bytes = 0;
        // 【逐行批注】 判断条件 (RegQueryValueExW(key, name, nullptr, &type, nullptr, &bytes) !=…，成立时执行对应分支。
        if (RegQueryValueExW(key, name, nullptr, &type, nullptr, &bytes) != ERROR_SUCCESS || type != REG_SZ) {
            // 【逐行批注】 调用 RegCloseKey 完成本行所需操作：RegCloseKey(key); return {};。
            RegCloseKey(key); return {};
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 声明对象 value，并通过构造函数初始化其状态。
        std::vector<wchar_t> value(bytes / sizeof(wchar_t) + 1, L'\0');
        // 【逐行批注】 计算右侧结果并写入 LONG result，更新当前运行状态。
        const LONG result = RegQueryValueExW(key, name, nullptr, nullptr, reinterpret_cast<BYTE*>(value.data()), &bytes);
        // 【逐行批注】 调用 RegCloseKey 完成本行所需操作：RegCloseKey(key);。
        RegCloseKey(key);
        // 【逐行批注】 结束当前函数并返回结果：return result == ERROR_SUCCESS ? std::optional<std::wstring>(value.da…。
        return result == ERROR_SUCCESS ? std::optional<std::wstring>(value.data()) : std::nullopt;
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
    auto value = read(kRegistryKey);
// 【逐行批注】 处理条件编译指令：#if defined(GANGYI_TEACHING_PREVIEW) || defined(GANGYI_MY_AI_PREVIEW)。
#if defined(GANGYI_TEACHING_PREVIEW) || defined(GANGYI_MY_AI_PREVIEW)
    // 按字段沿用未设置的正式配置，保留试用版已明确设置的空值与覆盖项。
    // 【逐行批注】 判断条件 (!value) value = read(L"Software\\GangyiAI");，成立时执行对应分支。
    if (!value) value = read(L"Software\\GangyiAI");
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 结束当前函数并返回结果：return value.value_or(fallback);。
    return value.value_or(fallback);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 readRegistryDword，执行该函数声明的职责。
DWORD readRegistryDword(const wchar_t* name, DWORD fallback) {
    // 【逐行批注】 计算右侧结果并写入 HKEY key，更新当前运行状态。
    HKEY key = nullptr;
    // 【逐行批注】 计算右侧结果并写入 DWORD value，更新当前运行状态。
    DWORD value = fallback;
    // 【逐行批注】 计算右侧结果并写入 DWORD type，更新当前运行状态。
    DWORD type = 0;
    // 【逐行批注】 计算右侧结果并写入 DWORD bytes，更新当前运行状态。
    DWORD bytes = sizeof(value);
    // 【逐行批注】 判断条件 (RegOpenKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, KEY_READ, &ke…，成立时执行对应分支。
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, KEY_READ, &key) == ERROR_SUCCESS) {
        // 【逐行批注】 判断条件 (RegQueryValueExW(key, name, nullptr, &type, reinterpret_cast<BY…，成立时执行对应分支。
        if (RegQueryValueExW(key, name, nullptr, &type, reinterpret_cast<BYTE*>(&value), &bytes) != ERROR_SUCCESS ||
            // 【逐行批注】 执行当前声明或表达式：type != REG_DWORD) value = fallback;。
            type != REG_DWORD) value = fallback;
        // 【逐行批注】 调用 RegCloseKey 完成本行所需操作：RegCloseKey(key);。
        RegCloseKey(key);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 writeRegistryString，执行该函数声明的职责。
bool writeRegistryString(const wchar_t* name, const std::wstring& value) {
    // 【逐行批注】 计算右侧结果并写入 HKEY key，更新当前运行状态。
    HKEY key = nullptr;
    // 【逐行批注】 判断条件 (RegCreateKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, nullptr, 0,…，成立时执行对应分支。
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS)
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 计算右侧结果并写入 LONG result，更新当前运行状态。
    const LONG result = RegSetValueExW(key, name, 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()),
        // 【逐行批注】 执行当前声明或表达式：static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));。
        static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
    // 【逐行批注】 调用 RegCloseKey 完成本行所需操作：RegCloseKey(key);。
    RegCloseKey(key);
    // 【逐行批注】 结束当前函数并返回结果：return result == ERROR_SUCCESS;。
    return result == ERROR_SUCCESS;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 writeRegistryDword，执行该函数声明的职责。
bool writeRegistryDword(const wchar_t* name, DWORD value) {
    // 【逐行批注】 计算右侧结果并写入 HKEY key，更新当前运行状态。
    HKEY key = nullptr;
    // 【逐行批注】 判断条件 (RegCreateKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, nullptr, 0,…，成立时执行对应分支。
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS)
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 计算右侧结果并写入 LONG result，更新当前运行状态。
    const LONG result = RegSetValueExW(key, name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value));
    // 【逐行批注】 调用 RegCloseKey 完成本行所需操作：RegCloseKey(key);。
    RegCloseKey(key);
    // 【逐行批注】 结束当前函数并返回结果：return result == ERROR_SUCCESS;。
    return result == ERROR_SUCCESS;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 readCredential，执行该函数声明的职责。
std::wstring readCredential(const wchar_t* target) {
    // 【逐行批注】 计算右侧结果并写入 PCREDENTIALW credential，更新当前运行状态。
    PCREDENTIALW credential = nullptr;
    // 【逐行批注】 判断条件 (!CredReadW(target, CRED_TYPE_GENERIC, 0, &credential)) return {…，成立时执行对应分支。
    if (!CredReadW(target, CRED_TYPE_GENERIC, 0, &credential)) return {};
    // 【逐行批注】 执行当前声明或表达式：std::wstring value;。
    std::wstring value;
    // 【逐行批注】 判断条件 (credential->CredentialBlob && credential->CredentialBlobSize % …，成立时执行对应分支。
    if (credential->CredentialBlob && credential->CredentialBlobSize % sizeof(wchar_t) == 0) {
        // 【逐行批注】 执行当前声明或表达式：value.assign(reinterpret_cast<const wchar_t*>(credential->CredentialBlob),。
        value.assign(reinterpret_cast<const wchar_t*>(credential->CredentialBlob),
            // 【逐行批注】 执行当前声明或表达式：credential->CredentialBlobSize / sizeof(wchar_t));。
            credential->CredentialBlobSize / sizeof(wchar_t));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 CredFree 完成本行所需操作：CredFree(credential);。
    CredFree(credential);
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 writeCredential，执行该函数声明的职责。
bool writeCredential(const wchar_t* target, const std::wstring& value) {
    // 【逐行批注】 判断条件 (value.empty() || value.size() * sizeof(wchar_t) > CRED_MAX_CRED…，成立时执行对应分支。
    if (value.empty() || value.size() * sizeof(wchar_t) > CRED_MAX_CREDENTIAL_BLOB_SIZE) return false;
    // 【逐行批注】 执行当前声明或表达式：CREDENTIALW credential{};。
    CREDENTIALW credential{};
    // 【逐行批注】 计算右侧结果并写入 credential.Type，更新当前运行状态。
    credential.Type = CRED_TYPE_GENERIC;
    // 【逐行批注】 计算右侧结果并写入 credential.TargetName，更新当前运行状态。
    credential.TargetName = const_cast<wchar_t*>(target);
    // 【逐行批注】 计算右侧结果并写入 credential.CredentialBlobSize，更新当前运行状态。
    credential.CredentialBlobSize = static_cast<DWORD>(value.size() * sizeof(wchar_t));
    // 【逐行批注】 计算右侧结果并写入 credential.CredentialBlob，更新当前运行状态。
    credential.CredentialBlob = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(value.data()));
    // 【逐行批注】 计算右侧结果并写入 credential.Persist，更新当前运行状态。
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    // 【逐行批注】 计算右侧结果并写入 credential.UserName，更新当前运行状态。
    credential.UserName = const_cast<wchar_t*>(L"gangyiAI");
    // 【逐行批注】 结束当前函数并返回结果：return CredWriteW(&credential, 0) == TRUE;。
    return CredWriteW(&credential, 0) == TRUE;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 readApiKey，执行该函数声明的职责。
std::wstring readApiKey() {
    // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
    auto value=readCredential(kCredentialTarget);
// 【逐行批注】 处理条件编译指令：#if defined(GANGYI_TEACHING_PREVIEW) || defined(GANGYI_MY_AI_PREVIEW)。
#if defined(GANGYI_TEACHING_PREVIEW) || defined(GANGYI_MY_AI_PREVIEW)
    // 【逐行批注】 计算右侧结果并写入 if(value.empty())value，更新当前运行状态。
    if(value.empty())value=readCredential(L"GangyiAI/APIKey");
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 readBochaKey，执行该函数声明的职责。
std::wstring readBochaKey() {
    // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
    auto value=readCredential(kBochaCredentialTarget);
// 【逐行批注】 处理条件编译指令：#if defined(GANGYI_TEACHING_PREVIEW) || defined(GANGYI_MY_AI_PREVIEW)。
#if defined(GANGYI_TEACHING_PREVIEW) || defined(GANGYI_MY_AI_PREVIEW)
    // 【逐行批注】 调用 readRegistryDword、readCredential 完成本行所需操作：if(value.empty()&&!readRegistryDword(L"BochaDisabled",0))value=readCr…。
    if(value.empty()&&!readRegistryDword(L"BochaDisabled",0))value=readCredential(L"GangyiAI/BochaKey");
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 writeApiKey，执行该函数声明的职责。
bool writeApiKey(const std::wstring& value) { return writeCredential(kCredentialTarget, value); }

// 【逐行批注】 定义函数 executablePath，执行该函数声明的职责。
std::wstring executablePath() {
    // 【逐行批注】 声明对象 path，并通过构造函数初始化其状态。
    std::vector<wchar_t> path(32768, L'\0');
    // 【逐行批注】 计算右侧结果并写入 DWORD length，更新当前运行状态。
    const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    // 【逐行批注】 结束当前函数并返回结果：return std::wstring(path.data(), length);。
    return std::wstring(path.data(), length);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 localAppDataPath，执行该函数声明的职责。
std::wstring localAppDataPath() {
    // 【逐行批注】 计算右侧结果并写入 DWORD required，更新当前运行状态。
    const DWORD required = GetEnvironmentVariableW(L"LOCALAPPDATA", nullptr, 0);
    // 【逐行批注】 判断条件 (required > 1) {，成立时执行对应分支。
    if (required > 1) {
        // 【逐行批注】 声明对象 value，并通过构造函数初始化其状态。
        std::vector<wchar_t> value(required, L'\0');
        // 【逐行批注】 判断条件 (GetEnvironmentVariableW(L"LOCALAPPDATA", value.data(), required…，成立时执行对应分支。
        if (GetEnvironmentVariableW(L"LOCALAPPDATA", value.data(), required) > 0) return value.data();
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：std::array<wchar_t, MAX_PATH> fallback{};。
    std::array<wchar_t, MAX_PATH> fallback{};
    // 【逐行批注】 判断条件 (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA | CSIDL…，成立时执行对应分支。
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, nullptr,
        // 【逐行批注】 执行当前声明或表达式：SHGFP_TYPE_CURRENT, fallback.data()))) return fallback.data();。
        SHGFP_TYPE_CURRENT, fallback.data()))) return fallback.data();
    // 【逐行批注】 结束当前函数并返回结果：return {};。
    return {};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 loadSettings，执行该函数声明的职责。
Settings loadSettings() {
    // 【逐行批注】 执行当前声明或表达式：Settings settings;。
    Settings settings;
    // 【逐行批注】 计算右侧结果并写入 settings.baseUrl，更新当前运行状态。
    settings.baseUrl = readRegistryString(L"AIBaseUrl", settings.baseUrl);
    // 【逐行批注】 计算右侧结果并写入 settings.model，更新当前运行状态。
    settings.model = readRegistryString(L"AIModel", settings.model);
    // 【逐行批注】 计算右侧结果并写入 std::wstring providerId，更新当前运行状态。
    const std::wstring providerId = readRegistryString(L"AIProvider");
    // 【逐行批注】 计算右侧结果并写入 settings.provider，更新当前运行状态。
    settings.provider = providerId.empty() ? gangyi::launcher::inferProvider(settings.baseUrl)
                                           // 【逐行批注】 调用 providerFromId 完成本行所需操作：: gangyi::launcher::providerFromId(providerId);。
                                           : gangyi::launcher::providerFromId(providerId);
    // 【逐行批注】 判断条件 (settings.provider == gangyi::launcher::AIProvider::DeepSeek && …，成立时执行对应分支。
    if (settings.provider == gangyi::launcher::AIProvider::DeepSeek && settings.model == L"deepseek-chat") {
        // 【逐行批注】 计算右侧结果并写入 settings.model，更新当前运行状态。
        settings.model = L"deepseek-v4-flash";
        // 【逐行批注】 调用 writeRegistryString 完成本行所需操作：writeRegistryString(L"AIModel", settings.model);。
        writeRegistryString(L"AIModel", settings.model);
    // 【逐行批注】 执行当前声明或表达式：} else if (settings.provider == gangyi::launcher::AIProvider::DeepSeek && settings.m…。
    } else if (settings.provider == gangyi::launcher::AIProvider::DeepSeek && settings.model == L"deepseek-reasoner") {
        // 【逐行批注】 计算右侧结果并写入 settings.model，更新当前运行状态。
        settings.model = L"deepseek-v4-pro";
        // 【逐行批注】 调用 writeRegistryString 完成本行所需操作：writeRegistryString(L"AIModel", settings.model);。
        writeRegistryString(L"AIModel", settings.model);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 settings.autoStart，更新当前运行状态。
    settings.autoStart = readRegistryDword(L"AutoStart", 0) != 0;
    // 【逐行批注】 结束当前函数并返回结果：return settings;。
    return settings;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 providerIndex，执行该函数声明的职责。
int providerIndex(gangyi::launcher::AIProvider provider) {
    // 【逐行批注】 根据表达式分支处理：switch (provider) {。
    switch (provider) {
    // 【逐行批注】 声明 switch 的分支入口：case gangyi::launcher::AIProvider::DeepSeek: return 0;。
    case gangyi::launcher::AIProvider::DeepSeek: return 0;
    // 【逐行批注】 声明 switch 的分支入口：case gangyi::launcher::AIProvider::OpenAI: return 1;。
    case gangyi::launcher::AIProvider::OpenAI: return 1;
    // 【逐行批注】 声明 switch 的分支入口：default: return 2;。
    default: return 2;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 selectedProvider，执行该函数声明的职责。
gangyi::launcher::AIProvider selectedProvider() {
    // 【逐行批注】 计算右侧结果并写入 LRESULT index，更新当前运行状态。
    const LRESULT index = SendMessageW(g.providerCombo, CB_GETCURSEL, 0, 0);
    // 【逐行批注】 判断条件 (index == 0) return gangyi::launcher::AIProvider::DeepSeek;，成立时执行对应分支。
    if (index == 0) return gangyi::launcher::AIProvider::DeepSeek;
    // 【逐行批注】 判断条件 (index == 1) return gangyi::launcher::AIProvider::OpenAI;，成立时执行对应分支。
    if (index == 1) return gangyi::launcher::AIProvider::OpenAI;
    // 【逐行批注】 结束当前函数并返回结果：return gangyi::launcher::AIProvider::Custom;。
    return gangyi::launcher::AIProvider::Custom;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 configureAutoStart，执行该函数声明的职责。
bool configureAutoStart(bool enabled) {
    // 【逐行批注】 计算右侧结果并写入 HKEY key，更新当前运行状态。
    HKEY key = nullptr;
    // 【逐行批注】 计算右侧结果并写入 constexpr wchar_t runKey[]，更新当前运行状态。
    constexpr wchar_t runKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    // 【逐行批注】 判断条件 (RegOpenKeyExW(HKEY_CURRENT_USER, runKey, 0, KEY_SET_VALUE, &key…，成立时执行对应分支。
    if (RegOpenKeyExW(HKEY_CURRENT_USER, runKey, 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS) return false;
    // 【逐行批注】 计算右侧结果并写入 LONG result，更新当前运行状态。
    LONG result = ERROR_SUCCESS;
    // 【逐行批注】 判断条件 (enabled) {，成立时执行对应分支。
    if (enabled) {
        // 【逐行批注】 计算右侧结果并写入 std::wstring command，更新当前运行状态。
        const std::wstring command = L"\"" + executablePath() + L"\" --background";
        // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
        result = RegSetValueExW(key, kDataName, 0, REG_SZ, reinterpret_cast<const BYTE*>(command.c_str()),
            // 【逐行批注】 执行当前声明或表达式：static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t)));。
            static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t)));
    // 【逐行批注】 执行当前声明或表达式：} else {。
    } else {
        // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
        result = RegDeleteValueW(key, kDataName);
        // 【逐行批注】 判断条件 (result == ERROR_FILE_NOT_FOUND) result = ERROR_SUCCESS;，成立时执行对应分支。
        if (result == ERROR_FILE_NOT_FOUND) result = ERROR_SUCCESS;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 RegCloseKey 完成本行所需操作：RegCloseKey(key);。
    RegCloseKey(key);
    // 【逐行批注】 结束当前函数并返回结果：return result == ERROR_SUCCESS;。
    return result == ERROR_SUCCESS;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 removeLocalSettings，执行该函数声明的职责。
void removeLocalSettings() {
    // 【逐行批注】 调用 configureAutoStart 完成本行所需操作：configureAutoStart(false);。
    configureAutoStart(false);
    // 【逐行批注】 调用 CredDeleteW 完成本行所需操作：CredDeleteW(kCredentialTarget, CRED_TYPE_GENERIC, 0);。
    CredDeleteW(kCredentialTarget, CRED_TYPE_GENERIC, 0);
    // 【逐行批注】 调用 CredDeleteW 完成本行所需操作：CredDeleteW(kBochaCredentialTarget, CRED_TYPE_GENERIC, 0);。
    CredDeleteW(kBochaCredentialTarget, CRED_TYPE_GENERIC, 0);
    // 【逐行批注】 调用 RegDeleteTreeW 完成本行所需操作：RegDeleteTreeW(HKEY_CURRENT_USER, kRegistryKey);。
    RegDeleteTreeW(HKEY_CURRENT_USER, kRegistryKey);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 startsWithIgnoreCase，执行该函数声明的职责。
bool startsWithIgnoreCase(const std::wstring& value, const wchar_t* prefix) {
    // 【逐行批注】 计算右侧结果并写入 size_t length，更新当前运行状态。
    const size_t length = wcslen(prefix);
    // 【逐行批注】 结束当前函数并返回结果：return value.size() >= length && _wcsnicmp(value.c_str(), prefix, len…。
    return value.size() >= length && _wcsnicmp(value.c_str(), prefix, length) == 0;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 validBaseUrl，执行该函数声明的职责。
bool validBaseUrl(const std::wstring& value) {
    // 【逐行批注】 结束当前函数并返回结果：return startsWithIgnoreCase(value, L"https://") || startsWithIgnoreCa…。
    return startsWithIgnoreCase(value, L"https://") || startsWithIgnoreCase(value, L"http://");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 canStartService，执行该函数声明的职责。
bool canStartService(const Settings& settings, bool hasApiKey, bool skipAiSetup) {
    // 【逐行批注】 结束当前函数并返回结果：return skipAiSetup || (validBaseUrl(settings.baseUrl) && !settings.mo…。
    return skipAiSetup || (validBaseUrl(settings.baseUrl) && !settings.model.empty() && hasApiKey);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 toUtf8，执行该函数声明的职责。
std::string toUtf8(const std::wstring& value) {
    // 【逐行批注】 判断条件 (value.empty()) return {};，成立时执行对应分支。
    if (value.empty()) return {};
    // 【逐行批注】 计算右侧结果并写入 int size，更新当前运行状态。
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        // 【逐行批注】 执行当前声明或表达式：nullptr, 0, nullptr, nullptr);。
        nullptr, 0, nullptr, nullptr);
    // 【逐行批注】 判断条件 (size <= 0) return {};，成立时执行对应分支。
    if (size <= 0) return {};
    // 【逐行批注】 声明对象 result，并通过构造函数初始化其状态。
    std::string result(static_cast<size_t>(size), '\0');
    // 【逐行批注】 调用 WideCharToMultiByte 完成本行所需操作：WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.…。
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        // 【逐行批注】 执行当前声明或表达式：result.data(), size, nullptr, nullptr);。
        result.data(), size, nullptr, nullptr);
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 toWide，执行该函数声明的职责。
std::wstring toWide(const std::string& value) {
    // 【逐行批注】 判断条件 (value.empty()) return {};，成立时执行对应分支。
    if (value.empty()) return {};
    // 【逐行批注】 计算右侧结果并写入 int size，更新当前运行状态。
    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    // 【逐行批注】 判断条件 (size <= 0) return {};，成立时执行对应分支。
    if (size <= 0) return {};
    // 【逐行批注】 声明对象 result，并通过构造函数初始化其状态。
    std::wstring result(static_cast<size_t>(size), L'\0');
    // 【逐行批注】 调用 MultiByteToWideChar 完成本行所需操作：MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.…。
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 controlText，执行该函数声明的职责。
std::wstring controlText(HWND control) {
    // 【逐行批注】 计算右侧结果并写入 int length，更新当前运行状态。
    const int length = GetWindowTextLengthW(control);
    // 【逐行批注】 声明对象 value，并通过构造函数初始化其状态。
    std::vector<wchar_t> value(static_cast<size_t>(length) + 1, L'\0');
    // 【逐行批注】 调用 GetWindowTextW 完成本行所需操作：GetWindowTextW(control, value.data(), static_cast<int>(value.size()));。
    GetWindowTextW(control, value.data(), static_cast<int>(value.size()));
    // 【逐行批注】 结束当前函数并返回结果：return value.data();。
    return value.data();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 randomToken，执行该函数声明的职责。
std::wstring randomToken() {
    // 【逐行批注】 执行当前声明或表达式：std::array<unsigned char, 32> bytes{};。
    std::array<unsigned char, 32> bytes{};
    // 【逐行批注】 判断条件 (BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(bytes…，成立时执行对应分支。
    if (BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(bytes.size()), BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0)
        // 【逐行批注】 结束当前函数并返回结果：return {};。
        return {};
    // 【逐行批注】 计算右侧结果并写入 constexpr wchar_t hex[]，更新当前运行状态。
    constexpr wchar_t hex[] = L"0123456789abcdef";
    // 【逐行批注】 执行当前声明或表达式：std::wstring token;。
    std::wstring token;
    // 【逐行批注】 执行当前声明或表达式：token.reserve(bytes.size() * 2);。
    token.reserve(bytes.size() * 2);
    // 【逐行批注】 按循环规则重复处理：for (const unsigned char value : bytes) {。
    for (const unsigned char value : bytes) {
        // 【逐行批注】 执行当前声明或表达式：token.push_back(hex[value >> 4]);。
        token.push_back(hex[value >> 4]);
        // 【逐行批注】 执行当前声明或表达式：token.push_back(hex[value & 0x0f]);。
        token.push_back(hex[value & 0x0f]);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return token;。
    return token;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 findAvailablePort，执行该函数声明的职责。
int findAvailablePort() {
    // 【逐行批注】 按循环规则重复处理：for (int port = kFirstPort; port <= kFirstPort+97; ++port) {。
    for (int port = kFirstPort; port <= kFirstPort+97; ++port) {
        // 【逐行批注】 计算右侧结果并写入 SOCKET candidate，更新当前运行状态。
        SOCKET candidate = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // 【逐行批注】 判断条件 (candidate == INVALID_SOCKET) return 0;，成立时执行对应分支。
        if (candidate == INVALID_SOCKET) return 0;
        // 【逐行批注】 执行当前声明或表达式：sockaddr_in address{};。
        sockaddr_in address{};
        // 【逐行批注】 计算右侧结果并写入 address.sin_family，更新当前运行状态。
        address.sin_family = AF_INET;
        // 【逐行批注】 计算右侧结果并写入 address.sin_addr.s_addr，更新当前运行状态。
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        // 【逐行批注】 计算右侧结果并写入 address.sin_port，更新当前运行状态。
        address.sin_port = htons(static_cast<u_short>(port));
        // 【逐行批注】 计算右侧结果并写入 bool available，更新当前运行状态。
        const bool available = bind(candidate, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == 0;
        // 【逐行批注】 调用 closesocket 完成本行所需操作：closesocket(candidate);。
        closesocket(candidate);
        // 【逐行批注】 判断条件 (available) return port;，成立时执行对应分支。
        if (available) return port;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return 0;。
    return 0;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 processRunning，执行该函数声明的职责。
bool processRunning() {
    // 【逐行批注】 判断条件 (!g.process) return false;，成立时执行对应分支。
    if (!g.process) return false;
    // 【逐行批注】 计算右侧结果并写入 DWORD exitCode，更新当前运行状态。
    DWORD exitCode = 0;
    // 【逐行批注】 结束当前函数并返回结果：return GetExitCodeProcess(g.process, &exitCode) && exitCode == STILL_…。
    return GetExitCodeProcess(g.process, &exitCode) && exitCode == STILL_ACTIVE;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 localHttpRequest，并通过构造函数初始化其状态。
bool localHttpRequest(int port, const char* method, const char* path, const std::wstring& token = {}) {
    // 【逐行批注】 计算右侧结果并写入 SOCKET client，更新当前运行状态。
    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // 【逐行批注】 判断条件 (client == INVALID_SOCKET) return false;，成立时执行对应分支。
    if (client == INVALID_SOCKET) return false;
    // 【逐行批注】 计算右侧结果并写入 DWORD timeout，更新当前运行状态。
    DWORD timeout = 1000;
    // 【逐行批注】 调用 setsockopt 完成本行所需操作：setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const ch…。
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    // 【逐行批注】 调用 setsockopt 完成本行所需操作：setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const ch…。
    setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    // 【逐行批注】 执行当前声明或表达式：sockaddr_in address{};。
    sockaddr_in address{};
    // 【逐行批注】 计算右侧结果并写入 address.sin_family，更新当前运行状态。
    address.sin_family = AF_INET;
    // 【逐行批注】 计算右侧结果并写入 address.sin_addr.s_addr，更新当前运行状态。
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    // 【逐行批注】 计算右侧结果并写入 address.sin_port，更新当前运行状态。
    address.sin_port = htons(static_cast<u_short>(port));
    // 【逐行批注】 判断条件 (connect(client, reinterpret_cast<const sockaddr*>(&address), si…，成立时执行对应分支。
    if (connect(client, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0) {
        // 【逐行批注】 调用 closesocket 完成本行所需操作：closesocket(client);。
        closesocket(client);
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 std::string request，更新当前运行状态。
    std::string request = std::string(method) + " " + path + " HTTP/1.1\r\nHost: 127.0.0.1:" +
        // 【逐行批注】 调用 to_string 完成本行所需操作：std::to_string(port) + "\r\nConnection: close\r\n";。
        std::to_string(port) + "\r\nConnection: close\r\n";
    // 【逐行批注】 判断条件 (!token.empty()) {，成立时执行对应分支。
    if (!token.empty()) {
        // 【逐行批注】 执行当前声明或表达式：std::string narrowToken;。
        std::string narrowToken;
        // 【逐行批注】 执行当前声明或表达式：narrowToken.reserve(token.size());。
        narrowToken.reserve(token.size());
        // 【逐行批注】 按循环规则重复处理：for (const wchar_t character : token) narrowToken.push_back(static_cast<ch…。
        for (const wchar_t character : token) narrowToken.push_back(static_cast<char>(character));
        // 【逐行批注】 计算右侧结果并写入 request +，更新当前运行状态。
        request += "X-Gangyi-Control-Token: " + narrowToken + "\r\n";
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 request +，更新当前运行状态。
    request += "Content-Length: 0\r\n\r\n";
    // 【逐行批注】 判断条件 (send(client, request.data(), static_cast<int>(request.size()), …，成立时执行对应分支。
    if (send(client, request.data(), static_cast<int>(request.size()), 0) == SOCKET_ERROR) {
        // 【逐行批注】 调用 closesocket 完成本行所需操作：closesocket(client);。
        closesocket(client);
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：std::array<char, 512> response{};。
    std::array<char, 512> response{};
    // 【逐行批注】 计算右侧结果并写入 int received，更新当前运行状态。
    const int received = recv(client, response.data(), static_cast<int>(response.size() - 1), 0);
    // 【逐行批注】 调用 closesocket 完成本行所需操作：closesocket(client);。
    closesocket(client);
    // 【逐行批注】 结束当前函数并返回结果：return received > 0 && std::string(response.data(), static_cast<size_…。
    return received > 0 && std::string(response.data(), static_cast<size_t>(received)).find(" 200 ") != std::string::npos;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 childEnvironment，执行该函数声明的职责。
std::vector<wchar_t> childEnvironment(const std::vector<std::pair<std::wstring, std::wstring>>& overrides) {
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::wstring> entries;。
    std::vector<std::wstring> entries;
    // 【逐行批注】 计算右侧结果并写入 LPWCH environment，更新当前运行状态。
    LPWCH environment = GetEnvironmentStringsW();
    // 【逐行批注】 判断条件 (environment) {，成立时执行对应分支。
    if (environment) {
        // 【逐行批注】 按循环规则重复处理：for (const wchar_t* cursor = environment; *cursor; cursor += wcslen(cursor…。
        for (const wchar_t* cursor = environment; *cursor; cursor += wcslen(cursor) + 1) {
            // 【逐行批注】 计算右侧结果并写入 bool replaced，更新当前运行状态。
            bool replaced = false;
            // 【逐行批注】 按循环规则重复处理：for (const auto& [name, value] : overrides) {。
            for (const auto& [name, value] : overrides) {
                // 【逐行批注】 计算右侧结果并写入 size_t equals，更新当前运行状态。
                const size_t equals = std::wstring(cursor).find(L'=', cursor[0] == L'=' ? 1 : 0);
                // 【逐行批注】 判断条件 (equals != std::wstring::npos && equals == name.size() &&，成立时执行对应分支。
                if (equals != std::wstring::npos && equals == name.size() &&
                    // 【逐行批注】 调用 _wcsnicmp 完成本行所需操作：_wcsnicmp(cursor, name.c_str(), name.size()) == 0) {。
                    _wcsnicmp(cursor, name.c_str(), name.size()) == 0) {
                    // 【逐行批注】 计算右侧结果并写入 replaced，更新当前运行状态。
                    replaced = true;
                    // 【逐行批注】 跳出当前循环或分支。
                    break;
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (!replaced) entries.emplace_back(cursor);，成立时执行对应分支。
            if (!replaced) entries.emplace_back(cursor);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 FreeEnvironmentStringsW 完成本行所需操作：FreeEnvironmentStringsW(environment);。
        FreeEnvironmentStringsW(environment);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& [name, value] : overrides) entries.push_back(name + L"=" …。
    for (const auto& [name, value] : overrides) entries.push_back(name + L"=" + value);
    // 【逐行批注】 调用 sort 完成本行所需操作：std::sort(entries.begin(), entries.end(), [](const std::wstring& left…。
    std::sort(entries.begin(), entries.end(), [](const std::wstring& left, const std::wstring& right) {
        // 【逐行批注】 结束当前函数并返回结果：return _wcsicmp(left.c_str(), right.c_str()) < 0;。
        return _wcsicmp(left.c_str(), right.c_str()) < 0;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 计算右侧结果并写入 size_t size，更新当前运行状态。
    size_t size = 1;
    // 【逐行批注】 按循环规则重复处理：for (const auto& entry : entries) size += entry.size() + 1;。
    for (const auto& entry : entries) size += entry.size() + 1;
    // 【逐行批注】 声明对象 block，并通过构造函数初始化其状态。
    std::vector<wchar_t> block(size, L'\0');
    // 【逐行批注】 计算右侧结果并写入 wchar_t* output，更新当前运行状态。
    wchar_t* output = block.data();
    // 【逐行批注】 按循环规则重复处理：for (const auto& entry : entries) {。
    for (const auto& entry : entries) {
        // 【逐行批注】 调用 copy 完成本行所需操作：std::copy(entry.begin(), entry.end(), output);。
        std::copy(entry.begin(), entry.end(), output);
        // 【逐行批注】 计算右侧结果并写入 output +，更新当前运行状态。
        output += entry.size() + 1;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return block;。
    return block;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 updateTrayTip，执行该函数声明的职责。
void updateTrayTip(const wchar_t* text) {
    // 【逐行批注】 执行当前声明或表达式：NOTIFYICONDATAW icon{};。
    NOTIFYICONDATAW icon{};
    // 【逐行批注】 计算右侧结果并写入 icon.cbSize，更新当前运行状态。
    icon.cbSize = sizeof(icon);
    // 【逐行批注】 计算右侧结果并写入 icon.hWnd，更新当前运行状态。
    icon.hWnd = g.window;
    // 【逐行批注】 计算右侧结果并写入 icon.uID，更新当前运行状态。
    icon.uID = 1;
    // 【逐行批注】 计算右侧结果并写入 icon.uFlags，更新当前运行状态。
    icon.uFlags = NIF_TIP;
    // 【逐行批注】 调用 wcsncpy_s、ARRAYSIZE 完成本行所需操作：wcsncpy_s(icon.szTip, ARRAYSIZE(icon.szTip), text, _TRUNCATE);。
    wcsncpy_s(icon.szTip, ARRAYSIZE(icon.szTip), text, _TRUNCATE);
    // 【逐行批注】 调用 Shell_NotifyIconW 完成本行所需操作：Shell_NotifyIconW(NIM_MODIFY, &icon);。
    Shell_NotifyIconW(NIM_MODIFY, &icon);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 showFailure，执行该函数声明的职责。
void showFailure(const wchar_t* message) {
    // 【逐行批注】 调用 updateTrayTip 完成本行所需操作：updateTrayTip(L"钢一定制AI - 启动失败");。
    updateTrayTip(L"钢一定制AI - 启动失败");
    // 【逐行批注】 判断条件 (g.background) {，成立时执行对应分支。
    if (g.background) {
        // 【逐行批注】 执行当前声明或表达式：NOTIFYICONDATAW icon{};。
        NOTIFYICONDATAW icon{};
        // 【逐行批注】 计算右侧结果并写入 icon.cbSize，更新当前运行状态。
        icon.cbSize = sizeof(icon);
        // 【逐行批注】 计算右侧结果并写入 icon.hWnd，更新当前运行状态。
        icon.hWnd = g.window;
        // 【逐行批注】 计算右侧结果并写入 icon.uID，更新当前运行状态。
        icon.uID = 1;
        // 【逐行批注】 计算右侧结果并写入 icon.uFlags，更新当前运行状态。
        icon.uFlags = NIF_INFO;
        // 【逐行批注】 调用 wcsncpy_s、ARRAYSIZE 完成本行所需操作：wcsncpy_s(icon.szInfoTitle, ARRAYSIZE(icon.szInfoTitle), L"钢一定制AI 启动失…。
        wcsncpy_s(icon.szInfoTitle, ARRAYSIZE(icon.szInfoTitle), L"钢一定制AI 启动失败", _TRUNCATE);
        // 【逐行批注】 调用 wcsncpy_s、ARRAYSIZE 完成本行所需操作：wcsncpy_s(icon.szInfo, ARRAYSIZE(icon.szInfo), message, _TRUNCATE);。
        wcsncpy_s(icon.szInfo, ARRAYSIZE(icon.szInfo), message, _TRUNCATE);
        // 【逐行批注】 计算右侧结果并写入 icon.dwInfoFlags，更新当前运行状态。
        icon.dwInfoFlags = NIIF_ERROR;
        // 【逐行批注】 调用 Shell_NotifyIconW 完成本行所需操作：Shell_NotifyIconW(NIM_MODIFY, &icon);。
        Shell_NotifyIconW(NIM_MODIFY, &icon);
    // 【逐行批注】 执行当前声明或表达式：} else {。
    } else {
        // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, message, L"钢一定制AI", MB_OK | MB_ICONERROR);。
        MessageBoxW(g.window, message, L"钢一定制AI", MB_OK | MB_ICONERROR);
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 isExternalWebUrl，执行该函数声明的职责。
bool isExternalWebUrl(const std::wstring& url) {
    // 【逐行批注】 判断条件 (!startsWithIgnoreCase(url, L"https://") && !startsWithIgnoreCas…，成立时执行对应分支。
    if (!startsWithIgnoreCase(url, L"https://") && !startsWithIgnoreCase(url, L"http://")) return false;
    // 【逐行批注】 计算右侧结果并写入 size_t authority，更新当前运行状态。
    const size_t authority = url.find(L"://") + 3;
    // 【逐行批注】 计算右侧结果并写入 size_t end，更新当前运行状态。
    const size_t end = url.find_first_of(L"/?#", authority);
    // 【逐行批注】 计算右侧结果并写入 std::wstring address，更新当前运行状态。
    const std::wstring address = url.substr(authority, end - authority);
    // 【逐行批注】 判断条件 (address.empty()) return false;，成立时执行对应分支。
    if (address.empty()) return false;
    // 【逐行批注】 计算右侧结果并写入 size_t port，更新当前运行状态。
    const size_t port = address.front() == L'[' ? address.find(L']') + 1 : address.find(L':');
    // 【逐行批注】 计算右侧结果并写入 std::wstring host，更新当前运行状态。
    const std::wstring host = address.substr(0, port);
    // 【逐行批注】 结束当前函数并返回结果：return _wcsicmp(host.c_str(), L"127.0.0.1") != 0 &&。
    return _wcsicmp(host.c_str(), L"127.0.0.1") != 0 &&
           // 【逐行批注】 调用 _wcsicmp 完成本行所需操作：_wcsicmp(host.c_str(), L"localhost") != 0 &&。
           _wcsicmp(host.c_str(), L"localhost") != 0 &&
           // 【逐行批注】 调用 _wcsicmp 完成本行所需操作：_wcsicmp(host.c_str(), L"[::1]") != 0;。
           _wcsicmp(host.c_str(), L"[::1]") != 0;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 isCurrentLocalUrl，执行该函数声明的职责。
bool isCurrentLocalUrl(const std::wstring& url) {
    // 【逐行批注】 判断条件 (url.compare(0, g.desktopOrigin.size(), g.desktopOrigin) != 0) r…，成立时执行对应分支。
    if (url.compare(0, g.desktopOrigin.size(), g.desktopOrigin) != 0) return false;
    // 【逐行批注】 结束当前函数并返回结果：return url.size() == g.desktopOrigin.size() || url[g.desktopOrigin.si…。
    return url.size() == g.desktopOrigin.size() || url[g.desktopOrigin.size()] == L'/' ||
           // 【逐行批注】 执行当前声明或表达式：url[g.desktopOrigin.size()] == L'?' || url[g.desktopOrigin.size()] == L'#';。
           url[g.desktopOrigin.size()] == L'?' || url[g.desktopOrigin.size()] == L'#';
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 openExternalUrl，执行该函数声明的职责。
void openExternalUrl(const std::wstring& url) {
    // 【逐行批注】 判断条件 (isExternalWebUrl(url)) ShellExecuteW(nullptr, L"open", url.c_st…，成立时执行对应分支。
    if (isExternalWebUrl(url)) ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明数据结构或类型 NavigationHandler 及其成员接口。
class NavigationHandler final : public ICoreWebView2NavigationStartingEventHandler {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 调用 QueryInterface 完成本行所需操作：HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** result) ov…。
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** result) override {
        // 【逐行批注】 判断条件 (!result) return E_POINTER;，成立时执行对应分支。
        if (!result) return E_POINTER;
        *result = nullptr;
        // 【逐行批注】 判断条件 (IsEqualIID(id, IID_IUnknown) || IsEqualIID(id, IID_ICoreWebView…，成立时执行对应分支。
        if (IsEqualIID(id, IID_IUnknown) || IsEqualIID(id, IID_ICoreWebView2NavigationStartingEventHandler)) {
            *result = this;
            // 【逐行批注】 调用 AddRef 完成本行所需操作：AddRef();。
            AddRef();
            // 【逐行批注】 结束当前函数并返回结果：return S_OK;。
            return S_OK;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return E_NOINTERFACE;。
        return E_NOINTERFACE;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 AddRef 完成本行所需操作：ULONG STDMETHODCALLTYPE AddRef() override { return ++refs_; }。
    ULONG STDMETHODCALLTYPE AddRef() override { return ++refs_; }
    // 【逐行批注】 调用 Release 完成本行所需操作：ULONG STDMETHODCALLTYPE Release() override {。
    ULONG STDMETHODCALLTYPE Release() override {
        // 【逐行批注】 计算右侧结果并写入 ULONG remaining，更新当前运行状态。
        const ULONG remaining = --refs_;
        // 【逐行批注】 判断条件 (!remaining) delete this;，成立时执行对应分支。
        if (!remaining) delete this;
        // 【逐行批注】 结束当前函数并返回结果：return remaining;。
        return remaining;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 Invoke 完成本行所需操作：HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2*, ICoreWebView2Navigat…。
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) override {
        // 【逐行批注】 计算右侧结果并写入 LPWSTR raw，更新当前运行状态。
        LPWSTR raw = nullptr;
        // 【逐行批注】 判断条件 (FAILED(args->get_Uri(&raw)) || !raw) return S_OK;，成立时执行对应分支。
        if (FAILED(args->get_Uri(&raw)) || !raw) return S_OK;
        // 【逐行批注】 调用 url 完成本行所需操作：const std::wstring url(raw);。
        const std::wstring url(raw);
        // 【逐行批注】 调用 CoTaskMemFree 完成本行所需操作：CoTaskMemFree(raw);。
        CoTaskMemFree(raw);
        // 【逐行批注】 判断条件 (url == L"about:blank") return S_OK;，成立时执行对应分支。
        if (url == L"about:blank") return S_OK;
        // 【逐行批注】 判断条件 (isCurrentLocalUrl(url)) {，成立时执行对应分支。
        if (isCurrentLocalUrl(url)) {
            // 【逐行批注】 计算右侧结果并写入 g.desktopRoute，更新当前运行状态。
            g.desktopRoute = url.substr(g.desktopOrigin.size());
            // 【逐行批注】 判断条件 (g.desktopRoute.empty()) g.desktopRoute = L"/";，成立时执行对应分支。
            if (g.desktopRoute.empty()) g.desktopRoute = L"/";
            // 【逐行批注】 判断条件 (g.classroomFullscreen && g.desktopRoute.rfind(L"/deep-course", …，成立时执行对应分支。
            if (g.classroomFullscreen && g.desktopRoute.rfind(L"/deep-course", 0) != 0 && g.desktopWindow)
                // 【逐行批注】 调用 PostMessageW 完成本行所需操作：PostMessageW(g.desktopWindow, kClassroomFullscreen, 0, 0);。
                PostMessageW(g.desktopWindow, kClassroomFullscreen, 0, 0);
            // 【逐行批注】 计算右侧结果并写入 g.pageReady，更新当前运行状态。
            g.pageReady = false;
            // 【逐行批注】 判断条件 (g.desktopWindow) SetTimer(g.desktopWindow, kDesktopLoadTimer, 1…，成立时执行对应分支。
            if (g.desktopWindow) SetTimer(g.desktopWindow, kDesktopLoadTimer, 15000, nullptr);
        // 【逐行批注】 执行当前声明或表达式：} else {。
        } else {
            // 【逐行批注】 调用 put_Cancel 完成本行所需操作：args->put_Cancel(TRUE);。
            args->put_Cancel(TRUE);
            // 【逐行批注】 调用 openExternalUrl 完成本行所需操作：openExternalUrl(url);。
            openExternalUrl(url);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return S_OK;。
        return S_OK;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：std::atomic<ULONG> refs_{1};。
    std::atomic<ULONG> refs_{1};
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 NewWindowHandler 及其成员接口。
class NewWindowHandler final : public ICoreWebView2NewWindowRequestedEventHandler {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 调用 QueryInterface 完成本行所需操作：HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** result) ov…。
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** result) override {
        // 【逐行批注】 判断条件 (!result) return E_POINTER;，成立时执行对应分支。
        if (!result) return E_POINTER;
        *result = nullptr;
        // 【逐行批注】 判断条件 (IsEqualIID(id, IID_IUnknown) || IsEqualIID(id, IID_ICoreWebView…，成立时执行对应分支。
        if (IsEqualIID(id, IID_IUnknown) || IsEqualIID(id, IID_ICoreWebView2NewWindowRequestedEventHandler)) {
            *result = this;
            // 【逐行批注】 调用 AddRef 完成本行所需操作：AddRef();。
            AddRef();
            // 【逐行批注】 结束当前函数并返回结果：return S_OK;。
            return S_OK;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return E_NOINTERFACE;。
        return E_NOINTERFACE;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 AddRef 完成本行所需操作：ULONG STDMETHODCALLTYPE AddRef() override { return ++refs_; }。
    ULONG STDMETHODCALLTYPE AddRef() override { return ++refs_; }
    // 【逐行批注】 调用 Release 完成本行所需操作：ULONG STDMETHODCALLTYPE Release() override {。
    ULONG STDMETHODCALLTYPE Release() override {
        // 【逐行批注】 计算右侧结果并写入 ULONG remaining，更新当前运行状态。
        const ULONG remaining = --refs_;
        // 【逐行批注】 判断条件 (!remaining) delete this;，成立时执行对应分支。
        if (!remaining) delete this;
        // 【逐行批注】 结束当前函数并返回结果：return remaining;。
        return remaining;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 Invoke 完成本行所需操作：HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2*, ICoreWebView2NewWind…。
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* args) override {
        // 【逐行批注】 计算右侧结果并写入 LPWSTR raw，更新当前运行状态。
        LPWSTR raw = nullptr;
        // 【逐行批注】 判断条件 (SUCCEEDED(args->get_Uri(&raw)) && raw) {，成立时执行对应分支。
        if (SUCCEEDED(args->get_Uri(&raw)) && raw) {
            // 【逐行批注】 判断条件 (isCurrentLocalUrl(raw) && g.browser) g.browser->Navigate(raw);，成立时执行对应分支。
            if (isCurrentLocalUrl(raw) && g.browser) g.browser->Navigate(raw);
            // 【逐行批注】 执行前一条件不成立时的分支。
            else openExternalUrl(raw);
            // 【逐行批注】 调用 CoTaskMemFree 完成本行所需操作：CoTaskMemFree(raw);。
            CoTaskMemFree(raw);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 put_Handled 完成本行所需操作：args->put_Handled(TRUE);。
        args->put_Handled(TRUE);
        // 【逐行批注】 结束当前函数并返回结果：return S_OK;。
        return S_OK;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：std::atomic<ULONG> refs_{1};。
    std::atomic<ULONG> refs_{1};
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 resizeDesktopWidget，执行该函数声明的职责。
void resizeDesktopWidget(HWND window) {
    // 【逐行批注】 判断条件 (!g.desktopView) return;，成立时执行对应分支。
    if (!g.desktopView) return;
    // 【逐行批注】 计算右侧结果并写入 auto* widget，更新当前运行状态。
    auto* widget = static_cast<HWND>(webview_get_native_handle(g.desktopView, WEBVIEW_NATIVE_HANDLE_KIND_UI_WIDGET));
    // 【逐行批注】 执行当前声明或表达式：RECT bounds{};。
    RECT bounds{};
    // 【逐行批注】 判断条件 (widget && GetClientRect(window, &bounds))，成立时执行对应分支。
    if (widget && GetClientRect(window, &bounds))
        // 【逐行批注】 调用 MoveWindow 完成本行所需操作：MoveWindow(widget, 0, 0, bounds.right, bounds.bottom, TRUE);。
        MoveWindow(widget, 0, 0, bounds.right, bounds.bottom, TRUE);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 applyClassroomFullscreen，执行该函数声明的职责。
void applyClassroomFullscreen(HWND window, bool enabled) {
    // 【逐行批注】 判断条件 (!window || g.classroomFullscreen == enabled) return;，成立时执行对应分支。
    if (!window || g.classroomFullscreen == enabled) return;
    // 【逐行批注】 判断条件 (enabled) {，成立时执行对应分支。
    if (enabled) {
        // 【逐行批注】 计算右侧结果并写入 g.beforeClassroomFullscreen，更新当前运行状态。
        g.beforeClassroomFullscreen = {};
        // 【逐行批注】 计算右侧结果并写入 g.beforeClassroomFullscreen.length，更新当前运行状态。
        g.beforeClassroomFullscreen.length = sizeof(g.beforeClassroomFullscreen);
        // 【逐行批注】 判断条件 (!GetWindowPlacement(window, &g.beforeClassroomFullscreen)) retu…，成立时执行对应分支。
        if (!GetWindowPlacement(window, &g.beforeClassroomFullscreen)) return;
        // 【逐行批注】 计算右侧结果并写入 g.classroomFullscreenStyle，更新当前运行状态。
        g.classroomFullscreenStyle = GetWindowLongPtrW(window, GWL_STYLE);
        // 【逐行批注】 执行当前声明或表达式：MONITORINFO monitor{};。
        MONITORINFO monitor{};
        // 【逐行批注】 计算右侧结果并写入 monitor.cbSize，更新当前运行状态。
        monitor.cbSize = sizeof(monitor);
        // 【逐行批注】 计算右侧结果并写入 auto target，更新当前运行状态。
        const auto target = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
        // 【逐行批注】 判断条件 (!target || !GetMonitorInfoW(target, &monitor)) return;，成立时执行对应分支。
        if (!target || !GetMonitorInfoW(target, &monitor)) return;
        // 【逐行批注】 计算右侧结果并写入 auto style，更新当前运行状态。
        const auto style = (g.classroomFullscreenStyle &
            // 【逐行批注】 执行当前声明或表达式：~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU)) | WS_P…。
            ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU)) | WS_POPUP;
        // 【逐行批注】 调用 SetWindowLongPtrW 完成本行所需操作：SetWindowLongPtrW(window, GWL_STYLE, style);。
        SetWindowLongPtrW(window, GWL_STYLE, style);
        // 【逐行批注】 调用 SetWindowPos 完成本行所需操作：SetWindowPos(window, HWND_TOPMOST, monitor.rcMonitor.left, monitor.rc…。
        SetWindowPos(window, HWND_TOPMOST, monitor.rcMonitor.left, monitor.rcMonitor.top,
            // 【逐行批注】 执行当前声明或表达式：monitor.rcMonitor.right - monitor.rcMonitor.left,。
            monitor.rcMonitor.right - monitor.rcMonitor.left,
            // 【逐行批注】 执行当前声明或表达式：monitor.rcMonitor.bottom - monitor.rcMonitor.top,。
            monitor.rcMonitor.bottom - monitor.rcMonitor.top,
            // 【逐行批注】 执行当前声明或表达式：SWP_FRAMECHANGED | SWP_SHOWWINDOW);。
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        // 【逐行批注】 计算右侧结果并写入 g.classroomFullscreen，更新当前运行状态。
        g.classroomFullscreen = true;
        // 【逐行批注】 调用 SetForegroundWindow 完成本行所需操作：SetForegroundWindow(window);。
        SetForegroundWindow(window);
    // 【逐行批注】 执行当前声明或表达式：} else {。
    } else {
        // 【逐行批注】 调用 SetWindowLongPtrW 完成本行所需操作：SetWindowLongPtrW(window, GWL_STYLE, g.classroomFullscreenStyle);。
        SetWindowLongPtrW(window, GWL_STYLE, g.classroomFullscreenStyle);
        // 【逐行批注】 调用 SetWindowPos 完成本行所需操作：SetWindowPos(window, HWND_NOTOPMOST, 0, 0, 0, 0,。
        SetWindowPos(window, HWND_NOTOPMOST, 0, 0, 0, 0,
            // 【逐行批注】 执行当前声明或表达式：SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);。
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        // 【逐行批注】 判断条件 (g.beforeClassroomFullscreen.length)，成立时执行对应分支。
        if (g.beforeClassroomFullscreen.length)
            // 【逐行批注】 调用 SetWindowPlacement 完成本行所需操作：SetWindowPlacement(window, &g.beforeClassroomFullscreen);。
            SetWindowPlacement(window, &g.beforeClassroomFullscreen);
        // 【逐行批注】 计算右侧结果并写入 g.classroomFullscreen，更新当前运行状态。
        g.classroomFullscreen = false;
        // 【逐行批注】 调用 ShowWindow 完成本行所需操作：ShowWindow(window, SW_SHOW);。
        ShowWindow(window, SW_SHOW);
        // 【逐行批注】 调用 SetForegroundWindow 完成本行所需操作：SetForegroundWindow(window);。
        SetForegroundWindow(window);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 resizeDesktopWidget 完成本行所需操作：resizeDesktopWidget(window);。
    resizeDesktopWidget(window);
// 【逐行批注】 结束当前代码块。
}

// 只记录还原态位置与尺寸，窗口最大化或最小化关闭时下次仍能回到同样的大小。
// 【逐行批注】 定义函数 saveDesktopGeometry，执行该函数声明的职责。
void saveDesktopGeometry(HWND window) {
    // 【逐行批注】 判断条件 (g.smokeMode || !window) return;，成立时执行对应分支。
    if (g.smokeMode || !window) return;
    // 【逐行批注】 执行当前声明或表达式：WINDOWPLACEMENT placement{};。
    WINDOWPLACEMENT placement{};
    // 【逐行批注】 计算右侧结果并写入 placement.length，更新当前运行状态。
    placement.length = sizeof(placement);
    // 【逐行批注】 判断条件 (g.classroomFullscreen && g.beforeClassroomFullscreen.length)，成立时执行对应分支。
    if (g.classroomFullscreen && g.beforeClassroomFullscreen.length)
        // 【逐行批注】 计算右侧结果并写入 placement，更新当前运行状态。
        placement = g.beforeClassroomFullscreen;
    // 【逐行批注】 在前一条件不成立时继续判断：(!GetWindowPlacement(window, &placement)) return;。
    else if (!GetWindowPlacement(window, &placement)) return;
    // 【逐行批注】 计算右侧结果并写入 RECT normal，更新当前运行状态。
    const RECT normal = placement.rcNormalPosition;
    // 【逐行批注】 计算右侧结果并写入 int width，更新当前运行状态。
    const int width = normal.right - normal.left;
    // 【逐行批注】 计算右侧结果并写入 int height，更新当前运行状态。
    const int height = normal.bottom - normal.top;
    // 【逐行批注】 判断条件 (width <= 0 || height <= 0) return;，成立时执行对应分支。
    if (width <= 0 || height <= 0) return;
    // 【逐行批注】 调用 writeRegistryDword 完成本行所需操作：writeRegistryDword(L"WindowX", static_cast<DWORD>(normal.left));。
    writeRegistryDword(L"WindowX", static_cast<DWORD>(normal.left));
    // 【逐行批注】 调用 writeRegistryDword 完成本行所需操作：writeRegistryDword(L"WindowY", static_cast<DWORD>(normal.top));。
    writeRegistryDword(L"WindowY", static_cast<DWORD>(normal.top));
    // 【逐行批注】 调用 writeRegistryDword 完成本行所需操作：writeRegistryDword(L"WindowWidth", static_cast<DWORD>(width));。
    writeRegistryDword(L"WindowWidth", static_cast<DWORD>(width));
    // 【逐行批注】 调用 writeRegistryDword 完成本行所需操作：writeRegistryDword(L"WindowHeight", static_cast<DWORD>(height));。
    writeRegistryDword(L"WindowHeight", static_cast<DWORD>(height));
    // 【逐行批注】 调用 writeRegistryDword 完成本行所需操作：writeRegistryDword(L"WindowMaximized", placement.showCmd == SW_SHOWMA…。
    writeRegistryDword(L"WindowMaximized", placement.showCmd == SW_SHOWMAXIMIZED ? 1u : 0u);
// 【逐行批注】 结束当前代码块。
}

// 读取上次保存的几何；落在已移除的显示器外时按无效处理，由调用方回退默认尺寸。
// 保存值可见时以它所在显示器的工作区为界，多屏下不会把窗口拉回主屏。
// 【逐行批注】 定义函数 desktopWindowPlacement，执行该函数声明的职责。
gangyi::launcher::WindowPlacement desktopWindowPlacement(const RECT& primaryWorkArea) {
    // 【逐行批注】 执行当前声明或表达式：gangyi::launcher::WindowRect area{primaryWorkArea.left, primaryWorkArea.top,。
    gangyi::launcher::WindowRect area{primaryWorkArea.left, primaryWorkArea.top,
        // 【逐行批注】 执行当前声明或表达式：primaryWorkArea.right - primaryWorkArea.left, primaryWorkArea.bottom - primaryWorkAr…。
        primaryWorkArea.right - primaryWorkArea.left, primaryWorkArea.bottom - primaryWorkArea.top};
    // 【逐行批注】 计算右侧结果并写入 bool hasSaved，更新当前运行状态。
    bool hasSaved = false, visible = false, savedMaximized = false;
    // 【逐行批注】 执行当前声明或表达式：gangyi::launcher::WindowRect saved{};。
    gangyi::launcher::WindowRect saved{};
    // 【逐行批注】 判断条件 (!g.smokeMode) {，成立时执行对应分支。
    if (!g.smokeMode) {
        // 【逐行批注】 计算右侧结果并写入 DWORD width，更新当前运行状态。
        const DWORD width = readRegistryDword(L"WindowWidth", 0);
        // 【逐行批注】 计算右侧结果并写入 DWORD height，更新当前运行状态。
        const DWORD height = readRegistryDword(L"WindowHeight", 0);
        // 【逐行批注】 判断条件 (width > 0 && height > 0) {，成立时执行对应分支。
        if (width > 0 && height > 0) {
            // 【逐行批注】 计算右侧结果并写入 saved，更新当前运行状态。
            saved = {static_cast<int>(readRegistryDword(L"WindowX", 0)),
                     // 【逐行批注】 调用 readRegistryDword 完成本行所需操作：static_cast<int>(readRegistryDword(L"WindowY", 0)),。
                     static_cast<int>(readRegistryDword(L"WindowY", 0)),
                     // 【逐行批注】 执行当前声明或表达式：static_cast<int>(width), static_cast<int>(height)};。
                     static_cast<int>(width), static_cast<int>(height)};
            // 【逐行批注】 执行当前声明或表达式：const RECT candidate{saved.x, saved.y, saved.x + saved.width, saved.y + saved.height…。
            const RECT candidate{saved.x, saved.y, saved.x + saved.width, saved.y + saved.height};
            // 【逐行批注】 执行当前声明或表达式：MONITORINFO info{};。
            MONITORINFO info{};
            // 【逐行批注】 计算右侧结果并写入 info.cbSize，更新当前运行状态。
            info.cbSize = sizeof(info);
            // 【逐行批注】 计算右侧结果并写入 HMONITOR monitor，更新当前运行状态。
            const HMONITOR monitor = MonitorFromRect(&candidate, MONITOR_DEFAULTTONULL);
            // 【逐行批注】 计算右侧结果并写入 visible，更新当前运行状态。
            visible = monitor && GetMonitorInfoW(monitor, &info);
            // 【逐行批注】 判断条件 (visible)，成立时执行对应分支。
            if (visible)
                // 【逐行批注】 计算右侧结果并写入 area，更新当前运行状态。
                area = {info.rcWork.left, info.rcWork.top, info.rcWork.right - info.rcWork.left,
                        // 【逐行批注】 执行当前声明或表达式：info.rcWork.bottom - info.rcWork.top};。
                        info.rcWork.bottom - info.rcWork.top};
            // 【逐行批注】 计算右侧结果并写入 hasSaved，更新当前运行状态。
            hasSaved = true;
            // 【逐行批注】 计算右侧结果并写入 savedMaximized，更新当前运行状态。
            savedMaximized = readRegistryDword(L"WindowMaximized", 0) != 0;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return gangyi::launcher::computeWindowPlacement(area, hasSaved, saved…。
    return gangyi::launcher::computeWindowPlacement(area, hasSaved, saved, visible, savedMaximized);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 desktopPageReady，执行该函数声明的职责。
void desktopPageReady(const char* id, const char*, void*) {
    // 【逐行批注】 计算右侧结果并写入 g.pageReady，更新当前运行状态。
    g.pageReady = true;
    // 【逐行批注】 判断条件 (g.desktopWindow) KillTimer(g.desktopWindow, kDesktopLoadTimer);，成立时执行对应分支。
    if (g.desktopWindow) KillTimer(g.desktopWindow, kDesktopLoadTimer);
    // 【逐行批注】 调用 webview_return 完成本行所需操作：webview_return(g.desktopView, id, 0, "null");。
    webview_return(g.desktopView, id, 0, "null");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 myAILocalOrigin，执行该函数声明的职责。
bool myAILocalOrigin() {
    // 【逐行批注】 执行当前声明或表达式：if(!g.desktopView||g.desktopOrigin.empty())return false;。
    if(!g.desktopView||g.desktopOrigin.empty())return false;
    // 【逐行批注】 计算右侧结果并写入 auto* controller，更新当前运行状态。
    auto* controller=static_cast<ICoreWebView2Controller*>(webview_get_native_handle(g.desktopView,WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER));
    // 【逐行批注】 调用 FAILED、get_CoreWebView2 完成本行所需操作：if(!controller)return false;ICoreWebView2* browser=nullptr;if(FAILED(…。
    if(!controller)return false;ICoreWebView2* browser=nullptr;if(FAILED(controller->get_CoreWebView2(&browser))||!browser)return false;
    // 【逐行批注】 计算右侧结果并写入 LPWSTR source，更新当前运行状态。
    LPWSTR source=nullptr;const auto result=browser->get_Source(&source);browser->Release();const std::wstring uri=source?source:L"";CoTaskMemFree(source);
    // 【逐行批注】 计算右侧结果并写入 auto expected，更新当前运行状态。
    const auto expected=g.desktopOrigin+L"/";return SUCCEEDED(result)&&uri.compare(0,expected.size(),expected)==0;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 执行当前声明或表达式：std::wstring myAIProposedModel;。
std::wstring myAIProposedModel;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kMyAIBackup，更新当前运行状态。
constexpr UINT kMyAIBackup = WM_APP + 61;
// 【逐行批注】 计算右侧结果并写入 constexpr UINT kMyAIRestore，更新当前运行状态。
constexpr UINT kMyAIRestore = WM_APP + 62;
// 【逐行批注】 定义函数 myAIBackupBridge，执行该函数声明的职责。
void myAIBackupBridge(const char* id,const char*,void*) {
    // 【逐行批注】 调用 myAILocalOrigin、webview_return 完成本行所需操作：if(!myAILocalOrigin()){webview_return(g.desktopView,id,1,"\"请在钢一定制AI的…。
    if(!myAILocalOrigin()){webview_return(g.desktopView,id,1,"\"请在钢一定制AI的本机页面操作\"");return;}
    // 【逐行批注】 调用 PostMessageW、webview_return 完成本行所需操作：if(!g.window || !PostMessageW(g.window,kMyAIBackup,0,0)) {webview_ret…。
    if(!g.window || !PostMessageW(g.window,kMyAIBackup,0,0)) {webview_return(g.desktopView,id,1,"\"无法打开备份窗口\"");return;}
    // 【逐行批注】 调用 webview_return 完成本行所需操作：webview_return(g.desktopView,id,0,"null");。
    webview_return(g.desktopView,id,0,"null");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 myAIRestoreBridge，执行该函数声明的职责。
void myAIRestoreBridge(const char* id,const char*,void*) {
    // 【逐行批注】 调用 myAILocalOrigin、webview_return 完成本行所需操作：if(!myAILocalOrigin()){webview_return(g.desktopView,id,1,"\"请在钢一定制AI的…。
    if(!myAILocalOrigin()){webview_return(g.desktopView,id,1,"\"请在钢一定制AI的本机页面操作\"");return;}
    // 【逐行批注】 调用 PostMessageW、webview_return 完成本行所需操作：if(!g.window || !PostMessageW(g.window,kMyAIRestore,0,0)) {webview_re…。
    if(!g.window || !PostMessageW(g.window,kMyAIRestore,0,0)) {webview_return(g.desktopView,id,1,"\"无法打开恢复窗口\"");return;}
    // 【逐行批注】 调用 webview_return 完成本行所需操作：webview_return(g.desktopView,id,0,"null");。
    webview_return(g.desktopView,id,0,"null");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 myAIDesktopInfo，执行该函数声明的职责。
void myAIDesktopInfo(const char* id,const char*,void*) {
    // 【逐行批注】 调用 myAILocalOrigin、webview_return 完成本行所需操作：if(!myAILocalOrigin()){webview_return(g.desktopView,id,1,"\"请在钢一定制AI的…。
    if(!myAILocalOrigin()){webview_return(g.desktopView,id,1,"\"请在钢一定制AI的本机页面操作\"");return;}
    // 【逐行批注】 计算右侧结果并写入 auto settings，更新当前运行状态。
    const auto settings=g.smokeMode?g.settings:loadSettings();nlohmann::json value={{"provider",toUtf8(gangyi::launcher::providerProfile(settings.provider).name)},{"baseUrl",toUtf8(settings.baseUrl)},{"model",toUtf8(settings.model)},{"configured",!g.smokeMode&&!readApiKey().empty()},{"autoStart",settings.autoStart}};
    // 【逐行批注】 调用 webview_return 完成本行所需操作：webview_return(g.desktopView,id,0,value.dump().c_str());。
    webview_return(g.desktopView,id,0,value.dump().c_str());
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 classroomFullscreenBridge，执行该函数声明的职责。
void classroomFullscreenBridge(const char* id, const char* request, void*) {
    // 【逐行批注】 判断条件 (!myAILocalOrigin() || !g.desktopWindow) {，成立时执行对应分支。
    if (!myAILocalOrigin() || !g.desktopWindow) {
        // 【逐行批注】 调用 webview_return 完成本行所需操作：webview_return(g.desktopView, id, 1, "\"只能从本机深度课堂切换系统全屏\"");。
        webview_return(g.desktopView, id, 1, "\"只能从本机深度课堂切换系统全屏\"");
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto arguments，更新当前运行状态。
        const auto arguments = nlohmann::json::parse(request ? request : "[]");
        // 【逐行批注】 计算右侧结果并写入 bool enabled，更新当前运行状态。
        const bool enabled = arguments.is_array() && !arguments.empty() &&
            // 【逐行批注】 执行当前声明或表达式：arguments.at(0).is_boolean() ? arguments.at(0).get<bool>() : false;。
            arguments.at(0).is_boolean() ? arguments.at(0).get<bool>() : false;
        // 【逐行批注】 判断条件 (!PostMessageW(g.desktopWindow, kClassroomFullscreen, enabled ? …，成立时执行对应分支。
        if (!PostMessageW(g.desktopWindow, kClassroomFullscreen, enabled ? 1 : 0, 0))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("无法切换系统全屏");。
            throw std::runtime_error("无法切换系统全屏");
        // 【逐行批注】 调用 webview_return 完成本行所需操作：webview_return(g.desktopView, id, 0, enabled ? "true" : "false");。
        webview_return(g.desktopView, id, 0, enabled ? "true" : "false");
    // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception& error) {。
    } catch (const std::exception& error) {
        // 【逐行批注】 调用 webview_return、json 完成本行所需操作：webview_return(g.desktopView, id, 1, nlohmann::json(error.what()).dum…。
        webview_return(g.desktopView, id, 1, nlohmann::json(error.what()).dump().c_str());
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 openApiSettings，执行该函数声明的职责。
void openApiSettings(const char* id, const char* request, void*) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto args，更新当前运行状态。
        const auto args=nlohmann::json::parse(request?request:"[]");
        // 【逐行批注】 执行当前声明或表达式：if(args.is_array()&&!args.empty()&&args[0].is_object()&&args[0].contains("model")) {。
        if(args.is_array()&&!args.empty()&&args[0].is_object()&&args[0].contains("model")) {
            // 【逐行批注】 调用 myAILocalOrigin、runtime_error 完成本行所需操作：if(!myAILocalOrigin())throw std::runtime_error("请在本机页面检查设置建议");。
            if(!myAILocalOrigin())throw std::runtime_error("请在本机页面检查设置建议");
            // 【逐行批注】 计算右侧结果并写入 auto model，更新当前运行状态。
            const auto model=args[0].at("model").get<std::string>(),before=args[0].value("before","");
            // 【逐行批注】 调用 runtime_error 完成本行所需操作：if(model.empty()||model.size()>200||model.find_first_of("\r\n")!=std:…。
            if(model.empty()||model.size()>200||model.find_first_of("\r\n")!=std::string::npos||model.find('\0')!=std::string::npos)throw std::runtime_error("模型名称无效");
            // 【逐行批注】 调用 toUtf8、loadSettings、runtime_error 完成本行所需操作：if(!before.empty()&&before!=toUtf8(loadSettings().model))throw std::r…。
            if(!before.empty()&&before!=toUtf8(loadSettings().model))throw std::runtime_error("当前模型已变化，请重新检查设置建议");
            // 【逐行批注】 计算右侧结果并写入 myAIProposedModel，更新当前运行状态。
            myAIProposedModel=toWide(model);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 调用 webview_return、json 完成本行所需操作：}catch(const std::exception& error){webview_return(g.desktopView,id,1…。
    }catch(const std::exception& error){webview_return(g.desktopView,id,1,nlohmann::json(error.what()).dump().c_str());return;}

    // 【逐行批注】 判断条件 (!g.window || !PostMessageW(g.window, kOpenApiSettings, 0, 0)) {，成立时执行对应分支。
    if (!g.window || !PostMessageW(g.window, kOpenApiSettings, 0, 0)) {
        // 【逐行批注】 调用 webview_return 完成本行所需操作：webview_return(g.desktopView, id, 1, "\"无法打开 API 接口设置，请从托盘菜单进入设置。\"");。
        webview_return(g.desktopView, id, 1, "\"无法打开 API 接口设置，请从托盘菜单进入设置。\"");
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 webview_return 完成本行所需操作：webview_return(g.desktopView, id, 0, "null");。
    webview_return(g.desktopView, id, 0, "null");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 计算右侧结果并写入 char* kDesktopScript，更新当前运行状态。
const char* kDesktopScript = R"JS((() => {
  // 应用页面骨架：服务端页面用 .site-header，独立页面（备课/课堂）用 body > header 与 main。
  // WebView2 自带的连接失败页不含这些元素，仍会被判定为未就绪。
  const landmark = () => document.querySelector('.site-header, body > header, main');
  const report = () => {
    const node = landmark();
    if (node && node.getBoundingClientRect().width > 0 &&
        document.documentElement.clientWidth > 0 && document.body.innerText.trim()) {
      window.gangyiPageReady();
    } else {
      setTimeout(report, 200);
    }
  };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', report, {once: true});
  else report();
})())JS";

// 【逐行批注】 定义函数 closeDesktopView，执行该函数声明的职责。
void closeDesktopView() {
    // 【逐行批注】 判断条件 (g.desktopWindow) KillTimer(g.desktopWindow, kDesktopLoadTimer);，成立时执行对应分支。
    if (g.desktopWindow) KillTimer(g.desktopWindow, kDesktopLoadTimer);
    // 【逐行批注】 判断条件 (g.browser) {，成立时执行对应分支。
    if (g.browser) {
        // 【逐行批注】 调用 remove_NavigationStarting 完成本行所需操作：g.browser->remove_NavigationStarting(g.navigationToken);。
        g.browser->remove_NavigationStarting(g.navigationToken);
        // 【逐行批注】 调用 remove_NewWindowRequested 完成本行所需操作：g.browser->remove_NewWindowRequested(g.newWindowToken);。
        g.browser->remove_NewWindowRequested(g.newWindowToken);
        // 【逐行批注】 调用 Release 完成本行所需操作：g.browser->Release();。
        g.browser->Release();
        // 【逐行批注】 计算右侧结果并写入 g.browser，更新当前运行状态。
        g.browser = nullptr;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (g.desktopView) {，成立时执行对应分支。
    if (g.desktopView) {
        // 【逐行批注】 调用 webview_destroy 完成本行所需操作：webview_destroy(g.desktopView);。
        webview_destroy(g.desktopView);
        // 【逐行批注】 计算右侧结果并写入 g.desktopView，更新当前运行状态。
        g.desktopView = nullptr;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 desktopWindowProc，执行该函数声明的职责。
LRESULT CALLBACK desktopWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    // 【逐行批注】 根据表达式分支处理：switch (message) {。
    switch (message) {
    // 【逐行批注】 声明 switch 的分支入口：case WM_GETMINMAXINFO: {。
    case WM_GETMINMAXINFO: {
        // 保证课堂与备课界面不会被拉伸到无法使用的尺寸。
        // 【逐行批注】 计算右侧结果并写入 auto* info，更新当前运行状态。
        auto* info = reinterpret_cast<MINMAXINFO*>(lParam);
        // 【逐行批注】 判断条件 (info) {，成立时执行对应分支。
        if (info) {
            // 【逐行批注】 执行当前声明或表达式：info->ptMinTrackSize.x = 1024;。
            info->ptMinTrackSize.x = 1024;
            // 【逐行批注】 执行当前声明或表达式：info->ptMinTrackSize.y = 640;。
            info->ptMinTrackSize.y = 640;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 声明 switch 的分支入口：case WM_EXITSIZEMOVE:。
    case WM_EXITSIZEMOVE:
        // 【逐行批注】 调用 saveDesktopGeometry 完成本行所需操作：saveDesktopGeometry(window);。
        saveDesktopGeometry(window);
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case WM_SIZE:。
    case WM_SIZE:
        // 【逐行批注】 调用 resizeDesktopWidget 完成本行所需操作：resizeDesktopWidget(window);。
        resizeDesktopWidget(window);
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case WM_MOVE:。
    case WM_MOVE:
        // 【逐行批注】 判断条件 (g.desktopView) {，成立时执行对应分支。
        if (g.desktopView) {
            // 【逐行批注】 计算右侧结果并写入 auto* controller，更新当前运行状态。
            auto* controller = static_cast<ICoreWebView2Controller*>(webview_get_native_handle(
                // 【逐行批注】 执行当前声明或表达式：g.desktopView, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER));。
                g.desktopView, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER));
            // 【逐行批注】 判断条件 (controller) controller->NotifyParentWindowPositionChanged();，成立时执行对应分支。
            if (controller) controller->NotifyParentWindowPositionChanged();
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case WM_DPICHANGED: {。
    case WM_DPICHANGED: {
        // 【逐行批注】 计算右侧结果并写入 auto* suggested，更新当前运行状态。
        const auto* suggested = reinterpret_cast<const RECT*>(lParam);
        // 【逐行批注】 判断条件 (suggested) SetWindowPos(window, nullptr, suggested->left, sugge…，成立时执行对应分支。
        if (suggested) SetWindowPos(window, nullptr, suggested->left, suggested->top,
            // 【逐行批注】 执行当前声明或表达式：suggested->right - suggested->left, suggested->bottom - suggested->top,。
            suggested->right - suggested->left, suggested->bottom - suggested->top,
            // 【逐行批注】 执行当前声明或表达式：SWP_NOZORDER | SWP_NOACTIVATE);。
            SWP_NOZORDER | SWP_NOACTIVATE);
        // 【逐行批注】 调用 resizeDesktopWidget 完成本行所需操作：resizeDesktopWidget(window);。
        resizeDesktopWidget(window);
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 声明 switch 的分支入口：case WM_TIMER:。
    case WM_TIMER:
        // 【逐行批注】 判断条件 (wParam == kDesktopLoadTimer && !g.pageReady) {，成立时执行对应分支。
        if (wParam == kDesktopLoadTimer && !g.pageReady) {
            // 【逐行批注】 调用 KillTimer 完成本行所需操作：KillTimer(window, kDesktopLoadTimer);。
            KillTimer(window, kDesktopLoadTimer);
            // 【逐行批注】 计算右侧结果并写入 int answer，更新当前运行状态。
            const int answer = MessageBoxW(window,
                // 【逐行批注】 执行当前声明或表达式：L"页面未能正常显示。请检查本地服务，然后重试；也可以退出程序。",。
                L"页面未能正常显示。请检查本地服务，然后重试；也可以退出程序。",
                // 【逐行批注】 执行当前声明或表达式：L"钢一定制AI - 加载失败", MB_RETRYCANCEL | MB_ICONWARNING);。
                L"钢一定制AI - 加载失败", MB_RETRYCANCEL | MB_ICONWARNING);
            // 【逐行批注】 判断条件 (answer == IDRETRY) {，成立时执行对应分支。
            if (answer == IDRETRY) {
                // 【逐行批注】 调用 SetTimer 完成本行所需操作：SetTimer(window, kDesktopLoadTimer, 15000, nullptr);。
                SetTimer(window, kDesktopLoadTimer, 15000, nullptr);
                // 【逐行批注】 调用 webview_navigate、toUtf8 完成本行所需操作：webview_navigate(g.desktopView, toUtf8(g.desktopOrigin + g.desktopRou…。
                webview_navigate(g.desktopView, toUtf8(g.desktopOrigin + g.desktopRoute).c_str());
            // 【逐行批注】 调用 PostMessageW 完成本行所需操作：} else PostMessageW(window, WM_CLOSE, 0, 0);。
            } else PostMessageW(window, WM_CLOSE, 0, 0);
            // 【逐行批注】 结束当前函数并返回结果：return 0;。
            return 0;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 跳出当前循环或分支。
        break;
    // 【逐行批注】 声明 switch 的分支入口：case kClassroomFullscreen:。
    case kClassroomFullscreen:
        // 【逐行批注】 调用 applyClassroomFullscreen 完成本行所需操作：applyClassroomFullscreen(window, wParam != 0);。
        applyClassroomFullscreen(window, wParam != 0);
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case WM_CLOSE:。
    case WM_CLOSE:
        // 【逐行批注】 调用 closeDesktopView 完成本行所需操作：closeDesktopView();。
        closeDesktopView();
        // 【逐行批注】 调用 DestroyWindow 完成本行所需操作：DestroyWindow(window);。
        DestroyWindow(window);
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case WM_DESTROY:。
    case WM_DESTROY:
        // 【逐行批注】 调用 saveDesktopGeometry 完成本行所需操作：saveDesktopGeometry(window);。
        saveDesktopGeometry(window);
        // 【逐行批注】 计算右侧结果并写入 g.desktopWindow，更新当前运行状态。
        g.desktopWindow = nullptr;
        // 【逐行批注】 计算右侧结果并写入 g.displayPort，更新当前运行状态。
        g.displayPort = 0;
        // 【逐行批注】 判断条件 (g.window && !g.shuttingDown) DestroyWindow(g.window);，成立时执行对应分支。
        if (g.window && !g.shuttingDown) DestroyWindow(g.window);
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return DefWindowProcW(window, message, wParam, lParam);。
    return DefWindowProcW(window, message, wParam, lParam);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 openDesktop，执行该函数声明的职责。
void openDesktop() {
    // 【逐行批注】 判断条件 (!g.port) return;，成立时执行对应分支。
    if (!g.port) return;
    // 【逐行批注】 判断条件 (!g.desktopWindow) {，成立时执行对应分支。
    if (!g.desktopWindow) {
        // 【逐行批注】 执行当前声明或表达式：RECT workArea{};。
        RECT workArea{};
        // 【逐行批注】 判断条件 (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0))，成立时执行对应分支。
        if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0))
            // 【逐行批注】 调用 SetRect、GetSystemMetrics 完成本行所需操作：SetRect(&workArea, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetr…。
            SetRect(&workArea, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        // 【逐行批注】 计算右侧结果并写入 auto placement，更新当前运行状态。
        const auto placement = desktopWindowPlacement(workArea);
        // 【逐行批注】 计算右侧结果并写入 g.desktopWindow，更新当前运行状态。
        g.desktopWindow = CreateWindowExW(0, kDesktopWindowClass, L"钢一定制AI", WS_OVERLAPPEDWINDOW,
            // 【逐行批注】 执行当前声明或表达式：placement.x, placement.y, placement.width, placement.height,。
            placement.x, placement.y, placement.width, placement.height,
            // 【逐行批注】 调用 GetModuleHandleW 完成本行所需操作：nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);。
            nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        // 【逐行批注】 判断条件 (!g.desktopWindow) { showFailure(L"无法创建桌面主窗口。"); return; }，成立时执行对应分支。
        if (!g.desktopWindow) { showFailure(L"无法创建桌面主窗口。"); return; }
        // 【逐行批注】 计算右侧结果并写入 g.desktopView，更新当前运行状态。
        g.desktopView = webview_create(0, g.desktopWindow);
        // 【逐行批注】 判断条件 (!g.desktopView) {，成立时执行对应分支。
        if (!g.desktopView) {
            // 【逐行批注】 计算右侧结果并写入 int answer，更新当前运行状态。
            const int answer = MessageBoxW(g.desktopWindow,
                // 【逐行批注】 执行当前声明或表达式：L"无法启动独立界面。请安装 Microsoft Edge WebView2 Runtime。是否打开官方下载页？",。
                L"无法启动独立界面。请安装 Microsoft Edge WebView2 Runtime。是否打开官方下载页？",
                // 【逐行批注】 执行当前声明或表达式：L"钢一定制AI", MB_YESNO | MB_ICONERROR);。
                L"钢一定制AI", MB_YESNO | MB_ICONERROR);
            // 【逐行批注】 判断条件 (answer == IDYES) ShellExecuteW(nullptr, L"open",，成立时执行对应分支。
            if (answer == IDYES) ShellExecuteW(nullptr, L"open",
                // 【逐行批注】 执行当前声明或表达式：L"https://developer.microsoft.com/microsoft-edge/webview2/", nullptr, nullptr, SW_SH…。
                L"https://developer.microsoft.com/microsoft-edge/webview2/", nullptr, nullptr, SW_SHOWNORMAL);
            // 【逐行批注】 调用 DestroyWindow 完成本行所需操作：DestroyWindow(g.desktopWindow);。
            DestroyWindow(g.desktopWindow);
            // 【逐行批注】 调用 DestroyWindow 完成本行所需操作：DestroyWindow(g.window);。
            DestroyWindow(g.window);
            // 【逐行批注】 结束当前函数并返回结果：return;。
            return;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 webview_bind 完成本行所需操作：webview_bind(g.desktopView, "gangyiPageReady", desktopPageReady, null…。
        webview_bind(g.desktopView, "gangyiPageReady", desktopPageReady, nullptr);
        // 【逐行批注】 调用 webview_bind 完成本行所需操作：webview_bind(g.desktopView, "gangyiOpenApiSettings", openApiSettings,…。
        webview_bind(g.desktopView, "gangyiOpenApiSettings", openApiSettings, nullptr);
        // 【逐行批注】 调用 webview_bind 完成本行所需操作：webview_bind(g.desktopView,"gangyiDesktopInfo",myAIDesktopInfo,nullpt…。
        webview_bind(g.desktopView,"gangyiDesktopInfo",myAIDesktopInfo,nullptr);
        // 【逐行批注】 调用 webview_bind 完成本行所需操作：webview_bind(g.desktopView,"gangyiBackupData",myAIBackupBridge,nullpt…。
        webview_bind(g.desktopView,"gangyiBackupData",myAIBackupBridge,nullptr);
        // 【逐行批注】 调用 webview_bind 完成本行所需操作：webview_bind(g.desktopView,"gangyiRestoreData",myAIRestoreBridge,null…。
        webview_bind(g.desktopView,"gangyiRestoreData",myAIRestoreBridge,nullptr);
        // 【逐行批注】 调用 webview_bind 完成本行所需操作：webview_bind(g.desktopView,"gangyiSetClassroomFullscreen",classroomFu…。
        webview_bind(g.desktopView,"gangyiSetClassroomFullscreen",classroomFullscreenBridge,nullptr);
        // 【逐行批注】 调用 webview_init 完成本行所需操作：webview_init(g.desktopView, kDesktopScript);。
        webview_init(g.desktopView, kDesktopScript);
        // 【逐行批注】 计算右侧结果并写入 auto* controller，更新当前运行状态。
        auto* controller = static_cast<ICoreWebView2Controller*>(webview_get_native_handle(
            // 【逐行批注】 执行当前声明或表达式：g.desktopView, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER));。
            g.desktopView, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER));
        // 【逐行批注】 判断条件 (!controller || FAILED(controller->get_CoreWebView2(&g.browser))…，成立时执行对应分支。
        if (!controller || FAILED(controller->get_CoreWebView2(&g.browser)) || !g.browser) {
            // 【逐行批注】 调用 showFailure 完成本行所需操作：showFailure(L"无法初始化桌面页面。");。
            showFailure(L"无法初始化桌面页面。");
            // 【逐行批注】 调用 closeDesktopView 完成本行所需操作：closeDesktopView();。
            closeDesktopView();
            // 【逐行批注】 调用 DestroyWindow 完成本行所需操作：DestroyWindow(g.desktopWindow);。
            DestroyWindow(g.desktopWindow);
            // 【逐行批注】 调用 DestroyWindow 完成本行所需操作：DestroyWindow(g.window);。
            DestroyWindow(g.window);
            // 【逐行批注】 结束当前函数并返回结果：return;。
            return;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 auto* navigation，更新当前运行状态。
        auto* navigation = new NavigationHandler();
        // 【逐行批注】 调用 add_NavigationStarting 完成本行所需操作：g.browser->add_NavigationStarting(navigation, &g.navigationToken);。
        g.browser->add_NavigationStarting(navigation, &g.navigationToken);
        // 【逐行批注】 调用 Release 完成本行所需操作：navigation->Release();。
        navigation->Release();
        // 【逐行批注】 计算右侧结果并写入 auto* newWindow，更新当前运行状态。
        auto* newWindow = new NewWindowHandler();
        // 【逐行批注】 调用 add_NewWindowRequested 完成本行所需操作：g.browser->add_NewWindowRequested(newWindow, &g.newWindowToken);。
        g.browser->add_NewWindowRequested(newWindow, &g.newWindowToken);
        // 【逐行批注】 调用 Release 完成本行所需操作：newWindow->Release();。
        newWindow->Release();
        // 【逐行批注】 调用 resizeDesktopWidget 完成本行所需操作：resizeDesktopWidget(g.desktopWindow);。
        resizeDesktopWidget(g.desktopWindow);
        // 上次最大化关闭时沿用最大化；还原态尺寸已在创建时确定。
        // 【逐行批注】 调用 ShowWindow 完成本行所需操作：ShowWindow(g.desktopWindow, placement.maximized ? SW_SHOWMAXIMIZED : …。
        ShowWindow(g.desktopWindow, placement.maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (g.displayPort != g.port) {，成立时执行对应分支。
    if (g.displayPort != g.port) {
        // 【逐行批注】 判断条件 (g.browser && !g.desktopOrigin.empty()) {，成立时执行对应分支。
        if (g.browser && !g.desktopOrigin.empty()) {
            // 【逐行批注】 计算右侧结果并写入 LPWSTR current，更新当前运行状态。
            LPWSTR current = nullptr;
            // 【逐行批注】 判断条件 (SUCCEEDED(g.browser->get_Source(&current)) && current) {，成立时执行对应分支。
            if (SUCCEEDED(g.browser->get_Source(&current)) && current) {
                // 【逐行批注】 调用 source 完成本行所需操作：const std::wstring source(current);。
                const std::wstring source(current);
                // 【逐行批注】 判断条件 (isCurrentLocalUrl(source)) g.desktopRoute = source.substr(g.des…，成立时执行对应分支。
                if (isCurrentLocalUrl(source)) g.desktopRoute = source.substr(g.desktopOrigin.size());
                // 【逐行批注】 调用 CoTaskMemFree 完成本行所需操作：CoTaskMemFree(current);。
                CoTaskMemFree(current);
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 g.desktopOrigin，更新当前运行状态。
        g.desktopOrigin = L"http://127.0.0.1:" + std::to_wstring(g.port);
        // 【逐行批注】 调用 configureUsage、path 完成本行所需操作：gangyi::AIActivity::configureUsage((std::filesystem::path(g.dataDir)/…。
        gangyi::AIActivity::configureUsage((std::filesystem::path(g.dataDir)/L"data"/L"gangyiAI.db").u8string());
        // 【逐行批注】 计算右侧结果并写入 g.displayPort，更新当前运行状态。
        g.displayPort = g.port;
        // 【逐行批注】 判断条件 (g.desktopRoute.empty() || g.desktopRoute.front() != L'/') g.des…，成立时执行对应分支。
        if (g.desktopRoute.empty() || g.desktopRoute.front() != L'/') g.desktopRoute = L"/";
        // 【逐行批注】 计算右侧结果并写入 g.pageReady，更新当前运行状态。
        g.pageReady = false;
        // 【逐行批注】 调用 SetTimer 完成本行所需操作：SetTimer(g.desktopWindow, kDesktopLoadTimer, 15000, nullptr);。
        SetTimer(g.desktopWindow, kDesktopLoadTimer, 15000, nullptr);
        // 【逐行批注】 调用 webview_navigate、toUtf8 完成本行所需操作：webview_navigate(g.desktopView, toUtf8(g.desktopOrigin + g.desktopRou…。
        webview_navigate(g.desktopView, toUtf8(g.desktopOrigin + g.desktopRoute).c_str());
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (IsIconic(g.desktopWindow)) ShowWindow(g.desktopWindow, SW_RESTO…，成立时执行对应分支。
    if (IsIconic(g.desktopWindow)) ShowWindow(g.desktopWindow, SW_RESTORE);
    // 【逐行批注】 在前一条件不成立时继续判断：(!IsZoomed(g.desktopWindow) && !IsWindowVisible(g.desktopWindow)…。
    else if (!IsZoomed(g.desktopWindow) && !IsWindowVisible(g.desktopWindow)) ShowWindow(g.desktopWindow, SW_SHOW);
    // 【逐行批注】 调用 SetForegroundWindow 完成本行所需操作：SetForegroundWindow(g.desktopWindow);。
    SetForegroundWindow(g.desktopWindow);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 stopService，执行该函数声明的职责。
void stopService() {
    // 【逐行批注】 计算右侧结果并写入 g.cancelHealth，更新当前运行状态。
    g.cancelHealth = true;
    // 【逐行批注】 判断条件 (g.healthThread.joinable()) g.healthThread.join();，成立时执行对应分支。
    if (g.healthThread.joinable()) g.healthThread.join();
    // 【逐行批注】 判断条件 (!g.process) return;，成立时执行对应分支。
    if (!g.process) return;
    // 【逐行批注】 判断条件 (processRunning()) {，成立时执行对应分支。
    if (processRunning()) {
        // 【逐行批注】 调用 localHttpRequest 完成本行所需操作：localHttpRequest(g.port, "POST", "/internal/shutdown", g.controlToken…。
        localHttpRequest(g.port, "POST", "/internal/shutdown", g.controlToken);
        // 【逐行批注】 判断条件 (WaitForSingleObject(g.process, 5000) == WAIT_TIMEOUT) Terminate…，成立时执行对应分支。
        if (WaitForSingleObject(g.process, 5000) == WAIT_TIMEOUT) TerminateProcess(g.process, 1);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 CloseHandle 完成本行所需操作：CloseHandle(g.process);。
    CloseHandle(g.process);
    // 【逐行批注】 计算右侧结果并写入 g.process，更新当前运行状态。
    g.process = nullptr;
    // 【逐行批注】 计算右侧结果并写入 g.port，更新当前运行状态。
    g.port = 0;
    // 【逐行批注】 执行当前声明或表达式：g.controlToken.clear();。
    g.controlToken.clear();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 startService，执行该函数声明的职责。
bool startService(bool openWhenReady, bool resetRestart = true) {
    // 【逐行批注】 调用 stopService 完成本行所需操作：stopService();。
    stopService();
    // 恢复备份或重启可能复用同一端口，也需要重新读取页面与本机记录。
    // 【逐行批注】 计算右侧结果并写入 g.displayPort，更新当前运行状态。
    g.displayPort = 0;
    // 【逐行批注】 判断条件 (resetRestart) {，成立时执行对应分支。
    if (resetRestart) {
        // 【逐行批注】 执行当前声明或表达式：g.restartPolicy.reset();。
        g.restartPolicy.reset();
        // 【逐行批注】 调用 KillTimer 完成本行所需操作：KillTimer(g.window, kRestartTimer);。
        KillTimer(g.window, kRestartTimer);
        // 【逐行批注】 调用 KillTimer 完成本行所需操作：KillTimer(g.window, kStableTimer);。
        KillTimer(g.window, kStableTimer);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 g.settings，更新当前运行状态。
    g.settings = g.smokeMode ? Settings{} : loadSettings();
    // 【逐行批注】 计算右侧结果并写入 std::wstring apiKey，更新当前运行状态。
    std::wstring apiKey = g.smokeMode ? std::wstring{} : readApiKey();
    // 【逐行批注】 判断条件 (!canStartService(g.settings, !apiKey.empty(), g.skipAiSetup)) {，成立时执行对应分支。
    if (!canStartService(g.settings, !apiKey.empty(), g.skipAiSetup)) {
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (g.skipAiSetup) {，成立时执行对应分支。
    if (g.skipAiSetup) {
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        // 【逐行批注】 执行当前声明或表达式：apiKey.clear();。
        apiKey.clear();
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 g.port，更新当前运行状态。
    g.port = findAvailablePort();
    // 【逐行批注】 计算右侧结果并写入 g.controlToken，更新当前运行状态。
    g.controlToken = randomToken();
    // 【逐行批注】 判断条件 (!g.port || g.controlToken.empty()) {，成立时执行对应分支。
    if (!g.port || g.controlToken.empty()) {
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        // 【逐行批注】 调用 showFailure 完成本行所需操作：showFailure(L"找不到可用端口，或无法生成本地控制令牌。");。
        showFailure(L"找不到可用端口，或无法生成本地控制令牌。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 计算右侧结果并写入 std::filesystem::path serverPath，更新当前运行状态。
    const std::filesystem::path serverPath = std::filesystem::path(g.installDir) / L"gangyiAI.exe";
    // 【逐行批注】 计算右侧结果并写入 std::filesystem::path databasePath，更新当前运行状态。
    const std::filesystem::path databasePath = std::filesystem::path(g.dataDir) / L"data" / L"gangyiAI.db";
    // 【逐行批注】 计算右侧结果并写入 std::filesystem::path logDir，更新当前运行状态。
    const std::filesystem::path logDir = std::filesystem::path(g.dataDir) / L"logs";
    // 【逐行批注】 执行当前声明或表达式：std::error_code error;。
    std::error_code error;
    // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(databasePath.parent_path(), error…。
    std::filesystem::create_directories(databasePath.parent_path(), error);
    // 【逐行批注】 判断条件 (error) {，成立时执行对应分支。
    if (error) {
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        // 【逐行批注】 调用 showFailure 完成本行所需操作：showFailure(L"无法创建本地数据库目录，请检查当前用户权限。");。
        showFailure(L"无法创建本地数据库目录，请检查当前用户权限。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(logDir, error);。
    std::filesystem::create_directories(logDir, error);
    // 【逐行批注】 判断条件 (error || !std::filesystem::is_regular_file(serverPath)) {，成立时执行对应分支。
    if (error || !std::filesystem::is_regular_file(serverPath)) {
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        // 【逐行批注】 调用 showFailure 完成本行所需操作：showFailure(L"程序文件或本地数据目录不可用，请重新安装。");。
        showFailure(L"程序文件或本地数据目录不可用，请重新安装。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 计算右侧结果并写入 std::wstring bochaKey，更新当前运行状态。
    std::wstring bochaKey = g.smokeMode ? std::wstring{} : readBochaKey();
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::pair<std::wstring, std::wstring>> overrides = {。
    std::vector<std::pair<std::wstring, std::wstring>> overrides = {
        // 【逐行批注】 执行当前声明或表达式：{L"HOST", L"127.0.0.1"},。
        {L"HOST", L"127.0.0.1"},
        // 【逐行批注】 调用 to_wstring 完成本行所需操作：{L"PORT", std::to_wstring(g.port)},。
        {L"PORT", std::to_wstring(g.port)},
        // 【逐行批注】 执行当前声明或表达式：{L"AI_BASE_URL", g.settings.baseUrl},。
        {L"AI_BASE_URL", g.settings.baseUrl},
        // 【逐行批注】 执行当前声明或表达式：{L"AI_API_KEY", apiKey},。
        {L"AI_API_KEY", apiKey},
        // 【逐行批注】 执行当前声明或表达式：{L"AI_MODEL", g.settings.model},。
        {L"AI_MODEL", g.settings.model},
        // 【逐行批注】 执行当前声明或表达式：{L"BOCHA_API_KEY", bochaKey},。
        {L"BOCHA_API_KEY", bochaKey},
        // 【逐行批注】 调用 path 完成本行所需操作：{L"RESOURCE_SEARCH_CACHE_DIR", (std::filesystem::path(g.dataDir)/L"re…。
        {L"RESOURCE_SEARCH_CACHE_DIR", (std::filesystem::path(g.dataDir)/L"resource-search-cache").wstring()},
        // 【逐行批注】 执行当前声明或表达式：{L"DATABASE_PATH", databasePath.wstring()},。
        {L"DATABASE_PATH", databasePath.wstring()},
        // 【逐行批注】 执行当前声明或表达式：{L"LOCAL_CONTROL_TOKEN", g.controlToken},。
        {L"LOCAL_CONTROL_TOKEN", g.controlToken},
        // 【逐行批注】 执行当前声明或表达式：{L"GANGYI_LAUNCH_SESSION_ID", g.launchSessionId},。
        {L"GANGYI_LAUNCH_SESSION_ID", g.launchSessionId},
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 std::filesystem::path caBundle，更新当前运行状态。
    const std::filesystem::path caBundle = std::filesystem::path(g.installDir) / L"curl-ca-bundle.crt";
    // 【逐行批注】 判断条件 (std::filesystem::is_regular_file(caBundle)) overrides.emplace_b…，成立时执行对应分支。
    if (std::filesystem::is_regular_file(caBundle)) overrides.emplace_back(L"CURL_CA_BUNDLE", caBundle.wstring());
    // 【逐行批注】 计算右侧结果并写入 environment，更新当前运行状态。
    auto environment = childEnvironment(overrides);
    // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
    SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
    // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));。
    SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));
    // 【逐行批注】 按循环规则重复处理：for (auto& [name, value] : overrides) if (name == L"AI_API_KEY" || name ==…。
    for (auto& [name, value] : overrides) if (name == L"AI_API_KEY" || name == L"BOCHA_API_KEY")
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(value.data(), value.size() * sizeof(wchar_t));。
        SecureZeroMemory(value.data(), value.size() * sizeof(wchar_t));

    // 【逐行批注】 执行当前声明或表达式：SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};。
    SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
    // 【逐行批注】 计算右侧结果并写入 std::filesystem::path logPath，更新当前运行状态。
    const std::filesystem::path logPath = logDir / L"gangyiAI.log";
    // 【逐行批注】 调用 rotateLogFiles 完成本行所需操作：gangyi::launcher::rotateLogFiles(logPath);。
    gangyi::launcher::rotateLogFiles(logPath);
    // 【逐行批注】 计算右侧结果并写入 HANDLE log，更新当前运行状态。
    HANDLE log = CreateFileW(logPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
        // 【逐行批注】 执行当前声明或表达式：&security, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);。
        &security, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    // 【逐行批注】 计算右侧结果并写入 HANDLE nullInput，更新当前运行状态。
    HANDLE nullInput = INVALID_HANDLE_VALUE;
    // 【逐行批注】 执行当前声明或表达式：STARTUPINFOW startup{};。
    STARTUPINFOW startup{};
    // 【逐行批注】 计算右侧结果并写入 startup.cb，更新当前运行状态。
    startup.cb = sizeof(startup);
    // 【逐行批注】 计算右侧结果并写入 startup.dwFlags，更新当前运行状态。
    startup.dwFlags = STARTF_USESHOWWINDOW;
    // 【逐行批注】 计算右侧结果并写入 startup.wShowWindow，更新当前运行状态。
    startup.wShowWindow = SW_HIDE;
    // 【逐行批注】 判断条件 (log != INVALID_HANDLE_VALUE) {，成立时执行对应分支。
    if (log != INVALID_HANDLE_VALUE) {
        // 【逐行批注】 计算右侧结果并写入 nullInput，更新当前运行状态。
        nullInput = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
            // 【逐行批注】 执行当前声明或表达式：&security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);。
            &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        // 【逐行批注】 计算右侧结果并写入 startup.dwFlags |，更新当前运行状态。
        startup.dwFlags |= STARTF_USESTDHANDLES;
        // 【逐行批注】 计算右侧结果并写入 startup.hStdOutput，更新当前运行状态。
        startup.hStdOutput = log;
        // 【逐行批注】 计算右侧结果并写入 startup.hStdError，更新当前运行状态。
        startup.hStdError = log;
        // 【逐行批注】 计算右侧结果并写入 startup.hStdInput，更新当前运行状态。
        startup.hStdInput = nullInput == INVALID_HANDLE_VALUE ? nullptr : nullInput;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：PROCESS_INFORMATION process{};。
    PROCESS_INFORMATION process{};
    // 【逐行批注】 计算右侧结果并写入 std::wstring command，更新当前运行状态。
    std::wstring command = L"\"" + serverPath.wstring() + L"\"";
    // 【逐行批注】 声明对象 mutableCommand，并通过构造函数初始化其状态。
    std::vector<wchar_t> mutableCommand(command.begin(), command.end());
    // 【逐行批注】 执行当前声明或表达式：mutableCommand.push_back(L'\0');。
    mutableCommand.push_back(L'\0');
    // 【逐行批注】 计算右侧结果并写入 BOOL created，更新当前运行状态。
    const BOOL created = CreateProcessW(serverPath.c_str(), mutableCommand.data(), nullptr, nullptr,
        // 【逐行批注】 执行当前声明或表达式：log != INVALID_HANDLE_VALUE, CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, environm…。
        log != INVALID_HANDLE_VALUE, CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, environment.data(),
        // 【逐行批注】 执行当前声明或表达式：g.installDir.c_str(), &startup, &process);。
        g.installDir.c_str(), &startup, &process);
    // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(environment.data(), environment.size() * sizeof(wcha…。
    SecureZeroMemory(environment.data(), environment.size() * sizeof(wchar_t));
    // 【逐行批注】 判断条件 (log != INVALID_HANDLE_VALUE) CloseHandle(log);，成立时执行对应分支。
    if (log != INVALID_HANDLE_VALUE) CloseHandle(log);
    // 【逐行批注】 判断条件 (nullInput != INVALID_HANDLE_VALUE) CloseHandle(nullInput);，成立时执行对应分支。
    if (nullInput != INVALID_HANDLE_VALUE) CloseHandle(nullInput);
    // 【逐行批注】 判断条件 (!created) {，成立时执行对应分支。
    if (!created) {
        // 【逐行批注】 调用 showFailure 完成本行所需操作：showFailure(L"无法启动钢一定制AI服务，请查看安装是否完整。");。
        showFailure(L"无法启动钢一定制AI服务，请查看安装是否完整。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 CloseHandle 完成本行所需操作：CloseHandle(process.hThread);。
    CloseHandle(process.hThread);
    // 【逐行批注】 计算右侧结果并写入 g.process，更新当前运行状态。
    g.process = process.hProcess;
    // 【逐行批注】 判断条件 (g.job && !AssignProcessToJobObject(g.job, g.process)) {，成立时执行对应分支。
    if (g.job && !AssignProcessToJobObject(g.job, g.process)) {
        // 【逐行批注】 调用 TerminateProcess 完成本行所需操作：TerminateProcess(g.process, 1);。
        TerminateProcess(g.process, 1);
        // 【逐行批注】 调用 CloseHandle 完成本行所需操作：CloseHandle(g.process);。
        CloseHandle(g.process);
        // 【逐行批注】 计算右侧结果并写入 g.process，更新当前运行状态。
        g.process = nullptr;
        // 【逐行批注】 调用 showFailure 完成本行所需操作：showFailure(L"无法接管本地服务进程，请重新启动程序。");。
        showFailure(L"无法接管本地服务进程，请重新启动程序。");
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 g.openWhenReady，更新当前运行状态。
    g.openWhenReady = openWhenReady;
    // 【逐行批注】 计算右侧结果并写入 g.cancelHealth，更新当前运行状态。
    g.cancelHealth = false;
    // 【逐行批注】 调用 updateTrayTip 完成本行所需操作：updateTrayTip(L"钢一定制AI - 正在启动");。
    updateTrayTip(L"钢一定制AI - 正在启动");
    // 【逐行批注】 计算右侧结果并写入 g.healthThread，更新当前运行状态。
    g.healthThread = std::thread([] {
        // 【逐行批注】 按循环规则重复处理：for (int attempt = 0; attempt < 100 && !g.cancelHealth; ++attempt) {。
        for (int attempt = 0; attempt < 100 && !g.cancelHealth; ++attempt) {
            // 【逐行批注】 判断条件 (!processRunning()) break;，成立时执行对应分支。
            if (!processRunning()) break;
            // 【逐行批注】 判断条件 (localHttpRequest(g.port, "GET", "/health")) {，成立时执行对应分支。
            if (localHttpRequest(g.port, "GET", "/health")) {
                // 【逐行批注】 判断条件 (!g.cancelHealth) PostMessageW(g.window, kServiceReady, 0, 0);，成立时执行对应分支。
                if (!g.cancelHealth) PostMessageW(g.window, kServiceReady, 0, 0);
                // 【逐行批注】 在条件成立期间循环处理：while (!g.cancelHealth) {。
                while (!g.cancelHealth) {
                    // 【逐行批注】 判断条件 (WaitForSingleObject(g.process, 500) == WAIT_OBJECT_0) {，成立时执行对应分支。
                    if (WaitForSingleObject(g.process, 500) == WAIT_OBJECT_0) {
                        // 【逐行批注】 计算右侧结果并写入 DWORD exitCode，更新当前运行状态。
                        DWORD exitCode = 0;
                        // 【逐行批注】 调用 GetExitCodeProcess 完成本行所需操作：GetExitCodeProcess(g.process, &exitCode);。
                        GetExitCodeProcess(g.process, &exitCode);
                        // 【逐行批注】 判断条件 (!g.cancelHealth) PostMessageW(g.window, kServiceFailed, exitCod…，成立时执行对应分支。
                        if (!g.cancelHealth) PostMessageW(g.window, kServiceFailed, exitCode, 0);
                        // 【逐行批注】 结束当前函数并返回结果：return;。
                        return;
                    // 【逐行批注】 结束当前代码块。
                    }
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 结束当前函数并返回结果：return;。
                return;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 Sleep 完成本行所需操作：Sleep(150);。
            Sleep(150);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (!g.cancelHealth) PostMessageW(g.window, kServiceFailed, 0, 0);，成立时执行对应分支。
        if (!g.cancelHealth) PostMessageW(g.window, kServiceFailed, 0, 0);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 scheduleAutomaticRestart，执行该函数声明的职责。
void scheduleAutomaticRestart(DWORD exitCode) {
    // 【逐行批注】 计算右侧结果并写入 g.lastExitCode，更新当前运行状态。
    g.lastExitCode = exitCode;
    // 【逐行批注】 调用 stopService 完成本行所需操作：stopService();。
    stopService();
    // 【逐行批注】 计算右侧结果并写入 auto delay，更新当前运行状态。
    const auto delay = g.restartPolicy.recordFailure();
    // 【逐行批注】 判断条件 (!delay) {，成立时执行对应分支。
    if (!delay) {
        // 【逐行批注】 调用 showFailure 完成本行所需操作：showFailure(L"本地服务连续三次恢复失败，已停止自动重启。请查看日志或导出诊断报告。");。
        showFailure(L"本地服务连续三次恢复失败，已停止自动重启。请查看日志或导出诊断报告。");
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 std::wstring tip，更新当前运行状态。
    const std::wstring tip = L"钢一定制AI - " + std::to_wstring(*delay) + L" 秒后尝试恢复";
    // 【逐行批注】 调用 updateTrayTip 完成本行所需操作：updateTrayTip(tip.c_str());。
    updateTrayTip(tip.c_str());
    // 【逐行批注】 调用 SetTimer 完成本行所需操作：SetTimer(g.window, kRestartTimer, *delay * 1000, nullptr);。
    SetTimer(g.window, kRestartTimer, *delay * 1000, nullptr);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 databasePath，执行该函数声明的职责。
std::filesystem::path databasePath() {
    // 【逐行批注】 结束当前函数并返回结果：return std::filesystem::path(g.dataDir) / L"data" / L"gangyiAI.db";。
    return std::filesystem::path(g.dataDir) / L"data" / L"gangyiAI.db";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 logPath，执行该函数声明的职责。
std::filesystem::path logPath() {
    // 【逐行批注】 结束当前函数并返回结果：return std::filesystem::path(g.dataDir) / L"logs" / L"gangyiAI.log";。
    return std::filesystem::path(g.dataDir) / L"logs" / L"gangyiAI.log";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 timestampText，执行该函数声明的职责。
std::wstring timestampText() {
    // 【逐行批注】 计算右侧结果并写入 std::time_t now，更新当前运行状态。
    const std::time_t now = std::time(nullptr);
    // 【逐行批注】 执行当前声明或表达式：std::tm local{};。
    std::tm local{};
    // 【逐行批注】 调用 localtime_s 完成本行所需操作：localtime_s(&local, &now);。
    localtime_s(&local, &now);
    // 【逐行批注】 执行当前声明或表达式：std::wostringstream value;。
    std::wostringstream value;
    // 【逐行批注】 调用 put_time 完成本行所需操作：value << std::put_time(&local, L"%Y%m%d-%H%M%S");。
    value << std::put_time(&local, L"%Y%m%d-%H%M%S");
    // 【逐行批注】 结束当前函数并返回结果：return value.str();。
    return value.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 chooseFile，并通过构造函数初始化其状态。
std::optional<std::filesystem::path> chooseFile(bool save, const wchar_t* title, const std::wstring& initialName,
                                                // 【逐行批注】 执行当前声明或表达式：const wchar_t* filter, const wchar_t* defaultExtension) {。
                                                const wchar_t* filter, const wchar_t* defaultExtension) {
    // 【逐行批注】 声明对象 buffer，并通过构造函数初始化其状态。
    std::vector<wchar_t> buffer(32768, L'\0');
    // 【逐行批注】 调用 copy_n、min 完成本行所需操作：std::copy_n(initialName.c_str(), std::min(initialName.size(), buffer.…。
    std::copy_n(initialName.c_str(), std::min(initialName.size(), buffer.size() - 1), buffer.data());
    // 【逐行批注】 执行当前声明或表达式：OPENFILENAMEW dialog{};。
    OPENFILENAMEW dialog{};
    // 【逐行批注】 计算右侧结果并写入 dialog.lStructSize，更新当前运行状态。
    dialog.lStructSize = sizeof(dialog);
    // 【逐行批注】 计算右侧结果并写入 dialog.hwndOwner，更新当前运行状态。
    dialog.hwndOwner = g.window;
    // 【逐行批注】 计算右侧结果并写入 dialog.lpstrFilter，更新当前运行状态。
    dialog.lpstrFilter = filter;
    // 【逐行批注】 计算右侧结果并写入 dialog.lpstrFile，更新当前运行状态。
    dialog.lpstrFile = buffer.data();
    // 【逐行批注】 计算右侧结果并写入 dialog.nMaxFile，更新当前运行状态。
    dialog.nMaxFile = static_cast<DWORD>(buffer.size());
    // 【逐行批注】 计算右侧结果并写入 dialog.lpstrTitle，更新当前运行状态。
    dialog.lpstrTitle = title;
    // 【逐行批注】 计算右侧结果并写入 dialog.lpstrDefExt，更新当前运行状态。
    dialog.lpstrDefExt = defaultExtension;
    // 【逐行批注】 计算右侧结果并写入 dialog.Flags，更新当前运行状态。
    dialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR |
        // 【逐行批注】 执行当前声明或表达式：(save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);。
        (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
    // 【逐行批注】 计算右侧结果并写入 BOOL selected，更新当前运行状态。
    const BOOL selected = save ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog);
    // 【逐行批注】 判断条件 (!selected) return std::nullopt;，成立时执行对应分支。
    if (!selected) return std::nullopt;
    // 【逐行批注】 结束当前函数并返回结果：return std::filesystem::path(buffer.data());。
    return std::filesystem::path(buffer.data());
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 viewLog，执行该函数声明的职责。
void viewLog() {
    // 【逐行批注】 执行当前声明或表达式：std::error_code error;。
    std::error_code error;
    // 【逐行批注】 调用 create_directories、logPath 完成本行所需操作：std::filesystem::create_directories(logPath().parent_path(), error);。
    std::filesystem::create_directories(logPath().parent_path(), error);
    // 【逐行批注】 判断条件 (!std::filesystem::exists(logPath())) {，成立时执行对应分支。
    if (!std::filesystem::exists(logPath())) {
        // 【逐行批注】 声明对象 create，并通过构造函数初始化其状态。
        std::ofstream create(logPath());
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 ShellExecuteW、logPath 完成本行所需操作：ShellExecuteW(nullptr, L"open", logPath().c_str(), nullptr, nullptr, …。
    ShellExecuteW(nullptr, L"open", logPath().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 openDataDirectory，执行该函数声明的职责。
void openDataDirectory() {
    // 【逐行批注】 执行当前声明或表达式：std::error_code error;。
    std::error_code error;
    // 【逐行批注】 调用 create_directories、path 完成本行所需操作：std::filesystem::create_directories(std::filesystem::path(g.dataDir) …。
    std::filesystem::create_directories(std::filesystem::path(g.dataDir) / L"data", error);
    // 【逐行批注】 调用 ShellExecuteW 完成本行所需操作：ShellExecuteW(nullptr, L"open", g.dataDir.c_str(), nullptr, nullptr, …。
    ShellExecuteW(nullptr, L"open", g.dataDir.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 backupCourseData，执行该函数声明的职责。
void backupCourseData() {
    // 【逐行批注】 判断条件 (!std::filesystem::is_regular_file(databasePath())) {，成立时执行对应分支。
    if (!std::filesystem::is_regular_file(databasePath())) {
        // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, L"尚未找到课程数据库，请先创建课程后再备份。", L"钢一定制AI", MB_OK | MB…。
        MessageBoxW(g.window, L"尚未找到课程数据库，请先创建课程后再备份。", L"钢一定制AI", MB_OK | MB_ICONINFORMATION);
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 constexpr wchar_t databaseFilter[]，更新当前运行状态。
    constexpr wchar_t databaseFilter[] = L"课程数据库 (*.db)\0*.db\0所有文件 (*.*)\0*.*\0";
    // 【逐行批注】 计算右侧结果并写入 auto selected，更新当前运行状态。
    const auto selected = chooseFile(true, L"备份课程数据", L"gangyiAI-backup-" + timestampText() + L".db",
        // 【逐行批注】 执行当前声明或表达式：databaseFilter, L"db");。
        databaseFilter, L"db");
    // 【逐行批注】 判断条件 (!selected) return;，成立时执行对应分支。
    if (!selected) return;
    // 【逐行批注】 执行当前声明或表达式：std::wstring error;。
    std::wstring error;
    // 【逐行批注】 判断条件 (!gangyi::launcher::backupDatabase(databasePath(), *selected, er…，成立时执行对应分支。
    if (!gangyi::launcher::backupDatabase(databasePath(), *selected, error)) {
        // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 备份失败", MB_OK | MB_ICO…。
        MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 备份失败", MB_OK | MB_ICONERROR);
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 MessageBoxW、wstring 完成本行所需操作：MessageBoxW(g.window, (L"课程数据已备份到：\n" + selected->wstring()).c_str(),。
    MessageBoxW(g.window, (L"课程数据已备份到：\n" + selected->wstring()).c_str(),
        // 【逐行批注】 执行当前声明或表达式：L"钢一定制AI", MB_OK | MB_ICONINFORMATION);。
        L"钢一定制AI", MB_OK | MB_ICONINFORMATION);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 restoreCourseData，执行该函数声明的职责。
void restoreCourseData() {
    // 【逐行批注】 计算右侧结果并写入 constexpr wchar_t databaseFilter[]，更新当前运行状态。
    constexpr wchar_t databaseFilter[] = L"课程数据库 (*.db)\0*.db\0所有文件 (*.*)\0*.*\0";
    // 【逐行批注】 计算右侧结果并写入 auto selected，更新当前运行状态。
    const auto selected = chooseFile(false, L"恢复课程数据", L"", databaseFilter, L"db");
    // 【逐行批注】 判断条件 (!selected) return;，成立时执行对应分支。
    if (!selected) return;
    // 【逐行批注】 执行当前声明或表达式：std::wstring error;。
    std::wstring error;
    // 【逐行批注】 判断条件 (!gangyi::launcher::validateDatabase(*selected, error)) {，成立时执行对应分支。
    if (!gangyi::launcher::validateDatabase(*selected, error)) {
        // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 无法恢复", MB_OK | MB_ICO…。
        MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 无法恢复", MB_OK | MB_ICONERROR);
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (MessageBoxW(g.window, L"恢复会替换当前课程数据，程序会自动保留恢复前副本。是否继续？",，成立时执行对应分支。
    if (MessageBoxW(g.window, L"恢复会替换当前课程数据，程序会自动保留恢复前副本。是否继续？",
        // 【逐行批注】 执行当前声明或表达式：L"钢一定制AI", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) return;。
        L"钢一定制AI", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) return;
    // 【逐行批注】 调用 stopService 完成本行所需操作：stopService();。
    stopService();
    // 【逐行批注】 计算右侧结果并写入 auto rollbackDir，更新当前运行状态。
    const auto rollbackDir = std::filesystem::path(g.dataDir) / L"backups";
    // 【逐行批注】 执行当前声明或表达式：std::error_code filesystemError;。
    std::error_code filesystemError;
    // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(rollbackDir, filesystemError);。
    std::filesystem::create_directories(rollbackDir, filesystemError);
    // 【逐行批注】 计算右侧结果并写入 auto rollback，更新当前运行状态。
    const auto rollback = rollbackDir / (L"before-restore-" + timestampText() + L".db");
    // 【逐行批注】 判断条件 (filesystemError || !gangyi::launcher::restoreDatabase(*selected…，成立时执行对应分支。
    if (filesystemError || !gangyi::launcher::restoreDatabase(*selected, databasePath(), rollback, error)) {
        // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 恢复失败", MB_OK | MB_ICO…。
        MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 恢复失败", MB_OK | MB_ICONERROR);
        // 【逐行批注】 调用 startService 完成本行所需操作：startService(false);。
        startService(false);
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 g.pendingRestoreRollback，更新当前运行状态。
    g.pendingRestoreRollback = rollback;
    // 【逐行批注】 判断条件 (!startService(false)) {，成立时执行对应分支。
    if (!startService(false)) {
        // 【逐行批注】 执行当前声明或表达式：std::wstring rollbackError;。
        std::wstring rollbackError;
        // 【逐行批注】 调用 stopService 完成本行所需操作：stopService();。
        stopService();
        // 【逐行批注】 调用 backupDatabase、databasePath 完成本行所需操作：gangyi::launcher::backupDatabase(rollback, databasePath(), rollbackEr…。
        gangyi::launcher::backupDatabase(rollback, databasePath(), rollbackError);
        // 【逐行批注】 执行当前声明或表达式：g.pendingRestoreRollback.clear();。
        g.pendingRestoreRollback.clear();
        // 【逐行批注】 调用 startService 完成本行所需操作：startService(false);。
        startService(false);
        // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, L"新数据库无法启动，已尝试恢复原有数据。请查看日志。",。
        MessageBoxW(g.window, L"新数据库无法启动，已尝试恢复原有数据。请查看日志。",
            // 【逐行批注】 执行当前声明或表达式：L"钢一定制AI - 已回滚", MB_OK | MB_ICONERROR);。
            L"钢一定制AI - 已回滚", MB_OK | MB_ICONERROR);
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, (L"课程数据恢复完成。恢复前副本位于：\n" + rollback.wstring()).c…。
    MessageBoxW(g.window, (L"课程数据恢复完成。恢复前副本位于：\n" + rollback.wstring()).c_str(),
        // 【逐行批注】 执行当前声明或表达式：L"钢一定制AI", MB_OK | MB_ICONINFORMATION);。
        L"钢一定制AI", MB_OK | MB_ICONINFORMATION);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 exportDiagnosticReport，执行该函数声明的职责。
void exportDiagnosticReport() {
    // 【逐行批注】 计算右侧结果并写入 constexpr wchar_t textFilter[]，更新当前运行状态。
    constexpr wchar_t textFilter[] = L"文本文件 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0";
    // 【逐行批注】 计算右侧结果并写入 auto selected，更新当前运行状态。
    const auto selected = chooseFile(true, L"导出诊断报告", L"gangyiAI-diagnostic-" + timestampText() + L".txt",
        // 【逐行批注】 执行当前声明或表达式：textFilter, L"txt");。
        textFilter, L"txt");
    // 【逐行批注】 判断条件 (!selected) return;，成立时执行对应分支。
    if (!selected) return;
    // 【逐行批注】 执行当前声明或表达式：gangyi::launcher::DiagnosticInfo info;。
    gangyi::launcher::DiagnosticInfo info;
    // 【逐行批注】 计算右侧结果并写入 info.version，更新当前运行状态。
    info.version = toWide(gangyi::kVersion);
    // 【逐行批注】 计算右侧结果并写入 info.installDir，更新当前运行状态。
    info.installDir = g.installDir;
    // 【逐行批注】 计算右侧结果并写入 info.dataDir，更新当前运行状态。
    info.dataDir = g.dataDir;
    // 【逐行批注】 计算右侧结果并写入 info.provider，更新当前运行状态。
    info.provider = gangyi::launcher::providerProfile(g.settings.provider).name;
    // 【逐行批注】 计算右侧结果并写入 info.baseUrl，更新当前运行状态。
    info.baseUrl = g.settings.baseUrl;
    // 【逐行批注】 计算右侧结果并写入 info.model，更新当前运行状态。
    info.model = g.settings.model;
    // 【逐行批注】 计算右侧结果并写入 info.serviceRunning，更新当前运行状态。
    info.serviceRunning = processRunning();
    // 【逐行批注】 计算右侧结果并写入 info.port，更新当前运行状态。
    info.port = g.port;
    // 【逐行批注】 计算右侧结果并写入 info.exitCode，更新当前运行状态。
    info.exitCode = g.lastExitCode;
    // 【逐行批注】 计算右侧结果并写入 info.restartFailures，更新当前运行状态。
    info.restartFailures = g.restartPolicy.failureCount();
    // 【逐行批注】 执行当前声明或表达式：std::wstring error;。
    std::wstring error;
    // 【逐行批注】 判断条件 (!gangyi::launcher::writeDiagnosticReport(*selected, info, error…，成立时执行对应分支。
    if (!gangyi::launcher::writeDiagnosticReport(*selected, info, error)) {
        // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 导出失败", MB_OK | MB_ICO…。
        MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 导出失败", MB_OK | MB_ICONERROR);
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, L"诊断报告已生成。报告不包含 API Key、课程内容或 AI 对话正文。",。
    MessageBoxW(g.window, L"诊断报告已生成。报告不包含 API Key、课程内容或 AI 对话正文。",
        // 【逐行批注】 执行当前声明或表达式：L"钢一定制AI", MB_OK | MB_ICONINFORMATION);。
        L"钢一定制AI", MB_OK | MB_ICONINFORMATION);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 loadControls，执行该函数声明的职责。
void loadControls() {
    // 【逐行批注】 计算右侧结果并写入 g.settings，更新当前运行状态。
    g.settings = loadSettings();
    // 【逐行批注】 计算右侧结果并写入 std::wstring apiKey，更新当前运行状态。
    std::wstring apiKey = readApiKey();
    // 【逐行批注】 计算右侧结果并写入 std::wstring bochaKey，更新当前运行状态。
    std::wstring bochaKey = readBochaKey();
    // 【逐行批注】 调用 SendMessageW、providerIndex 完成本行所需操作：SendMessageW(g.providerCombo, CB_SETCURSEL, providerIndex(g.settings.…。
    SendMessageW(g.providerCombo, CB_SETCURSEL, providerIndex(g.settings.provider), 0);
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.baseUrlEdit, g.settings.baseUrl.c_str());。
    SetWindowTextW(g.baseUrlEdit, g.settings.baseUrl.c_str());
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.apiKeyEdit, apiKey.c_str());。
    SetWindowTextW(g.apiKeyEdit, apiKey.c_str());
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.bochaKeyEdit, bochaKey.c_str());。
    SetWindowTextW(g.bochaKeyEdit, bochaKey.c_str());
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.modelEdit, g.settings.model.c_str());。
    SetWindowTextW(g.modelEdit, g.settings.model.c_str());
    // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.baseUrlEdit, g.settings.provider == gangyi::launcher::…。
    EnableWindow(g.baseUrlEdit, g.settings.provider == gangyi::launcher::AIProvider::Custom);
    // 【逐行批注】 调用 SendMessageW 完成本行所需操作：SendMessageW(g.autoStartCheck, BM_SETCHECK, g.settings.autoStart ? BS…。
    SendMessageW(g.autoStartCheck, BM_SETCHECK, g.settings.autoStart ? BST_CHECKED : BST_UNCHECKED, 0);
    // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
    SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
    // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));。
    SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 showSettingsWindow，执行该函数声明的职责。
void showSettingsWindow() {
    // 【逐行批注】 调用 loadControls 完成本行所需操作：loadControls();。
    loadControls();
    // 【逐行批注】 调用 ShowWindow 完成本行所需操作：ShowWindow(g.window, SW_SHOW);。
    ShowWindow(g.window, SW_SHOW);
    // 【逐行批注】 调用 SetForegroundWindow 完成本行所需操作：SetForegroundWindow(g.window);。
    SetForegroundWindow(g.window);
    // 【逐行批注】 调用 SetFocus 完成本行所需操作：SetFocus(g.baseUrlEdit);。
    SetFocus(g.baseUrlEdit);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 saveSettingsFromControls，执行该函数声明的职责。
bool saveSettingsFromControls() {
    // 【逐行批注】 执行当前声明或表达式：Settings settings;。
    Settings settings;
    // 【逐行批注】 计算右侧结果并写入 settings.provider，更新当前运行状态。
    settings.provider = selectedProvider();
    // 【逐行批注】 计算右侧结果并写入 settings.baseUrl，更新当前运行状态。
    settings.baseUrl = controlText(g.baseUrlEdit);
    // 【逐行批注】 计算右侧结果并写入 settings.model，更新当前运行状态。
    settings.model = controlText(g.modelEdit);
    // 【逐行批注】 计算右侧结果并写入 settings.autoStart，更新当前运行状态。
    settings.autoStart = SendMessageW(g.autoStartCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
    // 【逐行批注】 计算右侧结果并写入 std::wstring apiKey，更新当前运行状态。
    std::wstring apiKey = controlText(g.apiKeyEdit);
    // 【逐行批注】 计算右侧结果并写入 std::wstring bochaKey，更新当前运行状态。
    std::wstring bochaKey = controlText(g.bochaKeyEdit);
    // 【逐行批注】 判断条件 (!validBaseUrl(settings.baseUrl) || settings.model.empty() || ap…，成立时执行对应分支。
    if (!validBaseUrl(settings.baseUrl) || settings.model.empty() || apiKey.empty()) {
        // 【逐行批注】 调用 MessageBoxW、HTTP 完成本行所需操作：MessageBoxW(g.window, L"请填写有效的 HTTP(S) API 地址、API Key 和模型名称。", L"钢一定制…。
        MessageBoxW(g.window, L"请填写有效的 HTTP(S) API 地址、API Key 和模型名称。", L"钢一定制AI", MB_OK | MB_ICONWARNING);
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));。
        SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 bool saved，更新当前运行状态。
    const bool saved = writeRegistryString(L"AIBaseUrl", settings.baseUrl) &&
        // 【逐行批注】 调用 writeRegistryString 完成本行所需操作：writeRegistryString(L"AIModel", settings.model) &&。
        writeRegistryString(L"AIModel", settings.model) &&
        // 【逐行批注】 调用 writeRegistryString、providerProfile 完成本行所需操作：writeRegistryString(L"AIProvider", gangyi::launcher::providerProfile(…。
        writeRegistryString(L"AIProvider", gangyi::launcher::providerProfile(settings.provider).id) &&
        // 【逐行批注】 调用 writeRegistryDword 完成本行所需操作：writeRegistryDword(L"AutoStart", settings.autoStart ? 1 : 0) &&。
        writeRegistryDword(L"AutoStart", settings.autoStart ? 1 : 0) &&
        // 【逐行批注】 调用 writeApiKey 完成本行所需操作：writeApiKey(apiKey) && (bochaKey.empty() ?。
        writeApiKey(apiKey) && (bochaKey.empty() ?
            // 【逐行批注】 调用 CredDeleteW、GetLastError 完成本行所需操作：(CredDeleteW(kBochaCredentialTarget, CRED_TYPE_GENERIC, 0) || GetLast…。
            (CredDeleteW(kBochaCredentialTarget, CRED_TYPE_GENERIC, 0) || GetLastError() == ERROR_NOT_FOUND) :
            // 【逐行批注】 调用 writeCredential 完成本行所需操作：writeCredential(kBochaCredentialTarget, bochaKey)) &&。
            writeCredential(kBochaCredentialTarget, bochaKey)) &&
        // 【逐行批注】 调用 writeRegistryDword 完成本行所需操作：writeRegistryDword(L"BochaDisabled", bochaKey.empty() ? 1 : 0) &&。
        writeRegistryDword(L"BochaDisabled", bochaKey.empty() ? 1 : 0) &&
        // 【逐行批注】 调用 writeRegistryDword、configureAutoStart 完成本行所需操作：writeRegistryDword(L"SkipAISetup", 0) && configureAutoStart(settings.…。
        writeRegistryDword(L"SkipAISetup", 0) && configureAutoStart(settings.autoStart);
    // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
    SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
    // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));。
    SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.apiKeyEdit, L"");。
    SetWindowTextW(g.apiKeyEdit, L"");
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.bochaKeyEdit, L"");。
    SetWindowTextW(g.bochaKeyEdit, L"");
    // 【逐行批注】 判断条件 (!saved) {，成立时执行对应分支。
    if (!saved) {
        // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, L"配置保存失败，请检查当前用户权限。", L"钢一定制AI", MB_OK | MB_ICO…。
        MessageBoxW(g.window, L"配置保存失败，请检查当前用户权限。", L"钢一定制AI", MB_OK | MB_ICONERROR);
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 g.settings，更新当前运行状态。
    g.settings = settings;
    // 【逐行批注】 计算右侧结果并写入 g.hasConfig，更新当前运行状态。
    g.hasConfig = true;
    // 【逐行批注】 计算右侧结果并写入 g.skipAiSetup，更新当前运行状态。
    g.skipAiSetup = false;
    // 【逐行批注】 调用 ShowWindow 完成本行所需操作：ShowWindow(g.window, SW_HIDE);。
    ShowWindow(g.window, SW_HIDE);
    // 【逐行批注】 结束当前函数并返回结果：return startService(true);。
    return startService(true);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 skipAiSetup，执行该函数声明的职责。
void skipAiSetup() {
    // 【逐行批注】 计算右侧结果并写入 bool autoStart，更新当前运行状态。
    const bool autoStart = SendMessageW(g.autoStartCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
    // 【逐行批注】 判断条件 (!configureAutoStart(autoStart) || !writeRegistryDword(L"AutoSta…，成立时执行对应分支。
    if (!configureAutoStart(autoStart) || !writeRegistryDword(L"AutoStart", autoStart ? 1 : 0) ||
        // 【逐行批注】 调用 writeRegistryDword 完成本行所需操作：!writeRegistryDword(L"SkipAISetup", 1)) {。
        !writeRegistryDword(L"SkipAISetup", 1)) {
        // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(g.window, L"暂不配置的选择保存失败，请检查当前用户权限。", L"钢一定制AI", MB_OK | M…。
        MessageBoxW(g.window, L"暂不配置的选择保存失败，请检查当前用户权限。", L"钢一定制AI", MB_OK | MB_ICONERROR);
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.apiKeyEdit, L"");。
    SetWindowTextW(g.apiKeyEdit, L"");
    // 【逐行批注】 计算右侧结果并写入 g.skipAiSetup，更新当前运行状态。
    g.skipAiSetup = true;
    // 【逐行批注】 调用 ShowWindow 完成本行所需操作：ShowWindow(g.window, SW_HIDE);。
    ShowWindow(g.window, SW_HIDE);
    // 【逐行批注】 判断条件 (!startService(true)) showSettingsWindow();，成立时执行对应分支。
    if (!startService(true)) showSettingsWindow();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 connectionErrorMessage，执行该函数声明的职责。
std::wstring connectionErrorMessage(const std::string& type) {
    // 【逐行批注】 判断条件 (type == "missing_config") return L"配置不完整，请填写 API 地址、API Key 和模型…，成立时执行对应分支。
    if (type == "missing_config") return L"配置不完整，请填写 API 地址、API Key 和模型名称。";
    // 【逐行批注】 判断条件 (type == "auth_error") return L"认证失败，请检查 API Key 是否正确或是否具有访问权限。";，成立时执行对应分支。
    if (type == "auth_error") return L"认证失败，请检查 API Key 是否正确或是否具有访问权限。";
    // 【逐行批注】 判断条件 (type == "rate_limited") return L"接口请求过于频繁或余额不足，请稍后重试。";，成立时执行对应分支。
    if (type == "rate_limited") return L"接口请求过于频繁或余额不足，请稍后重试。";
    // 【逐行批注】 判断条件 (type == "timeout") return L"连接超时，请检查网络、接口地址或代理设置。";，成立时执行对应分支。
    if (type == "timeout") return L"连接超时，请检查网络、接口地址或代理设置。";
    // 【逐行批注】 判断条件 (type == "network_error") return L"无法连接 AI 接口，请检查网络和 API 地址。";，成立时执行对应分支。
    if (type == "network_error") return L"无法连接 AI 接口，请检查网络和 API 地址。";
    // 【逐行批注】 判断条件 (type == "provider_5xx") return L"AI 服务暂时不可用，请稍后重试。";，成立时执行对应分支。
    if (type == "provider_5xx") return L"AI 服务暂时不可用，请稍后重试。";
    // 【逐行批注】 判断条件 (type == "invalid_response") return L"接口已连接，但返回格式不是兼容的 Chat Comp…，成立时执行对应分支。
    if (type == "invalid_response") return L"接口已连接，但返回格式不是兼容的 Chat Completions 响应。";
    // 【逐行批注】 判断条件 (type == "models_unsupported") return L"该接口未提供模型列表，请手工填写模型名称。";，成立时执行对应分支。
    if (type == "models_unsupported") return L"该接口未提供模型列表，请手工填写模型名称。";
    // 【逐行批注】 结束当前函数并返回结果：return L"连接测试失败，请检查 API 地址和模型名称。";。
    return L"连接测试失败，请检查 API 地址和模型名称。";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 applyProviderSelection，执行该函数声明的职责。
void applyProviderSelection() {
    // 【逐行批注】 计算右侧结果并写入 auto provider，更新当前运行状态。
    const auto provider = selectedProvider();
    // 【逐行批注】 计算右侧结果并写入 auto& profile，更新当前运行状态。
    const auto& profile = gangyi::launcher::providerProfile(provider);
    // 【逐行批注】 计算右侧结果并写入 bool custom，更新当前运行状态。
    const bool custom = provider == gangyi::launcher::AIProvider::Custom;
    // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.baseUrlEdit, custom);。
    EnableWindow(g.baseUrlEdit, custom);
    // 【逐行批注】 判断条件 (!custom) {，成立时执行对应分支。
    if (!custom) {
        // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.baseUrlEdit, profile.baseUrl);。
        SetWindowTextW(g.baseUrlEdit, profile.baseUrl);
        // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.modelEdit, profile.recommendedModel);。
        SetWindowTextW(g.modelEdit, profile.recommendedModel);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.apiKeyEdit, L"");。
    SetWindowTextW(g.apiKeyEdit, L"");
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.statusLabel, L"服务商已切换，请填写对应的 API Key 并获取模型。");。
    SetWindowTextW(g.statusLabel, L"服务商已切换，请填写对应的 API Key 并获取模型。");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 fetchModelsFromControls，执行该函数声明的职责。
void fetchModelsFromControls() {
    // 【逐行批注】 判断条件 (g.modelListRunning.exchange(true)) return;，成立时执行对应分支。
    if (g.modelListRunning.exchange(true)) return;
    // 【逐行批注】 计算右侧结果并写入 std::wstring baseUrl，更新当前运行状态。
    std::wstring baseUrl = controlText(g.baseUrlEdit);
    // 【逐行批注】 计算右侧结果并写入 std::wstring apiKey，更新当前运行状态。
    std::wstring apiKey = controlText(g.apiKeyEdit);
    // 【逐行批注】 计算右侧结果并写入 std::wstring currentModel，更新当前运行状态。
    std::wstring currentModel = controlText(g.modelEdit);
    // 【逐行批注】 判断条件 (!validBaseUrl(baseUrl) || apiKey.empty()) {，成立时执行对应分支。
    if (!validBaseUrl(baseUrl) || apiKey.empty()) {
        // 【逐行批注】 计算右侧结果并写入 g.modelListRunning，更新当前运行状态。
        g.modelListRunning = false;
        // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.statusLabel, L"请先填写有效的 API 地址和 API Key。");。
        SetWindowTextW(g.statusLabel, L"请先填写有效的 API 地址和 API Key。");
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (g.modelListThread.joinable()) g.modelListThread.join();，成立时执行对应分支。
    if (g.modelListThread.joinable()) g.modelListThread.join();
    // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.fetchModelsButton, FALSE);。
    EnableWindow(g.fetchModelsButton, FALSE);
    // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.testButton, FALSE);。
    EnableWindow(g.testButton, FALSE);
    // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.saveButton, FALSE);。
    EnableWindow(g.saveButton, FALSE);
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.statusLabel, L"正在获取可用模型，请稍候……");。
    SetWindowTextW(g.statusLabel, L"正在获取可用模型，请稍候……");
    // 【逐行批注】 计算右侧结果并写入 g.modelListThread，更新当前运行状态。
    g.modelListThread = std::thread([baseUrl = std::move(baseUrl), apiKey = std::move(apiKey),
                                     // 【逐行批注】 计算右侧结果并写入 currentModel，更新当前运行状态。
                                     currentModel = std::move(currentModel)]() mutable {
        // 【逐行批注】 计算右侧结果并写入 bool succeeded，更新当前运行状态。
        bool succeeded = false;
        // 【逐行批注】 执行当前声明或表达式：std::wstring message;。
        std::wstring message;
        // 【逐行批注】 执行当前声明或表达式：std::vector<std::wstring> models;。
        std::vector<std::wstring> models;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::AIClientConfig config;。
            gangyi::AIClientConfig config;
            // 【逐行批注】 计算右侧结果并写入 config.baseUrl，更新当前运行状态。
            config.baseUrl = toUtf8(baseUrl);
            // 【逐行批注】 计算右侧结果并写入 config.apiKey，更新当前运行状态。
            config.apiKey = toUtf8(apiKey);
            // 【逐行批注】 计算右侧结果并写入 config.model，更新当前运行状态。
            config.model = toUtf8(currentModel);
            // 【逐行批注】 计算右侧结果并写入 config.timeoutMs，更新当前运行状态。
            config.timeoutMs = 15000;
            // 【逐行批注】 计算右侧结果并写入 config.retryAttempts，更新当前运行状态。
            config.retryAttempts = 1;
            // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
            SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
            // 【逐行批注】 声明对象 client，并通过构造函数初始化其状态。
            gangyi::AIClient client(std::move(config));
            // 【逐行批注】 按循环规则重复处理：for (const auto& model : client.listModels(15000)) models.push_back(toWide…。
            for (const auto& model : client.listModels(15000)) models.push_back(toWide(model));
            // 【逐行批注】 计算右侧结果并写入 succeeded，更新当前运行状态。
            succeeded = !models.empty();
            // 【逐行批注】 计算右侧结果并写入 message，更新当前运行状态。
            message = succeeded ? L"模型列表已更新，也可以继续手工输入模型名称。"
                                // 【逐行批注】 执行当前声明或表达式：: L"接口没有返回模型，请手工填写模型名称。";。
                                : L"接口没有返回模型，请手工填写模型名称。";
        // 【逐行批注】 执行当前声明或表达式：} catch (const gangyi::AIClientError& error) {。
        } catch (const gangyi::AIClientError& error) {
            // 【逐行批注】 计算右侧结果并写入 message，更新当前运行状态。
            message = connectionErrorMessage(error.errorType);
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 计算右侧结果并写入 message，更新当前运行状态。
            message = L"获取模型时发生未知错误，请手工填写模型名称。";
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        // 【逐行批注】 开始当前代码块。
        {
            // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
            std::lock_guard<std::mutex> lock(g.modelListMutex);
            // 【逐行批注】 计算右侧结果并写入 g.availableModels，更新当前运行状态。
            g.availableModels = std::move(models);
            // 【逐行批注】 计算右侧结果并写入 g.modelListSucceeded，更新当前运行状态。
            g.modelListSucceeded = succeeded;
            // 【逐行批注】 计算右侧结果并写入 g.modelListMessage，更新当前运行状态。
            g.modelListMessage = std::move(message);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (!g.shuttingDown && !PostMessageW(g.window, kModelListComplete, …，成立时执行对应分支。
        if (!g.shuttingDown && !PostMessageW(g.window, kModelListComplete, 0, 0)) g.modelListRunning = false;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 testConnectionFromControls，执行该函数声明的职责。
void testConnectionFromControls() {
    // 【逐行批注】 判断条件 (g.connectionTestRunning.exchange(true)) return;，成立时执行对应分支。
    if (g.connectionTestRunning.exchange(true)) return;
    // 【逐行批注】 计算右侧结果并写入 std::wstring baseUrl，更新当前运行状态。
    std::wstring baseUrl = controlText(g.baseUrlEdit);
    // 【逐行批注】 计算右侧结果并写入 std::wstring apiKey，更新当前运行状态。
    std::wstring apiKey = controlText(g.apiKeyEdit);
    // 【逐行批注】 计算右侧结果并写入 std::wstring model，更新当前运行状态。
    std::wstring model = controlText(g.modelEdit);
    // 【逐行批注】 判断条件 (!validBaseUrl(baseUrl) || apiKey.empty() || model.empty()) {，成立时执行对应分支。
    if (!validBaseUrl(baseUrl) || apiKey.empty() || model.empty()) {
        // 【逐行批注】 计算右侧结果并写入 g.connectionTestRunning，更新当前运行状态。
        g.connectionTestRunning = false;
        // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.statusLabel, L"请先填写有效的 API 地址、API Key 和模型名称。");。
        SetWindowTextW(g.statusLabel, L"请先填写有效的 API 地址、API Key 和模型名称。");
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (g.connectionTestThread.joinable()) g.connectionTestThread.join(…，成立时执行对应分支。
    if (g.connectionTestThread.joinable()) g.connectionTestThread.join();
    // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.testButton, FALSE);。
    EnableWindow(g.testButton, FALSE);
    // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.saveButton, FALSE);。
    EnableWindow(g.saveButton, FALSE);
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.statusLabel, L"正在测试 AI 连接，请稍候……");。
    SetWindowTextW(g.statusLabel, L"正在测试 AI 连接，请稍候……");
    // 【逐行批注】 计算右侧结果并写入 g.connectionTestThread，更新当前运行状态。
    g.connectionTestThread = std::thread([baseUrl = std::move(baseUrl), apiKey = std::move(apiKey),
                                         // 【逐行批注】 计算右侧结果并写入 model，更新当前运行状态。
                                         model = std::move(model)]() mutable {
        // 【逐行批注】 计算右侧结果并写入 bool succeeded，更新当前运行状态。
        bool succeeded = false;
        // 【逐行批注】 执行当前声明或表达式：std::wstring message;。
        std::wstring message;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：gangyi::AIClientConfig config;。
            gangyi::AIClientConfig config;
            // 【逐行批注】 计算右侧结果并写入 config.baseUrl，更新当前运行状态。
            config.baseUrl = toUtf8(baseUrl);
            // 【逐行批注】 计算右侧结果并写入 config.apiKey，更新当前运行状态。
            config.apiKey = toUtf8(apiKey);
            // 【逐行批注】 计算右侧结果并写入 config.model，更新当前运行状态。
            config.model = toUtf8(model);
            // 【逐行批注】 计算右侧结果并写入 config.timeoutMs，更新当前运行状态。
            config.timeoutMs = 15000;
            // 【逐行批注】 计算右侧结果并写入 config.retryAttempts，更新当前运行状态。
            config.retryAttempts = 1;
            // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
            SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));

            // 【逐行批注】 声明对象 client，并通过构造函数初始化其状态。
            gangyi::AIClient client(std::move(config));
            // 【逐行批注】 执行当前声明或表达式：gangyi::ChatOptions options;。
            gangyi::ChatOptions options;
            // 【逐行批注】 计算右侧结果并写入 options.activity.source，更新当前运行状态。
            options.activity.source = "AI 设置";
            // 【逐行批注】 计算右侧结果并写入 options.activity.purpose，更新当前运行状态。
            options.activity.purpose = "测试 API 地址、密钥和模型是否可用";
            // 【逐行批注】 计算右侧结果并写入 options.cancelled，更新当前运行状态。
            options.cancelled = [] { return g.shuttingDown.load(); };
            // 【逐行批注】 计算右侧结果并写入 options.messages，更新当前运行状态。
            options.messages = {{"user", "Reply with OK."}};
            // 【逐行批注】 计算右侧结果并写入 options.temperature，更新当前运行状态。
            options.temperature = 0;
            // 【逐行批注】 计算右侧结果并写入 options.maxTokens，更新当前运行状态。
            options.maxTokens = 16;
            // 【逐行批注】 计算右侧结果并写入 options.timeoutMs，更新当前运行状态。
            options.timeoutMs = 15000;
            // 【逐行批注】 计算右侧结果并写入 options.maxAttempts，更新当前运行状态。
            options.maxAttempts = 1;
            // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
            const auto result = client.chat(options);
            // 【逐行批注】 计算右侧结果并写入 succeeded，更新当前运行状态。
            succeeded = result.status >= 200 && result.status < 300;
            // 【逐行批注】 计算右侧结果并写入 message，更新当前运行状态。
            message = succeeded ? L"连接成功，API 地址、Key 和模型均可用。" : L"连接测试失败。";
        // 【逐行批注】 执行当前声明或表达式：} catch (const gangyi::AIClientError& error) {。
        } catch (const gangyi::AIClientError& error) {
            // 【逐行批注】 计算右侧结果并写入 message，更新当前运行状态。
            message = connectionErrorMessage(error.errorType);
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 计算右侧结果并写入 message，更新当前运行状态。
            message = L"连接测试发生未知错误，请检查配置后重试。";
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));。
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        // 【逐行批注】 开始当前代码块。
        {
            // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
            std::lock_guard<std::mutex> lock(g.connectionTestMutex);
            // 【逐行批注】 计算右侧结果并写入 g.connectionTestSucceeded，更新当前运行状态。
            g.connectionTestSucceeded = succeeded;
            // 【逐行批注】 计算右侧结果并写入 g.connectionTestMessage，更新当前运行状态。
            g.connectionTestMessage = std::move(message);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (!g.shuttingDown && !PostMessageW(g.window, kConnectionTestCompl…，成立时执行对应分支。
        if (!g.shuttingDown && !PostMessageW(g.window, kConnectionTestComplete, 0, 0)) {
            // 【逐行批注】 计算右侧结果并写入 g.connectionTestRunning，更新当前运行状态。
            g.connectionTestRunning = false;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 testBochaFromControls，执行该函数声明的职责。
void testBochaFromControls() {
    // 【逐行批注】 判断条件 (g.connectionTestRunning.exchange(true)) return;，成立时执行对应分支。
    if (g.connectionTestRunning.exchange(true)) return;
    // 【逐行批注】 计算右侧结果并写入 std::wstring key，更新当前运行状态。
    std::wstring key = controlText(g.bochaKeyEdit);
    // 【逐行批注】 判断条件 (key.empty()) {，成立时执行对应分支。
    if (key.empty()) {
        // 【逐行批注】 计算右侧结果并写入 g.connectionTestRunning，更新当前运行状态。
        g.connectionTestRunning = false;
        // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.statusLabel, L"请先填写博查 Key。");。
        SetWindowTextW(g.statusLabel, L"请先填写博查 Key。");
        // 【逐行批注】 结束当前函数并返回结果：return;。
        return;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (g.connectionTestThread.joinable()) g.connectionTestThread.join(…，成立时执行对应分支。
    if (g.connectionTestThread.joinable()) g.connectionTestThread.join();
    // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.bochaTestButton, FALSE);。
    EnableWindow(g.bochaTestButton, FALSE);
    // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.testButton, FALSE);。
    EnableWindow(g.testButton, FALSE);
    // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.saveButton, FALSE);。
    EnableWindow(g.saveButton, FALSE);
    // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.statusLabel, L"正在测试博查连接，请稍候……");。
    SetWindowTextW(g.statusLabel, L"正在测试博查连接，请稍候……");
    // 【逐行批注】 计算右侧结果并写入 g.connectionTestThread，更新当前运行状态。
    g.connectionTestThread = std::thread([key = std::move(key)]() mutable {
        // 【逐行批注】 计算右侧结果并写入 bool succeeded，更新当前运行状态。
        bool succeeded = false;
        // 【逐行批注】 执行当前声明或表达式：std::string diagnostic;。
        std::string diagnostic;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 std::string utf8Key，更新当前运行状态。
            std::string utf8Key = toUtf8(key);
            // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(key.data(), key.size() * sizeof(wchar_t));。
            SecureZeroMemory(key.data(), key.size() * sizeof(wchar_t));
            // 【逐行批注】 计算右侧结果并写入 succeeded，更新当前运行状态。
            succeeded = gangyi::SearchClient::testBochaKey(utf8Key, &diagnostic);
            // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(utf8Key.data(), utf8Key.size());。
            SecureZeroMemory(utf8Key.data(), utf8Key.size());
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {}。
        } catch (...) {}
        // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(key.data(), key.size() * sizeof(wchar_t));。
        SecureZeroMemory(key.data(), key.size() * sizeof(wchar_t));
        // 【逐行批注】 计算右侧结果并写入 std::wstring message，更新当前运行状态。
        const std::wstring message = succeeded ?
            // 【逐行批注】 执行当前声明或表达式：(diagnostic == "ok_direct" ? L"博查连接成功（已绕过故障代理）。" : L"博查连接成功。") :。
            (diagnostic == "ok_direct" ? L"博查连接成功（已绕过故障代理）。" : L"博查连接成功。") :
            // 【逐行批注】 执行当前声明或表达式：diagnostic == "http:401" ? L"博查认证失败（HTTP 401）：请确认粘贴的是 API Key。" :。
            diagnostic == "http:401" ? L"博查认证失败（HTTP 401）：请确认粘贴的是 API Key。" :
            // 【逐行批注】 执行当前声明或表达式：diagnostic == "http:403" ? L"博查权限或账户余额不足（HTTP 403）。" :。
            diagnostic == "http:403" ? L"博查权限或账户余额不足（HTTP 403）。" :
            // 【逐行批注】 执行当前声明或表达式：diagnostic == "http:429" ? L"博查请求频率受限（HTTP 429），请稍后重试。" :。
            diagnostic == "http:429" ? L"博查请求频率受限（HTTP 429），请稍后重试。" :
            // 【逐行批注】 执行当前声明或表达式：diagnostic.rfind("network_error:", 0) == 0 ? L"博查网络连接失败，请检查代理或证书。" :。
            diagnostic.rfind("network_error:", 0) == 0 ? L"博查网络连接失败，请检查代理或证书。" :
            // 【逐行批注】 执行当前声明或表达式：diagnostic == "invalid_response" ? L"博查已响应，但返回内容不符合接口格式。" :。
            diagnostic == "invalid_response" ? L"博查已响应，但返回内容不符合接口格式。" :
            // 【逐行批注】 执行当前声明或表达式：L"博查服务返回错误，请稍后重试。";。
            L"博查服务返回错误，请稍后重试。";
        // 【逐行批注】 开始当前代码块。
        {
            // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
            std::lock_guard<std::mutex> lock(g.connectionTestMutex);
            // 【逐行批注】 计算右侧结果并写入 g.connectionTestSucceeded，更新当前运行状态。
            g.connectionTestSucceeded = succeeded;
            // 【逐行批注】 计算右侧结果并写入 g.connectionTestMessage，更新当前运行状态。
            g.connectionTestMessage = message;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (!g.shuttingDown && !PostMessageW(g.window, kConnectionTestCompl…，成立时执行对应分支。
        if (!g.shuttingDown && !PostMessageW(g.window, kConnectionTestComplete, 0, 0))
            // 【逐行批注】 计算右侧结果并写入 g.connectionTestRunning，更新当前运行状态。
            g.connectionTestRunning = false;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 appIcon，执行该函数声明的职责。
HICON appIcon() {
    // 【逐行批注】 计算右侧结果并写入 HICON icon，更新当前运行状态。
    HICON icon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_GANGYI_AI));
    // 【逐行批注】 结束当前函数并返回结果：return icon ? icon : LoadIconW(nullptr, IDI_APPLICATION);。
    return icon ? icon : LoadIconW(nullptr, IDI_APPLICATION);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 addTrayIcon，执行该函数声明的职责。
void addTrayIcon() {
    // 【逐行批注】 执行当前声明或表达式：NOTIFYICONDATAW icon{};。
    NOTIFYICONDATAW icon{};
    // 【逐行批注】 计算右侧结果并写入 icon.cbSize，更新当前运行状态。
    icon.cbSize = sizeof(icon);
    // 【逐行批注】 计算右侧结果并写入 icon.hWnd，更新当前运行状态。
    icon.hWnd = g.window;
    // 【逐行批注】 计算右侧结果并写入 icon.uID，更新当前运行状态。
    icon.uID = 1;
    // 【逐行批注】 计算右侧结果并写入 icon.uFlags，更新当前运行状态。
    icon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    // 【逐行批注】 计算右侧结果并写入 icon.uCallbackMessage，更新当前运行状态。
    icon.uCallbackMessage = kTrayMessage;
    // 【逐行批注】 计算右侧结果并写入 icon.hIcon，更新当前运行状态。
    icon.hIcon = appIcon();
    // 【逐行批注】 调用 wcsncpy_s、ARRAYSIZE 完成本行所需操作：wcsncpy_s(icon.szTip, ARRAYSIZE(icon.szTip), L"钢一定制AI", _TRUNCATE);。
    wcsncpy_s(icon.szTip, ARRAYSIZE(icon.szTip), L"钢一定制AI", _TRUNCATE);
    // 【逐行批注】 调用 Shell_NotifyIconW 完成本行所需操作：Shell_NotifyIconW(NIM_ADD, &icon);。
    Shell_NotifyIconW(NIM_ADD, &icon);
    // 【逐行批注】 计算右侧结果并写入 icon.uVersion，更新当前运行状态。
    icon.uVersion = NOTIFYICON_VERSION_4;
    // 【逐行批注】 调用 Shell_NotifyIconW 完成本行所需操作：Shell_NotifyIconW(NIM_SETVERSION, &icon);。
    Shell_NotifyIconW(NIM_SETVERSION, &icon);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 removeTrayIcon，执行该函数声明的职责。
void removeTrayIcon() {
    // 【逐行批注】 执行当前声明或表达式：NOTIFYICONDATAW icon{};。
    NOTIFYICONDATAW icon{};
    // 【逐行批注】 计算右侧结果并写入 icon.cbSize，更新当前运行状态。
    icon.cbSize = sizeof(icon);
    // 【逐行批注】 计算右侧结果并写入 icon.hWnd，更新当前运行状态。
    icon.hWnd = g.window;
    // 【逐行批注】 计算右侧结果并写入 icon.uID，更新当前运行状态。
    icon.uID = 1;
    // 【逐行批注】 调用 Shell_NotifyIconW 完成本行所需操作：Shell_NotifyIconW(NIM_DELETE, &icon);。
    Shell_NotifyIconW(NIM_DELETE, &icon);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 showTrayMenu，执行该函数声明的职责。
void showTrayMenu() {
    // 【逐行批注】 执行当前声明或表达式：POINT point{};。
    POINT point{};
    // 【逐行批注】 调用 GetCursorPos 完成本行所需操作：GetCursorPos(&point);。
    GetCursorPos(&point);
    // 【逐行批注】 计算右侧结果并写入 HMENU menu，更新当前运行状态。
    HMENU menu = CreatePopupMenu();
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_STRING, kMenuOpen, L"打开钢一定制AI");。
    AppendMenuW(menu, MF_STRING, kMenuOpen, L"打开钢一定制AI");
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_STRING, kMenuSettings, L"设置");。
    AppendMenuW(menu, MF_STRING, kMenuSettings, L"设置");
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_STRING, kMenuRestart, L"重启服务");。
    AppendMenuW(menu, MF_STRING, kMenuRestart, L"重启服务");
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);。
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_STRING, kMenuViewLog, L"查看日志");。
    AppendMenuW(menu, MF_STRING, kMenuViewLog, L"查看日志");
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_STRING, kMenuOpenData, L"打开数据目录");。
    AppendMenuW(menu, MF_STRING, kMenuOpenData, L"打开数据目录");
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_STRING, kMenuDiagnostic, L"导出诊断报告");。
    AppendMenuW(menu, MF_STRING, kMenuDiagnostic, L"导出诊断报告");
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);。
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_STRING, kMenuBackup, L"备份课程数据");。
    AppendMenuW(menu, MF_STRING, kMenuBackup, L"备份课程数据");
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_STRING, kMenuRestore, L"恢复课程数据");。
    AppendMenuW(menu, MF_STRING, kMenuRestore, L"恢复课程数据");
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);。
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    // 【逐行批注】 调用 AppendMenuW 完成本行所需操作：AppendMenuW(menu, MF_STRING, kMenuExit, L"退出");。
    AppendMenuW(menu, MF_STRING, kMenuExit, L"退出");
    // 【逐行批注】 调用 SetForegroundWindow 完成本行所需操作：SetForegroundWindow(g.window);。
    SetForegroundWindow(g.window);
    // 【逐行批注】 调用 TrackPopupMenu 完成本行所需操作：TrackPopupMenu(menu, TPM_RIGHTBUTTON, point.x, point.y, 0, g.window, …。
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, point.x, point.y, 0, g.window, nullptr);
    // 【逐行批注】 调用 DestroyMenu 完成本行所需操作：DestroyMenu(menu);。
    DestroyMenu(menu);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 createLabel，执行该函数声明的职责。
void createLabel(HWND parent, const wchar_t* text, int x, int y, int width) {
    // 【逐行批注】 调用 CreateWindowW 完成本行所需操作：CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, width, 22…。
    CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, width, 22, parent, nullptr, nullptr, nullptr);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 windowProc，执行该函数声明的职责。
LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    // 【逐行批注】 根据表达式分支处理：switch (message) {。
    switch (message) {
    // 【逐行批注】 声明 switch 的分支入口：case WM_CREATE: {。
    case WM_CREATE: {
        // 【逐行批注】 计算右侧结果并写入 g.window，更新当前运行状态。
        g.window = window;
        // 【逐行批注】 计算右侧结果并写入 HFONT font，更新当前运行状态。
        HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        // 【逐行批注】 调用 createLabel 完成本行所需操作：createLabel(window, L"AI 服务商", 24, 18, 120);。
        createLabel(window, L"AI 服务商", 24, 18, 120);
        // 【逐行批注】 调用 createLabel 完成本行所需操作：createLabel(window, L"AI API 地址", 24, 72, 120);。
        createLabel(window, L"AI API 地址", 24, 72, 120);
        // 【逐行批注】 调用 createLabel 完成本行所需操作：createLabel(window, L"API Key", 24, 126, 120);。
        createLabel(window, L"API Key", 24, 126, 120);
        // 【逐行批注】 调用 createLabel 完成本行所需操作：createLabel(window, L"模型名称", 24, 180, 120);。
        createLabel(window, L"模型名称", 24, 180, 120);
        // 【逐行批注】 调用 createLabel 完成本行所需操作：createLabel(window, L"博查 Key（与 DeepSeek 并行搜索）", 24, 238, 400);。
        createLabel(window, L"博查 Key（与 DeepSeek 并行搜索）", 24, 238, 400);
        // 【逐行批注】 计算右侧结果并写入 g.providerCombo，更新当前运行状态。
        g.providerCombo = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
            // 【逐行批注】 执行当前声明或表达式：24, 41, 456, 160, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kProviderComb…。
            24, 41, 456, 160, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kProviderCombo)), nullptr, nullptr);
        // 【逐行批注】 调用 SendMessageW 完成本行所需操作：SendMessageW(g.providerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARA…。
        SendMessageW(g.providerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"DeepSeek"));
        // 【逐行批注】 调用 SendMessageW 完成本行所需操作：SendMessageW(g.providerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARA…。
        SendMessageW(g.providerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"OpenAI"));
        // 【逐行批注】 调用 SendMessageW 完成本行所需操作：SendMessageW(g.providerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARA…。
        SendMessageW(g.providerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"自定义 OpenAI 兼容接口"));
        // 【逐行批注】 计算右侧结果并写入 g.baseUrlEdit，更新当前运行状态。
        g.baseUrlEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            // 【逐行批注】 执行当前声明或表达式：24, 95, 456, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBaseUrlEdit))…。
            24, 95, 456, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBaseUrlEdit)), nullptr, nullptr);
        // 【逐行批注】 计算右侧结果并写入 g.apiKeyEdit，更新当前运行状态。
        g.apiKeyEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
            // 【逐行批注】 执行当前声明或表达式：24, 149, 456, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kApiKeyEdit))…。
            24, 149, 456, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kApiKeyEdit)), nullptr, nullptr);
        // 【逐行批注】 计算右侧结果并写入 g.modelEdit，更新当前运行状态。
        g.modelEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
            // 【逐行批注】 执行当前声明或表达式：WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL,。
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL,
            // 【逐行批注】 执行当前声明或表达式：24, 203, 346, 220, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kModelEdit))…。
            24, 203, 346, 220, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kModelEdit)), nullptr, nullptr);
        // 【逐行批注】 计算右侧结果并写入 g.fetchModelsButton，更新当前运行状态。
        g.fetchModelsButton = CreateWindowW(L"BUTTON", L"获取模型", WS_CHILD | WS_VISIBLE,
            // 【逐行批注】 执行当前声明或表达式：378, 202, 102, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFetchModels…。
            378, 202, 102, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFetchModelsButton)), nullptr, nullptr);
        // 【逐行批注】 计算右侧结果并写入 g.bochaKeyEdit，更新当前运行状态。
        g.bochaKeyEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
            // 【逐行批注】 执行当前声明或表达式：24, 260, 346, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBochaKeyEdit…。
            24, 260, 346, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBochaKeyEdit)), nullptr, nullptr);
        // 【逐行批注】 计算右侧结果并写入 g.bochaTestButton，更新当前运行状态。
        g.bochaTestButton = CreateWindowW(L"BUTTON", L"测试博查", WS_CHILD | WS_VISIBLE,
            // 【逐行批注】 执行当前声明或表达式：378, 259, 102, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBochaTestBu…。
            378, 259, 102, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBochaTestButton)), nullptr, nullptr);
        // 【逐行批注】 计算右侧结果并写入 g.autoStartCheck，更新当前运行状态。
        g.autoStartCheck = CreateWindowW(L"BUTTON", L"登录 Windows 后自动启动", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            // 【逐行批注】 执行当前声明或表达式：24, 299, 250, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kAutoStartChe…。
            24, 299, 250, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kAutoStartCheck)), nullptr, nullptr);
        // 【逐行批注】 调用 CreateWindowW 完成本行所需操作：CreateWindowW(L"BUTTON", L"暂不配置，直接使用", WS_CHILD | WS_VISIBLE,。
        CreateWindowW(L"BUTTON", L"暂不配置，直接使用", WS_CHILD | WS_VISIBLE,
            // 【逐行批注】 执行当前声明或表达式：24, 369, 148, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSkipAiButton…。
            24, 369, 148, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSkipAiButton)), nullptr, nullptr);
        // 【逐行批注】 计算右侧结果并写入 g.testButton，更新当前运行状态。
        g.testButton = CreateWindowW(L"BUTTON", L"测试连接", WS_CHILD | WS_VISIBLE,
            // 【逐行批注】 执行当前声明或表达式：180, 369, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTestButton))…。
            180, 369, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTestButton)), nullptr, nullptr);
        // 【逐行批注】 计算右侧结果并写入 g.saveButton，更新当前运行状态。
        g.saveButton = CreateWindowW(L"BUTTON", L"保存并启动", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            // 【逐行批注】 执行当前声明或表达式：282, 369, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSaveButton))…。
            282, 369, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSaveButton)), nullptr, nullptr);
        // 【逐行批注】 调用 CreateWindowW 完成本行所需操作：CreateWindowW(L"BUTTON", L"取消", WS_CHILD | WS_VISIBLE,。
        CreateWindowW(L"BUTTON", L"取消", WS_CHILD | WS_VISIBLE,
            // 【逐行批注】 执行当前声明或表达式：384, 369, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCancelButton…。
            384, 369, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCancelButton)), nullptr, nullptr);
        // 【逐行批注】 计算右侧结果并写入 g.statusLabel，更新当前运行状态。
        g.statusLabel = CreateWindowW(L"STATIC", L"可先使用本地功能；AI 功能可稍后在托盘“设置”中启用。", WS_CHILD | WS_VISIBLE,
            // 【逐行批注】 执行当前声明或表达式：24, 333, 456, 24, window, nullptr, nullptr, nullptr);。
            24, 333, 456, 24, window, nullptr, nullptr, nullptr);
        // 【逐行批注】 调用 EnumChildWindows 完成本行所需操作：EnumChildWindows(window, [](HWND child, LPARAM value) -> BOOL {。
        EnumChildWindows(window, [](HWND child, LPARAM value) -> BOOL {
            // 【逐行批注】 调用 SendMessageW 完成本行所需操作：SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(value), TRUE);。
            SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(value), TRUE);
            // 【逐行批注】 结束当前函数并返回结果：return TRUE;。
            return TRUE;
        // 【逐行批注】 执行当前声明或表达式：}, reinterpret_cast<LPARAM>(font));。
        }, reinterpret_cast<LPARAM>(font));
        // 【逐行批注】 调用 addTrayIcon 完成本行所需操作：addTrayIcon();。
        addTrayIcon();
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 声明 switch 的分支入口：case WM_COMMAND:。
    case WM_COMMAND:
        // 【逐行批注】 根据表达式分支处理：switch (LOWORD(wParam)) {。
        switch (LOWORD(wParam)) {
        // 【逐行批注】 声明 switch 的分支入口：case kProviderCombo:。
        case kProviderCombo:
            // 【逐行批注】 判断条件 (HIWORD(wParam) == CBN_SELCHANGE) applyProviderSelection();，成立时执行对应分支。
            if (HIWORD(wParam) == CBN_SELCHANGE) applyProviderSelection();
            // 【逐行批注】 结束当前函数并返回结果：return 0;。
            return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kFetchModelsButton: fetchModelsFromControls(); re…。
        case kFetchModelsButton: fetchModelsFromControls(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kTestButton: testConnectionFromControls(); return…。
        case kTestButton: testConnectionFromControls(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kBochaTestButton: testBochaFromControls(); return…。
        case kBochaTestButton: testBochaFromControls(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kSaveButton: saveSettingsFromControls(); return 0;。
        case kSaveButton: saveSettingsFromControls(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kSkipAiButton: skipAiSetup(); return 0;。
        case kSkipAiButton: skipAiSetup(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kCancelButton:。
        case kCancelButton:
            // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.apiKeyEdit, L"");。
            SetWindowTextW(g.apiKeyEdit, L"");
            // 【逐行批注】 判断条件 (g.hasConfig || g.skipAiSetup) ShowWindow(window, SW_HIDE); else…，成立时执行对应分支。
            if (g.hasConfig || g.skipAiSetup) ShowWindow(window, SW_HIDE); else DestroyWindow(window);
            // 【逐行批注】 结束当前函数并返回结果：return 0;。
            return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kMenuOpen: PostMessageW(window, kOpenDesktop, 0, …。
        case kMenuOpen: PostMessageW(window, kOpenDesktop, 0, 0); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kMenuSettings: showSettingsWindow(); return 0;。
        case kMenuSettings: showSettingsWindow(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kMenuRestart:。
        case kMenuRestart:
            // 【逐行批注】 判断条件 (!startService(false)) showSettingsWindow();，成立时执行对应分支。
            if (!startService(false)) showSettingsWindow();
            // 【逐行批注】 结束当前函数并返回结果：return 0;。
            return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kMenuViewLog: viewLog(); return 0;。
        case kMenuViewLog: viewLog(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kMenuOpenData: openDataDirectory(); return 0;。
        case kMenuOpenData: openDataDirectory(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kMenuBackup: backupCourseData(); return 0;。
        case kMenuBackup: backupCourseData(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kMenuRestore: restoreCourseData(); return 0;。
        case kMenuRestore: restoreCourseData(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kMenuDiagnostic: exportDiagnosticReport(); return…。
        case kMenuDiagnostic: exportDiagnosticReport(); return 0;
        // 【逐行批注】 声明 switch 的分支入口：case kMenuExit: DestroyWindow(window); return 0;。
        case kMenuExit: DestroyWindow(window); return 0;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 跳出当前循环或分支。
        break;
    // 【逐行批注】 声明 switch 的分支入口：case kTrayMessage:。
    case kTrayMessage:
        // 【逐行批注】 判断条件 (LOWORD(lParam) == WM_LBUTTONDBLCLK) PostMessageW(window, kOpenD…，成立时执行对应分支。
        if (LOWORD(lParam) == WM_LBUTTONDBLCLK) PostMessageW(window, kOpenDesktop, 0, 0);
        // 【逐行批注】 在前一条件不成立时继续判断：(LOWORD(lParam) == WM_CONTEXTMENU || LOWORD(lParam) == WM_RBUTTO…。
        else if (LOWORD(lParam) == WM_CONTEXTMENU || LOWORD(lParam) == WM_RBUTTONUP) showTrayMenu();
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case kServiceReady:。
    case kServiceReady:
        // 【逐行批注】 调用 updateTrayTip 完成本行所需操作：updateTrayTip(L"钢一定制AI - 运行中");。
        updateTrayTip(L"钢一定制AI - 运行中");
        // 【逐行批注】 执行当前声明或表达式：g.pendingRestoreRollback.clear();。
        g.pendingRestoreRollback.clear();
        // 【逐行批注】 调用 KillTimer 完成本行所需操作：KillTimer(window, kStableTimer);。
        KillTimer(window, kStableTimer);
        // 【逐行批注】 调用 SetTimer 完成本行所需操作：SetTimer(window, kStableTimer, 10 * 60 * 1000, nullptr);。
        SetTimer(window, kStableTimer, 10 * 60 * 1000, nullptr);
        // 【逐行批注】 判断条件 (g.openWhenReady || g.desktopWindow) openDesktop();，成立时执行对应分支。
        if (g.openWhenReady || g.desktopWindow) openDesktop();
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case kServiceFailed:。
    case kServiceFailed:
        // 【逐行批注】 调用 KillTimer 完成本行所需操作：KillTimer(window, kStableTimer);。
        KillTimer(window, kStableTimer);
        // 【逐行批注】 判断条件 (!g.pendingRestoreRollback.empty()) {，成立时执行对应分支。
        if (!g.pendingRestoreRollback.empty()) {
            // 【逐行批注】 计算右侧结果并写入 auto rollback，更新当前运行状态。
            const auto rollback = g.pendingRestoreRollback;
            // 【逐行批注】 执行当前声明或表达式：g.pendingRestoreRollback.clear();。
            g.pendingRestoreRollback.clear();
            // 【逐行批注】 调用 stopService 完成本行所需操作：stopService();。
            stopService();
            // 【逐行批注】 执行当前声明或表达式：std::wstring rollbackError;。
            std::wstring rollbackError;
            // 【逐行批注】 判断条件 (gangyi::launcher::backupDatabase(rollback, databasePath(), roll…，成立时执行对应分支。
            if (gangyi::launcher::backupDatabase(rollback, databasePath(), rollbackError)) {
                // 【逐行批注】 调用 startService 完成本行所需操作：startService(false);。
                startService(false);
                // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(window, L"恢复后的数据库无法启动，已恢复到操作前数据。",。
                MessageBoxW(window, L"恢复后的数据库无法启动，已恢复到操作前数据。",
                    // 【逐行批注】 执行当前声明或表达式：L"钢一定制AI - 已自动回滚", MB_OK | MB_ICONWARNING);。
                    L"钢一定制AI - 已自动回滚", MB_OK | MB_ICONWARNING);
            // 【逐行批注】 执行当前声明或表达式：} else {。
            } else {
                // 【逐行批注】 调用 showFailure 完成本行所需操作：showFailure(L"恢复后的数据库无法启动，自动回滚也失败。请保留数据目录并联系维护人员。");。
                showFailure(L"恢复后的数据库无法启动，自动回滚也失败。请保留数据目录并联系维护人员。");
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return 0;。
            return 0;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 scheduleAutomaticRestart 完成本行所需操作：scheduleAutomaticRestart(static_cast<DWORD>(wParam));。
        scheduleAutomaticRestart(static_cast<DWORD>(wParam));
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case WM_TIMER:。
    case WM_TIMER:
        // 【逐行批注】 判断条件 (wParam == kRestartTimer) {，成立时执行对应分支。
        if (wParam == kRestartTimer) {
            // 【逐行批注】 调用 KillTimer 完成本行所需操作：KillTimer(window, kRestartTimer);。
            KillTimer(window, kRestartTimer);
            // 【逐行批注】 判断条件 (!startService(false, false)) scheduleAutomaticRestart(g.lastExi…，成立时执行对应分支。
            if (!startService(false, false)) scheduleAutomaticRestart(g.lastExitCode);
            // 【逐行批注】 结束当前函数并返回结果：return 0;。
            return 0;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (wParam == kStableTimer) {，成立时执行对应分支。
        if (wParam == kStableTimer) {
            // 【逐行批注】 调用 KillTimer 完成本行所需操作：KillTimer(window, kStableTimer);。
            KillTimer(window, kStableTimer);
            // 【逐行批注】 执行当前声明或表达式：g.restartPolicy.reset();。
            g.restartPolicy.reset();
            // 【逐行批注】 结束当前函数并返回结果：return 0;。
            return 0;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 跳出当前循环或分支。
        break;
    // 【逐行批注】 声明 switch 的分支入口：case kOpenDesktop:。
    case kOpenDesktop:
        // 【逐行批注】 判断条件 (processRunning()) openDesktop(); else if (!startService(true)) …，成立时执行对应分支。
        if (processRunning()) openDesktop(); else if (!startService(true)) showSettingsWindow();
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case kMyAIBackup: backupCourseData(); return 0;。
    case kMyAIBackup: backupCourseData(); return 0;
    // 【逐行批注】 声明 switch 的分支入口：case kMyAIRestore: restoreCourseData(); return 0;。
    case kMyAIRestore: restoreCourseData(); return 0;
    // 【逐行批注】 声明 switch 的分支入口：case kOpenApiSettings:。
    case kOpenApiSettings:
        // 【逐行批注】 调用 showSettingsWindow 完成本行所需操作：showSettingsWindow();。
        showSettingsWindow();
        // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：if(!myAIProposedModel.empty()&&g.modelEdit){SetWindowTextW(g.modelEdi…。
        if(!myAIProposedModel.empty()&&g.modelEdit){SetWindowTextW(g.modelEdit,myAIProposedModel.c_str());myAIProposedModel.clear();}
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case kConnectionTestComplete: {。
    case kConnectionTestComplete: {
        // 【逐行批注】 执行当前声明或表达式：std::wstring message;。
        std::wstring message;
        // 【逐行批注】 计算右侧结果并写入 bool succeeded，更新当前运行状态。
        bool succeeded = false;
        // 【逐行批注】 开始当前代码块。
        {
            // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
            std::lock_guard<std::mutex> lock(g.connectionTestMutex);
            // 【逐行批注】 计算右侧结果并写入 message，更新当前运行状态。
            message = g.connectionTestMessage;
            // 【逐行批注】 计算右侧结果并写入 succeeded，更新当前运行状态。
            succeeded = g.connectionTestSucceeded;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (g.connectionTestThread.joinable()) g.connectionTestThread.join(…，成立时执行对应分支。
        if (g.connectionTestThread.joinable()) g.connectionTestThread.join();
        // 【逐行批注】 计算右侧结果并写入 g.connectionTestRunning，更新当前运行状态。
        g.connectionTestRunning = false;
        // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.testButton, TRUE);。
        EnableWindow(g.testButton, TRUE);
        // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.bochaTestButton, TRUE);。
        EnableWindow(g.bochaTestButton, TRUE);
        // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.saveButton, TRUE);。
        EnableWindow(g.saveButton, TRUE);
        // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.statusLabel, message.c_str());。
        SetWindowTextW(g.statusLabel, message.c_str());
        // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(window, message.c_str(), L"钢一定制AI - 连接测试",。
        MessageBoxW(window, message.c_str(), L"钢一定制AI - 连接测试",
            // 【逐行批注】 执行当前声明或表达式：MB_OK | (succeeded ? MB_ICONINFORMATION : MB_ICONWARNING));。
            MB_OK | (succeeded ? MB_ICONINFORMATION : MB_ICONWARNING));
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 声明 switch 的分支入口：case kModelListComplete: {。
    case kModelListComplete: {
        // 【逐行批注】 执行当前声明或表达式：std::wstring message;。
        std::wstring message;
        // 【逐行批注】 执行当前声明或表达式：std::vector<std::wstring> models;。
        std::vector<std::wstring> models;
        // 【逐行批注】 计算右侧结果并写入 bool succeeded，更新当前运行状态。
        bool succeeded = false;
        // 【逐行批注】 开始当前代码块。
        {
            // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
            std::lock_guard<std::mutex> lock(g.modelListMutex);
            // 【逐行批注】 计算右侧结果并写入 message，更新当前运行状态。
            message = g.modelListMessage;
            // 【逐行批注】 计算右侧结果并写入 models，更新当前运行状态。
            models = g.availableModels;
            // 【逐行批注】 计算右侧结果并写入 succeeded，更新当前运行状态。
            succeeded = g.modelListSucceeded;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (g.modelListThread.joinable()) g.modelListThread.join();，成立时执行对应分支。
        if (g.modelListThread.joinable()) g.modelListThread.join();
        // 【逐行批注】 计算右侧结果并写入 g.modelListRunning，更新当前运行状态。
        g.modelListRunning = false;
        // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.fetchModelsButton, TRUE);。
        EnableWindow(g.fetchModelsButton, TRUE);
        // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.testButton, TRUE);。
        EnableWindow(g.testButton, TRUE);
        // 【逐行批注】 调用 EnableWindow 完成本行所需操作：EnableWindow(g.saveButton, TRUE);。
        EnableWindow(g.saveButton, TRUE);
        // 【逐行批注】 判断条件 (succeeded) {，成立时执行对应分支。
        if (succeeded) {
            // 【逐行批注】 计算右侧结果并写入 std::wstring current，更新当前运行状态。
            const std::wstring current = controlText(g.modelEdit);
            // 【逐行批注】 调用 SendMessageW 完成本行所需操作：SendMessageW(g.modelEdit, CB_RESETCONTENT, 0, 0);。
            SendMessageW(g.modelEdit, CB_RESETCONTENT, 0, 0);
            // 【逐行批注】 计算右侧结果并写入 int selected，更新当前运行状态。
            int selected = -1;
            // 【逐行批注】 按循环规则重复处理：for (size_t index = 0; index < models.size(); ++index) {。
            for (size_t index = 0; index < models.size(); ++index) {
                // 【逐行批注】 调用 SendMessageW 完成本行所需操作：SendMessageW(g.modelEdit, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(m…。
                SendMessageW(g.modelEdit, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(models[index].c_str()));
                // 【逐行批注】 判断条件 (models[index] == current) selected = static_cast<int>(index);，成立时执行对应分支。
                if (models[index] == current) selected = static_cast<int>(index);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (selected >= 0) SendMessageW(g.modelEdit, CB_SETCURSEL, selected…，成立时执行对应分支。
            if (selected >= 0) SendMessageW(g.modelEdit, CB_SETCURSEL, selected, 0);
            // 【逐行批注】 在前一条件不成立时继续判断：(!current.empty()) SetWindowTextW(g.modelEdit, current.c_str());。
            else if (!current.empty()) SetWindowTextW(g.modelEdit, current.c_str());
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.statusLabel, message.c_str());。
        SetWindowTextW(g.statusLabel, message.c_str());
        // 【逐行批注】 判断条件 (!succeeded) MessageBoxW(window, message.c_str(), L"钢一定制AI - 获取模…，成立时执行对应分支。
        if (!succeeded) MessageBoxW(window, message.c_str(), L"钢一定制AI - 获取模型", MB_OK | MB_ICONWARNING);
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 声明 switch 的分支入口：case WM_CLOSE:。
    case WM_CLOSE:
        // 【逐行批注】 调用 SetWindowTextW 完成本行所需操作：SetWindowTextW(g.apiKeyEdit, L"");。
        SetWindowTextW(g.apiKeyEdit, L"");
        // 【逐行批注】 判断条件 (g.hasConfig || g.skipAiSetup) ShowWindow(window, SW_HIDE); else…，成立时执行对应分支。
        if (g.hasConfig || g.skipAiSetup) ShowWindow(window, SW_HIDE); else DestroyWindow(window);
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 声明 switch 的分支入口：case WM_DESTROY:。
    case WM_DESTROY:
        // 【逐行批注】 计算右侧结果并写入 g.shuttingDown，更新当前运行状态。
        g.shuttingDown = true;
        // 【逐行批注】 判断条件 (g.desktopWindow) DestroyWindow(g.desktopWindow);，成立时执行对应分支。
        if (g.desktopWindow) DestroyWindow(g.desktopWindow);
        // 【逐行批注】 判断条件 (g.connectionTestThread.joinable()) g.connectionTestThread.join(…，成立时执行对应分支。
        if (g.connectionTestThread.joinable()) g.connectionTestThread.join();
        // 【逐行批注】 判断条件 (g.modelListThread.joinable()) g.modelListThread.join();，成立时执行对应分支。
        if (g.modelListThread.joinable()) g.modelListThread.join();
        // 【逐行批注】 调用 KillTimer 完成本行所需操作：KillTimer(window, kRestartTimer);。
        KillTimer(window, kRestartTimer);
        // 【逐行批注】 调用 KillTimer 完成本行所需操作：KillTimer(window, kStableTimer);。
        KillTimer(window, kStableTimer);
        // 【逐行批注】 调用 removeTrayIcon 完成本行所需操作：removeTrayIcon();。
        removeTrayIcon();
        // 【逐行批注】 调用 stopService 完成本行所需操作：stopService();。
        stopService();
        // 【逐行批注】 调用 PostQuitMessage 完成本行所需操作：PostQuitMessage(0);。
        PostQuitMessage(0);
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return DefWindowProcW(window, message, wParam, lParam);。
    return DefWindowProcW(window, message, wParam, lParam);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 selfTestNativeHttp，执行该函数声明的职责。
bool selfTestNativeHttp() {
    // 仅监听临时回环端口，真实走启动器原生请求，不读取配置、凭据或连接产品服务。
    // 【逐行批注】 声明数据结构或类型 SocketOwner 及其成员接口。
    struct SocketOwner {
        // 【逐行批注】 计算右侧结果并写入 SOCKET value，更新当前运行状态。
        SOCKET value = INVALID_SOCKET;
        // 【逐行批注】 调用 SocketOwner、closesocket 完成本行所需操作：~SocketOwner() { if (value != INVALID_SOCKET) closesocket(value); }。
        ~SocketOwner() { if (value != INVALID_SOCKET) closesocket(value); }
    // 【逐行批注】 调用 socket 完成本行所需操作：} listener{socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)};。
    } listener{socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)};
    // 【逐行批注】 判断条件 (listener.value == INVALID_SOCKET) return false;，成立时执行对应分支。
    if (listener.value == INVALID_SOCKET) return false;
    // 【逐行批注】 执行当前声明或表达式：sockaddr_in address{};。
    sockaddr_in address{};
    // 【逐行批注】 计算右侧结果并写入 address.sin_family，更新当前运行状态。
    address.sin_family = AF_INET;
    // 【逐行批注】 计算右侧结果并写入 address.sin_addr.s_addr，更新当前运行状态。
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    // 【逐行批注】 计算右侧结果并写入 address.sin_port，更新当前运行状态。
    address.sin_port = 0;
    // 【逐行批注】 判断条件 (bind(listener.value, reinterpret_cast<const sockaddr*>(&address…，成立时执行对应分支。
    if (bind(listener.value, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0 ||
        // 【逐行批注】 调用 listen 完成本行所需操作：listen(listener.value, 2) != 0) return false;。
        listen(listener.value, 2) != 0) return false;
    // 【逐行批注】 计算右侧结果并写入 int addressSize，更新当前运行状态。
    int addressSize = sizeof(address);
    // 【逐行批注】 判断条件 (getsockname(listener.value, reinterpret_cast<sockaddr*>(&addres…，成立时执行对应分支。
    if (getsockname(listener.value, reinterpret_cast<sockaddr*>(&address), &addressSize) != 0) return false;
    // 【逐行批注】 计算右侧结果并写入 int port，更新当前运行状态。
    const int port = ntohs(address.sin_port);
    // 【逐行批注】 计算右侧结果并写入 std::string host，更新当前运行状态。
    const std::string host = "\r\nHost: 127.0.0.1:" + std::to_string(port) + "\r\n";
    // 【逐行批注】 计算右侧结果并写入 constexpr wchar_t token[]，更新当前运行状态。
    constexpr wchar_t token[] = L"launcher-native-fixture-token";
    // 【逐行批注】 计算右侧结果并写入 int validRequests，更新当前运行状态。
    int validRequests = 0;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        std::thread server([&] {
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try {
                // 【逐行批注】 按循环规则重复处理：for (int index = 0; index < 2; ++index) {。
                for (int index = 0; index < 2; ++index) {
                    // 【逐行批注】 执行当前声明或表达式：fd_set incoming;。
                    fd_set incoming;
                    // 【逐行批注】 调用 FD_ZERO、FD_SET 完成本行所需操作：FD_ZERO(&incoming); FD_SET(listener.value, &incoming);。
                    FD_ZERO(&incoming); FD_SET(listener.value, &incoming);
                    // 【逐行批注】 执行当前声明或表达式：timeval acceptWait{2, 0};。
                    timeval acceptWait{2, 0};
                    // 【逐行批注】 判断条件 (select(0, &incoming, nullptr, nullptr, &acceptWait) != 1) retur…，成立时执行对应分支。
                    if (select(0, &incoming, nullptr, nullptr, &acceptWait) != 1) return;
                    // 【逐行批注】 调用 accept 完成本行所需操作：SocketOwner client{accept(listener.value, nullptr, nullptr)};。
                    SocketOwner client{accept(listener.value, nullptr, nullptr)};
                    // 【逐行批注】 判断条件 (client.value == INVALID_SOCKET) return;，成立时执行对应分支。
                    if (client.value == INVALID_SOCKET) return;
                    // 【逐行批注】 计算右侧结果并写入 DWORD timeout，更新当前运行状态。
                    const DWORD timeout = 1000;
                    // 【逐行批注】 调用 setsockopt 完成本行所需操作：setsockopt(client.value, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<co…。
                    setsockopt(client.value, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
                    // 【逐行批注】 调用 setsockopt 完成本行所需操作：setsockopt(client.value, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<co…。
                    setsockopt(client.value, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
                    // 【逐行批注】 执行当前声明或表达式：std::string request;。
                    std::string request;
                    // 【逐行批注】 计算右侧结果并写入 auto deadline，更新当前运行状态。
                    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
                    // 【逐行批注】 在条件成立期间循环处理：while (request.find("\r\n\r\n") == std::string::npos && request.size() < 8…。
                    while (request.find("\r\n\r\n") == std::string::npos && request.size() < 8192 &&
                           // 【逐行批注】 调用 now 完成本行所需操作：std::chrono::steady_clock::now() < deadline) {。
                           std::chrono::steady_clock::now() < deadline) {
                        // 【逐行批注】 执行当前声明或表达式：fd_set readable;。
                        fd_set readable;
                        // 【逐行批注】 调用 FD_ZERO、FD_SET 完成本行所需操作：FD_ZERO(&readable); FD_SET(client.value, &readable);。
                        FD_ZERO(&readable); FD_SET(client.value, &readable);
                        // 【逐行批注】 执行当前声明或表达式：timeval readWait{0, 100000};。
                        timeval readWait{0, 100000};
                        // 【逐行批注】 计算右侧结果并写入 int ready，更新当前运行状态。
                        const int ready = select(0, &readable, nullptr, nullptr, &readWait);
                        // 【逐行批注】 判断条件 (ready == SOCKET_ERROR) return;，成立时执行对应分支。
                        if (ready == SOCKET_ERROR) return;
                        // 【逐行批注】 判断条件 (ready == 0) continue;，成立时执行对应分支。
                        if (ready == 0) continue;
                        // 【逐行批注】 执行当前声明或表达式：std::array<char, 1024> buffer{};。
                        std::array<char, 1024> buffer{};
                        // 【逐行批注】 计算右侧结果并写入 int received，更新当前运行状态。
                        const int received = recv(client.value, buffer.data(), static_cast<int>(buffer.size()), 0);
                        // 【逐行批注】 判断条件 (received <= 0) break;，成立时执行对应分支。
                        if (received <= 0) break;
                        // 【逐行批注】 执行当前声明或表达式：request.append(buffer.data(), static_cast<std::size_t>(received));。
                        request.append(buffer.data(), static_cast<std::size_t>(received));
                    // 【逐行批注】 结束当前代码块。
                    }
                    // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
                    const bool valid = request.find("\r\n\r\n") != std::string::npos && request.find(host) != std::string::npos &&
                        // 【逐行批注】 执行当前声明或表达式：request.find("\r\nOrigin:") == std::string::npos &&。
                        request.find("\r\nOrigin:") == std::string::npos &&
                        // 【逐行批注】 执行当前声明或表达式：(index == 0 ? request.rfind("GET /health HTTP/1.1\r\n", 0) == 0 &&。
                        (index == 0 ? request.rfind("GET /health HTTP/1.1\r\n", 0) == 0 &&
                            // 【逐行批注】 执行当前声明或表达式：request.find("\r\nX-Gangyi-Control-Token:") == std::string::npos :。
                            request.find("\r\nX-Gangyi-Control-Token:") == std::string::npos :
                            // 【逐行批注】 执行当前声明或表达式：request.rfind("POST /internal/shutdown HTTP/1.1\r\n", 0) == 0 &&。
                            request.rfind("POST /internal/shutdown HTTP/1.1\r\n", 0) == 0 &&
                            // 【逐行批注】 执行当前声明或表达式：request.find("\r\nX-Gangyi-Control-Token: launcher-native-fixture-token\r\n") != std…。
                            request.find("\r\nX-Gangyi-Control-Token: launcher-native-fixture-token\r\n") != std::string::npos);
                    // 【逐行批注】 判断条件 (valid) ++validRequests;，成立时执行对应分支。
                    if (valid) ++validRequests;
                    // 【逐行批注】 计算右侧结果并写入 std::string response，更新当前运行状态。
                    const std::string response = valid ?
                        // 【逐行批注】 执行当前声明或表达式："HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\n{}" :。
                        "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\n{}" :
                        // 【逐行批注】 执行当前声明或表达式："HTTP/1.1 403 Forbidden\r\nContent-Length: 2\r\nConnection: close\r\n\r\n{}";。
                        "HTTP/1.1 403 Forbidden\r\nContent-Length: 2\r\nConnection: close\r\n\r\n{}";
                    // 【逐行批注】 计算右侧结果并写入 std::size_t sent，更新当前运行状态。
                    std::size_t sent = 0;
                    // 【逐行批注】 在条件成立期间循环处理：while (sent < response.size()) {。
                    while (sent < response.size()) {
                        // 【逐行批注】 计算右侧结果并写入 int count，更新当前运行状态。
                        const int count = send(client.value, response.data() + sent, static_cast<int>(response.size() - sent), 0);
                        // 【逐行批注】 判断条件 (count <= 0) return;，成立时执行对应分支。
                        if (count <= 0) return;
                        // 【逐行批注】 计算右侧结果并写入 sent +，更新当前运行状态。
                        sent += static_cast<std::size_t>(count);
                    // 【逐行批注】 结束当前代码块。
                    }
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 执行当前声明或表达式：} catch (...) { /* 假服务异常通过有效请求数反映，不能越过线程边界。 * / }。
            } catch (...) { /* 假服务异常通过有效请求数反映，不能越过线程边界。 */ }
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 声明数据结构或类型 JoinThread 及其成员接口。
        struct JoinThread {
            // 【逐行批注】 执行当前声明或表达式：std::thread& value;。
            std::thread& value;
            // 【逐行批注】 调用 JoinThread 完成本行所需操作：~JoinThread() { if (value.joinable()) value.join(); }。
            ~JoinThread() { if (value.joinable()) value.join(); }
        // 【逐行批注】 执行当前声明或表达式：} joined{server};。
        } joined{server};
        // 【逐行批注】 计算右侧结果并写入 bool health，更新当前运行状态。
        bool health = false, shutdown = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 health，更新当前运行状态。
            health = localHttpRequest(port, "GET", "/health");
            // 【逐行批注】 计算右侧结果并写入 shutdown，更新当前运行状态。
            shutdown = localHttpRequest(port, "POST", "/internal/shutdown", token);
        // 【逐行批注】 执行当前声明或表达式：} catch (...) { /* 退出前始终等待具有超时的假服务线程。 * / }。
        } catch (...) { /* 退出前始终等待具有超时的假服务线程。 */ }
        // 【逐行批注】 执行当前声明或表达式：server.join();。
        server.join();
        // 【逐行批注】 结束当前函数并返回结果：return health && shutdown && validRequests == 2;。
        return health && shutdown && validRequests == 2;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) { return false; }。
    } catch (...) { return false; }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 selfTest，执行该函数声明的职责。
int selfTest() {
    // 【逐行批注】 计算右侧结果并写入 std::wstring token，更新当前运行状态。
    const std::wstring token = randomToken();
    // 【逐行批注】 判断条件 (!validBaseUrl(L"https://api.deepseek.com/v1") || validBaseUrl(L…，成立时执行对应分支。
    if (!validBaseUrl(L"https://api.deepseek.com/v1") || validBaseUrl(L"file:///tmp/key")) return 10;
    // 【逐行批注】 判断条件 (token.size() != 64) return 11;，成立时执行对应分支。
    if (token.size() != 64) return 11;
    // 【逐行批注】 判断条件 (findAvailablePort() < 39002) return 12;，成立时执行对应分支。
    if (findAvailablePort() < 39002) return 12;
    // 【逐行批注】 判断条件 (localAppDataPath().empty()) return 13;，成立时执行对应分支。
    if (localAppDataPath().empty()) return 13;
    // 【逐行批注】 执行当前声明或表达式：Settings settings;。
    Settings settings;
    // 【逐行批注】 判断条件 (canStartService(settings, false, false) || !canStartService(set…，成立时执行对应分支。
    if (canStartService(settings, false, false) || !canStartService(settings, false, true) ||
        // 【逐行批注】 调用 canStartService 完成本行所需操作：!canStartService(settings, true, false)) return 14;。
        !canStartService(settings, true, false)) return 14;
    // 【逐行批注】 执行当前声明或表达式：settings.baseUrl.clear();。
    settings.baseUrl.clear();
    // 【逐行批注】 判断条件 (canStartService(settings, true, false) || !canStartService(sett…，成立时执行对应分支。
    if (canStartService(settings, true, false) || !canStartService(settings, false, true)) return 15;
    // 【逐行批注】 判断条件 (!selfTestNativeHttp()) return 16;，成立时执行对应分支。
    if (!selfTestNativeHttp()) return 16;
    // 【逐行批注】 结束当前函数并返回结果：return 0;。
    return 0;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 定义函数 wWinMain，执行该函数声明的职责。
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int) {
    // 【逐行批注】 调用 enablePerMonitorDpi 完成本行所需操作：enablePerMonitorDpi();。
    enablePerMonitorDpi();
    // 【逐行批注】 判断条件 (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) retu…，成立时执行对应分支。
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 1;
    // 【逐行批注】 执行当前声明或表达式：WSADATA sockets{};。
    WSADATA sockets{};
    // 【逐行批注】 判断条件 (WSAStartup(MAKEWORD(2, 2), &sockets) != 0) {，成立时执行对应分支。
    if (WSAStartup(MAKEWORD(2, 2), &sockets) != 0) {
        // 【逐行批注】 调用 CoUninitialize 完成本行所需操作：CoUninitialize();。
        CoUninitialize();
        // 【逐行批注】 结束当前函数并返回结果：return 1;。
        return 1;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 std::wstring arguments，更新当前运行状态。
    std::wstring arguments = commandLine ? commandLine : L"";
    // Start-Process 可能在参数末尾保留空格，命令模式仍应准确识别。
    // 【逐行批注】 计算右侧结果并写入 auto firstArgument，更新当前运行状态。
    const auto firstArgument = arguments.find_first_not_of(L" \t\r\n");
    // 【逐行批注】 计算右侧结果并写入 arguments，更新当前运行状态。
    arguments = firstArgument == std::wstring::npos ? L"" :
        // 【逐行批注】 执行当前声明或表达式：arguments.substr(firstArgument, arguments.find_last_not_of(L" \t\r\n") - firstArgume…。
        arguments.substr(firstArgument, arguments.find_last_not_of(L" \t\r\n") - firstArgument + 1);
    // 【逐行批注】 判断条件 (arguments.find(L"--remove-credentials") != std::wstring::npos) {，成立时执行对应分支。
    if (arguments.find(L"--remove-credentials") != std::wstring::npos) {
        // 【逐行批注】 调用 removeLocalSettings 完成本行所需操作：removeLocalSettings();。
        removeLocalSettings();
        // 【逐行批注】 调用 WSACleanup 完成本行所需操作：WSACleanup();。
        WSACleanup();
        // 【逐行批注】 调用 CoUninitialize 完成本行所需操作：CoUninitialize();。
        CoUninitialize();
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (arguments == L"--self-test") {，成立时执行对应分支。
    if (arguments == L"--self-test") {
        // 【逐行批注】 计算右侧结果并写入 int result，更新当前运行状态。
        const int result = selfTest();
        // 【逐行批注】 调用 WSACleanup 完成本行所需操作：WSACleanup();。
        WSACleanup();
        // 【逐行批注】 调用 CoUninitialize 完成本行所需操作：CoUninitialize();。
        CoUninitialize();
        // 【逐行批注】 结束当前函数并返回结果：return result;。
        return result;
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 计算右侧结果并写入 g.smokeMode，更新当前运行状态。
    g.smokeMode = arguments.find(L"--self-test-ui") != std::wstring::npos;
    // 【逐行批注】 计算右侧结果并写入 HANDLE mutex，更新当前运行状态。
    HANDLE mutex = CreateMutexW(nullptr, TRUE, g.smokeMode ? L"GangyiAI.Launcher.UI.Smoke" : kMutexName);
    // 【逐行批注】 判断条件 (!mutex || GetLastError() == ERROR_ALREADY_EXISTS) {，成立时执行对应分支。
    if (!mutex || GetLastError() == ERROR_ALREADY_EXISTS) {
        // 【逐行批注】 判断条件 (HWND existing = FindWindowW(kWindowClass, nullptr)) PostMessage…，成立时执行对应分支。
        if (HWND existing = FindWindowW(kWindowClass, nullptr)) PostMessageW(existing, kOpenDesktop, 0, 0);
        // 【逐行批注】 判断条件 (mutex) CloseHandle(mutex);，成立时执行对应分支。
        if (mutex) CloseHandle(mutex);
        // 【逐行批注】 调用 WSACleanup 完成本行所需操作：WSACleanup();。
        WSACleanup();
        // 【逐行批注】 调用 CoUninitialize 完成本行所需操作：CoUninitialize();。
        CoUninitialize();
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 计算右侧结果并写入 g.launchSessionId，更新当前运行状态。
    g.launchSessionId = randomToken();
    // 【逐行批注】 计算右侧结果并写入 std::wstring exe，更新当前运行状态。
    const std::wstring exe = executablePath();
    // 【逐行批注】 计算右侧结果并写入 g.installDir，更新当前运行状态。
    g.installDir = std::filesystem::path(exe).parent_path().wstring();
    // 【逐行批注】 计算右侧结果并写入 g.dataDir，更新当前运行状态。
    g.dataDir = (std::filesystem::path(localAppDataPath()) / kDataName).wstring();
// 【逐行批注】 处理条件编译指令：#if defined(GANGYI_TEACHING_PREVIEW) || defined(GANGYI_MY_AI_PREVIEW)。
#if defined(GANGYI_TEACHING_PREVIEW) || defined(GANGYI_MY_AI_PREVIEW)
    // 【逐行批注】 判断条件 (!g.smokeMode) {，成立时执行对应分支。
    if (!g.smokeMode) {
        // 【逐行批注】 计算右侧结果并写入 auto target，更新当前运行状态。
        const auto target = std::filesystem::path(g.dataDir) / L"data" / L"gangyiAI.db";
        // 【逐行批注】 计算右侧结果并写入 auto original，更新当前运行状态。
        const auto original = std::filesystem::path(localAppDataPath()) / L"GangyiAI" / L"data" / L"gangyiAI.db";
        // 复用已有 SQLite 备份机制，首次复制学习档案；正式档案仅作为只读来源。
        // 【逐行批注】 判断条件 (!std::filesystem::exists(target) && std::filesystem::exists(ori…，成立时执行对应分支。
        if (!std::filesystem::exists(target) && std::filesystem::exists(original)) {
            // 【逐行批注】 调用 create_directories 完成本行所需操作：std::error_code directoryError; std::filesystem::create_directories(t…。
            std::error_code directoryError; std::filesystem::create_directories(target.parent_path(), directoryError);
            // 【逐行批注】 执行当前声明或表达式：std::wstring backupError;。
            std::wstring backupError;
            // 【逐行批注】 判断条件 (directoryError || !gangyi::launcher::backupDatabase(original, t…，成立时执行对应分支。
            if (directoryError || !gangyi::launcher::backupDatabase(original, target, backupError)) {
                // 【逐行批注】 调用 MessageBoxW 完成本行所需操作：MessageBoxW(nullptr, (L"无法复制现有学习档案到试用目录：" + backupError).c_str(), L"电…。
                MessageBoxW(nullptr, (L"无法复制现有学习档案到试用目录：" + backupError).c_str(), L"电脑版试用版", MB_OK | MB_ICONERROR);
                // 【逐行批注】 调用 CloseHandle、WSACleanup、CoUninitialize 完成本行所需操作：CloseHandle(mutex); WSACleanup(); CoUninitialize(); return 1;。
                CloseHandle(mutex); WSACleanup(); CoUninitialize(); return 1;
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 调用 configure、toUtf8、path 完成本行所需操作：gangyi::AIActivity::configure(toUtf8((std::filesystem::path(g.dataDir…。
    gangyi::AIActivity::configure(toUtf8((std::filesystem::path(g.dataDir) / L"data" / L"ai-activity.json").wstring()));
    // 【逐行批注】 计算右侧结果并写入 g.background，更新当前运行状态。
    g.background = arguments.find(L"--background") != std::wstring::npos;
    // 【逐行批注】 计算右侧结果并写入 g.settings，更新当前运行状态。
    g.settings = g.smokeMode ? Settings{} : loadSettings();
    // 【逐行批注】 计算右侧结果并写入 std::wstring key，更新当前运行状态。
    std::wstring key = g.smokeMode ? std::wstring{} : readApiKey();
    // 【逐行批注】 计算右侧结果并写入 g.hasConfig，更新当前运行状态。
    g.hasConfig = validBaseUrl(g.settings.baseUrl) && !g.settings.model.empty() && !key.empty();
    // 【逐行批注】 计算右侧结果并写入 g.skipAiSetup，更新当前运行状态。
    g.skipAiSetup = g.smokeMode || readRegistryDword(L"SkipAISetup", 0) != 0;
    // 【逐行批注】 调用 SecureZeroMemory 完成本行所需操作：SecureZeroMemory(key.data(), key.size() * sizeof(wchar_t));。
    SecureZeroMemory(key.data(), key.size() * sizeof(wchar_t));

    // 【逐行批注】 计算右侧结果并写入 g.job，更新当前运行状态。
    g.job = CreateJobObjectW(nullptr, nullptr);
    // 【逐行批注】 判断条件 (g.job) {，成立时执行对应分支。
    if (g.job) {
        // 【逐行批注】 执行当前声明或表达式：JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};。
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        // 【逐行批注】 计算右侧结果并写入 limits.BasicLimitInformation.LimitFlags，更新当前运行状态。
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        // 【逐行批注】 调用 SetInformationJobObject 完成本行所需操作：SetInformationJobObject(g.job, JobObjectExtendedLimitInformation, &li…。
        SetInformationJobObject(g.job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 执行当前声明或表达式：WNDCLASSEXW windowClass{};。
    WNDCLASSEXW windowClass{};
    // 【逐行批注】 计算右侧结果并写入 windowClass.cbSize，更新当前运行状态。
    windowClass.cbSize = sizeof(windowClass);
    // 【逐行批注】 计算右侧结果并写入 windowClass.lpfnWndProc，更新当前运行状态。
    windowClass.lpfnWndProc = windowProc;
    // 【逐行批注】 计算右侧结果并写入 windowClass.hInstance，更新当前运行状态。
    windowClass.hInstance = instance;
    // 【逐行批注】 计算右侧结果并写入 windowClass.hIcon，更新当前运行状态。
    windowClass.hIcon = appIcon();
    // 【逐行批注】 计算右侧结果并写入 windowClass.hIconSm，更新当前运行状态。
    windowClass.hIconSm = appIcon();
    // 【逐行批注】 计算右侧结果并写入 windowClass.hCursor，更新当前运行状态。
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    // 【逐行批注】 计算右侧结果并写入 windowClass.hbrBackground，更新当前运行状态。
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    // 【逐行批注】 计算右侧结果并写入 windowClass.lpszClassName，更新当前运行状态。
    windowClass.lpszClassName = kWindowClass;
    // 【逐行批注】 判断条件 (!RegisterClassExW(&windowClass)) return 1;，成立时执行对应分支。
    if (!RegisterClassExW(&windowClass)) return 1;

    // 【逐行批注】 计算右侧结果并写入 WNDCLASSEXW desktopClass，更新当前运行状态。
    WNDCLASSEXW desktopClass = windowClass;
    // 【逐行批注】 计算右侧结果并写入 desktopClass.lpfnWndProc，更新当前运行状态。
    desktopClass.lpfnWndProc = desktopWindowProc;
    // 【逐行批注】 计算右侧结果并写入 desktopClass.lpszClassName，更新当前运行状态。
    desktopClass.lpszClassName = kDesktopWindowClass;
    // 【逐行批注】 判断条件 (!RegisterClassExW(&desktopClass)) return 1;，成立时执行对应分支。
    if (!RegisterClassExW(&desktopClass)) return 1;

    // 【逐行批注】 计算右侧结果并写入 g.window，更新当前运行状态。
    g.window = CreateWindowExW(0, kWindowClass, L"钢一定制AI 配置", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        // 【逐行批注】 执行当前声明或表达式：CW_USEDEFAULT, CW_USEDEFAULT, 520, 455, nullptr, nullptr, instance, nullptr);。
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 455, nullptr, nullptr, instance, nullptr);
    // 【逐行批注】 判断条件 (!g.window) return 1;，成立时执行对应分支。
    if (!g.window) return 1;
    // 【逐行批注】 判断条件 (g.hasConfig || g.skipAiSetup) {，成立时执行对应分支。
    if (g.hasConfig || g.skipAiSetup) {
        // 【逐行批注】 判断条件 (!startService(!g.background)) showSettingsWindow();，成立时执行对应分支。
        if (!startService(!g.background)) showSettingsWindow();
    // 【逐行批注】 执行当前声明或表达式：} else {。
    } else {
        // 【逐行批注】 调用 showSettingsWindow 完成本行所需操作：showSettingsWindow();。
        showSettingsWindow();
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 执行当前声明或表达式：MSG message{};。
    MSG message{};
    // 【逐行批注】 在条件成立期间循环处理：while (GetMessageW(&message, nullptr, 0, 0) > 0) {。
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        // 【逐行批注】 调用 TranslateMessage 完成本行所需操作：TranslateMessage(&message);。
        TranslateMessage(&message);
        // 【逐行批注】 调用 DispatchMessageW 完成本行所需操作：DispatchMessageW(&message);。
        DispatchMessageW(&message);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (g.job) CloseHandle(g.job);，成立时执行对应分支。
    if (g.job) CloseHandle(g.job);
    // 【逐行批注】 调用 ReleaseMutex 完成本行所需操作：ReleaseMutex(mutex);。
    ReleaseMutex(mutex);
    // 【逐行批注】 调用 CloseHandle 完成本行所需操作：CloseHandle(mutex);。
    CloseHandle(mutex);
    // 【逐行批注】 调用 WSACleanup 完成本行所需操作：WSACleanup();。
    WSACleanup();
    // 【逐行批注】 调用 CoUninitialize 完成本行所需操作：CoUninitialize();。
    CoUninitialize();
    // 【逐行批注】 结束当前函数并返回结果：return static_cast<int>(message.wParam);。
    return static_cast<int>(message.wParam);
// 【逐行批注】 结束当前代码块。
}

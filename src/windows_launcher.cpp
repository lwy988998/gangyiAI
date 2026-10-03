#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>
#include <bcrypt.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <wincred.h>
#include <webview/webview.h>

#include "ai_client.hpp"
#include "search_client.hpp"
#include "launcher_support.hpp"
#include "version.hpp"
#include "windows_resource.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cwctype>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

constexpr wchar_t kWindowClass[] = L"GangyiAILauncherWindow";
constexpr wchar_t kDesktopWindowClass[] = L"GangyiAIDesktopWindow";
constexpr wchar_t kMutexName[] = L"GangyiAI.Launcher.v1";
constexpr wchar_t kRegistryKey[] = L"Software\\GangyiAI";
constexpr wchar_t kCredentialTarget[] = L"GangyiAI/APIKey";
constexpr wchar_t kBochaCredentialTarget[] = L"GangyiAI/BochaKey";
constexpr UINT kTrayMessage = WM_APP + 1;
constexpr UINT kServiceReady = WM_APP + 2;
constexpr UINT kServiceFailed = WM_APP + 3;
constexpr UINT kOpenDesktop = WM_APP + 4;
constexpr UINT kOpenApiSettings = WM_APP + 8;
constexpr UINT kConnectionTestComplete = WM_APP + 5;
constexpr UINT kModelListComplete = WM_APP + 6;
constexpr UINT kRestartTimer = 1;
constexpr UINT kStableTimer = 2;
constexpr UINT kDesktopLoadTimer = 3;

constexpr int kBaseUrlEdit = 101;
constexpr int kApiKeyEdit = 102;
constexpr int kModelEdit = 103;
constexpr int kAutoStartCheck = 104;
constexpr int kSaveButton = 105;
constexpr int kCancelButton = 106;
constexpr int kTestButton = 107;
constexpr int kProviderCombo = 108;
constexpr int kFetchModelsButton = 109;
constexpr int kSkipAiButton = 110;
constexpr int kBochaKeyEdit = 111;
constexpr int kBochaTestButton = 112;
constexpr int kMenuOpen = 201;
constexpr int kMenuSettings = 202;
constexpr int kMenuRestart = 203;
constexpr int kMenuExit = 204;
constexpr int kMenuViewLog = 205;
constexpr int kMenuOpenData = 206;
constexpr int kMenuBackup = 207;
constexpr int kMenuRestore = 208;
constexpr int kMenuDiagnostic = 209;

struct Settings {
    std::wstring baseUrl = L"https://api.deepseek.com";
    std::wstring model = L"deepseek-v4-flash";
    gangyi::launcher::AIProvider provider = gangyi::launcher::AIProvider::DeepSeek;
    bool autoStart = false;
};

struct AppState {
    HWND window = nullptr;
    HWND desktopWindow = nullptr;
    webview_t desktopView = nullptr;
    ICoreWebView2* browser = nullptr;
    EventRegistrationToken navigationToken{};
    EventRegistrationToken newWindowToken{};
    bool pageReady = false;
    std::wstring desktopOrigin;
    // 启动时先播放一次品牌动画，动画结束后由页面优雅回到首页。
    std::wstring desktopRoute = L"/startup";
    int displayPort = 0;
    HWND baseUrlEdit = nullptr;
    HWND apiKeyEdit = nullptr;
    HWND bochaKeyEdit = nullptr;
    HWND bochaTestButton = nullptr;
    HWND modelEdit = nullptr;
    HWND providerCombo = nullptr;
    HWND fetchModelsButton = nullptr;
    HWND autoStartCheck = nullptr;
    HWND testButton = nullptr;
    HWND saveButton = nullptr;
    HWND statusLabel = nullptr;
    HANDLE process = nullptr;
    HANDLE job = nullptr;
    std::thread healthThread;
    std::thread connectionTestThread;
    std::thread modelListThread;
    std::atomic<bool> cancelHealth{false};
    std::atomic<bool> connectionTestRunning{false};
    std::atomic<bool> modelListRunning{false};
    std::atomic<bool> shuttingDown{false};
    std::mutex connectionTestMutex;
    std::wstring connectionTestMessage;
    bool connectionTestSucceeded = false;
    std::mutex modelListMutex;
    std::wstring modelListMessage;
    std::vector<std::wstring> availableModels;
    bool modelListSucceeded = false;
    Settings settings;
    std::wstring installDir;
    std::wstring dataDir;
    std::wstring controlToken;
    std::wstring launchSessionId;
    int port = 0;
    bool hasConfig = false;
    bool skipAiSetup = false;
    bool smokeMode = false;
    bool background = false;
    bool openWhenReady = true;
    DWORD lastExitCode = 0;
    gangyi::launcher::RestartPolicy restartPolicy;
    std::filesystem::path pendingRestoreRollback;
};

AppState g;

void enablePerMonitorDpi() {
    // 高 DPI 屏幕由系统按每个显示器缩放，避免 WebView2 被 DPI 虚拟化后变模糊。
    using SetProcessDpiAwarenessContextFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
    const auto context = reinterpret_cast<DPI_AWARENESS_CONTEXT>(static_cast<INT_PTR>(-4));
    const auto user32 = GetModuleHandleW(L"user32.dll");
    const auto setContext = user32 ? reinterpret_cast<SetProcessDpiAwarenessContextFn>(
        GetProcAddress(user32, "SetProcessDpiAwarenessContext")) : nullptr;
    if (!setContext || !setContext(context)) SetProcessDPIAware();
}

std::wstring readRegistryString(const wchar_t* name, const std::wstring& fallback = {}) {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, KEY_READ, &key) != ERROR_SUCCESS) return fallback;
    DWORD type = 0;
    DWORD bytes = 0;
    if (RegQueryValueExW(key, name, nullptr, &type, nullptr, &bytes) != ERROR_SUCCESS || type != REG_SZ) {
        RegCloseKey(key);
        return fallback;
    }
    std::vector<wchar_t> value(bytes / sizeof(wchar_t) + 1, L'\0');
    const LONG result = RegQueryValueExW(key, name, nullptr, nullptr,
        reinterpret_cast<BYTE*>(value.data()), &bytes);
    RegCloseKey(key);
    return result == ERROR_SUCCESS ? std::wstring(value.data()) : fallback;
}

DWORD readRegistryDword(const wchar_t* name, DWORD fallback) {
    HKEY key = nullptr;
    DWORD value = fallback;
    DWORD type = 0;
    DWORD bytes = sizeof(value);
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, KEY_READ, &key) == ERROR_SUCCESS) {
        if (RegQueryValueExW(key, name, nullptr, &type, reinterpret_cast<BYTE*>(&value), &bytes) != ERROR_SUCCESS ||
            type != REG_DWORD) value = fallback;
        RegCloseKey(key);
    }
    return value;
}

bool writeRegistryString(const wchar_t* name, const std::wstring& value) {
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS)
        return false;
    const LONG result = RegSetValueExW(key, name, 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()),
        static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(key);
    return result == ERROR_SUCCESS;
}

bool writeRegistryDword(const wchar_t* name, DWORD value) {
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS)
        return false;
    const LONG result = RegSetValueExW(key, name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value));
    RegCloseKey(key);
    return result == ERROR_SUCCESS;
}

std::wstring readCredential(const wchar_t* target) {
    PCREDENTIALW credential = nullptr;
    if (!CredReadW(target, CRED_TYPE_GENERIC, 0, &credential)) return {};
    std::wstring value;
    if (credential->CredentialBlob && credential->CredentialBlobSize % sizeof(wchar_t) == 0) {
        value.assign(reinterpret_cast<const wchar_t*>(credential->CredentialBlob),
            credential->CredentialBlobSize / sizeof(wchar_t));
    }
    CredFree(credential);
    return value;
}

bool writeCredential(const wchar_t* target, const std::wstring& value) {
    if (value.empty() || value.size() * sizeof(wchar_t) > CRED_MAX_CREDENTIAL_BLOB_SIZE) return false;
    CREDENTIALW credential{};
    credential.Type = CRED_TYPE_GENERIC;
    credential.TargetName = const_cast<wchar_t*>(target);
    credential.CredentialBlobSize = static_cast<DWORD>(value.size() * sizeof(wchar_t));
    credential.CredentialBlob = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(value.data()));
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    credential.UserName = const_cast<wchar_t*>(L"gangyiAI");
    return CredWriteW(&credential, 0) == TRUE;
}
std::wstring readApiKey() { return readCredential(kCredentialTarget); }
bool writeApiKey(const std::wstring& value) { return writeCredential(kCredentialTarget, value); }

std::wstring executablePath() {
    std::vector<wchar_t> path(32768, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    return std::wstring(path.data(), length);
}

std::wstring localAppDataPath() {
    const DWORD required = GetEnvironmentVariableW(L"LOCALAPPDATA", nullptr, 0);
    if (required > 1) {
        std::vector<wchar_t> value(required, L'\0');
        if (GetEnvironmentVariableW(L"LOCALAPPDATA", value.data(), required) > 0) return value.data();
    }
    std::array<wchar_t, MAX_PATH> fallback{};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, nullptr,
        SHGFP_TYPE_CURRENT, fallback.data()))) return fallback.data();
    return {};
}

Settings loadSettings() {
    Settings settings;
    settings.baseUrl = readRegistryString(L"AIBaseUrl", settings.baseUrl);
    settings.model = readRegistryString(L"AIModel", settings.model);
    const std::wstring providerId = readRegistryString(L"AIProvider");
    settings.provider = providerId.empty() ? gangyi::launcher::inferProvider(settings.baseUrl)
                                           : gangyi::launcher::providerFromId(providerId);
    if (settings.provider == gangyi::launcher::AIProvider::DeepSeek && settings.model == L"deepseek-chat") {
        settings.model = L"deepseek-v4-flash";
        writeRegistryString(L"AIModel", settings.model);
    } else if (settings.provider == gangyi::launcher::AIProvider::DeepSeek && settings.model == L"deepseek-reasoner") {
        settings.model = L"deepseek-v4-pro";
        writeRegistryString(L"AIModel", settings.model);
    }
    settings.autoStart = readRegistryDword(L"AutoStart", 0) != 0;
    return settings;
}

int providerIndex(gangyi::launcher::AIProvider provider) {
    switch (provider) {
    case gangyi::launcher::AIProvider::DeepSeek: return 0;
    case gangyi::launcher::AIProvider::OpenAI: return 1;
    default: return 2;
    }
}

gangyi::launcher::AIProvider selectedProvider() {
    const LRESULT index = SendMessageW(g.providerCombo, CB_GETCURSEL, 0, 0);
    if (index == 0) return gangyi::launcher::AIProvider::DeepSeek;
    if (index == 1) return gangyi::launcher::AIProvider::OpenAI;
    return gangyi::launcher::AIProvider::Custom;
}

bool configureAutoStart(bool enabled) {
    HKEY key = nullptr;
    constexpr wchar_t runKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKeyExW(HKEY_CURRENT_USER, runKey, 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS) return false;
    LONG result = ERROR_SUCCESS;
    if (enabled) {
        const std::wstring command = L"\"" + executablePath() + L"\" --background";
        result = RegSetValueExW(key, L"GangyiAI", 0, REG_SZ, reinterpret_cast<const BYTE*>(command.c_str()),
            static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t)));
    } else {
        result = RegDeleteValueW(key, L"GangyiAI");
        if (result == ERROR_FILE_NOT_FOUND) result = ERROR_SUCCESS;
    }
    RegCloseKey(key);
    return result == ERROR_SUCCESS;
}

void removeLocalSettings() {
    configureAutoStart(false);
    CredDeleteW(kCredentialTarget, CRED_TYPE_GENERIC, 0);
    CredDeleteW(kBochaCredentialTarget, CRED_TYPE_GENERIC, 0);
    RegDeleteTreeW(HKEY_CURRENT_USER, kRegistryKey);
}

bool startsWithIgnoreCase(const std::wstring& value, const wchar_t* prefix) {
    const size_t length = wcslen(prefix);
    return value.size() >= length && _wcsnicmp(value.c_str(), prefix, length) == 0;
}

bool validBaseUrl(const std::wstring& value) {
    return startsWithIgnoreCase(value, L"https://") || startsWithIgnoreCase(value, L"http://");
}

bool canStartService(const Settings& settings, bool hasApiKey, bool skipAiSetup) {
    return skipAiSetup || (validBaseUrl(settings.baseUrl) && !settings.model.empty() && hasApiKey);
}

std::string toUtf8(const std::wstring& value) {
    if (value.empty()) return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};
    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        result.data(), size, nullptr, nullptr);
    return result;
}

std::wstring toWide(const std::string& value) {
    if (value.empty()) return {};
    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (size <= 0) return {};
    std::wstring result(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
    return result;
}

std::wstring controlText(HWND control) {
    const int length = GetWindowTextLengthW(control);
    std::vector<wchar_t> value(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(control, value.data(), static_cast<int>(value.size()));
    return value.data();
}

std::wstring randomToken() {
    std::array<unsigned char, 32> bytes{};
    if (BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(bytes.size()), BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0)
        return {};
    constexpr wchar_t hex[] = L"0123456789abcdef";
    std::wstring token;
    token.reserve(bytes.size() * 2);
    for (const unsigned char value : bytes) {
        token.push_back(hex[value >> 4]);
        token.push_back(hex[value & 0x0f]);
    }
    return token;
}

int findAvailablePort() {
    for (int port = 39002; port <= 39099; ++port) {
        SOCKET candidate = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (candidate == INVALID_SOCKET) return 0;
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(static_cast<u_short>(port));
        const bool available = bind(candidate, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == 0;
        closesocket(candidate);
        if (available) return port;
    }
    return 0;
}

bool processRunning() {
    if (!g.process) return false;
    DWORD exitCode = 0;
    return GetExitCodeProcess(g.process, &exitCode) && exitCode == STILL_ACTIVE;
}

bool localHttpRequest(int port, const char* method, const char* path, const std::wstring& token = {}) {
    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET) return false;
    DWORD timeout = 1000;
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(static_cast<u_short>(port));
    if (connect(client, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0) {
        closesocket(client);
        return false;
    }
    std::string request = std::string(method) + " " + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n";
    if (!token.empty()) {
        std::string narrowToken;
        narrowToken.reserve(token.size());
        for (const wchar_t character : token) narrowToken.push_back(static_cast<char>(character));
        request += "X-Gangyi-Control-Token: " + narrowToken + "\r\n";
    }
    request += "Content-Length: 0\r\n\r\n";
    if (send(client, request.data(), static_cast<int>(request.size()), 0) == SOCKET_ERROR) {
        closesocket(client);
        return false;
    }
    std::array<char, 512> response{};
    const int received = recv(client, response.data(), static_cast<int>(response.size() - 1), 0);
    closesocket(client);
    return received > 0 && std::string(response.data(), static_cast<size_t>(received)).find(" 200 ") != std::string::npos;
}

std::vector<wchar_t> childEnvironment(const std::vector<std::pair<std::wstring, std::wstring>>& overrides) {
    std::vector<std::wstring> entries;
    LPWCH environment = GetEnvironmentStringsW();
    if (environment) {
        for (const wchar_t* cursor = environment; *cursor; cursor += wcslen(cursor) + 1) {
            bool replaced = false;
            for (const auto& [name, value] : overrides) {
                const size_t equals = std::wstring(cursor).find(L'=', cursor[0] == L'=' ? 1 : 0);
                if (equals != std::wstring::npos && equals == name.size() &&
                    _wcsnicmp(cursor, name.c_str(), name.size()) == 0) {
                    replaced = true;
                    break;
                }
            }
            if (!replaced) entries.emplace_back(cursor);
        }
        FreeEnvironmentStringsW(environment);
    }
    for (const auto& [name, value] : overrides) entries.push_back(name + L"=" + value);
    std::sort(entries.begin(), entries.end(), [](const std::wstring& left, const std::wstring& right) {
        return _wcsicmp(left.c_str(), right.c_str()) < 0;
    });
    size_t size = 1;
    for (const auto& entry : entries) size += entry.size() + 1;
    std::vector<wchar_t> block(size, L'\0');
    wchar_t* output = block.data();
    for (const auto& entry : entries) {
        std::copy(entry.begin(), entry.end(), output);
        output += entry.size() + 1;
    }
    return block;
}

void updateTrayTip(const wchar_t* text) {
    NOTIFYICONDATAW icon{};
    icon.cbSize = sizeof(icon);
    icon.hWnd = g.window;
    icon.uID = 1;
    icon.uFlags = NIF_TIP;
    wcsncpy_s(icon.szTip, ARRAYSIZE(icon.szTip), text, _TRUNCATE);
    Shell_NotifyIconW(NIM_MODIFY, &icon);
}

void showFailure(const wchar_t* message) {
    updateTrayTip(L"钢一定制AI - 启动失败");
    if (g.background) {
        NOTIFYICONDATAW icon{};
        icon.cbSize = sizeof(icon);
        icon.hWnd = g.window;
        icon.uID = 1;
        icon.uFlags = NIF_INFO;
        wcsncpy_s(icon.szInfoTitle, ARRAYSIZE(icon.szInfoTitle), L"钢一定制AI 启动失败", _TRUNCATE);
        wcsncpy_s(icon.szInfo, ARRAYSIZE(icon.szInfo), message, _TRUNCATE);
        icon.dwInfoFlags = NIIF_ERROR;
        Shell_NotifyIconW(NIM_MODIFY, &icon);
    } else {
        MessageBoxW(g.window, message, L"钢一定制AI", MB_OK | MB_ICONERROR);
    }
}

bool isExternalWebUrl(const std::wstring& url) {
    if (!startsWithIgnoreCase(url, L"https://") && !startsWithIgnoreCase(url, L"http://")) return false;
    const size_t authority = url.find(L"://") + 3;
    const size_t end = url.find_first_of(L"/?#", authority);
    const std::wstring address = url.substr(authority, end - authority);
    if (address.empty()) return false;
    const size_t port = address.front() == L'[' ? address.find(L']') + 1 : address.find(L':');
    const std::wstring host = address.substr(0, port);
    return _wcsicmp(host.c_str(), L"127.0.0.1") != 0 &&
           _wcsicmp(host.c_str(), L"localhost") != 0 &&
           _wcsicmp(host.c_str(), L"[::1]") != 0;
}

bool isCurrentLocalUrl(const std::wstring& url) {
    if (url.compare(0, g.desktopOrigin.size(), g.desktopOrigin) != 0) return false;
    return url.size() == g.desktopOrigin.size() || url[g.desktopOrigin.size()] == L'/' ||
           url[g.desktopOrigin.size()] == L'?' || url[g.desktopOrigin.size()] == L'#';
}

void openExternalUrl(const std::wstring& url) {
    if (isExternalWebUrl(url)) ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

class NavigationHandler final : public ICoreWebView2NavigationStartingEventHandler {
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** result) override {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (IsEqualIID(id, IID_IUnknown) || IsEqualIID(id, IID_ICoreWebView2NavigationStartingEventHandler)) {
            *result = this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++refs_; }
    ULONG STDMETHODCALLTYPE Release() override {
        const ULONG remaining = --refs_;
        if (!remaining) delete this;
        return remaining;
    }
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) override {
        LPWSTR raw = nullptr;
        if (FAILED(args->get_Uri(&raw)) || !raw) return S_OK;
        const std::wstring url(raw);
        CoTaskMemFree(raw);
        if (url == L"about:blank") return S_OK;
        if (isCurrentLocalUrl(url)) {
            g.desktopRoute = url.substr(g.desktopOrigin.size());
            if (g.desktopRoute.empty()) g.desktopRoute = L"/";
            g.pageReady = false;
            if (g.desktopWindow) SetTimer(g.desktopWindow, kDesktopLoadTimer, 15000, nullptr);
        } else {
            args->put_Cancel(TRUE);
            openExternalUrl(url);
        }
        return S_OK;
    }
private:
    std::atomic<ULONG> refs_{1};
};

class NewWindowHandler final : public ICoreWebView2NewWindowRequestedEventHandler {
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** result) override {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (IsEqualIID(id, IID_IUnknown) || IsEqualIID(id, IID_ICoreWebView2NewWindowRequestedEventHandler)) {
            *result = this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++refs_; }
    ULONG STDMETHODCALLTYPE Release() override {
        const ULONG remaining = --refs_;
        if (!remaining) delete this;
        return remaining;
    }
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* args) override {
        LPWSTR raw = nullptr;
        if (SUCCEEDED(args->get_Uri(&raw)) && raw) {
            if (isCurrentLocalUrl(raw) && g.browser) g.browser->Navigate(raw);
            else openExternalUrl(raw);
            CoTaskMemFree(raw);
        }
        args->put_Handled(TRUE);
        return S_OK;
    }
private:
    std::atomic<ULONG> refs_{1};
};

void resizeDesktopWidget(HWND window) {
    if (!g.desktopView) return;
    auto* widget = static_cast<HWND>(webview_get_native_handle(g.desktopView, WEBVIEW_NATIVE_HANDLE_KIND_UI_WIDGET));
    RECT bounds{};
    if (widget && GetClientRect(window, &bounds))
        MoveWindow(widget, 0, 0, bounds.right, bounds.bottom, TRUE);
}

void desktopPageReady(const char* id, const char*, void*) {
    g.pageReady = true;
    if (g.desktopWindow) KillTimer(g.desktopWindow, kDesktopLoadTimer);
    webview_return(g.desktopView, id, 0, "null");
}

void openApiSettings(const char* id, const char*, void*) {
    if (!g.window || !PostMessageW(g.window, kOpenApiSettings, 0, 0)) {
        webview_return(g.desktopView, id, 1, "\"无法打开 API 接口设置，请从托盘菜单进入设置。\"");
        return;
    }
    webview_return(g.desktopView, id, 0, "null");
}

const char* kDesktopScript = R"JS((() => {
  const report = () => {
    const header = document.querySelector('.site-header');
    if (header && header.getBoundingClientRect().width > 0 &&
        document.documentElement.clientWidth > 0 && document.body.innerText.trim()) {
      window.gangyiPageReady();
    } else {
      setTimeout(report, 200);
    }
  };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', report, {once: true});
  else report();
})())JS";

void closeDesktopView() {
    if (g.desktopWindow) KillTimer(g.desktopWindow, kDesktopLoadTimer);
    if (g.browser) {
        g.browser->remove_NavigationStarting(g.navigationToken);
        g.browser->remove_NewWindowRequested(g.newWindowToken);
        g.browser->Release();
        g.browser = nullptr;
    }
    if (g.desktopView) {
        webview_destroy(g.desktopView);
        g.desktopView = nullptr;
    }
}

LRESULT CALLBACK desktopWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        resizeDesktopWidget(window);
        return 0;
    case WM_MOVE:
        if (g.desktopView) {
            auto* controller = static_cast<ICoreWebView2Controller*>(webview_get_native_handle(
                g.desktopView, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER));
            if (controller) controller->NotifyParentWindowPositionChanged();
        }
        return 0;
    case WM_DPICHANGED: {
        const auto* suggested = reinterpret_cast<const RECT*>(lParam);
        if (suggested) SetWindowPos(window, nullptr, suggested->left, suggested->top,
            suggested->right - suggested->left, suggested->bottom - suggested->top,
            SWP_NOZORDER | SWP_NOACTIVATE);
        resizeDesktopWidget(window);
        return 0;
    }
    case WM_TIMER:
        if (wParam == kDesktopLoadTimer && !g.pageReady) {
            KillTimer(window, kDesktopLoadTimer);
            const int answer = MessageBoxW(window,
                L"页面未能正常显示。请检查本地服务，然后重试；也可以退出程序。",
                L"钢一定制AI - 加载失败", MB_RETRYCANCEL | MB_ICONWARNING);
            if (answer == IDRETRY) {
                SetTimer(window, kDesktopLoadTimer, 15000, nullptr);
                webview_navigate(g.desktopView, toUtf8(g.desktopOrigin + g.desktopRoute).c_str());
            } else PostMessageW(window, WM_CLOSE, 0, 0);
            return 0;
        }
        break;
    case WM_CLOSE:
        closeDesktopView();
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        g.desktopWindow = nullptr;
        g.displayPort = 0;
        if (g.window && !g.shuttingDown) DestroyWindow(g.window);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

void openDesktop() {
    if (!g.port) return;
    if (!g.desktopWindow) {
        RECT workArea{};
        if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0))
            SetRect(&workArea, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        const int width = static_cast<int>(std::min<LONG>(1120, (workArea.right - workArea.left) * 9 / 10));
        const int height = static_cast<int>(std::min<LONG>(760, (workArea.bottom - workArea.top) * 9 / 10));
        g.desktopWindow = CreateWindowExW(0, kDesktopWindowClass, L"钢一定制AI", WS_OVERLAPPEDWINDOW,
            workArea.left + (workArea.right - workArea.left - width) / 2,
            workArea.top + (workArea.bottom - workArea.top - height) / 2,
            width, height, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!g.desktopWindow) { showFailure(L"无法创建桌面主窗口。"); return; }
        g.desktopView = webview_create(0, g.desktopWindow);
        if (!g.desktopView) {
            const int answer = MessageBoxW(g.desktopWindow,
                L"无法启动独立界面。请安装 Microsoft Edge WebView2 Runtime。是否打开官方下载页？",
                L"钢一定制AI", MB_YESNO | MB_ICONERROR);
            if (answer == IDYES) ShellExecuteW(nullptr, L"open",
                L"https://developer.microsoft.com/microsoft-edge/webview2/", nullptr, nullptr, SW_SHOWNORMAL);
            DestroyWindow(g.desktopWindow);
            DestroyWindow(g.window);
            return;
        }
        webview_bind(g.desktopView, "gangyiPageReady", desktopPageReady, nullptr);
        webview_bind(g.desktopView, "gangyiOpenApiSettings", openApiSettings, nullptr);
        webview_init(g.desktopView, kDesktopScript);
        auto* controller = static_cast<ICoreWebView2Controller*>(webview_get_native_handle(
            g.desktopView, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER));
        if (!controller || FAILED(controller->get_CoreWebView2(&g.browser)) || !g.browser) {
            showFailure(L"无法初始化桌面页面。");
            closeDesktopView();
            DestroyWindow(g.desktopWindow);
            DestroyWindow(g.window);
            return;
        }
        auto* navigation = new NavigationHandler();
        g.browser->add_NavigationStarting(navigation, &g.navigationToken);
        navigation->Release();
        auto* newWindow = new NewWindowHandler();
        g.browser->add_NewWindowRequested(newWindow, &g.newWindowToken);
        newWindow->Release();
        resizeDesktopWidget(g.desktopWindow);
        ShowWindow(g.desktopWindow, SW_SHOW);
    }
    if (g.displayPort != g.port) {
        if (g.browser && !g.desktopOrigin.empty()) {
            LPWSTR current = nullptr;
            if (SUCCEEDED(g.browser->get_Source(&current)) && current) {
                const std::wstring source(current);
                if (isCurrentLocalUrl(source)) g.desktopRoute = source.substr(g.desktopOrigin.size());
                CoTaskMemFree(current);
            }
        }
        g.desktopOrigin = L"http://127.0.0.1:" + std::to_wstring(g.port);
        g.displayPort = g.port;
        if (g.desktopRoute.empty() || g.desktopRoute.front() != L'/') g.desktopRoute = L"/";
        g.pageReady = false;
        SetTimer(g.desktopWindow, kDesktopLoadTimer, 15000, nullptr);
        webview_navigate(g.desktopView, toUtf8(g.desktopOrigin + g.desktopRoute).c_str());
    }
    ShowWindow(g.desktopWindow, IsIconic(g.desktopWindow) ? SW_RESTORE : SW_SHOW);
    SetForegroundWindow(g.desktopWindow);
}

void stopService() {
    g.cancelHealth = true;
    if (g.healthThread.joinable()) g.healthThread.join();
    if (!g.process) return;
    if (processRunning()) {
        localHttpRequest(g.port, "POST", "/internal/shutdown", g.controlToken);
        if (WaitForSingleObject(g.process, 5000) == WAIT_TIMEOUT) TerminateProcess(g.process, 1);
    }
    CloseHandle(g.process);
    g.process = nullptr;
    g.port = 0;
    g.controlToken.clear();
}

bool startService(bool openWhenReady, bool resetRestart = true) {
    stopService();
    if (resetRestart) {
        g.restartPolicy.reset();
        KillTimer(g.window, kRestartTimer);
        KillTimer(g.window, kStableTimer);
    }
    g.settings = g.smokeMode ? Settings{} : loadSettings();
    std::wstring apiKey = g.smokeMode ? std::wstring{} : readApiKey();
    if (!canStartService(g.settings, !apiKey.empty(), g.skipAiSetup)) {
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        return false;
    }
    if (g.skipAiSetup) {
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        apiKey.clear();
    }
    g.port = findAvailablePort();
    g.controlToken = randomToken();
    if (!g.port || g.controlToken.empty()) {
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        showFailure(L"找不到可用端口，或无法生成本地控制令牌。");
        return false;
    }

    const std::filesystem::path serverPath = std::filesystem::path(g.installDir) / L"gangyiAI.exe";
    const std::filesystem::path databasePath = std::filesystem::path(g.dataDir) / L"data" / L"gangyiAI.db";
    const std::filesystem::path logDir = std::filesystem::path(g.dataDir) / L"logs";
    std::error_code error;
    std::filesystem::create_directories(databasePath.parent_path(), error);
    if (error) {
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        showFailure(L"无法创建本地数据库目录，请检查当前用户权限。");
        return false;
    }
    std::filesystem::create_directories(logDir, error);
    if (error || !std::filesystem::is_regular_file(serverPath)) {
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        showFailure(L"程序文件或本地数据目录不可用，请重新安装。");
        return false;
    }

    std::wstring bochaKey = g.smokeMode ? std::wstring{} : readCredential(kBochaCredentialTarget);
    std::vector<std::pair<std::wstring, std::wstring>> overrides = {
        {L"HOST", L"127.0.0.1"},
        {L"PORT", std::to_wstring(g.port)},
        {L"AI_BASE_URL", g.settings.baseUrl},
        {L"AI_API_KEY", apiKey},
        {L"AI_MODEL", g.settings.model},
        {L"BOCHA_API_KEY", bochaKey},
        {L"DATABASE_PATH", databasePath.wstring()},
        {L"LOCAL_CONTROL_TOKEN", g.controlToken},
        {L"GANGYI_LAUNCH_SESSION_ID", g.launchSessionId},
    };
    const std::filesystem::path caBundle = std::filesystem::path(g.installDir) / L"curl-ca-bundle.crt";
    if (std::filesystem::is_regular_file(caBundle)) overrides.emplace_back(L"CURL_CA_BUNDLE", caBundle.wstring());
    auto environment = childEnvironment(overrides);
    SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
    SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));
    for (auto& [name, value] : overrides) if (name == L"AI_API_KEY" || name == L"BOCHA_API_KEY")
        SecureZeroMemory(value.data(), value.size() * sizeof(wchar_t));

    SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
    const std::filesystem::path logPath = logDir / L"gangyiAI.log";
    gangyi::launcher::rotateLogFiles(logPath);
    HANDLE log = CreateFileW(logPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    HANDLE nullInput = INVALID_HANDLE_VALUE;
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    if (log != INVALID_HANDLE_VALUE) {
        nullInput = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
            &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        startup.dwFlags |= STARTF_USESTDHANDLES;
        startup.hStdOutput = log;
        startup.hStdError = log;
        startup.hStdInput = nullInput == INVALID_HANDLE_VALUE ? nullptr : nullInput;
    }
    PROCESS_INFORMATION process{};
    std::wstring command = L"\"" + serverPath.wstring() + L"\"";
    std::vector<wchar_t> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back(L'\0');
    const BOOL created = CreateProcessW(serverPath.c_str(), mutableCommand.data(), nullptr, nullptr,
        log != INVALID_HANDLE_VALUE, CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, environment.data(),
        g.installDir.c_str(), &startup, &process);
    SecureZeroMemory(environment.data(), environment.size() * sizeof(wchar_t));
    if (log != INVALID_HANDLE_VALUE) CloseHandle(log);
    if (nullInput != INVALID_HANDLE_VALUE) CloseHandle(nullInput);
    if (!created) {
        showFailure(L"无法启动钢一定制AI服务，请查看安装是否完整。");
        return false;
    }
    CloseHandle(process.hThread);
    g.process = process.hProcess;
    if (g.job && !AssignProcessToJobObject(g.job, g.process)) {
        TerminateProcess(g.process, 1);
        CloseHandle(g.process);
        g.process = nullptr;
        showFailure(L"无法接管本地服务进程，请重新启动程序。");
        return false;
    }
    g.openWhenReady = openWhenReady;
    g.cancelHealth = false;
    updateTrayTip(L"钢一定制AI - 正在启动");
    g.healthThread = std::thread([] {
        for (int attempt = 0; attempt < 100 && !g.cancelHealth; ++attempt) {
            if (!processRunning()) break;
            if (localHttpRequest(g.port, "GET", "/health")) {
                if (!g.cancelHealth) PostMessageW(g.window, kServiceReady, 0, 0);
                while (!g.cancelHealth) {
                    if (WaitForSingleObject(g.process, 500) == WAIT_OBJECT_0) {
                        DWORD exitCode = 0;
                        GetExitCodeProcess(g.process, &exitCode);
                        if (!g.cancelHealth) PostMessageW(g.window, kServiceFailed, exitCode, 0);
                        return;
                    }
                }
                return;
            }
            Sleep(150);
        }
        if (!g.cancelHealth) PostMessageW(g.window, kServiceFailed, 0, 0);
    });
    return true;
}

void scheduleAutomaticRestart(DWORD exitCode) {
    g.lastExitCode = exitCode;
    stopService();
    const auto delay = g.restartPolicy.recordFailure();
    if (!delay) {
        showFailure(L"本地服务连续三次恢复失败，已停止自动重启。请查看日志或导出诊断报告。");
        return;
    }
    const std::wstring tip = L"钢一定制AI - " + std::to_wstring(*delay) + L" 秒后尝试恢复";
    updateTrayTip(tip.c_str());
    SetTimer(g.window, kRestartTimer, *delay * 1000, nullptr);
}

std::filesystem::path databasePath() {
    return std::filesystem::path(g.dataDir) / L"data" / L"gangyiAI.db";
}

std::filesystem::path logPath() {
    return std::filesystem::path(g.dataDir) / L"logs" / L"gangyiAI.log";
}

std::wstring timestampText() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    std::wostringstream value;
    value << std::put_time(&local, L"%Y%m%d-%H%M%S");
    return value.str();
}

std::optional<std::filesystem::path> chooseFile(bool save, const wchar_t* title, const std::wstring& initialName,
                                                const wchar_t* filter, const wchar_t* defaultExtension) {
    std::vector<wchar_t> buffer(32768, L'\0');
    std::copy_n(initialName.c_str(), std::min(initialName.size(), buffer.size() - 1), buffer.data());
    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = g.window;
    dialog.lpstrFilter = filter;
    dialog.lpstrFile = buffer.data();
    dialog.nMaxFile = static_cast<DWORD>(buffer.size());
    dialog.lpstrTitle = title;
    dialog.lpstrDefExt = defaultExtension;
    dialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR |
        (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
    const BOOL selected = save ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog);
    if (!selected) return std::nullopt;
    return std::filesystem::path(buffer.data());
}

void viewLog() {
    std::error_code error;
    std::filesystem::create_directories(logPath().parent_path(), error);
    if (!std::filesystem::exists(logPath())) {
        std::ofstream create(logPath());
    }
    ShellExecuteW(nullptr, L"open", logPath().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void openDataDirectory() {
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(g.dataDir) / L"data", error);
    ShellExecuteW(nullptr, L"open", g.dataDir.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void backupCourseData() {
    if (!std::filesystem::is_regular_file(databasePath())) {
        MessageBoxW(g.window, L"尚未找到课程数据库，请先创建课程后再备份。", L"钢一定制AI", MB_OK | MB_ICONINFORMATION);
        return;
    }
    constexpr wchar_t databaseFilter[] = L"课程数据库 (*.db)\0*.db\0所有文件 (*.*)\0*.*\0";
    const auto selected = chooseFile(true, L"备份课程数据", L"gangyiAI-backup-" + timestampText() + L".db",
        databaseFilter, L"db");
    if (!selected) return;
    std::wstring error;
    if (!gangyi::launcher::backupDatabase(databasePath(), *selected, error)) {
        MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 备份失败", MB_OK | MB_ICONERROR);
        return;
    }
    MessageBoxW(g.window, (L"课程数据已备份到：\n" + selected->wstring()).c_str(),
        L"钢一定制AI", MB_OK | MB_ICONINFORMATION);
}

void restoreCourseData() {
    constexpr wchar_t databaseFilter[] = L"课程数据库 (*.db)\0*.db\0所有文件 (*.*)\0*.*\0";
    const auto selected = chooseFile(false, L"恢复课程数据", L"", databaseFilter, L"db");
    if (!selected) return;
    std::wstring error;
    if (!gangyi::launcher::validateDatabase(*selected, error)) {
        MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 无法恢复", MB_OK | MB_ICONERROR);
        return;
    }
    if (MessageBoxW(g.window, L"恢复会替换当前课程数据，程序会自动保留恢复前副本。是否继续？",
        L"钢一定制AI", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) return;
    stopService();
    const auto rollbackDir = std::filesystem::path(g.dataDir) / L"backups";
    std::error_code filesystemError;
    std::filesystem::create_directories(rollbackDir, filesystemError);
    const auto rollback = rollbackDir / (L"before-restore-" + timestampText() + L".db");
    if (filesystemError || !gangyi::launcher::restoreDatabase(*selected, databasePath(), rollback, error)) {
        MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 恢复失败", MB_OK | MB_ICONERROR);
        startService(false);
        return;
    }
    g.pendingRestoreRollback = rollback;
    if (!startService(false)) {
        std::wstring rollbackError;
        stopService();
        gangyi::launcher::backupDatabase(rollback, databasePath(), rollbackError);
        g.pendingRestoreRollback.clear();
        startService(false);
        MessageBoxW(g.window, L"新数据库无法启动，已尝试恢复原有数据。请查看日志。",
            L"钢一定制AI - 已回滚", MB_OK | MB_ICONERROR);
        return;
    }
    MessageBoxW(g.window, (L"课程数据恢复完成。恢复前副本位于：\n" + rollback.wstring()).c_str(),
        L"钢一定制AI", MB_OK | MB_ICONINFORMATION);
}

void exportDiagnosticReport() {
    constexpr wchar_t textFilter[] = L"文本文件 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0";
    const auto selected = chooseFile(true, L"导出诊断报告", L"gangyiAI-diagnostic-" + timestampText() + L".txt",
        textFilter, L"txt");
    if (!selected) return;
    gangyi::launcher::DiagnosticInfo info;
    info.version = toWide(gangyi::kVersion);
    info.installDir = g.installDir;
    info.dataDir = g.dataDir;
    info.provider = gangyi::launcher::providerProfile(g.settings.provider).name;
    info.baseUrl = g.settings.baseUrl;
    info.model = g.settings.model;
    info.serviceRunning = processRunning();
    info.port = g.port;
    info.exitCode = g.lastExitCode;
    info.restartFailures = g.restartPolicy.failureCount();
    std::wstring error;
    if (!gangyi::launcher::writeDiagnosticReport(*selected, info, error)) {
        MessageBoxW(g.window, error.c_str(), L"钢一定制AI - 导出失败", MB_OK | MB_ICONERROR);
        return;
    }
    MessageBoxW(g.window, L"诊断报告已生成。报告不包含 API Key、课程内容或 AI 对话正文。",
        L"钢一定制AI", MB_OK | MB_ICONINFORMATION);
}

void loadControls() {
    g.settings = loadSettings();
    std::wstring apiKey = readApiKey();
    std::wstring bochaKey = readCredential(kBochaCredentialTarget);
    SendMessageW(g.providerCombo, CB_SETCURSEL, providerIndex(g.settings.provider), 0);
    SetWindowTextW(g.baseUrlEdit, g.settings.baseUrl.c_str());
    SetWindowTextW(g.apiKeyEdit, apiKey.c_str());
    SetWindowTextW(g.bochaKeyEdit, bochaKey.c_str());
    SetWindowTextW(g.modelEdit, g.settings.model.c_str());
    EnableWindow(g.baseUrlEdit, g.settings.provider == gangyi::launcher::AIProvider::Custom);
    SendMessageW(g.autoStartCheck, BM_SETCHECK, g.settings.autoStart ? BST_CHECKED : BST_UNCHECKED, 0);
    SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
    SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));
}

void showSettingsWindow() {
    loadControls();
    ShowWindow(g.window, SW_SHOW);
    SetForegroundWindow(g.window);
    SetFocus(g.baseUrlEdit);
}

bool saveSettingsFromControls() {
    Settings settings;
    settings.provider = selectedProvider();
    settings.baseUrl = controlText(g.baseUrlEdit);
    settings.model = controlText(g.modelEdit);
    settings.autoStart = SendMessageW(g.autoStartCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
    std::wstring apiKey = controlText(g.apiKeyEdit);
    std::wstring bochaKey = controlText(g.bochaKeyEdit);
    if (!validBaseUrl(settings.baseUrl) || settings.model.empty() || apiKey.empty()) {
        MessageBoxW(g.window, L"请填写有效的 HTTP(S) API 地址、API Key 和模型名称。", L"钢一定制AI", MB_OK | MB_ICONWARNING);
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));
        return false;
    }
    const bool saved = writeRegistryString(L"AIBaseUrl", settings.baseUrl) &&
        writeRegistryString(L"AIModel", settings.model) &&
        writeRegistryString(L"AIProvider", gangyi::launcher::providerProfile(settings.provider).id) &&
        writeRegistryDword(L"AutoStart", settings.autoStart ? 1 : 0) &&
        writeApiKey(apiKey) && (bochaKey.empty() ?
            (CredDeleteW(kBochaCredentialTarget, CRED_TYPE_GENERIC, 0) || GetLastError() == ERROR_NOT_FOUND) :
            writeCredential(kBochaCredentialTarget, bochaKey)) &&
        writeRegistryDword(L"SkipAISetup", 0) && configureAutoStart(settings.autoStart);
    SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
    SecureZeroMemory(bochaKey.data(), bochaKey.size() * sizeof(wchar_t));
    SetWindowTextW(g.apiKeyEdit, L"");
    SetWindowTextW(g.bochaKeyEdit, L"");
    if (!saved) {
        MessageBoxW(g.window, L"配置保存失败，请检查当前用户权限。", L"钢一定制AI", MB_OK | MB_ICONERROR);
        return false;
    }
    g.settings = settings;
    g.hasConfig = true;
    g.skipAiSetup = false;
    ShowWindow(g.window, SW_HIDE);
    return startService(true);
}

void skipAiSetup() {
    const bool autoStart = SendMessageW(g.autoStartCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
    if (!configureAutoStart(autoStart) || !writeRegistryDword(L"AutoStart", autoStart ? 1 : 0) ||
        !writeRegistryDword(L"SkipAISetup", 1)) {
        MessageBoxW(g.window, L"暂不配置的选择保存失败，请检查当前用户权限。", L"钢一定制AI", MB_OK | MB_ICONERROR);
        return;
    }
    SetWindowTextW(g.apiKeyEdit, L"");
    g.skipAiSetup = true;
    ShowWindow(g.window, SW_HIDE);
    if (!startService(true)) showSettingsWindow();
}

std::wstring connectionErrorMessage(const std::string& type) {
    if (type == "missing_config") return L"配置不完整，请填写 API 地址、API Key 和模型名称。";
    if (type == "auth_error") return L"认证失败，请检查 API Key 是否正确或是否具有访问权限。";
    if (type == "rate_limited") return L"接口请求过于频繁或余额不足，请稍后重试。";
    if (type == "timeout") return L"连接超时，请检查网络、接口地址或代理设置。";
    if (type == "network_error") return L"无法连接 AI 接口，请检查网络和 API 地址。";
    if (type == "provider_5xx") return L"AI 服务暂时不可用，请稍后重试。";
    if (type == "invalid_response") return L"接口已连接，但返回格式不是兼容的 Chat Completions 响应。";
    if (type == "models_unsupported") return L"该接口未提供模型列表，请手工填写模型名称。";
    return L"连接测试失败，请检查 API 地址和模型名称。";
}

void applyProviderSelection() {
    const auto provider = selectedProvider();
    const auto& profile = gangyi::launcher::providerProfile(provider);
    const bool custom = provider == gangyi::launcher::AIProvider::Custom;
    EnableWindow(g.baseUrlEdit, custom);
    if (!custom) {
        SetWindowTextW(g.baseUrlEdit, profile.baseUrl);
        SetWindowTextW(g.modelEdit, profile.recommendedModel);
    }
    SetWindowTextW(g.apiKeyEdit, L"");
    SetWindowTextW(g.statusLabel, L"服务商已切换，请填写对应的 API Key 并获取模型。");
}

void fetchModelsFromControls() {
    if (g.modelListRunning.exchange(true)) return;
    std::wstring baseUrl = controlText(g.baseUrlEdit);
    std::wstring apiKey = controlText(g.apiKeyEdit);
    std::wstring currentModel = controlText(g.modelEdit);
    if (!validBaseUrl(baseUrl) || apiKey.empty()) {
        g.modelListRunning = false;
        SetWindowTextW(g.statusLabel, L"请先填写有效的 API 地址和 API Key。");
        return;
    }
    if (g.modelListThread.joinable()) g.modelListThread.join();
    EnableWindow(g.fetchModelsButton, FALSE);
    EnableWindow(g.testButton, FALSE);
    EnableWindow(g.saveButton, FALSE);
    SetWindowTextW(g.statusLabel, L"正在获取可用模型，请稍候……");
    g.modelListThread = std::thread([baseUrl = std::move(baseUrl), apiKey = std::move(apiKey),
                                     currentModel = std::move(currentModel)]() mutable {
        bool succeeded = false;
        std::wstring message;
        std::vector<std::wstring> models;
        try {
            gangyi::AIClientConfig config;
            config.baseUrl = toUtf8(baseUrl);
            config.apiKey = toUtf8(apiKey);
            config.model = toUtf8(currentModel);
            config.timeoutMs = 15000;
            config.retryAttempts = 1;
            SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
            gangyi::AIClient client(std::move(config));
            for (const auto& model : client.listModels(15000)) models.push_back(toWide(model));
            succeeded = !models.empty();
            message = succeeded ? L"模型列表已更新，也可以继续手工输入模型名称。"
                                : L"接口没有返回模型，请手工填写模型名称。";
        } catch (const gangyi::AIClientError& error) {
            message = connectionErrorMessage(error.errorType);
        } catch (...) {
            message = L"获取模型时发生未知错误，请手工填写模型名称。";
        }
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        {
            std::lock_guard<std::mutex> lock(g.modelListMutex);
            g.availableModels = std::move(models);
            g.modelListSucceeded = succeeded;
            g.modelListMessage = std::move(message);
        }
        if (!g.shuttingDown && !PostMessageW(g.window, kModelListComplete, 0, 0)) g.modelListRunning = false;
    });
}

void testConnectionFromControls() {
    if (g.connectionTestRunning.exchange(true)) return;
    std::wstring baseUrl = controlText(g.baseUrlEdit);
    std::wstring apiKey = controlText(g.apiKeyEdit);
    std::wstring model = controlText(g.modelEdit);
    if (!validBaseUrl(baseUrl) || apiKey.empty() || model.empty()) {
        g.connectionTestRunning = false;
        SetWindowTextW(g.statusLabel, L"请先填写有效的 API 地址、API Key 和模型名称。");
        return;
    }
    if (g.connectionTestThread.joinable()) g.connectionTestThread.join();
    EnableWindow(g.testButton, FALSE);
    EnableWindow(g.saveButton, FALSE);
    SetWindowTextW(g.statusLabel, L"正在测试 AI 连接，请稍候……");
    g.connectionTestThread = std::thread([baseUrl = std::move(baseUrl), apiKey = std::move(apiKey),
                                         model = std::move(model)]() mutable {
        bool succeeded = false;
        std::wstring message;
        try {
            gangyi::AIClientConfig config;
            config.baseUrl = toUtf8(baseUrl);
            config.apiKey = toUtf8(apiKey);
            config.model = toUtf8(model);
            config.timeoutMs = 15000;
            config.retryAttempts = 1;
            SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));

            gangyi::AIClient client(std::move(config));
            gangyi::ChatOptions options;
            options.messages = {{"user", "Reply with OK."}};
            options.temperature = 0;
            options.maxTokens = 16;
            options.timeoutMs = 15000;
            options.maxAttempts = 1;
            const auto result = client.chat(options);
            succeeded = result.status >= 200 && result.status < 300;
            message = succeeded ? L"连接成功，API 地址、Key 和模型均可用。" : L"连接测试失败。";
        } catch (const gangyi::AIClientError& error) {
            message = connectionErrorMessage(error.errorType);
        } catch (...) {
            message = L"连接测试发生未知错误，请检查配置后重试。";
        }
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        {
            std::lock_guard<std::mutex> lock(g.connectionTestMutex);
            g.connectionTestSucceeded = succeeded;
            g.connectionTestMessage = std::move(message);
        }
        if (!g.shuttingDown && !PostMessageW(g.window, kConnectionTestComplete, 0, 0)) {
            g.connectionTestRunning = false;
        }
    });
}

void testBochaFromControls() {
    if (g.connectionTestRunning.exchange(true)) return;
    std::wstring key = controlText(g.bochaKeyEdit);
    if (key.empty()) {
        g.connectionTestRunning = false;
        SetWindowTextW(g.statusLabel, L"请先填写博查 Key。");
        return;
    }
    if (g.connectionTestThread.joinable()) g.connectionTestThread.join();
    EnableWindow(g.bochaTestButton, FALSE);
    EnableWindow(g.testButton, FALSE);
    EnableWindow(g.saveButton, FALSE);
    SetWindowTextW(g.statusLabel, L"正在测试博查连接，请稍候……");
    g.connectionTestThread = std::thread([key = std::move(key)]() mutable {
        bool succeeded = false;
        std::string diagnostic;
        try {
            std::string utf8Key = toUtf8(key);
            SecureZeroMemory(key.data(), key.size() * sizeof(wchar_t));
            succeeded = gangyi::SearchClient::testBochaKey(utf8Key, &diagnostic);
            SecureZeroMemory(utf8Key.data(), utf8Key.size());
        } catch (...) {}
        SecureZeroMemory(key.data(), key.size() * sizeof(wchar_t));
        const std::wstring message = succeeded ?
            (diagnostic == "ok_direct" ? L"博查连接成功（已绕过故障代理）。" : L"博查连接成功。") :
            diagnostic == "http:401" ? L"博查认证失败（HTTP 401）：请确认粘贴的是 API Key。" :
            diagnostic == "http:403" ? L"博查权限或账户余额不足（HTTP 403）。" :
            diagnostic == "http:429" ? L"博查请求频率受限（HTTP 429），请稍后重试。" :
            diagnostic.rfind("network_error:", 0) == 0 ? L"博查网络连接失败，请检查代理或证书。" :
            diagnostic == "invalid_response" ? L"博查已响应，但返回内容不符合接口格式。" :
            L"博查服务返回错误，请稍后重试。";
        {
            std::lock_guard<std::mutex> lock(g.connectionTestMutex);
            g.connectionTestSucceeded = succeeded;
            g.connectionTestMessage = message;
        }
        if (!g.shuttingDown && !PostMessageW(g.window, kConnectionTestComplete, 0, 0))
            g.connectionTestRunning = false;
    });
}

HICON appIcon() {
    HICON icon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_GANGYI_AI));
    return icon ? icon : LoadIconW(nullptr, IDI_APPLICATION);
}

void addTrayIcon() {
    NOTIFYICONDATAW icon{};
    icon.cbSize = sizeof(icon);
    icon.hWnd = g.window;
    icon.uID = 1;
    icon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    icon.uCallbackMessage = kTrayMessage;
    icon.hIcon = appIcon();
    wcsncpy_s(icon.szTip, ARRAYSIZE(icon.szTip), L"钢一定制AI", _TRUNCATE);
    Shell_NotifyIconW(NIM_ADD, &icon);
    icon.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &icon);
}

void removeTrayIcon() {
    NOTIFYICONDATAW icon{};
    icon.cbSize = sizeof(icon);
    icon.hWnd = g.window;
    icon.uID = 1;
    Shell_NotifyIconW(NIM_DELETE, &icon);
}

void showTrayMenu() {
    POINT point{};
    GetCursorPos(&point);
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kMenuOpen, L"打开钢一定制AI");
    AppendMenuW(menu, MF_STRING, kMenuSettings, L"设置");
    AppendMenuW(menu, MF_STRING, kMenuRestart, L"重启服务");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kMenuViewLog, L"查看日志");
    AppendMenuW(menu, MF_STRING, kMenuOpenData, L"打开数据目录");
    AppendMenuW(menu, MF_STRING, kMenuDiagnostic, L"导出诊断报告");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kMenuBackup, L"备份课程数据");
    AppendMenuW(menu, MF_STRING, kMenuRestore, L"恢复课程数据");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kMenuExit, L"退出");
    SetForegroundWindow(g.window);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, point.x, point.y, 0, g.window, nullptr);
    DestroyMenu(menu);
}

void createLabel(HWND parent, const wchar_t* text, int x, int y, int width) {
    CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, width, 22, parent, nullptr, nullptr, nullptr);
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        g.window = window;
        HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        createLabel(window, L"AI 服务商", 24, 18, 120);
        createLabel(window, L"AI API 地址", 24, 72, 120);
        createLabel(window, L"API Key", 24, 126, 120);
        createLabel(window, L"模型名称", 24, 180, 120);
        createLabel(window, L"博查 Key（联网检索）", 24, 238, 200);
        g.providerCombo = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
            24, 41, 456, 160, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kProviderCombo)), nullptr, nullptr);
        SendMessageW(g.providerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"DeepSeek"));
        SendMessageW(g.providerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"OpenAI"));
        SendMessageW(g.providerCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"自定义 OpenAI 兼容接口"));
        g.baseUrlEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            24, 95, 456, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBaseUrlEdit)), nullptr, nullptr);
        g.apiKeyEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
            24, 149, 456, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kApiKeyEdit)), nullptr, nullptr);
        g.modelEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL,
            24, 203, 346, 220, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kModelEdit)), nullptr, nullptr);
        g.fetchModelsButton = CreateWindowW(L"BUTTON", L"获取模型", WS_CHILD | WS_VISIBLE,
            378, 202, 102, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFetchModelsButton)), nullptr, nullptr);
        g.bochaKeyEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
            24, 260, 346, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBochaKeyEdit)), nullptr, nullptr);
        g.bochaTestButton = CreateWindowW(L"BUTTON", L"测试博查", WS_CHILD | WS_VISIBLE,
            378, 259, 102, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBochaTestButton)), nullptr, nullptr);
        g.autoStartCheck = CreateWindowW(L"BUTTON", L"登录 Windows 后自动启动", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            24, 299, 250, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kAutoStartCheck)), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"暂不配置，直接使用", WS_CHILD | WS_VISIBLE,
            24, 369, 148, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSkipAiButton)), nullptr, nullptr);
        g.testButton = CreateWindowW(L"BUTTON", L"测试连接", WS_CHILD | WS_VISIBLE,
            180, 369, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTestButton)), nullptr, nullptr);
        g.saveButton = CreateWindowW(L"BUTTON", L"保存并启动", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            282, 369, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSaveButton)), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"取消", WS_CHILD | WS_VISIBLE,
            384, 369, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCancelButton)), nullptr, nullptr);
        g.statusLabel = CreateWindowW(L"STATIC", L"可先使用本地功能；AI 功能可稍后在托盘“设置”中启用。", WS_CHILD | WS_VISIBLE,
            24, 333, 456, 24, window, nullptr, nullptr, nullptr);
        EnumChildWindows(window, [](HWND child, LPARAM value) -> BOOL {
            SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(value), TRUE);
            return TRUE;
        }, reinterpret_cast<LPARAM>(font));
        addTrayIcon();
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case kProviderCombo:
            if (HIWORD(wParam) == CBN_SELCHANGE) applyProviderSelection();
            return 0;
        case kFetchModelsButton: fetchModelsFromControls(); return 0;
        case kTestButton: testConnectionFromControls(); return 0;
        case kBochaTestButton: testBochaFromControls(); return 0;
        case kSaveButton: saveSettingsFromControls(); return 0;
        case kSkipAiButton: skipAiSetup(); return 0;
        case kCancelButton:
            SetWindowTextW(g.apiKeyEdit, L"");
            if (g.hasConfig || g.skipAiSetup) ShowWindow(window, SW_HIDE); else DestroyWindow(window);
            return 0;
        case kMenuOpen: PostMessageW(window, kOpenDesktop, 0, 0); return 0;
        case kMenuSettings: showSettingsWindow(); return 0;
        case kMenuRestart:
            if (!startService(false)) showSettingsWindow();
            return 0;
        case kMenuViewLog: viewLog(); return 0;
        case kMenuOpenData: openDataDirectory(); return 0;
        case kMenuBackup: backupCourseData(); return 0;
        case kMenuRestore: restoreCourseData(); return 0;
        case kMenuDiagnostic: exportDiagnosticReport(); return 0;
        case kMenuExit: DestroyWindow(window); return 0;
        }
        break;
    case kTrayMessage:
        if (LOWORD(lParam) == WM_LBUTTONDBLCLK) PostMessageW(window, kOpenDesktop, 0, 0);
        else if (LOWORD(lParam) == WM_CONTEXTMENU || LOWORD(lParam) == WM_RBUTTONUP) showTrayMenu();
        return 0;
    case kServiceReady:
        updateTrayTip(L"钢一定制AI - 运行中");
        g.pendingRestoreRollback.clear();
        KillTimer(window, kStableTimer);
        SetTimer(window, kStableTimer, 10 * 60 * 1000, nullptr);
        if (g.openWhenReady || g.desktopWindow) openDesktop();
        return 0;
    case kServiceFailed:
        KillTimer(window, kStableTimer);
        if (!g.pendingRestoreRollback.empty()) {
            const auto rollback = g.pendingRestoreRollback;
            g.pendingRestoreRollback.clear();
            stopService();
            std::wstring rollbackError;
            if (gangyi::launcher::backupDatabase(rollback, databasePath(), rollbackError)) {
                startService(false);
                MessageBoxW(window, L"恢复后的数据库无法启动，已恢复到操作前数据。",
                    L"钢一定制AI - 已自动回滚", MB_OK | MB_ICONWARNING);
            } else {
                showFailure(L"恢复后的数据库无法启动，自动回滚也失败。请保留数据目录并联系维护人员。");
            }
            return 0;
        }
        scheduleAutomaticRestart(static_cast<DWORD>(wParam));
        return 0;
    case WM_TIMER:
        if (wParam == kRestartTimer) {
            KillTimer(window, kRestartTimer);
            if (!startService(false, false)) scheduleAutomaticRestart(g.lastExitCode);
            return 0;
        }
        if (wParam == kStableTimer) {
            KillTimer(window, kStableTimer);
            g.restartPolicy.reset();
            return 0;
        }
        break;
    case kOpenDesktop:
        if (processRunning()) openDesktop(); else if (!startService(true)) showSettingsWindow();
        return 0;
    case kOpenApiSettings:
        showSettingsWindow();
        return 0;
    case kConnectionTestComplete: {
        std::wstring message;
        bool succeeded = false;
        {
            std::lock_guard<std::mutex> lock(g.connectionTestMutex);
            message = g.connectionTestMessage;
            succeeded = g.connectionTestSucceeded;
        }
        if (g.connectionTestThread.joinable()) g.connectionTestThread.join();
        g.connectionTestRunning = false;
        EnableWindow(g.testButton, TRUE);
        EnableWindow(g.bochaTestButton, TRUE);
        EnableWindow(g.saveButton, TRUE);
        SetWindowTextW(g.statusLabel, message.c_str());
        MessageBoxW(window, message.c_str(), L"钢一定制AI - 连接测试",
            MB_OK | (succeeded ? MB_ICONINFORMATION : MB_ICONWARNING));
        return 0;
    }
    case kModelListComplete: {
        std::wstring message;
        std::vector<std::wstring> models;
        bool succeeded = false;
        {
            std::lock_guard<std::mutex> lock(g.modelListMutex);
            message = g.modelListMessage;
            models = g.availableModels;
            succeeded = g.modelListSucceeded;
        }
        if (g.modelListThread.joinable()) g.modelListThread.join();
        g.modelListRunning = false;
        EnableWindow(g.fetchModelsButton, TRUE);
        EnableWindow(g.testButton, TRUE);
        EnableWindow(g.saveButton, TRUE);
        if (succeeded) {
            const std::wstring current = controlText(g.modelEdit);
            SendMessageW(g.modelEdit, CB_RESETCONTENT, 0, 0);
            int selected = -1;
            for (size_t index = 0; index < models.size(); ++index) {
                SendMessageW(g.modelEdit, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(models[index].c_str()));
                if (models[index] == current) selected = static_cast<int>(index);
            }
            if (selected >= 0) SendMessageW(g.modelEdit, CB_SETCURSEL, selected, 0);
            else if (!current.empty()) SetWindowTextW(g.modelEdit, current.c_str());
        }
        SetWindowTextW(g.statusLabel, message.c_str());
        if (!succeeded) MessageBoxW(window, message.c_str(), L"钢一定制AI - 获取模型", MB_OK | MB_ICONWARNING);
        return 0;
    }
    case WM_CLOSE:
        SetWindowTextW(g.apiKeyEdit, L"");
        if (g.hasConfig || g.skipAiSetup) ShowWindow(window, SW_HIDE); else DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        g.shuttingDown = true;
        if (g.desktopWindow) DestroyWindow(g.desktopWindow);
        if (g.connectionTestThread.joinable()) g.connectionTestThread.join();
        if (g.modelListThread.joinable()) g.modelListThread.join();
        KillTimer(window, kRestartTimer);
        KillTimer(window, kStableTimer);
        removeTrayIcon();
        stopService();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

int selfTest() {
    const std::wstring token = randomToken();
    if (!validBaseUrl(L"https://api.deepseek.com/v1") || validBaseUrl(L"file:///tmp/key")) return 10;
    if (token.size() != 64) return 11;
    if (findAvailablePort() < 39002) return 12;
    if (localAppDataPath().empty()) return 13;
    Settings settings;
    if (canStartService(settings, false, false) || !canStartService(settings, false, true) ||
        !canStartService(settings, true, false)) return 14;
    settings.baseUrl.clear();
    if (canStartService(settings, true, false) || !canStartService(settings, false, true)) return 15;
    return 0;
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int) {
    enablePerMonitorDpi();
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 1;
    WSADATA sockets{};
    if (WSAStartup(MAKEWORD(2, 2), &sockets) != 0) {
        CoUninitialize();
        return 1;
    }
    const std::wstring arguments = commandLine ? commandLine : L"";
    if (arguments.find(L"--remove-credentials") != std::wstring::npos) {
        removeLocalSettings();
        WSACleanup();
        CoUninitialize();
        return 0;
    }
    if (arguments == L"--self-test") {
        const int result = selfTest();
        WSACleanup();
        CoUninitialize();
        return result;
    }

    g.smokeMode = arguments.find(L"--self-test-ui") != std::wstring::npos;
    HANDLE mutex = CreateMutexW(nullptr, TRUE, g.smokeMode ? L"GangyiAI.Launcher.UI.Smoke" : kMutexName);
    if (!mutex || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (HWND existing = FindWindowW(kWindowClass, nullptr)) PostMessageW(existing, kOpenDesktop, 0, 0);
        if (mutex) CloseHandle(mutex);
        WSACleanup();
        CoUninitialize();
        return 0;
    }

    g.launchSessionId = randomToken();
    const std::wstring exe = executablePath();
    g.installDir = std::filesystem::path(exe).parent_path().wstring();
    g.dataDir = (std::filesystem::path(localAppDataPath()) / L"GangyiAI").wstring();
    g.background = arguments.find(L"--background") != std::wstring::npos;
    g.settings = g.smokeMode ? Settings{} : loadSettings();
    std::wstring key = g.smokeMode ? std::wstring{} : readApiKey();
    g.hasConfig = validBaseUrl(g.settings.baseUrl) && !g.settings.model.empty() && !key.empty();
    g.skipAiSetup = g.smokeMode || readRegistryDword(L"SkipAISetup", 0) != 0;
    SecureZeroMemory(key.data(), key.size() * sizeof(wchar_t));

    g.job = CreateJobObjectW(nullptr, nullptr);
    if (g.job) {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(g.job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
    }

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.hIcon = appIcon();
    windowClass.hIconSm = appIcon();
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = kWindowClass;
    if (!RegisterClassExW(&windowClass)) return 1;

    WNDCLASSEXW desktopClass = windowClass;
    desktopClass.lpfnWndProc = desktopWindowProc;
    desktopClass.lpszClassName = kDesktopWindowClass;
    if (!RegisterClassExW(&desktopClass)) return 1;

    g.window = CreateWindowExW(0, kWindowClass, L"钢一定制AI 配置", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 455, nullptr, nullptr, instance, nullptr);
    if (!g.window) return 1;
    if (g.hasConfig || g.skipAiSetup) {
        if (!startService(!g.background)) showSettingsWindow();
    } else {
        showSettingsWindow();
    }

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    if (g.job) CloseHandle(g.job);
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    WSACleanup();
    CoUninitialize();
    return static_cast<int>(message.wParam);
}

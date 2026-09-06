#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>
#include <bcrypt.h>
#include <shellapi.h>
#include <shlobj.h>
#include <wincred.h>

#include "ai_client.hpp"
#include "windows_resource.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cwctype>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

constexpr wchar_t kWindowClass[] = L"GangyiAILauncherWindow";
constexpr wchar_t kMutexName[] = L"GangyiAI.Launcher.v1";
constexpr wchar_t kRegistryKey[] = L"Software\\GangyiAI";
constexpr wchar_t kCredentialTarget[] = L"GangyiAI/APIKey";
constexpr UINT kTrayMessage = WM_APP + 1;
constexpr UINT kServiceReady = WM_APP + 2;
constexpr UINT kServiceFailed = WM_APP + 3;
constexpr UINT kOpenBrowser = WM_APP + 4;
constexpr UINT kConnectionTestComplete = WM_APP + 5;

constexpr int kBaseUrlEdit = 101;
constexpr int kApiKeyEdit = 102;
constexpr int kModelEdit = 103;
constexpr int kAutoStartCheck = 104;
constexpr int kSaveButton = 105;
constexpr int kCancelButton = 106;
constexpr int kTestButton = 107;
constexpr int kMenuOpen = 201;
constexpr int kMenuSettings = 202;
constexpr int kMenuRestart = 203;
constexpr int kMenuExit = 204;

struct Settings {
    std::wstring baseUrl = L"https://api.deepseek.com/v1";
    std::wstring model = L"deepseek-chat";
    bool autoStart = false;
};

struct AppState {
    HWND window = nullptr;
    HWND baseUrlEdit = nullptr;
    HWND apiKeyEdit = nullptr;
    HWND modelEdit = nullptr;
    HWND autoStartCheck = nullptr;
    HWND testButton = nullptr;
    HWND saveButton = nullptr;
    HWND statusLabel = nullptr;
    HANDLE process = nullptr;
    HANDLE job = nullptr;
    std::thread healthThread;
    std::thread connectionTestThread;
    std::atomic<bool> cancelHealth{false};
    std::atomic<bool> connectionTestRunning{false};
    std::atomic<bool> shuttingDown{false};
    std::mutex connectionTestMutex;
    std::wstring connectionTestMessage;
    bool connectionTestSucceeded = false;
    Settings settings;
    std::wstring installDir;
    std::wstring dataDir;
    std::wstring controlToken;
    int port = 0;
    bool hasConfig = false;
    bool background = false;
    bool openWhenReady = true;
};

AppState g;

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

std::wstring readApiKey() {
    PCREDENTIALW credential = nullptr;
    if (!CredReadW(kCredentialTarget, CRED_TYPE_GENERIC, 0, &credential)) return {};
    std::wstring value;
    if (credential->CredentialBlob && credential->CredentialBlobSize % sizeof(wchar_t) == 0) {
        value.assign(reinterpret_cast<const wchar_t*>(credential->CredentialBlob),
            credential->CredentialBlobSize / sizeof(wchar_t));
    }
    CredFree(credential);
    return value;
}

bool writeApiKey(const std::wstring& value) {
    if (value.empty() || value.size() * sizeof(wchar_t) > CRED_MAX_CREDENTIAL_BLOB_SIZE) return false;
    CREDENTIALW credential{};
    credential.Type = CRED_TYPE_GENERIC;
    credential.TargetName = const_cast<wchar_t*>(kCredentialTarget);
    credential.CredentialBlobSize = static_cast<DWORD>(value.size() * sizeof(wchar_t));
    credential.CredentialBlob = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(value.data()));
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    credential.UserName = const_cast<wchar_t*>(L"gangyiAI");
    return CredWriteW(&credential, 0) == TRUE;
}

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
    settings.autoStart = readRegistryDword(L"AutoStart", 0) != 0;
    return settings;
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
    RegDeleteTreeW(HKEY_CURRENT_USER, kRegistryKey);
}

bool startsWithIgnoreCase(const std::wstring& value, const wchar_t* prefix) {
    const size_t length = wcslen(prefix);
    return value.size() >= length && _wcsnicmp(value.c_str(), prefix, length) == 0;
}

bool validBaseUrl(const std::wstring& value) {
    return startsWithIgnoreCase(value, L"https://") || startsWithIgnoreCase(value, L"http://");
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

void openBrowser() {
    if (!g.port) return;
    const std::wstring url = L"http://127.0.0.1:" + std::to_wstring(g.port) + L"/";
    ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
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

bool startService(bool openWhenReady) {
    stopService();
    g.settings = loadSettings();
    std::wstring apiKey = readApiKey();
    if (!validBaseUrl(g.settings.baseUrl) || g.settings.model.empty() || apiKey.empty()) return false;
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

    std::vector<std::pair<std::wstring, std::wstring>> overrides = {
        {L"HOST", L"127.0.0.1"},
        {L"PORT", std::to_wstring(g.port)},
        {L"AI_BASE_URL", g.settings.baseUrl},
        {L"AI_API_KEY", apiKey},
        {L"AI_MODEL", g.settings.model},
        {L"DATABASE_PATH", databasePath.wstring()},
        {L"LOCAL_CONTROL_TOKEN", g.controlToken},
    };
    const std::filesystem::path caBundle = std::filesystem::path(g.installDir) / L"curl-ca-bundle.crt";
    if (std::filesystem::is_regular_file(caBundle)) overrides.emplace_back(L"CURL_CA_BUNDLE", caBundle.wstring());
    auto environment = childEnvironment(overrides);
    SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));

    SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
    const std::filesystem::path logPath = logDir / L"gangyiAI.log";
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
                        if (!g.cancelHealth) PostMessageW(g.window, kServiceFailed, 0, 0);
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

void loadControls() {
    g.settings = loadSettings();
    std::wstring apiKey = readApiKey();
    SetWindowTextW(g.baseUrlEdit, g.settings.baseUrl.c_str());
    SetWindowTextW(g.apiKeyEdit, apiKey.c_str());
    SetWindowTextW(g.modelEdit, g.settings.model.c_str());
    SendMessageW(g.autoStartCheck, BM_SETCHECK, g.settings.autoStart ? BST_CHECKED : BST_UNCHECKED, 0);
    SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
}

void showSettingsWindow() {
    loadControls();
    ShowWindow(g.window, SW_SHOW);
    SetForegroundWindow(g.window);
    SetFocus(g.baseUrlEdit);
}

bool saveSettingsFromControls() {
    Settings settings;
    settings.baseUrl = controlText(g.baseUrlEdit);
    settings.model = controlText(g.modelEdit);
    settings.autoStart = SendMessageW(g.autoStartCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
    std::wstring apiKey = controlText(g.apiKeyEdit);
    if (!validBaseUrl(settings.baseUrl) || settings.model.empty() || apiKey.empty()) {
        MessageBoxW(g.window, L"请填写有效的 HTTP(S) API 地址、API Key 和模型名称。", L"钢一定制AI", MB_OK | MB_ICONWARNING);
        SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
        return false;
    }
    const bool saved = writeRegistryString(L"AIBaseUrl", settings.baseUrl) &&
        writeRegistryString(L"AIModel", settings.model) &&
        writeRegistryDword(L"AutoStart", settings.autoStart ? 1 : 0) &&
        writeApiKey(apiKey) && configureAutoStart(settings.autoStart);
    SecureZeroMemory(apiKey.data(), apiKey.size() * sizeof(wchar_t));
    SetWindowTextW(g.apiKeyEdit, L"");
    if (!saved) {
        MessageBoxW(g.window, L"配置保存失败，请检查当前用户权限。", L"钢一定制AI", MB_OK | MB_ICONERROR);
        return false;
    }
    g.settings = settings;
    g.hasConfig = true;
    ShowWindow(g.window, SW_HIDE);
    return startService(true);
}

std::wstring connectionErrorMessage(const std::string& type) {
    if (type == "missing_config") return L"配置不完整，请填写 API 地址、API Key 和模型名称。";
    if (type == "auth_error") return L"认证失败，请检查 API Key 是否正确或是否具有访问权限。";
    if (type == "rate_limited") return L"接口请求过于频繁或余额不足，请稍后重试。";
    if (type == "timeout") return L"连接超时，请检查网络、接口地址或代理设置。";
    if (type == "network_error") return L"无法连接 AI 接口，请检查网络和 API 地址。";
    if (type == "provider_5xx") return L"AI 服务暂时不可用，请稍后重试。";
    if (type == "invalid_response") return L"接口已连接，但返回格式不是兼容的 Chat Completions 响应。";
    return L"连接测试失败，请检查 API 地址和模型名称。";
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
        createLabel(window, L"AI API 地址", 24, 24, 120);
        createLabel(window, L"API Key", 24, 82, 120);
        createLabel(window, L"模型名称", 24, 140, 120);
        g.baseUrlEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            24, 47, 456, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBaseUrlEdit)), nullptr, nullptr);
        g.apiKeyEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
            24, 105, 456, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kApiKeyEdit)), nullptr, nullptr);
        g.modelEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            24, 163, 456, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kModelEdit)), nullptr, nullptr);
        g.autoStartCheck = CreateWindowW(L"BUTTON", L"登录 Windows 后自动启动", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            24, 202, 250, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kAutoStartCheck)), nullptr, nullptr);
        g.testButton = CreateWindowW(L"BUTTON", L"测试连接", WS_CHILD | WS_VISIBLE,
            180, 272, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTestButton)), nullptr, nullptr);
        g.saveButton = CreateWindowW(L"BUTTON", L"保存并启动", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            282, 272, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSaveButton)), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"取消", WS_CHILD | WS_VISIBLE,
            384, 272, 96, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCancelButton)), nullptr, nullptr);
        g.statusLabel = CreateWindowW(L"STATIC", L"API Key 将安全保存在 Windows 凭据管理器中。", WS_CHILD | WS_VISIBLE,
            24, 239, 456, 22, window, nullptr, nullptr, nullptr);
        EnumChildWindows(window, [](HWND child, LPARAM value) -> BOOL {
            SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(value), TRUE);
            return TRUE;
        }, reinterpret_cast<LPARAM>(font));
        addTrayIcon();
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case kTestButton: testConnectionFromControls(); return 0;
        case kSaveButton: saveSettingsFromControls(); return 0;
        case kCancelButton:
            SetWindowTextW(g.apiKeyEdit, L"");
            if (g.hasConfig) ShowWindow(window, SW_HIDE); else DestroyWindow(window);
            return 0;
        case kMenuOpen: PostMessageW(window, kOpenBrowser, 0, 0); return 0;
        case kMenuSettings: showSettingsWindow(); return 0;
        case kMenuRestart:
            if (!startService(false)) showSettingsWindow();
            return 0;
        case kMenuExit: DestroyWindow(window); return 0;
        }
        break;
    case kTrayMessage:
        if (LOWORD(lParam) == WM_LBUTTONDBLCLK) PostMessageW(window, kOpenBrowser, 0, 0);
        else if (LOWORD(lParam) == WM_CONTEXTMENU || LOWORD(lParam) == WM_RBUTTONUP) showTrayMenu();
        return 0;
    case kServiceReady:
        updateTrayTip(L"钢一定制AI - 运行中");
        if (g.openWhenReady) openBrowser();
        return 0;
    case kServiceFailed:
        showFailure(L"本地服务未能启动，请通过托盘菜单重试，并查看日志。\n\n日志位置：%LOCALAPPDATA%\\GangyiAI\\logs\\gangyiAI.log");
        return 0;
    case kOpenBrowser:
        if (processRunning()) openBrowser(); else if (!startService(true)) showSettingsWindow();
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
        EnableWindow(g.saveButton, TRUE);
        SetWindowTextW(g.statusLabel, message.c_str());
        MessageBoxW(window, message.c_str(), L"钢一定制AI - 连接测试",
            MB_OK | (succeeded ? MB_ICONINFORMATION : MB_ICONWARNING));
        return 0;
    }
    case WM_CLOSE:
        SetWindowTextW(g.apiKeyEdit, L"");
        if (g.hasConfig) ShowWindow(window, SW_HIDE); else DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        g.shuttingDown = true;
        if (g.connectionTestThread.joinable()) g.connectionTestThread.join();
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
    return 0;
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int) {
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
    if (arguments.find(L"--self-test") != std::wstring::npos) {
        const int result = selfTest();
        WSACleanup();
        CoUninitialize();
        return result;
    }

    HANDLE mutex = CreateMutexW(nullptr, TRUE, kMutexName);
    if (!mutex || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (HWND existing = FindWindowW(kWindowClass, nullptr)) PostMessageW(existing, kOpenBrowser, 0, 0);
        if (mutex) CloseHandle(mutex);
        WSACleanup();
        CoUninitialize();
        return 0;
    }

    const std::wstring exe = executablePath();
    g.installDir = std::filesystem::path(exe).parent_path().wstring();
    g.dataDir = (std::filesystem::path(localAppDataPath()) / L"GangyiAI").wstring();
    g.background = arguments.find(L"--background") != std::wstring::npos;
    g.settings = loadSettings();
    std::wstring key = readApiKey();
    g.hasConfig = validBaseUrl(g.settings.baseUrl) && !g.settings.model.empty() && !key.empty();
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

    g.window = CreateWindowExW(0, kWindowClass, L"钢一定制AI 配置", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 350, nullptr, nullptr, instance, nullptr);
    if (!g.window) return 1;
    if (g.hasConfig) {
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

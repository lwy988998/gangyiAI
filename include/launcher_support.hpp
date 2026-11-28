// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <array>，为后续实现提供类型或接口声明。
#include <array>
// 【逐行批注】 引入头文件 <cstdint>，为后续实现提供类型或接口声明。
#include <cstdint>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <optional>，为后续实现提供类型或接口声明。
#include <optional>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi::launcher {。
namespace gangyi::launcher {

// 【逐行批注】 声明数据结构或类型 AIProvider 及其成员接口。
enum class AIProvider { DeepSeek, OpenAI, Custom };

// 【逐行批注】 声明数据结构或类型 ProviderProfile 及其成员接口。
struct ProviderProfile {
    // 【逐行批注】 执行当前声明或表达式：AIProvider provider;。
    AIProvider provider;
    // 【逐行批注】 执行当前声明或表达式：const wchar_t* id;。
    const wchar_t* id;
    // 【逐行批注】 执行当前声明或表达式：const wchar_t* name;。
    const wchar_t* name;
    // 【逐行批注】 执行当前声明或表达式：const wchar_t* baseUrl;。
    const wchar_t* baseUrl;
    // 【逐行批注】 执行当前声明或表达式：const wchar_t* recommendedModel;。
    const wchar_t* recommendedModel;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 调用 providerProfile 完成本行所需操作：const ProviderProfile& providerProfile(AIProvider provider);。
const ProviderProfile& providerProfile(AIProvider provider);
// 【逐行批注】 声明对象 providerFromId，并通过构造函数初始化其状态。
AIProvider providerFromId(const std::wstring& id);
// 【逐行批注】 声明对象 inferProvider，并通过构造函数初始化其状态。
AIProvider inferProvider(const std::wstring& baseUrl);
// 【逐行批注】 声明对象 sanitizeBaseUrl，并通过构造函数初始化其状态。
std::wstring sanitizeBaseUrl(const std::wstring& baseUrl);
// 【逐行批注】 声明数据结构或类型 RestartPolicy 及其成员接口。
class RestartPolicy {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 recordFailure，并通过构造函数初始化其状态。
    std::optional<unsigned> recordFailure();
    // 【逐行批注】 声明对象 reset，并通过构造函数初始化其状态。
    void reset();
    // 【逐行批注】 声明对象 failureCount，并通过构造函数初始化其状态。
    int failureCount() const;

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 计算右侧结果并写入 int failures_，更新当前运行状态。
    int failures_ = 0;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明对象 rotateLogFiles，并通过构造函数初始化其状态。
bool rotateLogFiles(const std::filesystem::path& activeLog, std::uintmax_t maxBytes = 5u * 1024u * 1024u,
                    // 【逐行批注】 计算右侧结果并写入 int retainedFiles，更新当前运行状态。
                    int retainedFiles = 3);
// 【逐行批注】 声明对象 validateDatabase，并通过构造函数初始化其状态。
bool validateDatabase(const std::filesystem::path& path, std::wstring& error);
// 【逐行批注】 声明对象 backupDatabase，并通过构造函数初始化其状态。
bool backupDatabase(const std::filesystem::path& source, const std::filesystem::path& destination,
                    // 【逐行批注】 执行当前声明或表达式：std::wstring& error);。
                    std::wstring& error);
// 【逐行批注】 声明对象 restoreDatabase，并通过构造函数初始化其状态。
bool restoreDatabase(const std::filesystem::path& backup, const std::filesystem::path& destination,
                     // 【逐行批注】 执行当前声明或表达式：const std::filesystem::path& rollback, std::wstring& error);。
                     const std::filesystem::path& rollback, std::wstring& error);

// 【逐行批注】 声明数据结构或类型 DiagnosticInfo 及其成员接口。
struct DiagnosticInfo {
    // 【逐行批注】 执行当前声明或表达式：std::wstring version;。
    std::wstring version;
    // 【逐行批注】 执行当前声明或表达式：std::filesystem::path installDir;。
    std::filesystem::path installDir;
    // 【逐行批注】 执行当前声明或表达式：std::filesystem::path dataDir;。
    std::filesystem::path dataDir;
    // 【逐行批注】 执行当前声明或表达式：std::wstring provider;。
    std::wstring provider;
    // 【逐行批注】 执行当前声明或表达式：std::wstring baseUrl;。
    std::wstring baseUrl;
    // 【逐行批注】 执行当前声明或表达式：std::wstring model;。
    std::wstring model;
    // 【逐行批注】 计算右侧结果并写入 bool serviceRunning，更新当前运行状态。
    bool serviceRunning = false;
    // 【逐行批注】 计算右侧结果并写入 int port，更新当前运行状态。
    int port = 0;
    // 【逐行批注】 计算右侧结果并写入 unsigned long exitCode，更新当前运行状态。
    unsigned long exitCode = 0;
    // 【逐行批注】 计算右侧结果并写入 int restartFailures，更新当前运行状态。
    int restartFailures = 0;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 主窗口几何：用物理像素记录位置与大小，供跨启动恢复使用。
// 【逐行批注】 声明数据结构或类型 WindowRect 及其成员接口。
struct WindowRect {
    // 【逐行批注】 计算右侧结果并写入 int x，更新当前运行状态。
    int x = 0;
    // 【逐行批注】 计算右侧结果并写入 int y，更新当前运行状态。
    int y = 0;
    // 【逐行批注】 计算右侧结果并写入 int width，更新当前运行状态。
    int width = 0;
    // 【逐行批注】 计算右侧结果并写入 int height，更新当前运行状态。
    int height = 0;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 WindowPlacement 及其成员接口。
struct WindowPlacement {
    // 【逐行批注】 计算右侧结果并写入 int x，更新当前运行状态。
    int x = 0;
    // 【逐行批注】 计算右侧结果并写入 int y，更新当前运行状态。
    int y = 0;
    // 【逐行批注】 计算右侧结果并写入 int width，更新当前运行状态。
    int width = 0;
    // 【逐行批注】 计算右侧结果并写入 int height，更新当前运行状态。
    int height = 0;
    // 【逐行批注】 计算右侧结果并写入 bool maximized，更新当前运行状态。
    bool maximized = false;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 默认取工作区九成并居中；保存值可见且不小于最小尺寸时优先沿用，超出当前工作区则收缩进去。
// 【逐行批注】 声明对象 computeWindowPlacement，并通过构造函数初始化其状态。
WindowPlacement computeWindowPlacement(const WindowRect& workArea, bool hasSavedGeometry,
                                       // 【逐行批注】 执行当前声明或表达式：const WindowRect& savedGeometry, bool savedVisible,。
                                       const WindowRect& savedGeometry, bool savedVisible,
                                       // 【逐行批注】 执行当前声明或表达式：bool savedMaximized);。
                                       bool savedMaximized);

// 【逐行批注】 声明对象 writeDiagnosticReport，并通过构造函数初始化其状态。
bool writeDiagnosticReport(const std::filesystem::path& destination, const DiagnosticInfo& info,
                           // 【逐行批注】 执行当前声明或表达式：std::wstring& error);。
                           std::wstring& error);

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi::launcher。
}  // namespace gangyi::launcher

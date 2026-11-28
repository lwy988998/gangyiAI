// 【逐行批注】 引入头文件 "ai_activity.hpp"，为后续实现提供类型或接口声明。
#include "ai_activity.hpp"
// 【逐行批注】 引入头文件 <sqlite3.h>，为后续实现提供类型或接口声明。
#include <sqlite3.h>
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <atomic>，为后续实现提供类型或接口声明。
#include <atomic>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
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
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
// 【逐行批注】 处理条件编译指令：#ifndef NOMINMAX。
#ifndef NOMINMAX
// 【逐行批注】 定义编译期常量或宏：#define NOMINMAX。
#define NOMINMAX
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 <windows.h>，为后续实现提供类型或接口声明。
#include <windows.h>
// 【逐行批注】 处理条件编译指令：#else。
#else
// 【逐行批注】 引入头文件 <fcntl.h>，为后续实现提供类型或接口声明。
#include <fcntl.h>
// 【逐行批注】 引入头文件 <signal.h>，为后续实现提供类型或接口声明。
#include <signal.h>
// 【逐行批注】 引入头文件 <sys/file.h>，为后续实现提供类型或接口声明。
#include <sys/file.h>
// 【逐行批注】 引入头文件 <unistd.h>，为后续实现提供类型或接口声明。
#include <unistd.h>
// 【逐行批注】 处理条件编译指令：#endif。
#endif

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 执行当前声明或表达式：std::mutex stateMutex;。
std::mutex stateMutex;
// 【逐行批注】 执行当前声明或表达式：std::filesystem::path statePath;。
std::filesystem::path statePath;
// 【逐行批注】 执行当前声明或表达式：std::string usageDatabase;。
std::string usageDatabase;
// 【逐行批注】 执行当前声明或表达式：Json memoryState;。
Json memoryState;
// 【逐行批注】 执行当前声明或表达式：std::atomic<unsigned long long> sequence{0};。
std::atomic<unsigned long long> sequence{0};
// 【逐行批注】 执行当前声明或表达式：std::atomic_bool closing{false};。
std::atomic_bool closing{false};
// 有界保留终态起点供同进程重试沿用；容量不足时终态优先淘汰。
// 【逐行批注】 计算右侧结果并写入 constexpr std::size_t kMaxTaskClocks，更新当前运行状态。
constexpr std::size_t kMaxTaskClocks = 1024;
// 【逐行批注】 声明数据结构或类型 TaskClock 及其成员接口。
struct TaskClock { std::chrono::steady_clock::time_point queued; Json metrics = Json::object(); bool finished = false; };
// 【逐行批注】 执行当前声明或表达式：std::map<std::string, TaskClock> taskClocks;。
std::map<std::string, TaskClock> taskClocks;
// 【逐行批注】 定义函数 markTaskTime，执行该函数声明的职责。
void markTaskTime(const std::string& id, const char* field) {
    // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
    const auto found = taskClocks.find(id);
    // 【逐行批注】 判断条件 (found == taskClocks.end() || found->second.metrics.contains(fie…，成立时执行对应分支。
    if (found == taskClocks.end() || found->second.metrics.contains(field)) return;
    // 【逐行批注】 执行当前声明或表达式：found->second.metrics[field] = std::chrono::duration_cast<std::chrono::milliseconds>(。
    found->second.metrics[field] = std::chrono::duration_cast<std::chrono::milliseconds>(
        // 【逐行批注】 调用 now 完成本行所需操作：std::chrono::steady_clock::now() - found->second.queued).count();。
        std::chrono::steady_clock::now() - found->second.queued).count();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 emptyState，执行该函数声明的职责。
Json emptyState() { return {{"paused", false}, {"pauseEpoch", 0}, {"todayCalls", 0}, {"tasks", Json::array()}}; }
// 【逐行批注】 定义函数 stamp，执行该函数声明的职责。
std::string stamp(bool day = false) {
    // 【逐行批注】 计算右侧结果并写入 auto now，更新当前运行状态。
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // 【逐行批注】 执行当前声明或表达式：std::tm date{};。
    std::tm date{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 判断条件 (day) localtime_s(&date, &now); else gmtime_s(&date, &now);，成立时执行对应分支。
    if (day) localtime_s(&date, &now); else gmtime_s(&date, &now);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 判断条件 (day) localtime_r(&now, &date); else gmtime_r(&now, &date);，成立时执行对应分支。
    if (day) localtime_r(&now, &date); else gmtime_r(&now, &date);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 调用 put_time 完成本行所需操作：std::ostringstream out; out << std::put_time(&date, day ? "%Y-%m-%d" …。
    std::ostringstream out; out << std::put_time(&date, day ? "%Y-%m-%d" : "%Y-%m-%dT%H:%M:%SZ"); return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 processId，执行该函数声明的职责。
long processId() {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 结束当前函数并返回结果：return static_cast<long>(GetCurrentProcessId());。
    return static_cast<long>(GetCurrentProcessId());
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 结束当前函数并返回结果：return static_cast<long>(getpid());。
    return static_cast<long>(getpid());
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 processAlive，执行该函数声明的职责。
bool processAlive(long pid) {
    // 【逐行批注】 判断条件 (pid == processId()) return true;，成立时执行对应分支。
    if (pid == processId()) return true;
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 计算右侧结果并写入 auto handle，更新当前运行状态。
    const auto handle = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));
    // 【逐行批注】 判断条件 (!handle) return false;，成立时执行对应分支。
    if (!handle) return false;
    // 【逐行批注】 计算右侧结果并写入 bool alive，更新当前运行状态。
    const bool alive = WaitForSingleObject(handle, 0) == WAIT_TIMEOUT; CloseHandle(handle); return alive;
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 结束当前函数并返回结果：return kill(pid, 0) == 0 || errno == EPERM;。
    return kill(pid, 0) == 0 || errno == EPERM;
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 声明数据结构或类型 FileLock 及其成员接口。
class FileLock {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 定义函数 FileLock，执行该函数声明的职责。
    explicit FileLock(const std::filesystem::path& path) {
        // 【逐行批注】 判断条件 (path.empty()) return;，成立时执行对应分支。
        if (path.empty()) return;
        // 【逐行批注】 计算右侧结果并写入 lockPath，更新当前运行状态。
        auto lockPath = path; lockPath += ".lock";
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
        // 【逐行批注】 计算右侧结果并写入 handle_，更新当前运行状态。
        handle_ = CreateFileW(lockPath.c_str(), GENERIC_READ | GENERIC_WRITE,
            // 【逐行批注】 执行当前声明或表达式：FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nul…。
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        // 【逐行批注】 判断条件 (handle_ == INVALID_HANDLE_VALUE || !LockFileEx(handle_, LOCKFIL…，成立时执行对应分支。
        if (handle_ == INVALID_HANDLE_VALUE || !LockFileEx(handle_, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &overlap_)) {
            // 【逐行批注】 判断条件 (handle_ != INVALID_HANDLE_VALUE) CloseHandle(handle_);，成立时执行对应分支。
            if (handle_ != INVALID_HANDLE_VALUE) CloseHandle(handle_);
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("无法锁定 AI 调用记录");。
            throw std::runtime_error("无法锁定 AI 调用记录");
        // 【逐行批注】 结束当前代码块。
        }
// 【逐行批注】 处理条件编译指令：#else。
#else
        // 【逐行批注】 计算右侧结果并写入 handle_，更新当前运行状态。
        handle_ = open(lockPath.c_str(), O_CREAT | O_RDWR, 0600);
        // 【逐行批注】 判断条件 (handle_ < 0 || flock(handle_, LOCK_EX) != 0) {，成立时执行对应分支。
        if (handle_ < 0 || flock(handle_, LOCK_EX) != 0) {
            // 【逐行批注】 判断条件 (handle_ >= 0) close(handle_);，成立时执行对应分支。
            if (handle_ >= 0) close(handle_);
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("无法锁定 AI 调用记录");。
            throw std::runtime_error("无法锁定 AI 调用记录");
        // 【逐行批注】 结束当前代码块。
        }
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 FileLock 完成本行所需操作：~FileLock() {。
    ~FileLock() {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
        // 【逐行批注】 判断条件 (handle_ != INVALID_HANDLE_VALUE) { UnlockFileEx(handle_, 0, 1, …，成立时执行对应分支。
        if (handle_ != INVALID_HANDLE_VALUE) { UnlockFileEx(handle_, 0, 1, 0, &overlap_); CloseHandle(handle_); }
// 【逐行批注】 处理条件编译指令：#else。
#else
        // 【逐行批注】 判断条件 (handle_ >= 0) { flock(handle_, LOCK_UN); close(handle_); }，成立时执行对应分支。
        if (handle_ >= 0) { flock(handle_, LOCK_UN); close(handle_); }
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 FileLock(const FileLock&)，更新当前运行状态。
    FileLock(const FileLock&) = delete;
    // 【逐行批注】 计算右侧结果并写入 FileLock& operator，更新当前运行状态。
    FileLock& operator=(const FileLock&) = delete;
// 【逐行批注】 执行当前声明或表达式：private:。
private:
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 计算右侧结果并写入 HANDLE handle_，更新当前运行状态。
    HANDLE handle_ = INVALID_HANDLE_VALUE;
    // 【逐行批注】 执行当前声明或表达式：OVERLAPPED overlap_{};。
    OVERLAPPED overlap_{};
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 计算右侧结果并写入 int handle_，更新当前运行状态。
    int handle_ = -1;
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 定义函数 readState，执行该函数声明的职责。
Json readState() {
    // 【逐行批注】 判断条件 (statePath.empty()) return memoryState.is_object() ? memoryState…，成立时执行对应分支。
    if (statePath.empty()) return memoryState.is_object() ? memoryState : emptyState();
    // 【逐行批注】 判断条件 (!std::filesystem::exists(statePath)) return emptyState();，成立时执行对应分支。
    if (!std::filesystem::exists(statePath)) return emptyState();
    // 【逐行批注】 声明对象 input，并通过构造函数初始化其状态。
    std::ifstream input(statePath);
    // 【逐行批注】 计算右侧结果并写入 state，更新当前运行状态。
    auto state = Json::parse(input, nullptr, false);
    // 控制文件损坏时禁止发出请求，不能悄悄丢掉用户的暂停决定。
    // 【逐行批注】 判断条件 (!state.is_object() || !state.value("tasks", Json()).is_array() …，成立时执行对应分支。
    if (!state.is_object() || !state.value("tasks", Json()).is_array() ||
        // 【逐行批注】 调用 Json、runtime_error 完成本行所需操作：!state.value("paused", Json()).is_boolean()) throw std::runtime_error…。
        !state.value("paused", Json()).is_boolean()) throw std::runtime_error("AI 调用记录损坏，已阻止新请求");
    // 【逐行批注】 结束当前函数并返回结果：return state;。
    return state;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 writeState，执行该函数声明的职责。
void writeState(const Json& state) {
    // 【逐行批注】 判断条件 (statePath.empty()) { memoryState = state; return; }，成立时执行对应分支。
    if (statePath.empty()) { memoryState = state; return; }
    // 【逐行批注】 计算右侧结果并写入 temp，更新当前运行状态。
    auto temp = statePath; temp += ".tmp";
    // 【逐行批注】 调用 output 完成本行所需操作：{ std::ofstream output(temp, std::ios::trunc); output << state.dump()…。
    { std::ofstream output(temp, std::ios::trunc); output << state.dump(); output.flush();
      // 【逐行批注】 判断条件 (!output) throw std::runtime_error("AI 调用记录保存失败"); }，成立时执行对应分支。
      if (!output) throw std::runtime_error("AI 调用记录保存失败"); }
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 判断条件 (!MoveFileExW(temp.c_str(), statePath.c_str(), MOVEFILE_REPLACE_…，成立时执行对应分支。
    if (!MoveFileExW(temp.c_str(), statePath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("AI 调用记录提交失败");。
        throw std::runtime_error("AI 调用记录提交失败");
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 rename 完成本行所需操作：std::filesystem::rename(temp, statePath);。
    std::filesystem::rename(temp, statePath);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 unfinished，执行该函数声明的职责。
bool unfinished(const std::string& status) { return status == "queued" || status == "requesting" || status == "receiving" || status == "paused"; }
// 【逐行批注】 定义函数 clockMillis，执行该函数声明的职责。
long long clockMillis() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
// 【逐行批注】 声明对象 saveUsage，并通过构造函数初始化其状态。
void saveUsage(const Json& row,const std::string& status,const Json& tokens=nullptr,const std::string& model={}) {
    // 【逐行批注】 执行当前声明或表达式：if(usageDatabase.empty())return;。
    if(usageDatabase.empty())return;
    // 【逐行批注】 计算右侧结果并写入 Json value;for(const auto* key:{"taskId","so…，更新当前运行状态。
    Json value;for(const auto* key:{"taskId","source","purpose","courseId","model"})value[key]=row.value(key,"");
    // 【逐行批注】 计算右侧结果并写入 value["day"]，更新当前运行状态。
    value["day"]=row.value("requestDay",stamp(true));value["at"]=row.value("requestAt",stamp());value["status"]=status;
    // 【逐行批注】 计算右侧结果并写入 value["durationMs"]，更新当前运行状态。
    value["durationMs"]=status=="requesting"?0LL:std::max(0LL,clockMillis()-row.value("requestStartedMs",clockMillis()));value["tokens"]=tokens;
    // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"queueMs", "firstContentMs", "completionMs"})。
    for (const auto* field : {"queueMs", "firstContentMs", "completionMs"})
        // 【逐行批注】 判断条件 (row.contains(field)) value[field] = row[field];，成立时执行对应分支。
        if (row.contains(field)) value[field] = row[field];
    // 【逐行批注】 执行当前声明或表达式：if(!model.empty())value["model"]=model;。
    if(!model.empty())value["model"]=model;
    // 请求标识随开始/完成状态一起持久化，控制文件丢失或任务重试也不能覆盖旧用量。
    // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
    const auto key=row.value("requestUsageId", "usage:"+row.at("id").get<std::string>()+":"+std::to_string(row.value("calls",0)));
    // 【逐行批注】 计算右侧结果并写入 sqlite3* connection，更新当前运行状态。
    sqlite3* connection=nullptr;sqlite3_stmt* statement=nullptr;
    // 【逐行批注】 计算右侧结果并写入 auto close，更新当前运行状态。
    const auto close=[&]{sqlite3_finalize(statement);if(connection)sqlite3_close(connection);};
    // 【逐行批注】 调用 sqlite3_open、close、runtime_error 完成本行所需操作：if(sqlite3_open(usageDatabase.c_str(),&connection)!=SQLITE_OK){close(…。
    if(sqlite3_open(usageDatabase.c_str(),&connection)!=SQLITE_OK){close();throw std::runtime_error("无法保存调用统计");}
    // 【逐行批注】 调用 sqlite3_busy_timeout 完成本行所需操作：sqlite3_busy_timeout(connection,5000);。
    sqlite3_busy_timeout(connection,5000);
    // 【逐行批注】 计算右侧结果并写入 auto payload，更新当前运行状态。
    const auto payload=value.dump(),at=stamp();
    // 【逐行批注】 计算右侧结果并写入 char* sql，更新当前运行状态。
    const char* sql="INSERT INTO MyAIRecord(id,kind,payload,updatedAt,version) VALUES(?,'usage',?,?,1) ON CONFLICT(id) DO UPDATE SET payload=excluded.payload,updatedAt=excluded.updatedAt,version=MyAIRecord.version+1";
    // 【逐行批注】 调用 sqlite3_prepare_v2、close、runtime_error 完成本行所需操作：if(sqlite3_prepare_v2(connection,sql,-1,&statement,nullptr)!=SQLITE_O…。
    if(sqlite3_prepare_v2(connection,sql,-1,&statement,nullptr)!=SQLITE_OK){close();throw std::runtime_error("调用统计结构不可用");}
    // 【逐行批注】 调用 sqlite3_bind_text 完成本行所需操作：sqlite3_bind_text(statement,1,key.c_str(),-1,SQLITE_TRANSIENT);sqlite…。
    sqlite3_bind_text(statement,1,key.c_str(),-1,SQLITE_TRANSIENT);sqlite3_bind_text(statement,2,payload.c_str(),-1,SQLITE_TRANSIENT);sqlite3_bind_text(statement,3,at.c_str(),-1,SQLITE_TRANSIENT);
    // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
    const auto result=sqlite3_step(statement);close();if(result!=SQLITE_DONE)throw std::runtime_error("调用统计保存失败");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 AIActivity::configureUsage，执行该函数声明的职责。
void AIActivity::configureUsage(const std::string& path) { std::lock_guard<std::mutex> lock(stateMutex);usageDatabase=path; }
// 【逐行批注】 定义函数 AIActivity::requestFinished，执行该函数声明的职责。
void AIActivity::requestFinished(const std::string& id,const std::string& status,const Json& tokens,const std::string& model,const std::string& receipt) {
    // 【逐行批注】 执行当前声明或表达式：Json request;。
    Json request;
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        std::lock_guard<std::mutex> lock(stateMutex);FileLock file(statePath);const auto state=readState();
        // 【逐行批注】 按循环规则重复处理：for(const auto& row:state["tasks"])if(row.value("id","")==id&&row.contains…。
        for(const auto& row:state["tasks"])if(row.value("id","")==id&&row.contains("requestStartedMs")&&
            // 【逐行批注】 执行当前声明或表达式：(receipt.empty() || row.value("requestUsageId", "") == receipt)){request=row;break;}。
            (receipt.empty() || row.value("requestUsageId", "") == receipt)){request=row;break;}
    // 【逐行批注】 结束当前代码块。
    }
    // 状态文件锁必须先释放；页面在数据库快照中读取暂停状态，不能形成互相等待。
    // 【逐行批注】 调用 saveUsage 完成本行所需操作：if(request.is_object())saveUsage(request,status,tokens,model);。
    if(request.is_object())saveUsage(request,status,tokens,model);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 AIActivity::configure，执行该函数声明的职责。
void AIActivity::configure(const std::string& path) {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex);
    // 【逐行批注】 计算右侧结果并写入 closing，更新当前运行状态。
    closing = false;
    // 【逐行批注】 计算右侧结果并写入 statePath，更新当前运行状态。
    statePath = path.empty() ? std::filesystem::path{} : std::filesystem::absolute(std::filesystem::u8path(path)).lexically_normal();
    // 【逐行批注】 判断条件 (!statePath.empty()) std::filesystem::create_directories(statePa…，成立时执行对应分支。
    if (!statePath.empty()) std::filesystem::create_directories(statePath.parent_path());
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::snapshot，执行该函数声明的职责。
Json AIActivity::snapshot() {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); auto state = readState();
    // 【逐行批注】 判断条件 (state.value("day", "") != stamp(true)) { state["day"] = stamp(t…，成立时执行对应分支。
    if (state.value("day", "") != stamp(true)) { state["day"] = stamp(true); state["todayCalls"] = 0; writeState(state); }
    // 【逐行批注】 按循环规则重复处理：for (auto& task : state["tasks"]) {。
    for (auto& task : state["tasks"]) {
        // 【逐行批注】 判断条件 (unfinished(task.value("status", "")) && !processAlive(task.valu…，成立时执行对应分支。
        if (unfinished(task.value("status", "")) && !processAlive(task.value("processId", 0L))) task["status"] = "interrupted";
        // 【逐行批注】 判断条件 (state.value("paused", false) && unfinished(task.value("status",…，成立时执行对应分支。
        if (state.value("paused", false) && unfinished(task.value("status", ""))) task["status"] = "paused";
        // 【逐行批注】 执行当前声明或表达式：task.erase("processId");。
        task.erase("processId");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return state;。
    return state;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::setPaused，执行该函数声明的职责。
void AIActivity::setPaused(bool paused) {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); auto state = readState();
    // 【逐行批注】 判断条件 (paused && !state.value("paused", false)) state["pauseEpoch"] = …，成立时执行对应分支。
    if (paused && !state.value("paused", false)) state["pauseEpoch"] = state.value("pauseEpoch", std::uint64_t{0}) + 1;
    // 【逐行批注】 计算右侧结果并写入 state["paused"]，更新当前运行状态。
    state["paused"] = paused; writeState(state);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::paused，执行该函数声明的职责。
bool AIActivity::paused() {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); return readState().value("paused", false);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::pauseEpoch，执行该函数声明的职责。
std::uint64_t AIActivity::pauseEpoch() {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); return readState().value("pauseEpoch", std::uint64_t{0});
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::begin，执行该函数声明的职责。
std::string AIActivity::begin(const AIActivityInfo& info, const std::string& model) {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); auto state = readState();
    // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
    const auto id = info.taskId.empty() ? "call-" + std::to_string(processId()) + "-" +
        // 【逐行批注】 调用 to_string、now 完成本行所需操作：std::to_string(std::chrono::system_clock::now().time_since_epoch().co…。
        std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "-" + std::to_string(++sequence) : info.taskId;
    // 【逐行批注】 计算右侧结果并写入 auto& rows，更新当前运行状态。
    auto& rows = state["tasks"]; Json* row = nullptr;
    // 【逐行批注】 按循环规则重复处理：for (auto& value : rows) if (value.value("id", "") == id) { row = &value; …。
    for (auto& value : rows) if (value.value("id", "") == id) { row = &value; break; }
    // 【逐行批注】 判断条件 (!row) {，成立时执行对应分支。
    if (!row) {
        // 只清理已经结束的元数据，任何进行中的请求都不能从列表消失。
        // 【逐行批注】 在条件成立期间循环处理：while (rows.size() >= 200) {。
        while (rows.size() >= 200) {
            // 【逐行批注】 计算右侧结果并写入 old，更新当前运行状态。
            auto old = std::find_if(rows.begin(), rows.end(), [](const Json& value) { return !unfinished(value.value("status", "")); });
            // 【逐行批注】 判断条件 (old == rows.end()) break;，成立时执行对应分支。
            if (old == rows.end()) break;
            // 【逐行批注】 执行当前声明或表达式：rows.erase(old);。
            rows.erase(old);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 rows.push_back({{"id", id}, {"calls", 0}, {"…，更新当前运行状态。
        rows.push_back({{"id", id}, {"calls", 0}, {"legacyCalls", info.legacyCalls}, {"createdAt", stamp()}}); row = &rows.back();
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"queueMs", "firstContentMs", "completionMs"}) ro…。
    for (const auto* field : {"queueMs", "firstContentMs", "completionMs"}) row->erase(field);
    // 【逐行批注】 计算右侧结果并写入 auto timing，更新当前运行状态。
    const auto timing = taskClocks.find(id);
    // 【逐行批注】 判断条件 (timing != taskClocks.end()) for (const auto& metric : timing->s…，成立时执行对应分支。
    if (timing != taskClocks.end()) for (const auto& metric : timing->second.metrics.items()) (*row)[metric.key()] = metric.value();
    // 【逐行批注】 计算右侧结果并写入 (*row)["taskId"]，更新当前运行状态。
    (*row)["taskId"] = info.taskId; (*row)["source"] = info.source; (*row)["purpose"] = info.purpose;
    // 【逐行批注】 计算右侧结果并写入 (*row)["scopeId"]，更新当前运行状态。
    (*row)["scopeId"] = info.scopeId; (*row)["courseId"] = info.courseId; (*row)["lessonId"] = info.lessonId;
    // 【逐行批注】 计算右侧结果并写入 (*row)["model"]，更新当前运行状态。
    (*row)["model"] = model; (*row)["processId"] = processId(); (*row)["updatedAt"] = stamp();
    // 【逐行批注】 计算右侧结果并写入 (*row)["status"]，更新当前运行状态。
    (*row)["status"] = state.value("paused", false) ? "paused" : "queued";
    // 【逐行批注】 调用 writeState 完成本行所需操作：writeState(state); return id;。
    writeState(state); return id;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::requestStarted，执行该函数声明的职责。
bool AIActivity::requestStarted(const std::string& id, std::string* receipt) {
    // 【逐行批注】 执行当前声明或表达式：Json request;。
    Json request;
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        std::lock_guard<std::mutex> lock(stateMutex);FileLock file(statePath);auto state=readState();
        // 【逐行批注】 执行当前声明或表达式：if(state.value("paused",false))return false;。
        if(state.value("paused",false))return false;
        // 【逐行批注】 调用 stamp 完成本行所需操作：if(state.value("day","")!=stamp(true)){state["day"]=stamp(true);state…。
        if(state.value("day","")!=stamp(true)){state["day"]=stamp(true);state["todayCalls"]=0;}
        // 【逐行批注】 按循环规则重复处理：for(auto& row:state["tasks"])if(row.value("id","")==id){。
        for(auto& row:state["tasks"])if(row.value("id","")==id){
            // 【逐行批注】 计算右侧结果并写入 row["status"]，更新当前运行状态。
            row["status"]="requesting";row["calls"]=row.value("calls",0)+1;row["updatedAt"]=stamp();
            // 【逐行批注】 计算右侧结果并写入 row["requestStartedMs"]，更新当前运行状态。
            row["requestStartedMs"]=clockMillis();row["requestAt"]=stamp();row["requestDay"]=stamp(true);
            // 【逐行批注】 计算右侧结果并写入 row["requestUsageId"]，更新当前运行状态。
            row["requestUsageId"]="usage:"+id+":request-"+std::to_string(processId())+"-"+
                // 【逐行批注】 调用 to_string、now 完成本行所需操作：std::to_string(std::chrono::system_clock::now().time_since_epoch().co…。
                std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+"-"+std::to_string(++sequence);
            // 【逐行批注】 计算右侧结果并写入 state["todayCalls"]，更新当前运行状态。
            state["todayCalls"]=state.value("todayCalls",0)+1;request=row;writeState(state);break;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 runtime_error 完成本行所需操作：if(!request.is_object())throw std::runtime_error("AI请求缺少调用记录");。
    if(!request.is_object())throw std::runtime_error("AI请求缺少调用记录");
    // 【逐行批注】 判断条件 (receipt) *receipt = request.at("requestUsageId").get<std::strin…，成立时执行对应分支。
    if (receipt) *receipt = request.at("requestUsageId").get<std::string>();
    // 【逐行批注】 调用 saveUsage 完成本行所需操作：saveUsage(request,"requesting");return true;。
    saveUsage(request,"requesting");return true;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 AIActivity::update，执行该函数声明的职责。
void AIActivity::update(const std::string& id, const std::string& status, const std::string& model) {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); auto state = readState();
    // 【逐行批注】 按循环规则重复处理：for (auto& row : state["tasks"]) if (row.value("id", "") == id) {。
    for (auto& row : state["tasks"]) if (row.value("id", "") == id) {
        // 【逐行批注】 判断条件 (row.value("status", "") == status && (model.empty() || row.valu…，成立时执行对应分支。
        if (row.value("status", "") == status && (model.empty() || row.value("model", "") == model)) return;
        // 【逐行批注】 判断条件 (!model.empty()) row["model"] = model;，成立时执行对应分支。
        if (!model.empty()) row["model"] = model;
        // 【逐行批注】 计算右侧结果并写入 row["status"]，更新当前运行状态。
        row["status"] = status; row["updatedAt"] = stamp(); writeState(state); return;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::shutdown，执行该函数声明的职责。
void AIActivity::shutdown() { closing = true; }
// 【逐行批注】 定义函数 AIActivity::stopping，执行该函数声明的职责。
bool AIActivity::stopping() { return closing.load(); }
// 【逐行批注】 定义函数 AIActivity::taskQueued，执行该函数声明的职责。
void AIActivity::taskQueued(const std::string& id) {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex);
    // 【逐行批注】 判断条件 (taskClocks.count(id)) return;，成立时执行对应分支。
    if (taskClocks.count(id)) return;
    // 【逐行批注】 判断条件 (taskClocks.size() >= kMaxTaskClocks) {，成立时执行对应分支。
    if (taskClocks.size() >= kMaxTaskClocks) {
        // 【逐行批注】 计算右侧结果并写入 auto oldest，更新当前运行状态。
        const auto oldest = std::min_element(taskClocks.begin(), taskClocks.end(), [](const auto& a, const auto& b) {
            // 【逐行批注】 判断条件 (a.second.finished != b.second.finished) return a.second.finishe…，成立时执行对应分支。
            if (a.second.finished != b.second.finished) return a.second.finished;
            // 【逐行批注】 结束当前函数并返回结果：return a.second.queued < b.second.queued;。
            return a.second.queued < b.second.queued;
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 优先释放终态；删除/长期搁置任务也受总量上限约束。起点淘汰后不重建，后续指标未知。
        // 【逐行批注】 执行当前声明或表达式：taskClocks.erase(oldest);。
        taskClocks.erase(oldest);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 now、object 完成本行所需操作：taskClocks.try_emplace(id, TaskClock{std::chrono::steady_clock::now()…。
    taskClocks.try_emplace(id, TaskClock{std::chrono::steady_clock::now(), Json::object()});
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::taskRunning，执行该函数声明的职责。
void AIActivity::taskRunning(const std::string& id) {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex); markTaskTime(id, "queueMs");
    // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
    const auto found = taskClocks.find(id);
    // 【逐行批注】 判断条件 (found != taskClocks.end()) { found->second.metrics.erase("compl…，成立时执行对应分支。
    if (found != taskClocks.end()) { found->second.metrics.erase("completionMs"); found->second.finished = false; }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::taskContent，执行该函数声明的职责。
void AIActivity::taskContent(const std::string& id) {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex); markTaskTime(id, "firstContentMs");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::taskFinished，执行该函数声明的职责。
void AIActivity::taskFinished(const std::string& id) {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex); markTaskTime(id, "completionMs");
    // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
    const auto found = taskClocks.find(id); if (found != taskClocks.end()) found->second.finished = true;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 AIActivity::taskTiming，执行该函数声明的职责。
Json AIActivity::taskTiming(const std::string& id) {
    // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
    std::lock_guard<std::mutex> lock(stateMutex);
    // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
    const auto found = taskClocks.find(id);
    // 【逐行批注】 结束当前函数并返回结果：return found == taskClocks.end() ? Json::object() : found->second.met…。
    return found == taskClocks.end() ? Json::object() : found->second.metrics;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

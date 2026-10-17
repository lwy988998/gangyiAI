#include "ai_activity.hpp"
#include <sqlite3.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace gangyi {
namespace {
using Json = nlohmann::json;
std::mutex stateMutex;
std::filesystem::path statePath;
std::string usageDatabase;
Json memoryState;
std::atomic<unsigned long long> sequence{0};
std::atomic_bool closing{false};
Json emptyState() { return {{"paused", false}, {"pauseEpoch", 0}, {"todayCalls", 0}, {"tasks", Json::array()}}; }
std::string stamp(bool day = false) {
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm date{};
#ifdef _WIN32
    if (day) localtime_s(&date, &now); else gmtime_s(&date, &now);
#else
    if (day) localtime_r(&now, &date); else gmtime_r(&now, &date);
#endif
    std::ostringstream out; out << std::put_time(&date, day ? "%Y-%m-%d" : "%Y-%m-%dT%H:%M:%SZ"); return out.str();
}
long processId() {
#ifdef _WIN32
    return static_cast<long>(GetCurrentProcessId());
#else
    return static_cast<long>(getpid());
#endif
}
bool processAlive(long pid) {
    if (pid == processId()) return true;
#ifdef _WIN32
    const auto handle = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));
    if (!handle) return false;
    const bool alive = WaitForSingleObject(handle, 0) == WAIT_TIMEOUT; CloseHandle(handle); return alive;
#else
    return kill(pid, 0) == 0 || errno == EPERM;
#endif
}
class FileLock {
public:
    explicit FileLock(const std::filesystem::path& path) {
        if (path.empty()) return;
        auto lockPath = path; lockPath += ".lock";
#ifdef _WIN32
        handle_ = CreateFileW(lockPath.c_str(), GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle_ == INVALID_HANDLE_VALUE || !LockFileEx(handle_, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &overlap_)) {
            if (handle_ != INVALID_HANDLE_VALUE) CloseHandle(handle_);
            throw std::runtime_error("无法锁定 AI 调用记录");
        }
#else
        handle_ = open(lockPath.c_str(), O_CREAT | O_RDWR, 0600);
        if (handle_ < 0 || flock(handle_, LOCK_EX) != 0) {
            if (handle_ >= 0) close(handle_);
            throw std::runtime_error("无法锁定 AI 调用记录");
        }
#endif
    }
    ~FileLock() {
#ifdef _WIN32
        if (handle_ != INVALID_HANDLE_VALUE) { UnlockFileEx(handle_, 0, 1, 0, &overlap_); CloseHandle(handle_); }
#else
        if (handle_ >= 0) { flock(handle_, LOCK_UN); close(handle_); }
#endif
    }
    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;
private:
#ifdef _WIN32
    HANDLE handle_ = INVALID_HANDLE_VALUE;
    OVERLAPPED overlap_{};
#else
    int handle_ = -1;
#endif
};
Json readState() {
    if (statePath.empty()) return memoryState.is_object() ? memoryState : emptyState();
    if (!std::filesystem::exists(statePath)) return emptyState();
    std::ifstream input(statePath);
    auto state = Json::parse(input, nullptr, false);
    // 控制文件损坏时禁止发出请求，不能悄悄丢掉用户的暂停决定。
    if (!state.is_object() || !state.value("tasks", Json()).is_array() ||
        !state.value("paused", Json()).is_boolean()) throw std::runtime_error("AI 调用记录损坏，已阻止新请求");
    return state;
}
void writeState(const Json& state) {
    if (statePath.empty()) { memoryState = state; return; }
    auto temp = statePath; temp += ".tmp";
    { std::ofstream output(temp, std::ios::trunc); output << state.dump(); output.flush();
      if (!output) throw std::runtime_error("AI 调用记录保存失败"); }
#ifdef _WIN32
    if (!MoveFileExW(temp.c_str(), statePath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        throw std::runtime_error("AI 调用记录提交失败");
#else
    std::filesystem::rename(temp, statePath);
#endif
}
bool unfinished(const std::string& status) { return status == "queued" || status == "requesting" || status == "receiving" || status == "paused"; }
long long clockMillis() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
void saveUsage(const Json& row,const std::string& status,const Json& tokens=nullptr,const std::string& model={}) {
    if(usageDatabase.empty())return;
    Json value;for(const auto* key:{"taskId","source","purpose","courseId","model"})value[key]=row.value(key,"");
    value["day"]=row.value("requestDay",stamp(true));value["at"]=row.value("requestAt",stamp());value["status"]=status;
    value["durationMs"]=status=="requesting"?0LL:std::max(0LL,clockMillis()-row.value("requestStartedMs",clockMillis()));value["tokens"]=tokens;
    if(!model.empty())value["model"]=model;
    // 请求标识随开始/完成状态一起持久化，控制文件丢失或任务重试也不能覆盖旧用量。
    const auto key=row.value("requestUsageId", "usage:"+row.at("id").get<std::string>()+":"+std::to_string(row.value("calls",0)));
    sqlite3* connection=nullptr;sqlite3_stmt* statement=nullptr;
    const auto close=[&]{sqlite3_finalize(statement);if(connection)sqlite3_close(connection);};
    if(sqlite3_open(usageDatabase.c_str(),&connection)!=SQLITE_OK){close();throw std::runtime_error("无法保存调用统计");}
    sqlite3_busy_timeout(connection,5000);
    const auto payload=value.dump(),at=stamp();
    const char* sql="INSERT INTO MyAIRecord(id,kind,payload,updatedAt,version) VALUES(?,'usage',?,?,1) ON CONFLICT(id) DO UPDATE SET payload=excluded.payload,updatedAt=excluded.updatedAt,version=MyAIRecord.version+1";
    if(sqlite3_prepare_v2(connection,sql,-1,&statement,nullptr)!=SQLITE_OK){close();throw std::runtime_error("调用统计结构不可用");}
    sqlite3_bind_text(statement,1,key.c_str(),-1,SQLITE_TRANSIENT);sqlite3_bind_text(statement,2,payload.c_str(),-1,SQLITE_TRANSIENT);sqlite3_bind_text(statement,3,at.c_str(),-1,SQLITE_TRANSIENT);
    const auto result=sqlite3_step(statement);close();if(result!=SQLITE_DONE)throw std::runtime_error("调用统计保存失败");
}
}

void AIActivity::configureUsage(const std::string& path) { std::lock_guard<std::mutex> lock(stateMutex);usageDatabase=path; }
void AIActivity::requestFinished(const std::string& id,const std::string& status,const Json& tokens,const std::string& model) {
    Json request;
    {
        std::lock_guard<std::mutex> lock(stateMutex);FileLock file(statePath);const auto state=readState();
        for(const auto& row:state["tasks"])if(row.value("id","")==id&&row.contains("requestStartedMs")){request=row;break;}
    }
    // 状态文件锁必须先释放；页面在数据库快照中读取暂停状态，不能形成互相等待。
    if(request.is_object())saveUsage(request,status,tokens,model);
}

void AIActivity::configure(const std::string& path) {
    std::lock_guard<std::mutex> lock(stateMutex);
    closing = false;
    statePath = path.empty() ? std::filesystem::path{} : std::filesystem::absolute(std::filesystem::u8path(path)).lexically_normal();
    if (!statePath.empty()) std::filesystem::create_directories(statePath.parent_path());
}
Json AIActivity::snapshot() {
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); auto state = readState();
    if (state.value("day", "") != stamp(true)) { state["day"] = stamp(true); state["todayCalls"] = 0; writeState(state); }
    for (auto& task : state["tasks"]) {
        if (unfinished(task.value("status", "")) && !processAlive(task.value("processId", 0L))) task["status"] = "interrupted";
        if (state.value("paused", false) && unfinished(task.value("status", ""))) task["status"] = "paused";
        task.erase("processId");
    }
    return state;
}
void AIActivity::setPaused(bool paused) {
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); auto state = readState();
    if (paused && !state.value("paused", false)) state["pauseEpoch"] = state.value("pauseEpoch", std::uint64_t{0}) + 1;
    state["paused"] = paused; writeState(state);
}
bool AIActivity::paused() {
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); return readState().value("paused", false);
}
std::uint64_t AIActivity::pauseEpoch() {
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); return readState().value("pauseEpoch", std::uint64_t{0});
}
std::string AIActivity::begin(const AIActivityInfo& info, const std::string& model) {
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); auto state = readState();
    const auto id = info.taskId.empty() ? "call-" + std::to_string(processId()) + "-" +
        std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "-" + std::to_string(++sequence) : info.taskId;
    auto& rows = state["tasks"]; Json* row = nullptr;
    for (auto& value : rows) if (value.value("id", "") == id) { row = &value; break; }
    if (!row) {
        // 只清理已经结束的元数据，任何进行中的请求都不能从列表消失。
        while (rows.size() >= 200) {
            auto old = std::find_if(rows.begin(), rows.end(), [](const Json& value) { return !unfinished(value.value("status", "")); });
            if (old == rows.end()) break;
            rows.erase(old);
        }
        rows.push_back({{"id", id}, {"calls", 0}, {"legacyCalls", info.legacyCalls}, {"createdAt", stamp()}}); row = &rows.back();
    }
    (*row)["taskId"] = info.taskId; (*row)["source"] = info.source; (*row)["purpose"] = info.purpose;
    (*row)["scopeId"] = info.scopeId; (*row)["courseId"] = info.courseId; (*row)["lessonId"] = info.lessonId;
    (*row)["model"] = model; (*row)["processId"] = processId(); (*row)["updatedAt"] = stamp();
    (*row)["status"] = state.value("paused", false) ? "paused" : "queued";
    writeState(state); return id;
}
bool AIActivity::requestStarted(const std::string& id) {
    Json request;
    {
        std::lock_guard<std::mutex> lock(stateMutex);FileLock file(statePath);auto state=readState();
        if(state.value("paused",false))return false;
        if(state.value("day","")!=stamp(true)){state["day"]=stamp(true);state["todayCalls"]=0;}
        for(auto& row:state["tasks"])if(row.value("id","")==id){
            row["status"]="requesting";row["calls"]=row.value("calls",0)+1;row["updatedAt"]=stamp();
            row["requestStartedMs"]=clockMillis();row["requestAt"]=stamp();row["requestDay"]=stamp(true);
            row["requestUsageId"]="usage:"+id+":request-"+std::to_string(processId())+"-"+
                std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+"-"+std::to_string(++sequence);
            state["todayCalls"]=state.value("todayCalls",0)+1;request=row;writeState(state);break;
        }
    }
    if(!request.is_object())throw std::runtime_error("AI请求缺少调用记录");
    saveUsage(request,"requesting");return true;
}

void AIActivity::update(const std::string& id, const std::string& status, const std::string& model) {
    std::lock_guard<std::mutex> lock(stateMutex); FileLock file(statePath); auto state = readState();
    for (auto& row : state["tasks"]) if (row.value("id", "") == id) {
        if (row.value("status", "") == status && (model.empty() || row.value("model", "") == model)) return;
        if (!model.empty()) row["model"] = model;
        row["status"] = status; row["updatedAt"] = stamp(); writeState(state); return;
    }
}
void AIActivity::shutdown() { closing = true; }
bool AIActivity::stopping() { return closing.load(); }
}

// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"
// 【逐行批注】 引入头文件 "agent_lessons.hpp"，为后续实现提供类型或接口声明。
#include "agent_lessons.hpp"
// 【逐行批注】 引入头文件 "agent_profiles.hpp"，为后续实现提供类型或接口声明。
#include "agent_profiles.hpp"
// 【逐行批注】 引入头文件 "question_evidence.hpp"，为后续实现提供类型或接口声明。
#include "question_evidence.hpp"
// 【逐行批注】 引入头文件 "teaching_evidence.hpp"，为后续实现提供类型或接口声明。
#include "teaching_evidence.hpp"
// 【逐行批注】 引入头文件 "search_client.hpp"，为后续实现提供类型或接口声明。
#include "search_client.hpp"
// 【逐行批注】 引入头文件 <curl/curl.h>，为后续实现提供类型或接口声明。
#include <curl/curl.h>
// 【逐行批注】 引入头文件 <atomic>，为后续实现提供类型或接口声明。
#include <atomic>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
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
// 【逐行批注】 处理条件编译指令：#endif。
#endif

// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 执行当前声明或表达式：std::atomic_bool evaluationSearchBlocked{false};。
std::atomic_bool evaluationSearchBlocked{false};
// 【逐行批注】 调用 __real_curl_easy_perform 完成本行所需操作：extern "C" CURLcode __real_curl_easy_perform(CURL* handle);。
extern "C" CURLcode __real_curl_easy_perform(CURL* handle);
// 【逐行批注】 调用 __wrap_curl_easy_perform 完成本行所需操作：extern "C" CURLcode __wrap_curl_easy_perform(CURL* handle) {。
extern "C" CURLcode __wrap_curl_easy_perform(CURL* handle) {
    // 【逐行批注】 计算右侧结果并写入 auto mode，更新当前运行状态。
    const auto mode = std::getenv("GANGYI_EVAL_MODE");
    // 【逐行批注】 判断条件 (mode && std::string(mode) == "live") {，成立时执行对应分支。
    if (mode && std::string(mode) == "live") {
        // 【逐行批注】 计算右侧结果并写入 char* url，更新当前运行状态。
        char* url = nullptr; curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &url);
        // 【逐行批注】 计算右侧结果并写入 std::string address，更新当前运行状态。
        const std::string address = url ? url : "";
        // 【逐行批注】 判断条件 (address.find("/tools/search") != std::string::npos || address.f…，成立时执行对应分支。
        if (address.find("/tools/search") != std::string::npos || address.find("/web-search") != std::string::npos) {
            // 两版教学都使用冻结资料；搜索另行验收，不能暗中绕过次数预算。
            // 【逐行批注】 计算右侧结果并写入 evaluationSearchBlocked，更新当前运行状态。
            evaluationSearchBlocked = true; return CURLE_ABORTED_BY_CALLBACK;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return __real_curl_easy_perform(handle);。
    return __real_curl_easy_perform(handle);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 localEvaluationEndpoint，执行该函数声明的职责。
bool localEvaluationEndpoint(const std::string& url) {
    // 【逐行批注】 按循环规则重复处理：for (const auto* prefix : {"http://127.0.0.1:", "http://localhost:", "http…。
    for (const auto* prefix : {"http://127.0.0.1:", "http://localhost:", "http://[::1]:"}) {
        // 【逐行批注】 调用 host 完成本行所需操作：const std::string host(prefix);。
        const std::string host(prefix);
        // 【逐行批注】 判断条件 (url.rfind(host, 0) != 0) continue;，成立时执行对应分支。
        if (url.rfind(host, 0) != 0) continue;
        // 【逐行批注】 计算右侧结果并写入 index，更新当前运行状态。
        auto index = host.size(); const auto start = index;
        // 【逐行批注】 在条件成立期间循环处理：while (index < url.size() && url[index] >= '0' && url[index] <= '9') ++ind…。
        while (index < url.size() && url[index] >= '0' && url[index] <= '9') ++index;
        // 【逐行批注】 结束当前函数并返回结果：return index > start && (index == url.size() || url[index] == '/');。
        return index > start && (index == url.size() || url[index] == '/');
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return false;。
    return false;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 validateEvaluationAuthorization，执行该函数声明的职责。
void validateEvaluationAuthorization(const std::string& endpoint) {
    // 【逐行批注】 计算右侧结果并写入 auto mode，更新当前运行状态。
    const auto mode = std::getenv("GANGYI_EVAL_MODE");
    // 【逐行批注】 判断条件 ((!mode || std::string(mode) != "live") && localEvaluationEndpoi…，成立时执行对应分支。
    if ((!mode || std::string(mode) != "live") && localEvaluationEndpoint(endpoint)) return;
    // 【逐行批注】 计算右侧结果并写入 auto path，更新当前运行状态。
    const auto path = std::getenv("GANGYI_EVAL_DECISION_FILE");
    // 【逐行批注】 计算右侧结果并写入 auto budgetPath，更新当前运行状态。
    const auto budgetPath = std::getenv("GANGYI_EVAL_BUDGET_FILE");
    // 【逐行批注】 计算右侧结果并写入 auto model，更新当前运行状态。
    const auto model = std::getenv("AI_MODEL");
    // 【逐行批注】 计算右侧结果并写入 auto denied，更新当前运行状态。
    const auto denied = [] { throw gangyi::AIClientError("eval_authorization", "本轮授权、冻结配置或累计预算不匹配，未发送请求"); };
    // 【逐行批注】 判断条件 (!mode || std::string(mode) != "live" || !path || !budgetPath ||…，成立时执行对应分支。
    if (!mode || std::string(mode) != "live" || !path || !budgetPath || !model) denied();
    // 【逐行批注】 计算右侧结果并写入 auto read，更新当前运行状态。
    const auto read = [](const std::string& filename) {
        // 【逐行批注】 声明对象 input，并通过构造函数初始化其状态。
        std::ifstream input(std::filesystem::u8path(filename), std::ios::binary);
        // 【逐行批注】 结束当前函数并返回结果：return std::string(std::istreambuf_iterator<char>(input), std::istrea…。
        return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto approved，更新当前运行状态。
    const auto approved = Json::parse(read(path));
    // 【逐行批注】 判断条件 (!approved.value("newPaidCallsAllowedThisRound", false) || appro…，成立时执行对应分支。
    if (!approved.value("newPaidCallsAllowedThisRound", false) || approved.value("baseUrl", "") != endpoint ||
        // 【逐行批注】 调用 denied 完成本行所需操作：approved.value("model", "") != model || endpoint.rfind("https://", 0)…。
        approved.value("model", "") != model || endpoint.rfind("https://", 0) != 0) denied();
    // 【逐行批注】 计算右侧结果并写入 auto original，更新当前运行状态。
    const auto original = approved.at("originalBudgetPath").get<std::string>();
    // 【逐行批注】 判断条件 (gangyi::resourceDigest(read(original)) != approved.at("original…，成立时执行对应分支。
    if (gangyi::resourceDigest(read(original)) != approved.at("originalBudgetSha256")) denied();
    // 【逐行批注】 计算右侧结果并写入 auto active，更新当前运行状态。
    const auto active = std::filesystem::weakly_canonical(std::filesystem::u8path(budgetPath));
    // 【逐行批注】 判断条件 (active != std::filesystem::weakly_canonical(std::filesystem::u8…，成立时执行对应分支。
    if (active != std::filesystem::weakly_canonical(std::filesystem::u8path(approved.at("activeBudgetPath").get<std::string>())) ||
        // 【逐行批注】 调用 weakly_canonical、u8path、denied 完成本行所需操作：active == std::filesystem::weakly_canonical(std::filesystem::u8path(o…。
        active == std::filesystem::weakly_canonical(std::filesystem::u8path(original))) denied();
    // 【逐行批注】 计算右侧结果并写入 auto budget，更新当前运行状态。
    const auto budget = Json::parse(read(budgetPath));
    // 【逐行批注】 计算右侧结果并写入 auto frozen，更新当前运行状态。
    const auto frozen = Json::parse(read(original));
    // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"limitTokens", "usedTokens", "reservedTokens"})。
    for (const auto* field : {"limitTokens", "usedTokens", "reservedTokens"})
        // 【逐行批注】 判断条件 (!budget.at(field).is_number_integer() || budget.at(field).get<l…，成立时执行对应分支。
        if (!budget.at(field).is_number_integer() || budget.at(field).get<long long>() < 0) denied();
    // 【逐行批注】 判断条件 (!approved.at("limitTokens").is_number_integer() || approved.at(…，成立时执行对应分支。
    if (!approved.at("limitTokens").is_number_integer() || approved.at("limitTokens").get<long long>() <= 0 ||
        // 【逐行批注】 执行当前声明或表达式：budget.at("limitTokens") != approved.at("limitTokens") ||。
        budget.at("limitTokens") != approved.at("limitTokens") ||
        // 【逐行批注】 执行当前声明或表达式：approved.at("knownUsedTokens") != frozen.at("usedTokens") || approved.at("unknownRes…。
        approved.at("knownUsedTokens") != frozen.at("usedTokens") || approved.at("unknownReservedTokens") != frozen.at("reservedTokens") ||
        // 【逐行批注】 执行当前声明或表达式：budget.at("usedTokens").get<long long>() < frozen.at("usedTokens").get<long long>() …。
        budget.at("usedTokens").get<long long>() < frozen.at("usedTokens").get<long long>() ||
        // 【逐行批注】 调用 denied 完成本行所需操作：budget.at("reservedTokens").get<long long>() < frozen.at("reservedTok…。
        budget.at("reservedTokens").get<long long>() < frozen.at("reservedTokens").get<long long>()) denied();
// 【逐行批注】 结束当前代码块。
}
// 仅开发验收使用：请求前预留输入字节上界、输出及全部SDK故障/兼容重试，未知用量不退款。
// 【逐行批注】 声明数据结构或类型 EvaluationBudget 及其成员接口。
class EvaluationBudget {
    // 【逐行批注】 计算右侧结果并写入 inline static bool storageFailed_，更新当前运行状态。
    inline static bool storageFailed_ = false;
    // 【逐行批注】 计算右侧结果并写入 std::filesystem::path file_; std::size_t res…，更新当前运行状态。
    std::filesystem::path file_; std::size_t reservation_ = 0, requestUpper_ = 0;
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, std::string> previous_;。
    std::map<std::string, std::string> previous_;
    // 【逐行批注】 定义函数 read，执行该函数声明的职责。
    Json read() const { std::ifstream input(file_); Json value; input >> value; return value; }
    // 【逐行批注】 定义函数 write，执行该函数声明的职责。
    void write(const Json& value) const {
        // 锁覆盖临时写入和原子提交；任何失败均保留旧文件，并使整个驱动拒绝后续请求。
        // 【逐行批注】 计算右侧结果并写入 temporary，更新当前运行状态。
        auto temporary = file_; temporary += ".tmp";
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 声明对象 output，并通过构造函数初始化其状态。
            std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
            // 【逐行批注】 判断条件 (!output) throw std::runtime_error("预算临时文件无法打开");，成立时执行对应分支。
            if (!output) throw std::runtime_error("预算临时文件无法打开");
            // 【逐行批注】 执行当前声明或表达式：output << value.dump(2); output.flush();。
            output << value.dump(2); output.flush();
            // 【逐行批注】 判断条件 (!output) throw std::runtime_error("预算临时文件写入失败");，成立时执行对应分支。
            if (!output) throw std::runtime_error("预算临时文件写入失败");
            // 【逐行批注】 执行当前声明或表达式：output.close();。
            output.close();
            // 【逐行批注】 判断条件 (!output) throw std::runtime_error("预算临时文件关闭失败");，成立时执行对应分支。
            if (!output) throw std::runtime_error("预算临时文件关闭失败");
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
            // 【逐行批注】 判断条件 (!MoveFileExW(temporary.c_str(), file_.c_str(), MOVEFILE_REPLACE…，成立时执行对应分支。
            if (!MoveFileExW(temporary.c_str(), file_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("预算原子提交失败");。
                throw std::runtime_error("预算原子提交失败");
// 【逐行批注】 处理条件编译指令：#else。
#else
            // 【逐行批注】 调用 rename 完成本行所需操作：std::filesystem::rename(temporary, file_);。
            std::filesystem::rename(temporary, file_);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 计算右侧结果并写入 storageFailed_，更新当前运行状态。
            storageFailed_ = true;
            // 【逐行批注】 检测到错误情形时抛出异常：throw gangyi::AIClientError("eval_budget_storage", "预算保存失败，停止全部后续请求");。
            throw gangyi::AIClientError("eval_budget_storage", "预算保存失败，停止全部后续请求");
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 声明数据结构或类型 Lock 及其成员接口。
    struct Lock {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
        // 【逐行批注】 计算右侧结果并写入 HANDLE handle，更新当前运行状态。
        HANDLE handle = INVALID_HANDLE_VALUE;
        // 【逐行批注】 定义函数 Lock，执行该函数声明的职责。
        explicit Lock(const std::filesystem::path& file) {
            // 【逐行批注】 计算右侧结果并写入 auto lockFile，更新当前运行状态。
            const auto lockFile = file.wstring()+L".lock";
            // 【逐行批注】 按循环规则重复处理：for (int attempt=0;attempt<3000;++attempt) {。
            for (int attempt=0;attempt<3000;++attempt) {
                // 【逐行批注】 计算右侧结果并写入 handle，更新当前运行状态。
                handle=CreateFileW(lockFile.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
                // 【逐行批注】 判断条件 (handle!=INVALID_HANDLE_VALUE) return;，成立时执行对应分支。
                if (handle!=INVALID_HANDLE_VALUE) return;
                // 【逐行批注】 调用 Sleep 完成本行所需操作：Sleep(10);。
                Sleep(10);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("验收用量锁暂不可用，未调用模型");。
            throw std::runtime_error("验收用量锁暂不可用，未调用模型");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 Lock、CloseHandle 完成本行所需操作：~Lock() { if(handle!=INVALID_HANDLE_VALUE)CloseHandle(handle); }。
        ~Lock() { if(handle!=INVALID_HANDLE_VALUE)CloseHandle(handle); }
// 【逐行批注】 处理条件编译指令：#else。
#else
        // 【逐行批注】 定义函数 Lock，执行该函数声明的职责。
        explicit Lock(const std::filesystem::path&) { throw std::runtime_error("预算验收入口仅用于本机Windows"); }
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 执行当前声明或表达式：};。
    };
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 定义函数 storageFailed，执行该函数声明的职责。
    static bool storageFailed() { return storageFailed_; }
    // 【逐行批注】 调用 EvaluationBudget 完成本行所需操作：EvaluationBudget(gangyi::Database& db, const gangyi::ChatOptions& opt…。
    EvaluationBudget(gangyi::Database& db, const gangyi::ChatOptions& options) {
        // 【逐行批注】 判断条件 (storageFailed_) throw gangyi::AIClientError("eval_budget_storag…，成立时执行对应分支。
        if (storageFailed_) throw gangyi::AIClientError("eval_budget_storage", "预算保存已失败，禁止继续请求");
        // 【逐行批注】 计算右侧结果并写入 auto path，更新当前运行状态。
        const auto path=std::getenv("GANGYI_EVAL_BUDGET_FILE"); if(!path||!*path)return;
        // 【逐行批注】 计算右侧结果并写入 file_，更新当前运行状态。
        file_=std::filesystem::u8path(path);
        // 【逐行批注】 计算右侧结果并写入 requestUpper_，更新当前运行状态。
        requestUpper_=static_cast<std::size_t>(options.maxTokens)+1024;
        // 【逐行批注】 按循环规则重复处理：for(const auto& message:options.messages)requestUpper_+=message.content.si…。
        for(const auto& message:options.messages)requestUpper_+=message.content.size()+64;
        // 【逐行批注】 计算右侧结果并写入 int attempts，更新当前运行状态。
        int attempts = options.maxAttempts;
        // 【逐行批注】 判断条件 (attempts <= 0) {，成立时执行对应分支。
        if (attempts <= 0) {
            // 【逐行批注】 计算右侧结果并写入 attempts，更新当前运行状态。
            attempts = 2;
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try { if (const auto setting = std::getenv("AI_RETRY_ATTEMPTS")) attempts = std::stoi(setting); } catch (...) {}
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (attempts < 1 || attempts > 100) throw gangyi::AIClientError("ev…，成立时执行对应分支。
        if (attempts < 1 || attempts > 100) throw gangyi::AIClientError("eval_budget", "SDK重试次数不在验收预算支持范围，未调用模型");
        // 【逐行批注】 计算右侧结果并写入 reservation_，更新当前运行状态。
        reservation_=requestUpper_*static_cast<std::size_t>(attempts+1);
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        Lock lock(file_); auto value=read();
        // 【逐行批注】 执行当前声明或表达式：if(value.value("usedTokens",0ULL)+value.value("reservedTokens",0ULL)+reservation_>va…。
        if(value.value("usedTokens",0ULL)+value.value("reservedTokens",0ULL)+reservation_>value.at("limitTokens").get<std::size_t>())
            // 【逐行批注】 检测到错误情形时抛出异常：throw gangyi::AIClientError("eval_budget","验收Token预算不足以预留下一次请求，未调用模型"…。
            throw gangyi::AIClientError("eval_budget","验收Token预算不足以预留下一次请求，未调用模型");
        // 【逐行批注】 计算右侧结果并写入 value["reservedTokens"]，更新当前运行状态。
        value["reservedTokens"]=value.value("reservedTokens",0ULL)+reservation_; write(value);
        // 【逐行批注】 按循环规则重复处理：for(const auto& row:db.myAIRecords("usage"))previous_[row.id]=row.payload;。
        for(const auto& row:db.myAIRecords("usage"))previous_[row.id]=row.payload;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 settle，执行该函数声明的职责。
    void settle(gangyi::Database& db) {
        // 【逐行批注】 执行当前声明或表达式：if(file_.empty()||reservation_==0)return;。
        if(file_.empty()||reservation_==0)return;
        // 【逐行批注】 计算右侧结果并写入 std::size_t tokens，更新当前运行状态。
        std::size_t tokens=0,unknown=0,requests=0;
        // 【逐行批注】 按循环规则重复处理：for(const auto& row:db.myAIRecords("usage"))if(!previous_.count(row.id)||p…。
        for(const auto& row:db.myAIRecords("usage"))if(!previous_.count(row.id)||previous_.at(row.id)!=row.payload) {
            // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
            const auto value=Json::parse(row.payload); ++requests;
            // 【逐行批注】 调用 Json 完成本行所需操作：if(value.value("tokens",Json()).is_object()&&value.at("tokens").value…。
            if(value.value("tokens",Json()).is_object()&&value.at("tokens").value("total_tokens",Json()).is_number())
                // 【逐行批注】 计算右侧结果并写入 tokens+，更新当前运行状态。
                tokens+=value.at("tokens").at("total_tokens").get<std::size_t>();
            // 【逐行批注】 执行前一条件不成立时的分支。
            else ++unknown;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        Lock lock(file_); auto value=read();
        // 【逐行批注】 计算右侧结果并写入 value["reservedTokens"]，更新当前运行状态。
        value["reservedTokens"]=value.value("reservedTokens",0ULL)-reservation_+(requests==0?reservation_:unknown*requestUpper_);
        // 【逐行批注】 计算右侧结果并写入 value["usedTokens"]，更新当前运行状态。
        value["usedTokens"]=value.value("usedTokens",0ULL)+tokens;
        // 【逐行批注】 计算右侧结果并写入 value["httpRequests"]，更新当前运行状态。
        value["httpRequests"]=value.value("httpRequests",0ULL)+requests;
        // 【逐行批注】 计算右侧结果并写入 value["unknownUsageRequests"]，更新当前运行状态。
        value["unknownUsageRequests"]=value.value("unknownUsageRequests",0ULL)+unknown;
        // 【逐行批注】 执行当前声明或表达式：if(tokens+unknown*requestUpper_>reservation_)value["unexpectedUsage"]=true;。
        if(tokens+unknown*requestUpper_>reservation_)value["unexpectedUsage"]=true;
        // 【逐行批注】 计算右侧结果并写入 write(value); reservation_，更新当前运行状态。
        write(value); reservation_=0;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 执行当前声明或表达式：};。
};
// 开发验收入口直接调用现有 C++ 主控；不进入安装包。评分标准只来自隔离用例。
// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main(int argc, char** argv) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (argc == 2 && std::string(argv[1]) == "--check-live-authorizatio…，成立时执行对应分支。
        if (argc == 2 && std::string(argv[1]) == "--check-live-authorization") {
            // 【逐行批注】 计算右侧结果并写入 auto endpoint，更新当前运行状态。
            const auto endpoint = std::getenv("AI_BASE_URL");
            // 【逐行批注】 调用 validateEvaluationAuthorization 完成本行所需操作：validateEvaluationAuthorization(endpoint ? endpoint : "");。
            validateEvaluationAuthorization(endpoint ? endpoint : "");
            // 【逐行批注】 执行当前声明或表达式：std::cout << "{\"authorized\":true,\"httpRequests\":0}" << std::endl; return 0;。
            std::cout << "{\"authorized\":true,\"httpRequests\":0}" << std::endl; return 0;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (argc == 2 && std::string(argv[1]) == "--check-live-search-block…，成立时执行对应分支。
        if (argc == 2 && std::string(argv[1]) == "--check-live-search-block") {
            // 【逐行批注】 计算右侧结果并写入 auto* handle，更新当前运行状态。
            auto* handle = curl_easy_init();
            // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(handle, CURLOPT_URL, "http://127.0.0.1:9/v1/tools/se…。
            curl_easy_setopt(handle, CURLOPT_URL, "http://127.0.0.1:9/v1/tools/search");
            // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
            const auto result = __wrap_curl_easy_perform(handle); curl_easy_cleanup(handle);
            // 【逐行批注】 判断条件 (result != CURLE_ABORTED_BY_CALLBACK || !evaluationSearchBlocked…，成立时执行对应分支。
            if (result != CURLE_ABORTED_BY_CALLBACK || !evaluationSearchBlocked) return 1;
            // 【逐行批注】 执行当前声明或表达式：std::cout << "{\"searchBlocked\":true,\"httpRequests\":0}" << std::endl; return 0;。
            std::cout << "{\"searchBlocked\":true,\"httpRequests\":0}" << std::endl; return 0;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (argc == 3 && std::string(argv[1]) == "--budget-self-test") {，成立时执行对应分支。
        if (argc == 3 && std::string(argv[1]) == "--budget-self-test") {
            // 免费重现旧版本请求ID更新、未知用量和请求前预算拦截。
            // 【逐行批注】 计算右侧结果并写入 auto folder，更新当前运行状态。
            const auto folder = std::filesystem::u8path(argv[2]);
            // 【逐行批注】 判断条件 (!std::filesystem::create_directory(folder)) throw std::runtime_…，成立时执行对应分支。
            if (!std::filesystem::create_directory(folder)) throw std::runtime_error("预算夹具目录已存在");
            // 【逐行批注】 计算右侧结果并写入 auto budgetPath，更新当前运行状态。
            const auto budgetPath = folder / "budget.json";
            // 【逐行批注】 调用 file 完成本行所需操作：{ std::ofstream file(budgetPath); file << Json{{"limitTokens", 100000…。
            { std::ofstream file(budgetPath); file << Json{{"limitTokens", 100000}, {"usedTokens", 0}, {"reservedTokens", 0}}.dump(); }
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
            // 【逐行批注】 调用 _putenv_s 完成本行所需操作：_putenv_s("GANGYI_EVAL_BUDGET_FILE", budgetPath.u8string().c_str());。
            _putenv_s("GANGYI_EVAL_BUDGET_FILE", budgetPath.u8string().c_str());
// 【逐行批注】 处理条件编译指令：#endif。
#endif
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database fixture; fixture.open((folder / "isolated.db").u8string()); fixture…。
            gangyi::Database fixture; fixture.open((folder / "isolated.db").u8string()); fixture.migrate();
            // 【逐行批注】 计算右侧结果并写入 auto save，更新当前运行状态。
            const auto save = [&](Json tokens) {
                // 【逐行批注】 计算右侧结果并写入 auto previous，更新当前运行状态。
                const auto previous = fixture.myAIRecord("usage:legacy-task:1");
                // 【逐行批注】 判断条件 (!fixture.saveMyAIRecord({"usage:legacy-task:1", "usage", Json{{…，成立时执行对应分支。
                if (!fixture.saveMyAIRecord({"usage:legacy-task:1", "usage", Json{{"tokens", tokens}}.dump(), "2026-10-14", 0}, previous ? previous->version : 0))
                    // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("预算夹具用量未保存");。
                    throw std::runtime_error("预算夹具用量未保存");
            // 【逐行批注】 执行当前声明或表达式：};。
            };
            // 【逐行批注】 调用 save 完成本行所需操作：save({{"total_tokens", 5}});。
            save({{"total_tokens", 5}});
            // 【逐行批注】 计算右侧结果并写入 gangyi::ChatOptions options; options.maxToke…，更新当前运行状态。
            gangyi::ChatOptions options; options.maxTokens = 1;
            // 【逐行批注】 调用 budget、save 完成本行所需操作：{ EvaluationBudget budget(fixture, options); save({{"total_tokens", 1…。
            { EvaluationBudget budget(fixture, options); save({{"total_tokens", 11}}); budget.settle(fixture); }
            // 【逐行批注】 调用 budget、save 完成本行所需操作：{ EvaluationBudget budget(fixture, options); save(nullptr); budget.se…。
            { EvaluationBudget budget(fixture, options); save(nullptr); budget.settle(fixture); }
            // 【逐行批注】 计算右侧结果并写入 options.maxTokens，更新当前运行状态。
            options.maxTokens = 100000; bool rejected = false;
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try { EvaluationBudget budget(fixture, options); } catch (const gangyi::AIClientError& error) { rejected = error.errorType == "eval_budget"; }
            // 【逐行批注】 调用 file 完成本行所需操作：Json value; { std::ifstream file(budgetPath); file >> value; }。
            Json value; { std::ifstream file(budgetPath); file >> value; }
            // 【逐行批注】 判断条件 (!rejected || value.at("usedTokens") != 11 || value.at("reserved…，成立时执行对应分支。
            if (!rejected || value.at("usedTokens") != 11 || value.at("reservedTokens") != 1025 ||
                // 【逐行批注】 执行当前声明或表达式：value.at("httpRequests") != 2 || value.at("unknownUsageRequests") != 1)。
                value.at("httpRequests") != 2 || value.at("unknownUsageRequests") != 1)
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("预算统计或请求前拦截不符合预期");。
                throw std::runtime_error("预算统计或请求前拦截不符合预期");
            // 【逐行批注】 执行当前声明或表达式：std::cout << "预算回归通过：旧ID更新计入、未知额度保留、超额请求未执行。\n";。
            std::cout << "预算回归通过：旧ID更新计入、未知额度保留、超额请求未执行。\n";
            // 【逐行批注】 结束当前函数并返回结果：return 0;。
            return 0;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (argc != 4) throw std::invalid_argument("参数：用例JSON 隔离数据库 输出JSON"…，成立时执行对应分支。
        if (argc != 4) throw std::invalid_argument("参数：用例JSON 隔离数据库 输出JSON");
        // 【逐行批注】 声明对象 input，并通过构造函数初始化其状态。
        std::ifstream input(std::filesystem::u8path(argv[1])); Json test; input >> test;
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(argv[2]); db.migrate();。
        gangyi::Database db; db.open(argv[2]); db.migrate();
        // 【逐行批注】 调用 configureUsage 完成本行所需操作：gangyi::AIActivity::configureUsage(argv[2]);。
        gangyi::AIActivity::configureUsage(argv[2]);
        // 【逐行批注】 判断条件 (test.contains("preferences"))，成立时执行对应分支。
        if (test.contains("preferences"))
            // 【逐行批注】 执行当前声明或表达式：db.saveMyAIRecord({"preferences", "preferences", test.at("preferences").dump(), "202…。
            db.saveMyAIRecord({"preferences", "preferences", test.at("preferences").dump(), "2026-10-13", 0}, 0);
        // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess access{"local-profile", {"eval-course"}};。
        const gangyi::AgentAccess access{"local-profile", {"eval-course"}};
        // 【逐行批注】 计算右侧结果并写入 std::string lessonId，更新当前运行状态。
        const std::string lessonId = "eval-lesson", courseId = "eval-course";
        // 【逐行批注】 计算右侧结果并写入 bool resumeExisting，更新当前运行状态。
        const bool resumeExisting = std::getenv("GANGYI_EVAL_RESUME") && db.getClassroomActivity(lessonId).has_value();
        // 【逐行批注】 判断条件 (!resumeExisting) {，成立时执行对应分支。
        if (!resumeExisting) {
        // 【逐行批注】 执行当前声明或表达式：db.insert(gangyi::Course{courseId, access.scopeId, {}, test.at("title"), "deep", tes…。
        db.insert(gangyi::Course{courseId, access.scopeId, {}, test.at("title"), "deep", test.at("title"),
            // 【逐行批注】 执行当前声明或表达式：{}, "ai", "active", "2026-10-13", "2026-10-13"});。
            {}, "ai", "active", "2026-10-13", "2026-10-13"});
        // 两版从同一已保存来源复制固定原文，避免资料变化影响同协议对照。
        // 【逐行批注】 判断条件 (test.value("libraryDatabase", Json()).is_string()) {，成立时执行对应分支。
        if (test.value("libraryDatabase", Json()).is_string()) {
            // 【逐行批注】 执行当前声明或表达式：gangyi::Database sources; sources.open(test.at("libraryDatabase").get<std::string>()…。
            gangyi::Database sources; sources.open(test.at("libraryDatabase").get<std::string>());
            // 【逐行批注】 计算右侧结果并写入 auto ids，更新当前运行状态。
            const auto ids = test.at("librarySourceIds").get<std::vector<std::string>>();
            // 【逐行批注】 按循环规则重复处理：for (const auto& id : ids) {。
            for (const auto& id : ids) {
                // 【逐行批注】 计算右侧结果并写入 auto source，更新当前运行状态。
                const auto source = sources.myAIRecord(id);
                // 【逐行批注】 判断条件 (!source || source->kind != "library-source") throw std::runtime…，成立时执行对应分支。
                if (!source || source->kind != "library-source") throw std::runtime_error("验收原文来源不存在");
                // 【逐行批注】 计算右侧结果并写入 payload，更新当前运行状态。
                auto payload = Json::parse(source->payload); payload["courseIds"] = {courseId};
                // 【逐行批注】 计算右侧结果并写入 auto snapshot，更新当前运行状态。
                const auto snapshot = sources.myAIRecord(payload.at("snapshotId"));
                // 【逐行批注】 判断条件 (!snapshot || snapshot->kind != "library-snapshot") throw std::r…，成立时执行对应分支。
                if (!snapshot || snapshot->kind != "library-snapshot") throw std::runtime_error("验收原文尚未读取");
                // 【逐行批注】 执行当前声明或表达式：db.saveMyAIRecord({source->id, source->kind, payload.dump(), source->updatedAt, 0}, …。
                db.saveMyAIRecord({source->id, source->kind, payload.dump(), source->updatedAt, 0}, 0);
                // 【逐行批注】 执行当前声明或表达式：db.saveMyAIRecord({snapshot->id, snapshot->kind, snapshot->payload, snapshot->update…。
                db.saveMyAIRecord({snapshot->id, snapshot->kind, snapshot->payload, snapshot->updatedAt, 0}, 0);
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 Json sections，更新当前运行状态。
        Json sections = Json::array();
        // 【逐行批注】 执行当前声明或表达式：sections.push_back({{"id", "concept"}, {"kind", "explanation"}, {"title", "学过的相关概念"},。
        sections.push_back({{"id", "concept"}, {"kind", "explanation"}, {"title", "学过的相关概念"},
            // 【逐行批注】 执行当前声明或表达式：{"body", test.at("concept")}, {"version", 1}, {"helpType", "explanation"}});。
            {"body", test.at("concept")}, {"version", 1}, {"helpType", "explanation"}});
        // 【逐行批注】 计算右侧结果并写入 int index，更新当前运行状态。
        int index = 0;
        // 【逐行批注】 按循环规则重复处理：for (const auto& question : test.at("questions")) {。
        for (const auto& question : test.at("questions")) {
            // 【逐行批注】 判断条件 (index > 0 && !test.value("seedSecond", true)) break;，成立时执行对应分支。
            if (index > 0 && !test.value("seedSecond", true)) break;
            // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
            const auto id = "question-" + std::to_string(index++);
            // 【逐行批注】 执行当前声明或表达式：sections.push_back({{"id", id}, {"kind", "question"}, {"title", "实际作答"}, {"questionK…。
            sections.push_back({{"id", id}, {"kind", "question"}, {"title", "实际作答"}, {"questionKind", "practice"},
                // 【逐行批注】 调用 teachingQuestionIdentity 完成本行所需操作：{"version", 1}, {"question", question}, {"questionId", gangyi::teachi…。
                {"version", 1}, {"question", question}, {"questionId", gangyi::teachingQuestionIdentity(courseId, question)}});
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
        Json lesson = {{"id", lessonId}, {"scopeId", access.scopeId}, {"courseId", courseId}, {"title", test.at("title")},
            // 【逐行批注】 执行当前声明或表达式：{"purpose", "依据真实回答诊断并安排独立复练"}, {"status", "ready"}, {"entered", true},。
            {"purpose", "依据真实回答诊断并安排独立复练"}, {"status", "ready"}, {"entered", true},
            // 【逐行批注】 执行当前声明或表达式：{"version", 1}, {"model", "预编验收材料"}, {"updatedAt", "2026-10-13"}, {"sections", secti…。
            {"version", 1}, {"model", "预编验收材料"}, {"updatedAt", "2026-10-13"}, {"sections", sections},
            // 【逐行批注】 执行当前声明或表达式：{"sourceLearningVersion", db.learningRevision()}, {"teachingFocus", {{"practiceSecti…。
            {"sourceLearningVersion", db.learningRevision()}, {"teachingFocus", {{"practiceSectionId", "question-0"}}}};
        // 【逐行批注】 执行当前声明或表达式：db.upsert(gangyi::ClassroomActivity{lessonId, courseId, "agent-lesson", lesson.dump(…。
        db.upsert(gangyi::ClassroomActivity{lessonId, courseId, "agent-lesson", lesson.dump(), "2026-10-13", 0, 0});
        // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"lessonId", lessonId}, {"sectio…。
        gangyi::agentExposeTask(db, access, {{"lessonId", lessonId}, {"sections", {{{"id", "concept"}, {"version", 1}}}}});
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 auto runStarted，更新当前运行状态。
        const auto runStarted = std::chrono::steady_clock::now();
        // 【逐行批注】 计算右侧结果并写入 Json usage，更新当前运行状态。
        Json usage = Json::array(), promptAudit = Json::array();
        // 【逐行批注】 判断条件 (resumeExisting) for (const auto& row : db.myAIRecords("usage"))…，成立时执行对应分支。
        if (resumeExisting) for (const auto& row : db.myAIRecords("usage")) {
            // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
            const auto saved = Json::parse(row.payload);
            // 【逐行批注】 调用 Json 完成本行所需操作：usage.push_back({{"model", saved.value("model", "")}, {"tokenUsage", …。
            usage.push_back({{"model", saved.value("model", "")}, {"tokenUsage", saved.value("tokens", Json())},
                // 【逐行批注】 执行当前声明或表达式：{"durationMs", saved.value("durationMs", 0)}, {"failed", saved.value("status", "") !…。
                {"durationMs", saved.value("durationMs", 0)}, {"failed", saved.value("status", "") != "completed"},
                // 【逐行批注】 执行当前声明或表达式：{"recoveredHttpRecord", true}});。
                {"recoveredHttpRecord", true}});
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
        gangyi::LearningAgent agent(argv[2], [&](const gangyi::ChatOptions& options, const std::function<bool(const std::string&)>& sink) {
            // 【逐行批注】 判断条件 (evaluationSearchBlocked) throw gangyi::AIClientError("eval_sear…，成立时执行对应分支。
            if (evaluationSearchBlocked) throw gangyi::AIClientError("eval_search_mode", "教学对照要求冻结资料；出现新搜索，停止后续模型请求");
            // 【逐行批注】 计算右侧结果并写入 Json prompt，更新当前运行状态。
            Json prompt = {{"purpose", options.activity.purpose}, {"inputBytes", 0}};
            // 【逐行批注】 计算右侧结果并写入 Json messages，更新当前运行状态。
            Json messages = Json::array(); std::size_t bytes = 0;
            // 【逐行批注】 按循环规则重复处理：for (const auto& message : options.messages) {。
            for (const auto& message : options.messages) {
                // 【逐行批注】 计算右侧结果并写入 bytes +，更新当前运行状态。
                bytes += message.content.size(); messages.push_back({{"role", message.role}, {"content", message.content}});
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 prompt["inputBytes"]，更新当前运行状态。
            prompt["inputBytes"] = bytes;
            // 【逐行批注】 判断条件 (test.value("capturePrompts", false)) prompt["messages"] = messa…，成立时执行对应分支。
            if (test.value("capturePrompts", false)) prompt["messages"] = messages;
            // 【逐行批注】 调用 move 完成本行所需操作：promptAudit.push_back(std::move(prompt));。
            promptAudit.push_back(std::move(prompt));
            // 【逐行批注】 判断条件 (test.value("fixtureMode", false)) {，成立时执行对应分支。
            if (test.value("fixtureMode", false)) {
                // 【逐行批注】 计算右侧结果并写入 auto context，更新当前运行状态。
                const auto context = Json::parse(options.messages[1].content);
                // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
                const auto event = context.at("event"); const auto actual = context.at("currentInputs").at(0);
                // 【逐行批注】 计算右侧结果并写入 Json decision，更新当前运行状态。
                Json decision = {{"teaching", {{"helpType", "none"}}}, {"message", "请继续独立作答。"},
                    // 【逐行批注】 调用 array 完成本行所需操作：{"actions", Json::array()}, {"state", "waiting_student"}};。
                    {"actions", Json::array()}, {"state", "waiting_student"}};
                // 【逐行批注】 判断条件 (options.activity.purpose == "判断本次回答或追问") {，成立时执行对应分支。
                if (options.activity.purpose == "判断本次回答或追问") {
                    // 【逐行批注】 执行当前声明或表达式：Json action;。
                    Json action;
                    // 【逐行批注】 判断条件 (event.at("text").get<std::string>().find("为什么") != std::string:…，成立时执行对应分支。
                    if (event.at("text").get<std::string>().find("为什么") != std::string::npos)
                        // 【逐行批注】 计算右侧结果并写入 action，更新当前运行状态。
                        action = {{"id", "classify"}, {"tool", "classify_input"}, {"args", {{"interactionId", actual.at("interactionId")},
                            // 【逐行批注】 执行当前声明或表达式：{"isAnswer", false}, {"reason", "这是方法原因的追问"}}}};。
                            {"isAnswer", false}, {"reason", "这是方法原因的追问"}}}};
                    // 【逐行批注】 执行前一条件不成立时的分支。
                    else {
                        // 【逐行批注】 计算右侧结果并写入 bool incorrect，更新当前运行状态。
                        const bool incorrect = event.at("text").get<std::string>().find("2/5") != std::string::npos;
                        // 【逐行批注】 计算右侧结果并写入 action，更新当前运行状态。
                        action = {{"id", "grade"}, {"tool", "evaluate_answer"}, {"args", {{"interactionId", actual.at("interactionId")},
                            // 【逐行批注】 执行当前声明或表达式：{"score", incorrect ? 20 : 95}, {"credible", true}, {"feedback", incorrect ? "分母不同，先…。
                            {"score", incorrect ? 20 : 95}, {"credible", true}, {"feedback", incorrect ? "分母不同，先统一每份大小，再相加。" : "结果与通分过程正确，再用自己的话说明理由。"},
                            // 【逐行批注】 执行当前声明或表达式：{"feedbackHelpType", incorrect ? "hint" : "none"},。
                            {"feedbackHelpType", incorrect ? "hint" : "none"},
                            // 【逐行批注】 执行当前声明或表达式：{"reason", "依据本题原题和实际回答"}, {"uncertainty", "固定页面演示记录，不代表真实教学验收。"},。
                            {"reason", "依据本题原题和实际回答"}, {"uncertainty", "固定页面演示记录，不代表真实教学验收。"},
                            // 【逐行批注】 执行当前声明或表达式：{"diagnosis", {{"summary", incorrect ? "分子分母分别相加，忽略每份大小不同。" : "本次计算正确，需要结合帮助程度判断独立理解…。
                            {"diagnosis", {{"summary", incorrect ? "分子分母分别相加，忽略每份大小不同。" : "本次计算正确，需要结合帮助程度判断独立理解。"},
                                // 【逐行批注】 执行当前声明或表达式：{"evidence", {event.at("text")}}, {"teachingDirection", "用等分单位解释通分的理由。"}, {"needsCla…。
                                {"evidence", {event.at("text")}}, {"teachingDirection", "用等分单位解释通分的理由。"}, {"needsClarification", false}}}}}};
                    // 【逐行批注】 结束当前代码块。
                    }
                    // 【逐行批注】 计算右侧结果并写入 decision["actions"].push_back(action); decis…，更新当前运行状态。
                    decision["actions"].push_back(action); decision["state"] = "continue"; decision["message"] = "";
                // 【逐行批注】 执行当前声明或表达式：} else if (event.value("requestType", "") == "retry_question") {。
                } else if (event.value("requestType", "") == "retry_question") {
                    // 【逐行批注】 计算右侧结果并写入 decision["message"]，更新当前运行状态。
                    decision["message"] = "下面是关联原问题的新题。请独立写出结果，并解释为什么先通分。";
                    // 【逐行批注】 执行当前声明或表达式：decision["actions"].push_back({{"id", "retry-question"}, {"tool", "append_section"},…。
                    decision["actions"].push_back({{"id", "retry-question"}, {"tool", "append_section"}, {"args", {{"lessonId", lessonId},
                        // 【逐行批注】 执行当前声明或表达式：{"section", {{"kind", "question"}, {"title", "关联原题 · 独立复练"}, {"questionKind", "pract…。
                        {"section", {{"kind", "question"}, {"title", "关联原题 · 独立复练"}, {"questionKind", "practice"},
                            // 【逐行批注】 执行当前声明或表达式：{"rePracticeOf", actual.at("questionId")}, {"question", test.at("questions").at(1)}}…。
                            {"rePracticeOf", actual.at("questionId")}, {"question", test.at("questions").at(1)}}}}}});
                // 【逐行批注】 执行当前声明或表达式：} else if (event.value("requestType", "") == "worked_example") {。
                } else if (event.value("requestType", "") == "worked_example") {
                    // 【逐行批注】 计算右侧结果并写入 decision["teaching"]["helpType"]，更新当前运行状态。
                    decision["teaching"]["helpType"] = "worked_example";
                    // 【逐行批注】 计算右侧结果并写入 decision["message"]，更新当前运行状态。
                    decision["message"] = "示范：先统一每份大小。1/2=3/6，1/3=2/6，相加得到5/6。接着换一道新题，检验能否独立完成。";
                // 【逐行批注】 执行当前声明或表达式：} else {。
                } else {
                    // 【逐行批注】 计算右侧结果并写入 decision["teaching"]["helpType"]，更新当前运行状态。
                    decision["teaching"]["helpType"] = event.value("requestType", "") == "hint" ? "hint" : "explanation";
                    // 【逐行批注】 计算右侧结果并写入 decision["message"]，更新当前运行状态。
                    decision["message"] = "先想想：两种分数的每份大小相同吗？尝试把它们换成同样大小的份。";
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 auto content，更新当前运行状态。
                const auto content = decision.dump();
                // 【逐行批注】 按循环规则重复处理：for (std::size_t offset=0;offset<content.size();offset+=13) sink(content.s…。
                for (std::size_t offset=0;offset<content.size();offset+=13) sink(content.substr(offset,13));
                // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
                gangyi::AIResult result; result.content = content; result.model = "页面固定演示"; result.finishReason = "stop";
                // 【逐行批注】 结束当前函数并返回结果：return result;。
                return result;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 auto start，更新当前运行状态。
            const auto start = std::chrono::steady_clock::now();
            // 【逐行批注】 判断条件 (const auto limit=std::getenv("GANGYI_EVAL_MAX_CALLS"))，成立时执行对应分支。
            if (const auto limit=std::getenv("GANGYI_EVAL_MAX_CALLS"))
                // 【逐行批注】 判断条件 (usage.size()>=static_cast<std::size_t>(std::stoul(limit)))，成立时执行对应分支。
                if (usage.size()>=static_cast<std::size_t>(std::stoul(limit)))
                    // 【逐行批注】 检测到错误情形时抛出异常：throw gangyi::AIClientError("eval_call_limit","本次协议复核的请求次数已用完，未继续调用模型…。
                    throw gangyi::AIClientError("eval_call_limit","本次协议复核的请求次数已用完，未继续调用模型");
            // 【逐行批注】 计算右侧结果并写入 requestOptions，更新当前运行状态。
            auto requestOptions=options;
            // 【逐行批注】 判断条件 (const auto limit=std::getenv("GANGYI_EVAL_MAX_OUTPUT_TOKENS"))，成立时执行对应分支。
            if (const auto limit=std::getenv("GANGYI_EVAL_MAX_OUTPUT_TOKENS"))
                // 【逐行批注】 计算右侧结果并写入 requestOptions.maxTokens，更新当前运行状态。
                requestOptions.maxTokens=std::min(options.maxTokens,std::stoi(limit));
            // 原生发送点独立校验本轮授权，不能只靠 Python 或环境开关放行。
            // 【逐行批注】 计算右侧结果并写入 std::string endpoint，更新当前运行状态。
            const std::string endpoint = std::getenv("AI_BASE_URL") ? std::getenv("AI_BASE_URL") : "";
            // 【逐行批注】 调用 validateEvaluationAuthorization 完成本行所需操作：validateEvaluationAuthorization(endpoint);。
            validateEvaluationAuthorization(endpoint);
            // 【逐行批注】 判断条件 (const auto mode = std::getenv("GANGYI_EVAL_MODE"); mode && std:…，成立时执行对应分支。
            if (const auto mode = std::getenv("GANGYI_EVAL_MODE"); mode && std::string(mode) == "live" &&
                // 【逐行批注】 调用 getenv 完成本行所需操作：(std::getenv("GANGYI_EVAL_MAX_OUTPUT_TOKENS") || options.maxTokens !=…。
                (std::getenv("GANGYI_EVAL_MAX_OUTPUT_TOKENS") || options.maxTokens != requestOptions.maxTokens))
                // 【逐行批注】 检测到错误情形时抛出异常：throw gangyi::AIClientError("eval_authorization", "真实对照禁止缩减产品输出上限");。
                throw gangyi::AIClientError("eval_authorization", "真实对照禁止缩减产品输出上限");
            // 【逐行批注】 执行当前声明或表达式：gangyi::AIClient client; gangyi::AIResult response;。
            gangyi::AIClient client; gangyi::AIResult response;
            // 【逐行批注】 声明对象 budget，并通过构造函数初始化其状态。
            EvaluationBudget budget(db,requestOptions);
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try { response = client.chatStream(requestOptions, sink); }
            // 【逐行批注】 捕获并处理异常：catch (...) { budget.settle(db); usage.push_back({{"model", options.m…。
            catch (...) { budget.settle(db); usage.push_back({{"model", options.model}, {"tokenUsage", nullptr}, {"failed", true},
                // 【逐行批注】 调用 now 完成本行所需操作：{"durationMs", std::chrono::duration_cast<std::chrono::milliseconds>(…。
                {"durationMs", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count()}}); throw; }
            // 【逐行批注】 执行当前声明或表达式：budget.settle(db);。
            budget.settle(db);
            // 【逐行批注】 执行当前声明或表达式：usage.push_back({{"model", response.model}, {"tokenUsage", response.tokenUsage},。
            usage.push_back({{"model", response.model}, {"tokenUsage", response.tokenUsage},
                // 【逐行批注】 调用 now 完成本行所需操作：{"durationMs", std::chrono::duration_cast<std::chrono::milliseconds>(…。
                {"durationMs", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count()}});
            // 【逐行批注】 结束当前函数并返回结果：return response;。
            return response;
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(agent);。
        gangyi::registerAgentLessonTools(agent);
        // 【逐行批注】 计算右侧结果并写入 Json turns，更新当前运行状态。
        Json turns = Json::array(); int index = 0;
        // 每轮留下独立检查点；验收执行器退出也不会丢失已完成对话及实际HTTP用量。
        // 【逐行批注】 计算右侧结果并写入 auto checkpoint，更新当前运行状态。
        const auto checkpoint = [&] {
            // 【逐行批注】 计算右侧结果并写入 Json interactions，更新当前运行状态。
            Json interactions = Json::array(), httpRequests = Json::array();
            // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.listInteractions()) interactions.push_back({{"id…。
            for (const auto& row : db.listInteractions()) interactions.push_back({{"id", row.id}, {"kind", row.kind}, {"payload", Json::parse(row.payload)}});
            // 【逐行批注】 按循环规则重复处理：for (const auto& row : db.myAIRecords("usage")) {。
            for (const auto& row : db.myAIRecords("usage")) {
                // 【逐行批注】 计算右侧结果并写入 request，更新当前运行状态。
                auto request = Json::parse(row.payload); request["id"] = row.id; httpRequests.push_back(std::move(request));
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
            const Json result = {{"caseId", test.at("id")}, {"turns", turns}, {"interactions", interactions}, {"usage", usage},
                // 【逐行批注】 执行当前声明或表达式：{"httpRequests", httpRequests}, {"httpRequestCount", httpRequests.size()}, {"promptA…。
                {"httpRequests", httpRequests}, {"httpRequestCount", httpRequests.size()}, {"promptAudit", promptAudit},
                // 【逐行批注】 调用 now 完成本行所需操作：{"externalElapsedMs", std::chrono::duration_cast<std::chrono::millise…。
                {"externalElapsedMs", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-runStarted).count()},
                // 【逐行批注】 执行当前声明或表达式：{"externalTimingBoundary", "单调时钟：初始化隔离材料后至检查点前，含process与动作保存，不含人类等待；并非产品内任务指标"},。
                {"externalTimingBoundary", "单调时钟：初始化隔离材料后至检查点前，含process与动作保存，不含人类等待；并非产品内任务指标"},
                // 【逐行批注】 调用 parse 完成本行所需操作：{"resumedExisting", resumeExisting}, {"lesson", Json::parse(db.getCla…。
                {"resumedExisting", resumeExisting}, {"lesson", Json::parse(db.getClassroomActivity(lessonId)->payload)},
                // 【逐行批注】 调用 agentProfileEvidence 完成本行所需操作：{"profileEvidence", gangyi::agentProfileEvidence(db, access)}};。
                {"profileEvidence", gangyi::agentProfileEvidence(db, access)}};
            // 【逐行批注】 计算右侧结果并写入 auto outputPath，更新当前运行状态。
            const auto outputPath = std::filesystem::u8path(argv[3]); auto temporaryPath = outputPath; temporaryPath += ".tmp";
            // 【逐行批注】 调用 output 完成本行所需操作：{ std::ofstream output(temporaryPath); output << result.dump(2); }。
            { std::ofstream output(temporaryPath); output << result.dump(2); }
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
            // 【逐行批注】 判断条件 (!MoveFileExW(temporaryPath.c_str(), outputPath.c_str(), MOVEFIL…，成立时执行对应分支。
            if (!MoveFileExW(temporaryPath.c_str(), outputPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("验收检查点保存失败");。
                throw std::runtime_error("验收检查点保存失败");
// 【逐行批注】 处理条件编译指令：#else。
#else
            // 【逐行批注】 调用 rename 完成本行所需操作：std::filesystem::rename(temporaryPath, outputPath);。
            std::filesystem::rename(temporaryPath, outputPath);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 按循环规则重复处理：for (auto event : test.at("turns")) {。
        for (auto event : test.at("turns")) {
            // 【逐行批注】 计算右侧结果并写入 saved，更新当前运行状态。
            auto saved = Json::parse(db.getClassroomActivity(lessonId)->payload);
            // 【逐行批注】 判断条件 (event.value("sectionId", "") == "$latest") {，成立时执行对应分支。
            if (event.value("sectionId", "") == "$latest") {
                // 【逐行批注】 按循环规则重复处理：for (auto it = saved["sections"].rbegin(); it != saved["sections"].rend();…。
                for (auto it = saved["sections"].rbegin(); it != saved["sections"].rend(); ++it)
                    // 【逐行批注】 判断条件 (it->value("kind", "") == "question") { event["sectionId"] = it-…，成立时执行对应分支。
                    if (it->value("kind", "") == "question") { event["sectionId"] = it->at("id"); break; }
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 event["courseId"]，更新当前运行状态。
            event["courseId"] = courseId; event["lessonId"] = lessonId; event["view"] = "practice";
            // 【逐行批注】 计算右侧结果并写入 event["requestId"]，更新当前运行状态。
            event["requestId"] = "eval-turn-" + std::to_string(index++);
            // 【逐行批注】 判断条件 (event.contains("sectionId")) event["sectionVersion"] = 1;，成立时执行对应分支。
            if (event.contains("sectionId")) event["sectionVersion"] = 1;
            // 【逐行批注】 执行当前声明或表达式：Json task;。
            Json task;
            // 【逐行批注】 判断条件 (resumeExisting) for (const auto& savedTask : db.listClassroomAc…，成立时执行对应分支。
            if (resumeExisting) for (const auto& savedTask : db.listClassroomActivities(courseId)) {
                // 【逐行批注】 判断条件 (savedTask.kind != "agent-task") continue;，成立时执行对应分支。
                if (savedTask.kind != "agent-task") continue;
                // 【逐行批注】 计算右侧结果并写入 auto rawTask，更新当前运行状态。
                const auto rawTask = Json::parse(savedTask.payload);
                // 【逐行批注】 判断条件 (rawTask.value("event", Json::object()).value("requestId", "") =…，成立时执行对应分支。
                if (rawTask.value("event", Json::object()).value("requestId", "") == event.at("requestId")) {
                    // 【逐行批注】 判断条件 (!task.is_null()) throw std::runtime_error("同一验收轮存在重复任务，不能自动恢复");，成立时执行对应分支。
                    if (!task.is_null()) throw std::runtime_error("同一验收轮存在重复任务，不能自动恢复");
                    // 【逐行批注】 计算右侧结果并写入 task，更新当前运行状态。
                    task = rawTask;
                    // 【逐行批注】 计算右侧结果并写入 event，更新当前运行状态。
                    event = rawTask.at("event");
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (task.is_null()) task = gangyi::agentSubmit(db, access, event);，成立时执行对应分支。
            if (task.is_null()) task = gangyi::agentSubmit(db, access, event);
            // 【逐行批注】 计算右侧结果并写入 bool alreadyShown，更新当前运行状态。
            const bool alreadyShown = task.value("status", "") == "waiting_student" || task.value("status", "") == "completed" || task.value("status", "") == "ready";
            // 恢复失败/暂停轮必须走原生控制；process不会自行重启终态任务。
            // 【逐行批注】 判断条件 (resumeExisting && (task.value("status", "") == "failed" || task…，成立时执行对应分支。
            if (resumeExisting && (task.value("status", "") == "failed" || task.value("status", "") == "paused"))
                // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", task.at("status") == "p…。
                gangyi::agentControl(db, access, {{"command", task.at("status") == "paused" ? "resume" : "retry"}, {"taskId", task.at("id")}});
            // 【逐行批注】 计算右侧结果并写入 auto processStarted，更新当前运行状态。
            const auto processStarted = std::chrono::steady_clock::now();
            // 【逐行批注】 判断条件 (!alreadyShown) agent.process(db, task.at("id"));，成立时执行对应分支。
            if (!alreadyShown) agent.process(db, task.at("id"));
            // 主控会将回调异常保存为任务失败；另用专用退出码阻止Python把旧预算当作新结算。
            // 【逐行批注】 判断条件 (EvaluationBudget::storageFailed()) throw std::runtime_error("预算…，成立时执行对应分支。
            if (EvaluationBudget::storageFailed()) throw std::runtime_error("预算保存失败，不能继续验收");
            // 【逐行批注】 判断条件 (evaluationSearchBlocked) throw std::runtime_error("教学对照出现未冻结的新搜…，成立时执行对应分支。
            if (evaluationSearchBlocked) throw std::runtime_error("教学对照出现未冻结的新搜索，已在发送前阻止，不能作为完整验收");
            // 【逐行批注】 计算右侧结果并写入 auto externalProcessMs，更新当前运行状态。
            const auto externalProcessMs = alreadyShown ? Json() : Json(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-processStarted).count());
            // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
            const auto row = db.getClassroomActivity(task.at("id")); const auto raw = Json::parse(row->payload);
            // 【逐行批注】 判断条件 (!alreadyShown) {，成立时执行对应分支。
            if (!alreadyShown) {
            // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"taskId", task.at("id")}, {"seq…。
            gangyi::agentExposeTask(db, access, {{"taskId", task.at("id")}, {"seq", raw.at("seq")}, {"complete", true}});
            // 【逐行批注】 计算右侧结果并写入 Json exposed，更新当前运行状态。
            Json exposed = Json::array(); saved = Json::parse(db.getClassroomActivity(lessonId)->payload);
            // 【逐行批注】 按循环规则重复处理：for (const auto& section : saved.at("sections")) exposed.push_back({{"id",…。
            for (const auto& section : saved.at("sections")) exposed.push_back({{"id", section.at("id")}, {"version", section.at("version")}});
            // 【逐行批注】 计算右侧结果并写入 Json evaluations，更新当前运行状态。
            Json evaluations = Json::array();
            // 【逐行批注】 计算右侧结果并写入 auto shownLesson，更新当前运行状态。
            const auto shownLesson = gangyi::agentLessonView(db, access, lessonId);
            // 【逐行批注】 按循环规则重复处理：for (const auto& evaluation : shownLesson.at("evaluations"))。
            for (const auto& evaluation : shownLesson.at("evaluations"))
                // 【逐行批注】 判断条件 (!evaluation.value("feedback", "").empty()) evaluations.push_bac…，成立时执行对应分支。
                if (!evaluation.value("feedback", "").empty()) evaluations.push_back({{"id", evaluation.at("id")}, {"feedback", evaluation.at("feedback")}, {"complete", true}});
            // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db, access, {{"lessonId", lessonId}, {"sectio…。
            gangyi::agentExposeTask(db, access, {{"lessonId", lessonId}, {"sections", exposed}, {"evaluations", evaluations}, {"complete", true}});
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 执行当前声明或表达式：turns.push_back({{"event", event}, {"task", raw}, {"externalProcessMs", externalProc…。
            turns.push_back({{"event", event}, {"task", raw}, {"externalProcessMs", externalProcessMs},
                // 【逐行批注】 调用 agentContext 完成本行所需操作：{"currentInputs", gangyi::agentContext(db, access, event).at("current…。
                {"currentInputs", gangyi::agentContext(db, access, event).at("currentInputs")}});
            // 【逐行批注】 调用 checkpoint 完成本行所需操作：checkpoint();。
            checkpoint();
            // 【逐行批注】 判断条件 (raw.at("status") == "failed" || raw.at("status") == "superseded…，成立时执行对应分支。
            if (raw.at("status") == "failed" || raw.at("status") == "superseded") break;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 checkpoint 完成本行所需操作：checkpoint();。
        checkpoint();
        // 【逐行批注】 执行当前声明或表达式：std::cout << test.at("id") << "：完成 " << turns.size() << " 轮，实际调用 " << usage.size() <…。
        std::cout << test.at("id") << "：完成 " << turns.size() << " 轮，实际调用 " << usage.size() << " 次\n";
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 调用 storageFailed 完成本行所需操作：} catch (const std::exception& error) { std::cerr << "验收主控错误：" << err…。
    } catch (const std::exception& error) { std::cerr << "验收主控错误：" << error.what() << '\n'; return EvaluationBudget::storageFailed() ? 73 : 1; }
// 【逐行批注】 结束当前代码块。
}

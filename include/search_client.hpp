// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 "resource_types.hpp"，为后续实现提供类型或接口声明。
#include "resource_types.hpp"

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>
// 【逐行批注】 引入头文件 <functional>，为后续实现提供类型或接口声明。
#include <functional>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 与检索缓存共用的SHA256，避免不同平台产生不同的资料身份。
// 【逐行批注】 声明对象 resourceDigest，并通过构造函数初始化其状态。
std::string resourceDigest(const std::string& input);
// 【逐行批注】 声明数据结构或类型 ResourceQuery 及其成员接口。
struct ResourceQuery {
    // 【逐行批注】 执行当前声明或表达式：std::string query;。
    std::string query;
    // 【逐行批注】 计算右侧结果并写入 std::string freshness，更新当前运行状态。
    std::string freshness = "noLimit";
    // 【逐行批注】 计算右侧结果并写入 int count，更新当前运行状态。
    int count = 10;
    // 【逐行批注】 计算右侧结果并写入 bool refresh，更新当前运行状态。
    bool refresh = false;
    // 【逐行批注】 调用 bool 完成本行所需操作：std::function<bool()> cancelled;。
    std::function<bool()> cancelled;
    // 【逐行批注】 执行当前声明或表达式：std::string scopeId;。
    std::string scopeId;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 ResourceQueryResult 及其成员接口。
struct ResourceQueryResult {
    // 【逐行批注】 执行当前声明或表达式：std::vector<SearchResource> resources;。
    std::vector<SearchResource> resources;
    // 【逐行批注】 执行当前声明或表达式：std::string status;。
    std::string status;
    // 【逐行批注】 计算右侧结果并写入 int attempts，更新当前运行状态。
    int attempts = 0;
    // 【逐行批注】 计算右侧结果并写入 bool cached，更新当前运行状态。
    bool cached = false;
    // 【逐行批注】 计算右侧结果并写入 bool shared，更新当前运行状态。
    bool shared = false;
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json providers，更新当前运行状态。
    nlohmann::json providers = nlohmann::json::array();
// 【逐行批注】 执行当前声明或表达式：};。
};

// 所有联网场景统一并行搜索 DeepSeek 与博查，分别缓存、合并去重并保留状态。
// 【逐行批注】 声明数据结构或类型 SearchClient 及其成员接口。
class SearchClient {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 调用 SearchClient 完成本行所需操作：SearchClient();。
    SearchClient();
    // 【逐行批注】 计算右侧结果并写入 bool testBochaKey(const std::string& key, st…，更新当前运行状态。
    static bool testBochaKey(const std::string& key, std::string* diagnostic = nullptr);

    // 搜索 goal 相关资源，失败返回空 vector（不抛异常），最多 limit 条。
    // 【逐行批注】 声明对象 search，并通过构造函数初始化其状态。
    std::vector<SearchResource> search(const std::string& goal, size_t limit = 20,
        // 【逐行批注】 调用 bool 完成本行所需操作：const std::function<bool()>& cancelled = {}) const;。
        const std::function<bool()>& cancelled = {}) const;
    // 主控指定查询角度和时效；一家失败仍返回另一家的有效资料。
    // 【逐行批注】 声明对象 query，并通过构造函数初始化其状态。
    ResourceQueryResult query(const ResourceQuery& request) const;

    // 兼容旧调用者：返回双引擎标识，以及是否出现单家失败或缺配置。
    // 【逐行批注】 定义函数 lastFallbackUsed，执行该函数声明的职责。
    bool lastFallbackUsed() const { return fallbackUsed_; }
    // 【逐行批注】 定义函数 lastProvider，执行该函数声明的职责。
    std::string lastProvider() const { return lastProvider_; }

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：std::string cacheDir_; // data/resource-search-cache。
    std::string cacheDir_;          // data/resource-search-cache
    // 【逐行批注】 执行当前声明或表达式：mutable std::string lastProvider_;。
    mutable std::string lastProvider_;
    // 【逐行批注】 计算右侧结果并写入 mutable bool fallbackUsed_，更新当前运行状态。
    mutable bool fallbackUsed_ = false;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

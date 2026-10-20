#pragma once

#include "resource_types.hpp"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <functional>

namespace gangyi {
// 与检索缓存共用的SHA256，避免不同平台产生不同的资料身份。
std::string resourceDigest(const std::string& input);
struct ResourceQuery {
    std::string query;
    std::string freshness = "noLimit";
    int count = 10;
    bool refresh = false;
    std::function<bool()> cancelled;
    std::string scopeId;
};
struct ResourceQueryResult {
    std::vector<SearchResource> resources;
    std::string status;
    int attempts = 0;
    bool cached = false;
    bool shared = false;
    nlohmann::json providers = nlohmann::json::array();
};

// 所有联网场景统一并行搜索 DeepSeek 与博查，分别缓存、合并去重并保留状态。
class SearchClient {
public:
    SearchClient();
    static bool testBochaKey(const std::string& key, std::string* diagnostic = nullptr);

    // 搜索 goal 相关资源，失败返回空 vector（不抛异常），最多 limit 条。
    std::vector<SearchResource> search(const std::string& goal, size_t limit = 20,
        const std::function<bool()>& cancelled = {}) const;
    // 主控指定查询角度和时效；一家失败仍返回另一家的有效资料。
    ResourceQueryResult query(const ResourceQuery& request) const;

    // 兼容旧调用者：返回双引擎标识，以及是否出现单家失败或缺配置。
    bool lastFallbackUsed() const { return fallbackUsed_; }
    std::string lastProvider() const { return lastProvider_; }

private:
    std::string cacheDir_;          // data/resource-search-cache
    mutable std::string lastProvider_;
    mutable bool fallbackUsed_ = false;
};

}  // namespace gangyi

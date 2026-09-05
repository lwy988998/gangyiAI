#pragma once

#include "resource_types.hpp"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace gangyi {

// 联网资源搜索：领域检测 → 查询构造 → provider（主 tavily、备 bocha）→ 归一化 → 去重排序 → 7 天缓存。
// 对齐校园版 lib/search/ 的行为（searchResources.ts / searchProvider.ts / normalizeResource.ts / resourceCache.ts）。
class SearchClient {
public:
    SearchClient();

    // 搜索 goal 相关资源，失败返回空 vector（不抛异常），最多 limit 条。
    std::vector<SearchResource> search(const std::string& goal, size_t limit = 20) const;

    // 最近一次搜索实际使用的 provider 与是否用了 fallback。
    bool lastFallbackUsed() const { return fallbackUsed_; }
    std::string lastProvider() const { return lastProvider_; }

private:
    std::string provider_;          // SEARCH_PROVIDER，默认 tavily
    std::string fallbackProvider_;  // SEARCH_FALLBACK_PROVIDER，默认 bocha
    std::string cacheDir_;          // data/resource-search-cache
    mutable std::string lastProvider_;
    mutable bool fallbackUsed_ = false;
};

}  // namespace gangyi

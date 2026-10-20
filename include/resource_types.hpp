#pragma once

#include <string>
#include <vector>

namespace gangyi {

// 联网搜索结果的统一结构，供课程规划和阶段展开共用。
struct SearchResource {
    std::string title;
    std::string url;
    std::string source;
    std::string description;
    std::string type;
    std::string difficulty;
    std::string language;
    bool free = true;
    double score = 70.0;
    std::string reason;
    // 这些日期来自服务商或原网页；空字符串表示未知。
    std::string publishedAt = {};
    std::string updatedAt = {};
    // 搜索引擎来源与原网站来源分开保存，重复网页可由两家共同命中。
    std::vector<std::string> searchProviders = {};
};

}  // namespace gangyi

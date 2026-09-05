#pragma once

#include <string>

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
};

}  // namespace gangyi

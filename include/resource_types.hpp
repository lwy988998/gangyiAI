// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 联网搜索结果的统一结构，供课程规划和阶段展开共用。
// 【逐行批注】 声明数据结构或类型 SearchResource 及其成员接口。
struct SearchResource {
    // 【逐行批注】 执行当前声明或表达式：std::string title;。
    std::string title;
    // 【逐行批注】 执行当前声明或表达式：std::string url;。
    std::string url;
    // 【逐行批注】 执行当前声明或表达式：std::string source;。
    std::string source;
    // 【逐行批注】 执行当前声明或表达式：std::string description;。
    std::string description;
    // 【逐行批注】 执行当前声明或表达式：std::string type;。
    std::string type;
    // 【逐行批注】 执行当前声明或表达式：std::string difficulty;。
    std::string difficulty;
    // 【逐行批注】 执行当前声明或表达式：std::string language;。
    std::string language;
    // 【逐行批注】 计算右侧结果并写入 bool free，更新当前运行状态。
    bool free = true;
    // 【逐行批注】 计算右侧结果并写入 double score，更新当前运行状态。
    double score = 70.0;
    // 【逐行批注】 执行当前声明或表达式：std::string reason;。
    std::string reason;
    // 这些日期来自服务商或原网页；空字符串表示未知。
    // 【逐行批注】 计算右侧结果并写入 std::string publishedAt，更新当前运行状态。
    std::string publishedAt = {};
    // 【逐行批注】 计算右侧结果并写入 std::string updatedAt，更新当前运行状态。
    std::string updatedAt = {};
    // 搜索引擎来源与原网站来源分开保存，重复网页可由两家共同命中。
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> searchProviders = {};。
    std::vector<std::string> searchProviders = {};
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

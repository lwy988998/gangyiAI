// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <optional>，为后续实现提供类型或接口声明。
#include <optional>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 声明数据结构或类型 ProgressSaveResult 及其成员接口。
struct ProgressSaveResult {
    // 【逐行批注】 计算右侧结果并写入 bool ok，更新当前运行状态。
    bool ok = false;
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json item，更新当前运行状态。
    nlohmann::json item = nlohmann::json::object();
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明对象 normalizeCardStatus，并通过构造函数初始化其状态。
std::string normalizeCardStatus(const std::string& status);
// 【逐行批注】 声明对象 normalizeStepStatus，并通过构造函数初始化其状态。
std::string normalizeStepStatus(const std::string& status);
// 【逐行批注】 声明对象 saveTaskProgress，并通过构造函数初始化其状态。
ProgressSaveResult saveTaskProgress(Database& db, const nlohmann::json& body);
// 【逐行批注】 声明对象 recomputeCourseProgress，并通过构造函数初始化其状态。
std::optional<nlohmann::json> recomputeCourseProgress(Database& db, const std::string& courseId,
                                                      // 【逐行批注】 执行当前声明或表达式：const std::string& anonymousId,。
                                                      const std::string& anonymousId,
                                                      // 【逐行批注】 执行当前声明或表达式：const std::string& goal);。
                                                      const std::string& goal);
// 【逐行批注】 声明对象 resetCourseProgress，并通过构造函数初始化其状态。
std::optional<nlohmann::json> resetCourseProgress(Database& db, const std::string& courseId,
                                                  // 【逐行批注】 执行当前声明或表达式：const std::string& anonymousId,。
                                                  const std::string& anonymousId,
                                                  // 【逐行批注】 执行当前声明或表达式：const std::string& goal);。
                                                  const std::string& goal);
// 【逐行批注】 声明对象 updateLastVisited，并通过构造函数初始化其状态。
bool updateLastVisited(Database& db, const std::string& courseId, const std::string& anonymousId,
                       // 【逐行批注】 执行当前声明或表达式：const std::string& goal, const std::string& mode,。
                       const std::string& goal, const std::string& mode,
                       // 【逐行批注】 执行当前声明或表达式：const nlohmann::json& body);。
                       const nlohmann::json& body);

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明数据结构或类型 Database 及其成员接口。
class Database;
// 【逐行批注】 声明数据结构或类型 AIClient 及其成员接口。
class AIClient;

// 【逐行批注】 声明对象 profileView，并通过构造函数初始化其状态。
nlohmann::json profileView(Database& db);
// 【逐行批注】 声明对象 recentCourses，并通过构造函数初始化其状态。
nlohmann::json recentCourses(Database& db);
// 【逐行批注】 声明对象 profileEvidenceSummary，并通过构造函数初始化其状态。
nlohmann::json profileEvidenceSummary(Database& db);
// 【逐行批注】 声明对象 profileContext，并通过构造函数初始化其状态。
std::string profileContext(Database& db);
// 【逐行批注】 声明对象 refreshProfile，并通过构造函数初始化其状态。
bool refreshProfile(Database& db, AIClient& ai, std::string& error);
// 【逐行批注】 声明对象 refreshTopicMastery，并通过构造函数初始化其状态。
bool refreshTopicMastery(Database& db, AIClient& ai, std::string& error);
// 【逐行批注】 声明对象 radarPreferences，并通过构造函数初始化其状态。
nlohmann::json radarPreferences(Database& db);
// 【逐行批注】 声明对象 saveRadarPreferences，并通过构造函数初始化其状态。
nlohmann::json saveRadarPreferences(Database& db, const nlohmann::json& value);
// 【逐行批注】 声明对象 abilityProfileView，并通过构造函数初始化其状态。
nlohmann::json abilityProfileView(Database& db);
// 【逐行批注】 声明对象 refreshAbilityProfile，并通过构造函数初始化其状态。
bool refreshAbilityProfile(Database& db, AIClient& ai);
// 【逐行批注】 声明对象 nextLearning，并通过构造函数初始化其状态。
nlohmann::json nextLearning(Database& db, const std::string& courseId);
// 【逐行批注】 结束当前代码块。
}

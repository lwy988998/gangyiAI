// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"
// 【逐行批注】 引入头文件 "library_io.hpp"，为后续实现提供类型或接口声明。
#include "library_io.hpp"
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明数据结构或类型 LearningAgent 及其成员接口。
class LearningAgent;
// 【逐行批注】 声明对象 libraryRead，并通过构造函数初始化其状态。
nlohmann::json libraryRead(Database& db, const nlohmann::json& query = nlohmann::json::object());
// 【逐行批注】 声明对象 libraryWrite，并通过构造函数初始化其状态。
nlohmann::json libraryWrite(Database& db, const nlohmann::json& body);
nlohmann::json libraryCurriculumRead(Database& db, const std::string& courseId);
nlohmann::json libraryCurriculumWrite(Database& db, const nlohmann::json& body);
nlohmann::json libraryVerifiedCurriculumChapter(Database& db, const std::string& courseId);
// 【逐行批注】 声明对象 libraryContext，并通过构造函数初始化其状态。
nlohmann::json libraryContext(Database& db, const std::string& courseId = {}, const std::string& question = {});
nlohmann::json libraryTeachingMaterial(Database& db, const std::string& courseId, const nlohmann::json& ref);
std::string libraryTeachingConversationId(const std::string& scopeId, const std::string& courseId, const nlohmann::json& ref);
// 【逐行批注】 声明对象 libraryUsage，并通过构造函数初始化其状态。
nlohmann::json libraryUsage(Database& db);
// 【逐行批注】 声明对象 libraryValidateCitations，并通过构造函数初始化其状态。
nlohmann::json libraryValidateCitations(Database& db, const nlohmann::json& citations, bool teaching = true, const std::string& courseId = {});
// 【逐行批注】 声明对象 registerLibraryTools，并通过构造函数初始化其状态。
void registerLibraryTools(LearningAgent& agent);
// 【逐行批注】 结束当前代码块。
}

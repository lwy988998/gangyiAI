// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 计算右侧结果并写入 using MyAIJson，更新当前运行状态。
using MyAIJson = nlohmann::json;
// 【逐行批注】 声明对象 myAIRead，并通过构造函数初始化其状态。
MyAIJson myAIRead(Database& db, const std::string& section, const MyAIJson& query = MyAIJson::object());
// 【逐行批注】 声明对象 myAIWrite，并通过构造函数初始化其状态。
MyAIJson myAIWrite(Database& db, const std::string& section, const MyAIJson& body, bool transactional = true);
// 【逐行批注】 声明对象 myAIChatMessage，并通过构造函数初始化其状态。
MyAIJson myAIChatMessage(Database& db, const std::string& chatId, const MyAIJson& body);
// 【逐行批注】 声明对象 myAIChatTask，并通过构造函数初始化其状态。
void myAIChatTask(Database& db, const std::string& chatId, const std::string& requestId, const MyAIJson& task);
// 【逐行批注】 声明对象 myAIChatReply，并通过构造函数初始化其状态。
void myAIChatReply(Database& db, const MyAIJson& task, const std::string& text);
// 【逐行批注】 声明对象 myAIConfirm，并通过构造函数初始化其状态。
MyAIJson myAIConfirm(Database& db, const std::string& id, const MyAIJson& body);
// 【逐行批注】 声明对象 myAIProposal，并通过构造函数初始化其状态。
MyAIJson myAIProposal(Database& db, const MyAIJson& args, const std::string& actionId);
// 【逐行批注】 声明对象 myAISnapshot，并通过构造函数初始化其状态。
void myAISnapshot(Database& db);
// 【逐行批注】 声明对象 renderMyAIPage，并通过构造函数初始化其状态。
std::string renderMyAIPage(const std::string& page);
// 【逐行批注】 声明数据结构或类型 LearningAgent 及其成员接口。
class LearningAgent;
// 【逐行批注】 声明对象 registerMyAITools，并通过构造函数初始化其状态。
void registerMyAITools(LearningAgent& agent);
// 【逐行批注】 声明对象 myAIResumeControls，并通过构造函数初始化其状态。
void myAIResumeControls(Database& db);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 独立聊天只按课时保存会话，不读取课时正文、学习记录、画像或长期记忆。
// 【逐行批注】 声明对象 independentChatView，并通过构造函数初始化其状态。
AgentJson independentChatView(Database& db, const AgentAccess& access, const std::string& lessonId);
// 【逐行批注】 声明对象 independentChatSettings，并通过构造函数初始化其状态。
AgentJson independentChatSettings(Database& db, const AgentJson& body);
// 【逐行批注】 声明对象 independentChatSend，并通过构造函数初始化其状态。
AgentJson independentChatSend(Database& db, const AgentAccess& access, const AgentJson& body);
// 【逐行批注】 声明对象 independentChatClear，并通过构造函数初始化其状态。
AgentJson independentChatClear(Database& db, const AgentAccess& access, const AgentJson& body);

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
} // namespace gangyi

#pragma once
#include "db.hpp"
#include <nlohmann/json.hpp>

namespace gangyi {
using MyAIJson = nlohmann::json;
MyAIJson myAIRead(Database& db, const std::string& section, const MyAIJson& query = MyAIJson::object());
MyAIJson myAIWrite(Database& db, const std::string& section, const MyAIJson& body, bool transactional = true);
MyAIJson myAIChatMessage(Database& db, const std::string& chatId, const MyAIJson& body);
void myAIChatTask(Database& db, const std::string& chatId, const std::string& requestId, const MyAIJson& task);
void myAIChatReply(Database& db, const MyAIJson& task, const std::string& text);
MyAIJson myAIConfirm(Database& db, const std::string& id, const MyAIJson& body);
MyAIJson myAIProposal(Database& db, const MyAIJson& args, const std::string& actionId);
void myAISnapshot(Database& db);
std::string renderMyAIPage(const std::string& page);
class LearningAgent;
void registerMyAITools(LearningAgent& agent);
void myAIResumeControls(Database& db);
}

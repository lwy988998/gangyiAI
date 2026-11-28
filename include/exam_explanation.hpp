#pragma once

#include "ai_client.hpp"
#include "db.hpp"
#include "resource_types.hpp"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace gangyi {

using ExamJson = nlohmann::json;

// 试卷会话服务只保存本机档案；原始页与可修正的识别层分开保存。
ExamJson examExplanationList(Database& db);
ExamJson examExplanationView(Database& db, const std::string& id);
ExamJson examTeachingQuestion(Database& db, const ExamJson& reference);
std::string examTeachingConversationId(const std::string& scopeId, const std::string& courseId, const ExamJson& reference);
ExamJson examExplanationSources(Database& db, const std::string& id);
ExamJson examExplanationCreate(Database& db, const ExamJson& body);
ExamJson examExplanationUpdate(Database& db, const std::string& id, const ExamJson& body);
ExamJson examExplanationAppendUserMessage(Database& db, const std::string& id, const ExamJson& body);
ExamJson examExplanationReserveTask(Database& db, const std::string& id, const std::string& requestId,
                                    const std::string& status,
                                    const std::optional<std::string>& expectedLatestRequestId = std::nullopt);
// 失败后从已提交批次继续；重复请求只返回当前状态，不再次占用任务。
ExamJson examExplanationRetryRecognition(Database& db, const std::string& id, const std::string& requestId);
ExamJson examExplanationSetTask(Database& db, const std::string& id, const std::string& taskId,
                                const std::string& status, const std::string& requestId = {});
void examExplanationFailTask(Database& db, const std::string& id, const std::string& requestId);
void examExplanationClearTask(Database& db, const std::string& id, const std::string& taskId);
ExamJson examExplanationPrompt(Database& db, const std::string& id, const std::string& action,
                               const std::string& text, const std::string& taskId,
                               int fromPage = 0, int pageLimit = 4);
void examExplanationApplyModel(Database& db, const std::string& id, const std::string& action,
                               const std::string& requestId, const AIResult& response, bool finalBatch = true,
                               const std::string& taskId = {}, int expectedPageOffset = -1);
void examExplanationAttachSources(Database& db, const std::string& id, const std::vector<SearchResource>& resources);
std::pair<std::string, std::string> examExplanationPage(Database& db, const std::string& id, int pageIndex);
std::pair<std::string, std::string> examExplanationFile(Database& db, const std::string& id, int fileIndex);

} // namespace gangyi

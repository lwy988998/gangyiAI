#pragma once

#include "db.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace gangyi {

using Json = nlohmann::json;

std::string classroomKey(const std::string& courseId, int phaseIndex, int topicIndex);
std::string diagnosticMode(const Json& answers, bool skipped);
bool credibleOpenEvaluation(const Json& evaluation);
bool sufficientPathEvidence(const Json& evidence, const std::string& direction);
Json defaultAvailability();
Json buildWeeklyDraft(const Json& availability, const Json& topics, const Json& reviews,
                      const std::string& monday, const std::string& notBefore = {});
Json publicQuestion(const Json& item);
// 对公开课堂及备课响应递归应用白名单，标准答案与内部生成字段仅留在本机。
Json publicLearningContent(const Json& content);
// 旧课程公开边界保留课纲与动态阶段键，只公开题目内容和生成来源。
Json publicCoursePayload(const Json& content);
Json classroomQuestions(Database& db, const Json& body);
Json classroomQuestion(Database& db, const Json& body);
Json classroomQuestionScope(Database& db, const Json& body);
std::string dialogueQuestionKey(const Json& body);
Json skipDialogueQuestion(Database& db, const Json& body);
Json completeReviewFromEvaluations(Database& db, const Json& body, const Json& results);
Json classroomStart(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                    const std::string& kind, const std::string& topic, const std::string& learningContext = "");
Json classroomSubmit(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                     const std::string& kind, int index, const Json& answer);
Json classroomSkip(Database& db, const std::string& courseId, int phaseIndex, int topicIndex);
Json classroomSkipActivity(Database& db, const std::string& courseId, int phaseIndex, int topicIndex, int index);
Json classroomState(Database& db, const std::string& courseId, int phaseIndex, int topicIndex);
Json classroomHint(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                   const std::string& kind, int index);
Json classroomRemedial(Database& db, const std::string& courseId, int phaseIndex, int topicIndex);
Json submitReview(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                  int day, const Json& answers, const std::string& today,
                  const std::string& reviewId = {}, const Json& displayedQuestions = Json());
Json dueReviews(Database& db, const std::string& courseId, const std::string& today);
Json getWeeklyPlan(Database& db, const std::string& courseId, const Json& topics, const std::string& monday,
                   const std::string& today = {});
Json editWeeklyPlan(Database& db, const std::string& courseId, const Json& body, const std::string& today = {});
Json replanWeeklyPlan(Database& db, const std::string& courseId, const Json& topics,
                      const std::string& monday, bool confirm, const Json& edits = Json::object(),
                      const std::string& today = {});
Json finishClassroom(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                     bool passed, const std::string& today, bool preserveOutline = true);

}

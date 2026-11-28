// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;

// 【逐行批注】 声明对象 classroomKey，并通过构造函数初始化其状态。
std::string classroomKey(const std::string& courseId, int phaseIndex, int topicIndex);
// 【逐行批注】 声明对象 diagnosticMode，并通过构造函数初始化其状态。
std::string diagnosticMode(const Json& answers, bool skipped);
// 【逐行批注】 声明对象 credibleOpenEvaluation，并通过构造函数初始化其状态。
bool credibleOpenEvaluation(const Json& evaluation);
// 【逐行批注】 声明对象 sufficientPathEvidence，并通过构造函数初始化其状态。
bool sufficientPathEvidence(const Json& evidence, const std::string& direction);
// 【逐行批注】 声明对象 defaultAvailability，并通过构造函数初始化其状态。
Json defaultAvailability();
// 【逐行批注】 声明对象 buildWeeklyDraft，并通过构造函数初始化其状态。
Json buildWeeklyDraft(const Json& availability, const Json& topics, const Json& reviews,
                      // 【逐行批注】 计算右侧结果并写入 std::string& monday, const std::string& notB…，更新当前运行状态。
                      const std::string& monday, const std::string& notBefore = {});
// 【逐行批注】 声明对象 publicQuestion，并通过构造函数初始化其状态。
Json publicQuestion(const Json& item);
// 对公开课堂及备课响应递归应用白名单，标准答案与内部生成字段仅留在本机。
// 【逐行批注】 声明对象 publicLearningContent，并通过构造函数初始化其状态。
Json publicLearningContent(const Json& content);
// 旧课程公开边界保留课纲与动态阶段键，只公开题目内容和生成来源。
// 【逐行批注】 声明对象 publicCoursePayload，并通过构造函数初始化其状态。
Json publicCoursePayload(const Json& content);
// 【逐行批注】 声明对象 classroomQuestions，并通过构造函数初始化其状态。
Json classroomQuestions(Database& db, const Json& body);
// 【逐行批注】 声明对象 classroomQuestion，并通过构造函数初始化其状态。
Json classroomQuestion(Database& db, const Json& body);
// 【逐行批注】 声明对象 classroomQuestionScope，并通过构造函数初始化其状态。
Json classroomQuestionScope(Database& db, const Json& body);
// 【逐行批注】 声明对象 dialogueQuestionKey，并通过构造函数初始化其状态。
std::string dialogueQuestionKey(const Json& body);
// 【逐行批注】 声明对象 skipDialogueQuestion，并通过构造函数初始化其状态。
Json skipDialogueQuestion(Database& db, const Json& body);
// 【逐行批注】 声明对象 completeReviewFromEvaluations，并通过构造函数初始化其状态。
Json completeReviewFromEvaluations(Database& db, const Json& body, const Json& results);
// 【逐行批注】 声明对象 classroomStart，并通过构造函数初始化其状态。
Json classroomStart(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                    // 【逐行批注】 计算右侧结果并写入 std::string& kind, const std::string& topic,…，更新当前运行状态。
                    const std::string& kind, const std::string& topic, const std::string& learningContext = "");
// 【逐行批注】 声明对象 classroomSubmit，并通过构造函数初始化其状态。
Json classroomSubmit(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                     // 【逐行批注】 执行当前声明或表达式：const std::string& kind, int index, const Json& answer);。
                     const std::string& kind, int index, const Json& answer);
// 【逐行批注】 声明对象 classroomSkip，并通过构造函数初始化其状态。
Json classroomSkip(Database& db, const std::string& courseId, int phaseIndex, int topicIndex);
// 【逐行批注】 声明对象 classroomSkipActivity，并通过构造函数初始化其状态。
Json classroomSkipActivity(Database& db, const std::string& courseId, int phaseIndex, int topicIndex, int index);
// 【逐行批注】 声明对象 classroomState，并通过构造函数初始化其状态。
Json classroomState(Database& db, const std::string& courseId, int phaseIndex, int topicIndex);
// 【逐行批注】 声明对象 classroomHint，并通过构造函数初始化其状态。
Json classroomHint(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                   // 【逐行批注】 执行当前声明或表达式：const std::string& kind, int index);。
                   const std::string& kind, int index);
// 【逐行批注】 声明对象 classroomRemedial，并通过构造函数初始化其状态。
Json classroomRemedial(Database& db, const std::string& courseId, int phaseIndex, int topicIndex);
// 【逐行批注】 声明对象 submitReview，并通过构造函数初始化其状态。
Json submitReview(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                  // 【逐行批注】 执行当前声明或表达式：int day, const Json& answers, const std::string& today,。
                  int day, const Json& answers, const std::string& today,
                  // 【逐行批注】 计算右侧结果并写入 std::string& reviewId，更新当前运行状态。
                  const std::string& reviewId = {}, const Json& displayedQuestions = Json());
// 【逐行批注】 声明对象 dueReviews，并通过构造函数初始化其状态。
Json dueReviews(Database& db, const std::string& courseId, const std::string& today);
// 【逐行批注】 声明对象 getWeeklyPlan，并通过构造函数初始化其状态。
Json getWeeklyPlan(Database& db, const std::string& courseId, const Json& topics, const std::string& monday,
                   // 【逐行批注】 计算右侧结果并写入 std::string& today，更新当前运行状态。
                   const std::string& today = {});
// 【逐行批注】 声明对象 editWeeklyPlan，并通过构造函数初始化其状态。
Json editWeeklyPlan(Database& db, const std::string& courseId, const Json& body, const std::string& today = {});
// 【逐行批注】 声明对象 replanWeeklyPlan，并通过构造函数初始化其状态。
Json replanWeeklyPlan(Database& db, const std::string& courseId, const Json& topics,
                      // 【逐行批注】 计算右侧结果并写入 std::string& monday, bool confirm, const Jso…，更新当前运行状态。
                      const std::string& monday, bool confirm, const Json& edits = Json::object(),
                      // 【逐行批注】 计算右侧结果并写入 std::string& today，更新当前运行状态。
                      const std::string& today = {});
// 【逐行批注】 声明对象 finishClassroom，并通过构造函数初始化其状态。
Json finishClassroom(Database& db, const std::string& courseId, int phaseIndex, int topicIndex,
                     // 【逐行批注】 计算右侧结果并写入 bool passed, const std::string& today, bool …，更新当前运行状态。
                     bool passed, const std::string& today, bool preserveOutline = true);

// 【逐行批注】 结束当前代码块。
}

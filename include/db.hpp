// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <optional>，为后续实现提供类型或接口声明。
#include <optional>
// 【逐行批注】 引入头文件 <functional>，为后续实现提供类型或接口声明。
#include <functional>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明数据结构或类型 sqlite3 及其成员接口。
struct sqlite3;

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 声明数据结构或类型 Course 及其成员接口。
struct Course {
    // 【逐行批注】 执行当前声明或表达式：std::string id; std::optional<std::string> anonymousId; std::optional<std::string> u…。
    std::string id; std::optional<std::string> anonymousId; std::optional<std::string> userId;
    // 【逐行批注】 执行当前声明或表达式：std::string goal, mode, title; std::optional<std::string> summary;。
    std::string goal, mode, title; std::optional<std::string> summary;
    // 【逐行批注】 计算右侧结果并写入 std::string source，更新当前运行状态。
    std::string source = "ai", status = "active", createdAt, updatedAt;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 User 及其成员接口。
struct User {
    // 【逐行批注】 执行当前声明或表达式：std::string id, email; std::optional<std::string> name; std::string passwordHash;。
    std::string id, email; std::optional<std::string> name; std::string passwordHash;
    // 【逐行批注】 计算右侧结果并写入 std::string membershipTier，更新当前运行状态。
    std::string membershipTier = "free", membershipStatus = "active";
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::string> membershipStartedAt, membershipExpiresAt; std::string cre…。
    std::optional<std::string> membershipStartedAt, membershipExpiresAt; std::string createdAt, updatedAt;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 UserSession 及其成员接口。
struct UserSession { std::string id, userId, tokenHash, createdAt, expiresAt; };
// 【逐行批注】 声明数据结构或类型 UsageCounter 及其成员接口。
struct UsageCounter {
    // 【逐行批注】 计算右侧结果并写入 std::string id, scopeId, scopeType, type, da…，更新当前运行状态。
    std::string id, scopeId, scopeType, type, dateKey; int count = 0;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 CourseProgress 及其成员接口。
struct CourseProgress {
    // 【逐行批注】 执行当前声明或表达式：std::string id, courseId; std::optional<std::string> anonymousId, userId;。
    std::string id, courseId; std::optional<std::string> anonymousId, userId;
    // 【逐行批注】 执行当前声明或表达式：std::string goal; std::optional<std::string> mode; int overallPercent = 0, completed…。
    std::string goal; std::optional<std::string> mode; int overallPercent = 0, completedCount = 0, totalCount = 0;
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::string> lastVisitedUrl, lastPageType; std::optional<int> lastPhas…。
    std::optional<std::string> lastVisitedUrl, lastPageType; std::optional<int> lastPhaseIndex;
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::string> lastPhaseName; std::optional<int> lastTopicIndex;。
    std::optional<std::string> lastPhaseName; std::optional<int> lastTopicIndex;
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::string> lastTopicTitle; std::string updatedAt, createdAt;。
    std::optional<std::string> lastTopicTitle; std::string updatedAt, createdAt;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 CourseSnapshot 及其成员接口。
struct CourseSnapshot { std::string id, courseId; int version = 1; std::string payload, createdAt; };
// 【逐行批注】 声明数据结构或类型 TaskProgress 及其成员接口。
struct TaskProgress {
    // 【逐行批注】 执行当前声明或表达式：std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;。
    std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::string> mode; int phaseIndex; std::string phaseName; int taskInde…。
    std::optional<std::string> mode; int phaseIndex; std::string phaseName; int taskIndex; std::string taskTitle;
    // 【逐行批注】 计算右侧结果并写入 std::string status，更新当前运行状态。
    std::string status = "not_started";
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 LearningStepProgress 及其成员接口。
struct LearningStepProgress {
    // 【逐行批注】 执行当前声明或表达式：std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;。
    std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::string> mode; int phaseIndex; std::string phaseName; int stepInde…。
    std::optional<std::string> mode; int phaseIndex; std::string phaseName; int stepIndex; std::string stepTitle;
    // 【逐行批注】 计算右侧结果并写入 std::string status，更新当前运行状态。
    std::string status = "not_started";
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 LearningCardProgress 及其成员接口。
struct LearningCardProgress {
    // 【逐行批注】 执行当前声明或表达式：std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;。
    std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::string> mode; int phaseIndex; std::string phaseName; int topicInd…。
    std::optional<std::string> mode; int phaseIndex; std::string phaseName; int topicIndex; std::string topicTitle;
    // 【逐行批注】 计算右侧结果并写入 std::string status，更新当前运行状态。
    std::string status = "not_started";
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 LearningSession 及其成员接口。
struct LearningSession {
    // 【逐行批注】 执行当前声明或表达式：std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;。
    std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::string> mode; int phaseIndex; std::string phaseName; int topicInd…。
    std::optional<std::string> mode; int phaseIndex; std::string phaseName; int topicIndex; std::string topicTitle;
    // 【逐行批注】 执行当前声明或表达式：std::string title; std::optional<std::string> summary, searchQuery; std::string cont…。
    std::string title; std::optional<std::string> summary, searchQuery; std::string content;
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::string> references; int fallbackUsed = 0; std::string source = "a…。
    std::optional<std::string> references; int fallbackUsed = 0; std::string source = "ai";
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 LearningInteraction 及其成员接口。
struct LearningInteraction {
    // 【逐行批注】 执行当前声明或表达式：std::string id, kind, payload, createdAt;。
    std::string id, kind, payload, createdAt;
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::string> courseId, conversationId, subject;。
    std::optional<std::string> courseId, conversationId, subject;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 SubjectMastery 及其成员接口。
struct SubjectMastery {
    // 【逐行批注】 计算右侧结果并写入 std::string subject, rationale, weakPoints，更新当前运行状态。
    std::string subject, rationale, weakPoints = "[]", recommendation, model, status = "pending", updatedAt;
    // 【逐行批注】 执行当前声明或表达式：std::optional<int> score;。
    std::optional<int> score;
    // 【逐行批注】 计算右侧结果并写入 int evidenceCount，更新当前运行状态。
    int evidenceCount = 0;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 ClassroomActivity 及其成员接口。
struct ClassroomActivity {
    // 【逐行批注】 执行当前声明或表达式：std::string id, courseId, kind, payload, updatedAt;。
    std::string id, courseId, kind, payload, updatedAt;
    // 【逐行批注】 计算右侧结果并写入 int phaseIndex，更新当前运行状态。
    int phaseIndex = 0, topicIndex = 0;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 WeeklyPlan 及其成员接口。
struct WeeklyPlan {
    // 【逐行批注】 执行当前声明或表达式：std::string courseId, payload, updatedAt;。
    std::string courseId, payload, updatedAt;
    // 【逐行批注】 计算右侧结果并写入 int version，更新当前运行状态。
    int version = 1;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 TopicMastery 及其成员接口。
struct TopicMastery {
    // 【逐行批注】 计算右侧结果并写入 std::string courseId, topic, rationale, weak…，更新当前运行状态。
    std::string courseId, topic, rationale, weakPoints = "[]", recommendation;
    // 【逐行批注】 计算右侧结果并写入 std::string evidenceIds，更新当前运行状态。
    std::string evidenceIds = "[]", nextReviewAt, model, status = "insufficient", updatedAt;
    // 【逐行批注】 计算右侧结果并写入 int phaseIndex，更新当前运行状态。
    int phaseIndex = 0, evidenceCount = 0, version = 0;
    // 【逐行批注】 执行当前声明或表达式：std::optional<int> score;。
    std::optional<int> score;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 个人中心的版本化记录；修改必须携带读取时的版本。
// 【逐行批注】 声明数据结构或类型 MyAIRecord 及其成员接口。
struct MyAIRecord { std::string id, kind, payload, updatedAt; int version = 0; };
// 【逐行批注】 声明数据结构或类型 IndependentChatSession 及其成员接口。
struct IndependentChatSession {
    // 【逐行批注】 执行当前声明或表达式：std::string lessonId, activeTaskId, createdAt, updatedAt;。
    std::string lessonId, activeTaskId, createdAt, updatedAt;
    // 【逐行批注】 计算右侧结果并写入 bool webEnabled，更新当前运行状态。
    bool webEnabled = false;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 IndependentChatMessage 及其成员接口。
struct IndependentChatMessage {
    // 【逐行批注】 计算右侧结果并写入 std::string id, lessonId, requestId, role, c…，更新当前运行状态。
    std::string id, lessonId, requestId, role, content, status = "ready";
    // 【逐行批注】 计算右侧结果并写入 std::string sources，更新当前运行状态。
    std::string sources = "[]", model, error, createdAt, updatedAt;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 Database 及其成员接口。
class Database {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 计算右侧结果并写入 Database()，更新当前运行状态。
    Database() = default;
    // 【逐行批注】 调用 Database 完成本行所需操作：~Database();。
    ~Database();
    // 【逐行批注】 计算右侧结果并写入 Database(const Database&)，更新当前运行状态。
    Database(const Database&) = delete;
    // 【逐行批注】 计算右侧结果并写入 Database& operator，更新当前运行状态。
    Database& operator=(const Database&) = delete;
    // 【逐行批注】 声明对象 open，并通过构造函数初始化其状态。
    void open(const std::string& path);
    // 【逐行批注】 声明对象 migrate，并通过构造函数初始化其状态。
    void migrate();
    // 【逐行批注】 声明对象 close，并通过构造函数初始化其状态。
    void close();
    // 在独立连接中一次提交完整课堂/评价；异常时恢复原有版本。
    // 【逐行批注】 声明对象 transaction，并通过构造函数初始化其状态。
    void transaction(const std::function<void()>& action);
    // 多次关联读取使用同一数据库快照，避免混合建课前后的权限与任务状态。
    // 【逐行批注】 声明对象 readSnapshot，并通过构造函数初始化其状态。
    void readSnapshot(const std::function<void()>& action);

    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const Course&); std::optional<Course> getCourse(const std::string&) const; std::vector<Course> listCourses() const; bool update(const Course&); bool deleteCourse(const std::string&);
    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const User&); std::optional<User> getUser(const std::string&) const; std::vector<User> listUsers() const; bool update(const User&); bool deleteUser(const std::string&); std::optional<User> findByEmail(const std::string&) const;
    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const UserSession&); std::optional<UserSession> getUserSession(const std::string&) const; std::vector<UserSession> listUserSessions() const; bool update(const UserSession&); bool deleteUserSession(const std::string&); std::optional<UserSession> findSessionByTokenHash(const std::string&) const;
    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const UsageCounter&); std::optional<UsageCounter> getUsageCounter(const std::string&) const; std::vector<UsageCounter> listUsageCounters() const; bool update(const UsageCounter&); bool deleteUsageCounter(const std::string&); std::optional<UsageCounter> findUsageCounter(const std::string&, const std::string&, const std::string&, const std::string&) const;
    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const CourseProgress&); std::optional<CourseProgress> getCourseProgress(const std::string&) const; std::vector<CourseProgress> listCourseProgress() const; bool update(const CourseProgress&); bool deleteCourseProgress(const std::string&); std::optional<CourseProgress> findProgressByCourseId(const std::string&) const;
    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const CourseSnapshot&); std::optional<CourseSnapshot> getCourseSnapshot(const std::string&) const; std::vector<CourseSnapshot> listCourseSnapshots() const; bool update(const CourseSnapshot&); bool deleteCourseSnapshot(const std::string&); std::vector<CourseSnapshot> findSnapshotsByCourseId(const std::string&) const;
    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const TaskProgress&); std::optional<TaskProgress> getTaskProgress(const std::string&) const; std::vector<TaskProgress> listTaskProgress() const; bool update(const TaskProgress&); bool deleteTaskProgress(const std::string&); std::optional<TaskProgress> findTaskProgress(const std::string&, int, int) const;
    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const LearningStepProgress&); std::optional<LearningStepProgress> getLearningStepProgress(const std::string&) const; std::vector<LearningStepProgress> listLearningStepProgress() const; bool update(const LearningStepProgress&); bool deleteLearningStepProgress(const std::string&); std::optional<LearningStepProgress> findLearningStepProgress(const std::string&, int, int) const;
    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const LearningCardProgress&); std::optional<LearningCardProgress> getLearningCardProgress(const std::string&) const; std::vector<LearningCardProgress> listLearningCardProgress() const; bool update(const LearningCardProgress&); bool deleteLearningCardProgress(const std::string&); std::optional<LearningCardProgress> findLearningCardProgress(const std::string&, int, int) const;
    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const LearningSession&); std::optional<LearningSession> getLearningSession(const std::string&) const; std::vector<LearningSession> listLearningSessions() const; bool update(const LearningSession&); bool deleteLearningSession(const std::string&); std::optional<LearningSession> findLearningSession(const std::string&, int, int) const;
    // 【逐行批注】 声明对象 insert，并通过构造函数初始化其状态。
    bool insert(const LearningInteraction&); std::vector<LearningInteraction> listInteractions() const;
    // 【逐行批注】 声明对象 upsert，并通过构造函数初始化其状态。
    bool upsert(const ClassroomActivity&); std::optional<ClassroomActivity> getClassroomActivity(const std::string&) const;
    // 执行轨迹单独追加保存，避免每次流式事件都重写整份任务 JSON。
    // 【逐行批注】 声明对象 appendAgentRunEvent，并通过构造函数初始化其状态。
    bool appendAgentRunEvent(const std::string& taskId, int seq, const std::string& payload, const std::string& createdAt);
    // 【逐行批注】 声明对象 agentRunEvents，并通过构造函数初始化其状态。
    std::vector<std::string> agentRunEvents(const std::string& taskId, int afterSeq = 0, int limit = 500) const;
    // 单次查询读取关联状态，避免后台评价完成时混合新旧版本。
    // 【逐行批注】 声明对象 classroomPayloadSnapshot，并通过构造函数初始化其状态。
    std::vector<std::string> classroomPayloadSnapshot(const std::vector<std::string>& keys) const;
    // 【逐行批注】 声明对象 listClassroomActivities，并通过构造函数初始化其状态。
    std::vector<ClassroomActivity> listClassroomActivities(const std::string&) const;
    // 【逐行批注】 声明对象 upsert，并通过构造函数初始化其状态。
    bool upsert(const WeeklyPlan&); std::optional<WeeklyPlan> getWeeklyPlan(const std::string&) const;
    // 【逐行批注】 声明对象 deleteClassroomData，并通过构造函数初始化其状态。
    void deleteClassroomData(const std::string&);
    // 【逐行批注】 声明对象 deleteConversation，并通过构造函数初始化其状态。
    bool deleteConversation(const std::string&); bool deleteInteractionsForCourse(const std::string&);
    // 【逐行批注】 声明对象 deleteInteractionsForConversation，并通过构造函数初始化其状态。
    void deleteInteractionsForConversation(const std::string& id);
    // 【逐行批注】 声明对象 upsert，并通过构造函数初始化其状态。
    bool upsert(const SubjectMastery&); void replaceMastery(const std::vector<SubjectMastery>&); std::vector<SubjectMastery> listMastery() const;
    // 【逐行批注】 声明对象 upsert，并通过构造函数初始化其状态。
    bool upsert(const TopicMastery&); std::vector<TopicMastery> listTopicMastery() const;
    // 【逐行批注】 声明对象 deleteTopicMasteryForCourse，并通过构造函数初始化其状态。
    bool deleteTopicMasteryForCourse(const std::string&);
    // 【逐行批注】 声明对象 profileDirty，并通过构造函数初始化其状态。
    bool profileDirty() const; int profileRevision() const; int profileAssessedRevision() const;
    // 【逐行批注】 声明对象 markProfileDirty，并通过构造函数初始化其状态。
    void markProfileDirty(); void setProfileAssessed(int);
    // 【逐行批注】 声明对象 profileError，并通过构造函数初始化其状态。
    std::string profileError() const; void setProfileError(const std::string&);
    // 【逐行批注】 声明对象 profileSearchStatus，并通过构造函数初始化其状态。
    std::string profileSearchStatus() const; std::string profileSources() const;
    // 【逐行批注】 声明对象 setProfileSearch，并通过构造函数初始化其状态。
    void setProfileSearch(const std::string& status, const std::string& sources);
    // 【逐行批注】 声明对象 profileMeta，并通过构造函数初始化其状态。
    std::string profileMeta(const std::string& key) const;
    // 【逐行批注】 声明对象 setProfileMeta，并通过构造函数初始化其状态。
    void setProfileMeta(const std::string& key, const std::string& value);
    // 【逐行批注】 声明对象 myAIRecord，并通过构造函数初始化其状态。
    std::optional<MyAIRecord> myAIRecord(const std::string& id) const;
    // 【逐行批注】 声明对象 myAIRecords，并通过构造函数初始化其状态。
    std::vector<MyAIRecord> myAIRecords(const std::string& kind) const;
    // 上下文失效签名只读取版本列，不装载记忆或对话正文。
    // 【逐行批注】 声明对象 myAIContextVersions，并通过构造函数初始化其状态。
    std::string myAIContextVersions(const std::string& chatId = {}) const;
    // 【逐行批注】 声明对象 saveMyAIRecord，并通过构造函数初始化其状态。
    bool saveMyAIRecord(const MyAIRecord& record, int expectedVersion);
    // 【逐行批注】 声明对象 independentChatSession，并通过构造函数初始化其状态。
    std::optional<IndependentChatSession> independentChatSession(const std::string& lessonId) const;
    // 【逐行批注】 声明对象 saveIndependentChatSession，并通过构造函数初始化其状态。
    bool saveIndependentChatSession(const IndependentChatSession& value);
    // 【逐行批注】 声明对象 independentChatMessages，并通过构造函数初始化其状态。
    std::vector<IndependentChatMessage> independentChatMessages(const std::string& lessonId, int limit = 200) const;
    // 【逐行批注】 声明对象 upsertIndependentChatMessage，并通过构造函数初始化其状态。
    bool upsertIndependentChatMessage(const IndependentChatMessage& value);
    // 【逐行批注】 声明对象 clearIndependentChat，并通过构造函数初始化其状态。
    bool clearIndependentChat(const std::string& lessonId);
    // 【逐行批注】 声明对象 setProfileMetaAtRevision，并通过构造函数初始化其状态。
    bool setProfileMetaAtRevision(const std::string& key, const std::string& value, int revision);
    // 【逐行批注】 声明对象 replaceMasteryAtRevision，并通过构造函数初始化其状态。
    bool replaceMasteryAtRevision(const std::vector<SubjectMastery>& values, int revision);
    // 【逐行批注】 声明对象 upsertTopicMasteryAtRevision，并通过构造函数初始化其状态。
    bool upsertTopicMasteryAtRevision(const TopicMastery& value, int revision);
    // 学习联动版本独立于画像评分版本；聊天可以影响教学，但不直接形成分数。
    // 【逐行批注】 声明对象 learningRevision，并通过构造函数初始化其状态。
    int learningRevision() const;
    // 【逐行批注】 声明对象 markLearningDirty，并通过构造函数初始化其状态。
    void markLearningDirty();
    // 【逐行批注】 声明对象 compareProfileMeta，并通过构造函数初始化其状态。
    bool compareProfileMeta(const std::string& key, const std::string& expected, const std::string& value);
    // 【逐行批注】 声明对象 compareClassroomActivity，并通过构造函数初始化其状态。
    bool compareClassroomActivity(const ClassroomActivity& value, const std::string& expected,
                                  // 【逐行批注】 计算右侧结果并写入 std::string& guardId，更新当前运行状态。
                                  const std::string& guardId = {}, const std::string& guardPayload = {});
    // 【逐行批注】 声明对象 updateLearningSessionAtRevision，并通过构造函数初始化其状态。
    bool updateLearningSessionAtRevision(const LearningSession& value, const std::string& expected,
                                        // 【逐行批注】 执行当前声明或表达式：int revision, const std::string& exposureKey,。
                                        int revision, const std::string& exposureKey,
                                        // 【逐行批注】 执行当前声明或表达式：const std::string& exposure);。
                                        const std::string& exposure);

// 【逐行批注】 计算右侧结果并写入 private: sqlite3* db_，更新当前运行状态。
private: sqlite3* db_ = nullptr;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 结束当前代码块。
}

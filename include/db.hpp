#pragma once

#include <optional>
#include <string>
#include <vector>

struct sqlite3;

namespace gangyi {

struct Course {
    std::string id; std::optional<std::string> anonymousId; std::optional<std::string> userId;
    std::string goal, mode, title; std::optional<std::string> summary;
    std::string source = "ai", status = "active", createdAt, updatedAt;
};
struct User {
    std::string id, email; std::optional<std::string> name; std::string passwordHash;
    std::string membershipTier = "free", membershipStatus = "active";
    std::optional<std::string> membershipStartedAt, membershipExpiresAt; std::string createdAt, updatedAt;
};
struct UserSession { std::string id, userId, tokenHash, createdAt, expiresAt; };
struct UsageCounter {
    std::string id, scopeId, scopeType, type, dateKey; int count = 0;
};
struct CourseProgress {
    std::string id, courseId; std::optional<std::string> anonymousId, userId;
    std::string goal; std::optional<std::string> mode; int overallPercent = 0, completedCount = 0, totalCount = 0;
    std::optional<std::string> lastVisitedUrl, lastPageType; std::optional<int> lastPhaseIndex;
    std::optional<std::string> lastPhaseName; std::optional<int> lastTopicIndex;
    std::optional<std::string> lastTopicTitle; std::string updatedAt, createdAt;
};
struct CourseSnapshot { std::string id, courseId; int version = 1; std::string payload, createdAt; };
struct TaskProgress {
    std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;
    std::optional<std::string> mode; int phaseIndex; std::string phaseName; int taskIndex; std::string taskTitle;
    std::string status = "not_started";
};
struct LearningStepProgress {
    std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;
    std::optional<std::string> mode; int phaseIndex; std::string phaseName; int stepIndex; std::string stepTitle;
    std::string status = "not_started";
};
struct LearningCardProgress {
    std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;
    std::optional<std::string> mode; int phaseIndex; std::string phaseName; int topicIndex; std::string topicTitle;
    std::string status = "not_started";
};
struct LearningSession {
    std::string id; std::optional<std::string> courseId, anonymousId; std::string goal;
    std::optional<std::string> mode; int phaseIndex; std::string phaseName; int topicIndex; std::string topicTitle;
    std::string title; std::optional<std::string> summary, searchQuery; std::string content;
    std::optional<std::string> references; int fallbackUsed = 0; std::string source = "ai";
};
struct LearningInteraction {
    std::string id, kind, payload, createdAt;
    std::optional<std::string> courseId, conversationId, subject;
};
struct SubjectMastery {
    std::string subject, rationale, weakPoints = "[]", recommendation, model, status = "pending", updatedAt;
    std::optional<int> score;
    int evidenceCount = 0;
};
struct ClassroomActivity {
    std::string id, courseId, kind, payload, updatedAt;
    int phaseIndex = 0, topicIndex = 0;
};
struct WeeklyPlan {
    std::string courseId, payload, updatedAt;
    int version = 1;
};
struct TopicMastery {
    std::string courseId, topic, rationale, weakPoints = "[]", recommendation;
    std::string evidenceIds = "[]", nextReviewAt, model, status = "insufficient", updatedAt;
    int phaseIndex = 0, evidenceCount = 0, version = 0;
    std::optional<int> score;
};

class Database {
public:
    Database() = default;
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    void open(const std::string& path);
    void migrate();
    void close();

    bool insert(const Course&); std::optional<Course> getCourse(const std::string&) const; std::vector<Course> listCourses() const; bool update(const Course&); bool deleteCourse(const std::string&);
    bool insert(const User&); std::optional<User> getUser(const std::string&) const; std::vector<User> listUsers() const; bool update(const User&); bool deleteUser(const std::string&); std::optional<User> findByEmail(const std::string&) const;
    bool insert(const UserSession&); std::optional<UserSession> getUserSession(const std::string&) const; std::vector<UserSession> listUserSessions() const; bool update(const UserSession&); bool deleteUserSession(const std::string&); std::optional<UserSession> findSessionByTokenHash(const std::string&) const;
    bool insert(const UsageCounter&); std::optional<UsageCounter> getUsageCounter(const std::string&) const; std::vector<UsageCounter> listUsageCounters() const; bool update(const UsageCounter&); bool deleteUsageCounter(const std::string&); std::optional<UsageCounter> findUsageCounter(const std::string&, const std::string&, const std::string&, const std::string&) const;
    bool insert(const CourseProgress&); std::optional<CourseProgress> getCourseProgress(const std::string&) const; std::vector<CourseProgress> listCourseProgress() const; bool update(const CourseProgress&); bool deleteCourseProgress(const std::string&); std::optional<CourseProgress> findProgressByCourseId(const std::string&) const;
    bool insert(const CourseSnapshot&); std::optional<CourseSnapshot> getCourseSnapshot(const std::string&) const; std::vector<CourseSnapshot> listCourseSnapshots() const; bool update(const CourseSnapshot&); bool deleteCourseSnapshot(const std::string&); std::vector<CourseSnapshot> findSnapshotsByCourseId(const std::string&) const;
    bool insert(const TaskProgress&); std::optional<TaskProgress> getTaskProgress(const std::string&) const; std::vector<TaskProgress> listTaskProgress() const; bool update(const TaskProgress&); bool deleteTaskProgress(const std::string&); std::optional<TaskProgress> findTaskProgress(const std::string&, int, int) const;
    bool insert(const LearningStepProgress&); std::optional<LearningStepProgress> getLearningStepProgress(const std::string&) const; std::vector<LearningStepProgress> listLearningStepProgress() const; bool update(const LearningStepProgress&); bool deleteLearningStepProgress(const std::string&); std::optional<LearningStepProgress> findLearningStepProgress(const std::string&, int, int) const;
    bool insert(const LearningCardProgress&); std::optional<LearningCardProgress> getLearningCardProgress(const std::string&) const; std::vector<LearningCardProgress> listLearningCardProgress() const; bool update(const LearningCardProgress&); bool deleteLearningCardProgress(const std::string&); std::optional<LearningCardProgress> findLearningCardProgress(const std::string&, int, int) const;
    bool insert(const LearningSession&); std::optional<LearningSession> getLearningSession(const std::string&) const; std::vector<LearningSession> listLearningSessions() const; bool update(const LearningSession&); bool deleteLearningSession(const std::string&); std::optional<LearningSession> findLearningSession(const std::string&, int, int) const;
    bool insert(const LearningInteraction&); std::vector<LearningInteraction> listInteractions() const;
    bool upsert(const ClassroomActivity&); std::optional<ClassroomActivity> getClassroomActivity(const std::string&) const;
    // 单次查询读取关联状态，避免后台评价完成时混合新旧版本。
    std::vector<std::string> classroomPayloadSnapshot(const std::vector<std::string>& keys) const;
    std::vector<ClassroomActivity> listClassroomActivities(const std::string&) const;
    bool upsert(const WeeklyPlan&); std::optional<WeeklyPlan> getWeeklyPlan(const std::string&) const;
    void deleteClassroomData(const std::string&);
    bool deleteConversation(const std::string&); bool deleteInteractionsForCourse(const std::string&);
    bool upsert(const SubjectMastery&); void replaceMastery(const std::vector<SubjectMastery>&); std::vector<SubjectMastery> listMastery() const;
    bool upsert(const TopicMastery&); std::vector<TopicMastery> listTopicMastery() const;
    bool deleteTopicMasteryForCourse(const std::string&);
    bool profileDirty() const; int profileRevision() const; int profileAssessedRevision() const;
    void markProfileDirty(); void setProfileAssessed(int);
    std::string profileError() const; void setProfileError(const std::string&);
    std::string profileSearchStatus() const; std::string profileSources() const;
    void setProfileSearch(const std::string& status, const std::string& sources);
    std::string profileMeta(const std::string& key) const;
    void setProfileMeta(const std::string& key, const std::string& value);
    bool setProfileMetaAtRevision(const std::string& key, const std::string& value, int revision);
    bool replaceMasteryAtRevision(const std::vector<SubjectMastery>& values, int revision);
    bool upsertTopicMasteryAtRevision(const TopicMastery& value, int revision);
    // 学习联动版本独立于画像评分版本；聊天可以影响教学，但不直接形成分数。
    int learningRevision() const;
    void markLearningDirty();
    bool compareProfileMeta(const std::string& key, const std::string& expected, const std::string& value);
    bool compareClassroomActivity(const ClassroomActivity& value, const std::string& expected,
                                  const std::string& guardId = {}, const std::string& guardPayload = {});
    bool updateLearningSessionAtRevision(const LearningSession& value, const std::string& expected,
                                        int revision, const std::string& exposureKey,
                                        const std::string& exposure);

private: sqlite3* db_ = nullptr;
};
}

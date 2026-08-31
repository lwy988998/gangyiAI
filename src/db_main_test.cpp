#include "db.hpp"

#include <cassert>
#include <iostream>

int main() {
    gangyi::Database database;
    database.open(":memory:");
    database.migrate();

    // User
    gangyi::User user{"user-1", "student@example.com", "Student", "hash"};
    user.createdAt = user.updatedAt = "2026-08-31T05:00:00Z";
    assert(database.insert(user));
    assert(database.findByEmail(user.email)->id == user.id);
    assert(database.getUser("user-1")->name == "Student");

    // Course
    gangyi::Course course;
    course.id = "course-1";
    course.userId = user.id;
    course.goal = "Learn C++";
    course.mode = "guided";
    course.title = "C++ basics";
    course.createdAt = course.updatedAt = "2026-08-31T05:00:00Z";
    assert(database.insert(course));
    assert(database.getCourse(course.id) && database.getCourse(course.id)->title == course.title);

    // UserSession
    gangyi::UserSession session{"sess-1", "user-1", "tokenhash123", "2026-08-31T05:00:00Z", "2026-09-01T05:00:00Z"};
    assert(database.insert(session));
    assert(database.findSessionByTokenHash("tokenhash123")->id == "sess-1");

    // UsageCounter
    gangyi::UsageCounter usage{"uc-1", "user-1", "user", "course_generate", "2026-08-31", 2};
    assert(database.insert(usage));
    assert(database.findUsageCounter("user-1", "user", "course_generate", "2026-08-31")->count == 2);

    // CourseProgress
    gangyi::CourseProgress progress;
    progress.id = "cp-1"; progress.courseId = "course-1"; progress.goal = "Learn C++";
    progress.overallPercent = 50; progress.completedCount = 2; progress.totalCount = 4;
    progress.updatedAt = progress.createdAt = "2026-08-31T05:00:00Z";
    assert(database.insert(progress));
    assert(database.findProgressByCourseId("course-1")->overallPercent == 50);

    // CourseSnapshot
    gangyi::CourseSnapshot snapshot{"snap-1", "course-1", 1, "{\"roadmap\":[]}", "2026-08-31T05:00:00Z"};
    assert(database.insert(snapshot));
    assert(database.findSnapshotsByCourseId("course-1").size() == 1);

    // TaskProgress
    gangyi::TaskProgress task;
    task.id = "tp-1"; task.courseId = "course-1"; task.goal = "Learn C++";
    task.phaseIndex = 1; task.phaseName = "Phase 1"; task.taskIndex = 0; task.taskTitle = "Task A";
    assert(database.insert(task));
    assert(database.findTaskProgress("course-1", 1, 0)->taskTitle == "Task A");
    task.status = "completed";
    assert(database.update(task));
    assert(database.getTaskProgress("tp-1")->status == "completed");

    // LearningStepProgress
    gangyi::LearningStepProgress step;
    step.id = "ls-1"; step.courseId = "course-1"; step.goal = "Learn C++";
    step.phaseIndex = 1; step.phaseName = "Phase 1"; step.stepIndex = 0; step.stepTitle = "Step A";
    assert(database.insert(step));
    assert(database.findLearningStepProgress("course-1", 1, 0)->stepTitle == "Step A");

    // LearningCardProgress
    gangyi::LearningCardProgress card;
    card.id = "lc-1"; card.courseId = "course-1"; card.goal = "Learn C++";
    card.phaseIndex = 1; card.phaseName = "Phase 1"; card.topicIndex = 0; card.topicTitle = "Topic A";
    assert(database.insert(card));
    assert(database.findLearningCardProgress("course-1", 1, 0)->topicTitle == "Topic A");

    // LearningSession
    gangyi::LearningSession learning;
    learning.id = "learn-1"; learning.courseId = "course-1"; learning.goal = "Learn C++";
    learning.phaseIndex = 1; learning.phaseName = "Phase 1"; learning.topicIndex = 0; learning.topicTitle = "Topic A";
    learning.title = "C++ basics lesson"; learning.content = "{\"lessonSteps\":[]}";
    learning.references = "[{\"url\":\"https://example.com\"}]";
    assert(database.insert(learning));
    assert(database.findLearningSession("course-1", 1, 0)->title == "C++ basics lesson");
    assert(database.findLearningSession("course-1", 1, 0)->references.has_value());

    // Delete checks
    assert(database.deleteCourse("course-1"));
    assert(!database.getCourse("course-1").has_value());

    std::cout << "database full test passed: "
              << database.listUsers().size() << " users, "
              << database.listCourses().size() << " courses\n";
}

#pragma once

#include <string>

namespace gangyi {

std::string renderHomePage();
std::string renderPlanPage(const std::string& goal, const std::string& mode,
                           const std::string& courseId = {}, const std::string& anonymousId = {});
std::string renderLearnPage(const std::string& courseId, const std::string& phaseIndex, const std::string& topicIndex);
std::string renderProgressPage(const std::string& courseId);
std::string renderLoginPage();
std::string renderAskPage(const std::string& goal);
std::string renderMyCoursesPage();

}  // namespace gangyi

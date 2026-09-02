#pragma once

#include <string>

namespace gangyi {

std::string renderHomePage();
std::string renderPlanPage(const std::string& goal, const std::string& mode,
                           const std::string& courseId = {}, const std::string& anonymousId = {});
std::string renderLearnPage(const std::string& courseId, const std::string& goal, const std::string& mode,
                            const std::string& phaseIndex, const std::string& phaseName,
                            const std::string& topicIndex, const std::string& topic,
                            const std::string& anonymousId = {}, const std::string& regenerate = {},
                            const std::string& forceLearn = {}, const std::string& retry = {});
std::string renderProgressPage(const std::string& courseId, const std::string& anonymousId = {});
std::string renderLoginPage();
std::string renderAskPage(const std::string& goal);
std::string renderMyCoursesPage();

}  // namespace gangyi

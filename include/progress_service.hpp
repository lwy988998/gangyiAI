#pragma once

#include "db.hpp"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace gangyi {

struct ProgressSaveResult {
    bool ok = false;
    nlohmann::json item = nlohmann::json::object();
};

std::string normalizeCardStatus(const std::string& status);
std::string normalizeStepStatus(const std::string& status);
ProgressSaveResult saveTaskProgress(Database& db, const nlohmann::json& body);
std::optional<nlohmann::json> recomputeCourseProgress(Database& db, const std::string& courseId,
                                                      const std::string& anonymousId,
                                                      const std::string& goal);
std::optional<nlohmann::json> resetCourseProgress(Database& db, const std::string& courseId,
                                                  const std::string& anonymousId,
                                                  const std::string& goal);
bool updateLastVisited(Database& db, const std::string& courseId, const std::string& anonymousId,
                       const std::string& goal, const std::string& mode,
                       const nlohmann::json& body);

}  // namespace gangyi

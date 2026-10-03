#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace gangyi {
class Database;
class AIClient;

nlohmann::json profileView(Database& db);
nlohmann::json recentCourses(Database& db);
nlohmann::json profileEvidenceSummary(Database& db);
std::string profileContext(Database& db);
bool refreshProfile(Database& db, AIClient& ai, std::string& error);
bool refreshTopicMastery(Database& db, AIClient& ai, std::string& error);
nlohmann::json radarPreferences(Database& db);
nlohmann::json saveRadarPreferences(Database& db, const nlohmann::json& value);
nlohmann::json abilityProfileView(Database& db);
bool refreshAbilityProfile(Database& db, AIClient& ai);
nlohmann::json nextLearning(Database& db, const std::string& courseId);
}

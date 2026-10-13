#pragma once
#include "db.hpp"
#include "library_io.hpp"
#include <nlohmann/json.hpp>
namespace gangyi {
class LearningAgent;
nlohmann::json libraryRead(Database& db, const nlohmann::json& query = nlohmann::json::object());
nlohmann::json libraryWrite(Database& db, const nlohmann::json& body);
nlohmann::json libraryContext(Database& db, const std::string& courseId = {}, const std::string& question = {});
nlohmann::json libraryUsage(Database& db);
nlohmann::json libraryValidateCitations(Database& db, const nlohmann::json& citations, bool teaching = true, const std::string& courseId = {});
void registerLibraryTools(LearningAgent& agent);
}

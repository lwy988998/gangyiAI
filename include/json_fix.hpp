#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace gangyi {
nlohmann::json parseAIJson(const std::string& content);
}

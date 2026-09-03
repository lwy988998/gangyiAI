#pragma once

#include "db.hpp"

#include <crow.h>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace gangyi {

struct AuthResult {
    bool ok = false;
    std::string error;
    User user;
    std::string token;
};

AuthResult registerUser(Database& db, const std::string& email, const std::string& name,
                        const std::string& password, const std::string& anonymousId);
AuthResult loginUser(Database& db, const std::string& email, const std::string& password,
                     const std::string& anonymousId);
std::optional<User> currentUser(Database& db, const crow::request& request);
bool logoutUser(Database& db, const crow::request& request);
nlohmann::json publicUser(const User& user);

}  // namespace gangyi

#pragma once

#include <cstdlib>
#include <string>

namespace gangyi {

struct Config {
    int port = 39002;
    std::string ai_base_url = "https://api.openai.com/v1";
    std::string ai_api_key;
    std::string ai_model = "gpt-4o-mini";
    std::string database_path = "gangyiAI.db";
    std::string admin_emails;

    static Config from_environment() {
        Config config;
        if (const char* value = std::getenv("PORT")) {
            try {
                config.port = std::stoi(value);
            } catch (...) {
                config.port = 39002;
            }
        }
        if (const char* value = std::getenv("AI_BASE_URL")) config.ai_base_url = value;
        if (const char* value = std::getenv("AI_API_KEY")) config.ai_api_key = value;
        if (const char* value = std::getenv("AI_MODEL")) config.ai_model = value;
        if (const char* value = std::getenv("DATABASE_PATH")) config.database_path = value;
        if (const char* value = std::getenv("ADMIN_EMAILS")) config.admin_emails = value;
        return config;
    }
};

}  // namespace gangyi

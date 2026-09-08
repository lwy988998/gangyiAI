#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>

namespace gangyi {

struct Config {
    std::string host = "0.0.0.0";
    int port = 39002;
    std::string ai_base_url = "https://api.deepseek.com/v1";
    std::string ai_api_key;
    std::string ai_model = "deepseek-chat";
    std::string database_path = "gangyiAI.db";
    std::string admin_emails;
    std::string local_control_token;

    static Config from_environment() {
        Config config;
        if (const char* value = std::getenv("HOST")) config.host = value;
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
#ifdef _WIN32
        if (const wchar_t* value = _wgetenv(L"DATABASE_PATH")) {
            config.database_path = std::filesystem::path(value).u8string();
        }
#else
        if (const char* value = std::getenv("DATABASE_PATH")) config.database_path = value;
#endif
        if (const char* value = std::getenv("ADMIN_EMAILS")) config.admin_emails = value;
        if (const char* value = std::getenv("LOCAL_CONTROL_TOKEN")) config.local_control_token = value;
        return config;
    }
};

}  // namespace gangyi

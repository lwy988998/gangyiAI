// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 声明数据结构或类型 Config 及其成员接口。
struct Config {
    // 【逐行批注】 计算右侧结果并写入 std::string host，更新当前运行状态。
    std::string host = "127.0.0.1";
    // 【逐行批注】 计算右侧结果并写入 int port，更新当前运行状态。
    int port = 39002;
    // 【逐行批注】 计算右侧结果并写入 std::string ai_base_url，更新当前运行状态。
    std::string ai_base_url = "https://api.deepseek.com/v1";
    // 【逐行批注】 执行当前声明或表达式：std::string ai_api_key;。
    std::string ai_api_key;
    // 【逐行批注】 计算右侧结果并写入 std::string ai_model，更新当前运行状态。
    std::string ai_model = "deepseek-chat";
    // 【逐行批注】 计算右侧结果并写入 std::string database_path，更新当前运行状态。
    std::string database_path = "gangyiAI.db";
    // 【逐行批注】 执行当前声明或表达式：std::string admin_emails;。
    std::string admin_emails;
    // 【逐行批注】 执行当前声明或表达式：std::string local_control_token;。
    std::string local_control_token;
    // 【逐行批注】 执行当前声明或表达式：std::string launch_session_id;。
    std::string launch_session_id;

    // 【逐行批注】 定义函数 from_environment，执行该函数声明的职责。
    static Config from_environment() {
        // 【逐行批注】 执行当前声明或表达式：Config config;。
        Config config;
        // 【逐行批注】 判断条件 (const char* value = std::getenv("HOST")) {，成立时执行对应分支。
        if (const char* value = std::getenv("HOST")) {
            // 【逐行批注】 判断条件 (std::string(value) == "127.0.0.1") config.host = value;，成立时执行对应分支。
            if (std::string(value) == "127.0.0.1") config.host = value;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (const char* value = std::getenv("PORT")) {，成立时执行对应分支。
        if (const char* value = std::getenv("PORT")) {
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try {
                // 【逐行批注】 计算右侧结果并写入 config.port，更新当前运行状态。
                config.port = std::stoi(value);
            // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
            } catch (...) {
                // 【逐行批注】 计算右侧结果并写入 config.port，更新当前运行状态。
                config.port = 39002;
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (const char* value = std::getenv("AI_BASE_URL")) config.ai_base_…，成立时执行对应分支。
        if (const char* value = std::getenv("AI_BASE_URL")) config.ai_base_url = value;
        // 【逐行批注】 判断条件 (const char* value = std::getenv("AI_API_KEY")) config.ai_api_ke…，成立时执行对应分支。
        if (const char* value = std::getenv("AI_API_KEY")) config.ai_api_key = value;
        // 【逐行批注】 判断条件 (const char* value = std::getenv("AI_MODEL")) config.ai_model = …，成立时执行对应分支。
        if (const char* value = std::getenv("AI_MODEL")) config.ai_model = value;
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
        // 【逐行批注】 判断条件 (const wchar_t* value = _wgetenv(L"DATABASE_PATH")) {，成立时执行对应分支。
        if (const wchar_t* value = _wgetenv(L"DATABASE_PATH")) {
            // 【逐行批注】 计算右侧结果并写入 config.database_path，更新当前运行状态。
            config.database_path = std::filesystem::path(value).u8string();
        // 【逐行批注】 结束当前代码块。
        }
// 【逐行批注】 处理条件编译指令：#else。
#else
        // 【逐行批注】 判断条件 (const char* value = std::getenv("DATABASE_PATH")) config.databa…，成立时执行对应分支。
        if (const char* value = std::getenv("DATABASE_PATH")) config.database_path = value;
// 【逐行批注】 处理条件编译指令：#endif。
#endif
        // 【逐行批注】 判断条件 (const char* value = std::getenv("ADMIN_EMAILS")) config.admin_e…，成立时执行对应分支。
        if (const char* value = std::getenv("ADMIN_EMAILS")) config.admin_emails = value;
        // 【逐行批注】 判断条件 (const char* value = std::getenv("LOCAL_CONTROL_TOKEN")) config.…，成立时执行对应分支。
        if (const char* value = std::getenv("LOCAL_CONTROL_TOKEN")) config.local_control_token = value;
        // 【逐行批注】 判断条件 (const char* value = std::getenv("GANGYI_LAUNCH_SESSION_ID")) co…，成立时执行对应分支。
        if (const char* value = std::getenv("GANGYI_LAUNCH_SESSION_ID")) config.launch_session_id = value;
        // 【逐行批注】 结束当前函数并返回结果：return config;。
        return config;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

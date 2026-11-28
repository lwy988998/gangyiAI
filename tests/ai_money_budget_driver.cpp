#include "ai_client.hpp"
#include "ai_money_budget.hpp"
#include <iostream>
#include <string>

// 专项夹具只能访问本机；所有账户与用量均为合成，不读取正式配置和凭据。
#ifdef _WIN32
int wmain(int argc, wchar_t** wideArgv) {
    std::vector<std::string> values;
    for (int index = 0; index < argc; ++index) {
        const std::wstring value(wideArgv[index]);
        const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
        if (size <= 0) return 2;
        std::string converted(size, '\0');
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), converted.data(), size, nullptr, nullptr);
        values.push_back(converted);
    }
    std::vector<char*> pointers; for (auto& value : values) pointers.push_back(value.data());
    char** argv = pointers.data();
#else
int main(int argc, char** argv) {
#endif
    if (argc < 3) return 2;
    const std::string command = argv[1], endpoint = argv[2];
    if (endpoint.rfind("http://127.0.0.1:", 0) != 0) return 2;
    try {
        gangyi::ChatOptions options;
        options.messages = {{"user", argc > 3 ? argv[3] : "预算验证"}};
        options.maxTokens = 128; options.maxAttempts = 2;
        if (command == "image") options.messages[0].imageDataUrl = "data:image/png;base64,synthetic";
        if (command == "search") options.searchQuery = "不应发起搜索";
        if (command == "reserve") {
            gangyi::AIMoneyBudget reservation;
            reservation.reserve(endpoint + "/chat/completions", "fixture-money", options, "{\"messages\":[]}");
            std::cout << "{\"reserved\":true}"; return 0;
        }
        gangyi::AIClient client({endpoint, "synthetic-only", "fixture-money", 1000, 2});
        const auto result = command == "stream" ? client.chatStream(options, [](const std::string&) { return true; }) : client.chat(options);
        std::cout << nlohmann::json{{"content", result.content}, {"usage", result.tokenUsage}}.dump(); return 0;
    } catch (const gangyi::AIClientError& error) {
        std::cout << nlohmann::json{{"errorType", error.errorType}, {"message", error.what()}}.dump(); return 1;
    } catch (const std::exception& error) {
        std::cout << nlohmann::json{{"errorType", "configuration_or_ledger"}, {"message", error.what()}}.dump(); return 1;
    }
}

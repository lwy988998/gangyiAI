#include "ai_client.hpp"
#include "json_fix.hpp"
#include <iostream>

int main() {
    gangyi::AIClient client;
    gangyi::ChatOptions options;
    options.messages = {
        {"system", "你是一个测试助手，只回复 OK 两个字母。"},
        {"user", "测试"}
    };
    options.temperature = 0.1;
    options.maxTokens = 50;
    options.responseFormat = "";
    try {
        gangyi::AIResult result = client.chat(options);
        std::cout << "SUCCESS model=" << result.model << " status=" << result.status << " content=[" << result.content << "]\n";
        return 0;
    } catch (const gangyi::AIClientError& e) {
        std::cout << "ERROR type=" << e.errorType << " msg=" << e.what() << "\n";
        return 1;
    }
}

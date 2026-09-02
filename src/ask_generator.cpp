#include "ask_generator.hpp"

#include "json_fix.hpp"

#include <algorithm>
#include <cstdlib>

namespace gangyi {
namespace {

using json = nlohmann::json;

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string normalizeMode(const std::string& mode) {
    return mode == "lite" ? "lite" : "deep";
}

int askTimeoutMs() {
    try {
        if (const char* value = std::getenv("AI_ASK_TIMEOUT_MS")) {
            const int timeout = std::stoi(value);
            if (timeout > 0) return timeout;
        }
    } catch (...) {}
    return 25000;
}

std::vector<std::string> strings(const json& value, size_t limit) {
    std::vector<std::string> result;
    if (!value.is_array()) return result;
    for (const auto& item : value) {
        if (!item.is_string()) continue;
        const std::string text = trim(item.get<std::string>());
        if (!text.empty()) result.push_back(text.substr(0, 2000));
        if (result.size() == limit) break;
    }
    return result;
}

std::vector<ChatMessage> prompt(const std::string& goal, const std::string& question, const std::string& mode) {
    const bool lite = mode == "lite";
    const std::string rules = lite
        ? "steps 2-4 条；commands 0-2 条；tips 1-2 条。"
        : "steps 3-6 条；commands 0-3 条；tips 2-4 条。";
    return {
        {"system", u8"你是钢一定制AI的学习问答助手，面向普通用户和学生，用中文回答具体问题。严格遵守用户的模式：lite 时精炼但可执行，deep 时系统且解释完整。优先解决安装、注册、登录、下载、环境配置、入门报错等具体操作；不要把具体操作扩展成长期课程。问题模糊时要求补充操作系统、报错信息或已尝试步骤。有命令时放入 commands 数组。不得编造外部搜索结果，不输出 Markdown 或代码块，只输出 JSON 对象：{\"title\":\"简短标题\",\"steps\":[\"步骤\"],\"commands\":[\"可复制命令\"],\"tips\":[\"提示\"]}。" + rules},
        {"user", u8"学习目标：" + goal + u8"\n用户问题：" + question},
    };
}

}  // namespace

AskGenerator::AskGenerator(AIClient& client) : client_(client) {}

AskAnswer AskGenerator::generate(const std::string& goal, const std::string& question, const std::string& mode) const {
    const std::string safeQuestion = trim(question);
    if (safeQuestion.empty()) throw AIClientError("invalid_request", "请提供问题");
    const std::string safeMode = normalizeMode(mode);
    ChatOptions options;
    options.messages = prompt(trim(goal).empty() ? u8"学习" : trim(goal), safeQuestion, safeMode);
    options.temperature = safeMode == "lite" ? 0.25 : 0.3;
    options.maxTokens = safeMode == "lite" ? 1500 : 2000;
    options.timeoutMs = askTimeoutMs();
    options.responseFormat = "json_object";
    options.maxAttempts = 1;

    const json raw = parseAIJson(client_.chat(options).content);
    if (!raw.is_object() || !raw.value("title", json()).is_string()) {
        throw AIClientError("invalid_response", "问答内容结构不完整");
    }
    AskAnswer answer;
    answer.title = trim(raw.value("title", ""));
    answer.steps = strings(raw.value("steps", json::array()), safeMode == "lite" ? 4 : 6);
    answer.commands = strings(raw.value("commands", json::array()), safeMode == "lite" ? 2 : 3);
    answer.tips = strings(raw.value("tips", json::array()), safeMode == "lite" ? 2 : 4);
    if (answer.title.empty() || answer.steps.empty()) {
        throw AIClientError("invalid_response", "问答内容结构不完整");
    }
    return answer;
}

}  // namespace gangyi

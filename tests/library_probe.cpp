#include "learning_library.hpp"
#include "learning_agent.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
// 开发验收入口不进入安装包；使用隔离档案，沿用现有模型和博查配置。
int main(int argc, char** argv) {
    using Json = nlohmann::json;
    try {
        if (argc != 4) return 2;
        std::ifstream file(std::filesystem::u8path(argv[1])); Json test; file >> test;
        gangyi::Database db; db.open(argv[2]); db.migrate(); gangyi::AIActivity::configureUsage(argv[2]);
        if (test.contains("preferences")) db.saveMyAIRecord({"preferences", "preferences", test["preferences"].dump(), "2026-10-13", 0}, 0);
        const gangyi::AgentAccess access{test.value("scopeId", "local-profile"), test.value("courseIds", std::vector<std::string>{})};
        Json sourceIds = Json::array();
        for (const auto& url : test.value("urls", Json::array())) sourceIds.push_back(gangyi::libraryWrite(db, {{"url", url}}).at("id"));
        const auto task = test.value("retryTaskId", "").empty() ?
            gangyi::agentSubmit(db, access, {{"type", "library_research"}, {"goal", test.at("goal")}, {"text", test.at("goal")}, {"requestId", test.value("requestId", "library-live-check")}}) :
            gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", test.at("retryTaskId")}});
        int calls = 0;
        Json promptAudit = Json::array();
        gangyi::LearningAgent agent(argv[2], [&](const gangyi::ChatOptions& options, const auto& sink) {
            if (test.value("readSourcesOnly", false)) {
                Json decision = {{"message", ""}, {"actions", Json::array()}, {"state", "completed"}};
                if (calls < static_cast<int>(sourceIds.size())) {
                    decision["state"] = "continue";
                    decision["actions"].push_back({{"id", "read-"+std::to_string(calls)}, {"tool", "read_resource"},
                        {"args", {{"sourceId", sourceIds.at(calls)}, {"refresh", false}}}});
                }
                ++calls;
                gangyi::AIResult result; result.content = decision.dump(); result.model = "原文读取专项夹具"; result.finishReason = "stop";
                sink(result.content); return result;
            }
            if (++calls > test.value("maxCalls", 8)) throw gangyi::AIClientError("eval_limit", "本次验收调用上限已到，保留实际记录");
            auto request = options; request.maxTokens = std::min(request.maxTokens, test.value("maxOutputTokens", 4096));
            size_t bytes = 0; for (const auto& message : request.messages) bytes += message.content.size();
            promptAudit.push_back({{"inputBytes", bytes}});
            return gangyi::AIClient().chatStream(request, sink);
        });
        agent.process(db, task.at("id"));
        Json output = {{"task", gangyi::agentView(db, access, task.at("id"))}, {"library", gangyi::libraryRead(db, Json::object())}};
        output["usage"] = Json::array();
        output["promptAudit"] = promptAudit;
        for(const auto& row:db.myAIRecords("usage")) output["usage"].push_back(Json::parse(row.payload));
        std::ofstream result(std::filesystem::u8path(argv[3])); result << output.dump(2);
        auto searchUsage=output["library"]["usage"]; searchUsage.erase("items");
        std::cout << Json{{"status",output["task"]["status"]},{"calls",calls},{"sources",output["library"]["items"].size()},{"searchUsage",searchUsage}}.dump() << '\n';
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

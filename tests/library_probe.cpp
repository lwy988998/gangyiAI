// 【逐行批注】 处理条件编译指令：#ifndef GANGYI_BASELINE_EVAL。
#ifndef GANGYI_BASELINE_EVAL
// 【逐行批注】 引入头文件 "learning_library.hpp"，为后续实现提供类型或接口声明。
#include "learning_library.hpp"
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 开发验收入口不进入安装包；使用隔离档案，沿用现有模型和博查配置。
// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main(int argc, char** argv) {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (argc != 4) return 2;，成立时执行对应分支。
        if (argc != 4) return 2;
        // 【逐行批注】 声明对象 file，并通过构造函数初始化其状态。
        std::ifstream file(std::filesystem::u8path(argv[1])); Json test; file >> test;
        // 【逐行批注】 调用 configureUsage 完成本行所需操作：gangyi::Database db; db.open(argv[2]); db.migrate(); gangyi::AIActivi…。
        gangyi::Database db; db.open(argv[2]); db.migrate(); gangyi::AIActivity::configureUsage(argv[2]);
        // 【逐行批注】 判断条件 (test.contains("preferences")) db.saveMyAIRecord({"preferences",…，成立时执行对应分支。
        if (test.contains("preferences")) db.saveMyAIRecord({"preferences", "preferences", test["preferences"].dump(), "2026-10-13", 0}, 0);
        // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess access{test.value("scopeId", "local-profile"), test.value(…。
        const gangyi::AgentAccess access{test.value("scopeId", "local-profile"), test.value("courseIds", std::vector<std::string>{})};
        // 【逐行批注】 计算右侧结果并写入 Json sourceIds，更新当前运行状态。
        Json sourceIds = Json::array();
// 【逐行批注】 处理条件编译指令：#ifdef GANGYI_BASELINE_EVAL。
#ifdef GANGYI_BASELINE_EVAL
        // 【逐行批注】 判断条件 (test.contains("urls") || test.value("readSourcesOnly", false))，成立时执行对应分支。
        if (test.contains("urls") || test.value("readSourcesOnly", false))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("基线验收不能注入新增资料能力");。
            throw std::invalid_argument("基线验收不能注入新增资料能力");
// 【逐行批注】 处理条件编译指令：#else。
#else
        // 【逐行批注】 按循环规则重复处理：for (const auto& url : test.value("urls", Json::array())) sourceIds.push_b…。
        for (const auto& url : test.value("urls", Json::array())) sourceIds.push_back(gangyi::libraryWrite(db, {{"url", url}}).at("id"));
// 【逐行批注】 处理条件编译指令：#endif。
#endif
        // 【逐行批注】 计算右侧结果并写入 Json event，更新当前运行状态。
        Json event={{"type", "library_research"}, {"goal", test.at("goal")}, {"text", test.at("goal")}, {"requestId", test.value("requestId", "library-live-check")}};
        // 【逐行批注】 按循环规则重复处理：for(const auto* field:{"sourceId","refresh"})if(test.contains(field))event…。
        for(const auto* field:{"sourceId","refresh"})if(test.contains(field))event[field]=test.at(field);
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = test.value("retryTaskId", "").empty() ?
            // 【逐行批注】 调用 agentSubmit 完成本行所需操作：gangyi::agentSubmit(db, access, event) :。
            gangyi::agentSubmit(db, access, event) :
            // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", te…。
            gangyi::agentControl(db, access, {{"command", "retry"}, {"taskId", test.at("retryTaskId")}});
        // 【逐行批注】 计算右侧结果并写入 int calls，更新当前运行状态。
        int calls = 0;
        // 【逐行批注】 计算右侧结果并写入 Json promptAudit，更新当前运行状态。
        Json promptAudit = Json::array();
        // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
        gangyi::LearningAgent agent(argv[2], [&](const gangyi::ChatOptions& options, const auto& sink) {
            // 【逐行批注】 判断条件 (test.value("readSourcesOnly", false)) {，成立时执行对应分支。
            if (test.value("readSourcesOnly", false)) {
                // 【逐行批注】 计算右侧结果并写入 Json decision，更新当前运行状态。
                Json decision = {{"message", ""}, {"actions", Json::array()}, {"state", "completed"}};
                // 【逐行批注】 判断条件 (calls < static_cast<int>(sourceIds.size())) {，成立时执行对应分支。
                if (calls < static_cast<int>(sourceIds.size())) {
                    // 【逐行批注】 计算右侧结果并写入 decision["state"]，更新当前运行状态。
                    decision["state"] = "continue";
                    // 【逐行批注】 调用 to_string 完成本行所需操作：decision["actions"].push_back({{"id", "read-"+std::to_string(calls)},…。
                    decision["actions"].push_back({{"id", "read-"+std::to_string(calls)}, {"tool", "read_resource"},
                        // 【逐行批注】 执行当前声明或表达式：{"args", {{"sourceId", sourceIds.at(calls)}, {"refresh", false}}}});。
                        {"args", {{"sourceId", sourceIds.at(calls)}, {"refresh", false}}}});
                    // 【逐行批注】 判断条件 (test.contains("readQueries")) decision["actions"].back()["args"…，成立时执行对应分支。
                    if (test.contains("readQueries")) decision["actions"].back()["args"]["query"] = test.at("readQueries").at(calls);
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 执行当前声明或表达式：++calls;。
                ++calls;
                // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result; result.content，更新当前运行状态。
                gangyi::AIResult result; result.content = decision.dump(); result.model = "原文读取专项夹具"; result.finishReason = "stop";
                // 【逐行批注】 调用 sink 完成本行所需操作：sink(result.content); return result;。
                sink(result.content); return result;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (++calls > test.value("maxCalls", 8)) throw gangyi::AIClientErro…，成立时执行对应分支。
            if (++calls > test.value("maxCalls", 8)) throw gangyi::AIClientError("eval_limit", "本次验收调用上限已到，保留实际记录");
            // 【逐行批注】 计算右侧结果并写入 request，更新当前运行状态。
            auto request = options; request.maxTokens = std::min(request.maxTokens, test.value("maxOutputTokens", 4096));
            // 【逐行批注】 计算右侧结果并写入 size_t bytes，更新当前运行状态。
            size_t bytes = 0; for (const auto& message : request.messages) bytes += message.content.size();
            // 【逐行批注】 执行当前声明或表达式：promptAudit.push_back({{"inputBytes", bytes}});。
            promptAudit.push_back({{"inputBytes", bytes}});
            // 【逐行批注】 结束当前函数并返回结果：return gangyi::AIClient().chatStream(request, sink);。
            return gangyi::AIClient().chatStream(request, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 执行当前声明或表达式：agent.process(db, task.at("id"));。
        agent.process(db, task.at("id"));
        // 【逐行批注】 计算右侧结果并写入 Json output，更新当前运行状态。
        Json output = {{"task", gangyi::agentView(db, access, task.at("id"))}};
// 【逐行批注】 处理条件编译指令：#ifndef GANGYI_BASELINE_EVAL。
#ifndef GANGYI_BASELINE_EVAL
        // 【逐行批注】 计算右侧结果并写入 output["library"]，更新当前运行状态。
        output["library"] = gangyi::libraryRead(db, Json::object());
// 【逐行批注】 处理条件编译指令：#endif。
#endif
        // 【逐行批注】 计算右侧结果并写入 output["usage"]，更新当前运行状态。
        output["usage"] = Json::array();
        // 【逐行批注】 计算右侧结果并写入 output["promptAudit"]，更新当前运行状态。
        output["promptAudit"] = promptAudit;
        // 【逐行批注】 按循环规则重复处理：for(const auto& row:db.myAIRecords("usage")) output["usage"].push_back(Jso…。
        for(const auto& row:db.myAIRecords("usage")) output["usage"].push_back(Json::parse(row.payload));
        // 【逐行批注】 声明对象 result，并通过构造函数初始化其状态。
        std::ofstream result(std::filesystem::u8path(argv[3])); result << output.dump(2);
        // 【逐行批注】 执行当前声明或表达式：std::cout << Json{{"status",output["task"]["status"]},{"calls",calls}}.dump() << '\n…。
        std::cout << Json{{"status",output["task"]["status"]},{"calls",calls}}.dump() << '\n';
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1;…。
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
// 【逐行批注】 结束当前代码块。
}

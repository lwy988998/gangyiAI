#pragma once
#include "ai_client.hpp"
#include "db.hpp"
#include <nlohmann/json.hpp>
#include <atomic>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace gangyi {
using AgentJson = nlohmann::json;
struct AgentAccess {
    std::string scopeId;
    std::vector<std::string> courseIds;
};
struct PreparedAgentTool {
    AgentJson result = AgentJson::object();
    // 真实模型调用在准备阶段完成；提交闭包只执行数据库操作。
    std::function<void(Database&)> commit;
};
struct AgentTool {
    std::string description;
    bool readOnly = false;
    std::function<PreparedAgentTool(Database&, const AgentJson&, const AgentJson&, const AIResult&)> prepare;
};
std::string agentLearnerFingerprint(Database& db, const AgentAccess& access);
AgentJson agentContext(Database& db, const AgentAccess& access, const AgentJson& event);
AgentJson agentSubmit(Database& db, const AgentAccess& access, const AgentJson& event);
AgentJson agentView(Database& db, const AgentAccess& access, const std::string& taskId = {});
AgentJson agentControl(Database& db, const AgentAccess& access, const AgentJson& body);

// 主控由真实模型选择工具与结束时机；程序只负责执行、持久化和冲突校验。
class LearningAgent {
public:
    using ModelCall = std::function<AIResult(const ChatOptions&, const std::function<bool(const std::string&)>&)>;
    explicit LearningAgent(std::string databasePath, ModelCall model = {});
    ~LearningAgent();
    void registerTool(const std::string& name, AgentTool tool);
    void start();
    void stop();
    void process(Database& db, const std::string& taskId);
private:
    void run();
    std::string databasePath_;
    ModelCall model_;
    std::map<std::string, AgentTool> tools_;
    std::atomic_bool stopped_{false};
    std::thread worker_;
};
} // namespace gangyi

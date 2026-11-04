// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "learning_library.hpp"，为后续实现提供类型或接口声明。
#include "learning_library.hpp"
// 【逐行批注】 引入头文件 "agent_efficiency.hpp"，为后续实现提供类型或接口声明。
#include "agent_efficiency.hpp"
// 【逐行批注】 引入头文件 "agent_preferences.hpp"，为后续实现提供类型或接口声明。
#include "agent_preferences.hpp"
// 【逐行批注】 引入头文件 <cassert>，为后续实现提供类型或接口声明。
#include <cassert>

// 走真实缓存读取和推荐持久化路径，只用固定模型回调，不发模型或资料网络请求。
// 【逐行批注】 定义函数 runLibraryRevisionChecks，执行该函数声明的职责。
inline void runLibraryRevisionChecks() {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 【逐行批注】 执行当前声明或表达式：Database db; db.open(":memory:"); db.migrate();。
    Database db; db.open(":memory:"); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：const AgentAccess access{"library-revision-check", {}};。
    const AgentAccess access{"library-revision-check", {}};
    // 【逐行批注】 计算右侧结果并写入 auto source，更新当前运行状态。
    const auto source = libraryWrite(db, {{"url", "https://example.org/revision-fixture"}, {"title", "版本隔离合成资料"}});
    // 【逐行批注】 计算右侧结果并写入 std::string sourceId，更新当前运行状态。
    const std::string sourceId = source.at("id");
    // 【逐行批注】 计算右侧结果并写入 Json snapshot，更新当前运行状态。
    const Json snapshot = {{"sourceId", sourceId}, {"checkedAt", "2026-10-27T00:00:00Z"},
        // 【逐行批注】 执行当前声明或表达式：{"segments", {{{"id", "s1"}, {"text", "仅用于缓存版本检查的合成原文。"}}}}};。
        {"segments", {{{"id", "s1"}, {"text", "仅用于缓存版本检查的合成原文。"}}}}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"revision-snapshot", "library-snapshot", sn…。
    assert(db.saveMyAIRecord({"revision-snapshot", "library-snapshot", snapshot.dump(), "now", 0}, 0));
    // 【逐行批注】 计算右侧结果并写入 stored，更新当前运行状态。
    auto stored = Json::parse(db.myAIRecord(sourceId)->payload);
    // 【逐行批注】 计算右侧结果并写入 stored["snapshotId"]，更新当前运行状态。
    stored["snapshotId"] = "revision-snapshot"; stored["readStatus"] = "read";
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({sourceId, "library-source", stored.dump(), …。
    assert(db.saveMyAIRecord({sourceId, "library-source", stored.dump(), "now", 0}, source.at("version")));
    // 【逐行批注】 计算右侧结果并写入 auto response，更新当前运行状态。
    const auto response = [](const Json& value, const auto& sink) {
        // 【逐行批注】 计算右侧结果并写入 AIResult reply; reply.content，更新当前运行状态。
        AIResult reply; reply.content = value.dump(); reply.model = "资料版本离线夹具"; reply.finishReason = "stop";
        // 【逐行批注】 调用 sink 完成本行所需操作：sink(reply.content); return reply;。
        sink(reply.content); return reply;
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 int recommendationCalls，更新当前运行状态。
    int recommendationCalls = 0;
    // 【逐行批注】 定义函数 recommender，执行该函数声明的职责。
    LearningAgent recommender(":memory:", [&](const auto&, const auto& sink) {
        // 【逐行批注】 执行当前声明或表达式：++recommendationCalls;。
        ++recommendationCalls;
        // 【逐行批注】 结束当前函数并返回结果：return response({{"message", "依据合成状态保存推荐。"}, {"state", "completed"},。
        return response({{"message", "依据合成状态保存推荐。"}, {"state", "completed"},
            // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "recommend"}, {"tool", "update_recommendations"},。
            {"actions", {{{"id", "recommend"}, {"tool", "update_recommendations"},
                // 【逐行批注】 执行当前声明或表达式：{"args", {{"lite", {"合成概念"}}, {"deep", {"合成练习"}}, {"reason", "离线检查"}}}}}}}, sink);。
                {"args", {{"lite", {"合成概念"}}, {"deep", {"合成练习"}}, {"reason", "离线检查"}}}}}}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentPreferenceTools 完成本行所需操作：registerAgentPreferenceTools(recommender);。
    registerAgentPreferenceTools(recommender);
    // 【逐行批注】 计算右侧结果并写入 int launches，更新当前运行状态。
    int launches = 0;
    // 【逐行批注】 计算右侧结果并写入 auto launch，更新当前运行状态。
    const auto launch = [&] { return agentSubmit(db, access, {{"type", "startup"},
        // 【逐行批注】 调用 to_string 完成本行所需操作：{"requestId", "revision-launch-" + std::to_string(++launches)}, {"tex…。
        {"requestId", "revision-launch-" + std::to_string(++launches)}, {"text", "读取合成资料并保存推荐"}}); };
    // 【逐行批注】 计算右侧结果并写入 first，更新当前运行状态。
    auto first = launch(); recommender.process(db, first.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, first.at("id")).at("status") == "ready" …。
    assert(agentView(db, access, first.at("id")).at("status") == "ready" && recommendationCalls == 1);

    // 【逐行批注】 计算右侧结果并写入 AgentContextReuse context; int builds，更新当前运行状态。
    AgentContextReuse context; int builds = 0;
    // 【逐行批注】 计算右侧结果并写入 Json contextEvent，更新当前运行状态。
    const Json contextEvent = {{"type", "library_research"}, {"goal", "缓存版本检查"}};
    // 【逐行批注】 计算右侧结果并写入 auto readContext，更新当前运行状态。
    const auto readContext = [&] { return context.read(db, access, contextEvent, [&] {
        // 【逐行批注】 调用 agentContext 完成本行所需操作：++builds; return agentContext(db, access, contextEvent);。
        ++builds; return agentContext(db, access, contextEvent);
    // 【逐行批注】 执行当前声明或表达式：}); };。
    }); };
    // 【逐行批注】 调用 readContext、assert 完成本行所需操作：readContext(); assert(builds == 1);。
    readContext(); assert(builds == 1);
    // 【逐行批注】 计算右侧结果并写入 auto revision，更新当前运行状态。
    const auto revision = db.profileMeta("library-revision");
    // 【逐行批注】 计算右侧结果并写入 auto originalSource，更新当前运行状态。
    const auto originalSource = db.myAIRecord(sourceId), originalSnapshot = db.myAIRecord("revision-snapshot");
    // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
    const auto task = agentSubmit(db, access, {{"type", "library_research"}, {"goal", "只读已保存快照"}, {"requestId", "revision-cache-hit"}});
    // 【逐行批注】 定义函数 reader，执行该函数声明的职责。
    LearningAgent reader(":memory:", [&](const auto&, const auto& sink) {
        // 【逐行批注】 结束当前函数并返回结果：return response({{"message", ""}, {"state", "completed"},。
        return response({{"message", ""}, {"state", "completed"},
            // 【逐行批注】 执行当前声明或表达式：{"actions", {{{"id", "cached-read"}, {"tool", "read_resource"}, {"args", {{"sourceId…。
            {"actions", {{{"id", "cached-read"}, {"tool", "read_resource"}, {"args", {{"sourceId", sourceId}}}}}}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 执行当前声明或表达式：reader.process(db, task.at("id"));。
    reader.process(db, task.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, task.at("id")).at("status") == "ready");。
    assert(agentView(db, access, task.at("id")).at("status") == "ready");
    // 【逐行批注】 调用 assert、libraryUsage 完成本行所需操作：assert(libraryUsage(db).at("cacheHits") == 1 && libraryUsage(db).at("…。
    assert(libraryUsage(db).at("cacheHits") == 1 && libraryUsage(db).at("fetchRequests") == 0);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("library-revision") == revision);。
    assert(db.profileMeta("library-revision") == revision);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.myAIRecord(sourceId)->payload == originalSource->payload &&…。
    assert(db.myAIRecord(sourceId)->payload == originalSource->payload && db.myAIRecord(sourceId)->version == originalSource->version);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.myAIRecord("revision-snapshot")->payload == originalSnapsho…。
    assert(db.myAIRecord("revision-snapshot")->payload == originalSnapshot->payload && db.myAIRecord("revision-snapshot")->version == originalSnapshot->version);
    // 【逐行批注】 调用 readContext、assert 完成本行所需操作：readContext(); assert(builds == 1);。
    readContext(); assert(builds == 1);
    // 【逐行批注】 计算右侧结果并写入 reused，更新当前运行状态。
    auto reused = launch(); assert(reused.at("id") == first.at("id") && reused.at("reuseReason") == "unchanged_startup_state");
    // 【逐行批注】 调用 assert 完成本行所需操作：recommender.process(db, reused.at("id")); assert(recommendationCalls …。
    recommender.process(db, reused.at("id")); assert(recommendationCalls == 1);

    // 真正修改来源与手动选用仍使两层复用失效；不扩大统计记录的例外范围。
    // 【逐行批注】 计算右侧结果并写入 detail，更新当前运行状态。
    auto detail = libraryRead(db, {{"id", sourceId}});
    // 【逐行批注】 调用 libraryWrite 完成本行所需操作：libraryWrite(db, {{"id", sourceId}, {"version", detail.at("version")}…。
    libraryWrite(db, {{"id", sourceId}, {"version", detail.at("version")}, {"favorite", true}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("library-revision") != revision);。
    assert(db.profileMeta("library-revision") != revision);
    // 【逐行批注】 调用 readContext、assert 完成本行所需操作：readContext(); assert(builds == 2);。
    readContext(); assert(builds == 2);
    // 【逐行批注】 计算右侧结果并写入 changed，更新当前运行状态。
    auto changed = launch(); assert(changed.at("id") != first.at("id"));
    // 【逐行批注】 调用 assert 完成本行所需操作：recommender.process(db, changed.at("id")); assert(recommendationCalls…。
    recommender.process(db, changed.at("id")); assert(recommendationCalls == 2);
    // 【逐行批注】 计算右侧结果并写入 auto sourceRevision，更新当前运行状态。
    const auto sourceRevision = db.profileMeta("library-revision");
    // 【逐行批注】 计算右侧结果并写入 detail，更新当前运行状态。
    detail = libraryRead(db, {{"id", sourceId}});
    // 【逐行批注】 调用 libraryWrite 完成本行所需操作：libraryWrite(db, {{"id", sourceId}, {"version", detail.at("version")}…。
    libraryWrite(db, {{"id", sourceId}, {"version", detail.at("version")}, {"state", "selected"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("library-revision") != sourceRevision);。
    assert(db.profileMeta("library-revision") != sourceRevision);
    // 【逐行批注】 调用 readContext、assert 完成本行所需操作：readContext(); assert(builds == 3);。
    readContext(); assert(builds == 3);
    // 【逐行批注】 调用 assert、launch 完成本行所需操作：assert(launch().at("id") != changed.at("id"));。
    assert(launch().at("id") != changed.at("id"));
// 【逐行批注】 结束当前代码块。
}

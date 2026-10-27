#pragma once
#include "learning_library.hpp"
#include "agent_efficiency.hpp"
#include "agent_preferences.hpp"
#include <cassert>

// 走真实缓存读取和推荐持久化路径，只用固定模型回调，不发模型或资料网络请求。
inline void runLibraryRevisionChecks() {
    using namespace gangyi;
    using Json = nlohmann::json;
    Database db; db.open(":memory:"); db.migrate();
    const AgentAccess access{"library-revision-check", {}};
    const auto source = libraryWrite(db, {{"url", "https://example.org/revision-fixture"}, {"title", "版本隔离合成资料"}});
    const std::string sourceId = source.at("id");
    const Json snapshot = {{"sourceId", sourceId}, {"checkedAt", "2026-10-27T00:00:00Z"},
        {"segments", {{{"id", "s1"}, {"text", "仅用于缓存版本检查的合成原文。"}}}}};
    assert(db.saveMyAIRecord({"revision-snapshot", "library-snapshot", snapshot.dump(), "now", 0}, 0));
    auto stored = Json::parse(db.myAIRecord(sourceId)->payload);
    stored["snapshotId"] = "revision-snapshot"; stored["readStatus"] = "read";
    assert(db.saveMyAIRecord({sourceId, "library-source", stored.dump(), "now", 0}, source.at("version")));
    const auto response = [](const Json& value, const auto& sink) {
        AIResult reply; reply.content = value.dump(); reply.model = "资料版本离线夹具"; reply.finishReason = "stop";
        sink(reply.content); return reply;
    };
    int recommendationCalls = 0;
    LearningAgent recommender(":memory:", [&](const auto&, const auto& sink) {
        ++recommendationCalls;
        return response({{"message", "依据合成状态保存推荐。"}, {"state", "completed"},
            {"actions", {{{"id", "recommend"}, {"tool", "update_recommendations"},
                {"args", {{"lite", {"合成概念"}}, {"deep", {"合成练习"}}, {"reason", "离线检查"}}}}}}}, sink);
    });
    registerAgentPreferenceTools(recommender);
    int launches = 0;
    const auto launch = [&] { return agentSubmit(db, access, {{"type", "startup"},
        {"requestId", "revision-launch-" + std::to_string(++launches)}, {"text", "读取合成资料并保存推荐"}}); };
    auto first = launch(); recommender.process(db, first.at("id"));
    assert(agentView(db, access, first.at("id")).at("status") == "ready" && recommendationCalls == 1);

    AgentContextReuse context; int builds = 0;
    const Json contextEvent = {{"type", "library_research"}, {"goal", "缓存版本检查"}};
    const auto readContext = [&] { return context.read(db, access, contextEvent, [&] {
        ++builds; return agentContext(db, access, contextEvent);
    }); };
    readContext(); assert(builds == 1);
    const auto revision = db.profileMeta("library-revision");
    const auto originalSource = db.myAIRecord(sourceId), originalSnapshot = db.myAIRecord("revision-snapshot");
    const auto task = agentSubmit(db, access, {{"type", "library_research"}, {"goal", "只读已保存快照"}, {"requestId", "revision-cache-hit"}});
    LearningAgent reader(":memory:", [&](const auto&, const auto& sink) {
        return response({{"message", ""}, {"state", "completed"},
            {"actions", {{{"id", "cached-read"}, {"tool", "read_resource"}, {"args", {{"sourceId", sourceId}}}}}}}, sink);
    });
    reader.process(db, task.at("id"));
    assert(agentView(db, access, task.at("id")).at("status") == "ready");
    assert(libraryUsage(db).at("cacheHits") == 1 && libraryUsage(db).at("fetchRequests") == 0);
    assert(db.profileMeta("library-revision") == revision);
    assert(db.myAIRecord(sourceId)->payload == originalSource->payload && db.myAIRecord(sourceId)->version == originalSource->version);
    assert(db.myAIRecord("revision-snapshot")->payload == originalSnapshot->payload && db.myAIRecord("revision-snapshot")->version == originalSnapshot->version);
    readContext(); assert(builds == 1);
    auto reused = launch(); assert(reused.at("id") == first.at("id") && reused.at("reuseReason") == "unchanged_startup_state");
    recommender.process(db, reused.at("id")); assert(recommendationCalls == 1);

    // 真正修改来源与手动选用仍使两层复用失效；不扩大统计记录的例外范围。
    auto detail = libraryRead(db, {{"id", sourceId}});
    libraryWrite(db, {{"id", sourceId}, {"version", detail.at("version")}, {"favorite", true}});
    assert(db.profileMeta("library-revision") != revision);
    readContext(); assert(builds == 2);
    auto changed = launch(); assert(changed.at("id") != first.at("id"));
    recommender.process(db, changed.at("id")); assert(recommendationCalls == 2);
    const auto sourceRevision = db.profileMeta("library-revision");
    detail = libraryRead(db, {{"id", sourceId}});
    libraryWrite(db, {{"id", sourceId}, {"version", detail.at("version")}, {"state", "selected"}});
    assert(db.profileMeta("library-revision") != sourceRevision);
    readContext(); assert(builds == 3);
    assert(launch().at("id") != changed.at("id"));
}

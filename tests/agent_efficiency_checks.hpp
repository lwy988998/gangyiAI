// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 "agent_efficiency.hpp"，为后续实现提供类型或接口声明。
#include "agent_efficiency.hpp"
// 【逐行批注】 引入头文件 "agent_prompt.hpp"，为后续实现提供类型或接口声明。
#include "agent_prompt.hpp"
// 【逐行批注】 引入头文件 "my_ai_context.hpp"，为后续实现提供类型或接口声明。
#include "my_ai_context.hpp"

// 【逐行批注】 定义函数 efficiencySwitch，执行该函数声明的职责。
inline void efficiencySwitch(const char* key, const char* value) {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 _putenv_s 完成本行所需操作：_putenv_s(key, value);。
    _putenv_s(key, value);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 判断条件 (*value) setenv(key, value, 1); else unsetenv(key);，成立时执行对应分支。
    if (*value) setenv(key, value, 1); else unsetenv(key);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 runAgentEfficiencyChecks，执行该函数声明的职责。
inline void runAgentEfficiencyChecks(const std::filesystem::path& path) {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 【逐行批注】 执行当前声明或表达式：Database db; db.open(path.string()); db.migrate();。
    Database db; db.open(path.string()); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：const AgentAccess access{"efficiency", {"math", "science"}};。
    const AgentAccess access{"efficiency", {"math", "science"}};
    // 【逐行批注】 按循环规则重复处理：for (const auto* id : {"math", "science"}) assert(db.insert(Course{id, acc…。
    for (const auto* id : {"math", "science"}) assert(db.insert(Course{id, access.scopeId, {}, id, "deep", id, {}, "ai", "active", "2026-10-19", "2026-10-19"}));
    // 【逐行批注】 计算右侧结果并写入 Json event，更新当前运行状态。
    const Json event = {{"type", "chat"}, {"courseId", "math"}, {"requestId", "projection"}, {"text", "按原题补讲"}};
    // 【逐行批注】 计算右侧结果并写入 Json evidence，更新当前运行状态。
    const Json evidence = {{"scopeId", access.scopeId}, {"text", "学生原始回答"}, {"questionSnapshot", {{"question", "1/2+1/3"}, {"rubric", "原题私有标准"}}},
        // 【逐行批注】 执行当前声明或表达式：{"helpHistory", {{{"type", "hint"}, {"text", "原始小提示"}}}}, {"relatedLearning", {"普通讲解…。
        {"helpHistory", {{{"type", "hint"}, {"text", "原始小提示"}}}}, {"relatedLearning", {"普通讲解"}},
        // 【逐行批注】 执行当前声明或表达式：{"citations", {{{"sourceId", "source"}, {"snapshotId", "snapshot"}, {"segmentId", "p…。
        {"citations", {{{"sourceId", "source"}, {"snapshotId", "snapshot"}, {"segmentId", "p1"}}}}, {"demonstratedQuestions", {"示范原题"}}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(LearningInteraction{"evidence", "chat-user", evidenc…。
    assert(db.insert(LearningInteraction{"evidence", "chat-user", evidence.dump(), "2026-10-19", "math", "conversation", {}}));
    // 【逐行批注】 调用 assert、string 完成本行所需操作：assert(db.insert(LearningInteraction{"other", "chat-user", Json{{"tex…。
    assert(db.insert(LearningInteraction{"other", "chat-user", Json{{"text", std::string(20000, 'x')}}.dump(), "2026-10-19", "science", "conversation", {}}));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(LearningInteraction{"private", "chat-user", Json{{"s…。
    assert(db.insert(LearningInteraction{"private", "chat-user", Json{{"scopeId", "other-owner"}, {"text", "OTHER-SCOPE-SECRET"}}.dump(), "2026-10-19", "math", "conversation", {}}));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"preferences", "preferences", Json{{"instru…。
    assert(db.saveMyAIRecord({"preferences", "preferences", Json{{"instructions", "手动要求优先"}, {"autoMemory", false}}.dump(), "now", 0}, 0));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"memory", "memory", Json{{"content", "旧记忆"}…。
    assert(db.saveMyAIRecord({"memory", "memory", Json{{"content", "旧记忆"}, {"source", "manual"}}.dump(), "now", 0}, 0));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"foreign-memory", "memory", Json{{"content"…。
    assert(db.saveMyAIRecord({"foreign-memory", "memory", Json{{"content", "另一课程的推测"}, {"source", "inferred"}, {"courseId", "science"}}.dump(), "now", 0}, 0));
    // 【逐行批注】 计算右侧结果并写入 full，更新当前运行状态。
    auto full = agentContext(db, access, event);
    // 【逐行批注】 计算右侧结果并写入 compact，更新当前运行状态。
    auto compact = agentPromptContext(full, false);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(compact.at("feedback").size() == 1);。
    assert(compact.at("feedback").size() == 1);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(compact.at("feedback")[0].at("payload").at("questionSnapshot")…。
    assert(compact.at("feedback")[0].at("payload").at("questionSnapshot") == evidence.at("questionSnapshot"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(compact.at("feedback")[0].at("payload").at("citations") == evi…。
    assert(compact.at("feedback")[0].at("payload").at("citations") == evidence.at("citations"));
    // 【逐行批注】 调用 assert、array 完成本行所需操作：assert(compact.at("feedback")[0].at("originalRecord").at("ids") == Js…。
    assert(compact.at("feedback")[0].at("originalRecord").at("ids") == Json::array({"evidence"}));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(compact.at("personalCenter").at("preferences") == full.at("per…。
    assert(compact.at("personalCenter").at("preferences") == full.at("personalCenter").at("preferences"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(compact.at("personalCenter").at("memories").size() == 1);。
    assert(compact.at("personalCenter").at("memories").size() == 1);
    // 【逐行批注】 调用 assert、array 完成本行所需操作：assert(compact.at("contextAvailability").at("recordIds") == Json::arr…。
    assert(compact.at("contextAvailability").at("recordIds") == Json::array({"other"}));
    // 【逐行批注】 调用 assert、array 完成本行所需操作：assert(compact.at("contextAvailability").at("memoryIds") == Json::arr…。
    assert(compact.at("contextAvailability").at("memoryIds") == Json::array({"foreign-memory"}));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(full.dump().find("OTHER-SCOPE-SECRET") == std::string::npos);。
    assert(full.dump().find("OTHER-SCOPE-SECRET") == std::string::npos);
    // 【逐行批注】 计算右侧结果并写入 unrelated，更新当前运行状态。
    auto unrelated = full;
    // 【逐行批注】 计算右侧结果并写入 unrelated["currentInputs"]，更新当前运行状态。
    unrelated["currentInputs"] = Json::array({{{"questionId", "current-question"}}});
    // 【逐行批注】 按循环规则重复处理：for (auto& record : unrelated["feedback"]) if (record.at("id") == "evidenc…。
    for (auto& record : unrelated["feedback"]) if (record.at("id") == "evidence") record["payload"]["questionId"] = "old-question";
    // 【逐行批注】 计算右侧结果并写入 unrelatedProjection，更新当前运行状态。
    auto unrelatedProjection = agentPromptContext(unrelated, false);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(unrelatedProjection["feedback"][0].contains("originalRecord"));。
    assert(unrelatedProjection["feedback"][0].contains("originalRecord"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!unrelatedProjection["feedback"][0]["payload"].contains("helpH…。
    assert(!unrelatedProjection["feedback"][0]["payload"].contains("helpHistory"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(unrelatedProjection["feedback"][0]["payload"]["questionSnapsho…。
    assert(unrelatedProjection["feedback"][0]["payload"]["questionSnapshot"] == evidence.at("questionSnapshot"));
    // 【逐行批注】 计算右侧结果并写入 unrelated["currentInputs"][0]["rePracticeOf"]，更新当前运行状态。
    unrelated["currentInputs"][0]["rePracticeOf"] = "old-question";
    // 【逐行批注】 计算右侧结果并写入 relatedProjection，更新当前运行状态。
    auto relatedProjection = agentPromptContext(unrelated, false);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(relatedProjection["currentInputs"] == unrelated["currentInputs…。
    assert(relatedProjection["currentInputs"] == unrelated["currentInputs"]);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(relatedProjection["feedback"][0].contains("originalRecord"));。
    assert(relatedProjection["feedback"][0].contains("originalRecord"));
    // 【逐行批注】 调用 efficiencySwitch 完成本行所需操作：efficiencySwitch("GANGYI_AGENT_CONTEXT_PROJECTION", "0");。
    efficiencySwitch("GANGYI_AGENT_CONTEXT_PROJECTION", "0");
    // 【逐行批注】 调用 assert、agentPromptContext 完成本行所需操作：assert(agentPromptContext(full, false) == full);。
    assert(agentPromptContext(full, false) == full);
    // 【逐行批注】 调用 efficiencySwitch 完成本行所需操作：efficiencySwitch("GANGYI_AGENT_CONTEXT_PROJECTION", "");。
    efficiencySwitch("GANGYI_AGENT_CONTEXT_PROJECTION", "");

    // 资料任务的每个补读索引均须落在原授权范围内，不能靠新增权限使入口可达。
    // 【逐行批注】 计算右侧结果并写入 researchContext，更新当前运行状态。
    auto researchContext = full; researchContext["event"]["type"] = "library_research";
    // 【逐行批注】 计算右侧结果并写入 auto researchProjection，更新当前运行状态。
    const auto researchProjection = agentPromptContext(researchContext, false);
    // 【逐行批注】 计算右侧结果并写入 Json researchTask，更新当前运行状态。
    const Json researchTask = {{"event", researchContext.at("event")}};
    // 【逐行批注】 调用 assert、agentToolAuthorized 完成本行所需操作：assert(!agentToolAuthorized("read_outline", researchTask));。
    assert(!agentToolAuthorized("read_outline", researchTask));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(researchProjection.at("contextAvailability").at("outlineTool")…。
    assert(researchProjection.at("contextAvailability").at("outlineTool") == "read_context");
    // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"historyTool", "fullContextTool", "outlineTool",…。
    for (const auto* field : {"historyTool", "fullContextTool", "outlineTool", "resourceTool"})
        // 【逐行批注】 调用 assert、agentToolAuthorized 完成本行所需操作：assert(agentToolAuthorized(researchProjection.at("contextAvailability…。
        assert(agentToolAuthorized(researchProjection.at("contextAvailability").at(field).get<std::string>(), researchTask));
    // 【逐行批注】 按循环规则重复处理：for (const auto& record : researchProjection.at("feedback"))。
    for (const auto& record : researchProjection.at("feedback"))
        // 【逐行批注】 调用 assert、agentToolAuthorized 完成本行所需操作：assert(agentToolAuthorized(record.at("originalRecord").at("tool").get…。
        assert(agentToolAuthorized(record.at("originalRecord").at("tool").get<std::string>(), researchTask));

    // 个人中心版本查询不读取正文；分别证明学习、资料、偏好、记忆、访问范围和数据库变更失效。
    // 【逐行批注】 计算右侧结果并写入 AgentContextReuse reuse; int builds，更新当前运行状态。
    AgentContextReuse reuse; int builds = 0;
    // 【逐行批注】 计算右侧结果并写入 auto read，更新当前运行状态。
    const auto read = [&](const AgentAccess& who) { return reuse.read(db, who, event, [&] { ++builds; return agentContext(db, who, event); }); };
    // 【逐行批注】 调用 assert、read 完成本行所需操作：assert(read(access) == read(access) && builds == 1);。
    assert(read(access) == read(access) && builds == 1);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"memory", "memory", Json{{"content", ""}, {…。
    assert(db.saveMyAIRecord({"memory", "memory", Json{{"content", ""}, {"deleted", true}}.dump(), "now", 0}, 1));
    // 【逐行批注】 计算右侧结果并写入 deleted，更新当前运行状态。
    auto deleted = read(access); assert(builds == 2 && deleted.dump().find("旧记忆") == std::string::npos);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"preferences", "preferences", Json{{"instru…。
    assert(db.saveMyAIRecord({"preferences", "preferences", Json{{"instructions", "新手动要求"}, {"autoMemory", false}}.dump(), "now", 0}, 1));
    // 【逐行批注】 调用 assert、read 完成本行所需操作：assert(read(access).at("personalCenter").at("preferences").at("instru…。
    assert(read(access).at("personalCenter").at("preferences").at("instructions") == "新手动要求" && builds == 3);
    // 【逐行批注】 调用 read、assert 完成本行所需操作：db.setProfileMeta("library-revision", "2"); read(access); assert(buil…。
    db.setProfileMeta("library-revision", "2"); read(access); assert(builds == 4);
    // 【逐行批注】 调用 read、assert 完成本行所需操作：db.markLearningDirty(); read(access); assert(builds == 5);。
    db.markLearningDirty(); read(access); assert(builds == 5);
    // 【逐行批注】 调用 read、assert 完成本行所需操作：read({access.scopeId, {"math"}}); assert(builds == 6);。
    read({access.scopeId, {"math"}}); assert(builds == 6);
    // 【逐行批注】 执行当前声明或表达式：Database other; other.open(":memory:"); other.migrate();。
    Database other; other.open(":memory:"); other.migrate();
    // 【逐行批注】 调用 assert 完成本行所需操作：reuse.read(other, access, event, [&] { ++builds; return Json{{"isolat…。
    reuse.read(other, access, event, [&] { ++builds; return Json{{"isolated", true}}; }); assert(builds == 7);
    // 【逐行批注】 调用 read、assert 完成本行所需操作：reuse.clear(); read(access); assert(builds == 8);。
    reuse.clear(); read(access); assert(builds == 8);
    // 【逐行批注】 调用 efficiencySwitch、read、assert 完成本行所需操作：efficiencySwitch("GANGYI_AGENT_CONTEXT_REUSE", "0"); read(access); re…。
    efficiencySwitch("GANGYI_AGENT_CONTEXT_REUSE", "0"); read(access); read(access); assert(builds == 10);
    // 【逐行批注】 调用 efficiencySwitch 完成本行所需操作：efficiencySwitch("GANGYI_AGENT_CONTEXT_REUSE", "");。
    efficiencySwitch("GANGYI_AGENT_CONTEXT_REUSE", "");
    // 【逐行批注】 调用 read、assert 完成本行所需操作：db.setProfileAssessed(777); read(access); assert(builds == 11);。
    db.setProfileAssessed(777); read(access); assert(builds == 11);

    // 另一连接在相同版本写入真实画像正文，不能依赖本 process 的 commit 清缓存。
    // 【逐行批注】 执行当前声明或表达式：Database profileWriter; profileWriter.open(path.string());。
    Database profileWriter; profileWriter.open(path.string());
    // 【逐行批注】 计算右侧结果并写入 int profileRevision，更新当前运行状态。
    const int profileRevision = db.profileRevision(), assessedRevision = db.profileAssessedRevision(), learningRevision = db.learningRevision();
    // 【逐行批注】 执行当前声明或表达式：profileWriter.setProfileMeta("ability-profile", Json{{"version", profileRevision}, {…。
    profileWriter.setProfileMeta("ability-profile", Json{{"version", profileRevision}, {"rationale", "原能力依据"}}.dump());
    // 【逐行批注】 计算右侧结果并写入 AgentContextReuse profileReuse; int profileB…，更新当前运行状态。
    AgentContextReuse profileReuse; int profileBuilds = 0;
    // 【逐行批注】 计算右侧结果并写入 auto readProfile，更新当前运行状态。
    const auto readProfile = [&] { return profileReuse.read(db, access, event, [&] { ++profileBuilds; return agentContext(db, access, event); }); };
    // 【逐行批注】 调用 assert、readProfile 完成本行所需操作：assert(readProfile().at("abilities").at("rationale") == "原能力依据" && pr…。
    assert(readProfile().at("abilities").at("rationale") == "原能力依据" && profileBuilds == 1);
    // 【逐行批注】 执行当前声明或表达式：profileWriter.setProfileMeta("ability-profile", Json{{"version", profileRevision}, {…。
    profileWriter.setProfileMeta("ability-profile", Json{{"version", profileRevision}, {"rationale", "同版本的新能力依据"}}.dump());
    // 【逐行批注】 执行当前声明或表达式：profileWriter.setProfileAssessed(assessedRevision);。
    profileWriter.setProfileAssessed(assessedRevision);
    // 【逐行批注】 调用 assert、readProfile 完成本行所需操作：assert(readProfile().at("abilities").at("rationale") == "同版本的新能力依据" &…。
    assert(readProfile().at("abilities").at("rationale") == "同版本的新能力依据" && profileBuilds == 2);
    // 【逐行批注】 调用 readProfile、assert 完成本行所需操作：readProfile(); assert(profileBuilds == 2);。
    readProfile(); assert(profileBuilds == 2);
    // 【逐行批注】 计算右侧结果并写入 SubjectMastery subject; subject.subject，更新当前运行状态。
    SubjectMastery subject; subject.subject = "math"; subject.score = 40; subject.rationale = "原学科依据"; subject.recommendation = "原建议";
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(profileWriter.upsert(subject));。
    assert(profileWriter.upsert(subject));
    // 【逐行批注】 调用 assert、readProfile 完成本行所需操作：assert(readProfile().at("subjects")[0].at("score") == 40 && profileBu…。
    assert(readProfile().at("subjects")[0].at("score") == 40 && profileBuilds == 3);
    // 【逐行批注】 计算右侧结果并写入 subject.score，更新当前运行状态。
    subject.score = 60; subject.rationale = "同版本的新学科依据"; subject.recommendation = "同版本的新建议";
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(profileWriter.upsert(subject));。
    assert(profileWriter.upsert(subject));
    // 【逐行批注】 计算右侧结果并写入 auto updatedSubjects，更新当前运行状态。
    const auto updatedSubjects = readProfile().at("subjects");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(updatedSubjects[0].at("score") == 60 && updatedSubjects[0].at(…。
    assert(updatedSubjects[0].at("score") == 60 && updatedSubjects[0].at("rationale") == subject.rationale &&
        // 【逐行批注】 执行当前声明或表达式：updatedSubjects[0].at("recommendation") == subject.recommendation && profileBuilds =…。
        updatedSubjects[0].at("recommendation") == subject.recommendation && profileBuilds == 4);
    // 【逐行批注】 调用 readProfile、assert 完成本行所需操作：readProfile(); assert(profileBuilds == 4);。
    readProfile(); assert(profileBuilds == 4);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileRevision() == profileRevision && db.profileAssessedR…。
    assert(db.profileRevision() == profileRevision && db.profileAssessedRevision() == assessedRevision && db.learningRevision() == learningRevision);

    // 真实主控：未加载详细说明的工具可发现、补读、调用；完整原件按ID和跨课程回查。
    // 【逐行批注】 计算右侧结果并写入 std::string completeDescription，更新当前运行状态。
    const std::string completeDescription = "读取特殊证据。参数 evidenceId 必须逐字使用原件ID；返回原始帮助与引用。";
    // 【逐行批注】 计算右侧结果并写入 int calls，更新当前运行状态。
    int calls = 0, executions = 0; std::size_t stagedBytes = 0, fullBytes = 0;
    // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
    const auto task = agentSubmit(db, access, event);
    // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
    LearningAgent agent(path.string(), [&](const ChatOptions& options, const auto& sink) {
        // 【逐行批注】 执行当前声明或表达式：++calls;。
        ++calls;
        // 【逐行批注】 判断条件 (calls == 1) {，成立时执行对应分支。
        if (calls == 1) {
            // 【逐行批注】 计算右侧结果并写入 auto& system，更新当前运行状态。
            const auto& system = options.messages.front().content;
            // 【逐行批注】 计算右侧结果并写入 auto position，更新当前运行状态。
            const auto position = system.find("可用工具：");
            // 【逐行批注】 计算右侧结果并写入 auto catalog，更新当前运行状态。
            const auto catalog = Json::parse(system.substr(position + std::string("可用工具：").size()));
            // 【逐行批注】 计算右侧结果并写入 bool found，更新当前运行状态。
            bool found = false;
            // 【逐行批注】 按循环规则重复处理：for (const auto& tool : catalog) if (tool.at("name") == "read_special_evid…。
            for (const auto& tool : catalog) if (tool.at("name") == "read_special_evidence") {
                // 【逐行批注】 计算右侧结果并写入 found，更新当前运行状态。
                found = true; assert(tool.at("description") == "读取特殊证据。" && tool.at("detailsTool") == "describe_tools");
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(found);。
            assert(found);
            // 【逐行批注】 计算右侧结果并写入 stagedBytes，更新当前运行状态。
            stagedBytes = system.size();
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", ""}, {"actions", {。
            return output({{"message", ""}, {"actions", {
                // 【逐行批注】 执行当前声明或表达式：{{"id", "describe"}, {"tool", "describe_tools"}, {"args", {{"names", {"read_special_…。
                {{"id", "describe"}, {"tool", "describe_tools"}, {"args", {{"names", {"read_special_evidence"}}}}},
                // 【逐行批注】 执行当前声明或表达式：{{"id", "history"}, {"tool", "read_history"}, {"args", {{"ids", {"evidence", "privat…。
                {{"id", "history"}, {"tool", "read_history"}, {"args", {{"ids", {"evidence", "private"}}}}},
                // 【逐行批注】 执行当前声明或表达式：{{"id", "context"}, {"tool", "read_context"}, {"args", {{"courseId", "science"}}}}。
                {{"id", "context"}, {"tool", "read_context"}, {"args", {{"courseId", "science"}}}}
            // 【逐行批注】 执行当前声明或表达式：}}, {"state", "continue"}}, sink);。
            }}, {"state", "continue"}}, sink);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
        const auto raw = Json::parse(db.getClassroomActivity(task.at("id"))->payload);
        // 【逐行批注】 计算右侧结果并写入 auto& receipts，更新当前运行状态。
        const auto& receipts = raw.at("completedActions");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(receipts.at("describe").at("result").at("tools")[0].at("descri…。
        assert(receipts.at("describe").at("result").at("tools")[0].at("description") == completeDescription);
        // 【逐行批注】 计算右侧结果并写入 auto records，更新当前运行状态。
        const auto records = receipts.at("history").at("result").at("records");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(records.size() == 1 && records[0].at("id") == "evidence" && re…。
        assert(records.size() == 1 && records[0].at("id") == "evidence" && records[0].at("payload") == evidence);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(receipts.at("context").at("result").at("courses").size() == 1);。
        assert(receipts.at("context").at("result").at("courses").size() == 1);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(receipts.at("context").at("result").at("courses")[0].at("id") …。
        assert(receipts.at("context").at("result").at("courses")[0].at("id") == "science");
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "已核对原题和帮助。"}, {"actions", {{{"id", "specia…。
        return output({{"message", "已核对原题和帮助。"}, {"actions", {{{"id", "special"}, {"tool", "read_special_evidence"}, {"args", {{"evidenceId", "evidence"}}}}}}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("read_special_evidence", {completeDescription, true, [&](Database…。
    agent.registerTool("read_special_evidence", {completeDescription, true, [&](Database&, const Json& args, const Json&, const AIResult&) {
        // 【逐行批注】 调用 assert 完成本行所需操作：++executions; assert(args.at("evidenceId") == "evidence"); return Pre…。
        ++executions; assert(args.at("evidenceId") == "evidence"); return PreparedAgentTool{{{"evidence", evidence}}, {}};
    // 【逐行批注】 执行当前声明或表达式：}});。
    }});
    // 【逐行批注】 执行当前声明或表达式：agent.process(db, task.at("id"));。
    agent.process(db, task.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, task.at("id")).at("status") == "waiting_…。
    assert(agentView(db, access, task.at("id")).at("status") == "waiting_student" && calls == 2 && executions == 1);

    // 所有阶段保持同一授权集合，目录回退只改变说明加载；后台及资料权限不扩大。
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, AgentTool> tools;。
    std::map<std::string, AgentTool> tools;
    // 【逐行批注】 按循环规则重复处理：for (const auto* name : {"create_course", "append_section", "evaluate_answ…。
    for (const auto* name : {"create_course", "append_section", "evaluate_answer", "classify_input", "read_context", "read_library", "search_resources", "read_resource", "select_resources", "adjust_schedule", "describe_tools"})
        // 【逐行批注】 计算右侧结果并写入 tools[name]，更新当前运行状态。
        tools[name] = {std::string("用途。完整参数：") + std::string(1000, 'x'), true, {}};
    // 【逐行批注】 按循环规则重复处理：for (const auto* type : {"assistant_chat", "plan_course", "prepare_next", …。
    for (const auto* type : {"assistant_chat", "plan_course", "prepare_next", "question_answer", "chat", "library_research", "startup"}) {
        // 【逐行批注】 计算右侧结果并写入 Json stageTask，更新当前运行状态。
        Json stageTask = {{"event", {{"type", type}, {"lessonId", std::string(type) == "chat" ? "lesson" : ""}}}};
        // 【逐行批注】 计算右侧结果并写入 auto staged，更新当前运行状态。
        const auto staged = agentToolCatalog(tools, stageTask);
        // 【逐行批注】 调用 efficiencySwitch 完成本行所需操作：efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "0");。
        efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "0");
        // 【逐行批注】 计算右侧结果并写入 auto legacy，更新当前运行状态。
        const auto legacy = agentToolCatalog(tools, stageTask);
        // 【逐行批注】 调用 efficiencySwitch 完成本行所需操作：efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "");。
        efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(staged.size() == legacy.size());。
        assert(staged.size() == legacy.size());
        // 【逐行批注】 按循环规则重复处理：for (std::size_t i = 0; i < staged.size(); ++i) assert(staged[i].at("name"…。
        for (std::size_t i = 0; i < staged.size(); ++i) assert(staged[i].at("name") == legacy[i].at("name"));
        // 【逐行批注】 调用 assert、agentToolAuthorized 完成本行所需操作：assert(!agentToolAuthorized("adjust_schedule", stageTask));。
        assert(!agentToolAuthorized("adjust_schedule", stageTask));
        // 【逐行批注】 判断条件 (std::string(type) == "library_research") assert(!agentToolAutho…，成立时执行对应分支。
        if (std::string(type) == "library_research") assert(!agentToolAuthorized("create_course", stageTask));
        // 【逐行批注】 判断条件 (std::string(type) == "startup") assert(!agentToolAuthorized("re…，成立时执行对应分支。
        if (std::string(type) == "startup") assert(!agentToolAuthorized("read_resource", stageTask));
    // 【逐行批注】 结束当前代码块。
    }
    // 恢复不持久化缓存，重建主控仍读取当前版本；越权说明和越权执行均拒绝。
    // 【逐行批注】 按循环规则重复处理：for (const auto* forbidden : {"describe_tools", "read_special_evidence"}) {。
    for (const auto* forbidden : {"describe_tools", "read_special_evidence"}) {
        // 【逐行批注】 计算右侧结果并写入 deniedTask，更新当前运行状态。
        auto deniedTask = agentSubmit(db, access, {{"type", "library_research"}, {"courseId", "math"}, {"requestId", std::string("deny-") + forbidden}});
        // 【逐行批注】 定义函数 denied，执行该函数声明的职责。
        LearningAgent denied(path.string(), [&](const auto&, const auto& sink) {
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", ""}, {"actions", {{{"id", "forbidden"}, {"…。
            return output({{"message", ""}, {"actions", {{{"id", "forbidden"}, {"tool", forbidden}, {"args", {{"names", {"create_course"}}}}}}}, {"state", "continue"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 执行当前声明或表达式：denied.process(db, deniedTask.at("id"));。
        denied.process(db, deniedTask.at("id"));
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, deniedTask.at("id")).at("status") == "fa…。
        assert(agentView(db, access, deniedTask.at("id")).at("status") == "failed");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 efficiencySwitch 完成本行所需操作：efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "0");。
    efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "0");
    // 【逐行批注】 计算右侧结果并写入 fallbackTask，更新当前运行状态。
    auto fallbackTask = agentSubmit(db, access, {{"type", "chat"}, {"text", "回退检查"}, {"courseId", "math"}, {"requestId", "fallback"}});
    // 【逐行批注】 定义函数 fallback，执行该函数声明的职责。
    LearningAgent fallback(path.string(), [&](const ChatOptions& options, const auto& sink) {
        // 【逐行批注】 计算右侧结果并写入 fullBytes，更新当前运行状态。
        fullBytes = options.messages.front().content.size();
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.messages.front().content.find(completeDescription) != …。
        assert(options.messages.front().content.find(completeDescription) != std::string::npos);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.messages[1].content.find("旧记忆") == std::string::npos);。
        assert(options.messages[1].content.find("旧记忆") == std::string::npos);
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "回退仍可继续"}, {"actions", Json::array()}, {"s…。
        return output({{"message", "回退仍可继续"}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册可由 AI 主控调用的工具：fallback.registerTool("read_special_evidence", {completeDescription, true, [](Databa…。
    fallback.registerTool("read_special_evidence", {completeDescription, true, [](Database&, const Json&, const Json&, const AIResult&) { return PreparedAgentTool{}; }});
    // 【逐行批注】 执行当前声明或表达式：fallback.process(db, fallbackTask.at("id"));。
    fallback.process(db, fallbackTask.at("id"));
    // 【逐行批注】 调用 efficiencySwitch 完成本行所需操作：efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "");。
    efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "");
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(agentView(db, access, fallbackTask.at("id")).at("status") == "…。
    assert(agentView(db, access, fallbackTask.at("id")).at("status") == "waiting_student");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(stagedBytes < fullBytes);。
    assert(stagedBytes < fullBytes);
    // 【逐行批注】 执行当前声明或表达式：std::cout << "阶段目录离线字节：" << fullBytes << " -> " << stagedBytes << "；上下文字节：" << full.…。
    std::cout << "阶段目录离线字节：" << fullBytes << " -> " << stagedBytes << "；上下文字节：" << full.dump().size() << " -> " << compact.dump().size() << "（不代表真实Token）\n";
// 【逐行批注】 结束当前代码块。
}

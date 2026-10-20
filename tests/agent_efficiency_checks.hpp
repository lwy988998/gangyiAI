#pragma once
#include "agent_efficiency.hpp"
#include "agent_prompt.hpp"
#include "my_ai_context.hpp"

inline void efficiencySwitch(const char* key, const char* value) {
#ifdef _WIN32
    _putenv_s(key, value);
#else
    if (*value) setenv(key, value, 1); else unsetenv(key);
#endif
}

inline void runAgentEfficiencyChecks(const std::filesystem::path& path) {
    using namespace gangyi;
    Database db; db.open(path.string()); db.migrate();
    const AgentAccess access{"efficiency", {"math", "science"}};
    for (const auto* id : {"math", "science"}) assert(db.insert(Course{id, access.scopeId, {}, id, "deep", id, {}, "ai", "active", "2026-10-19", "2026-10-19"}));
    const Json event = {{"type", "chat"}, {"courseId", "math"}, {"requestId", "projection"}, {"text", "按原题补讲"}};
    const Json evidence = {{"scopeId", access.scopeId}, {"text", "学生原始回答"}, {"questionSnapshot", {{"question", "1/2+1/3"}, {"rubric", "原题私有标准"}}},
        {"helpHistory", {{{"type", "hint"}, {"text", "原始小提示"}}}}, {"relatedLearning", {"普通讲解"}},
        {"citations", {{{"sourceId", "source"}, {"snapshotId", "snapshot"}, {"segmentId", "p1"}}}}, {"demonstratedQuestions", {"示范原题"}}};
    assert(db.insert(LearningInteraction{"evidence", "chat-user", evidence.dump(), "2026-10-19", "math", "conversation", {}}));
    assert(db.insert(LearningInteraction{"other", "chat-user", Json{{"text", std::string(20000, 'x')}}.dump(), "2026-10-19", "science", "conversation", {}}));
    assert(db.insert(LearningInteraction{"private", "chat-user", Json{{"scopeId", "other-owner"}, {"text", "OTHER-SCOPE-SECRET"}}.dump(), "2026-10-19", "math", "conversation", {}}));
    assert(db.saveMyAIRecord({"preferences", "preferences", Json{{"instructions", "手动要求优先"}, {"autoMemory", false}}.dump(), "now", 0}, 0));
    assert(db.saveMyAIRecord({"memory", "memory", Json{{"content", "旧记忆"}, {"source", "manual"}}.dump(), "now", 0}, 0));
    assert(db.saveMyAIRecord({"foreign-memory", "memory", Json{{"content", "另一课程的推测"}, {"source", "inferred"}, {"courseId", "science"}}.dump(), "now", 0}, 0));
    auto full = agentContext(db, access, event);
    auto compact = agentPromptContext(full, false);
    assert(compact.at("feedback").size() == 1);
    assert(compact.at("feedback")[0].at("payload").at("questionSnapshot") == evidence.at("questionSnapshot"));
    assert(compact.at("feedback")[0].at("payload").at("citations") == evidence.at("citations"));
    assert(compact.at("feedback")[0].at("originalRecord").at("ids") == Json::array({"evidence"}));
    assert(compact.at("personalCenter").at("preferences") == full.at("personalCenter").at("preferences"));
    assert(compact.at("personalCenter").at("memories").size() == 1);
    assert(compact.at("contextAvailability").at("recordIds") == Json::array({"other"}));
    assert(compact.at("contextAvailability").at("memoryIds") == Json::array({"foreign-memory"}));
    assert(full.dump().find("OTHER-SCOPE-SECRET") == std::string::npos);
    auto unrelated = full;
    unrelated["currentInputs"] = Json::array({{{"questionId", "current-question"}}});
    for (auto& record : unrelated["feedback"]) if (record.at("id") == "evidence") record["payload"]["questionId"] = "old-question";
    auto unrelatedProjection = agentPromptContext(unrelated, false);
    assert(unrelatedProjection["feedback"][0].contains("originalRecord"));
    assert(!unrelatedProjection["feedback"][0]["payload"].contains("helpHistory"));
    assert(unrelatedProjection["feedback"][0]["payload"]["questionSnapshot"] == evidence.at("questionSnapshot"));
    unrelated["currentInputs"][0]["rePracticeOf"] = "old-question";
    auto relatedProjection = agentPromptContext(unrelated, false);
    assert(relatedProjection["currentInputs"] == unrelated["currentInputs"]);
    assert(relatedProjection["feedback"][0].contains("originalRecord"));
    efficiencySwitch("GANGYI_AGENT_CONTEXT_PROJECTION", "0");
    assert(agentPromptContext(full, false) == full);
    efficiencySwitch("GANGYI_AGENT_CONTEXT_PROJECTION", "");

    // 资料任务的每个补读索引均须落在原授权范围内，不能靠新增权限使入口可达。
    auto researchContext = full; researchContext["event"]["type"] = "library_research";
    const auto researchProjection = agentPromptContext(researchContext, false);
    const Json researchTask = {{"event", researchContext.at("event")}};
    assert(!agentToolAuthorized("read_outline", researchTask));
    assert(researchProjection.at("contextAvailability").at("outlineTool") == "read_context");
    for (const auto* field : {"historyTool", "fullContextTool", "outlineTool", "resourceTool"})
        assert(agentToolAuthorized(researchProjection.at("contextAvailability").at(field).get<std::string>(), researchTask));
    for (const auto& record : researchProjection.at("feedback"))
        assert(agentToolAuthorized(record.at("originalRecord").at("tool").get<std::string>(), researchTask));

    // 个人中心版本查询不读取正文；分别证明学习、资料、偏好、记忆、访问范围和数据库变更失效。
    AgentContextReuse reuse; int builds = 0;
    const auto read = [&](const AgentAccess& who) { return reuse.read(db, who, event, [&] { ++builds; return agentContext(db, who, event); }); };
    assert(read(access) == read(access) && builds == 1);
    assert(db.saveMyAIRecord({"memory", "memory", Json{{"content", ""}, {"deleted", true}}.dump(), "now", 0}, 1));
    auto deleted = read(access); assert(builds == 2 && deleted.dump().find("旧记忆") == std::string::npos);
    assert(db.saveMyAIRecord({"preferences", "preferences", Json{{"instructions", "新手动要求"}, {"autoMemory", false}}.dump(), "now", 0}, 1));
    assert(read(access).at("personalCenter").at("preferences").at("instructions") == "新手动要求" && builds == 3);
    db.setProfileMeta("library-revision", "2"); read(access); assert(builds == 4);
    db.markLearningDirty(); read(access); assert(builds == 5);
    read({access.scopeId, {"math"}}); assert(builds == 6);
    Database other; other.open(":memory:"); other.migrate();
    reuse.read(other, access, event, [&] { ++builds; return Json{{"isolated", true}}; }); assert(builds == 7);
    reuse.clear(); read(access); assert(builds == 8);
    efficiencySwitch("GANGYI_AGENT_CONTEXT_REUSE", "0"); read(access); read(access); assert(builds == 10);
    efficiencySwitch("GANGYI_AGENT_CONTEXT_REUSE", "");
    db.setProfileAssessed(777); read(access); assert(builds == 11);

    // 另一连接在相同版本写入真实画像正文，不能依赖本 process 的 commit 清缓存。
    Database profileWriter; profileWriter.open(path.string());
    const int profileRevision = db.profileRevision(), assessedRevision = db.profileAssessedRevision(), learningRevision = db.learningRevision();
    profileWriter.setProfileMeta("ability-profile", Json{{"version", profileRevision}, {"rationale", "原能力依据"}}.dump());
    AgentContextReuse profileReuse; int profileBuilds = 0;
    const auto readProfile = [&] { return profileReuse.read(db, access, event, [&] { ++profileBuilds; return agentContext(db, access, event); }); };
    assert(readProfile().at("abilities").at("rationale") == "原能力依据" && profileBuilds == 1);
    profileWriter.setProfileMeta("ability-profile", Json{{"version", profileRevision}, {"rationale", "同版本的新能力依据"}}.dump());
    profileWriter.setProfileAssessed(assessedRevision);
    assert(readProfile().at("abilities").at("rationale") == "同版本的新能力依据" && profileBuilds == 2);
    readProfile(); assert(profileBuilds == 2);
    SubjectMastery subject; subject.subject = "math"; subject.score = 40; subject.rationale = "原学科依据"; subject.recommendation = "原建议";
    assert(profileWriter.upsert(subject));
    assert(readProfile().at("subjects")[0].at("score") == 40 && profileBuilds == 3);
    subject.score = 60; subject.rationale = "同版本的新学科依据"; subject.recommendation = "同版本的新建议";
    assert(profileWriter.upsert(subject));
    const auto updatedSubjects = readProfile().at("subjects");
    assert(updatedSubjects[0].at("score") == 60 && updatedSubjects[0].at("rationale") == subject.rationale &&
        updatedSubjects[0].at("recommendation") == subject.recommendation && profileBuilds == 4);
    readProfile(); assert(profileBuilds == 4);
    assert(db.profileRevision() == profileRevision && db.profileAssessedRevision() == assessedRevision && db.learningRevision() == learningRevision);

    // 真实主控：未加载详细说明的工具可发现、补读、调用；完整原件按ID和跨课程回查。
    const std::string completeDescription = "读取特殊证据。参数 evidenceId 必须逐字使用原件ID；返回原始帮助与引用。";
    int calls = 0, executions = 0; std::size_t stagedBytes = 0, fullBytes = 0;
    const auto task = agentSubmit(db, access, event);
    LearningAgent agent(path.string(), [&](const ChatOptions& options, const auto& sink) {
        ++calls;
        if (calls == 1) {
            const auto& system = options.messages.front().content;
            const auto position = system.find("可用工具：");
            const auto catalog = Json::parse(system.substr(position + std::string("可用工具：").size()));
            bool found = false;
            for (const auto& tool : catalog) if (tool.at("name") == "read_special_evidence") {
                found = true; assert(tool.at("description") == "读取特殊证据。" && tool.at("detailsTool") == "describe_tools");
            }
            assert(found);
            stagedBytes = system.size();
            return output({{"message", ""}, {"actions", {
                {{"id", "describe"}, {"tool", "describe_tools"}, {"args", {{"names", {"read_special_evidence"}}}}},
                {{"id", "history"}, {"tool", "read_history"}, {"args", {{"ids", {"evidence", "private"}}}}},
                {{"id", "context"}, {"tool", "read_context"}, {"args", {{"courseId", "science"}}}}
            }}, {"state", "continue"}}, sink);
        }
        const auto raw = Json::parse(db.getClassroomActivity(task.at("id"))->payload);
        const auto& receipts = raw.at("completedActions");
        assert(receipts.at("describe").at("result").at("tools")[0].at("description") == completeDescription);
        const auto records = receipts.at("history").at("result").at("records");
        assert(records.size() == 1 && records[0].at("id") == "evidence" && records[0].at("payload") == evidence);
        assert(receipts.at("context").at("result").at("courses").size() == 1);
        assert(receipts.at("context").at("result").at("courses")[0].at("id") == "science");
        return output({{"message", "已核对原题和帮助。"}, {"actions", {{{"id", "special"}, {"tool", "read_special_evidence"}, {"args", {{"evidenceId", "evidence"}}}}}}, {"state", "waiting_student"}}, sink);
    });
    agent.registerTool("read_special_evidence", {completeDescription, true, [&](Database&, const Json& args, const Json&, const AIResult&) {
        ++executions; assert(args.at("evidenceId") == "evidence"); return PreparedAgentTool{{{"evidence", evidence}}, {}};
    }});
    agent.process(db, task.at("id"));
    assert(agentView(db, access, task.at("id")).at("status") == "waiting_student" && calls == 2 && executions == 1);

    // 所有阶段保持同一授权集合，目录回退只改变说明加载；后台及资料权限不扩大。
    std::map<std::string, AgentTool> tools;
    for (const auto* name : {"create_course", "append_section", "evaluate_answer", "classify_input", "read_context", "read_library", "search_resources", "read_resource", "select_resources", "adjust_schedule", "describe_tools"})
        tools[name] = {std::string("用途。完整参数：") + std::string(1000, 'x'), true, {}};
    for (const auto* type : {"assistant_chat", "plan_course", "prepare_next", "question_answer", "chat", "library_research", "startup"}) {
        Json stageTask = {{"event", {{"type", type}, {"lessonId", std::string(type) == "chat" ? "lesson" : ""}}}};
        const auto staged = agentToolCatalog(tools, stageTask);
        efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "0");
        const auto legacy = agentToolCatalog(tools, stageTask);
        efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "");
        assert(staged.size() == legacy.size());
        for (std::size_t i = 0; i < staged.size(); ++i) assert(staged[i].at("name") == legacy[i].at("name"));
        assert(!agentToolAuthorized("adjust_schedule", stageTask));
        if (std::string(type) == "library_research") assert(!agentToolAuthorized("create_course", stageTask));
        if (std::string(type) == "startup") assert(!agentToolAuthorized("read_resource", stageTask));
    }
    // 恢复不持久化缓存，重建主控仍读取当前版本；越权说明和越权执行均拒绝。
    for (const auto* forbidden : {"describe_tools", "read_special_evidence"}) {
        auto deniedTask = agentSubmit(db, access, {{"type", "library_research"}, {"courseId", "math"}, {"requestId", std::string("deny-") + forbidden}});
        LearningAgent denied(path.string(), [&](const auto&, const auto& sink) {
            return output({{"message", ""}, {"actions", {{{"id", "forbidden"}, {"tool", forbidden}, {"args", {{"names", {"create_course"}}}}}}}, {"state", "continue"}}, sink);
        });
        denied.process(db, deniedTask.at("id"));
        assert(agentView(db, access, deniedTask.at("id")).at("status") == "failed");
    }
    efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "0");
    auto fallbackTask = agentSubmit(db, access, {{"type", "chat"}, {"text", "回退检查"}, {"courseId", "math"}, {"requestId", "fallback"}});
    LearningAgent fallback(path.string(), [&](const ChatOptions& options, const auto& sink) {
        fullBytes = options.messages.front().content.size();
        assert(options.messages.front().content.find(completeDescription) != std::string::npos);
        assert(options.messages[1].content.find("旧记忆") == std::string::npos);
        return output({{"message", "回退仍可继续"}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
    });
    fallback.registerTool("read_special_evidence", {completeDescription, true, [](Database&, const Json&, const Json&, const AIResult&) { return PreparedAgentTool{}; }});
    fallback.process(db, fallbackTask.at("id"));
    efficiencySwitch("GANGYI_AGENT_STAGE_TOOLS", "");
    assert(agentView(db, access, fallbackTask.at("id")).at("status") == "waiting_student");
    assert(stagedBytes < fullBytes);
    std::cout << "阶段目录离线字节：" << fullBytes << " -> " << stagedBytes << "；上下文字节：" << full.dump().size() << " -> " << compact.dump().size() << "（不代表真实Token）\n";
}

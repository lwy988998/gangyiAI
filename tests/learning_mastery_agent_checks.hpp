#pragma once
#include "learning_library.hpp"

// 经正式主控读取跨课时证据；故意混入同题复做、暂存、错档和错知识点。
inline void runTopicEvidenceChecks(const std::filesystem::path& path) {
    const gangyi::AgentAccess access{"topic-owner", {"math", "other-course"}};
    const std::string when = "2026-11-19T00:00:00Z";
    const std::string topic = "sets-topic";
    Json expected;
    {
        gangyi::Database db;
        db.open(path.string()); db.migrate();
        for (const auto* course : {"math", "other-course"})
            assert(db.insert(gangyi::Course{course, access.scopeId, {}, "集合关系", "deep", "数学", {}, "ai", "active", when, when}));
        const Json outline = {{"courseStructure", {{{"stage", "集合"}, {"topics", {{{"id", topic}, {"title", "集合关系"}}}}}}}};
        assert(db.insert(gangyi::CourseSnapshot{"topic-outline", "math", 1, outline.dump(), when}));

        const auto seed = [&](const std::string& id, const std::string& course, const std::string& scope,
                              const std::string& savedTopic, const Json& question, bool assisted, bool credible) {
            const auto questionId = id + "-local-question";
            const Json section = {{"id", "q1"}, {"kind", "question"}, {"title", "集合练习"},
                {"questionId", questionId}, {"version", 1}, {"question", question}};
            const Json lesson = {{"id", id}, {"courseId", course}, {"scopeId", scope}, {"topicId", savedTopic},
                {"title", "集合练习"}, {"purpose", "结合已有练习继续学习"}, {"status", "ready"}, {"entered", true},
                {"version", 1}, {"sections", Json::array({section})}};
            assert(db.upsert(gangyi::ClassroomActivity{id, course, "agent-lesson", lesson.dump(), when, 0, 0}));
            const std::string answer = credible ? "按照包含关系判断" : "我还是分不清交集和子集";
            const Json input = {{"type", "question_answer"}, {"action", "answer"}, {"scopeId", scope}, {"lessonId", id},
                {"sectionId", "q1"}, {"requestId", id + "-request"}, {"topicId", savedTopic}, {"questionId", questionId},
                {"questionSnapshot", question}, {"text", answer}};
            assert(db.insert(gangyi::LearningInteraction{id + "-input", "chat-user", input.dump(), when, course, id, {}}));
            const Json evaluation = {{"interactionId", id + "-input"}, {"source", "ai"}, {"status", "valid"}, {"response", answer},
                {"questionId", questionId}, {"questionSnapshot", question}, {"credible", credible},
                {"score", credible ? Json(90) : Json()}, {"assisted", assisted}, {"helpMode", assisted ? "assisted" : "independent"},
                {"helpHistory", assisted ? Json::array({{{"helpType", "hint"}}}) : Json::array()},
                {"diagnosis", {{"status", credible ? "supported" : "uncertain"}, {"needsClarification", !credible},
                               {"summary", "先区分包含与共有"}, {"evidence", Json::array({answer})}}}};
            assert(db.insert(gangyi::LearningInteraction{id + "-evaluation", "practice", evaluation.dump(), when, course, id, {}}));
            return evaluation;
        };
        const Json first = {{"question", "A 是 B 的子集吗？"}, {"options", {"是", "否"}}, {"expectedAnswer", "PRIVATE-TOPIC"}};
        const Json repeat = {{"question", " A  是 B 的子集吗？ "}, {"options", {"否", "是"}}, {"expectedAnswer", "PRIVATE-REPEAT"}};
        const Json second = {{"question", "A 与 B 共有的元素有哪些？"}, {"expectedAnswer", "PRIVATE-EXERCISE"}};
        auto correction = seed("sets-first", "math", access.scopeId, topic, first, true, true);
        seed("sets-repeat", "math", access.scopeId, topic, repeat, false, true);
        seed("sets-second", "math", access.scopeId, topic, second, false, true);
        seed("sets-uncertain", "math", access.scopeId, topic, second, false, false);
        seed("other-topic", "math", access.scopeId, "different-topic", first, false, true);
        seed("foreign-owner", "math", "foreign-scope", topic, first, false, true);
        seed("different-course", "other-course", access.scopeId, "sets-b-topic", repeat, false, true);
        seed("sets-b-supplement", "other-course", access.scopeId, "sets-b-topic",
             {{"question", "若 A={1,2}，B={2,3}，A∪B 是什么？"}}, false, true);
        seed("sets-b-foreign-owner", "other-course", "foreign-scope", "sets-b-topic", first, false, true);
        assert(db.upsert(gangyi::ClassroomActivity{"staged-grade", "math", "agent-evaluation", correction.dump(), when, 0, 0}));
        correction["score"] = 72;
        assert(db.insert(gangyi::LearningInteraction{"latest-correction", "practice", correction.dump(), when, "math", "sets-first", {}}));

        expected = gangyi::agentTopicEvidence(db, access, "sets-uncertain");
        assert(expected == gangyi::agentLessonView(db, access, "sets-uncertain").at("topicEvidence"));
        assert(expected.at("lessonCount") == 4 && expected.at("evaluationCount") == 4);
        assert(expected.at("independentFirstAttemptCount") == 1 && expected.at("assistedQuestionCount") == 1);
        assert(expected.at("distinctSupportedQuestionCount") == 2);
        const auto& recent = expected.at("recentAttempts");
        assert(recent[0].at("evaluationId") == "latest-correction" && recent[0].at("score") == 72);
        assert(recent[0].at("questionKey") == recent[1].at("questionKey"));
        assert(recent[1].at("repeat") == true && recent[1].at("independentFirstAttempt") == false);
        assert(recent[3].at("needsClarification") == true && !recent[3].contains("score"));
        for (const auto* privateText : {"PRIVATE", "different-course", "foreign-owner", "other-topic", "staged-grade", "我还是分不清"})
            assert(expected.dump().find(privateText) == std::string::npos);

        auto oldRow = db.getClassroomActivity("sets-first").value();
        auto oldLesson = Json::parse(oldRow.payload);
        oldLesson.at("sections").push_back({{"id", "shown-example"}, {"kind", "explanation"},
            {"title", "已展示的同题示范"}, {"body", "先确认共有元素"}, {"version", 1},
            {"demonstratedQuestions", Json::array({second})}});
        oldRow.payload = oldLesson.dump();
        assert(db.upsert(oldRow));
        db.setProfileMeta("agent-section-exposure:sets-first", Json{{"shown-example", 1}}.dump());
        db.setProfileMeta("agent-section-display:sets-first", Json{{"shown-example", {{"version", 1}, {"complete", true}}}}.dump());
        const auto newAnswer = gangyi::agentSubmit(db, access, {{"type", "question_answer"}, {"courseId", "math"},
            {"lessonId", "sets-second"}, {"sectionId", "q1"}, {"sectionVersion", 1},
            {"requestId", "after-exposed-example"}, {"text", "共有元素有两个"}});
        bool boundExample = false;
        for (const auto& row : db.listInteractions()) if (row.kind == "chat-user") {
            const auto input = Json::parse(row.payload);
            if (input.value("requestId", "") != "after-exposed-example") continue;
            assert(input.at("topicId") == topic && input.at("assisted") == true);
            assert(input.at("helpHistory").at(0).at("sourceLessonId") == "sets-first");
            assert(input.at("helpHistory").at(0).at("helpType") == "worked_example");
            boundExample = true;
        }
        assert(boundExample);
        gangyi::agentControl(db, access, {{"command", "cancel"}, {"taskId", newAnswer.at("id")}});

        const auto task = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"courseId", "math"},
            {"lessonId", "sets-uncertain"}, {"requestId", "topic-context-check"}, {"text", "请结合我上次的练习复习集合"}});
        bool received = false;
        gangyi::LearningAgent agent(path.string(), [&](const auto& options, const auto& sink) {
            for (const auto& message : options.messages) {
                const auto context = Json::parse(message.content, nullptr, false);
                if (context.is_object() && context.contains("lessonContext")) {
                    assert(context.at("lessonContext").at("topicEvidence") == expected);
                    received = true;
                }
            }
            return output({{"message", "先检查你这次提到的交集与子集区别。"}, {"state", "waiting_student"},
                {"actions", Json::array()}}, sink);
        });
        gangyi::registerAgentLessonTools(agent);
        agent.process(db, task.at("id"));
        assert(received && gangyi::agentView(db, access, task.at("id")).at("status") == "waiting_student");

        const auto prepare = gangyi::agentSubmit(db, access, {{"type", "prepare_next"}, {"courseId", "math"},
            {"topicId", topic}, {"requestId", "scoped-topic-review"}, {"text", "复习集合关系"}});
        int calls = 0;
        bool receivedDuringPreparation = false;
        gangyi::LearningAgent reviewer(path.string(), [&](const auto& options, const auto& sink) {
            ++calls;
            for (const auto& message : options.messages) {
                const auto context = Json::parse(message.content, nullptr, false);
                if (context.is_object() && context.contains("event") && context.contains("topicEvidence")) {
                    assert(context.at("topicEvidence") == expected);
                    receivedDuringPreparation = true;
                }
            }
            const Json actions = calls == 1 ? Json::array({{{"id", "create-review"}, {"tool", "create_lesson"},
                {"args", {{"courseId", "math"}, {"intent", "review"}, {"topicId", topic},
                    {"title", "集合复习"}, {"purpose", "复习同一知识点"}}}}}) : Json::array();
            return output({{"message", calls == 1 ? "" : "复习课时已建立。"},
                {"state", calls == 1 ? "continue" : "waiting_student"}, {"actions", actions}}, sink);
        });
        gangyi::registerAgentLessonTools(reviewer);
        reviewer.process(db, prepare.at("id"));
        assert(receivedDuringPreparation);
        const auto prepareRow = db.getClassroomActivity(prepare.at("id").get<std::string>()).value();
        const auto savedTask = Json::parse(prepareRow.payload);
        const auto reviewId = savedTask.at("completedActions").at("create-review").at("result").at("lessonId").get<std::string>();
        const auto reviewLesson = Json::parse(db.getClassroomActivity(reviewId).value().payload);
        assert(reviewLesson.at("topicId") == topic && reviewLesson.at("intent") == "review");
        assert(!reviewLesson.contains("phaseIndex") && !reviewLesson.contains("topicIndex"));
    }
    {
        gangyi::Database db;
        db.open(path.string());
        assert(gangyi::agentTopicEvidence(db, access, "sets-uncertain") == expected);
        db.deleteInteractionsForConversation("sets-uncertain");
        const auto updated = gangyi::agentTopicEvidence(db, access, "sets-uncertain");
        assert(updated.at("evaluationCount") == 3);
        assert(updated.at("recentAttempts").back().at("evaluationId") == "sets-second-evaluation");

        const auto addOutline = [&](const std::string& course, int version, const std::string& id, const std::string& title) {
            const Json outline = {{"courseStructure", {{{"stage", "集合"}, {"topics", {{{"id", id}, {"title", title}}}}}}}};
            assert(db.insert(gangyi::CourseSnapshot{course + "-edition-outline-" + std::to_string(version),
                course, version, outline.dump(), when}));
        };
        addOutline("math", 2, topic, "1.3 集合的基本运算");
        addOutline("other-course", 1, "sets-b-topic", "1.1.3 集合的基本运算");
        const auto chapters = gangyi::libraryCurriculumRead(db, "math").at("chapters");
        for (const auto& entry : chapters) for (const auto& source : entry.at("sources")) {
            const auto url = source.at("url").get<std::string>();
            const auto id = "library-source:" + gangyi::libraryDigest(gangyi::libraryCanonicalURL(url));
            if (db.myAIRecordExists(id, "library-source")) continue;
            const auto snapshotId = id + "-fixture";
            const bool standard = source.at("role") == "standard";
            const bool editionA = entry.at("chapter").at("id") == "pep-a-sets-logic";
            const Json record = {{"id", id}, {"url", url}, {"title", source.at("title")}, {"snapshotId", snapshotId}};
            const Json segment = {{"id", "spot"}, {"page", standard ? 23 : 0}, {"text", standard ? "集合的基本关系" :
                (editionA ? "人教A版 第一章 集合与常用逻辑用语" : "高中数学B版同步教学 第一章 集合与常用逻辑用语")}};
            Json snapshot = {{"id", snapshotId}, {"sourceId", id}, {"kind", standard ? "pdf" : "html"},
                {"retrievedUrl", url}, {"checkedAt", when}, {"segments", Json::array({segment})}};
            if (standard) snapshot["documentDigest"] = "cc4d3a20daa09f5185037e93fbacd58f7df1147392e3dae4bf261c3700ba4195";
            assert(db.saveMyAIRecord({id, "library-source", record.dump(), when, 1}, 0));
            assert(db.saveMyAIRecord({snapshotId, "library-snapshot", snapshot.dump(), when, 1}, 0));
        }
        const auto choose = [&](const std::string& course, const std::string& chapter, int version) {
            return gangyi::libraryCurriculumWrite(db, {{"courseId", course}, {"chapterId", chapter}, {"version", version}}).at("selection");
        };
        assert(choose("math", "pep-a-sets-logic", 0).at("status") == "verified");
        assert(gangyi::agentTopicEvidence(db, access, "sets-uncertain").at("evaluationCount") == 3);
        assert(choose("other-course", "pep-b-sets-logic", 0).at("status") == "verified");
        const auto combined = gangyi::agentTopicEvidence(db, access, "sets-uncertain");
        assert(combined.at("curriculum").at("knowledgeId") == "cn-hs-math:set-operations");
        assert(combined.at("evaluationCount") == 5 && combined.at("distinctSupportedQuestionCount") == 3);
        assert(combined.at("independentFirstAttemptCount") == 2);
        const auto& attempts = combined.at("recentAttempts");
        const auto cross = std::find_if(attempts.begin(), attempts.end(), [](const auto& item) {
            return item.at("lessonId") == "different-course";
        });
        assert(cross != attempts.end() && cross->at("repeat") == true);
        assert(cross->at("curriculum").at("section") == "1.1.3" && cross->at("score") == 90);
        assert(combined.dump().find("sets-b-foreign-owner") == std::string::npos);
        assert(gangyi::agentTopicEvidenceForPreparation(db, access, "other-course", "sets-b-topic")
                   .at("distinctSupportedQuestionCount") == 3);
        assert(gangyi::agentTopicEvidence(db, {access.scopeId, {"math"}}, "sets-uncertain").at("evaluationCount") == 3);
        assert(gangyi::agentLessonView(db, access, "sets-uncertain").at("evaluations").empty());
        choose("other-course", "", 1);
        assert(gangyi::agentTopicEvidence(db, access, "sets-uncertain").at("evaluationCount") == 3);
    }
}

// 固定夹具只证明状态、权限与恢复机制，不替代真实模型对学生理解的判断质量。
inline void runLearningMasteryAgentChecks(const std::filesystem::path& path) {
    const gangyi::AgentAccess access{"mastery-check", {"math"}};
    const Json proposal = {{"lessonId", "mastery-lesson"}, {"status", "VERIFIED_MASTERY"},
        {"reason", "两道不同题均有正式独立作答证据"}, {"uncertainty", "尚无延迟记忆或新情境迁移证据，结论仅限本次验证范围"},
        {"evidenceIds", {"mastery-eval-1", "mastery-eval-2"}}, {"nextAction", "review"}};
    Json savedState;
    {
        gangyi::Database db; db.open(path.string()); db.migrate();
        assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "分数加法", "deep", "数学", {}, "ai", "active", "2026-11-16", "2026-11-16"}));
        Json lesson = {{"id", "mastery-lesson"}, {"scopeId", access.scopeId}, {"courseId", "math"},
            {"title", "分数加法"}, {"purpose", "根据真实回答判断理解"}, {"status", "ready"}, {"entered", true}, {"version", 1},
            {"updatedAt", "2026-11-16"}, {"sourceLearningVersion", db.learningRevision()}, {"sections", Json::array()}};
        for (int index = 1; index <= 2; ++index) {
            const auto suffix = std::to_string(index);
            const Json question = {{"question", index == 1 ? "1/2+1/3=?" : "1/4+1/6=?"}, {"expectedAnswer", "PRIVATE-MASTERY-ANSWER"}};
            const auto answer = index == 1 ? "通分后是5/6" : "通分后是5/12";
            lesson["sections"].push_back({{"id", "q" + suffix}, {"kind", "question"}, {"title", "独立练习"},
                {"questionKind", "practice"}, {"questionId", "mastery-q" + suffix}, {"version", 1}, {"question", question}});
            Json input = {{"scopeId", access.scopeId}, {"lessonId", "mastery-lesson"}, {"sectionId", "q" + suffix},
                {"requestId", "mastery-original-" + suffix}, {"questionId", "mastery-q" + suffix}, {"questionSnapshot", question},
                {"text", answer}, {"type", "question_answer"}, {"action", "answer"}};
            assert(db.insert(gangyi::LearningInteraction{"mastery-input-" + suffix, "chat-user", input.dump(), "2026-11-16", "math", {}, {}}));
            Json evaluation = {{"interactionId", "mastery-input-" + suffix}, {"questionId", "mastery-q" + suffix},
                {"questionSnapshot", question}, {"response", answer}, {"source", "ai"}, {"status", "valid"}, {"score", 100},
                {"credible", true}, {"assisted", false}, {"helpHistory", Json::array()}, {"helpMode", "independent"},
                {"diagnosis", {{"summary", "独立说明通分"}, {"evidence", {"通分"}}, {"teachingDirection", "换新问题验证"},
                               {"needsClarification", false}, {"status", "supported"}}}};
            assert(db.insert(gangyi::LearningInteraction{"mastery-eval-" + suffix, "practice", evaluation.dump(), "2026-11-16", "math", {}, {}}));
        }
        assert(db.upsert(gangyi::ClassroomActivity{"mastery-lesson", "math", "agent-lesson", lesson.dump(), "2026-11-16", 0, 0}));
        const auto task = gangyi::agentSubmit(db, access, {{"type", "chat"}, {"courseId", "math"}, {"lessonId", "mastery-lesson"},
                                                         {"requestId", "mastery-first"}, {"text", "请根据实际记录判断理解程度"}});
        int calls = 0;
        gangyi::LearningAgent agent(path.string(), [&](const auto&, const auto& sink) {
            ++calls;
            return output({{"message", calls == 1 ? "" : "已有本次独立作答依据，之后再安排复习。"},
                {"actions", calls == 1 ? Json::array({{{"id", "save-state"}, {"tool", "save_learning_state"}, {"args", proposal}}}) : Json::array()},
                {"state", calls == 1 ? "continue" : "waiting_student"}}, sink);
        });
        gangyi::registerAgentLessonTools(agent);
        agent.process(db, task.at("id"));
        assert(calls == 2 && gangyi::agentView(db, access, task.at("id")).at("status") == "waiting_student");
        savedState = gangyi::agentLessonView(db, access, "mastery-lesson").at("learningState");
        assert(savedState.at("mastery").at("status") == "VERIFIED_MASTERY");
        assert(savedState.at("nextTeachingAction") == Json({{"type", "review"}, {"executed", false}}));
        assert(savedState.dump().find("PRIVATE-MASTERY") == std::string::npos);
        bool denied = false;
        try { gangyi::agentLessonView(db, {"another-scope", {"math"}}, "mastery-lesson"); } catch (...) { denied = true; }
        assert(denied);
        assert(gangyi::agentLessonView(db, access, "mastery-lesson").at("evaluations").size() == 2);
    }
    {
        // 真正关闭并重新打开SQLite连接；纯读取不触发模型、改分或假装执行复习。
        gangyi::Database db; db.open(path.string());
        auto shown = gangyi::agentLessonView(db, access, "mastery-lesson");
        assert(shown.at("learningState") == savedState);
        auto row = db.getClassroomActivity("learning-state:mastery-lesson").value();
        auto raw = Json::parse(row.payload);
        raw["status"] = "MADE_UP"; row.payload = raw.dump(); assert(db.upsert(row));
        assert(gangyi::agentLessonView(db, access, "mastery-lesson").at("learningState").at("mastery").at("status") == "UNKNOWN");
        raw["status"] = "VERIFIED_MASTERY"; row.payload = raw.dump(); assert(db.upsert(row));
        // 追加新的不确定回答，不修改不可变原作答与成绩；旧依据仍须立即失效。
        Json nextInput;
        for (const auto& record : db.listInteractions()) if (record.id == "mastery-input-2") nextInput = Json::parse(record.payload);
        nextInput["requestId"] = "mastery-followup"; nextInput["text"] = "我还是不确定为什么要通分";
        assert(db.insert(gangyi::LearningInteraction{"mastery-input-3", "chat-user", nextInput.dump(), "2026-11-16", "math", {}, {}}));
        Json nextEvaluation = {{"interactionId", "mastery-input-3"}, {"questionId", "mastery-q2"},
            {"questionSnapshot", nextInput.at("questionSnapshot")}, {"response", nextInput.at("text")}, {"source", "ai"},
            {"status", "valid"}, {"credible", false}, {"score", nullptr},
            {"diagnosis", {{"status", "uncertain"}, {"needsClarification", true}, {"evidence", Json::array()}}}};
        assert(db.insert(gangyi::LearningInteraction{"mastery-eval-3", "practice", nextEvaluation.dump(), "2026-11-16", "math", {}, {}}));
        shown = gangyi::agentLessonView(db, access, "mastery-lesson");
        assert(shown.at("learningState").at("mastery").at("status") == "UNKNOWN");
        assert(shown.at("learningState").at("mastery").at("stale") == true);
    }
}

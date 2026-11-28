// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>

// 使用确定性工具动作验证保存回执与公开页面的一致性，不发起外部模型请求。
// 【逐行批注】 定义函数 runSectionNavigationChecks，执行该函数声明的职责。
inline void runSectionNavigationChecks(const std::filesystem::path& path) {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 【逐行批注】 执行当前声明或表达式：Database db; db.open(path.string()); db.migrate();。
    Database db; db.open(path.string()); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：const AgentAccess access{"section-navigation", {"navigation-math"}};。
    const AgentAccess access{"section-navigation", {"navigation-math"}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(Course{"navigation-math", access.scopeId, {}, "分数", …。
    assert(db.insert(Course{"navigation-math", access.scopeId, {}, "分数", "deep", "分数", {}, "ai", "active", "now", "now"}));
    // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
    Json lesson = {{"id", "navigation-lesson"}, {"scopeId", access.scopeId}, {"courseId", "navigation-math"},
        // 【逐行批注】 执行当前声明或表达式：{"title", "原卡片导航"}, {"purpose", "验证保存位置"}, {"status", "ready"}, {"entered", true},。
        {"title", "原卡片导航"}, {"purpose", "验证保存位置"}, {"status", "ready"}, {"entered", true},
        // 【逐行批注】 执行当前声明或表达式：{"version", 1}, {"sourceLearningVersion", db.learningRevision()}, {"sections", {。
        {"version", 1}, {"sourceLearningVersion", db.learningRevision()}, {"sections", {
            // 【逐行批注】 执行当前声明或表达式：{{"id", "original-question"}, {"kind", "question"}, {"title", "原练习"}, {"questionKind…。
            {{"id", "original-question"}, {"kind", "question"}, {"title", "原练习"}, {"questionKind", "practice"},
                // 【逐行批注】 执行当前声明或表达式：{"questionId", "original-question-id"}, {"version", 1}, {"question", {。
                {"questionId", "original-question-id"}, {"version", 1}, {"question", {
                    // 【逐行批注】 计算右侧结果并写入 {"question", "1/2+1/3，更新当前运行状态。
                    {"question", "1/2+1/3=?"}, {"type", "open"}, {"expectedAnswer", "PRIVATE-NAV-ANSWER"}, {"rubric", "PRIVATE-NAV-RUBRIC"}}}}
        // 【逐行批注】 执行当前声明或表达式：}}};。
        }}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(ClassroomActivity{"navigation-lesson", "navigation-m…。
    assert(db.upsert(ClassroomActivity{"navigation-lesson", "navigation-math", "agent-lesson", lesson.dump(), "now", 0, 0}));
    // 【逐行批注】 计算右侧结果并写入 int calls，更新当前运行状态。
    int calls = 0;
    // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
    LearningAgent agent(path.string(), [&](const auto&, const auto& sink) {
        // 【逐行批注】 执行当前声明或表达式：++calls;。
        ++calls;
        // 【逐行批注】 计算右侧结果并写入 Json actions，更新当前运行状态。
        Json actions = Json::array();
        // 【逐行批注】 计算右侧结果并写入 auto append，更新当前运行状态。
        const auto append = [&](const std::string& id, const Json& section, bool activate) {
            // 【逐行批注】 执行当前声明或表达式：actions.push_back({{"id", id}, {"tool", "append_section"}, {"args", {。
            actions.push_back({{"id", id}, {"tool", "append_section"}, {"args", {
                // 【逐行批注】 执行当前声明或表达式：{"lessonId", "navigation-lesson"}, {"section", section}, {"activate", activate}}}});。
                {"lessonId", "navigation-lesson"}, {"section", section}, {"activate", activate}}}});
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 调用 append 完成本行所需操作：append("saved-explanation", {{"kind", "explanation"}, {"title", "原题的一…。
        append("saved-explanation", {{"kind", "explanation"}, {"title", "原题的一点提示"}, {"body", "先比较两个分数的单位。"},
            // 【逐行批注】 执行当前声明或表达式：{"helpType", "hint"}, {"targetQuestionId", "original-question-id"}}, false);。
            {"helpType", "hint"}, {"targetQuestionId", "original-question-id"}}, false);
        // 【逐行批注】 按循环规则重复处理：for (const auto* kind : {"interaction", "practice"}) append(kind,。
        for (const auto* kind : {"interaction", "practice"}) append(kind,
            // 【逐行批注】 调用 string 完成本行所需操作：{{"kind", "question"}, {"questionKind", kind}, {"title", std::string(…。
            {{"kind", "question"}, {"questionKind", kind}, {"title", std::string(kind) + "新题"},
                // 【逐行批注】 调用 string 完成本行所需操作：{"question", {{"question", std::string(kind) + "：你怎样判断分数单位？"}, {"type…。
                {"question", {{"question", std::string(kind) + "：你怎样判断分数单位？"}, {"type", "open"},
                    // 【逐行批注】 执行当前声明或表达式：{"expectedAnswer", "PRIVATE-NAV-ANSWER"}, {"rubric", "PRIVATE-NAV-RUBRIC"}}}}, true);。
                    {"expectedAnswer", "PRIVATE-NAV-ANSWER"}, {"rubric", "PRIVATE-NAV-RUBRIC"}}}}, true);
        // 【逐行批注】 调用 append 完成本行所需操作：append("summary", {{"kind", "summary"}, {"title", "本次小结"}, {"body", "…。
        append("summary", {{"kind", "summary"}, {"title", "本次小结"}, {"body", "同一整体和相同分数单位需要分清。"}, {"helpType", "explanation"}}, true);
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "内容已保存，请按原卡片位置查看。"}, {"teaching", {{"helpT…。
        return output({{"message", "内容已保存，请按原卡片位置查看。"}, {"teaching", {{"helpType", "explanation"}}},
            // 【逐行批注】 执行当前声明或表达式：{"actions", actions}, {"state", "waiting_student"}}, sink);。
            {"actions", actions}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：registerAgentLessonTools(agent);。
    registerAgentLessonTools(agent);
    // 【逐行批注】 按循环规则重复处理：for (int index = 0; index < 3; ++index) {。
    for (int index = 0; index < 3; ++index) {
        // 【逐行批注】 计算右侧结果并写入 Json event，更新当前运行状态。
        Json event = {{"type", "chat"}, {"courseId", "navigation-math"}, {"lessonId", "navigation-lesson"},
            // 【逐行批注】 调用 to_string 完成本行所需操作：{"text", "继续解释分数单位"}, {"requestId", "navigation-" + std::to_string(in…。
            {"text", "继续解释分数单位"}, {"requestId", "navigation-" + std::to_string(index)}};
        // 【逐行批注】 判断条件 (index < 2) event["view"] = index == 0 ? "practice" : "learn";，成立时执行对应分支。
        if (index < 2) event["view"] = index == 0 ? "practice" : "learn";
        // 【逐行批注】 执行前一条件不成立时的分支。
        else event["sectionId"] = "original-question";
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task = agentSubmit(db, access, event);
        // 【逐行批注】 执行当前声明或表达式：agent.process(db, task.at("id"));。
        agent.process(db, task.at("id"));
        // 【逐行批注】 计算右侧结果并写入 auto finished，更新当前运行状态。
        const auto finished = agentView(db, access, task.at("id"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(finished.at("status") == "waiting_student" && calls == index +…。
        assert(finished.at("status") == "waiting_student" && calls == index + 1);
        // 【逐行批注】 计算右侧结果并写入 taskRow，更新当前运行状态。
        auto taskRow = *db.getClassroomActivity(task.at("id"));
        // 【逐行批注】 计算右侧结果并写入 raw，更新当前运行状态。
        auto raw = Json::parse(taskRow.payload);
        // 【逐行批注】 计算右侧结果并写入 auto beforeLesson，更新当前运行状态。
        const auto beforeLesson = db.getClassroomActivity("navigation-lesson")->payload;
        // 【逐行批注】 计算右侧结果并写入 auto beforeSectionExposure，更新当前运行状态。
        const auto beforeSectionExposure = db.profileMeta("agent-section-exposure:navigation-lesson");
        // 【逐行批注】 计算右侧结果并写入 auto beforeMessageExposure，更新当前运行状态。
        const auto beforeMessageExposure = db.profileMeta("agent-exposure:" + task.at("id").get<std::string>());
        // 【逐行批注】 计算右侧结果并写入 auto shown，更新当前运行状态。
        const auto shown = agentLessonView(db, access, "navigation-lesson");
        // 【逐行批注】 计算右侧结果并写入 std::size_t matchingTimeline，更新当前运行状态。
        std::size_t matchingTimeline = 0;
        // 【逐行批注】 按循环规则重复处理：for (const auto& action : raw.at("completedActions").items()) {。
        for (const auto& action : raw.at("completedActions").items()) {
            // 【逐行批注】 计算右侧结果并写入 auto& receipt，更新当前运行状态。
            const auto& receipt = action.value().at("result");
            // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
            const auto id = receipt.at("sectionId");
            // 【逐行批注】 计算右侧结果并写入 auto section，更新当前运行状态。
            const auto section = std::find_if(shown.at("sections").begin(), shown.at("sections").end(),
                // 【逐行批注】 执行当前声明或表达式：[&](const Json& value) { return value.at("id") == id; });。
                [&](const Json& value) { return value.at("id") == id; });
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(section != shown.at("sections").end());。
            assert(section != shown.at("sections").end());
            // 【逐行批注】 计算右侧结果并写入 auto expectedView，更新当前运行状态。
            const auto expectedView = section->at("kind") == "summary" ? "summary" : index != 1 ? "practice" :
                // 【逐行批注】 调用 value 完成本行所需操作：section->value("questionKind", "") == "practice" ? "practice" : "lear…。
                section->value("questionKind", "") == "practice" ? "practice" : "learn";
            // 【逐行批注】 调用 assert、at 完成本行所需操作：assert(receipt.at("view") == expectedView && section->at("view") == e…。
            assert(receipt.at("view") == expectedView && section->at("view") == expectedView);
            // 【逐行批注】 调用 assert、at 完成本行所需操作：assert(receipt.at("href") == section->at("href"));。
            assert(receipt.at("href") == section->at("href"));
            // 【逐行批注】 调用 assert、string 完成本行所需操作：assert(receipt.at("href") == "/" + std::string(expectedView) + "?less…。
            assert(receipt.at("href") == "/" + std::string(expectedView) + "?lessonId=navigation-lesson&sectionId=" + id.get<std::string>());
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : shown.at("timeline")) if (item.value("type", "") =…。
            for (const auto& item : shown.at("timeline")) if (item.value("type", "") == "section" && item.at("sectionId") == id) {
                // 【逐行批注】 调用 assert 完成本行所需操作：++matchingTimeline; assert(item.at("href") == receipt.at("href") && i…。
                ++matchingTimeline; assert(item.at("href") == receipt.at("href") && item.at("view") == receipt.at("view"));
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (action.key() == "saved-explanation") {，成立时执行对应分支。
            if (action.key() == "saved-explanation") {
                // 【逐行批注】 调用 assert、at 完成本行所需操作：assert(section->at("helpType") == "hint" && section->at("targetQuesti…。
                assert(section->at("helpType") == "hint" && section->at("targetQuestionId") == "original-question-id");
                // 【逐行批注】 调用 assert、at 完成本行所需操作：assert(!section->at("displayed").get<bool>());。
                assert(!section->at("displayed").get<bool>());
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(matchingTimeline == 3);。
        assert(matchingTimeline == 3);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.dump().find("PRIVATE-NAV") == std::string::npos);。
        assert(shown.dump().find("PRIVATE-NAV") == std::string::npos);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(shown.dump().find("expectedAnswer") == std::string::npos && sh…。
        assert(shown.dump().find("expectedAnswer") == std::string::npos && shown.dump().find("rubric") == std::string::npos);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity("navigation-lesson")->payload == befor…。
        assert(db.getClassroomActivity("navigation-lesson")->payload == beforeLesson);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("agent-section-exposure:navigation-lesson") == …。
        assert(db.profileMeta("agent-section-exposure:navigation-lesson") == beforeSectionExposure);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("agent-exposure:" + task.at("id").get<std::stri…。
        assert(db.profileMeta("agent-exposure:" + task.at("id").get<std::string>()) == beforeMessageExposure);
        // 旧版回执缺少新字段时，从原任务恢复同一板块归属，读取不回写旧记录。
        // 【逐行批注】 按循环规则重复处理：for (auto& action : raw["completedActions"].items()) { action.value()["res…。
        for (auto& action : raw["completedActions"].items()) { action.value()["result"].erase("view"); action.value()["result"].erase("href"); }
        // 【逐行批注】 计算右侧结果并写入 taskRow.payload，更新当前运行状态。
        taskRow.payload = raw.dump(); assert(db.upsert(taskRow));
        // 【逐行批注】 计算右侧结果并写入 auto recovered，更新当前运行状态。
        const auto recovered = agentLessonView(db, access, "navigation-lesson");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(recovered.at("sections") == shown.at("sections") && recovered.…。
        assert(recovered.at("sections") == shown.at("sections") && recovered.at("timeline") == shown.at("timeline"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity(task.at("id"))->payload == taskRow.pay…。
        assert(db.getClassroomActivity(task.at("id"))->payload == taskRow.payload && calls == index + 1);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 bool denied，更新当前运行状态。
    bool denied = false;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { agentLessonView(db, {"another-scope", {"navigation-math"}}, "navigation-lesson"); }
    // 【逐行批注】 捕获并处理异常：catch (const std::invalid_argument&) { denied = true; }。
    catch (const std::invalid_argument&) { denied = true; }
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(denied);。
    assert(denied);
    // 【逐行批注】 计算右侧结果并写入 denied，更新当前运行状态。
    denied = false;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { agentLessonView(db, {access.scopeId, {}}, "navigation-lesson"); }
    // 【逐行批注】 捕获并处理异常：catch (const std::invalid_argument&) { denied = true; }。
    catch (const std::invalid_argument&) { denied = true; }
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(denied);。
    assert(denied);
    // 地址按查询参数编码，特殊字符不能截断课时和板块身份。
    // 【逐行批注】 计算右侧结果并写入 auto encoded，更新当前运行状态。
    const auto encoded = agentSectionLocation({{"id", "lesson &/"}}, {{"id", "section?一"}, {"kind", "explanation"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(encoded.at("href") == "/learn?lessonId=lesson%20%26%2F&section…。
    assert(encoded.at("href") == "/learn?lessonId=lesson%20%26%2F&sectionId=section%3F%E4%B8%80");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 其他题的完整示范不污染原题，但以后复做被示范题时不能再算独立作答。
// 【逐行批注】 定义函数 runDemonstratedQuestionChecks，执行该函数声明的职责。
inline void runDemonstratedQuestionChecks(const std::filesystem::path& path) {
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(path.string()); db.migrate();。
    gangyi::Database db; db.open(path.string()); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess access{"demo-check", {"math"}};。
    const gangyi::AgentAccess access{"demo-check", {"math"}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "数学", "de…。
    assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "数学", "deep", "数学", {}, "ai", "active", "2026-10-14", "2026-10-14"}));
    // 【逐行批注】 计算右侧结果并写入 Json q1，更新当前运行状态。
    Json q1={{"question","计算1/2+1/3"},{"expectedAnswer","PRIVATE-5/6"}};
    // 【逐行批注】 计算右侧结果并写入 Json q2，更新当前运行状态。
    Json q2={{"question","计算1/4+1/6"},{"expectedAnswer","PRIVATE-5/12"}};
    // 【逐行批注】 计算右侧结果并写入 q3，更新当前运行状态。
    auto q3=q2; q3["materials"]={{{"text","把第二个分数改为1/8再计算"}}};
    // 【逐行批注】 计算右侧结果并写入 Json sections，更新当前运行状态。
    Json sections=Json::array();
    // 【逐行批注】 按循环规则重复处理：for(const auto& pair:std::vector<std::pair<std::string,Json>>{{"q1",q1},{"…。
    for(const auto& pair:std::vector<std::pair<std::string,Json>>{{"q1",q1},{"q2",q2},{"q3",q3}})
        // 【逐行批注】 执行当前声明或表达式：sections.push_back({{"id",pair.first},{"kind","question"},{"questionKind","practice"…。
        sections.push_back({{"id",pair.first},{"kind","question"},{"questionKind","practice"},{"version",1},
            // 【逐行批注】 调用 teachingQuestionIdentity 完成本行所需操作：{"question",pair.second},{"questionId",gangyi::teachingQuestionIdenti…。
            {"question",pair.second},{"questionId",gangyi::teachingQuestionIdentity("math",pair.second)}});
    // 【逐行批注】 计算右侧结果并写入 Json lesson，更新当前运行状态。
    const Json lesson={{"id","demo-lesson"},{"scopeId",access.scopeId},{"courseId","math"},{"title","分数"},{"purpose","验证示范题与复做关联"},
        // 【逐行批注】 执行当前声明或表达式：{"status","ready"},{"entered",true},{"version",1},{"updatedAt","2026-10-14"},{"sourc…。
        {"status","ready"},{"entered",true},{"version",1},{"updatedAt","2026-10-14"},{"sourceLearningVersion",db.learningRevision()},{"sections",sections}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(gangyi::ClassroomActivity{"demo-lesson","math","agen…。
    assert(db.upsert(gangyi::ClassroomActivity{"demo-lesson","math","agent-lesson",lesson.dump(),"2026-10-14",0,0}));
    // 【逐行批注】 计算右侧结果并写入 int serial，更新当前运行状态。
    int serial=0;
    // 【逐行批注】 计算右侧结果并写入 auto submit，更新当前运行状态。
    const auto submit=[&](const std::string& section,const std::string& type="question_answer") {
        // 【逐行批注】 结束当前函数并返回结果：return gangyi::agentSubmit(db,access,{{"type",type},{"text","5/6"},{"…。
        return gangyi::agentSubmit(db,access,{{"type",type},{"text","5/6"},{"courseId","math"},{"lessonId","demo-lesson"},
            // 【逐行批注】 调用 to_string 完成本行所需操作：{"sectionId",section},{"sectionVersion",1},{"view","practice"},{"requ…。
            {"sectionId",section},{"sectionVersion",1},{"view","practice"},{"requestId","demo-"+std::to_string(++serial)}});
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto inputFor，更新当前运行状态。
    const auto inputFor=[&](const Json& task) {
        // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
        const auto raw=Json::parse(db.getClassroomActivity(task.at("id"))->payload);
        // 【逐行批注】 结束当前函数并返回结果：return gangyi::agentContext(db,access,raw.at("event")).at("currentInp…。
        return gangyi::agentContext(db,access,raw.at("event")).at("currentInputs")[0];
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto inspect，更新当前运行状态。
    const auto inspect=[&](const std::string& section) {
        // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
        const auto task=submit(section),input=inputFor(task);
        // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db,access,{{"command","cancel"},{"taskId",task.a…。
        gangyi::agentControl(db,access,{{"command","cancel"},{"taskId",task.at("id")}}); return input;
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto teaching，更新当前运行状态。
    const auto teaching=submit("q1","chat");
    // 【逐行批注】 定义函数 teacher，执行该函数声明的职责。
    gangyi::LearningAgent teacher(path.string(),[&](const auto&,const auto& sink) {
        // 【逐行批注】 计算右侧结果并写入 Json help，更新当前运行状态。
        const Json help={{"helpType","explanation"},{"demonstratedQuestions",{q2}}};
        // 【逐行批注】 计算右侧结果并写入 Json section，更新当前运行状态。
        Json section=help;section["kind"]="explanation";section["title"]="用另一题讲单位";
        // 【逐行批注】 计算右侧结果并写入 section["body"]，更新当前运行状态。
        section["body"]="举例：1/4=3/12，1/6=2/12，所以1/4+1/6=5/12。";
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message",section.at("body")},{"teaching",help},{"act…。
        return output({{"message",section.at("body")},{"teaching",help},{"actions",{{{"id","demo"},{"tool","append_section"},
            // 【逐行批注】 执行当前声明或表达式：{"args",{{"lessonId","demo-lesson"},{"section",section}}}}}},{"state","waiting_stude…。
            {"args",{{"lessonId","demo-lesson"},{"section",section}}}}}},{"state","waiting_student"}},sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(teacher);teacher.process(db,teaching…。
    gangyi::registerAgentLessonTools(teacher);teacher.process(db,teaching.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db,access,teaching.at("id")).at("status")=="…。
    assert(gangyi::agentView(db,access,teaching.at("id")).at("status")=="waiting_student");
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db,access,teaching.at("id")).dump().find("PR…。
    assert(gangyi::agentView(db,access,teaching.at("id")).dump().find("PRIVATE-")==std::string::npos);
    // 【逐行批注】 计算右侧结果并写入 auto notShown，更新当前运行状态。
    const auto notShown=inspect("q2");assert(notShown.at("helpMode")=="independent"&&notShown.at("shownDemonstrations").empty());
    // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
    const auto saved=Json::parse(db.getClassroomActivity("demo-lesson")->payload).at("sections").back();
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(saved.at("demonstratedQuestions").dump().find("PRIVATE-")==std…。
    assert(saved.at("demonstratedQuestions").dump().find("PRIVATE-")==std::string::npos);
    // 旧客户端只有板块版本，保留已展示事实，但完整程度不能猜测。
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"secti…。
    gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"sections",{{{"id",saved.at("id")},{"version",1}}}}});
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q2").at("helpMode")=="help_unknown");。
    assert(inspect("q2").at("helpMode")=="help_unknown");
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q3").at("shownDemonstrations")[0].at("complete")==fal…。
    assert(inspect("q3").at("shownDemonstrations")[0].at("complete")==false);
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"secti…。
    gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"sections",{{{"id",saved.at("id")},{"version",1},{"complete",false}}}}});
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q2").at("helpMode")=="help_unknown");。
    assert(inspect("q2").at("helpMode")=="help_unknown");
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"secti…。
    gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"sections",{{{"id",saved.at("id")},{"version",1},{"complete",true}}}}});
    // 模拟刷新后收到旧的部分展示回执，不能把已完整展示的同版本记录降级。
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"secti…。
    gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"sections",{{{"id",saved.at("id")},{"version",1},{"complete",false}}}}});
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q2").at("helpMode")=="after_example");。
    assert(inspect("q2").at("helpMode")=="after_example");
    // 【逐行批注】 计算右侧结果并写入 auto original，更新当前运行状态。
    const auto original=inspect("q1");assert(original.at("helpMode")=="independent");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(original.at("relatedLearning")[0].at("demonstratedQuestions")[…。
    assert(original.at("relatedLearning")[0].at("demonstratedQuestions")[0].at("question")==q2.at("question"));
    // 【逐行批注】 计算右侧结果并写入 auto different，更新当前运行状态。
    const auto different=inspect("q3");assert(different.at("helpMode")=="independent");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(different.at("shownDemonstrations").size()==1&&different.at("s…。
    assert(different.at("shownDemonstrations").size()==1&&different.at("shownDemonstrations")[0].at("complete")==true);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(different.at("shownDemonstrations")[0].at("questionSnapshot").…。
    assert(different.at("shownDemonstrations")[0].at("questionSnapshot").at("question")==q2.at("question"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(different.at("shownDemonstrations").dump().find("PRIVATE-")==s…。
    assert(different.at("shownDemonstrations").dump().find("PRIVATE-")==std::string::npos);
    // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
    const auto raw=Json::parse(db.getClassroomActivity(teaching.at("id"))->payload);
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db,access,{{"taskId",teaching.at("id")},{"seq…。
    gangyi::agentExposeTask(db,access,{{"taskId",teaching.at("id")},{"seq",raw.at("seq")}});
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q2").at("helpMode")=="after_example");。
    assert(inspect("q2").at("helpMode")=="after_example");
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q2").at("helpHistory").back().at("helpType")=="unknow…。
    assert(inspect("q2").at("helpHistory").back().at("helpType")=="unknown");
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db,access,{{"taskId",teaching.at("id")},{"seq…。
    gangyi::agentExposeTask(db,access,{{"taskId",teaching.at("id")},{"seq",raw.at("seq")},{"complete",true}});
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q2").at("helpHistory").size()==2);。
    assert(inspect("q2").at("helpHistory").size()==2);
    // 【逐行批注】 调用 assert、inspect 完成本行所需操作：assert(inspect("q3").at("shownDemonstrations")[0].at("sources").size(…。
    assert(inspect("q3").at("shownDemonstrations")[0].at("sources").size()==2);
    // 已发布反馈中的另一题示范也使用同一关联机制，部分展示仍不能认定完整示范。
    // 【逐行批注】 计算右侧结果并写入 auto answer，更新当前运行状态。
    const auto answer=submit("q1");const auto input=inputFor(answer);int calls=0;
    // 【逐行批注】 定义函数 grader，执行该函数声明的职责。
    gangyi::LearningAgent grader(path.string(),[&](const auto&,const auto& sink) {
        // 【逐行批注】 计算右侧结果并写入 Json args，更新当前运行状态。
        const Json args={{"interactionId",input.at("interactionId")},{"score",90},{"credible",true},
            // 【逐行批注】 计算右侧结果并写入 {"feedback","原题计算正确。另一题1/4+1/6，更新当前运行状态。
            {"feedback","原题计算正确。另一题1/4+1/6=3/12+2/12=5/12。"},{"feedbackHelpType","explanation"},
            // 【逐行批注】 执行当前声明或表达式：{"demonstratedQuestions",{q2}},{"reason","依据本次回答"},{"uncertainty","固定协议夹具不证明教学成效"},。
            {"demonstratedQuestions",{q2}},{"reason","依据本次回答"},{"uncertainty","固定协议夹具不证明教学成效"},
            // 【逐行批注】 执行当前声明或表达式：{"diagnosis",{{"summary","本次计算正确"},{"evidence",{"5/6"}},{"teachingDirection","继续解释分数…。
            {"diagnosis",{{"summary","本次计算正确"},{"evidence",{"5/6"}},{"teachingDirection","继续解释分数单位"},{"needsClarification",false}}}};
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message",calls++?"请继续说明理由。":""},{"teaching",{{"helpT…。
        return output({{"message",calls++?"请继续说明理由。":""},{"teaching",{{"helpType","none"}}},
            // 【逐行批注】 调用 array 完成本行所需操作：{"actions",calls==1?Json::array({{{"id","grade"},{"tool","evaluate_an…。
            {"actions",calls==1?Json::array({{{"id","grade"},{"tool","evaluate_answer"},{"args",args}}}):Json::array()},
            // 【逐行批注】 执行当前声明或表达式：{"state",calls==1?"continue":"waiting_student"}},sink);。
            {"state",calls==1?"continue":"waiting_student"}},sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 调用 registerAgentLessonTools 完成本行所需操作：gangyi::registerAgentLessonTools(grader);grader.process(db,answer.at(…。
    gangyi::registerAgentLessonTools(grader);grader.process(db,answer.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto evaluation，更新当前运行状态。
    const auto evaluation=gangyi::agentLessonView(db,access,"demo-lesson").at("evaluations").back();
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"evalu…。
    gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"evaluations",{{{"id",evaluation.at("id")},{"feedback",evaluation.at("feedback")},{"complete",false}}}}});
    // 【逐行批注】 计算右侧结果并写入 auto feedbackFor，更新当前运行状态。
    const auto feedbackFor=[&](const Json& input) { for(const auto& item:input.at("helpHistory")) if(item.value("source","")=="evaluation") return item; return Json(); };
    // 【逐行批注】 计算右侧结果并写入 auto partial，更新当前运行状态。
    const auto partial=inspect("q2");assert(partial.at("helpHistory").size()==3&&feedbackFor(partial).at("helpType")=="unknown");
    // 【逐行批注】 调用 agentExposeTask 完成本行所需操作：gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"evalu…。
    gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"evaluations",{{{"id",evaluation.at("id")},{"feedback",evaluation.at("feedback")},{"complete",true}}}}});
    // 【逐行批注】 调用 assert、feedbackFor、inspect 完成本行所需操作：assert(feedbackFor(inspect("q2")).at("helpType")=="worked_example");。
    assert(feedbackFor(inspect("q2")).at("helpType")=="worked_example");
    // 【逐行批注】 计算右侧结果并写入 auto hint，更新当前运行状态。
    const auto hint=gangyi::agentSubmit(db,access,{{"type","teaching_request"},{"requestType","hint"},{"text","给一点提示"},
        // 【逐行批注】 执行当前声明或表达式：{"courseId","math"},{"lessonId","demo-lesson"},{"sectionId","q1"},{"sectionVersion",…。
        {"courseId","math"},{"lessonId","demo-lesson"},{"sectionId","q1"},{"sectionVersion",1},{"view","practice"},{"requestId","demo-hint"}});
    // 【逐行批注】 计算右侧结果并写入 teacher.process(db,hint.at("id"));const auto…，更新当前运行状态。
    teacher.process(db,hint.at("id"));const auto refused=gangyi::agentView(db,access,hint.at("id"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(refused.at("status")=="failed"&&refused.value("message","").em…。
    assert(refused.at("status")=="failed"&&refused.value("message","").empty());
    // 【逐行批注】 调用 assert、checkedDemonstrations 完成本行所需操作：assert(gangyi::checkedDemonstrations({{"demonstratedQuestions",{q2,q2…。
    assert(gangyi::checkedDemonstrations({{"demonstratedQuestions",{q2,q2}}}).size()==1);
    // 【逐行批注】 计算右侧结果并写入 bool rejected，更新当前运行状态。
    bool rejected=false;try{gangyi::checkedDemonstrations({{"demonstratedQuestions",{{{"question"," "}}}}});}catch(...){rejected=true;}assert(rejected);
// 【逐行批注】 结束当前代码块。
}

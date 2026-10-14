#pragma once

// 其他题的完整示范不污染原题，但以后复做被示范题时不能再算独立作答。
inline void runDemonstratedQuestionChecks(const std::filesystem::path& path) {
    gangyi::Database db; db.open(path.string()); db.migrate();
    const gangyi::AgentAccess access{"demo-check", {"math"}};
    assert(db.insert(gangyi::Course{"math", access.scopeId, {}, "数学", "deep", "数学", {}, "ai", "active", "2026-10-14", "2026-10-14"}));
    Json q1={{"question","计算1/2+1/3"},{"expectedAnswer","PRIVATE-5/6"}};
    Json q2={{"question","计算1/4+1/6"},{"expectedAnswer","PRIVATE-5/12"}};
    auto q3=q2; q3["materials"]={{{"text","把第二个分数改为1/8再计算"}}};
    Json sections=Json::array();
    for(const auto& pair:std::vector<std::pair<std::string,Json>>{{"q1",q1},{"q2",q2},{"q3",q3}})
        sections.push_back({{"id",pair.first},{"kind","question"},{"questionKind","practice"},{"version",1},
            {"question",pair.second},{"questionId",gangyi::teachingQuestionIdentity("math",pair.second)}});
    const Json lesson={{"id","demo-lesson"},{"scopeId",access.scopeId},{"courseId","math"},{"title","分数"},{"purpose","验证示范题与复做关联"},
        {"status","ready"},{"entered",true},{"version",1},{"updatedAt","2026-10-14"},{"sourceLearningVersion",db.learningRevision()},{"sections",sections}};
    assert(db.upsert(gangyi::ClassroomActivity{"demo-lesson","math","agent-lesson",lesson.dump(),"2026-10-14",0,0}));
    int serial=0;
    const auto submit=[&](const std::string& section,const std::string& type="question_answer") {
        return gangyi::agentSubmit(db,access,{{"type",type},{"text","5/6"},{"courseId","math"},{"lessonId","demo-lesson"},
            {"sectionId",section},{"sectionVersion",1},{"view","practice"},{"requestId","demo-"+std::to_string(++serial)}});
    };
    const auto inputFor=[&](const Json& task) {
        const auto raw=Json::parse(db.getClassroomActivity(task.at("id"))->payload);
        return gangyi::agentContext(db,access,raw.at("event")).at("currentInputs")[0];
    };
    const auto inspect=[&](const std::string& section) {
        const auto task=submit(section),input=inputFor(task);
        gangyi::agentControl(db,access,{{"command","cancel"},{"taskId",task.at("id")}}); return input;
    };
    const auto teaching=submit("q1","chat");
    gangyi::LearningAgent teacher(path.string(),[&](const auto&,const auto& sink) {
        const Json help={{"helpType","explanation"},{"demonstratedQuestions",{q2}}};
        Json section=help;section["kind"]="explanation";section["title"]="用另一题讲单位";
        section["body"]="举例：1/4=3/12，1/6=2/12，所以1/4+1/6=5/12。";
        return output({{"message",section.at("body")},{"teaching",help},{"actions",{{{"id","demo"},{"tool","append_section"},
            {"args",{{"lessonId","demo-lesson"},{"section",section}}}}}},{"state","waiting_student"}},sink);
    });
    gangyi::registerAgentLessonTools(teacher);teacher.process(db,teaching.at("id"));
    assert(gangyi::agentView(db,access,teaching.at("id")).at("status")=="waiting_student");
    assert(gangyi::agentView(db,access,teaching.at("id")).dump().find("PRIVATE-")==std::string::npos);
    assert(inspect("q2").at("helpMode")=="independent");
    const auto saved=Json::parse(db.getClassroomActivity("demo-lesson")->payload).at("sections").back();
    assert(saved.at("demonstratedQuestions").dump().find("PRIVATE-")==std::string::npos);
    gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"sections",{{{"id",saved.at("id")},{"version",1}}}}});
    assert(inspect("q2").at("helpMode")=="after_example");
    const auto original=inspect("q1");assert(original.at("helpMode")=="independent");
    assert(original.at("relatedLearning")[0].at("demonstratedQuestions")[0].at("question")==q2.at("question"));
    assert(inspect("q3").at("helpMode")=="independent");
    const auto raw=Json::parse(db.getClassroomActivity(teaching.at("id"))->payload);
    gangyi::agentExposeTask(db,access,{{"taskId",teaching.at("id")},{"seq",raw.at("seq")}});
    assert(inspect("q2").at("helpHistory").size()==2);
    // 已发布反馈中的另一题示范也使用同一关联机制，部分展示仍不能认定完整示范。
    const auto answer=submit("q1");const auto input=inputFor(answer);int calls=0;
    gangyi::LearningAgent grader(path.string(),[&](const auto&,const auto& sink) {
        const Json args={{"interactionId",input.at("interactionId")},{"score",90},{"credible",true},
            {"feedback","原题计算正确。另一题1/4+1/6=3/12+2/12=5/12。"},{"feedbackHelpType","explanation"},
            {"demonstratedQuestions",{q2}},{"reason","依据本次回答"},{"uncertainty","固定协议夹具不证明教学成效"},
            {"diagnosis",{{"summary","本次计算正确"},{"evidence",{"5/6"}},{"teachingDirection","继续解释分数单位"},{"needsClarification",false}}}};
        return output({{"message",calls++?"请继续说明理由。":""},{"teaching",{{"helpType","none"}}},
            {"actions",calls==1?Json::array({{{"id","grade"},{"tool","evaluate_answer"},{"args",args}}}):Json::array()},
            {"state",calls==1?"continue":"waiting_student"}},sink);
    });
    gangyi::registerAgentLessonTools(grader);grader.process(db,answer.at("id"));
    const auto evaluation=gangyi::agentLessonView(db,access,"demo-lesson").at("evaluations").back();
    gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"evaluations",{{{"id",evaluation.at("id")},{"feedback",evaluation.at("feedback")},{"complete",false}}}}});
    const auto feedbackFor=[&](const Json& input) { for(const auto& item:input.at("helpHistory")) if(item.value("source","")=="evaluation") return item; return Json(); };
    const auto partial=inspect("q2");assert(partial.at("helpHistory").size()==3&&feedbackFor(partial).at("helpType")=="unknown");
    gangyi::agentExposeTask(db,access,{{"lessonId","demo-lesson"},{"evaluations",{{{"id",evaluation.at("id")},{"feedback",evaluation.at("feedback")},{"complete",true}}}}});
    assert(feedbackFor(inspect("q2")).at("helpType")=="worked_example");
    const auto hint=gangyi::agentSubmit(db,access,{{"type","teaching_request"},{"requestType","hint"},{"text","给一点提示"},
        {"courseId","math"},{"lessonId","demo-lesson"},{"sectionId","q1"},{"sectionVersion",1},{"view","practice"},{"requestId","demo-hint"}});
    teacher.process(db,hint.at("id"));const auto refused=gangyi::agentView(db,access,hint.at("id"));
    assert(refused.at("status")=="failed"&&refused.value("message","").empty());
    assert(gangyi::checkedDemonstrations({{"demonstratedQuestions",{q2,q2}}}).size()==1);
    bool rejected=false;try{gangyi::checkedDemonstrations({{"demonstratedQuestions",{{{"question"," "}}}}});}catch(...){rejected=true;}assert(rejected);
}

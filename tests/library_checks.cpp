#ifdef NDEBUG
#undef NDEBUG
#endif
#include "learning_library.hpp"
#include "learning_agent.hpp"
#include "agent_prompt.hpp"
#include <cassert>
#include <algorithm>
#include <iostream>
using Json = nlohmann::json;
void runLibraryChecks() {
    gangyi::Database db; db.open(":memory:"); db.migrate();
    assert(gangyi::libraryDigest("abc")=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    assert(!gangyi::libraryPublicURL("file:///C:/Users/private.txt"));
    assert(!gangyi::libraryPublicURL("http://127.0.0.1/api/my-ai/profile"));
    assert(!gangyi::libraryPublicURL("https://user:password@example.org/"));
    assert(!gangyi::libraryPublicURL("http://172.16.2.3/private"));
    assert(!gangyi::libraryPublicURL("http://[fc00::1]/private"));
    assert(!gangyi::libraryPublicURL("http://localhost./private"));
    assert(gangyi::libraryPublicURL("https://example.org/Study?Version=A"));
    assert(gangyi::libraryCanonicalURL("https://Example.org/Study?Version=A#part") == "https://example.org/Study?Version=A");
    auto source = gangyi::libraryWrite(db, {{"url","https://example.org/Study"},{"title","固定原文"}});
    const std::string id = source["id"];
    const Json first = {{"sourceId",id},{"checkedAt","2026-10-13T00:00:00Z"},{"publishedAt",nullptr},
        {"segments",Json::array({{{"id","s1"},{"text","分母相同才可以直接相加分子。"}}})}};
    assert(db.saveMyAIRecord({"snap-first","library-snapshot",first.dump(),"2026-10-13",0},0));
    auto value = Json::parse(db.myAIRecord(id)->payload); value["snapshotId"]="snap-first"; value["readStatus"]="read";
    assert(db.saveMyAIRecord({id,"library-source",value.dump(),"2026-10-13",0},source["version"]));
    const Json citation = {{"sourceId",id},{"snapshotId","snap-first"},{"segmentId","s1"},{"quote","分母相同"}};
    const auto verified=gangyi::libraryValidateCitations(db,Json::array({citation}));
    assert(verified[0]["publishedAt"].is_null()&&verified[0]["locator"]["text"]==first["segments"][0]["text"]);
    bool badQuote=false;try{auto invalid=citation;invalid["quote"]="原文并不存在这句话";gangyi::libraryValidateCitations(db,Json::array({invalid}));}catch(...){badQuote=true;}assert(badQuote);
    bool badSegment=false;try{auto invalid=citation;invalid["segmentId"]="p99";gangyi::libraryValidateCitations(db,Json::array({invalid}));}catch(...){badSegment=true;}assert(badSegment);
    auto detail=gangyi::libraryRead(db,{{"id",id}});
    gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"favorite",true}});
    bool conflict=false;try{gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"favorite",false}});}catch(...){conflict=true;}assert(conflict);
    detail=gangyi::libraryRead(db,{{"id",id}});
    gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"excluded",true}});
    assert(gangyi::libraryContext(db)["sources"].empty());
    assert(!gangyi::libraryValidateCitations(db,Json::array({citation}),false).empty());
    bool excluded=false;try{gangyi::libraryValidateCitations(db,Json::array({citation}));}catch(...){excluded=true;}assert(excluded);
    const auto captions=gangyi::libraryParseCaptions("WEBVTT\n\n00:01.000 --> 00:03.000\n先通分。\n\n00:03.000 --> 00:05.000\n再相加。\n","text/vtt");
    assert(captions["segments"].size()==2&&captions["segments"][0]["start"]==1.0&&captions["segments"][1]["end"]==5.0);
    assert(captions["automatic"].is_null());
    const auto jsonCaptions=gangyi::libraryParseCaptions(R"({"body":[{"from":2,"to":4,"content":"第二段字幕"}]})","application/json");
    assert(jsonCaptions["status"]=="read"&&jsonCaptions["segments"][0]["id"]=="t1");
    assert(gangyi::libraryParseCaptions("WEBVTT\n\n00:07.000 --> 00:03.000\n错误时间范围\n","text/vtt")["segments"].empty());
    assert(gangyi::libraryParseCaptions(R"({"body":[{"from":-1,"to":"invalid","content":"不能用于引用"}]})","application/json")["segments"].empty());
    assert(gangyi::libraryParseCaptions(R"({"automatic":true,"body":[{"from":0,"to":1,"content":"自动字幕"}]})","application/json")["automatic"]==true);
    assert(gangyi::libraryParseCaptions("WEBVTT\n\n"+std::string(400,'9')+":00:01.000 --> 00:02.000\n超长时间不可引用\n","text/vtt")["segments"].empty());
    gangyi::LibraryDocument transcript; transcript.mime="text/plain"; transcript.body="公开文字稿第一段。\n\n第二段不伪造时间戳。";
    const auto parsedTranscript=gangyi::libraryParseDocument(transcript);
    assert(parsedTranscript["segments"].size()==2&&!parsedTranscript["segments"][0].contains("start"));
    assert(gangyi::libraryUsage(db)["searchRequests"]==0&&gangyi::libraryUsage(db)["tokens"].is_null());
    assert(db.saveMyAIRecord({"partial-read","library-usage",Json{{"kind","fetch"},{"status","partial"},{"attempts",1},{"cached",false}}.dump(),"2026-10-14",0},0));
    assert(gangyi::libraryUsage(db)["partialReads"]==1&&gangyi::libraryUsage(db)["failures"]==0&&gangyi::libraryUsage(db)["fetchRequests"]==1);
    assert(db.listInteractions().empty()&&db.listMastery().empty());
    Json originalContext = {{"event", {{"type", "library_research"}}}, {"personalCenter", {{"preferences", "先解释再练习"}}},
        {"savedLibrary", {{"sources", Json::array({{{"id", id}, {"snapshotId", "snap-first"}, {"publishedAt", nullptr},
            {"choice", {{"manual", true}, {"state", "selected"}}}, {"segments", first["segments"]}}})}}}};
    const auto promptContext=gangyi::agentPromptContext(originalContext,false);
    assert(promptContext["personalCenter"]==originalContext["personalCenter"]&&promptContext["savedLibrary"]["sources"][0]["choice"]["manual"]==true);
    assert(!promptContext["savedLibrary"]["sources"][0].contains("segments")&&originalContext["savedLibrary"]["sources"][0].contains("segments"));
    const Json readResult={{"snapshot",first}};
    const Json completed={{"read-1",{{"encoded",Json{{"tool","read_resource"},{"args",{{"sourceId",id}}}}.dump()},{"result",readResult}}}};
    const Json messages=Json::array({{{"role","user"},{"content",Json{{"toolResults",Json::array({{{"id","read-1"},{"result",readResult}}})}}.dump()}},
        {{"role","user"},{"content",Json{{"completedActions",completed},{"toolError","原始错误必须保留"}}.dump()}}});
    const auto compactHistory=gangyi::agentPromptHistory(messages,completed,{"read_resource"});
    const auto firstMessage=Json::parse(compactHistory[0].at("content").get<std::string>()),lastMessage=Json::parse(compactHistory[1].at("content").get<std::string>());
    assert(firstMessage["toolResults"][0]["result"]==readResult&&lastMessage["completedActions"]["read-1"]["result"]["unchangedFrom"]=="read-1");
    assert(lastMessage["toolError"]=="原始错误必须保留"&&messages[1].at("content").get<std::string>().find("snapshot")!=std::string::npos);
    const gangyi::AgentAccess access{"local-profile", {}};
    const auto task=gangyi::agentSubmit(db,access,{{"type","library_research"},{"goal","核验假引用拒绝"},{"requestId","invalid-citation"}});
    gangyi::LearningAgent guard(":memory:",[&](const auto&, const auto& sink){
        gangyi::AIResult result;result.model="引用拒绝夹具";result.finishReason="stop";
        result.content=Json{{"message","UNVERIFIED-CITATION-MUST-NOT-APPEAR"},{"materialOrigin","source_based"},
            {"citations",Json::array({{{"sourceId","missing-source"},{"snapshotId","missing-snapshot"},{"segmentId","s1"}}})},
            {"actions",Json::array()},{"state","completed"}}.dump();sink(result.content);return result;
    });
    guard.process(db,task.at("id"));
    const auto rejected=gangyi::agentView(db,access,task.at("id"));
    assert(rejected["status"]=="failed"&&rejected.dump().find("UNVERIFIED-CITATION-MUST-NOT-APPEAR")==std::string::npos);
    const auto noSourceTask=gangyi::agentSubmit(db,access,{{"type","chat"},{"text","用原文讲解"},{"requestId","invalid-citation-without-source"}});
    assert(gangyi::libraryContext(db)["sources"].empty());
    guard.process(db,noSourceTask.at("id"));
    const auto noSourceRejected=gangyi::agentView(db,access,noSourceTask.at("id"));
    assert(noSourceRejected["status"]=="failed"&&noSourceRejected.dump().find("UNVERIFIED-CITATION-MUST-NOT-APPEAR")==std::string::npos);
    detail=gangyi::libraryRead(db,{{"id",id}});
    gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"excluded",false}});
    const auto pausedTask=gangyi::agentSubmit(db,access,{{"type","library_research"},{"goal","验证已读步骤暂停恢复"},{"requestId","library-resume"}});
    int resumeCalls=0;
    gangyi::LearningAgent resumeGuard(":memory:",[&](const auto&,const auto& sink){
        ++resumeCalls;
        if(resumeCalls==2)gangyi::agentControl(db,access,{{"command","pause"},{"taskId",pausedTask.at("id")}});
        Json decision={{"message",""},{"actions",Json::array({{{"id","saved-read"},{"tool","read_resource"},{"args",{{"sourceId",id}}}}})},
            {"state",resumeCalls==1?"continue":"completed"}};
        gangyi::AIResult result;result.model="缓存恢复夹具";result.finishReason="stop";result.content=decision.dump();sink(result.content);return result;
    });
    const auto cacheBefore=gangyi::libraryUsage(db).at("cacheHits").get<int>();
    resumeGuard.process(db,pausedTask.at("id"));
    assert(gangyi::agentView(db,access,pausedTask.at("id"))["status"]=="paused");
    gangyi::agentControl(db,access,{{"command","resume"},{"taskId",pausedTask.at("id")}});
    resumeGuard.process(db,pausedTask.at("id"));
    assert(gangyi::agentView(db,access,pausedTask.at("id"))["status"]=="ready"&&resumeCalls==3);
    assert(gangyi::libraryUsage(db).at("cacheHits").get<int>()==cacheBefore+1&&gangyi::libraryUsage(db)["fetchRequests"]==1);
    const auto longSource=gangyi::libraryWrite(db,{{"url","https://example.org/long-document"},{"title","长篇原文缓存"}});
    const std::string longId=longSource.at("id");Json paragraphs=Json::array();
    for(int index=1;index<=40;++index)paragraphs.push_back({{"id","s"+std::to_string(index)},{"text",index==35?"list.copy creates a shallow copy.":"正文段落 "+std::to_string(index)}});
    const Json longSnapshot={{"sourceId",longId},{"segments",paragraphs}};
    assert(db.saveMyAIRecord({"long-snapshot","library-snapshot",longSnapshot.dump(),"2026-10-14",0},0));
    auto longValue=Json::parse(db.myAIRecord(longId)->payload);longValue["snapshotId"]="long-snapshot";longValue["readStatus"]="read";
    assert(db.saveMyAIRecord({longId,"library-source",longValue.dump(),"2026-10-14",0},longSource.at("version")));
    const auto originalLong=db.myAIRecord("long-snapshot")->payload;
    const auto queryRead=[&](const std::string& requestId,Json args){
        args["sourceId"]=longId;
        const auto current=gangyi::agentSubmit(db,access,{{"type","library_research"},{"goal","定位快照后半部分"},{"requestId",requestId}});
        gangyi::LearningAgent reader(":memory:",[&](const auto&,const auto& sink){
            gangyi::AIResult response;response.model="关键词读取夹具";response.finishReason="stop";
            response.content=Json{{"message",""},{"actions",Json::array({{{"id","keyword-read"},{"tool","read_resource"},{"args",args}}})},{"state","completed"}}.dump();sink(response.content);return response;
        });reader.process(db,current.at("id"));
        return Json::parse(db.getClassroomActivity(current.at("id"))->payload);
    };
    const auto found=queryRead("query-copy",{{"query","COPY"}}).at("completedActions").at("keyword-read").at("result").at("snapshot");
    assert(found["segments"].size()==1&&found["segments"][0]["id"]=="s35"&&found["totalSegments"]==40&&found["matchedSegments"]==1&&found["excerpted"]==true);
    const auto absent=queryRead("query-absent",{{"query","missing phrase"}}).at("completedActions").at("keyword-read").at("result").at("snapshot");
    assert(absent["segments"].empty()&&absent["matchedSegments"]==0&&absent["totalSegments"]==40&&absent["nextOffset"].is_null());
    const auto queryHits=gangyi::libraryUsage(db).at("cacheHits");
    assert(queryRead("query-invalid",{{"query",""}})["status"]=="failed"&&gangyi::libraryUsage(db).at("cacheHits")==queryHits);
    assert(db.myAIRecord("long-snapshot")->payload==originalLong&&gangyi::libraryUsage(db)["fetchRequests"]==1);
    const auto selectedContext=gangyi::libraryContext(db);
    assert(std::any_of(selectedContext["sources"].begin(),selectedContext["sources"].end(),[&](const auto& source){return source["id"]==longId&&source["totalSegments"]==40&&source["excerpted"]==true;}));
    std::cout<<"资料引用、快照、排除、冲突及字幕专项检查通过\n";
}

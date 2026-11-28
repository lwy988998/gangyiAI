// 【逐行批注】 处理条件编译指令：#ifdef NDEBUG。
#ifdef NDEBUG
// 【逐行批注】 执行当前声明或表达式：#undef NDEBUG。
#undef NDEBUG
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 "learning_library.hpp"，为后续实现提供类型或接口声明。
#include "learning_library.hpp"
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"
// 【逐行批注】 引入头文件 "agent_prompt.hpp"，为后续实现提供类型或接口声明。
#include "agent_prompt.hpp"
// 【逐行批注】 引入头文件 "library_revision_checks.hpp"，为后续实现提供类型或接口声明。
#include "library_revision_checks.hpp"
// 【逐行批注】 引入头文件 <cassert>，为后续实现提供类型或接口声明。
#include <cassert>
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 定义函数 runLibraryChecks，执行该函数声明的职责。
void runLibraryChecks() {
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(":memory:"); db.migrate();。
    gangyi::Database db; db.open(":memory:"); db.migrate();
    // 【逐行批注】 调用 assert、libraryDigest 完成本行所需操作：assert(gangyi::libraryDigest("abc")=="ba7816bf8f01cfea414140de5dae222…。
    assert(gangyi::libraryDigest("abc")=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    // 【逐行批注】 调用 assert、libraryPublicURL 完成本行所需操作：assert(!gangyi::libraryPublicURL("file:///C:/Users/private.txt"));。
    assert(!gangyi::libraryPublicURL("file:///C:/Users/private.txt"));
    // 【逐行批注】 调用 assert、libraryPublicURL 完成本行所需操作：assert(!gangyi::libraryPublicURL("http://127.0.0.1/api/my-ai/profile"…。
    assert(!gangyi::libraryPublicURL("http://127.0.0.1/api/my-ai/profile"));
    // 【逐行批注】 调用 assert、libraryPublicURL 完成本行所需操作：assert(!gangyi::libraryPublicURL("https://user:password@example.org/"…。
    assert(!gangyi::libraryPublicURL("https://user:password@example.org/"));
    // 【逐行批注】 调用 assert、libraryPublicURL 完成本行所需操作：assert(!gangyi::libraryPublicURL("http://172.16.2.3/private"));。
    assert(!gangyi::libraryPublicURL("http://172.16.2.3/private"));
    // 【逐行批注】 调用 assert、libraryPublicURL 完成本行所需操作：assert(!gangyi::libraryPublicURL("http://[fc00::1]/private"));。
    assert(!gangyi::libraryPublicURL("http://[fc00::1]/private"));
    // 【逐行批注】 调用 assert、libraryPublicURL 完成本行所需操作：assert(!gangyi::libraryPublicURL("http://localhost./private"));。
    assert(!gangyi::libraryPublicURL("http://localhost./private"));
    // 【逐行批注】 计算右侧结果并写入 assert(gangyi::libraryPublicURL("https://exa…，更新当前运行状态。
    assert(gangyi::libraryPublicURL("https://example.org/Study?Version=A"));
    // 【逐行批注】 计算右侧结果并写入 assert(gangyi::libraryCanonicalURL("https://…，更新当前运行状态。
    assert(gangyi::libraryCanonicalURL("https://Example.org/Study?Version=A#part") == "https://example.org/Study?Version=A");
    // 【逐行批注】 计算右侧结果并写入 source，更新当前运行状态。
    auto source = gangyi::libraryWrite(db, {{"url","https://example.org/Study"},{"title","固定原文"}});
    // 【逐行批注】 计算右侧结果并写入 std::string id，更新当前运行状态。
    const std::string id = source["id"];
    // 【逐行批注】 计算右侧结果并写入 Json first，更新当前运行状态。
    const Json first = {{"sourceId",id},{"checkedAt","2026-10-13T00:00:00Z"},{"publishedAt",nullptr},
        // 【逐行批注】 调用 array 完成本行所需操作：{"segments",Json::array({{{"id","s1"},{"text","分母相同才可以直接相加分子。"}}})}};。
        {"segments",Json::array({{{"id","s1"},{"text","分母相同才可以直接相加分子。"}}})}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"snap-first","library-snapshot",first.dump(…。
    assert(db.saveMyAIRecord({"snap-first","library-snapshot",first.dump(),"2026-10-13",0},0));
    // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
    auto value = Json::parse(db.myAIRecord(id)->payload); value["snapshotId"]="snap-first"; value["readStatus"]="read";
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({id,"library-source",value.dump(),"2026-10-1…。
    assert(db.saveMyAIRecord({id,"library-source",value.dump(),"2026-10-13",0},source["version"]));
    // 【逐行批注】 计算右侧结果并写入 Json citation，更新当前运行状态。
    const Json citation = {{"sourceId",id},{"snapshotId","snap-first"},{"segmentId","s1"},{"quote","分母相同"}};
    // 【逐行批注】 计算右侧结果并写入 auto verified，更新当前运行状态。
    const auto verified=gangyi::libraryValidateCitations(db,Json::array({citation}));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(verified[0]["publishedAt"].is_null()&&verified[0]["locator"]["…。
    assert(verified[0]["publishedAt"].is_null()&&verified[0]["locator"]["text"]==first["segments"][0]["text"]);
    // 【逐行批注】 计算右侧结果并写入 bool badQuote，更新当前运行状态。
    bool badQuote=false;try{auto invalid=citation;invalid["quote"]="原文并不存在这句话";gangyi::libraryValidateCitations(db,Json::array({invalid}));}catch(...){badQuote=true;}assert(badQuote);
    // 【逐行批注】 计算右侧结果并写入 bool badSegment，更新当前运行状态。
    bool badSegment=false;try{auto invalid=citation;invalid["segmentId"]="p99";gangyi::libraryValidateCitations(db,Json::array({invalid}));}catch(...){badSegment=true;}assert(badSegment);
    // 【逐行批注】 计算右侧结果并写入 detail，更新当前运行状态。
    auto detail=gangyi::libraryRead(db,{{"id",id}});
    // 【逐行批注】 调用 libraryWrite 完成本行所需操作：gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"fa…。
    gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"favorite",true}});
    // 【逐行批注】 计算右侧结果并写入 bool conflict，更新当前运行状态。
    bool conflict=false;try{gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"favorite",false}});}catch(...){conflict=true;}assert(conflict);
    // 【逐行批注】 计算右侧结果并写入 detail，更新当前运行状态。
    detail=gangyi::libraryRead(db,{{"id",id}});
    // 【逐行批注】 调用 libraryWrite 完成本行所需操作：gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"ex…。
    gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"excluded",true}});
    // 【逐行批注】 调用 assert、libraryContext 完成本行所需操作：assert(gangyi::libraryContext(db)["sources"].empty());。
    assert(gangyi::libraryContext(db)["sources"].empty());
    // 【逐行批注】 调用 assert、libraryValidateCitations、array 完成本行所需操作：assert(!gangyi::libraryValidateCitations(db,Json::array({citation}),f…。
    assert(!gangyi::libraryValidateCitations(db,Json::array({citation}),false).empty());
    // 【逐行批注】 计算右侧结果并写入 bool excluded，更新当前运行状态。
    bool excluded=false;try{gangyi::libraryValidateCitations(db,Json::array({citation}));}catch(...){excluded=true;}assert(excluded);
    // 【逐行批注】 计算右侧结果并写入 auto captions，更新当前运行状态。
    const auto captions=gangyi::libraryParseCaptions("WEBVTT\n\n00:01.000 --> 00:03.000\n先通分。\n\n00:03.000 --> 00:05.000\n再相加。\n","text/vtt");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(captions["segments"].size()==2&&captions["segments"][0]["start…。
    assert(captions["segments"].size()==2&&captions["segments"][0]["start"]==1.0&&captions["segments"][1]["end"]==5.0);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(captions["automatic"].is_null());。
    assert(captions["automatic"].is_null());
    // 【逐行批注】 计算右侧结果并写入 auto jsonCaptions，更新当前运行状态。
    const auto jsonCaptions=gangyi::libraryParseCaptions(R"({"body":[{"from":2,"to":4,"content":"第二段字幕"}]})","application/json");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(jsonCaptions["status"]=="read"&&jsonCaptions["segments"][0]["i…。
    assert(jsonCaptions["status"]=="read"&&jsonCaptions["segments"][0]["id"]=="t1");
    // 【逐行批注】 调用 assert、libraryParseCaptions 完成本行所需操作：assert(gangyi::libraryParseCaptions("WEBVTT\n\n00:07.000 --> 00:03.00…。
    assert(gangyi::libraryParseCaptions("WEBVTT\n\n00:07.000 --> 00:03.000\n错误时间范围\n","text/vtt")["segments"].empty());
    // 【逐行批注】 调用 assert、libraryParseCaptions 完成本行所需操作：assert(gangyi::libraryParseCaptions(R"({"body":[{"from":-1,"to":"inva…。
    assert(gangyi::libraryParseCaptions(R"({"body":[{"from":-1,"to":"invalid","content":"不能用于引用"}]})","application/json")["segments"].empty());
    // 【逐行批注】 调用 assert、libraryParseCaptions 完成本行所需操作：assert(gangyi::libraryParseCaptions(R"({"automatic":true,"body":[{"fr…。
    assert(gangyi::libraryParseCaptions(R"({"automatic":true,"body":[{"from":0,"to":1,"content":"自动字幕"}]})","application/json")["automatic"]==true);
    // 【逐行批注】 调用 assert、libraryParseCaptions、string 完成本行所需操作：assert(gangyi::libraryParseCaptions("WEBVTT\n\n"+std::string(400,'9')…。
    assert(gangyi::libraryParseCaptions("WEBVTT\n\n"+std::string(400,'9')+":00:01.000 --> 00:02.000\n超长时间不可引用\n","text/vtt")["segments"].empty());
    // 【逐行批注】 计算右侧结果并写入 gangyi::LibraryDocument transcript; transcri…，更新当前运行状态。
    gangyi::LibraryDocument transcript; transcript.mime="text/plain"; transcript.body="公开文字稿第一段。\n\n第二段不伪造时间戳。";
    // 【逐行批注】 计算右侧结果并写入 auto parsedTranscript，更新当前运行状态。
    const auto parsedTranscript=gangyi::libraryParseDocument(transcript);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(parsedTranscript["segments"].size()==2&&!parsedTranscript["seg…。
    assert(parsedTranscript["segments"].size()==2&&!parsedTranscript["segments"][0].contains("start"));
    // 【逐行批注】 调用 assert、libraryUsage 完成本行所需操作：assert(gangyi::libraryUsage(db)["searchRequests"]==0&&gangyi::library…。
    assert(gangyi::libraryUsage(db)["searchRequests"]==0&&gangyi::libraryUsage(db)["tokens"].is_null());
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"partial-read","library-usage",Json{{"kind"…。
    assert(db.saveMyAIRecord({"partial-read","library-usage",Json{{"kind","fetch"},{"status","partial"},{"attempts",1},{"cached",false}}.dump(),"2026-10-14",0},0));
    // 【逐行批注】 调用 assert、libraryUsage 完成本行所需操作：assert(gangyi::libraryUsage(db)["partialReads"]==1&&gangyi::libraryUs…。
    assert(gangyi::libraryUsage(db)["partialReads"]==1&&gangyi::libraryUsage(db)["failures"]==0&&gangyi::libraryUsage(db)["fetchRequests"]==1);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.listInteractions().empty()&&db.listMastery().empty());。
    assert(db.listInteractions().empty()&&db.listMastery().empty());
    // 【逐行批注】 计算右侧结果并写入 Json originalContext，更新当前运行状态。
    Json originalContext = {{"event", {{"type", "library_research"}}}, {"personalCenter", {{"preferences", "先解释再练习"}}},
        // 【逐行批注】 调用 array 完成本行所需操作：{"savedLibrary", {{"sources", Json::array({{{"id", id}, {"snapshotId"…。
        {"savedLibrary", {{"sources", Json::array({{{"id", id}, {"snapshotId", "snap-first"}, {"publishedAt", nullptr},
            // 【逐行批注】 执行当前声明或表达式：{"choice", {{"manual", true}, {"state", "selected"}}}, {"segments", first["segments"…。
            {"choice", {{"manual", true}, {"state", "selected"}}}, {"segments", first["segments"]}}})}}}};
    // 【逐行批注】 计算右侧结果并写入 auto promptContext，更新当前运行状态。
    const auto promptContext=gangyi::agentPromptContext(originalContext,false);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(promptContext["personalCenter"]==originalContext["personalCent…。
    assert(promptContext["personalCenter"]==originalContext["personalCenter"]&&promptContext["savedLibrary"]["sources"][0]["choice"]["manual"]==true);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!promptContext["savedLibrary"]["sources"][0].contains("segment…。
    assert(!promptContext["savedLibrary"]["sources"][0].contains("segments")&&originalContext["savedLibrary"]["sources"][0].contains("segments"));
    // 【逐行批注】 计算右侧结果并写入 Json readResult，更新当前运行状态。
    const Json readResult={{"snapshot",first}};
    // 【逐行批注】 计算右侧结果并写入 Json completed，更新当前运行状态。
    const Json completed={{"read-1",{{"encoded",Json{{"tool","read_resource"},{"args",{{"sourceId",id}}}}.dump()},{"result",readResult}}}};
    // 【逐行批注】 计算右侧结果并写入 Json messages，更新当前运行状态。
    const Json messages=Json::array({{{"role","user"},{"content",Json{{"toolResults",Json::array({{{"id","read-1"},{"result",readResult}}})}}.dump()}},
        // 【逐行批注】 执行当前声明或表达式：{{"role","user"},{"content",Json{{"completedActions",completed},{"toolError","原始错误必须…。
        {{"role","user"},{"content",Json{{"completedActions",completed},{"toolError","原始错误必须保留"}}.dump()}}});
    // 【逐行批注】 计算右侧结果并写入 auto compactHistory，更新当前运行状态。
    const auto compactHistory=gangyi::agentPromptHistory(messages,completed,{"read_resource"});
    // 【逐行批注】 计算右侧结果并写入 auto firstMessage，更新当前运行状态。
    const auto firstMessage=Json::parse(compactHistory[0].at("content").get<std::string>()),lastMessage=Json::parse(compactHistory[1].at("content").get<std::string>());
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(firstMessage["toolResults"][0]["result"]==readResult&&lastMess…。
    assert(firstMessage["toolResults"][0]["result"]==readResult&&lastMessage["completedActions"]["read-1"]["result"]["unchangedFrom"]=="read-1");
    // 【逐行批注】 计算右侧结果并写入 Json restoredMessages，更新当前运行状态。
    const Json restoredMessages=Json::array({{{"role","user"},{"content",Json{{"savedActions",completed}}.dump()}},
        // 【逐行批注】 执行当前声明或表达式：{{"role","user"},{"content",Json{{"completedActions",completed},{"toolError","输出被截断"…。
        {{"role","user"},{"content",Json{{"completedActions",completed},{"toolError","输出被截断"}}.dump()}}});
    // 【逐行批注】 计算右侧结果并写入 auto restoredHistory，更新当前运行状态。
    const auto restoredHistory=gangyi::agentPromptHistory(restoredMessages,completed,{"read_resource"});
    // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(restoredHistory[0].at("content").get<std::string>(…。
    assert(Json::parse(restoredHistory[0].at("content").get<std::string>())["savedActions"]["read-1"]["result"]==readResult);
    // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(restoredHistory[1].at("content").get<std::string>(…。
    assert(Json::parse(restoredHistory[1].at("content").get<std::string>())["completedActions"]["read-1"]["result"]["unchangedFrom"]=="read-1");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(restoredMessages[1].at("content").get<std::string>().find("输出被…。
    assert(restoredMessages[1].at("content").get<std::string>().find("输出被截断")!=std::string::npos);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(lastMessage["toolError"]=="原始错误必须保留"&&messages[1].at("content"…。
    assert(lastMessage["toolError"]=="原始错误必须保留"&&messages[1].at("content").get<std::string>().find("snapshot")!=std::string::npos);
    // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess access{"local-profile", {}};。
    const gangyi::AgentAccess access{"local-profile", {}};
    // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
    const auto task=gangyi::agentSubmit(db,access,{{"type","library_research"},{"goal","核验假引用拒绝"},{"requestId","invalid-citation"}});
    // 【逐行批注】 定义函数 guard，执行该函数声明的职责。
    gangyi::LearningAgent guard(":memory:",[&](const auto&, const auto& sink){
        // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result;result.model，更新当前运行状态。
        gangyi::AIResult result;result.model="引用拒绝夹具";result.finishReason="stop";
        // 【逐行批注】 计算右侧结果并写入 result.content，更新当前运行状态。
        result.content=Json{{"message","UNVERIFIED-CITATION-MUST-NOT-APPEAR"},{"materialOrigin","source_based"},
            // 【逐行批注】 调用 array 完成本行所需操作：{"citations",Json::array({{{"sourceId","missing-source"},{"snapshotId…。
            {"citations",Json::array({{{"sourceId","missing-source"},{"snapshotId","missing-snapshot"},{"segmentId","s1"}}})},
            // 【逐行批注】 调用 array、sink 完成本行所需操作：{"actions",Json::array()},{"state","completed"}}.dump();sink(result.c…。
            {"actions",Json::array()},{"state","completed"}}.dump();sink(result.content);return result;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 执行当前声明或表达式：guard.process(db,task.at("id"));。
    guard.process(db,task.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto rejected，更新当前运行状态。
    const auto rejected=gangyi::agentView(db,access,task.at("id"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(rejected["status"]=="failed"&&rejected.dump().find("UNVERIFIED…。
    assert(rejected["status"]=="failed" && rejected.value("message", "").empty());
    assert(std::any_of(rejected.at("events").begin(), rejected.at("events").end(), [](const auto& event) {
        return event.value("type", "") == "stream_status" && event.value("state", "") == "interrupted";
    }));
    // 【逐行批注】 计算右侧结果并写入 auto noSourceTask，更新当前运行状态。
    const auto noSourceTask=gangyi::agentSubmit(db,access,{{"type","chat"},{"text","用原文讲解"},{"requestId","invalid-citation-without-source"}});
    // 【逐行批注】 调用 assert、libraryContext 完成本行所需操作：assert(gangyi::libraryContext(db)["sources"].empty());。
    assert(gangyi::libraryContext(db)["sources"].empty());
    // 【逐行批注】 执行当前声明或表达式：guard.process(db,noSourceTask.at("id"));。
    guard.process(db,noSourceTask.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto noSourceRejected，更新当前运行状态。
    const auto noSourceRejected=gangyi::agentView(db,access,noSourceTask.at("id"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(noSourceRejected["status"]=="failed"&&noSourceRejected.dump().…。
    assert(noSourceRejected["status"]=="failed" && noSourceRejected.value("message", "").empty());
    assert(std::any_of(noSourceRejected.at("events").begin(), noSourceRejected.at("events").end(), [](const auto& event) {
        return event.value("type", "") == "stream_status" && event.value("state", "") == "interrupted";
    }));
    // 【逐行批注】 计算右侧结果并写入 detail，更新当前运行状态。
    detail=gangyi::libraryRead(db,{{"id",id}});
    // 【逐行批注】 调用 libraryWrite 完成本行所需操作：gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"ex…。
    gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"excluded",false}});
    // 省略offset与显式offset=0都经过实际缓存读取；只精简给模型的副本，不改原快照和凭据。
    // 【逐行批注】 计算右侧结果并写入 auto duplicateTask，更新当前运行状态。
    const auto duplicateTask=gangyi::agentSubmit(db,access,{{"type","library_research"},{"goal","核验相同回执去重"},{"requestId","library-identical-results"}});
    // 【逐行批注】 计算右侧结果并写入 auto snapshotBefore，更新当前运行状态。
    const auto snapshotBefore=db.myAIRecord("snap-first")->payload;
    // 【逐行批注】 计算右侧结果并写入 int duplicateCalls，更新当前运行状态。
    int duplicateCalls=0;std::size_t receiptBytesBefore=0,receiptBytesAfter=0;
    // 【逐行批注】 定义函数 duplicateGuard，执行该函数声明的职责。
    gangyi::LearningAgent duplicateGuard(":memory:",[&](const auto& options,const auto& sink){
        // 【逐行批注】 计算右侧结果并写入 bool firstCall，更新当前运行状态。
        const bool firstCall=++duplicateCalls==1;
        // 【逐行批注】 执行当前声明或表达式：if(!firstCall) {。
        if(!firstCall) {
            // 【逐行批注】 计算右侧结果并写入 auto persisted，更新当前运行状态。
            const auto persisted=Json::parse(db.getClassroomActivity(duplicateTask.at("id"))->payload);
            // 【逐行批注】 计算右侧结果并写入 auto& saved，更新当前运行状态。
            const auto& saved=persisted.at("completedActions");
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(saved.at("default-offset").at("result")==saved.at("explicit-of…。
            assert(saved.at("default-offset").at("result")==saved.at("explicit-offset").at("result"));
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(saved.at("default-offset").at("encoded")!=saved.at("explicit-o…。
            assert(saved.at("default-offset").at("encoded")!=saved.at("explicit-offset").at("encoded"));
            // 【逐行批注】 计算右侧结果并写入 bool checked，更新当前运行状态。
            bool checked=false;
            // 【逐行批注】 按循环规则重复处理：for(const auto& message:options.messages)if(message.role=="user") {。
            for(const auto& message:options.messages)if(message.role=="user") {
                // 【逐行批注】 计算右侧结果并写入 auto content，更新当前运行状态。
                const auto content=Json::parse(message.content,nullptr,false);
                // 【逐行批注】 执行当前声明或表达式：if(!content.is_object()||!content.contains("toolResults"))continue;。
                if(!content.is_object()||!content.contains("toolResults"))continue;
                // 【逐行批注】 计算右侧结果并写入 auto& results，更新当前运行状态。
                const auto& results=content.at("toolResults");assert(results.size()==2);
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(results[0].at("result")==saved.at("default-offset").at("result…。
                assert(results[0].at("result")==saved.at("default-offset").at("result"));
                // 【逐行批注】 调用 assert、Json 完成本行所需操作：assert(results[1].at("result")==Json({{"unchangedFrom","default-offse…。
                assert(results[1].at("result")==Json({{"unchangedFrom","default-offset"}}));
                // 【逐行批注】 计算右侧结果并写入 restored，更新当前运行状态。
                auto restored=content;restored["toolResults"][1]["result"]=results[0].at("result");
                // 【逐行批注】 计算右侧结果并写入 auto original，更新当前运行状态。
                const auto original=Json::parse(persisted.at("messages").back().at("content").get<std::string>());
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(restored==original);。
                assert(restored==original);
                // 【逐行批注】 计算右侧结果并写入 receiptBytesBefore，更新当前运行状态。
                receiptBytesBefore=original.dump().size();receiptBytesAfter=content.dump().size();
                // 【逐行批注】 调用 assert 完成本行所需操作：assert(receiptBytesAfter<receiptBytesBefore);checked=true;。
                assert(receiptBytesAfter<receiptBytesBefore);checked=true;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(checked);。
            assert(checked);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 Json decision，更新当前运行状态。
        Json decision={{"message",""},{"actions",firstCall?Json::array({
            // 【逐行批注】 执行当前声明或表达式：{{"id","default-offset"},{"tool","read_resource"},{"args",{{"sourceId",id}}}},。
            {{"id","default-offset"},{"tool","read_resource"},{"args",{{"sourceId",id}}}},
            // 【逐行批注】 执行当前声明或表达式：{{"id","explicit-offset"},{"tool","read_resource"},{"args",{{"sourceId",id},{"offset…。
            {{"id","explicit-offset"},{"tool","read_resource"},{"args",{{"sourceId",id},{"offset",0}}}}
        // 【逐行批注】 调用 array 完成本行所需操作：}):Json::array()},{"state",firstCall?"continue":"completed"}};。
        }):Json::array()},{"state",firstCall?"continue":"completed"}};
        // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result;result.model，更新当前运行状态。
        gangyi::AIResult result;result.model="同回执去重夹具";result.finishReason="stop";result.content=decision.dump();sink(result.content);return result;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 执行当前声明或表达式：duplicateGuard.process(db,duplicateTask.at("id"));。
    duplicateGuard.process(db,duplicateTask.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db,access,duplicateTask.at("id"))["status"]=…。
    assert(gangyi::agentView(db,access,duplicateTask.at("id"))["status"]=="ready"&&duplicateCalls==2);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.myAIRecord("snap-first")->payload==snapshotBefore&&db.listM…。
    assert(db.myAIRecord("snap-first")->payload==snapshotBefore&&db.listMastery().empty());
    // 【逐行批注】 执行当前声明或表达式：std::cout<<"[prompt-dedup] "<<Json{{"receiptBytesBefore",receiptBytesBefore},{"recei…。
    std::cout<<"[prompt-dedup] "<<Json{{"receiptBytesBefore",receiptBytesBefore},{"receiptBytesAfter",receiptBytesAfter},
        // 【逐行批注】 执行当前声明或表达式：{"roundTripPreserved",true},{"rawReceiptsPreserved",true},{"paidRequests",0}}.dump()…。
        {"roundTripPreserved",true},{"rawReceiptsPreserved",true},{"paidRequests",0}}.dump()<<'\n';
    // 【逐行批注】 计算右侧结果并写入 auto pausedTask，更新当前运行状态。
    const auto pausedTask=gangyi::agentSubmit(db,access,{{"type","library_research"},{"goal","验证已读步骤暂停恢复"},{"requestId","library-resume"}});
    // 【逐行批注】 计算右侧结果并写入 int resumeCalls，更新当前运行状态。
    int resumeCalls=0;
    // 【逐行批注】 定义函数 resumeGuard，执行该函数声明的职责。
    gangyi::LearningAgent resumeGuard(":memory:",[&](const auto&,const auto& sink){
        // 【逐行批注】 执行当前声明或表达式：++resumeCalls;。
        ++resumeCalls;
        // 【逐行批注】 调用 agentControl 完成本行所需操作：if(resumeCalls==2)gangyi::agentControl(db,access,{{"command","pause"}…。
        if(resumeCalls==2)gangyi::agentControl(db,access,{{"command","pause"},{"taskId",pausedTask.at("id")}});
        // 【逐行批注】 计算右侧结果并写入 Json decision，更新当前运行状态。
        Json decision={{"message",""},{"actions",Json::array({{{"id","saved-read"},{"tool","read_resource"},{"args",{{"sourceId",id}}}}})},
            // 【逐行批注】 执行当前声明或表达式：{"state",resumeCalls==1?"continue":"completed"}};。
            {"state",resumeCalls==1?"continue":"completed"}};
        // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult result;result.model，更新当前运行状态。
        gangyi::AIResult result;result.model="缓存恢复夹具";result.finishReason="stop";result.content=decision.dump();sink(result.content);return result;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 计算右侧结果并写入 auto cacheBefore，更新当前运行状态。
    const auto cacheBefore=gangyi::libraryUsage(db).at("cacheHits").get<int>();
    // 【逐行批注】 执行当前声明或表达式：resumeGuard.process(db,pausedTask.at("id"));。
    resumeGuard.process(db,pausedTask.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db,access,pausedTask.at("id"))["status"]=="p…。
    assert(gangyi::agentView(db,access,pausedTask.at("id"))["status"]=="paused");
    // 【逐行批注】 调用 agentControl 完成本行所需操作：gangyi::agentControl(db,access,{{"command","resume"},{"taskId",paused…。
    gangyi::agentControl(db,access,{{"command","resume"},{"taskId",pausedTask.at("id")}});
    // 【逐行批注】 执行当前声明或表达式：resumeGuard.process(db,pausedTask.at("id"));。
    resumeGuard.process(db,pausedTask.at("id"));
    // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(db,access,pausedTask.at("id"))["status"]=="r…。
    assert(gangyi::agentView(db,access,pausedTask.at("id"))["status"]=="ready"&&resumeCalls==3);
    // 【逐行批注】 调用 assert、libraryUsage 完成本行所需操作：assert(gangyi::libraryUsage(db).at("cacheHits").get<int>()==cacheBefo…。
    assert(gangyi::libraryUsage(db).at("cacheHits").get<int>()==cacheBefore+1&&gangyi::libraryUsage(db)["fetchRequests"]==1);
    // 【逐行批注】 计算右侧结果并写入 auto longSource，更新当前运行状态。
    const auto longSource=gangyi::libraryWrite(db,{{"url","https://example.org/long-document"},{"title","长篇原文缓存"}});
    // 【逐行批注】 计算右侧结果并写入 std::string longId，更新当前运行状态。
    const std::string longId=longSource.at("id");Json paragraphs=Json::array();
    // 【逐行批注】 按循环规则重复处理：for(int index=1;index<=40;++index)paragraphs.push_back({{"id","s"+std::to_…。
    for(int index=1;index<=40;++index)paragraphs.push_back({{"id","s"+std::to_string(index)},{"text",index==35?"list.copy creates a shallow copy.":"正文段落 "+std::to_string(index)}});
    // 【逐行批注】 计算右侧结果并写入 Json longSnapshot，更新当前运行状态。
    const Json longSnapshot={{"sourceId",longId},{"segments",paragraphs}};
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"long-snapshot","library-snapshot",longSnap…。
    assert(db.saveMyAIRecord({"long-snapshot","library-snapshot",longSnapshot.dump(),"2026-10-14",0},0));
    // 【逐行批注】 计算右侧结果并写入 longValue，更新当前运行状态。
    auto longValue=Json::parse(db.myAIRecord(longId)->payload);longValue["snapshotId"]="long-snapshot";longValue["readStatus"]="read";
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({longId,"library-source",longValue.dump(),"2…。
    assert(db.saveMyAIRecord({longId,"library-source",longValue.dump(),"2026-10-14",0},longSource.at("version")));
    // 【逐行批注】 计算右侧结果并写入 auto originalLong，更新当前运行状态。
    const auto originalLong=db.myAIRecord("long-snapshot")->payload;
    // 【逐行批注】 计算右侧结果并写入 auto queryRead，更新当前运行状态。
    const auto queryRead=[&](const std::string& requestId,Json args){
        // 【逐行批注】 计算右侧结果并写入 args["sourceId"]，更新当前运行状态。
        args["sourceId"]=longId;
        // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
        const auto current=gangyi::agentSubmit(db,access,{{"type","library_research"},{"goal","定位快照后半部分"},{"requestId",requestId}});
        // 【逐行批注】 定义函数 reader，执行该函数声明的职责。
        gangyi::LearningAgent reader(":memory:",[&](const auto&,const auto& sink){
            // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult response;response.model，更新当前运行状态。
            gangyi::AIResult response;response.model="关键词读取夹具";response.finishReason="stop";
            // 【逐行批注】 计算右侧结果并写入 response.content，更新当前运行状态。
            response.content=Json{{"message",""},{"actions",Json::array({{{"id","keyword-read"},{"tool","read_resource"},{"args",args}}})},{"state","completed"}}.dump();sink(response.content);return response;
        // 【逐行批注】 执行当前声明或表达式：});reader.process(db,current.at("id"));。
        });reader.process(db,current.at("id"));
        // 【逐行批注】 结束当前函数并返回结果：return Json::parse(db.getClassroomActivity(current.at("id"))->payload…。
        return Json::parse(db.getClassroomActivity(current.at("id"))->payload);
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
    const auto found=queryRead("query-copy",{{"query","COPY"}}).at("completedActions").at("keyword-read").at("result").at("snapshot");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(found["segments"].size()==1&&found["segments"][0]["id"]=="s35"…。
    assert(found["segments"].size()==1&&found["segments"][0]["id"]=="s35"&&found["totalSegments"]==40&&found["matchedSegments"]==1&&found["excerpted"]==true);
    // 【逐行批注】 计算右侧结果并写入 auto absent，更新当前运行状态。
    const auto absent=queryRead("query-absent",{{"query","missing phrase"}}).at("completedActions").at("keyword-read").at("result").at("snapshot");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(absent["segments"].empty()&&absent["matchedSegments"]==0&&abse…。
    assert(absent["segments"].empty()&&absent["matchedSegments"]==0&&absent["totalSegments"]==40&&absent["nextOffset"].is_null());
    // 【逐行批注】 计算右侧结果并写入 auto queryHits，更新当前运行状态。
    const auto queryHits=gangyi::libraryUsage(db).at("cacheHits");
    // 【逐行批注】 调用 assert、queryRead、libraryUsage 完成本行所需操作：assert(queryRead("query-invalid",{{"query",""}})["status"]=="failed"&…。
    assert(queryRead("query-invalid",{{"query",""}})["status"]=="failed"&&gangyi::libraryUsage(db).at("cacheHits")==queryHits);
    // 【逐行批注】 调用 assert、libraryUsage 完成本行所需操作：assert(db.myAIRecord("long-snapshot")->payload==originalLong&&gangyi:…。
    assert(db.myAIRecord("long-snapshot")->payload==originalLong&&gangyi::libraryUsage(db)["fetchRequests"]==1);
    // 【逐行批注】 计算右侧结果并写入 auto selectedContext，更新当前运行状态。
    const auto selectedContext=gangyi::libraryContext(db);
    // 【逐行批注】 调用 assert、any_of 完成本行所需操作：assert(std::any_of(selectedContext["sources"].begin(),selectedContext…。
    assert(std::any_of(selectedContext["sources"].begin(),selectedContext["sources"].end(),[&](const auto& source){return source["id"]==longId&&source["totalSegments"]==40&&source["excerpted"]==true;}));
    // 原文更新后仍按快照ID补读旧引用；回查不联网，也不绕过排除状态。
    // 【逐行批注】 计算右侧结果并写入 refreshed，更新当前运行状态。
    auto refreshed = Json::parse(db.myAIRecord(longId)->payload);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"new-snapshot","library-snapshot",Json{{"so…。
    assert(db.saveMyAIRecord({"new-snapshot","library-snapshot",Json{{"sourceId",longId},{"segments",{{{"id","new"},{"text","新版原文"}}}}}.dump(),"now",0},0));
    // 【逐行批注】 计算右侧结果并写入 refreshed["snapshotId"]，更新当前运行状态。
    refreshed["snapshotId"]="new-snapshot";
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({longId,"library-source",refreshed.dump(),"n…。
    assert(db.saveMyAIRecord({longId,"library-source",refreshed.dump(),"now",0},db.myAIRecord(longId)->version));
    // 【逐行批注】 执行当前声明或表达式：db.setProfileMeta("library-revision","100");。
    db.setProfileMeta("library-revision","100");
    // 【逐行批注】 计算右侧结果并写入 auto oldRead，更新当前运行状态。
    const auto oldRead=queryRead("old-snapshot-read",{{"snapshotId","long-snapshot"},{"query","COPY"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(oldRead.at("completedActions").at("keyword-read").at("result")…。
    assert(oldRead.at("completedActions").at("keyword-read").at("result").at("snapshot").at("segments")[0].at("id")=="s35");
    // 【逐行批注】 调用 assert、queryRead 完成本行所需操作：assert(queryRead("old-snapshot-refresh-conflict",{{"snapshotId","long…。
    assert(queryRead("old-snapshot-refresh-conflict",{{"snapshotId","long-snapshot"},{"refresh",true}}).at("status")=="failed");
    // 【逐行批注】 调用 assert、queryRead 完成本行所需操作：assert(queryRead("wrong-source-snapshot",{{"snapshotId","snap-first"}…。
    assert(queryRead("wrong-source-snapshot",{{"snapshotId","snap-first"}}).at("status")=="failed");
    // 【逐行批注】 计算右侧结果并写入 refreshed["excluded"]，更新当前运行状态。
    refreshed["excluded"]=true;
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({longId,"library-source",refreshed.dump(),"n…。
    assert(db.saveMyAIRecord({longId,"library-source",refreshed.dump(),"now",0},db.myAIRecord(longId)->version));
    // 【逐行批注】 调用 assert、queryRead 完成本行所需操作：assert(queryRead("excluded-old-snapshot",{{"snapshotId","long-snapsho…。
    assert(queryRead("excluded-old-snapshot",{{"snapshotId","long-snapshot"}}).at("status")=="failed");
    // 【逐行批注】 调用 assert、libraryUsage 完成本行所需操作：assert(db.myAIRecord("long-snapshot")->payload==originalLong&&gangyi:…。
    assert(db.myAIRecord("long-snapshot")->payload==originalLong&&gangyi::libraryUsage(db)["fetchRequests"]==1);
    // 【逐行批注】 开始当前代码块。
    {
        // 两份都选用时保留AI顺序；改变顺序后同步影响页面和课堂，手动选择仍优先。
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database orderedDb;orderedDb.open(":memory:");orderedDb.migrate();。
        gangyi::Database orderedDb;orderedDb.open(":memory:");orderedDb.migrate();
        // 【逐行批注】 计算右侧结果并写入 auto tutorial，更新当前运行状态。
        const auto tutorial=gangyi::libraryWrite(orderedDb,{{"url","https://example.org/tutorial"},{"title","教程"}});
        // 【逐行批注】 计算右侧结果并写入 auto specification，更新当前运行状态。
        const auto specification=gangyi::libraryWrite(orderedDb,{{"url","https://example.org/spec"},{"title","接口规范"}});
        // 【逐行批注】 执行当前声明或表达式：const gangyi::AgentAccess orderedAccess{"ordering-profile",{}};。
        const gangyi::AgentAccess orderedAccess{"ordering-profile",{}};
        // 【逐行批注】 计算右侧结果并写入 auto reorder，更新当前运行状态。
        const auto reorder=[&](const std::string& requestId,Json ids){
            // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
            const auto current=gangyi::agentSubmit(orderedDb,orderedAccess,{{"type","library_research"},{"goal","验证推荐顺序"},{"requestId",requestId}});
            // 【逐行批注】 定义函数 selector，执行该函数声明的职责。
            gangyi::LearningAgent selector(":memory:",[&](const auto&,const auto& sink){
                // 【逐行批注】 计算右侧结果并写入 Json items，更新当前运行状态。
                Json items=Json::array();for(const auto& sourceId:ids)items.push_back({{"sourceId",sourceId},{"state","selected"},{"reason","顺序契约测试"}});
                // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult response;response.model，更新当前运行状态。
                gangyi::AIResult response;response.model="推荐排序夹具";response.finishReason="stop";
                // 【逐行批注】 计算右侧结果并写入 response.content，更新当前运行状态。
                response.content=Json{{"message",""},{"actions",Json::array({{{"id","select-order"},{"tool","select_resources"},{"args",{{"items",items}}}}})},{"state","completed"}}.dump();
                // 【逐行批注】 调用 sink 完成本行所需操作：sink(response.content);return response;。
                sink(response.content);return response;
            // 【逐行批注】 执行当前声明或表达式：});selector.process(orderedDb,current.at("id"));。
            });selector.process(orderedDb,current.at("id"));
            // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(gangyi::agentView(orderedDb,orderedAccess,current.at("id"))["s…。
            assert(gangyi::agentView(orderedDb,orderedAccess,current.at("id"))["status"]=="ready");
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 调用 reorder、array 完成本行所需操作：reorder("beginner-order",Json::array({tutorial["id"],specification["i…。
        reorder("beginner-order",Json::array({tutorial["id"],specification["id"]}));
        // 【逐行批注】 调用 assert、libraryRead 完成本行所需操作：assert(gangyi::libraryRead(orderedDb)["items"][0]["id"]==tutorial["id…。
        assert(gangyi::libraryRead(orderedDb)["items"][0]["id"]==tutorial["id"]);
        // 【逐行批注】 调用 reorder、array 完成本行所需操作：reorder("advanced-order",Json::array({specification["id"],tutorial["i…。
        reorder("advanced-order",Json::array({specification["id"],tutorial["id"]}));
        // 【逐行批注】 调用 assert、libraryRead 完成本行所需操作：assert(gangyi::libraryRead(orderedDb)["items"][0]["id"]==specificatio…。
        assert(gangyi::libraryRead(orderedDb)["items"][0]["id"]==specification["id"]);
        // 【逐行批注】 调用 assert、libraryContext 完成本行所需操作：assert(gangyi::libraryContext(orderedDb)["sources"][0]["id"]==specifi…。
        assert(gangyi::libraryContext(orderedDb)["sources"][0]["id"]==specification["id"]);
        // 【逐行批注】 计算右侧结果并写入 auto tutorialDetail，更新当前运行状态。
        const auto tutorialDetail=gangyi::libraryRead(orderedDb,{{"id",tutorial["id"]}});
        // 【逐行批注】 调用 libraryWrite 完成本行所需操作：gangyi::libraryWrite(orderedDb,{{"id",tutorial["id"]},{"version",tuto…。
        gangyi::libraryWrite(orderedDb,{{"id",tutorial["id"]},{"version",tutorialDetail["version"]},{"state","selected"}});
        // 【逐行批注】 调用 assert、libraryRead 完成本行所需操作：assert(gangyi::libraryRead(orderedDb)["items"][0]["id"]==tutorial["id…。
        assert(gangyi::libraryRead(orderedDb)["items"][0]["id"]==tutorial["id"]);
        // 【逐行批注】 调用 reorder、array 完成本行所需操作：reorder("manual-priority",Json::array({specification["id"],tutorial["…。
        reorder("manual-priority",Json::array({specification["id"],tutorial["id"]}));
        // 【逐行批注】 调用 assert、libraryRead 完成本行所需操作：assert(gangyi::libraryRead(orderedDb)["items"][0]["id"]==tutorial["id…。
        assert(gangyi::libraryRead(orderedDb)["items"][0]["id"]==tutorial["id"]);
        // 【逐行批注】 计算右侧结果并写入 auto manualDetail，更新当前运行状态。
        const auto manualDetail=gangyi::libraryRead(orderedDb,{{"id",tutorial["id"]}});
        // 【逐行批注】 调用 libraryWrite 完成本行所需操作：gangyi::libraryWrite(orderedDb,{{"id",tutorial["id"]},{"version",manu…。
        gangyi::libraryWrite(orderedDb,{{"id",tutorial["id"]},{"version",manualDetail["version"]},{"state","excluded"}});
        // 【逐行批注】 调用 reorder、array 完成本行所需操作：reorder("excluded-priority",Json::array({tutorial["id"],specification…。
        reorder("excluded-priority",Json::array({tutorial["id"],specification["id"]}));
        // 【逐行批注】 调用 assert、libraryRead 完成本行所需操作：assert(gangyi::libraryRead(orderedDb)["items"][0]["id"]==specificatio…。
        assert(gangyi::libraryRead(orderedDb)["items"][0]["id"]==specification["id"]);
        // 【逐行批注】 调用 assert、libraryRead 完成本行所需操作：assert(gangyi::libraryRead(orderedDb,{{"id",tutorial["id"]}})["choice…。
        assert(gangyi::libraryRead(orderedDb,{{"id",tutorial["id"]}})["choice"]["state"]=="excluded");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const bool stillLimited : {false, true}) {。
    for (const bool stillLimited : {false, true}) {
        // 【逐行批注】 计算右侧结果并写入 auto limitTask，更新当前运行状态。
        const auto limitTask=gangyi::agentSubmit(db,access,{{"type","library_research"},{"goal","验证截断后恢复"},
            // 【逐行批注】 执行当前声明或表达式：{"requestId",stillLimited?"output-limit-stop":"output-limit-recover"}});。
            {"requestId",stillLimited?"output-limit-stop":"output-limit-recover"}});
        // 【逐行批注】 计算右侧结果并写入 int limitCalls，更新当前运行状态。
        int limitCalls=0;
        // 【逐行批注】 定义函数 recovery，执行该函数声明的职责。
        gangyi::LearningAgent recovery(":memory:",[&](const auto& options,const auto& sink){
            // 【逐行批注】 执行当前声明或表达式：++limitCalls;。
            ++limitCalls;
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.maxTokens==(limitCalls==1?8192:16384));。
            assert(options.maxTokens==(limitCalls==1?8192:16384));
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(options.timeoutMs==240000);。
            assert(options.timeoutMs==240000);
            // 【逐行批注】 执行当前声明或表达式：if(limitCalls==1||stillLimited) {。
            if(limitCalls==1||stillLimited) {
                // 【逐行批注】 调用 sink 完成本行所需操作：sink("{\"message\":\"未校验的截断片段");。
                sink("{\"message\":\"未校验的截断片段");
                // 【逐行批注】 检测到错误情形时抛出异常：throw gangyi::AIClientError("output_limit","服务商报告length");。
                throw gangyi::AIClientError("output_limit","服务商报告length");
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 gangyi::AIResult reply;reply.model，更新当前运行状态。
            gangyi::AIResult reply;reply.model="长度恢复夹具";reply.finishReason="stop";
            // 【逐行批注】 计算右侧结果并写入 reply.content，更新当前运行状态。
            reply.content=Json{{"message","恢复成功"},{"materialOrigin","ai_supplement"},{"actions",Json::array()},{"state","completed"}}.dump();
            // 【逐行批注】 调用 sink 完成本行所需操作：sink(reply.content);return reply;。
            sink(reply.content);return reply;
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 执行当前声明或表达式：recovery.process(db,limitTask.at("id"));。
        recovery.process(db,limitTask.at("id"));
        // 【逐行批注】 计算右侧结果并写入 auto limitView，更新当前运行状态。
        const auto limitView=gangyi::agentView(db,access,limitTask.at("id"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(limitCalls==2&&limitView["status"]==(stillLimited?"failed":"re…。
        assert(limitCalls==2&&limitView["status"]==(stillLimited?"failed":"ready"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(limitView.value("message", "").find("未校验的截断片段")==std::string::npos);。
        assert(limitView.value("message", "").find("未校验的截断片段")==std::string::npos);
        // 【逐行批注】 计算右侧结果并写入 auto limitSaved，更新当前运行状态。
        const auto limitSaved=Json::parse(db.getClassroomActivity(limitTask.at("id"))->payload);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(limitSaved["responseTokenLimit"]==16384);。
        assert(limitSaved["responseTokenLimit"]==16384);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 runLibraryRevisionChecks 完成本行所需操作：runLibraryRevisionChecks();。
    runLibraryRevisionChecks();
    // 【逐行批注】 执行当前声明或表达式：std::cout<<"资料引用、快照、排除、冲突及字幕专项检查通过\n";。
    std::cout<<"资料引用、快照、排除、冲突及字幕专项检查通过\n";
// 【逐行批注】 结束当前代码块。
}

#ifdef NDEBUG
#undef NDEBUG
#endif
#include "my_ai.hpp"
#include "my_ai_context.hpp"
#include "ai_activity.hpp"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <limits>
#include <sqlite3.h>
#include <chrono>
using Json = nlohmann::json;
int main() {
    gangyi::Database db; db.open(":memory:"); db.migrate();
    auto profile = gangyi::myAIRead(db, "profile"); assert(profile["version"] == 0);
    auto saved = gangyi::myAIWrite(db, "profile", {{"version", 0}, {"nickname", "小林"}, {"goal", "理解高中数学"}});
    assert(saved["version"] == 1);
    bool conflict = false;
    try { gangyi::myAIWrite(db, "profile", {{"version", 0}, {"nickname", "旧窗口"}}); } catch (...) { conflict = true; }
    assert(conflict && gangyi::myAIRead(db,"profile")["nickname"] == "小林");
    auto prefs=gangyi::myAIWrite(db,"preferences",{{"version",0},{"instructions","先举例，再讲定义"},{"autoMemory",false}});
    bool disabled=false;try{gangyi::myAIWrite(db,"memories",{{"content","推测风格"},{"source","inferred"}});}catch(...){disabled=true;}
    assert(disabled&&gangyi::myAIPersonalContext(db)["preferences"]["instructions"]=="先举例，再讲定义");
    auto memory = gangyi::myAIWrite(db,"memories", {{"content","喜欢先举生活中的例子"}, {"source","manual"}});
    assert(gangyi::myAIPersonalContext(db)["memories"].size() == 1);
    gangyi::myAIWrite(db,"memories", {{"id",memory["id"]}, {"version",memory["version"]}, {"deleted",true}});
    assert(gangyi::myAIPersonalContext(db)["memories"].empty());
    auto chat = gangyi::myAIWrite(db,"chats", {{"title","数学安排"}});
    Json message = {{"requestId","one"}, {"text","请调整讲解风格"}};
    auto first = gangyi::myAIChatMessage(db, chat["id"], message);
    assert(gangyi::myAIChatMessage(db, chat["id"], message) == first);
    auto detail = gangyi::myAIRead(db,"chat",{{"id",chat["id"]}}); assert(detail["messages"].size() == 1);
    auto draft = gangyi::myAIWrite(db,"chat",{{"id",chat["id"]},{"version",detail["version"]},{"draft","保留草稿"},{"scroll",18}});
    assert(draft["draft"] == "保留草稿");
    db.insert(gangyi::Course{"course",{}, {},"函数","deep","函数课程",{},"ai","active","2026-10-12","2026-10-12"});
    auto proposal = gangyi::myAIProposal(db,{{"kind","delete_course"},{"target","course"},{"reason","用户要求删除"}},"delete-one");
    assert(db.getCourse("course"));
    gangyi::myAIConfirm(db,proposal["id"],{{"decision","cancel"},{"version",proposal["version"]}});
    assert(db.getCourse("course"));
    auto second = gangyi::myAIProposal(db,{{"kind","delete_course"},{"target","course"},{"reason","用户再次要求删除"}},"delete-two");
    auto done = gangyi::myAIConfirm(db,second["id"],{{"decision","confirm"},{"version",second["version"]}});
    assert(!db.getCourse("course")); assert(gangyi::myAIConfirm(db,second["id"],{{"decision","confirm"},{"version",second["version"]}}) == done);
    auto usage = gangyi::myAIRead(db,"usage"); assert(usage["calls"] == 0 && usage["tokens"].is_null());
    assert(usage["tokenBreakdown"]["input"]["tokens"].is_null() && usage["tokenBreakdown"]["input"]["knownCalls"] == 0);
    assert(db.upsert(gangyi::SubjectMastery{"数学","真实零分","[]","练习概念","fixture","ready","2026-10-12",0,1}));
    auto report=gangyi::myAIRead(db,"reports");assert(report["subjects"][0]["score"]==0);
    db.insert(gangyi::Course{"time-course",{}, {},"函数","deep","函数课程",{},"ai","active","2026-10-12","2026-10-12"});
    const auto clock=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    assert(db.saveMyAIRecord({"study-lease","lease",Json{{"owner","window-a"},{"last",clock-10000}}.dump(),"2026-10-12",0},0));
    auto other=gangyi::myAIWrite(db,"heartbeat",{{"courseId","time-course"},{"owner","window-b"},{"visible",true},{"recent",true}});assert(other["seconds"]==0);
    auto ending=gangyi::myAIWrite(db,"heartbeat",{{"courseId","time-course"},{"owner","window-a"},{"visible",true},{"recent",true},{"ending",true},{"day","2026-10-12"}});assert(ending["seconds"]>=10);
    const auto recorded=gangyi::myAIRead(db,"reports")["studySeconds"];
    gangyi::myAIWrite(db,"heartbeat",{{"courseId","time-course"},{"owner","window-a"},{"visible",false},{"recent",false}});
    assert(gangyi::myAIRead(db,"reports")["studySeconds"]==recorded);
    bool missing=false;try{gangyi::myAIWrite(db,"heartbeat",{{"courseId","missing"},{"owner","x"},{"visible",false},{"recent",false}});}catch(...){missing=true;}assert(missing);
    const auto upgrade=std::filesystem::temp_directory_path()/("gangyi-my-ai-upgrade-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".db");
    {gangyi::Database old;old.open(upgrade.string());old.migrate();old.setProfileMeta("kept","原有档案");}
    sqlite3* legacy=nullptr;assert(sqlite3_open(upgrade.string().c_str(),&legacy)==SQLITE_OK);assert(sqlite3_exec(legacy,"DROP TABLE MyAIRecord;PRAGMA user_version=6",nullptr,nullptr,nullptr)==SQLITE_OK);sqlite3_close(legacy);
    {gangyi::Database next;next.open(upgrade.string());next.migrate();assert(next.profileMeta("kept")=="原有档案");assert(next.myAIRecords("chat").empty());}
    sqlite3* upgraded=nullptr;sqlite3_stmt* version=nullptr;sqlite3_open(upgrade.string().c_str(),&upgraded);sqlite3_prepare_v2(upgraded,"PRAGMA user_version",-1,&version,nullptr);assert(sqlite3_step(version)==SQLITE_ROW&&sqlite3_column_int(version,0)==7);sqlite3_finalize(version);sqlite3_close(upgraded);
    assert(std::filesystem::exists(upgrade.string()+".pre-v7.db"));std::filesystem::remove(upgrade);std::filesystem::remove(upgrade.string()+".pre-v7.db");
    // 同一任务在控制记录丢失后重新发起请求，两条实际请求的用量必须分别保留。
    const auto usageFolder=std::filesystem::temp_directory_path()/("gangyi-usage-retry-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(usageFolder);
    {
        const auto usagePath=usageFolder/"usage.db",controlPath=usageFolder/"control.json";
        gangyi::Database usageDb;usageDb.open(usagePath.u8string());usageDb.migrate();
        gangyi::AIActivity::configure(controlPath.u8string());gangyi::AIActivity::configureUsage(usagePath.u8string());
        gangyi::AIActivityInfo info;info.taskId="persisted-same-task";info.source="用量恢复夹具";
        std::string firstReceipt;
        auto id=gangyi::AIActivity::begin(info,"fixture-model");assert(gangyi::AIActivity::requestStarted(id, &firstReceipt));
        const Json detailedUsage={{"prompt_tokens",12},{"completion_tokens",5},{"total_tokens",17},
            {"prompt_cache_hit_tokens",8},{"prompt_cache_miss_tokens",4},
            {"prompt_tokens_details",{{"cached_tokens",8}}},{"completion_tokens_details",{{"reasoning_tokens",2}}}};
        gangyi::AIActivity::requestFinished(id,"completed",detailedUsage,"fixture-model");
        const auto original=usageDb.myAIRecords("usage");assert(original.size()==1);
        assert(Json::parse(original[0].payload).at("tokens")==detailedUsage);
        std::filesystem::remove(controlPath);
        gangyi::AIActivity::configure(controlPath.u8string());
        id=gangyi::AIActivity::begin(info,"fixture-model");assert(gangyi::AIActivity::requestStarted(id));
        gangyi::AIActivity::requestFinished(id,"failed",nullptr,"fixture-model");
        // 重复完成回执更新当前请求，不另算一次请求。
        gangyi::AIActivity::requestFinished(id,"failed",nullptr,"fixture-model");
        const auto beforeStale=usageDb.myAIRecords("usage");
        gangyi::AIActivity::requestFinished(id,"completed",detailedUsage,"fixture-model",firstReceipt);
        const auto recorded=usageDb.myAIRecords("usage");assert(recorded.size()==2);
        assert(recorded[0].payload==beforeStale[0].payload && recorded[1].payload==beforeStale[1].payload);
        assert(usageDb.myAIRecord(original[0].id)->payload==original[0].payload);
        const auto statistics=gangyi::myAIRead(usageDb,"usage");
        assert(statistics["calls"]==2&&statistics["tokens"]==17);
        for (const auto& item : statistics["items"]) assert(!item.contains("queueMs") && !item.contains("firstContentMs") && !item.contains("completionMs"));
        assert(statistics["tokenBreakdown"]["cachedInput"]["tokens"]==8 && statistics["tokenBreakdown"]["cachedInput"]["knownCalls"]==1);
        assert(statistics["tokenBreakdown"]["reasoning"]["tokens"]==2 && statistics["knownTokenCalls"]==1);
        const Json timedTask = {{"queueMs", 11}, {"firstContentMs", 42}, {"completionMs", 67}, {"privateTimingOrigin", "不得输出"}};
        assert(usageDb.upsert(gangyi::ClassroomActivity{info.taskId, "", "agent-task", timedTask.dump(), "now", 0, 0}));
        const auto timed = gangyi::myAIRead(usageDb, "usage");
        for (const auto& item : timed["items"]) {
            assert(item.at("queueMs") == 11 && item.at("firstContentMs") == 42 && item.at("completionMs") == 67);
            assert(!item.contains("privateTimingOrigin"));
        }
        // 旧格式、真实零值、无效值和筛选后的覆盖率均保持真实，双协议缓存不相加。
        const auto addUsage = [&](const char* key, const Json& tokens, const char* model) {
            assert(usageDb.saveMyAIRecord({key,"usage",Json{{"tokens",tokens},{"model",model},{"day","2026-10-17"}}.dump(),"2026-10-17",0},0));
        };
        addUsage("partial",{{"prompt_tokens",0},{"prompt_tokens_details",{{"cached_tokens",0}}},{"completion_tokens_details",{{"reasoning_tokens",0}}}},"partial");
        const auto partial=gangyi::myAIRead(usageDb,"usage",{{"model","partial"},{"from","2026-10-17"},{"to","2026-10-17"}});
        assert(partial["tokens"].is_null() && partial["tokenBreakdown"]["cachedInput"]["tokens"]==0 && partial["tokenBreakdown"]["cachedInput"]["knownCalls"]==1);
        addUsage("invalid",{{"total_tokens",-1},{"prompt_tokens",true},{"completion_tokens","7"},{"prompt_cache_hit_tokens",1.5},{"completion_tokens_details",{{"reasoning_tokens",-2}}}},"invalid");
        const auto invalid=gangyi::myAIRead(usageDb,"usage",{{"model","invalid"}});
        assert(invalid["tokens"].is_null() && invalid["knownTokenCalls"]==0 && invalid["tokenBreakdown"]["input"]["tokens"].is_null());
        addUsage("huge-a",{{"total_tokens",std::numeric_limits<long long>::max()}},"huge");
        addUsage("huge-b",{{"total_tokens",1}},"huge");
        const auto huge=gangyi::myAIRead(usageDb,"usage",{{"model","huge"}});
        assert(huge["tokens"].is_null() && huge["knownTokenCalls"]==2);
        assert(gangyi::myAIRead(usageDb,"usage")["tokenBreakdown"]["cachedInput"]["tokens"]==8);
        assert(std::any_of(recorded.begin(),recorded.end(),[](const auto& row){const auto item=Json::parse(row.payload);return item["status"]=="failed"&&item["tokens"].is_null();}));
        gangyi::AIActivity::configureUsage("");gangyi::AIActivity::configure("");
    }
    std::filesystem::remove_all(usageFolder);
    // 仅创建内存时钟，不创建模型请求：1024项边界优先淘汰终态，再淘汰最旧起点。
    constexpr int clockCapacity = 1024;
    for (int i = 0; i < clockCapacity; ++i) {
        const auto id = "clock-bound-fixture-" + std::to_string(i);
        gangyi::AIActivity::taskQueued(id); gangyi::AIActivity::taskRunning(id);
    }
    const auto firstClock = gangyi::AIActivity::taskTiming("clock-bound-fixture-0");
    assert(firstClock.contains("queueMs"));
    assert(db.upsert(gangyi::ClassroomActivity{"clock-bound-fixture-0", "", "agent-task", firstClock.dump(), "now", 0, 0}));
    gangyi::AIActivity::taskFinished("clock-bound-fixture-1023");
    gangyi::AIActivity::taskQueued("clock-bound-extra-one");
    assert(gangyi::AIActivity::taskTiming("clock-bound-fixture-1023").empty());
    assert(gangyi::AIActivity::taskTiming("clock-bound-fixture-0") == firstClock);
    gangyi::AIActivity::taskQueued("clock-bound-extra-two");
    gangyi::AIActivity::taskRunning("clock-bound-fixture-0");
    gangyi::AIActivity::taskContent("clock-bound-fixture-0");
    gangyi::AIActivity::taskFinished("clock-bound-fixture-0");
    assert(gangyi::AIActivity::taskTiming("clock-bound-fixture-0").empty());
    assert(db.getClassroomActivity("clock-bound-fixture-0")->payload == firstClock.dump());
    std::cout << "个人中心版本、聊天、记忆、用量恢复和确认操作通过\n";
}

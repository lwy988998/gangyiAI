#ifdef NDEBUG
#undef NDEBUG
#endif
#include "my_ai.hpp"
#include "my_ai_context.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
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
    std::cout << "个人中心版本、聊天、记忆和确认操作通过\n";
}

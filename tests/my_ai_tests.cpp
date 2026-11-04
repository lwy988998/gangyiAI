// 【逐行批注】 处理条件编译指令：#ifdef NDEBUG。
#ifdef NDEBUG
// 【逐行批注】 执行当前声明或表达式：#undef NDEBUG。
#undef NDEBUG
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 "my_ai.hpp"，为后续实现提供类型或接口声明。
#include "my_ai.hpp"
// 【逐行批注】 引入头文件 "my_ai_context.hpp"，为后续实现提供类型或接口声明。
#include "my_ai_context.hpp"
// 【逐行批注】 引入头文件 "ai_activity.hpp"，为后续实现提供类型或接口声明。
#include "ai_activity.hpp"
// 【逐行批注】 引入头文件 "db_schema_version.hpp"，为后续实现提供类型或接口声明。
#include "db_schema_version.hpp"
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <cassert>，为后续实现提供类型或接口声明。
#include <cassert>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <limits>，为后续实现提供类型或接口声明。
#include <limits>
// 【逐行批注】 引入头文件 <sqlite3.h>，为后续实现提供类型或接口声明。
#include <sqlite3.h>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
    // 【逐行批注】 执行当前声明或表达式：gangyi::Database db; db.open(":memory:"); db.migrate();。
    gangyi::Database db; db.open(":memory:"); db.migrate();
    // 【逐行批注】 计算右侧结果并写入 profile，更新当前运行状态。
    auto profile = gangyi::myAIRead(db, "profile"); assert(profile["version"] == 0);
    // 【逐行批注】 计算右侧结果并写入 saved，更新当前运行状态。
    auto saved = gangyi::myAIWrite(db, "profile", {{"version", 0}, {"nickname", "小林"}, {"goal", "理解高中数学"}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(saved["version"] == 1);。
    assert(saved["version"] == 1);
    // 【逐行批注】 计算右侧结果并写入 bool conflict，更新当前运行状态。
    bool conflict = false;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { gangyi::myAIWrite(db, "profile", {{"version", 0}, {"nickname", "旧窗口"}}); } catch (...) { conflict = true; }
    // 【逐行批注】 调用 assert、myAIRead 完成本行所需操作：assert(conflict && gangyi::myAIRead(db,"profile")["nickname"] == "小林"…。
    assert(conflict && gangyi::myAIRead(db,"profile")["nickname"] == "小林");
    // 【逐行批注】 计算右侧结果并写入 prefs，更新当前运行状态。
    auto prefs=gangyi::myAIWrite(db,"preferences",{{"version",0},{"instructions","先举例，再讲定义"},{"autoMemory",false}});
    // 【逐行批注】 计算右侧结果并写入 bool disabled，更新当前运行状态。
    bool disabled=false;try{gangyi::myAIWrite(db,"memories",{{"content","推测风格"},{"source","inferred"}});}catch(...){disabled=true;}
    // 【逐行批注】 调用 assert、myAIPersonalContext 完成本行所需操作：assert(disabled&&gangyi::myAIPersonalContext(db)["preferences"]["inst…。
    assert(disabled&&gangyi::myAIPersonalContext(db)["preferences"]["instructions"]=="先举例，再讲定义");
    // 【逐行批注】 计算右侧结果并写入 memory，更新当前运行状态。
    auto memory = gangyi::myAIWrite(db,"memories", {{"content","喜欢先举生活中的例子"}, {"source","manual"}});
    // 【逐行批注】 调用 assert、myAIPersonalContext 完成本行所需操作：assert(gangyi::myAIPersonalContext(db)["memories"].size() == 1);。
    assert(gangyi::myAIPersonalContext(db)["memories"].size() == 1);
    // 【逐行批注】 调用 myAIWrite 完成本行所需操作：gangyi::myAIWrite(db,"memories", {{"id",memory["id"]}, {"version",mem…。
    gangyi::myAIWrite(db,"memories", {{"id",memory["id"]}, {"version",memory["version"]}, {"deleted",true}});
    // 【逐行批注】 调用 assert、myAIPersonalContext 完成本行所需操作：assert(gangyi::myAIPersonalContext(db)["memories"].empty());。
    assert(gangyi::myAIPersonalContext(db)["memories"].empty());
    // 【逐行批注】 计算右侧结果并写入 chat，更新当前运行状态。
    auto chat = gangyi::myAIWrite(db,"chats", {{"title","数学安排"}});
    // 【逐行批注】 计算右侧结果并写入 Json message，更新当前运行状态。
    Json message = {{"requestId","one"}, {"text","请调整讲解风格"}};
    // 【逐行批注】 计算右侧结果并写入 first，更新当前运行状态。
    auto first = gangyi::myAIChatMessage(db, chat["id"], message);
    // 【逐行批注】 调用 assert、myAIChatMessage 完成本行所需操作：assert(gangyi::myAIChatMessage(db, chat["id"], message) == first);。
    assert(gangyi::myAIChatMessage(db, chat["id"], message) == first);
    // 【逐行批注】 计算右侧结果并写入 detail，更新当前运行状态。
    auto detail = gangyi::myAIRead(db,"chat",{{"id",chat["id"]}}); assert(detail["messages"].size() == 1);
    // 【逐行批注】 计算右侧结果并写入 draft，更新当前运行状态。
    auto draft = gangyi::myAIWrite(db,"chat",{{"id",chat["id"]},{"version",detail["version"]},{"draft","保留草稿"},{"scroll",18}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(draft["draft"] == "保留草稿");。
    assert(draft["draft"] == "保留草稿");
    // 【逐行批注】 执行当前声明或表达式：db.insert(gangyi::Course{"course",{}, {},"函数","deep","函数课程",{},"ai","active","2026-1…。
    db.insert(gangyi::Course{"course",{}, {},"函数","deep","函数课程",{},"ai","active","2026-10-12","2026-10-12"});
    // 【逐行批注】 计算右侧结果并写入 proposal，更新当前运行状态。
    auto proposal = gangyi::myAIProposal(db,{{"kind","delete_course"},{"target","course"},{"reason","用户要求删除"}},"delete-one");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse("course"));。
    assert(db.getCourse("course"));
    // 【逐行批注】 调用 myAIConfirm 完成本行所需操作：gangyi::myAIConfirm(db,proposal["id"],{{"decision","cancel"},{"versio…。
    gangyi::myAIConfirm(db,proposal["id"],{{"decision","cancel"},{"version",proposal["version"]}});
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getCourse("course"));。
    assert(db.getCourse("course"));
    // 【逐行批注】 计算右侧结果并写入 second，更新当前运行状态。
    auto second = gangyi::myAIProposal(db,{{"kind","delete_course"},{"target","course"},{"reason","用户再次要求删除"}},"delete-two");
    // 【逐行批注】 计算右侧结果并写入 done，更新当前运行状态。
    auto done = gangyi::myAIConfirm(db,second["id"],{{"decision","confirm"},{"version",second["version"]}});
    // 【逐行批注】 调用 assert、myAIConfirm 完成本行所需操作：assert(!db.getCourse("course")); assert(gangyi::myAIConfirm(db,second…。
    assert(!db.getCourse("course")); assert(gangyi::myAIConfirm(db,second["id"],{{"decision","confirm"},{"version",second["version"]}}) == done);
    // 【逐行批注】 计算右侧结果并写入 usage，更新当前运行状态。
    auto usage = gangyi::myAIRead(db,"usage"); assert(usage["calls"] == 0 && usage["tokens"].is_null());
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(usage["tokenBreakdown"]["input"]["tokens"].is_null() && usage[…。
    assert(usage["tokenBreakdown"]["input"]["tokens"].is_null() && usage["tokenBreakdown"]["input"]["knownCalls"] == 0);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(gangyi::SubjectMastery{"数学","真实零分","[]","练习概念","fixt…。
    assert(db.upsert(gangyi::SubjectMastery{"数学","真实零分","[]","练习概念","fixture","ready","2026-10-12",0,1}));
    // 【逐行批注】 计算右侧结果并写入 report，更新当前运行状态。
    auto report=gangyi::myAIRead(db,"reports");assert(report["subjects"][0]["score"]==0);
    // 【逐行批注】 执行当前声明或表达式：db.insert(gangyi::Course{"time-course",{}, {},"函数","deep","函数课程",{},"ai","active","2…。
    db.insert(gangyi::Course{"time-course",{}, {},"函数","deep","函数课程",{},"ai","active","2026-10-12","2026-10-12"});
    // 【逐行批注】 计算右侧结果并写入 auto clock，更新当前运行状态。
    const auto clock=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.saveMyAIRecord({"study-lease","lease",Json{{"owner","window…。
    assert(db.saveMyAIRecord({"study-lease","lease",Json{{"owner","window-a"},{"last",clock-10000}}.dump(),"2026-10-12",0},0));
    // 【逐行批注】 计算右侧结果并写入 other，更新当前运行状态。
    auto other=gangyi::myAIWrite(db,"heartbeat",{{"courseId","time-course"},{"owner","window-b"},{"visible",true},{"recent",true}});assert(other["seconds"]==0);
    // 【逐行批注】 计算右侧结果并写入 ending，更新当前运行状态。
    auto ending=gangyi::myAIWrite(db,"heartbeat",{{"courseId","time-course"},{"owner","window-a"},{"visible",true},{"recent",true},{"ending",true},{"day","2026-10-12"}});assert(ending["seconds"]>=10);
    // 【逐行批注】 计算右侧结果并写入 auto recorded，更新当前运行状态。
    const auto recorded=gangyi::myAIRead(db,"reports")["studySeconds"];
    // 【逐行批注】 调用 myAIWrite 完成本行所需操作：gangyi::myAIWrite(db,"heartbeat",{{"courseId","time-course"},{"owner"…。
    gangyi::myAIWrite(db,"heartbeat",{{"courseId","time-course"},{"owner","window-a"},{"visible",false},{"recent",false}});
    // 【逐行批注】 调用 assert、myAIRead 完成本行所需操作：assert(gangyi::myAIRead(db,"reports")["studySeconds"]==recorded);。
    assert(gangyi::myAIRead(db,"reports")["studySeconds"]==recorded);
    // 【逐行批注】 计算右侧结果并写入 bool missing，更新当前运行状态。
    bool missing=false;try{gangyi::myAIWrite(db,"heartbeat",{{"courseId","missing"},{"owner","x"},{"visible",false},{"recent",false}});}catch(...){missing=true;}assert(missing);
    // 【逐行批注】 计算右侧结果并写入 auto upgrade，更新当前运行状态。
    const auto upgrade=std::filesystem::temp_directory_path()/("gangyi-my-ai-upgrade-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".db");
    // 【逐行批注】 执行当前声明或表达式：{gangyi::Database old;old.open(upgrade.string());old.migrate();old.setProfileMeta("k…。
    {gangyi::Database old;old.open(upgrade.string());old.migrate();old.setProfileMeta("kept","原有档案");}
    // 【逐行批注】 计算右侧结果并写入 sqlite3* legacy，更新当前运行状态。
    sqlite3* legacy=nullptr;assert(sqlite3_open(upgrade.string().c_str(),&legacy)==SQLITE_OK);assert(sqlite3_exec(legacy,"DROP TABLE MyAIRecord;PRAGMA user_version=6",nullptr,nullptr,nullptr)==SQLITE_OK);sqlite3_close(legacy);
    // 【逐行批注】 调用 assert 完成本行所需操作：{gangyi::Database next;next.open(upgrade.string());next.migrate();ass…。
    {gangyi::Database next;next.open(upgrade.string());next.migrate();assert(next.profileMeta("kept")=="原有档案");assert(next.myAIRecords("chat").empty());}
    // 【逐行批注】 计算右侧结果并写入 sqlite3* upgraded，更新当前运行状态。
    sqlite3* upgraded=nullptr;sqlite3_stmt* version=nullptr;sqlite3_open(upgrade.string().c_str(),&upgraded);sqlite3_prepare_v2(upgraded,"PRAGMA user_version",-1,&version,nullptr);assert(sqlite3_step(version)==SQLITE_ROW&&sqlite3_column_int(version,0)==gangyi::kDatabaseSchemaVersion);sqlite3_finalize(version);sqlite3_close(upgraded);
    // 【逐行批注】 调用 assert、exists、remove 完成本行所需操作：assert(std::filesystem::exists(upgrade.string()+".pre-v7.db"));std::f…。
    assert(std::filesystem::exists(upgrade.string()+".pre-v7.db"));std::filesystem::remove(upgrade);std::filesystem::remove(upgrade.string()+".pre-v7.db");
    // 同一任务在控制记录丢失后重新发起请求，两条实际请求的用量必须分别保留。
    // 【逐行批注】 计算右侧结果并写入 auto usageFolder，更新当前运行状态。
    const auto usageFolder=std::filesystem::temp_directory_path()/("gangyi-usage-retry-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(usageFolder);。
    std::filesystem::create_directories(usageFolder);
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 auto usagePath，更新当前运行状态。
        const auto usagePath=usageFolder/"usage.db",controlPath=usageFolder/"control.json";
        // 【逐行批注】 执行当前声明或表达式：gangyi::Database usageDb;usageDb.open(usagePath.u8string());usageDb.migrate();。
        gangyi::Database usageDb;usageDb.open(usagePath.u8string());usageDb.migrate();
        // 【逐行批注】 调用 configure、configureUsage 完成本行所需操作：gangyi::AIActivity::configure(controlPath.u8string());gangyi::AIActiv…。
        gangyi::AIActivity::configure(controlPath.u8string());gangyi::AIActivity::configureUsage(usagePath.u8string());
        // 【逐行批注】 计算右侧结果并写入 gangyi::AIActivityInfo info;info.taskId，更新当前运行状态。
        gangyi::AIActivityInfo info;info.taskId="persisted-same-task";info.source="用量恢复夹具";
        // 【逐行批注】 执行当前声明或表达式：std::string firstReceipt;。
        std::string firstReceipt;
        // 【逐行批注】 计算右侧结果并写入 id，更新当前运行状态。
        auto id=gangyi::AIActivity::begin(info,"fixture-model");assert(gangyi::AIActivity::requestStarted(id, &firstReceipt));
        // 【逐行批注】 计算右侧结果并写入 Json detailedUsage，更新当前运行状态。
        const Json detailedUsage={{"prompt_tokens",12},{"completion_tokens",5},{"total_tokens",17},
            // 【逐行批注】 执行当前声明或表达式：{"prompt_cache_hit_tokens",8},{"prompt_cache_miss_tokens",4},。
            {"prompt_cache_hit_tokens",8},{"prompt_cache_miss_tokens",4},
            // 【逐行批注】 执行当前声明或表达式：{"prompt_tokens_details",{{"cached_tokens",8}}},{"completion_tokens_details",{{"reas…。
            {"prompt_tokens_details",{{"cached_tokens",8}}},{"completion_tokens_details",{{"reasoning_tokens",2}}}};
        // 【逐行批注】 调用 requestFinished 完成本行所需操作：gangyi::AIActivity::requestFinished(id,"completed",detailedUsage,"fix…。
        gangyi::AIActivity::requestFinished(id,"completed",detailedUsage,"fixture-model");
        // 【逐行批注】 计算右侧结果并写入 auto original，更新当前运行状态。
        const auto original=usageDb.myAIRecords("usage");assert(original.size()==1);
        // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(original[0].payload).at("tokens")==detailedUsage);。
        assert(Json::parse(original[0].payload).at("tokens")==detailedUsage);
        // 【逐行批注】 调用 remove 完成本行所需操作：std::filesystem::remove(controlPath);。
        std::filesystem::remove(controlPath);
        // 【逐行批注】 调用 configure 完成本行所需操作：gangyi::AIActivity::configure(controlPath.u8string());。
        gangyi::AIActivity::configure(controlPath.u8string());
        // 【逐行批注】 计算右侧结果并写入 id，更新当前运行状态。
        id=gangyi::AIActivity::begin(info,"fixture-model");assert(gangyi::AIActivity::requestStarted(id));
        // 【逐行批注】 调用 requestFinished 完成本行所需操作：gangyi::AIActivity::requestFinished(id,"failed",nullptr,"fixture-mode…。
        gangyi::AIActivity::requestFinished(id,"failed",nullptr,"fixture-model");
        // 重复完成回执更新当前请求，不另算一次请求。
        // 【逐行批注】 调用 requestFinished 完成本行所需操作：gangyi::AIActivity::requestFinished(id,"failed",nullptr,"fixture-mode…。
        gangyi::AIActivity::requestFinished(id,"failed",nullptr,"fixture-model");
        // 【逐行批注】 计算右侧结果并写入 auto beforeStale，更新当前运行状态。
        const auto beforeStale=usageDb.myAIRecords("usage");
        // 【逐行批注】 调用 requestFinished 完成本行所需操作：gangyi::AIActivity::requestFinished(id,"completed",detailedUsage,"fix…。
        gangyi::AIActivity::requestFinished(id,"completed",detailedUsage,"fixture-model",firstReceipt);
        // 【逐行批注】 计算右侧结果并写入 auto recorded，更新当前运行状态。
        const auto recorded=usageDb.myAIRecords("usage");assert(recorded.size()==2);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(recorded[0].payload==beforeStale[0].payload && recorded[1].pay…。
        assert(recorded[0].payload==beforeStale[0].payload && recorded[1].payload==beforeStale[1].payload);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(usageDb.myAIRecord(original[0].id)->payload==original[0].paylo…。
        assert(usageDb.myAIRecord(original[0].id)->payload==original[0].payload);
        // 【逐行批注】 计算右侧结果并写入 auto statistics，更新当前运行状态。
        const auto statistics=gangyi::myAIRead(usageDb,"usage");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(statistics["calls"]==2&&statistics["tokens"]==17);。
        assert(statistics["calls"]==2&&statistics["tokens"]==17);
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : statistics["items"]) assert(!item.contains("queueM…。
        for (const auto& item : statistics["items"]) assert(!item.contains("queueMs") && !item.contains("firstContentMs") && !item.contains("completionMs"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(statistics["tokenBreakdown"]["cachedInput"]["tokens"]==8 && st…。
        assert(statistics["tokenBreakdown"]["cachedInput"]["tokens"]==8 && statistics["tokenBreakdown"]["cachedInput"]["knownCalls"]==1);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(statistics["tokenBreakdown"]["reasoning"]["tokens"]==2 && stat…。
        assert(statistics["tokenBreakdown"]["reasoning"]["tokens"]==2 && statistics["knownTokenCalls"]==1);
        // 【逐行批注】 计算右侧结果并写入 Json timedTask，更新当前运行状态。
        const Json timedTask = {{"queueMs", 11}, {"firstContentMs", 42}, {"completionMs", 67}, {"privateTimingOrigin", "不得输出"}};
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(usageDb.upsert(gangyi::ClassroomActivity{info.taskId, "", "age…。
        assert(usageDb.upsert(gangyi::ClassroomActivity{info.taskId, "", "agent-task", timedTask.dump(), "now", 0, 0}));
        // 【逐行批注】 计算右侧结果并写入 auto timed，更新当前运行状态。
        const auto timed = gangyi::myAIRead(usageDb, "usage");
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : timed["items"]) {。
        for (const auto& item : timed["items"]) {
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(item.at("queueMs") == 11 && item.at("firstContentMs") == 42 &&…。
            assert(item.at("queueMs") == 11 && item.at("firstContentMs") == 42 && item.at("completionMs") == 67);
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(!item.contains("privateTimingOrigin"));。
            assert(!item.contains("privateTimingOrigin"));
        // 【逐行批注】 结束当前代码块。
        }
        // 旧格式、真实零值、无效值和筛选后的覆盖率均保持真实，双协议缓存不相加。
        // 【逐行批注】 计算右侧结果并写入 auto addUsage，更新当前运行状态。
        const auto addUsage = [&](const char* key, const Json& tokens, const char* model) {
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(usageDb.saveMyAIRecord({key,"usage",Json{{"tokens",tokens},{"m…。
            assert(usageDb.saveMyAIRecord({key,"usage",Json{{"tokens",tokens},{"model",model},{"day","2026-10-17"}}.dump(),"2026-10-17",0},0));
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 调用 addUsage 完成本行所需操作：addUsage("partial",{{"prompt_tokens",0},{"prompt_tokens_details",{{"c…。
        addUsage("partial",{{"prompt_tokens",0},{"prompt_tokens_details",{{"cached_tokens",0}}},{"completion_tokens_details",{{"reasoning_tokens",0}}}},"partial");
        // 【逐行批注】 计算右侧结果并写入 auto partial，更新当前运行状态。
        const auto partial=gangyi::myAIRead(usageDb,"usage",{{"model","partial"},{"from","2026-10-17"},{"to","2026-10-17"}});
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(partial["tokens"].is_null() && partial["tokenBreakdown"]["cach…。
        assert(partial["tokens"].is_null() && partial["tokenBreakdown"]["cachedInput"]["tokens"]==0 && partial["tokenBreakdown"]["cachedInput"]["knownCalls"]==1);
        // 【逐行批注】 调用 addUsage 完成本行所需操作：addUsage("invalid",{{"total_tokens",-1},{"prompt_tokens",true},{"comp…。
        addUsage("invalid",{{"total_tokens",-1},{"prompt_tokens",true},{"completion_tokens","7"},{"prompt_cache_hit_tokens",1.5},{"completion_tokens_details",{{"reasoning_tokens",-2}}}},"invalid");
        // 【逐行批注】 计算右侧结果并写入 auto invalid，更新当前运行状态。
        const auto invalid=gangyi::myAIRead(usageDb,"usage",{{"model","invalid"}});
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(invalid["tokens"].is_null() && invalid["knownTokenCalls"]==0 &…。
        assert(invalid["tokens"].is_null() && invalid["knownTokenCalls"]==0 && invalid["tokenBreakdown"]["input"]["tokens"].is_null());
        // 【逐行批注】 调用 addUsage、max 完成本行所需操作：addUsage("huge-a",{{"total_tokens",std::numeric_limits<long long>::ma…。
        addUsage("huge-a",{{"total_tokens",std::numeric_limits<long long>::max()}},"huge");
        // 【逐行批注】 调用 addUsage 完成本行所需操作：addUsage("huge-b",{{"total_tokens",1}},"huge");。
        addUsage("huge-b",{{"total_tokens",1}},"huge");
        // 【逐行批注】 计算右侧结果并写入 auto huge，更新当前运行状态。
        const auto huge=gangyi::myAIRead(usageDb,"usage",{{"model","huge"}});
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(huge["tokens"].is_null() && huge["knownTokenCalls"]==2);。
        assert(huge["tokens"].is_null() && huge["knownTokenCalls"]==2);
        // 【逐行批注】 调用 assert、myAIRead 完成本行所需操作：assert(gangyi::myAIRead(usageDb,"usage")["tokenBreakdown"]["cachedInp…。
        assert(gangyi::myAIRead(usageDb,"usage")["tokenBreakdown"]["cachedInput"]["tokens"]==8);
        // 【逐行批注】 计算右侧结果并写入 assert(std::any_of(recorded.begin(),recorded…，更新当前运行状态。
        assert(std::any_of(recorded.begin(),recorded.end(),[](const auto& row){const auto item=Json::parse(row.payload);return item["status"]=="failed"&&item["tokens"].is_null();}));
        // 【逐行批注】 调用 configureUsage、configure 完成本行所需操作：gangyi::AIActivity::configureUsage("");gangyi::AIActivity::configure(…。
        gangyi::AIActivity::configureUsage("");gangyi::AIActivity::configure("");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 remove_all 完成本行所需操作：std::filesystem::remove_all(usageFolder);。
    std::filesystem::remove_all(usageFolder);
    // 仅创建内存时钟，不创建模型请求：1024项边界优先淘汰终态，再淘汰最旧起点。
    // 【逐行批注】 计算右侧结果并写入 constexpr int clockCapacity，更新当前运行状态。
    constexpr int clockCapacity = 1024;
    // 【逐行批注】 按循环规则重复处理：for (int i = 0; i < clockCapacity; ++i) {。
    for (int i = 0; i < clockCapacity; ++i) {
        // 【逐行批注】 计算右侧结果并写入 auto id，更新当前运行状态。
        const auto id = "clock-bound-fixture-" + std::to_string(i);
        // 【逐行批注】 调用 taskQueued、taskRunning 完成本行所需操作：gangyi::AIActivity::taskQueued(id); gangyi::AIActivity::taskRunning(i…。
        gangyi::AIActivity::taskQueued(id); gangyi::AIActivity::taskRunning(id);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto firstClock，更新当前运行状态。
    const auto firstClock = gangyi::AIActivity::taskTiming("clock-bound-fixture-0");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(firstClock.contains("queueMs"));。
    assert(firstClock.contains("queueMs"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(gangyi::ClassroomActivity{"clock-bound-fixture-0", "…。
    assert(db.upsert(gangyi::ClassroomActivity{"clock-bound-fixture-0", "", "agent-task", firstClock.dump(), "now", 0, 0}));
    // 【逐行批注】 调用 taskFinished 完成本行所需操作：gangyi::AIActivity::taskFinished("clock-bound-fixture-1023");。
    gangyi::AIActivity::taskFinished("clock-bound-fixture-1023");
    // 【逐行批注】 调用 taskQueued 完成本行所需操作：gangyi::AIActivity::taskQueued("clock-bound-extra-one");。
    gangyi::AIActivity::taskQueued("clock-bound-extra-one");
    // 【逐行批注】 调用 assert、taskTiming 完成本行所需操作：assert(gangyi::AIActivity::taskTiming("clock-bound-fixture-1023").emp…。
    assert(gangyi::AIActivity::taskTiming("clock-bound-fixture-1023").empty());
    // 【逐行批注】 调用 assert、taskTiming 完成本行所需操作：assert(gangyi::AIActivity::taskTiming("clock-bound-fixture-0") == fir…。
    assert(gangyi::AIActivity::taskTiming("clock-bound-fixture-0") == firstClock);
    // 【逐行批注】 调用 taskQueued 完成本行所需操作：gangyi::AIActivity::taskQueued("clock-bound-extra-two");。
    gangyi::AIActivity::taskQueued("clock-bound-extra-two");
    // 【逐行批注】 调用 taskRunning 完成本行所需操作：gangyi::AIActivity::taskRunning("clock-bound-fixture-0");。
    gangyi::AIActivity::taskRunning("clock-bound-fixture-0");
    // 【逐行批注】 调用 taskContent 完成本行所需操作：gangyi::AIActivity::taskContent("clock-bound-fixture-0");。
    gangyi::AIActivity::taskContent("clock-bound-fixture-0");
    // 【逐行批注】 调用 taskFinished 完成本行所需操作：gangyi::AIActivity::taskFinished("clock-bound-fixture-0");。
    gangyi::AIActivity::taskFinished("clock-bound-fixture-0");
    // 【逐行批注】 调用 assert、taskTiming 完成本行所需操作：assert(gangyi::AIActivity::taskTiming("clock-bound-fixture-0").empty(…。
    assert(gangyi::AIActivity::taskTiming("clock-bound-fixture-0").empty());
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.getClassroomActivity("clock-bound-fixture-0")->payload == f…。
    assert(db.getClassroomActivity("clock-bound-fixture-0")->payload == firstClock.dump());
    // 【逐行批注】 执行当前声明或表达式：std::cout << "个人中心版本、聊天、记忆、用量恢复和确认操作通过\n";。
    std::cout << "个人中心版本、聊天、记忆、用量恢复和确认操作通过\n";
// 【逐行批注】 结束当前代码块。
}

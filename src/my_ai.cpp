// 【逐行批注】 引入头文件 "my_ai.hpp"，为后续实现提供类型或接口声明。
#include "my_ai.hpp"
// 【逐行批注】 引入头文件 "my_ai_context.hpp"，为后续实现提供类型或接口声明。
#include "my_ai_context.hpp"
// 【逐行批注】 引入头文件 "profile_service.hpp"，为后续实现提供类型或接口声明。
#include "profile_service.hpp"
// 【逐行批注】 引入头文件 "course_service.hpp"，为后续实现提供类型或接口声明。
#include "course_service.hpp"
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <ctime>，为后续实现提供类型或接口声明。
#include <ctime>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <limits>，为后续实现提供类型或接口声明。
#include <limits>
#include <map>
// 【逐行批注】 引入头文件 <random>，为后续实现提供类型或接口声明。
#include <random>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = MyAIJson;
// 【逐行批注】 定义函数 now，执行该函数声明的职责。
std::string now() {
    // 【逐行批注】 计算右侧结果并写入 clock，更新当前运行状态。
    auto clock=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); std::tm value{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&value,&clock);。
    gmtime_s(&value,&clock);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&clock,&value);。
    gmtime_r(&clock,&value);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 调用 put_time 完成本行所需操作：std::ostringstream out;out<<std::put_time(&value,"%Y-%m-%dT%H:%M:%SZ"…。
    std::ostringstream out;out<<std::put_time(&value,"%Y-%m-%dT%H:%M:%SZ");return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 calendarDay，执行该函数声明的职责。
std::string calendarDay(const std::string& at) {
    // 【逐行批注】 执行当前声明或表达式：if(at.size()<20||at.back()!='Z')return at.substr(0,10);。
    if(at.size()<20||at.back()!='Z')return at.substr(0,10);
    // 【逐行批注】 调用 input、get_time 完成本行所需操作：std::tm utc{},local{};std::istringstream input(at);input>>std::get_ti…。
    std::tm utc{},local{};std::istringstream input(at);input>>std::get_time(&utc,"%Y-%m-%dT%H:%M:%S");if(input.fail())return at.substr(0,10);
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 计算右侧结果并写入 auto clock，更新当前运行状态。
    const auto clock=_mkgmtime(&utc);if(clock<0)return at.substr(0,10);localtime_s(&local,&clock);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 计算右侧结果并写入 auto clock，更新当前运行状态。
    const auto clock=timegm(&utc);if(clock<0)return at.substr(0,10);localtime_r(&clock,&local);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 调用 put_time 完成本行所需操作：std::ostringstream out;out<<std::put_time(&local,"%Y-%m-%d");return o…。
    std::ostringstream out;out<<std::put_time(&local,"%Y-%m-%d");return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 millis，执行该函数声明的职责。
long long millis() {return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();}
// 【逐行批注】 定义函数 id，执行该函数声明的职责。
std::string id(const std::string& prefix) {static thread_local std::mt19937_64 random(std::random_device{}());std::ostringstream out;out<<prefix<<std::hex<<random()<<random();return out.str();}
// 【逐行批注】 定义函数 memoryFingerprint，执行该函数声明的职责。
std::string memoryFingerprint(const std::string& content) {unsigned long long hash=14695981039346656037ULL;for(unsigned char byte:content){hash^=byte;hash*=1099511628211ULL;}std::ostringstream out;out<<std::hex<<hash;return out.str();}
// 【逐行批注】 定义函数 parse，执行该函数声明的职责。
Json parse(const std::string& raw) {auto value=Json::parse(raw,nullptr,false);if(!value.is_object()) throw std::runtime_error("个人中心记录损坏，请从备份恢复");return value;}
// 【逐行批注】 定义函数 read，执行该函数声明的职责。
Json read(Database& db,const std::string& key,Json fallback=Json::object()) {
    // 【逐行批注】 计算右侧结果并写入 if(auto row，更新当前运行状态。
    if(auto row=db.myAIRecord(key)){auto value=parse(row->payload);value["id"]=key;value["version"]=row->version;value["updatedAt"]=row->updatedAt;return value;}
    // 【逐行批注】 计算右侧结果并写入 fallback["version"]，更新当前运行状态。
    fallback["version"]=0;return fallback;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 save，执行该函数声明的职责。
Json save(Database& db,const std::string& key,const std::string& kind,Json value,int version) {
    // 【逐行批注】 计算右侧结果并写入 value.erase("version");value.erase("updatedA…，更新当前运行状态。
    value.erase("version");value.erase("updatedAt");value["id"]=key;
    // 【逐行批注】 计算右侧结果并写入 auto at，更新当前运行状态。
    const auto at=now();if(!db.saveMyAIRecord({key,kind,value.dump(),at,0},version))throw std::runtime_error("内容已在其他窗口更新，请重新读取后再修改");
    // 【逐行批注】 计算右侧结果并写入 value["version"]，更新当前运行状态。
    value["version"]=version+1;value["updatedAt"]=at;return value;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 text，执行该函数声明的职责。
std::string text(const Json& body,const char* key,size_t limit=4000) {
    // 【逐行批注】 调用 Json、invalid_argument、string 完成本行所需操作：if(!body.value(key,Json()).is_string())throw std::invalid_argument(st…。
    if(!body.value(key,Json()).is_string())throw std::invalid_argument(std::string(key)+" 必须是文字");
    // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
    auto value=body[key].get<std::string>();if(value.size()>limit)throw std::invalid_argument("文字超过允许长度");return value;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 required，执行该函数声明的职责。
Json required(Database& db,const std::string& key,const std::string& kind) {
    // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
    const auto row=db.myAIRecord(key);if(!row||row->kind!=kind)throw std::invalid_argument("记录不存在");return read(db,key);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 preferences，执行该函数声明的职责。
Json preferences(Database& db) {return read(db,"preferences",{{"style","循序引导"},{"tone","自然鼓励"},{"pace","随理解程度调整"},{"detail","适中"},{"instructions",""},{"autoMemory",true}});}
// 【逐行批注】 定义函数 profile，执行该函数声明的职责。
Json profile(Database& db) {return read(db,"profile",{{"nickname","同学"},{"avatar",""},{"goal",""}});}
// 【逐行批注】 定义函数 messages，执行该函数声明的职责。
Json messages(Database& db,const std::string& chatId) {
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result=Json::array();for(const auto& item:db.listInteractions())if(item.conversationId==chatId&&(item.kind=="my-ai-user"||item.kind=="my-ai-assistant")){
        // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
        auto value=parse(item.payload);value["id"]=item.id;value["at"]=item.createdAt;value["role"]=item.kind=="my-ai-user"?"user":"assistant";result.push_back(value);
    // 【逐行批注】 执行当前声明或表达式：}return result;。
    }return result;
// 【逐行批注】 结束当前代码块。
}
// 各项独立统计覆盖率；缓存与思考属于输入、输出的明细，不能再加进总量。
// 【逐行批注】 声明数据结构或类型 ReportTokenCounter 及其成员接口。
struct ReportTokenCounter {
    // 【逐行批注】 计算右侧结果并写入 long long total，更新当前运行状态。
    long long total = 0;
    // 【逐行批注】 计算右侧结果并写入 int known，更新当前运行状态。
    int known = 0;
    // 【逐行批注】 计算右侧结果并写入 bool overflow，更新当前运行状态。
    bool overflow = false;
    // 【逐行批注】 定义函数 add，执行该函数声明的职责。
    bool add(const Json& value) {
        // 【逐行批注】 判断条件 (!value.is_number_integer() || (value.is_number_unsigned() &&，成立时执行对应分支。
        if (!value.is_number_integer() || (value.is_number_unsigned() &&
            // 【逐行批注】 调用 max 完成本行所需操作：value.get<unsigned long long>() > static_cast<unsigned long long>(std…。
            value.get<unsigned long long>() > static_cast<unsigned long long>(std::numeric_limits<long long>::max()))) return false;
        // 【逐行批注】 计算右侧结果并写入 auto count，更新当前运行状态。
        const auto count = value.get<long long>();
        // 【逐行批注】 判断条件 (count < 0) return false;，成立时执行对应分支。
        if (count < 0) return false;
        // 【逐行批注】 执行当前声明或表达式：++known;。
        ++known;
        // 【逐行批注】 判断条件 (count > std::numeric_limits<long long>::max() - total) overflow…，成立时执行对应分支。
        if (count > std::numeric_limits<long long>::max() - total) overflow = true;
        // 【逐行批注】 在前一条件不成立时继续判断：(!overflow) total += count;。
        else if (!overflow) total += count;
        // 【逐行批注】 结束当前函数并返回结果：return true;。
        return true;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 result，执行该函数声明的职责。
    Json result() const { return {{"tokens", known && !overflow ? Json(total) : Json()}, {"knownCalls", known}, {"overflow", overflow}}; }
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 myAIRead，执行该函数声明的职责。
Json myAIRead(Database& db,const std::string& section,const Json& query) {
    // 【逐行批注】 调用 profile 完成本行所需操作：if(section=="profile")return profile(db);。
    if(section=="profile")return profile(db);
    // 【逐行批注】 调用 preferences 完成本行所需操作：if(section=="preferences")return preferences(db);。
    if(section=="preferences")return preferences(db);
    // 【逐行批注】 执行当前声明或表达式：if(section=="memories"||section=="chats"||section=="actions") {。
    if(section=="memories"||section=="chats"||section=="actions") {
        // 【逐行批注】 计算右侧结果并写入 Json rows，更新当前运行状态。
        Json rows=Json::array();const auto kind=section=="memories"?"memory":section=="chats"?"chat":"action";
        // 【逐行批注】 按循环规则重复处理：for(const auto& row:db.myAIRecords(kind)){。
        for(const auto& row:db.myAIRecords(kind)){
            // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
            auto value=read(db,row.id);if(value.value("deleted",false))continue;
            // 【逐行批注】 执行当前声明或表达式：if(section=="chats"){value.erase("requests");value.erase("draft");value.erase("scrol…。
            if(section=="chats"){value.erase("requests");value.erase("draft");value.erase("scroll");}
            // 【逐行批注】 执行当前声明或表达式：if(section=="chats"&&value.value("archived",false)!=query.value("archived",false))co…。
            if(section=="chats"&&value.value("archived",false)!=query.value("archived",false))continue;
            // 【逐行批注】 计算右侧结果并写入 auto search，更新当前运行状态。
            const auto search=query.value("search","");if(!search.empty()&&value.dump().find(search)==std::string::npos&&messages(db,row.id).dump().find(search)==std::string::npos)continue;
            // 【逐行批注】 执行当前声明或表达式：rows.push_back(value);。
            rows.push_back(value);
        // 【逐行批注】 执行当前声明或表达式：}return {{"items",rows},{"version",db.learningRevision()}};。
        }return {{"items",rows},{"version",db.learningRevision()}};
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 array、read 完成本行所需操作：if(section=="changes") {Json items=Json::array();for(const auto& row:…。
    if(section=="changes") {Json items=Json::array();for(const auto& row:db.myAIRecords("undo")){auto change=read(db,row.id);change.erase("before");items.push_back(change);}return {{"items",items}};}
    // 【逐行批注】 调用 required、invalid_argument、messages 完成本行所需操作：if(section=="chat") {auto value=required(db,query.at("id"),"chat");if…。
    if(section=="chat") {auto value=required(db,query.at("id"),"chat");if(value.value("deleted",false))throw std::invalid_argument("聊天已删除");value["messages"]=messages(db,query.at("id"));value.erase("requests");return value;}
    // 【逐行批注】 执行当前声明或表达式：if(section=="usage") {。
    if(section=="usage") {
        // 【逐行批注】 计算右侧结果并写入 Json rows，更新当前运行状态。
        Json rows=Json::array();long long duration=0;int success=0;std::set<std::string> tasks;
        // 【逐行批注】 执行当前声明或表达式：ReportTokenCounter total, input, output, cached, missed, reasoning;。
        ReportTokenCounter total, input, output, cached, missed, reasoning;
        // 【逐行批注】 按循环规则重复处理：for(const auto& row:db.myAIRecords("usage")) {auto value=read(db,row.id);c…。
        for(const auto& row:db.myAIRecords("usage")) {auto value=read(db,row.id);const auto day=value.value("day",row.updatedAt.substr(0,10));
            // 【逐行批注】 执行当前声明或表达式：if(day<query.value("from","")||(!query.value("to","").empty()&&day>query.value("to",…。
            if(day<query.value("from","")||(!query.value("to","").empty()&&day>query.value("to","")))continue;
            // 【逐行批注】 执行当前声明或表达式：if(!query.value("model","").empty()&&value.value("model","")!=query.value("model",""…。
            if(!query.value("model","").empty()&&value.value("model","")!=query.value("model",""))continue;
            // 【逐行批注】 计算右侧结果并写入 auto taskRow，更新当前运行状态。
            const auto taskRow = db.getClassroomActivity(value.value("taskId", ""));
            // 【逐行批注】 判断条件 (taskRow && taskRow->kind == "agent-task") {，成立时执行对应分支。
            if (taskRow && taskRow->kind == "agent-task") {
                // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
                const auto task = Json::parse(taskRow->payload, nullptr, false);
                // 【逐行批注】 判断条件 (task.is_object()) for (const auto* field : {"queueMs", "firstCo…，成立时执行对应分支。
                if (task.is_object()) for (const auto* field : {"queueMs", "firstContentMs", "completionMs"})
                    // 【逐行批注】 判断条件 (task.contains(field)) value[field] = task[field];，成立时执行对应分支。
                    if (task.contains(field)) value[field] = task[field];
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 auto usage，更新当前运行状态。
            const auto usage = value.value("tokens", Json());
            // 【逐行批注】 判断条件 (usage.is_object()) {，成立时执行对应分支。
            if (usage.is_object()) {
                // 【逐行批注】 调用 Json 完成本行所需操作：total.add(usage.value("total_tokens", Json())); input.add(usage.value…。
                total.add(usage.value("total_tokens", Json())); input.add(usage.value("prompt_tokens", Json()));
                // 【逐行批注】 调用 Json 完成本行所需操作：output.add(usage.value("completion_tokens", Json())); missed.add(usag…。
                output.add(usage.value("completion_tokens", Json())); missed.add(usage.value("prompt_cache_miss_tokens", Json()));
                // 【逐行批注】 计算右侧结果并写入 auto prompt，更新当前运行状态。
                const auto prompt = usage.value("prompt_tokens_details", Json());
                // 【逐行批注】 计算右侧结果并写入 auto completion，更新当前运行状态。
                const auto completion = usage.value("completion_tokens_details", Json());
                // 两种服务商协议同时提供时只记一次，保留每次请求的原始回执供核对。
                // 【逐行批注】 判断条件 (!cached.add(usage.value("prompt_cache_hit_tokens", Json())) && …，成立时执行对应分支。
                if (!cached.add(usage.value("prompt_cache_hit_tokens", Json())) && prompt.is_object()) cached.add(prompt.value("cached_tokens", Json()));
                // 【逐行批注】 判断条件 (completion.is_object()) reasoning.add(completion.value("reasoni…，成立时执行对应分支。
                if (completion.is_object()) reasoning.add(completion.value("reasoning_tokens", Json()));
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 duration+，更新当前运行状态。
            duration+=value.value("durationMs",0LL);success+=value.value("status","")=="completed";
            // 【逐行批注】 执行当前声明或表达式：if(!value.value("taskId","").empty())tasks.insert(value["taskId"]);。
            if(!value.value("taskId","").empty())tasks.insert(value["taskId"]);
            // 【逐行批注】 执行当前声明或表达式：rows.push_back(value);。
            rows.push_back(value);
        // 【逐行批注】 执行当前声明或表达式：}return {{"items",rows},{"calls",rows.size()},{"tasks",tasks.size()},{"success",succ…。
        }return {{"items",rows},{"calls",rows.size()},{"tasks",tasks.size()},{"success",success},{"durationMs",duration},{"tokens",total.result()["tokens"]},{"knownTokenCalls",total.known},
            // 【逐行批注】 执行当前声明或表达式：{"tokenBreakdown",{{"input",input.result()},{"output",output.result()},{"cachedInput…。
            {"tokenBreakdown",{{"input",input.result()},{"output",output.result()},{"cachedInput",cached.result()},{"uncachedInput",missed.result()},{"reasoning",reasoning.result()}}},
            // 【逐行批注】 执行当前声明或表达式：{"version",rows.size()},{"note","用量仅来自实际请求；各项覆盖的请求可能不同，缺失不补零，超出统计范围显示未提供。缓存属于输入，思考属于…。
            {"version",rows.size()},{"note","用量仅来自实际请求；各项覆盖的请求可能不同，缺失不补零，超出统计范围显示未提供。缓存属于输入，思考属于输出，均不重复计入总量。"}};
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：if(section=="reports"||section=="overview") {。
    if(section=="reports"||section=="overview") {
        // 【逐行批注】 计算右侧结果并写入 Json courses，更新当前运行状态。
        Json courses=Json::array(),topics=Json::array(),evidence=Json::array(),history=Json::array(),time=Json::array();int completed=0;long long seconds=0;
        // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
        const auto courseId=query.value("courseId",""),subject=query.value("subject",""),from=query.value("from",""),to=query.value("to","");
        // 【逐行批注】 执行当前声明或表达式：std::set<std::string> subjectEvidence;。
        std::set<std::string> subjectEvidence;
        // 【逐行批注】 计算右侧结果并写入 auto references，更新当前运行状态。
        const auto references=Json::parse(db.profileMeta("agent-profile-evidence"),nullptr,false);
        // 【逐行批注】 调用 Json 完成本行所需操作：if(!subject.empty()&&references.is_object()&&references.value("subjec…。
        if(!subject.empty()&&references.is_object()&&references.value("subjects",Json()).is_object()&&references["subjects"].contains(subject))
            // 【逐行批注】 按循环规则重复处理：for(const auto& key:references["subjects"][subject].value("evidenceIds",Js…。
            for(const auto& key:references["subjects"][subject].value("evidenceIds",Json::array()))if(key.is_string())subjectEvidence.insert(key.get<std::string>());
        // 【逐行批注】 计算右侧结果并写入 auto belongsToSubject，更新当前运行状态。
        const auto belongsToSubject=[&](const std::string& record){if(subject.empty())return true;for(const auto& key:subjectEvidence)if(key==record||key.rfind(record+":",0)==0)return true;return false;};
        // 【逐行批注】 计算右侧结果并写入 auto inDate，更新当前运行状态。
        const auto inDate=[&](const std::string& date){const auto day=calendarDay(date);return day>=from&&(to.empty()||day<=to);};
        // 【逐行批注】 按循环规则重复处理：for(const auto& c:db.listCourses())if(c.status=="active"&&(courseId.empty(…。
        for(const auto& c:db.listCourses())if(c.status=="active"&&(courseId.empty()||c.id==courseId)){
            // 【逐行批注】 计算右侧结果并写入 p，更新当前运行状态。
            auto p=db.findProgressByCourseId(c.id);const int done=p?p->completedCount:0;completed+=done;
            // 【逐行批注】 执行当前声明或表达式：courses.push_back({{"id",c.id},{"title",c.title},{"percent",p?p->overallPercent:0},{…。
            courses.push_back({{"id",c.id},{"title",c.title},{"percent",p?p->overallPercent:0},{"completed",done},{"total",p?p->totalCount:0}});
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for(const auto& t:db.listTopicMastery())if((courseId.empty()||t.courseId==…。
        for(const auto& t:db.listTopicMastery())if((courseId.empty()||t.courseId==courseId)&&inDate(t.updatedAt)){
            // 【逐行批注】 计算右侧结果并写入 auto c，更新当前运行状态。
            const auto c=db.getCourse(t.courseId);auto ids=Json::parse(t.evidenceIds,nullptr,false);bool matches=subject.empty()||(c&&(c->title.find(subject)!=std::string::npos||c->goal.find(subject)!=std::string::npos));
            // 【逐行批注】 执行当前声明或表达式：if(ids.is_array())for(const auto& key:ids)if(key.is_string()&&subjectEvidence.count(…。
            if(ids.is_array())for(const auto& key:ids)if(key.is_string()&&subjectEvidence.count(key.get<std::string>()))matches=true;
            // 【逐行批注】 执行当前声明或表达式：if(!matches)continue;。
            if(!matches)continue;
            // 【逐行批注】 调用 Json、parse、array 完成本行所需操作：topics.push_back({{"courseId",t.courseId},{"topic",t.topic},{"phaseIn…。
            topics.push_back({{"courseId",t.courseId},{"topic",t.topic},{"phaseIndex",t.phaseIndex},{"score",t.score&&t.status=="ready"?Json(*t.score):Json()},{"status",t.status},{"rationale",t.rationale},{"weakPoints",Json::parse(t.weakPoints,nullptr,false)},{"recommendation",t.recommendation},{"evidenceCount",t.evidenceCount},{"evidenceIds",ids.is_array()?ids:Json::array()}});
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 auto learningRecords，更新当前运行状态。
        const auto learningRecords = db.listInteractions(); std::set<std::string> publishedInputs;
        // 【逐行批注】 按循环规则重复处理：for (const auto& row : learningRecords) if (row.kind == "practice" && inDa…。
        for (const auto& row : learningRecords) if (row.kind == "practice" && inDate(row.createdAt) &&
            // 【逐行批注】 调用 belongsToSubject 完成本行所需操作：(courseId.empty() || row.courseId == courseId) && (subject.empty() ||…。
            (courseId.empty() || row.courseId == courseId) && (subject.empty() || row.subject == subject || belongsToSubject(row.id))) {
            // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
            const auto value = Json::parse(row.payload, nullptr, false);
            // 【逐行批注】 判断条件 (value.is_object() && value.value("status", "") == "valid" && va…，成立时执行对应分支。
            if (value.is_object() && value.value("status", "") == "valid" && value.value("interactionId", Json()).is_string())
                // 【逐行批注】 执行当前声明或表达式：publishedInputs.insert(value.at("interactionId"));。
                publishedInputs.insert(value.at("interactionId"));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for(const auto& row:learningRecords)if(inDate(row.createdAt)&&(courseId.em…。
        for(const auto& row:learningRecords)if(inDate(row.createdAt)&&(courseId.empty()||row.courseId==courseId)&&(subject.empty()||row.subject==subject||belongsToSubject(row.id))){
            // 【逐行批注】 判断条件 (row.kind == "chat-user" && publishedInputs.count(row.id)) conti…，成立时执行对应分支。
            if (row.kind == "chat-user" && publishedInputs.count(row.id)) continue;
            // 【逐行批注】 执行当前声明或表达式：if(row.kind!="quiz"&&row.kind!="question-evaluation"&&row.kind!="practice"&&row.kind…。
            if(row.kind!="quiz"&&row.kind!="question-evaluation"&&row.kind!="practice"&&row.kind!="review"&&row.kind!="chat-user")continue;
            // 【逐行批注】 计算右侧结果并写入 payload，更新当前运行状态。
            auto payload=Json::parse(row.payload,nullptr,false);if(!payload.is_object())continue;
            // 只公开学生自己的输入和身份，不暴露未完成题目的评分标准。
            // 【逐行批注】 计算右侧结果并写入 Json item，更新当前运行状态。
            Json item={{"id",row.id},{"courseId",row.courseId.value_or("")},{"kind",row.kind},{"at",row.createdAt},{"text",payload.value("text","")}};
            // 【逐行批注】 计算右侧结果并写入 if(payload.value("lessonId",Json()).is_strin…，更新当前运行状态。
            if(payload.value("lessonId",Json()).is_string())item["lessonId"]=payload["lessonId"];
            // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"sectionId", "questionId", "view", "requestType", …。
            for (const auto* key : {"sectionId", "questionId", "view", "requestType", "helpMode", "rePracticeOf", "helpHistory", "relatedLearning"})
                // 【逐行批注】 判断条件 (payload.contains(key)) item[key] = payload.at(key);，成立时执行对应分支。
                if (payload.contains(key)) item[key] = payload.at(key);
            // 【逐行批注】 判断条件 (payload.contains("questionSnapshot")) item["question"] = payloa…，成立时执行对应分支。
            if (payload.contains("questionSnapshot")) item["question"] = payload.at("questionSnapshot").value("question", "");
            // 【逐行批注】 判断条件 (payload.contains("questionId") && !item.contains("helpMode")) i…，成立时执行对应分支。
            if (payload.contains("questionId") && !item.contains("helpMode")) item["helpMode"] = "help_unknown";
            // 【逐行批注】 判断条件 (row.kind == "chat-user") {，成立时执行对应分支。
            if (row.kind == "chat-user") {
                // 【逐行批注】 计算右侧结果并写入 item["inputKind"]，更新当前运行状态。
                item["inputKind"] = payload.value("type", "") == "teaching_request" ? "teaching_request" :
                    // 【逐行批注】 执行当前声明或表达式：!db.profileMeta("agent-input-classification:" + row.id).empty() ? "followup" : "answ…。
                    !db.profileMeta("agent-input-classification:" + row.id).empty() ? "followup" : "answer";
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 执行当前声明或表达式：if(row.kind=="practice"&&payload.value("status","")=="valid") {。
            if(row.kind=="practice"&&payload.value("status","")=="valid") {
                // 【逐行批注】 计算右侧结果并写入 auto snapshot，更新当前运行状态。
                const auto snapshot=payload.value("questionSnapshot",Json::object());
                // 【逐行批注】 计算右侧结果并写入 item["question"]，更新当前运行状态。
                item["question"]=snapshot.value("question","");item["text"]=payload.value("response","");item["assessed"]=payload.value("credible",false);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (row.kind == "practice") {，成立时执行对应分支。
            if (row.kind == "practice") {
                // 【逐行批注】 按循环规则重复处理：for (const auto* key : {"diagnosis", "feedback", "feedbackHelpType", "reas…。
                for (const auto* key : {"diagnosis", "feedback", "feedbackHelpType", "reason", "uncertainty", "credible", "score"})
                    // 【逐行批注】 判断条件 (payload.contains(key)) item[key] = payload.at(key);，成立时执行对应分支。
                    if (payload.contains(key)) item[key] = payload.at(key);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 Json 完成本行所需操作：if((row.kind=="quiz"||row.kind=="question-evaluation")&&payload.value…。
            if((row.kind=="quiz"||row.kind=="question-evaluation")&&payload.value("results",Json()).is_array()) {
                // 【逐行批注】 计算右侧结果并写入 int index，更新当前运行状态。
                int index=0;for(const auto& answer:payload["results"]){if(answer.value("answered",false)&&answer.value("credible",false)&&(subject.empty()||row.subject==subject||subjectEvidence.count(row.id+":"+std::to_string(index)))){
                    // 【逐行批注】 计算右侧结果并写入 assessed，更新当前运行状态。
                    auto assessed=item;assessed["id"]=row.id+":"+std::to_string(index);assessed["question"]=answer.value("questionSnapshot",Json::object()).value("question","");
                    // 【逐行批注】 计算右侧结果并写入 auto given，更新当前运行状态。
                    const auto given=answer.value("givenAnswer",Json());assessed["text"]=given.is_string()?given.get<std::string>():given.is_null()?"":given.dump();assessed["assessed"]=true;evidence.push_back(assessed);
                // 【逐行批注】 执行当前声明或表达式：}++index;}。
                }++index;}
            // 【逐行批注】 执行当前声明或表达式：}else evidence.push_back(item);。
            }else evidence.push_back(item);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for(const auto& row:db.myAIRecords("study-time")){auto value=read(db,row.i…。
        for(const auto& row:db.myAIRecords("study-time")){auto value=read(db,row.id);if(!inDate(value.value("day",""))||(!courseId.empty()&&value.value("courseId","")!=courseId))continue;seconds+=value.value("seconds",0LL);time.push_back(value);}
        // 【逐行批注】 按循环规则重复处理：for(const auto& row:db.myAIRecords("profile-snapshot")){auto value=read(db…。
        for(const auto& row:db.myAIRecords("profile-snapshot")){auto value=read(db,row.id);if(inDate(row.updatedAt))history.push_back(value);}
        // 【逐行批注】 计算右侧结果并写入 view，更新当前运行状态。
        auto view=profileView(db);if(!subject.empty()){Json selected=Json::array();for(const auto& s:view.value("subjects",Json::array()))if(s.value("subject","")==subject)selected.push_back(s);view["subjects"]=selected;}
        // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
        Json result={{"profile",profile(db)},{"courses",courses},{"completed",completed},{"studySeconds",seconds},{"subjects",view.value("subjects",Json::array())},{"abilities",abilityProfileView(db)},{"topics",topics},{"evidence",evidence},{"history",history},{"time",time},{"version",db.learningRevision()},{"historyNote","历史学习时长和画像快照未补造；新统计从本次升级后积累。"}};
        if (section == "reports") {
            auto& reviews = result["completedReviews"] = Json::array();
            std::map<std::string, const LearningInteraction*> inputs, latest;
            for (const auto& record : learningRecords) {
                if (record.kind == "chat-user") inputs[record.id] = &record;
                if (record.kind != "practice") continue;
                const auto value = Json::parse(record.payload, nullptr, false);
                if (value.is_object() && value.value("source", "") == "ai" && value.value("interactionId", Json()).is_string())
                    latest[value.at("interactionId")] = &record;
            }
            for (const auto& course : courses) for (const auto& row : db.listClassroomActivities(course.at("id"))) {
                if (row.kind != "review") continue;
                const auto review = Json::parse(row.payload, nullptr, false);
                if (!review.is_object() || review.value("scopeId", "") != "local-profile" ||
                    review.value("status", "") != "completed" || !review.value("completion", Json()).is_object()) continue;
                const auto& completion = review.at("completion");
                const auto when = completion.value("at", "");
                const auto lessonId = completion.value("lessonId", "");
                const auto selected = completion.value("evidence", Json::array());
                if (when.empty() || !inDate(when) || lessonId.empty() || !selected.is_array() || selected.empty()) continue;
                const auto lesson = db.getClassroomActivity(lessonId);
                if (!lesson || lesson->kind != "agent-lesson" || lesson->courseId != row.courseId) continue;
                const auto saved = Json::parse(lesson->payload, nullptr, false);
                if (!saved.is_object() || saved.value("scopeId", "") != "local-profile" ||
                    saved.value("scheduledReviewId", "") != row.id) continue;
                bool matches = subject.empty();
                Json currentIds = Json::array();
                std::size_t changedCount = 0;
                for (const auto& anchor : selected) {
                    if (!anchor.is_object()) { ++changedCount; continue; }
                    const auto id = anchor.value("id", "");
                    matches = matches || belongsToSubject(id);
                    const auto inputId = anchor.value("interactionId", "");
                    const auto input = inputs.find(inputId), found = latest.find(inputId);
                    if (input == inputs.end() || found == latest.end() ||
                        input->second->courseId != row.courseId || found->second->courseId != row.courseId) {
                        ++changedCount; continue;
                    }
                    matches = matches || found->second->subject == subject;
                    const auto learner = Json::parse(input->second->payload, nullptr, false);
                    const auto evaluation = Json::parse(found->second->payload, nullptr, false);
                    const auto diagnosis = evaluation.value("diagnosis", Json::object());
                    if (!learner.is_object() || learner.value("scopeId", "") != "local-profile" ||
                        learner.value("lessonId", "") != lessonId || evaluation.value("status", "") != "valid" ||
                        evaluation.value("credible", false) != true || !diagnosis.is_object() ||
                        diagnosis.value("needsClarification", true)) {
                        ++changedCount; continue;
                    }
                    const bool intervalEvidenceChanged =
                        (anchor.contains("submittedAt") && anchor.at("submittedAt") != Json(input->second->createdAt)) ||
                        (anchor.contains("evaluatedAt") && anchor.at("evaluatedAt") != evaluation.value("at", Json())) ||
                        (anchor.contains("helpMode") && anchor.at("helpMode") != evaluation.value("helpMode", Json()));
                    if (found->second->id != id || evaluation.value("score", Json()) != anchor.value("score", Json()) || intervalEvidenceChanged)
                        ++changedCount;
                    currentIds.push_back(found->second->id);
                }
                auto timing = completion.value("timing", Json{{"status", "not_requested"}, {"learningEffectVerified", false}});
                timing["evidenceCurrent"] = changedCount == 0;
                if (matches) reviews.push_back({{"id", row.id}, {"courseId", row.courseId}, {"courseTitle", course.at("title")},
                    {"title", review.at("title")}, {"lessonId", lessonId}, {"completedAt", when},
                    {"questionCount", selected.size()}, {"changedCount", changedCount}, {"evidenceIds", currentIds}, {"timing", timing}});
            }
            std::stable_sort(reviews.begin(), reviews.end(), [](const Json& a, const Json& b) {
                return a.at("completedAt") > b.at("completedAt");
            });
        }
        
        // 【逐行批注】 计算右侧结果并写入 result["suggestions"]，更新当前运行状态。
        result["suggestions"]=Json::array();
        // 【逐行批注】 计算右侧结果并写入 auto recommended，更新当前运行状态。
        const auto recommended=Json::parse(db.profileMeta("agent-recommendations:"+memoryFingerprint("local-profile")),nullptr,false);
        // 【逐行批注】 调用 Json、array 完成本行所需操作：if(recommended.is_object()&&recommended.value("items",Json()).is_obje…。
        if(recommended.is_object()&&recommended.value("items",Json()).is_object())for(const auto* mode:{"lite","deep"})for(const auto& value:recommended["items"].value(mode,Json::array()))if(value.is_string())result["suggestions"].push_back(value);
        // 【逐行批注】 按循环规则重复处理：for(const auto& s:result["subjects"])if(!s.value("recommendation","").empt…。
        for(const auto& s:result["subjects"])if(!s.value("recommendation","").empty())result["suggestions"].push_back(s["recommendation"]);
        // 【逐行批注】 结束当前函数并返回结果：return result;。
        return result;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("未知个人中心页面");。
    throw std::invalid_argument("未知个人中心页面");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 myAIWrite，执行该函数声明的职责。
Json myAIWrite(Database& db,const std::string& section,const Json& body,bool transactional) {
    // 【逐行批注】 计算右侧结果并写入 Json result;const auto action，更新当前运行状态。
    Json result;const auto action=[&]{
        // 【逐行批注】 执行当前声明或表达式：if(section=="profile"||section=="preferences") {。
        if(section=="profile"||section=="preferences") {
            // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
            auto value=section=="profile"?profile(db):preferences(db);int version=body.at("version");
            // 【逐行批注】 调用 text、string 完成本行所需操作：if(section=="profile"){for(const auto* field:{"nickname","goal","avat…。
            if(section=="profile"){for(const auto* field:{"nickname","goal","avatar"})if(body.contains(field))value[field]=text(body,field,std::string(field)=="avatar"?300000:4000);
                // 【逐行批注】 计算右侧结果并写入 auto avatar，更新当前运行状态。
                const auto avatar=value.value("avatar","");if(!avatar.empty()&&avatar.rfind("data:image/png;base64,",0)!=0&&avatar.rfind("data:image/jpeg;base64,",0)!=0)throw std::invalid_argument("头像仅支持本地 PNG 或 JPEG 图片");}
            // 【逐行批注】 执行前一条件不成立时的分支。
            else{for(const auto* field:{"style","tone","pace","detail","instructions"})if(body.contains(field))value[field]=text(body,field);
                // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(body.contains("autoMemory")){if(!body["autoMemory"].is_boolean())t…。
                if(body.contains("autoMemory")){if(!body["autoMemory"].is_boolean())throw std::invalid_argument("自动记忆开关无效");value["autoMemory"]=body["autoMemory"];}}
            // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
            result=save(db,section,section,value,version);db.markLearningDirty();return;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：if(section=="memories") {。
        if(section=="memories") {
            // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
            const auto key=body.value("id",id("memory-"));auto value=body.contains("id")?required(db,key,"memory"):Json{{"source","manual"},{"createdAt",now()},{"deleted",false}};
            // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(value.value("deleted",false))throw std::invalid_argument("已删除的记忆不会…。
            if(value.value("deleted",false))throw std::invalid_argument("已删除的记忆不会从旧来源重新恢复");
            // 【逐行批注】 计算右侧结果并写入 if(body.contains("content")){value["content"]，更新当前运行状态。
            if(body.contains("content")){value["content"]=text(body,"content");if(value["content"]=="")throw std::invalid_argument("记忆不能为空");}
            // 【逐行批注】 计算右侧结果并写入 if(body.contains("deleted"))value["deleted"]，更新当前运行状态。
            if(body.contains("deleted"))value["deleted"]=body.at("deleted").get<bool>();
            // 【逐行批注】 计算右侧结果并写入 if(body.contains("source"))value["source"]，更新当前运行状态。
            if(body.contains("source"))value["source"]=text(body,"source",100);
            // 【逐行批注】 计算右侧结果并写入 if(body.contains("sourceId"))value["sourceId…，更新当前运行状态。
            if(body.contains("sourceId"))value["sourceId"]=text(body,"sourceId",160);
            // 【逐行批注】 按循环规则重复处理：for(const auto* key:{"sourceChatId","sourceLessonId"})if(body.contains(key…。
            for(const auto* key:{"sourceChatId","sourceLessonId"})if(body.contains(key))value[key]=text(body,key,160);
            // 【逐行批注】 调用 preferences、invalid_argument 完成本行所需操作：if(value.value("source","")=="inferred"&&!preferences(db).value("auto…。
            if(value.value("source","")=="inferred"&&!preferences(db).value("autoMemory",true))throw std::invalid_argument("自动记忆已关闭");
            // 【逐行批注】 计算右侧结果并写入 auto contentHash，更新当前运行状态。
            const auto contentHash=memoryFingerprint(value.value("content",""));
            // 【逐行批注】 执行当前声明或表达式：if(!value.contains("originalContentHash"))value["originalContentHash"]=contentHash;。
            if(!value.contains("originalContentHash"))value["originalContentHash"]=contentHash;
            // 【逐行批注】 按循环规则重复处理：for(const auto& old:db.myAIRecords("memory")){auto other=parse(old.payload…。
            for(const auto& old:db.myAIRecords("memory")){auto other=parse(old.payload);if(old.id==key)continue;
                // 【逐行批注】 计算右侧结果并写入 bool same，更新当前运行状态。
                const bool same=other.value("content","")==value.value("content","")||other.value("contentHash","")==contentHash||other.value("originalContentHash","")==contentHash;
                // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(same&&((other.value("deleted",false)&&(value.value("source","")=="…。
                if(same&&((other.value("deleted",false)&&(value.value("source","")=="inferred"||other.value("sourceId","")==value.value("sourceId","")))||(!other.value("deleted",false)&&other.value("sourceId","")==value.value("sourceId",""))))throw std::invalid_argument("这条记忆已经保存或被删除，不能从旧来源重复建立");
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 if(value.value("deleted",false)){value["cont…，更新当前运行状态。
            if(value.value("deleted",false)){value["contentHash"]=contentHash;value["content"]="";value.erase("sourceChatId");value.erase("sourceLessonId");}
            // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
            result=save(db,key,"memory",value,body.value("version",0));db.markLearningDirty();return;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：if(section=="chats"||section=="chat") {。
        if(section=="chats"||section=="chat") {
            // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
            const auto key=body.value("id",id("chat-"));auto value=body.contains("id")?required(db,key,"chat"):Json{{"title","新聊天"},{"archived",false},{"deleted",false},{"draft",""},{"scroll",0},{"requests",Json::object()}};
            // 【逐行批注】 按循环规则重复处理：for(const auto* field:{"title","draft"})if(body.contains(field))value[fiel…。
            for(const auto* field:{"title","draft"})if(body.contains(field))value[field]=text(body,field,std::string(field)=="title"?240:16000);
            // 【逐行批注】 按循环规则重复处理：for(const auto* field:{"archived","deleted"})if(body.contains(field))value…。
            for(const auto* field:{"archived","deleted"})if(body.contains(field))value[field]=body.at(field).get<bool>();
            // 【逐行批注】 计算右侧结果并写入 if(body.contains("scroll"))value["scroll"]，更新当前运行状态。
            if(body.contains("scroll"))value["scroll"]=std::max(0.0,body["scroll"].get<double>());
            // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
            result=save(db,key,"chat",value,body.value("version",0));return;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：if(section=="heartbeat") {。
        if(section=="heartbeat") {
            // 【逐行批注】 计算右侧结果并写入 auto courseId，更新当前运行状态。
            const auto courseId=text(body,"courseId",160);if(!db.getCourse(courseId))throw std::invalid_argument("课程不存在");
            // 【逐行批注】 计算右侧结果并写入 auto clock，更新当前运行状态。
            const auto clock=millis();auto lease=read(db,"study-lease",{{"last",clock},{"owner",""}});
            // 【逐行批注】 计算右侧结果并写入 auto owner，更新当前运行状态。
            const auto owner=text(body,"owner",160);const auto last=lease.value("last",clock);const auto active=body.value("visible",false)&&body.value("recent",false);
            // 【逐行批注】 计算右侧结果并写入 bool owns，更新当前运行状态。
            const bool owns=lease.value("owner","")==owner||clock-last>30000;
            // 【逐行批注】 计算右侧结果并写入 auto seconds，更新当前运行状态。
            const auto seconds=active&&owns&&lease.value("owner","")==owner&&clock-last<=20000?std::max(0LL,std::min(20LL,(clock-last)/1000)):0LL;
            // 【逐行批注】 执行当前声明或表达式：if(active&&(owns||lease.value("owner","").empty())){。
            if(active&&(owns||lease.value("owner","").empty())){
                // 【逐行批注】 计算右侧结果并写入 lease["owner"]，更新当前运行状态。
                lease["owner"]=body.value("ending",false)?"":owner;lease["last"]=clock;save(db,"study-lease","lease",lease,lease["version"]);
                // 【逐行批注】 计算右侧结果并写入 if(seconds){const auto day，更新当前运行状态。
                if(seconds){const auto day=body.value("day",now().substr(0,10));if(day.size()!=10)throw std::invalid_argument("统计日期无效");const auto key="time:"+day+":"+courseId;auto row=read(db,key,{{"day",day},{"courseId",courseId},{"seconds",0}});row["seconds"]=row.value("seconds",0LL)+seconds;save(db,key,"study-time",row,row["version"]);}
            // 【逐行批注】 调用 save 完成本行所需操作：}else if(!active&&lease.value("owner","")==owner){lease["last"]=clock…。
            }else if(!active&&lease.value("owner","")==owner){lease["last"]=clock;lease["owner"]="";save(db,"study-lease","lease",lease,lease["version"]);}
            // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
            result={{"seconds",seconds}};return;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("未知个人中心操作");。
        throw std::invalid_argument("未知个人中心操作");
    // 【逐行批注】 调用 action 完成本行所需操作：};if(transactional)db.transaction(action);else action();return result;。
    };if(transactional)db.transaction(action);else action();return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 myAIChatMessage，执行该函数声明的职责。
Json myAIChatMessage(Database& db,const std::string& chatId,const Json& body) {
    // 【逐行批注】 计算右侧结果并写入 Json result;db.transaction([&]{auto chat，更新当前运行状态。
    Json result;db.transaction([&]{auto chat=required(db,chatId,"chat");if(chat.value("deleted",false))throw std::invalid_argument("聊天已删除");
        // 【逐行批注】 计算右侧结果并写入 auto requestId，更新当前运行状态。
        const auto requestId=text(body,"requestId",160),content=text(body,"text",16000);if(requestId.empty()||content.empty())throw std::invalid_argument("请输入消息");
        // 【逐行批注】 计算右侧结果并写入 auto& requests，更新当前运行状态。
        auto& requests=chat["requests"];if(requests.contains(requestId)){if(requests[requestId].value("text","")!=content)throw std::invalid_argument("重复请求的内容不一致");result=requests[requestId];return;}
        // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
        result={{"requestId",requestId},{"text",content}};requests[requestId]=result;chat["draft"]="";
        // 【逐行批注】 调用 id、now、runtime_error 完成本行所需操作：if(!db.insert(LearningInteraction{id("message-"),"my-ai-user",result.…。
        if(!db.insert(LearningInteraction{id("message-"),"my-ai-user",result.dump(),now(),{},chatId,{}}))throw std::runtime_error("消息保存失败");
        // 【逐行批注】 调用 save 完成本行所需操作：save(db,chatId,"chat",chat,chat["version"]);。
        save(db,chatId,"chat",chat,chat["version"]);
    // 【逐行批注】 执行当前声明或表达式：});return result;。
    });return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 myAIChatTask，执行该函数声明的职责。
void myAIChatTask(Database& db,const std::string& chatId,const std::string& requestId,const Json& task) {
    // 【逐行批注】 计算右侧结果并写入 db.transaction([&]{auto chat，更新当前运行状态。
    db.transaction([&]{auto chat=required(db,chatId,"chat");chat["requests"][requestId]["taskId"]=task.at("id");chat["taskId"]=task.at("id");save(db,chatId,"chat",chat,chat["version"]);});
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 myAIChatReply，执行该函数声明的职责。
void myAIChatReply(Database& db,const Json& task,const std::string& content) {
    // 【逐行批注】 计算右侧结果并写入 auto chatId，更新当前运行状态。
    const auto chatId=task.at("event").value("chatId","");if(chatId.empty())return;
    // 【逐行批注】 计算右侧结果并写入 auto chat，更新当前运行状态。
    const auto chat=required(db,chatId,"chat");if(chat.value("deleted",false))throw std::runtime_error("聊天已删除，停止保存旧回复");
    // 在主控的提交事务中调用，同一有效模型轮次只保存一次。
    // 【逐行批注】 计算右侧结果并写入 auto messageId，更新当前运行状态。
    const auto messageId="reply:"+task.at("id").get<std::string>()+":"+std::to_string(task.value("calls",0));
    // 【逐行批注】 调用 now 完成本行所需操作：db.insert(LearningInteraction{messageId,"my-ai-assistant",Json{{"text…。
    db.insert(LearningInteraction{messageId,"my-ai-assistant",Json{{"text",content},{"taskId",task.at("id")}}.dump(),now(),{},chatId,{}});
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 myAIProposal，执行该函数声明的职责。
Json myAIProposal(Database& db,const Json& args,const std::string& actionId) {
    // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
    const auto key="action:"+actionId;if(auto old=db.myAIRecord(key))return read(db,key);
    // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
    const auto kind=text(args,"kind",80),target=args.value("target","");
    // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(kind!="delete_course"&&kind!="delete_chat"&&kind!="clear_chats"&&k…。
    if(kind!="delete_course"&&kind!="delete_chat"&&kind!="clear_chats"&&kind!="clear_memories"&&kind!="settings"&&kind!="restore")throw std::invalid_argument("不支持该操作");
    // 【逐行批注】 调用 invalid_argument 完成本行所需操作：Json before;if(kind=="delete_course"){const auto course=db.getCourse(…。
    Json before;if(kind=="delete_course"){const auto course=db.getCourse(target);if(!course)throw std::invalid_argument("课程不存在");before={{"title",course->title},{"updatedAt",course->updatedAt}};}
    // 【逐行批注】 调用 required 完成本行所需操作：if(kind=="delete_chat")before=required(db,target,"chat");。
    if(kind=="delete_chat")before=required(db,target,"chat");
    // 【逐行批注】 调用 invalid_argument、getenv 完成本行所需操作：if(kind=="settings"&&!target.empty()){if(target.size()>200||target.fi…。
    if(kind=="settings"&&!target.empty()){if(target.size()>200||target.find_first_of("\r\n")!=std::string::npos||target.find('\0')!=std::string::npos)throw std::invalid_argument("模型名称无效");before={{"model",std::getenv("AI_MODEL")?std::getenv("AI_MODEL"):""}};}
    // 【逐行批注】 计算右侧结果并写入 Json value，更新当前运行状态。
    Json value={{"kind",kind},{"target",target},{"reason",text(args,"reason")},{"before",before},{"state","pending"},{"learningVersion",db.learningRevision()}};
    // 【逐行批注】 计算右侧结果并写入 value["impact"]，更新当前运行状态。
    value["impact"]=kind=="delete_course"?"删除课程及其学习内容、进度和关联记录":kind=="delete_chat"?"删除该聊天及其管家记录":kind=="clear_chats"?"清空全部管家聊天":kind=="clear_memories"?"删除全部长期记忆":kind=="settings"?"进入独立设置页，由你填写并保存AI配置":"选择备份文件并确认恢复，将替换当前档案";
    // 【逐行批注】 执行当前声明或表达式：if(kind=="settings"&&!target.empty())value["impact"]="检查模型切换："+before.value("model",…。
    if(kind=="settings"&&!target.empty())value["impact"]="检查模型切换："+before.value("model","")+" → "+target+"。最终在桌面设置中保存后生效。";
    // 【逐行批注】 结束当前函数并返回结果：return save(db,key,"action",value,0);。
    return save(db,key,"action",value,0);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 myAIConfirm，执行该函数声明的职责。
Json myAIConfirm(Database& db,const std::string& key,const Json& body) {
    // 【逐行批注】 计算右侧结果并写入 Json result;db.transaction([&]{auto value，更新当前运行状态。
    Json result;db.transaction([&]{auto value=required(db,key,"action");if(value["state"]!="pending"){result=value;return;}
        // 【逐行批注】 调用 runtime_error 完成本行所需操作：if(body.at("version")!=value["version"])throw std::runtime_error("确认记…。
        if(body.at("version")!=value["version"])throw std::runtime_error("确认记录已经更新");
        // 【逐行批注】 计算右侧结果并写入 auto choice，更新当前运行状态。
        const auto choice=body.value("decision","");
        // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(choice!="cancel"&&choice!="confirm")throw std::invalid_argument("请…。
        if(choice!="cancel"&&choice!="confirm")throw std::invalid_argument("请选择确认或取消");
        // 【逐行批注】 执行当前声明或表达式：if(choice=="confirm"){。
        if(choice=="confirm"){
            // 【逐行批注】 调用 runtime_error 完成本行所需操作：if(value["learningVersion"]!=db.learningRevision())throw std::runtime…。
            if(value["learningVersion"]!=db.learningRevision())throw std::runtime_error("学习内容已变化，请重新生成操作预览");
            // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
            const auto kind=value.at("kind").get<std::string>(),target=value.value("target","");
            // 【逐行批注】 调用 runtime_error、deleteCourseForIdentity 完成本行所需操作：if(kind=="delete_course"){const auto course=db.getCourse(target);if(!…。
            if(kind=="delete_course"){const auto course=db.getCourse(target);if(!course||course->updatedAt!=value["before"]["updatedAt"])throw std::runtime_error("课程已经变化");if(!deleteCourseForIdentity(db,target,course->userId.value_or(""),course->anonymousId.value_or("")))throw std::runtime_error("课程删除失败");db.markLearningDirty();}
            // 【逐行批注】 调用 required、runtime_error、save 完成本行所需操作：if(kind=="delete_chat"){auto chat=required(db,target,"chat");if(chat[…。
            if(kind=="delete_chat"){auto chat=required(db,target,"chat");if(chat["version"]!=value["before"]["version"])throw std::runtime_error("聊天已经更新");const int chatVersion=chat["version"];chat={{"deleted",true}};save(db,target,"chat",chat,chatVersion);db.deleteConversation(target);db.deleteInteractionsForConversation(target);db.markLearningDirty();}
            // 【逐行批注】 调用 read、memoryFingerprint、save 完成本行所需操作：if(kind=="clear_chats"||kind=="clear_memories")for(const auto& row:db…。
            if(kind=="clear_chats"||kind=="clear_memories")for(const auto& row:db.myAIRecords(kind=="clear_chats"?"chat":"memory")){auto record=read(db,row.id);if(kind=="clear_chats")record={{"deleted",true}};else{record["contentHash"]=memoryFingerprint(record.value("content",""));record["content"]="";record["deleted"]=true;record.erase("sourceChatId");record.erase("sourceLessonId");}save(db,row.id,row.kind,record,row.version);if(kind=="clear_chats"){db.deleteConversation(row.id);db.deleteInteractionsForConversation(row.id);}}
            // 【逐行批注】 执行当前声明或表达式：if(kind=="clear_memories"||kind=="clear_chats")db.markLearningDirty();。
            if(kind=="clear_memories"||kind=="clear_chats")db.markLearningDirty();
            // 【逐行批注】 计算右侧结果并写入 value["state"]，更新当前运行状态。
            value["state"]=(kind=="settings"||kind=="restore")?"awaiting_native":"applied";
            // 【逐行批注】 执行当前声明或表达式：if(kind=="settings")value["href"]="/my-ai/settings";。
            if(kind=="settings")value["href"]="/my-ai/settings";
            // 【逐行批注】 执行当前声明或表达式：if(kind=="restore")value["href"]="/my-ai/data";。
            if(kind=="restore")value["href"]="/my-ai/data";
        // 【逐行批注】 计算右侧结果并写入 }else value["state"]，更新当前运行状态。
        }else value["state"]="cancelled";
        // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
        result=save(db,key,"action",value,value["version"]);
    // 【逐行批注】 执行当前声明或表达式：});return result;。
    });return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 myAISnapshot，执行该函数声明的职责。
void myAISnapshot(Database& db) {
    // 【逐行批注】 计算右侧结果并写入 auto assessed，更新当前运行状态。
    const auto assessed=db.profileAssessedRevision();if(assessed<=0)return;
    // 【逐行批注】 计算右侧结果并写入 Json scores，更新当前运行状态。
    Json scores=Json::object();for(const auto& item:db.listMastery())scores[item.subject]=item.score?Json(*item.score):Json();
    // 【逐行批注】 计算右侧结果并写入 Json assessment，更新当前运行状态。
    const Json assessment={{"scores",scores},{"abilities",abilityProfileView(db)}};
    // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
    const auto key="snapshot:"+std::to_string(assessed)+":"+memoryFingerprint(assessment.dump());if(db.myAIRecord(key))return;
    // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
    auto value=assessment;value["learningVersion"]=db.learningRevision();save(db,key,"profile-snapshot",value,0);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

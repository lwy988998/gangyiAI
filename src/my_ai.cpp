#include "my_ai.hpp"
#include "my_ai_context.hpp"
#include "profile_service.hpp"
#include "course_service.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>

namespace gangyi {
namespace {
using Json = MyAIJson;
std::string now() {
    auto clock=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); std::tm value{};
#ifdef _WIN32
    gmtime_s(&value,&clock);
#else
    gmtime_r(&clock,&value);
#endif
    std::ostringstream out;out<<std::put_time(&value,"%Y-%m-%dT%H:%M:%SZ");return out.str();
}
std::string calendarDay(const std::string& at) {
    if(at.size()<20||at.back()!='Z')return at.substr(0,10);
    std::tm utc{},local{};std::istringstream input(at);input>>std::get_time(&utc,"%Y-%m-%dT%H:%M:%S");if(input.fail())return at.substr(0,10);
#ifdef _WIN32
    const auto clock=_mkgmtime(&utc);if(clock<0)return at.substr(0,10);localtime_s(&local,&clock);
#else
    const auto clock=timegm(&utc);if(clock<0)return at.substr(0,10);localtime_r(&clock,&local);
#endif
    std::ostringstream out;out<<std::put_time(&local,"%Y-%m-%d");return out.str();
}
long long millis() {return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();}
std::string id(const std::string& prefix) {static thread_local std::mt19937_64 random(std::random_device{}());std::ostringstream out;out<<prefix<<std::hex<<random()<<random();return out.str();}
std::string memoryFingerprint(const std::string& content) {unsigned long long hash=14695981039346656037ULL;for(unsigned char byte:content){hash^=byte;hash*=1099511628211ULL;}std::ostringstream out;out<<std::hex<<hash;return out.str();}
Json parse(const std::string& raw) {auto value=Json::parse(raw,nullptr,false);if(!value.is_object()) throw std::runtime_error("个人中心记录损坏，请从备份恢复");return value;}
Json read(Database& db,const std::string& key,Json fallback=Json::object()) {
    if(auto row=db.myAIRecord(key)){auto value=parse(row->payload);value["id"]=key;value["version"]=row->version;value["updatedAt"]=row->updatedAt;return value;}
    fallback["version"]=0;return fallback;
}
Json save(Database& db,const std::string& key,const std::string& kind,Json value,int version) {
    value.erase("version");value.erase("updatedAt");value["id"]=key;
    const auto at=now();if(!db.saveMyAIRecord({key,kind,value.dump(),at,0},version))throw std::runtime_error("内容已在其他窗口更新，请重新读取后再修改");
    value["version"]=version+1;value["updatedAt"]=at;return value;
}
std::string text(const Json& body,const char* key,size_t limit=4000) {
    if(!body.value(key,Json()).is_string())throw std::invalid_argument(std::string(key)+" 必须是文字");
    auto value=body[key].get<std::string>();if(value.size()>limit)throw std::invalid_argument("文字超过允许长度");return value;
}
Json required(Database& db,const std::string& key,const std::string& kind) {
    const auto row=db.myAIRecord(key);if(!row||row->kind!=kind)throw std::invalid_argument("记录不存在");return read(db,key);
}
Json preferences(Database& db) {return read(db,"preferences",{{"style","循序引导"},{"tone","自然鼓励"},{"pace","随理解程度调整"},{"detail","适中"},{"instructions",""},{"autoMemory",true}});}
Json profile(Database& db) {return read(db,"profile",{{"nickname","同学"},{"avatar",""},{"goal",""}});}
Json messages(Database& db,const std::string& chatId) {
    Json result=Json::array();for(const auto& item:db.listInteractions())if(item.conversationId==chatId&&(item.kind=="my-ai-user"||item.kind=="my-ai-assistant")){
        auto value=parse(item.payload);value["id"]=item.id;value["at"]=item.createdAt;value["role"]=item.kind=="my-ai-user"?"user":"assistant";result.push_back(value);
    }return result;
}
}

Json myAIRead(Database& db,const std::string& section,const Json& query) {
    if(section=="profile")return profile(db);
    if(section=="preferences")return preferences(db);
    if(section=="memories"||section=="chats"||section=="actions") {
        Json rows=Json::array();const auto kind=section=="memories"?"memory":section=="chats"?"chat":"action";
        for(const auto& row:db.myAIRecords(kind)){
            auto value=read(db,row.id);if(value.value("deleted",false))continue;
            if(section=="chats"){value.erase("requests");value.erase("draft");value.erase("scroll");}
            if(section=="chats"&&value.value("archived",false)!=query.value("archived",false))continue;
            const auto search=query.value("search","");if(!search.empty()&&value.dump().find(search)==std::string::npos&&messages(db,row.id).dump().find(search)==std::string::npos)continue;
            rows.push_back(value);
        }return {{"items",rows},{"version",db.learningRevision()}};
    }
    if(section=="changes") {Json items=Json::array();for(const auto& row:db.myAIRecords("undo")){auto change=read(db,row.id);change.erase("before");items.push_back(change);}return {{"items",items}};}
    if(section=="chat") {auto value=required(db,query.at("id"),"chat");if(value.value("deleted",false))throw std::invalid_argument("聊天已删除");value["messages"]=messages(db,query.at("id"));value.erase("requests");return value;}
    if(section=="usage") {
        Json rows=Json::array();long long tokens=0,duration=0;int known=0,success=0;std::set<std::string> tasks;
        for(const auto& row:db.myAIRecords("usage")) {auto value=read(db,row.id);const auto day=value.value("day",row.updatedAt.substr(0,10));
            if(day<query.value("from","")||(!query.value("to","").empty()&&day>query.value("to","")))continue;
            if(!query.value("model","").empty()&&value.value("model","")!=query.value("model",""))continue;
            if(value.value("tokens",Json()).is_object()&&value["tokens"].value("total_tokens",Json()).is_number_integer()){tokens+=value["tokens"]["total_tokens"].get<long long>();++known;}
            duration+=value.value("durationMs",0LL);success+=value.value("status","")=="completed";
            if(!value.value("taskId","").empty())tasks.insert(value["taskId"]);rows.push_back(value);
        }return {{"items",rows},{"calls",rows.size()},{"tasks",tasks.size()},{"success",success},{"durationMs",duration},{"tokens",known?Json(tokens):Json()},{"knownTokenCalls",known},{"version",rows.size()},{"note","用量仅来自实际请求；未提供的 Token 不计入已知合计。统计从升级后开始。"}};
    }
    if(section=="reports"||section=="overview") {
        Json courses=Json::array(),topics=Json::array(),evidence=Json::array(),history=Json::array(),time=Json::array();int completed=0;long long seconds=0;
        const auto courseId=query.value("courseId",""),subject=query.value("subject",""),from=query.value("from",""),to=query.value("to","");
        std::set<std::string> subjectEvidence;
        const auto references=Json::parse(db.profileMeta("agent-profile-evidence"),nullptr,false);
        if(!subject.empty()&&references.is_object()&&references.value("subjects",Json()).is_object()&&references["subjects"].contains(subject))
            for(const auto& key:references["subjects"][subject].value("evidenceIds",Json::array()))if(key.is_string())subjectEvidence.insert(key.get<std::string>());
        const auto belongsToSubject=[&](const std::string& record){if(subject.empty())return true;for(const auto& key:subjectEvidence)if(key==record||key.rfind(record+":",0)==0)return true;return false;};
        const auto inDate=[&](const std::string& date){const auto day=calendarDay(date);return day>=from&&(to.empty()||day<=to);};
        for(const auto& c:db.listCourses())if(c.status=="active"&&(courseId.empty()||c.id==courseId)){
            auto p=db.findProgressByCourseId(c.id);const int done=p?p->completedCount:0;completed+=done;
            courses.push_back({{"id",c.id},{"title",c.title},{"percent",p?p->overallPercent:0},{"completed",done},{"total",p?p->totalCount:0}});
        }
        for(const auto& t:db.listTopicMastery())if((courseId.empty()||t.courseId==courseId)&&inDate(t.updatedAt)){
            const auto c=db.getCourse(t.courseId);auto ids=Json::parse(t.evidenceIds,nullptr,false);bool matches=subject.empty()||(c&&(c->title.find(subject)!=std::string::npos||c->goal.find(subject)!=std::string::npos));
            if(ids.is_array())for(const auto& key:ids)if(key.is_string()&&subjectEvidence.count(key.get<std::string>()))matches=true;
            if(!matches)continue;
            topics.push_back({{"courseId",t.courseId},{"topic",t.topic},{"phaseIndex",t.phaseIndex},{"score",t.score&&t.status=="ready"?Json(*t.score):Json()},{"status",t.status},{"rationale",t.rationale},{"weakPoints",Json::parse(t.weakPoints,nullptr,false)},{"recommendation",t.recommendation},{"evidenceCount",t.evidenceCount},{"evidenceIds",ids.is_array()?ids:Json::array()}});
        }
        for(const auto& row:db.listInteractions())if(inDate(row.createdAt)&&(courseId.empty()||row.courseId==courseId)&&(subject.empty()||row.subject==subject||belongsToSubject(row.id))){
            if(row.kind!="quiz"&&row.kind!="question-evaluation"&&row.kind!="practice"&&row.kind!="review"&&row.kind!="chat-user")continue;
            auto payload=Json::parse(row.payload,nullptr,false);if(!payload.is_object())continue;
            // 只公开学生自己的输入和身份，不暴露未完成题目的评分标准。
            Json item={{"id",row.id},{"courseId",row.courseId.value_or("")},{"kind",row.kind},{"at",row.createdAt},{"text",payload.value("text","")}};
            if(payload.value("lessonId",Json()).is_string())item["lessonId"]=payload["lessonId"];
            if(row.kind=="practice"&&payload.value("status","")=="valid"&&payload.value("credible",false)) {
                const auto snapshot=payload.value("questionSnapshot",Json::object());
                item["question"]=snapshot.value("question","");item["text"]=payload.value("response","");item["assessed"]=true;
            }
            if((row.kind=="quiz"||row.kind=="question-evaluation")&&payload.value("results",Json()).is_array()) {
                int index=0;for(const auto& answer:payload["results"]){if(answer.value("answered",false)&&answer.value("credible",false)&&(subject.empty()||row.subject==subject||subjectEvidence.count(row.id+":"+std::to_string(index)))){
                    auto assessed=item;assessed["id"]=row.id+":"+std::to_string(index);assessed["question"]=answer.value("questionSnapshot",Json::object()).value("question","");
                    const auto given=answer.value("givenAnswer",Json());assessed["text"]=given.is_string()?given.get<std::string>():given.is_null()?"":given.dump();assessed["assessed"]=true;evidence.push_back(assessed);
                }++index;}
            }else evidence.push_back(item);
        }
        for(const auto& row:db.myAIRecords("study-time")){auto value=read(db,row.id);if(!inDate(value.value("day",""))||(!courseId.empty()&&value.value("courseId","")!=courseId))continue;seconds+=value.value("seconds",0LL);time.push_back(value);}
        for(const auto& row:db.myAIRecords("profile-snapshot")){auto value=read(db,row.id);if(inDate(row.updatedAt))history.push_back(value);}
        auto view=profileView(db);if(!subject.empty()){Json selected=Json::array();for(const auto& s:view.value("subjects",Json::array()))if(s.value("subject","")==subject)selected.push_back(s);view["subjects"]=selected;}
        Json result={{"profile",profile(db)},{"courses",courses},{"completed",completed},{"studySeconds",seconds},{"subjects",view.value("subjects",Json::array())},{"abilities",abilityProfileView(db)},{"topics",topics},{"evidence",evidence},{"history",history},{"time",time},{"version",db.learningRevision()},{"historyNote","历史学习时长和画像快照未补造；新统计从本次升级后积累。"}};
        
        result["suggestions"]=Json::array();
        const auto recommended=Json::parse(db.profileMeta("agent-recommendations:"+memoryFingerprint("local-profile")),nullptr,false);
        if(recommended.is_object()&&recommended.value("items",Json()).is_object())for(const auto* mode:{"lite","deep"})for(const auto& value:recommended["items"].value(mode,Json::array()))if(value.is_string())result["suggestions"].push_back(value);
        for(const auto& s:result["subjects"])if(!s.value("recommendation","").empty())result["suggestions"].push_back(s["recommendation"]);
        return result;
    }
    throw std::invalid_argument("未知个人中心页面");
}

Json myAIWrite(Database& db,const std::string& section,const Json& body,bool transactional) {
    Json result;const auto action=[&]{
        if(section=="profile"||section=="preferences") {
            auto value=section=="profile"?profile(db):preferences(db);int version=body.at("version");
            if(section=="profile"){for(const auto* field:{"nickname","goal","avatar"})if(body.contains(field))value[field]=text(body,field,std::string(field)=="avatar"?300000:4000);
                const auto avatar=value.value("avatar","");if(!avatar.empty()&&avatar.rfind("data:image/png;base64,",0)!=0&&avatar.rfind("data:image/jpeg;base64,",0)!=0)throw std::invalid_argument("头像仅支持本地 PNG 或 JPEG 图片");}
            else{for(const auto* field:{"style","tone","pace","detail","instructions"})if(body.contains(field))value[field]=text(body,field);
                if(body.contains("autoMemory")){if(!body["autoMemory"].is_boolean())throw std::invalid_argument("自动记忆开关无效");value["autoMemory"]=body["autoMemory"];}}
            result=save(db,section,section,value,version);db.markLearningDirty();return;
        }
        if(section=="memories") {
            const auto key=body.value("id",id("memory-"));auto value=body.contains("id")?required(db,key,"memory"):Json{{"source","manual"},{"createdAt",now()},{"deleted",false}};
            if(value.value("deleted",false))throw std::invalid_argument("已删除的记忆不会从旧来源重新恢复");
            if(body.contains("content")){value["content"]=text(body,"content");if(value["content"]=="")throw std::invalid_argument("记忆不能为空");}
            if(body.contains("deleted"))value["deleted"]=body.at("deleted").get<bool>();
            if(body.contains("source"))value["source"]=text(body,"source",100);
            if(body.contains("sourceId"))value["sourceId"]=text(body,"sourceId",160);
            for(const auto* key:{"sourceChatId","sourceLessonId"})if(body.contains(key))value[key]=text(body,key,160);
            if(value.value("source","")=="inferred"&&!preferences(db).value("autoMemory",true))throw std::invalid_argument("自动记忆已关闭");
            const auto contentHash=memoryFingerprint(value.value("content",""));
            if(!value.contains("originalContentHash"))value["originalContentHash"]=contentHash;
            for(const auto& old:db.myAIRecords("memory")){auto other=parse(old.payload);if(old.id==key)continue;
                const bool same=other.value("content","")==value.value("content","")||other.value("contentHash","")==contentHash||other.value("originalContentHash","")==contentHash;
                if(same&&((other.value("deleted",false)&&(value.value("source","")=="inferred"||other.value("sourceId","")==value.value("sourceId","")))||(!other.value("deleted",false)&&other.value("sourceId","")==value.value("sourceId",""))))throw std::invalid_argument("这条记忆已经保存或被删除，不能从旧来源重复建立");
            }
            if(value.value("deleted",false)){value["contentHash"]=contentHash;value["content"]="";value.erase("sourceChatId");value.erase("sourceLessonId");}
            result=save(db,key,"memory",value,body.value("version",0));db.markLearningDirty();return;
        }
        if(section=="chats"||section=="chat") {
            const auto key=body.value("id",id("chat-"));auto value=body.contains("id")?required(db,key,"chat"):Json{{"title","新聊天"},{"archived",false},{"deleted",false},{"draft",""},{"scroll",0},{"requests",Json::object()}};
            for(const auto* field:{"title","draft"})if(body.contains(field))value[field]=text(body,field,std::string(field)=="title"?240:16000);
            for(const auto* field:{"archived","deleted"})if(body.contains(field))value[field]=body.at(field).get<bool>();
            if(body.contains("scroll"))value["scroll"]=std::max(0.0,body["scroll"].get<double>());
            result=save(db,key,"chat",value,body.value("version",0));return;
        }
        if(section=="heartbeat") {
            const auto courseId=text(body,"courseId",160);if(!db.getCourse(courseId))throw std::invalid_argument("课程不存在");
            const auto clock=millis();auto lease=read(db,"study-lease",{{"last",clock},{"owner",""}});
            const auto owner=text(body,"owner",160);const auto last=lease.value("last",clock);const auto active=body.value("visible",false)&&body.value("recent",false);
            const bool owns=lease.value("owner","")==owner||clock-last>30000;
            const auto seconds=active&&owns&&lease.value("owner","")==owner&&clock-last<=20000?std::max(0LL,std::min(20LL,(clock-last)/1000)):0LL;
            if(active&&(owns||lease.value("owner","").empty())){
                lease["owner"]=body.value("ending",false)?"":owner;lease["last"]=clock;save(db,"study-lease","lease",lease,lease["version"]);
                if(seconds){const auto day=body.value("day",now().substr(0,10));if(day.size()!=10)throw std::invalid_argument("统计日期无效");const auto key="time:"+day+":"+courseId;auto row=read(db,key,{{"day",day},{"courseId",courseId},{"seconds",0}});row["seconds"]=row.value("seconds",0LL)+seconds;save(db,key,"study-time",row,row["version"]);}
            }else if(!active&&lease.value("owner","")==owner){lease["last"]=clock;lease["owner"]="";save(db,"study-lease","lease",lease,lease["version"]);}
            result={{"seconds",seconds}};return;
        }
        throw std::invalid_argument("未知个人中心操作");
    };if(transactional)db.transaction(action);else action();return result;
}

Json myAIChatMessage(Database& db,const std::string& chatId,const Json& body) {
    Json result;db.transaction([&]{auto chat=required(db,chatId,"chat");if(chat.value("deleted",false))throw std::invalid_argument("聊天已删除");
        const auto requestId=text(body,"requestId",160),content=text(body,"text",16000);if(requestId.empty()||content.empty())throw std::invalid_argument("请输入消息");
        auto& requests=chat["requests"];if(requests.contains(requestId)){if(requests[requestId].value("text","")!=content)throw std::invalid_argument("重复请求的内容不一致");result=requests[requestId];return;}
        result={{"requestId",requestId},{"text",content}};requests[requestId]=result;chat["draft"]="";
        if(!db.insert(LearningInteraction{id("message-"),"my-ai-user",result.dump(),now(),{},chatId,{}}))throw std::runtime_error("消息保存失败");
        save(db,chatId,"chat",chat,chat["version"]);
    });return result;
}
void myAIChatTask(Database& db,const std::string& chatId,const std::string& requestId,const Json& task) {
    db.transaction([&]{auto chat=required(db,chatId,"chat");chat["requests"][requestId]["taskId"]=task.at("id");chat["taskId"]=task.at("id");save(db,chatId,"chat",chat,chat["version"]);});
}
void myAIChatReply(Database& db,const Json& task,const std::string& content) {
    const auto chatId=task.at("event").value("chatId","");if(chatId.empty())return;
    const auto chat=required(db,chatId,"chat");if(chat.value("deleted",false))throw std::runtime_error("聊天已删除，停止保存旧回复");
    // 在主控的提交事务中调用，同一有效模型轮次只保存一次。
    const auto messageId="reply:"+task.at("id").get<std::string>()+":"+std::to_string(task.value("calls",0));
    db.insert(LearningInteraction{messageId,"my-ai-assistant",Json{{"text",content},{"taskId",task.at("id")}}.dump(),now(),{},chatId,{}});
}

Json myAIProposal(Database& db,const Json& args,const std::string& actionId) {
    const auto key="action:"+actionId;if(auto old=db.myAIRecord(key))return read(db,key);
    const auto kind=text(args,"kind",80),target=args.value("target","");
    if(kind!="delete_course"&&kind!="delete_chat"&&kind!="clear_chats"&&kind!="clear_memories"&&kind!="settings"&&kind!="restore")throw std::invalid_argument("不支持该操作");
    Json before;if(kind=="delete_course"){const auto course=db.getCourse(target);if(!course)throw std::invalid_argument("课程不存在");before={{"title",course->title},{"updatedAt",course->updatedAt}};}
    if(kind=="delete_chat")before=required(db,target,"chat");
    if(kind=="settings"&&!target.empty()){if(target.size()>200||target.find_first_of("\r\n")!=std::string::npos||target.find('\0')!=std::string::npos)throw std::invalid_argument("模型名称无效");before={{"model",std::getenv("AI_MODEL")?std::getenv("AI_MODEL"):""}};}
    Json value={{"kind",kind},{"target",target},{"reason",text(args,"reason")},{"before",before},{"state","pending"},{"learningVersion",db.learningRevision()}};
    value["impact"]=kind=="delete_course"?"删除课程及其学习内容、进度和关联记录":kind=="delete_chat"?"删除该聊天及其管家记录":kind=="clear_chats"?"清空全部管家聊天":kind=="clear_memories"?"删除全部长期记忆":kind=="settings"?"进入独立设置页，由你填写并保存AI配置":"选择备份文件并确认恢复，将替换当前档案";
    if(kind=="settings"&&!target.empty())value["impact"]="检查模型切换："+before.value("model","")+" → "+target+"。最终在桌面设置中保存后生效。";
    return save(db,key,"action",value,0);
}
Json myAIConfirm(Database& db,const std::string& key,const Json& body) {
    Json result;db.transaction([&]{auto value=required(db,key,"action");if(value["state"]!="pending"){result=value;return;}
        if(body.at("version")!=value["version"])throw std::runtime_error("确认记录已经更新");const auto choice=body.value("decision","");
        if(choice!="cancel"&&choice!="confirm")throw std::invalid_argument("请选择确认或取消");
        if(choice=="confirm"){
            if(value["learningVersion"]!=db.learningRevision())throw std::runtime_error("学习内容已变化，请重新生成操作预览");
            const auto kind=value.at("kind").get<std::string>(),target=value.value("target","");
            if(kind=="delete_course"){const auto course=db.getCourse(target);if(!course||course->updatedAt!=value["before"]["updatedAt"])throw std::runtime_error("课程已经变化");if(!deleteCourseForIdentity(db,target,course->userId.value_or(""),course->anonymousId.value_or("")))throw std::runtime_error("课程删除失败");db.markLearningDirty();}
            if(kind=="delete_chat"){auto chat=required(db,target,"chat");if(chat["version"]!=value["before"]["version"])throw std::runtime_error("聊天已经更新");const int chatVersion=chat["version"];chat={{"deleted",true}};save(db,target,"chat",chat,chatVersion);db.deleteConversation(target);db.deleteInteractionsForConversation(target);db.markLearningDirty();}
            if(kind=="clear_chats"||kind=="clear_memories")for(const auto& row:db.myAIRecords(kind=="clear_chats"?"chat":"memory")){auto record=read(db,row.id);if(kind=="clear_chats")record={{"deleted",true}};else{record["contentHash"]=memoryFingerprint(record.value("content",""));record["content"]="";record["deleted"]=true;record.erase("sourceChatId");record.erase("sourceLessonId");}save(db,row.id,row.kind,record,row.version);if(kind=="clear_chats"){db.deleteConversation(row.id);db.deleteInteractionsForConversation(row.id);}}
            if(kind=="clear_memories"||kind=="clear_chats")db.markLearningDirty();
            value["state"]=(kind=="settings"||kind=="restore")?"awaiting_native":"applied";
            if(kind=="settings")value["href"]="/my-ai/settings";if(kind=="restore")value["href"]="/my-ai/data";
        }else value["state"]="cancelled";
        result=save(db,key,"action",value,value["version"]);
    });return result;
}
void myAISnapshot(Database& db) {
    const auto assessed=db.profileAssessedRevision();if(assessed<=0)return;
    Json scores=Json::object();for(const auto& item:db.listMastery())scores[item.subject]=item.score?Json(*item.score):Json();
    const Json assessment={{"scores",scores},{"abilities",abilityProfileView(db)}};
    const auto key="snapshot:"+std::to_string(assessed)+":"+memoryFingerprint(assessment.dump());if(db.myAIRecord(key))return;
    auto value=assessment;value["learningVersion"]=db.learningRevision();save(db,key,"profile-snapshot",value,0);
}
}

#include "my_ai.hpp"
#include "my_ai_context.hpp"
#include "learning_agent.hpp"
namespace gangyi {
void myAIResumeControls(Database& db) {
    using Json=MyAIJson;
    for(const auto& row:db.myAIRecords("control")) {
        auto value=Json::parse(row.payload);if(value.value("state","")!="pending")continue;
        try {
            const auto args=value.at("args");AgentAccess access{value.at("scopeId"),value.at("courseIds").get<std::vector<std::string>>()};
            bool current=true;
            if(args["command"]=="pause_all") {current=AIActivity::pauseEpoch()==value.value("epoch",0ULL);if(current)AIActivity::setPaused(true);}
            else {const auto target=agentView(db,access,args.at("taskId"));current=target["version"]==value["targetVersion"];if(current)agentControl(db,access,args);}
            value["state"]=current?"applied":"superseded";
        }catch(const std::exception& error){value["state"]="failed";value["error"]=error.what();}
        if(!db.saveMyAIRecord({row.id,row.kind,value.dump(),row.updatedAt,0},row.version))throw std::runtime_error("控制记录已经更新");
    }
}
void registerMyAITools(LearningAgent& agent) {
    using Json=MyAIJson;
    agent.registerTool("control_ai_task",{"按学生明确指令暂停、继续或重试AI任务。参数 command:pause|resume|retry，taskId:现有任务ID。全局暂停使用 command:pause_all。",false,
        [](Database& db,const Json& args,const Json& task,const AIResult&){
            if(task.at("event").value("type","")!="assistant_chat")throw std::invalid_argument("请在AI管家中发起控制请求");
            const auto command=args.value("command","");if(command!="pause"&&command!="resume"&&command!="retry"&&command!="pause_all")throw std::invalid_argument("控制命令无效");
            AgentAccess access{task.at("scopeId"),task.at("courseIds").get<std::vector<std::string>>()};
            Json targetVersion;if(command!="pause_all")targetVersion=agentView(db,access,args.at("taskId")).at("version");
            const auto key="control:"+task.at("id").get<std::string>()+":"+std::to_string(task.value("calls",0))+":"+command+":"+args.value("taskId","");
            const auto queued=Json{{"args",args},{"scopeId",access.scopeId},{"courseIds",access.courseIds},{"targetVersion",targetVersion},{"epoch",AIActivity::pauseEpoch()},{"state","pending"}};
            PreparedAgentTool tool;tool.result={{"command",command},{"controlId",key},{"href","/my-ai/tasks"}};
            tool.commit=[key,queued,task](Database& connection){if(!connection.myAIRecord(key))connection.saveMyAIRecord({key,"control",queued.dump(),task.at("createdAt"),0},0);};
            tool.afterCommit=[key](Database& connection){myAIResumeControls(connection);const auto row=connection.myAIRecord(key);if(row){const auto result=Json::parse(row->payload);if(result.value("state","")!="applied")throw std::runtime_error(result.value("error","任务状态已变化，控制没有应用。"));}};return tool;
        }});
    agent.registerTool("read_personal_center",{"读取个人档案、教学偏好、记忆、学习报告、调用统计和管家聊天。参数 section:overview|profile|preferences|memories|reports|usage|chats|tasks。",true,
        [](Database& db,const Json& args,const Json& task,const AIResult&){
            if(args["section"]=="tasks") {Json items=Json::array();const auto ids=Json::parse(db.profileMeta("agent-index"),nullptr,false);const AgentAccess access{task.at("scopeId"),task.at("courseIds").get<std::vector<std::string>>()};if(ids.is_array())for(const auto& key:ids)try{items.push_back(agentView(db,access,key.get<std::string>(),true));}catch(const std::exception&){}return PreparedAgentTool{{{"items",items}},{}};}
            auto result=myAIRead(db,args.at("section"));result.erase("avatar");if(result.contains("profile")&&result["profile"].is_object())result["profile"].erase("avatar");return PreparedAgentTool{result,{}};}});
    agent.registerTool("set_teaching_preferences",{"按学生当前明确要求调整教学偏好。参数 style、tone、pace、detail、instructions、autoMemory；只传学生要求改动的字段。无需再次确认，可撤回。",false,
        [](Database& db,const Json& args,const Json& task,const AIResult&){
            if(task.at("event").value("type","")!="assistant_chat")throw std::invalid_argument("请由AI管家执行明确的设置请求");
            const auto before=myAIRead(db,"preferences");auto body=args;body["version"]=before["version"];
            bool different=false;for(const auto& item:args.items())if(!before.contains(item.key())||before[item.key()]!=item.value())different=true;
            if(!different)return PreparedAgentTool{{{"changed",false},{"href","/my-ai/preferences"}}, {}};
            const auto changeId="preferences:"+task.at("id").get<std::string>()+":"+std::to_string(task.value("calls",0))+":"+std::to_string(std::hash<std::string>{}(args.dump()));
            return PreparedAgentTool{{{"changed",true},{"undoId",changeId},{"href","/my-ai/preferences"}},[body,before,changeId](Database& connection){
                const auto after=myAIWrite(connection,"preferences",body,false);
                connection.saveMyAIRecord({changeId,"undo",Json{{"before",before},{"afterVersion",after["version"]},{"state","applied"}}.dump(),after["updatedAt"],0},0);
            }};
        }});
    agent.registerTool("remember_preference",{"记住学生明确表达或从真实互动中推测的教学偏好。参数 content、source:explicit|inferred。推测必须有依据，手动要求优先；关闭自动记忆后不保存推测。",false,
        [](Database& db,const Json& args,const Json& task,const AIResult&){auto body=args;body["sourceId"]=task.at("requestId");
            for(const auto* field:{"chatId","lessonId"})if(task.at("event").contains(field))body[std::string("source")+(std::string(field)=="chatId"?"ChatId":"LessonId")]=task.at("event")[field];
            if(body.value("source","")!="explicit"&&body.value("source","")!="inferred")throw std::invalid_argument("记忆来源无效");
            if(body["source"]=="inferred"&&!myAIRead(db,"preferences").value("autoMemory",true))throw std::invalid_argument("自动记忆已关闭");
            return PreparedAgentTool{{{"remembered",true},{"href","/my-ai/memories"}},[body](Database& connection){myAIWrite(connection,"memories",body,false);}};
        }});
    agent.registerTool("propose_personal_action",{"按学生明确要求生成重要操作预览，等待页面确认后才执行。参数 kind:delete_course|delete_chat|clear_chats|clear_memories|settings|restore，target:真实课程或聊天ID；settings操作的target可填写建议模型名，reason:具体原因。不得声称已执行。",false,
        [](Database&,const Json& args,const Json& task,const AIResult&){
            if(task.at("event").value("type","")!="assistant_chat")throw std::invalid_argument("重要个人操作仅由管家受理");
            const auto key=task.at("id").get<std::string>()+":"+std::to_string(task.value("calls",0));
            return PreparedAgentTool{{{"requiresConfirmation",true},{"actionId","action:"+key},{"href","/my-ai/data"}},[args,key](Database& db){myAIProposal(db,args,key);}};
        }});
    agent.registerTool("open_personal_page",{"打开独立功能页面。参数 page:reports|plan|preferences|memories|tasks|usage|profile|settings|data；返回入口，由学生点击跳转。",true,
        [](Database&,const Json& args,const Json&,const AIResult&){const std::string page=args.at("page");
            if(page!="reports"&&page!="plan"&&page!="preferences"&&page!="memories"&&page!="tasks"&&page!="usage"&&page!="profile"&&page!="settings"&&page!="data")throw std::invalid_argument("页面不存在");
            return PreparedAgentTool{{{"href","/my-ai/"+page}}, {}};
        }});
}
}

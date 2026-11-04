// 【逐行批注】 引入头文件 "my_ai.hpp"，为后续实现提供类型或接口声明。
#include "my_ai.hpp"
// 【逐行批注】 引入头文件 "my_ai_context.hpp"，为后续实现提供类型或接口声明。
#include "my_ai_context.hpp"
// 【逐行批注】 引入头文件 "learning_agent.hpp"，为后续实现提供类型或接口声明。
#include "learning_agent.hpp"
// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 定义函数 myAIResumeControls，执行该函数声明的职责。
void myAIResumeControls(Database& db) {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json=MyAIJson;
    // 【逐行批注】 按循环规则重复处理：for(const auto& row:db.myAIRecords("control")) {。
    for(const auto& row:db.myAIRecords("control")) {
        // 【逐行批注】 计算右侧结果并写入 value，更新当前运行状态。
        auto value=Json::parse(row.payload);if(value.value("state","")!="pending")continue;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 auto args，更新当前运行状态。
            const auto args=value.at("args");AgentAccess access{value.at("scopeId"),value.at("courseIds").get<std::vector<std::string>>()};
            // 【逐行批注】 计算右侧结果并写入 bool current，更新当前运行状态。
            bool current=true;
            // 【逐行批注】 调用 pauseEpoch、setPaused 完成本行所需操作：if(args["command"]=="pause_all") {current=AIActivity::pauseEpoch()==v…。
            if(args["command"]=="pause_all") {current=AIActivity::pauseEpoch()==value.value("epoch",0ULL);if(current)AIActivity::setPaused(true);}
            // 【逐行批注】 执行前一条件不成立时的分支。
            else {const auto target=agentView(db,access,args.at("taskId"));current=target["version"]==value["targetVersion"];if(current)agentControl(db,access,args);}
            // 【逐行批注】 计算右侧结果并写入 value["state"]，更新当前运行状态。
            value["state"]=current?"applied":"superseded";
        // 【逐行批注】 计算右侧结果并写入 }catch(const std::exception& error){value["s…，更新当前运行状态。
        }catch(const std::exception& error){value["state"]="failed";value["error"]=error.what();}
        // 【逐行批注】 调用 runtime_error 完成本行所需操作：if(!db.saveMyAIRecord({row.id,row.kind,value.dump(),row.updatedAt,0},…。
        if(!db.saveMyAIRecord({row.id,row.kind,value.dump(),row.updatedAt,0},row.version))throw std::runtime_error("控制记录已经更新");
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 registerMyAITools，执行该函数声明的职责。
void registerMyAITools(LearningAgent& agent) {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json=MyAIJson;
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("control_ai_task",{"按学生明确指令暂停、继续或重试AI任务。参数 command:pause|resume|r…。
    agent.registerTool("control_ai_task",{"按学生明确指令暂停、继续或重试AI任务。参数 command:pause|resume|retry，taskId:现有任务ID。全局暂停使用 command:pause_all。",false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db,const Json& args,const Json& task,const AIResult&){。
        [](Database& db,const Json& args,const Json& task,const AIResult&){
            // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(task.at("event").value("type","")!="assistant_chat")throw std::inv…。
            if(task.at("event").value("type","")!="assistant_chat")throw std::invalid_argument("请在AI管家中发起控制请求");
            // 【逐行批注】 计算右侧结果并写入 auto command，更新当前运行状态。
            const auto command=args.value("command","");if(command!="pause"&&command!="resume"&&command!="retry"&&command!="pause_all")throw std::invalid_argument("控制命令无效");
            // 【逐行批注】 执行当前声明或表达式：AgentAccess access{task.at("scopeId"),task.at("courseIds").get<std::vector<std::stri…。
            AgentAccess access{task.at("scopeId"),task.at("courseIds").get<std::vector<std::string>>()};
            // 【逐行批注】 调用 agentView 完成本行所需操作：Json targetVersion;if(command!="pause_all")targetVersion=agentView(db…。
            Json targetVersion;if(command!="pause_all")targetVersion=agentView(db,access,args.at("taskId")).at("version");
            // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
            const auto key="control:"+task.at("id").get<std::string>()+":"+std::to_string(task.value("calls",0))+":"+command+":"+args.value("taskId","");
            // 【逐行批注】 计算右侧结果并写入 auto queued，更新当前运行状态。
            const auto queued=Json{{"args",args},{"scopeId",access.scopeId},{"courseIds",access.courseIds},{"targetVersion",targetVersion},{"epoch",AIActivity::pauseEpoch()},{"state","pending"}};
            // 【逐行批注】 计算右侧结果并写入 PreparedAgentTool tool;tool.result，更新当前运行状态。
            PreparedAgentTool tool;tool.result={{"command",command},{"controlId",key},{"href","/my-ai/tasks"}};
            // 【逐行批注】 计算右侧结果并写入 tool.commit，更新当前运行状态。
            tool.commit=[key,queued,task](Database& connection){if(!connection.myAIRecord(key))connection.saveMyAIRecord({key,"control",queued.dump(),task.at("createdAt"),0},0);};
            // 【逐行批注】 计算右侧结果并写入 tool.afterCommit，更新当前运行状态。
            tool.afterCommit=[key](Database& connection){myAIResumeControls(connection);const auto row=connection.myAIRecord(key);if(row){const auto result=Json::parse(row->payload);if(result.value("state","")!="applied")throw std::runtime_error(result.value("error","任务状态已变化，控制没有应用。"));}};return tool;
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("read_personal_center",{"读取个人档案、教学偏好、记忆、学习报告、调用统计和管家聊天。参数 section…。
    agent.registerTool("read_personal_center",{"读取个人档案、教学偏好、记忆、学习报告、调用统计和管家聊天。参数 section:overview|profile|preferences|memories|reports|usage|chats|tasks。",true,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db,const Json& args,const Json& task,const AIResult&){。
        [](Database& db,const Json& args,const Json& task,const AIResult&){
            // 【逐行批注】 调用 array、parse、agentView 完成本行所需操作：if(args["section"]=="tasks") {Json items=Json::array();const auto ids…。
            if(args["section"]=="tasks") {Json items=Json::array();const auto ids=Json::parse(db.profileMeta("agent-index"),nullptr,false);const AgentAccess access{task.at("scopeId"),task.at("courseIds").get<std::vector<std::string>>()};if(ids.is_array())for(const auto& key:ids)try{items.push_back(agentView(db,access,key.get<std::string>(),true));}catch(const std::exception&){}return PreparedAgentTool{{{"items",items}},{}};}
            // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
            auto result=myAIRead(db,args.at("section"));result.erase("avatar");if(result.contains("profile")&&result["profile"].is_object())result["profile"].erase("avatar");return PreparedAgentTool{result,{}};}});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("set_teaching_preferences",{"按学生当前明确要求调整教学偏好。参数 style、tone、pace、d…。
    agent.registerTool("set_teaching_preferences",{"按学生当前明确要求调整教学偏好。参数 style、tone、pace、detail、instructions、autoMemory；只传学生要求改动的字段。无需再次确认，可撤回。",false,
        // 【逐行批注】 执行当前声明或表达式：[](Database& db,const Json& args,const Json& task,const AIResult&){。
        [](Database& db,const Json& args,const Json& task,const AIResult&){
            // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(task.at("event").value("type","")!="assistant_chat")throw std::inv…。
            if(task.at("event").value("type","")!="assistant_chat")throw std::invalid_argument("请由AI管家执行明确的设置请求");
            // 【逐行批注】 计算右侧结果并写入 auto before，更新当前运行状态。
            const auto before=myAIRead(db,"preferences");auto body=args;body["version"]=before["version"];
            // 【逐行批注】 计算右侧结果并写入 bool different，更新当前运行状态。
            bool different=false;for(const auto& item:args.items())if(!before.contains(item.key())||before[item.key()]!=item.value())different=true;
            // 【逐行批注】 执行当前声明或表达式：if(!different)return PreparedAgentTool{{{"changed",false},{"href","/my-ai/preference…。
            if(!different)return PreparedAgentTool{{{"changed",false},{"href","/my-ai/preferences"}}, {}};
            // 【逐行批注】 计算右侧结果并写入 auto changeId，更新当前运行状态。
            const auto changeId="preferences:"+task.at("id").get<std::string>()+":"+std::to_string(task.value("calls",0))+":"+std::to_string(std::hash<std::string>{}(args.dump()));
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"changed",true},{"undoId",changeId},{"href…。
            return PreparedAgentTool{{{"changed",true},{"undoId",changeId},{"href","/my-ai/preferences"}},[body,before,changeId](Database& connection){
                // 【逐行批注】 计算右侧结果并写入 auto after，更新当前运行状态。
                const auto after=myAIWrite(connection,"preferences",body,false);
                // 【逐行批注】 执行当前声明或表达式：connection.saveMyAIRecord({changeId,"undo",Json{{"before",before},{"afterVersion",af…。
                connection.saveMyAIRecord({changeId,"undo",Json{{"before",before},{"afterVersion",after["version"]},{"state","applied"}}.dump(),after["updatedAt"],0},0);
            // 【逐行批注】 执行当前声明或表达式：}};。
            }};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("remember_preference",{"记住学生明确表达或从真实互动中推测的教学偏好。参数 content、source:…。
    agent.registerTool("remember_preference",{"记住学生明确表达或从真实互动中推测的教学偏好。参数 content、source:explicit|inferred。推测必须有依据，手动要求优先；关闭自动记忆后不保存推测。",false,
        // 【逐行批注】 计算右侧结果并写入 [](Database& db,const Json& args,const Json&…，更新当前运行状态。
        [](Database& db,const Json& args,const Json& task,const AIResult&){auto body=args;body["sourceId"]=task.at("requestId");
            // 【逐行批注】 按循环规则重复处理：for(const auto* field:{"chatId","lessonId"})if(task.at("event").contains(f…。
            for(const auto* field:{"chatId","lessonId"})if(task.at("event").contains(field))body[std::string("source")+(std::string(field)=="chatId"?"ChatId":"LessonId")]=task.at("event")[field];
            // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(body.value("source","")!="explicit"&&body.value("source","")!="inf…。
            if(body.value("source","")!="explicit"&&body.value("source","")!="inferred")throw std::invalid_argument("记忆来源无效");
            // 【逐行批注】 调用 myAIRead、invalid_argument 完成本行所需操作：if(body["source"]=="inferred"&&!myAIRead(db,"preferences").value("aut…。
            if(body["source"]=="inferred"&&!myAIRead(db,"preferences").value("autoMemory",true))throw std::invalid_argument("自动记忆已关闭");
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"remembered",true},{"href","/my-ai/memorie…。
            return PreparedAgentTool{{{"remembered",true},{"href","/my-ai/memories"}},[body](Database& connection){myAIWrite(connection,"memories",body,false);}};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("propose_personal_action",{"按学生明确要求生成重要操作预览，等待页面确认后才执行。参数 kind:de…。
    agent.registerTool("propose_personal_action",{"按学生明确要求生成重要操作预览，等待页面确认后才执行。参数 kind:delete_course|delete_chat|clear_chats|clear_memories|settings|restore，target:真实课程或聊天ID；settings操作的target可填写建议模型名，reason:具体原因。不得声称已执行。",false,
        // 【逐行批注】 执行当前声明或表达式：[](Database&,const Json& args,const Json& task,const AIResult&){。
        [](Database&,const Json& args,const Json& task,const AIResult&){
            // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(task.at("event").value("type","")!="assistant_chat")throw std::inv…。
            if(task.at("event").value("type","")!="assistant_chat")throw std::invalid_argument("重要个人操作仅由管家受理");
            // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
            const auto key=task.at("id").get<std::string>()+":"+std::to_string(task.value("calls",0));
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"requiresConfirmation",true},{"actionId","…。
            return PreparedAgentTool{{{"requiresConfirmation",true},{"actionId","action:"+key},{"href","/my-ai/data"}},[args,key](Database& db){myAIProposal(db,args,key);}};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("open_personal_page",{"打开独立功能页面。参数 page:library|reports|plan|pref…。
    agent.registerTool("open_personal_page",{"打开独立功能页面。参数 page:library|reports|plan|preferences|memories|tasks|usage|profile|settings|data；返回入口，由学生点击跳转。",true,
        // 【逐行批注】 计算右侧结果并写入 [](Database&,const Json& args,const Json&,co…，更新当前运行状态。
        [](Database&,const Json& args,const Json&,const AIResult&){const std::string page=args.at("page");
            // 【逐行批注】 调用 invalid_argument 完成本行所需操作：if(page!="library"&&page!="reports"&&page!="plan"&&page!="preferences…。
            if(page!="library"&&page!="reports"&&page!="plan"&&page!="preferences"&&page!="memories"&&page!="tasks"&&page!="usage"&&page!="profile"&&page!="settings"&&page!="data")throw std::invalid_argument("页面不存在");
            // 【逐行批注】 结束当前函数并返回结果：return PreparedAgentTool{{{"href","/my-ai/"+page}}, {}};。
            return PreparedAgentTool{{{"href","/my-ai/"+page}}, {}};
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

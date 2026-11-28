// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 <atomic>，为后续实现提供类型或接口声明。
#include <atomic>
// 【逐行批注】 引入头文件 <mutex>，为后续实现提供类型或接口声明。
#include <mutex>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <utility>，为后续实现提供类型或接口声明。
#include <utility>

// 【逐行批注】 定义函数 runAgentQuantumChecks，执行该函数声明的职责。
inline void runAgentQuantumChecks(const std::filesystem::path& path) {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 【逐行批注】 执行当前声明或表达式：Database db; db.open(path.string()); db.migrate();。
    Database db; db.open(path.string()); db.migrate();
    // 【逐行批注】 执行当前声明或表达式：const AgentAccess access{"quantum-fixture", {}};。
    const AgentAccess access{"quantum-fixture", {}};
    // 【逐行批注】 计算右侧结果并写入 task，更新当前运行状态。
    auto task = agentSubmit(db, access, {{"type", "inspect"}, {"requestId", "two-rounds"}});
    // 【逐行批注】 计算右侧结果并写入 int calls，更新当前运行状态。
    int calls = 0, commits = 0;
    // 【逐行批注】 定义函数 agent，执行该函数声明的职责。
    LearningAgent agent(path.string(), [&](const ChatOptions& options, const auto& sink) {
        // 【逐行批注】 调用 assert 完成本行所需操作：++calls; assert(options.maxAttempts == 1);。
        ++calls; assert(options.maxAttempts == 1);
        // 【逐行批注】 调用 assert、sink 完成本行所需操作：assert(sink("{"));。
        assert(sink("{"));
        // 【逐行批注】 调用 assert、agentView 完成本行所需操作：assert(!agentView(db, access, task.at("id")).contains("firstContentMs…。
        assert(!agentView(db, access, task.at("id")).contains("firstContentMs"));
        // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(30));。
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        // 【逐行批注】 计算右侧结果并写入 Json decision，更新当前运行状态。
        const Json decision = {{"message", calls == 1 ? "" : "已完成有效回复"},
            // 【逐行批注】 调用 array、object 完成本行所需操作：{"actions", calls == 1 ? Json::array({{{"id", "saved"}, {"tool", "sav…。
            {"actions", calls == 1 ? Json::array({{{"id", "saved"}, {"tool", "save_fixture"}, {"args", Json::object()}}}) : Json::array()},
            // 【逐行批注】 执行当前声明或表达式：{"state", calls == 1 ? "continue" : "waiting_student"}};。
            {"state", calls == 1 ? "continue" : "waiting_student"}};
        // 【逐行批注】 计算右侧结果并写入 encoded，更新当前运行状态。
        auto encoded = decision.dump(); assert(sink(encoded.substr(1)));
        // 【逐行批注】 执行当前声明或表达式：AIResult result;。
        AIResult result;
        // 【逐行批注】 计算右侧结果并写入 result.content，更新当前运行状态。
        result.content = encoded;
        // 【逐行批注】 计算右侧结果并写入 result.model，更新当前运行状态。
        result.model = "fixture";
        // 【逐行批注】 计算右侧结果并写入 result.status，更新当前运行状态。
        result.status = 200;
        // 【逐行批注】 计算右侧结果并写入 result.finishReason，更新当前运行状态。
        result.finishReason = "stop";
        // 【逐行批注】 结束当前函数并返回结果：return result;。
        return result;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册可由 AI 主控调用的工具：agent.registerTool("save_fixture", {"保存检查点夹具。", false, [&](Database&, const Json&, c…。
    agent.registerTool("save_fixture", {"保存检查点夹具。", false, [&](Database&, const Json&, const Json&, const AIResult&) {
        // 【逐行批注】 计算右侧结果并写入 PreparedAgentTool prepared; prepared.commit，更新当前运行状态。
        PreparedAgentTool prepared; prepared.commit = [&](Database& saved) {
            // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(30)); ++commits;。
            std::this_thread::sleep_for(std::chrono::milliseconds(30)); ++commits;
            // 【逐行批注】 执行当前声明或表达式：saved.setProfileMeta("quantum-committed", "yes");。
            saved.setProfileMeta("quantum-committed", "yes");
        // 【逐行批注】 执行当前声明或表达式：}; return prepared;。
        }; return prepared;
    // 【逐行批注】 执行当前声明或表达式：}});。
    }});
    // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(20)); agent.pro…。
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); agent.processQuantum(db, task.at("id"));
    // 【逐行批注】 计算右侧结果并写入 first，更新当前运行状态。
    auto first = agentView(db, access, task.at("id"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(calls == 1 && commits == 1 && first.at("calls") == 1 && first.…。
    assert(calls == 1 && commits == 1 && first.at("calls") == 1 && first.at("status") == "pending");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.profileMeta("quantum-committed") == "yes" && first.at("queu…。
    assert(db.profileMeta("quantum-committed") == "yes" && first.at("queueMs").get<int>() >= 15);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!first.contains("firstContentMs") && !first.contains("completi…。
    assert(!first.contains("firstContentMs") && !first.contains("completionMs"));
    // 【逐行批注】 执行当前声明或表达式：agent.processQuantum(db, task.at("id"));。
    agent.processQuantum(db, task.at("id"));
    // 【逐行批注】 计算右侧结果并写入 final，更新当前运行状态。
    auto final = agentView(db, access, task.at("id"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(calls == 2 && commits == 1 && final.at("calls") == 2 && final.…。
    assert(calls == 2 && commits == 1 && final.at("calls") == 2 && final.at("status") == "waiting_student");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(final.at("firstContentMs").get<int>() >= 75 && final.at("compl…。
    assert(final.at("firstContentMs").get<int>() >= 75 && final.at("completionMs") >= final.at("firstContentMs"));
    // 【逐行批注】 计算右侧结果并写入 failureTask，更新当前运行状态。
    auto failureTask = agentSubmit(db, access, {{"type", "inspect"}, {"requestId", "failure-budget"}});
    // 【逐行批注】 计算右侧结果并写入 int failures，更新当前运行状态。
    int failures = 0;
    // 【逐行批注】 声明对象 failing，并通过构造函数初始化其状态。
    LearningAgent failing(path.string(), [&](const auto&, const auto&) -> AIResult {
        // 【逐行批注】 调用 AIClientError 完成本行所需操作：++failures; throw AIClientError("provider_5xx", "本机服务异常", 500);。
        ++failures; throw AIClientError("provider_5xx", "本机服务异常", 500);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 按循环规则重复处理：for (int round = 0; round < 4; ++round) failing.processQuantum(db, failure…。
    for (int round = 0; round < 4; ++round) failing.processQuantum(db, failureTask.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto failed，更新当前运行状态。
    const auto failed = agentView(db, access, failureTask.at("id"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(failures == 3 && failed.at("calls") == 3 && failed.at("status"…。
    assert(failures == 3 && failed.at("calls") == 3 && failed.at("status") == "failed");
    // 【逐行批注】 计算右侧结果并写入 rejectedTask，更新当前运行状态。
    auto rejectedTask = agentSubmit(db, access, {{"type", "chat"}, {"requestId", "rolled-back-content"}, {"text", "只接收有效内容"}});
    // 【逐行批注】 定义函数 rejected，执行该函数声明的职责。
    LearningAgent rejected(path.string(), [&](const auto&, const auto& sink) {
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "尚未通过校验的文字"}, {"actions", {{{"id", "reject…。
        return output({{"message", "尚未通过校验的文字"}, {"actions", {{{"id", "reject"}, {"tool", "reject_fixture"}, {"args", Json::object()}}}}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 注册可由 AI 主控调用的工具：rejected.registerTool("reject_fixture", {"拒绝未通过校验的动作。", false, [](Database&, const J…。
    rejected.registerTool("reject_fixture", {"拒绝未通过校验的动作。", false, [](Database&, const Json&, const Json&, const AIResult&) {
        // 【逐行批注】 计算右侧结果并写入 PreparedAgentTool result; result.commit，更新当前运行状态。
        PreparedAgentTool result; result.commit = [](Database&) { throw std::runtime_error("测试事务回滚"); }; return result;
    // 【逐行批注】 执行当前声明或表达式：}});。
    }});
    // 【逐行批注】 执行当前声明或表达式：rejected.process(db, rejectedTask.at("id"));。
    rejected.process(db, rejectedTask.at("id"));
    // 【逐行批注】 计算右侧结果并写入 auto rejectedView，更新当前运行状态。
    const auto rejectedView = agentView(db, access, rejectedTask.at("id"));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(rejectedView.at("status") == "failed" && !rejectedView.contain…。
    // 草稿已实时显示，计时应包含它；动作事务仍须回滚，不能成为正式结果。
    assert(rejectedView.at("status") == "failed" && rejectedView.at("firstContentMs").get<long long>() >= 0);
    // 无本进程起点的恢复任务不能把持久化的旧单调值当作当前耗时。
    // 【逐行批注】 计算右侧结果并写入 auto legacyId，更新当前运行状态。
    const auto legacyId = "legacy-no-clock";
    // 【逐行批注】 计算右侧结果并写入 legacy，更新当前运行状态。
    auto legacy = Json::parse(db.getClassroomActivity(failureTask.at("id"))->payload);
    // 【逐行批注】 计算右侧结果并写入 legacy["id"]，更新当前运行状态。
    legacy["id"] = legacyId; legacy["status"] = "pending"; legacy.erase("queueMs"); legacy.erase("completionMs");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(db.upsert(ClassroomActivity{legacyId, "", "agent-task", legacy…。
    assert(db.upsert(ClassroomActivity{legacyId, "", "agent-task", legacy.dump(), "now", 0, 0}));
    // 【逐行批注】 执行当前声明或表达式：failing.processQuantum(db, legacyId);。
    failing.processQuantum(db, legacyId);
    // 【逐行批注】 计算右侧结果并写入 auto restored，更新当前运行状态。
    const auto restored = agentView(db, access, legacyId);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!restored.contains("queueMs") && !restored.contains("firstCont…。
    assert(!restored.contains("queueMs") && !restored.contains("firstContentMs") && !restored.contains("completionMs"));

    // 真正的worker以完整模型轮次和已保存动作作为切换点，前台事件逐个保留。
    // 【逐行批注】 计算右侧结果并写入 auto workerPath，更新当前运行状态。
    const auto workerPath = path.string() + ".worker";
    // 【逐行批注】 执行当前声明或表达式：Database workerDb; workerDb.open(workerPath); workerDb.migrate();。
    Database workerDb; workerDb.open(workerPath); workerDb.migrate();
    // 【逐行批注】 计算右侧结果并写入 background，更新当前运行状态。
    auto background = agentSubmit(workerDb, access, {{"type", "learning_updated"}, {"requestId", "background"}});
    // 【逐行批注】 执行当前声明或表达式：std::atomic_int backgroundCalls{0}, foregroundCalls{0}, activeCalls{0}, maxActiveCal…。
    std::atomic_int backgroundCalls{0}, foregroundCalls{0}, activeCalls{0}, maxActiveCalls{0};
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> order;。
    std::vector<std::string> order;
    // 【逐行批注】 执行当前声明或表达式：std::mutex orderMutex;。
    std::mutex orderMutex;
    // 两个工作线程可以并行执行不同任务，但同一任务绝不能被同时处理两次。
    // 【逐行批注】 执行当前声明或表达式：std::mutex activeIdsMutex;。
    std::mutex activeIdsMutex;
    // 【逐行批注】 执行当前声明或表达式：std::set<std::string> activeIds;。
    std::set<std::string> activeIds;
    // 【逐行批注】 计算右侧结果并写入 bool duplicateActive，更新当前运行状态。
    bool duplicateActive = false;
    // 【逐行批注】 声明数据结构或类型 ActiveTaskGuard 及其成员接口。
    struct ActiveTaskGuard {
        // 【逐行批注】 执行当前声明或表达式：std::mutex& mutex; std::set<std::string>& ids; std::string id; bool& duplicate;。
        std::mutex& mutex; std::set<std::string>& ids; std::string id; bool& duplicate;
        // 【逐行批注】 调用 ActiveTaskGuard 完成本行所需操作：ActiveTaskGuard(std::mutex& m, std::set<std::string>& s, std::string …。
        ActiveTaskGuard(std::mutex& m, std::set<std::string>& s, std::string value, bool& dup)
            // 【逐行批注】 定义函数 mutex，执行该函数声明的职责。
            : mutex(m), ids(s), id(std::move(value)), duplicate(dup) {
            // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
            std::lock_guard<std::mutex> lock(mutex);
            // 【逐行批注】 判断条件 (!ids.insert(id).second) duplicate = true;，成立时执行对应分支。
            if (!ids.insert(id).second) duplicate = true;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 ActiveTaskGuard 完成本行所需操作：~ActiveTaskGuard() {。
        ~ActiveTaskGuard() {
            // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
            std::lock_guard<std::mutex> lock(mutex);
            // 【逐行批注】 执行当前声明或表达式：ids.erase(id);。
            ids.erase(id);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 定义函数 worker，执行该函数声明的职责。
    LearningAgent worker(workerPath, [&](const ChatOptions& options, const auto& sink) {
        // 【逐行批注】 调用 guarded 完成本行所需操作：const ActiveTaskGuard guarded(activeIdsMutex, activeIds, options.acti…。
        const ActiveTaskGuard guarded(activeIdsMutex, activeIds, options.activity.taskId, duplicateActive);
        // 【逐行批注】 计算右侧结果并写入 int active，更新当前运行状态。
        const int active = ++activeCalls;
        // 【逐行批注】 计算右侧结果并写入 int previous，更新当前运行状态。
        int previous = maxActiveCalls.load();
        // 【逐行批注】 在条件成立期间循环处理：while (previous < active && !maxActiveCalls.compare_exchange_weak(previous…。
        while (previous < active && !maxActiveCalls.compare_exchange_weak(previous, active)) {}
        // 【逐行批注】 开始当前代码块。
        {
            // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
            std::lock_guard<std::mutex> lock(orderMutex);
            // 【逐行批注】 执行当前声明或表达式：order.push_back(options.activity.taskId);。
            order.push_back(options.activity.taskId);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(30));。
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        // 【逐行批注】 判断条件 (options.activity.taskId == background.at("id")) {，成立时执行对应分支。
        if (options.activity.taskId == background.at("id")) {
            // 【逐行批注】 判断条件 (++backgroundCalls == 1) {，成立时执行对应分支。
            if (++backgroundCalls == 1) {
                // 【逐行批注】 执行当前声明或表达式：Database incoming; incoming.open(workerPath);。
                Database incoming; incoming.open(workerPath);
                // 【逐行批注】 调用 agentSubmit 完成本行所需操作：agentSubmit(incoming, access, {{"type", "inspect"}, {"requestId", "fo…。
                agentSubmit(incoming, access, {{"type", "inspect"}, {"requestId", "foreground-one"}});
                // 【逐行批注】 调用 agentSubmit 完成本行所需操作：agentSubmit(incoming, access, {{"type", "inspect"}, {"requestId", "fo…。
                agentSubmit(incoming, access, {{"type", "inspect"}, {"requestId", "foreground-two"}});
                // 【逐行批注】 按循环规则重复处理：for (int wait = 0; wait < 150 && activeCalls < 2; ++wait)。
                for (int wait = 0; wait < 150 && activeCalls < 2; ++wait)
                    // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(10));。
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                // 【逐行批注】 执行当前声明或表达式：--activeCalls;。
                --activeCalls;
                // 【逐行批注】 结束当前函数并返回结果：return output({{"message", ""}, {"actions", Json::array()}, {"state",…。
                return output({{"message", ""}, {"actions", Json::array()}, {"state", "continue"}}, sink);
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 执行当前声明或表达式：} else ++foregroundCalls;。
        } else ++foregroundCalls;
        // 【逐行批注】 执行当前声明或表达式：--activeCalls;。
        --activeCalls;
        // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "处理完成"}, {"actions", Json::array()}, {"sta…。
        return output({{"message", "处理完成"}, {"actions", Json::array()}, {"state", "waiting_student"}}, sink);
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 执行当前声明或表达式：worker.start();。
    worker.start();
    // 【逐行批注】 计算右侧结果并写入 auto deadline，更新当前运行状态。
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    // 【逐行批注】 在条件成立期间循环处理：while ((backgroundCalls < 2 || foregroundCalls < 2) && std::chrono::steady…。
    while ((backgroundCalls < 2 || foregroundCalls < 2) && std::chrono::steady_clock::now() < deadline)
        // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(20));。
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    // 【逐行批注】 执行当前声明或表达式：worker.stop();。
    worker.stop();
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(backgroundCalls == 2 && foregroundCalls == 2 && order.size() >…。
    assert(backgroundCalls == 2 && foregroundCalls == 2 && order.size() >= 4);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(maxActiveCalls.load() >= 2);。
    assert(maxActiveCalls.load() >= 2);
    // 【逐行批注】 调用 assert、count 完成本行所需操作：assert(std::count(order.begin(), order.end(), background.at("id")) ==…。
    assert(std::count(order.begin(), order.end(), background.at("id")) == 2);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(!duplicateActive);。
    assert(!duplicateActive);

    // 使用真实工具推进学习版本，验证同轮第二动作、历史以及让出/完成均不被旧请求版本阻断。
    // 【逐行批注】 按循环规则重复处理：for (const auto* state : {"continue", "completed"}) {。
    for (const auto* state : {"continue", "completed"}) {
        // 【逐行批注】 计算右侧结果并写入 auto revisionPath，更新当前运行状态。
        const auto revisionPath = path.string() + ".revision." + state;
        // 【逐行批注】 执行当前声明或表达式：Database revisionDb; revisionDb.open(revisionPath); revisionDb.migrate();。
        Database revisionDb; revisionDb.open(revisionPath); revisionDb.migrate();
        // 【逐行批注】 执行当前声明或表达式：const AgentAccess revisionAccess{"revision-fixture", {"first-course", "second-course…。
        const AgentAccess revisionAccess{"revision-fixture", {"first-course", "second-course"}};
        // 【逐行批注】 按循环规则重复处理：for (const auto& courseId : revisionAccess.courseIds)。
        for (const auto& courseId : revisionAccess.courseIds)
            // 【逐行批注】 调用 assert 完成本行所需操作：assert(revisionDb.insert(Course{courseId, "fixture", {}, "原目标", "deep…。
            assert(revisionDb.insert(Course{courseId, "fixture", {}, "原目标", "deep", "课程",
                // 【逐行批注】 执行当前声明或表达式：{}, "ai", "active", "2026-10-04", "2026-10-04"}));。
                {}, "ai", "active", "2026-10-04", "2026-10-04"}));
        // 【逐行批注】 计算右侧结果并写入 auto revisionTask，更新当前运行状态。
        const auto revisionTask = agentSubmit(revisionDb, revisionAccess, {{"type", "adjust"}, {"requestId", "two-real-actions"}});
        // 【逐行批注】 计算右侧结果并写入 int revisionCalls，更新当前运行状态。
        int revisionCalls = 0;
        // 【逐行批注】 定义函数 advancing，执行该函数声明的职责。
        LearningAgent advancing(revisionPath, [&](const auto&, const auto& sink) {
            // 【逐行批注】 执行当前声明或表达式：++revisionCalls;。
            ++revisionCalls;
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", "两个课程目标均已调整。"}, {"actions", revisionCalls …。
            return output({{"message", "两个课程目标均已调整。"}, {"actions", revisionCalls == 1 ? Json::array({
                // 【逐行批注】 执行当前声明或表达式：{{"id", "first"}, {"tool", "adjust_goal"}, {"args", {{"courseId", "first-course"}, {…。
                {{"id", "first"}, {"tool", "adjust_goal"}, {"args", {{"courseId", "first-course"}, {"goal", "新目标一"}, {"reason", "按学习反馈调整第一门课程"}}}},
                // 【逐行批注】 执行当前声明或表达式：{{"id", "second"}, {"tool", "adjust_goal"}, {"args", {{"courseId", "second-course"},…。
                {{"id", "second"}, {"tool", "adjust_goal"}, {"args", {{"courseId", "second-course"}, {"goal", "新目标二"}, {"reason", "按学习反馈调整第二门课程"}}}}
            // 【逐行批注】 调用 array 完成本行所需操作：}) : Json::array()}, {"state", revisionCalls == 1 ? state : "complete…。
            }) : Json::array()}, {"state", revisionCalls == 1 ? state : "completed"}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 registerAgentPreferenceTools 完成本行所需操作：registerAgentPreferenceTools(advancing);。
        registerAgentPreferenceTools(advancing);
        // 【逐行批注】 计算右侧结果并写入 auto beforeRevision，更新当前运行状态。
        const auto beforeRevision = revisionDb.learningRevision();
        // 【逐行批注】 执行当前声明或表达式：advancing.processQuantum(revisionDb, revisionTask.at("id"));。
        advancing.processQuantum(revisionDb, revisionTask.at("id"));
        // 【逐行批注】 计算右侧结果并写入 auto saved，更新当前运行状态。
        const auto saved = Json::parse(revisionDb.getClassroomActivity(revisionTask.at("id"))->payload);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(revisionCalls == 1 && revisionDb.learningRevision() >= beforeR…。
        assert(revisionCalls == 1 && revisionDb.learningRevision() >= beforeRevision + 2);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(revisionDb.getCourse("first-course")->goal == "新目标一" && revisi…。
        assert(revisionDb.getCourse("first-course")->goal == "新目标一" && revisionDb.getCourse("second-course")->goal == "新目标二");
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(saved.at("learningVersion") == revisionDb.learningRevision());。
        assert(saved.at("learningVersion") == revisionDb.learningRevision());
        // 【逐行批注】 调用 assert、string 完成本行所需操作：assert(saved.at("status") == (std::string(state) == "continue" ? "pen…。
        assert(saved.at("status") == (std::string(state) == "continue" ? "pending" : "ready"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(saved.at("completedActions").size() == 2 && saved.at("changes"…。
        assert(saved.at("completedActions").size() == 2 && saved.at("changes").size() == 2);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(saved.at("results").size() == 1 && saved.at("results")[0].size…。
        assert(saved.at("results").size() == 1 && saved.at("results")[0].size() == 2);
        // 【逐行批注】 调用 assert、parse 完成本行所需操作：assert(Json::parse(saved.at("messages").back().at("content").get<std:…。
        assert(Json::parse(saved.at("messages").back().at("content").get<std::string>()).at("toolResults").size() == 2);
        // 【逐行批注】 计算右侧结果并写入 int actionEvents，更新当前运行状态。
        int actionEvents = 0;
        // 【逐行批注】 按循环规则重复处理：for (const auto& event : saved.at("events")) if (event.value("type", "") =…。
        for (const auto& event : saved.at("events")) if (event.value("type", "") == "action") ++actionEvents;
        // 【逐行批注】 调用 assert、agentChanges 完成本行所需操作：assert(actionEvents == 2 && agentChanges(revisionDb, revisionAccess).…。
        assert(actionEvents == 2 && agentChanges(revisionDb, revisionAccess).size() == 2);
        // 【逐行批注】 执行当前声明或表达式：advancing.processQuantum(revisionDb, revisionTask.at("id"));。
        advancing.processQuantum(revisionDb, revisionTask.at("id"));
        // 【逐行批注】 计算右侧结果并写入 auto completed，更新当前运行状态。
        const auto completed = agentView(revisionDb, revisionAccess, revisionTask.at("id"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(completed.at("status") == "ready" && completed.contains("compl…。
        assert(completed.at("status") == "ready" && completed.contains("completionMs"));
        // 【逐行批注】 调用 assert、string 完成本行所需操作：assert(revisionCalls == (std::string(state) == "continue" ? 2 : 1));。
        assert(revisionCalls == (std::string(state) == "continue" ? 2 : 1));
        // 【逐行批注】 调用 assert、agentChanges 完成本行所需操作：assert(agentChanges(revisionDb, revisionAccess).size() == 2);。
        assert(agentChanges(revisionDb, revisionAccess).size() == 2);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto* command : {"pause", "cancel"}) for (const auto* state : {…。
    for (const auto* command : {"pause", "cancel"}) for (const auto* state : {"continue", "waiting_student"}) {
        // 【逐行批注】 计算右侧结果并写入 auto controlPath，更新当前运行状态。
        const auto controlPath = path.string() + "." + command + "." + state;
        // 【逐行批注】 执行当前声明或表达式：Database controlledDb; controlledDb.open(controlPath); controlledDb.migrate();。
        Database controlledDb; controlledDb.open(controlPath); controlledDb.migrate();
        // 【逐行批注】 计算右侧结果并写入 auto controlled，更新当前运行状态。
        const auto controlled = agentSubmit(controlledDb, access, {{"type", "chat"}, {"text", "确定性并发控制"}, {"requestId", "control"}});
        // 【逐行批注】 计算右侧结果并写入 int firstWrites，更新当前运行状态。
        int firstWrites = 0, laterWrites = 0, laterPrepares = 0;
        // 【逐行批注】 执行当前声明或表达式：std::string controlledPayload;。
        std::string controlledPayload;
        // 【逐行批注】 定义函数 guarded，执行该函数声明的职责。
        LearningAgent guarded(controlPath, [&](const auto&, const auto& sink) {
            // 【逐行批注】 结束当前函数并返回结果：return output({{"message", ""}, {"actions", {。
            return output({{"message", ""}, {"actions", {
                // 【逐行批注】 执行当前声明或表达式：{{"id", "first"}, {"tool", "append_section"}, {"args", {{"section", {{"kind", "expla…。
                {{"id", "first"}, {"tool", "append_section"}, {"args", {{"section", {{"kind", "explanation"}, {"body", "已提交的有效板块"}}}}}},
                // 【逐行批注】 调用 object 完成本行所需操作：{{"id", "later"}, {"tool", "save_remaining"}, {"args", Json::object()…。
                {{"id", "later"}, {"tool", "save_remaining"}, {"args", Json::object()}}}}, {"state", state}}, sink);
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 注册可由 AI 主控调用的工具：guarded.registerTool("append_section", {"保存首个有效板块。", false, [&](Database&, const Jso…。
        guarded.registerTool("append_section", {"保存首个有效板块。", false, [&](Database&, const Json&, const Json&, const AIResult&) {
            // 【逐行批注】 执行当前声明或表达式：PreparedAgentTool prepared;。
            PreparedAgentTool prepared;
            // 【逐行批注】 计算右侧结果并写入 prepared.commit，更新当前运行状态。
            prepared.commit = [&](Database& saved) { ++firstWrites; saved.setProfileMeta("first-section", "committed"); };
            // 【逐行批注】 计算右侧结果并写入 prepared.afterCommit，更新当前运行状态。
            prepared.afterCommit = [&](Database&) {
                // 此门闩位于首动作已提交、独立计时事务尚未开始的确定性窗口。
                // 【逐行批注】 执行当前声明或表达式：std::exception_ptr failure;。
                std::exception_ptr failure;
                // 【逐行批注】 声明对象 controller，并通过构造函数初始化其状态。
                std::thread controller([&] {
                    // 【逐行批注】 开始可能抛出异常的保护代码块。
                    try {
                        // 【逐行批注】 执行当前声明或表达式：Database incoming; incoming.open(controlPath);。
                        Database incoming; incoming.open(controlPath);
                        // 【逐行批注】 计算右侧结果并写入 auto before，更新当前运行状态。
                        const auto before = Json::parse(incoming.getClassroomActivity(controlled.at("id"))->payload);
                        // 【逐行批注】 调用 assert 完成本行所需操作：assert(incoming.profileMeta("first-section") == "committed" && before…。
                        assert(incoming.profileMeta("first-section") == "committed" && before.at("completedActions").contains("first"));
                        // 【逐行批注】 调用 assert 完成本行所需操作：assert(!before.contains("firstContentMs"));。
                        assert(before.at("firstContentMs").get<long long>() >= 0);
                        // 【逐行批注】 调用 agentControl 完成本行所需操作：agentControl(incoming, access, {{"command", command}, {"taskId", cont…。
                        agentControl(incoming, access, {{"command", command}, {"taskId", controlled.at("id")}});
                        // 【逐行批注】 计算右侧结果并写入 controlledPayload，更新当前运行状态。
                        controlledPayload = incoming.getClassroomActivity(controlled.at("id"))->payload;
                    // 【逐行批注】 计算右侧结果并写入 } catch (...) { failure，更新当前运行状态。
                    } catch (...) { failure = std::current_exception(); }
                // 【逐行批注】 执行当前声明或表达式：});。
                });
                // 【逐行批注】 调用 rethrow_exception 完成本行所需操作：controller.join(); if (failure) std::rethrow_exception(failure);。
                controller.join(); if (failure) std::rethrow_exception(failure);
            // 【逐行批注】 执行当前声明或表达式：};。
            };
            // 【逐行批注】 结束当前函数并返回结果：return prepared;。
            return prepared;
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
        // 【逐行批注】 注册可由 AI 主控调用的工具：guarded.registerTool("save_remaining", {"不应在控制操作后继续准备或保存。", false, [&](Database&, co…。
        guarded.registerTool("save_remaining", {"不应在控制操作后继续准备或保存。", false, [&](Database&, const Json&, const Json&, const AIResult&) {
            // 【逐行批注】 执行当前声明或表达式：++laterPrepares; PreparedAgentTool prepared;。
            ++laterPrepares; PreparedAgentTool prepared;
            // 【逐行批注】 计算右侧结果并写入 prepared.commit，更新当前运行状态。
            prepared.commit = [&](Database&) { ++laterWrites; }; return prepared;
        // 【逐行批注】 执行当前声明或表达式：}});。
        }});
        // 【逐行批注】 执行当前声明或表达式：guarded.processQuantum(controlledDb, controlled.at("id"));。
        guarded.processQuantum(controlledDb, controlled.at("id"));
        // 【逐行批注】 计算右侧结果并写入 auto stopped，更新当前运行状态。
        const auto stopped = agentView(controlledDb, access, controlled.at("id"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(firstWrites == 1 && laterWrites == 0 && laterPrepares == 0);。
        assert(firstWrites == 1 && laterWrites == 0 && laterPrepares == 0);
        // 【逐行批注】 调用 assert、string 完成本行所需操作：assert(stopped.at("status") == (std::string(command) == "pause" ? "pa…。
        assert(stopped.at("status") == (std::string(command) == "pause" ? "paused" : "cancelled"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(controlledDb.getClassroomActivity(controlled.at("id"))->payloa…。
        assert(controlledDb.getClassroomActivity(controlled.at("id"))->payload == controlledPayload);
        // 【逐行批注】 执行当前声明或表达式：guarded.processQuantum(controlledDb, controlled.at("id"));。
        guarded.processQuantum(controlledDb, controlled.at("id"));
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(controlledDb.getClassroomActivity(controlled.at("id"))->payloa…。
        assert(controlledDb.getClassroomActivity(controlled.at("id"))->payload == controlledPayload);
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

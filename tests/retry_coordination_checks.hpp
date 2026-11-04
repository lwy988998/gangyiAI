// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 <ctime>，为后续实现提供类型或接口声明。
#include <ctime>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>

// 【逐行批注】 定义函数 runRetryCoordinationChecks，执行该函数声明的职责。
static void runRetryCoordinationChecks() {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 【逐行批注】 计算右侧结果并写入 using Clock，更新当前运行状态。
    using Clock = std::chrono::steady_clock;
    // 【逐行批注】 计算右侧结果并写入 auto normal，更新当前运行状态。
    const auto normal = response(200, R"({"model":"fixture","choices":[{"message":{"content":"OK"},"finish_reason":"stop"}]})");
    // 【逐行批注】 计算右侧结果并写入 auto stream，更新当前运行状态。
    const auto stream = response(200, "data: {\"choices\":[{\"delta\":{\"content\":\"OK\"},\"finish_reason\":\"stop\"}]}\n\ndata: [DONE]\n\n");
    // 【逐行批注】 计算右侧结果并写入 auto limited，更新当前运行状态。
    const auto limited = [&](const std::string& value) {
        // 【逐行批注】 计算右侧结果并写入 text，更新当前运行状态。
        auto text = response(429, "{}"); text.insert(text.find("\r\n") + 2, "rEtRy-AfTeR: " + value + "\r\n"); return text;
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 auto failOnce，更新当前运行状态。
    const auto failOnce = [](AIClient& client, ChatOptions options, const std::string& kind) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { client.chat(options); expect(false, "失败夹具不应成功"); }
        // 【逐行批注】 捕获并处理异常：catch (const AIClientError& error) { expect(error.errorType == kind, …。
        catch (const AIClientError& error) { expect(error.errorType == kind, "公共退避错误分类应保持"); }
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 ChatOptions options; options.messages，更新当前运行状态。
    ChatOptions options; options.messages = {{"user", "本机冷却夹具"}}; options.maxAttempts = 1;
    // 【逐行批注】 按循环规则重复处理：for (const auto* kind : {"network_error", "invalid_response"}) {。
    for (const auto* kind : {"network_error", "invalid_response"}) {
        // 【逐行批注】 声明数据结构或类型 LocalCallbackError 及其成员接口。
        struct LocalCallbackError : AIClientError {
            // 【逐行批注】 定义函数 LocalCallbackError，执行该函数声明的职责。
            explicit LocalCallbackError(const std::string& kind) : AIClientError(kind, "回调原始异常", 599) {}
            // 【逐行批注】 计算右侧结果并写入 int marker，更新当前运行状态。
            int marker = 77;
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{stream, normal});
        // 【逐行批注】 计算右侧结果并写入 retry，更新当前运行状态。
        auto retry = options; retry.maxAttempts = 3; retry.activity.taskId = std::string("callback-origin-") + kind;
        // 【逐行批注】 计算右侧结果并写入 int chunks，更新当前运行状态。
        int chunks = 0; const auto calls = AIActivity::snapshot().at("todayCalls").get<int>();
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 调用 clientFor、LocalCallbackError 完成本行所需操作：clientFor(server).chatStream(retry, [&](const auto&) -> bool { ++chun…。
            clientFor(server).chatStream(retry, [&](const auto&) -> bool { ++chunks; throw LocalCallbackError(kind); });
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(false, "回调异常应传播给调用方");。
            expect(false, "回调异常应传播给调用方");
        // 【逐行批注】 执行当前声明或表达式：} catch (const LocalCallbackError& error) {。
        } catch (const LocalCallbackError& error) {
            // 【逐行批注】 调用 expect、string 完成本行所需操作：expect(error.marker == 77 && error.errorType == kind && error.httpSta…。
            expect(error.marker == 77 && error.errorType == kind && error.httpStatus == 599 && std::string(error.what()) == "回调原始异常",
                // 【逐行批注】 执行当前声明或表达式："回调异常的动态类型和原字段必须完整保留");。
                "回调异常的动态类型和原字段必须完整保留");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 expect、snapshot 完成本行所需操作：expect(chunks == 1 && server.requestCount() == 1 && AIActivity::snaps…。
        expect(chunks == 1 && server.requestCount() == 1 && AIActivity::snapshot().at("todayCalls") == calls + 1,
            // 【逐行批注】 执行当前声明或表达式："可重试类型的回调异常也只能一次HTTP和一次交付");。
            "可重试类型的回调异常也只能一次HTTP和一次交付");
        // 【逐行批注】 计算右侧结果并写入 auto begin，更新当前运行状态。
        const auto begin = Clock::now();
        // 【逐行批注】 调用 expect、clientFor、now、milliseconds 完成本行所需操作：expect(clientFor(server).chat(options).content == "OK" && Clock::now(…。
        expect(clientFor(server).chat(options).content == "OK" && Clock::now() - begin < std::chrono::milliseconds(650),
            // 【逐行批注】 执行当前声明或表达式："回调异常不得污染同配置的服务商冷却");。
            "回调异常不得污染同配置的服务商冷却");
        // 【逐行批注】 调用 expect、snapshot 完成本行所需操作：expect(server.requestCount() == 2 && AIActivity::snapshot().at("today…。
        expect(server.requestCount() == 2 && AIActivity::snapshot().at("todayCalls") == calls + 2, "回调失败和后续成功仍各结算一次HTTP");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{limited("1"), normal, stream});
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server); failOnce(client, options, "rate_limited");
        // 【逐行批注】 计算右侧结果并写入 auto begin，更新当前运行状态。
        const auto begin = Clock::now();
        // 【逐行批注】 计算右侧结果并写入 waiting，更新当前运行状态。
        auto waiting = std::async(std::launch::async, [&] { return clientFor(server).chatStream(options, [](const auto&) { return true; }); });
        // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(30));。
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        // 【逐行批注】 调用 expect、clientFor、now、milliseconds 完成本行所需操作：expect(clientFor(server, "/v1", "other-model").chat(options).content …。
        expect(clientFor(server, "/v1", "other-model").chat(options).content == "OK" && Clock::now() - begin < std::chrono::milliseconds(650),
            // 【逐行批注】 执行当前声明或表达式："一个调用等待时公共互斥锁不能阻塞其它配置");。
            "一个调用等待时公共互斥锁不能阻塞其它配置");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(waiting.get().content == "OK", "流式应复用普通调用冷却");。
        expect(waiting.get().content == "OK", "流式应复用普通调用冷却");
        // 【逐行批注】 调用 expect、now、milliseconds 完成本行所需操作：expect(Clock::now() - begin >= std::chrono::milliseconds(850), "最后一次4…。
        expect(Clock::now() - begin >= std::chrono::milliseconds(850), "最后一次429仍应使同配置新实例等待");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requestCount() == 3, "共享退避不能重复计算HTTP");。
        expect(server.requestCount() == 3, "共享退避不能重复计算HTTP");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& header : {std::string("invalid"), std::string("-3"), std:…。
    for (const auto& header : {std::string("invalid"), std::string("-3"), std::string("99999999999999999999999")}) {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{limited(header), normal}); auto client = clientFor(server);
        // 【逐行批注】 计算右侧结果并写入 retry，更新当前运行状态。
        auto retry = options; retry.maxAttempts = 2; const auto begin = Clock::now();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(client.chat(retry).content == "OK", "无效Retry-After仍按原策略重试");。
        expect(client.chat(retry).content == "OK", "无效Retry-After仍按原策略重试");
        // 【逐行批注】 调用 expect、now、seconds 完成本行所需操作：expect(Clock::now() - begin < std::chrono::seconds(3), "无效或溢出头不得产生巨大等…。
        expect(Clock::now() - begin < std::chrono::seconds(3), "无效或溢出头不得产生巨大等待");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 auto date，更新当前运行状态。
        const auto date = std::time(nullptr) + 2; std::tm utc{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
        // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&utc, &date);。
        gmtime_s(&utc, &date);
// 【逐行批注】 处理条件编译指令：#else。
#else
        // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&date, &utc);。
        gmtime_r(&date, &utc);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
        // 【逐行批注】 调用 put_time 完成本行所需操作：std::ostringstream formatted; formatted << std::put_time(&utc, "%a, %…。
        std::ostringstream formatted; formatted << std::put_time(&utc, "%a, %d %b %Y %H:%M:%S GMT");
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{limited(formatted.str()), normal}); auto client = clientFor(server);
        // 【逐行批注】 计算右侧结果并写入 failOnce(client, options, "rate_limited"); c…，更新当前运行状态。
        failOnce(client, options, "rate_limited"); const auto begin = Clock::now(); client.chat(options);
        // 【逐行批注】 调用 expect、now、milliseconds 完成本行所需操作：expect(Clock::now() - begin >= std::chrono::milliseconds(850), "HTTP日…。
        expect(Clock::now() - begin >= std::chrono::milliseconds(850), "HTTP日期Retry-After应生效");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto* isolation : {"model", "credential", "url"}) {。
    for (const auto* isolation : {"model", "credential", "url"}) {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{limited("2"), normal}); auto client = clientFor(server);
        // 【逐行批注】 计算右侧结果并写入 failOnce(client, options, "rate_limited"); c…，更新当前运行状态。
        failOnce(client, options, "rate_limited"); const auto begin = Clock::now();
        // 【逐行批注】 声明对象 isolated，并通过构造函数初始化其状态。
        AIClient isolated({"http://127.0.0.1:" + std::to_string(server.port()) + (std::string(isolation) == "url" ? "/other" : "/v1"),
            // 【逐行批注】 调用 string 完成本行所需操作：std::string(isolation) == "credential" ? "other-fixture-secret" : "te…。
            std::string(isolation) == "credential" ? "other-fixture-secret" : "test-secret-key",
            // 【逐行批注】 调用 string 完成本行所需操作：std::string(isolation) == "model" ? "other-model" : "unused", 1000, 1…。
            std::string(isolation) == "model" ? "other-model" : "unused", 1000, 1});
        // 【逐行批注】 调用 expect、now、milliseconds 完成本行所需操作：expect(isolated.chat(options).content == "OK" && Clock::now() - begin…。
        expect(isolated.chat(options).content == "OK" && Clock::now() - begin < std::chrono::milliseconds(650), "不同URL、模型、凭据的冷却必须隔离");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto code : {401, 402}) {。
    for (const auto code : {401, 402}) {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{response(code, "{}"), normal}); auto client = clientFor(server);
        // 【逐行批注】 计算右侧结果并写入 retry，更新当前运行状态。
        auto retry = options; retry.maxAttempts = 3;
        // 【逐行批注】 调用 failOnce、now 完成本行所需操作：failOnce(client, retry, code == 401 ? "auth_error" : "provider_reject…。
        failOnce(client, retry, code == 401 ? "auth_error" : "provider_rejected"); const auto begin = Clock::now();
        // 【逐行批注】 调用 expect、now、milliseconds 完成本行所需操作：expect(client.chat(options).content == "OK" && Clock::now() - begin <…。
        expect(client.chat(options).content == "OK" && Clock::now() - begin < std::chrono::milliseconds(650), "认证和402不重试且不污染冷却");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requestCount() == 2, "认证或402只能发一次故障请求");。
        expect(server.requestCount() == 2, "认证或402只能发一次故障请求");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{limited("1"), stream}); auto client = clientFor(server);
        // 【逐行批注】 调用 failOnce 完成本行所需操作：failOnce(client, options, "rate_limited");。
        failOnce(client, options, "rate_limited");
        // 【逐行批注】 计算右侧结果并写入 cancel，更新当前运行状态。
        auto cancel = options; const auto begin = Clock::now();
        // 【逐行批注】 计算右侧结果并写入 cancel.cancelled，更新当前运行状态。
        cancel.cancelled = [&] { return Clock::now() - begin > std::chrono::milliseconds(70); };
        // 【逐行批注】 调用 failOnce、expect 完成本行所需操作：failOnce(client, cancel, "cancelled"); expect(server.requestCount() =…。
        failOnce(client, cancel, "cancelled"); expect(server.requestCount() == 1, "等待中取消不得新增HTTP");
        // 【逐行批注】 计算右侧结果并写入 paused，更新当前运行状态。
        auto paused = options; paused.activity.taskId = "cooldown-paused-fixture";
        // 【逐行批注】 计算右侧结果并写入 outcome，更新当前运行状态。
        auto outcome = std::async(std::launch::async, [&] {
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try { client.chatStream(paused, [](const auto&) { return true; }); return std::string("completed"); }
            // 【逐行批注】 捕获并处理异常：catch (const AIClientError& error) { return error.errorType; }。
            catch (const AIClientError& error) { return error.errorType; }
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 sleep_for、milliseconds、setPaused 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(50)); AIActivit…。
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); AIActivity::setPaused(true);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(outcome.get() == "paused" && server.requestCount() == 1, "冷却暂停…。
        expect(outcome.get() == "paused" && server.requestCount() == 1, "冷却暂停须退出且零新增HTTP");
        // 【逐行批注】 调用 setPaused 完成本行所需操作：AIActivity::setPaused(false);。
        AIActivity::setPaused(false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(client.chatStream(paused, [](const auto&) { return true; }).co…。
        expect(client.chatStream(paused, [](const auto&) { return true; }).content == "OK", "恢复后只请求一次");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, "data: {\"choices\":[{\"delta\":{\"content\":\"已交付\"}}]}\n\n"));
        // 【逐行批注】 计算右侧结果并写入 retry，更新当前运行状态。
        auto retry = options; retry.maxAttempts = 3; int chunks = 0;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { clientFor(server).chatStream(retry, [&](const auto&) { ++chunks; return true; }); expect(false, "半流应失败"); }
        // 【逐行批注】 捕获并处理异常：catch (const AIClientError& error) { expect(error.contentDelivered, "…。
        catch (const AIClientError& error) { expect(error.contentDelivered, "部分流错误必须携带已交付标记"); }
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(chunks == 1 && server.requestCount() == 1, "部分流断开不得重放相同回调");。
        expect(chunks == 1 && server.requestCount() == 1, "部分流断开不得重放相同回调");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{"HTTP/1.1 100 Continue\r\nRetry-After: 10\r\n\r\n" + limited("invalid"), normal});
        // 【逐行批注】 计算右侧结果并写入 retry，更新当前运行状态。
        auto retry = options; retry.maxAttempts = 2; const auto begin = Clock::now();
        // 【逐行批注】 调用 expect、clientFor、now、seconds 完成本行所需操作：expect(clientFor(server).chat(retry).content == "OK" && Clock::now() …。
        expect(clientFor(server).chat(retry).content == "OK" && Clock::now() - begin < std::chrono::seconds(3),
            // 【逐行批注】 执行当前声明或表达式："新状态块应清除前一个响应头中的Retry-After");。
            "新状态块应清除前一个响应头中的Retry-After");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{response(400, "stream_options unsupported"), response(500, "{}"), stream});
        // 【逐行批注】 计算右侧结果并写入 retry，更新当前运行状态。
        auto retry = options; retry.maxAttempts = 2;
        // 【逐行批注】 调用 expect、clientFor 完成本行所需操作：expect(clientFor(server).chatStream(retry, [](const auto&) { return t…。
        expect(clientFor(server).chatStream(retry, [](const auto&) { return true; }).content == "OK", "兼容重发后仍可重试服务商故障");
        // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
        const auto first = server.request().find("stream_options");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(first != std::string::npos && server.request().find("stream_op…。
        expect(first != std::string::npos && server.request().find("stream_options", first + 1) == std::string::npos && server.requestCount() == 3,
            // 【逐行批注】 执行当前声明或表达式："兼容重发独立计HTTP且后续故障重试不能重新发送被拒选项");。
            "兼容重发独立计HTTP且后续故障重试不能重新发送被拒选项");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(stream); int chunks = 0;
        // 【逐行批注】 计算右侧结果并写入 result，更新当前运行状态。
        auto result = std::async(std::launch::async, [&] {
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try { clientFor(server).chatStream(options, [&](const auto&) { ++chunks; AIActivity::setPaused(true); return true; }); return std::string("completed"); }
            // 【逐行批注】 捕获并处理异常：catch (const AIClientError& error) { return error.errorType; }。
            catch (const AIClientError& error) { return error.errorType; }
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 expect、seconds 完成本行所需操作：expect(result.wait_for(std::chrono::seconds(2)) == std::future_status…。
        expect(result.wait_for(std::chrono::seconds(2)) == std::future_status::ready, "独立流已交付内容后暂停应立即退出，不能内部等待恢复重放");
        // 【逐行批注】 调用 setPaused 完成本行所需操作：AIActivity::setPaused(false);。
        AIActivity::setPaused(false);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result.get() == "paused" && chunks == 1 && server.requestCount…。
        expect(result.get() == "paused" && chunks == 1 && server.requestCount() == 1, "部分流暂停必须保留一次交付且零自动重放");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{response(500, "{}"), response(500, "{}"), response(500, "{}"), normal});
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server);
        // 【逐行批注】 调用 failOnce 完成本行所需操作：failOnce(client, options, "provider_5xx");。
        failOnce(client, options, "provider_5xx");
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { clientFor(server).chatStream(options, [](const auto&) { return true; }); expect(false, "流式故障夹具应失败"); }
        // 【逐行批注】 捕获并处理异常：catch (const AIClientError& error) { expect(error.errorType == "provi…。
        catch (const AIClientError& error) { expect(error.errorType == "provider_5xx", "流式应共享最终失败计数"); }
        // 【逐行批注】 调用 failOnce 完成本行所需操作：failOnce(client, options, "provider_5xx");。
        failOnce(client, options, "provider_5xx");
        // 超过普通800ms后仍不能发出第四笔，证明新实例没有重置三次失败的保护。
        // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(900));。
        std::this_thread::sleep_for(std::chrono::milliseconds(900));
        // 【逐行批注】 计算右侧结果并写入 auto begin，更新当前运行状态。
        const auto begin = Clock::now(); auto cancelled = options;
        // 【逐行批注】 计算右侧结果并写入 cancelled.cancelled，更新当前运行状态。
        cancelled.cancelled = [&] { return Clock::now() - begin > std::chrono::milliseconds(150); };
        // 【逐行批注】 计算右侧结果并写入 next，更新当前运行状态。
        auto next = clientFor(server); failOnce(next, cancelled, "cancelled");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requestCount() == 3, "跨普通流式连续三次故障应共享60秒冷却，第四次取消零HTTP");。
        expect(server.requestCount() == 3, "跨普通流式连续三次故障应共享60秒冷却，第四次取消零HTTP");
        // 【逐行批注】 调用 expect、clientFor 完成本行所需操作：expect(clientFor(server, "/v1", "unaffected-model").chat(options).con…。
        expect(clientFor(server, "/v1", "unaffected-model").chat(options).content == "OK", "持续故障保护不能影响其它模型");
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

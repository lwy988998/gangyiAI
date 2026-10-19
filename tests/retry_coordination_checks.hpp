#pragma once
#include <ctime>
#include <iomanip>
#include <sstream>

static void runRetryCoordinationChecks() {
    using namespace gangyi;
    using Clock = std::chrono::steady_clock;
    const auto normal = response(200, R"({"model":"fixture","choices":[{"message":{"content":"OK"},"finish_reason":"stop"}]})");
    const auto stream = response(200, "data: {\"choices\":[{\"delta\":{\"content\":\"OK\"},\"finish_reason\":\"stop\"}]}\n\ndata: [DONE]\n\n");
    const auto limited = [&](const std::string& value) {
        auto text = response(429, "{}"); text.insert(text.find("\r\n") + 2, "rEtRy-AfTeR: " + value + "\r\n"); return text;
    };
    const auto failOnce = [](AIClient& client, ChatOptions options, const std::string& kind) {
        try { client.chat(options); expect(false, "失败夹具不应成功"); }
        catch (const AIClientError& error) { expect(error.errorType == kind, "公共退避错误分类应保持"); }
    };
    ChatOptions options; options.messages = {{"user", "本机冷却夹具"}}; options.maxAttempts = 1;
    for (const auto* kind : {"network_error", "invalid_response"}) {
        struct LocalCallbackError : AIClientError {
            explicit LocalCallbackError(const std::string& kind) : AIClientError(kind, "回调原始异常", 599) {}
            int marker = 77;
        };
        MockServer server(std::vector<std::string>{stream, normal});
        auto retry = options; retry.maxAttempts = 3; retry.activity.taskId = std::string("callback-origin-") + kind;
        int chunks = 0; const auto calls = AIActivity::snapshot().at("todayCalls").get<int>();
        try {
            clientFor(server).chatStream(retry, [&](const auto&) -> bool { ++chunks; throw LocalCallbackError(kind); });
            expect(false, "回调异常应传播给调用方");
        } catch (const LocalCallbackError& error) {
            expect(error.marker == 77 && error.errorType == kind && error.httpStatus == 599 && std::string(error.what()) == "回调原始异常",
                "回调异常的动态类型和原字段必须完整保留");
        }
        expect(chunks == 1 && server.requestCount() == 1 && AIActivity::snapshot().at("todayCalls") == calls + 1,
            "可重试类型的回调异常也只能一次HTTP和一次交付");
        const auto begin = Clock::now();
        expect(clientFor(server).chat(options).content == "OK" && Clock::now() - begin < std::chrono::milliseconds(650),
            "回调异常不得污染同配置的服务商冷却");
        expect(server.requestCount() == 2 && AIActivity::snapshot().at("todayCalls") == calls + 2, "回调失败和后续成功仍各结算一次HTTP");
    }
    {
        MockServer server(std::vector<std::string>{limited("1"), normal, stream});
        auto client = clientFor(server); failOnce(client, options, "rate_limited");
        const auto begin = Clock::now();
        auto waiting = std::async(std::launch::async, [&] { return clientFor(server).chatStream(options, [](const auto&) { return true; }); });
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        expect(clientFor(server, "/v1", "other-model").chat(options).content == "OK" && Clock::now() - begin < std::chrono::milliseconds(650),
            "一个调用等待时公共互斥锁不能阻塞其它配置");
        expect(waiting.get().content == "OK", "流式应复用普通调用冷却");
        expect(Clock::now() - begin >= std::chrono::milliseconds(850), "最后一次429仍应使同配置新实例等待");
        expect(server.requestCount() == 3, "共享退避不能重复计算HTTP");
    }
    for (const auto& header : {std::string("invalid"), std::string("-3"), std::string("99999999999999999999999")}) {
        MockServer server(std::vector<std::string>{limited(header), normal}); auto client = clientFor(server);
        auto retry = options; retry.maxAttempts = 2; const auto begin = Clock::now();
        expect(client.chat(retry).content == "OK", "无效Retry-After仍按原策略重试");
        expect(Clock::now() - begin < std::chrono::seconds(3), "无效或溢出头不得产生巨大等待");
    }
    {
        const auto date = std::time(nullptr) + 2; std::tm utc{};
#ifdef _WIN32
        gmtime_s(&utc, &date);
#else
        gmtime_r(&date, &utc);
#endif
        std::ostringstream formatted; formatted << std::put_time(&utc, "%a, %d %b %Y %H:%M:%S GMT");
        MockServer server(std::vector<std::string>{limited(formatted.str()), normal}); auto client = clientFor(server);
        failOnce(client, options, "rate_limited"); const auto begin = Clock::now(); client.chat(options);
        expect(Clock::now() - begin >= std::chrono::milliseconds(850), "HTTP日期Retry-After应生效");
    }
    for (const auto* isolation : {"model", "credential", "url"}) {
        MockServer server(std::vector<std::string>{limited("2"), normal}); auto client = clientFor(server);
        failOnce(client, options, "rate_limited"); const auto begin = Clock::now();
        AIClient isolated({"http://127.0.0.1:" + std::to_string(server.port()) + (std::string(isolation) == "url" ? "/other" : "/v1"),
            std::string(isolation) == "credential" ? "other-fixture-secret" : "test-secret-key",
            std::string(isolation) == "model" ? "other-model" : "unused", 1000, 1});
        expect(isolated.chat(options).content == "OK" && Clock::now() - begin < std::chrono::milliseconds(650), "不同URL、模型、凭据的冷却必须隔离");
    }
    for (const auto code : {401, 402}) {
        MockServer server(std::vector<std::string>{response(code, "{}"), normal}); auto client = clientFor(server);
        auto retry = options; retry.maxAttempts = 3;
        failOnce(client, retry, code == 401 ? "auth_error" : "provider_rejected"); const auto begin = Clock::now();
        expect(client.chat(options).content == "OK" && Clock::now() - begin < std::chrono::milliseconds(650), "认证和402不重试且不污染冷却");
        expect(server.requestCount() == 2, "认证或402只能发一次故障请求");
    }
    {
        MockServer server(std::vector<std::string>{limited("1"), stream}); auto client = clientFor(server);
        failOnce(client, options, "rate_limited");
        auto cancel = options; const auto begin = Clock::now();
        cancel.cancelled = [&] { return Clock::now() - begin > std::chrono::milliseconds(70); };
        failOnce(client, cancel, "cancelled"); expect(server.requestCount() == 1, "等待中取消不得新增HTTP");
        auto paused = options; paused.activity.taskId = "cooldown-paused-fixture";
        auto outcome = std::async(std::launch::async, [&] {
            try { client.chatStream(paused, [](const auto&) { return true; }); return std::string("completed"); }
            catch (const AIClientError& error) { return error.errorType; }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); AIActivity::setPaused(true);
        expect(outcome.get() == "paused" && server.requestCount() == 1, "冷却暂停须退出且零新增HTTP");
        AIActivity::setPaused(false);
        expect(client.chatStream(paused, [](const auto&) { return true; }).content == "OK", "恢复后只请求一次");
    }
    {
        MockServer server(response(200, "data: {\"choices\":[{\"delta\":{\"content\":\"已交付\"}}]}\n\n"));
        auto retry = options; retry.maxAttempts = 3; int chunks = 0;
        try { clientFor(server).chatStream(retry, [&](const auto&) { ++chunks; return true; }); expect(false, "半流应失败"); }
        catch (const AIClientError& error) { expect(error.contentDelivered, "部分流错误必须携带已交付标记"); }
        expect(chunks == 1 && server.requestCount() == 1, "部分流断开不得重放相同回调");
    }
    {
        MockServer server(std::vector<std::string>{"HTTP/1.1 100 Continue\r\nRetry-After: 10\r\n\r\n" + limited("invalid"), normal});
        auto retry = options; retry.maxAttempts = 2; const auto begin = Clock::now();
        expect(clientFor(server).chat(retry).content == "OK" && Clock::now() - begin < std::chrono::seconds(3),
            "新状态块应清除前一个响应头中的Retry-After");
    }
    {
        MockServer server(std::vector<std::string>{response(400, "stream_options unsupported"), response(500, "{}"), stream});
        auto retry = options; retry.maxAttempts = 2;
        expect(clientFor(server).chatStream(retry, [](const auto&) { return true; }).content == "OK", "兼容重发后仍可重试服务商故障");
        const auto first = server.request().find("stream_options");
        expect(first != std::string::npos && server.request().find("stream_options", first + 1) == std::string::npos && server.requestCount() == 3,
            "兼容重发独立计HTTP且后续故障重试不能重新发送被拒选项");
    }
    {
        MockServer server(stream); int chunks = 0;
        auto result = std::async(std::launch::async, [&] {
            try { clientFor(server).chatStream(options, [&](const auto&) { ++chunks; AIActivity::setPaused(true); return true; }); return std::string("completed"); }
            catch (const AIClientError& error) { return error.errorType; }
        });
        expect(result.wait_for(std::chrono::seconds(2)) == std::future_status::ready, "独立流已交付内容后暂停应立即退出，不能内部等待恢复重放");
        AIActivity::setPaused(false);
        expect(result.get() == "paused" && chunks == 1 && server.requestCount() == 1, "部分流暂停必须保留一次交付且零自动重放");
    }
    {
        MockServer server(std::vector<std::string>{response(500, "{}"), response(500, "{}"), response(500, "{}"), normal});
        auto client = clientFor(server);
        failOnce(client, options, "provider_5xx");
        try { clientFor(server).chatStream(options, [](const auto&) { return true; }); expect(false, "流式故障夹具应失败"); }
        catch (const AIClientError& error) { expect(error.errorType == "provider_5xx", "流式应共享最终失败计数"); }
        failOnce(client, options, "provider_5xx");
        // 超过普通800ms后仍不能发出第四笔，证明新实例没有重置三次失败的保护。
        std::this_thread::sleep_for(std::chrono::milliseconds(900));
        const auto begin = Clock::now(); auto cancelled = options;
        cancelled.cancelled = [&] { return Clock::now() - begin > std::chrono::milliseconds(150); };
        auto next = clientFor(server); failOnce(next, cancelled, "cancelled");
        expect(server.requestCount() == 3, "跨普通流式连续三次故障应共享60秒冷却，第四次取消零HTTP");
        expect(clientFor(server, "/v1", "unaffected-model").chat(options).content == "OK", "持续故障保护不能影响其它模型");
    }
}

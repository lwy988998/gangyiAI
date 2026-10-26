#pragma once

static void expectPrivateTransportDiagnostic(const gangyi::AIClientError& error, const std::string& phase, int code) {
    const auto& value = error.transportDiagnostic;
    expect(value.is_object() && value.value("phase", "") == phase && value.value("curlCode", -1) == code,
        "传输错误必须保留固定错误码和可定位的失败阶段");
    const auto serialized = value.dump() + error.what();
    for (const auto* secret : {"endpoint-secret", "query-secret", "credential-secret", "Authorization", "127.0.0.1"})
        expect(serialized.find(secret) == std::string::npos, "传输诊断不得泄露端点、请求头或凭据");
    const std::set<std::string> allowed = {"curlCode", "curlReason", "phase", "connectionEstablished", "firstByteReceived",
        "tlsRequested", "tlsBackend", "tlsHandshakeCompleted", "tlsVerifyResult", "proxyConnectStatus"};
    for (const auto& item : value.items()) expect(allowed.count(item.key()) == 1, "诊断只允许脱敏字段白名单");
}

static void runTransportDiagnosticChecks() {
    using namespace gangyi;
    // 保留一个未listen的本机端口，稳定复现拒绝连接，不查询公网或更改系统代理。
    struct BoundSocket {
        Socket value = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        ~BoundSocket() { if (value != kInvalidSocket) closeSocket(value); }
    } reserved;
    sockaddr_in address{}; address.sin_family = AF_INET; address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (reserved.value == kInvalidSocket || bind(reserved.value, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
        throw std::runtime_error("无法保留传输诊断测试端口");
    SocketLength size = sizeof(address);
    if (getsockname(reserved.value, reinterpret_cast<sockaddr*>(&address), &size) != 0) throw std::runtime_error("无法取得传输诊断测试端口");
    const auto endpoint = "http://endpoint-secret@127.0.0.1:" + std::to_string(ntohs(address.sin_port)) + "/v1?query-secret";
    for (int entry = 0; entry < 3; ++entry) {
        AIClient client({endpoint, "credential-secret", "transport-fixture-" + std::to_string(entry), 5000, 1});
        ChatOptions options; options.messages = {{"user", "本机传输故障"}}; options.maxAttempts = 1;
        try {
            if (entry == 0) client.chat(options);
            else if (entry == 1) client.chatStream(options, [](const auto&) { return true; });
            else client.listModels(5000);
            expect(false, "拒绝连接不能伪造成功");
        } catch (const AIClientError& error) {
            expect(error.errorType == "network_error" && error.retryable && error.httpStatus == 0, "补充诊断不得改变原有网络错误及重试资格");
            expectPrivateTransportDiagnostic(error, "tcp_connect", CURLE_COULDNT_CONNECT);
            expect(!error.transportDiagnostic.value("connectionEstablished", true), "拒绝连接不能标成连接成功");
        }
    }
    {
        MockServer server(response(200, "not-tls"));
        AIClient client({"https://endpoint-secret@127.0.0.1:" + std::to_string(server.port()) + "/v1?query-secret",
            "credential-secret", "tls-fixture", 1000, 1});
        ChatOptions options; options.messages = {{"user", "本机TLS故障"}}; options.maxAttempts = 1;
        try { client.chatStream(options, [](const auto&) { return true; }); expect(false, "明文服务不能完成TLS握手"); }
        catch (const AIClientError& error) {
            expectPrivateTransportDiagnostic(error, "tls_handshake", CURLE_SSL_CONNECT_ERROR);
            expect(error.transportDiagnostic.value("tlsRequested", false) &&
                !error.transportDiagnostic.value("tlsHandshakeCompleted", true), "TLS失败必须标出尚未完成握手");
            const auto& verified = error.transportDiagnostic.at("tlsVerifyResult");
            expect(verified.is_null() || verified != 0, "握手未完成不能把默认零值冒充证书验证通过");
        }
        server.wait(); expect(server.requestCount() == 1, "诊断TLS失败不得增加连接尝试");
    }
    {
        MockServer server(std::string{}, 250);
        try { clientFor(server).listModels(50); expect(false, "等待响应夹具应超时"); }
        catch (const AIClientError& error) {
            expectPrivateTransportDiagnostic(error, "response_headers", CURLE_OPERATION_TIMEDOUT);
            expect(error.errorType == "timeout" && error.transportDiagnostic.value("connectionEstablished", false),
                "已连通后的超时应与连接失败区分");
        }
        server.wait(); expect(server.requestCount() == 1, "获取模型的诊断不得自动重试");
    }
    {
        MockServer server(response(401, "credential-secret"));
        try { clientFor(server).listModels(); expect(false, "认证夹具应拒绝"); }
        catch (const AIClientError& error) {
            expect(error.errorType == "auth_error" && error.transportDiagnostic.is_null() &&
                std::string(error.what()) == "AI provider returned HTTP 401", "HTTP错误不得误报TLS诊断或回显正文");
        }
        server.wait();
    }
}

static void runStreamIntegrityChecks() {
    using namespace gangyi;
    const std::string prefix = "data: {\"choices\":[{\"delta\":{\"content\":\"前段\"}}]}\n\n";
    const std::string done = "data: [DONE]\n\n";
    {
        const std::string decision = R"({"message":"说明分数单位","actions":[{"id":"one","tool":"append_section","args":{"lessonId":"fixture"}],"state":"waiting_student"})";
        const nlohmann::json event = {{"choices", {{{"delta", {{"content", decision}}}, {"finish_reason", "stop"}}}}};
        MockServer server(response(200, "data: " + event.dump() + "\n\n" + done));
        ChatOptions options; options.messages = {{"user", "合法传输中的坏模型决策"}};
        const auto result = clientFor(server).chatStream(options, [](const auto&) { return true; });
        server.wait();
        expect(result.content == decision && nlohmann::json::parse(result.content, nullptr, false).is_discarded(),
            "合法SSE必须逐字保留模型正文，不能擅自修补坏决策；主控另行校验动作");
        expect(server.requestCount() == 1, "传输层不得为修补模型正文增加调用");
    }
    for (const auto& body : {
        prefix + "data: {broken-json}\n\n" + done,
        prefix + "data: {\"choices\":\"broken-shape\"}\n\n" + done,
        prefix + "data: {\"choices\":[{\"delta\":{\"content\":4}}]}\n\n" + done,
        prefix + "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"content_filter\"}]}\n\n" + done,
        prefix + "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"content_filter\"}]}\n\n" +
            "data: {\"choices\":[{\"delta\":null,\"finish_reason\":\"stop\"}]}\n\n" + done,
        prefix + done + prefix,
        prefix + "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"stop\"}]}\n\n" + prefix + done,
        prefix + "data: {\"choices\":[{\"delta\":{\"content\":\"未闭合\"}"
    }) {
        MockServer server(response(200, body));
        ChatOptions options; options.messages = {{"user", "本机损坏流回归"}}; options.maxAttempts = 3;
        std::string delivered;
        try { clientFor(server).chatStream(options, [&](const auto& chunk) { delivered += chunk; return true; }); expect(false, "损坏流不能因DONE而成为成功"); }
        catch (const AIClientError& error) { expect(error.errorType == "invalid_response" && error.contentDelivered, "损坏帧应报告不完整并保留已交付标记"); }
        server.wait(); expect(server.requestCount() == 1 && delivered == "前段", "部分流失败不得自动重放或继续交付终止后的片段");
    }
    {
        const auto final = "data: {\"choices\":[{\"delta\":{\"content\":\"尾段\"},\"finish_reason\":\"stop\"}]}";
        MockServer server(response(200, prefix + final));
        ChatOptions options; options.messages = {{"user", "无换行尾帧"}};
        std::string delivered;
        const auto result = clientFor(server).chatStream(options, [&](const auto& chunk) { delivered += chunk; return true; });
        server.wait(); expect(result.content == "前段尾段" && delivered == result.content && server.requestCount() == 1,
            "合法无换行尾帧必须解析并交付，不得静默丢弃");
    }
    {
        const auto stopped = "data: {\"choices\":[{\"delta\":{\"content\":\"尾段\"},\"finish_reason\":\"stop\"}]}\n\n";
        const auto usage = "data: {\"choices\":[],\"usage\":{\"prompt_tokens\":4,\"completion_tokens\":6,\"total_tokens\":10}}\n\n";
        MockServer server(response(200, prefix + stopped + usage + done));
        ChatOptions options; options.messages = {{"user", "正常终态后补用量"}};
        const auto result = clientFor(server).chatStream(options, [](const auto&) { return true; });
        server.wait();
        expect(result.content == "前段尾段" && result.tokenUsage.at("total_tokens") == 10 && server.requestCount() == 1,
            "stop同帧正文和后续usage-only必须合法且完整保留，不新增调用");
    }
    {
        const auto limited = "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"length\"}]}\n\n";
        const auto stopped = "data: {\"choices\":[{\"delta\":null,\"finish_reason\":\"stop\"}]}\n\n";
        MockServer server(response(200, prefix + limited + stopped + done));
        ChatOptions options; options.messages = {{"user", "截断状态不能被覆盖"}}; options.maxAttempts = 3;
        try { clientFor(server).chatStream(options, [](const auto&) { return true; }); expect(false, "length后stop不能使片段成功"); }
        catch (const AIClientError& error) { expect(error.errorType == "output_limit", "服务商明确截断应保持输出限制分类"); }
        server.wait(); expect(server.requestCount() == 1, "明确截断仍由既有主控恢复，SDK不增加重放");
    }
}

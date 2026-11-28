// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 定义函数 expectPrivateTransportDiagnostic，执行该函数声明的职责。
static void expectPrivateTransportDiagnostic(const gangyi::AIClientError& error, const std::string& phase, int code) {
    // 【逐行批注】 计算右侧结果并写入 auto& value，更新当前运行状态。
    const auto& value = error.transportDiagnostic;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(value.is_object() && value.value("phase", "") == phase && valu…。
    expect(value.is_object() && value.value("phase", "") == phase && value.value("curlCode", -1) == code,
        // 【逐行批注】 执行当前声明或表达式："传输错误必须保留固定错误码和可定位的失败阶段");。
        "传输错误必须保留固定错误码和可定位的失败阶段");
    // 【逐行批注】 计算右侧结果并写入 auto serialized，更新当前运行状态。
    const auto serialized = value.dump() + error.what();
    // 【逐行批注】 按循环规则重复处理：for (const auto* secret : {"endpoint-secret", "query-secret", "credential-…。
    for (const auto* secret : {"endpoint-secret", "query-secret", "credential-secret", "Authorization", "127.0.0.1"})
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(serialized.find(secret) == std::string::npos, "传输诊断不得泄露端点、请求头或…。
        expect(serialized.find(secret) == std::string::npos, "传输诊断不得泄露端点、请求头或凭据");
    // 【逐行批注】 执行当前声明或表达式：const std::set<std::string> allowed = {"curlCode", "curlReason", "phase", "connectio…。
    const std::set<std::string> allowed = {"curlCode", "curlReason", "phase", "connectionEstablished", "firstByteReceived",
        // 【逐行批注】 执行当前声明或表达式："tlsRequested", "tlsBackend", "tlsHandshakeCompleted", "tlsVerifyResult", "proxyConn…。
        "tlsRequested", "tlsBackend", "tlsHandshakeCompleted", "tlsVerifyResult", "proxyConnectStatus"};
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : value.items()) expect(allowed.count(item.key()) ==…。
    for (const auto& item : value.items()) expect(allowed.count(item.key()) == 1, "诊断只允许脱敏字段白名单");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 runTransportDiagnosticChecks，执行该函数声明的职责。
static void runTransportDiagnosticChecks() {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 保留一个未listen的本机端口，稳定复现拒绝连接，不查询公网或更改系统代理。
    // 【逐行批注】 声明数据结构或类型 BoundSocket 及其成员接口。
    struct BoundSocket {
        // 【逐行批注】 计算右侧结果并写入 Socket value，更新当前运行状态。
        Socket value = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // 【逐行批注】 调用 BoundSocket、closeSocket 完成本行所需操作：~BoundSocket() { if (value != kInvalidSocket) closeSocket(value); }。
        ~BoundSocket() { if (value != kInvalidSocket) closeSocket(value); }
    // 【逐行批注】 执行当前声明或表达式：} reserved;。
    } reserved;
    // 【逐行批注】 计算右侧结果并写入 sockaddr_in address{}; address.sin_family，更新当前运行状态。
    sockaddr_in address{}; address.sin_family = AF_INET; address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    // 【逐行批注】 判断条件 (reserved.value == kInvalidSocket || bind(reserved.value, reinte…，成立时执行对应分支。
    if (reserved.value == kInvalidSocket || bind(reserved.value, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("无法保留传输诊断测试端口");。
        throw std::runtime_error("无法保留传输诊断测试端口");
    // 【逐行批注】 计算右侧结果并写入 SocketLength size，更新当前运行状态。
    SocketLength size = sizeof(address);
    // 【逐行批注】 判断条件 (getsockname(reserved.value, reinterpret_cast<sockaddr*>(&addres…，成立时执行对应分支。
    if (getsockname(reserved.value, reinterpret_cast<sockaddr*>(&address), &size) != 0) throw std::runtime_error("无法取得传输诊断测试端口");
    // 【逐行批注】 计算右侧结果并写入 auto endpoint，更新当前运行状态。
    const auto endpoint = "http://endpoint-secret@127.0.0.1:" + std::to_string(ntohs(address.sin_port)) + "/v1?query-secret";
    // 【逐行批注】 按循环规则重复处理：for (int entry = 0; entry < 3; ++entry) {。
    for (int entry = 0; entry < 3; ++entry) {
        // 【逐行批注】 声明对象 client，并通过构造函数初始化其状态。
        AIClient client({endpoint, "credential-secret", "transport-fixture-" + std::to_string(entry), 5000, 1});
        // 【逐行批注】 计算右侧结果并写入 ChatOptions options; options.messages，更新当前运行状态。
        ChatOptions options; options.messages = {{"user", "本机传输故障"}}; options.maxAttempts = 1;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 判断条件 (entry == 0) client.chat(options);，成立时执行对应分支。
            if (entry == 0) client.chat(options);
            // 【逐行批注】 在前一条件不成立时继续判断：(entry == 1) client.chatStream(options, [](const auto&) { return…。
            else if (entry == 1) client.chatStream(options, [](const auto&) { return true; });
            // 【逐行批注】 执行前一条件不成立时的分支。
            else client.listModels(5000);
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(false, "拒绝连接不能伪造成功");。
            expect(false, "拒绝连接不能伪造成功");
        // 【逐行批注】 执行当前声明或表达式：} catch (const AIClientError& error) {。
        } catch (const AIClientError& error) {
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(error.errorType == "network_error" && error.retryable && error…。
            expect(error.errorType == "network_error" && error.retryable && error.httpStatus == 0, "补充诊断不得改变原有网络错误及重试资格");
            // 【逐行批注】 调用 expectPrivateTransportDiagnostic 完成本行所需操作：expectPrivateTransportDiagnostic(error, "tcp_connect", CURLE_COULDNT_…。
            expectPrivateTransportDiagnostic(error, "tcp_connect", CURLE_COULDNT_CONNECT);
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(!error.transportDiagnostic.value("connectionEstablished", true…。
            expect(!error.transportDiagnostic.value("connectionEstablished", true), "拒绝连接不能标成连接成功");
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, "not-tls"));
        // 【逐行批注】 声明对象 client，并通过构造函数初始化其状态。
        AIClient client({"https://endpoint-secret@127.0.0.1:" + std::to_string(server.port()) + "/v1?query-secret",
            // 【逐行批注】 执行当前声明或表达式："credential-secret", "tls-fixture", 1000, 1});。
            "credential-secret", "tls-fixture", 1000, 1});
        // 【逐行批注】 计算右侧结果并写入 ChatOptions options; options.messages，更新当前运行状态。
        ChatOptions options; options.messages = {{"user", "本机TLS故障"}}; options.maxAttempts = 1;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { client.chatStream(options, [](const auto&) { return true; }); expect(false, "明文服务不能完成TLS握手"); }
        // 【逐行批注】 捕获并处理异常：catch (const AIClientError& error) {。
        catch (const AIClientError& error) {
            // 【逐行批注】 调用 expectPrivateTransportDiagnostic 完成本行所需操作：expectPrivateTransportDiagnostic(error, "tls_handshake", CURLE_SSL_CO…。
            expectPrivateTransportDiagnostic(error, "tls_handshake", CURLE_SSL_CONNECT_ERROR);
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(error.transportDiagnostic.value("tlsRequested", false) &&。
            expect(error.transportDiagnostic.value("tlsRequested", false) &&
                // 【逐行批注】 执行当前声明或表达式：!error.transportDiagnostic.value("tlsHandshakeCompleted", true), "TLS失败必须标出尚未完成握手");。
                !error.transportDiagnostic.value("tlsHandshakeCompleted", true), "TLS失败必须标出尚未完成握手");
            // 【逐行批注】 计算右侧结果并写入 auto& verified，更新当前运行状态。
            const auto& verified = error.transportDiagnostic.at("tlsVerifyResult");
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(verified.is_null() || verified != 0, "握手未完成不能把默认零值冒充证书验证通过");。
            expect(verified.is_null() || verified != 0, "握手未完成不能把默认零值冒充证书验证通过");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 expect 完成本行所需操作：server.wait(); expect(server.requestCount() == 1, "诊断TLS失败不得增加连接尝试");。
        server.wait(); expect(server.requestCount() == 1, "诊断TLS失败不得增加连接尝试");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::string{}, 250);
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { clientFor(server).listModels(50); expect(false, "等待响应夹具应超时"); }
        // 【逐行批注】 捕获并处理异常：catch (const AIClientError& error) {。
        catch (const AIClientError& error) {
            // 【逐行批注】 调用 expectPrivateTransportDiagnostic 完成本行所需操作：expectPrivateTransportDiagnostic(error, "response_headers", CURLE_OPE…。
            expectPrivateTransportDiagnostic(error, "response_headers", CURLE_OPERATION_TIMEDOUT);
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(error.errorType == "timeout" && error.transportDiagnostic.valu…。
            expect(error.errorType == "timeout" && error.transportDiagnostic.value("connectionEstablished", false),
                // 【逐行批注】 执行当前声明或表达式："已连通后的超时应与连接失败区分");。
                "已连通后的超时应与连接失败区分");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 expect 完成本行所需操作：server.wait(); expect(server.requestCount() == 1, "获取模型的诊断不得自动重试");。
        server.wait(); expect(server.requestCount() == 1, "获取模型的诊断不得自动重试");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(401, "credential-secret"));
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { clientFor(server).listModels(); expect(false, "认证夹具应拒绝"); }
        // 【逐行批注】 捕获并处理异常：catch (const AIClientError& error) {。
        catch (const AIClientError& error) {
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(error.errorType == "auth_error" && error.transportDiagnostic.i…。
            expect(error.errorType == "auth_error" && error.transportDiagnostic.is_null() &&
                // 【逐行批注】 调用 string 完成本行所需操作：std::string(error.what()) == "AI provider returned HTTP 401", "HTTP错误…。
                std::string(error.what()) == "AI provider returned HTTP 401", "HTTP错误不得误报TLS诊断或回显正文");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 runStreamIntegrityChecks，执行该函数声明的职责。
static void runStreamIntegrityChecks() {
    // 【逐行批注】 引入命名空间中的名称：using namespace gangyi;。
    using namespace gangyi;
    // 【逐行批注】 计算右侧结果并写入 std::string prefix，更新当前运行状态。
    const std::string prefix = "data: {\"choices\":[{\"delta\":{\"content\":\"前段\"}}]}\n\n";
    // 【逐行批注】 计算右侧结果并写入 std::string done，更新当前运行状态。
    const std::string done = "data: [DONE]\n\n";
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 std::string decision，更新当前运行状态。
        const std::string decision = R"({"message":"说明分数单位","actions":[{"id":"one","tool":"append_section","args":{"lessonId":"fixture"}],"state":"waiting_student"})";
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json event，更新当前运行状态。
        const nlohmann::json event = {{"choices", {{{"delta", {{"content", decision}}}, {"finish_reason", "stop"}}}}};
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, "data: " + event.dump() + "\n\n" + done));
        // 【逐行批注】 计算右侧结果并写入 ChatOptions options; options.messages，更新当前运行状态。
        ChatOptions options; options.messages = {{"user", "合法传输中的坏模型决策"}};
        // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
        const auto result = clientFor(server).chatStream(options, [](const auto&) { return true; });
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect、parse 完成本行所需操作：expect(result.content == decision && nlohmann::json::parse(result.con…。
        expect(result.content == decision && nlohmann::json::parse(result.content, nullptr, false).is_discarded(),
            // 【逐行批注】 执行当前声明或表达式："合法SSE必须逐字保留模型正文，不能擅自修补坏决策；主控另行校验动作");。
            "合法SSE必须逐字保留模型正文，不能擅自修补坏决策；主控另行校验动作");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requestCount() == 1, "传输层不得为修补模型正文增加调用");。
        expect(server.requestCount() == 1, "传输层不得为修补模型正文增加调用");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& body : {。
    for (const auto& body : {
        // 【逐行批注】 执行当前声明或表达式：prefix + "data: {broken-json}\n\n" + done,。
        prefix + "data: {broken-json}\n\n" + done,
        // 【逐行批注】 执行当前声明或表达式：prefix + "data: {\"choices\":\"broken-shape\"}\n\n" + done,。
        prefix + "data: {\"choices\":\"broken-shape\"}\n\n" + done,
        // 【逐行批注】 执行当前声明或表达式：prefix + "data: {\"choices\":[{\"delta\":{\"content\":4}}]}\n\n" + done,。
        prefix + "data: {\"choices\":[{\"delta\":{\"content\":4}}]}\n\n" + done,
        // 【逐行批注】 执行当前声明或表达式：prefix + "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"content_filter\"}]}\…。
        prefix + "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"content_filter\"}]}\n\n" + done,
        // 【逐行批注】 执行当前声明或表达式：prefix + "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"content_filter\"}]}\…。
        prefix + "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"content_filter\"}]}\n\n" +
            // 【逐行批注】 执行当前声明或表达式："data: {\"choices\":[{\"delta\":null,\"finish_reason\":\"stop\"}]}\n\n" + done,。
            "data: {\"choices\":[{\"delta\":null,\"finish_reason\":\"stop\"}]}\n\n" + done,
        // 【逐行批注】 执行当前声明或表达式：prefix + done + prefix,。
        prefix + done + prefix,
        // 【逐行批注】 执行当前声明或表达式：prefix + "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"stop\"}]}\n\n" + pre…。
        prefix + "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"stop\"}]}\n\n" + prefix + done,
        // 【逐行批注】 执行当前声明或表达式：prefix + "data: {\"choices\":[{\"delta\":{\"content\":\"未闭合\"}"。
        prefix + "data: {\"choices\":[{\"delta\":{\"content\":\"未闭合\"}"
    // 【逐行批注】 执行当前声明或表达式：}) {。
    }) {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, body));
        // 【逐行批注】 计算右侧结果并写入 ChatOptions options; options.messages，更新当前运行状态。
        ChatOptions options; options.messages = {{"user", "本机损坏流回归"}}; options.maxAttempts = 3;
        // 【逐行批注】 执行当前声明或表达式：std::string delivered;。
        std::string delivered;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { clientFor(server).chatStream(options, [&](const auto& chunk) { delivered += chunk; return true; }); expect(false, "损坏流不能因DONE而成为成功"); }
        // 【逐行批注】 捕获并处理异常：catch (const AIClientError& error) { expect(error.errorType == "inval…。
        catch (const AIClientError& error) { expect(error.errorType == "invalid_response" && error.contentDelivered, "损坏帧应报告不完整并保留已交付标记"); }
        // 【逐行批注】 调用 expect 完成本行所需操作：server.wait(); expect(server.requestCount() == 1 && delivered == "前段"…。
        server.wait(); expect(server.requestCount() == 1 && delivered == "前段", "部分流失败不得自动重放或继续交付终止后的片段");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 auto final，更新当前运行状态。
        const auto final = "data: {\"choices\":[{\"delta\":{\"content\":\"尾段\"},\"finish_reason\":\"stop\"}]}";
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, prefix + final));
        // 【逐行批注】 计算右侧结果并写入 ChatOptions options; options.messages，更新当前运行状态。
        ChatOptions options; options.messages = {{"user", "无换行尾帧"}};
        // 【逐行批注】 执行当前声明或表达式：std::string delivered;。
        std::string delivered;
        // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
        const auto result = clientFor(server).chatStream(options, [&](const auto& chunk) { delivered += chunk; return true; });
        // 【逐行批注】 调用 expect 完成本行所需操作：server.wait(); expect(result.content == "前段尾段" && delivered == result…。
        server.wait(); expect(result.content == "前段尾段" && delivered == result.content && server.requestCount() == 1,
            // 【逐行批注】 执行当前声明或表达式："合法无换行尾帧必须解析并交付，不得静默丢弃");。
            "合法无换行尾帧必须解析并交付，不得静默丢弃");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 auto stopped，更新当前运行状态。
        const auto stopped = "data: {\"choices\":[{\"delta\":{\"content\":\"尾段\"},\"finish_reason\":\"stop\"}]}\n\n";
        // 【逐行批注】 计算右侧结果并写入 auto usage，更新当前运行状态。
        const auto usage = "data: {\"choices\":[],\"usage\":{\"prompt_tokens\":4,\"completion_tokens\":6,\"total_tokens\":10}}\n\n";
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, prefix + stopped + usage + done));
        // 【逐行批注】 计算右侧结果并写入 ChatOptions options; options.messages，更新当前运行状态。
        ChatOptions options; options.messages = {{"user", "正常终态后补用量"}};
        // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
        const auto result = clientFor(server).chatStream(options, [](const auto&) { return true; });
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result.content == "前段尾段" && result.tokenUsage.at("total_tokens…。
        expect(result.content == "前段尾段" && result.tokenUsage.at("total_tokens") == 10 && server.requestCount() == 1,
            // 【逐行批注】 执行当前声明或表达式："stop同帧正文和后续usage-only必须合法且完整保留，不新增调用");。
            "stop同帧正文和后续usage-only必须合法且完整保留，不新增调用");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 auto limited，更新当前运行状态。
        const auto limited = "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"length\"}]}\n\n";
        // 【逐行批注】 计算右侧结果并写入 auto stopped，更新当前运行状态。
        const auto stopped = "data: {\"choices\":[{\"delta\":null,\"finish_reason\":\"stop\"}]}\n\n";
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, prefix + limited + stopped + done));
        // 【逐行批注】 计算右侧结果并写入 ChatOptions options; options.messages，更新当前运行状态。
        ChatOptions options; options.messages = {{"user", "截断状态不能被覆盖"}}; options.maxAttempts = 3;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { clientFor(server).chatStream(options, [](const auto&) { return true; }); expect(false, "length后stop不能使片段成功"); }
        // 【逐行批注】 捕获并处理异常：catch (const AIClientError& error) { expect(error.errorType == "outpu…。
        catch (const AIClientError& error) { expect(error.errorType == "output_limit", "服务商明确截断应保持输出限制分类"); }
        // 【逐行批注】 调用 expect 完成本行所需操作：server.wait(); expect(server.requestCount() == 1, "明确截断仍由既有主控恢复，SDK不增…。
        server.wait(); expect(server.requestCount() == 1, "明确截断仍由既有主控恢复，SDK不增加重放");
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

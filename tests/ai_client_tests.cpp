#include "ai_client.hpp"
#include "ask_generator.hpp"
#include "json_fix.hpp"
#include "plan_generator.hpp"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
using Socket = SOCKET;
using SocketLength = int;
constexpr Socket kInvalidSocket = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using Socket = int;
using SocketLength = socklen_t;
constexpr Socket kInvalidSocket = -1;
#endif

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "失败: " << message << '\n';
        ++failures;
    }
}

void closeSocket(Socket socket) {
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}

class MockServer {
public:
    MockServer(std::string response, int delayMs = 0)
        : MockServer(std::vector<std::string>{std::move(response)}, delayMs) {}

    MockServer(std::vector<std::string> responses, int delayMs = 0)
        : responses_(std::move(responses)), delayMs_(delayMs) {
        server_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_ == kInvalidSocket) throw std::runtime_error("无法创建模拟服务器套接字");
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        if (bind(server_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 || listen(server_, 1) != 0) {
            closeSocket(server_);
            throw std::runtime_error("无法启动模拟服务器");
        }
        SocketLength length = sizeof(address);
        if (getsockname(server_, reinterpret_cast<sockaddr*>(&address), &length) != 0) {
            closeSocket(server_);
            throw std::runtime_error("无法读取模拟服务器端口");
        }
        port_ = ntohs(address.sin_port);
        worker_ = std::thread([this] { serve(); });
    }

    ~MockServer() {
        wait();
        closeSocket(server_);
    }

    void wait() {
        if (worker_.joinable()) worker_.join();
    }
    int port() const { return port_; }
    int requestCount() const { return requestCount_; }
    const std::string& request() const { return request_; }

private:
    void serve() {
        for (const auto& responseText : responses_) {
            sockaddr_in clientAddress{};
            SocketLength length = sizeof(clientAddress);
            Socket client = accept(server_, reinterpret_cast<sockaddr*>(&clientAddress), &length);
            if (client == kInvalidSocket) return;
            ++requestCount_;
            std::string currentRequest;
            char buffer[2048];
            while (currentRequest.find("\r\n\r\n") == std::string::npos) {
                const int received = recv(client, buffer, sizeof(buffer), 0);
                if (received <= 0) break;
                currentRequest.append(buffer, static_cast<size_t>(received));
            }
            const auto headerEnd = currentRequest.find("\r\n\r\n");
            const auto lengthStart = currentRequest.find("Content-Length:");
            if (headerEnd != std::string::npos && lengthStart != std::string::npos) {
                const auto valueStart = currentRequest.find_first_not_of(" ", lengthStart + 15);
                const auto valueEnd = currentRequest.find("\r\n", valueStart);
                const size_t contentLength = static_cast<size_t>(
                    std::stoul(currentRequest.substr(valueStart, valueEnd - valueStart)));
                while (currentRequest.size() < headerEnd + 4 + contentLength) {
                    const int received = recv(client, buffer, sizeof(buffer), 0);
                    if (received <= 0) break;
                    currentRequest.append(buffer, static_cast<size_t>(received));
                }
            }
            request_ += currentRequest;
            if (delayMs_ > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delayMs_));
            if (!responseText.empty()) send(client, responseText.data(), static_cast<int>(responseText.size()), 0);
            closeSocket(client);
        }
    }

    Socket server_ = kInvalidSocket;
    int port_ = 0;
    std::vector<std::string> responses_;
    int delayMs_ = 0;
    std::thread worker_;
    std::atomic<int> requestCount_{0};
    std::string request_;
};

std::string response(int status, const std::string& body) {
    return "HTTP/1.1 " + std::to_string(status) + " Test\r\nContent-Type: application/json\r\nContent-Length: " +
        std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
}

gangyi::AIClient clientFor(const MockServer& server, const std::string& path = "/v1",
                           const std::string& model = "unused") {
    return gangyi::AIClient({"http://127.0.0.1:" + std::to_string(server.port()) + path,
                            "test-secret-key", model, 1000, 1});
}

void expectError(const char* expectedType, MockServer& server, int timeoutMs = 1000) {
    try {
        clientFor(server).listModels(timeoutMs);
        expect(false, "请求应当失败");
    } catch (const gangyi::AIClientError& error) {
        expect(error.errorType == expectedType, "错误分类不正确");
    }
    server.wait();
    expect(server.requestCount() == 1, "获取模型不得自动重试");
}

}  // namespace

int main() {
#ifdef _WIN32
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return 2;
#endif
    {
        const auto parsed = gangyi::parseAIJson(
            R"(分析中可能会提到示例 {x: 1}，最终结果如下：```json
{"goal":"掌握函数单调性","summary":"完成定义与题型练习"}
```)"
        );
        expect(parsed.value("goal", "") == "掌握函数单调性",
               "JSON 解析器应提取推理文本末尾的完整对象");
    }
    {
        MockServer server(response(200, R"({"choices":[{"message":{"content":" 好的 "}}]})"));
        auto client = clientFor(server, "/v1", "configured-ask-model");
        gangyi::AskGenerator generator(client);
        const auto answer = generator.generate("请解释函数单调性");
        server.wait();
        expect(answer.content == "好的", "问答应返回去除首尾空白的内容");
        expect(answer.model == "configured-ask-model", "响应缺少模型字段时应回退到配置模型");
        expect(server.request().find("POST /v1/chat/completions HTTP/") != std::string::npos,
               "问答应请求 Chat Completions 端点");
        expect(server.request().find("\"model\":\"configured-ask-model\"") != std::string::npos,
               "问答不得覆盖用户配置的模型");
        expect(server.request().find("deepseek-v4-flash") == std::string::npos,
               "问答请求不得包含硬编码模型");
    }
    {
        MockServer server(response(200, R"({"data":[{"id":"z-model"},{"id":"a-model"},{"id":"a-model"}]})"));
        const auto models = clientFor(server, "/v1/chat/completions/").listModels();
        server.wait();
        expect(models == std::vector<std::string>({"a-model", "z-model"}), "模型应排序并去重");
        expect(server.request().find("GET /v1/models HTTP/") != std::string::npos,
               "尾部斜杠的 chat 端点应正确归一化");
        expect(server.request().find("Authorization: Bearer test-secret-key") != std::string::npos,
               "请求应使用当前未保存的 Key");
        expect(server.requestCount() == 1, "成功请求只发送一次");
    }
    {
        MockServer server(response(401, R"({"error":"unauthorized"})"));
        expectError("auth_error", server);
    }
    {
        MockServer server(response(404, R"({"error":"missing"})"));
        expectError("models_unsupported", server);
    }
    {
        MockServer server(response(429, R"({"error":"limited"})"));
        expectError("rate_limited", server);
    }
    {
        MockServer server(response(500, R"({"error":"temporary"})"));
        expectError("provider_5xx", server);
    }
    {
        MockServer server(response(200, "not-json"));
        expectError("invalid_response", server);
    }
    {
        MockServer server(response(200, R"({"data":[]})"));
        expectError("invalid_response", server);
    }
    {
        MockServer server(std::string{}, 250);
        expectError("timeout", server, 50);
    }
    {
        MockServer server(std::vector<std::string>{
            response(500, R"({"error":"temporary"})"),
            response(200, R"({"model":"provider-model","choices":[{"message":{"content":"恢复成功"}}]})")});
        gangyi::AIClient client({"http://127.0.0.1:" + std::to_string(server.port()) + "/v1",
                                 "test-secret-key", "configured-model", 1000, 2});
        gangyi::ChatOptions options;
        options.messages = {{"user", "测试可重试错误"}};
        options.maxAttempts = 2;
        const auto result = client.chat(options);
        server.wait();
        expect(result.content == "恢复成功", "服务端临时错误后应成功返回");
        expect(result.model == "provider-model", "服务商返回模型时应按实际模型响应");
        expect(server.requestCount() == 2, "聊天请求应仅按配置重试一次");
    }
    {
        MockServer server(std::vector<std::string>{
            response(200, "not-json"),
            response(200, R"({"model":"provider-model","choices":[{"message":{"content":"恢复成功"}}]})")});
        auto client = clientFor(server, "/v1", "configured-model");
        gangyi::ChatOptions options;
        options.messages = {{"user", "测试无效响应重试"}};
        options.maxAttempts = 2;
        const auto result = client.chat(options);
        server.wait();
        expect(result.content == "恢复成功", "服务商临时返回无效内容后应成功恢复");
        expect(server.requestCount() == 2, "无效聊天响应应仅按配置重试一次");
    }
    {
        MockServer server(response(200, R"({"choices":[{"message":{"content":"{\"inferredDomain\":\"数学\",\"learnerGoal\":\"掌握函数单调性\",\"courseTitle\":\"函数单调性课程\",\"courseSummary\":\"分阶段掌握定义和题型\",\"durationWeeks\":2,\"phases\":[{\"name\":\"概念\",\"durationWeeks\":1,\"objective\":\"理解单调性定义\",\"topics\":[\"增函数\",\"减函数\",\"定义域\"],\"tasks\":[\"完成定义练习\",\"整理错题\"],\"checkpoint\":\"能用定义判断\",\"output\":\"定义题练习报告\",\"commonMistakes\":[\"忽略定义域\"]},{\"name\":\"图像\",\"durationWeeks\":1,\"objective\":\"结合图像判断\",\"topics\":[\"图像趋势\",\"区间\",\"端点\"],\"tasks\":[\"完成图像练习\",\"标注区间\"],\"checkpoint\":\"能读图判断\",\"output\":\"图像题练习报告\",\"commonMistakes\":[\"看错区间\"]},{\"name\":\"综合\",\"durationWeeks\":1,\"objective\":\"解决综合题\",\"topics\":[\"参数题\",\"证明题\",\"应用题\"],\"tasks\":[\"完成综合练习\",\"复盘错题\"],\"checkpoint\":\"能独立解题\",\"output\":\"综合题练习报告\",\"commonMistakes\":[\"步骤不完整\"]}]}"}}],"model":"configured-plan-model"})"));
        auto client = clientFor(server, "/v1", "configured-plan-model");
        gangyi::PlanGenerator generator(client);
        const auto plan = generator.generate("掌握函数单调性", "lite");
        server.wait();
        expect(plan.title == "函数单调性课程", "课程标题应兼容 courseTitle 字段");
        expect(plan.goal == "掌握函数单调性", "学习目标应兼容 learnerGoal 字段");
        expect(plan.phases.size() == 3, "快速规划应保留三个阶段");
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return failures == 0 ? 0 : 1;
}

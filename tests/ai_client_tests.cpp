#include "ai_client.hpp"

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
        : response_(std::move(response)), delayMs_(delayMs) {
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
        sockaddr_in clientAddress{};
        SocketLength length = sizeof(clientAddress);
        Socket client = accept(server_, reinterpret_cast<sockaddr*>(&clientAddress), &length);
        if (client == kInvalidSocket) return;
        ++requestCount_;
        char buffer[2048];
        while (request_.find("\r\n\r\n") == std::string::npos) {
            const int received = recv(client, buffer, sizeof(buffer), 0);
            if (received <= 0) break;
            request_.append(buffer, static_cast<size_t>(received));
        }
        if (delayMs_ > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delayMs_));
        if (!response_.empty()) send(client, response_.data(), static_cast<int>(response_.size()), 0);
        closeSocket(client);
    }

    Socket server_ = kInvalidSocket;
    int port_ = 0;
    std::string response_;
    int delayMs_ = 0;
    std::thread worker_;
    std::atomic<int> requestCount_{0};
    std::string request_;
};

std::string response(int status, const std::string& body) {
    return "HTTP/1.1 " + std::to_string(status) + " Test\r\nContent-Type: application/json\r\nContent-Length: " +
        std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
}

gangyi::AIClient clientFor(const MockServer& server, const std::string& path = "/v1") {
    return gangyi::AIClient({"http://127.0.0.1:" + std::to_string(server.port()) + path,
                            "test-secret-key", "unused", 1000, 1});
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
        MockServer server(response(200, "not-json"));
        expectError("invalid_response", server);
    }
    {
        MockServer server({}, 250);
        expectError("timeout", server, 50);
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return failures == 0 ? 0 : 1;
}

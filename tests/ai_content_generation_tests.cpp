#include "learning_generator.hpp"
#include "phase_generator.hpp"

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
#include <iostream>
#include <string>
#include <thread>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "失败: " << message << '\n'; ++failures; }
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
    MockServer(std::string body, int expected) : expected_(expected) {
        response_ = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " +
            std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
        server_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        if (server_ == kInvalidSocket || bind(server_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
            listen(server_, expected) != 0) throw std::runtime_error("无法启动模拟 AI 服务");
        SocketLength length = sizeof(address);
        getsockname(server_, reinterpret_cast<sockaddr*>(&address), &length);
        port_ = ntohs(address.sin_port);
        worker_ = std::thread([this] { serve(); });
    }

    ~MockServer() { if (worker_.joinable()) worker_.join(); closeSocket(server_); }
    int port() const { return port_; }
    int count() const { return count_; }
    const std::string& requests() const { return requests_; }
    void wait() { if (worker_.joinable()) worker_.join(); }

private:
    void serve() {
        for (int index = 0; index < expected_; ++index) {
            sockaddr_in clientAddress{};
            SocketLength length = sizeof(clientAddress);
            Socket client = accept(server_, reinterpret_cast<sockaddr*>(&clientAddress), &length);
            if (client == kInvalidSocket) return;
            std::string request;
            char buffer[4096];
            while (request.find("\r\n\r\n") == std::string::npos) {
                const int received = recv(client, buffer, sizeof(buffer), 0);
                if (received <= 0) break;
                request.append(buffer, static_cast<size_t>(received));
            }
            requests_ += request;
            ++count_;
            send(client, response_.data(), static_cast<int>(response_.size()), 0);
            closeSocket(client);
        }
    }

    Socket server_ = kInvalidSocket;
    int port_ = 0;
    int expected_ = 0;
    std::string response_;
    std::thread worker_;
    std::atomic<int> count_{0};
    std::string requests_;
};

std::string providerResponse(const nlohmann::json& content) {
    return nlohmann::json{{"model", "configured-model"},
        {"choices", nlohmann::json::array({{{"message", {{"content", content.dump()}}}}})}}.dump();
}

gangyi::AIClient clientFor(const MockServer& server) {
    return gangyi::AIClient({"http://127.0.0.1:" + std::to_string(server.port()) + "/v1",
        "test-key", "configured-model", 2000, 1});
}

}  // namespace

int main() {
#ifdef _WIN32
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return 2;
#endif
    const nlohmann::json plan = {{"title", "氧化还原反应课程"}, {"generation", {{"source", "ai"}}}};
    const std::string goal = "高中化学氧化还原反应";
    const std::string phase = "氧化还原基础";
    const std::string topic = "氧化数变化与电子转移";
    {
        const nlohmann::json content = {{"title", topic + "入门"},
            {"summary", "围绕" + goal + "，在" + phase + "阶段通过" + topic + "判断氧化还原反应"}, {"inferredDomain", "高中化学"},
            {"keyConcepts", {topic, "氧化剂", "还原剂"}}};
        MockServer server(providerResponse(content), 1);
        auto client = clientFor(server);
        gangyi::LearningGenerator generator(client);
        const auto result = generator.generateBlock(goal, plan, phase,
            topic, 1, "deep", "overview", nlohmann::json::object(), {});
        server.wait();
        expect(result["_generation"].value("source", "") == "ai", "成功板块必须记录 AI 来源");
        expect(result["_generation"].value("model", "") == "configured-model", "必须记录实际返回模型");
        expect(server.requests().find("configured-model") != std::string::npos, "必须使用配置的模型");
        expect(server.requests().find("deepseek-v4-flash") == std::string::npos, "不得硬模型");
        expect(server.requests().find("coursePlan") != std::string::npos, "请求必须携带已保存课程主线");
        expect(server.requests().find("previousBlocks") != std::string::npos, "请求必须携带已生成前置板块");
        expect(server.requests().find("原样出现输入中的 goal、phase、topic") != std::string::npos,
            "板块提示词必须要求原样包含目标、阶段和主题");
        expect(server.requests().find("\"max_tokens\":4200") != std::string::npos,
            "深度板块必须预留完整 JSON 输出空间");
    }
    {
        MockServer server(providerResponse({{"title", topic}}), 3);
        auto client = clientFor(server);
        gangyi::LearningGenerator generator(client);
        try {
            generator.generateBlock(goal, plan, phase,
                topic, 1, "deep", "overview", nlohmann::json::object(), {});
            expect(false, "不完整 AI 输出不得成为课程内容");
        } catch (const gangyi::AIClientError& error) {
            expect(error.errorType == "quality_rejected", "三次质量失败应返回 quality_rejected");
        }
        server.wait();
        expect(server.count() == 3, "板块生成失败必须自动尝试三次");
    }
    {
        MockServer server(providerResponse({{"objective", "氧化还原基础"}}), 3);
        auto client = clientFor(server);
        gangyi::PhaseGenerator generator(client);
        try {
            generator.generate(goal, "deep", 1, phase,
                {topic, "氧化剂与还原剂", "氧化还原方程式配平"}, {});
            expect(false, "不完整阶段输出不得回退模板");
        } catch (const gangyi::AIClientError& error) {
            expect(error.errorType == "quality_rejected", "阶段三次质量失败应返回 quality_rejected");
        }
        server.wait();
        expect(server.count() == 3, "阶段生成失败必须自动尝试三次");
        expect(server.requests().find("禁止省略任何字段") != std::string::npos,
            "阶段提示词必须禁止截断或省略字段");
        expect(server.requests().find("\"max_tokens\":6000") != std::string::npos,
            "深度阶段必须预留完整 JSON 输出空间");
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return failures == 0 ? 0 : 1;
}

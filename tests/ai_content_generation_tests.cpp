#include "learning_generator.hpp"
#include "phase_generator.hpp"
#include "text_utils.hpp"

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
#include <vector>

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
        responses_.assign(static_cast<size_t>(expected), std::move(body));
        start();
    }

    explicit MockServer(std::vector<std::string> bodies) : responses_(std::move(bodies)), expected_(static_cast<int>(responses_.size())) {
        start();
    }

private:
    void start() {
        server_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        if (server_ == kInvalidSocket || bind(server_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
            listen(server_, expected_) != 0) throw std::runtime_error("无法启动模拟 AI 服务");
        SocketLength length = sizeof(address);
        getsockname(server_, reinterpret_cast<sockaddr*>(&address), &length);
        port_ = ntohs(address.sin_port);
        worker_ = std::thread([this] { serve(); });
    }

public:
    ~MockServer() { if (worker_.joinable()) worker_.join(); closeSocket(server_); }
    int port() const { return port_; }
    int count() const { return count_; }
    const std::string& requests() const { return requests_; }
    void wait() { if (worker_.joinable()) worker_.join(); }

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
            const std::string& body = responses_[static_cast<size_t>(index)];
            std::string response = "HTTP/1.1 200 OK";
            const auto newline = [&response] {
                response.push_back(static_cast<char>(13));
                response.push_back(static_cast<char>(10));
            };
            newline();
            response += "Content-Type: application/json";
            newline();
            response += "Content-Length: " + std::to_string(body.size());
            newline();
            response += "Connection: close";
            newline();
            newline();
            response += body;
            send(client, response.data(), static_cast<int>(response.size()), 0);
            closeSocket(client);
        }
    }

    Socket server_ = kInvalidSocket;
    int port_ = 0;
    int expected_ = 0;
    std::vector<std::string> responses_;
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
    const std::string utf8 = u8"abc中文";
    expect(gangyi::truncateUtf8(utf8, 3) == "abc", "ASCII 截断必须保留完整字符");
    expect(gangyi::truncateUtf8(utf8, 4) == "abc", "不得保留半个中文字符");
    expect(gangyi::truncateUtf8(utf8, 6) == u8"abc中", "中文结束边界必须保留完整字符");
    expect(gangyi::truncateUtf8(utf8, 9) == utf8, "长度上限足够时必须保留完整内容");
    const std::string incomplete = std::string("abc") + static_cast<char>(0xE4) + static_cast<char>(0xB8);
    const std::string validPrefix = gangyi::truncateUtf8(incomplete, incomplete.size());
    expect(validPrefix == "abc", "不完整 UTF-8 末尾必须返回已确认合法的前缀");
    try {
        const std::string dumped = nlohmann::json{{"value", validPrefix}}.dump();
        expect(!dumped.empty(), "安全截断结果必须可执行 JSON dump");
    } catch (...) {
        expect(false, "安全截断结果不得导致 JSON dump 异常");
    }
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
        gangyi::SearchResource resource;
        resource.description = std::string(299, 'a') + "中";
        const auto result = generator.generateBlock(goal, plan, phase,
            topic, 1, "deep", "overview", nlohmann::json::object(), {resource});
        server.wait();
        expect(result["_generation"].value("source", "") == "ai", "成功板块必须记录 AI 来源");
        expect(result["_generation"].value("model", "") == "configured-model", "必须记录实际返回模型");
        expect(server.requests().find("configured-model") != std::string::npos, "必须使用配置的模型");
        expect(server.requests().find("deepseek-v4-flash") == std::string::npos, "不得硬模型");
        expect(server.requests().find("coursePlan") != std::string::npos, "请求必须携带已保存课程主线");
        expect(server.requests().find("previousBlocks") != std::string::npos, "请求必须携带已生成前置板块");
        expect(server.requests().find("原样出现输入中的 goal、phase、topic") != std::string::npos,
            "提示词必须与目标、阶段、主题的质量门禁一致");
        expect(server.requests().find("禁止使用反斜杠或 LaTeX 命令") != std::string::npos,
            "数学内容必须避免生成破坏 JSON 的 LaTeX 反斜杠");
        expect(server.requests().find("\"max_tokens\":6000") != std::string::npos,
            "深度板块必须预留完整 JSON 输出空间");
        expect(server.requests().find(std::string(299, 'a')) != std::string::npos,
            "资源摘要截断不得破坏 UTF-8，且必须继续调用 AI");
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
        const std::string lengthResponse = nlohmann::json{{"model", "configured-model"},
            {"choices", nlohmann::json::array({{{"message", {{"content", ""}, {"reasoning_content", "内部推理"}}},
                {"finish_reason", "length"}}})}}.dump();
        const nlohmann::json content = {{"title", topic + "入门"},
            {"summary", "围绕" + goal + "，在" + phase + "阶段学习" + topic}, {"inferredDomain", "高中化学"},
            {"keyConcepts", {topic, "氧化剂", "还原剂"}}};
        MockServer server(std::vector<std::string>{lengthResponse, providerResponse(content)});
        auto client = clientFor(server);
        gangyi::LearningGenerator generator(client);
        const auto result = generator.generateBlock(goal, plan, phase,
            topic, 1, "deep", "overview", nlohmann::json::object(), {});
        server.wait();
        expect(result["_generation"].value("attempts", 0) == 2, "length 后必须重新调用真实 AI 并记录第二次成功");
        expect(server.count() == 2, "第一次 length、第二次成功必须恰好调用两次真实 AI");
        expect(server.requests().find("上一次输出达到长度限制") != std::string::npos,
            "length 后的真实 AI 重试必须携带专门的长度反馈");
    }
    {
        const nlohmann::json content = {{"title", topic + "入门"},
            {"summary", "围绕" + goal + "，在" + phase + "阶段学习" + topic}, {"inferredDomain", "高中化学"},
            {"keyConcepts", {topic, "氧化剂", "还原剂"}}};
        MockServer server(providerResponse(content), 1);
        auto client = clientFor(server);
        gangyi::LearningGenerator generator(client);
        nlohmann::json previous = nlohmann::json::object();
        previous["steps"] = {{"lessonSteps", nlohmann::json::array({{{"title", "前置步骤"},
            {"explanation", std::string(30000, 'x')}, {"example", std::string(30000, 'y')},
            {"action", std::string(30000, 'z')}, {"check", "检查"}}})}};
        (void)generator.generateBlock(goal, plan, phase,
            topic, 1, "deep", "overview", previous, {});
        server.wait();
        expect(server.requests().size() < 20000, "后续板块不得重复发送全部前置正文");
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
        expect(server.requests().find("所有数组必须使用 JSON 数组") != std::string::npos,
            "阶段提示词必须明确嵌套数组类型");
        expect(server.requests().find("\"max_tokens\":6000") != std::string::npos,
            "深度阶段必须预留完整 JSON 输出空间");
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return failures == 0 ? 0 : 1;
}

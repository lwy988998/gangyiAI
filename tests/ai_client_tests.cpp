// 【逐行批注】 引入头文件 "ai_client.hpp"，为后续实现提供类型或接口声明。
#include "ai_client.hpp"
// 【逐行批注】 引入头文件 "ask_generator.hpp"，为后续实现提供类型或接口声明。
#include "ask_generator.hpp"
// 【逐行批注】 引入头文件 "json_fix.hpp"，为后续实现提供类型或接口声明。
#include "json_fix.hpp"
// 【逐行批注】 引入头文件 "plan_generator.hpp"，为后续实现提供类型或接口声明。
#include "plan_generator.hpp"
// 【逐行批注】 引入头文件 "search_client.hpp"，为后续实现提供类型或接口声明。
#include "search_client.hpp"
// 【逐行批注】 引入头文件 <curl/curl.h>，为后续实现提供类型或接口声明。
#include <curl/curl.h>

// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
// 【逐行批注】 处理条件编译指令：#ifndef NOMINMAX。
#ifndef NOMINMAX
// 【逐行批注】 定义编译期常量或宏：#define NOMINMAX。
#define NOMINMAX
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 <winsock2.h>，为后续实现提供类型或接口声明。
#include <winsock2.h>
// 【逐行批注】 引入头文件 <ws2tcpip.h>，为后续实现提供类型或接口声明。
#include <ws2tcpip.h>
// 【逐行批注】 计算右侧结果并写入 using Socket，更新当前运行状态。
using Socket = SOCKET;
// 【逐行批注】 计算右侧结果并写入 using SocketLength，更新当前运行状态。
using SocketLength = int;
// 【逐行批注】 计算右侧结果并写入 constexpr Socket kInvalidSocket，更新当前运行状态。
constexpr Socket kInvalidSocket = INVALID_SOCKET;
// 【逐行批注】 处理条件编译指令：#else。
#else
// 【逐行批注】 引入头文件 <arpa/inet.h>，为后续实现提供类型或接口声明。
#include <arpa/inet.h>
// 【逐行批注】 引入头文件 <netinet/in.h>，为后续实现提供类型或接口声明。
#include <netinet/in.h>
// 【逐行批注】 引入头文件 <sys/socket.h>，为后续实现提供类型或接口声明。
#include <sys/socket.h>
// 【逐行批注】 引入头文件 <unistd.h>，为后续实现提供类型或接口声明。
#include <unistd.h>
// 【逐行批注】 计算右侧结果并写入 using Socket，更新当前运行状态。
using Socket = int;
// 【逐行批注】 计算右侧结果并写入 using SocketLength，更新当前运行状态。
using SocketLength = socklen_t;
// 【逐行批注】 计算右侧结果并写入 constexpr Socket kInvalidSocket，更新当前运行状态。
constexpr Socket kInvalidSocket = -1;
// 【逐行批注】 处理条件编译指令：#endif。
#endif

// 【逐行批注】 引入头文件 <atomic>，为后续实现提供类型或接口声明。
#include <atomic>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <condition_variable>，为后续实现提供类型或接口声明。
#include <condition_variable>
// 【逐行批注】 引入头文件 <mutex>，为后续实现提供类型或接口声明。
#include <mutex>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <future>，为后续实现提供类型或接口声明。
#include <future>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 int failures，更新当前运行状态。
int failures = 0;

// 【逐行批注】 定义函数 expect，执行该函数声明的职责。
void expect(bool condition, const char* message) {
    // 【逐行批注】 判断条件 (!condition) {，成立时执行对应分支。
    if (!condition) {
        // 【逐行批注】 执行当前声明或表达式：std::cerr << "失败: " << message << '\n';。
        std::cerr << "失败: " << message << '\n';
        // 【逐行批注】 执行当前声明或表达式：++failures;。
        ++failures;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 closeSocket，执行该函数声明的职责。
void closeSocket(Socket socket) {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 closesocket 完成本行所需操作：closesocket(socket);。
    closesocket(socket);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 close 完成本行所需操作：close(socket);。
    close(socket);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明数据结构或类型 MockServer 及其成员接口。
class MockServer {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 计算右侧结果并写入 MockServer(std::string response, int delayMs，更新当前运行状态。
    MockServer(std::string response, int delayMs = 0)
        // 【逐行批注】 声明对象 MockServer，并通过构造函数初始化其状态。
        : MockServer(std::vector<std::string>{std::move(response)}, delayMs) {}

    // 【逐行批注】 调用 MockServer 完成本行所需操作：MockServer(std::vector<std::string> responses, int delayMs = 0, bool …。
    MockServer(std::vector<std::string> responses, int delayMs = 0, bool holdResponses = false)
        // 【逐行批注】 定义函数 responses_，执行该函数声明的职责。
        : responses_(std::move(responses)), delayMs_(delayMs), released_(!holdResponses) {
        // 【逐行批注】 计算右侧结果并写入 server_，更新当前运行状态。
        server_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // 【逐行批注】 判断条件 (server_ == kInvalidSocket) throw std::runtime_error("无法创建模拟服务器套…，成立时执行对应分支。
        if (server_ == kInvalidSocket) throw std::runtime_error("无法创建模拟服务器套接字");
        // 【逐行批注】 执行当前声明或表达式：sockaddr_in address{};。
        sockaddr_in address{};
        // 【逐行批注】 计算右侧结果并写入 address.sin_family，更新当前运行状态。
        address.sin_family = AF_INET;
        // 【逐行批注】 计算右侧结果并写入 address.sin_addr.s_addr，更新当前运行状态。
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        // 【逐行批注】 计算右侧结果并写入 address.sin_port，更新当前运行状态。
        address.sin_port = 0;
        // 【逐行批注】 判断条件 (bind(server_, reinterpret_cast<sockaddr*>(&address), sizeof(add…，成立时执行对应分支。
        if (bind(server_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 || listen(server_, 1) != 0) {
            // 【逐行批注】 调用 closeSocket 完成本行所需操作：closeSocket(server_);。
            closeSocket(server_);
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("无法启动模拟服务器");。
            throw std::runtime_error("无法启动模拟服务器");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 SocketLength length，更新当前运行状态。
        SocketLength length = sizeof(address);
        // 【逐行批注】 判断条件 (getsockname(server_, reinterpret_cast<sockaddr*>(&address), &le…，成立时执行对应分支。
        if (getsockname(server_, reinterpret_cast<sockaddr*>(&address), &length) != 0) {
            // 【逐行批注】 调用 closeSocket 完成本行所需操作：closeSocket(server_);。
            closeSocket(server_);
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("无法读取模拟服务器端口");。
            throw std::runtime_error("无法读取模拟服务器端口");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 port_，更新当前运行状态。
        port_ = ntohs(address.sin_port);
        // 【逐行批注】 计算右侧结果并写入 worker_，更新当前运行状态。
        worker_ = std::thread([this] { serve(); });
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 调用 MockServer 完成本行所需操作：~MockServer() {。
    ~MockServer() {
        // 【逐行批注】 调用 release 完成本行所需操作：release();。
        release();
        // 【逐行批注】 调用 wait 完成本行所需操作：wait();。
        wait();
        // 【逐行批注】 调用 closeSocket 完成本行所需操作：closeSocket(server_);。
        closeSocket(server_);
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 定义函数 wait，执行该函数声明的职责。
    void wait() {
        // 【逐行批注】 判断条件 (worker_.joinable()) worker_.join();，成立时执行对应分支。
        if (worker_.joinable()) worker_.join();
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 port，执行该函数声明的职责。
    int port() const { return port_; }
    // 【逐行批注】 定义函数 requestCount，执行该函数声明的职责。
    int requestCount() const { return requestCount_; }
    // 【逐行批注】 定义函数 waitRequests，执行该函数声明的职责。
    bool waitRequests(int count, int milliseconds = 2000) {
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        std::unique_lock<std::mutex> lock(gateMutex_);
        // 【逐行批注】 结束当前函数并返回结果：return gate_.wait_for(lock, std::chrono::milliseconds(milliseconds), …。
        return gate_.wait_for(lock, std::chrono::milliseconds(milliseconds), [&] { return requestCount_ >= count; });
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 release，执行该函数声明的职责。
    void release() { { std::lock_guard<std::mutex> lock(gateMutex_); released_ = true; } gate_.notify_all(); }
    // 【逐行批注】 定义函数 request，执行该函数声明的职责。
    const std::string& request() const { return request_; }

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 定义函数 serve，执行该函数声明的职责。
    void serve() {
        // 【逐行批注】 按循环规则重复处理：for (const auto& responseText : responses_) {。
        for (const auto& responseText : responses_) {
            // 【逐行批注】 执行当前声明或表达式：sockaddr_in clientAddress{};。
            sockaddr_in clientAddress{};
            // 【逐行批注】 计算右侧结果并写入 SocketLength length，更新当前运行状态。
            SocketLength length = sizeof(clientAddress);
            // 【逐行批注】 调用 FD_ZERO、FD_SET 完成本行所需操作：fd_set ready; FD_ZERO(&ready); FD_SET(server_, &ready); timeval timeo…。
            fd_set ready; FD_ZERO(&ready); FD_SET(server_, &ready); timeval timeout{5, 0};
            // 【逐行批注】 判断条件 (select(static_cast<int>(server_) + 1, &ready, nullptr, nullptr,…，成立时执行对应分支。
            if (select(static_cast<int>(server_) + 1, &ready, nullptr, nullptr, &timeout) <= 0) return;
            // 【逐行批注】 计算右侧结果并写入 Socket client，更新当前运行状态。
            Socket client = accept(server_, reinterpret_cast<sockaddr*>(&clientAddress), &length);
            // 【逐行批注】 判断条件 (client == kInvalidSocket) return;，成立时执行对应分支。
            if (client == kInvalidSocket) return;
            // 【逐行批注】 执行当前声明或表达式：++requestCount_;。
            ++requestCount_;
            // 【逐行批注】 执行当前声明或表达式：gate_.notify_all();。
            gate_.notify_all();
            // 【逐行批注】 执行当前声明或表达式：std::string currentRequest;。
            std::string currentRequest;
            // 【逐行批注】 执行当前声明或表达式：char buffer[2048];。
            char buffer[2048];
            // 【逐行批注】 在条件成立期间循环处理：while (currentRequest.find("\r\n\r\n") == std::string::npos) {。
            while (currentRequest.find("\r\n\r\n") == std::string::npos) {
                // 【逐行批注】 计算右侧结果并写入 int received，更新当前运行状态。
                const int received = recv(client, buffer, sizeof(buffer), 0);
                // 【逐行批注】 判断条件 (received <= 0) break;，成立时执行对应分支。
                if (received <= 0) break;
                // 【逐行批注】 执行当前声明或表达式：currentRequest.append(buffer, static_cast<size_t>(received));。
                currentRequest.append(buffer, static_cast<size_t>(received));
                // TLS失败夹具收到ClientHello即回送明文HTTP，不等待不存在的HTTP请求头。
                // 【逐行批注】 判断条件 (!currentRequest.empty() && static_cast<unsigned char>(currentRe…，成立时执行对应分支。
                if (!currentRequest.empty() && static_cast<unsigned char>(currentRequest.front()) == 0x16) break;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 auto headerEnd，更新当前运行状态。
            const auto headerEnd = currentRequest.find("\r\n\r\n");
            // 【逐行批注】 计算右侧结果并写入 auto lengthStart，更新当前运行状态。
            const auto lengthStart = currentRequest.find("Content-Length:");
            // 【逐行批注】 判断条件 (headerEnd != std::string::npos && lengthStart != std::string::n…，成立时执行对应分支。
            if (headerEnd != std::string::npos && lengthStart != std::string::npos) {
                // 【逐行批注】 计算右侧结果并写入 auto valueStart，更新当前运行状态。
                const auto valueStart = currentRequest.find_first_not_of(" ", lengthStart + 15);
                // 【逐行批注】 计算右侧结果并写入 auto valueEnd，更新当前运行状态。
                const auto valueEnd = currentRequest.find("\r\n", valueStart);
                // 【逐行批注】 计算右侧结果并写入 size_t contentLength，更新当前运行状态。
                const size_t contentLength = static_cast<size_t>(
                    // 【逐行批注】 调用 stoul 完成本行所需操作：std::stoul(currentRequest.substr(valueStart, valueEnd - valueStart)));。
                    std::stoul(currentRequest.substr(valueStart, valueEnd - valueStart)));
                // 【逐行批注】 在条件成立期间循环处理：while (currentRequest.size() < headerEnd + 4 + contentLength) {。
                while (currentRequest.size() < headerEnd + 4 + contentLength) {
                    // 【逐行批注】 计算右侧结果并写入 int received，更新当前运行状态。
                    const int received = recv(client, buffer, sizeof(buffer), 0);
                    // 【逐行批注】 判断条件 (received <= 0) break;，成立时执行对应分支。
                    if (received <= 0) break;
                    // 【逐行批注】 执行当前声明或表达式：currentRequest.append(buffer, static_cast<size_t>(received));。
                    currentRequest.append(buffer, static_cast<size_t>(received));
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 request_ +，更新当前运行状态。
            request_ += currentRequest;
            // 【逐行批注】 调用 lock 完成本行所需操作：{ std::unique_lock<std::mutex> lock(gateMutex_); gate_.wait(lock, [&]…。
            { std::unique_lock<std::mutex> lock(gateMutex_); gate_.wait(lock, [&] { return released_; }); }
            // 【逐行批注】 判断条件 (delayMs_ > 0) std::this_thread::sleep_for(std::chrono::millisec…，成立时执行对应分支。
            if (delayMs_ > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delayMs_));
            // 【逐行批注】 判断条件 (!responseText.empty()) send(client, responseText.data(), static…，成立时执行对应分支。
            if (!responseText.empty()) send(client, responseText.data(), static_cast<int>(responseText.size()),
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
                // 【逐行批注】 执行当前声明或表达式：0。
                0
// 【逐行批注】 处理条件编译指令：#else。
#else
                // 【逐行批注】 执行当前声明或表达式：MSG_NOSIGNAL。
                MSG_NOSIGNAL
// 【逐行批注】 处理条件编译指令：#endif。
#endif
            // 【逐行批注】 执行当前声明或表达式：);。
            );
            // 【逐行批注】 调用 closeSocket 完成本行所需操作：closeSocket(client);。
            closeSocket(client);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 计算右侧结果并写入 Socket server_，更新当前运行状态。
    Socket server_ = kInvalidSocket;
    // 【逐行批注】 计算右侧结果并写入 int port_，更新当前运行状态。
    int port_ = 0;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> responses_;。
    std::vector<std::string> responses_;
    // 【逐行批注】 计算右侧结果并写入 int delayMs_，更新当前运行状态。
    int delayMs_ = 0;
    // 【逐行批注】 执行当前声明或表达式：std::thread worker_;。
    std::thread worker_;
    // 【逐行批注】 执行当前声明或表达式：std::mutex gateMutex_;。
    std::mutex gateMutex_;
    // 【逐行批注】 执行当前声明或表达式：std::condition_variable gate_;。
    std::condition_variable gate_;
    // 【逐行批注】 计算右侧结果并写入 bool released_，更新当前运行状态。
    bool released_ = true;
    // 【逐行批注】 执行当前声明或表达式：std::atomic<int> requestCount_{0};。
    std::atomic<int> requestCount_{0};
    // 【逐行批注】 执行当前声明或表达式：std::string request_;。
    std::string request_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 response，执行该函数声明的职责。
std::string response(int status, const std::string& body) {
    // 【逐行批注】 结束当前函数并返回结果：return "HTTP/1.1 " + std::to_string(status) + " Test\r\nContent-Type:…。
    return "HTTP/1.1 " + std::to_string(status) + " Test\r\nContent-Type: application/json\r\nContent-Length: " +
        // 【逐行批注】 调用 to_string 完成本行所需操作：std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;。
        std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 clientFor，并通过构造函数初始化其状态。
gangyi::AIClient clientFor(const MockServer& server, const std::string& path = "/v1",
                           // 【逐行批注】 计算右侧结果并写入 std::string& model，更新当前运行状态。
                           const std::string& model = "unused") {
    // 【逐行批注】 结束当前函数并返回结果：return gangyi::AIClient({"http://127.0.0.1:" + std::to_string(server.…。
    return gangyi::AIClient({"http://127.0.0.1:" + std::to_string(server.port()) + path,
                            // 【逐行批注】 执行当前声明或表达式："test-secret-key", model, 1000, 1});。
                            "test-secret-key", model, 1000, 1});
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 expectUsage，执行该函数声明的职责。
void expectUsage(const nlohmann::json& reported, const nlohmann::json& expected) {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 同一份真实格式回执分别经过普通响应和SSE解析，不访问外部模型。
    // 【逐行批注】 按循环规则重复处理：for (const bool stream : {false, true}) {。
    for (const bool stream : {false, true}) {
        // 【逐行批注】 计算右侧结果并写入 Json completion，更新当前运行状态。
        Json completion = {{"model", "usage-fixture"}, {"usage", reported},
            // 【逐行批注】 调用 array 完成本行所需操作：{"choices", Json::array({{{stream ? "delta" : "message", {{"content",…。
            {"choices", Json::array({{{stream ? "delta" : "message", {{"content", "教学内容保持完整"}}},
                                      // 【逐行批注】 执行当前声明或表达式：{"finish_reason", "stop"}}})}};。
                                      {"finish_reason", "stop"}}})}};
        // 【逐行批注】 计算右侧结果并写入 auto body，更新当前运行状态。
        const auto body = stream ? "data: " + completion.dump() + "\n\ndata: [DONE]\n\n" : completion.dump();
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, body));
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server);
        // 【逐行批注】 计算右侧结果并写入 gangyi::ChatOptions options; options.messages，更新当前运行状态。
        gangyi::ChatOptions options; options.messages = {{"user", "保留原有教学请求"}};
        // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
        const auto result = stream ? client.chatStream(options, [](const std::string&) { return true; }) : client.chat(options);
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result.tokenUsage == expected, "两种响应必须保留有效用量明细，未知或无效字段不得补零");。
        expect(result.tokenUsage == expected, "两种响应必须保留有效用量明细，未知或无效字段不得补零");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result.content == "教学内容保持完整", "统计明细不得改变教学回复");。
        expect(result.content == "教学内容保持完整", "统计明细不得改变教学回复");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requestCount() == 1, "保存用量明细不得增加模型请求");。
        expect(server.requestCount() == 1, "保存用量明细不得增加模型请求");
        // 【逐行批注】 计算右侧结果并写入 auto payload，更新当前运行状态。
        const auto payload = Json::parse(server.request().substr(server.request().find("\r\n\r\n") + 4));
        // 【逐行批注】 调用 expect、array 完成本行所需操作：expect(payload.at("messages") == Json::array({{{"role", "user"}, {"co…。
        expect(payload.at("messages") == Json::array({{{"role", "user"}, {"content", "保留原有教学请求"}}}) &&
               // 【逐行批注】 执行当前声明或表达式：!payload.contains("thinking") && !payload.contains("reasoning_effort"),。
               !payload.contains("thinking") && !payload.contains("reasoning_effort"),
               // 【逐行批注】 执行当前声明或表达式："用量统计不得修改教学输入和现有思考策略");。
               "用量统计不得修改教学输入和现有思考策略");
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 通过本地 HTTP 服务器核对实际请求：多页图像和普通消息在两种调用方式中保持一致。
void expectMultimodalRequests() {
    using Json = nlohmann::json;
    const std::string firstImage = "data:image/png;base64,iVBORw0KGgo=";
    const std::string secondImage = "data:image/jpeg;base64,/9j/4AAQ";
    Json previousMessages;
    for (const bool stream : {false, true}) {
        const std::string body = stream
            ? "data: {\"choices\":[{\"delta\":{\"content\":\"已收到\"},\"finish_reason\":\"stop\"}]}\n\ndata: [DONE]\n\n"
            : R"({"choices":[{"message":{"content":"已收到"},"finish_reason":"stop"}]})";
        MockServer server(response(200, body));
        auto client = clientFor(server);
        gangyi::ChatOptions options;
        options.messages = {{"system", "按原卷逐题识别", ""},
                            {"user", "第 1 页：含引号 \"与换行\n", firstImage},
                            {"user", "第 2 页", secondImage},
                            {"user", "补充说明", ""}};
        const auto result = stream ? client.chatStream(options, [](const std::string&) { return true; }) : client.chat(options);
        server.wait();
        expect(result.content == "已收到" && server.requestCount() == 1, "模拟服务必须实际接收一次请求并返回结果");
        const auto request = server.request();
        const auto bodyOffset = request.find("\r\n\r\n");
        expect(bodyOffset != std::string::npos, "请求缺少 HTTP 消息体");
        const auto payload = Json::parse(request.substr(bodyOffset + 4));
        const auto& messages = payload.at("messages");
        expect(messages.size() == 4 && messages[0].at("content") == "按原卷逐题识别" &&
               messages[3].at("content") == "补充说明", "纯文本消息和顺序必须保持不变");
        expect(messages[1].at("content")[0].at("text") == "第 1 页：含引号 \"与换行\n" &&
               messages[1].at("content")[1].at("type") == "image_url" &&
               messages[1].at("content")[1].at("image_url").at("url") == firstImage &&
               messages[2].at("content")[0].at("text") == "第 2 页" &&
               messages[2].at("content")[1].at("image_url").at("url") == secondImage,
               "试卷每页图像必须以正确顺序实际发送到模型接口");
        if (stream) expect(messages == previousMessages && payload.value("stream", false), "流式与普通请求必须发送相同图文消息");
        else previousMessages = messages;
    }
}

// 【逐行批注】 定义函数 expectError，执行该函数声明的职责。
void expectError(const char* expectedType, MockServer& server, int timeoutMs = 1000) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 调用 clientFor 完成本行所需操作：clientFor(server).listModels(timeoutMs);。
        clientFor(server).listModels(timeoutMs);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(false, "请求应当失败");。
        expect(false, "请求应当失败");
    // 【逐行批注】 执行当前声明或表达式：} catch (const gangyi::AIClientError& error) {。
    } catch (const gangyi::AIClientError& error) {
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(error.errorType == expectedType, "错误分类不正确");。
        expect(error.errorType == expectedType, "错误分类不正确");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：server.wait();。
    server.wait();
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requestCount() == 1, "获取模型不得自动重试");。
    expect(server.requestCount() == 1, "获取模型不得自动重试");
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 引入头文件 "retry_coordination_checks.hpp"，为后续实现提供类型或接口声明。
#include "retry_coordination_checks.hpp"
// 【逐行批注】 引入头文件 "transport_diagnostic_checks.hpp"，为后续实现提供类型或接口声明。
#include "transport_diagnostic_checks.hpp"

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main(int argc, char** argv) {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 执行当前声明或表达式：WSADATA data{};。
    WSADATA data{};
    // 【逐行批注】 判断条件 (WSAStartup(MAKEWORD(2, 2), &data) != 0) return 2;，成立时执行对应分支。
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return 2;
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    expectMultimodalRequests();
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 auto parsed，更新当前运行状态。
        const auto parsed = gangyi::parseAIJson(
            // 【逐行批注】 执行当前声明或表达式：R"(分析中可能会提到示例 {x: 1}，最终结果如下：```json。
            R"(分析中可能会提到示例 {x: 1}，最终结果如下：```json
{"goal":"掌握函数单调性","summary":"完成定义与题型练习"}
```)"
        // 【逐行批注】 执行当前声明或表达式：);。
        );
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(parsed.value("goal", "") == "掌握函数单调性",。
        expect(parsed.value("goal", "") == "掌握函数单调性",
               // 【逐行批注】 执行当前声明或表达式："JSON 解析器应提取推理文本末尾的完整对象");。
               "JSON 解析器应提取推理文本末尾的完整对象");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (argc == 4 && std::string(argv[1]) == "--monitor-probe") {，成立时执行对应分支。
    if (argc == 4 && std::string(argv[1]) == "--monitor-probe") {
        // 【逐行批注】 调用 configure 完成本行所需操作：gangyi::AIActivity::configure(argv[2]);。
        gangyi::AIActivity::configure(argv[2]);
        // 【逐行批注】 声明对象 client，并通过构造函数初始化其状态。
        gangyi::AIClient client({argv[3], "fictional-fixture-key", "fixture", 10000, 1});
        // 【逐行批注】 计算右侧结果并写入 gangyi::ChatOptions options; options.messages，更新当前运行状态。
        gangyi::ChatOptions options; options.messages = {{"user", "Reply with OK."}}; options.maxTokens = 16; options.maxAttempts = 1;
        // 【逐行批注】 计算右侧结果并写入 options.activity.source，更新当前运行状态。
        options.activity.source = "AI 设置"; options.activity.purpose = "测试 API 地址、密钥和模型是否可用";
        // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
        const auto result = client.chat(options);
        // 【逐行批注】 结束当前函数并返回结果：return result.content == "OK" ? 0 : 1;。
        return result.content == "OK" ? 0 : 1;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
        using Json = nlohmann::json;
        // 【逐行批注】 计算右侧结果并写入 Json full，更新当前运行状态。
        const Json full = {{"prompt_tokens", 10}, {"completion_tokens", 20}, {"total_tokens", 30},
            // 【逐行批注】 执行当前声明或表达式：{"prompt_cache_hit_tokens", 6}, {"prompt_cache_miss_tokens", 4},。
            {"prompt_cache_hit_tokens", 6}, {"prompt_cache_miss_tokens", 4},
            // 【逐行批注】 执行当前声明或表达式：{"prompt_tokens_details", {{"cached_tokens", 6}}},。
            {"prompt_tokens_details", {{"cached_tokens", 6}}},
            // 【逐行批注】 执行当前声明或表达式：{"completion_tokens_details", {{"reasoning_tokens", 15}}}};。
            {"completion_tokens_details", {{"reasoning_tokens", 15}}}};
        // 【逐行批注】 计算右侧结果并写入 withExtra，更新当前运行状态。
        auto withExtra = full; withExtra["unknown"] = "忽略未经识别的内容";
        // 【逐行批注】 计算右侧结果并写入 withExtra["completion_tokens_details"]["reas…，更新当前运行状态。
        withExtra["completion_tokens_details"]["reasoning_content"] = "不作为统计保存的文本";
        // 【逐行批注】 调用 expectUsage 完成本行所需操作：expectUsage(withExtra, full);。
        expectUsage(withExtra, full);
        // 【逐行批注】 调用 expectUsage 完成本行所需操作：expectUsage({{"prompt_tokens", 0}, {"completion_tokens", "20"}, {"tot…。
        expectUsage({{"prompt_tokens", 0}, {"completion_tokens", "20"}, {"total_tokens", -1},
            // 【逐行批注】 执行当前声明或表达式：{"prompt_cache_hit_tokens", false}, {"prompt_cache_miss_tokens", 3.5},。
            {"prompt_cache_hit_tokens", false}, {"prompt_cache_miss_tokens", 3.5},
            // 【逐行批注】 执行当前声明或表达式：{"prompt_tokens_details", {{"cached_tokens", -1}}},。
            {"prompt_tokens_details", {{"cached_tokens", -1}}},
            // 【逐行批注】 执行当前声明或表达式：{"completion_tokens_details", {{"reasoning_tokens", "15"}}}}, {{"prompt_tokens", 0}}…。
            {"completion_tokens_details", {{"reasoning_tokens", "15"}}}}, {{"prompt_tokens", 0}});
        // 【逐行批注】 计算右侧结果并写入 Json partial，更新当前运行状态。
        const Json partial = {{"prompt_tokens_details", {{"cached_tokens", 0}}},
            // 【逐行批注】 执行当前声明或表达式：{"completion_tokens_details", {{"reasoning_tokens", 0}}}};。
            {"completion_tokens_details", {{"reasoning_tokens", 0}}}};
        // 【逐行批注】 调用 expectUsage 完成本行所需操作：expectUsage(partial, partial);。
        expectUsage(partial, partial);
        // 【逐行批注】 调用 expectUsage 完成本行所需操作：expectUsage(nullptr, nullptr);。
        expectUsage(nullptr, nullptr);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 真实模型偶尔漏写嵌套对象的右花括号；补全只能加括号，不能改写已有字段。
        // 【逐行批注】 计算右侧结果并写入 std::string valid，更新当前运行状态。
        const std::string valid = R"({"message":"先标化合价","actions":[{"id":"a1","tool":"create_lesson","args":{"courseId":"c1","title":"氧化还原"}}],"state":"completed"})";
        // 【逐行批注】 计算右侧结果并写入 auto missing，更新当前运行状态。
        const auto missing = valid.find("}}],\"state\"");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(missing != std::string::npos, "测试用例应包含行动对象与行动数组的连续右括号");。
        expect(missing != std::string::npos, "测试用例应包含行动对象与行动数组的连续右括号");
        // 【逐行批注】 计算右侧结果并写入 std::string broken，更新当前运行状态。
        std::string broken = valid;
        // 【逐行批注】 判断条件 (missing != std::string::npos) broken.erase(missing, 1);，成立时执行对应分支。
        if (missing != std::string::npos) broken.erase(missing, 1);
        // 【逐行批注】 计算右侧结果并写入 auto parsed，更新当前运行状态。
        const auto parsed = gangyi::parseAIJson(broken);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(parsed.value("message", "") == "先标化合价" && parsed.at("actions")…。
        expect(parsed.value("message", "") == "先标化合价" && parsed.at("actions").size() == 1,
               // 【逐行批注】 执行当前声明或表达式："缺少一个右花括号时应补齐括号并保留顶层消息与行动");。
               "缺少一个右花括号时应补齐括号并保留顶层消息与行动");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(parsed.at("actions")[0].at("args").at("title") == "氧化还原",。
        expect(parsed.at("actions")[0].at("args").at("title") == "氧化还原",
               // 【逐行批注】 执行当前声明或表达式："括号补全不得改写行动参数内容");。
               "括号补全不得改写行动参数内容");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(parsed.value("state", "") == "completed", "括号补全后应保留状态字段");。
        expect(parsed.value("state", "") == "completed", "括号补全后应保留状态字段");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 bool rejected，更新当前运行状态。
        bool rejected = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { gangyi::parseAIJson("这不是 JSON，也没有任何对象"); } catch (...) { rejected = true; }
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(rejected, "完全无法解析的文本必须报错，不能伪造主控结构");。
        expect(rejected, "完全无法解析的文本必须报错，不能伪造主控结构");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, R"({"choices":[{"message":{"content":" 好的 "}}]})"));
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server, "/v1", "configured-ask-model");
        // 【逐行批注】 声明对象 generator，并通过构造函数初始化其状态。
        gangyi::AskGenerator generator(client);
        // 【逐行批注】 计算右侧结果并写入 auto answer，更新当前运行状态。
        const auto answer = generator.generate("请解释函数单调性", "共享画像", {{"user", "最近的问题"}, {"assistant", "最近的回答"}});
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(answer.content == "好的", "问答应返回去除首尾空白的内容");。
        expect(answer.content == "好的", "问答应返回去除首尾空白的内容");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(answer.model == "configured-ask-model", "响应缺少模型字段时应回退到配置模型");。
        expect(answer.model == "configured-ask-model", "响应缺少模型字段时应回退到配置模型");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("POST /v1/chat/completions HTTP/") != st…。
        expect(server.request().find("POST /v1/chat/completions HTTP/") != std::string::npos,
               // 【逐行批注】 执行当前声明或表达式："问答应请求 Chat Completions 端点");。
               "问答应请求 Chat Completions 端点");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("\"model\":\"configured-ask-model\"") !=…。
        expect(server.request().find("\"model\":\"configured-ask-model\"") != std::string::npos,
               // 【逐行批注】 执行当前声明或表达式："问答不得覆盖用户配置的模型");。
               "问答不得覆盖用户配置的模型");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("deepseek-v4-flash") == std::string::npo…。
        expect(server.request().find("deepseek-v4-flash") == std::string::npos,
               // 【逐行批注】 执行当前声明或表达式："问答请求不得包含硬编码模型");。
               "问答请求不得包含硬编码模型");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("最近的问题") != std::string::npos &&。
        expect(server.request().find("最近的问题") != std::string::npos &&
               // 【逐行批注】 执行当前声明或表达式：server.request().find("最近的回答") != std::string::npos &&。
               server.request().find("最近的回答") != std::string::npos &&
               // 【逐行批注】 执行当前声明或表达式：server.request().find("共享画像") != std::string::npos, "导师应包含最近对话与共享画像");。
               server.request().find("共享画像") != std::string::npos, "导师应包含最近对话与共享画像");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 std::string events，更新当前运行状态。
        const std::string events = "data: {\"model\":\"mock\",\"choices\":[{\"delta\":{\"content\":\"第一段\"}}]}\n\n"
            // 【逐行批注】 执行当前声明或表达式："data:{\"model\":\"mock\",\"choices\":[{\"delta\":{\"content\":\"第二段\"}}]}\n\n"。
            "data:{\"model\":\"mock\",\"choices\":[{\"delta\":{\"content\":\"第二段\"}}]}\n\n"
            // 【逐行批注】 执行当前声明或表达式："data: [DONE]\n\n";。
            "data: [DONE]\n\n";
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server("HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nContent-Length: " +
            // 【逐行批注】 调用 to_string 完成本行所需操作：std::to_string(events.size()) + "\r\nConnection: close\r\n\r\n" + eve…。
            std::to_string(events.size()) + "\r\nConnection: close\r\n\r\n" + events);
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server);
        // 【逐行批注】 执行当前声明或表达式：gangyi::ChatOptions options;。
        gangyi::ChatOptions options;
        // 【逐行批注】 计算右侧结果并写入 options.messages，更新当前运行状态。
        options.messages = {{"user", "测试", ""}};
        // 【逐行批注】 计算右侧结果并写入 options.responseFormat，更新当前运行状态。
        options.responseFormat = "json_object";
        // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> chunks;。
        std::vector<std::string> chunks;
        // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
        const auto result = client.chatStream(options, [&](const std::string& chunk) { chunks.push_back(chunk); return true; });
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(chunks.size() == 2 && chunks[0] == "第一段" && chunks[1] == "第二段"…。
        expect(chunks.size() == 2 && chunks[0] == "第一段" && chunks[1] == "第二段", "流式回调应逐段返回内容");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result.content == "第一段第二段" && result.model == "mock", "流式结果应合并…。
        expect(result.content == "第一段第二段" && result.model == "mock", "流式结果应合并内容和模型");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("\"stream\":true") != std::string::npos,…。
        expect(server.request().find("\"stream\":true") != std::string::npos, "流式请求应启用 stream 字段");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("\"response_format\":{\"type\":\"json_ob…。
        expect(server.request().find("\"response_format\":{\"type\":\"json_object\"}") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："流式教学主控应向模型请求 JSON 对象格式");。
            "流式教学主控应向模型请求 JSON 对象格式");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 服务商明确报告长度截断时，不把片段当成功响应或自动原样重放。
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, R"({"choices":[{"message":{"content":"未完成"},"finish_reason":"length"}],"usage":{"prompt_tokens":1,"completion_tokens":8,"total_tokens":9}})"));
        // 【逐行批注】 计算右侧结果并写入 gangyi::ChatOptions options; options.messages，更新当前运行状态。
        gangyi::ChatOptions options; options.messages = {{"user", "普通响应长度限制"}};
        // 【逐行批注】 计算右侧结果并写入 bool limited，更新当前运行状态。
        bool limited = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { clientFor(server).chat(options); }
        // 【逐行批注】 捕获并处理异常：catch (const gangyi::AIClientError& error) { limited = error.errorTyp…。
        catch (const gangyi::AIClientError& error) { limited = error.errorType == "output_limit"; }
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(limited && server.requestCount() == 1, "普通响应截断必须报告长度限制，不交付片段");。
        expect(limited && server.requestCount() == 1, "普通响应截断必须报告长度限制，不交付片段");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 std::string events，更新当前运行状态。
        const std::string events = "data: {\"choices\":[{\"delta\":{\"content\":\"未完成\"},\"finish_reason\":\"length\"}],\"usage\":{\"prompt_tokens\":1,\"completion_tokens\":8,\"total_tokens\":9}}\n\ndata: [DONE]\n\n";
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, events));
        // 【逐行批注】 计算右侧结果并写入 gangyi::ChatOptions options; options.messages，更新当前运行状态。
        gangyi::ChatOptions options; options.messages = {{"user", "流式响应长度限制"}};
        // 【逐行批注】 计算右侧结果并写入 bool limited，更新当前运行状态。
        bool limited = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { clientFor(server).chatStream(options, [](const std::string&) { return true; }); }
        // 【逐行批注】 捕获并处理异常：catch (const gangyi::AIClientError& error) { limited = error.errorTyp…。
        catch (const gangyi::AIClientError& error) { limited = error.errorType == "output_limit"; }
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(limited && server.requestCount() == 1, "流式截断必须报告长度限制，不在SDK中重复相…。
        expect(limited && server.requestCount() == 1, "流式截断必须报告长度限制，不在SDK中重复相同预算");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 std::string events，更新当前运行状态。
        const std::string events = "data: {\"model\":\"mock\",\"choices\":[{\"delta\":{\"content\":\"测试\"}}]}\n\ndata: [DONE]\n\n";
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server("HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nContent-Length: " +
            // 【逐行批注】 调用 to_string 完成本行所需操作：std::to_string(events.size()) + "\r\nConnection: close\r\n\r\n" + eve…。
            std::to_string(events.size()) + "\r\nConnection: close\r\n\r\n" + events);
        // 【逐行批注】 计算右侧结果并写入 gangyi::ChatOptions options; options.messages，更新当前运行状态。
        gangyi::ChatOptions options; options.messages = {{"user", "回调异常测试"}};
        // 【逐行批注】 计算右侧结果并写入 bool caught，更新当前运行状态。
        bool caught = false;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 调用 clientFor、runtime_error 完成本行所需操作：clientFor(server).chatStream(options, [](const std::string&) -> bool …。
            clientFor(server).chatStream(options, [](const std::string&) -> bool { throw std::runtime_error("回调保存失败"); });
        // 【逐行批注】 计算右侧结果并写入 } catch (const std::runtime_error& error) { …，更新当前运行状态。
        } catch (const std::runtime_error& error) { caught = std::string(error.what()) == "回调保存失败"; }
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(caught, "流式回调异常应在释放 C 客户端资源后保持原始原因");。
        expect(caught, "流式回调异常应在释放 C 客户端资源后保持原始原因");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, R"({"data":[{"id":"z-model"},{"id":"a-model"},{"id":"a-model"}]})"));
        // 【逐行批注】 计算右侧结果并写入 auto models，更新当前运行状态。
        const auto models = clientFor(server, "/v1/chat/completions/").listModels();
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(models == std::vector<std::string>({"a-model", "z-model"}), "模…。
        expect(models == std::vector<std::string>({"a-model", "z-model"}), "模型应排序并去重");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("GET /v1/models HTTP/") != std::string::…。
        expect(server.request().find("GET /v1/models HTTP/") != std::string::npos,
               // 【逐行批注】 执行当前声明或表达式："尾部斜杠的 chat 端点应正确归一化");。
               "尾部斜杠的 chat 端点应正确归一化");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("Authorization: Bearer test-secret-key")…。
        expect(server.request().find("Authorization: Bearer test-secret-key") != std::string::npos,
               // 【逐行批注】 执行当前声明或表达式："请求应使用当前未保存的 Key");。
               "请求应使用当前未保存的 Key");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requestCount() == 1, "成功请求只发送一次");。
        expect(server.requestCount() == 1, "成功请求只发送一次");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(401, R"({"error":"unauthorized"})"));
        // 【逐行批注】 调用 expectError 完成本行所需操作：expectError("auth_error", server);。
        expectError("auth_error", server);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(404, R"({"error":"missing"})"));
        // 【逐行批注】 调用 expectError 完成本行所需操作：expectError("models_unsupported", server);。
        expectError("models_unsupported", server);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(429, R"({"error":"limited"})"));
        // 【逐行批注】 调用 expectError 完成本行所需操作：expectError("rate_limited", server);。
        expectError("rate_limited", server);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(500, R"({"error":"temporary"})"));
        // 【逐行批注】 调用 expectError 完成本行所需操作：expectError("provider_5xx", server);。
        expectError("provider_5xx", server);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, "not-json"));
        // 【逐行批注】 调用 expectError 完成本行所需操作：expectError("invalid_response", server);。
        expectError("invalid_response", server);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, R"({"data":[]})"));
        // 【逐行批注】 调用 expectError 完成本行所需操作：expectError("invalid_response", server);。
        expectError("invalid_response", server);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::string{}, 250);
        // 【逐行批注】 调用 expectError 完成本行所需操作：expectError("timeout", server, 50);。
        expectError("timeout", server, 50);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{
            // 【逐行批注】 调用 response 完成本行所需操作：response(500, R"({"error":"temporary"})"),。
            response(500, R"({"error":"temporary"})"),
            // 【逐行批注】 调用 response 完成本行所需操作：response(200, R"({"model":"provider-model","choices":[{"message":{"co…。
            response(200, R"({"model":"provider-model","choices":[{"message":{"content":"恢复成功"}}]})")});
        // 【逐行批注】 声明对象 client，并通过构造函数初始化其状态。
        gangyi::AIClient client({"http://127.0.0.1:" + std::to_string(server.port()) + "/v1",
                                 // 【逐行批注】 执行当前声明或表达式："test-secret-key", "configured-model", 1000, 2});。
                                 "test-secret-key", "configured-model", 1000, 2});
        // 【逐行批注】 执行当前声明或表达式：gangyi::ChatOptions options;。
        gangyi::ChatOptions options;
        // 【逐行批注】 计算右侧结果并写入 options.messages，更新当前运行状态。
        options.messages = {{"user", "测试可重试错误"}};
        // 【逐行批注】 计算右侧结果并写入 options.maxAttempts，更新当前运行状态。
        options.maxAttempts = 2;
        // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
        const auto result = client.chat(options);
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result.content == "恢复成功", "服务端临时错误后应成功返回");。
        expect(result.content == "恢复成功", "服务端临时错误后应成功返回");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result.model == "provider-model", "服务商返回模型时应按实际模型响应");。
        expect(result.model == "provider-model", "服务商返回模型时应按实际模型响应");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requestCount() == 2, "聊天请求应仅按配置重试一次");。
        expect(server.requestCount() == 2, "聊天请求应仅按配置重试一次");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{
            // 【逐行批注】 调用 response 完成本行所需操作：response(200, "not-json"),。
            response(200, "not-json"),
            // 【逐行批注】 调用 response 完成本行所需操作：response(200, R"({"model":"provider-model","choices":[{"message":{"co…。
            response(200, R"({"model":"provider-model","choices":[{"message":{"content":"恢复成功"}}]})")});
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server, "/v1", "configured-model");
        // 【逐行批注】 执行当前声明或表达式：gangyi::ChatOptions options;。
        gangyi::ChatOptions options;
        // 【逐行批注】 计算右侧结果并写入 options.messages，更新当前运行状态。
        options.messages = {{"user", "测试无效响应重试"}};
        // 【逐行批注】 计算右侧结果并写入 options.maxAttempts，更新当前运行状态。
        options.maxAttempts = 2;
        // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
        const auto result = client.chat(options);
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result.content == "恢复成功", "服务商临时返回无效内容后应成功恢复");。
        expect(result.content == "恢复成功", "服务商临时返回无效内容后应成功恢复");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requestCount() == 2, "无效聊天响应应仅按配置重试一次");。
        expect(server.requestCount() == 2, "无效聊天响应应仅按配置重试一次");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, R"({"choices":[{"message":{"content":"{\"inferredDomain\":\"数学\",\"learnerGoal\":\"掌握函数单调性\",\"courseTitle\":\"函数单调性课程\",\"courseSummary\":\"分阶段掌握定义和题型\",\"durationWeeks\":2,\"phases\":[{\"name\":\"概念\",\"durationWeeks\":1,\"objective\":\"理解单调性定义\",\"topics\":[\"增函数\",\"减函数\",\"定义域\"],\"tasks\":[\"完成定义练习\",\"整理错题\"],\"checkpoint\":\"能用定义判断\",\"output\":\"定义题练习报告\",\"commonMistakes\":[\"忽略定义域\"]},{\"name\":\"图像\",\"durationWeeks\":1,\"objective\":\"结合图像判断\",\"topics\":[\"图像趋势\",\"区间\",\"端点\"],\"tasks\":[\"完成图像练习\",\"标注区间\"],\"checkpoint\":\"能读图判断\",\"output\":\"图像题练习报告\",\"commonMistakes\":[\"看错区间\"]},{\"name\":\"综合\",\"durationWeeks\":1,\"objective\":\"解决综合题\",\"topics\":[\"参数题\",\"证明题\",\"应用题\"],\"tasks\":[\"完成综合练习\",\"复盘错题\"],\"checkpoint\":\"能独立解题\",\"output\":\"综合题练习报告\",\"commonMistakes\":[\"步骤不完整\"]}]}"}}],"model":"configured-plan-model"})"));
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server, "/v1", "configured-plan-model");
        // 【逐行批注】 声明对象 generator，并通过构造函数初始化其状态。
        gangyi::PlanGenerator generator(client);
        // 【逐行批注】 计算右侧结果并写入 auto plan，更新当前运行状态。
        const auto plan = generator.generate("掌握函数单调性", "lite");
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(plan.title == "函数单调性课程", "课程标题应兼容 courseTitle 字段");。
        expect(plan.title == "函数单调性课程", "课程标题应兼容 courseTitle 字段");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(plan.goal == "掌握函数单调性", "学习目标应兼容 learnerGoal 字段");。
        expect(plan.goal == "掌握函数单调性", "学习目标应兼容 learnerGoal 字段");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(plan.phases.size() == 3, "快速规划应保留三个阶段");。
        expect(plan.phases.size() == 3, "快速规划应保留三个阶段");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(plan.searchStatus != "not_requested", "课程规划前必须取得检索状态");。
        expect(plan.searchStatus != "not_requested", "课程规划前必须取得检索状态");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("\"max_tokens\":4500") != std::string::n…。
        expect(server.request().find("\"max_tokens\":4500") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："快速规划必须预留完整 JSON 输出空间");。
            "快速规划必须预留完整 JSON 输出空间");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("phases 必须恰好 3 项") != std::string::npos,。
        expect(server.request().find("phases 必须恰好 3 项") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："课程规划提示词必须明确阶段精确数量");。
            "课程规划提示词必须明确阶段精确数量");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(200, R"({"code":200,"data":{}})"));
        // 【逐行批注】 计算右侧结果并写入 std::string url，更新当前运行状态。
        const std::string url = "http://127.0.0.1:" + std::to_string(server.port());
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
        // 【逐行批注】 调用 _putenv_s 完成本行所需操作：_putenv_s("BOCHA_BASE_URL", url.c_str());。
        _putenv_s("BOCHA_BASE_URL", url.c_str());
// 【逐行批注】 处理条件编译指令：#else。
#else
        // 【逐行批注】 调用 setenv 完成本行所需操作：setenv("BOCHA_BASE_URL", url.c_str(), 1);。
        setenv("BOCHA_BASE_URL", url.c_str(), 1);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
        // 【逐行批注】 执行当前声明或表达式：std::string diagnostic;。
        std::string diagnostic;
        // 【逐行批注】 调用 expect、testBochaKey 完成本行所需操作：expect(gangyi::SearchClient::testBochaKey(" test-key ", &diagnostic) …。
        expect(gangyi::SearchClient::testBochaKey(" test-key ", &diagnostic) && diagnostic == "ok",
            // 【逐行批注】 执行当前声明或表达式："博查连接测试应识别成功响应并修剪 Key 空白");。
            "博查连接测试应识别成功响应并修剪 Key 空白");
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.request().find("Bearer test-key") != std::string::npos,。
        expect(server.request().find("Bearer test-key") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："博查请求头不得携带粘贴时的空白");。
            "博查请求头不得携带粘贴时的空白");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(response(401, R"({"code":401})"));
        // 【逐行批注】 计算右侧结果并写入 std::string url，更新当前运行状态。
        const std::string url = "http://127.0.0.1:" + std::to_string(server.port());
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
        // 【逐行批注】 调用 _putenv_s 完成本行所需操作：_putenv_s("BOCHA_BASE_URL", url.c_str());。
        _putenv_s("BOCHA_BASE_URL", url.c_str());
// 【逐行批注】 处理条件编译指令：#else。
#else
        // 【逐行批注】 调用 setenv 完成本行所需操作：setenv("BOCHA_BASE_URL", url.c_str(), 1);。
        setenv("BOCHA_BASE_URL", url.c_str(), 1);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
        // 【逐行批注】 执行当前声明或表达式：std::string diagnostic;。
        std::string diagnostic;
        // 【逐行批注】 调用 expect、testBochaKey 完成本行所需操作：expect(!gangyi::SearchClient::testBochaKey("test-key", &diagnostic) &…。
        expect(!gangyi::SearchClient::testBochaKey("test-key", &diagnostic) && diagnostic == "http:401",
            // 【逐行批注】 执行当前声明或表达式："博查连接测试应报告认证状态码");。
            "博查连接测试应报告认证状态码");
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 _putenv_s 完成本行所需操作：_putenv_s("BOCHA_BASE_URL", "");。
    _putenv_s("BOCHA_BASE_URL", "");
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 unsetenv 完成本行所需操作：unsetenv("BOCHA_BASE_URL");。
    unsetenv("BOCHA_BASE_URL");
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 开始当前代码块。
    {
        // 两个独立入口并发，暂停等待首字的请求，恢复后两项都继续。
        // 【逐行批注】 计算右侧结果并写入 auto folder，更新当前运行状态。
        const auto folder = std::filesystem::temp_directory_path() / ("gangyi-ai-monitor-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        // 【逐行批注】 计算右侧结果并写入 auto record，更新当前运行状态。
        const auto record = folder / "activity.json";
        // 【逐行批注】 调用 configure 完成本行所需操作：gangyi::AIActivity::configure(record.u8string());。
        gangyi::AIActivity::configure(record.u8string());
        // 【逐行批注】 计算右侧结果并写入 auto payload，更新当前运行状态。
        const auto payload = response(200, R"({"choices":[{"message":{"content":"已恢复"}}]})");
        // 【逐行批注】 声明对象 first，并通过构造函数初始化其状态。
        MockServer first(std::vector<std::string>{payload, payload}, 0, true), second(std::vector<std::string>{payload, payload}, 0, true);
        // 【逐行批注】 执行当前声明或表达式：std::exception_ptr firstError, secondError;。
        std::exception_ptr firstError, secondError;
        // 【逐行批注】 声明对象 one，并通过构造函数初始化其状态。
        std::thread one([&] { try {
            // 【逐行批注】 计算右侧结果并写入 gangyi::ChatOptions options; options.messages，更新当前运行状态。
            gangyi::ChatOptions options; options.messages = {{"user", "PRIVATE-PROMPT-ONE"}}; options.activity.source = "目标图片"; options.activity.purpose = "识别学习目标";
            // 【逐行批注】 调用 clientFor 完成本行所需操作：clientFor(first).chat(options);。
            clientFor(first).chat(options);
        // 【逐行批注】 计算右侧结果并写入 } catch (...) { firstError，更新当前运行状态。
        } catch (...) { firstError = std::current_exception(); } });
        // 【逐行批注】 声明对象 two，并通过构造函数初始化其状态。
        std::thread two([&] { try {
            // 【逐行批注】 计算右侧结果并写入 gangyi::ChatOptions options; options.messages，更新当前运行状态。
            gangyi::ChatOptions options; options.messages = {{"user", "PRIVATE-PROMPT-TWO"}}; options.activity.source = "AI 设置"; options.activity.purpose = "测试连接";
            // 【逐行批注】 调用 clientFor 完成本行所需操作：clientFor(second).chat(options);。
            clientFor(second).chat(options);
        // 【逐行批注】 计算右侧结果并写入 } catch (...) { secondError，更新当前运行状态。
        } catch (...) { secondError = std::current_exception(); } });
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(first.waitRequests(1) && second.waitRequests(1), "两个独立入口应同时发起请…。
        expect(first.waitRequests(1) && second.waitRequests(1), "两个独立入口应同时发起请求");
        // 【逐行批注】 计算右侧结果并写入 auto before，更新当前运行状态。
        const auto before = gangyi::AIActivity::snapshot();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(before.at("tasks").size() == 2 && before.at("todayCalls") == 2…。
        expect(before.at("tasks").size() == 2 && before.at("todayCalls") == 2, "实际请求次数应包含所有并发入口");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(before.dump().find("PRIVATE-PROMPT") == std::string::npos, "监控…。
        expect(before.dump().find("PRIVATE-PROMPT") == std::string::npos, "监控不得保存提示词或返回正文");
        // 【逐行批注】 调用 setPaused 完成本行所需操作：gangyi::AIActivity::setPaused(true);。
        gangyi::AIActivity::setPaused(true);
        // 【逐行批注】 执行当前声明或表达式：first.release(); second.release();。
        first.release(); second.release();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(!first.waitRequests(2, 350) && !second.waitRequests(2, 350), "…。
        expect(!first.waitRequests(2, 350) && !second.waitRequests(2, 350), "暂停后独立调用不得自动重新发起请求");
        // 【逐行批注】 计算右侧结果并写入 auto paused，更新当前运行状态。
        const auto paused = gangyi::AIActivity::snapshot();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(paused.at("tasks").at(0).at("status") == "paused" && paused.at…。
        expect(paused.at("tasks").at(0).at("status") == "paused" && paused.at("tasks").at(1).at("status") == "paused", "所有并发请求应显示暂停");
        // 【逐行批注】 调用 setPaused 完成本行所需操作：gangyi::AIActivity::setPaused(false);。
        gangyi::AIActivity::setPaused(false);
        // 【逐行批注】 执行当前声明或表达式：one.join(); two.join(); first.wait(); second.wait();。
        one.join(); two.join(); first.wait(); second.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(!firstError && !secondError, "恢复全部应继续所有独立调用");。
        expect(!firstError && !secondError, "恢复全部应继续所有独立调用");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(first.requestCount() == 2 && second.requestCount() == 2, "恢复应仅…。
        expect(first.requestCount() == 2 && second.requestCount() == 2, "恢复应仅重发各自未完成的请求");
        // 【逐行批注】 调用 expect、snapshot 完成本行所需操作：expect(gangyi::AIActivity::snapshot().at("todayCalls") == 4, "暂停前请求与恢…。
        expect(gangyi::AIActivity::snapshot().at("todayCalls") == 4, "暂停前请求与恢复请求应各计一次");
        // 【逐行批注】 调用 setPaused 完成本行所需操作：gangyi::AIActivity::setPaused(true);。
        gangyi::AIActivity::setPaused(true);
        // 【逐行批注】 调用 configure 完成本行所需操作：gangyi::AIActivity::configure(record.u8string());。
        gangyi::AIActivity::configure(record.u8string());
        // 【逐行批注】 调用 expect、paused 完成本行所需操作：expect(gangyi::AIActivity::paused(), "重新读取控制记录必须保留暂停开关");。
        expect(gangyi::AIActivity::paused(), "重新读取控制记录必须保留暂停开关");
        // 【逐行批注】 调用 setPaused、configure 完成本行所需操作：gangyi::AIActivity::setPaused(false); gangyi::AIActivity::configure("…。
        gangyi::AIActivity::setPaused(false); gangyi::AIActivity::configure("");
        // 【逐行批注】 调用 remove_all 完成本行所需操作：std::filesystem::remove_all(folder);。
        std::filesystem::remove_all(folder);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 流式主控在暂停时退出本轮网络调用，由持久任务恢复，不能把暂停计为失败。
        // 【逐行批注】 计算右侧结果并写入 std::string body，更新当前运行状态。
        const std::string body = "data: {\"model\":\"fixture\",\"choices\":[{\"delta\":{\"content\":\"OK\"},\"finish_reason\":\"stop\"}]}\n\ndata: [DONE]\n\n";
        // 【逐行批注】 计算右侧结果并写入 auto payload，更新当前运行状态。
        const auto payload = response(200, body);
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(std::vector<std::string>{payload, payload}, 0, true);
        // 【逐行批注】 计算右侧结果并写入 gangyi::ChatOptions options; options.messages，更新当前运行状态。
        gangyi::ChatOptions options; options.messages = {{"user", "流式暂停测试"}}; options.activity.taskId = "stream-pause-test";
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server); std::promise<std::string> outcome; auto done = outcome.get_future();
        // 【逐行批注】 声明对象 worker，并通过构造函数初始化其状态。
        std::thread worker([&] {
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try { client.chatStream(options, [](const std::string&) { return true; }); outcome.set_value("completed"); }
            // 【逐行批注】 捕获并处理异常：catch (const gangyi::AIClientError& error) { outcome.set_value(error.…。
            catch (const gangyi::AIClientError& error) { outcome.set_value(error.errorType); }
            // 【逐行批注】 捕获并处理异常：catch (...) { outcome.set_value("unexpected"); }。
            catch (...) { outcome.set_value("unexpected"); }
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.waitRequests(1), "流式主控应开始真实网络请求");。
        expect(server.waitRequests(1), "流式主控应开始真实网络请求");
        // 【逐行批注】 调用 setPaused 完成本行所需操作：gangyi::AIActivity::setPaused(true);。
        gangyi::AIActivity::setPaused(true);
        // 【逐行批注】 计算右侧结果并写入 auto stopped，更新当前运行状态。
        const auto stopped = done.wait_for(std::chrono::milliseconds(1500));
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(stopped == std::future_status::ready, "等待首字时应立即中断流式请求");。
        expect(stopped == std::future_status::ready, "等待首字时应立即中断流式请求");
        // 【逐行批注】 调用 setPaused 完成本行所需操作：gangyi::AIActivity::setPaused(false); server.release(); worker.join();。
        gangyi::AIActivity::setPaused(false); server.release(); worker.join();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(done.get() == "paused", "全局暂停不能变成网络失败或伪造成功");。
        expect(done.get() == "paused", "全局暂停不能变成网络失败或伪造成功");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(client.chatStream(options, [](const std::string&) { return tru…。
        expect(client.chatStream(options, [](const std::string&) { return true; }).content == "OK", "恢复后可以重新执行未完成的流式请求");
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 计算右侧结果并写入 auto view，更新当前运行状态。
        const auto view = gangyi::AIActivity::snapshot();
        // 【逐行批注】 按循环规则重复处理：for (const auto& task : view.at("tasks")) if (task.at("id") == "stream-pau…。
        for (const auto& task : view.at("tasks")) if (task.at("id") == "stream-pause-test")
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(task.at("calls") == 2 && task.at("status") == "completed", "流式…。
            expect(task.at("calls") == 2 && task.at("status") == "completed", "流式请求的中断及恢复应准确累计");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 runTransportDiagnosticChecks 完成本行所需操作：runTransportDiagnosticChecks();。
    runTransportDiagnosticChecks();
    // 【逐行批注】 调用 runStreamIntegrityChecks 完成本行所需操作：runStreamIntegrityChecks();。
    runStreamIntegrityChecks();
    // 【逐行批注】 调用 runRetryCoordinationChecks 完成本行所需操作：runRetryCoordinationChecks();。
    runRetryCoordinationChecks();
    // 【逐行批注】 判断条件 (std::getenv("LIVE_BOCHA_PROBE")) {，成立时执行对应分支。
    if (std::getenv("LIVE_BOCHA_PROBE")) {
        // 【逐行批注】 执行当前声明或表达式：std::string diagnostic;。
        std::string diagnostic;
        // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
        const bool valid = gangyi::SearchClient::testBochaKey("invalid-diagnostic-key", &diagnostic);
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(!valid && diagnostic == "http:401", "代理故障时博查应直连并返回 HTTP 401");。
        expect(!valid && diagnostic == "http:401", "代理故障时博查应直连并返回 HTTP 401");
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 WSACleanup 完成本行所需操作：WSACleanup();。
    WSACleanup();
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 结束当前函数并返回结果：return failures == 0 ? 0 : 1;。
    return failures == 0 ? 0 : 1;
// 【逐行批注】 结束当前代码块。
}

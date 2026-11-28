// 【逐行批注】 引入头文件 "learning_generator.hpp"，为后续实现提供类型或接口声明。
#include "learning_generator.hpp"
// 【逐行批注】 引入头文件 "phase_generator.hpp"，为后续实现提供类型或接口声明。
#include "phase_generator.hpp"
// 【逐行批注】 引入头文件 "text_utils.hpp"，为后续实现提供类型或接口声明。
#include "text_utils.hpp"

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
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <cstring>，为后续实现提供类型或接口声明。
#include <cstring>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>

// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 int failures，更新当前运行状态。
int failures = 0;

// 【逐行批注】 定义函数 expect，执行该函数声明的职责。
void expect(bool condition, const char* message) {
    // 【逐行批注】 判断条件 (!condition) { std::cerr << "失败: " << message << '\n'; ++failure…，成立时执行对应分支。
    if (!condition) { std::cerr << "失败: " << message << '\n'; ++failures; }
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
    // 【逐行批注】 调用 MockServer、expected_ 完成本行所需操作：MockServer(std::string body, int expected) : expected_(expected) {。
    MockServer(std::string body, int expected) : expected_(expected) {
        // 【逐行批注】 计算右侧结果并写入 response_，更新当前运行状态。
        response_ = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " +
            // 【逐行批注】 调用 to_string 完成本行所需操作：std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;。
            std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
        // 【逐行批注】 计算右侧结果并写入 server_，更新当前运行状态。
        server_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // 【逐行批注】 执行当前声明或表达式：sockaddr_in address{};。
        sockaddr_in address{};
        // 【逐行批注】 计算右侧结果并写入 address.sin_family，更新当前运行状态。
        address.sin_family = AF_INET;
        // 【逐行批注】 计算右侧结果并写入 address.sin_addr.s_addr，更新当前运行状态。
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        // 【逐行批注】 计算右侧结果并写入 address.sin_port，更新当前运行状态。
        address.sin_port = 0;
        // 【逐行批注】 判断条件 (server_ == kInvalidSocket || bind(server_, reinterpret_cast<soc…，成立时执行对应分支。
        if (server_ == kInvalidSocket || bind(server_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
            // 【逐行批注】 调用 listen、runtime_error 完成本行所需操作：listen(server_, expected) != 0) throw std::runtime_error("无法启动模拟 AI 服…。
            listen(server_, expected) != 0) throw std::runtime_error("无法启动模拟 AI 服务");
        // 【逐行批注】 计算右侧结果并写入 SocketLength length，更新当前运行状态。
        SocketLength length = sizeof(address);
        // 【逐行批注】 调用 getsockname 完成本行所需操作：getsockname(server_, reinterpret_cast<sockaddr*>(&address), &length);。
        getsockname(server_, reinterpret_cast<sockaddr*>(&address), &length);
        // 【逐行批注】 计算右侧结果并写入 port_，更新当前运行状态。
        port_ = ntohs(address.sin_port);
        // 【逐行批注】 计算右侧结果并写入 worker_，更新当前运行状态。
        worker_ = std::thread([this] { serve(); });
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 调用 MockServer、closeSocket 完成本行所需操作：~MockServer() { if (worker_.joinable()) worker_.join(); closeSocket(s…。
    ~MockServer() { if (worker_.joinable()) worker_.join(); closeSocket(server_); }
    // 【逐行批注】 定义函数 port，执行该函数声明的职责。
    int port() const { return port_; }
    // 【逐行批注】 定义函数 count，执行该函数声明的职责。
    int count() const { return count_; }
    // 【逐行批注】 定义函数 requests，执行该函数声明的职责。
    const std::string& requests() const { return requests_; }
    // 【逐行批注】 定义函数 wait，执行该函数声明的职责。
    void wait() { if (worker_.joinable()) worker_.join(); }

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 定义函数 serve，执行该函数声明的职责。
    void serve() {
        // 【逐行批注】 按循环规则重复处理：for (int index = 0; index < expected_; ++index) {。
        for (int index = 0; index < expected_; ++index) {
            // 【逐行批注】 执行当前声明或表达式：sockaddr_in clientAddress{};。
            sockaddr_in clientAddress{};
            // 【逐行批注】 计算右侧结果并写入 SocketLength length，更新当前运行状态。
            SocketLength length = sizeof(clientAddress);
            // 【逐行批注】 计算右侧结果并写入 Socket client，更新当前运行状态。
            Socket client = accept(server_, reinterpret_cast<sockaddr*>(&clientAddress), &length);
            // 【逐行批注】 判断条件 (client == kInvalidSocket) return;，成立时执行对应分支。
            if (client == kInvalidSocket) return;
            // 【逐行批注】 执行当前声明或表达式：std::string request;。
            std::string request;
            // 【逐行批注】 执行当前声明或表达式：char buffer[4096];。
            char buffer[4096];
            // 必须按 Content-Length 读完请求体：只读到空行会漏掉整个 JSON，断言会随分片时序随机失败。
            // 【逐行批注】 计算右侧结果并写入 size_t contentLength，更新当前运行状态。
            size_t contentLength = std::string::npos;
            // 【逐行批注】 在条件成立期间循环处理：while (true) {。
            while (true) {
                // 【逐行批注】 计算右侧结果并写入 size_t headerEnd，更新当前运行状态。
                const size_t headerEnd = request.find("\r\n\r\n");
                // 【逐行批注】 判断条件 (headerEnd != std::string::npos) {，成立时执行对应分支。
                if (headerEnd != std::string::npos) {
                    // 【逐行批注】 判断条件 (contentLength == std::string::npos) {，成立时执行对应分支。
                    if (contentLength == std::string::npos) {
                        // 【逐行批注】 计算右侧结果并写入 std::string headers，更新当前运行状态。
                        const std::string headers = request.substr(0, headerEnd + 4);
                        // 【逐行批注】 按循环规则重复处理：for (const char* name : {"Content-Length:", "content-length:"}) {。
                        for (const char* name : {"Content-Length:", "content-length:"}) {
                            // 【逐行批注】 计算右侧结果并写入 size_t at，更新当前运行状态。
                            const size_t at = headers.find(name);
                            // 【逐行批注】 判断条件 (at == std::string::npos) continue;，成立时执行对应分支。
                            if (at == std::string::npos) continue;
                            // 【逐行批注】 计算右侧结果并写入 contentLength，更新当前运行状态。
                            contentLength = static_cast<size_t>(std::strtoul(headers.c_str() + at + std::strlen(name), nullptr, 10));
                            // 【逐行批注】 跳出当前循环或分支。
                            break;
                        // 【逐行批注】 结束当前代码块。
                        }
                    // 【逐行批注】 结束当前代码块。
                    }
                    // 【逐行批注】 判断条件 (contentLength != std::string::npos && request.size() >= headerE…，成立时执行对应分支。
                    if (contentLength != std::string::npos && request.size() >= headerEnd + 4 + contentLength) break;
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 计算右侧结果并写入 int received，更新当前运行状态。
                const int received = recv(client, buffer, sizeof(buffer), 0);
                // 【逐行批注】 判断条件 (received <= 0) break;，成立时执行对应分支。
                if (received <= 0) break;
                // 【逐行批注】 执行当前声明或表达式：request.append(buffer, static_cast<size_t>(received));。
                request.append(buffer, static_cast<size_t>(received));
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 requests_ +，更新当前运行状态。
            requests_ += request;
            // 【逐行批注】 执行当前声明或表达式：++count_;。
            ++count_;
            // 【逐行批注】 调用 send 完成本行所需操作：send(client, response_.data(), static_cast<int>(response_.size()), 0);。
            send(client, response_.data(), static_cast<int>(response_.size()), 0);
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
    // 【逐行批注】 计算右侧结果并写入 int expected_，更新当前运行状态。
    int expected_ = 0;
    // 【逐行批注】 执行当前声明或表达式：std::string response_;。
    std::string response_;
    // 【逐行批注】 执行当前声明或表达式：std::thread worker_;。
    std::thread worker_;
    // 【逐行批注】 执行当前声明或表达式：std::atomic<int> count_{0};。
    std::atomic<int> count_{0};
    // 【逐行批注】 执行当前声明或表达式：std::string requests_;。
    std::string requests_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 providerResponse，执行该函数声明的职责。
std::string providerResponse(const nlohmann::json& content) {
    // 【逐行批注】 结束当前函数并返回结果：return nlohmann::json{{"model", "configured-model"},。
    return nlohmann::json{{"model", "configured-model"},
        // 【逐行批注】 调用 array 完成本行所需操作：{"choices", nlohmann::json::array({{{"message", {{"content", content.…。
        {"choices", nlohmann::json::array({{{"message", {{"content", content.dump()}}}}})}}.dump();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 clientFor，执行该函数声明的职责。
gangyi::AIClient clientFor(const MockServer& server) {
    // 【逐行批注】 结束当前函数并返回结果：return gangyi::AIClient({"http://127.0.0.1:" + std::to_string(server.…。
    return gangyi::AIClient({"http://127.0.0.1:" + std::to_string(server.port()) + "/v1",
        // 【逐行批注】 执行当前声明或表达式："test-key", "configured-model", 2000, 1});。
        "test-key", "configured-model", 2000, 1});
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 执行当前声明或表达式：WSADATA data{};。
    WSADATA data{};
    // 【逐行批注】 判断条件 (WSAStartup(MAKEWORD(2, 2), &data) != 0) return 2;，成立时执行对应分支。
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return 2;
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 计算右侧结果并写入 std::string utf8，更新当前运行状态。
    const std::string utf8 = u8"abc中文";
    // 【逐行批注】 调用 expect、truncateUtf8 完成本行所需操作：expect(gangyi::truncateUtf8(utf8, 3) == "abc", "ASCII 截断必须保留完整字符");。
    expect(gangyi::truncateUtf8(utf8, 3) == "abc", "ASCII 截断必须保留完整字符");
    // 【逐行批注】 调用 expect、truncateUtf8 完成本行所需操作：expect(gangyi::truncateUtf8(utf8, 4) == "abc", "不得保留半个中文字符");。
    expect(gangyi::truncateUtf8(utf8, 4) == "abc", "不得保留半个中文字符");
    // 【逐行批注】 调用 expect、truncateUtf8 完成本行所需操作：expect(gangyi::truncateUtf8(utf8, 6) == u8"abc中", "中文结束边界必须保留完整字符");。
    expect(gangyi::truncateUtf8(utf8, 6) == u8"abc中", "中文结束边界必须保留完整字符");
    // 【逐行批注】 调用 expect、truncateUtf8 完成本行所需操作：expect(gangyi::truncateUtf8(utf8, 9) == utf8, "长度上限足够时必须保留完整内容");。
    expect(gangyi::truncateUtf8(utf8, 9) == utf8, "长度上限足够时必须保留完整内容");
    // 【逐行批注】 计算右侧结果并写入 std::string incomplete，更新当前运行状态。
    const std::string incomplete = std::string("abc") + static_cast<char>(0xE4) + static_cast<char>(0xB8);
    // 【逐行批注】 计算右侧结果并写入 std::string validPrefix，更新当前运行状态。
    const std::string validPrefix = gangyi::truncateUtf8(incomplete, incomplete.size());
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(validPrefix == "abc", "不完整 UTF-8 末尾必须返回已确认合法的前缀");。
    expect(validPrefix == "abc", "不完整 UTF-8 末尾必须返回已确认合法的前缀");
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 std::string dumped，更新当前运行状态。
        const std::string dumped = nlohmann::json{{"value", validPrefix}}.dump();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(!dumped.empty(), "安全截断结果必须可执行 JSON dump");。
        expect(!dumped.empty(), "安全截断结果必须可执行 JSON dump");
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(false, "安全截断结果不得导致 JSON dump 异常");。
        expect(false, "安全截断结果不得导致 JSON dump 异常");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json plan，更新当前运行状态。
    const nlohmann::json plan = {{"title", "氧化还原反应课程"}, {"generation", {{"source", "ai"}}}};
    // 【逐行批注】 计算右侧结果并写入 std::string goal，更新当前运行状态。
    const std::string goal = "高中化学氧化还原反应";
    // 【逐行批注】 计算右侧结果并写入 std::string phase，更新当前运行状态。
    const std::string phase = "氧化还原基础";
    // 【逐行批注】 计算右侧结果并写入 std::string topic，更新当前运行状态。
    const std::string topic = "氧化数变化与电子转移";
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json content，更新当前运行状态。
        const nlohmann::json content = {{"title", topic + "入门"},
            // 【逐行批注】 执行当前声明或表达式：{"summary", "围绕" + goal + "，在" + phase + "阶段通过" + topic + "判断氧化还原反应"}, {"inferredDom…。
            {"summary", "围绕" + goal + "，在" + phase + "阶段通过" + topic + "判断氧化还原反应"}, {"inferredDomain", "高中化学"},
            // 【逐行批注】 执行当前声明或表达式：{"keyConcepts", {topic, "氧化剂", "还原剂"}}};。
            {"keyConcepts", {topic, "氧化剂", "还原剂"}}};
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(providerResponse(content), 1);
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server);
        // 【逐行批注】 声明对象 generator，并通过构造函数初始化其状态。
        gangyi::LearningGenerator generator(client);
        // 【逐行批注】 执行当前声明或表达式：gangyi::SearchResource resource;。
        gangyi::SearchResource resource;
        // 【逐行批注】 计算右侧结果并写入 resource.description，更新当前运行状态。
        resource.description = std::string(299, 'a') + "中";
        // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
        const auto result = generator.generateBlock(goal, plan, phase,
            // 【逐行批注】 调用 object 完成本行所需操作：topic, 1, "deep", "overview", nlohmann::json::object(), {resource});。
            topic, 1, "deep", "overview", nlohmann::json::object(), {resource});
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result["_generation"].value("source", "") == "ai", "成功板块必须记录 A…。
        expect(result["_generation"].value("source", "") == "ai", "成功板块必须记录 AI 来源");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result["_generation"].value("searchStatus", "not_requested") !…。
        expect(result["_generation"].value("searchStatus", "not_requested") != "not_requested",
            // 【逐行批注】 执行当前声明或表达式："微课生成前必须获得检索状态");。
            "微课生成前必须获得检索状态");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(result["_generation"].value("model", "") == "configured-model"…。
        expect(result["_generation"].value("model", "") == "configured-model", "必须记录实际返回模型");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("configured-model") != std::string::npo…。
        expect(server.requests().find("configured-model") != std::string::npos, "必须使用配置的模型");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("deepseek-v4-flash") == std::string::np…。
        expect(server.requests().find("deepseek-v4-flash") == std::string::npos, "不得硬模型");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("coursePlan") != std::string::npos, "请求…。
        expect(server.requests().find("coursePlan") != std::string::npos, "请求必须携带已保存课程主线");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("previousBlocks") != std::string::npos,…。
        expect(server.requests().find("previousBlocks") != std::string::npos, "请求必须携带已生成前置板块");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("原样出现输入中的 goal、phase、topic") != std::st…。
        expect(server.requests().find("原样出现输入中的 goal、phase、topic") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："提示词必须与目标、阶段、主题的质量门禁一致");。
            "提示词必须与目标、阶段、主题的质量门禁一致");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("禁止使用反斜杠或 LaTeX 命令") != std::string::np…。
        expect(server.requests().find("禁止使用反斜杠或 LaTeX 命令") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："数学内容必须避免生成破坏 JSON 的 LaTeX 反斜杠");。
            "数学内容必须避免生成破坏 JSON 的 LaTeX 反斜杠");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("\"max_tokens\":8192") != std::string::…。
        expect(server.requests().find("\"max_tokens\":8192") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："深度板块必须预留完整 JSON 输出空间");。
            "深度板块必须预留完整 JSON 输出空间");
        // 【逐行批注】 调用 expect、string 完成本行所需操作：expect(server.requests().find(std::string(299, 'a')) != std::string::…。
        expect(server.requests().find(std::string(299, 'a')) != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："资源摘要截断不得破坏 UTF-8，且必须继续调用 AI");。
            "资源摘要截断不得破坏 UTF-8，且必须继续调用 AI");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(providerResponse({{"title", topic}}), 3);
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server);
        // 【逐行批注】 声明对象 generator，并通过构造函数初始化其状态。
        gangyi::LearningGenerator generator(client);
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：generator.generateBlock(goal, plan, phase,。
            generator.generateBlock(goal, plan, phase,
                // 【逐行批注】 调用 object 完成本行所需操作：topic, 1, "deep", "overview", nlohmann::json::object(), {});。
                topic, 1, "deep", "overview", nlohmann::json::object(), {});
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(false, "不完整 AI 输出不得成为课程内容");。
            expect(false, "不完整 AI 输出不得成为课程内容");
        // 【逐行批注】 执行当前声明或表达式：} catch (const gangyi::AIClientError& error) {。
        } catch (const gangyi::AIClientError& error) {
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(error.errorType == "quality_rejected", "三次质量失败应返回 quality_reje…。
            expect(error.errorType == "quality_rejected", "三次质量失败应返回 quality_rejected");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.count() == 3, "板块生成失败必须自动尝试三次");。
        expect(server.count() == 3, "板块生成失败必须自动尝试三次");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 计算右侧结果并写入 nlohmann::json overview，更新当前运行状态。
        const nlohmann::json overview = {{"title", topic}, {"summary", topic + "：先比较反应前后化合价，再说明电子得失。"},
            // 【逐行批注】 调用 array 完成本行所需操作：{"inferredDomain", "化学"}, {"keyConcepts", nlohmann::json::array({"化合价…。
            {"inferredDomain", "化学"}, {"keyConcepts", nlohmann::json::array({"化合价", "升降", "电子得失"})}};
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(providerResponse({{"overview", overview}}), 1);
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server); gangyi::LearningGenerator generator(client);
        // 【逐行批注】 计算右侧结果并写入 auto adapted，更新当前运行状态。
        const auto adapted = generator.adaptBlocks(goal, plan, phase, topic, {"overview"}, nlohmann::json::object(), {});
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(adapted["overview"]["_generation"]["promptVersion"] == "ai-ada…。
        expect(adapted["overview"]["_generation"]["promptVersion"] == "ai-adaptation-v1", "重备必须来自真实模型请求");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(adapted["overview"]["title"] == topic && server.count() == 1, …。
        expect(adapted["overview"]["title"] == topic && server.count() == 1, "重备围绕原主题，单次请求无需重复完整课程名");

        auto broken = nlohmann::json{{"overview", overview}}.dump();
        broken.pop_back();
        const nlohmann::json brokenResponse = {{"model", "configured-model"},
            {"choices", nlohmann::json::array({{{"message", {{"content", broken}}}}})}};
        MockServer brokenServer(brokenResponse.dump(), 1);
        auto brokenClient = clientFor(brokenServer);
        gangyi::LearningGenerator brokenGenerator(brokenClient);
        try {
            brokenGenerator.adaptBlocks(goal, plan, phase, topic, {"overview"}, nlohmann::json::object(), {});
            expect(false, "已读到完整板块但回复被截断时，不得将该板块交给保存流程");
        } catch (const gangyi::AIClientError& error) {
            expect(error.errorType == "json_parse_error", "截断回复应保留结构错误分类");
        }
        brokenServer.wait();
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(providerResponse({{"overview", {{"title", "无关课程"}, {"summary", "无关的知识内容"},
            // 【逐行批注】 调用 array 完成本行所需操作：{"inferredDomain", "其他"}, {"keyConcepts", nlohmann::json::array({"一",…。
            {"inferredDomain", "其他"}, {"keyConcepts", nlohmann::json::array({"一", "二", "三"})}}}}), 1);
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server); gangyi::LearningGenerator generator(client);
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { generator.adaptBlocks(goal, plan, phase, topic, {"overview"}, nlohmann::json::object(), {});
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(false, "离开当前主题的备课不得保存");。
            expect(false, "离开当前主题的备课不得保存");
        // 【逐行批注】 调用 expect 完成本行所需操作：} catch (const gangyi::AIClientError& error) { expect(error.errorType…。
        } catch (const gangyi::AIClientError& error) { expect(error.errorType == "quality_rejected", "无关备课应拒绝且保留旧内容"); }
        // 【逐行批注】 调用 expect 完成本行所需操作：server.wait(); expect(server.count() == 1, "备课失败不得自动循环请求");。
        server.wait(); expect(server.count() == 1, "备课失败不得自动循环请求");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 server，并通过构造函数初始化其状态。
        MockServer server(providerResponse({{"objective", "氧化还原基础"}}), 3);
        // 【逐行批注】 计算右侧结果并写入 client，更新当前运行状态。
        auto client = clientFor(server);
        // 【逐行批注】 声明对象 generator，并通过构造函数初始化其状态。
        gangyi::PhaseGenerator generator(client);
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 执行当前声明或表达式：generator.generate(goal, "deep", 1, phase,。
            generator.generate(goal, "deep", 1, phase,
                // 【逐行批注】 执行当前声明或表达式：{topic, "氧化剂与还原剂", "氧化还原方程式配平"}, {});。
                {topic, "氧化剂与还原剂", "氧化还原方程式配平"}, {});
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(false, "不完整阶段输出不得回退模板");。
            expect(false, "不完整阶段输出不得回退模板");
        // 【逐行批注】 执行当前声明或表达式：} catch (const gangyi::AIClientError& error) {。
        } catch (const gangyi::AIClientError& error) {
            // 【逐行批注】 调用 expect 完成本行所需操作：expect(error.errorType == "quality_rejected", "阶段三次质量失败应返回 quality_re…。
            expect(error.errorType == "quality_rejected", "阶段三次质量失败应返回 quality_rejected");
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：server.wait();。
        server.wait();
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.count() == 3, "阶段生成失败必须自动尝试三次");。
        expect(server.count() == 3, "阶段生成失败必须自动尝试三次");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("联网检索") != std::string::npos ||。
        expect(server.requests().find("联网检索") != std::string::npos ||
            // 【逐行批注】 执行当前声明或表达式：server.requests().find("未获得联网依据") != std::string::npos,。
            server.requests().find("未获得联网依据") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："阶段生成前必须注入检索结果或失败状态");。
            "阶段生成前必须注入检索结果或失败状态");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("禁止省略任何字段") != std::string::npos,。
        expect(server.requests().find("禁止省略任何字段") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："阶段提示词必须禁止截断或省略字段");。
            "阶段提示词必须禁止截断或省略字段");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("所有数组必须使用 JSON 数组") != std::string::npo…。
        expect(server.requests().find("所有数组必须使用 JSON 数组") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："阶段提示词必须明确嵌套数组类型");。
            "阶段提示词必须明确嵌套数组类型");
        // 【逐行批注】 调用 expect 完成本行所需操作：expect(server.requests().find("\"max_tokens\":6000") != std::string::…。
        expect(server.requests().find("\"max_tokens\":6000") != std::string::npos,
            // 【逐行批注】 执行当前声明或表达式："深度阶段必须预留完整 JSON 输出空间");。
            "深度阶段必须预留完整 JSON 输出空间");
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

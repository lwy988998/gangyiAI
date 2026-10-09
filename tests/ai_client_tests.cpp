#include "ai_client.hpp"
#include "ask_generator.hpp"
#include "json_fix.hpp"
#include "plan_generator.hpp"
#include "search_client.hpp"

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
#include <condition_variable>
#include <mutex>
#include <filesystem>
#include <future>
#include <cstdlib>
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

    MockServer(std::vector<std::string> responses, int delayMs = 0, bool holdResponses = false)
        : responses_(std::move(responses)), delayMs_(delayMs), released_(!holdResponses) {
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
        release();
        wait();
        closeSocket(server_);
    }

    void wait() {
        if (worker_.joinable()) worker_.join();
    }
    int port() const { return port_; }
    int requestCount() const { return requestCount_; }
    bool waitRequests(int count, int milliseconds = 2000) {
        std::unique_lock<std::mutex> lock(gateMutex_);
        return gate_.wait_for(lock, std::chrono::milliseconds(milliseconds), [&] { return requestCount_ >= count; });
    }
    void release() { { std::lock_guard<std::mutex> lock(gateMutex_); released_ = true; } gate_.notify_all(); }
    const std::string& request() const { return request_; }

private:
    void serve() {
        for (const auto& responseText : responses_) {
            sockaddr_in clientAddress{};
            SocketLength length = sizeof(clientAddress);
            Socket client = accept(server_, reinterpret_cast<sockaddr*>(&clientAddress), &length);
            if (client == kInvalidSocket) return;
            ++requestCount_;
            gate_.notify_all();
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
            { std::unique_lock<std::mutex> lock(gateMutex_); gate_.wait(lock, [&] { return released_; }); }
            if (delayMs_ > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delayMs_));
            if (!responseText.empty()) send(client, responseText.data(), static_cast<int>(responseText.size()),
#ifdef _WIN32
                0
#else
                MSG_NOSIGNAL
#endif
            );
            closeSocket(client);
        }
    }

    Socket server_ = kInvalidSocket;
    int port_ = 0;
    std::vector<std::string> responses_;
    int delayMs_ = 0;
    std::thread worker_;
    std::mutex gateMutex_;
    std::condition_variable gate_;
    bool released_ = true;
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

int main(int argc, char** argv) {
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
    if (argc == 4 && std::string(argv[1]) == "--monitor-probe") {
        gangyi::AIActivity::configure(argv[2]);
        gangyi::AIClient client({argv[3], "fictional-fixture-key", "fixture", 10000, 1});
        gangyi::ChatOptions options; options.messages = {{"user", "Reply with OK."}}; options.maxTokens = 16; options.maxAttempts = 1;
        options.activity.source = "AI 设置"; options.activity.purpose = "测试 API 地址、密钥和模型是否可用";
        const auto result = client.chat(options);
        return result.content == "OK" ? 0 : 1;
    }
    {
        // 真实模型偶尔漏写嵌套对象的右花括号；补全只能加括号，不能改写已有字段。
        const std::string valid = R"({"message":"先标化合价","actions":[{"id":"a1","tool":"create_lesson","args":{"courseId":"c1","title":"氧化还原"}}],"state":"completed"})";
        const auto missing = valid.find("}}],\"state\"");
        expect(missing != std::string::npos, "测试用例应包含行动对象与行动数组的连续右括号");
        std::string broken = valid;
        if (missing != std::string::npos) broken.erase(missing, 1);
        const auto parsed = gangyi::parseAIJson(broken);
        expect(parsed.value("message", "") == "先标化合价" && parsed.at("actions").size() == 1,
               "缺少一个右花括号时应补齐括号并保留顶层消息与行动");
        expect(parsed.at("actions")[0].at("args").at("title") == "氧化还原",
               "括号补全不得改写行动参数内容");
        expect(parsed.value("state", "") == "completed", "括号补全后应保留状态字段");
    }
    {
        bool rejected = false;
        try { gangyi::parseAIJson("这不是 JSON，也没有任何对象"); } catch (...) { rejected = true; }
        expect(rejected, "完全无法解析的文本必须报错，不能伪造主控结构");
    }
    {
        MockServer server(response(200, R"({"choices":[{"message":{"content":" 好的 "}}]})"));
        auto client = clientFor(server, "/v1", "configured-ask-model");
        gangyi::AskGenerator generator(client);
        const auto answer = generator.generate("请解释函数单调性", "共享画像", {{"user", "最近的问题"}, {"assistant", "最近的回答"}});
        server.wait();
        expect(answer.content == "好的", "问答应返回去除首尾空白的内容");
        expect(answer.model == "configured-ask-model", "响应缺少模型字段时应回退到配置模型");
        expect(server.request().find("POST /v1/chat/completions HTTP/") != std::string::npos,
               "问答应请求 Chat Completions 端点");
        expect(server.request().find("\"model\":\"configured-ask-model\"") != std::string::npos,
               "问答不得覆盖用户配置的模型");
        expect(server.request().find("deepseek-v4-flash") == std::string::npos,
               "问答请求不得包含硬编码模型");
        expect(server.request().find("最近的问题") != std::string::npos &&
               server.request().find("最近的回答") != std::string::npos &&
               server.request().find("共享画像") != std::string::npos, "导师应包含最近对话与共享画像");
    }
    {
        const std::string events = "data: {\"model\":\"mock\",\"choices\":[{\"delta\":{\"content\":\"第一段\"}}]}\n\n"
            "data:{\"model\":\"mock\",\"choices\":[{\"delta\":{\"content\":\"第二段\"}}]}\n\n"
            "data: [DONE]\n\n";
        MockServer server("HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nContent-Length: " +
            std::to_string(events.size()) + "\r\nConnection: close\r\n\r\n" + events);
        auto client = clientFor(server);
        gangyi::ChatOptions options;
        options.messages = {{"user", "测试", ""}};
        options.responseFormat = "json_object";
        std::vector<std::string> chunks;
        const auto result = client.chatStream(options, [&](const std::string& chunk) { chunks.push_back(chunk); return true; });
        server.wait();
        expect(chunks.size() == 2 && chunks[0] == "第一段" && chunks[1] == "第二段", "流式回调应逐段返回内容");
        expect(result.content == "第一段第二段" && result.model == "mock", "流式结果应合并内容和模型");
        expect(server.request().find("\"stream\":true") != std::string::npos, "流式请求应启用 stream 字段");
        expect(server.request().find("\"response_format\":{\"type\":\"json_object\"}") != std::string::npos,
            "流式教学主控应向模型请求 JSON 对象格式");
    }
    {
        const std::string events = "data: {\"model\":\"mock\",\"choices\":[{\"delta\":{\"content\":\"测试\"}}]}\n\ndata: [DONE]\n\n";
        MockServer server("HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nContent-Length: " +
            std::to_string(events.size()) + "\r\nConnection: close\r\n\r\n" + events);
        gangyi::ChatOptions options; options.messages = {{"user", "回调异常测试"}};
        bool caught = false;
        try {
            clientFor(server).chatStream(options, [](const std::string&) -> bool { throw std::runtime_error("回调保存失败"); });
        } catch (const std::runtime_error& error) { caught = std::string(error.what()) == "回调保存失败"; }
        server.wait();
        expect(caught, "流式回调异常应在释放 C 客户端资源后保持原始原因");
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
        expect(plan.searchStatus != "not_requested", "课程规划前必须取得检索状态");
        expect(server.request().find("\"max_tokens\":4500") != std::string::npos,
            "快速规划必须预留完整 JSON 输出空间");
        expect(server.request().find("phases 必须恰好 3 项") != std::string::npos,
            "课程规划提示词必须明确阶段精确数量");
    }
    {
        MockServer server(response(200, R"({"code":200,"data":{}})"));
        const std::string url = "http://127.0.0.1:" + std::to_string(server.port());
#ifdef _WIN32
        _putenv_s("BOCHA_BASE_URL", url.c_str());
#else
        setenv("BOCHA_BASE_URL", url.c_str(), 1);
#endif
        std::string diagnostic;
        expect(gangyi::SearchClient::testBochaKey(" test-key ", &diagnostic) && diagnostic == "ok",
            "博查连接测试应识别成功响应并修剪 Key 空白");
        server.wait();
        expect(server.request().find("Bearer test-key") != std::string::npos,
            "博查请求头不得携带粘贴时的空白");
    }
    {
        MockServer server(response(401, R"({"code":401})"));
        const std::string url = "http://127.0.0.1:" + std::to_string(server.port());
#ifdef _WIN32
        _putenv_s("BOCHA_BASE_URL", url.c_str());
#else
        setenv("BOCHA_BASE_URL", url.c_str(), 1);
#endif
        std::string diagnostic;
        expect(!gangyi::SearchClient::testBochaKey("test-key", &diagnostic) && diagnostic == "http:401",
            "博查连接测试应报告认证状态码");
        server.wait();
    }
#ifdef _WIN32
    _putenv_s("BOCHA_BASE_URL", "");
#else
    unsetenv("BOCHA_BASE_URL");
#endif
    {
        // 两个独立入口并发，暂停等待首字的请求，恢复后两项都继续。
        const auto folder = std::filesystem::temp_directory_path() / ("gangyi-ai-monitor-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        const auto record = folder / "activity.json";
        gangyi::AIActivity::configure(record.u8string());
        const auto payload = response(200, R"({"choices":[{"message":{"content":"已恢复"}}]})");
        MockServer first(std::vector<std::string>{payload, payload}, 0, true), second(std::vector<std::string>{payload, payload}, 0, true);
        std::exception_ptr firstError, secondError;
        std::thread one([&] { try {
            gangyi::ChatOptions options; options.messages = {{"user", "PRIVATE-PROMPT-ONE"}}; options.activity.source = "目标图片"; options.activity.purpose = "识别学习目标";
            clientFor(first).chat(options);
        } catch (...) { firstError = std::current_exception(); } });
        std::thread two([&] { try {
            gangyi::ChatOptions options; options.messages = {{"user", "PRIVATE-PROMPT-TWO"}}; options.activity.source = "AI 设置"; options.activity.purpose = "测试连接";
            clientFor(second).chat(options);
        } catch (...) { secondError = std::current_exception(); } });
        expect(first.waitRequests(1) && second.waitRequests(1), "两个独立入口应同时发起请求");
        const auto before = gangyi::AIActivity::snapshot();
        expect(before.at("tasks").size() == 2 && before.at("todayCalls") == 2, "实际请求次数应包含所有并发入口");
        expect(before.dump().find("PRIVATE-PROMPT") == std::string::npos, "监控不得保存提示词或返回正文");
        gangyi::AIActivity::setPaused(true);
        first.release(); second.release();
        expect(!first.waitRequests(2, 350) && !second.waitRequests(2, 350), "暂停后独立调用不得自动重新发起请求");
        const auto paused = gangyi::AIActivity::snapshot();
        expect(paused.at("tasks").at(0).at("status") == "paused" && paused.at("tasks").at(1).at("status") == "paused", "所有并发请求应显示暂停");
        gangyi::AIActivity::setPaused(false);
        one.join(); two.join(); first.wait(); second.wait();
        expect(!firstError && !secondError, "恢复全部应继续所有独立调用");
        expect(first.requestCount() == 2 && second.requestCount() == 2, "恢复应仅重发各自未完成的请求");
        expect(gangyi::AIActivity::snapshot().at("todayCalls") == 4, "暂停前请求与恢复请求应各计一次");
        gangyi::AIActivity::setPaused(true);
        gangyi::AIActivity::configure(record.u8string());
        expect(gangyi::AIActivity::paused(), "重新读取控制记录必须保留暂停开关");
        gangyi::AIActivity::setPaused(false); gangyi::AIActivity::configure("");
        std::filesystem::remove_all(folder);
    }
    {
        // 流式主控在暂停时退出本轮网络调用，由持久任务恢复，不能把暂停计为失败。
        const std::string body = "data: {\"model\":\"fixture\",\"choices\":[{\"delta\":{\"content\":\"OK\"},\"finish_reason\":\"stop\"}]}\n\ndata: [DONE]\n\n";
        const auto payload = response(200, body);
        MockServer server(std::vector<std::string>{payload, payload}, 0, true);
        gangyi::ChatOptions options; options.messages = {{"user", "流式暂停测试"}}; options.activity.taskId = "stream-pause-test";
        auto client = clientFor(server); std::promise<std::string> outcome; auto done = outcome.get_future();
        std::thread worker([&] {
            try { client.chatStream(options, [](const std::string&) { return true; }); outcome.set_value("completed"); }
            catch (const gangyi::AIClientError& error) { outcome.set_value(error.errorType); }
            catch (...) { outcome.set_value("unexpected"); }
        });
        expect(server.waitRequests(1), "流式主控应开始真实网络请求");
        gangyi::AIActivity::setPaused(true);
        const auto stopped = done.wait_for(std::chrono::milliseconds(1500));
        expect(stopped == std::future_status::ready, "等待首字时应立即中断流式请求");
        gangyi::AIActivity::setPaused(false); server.release(); worker.join();
        expect(done.get() == "paused", "全局暂停不能变成网络失败或伪造成功");
        expect(client.chatStream(options, [](const std::string&) { return true; }).content == "OK", "恢复后可以重新执行未完成的流式请求");
        server.wait();
        const auto view = gangyi::AIActivity::snapshot();
        for (const auto& task : view.at("tasks")) if (task.at("id") == "stream-pause-test")
            expect(task.at("calls") == 2 && task.at("status") == "completed", "流式请求的中断及恢复应准确累计");
    }
    if (std::getenv("LIVE_BOCHA_PROBE")) {
        std::string diagnostic;
        const bool valid = gangyi::SearchClient::testBochaKey("invalid-diagnostic-key", &diagnostic);
        expect(!valid && diagnostic == "http:401", "代理故障时博查应直连并返回 HTTP 401");
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return failures == 0 ? 0 : 1;
}

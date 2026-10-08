#include "crow.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

int main() {
    asio::io_context clientContext;
    crow::tcp::acceptor reservation(clientContext,
        crow::tcp::endpoint(asio::ip::address_v4::loopback(), 0));
    const auto port = reservation.local_endpoint().port();
    reservation.close();
    crow::SimpleApp app;
    app.loglevel(crow::LogLevel::Warning);
    CROW_ROUTE(app, "/deferred")([](const crow::request& request, crow::response& response) {
        // 请求处理栈已经返回，仅由响应回调持有连接时才完成写入。
        auto timer = std::make_shared<asio::steady_timer>(*request.io_context,
            std::chrono::milliseconds(40));
        timer->async_wait([timer, &response](const asio::error_code& error) {
            if (!error) response.end("late-response");
        });
    });
    std::thread server([&] { app.bindaddr("127.0.0.1").port(port).concurrency(2).run(); });
    bool passed = false;
    try {
        for (int index = 0; index < 8; ++index) {
            crow::tcp::socket socket(clientContext);
            asio::error_code error;
            for (int retry = 0; retry < 100; ++retry) {
                socket.connect(crow::tcp::endpoint(asio::ip::address_v4::loopback(), port), error);
                if (!error) break;
                socket.close(); socket = crow::tcp::socket(clientContext);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if (error) throw std::runtime_error("验收服务未启动");
            const std::string request = "GET /deferred HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
            asio::write(socket, asio::buffer(request));
            asio::streambuf received;
            asio::read_until(socket, received, "\r\n\r\n");
            std::istream stream(&received);
            std::string line;
            std::getline(stream, line);
            if (line.find("200") == std::string::npos) throw std::runtime_error("延迟响应状态无效");
            size_t length = 0;
            while (std::getline(stream, line) && line != "\r")
                if (line.rfind("Content-Length: ", 0) == 0) length = std::stoul(line.substr(16));
            if (length != 13) throw std::runtime_error("延迟响应长度无效");
            if (received.size() < length)
                asio::read(socket, received, asio::transfer_exactly(length - received.size()));
            std::string body(length, '\0'); stream.read(body.data(), length);
            if (body != "late-response") throw std::runtime_error("延迟响应正文损坏");
        }
        passed = true;
    } catch (const std::exception& error) {
        std::cerr << "延迟响应连接验收失败：" << error.what() << '\n';
    }
    app.stop(); server.join();
    if (passed) std::cout << "延迟响应持有连接直到发送结束，后续请求正常。\n";
    return passed ? 0 : 1;
}

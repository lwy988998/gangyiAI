// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 <crow.h>，为后续实现提供类型或接口声明。
#include <crow.h>
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 本机原生调用可不带 Origin；浏览器请求必须来自当前服务的同一来源。
// 【逐行批注】 声明数据结构或类型 LocalRequestGuard 及其成员接口。
struct LocalRequestGuard {
    // 【逐行批注】 声明数据结构或类型 context 及其成员接口。
    struct context {};
    // 【逐行批注】 计算右侧结果并写入 int port，更新当前运行状态。
    int port = 39002;

    // 【逐行批注】 定义函数 lower，执行该函数声明的职责。
    static std::string lower(std::string value) {
        // 【逐行批注】 调用 transform 完成本行所需操作：std::transform(value.begin(), value.end(), value.begin(), [](unsigned…。
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char byte) {
            // 【逐行批注】 结束当前函数并返回结果：return static_cast<char>(std::tolower(byte));。
            return static_cast<char>(std::tolower(byte));
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 结束当前函数并返回结果：return value;。
        return value;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 allows，执行该函数声明的职责。
    bool allows(const crow::request& request) const {
        // 【逐行批注】 判断条件 (request.headers.count("Host") != 1 || request.headers.count("Or…，成立时执行对应分支。
        if (request.headers.count("Host") != 1 || request.headers.count("Origin") > 1 ||
            // 【逐行批注】 执行当前声明或表达式：request.headers.count("Sec-Fetch-Site") > 1) return false;。
            request.headers.count("Sec-Fetch-Site") > 1) return false;
        // 【逐行批注】 计算右侧结果并写入 auto host，更新当前运行状态。
        const auto host = lower(request.get_header_value("Host"));
        // 【逐行批注】 计算右侧结果并写入 auto suffix，更新当前运行状态。
        const auto suffix = ":" + std::to_string(port);
        // 【逐行批注】 判断条件 (host != "127.0.0.1" + suffix && host != "localhost" + suffix &&，成立时执行对应分支。
        if (host != "127.0.0.1" + suffix && host != "localhost" + suffix &&
            // 【逐行批注】 执行当前声明或表达式：!(port == 80 && (host == "127.0.0.1" || host == "localhost"))) return false;。
            !(port == 80 && (host == "127.0.0.1" || host == "localhost"))) return false;
        // 【逐行批注】 计算右侧结果并写入 auto canonicalHost，更新当前运行状态。
        const auto canonicalHost = port == 80 && host.size() > 3 && host.substr(host.size() - 3) == ":80"
            // 【逐行批注】 执行当前声明或表达式：? host.substr(0, host.size() - 3) : host;。
            ? host.substr(0, host.size() - 3) : host;
        // 【逐行批注】 计算右侧结果并写入 auto origin，更新当前运行状态。
        const auto origin = lower(request.get_header_value("Origin"));
        // 【逐行批注】 判断条件 (request.headers.count("Origin") && origin != "http://" + canoni…，成立时执行对应分支。
        if (request.headers.count("Origin") && origin != "http://" + canonicalHost &&
            // 【逐行批注】 执行当前声明或表达式：!(port == 80 && origin == "http://" + canonicalHost + ":80")) return false;。
            !(port == 80 && origin == "http://" + canonicalHost + ":80")) return false;
        // 【逐行批注】 计算右侧结果并写入 auto site，更新当前运行状态。
        const auto site = lower(request.get_header_value("Sec-Fetch-Site"));
        // 【逐行批注】 判断条件 (!site.empty() && site != "same-origin" && site != "none") retur…，成立时执行对应分支。
        if (!site.empty() && site != "same-origin" && site != "none") return false;
        // 【逐行批注】 结束当前函数并返回结果：return true;。
        return true;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 before_handle，执行该函数声明的职责。
    void before_handle(crow::request& request, crow::response& response, context&) {
        // Crow 的升级路径不检查中间件终态；所有 WebSocket 在共享 onaccept 再做同一校验。
        // 【逐行批注】 判断条件 ((request.check_version(1, 1) && request.upgrade &&，成立时执行对应分支。
        if ((request.check_version(1, 1) && request.upgrade &&
             // 【逐行批注】 调用 lower、allows 完成本行所需操作：lower(request.get_header_value("Upgrade")) == "websocket") || allows(…。
             lower(request.get_header_value("Upgrade")) == "websocket") || allows(request)) return;
        // 【逐行批注】 计算右侧结果并写入 response.code，更新当前运行状态。
        response.code = 403;
        // 【逐行批注】 计算右侧结果并写入 response.set_header("Content-Type", "applica…，更新当前运行状态。
        response.set_header("Content-Type", "application/json; charset=utf-8");
        // 【逐行批注】 执行当前声明或表达式：response.end(u8"{\"error\":\"请求来源无效，请从本机软件页面操作。\"}");。
        response.end(u8"{\"error\":\"请求来源无效，请从本机软件页面操作。\"}");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 after_handle，执行该函数声明的职责。
    void after_handle(crow::request&, crow::response&, context&) {}
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 结束当前代码块。
}

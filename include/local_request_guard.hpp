#pragma once
#include <crow.h>
#include <algorithm>
#include <cctype>
#include <string>

namespace gangyi {
// 本机原生调用可不带 Origin；浏览器请求必须来自当前服务的同一来源。
struct LocalRequestGuard {
    struct context {};
    int port = 39002;

    static std::string lower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char byte) {
            return static_cast<char>(std::tolower(byte));
        });
        return value;
    }
    bool allows(const crow::request& request) const {
        if (request.headers.count("Host") != 1 || request.headers.count("Origin") > 1 ||
            request.headers.count("Sec-Fetch-Site") > 1) return false;
        const auto host = lower(request.get_header_value("Host"));
        const auto suffix = ":" + std::to_string(port);
        if (host != "127.0.0.1" + suffix && host != "localhost" + suffix &&
            !(port == 80 && (host == "127.0.0.1" || host == "localhost"))) return false;
        const auto canonicalHost = port == 80 && host.size() > 3 && host.substr(host.size() - 3) == ":80"
            ? host.substr(0, host.size() - 3) : host;
        const auto origin = lower(request.get_header_value("Origin"));
        if (request.headers.count("Origin") && origin != "http://" + canonicalHost &&
            !(port == 80 && origin == "http://" + canonicalHost + ":80")) return false;
        const auto site = lower(request.get_header_value("Sec-Fetch-Site"));
        if (!site.empty() && site != "same-origin" && site != "none") return false;
        return true;
    }
    void before_handle(crow::request& request, crow::response& response, context&) {
        // Crow 的升级路径不检查中间件终态；所有 WebSocket 在共享 onaccept 再做同一校验。
        if ((request.check_version(1, 1) && request.upgrade &&
             lower(request.get_header_value("Upgrade")) == "websocket") || allows(request)) return;
        response.code = 403;
        response.set_header("Content-Type", "application/json; charset=utf-8");
        response.end(u8"{\"error\":\"请求来源无效，请从本机软件页面操作。\"}");
    }
    void after_handle(crow::request&, crow::response&, context&) {}
};
}

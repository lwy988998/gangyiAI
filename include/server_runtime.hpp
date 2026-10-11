#pragma once
#include "config.hpp"
#include <string>
#include <nlohmann/json.hpp>

namespace gangyi {
struct ServerOptions {
    std::string publicRoot = "public";
    std::string sessionToken;
    bool mobile = false;
};
// 桌面进程与安卓 JNI 共用服务入口，不改变课程接口和数据库。
int runServer(const Config& config, const ServerOptions& options = {});
void stopServer();
nlohmann::json runtimeActivities(const std::string& databasePath);
void runtimeForeground(const std::string& databasePath, bool visible);
}

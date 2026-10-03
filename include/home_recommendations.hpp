#pragma once

#include <atomic>
#include <filesystem>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

namespace gangyi {

// 每次启动只尝试一次模型生成，页面读取不会触发额外请求。
class HomeRecommendations {
public:
    HomeRecommendations(std::string databasePath, std::string launchId);
    ~HomeRecommendations();
    void start();
    void stop();
    nlohmann::json view() const;

private:
    bool saveCache(const nlohmann::json& value) const;
    void generate();
    std::string databasePath_, launchId_;
    std::filesystem::path cachePath_;
    mutable std::mutex mutex_;
    nlohmann::json state_;
    std::atomic_bool stopped_{false};
    bool attempted_ = false;
    std::thread worker_;
};

}

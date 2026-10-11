#pragma once
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace gangyi {
// 安卓生命周期只控制执行时机，教学判断仍由现有主控完成。
class MobilePolicy {
public:
    static void configure(bool enabled) {
        std::lock_guard<std::mutex> lock(mutex_);
        enabled_ = enabled; foreground_ = true; interacted_ = false; allowed_.clear();
    }
    static bool enabled() { std::lock_guard<std::mutex> lock(mutex_); return enabled_; }
    static void submitted(const std::string& taskId, const std::string& type) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (enabled_ && foreground_ && type != "startup") {
            if (type != "learning_updated") interacted_ = true;
            if (interacted_) allowed_.insert(taskId);
        }
    }
    static void resumed(const std::string& taskId) { submitted(taskId, "resume"); }
    static void setForeground(bool foreground, const std::vector<std::string>& running = {}) {
        std::lock_guard<std::mutex> lock(mutex_);
        foreground_ = foreground;
        if (!foreground) allowed_ = std::set<std::string>(running.begin(), running.end());
    }
    static bool automaticWorkAllowed() {
        std::lock_guard<std::mutex> lock(mutex_);
        return !enabled_ || (foreground_ && interacted_);
    }
    static bool allows(const std::string& taskId) {
        std::lock_guard<std::mutex> lock(mutex_);
        return !enabled_ || allowed_.count(taskId) != 0;
    }
    static bool requestAllowed(const std::string& taskId) {
        std::lock_guard<std::mutex> lock(mutex_);
        return !enabled_ || foreground_ || allowed_.count(taskId) != 0;
    }
private:
    inline static std::mutex mutex_;
    inline static bool enabled_ = false;
    inline static bool foreground_ = true;
    inline static bool interacted_ = false;
    inline static std::set<std::string> allowed_;
};
}

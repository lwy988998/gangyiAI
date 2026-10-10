#pragma once
#include <nlohmann/json.hpp>
#include <cstdint>
#include <string>

namespace gangyi {
struct AIActivityInfo {
    std::string taskId;
    std::string source = "AI 服务";
    std::string purpose = "生成 AI 内容";
    std::string scopeId;
    std::string courseId;
    std::string lessonId;
    int legacyCalls = 0;
};

// 启动器与服务共享调用元数据和暂停开关，不保存提示词、密钥或模型正文。
class AIActivity {
public:
    static void configure(const std::string& path);
    static nlohmann::json snapshot();
    static void setPaused(bool paused);
    static bool paused();
    static std::uint64_t pauseEpoch();
    static std::string begin(const AIActivityInfo& info, const std::string& model);
    static bool requestStarted(const std::string& id);
    static void update(const std::string& id, const std::string& status, const std::string& model = {});
    static void shutdown();
    static bool stopping();
};
}

#pragma once
#include "ai_client.hpp"

namespace gangyi {
// 安卓设置复用模型目录解析，同时接受全局暂停与生命周期取消。
std::vector<std::string> loadModelCatalog(const AIClientConfig& config,
    const std::function<bool()>& cancelled, int timeoutMs = 15000);
}

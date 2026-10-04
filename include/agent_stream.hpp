#pragma once

#include <functional>
#include <map>
#include <string>

namespace gangyi {

struct AgentStreamDelta {
    std::string field;
    int actionIndex = -1;
    int optionIndex = -1;
    std::string text;
};

// 从真实模型的 JSON 分片中只提取面向学生的文字，隐藏答案和行动参数。
class AgentPublicStream {
public:
    using Sink = std::function<void(const AgentStreamDelta&)>;
    explicit AgentPublicStream(Sink sink);
    void push(const std::string& chunk);
    const std::string& content() const { return content_; }
private:
    Sink sink_;
    std::string content_;
    std::map<std::string, std::string> emitted_;
};

} // namespace gangyi

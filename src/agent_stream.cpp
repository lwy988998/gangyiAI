#include "agent_stream.hpp"
#include <nlohmann/json.hpp>
#include <cctype>
#include <optional>
#include <stdexcept>
#include <vector>

namespace gangyi {
namespace {
using Json = nlohmann::json;
struct StringValue {
    std::vector<std::string> path;
    std::string value;
};
struct Token { std::string value; bool complete = false; };

// 不完整的 Unicode 字符、转义和代理对留在缓冲区，下一片到达后再显示。
std::optional<std::string> decodePrefix(std::string encoded, bool complete) {
    for (int trim = 0; trim <= (complete ? 0 : 12) && !encoded.empty(); ++trim) {
        try { return Json::parse(encoded + '"').get<std::string>(); }
        catch (...) { encoded.pop_back(); }
    }
    return std::nullopt;
}

class PrefixReader {
public:
    explicit PrefixReader(const std::string& input) : input_(input) {}
    std::vector<StringValue> read() { value({}); return strings_; }
private:
    void space() { while (position_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[position_]))) ++position_; }
    Token string() {
        const auto start = position_++;
        bool escaped = false;
        while (position_ < input_.size()) {
            const char next = input_[position_++];
            if (!escaped && next == '"') {
                const auto raw = input_.substr(start, position_ - start);
                try { return {Json::parse(raw).get<std::string>(), true}; }
                catch (...) { return {}; }
            }
            if (!escaped && next == '\\') escaped = true;
            else escaped = false;
        }
        const auto decoded = decodePrefix(input_.substr(start), false);
        return {decoded.value_or(""), false};
    }
    bool value(std::vector<std::string> path) {
        space();
        if (position_ == input_.size()) return false;
        if (input_[position_] == '"') {
            const auto token = string();
            strings_.push_back({std::move(path), token.value});
            return token.complete;
        }
        if (input_[position_] == '{') {
            ++position_; space();
            if (position_ < input_.size() && input_[position_] == '}') { ++position_; return true; }
            while (position_ < input_.size()) {
                space();
                if (position_ == input_.size() || input_[position_] != '"') return false;
                const auto key = string();
                if (!key.complete) return false;
                space();
                if (position_ == input_.size() || input_[position_++] != ':') return false;
                auto child = path; child.push_back(key.value);
                if (!value(std::move(child))) return false;
                space();
                if (position_ == input_.size()) return false;
                if (input_[position_] == '}') { ++position_; return true; }
                if (input_[position_++] != ',') return false;
            }
            return false;
        }
        if (input_[position_] == '[') {
            ++position_; space();
            if (position_ < input_.size() && input_[position_] == ']') { ++position_; return true; }
            for (int index = 0; position_ < input_.size(); ++index) {
                auto child = path; child.push_back(std::to_string(index));
                if (!value(std::move(child))) return false;
                space();
                if (position_ == input_.size()) return false;
                if (input_[position_] == ']') { ++position_; return true; }
                if (input_[position_++] != ',') return false;
            }
            return false;
        }
        const auto begin = position_;
        while (position_ < input_.size() && input_[position_] != ',' && input_[position_] != '}' &&
               input_[position_] != ']' && !std::isspace(static_cast<unsigned char>(input_[position_]))) ++position_;
        return position_ > begin;
    }
    const std::string& input_;
    size_t position_ = 0;
    std::vector<StringValue> strings_;
};

std::string key(const std::vector<std::string>& path) {
    // JSON 数组序列化可以区分包含斜杠的键，避免伪造字段路径。
    return Json(path).dump();
}
} // namespace

AgentPublicStream::AgentPublicStream(Sink sink) : sink_(std::move(sink)) {}
void AgentPublicStream::push(const std::string& chunk) {
    if (content_.size() + chunk.size() > 2 * 1024 * 1024) throw std::runtime_error("模型响应过大");
    content_ += chunk;
    const auto values = PrefixReader(content_).read();
    std::map<std::string, std::string> properties;
    for (const auto& item : values) properties[key(item.path)] = item.value;
    for (const auto& item : values) {
        AgentStreamDelta delta;
        if (item.path == std::vector<std::string>{"message"}) delta.field = "message";
        else if (item.path.size() >= 5 && item.path[0] == "actions" && item.path[2] == "args" && item.path[3] == "section") {
            const auto prefix = std::vector<std::string>{"actions", item.path[1]};
            auto toolPath = prefix; toolPath.push_back("tool");
            auto kindPath = prefix; kindPath.insert(kindPath.end(), {"args", "section", "kind"});
            if (properties[key(toolPath)] != "append_section") continue;
            const auto kind = properties[key(kindPath)];
            if (kind != "explanation" && kind != "question" && kind != "summary") continue;
            if (item.path.size() == 5 && item.path[4] == "title") delta.field = "title";
            else if (item.path.size() == 5 && item.path[4] == "body" && kind != "question") delta.field = "body";
            else if (item.path.size() == 6 && item.path[4] == "question" && item.path[5] == "question" && kind == "question")
                delta.field = "question";
            else if (item.path.size() == 7 && item.path[4] == "question" && item.path[5] == "options" && kind == "question") {
                delta.field = "option";
                delta.optionIndex = std::stoi(item.path[6]);
            } else continue;
            delta.actionIndex = std::stoi(item.path[1]);
        } else continue;
        auto& previous = emitted_[key(item.path)];
        if (item.value.size() < previous.size() || item.value.compare(0, previous.size(), previous) != 0) continue;
        delta.text = item.value.substr(previous.size());
        previous = item.value;
        if (!delta.text.empty()) sink_(delta);
    }
}
} // namespace gangyi

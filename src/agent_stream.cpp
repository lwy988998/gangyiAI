// 【逐行批注】 引入头文件 "agent_stream.hpp"，为后续实现提供类型或接口声明。
#include "agent_stream.hpp"
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>
// 【逐行批注】 引入头文件 <optional>，为后续实现提供类型或接口声明。
#include <optional>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 声明数据结构或类型 StringValue 及其成员接口。
struct StringValue {
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> path;。
    std::vector<std::string> path;
    // 【逐行批注】 执行当前声明或表达式：std::string value;。
    std::string value;
    // 【逐行批注】 计算右侧结果并写入 bool complete，更新当前运行状态。
    bool complete = false;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明数据结构或类型 Token 及其成员接口。
struct Token { std::string value; bool complete = false; };

// 不完整的 Unicode 字符、转义和代理对留在缓冲区，下一片到达后再显示。
// 【逐行批注】 定义函数 decodePrefix，执行该函数声明的职责。
std::optional<std::string> decodePrefix(std::string encoded, bool complete) {
    // 【逐行批注】 按循环规则重复处理：for (int trim = 0; trim <= (complete ? 0 : 12) && !encoded.empty(); ++trim…。
    for (int trim = 0; trim <= (complete ? 0 : 12) && !encoded.empty(); ++trim) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { return Json::parse(encoded + '"').get<std::string>(); }
        // 【逐行批注】 捕获并处理异常：catch (...) { encoded.pop_back(); }。
        catch (...) { encoded.pop_back(); }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return std::nullopt;。
    return std::nullopt;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明数据结构或类型 PrefixReader 及其成员接口。
class PrefixReader {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 定义函数 PrefixReader，执行该函数声明的职责。
    explicit PrefixReader(const std::string& input) : input_(input) {}
    // 【逐行批注】 定义函数 read，执行该函数声明的职责。
    std::vector<StringValue> read() { value({}); return strings_; }
// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 定义函数 space，执行该函数声明的职责。
    void space() { while (position_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[position_]))) ++position_; }
    // 【逐行批注】 定义函数 string，执行该函数声明的职责。
    Token string() {
        // 【逐行批注】 计算右侧结果并写入 auto start，更新当前运行状态。
        const auto start = position_++;
        // 【逐行批注】 计算右侧结果并写入 bool escaped，更新当前运行状态。
        bool escaped = false;
        // 【逐行批注】 在条件成立期间循环处理：while (position_ < input_.size()) {。
        while (position_ < input_.size()) {
            // 【逐行批注】 计算右侧结果并写入 char next，更新当前运行状态。
            const char next = input_[position_++];
            // 【逐行批注】 判断条件 (!escaped && next == '"') {，成立时执行对应分支。
            if (!escaped && next == '"') {
                // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
                const auto raw = input_.substr(start, position_ - start);
                // 【逐行批注】 开始可能抛出异常的保护代码块。
                try { return {Json::parse(raw).get<std::string>(), true}; }
                // 【逐行批注】 捕获并处理异常：catch (...) { return {}; }。
                catch (...) { return {}; }
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 判断条件 (!escaped && next == '\\') escaped = true;，成立时执行对应分支。
            if (!escaped && next == '\\') escaped = true;
            // 【逐行批注】 执行前一条件不成立时的分支。
            else escaped = false;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 auto decoded，更新当前运行状态。
        const auto decoded = decodePrefix(input_.substr(start), false);
        // 【逐行批注】 结束当前函数并返回结果：return {decoded.value_or(""), false};。
        return {decoded.value_or(""), false};
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 value，执行该函数声明的职责。
    bool value(std::vector<std::string> path) {
        // 【逐行批注】 调用 space 完成本行所需操作：space();。
        space();
        // 【逐行批注】 判断条件 (position_ == input_.size()) return false;，成立时执行对应分支。
        if (position_ == input_.size()) return false;
        // 【逐行批注】 判断条件 (input_[position_] == '"') {，成立时执行对应分支。
        if (input_[position_] == '"') {
            // 【逐行批注】 计算右侧结果并写入 auto token，更新当前运行状态。
            const auto token = string();
            // 【逐行批注】 调用 move 完成本行所需操作：strings_.push_back({std::move(path), token.value, token.complete});。
            strings_.push_back({std::move(path), token.value, token.complete});
            // 【逐行批注】 结束当前函数并返回结果：return token.complete;。
            return token.complete;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (input_[position_] == '{') {，成立时执行对应分支。
        if (input_[position_] == '{') {
            // 【逐行批注】 调用 space 完成本行所需操作：++position_; space();。
            ++position_; space();
            // 【逐行批注】 判断条件 (position_ < input_.size() && input_[position_] == '}') { ++posi…，成立时执行对应分支。
            if (position_ < input_.size() && input_[position_] == '}') { ++position_; return true; }
            // 【逐行批注】 在条件成立期间循环处理：while (position_ < input_.size()) {。
            while (position_ < input_.size()) {
                // 【逐行批注】 调用 space 完成本行所需操作：space();。
                space();
                // 【逐行批注】 判断条件 (position_ == input_.size() || input_[position_] != '"') return …，成立时执行对应分支。
                if (position_ == input_.size() || input_[position_] != '"') return false;
                // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
                const auto key = string();
                // 【逐行批注】 判断条件 (!key.complete) return false;，成立时执行对应分支。
                if (!key.complete) return false;
                // 【逐行批注】 调用 space 完成本行所需操作：space();。
                space();
                // 【逐行批注】 判断条件 (position_ == input_.size() || input_[position_++] != ':') retur…，成立时执行对应分支。
                if (position_ == input_.size() || input_[position_++] != ':') return false;
                // 【逐行批注】 计算右侧结果并写入 child，更新当前运行状态。
                auto child = path; child.push_back(key.value);
                // 【逐行批注】 判断条件 (!value(std::move(child))) return false;，成立时执行对应分支。
                if (!value(std::move(child))) return false;
                // 【逐行批注】 调用 space 完成本行所需操作：space();。
                space();
                // 【逐行批注】 判断条件 (position_ == input_.size()) return false;，成立时执行对应分支。
                if (position_ == input_.size()) return false;
                // 【逐行批注】 判断条件 (input_[position_] == '}') { ++position_; return true; }，成立时执行对应分支。
                if (input_[position_] == '}') { ++position_; return true; }
                // 【逐行批注】 判断条件 (input_[position_++] != ',') return false;，成立时执行对应分支。
                if (input_[position_++] != ',') return false;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return false;。
            return false;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (input_[position_] == '[') {，成立时执行对应分支。
        if (input_[position_] == '[') {
            // 【逐行批注】 调用 space 完成本行所需操作：++position_; space();。
            ++position_; space();
            // 【逐行批注】 判断条件 (position_ < input_.size() && input_[position_] == ']') { ++posi…，成立时执行对应分支。
            if (position_ < input_.size() && input_[position_] == ']') { ++position_; return true; }
            // 【逐行批注】 按循环规则重复处理：for (int index = 0; position_ < input_.size(); ++index) {。
            for (int index = 0; position_ < input_.size(); ++index) {
                // 【逐行批注】 计算右侧结果并写入 child，更新当前运行状态。
                auto child = path; child.push_back(std::to_string(index));
                // 【逐行批注】 判断条件 (!value(std::move(child))) return false;，成立时执行对应分支。
                if (!value(std::move(child))) return false;
                // 【逐行批注】 调用 space 完成本行所需操作：space();。
                space();
                // 【逐行批注】 判断条件 (position_ == input_.size()) return false;，成立时执行对应分支。
                if (position_ == input_.size()) return false;
                // 【逐行批注】 判断条件 (input_[position_] == ']') { ++position_; return true; }，成立时执行对应分支。
                if (input_[position_] == ']') { ++position_; return true; }
                // 【逐行批注】 判断条件 (input_[position_++] != ',') return false;，成立时执行对应分支。
                if (input_[position_++] != ',') return false;
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 结束当前函数并返回结果：return false;。
            return false;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 auto begin，更新当前运行状态。
        const auto begin = position_;
        // 【逐行批注】 在条件成立期间循环处理：while (position_ < input_.size() && input_[position_] != ',' && input_[pos…。
        while (position_ < input_.size() && input_[position_] != ',' && input_[position_] != '}' &&
               // 【逐行批注】 调用 isspace 完成本行所需操作：input_[position_] != ']' && !std::isspace(static_cast<unsigned char>(…。
               input_[position_] != ']' && !std::isspace(static_cast<unsigned char>(input_[position_]))) ++position_;
        // 【逐行批注】 结束当前函数并返回结果：return position_ > begin;。
        return position_ > begin;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：const std::string& input_;。
    const std::string& input_;
    // 【逐行批注】 计算右侧结果并写入 size_t position_，更新当前运行状态。
    size_t position_ = 0;
    // 【逐行批注】 执行当前声明或表达式：std::vector<StringValue> strings_;。
    std::vector<StringValue> strings_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 key，执行该函数声明的职责。
std::string key(const std::vector<std::string>& path) {
    // JSON 数组序列化可以区分包含斜杠的键，避免伪造字段路径。
    // 【逐行批注】 结束当前函数并返回结果：return Json(path).dump();。
    return Json(path).dump();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 执行当前声明或表达式：} // namespace。
} // namespace

// 【逐行批注】 调用 AgentPublicStream、sink_、move 完成本行所需操作：AgentPublicStream::AgentPublicStream(Sink sink) : sink_(std::move(sin…。
AgentPublicStream::AgentPublicStream(Sink sink) : sink_(std::move(sink)) {}
// 【逐行批注】 定义函数 AgentPublicStream::push，执行该函数声明的职责。
void AgentPublicStream::push(const std::string& chunk) {
    // 【逐行批注】 判断条件 (content_.size() + chunk.size() > 2 * 1024 * 1024) throw std::ru…，成立时执行对应分支。
    if (content_.size() + chunk.size() > 2 * 1024 * 1024) throw std::runtime_error("模型响应过大");
    // 【逐行批注】 计算右侧结果并写入 content_ +，更新当前运行状态。
    content_ += chunk;
    // 【逐行批注】 计算右侧结果并写入 auto values，更新当前运行状态。
    const auto values = PrefixReader(content_).read();
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, std::string> properties;。
    std::map<std::string, std::string> properties;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : values) {。
    for (const auto& item : values) {
        // 【逐行批注】 计算右侧结果并写入 properties[key(item.path)]，更新当前运行状态。
        properties[key(item.path)] = item.value;
        // 【逐行批注】 判断条件 (item.path == std::vector<std::string>{"teaching", "helpType"} &…，成立时执行对应分支。
        if (item.path == std::vector<std::string>{"teaching", "helpType"} && item.complete) helpType_ = item.value;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : values) {。
    for (const auto& item : values) {
        // 【逐行批注】 执行当前声明或表达式：AgentStreamDelta delta;。
        AgentStreamDelta delta;
        // 【逐行批注】 判断条件 (item.path == std::vector<std::string>{"message"}) delta.field =…，成立时执行对应分支。
        if (item.path == std::vector<std::string>{"message"}) delta.field = "message";
        // 【逐行批注】 在前一条件不成立时继续判断：(item.path.size() >= 5 && item.path[0] == "actions" && item.path…。
        else if (item.path.size() >= 5 && item.path[0] == "actions" && item.path[2] == "args" && item.path[3] == "section") {
            // 【逐行批注】 计算右侧结果并写入 auto prefix，更新当前运行状态。
            const auto prefix = std::vector<std::string>{"actions", item.path[1]};
            // 【逐行批注】 计算右侧结果并写入 toolPath，更新当前运行状态。
            auto toolPath = prefix; toolPath.push_back("tool");
            // 【逐行批注】 计算右侧结果并写入 kindPath，更新当前运行状态。
            auto kindPath = prefix; kindPath.insert(kindPath.end(), {"args", "section", "kind"});
            // 【逐行批注】 判断条件 (properties[key(toolPath)] != "append_section") continue;，成立时执行对应分支。
            if (properties[key(toolPath)] != "append_section") continue;
            // 【逐行批注】 计算右侧结果并写入 auto kind，更新当前运行状态。
            const auto kind = properties[key(kindPath)];
            // 【逐行批注】 判断条件 (kind != "explanation" && kind != "question" && kind != "summary…，成立时执行对应分支。
            if (kind != "explanation" && kind != "question" && kind != "summary") continue;
            // 【逐行批注】 判断条件 (item.path.size() == 5 && item.path[4] == "title") delta.field =…，成立时执行对应分支。
            if (item.path.size() == 5 && item.path[4] == "title") delta.field = "title";
            // 【逐行批注】 在前一条件不成立时继续判断：(item.path.size() == 5 && item.path[4] == "body" && kind != "que…。
            else if (item.path.size() == 5 && item.path[4] == "body" && kind != "question") delta.field = "body";
            // 【逐行批注】 在前一条件不成立时继续判断：(item.path.size() == 6 && item.path[4] == "question" && item.pat…。
            else if (item.path.size() == 6 && item.path[4] == "question" && item.path[5] == "question" && kind == "question")
                // 【逐行批注】 计算右侧结果并写入 delta.field，更新当前运行状态。
                delta.field = "question";
            // 【逐行批注】 在前一条件不成立时继续判断：(item.path.size() == 7 && item.path[4] == "question" && item.pat…。
            else if (item.path.size() == 7 && item.path[4] == "question" && item.path[5] == "options" && kind == "question") {
                // 【逐行批注】 计算右侧结果并写入 delta.field，更新当前运行状态。
                delta.field = "option";
                // 【逐行批注】 计算右侧结果并写入 delta.optionIndex，更新当前运行状态。
                delta.optionIndex = std::stoi(item.path[6]);
            // 【逐行批注】 执行当前声明或表达式：} else continue;。
            } else continue;
            // 【逐行批注】 计算右侧结果并写入 delta.actionIndex，更新当前运行状态。
            delta.actionIndex = std::stoi(item.path[1]);
        // 【逐行批注】 执行当前声明或表达式：} else continue;。
        } else continue;
        // 【逐行批注】 计算右侧结果并写入 auto& previous，更新当前运行状态。
        auto& previous = emitted_[key(item.path)];
        // 【逐行批注】 判断条件 (item.value.size() < previous.size() || item.value.compare(0, pr…，成立时执行对应分支。
        if (item.value.size() < previous.size() || item.value.compare(0, previous.size(), previous) != 0) continue;
        // 【逐行批注】 计算右侧结果并写入 delta.text，更新当前运行状态。
        delta.text = item.value.substr(previous.size());
        // 【逐行批注】 计算右侧结果并写入 previous，更新当前运行状态。
        previous = item.value;
        // 【逐行批注】 判断条件 (!delta.text.empty()) sink_(delta);，成立时执行对应分支。
        if (!delta.text.empty()) sink_(delta);
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
} // namespace gangyi

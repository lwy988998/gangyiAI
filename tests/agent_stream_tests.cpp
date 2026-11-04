// 【逐行批注】 处理条件编译指令：#ifdef NDEBUG。
#ifdef NDEBUG
// 【逐行批注】 执行当前声明或表达式：#undef NDEBUG。
#undef NDEBUG
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 "agent_stream.hpp"，为后续实现提供类型或接口声明。
#include "agent_stream.hpp"
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <cassert>，为后续实现提供类型或接口声明。
#include <cassert>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 【逐行批注】 计算右侧结果并写入 Json response，更新当前运行状态。
    const Json response = {
        // 【逐行批注】 执行当前声明或表达式：{"message", "先看你实际写出的理由。\n化合价升降才是依据。🚀"},。
        {"message", "先看你实际写出的理由。\n化合价升降才是依据。🚀"},
        // 【逐行批注】 调用 array 完成本行所需操作：{"actions", Json::array({。
        {"actions", Json::array({
            // 【逐行批注】 执行当前声明或表达式：{{"tool", "append_section"}, {"args", {。
            {{"tool", "append_section"}, {"args", {
                // 【逐行批注】 执行当前声明或表达式：{"lessonTaskId", "lesson-real"},。
                {"lessonTaskId", "lesson-real"},
                // 【逐行批注】 执行当前声明或表达式：{"section", {{"kind", "question"}, {"title", "试试看"},。
                {"section", {{"kind", "question"}, {"title", "试试看"},
                    // 【逐行批注】 执行当前声明或表达式：{"question", {{"question", "判断这个反应，说明依据。"}, {"options", {"甲", "乙"}},。
                    {"question", {{"question", "判断这个反应，说明依据。"}, {"options", {"甲", "乙"}},
                        // 【逐行批注】 执行当前声明或表达式：{"answerIndex", 1}, {"expectedAnswer", "隐藏标准答案"}, {"rubric", "隐藏评分标准"}}}}}。
                        {"answerIndex", 1}, {"expectedAnswer", "隐藏标准答案"}, {"rubric", "隐藏评分标准"}}}}}
            // 【逐行批注】 执行当前声明或表达式：}}},。
            }}},
            // 【逐行批注】 执行当前声明或表达式：{{"tool", "append_section"}, {"args", {{"section", {。
            {{"tool", "append_section"}, {"args", {{"section", {
                // 【逐行批注】 执行当前声明或表达式：{"kind", "explanation"}, {"title", "补讲"}, {"body", "这是新课的真实讲解。\n$x^2$"}。
                {"kind", "explanation"}, {"title", "补讲"}, {"body", "这是新课的真实讲解。\n$x^2$"}
            // 【逐行批注】 执行当前声明或表达式：}}}}},。
            }}}}},
            // 【逐行批注】 执行当前声明或表达式：{{"tool", "adjust_goal"}, {"args", {{"section", {。
            {{"tool", "adjust_goal"}, {"args", {{"section", {
                // 【逐行批注】 执行当前声明或表达式：{"kind", "explanation"}, {"body", "禁止把其他工具的参数当正文"}。
                {"kind", "explanation"}, {"body", "禁止把其他工具的参数当正文"}
            // 【逐行批注】 执行当前声明或表达式：}}}}}。
            }}}}}
        // 【逐行批注】 执行当前声明或表达式：})}。
        })}
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, std::string> rendered;。
    std::map<std::string, std::string> rendered;
    // 【逐行批注】 计算右侧结果并写入 int fragments，更新当前运行状态。
    int fragments = 0;
    // 【逐行批注】 定义函数 stream，执行该函数声明的职责。
    gangyi::AgentPublicStream stream([&](const auto& delta) {
        // 【逐行批注】 计算右侧结果并写入 rendered[delta.field + ":" + std::to_string(…，更新当前运行状态。
        rendered[delta.field + ":" + std::to_string(delta.actionIndex) + ":" + std::to_string(delta.optionIndex)] += delta.text;
        // 【逐行批注】 执行当前声明或表达式：++fragments;。
        ++fragments;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
    const auto raw = response.dump(-1, ' ', true);
    // 【逐行批注】 按循环规则重复处理：for (const auto byte : raw) stream.push(std::string(1, byte));。
    for (const auto byte : raw) stream.push(std::string(1, byte));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(rendered["message:-1:-1"] == response["message"]);。
    assert(rendered["message:-1:-1"] == response["message"]);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(rendered["question:0:-1"] == "判断这个反应，说明依据。");。
    assert(rendered["question:0:-1"] == "判断这个反应，说明依据。");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(rendered["body:1:-1"] == "这是新课的真实讲解。\n$x^2$");。
    assert(rendered["body:1:-1"] == "这是新课的真实讲解。\n$x^2$");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(rendered["option:0:1"] == "乙");。
    assert(rendered["option:0:1"] == "乙");
    // 【逐行批注】 按循环规则重复处理：for (const auto& [name, text] : rendered) {。
    for (const auto& [name, text] : rendered) {
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(text.find("隐藏") == std::string::npos);。
        assert(text.find("隐藏") == std::string::npos);
        // 【逐行批注】 调用 assert 完成本行所需操作：assert(text.find("禁止") == std::string::npos);。
        assert(text.find("禁止") == std::string::npos);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(fragments > 20);。
    assert(fragments > 20);
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(stream.content() == raw);。
    assert(stream.content() == raw);
    // 伪造带斜杠的属性不能匹配真正的公开字段路径。
    // 【逐行批注】 执行当前声明或表达式：rendered.clear();。
    rendered.clear();
    // 【逐行批注】 定义函数 malicious，执行该函数声明的职责。
    gangyi::AgentPublicStream malicious([&](const auto& delta) { rendered[delta.field] += delta.text; });
    // 【逐行批注】 执行当前声明或表达式：malicious.push(R"({"actions/0/args/section/body":"私密","message":"公开","answer":"私密"})…。
    malicious.push(R"({"actions/0/args/section/body":"私密","message":"公开","answer":"私密"})");
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(rendered.size() == 1 && rendered["message"] == "公开");。
    assert(rendered.size() == 1 && rendered["message"] == "公开");
    // 未转义中文按单字节分片传输时也不得损坏编码。
    // 【逐行批注】 执行当前声明或表达式：rendered.clear();。
    rendered.clear();
    // 【逐行批注】 定义函数 utf8，执行该函数声明的职责。
    gangyi::AgentPublicStream utf8([&](const auto& delta) { rendered[delta.field] += delta.text; });
    // 【逐行批注】 计算右侧结果并写入 std::string chinese，更新当前运行状态。
    const std::string chinese = R"({"message":"中文🚀测试"})";
    // 【逐行批注】 按循环规则重复处理：for (char byte : chinese) utf8.push(std::string(1, byte));。
    for (char byte : chinese) utf8.push(std::string(1, byte));
    // 【逐行批注】 调用 assert 完成本行所需操作：assert(rendered["message"] == "中文🚀测试");。
    assert(rendered["message"] == "中文🚀测试");
    // 【逐行批注】 执行当前声明或表达式：std::cout << "公开分片、Unicode 与答案隔离测试通过\n";。
    std::cout << "公开分片、Unicode 与答案隔离测试通过\n";
// 【逐行批注】 结束当前代码块。
}

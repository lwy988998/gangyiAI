#ifdef NDEBUG
#undef NDEBUG
#endif
#include "agent_stream.hpp"
#include <nlohmann/json.hpp>
#include <cassert>
#include <iostream>
#include <map>

int main() {
    using Json = nlohmann::json;
    const Json response = {
        {"message", "先看你实际写出的理由。\n化合价升降才是依据。🚀"},
        {"actions", Json::array({
            {{"tool", "append_section"}, {"args", {
                {"lessonTaskId", "lesson-real"},
                {"section", {{"kind", "question"}, {"title", "试试看"},
                    {"question", {{"question", "判断这个反应，说明依据。"}, {"options", {"甲", "乙"}},
                        {"answerIndex", 1}, {"expectedAnswer", "隐藏标准答案"}, {"rubric", "隐藏评分标准"}}}}}
            }}},
            {{"tool", "append_section"}, {"args", {{"section", {
                {"kind", "explanation"}, {"title", "补讲"}, {"body", "这是新课的真实讲解。\n$x^2$"}
            }}}}},
            {{"tool", "adjust_goal"}, {"args", {{"section", {
                {"kind", "explanation"}, {"body", "禁止把其他工具的参数当正文"}
            }}}}}
        })}
    };
    std::map<std::string, std::string> rendered;
    int fragments = 0;
    gangyi::AgentPublicStream stream([&](const auto& delta) {
        rendered[delta.field + ":" + std::to_string(delta.actionIndex) + ":" + std::to_string(delta.optionIndex)] += delta.text;
        ++fragments;
    });
    const auto raw = response.dump(-1, ' ', true);
    for (const auto byte : raw) stream.push(std::string(1, byte));
    assert(rendered["message:-1:-1"] == response["message"]);
    assert(rendered["question:0:-1"] == "判断这个反应，说明依据。");
    assert(rendered["body:1:-1"] == "这是新课的真实讲解。\n$x^2$");
    assert(rendered["option:0:1"] == "乙");
    for (const auto& [name, text] : rendered) {
        assert(text.find("隐藏") == std::string::npos);
        assert(text.find("禁止") == std::string::npos);
    }
    assert(fragments > 20);
    assert(stream.content() == raw);
    // 伪造带斜杠的属性不能匹配真正的公开字段路径。
    rendered.clear();
    gangyi::AgentPublicStream malicious([&](const auto& delta) { rendered[delta.field] += delta.text; });
    malicious.push(R"({"actions/0/args/section/body":"私密","message":"公开","answer":"私密"})");
    assert(rendered.size() == 1 && rendered["message"] == "公开");
    // 未转义中文按单字节分片传输时也不得损坏编码。
    rendered.clear();
    gangyi::AgentPublicStream utf8([&](const auto& delta) { rendered[delta.field] += delta.text; });
    const std::string chinese = R"({"message":"中文🚀测试"})";
    for (char byte : chinese) utf8.push(std::string(1, byte));
    assert(rendered["message"] == "中文🚀测试");
    std::cout << "公开分片、Unicode 与答案隔离测试通过\n";
}

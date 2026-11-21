#include "exam_message_stream.hpp"
#include "exam_search_query.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    const std::string query = "试卷ABC";
    if (gangyi::examUtf8Prefix(query, 4) != "试" || gangyi::examUtf8Prefix(query, 6) != "试卷" ||
        gangyi::examUtf8Prefix(query, 8) != "试卷AB" || !gangyi::examUtf8Prefix(query, 0).empty())
        throw std::runtime_error("检索前缀不能截断中文字符");
    const auto queries = gangyi::examSearchQueries({{"title", "历史试卷"}, {"questions", nlohmann::json::array({
        {{"recognitionStatus", "uncertain"}, {"stem", "缺失题干不能作为搜索线索"}},
        {{"recognitionStatus", "ready"}, {"stem", "清晰题目一包含足够长的文本线索"}},
        {{"recognitionStatus", "ready"}, {"stem", "清晰题目二包含不同的文本线索"}},
        {{"recognitionStatus", "ready"}, {"stem", "第三题不应继续增加搜索请求"}}
    })}});
    if (queries.size() != 2 || queries[0].find("清晰题目一") == std::string::npos || queries[1].find("清晰题目二") == std::string::npos)
        throw std::runtime_error("应使用两道清晰题目搜索，跳过残缺题目");
    const std::string payload = R"({"message":"第一行\n第二行：\u4e2d\u6587和\"引号\"","artifacts":[{"content":"不能提前输出"}]})";
    std::string previous;
    for (std::size_t index = 0; index <= payload.size(); ++index) {
        const auto text = gangyi::examMessageStreamText(payload.substr(0, index));
        if (text.compare(0, previous.size(), previous) != 0) throw std::runtime_error("消息增量回退或改变");
        if (text.find("不能提前输出") != std::string::npos) throw std::runtime_error("其他字段被提前输出");
        previous = text;
    }
    if (previous != "第一行\n第二行：中文和\"引号\"") throw std::runtime_error("消息解码不正确");
    if (!gangyi::examMessageStreamText(R"({"answer":"隐藏","message":"晚到"})").empty())
        throw std::runtime_error("未确认消息字段顺序时不应输出内容");
    const auto repaired = gangyi::parseExamModelJson(R"({"message":"图中有 ] 和 \\\"引号\\\"","artifacts":[{"type":"mindmap","content":{"root":{"label":"主题"}}],"state":"ok"})");
    if (!repaired.is_object() || repaired["artifacts"][0]["content"]["root"]["label"] != "主题" || repaired["state"] != "ok")
        throw std::runtime_error("缺失一个内部闭合符时应保持原文并修复");
    if (!gangyi::parseExamModelJson(R"({"message":"截断","artifacts":[{"type":"mindmap"})").is_discarded())
        throw std::runtime_error("结尾截断不能自动补全");
    if (!gangyi::parseExamModelJson(R"({"message":"多处缺失","artifacts":[{"a":[{"x":1],"b":[{"y":2]}]})").is_discarded())
        throw std::runtime_error("多处缺失闭合符不能自动补全");
    std::cout << "试卷讲解消息流测试通过\n";
}

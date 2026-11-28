#include "structured_reply.hpp"
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
    const auto accepted = gangyi::parseStructuredReply(" ```json\r\n{\"message\":\"含括号 ] 的正文\"}\r\n```\n");
    if (accepted.value("message", "") != "含括号 ] 的正文") throw std::runtime_error("完整 JSON 围栏没有正确解析");
    const std::string rejected[] = {
        R"({"message":"截断","artifacts":[{"type":"mindmap"})",
        R"({"message":"缺少内部闭合符","artifacts":[{"type":"mindmap","content":{"root":{"label":"主题"}}],"state":"ok"})",
        R"({"message":"前一对象"} {"message":"后一对象"})",
        "说明后面才是 JSON：{\"message\":\"含前言\"}",
        "```json\n{\"message\":\"额外正文\"}\n```\n注释",
    };
    for (const auto& reply : rejected) {
        try {
            (void)gangyi::parseStructuredReply(reply);
        } catch (const nlohmann::json::parse_error&) {
            continue;
        }
        throw std::runtime_error("不完整或混入额外内容的回复必须拒绝");
    }
    std::cout << "结构化回复边界测试通过\n";
}

// 试卷讲解离线契约检查：不启动模型、不联网，也不修改应用数据库。
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

void requireContains(const std::string& document, const std::string& needle, const char* message) {
    if (document.find(needle) == std::string::npos) {
        std::cerr << "失败: " << message << "，缺少 " << needle << '\n';
        std::exit(EXIT_FAILURE);
    }
}

std::size_t countOccurrences(const std::string& document, const std::string& needle) {
    std::size_t count = 0;
    std::size_t offset = 0;
    while ((offset = document.find(needle, offset)) != std::string::npos) {
        ++count;
        offset += needle.size();
    }
    return count;
}

}  // namespace

int main(int argc, char** argv) {
    const std::string path = argc > 1 ? argv[1] : "tests/fixtures/exam-explanation-contract.json";
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        std::cerr << "失败: 无法读取试卷讲解契约夹具 " << path << '\n';
        return EXIT_FAILURE;
    }
    const std::string document((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

    requireContains(document, "\"schemaVersion\": 1", "契约版本不正确");
    if (countOccurrences(document, "{ \"name\":") < 10) {
        std::cerr << "失败: 高中学科覆盖少于 10 个\n";
        return EXIT_FAILURE;
    }
    requireContains(document, "\"expected\": \"startLearning\"", "清晰题目没有立即开学契约");
    requireContains(document, "\"expected\": \"requestCorrectionBeforeTeachingThatQuestion\"", "模糊题目没有校对契约");
    requireContains(document, "\"expected\": \"continueWithBestEffortAndMarkUncertain\"", "手写不确定内容没有继续批改契约");
    requireContains(document, "\"match\": \"confirmedSameExam\"", "同卷匹配状态缺失");
    requireContains(document, "\"gradingEligible\": true", "确认同卷没有批改资格");
    requireContains(document, "\"gradingEligible\": false", "相似试卷必须禁止作为正式批改依据");
    requireContains(document, "AI 估分，非官方", "AI 估分缺少非官方标记");
    requireContains(document, "scoringBasis", "估分缺少评分依据字段");
    requireContains(document, "uncertainty", "估分缺少不确定性字段");

    for (const auto* control : {"directAnswer", "skip", "previousQuestion", "pause", "resume", "editAnswer"})
        requireContains(document, control, "自然对话控制不完整");
    for (const auto* artifact : {"table", "code", "math", "chart", "functionPlot", "flowchart", "mindmap"})
        requireContains(document, artifact, "富内容类型不完整");
    requireContains(document, "onlyAfterExplicitRequestAndInIsolation", "代码执行缺少显式请求和隔离约束");
    requireContains(document, "renderAsTextOrSanitizedMarkup", "富内容没有文本净化约束");
    requireContains(document, "inventedValuesAllowed\": false", "图表不应允许编造数值");
    requireContains(document, "\"storage\": \"localOnly\"", "持久化范围不是仅本机");
    requireContains(document, "preserveOriginalPageBytes", "原始试卷页面没有保留约束");
    requireContains(document, "generativeImageCreation", "暂缓项没有记录生成式配图");

    std::cout << "试卷讲解 C++ 离线契约通过: " << path << '\n';
    return EXIT_SUCCESS;
}

#include "learning_generator.hpp"
#include <iostream>
#include <string>

int main() {
    using Json = nlohmann::json;
    int failures = 0;
    const auto expect = [&](bool condition, const char* message) {
        if (!condition) { std::cerr << "失败：" << message << '\n'; ++failures; }
    };
    const Json privateExample = {{"examples", Json::array({{{"title", "例题"}, {"content", "求 x"},
        {"solution", "秘密答案"}, {"rubric", "秘密评分"}, {"extra", {{"title", "秘密字段"}}}}})}};
    const auto publicExample = gangyi::publicPreparationBlock("examples", privateExample);
    expect(publicExample.dump().find("秘密") == std::string::npos, "公开例题必须采用路径白名单");
    expect(publicExample["examples"][0]["content"] == "求 x", "公开例题必须保留材料");
    const std::string raw = Json{{"quiz", Json::array({{{"question", "真实问题？"}, {"options", {"甲", "乙", "丙", "丁"}},
        {"answerIndex", 1}, {"explanation", "秘密解析"}, {"nested", {{"question", "秘密嵌套"}}}}})}}.dump();
    gangyi::PublicPreparationStream stream("quiz");
    std::string visible;
    for (std::size_t i = 0; i < raw.size(); ++i)
        for (const auto& chunk : stream.feed(raw.substr(i, 1))) visible += chunk;
    expect(visible.find("真实问题") != std::string::npos, "真实分片逐字输入仍能提取公开题干");
    expect(visible.find("秘密") == std::string::npos, "公开流不能泄漏私有解析与嵌套字段");
    expect(visible.find("answerIndex") == std::string::npos, "公开流不能泄漏答案索引");
    gangyi::PublicPreparationStream reason("decision");
    std::string adjusted;
    for (const auto& chunk : reason.feed("{\"reason\":\"先补")) adjusted += chunk;
    expect(adjusted == "先补", "调整说明必须在 JSON 尚未完成时展示");
    for (const auto& chunk : reason.feed("弱\",\"taskId\":\"私有任务\"}")) adjusted += chunk;
    expect(adjusted == "先补弱", "内部决策字段不能混入调整说明");
    gangyi::PublicPreparationStream escaped("decision");
    std::string decoded;
    for (const auto& chunk : escaped.feed("{\"reason\":\"\\u4e")) decoded += chunk;
    expect(decoded.empty(), "不完整 Unicode 转义不可提前显示");
    for (const auto& chunk : escaped.feed("2d文\\n解释\"}")) decoded += chunk;
    expect(decoded == "中文\n解释", "分片转义必须正确解码");
    gangyi::PublicPreparationStream malformed("quiz");
    std::string privateNested;
    for (const auto& chunk : malformed.feed("{\"quiz\":[{\"options\":{\"answerIndex\":\"秘密答案\"}}]}")) privateNested += chunk;
    expect(privateNested.empty(), "私有字段不能借错误的数组结构混入公开流");
    gangyi::PublicPreparationStream numericObject("quiz");
    for (const auto& chunk : numericObject.feed("{\"quiz\":[{\"options\":{\"0\":\"秘密答案\"}}]}")) privateNested += chunk;
    expect(privateNested.empty(), "对象的数字键不能冒充选项数组索引");
    return failures ? 1 : 0;
}

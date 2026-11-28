// 【逐行批注】 引入头文件 "learning_generator.hpp"，为后续实现提供类型或接口声明。
#include "learning_generator.hpp"
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
    // 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
    using Json = nlohmann::json;
    // 【逐行批注】 计算右侧结果并写入 int failures，更新当前运行状态。
    int failures = 0;
    // 【逐行批注】 计算右侧结果并写入 auto expect，更新当前运行状态。
    const auto expect = [&](bool condition, const char* message) {
        // 【逐行批注】 判断条件 (!condition) { std::cerr << "失败：" << message << '\n'; ++failures…，成立时执行对应分支。
        if (!condition) { std::cerr << "失败：" << message << '\n'; ++failures; }
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 Json privateExample，更新当前运行状态。
    const Json privateExample = {{"examples", Json::array({{{"title", "例题"}, {"content", "求 x"},
        // 【逐行批注】 执行当前声明或表达式：{"solution", "秘密答案"}, {"rubric", "秘密评分"}, {"extra", {{"title", "秘密字段"}}}}})}};。
        {"solution", "秘密答案"}, {"rubric", "秘密评分"}, {"extra", {{"title", "秘密字段"}}}}})}};
    // 【逐行批注】 计算右侧结果并写入 auto publicExample，更新当前运行状态。
    const auto publicExample = gangyi::publicPreparationBlock("examples", privateExample);
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(publicExample.dump().find("秘密") == std::string::npos, "公开例题必须采…。
    expect(publicExample.dump().find("秘密") == std::string::npos, "公开例题必须采用路径白名单");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(publicExample["examples"][0]["content"] == "求 x", "公开例题必须保留材料"…。
    expect(publicExample["examples"][0]["content"] == "求 x", "公开例题必须保留材料");
    // 【逐行批注】 计算右侧结果并写入 std::string raw，更新当前运行状态。
    const std::string raw = Json{{"quiz", Json::array({{{"question", "真实问题？"}, {"options", {"甲", "乙", "丙", "丁"}},
        // 【逐行批注】 执行当前声明或表达式：{"answerIndex", 1}, {"explanation", "秘密解析"}, {"nested", {{"question", "秘密嵌套"}}}}})}}…。
        {"answerIndex", 1}, {"explanation", "秘密解析"}, {"nested", {{"question", "秘密嵌套"}}}}})}}.dump();
    // 【逐行批注】 声明对象 stream，并通过构造函数初始化其状态。
    gangyi::PublicPreparationStream stream("quiz");
    // 【逐行批注】 执行当前声明或表达式：std::string visible;。
    std::string visible;
    // 【逐行批注】 按循环规则重复处理：for (std::size_t i = 0; i < raw.size(); ++i)。
    for (std::size_t i = 0; i < raw.size(); ++i)
        // 【逐行批注】 按循环规则重复处理：for (const auto& chunk : stream.feed(raw.substr(i, 1))) visible += chunk;。
        for (const auto& chunk : stream.feed(raw.substr(i, 1))) visible += chunk;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(visible.find("真实问题") != std::string::npos, "真实分片逐字输入仍能提取公开题干");。
    expect(visible.find("真实问题") != std::string::npos, "真实分片逐字输入仍能提取公开题干");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(visible.find("秘密") == std::string::npos, "公开流不能泄漏私有解析与嵌套字段");。
    expect(visible.find("秘密") == std::string::npos, "公开流不能泄漏私有解析与嵌套字段");
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(visible.find("answerIndex") == std::string::npos, "公开流不能泄漏答案索引…。
    expect(visible.find("answerIndex") == std::string::npos, "公开流不能泄漏答案索引");
    // 【逐行批注】 声明对象 reason，并通过构造函数初始化其状态。
    gangyi::PublicPreparationStream reason("decision");
    // 【逐行批注】 执行当前声明或表达式：std::string adjusted;。
    std::string adjusted;
    // 【逐行批注】 按循环规则重复处理：for (const auto& chunk : reason.feed("{\"reason\":\"先补")) adjusted += chun…。
    for (const auto& chunk : reason.feed("{\"reason\":\"先补")) adjusted += chunk;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(adjusted == "先补", "调整说明必须在 JSON 尚未完成时展示");。
    expect(adjusted == "先补", "调整说明必须在 JSON 尚未完成时展示");
    // 【逐行批注】 按循环规则重复处理：for (const auto& chunk : reason.feed("弱\",\"taskId\":\"私有任务\"}")) adjusted…。
    for (const auto& chunk : reason.feed("弱\",\"taskId\":\"私有任务\"}")) adjusted += chunk;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(adjusted == "先补弱", "内部决策字段不能混入调整说明");。
    expect(adjusted == "先补弱", "内部决策字段不能混入调整说明");
    // 【逐行批注】 声明对象 escaped，并通过构造函数初始化其状态。
    gangyi::PublicPreparationStream escaped("decision");
    // 【逐行批注】 执行当前声明或表达式：std::string decoded;。
    std::string decoded;
    // 【逐行批注】 按循环规则重复处理：for (const auto& chunk : escaped.feed("{\"reason\":\"\\u4e")) decoded += c…。
    for (const auto& chunk : escaped.feed("{\"reason\":\"\\u4e")) decoded += chunk;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(decoded.empty(), "不完整 Unicode 转义不可提前显示");。
    expect(decoded.empty(), "不完整 Unicode 转义不可提前显示");
    // 【逐行批注】 按循环规则重复处理：for (const auto& chunk : escaped.feed("2d文\\n解释\"}")) decoded += chunk;。
    for (const auto& chunk : escaped.feed("2d文\\n解释\"}")) decoded += chunk;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(decoded == "中文\n解释", "分片转义必须正确解码");。
    expect(decoded == "中文\n解释", "分片转义必须正确解码");
    // 【逐行批注】 声明对象 malformed，并通过构造函数初始化其状态。
    gangyi::PublicPreparationStream malformed("quiz");
    // 【逐行批注】 执行当前声明或表达式：std::string privateNested;。
    std::string privateNested;
    // 【逐行批注】 按循环规则重复处理：for (const auto& chunk : malformed.feed("{\"quiz\":[{\"options\":{\"answer…。
    for (const auto& chunk : malformed.feed("{\"quiz\":[{\"options\":{\"answerIndex\":\"秘密答案\"}}]}")) privateNested += chunk;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(privateNested.empty(), "私有字段不能借错误的数组结构混入公开流");。
    expect(privateNested.empty(), "私有字段不能借错误的数组结构混入公开流");
    // 【逐行批注】 声明对象 numericObject，并通过构造函数初始化其状态。
    gangyi::PublicPreparationStream numericObject("quiz");
    // 【逐行批注】 按循环规则重复处理：for (const auto& chunk : numericObject.feed("{\"quiz\":[{\"options\":{\"0\…。
    for (const auto& chunk : numericObject.feed("{\"quiz\":[{\"options\":{\"0\":\"秘密答案\"}}]}")) privateNested += chunk;
    // 【逐行批注】 调用 expect 完成本行所需操作：expect(privateNested.empty(), "对象的数字键不能冒充选项数组索引");。
    expect(privateNested.empty(), "对象的数字键不能冒充选项数组索引");
    // 【逐行批注】 结束当前函数并返回结果：return failures ? 1 : 0;。
    return failures ? 1 : 0;
// 【逐行批注】 结束当前代码块。
}

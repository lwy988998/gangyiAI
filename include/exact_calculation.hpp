#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string>

namespace gangyi {
namespace exact_calculation {

// 限制每一步的分子、分母和树规模；超限即拒绝，不能溢出后再给教学结论。
inline constexpr std::int64_t limit = 1000000000000LL;
inline constexpr int nodeLimit = 63;
inline constexpr int depthLimit = 12;

struct Rational {
    std::int64_t numerator;
    std::int64_t denominator;
};

inline std::int64_t magnitude(std::int64_t value) {
    // 调用方先约束到 ±limit，避免对 INT64_MIN 取负产生未定义行为。
    return value < 0 ? -value : value;
}

inline std::int64_t checkedInteger(const nlohmann::json& value) {
    if (!value.is_number_integer())
        throw std::invalid_argument("精确计算的整数须在正负10^12以内，不能使用浮点数或布尔值");
    // JSON无符号大整数不能先转有符号，亦不能依赖不同数值类型的比较转换。
    if (value.is_number_unsigned()) {
        const auto integer = value.get<std::uint64_t>();
        if (integer > static_cast<std::uint64_t>(limit)) throw std::invalid_argument("整数超过精确计算范围");
        return static_cast<std::int64_t>(integer);
    }
    const auto integer = value.get<std::int64_t>();
    if (integer < -limit || integer > limit) throw std::invalid_argument("整数超过精确计算范围");
    return integer;
}

inline Rational normalized(std::int64_t numerator, std::int64_t denominator) {
    if (!denominator) throw std::invalid_argument("分母不能为零，当前条件下无法完成计算");
    if (denominator < 0) { numerator = -numerator; denominator = -denominator; }
    const auto factor = std::gcd(numerator, denominator);
    return {numerator / factor, denominator / factor};
}

inline std::int64_t multiply(std::int64_t a, std::int64_t b) {
    if (a && magnitude(b) > limit / magnitude(a))
        throw std::invalid_argument("精确计算中间值超过10^12，请缩小计算范围或核对题目");
    return a * b;
}

inline std::int64_t add(std::int64_t a, std::int64_t b) {
    if ((b > 0 && a > limit - b) || (b < 0 && a < -limit - b))
        throw std::invalid_argument("精确计算中间值超过10^12，请缩小计算范围或核对题目");
    return a + b;
}

inline Rational decimal(const std::string& value) {
    // 只解析有限十进制，最多六位小数；不解释表达式、科学计数法或任何代码。
    if (value.empty() || value.size() > 22) throw std::invalid_argument("十进制文字为空或过长");
    std::size_t start = value.front() == '-' ? 1 : 0;
    if (start == value.size()) throw std::invalid_argument("十进制文字缺少数字");
    std::int64_t numerator = 0, denominator = 1;
    bool dot = false, digitBefore = false, digitAfter = false;
    for (std::size_t i = start; i < value.size(); ++i) {
        const auto character = value[i];
        if (character == '.' && !dot && digitBefore) { dot = true; continue; }
        if (character < '0' || character > '9') throw std::invalid_argument("只允许普通十进制数字文字");
        numerator = add(multiply(numerator, 10), character - '0');
        if (dot) {
            digitAfter = true;
            if (denominator == 1000000) throw std::invalid_argument("十进制输入最多六位小数");
            denominator *= 10;
        } else digitBefore = true;
    }
    if (!digitBefore || (dot && !digitAfter)) throw std::invalid_argument("十进制文字格式无效");
    return normalized(start ? -numerator : numerator, denominator);
}

inline void keys(const nlohmann::json& object, std::initializer_list<const char*> allowed) {
    for (const auto& item : object.items()) {
        bool found = false;
        for (const auto* key : allowed) found = found || item.key() == key;
        if (!found) throw std::invalid_argument("精确计算包含未授权字段，不能指定代码、文件、答案或成绩");
    }
}

inline Rational evaluate(const nlohmann::json& node, int depth, int& visited) {
    if (++visited > nodeLimit || depth > depthLimit)
        throw std::invalid_argument("精确计算表达式过大或嵌套过深");
    if (node.is_number_integer()) return {checkedInteger(node), 1};
    if (node.is_string()) return decimal(node.get<std::string>());
    if (!node.is_object()) throw std::invalid_argument("表达式只接受整数、十进制文字、分数或四则运算对象");
    if (node.contains("numerator")) {
        keys(node, {"numerator", "denominator"});
        return normalized(checkedInteger(node.at("numerator")), checkedInteger(node.at("denominator")));
    }
    keys(node, {"op", "left", "right"});
    const auto operation = node.value("op", "");
    if (operation != "add" && operation != "subtract" && operation != "multiply" && operation != "divide")
        throw std::invalid_argument("只支持add、subtract、multiply、divide四则运算");
    auto a = evaluate(node.at("left"), depth + 1, visited);
    auto b = evaluate(node.at("right"), depth + 1, visited);
    if (operation == "add" || operation == "subtract") {
        const auto common = std::gcd(a.denominator, b.denominator);
        const auto left = multiply(a.numerator, b.denominator / common);
        const auto right = multiply(b.numerator, a.denominator / common);
        return normalized(add(left, operation == "add" ? right : -right),
                          multiply(a.denominator, b.denominator / common));
    }
    if (operation == "divide") {
        if (!b.numerator) throw std::invalid_argument("除数不能为零，当前条件下无法完成计算");
        b = normalized(b.denominator, b.numerator);
    }
    // 先交叉约分，能精确处理大分数相消，同时仍对剩余乘法做溢出检查。
    const auto crossA = std::gcd(a.numerator, b.denominator);
    const auto crossB = std::gcd(b.numerator, a.denominator);
    return normalized(multiply(a.numerator / crossA, b.numerator / crossB),
                      multiply(a.denominator / crossB, b.denominator / crossA));
}

} // namespace exact_calculation

inline nlohmann::json calculateExact(const nlohmann::json& expression) {
    int visited = 0;
    const auto result = exact_calculation::evaluate(expression, 1, visited);
    const auto exact = std::to_string(result.numerator) +
        (result.denominator == 1 ? std::string() : "/" + std::to_string(result.denominator));
    // verified 只指这个结构化算式的算术结果，不证明模型从原题提取的算式正确。
    return {{"protocol", "exact-calculation-v1"}, {"expression", expression},
            {"numerator", result.numerator}, {"denominator", result.denominator}, {"exact", exact},
            {"approximation", static_cast<double>(result.numerator) / static_cast<double>(result.denominator)},
            {"verificationScope", "arithmetic_only"}, {"verified", true}, {"nodeCount", visited},
            {"scoringEligible", false}};
}

} // namespace gangyi

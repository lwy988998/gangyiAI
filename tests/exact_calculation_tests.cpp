#ifdef NDEBUG
#undef NDEBUG
#endif

#include "exact_calculation.hpp"

#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>

using Json = nlohmann::json;

Json fraction(long long numerator, long long denominator) {
    return {{"numerator", numerator}, {"denominator", denominator}};
}

Json operation(const char* name, Json left, Json right) {
    return {{"op", name}, {"left", std::move(left)}, {"right", std::move(right)}};
}

int main() {
    const auto rejects = [](const Json& expression) {
        bool rejected = false;
        try { gangyi::calculateExact(expression); } catch (const std::exception&) { rejected = true; }
        if (!rejected) std::cerr << "应拒绝却被接受的输入：" << expression.dump() << '\n';
        assert(rejected);
    };
    assert(gangyi::calculateExact(operation("add", fraction(1, 2), fraction(1, 3)))["exact"] == "5/6");
    assert(gangyi::calculateExact(operation("add", "0.1", "0.2"))["exact"] == "3/10");
    assert(gangyi::calculateExact(operation("divide", fraction(3, -4), fraction(-1, 2)))["exact"] == "3/2");
    assert(gangyi::calculateExact(operation("divide", operation("subtract", 6, 2), 2))["exact"] == "2");
    assert(gangyi::calculateExact(operation("divide", 200000, 100000))["exact"] == "2");
    assert(gangyi::calculateExact(operation("multiply", fraction(1000000000000LL, 3), fraction(3, 1000000000000LL)))["exact"] == "1");
    assert(gangyi::calculateExact(fraction(0, -3))["exact"] == "0");
    assert(gangyi::calculateExact("-0.000001")["exact"] == "-1/1000000");

    // 与独立的整数交叉乘积及浮点近似双重比较，覆盖负数、空值和约分。
    for (int a = -12; a <= 12; ++a) for (int b = 1; b <= 11; ++b)
        for (int c = -8; c <= 8; ++c) for (int d = 1; d <= 7; ++d) {
            const auto result = gangyi::calculateExact(operation("add", fraction(a, b), fraction(c, d)));
            const auto numerator = result["numerator"].get<long long>();
            const auto denominator = result["denominator"].get<long long>();
            assert(numerator * (b * d) == (a * d + c * b) * denominator);
            assert(denominator > 0 && std::gcd(numerator, denominator) == 1);
            assert(std::abs(result["approximation"].get<double>() - (double(a) / b + double(c) / d)) < 1e-10);
        }

    for (const auto& expression : {Json(true), Json(0.1), Json(nullptr), Json::array(), Json("1+2"), Json("1e3"),
            Json("1."), Json(".5"), Json("+1"), Json("0.0000001"), Json("open('secret')"),
            Json(std::numeric_limits<long long>::min()), Json(std::numeric_limits<unsigned long long>::max()),
            fraction(1, 0), operation("divide", 1, 0), operation("power", 1, 2),
            operation("add", 1000000000000LL, 1), operation("multiply", 1000000000000LL, 2),
            Json{{"numerator", 1}, {"denominator", 2}, {"score", 100}}, Json{{"op", "add"}, {"left", 1}}})
        rejects(expression);
    Json deep = 1;
    for (int i = 0; i < 12; ++i) deep = operation("add", deep, 1);
    rejects(deep);
    Json wide = 1;
    for (int i = 0; i < 6; ++i) wide = operation("add", wide, wide);
    rejects(wide);
    const auto result = gangyi::calculateExact(operation("add", 1, 2));
    assert(result["verified"] == true && result["verificationScope"] == "arithmetic_only");
    assert(result["scoringEligible"] == false);
    std::cout << "精确分数、十进制、32725组交叉核验与除零/溢出/代码拒绝检查通过\n";
}

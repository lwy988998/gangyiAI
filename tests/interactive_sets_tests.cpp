#ifdef NDEBUG
#undef NDEBUG
#endif

#include "interactive_sets.hpp"
#include <cassert>
#include <functional>
#include <iostream>

using Json = nlohmann::json;

int main() {
    const auto rejects = [](const std::function<void()>& call) {
        bool denied = false;
        try { call(); } catch (const std::exception&) { denied = true; }
        assert(denied);
    };
    Json config = {{"universe", {"1", "2", "3"}}, {"setA", {"1", "2"}}, {"setB", {"2", "3"}}};
    Json state = {{"schemaVersion", 1}, {"contentVersion", 4}, {"setA", {"1", "2"}}, {"setB", {"1", "2", "3"}}};
    const auto result = gangyi::checkedSetObservation(config, state, 4);
    assert(result["intersection"] == Json({"1", "2"}));
    assert(result["scoringEligible"] == false && result["inputSource"] == "browser_report");
    auto empty = state; empty["setA"] = Json::array();
    assert(gangyi::checkedSetObservation(config, empty, 4)["intersection"].empty());
    for (int kind = 0; kind < 7; ++kind) {
        auto bad = state;
        if (kind == 0) bad["setA"] = {"1", "1"};
        if (kind == 1) bad["setA"] = {"99"};
        if (kind == 2) bad["setA"] = {1};
        if (kind == 3) bad["contentVersion"] = 3;
        if (kind == 4) bad["intersection"] = {"fake"};
        if (kind == 5) bad["score"] = 100;
        if (kind == 6) bad["schemaVersion"] = true;
        rejects([&] { gangyi::checkedSetObservation(config, bad, 4); });
    }
    for (const auto& values : {Json({" "}), Json({std::string(65, 'x')}), Json({"1", "1"})}) {
        auto bad = config; bad["universe"] = values;
        rejects([&] { gangyi::checkedSetConfig(bad); });
    }
    // 元素按文本处理，不执行其中内容；配置校验不输出任何脚本运行入口。
    auto literal = config;
    literal["universe"].push_back("<script>alert(1)</script>");
    assert(gangyi::checkedSetConfig(literal)["universe"].size() == 4);
    // 互补、并交与包含关系使用全部小集合组合验证代数性质。
    for (int maskA = 0; maskA < 16; ++maskA) for (int maskB = 0; maskB < 16; ++maskB) {
        Json a = Json::array(), b = Json::array(), universe = {"1", "2", "3", "4"};
        for (int index = 0; index < 4; ++index) {
            if (maskA & (1 << index)) a.push_back(universe[index]);
            if (maskB & (1 << index)) b.push_back(universe[index]);
        }
        const Json current = {{"schemaVersion",1},{"contentVersion",1},{"setA",a},{"setB",b}};
        const auto observed = gangyi::checkedSetObservation({{"universe",universe},{"setA",a},{"setB",b},{"operation","relations"}}, current, 1);
        assert(observed["union"].size() + observed["intersection"].size() == a.size() + b.size());
        assert(observed["complementA"].size() + a.size() == 4);
        assert(observed["complementB"].size() + b.size() == 4);
        assert(observed["relations"]["equal"] == (maskA == maskB));
        assert(observed["relations"]["aSubsetB"] == ((maskA & maskB) == maskA));
        assert(observed["relations"]["aProperSubsetB"] == ((maskA & maskB) == maskA && maskA != maskB));
    }
    auto invalidOperation = config; invalidOperation["operation"] = "execute_code";
    rejects([&] { gangyi::checkedSetConfig(invalidOperation); });
    std::cout << "集合参数、确定性交集、过期状态与探索不评分检查通过\n";
}

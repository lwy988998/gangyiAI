#include "interactive_labs.hpp"
#include <cassert>
#include <fstream>
#include <iostream>

using Json = nlohmann::json;
int main(int argc, char** argv) {
    assert(argc == 2);
    Json fixture; std::ifstream(argv[1]) >> fixture;
    for (const auto& item : fixture.at("cases")) {
        const auto result = gangyi::checkedLabObservation(item.at("config"), item.at("state"), 1);
        for (const auto& expected : item.at("expected").items()) {
            if (expected.value().is_number())
                assert(std::abs(result.at(expected.key()).get<double>() - expected.value().get<double>()) < 1e-9);
            else assert(result.at(expected.key()) == expected.value());
        }
        assert(result.at("scoringEligible") == false && result.at("kind") == "exploration");
        auto forged = item.at("state"); forged["score"] = 100;
        bool denied = false;
        try { gangyi::checkedLabObservation(item.at("config"), forged, 1); } catch (...) { denied = true; }
        assert(denied);
        denied = false;
        try { gangyi::checkedLabObservation(item.at("config"), item.at("state"), 2); } catch (...) { denied = true; }
        assert(denied);
    }
    const Json statisticsConfig = {{"type","statistics_lab"}};
    const Json statisticsState = {{"schemaVersion",1},{"contentVersion",1},{"values",{2,3,3,4,8}},{"varianceBasis","population"}};
    for (const auto& values : Json::array({Json::array(),Json(std::vector<int>(33,0)),Json::array({true}),Json::array({nullptr}),
        Json::array({"3"}),Json::array({100.5}),Json::array({-101}),Json::array({0.1})})) {
        auto config = statisticsConfig; config["values"] = values;
        auto state = statisticsState; state["values"] = values;
        bool configDenied = false, stateDenied = false;
        try { gangyi::checkedLabConfig(config); } catch (...) { configDenied = true; }
        try { gangyi::checkedLabObservation(statisticsConfig,state,1); } catch (...) { stateDenied = true; }
        assert(configDenied && stateDenied);
    }
    for (const auto& patch : Json::array({Json{{"varianceBasis","infer"}},Json{{"varianceBasis",nullptr}},Json{{"mean",4}},Json{{"schemaVersion",true}}})) {
        auto state = statisticsState; state.update(patch); bool denied = false;
        try { gangyi::checkedLabObservation(statisticsConfig,state,1); } catch (...) { denied = true; }
        assert(denied);
    }
    for (int a=-3;a<=3;++a) for (int b=-2;b<=2;++b) {
        auto state = statisticsState; state["values"] = {a,b,a+0.5};
        const auto original = gangyi::checkedLabObservation(statisticsConfig,state,1);
        state["values"] = {a+0.5,b,a};
        const auto reversed = gangyi::checkedLabObservation(statisticsConfig,state,1);
        assert(original.at("exact") == reversed.at("exact") && original.at("frequencies") == reversed.at("frequencies"));
        state["values"] = {a+2,b+2,a+2.5};
        const auto shifted = gangyi::checkedLabObservation(statisticsConfig,state,1);
        assert(std::abs(shifted.at("mean").get<double>()-original.at("mean").get<double>()-2)<1e-12);
        assert(shifted.at("exact").at("populationVariance") == original.at("exact").at("populationVariance"));
        state["values"] = {-2*a,-2*b,-2*a-1};
        const auto scaled = gangyi::checkedLabObservation(statisticsConfig,state,1);
        assert(std::abs(scaled.at("populationVariance").get<double>()-4*original.at("populationVariance").get<double>())<1e-12);
        state["values"] = {a,a,b,b,a+0.5,a+0.5};
        const auto duplicated = gangyi::checkedLabObservation(statisticsConfig,state,1);
        assert(duplicated.at("exact").at("populationVariance") == original.at("exact").at("populationVariance") && duplicated.at("count") == 6);
    }
    const Json equationState = {{"schemaVersion",1},{"contentVersion",1},{"steps",Json::array()}};
    for (const auto& steps : Json::array({Json::array({{{"op","multiply"},{"value",0}}}),
        Json::array({{{"op","divide"},{"value",0}}}),Json::array({{{"op","add_x"},{"value",true}}}),
        Json::array({{{"op","add_x"},{"value",0.5}}}),Json::array({{{"op","add_constant"},{"value",11}}}),
        Json::array({{{"op","left_only"},{"value",1}}}),Json::array({{{"op","add_x"},{"value",1},{"score",100}}}),
        Json(std::vector<Json>(13,{{"op","add_constant"},{"value",1}})),
        Json(std::vector<Json>(6,{{"op","multiply"},{"value",10}}))})) {
        auto state = equationState; state["steps"] = steps;
        bool denied = false; try { gangyi::checkedLabObservation({{"type","equation_lab"}},state,1); } catch (...) { denied = true; }
        assert(denied);
    }
    for (int a = -3; a <= 3; ++a) for (int b = -2; b <= 2; ++b) for (int c = -2; c <= 2; ++c) {
        const Json config = {{"type","equation_lab"},{"left",{a,b}},{"right",{c,1}}};
        const auto original = gangyi::checkedLabObservation(config,equationState,1);
        auto state = equationState; state["steps"] = {{{"op","add_x"},{"value",2}},{{"op","add_constant"},{"value",-3}},
            {{"op","divide"},{"value",-3}},{{"op","multiply"},{"value",-3}}};
        const auto changed = gangyi::checkedLabObservation(config,state,1);
        assert(original.at("solutionKind") == changed.at("solutionKind") && original.at("solution") == changed.at("solution"));
        if (!changed.at("solution").is_null()) {
            const auto n = changed.at("solution").at("numerator").get<long long>(), d = changed.at("solution").at("denominator").get<long long>();
            assert((a-c)*n == (1-b)*d);
        }
    }
    for (const auto& bad : Json::array({{{"vectorA",{1}}},{{"vectorA",{true,2}}},{{"vectorA",{0.1,2}}},
        {{"vectorB",nullptr}},{{"vectorB",{11,0}}},{{"scalar",3.5}},{{"operation","cross_3d"}}})) {
        auto config = bad; config["type"] = "vector_lab";
        bool denied = false; try { gangyi::checkedLabConfig(config); } catch (...) { denied = true; } assert(denied);
    }
    for (const auto& bad : Json::array({{{"lower",3},{"upper",2}},{{"lower",nullptr},{"lowerClosed",true}},
        {{"upper",nullptr},{"upperClosed",true}},{{"lowerClosed",1}},{{"point",nullptr}},{{"point",20.5}},{{"lower",0.1}}})) {
        auto config = bad; config["type"] = "number_line";
        bool denied = false; try { gangyi::checkedLabConfig(config); } catch (...) { denied = true; } assert(denied);
    }
    const Json annotation = {{"type","annotation_lab"},{"mode","selection"},{"fragments",{"甲🌱","乙，丙。"}}};
    const Json selection = {{"start",1},{"end",3},{"label",1},{"reason","文字理由"}};
    for (const auto& invalid : Json::array({{{"start",-1},{"end",3},{"label",1},{"reason",""}},
        {{"start",1},{"end",7},{"label",1},{"reason",""}},{{"start",1},{"end",1},{"label",1},{"reason",""}},
        {{"start",1},{"end",3},{"label",0},{"reason",""}},{{"start",1},{"end",3},{"label",1},{"reason",""},{"text","替换引文"}},
        {{"start",1},{"end",3},{"label",1},{"reason",std::string(481, 'x')}}})) {
        bool denied = false;
        try { gangyi::checkedLabObservation(annotation, {{"schemaVersion",1},{"contentVersion",1},{"selections",{invalid}}}, 1); }
        catch (...) { denied = true; } assert(denied);
    }
    const auto quoted = gangyi::checkedLabObservation(annotation, {{"schemaVersion",1},{"contentVersion",1},{"selections",{selection}}}, 1);
    assert(quoted.at("annotations")[0].at("text") == "🌱乙");
    // 恒加速度运动的速度位移关系与路程累积应在负速度和转向时同样成立。
    for (const double velocity : {-6.0, 0.0, 6.0}) for (const double acceleration : {-2.0, 0.0, 2.0}) {
        double previousDistance = 0;
        for (int tick = 0; tick <= 12; ++tick) {
            const Json state = {{"schemaVersion",1},{"contentVersion",1},{"initialPositionM",5},
                {"initialVelocityMps",velocity},{"accelerationMps2",acceleration},{"timeSeconds",tick * 0.5}};
            const auto result = gangyi::checkedLabObservation({{"type","motion_lab"}}, state, 1);
            const double speed = result.at("velocityMps"), displacement = result.at("displacementM"), distance = result.at("travelDistanceM");
            assert(std::abs(speed * speed - velocity * velocity - 2 * acceleration * displacement) < 1e-9);
            assert(distance + 1e-9 >= std::abs(displacement) && distance + 1e-9 >= previousDistance);
            previousDistance = distance;
        }
    }
    for (const auto& bad : Json::array({{{"type","motion_lab"},{"timeSeconds",-1}},
        {{"type","motion_lab"},{"accelerationMps2",11}},{{"type","motion_lab"},{"initialVelocityMps",true}}})) {
        bool denied = false; try { gangyi::checkedLabConfig(bad); } catch (...) { denied = true; } assert(denied);
    }
    // 学生不能通过状态更改模型指定的定义域或自报值域。
    const Json functionConfig = {{"type","function_lab"},{"domain",{{"min",2},{"max",4}}}};
    for (const auto& domain : Json::array({nullptr,Json::array(),{{"min",1},{"max",1}},{{"min",3},{"max",2}},
        {{"min",true},{"max",2}},{{"min",-21},{"max",1}},{{"min",0},{"max",2},{"open",true}}})) {
        auto bad = functionConfig; bad["domain"] = domain;
        bool denied = false; try { gangyi::checkedLabConfig(bad); } catch (...) { denied = true; } assert(denied);
    }
    for (const auto& field : {"domain", "functionAnalysis"}) {
        Json state = {{"schemaVersion",1},{"contentVersion",1},{"coefficients",{1,0,0}},{field,Json::object()}};
        bool denied = false; try { gangyi::checkedLabObservation(functionConfig, state, 1); } catch (...) { denied = true; } assert(denied);
    }
    for (const auto& bounds : Json::array({{5,10},{-5,2},{4,5}})) {
        auto invalidViewport = functionConfig; invalidViewport["xMin"] = bounds.at(0); invalidViewport["xMax"] = bounds.at(1);
        bool denied = false; try { gangyi::checkedLabConfig(invalidViewport); } catch (...) { denied = true; } assert(denied);
    }
    auto partialViewport = functionConfig; partialViewport["xMin"] = 3; partialViewport["xMax"] = 5;
    assert(gangyi::checkedLabConfig(partialViewport).at("domain") == functionConfig.at("domain"));
    // 检查整个区间的取值包络和零点残差，不把绘图视窗当作定义域。
    for (const auto& coefficients : Json::array({{1,-2,0},{-2,3,1},{0,-1,2},{0,0,3},{0.5,-1,-2}})) {
        const auto result = gangyi::checkedLabObservation({{"type","function_lab"},{"domain",{{"min",-3},{"max",4}}}},
            {{"schemaVersion",1},{"contentVersion",1},{"coefficients",coefficients}}, 1).at("functionAnalysis");
        const double a = coefficients.at(0), b = coefficients.at(1), c = coefficients.at(2);
        for (int index = 0; index <= 140; ++index) {
            const double x = -3 + index * 0.05, y = a*x*x + b*x + c;
            assert(y >= result.at("range").at("min").get<double>() - 1e-9 && y <= result.at("range").at("max").get<double>() + 1e-9);
        }
        for (const double x : result.at("zeros").at("values")) { assert(x >= -3 && x <= 4); assert(std::abs(a*x*x+b*x+c) < 1e-9); }
    }
    const auto realDomain = gangyi::checkedLabObservation({{"type","function_lab"},{"xMin",-1},{"xMax",1}},
        {{"schemaVersion",1},{"contentVersion",1},{"coefficients",{1,0,0}}},1).at("functionAnalysis");
    assert(realDomain.at("domain").at("kind") == "real" && realDomain.at("range").at("max").is_null());
    // 电阻始终为正，检查串联电压和并联电流守恒，而非只比较同一公式的副本。
    for (int r1 = 1; r1 <= 10; ++r1) for (int r2 = 1; r2 <= 10; ++r2) for (int voltage = 1; voltage <= 24; ++voltage)
        for (const auto* mode : {"series", "parallel"}) {
            const Json state = {{"schemaVersion",1},{"contentVersion",1},{"mode",mode},{"voltage",voltage},{"r1",r1},{"r2",r2}};
            const auto value = gangyi::checkedLabObservation({{"type","circuit_lab"}}, state, 1);
            if (std::string(mode) == "series")
                assert(std::abs(value.at("voltage1").get<double>() + value.at("voltage2").get<double>() - voltage) < 1e-9);
            else assert(std::abs(value.at("current1").get<double>() + value.at("current2").get<double>() - value.at("totalCurrent").get<double>()) < 1e-9);
        }
    for (const auto& bad : Json::array({{{"type","circuit_lab"},{"r1",0}}, {{"type","circuit_lab"},{"voltage",true}},
        {{"type","circuit_lab"},{"mode","script"}}, {{"type","function_lab"},{"coefficients",{1,0.1,0}}},
        {{"type","chemical_balance"},{"reactionId","unknown"}}, {{"type","chemical_balance"},{"coefficients",{0,1,1}}}})) {
        bool denied = false; try { gangyi::checkedLabConfig(bad); } catch (...) { denied = true; } assert(denied);
    }
    for (const auto& bad : Json::array({{{"type","cell_lab"},{"cellType","animal"},{"selectedPart","chloroplast"}},
        {{"type","history_lab"},{"order",{"launch","launch","return"}}}, {{"type","climate_lab"},{"month",0}},
        {{"type","annotation_lab"},{"fragments",{"a","b"}},{"marks",{1,4}}},
        {{"type","sentence_lab"},{"fragments",{"a","b"}},{"roles",{1}}}, {{"type","algorithm_lab"},{"values",{2,1}},{"step",2}}})) {
        bool denied = false; try { gangyi::checkedLabConfig(bad); } catch (...) { denied = true; } assert(denied);
    }
    // 以标准库排序和多重集不变性检查含重复值数组，不依赖前端的排序实现。
    for (int encoded = 0; encoded < 256; ++encoded) {
        std::vector<int> values; for (int index = 0; index < 4; ++index) values.push_back((encoded >> (index * 2)) & 3);
        auto sorted = values; std::sort(sorted.begin(), sorted.end()); auto previous = values;
        for (int step = 0; step <= 6; ++step) {
            const auto result = gangyi::checkedLabObservation({{"type","algorithm_lab"},{"values",values}}, {{"schemaVersion",1},{"contentVersion",1},{"step",step}}, 1);
            const auto current = result.at("array").get<std::vector<int>>(); auto multiset = current; std::sort(multiset.begin(), multiset.end()); assert(multiset == sorted);
            std::vector<int> changed; for (int i = 0; i < 4; ++i) if (current[i] != previous[i]) changed.push_back(i);
            assert(changed.empty() || (changed.size() == 2 && changed[1] == changed[0] + 1));
            previous = current; if (step == 6) assert(current == sorted && result.at("finished") == true);
        }
    }
    // 每一步保留目标的候选索引，查找结束与线性存在性核对。
    for (int length = 2; length <= 8; ++length) for (int target = 0; target <= 12; ++target) {
        std::vector<int> values;
        for (int index = 0; index < length; ++index) values.push_back(index / 2 * 2);
        const Json config = {{"type","algorithm_lab"},{"algorithm","binary_search"},{"values",values},{"target",target}};
        int total = length;
        for (int step = 0; step <= total; ++step) {
            const auto result = gangyi::checkedLabObservation(config, {{"schemaVersion",1},{"contentVersion",1},{"step",step}}, 1);
            total = result.at("totalSteps");
            assert(result.at("array") == values);
            if (result.at("finished") == true) {
                const bool exists = std::find(values.begin(), values.end(), target) != values.end();
                assert(!result.at("foundIndex").is_null() == exists);
                if (exists) assert(values.at(result.at("foundIndex").get<int>()) == target);
            } else if (std::find(values.begin(), values.end(), target) != values.end()) {
                bool retained = false;
                for (int index = result.at("low"); index <= result.at("high"); ++index)
                    retained = retained || values.at(index) == target;
                assert(retained);
            }
        }
    }
    for (const auto& bad : Json::array({{{"type","algorithm_lab"},{"algorithm","binary_search"},{"values",{3,1}},{"target",1}},
        {{"type","algorithm_lab"},{"algorithm","binary_search"}},
        {{"type","algorithm_lab"},{"algorithm","binary_search"},{"values",{1,2}},{"target",3},{"step",3}}})) {
        bool denied = false; try { gangyi::checkedLabConfig(bad); } catch (...) { denied = true; } assert(denied);
    }
    std::cout << "学科基础切片、电路守恒、冒泡排序和二分查找性质检查通过\n";
}

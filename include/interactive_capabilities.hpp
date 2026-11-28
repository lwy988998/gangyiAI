#pragma once

#include "interactive_sets.hpp"
#include "interactive_labs.hpp"

namespace gangyi {

// 从生产校验器取得默认配置，能力说明明确列出限制，避免把注册名称当作无限能力。
inline nlohmann::json interactiveCapabilities() {
    using Json = nlohmann::json;
    const Json descriptors = Json::array({
        {{"type","sets"},{"subject","数学"},{"purpose","有限集合交并补、子集、真子集与相等"},
         {"limits",{{"members",16},{"labelUtf8Bytes",64},{"operations",setOperations()}}},
         {"example",{{"universe",{"1","2","3"}},{"setA",{"1","2"}},{"setB",{"2","3"}},{"operation","relations"}}}},
        {{"type","function_lab"},{"subject","数学"},{"purpose","常数、一次、二次多项式系数、定义域、值域和零点探索"},
         {"limits",{{"coefficientMin",-10},{"coefficientMax",10},{"coefficientStep",0.5},{"viewportMin",-20},{"viewportMax",20},
            {"domain",{{"default","real"},{"optional","closed_interval"},{"min",-20},{"max",20}}},{"arbitraryExpressionSupported",false}}}},
        {{"type","wave_lab"},{"subject","物理"},{"purpose","向右传播的理想简谐横波、波长周期与固定质点振动"},
         {"limits",{{"amplitudeM",{0,2}},{"waveSpeedMps",{1,5}},{"periodSeconds",{1,4}},
            {"timeSeconds",{0,8}},{"probeXM",{0,12}},{"parameterSteps",{{"amplitudeM",0.5},{"waveSpeedMps",1},{"periodSeconds",0.5},{"timeSeconds",0.25},{"probeXM",0.5}}},
            {"wavelength","waveSpeedMps * periodSeconds"},{"playback","手动播放，暂停保存，不逐帧触发AI"},
            {"reflectionInterferenceEnergy",false}}}},
        {{"type","triangle_lab"},{"subject","数学"},{"purpose","平面三角形顶点、边长、角度和面积探索"},
         {"limits",{{"dimensions",2},{"vertexCount",3},{"coordinateRange",{-10,10}},{"coordinateStep",0.5},
            {"degenerate","顶点共线或重合时面积为零，不判断边角类型"},{"exactEquilateral",false},
            {"equilateralLimit","半整数坐标网格无法精确构造非退化等边三角形"},{"spatialGeometry",false},{"arbitraryConstruction",false}}}},
        {{"type","statistics_lab"},{"subject","数学"},{"purpose","有限单变量数据的平均数、中位数、众数及两种方差"},
         {"limits",{{"items",{1,32}},{"valueRange",{-100,100}},{"step",0.5},{"varianceBasis",{"population","sample"}},
            {"sampleVarianceMinimumCount",2},{"allUniqueModes","此处记无众数"},{"populationInference",false}}}},
        {{"type","equation_lab"},{"subject","数学"},{"purpose","一元一次方程的同解变形与精确分数"},
         {"limits",{{"sideFormat","left/right:[一次项系数,常数]"},{"integerCoefficients",{-20,20}},
            {"operations",{"add_x","add_constant","multiply","divide"}},{"integerOperand",{-10,10}},
            {"maxSteps",12},{"nonzeroScaleRequired",true},{"rationalNumeratorDenominatorMax",1000000},
            {"nonlinearEquations",false},{"inequalities",false}}}},
        {{"type","vector_lab"},{"subject","数学"},{"purpose","平面向量加减、实数倍、长度、数量积与夹角"},
         {"limits",{{"dimensions",2},{"coordinateRange",{-10,10}},{"scalarRange",{-3,3}},{"step",0.5},
            {"operations",{"add","subtract","scale_a"}},{"zeroVectorDirection","零向量的方向、夹角及方向关系不作判断"},
            {"spatialVectors",false}}}},
        {{"type","number_line"},{"subject","数学"},{"purpose","区间开闭、射线、空集、单点集与观察点归属"},
         {"limits",{{"numericRange",{-20,20}},{"step",0.5},{"unboundedEndpoint","lower为null表示负无穷，upper为null表示正无穷；无穷端不能闭合"},
            {"stateFields",{"lower","upper","lowerClosed","upperClosed","point"}},{"multipleIntervals",false}}}},
        {{"type","circuit_lab"},{"subject","物理"},{"purpose","理想两电阻串联或并联"},
         {"limits",{{"modes",{"series","parallel"}},{"voltageV",{1,24}},{"resistanceOhm",{1,100}},{"integerOnly",true}}}},
        {{"type","motion_lab"},{"subject","物理"},{"purpose","一维恒定加速度下的位置、位移、路程、速度与速率"},
         {"limits",{{"initialPositionM",{-100,100}},{"initialVelocityMps",{-20,20}},
            {"accelerationMps2",{-10,10}},{"timeSeconds",{0,20}},{"variableAcceleration",false}}}},
        {{"type","periodic_lab"},{"subject","化学"},{"purpose","118种元素选择、周期与族比较、中性原子组成及前20号电子层示意"},
         {"limits",{{"datasetId",kPeriodicElementsId},{"atomicNumber",{1,118}},{"selectionFields",{"selectedZ","comparisonZ"}},
            {"highlight",{"none","period","group"}},{"neutralShellDiagramThroughAtomicNumber",20},
            {"seriesGroup","镧系及锕系单列，group为null，不作统一同族判断"},
            {"neutrons","缺少具体同位素，保持未知"},{"ionsIsotopesBondsReactionPrediction",false}}}},
        {{"type","chemical_balance"},{"subject","化学"},{"purpose","已注册反应的原子守恒配平"},
         {"limits",{{"reactionIds",{"water","methane","iron_oxide"}},{"coefficients",{1,8}},{"arbitraryReactionSupported",false}}}},
        {{"type","cell_lab"},{"subject","生物"},{"purpose","典型真核植物/动物细胞结构功能比较"},
         {"limits",{{"cellTypes",{"plant","animal"}},{"allOrganellesSupported",false},{"toScale",false}}}},
        {{"type","history_lab"},{"subject","历史"},{"purpose","Apollo11三事件排序与日期核对"},
         {"limits",{{"datasetIds",{"apollo11"}},{"eventIds",{"launch","landing","return"}},{"generalHistorySupported",false}}}},
        {{"type","climate_lab"},{"subject","地理"},{"purpose","自编教学数据的月气温/降水比较"},
         {"limits",{{"datasetIds",{"illustrative_north"}},{"months",{1,12}},{"realStationData",false}}}},
        {{"type","annotation_lab"},{"subject","语文"},{"purpose","本课原文片段阅读标注"},
         {"limits",{{"fragmentCount",{2,16}},{"fragmentUtf8Bytes",240},{"totalUtf8Bytes",2400},{"markCodes",{0,1,2,3}},
            {"selectionMode",{{"maxSelections",16},{"reasonUtf8Bytes",480},{"offsetUnit","Unicode code points"},{"range","start含端点，end不含端点，按拼接后的原文索引"}}},{"uniqueAnswer",false}}},
         {"example",{{"fragments",{"本课已有的第一段原文。","本课已有的第二段原文。"}}}}},
        {{"type","sentence_lab"},{"subject","英语"},{"purpose","本课真实句子片段成分标注"},
         {"limits",{{"fragmentCount",{2,16}},{"fragmentUtf8Bytes",240},{"totalUtf8Bytes",2400},{"roleCodes",{0,1,2,3,4}},{"audioSupported",false}}},
         {"example",{{"fragments",{"The students","read books."}}}}},
        {{"type","algorithm_lab"},{"subject","计算机"},{"purpose","升序冒泡排序与有序数组二分查找步进"},
         {"limits",{{"algorithms",{"bubble_sort","binary_search"}},{"length",{2,8}},{"integerValues",{0,99}},
            {"binarySearch",{{"sortedRequired",true},{"target",{0,99}},{"duplicates","返回本次命中索引，不保证首个"}}},{"arbitraryExecution",false}}}}
    });
    Json result = Json::array();
    for (auto item : descriptors) {
        const auto type = item.at("type").get<std::string>();
        auto example = item.value("example", Json::object()); example["type"] = type;
        if (type == "sets") {
            item["example"] = checkedSetConfig(example); item["example"]["type"] = "sets";
            item["checkedExercise"] = {{"tool", "append_section"}, {"field", "section.question.setExercise"}, {"schemaVersion", 1},
                {"description", u8"单选题从全集和子集生成题面及私有答案。提供 config 或同课已保存的 mediaId。运算题提供2至6个候选集合，关系题由程序生成互斥选项。探索仍不评分。"}};
        } else item["example"] = checkedLabConfig(example);
        item["schemaVersion"] = 1;
        item["activityReading"] = {{"default","componentActivities摘要"},{"detailsTool","read_component_activity"},{"pageLimit",50}};
        item["scoringEligible"] = false;
        item["recovery"] = "按课时、组件、内容版本保存合法状态；版本变化时不声称完整恢复";
        result.push_back(std::move(item));
    }
    return {{"schemaVersion",1},{"revision","2026-11-28-r9"},{"components",result},
        {"unsupportedBehavior","使用文字回退并说明限制，不生成或执行任意组件代码"}};
}

}

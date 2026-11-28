#pragma once

#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace gangyi {

inline bool isSubjectLab(const std::string& type) {
    static const std::set<std::string> types = {"cell_lab", "history_lab", "climate_lab", "annotation_lab", "sentence_lab", "algorithm_lab"};
    return types.count(type) != 0;
}

inline int subjectInteger(const nlohmann::json& value, int low, int high) {
    if (!value.is_number_integer() || value < low || value > high) throw std::invalid_argument("学科互动参数须为范围内整数");
    return value.get<int>();
}

inline std::vector<std::string> cellParts(const std::string& type) {
    auto parts = std::vector<std::string>{"membrane", "cytoplasm", "nucleus", "mitochondrion"};
    if (type == "plant") parts.insert(parts.end(), {"wall", "chloroplast", "vacuole"});
    return parts;
}

inline const nlohmann::json& cellFacts() {
    // 自编简述，依据 OpenStax Biology 2e 4.3；不是按比例的完整细胞模型。
    static const nlohmann::json facts = {
        {"membrane", {"细胞膜", "选择性地调节物质进出；植物细胞也有细胞膜。"}},
        {"cytoplasm", {"细胞质", "细胞膜以内、细胞核以外的区域，包含细胞质基质与多种细胞器。"}},
        {"nucleus", {"细胞核", "含有大部分遗传物质；此处展示有细胞核的典型真核细胞。"}},
        {"mitochondrion", {"线粒体", "参与细胞呼吸和ATP生成；植物细胞也有线粒体。"}},
        {"wall", {"细胞壁", "位于细胞膜外，提供支持与保护；典型植物细胞具有细胞壁。"}},
        {"chloroplast", {"叶绿体", "在绿色植物的光合细胞中参与光合作用；不是所有植物细胞都有叶绿体。"}},
        {"vacuole", {"中央液泡", "成熟植物细胞中常见的大型液泡参与储存及维持膨压。"}}
    };
    return facts;
}

inline const nlohmann::json& historyFacts() {
    // 日期按 NASA Apollo 11 任务资料核对；时间先后不等于因果证明。
    static const nlohmann::json facts = {
        {"launch", {{"label", "阿波罗11号发射"}, {"date", "1969-07-16"}}},
        {"landing", {{"label", "登月舱着陆月面"}, {"date", "1969-07-20"}}},
        {"return", {{"label", "指令舱返回地球溅落"}, {"date", "1969-07-24"}}}
    };
    return facts;
}

inline nlohmann::json subjectFragments(const nlohmann::json& value) {
    if (!value.is_array() || value.size() < 2 || value.size() > 16) throw std::invalid_argument("文本互动需要2至16个片段");
    std::size_t total = 0;
    for (const auto& item : value) {
        if (!item.is_string()) throw std::invalid_argument("文本片段须为文字");
        const auto text = item.get<std::string>(); total += text.size();
        if (text.empty() || text.size() > 240 || text.find_first_not_of(" \t\r\n") == std::string::npos)
            throw std::invalid_argument("文本片段为空或超过240字节");
    }
    if (total > 2400) throw std::invalid_argument("文本互动总长度超过2400字节");
    return value;
}

inline std::string annotationPassage(const nlohmann::json& fragments) {
    std::string text;
    for (const auto& fragment : fragments) text += fragment.get<std::string>();
    return text;
}

inline std::vector<std::size_t> annotationOffsets(const std::string& text) {
    // 选区采用 Unicode 码点索引，避免中文和 emoji 的 UTF-8 字节偏移不一致。
    nlohmann::json(text).dump();
    std::vector<std::size_t> offsets;
    for (std::size_t i = 0; i < text.size(); ++i)
        if ((static_cast<unsigned char>(text[i]) & 0xc0) != 0x80) offsets.push_back(i);
    offsets.push_back(text.size());
    return offsets;
}

inline nlohmann::json checkedAnnotationSelections(const nlohmann::json& fragments, const nlohmann::json& selections) {
    const auto offsets = annotationOffsets(annotationPassage(fragments));
    if (!selections.is_array() || selections.size() > 16) throw std::invalid_argument("原文选区最多16条");
    for (const auto& item : selections) {
        if (!item.is_object() || item.size() != 4 || !item.contains("start") || !item.contains("end") ||
            !item.contains("label") || !item.value("reason", nlohmann::json()).is_string())
            throw std::invalid_argument("选区只接受起止位置、类别和阅读理由");
        const int start = subjectInteger(item.at("start"), 0, int(offsets.size()) - 2);
        const int end = subjectInteger(item.at("end"), start + 1, int(offsets.size()) - 1);
        (void)end;
        subjectInteger(item.at("label"), 1, 3);
        if (item.at("reason").get<std::string>().size() > 480) throw std::invalid_argument("标注理由超过480字节");
    }
    return selections;
}

inline nlohmann::json binarySearchSteps(const std::vector<int>& values, int target) {
    using Json = nlohmann::json;
    int low = 0, high = int(values.size()) - 1;
    Json steps = Json::array({{{"low",low},{"high",high},{"comparedIndices",Json::array()},
        {"foundIndex",nullptr},{"finished",false}}});
    while (low <= high) {
        const int middle = low + (high - low) / 2;
        const bool found = values[middle] == target;
        if (!found) {
            if (values[middle] < target) low = middle + 1;
            else high = middle - 1;
        }
        steps.push_back({{"low",low},{"high",high},{"comparedIndices",{middle}},
            {"foundIndex",found ? Json(middle) : Json(nullptr)},{"finished",found || low > high}});
        if (found) break;
    }
    return steps;
}

inline nlohmann::json checkedSubjectConfig(const nlohmann::json& source) {
    using Json = nlohmann::json;
    const auto type = source.value("type", "");
    if (!isSubjectLab(type) || source.value("schemaVersion", Json(1)) != 1) throw std::invalid_argument("学科互动类型或协议版本无效");
    Json result = {{"type", type}, {"schemaVersion", 1}};
    if (type == "cell_lab") {
        const auto cellType = source.value("cellType", "plant");
        if (cellType != "plant" && cellType != "animal") throw std::invalid_argument("只支持典型植物或动物真核细胞");
        const auto part = source.value("selectedPart", "nucleus");
        const auto parts = cellParts(cellType);
        if (std::find(parts.begin(), parts.end(), part) == parts.end()) throw std::invalid_argument("此细胞示意中没有该结构");
        result["cellType"] = cellType; result["selectedPart"] = part;
    } else if (type == "history_lab") {
        if (source.value("datasetId", "apollo11") != "apollo11") throw std::invalid_argument("历史资料集尚未核对");
        result["datasetId"] = "apollo11";
        const auto order = source.value("order", Json::array({"landing", "return", "launch"}));
        if (!order.is_array() || order.size() != 3) throw std::invalid_argument("事件顺序须包含三个已核对事件");
        std::set<std::string> seen;
        for (const auto& id : order) if (!id.is_string() || !historyFacts().contains(id.get<std::string>()) || !seen.insert(id.get<std::string>()).second)
            throw std::invalid_argument("历史事件重复或未知");
        const auto selected = source.value("selectedEvent", "landing");
        if (!seen.count(selected)) throw std::invalid_argument("所选事件不在当前资料中");
        result["order"] = order; result["selectedEvent"] = selected;
    } else if (type == "climate_lab") {
        if (source.value("datasetId", "illustrative_north") != "illustrative_north") throw std::invalid_argument("气候数据集尚未注册");
        result["datasetId"] = "illustrative_north";
        result["month"] = subjectInteger(source.value("month", Json(1)), 1, 12);
        result["compareMonth"] = subjectInteger(source.value("compareMonth", Json(7)), 1, 12);
    } else if (type == "annotation_lab" || type == "sentence_lab") {
        result["fragments"] = subjectFragments(source.at("fragments"));
        const auto key = type == "annotation_lab" ? "marks" : "roles";
        const int maximum = type == "annotation_lab" ? 3 : 4;
        const auto values = source.value(key, Json(std::vector<int>(result.at("fragments").size(), 0)));
        if (!values.is_array() || values.size() != result.at("fragments").size()) throw std::invalid_argument("标注数量须与原文片段一致");
        for (const auto& value : values) subjectInteger(value, 0, maximum);
        result[key] = values;
        if (type == "annotation_lab") {
            const auto mode = source.value("mode", "fragments");
            if (mode == "selection") {
                result["mode"] = mode;
                result["selections"] = checkedAnnotationSelections(result.at("fragments"), source.value("selections", Json::array()));
            } else if (mode != "fragments") throw std::invalid_argument("阅读标注模式无效");
        }
    } else {
        const auto values = source.value("values", Json::array({5, 2, 4, 1}));
        if (!values.is_array() || values.size() < 2 || values.size() > 8) throw std::invalid_argument("算法数组需要2至8项");
        for (const auto& value : values) subjectInteger(value, 0, 99);
        result["values"] = values;
        const auto algorithm = source.value("algorithm", "bubble_sort");
        int maximum = int(values.size() * (values.size() - 1) / 2);
        if (algorithm == "binary_search") {
            const auto ordered = values.get<std::vector<int>>();
            if (!std::is_sorted(ordered.begin(), ordered.end())) throw std::invalid_argument("二分查找需要升序排列的数组");
            result["algorithm"] = algorithm;
            result["target"] = subjectInteger(source.at("target"), 0, 99);
            maximum = int(binarySearchSteps(ordered, result.at("target")).size()) - 1;
        } else if (algorithm != "bubble_sort") throw std::invalid_argument("算法尚未注册");
        result["step"] = subjectInteger(source.value("step", Json(0)), 0, maximum);
    }
    return result;
}

inline std::set<std::string> subjectStateFields(const std::string& type) {
    if (type == "cell_lab") return {"cellType", "selectedPart"};
    if (type == "history_lab") return {"order", "selectedEvent"};
    if (type == "climate_lab") return {"month", "compareMonth"};
    if (type == "annotation_lab") return {"marks"};
    if (type == "sentence_lab") return {"roles"};
    return {"step"};
}

inline nlohmann::json checkedSubjectObservation(const nlohmann::json& source, const nlohmann::json& state, int version) {
    using Json = nlohmann::json;
    const auto config = checkedSubjectConfig(source);
    const auto type = config.at("type").get<std::string>();
    if (version < 1 || !state.is_object() || !state.value("schemaVersion", Json()).is_number_integer() || state.at("schemaVersion") != 1 ||
        !state.value("contentVersion", Json()).is_number_integer() || state.at("contentVersion") != version)
        throw std::invalid_argument("互动状态已过期，请重新探索当前内容");
    const auto fields = config.value("mode", "") == "selection" ? std::set<std::string>{"selections"} : subjectStateFields(type);
    for (const auto& item : state.items()) if (item.key() != "schemaVersion" && item.key() != "contentVersion" && !fields.count(item.key()))
        throw std::invalid_argument("不能上报额外结果、文本、分数或配置");
    auto input = config;
    for (const auto& field : fields) {
        if (!state.contains(field)) throw std::invalid_argument("互动状态缺少操作参数");
        input[field] = state.at(field);
    }
    const auto checked = checkedSubjectConfig(input);
    Json result = state;
    result["kind"] = "exploration"; result["scoringEligible"] = false;
    result["inputSource"] = "browser_report"; result["resultSource"] = "program_calculation";
    if (type == "cell_lab") {
        result["part"] = cellFacts().at(checked.at("selectedPart").get<std::string>());
        result["availableParts"] = cellParts(checked.at("cellType"));
    } else if (type == "history_lab") {
        Json dates = Json::array();
        for (const auto& id : checked.at("order")) dates.push_back(historyFacts().at(id.get<std::string>()).at("date"));
        result["dates"] = dates; result["chronological"] = dates[0] < dates[1] && dates[1] < dates[2];
        result["selectedFact"] = historyFacts().at(checked.at("selectedEvent").get<std::string>());
        result["sourceUrl"] = "https://www.nasa.gov/mission/apollo-11/";
    } else if (type == "climate_lab") {
        static const int temperatures[] = {2,4,8,14,19,23,26,25,21,15,9,4};
        static const int rainfall[] = {30,35,50,75,110,150,180,160,100,65,40,25};
        const int index = checked.at("month").get<int>() - 1, compare = checked.at("compareMonth").get<int>() - 1;
        result["temperatureC"] = temperatures[index]; result["precipitationMm"] = rainfall[index];
        result["temperatureDifferenceC"] = temperatures[index] - temperatures[compare];
        result["precipitationDifferenceMm"] = rainfall[index] - rainfall[compare];
        result["dataKind"] = "illustrative_not_observed";
    } else if (type == "annotation_lab" || type == "sentence_lab") {
        const std::vector<std::string> labels = type == "annotation_lab" ? std::vector<std::string>{"未标注", "意象", "表达", "疑问"} :
            std::vector<std::string>{"未标注", "主语", "谓语动词", "宾语", "修饰或状语"};
        const auto key = type == "annotation_lab" ? "marks" : "roles";
        result["annotations"] = Json::array();
        if (config.value("mode", "") == "selection") {
            const auto text = annotationPassage(config.at("fragments"));
            const auto offsets = annotationOffsets(text);
            for (const auto& item : checked.at("selections")) {
                const auto start = item.at("start").get<int>(), end = item.at("end").get<int>();
                result["annotations"].push_back({{"start",start},{"end",end},
                    {"text",text.substr(offsets[start], offsets[end] - offsets[start])},
                    {"label",labels.at(item.at("label").get<int>())},{"reason",item.at("reason")}});
            }
            result["interpretation"] = "student_annotation_not_correctness";
            return result;
        }
        for (std::size_t i = 0; i < checked.at(key).size(); ++i) if (checked.at(key).at(i) != 0)
            result["annotations"].push_back({{"index", i}, {"text", config.at("fragments").at(i)}, {"label", labels.at(checked.at(key).at(i).get<int>())}});
        result["interpretation"] = "student_annotation_not_correctness";
    } else if (config.value("algorithm", "") == "binary_search") {
        const auto steps = binarySearchSteps(config.at("values").get<std::vector<int>>(), config.at("target"));
        result.update(steps.at(checked.at("step").get<int>()));
        result["array"] = config.at("values"); result["target"] = config.at("target");
        result["totalSteps"] = steps.size() - 1; result["algorithm"] = "binary_search_ascending";
    } else {
        // 从保存数组重放冒泡排序，浏览器只能报告步数，不能自报排序结果或执行代码。
        auto values = config.at("values").get<std::vector<int>>();
        const int step = checked.at("step"); int count = 0;
        result["comparedIndices"] = Json::array(); result["swapped"] = false;
        for (int end = int(values.size()) - 1; end > 0; --end) for (int index = 0; index < end; ++index) {
            if (count++ >= step) continue;
            const bool swap = values[index] > values[index+1];
            if (swap) std::swap(values[index], values[index+1]);
            result["comparedIndices"] = {index, index+1}; result["swapped"] = swap;
        }
        result["array"] = values; result["totalSteps"] = count; result["finished"] = step == count;
        result["algorithm"] = "bubble_sort_ascending";
    }
    return result;
}

} // namespace gangyi

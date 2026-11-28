#pragma once

#include "interactive_sets.hpp"

#include <algorithm>
#include <cstdint>
#include <map>

namespace gangyi {

inline int checkedSetOption(const nlohmann::json& question, const nlohmann::json& selected) {
    const auto count = question.at("options").size();
    if (!selected.is_number_integer() || (selected.is_number_unsigned()
        ? selected.get<std::uint64_t>() >= count
        : selected.get<std::int64_t>() < 0 || static_cast<std::uint64_t>(selected.get<std::int64_t>()) >= count))
        throw std::invalid_argument("提交的选项不存在，请核对当前题目后重新选择");
    return selected.get<int>();
}

inline nlohmann::json checkedSetExerciseQuestion(const nlohmann::json& question,
                                                const nlohmann::json& deepContent) {
    using Json = nlohmann::json;
    for (const auto& field : question.items()) {
        if (field.key() != "setExercise" && (field.key() != "type" || field.value() != "single_choice"))
            throw std::invalid_argument("集合题使用 setExercise 自动生成题干、选项和评分依据");
    }
    const auto& exercise = question.at("setExercise");
    if (!exercise.is_object() || !exercise.value("schemaVersion", Json()).is_number_integer() ||
        exercise.at("schemaVersion") != 1 || exercise.contains("config") == exercise.contains("mediaId"))
        throw std::invalid_argument("集合题须使用 schemaVersion=1，并提供 config 或同课 mediaId");
    for (const auto& field : exercise.items())
        if (field.key() != "schemaVersion" && field.key() != "config" && field.key() != "mediaId" && field.key() != "options")
            throw std::invalid_argument("集合题包含未支持的字段");

    Json config, origin = nullptr;
    if (exercise.contains("config")) {
        const auto& supplied = exercise.at("config");
        if (!supplied.is_object() || !supplied.value("operation", Json()).is_string())
            throw std::invalid_argument("集合题 config 须指定运算和全集、集合 A、集合 B");
        for (const auto& field : supplied.items())
            if (field.key() != "operation" && field.key() != "universe" && field.key() != "setA" && field.key() != "setB")
                throw std::invalid_argument("集合题 config 包含未支持的字段");
        config = checkedSetConfig(supplied);
    } else {
        const auto mediaId = exercise.value("mediaId", "");
        const auto media = deepContent.value("media", Json::array());
        for (const auto& item : media) {
            if (mediaId.empty() || !item.is_object() || item.value("id", "") != mediaId ||
                item.value("type", "") != "sets" || item.value("status", "") != "ready") continue;
            config = checkedSetConfig(item);
            origin = {{"mediaId", mediaId}, {"contentVersion", deepContent.at("version")}};
            break;
        }
        if (config.is_null()) throw std::invalid_argument("集合题只能引用本课已保存且可用的集合组件");
    }

    const auto& universe = config.at("universe");
    for (const auto& value : universe) {
        const auto& label = value.get_ref<const std::string&>();
        if (label.find_first_of("{},\"\\") != std::string::npos ||
            std::any_of(label.begin(), label.end(), [](unsigned char byte) { return byte <= ' ' || byte == 0x7f; }))
            throw std::invalid_argument("集合题元素须为不含 ASCII 空白、花括号、逗号、引号或反斜杠的清晰标签");
    }
    const auto displaySet = [](const Json& values) {
        if (values.empty()) return std::string(u8"∅");
        const std::set<std::string> members(values.begin(), values.end());
        std::string result = "{";
        for (const auto& label : members) {
            if (result.size() > 1) result += ", ";
            result += label;
        }
        return result + "}";
    };
    const auto savedState = Json{{"schemaVersion", 1}, {"contentVersion", 1}, {"setA", config.at("setA")}, {"setB", config.at("setB")}};
    const auto result = checkedSetObservation(config, savedState, 1);
    const auto operation = config.at("operation").get<std::string>();
    const auto descriptor = "U = " + displaySet(universe) + ", A = " + displaySet(config.at("setA")) + ", B = " + displaySet(config.at("setB"));
    std::string stem;
    Json options = Json::array();
    int answerIndex = -1;
    if (operation == "relations") {
        if (exercise.contains("options")) throw std::invalid_argument("集合关系题由程序生成互斥选项");
        stem = u8"已知 " + descriptor + u8"。下列哪一项准确描述 A 与 B 的关系？";
        options = {u8"A 与 B 相等", u8"A 是 B 的真子集", u8"B 是 A 的真子集", u8"A 与 B 互不包含"};
        const auto& relations = result.at("relations");
        if (relations.at("equal") == true) answerIndex = 0;
        else if (relations.at("aProperSubsetB") == true) answerIndex = 1;
        else if (relations.at("bProperSubsetA") == true) answerIndex = 2;
        else answerIndex = 3;
    } else {
        static const std::map<std::string, std::pair<std::string, std::string>> operations = {
            {"intersection", {u8"交集 A ∩ B", "intersection"}}, {"union", {u8"并集 A ∪ B", "union"}},
            {"complement_a", {u8"A 相对于 U 的补集", "complementA"}}, {"complement_b", {u8"B 相对于 U 的补集", "complementB"}}
        };
        const auto& [label, field] = operations.at(operation);
        stem = u8"已知 " + descriptor + u8"。" + label + u8"是什么？";
        const auto candidates = exercise.value("options", Json());
        if (!candidates.is_array() || candidates.size() < 2 || candidates.size() > 6)
            throw std::invalid_argument("集合运算题需提供 2 到 6 个候选集合");
        const auto answer = displaySet(result.at(field));
        const std::set<std::string> allowed(universe.begin(), universe.end());
        std::set<std::string> distinct;
        for (const auto& candidate : candidates) {
            const auto option = displaySet(checkedSetMembers(candidate, &allowed));
            if (!distinct.insert(option).second) throw std::invalid_argument("集合运算题选项不能重复，调换元素顺序仍是同一个集合");
            if (option == answer) answerIndex = static_cast<int>(options.size());
            options.push_back(option);
        }
        if (answerIndex < 0) throw std::invalid_argument("集合运算题选项没有包含程序核验的正确结果");
    }
    return {{"question", stem}, {"type", "single_choice"}, {"options", options}, {"answerIndex", answerIndex},
            {"expectedAnswer", options.at(answerIndex)},
            {"rubric", u8"按学生明确提交的 selectedOption 核对选项，从0开始，对照私有 answerIndex。选对记100，选错记0，理由中的理解或误区另行诊断。没有明确选择时先澄清，不能猜选项。探索中的临时集合不替代题面条件。"},
            {"setExercise", {{"schemaVersion", 1}, {"config", config}, {"origin", origin}}}};
}

} // namespace gangyi

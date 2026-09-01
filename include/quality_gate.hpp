#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace gangyi {

struct QualityResult {
    bool valid = false;
    int score = 0;
    std::vector<std::string> reasons;  // 未通过的致命/模块级原因（中文，用于日志）
};

// 校验 MockPlan 形状课程内容（对齐校园版 lib/courseContentQuality.ts 的
// validateUserVisibleCourseContent）：泛化句黑名单、阶段数（lite 3-5 / deep 4-6）、
// 关键词相关性、重复度、动作/产出词正则；valid = 无 fatal 且 score >= 55。
// 调用方在课程保存前与展示前使用；不通过则拒绝保存/展示。
QualityResult validateCourseContent(const nlohmann::json& plan, const std::string& goal,
                                    const std::string& mode, const std::string& courseTitle);

}  // namespace gangyi

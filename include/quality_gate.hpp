// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 声明数据结构或类型 QualityResult 及其成员接口。
struct QualityResult {
    // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
    bool valid = false;
    // 【逐行批注】 计算右侧结果并写入 int score，更新当前运行状态。
    int score = 0;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> reasons; // 未通过的致命/模块级原因（中文，用于日志）。
    std::vector<std::string> reasons;  // 未通过的致命/模块级原因（中文，用于日志）
// 【逐行批注】 执行当前声明或表达式：};。
};

// 校验 MockPlan 形状课程内容（对齐校园版 lib/courseContentQuality.ts 的
// validateUserVisibleCourseContent）：泛化句黑名单、阶段数（lite 3-5 / deep 4-6）、
// 关键词相关性、重复度、动作/产出词正则；valid = 无 fatal 且 score >= 55。
// 调用方在课程保存前与展示前使用；不通过则拒绝保存/展示。
// 【逐行批注】 声明对象 validateCourseContent，并通过构造函数初始化其状态。
QualityResult validateCourseContent(const nlohmann::json& plan, const std::string& goal,
                                    // 【逐行批注】 执行当前声明或表达式：const std::string& mode, const std::string& courseTitle);。
                                    const std::string& mode, const std::string& courseTitle);

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

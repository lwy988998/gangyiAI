// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 <cstddef>，为后续实现提供类型或接口声明。
#include <cstddef>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 定义函数 truncateUtf8，执行该函数声明的职责。
inline std::string truncateUtf8(const std::string& value, size_t maxBytes) {
    // 【逐行批注】 计算右侧结果并写入 size_t end，更新当前运行状态。
    size_t end = 0;
    // 【逐行批注】 在条件成立期间循环处理：while (end < value.size() && end < maxBytes) {。
    while (end < value.size() && end < maxBytes) {
        // 【逐行批注】 计算右侧结果并写入 unsigned char lead，更新当前运行状态。
        const unsigned char lead = static_cast<unsigned char>(value[end]);
        // 【逐行批注】 计算右侧结果并写入 size_t width，更新当前运行状态。
        size_t width = 0;
        // 【逐行批注】 判断条件 (lead <= 0x7Fu) width = 1;，成立时执行对应分支。
        if (lead <= 0x7Fu) width = 1;
        // 【逐行批注】 在前一条件不成立时继续判断：(lead >= 0xC2u && lead <= 0xDFu) width = 2;。
        else if (lead >= 0xC2u && lead <= 0xDFu) width = 2;
        // 【逐行批注】 在前一条件不成立时继续判断：(lead >= 0xE0u && lead <= 0xEFu) width = 3;。
        else if (lead >= 0xE0u && lead <= 0xEFu) width = 3;
        // 【逐行批注】 在前一条件不成立时继续判断：(lead >= 0xF0u && lead <= 0xF4u) width = 4;。
        else if (lead >= 0xF0u && lead <= 0xF4u) width = 4;
        // 【逐行批注】 执行前一条件不成立时的分支。
        else break;
        // 【逐行批注】 判断条件 (end + width > maxBytes || end + width > value.size()) break;，成立时执行对应分支。
        if (end + width > maxBytes || end + width > value.size()) break;

        // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
        bool valid = true;
        // 【逐行批注】 按循环规则重复处理：for (size_t index = 1; index < width; ++index) {。
        for (size_t index = 1; index < width; ++index) {
            // 【逐行批注】 计算右侧结果并写入 unsigned char continuation，更新当前运行状态。
            const unsigned char continuation = static_cast<unsigned char>(value[end + index]);
            // 【逐行批注】 判断条件 ((continuation & 0xC0u) != 0x80u) {，成立时执行对应分支。
            if ((continuation & 0xC0u) != 0x80u) {
                // 【逐行批注】 计算右侧结果并写入 valid，更新当前运行状态。
                valid = false;
                // 【逐行批注】 跳出当前循环或分支。
                break;
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (!valid) break;，成立时执行对应分支。
        if (!valid) break;

        // 【逐行批注】 计算右侧结果并写入 unsigned char second，更新当前运行状态。
        const unsigned char second = width > 1 ? static_cast<unsigned char>(value[end + 1]) : 0;
        // 【逐行批注】 判断条件 ((lead == 0xE0u && second < 0xA0u) ||，成立时执行对应分支。
        if ((lead == 0xE0u && second < 0xA0u) ||
            // 【逐行批注】 执行当前声明或表达式：(lead == 0xEDu && second > 0x9Fu) ||。
            (lead == 0xEDu && second > 0x9Fu) ||
            // 【逐行批注】 执行当前声明或表达式：(lead == 0xF0u && second < 0x90u) ||。
            (lead == 0xF0u && second < 0x90u) ||
            // 【逐行批注】 执行当前声明或表达式：(lead == 0xF4u && second > 0x8Fu)) break;。
            (lead == 0xF4u && second > 0x8Fu)) break;
        // 【逐行批注】 计算右侧结果并写入 end +，更新当前运行状态。
        end += width;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return value.substr(0, end);。
    return value.substr(0, end);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

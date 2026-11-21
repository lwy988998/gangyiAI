// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 处理条件编译指令：#ifdef GANGYI_OFFLINE_CANDIDATE。
#ifdef GANGYI_OFFLINE_CANDIDATE
// 【逐行批注】 计算右侧结果并写入 inline constexpr const char* kVersion，更新当前运行状态。
inline constexpr const char* kVersion = "v5.7.2 离线候选";
// 【逐行批注】 处理条件编译指令：#elif defined(GANGYI_TEACHING_PREVIEW)。
#elif defined(GANGYI_TEACHING_PREVIEW)
// 【逐行批注】 计算右侧结果并写入 inline constexpr const char* kVersion，更新当前运行状态。
inline constexpr const char* kVersion = "v5.7.2 教学试用1";
// 【逐行批注】 处理条件编译指令：#else。
#else
// 【逐行批注】 计算右侧结果并写入 inline constexpr const char* kVersion，更新当前运行状态。
inline constexpr const char* kVersion = "v5.7.2";
// 【逐行批注】 处理条件编译指令：#endif。
#endif

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

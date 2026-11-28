// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 计算右侧结果并写入 constexpr int kDatabaseSchemaVersion，更新当前运行状态。
constexpr int kDatabaseSchemaVersion = 9;

// 【逐行批注】 结束当前代码块。
}

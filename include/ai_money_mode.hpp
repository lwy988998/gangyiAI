#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <cstdlib>
#endif

namespace gangyi {
// 两类发送路径检测同一个实验预算设置。
inline bool aiMoneyBudgetGuarded() {
#ifdef _WIN32
    wchar_t required[2]{};
    return GetEnvironmentVariableW(L"GANGYI_MONEY_CONFIG", nullptr, 0) > 0 ||
        GetEnvironmentVariableW(L"GANGYI_MONEY_CUMULATIVE_LIMIT_MICROS", nullptr, 0) > 0 ||
        (GetEnvironmentVariableW(L"GANGYI_MONEY_REQUIRED", required, 2) == 1 && required[0] == L'1');
#else
    const auto* path = std::getenv("GANGYI_MONEY_CONFIG");
    const auto* required = std::getenv("GANGYI_MONEY_REQUIRED");
    const auto* scopedLimit = std::getenv("GANGYI_MONEY_CUMULATIVE_LIMIT_MICROS");
    return (path && *path) || (scopedLimit && *scopedLimit) || (required && required[0] == '1' && required[1] == '\0');
#endif
}
}

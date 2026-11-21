// 【逐行批注】 引入头文件 "search_client.hpp"，为后续实现提供类型或接口声明。
#include "search_client.hpp"
#include "ai_money_mode.hpp"

// 【逐行批注】 引入头文件 <curl/curl.h>，为后续实现提供类型或接口声明。
#include <curl/curl.h>
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <array>，为后续实现提供类型或接口声明。
#include <array>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <cmath>，为后续实现提供类型或接口声明。
#include <cmath>
// 【逐行批注】 引入头文件 <cstdint>，为后续实现提供类型或接口声明。
#include <cstdint>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>
// 【逐行批注】 引入头文件 <future>，为后续实现提供类型或接口声明。
#include <future>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <limits>，为后续实现提供类型或接口声明。
#include <limits>
// 【逐行批注】 引入头文件 <map>，为后续实现提供类型或接口声明。
#include <map>
// 【逐行批注】 引入头文件 <memory>，为后续实现提供类型或接口声明。
#include <memory>
// 【逐行批注】 引入头文件 <mutex>，为后续实现提供类型或接口声明。
#include <mutex>
// 【逐行批注】 引入头文件 <optional>，为后续实现提供类型或接口声明。
#include <optional>
// 【逐行批注】 引入头文件 <set>，为后续实现提供类型或接口声明。
#include <set>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <ctime>，为后续实现提供类型或接口声明。
#include <ctime>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
// 【逐行批注】 处理条件编译指令：#ifndef NOMINMAX。
#ifndef NOMINMAX
// 【逐行批注】 定义编译期常量或宏：#define NOMINMAX。
#define NOMINMAX
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 引入头文件 <windows.h>，为后续实现提供类型或接口声明。
#include <windows.h>
// 【逐行批注】 处理条件编译指令：#else。
#else
// 【逐行批注】 引入头文件 <fcntl.h>，为后续实现提供类型或接口声明。
#include <fcntl.h>
// 【逐行批注】 引入头文件 <sys/file.h>，为后续实现提供类型或接口声明。
#include <sys/file.h>
// 【逐行批注】 引入头文件 <unistd.h>，为后续实现提供类型或接口声明。
#include <unistd.h>
// 【逐行批注】 处理条件编译指令：#endif。
#endif

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 using json，更新当前运行状态。
using json = nlohmann::json;
// 【逐行批注】 声明或进入命名空间：namespace fs = std::filesystem;。
namespace fs = std::filesystem;

// 【逐行批注】 声明对象 env，并通过构造函数初始化其状态。
std::string env(const char* name, const std::string& fallback = {}) {
    // 【逐行批注】 计算右侧结果并写入 char* value，更新当前运行状态。
    const char* value = std::getenv(name);
    // 【逐行批注】 结束当前函数并返回结果：return value ? value : fallback;。
    return value ? value : fallback;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 env_ll，执行该函数声明的职责。
long long env_ll(const char* name, long long fallback) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 std::string value，更新当前运行状态。
        const std::string value = env(name, std::to_string(fallback));
        // 【逐行批注】 计算右侧结果并写入 size_t pos，更新当前运行状态。
        size_t pos = 0;
        // 【逐行批注】 计算右侧结果并写入 long long parsed，更新当前运行状态。
        const long long parsed = std::stoll(value, &pos);
        // 【逐行批注】 结束当前函数并返回结果：return pos == value.size() ? parsed : fallback;。
        return pos == value.size() ? parsed : fallback;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return fallback;。
        return fallback;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 writeBody，执行该函数声明的职责。
size_t writeBody(char* data, size_t size, size_t count, void* user) {
    // 【逐行批注】 调用 append 完成本行所需操作：static_cast<std::string*>(user)->append(data, size * count);。
    static_cast<std::string*>(user)->append(data, size * count);
    // 【逐行批注】 结束当前函数并返回结果：return size * count;。
    return size * count;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 trim，执行该函数声明的职责。
std::string trim(const std::string& value) {
    // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
    const auto first = value.find_first_not_of(" \t\r\n\f\v");
    // 【逐行批注】 判断条件 (first == std::string::npos) return {};，成立时执行对应分支。
    if (first == std::string::npos) return {};
    // 【逐行批注】 计算右侧结果并写入 auto last，更新当前运行状态。
    const auto last = value.find_last_not_of(" \t\r\n\f\v");
    // 【逐行批注】 结束当前函数并返回结果：return value.substr(first, last - first + 1);。
    return value.substr(first, last - first + 1);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 lower，执行该函数声明的职责。
std::string lower(std::string value) {
    // 【逐行批注】 按循环规则重复处理：for (char& ch : value) ch = static_cast<char>(std::tolower(static_cast<uns…。
    for (char& ch : value) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 containsAny，执行该函数声明的职责。
bool containsAny(const std::string& text, const std::vector<std::string>& needles) {
    // 【逐行批注】 按循环规则重复处理：for (const auto& needle : needles) {。
    for (const auto& needle : needles) {
        // 【逐行批注】 判断条件 (!needle.empty() && text.find(needle) != std::string::npos) retu…，成立时执行对应分支。
        if (!needle.empty() && text.find(needle) != std::string::npos) return true;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return false;。
    return false;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 countChineseChars，执行该函数声明的职责。
int countChineseChars(const std::string& text) {
    // 【逐行批注】 计算右侧结果并写入 int count，更新当前运行状态。
    int count = 0;
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < text.size(); ++i) {。
    for (size_t i = 0; i < text.size(); ++i) {
        // 【逐行批注】 计算右侧结果并写入 unsigned char c，更新当前运行状态。
        const unsigned char c = static_cast<unsigned char>(text[i]);
        // 【逐行批注】 判断条件 (c >= 0xE4) {，成立时执行对应分支。
        if (c >= 0xE4) {
            // 【逐行批注】 执行当前声明或表达式：++count;。
            ++count;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return count;。
    return count;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 sha256Hex，执行该函数声明的职责。
std::string sha256Hex(const std::string& input) {
    // 【逐行批注】 执行当前声明或表达式：static constexpr std::array<uint32_t, 64> k = {。
    static constexpr std::array<uint32_t, 64> k = {
        // 【逐行批注】 执行当前声明或表达式：0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,。
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
        // 【逐行批注】 执行当前声明或表达式：0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,。
        0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        // 【逐行批注】 执行当前声明或表达式：0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,。
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
        // 【逐行批注】 执行当前声明或表达式：0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,。
        0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        // 【逐行批注】 执行当前声明或表达式：0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,。
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
        // 【逐行批注】 执行当前声明或表达式：0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,。
        0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        // 【逐行批注】 执行当前声明或表达式：0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,。
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        // 【逐行批注】 执行当前声明或表达式：0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,。
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        // 【逐行批注】 执行当前声明或表达式：0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,。
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
        // 【逐行批注】 执行当前声明或表达式：0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,。
        0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        // 【逐行批注】 执行当前声明或表达式：0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};。
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
    // 【逐行批注】 计算右侧结果并写入 rotr，更新当前运行状态。
    auto rotr = [](uint32_t value, uint32_t bits) { return (value >> bits) | (value << (32U - bits)); };
    // 【逐行批注】 计算右侧结果并写入 ch，更新当前运行状态。
    auto ch = [](uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); };
    // 【逐行批注】 计算右侧结果并写入 maj，更新当前运行状态。
    auto maj = [](uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); };
    // 【逐行批注】 计算右侧结果并写入 b0，更新当前运行状态。
    auto b0 = [&](uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); };
    // 【逐行批注】 计算右侧结果并写入 b1，更新当前运行状态。
    auto b1 = [&](uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); };
    // 【逐行批注】 计算右侧结果并写入 s0，更新当前运行状态。
    auto s0 = [&](uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); };
    // 【逐行批注】 计算右侧结果并写入 s1，更新当前运行状态。
    auto s1 = [&](uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); };
    // 【逐行批注】 执行当前声明或表达式：std::array<uint32_t, 8> h = {。
    std::array<uint32_t, 8> h = {
        // 【逐行批注】 执行当前声明或表达式：0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,。
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        // 【逐行批注】 执行当前声明或表达式：0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};。
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    // 【逐行批注】 调用 data 完成本行所需操作：std::vector<unsigned char> data(input.begin(), input.end());。
    std::vector<unsigned char> data(input.begin(), input.end());
    // 【逐行批注】 计算右侧结果并写入 uint64_t bitLen，更新当前运行状态。
    const uint64_t bitLen = static_cast<uint64_t>(data.size()) * 8ULL;
    // 【逐行批注】 执行当前声明或表达式：data.push_back(0x80);。
    data.push_back(0x80);
    // 【逐行批注】 在条件成立期间循环处理：while ((data.size() % 64) != 56) data.push_back(0x00);。
    while ((data.size() % 64) != 56) data.push_back(0x00);
    // 【逐行批注】 按循环规则重复处理：for (int shift = 7; shift >= 0; --shift) data.push_back(static_cast<unsign…。
    for (int shift = 7; shift >= 0; --shift) data.push_back(static_cast<unsigned char>((bitLen >> (shift * 8)) & 0xff));
    // 【逐行批注】 按循环规则重复处理：for (size_t offset = 0; offset < data.size(); offset += 64) {。
    for (size_t offset = 0; offset < data.size(); offset += 64) {
        // 【逐行批注】 执行当前声明或表达式：std::array<uint32_t, 64> w{};。
        std::array<uint32_t, 64> w{};
        // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < 16; ++i) {。
        for (size_t i = 0; i < 16; ++i) {
            // 【逐行批注】 计算右侧结果并写入 size_t base，更新当前运行状态。
            const size_t base = offset + i * 4;
            // 【逐行批注】 计算右侧结果并写入 w[i]，更新当前运行状态。
            w[i] = (static_cast<uint32_t>(data[base]) << 24) |
                   // 【逐行批注】 执行当前声明或表达式：(static_cast<uint32_t>(data[base + 1]) << 16) |。
                   (static_cast<uint32_t>(data[base + 1]) << 16) |
                   // 【逐行批注】 执行当前声明或表达式：(static_cast<uint32_t>(data[base + 2]) << 8) |。
                   (static_cast<uint32_t>(data[base + 2]) << 8) |
                   // 【逐行批注】 执行当前声明或表达式：static_cast<uint32_t>(data[base + 3]);。
                   static_cast<uint32_t>(data[base + 3]);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for (size_t i = 16; i < 64; ++i) w[i] = s1(w[i - 2]) + w[i - 7] + s0(w[i -…。
        for (size_t i = 16; i < 64; ++i) w[i] = s1(w[i - 2]) + w[i - 7] + s0(w[i - 15]) + w[i - 16];
        // 【逐行批注】 计算右侧结果并写入 uint32_t a，更新当前运行状态。
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < 64; ++i) {。
        for (size_t i = 0; i < 64; ++i) {
            // 【逐行批注】 计算右侧结果并写入 uint32_t t1，更新当前运行状态。
            const uint32_t t1 = hh + b1(e) + ch(e, f, g) + k[i] + w[i];
            // 【逐行批注】 计算右侧结果并写入 uint32_t t2，更新当前运行状态。
            const uint32_t t2 = b0(a) + maj(a, b, c);
            // 【逐行批注】 计算右侧结果并写入 hh，更新当前运行状态。
            hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 h[0] +，更新当前运行状态。
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream out;。
    std::ostringstream out;
    // 【逐行批注】 调用 setfill 完成本行所需操作：out << std::hex << std::setfill('0');。
    out << std::hex << std::setfill('0');
    // 【逐行批注】 按循环规则重复处理：for (uint32_t value : h) {。
    for (uint32_t value : h) {
        // 【逐行批注】 调用 setw 完成本行所需操作：out << std::setw(8) << value;。
        out << std::setw(8) << value;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return out.str();。
    return out.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 hostFromUrl，执行该函数声明的职责。
std::string hostFromUrl(const std::string& url) {
    // 【逐行批注】 计算右侧结果并写入 std::string value，更新当前运行状态。
    std::string value = lower(url);
    // 【逐行批注】 计算右侧结果并写入 auto scheme，更新当前运行状态。
    const auto scheme = value.find("://");
    // 【逐行批注】 判断条件 (scheme != std::string::npos) value = value.substr(scheme + 3);，成立时执行对应分支。
    if (scheme != std::string::npos) value = value.substr(scheme + 3);
    // 【逐行批注】 计算右侧结果并写入 auto slash，更新当前运行状态。
    const auto slash = value.find('/');
    // 【逐行批注】 判断条件 (slash != std::string::npos) value = value.substr(0, slash);，成立时执行对应分支。
    if (slash != std::string::npos) value = value.substr(0, slash);
    // 【逐行批注】 计算右侧结果并写入 auto at，更新当前运行状态。
    const auto at = value.find('@');
    // 【逐行批注】 判断条件 (at != std::string::npos) value = value.substr(at + 1);，成立时执行对应分支。
    if (at != std::string::npos) value = value.substr(at + 1);
    // 【逐行批注】 计算右侧结果并写入 auto colon，更新当前运行状态。
    const auto colon = value.find(':');
    // 【逐行批注】 判断条件 (colon != std::string::npos) value = value.substr(0, colon);，成立时执行对应分支。
    if (colon != std::string::npos) value = value.substr(0, colon);
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 inferSource，执行该函数声明的职责。
std::string inferSource(const std::string& url, const std::string& title, const std::string& description) {
    // 【逐行批注】 计算右侧结果并写入 std::string host，更新当前运行状态。
    const std::string host = hostFromUrl(url);
    // 【逐行批注】 计算右侧结果并写入 std::string merged，更新当前运行状态。
    const std::string merged = lower(title + " " + description + " " + host);
    // 【逐行批注】 判断条件 (merged.find("khan") != std::string::npos) return "Khan Academy";，成立时执行对应分支。
    if (merged.find("khan") != std::string::npos) return "Khan Academy";
    // 【逐行批注】 判断条件 (merged.find("3blue1brown") != std::string::npos || merged.find(…，成立时执行对应分支。
    if (merged.find("3blue1brown") != std::string::npos || merged.find("3b1b") != std::string::npos) return "3Blue1Brown";
    // 【逐行批注】 判断条件 (merged.find("bilibili") != std::string::npos || merged.find("b2…，成立时执行对应分支。
    if (merged.find("bilibili") != std::string::npos || merged.find("b23.tv") != std::string::npos || merged.find("b站") != std::string::npos) return "B站";
    // 【逐行批注】 判断条件 (merged.find("youtube") != std::string::npos || merged.find("you…，成立时执行对应分支。
    if (merged.find("youtube") != std::string::npos || merged.find("youtu.be") != std::string::npos) return "YouTube";
    // 【逐行批注】 判断条件 (merged.find("github") != std::string::npos) return "GitHub";，成立时执行对应分支。
    if (merged.find("github") != std::string::npos) return "GitHub";
    // 【逐行批注】 判断条件 (merged.find("developer.mozilla") != std::string::npos || merged…，成立时执行对应分支。
    if (merged.find("developer.mozilla") != std::string::npos || merged.find("mdn") != std::string::npos) return "MDN";
    // 【逐行批注】 判断条件 (merged.find("coursera") != std::string::npos) return "Coursera";，成立时执行对应分支。
    if (merged.find("coursera") != std::string::npos) return "Coursera";
    // 【逐行批注】 判断条件 (merged.find("udemy") != std::string::npos) return "Udemy";，成立时执行对应分支。
    if (merged.find("udemy") != std::string::npos) return "Udemy";
    // 【逐行批注】 判断条件 (merged.find("stackoverflow") != std::string::npos) return "Stac…，成立时执行对应分支。
    if (merged.find("stackoverflow") != std::string::npos) return "Stack Overflow";
    // 【逐行批注】 结束当前函数并返回结果：return host.empty() ? "Web" : host;。
    return host.empty() ? "Web" : host;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 inferType，执行该函数声明的职责。
std::string inferType(const std::string& source, const std::string& title, const std::string& description) {
    // 【逐行批注】 计算右侧结果并写入 std::string merged，更新当前运行状态。
    const std::string merged = lower(source + " " + title + " " + description);
    // 【逐行批注】 判断条件 (containsAny(merged, {"github", "repository", "repo", "源码", "开源"…，成立时执行对应分支。
    if (containsAny(merged, {"github", "repository", "repo", "源码", "开源"})) return "开源项目";
    // 【逐行批注】 判断条件 (containsAny(merged, {"mdn", "docs", "documentation", "官方文档", "a…，成立时执行对应分支。
    if (containsAny(merged, {"mdn", "docs", "documentation", "官方文档", "api reference", "api文档"})) return "官方文档";
    // 【逐行批注】 判断条件 (containsAny(merged, {"video", "youtube", "bilibili", "youtube"}…，成立时执行对应分支。
    if (containsAny(merged, {"video", "youtube", "bilibili", "youtube"})) return "视频教程";
    // 【逐行批注】 判断条件 (containsAny(merged, {"coursera", "udemy", "course", "课程", "less…，成立时执行对应分支。
    if (containsAny(merged, {"coursera", "udemy", "course", "课程", "lesson"})) return "在线课程";
    // 【逐行批注】 判断条件 (containsAny(merged, {"exercise", "practice", "quiz", "题库", "练习"…，成立时执行对应分支。
    if (containsAny(merged, {"exercise", "practice", "quiz", "题库", "练习"})) return "练习题库";
    // 【逐行批注】 判断条件 (containsAny(merged, {"project", "实战", "demo", "案例"})) return "项…，成立时执行对应分支。
    if (containsAny(merged, {"project", "实战", "demo", "案例"})) return "项目实战";
    // 【逐行批注】 判断条件 (containsAny(merged, {"compiler", "environment", "工具", "环境", "se…，成立时执行对应分支。
    if (containsAny(merged, {"compiler", "environment", "工具", "环境", "setup"})) return "工具环境";
    // 【逐行批注】 判断条件 (containsAny(merged, {"community", "forum", "discord", "群", "社区"…，成立时执行对应分支。
    if (containsAny(merged, {"community", "forum", "discord", "群", "社区"})) return "社区资源";
    // 【逐行批注】 结束当前函数并返回结果：return "图文教程";。
    return "图文教程";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 inferDifficulty，执行该函数声明的职责。
std::string inferDifficulty(const std::string& title, const std::string& description) {
    // 【逐行批注】 计算右侧结果并写入 std::string merged，更新当前运行状态。
    const std::string merged = lower(title + " " + description);
    // 【逐行批注】 判断条件 (containsAny(merged, {"advanced", "高级", "expert", "pro"})) retur…，成立时执行对应分支。
    if (containsAny(merged, {"advanced", "高级", "expert", "pro"})) return "高级";
    // 【逐行批注】 判断条件 (containsAny(merged, {"intermediate", "进阶", "中级"})) return "进阶";，成立时执行对应分支。
    if (containsAny(merged, {"intermediate", "进阶", "中级"})) return "进阶";
    // 【逐行批注】 结束当前函数并返回结果：return "入门";。
    return "入门";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 inferLanguage，执行该函数声明的职责。
std::string inferLanguage(const std::string& title, const std::string& description) {
    // 【逐行批注】 计算右侧结果并写入 std::string merged，更新当前运行状态。
    const std::string merged = title + " " + description;
    // 【逐行批注】 结束当前函数并返回结果：return countChineseChars(merged) >= 4 ? "中文" : "英文";。
    return countChineseChars(merged) >= 4 ? "中文" : "英文";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 inferFree，执行该函数声明的职责。
bool inferFree(const std::string& source, const std::string& description) {
    // 【逐行批注】 计算右侧结果并写入 std::string merged，更新当前运行状态。
    const std::string merged = lower(source + " " + description);
    // 【逐行批注】 判断条件 (merged.find("free") != std::string::npos || merged.find("open")…，成立时执行对应分支。
    if (merged.find("free") != std::string::npos || merged.find("open") != std::string::npos || merged.find("免费") != std::string::npos) return true;
    // 【逐行批注】 判断条件 (containsAny(merged, {"coursera", "udemy"}) && merged.find("free…，成立时执行对应分支。
    if (containsAny(merged, {"coursera", "udemy"}) && merged.find("free") == std::string::npos) return false;
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 inferScore，执行该函数声明的职责。
double inferScore(double score, const std::string& title, const std::string& description) {
    // 【逐行批注】 判断条件 (score <= 1.0) return std::round(score * 100.0);，成立时执行对应分支。
    if (score <= 1.0) return std::round(score * 100.0);
    // 【逐行批注】 判断条件 (score > 100.0) return 100.0;，成立时执行对应分支。
    if (score > 100.0) return 100.0;
    // 【逐行批注】 判断条件 (score <= 0.0) {，成立时执行对应分支。
    if (score <= 0.0) {
        // 【逐行批注】 判断条件 (containsAny(lower(title + " " + description), {"official", "官方"…，成立时执行对应分支。
        if (containsAny(lower(title + " " + description), {"official", "官方", "khan", "mdn", "github", "youtube"})) return 82.0;
        // 【逐行批注】 结束当前函数并返回结果：return 70.0;。
        return 70.0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return score;。
    return score;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 recommendReason，执行该函数声明的职责。
std::string recommendReason(const std::string& domain) {
    // 【逐行批注】 判断条件 (domain == "programming") return "适合作为编程学习的真实参考资料，覆盖实现、文档和练习。";，成立时执行对应分支。
    if (domain == "programming") return "适合作为编程学习的真实参考资料，覆盖实现、文档和练习。";
    // 【逐行批注】 判断条件 (domain == "math") return "适合数学学习，便于理解概念、观看推导和做题。";，成立时执行对应分支。
    if (domain == "math") return "适合数学学习，便于理解概念、观看推导和做题。";
    // 【逐行批注】 判断条件 (domain == "language") return "适合语言学习，兼顾输入、练习和真实语境。";，成立时执行对应分支。
    if (domain == "language") return "适合语言学习，兼顾输入、练习和真实语境。";
    // 【逐行批注】 判断条件 (domain == "office") return "适合办公技能入门和模板化练习。";，成立时执行对应分支。
    if (domain == "office") return "适合办公技能入门和模板化练习。";
    // 【逐行批注】 判断条件 (domain == "design") return "适合设计类学习，能直接观察作品与方法。";，成立时执行对应分支。
    if (domain == "design") return "适合设计类学习，能直接观察作品与方法。";
    // 【逐行批注】 判断条件 (domain == "ai") return "适合 AI 学习，兼顾原理、工具和实践案例。";，成立时执行对应分支。
    if (domain == "ai") return "适合 AI 学习，兼顾原理、工具和实践案例。";
    // 【逐行批注】 判断条件 (domain == "guitar") return "适合吉他练习，便于按阶段掌握动作和曲目。";，成立时执行对应分支。
    if (domain == "guitar") return "适合吉他练习，便于按阶段掌握动作和曲目。";
    // 【逐行批注】 判断条件 (domain == "nextjs") return "适合 Next.js 学习，接近官方实践路径。";，成立时执行对应分支。
    if (domain == "nextjs") return "适合 Next.js 学习，接近官方实践路径。";
    // 【逐行批注】 判断条件 (domain == "aiDrawing") return "适合 AI 绘图学习，能快速看到实际效果。";，成立时执行对应分支。
    if (domain == "aiDrawing") return "适合 AI 绘图学习，能快速看到实际效果。";
    // 【逐行批注】 判断条件 (domain == "pcBuild") return "适合装机和硬件入门，便于对照实操。";，成立时执行对应分支。
    if (domain == "pcBuild") return "适合装机和硬件入门，便于对照实操。";
    // 【逐行批注】 判断条件 (domain == "examEnglish") return "适合考试英语复习，方便做题和查漏补缺。";，成立时执行对应分支。
    if (domain == "examEnglish") return "适合考试英语复习，方便做题和查漏补缺。";
    // 【逐行批注】 判断条件 (domain == "photography") return "适合摄影学习，便于观察范例和参数搭配。";，成立时执行对应分支。
    if (domain == "photography") return "适合摄影学习，便于观察范例和参数搭配。";
    // 【逐行批注】 结束当前函数并返回结果：return "作为该主题的补充学习资源较合适。";。
    return "作为该主题的补充学习资源较合适。";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 domainQueries，执行该函数声明的职责。
std::vector<std::string> domainQueries(const std::string& domain, const std::string& goal) {
    // 【逐行批注】 计算右侧结果并写入 std::string q，更新当前运行状态。
    const std::string q = trim(goal);
    // 【逐行批注】 判断条件 (domain == "programming") return {q + " 官方文档", q + " GitHub 开源项目…，成立时执行对应分支。
    if (domain == "programming") return {q + " 官方文档", q + " GitHub 开源项目", q + " tutorial", q + " examples", q + " practice"};
    // 【逐行批注】 判断条件 (domain == "math") return {q + " Khan Academy", q + " 3Blue1Brow…，成立时执行对应分支。
    if (domain == "math") return {q + " Khan Academy", q + " 3Blue1Brown", q + " 练习题", q + " proof", q + " lecture"};
    // 【逐行批注】 判断条件 (domain == "language") return {q + " learning resources", q + " …，成立时执行对应分支。
    if (domain == "language") return {q + " learning resources", q + " practice", q + " listening", q + " speaking", q + " course"};
    // 【逐行批注】 判断条件 (domain == "office") return {q + " tutorial", q + " template", q…，成立时执行对应分支。
    if (domain == "office") return {q + " tutorial", q + " template", q + " practice", q + " official docs"};
    // 【逐行批注】 判断条件 (domain == "design") return {q + " portfolio", q + " tutorial", …，成立时执行对应分支。
    if (domain == "design") return {q + " portfolio", q + " tutorial", q + " case study", q + " design system"};
    // 【逐行批注】 判断条件 (domain == "ai") return {q + " official docs", q + " course", q …，成立时执行对应分支。
    if (domain == "ai") return {q + " official docs", q + " course", q + " GitHub", q + " tutorial", q + " examples"};
    // 【逐行批注】 判断条件 (domain == "guitar") return {q + " tutorial", q + " chords", q +…，成立时执行对应分支。
    if (domain == "guitar") return {q + " tutorial", q + " chords", q + " practice", q + " lesson"};
    // 【逐行批注】 判断条件 (domain == "nextjs") return {q + " official docs", q + " GitHub"…，成立时执行对应分支。
    if (domain == "nextjs") return {q + " official docs", q + " GitHub", q + " tutorial", q + " example app"};
    // 【逐行批注】 判断条件 (domain == "aiDrawing") return {q + " prompt guide", q + " tutor…，成立时执行对应分支。
    if (domain == "aiDrawing") return {q + " prompt guide", q + " tutorial", q + " examples", q + " art community"};
    // 【逐行批注】 判断条件 (domain == "pcBuild") return {q + " build guide", q + " hardware…，成立时执行对应分支。
    if (domain == "pcBuild") return {q + " build guide", q + " hardware review", q + " tutorial", q + " forum"};
    // 【逐行批注】 判断条件 (domain == "examEnglish") return {q + " exam practice", q + " vo…，成立时执行对应分支。
    if (domain == "examEnglish") return {q + " exam practice", q + " vocabulary", q + " listening", q + " reading"};
    // 【逐行批注】 判断条件 (domain == "photography") return {q + " tutorial", q + " exposur…，成立时执行对应分支。
    if (domain == "photography") return {q + " tutorial", q + " exposure", q + " composition", q + " examples"};
    // 【逐行批注】 结束当前函数并返回结果：return {q, q + " tutorial", q + " official docs", q + " examples"};。
    return {q, q + " tutorial", q + " official docs", q + " examples"};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 detectDomain，执行该函数声明的职责。
std::string detectDomain(const std::string& goal) {
    // 【逐行批注】 计算右侧结果并写入 std::string text，更新当前运行状态。
    const std::string text = lower(goal);
    // 【逐行批注】 判断条件 (containsAny(text, {"python", "java", "cpp", "c++", "rust", "go"…，成立时执行对应分支。
    if (containsAny(text, {"python", "java", "cpp", "c++", "rust", "go", "编程", "代码", "开发", "api", "web", "frontend", "backend", "算法"})) return "programming";
    // 【逐行批注】 判断条件 (containsAny(text, {"数学", "代数", "几何", "微积分", "概率", "统计", "函数", "…，成立时执行对应分支。
    if (containsAny(text, {"数学", "代数", "几何", "微积分", "概率", "统计", "函数", "导数", "matrix"})) return "math";
    // 【逐行批注】 判断条件 (containsAny(text, {"英语", "日语", "法语", "韩语", "语言", "听力", "口语", "词…，成立时执行对应分支。
    if (containsAny(text, {"英语", "日语", "法语", "韩语", "语言", "听力", "口语", "词汇", "语法"})) return "language";
    // 【逐行批注】 判断条件 (containsAny(text, {"office", "excel", "word", "ppt", "pptx", "表…，成立时执行对应分支。
    if (containsAny(text, {"office", "excel", "word", "ppt", "pptx", "表格", "文档", "办公"})) return "office";
    // 【逐行批注】 判断条件 (containsAny(text, {"design", "ui", "ux", "视觉", "平面", "品牌", "fig…，成立时执行对应分支。
    if (containsAny(text, {"design", "ui", "ux", "视觉", "平面", "品牌", "figma", "sketch"})) return "design";
    // 【逐行批注】 判断条件 (containsAny(text, {"ai", "llm", "机器学习", "深度学习", "模型", "prompt",…，成立时执行对应分支。
    if (containsAny(text, {"ai", "llm", "机器学习", "深度学习", "模型", "prompt", "人工智能"})) return "ai";
    // 【逐行批注】 判断条件 (containsAny(text, {"guitar", "吉他", "弹唱", "和弦"})) return "guitar…，成立时执行对应分支。
    if (containsAny(text, {"guitar", "吉他", "弹唱", "和弦"})) return "guitar";
    // 【逐行批注】 判断条件 (containsAny(text, {"nextjs", "next.js", "react", "前端框架"})) retu…，成立时执行对应分支。
    if (containsAny(text, {"nextjs", "next.js", "react", "前端框架"})) return "nextjs";
    // 【逐行批注】 判断条件 (containsAny(text, {"绘画", "画图", "midjourney", "stable diffusion"…，成立时执行对应分支。
    if (containsAny(text, {"绘画", "画图", "midjourney", "stable diffusion", "aigc", "ai绘图"})) return "aiDrawing";
    // 【逐行批注】 判断条件 (containsAny(text, {"装机", "电脑", "硬件", "显卡", "cpu", "主板", "内存"}))…，成立时执行对应分支。
    if (containsAny(text, {"装机", "电脑", "硬件", "显卡", "cpu", "主板", "内存"})) return "pcBuild";
    // 【逐行批注】 判断条件 (containsAny(text, {"考研英语", "四六级", "雅思", "托福", "英语考试"})) return …，成立时执行对应分支。
    if (containsAny(text, {"考研英语", "四六级", "雅思", "托福", "英语考试"})) return "examEnglish";
    // 【逐行批注】 判断条件 (containsAny(text, {"摄影", "相机", "镜头", "构图", "曝光"})) return "phot…，成立时执行对应分支。
    if (containsAny(text, {"摄影", "相机", "镜头", "构图", "曝光"})) return "photography";
    // 【逐行批注】 结束当前函数并返回结果：return "general";。
    return "general";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明数据结构或类型 HttpResult 及其成员接口。
struct HttpResult { long status = 0; std::string body; CURLcode error = CURLE_OK; bool directRetry = false; int attempts = 0; bool budgetDenied = false; };

// 【逐行批注】 定义函数 evaluationSearchAttempt，执行该函数声明的职责。
bool evaluationSearchAttempt() {
    // 【逐行批注】 计算右侧结果并写入 auto filename，更新当前运行状态。
    const auto filename = std::getenv("GANGYI_EVAL_SEARCH_BUDGET_FILE");
    // 【逐行批注】 判断条件 (!filename || !*filename) return true;，成立时执行对应分支。
    if (!filename || !*filename) return true;
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 仅开发验收启用；发送前先持久记账，未知请求不退款，各进程共用同一把文件锁。
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto path，更新当前运行状态。
        const auto path = fs::u8path(filename);
        // 【逐行批注】 声明数据结构或类型 Lock 及其成员接口。
        struct Lock {
            // 【逐行批注】 计算右侧结果并写入 HANDLE handle，更新当前运行状态。
            HANDLE handle = INVALID_HANDLE_VALUE;
            // 【逐行批注】 定义函数 Lock，执行该函数声明的职责。
            explicit Lock(const fs::path& file) {
                // 【逐行批注】 计算右侧结果并写入 auto name，更新当前运行状态。
                const auto name = file.wstring() + L".lock";
                // 【逐行批注】 按循环规则重复处理：for (int i = 0; i < 1500; ++i) {。
                for (int i = 0; i < 1500; ++i) {
                    // 【逐行批注】 计算右侧结果并写入 handle，更新当前运行状态。
                    handle = CreateFileW(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                    // 【逐行批注】 判断条件 (handle != INVALID_HANDLE_VALUE) return;，成立时执行对应分支。
                    if (handle != INVALID_HANDLE_VALUE) return;
                    // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(10));。
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                // 【逐行批注】 结束当前代码块。
                }
                // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("搜索预算锁不可用");。
                throw std::runtime_error("搜索预算锁不可用");
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 调用 Lock、CloseHandle 完成本行所需操作：~Lock() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }。
            ~Lock() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
        // 【逐行批注】 调用 lock 完成本行所需操作：} lock(path);。
        } lock(path);
        // 【逐行批注】 执行当前声明或表达式：json value;。
        json value;
        // 【逐行批注】 调用 input 完成本行所需操作：{ std::ifstream input(path, std::ios::binary); input >> value; }。
        { std::ifstream input(path, std::ios::binary); input >> value; }
        // 【逐行批注】 按循环规则重复处理：for (const auto* field : {"limit", "attempts", "reserved"})。
        for (const auto* field : {"limit", "attempts", "reserved"})
            // 【逐行批注】 判断条件 (!value.at(field).is_number_integer() || value.at(field).get<lon…，成立时执行对应分支。
            if (!value.at(field).is_number_integer() || value.at(field).get<long long>() < 0) return false;
        // 【逐行批注】 计算右侧结果并写入 auto limit，更新当前运行状态。
        const auto limit = value.at("limit").get<long long>();
        // 【逐行批注】 判断条件 (limit > 20 || value.at("attempts").get<long long>() + value.at(…，成立时执行对应分支。
        if (limit > 20 || value.at("attempts").get<long long>() + value.at("reserved").get<long long>() >= limit) return false;
        // 【逐行批注】 计算右侧结果并写入 value["attempts"]，更新当前运行状态。
        value["attempts"] = value.at("attempts").get<long long>() + 1;
        // 【逐行批注】 计算右侧结果并写入 value["countingPolicy"]，更新当前运行状态。
        value["countingPolicy"] = "发送前计入；中断及未知请求不退款";
        // 【逐行批注】 计算右侧结果并写入 temporary，更新当前运行状态。
        auto temporary = path; temporary += ".tmp";
        // 【逐行批注】 调用 output 完成本行所需操作：{ std::ofstream output(temporary, std::ios::binary | std::ios::trunc)…。
        { std::ofstream output(temporary, std::ios::binary | std::ios::trunc); output << value.dump(2); output.flush();
          // 【逐行批注】 判断条件 (!output) return false;，成立时执行对应分支。
          if (!output) return false;
          // 【逐行批注】 执行当前声明或表达式：output.close(); if (!output) return false; }。
          output.close(); if (!output) return false; }
        // 【逐行批注】 结束当前函数并返回结果：return MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_…。
        return MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) { return false; }。
    } catch (...) { return false; }
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 结束当前函数并返回结果：return false;。
    return false;
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 searchProgress，执行该函数声明的职责。
int searchProgress(void* user, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    // 【逐行批注】 计算右侧结果并写入 auto* cancelled，更新当前运行状态。
    const auto* cancelled = static_cast<const std::function<bool()>*>(user);
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { return cancelled && *cancelled && (*cancelled)() ? 1 : 0; } catch (...) { return 1; }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 声明对象 postJson，并通过构造函数初始化其状态。
HttpResult postJson(const std::string& url, const std::string& key, const json& body, long timeoutMs,
                    // 【逐行批注】 调用 bool 完成本行所需操作：const std::function<bool()>& cancelled = {}, const std::function<bool…。
                    const std::function<bool()>& cancelled = {}, const std::function<bool()>& started = {}) {
    // 【逐行批注】 执行当前声明或表达式：HttpResult result;。
    HttpResult result;
    // 【逐行批注】 计算右侧结果并写入 CURL* curl，更新当前运行状态。
    // 实验总账只覆盖聊天模型用量，按次计费的搜索不能单独发送。
    if (aiMoneyBudgetGuarded()) {
        result.budgetDenied = true;
        return result;
    }
    CURL* curl = curl_easy_init();
    // 【逐行批注】 判断条件 (!curl) { result.error = CURLE_FAILED_INIT; return result; }，成立时执行对应分支。
    if (!curl) { result.error = CURLE_FAILED_INIT; return result; }
    // 【逐行批注】 声明数据结构或类型 curl_slist 及其成员接口。
    struct curl_slist* headers = nullptr;
    // 【逐行批注】 计算右侧结果并写入 headers，更新当前运行状态。
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // 【逐行批注】 判断条件 (!key.empty()) {，成立时执行对应分支。
    if (!key.empty()) {
        // 【逐行批注】 计算右侧结果并写入 std::string auth，更新当前运行状态。
        const std::string auth = "Authorization: Bearer " + trim(key);
        // 【逐行批注】 计算右侧结果并写入 headers，更新当前运行状态。
        headers = curl_slist_append(headers, auth.c_str());
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 std::string payload，更新当前运行状态。
    const std::string payload = body.dump();
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_URL, url.c_str());。
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_POST, 1L);。
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());。
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(paylo…。
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);。
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBody);。
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBody);
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.body);。
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.body);
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);。
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);。
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);。
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);。
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);。
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, searchProgress);。
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, searchProgress);
    // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &cancelled);。
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &cancelled);
    // 【逐行批注】 计算右侧结果并写入 auto perform，更新当前运行状态。
    const auto perform = [&] {
        // 【逐行批注】 判断条件 (!(started ? started() : evaluationSearchAttempt())) return CURL…，成立时执行对应分支。
        if (!(started ? started() : evaluationSearchAttempt())) return CURLE_ABORTED_BY_CALLBACK;
        // 【逐行批注】 调用 curl_easy_perform 完成本行所需操作：++result.attempts; return curl_easy_perform(curl);。
        ++result.attempts; return curl_easy_perform(curl);
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 计算右侧结果并写入 result.error，更新当前运行状态。
    result.error = perform();
    // 【逐行批注】 调用 curl_easy_getinfo 完成本行所需操作：curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);。
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);
    // 【逐行批注】 计算右侧结果并写入 long usedProxy，更新当前运行状态。
    long usedProxy = 0;
// 【逐行批注】 处理条件编译指令：#if LIBCURL_VERSION_NUM >= 0x080700。
#if LIBCURL_VERSION_NUM >= 0x080700
    // 【逐行批注】 调用 curl_easy_getinfo 完成本行所需操作：curl_easy_getinfo(curl, CURLINFO_USED_PROXY, &usedProxy);。
    curl_easy_getinfo(curl, CURLINFO_USED_PROXY, &usedProxy);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 判断条件 (result.error != CURLE_OK && result.error != CURLE_ABORTED_BY_CA…，成立时执行对应分支。
    if (result.error != CURLE_OK && result.error != CURLE_ABORTED_BY_CALLBACK && usedProxy && url.rfind("https://api.bocha", 0) == 0) {
        // 【逐行批注】 执行当前声明或表达式：result.body.clear();。
        result.body.clear();
        // 【逐行批注】 计算右侧结果并写入 result.status，更新当前运行状态。
        result.status = 0;
        // 【逐行批注】 计算右侧结果并写入 result.directRetry，更新当前运行状态。
        result.directRetry = true;
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl, CURLOPT_NOPROXY, "*");。
        curl_easy_setopt(curl, CURLOPT_NOPROXY, "*");
        // 【逐行批注】 计算右侧结果并写入 result.error，更新当前运行状态。
        result.error = perform();
        // 【逐行批注】 调用 curl_easy_getinfo 完成本行所需操作：curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);。
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 curl_slist_free_all 完成本行所需操作：curl_slist_free_all(headers);。
    curl_slist_free_all(headers);
    // 【逐行批注】 调用 curl_easy_cleanup 完成本行所需操作：curl_easy_cleanup(curl);。
    curl_easy_cleanup(curl);
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 parseBocha，执行该函数声明的职责。
std::vector<SearchResource> parseBocha(const std::string& body) {
    // 【逐行批注】 执行当前声明或表达式：std::vector<SearchResource> out;。
    std::vector<SearchResource> out;
    // 【逐行批注】 计算右侧结果并写入 json parsed，更新当前运行状态。
    json parsed = json::parse(body, nullptr, false);
    // 【逐行批注】 判断条件 (!parsed.is_object()) return out;，成立时执行对应分支。
    if (!parsed.is_object()) return out;
    // 【逐行批注】 计算右侧结果并写入 json items，更新当前运行状态。
    json items = json::array();
    // 【逐行批注】 判断条件 (parsed.contains("data") && parsed["data"].is_object() && parsed…，成立时执行对应分支。
    if (parsed.contains("data") && parsed["data"].is_object() && parsed["data"].contains("webPages") &&
        // 【逐行批注】 执行当前声明或表达式：parsed["data"]["webPages"].is_object() && parsed["data"]["webPages"].contains("value…。
        parsed["data"]["webPages"].is_object() && parsed["data"]["webPages"].contains("value") &&
        // 【逐行批注】 执行当前声明或表达式：parsed["data"]["webPages"]["value"].is_array()) {。
        parsed["data"]["webPages"]["value"].is_array()) {
        // 【逐行批注】 计算右侧结果并写入 items，更新当前运行状态。
        items = parsed["data"]["webPages"]["value"];
    // 【逐行批注】 执行当前声明或表达式：} else if (parsed.contains("data") && parsed["data"].is_object() && parsed["data"].c…。
    } else if (parsed.contains("data") && parsed["data"].is_object() && parsed["data"].contains("results") &&
               // 【逐行批注】 执行当前声明或表达式：parsed["data"]["results"].is_array()) {。
               parsed["data"]["results"].is_array()) {
        // 【逐行批注】 计算右侧结果并写入 items，更新当前运行状态。
        items = parsed["data"]["results"];
    // 【逐行批注】 执行当前声明或表达式：} else if (parsed.contains("results") && parsed["results"].is_array()) {。
    } else if (parsed.contains("results") && parsed["results"].is_array()) {
        // 【逐行批注】 计算右侧结果并写入 items，更新当前运行状态。
        items = parsed["results"];
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : items) {。
    for (const auto& item : items) {
        // 【逐行批注】 判断条件 (!item.is_object()) continue;，成立时执行对应分支。
        if (!item.is_object()) continue;
        // 【逐行批注】 执行当前声明或表达式：SearchResource resource;。
        SearchResource resource;
        // 【逐行批注】 计算右侧结果并写入 resource.title，更新当前运行状态。
        resource.title = item.value("name", item.value("title", ""));
        // 【逐行批注】 计算右侧结果并写入 resource.url，更新当前运行状态。
        resource.url = item.value("url", item.value("link", ""));
        // 【逐行批注】 计算右侧结果并写入 resource.description，更新当前运行状态。
        resource.description = item.value("summary", item.value("snippet", item.value("content", item.value("description", ""))));
        // 【逐行批注】 计算右侧结果并写入 resource.source，更新当前运行状态。
        resource.source = item.value("siteName", json()).is_string() ? item["siteName"].get<std::string>() : "";
        // 【逐行批注】 计算右侧结果并写入 resource.publishedAt，更新当前运行状态。
        resource.publishedAt = item.value("datePublished", json()).is_string() ? item["datePublished"].get<std::string>() : "";
        // 【逐行批注】 计算右侧结果并写入 resource.updatedAt，更新当前运行状态。
        resource.updatedAt = item.value("dateModified", json()).is_string() ? item["dateModified"].get<std::string>() : "";
        // 【逐行批注】 计算右侧结果并写入 resource.score，更新当前运行状态。
        resource.score = inferScore(item.value("score", 0.0), resource.title, resource.description);
        // 【逐行批注】 调用 move 完成本行所需操作：out.push_back(std::move(resource));。
        out.push_back(std::move(resource));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return out;。
    return out;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 normalizeResources，执行该函数声明的职责。
std::vector<SearchResource> normalizeResources(std::vector<SearchResource> resources, const std::string& domain) {
    // 【逐行批注】 执行当前声明或表达式：std::set<std::string> seen;。
    std::set<std::string> seen;
    // 【逐行批注】 执行当前声明或表达式：std::vector<SearchResource> out;。
    std::vector<SearchResource> out;
    // 【逐行批注】 按循环规则重复处理：for (auto& resource : resources) {。
    for (auto& resource : resources) {
        // 【逐行批注】 计算右侧结果并写入 resource.url，更新当前运行状态。
        resource.url = trim(resource.url);
        // 【逐行批注】 计算右侧结果并写入 resource.title，更新当前运行状态。
        resource.title = trim(resource.title);
        // 【逐行批注】 计算右侧结果并写入 resource.description，更新当前运行状态。
        resource.description = trim(resource.description);
        // 【逐行批注】 判断条件 (resource.url.empty()) continue;，成立时执行对应分支。
        if (resource.url.empty()) continue;
        // 【逐行批注】 计算右侧结果并写入 std::string key，更新当前运行状态。
        std::string key = resource.url.substr(0, resource.url.find('#'));
        // 【逐行批注】 判断条件 (!seen.insert(key).second) {，成立时执行对应分支。
        if (!seen.insert(key).second) {
            // 【逐行批注】 计算右侧结果并写入 auto& saved，更新当前运行状态。
            auto& saved = *std::find_if(out.begin(), out.end(), [&](const auto& item) { return item.url.substr(0, item.url.find('#')) == key; });
            // 【逐行批注】 按循环规则重复处理：for (const auto& provider : resource.searchProviders)。
            for (const auto& provider : resource.searchProviders)
                // 【逐行批注】 判断条件 (std::find(saved.searchProviders.begin(), saved.searchProviders.…，成立时执行对应分支。
                if (std::find(saved.searchProviders.begin(), saved.searchProviders.end(), provider) == saved.searchProviders.end()) saved.searchProviders.push_back(provider);
            // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
            continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (resource.source.empty()) resource.source = inferSource(resource…，成立时执行对应分支。
        if (resource.source.empty()) resource.source = inferSource(resource.url, resource.title, resource.description);
        // 【逐行批注】 计算右侧结果并写入 resource.type，更新当前运行状态。
        resource.type = inferType(resource.source, resource.title, resource.description);
        // 【逐行批注】 计算右侧结果并写入 resource.difficulty，更新当前运行状态。
        resource.difficulty = inferDifficulty(resource.title, resource.description);
        // 【逐行批注】 计算右侧结果并写入 resource.language，更新当前运行状态。
        resource.language = inferLanguage(resource.title, resource.description);
        // 【逐行批注】 计算右侧结果并写入 resource.free，更新当前运行状态。
        resource.free = inferFree(resource.source, resource.description);
        // 【逐行批注】 计算右侧结果并写入 resource.score，更新当前运行状态。
        resource.score = inferScore(resource.score, resource.title, resource.description);
        // 【逐行批注】 计算右侧结果并写入 resource.reason，更新当前运行状态。
        resource.reason = recommendReason(domain);
        // 【逐行批注】 调用 move 完成本行所需操作：out.push_back(std::move(resource));。
        out.push_back(std::move(resource));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 sort 完成本行所需操作：std::sort(out.begin(), out.end(), [](const SearchResource& a, const S…。
    std::sort(out.begin(), out.end(), [](const SearchResource& a, const SearchResource& b) {
        // 【逐行批注】 判断条件 (a.score != b.score) return a.score > b.score;，成立时执行对应分支。
        if (a.score != b.score) return a.score > b.score;
        // 【逐行批注】 结束当前函数并返回结果：return a.title < b.title;。
        return a.title < b.title;
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 结束当前函数并返回结果：return out;。
    return out;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 serializeCache，执行该函数声明的职责。
json serializeCache(const std::vector<SearchResource>& resources) {
    // 【逐行批注】 计算右侧结果并写入 json arr，更新当前运行状态。
    json arr = json::array();
    // 【逐行批注】 按循环规则重复处理：for (const auto& resource : resources) {。
    for (const auto& resource : resources) {
        // 【逐行批注】 执行当前声明或表达式：arr.push_back({{"title", resource.title},。
        arr.push_back({{"title", resource.title},
                       // 【逐行批注】 执行当前声明或表达式：{"url", resource.url},。
                       {"url", resource.url},
                       // 【逐行批注】 执行当前声明或表达式：{"source", resource.source},。
                       {"source", resource.source},
                       // 【逐行批注】 执行当前声明或表达式：{"description", resource.description},。
                       {"description", resource.description},
                       // 【逐行批注】 执行当前声明或表达式：{"type", resource.type},。
                       {"type", resource.type},
                       // 【逐行批注】 执行当前声明或表达式：{"difficulty", resource.difficulty},。
                       {"difficulty", resource.difficulty},
                       // 【逐行批注】 执行当前声明或表达式：{"language", resource.language},。
                       {"language", resource.language},
                       // 【逐行批注】 执行当前声明或表达式：{"free", resource.free},。
                       {"free", resource.free},
                       // 【逐行批注】 执行当前声明或表达式：{"score", resource.score},。
                       {"score", resource.score},
                       // 【逐行批注】 执行当前声明或表达式：{"reason", resource.reason}});。
                       {"reason", resource.reason}});
        // 【逐行批注】 计算右侧结果并写入 arr.back()["publishedAt"]，更新当前运行状态。
        arr.back()["publishedAt"] = resource.publishedAt;
        // 【逐行批注】 计算右侧结果并写入 arr.back()["updatedAt"]，更新当前运行状态。
        arr.back()["updatedAt"] = resource.updatedAt;
        // 【逐行批注】 计算右侧结果并写入 arr.back()["searchProviders"]，更新当前运行状态。
        arr.back()["searchProviders"] = resource.searchProviders;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return { {"createdAt", std::chrono::duration_cast<std::chrono::second…。
    return { {"createdAt", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()}, {"resources", arr} };
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 readCache，执行该函数声明的职责。
std::optional<std::vector<SearchResource>> readCache(const std::string& dir, const std::string& key, long long ttlSeconds) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 fs::path path，更新当前运行状态。
        const fs::path path = fs::u8path(dir) / (key + ".json");
        // 【逐行批注】 判断条件 (!fs::exists(path)) return std::nullopt;，成立时执行对应分支。
        if (!fs::exists(path)) return std::nullopt;
        // 【逐行批注】 声明对象 in，并通过构造函数初始化其状态。
        std::ifstream in(path, std::ios::binary);
        // 【逐行批注】 判断条件 (!in) return std::nullopt;，成立时执行对应分支。
        if (!in) return std::nullopt;
        // 【逐行批注】 执行当前声明或表达式：std::ostringstream buffer;。
        std::ostringstream buffer;
        // 【逐行批注】 执行当前声明或表达式：buffer << in.rdbuf();。
        buffer << in.rdbuf();
        // 【逐行批注】 计算右侧结果并写入 json parsed，更新当前运行状态。
        json parsed = json::parse(buffer.str(), nullptr, false);
        // 【逐行批注】 判断条件 (!parsed.is_object() || !parsed.contains("createdAt") || !parsed…，成立时执行对应分支。
        if (!parsed.is_object() || !parsed.contains("createdAt") || !parsed.contains("resources") || !parsed["resources"].is_array()) return std::nullopt;
        // 【逐行批注】 计算右侧结果并写入 long long createdAt，更新当前运行状态。
        const long long createdAt = parsed.value("createdAt", 0LL);
        // 【逐行批注】 计算右侧结果并写入 long long now，更新当前运行状态。
        const long long now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        // 【逐行批注】 判断条件 (now < createdAt || now - createdAt > ttlSeconds) return std::nu…，成立时执行对应分支。
        if (now < createdAt || now - createdAt > ttlSeconds) return std::nullopt;
        // 【逐行批注】 执行当前声明或表达式：std::vector<SearchResource> resources;。
        std::vector<SearchResource> resources;
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : parsed["resources"]) {。
        for (const auto& item : parsed["resources"]) {
            // 【逐行批注】 判断条件 (!item.is_object()) continue;，成立时执行对应分支。
            if (!item.is_object()) continue;
            // 【逐行批注】 执行当前声明或表达式：SearchResource resource;。
            SearchResource resource;
            // 【逐行批注】 计算右侧结果并写入 resource.title，更新当前运行状态。
            resource.title = item.value("title", "");
            // 【逐行批注】 计算右侧结果并写入 resource.url，更新当前运行状态。
            resource.url = item.value("url", "");
            // 【逐行批注】 计算右侧结果并写入 resource.source，更新当前运行状态。
            resource.source = item.value("source", "");
            // 【逐行批注】 计算右侧结果并写入 resource.description，更新当前运行状态。
            resource.description = item.value("description", "");
            // 【逐行批注】 计算右侧结果并写入 resource.type，更新当前运行状态。
            resource.type = item.value("type", "");
            // 【逐行批注】 计算右侧结果并写入 resource.difficulty，更新当前运行状态。
            resource.difficulty = item.value("difficulty", "");
            // 【逐行批注】 计算右侧结果并写入 resource.language，更新当前运行状态。
            resource.language = item.value("language", "");
            // 【逐行批注】 计算右侧结果并写入 resource.free，更新当前运行状态。
            resource.free = item.value("free", true);
            // 【逐行批注】 计算右侧结果并写入 resource.score，更新当前运行状态。
            resource.score = item.value("score", 70.0);
            // 【逐行批注】 计算右侧结果并写入 resource.reason，更新当前运行状态。
            resource.reason = item.value("reason", "");
            // 【逐行批注】 计算右侧结果并写入 resource.publishedAt，更新当前运行状态。
            resource.publishedAt = item.value("publishedAt", "");
            // 【逐行批注】 计算右侧结果并写入 resource.updatedAt，更新当前运行状态。
            resource.updatedAt = item.value("updatedAt", "");
            // 【逐行批注】 计算右侧结果并写入 resource.searchProviders，更新当前运行状态。
            resource.searchProviders = item.value("searchProviders", std::vector<std::string>{});
            // 【逐行批注】 调用 move 完成本行所需操作：resources.push_back(std::move(resource));。
            resources.push_back(std::move(resource));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return resources;。
        return resources;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return std::nullopt;。
        return std::nullopt;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 每个精确缓存键使用系统文件锁，独立进程也必须先核对世代再提交。
// 【逐行批注】 声明数据结构或类型 SearchCacheLock 及其成员接口。
class SearchCacheLock {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 调用 SearchCacheLock、bool 完成本行所需操作：SearchCacheLock(const fs::path& path, const std::function<bool()>& ca…。
    SearchCacheLock(const fs::path& path, const std::function<bool()>& cancelled = {}) {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
        // 【逐行批注】 计算右侧结果并写入 handle_，更新当前运行状态。
        handle_ = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            // 【逐行批注】 执行当前声明或表达式：nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);。
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        // 【逐行批注】 判断条件 (handle_ == INVALID_HANDLE_VALUE) throw std::runtime_error("搜索缓存…，成立时执行对应分支。
        if (handle_ == INVALID_HANDLE_VALUE) throw std::runtime_error("搜索缓存锁不可用");
// 【逐行批注】 处理条件编译指令：#else。
#else
        // 【逐行批注】 计算右侧结果并写入 handle_，更新当前运行状态。
        handle_ = open(path.c_str(), O_CREAT | O_RDWR, 0600);
        // 【逐行批注】 判断条件 (handle_ < 0) throw std::runtime_error("搜索缓存锁不可用");，成立时执行对应分支。
        if (handle_ < 0) throw std::runtime_error("搜索缓存锁不可用");
// 【逐行批注】 处理条件编译指令：#endif。
#endif
        // 【逐行批注】 按循环规则重复处理：for (int attempt = 0; attempt < 100; ++attempt) {。
        for (int attempt = 0; attempt < 100; ++attempt) {
            // 【逐行批注】 判断条件 (cancelled && cancelled()) break;，成立时执行对应分支。
            if (cancelled && cancelled()) break;
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
            // 【逐行批注】 判断条件 (LockFileEx(handle_, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMM…，成立时执行对应分支。
            if (LockFileEx(handle_, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &overlap_)) return;
// 【逐行批注】 处理条件编译指令：#else。
#else
            // 【逐行批注】 判断条件 (flock(handle_, LOCK_EX | LOCK_NB) == 0) return;，成立时执行对应分支。
            if (flock(handle_, LOCK_EX | LOCK_NB) == 0) return;
// 【逐行批注】 处理条件编译指令：#endif。
#endif
            // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(10));。
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 close、runtime_error 完成本行所需操作：close(); throw std::runtime_error("搜索缓存锁等待已结束");。
        close(); throw std::runtime_error("搜索缓存锁等待已结束");
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 SearchCacheLock、close 完成本行所需操作：~SearchCacheLock() { close(); }。
    ~SearchCacheLock() { close(); }
    // 【逐行批注】 计算右侧结果并写入 SearchCacheLock(const SearchCacheLock&)，更新当前运行状态。
    SearchCacheLock(const SearchCacheLock&) = delete;
    // 【逐行批注】 计算右侧结果并写入 SearchCacheLock& operator，更新当前运行状态。
    SearchCacheLock& operator=(const SearchCacheLock&) = delete;
// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 定义函数 close，执行该函数声明的职责。
    void close() {
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
        // 【逐行批注】 判断条件 (handle_ != INVALID_HANDLE_VALUE) {，成立时执行对应分支。
        if (handle_ != INVALID_HANDLE_VALUE) {
            // 【逐行批注】 计算右侧结果并写入 UnlockFileEx(handle_, 0, 1, 0, &overlap_); C…，更新当前运行状态。
            UnlockFileEx(handle_, 0, 1, 0, &overlap_); CloseHandle(handle_); handle_ = INVALID_HANDLE_VALUE;
        // 【逐行批注】 结束当前代码块。
        }
// 【逐行批注】 处理条件编译指令：#else。
#else
        // 【逐行批注】 判断条件 (handle_ >= 0) { flock(handle_, LOCK_UN); ::close(handle_); hand…，成立时执行对应分支。
        if (handle_ >= 0) { flock(handle_, LOCK_UN); ::close(handle_); handle_ = -1; }
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 计算右侧结果并写入 HANDLE handle_，更新当前运行状态。
    HANDLE handle_ = INVALID_HANDLE_VALUE;
    // 【逐行批注】 执行当前声明或表达式：OVERLAPPED overlap_{};。
    OVERLAPPED overlap_{};
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 计算右侧结果并写入 int handle_，更新当前运行状态。
    int handle_ = -1;
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 replaceCacheFile，执行该函数声明的职责。
void replaceCacheFile(const fs::path& path, const json& value) {
    // 【逐行批注】 计算右侧结果并写入 temporary，更新当前运行状态。
    auto temporary = path; temporary += ".tmp";
    // 【逐行批注】 声明对象 output，并通过构造函数初始化其状态。
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    // 【逐行批注】 执行当前声明或表达式：output << value.dump(2); output.flush();。
    output << value.dump(2); output.flush();
    // 【逐行批注】 判断条件 (!output) throw std::runtime_error("搜索缓存保存失败");，成立时执行对应分支。
    if (!output) throw std::runtime_error("搜索缓存保存失败");
    // 【逐行批注】 执行当前声明或表达式：output.close();。
    output.close();
    // 【逐行批注】 判断条件 (!output) throw std::runtime_error("搜索缓存关闭失败");，成立时执行对应分支。
    if (!output) throw std::runtime_error("搜索缓存关闭失败");
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 判断条件 (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_…，成立时执行对应分支。
    if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("搜索缓存提交失败");。
        throw std::runtime_error("搜索缓存提交失败");
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 rename 完成本行所需操作：fs::rename(temporary, path);。
    fs::rename(temporary, path);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明数据结构或类型 SearchCacheVersion 及其成员接口。
struct SearchCacheVersion {
    // 【逐行批注】 计算右侧结果并写入 std::uint64_t generation，更新当前运行状态。
    std::uint64_t generation = 0;
    // 【逐行批注】 计算右侧结果并写入 std::uint64_t committed，更新当前运行状态。
    std::uint64_t committed = 0;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 cacheVersion，执行该函数声明的职责。
SearchCacheVersion cacheVersion(const fs::path& path) {
    // 【逐行批注】 判断条件 (!fs::exists(path)) return {};，成立时执行对应分支。
    if (!fs::exists(path)) return {};
    // 【逐行批注】 声明对象 input，并通过构造函数初始化其状态。
    std::ifstream input(path, std::ios::binary);
    // 【逐行批注】 执行当前声明或表达式：json value; input >> value;。
    json value; input >> value;
    // 【逐行批注】 按循环规则重复处理：for (const auto* name : {"generation", "committed"})。
    for (const auto* name : {"generation", "committed"})
        // 【逐行批注】 判断条件 (!value.at(name).is_number_integer() ||，成立时执行对应分支。
        if (!value.at(name).is_number_integer() ||
            // 【逐行批注】 执行当前声明或表达式：(!value.at(name).is_number_unsigned() && value.at(name).get<long long>() < 0))。
            (!value.at(name).is_number_unsigned() && value.at(name).get<long long>() < 0))
            // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("搜索缓存世代无效");。
            throw std::runtime_error("搜索缓存世代无效");
    // 【逐行批注】 结束当前函数并返回结果：return {value.at("generation").get<std::uint64_t>(), value.at("commit…。
    return {value.at("generation").get<std::uint64_t>(), value.at("committed").get<std::uint64_t>()};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 saveCacheVersion，执行该函数声明的职责。
void saveCacheVersion(const fs::path& path, const SearchCacheVersion& value) {
    // 【逐行批注】 调用 replaceCacheFile 完成本行所需操作：replaceCacheFile(path, {{"generation", value.generation}, {"committed…。
    replaceCacheFile(path, {{"generation", value.generation}, {"committed", value.committed}});
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明数据结构或类型 SearchCacheTicket 及其成员接口。
struct SearchCacheTicket {
    // 【逐行批注】 执行当前声明或表达式：std::string dir, key;。
    std::string dir, key;
    // 【逐行批注】 执行当前声明或表达式：SearchCacheVersion version;。
    SearchCacheVersion version;
    // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
    bool valid = false;
    // 【逐行批注】 执行当前声明或表达式：std::optional<std::vector<SearchResource>> cached;。
    std::optional<std::vector<SearchResource>> cached;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 currentCache，执行该函数声明的职责。
std::optional<std::vector<SearchResource>> currentCache(const SearchCacheTicket& ticket, long long ttlSeconds) {
    // 【逐行批注】 判断条件 (!ticket.valid) return std::nullopt;，成立时执行对应分支。
    if (!ticket.valid) return std::nullopt;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto root，更新当前运行状态。
        const auto root = fs::u8path(ticket.dir);
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        SearchCacheLock lock(root / (ticket.key + ".lock"));
        // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
        const auto current = cacheVersion(root / (ticket.key + ".state"));
        // 【逐行批注】 判断条件 (current.generation == ticket.version.generation) return readCac…，成立时执行对应分支。
        if (current.generation == ticket.version.generation) return readCache(ticket.dir, ticket.key, ttlSeconds);
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {}。
    } catch (...) {}
    // 【逐行批注】 结束当前函数并返回结果：return std::nullopt;。
    return std::nullopt;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 prepareCache，并通过构造函数初始化其状态。
SearchCacheTicket prepareCache(const std::string& dir, const std::string& key, long long ttlSeconds,
                              // 【逐行批注】 调用 bool 完成本行所需操作：bool refresh, const std::function<bool()>& cancelled) {。
                              bool refresh, const std::function<bool()>& cancelled) {
    // 【逐行批注】 执行当前声明或表达式：SearchCacheTicket ticket{dir, key, {}, false, std::nullopt};。
    SearchCacheTicket ticket{dir, key, {}, false, std::nullopt};
    // 显式空目录表示不使用磁盘缓存；此时刷新仍按普通网络请求执行。
    // 【逐行批注】 判断条件 (dir.empty()) return ticket;，成立时执行对应分支。
    if (dir.empty()) return ticket;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto root，更新当前运行状态。
        const auto root = fs::u8path(dir);
        // 【逐行批注】 调用 create_directories 完成本行所需操作：fs::create_directories(root);。
        fs::create_directories(root);
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        SearchCacheLock lock(root / (key + ".lock"), cancelled);
        // 【逐行批注】 计算右侧结果并写入 auto path，更新当前运行状态。
        const auto path = root / (key + ".state");
        // 【逐行批注】 计算右侧结果并写入 ticket.version，更新当前运行状态。
        ticket.version = cacheVersion(path);
        // 【逐行批注】 判断条件 (refresh) {，成立时执行对应分支。
        if (refresh) {
            // 【逐行批注】 判断条件 (ticket.version.generation == std::numeric_limits<std::uint64_t>…，成立时执行对应分支。
            if (ticket.version.generation == std::numeric_limits<std::uint64_t>::max()) return ticket;
            // 【逐行批注】 执行当前声明或表达式：++ticket.version.generation;。
            ++ticket.version.generation;
            // 【逐行批注】 调用 saveCacheVersion 完成本行所需操作：saveCacheVersion(path, ticket.version);。
            saveCacheVersion(path, ticket.version);
        // 【逐行批注】 计算右侧结果并写入 } else ticket.cached，更新当前运行状态。
        } else ticket.cached = readCache(dir, key, ttlSeconds);
        // 【逐行批注】 计算右侧结果并写入 ticket.valid，更新当前运行状态。
        ticket.valid = true;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) { /* 无法协调时不读取或写入缓存，普通搜索仍可返回实际网络结果。 * / }。
    } catch (...) { /* 无法协调时不读取或写入缓存，普通搜索仍可返回实际网络结果。 */ }
    // 【逐行批注】 结束当前函数并返回结果：return ticket;。
    return ticket;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 writeCache，并通过构造函数初始化其状态。
bool writeCache(const SearchCacheTicket& ticket, bool refresh, const std::vector<SearchResource>& resources,
                // 【逐行批注】 调用 bool 完成本行所需操作：const std::function<bool()>& cancelled,。
                const std::function<bool()>& cancelled,
                // 【逐行批注】 调用 bool、void 完成本行所需操作：const std::function<bool(const std::function<void()>&)>& publish) {。
                const std::function<bool(const std::function<void()>&)>& publish) {
    // 【逐行批注】 判断条件 (cancelled && cancelled()) return false;，成立时执行对应分支。
    if (cancelled && cancelled()) return false;
    // 【逐行批注】 判断条件 (!ticket.valid) return publish ? publish([] {}) : true;，成立时执行对应分支。
    if (!ticket.valid) return publish ? publish([] {}) : true;
    // 【逐行批注】 计算右侧结果并写入 bool committing，更新当前运行状态。
    bool committing = false;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto root，更新当前运行状态。
        const auto root = fs::u8path(ticket.dir);
        // 等待跨进程锁期间不能持有等待者互斥锁，最后一次取消必须能立即退出。
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        SearchCacheLock lock(root / (ticket.key + ".lock"), cancelled);
        // 【逐行批注】 判断条件 (cancelled && cancelled()) return false;，成立时执行对应分支。
        if (cancelled && cancelled()) return false;
        // 【逐行批注】 计算右侧结果并写入 auto commit，更新当前运行状态。
        const auto commit = [&] {
            // 【逐行批注】 计算右侧结果并写入 auto state，更新当前运行状态。
            const auto state = root / (ticket.key + ".state");
            // 【逐行批注】 计算右侧结果并写入 current，更新当前运行状态。
            auto current = cacheVersion(state);
            // 【逐行批注】 判断条件 (current.generation != ticket.version.generation ||，成立时执行对应分支。
            if (current.generation != ticket.version.generation ||
                // 【逐行批注】 执行当前声明或表达式：(!refresh && current.committed != ticket.version.committed) ||。
                (!refresh && current.committed != ticket.version.committed) ||
                // 【逐行批注】 调用 max 完成本行所需操作：current.committed == std::numeric_limits<std::uint64_t>::max()) retur…。
                current.committed == std::numeric_limits<std::uint64_t>::max()) return;
            // 此处是提交临界区的起点；之后的取消不能把已提交缓存说成未缓存。
            // 【逐行批注】 计算右侧结果并写入 committing，更新当前运行状态。
            committing = true;
            // 【逐行批注】 调用 saveCacheVersion 完成本行所需操作：++current.committed; saveCacheVersion(state, current);。
            ++current.committed; saveCacheVersion(state, current);
            // 【逐行批注】 调用 replaceCacheFile、serializeCache 完成本行所需操作：replaceCacheFile(root / (ticket.key + ".json"), serializeCache(resour…。
            replaceCacheFile(root / (ticket.key + ".json"), serializeCache(resources));
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 共享请求在此才持等待者锁，核对全部取消并提交；刷新则再次读取自己的取消状态。
        // 【逐行批注】 判断条件 (publish) return publish(commit);，成立时执行对应分支。
        if (publish) return publish(commit);
        // 【逐行批注】 判断条件 (cancelled && cancelled()) return false;，成立时执行对应分支。
        if (cancelled && cancelled()) return false;
        // 【逐行批注】 调用 commit 完成本行所需操作：commit(); return true;。
        commit(); return true;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 缓存失败不改变已校验的网络结果；仅在进入提交前取消时报告未提交。
        // 【逐行批注】 结束当前函数并返回结果：return committing || !(cancelled && cancelled());。
        return committing || !(cancelled && cancelled());
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 detectQueries，执行该函数声明的职责。
std::vector<std::string> detectQueries(const std::string& domain, const std::string& goal) {
    // 【逐行批注】 结束当前函数并返回结果：return domainQueries(domain, goal);。
    return domainQueries(domain, goal);
// 【逐行批注】 结束当前代码块。
}

// 搜索结果使用规范链接去重；原网站与搜索引擎来源分别保留。
// 【逐行批注】 定义函数 searchURL，执行该函数声明的职责。
std::string searchURL(std::string url) {
    // 【逐行批注】 计算右侧结果并写入 url，更新当前运行状态。
    url = trim(url.substr(0, url.find('#')));
    // 【逐行批注】 计算右侧结果并写入 auto scheme，更新当前运行状态。
    const auto scheme = url.find("://");
    // 【逐行批注】 判断条件 (scheme == std::string::npos) return {};，成立时执行对应分支。
    if (scheme == std::string::npos) return {};
    // 【逐行批注】 计算右侧结果并写入 auto protocol，更新当前运行状态。
    const auto protocol = lower(url.substr(0, scheme));
    // 【逐行批注】 判断条件 (protocol != "http" && protocol != "https") return {};，成立时执行对应分支。
    if (protocol != "http" && protocol != "https") return {};
    // 【逐行批注】 计算右侧结果并写入 auto hostEnd，更新当前运行状态。
    const auto hostEnd = url.find_first_of("/?", scheme + 3);
    // 【逐行批注】 计算右侧结果并写入 auto authority，更新当前运行状态。
    const auto authority = url.substr(scheme + 3, hostEnd == std::string::npos ? std::string::npos : hostEnd - scheme - 3);
    // 【逐行批注】 判断条件 (authority.empty() || authority.find('@') != std::string::npos) …，成立时执行对应分支。
    if (authority.empty() || authority.find('@') != std::string::npos) return {};
    // 【逐行批注】 计算右侧结果并写入 std::string host，更新当前运行状态。
    std::string host = lower(authority);
    // 【逐行批注】 计算右侧结果并写入 auto defaultPort，更新当前运行状态。
    const auto defaultPort = protocol == "https" ? ":443" : ":80";
    // 【逐行批注】 判断条件 (host.size() > std::char_traits<char>::length(defaultPort) &&，成立时执行对应分支。
    if (host.size() > std::char_traits<char>::length(defaultPort) &&
        // 【逐行批注】 调用 length 完成本行所需操作：host.compare(host.size() - std::char_traits<char>::length(defaultPort…。
        host.compare(host.size() - std::char_traits<char>::length(defaultPort), std::string::npos, defaultPort) == 0)
        // 【逐行批注】 调用 length 完成本行所需操作：host.resize(host.size() - std::char_traits<char>::length(defaultPort)…。
        host.resize(host.size() - std::char_traits<char>::length(defaultPort));
    // 【逐行批注】 计算右侧结果并写入 suffix，更新当前运行状态。
    auto suffix = hostEnd == std::string::npos ? std::string() : url.substr(hostEnd);
    // 【逐行批注】 计算右侧结果并写入 auto queryStart，更新当前运行状态。
    const auto queryStart = suffix.find('?');
    // 【逐行批注】 计算右侧结果并写入 path，更新当前运行状态。
    auto path = suffix.substr(0, queryStart);
    // 【逐行批注】 判断条件 (path.empty()) path = "/";，成立时执行对应分支。
    if (path.empty()) path = "/";
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> args;。
    std::vector<std::string> args;
    // 【逐行批注】 判断条件 (queryStart != std::string::npos) {，成立时执行对应分支。
    if (queryStart != std::string::npos) {
        // 【逐行批注】 声明对象 query，并通过构造函数初始化其状态。
        std::istringstream query(suffix.substr(queryStart + 1));
        // 【逐行批注】 执行当前声明或表达式：std::string item;。
        std::string item;
        // 【逐行批注】 在条件成立期间循环处理：while (std::getline(query, item, '&')) {。
        while (std::getline(query, item, '&')) {
            // 【逐行批注】 计算右侧结果并写入 auto name，更新当前运行状态。
            const auto name = lower(item.substr(0, item.find('=')));
            // 【逐行批注】 判断条件 (name.rfind("utm_", 0) != 0 && name != "gclid" && name != "fbcli…，成立时执行对应分支。
            if (name.rfind("utm_", 0) != 0 && name != "gclid" && name != "fbclid" && !item.empty()) args.push_back(item);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 std::string result，更新当前运行状态。
    std::string result = protocol + "://" + host + path;
    // 【逐行批注】 按循环规则重复处理：for (std::size_t i = 0; i < args.size(); ++i) result += (i ? "&" : "?") + …。
    for (std::size_t i = 0; i < args.size(); ++i) result += (i ? "&" : "?") + args[i];
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 utcDate，执行该函数声明的职责。
std::string utcDate(std::chrono::system_clock::time_point value) {
    // 【逐行批注】 计算右侧结果并写入 auto raw，更新当前运行状态。
    const auto raw = std::chrono::system_clock::to_time_t(value);
    // 【逐行批注】 执行当前声明或表达式：std::tm parts{};。
    std::tm parts{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&parts, &raw);。
    gmtime_s(&parts, &raw);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&raw, &parts);。
    gmtime_r(&raw, &parts);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream output;。
    std::ostringstream output;
    // 【逐行批注】 调用 put_time 完成本行所需操作：output << std::put_time(&parts, "%Y-%m-%d");。
    output << std::put_time(&parts, "%Y-%m-%d");
    // 【逐行批注】 结束当前函数并返回结果：return output.str();。
    return output.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 deepseekQuery，执行该函数声明的职责。
std::string deepseekQuery(const ResourceQuery& request) {
    // 【逐行批注】 计算右侧结果并写入 query，更新当前运行状态。
    auto query = trim(request.query);
    // 【逐行批注】 判断条件 (request.freshness != "noLimit") {，成立时执行对应分支。
    if (request.freshness != "noLimit") {
        // 【逐行批注】 执行当前声明或表达式：const std::map<std::string, int> days{{"oneDay", 1}, {"oneWeek", 7}, {"oneMonth", 30…。
        const std::map<std::string, int> days{{"oneDay", 1}, {"oneWeek", 7}, {"oneMonth", 30}, {"oneYear", 365}};
        // 【逐行批注】 计算右侧结果并写入 auto found，更新当前运行状态。
        const auto found = days.find(request.freshness);
        // 【逐行批注】 判断条件 (found != days.end()) query += " after:" + utcDate(std::chrono::…，成立时执行对应分支。
        if (found != days.end()) query += " after:" + utcDate(std::chrono::system_clock::now() - std::chrono::hours(24 * found->second));
        // 【逐行批注】 执行前一条件不成立时的分支。
        else {
            // 【逐行批注】 计算右侧结果并写入 query +，更新当前运行状态。
            query += " after:" + request.freshness.substr(0, 10);
            // 【逐行批注】 判断条件 (request.freshness.size() == 22) query += " before:" + request.f…，成立时执行对应分支。
            if (request.freshness.size() == 22) query += " before:" + request.freshness.substr(12, 10);
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // API 以 Unicode 字符计数，不按 UTF-8 字节截断或改写用户搜索词。
    // 【逐行批注】 计算右侧结果并写入 std::size_t characters，更新当前运行状态。
    std::size_t characters = 0;
    // 【逐行批注】 按循环规则重复处理：for (const unsigned char byte : query) if ((byte & 0xc0) != 0x80) ++charac…。
    for (const unsigned char byte : query) if ((byte & 0xc0) != 0x80) ++characters;
    // 【逐行批注】 结束当前函数并返回结果：return characters <= 128 ? query : std::string();。
    return characters <= 128 ? query : std::string();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明数据结构或类型 ProviderConfiguration 及其成员接口。
struct ProviderConfiguration {
    // 【逐行批注】 执行当前声明或表达式：std::string base;。
    std::string base;
    // 【逐行批注】 执行当前声明或表达式：std::string key;。
    std::string key;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 providerConfiguration，执行该函数声明的职责。
ProviderConfiguration providerConfiguration(const std::string& provider) {
    // 【逐行批注】 计算右侧结果并写入 bool deepseek，更新当前运行状态。
    const bool deepseek = provider == "deepseek";
    // 【逐行批注】 计算右侧结果并写入 base，更新当前运行状态。
    auto base = trim(env(deepseek ? "DEEPSEEK_SEARCH_BASE_URL" : "BOCHA_BASE_URL",
                         // 【逐行批注】 执行当前声明或表达式：deepseek ? "https://api.deepseek.com" : "https://api.bochaai.com"));。
                         deepseek ? "https://api.deepseek.com" : "https://api.bochaai.com"));
    // 【逐行批注】 在条件成立期间循环处理：while (!base.empty() && base.back() == '/') base.pop_back();。
    while (!base.empty() && base.back() == '/') base.pop_back();
    // 【逐行批注】 判断条件 (deepseek && base.size() >= 3 && base.compare(base.size() - 3, 3…，成立时执行对应分支。
    if (deepseek && base.size() >= 3 && base.compare(base.size() - 3, 3, "/v1") == 0) base.resize(base.size() - 3);
    // 【逐行批注】 计算右侧结果并写入 key，更新当前运行状态。
    auto key = trim(env(deepseek ? "DEEPSEEK_SEARCH_API_KEY" : "BOCHA_API_KEY"));
    // 【逐行批注】 判断条件 (deepseek && key.empty()) key = trim(env("DEEPSEEK_API_KEY"));，成立时执行对应分支。
    if (deepseek && key.empty()) key = trim(env("DEEPSEEK_API_KEY"));
    // 只复用官方 DeepSeek 的模型凭据，其他模型服务商的密钥不会发给 DeepSeek。
    // 【逐行批注】 判断条件 (deepseek && key.empty() && hostFromUrl(env("AI_BASE_URL", "http…，成立时执行对应分支。
    if (deepseek && key.empty() && hostFromUrl(env("AI_BASE_URL", "https://api.deepseek.com")) == "api.deepseek.com")
        // 【逐行批注】 计算右侧结果并写入 key，更新当前运行状态。
        key = trim(env("AI_API_KEY"));
    // 【逐行批注】 结束当前函数并返回结果：return {base, key};。
    return {base, key};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 providerQueryOnce，并通过构造函数初始化其状态。
ResourceQueryResult providerQueryOnce(const std::string& provider, const ResourceQuery& request,
                                    // 【逐行批注】 执行当前声明或表达式：const SearchCacheTicket& cache, const ProviderConfiguration& configuration,。
                                    const SearchCacheTicket& cache, const ProviderConfiguration& configuration,
                                    // 【逐行批注】 调用 bool 完成本行所需操作：const std::function<bool()>& started = {},。
                                    const std::function<bool()>& started = {},
                                    // 【逐行批注】 调用 bool、void 完成本行所需操作：const std::function<bool(const std::function<void()>&)>& publish = {}…。
                                    const std::function<bool(const std::function<void()>&)>& publish = {}) {
    // 【逐行批注】 执行当前声明或表达式：ResourceQueryResult result;。
    ResourceQueryResult result;
    // 【逐行批注】 计算右侧结果并写入 result.status，更新当前运行状态。
    result.status = "missing_key";
    // 【逐行批注】 计算右侧结果并写入 bool deepseek，更新当前运行状态。
    const bool deepseek = provider == "deepseek";
    // 【逐行批注】 计算右侧结果并写入 auto& base，更新当前运行状态。
    const auto& base = configuration.base;
    // 【逐行批注】 计算右侧结果并写入 auto& key，更新当前运行状态。
    const auto& key = configuration.key;
    // 【逐行批注】 判断条件 (key.empty()) return result;，成立时执行对应分支。
    if (key.empty()) return result;
    // 【逐行批注】 判断条件 (request.cancelled && request.cancelled()) { result.status = "ca…，成立时执行对应分支。
    if (request.cancelled && request.cancelled()) { result.status = "cancelled"; return result; }
    // 【逐行批注】 计算右侧结果并写入 auto query，更新当前运行状态。
    const auto query = deepseek ? deepseekQuery(request) : request.query;
    // 【逐行批注】 判断条件 (query.empty()) { result.status = "query_too_long"; return resul…，成立时执行对应分支。
    if (query.empty()) { result.status = "query_too_long"; return result; }
    // 准备票据与进入工作线程之间，另一请求可能刚完成；再次读取避免重复发送。
    // 【逐行批注】 判断条件 (!request.refresh) if (const auto cached = currentCache(cache, r…，成立时执行对应分支。
    if (!request.refresh) if (const auto cached = currentCache(cache, request.freshness == "noLimit" ? 86400LL : 3600LL)) {
        // 【逐行批注】 计算右侧结果并写入 result.resources，更新当前运行状态。
        result.resources = *cached; result.status = "completed"; result.cached = true; return result;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 json body，更新当前运行状态。
    json body = deepseek ? json{{"query", query}, {"count", request.count}, {"need_content", false}} :
        // 【逐行批注】 执行当前声明或表达式：json{{"query", query}, {"freshness", request.freshness}, {"count", request.count}, {…。
        json{{"query", query}, {"freshness", request.freshness}, {"count", request.count}, {"summary", true}};
    // 【逐行批注】 计算右侧结果并写入 auto response，更新当前运行状态。
    const auto response = postJson(base + (deepseek ? "/v1/tools/search" : "/v1/web-search"), key, body,
        // 【逐行批注】 调用 clamp、env_ll 完成本行所需操作：static_cast<long>(std::clamp(env_ll("RESOURCE_SEARCH_TIMEOUT_MS", 150…。
        static_cast<long>(std::clamp(env_ll("RESOURCE_SEARCH_TIMEOUT_MS", 15000LL), 1000LL, 60000LL)), request.cancelled, started);
    // 【逐行批注】 计算右侧结果并写入 result.attempts，更新当前运行状态。
    result.attempts = response.attempts;
    // 【逐行批注】 判断条件 (response.error == CURLE_ABORTED_BY_CALLBACK) result.status = "c…，成立时执行对应分支。
    if (response.budgetDenied) result.status = "money_budget";
    else if (response.error == CURLE_ABORTED_BY_CALLBACK) result.status = "cancelled";
    // 【逐行批注】 在前一条件不成立时继续判断：(response.error != CURLE_OK) result.status = "network_error";。
    else if (response.error != CURLE_OK) result.status = "network_error";
    // 【逐行批注】 在前一条件不成立时继续判断：(response.status < 200 || response.status >= 300) result.status …。
    else if (response.status < 200 || response.status >= 300) result.status = "http_" + std::to_string(response.status);
    // 【逐行批注】 执行前一条件不成立时的分支。
    else {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 auto parsed，更新当前运行状态。
            const auto parsed = json::parse(response.body);
            // 【逐行批注】 判断条件 (!parsed.is_object()) throw std::runtime_error("搜索响应不是对象");，成立时执行对应分支。
            if (!parsed.is_object()) throw std::runtime_error("搜索响应不是对象");
            // 【逐行批注】 判断条件 (deepseek) {，成立时执行对应分支。
            if (deepseek) {
                // 【逐行批注】 判断条件 (!parsed.contains("results") || !parsed["results"].is_array()) t…，成立时执行对应分支。
                if (!parsed.contains("results") || !parsed["results"].is_array()) throw std::runtime_error("缺少搜索结果数组");
                // 【逐行批注】 按循环规则重复处理：for (const auto& item : parsed["results"]) {。
                for (const auto& item : parsed["results"]) {
                    // 【逐行批注】 判断条件 (!item.is_object() || !item.value("url", json()).is_string() || …，成立时执行对应分支。
                    if (!item.is_object() || !item.value("url", json()).is_string() || !item.value("title", json()).is_string())
                        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("搜索条目结构无效");。
                        throw std::runtime_error("搜索条目结构无效");
                    // 【逐行批注】 执行当前声明或表达式：SearchResource resource;。
                    SearchResource resource;
                    // 【逐行批注】 计算右侧结果并写入 resource.title，更新当前运行状态。
                    resource.title = item.at("title").get<std::string>(); resource.url = searchURL(item.at("url").get<std::string>());
                    // 【逐行批注】 计算右侧结果并写入 resource.description，更新当前运行状态。
                    resource.description = item.value("summary", json()).is_string() ? item["summary"].get<std::string>() : item.value("snippet", "");
                    // 【逐行批注】 计算右侧结果并写入 resource.source，更新当前运行状态。
                    resource.source = item.value("site_name", json()).is_string() ? item["site_name"].get<std::string>() : "";
                    // 【逐行批注】 计算右侧结果并写入 resource.publishedAt，更新当前运行状态。
                    resource.publishedAt = item.value("date", json()).is_string() ? item["date"].get<std::string>() : "";
                    // 【逐行批注】 计算右侧结果并写入 resource.score，更新当前运行状态。
                    resource.score = item.value("rerank_score", json()).is_number() ? std::clamp(item["rerank_score"].get<double>() / 3.0 * 100.0, 0.0, 100.0) : 0.0;
                    // 【逐行批注】 调用 move 完成本行所需操作：result.resources.push_back(std::move(resource));。
                    result.resources.push_back(std::move(resource));
                // 【逐行批注】 结束当前代码块。
                }
            // 【逐行批注】 执行当前声明或表达式：} else {。
            } else {
                // 【逐行批注】 判断条件 (parsed.contains("code") && parsed["code"] != 200) throw std::ru…，成立时执行对应分支。
                if (parsed.contains("code") && parsed["code"] != 200) throw std::runtime_error("博查返回错误");
                // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
                const bool valid = (parsed.value("data", json()).is_object() &&
                    // 【逐行批注】 调用 json 完成本行所需操作：((parsed["data"].value("webPages", json()).is_object() && parsed["dat…。
                    ((parsed["data"].value("webPages", json()).is_object() && parsed["data"]["webPages"].value("value", json()).is_array()) ||
                     // 【逐行批注】 调用 json 完成本行所需操作：parsed["data"].value("results", json()).is_array())) || parsed.value(…。
                     parsed["data"].value("results", json()).is_array())) || parsed.value("results", json()).is_array();
                // 【逐行批注】 判断条件 (!valid) throw std::runtime_error("缺少博查结果数组");，成立时执行对应分支。
                if (!valid) throw std::runtime_error("缺少博查结果数组");
                // 【逐行批注】 计算右侧结果并写入 result.resources，更新当前运行状态。
                result.resources = parseBocha(response.body);
                // 【逐行批注】 按循环规则重复处理：for (auto& resource : result.resources) resource.url = searchURL(resource.…。
                for (auto& resource : result.resources) resource.url = searchURL(resource.url);
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 result.resources，更新当前运行状态。
            result.resources = normalizeResources(std::move(result.resources), detectDomain(request.query));
            // 【逐行批注】 按循环规则重复处理：for (auto& resource : result.resources) resource.searchProviders = {provid…。
            for (auto& resource : result.resources) resource.searchProviders = {provider};
            // 【逐行批注】 判断条件 (request.cancelled && request.cancelled()) {，成立时执行对应分支。
            if (request.cancelled && request.cancelled()) {
                // 【逐行批注】 计算右侧结果并写入 result.resources.clear(); result.status，更新当前运行状态。
                result.resources.clear(); result.status = "cancelled";
            // 【逐行批注】 执行当前声明或表达式：} else {。
            } else {
                // 【逐行批注】 计算右侧结果并写入 bool saved，更新当前运行状态。
                const bool saved = writeCache(cache, request.refresh, result.resources, request.cancelled, publish);
                // 【逐行批注】 计算右侧结果并写入 result.status，更新当前运行状态。
                result.status = saved ? "completed" : "cancelled";
                // 【逐行批注】 判断条件 (!saved) result.resources.clear();，成立时执行对应分支。
                if (!saved) result.resources.clear();
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 计算右侧结果并写入 } catch (...) { result.resources.clear(); re…，更新当前运行状态。
        } catch (...) { result.resources.clear(); result.status = "invalid_response"; }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明数据结构或类型 SearchWaiters 及其成员接口。
struct SearchWaiters {
    // 【逐行批注】 执行当前声明或表达式：std::mutex mutex;。
    std::mutex mutex;
    // 【逐行批注】 计算右侧结果并写入 int active，更新当前运行状态。
    int active = 1;
    // 【逐行批注】 计算右侧结果并写入 int unreportedAttempts，更新当前运行状态。
    int unreportedAttempts = 0;
    // 【逐行批注】 计算右侧结果并写入 bool stopped，更新当前运行状态。
    bool stopped = false;
    // 【逐行批注】 定义函数 join，执行该函数声明的职责。
    bool join() {
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        std::lock_guard<std::mutex> lock(mutex);
        // 【逐行批注】 判断条件 (stopped) return false;，成立时执行对应分支。
        if (stopped) return false;
        // 【逐行批注】 执行当前声明或表达式：++active; return true;。
        ++active; return true;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 leave，执行该函数声明的职责。
    int leave() {
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        std::lock_guard<std::mutex> lock(mutex);
        // 【逐行批注】 判断条件 (--active == 0) stopped = true;，成立时执行对应分支。
        if (--active == 0) stopped = true;
        // 【逐行批注】 计算右侧结果并写入 auto attempts，更新当前运行状态。
        const auto attempts = unreportedAttempts;
        // 【逐行批注】 计算右侧结果并写入 unreportedAttempts，更新当前运行状态。
        unreportedAttempts = 0;
        // 【逐行批注】 结束当前函数并返回结果：return attempts;。
        return attempts;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 start，执行该函数声明的职责。
    bool start() {
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        std::lock_guard<std::mutex> lock(mutex);
        // 【逐行批注】 判断条件 (stopped || !evaluationSearchAttempt()) return false;，成立时执行对应分支。
        if (stopped || !evaluationSearchAttempt()) return false;
        // 【逐行批注】 执行当前声明或表达式：++unreportedAttempts; return true;。
        ++unreportedAttempts; return true;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 cancelled，执行该函数声明的职责。
    bool cancelled() {
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        std::lock_guard<std::mutex> lock(mutex);
        // 【逐行批注】 结束当前函数并返回结果：return stopped;。
        return stopped;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 定义函数 publish，执行该函数声明的职责。
    bool publish(const std::function<void()>& save) {
        // 跨进程锁已在外层取得；这里只串行最后一次取消与最终缓存提交。
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        std::lock_guard<std::mutex> lock(mutex);
        // 【逐行批注】 判断条件 (stopped) return false;，成立时执行对应分支。
        if (stopped) return false;
        // 【逐行批注】 调用 save 完成本行所需操作：save(); return true;。
        save(); return true;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 SearchFlight 及其成员接口。
struct SearchFlight {
    // 【逐行批注】 执行当前声明或表达式：std::shared_ptr<SearchWaiters> waiters = std::make_shared<SearchWaiters>();。
    std::shared_ptr<SearchWaiters> waiters = std::make_shared<SearchWaiters>();
    // 【逐行批注】 执行当前声明或表达式：std::shared_future<ResourceQueryResult> result;。
    std::shared_future<ResourceQueryResult> result;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 定义函数 providerQuery，执行该函数声明的职责。
ResourceQueryResult providerQuery(const std::string& provider, const ResourceQuery& request, const std::string& cacheDir) {
    // 【逐行批注】 计算右侧结果并写入 auto configuration，更新当前运行状态。
    const auto configuration = providerConfiguration(provider);
    // 【逐行批注】 执行当前声明或表达式：ResourceQueryResult immediate;。
    ResourceQueryResult immediate;
    // 【逐行批注】 判断条件 (configuration.key.empty()) { immediate.status = "missing_key"; …，成立时执行对应分支。
    if (configuration.key.empty()) { immediate.status = "missing_key"; return immediate; }
    // 【逐行批注】 判断条件 (request.cancelled && request.cancelled()) { immediate.status = …，成立时执行对应分支。
    if (request.cancelled && request.cancelled()) { immediate.status = "cancelled"; return immediate; }
    // 【逐行批注】 计算右侧结果并写入 auto query，更新当前运行状态。
    const auto query = provider == "deepseek" ? deepseekQuery(request) : request.query;
    // 【逐行批注】 判断条件 (query.empty()) { immediate.status = "query_too_long"; return im…，成立时执行对应分支。
    if (query.empty()) { immediate.status = "query_too_long"; return immediate; }
    // 新协议隔离不遵守世代锁的旧进程；升级首次检索会冷启动，字段范围和有效期不变。
    // 【逐行批注】 计算右侧结果并写入 auto cacheKey，更新当前运行状态。
    const auto cacheKey = sha256Hex("dual-v2:" + provider + ":" + configuration.base + ":" + sha256Hex(configuration.key) + ":" + query + ":" + request.freshness + ":" + std::to_string(request.count) +
        // 【逐行批注】 调用 string、sha256Hex 完成本行所需操作：(request.scopeId.empty() ? std::string() : ":scope:" + sha256Hex(requ…。
        (request.scopeId.empty() ? std::string() : ":scope:" + sha256Hex(request.scopeId)));
    // 【逐行批注】 计算右侧结果并写入 auto cache，更新当前运行状态。
    const auto cache = prepareCache(cacheDir, cacheKey, request.freshness == "noLimit" ? 86400LL : 3600LL, request.refresh, request.cancelled);
    // 【逐行批注】 判断条件 (request.cancelled && request.cancelled()) { immediate.status = …，成立时执行对应分支。
    if (request.cancelled && request.cancelled()) { immediate.status = "cancelled"; return immediate; }
    // 【逐行批注】 判断条件 (request.refresh && !cacheDir.empty() && !cache.valid) {，成立时执行对应分支。
    if (request.refresh && !cacheDir.empty() && !cache.valid) {
        // 未建立持久世代隔离的刷新不能伪称成功；不发送，也不叠加网络重试。
        // 【逐行批注】 计算右侧结果并写入 immediate.status，更新当前运行状态。
        immediate.status = "cache_unavailable"; return immediate;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (cache.cached) {，成立时执行对应分支。
    if (cache.cached) {
        // 【逐行批注】 计算右侧结果并写入 immediate.resources，更新当前运行状态。
        immediate.resources = *cache.cached; immediate.status = "completed"; immediate.cached = true; return immediate;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (request.refresh || env("GANGYI_SEARCH_SINGLEFLIGHT") == "0" || …，成立时执行对应分支。
    if (request.refresh || env("GANGYI_SEARCH_SINGLEFLIGHT") == "0" || (!cacheDir.empty() && !cache.valid))
        // 【逐行批注】 结束当前函数并返回结果：return providerQueryOnce(provider, request, cache, configuration);。
        return providerQueryOnce(provider, request, cache, configuration);
    // 【逐行批注】 声明数据结构或类型 Flights 及其成员接口。
    struct Flights {
        // 【逐行批注】 执行当前声明或表达式：std::mutex mutex;。
        std::mutex mutex;
        // 【逐行批注】 执行当前声明或表达式：std::map<std::string, std::shared_ptr<SearchFlight>> active;。
        std::map<std::string, std::shared_ptr<SearchFlight>> active;
        // 已取消的工作仍由有界网络超时收尾，调用者退出时无需等待异步析构。
        // 【逐行批注】 执行当前声明或表达式：std::vector<std::shared_ptr<SearchFlight>> retiring;。
        std::vector<std::shared_ptr<SearchFlight>> retiring;
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 执行当前声明或表达式：static Flights flights;。
    static Flights flights;
    // 【逐行批注】 计算右侧结果并写入 auto identity，更新当前运行状态。
    const auto identity = json{{"provider", provider}, {"base", configuration.base},
        // 【逐行批注】 调用 sha256Hex 完成本行所需操作：{"credential", sha256Hex(configuration.key)}, {"query", request.query…。
        {"credential", sha256Hex(configuration.key)}, {"query", request.query},
        // 【逐行批注】 调用 deepseekQuery 完成本行所需操作：{"effectiveQuery", provider == "deepseek" ? deepseekQuery(request) : …。
        {"effectiveQuery", provider == "deepseek" ? deepseekQuery(request) : request.query},
        // 【逐行批注】 执行当前声明或表达式：{"freshness", request.freshness}, {"count", request.count}, {"scope", request.scopeI…。
        {"freshness", request.freshness}, {"count", request.count}, {"scope", request.scopeId},
        // 【逐行批注】 调用 string、absolute、u8path 完成本行所需操作：{"cacheDir", cacheDir.empty() ? std::string() : fs::absolute(fs::u8pa…。
        {"cacheDir", cacheDir.empty() ? std::string() : fs::absolute(fs::u8path(cacheDir)).lexically_normal().u8string()},
        // 【逐行批注】 执行当前声明或表达式：{"cacheGeneration", cache.version.generation}}.dump();。
        {"cacheGeneration", cache.version.generation}}.dump();
    // 【逐行批注】 计算右侧结果并写入 auto identityKey，更新当前运行状态。
    const auto identityKey = sha256Hex(identity);
    // 【逐行批注】 执行当前声明或表达式：std::shared_ptr<SearchFlight> flight;。
    std::shared_ptr<SearchFlight> flight;
    // 【逐行批注】 计算右侧结果并写入 bool shared，更新当前运行状态。
    bool shared = false;
    // 【逐行批注】 开始当前代码块。
    {
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        std::lock_guard<std::mutex> lock(flights.mutex);
        // 【逐行批注】 计算右侧结果并写入 auto ready，更新当前运行状态。
        const auto ready = [](const auto& value) {
            // 【逐行批注】 结束当前函数并返回结果：return value->result.valid() && value->result.wait_for(std::chrono::m…。
            return value->result.valid() && value->result.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 调用 remove_if 完成本行所需操作：flights.retiring.erase(std::remove_if(flights.retiring.begin(), fligh…。
        flights.retiring.erase(std::remove_if(flights.retiring.begin(), flights.retiring.end(), ready), flights.retiring.end());
        // 【逐行批注】 按循环规则重复处理：for (auto it = flights.active.begin(); it != flights.active.end();) {。
        for (auto it = flights.active.begin(); it != flights.active.end();) {
            // 【逐行批注】 判断条件 (ready(it->second)) it = flights.active.erase(it); else ++it;，成立时执行对应分支。
            if (ready(it->second)) it = flights.active.erase(it); else ++it;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 auto existing，更新当前运行状态。
        const auto existing = flights.active.find(identityKey);
        // 【逐行批注】 判断条件 (existing != flights.active.end()) {，成立时执行对应分支。
        if (existing != flights.active.end()) {
            // 【逐行批注】 判断条件 (existing->second->waiters->join()) { flight = existing->second;…，成立时执行对应分支。
            if (existing->second->waiters->join()) { flight = existing->second; shared = true; }
            // 【逐行批注】 执行前一条件不成立时的分支。
            else { flights.retiring.push_back(existing->second); flights.active.erase(existing); }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (!flight) {，成立时执行对应分支。
        if (!flight) {
            // 【逐行批注】 计算右侧结果并写入 flight，更新当前运行状态。
            flight = std::make_shared<SearchFlight>();
            // 【逐行批注】 计算右侧结果并写入 work，更新当前运行状态。
            auto work = request;
            // 【逐行批注】 计算右侧结果并写入 work.cancelled，更新当前运行状态。
            work.cancelled = [waiters = flight->waiters] { return waiters->cancelled(); };
            // 【逐行批注】 调用 async 完成本行所需操作：flight->result = std::async(std::launch::async, [provider, work, cach…。
            flight->result = std::async(std::launch::async, [provider, work, cache, configuration, waiters = flight->waiters] {
                // 【逐行批注】 结束当前函数并返回结果：return providerQueryOnce(provider, work, cache, configuration, [waite…。
                return providerQueryOnce(provider, work, cache, configuration, [waiters] { return waiters->start(); },
                    // 【逐行批注】 调用 publish 完成本行所需操作：[waiters](const auto& save) { return waiters->publish(save); });。
                    [waiters](const auto& save) { return waiters->publish(save); });
            // 【逐行批注】 执行当前声明或表达式：}).share();。
            }).share();
            // 【逐行批注】 计算右侧结果并写入 flights.active[identityKey]，更新当前运行状态。
            flights.active[identityKey] = flight;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 声明数据结构或类型 Subscription 及其成员接口。
    struct Subscription {
        // 【逐行批注】 执行当前声明或表达式：std::shared_ptr<SearchWaiters> waiters;。
        std::shared_ptr<SearchWaiters> waiters;
        // 【逐行批注】 定义函数 finish，执行该函数声明的职责。
        int finish() { const auto attempts = waiters->leave(); waiters.reset(); return attempts; }
        // 【逐行批注】 调用 Subscription、leave 完成本行所需操作：~Subscription() { if (waiters) waiters->leave(); }。
        ~Subscription() { if (waiters) waiters->leave(); }
    // 【逐行批注】 执行当前声明或表达式：} subscription{flight->waiters};。
    } subscription{flight->waiters};
    // 【逐行批注】 在条件成立期间循环处理：while (true) {。
    while (true) {
        // 【逐行批注】 判断条件 (request.cancelled && request.cancelled()) {，成立时执行对应分支。
        if (request.cancelled && request.cancelled()) {
            // 【逐行批注】 计算右侧结果并写入 ResourceQueryResult result; result.status，更新当前运行状态。
            ResourceQueryResult result; result.status = "cancelled"; result.shared = shared;
            // 每笔实际请求只由一个回执领取；最后退出与发送使用同一把锁。
            // 【逐行批注】 计算右侧结果并写入 result.attempts，更新当前运行状态。
            result.attempts = subscription.finish();
            // 【逐行批注】 结束当前函数并返回结果：return result;。
            return result;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (flight->result.wait_for(std::chrono::milliseconds(20)) == std::…，成立时执行对应分支。
        if (flight->result.wait_for(std::chrono::milliseconds(20)) == std::future_status::ready) {
            // 【逐行批注】 执行当前声明或表达式：ResourceQueryResult result;。
            ResourceQueryResult result;
            // 【逐行批注】 开始可能抛出异常的保护代码块。
            try { result = flight->result.get(); } catch (...) { result.status = "internal_error"; }
            // 【逐行批注】 计算右侧结果并写入 result.shared，更新当前运行状态。
            result.shared = shared;
            // 【逐行批注】 计算右侧结果并写入 result.attempts，更新当前运行状态。
            result.attempts = subscription.finish();
            // 【逐行批注】 结束当前函数并返回结果：return result;。
            return result;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 调用 SearchClient 完成本行所需操作：SearchClient::SearchClient()。
SearchClient::SearchClient()
    // 【逐行批注】 声明对象 cacheDir_，并通过构造函数初始化其状态。
    : cacheDir_(env("RESOURCE_SEARCH_CACHE_DIR", env("LOCALAPPDATA").empty()
          // 【逐行批注】 调用 path、env 完成本行所需操作：? "data/resource-search-cache" : (fs::path(env("LOCALAPPDATA")) / "Ga…。
          ? "data/resource-search-cache" : (fs::path(env("LOCALAPPDATA")) / "GangyiAI" / "resource-search-cache").string())) {
    // 【逐行批注】 计算右侧结果并写入 const int initialized，更新当前运行状态。
    static const int initialized = curl_global_init(CURL_GLOBAL_DEFAULT);
    // 【逐行批注】 执行当前声明或表达式：(void)initialized;。
    (void)initialized;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 resourceDigest，执行该函数声明的职责。
std::string resourceDigest(const std::string& input) { return sha256Hex(input); }

// 【逐行批注】 定义函数 SearchClient::testBochaKey，执行该函数声明的职责。
bool SearchClient::testBochaKey(const std::string& key, std::string* diagnostic) {
    // 【逐行批注】 判断条件 (trim(key).empty()) { if (diagnostic) *diagnostic = "missing_key…，成立时执行对应分支。
    if (trim(key).empty()) { if (diagnostic) *diagnostic = "missing_key"; return false; }
    // 【逐行批注】 调用 curl_global_init 完成本行所需操作：curl_global_init(CURL_GLOBAL_DEFAULT);。
    curl_global_init(CURL_GLOBAL_DEFAULT);
    // 【逐行批注】 计算右侧结果并写入 std::string base，更新当前运行状态。
    std::string base = trim(env("BOCHA_BASE_URL", "https://api.bochaai.com"));
    // 【逐行批注】 在条件成立期间循环处理：while (!base.empty() && base.back() == '/') base.pop_back();。
    while (!base.empty() && base.back() == '/') base.pop_back();
    // 【逐行批注】 计算右侧结果并写入 auto response，更新当前运行状态。
    const auto response = postJson(base + "/v1/web-search", key,
        // 【逐行批注】 执行当前声明或表达式：{{"query", u8"高中数学知识点"}, {"count", 1}, {"summary", false}}, 8000);。
        {{"query", u8"高中数学知识点"}, {"count", 1}, {"summary", false}}, 8000);
    // 【逐行批注】 判断条件 (response.error != CURLE_OK) {，成立时执行对应分支。
    if (response.budgetDenied) {
        if (diagnostic) *diagnostic = "money_budget";
        return false;
    }
    if (response.error != CURLE_OK) {
        // 【逐行批注】 判断条件 (diagnostic) *diagnostic = "network_error:" + std::to_string(sta…，成立时执行对应分支。
        if (diagnostic) *diagnostic = "network_error:" + std::to_string(static_cast<int>(response.error));
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (response.status < 200 || response.status >= 300) {，成立时执行对应分支。
    if (response.status < 200 || response.status >= 300) {
        // 【逐行批注】 判断条件 (diagnostic) *diagnostic = "http:" + std::to_string(response.sta…，成立时执行对应分支。
        if (diagnostic) *diagnostic = "http:" + std::to_string(response.status);
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 auto body，更新当前运行状态。
    const auto body = json::parse(response.body, nullptr, false);
    // 【逐行批注】 计算右侧结果并写入 bool valid，更新当前运行状态。
    const bool valid = body.is_object() && (!body.contains("code") ||
        // 【逐行批注】 执行当前声明或表达式：(body["code"].is_number_integer() && body["code"].get<int>() == 200));。
        (body["code"].is_number_integer() && body["code"].get<int>() == 200));
    // 【逐行批注】 判断条件 (diagnostic) *diagnostic = valid ? (response.directRetry ? "ok_d…，成立时执行对应分支。
    if (diagnostic) *diagnostic = valid ? (response.directRetry ? "ok_direct" : "ok") : "invalid_response";
    // 【逐行批注】 结束当前函数并返回结果：return valid;。
    return valid;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 调用 search 完成本行所需操作：std::vector<SearchResource> SearchClient::search(const std::string& g…。
std::vector<SearchResource> SearchClient::search(const std::string& goal, size_t limit,
                                               // 【逐行批注】 调用 bool 完成本行所需操作：const std::function<bool()>& cancelled) const {。
                                               const std::function<bool()>& cancelled) const {
    // 【逐行批注】 执行当前声明或表达式：std::vector<SearchResource> collected;。
    std::vector<SearchResource> collected;
    // 【逐行批注】 计算右侧结果并写入 lastProvider_，更新当前运行状态。
    lastProvider_ = "deepseek+bocha"; fallbackUsed_ = false;
    // 【逐行批注】 判断条件 (limit == 0) return collected;，成立时执行对应分支。
    if (limit == 0) return collected;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 按循环规则重复处理：for (const auto& queryText : detectQueries(detectDomain(goal), goal)) {。
        for (const auto& queryText : detectQueries(detectDomain(goal), goal)) {
            // 【逐行批注】 判断条件 (cancelled && cancelled()) break;，成立时执行对应分支。
            if (cancelled && cancelled()) break;
            // 【逐行批注】 计算右侧结果并写入 ResourceQuery request; request.query，更新当前运行状态。
            ResourceQuery request; request.query = queryText; request.count = 5; request.cancelled = cancelled;
            // 【逐行批注】 计算右侧结果并写入 auto result，更新当前运行状态。
            const auto result = query(request);
            // 【逐行批注】 计算右侧结果并写入 fallbackUsed_，更新当前运行状态。
            fallbackUsed_ = fallbackUsed_ || result.status != "completed";
            // 【逐行批注】 执行当前声明或表达式：collected.insert(collected.end(), result.resources.begin(), result.resources.end());。
            collected.insert(collected.end(), result.resources.begin(), result.resources.end());
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 collected，更新当前运行状态。
        collected = normalizeResources(std::move(collected), detectDomain(goal));
        // 【逐行批注】 判断条件 (collected.size() > limit) collected.resize(limit);，成立时执行对应分支。
        if (collected.size() > limit) collected.resize(limit);
    // 【逐行批注】 计算右侧结果并写入 } catch (...) { fallbackUsed_，更新当前运行状态。
    } catch (...) { fallbackUsed_ = true; }
    // 【逐行批注】 结束当前函数并返回结果：return collected;。
    return collected;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 SearchClient::query，执行该函数声明的职责。
ResourceQueryResult SearchClient::query(const ResourceQuery& request) const {
    // 【逐行批注】 判断条件 (trim(request.query).empty() || request.query.size() > 3000 || r…，成立时执行对应分支。
    if (trim(request.query).empty() || request.query.size() > 3000 || request.count < 1 || request.count > 50)
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("搜索词或结果数量无效");。
        throw std::invalid_argument("搜索词或结果数量无效");
    // 【逐行批注】 执行当前声明或表达式：const std::set<std::string> ranges{"noLimit", "oneYear", "oneMonth", "oneWeek", "one…。
    const std::set<std::string> ranges{"noLimit", "oneYear", "oneMonth", "oneWeek", "oneDay"};
    // 【逐行批注】 计算右侧结果并写入 bool customDate，更新当前运行状态。
    const bool customDate = (request.freshness.size() == 10 || request.freshness.size() == 22) &&
        // 【逐行批注】 执行当前声明或表达式：request.freshness.find_first_not_of("0123456789-.") == std::string::npos;。
        request.freshness.find_first_not_of("0123456789-.") == std::string::npos;
    // 【逐行批注】 判断条件 (!ranges.count(request.freshness) && !customDate) throw std::inv…，成立时执行对应分支。
    if (!ranges.count(request.freshness) && !customDate) throw std::invalid_argument("检索日期范围无效");
    // 两个网络线程共享取消回调时串行读取档案，避免并发操作同一 SQLite 连接。
    // 【逐行批注】 计算右侧结果并写入 guarded，更新当前运行状态。
    auto guarded = request;
    // 【逐行批注】 计算右侧结果并写入 auto mutex，更新当前运行状态。
    const auto mutex = std::make_shared<std::mutex>();
    // 【逐行批注】 计算右侧结果并写入 guarded.cancelled，更新当前运行状态。
    guarded.cancelled = [mutex, callback = request.cancelled] {
        // 【逐行批注】 声明对象 lock，并通过构造函数初始化其状态。
        std::lock_guard<std::mutex> lock(*mutex);
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { return callback && callback(); } catch (...) { return true; }
    // 【逐行批注】 执行当前声明或表达式：};。
    };
    // 【逐行批注】 判断条件 (guarded.cancelled()) throw std::runtime_error("资料检索已中断");，成立时执行对应分支。
    if (guarded.cancelled()) throw std::runtime_error("资料检索已中断");
    // 【逐行批注】 计算右侧结果并写入 deepseek，更新当前运行状态。
    auto deepseek = std::async(std::launch::async, [&] { return providerQuery("deepseek", guarded, cacheDir_); });
    // 【逐行批注】 计算右侧结果并写入 bocha，更新当前运行状态。
    auto bocha = std::async(std::launch::async, [&] { return providerQuery("bocha", guarded, cacheDir_); });
    // 【逐行批注】 执行当前声明或表达式：ResourceQueryResult result;。
    ResourceQueryResult result;
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, SearchResource> merged;。
    std::map<std::string, SearchResource> merged;
    // 【逐行批注】 执行当前声明或表达式：std::map<std::string, double> ranks;。
    std::map<std::string, double> ranks;
    // 【逐行批注】 计算右侧结果并写入 int completed，更新当前运行状态。
    int completed = 0;
    // 【逐行批注】 按循环规则重复处理：for (const auto& provider : {std::string("deepseek"), std::string("bocha")…。
    for (const auto& provider : {std::string("deepseek"), std::string("bocha")}) {
        // 【逐行批注】 执行当前声明或表达式：ResourceQueryResult part;。
        ResourceQueryResult part;
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { part = provider == "deepseek" ? deepseek.get() : bocha.get(); }
        // 【逐行批注】 捕获并处理异常：catch (...) { part.status = "internal_error"; }。
        catch (...) { part.status = "internal_error"; }
        // 【逐行批注】 计算右侧结果并写入 result.attempts +，更新当前运行状态。
        result.attempts += part.attempts;
        // 【逐行批注】 计算右侧结果并写入 result.shared，更新当前运行状态。
        result.shared = result.shared || part.shared;
        // 【逐行批注】 判断条件 (part.status == "completed") ++completed;，成立时执行对应分支。
        if (part.status == "completed") ++completed;
        // 【逐行批注】 执行当前声明或表达式：result.providers.push_back({{"provider", provider}, {"status", part.status}, {"attem…。
        result.providers.push_back({{"provider", provider}, {"status", part.status}, {"attempts", part.attempts},
            // 【逐行批注】 执行当前声明或表达式：{"cached", part.cached}, {"shared", part.shared},。
            {"cached", part.cached}, {"shared", part.shared},
            // 【逐行批注】 执行当前声明或表达式：{"reuseReason", part.shared ? "in_flight" : part.cached ? "cache" : "none"}, {"resul…。
            {"reuseReason", part.shared ? "in_flight" : part.cached ? "cache" : "none"}, {"results", part.resources.size()}});
        // 【逐行批注】 计算右侧结果并写入 std::size_t rank，更新当前运行状态。
        std::size_t rank = 0;
        // 【逐行批注】 按循环规则重复处理：for (auto& resource : part.resources) {。
        for (auto& resource : part.resources) {
            // 【逐行批注】 计算右侧结果并写入 auto key，更新当前运行状态。
            const auto key = resource.url;
            // 采用名次融合，不直接比较两家不同量纲的原始分数。
            // 【逐行批注】 计算右侧结果并写入 ranks[key] +，更新当前运行状态。
            ranks[key] += 1.0 / (60.0 + static_cast<double>(++rank));
            // 【逐行批注】 判断条件 (!merged.count(key)) merged[key] = resource;，成立时执行对应分支。
            if (!merged.count(key)) merged[key] = resource;
            // 【逐行批注】 计算右侧结果并写入 auto& saved，更新当前运行状态。
            auto& saved = merged[key];
            // 【逐行批注】 判断条件 (std::find(saved.searchProviders.begin(), saved.searchProviders.…，成立时执行对应分支。
            if (std::find(saved.searchProviders.begin(), saved.searchProviders.end(), provider) == saved.searchProviders.end()) saved.searchProviders.push_back(provider);
            // 【逐行批注】 判断条件 (saved.description.size() < resource.description.size()) saved.d…，成立时执行对应分支。
            if (saved.description.size() < resource.description.size()) saved.description = resource.description;
            // 【逐行批注】 判断条件 (saved.publishedAt.empty()) saved.publishedAt = resource.publish…，成立时执行对应分支。
            if (saved.publishedAt.empty()) saved.publishedAt = resource.publishedAt;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 result.status，更新当前运行状态。
    result.status = guarded.cancelled() ? "cancelled" : completed == 2 ? "completed" : completed == 1 ? "partial" : "failed";
    // 【逐行批注】 计算右侧结果并写入 result.cached，更新当前运行状态。
    result.cached = std::all_of(result.providers.begin(), result.providers.end(), [](const auto& item) { return item.at("cached") == true; });
    // 【逐行批注】 判断条件 (result.status != "cancelled") for (auto& [key, resource] : merg…，成立时执行对应分支。
    if (result.status != "cancelled") for (auto& [key, resource] : merged) {
        // 【逐行批注】 计算右侧结果并写入 resource.score，更新当前运行状态。
        resource.score = ranks[key] * 3050.0;
        // 【逐行批注】 调用 move 完成本行所需操作：result.resources.push_back(std::move(resource));。
        result.resources.push_back(std::move(resource));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 result.resources，更新当前运行状态。
    result.resources = normalizeResources(std::move(result.resources), detectDomain(request.query));
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

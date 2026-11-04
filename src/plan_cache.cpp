// 【逐行批注】 引入头文件 "plan_cache.hpp"，为后续实现提供类型或接口声明。
#include "plan_cache.hpp"

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <array>，为后续实现提供类型或接口声明。
#include <array>
// 【逐行批注】 引入头文件 <cctype>，为后续实现提供类型或接口声明。
#include <cctype>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <cmath>，为后续实现提供类型或接口声明。
#include <cmath>
// 【逐行批注】 引入头文件 <cstdint>，为后续实现提供类型或接口声明。
#include <cstdint>
// 【逐行批注】 引入头文件 <ctime>，为后续实现提供类型或接口声明。
#include <ctime>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <optional>，为后续实现提供类型或接口声明。
#include <optional>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

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

// 【逐行批注】 定义函数 trim，执行该函数声明的职责。
std::string trim(const std::string& input) {
    // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
    const auto first = input.find_first_not_of(" \t\r\n\f\v");
    // 【逐行批注】 判断条件 (first == std::string::npos) return {};，成立时执行对应分支。
    if (first == std::string::npos) return {};
    // 【逐行批注】 计算右侧结果并写入 auto last，更新当前运行状态。
    const auto last = input.find_last_not_of(" \t\r\n\f\v");
    // 【逐行批注】 结束当前函数并返回结果：return input.substr(first, last - first + 1);。
    return input.substr(first, last - first + 1);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 collapseWhitespace，执行该函数声明的职责。
std::string collapseWhitespace(std::string value) {
    // 【逐行批注】 执行当前声明或表达式：std::string out;。
    std::string out;
    // 【逐行批注】 执行当前声明或表达式：out.reserve(value.size());。
    out.reserve(value.size());
    // 【逐行批注】 计算右侧结果并写入 bool inSpace，更新当前运行状态。
    bool inSpace = false;
    // 【逐行批注】 按循环规则重复处理：for (unsigned char ch : value) {。
    for (unsigned char ch : value) {
        // 【逐行批注】 判断条件 (std::isspace(ch)) {，成立时执行对应分支。
        if (std::isspace(ch)) {
            // 【逐行批注】 计算右侧结果并写入 inSpace，更新当前运行状态。
            inSpace = true;
            // 【逐行批注】 跳过本轮剩余步骤并进入下一次循环。
            continue;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (inSpace && !out.empty()) out.push_back(' ');，成立时执行对应分支。
        if (inSpace && !out.empty()) out.push_back(' ');
        // 【逐行批注】 执行当前声明或表达式：out.push_back(static_cast<char>(ch));。
        out.push_back(static_cast<char>(ch));
        // 【逐行批注】 计算右侧结果并写入 inSpace，更新当前运行状态。
        inSpace = false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return out;。
    return out;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 lowerAscii，执行该函数声明的职责。
std::string lowerAscii(std::string value) {
    // 【逐行批注】 按循环规则重复处理：for (char& ch : value) {。
    for (char& ch : value) {
        // 【逐行批注】 计算右侧结果并写入 ch，更新当前运行状态。
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 normalizeMode，执行该函数声明的职责。
std::string normalizeMode(const std::string& mode) {
    // 【逐行批注】 计算右侧结果并写入 std::string lowered，更新当前运行状态。
    const std::string lowered = lowerAscii(trim(collapseWhitespace(mode)));
    // 【逐行批注】 结束当前函数并返回结果：return lowered == "lite" ? "lite" : "deep";。
    return lowered == "lite" ? "lite" : "deep";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 trimGoalText，执行该函数声明的职责。
std::string trimGoalText(const std::string& goal) {
    // 【逐行批注】 结束当前函数并返回结果：return trim(goal);。
    return trim(goal);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 nowIso8601，执行该函数声明的职责。
std::string nowIso8601() {
    // 【逐行批注】 计算右侧结果并写入 auto now，更新当前运行状态。
    const auto now = std::chrono::system_clock::now();
    // 【逐行批注】 计算右侧结果并写入 std::time_t time，更新当前运行状态。
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    // 【逐行批注】 执行当前声明或表达式：std::tm tm{};。
    std::tm tm{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&tm, &time);。
    gmtime_s(&tm, &time);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&time, &tm);。
    gmtime_r(&time, &tm);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream out;。
    std::ostringstream out;
    // 【逐行批注】 调用 put_time 完成本行所需操作：out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");。
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    // 【逐行批注】 结束当前函数并返回结果：return out.str();。
    return out.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 parseIso8601Utc，执行该函数声明的职责。
bool parseIso8601Utc(const std::string& value, std::time_t& out) {
    // 【逐行批注】 执行当前声明或表达式：std::tm tm{};。
    std::tm tm{};
    // 【逐行批注】 声明对象 in，并通过构造函数初始化其状态。
    std::istringstream in(value);
    // 【逐行批注】 调用 get_time 完成本行所需操作：in >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");。
    in >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    // 【逐行批注】 判断条件 (in.fail()) return false;，成立时执行对应分支。
    if (in.fail()) return false;
// 【逐行批注】 处理条件编译指令：#if defined(_WIN32)。
#if defined(_WIN32)
    // 【逐行批注】 计算右侧结果并写入 out，更新当前运行状态。
    out = _mkgmtime(&tm);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 计算右侧结果并写入 out，更新当前运行状态。
    out = timegm(&tm);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 结束当前函数并返回结果：return out != static_cast<std::time_t>(-1);。
    return out != static_cast<std::time_t>(-1);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 hexEncode，执行该函数声明的职责。
std::string hexEncode(const std::array<unsigned char, 32>& digest) {
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream out;。
    std::ostringstream out;
    // 【逐行批注】 调用 setfill 完成本行所需操作：out << std::hex << std::setfill('0');。
    out << std::hex << std::setfill('0');
    // 【逐行批注】 按循环规则重复处理：for (unsigned char byte : digest) {。
    for (unsigned char byte : digest) {
        // 【逐行批注】 调用 setw 完成本行所需操作：out << std::setw(2) << static_cast<int>(byte);。
        out << std::setw(2) << static_cast<int>(byte);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return out.str();。
    return out.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 sha256，执行该函数声明的职责。
std::array<unsigned char, 32> sha256(const std::string& input) {
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
    // 【逐行批注】 计算右侧结果并写入 big0，更新当前运行状态。
    auto big0 = [&](uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); };
    // 【逐行批注】 计算右侧结果并写入 big1，更新当前运行状态。
    auto big1 = [&](uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); };
    // 【逐行批注】 计算右侧结果并写入 small0，更新当前运行状态。
    auto small0 = [&](uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); };
    // 【逐行批注】 计算右侧结果并写入 small1，更新当前运行状态。
    auto small1 = [&](uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); };

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

    // 【逐行批注】 按循环规则重复处理：for (size_t chunk = 0; chunk < data.size(); chunk += 64) {。
    for (size_t chunk = 0; chunk < data.size(); chunk += 64) {
        // 【逐行批注】 执行当前声明或表达式：std::array<uint32_t, 64> w{};。
        std::array<uint32_t, 64> w{};
        // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < 16; ++i) {。
        for (size_t i = 0; i < 16; ++i) {
            // 【逐行批注】 计算右侧结果并写入 size_t offset，更新当前运行状态。
            const size_t offset = chunk + i * 4;
            // 【逐行批注】 计算右侧结果并写入 w[i]，更新当前运行状态。
            w[i] = (static_cast<uint32_t>(data[offset]) << 24) |
                   // 【逐行批注】 执行当前声明或表达式：(static_cast<uint32_t>(data[offset + 1]) << 16) |。
                   (static_cast<uint32_t>(data[offset + 1]) << 16) |
                   // 【逐行批注】 执行当前声明或表达式：(static_cast<uint32_t>(data[offset + 2]) << 8) |。
                   (static_cast<uint32_t>(data[offset + 2]) << 8) |
                   // 【逐行批注】 执行当前声明或表达式：static_cast<uint32_t>(data[offset + 3]);。
                   static_cast<uint32_t>(data[offset + 3]);
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for (size_t i = 16; i < 64; ++i) w[i] = small1(w[i - 2]) + w[i - 7] + smal…。
        for (size_t i = 16; i < 64; ++i) w[i] = small1(w[i - 2]) + w[i - 7] + small0(w[i - 15]) + w[i - 16];
        // 【逐行批注】 计算右侧结果并写入 uint32_t a，更新当前运行状态。
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < 64; ++i) {。
        for (size_t i = 0; i < 64; ++i) {
            // 【逐行批注】 计算右侧结果并写入 uint32_t t1，更新当前运行状态。
            const uint32_t t1 = hh + big1(e) + ch(e, f, g) + k[i] + w[i];
            // 【逐行批注】 计算右侧结果并写入 uint32_t t2，更新当前运行状态。
            const uint32_t t2 = big0(a) + maj(a, b, c);
            // 【逐行批注】 计算右侧结果并写入 hh，更新当前运行状态。
            hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 h[0] +，更新当前运行状态。
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    // 【逐行批注】 结束当前代码块。
    }

    // 【逐行批注】 执行当前声明或表达式：std::array<unsigned char, 32> digest{};。
    std::array<unsigned char, 32> digest{};
    // 【逐行批注】 按循环规则重复处理：for (size_t i = 0; i < h.size(); ++i) {。
    for (size_t i = 0; i < h.size(); ++i) {
        // 【逐行批注】 计算右侧结果并写入 digest[i * 4 + 0]，更新当前运行状态。
        digest[i * 4 + 0] = static_cast<unsigned char>((h[i] >> 24) & 0xff);
        // 【逐行批注】 计算右侧结果并写入 digest[i * 4 + 1]，更新当前运行状态。
        digest[i * 4 + 1] = static_cast<unsigned char>((h[i] >> 16) & 0xff);
        // 【逐行批注】 计算右侧结果并写入 digest[i * 4 + 2]，更新当前运行状态。
        digest[i * 4 + 2] = static_cast<unsigned char>((h[i] >> 8) & 0xff);
        // 【逐行批注】 计算右侧结果并写入 digest[i * 4 + 3]，更新当前运行状态。
        digest[i * 4 + 3] = static_cast<unsigned char>(h[i] & 0xff);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return digest;。
    return digest;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 sha256Hex，执行该函数声明的职责。
std::string sha256Hex(const std::string& input) {
    // 【逐行批注】 结束当前函数并返回结果：return hexEncode(sha256(input));。
    return hexEncode(sha256(input));
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 hasString，执行该函数声明的职责。
bool hasString(const json& value) {
    // 【逐行批注】 结束当前函数并返回结果：return value.is_string() && !trim(value.get<std::string>()).empty();。
    return value.is_string() && !trim(value.get<std::string>()).empty();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 hasStringArray，执行该函数声明的职责。
bool hasStringArray(const json& value) {
    // 【逐行批注】 判断条件 (!value.is_array()) return false;，成立时执行对应分支。
    if (!value.is_array()) return false;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : value) {。
    for (const auto& item : value) {
        // 【逐行批注】 判断条件 (!item.is_string()) return false;，成立时执行对应分支。
        if (!item.is_string()) return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 validStep，执行该函数声明的职责。
bool validStep(const json& step) {
    // 【逐行批注】 结束当前函数并返回结果：return step.is_object() && trim(step.value("title", "")).size() > 0 &…。
    return step.is_object() && trim(step.value("title", "")).size() > 0 && trim(step.value("explanation", "")).size() > 0 &&
           // 【逐行批注】 调用 trim 完成本行所需操作：trim(step.value("action", "")).size() > 0 && trim(step.value("check",…。
           trim(step.value("action", "")).size() > 0 && trim(step.value("check", "")).size() > 0;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 validPhase，执行该函数声明的职责。
bool validPhase(const json& phase) {
    // 【逐行批注】 判断条件 (!phase.is_object()) return false;，成立时执行对应分支。
    if (!phase.is_object()) return false;
    // 【逐行批注】 判断条件 (trim(phase.value("name", "")).empty() || !phase.contains("durat…，成立时执行对应分支。
    if (trim(phase.value("name", "")).empty() || !phase.contains("durationWeeks") || !phase["durationWeeks"].is_number() ||
        // 【逐行批注】 调用 trim 完成本行所需操作：trim(phase.value("objective", "")).empty() || trim(phase.value("descr…。
        trim(phase.value("objective", "")).empty() || trim(phase.value("description", "")).empty() ||
        // 【逐行批注】 调用 hasStringArray 完成本行所需操作：!phase.contains("topics") || !hasStringArray(phase["topics"])) {。
        !phase.contains("topics") || !hasStringArray(phase["topics"])) {
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (phase.contains("steps") && !phase["steps"].is_null()) {，成立时执行对应分支。
    if (phase.contains("steps") && !phase["steps"].is_null()) {
        // 【逐行批注】 判断条件 (!phase["steps"].is_array()) return false;，成立时执行对应分支。
        if (!phase["steps"].is_array()) return false;
        // 【逐行批注】 按循环规则重复处理：for (const auto& step : phase["steps"]) {。
        for (const auto& step : phase["steps"]) {
            // 【逐行批注】 判断条件 (!validStep(step)) return false;，成立时执行对应分支。
            if (!validStep(step)) return false;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 validResource，执行该函数声明的职责。
bool validResource(const json& resource) {
    // 【逐行批注】 结束当前函数并返回结果：return resource.is_object() && trim(resource.value("name", "")).size(…。
    return resource.is_object() && trim(resource.value("name", "")).size() > 0 && trim(resource.value("type", "")).size() > 0 &&
           // 【逐行批注】 调用 trim 完成本行所需操作：trim(resource.value("difficulty", "")).size() > 0 && trim(resource.va…。
           trim(resource.value("difficulty", "")).size() > 0 && trim(resource.value("description", "")).size() > 0 &&
           // 【逐行批注】 调用 trim 完成本行所需操作：trim(resource.value("url", "")).size() > 0 && resource.contains("free…。
           trim(resource.value("url", "")).size() > 0 && resource.contains("free") && resource["free"].is_boolean();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 validProject，执行该函数声明的职责。
bool validProject(const json& project) {
    // 【逐行批注】 结束当前函数并返回结果：return project.is_object() && trim(project.value("name", "")).size() …。
    return project.is_object() && trim(project.value("name", "")).size() > 0 && trim(project.value("difficulty", "")).size() > 0 &&
           // 【逐行批注】 调用 trim 完成本行所需操作：project.contains("estimatedHours") && project["estimatedHours"].is_nu…。
           project.contains("estimatedHours") && project["estimatedHours"].is_number() && trim(project.value("output", "")).size() > 0 &&
           // 【逐行批注】 调用 hasStringArray 完成本行所需操作：project.contains("acceptanceCriteria") && hasStringArray(project["acc…。
           project.contains("acceptanceCriteria") && hasStringArray(project["acceptanceCriteria"]);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 调用 PlanCache 完成本行所需操作：PlanCache::PlanCache(const std::string& cacheDir)。
PlanCache::PlanCache(const std::string& cacheDir)
    // 【逐行批注】 定义函数 dir_，执行该函数声明的职责。
    : dir_(env("AI_PLAN_CACHE_DIR", cacheDir)), ttlSeconds_(env_ll("AI_PLAN_CACHE_TTL_SECONDS", 604800LL)) {}

// 【逐行批注】 定义函数 PlanCache::normalizeGoal，执行该函数声明的职责。
std::string PlanCache::normalizeGoal(const std::string& goal) {
    // 【逐行批注】 结束当前函数并返回结果：return lowerAscii(collapseWhitespace(trim(goal)));。
    return lowerAscii(collapseWhitespace(trim(goal)));
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 PlanCache::cacheKey，执行该函数声明的职责。
std::string PlanCache::cacheKey(const std::string& goal, const std::string& mode) {
    // 【逐行批注】 结束当前函数并返回结果：return sha256Hex(normalizeMode(mode) + ":" + normalizeGoal(goal));。
    return sha256Hex(normalizeMode(mode) + ":" + normalizeGoal(goal));
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 PlanCache::isValidPlan，执行该函数声明的职责。
bool PlanCache::isValidPlan(const json& plan) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (!plan.is_object()) return false;，成立时执行对应分支。
        if (!plan.is_object()) return false;
        // MockPlan 形状（适配后缓存）：title/summary 非空 + roadmap/courseStructure 非空数组
        // 【逐行批注】 判断条件 (plan.contains("roadmap") && plan.contains("courseStructure")) {，成立时执行对应分支。
        if (plan.contains("roadmap") && plan.contains("courseStructure")) {
            // 【逐行批注】 结束当前函数并返回结果：return hasString(plan.value("title", "")) && hasString(plan.value("su…。
            return hasString(plan.value("title", "")) && hasString(plan.value("summary", "")) &&
                   // 【逐行批注】 执行当前声明或表达式：plan["roadmap"].is_array() && !plan["roadmap"].empty() &&。
                   plan["roadmap"].is_array() && !plan["roadmap"].empty() &&
                   // 【逐行批注】 执行当前声明或表达式：plan["courseStructure"].is_array() && !plan["courseStructure"].empty();。
                   plan["courseStructure"].is_array() && !plan["courseStructure"].empty();
        // 【逐行批注】 结束当前代码块。
        }
        // GeneratedPlan 形状（原始 AI 输出）
        // 【逐行批注】 判断条件 (!hasString(plan.value("title", "")) || !hasString(plan.value("g…，成立时执行对应分支。
        if (!hasString(plan.value("title", "")) || !hasString(plan.value("goal", "")) ||
            // 【逐行批注】 调用 hasString 完成本行所需操作：!plan.contains("durationWeeks") || !plan["durationWeeks"].is_number()…。
            !plan.contains("durationWeeks") || !plan["durationWeeks"].is_number() || !hasString(plan.value("summary", "")) ||
            // 【逐行批注】 执行当前声明或表达式：!plan.contains("phases") || !plan["phases"].is_array() || plan["phases"].empty() ||。
            !plan.contains("phases") || !plan["phases"].is_array() || plan["phases"].empty() ||
            // 【逐行批注】 执行当前声明或表达式：!plan.contains("resources") || !plan["resources"].is_array() || !plan.contains("proj…。
            !plan.contains("resources") || !plan["resources"].is_array() || !plan.contains("projects") || !plan["projects"].is_array() ||
            // 【逐行批注】 执行当前声明或表达式：plan["projects"].empty()) {。
            plan["projects"].empty()) {
            // 【逐行批注】 结束当前函数并返回结果：return false;。
            return false;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for (const auto& phase : plan["phases"]) {。
        for (const auto& phase : plan["phases"]) {
            // 【逐行批注】 判断条件 (!validPhase(phase)) return false;，成立时执行对应分支。
            if (!validPhase(phase)) return false;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for (const auto& resource : plan["resources"]) {。
        for (const auto& resource : plan["resources"]) {
            // 【逐行批注】 判断条件 (!validResource(resource)) return false;，成立时执行对应分支。
            if (!validResource(resource)) return false;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 按循环规则重复处理：for (const auto& project : plan["projects"]) {。
        for (const auto& project : plan["projects"]) {
            // 【逐行批注】 判断条件 (!validProject(project)) return false;，成立时执行对应分支。
            if (!validProject(project)) return false;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return true;。
        return true;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 PlanCache::read，执行该函数声明的职责。
std::optional<json> PlanCache::read(const std::string& goal, const std::string& mode) const {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 std::string key，更新当前运行状态。
        const std::string key = cacheKey(goal, mode);
        // 【逐行批注】 计算右侧结果并写入 fs::path file，更新当前运行状态。
        const fs::path file = fs::path(dir_) / (key + ".json");
        // 【逐行批注】 判断条件 (!fs::exists(file)) return std::nullopt;，成立时执行对应分支。
        if (!fs::exists(file)) return std::nullopt;
        // 【逐行批注】 声明对象 in，并通过构造函数初始化其状态。
        std::ifstream in(file, std::ios::binary);
        // 【逐行批注】 判断条件 (!in) return std::nullopt;，成立时执行对应分支。
        if (!in) return std::nullopt;
        // 【逐行批注】 执行当前声明或表达式：std::ostringstream buffer;。
        std::ostringstream buffer;
        // 【逐行批注】 执行当前声明或表达式：buffer << in.rdbuf();。
        buffer << in.rdbuf();
        // 【逐行批注】 计算右侧结果并写入 json cached，更新当前运行状态。
        const json cached = json::parse(buffer.str());
        // 【逐行批注】 判断条件 (!cached.is_object() || !cached.contains("createdAt") || !cached…，成立时执行对应分支。
        if (!cached.is_object() || !cached.contains("createdAt") || !cached.contains("plan") || !cached["plan"].is_object()) {
            // 【逐行批注】 结束当前函数并返回结果：return std::nullopt;。
            return std::nullopt;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 计算右侧结果并写入 std::time_t createdAt，更新当前运行状态。
        std::time_t createdAt = 0;
        // 【逐行批注】 判断条件 (!parseIso8601Utc(cached.value("createdAt", ""), createdAt)) ret…，成立时执行对应分支。
        if (!parseIso8601Utc(cached.value("createdAt", ""), createdAt)) return std::nullopt;
        // 【逐行批注】 计算右侧结果并写入 std::time_t now，更新当前运行状态。
        const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        // 【逐行批注】 判断条件 (now < createdAt) return std::nullopt;，成立时执行对应分支。
        if (now < createdAt) return std::nullopt;
        // 【逐行批注】 判断条件 (static_cast<long long>(now - createdAt) > ttlSeconds_) return s…，成立时执行对应分支。
        if (static_cast<long long>(now - createdAt) > ttlSeconds_) return std::nullopt;
        // 【逐行批注】 判断条件 (!isValidPlan(cached["plan"])) return std::nullopt;，成立时执行对应分支。
        if (!isValidPlan(cached["plan"])) return std::nullopt;
        // 【逐行批注】 结束当前函数并返回结果：return cached["plan"];。
        return cached["plan"];
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return std::nullopt;。
        return std::nullopt;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 PlanCache::write，执行该函数声明的职责。
bool PlanCache::write(const std::string& goal, const std::string& mode, const json& plan) const {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (!isValidPlan(plan)) return false;，成立时执行对应分支。
        if (!isValidPlan(plan)) return false;
        // 【逐行批注】 调用 create_directories 完成本行所需操作：fs::create_directories(dir_);。
        fs::create_directories(dir_);
        // 【逐行批注】 计算右侧结果并写入 std::string key，更新当前运行状态。
        const std::string key = cacheKey(goal, mode);
        // 【逐行批注】 计算右侧结果并写入 fs::path file，更新当前运行状态。
        const fs::path file = fs::path(dir_) / (key + ".json");
        // 【逐行批注】 计算右侧结果并写入 json cached，更新当前运行状态。
        const json cached = {
            // 【逐行批注】 调用 trimGoalText 完成本行所需操作：{"goal", trimGoalText(goal)},。
            {"goal", trimGoalText(goal)},
            // 【逐行批注】 调用 normalizeMode 完成本行所需操作：{"mode", normalizeMode(mode)},。
            {"mode", normalizeMode(mode)},
            // 【逐行批注】 调用 nowIso8601 完成本行所需操作：{"createdAt", nowIso8601()},。
            {"createdAt", nowIso8601()},
            // 【逐行批注】 执行当前声明或表达式：{"plan", plan}};。
            {"plan", plan}};
        // 【逐行批注】 计算右侧结果并写入 fs::path temp，更新当前运行状态。
        const fs::path temp = file.string() + ".tmp";
        // 【逐行批注】 开始当前代码块。
        {
            // 【逐行批注】 声明对象 out，并通过构造函数初始化其状态。
            std::ofstream out(temp, std::ios::binary | std::ios::trunc);
            // 【逐行批注】 判断条件 (!out) return false;，成立时执行对应分支。
            if (!out) return false;
            // 【逐行批注】 执行当前声明或表达式：out << cached.dump(2);。
            out << cached.dump(2);
            // 【逐行批注】 判断条件 (!out.good()) return false;，成立时执行对应分支。
            if (!out.good()) return false;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：std::error_code ec;。
        std::error_code ec;
        // 【逐行批注】 调用 rename 完成本行所需操作：fs::rename(temp, file, ec);。
        fs::rename(temp, file, ec);
        // 【逐行批注】 判断条件 (ec) {，成立时执行对应分支。
        if (ec) {
            // 【逐行批注】 调用 remove 完成本行所需操作：fs::remove(file, ec);。
            fs::remove(file, ec);
            // 【逐行批注】 执行当前声明或表达式：ec.clear();。
            ec.clear();
            // 【逐行批注】 调用 rename 完成本行所需操作：fs::rename(temp, file, ec);。
            fs::rename(temp, file, ec);
            // 【逐行批注】 判断条件 (ec) {，成立时执行对应分支。
            if (ec) {
                // 【逐行批注】 调用 remove 完成本行所需操作：fs::remove(temp, ec);。
                fs::remove(temp, ec);
                // 【逐行批注】 结束当前函数并返回结果：return false;。
                return false;
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return true;。
        return true;
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi

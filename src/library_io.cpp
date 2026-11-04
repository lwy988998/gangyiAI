// 【逐行批注】 引入头文件 "library_io.hpp"，为后续实现提供类型或接口声明。
#include "library_io.hpp"
// 【逐行批注】 引入头文件 "search_client.hpp"，为后续实现提供类型或接口声明。
#include "search_client.hpp"
// 【逐行批注】 引入头文件 <curl/curl.h>，为后续实现提供类型或接口声明。
#include <curl/curl.h>
// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <cmath>，为后续实现提供类型或接口声明。
#include <cmath>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <memory>，为后续实现提供类型或接口声明。
#include <memory>
// 【逐行批注】 引入头文件 <random>，为后续实现提供类型或接口声明。
#include <random>
// 【逐行批注】 引入头文件 <regex>，为后续实现提供类型或接口声明。
#include <regex>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
// 【逐行批注】 引入头文件 <winsock2.h>，为后续实现提供类型或接口声明。
#include <winsock2.h>
// 【逐行批注】 引入头文件 <windows.h>，为后续实现提供类型或接口声明。
#include <windows.h>
// 【逐行批注】 处理条件编译指令：#else。
#else
// 【逐行批注】 引入头文件 <netinet/in.h>，为后续实现提供类型或接口声明。
#include <netinet/in.h>
// 【逐行批注】 引入头文件 <arpa/inet.h>，为后续实现提供类型或接口声明。
#include <arpa/inet.h>
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 计算右侧结果并写入 using CurlURL，更新当前运行状态。
using CurlURL = std::unique_ptr<CURLU, decltype(&curl_url_cleanup)>;
// 【逐行批注】 定义函数 part，执行该函数声明的职责。
std::string part(CURLU* url, CURLUPart name) {
    // 【逐行批注】 计算右侧结果并写入 char* raw，更新当前运行状态。
    char* raw = nullptr;
    // 【逐行批注】 判断条件 (curl_url_get(url, name, &raw, 0) != CURLUE_OK) return {};，成立时执行对应分支。
    if (curl_url_get(url, name, &raw, 0) != CURLUE_OK) return {};
    // 【逐行批注】 声明对象 result，并通过构造函数初始化其状态。
    std::string result(raw); curl_free(raw); return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 publicAddress，执行该函数声明的职责。
bool publicAddress(const sockaddr* address) {
    // 【逐行批注】 判断条件 (address->sa_family == AF_INET) {，成立时执行对应分支。
    if (address->sa_family == AF_INET) {
        // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
        const auto value = ntohl(reinterpret_cast<const sockaddr_in*>(address)->sin_addr.s_addr);
        // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
        const auto first = value >> 24, second = (value >> 16) & 255;
        // 【逐行批注】 结束当前函数并返回结果：return first != 0 && first != 10 && first != 127 && first < 224 &&。
        return first != 0 && first != 10 && first != 127 && first < 224 &&
            // 【逐行批注】 执行当前声明或表达式：!(first == 169 && second == 254) && !(first == 172 && second >= 16 && second <= 31) …。
            !(first == 169 && second == 254) && !(first == 172 && second >= 16 && second <= 31) &&
            // 【逐行批注】 执行当前声明或表达式：!(first == 192 && second == 168) && !(first == 100 && second >= 64 && second <= 127);。
            !(first == 192 && second == 168) && !(first == 100 && second >= 64 && second <= 127);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (address->sa_family == AF_INET6) {，成立时执行对应分支。
    if (address->sa_family == AF_INET6) {
        // 【逐行批注】 计算右侧结果并写入 auto* bytes，更新当前运行状态。
        const auto* bytes = reinterpret_cast<const unsigned char*>(&reinterpret_cast<const sockaddr_in6*>(address)->sin6_addr);
        // 【逐行批注】 判断条件 ((bytes[0] & 0xfe) == 0xfc || bytes[0] == 0xff || (bytes[0] == 0…，成立时执行对应分支。
        if ((bytes[0] & 0xfe) == 0xfc || bytes[0] == 0xff || (bytes[0] == 0xfe && (bytes[1] & 0xc0) == 0x80)) return false;
        // 【逐行批注】 计算右侧结果并写入 bool zero，更新当前运行状态。
        bool zero = true; for (int i = 0; i < 15; ++i) zero = zero && bytes[i] == 0;
        // 【逐行批注】 判断条件 (zero) return false;，成立时执行对应分支。
        if (zero) return false;
        // 【逐行批注】 计算右侧结果并写入 bool mapped，更新当前运行状态。
        bool mapped = bytes[10] == 255 && bytes[11] == 255;
        // 【逐行批注】 按循环规则重复处理：for (int i = 0; i < 10; ++i) mapped = mapped && bytes[i] == 0;。
        for (int i = 0; i < 10; ++i) mapped = mapped && bytes[i] == 0;
        // 【逐行批注】 判断条件 (mapped) {，成立时执行对应分支。
        if (mapped) {
            // 【逐行批注】 计算右侧结果并写入 sockaddr_in ipv4{}; ipv4.sin_family，更新当前运行状态。
            sockaddr_in ipv4{}; ipv4.sin_family = AF_INET;
            // 【逐行批注】 调用 copy 完成本行所需操作：std::copy(bytes + 12, bytes + 16, reinterpret_cast<unsigned char*>(&i…。
            std::copy(bytes + 12, bytes + 16, reinterpret_cast<unsigned char*>(&ipv4.sin_addr));
            // 【逐行批注】 结束当前函数并返回结果：return publicAddress(reinterpret_cast<const sockaddr*>(&ipv4));。
            return publicAddress(reinterpret_cast<const sockaddr*>(&ipv4));
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 结束当前函数并返回结果：return true;。
        return true;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return false;。
    return false;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 openPublicSocket，执行该函数声明的职责。
curl_socket_t openPublicSocket(void*, curlsocktype purpose, curl_sockaddr* address) {
    // 【逐行批注】 判断条件 (purpose != CURLSOCKTYPE_IPCXN || !publicAddress(&address->addr)…，成立时执行对应分支。
    if (purpose != CURLSOCKTYPE_IPCXN || !publicAddress(&address->addr)) return CURL_SOCKET_BAD;
    // 【逐行批注】 结束当前函数并返回结果：return socket(address->family, address->socktype, address->protocol);。
    return socket(address->family, address->socktype, address->protocol);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 声明数据结构或类型 Transfer 及其成员接口。
struct Transfer {
    // 【逐行批注】 执行当前声明或表达式：std::string body;。
    std::string body;
    // 【逐行批注】 计算右侧结果并写入 LibraryCancel* cancelled，更新当前运行状态。
    const LibraryCancel* cancelled = nullptr;
    // 【逐行批注】 计算右侧结果并写入 bool tooLarge，更新当前运行状态。
    bool tooLarge = false;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 定义函数 receive，执行该函数声明的职责。
size_t receive(char* data, size_t size, size_t count, void* raw) {
    // 【逐行批注】 计算右侧结果并写入 auto& transfer，更新当前运行状态。
    auto& transfer = *static_cast<Transfer*>(raw);
    // 【逐行批注】 计算右侧结果并写入 auto length，更新当前运行状态。
    const auto length = size * count;
    // 【逐行批注】 判断条件 (length > 24 * 1024 * 1024 || transfer.body.size() + length > 24…，成立时执行对应分支。
    if (length > 24 * 1024 * 1024 || transfer.body.size() + length > 24 * 1024 * 1024) {
        // 【逐行批注】 计算右侧结果并写入 transfer.tooLarge，更新当前运行状态。
        transfer.tooLarge = true; return 0;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：transfer.body.append(data, length); return length;。
    transfer.body.append(data, length); return length;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 progress，执行该函数声明的职责。
int progress(void* raw, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    // 【逐行批注】 计算右侧结果并写入 auto& state，更新当前运行状态。
    const auto& state = *static_cast<Transfer*>(raw);
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { return state.cancelled && *state.cancelled && (*state.cancelled)() ? 1 : 0; } catch (...) { return 1; }
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 captionTime，执行该函数声明的职责。
double captionTime(const std::string& value) {
    // 【逐行批注】 判断条件 (value.size() > 32 || !std::regex_match(value, std::regex(R"((\d…，成立时执行对应分支。
    if (value.size() > 32 || !std::regex_match(value, std::regex(R"((\d{2,}:)?[0-5]\d:[0-5]\d[.,]\d{3})"))) return -1;
    // 【逐行批注】 计算右侧结果并写入 raw，更新当前运行状态。
    auto raw = value; std::replace(raw.begin(), raw.end(), ',', '.');
    // 【逐行批注】 调用 input 完成本行所需操作：std::vector<double> parts; std::istringstream input(raw); std::string…。
    std::vector<double> parts; std::istringstream input(raw); std::string part;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { while (std::getline(input, part, ':')) parts.push_back(std::stod(part)); }
    // 【逐行批注】 捕获并处理异常：catch (const std::exception&) { return -1; }。
    catch (const std::exception&) { return -1; }
    // 【逐行批注】 计算右侧结果并写入 double result，更新当前运行状态。
    double result = 0; for (const auto number : parts) result = result * 60 + number; return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 libraryNow，执行该函数声明的职责。
std::string libraryNow() {
    // 【逐行批注】 计算右侧结果并写入 auto time，更新当前运行状态。
    const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); std::tm utc{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&utc, &time);。
    gmtime_s(&utc, &time);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&time, &utc);。
    gmtime_r(&time, &utc);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 调用 put_time 完成本行所需操作：std::ostringstream result; result << std::put_time(&utc, "%Y-%m-%dT%H…。
    std::ostringstream result; result << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ"); return result.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 libraryDigest，执行该函数声明的职责。
std::string libraryDigest(const std::string& text) {
    // 【逐行批注】 结束当前函数并返回结果：return resourceDigest(text);。
    return resourceDigest(text);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 libraryPublicURL，执行该函数声明的职责。
bool libraryPublicURL(const std::string& value) {
    // 【逐行批注】 判断条件 (value.size() > 8000 || value.find_first_of("\r\n") != std::stri…，成立时执行对应分支。
    if (value.size() > 8000 || value.find_first_of("\r\n") != std::string::npos) return false;
    // 【逐行批注】 声明对象 url，并通过构造函数初始化其状态。
    CurlURL url(curl_url(), curl_url_cleanup);
    // 【逐行批注】 判断条件 (!url || curl_url_set(url.get(), CURLUPART_URL, value.c_str(), 0…，成立时执行对应分支。
    if (!url || curl_url_set(url.get(), CURLUPART_URL, value.c_str(), 0) != CURLUE_OK) return false;
    // 【逐行批注】 计算右侧结果并写入 host，更新当前运行状态。
    auto host = part(url.get(), CURLUPART_HOST);
    // 【逐行批注】 调用 transform、tolower 完成本行所需操作：std::transform(host.begin(), host.end(), host.begin(), [](unsigned ch…。
    std::transform(host.begin(), host.end(), host.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    // 【逐行批注】 计算右侧结果并写入 auto scheme，更新当前运行状态。
    const auto scheme = part(url.get(), CURLUPART_SCHEME);
    // 【逐行批注】 判断条件 ((scheme != "http" && scheme != "https") || !part(url.get(), CUR…，成立时执行对应分支。
    if ((scheme != "http" && scheme != "https") || !part(url.get(), CURLUPART_USER).empty() ||
        // 【逐行批注】 调用 part 完成本行所需操作：!part(url.get(), CURLUPART_PASSWORD).empty() || host.empty()) return …。
        !part(url.get(), CURLUPART_PASSWORD).empty() || host.empty()) return false;
    // 【逐行批注】 判断条件 (host.back() == '.') host.pop_back();，成立时执行对应分支。
    if (host.back() == '.') host.pop_back();
    // 【逐行批注】 计算右侧结果并写入 numeric，更新当前运行状态。
    auto numeric = host;
    // 【逐行批注】 判断条件 (numeric.front() == '[' && numeric.back() == ']') numeric = nume…，成立时执行对应分支。
    if (numeric.front() == '[' && numeric.back() == ']') numeric = numeric.substr(1, numeric.size() - 2);
    // 【逐行批注】 计算右侧结果并写入 sockaddr_in ipv4{}; ipv4.sin_family，更新当前运行状态。
    sockaddr_in ipv4{}; ipv4.sin_family = AF_INET;
    // 【逐行批注】 计算右侧结果并写入 sockaddr_in6 ipv6{}; ipv6.sin6_family，更新当前运行状态。
    sockaddr_in6 ipv6{}; ipv6.sin6_family = AF_INET6;
    // 【逐行批注】 判断条件 (inet_pton(AF_INET, numeric.c_str(), &ipv4.sin_addr) == 1) retur…，成立时执行对应分支。
    if (inet_pton(AF_INET, numeric.c_str(), &ipv4.sin_addr) == 1) return publicAddress(reinterpret_cast<const sockaddr*>(&ipv4));
    // 【逐行批注】 判断条件 (inet_pton(AF_INET6, numeric.c_str(), &ipv6.sin6_addr) == 1) ret…，成立时执行对应分支。
    if (inet_pton(AF_INET6, numeric.c_str(), &ipv6.sin6_addr) == 1) return publicAddress(reinterpret_cast<const sockaddr*>(&ipv6));
    // 【逐行批注】 判断条件 (host == "localhost" || host.find(".localhost") != std::string::…，成立时执行对应分支。
    if (host == "localhost" || host.find(".localhost") != std::string::npos || host.find(".local") != std::string::npos ||
        // 【逐行批注】 执行当前声明或表达式：host == "[::1]" || host == "0.0.0.0" || host.rfind("127.", 0) == 0 || host.rfind("10…。
        host == "[::1]" || host == "0.0.0.0" || host.rfind("127.", 0) == 0 || host.rfind("10.", 0) == 0 ||
        // 【逐行批注】 执行当前声明或表达式：host.rfind("192.168.", 0) == 0 || host.rfind("169.254.", 0) == 0) return false;。
        host.rfind("192.168.", 0) == 0 || host.rfind("169.254.", 0) == 0) return false;
    // 最终连接的实际IP仍由套接字回调检查，阻止重定向及DNS解析进入本机网络。
    // 【逐行批注】 结束当前函数并返回结果：return true;。
    return true;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 libraryCanonicalURL，执行该函数声明的职责。
std::string libraryCanonicalURL(const std::string& value) {
    // 【逐行批注】 声明对象 url，并通过构造函数初始化其状态。
    CurlURL url(curl_url(), curl_url_cleanup);
    // 【逐行批注】 判断条件 (!url || curl_url_set(url.get(), CURLUPART_URL, value.c_str(), 0…，成立时执行对应分支。
    if (!url || curl_url_set(url.get(), CURLUPART_URL, value.c_str(), 0) != CURLUE_OK) return {};
    // 【逐行批注】 调用 curl_url_set 完成本行所需操作：curl_url_set(url.get(), CURLUPART_FRAGMENT, nullptr, 0);。
    curl_url_set(url.get(), CURLUPART_FRAGMENT, nullptr, 0);
    // 【逐行批注】 计算右侧结果并写入 host，更新当前运行状态。
    auto host = part(url.get(), CURLUPART_HOST);
    // 【逐行批注】 调用 transform、tolower 完成本行所需操作：std::transform(host.begin(), host.end(), host.begin(), [](unsigned ch…。
    std::transform(host.begin(), host.end(), host.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    // 【逐行批注】 调用 curl_url_set 完成本行所需操作：curl_url_set(url.get(), CURLUPART_HOST, host.c_str(), 0);。
    curl_url_set(url.get(), CURLUPART_HOST, host.c_str(), 0);
    // 【逐行批注】 结束当前函数并返回结果：return part(url.get(), CURLUPART_URL);。
    return part(url.get(), CURLUPART_URL);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 libraryFetch，执行该函数声明的职责。
LibraryDocument libraryFetch(const std::string& input, const LibraryCancel& cancelled) {
    // 【逐行批注】 计算右侧结果并写入 LibraryDocument document; document.url，更新当前运行状态。
    LibraryDocument document; document.url = input;
    // 【逐行批注】 按循环规则重复处理：for (int redirect = 0; redirect <= 5; ++redirect) {。
    for (int redirect = 0; redirect <= 5; ++redirect) {
        // 【逐行批注】 判断条件 (cancelled && cancelled()) { document.error = "cancelled"; retur…，成立时执行对应分支。
        if (cancelled && cancelled()) { document.error = "cancelled"; return document; }
        // 【逐行批注】 判断条件 (!libraryPublicURL(document.url)) { document.error = "restricted…，成立时执行对应分支。
        if (!libraryPublicURL(document.url)) { document.error = "restricted_url"; return document; }
        // 【逐行批注】 调用 decltype、curl、curl_easy_init 完成本行所需操作：std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_in…。
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
        // 【逐行批注】 判断条件 (!curl) { document.error = "network_unavailable"; return documen…，成立时执行对应分支。
        if (!curl) { document.error = "network_unavailable"; return document; }
        // 【逐行批注】 计算右侧结果并写入 Transfer transfer; transfer.cancelled，更新当前运行状态。
        Transfer transfer; transfer.cancelled = &cancelled;
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_URL, document.url.c_str());。
        curl_easy_setopt(curl.get(), CURLOPT_URL, document.url.c_str());
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_PROTOCOLS_STR, "http,https");。
        curl_easy_setopt(curl.get(), CURLOPT_PROTOCOLS_STR, "http,https");
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_PROXY, "");。
        curl_easy_setopt(curl.get(), CURLOPT_PROXY, "");
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_OPENSOCKETFUNCTION, openPublicSo…。
        curl_easy_setopt(curl.get(), CURLOPT_OPENSOCKETFUNCTION, openPublicSocket);
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, receive);。
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, receive);
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &transfer);。
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &transfer);
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);。
        curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, progress);。
        curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, progress);
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, &transfer);。
        curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, &transfer);
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "GangyiAI/5.6.4 learn…。
        curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "GangyiAI/5.6.4 learning-source-reader");
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT_MS, 8000L);。
        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT_MS, 8000L);
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, 25000L);。
        curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, 25000L);
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_ACCEPT_ENCODING, "");。
        curl_easy_setopt(curl.get(), CURLOPT_ACCEPT_ENCODING, "");
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);。
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);。
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);
        // 【逐行批注】 调用 curl_easy_setopt 完成本行所需操作：curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);。
        curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
        // 【逐行批注】 执行当前声明或表达式：++document.attempts;。
        ++document.attempts;
        // 【逐行批注】 计算右侧结果并写入 auto code，更新当前运行状态。
        const auto code = curl_easy_perform(curl.get());
        // 【逐行批注】 调用 curl_easy_getinfo 完成本行所需操作：curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &document.statu…。
        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &document.status);
        // 【逐行批注】 计算右侧结果并写入 char* location，更新当前运行状态。
        char* location = nullptr; curl_easy_getinfo(curl.get(), CURLINFO_REDIRECT_URL, &location);
        // 【逐行批注】 判断条件 (document.status >= 300 && document.status < 400 && location && …，成立时执行对应分支。
        if (document.status >= 300 && document.status < 400 && location && code == CURLE_OK) { document.url = location; continue; }
        // 【逐行批注】 计算右侧结果并写入 char* mime，更新当前运行状态。
        char* mime = nullptr; curl_easy_getinfo(curl.get(), CURLINFO_CONTENT_TYPE, &mime);
        // 【逐行批注】 计算右侧结果并写入 document.mime，更新当前运行状态。
        document.mime = mime ? mime : "";
        // 【逐行批注】 计算右侧结果并写入 document.body，更新当前运行状态。
        document.body = std::move(transfer.body);
        // 【逐行批注】 判断条件 (transfer.tooLarge) document.error = "document_too_large";，成立时执行对应分支。
        if (transfer.tooLarge) document.error = "document_too_large";
        // 【逐行批注】 在前一条件不成立时继续判断：(code == CURLE_ABORTED_BY_CALLBACK) document.error = "cancelled";。
        else if (code == CURLE_ABORTED_BY_CALLBACK) document.error = "cancelled";
        // 【逐行批注】 在前一条件不成立时继续判断：(code != CURLE_OK) document.error = "network_error";。
        else if (code != CURLE_OK) document.error = "network_error";
        // 【逐行批注】 在前一条件不成立时继续判断：(document.status == 401 || document.status == 403) document.erro…。
        else if (document.status == 401 || document.status == 403) document.error = "login_or_access_required";
        // 【逐行批注】 在前一条件不成立时继续判断：(document.status < 200 || document.status >= 300) document.error…。
        else if (document.status < 200 || document.status >= 300) document.error = "http_" + std::to_string(document.status);
        // 【逐行批注】 结束当前函数并返回结果：return document;。
        return document;
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 document.error，更新当前运行状态。
    document.error = "too_many_redirects"; return document;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 libraryParseCaptions，执行该函数声明的职责。
Json libraryParseCaptions(const std::string& text, const std::string& mime) {
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = {{"segments", Json::array()}, {"kind", "captions"}, {"status", "unreadable"}, {"automatic", nullptr}};
    // 【逐行批注】 计算右侧结果并写入 auto parsed，更新当前运行状态。
    const auto parsed = Json::parse(text, nullptr, false);
    // 【逐行批注】 判断条件 (parsed.is_object() && parsed.value("body", Json()).is_array()) {，成立时执行对应分支。
    if (parsed.is_object() && parsed.value("body", Json()).is_array()) {
        // 【逐行批注】 计算右侧结果并写入 if(parsed.value("automatic",Json()).is_boole…，更新当前运行状态。
        if(parsed.value("automatic",Json()).is_boolean())result["automatic"]=parsed["automatic"];
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : parsed["body"]) if (item.is_object() && item.value…。
        for (const auto& item : parsed["body"]) if (item.is_object() && item.value("content", Json()).is_string() && item.value("from", Json()).is_number()) {
            // 【逐行批注】 计算右侧结果并写入 auto end，更新当前运行状态。
            const auto end = item.value("to", item["from"]);
            // 【逐行批注】 计算右侧结果并写入 double start，更新当前运行状态。
            const double start = item["from"].get<double>();
            // 【逐行批注】 判断条件 (end.is_number() && std::isfinite(start) && std::isfinite(end.ge…，成立时执行对应分支。
            if (end.is_number() && std::isfinite(start) && std::isfinite(end.get<double>()) && start >= 0 && end.get<double>() >= start)
                // 【逐行批注】 执行当前声明或表达式：result["segments"].push_back({{"text", item["content"]}, {"start", start}, {"end", e…。
                result["segments"].push_back({{"text", item["content"]}, {"start", start}, {"end", end}});
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：} else if (text.rfind("WEBVTT", 0) == 0 || mime.find("vtt") != std::string::npos || …。
    } else if (text.rfind("WEBVTT", 0) == 0 || mime.find("vtt") != std::string::npos || mime.find("subrip") != std::string::npos ||
        // 【逐行批注】 调用 regex_search、regex 完成本行所需操作：std::regex_search(text.substr(0, 1000), std::regex(R"(\d\d:\d\d[.,]\d…。
        std::regex_search(text.substr(0, 1000), std::regex(R"(\d\d:\d\d[.,]\d\d\d\s+-->)"))) {
        // 【逐行批注】 声明对象 lines，并通过构造函数初始化其状态。
        std::istringstream lines(text); std::string line, content; double start = -1, end = -1;
        // 【逐行批注】 调用 timing 完成本行所需操作：const std::regex timing(R"(([\d:.]+[.,]\d{3})\s+-->\s+([\d:.]+[.,]\d{…。
        const std::regex timing(R"(([\d:.]+[.,]\d{3})\s+-->\s+([\d:.]+[.,]\d{3}))");
        // 【逐行批注】 计算右侧结果并写入 auto append，更新当前运行状态。
        const auto append = [&] { if (start >= 0 && end >= start && !content.empty()) result["segments"].push_back({{"text", std::regex_replace(content, std::regex("<[^>]+>"), "")}, {"start", start}, {"end", end}}); content.clear(); };
        // 【逐行批注】 在条件成立期间循环处理：while (std::getline(lines, line)) {。
        while (std::getline(lines, line)) {
            // 【逐行批注】 判断条件 (!line.empty() && line.back() == '\r') line.pop_back();，成立时执行对应分支。
            if (!line.empty() && line.back() == '\r') line.pop_back();
            // 【逐行批注】 执行当前声明或表达式：std::smatch match;。
            std::smatch match;
            // 【逐行批注】 判断条件 (std::regex_search(line, match, timing)) { append(); start = cap…，成立时执行对应分支。
            if (std::regex_search(line, match, timing)) { append(); start = captionTime(match[1]); end = captionTime(match[2]); }
            // 【逐行批注】 在前一条件不成立时继续判断：(line.empty()) { append(); start = -1; }。
            else if (line.empty()) { append(); start = -1; }
            // 【逐行批注】 在前一条件不成立时继续判断：(start >= 0) content += (content.empty() ? "" : "\n") + line;。
            else if (start >= 0) content += (content.empty() ? "" : "\n") + line;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 append 完成本行所需操作：append();。
        append();
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 int index，更新当前运行状态。
    int index = 0; for (auto& segment : result["segments"]) segment["id"] = "t" + std::to_string(++index);
    // 【逐行批注】 判断条件 (index) result["status"] = "read";，成立时执行对应分支。
    if (index) result["status"] = "read";
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 libraryParseDocument，执行该函数声明的职责。
Json libraryParseDocument(const LibraryDocument& document, const LibraryCancel& cancelled) {
    // 【逐行批注】 判断条件 (!document.error.empty()) return {{"status", "unreadable"}, {"re…，成立时执行对应分支。
    if (!document.error.empty()) return {{"status", "unreadable"}, {"reason", document.error}, {"segments", Json::array()}};
    // 【逐行批注】 计算右侧结果并写入 captions，更新当前运行状态。
    auto captions = libraryParseCaptions(document.body, document.mime);
    // 【逐行批注】 判断条件 (captions["status"] == "read") return captions;，成立时执行对应分支。
    if (captions["status"] == "read") return captions;
    // 【逐行批注】 判断条件 (document.mime.rfind("text/plain", 0) == 0) {，成立时执行对应分支。
    if (document.mime.rfind("text/plain", 0) == 0) {
        // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
        Json result = {{"status", "unreadable"}, {"kind", "transcript"}, {"segments", Json::array()},
            // 【逐行批注】 执行当前声明或表达式：{"coverage", "已读取公开文字稿；未提供时间戳时仅定位文字段落"}};。
            {"coverage", "已读取公开文字稿；未提供时间戳时仅定位文字段落"}};
        // 【逐行批注】 声明对象 lines，并通过构造函数初始化其状态。
        std::istringstream lines(document.body); std::string line, paragraph;
        // 【逐行批注】 计算右侧结果并写入 auto append，更新当前运行状态。
        const auto append = [&] {
            // 【逐行批注】 判断条件 (paragraph.empty()) return;，成立时执行对应分支。
            if (paragraph.empty()) return;
            // 【逐行批注】 调用 to_string 完成本行所需操作：result["segments"].push_back({{"id", "s" + std::to_string(result["seg…。
            result["segments"].push_back({{"id", "s" + std::to_string(result["segments"].size() + 1)}, {"text", paragraph}});
            // 【逐行批注】 执行当前声明或表达式：paragraph.clear();。
            paragraph.clear();
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 在条件成立期间循环处理：while (std::getline(lines, line)) {。
        while (std::getline(lines, line)) {
            // 【逐行批注】 判断条件 (line.empty() || paragraph.size() > 2600) append();，成立时执行对应分支。
            if (line.empty() || paragraph.size() > 2600) append();
            // 【逐行批注】 判断条件 (!line.empty()) paragraph += (paragraph.empty() ? "" : "\n") + l…，成立时执行对应分支。
            if (!line.empty()) paragraph += (paragraph.empty() ? "" : "\n") + line;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 append 完成本行所需操作：append(); if (!result["segments"].empty()) result["status"] = "read";。
        append(); if (!result["segments"].empty()) result["status"] = "read";
        // 【逐行批注】 结束当前函数并返回结果：return result;。
        return result;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 GetModuleFileNameW 完成本行所需操作：wchar_t executable[MAX_PATH]{}; GetModuleFileNameW(nullptr, executabl…。
    wchar_t executable[MAX_PATH]{}; GetModuleFileNameW(nullptr, executable, MAX_PATH);
    // 【逐行批注】 计算右侧结果并写入 base，更新当前运行状态。
    auto base = std::filesystem::path(executable).parent_path();
    // 【逐行批注】 计算右侧结果并写入 helper，更新当前运行状态。
    auto helper = base / L"gangyiAI-source-reader.exe";
    // 【逐行批注】 判断条件 (const auto* configured = std::getenv("LIBRARY_READER_PATH")) he…，成立时执行对应分支。
    if (const auto* configured = std::getenv("LIBRARY_READER_PATH")) helper = std::filesystem::u8path(configured);
    // 【逐行批注】 判断条件 (!std::filesystem::exists(helper)) return {{"status", "unreadabl…，成立时执行对应分支。
    if (!std::filesystem::exists(helper)) return {{"status", "unreadable"}, {"reason", "reader_unavailable"}, {"segments", Json::array()}};
    // 【逐行批注】 调用 random 完成本行所需操作：static thread_local std::mt19937_64 random(std::random_device{}());。
    static thread_local std::mt19937_64 random(std::random_device{}());
    // 【逐行批注】 计算右侧结果并写入 auto scratch，更新当前运行状态。
    const auto scratch = std::filesystem::temp_directory_path() / ("gangyi-source-" + std::to_string(random()));
    // 【逐行批注】 调用 create_directories 完成本行所需操作：std::filesystem::create_directories(scratch);。
    std::filesystem::create_directories(scratch);
    // 【逐行批注】 声明数据结构或类型 Cleanup 及其成员接口。
    struct Cleanup { std::filesystem::path path; ~Cleanup() { std::error_code error; std::filesystem::remove_all(path, error); } } cleanup{scratch};
    // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
    const auto input = scratch / "input.json", output = scratch / "output.json";
    // 【逐行批注】 调用 file 完成本行所需操作：{ std::ofstream file(scratch / "document.bin", std::ios::binary); fil…。
    { std::ofstream file(scratch / "document.bin", std::ios::binary); file.write(document.body.data(), static_cast<std::streamsize>(document.body.size())); }
    // 【逐行批注】 调用 file 完成本行所需操作：{ std::ofstream file(input, std::ios::binary); file << Json{{"url", d…。
    { std::ofstream file(input, std::ios::binary); file << Json{{"url", document.url}, {"mime", document.mime}}.dump(); }
    // 【逐行批注】 计算右侧结果并写入 command，更新当前运行状态。
    auto command = L"\"" + helper.wstring() + L"\" \"" + input.wstring() + L"\" \"" + output.wstring() + L"\"";
    // 【逐行批注】 计算右侧结果并写入 STARTUPINFOW startup{}; startup.cb，更新当前运行状态。
    STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESHOWWINDOW; startup.wShowWindow = SW_HIDE;
    // 【逐行批注】 执行当前声明或表达式：PROCESS_INFORMATION process{};。
    PROCESS_INFORMATION process{};
    // 【逐行批注】 判断条件 (!CreateProcessW(helper.c_str(), command.data(), nullptr, nullpt…，成立时执行对应分支。
    if (!CreateProcessW(helper.c_str(), command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr, base.c_str(), &startup, &process))
        // 【逐行批注】 结束当前函数并返回结果：return {{"status", "unreadable"}, {"reason", "reader_start_failed"}, …。
        return {{"status", "unreadable"}, {"reason", "reader_start_failed"}, {"segments", Json::array()}};
    // 【逐行批注】 计算右侧结果并写入 auto job，更新当前运行状态。
    const auto job = CreateJobObjectW(nullptr, nullptr);
    // 【逐行批注】 计算右侧结果并写入 bool confined，更新当前运行状态。
    bool confined = false;
    // 【逐行批注】 判断条件 (job) {，成立时执行对应分支。
    if (job) {
        // 【逐行批注】 执行当前声明或表达式：JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};。
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        // 【逐行批注】 计算右侧结果并写入 limits.BasicLimitInformation.LimitFlags，更新当前运行状态。
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_JOB_MEMORY;
        // 【逐行批注】 计算右侧结果并写入 limits.JobMemoryLimit，更新当前运行状态。
        limits.JobMemoryLimit = 768ULL * 1024 * 1024;
        // 【逐行批注】 计算右侧结果并写入 confined，更新当前运行状态。
        confined = SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits)) && AssignProcessToJobObject(job, process.hProcess);
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!confined || ResumeThread(process.hThread) == static_cast<DWORD…，成立时执行对应分支。
    if (!confined || ResumeThread(process.hThread) == static_cast<DWORD>(-1)) {
        // 【逐行批注】 调用 TerminateProcess、CloseHandle 完成本行所需操作：TerminateProcess(process.hProcess, 1); CloseHandle(process.hThread); …。
        TerminateProcess(process.hProcess, 1); CloseHandle(process.hThread); CloseHandle(process.hProcess); if (job) CloseHandle(job);
        // 【逐行批注】 结束当前函数并返回结果：return {{"status", "unreadable"}, {"reason", "reader_isolation_failed…。
        return {{"status", "unreadable"}, {"reason", "reader_isolation_failed"}, {"segments", Json::array()}};
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 CloseHandle 完成本行所需操作：CloseHandle(process.hThread);。
    CloseHandle(process.hThread);
    // 【逐行批注】 计算右侧结果并写入 auto began，更新当前运行状态。
    const auto began = std::chrono::steady_clock::now(); bool interrupted = false;
    // 【逐行批注】 在条件成立期间循环处理：while (WaitForSingleObject(process.hProcess, 100) == WAIT_TIMEOUT) {。
    while (WaitForSingleObject(process.hProcess, 100) == WAIT_TIMEOUT) {
        // 【逐行批注】 判断条件 ((cancelled && cancelled()) || std::chrono::steady_clock::now() …，成立时执行对应分支。
        if ((cancelled && cancelled()) || std::chrono::steady_clock::now() - began > std::chrono::seconds(60)) {
            // 【逐行批注】 计算右侧结果并写入 interrupted，更新当前运行状态。
            interrupted = true; TerminateProcess(process.hProcess, 1); WaitForSingleObject(process.hProcess, 3000); break;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 调用 CloseHandle 完成本行所需操作：CloseHandle(process.hProcess); if (job) CloseHandle(job);。
    CloseHandle(process.hProcess); if (job) CloseHandle(job);
    // 【逐行批注】 判断条件 (interrupted) return {{"status", "unreadable"}, {"reason", "read…，成立时执行对应分支。
    if (interrupted) return {{"status", "unreadable"}, {"reason", "reader_interrupted"}, {"segments", Json::array()}};
    // 【逐行批注】 声明对象 file，并通过构造函数初始化其状态。
    std::ifstream file(output, std::ios::binary); Json result;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try { file >> result; } catch (...) { return {{"status", "unreadable"}, {"reason", "reader_failed"}, {"segments", Json::array()}}; }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 执行当前声明或表达式：(void)cancelled;。
    (void)cancelled;
    // 【逐行批注】 结束当前函数并返回结果：return {{"status", "unreadable"}, {"reason", "windows_reader_required…。
    return {{"status", "unreadable"}, {"reason", "windows_reader_required"}, {"segments", Json::array()}};
// 【逐行批注】 处理条件编译指令：#endif。
#endif
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}

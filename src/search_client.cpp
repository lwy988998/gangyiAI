#include "search_client.hpp"

#include <curl/curl.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <ctime>
#include <vector>

namespace gangyi {
namespace {

using json = nlohmann::json;
namespace fs = std::filesystem;

std::string env(const char* name, const std::string& fallback = {}) {
    const char* value = std::getenv(name);
    return value ? value : fallback;
}

long long env_ll(const char* name, long long fallback) {
    try {
        const std::string value = env(name, std::to_string(fallback));
        size_t pos = 0;
        const long long parsed = std::stoll(value, &pos);
        return pos == value.size() ? parsed : fallback;
    } catch (...) {
        return fallback;
    }
}

size_t writeBody(char* data, size_t size, size_t count, void* user) {
    static_cast<std::string*>(user)->append(data, size * count);
    return size * count;
}

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n\f\v");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n\f\v");
    return value.substr(first, last - first + 1);
}

std::string lower(std::string value) {
    for (char& ch : value) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return value;
}

bool containsAny(const std::string& text, const std::vector<std::string>& needles) {
    for (const auto& needle : needles) {
        if (!needle.empty() && text.find(needle) != std::string::npos) return true;
    }
    return false;
}

int countChineseChars(const std::string& text) {
    int count = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        if (c >= 0xE4) {
            ++count;
        }
    }
    return count;
}

std::string sha256Hex(const std::string& input) {
    static constexpr std::array<uint32_t, 64> k = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
        0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
        0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
        0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
        0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
    auto rotr = [](uint32_t value, uint32_t bits) { return (value >> bits) | (value << (32U - bits)); };
    auto ch = [](uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); };
    auto maj = [](uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); };
    auto b0 = [&](uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); };
    auto b1 = [&](uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); };
    auto s0 = [&](uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); };
    auto s1 = [&](uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); };
    std::array<uint32_t, 8> h = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    std::vector<unsigned char> data(input.begin(), input.end());
    const uint64_t bitLen = static_cast<uint64_t>(data.size()) * 8ULL;
    data.push_back(0x80);
    while ((data.size() % 64) != 56) data.push_back(0x00);
    for (int shift = 7; shift >= 0; --shift) data.push_back(static_cast<unsigned char>((bitLen >> (shift * 8)) & 0xff));
    for (size_t offset = 0; offset < data.size(); offset += 64) {
        std::array<uint32_t, 64> w{};
        for (size_t i = 0; i < 16; ++i) {
            const size_t base = offset + i * 4;
            w[i] = (static_cast<uint32_t>(data[base]) << 24) |
                   (static_cast<uint32_t>(data[base + 1]) << 16) |
                   (static_cast<uint32_t>(data[base + 2]) << 8) |
                   static_cast<uint32_t>(data[base + 3]);
        }
        for (size_t i = 16; i < 64; ++i) w[i] = s1(w[i - 2]) + w[i - 7] + s0(w[i - 15]) + w[i - 16];
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (size_t i = 0; i < 64; ++i) {
            const uint32_t t1 = hh + b1(e) + ch(e, f, g) + k[i] + w[i];
            const uint32_t t2 = b0(a) + maj(a, b, c);
            hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (uint32_t value : h) {
        out << std::setw(8) << value;
    }
    return out.str();
}

std::string hostFromUrl(const std::string& url) {
    std::string value = lower(url);
    const auto scheme = value.find("://");
    if (scheme != std::string::npos) value = value.substr(scheme + 3);
    const auto slash = value.find('/');
    if (slash != std::string::npos) value = value.substr(0, slash);
    const auto at = value.find('@');
    if (at != std::string::npos) value = value.substr(at + 1);
    const auto colon = value.find(':');
    if (colon != std::string::npos) value = value.substr(0, colon);
    return value;
}

std::string inferSource(const std::string& url, const std::string& title, const std::string& description) {
    const std::string host = hostFromUrl(url);
    const std::string merged = lower(title + " " + description + " " + host);
    if (merged.find("khan") != std::string::npos) return "Khan Academy";
    if (merged.find("3blue1brown") != std::string::npos || merged.find("3b1b") != std::string::npos) return "3Blue1Brown";
    if (merged.find("bilibili") != std::string::npos || merged.find("b23.tv") != std::string::npos || merged.find("b站") != std::string::npos) return "B站";
    if (merged.find("youtube") != std::string::npos || merged.find("youtu.be") != std::string::npos) return "YouTube";
    if (merged.find("github") != std::string::npos) return "GitHub";
    if (merged.find("developer.mozilla") != std::string::npos || merged.find("mdn") != std::string::npos) return "MDN";
    if (merged.find("coursera") != std::string::npos) return "Coursera";
    if (merged.find("udemy") != std::string::npos) return "Udemy";
    if (merged.find("stackoverflow") != std::string::npos) return "Stack Overflow";
    return host.empty() ? "Web" : host;
}

std::string inferType(const std::string& source, const std::string& title, const std::string& description) {
    const std::string merged = lower(source + " " + title + " " + description);
    if (containsAny(merged, {"github", "repository", "repo", "源码", "开源"})) return "开源项目";
    if (containsAny(merged, {"mdn", "docs", "documentation", "官方文档", "api reference", "api文档"})) return "官方文档";
    if (containsAny(merged, {"video", "youtube", "bilibili", "youtube"})) return "视频教程";
    if (containsAny(merged, {"coursera", "udemy", "course", "课程", "lesson"})) return "在线课程";
    if (containsAny(merged, {"exercise", "practice", "quiz", "题库", "练习"})) return "练习题库";
    if (containsAny(merged, {"project", "实战", "demo", "案例"})) return "项目实战";
    if (containsAny(merged, {"compiler", "environment", "工具", "环境", "setup"})) return "工具环境";
    if (containsAny(merged, {"community", "forum", "discord", "群", "社区"})) return "社区资源";
    return "图文教程";
}

std::string inferDifficulty(const std::string& title, const std::string& description) {
    const std::string merged = lower(title + " " + description);
    if (containsAny(merged, {"advanced", "高级", "expert", "pro"})) return "高级";
    if (containsAny(merged, {"intermediate", "进阶", "中级"})) return "进阶";
    return "入门";
}

std::string inferLanguage(const std::string& title, const std::string& description) {
    const std::string merged = title + " " + description;
    return countChineseChars(merged) >= 4 ? "中文" : "英文";
}

bool inferFree(const std::string& source, const std::string& description) {
    const std::string merged = lower(source + " " + description);
    if (merged.find("free") != std::string::npos || merged.find("open") != std::string::npos || merged.find("免费") != std::string::npos) return true;
    if (containsAny(merged, {"coursera", "udemy"}) && merged.find("free") == std::string::npos) return false;
    return true;
}

double inferScore(double score, const std::string& title, const std::string& description) {
    if (score <= 1.0) return std::round(score * 100.0);
    if (score > 100.0) return 100.0;
    if (score <= 0.0) {
        if (containsAny(lower(title + " " + description), {"official", "官方", "khan", "mdn", "github", "youtube"})) return 82.0;
        return 70.0;
    }
    return score;
}

std::string recommendReason(const std::string& domain) {
    if (domain == "programming") return "适合作为编程学习的真实参考资料，覆盖实现、文档和练习。";
    if (domain == "math") return "适合数学学习，便于理解概念、观看推导和做题。";
    if (domain == "language") return "适合语言学习，兼顾输入、练习和真实语境。";
    if (domain == "office") return "适合办公技能入门和模板化练习。";
    if (domain == "design") return "适合设计类学习，能直接观察作品与方法。";
    if (domain == "ai") return "适合 AI 学习，兼顾原理、工具和实践案例。";
    if (domain == "guitar") return "适合吉他练习，便于按阶段掌握动作和曲目。";
    if (domain == "nextjs") return "适合 Next.js 学习，接近官方实践路径。";
    if (domain == "aiDrawing") return "适合 AI 绘图学习，能快速看到实际效果。";
    if (domain == "pcBuild") return "适合装机和硬件入门，便于对照实操。";
    if (domain == "examEnglish") return "适合考试英语复习，方便做题和查漏补缺。";
    if (domain == "photography") return "适合摄影学习，便于观察范例和参数搭配。";
    return "作为该主题的补充学习资源较合适。";
}

std::vector<std::string> domainQueries(const std::string& domain, const std::string& goal) {
    const std::string q = trim(goal);
    if (domain == "programming") return {q + " 官方文档", q + " GitHub 开源项目", q + " tutorial", q + " examples", q + " practice"};
    if (domain == "math") return {q + " Khan Academy", q + " 3Blue1Brown", q + " 练习题", q + " proof", q + " lecture"};
    if (domain == "language") return {q + " learning resources", q + " practice", q + " listening", q + " speaking", q + " course"};
    if (domain == "office") return {q + " tutorial", q + " template", q + " practice", q + " official docs"};
    if (domain == "design") return {q + " portfolio", q + " tutorial", q + " case study", q + " design system"};
    if (domain == "ai") return {q + " official docs", q + " course", q + " GitHub", q + " tutorial", q + " examples"};
    if (domain == "guitar") return {q + " tutorial", q + " chords", q + " practice", q + " lesson"};
    if (domain == "nextjs") return {q + " official docs", q + " GitHub", q + " tutorial", q + " example app"};
    if (domain == "aiDrawing") return {q + " prompt guide", q + " tutorial", q + " examples", q + " art community"};
    if (domain == "pcBuild") return {q + " build guide", q + " hardware review", q + " tutorial", q + " forum"};
    if (domain == "examEnglish") return {q + " exam practice", q + " vocabulary", q + " listening", q + " reading"};
    if (domain == "photography") return {q + " tutorial", q + " exposure", q + " composition", q + " examples"};
    return {q, q + " tutorial", q + " official docs", q + " examples"};
}

std::string detectDomain(const std::string& goal) {
    const std::string text = lower(goal);
    if (containsAny(text, {"python", "java", "cpp", "c++", "rust", "go", "编程", "代码", "开发", "api", "web", "frontend", "backend", "算法"})) return "programming";
    if (containsAny(text, {"数学", "代数", "几何", "微积分", "概率", "统计", "函数", "导数", "matrix"})) return "math";
    if (containsAny(text, {"英语", "日语", "法语", "韩语", "语言", "听力", "口语", "词汇", "语法"})) return "language";
    if (containsAny(text, {"office", "excel", "word", "ppt", "pptx", "表格", "文档", "办公"})) return "office";
    if (containsAny(text, {"design", "ui", "ux", "视觉", "平面", "品牌", "figma", "sketch"})) return "design";
    if (containsAny(text, {"ai", "llm", "机器学习", "深度学习", "模型", "prompt", "人工智能"})) return "ai";
    if (containsAny(text, {"guitar", "吉他", "弹唱", "和弦"})) return "guitar";
    if (containsAny(text, {"nextjs", "next.js", "react", "前端框架"})) return "nextjs";
    if (containsAny(text, {"绘画", "画图", "midjourney", "stable diffusion", "aigc", "ai绘图"})) return "aiDrawing";
    if (containsAny(text, {"装机", "电脑", "硬件", "显卡", "cpu", "主板", "内存"})) return "pcBuild";
    if (containsAny(text, {"考研英语", "四六级", "雅思", "托福", "英语考试"})) return "examEnglish";
    if (containsAny(text, {"摄影", "相机", "镜头", "构图", "曝光"})) return "photography";
    return "general";
}

struct HttpResult { long status = 0; std::string body; CURLcode error = CURLE_OK; bool directRetry = false; };

int searchProgress(void* user, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    const auto* cancelled = static_cast<const std::function<bool()>*>(user);
    try { return cancelled && *cancelled && (*cancelled)() ? 1 : 0; } catch (...) { return 1; }
}
HttpResult postJson(const std::string& url, const std::string& key, const json& body, long timeoutMs,
                    const std::function<bool()>& cancelled = {}) {
    HttpResult result;
    CURL* curl = curl_easy_init();
    if (!curl) { result.error = CURLE_FAILED_INIT; return result; }
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!key.empty()) {
        const std::string auth = "Authorization: Bearer " + trim(key);
        headers = curl_slist_append(headers, auth.c_str());
    }
    const std::string payload = body.dump();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, searchProgress);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &cancelled);
    result.error = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);
    long usedProxy = 0;
#if LIBCURL_VERSION_NUM >= 0x080700
    curl_easy_getinfo(curl, CURLINFO_USED_PROXY, &usedProxy);
#endif
    if (result.error != CURLE_OK && result.error != CURLE_ABORTED_BY_CALLBACK && usedProxy && url.rfind("https://api.bocha", 0) == 0) {
        result.body.clear();
        result.status = 0;
        result.directRetry = true;
        curl_easy_setopt(curl, CURLOPT_NOPROXY, "*");
        result.error = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return result;
}

std::vector<SearchResource> parseBocha(const std::string& body) {
    std::vector<SearchResource> out;
    json parsed = json::parse(body, nullptr, false);
    if (!parsed.is_object()) return out;
    json items = json::array();
    if (parsed.contains("data") && parsed["data"].is_object() && parsed["data"].contains("webPages") &&
        parsed["data"]["webPages"].is_object() && parsed["data"]["webPages"].contains("value") &&
        parsed["data"]["webPages"]["value"].is_array()) {
        items = parsed["data"]["webPages"]["value"];
    } else if (parsed.contains("data") && parsed["data"].is_object() && parsed["data"].contains("results") &&
               parsed["data"]["results"].is_array()) {
        items = parsed["data"]["results"];
    } else if (parsed.contains("results") && parsed["results"].is_array()) {
        items = parsed["results"];
    }
    for (const auto& item : items) {
        if (!item.is_object()) continue;
        SearchResource resource;
        resource.title = item.value("name", item.value("title", ""));
        resource.url = item.value("url", item.value("link", ""));
        resource.description = item.value("summary", item.value("snippet", item.value("content", item.value("description", ""))));
        resource.source = item.value("siteName", json()).is_string() ? item["siteName"].get<std::string>() : "";
        resource.publishedAt = item.value("datePublished", json()).is_string() ? item["datePublished"].get<std::string>() : "";
        resource.updatedAt = item.value("dateModified", json()).is_string() ? item["dateModified"].get<std::string>() : "";
        resource.score = inferScore(item.value("score", 0.0), resource.title, resource.description);
        out.push_back(std::move(resource));
    }
    return out;
}

std::vector<SearchResource> normalizeResources(std::vector<SearchResource> resources, const std::string& domain) {
    std::set<std::string> seen;
    std::vector<SearchResource> out;
    for (auto& resource : resources) {
        resource.url = trim(resource.url);
        resource.title = trim(resource.title);
        resource.description = trim(resource.description);
        if (resource.url.empty()) continue;
        std::string key = resource.url.substr(0, resource.url.find('#'));
        if (!seen.insert(key).second) {
            auto& saved = *std::find_if(out.begin(), out.end(), [&](const auto& item) { return item.url.substr(0, item.url.find('#')) == key; });
            for (const auto& provider : resource.searchProviders)
                if (std::find(saved.searchProviders.begin(), saved.searchProviders.end(), provider) == saved.searchProviders.end()) saved.searchProviders.push_back(provider);
            continue;
        }
        if (resource.source.empty()) resource.source = inferSource(resource.url, resource.title, resource.description);
        resource.type = inferType(resource.source, resource.title, resource.description);
        resource.difficulty = inferDifficulty(resource.title, resource.description);
        resource.language = inferLanguage(resource.title, resource.description);
        resource.free = inferFree(resource.source, resource.description);
        resource.score = inferScore(resource.score, resource.title, resource.description);
        resource.reason = recommendReason(domain);
        out.push_back(std::move(resource));
    }
    std::sort(out.begin(), out.end(), [](const SearchResource& a, const SearchResource& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.title < b.title;
    });
    return out;
}

json serializeCache(const std::vector<SearchResource>& resources) {
    json arr = json::array();
    for (const auto& resource : resources) {
        arr.push_back({{"title", resource.title},
                       {"url", resource.url},
                       {"source", resource.source},
                       {"description", resource.description},
                       {"type", resource.type},
                       {"difficulty", resource.difficulty},
                       {"language", resource.language},
                       {"free", resource.free},
                       {"score", resource.score},
                       {"reason", resource.reason}});
        arr.back()["publishedAt"] = resource.publishedAt;
        arr.back()["updatedAt"] = resource.updatedAt;
        arr.back()["searchProviders"] = resource.searchProviders;
    }
    return { {"createdAt", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()}, {"resources", arr} };
}

std::optional<std::vector<SearchResource>> readCache(const std::string& dir, const std::string& key, long long ttlSeconds) {
    try {
        const fs::path path = fs::path(dir) / (key + ".json");
        if (!fs::exists(path)) return std::nullopt;
        std::ifstream in(path, std::ios::binary);
        if (!in) return std::nullopt;
        std::ostringstream buffer;
        buffer << in.rdbuf();
        json parsed = json::parse(buffer.str(), nullptr, false);
        if (!parsed.is_object() || !parsed.contains("createdAt") || !parsed.contains("resources") || !parsed["resources"].is_array()) return std::nullopt;
        const long long createdAt = parsed.value("createdAt", 0LL);
        const long long now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (now < createdAt || now - createdAt > ttlSeconds) return std::nullopt;
        std::vector<SearchResource> resources;
        for (const auto& item : parsed["resources"]) {
            if (!item.is_object()) continue;
            SearchResource resource;
            resource.title = item.value("title", "");
            resource.url = item.value("url", "");
            resource.source = item.value("source", "");
            resource.description = item.value("description", "");
            resource.type = item.value("type", "");
            resource.difficulty = item.value("difficulty", "");
            resource.language = item.value("language", "");
            resource.free = item.value("free", true);
            resource.score = item.value("score", 70.0);
            resource.reason = item.value("reason", "");
            resource.publishedAt = item.value("publishedAt", "");
            resource.updatedAt = item.value("updatedAt", "");
            resource.searchProviders = item.value("searchProviders", std::vector<std::string>{});
            resources.push_back(std::move(resource));
        }
        return resources;
    } catch (...) {
        return std::nullopt;
    }
}

void writeCache(const std::string& dir, const std::string& key, const std::vector<SearchResource>& resources) {
    try {
        fs::create_directories(dir);
        const fs::path path = fs::path(dir) / (key + ".json");
        const fs::path temp = path.string() + ".tmp";
        const json payload = serializeCache(resources);
        {
            std::ofstream out(temp, std::ios::binary | std::ios::trunc);
            if (!out) return;
            out << payload.dump(2);
            if (!out.good()) return;
        }
        std::error_code ec;
        fs::rename(temp, path, ec);
        if (ec) {
            fs::remove(path, ec);
            ec.clear();
            fs::rename(temp, path, ec);
            if (ec) fs::remove(temp, ec);
        }
    } catch (...) {}
}

std::vector<std::string> detectQueries(const std::string& domain, const std::string& goal) {
    return domainQueries(domain, goal);
}

// 搜索结果使用规范链接去重；原网站与搜索引擎来源分别保留。
std::string searchURL(std::string url) {
    url = trim(url.substr(0, url.find('#')));
    const auto scheme = url.find("://");
    if (scheme == std::string::npos) return {};
    const auto protocol = lower(url.substr(0, scheme));
    if (protocol != "http" && protocol != "https") return {};
    const auto hostEnd = url.find_first_of("/?", scheme + 3);
    const auto authority = url.substr(scheme + 3, hostEnd == std::string::npos ? std::string::npos : hostEnd - scheme - 3);
    if (authority.empty() || authority.find('@') != std::string::npos) return {};
    std::string host = lower(authority);
    const auto defaultPort = protocol == "https" ? ":443" : ":80";
    if (host.size() > std::char_traits<char>::length(defaultPort) &&
        host.compare(host.size() - std::char_traits<char>::length(defaultPort), std::string::npos, defaultPort) == 0)
        host.resize(host.size() - std::char_traits<char>::length(defaultPort));
    auto suffix = hostEnd == std::string::npos ? std::string() : url.substr(hostEnd);
    const auto queryStart = suffix.find('?');
    auto path = suffix.substr(0, queryStart);
    if (path.empty()) path = "/";
    std::vector<std::string> args;
    if (queryStart != std::string::npos) {
        std::istringstream query(suffix.substr(queryStart + 1));
        std::string item;
        while (std::getline(query, item, '&')) {
            const auto name = lower(item.substr(0, item.find('=')));
            if (name.rfind("utm_", 0) != 0 && name != "gclid" && name != "fbclid" && !item.empty()) args.push_back(item);
        }
    }
    std::string result = protocol + "://" + host + path;
    for (std::size_t i = 0; i < args.size(); ++i) result += (i ? "&" : "?") + args[i];
    return result;
}

std::string utcDate(std::chrono::system_clock::time_point value) {
    const auto raw = std::chrono::system_clock::to_time_t(value);
    std::tm parts{};
#ifdef _WIN32
    gmtime_s(&parts, &raw);
#else
    gmtime_r(&raw, &parts);
#endif
    std::ostringstream output;
    output << std::put_time(&parts, "%Y-%m-%d");
    return output.str();
}

std::string deepseekQuery(const ResourceQuery& request) {
    auto query = trim(request.query);
    if (request.freshness != "noLimit") {
        const std::map<std::string, int> days{{"oneDay", 1}, {"oneWeek", 7}, {"oneMonth", 30}, {"oneYear", 365}};
        const auto found = days.find(request.freshness);
        if (found != days.end()) query += " after:" + utcDate(std::chrono::system_clock::now() - std::chrono::hours(24 * found->second));
        else {
            query += " after:" + request.freshness.substr(0, 10);
            if (request.freshness.size() == 22) query += " before:" + request.freshness.substr(12, 10);
        }
    }
    // API 以 Unicode 字符计数，不按 UTF-8 字节截断或改写用户搜索词。
    std::size_t characters = 0;
    for (const unsigned char byte : query) if ((byte & 0xc0) != 0x80) ++characters;
    return characters <= 128 ? query : std::string();
}

ResourceQueryResult providerQuery(const std::string& provider, const ResourceQuery& request, const std::string& cacheDir) {
    ResourceQueryResult result;
    result.status = "missing_key";
    const bool deepseek = provider == "deepseek";
    auto base = trim(env(deepseek ? "DEEPSEEK_SEARCH_BASE_URL" : "BOCHA_BASE_URL",
                         deepseek ? "https://api.deepseek.com" : "https://api.bochaai.com"));
    while (!base.empty() && base.back() == '/') base.pop_back();
    if (deepseek && base.size() >= 3 && base.compare(base.size() - 3, 3, "/v1") == 0) base.resize(base.size() - 3);
    auto key = trim(env(deepseek ? "DEEPSEEK_SEARCH_API_KEY" : "BOCHA_API_KEY"));
    if (deepseek && key.empty()) key = trim(env("DEEPSEEK_API_KEY"));
    // 只复用官方 DeepSeek 的模型凭据，其他模型服务商的密钥不会发给 DeepSeek。
    if (deepseek && key.empty() && hostFromUrl(env("AI_BASE_URL", "https://api.deepseek.com")) == "api.deepseek.com")
        key = trim(env("AI_API_KEY"));
    if (key.empty()) return result;
    if (request.cancelled && request.cancelled()) { result.status = "cancelled"; return result; }
    const auto query = deepseek ? deepseekQuery(request) : request.query;
    if (query.empty()) { result.status = "query_too_long"; return result; }
    const auto cacheKey = sha256Hex("dual-v1:" + provider + ":" + base + ":" + sha256Hex(key) + ":" + query + ":" + request.freshness + ":" + std::to_string(request.count));
    if (!request.refresh) if (const auto cached = readCache(cacheDir, cacheKey, request.freshness == "noLimit" ? 86400LL : 3600LL)) {
        result.resources = *cached; result.status = "completed"; result.cached = true; return result;
    }
    json body = deepseek ? json{{"query", query}, {"count", request.count}, {"need_content", false}} :
        json{{"query", query}, {"freshness", request.freshness}, {"count", request.count}, {"summary", true}};
    const auto response = postJson(base + (deepseek ? "/v1/tools/search" : "/v1/web-search"), key, body,
        static_cast<long>(std::clamp(env_ll("RESOURCE_SEARCH_TIMEOUT_MS", 15000LL), 1000LL, 60000LL)), request.cancelled);
    result.attempts = response.error == CURLE_FAILED_INIT ? 0 : response.directRetry ? 2 : 1;
    if (response.error == CURLE_ABORTED_BY_CALLBACK) result.status = "cancelled";
    else if (response.error != CURLE_OK) result.status = "network_error";
    else if (response.status < 200 || response.status >= 300) result.status = "http_" + std::to_string(response.status);
    else {
        try {
            const auto parsed = json::parse(response.body);
            if (!parsed.is_object()) throw std::runtime_error("搜索响应不是对象");
            if (deepseek) {
                if (!parsed.contains("results") || !parsed["results"].is_array()) throw std::runtime_error("缺少搜索结果数组");
                for (const auto& item : parsed["results"]) {
                    if (!item.is_object() || !item.value("url", json()).is_string() || !item.value("title", json()).is_string())
                        throw std::runtime_error("搜索条目结构无效");
                    SearchResource resource;
                    resource.title = item.at("title").get<std::string>(); resource.url = searchURL(item.at("url").get<std::string>());
                    resource.description = item.value("summary", json()).is_string() ? item["summary"].get<std::string>() : item.value("snippet", "");
                    resource.source = item.value("site_name", json()).is_string() ? item["site_name"].get<std::string>() : "";
                    resource.publishedAt = item.value("date", json()).is_string() ? item["date"].get<std::string>() : "";
                    resource.score = item.value("rerank_score", json()).is_number() ? std::clamp(item["rerank_score"].get<double>() / 3.0 * 100.0, 0.0, 100.0) : 0.0;
                    result.resources.push_back(std::move(resource));
                }
            } else {
                if (parsed.contains("code") && parsed["code"] != 200) throw std::runtime_error("博查返回错误");
                const bool valid = (parsed.value("data", json()).is_object() &&
                    ((parsed["data"].value("webPages", json()).is_object() && parsed["data"]["webPages"].value("value", json()).is_array()) ||
                     parsed["data"].value("results", json()).is_array())) || parsed.value("results", json()).is_array();
                if (!valid) throw std::runtime_error("缺少博查结果数组");
                result.resources = parseBocha(response.body);
                for (auto& resource : result.resources) resource.url = searchURL(resource.url);
            }
            result.resources = normalizeResources(std::move(result.resources), detectDomain(request.query));
            for (auto& resource : result.resources) resource.searchProviders = {provider};
            result.status = "completed";
            writeCache(cacheDir, cacheKey, result.resources);
        } catch (...) { result.resources.clear(); result.status = "invalid_response"; }
    }
    return result;
}

}  // namespace

SearchClient::SearchClient()
    : cacheDir_(env("RESOURCE_SEARCH_CACHE_DIR", env("LOCALAPPDATA").empty()
          ? "data/resource-search-cache" : (fs::path(env("LOCALAPPDATA")) / "GangyiAI" / "resource-search-cache").string())) {
    static const int initialized = curl_global_init(CURL_GLOBAL_DEFAULT);
    (void)initialized;
}

std::string resourceDigest(const std::string& input) { return sha256Hex(input); }

bool SearchClient::testBochaKey(const std::string& key, std::string* diagnostic) {
    if (trim(key).empty()) { if (diagnostic) *diagnostic = "missing_key"; return false; }
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::string base = trim(env("BOCHA_BASE_URL", "https://api.bochaai.com"));
    while (!base.empty() && base.back() == '/') base.pop_back();
    const auto response = postJson(base + "/v1/web-search", key,
        {{"query", u8"高中数学知识点"}, {"count", 1}, {"summary", false}}, 8000);
    if (response.error != CURLE_OK) {
        if (diagnostic) *diagnostic = "network_error:" + std::to_string(static_cast<int>(response.error));
        return false;
    }
    if (response.status < 200 || response.status >= 300) {
        if (diagnostic) *diagnostic = "http:" + std::to_string(response.status);
        return false;
    }
    const auto body = json::parse(response.body, nullptr, false);
    const bool valid = body.is_object() && (!body.contains("code") ||
        (body["code"].is_number_integer() && body["code"].get<int>() == 200));
    if (diagnostic) *diagnostic = valid ? (response.directRetry ? "ok_direct" : "ok") : "invalid_response";
    return valid;
}

std::vector<SearchResource> SearchClient::search(const std::string& goal, size_t limit,
                                               const std::function<bool()>& cancelled) const {
    std::vector<SearchResource> collected;
    lastProvider_ = "deepseek+bocha"; fallbackUsed_ = false;
    if (limit == 0) return collected;
    try {
        for (const auto& queryText : detectQueries(detectDomain(goal), goal)) {
            if (cancelled && cancelled()) break;
            ResourceQuery request; request.query = queryText; request.count = 5; request.cancelled = cancelled;
            const auto result = query(request);
            fallbackUsed_ = fallbackUsed_ || result.status != "completed";
            collected.insert(collected.end(), result.resources.begin(), result.resources.end());
        }
        collected = normalizeResources(std::move(collected), detectDomain(goal));
        if (collected.size() > limit) collected.resize(limit);
    } catch (...) { fallbackUsed_ = true; }
    return collected;
}

ResourceQueryResult SearchClient::query(const ResourceQuery& request) const {
    if (trim(request.query).empty() || request.query.size() > 3000 || request.count < 1 || request.count > 50)
        throw std::invalid_argument("搜索词或结果数量无效");
    const std::set<std::string> ranges{"noLimit", "oneYear", "oneMonth", "oneWeek", "oneDay"};
    const bool customDate = (request.freshness.size() == 10 || request.freshness.size() == 22) &&
        request.freshness.find_first_not_of("0123456789-.") == std::string::npos;
    if (!ranges.count(request.freshness) && !customDate) throw std::invalid_argument("检索日期范围无效");
    // 两个网络线程共享取消回调时串行读取档案，避免并发操作同一 SQLite 连接。
    auto guarded = request;
    const auto mutex = std::make_shared<std::mutex>();
    guarded.cancelled = [mutex, callback = request.cancelled] {
        std::lock_guard<std::mutex> lock(*mutex);
        try { return callback && callback(); } catch (...) { return true; }
    };
    if (guarded.cancelled()) throw std::runtime_error("资料检索已中断");
    auto deepseek = std::async(std::launch::async, [&] { return providerQuery("deepseek", guarded, cacheDir_); });
    auto bocha = std::async(std::launch::async, [&] { return providerQuery("bocha", guarded, cacheDir_); });
    ResourceQueryResult result;
    std::map<std::string, SearchResource> merged;
    std::map<std::string, double> ranks;
    int completed = 0;
    for (const auto& provider : {std::string("deepseek"), std::string("bocha")}) {
        ResourceQueryResult part;
        try { part = provider == "deepseek" ? deepseek.get() : bocha.get(); }
        catch (...) { part.status = "internal_error"; }
        result.attempts += part.attempts;
        if (part.status == "completed") ++completed;
        result.providers.push_back({{"provider", provider}, {"status", part.status}, {"attempts", part.attempts},
            {"cached", part.cached}, {"results", part.resources.size()}});
        std::size_t rank = 0;
        for (auto& resource : part.resources) {
            const auto key = resource.url;
            // 采用名次融合，不直接比较两家不同量纲的原始分数。
            ranks[key] += 1.0 / (60.0 + static_cast<double>(++rank));
            if (!merged.count(key)) merged[key] = resource;
            auto& saved = merged[key];
            if (std::find(saved.searchProviders.begin(), saved.searchProviders.end(), provider) == saved.searchProviders.end()) saved.searchProviders.push_back(provider);
            if (saved.description.size() < resource.description.size()) saved.description = resource.description;
            if (saved.publishedAt.empty()) saved.publishedAt = resource.publishedAt;
        }
    }
    result.status = guarded.cancelled() ? "cancelled" : completed == 2 ? "completed" : completed == 1 ? "partial" : "failed";
    result.cached = std::all_of(result.providers.begin(), result.providers.end(), [](const auto& item) { return item.at("cached") == true; });
    if (result.status != "cancelled") for (auto& [key, resource] : merged) {
        resource.score = ranks[key] * 3050.0;
        result.resources.push_back(std::move(resource));
    }
    result.resources = normalizeResources(std::move(result.resources), detectDomain(request.query));
    return result;
}

}  // namespace gangyi

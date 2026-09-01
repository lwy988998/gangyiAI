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
#include <iomanip>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
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

std::string envToLower(const char* name, const std::string& fallback) {
    return lower(trim(env(name, fallback)));
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

std::vector<std::string> splitWords(const std::string& text) {
    std::vector<std::string> out;
    std::string current;
    for (char ch : text) {
        if (std::isalnum(static_cast<unsigned char>(ch)) || static_cast<unsigned char>(ch) >= 0x80) {
            current.push_back(ch);
        } else if (!current.empty()) {
            out.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) out.push_back(current);
    return out;
}

std::vector<std::string> buildNgrams(const std::string& goal) {
    const std::string compact = lower(trim(goal));
    std::vector<std::string> grams;
    const std::vector<std::string> words = splitWords(compact);
    for (const auto& word : words) {
        if (word.size() >= 2) grams.push_back(word.substr(0, std::min<size_t>(4, word.size())));
        if (word.size() >= 3) grams.push_back(word.substr(0, std::min<size_t>(3, word.size())));
        if (word.size() >= 4) grams.push_back(word.substr(0, 2));
    }
    if (grams.empty() && compact.size() >= 2) {
        grams.push_back(compact.substr(0, std::min<size_t>(4, compact.size())));
    }
    std::sort(grams.begin(), grams.end());
    grams.erase(std::unique(grams.begin(), grams.end()), grams.end());
    if (grams.size() > 6) grams.resize(6);
    return grams;
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

std::string recommendReason(const std::string& domain, const std::string& type, const std::string& source) {
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

struct HttpResult { long status = 0; std::string body; };

HttpResult postJson(const std::string& url, const std::string& key, const json& body, long timeoutMs, bool bearer = true) {
    HttpResult result;
    CURL* curl = curl_easy_init();
    if (!curl) return result;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!key.empty()) {
        const std::string auth = "Authorization: Bearer " + key;
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
    curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return result;
}

std::vector<SearchResource> parseTavily(const std::string& body) {
    std::vector<SearchResource> out;
    json parsed = json::parse(body, nullptr, false);
    if (!parsed.is_object() || !parsed.contains("results") || !parsed["results"].is_array()) return out;
    for (const auto& item : parsed["results"]) {
        if (!item.is_object()) continue;
        SearchResource resource;
        resource.title = item.value("title", "");
        resource.url = item.value("url", "");
        resource.description = item.value("content", item.value("description", ""));
        resource.score = inferScore(item.value("score", 0.0), resource.title, resource.description);
        out.push_back(std::move(resource));
    }
    return out;
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
        std::string key = lower(resource.url);
        while (!key.empty() && key.back() == '/') key.pop_back();
        if (!seen.insert(key).second) continue;
        resource.source = inferSource(resource.url, resource.title, resource.description);
        resource.type = inferType(resource.source, resource.title, resource.description);
        resource.difficulty = inferDifficulty(resource.title, resource.description);
        resource.language = inferLanguage(resource.title, resource.description);
        resource.free = inferFree(resource.source, resource.description);
        resource.score = inferScore(resource.score, resource.title, resource.description);
        resource.reason = recommendReason(domain, resource.type, resource.source);
        out.push_back(std::move(resource));
    }
    std::sort(out.begin(), out.end(), [](const SearchResource& a, const SearchResource& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.title < b.title;
    });
    if (out.size() > 20) out.resize(20);
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

std::vector<SearchResource> fetchFromProvider(const std::string& provider, const std::string& key, const std::string& baseUrl,
                                              const std::vector<std::string>& queries, const std::string& domain) {
    std::vector<SearchResource> collected;
    const std::string url = provider == "bocha" ? (trim(baseUrl).empty() ? "https://api.bochaai.com" : trim(baseUrl)) + "/v1/web-search"
                                                : "https://api.tavily.com/search";
    for (const auto& query : queries) {
        if (query.empty()) continue;
        json body;
        if (provider == "bocha") {
            body = {{"query", query}, {"count", 5}, {"summary", true}, {"freshness", "noLimit"}};
        } else {
            body = {{"query", query}, {"search_depth", "basic"}, {"max_results", 5},
                    {"include_answer", false}, {"include_raw_content", false}};
        }
        const HttpResult response = postJson(url, key, body, 15000, provider != "bocha");
        if (response.status < 200 || response.status >= 300) continue;
        std::vector<SearchResource> parsed = provider == "bocha" ? parseBocha(response.body) : parseTavily(response.body);
        collected.insert(collected.end(), parsed.begin(), parsed.end());
        if (!collected.empty()) break;
    }
    return normalizeResources(std::move(collected), domain);
}

}  // namespace

SearchClient::SearchClient()
    : provider_(envToLower("SEARCH_PROVIDER", "tavily")),
      fallbackProvider_(envToLower("SEARCH_FALLBACK_PROVIDER", "bocha")),
      cacheDir_(env("RESOURCE_SEARCH_CACHE_DIR", "data/resource-search-cache")) {
    if (provider_ != "bocha") provider_ = "tavily";
    if (fallbackProvider_ != "tavily") fallbackProvider_ = "bocha";
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::vector<SearchResource> SearchClient::search(const std::string& goal, size_t limit) const {
    try {
        const std::string domain = detectDomain(goal);
        const std::string cacheKey = sha256Hex(provider_ + ":" + fallbackProvider_ + ":" + domain + ":" + lower(trim(goal)));
        if (const auto cached = readCache(cacheDir_, cacheKey, env_ll("RESOURCE_SEARCH_CACHE_TTL_SECONDS", 604800LL))) {
            lastProvider_ = "cache";
            fallbackUsed_ = false;
            std::vector<SearchResource> value = *cached;
            if (value.size() > limit) value.resize(limit);
            return value;
        }

        const std::vector<std::string> queries = detectQueries(domain, goal);
        std::string liveProvider = provider_;
        std::vector<SearchResource> result = fetchFromProvider(provider_, env(provider_ == "bocha" ? "BOCHA_API_KEY" : "TAVILY_API_KEY"),
                                                               provider_ == "bocha" ? env("BOCHA_BASE_URL", "https://api.bochaai.com") : std::string{},
                                                               queries, domain);
        bool usedFallback = false;
        if (result.empty()) {
            const std::string fallbackKey = env(fallbackProvider_ == "bocha" ? "BOCHA_API_KEY" : "TAVILY_API_KEY");
            if (!fallbackProvider_.empty() && !fallbackKey.empty()) {
                std::vector<SearchResource> fallback = fetchFromProvider(fallbackProvider_, fallbackKey,
                                                                         fallbackProvider_ == "bocha" ? env("BOCHA_BASE_URL", "https://api.bochaai.com") : std::string{},
                                                                         queries, domain);
                if (!fallback.empty()) {
                    result = std::move(fallback);
                    liveProvider = fallbackProvider_;
                    usedFallback = true;
                }
            }
        }
        lastProvider_ = liveProvider;
        fallbackUsed_ = usedFallback;
        if (!result.empty()) writeCache(cacheDir_, cacheKey, result);
        if (result.size() > limit) result.resize(limit);
        return result;
    } catch (...) {
        lastProvider_ = provider_;
        fallbackUsed_ = false;
        return {};
    }
}

}  // namespace gangyi

#include "plan_cache.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
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

std::string trim(const std::string& input) {
    const auto first = input.find_first_not_of(" \t\r\n\f\v");
    if (first == std::string::npos) return {};
    const auto last = input.find_last_not_of(" \t\r\n\f\v");
    return input.substr(first, last - first + 1);
}

std::string collapseWhitespace(std::string value) {
    std::string out;
    out.reserve(value.size());
    bool inSpace = false;
    for (unsigned char ch : value) {
        if (std::isspace(ch)) {
            inSpace = true;
            continue;
        }
        if (inSpace && !out.empty()) out.push_back(' ');
        out.push_back(static_cast<char>(ch));
        inSpace = false;
    }
    return out;
}

std::string lowerAscii(std::string value) {
    for (char& ch : value) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

std::string normalizeMode(const std::string& mode) {
    const std::string lowered = lowerAscii(trim(collapseWhitespace(mode)));
    return lowered == "lite" ? "lite" : "deep";
}

std::string trimGoalText(const std::string& goal) {
    return trim(goal);
}

std::string nowIso8601() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &time);
#else
    gmtime_r(&time, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

bool parseIso8601Utc(const std::string& value, std::time_t& out) {
    std::tm tm{};
    std::istringstream in(value);
    in >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (in.fail()) return false;
#if defined(_WIN32)
    out = _mkgmtime(&tm);
#else
    out = timegm(&tm);
#endif
    return out != static_cast<std::time_t>(-1);
}

std::string hexEncode(const std::array<unsigned char, 32>& digest) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (unsigned char byte : digest) {
        out << std::setw(2) << static_cast<int>(byte);
    }
    return out.str();
}

std::array<unsigned char, 32> sha256(const std::string& input) {
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
    auto big0 = [&](uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); };
    auto big1 = [&](uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); };
    auto small0 = [&](uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); };
    auto small1 = [&](uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); };

    std::array<uint32_t, 8> h = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

    std::vector<unsigned char> data(input.begin(), input.end());
    const uint64_t bitLen = static_cast<uint64_t>(data.size()) * 8ULL;
    data.push_back(0x80);
    while ((data.size() % 64) != 56) data.push_back(0x00);
    for (int shift = 7; shift >= 0; --shift) data.push_back(static_cast<unsigned char>((bitLen >> (shift * 8)) & 0xff));

    for (size_t chunk = 0; chunk < data.size(); chunk += 64) {
        std::array<uint32_t, 64> w{};
        for (size_t i = 0; i < 16; ++i) {
            const size_t offset = chunk + i * 4;
            w[i] = (static_cast<uint32_t>(data[offset]) << 24) |
                   (static_cast<uint32_t>(data[offset + 1]) << 16) |
                   (static_cast<uint32_t>(data[offset + 2]) << 8) |
                   static_cast<uint32_t>(data[offset + 3]);
        }
        for (size_t i = 16; i < 64; ++i) w[i] = small1(w[i - 2]) + w[i - 7] + small0(w[i - 15]) + w[i - 16];
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (size_t i = 0; i < 64; ++i) {
            const uint32_t t1 = hh + big1(e) + ch(e, f, g) + k[i] + w[i];
            const uint32_t t2 = big0(a) + maj(a, b, c);
            hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    std::array<unsigned char, 32> digest{};
    for (size_t i = 0; i < h.size(); ++i) {
        digest[i * 4 + 0] = static_cast<unsigned char>((h[i] >> 24) & 0xff);
        digest[i * 4 + 1] = static_cast<unsigned char>((h[i] >> 16) & 0xff);
        digest[i * 4 + 2] = static_cast<unsigned char>((h[i] >> 8) & 0xff);
        digest[i * 4 + 3] = static_cast<unsigned char>(h[i] & 0xff);
    }
    return digest;
}

std::string sha256Hex(const std::string& input) {
    return hexEncode(sha256(input));
}

bool hasString(const json& value) {
    return value.is_string() && !trim(value.get<std::string>()).empty();
}

bool hasStringArray(const json& value) {
    if (!value.is_array()) return false;
    for (const auto& item : value) {
        if (!item.is_string()) return false;
    }
    return true;
}

bool validStep(const json& step) {
    return step.is_object() && trim(step.value("title", "")).size() > 0 && trim(step.value("explanation", "")).size() > 0 &&
           trim(step.value("action", "")).size() > 0 && trim(step.value("check", "")).size() > 0;
}

bool validPhase(const json& phase) {
    if (!phase.is_object()) return false;
    if (trim(phase.value("name", "")).empty() || !phase.contains("durationWeeks") || !phase["durationWeeks"].is_number() ||
        trim(phase.value("objective", "")).empty() || trim(phase.value("description", "")).empty() ||
        !phase.contains("topics") || !hasStringArray(phase["topics"])) {
        return false;
    }
    if (phase.contains("steps") && !phase["steps"].is_null()) {
        if (!phase["steps"].is_array()) return false;
        for (const auto& step : phase["steps"]) {
            if (!validStep(step)) return false;
        }
    }
    return true;
}

bool validResource(const json& resource) {
    return resource.is_object() && trim(resource.value("name", "")).size() > 0 && trim(resource.value("type", "")).size() > 0 &&
           trim(resource.value("difficulty", "")).size() > 0 && trim(resource.value("description", "")).size() > 0 &&
           trim(resource.value("url", "")).size() > 0 && resource.contains("free") && resource["free"].is_boolean();
}

bool validProject(const json& project) {
    return project.is_object() && trim(project.value("name", "")).size() > 0 && trim(project.value("difficulty", "")).size() > 0 &&
           project.contains("estimatedHours") && project["estimatedHours"].is_number() && trim(project.value("output", "")).size() > 0 &&
           project.contains("acceptanceCriteria") && hasStringArray(project["acceptanceCriteria"]);
}

}  // namespace

PlanCache::PlanCache(const std::string& cacheDir)
    : dir_(env("AI_PLAN_CACHE_DIR", cacheDir)), ttlSeconds_(env_ll("AI_PLAN_CACHE_TTL_SECONDS", 604800LL)) {}

std::string PlanCache::normalizeGoal(const std::string& goal) {
    return lowerAscii(collapseWhitespace(trim(goal)));
}

std::string PlanCache::cacheKey(const std::string& goal, const std::string& mode) {
    return sha256Hex(normalizeMode(mode) + ":" + normalizeGoal(goal));
}

bool PlanCache::isValidPlan(const json& plan) {
    try {
        if (!plan.is_object()) return false;
        // MockPlan 形状（适配后缓存）：title/summary 非空 + roadmap/courseStructure 非空数组
        if (plan.contains("roadmap") && plan.contains("courseStructure")) {
            return hasString(plan.value("title", "")) && hasString(plan.value("summary", "")) &&
                   plan["roadmap"].is_array() && !plan["roadmap"].empty() &&
                   plan["courseStructure"].is_array() && !plan["courseStructure"].empty();
        }
        // GeneratedPlan 形状（原始 AI 输出）
        if (!hasString(plan.value("title", "")) || !hasString(plan.value("goal", "")) ||
            !plan.contains("durationWeeks") || !plan["durationWeeks"].is_number() || !hasString(plan.value("summary", "")) ||
            !plan.contains("phases") || !plan["phases"].is_array() || plan["phases"].empty() ||
            !plan.contains("resources") || !plan["resources"].is_array() || !plan.contains("projects") || !plan["projects"].is_array() ||
            plan["projects"].empty()) {
            return false;
        }
        for (const auto& phase : plan["phases"]) {
            if (!validPhase(phase)) return false;
        }
        for (const auto& resource : plan["resources"]) {
            if (!validResource(resource)) return false;
        }
        for (const auto& project : plan["projects"]) {
            if (!validProject(project)) return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

std::optional<json> PlanCache::read(const std::string& goal, const std::string& mode) const {
    try {
        const std::string key = cacheKey(goal, mode);
        const fs::path file = fs::path(dir_) / (key + ".json");
        if (!fs::exists(file)) return std::nullopt;
        std::ifstream in(file, std::ios::binary);
        if (!in) return std::nullopt;
        std::ostringstream buffer;
        buffer << in.rdbuf();
        const json cached = json::parse(buffer.str());
        if (!cached.is_object() || !cached.contains("createdAt") || !cached.contains("plan") || !cached["plan"].is_object()) {
            return std::nullopt;
        }
        std::time_t createdAt = 0;
        if (!parseIso8601Utc(cached.value("createdAt", ""), createdAt)) return std::nullopt;
        const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (now < createdAt) return std::nullopt;
        if (static_cast<long long>(now - createdAt) > ttlSeconds_) return std::nullopt;
        if (!isValidPlan(cached["plan"])) return std::nullopt;
        return cached["plan"];
    } catch (...) {
        return std::nullopt;
    }
}

bool PlanCache::write(const std::string& goal, const std::string& mode, const json& plan) const {
    try {
        if (!isValidPlan(plan)) return false;
        fs::create_directories(dir_);
        const std::string key = cacheKey(goal, mode);
        const fs::path file = fs::path(dir_) / (key + ".json");
        const json cached = {
            {"goal", trimGoalText(goal)},
            {"mode", normalizeMode(mode)},
            {"createdAt", nowIso8601()},
            {"plan", plan}};
        const fs::path temp = file.string() + ".tmp";
        {
            std::ofstream out(temp, std::ios::binary | std::ios::trunc);
            if (!out) return false;
            out << cached.dump(2);
            if (!out.good()) return false;
        }
        std::error_code ec;
        fs::rename(temp, file, ec);
        if (ec) {
            fs::remove(file, ec);
            ec.clear();
            fs::rename(temp, file, ec);
            if (ec) {
                fs::remove(temp, ec);
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace gangyi

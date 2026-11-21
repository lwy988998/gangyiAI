#pragma once

#include <nlohmann/json.hpp>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace gangyi {

// 只接受服务端生成的完整 UTC 秒时间，日期归一化后必须仍与原文一致。
inline std::optional<std::time_t> reviewUtcSeconds(const std::string& text) {
    if (text.size() != 20) return std::nullopt;
    std::tm parsed{};
    std::istringstream input(text);
    input >> std::get_time(&parsed, "%Y-%m-%dT%H:%M:%SZ");
    if (input.fail()) return std::nullopt;
#ifdef _WIN32
    const auto seconds = _mkgmtime(&parsed);
#else
    const auto seconds = timegm(&parsed);
#endif
    if (seconds < 0) return std::nullopt;
    std::tm normalized{};
#ifdef _WIN32
    if (gmtime_s(&normalized, &seconds)) return std::nullopt;
#else
    if (!gmtime_r(&seconds, &normalized)) return std::nullopt;
#endif
    std::ostringstream encoded;
    encoded << std::put_time(&normalized, "%Y-%m-%dT%H:%M:%SZ");
    return encoded.str() == text ? std::optional<std::time_t>(seconds) : std::nullopt;
}

inline int checkedReviewDelay(const nlohmann::json& value) {
    if (!value.is_number_integer()) throw std::invalid_argument("复习间隔须为1至31536000秒的整数");
    if (value.is_number_unsigned()) {
        const auto count = value.get<unsigned long long>();
        if (count >= 1 && count <= 31536000) return static_cast<int>(count);
    } else {
        const auto count = value.get<long long>();
        if (count >= 1 && count <= 31536000) return static_cast<int>(count);
    }
    throw std::invalid_argument("复习间隔须为1至31536000秒的整数");
}

// 间隔只说明实际提交的时间条件。分数、帮助程度和长期记忆效果须分别核对。
inline nlohmann::json reviewIntervalTiming(const nlohmann::json& review,
                                          const nlohmann::json& evidence,
                                          const std::string& completedAt) {
    using Json = nlohmann::json;
    Json result = {{"schemaVersion", 1}, {"status", "not_requested"}, {"learningEffectVerified", false},
                   {"clockSource", "local_server_clock"}};
    if (!review.contains("minimumDelaySeconds")) return result;
    result["status"] = "unavailable";
    int required;
    try { required = checkedReviewDelay(review.at("minimumDelaySeconds")); }
    catch (const std::invalid_argument&) { return result; }
    result["minimumDelaySeconds"] = required;
    const auto baseline = reviewUtcSeconds(review.value("scheduledAt", ""));
    const auto completion = reviewUtcSeconds(completedAt);
    if (!baseline || !completion || *completion < *baseline || !evidence.is_array() || evidence.empty()) return result;
    long long minimum = 315360000000LL;
    std::size_t independent = 0;
    for (const auto& item : evidence) {
        if (!item.is_object()) return result;
        const auto submitted = reviewUtcSeconds(item.value("submittedAt", ""));
        const auto evaluated = reviewUtcSeconds(item.value("evaluatedAt", ""));
        if (!submitted || !evaluated || *submitted < *baseline || *evaluated < *submitted || *completion < *evaluated) return result;
        minimum = std::min(minimum, static_cast<long long>(*submitted - *baseline));
        if (item.value("helpMode", "") == "independent") ++independent;
    }
    result["baselineAt"] = review.at("scheduledAt");
    result["minimumElapsedSeconds"] = minimum;
    result["independentAnswerCount"] = independent;
    result["questionCount"] = evidence.size();
    result["status"] = minimum >= required ? "delay_met" : "too_early";
    return result;
}

} // namespace gangyi

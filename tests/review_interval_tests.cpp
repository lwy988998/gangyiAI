#ifdef NDEBUG
#undef NDEBUG
#endif
#include "review_interval.hpp"
#include <cassert>
#include <iostream>

int main() {
    using Json = nlohmann::json;
    using gangyi::reviewIntervalTiming;
    const Json review = {{"scheduledAt", "2026-11-19T01:00:00Z"}, {"minimumDelaySeconds", 60}};
    const auto answer = [](const char* submitted, const char* evaluated, const char* mode = "independent") {
        return Json{{"submittedAt", submitted}, {"evaluatedAt", evaluated}, {"helpMode", mode}};
    };
    Json evidence = Json::array({answer("2026-11-19T01:01:00Z", "2026-11-19T01:02:00Z")});
    auto timing = reviewIntervalTiming(review, evidence, "2026-11-19T01:03:00Z");
    assert(timing.at("status") == "delay_met" && timing.at("minimumElapsedSeconds") == 60);
    assert(timing.at("learningEffectVerified") == false);
    // 迟批改、迟确认不能把间隔前的作答变为间隔后的记忆证据。
    evidence.push_back(answer("2026-11-19T01:00:59Z", "2026-11-19T01:02:00Z", "hint"));
    timing = reviewIntervalTiming(review, evidence, "2026-11-20T01:00:00Z");
    assert(timing.at("status") == "too_early" && timing.at("minimumElapsedSeconds") == 59);
    assert(timing.at("independentAnswerCount") == 1);
    assert(reviewIntervalTiming(Json::object(), evidence, "invalid").at("status") == "not_requested");
    for (const char* invalid : {"", "2026-02-29T01:01:00Z", "2026-11-19", "2026-11-19T01:01:00+08:00", "2026-11-19T01:61:00Z"})
        assert(!gangyi::reviewUtcSeconds(invalid));
    assert(gangyi::reviewUtcSeconds("2024-02-29T00:00:00Z"));
    for (const auto& invalid : {Json(true), Json(-1), Json(0), Json(1.5), Json(31536001), Json(4294967297ULL)}) {
        bool rejected = false;
        try { gangyi::checkedReviewDelay(invalid); } catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
    }
    assert(gangyi::checkedReviewDelay(Json(31536000)) == 31536000);
    evidence = Json::array({answer("2026-11-19T00:59:59Z", "2026-11-19T01:02:00Z")});
    assert(reviewIntervalTiming(review, evidence, "2026-11-19T01:03:00Z").at("status") == "unavailable");
    evidence = Json::array({answer("2026-11-19T01:01:00Z", "2026-11-19T01:00:59Z")});
    assert(reviewIntervalTiming(review, evidence, "2026-11-19T01:03:00Z").at("status") == "unavailable");
    evidence = Json::array({answer("2026-11-19T01:01:00Z", "2026-11-19T01:02:00Z")});
    assert(reviewIntervalTiming(review, evidence, "2026-11-19T01:00:00Z").at("status") == "unavailable");
    evidence[0].erase("submittedAt");
    assert(reviewIntervalTiming(review, evidence, "2026-11-19T01:03:00Z").at("status") == "unavailable");
    std::cout << "复习间隔按实际提交核对、缺失降级和非学习效果边界通过。\n";
}

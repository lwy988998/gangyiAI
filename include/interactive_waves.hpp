#pragma once

#include <nlohmann/json.hpp>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>

namespace gangyi {

inline double waveParameter(const nlohmann::json& value, double low, double high, double step) {
    if (!value.is_number()) throw std::invalid_argument("波动参数须为数值");
    const auto number = value.get<double>();
    if (!std::isfinite(number) || number < low || number > high || std::floor(number / step) != number / step)
        throw std::invalid_argument("波动参数超出当前范围或步长");
    return number;
}

inline nlohmann::json checkedWaveConfig(const nlohmann::json& source) {
    using Json = nlohmann::json;
    if (source.value("type", "") != "wave_lab" || source.value("schemaVersion", Json(1)) != 1)
        throw std::invalid_argument("波动组件类型或版本无效");
    return {{"type", "wave_lab"}, {"schemaVersion", 1},
        {"amplitudeM", waveParameter(source.value("amplitudeM", Json(1)), 0, 2, 0.5)},
        {"waveSpeedMps", waveParameter(source.value("waveSpeedMps", Json(2)), 1, 5, 1)},
        {"periodSeconds", waveParameter(source.value("periodSeconds", Json(2)), 1, 4, 0.5)},
        {"timeSeconds", waveParameter(source.value("timeSeconds", Json(0)), 0, 8, 0.25)},
        {"probeXM", waveParameter(source.value("probeXM", Json(1)), 0, 12, 0.5)}};
}

inline nlohmann::json checkedWaveObservation(const nlohmann::json& source, const nlohmann::json& state, int contentVersion) {
    using Json = nlohmann::json;
    const auto config = checkedWaveConfig(source);
    const std::set<std::string> fields = {"schemaVersion", "contentVersion", "amplitudeM", "waveSpeedMps", "periodSeconds", "timeSeconds", "probeXM"};
    if (contentVersion < 1 || !state.is_object() || state.size() != fields.size() ||
        !state.value("schemaVersion", Json()).is_number_integer() || state.at("schemaVersion") != 1 ||
        !state.value("contentVersion", Json()).is_number_integer() || state.at("contentVersion") != contentVersion)
        throw std::invalid_argument("波动探索状态已变化或缺少参数");
    for (const auto& item : state.items())
        if (!fields.count(item.key())) throw std::invalid_argument("波动状态不能自报计算结果或成绩");
    auto input = config;
    for (const auto& name : {"amplitudeM", "waveSpeedMps", "periodSeconds", "timeSeconds", "probeXM"}) input[name] = state.at(name);
    const auto checked = checkedWaveConfig(input);
    const double amplitude = checked.at("amplitudeM"), speed = checked.at("waveSpeedMps"), period = checked.at("periodSeconds");
    const double time = checked.at("timeSeconds"), position = checked.at("probeXM"), wavelength = speed * period;
    const double phase = 2 * std::acos(-1.0) * (position / wavelength - time / period);
    const auto clean = [](double value) { return std::abs(value) < 1e-12 ? 0.0 : value; };
    Json result = state;
    result.update({{"kind", "exploration"}, {"scoringEligible", false}, {"inputSource", "browser_report"},
        {"resultSource", "program_calculation"}, {"model", "ideal_right_traveling_sinusoidal_transverse_wave"},
        {"wavelengthM", wavelength}, {"frequencyHz", 1 / period}, {"propagationDirection", "positive_x"},
        {"probeDisplacementM", clean(amplitude * std::sin(phase))},
        {"particleVelocityMps", clean(-amplitude * 2 * std::acos(-1.0) / period * std::cos(phase))},
        {"particleFixedXM", position}, {"visibleOscillation", amplitude > 0}});
    return result;
}

}

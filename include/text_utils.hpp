#pragma once

#include <cstddef>
#include <string>

namespace gangyi {

inline std::string truncateUtf8(const std::string& value, size_t maxBytes) {
    if (value.size() <= maxBytes) return value;
    size_t end = 0;
    while (end < value.size()) {
        const unsigned char lead = static_cast<unsigned char>(value[end]);
        size_t width = 1;
        if ((lead & 0xE0u) == 0xC0u) width = 2;
        else if ((lead & 0xF0u) == 0xE0u) width = 3;
        else if ((lead & 0xF8u) == 0xF0u) width = 4;
        if (end + width > maxBytes || end + width > value.size()) break;
        end += width;
    }
    return value.substr(0, end) + "…";
}

}  // namespace gangyi

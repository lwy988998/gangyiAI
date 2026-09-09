#pragma once

#include <cstddef>
#include <string>

namespace gangyi {

inline std::string truncateUtf8(const std::string& value, size_t maxBytes) {
    size_t end = 0;
    while (end < value.size() && end < maxBytes) {
        const unsigned char lead = static_cast<unsigned char>(value[end]);
        size_t width = 0;
        if (lead <= 0x7Fu) width = 1;
        else if (lead >= 0xC2u && lead <= 0xDFu) width = 2;
        else if (lead >= 0xE0u && lead <= 0xEFu) width = 3;
        else if (lead >= 0xF0u && lead <= 0xF4u) width = 4;
        else break;
        if (end + width > maxBytes || end + width > value.size()) break;

        bool valid = true;
        for (size_t index = 1; index < width; ++index) {
            const unsigned char continuation = static_cast<unsigned char>(value[end + index]);
            if ((continuation & 0xC0u) != 0x80u) {
                valid = false;
                break;
            }
        }
        if (!valid) break;

        const unsigned char second = width > 1 ? static_cast<unsigned char>(value[end + 1]) : 0;
        if ((lead == 0xE0u && second < 0xA0u) ||
            (lead == 0xEDu && second > 0x9Fu) ||
            (lead == 0xF0u && second < 0x90u) ||
            (lead == 0xF4u && second > 0x8Fu)) break;
        end += width;
    }
    return value.substr(0, end);
}

}  // namespace gangyi

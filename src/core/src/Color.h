#ifndef SRC_EPOCH_CORE_COLOR_H_
#define SRC_EPOCH_CORE_COLOR_H_

#include <cstdint>

namespace epoch
{
    union Color
    {
        uint32_t rgba;

        Color() : rgba{} {}
        explicit Color(const uint32_t color) : rgba{ color } {}
        Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) : rgba((a << 24) | (b << 16) | (g << 8) | r) {}
    };
}

#endif

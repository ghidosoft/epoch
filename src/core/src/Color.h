#ifndef SRC_EPOCH_CORE_COLOR_H_
#define SRC_EPOCH_CORE_COLOR_H_

#include <cstdint>

namespace epoch
{
    union Color
    {
        struct
        {
            uint8_t r, g, b, a;
        };
        uint32_t color;

        Color() : color{} {}
        explicit Color(const uint32_t color) : color{ color } {}
        Color(const uint8_t r, const uint8_t g, const uint8_t b) : r(r), g(g), b(b), a(255) {}
        Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) : r(r), g(g), b(b), a(a) {}
    };
}

#endif

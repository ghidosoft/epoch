/* This file is part of Epoch, Copyright (C) 2023 Andrea Ghidini.
 *
 * Epoch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Epoch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Epoch.  If not, see <https://www.gnu.org/licenses/>.
 */

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

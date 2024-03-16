/* This file is part of Epoch, Copyright (C) 2024 Andrea Ghidini.
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

#ifndef SRC_EPOCH_ZXSPECTRUM_CONSTANTS_HPP_
#define SRC_EPOCH_ZXSPECTRUM_CONSTANTS_HPP_

#include <array>
#include <cstdint>

namespace epoch::zxspectrum
{
    constexpr std::size_t TStatesPerFrame = 69888;
    constexpr std::size_t CPUClock = 3500000;
    constexpr double FramesPerSecond = 50.08;

    constexpr int ScreenWidth = 256;
    constexpr int ScreenHeight = 192;
    constexpr int BorderLeft = 48;
    constexpr int BorderRight = 48;
    constexpr int BorderTop = 48;
    constexpr int BorderBottom = 56;

    constexpr int VerticalRetrace = 16;        // 16 lines
    constexpr int HorizontalRetrace = 48 * 2;  // Pixels (48 T-states)
    constexpr int InterruptActiveTStates = 32;

    constexpr auto Width = ScreenWidth + BorderLeft + BorderRight;
    constexpr auto Height = ScreenHeight + BorderTop + BorderBottom;

    static_assert((VerticalRetrace + Height) * (Width + HorizontalRetrace) ==
                  static_cast<int>(TStatesPerFrame) * 2);  // total frame length, 2 pixels per T-state

    constexpr auto AudioInThreshold = 0.2f;

    constexpr uint16_t MemoryBankSize = 0x4000;  // 16K

    using MemoryBank = std::array<uint8_t, MemoryBankSize>;
}  // namespace epoch::zxspectrum

#endif

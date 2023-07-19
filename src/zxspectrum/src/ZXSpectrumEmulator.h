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

#ifndef SRC_EPOCH_ZXSPECTRUM_ZXSPECTRUMEMULATOR_H_
#define SRC_EPOCH_ZXSPECTRUM_ZXSPECTRUMEMULATOR_H_

#include <array>
#include <memory>

#include <epoch/core.h>

#include "Constants.h"

namespace epoch::zxspectrum
{
    class Ula;
    class Z80Cpu;

    class ZXSpectrumEmulator : public Emulator
    {
    public:
        static const Palette DefaultPalette;

    public:
        ZXSpectrumEmulator();
        ~ZXSpectrumEmulator() override;

    public:
        ZXSpectrumEmulator(const ZXSpectrumEmulator& other) = delete;
        ZXSpectrumEmulator(ZXSpectrumEmulator&& other) noexcept = delete;
        ZXSpectrumEmulator& operator=(const ZXSpectrumEmulator& other) = delete;
        ZXSpectrumEmulator& operator=(ZXSpectrumEmulator&& other) noexcept = delete;

    public:
        void clock() override;
        void reset() override;

        [[nodiscard]] uint8_t vramRead(uint16_t address) const;

        [[nodiscard]] std::span<const uint32_t> screenBuffer() override;
        
        void keyEvent(Key key, KeyAction action) override;

    private:
        const std::unique_ptr<Ula> m_ula;
        const std::unique_ptr<Z80Cpu> m_cpu;
        uint64_t m_clockCounter{};

        using MemoryBank = std::array<uint8_t, 0x4000>;
        MemoryBank m_rom48k{};
        std::array<MemoryBank, 8> m_ram{};

        std::array<uint32_t, static_cast<std::size_t>(Width* Height)> m_screenBuffer{};
    };
}

#endif

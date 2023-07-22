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

#ifndef SRC_EPOCH_ZXSPECTRUM_ULA_H_
#define SRC_EPOCH_ZXSPECTRUM_ULA_H_

#include <array>
#include <cstdint>
#include <span>

#include "Constants.h"
#include "Z80Interface.h"

namespace epoch::zxspectrum
{
    class Ula final : public Z80Interface
    {
    public:
        using MemoryBank = std::array<uint8_t, 0x4000>;

        Ula(MemoryBank& rom48k, std::array<MemoryBank, 8>& ram);

    public:
        void clock();
        void reset();

        uint8_t read(uint16_t address) override;
        void write(uint16_t address, uint8_t value) override;
        uint8_t ioRead(uint16_t port) override;
        void ioWrite(uint16_t port, uint8_t value) override;
        
        [[nodiscard]] bool isCpuStalled() const { return m_cpuStalled > 0; }

        [[nodiscard]] std::span<const uint8_t> borderBuffer() const { return m_borderBuffer; }
        [[nodiscard]] bool invertPaperInk() const { return m_frameCounter & 0x10; } // 16 frames

        [[nodiscard]] bool interruptRequested() const { return m_y == -VerticalRetrace && m_x < -HorizontalRetrace + 64; } // 32 t-states
        [[nodiscard]] bool frameReady() const { return m_y == -VerticalRetrace && m_x == -HorizontalRetrace; }

        [[nodiscard]] bool audioOutput() const { return m_ear; }
        [[nodiscard]] uint8_t border() const { return m_border; }

        void setKeyState(int row, int col, bool state);

    private:
        MemoryBank& m_rom48k;
        std::array<MemoryBank, 8>& m_ram;

        uint8_t m_floatingBusValue{};
        uint8_t m_border{};
        std::array<uint8_t, 8> m_keyboardState{}; // Rows 0=Caps, A, Q, 1, 6, Y, H, 7=B
        bool m_ear{};
        int m_cpuStalled{};

        uint64_t m_frameCounter{};
        std::array<uint8_t, static_cast<std::size_t>(Width* Height)> m_borderBuffer{};
        int m_x{ -HorizontalRetrace }, m_y{ -VerticalRetrace };
        bool m_frameReady{};
    };
}

#endif

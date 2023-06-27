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

#ifndef SRC_EPOCH_ZXSPECTRUM_Z80CPU_H_
#define SRC_EPOCH_ZXSPECTRUM_Z80CPU_H_

#include <cstdint>

namespace epoch::zxspectrum
{
    struct Z80Registers
    {
        struct WordRegister
        {
            uint16_t value;
            [[nodiscard]] uint8_t low() const { return value & 0xff; }
            [[nodiscard]] uint8_t high() const { return (value >> 8) & 0xff; }
            void low(const uint8_t v) { value = (value & 0xff00) | v; }
            void high(const uint8_t v) { value = (value & 0x00ff) | static_cast<uint16_t>(v << 8); }
        };

        struct WordFlagsRegister : WordRegister
        {
            [[nodiscard]] bool s() const { return value & 0b10000000; }
            [[nodiscard]] bool z() const { return value & 0b01000000; }
            [[nodiscard]] bool y() const { return value & 0b00100000; }
            [[nodiscard]] bool h() const { return value & 0b00010000; }
            [[nodiscard]] bool x() const { return value & 0b00001000; }
            [[nodiscard]] bool p() const { return value & 0b00000100; }
            [[nodiscard]] bool n() const { return value & 0b00000010; }
            [[nodiscard]] bool c() const { return value & 0b00000001; }

            void s(const bool f) { value = (value & ~0b10000000) | static_cast<uint16_t>(f << 7); }
            void z(const bool f) { value = (value & ~0b01000000) | static_cast<uint16_t>(f << 6); }
            void y(const bool f) { value = (value & ~0b00100000) | static_cast<uint16_t>(f << 5); }
            void h(const bool f) { value = (value & ~0b00010000) | static_cast<uint16_t>(f << 4); }
            void x(const bool f) { value = (value & ~0b00001000) | static_cast<uint16_t>(f << 3); }
            void p(const bool f) { value = (value & ~0b00000100) | static_cast<uint16_t>(f << 2); }
            void n(const bool f) { value = (value & ~0b00000010) | static_cast<uint16_t>(f << 1); }
            void c(const bool f) { value = (value & ~0b00000001) | static_cast<uint16_t>(f << 0); }
        };

        // PC Program counter
        uint16_t pc{0x0000};
        // SP Stack pointer
        uint16_t sp{0xffff};
        // IX Index X
        uint16_t ix{0xffff};
        // IY Index Y
        uint16_t iy{0xffff};
        // IR Interrupt/Refresh register
        WordRegister ir{0x00ff};
        // AF Accumulator and flags
        WordFlagsRegister af{0xffff};
        // BC General purpose
        WordRegister bc{0xffff};
        // DE General purpose
        WordRegister de{0xffff};
        // HL General purpose
        WordRegister hl{0xffff};
        // AF shadow
        WordFlagsRegister af2{0xffff};
        // BC shadow
        WordRegister bc2{0xfff};
        // DE shadow
        WordRegister de2{0xffff};
        // HL shadow
        WordRegister hl2{0xffff};
    };

    class Z80Cpu final
    {
    public:
        Z80Cpu();

    public:
        void clock();
        void reset();

        Z80Registers& registers();

    private:
        Z80Registers m_registers{};
    };
}

#endif

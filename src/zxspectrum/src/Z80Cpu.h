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
        union WordRegister
        {
            uint16_t value;
            struct
            {
                // TODO: endianness
                uint8_t low;
                uint8_t high;
            };
        };

        union Flags
        {
            struct
            {
                // TODO: endianness
                bool c : 1;
                bool n : 1;
                bool p : 1;
                bool x : 1;
                bool h : 1;
                bool y : 1;
                bool z : 1;
                bool s : 1;
            };
            uint8_t value;
        };

        union WordFlagsRegister
        {
            uint16_t value;
            struct
            {
                // TODO: endianness
                uint8_t low;
                uint8_t high;
            };
            Flags flags;
        };

        // PC Program counter
        uint16_t pc{0x0000};
        // SP Stack pointer
        uint16_t sp{0xffff};
        // IX Index X
        WordRegister ix{0xffff};
        // IY Index Y
        WordRegister iy{0xffff};
        // IR Interrupt/Refresh register
        WordRegister ir{0x00ff};
        // AF Accumulator and flags
        WordFlagsRegister af{0xffff};
        // BC generale purpose
        WordRegister bc{0xffff};
        // DE generale purpose
        WordRegister de{0xffff};
        // HL generale purpose
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

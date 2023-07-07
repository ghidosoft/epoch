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

#include <array>
#include <cstdint>
#include <istream>
#include <vector>

#include "Z80Interface.h"

namespace epoch::zxspectrum
{
    struct Z80Registers
    {
        struct WordRegister
        {
            WordRegister() = default;
            WordRegister(const uint16_t value) : value(value) {}
            uint16_t value{};
            [[nodiscard]] uint8_t low() const { return value & 0xff; }
            [[nodiscard]] uint8_t high() const { return (value >> 8) & 0xff; }
            void low(const uint8_t v) { value = (value & 0xff00) | v; }
            void high(const uint8_t v) { value = (value & 0x00ff) | static_cast<uint16_t>(v << 8); }

            operator uint16_t() const { return value; }
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
        uint16_t pc{};
        // SP Stack pointer
        uint16_t sp{};
        // IX Index X
        uint16_t ix{};
        // IY Index Y
        uint16_t iy{};
        // IR Interrupt/Refresh register
        WordRegister ir{};
        // AF Accumulator and flags
        WordFlagsRegister af{};
        // BC General purpose
        WordRegister bc{};
        // DE General purpose
        WordRegister de{};
        // HL General purpose
        WordRegister hl{};
        // AF shadow
        WordFlagsRegister af2{};
        // BC shadow
        WordRegister bc2{};
        // DE shadow
        WordRegister de2{};
        // HL shadow
        WordRegister hl2{};
    };

    enum class Z80MachineCycle
    {
        opcode,
        memRead,
        memWrite,
        ioRead,
        ioWrite,
        intAck,
        extra,
    };

    enum class Z80InstructionType
    {
        custom,
        LD,
    };

    enum class Z80Operand
    {
        none,

        af,
        bc,
        de,
        hl,
        memDe,
        n8,

        a,
        b,
        c,
        d,
        e,
        f,
        n16,
    };

    struct Z80Instruction
    {
        std::string mnemonic;
        std::vector<Z80MachineCycle> machineCycles;
        Z80InstructionType type;
        Z80Operand op1;
        Z80Operand op2;
        int totalCycles;
    };

    class Z80Cpu final
    {
    public:
        explicit Z80Cpu(Z80Interface& bus);

    public:
        void clock();
        void reset();

        Z80Registers& registers();
        [[nodiscard]] const Z80Registers& registers() const;

    private:
        Z80Registers m_registers{};
        uint8_t m_opcode{};
        int m_remainingCycles{};

        std::array<Z80Instruction, 256> m_instructions{};

        Z80Interface& m_bus;

        bool m_halted{};

        uint8_t fetchOpcode();
    };
}

#endif

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
    struct Z80Flags
    {
        enum Values : uint8_t
        {
            c = 1 << 0,
            n = 1 << 1,
            p = 1 << 2,
            v = p,
            x = 1 << 3,
            h = 1 << 4,
            y = 1 << 5,
            z = 1 << 6,
            s = 1 << 7,
        };
    };

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
            [[nodiscard]] bool s() const { return value & Z80Flags::s; }
            [[nodiscard]] bool z() const { return value & Z80Flags::z; }
            [[nodiscard]] bool y() const { return value & Z80Flags::y; }
            [[nodiscard]] bool h() const { return value & Z80Flags::h; }
            [[nodiscard]] bool x() const { return value & Z80Flags::x; }
            [[nodiscard]] bool p() const { return value & Z80Flags::p; }
            [[nodiscard]] bool n() const { return value & Z80Flags::n; }
            [[nodiscard]] bool c() const { return value & Z80Flags::c; }

            void s(const bool f) { value = (value & ~Z80Flags::s) | static_cast<uint16_t>(f << 7); }
            void z(const bool f) { value = (value & ~Z80Flags::z) | static_cast<uint16_t>(f << 6); }
            void y(const bool f) { value = (value & ~Z80Flags::y) | static_cast<uint16_t>(f << 5); }
            void h(const bool f) { value = (value & ~Z80Flags::h) | static_cast<uint16_t>(f << 4); }
            void x(const bool f) { value = (value & ~Z80Flags::x) | static_cast<uint16_t>(f << 3); }
            void p(const bool f) { value = (value & ~Z80Flags::p) | static_cast<uint16_t>(f << 2); }
            void n(const bool f) { value = (value & ~Z80Flags::n) | static_cast<uint16_t>(f << 1); }
            void c(const bool f) { value = (value & ~Z80Flags::c) | static_cast<uint16_t>(f << 0); }
        };

        // PC Program counter
        uint16_t pc{};
        // SP Stack pointer
        uint16_t sp{ 0xffff };
        // IX Index X
        uint16_t ix{ 0xffff };
        // IY Index Y
        uint16_t iy{ 0xffff };
        // IR Interrupt/Refresh register
        WordRegister ir{};
        // AF Accumulator and flags
        WordFlagsRegister af{ 0xffff };
        // BC General purpose
        WordRegister bc{ 0xffff };
        // DE General purpose
        WordRegister de{ 0xffff };
        // HL General purpose
        WordRegister hl{ 0xffff };
        // AF shadow
        WordFlagsRegister af2{ 0xffff };
        // BC shadow
        WordRegister bc2{ 0xffff };
        // DE shadow
        WordRegister de2{ 0xffff };
        // HL shadow
        WordRegister hl2{ 0xffff };
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
        void step();
        void reset();

        Z80Registers& registers();
        [[nodiscard]] const Z80Registers& registers() const;

    private:
        Z80Registers m_registers{};
        uint8_t m_opcode{};
        int m_remainingCycles{};

        std::array<Z80Instruction, 256> m_instructions{};

        Z80Interface& m_bus;

        std::array<uint8_t*, 8> m_registersPointers;

        uint8_t fetchOpcode();
        uint8_t busRead(uint16_t address);
        void busWrite(uint16_t address, uint8_t value);
        uint8_t ioRead(uint8_t port);
        void ioWrite(uint8_t port, uint8_t value);

        void mainQuadrant0();
        void mainQuadrant1();
        void mainQuadrant2();
        void mainQuadrant3();
        uint16_t read16();
        uint8_t add8(uint8_t a, uint8_t b, uint8_t carryFlag);
        uint8_t sub8(uint8_t a, uint8_t b, uint8_t carryFlag);
    };
}

#endif

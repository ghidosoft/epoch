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

#ifndef SRC_EPOCH_ZXSPECTRUM_Z80CPU_HPP_
#define SRC_EPOCH_ZXSPECTRUM_Z80CPU_HPP_

#include <array>
#include <cstdint>

#include "Z80Interface.hpp"

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
            WordRegister(const uint16_t v) : low{ static_cast<uint8_t>(v & 0xff) }, high{ static_cast<uint8_t>(v >> 8) } {}
            uint8_t low{};
            uint8_t high{};
            [[nodiscard]] uint16_t value() const { return static_cast<uint16_t>(low) | static_cast<uint16_t>(high << 8); }

            operator uint16_t() const { return value(); }
        };

        struct WordFlagsRegister : WordRegister
        {
            WordFlagsRegister() = default;
            WordFlagsRegister(const uint16_t v) : WordRegister(v) {}

            [[nodiscard]] bool s() const { return low & Z80Flags::s; }
            [[nodiscard]] bool z() const { return low & Z80Flags::z; }
            [[nodiscard]] bool y() const { return low & Z80Flags::y; }
            [[nodiscard]] bool h() const { return low & Z80Flags::h; }
            [[nodiscard]] bool x() const { return low & Z80Flags::x; }
            [[nodiscard]] bool p() const { return low & Z80Flags::p; }
            [[nodiscard]] bool n() const { return low & Z80Flags::n; }
            [[nodiscard]] bool c() const { return low & Z80Flags::c; }

            void s(const bool f) { low = (low & ~Z80Flags::s) | static_cast<uint8_t>(f << 7); }
            void z(const bool f) { low = (low & ~Z80Flags::z) | static_cast<uint8_t>(f << 6); }
            void y(const bool f) { low = (low & ~Z80Flags::y) | static_cast<uint8_t>(f << 5); }
            void h(const bool f) { low = (low & ~Z80Flags::h) | static_cast<uint8_t>(f << 4); }
            void x(const bool f) { low = (low & ~Z80Flags::x) | static_cast<uint8_t>(f << 3); }
            void p(const bool f) { low = (low & ~Z80Flags::p) | static_cast<uint8_t>(f << 2); }
            void n(const bool f) { low = (low & ~Z80Flags::n) | static_cast<uint8_t>(f << 1); }
            void c(const bool f) { low = (low & ~Z80Flags::c) | static_cast<uint8_t>(f << 0); }
        };

        // PC Program counter
        uint16_t pc{};
        // SP Stack pointer
        uint16_t sp{ 0xffff };
        // IX Index X
        WordRegister ix{ 0xffff };
        // IY Index Y
        WordRegister iy{ 0xffff };
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

        // WZ (or MEMPTR) internal register
        uint16_t wz{ 0x0000 };

        uint8_t interruptMode{ 0 };
        bool iff1{ false };
        bool iff2{ false };
        bool interruptJustEnabled{ false };
    };

    enum class Z80OpcodePrefix
    {
        none = 0,
        ix = 1,
        iy = 2,
    };

    class Z80Cpu final
    {
    public:
        explicit Z80Cpu(Z80Interface& bus);

    public:
        void clock();
        void step();
        void reset();

        void interruptRequest(bool requested);

        [[nodiscard]] Z80Registers& registers() { return m_registers; }
        [[nodiscard]] const Z80Registers& registers() const { return m_registers; }

        [[nodiscard]] std::size_t clockCounter() const { return m_clockCounter; }

    private:
        Z80Registers m_registers{};
        uint8_t m_opcode{};
        Z80OpcodePrefix m_currentPrefix{ Z80OpcodePrefix::none };
        bool m_interruptRequested{};
        int m_remainingCycles{};
        std::size_t m_clockCounter{};

        Z80Interface& m_bus;

        std::array<std::array<uint8_t*, 8>, 3> m_registersPointers;

        void executeInstruction();
        void handleInterrupt();

        uint8_t fetchOpcode();
        uint8_t busRead(uint16_t address);
        void busWrite(uint16_t address, uint8_t value);
        uint8_t ioRead(uint16_t port);
        void ioWrite(uint16_t port, uint8_t value);

        void mainQuadrant0();
        void mainQuadrant1();
        void mainQuadrant2();
        void mainQuadrant3();

        void prefixCb();
        void prefixDd();
        void prefixEd();
        void prefixFd();

        [[nodiscard]] uint16_t getHL() const;
        void setHL(uint16_t value);

        uint8_t busReadHL();
        void busWriteHL(uint8_t value);
        uint16_t fetch16();
        uint16_t read16(uint16_t address);
        void write16(uint16_t address, uint16_t value);
        uint8_t add8(uint8_t a, uint8_t b, bool carryFlag = false);
        uint8_t sub8(uint8_t a, uint8_t b, bool carryFlag = false);
        uint16_t add16(uint16_t a, uint16_t b);
        uint16_t add16(uint16_t a, uint16_t b, bool carryFlag);
        uint16_t sub16(uint16_t a, uint16_t b);
        uint16_t sub16(uint16_t a, uint16_t b, bool carryFlag);
        void alu8(int operation, uint8_t a, uint8_t b);
        [[nodiscard]] bool evaluateCondition(int condition) const;
        void jr(bool condition);
        void push16(uint16_t value);
        uint16_t pop16();
        void ldi();
        void ldd();
        void cpi();
        void cpd();
        void ini();
        void ind();
        void outi();
        void outd();
        uint8_t prefixCbRead(int8_t d, int z);
        void prefixCbWrite(int8_t d, int z, uint8_t value);
    };
}

#endif

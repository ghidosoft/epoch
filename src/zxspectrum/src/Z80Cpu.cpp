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

#include "Z80Cpu.h"

#include <bit>
#include <cassert>
#include <optional>
#include <sstream>

#include "Z80Config.inc.h"

namespace epoch::zxspectrum
{
    static std::optional<Z80MachineCycle> parseMachineCycle(const std::string& s)
    {
        if (s == "op") return Z80MachineCycle::opcode;
        if (s == "mr") return Z80MachineCycle::memRead;
        if (s == "mw") return Z80MachineCycle::memRead;
        if (s == "ior") return Z80MachineCycle::ioRead;
        if (s == "iow") return Z80MachineCycle::ioWrite;
        if (s == "-") return {};
        throw std::runtime_error("Invalid machine cycle");
    }

    static Z80InstructionType parseInstructionType(const std::string& s)
    {
        if (s == "LD") return Z80InstructionType::LD;
        return Z80InstructionType::custom;
    }

    static Z80Operand parseOperand(const std::string& s)
    {
        if (s == "AF") return Z80Operand::af;
        if (s == "BC") return Z80Operand::bc;
        if (s == "DE") return Z80Operand::de;
        if (s == "HL") return Z80Operand::hl;
        if (s == "A") return Z80Operand::a;
        if (s == "B") return Z80Operand::b;
        if (s == "C") return Z80Operand::c;
        if (s == "D") return Z80Operand::d;
        if (s == "E") return Z80Operand::e;
        if (s == "F") return Z80Operand::f;
        if (s == "n") return Z80Operand::n8;
        if (s == "nn") return Z80Operand::n16;
        if (s == "(DE)") return Z80Operand::memDe;
        return Z80Operand::none;
    }

    static const uint8_t s_flagsLookupSZP[256] = {
          0x44,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
          0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
          0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
          0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
          0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
          0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
          0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
          0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
          0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
          0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
          0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
          0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
          0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
          0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
          0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
          0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
    };

    Z80Cpu::Z80Cpu(Z80Interface& bus) :
        m_bus{ bus },
        m_registersPointers{ // TODO: endianness
            reinterpret_cast<uint8_t*>(&m_registers.bc.value) + 1,
            reinterpret_cast<uint8_t*>(&m_registers.bc.value),
            reinterpret_cast<uint8_t*>(&m_registers.de.value) + 1,
            reinterpret_cast<uint8_t*>(&m_registers.de.value),
            reinterpret_cast<uint8_t*>(&m_registers.hl.value) + 1,
            reinterpret_cast<uint8_t*>(&m_registers.hl.value),
            nullptr, // (HL)
            reinterpret_cast<uint8_t*>(&m_registers.af.value) + 1,
        }
    {
        std::istringstream is{ Z80CONFIG };

        std::string line;
        while (std::getline(is, line))
        {
            if (!line.empty() && line[0] != '#')
            {
                std::istringstream is{ line };
                uint32_t op;
                std::vector<Z80MachineCycle> machineCycles{};
                std::string mnemonic, op1, op2;
                is >> std::hex >> op;
                for (auto i = 0; i < 6; i++)
                {
                    std::string m;
                    is >> m;
                    if (const auto mc = parseMachineCycle(m); mc.has_value())
                    {
                        machineCycles.push_back(mc.value());
                    }
                }
                is >> mnemonic;
                is >> op1;
                is >> op2;
                // TODO: parse config and generate instructions
                // TODO: support for 2/3-bytes opcodes
                assert(op <= 0xff);
                m_instructions[op] = {
                    .mnemonic = mnemonic,
                    .machineCycles = machineCycles,
                    .type = parseInstructionType(mnemonic),
                    .op1 = parseOperand(op1),
                    .op2 = parseOperand(op2),
                };
            }
        }

        reset();
    }

    void Z80Cpu::clock()
    {
        if (m_remainingCycles == 0)
        {
            /* if (m_registers.pc == 0xe84)
            {
                assert(m_registers.pc);
            } */
            executeInstruction();
        }

        m_clockCounter++;
        m_remainingCycles--;
    }

    void Z80Cpu::step()
    {
        do
        {
            clock();
        } while (m_remainingCycles > 0);
    }

    void Z80Cpu::reset()
    {
        m_registers = {};
        m_currentPrefix = Z80OpcodePrefix::none;
        m_remainingCycles = {};
        m_clockCounter = {};
    }

    void Z80Cpu::executeInstruction()
    {
        m_opcode = fetchOpcode();

        const auto quadrant = m_opcode >> 6;

        if (quadrant == 0)
        {
            mainQuadrant0();
        }
        else if (quadrant == 1)
        {
            // LD 8bit / HALT
            mainQuadrant1();
        }
        else if (quadrant == 2)
        {
            // ALU operations
            mainQuadrant2();
        }
        else
        {
            mainQuadrant3();
        }

        if (m_opcode != 0xfb) m_registers.interruptJustEnabled = false;

        m_currentPrefix = Z80OpcodePrefix::none;
    }

    uint8_t Z80Cpu::fetchOpcode()
    {
        m_registers.ir.low((m_registers.ir.low() + 1) & 0b01111111);
        m_remainingCycles += 4;
        return m_bus.read(m_registers.pc++);
    }

    uint8_t Z80Cpu::busRead(const uint16_t address)
    {
        m_remainingCycles += 3;
        return m_bus.read(address);
    }

    void Z80Cpu::busWrite(const uint16_t address, const uint8_t value)
    {
        m_remainingCycles += 3;
        m_bus.write(address, value);
    }

    uint8_t Z80Cpu::ioRead(const uint16_t port)
    {
        m_remainingCycles += 4;
        return m_bus.ioRead(port);
    }

    void Z80Cpu::ioWrite(const uint16_t port, const uint8_t value)
    {
        m_remainingCycles += 4;
        m_bus.ioWrite(port, value);
    }

    void Z80Cpu::mainQuadrant0()
    {
        const auto y = (m_opcode & 0b00111000) >> 3;
        const auto z = (m_opcode & 0b00000111);
        if (z == 0b000)
        {
            switch (y)
            {
            case 0b000:
                // NOP
                break;
            case 0b001:
                // EX AF, AF'
                std::swap(m_registers.af.value, m_registers.af2.value);
                break;
            case 0b010:
                // DJNZ d
                {
                    const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                    m_remainingCycles++;
                    const uint8_t b = m_registers.bc.high() - 1;
                    m_registers.bc.high(b);
                    if (b != 0)
                    {
                        m_remainingCycles += 5;
                        m_registers.pc += d;
                    }
                }
                break;
            case 0b011:
                // JR d
                jr(true);
                break;
            case 0b100:
                // JR NZ, d
                jr(m_registers.af.z() == false);
                break;
            case 0b101:
                // JR Z, d
                jr(m_registers.af.z() == true);
                break;
            case 0b110:
                // JR NC, d
                jr(m_registers.af.c() == false);
                break;
            case 0b111:
                // JR C, d
                jr(m_registers.af.c() == true);
                break;
            }
        }
        else if (z == 0b001)
        {
            switch (y)
            {
            case 0b000:
                // LD BC, nn
                m_registers.bc = fetch16();
                break;
            case 0b001:
                // ADD HL, BC
                m_remainingCycles += 7;
                setHL(add16(getHL(), m_registers.bc));
                break;
            case 0b010:
                // LD DE, nn
                m_registers.de = fetch16();
                break;
            case 0b011:
                // ADD HL, DE
                m_remainingCycles += 7;
                setHL(add16(getHL(), m_registers.de));
                break;
            case 0b100:
                // LD HL, nn
                setHL(fetch16());
                break;
            case 0b101:
                // ADD HL, HL
                m_remainingCycles += 7;
                setHL(add16(getHL(), getHL()));
                break;
            case 0b110:
                // LD SP, nn
                m_registers.sp = fetch16();
                break;
            case 0b111:
                // ADD HL, SP
                m_remainingCycles += 7;
                setHL(add16(getHL(), m_registers.sp));
                break;
            }
        }
        else if (z == 0b010)
        {
            switch (y)
            {
            case 0b000:
                // LD (BC), A
                busWrite(m_registers.bc, m_registers.af.high());
                break;
            case 0b001:
                // LD A, (BC)
                m_registers.af.high(busRead(m_registers.bc));
                break;
            case 0b010:
                // LD (DE), A
                busWrite(m_registers.de, m_registers.af.high());
                break;
            case 0b011:
                // LD A, (DE)
                m_registers.af.high(busRead(m_registers.de));
                break;
            case 0b100:
                // LD (nn), HL
                write16(fetch16(), getHL());
                break;
            case 0b101:
                // LD HL, (nn)
                setHL(read16(fetch16()));
                break;
            case 0b110:
                // LD (nn), A
                busWrite(fetch16(), m_registers.af.high());
                break;
            case 0b111:
                // LD A, (nn)
                m_registers.af.high(busRead(fetch16()));
                break;
            }
        }
        else if (z == 0b011)
        {
            switch (y)
            {
            case 0b000:
                // INC BC
                m_registers.bc.value++;
                break;
            case 0b001:
                // DEC BC
                m_registers.bc.value--;
                break;
            case 0b010:
                // INC DE
                m_registers.de.value++;
                break;
            case 0b011:
                // DEC BC
                m_registers.de.value--;
                break;
            case 0b100:
                // INC HL
                setHL(getHL() + 1);
                break;
            case 0b101:
                // DEC HL
                setHL(getHL() - 1);
                break;
            case 0b110:
                // INC SP
                m_registers.sp++;
                break;
            case 0b111:
                // DEC SP
                m_registers.sp--;
                break;
            }
            m_remainingCycles += 2;
        }
        else if (z == 0b100)
        {
            // INC 8bit
            const auto c = m_registers.af.c();
            uint8_t n;
            if (y == 0b110)
            {
                // INC (HL)
                uint8_t n;
                int8_t d;
                switch (m_currentPrefix)
                {
                case Z80OpcodePrefix::none:
                    n = busRead(m_registers.hl);
                    m_remainingCycles++;
                    busWrite(m_registers.hl, n + 1);
                    break;
                case Z80OpcodePrefix::ix:
                    d = static_cast<int8_t>(busRead(m_registers.pc++));
                    m_remainingCycles += 5;
                    n = busRead(m_registers.ix + d);
                    busWrite(m_registers.ix + d, n + 1);
                    break;
                case Z80OpcodePrefix::iy:
                    d = static_cast<int8_t>(busRead(m_registers.pc++));
                    m_remainingCycles += 5;
                    n = busRead(m_registers.iy + d);
                    busWrite(m_registers.iy + d, n + 1);
                    break;
                }
                add8(n, 1);
            }
            else
            {
                n = (*m_registersPointers[y])++;
                add8(n, 1);
            }
            m_registers.af.c(c); // restore carry
        }
        else if (z == 0b101)
        {
            // DEC 8bit
            const auto c = m_registers.af.c();
            uint8_t n;
            if (y == 0b110)
            {
                // DEC (HL)
                uint8_t n;
                int8_t d;
                switch (m_currentPrefix)
                {
                case Z80OpcodePrefix::none:
                    n = busRead(m_registers.hl);
                    m_remainingCycles++;
                    busWrite(m_registers.hl, n - 1);
                    break;
                case Z80OpcodePrefix::ix:
                    d = static_cast<int8_t>(busRead(m_registers.pc++));
                    m_remainingCycles += 5;
                    n = busRead(m_registers.ix + d);
                    busWrite(m_registers.ix + d, n - 1);
                    break;
                case Z80OpcodePrefix::iy:
                    d = static_cast<int8_t>(busRead(m_registers.pc++));
                    m_remainingCycles += 5;
                    n = busRead(m_registers.iy + d);
                    busWrite(m_registers.iy + d, n - 1);
                    break;
                }
                sub8(n, 1);
            }
            else
            {
                n = (*m_registersPointers[y])--;
                sub8(n, 1);
            }
            m_registers.af.c(c); // restore carry
        }
        else if (z == 0b110)
        {
            // LD 8bit
            if (y == 0b110)
            {
                // LD (HL), n
                switch (m_currentPrefix)
                {
                case Z80OpcodePrefix::none:
                    busWrite(m_registers.hl, busRead(m_registers.pc++));
                    // m_remainingCycles++;
                    break;
                case Z80OpcodePrefix::ix:
                    {
                        const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                        const auto value = busRead(m_registers.pc++);
                        // m_remainingCycles += 5;
                        busWrite(static_cast<uint16_t>(m_registers.ix + d), value);
                        break;
                    }
                case Z80OpcodePrefix::iy:
                    {
                        const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                        const auto value = busRead(m_registers.pc++);
                        // m_remainingCycles += 5;
                        busWrite(static_cast<uint16_t>(m_registers.iy + d), value);
                        break;
                    }
                }
            }
            else
            {
                *m_registersPointers[y] = busRead(m_registers.pc++);
            }
        }
        else // if (z == 0b111)
        {
            switch (y)
            {
            case 0b000:
                // RLCA
                m_registers.af.high(std::rotl(m_registers.af.high(), 1));
                m_registers.af.c(m_registers.af & 0x0100);
                m_registers.af.h(false);
                m_registers.af.n(false);
                break;
            case 0b001:
                // RRCA
                m_registers.af.high(std::rotr(m_registers.af.high(), 1));
                m_registers.af.c(m_registers.af & 0x8000);
                m_registers.af.h(false);
                m_registers.af.n(false);
                break;
            case 0b010:
                // RLA
                {
                    const uint8_t carry = m_registers.af.c() ? 1 : 0;
                    m_registers.af.c(m_registers.af & 0x8000);
                    m_registers.af.high(static_cast<uint8_t>(m_registers.af.high() << 1) | carry);
                    m_registers.af.h(false);
                    m_registers.af.n(false);
                }
                break;
            case 0b011:
                // RRA
                {
                    const uint8_t carry = m_registers.af.c() ? 0x80 : 0x00;
                    m_registers.af.c(m_registers.af & 0x0100);
                    m_registers.af.high(static_cast<uint8_t>(m_registers.af.high() >> 1) | carry);
                    m_registers.af.h(false);
                    m_registers.af.n(false);
                }
                break;
            case 0b100:
                // DAA
                {
                    auto a = m_registers.af.high();
                    if ((a & 0x0f) > 9 || m_registers.af.h())
                    {
                        a += 0x06;
                    }
                    if ((a >> 4) > 9 || m_registers.af.c())
                    {
                        a += 0x60;
                        m_registers.af.c(true);
                    }
                    else
                    {
                        m_registers.af.c(false);
                    }
                    m_registers.af.low(s_flagsLookupSZP[a] | (m_registers.af.low() & 0b00111011));
                    m_registers.af.high(a);
                    // TODO: n = 1
                }
                break;
            case 0b101:
                // CPL
                m_registers.af.high(~m_registers.af.high());
                m_registers.af.h(true);
                m_registers.af.n(true);
                break;
            case 0b110:
                // SCF
                m_registers.af.c(true);
                m_registers.af.h(false);
                m_registers.af.n(false);
                break;
            case 0b111:
                // CCF
                m_registers.af.c(!m_registers.af.c());
                m_registers.af.h(!m_registers.af.h());
                m_registers.af.n(false);
                break;
            }
        }
    }

    void Z80Cpu::mainQuadrant1()
    {
        const auto dst = (m_opcode & 0b00111000) >> 3;
        const auto src = (m_opcode & 0b00000111);
        const uint8_t* srcPtr = m_registersPointers[src];
        uint8_t* dstPtr = m_registersPointers[dst];
        if (src == 0b110)
        {
            if (dst == 0b110)
            {
                // HALT
                m_registers.pc--;
            }
            else
            {
                // LD dst, (HL)
                *dstPtr = busReadHL();
            }
        }
        else
        {
            if (dst == 0b110)
            {
                // LD (HL), src
                busWriteHL(*srcPtr);
            }
            else
            {
                // LD dst, src
                *dstPtr = *srcPtr;
            }
        }
    }

    void Z80Cpu::mainQuadrant2()
    {
        const auto operation = (m_opcode & 0b00111000) >> 3;
        const auto src = (m_opcode & 0b00000111);
        const uint8_t* srcPtr = m_registersPointers[src];
        const auto a = m_registers.af.high();
        uint8_t b;
        if (src == 0b110)
        {
            b = busReadHL();
        }
        else
        {
            b = *srcPtr;
        }
        alu8(operation, a, b);
    }

    void Z80Cpu::mainQuadrant3()
    {
        const uint8_t y = (m_opcode & 0b00111000) >> 3;
        const auto z = (m_opcode & 0b00000111);
        if (z == 0b000)
        {
            // RET [cond]
            m_remainingCycles++;
            if (evaluateCondition(y))
            {
                m_registers.pc = pop16();
            }
        }
        else if (z == 0b001)
        {
            switch (y)
            {
            case 0b000:
                // POP BC
                m_registers.bc = pop16();
                break;
            case 0b001:
                // RET
                m_registers.pc = pop16();
                break;
            case 0b010:
                // POP DE
                m_registers.de = pop16();
                break;
            case 0b011:
                // EXX
                std::swap(m_registers.bc.value, m_registers.bc2.value);
                std::swap(m_registers.de.value, m_registers.de2.value);
                std::swap(m_registers.hl.value, m_registers.hl2.value);
                break;
            case 0b100:
                // POP HL
                setHL(pop16());
                break;
            case 0b101:
                // JP HL
                m_registers.pc = getHL();
                break;
            case 0b110:
                // POP AF
                m_registers.af = pop16();
                break;
            case 0b111:
                // LD SP, HL
                m_registers.sp = getHL();
                m_remainingCycles++;
                break;
            }
        }
        else if (z == 0b010)
        {
            // JP[cond] nn
            const auto nn = fetch16();
            if (evaluateCondition(y))
            {
                m_registers.pc = nn;
            }
        }
        else if (z == 0b011)
        {
            switch (y)
            {
            case 0b000:
                // JP nn
                m_registers.pc = fetch16();
                break;
            case 0b001:
                // CB prefix
                prefixCb();
                break;
            case 0b010:
                // OUT (n), A
                ioWrite(busRead(m_registers.pc++), m_registers.af.high());
                break;
            case 0b011:
                // IN A, (n)
                m_registers.af.high(ioRead(busRead(m_registers.pc++)));
                break;
            case 0b100:
                // EX (SP), HL
                {
                    const auto low = busRead(m_registers.sp);
                    m_remainingCycles++;
                    const auto high = busRead(m_registers.sp + 1);
                    const auto value = getHL();
                    busWrite(m_registers.sp, value & 0xff);
                    busWrite(m_registers.sp + 1, value >> 8);
                    m_remainingCycles += 2;
                    setHL(static_cast<uint16_t>(high << 8) | low);
                }
                break;
            case 0b101:
                // EX DE, HL
                std::swap(m_registers.de.value, m_registers.hl.value);
                break;
            case 0b110:
                // DI
                m_registers.iff1 = m_registers.iff2 = false;
                break;
            case 0b111:
                // EI
                m_registers.iff1 = m_registers.iff2 = true;
                m_registers.interruptJustEnabled = true;
                break;
            }
        }
        else if (z == 0b100)
        {
            // CALL [cond], nn
            const auto nn = fetch16();
            if (evaluateCondition(y))
            {
                m_remainingCycles++;
                push16(m_registers.pc);
                m_registers.pc = nn;
            }
        }
        else if (z == 0b101)
        {
            switch (y)
            {
            case 0b000:
                // PUSH BC
                push16(m_registers.bc);
                m_remainingCycles++;
                break;
            case 0b001:
                // CALL nn
                {
                    const auto nn = fetch16();
                    m_remainingCycles++;
                    push16(m_registers.pc);
                    m_registers.pc = nn;
                }
                break;
            case 0b010:
                // PUSH DE
                push16(m_registers.de);
                m_remainingCycles++;
                break;
            case 0b011:
                // DD prefix
                prefixDd();
                break;
            case 0b100:
                // PUSH HL
                push16(getHL());
                m_remainingCycles++;
                break;
            case 0b101:
                // ED prefix
                prefixEd();
                break;
            case 0b110:
                // PUSH AF
                push16(m_registers.af);
                m_remainingCycles++;
                break;
            case 0b111:
                // FD prefix
                prefixFd();
                break;
            }
        }
        else if (z == 0b110)
        {
            // ALU immediate
            const auto a = m_registers.af.high();
            const auto b = busRead(m_registers.pc++);
            alu8(y, a, b);
        }
        else // if (z == 0b111)
        {
            // RST xx
            m_remainingCycles++;
            push16(m_registers.pc);
            static constexpr uint16_t targets[] = {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38};
            m_registers.pc = targets[y];
        }
    }

    void Z80Cpu::prefixCb()
    {
        int8_t d = 0;
        if (m_currentPrefix != Z80OpcodePrefix::none)
        {
            m_remainingCycles += 4;
            d = static_cast<int8_t>(m_bus.read(m_registers.pc++));
        }
        m_opcode = fetchOpcode();
        const uint8_t x = m_opcode >> 6;
        const uint8_t y = (m_opcode & 0b00111000) >> 3;
        const uint8_t z = m_opcode & 0b00000111;
        if (x == 0)
        {
            // TODO: rotate/shift
            assert(false);
        }
        else if (x == 1)
        {
            // BIT
            auto value = prefixCbRead(d, z);
            value &= (1 << y);
            m_registers.af.h(true);
            m_registers.af.n(false);
            m_registers.af.z(!value);
        }
        else if (x == 2)
        {
            // RES
            auto value = prefixCbRead(d, z);
            value &= ~(1 << y);
            prefixCbWrite(d, z, value);
        }
        else // if (x == 3)
        {
            // SET
            auto value = prefixCbRead(d, z);
            value |= (1 << y);
            prefixCbWrite(d, z, value);
        }
    }

    void Z80Cpu::prefixDd()
    {
        m_currentPrefix = Z80OpcodePrefix::ix;
        executeInstruction();
    }

    void Z80Cpu::prefixEd()
    {
        m_currentPrefix = Z80OpcodePrefix::none;
        m_opcode = fetchOpcode();
        const auto x = m_opcode >> 6;
        const auto y = (m_opcode & 0b00111000) >> 3;
        const auto z = (m_opcode & 0b00000111);
        if (x == 1)
        {
            // ED: quadrant 1
            if (z == 0b000)
            {
                const auto value = ioRead(m_registers.bc);
                m_registers.af.low(s_flagsLookupSZP[value]);
                switch (y)
                {
                case 0b000:
                    // IN B, (C)
                    m_registers.bc.high(value);
                    break;
                case 0b001:
                    // IN C, (C)
                    m_registers.bc.low(value);
                    break;
                case 0b010:
                    // IN D, (C)
                    m_registers.de.high(value);
                    break;
                case 0b011:
                    // IN E, (C)
                    m_registers.de.low(value);
                    break;
                case 0b100:
                    // IN H, (C)
                    m_registers.hl.high(value);
                    break;
                case 0b101:
                    // IN L, (C)
                    m_registers.hl.low(value);
                    break;
                case 0b110:
                    // IN (C)
                    break;
                case 0b111:
                    // IN A, (C)
                    m_registers.af.high(value);
                    break;
                }
            }
            else if (z == 0b001)
            {
                uint8_t value = 0;
                m_registers.af.low(s_flagsLookupSZP[value]);
                switch (y)
                {
                case 0b000:
                    // OUT (C), B
                    value = m_registers.bc.high();
                    break;
                case 0b001:
                    // OUT (C), C
                    value = m_registers.bc.low();
                    break;
                case 0b010:
                    // OUT (C), D
                    value = m_registers.de.high();
                    break;
                case 0b011:
                    // OUT (C), E
                    value = m_registers.de.low();
                    break;
                case 0b100:
                    // OUT (C), H
                    value = m_registers.hl.high();
                    break;
                case 0b101:
                    // OUT (C), L
                    value = m_registers.hl.low();
                    break;
                case 0b110:
                    // OUT (C)
                    break;
                case 0b111:
                    // OUT (C), A
                    value = m_registers.af.high();
                    break;
                }
                ioWrite(m_registers.bc, value);
            }
            else if (z == 0b010)
            {
                switch (y)
                {
                case 0b000:
                    // SBC HL, BC
                    m_registers.hl = sub16(m_registers.hl, m_registers.bc, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b001:
                    // ADC HL, BC
                    m_registers.hl = add16(m_registers.hl, m_registers.bc, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b010:
                    // SBC HL, DE
                    m_registers.hl = sub16(m_registers.hl, m_registers.de, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b011:
                    // ADC HL, DE
                    m_registers.hl = add16(m_registers.hl, m_registers.de, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b100:
                    // SBC HL, HL
                    m_registers.hl = sub16(m_registers.hl, m_registers.hl, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b101:
                    // ADC HL, HL
                    m_registers.hl = add16(m_registers.hl, m_registers.hl, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b110:
                    // SBC HL, SP
                    m_registers.hl = sub16(m_registers.hl, m_registers.sp, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b111:
                    // ADC HL, SP
                    m_registers.hl = add16(m_registers.hl, m_registers.sp, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                }
            }
            else if (z == 0b011)
            {
                const auto nn = fetch16();
                switch (y)
                {
                case 0b000:
                    // LD (nn), BC
                    write16(nn, m_registers.bc);
                    break;
                case 0b001:
                    // LD BC, (nn)
                    m_registers.bc = read16(nn);
                    break;
                case 0b010:
                    // LD (nn), DE
                    write16(nn, m_registers.de);
                    break;
                case 0b011:
                    // LD DE, (nn)
                    m_registers.de = read16(nn);
                    break;
                case 0b100:
                    // LD (nn), HL
                    write16(nn, m_registers.hl);
                    break;
                case 0b101:
                    // LD HL, (nn)
                    m_registers.hl = read16(nn);
                    break;
                case 0b110:
                    // LD (nn), SP
                    write16(nn, m_registers.sp);
                    break;
                case 0b111:
                    // LD SP, (nn)
                    m_registers.sp = read16(nn);
                    break;
                }
            }
            else if (z == 0b100)
            {
                // NEG
                m_registers.af.high(sub8(0, m_registers.af.high()));
            }
            else if (z == 0b101)
            {
                // RETI/RETN
                m_registers.iff1 = m_registers.iff2;
                m_registers.pc = pop16();
            }
            else if (z == 0b110)
            {
                switch (y)
                {
                case 0b000:
                case 0b001:
                case 0b100:
                case 0b101:
                    m_registers.interruptMode = 0;
                    break;
                case 0b010:
                case 0b110:
                    m_registers.interruptMode = 1;
                    break;
                case 0b011:
                case 0b111:
                    m_registers.interruptMode = 2;
                    break;
                }
            }
            else if (z == 0b111)
            {
                switch (y)
                {
                case 0b000:
                    // LD I, A
                    m_registers.ir.high(m_registers.af.high());
                    m_remainingCycles++;
                    break;
                case 0b001:
                    // LD R, A
                    m_registers.ir.low(m_registers.af.high());
                    m_remainingCycles++;
                    break;
                case 0b010:
                    // LD A, I
                    m_registers.af.high(m_registers.ir.high());
                    m_remainingCycles++;
                    break;
                case 0b011:
                    // LD A, R
                    m_registers.af.high(m_registers.ir.low());
                    m_remainingCycles++;
                    break;
                case 0b100:
                    // RRD
                    m_remainingCycles += 4;
                    assert(false); // TODO
                    break;
                case 0b101:
                    // RLD
                    m_remainingCycles += 4;
                    assert(false); // TODO
                    break;
                case 0b110:
                case 0b111:
                    // ED NOP
                    break;
                }
            }
        }
        else if (x == 2)
        {
            // ED: quadrant 2
            if (z == 0b000)
            {
                switch (y)
                {
                case 0b100:
                    // LDI
                    ldi();
                    break;
                case 0b101:
                    // LDD
                    ldd();
                    break;
                case 0b110:
                    // LDIR
                    ldi();
                    if (m_registers.af.p())
                    {
                        m_registers.pc -= 2;
                        m_remainingCycles += 5;
                    }
                    break;
                case 0b111:
                    // LDDR
                    ldd();
                    if (m_registers.af.p())
                    {
                        m_registers.pc -= 2;
                        m_remainingCycles += 5;
                    }
                    break;
                    // Rest is NOP
                }
            }
            else
            {
                // TODO
                assert(false);
            }
        }
    }

    void Z80Cpu::prefixFd()
    {
        m_currentPrefix = Z80OpcodePrefix::iy;
        executeInstruction();
    }

    uint16_t Z80Cpu::getHL() const
    {
        switch (m_currentPrefix)
        {
        case Z80OpcodePrefix::none: return m_registers.hl;
        case Z80OpcodePrefix::ix:   return m_registers.ix;
        case Z80OpcodePrefix::iy:   return m_registers.iy;
        }
        assert(false);
        return 0;
    }

    void Z80Cpu::setHL(const uint16_t value)
    {
        switch (m_currentPrefix)
        {
        case Z80OpcodePrefix::none: m_registers.hl = value; break;
        case Z80OpcodePrefix::ix:   m_registers.ix = value; break;
        case Z80OpcodePrefix::iy:   m_registers.iy = value; break;
        }
    }

    uint8_t Z80Cpu::busReadHL()
    {
        switch (m_currentPrefix)
        {
        case Z80OpcodePrefix::none:
            return busRead(m_registers.hl);
        case Z80OpcodePrefix::ix:
            {
                const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                m_remainingCycles += 5;
                return busRead(m_registers.ix + d);
            }
        case Z80OpcodePrefix::iy:
            {
                const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                m_remainingCycles += 5;
                return busRead(m_registers.iy + d);
            }
        }
        assert(false);
        return 0;
    }

    void Z80Cpu::busWriteHL(const uint8_t value)
    {
        switch (m_currentPrefix)
        {
        case Z80OpcodePrefix::none:
            return busWrite(m_registers.hl, value);
        case Z80OpcodePrefix::ix:
            {
                const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                m_remainingCycles += 5;
                return busWrite(static_cast<uint16_t>(m_registers.ix + d), value);
            }
        case Z80OpcodePrefix::iy:
            {
                const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                m_remainingCycles += 5;
                return busWrite(static_cast<uint16_t>(m_registers.iy + d), false);
            }
        }
        assert(false);
    }

    uint16_t Z80Cpu::fetch16()
    {
        const auto low = busRead(m_registers.pc++);
        const auto high = busRead(m_registers.pc++);
        return static_cast<uint16_t>(high << 8) | low;
    }

    uint16_t Z80Cpu::read16(const uint16_t address)
    {
        const auto low = busRead(address);
        const auto high = busRead(address + 1);
        return static_cast<uint16_t>(high << 8) | low;
    }

    void Z80Cpu::write16(const uint16_t address, const uint16_t value)
    {
        busWrite(address, value & 0xff);
        busWrite(address + 1, value >> 8);
    }

    uint8_t Z80Cpu::add8(const uint8_t a, const uint8_t b, const bool carryFlag)
    {
        uint8_t result;
        bool carry;
        if (carryFlag)
        {
            result = a + b + 1;
            carry = a >= 0xff - b;
        }
        else
        {
            result = a + b;
            carry = a > 0xff - b;
        }
        const auto carryIn = result ^ a ^ b;
        const auto overflow = (carryIn >> 7) ^ carry;
        m_registers.af.n(false);
        m_registers.af.c(carry);
        m_registers.af.h((carryIn >> 4) & 0x01);
        m_registers.af.p(overflow);
        m_registers.af.s(result >> 7);
        m_registers.af.z(result == 0);
        return result;
    }

    uint8_t Z80Cpu::sub8(const uint8_t a, const uint8_t b, const bool carryFlag)
    {
        const auto result = add8(a, ~b, !carryFlag);
        m_registers.af.value ^= Z80Flags::h | Z80Flags::n | Z80Flags::c; // invert HNC
        return result;
    }

    uint16_t Z80Cpu::add16(uint16_t a, uint16_t b)
    {
        const uint16_t lowResult = (a & 0xff) + (b & 0xff);
        const bool lowCarry = lowResult & 0x100;
        const auto highA = a >> 8;
        const auto highB = b >> 8;
        const auto highResult = highA + highB + lowCarry;
        bool carry;
        if (lowCarry)
        {
            carry = highA >= 0xff - highB;
        }
        else
        {
            carry = highA > 0xff - highB;
        }
        const auto carryIn = (highResult & 0xff) ^ highA ^ highB;
        m_registers.af.h((carryIn >> 4) & 0x01);
        m_registers.af.n(false);
        m_registers.af.c(carry);
        return ((highResult & 0xff) << 8) | (lowResult & 0xff);
    }

    uint16_t Z80Cpu::add16(const uint16_t a, const uint16_t b, const bool carryFlag)
    {
        const uint16_t lowResult = (a & 0xff) + (b & 0xff) + carryFlag;
        const bool lowCarry = lowResult & 0x100;
        const auto highA = a >> 8;
        const auto highB = b >> 8;
        const auto highResult = highA + highB + lowCarry;
        bool carry;
        if (lowCarry)
        {
            carry = highA >= 0xff - highB;
        }
        else
        {
            carry = highA > 0xff - highB;
        }
        const auto carryIn = (highResult & 0xff) ^ highA ^ highB;
        const auto overflow = (carryIn >> 7) ^ carry;
        const auto result = ((highResult & 0xff) << 8) | (lowResult & 0xff);
        m_registers.af.s(result & 0x8000);
        m_registers.af.z(result == 0);
        m_registers.af.h((carryIn >> 4) & 0x01);
        m_registers.af.p(overflow);
        m_registers.af.n(false);
        m_registers.af.c(carry);
        return static_cast<uint16_t>(result & 0xffff);
    }

    uint16_t Z80Cpu::sub16(uint16_t a, uint16_t b)
    {
        const auto result = add16(a, ~b + 1);
        m_registers.af.value ^= Z80Flags::h | Z80Flags::n | Z80Flags::c; // invert HNC
        return result;
    }

    uint16_t Z80Cpu::sub16(const uint16_t a, const uint16_t b, const bool carryFlag)
    {
        const auto result = add16(a, ~b, !carryFlag);
        m_registers.af.value ^= Z80Flags::h | Z80Flags::n | Z80Flags::c; // invert HNC
        return result;
    }

    void Z80Cpu::alu8(const int operation, const uint8_t a, const uint8_t b)
    {
        switch (operation)
        {
        case 0b000:
            // ADD
            m_registers.af.high(add8(a, b));
            return;
        case 0b001:
            // ADC
            m_registers.af.high(add8(a, b, m_registers.af.c()));
            return;
        case 0b010:
            // SUB
            m_registers.af.high(sub8(a, b));
            return;
        case 0b011:
            // SBC
            m_registers.af.high(sub8(a, b, m_registers.af.c()));
            return;
        case 0b100:
            // AND
            {
                const uint8_t result = a & b;
                m_registers.af.high(result);
                m_registers.af.low(s_flagsLookupSZP[result] | Z80Flags::h);
            }
            return;
        case 0b101:
            // XOR
            {
                const uint8_t result = a ^ b;
                m_registers.af.high(result);
                m_registers.af.low(s_flagsLookupSZP[result]);
            }
            return;
        case 0b110:
            // OR
            {
                const uint8_t result = a | b;
                m_registers.af.high(result);
                m_registers.af.low(s_flagsLookupSZP[result]);
            }
            return;
        case 0b111:
            // CP
            sub8(a, b);
            return;
        }
        assert(false);
    }

    bool Z80Cpu::evaluateCondition(const int condition) const
    {
        switch (condition)
        {
        case 0b000:
            // NZ
            return m_registers.af.z() == false;
        case 0b001:
            // Z
            return m_registers.af.z() == true;
        case 0b010:
            // NC
            return m_registers.af.c() == false;
        case 0b011:
            // C
            return m_registers.af.c() == true;
        case 0b100:
            // PO
            return m_registers.af.p() == false;
        case 0b101:
            // PE
            return m_registers.af.p() == true;
        case 0b110:
            // P
            return m_registers.af.s() == false;
        case 0b111:
            // M
            return m_registers.af.s() == true;
        }
        assert(false);
        return false;
    }

    void Z80Cpu::jr(const bool condition)
    {
        const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
        if (condition)
        {
            m_remainingCycles += 5;
            m_registers.pc += d;
        }
    }

    void Z80Cpu::push16(const uint16_t value)
    {
        busWrite(--m_registers.sp, value >> 8);
        busWrite(--m_registers.sp, value & 0xff);
    }

    uint16_t Z80Cpu::pop16()
    {
        const auto low = busRead(m_registers.sp++);
        const auto high = busRead(m_registers.sp++);
        return static_cast<uint16_t>(high << 8) | low;
    }

    void Z80Cpu::ldi()
    {
        const auto n = busRead(m_registers.hl.value++);
        busWrite(m_registers.de.value++, n);
        m_registers.bc.value--;
        m_registers.af.n(false);
        m_registers.af.h(false);
        m_registers.af.p(m_registers.bc.value != 0);
        m_remainingCycles += 2;
    }

    void Z80Cpu::ldd()
    {
        const auto n = busRead(m_registers.hl.value--);
        busWrite(m_registers.de.value--, n);
        m_registers.bc.value--;
        m_registers.af.n(false);
        m_registers.af.h(false);
        m_registers.af.p(m_registers.bc.value != 0);
        m_remainingCycles += 2;
    }

    uint8_t Z80Cpu::prefixCbRead(const int8_t d, const int z)
    {
        if (z == 0b110)
        {
            // (HL)
            switch (m_currentPrefix)
            {
            case Z80OpcodePrefix::none:
                return busRead(m_registers.hl);
            case Z80OpcodePrefix::ix:
                return busRead(m_registers.ix + d);
            case Z80OpcodePrefix::iy:
                return busRead(m_registers.iy + d);
            }
            assert(false);
            return 0;
        }
        else
        {
            return *m_registersPointers[z];
        }
    }

    void Z80Cpu::prefixCbWrite(const int8_t d, const int z, const uint8_t value)
    {
        if (z == 0b110)
        {
            // (HL)
            switch (m_currentPrefix)
            {
            case Z80OpcodePrefix::none:
                busWrite(m_registers.hl, value);
                break;
            case Z80OpcodePrefix::ix:
                busWrite(m_registers.ix + d, value);
                break;
            case Z80OpcodePrefix::iy:
                busWrite(m_registers.iy + d, value);
                break;
            }
        }
        else
        {
            *m_registersPointers[z] = value;
            switch (m_currentPrefix)
            {
            case Z80OpcodePrefix::ix:
                busWrite(m_registers.ix + d, value);
                break;
            case Z80OpcodePrefix::iy:
                busWrite(m_registers.iy + d, value);
                break;
            }
        }
    }
}

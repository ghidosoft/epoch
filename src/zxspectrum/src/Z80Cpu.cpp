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

    static const uint8_t signZeroParityFlags[256] = {
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
    }

    void Z80Cpu::clock()
    {
        if (m_remainingCycles == 0)
        {
            if (m_halted)
            {
                return;
            }
            m_opcode = fetchOpcode();

            if ((m_opcode & 0b11000000) == 0b01000000)
            {
                // LD 8bit / HALT
                const auto dst = (m_opcode & 0b00111000) >> 3;
                const auto src = (m_opcode & 0b00000111);
                const uint8_t* srcPtr = m_registersPointers[src];
                uint8_t* dstPtr = m_registersPointers[dst];
                if (src == 0b110)
                {
                    if (dst == 0b110)
                    {
                        // HALT
                        m_halted = true;
                    }
                    else
                    {
                        // LD dst, (HL)
                        *dstPtr = busRead(m_registers.hl.value);
                    }
                }
                else
                {
                    if (dst == 0b110)
                    {
                        // LD (HL), src
                        busWrite(m_registers.hl.value, *srcPtr);
                    }
                    else
                    {
                        // LD dst, src
                        *dstPtr = *srcPtr;
                    }
                }
            }
            else if ((m_opcode & 0b11000000) == 0b10000000)
            {
                // ALU operations
                const auto operation = (m_opcode & 0b00111000) >> 3;
                const auto src = (m_opcode & 0b00000111);
                const uint8_t* srcPtr = m_registersPointers[src];
                const auto a = m_registers.af.high();
                uint8_t b;
                if (src == 0b110)
                {
                    b = busRead(m_registers.hl.value);
                }
                else
                {
                    b = *srcPtr;
                }
                switch (operation)
                {
                case 0b000:
                    // ADD
                    add8(a, b, 0);
                    break;
                case 0b001:
                    // ADC
                    add8(a, b, m_registers.af.c());
                    break;
                case 0b010:
                    // SUB
                    add8(a, ~b, 1);
                    m_registers.af.c(!m_registers.af.c());
                    break;
                case 0b011:
                    // SBC
                    add8(a, ~b, !m_registers.af.c());
                    m_registers.af.c(!m_registers.af.c());
                    break;
                case 0b100:
                    // AND
                    {
                        const uint8_t result = a & b;
                        m_registers.af.high(result);
                        m_registers.af.low(signZeroParityFlags[result] | Z80Flags::halfCarry);
                    }
                    break;
                case 0b101:
                    // XOR
                    {
                        const uint8_t result = a ^ b;
                        m_registers.af.high(result);
                        m_registers.af.low(signZeroParityFlags[result]);
                    }
                    break;
                case 0b110:
                    // OR
                {
                    const uint8_t result = a | b;
                    m_registers.af.high(result);
                    m_registers.af.low(signZeroParityFlags[result]);
                }
                    break;
                default:
                    // TODO: AND XOR OR CP
                    assert(false);
                    break;
                }
            }
        }

        m_remainingCycles--;
    }

    void Z80Cpu::step()
    {
        if (m_halted) return;
        do
        {
            clock();
        } while (m_remainingCycles > 0);
    }

    void Z80Cpu::reset()
    {
        m_registers = {};
        m_remainingCycles = {};
        m_halted = {};
    }

    Z80Registers& Z80Cpu::registers()
    {
        return m_registers;
    }

    const Z80Registers& Z80Cpu::registers() const
    {
        return m_registers;
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

    void Z80Cpu::add8(const uint8_t a, const uint8_t b, const uint8_t carryFlag)
    {
        uint8_t result;
        int carry;
        if (carryFlag)
        {
            result = a + b + 1;
            carry = (a >= 0xff - b) ? 1 : 0;
        }
        else
        {
            result = a + b;
            carry = (a > 0xff - b) ? 1 : 0;
        }
        const auto carryIn = result ^ a ^ b;
        const auto overflow = (carryIn >> 7) ^ carry;
        m_registers.af.high(result);
        m_registers.af.n(false);
        m_registers.af.c(carry);
        m_registers.af.h((carryIn >> 4) & 0x01);
        m_registers.af.p(overflow);
        m_registers.af.s(result >> 7);
        m_registers.af.z(result == 0);
    }
}

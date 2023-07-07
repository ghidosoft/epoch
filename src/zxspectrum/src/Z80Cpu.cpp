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
            m_remainingCycles = 4;

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
                        m_remainingCycles += 3;
                        *dstPtr = m_bus.read(m_registers.hl.value);
                    }
                }
                else
                {
                    if (dst == 0b110)
                    {
                        // LD (HL), src
                        m_remainingCycles += 3;
                        m_bus.write(m_registers.hl.value, *srcPtr);
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
                // TODO: ALU operations
            }
        }

        m_remainingCycles--;
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
        return m_bus.read(m_registers.pc++);
    }
}

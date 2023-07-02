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
#include <sstream>

#include "Z80Config.inc.h"

namespace epoch::zxspectrum
{
    Z80Cpu::Z80Cpu(Z80Interface& bus) : m_bus{ bus }
    {
        std::istringstream is{ Z80CONFIG };

        std::string line;
        while (std::getline(is, line))
        {
            if (!line.empty() && line[0] != '#')
            {
                std::istringstream is{ line };
                uint32_t op;
                std::string m1, m2, m3, m4, m5, m6, mnemonic, op1, op2;
                is >> std::hex >> op;
                is >> m1;
                is >> m2;
                is >> m3;
                is >> m4;
                is >> m5;
                is >> m6;
                is >> mnemonic;
                is >> op1;
                is >> op2;
                // TODO: parse config and generate instructions
                // TODO: support for 2/3-bytes opcodes
                assert(op <= 0xff);
                m_instructions[op] = {
                    .mnemonic = mnemonic,
                };
            }
        }
    }

    void Z80Cpu::clock()
    {
        if (m_remainingCycles > 1)
        {
            m_remainingCycles--;
            return;
        }

        switch (m_machineCycle)
        {
        case 0:
            const auto value = m_bus.read(m_registers.pc++);
            m_remainingCycles = 4;
            m_machineCycle++;
            const auto& instruction = m_instructions[value];
            break;
        }

        m_remainingCycles--;
    }

    void Z80Cpu::reset()
    {
        m_registers = {};
        m_machineCycle = {};
        m_remainingCycles = {};
    }

    Z80Registers& Z80Cpu::registers()
    {
        return m_registers;
    }

    const Z80Registers& Z80Cpu::registers() const
    {
        return m_registers;
    }
}

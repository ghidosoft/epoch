#include "Z80Cpu.h"

namespace epoch::zxspectrum
{
    Z80Cpu::Z80Cpu() = default;

    void Z80Cpu::clock()
    {
    }

    void Z80Cpu::reset()
    {
        m_registers = {};
    }

    Z80Registers& Z80Cpu::registers()
    {
        return m_registers;
    }
}

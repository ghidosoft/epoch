#include "Z80Cpu.h"

namespace epoch::zxspectrum
{
    Z80Cpu::Z80Cpu()
    {
        reset();
    }

    void Z80Cpu::clock()
    {
    }

    void Z80Cpu::reset()
    {
        m_programCounter = 0;
    }
}
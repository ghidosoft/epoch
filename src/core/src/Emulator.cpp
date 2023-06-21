#include "Emulator.h"

namespace epoch
{
    Emulator::Emulator(EmulatorInfo info) : m_info{ info }
    {
    }

    Emulator::~Emulator() = default;

    const EmulatorInfo& Emulator::info() const
    {
        return m_info;
    }
}

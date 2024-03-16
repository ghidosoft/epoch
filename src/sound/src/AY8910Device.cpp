/* This file is part of Epoch, Copyright (C) 2024 Andrea Ghidini.
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

#include "AY8910Device.hpp"

#include <cassert>

namespace epoch::sound
{
    AY8910Device::AY8910Device() = default;

    void AY8910Device::reset()
    {
        m_address = 0;
        m_data = 0;
    }

    void AY8910Device::clock()
    {
        for (auto& tone : m_tones)
        {
            const auto period = std::max(tone.period, 1u);
            tone.count++;
            while (tone.count > period)
            {
                tone.output = !tone.output;
                tone.count -= period;
            }
        }
    }

    void AY8910Device::address(const uint8_t value)
    {
        assert(value < m_registers.size());
        m_address = value;
    }

    void AY8910Device::data(const uint8_t data)
    {
        m_registers[m_address] = data;
        switch (m_address)
        {
            case 0:
            case 1:
                m_tones[0].period = (m_registers[0] | (m_registers[1] << 8));
                break;
            case 2:
            case 3:
                m_tones[1].period = (m_registers[2] | (m_registers[3] << 8));
                break;
            case 4:
            case 5:
                m_tones[0].period = (m_registers[4] | (m_registers[5] << 8));
                break;
        }
    }

    uint8_t AY8910Device::data() const { return m_registers[m_address]; }

    float AY8910Device::output() const
    {
        const bool noise = m_registers[7] & 0b00001000;
        const bool channelA = noise & (m_tones[0].output | (m_registers[7] & 0b00000001));
        const bool channelB = noise & (m_tones[1].output | (m_registers[7] & 0b00000010));
        const bool channelC = noise & (m_tones[2].output | (m_registers[7] & 0b00000100));

        float output = 0.f;

        if (channelA)
        {
            output += 1.f;
        }
        if (channelB)
        {
            output += 1.f;
        }
        if (channelC)
        {
            output += 1.f;
        }

        return output / 3.f;
    }
}  // namespace epoch::sound

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

namespace
{
    float VOLUME_VALUES[16] = {0.f / 65535.f,     513.f / 65535.f,   828.f / 65535.f,   1239.f / 65535.f,
                               1923.f / 65535.f,  3238.f / 65535.f,  4926.f / 65535.f,  9110.f / 65535.f,
                               10344.f / 65535.f, 17876.f / 65535.f, 24682.f / 65535.f, 30442.f / 65535.f,
                               38844.f / 65535.f, 47270.f / 65535.f, 56402.f / 65535.f, 65535.f / 65535.f};
}

namespace epoch::sound
{
    AY8910Device::AY8910Device() = default;

    void AY8910Device::reset()
    {
        m_address = 0;
        m_data = 0;
        m_counter = 0;
        m_channels = {};
        m_noise = {};
        m_envelope = {};
    }

    void AY8910Device::clock()
    {
        if (m_counter == 0)
        {
            for (auto& tone : m_channels)
            {
                const auto period = tone.period > 0 ? tone.period : 1;
                tone.count++;
                while (tone.count > period)
                {
                    tone.output = !tone.output;
                    tone.count -= period;
                }
            }
            m_counter = 16;
        }
        m_counter--;
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
                m_channels[0].period = m_registers[0] | ((m_registers[1] & 0x0f) << 8);
                break;
            case 2:
            case 3:
                m_channels[1].period = m_registers[2] | ((m_registers[3] & 0x0f) << 8);
                break;
            case 4:
            case 5:
                m_channels[2].period = m_registers[4] | ((m_registers[5] & 0x0f) << 8);
                break;
            case 6:
                m_noise.period = m_registers[6] & 0x1f;
                break;
            case 8:
                m_channels[0].envelope = m_registers[8] & 0x10;
                m_channels[0].volume = VOLUME_VALUES[m_registers[8] & 0x0f];
                break;
            case 9:
                m_channels[1].envelope = m_registers[9] & 0x10;
                m_channels[1].volume = VOLUME_VALUES[m_registers[9] & 0x0f];
                break;
            case 10:
                m_channels[2].envelope = m_registers[10] & 0x10;
                m_channels[2].volume = VOLUME_VALUES[m_registers[10] & 0x0f];
                break;
            case 11:
            case 12:
                m_envelope.period = (m_registers[11] | (m_registers[12] << 8));
                break;
            case 13:
                m_envelope.hold = m_registers[13] & 0x01;
                m_envelope.alternate = m_registers[13] & 0x02;
                m_envelope.attack = m_registers[13] & 0x04;
                m_envelope.cont = m_registers[13] & 0x08;
                break;
        }
    }

    uint8_t AY8910Device::data() const { return m_registers[m_address]; }

    float AY8910Device::output() const
    {
        const bool channelA = (m_registers[7] & 0b00001000) && (m_channels[0].output | (m_registers[7] & 0b00000001));
        const bool channelB = (m_registers[7] & 0b00010000) && (m_channels[1].output | (m_registers[7] & 0b00000010));
        const bool channelC = (m_registers[7] & 0b00100000) && (m_channels[2].output | (m_registers[7] & 0b00000100));

        float output = 0.f;

        if (channelA)
        {
            output += m_channels[0].volume;
        }
        if (channelB)
        {
            output += m_channels[1].volume;
        }
        if (channelC)
        {
            output += m_channels[2].volume;
        }

        return output / 3.f;
    }
}  // namespace epoch::sound

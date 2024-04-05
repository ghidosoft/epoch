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
                tone.count++;
                while (tone.count >= tone.period)
                {
                    tone.output = !tone.output;
                    tone.count -= tone.period;
                }
            }
            {
                m_noise.count++;
                if (m_noise.count >= m_noise.period)
                {
                    m_noise.random =
                        (m_noise.random >> 1) | (((m_noise.random & 0x01) ^ ((m_noise.random >> 3) & 0x01)) << 16);
                    m_noise.output = m_noise.random & 0x01;
                    m_noise.count -= m_noise.period;
                }
            }
            {
                m_envelope.count++;
                if (m_envelope.count >= m_envelope.period)
                {
                    m_envelope.step++;
                    if (m_envelope.step >= EnvelopeLookupTable::Size) m_envelope.step = EnvelopeLookupTable::Size / 2;
                    m_envelope.count = 0;
                }
                m_envelope.volume = m_envelopeLookup.get(m_envelope.shape, m_envelope.step);
            }
            m_counter = 8;
        }
        m_counter--;
    }

    void AY8910Device::address(const uint8_t value) { m_address = value & 0x0f; }

    void AY8910Device::data(const uint8_t data)
    {
        m_registers[m_address] = data;
        switch (m_address)
        {
            case 0:
            case 1:
                m_channels[0].period = std::max(m_registers[0] | ((m_registers[1] & 0x0f) << 8), 1);
                break;
            case 2:
            case 3:
                m_channels[1].period = std::max(m_registers[2] | ((m_registers[3] & 0x0f) << 8), 1);
                break;
            case 4:
            case 5:
                m_channels[2].period = std::max(m_registers[4] | ((m_registers[5] & 0x0f) << 8), 1);
                break;
            case 6:
                m_noise.period = std::max(m_registers[6] & 0x1f, 1);
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
                m_envelope.period = std::max((m_registers[11] | (m_registers[12] << 8)), 1);
                break;
            case 13:
                m_envelope.shape = m_registers[13] & 0x0f;
                m_envelope.count = 0;
                m_envelope.step = 0;
                break;
        }
    }

    uint8_t AY8910Device::data() const { return m_registers[m_address]; }

    SoundSample AY8910Device::output() const
    {
        const bool channelA = (m_noise.output || (m_registers[7] & 0b00001000)) &&
                              (m_channels[0].output || (m_registers[7] & 0b00000001));
        const bool channelB = (m_noise.output || (m_registers[7] & 0b00010000)) &&
                              (m_channels[1].output || (m_registers[7] & 0b00000010));
        const bool channelC = (m_noise.output || (m_registers[7] & 0b00100000)) &&
                              (m_channels[2].output || (m_registers[7] & 0b00000100));

        SoundSample output{};

        if (channelA)
        {
            output += (m_channels[0].envelope ? m_envelope.volume : m_channels[0].volume) * m_channelMix[0];
        }
        if (channelB)
        {
            output += (m_channels[1].envelope ? m_envelope.volume : m_channels[1].volume) * m_channelMix[1];
        }
        if (channelC)
        {
            output += (m_channels[2].envelope ? m_envelope.volume : m_channels[2].volume) * m_channelMix[2];
        }

        return output * .333333333f;
    }

    AY8910Device::EnvelopeLookupTable::EnvelopeLookupTable()
    {
        for (auto shape = 0; shape < 16; shape++)
        {
            const bool attack = shape & 0x04;
            auto hold = false;
            auto dir = attack ? 1 : -1;
            auto vol = attack ? -1 : 32;
            for (auto i = 0; i < 128; i++)
            {
                if (!hold)
                {
                    vol += dir;
                    if (vol < 0 || vol >= 32)
                    {
                        if (shape & 0x08)
                        {
                            if (shape & 0x02)
                            {
                                dir = -dir;
                            }
                            vol = (dir > 0) ? 0 : 31;
                            if (shape & 0x01)
                            {
                                hold = true;
                                vol = (dir > 0) ? 31 : 0;
                            }
                        }
                        else
                        {
                            vol = 0;
                            hold = true;
                        }
                    }
                }
                m_values[shape][i] = static_cast<float>(vol) / 31.f;
            }
        }
    }
}  // namespace epoch::sound

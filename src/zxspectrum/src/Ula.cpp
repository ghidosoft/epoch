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

#include "Ula.h"

#include <bit>
#include <cassert>

namespace epoch::zxspectrum
{
    Ula::Ula(MemoryBank& rom48k, std::array<MemoryBank, 8>& ram) : m_rom48k{ rom48k }, m_ram{ ram },
        m_keyboardState{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
    {
    }

    void Ula::clock()
    {
        if (m_cpuStalled > 0) m_cpuStalled--;

        if (m_y >= 0 && m_x >= 0)
        {
            m_borderBuffer[m_y * Width + m_x] = m_border;
            m_borderBuffer[m_y * Width + m_x + 1] = m_border;
        }

        m_x++;
        m_x++;

        if (m_x >= Width)
        {
            m_x = -HorizontalRetrace;
            m_y++;
        }
        if (m_y >= Height)
        {
            m_y = -VerticalRetrace;
            m_frameCounter++;
        }
    }

    void Ula::reset()
    {
        m_floatingBusValue = {};
        m_border = {};
        m_ear = m_mic = {};
        m_cpuStalled = {};

        m_frameCounter = 0;
        m_x = -HorizontalRetrace;
        m_y = -VerticalRetrace;
    }

    uint8_t Ula::read(const uint16_t address)
    {
        if (address <= 0x3fff)
        {
            m_floatingBusValue = m_rom48k[address];
        }
        else if (address <= 0x7fff)
        {
            m_floatingBusValue = m_ram[5][address & 0x3fff];
        }
        else if (address <= 0xbfff)
        {
            m_floatingBusValue = m_ram[2][address & 0x3fff];
        }
        else
        {
            // TODO: allow switching bank for 128K spectrums
            m_floatingBusValue = m_ram[0][address & 0x3fff];
        }
        return m_floatingBusValue;
    }

    void Ula::write(const uint16_t address, const uint8_t value)
    {
        if (address <= 0x3fff)
        {
            // ROM
        }
        else if (address <= 0x7fff)
        {
            m_ram[5][address & 0x3fff] = m_floatingBusValue = value;
        }
        else if (address <= 0xbfff)
        {
            m_ram[2][address & 0x3fff] = m_floatingBusValue = value;
        }
        else
        {
            // TODO: allow switching bank for 128K spectrums
            m_ram[0][address & 0x3fff] = m_floatingBusValue = value;
        }
    }

    uint8_t Ula::ioRead(const uint16_t port)
    {
        if ((port & 0x01) == 0)
        {
            const uint8_t portHigh = port >> 8;
            uint8_t result = 0b00011111;
            for (auto i = 0; i < 8; i++)
            {
                if ((portHigh & (1 << i)) == 0)
                {
                    result &= m_keyboardState[i];
                }
            }
            if (m_ear) result |= 0b01000000;
            return result | 0b10100000;
        }
        return 0xff;
    }

    void Ula::ioWrite(const uint16_t port, const uint8_t value)
    {
        if ((port & 0x01) == 0)
        {
            const auto newEar = value & 0b00010000;
            const auto newMic = !(value & 0b00001000);
            /*if (m_ear != newEar || m_mic != newMic)
            {
                m_audioOutput = !m_audioOutput;
            }*/
            m_ear = newEar;
            m_mic = newMic;
            m_border = value & 0x07;
        }
    }

    void Ula::setKeyState(const int row, const int col, const bool state)
    {
        assert(row < 8);
        assert(col < 5);
        if (state)
        {
            m_keyboardState[row] &= ~(1 << col);
        }
        else
        {
            m_keyboardState[row] |= 1 << col;
        }
    }
}

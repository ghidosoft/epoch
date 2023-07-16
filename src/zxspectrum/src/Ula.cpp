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

#include <cassert>

namespace epoch::zxspectrum
{
    Ula::Ula(MemoryBank& rom48k, std::array<MemoryBank, 8>& ram) : m_rom48k{ rom48k }, m_ram{ ram }
    {
    }

    void Ula::clock()
    {
        if (m_cpuStalled > 0) m_cpuStalled--;

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

        if (m_y >= 0 && m_x >= 0)
        {
            m_borderBuffer[m_y * Width + m_x] = m_border;
            m_borderBuffer[m_y * Width + m_x + 1] = m_border;
        }

        m_x++;
        m_x++;
    }

    void Ula::reset()
    {
        m_floatingBusValue = {};
        m_border = {};
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

    uint8_t Ula::ioRead(const uint8_t port)
    {
        // TODO
        return 0;
    }

    void Ula::ioWrite(const uint8_t port, const uint8_t value)
    {
        if ((port & 0x01) == 0)
        {
            // TODO ear/mic
            m_border = value & 0x07;
        }
    }

    bool Ula::invertPaperInk() const
    {
        // every 16 frames
        return m_frameCounter & 0x10;
    }
}

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

#include "Ula.hpp"

#include <cassert>

namespace epoch::zxspectrum
{
    Ula::Ula(const UlaType type, const std::span<const uint8_t> rom) : m_type{ type }, m_keyboardState { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
    {
        assert(rom.size() <= sizeof(m_rom));
        std::memcpy(m_rom.data(), rom.data(), rom.size());
    }

    void Ula::clock()
    {
        if (m_cpuStalled > 0) m_cpuStalled--;

        if (m_y >= 0 && m_x >= 0)
        {
            m_borderBuffer[m_y * Width + m_x] = m_border;
            m_borderBuffer[m_y * Width + m_x + 1] = m_border;
        }

        // 2 pixels per T-state
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

        m_ramSelect = 0;
        m_vramSelect = 5;
        m_romSelect = 0;
        m_pagingState = 0;

        m_frameCounter = 0;
        m_x = -HorizontalRetrace;
        m_y = -VerticalRetrace;
    }

    uint8_t Ula::read(const uint16_t address)
    {
        if (address <= 0x3fff)
        {
            m_floatingBusValue = m_rom[m_romSelect][address];
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
            m_floatingBusValue = m_ram[m_ramSelect][address & 0x3fff];
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
            m_ram[m_ramSelect][address & 0x3fff] = m_floatingBusValue = value;
        }
    }

    uint8_t Ula::ioRead(const uint16_t port)
    {
        if ((port & 0b11100000) == 0)
        {
            // Kempston joystick takes priority
            return m_kempstonState;
        }
        else if ((port & 0x01) == 0)
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
            if (m_ear || m_audioIn) result |= 0b01000000;
            return result | 0b10100000;
        }
        switch (m_type)
        {
        case UlaType::zx48k:
            break;
        case UlaType::zx128k:
            if ((port & 0b1000000000000010) == 0)
            {
                return m_pagingState;
            }
            break;
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
        switch (m_type)
        {
        case UlaType::zx48k:
            break;
        case UlaType::zx128k:
            if ((port & 0b1000000000000010) == 0)
            {
                if ((m_pagingState & 0b00100000) == 0)
                {
                    m_pagingState = value;
                    m_ramSelect = m_pagingState & 0x07;
                    m_vramSelect = (m_pagingState & 0b00001000) ? 7 : 5;
                    m_romSelect = (m_pagingState >> 4) & 0x01;
                }
            }
            break;
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

    void Ula::setKempstonState(const int button, const bool state)
    {
        assert(button < 5);
        if (state)
        {
            m_kempstonState |= 1 << button;
        }
        else
        {
            m_kempstonState &= ~(1 << button);
        }
    }

    uint8_t Ula::vramRead(const uint16_t address) const
    {
        return m_ram[m_vramSelect][address & 0x3fff]; // TODO: should update floating bus value?
    }
}

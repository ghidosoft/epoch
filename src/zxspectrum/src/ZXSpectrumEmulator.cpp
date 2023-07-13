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

#include "ZXSpectrumEmulator.h"

#include "Rom.h"
#include "Ula.h"
#include "Z80Cpu.h"

#include <cstring>

namespace epoch::zxspectrum
{
    static constexpr uint8_t darkColor = 0xd9;
    static constexpr uint8_t brightColor = 0xff;
    static Color defaultColors[] = {
        {0x00, 0x00, 0x00},
        {0x00, 0x00, darkColor},
        {darkColor, 0x00, 0x00},
        {darkColor, 0x00, darkColor},
        {0x00, darkColor, 0x00},
        {0x00, darkColor, darkColor},
        {darkColor, darkColor, 0x00},
        {darkColor, darkColor, darkColor},
        {0x00, 0x00, 0x00},
        {0x00, 0x00, brightColor},
        {brightColor, 0x00, 0x00},
        {brightColor, 0x00, brightColor},
        {0x00, brightColor, 0x00},
        {0x00, brightColor, brightColor},
        {brightColor, brightColor, 0x00},
        {brightColor, brightColor, brightColor},
    };
    const Palette ZXSpectrumEmulator::DefaultPalette{ defaultColors };

    ZXSpectrumEmulator::ZXSpectrumEmulator() :
        Emulator{ {"ZX Spectrum", ScreenWidth + BorderLeft + BorderRight, ScreenHeight + BorderTop + BorderBottom} },
        m_ula{std::make_unique<Ula>(m_rom48k, m_ram)},
        m_cpu{std::make_unique<Z80Cpu>(*m_ula)}
    {
        // Assembled with https://www.asm80.com/
        // TODO: remove this simple ROM test
        static const uint8_t bytes[] = {
            0x11, 0x00, 0x58,   // LD   de, 22528
            0x3E, 0x07,         // LD   a, 7
            0xD3, 0xFE,         // OUT  $fe, a
            0x12,               // LD   (de), a
            0x11, 0x00, 0x40,   // LD   de, 16384
            0x3E, 0xFF,         // LD   a, 255
            0x06, 0x08,         // LD   b, 8
            // loop:
            0x12,               // LD   (de), a
            0x14,               // INC  d
            0x10, 0xFC,         // DJNZ loop
            // main:
            0x18, 0xFE,         // JR   main
        };
        // std::memcpy(m_rom48k.data(), bytes, sizeof(bytes));
        std::memcpy(m_rom48k.data(), ROM_48K, sizeof(ROM_48K));
    }

    ZXSpectrumEmulator::~ZXSpectrumEmulator() = default;

    void ZXSpectrumEmulator::clock()
    {
        if (!m_ula->isCpuStalled())
        {
            m_cpu->clock();
        }
        m_ula->clock();
        m_clockCounter++;

        if (m_x >= Width)
        {
            m_x = -HorizontalRetrace;
            m_y++;
        }
        if (m_y >= Height)
        {
            m_y = -VerticalRetrace;
        }

        if (m_y >= 0 && m_x >= 0)
        {
            m_borderBuffer[m_y * Width + m_x] = m_ula->borderColor();
            m_borderBuffer[m_y * Width + m_x + 1] = m_ula->borderColor();
        }

        m_x++;
        m_x++;
    }

    void ZXSpectrumEmulator::reset()
    {
        m_ula->reset();
        m_cpu->reset();
        m_clockCounter = 0;
        m_frameCounter = 0;
        m_x = -HorizontalRetrace;
        m_y = -VerticalRetrace;
    }

    uint8_t ZXSpectrumEmulator::vramRead(const uint16_t address) const
    {
        // TODO: allow switching bank for 128K spectrums
        return m_ram[5][address & 0x3fff]; // TODO: should update floating bus value?
    }

    std::span<const uint32_t> ZXSpectrumEmulator::screenBuffer()
    {
        std::size_t source = 0;
        std::size_t dest = 0;
        for (std::size_t y = 0; y < ScreenHeight + BorderTop + BorderBottom; y++)
        {
            for (std::size_t x = 0; x < ScreenWidth + BorderLeft + BorderRight; x++)
            {
                Color color;
                if (y < BorderTop || y >= ScreenHeight + BorderTop || x < BorderLeft || x >= ScreenWidth + BorderLeft)
                {
                    color = DefaultPalette.map(m_borderBuffer[source]);
                }
                else
                {
                    const auto xPixel = (x - BorderLeft);
                    const auto yPixel = (y - BorderTop);

                    uint16_t pixelAddress = 0x4000;
                    pixelAddress |= (xPixel >> 3) & 0b11111;
                    pixelAddress |= (yPixel & 0b00000111) << 8;
                    pixelAddress |= (yPixel & 0b00111000) << 2;
                    pixelAddress |= (yPixel & 0b11000000) << 5;
                    const auto pixelData = vramRead(pixelAddress);
                    const bool pixel = (pixelData >> (7 - (xPixel & 0b111))) & 0x01;

                    const auto attribute = vramRead(0x5800 + ((yPixel >> 3) << 5) + (xPixel >> 3));

                    auto paper = (attribute >> 3) & 0x07;
                    auto ink = attribute & 0x07;
                    const bool bright = attribute & 0x40;
                    if (bright)
                    {
                        paper += 0x08;
                        ink += 0x08;
                    }
                    const bool flash = (attribute & 0x80) && (m_frameCounter & 0x10); // every 16 frames
                    color = DefaultPalette.map((pixel && !flash) || (!pixel && flash) ? ink : paper);
                }

                source++;
                m_screenBuffer[dest++] = color.rgba;
            }
        }
        return m_screenBuffer;
    }
}

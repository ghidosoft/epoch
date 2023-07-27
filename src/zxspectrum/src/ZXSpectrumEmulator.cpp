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

#include "Io.h"
#include "Rom.h"
#include "TapeInterface.h"
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
        Emulator{ {"ZX Spectrum", Width, Height, TStatesPerFrame, FramesPerSecond } },
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
        // std::memcpy(m_rom48k.data(), ZX_Test_Rom, sizeof(ZX_Test_Rom));
    }

    ZXSpectrumEmulator::~ZXSpectrumEmulator() = default;

    void ZXSpectrumEmulator::reset()
    {
        m_ula->reset();
        m_cpu->reset();
        m_tape = {};
        m_clockCounter = 0;
    }

    void ZXSpectrumEmulator::loadSnapshot(const std::string& path)
    {
        load(path, this);
    }

    void ZXSpectrumEmulator::saveSnapshot(const std::string& path)
    {
        save(path, this);
    }

    void ZXSpectrumEmulator::loadTape(const std::string& path)
    {
        m_tape = epoch::zxspectrum::loadTape(path);
    }

    void ZXSpectrumEmulator::keyEvent(const Key key, const KeyAction action)
    {
        switch (key)
        {
        case Key::LeftShift:
        case Key::RightShift:
            m_ula->setKeyState(0, 0, action != KeyAction::release);
            break;
        case Key::Z:
            m_ula->setKeyState(0, 1, action != KeyAction::release);
            break;
        case Key::X:
            m_ula->setKeyState(0, 2, action != KeyAction::release);
            break;
        case Key::C:
            m_ula->setKeyState(0, 3, action != KeyAction::release);
            break;
        case Key::V:
            m_ula->setKeyState(0, 4, action != KeyAction::release);
            break;

        case Key::A:
            m_ula->setKeyState(1, 0, action != KeyAction::release);
            break;
        case Key::S:
            m_ula->setKeyState(1, 1, action != KeyAction::release);
            break;
        case Key::D:
            m_ula->setKeyState(1, 2, action != KeyAction::release);
            break;
        case Key::F:
            m_ula->setKeyState(1, 3, action != KeyAction::release);
            break;
        case Key::G:
            m_ula->setKeyState(1, 4, action != KeyAction::release);
            break;

        case Key::Q:
            m_ula->setKeyState(2, 0, action != KeyAction::release);
            break;
        case Key::W:
            m_ula->setKeyState(2, 1, action != KeyAction::release);
            break;
        case Key::E:
            m_ula->setKeyState(2, 2, action != KeyAction::release);
            break;
        case Key::R:
            m_ula->setKeyState(2, 3, action != KeyAction::release);
            break;
        case Key::T:
            m_ula->setKeyState(2, 4, action != KeyAction::release);
            break;

        case Key::D1:
            m_ula->setKeyState(3, 0, action != KeyAction::release);
            break;
        case Key::D2:
            m_ula->setKeyState(3, 1, action != KeyAction::release);
            break;
        case Key::D3:
            m_ula->setKeyState(3, 2, action != KeyAction::release);
            break;
        case Key::D4:
            m_ula->setKeyState(3, 3, action != KeyAction::release);
            break;
        case Key::D5:
            m_ula->setKeyState(3, 4, action != KeyAction::release);
            break;

        case Key::D6:
            m_ula->setKeyState(4, 4, action != KeyAction::release);
            break;
        case Key::D7:
            m_ula->setKeyState(4, 3, action != KeyAction::release);
            break;
        case Key::D8:
            m_ula->setKeyState(4, 2, action != KeyAction::release);
            break;
        case Key::D9:
            m_ula->setKeyState(4, 1, action != KeyAction::release);
            break;
        case Key::D0:
            m_ula->setKeyState(4, 0, action != KeyAction::release);
            break;

        case Key::Y:
            m_ula->setKeyState(5, 4, action != KeyAction::release);
            break;
        case Key::U:
            m_ula->setKeyState(5, 3, action != KeyAction::release);
            break;
        case Key::I:
            m_ula->setKeyState(5, 2, action != KeyAction::release);
            break;
        case Key::O:
            m_ula->setKeyState(5, 1, action != KeyAction::release);
            break;
        case Key::P:
            m_ula->setKeyState(5, 0, action != KeyAction::release);
            break;

        case Key::H:
            m_ula->setKeyState(6, 4, action != KeyAction::release);
            break;
        case Key::J:
            m_ula->setKeyState(6, 3, action != KeyAction::release);
            break;
        case Key::K:
            m_ula->setKeyState(6, 2, action != KeyAction::release);
            break;
        case Key::L:
            m_ula->setKeyState(6, 1, action != KeyAction::release);
            break;
        case Key::Enter:
            m_ula->setKeyState(6, 0, action != KeyAction::release);
            break;

        case Key::B:
            m_ula->setKeyState(7, 4, action != KeyAction::release);
            break;
        case Key::N:
            m_ula->setKeyState(7, 3, action != KeyAction::release);
            break;
        case Key::M:
            m_ula->setKeyState(7, 2, action != KeyAction::release);
            break;
        case Key::LeftControl:
            m_ula->setKeyState(7, 1, action != KeyAction::release);
            break;
        case Key::Space:
            m_ula->setKeyState(7, 0, action != KeyAction::release);
            break;

        case Key::Right:
            m_ula->setKempstonState(0, action != KeyAction::release);
            break;
        case Key::Left:
            m_ula->setKempstonState(1, action != KeyAction::release);
            break;
        case Key::Down:
            m_ula->setKempstonState(2, action != KeyAction::release);
            break;
        case Key::Up:
            m_ula->setKempstonState(3, action != KeyAction::release);
            break;
        case Key::RightControl:
            m_ula->setKempstonState(4, action != KeyAction::release);
            break;
        }
    }

    void ZXSpectrumEmulator::doClock()
    {
        if (!m_ula->isCpuStalled())
        {
            m_cpu->interruptRequest(m_ula->interruptRequested());
            m_cpu->clock();
        }
        m_ula->clock();
        m_ula->setAudioIn(m_audioIn > AudioInThreshold);
        m_audioOut = m_ula->audioOutput();
        if (m_ula->frameReady())
        {
            updateScreenBuffer();
        }

        if (m_tape)
        {
            if (m_tape->completed())
            {
                m_tape = nullptr;
                m_audioIn = 0.f;
            }
            else
            {
                m_audioIn = m_tape->clock();
            }
        }

        m_clockCounter++;
    }

    uint8_t ZXSpectrumEmulator::vramRead(const uint16_t address) const
    {
        // TODO: allow switching bank for 128K spectrums
        return m_ram[5][address & 0x3fff]; // TODO: should update floating bus value?
    }

    void ZXSpectrumEmulator::updateScreenBuffer()
    {
        std::size_t source = 0;
        std::size_t dest = 0;
        const auto borderBuffer = m_ula->borderBuffer();
        const auto invertPaperInk = m_ula->invertPaperInk();
        for (std::size_t y = 0; y < ScreenHeight + BorderTop + BorderBottom; y++)
        {
            for (std::size_t x = 0; x < ScreenWidth + BorderLeft + BorderRight; x++)
            {
                Color color;
                if (y < BorderTop || y >= ScreenHeight + BorderTop || x < BorderLeft || x >= ScreenWidth + BorderLeft)
                {
                    color = DefaultPalette.map(borderBuffer[source]);
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
                    const bool flash = (attribute & 0x80) && invertPaperInk;
                    color = DefaultPalette.map((pixel && !flash) || (!pixel && flash) ? ink : paper);
                }

                source++;
                m_screenBuffer[dest++] = color.rgba;
            }
        }
    }
}

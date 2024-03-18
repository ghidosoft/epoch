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

#include "ZXSpectrumEmulator.hpp"

#include "Io.hpp"
#include "PulsesTape.hpp"
#include "Roms.hpp"
#include "Ula.hpp"
#include "Z80Cpu.hpp"

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
    const Palette ZXSpectrumEmulator::DefaultPalette{defaultColors};

    ZXSpectrumEmulator::ZXSpectrumEmulator(std::unique_ptr<Ula> ula)
        : Emulator{{Width,
                    Height,
                    TStatesPerFrame,
                    FramesPerSecond,
                    {
                        {"Tapes", ".tap,.tzx", true, false},
                        {"SNA Snapshots", ".sna", true, true},
                        {"Z80 Snapshots", ".z80", true, false},
                    }}},
          m_ula{std::move(ula)},
          m_cpu{std::make_unique<Z80Cpu>(*m_ula)}
    {
    }

    ZXSpectrumEmulator::~ZXSpectrumEmulator() = default;

    std::unique_ptr<ZXSpectrumEmulator> ZXSpectrumEmulator::create48K()
    {
        return std::make_unique<ZXSpectrumEmulator>(std::make_unique<Ula>(UlaType::zx48k, roms::Rom48K));
    }

    std::unique_ptr<ZXSpectrumEmulator> ZXSpectrumEmulator::create128K()
    {
        return std::make_unique<ZXSpectrumEmulator>(std::make_unique<Ula>(UlaType::zx128k, roms::Rom128K));
    }

    std::unique_ptr<ZXSpectrumEmulator> ZXSpectrumEmulator::create128KPlus2()
    {
        return std::make_unique<ZXSpectrumEmulator>(std::make_unique<Ula>(UlaType::zx128k, roms::Rom128KPlus2));
    }

    std::unique_ptr<ZXSpectrumEmulator> ZXSpectrumEmulator::create128KPlus3()
    {
        return std::make_unique<ZXSpectrumEmulator>(std::make_unique<Ula>(UlaType::zx128kplus3, roms::Rom128KPlus3));
    }

    void ZXSpectrumEmulator::reset()
    {
        m_ula->reset();
        m_cpu->reset();
        m_tape = {};
        m_clockCounter = 0;
    }

    void ZXSpectrumEmulator::load(const std::string& path) { m_tape = epoch::zxspectrum::load(path, this); }

    void ZXSpectrumEmulator::save(const std::string& path) { epoch::zxspectrum::save(path, this); }

    void ZXSpectrumEmulator::keyEvent(const Key key, const KeyAction action)
    {
        const auto state = action != KeyAction::release;
        switch (key)
        {
            case Key::LeftShift:
            case Key::RightShift:
                m_ula->setKeyState(0, 0, state);
                break;
            case Key::Z:
                m_ula->setKeyState(0, 1, state);
                break;
            case Key::X:
                m_ula->setKeyState(0, 2, state);
                break;
            case Key::C:
                m_ula->setKeyState(0, 3, state);
                break;
            case Key::V:
                m_ula->setKeyState(0, 4, state);
                break;

            case Key::A:
                m_ula->setKeyState(1, 0, state);
                break;
            case Key::S:
                m_ula->setKeyState(1, 1, state);
                break;
            case Key::D:
                m_ula->setKeyState(1, 2, state);
                break;
            case Key::F:
                m_ula->setKeyState(1, 3, state);
                break;
            case Key::G:
                m_ula->setKeyState(1, 4, state);
                break;

            case Key::Q:
                m_ula->setKeyState(2, 0, state);
                break;
            case Key::W:
                m_ula->setKeyState(2, 1, state);
                break;
            case Key::E:
                m_ula->setKeyState(2, 2, state);
                break;
            case Key::R:
                m_ula->setKeyState(2, 3, state);
                break;
            case Key::T:
                m_ula->setKeyState(2, 4, state);
                break;

            case Key::D1:
                m_ula->setKeyState(3, 0, state);
                break;
            case Key::D2:
                m_ula->setKeyState(3, 1, state);
                break;
            case Key::D3:
                m_ula->setKeyState(3, 2, state);
                break;
            case Key::D4:
                m_ula->setKeyState(3, 3, state);
                break;
            case Key::D5:
                m_ula->setKeyState(3, 4, state);
                break;

            case Key::D6:
                m_ula->setKeyState(4, 4, state);
                break;
            case Key::D7:
                m_ula->setKeyState(4, 3, state);
                break;
            case Key::D8:
                m_ula->setKeyState(4, 2, state);
                break;
            case Key::D9:
                m_ula->setKeyState(4, 1, state);
                break;
            case Key::D0:
                m_ula->setKeyState(4, 0, state);
                break;

            case Key::Y:
                m_ula->setKeyState(5, 4, state);
                break;
            case Key::U:
                m_ula->setKeyState(5, 3, state);
                break;
            case Key::I:
                m_ula->setKeyState(5, 2, state);
                break;
            case Key::O:
                m_ula->setKeyState(5, 1, state);
                break;
            case Key::P:
                m_ula->setKeyState(5, 0, state);
                break;

            case Key::H:
                m_ula->setKeyState(6, 4, state);
                break;
            case Key::J:
                m_ula->setKeyState(6, 3, state);
                break;
            case Key::K:
                m_ula->setKeyState(6, 2, state);
                break;
            case Key::L:
                m_ula->setKeyState(6, 1, state);
                break;
            case Key::Enter:
                m_ula->setKeyState(6, 0, state);
                break;

            case Key::B:
                m_ula->setKeyState(7, 4, state);
                break;
            case Key::N:
                m_ula->setKeyState(7, 3, state);
                break;
            case Key::M:
                m_ula->setKeyState(7, 2, state);
                break;
            case Key::LeftControl:
                m_ula->setKeyState(7, 1, state);
                break;
            case Key::Space:
                m_ula->setKeyState(7, 0, state);
                break;

            case Key::Right:
                m_ula->setKempstonState(0, state);
                break;
            case Key::Left:
                m_ula->setKempstonState(1, state);
                break;
            case Key::Down:
                m_ula->setKempstonState(2, state);
                break;
            case Key::Up:
                m_ula->setKempstonState(3, state);
                break;
            case Key::RightControl:
                m_ula->setKempstonState(4, state);
                break;
        }
    }

    std::array<MemoryBank, 8>& ZXSpectrumEmulator::ram() { return m_ula->ram(); }

    const std::array<MemoryBank, 8>& ZXSpectrumEmulator::ram() const { return m_ula->ram(); }

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
                    const auto pixelData = m_ula->vramRead(pixelAddress);
                    const bool pixel = (pixelData >> (7 - (xPixel & 0b111))) & 0x01;

                    const auto attribute = m_ula->vramRead(0x5800 + ((yPixel >> 3) << 5) + (xPixel >> 3));

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
}  // namespace epoch::zxspectrum

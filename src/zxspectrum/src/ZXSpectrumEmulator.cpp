#include "ZXSpectrumEmulator.h"

#include "Z80Cpu.h"

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
        m_cpu{std::make_unique<Z80Cpu>()}
    {
        for (auto i = 0; i < 768; i++)
            busWrite(0x5800 + i, i % 2 == 0 ? 98 : 32);
    }

    ZXSpectrumEmulator::~ZXSpectrumEmulator() = default;

    void ZXSpectrumEmulator::clock()
    {
        m_cpu->clock();
    }

    void ZXSpectrumEmulator::reset()
    {
        m_cpu->reset();
    }

    uint8_t ZXSpectrumEmulator::busRead(const uint16_t address)
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

    void ZXSpectrumEmulator::busWrite(const uint16_t address, const uint8_t value)
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

    uint8_t ZXSpectrumEmulator::vramRead(uint16_t address)
    {
        // TODO: allow switching bank for 128K spectrums
        return m_ram[5][address & 0x3fff]; // TODO: should update floating bus value?
    }

    std::span<const uint8_t> ZXSpectrumEmulator::screenBuffer()
    {
        std::size_t source = 0;
        std::size_t dest = 0;
        for (auto y = 0; y < ScreenHeight + BorderTop + BorderBottom; y++)
        {
            for (auto x = 0; x < ScreenWidth + BorderLeft + BorderRight; x++)
            {
                Color color;
                if (y < BorderTop || y >= ScreenHeight + BorderTop || x < BorderLeft || x >= ScreenWidth + BorderLeft)
                {
                    color = DefaultPalette.map(m_borderBuffer[source]);
                }
                else
                {
                    const auto attribute = vramRead(0x5800 + (((y - BorderTop) >> 3) << 5) + ((x - BorderLeft) >> 3));
                    // TODO: read vram (pixel)
                    // TODO: flash
                    const auto pixel = x % 2 == 0;
                    auto paper = (attribute >> 3) & 0x07;
                    auto ink = attribute & 0x07;
                    const auto bright = attribute & 0x40;
                    if (bright)
                    {
                        paper += 0x08;
                        ink += 0x08;
                    }
                    color = DefaultPalette.map(pixel ? paper : ink);
                }

                source++;
                m_screenBuffer[dest++] = color.r;
                m_screenBuffer[dest++] = color.g;
                m_screenBuffer[dest++] = color.b;
                m_screenBuffer[dest++] = color.a;
            }
        }
        return m_screenBuffer;
    }
}

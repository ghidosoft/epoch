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

        // TODO: test only
        static uint16_t index = 0;
        for (auto x = 0; x < 16; x++)
            busWrite(0x4000 + index++, 0x55);
        if (index >= 6144) index = 0;
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
                    const auto xPixel = (x - BorderLeft);
                    const auto yPixel = (y - BorderTop);

                    uint16_t pixelAddress = 0x4000;
                    pixelAddress |= (xPixel >> 3) & 0b11111;
                    pixelAddress |= (yPixel & 0b00000111) << 8;
                    pixelAddress |= (yPixel & 0b00111000) << 2;
                    pixelAddress |= (yPixel & 0b11000000) << 5;
                    const auto pixelData = vramRead(pixelAddress);
                    const bool pixel = (pixelData >> (xPixel & 0b111)) & 0x01;

                    const auto attribute = vramRead(0x5800 + ((yPixel >> 3) << 5) + (xPixel >> 3));

                    auto paper = (attribute >> 3) & 0x07;
                    auto ink = attribute & 0x07;
                    const bool bright = attribute & 0x40;
                    if (bright)
                    {
                        paper += 0x08;
                        ink += 0x08;
                    }
                    // TODO: flash
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

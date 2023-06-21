#include "ZXSpectrumEmulator.h"

#include "Z80Cpu.h"

namespace epoch::zxspectrum
{
    static Color defaultColors[] = {
        {0x00, 0x00, 0x00},
        {0x00, 0x00, 0xee},
        {0xee, 0x00, 0x00},
        {0xee, 0x00, 0xee},
        {0x00, 0xee, 0x00},
        {0x00, 0xee, 0xee},
        {0xee, 0xee, 0x00},
        {0xee, 0xee, 0xee},
        {0x00, 0x00, 0x00},
        {0x00, 0x00, 0xff},
        {0xff, 0x00, 0x00},
        {0xff, 0x00, 0xff},
        {0x00, 0xff, 0x00},
        {0x00, 0xff, 0xff},
        {0xff, 0xff, 0x00},
        {0xff, 0xff, 0xff},
    };
    const Palette ZXSpectrumEmulator::DefaultPalette{ defaultColors };

    ZXSpectrumEmulator::ZXSpectrumEmulator() :
        Emulator{ {"ZX Spectrum", ScreenWidth + BorderLeft + BorderRight, ScreenHeight + BorderTop + BorderBottom} },
        m_cpu{std::make_unique<Z80Cpu>()}
    {
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
}

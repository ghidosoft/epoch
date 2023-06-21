#include "ZXSpectrumEmulator.h"

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
}

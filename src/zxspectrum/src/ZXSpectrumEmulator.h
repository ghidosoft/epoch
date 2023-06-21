#ifndef SRC_EPOCH_ZXSPECTRUM_ZXSPECTRUMEMULATOR_H_
#define SRC_EPOCH_ZXSPECTRUM_ZXSPECTRUMEMULATOR_H_

#include <array>

#include <epoch/core.h>

namespace epoch::zxspectrum
{
    class ZXSpectrumEmulator : public Emulator
    {
    public:
        static constexpr int ScreenWidth = 256;
        static constexpr int ScreenHeight = 192;
        static constexpr int BorderLeft = 48;
        static constexpr int BorderRight = 48;
        static constexpr int BorderTop = 48;
        static constexpr int BorderBottom = 56;

        static const Palette DefaultPalette;

    private:
        using MemoryBank = std::array<uint8_t, 0x4000>;
        MemoryBank m_rom48k{};
        // MemoryBank m_rom128k{};
        std::array<MemoryBank, 10> m_ram{};

        std::array<uint8_t, (ScreenWidth + BorderLeft + BorderRight) * (ScreenHeight + BorderTop + BorderBottom)> m_screenBuffer{};
    };
}

#endif

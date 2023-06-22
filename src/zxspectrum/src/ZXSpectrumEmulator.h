#ifndef SRC_EPOCH_ZXSPECTRUM_ZXSPECTRUMEMULATOR_H_
#define SRC_EPOCH_ZXSPECTRUM_ZXSPECTRUMEMULATOR_H_

#include <array>
#include <memory>

#include <epoch/core.h>

namespace epoch::zxspectrum
{
    class Z80Cpu;

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

    public:
        ZXSpectrumEmulator();
        ~ZXSpectrumEmulator() override;

    public:
        void clock() override;
        void reset() override;

        uint8_t busRead(uint16_t address);
        void busWrite(uint16_t address, uint8_t value);
        uint8_t vramRead(uint16_t address);

        [[nodiscard]] std::span<const uint8_t> screenBuffer() override;

    private:
        const std::unique_ptr<Z80Cpu> m_cpu;

        using MemoryBank = std::array<uint8_t, 0x4000>;
        MemoryBank m_rom48k{};
        std::array<MemoryBank, 8> m_ram{};

        std::array<uint8_t, (ScreenWidth + BorderLeft + BorderRight) * (ScreenHeight + BorderTop + BorderBottom)> m_borderBuffer{};
        std::array<uint8_t, (ScreenWidth + BorderLeft + BorderRight) * (ScreenHeight + BorderTop + BorderBottom) * 4> m_screenBuffer{};

        uint8_t m_floatingBusValue{};
    };
}

#endif

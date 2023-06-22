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
        static constexpr std::size_t ScreenWidth = 256;
        static constexpr std::size_t ScreenHeight = 192;
        static constexpr std::size_t BorderLeft = 48;
        static constexpr std::size_t BorderRight = 48;
        static constexpr std::size_t BorderTop = 48;
        static constexpr std::size_t BorderBottom = 56;

        static const Palette DefaultPalette;

    public:
        ZXSpectrumEmulator();
        ~ZXSpectrumEmulator() override;

    public:
        ZXSpectrumEmulator(const ZXSpectrumEmulator& other) = delete;
        ZXSpectrumEmulator(ZXSpectrumEmulator&& other) noexcept = delete;
        ZXSpectrumEmulator& operator=(const ZXSpectrumEmulator& other) = delete;
        ZXSpectrumEmulator& operator=(ZXSpectrumEmulator&& other) noexcept = delete;

    public:
        void clock() override;
        void reset() override;

        uint8_t busRead(uint16_t address);
        void busWrite(uint16_t address, uint8_t value);
        [[nodiscard]] uint8_t vramRead(uint16_t address) const;

        [[nodiscard]] std::span<const uint32_t> screenBuffer() override;

    private:
        const std::unique_ptr<Z80Cpu> m_cpu;

        using MemoryBank = std::array<uint8_t, 0x4000>;
        MemoryBank m_rom48k{};
        std::array<MemoryBank, 8> m_ram{};

        std::array<uint8_t, (ScreenWidth + BorderLeft + BorderRight) * (ScreenHeight + BorderTop + BorderBottom)> m_borderBuffer{};
        std::array<uint32_t, (ScreenWidth + BorderLeft + BorderRight) * (ScreenHeight + BorderTop + BorderBottom)> m_screenBuffer{};

        uint8_t m_floatingBusValue{};
    };
}

#endif

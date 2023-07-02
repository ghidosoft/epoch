#include <gtest/gtest.h>

#include <array>

#include "../../src/zxspectrum/src/Z80Cpu.h"

namespace epoch::zxspectrum
{
    class TestZ80Interface : public Z80Interface
    {
    public:
        uint8_t read(const uint16_t address) override
        {
            return m_ram[address];
        }
        void write(const uint16_t address, const uint8_t value) override
        {
            m_ram[address] = value;
        }

    private:
        std::array<uint8_t, 0xffff> m_ram{};
    };

    TEST(Z80Cpu, ResetBehavior) {
        TestZ80Interface ram{};
        Z80Cpu sut{ ram };
        sut.reset();
        EXPECT_EQ(sut.registers().pc, 0);
    }

}

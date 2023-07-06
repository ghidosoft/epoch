#include <gtest/gtest.h>

#include <array>
#include <cstring>
#include <span>

#include "../../src/zxspectrum/src/Z80Cpu.h"

namespace epoch::zxspectrum
{
    class TestZ80Interface : public Z80Interface
    {
    public:
        TestZ80Interface() = default;
        TestZ80Interface(std::span<const uint8_t> initRam)
        {
            assert(initRam.size_bytes() <= m_ram.size());
            std::memcpy(m_ram.data(), initRam.data(), initRam.size_bytes());
        }

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
        TestZ80Interface bus{};
        Z80Cpu sut{ bus };
        sut.reset();
        EXPECT_EQ(sut.registers().pc, 0);
    }

    TEST(Z80Cpu, Opcode01xxxxxx_LD_C_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x48 } };
        Z80Cpu sut{ bus };
        sut.registers().bc.value = 0x00aa;
        sut.clock();
        sut.clock();
        sut.clock();
        sut.clock();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().bc.value, 0xaa00);
    }

}

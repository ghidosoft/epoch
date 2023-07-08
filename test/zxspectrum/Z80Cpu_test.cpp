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

        [[nodiscard]] std::span<const uint8_t> ram() const { return m_ram; }

    private:
        std::array<uint8_t, 0xffff> m_ram{};
    };

    TEST(Z80Cpu, ResetBehavior) {
        TestZ80Interface bus{};
        Z80Cpu sut{ bus };
        sut.reset();
        EXPECT_EQ(sut.registers().pc, 0);
        EXPECT_EQ(sut.registers().ir, 0);
    }

    TEST(Z80Cpu, Opcode01xxxxxx_LD_C_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x48 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0xaa00;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().bc, 0xaaaa);
    }

    TEST(Z80Cpu, Opcode01xxxxxx_LD_C_B_LD_H_C) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x48, 0x61 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0xaa00;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().bc, 0xaaaa);
        EXPECT_EQ(sut.registers().hl, 0x0000);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().bc, 0xaaaa);
        EXPECT_EQ(sut.registers().hl, 0xaa00);
    }

    TEST(Z80Cpu, Opcode01xxxxxx_LD_HL_E) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x73 } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0xbbaa;
        sut.registers().hl = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().de, 0xbbaa);
        EXPECT_EQ(bus.ram()[0x1234], 0xaa);
    }

    TEST(Z80Cpu, Opcode01xxxxxx_HALT) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x76, 0x48 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0xaa00;
        sut.step();
        sut.clock();
        sut.clock();
        sut.clock();
        sut.clock();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().bc, 0xaa00);
    }

    TEST(Z80Cpu, Opcode10xxxxxx_ADD_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x80, 0x80 } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0x04);
        sut.registers().bc = 0x8000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x84);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().af.high(), 0x04);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode10xxxxxx_ADD_L) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x85 } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0x00);
        sut.registers().hl = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x00);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_TRUE(sut.registers().af.z());
    }
}

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

        uint8_t ioRead(uint8_t port) override { return 0; }
        void ioWrite(uint8_t port, uint8_t value) override {}

        [[nodiscard]] std::span<uint8_t> ram() { return m_ram; }
        [[nodiscard]] uint8_t ram(const uint16_t address) const { return m_ram[address]; }

    private:
        std::array<uint8_t, 0x10000> m_ram{};
    };

    TEST(Z80Cpu, ResetBehavior) {
        TestZ80Interface bus{};
        Z80Cpu sut{ bus };
        EXPECT_EQ(sut.registers().pc, 0);
        EXPECT_EQ(sut.registers().ir, 0);
        EXPECT_EQ(sut.registers().af, 0xffff);
        EXPECT_EQ(sut.registers().bc, 0xffff);
        EXPECT_EQ(sut.registers().de, 0xffff);
        EXPECT_EQ(sut.registers().hl, 0xffff);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().ix, 0xffff);
        EXPECT_EQ(sut.registers().iy, 0xffff);
        EXPECT_EQ(sut.registers().af2, 0xffff);
        EXPECT_EQ(sut.registers().bc2, 0xffff);
        EXPECT_EQ(sut.registers().de2, 0xffff);
        EXPECT_EQ(sut.registers().hl2, 0xffff);
        EXPECT_EQ(sut.registers().interruptMode, 0x00);
        EXPECT_TRUE(sut.registers().iff1);
        EXPECT_TRUE(sut.registers().iff2);
        EXPECT_FALSE(sut.registers().interruptJustEnabled);
        std::memset(&sut.registers(), 0xcc, sizeof(Z80Registers));
        sut.reset();
        EXPECT_EQ(sut.registers().pc, 0);
        EXPECT_EQ(sut.registers().ir, 0);
        EXPECT_EQ(sut.registers().af, 0xffff);
        EXPECT_EQ(sut.registers().bc, 0xffff);
        EXPECT_EQ(sut.registers().de, 0xffff);
        EXPECT_EQ(sut.registers().hl, 0xffff);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().ix, 0xffff);
        EXPECT_EQ(sut.registers().iy, 0xffff);
        EXPECT_EQ(sut.registers().af2, 0xffff);
        EXPECT_EQ(sut.registers().bc2, 0xffff);
        EXPECT_EQ(sut.registers().de2, 0xffff);
        EXPECT_EQ(sut.registers().hl2, 0xffff);
        EXPECT_EQ(sut.registers().interruptMode, 0x00);
        EXPECT_TRUE(sut.registers().iff1);
        EXPECT_TRUE(sut.registers().iff2);
        EXPECT_FALSE(sut.registers().interruptJustEnabled);
    }

    TEST(Z80Cpu, Opcode00000000_NOP) {
        TestZ80Interface bus{};
        Z80Cpu sut{ bus };
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        sut.step();
        sut.step();
        sut.step();
        sut.step();
        sut.step();
        sut.step();
        sut.step();

        EXPECT_EQ(sut.registers().pc, 8);
        EXPECT_EQ(sut.registers().ir, 8);
    }

    TEST(Z80Cpu, Opcode00001000_EX_AF_AF2) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x08 } };
        Z80Cpu sut{ bus };
        sut.registers().af.value = 0x1234;
        sut.registers().af2.value = 0x5678;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af, 0x5678);
        EXPECT_EQ(sut.registers().af2, 0x1234);
    }

    TEST(Z80Cpu, Opcode00000001_LD_BC_nn) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x01, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().bc, 0x1234);
    }

    TEST(Z80Cpu, Opcode00010001_ADD_HL_BC) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x09 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0xabcd;
        sut.registers().hl = 0x5678;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().bc, 0xabcd);
        EXPECT_EQ(sut.registers().hl, 0x0245);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode00010001_LD_DE_nn) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x11, 0x00, 0x40 } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().de, 0x4000);
    }

    TEST(Z80Cpu, Opcode00011001_ADD_HL_DE) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x19 } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0x0101;
        sut.registers().hl = 0x5678;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().de, 0x0101);
        EXPECT_EQ(sut.registers().hl, 0x5779);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode00100001_LD_HL_nn) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x21, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().hl = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().hl, 0x1234);
    }

    TEST(Z80Cpu, Opcode00101001_ADD_HL_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x29 } };
        Z80Cpu sut{ bus };
        sut.registers().hl = 0x5678;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().hl, 0xacf0);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode00110001_LD_SP_nn) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x31, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().sp = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().sp, 0x1234);
    }

    TEST(Z80Cpu, Opcode00111001_ADD_HL_SP) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x39 } };
        Z80Cpu sut{ bus };
        sut.registers().sp = 0xfedc;
        sut.registers().hl = 0x5678;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().sp, 0xfedc);
        EXPECT_EQ(sut.registers().hl, 0x5554);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode00000010_LD_BC_A) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x02 } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0x78);
        sut.registers().bc = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(bus.ram()[0x1234], 0x78);
    }

    TEST(Z80Cpu, Opcode00001010_LD_A_BC) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x0a } };
        bus.ram()[0x1234] = 0x78;
        Z80Cpu sut{ bus };
        sut.registers().af.high(0x56);
        sut.registers().bc = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x78);
    }

    TEST(Z80Cpu, Opcode00010010_LD_DE_A) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0x78);
        sut.registers().de = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(bus.ram()[0x1234], 0x78);
    }

    TEST(Z80Cpu, Opcode00011010_LD_A_DE) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x1a } };
        bus.ram()[0x1234] = 0x78;
        Z80Cpu sut{ bus };
        sut.registers().af.high(0x56);
        sut.registers().de = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x78);
    }

    TEST(Z80Cpu, Opcode00100010_LD_mnn_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x22, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().hl = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(bus.ram()[0x1234], 0xcd);
        EXPECT_EQ(bus.ram()[0x1235], 0xab);
    }

    TEST(Z80Cpu, Opcode00101010_LD_HL_mnn) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x2a, 0x34, 0x12 } };
        bus.ram()[0x1234] = 0xcd;
        bus.ram()[0x1235] = 0xab;
        Z80Cpu sut{ bus };
        sut.registers().hl = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().hl, 0xabcd);
        EXPECT_EQ(bus.ram()[0x1234], 0xcd);
        EXPECT_EQ(bus.ram()[0x1235], 0xab);
    }

    TEST(Z80Cpu, Opcode00110010_LD_mnn_A) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x32, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0xab);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(bus.ram()[0x1234], 0xab);
        EXPECT_EQ(bus.ram()[0x1235], 0x00);
    }

    TEST(Z80Cpu, Opcode00111010_LD_A_mnn) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x3a, 0x34, 0x12 } };
        bus.ram()[0x1234] = 0xcd;
        bus.ram()[0x1235] = 0xab;
        Z80Cpu sut{ bus };
        sut.registers().af.high(0);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0xcd);
        EXPECT_EQ(bus.ram()[0x1234], 0xcd);
        EXPECT_EQ(bus.ram()[0x1235], 0xab);
    }

    TEST(Z80Cpu, Opcode00000011_INC_BC) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x03 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().bc, 0x0001);
    }

    TEST(Z80Cpu, Opcode00001011_DEC_BC) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x0b } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().bc, 0xffff);
    }

    TEST(Z80Cpu, Opcode00010011_INC_DE) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x13 } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().de, 0x0001);
    }

    TEST(Z80Cpu, Opcode00011011_DEC_DE) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x1b } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().de, 0xffff);
    }

    TEST(Z80Cpu, Opcode00100011_INC_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x23 } };
        Z80Cpu sut{ bus };
        sut.registers().hl = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().hl, 0x0001);
    }

    TEST(Z80Cpu, Opcode00101011_DEC_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x2b } };
        Z80Cpu sut{ bus };
        sut.registers().hl = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().hl, 0xffff);
    }

    TEST(Z80Cpu, Opcode00110011_INC_SP) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x33 } };
        Z80Cpu sut{ bus };
        sut.registers().sp = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().sp, 0x0001);
    }

    TEST(Z80Cpu, Opcode00111011_DEC_SP) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x3b } };
        Z80Cpu sut{ bus };
        sut.registers().sp = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().sp, 0xffff);
    }

    TEST(Z80Cpu, Opcode00xxx100_INC_D) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x14, 0x14 } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0xfecc;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().de, 0xffcc);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().de, 0x00cc);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_TRUE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_TRUE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode00xxx101_DEC_D) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x15, 0x15 } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0x01cc;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().de, 0x00cc);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_TRUE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().de, 0xffcc);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_TRUE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode00xxxxxx_LD_B_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x06, 0x08 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0x33cc;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().bc, 0x08cc);
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

    TEST(Z80Cpu, Opcode00000111_RLCA) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x07 } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0b11110000);
        sut.registers().af.low(0x00);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0b11100001);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode00001111_RRCA) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x0f } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0b00001111);
        sut.registers().af.low(0x00);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0b10000111);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode00010111_RLA) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x17 } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0b10100001);
        sut.registers().af.low(0x00);
        sut.registers().af.c(true);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0b01000011);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode00011111_RRA) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x1f } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0b00001111);
        sut.registers().af.low(0x00);
        sut.registers().af.c(true);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0b10000111);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode00100111_DAA) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x27 } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0xd5);
        sut.registers().af.low(0x00);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x35);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode00101111_CPL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x2f } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0b01010101);
        sut.registers().af.low(0x00);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0b10101010);
        EXPECT_TRUE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode00110111_SCF) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x37 } };
        Z80Cpu sut{ bus };
        sut.registers().af.low(0x00);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode00111111_CCF) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x3f } };
        Z80Cpu sut{ bus };
        sut.registers().af.low(0x00);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_TRUE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu, Opcode01xxxxxx_LD_C_B_LD_H_C) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x48, 0x61 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0xaa00;
        sut.registers().hl = 0x0000;
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
        EXPECT_EQ(sut.registers().pc, 0);
        EXPECT_EQ(sut.registers().ir, 1);
        sut.clock();
        sut.clock();
        sut.clock();
        sut.clock();
        EXPECT_EQ(sut.registers().pc, 0);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().bc, 0xaa00);
    }

    TEST(Z80Cpu, Opcode10xxxxxx_ADD_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x80, 0x80 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0x04);
        sut.registers().bc = 0x8000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x84);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().af.high(), 0x04);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode10000xxx_ADD_L) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x85 } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0xff);
        sut.registers().hl = 0x0001;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x00);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_TRUE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_TRUE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode10001xxx_ADC_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x88, 0x88 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0x04);
        sut.registers().bc = 0x8000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x85);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().af.high(), 0x05);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode10010xxx_SUB_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x90, 0x90 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0x03);
        sut.registers().bc = 0x6000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0xa3);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().af.high(), 0x43);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode10011xxx_SBC_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0x98, 0x98 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0x04);
        sut.registers().bc = 0x8000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x83);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().af.high(), 0x02);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode10100xxx_AND_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xa0 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0xa5);
        sut.registers().bc = 0xcc00;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x84);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_TRUE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode10101xxx_XOR_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xa8 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0xa5);
        sut.registers().bc = 0xcc00;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x69);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode10110xxx_OR_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xb0 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0xa5);
        sut.registers().bc = 0xcc00;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0xed);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode10111xxx_CP_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xb8 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0x03);
        sut.registers().bc = 0x6000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x03);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode11000011_JP_nn) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xc3, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0x1234);
        EXPECT_EQ(sut.registers().ir, 1);
    }

    TEST(Z80Cpu, Opcode11100011_EX_mSP_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xe3 } };
        bus.ram()[0x1234] = 0x56;
        bus.ram()[0x1235] = 0x78;
        Z80Cpu sut{ bus };
        sut.registers().sp = 0x1234;
        sut.registers().hl = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().sp, 0x1234);
        EXPECT_EQ(sut.registers().hl, 0x7856);
        EXPECT_EQ(bus.ram()[0x1234], 0xcd);
        EXPECT_EQ(bus.ram()[0x1235], 0xab);
    }

    TEST(Z80Cpu, Opcode11101011_EX_DE_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xeb } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0x1234;
        sut.registers().hl = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().de, 0xabcd);
        EXPECT_EQ(sut.registers().hl, 0x1234);
    }

    TEST(Z80Cpu, Opcode11110011_DI) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xf3 } };
        Z80Cpu sut{ bus };
        sut.registers().iff1 = true;
        sut.registers().iff2 = true;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_FALSE(sut.registers().iff1);
        EXPECT_FALSE(sut.registers().iff2);
        EXPECT_FALSE(sut.registers().interruptJustEnabled);
    }

    TEST(Z80Cpu, Opcode11111011_EI) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfb, 0x00 } };
        Z80Cpu sut{ bus };
        sut.registers().iff1 = false;
        sut.registers().iff2 = false;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_TRUE(sut.registers().iff1);
        EXPECT_TRUE(sut.registers().iff2);
        EXPECT_TRUE(sut.registers().interruptJustEnabled);
        sut.step();
        EXPECT_TRUE(sut.registers().iff1);
        EXPECT_TRUE(sut.registers().iff2);
        EXPECT_FALSE(sut.registers().interruptJustEnabled);
    }

    TEST(Z80Cpu, Opcode11xxx000_PUSH_BC_POP_BC) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xc5, 0xc1 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().sp, 0xfffd);
        EXPECT_EQ(sut.registers().bc, 0x1234);
        EXPECT_EQ(bus.ram(0xffff), 0x00);
        EXPECT_EQ(bus.ram(0xfffe), 0x12);
        EXPECT_EQ(bus.ram(0xfffd), 0x34);
        sut.registers().bc = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().bc, 0x1234);
        EXPECT_EQ(bus.ram(0xffff), 0x00);
        EXPECT_EQ(bus.ram(0xfffe), 0x12);
        EXPECT_EQ(bus.ram(0xfffd), 0x34);
    }

    TEST(Z80Cpu, Opcode11xxx010_PUSH_DE_POP_DE) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xd5, 0xd1 } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().sp, 0xfffd);
        EXPECT_EQ(sut.registers().de, 0x1234);
        EXPECT_EQ(bus.ram(0xffff), 0x00);
        EXPECT_EQ(bus.ram(0xfffe), 0x12);
        EXPECT_EQ(bus.ram(0xfffd), 0x34);
        sut.registers().de = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().de, 0x1234);
        EXPECT_EQ(bus.ram(0xffff), 0x00);
        EXPECT_EQ(bus.ram(0xfffe), 0x12);
        EXPECT_EQ(bus.ram(0xfffd), 0x34);
    }

    TEST(Z80Cpu, Opcode11011001_EXX) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xd9 } };
        Z80Cpu sut{ bus };
        sut.registers().af = 0x1234;
        sut.registers().bc = 0x5678;
        sut.registers().de = 0x9abc;
        sut.registers().hl = 0xdef0;
        sut.registers().af2 = 0x0fed;
        sut.registers().bc2 = 0xcba9;
        sut.registers().de2 = 0x8765;
        sut.registers().hl2 = 0x4321;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af, 0x1234);
        EXPECT_EQ(sut.registers().bc, 0xcba9);
        EXPECT_EQ(sut.registers().de, 0x8765);
        EXPECT_EQ(sut.registers().hl, 0x4321);
    }

    TEST(Z80Cpu, Opcode11xxx100_PUSH_HL_POP_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xe5, 0xe1 } };
        Z80Cpu sut{ bus };
        sut.registers().hl = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().sp, 0xfffd);
        EXPECT_EQ(sut.registers().hl, 0x1234);
        EXPECT_EQ(bus.ram(0xffff), 0x00);
        EXPECT_EQ(bus.ram(0xfffe), 0x12);
        EXPECT_EQ(bus.ram(0xfffd), 0x34);
        sut.registers().hl = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().hl, 0x1234);
        EXPECT_EQ(bus.ram(0xffff), 0x00);
        EXPECT_EQ(bus.ram(0xfffe), 0x12);
        EXPECT_EQ(bus.ram(0xfffd), 0x34);
    }

    TEST(Z80Cpu, Opcode11011001_JP_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xe9 } };
        Z80Cpu sut{ bus };
        sut.registers().hl = 0xdef0;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0xdef0);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().hl, 0xdef0);
    }

    TEST(Z80Cpu, Opcode11xxx110_PUSH_AF_POP_AF) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xf5, 0xf1 } };
        Z80Cpu sut{ bus };
        sut.registers().af = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().sp, 0xfffd);
        EXPECT_EQ(sut.registers().af, 0x1234);
        EXPECT_EQ(bus.ram(0xffff), 0x00);
        EXPECT_EQ(bus.ram(0xfffe), 0x12);
        EXPECT_EQ(bus.ram(0xfffd), 0x34);
        sut.registers().af = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().af, 0x1234);
        EXPECT_EQ(bus.ram(0xffff), 0x00);
        EXPECT_EQ(bus.ram(0xfffe), 0x12);
        EXPECT_EQ(bus.ram(0xfffd), 0x34);
    }

    TEST(Z80Cpu, Opcode11111001_LD_SP_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xf9 } };
        Z80Cpu sut{ bus };
        sut.registers().hl = 0xdef0;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 1);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().hl, 0xdef0);
        EXPECT_EQ(sut.registers().sp, 0xdef0);
    }

    TEST(Z80Cpu, Opcode11000110_ADD_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xc6, 0x80, 0xc6, 0x80 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0x04);
        sut.registers().bc = 0x8000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x84);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().af.high(), 0x04);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode11001110_ADC_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xce, 0x80, 0xce, 0x80 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0x04);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x85);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().af.high(), 0x05);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode11010110_SUB_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xd6, 0x60, 0xd6, 0x60 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0x03);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0xa3);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().af.high(), 0x43);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode11011110_SBC_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xde, 0x80, 0xde, 0x80 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0x04);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x83);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        sut.step();
        EXPECT_EQ(sut.registers().af.high(), 0x02);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode11100110_AND_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xe6, 0xcc } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0xa5);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x84);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_TRUE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode11101110_XOR_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xee, 0xcc } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0xa5);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x69);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode11110110_OR_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xf6, 0xcc } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0xa5);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0xed);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu, Opcode11111110_CP_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfe, 0x60 } };
        Z80Cpu sut{ bus };
        sut.registers().af.c(true);
        sut.registers().af.high(0x03);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 1);
        EXPECT_EQ(sut.registers().af.high(), 0x03);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }
}

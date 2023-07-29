/* This file is part of Epoch, Copyright (C) 2023 Andrea Ghidini.
 *
 * Epoch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Epoch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Epoch.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "../../src/zxspectrum/src/Z80Cpu.h"
#include "TestZ80Interface.h"

namespace epoch::zxspectrum
{
    TEST(Z80Cpu_ED, Opcode_NOP) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x00 } };
        Z80Cpu sut{ bus };
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 8);
    }

    TEST(Z80Cpu_ED, Opcode_01000011_LD_mNN_BC) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x43, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 20);
        EXPECT_EQ(sut.registers().bc, 0xabcd);
        EXPECT_EQ(bus.ram(0x1234), 0xcd);
        EXPECT_EQ(bus.ram(0x1235), 0xab);
    }

    TEST(Z80Cpu_ED, Opcode_01xxx100_NEG) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x44, 0xed, 0x44 } };
        Z80Cpu sut{ bus };
        sut.registers().af = 0xabff;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 8);
        EXPECT_EQ(sut.registers().af, 0x5513);
        sut.step();
        EXPECT_EQ(sut.registers().af, 0xabbb);
    }

    TEST(Z80Cpu_ED, Opcode_01001010_ADC_HL_BC) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x4a, 0xed, 0x4a } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0x5678;
        sut.registers().hl = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 15);
        EXPECT_EQ(sut.registers().bc, 0x5678);
        EXPECT_EQ(sut.registers().hl, 0x68ad);
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.c());
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 4);
        EXPECT_EQ(sut.registers().bc, 0x5678);
        EXPECT_EQ(sut.registers().hl, 0xbf25);
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.c());
    }

    TEST(Z80Cpu_ED, Opcode_01001011_LD_BC_mNN) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x4b, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        bus.ram()[0x1234] = 0x78;
        bus.ram()[0x1235] = 0x56;
        sut.registers().bc = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 20);
        EXPECT_EQ(sut.registers().bc, 0x5678);
        EXPECT_EQ(bus.ram(0x1234), 0x78);
        EXPECT_EQ(bus.ram(0x1235), 0x56);
    }

    TEST(Z80Cpu_ED, Opcode_01010011_LD_mNN_DE) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x53, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 20);
        EXPECT_EQ(sut.registers().de, 0xabcd);
        EXPECT_EQ(bus.ram(0x1234), 0xcd);
        EXPECT_EQ(bus.ram(0x1235), 0xab);
    }

    TEST(Z80Cpu_ED, Opcode_01011011_LD_DE_mNN) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x5b, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        bus.ram()[0x1234] = 0x78;
        bus.ram()[0x1235] = 0x56;
        sut.registers().de = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 20);
        EXPECT_EQ(sut.registers().de, 0x5678);
        EXPECT_EQ(bus.ram(0x1234), 0x78);
        EXPECT_EQ(bus.ram(0x1235), 0x56);
    }

    TEST(Z80Cpu_ED, Opcode_01100011_LD_mNN_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x63, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().hl = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 20);
        EXPECT_EQ(sut.registers().hl, 0xabcd);
        EXPECT_EQ(bus.ram(0x1234), 0xcd);
        EXPECT_EQ(bus.ram(0x1235), 0xab);
    }

    TEST(Z80Cpu_ED, Opcode_01101011_LD_HL_mNN) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x6b, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        bus.ram()[0x1234] = 0x78;
        bus.ram()[0x1235] = 0x56;
        sut.registers().hl = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 20);
        EXPECT_EQ(sut.registers().hl, 0x5678);
        EXPECT_EQ(bus.ram(0x1234), 0x78);
        EXPECT_EQ(bus.ram(0x1235), 0x56);
    }

    TEST(Z80Cpu_ED, Opcode_01110011_LD_mNN_SP) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x73, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().sp = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 20);
        EXPECT_EQ(sut.registers().sp, 0xabcd);
        EXPECT_EQ(bus.ram(0x1234), 0xcd);
        EXPECT_EQ(bus.ram(0x1235), 0xab);
    }

    TEST(Z80Cpu_ED, Opcode_01111011_LD_SP_mNN) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x7b, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        bus.ram()[0x1234] = 0x78;
        bus.ram()[0x1235] = 0x56;
        sut.registers().sp = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 20);
        EXPECT_EQ(sut.registers().sp, 0x5678);
        EXPECT_EQ(bus.ram(0x1234), 0x78);
        EXPECT_EQ(bus.ram(0x1235), 0x56);
    }

    TEST(Z80Cpu_ED, Opcode_01100111_RRD) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x67, 0xed, 0x67, 0xed, 0x67 } };
        Z80Cpu sut{ bus };
        bus.ram()[0x0100] = 0xab;
        bus.ram()[0x0101] = 0x56;
        sut.registers().af = 0x12ff;
        sut.registers().hl = 0x0100;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 18);
        EXPECT_EQ(sut.registers().af, 0x1b0d);
        EXPECT_EQ(sut.registers().hl, 0x0100);
        EXPECT_EQ(bus.ram(0x0100), 0x2a);
        EXPECT_EQ(bus.ram(0x0101), 0x56);
        sut.step();
        EXPECT_EQ(sut.registers().af, 0x1a09);
        EXPECT_EQ(bus.ram(0x0100), 0xb2);
        EXPECT_EQ(bus.ram(0x0101), 0x56);
        sut.step();
        EXPECT_EQ(sut.registers().af, 0x1205);
        EXPECT_EQ(bus.ram(0x0100), 0xab);
        EXPECT_EQ(bus.ram(0x0101), 0x56);
    }

    TEST(Z80Cpu_ED, Opcode_01101111_RLD) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x6f, 0xed, 0x6f, 0xed, 0x6f } };
        Z80Cpu sut{ bus };
        bus.ram()[0x0100] = 0xab;
        bus.ram()[0x0101] = 0x56;
        sut.registers().af = 0x12ff;
        sut.registers().hl = 0x0100;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 18);
        EXPECT_EQ(sut.registers().af, 0x1a09);
        EXPECT_EQ(sut.registers().hl, 0x0100);
        EXPECT_EQ(bus.ram(0x0100), 0xb2);
        EXPECT_EQ(bus.ram(0x0101), 0x56);
        sut.step();
        EXPECT_EQ(sut.registers().af, 0x1b0d);
        EXPECT_EQ(bus.ram(0x0100), 0x2a);
        EXPECT_EQ(bus.ram(0x0101), 0x56);
        sut.step();
        EXPECT_EQ(sut.registers().af, 0x1205);
        EXPECT_EQ(bus.ram(0x0100), 0xab);
        EXPECT_EQ(bus.ram(0x0101), 0x56);
    }

    TEST(Z80Cpu_ED, Opcode_01x00110_IM_0) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x46 } };
        Z80Cpu sut{ bus };
        sut.registers().interruptMode = 1;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 8);
        EXPECT_EQ(sut.registers().interruptMode, 0);
    }

    TEST(Z80Cpu_ED, Opcode_01x10110_IM_1) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x56 } };
        Z80Cpu sut{ bus };
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 8);
        EXPECT_EQ(sut.registers().interruptMode, 1);
    }

    TEST(Z80Cpu_ED, Opcode_01x11110_IM_2) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x5e } };
        Z80Cpu sut{ bus };
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 8);
        EXPECT_EQ(sut.registers().interruptMode, 2);
    }

    TEST(Z80Cpu_ED, Opcode_010x1111_LD_A_R_LD_R_A) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x4f, 0xed, 0x5f } };
        Z80Cpu sut{ bus };
        sut.registers().af = 0xaaff;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 0x00aa);
        EXPECT_EQ(sut.clockCounter(), 9);
        EXPECT_EQ(sut.registers().af, 0xaaff);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 0x00ac);
        EXPECT_EQ(sut.clockCounter(), 18);
        EXPECT_EQ(sut.registers().af, 0xaca9);
    }

    TEST(Z80Cpu_ED, Opcode_10100001_CPI) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0xa1, 0xed, 0xa1 } };
        Z80Cpu sut{ bus };
        bus.ram()[0x0100] = 0x38;
        bus.ram()[0x0101] = 0xf1;
        sut.registers().af = 0xc8ff;
        sut.registers().bc = 0x0002;
        sut.registers().hl = 0x0100;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 16);
        EXPECT_EQ(sut.registers().af, 0xc887);
        EXPECT_EQ(sut.registers().bc, 0x0001);
        EXPECT_EQ(sut.registers().hl, 0x0101);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 4);
        EXPECT_EQ(sut.registers().af, 0xc8a3);
        EXPECT_EQ(sut.registers().bc, 0x0000);
        EXPECT_EQ(sut.registers().hl, 0x0102);
        EXPECT_EQ(bus.ram(0x0100), 0x38);
        EXPECT_EQ(bus.ram(0x0101), 0xf1);
    }

    TEST(Z80Cpu_ED, Opcode_10101001_CPD) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0xa9, 0xed, 0xa9 } };
        Z80Cpu sut{ bus };
        bus.ram()[0x0100] = 0x38;
        bus.ram()[0x0101] = 0xf1;
        sut.registers().af = 0xc8ff;
        sut.registers().bc = 0x0002;
        sut.registers().hl = 0x0101;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 16);
        EXPECT_EQ(sut.registers().af, 0xc8a7);
        EXPECT_EQ(sut.registers().bc, 0x0001);
        EXPECT_EQ(sut.registers().hl, 0x0100);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 4);
        EXPECT_EQ(sut.registers().af, 0xc883);
        EXPECT_EQ(sut.registers().bc, 0x0000);
        EXPECT_EQ(sut.registers().hl, 0x00ff);
        EXPECT_EQ(bus.ram(0x0100), 0x38);
        EXPECT_EQ(bus.ram(0x0101), 0xf1);
    }

    TEST(Z80Cpu_ED, Opcode_10110001_CPIR) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0xb1 } };
        Z80Cpu sut{ bus };
        bus.ram()[0x0100] = 0x38;
        bus.ram()[0x0101] = 0xf1;
        sut.registers().af = 0xc8ff;
        sut.registers().bc = 0x0002;
        sut.registers().hl = 0x0100;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 21);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 4);
        EXPECT_EQ(sut.clockCounter(), 21+16);
        EXPECT_EQ(sut.registers().af, 0xc8a3);
        EXPECT_EQ(sut.registers().bc, 0x0000);
        EXPECT_EQ(sut.registers().hl, 0x0102);
        EXPECT_EQ(bus.ram(0x0100), 0x38);
        EXPECT_EQ(bus.ram(0x0101), 0xf1);
        sut.step(); // execute additional NOP
        EXPECT_EQ(sut.registers().pc, 3);
    }

    TEST(Z80Cpu_ED, Opcode_10111001_CPDR) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0xb9 } };
        Z80Cpu sut{ bus };
        bus.ram()[0x0100] = 0x38;
        bus.ram()[0x0101] = 0xf0;
        bus.ram()[0x0102] = 0xf1;
        bus.ram()[0x0103] = 0x32;
        bus.ram()[0x0104] = 0x33;
        bus.ram()[0x0105] = 0x34;
        bus.ram()[0x0106] = 0x35;
        sut.registers().af = 0xf1ff;
        sut.registers().bc = 0x0007;
        sut.registers().hl = 0x0106;

        sut.step();
        EXPECT_EQ(sut.registers().pc, 0);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.clockCounter(), 21);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0);
        EXPECT_EQ(sut.clockCounter(), 21*2);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0);

        EXPECT_EQ(sut.clockCounter(), 21*3);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0);
        EXPECT_EQ(sut.clockCounter(), 21*4);

        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 10);
        EXPECT_EQ(sut.clockCounter(), 21*4+16);
        EXPECT_EQ(sut.registers().af, 0xf147);
        EXPECT_EQ(sut.registers().bc, 0x0002);
        EXPECT_EQ(sut.registers().hl, 0x0101);

        sut.step(); // execute additional NOP
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().af, 0xf147);
        EXPECT_EQ(sut.registers().bc, 0x0002);
        EXPECT_EQ(sut.registers().hl, 0x0101);
    }
}

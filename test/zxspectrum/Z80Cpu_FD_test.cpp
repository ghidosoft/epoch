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
    TEST(Z80Cpu_FD, Opcode00010001_ADD_IY_BC) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x09 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0xabcd;
        sut.registers().hl = 0x0000;
        sut.registers().iy = 0x5678;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().bc, 0xabcd);
        EXPECT_EQ(sut.registers().hl, 0x0000);
        EXPECT_EQ(sut.registers().iy, 0x0245);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu_FD, Opcode00100001_LD_IY_nn) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x21, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().hl = 0x0000;
        sut.registers().iy = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().hl, 0x0000);
        EXPECT_EQ(sut.registers().iy, 0x1234);
    }

    TEST(Z80Cpu_FD, Opcode00101001_ADD_HL_HL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x29 } };
        Z80Cpu sut{ bus };
        sut.registers().iy = 0x5678;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().iy, 0xacf0);
        EXPECT_FALSE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu_FD, Opcode00111001_ADD_HL_SP) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x39 } };
        Z80Cpu sut{ bus };
        sut.registers().sp = 0xfedc;
        sut.registers().iy = 0x5678;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().sp, 0xfedc);
        EXPECT_EQ(sut.registers().iy, 0x5554);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_FALSE(sut.registers().af.n());
    }

    TEST(Z80Cpu_FD, Opcode00100010_LD_mnn_IY) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x22, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().iy = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(bus.ram()[0x1234], 0xcd);
        EXPECT_EQ(bus.ram()[0x1235], 0xab);
    }

    TEST(Z80Cpu_FD, Opcode00101010_LD_IY_mnn) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x2a, 0x34, 0x12 } };
        bus.ram()[0x1234] = 0xcd;
        bus.ram()[0x1235] = 0xab;
        Z80Cpu sut{ bus };
        sut.registers().iy = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().iy, 0xabcd);
        EXPECT_EQ(bus.ram()[0x1234], 0xcd);
        EXPECT_EQ(bus.ram()[0x1235], 0xab);
    }

    TEST(Z80Cpu_FD, Opcode00100011_INC_IY) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x23 } };
        Z80Cpu sut{ bus };
        sut.registers().iy = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().iy, 0x0001);
    }

    TEST(Z80Cpu_FD, Opcode00100110_LD_IYH_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x26, 0x46 } };
        Z80Cpu sut{ bus };
        sut.registers().iy = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().iy, 0x4600);
    }

    TEST(Z80Cpu_FD, Opcode00101011_DEC_IY) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x2b } };
        Z80Cpu sut{ bus };
        sut.registers().iy = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().iy, 0xffff);
    }

    TEST(Z80Cpu_FD, Opcode00101110_LD_IYL_n) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x2e, 0x46 } };
        Z80Cpu sut{ bus };
        sut.registers().iy = 0x0000;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().iy, 0x0046);
    }

    TEST(Z80Cpu_FD, Opcode01101110_LD_L_IYd) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x6e, 0x04 } };
        bus.ram()[0x1234] = 0x89;
        Z80Cpu sut{ bus };
        sut.registers().hl = 0x0000;
        sut.registers().iy = 0x1230;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().hl, 0x0089);
    }

    TEST(Z80Cpu_FD, Opcode01110011_LD_IYd_E) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x73, 0x04 } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0xbbaa;
        sut.registers().iy = 0x1230;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 3);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().de, 0xbbaa);
        EXPECT_EQ(bus.ram()[0x1234], 0xaa);
    }

    TEST(Z80Cpu_FD, Opcode10000101_ADD_IYL) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0x85 } };
        Z80Cpu sut{ bus };
        sut.registers().af.high(0xff);
        sut.registers().iy = 0xcc03;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().af.high(), 0x02);
        EXPECT_TRUE(sut.registers().af.c());
        EXPECT_TRUE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
    }

    TEST(Z80Cpu_FD, Opcode11100011_EX_mSP_IY) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0xe3 } };
        bus.ram()[0x1234] = 0x56;
        bus.ram()[0x1235] = 0x78;
        Z80Cpu sut{ bus };
        sut.registers().sp = 0x1234;
        sut.registers().iy = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().sp, 0x1234);
        EXPECT_EQ(sut.registers().iy, 0x7856);
        EXPECT_EQ(bus.ram()[0x1234], 0xcd);
        EXPECT_EQ(bus.ram()[0x1235], 0xab);
    }

    TEST(Z80Cpu_FD, Opcode11101011_EX_DE_HL) {
        // HL not changed to IY
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0xeb } };
        Z80Cpu sut{ bus };
        sut.registers().de = 0x1234;
        sut.registers().hl = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().de, 0xabcd);
        EXPECT_EQ(sut.registers().hl, 0x1234);
    }

    TEST(Z80Cpu_FD, Opcode11xxx100_PUSH_IY_POP_IY) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0xe5, 0xfd, 0xe1 } };
        Z80Cpu sut{ bus };
        sut.registers().iy = 0x1234;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().sp, 0xfffd);
        EXPECT_EQ(sut.registers().iy, 0x1234);
        EXPECT_EQ(bus.ram(0xffff), 0x00);
        EXPECT_EQ(bus.ram(0xfffe), 0x12);
        EXPECT_EQ(bus.ram(0xfffd), 0x34);
        sut.registers().iy = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 4);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().iy, 0x1234);
        EXPECT_EQ(bus.ram(0xffff), 0x00);
        EXPECT_EQ(bus.ram(0xfffe), 0x12);
        EXPECT_EQ(bus.ram(0xfffd), 0x34);
    }

    TEST(Z80Cpu_FD, Opcode11111001_LD_SP_IY) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0xf9 } };
        Z80Cpu sut{ bus };
        sut.registers().iy = 0xdef0;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().iy, 0xdef0);
        EXPECT_EQ(sut.registers().sp, 0xdef0);
    }

    TEST(Z80Cpu_FD, OpcodeCB_BIT_3_IY1) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0xcb, 0x01, 0x5e } };
        Z80Cpu sut{ bus };
        bus.ram()[0x0100] = 0x00;
        sut.registers().iy = 0x00ff;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().af, 0xff55);
        EXPECT_EQ(sut.registers().iy, 0x00ff);
    }

    TEST(Z80Cpu_FD, OpcodeCB_RLC) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xfd, 0xcb, 0x01, 0x06, 0xfd, 0xcb, 0x01, 0x06 } };
        Z80Cpu sut{ bus };
        bus.ram()[0x0100] = 0x5a;
        sut.registers().iy = 0x00ff;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().af, 0xffa4);
        EXPECT_EQ(sut.registers().iy, 0x00ff);
        EXPECT_EQ(bus.ram(0x0100), 0xb4);
        sut.step();
        EXPECT_EQ(sut.registers().af, 0xff2d);
        EXPECT_EQ(bus.ram(0x0100), 0x69);
    }
}

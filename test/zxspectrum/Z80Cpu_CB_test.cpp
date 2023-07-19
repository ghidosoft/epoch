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
    TEST(Z80Cpu_CB, Opcode_00000xxx_RLC_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xcb, 0x00, 0xcb, 0x00 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0x6100;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().af, 0xff80);
        EXPECT_EQ(sut.registers().bc, 0xc200);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 4);
        EXPECT_EQ(sut.registers().af, 0xff81);
        EXPECT_EQ(sut.registers().bc, 0x8500);
    }

    TEST(Z80Cpu_CB, Opcode_00001xxx_RRC_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xcb, 0x08, 0xcb, 0x08 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0x6100;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().af, 0xffa1);
        EXPECT_EQ(sut.registers().bc, 0xb000);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 4);
        EXPECT_EQ(sut.registers().af, 0xff08);
        EXPECT_EQ(sut.registers().bc, 0x5800);
    }

    TEST(Z80Cpu_CB, Opcode_00010xxx_RL_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xcb, 0x10, 0xcb, 0x10 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0x6100;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().af, 0xff84);
        EXPECT_EQ(sut.registers().bc, 0xc300);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 4);
        EXPECT_EQ(sut.registers().af, 0xff81);
        EXPECT_EQ(sut.registers().bc, 0x8600);
    }

    TEST(Z80Cpu_CB, Opcode_00011xxx_RR_B) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xcb, 0x18, 0xcb, 0x18 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0x6100;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().af, 0xffa1);
        EXPECT_EQ(sut.registers().bc, 0xb000);
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 4);
        EXPECT_EQ(sut.registers().af, 0xff8c);
        EXPECT_EQ(sut.registers().bc, 0xd800);
    }
}

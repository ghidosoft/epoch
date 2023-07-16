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
    }

    TEST(Z80Cpu_ED, Opcode_01000011_LD_mNN_BC) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x43, 0x34, 0x12 } };
        Z80Cpu sut{ bus };
        sut.registers().bc = 0xabcd;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 4);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().bc, 0xabcd);
        EXPECT_EQ(bus.ram(0x1234), 0xcd);
        EXPECT_EQ(bus.ram(0x1235), 0xab);
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
        EXPECT_EQ(sut.registers().sp, 0x5678);
        EXPECT_EQ(bus.ram(0x1234), 0x78);
        EXPECT_EQ(bus.ram(0x1235), 0x56);
    }

    TEST(Z80Cpu_ED, Opcode_01x00110_IM_0) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x46 } };
        Z80Cpu sut{ bus };
        sut.registers().interruptMode = 1;
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().interruptMode, 0);
    }

    TEST(Z80Cpu_ED, Opcode_01x10110_IM_1) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x56 } };
        Z80Cpu sut{ bus };
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().interruptMode, 1);
    }

    TEST(Z80Cpu_ED, Opcode_01x11110_IM_2) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{ 0xed, 0x5e } };
        Z80Cpu sut{ bus };
        sut.step();
        EXPECT_EQ(sut.registers().pc, 2);
        EXPECT_EQ(sut.registers().ir, 2);
        EXPECT_EQ(sut.registers().interruptMode, 2);
    }
}

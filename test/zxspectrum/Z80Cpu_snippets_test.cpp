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
    TEST(Z80Cpu_snippets, Multiply) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{
            0xc3, 0x13, 0x00, // jp main
            // mult:
            0xc5, // push bc
            0x21, 0x00, 0x00, // ld hl, 0
            0x78, // ld a, b
            0xb7, // or a
            0x28, 0x06, // jr z, mend
            0x16, 0x00, // ld d, 0
            0x59, // ld e, c
            // loop:
            0x19, // add hl, de
            0x10, 0xfd, // djnz loop
            // mend:
            0xc1, // pop bc
            0xc9, // ret
            // end:
            0x06, 0xf0, // lb b, $f0
            0x0e, 0x82, // ld c, $82
            0xcd, 0x03, 0x00, // call mult
        } };
        Z80Cpu sut{ bus };
        while (sut.registers().pc < 0x0020)
        {
            sut.step();
        }
        EXPECT_EQ(sut.registers().pc, 0x0020);
        EXPECT_EQ(sut.registers().af.high(), 0xf0);
        EXPECT_EQ(sut.registers().bc, 0xf082);
        EXPECT_EQ(sut.registers().de, 0x0082);
        EXPECT_EQ(sut.registers().hl, 0x79e0);
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.c());
    }

    TEST(Z80Cpu_snippets, CB_Prefix) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{
            0x06, 0x00, // ld b, 0
            0xcb, 0xc0, // set 0, b
            0xcb, 0xd8, // set 3, b
            0xcb, 0xf8, // set 7, b
            0xcb, 0x98, // res 3, b
            0xcb, 0x78, // bit 7, b
            0xcb, 0x70, // bit 6, b
            0xcb, 0x00, // rlc b
        } };
        Z80Cpu sut{ bus };
        sut.step();
        sut.step();
        sut.step();
        sut.step();
        sut.step();
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0x000c);
        EXPECT_EQ(sut.registers().bc.high(), 0x81);
        // EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        EXPECT_TRUE(sut.registers().af.h());
        // EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.c());
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0x000e);
        EXPECT_EQ(sut.registers().bc.high(), 0x81);
        // EXPECT_TRUE(sut.registers().af.s());
        EXPECT_TRUE(sut.registers().af.z());
        EXPECT_TRUE(sut.registers().af.h());
        // EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.c());
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0x0010);
        EXPECT_EQ(sut.registers().bc.high(), 0x03);
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        // EXPECT_TRUE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.c());
    }

    TEST(Z80Cpu_snippets, LD_IX_IY_MEM) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{
            0xdd, 0x21, 0x1e, 0x00, // ld ix, $001e
            0xdd, 0xe5, // push ix
            0xdd, 0x23, // inc ix
            0xdd, 0x36, 0x01, 0x35, // ld (ix+1), $35
            0xfd, 0xe1, // pop iy
            0xfd, 0x36, 0xff, 0x42, // ld (iy-1), $42
            0xdd, 0x34, 0x01, // inc (ix+1)
            0xfd, 0x35, 0xff, // dec (iy-1)
            0x3e, 0x25, // ld a, $25
            0xdd, 0x96, 0x01, // sub (ix+1)
        } };
        Z80Cpu sut{ bus };
        sut.step();
        sut.step();
        sut.step();
        sut.step();
        sut.step();
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0x0012);
        EXPECT_EQ(sut.registers().af, 0xffff);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().ix, 0x1f);
        EXPECT_EQ(sut.registers().iy, 0x1e);
        EXPECT_EQ(bus.ram(0x1d), 0x42);
        EXPECT_EQ(bus.ram(0x20), 0x35);
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_TRUE(sut.registers().af.z());
        EXPECT_TRUE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.c());
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0x0015);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().ix, 0x1f);
        EXPECT_EQ(sut.registers().iy, 0x1e);
        EXPECT_EQ(bus.ram(0x1d), 0x42);
        EXPECT_EQ(bus.ram(0x20), 0x36);
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.c());
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0x0018);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().ix, 0x1f);
        EXPECT_EQ(sut.registers().iy, 0x1e);
        EXPECT_EQ(bus.ram(0x1d), 0x41);
        EXPECT_EQ(bus.ram(0x20), 0x36);
        EXPECT_FALSE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.c());
        sut.step();
        sut.step();
        EXPECT_EQ(sut.registers().pc, 0x001d);
        EXPECT_EQ(sut.registers().sp, 0xffff);
        EXPECT_EQ(sut.registers().af.high(), 0xef);
        EXPECT_EQ(sut.registers().ix, 0x1f);
        EXPECT_EQ(sut.registers().iy, 0x1e);
        EXPECT_EQ(bus.ram(0x1d), 0x41);
        EXPECT_EQ(bus.ram(0x20), 0x36);
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        EXPECT_TRUE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_TRUE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.c());
    }

    TEST(Z80Cpu_snippets, LDIR) {
        TestZ80Interface bus{ std::initializer_list<uint8_t>{
            0x21, 0x00, 0x00, // ld hl, $0000
            0x11, 0x20, 0x00, // ld de, $2000
            0x01, 0x07, 0x00, // ld b, $7
            0xed, 0xb0, // ldir
        } };
        Z80Cpu sut{ bus };
        while (sut.registers().pc < 0x00b)
        {
            sut.step();
        }
        EXPECT_EQ(sut.registers().pc, 0x00b);
        EXPECT_EQ(sut.registers().bc, 0x0000);
        EXPECT_EQ(sut.registers().de, 0x0027);
        EXPECT_EQ(sut.registers().hl, 0x0007);
        EXPECT_EQ(bus.ram(0x20), 0x21);
        EXPECT_EQ(bus.ram(0x21), 0x00);
        EXPECT_EQ(bus.ram(0x22), 0x00);
        EXPECT_EQ(bus.ram(0x23), 0x11);
        EXPECT_EQ(bus.ram(0x24), 0x20);
        EXPECT_EQ(bus.ram(0x25), 0x00);
        EXPECT_EQ(bus.ram(0x26), 0x01);
        EXPECT_EQ(bus.ram(0x27), 0x00);
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_TRUE(sut.registers().af.z());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_FALSE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_TRUE(sut.registers().af.c());
    }
}

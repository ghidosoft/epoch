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
            0x06, 0xf0, // lb b, $f0
            0x0e, 0x82, // ld c, $82
            0x21, 0x00, 0x00, // ld hl, 0
            0x78, // ld a, b
            0xb7, // or a
            0x28, 0x06, // jr z, end
            0x16, 0x00, // ld d, 0
            0x59, // ld e, c
            // loop:
            0x19, // add hl, de
            0x10, 0xfd, // djnz loop
            // end:
        } };
        Z80Cpu sut{ bus };
        while (sut.registers().pc != 0x0011)
        {
            sut.step();
        }
        EXPECT_EQ(sut.registers().pc, 0x0011);
        EXPECT_EQ(sut.registers().af.high(), 0xf0);
        EXPECT_EQ(sut.registers().bc.high(), 0x82);
        EXPECT_EQ(sut.registers().de.high(), 0x82);
        EXPECT_EQ(sut.registers().hl, 0x79e0);
        EXPECT_TRUE(sut.registers().af.s());
        EXPECT_FALSE(sut.registers().af.z());
        EXPECT_FALSE(sut.registers().af.h());
        EXPECT_TRUE(sut.registers().af.p());
        EXPECT_FALSE(sut.registers().af.n());
        EXPECT_FALSE(sut.registers().af.c());
    }
}

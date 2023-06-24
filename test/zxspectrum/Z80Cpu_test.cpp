#include <gtest/gtest.h>

#include "../../src/zxspectrum/src/Z80Cpu.h"

namespace epoch::zxspectrum
{
    TEST(Z80Cpu, ResetBehavior) {
        Z80Cpu sut;
        sut.reset();
        EXPECT_EQ(sut.registers().pc, 0);
    }

}

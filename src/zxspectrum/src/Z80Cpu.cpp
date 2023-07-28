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

#include "Z80Cpu.h"

#include <bit>
#include <cassert>
#include <optional>
#include <sstream>

namespace epoch::zxspectrum
{
    static const uint8_t s_flagsLookupSZP[256] = {
          0x44, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00, 0x08, 0x0c, 0x0c, 0x08, 0x0c, 0x08, 0x08, 0x0c,
          0x00, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00, 0x04, 0x0c, 0x08, 0x08, 0x0c, 0x08, 0x0c, 0x0c, 0x08,
          0x20, 0x24, 0x24, 0x20, 0x24, 0x20, 0x20, 0x24, 0x2c, 0x28, 0x28, 0x2c, 0x28, 0x2c, 0x2c, 0x28,
          0x24, 0x20, 0x20, 0x24, 0x20, 0x24, 0x24, 0x20, 0x28, 0x2c, 0x2c, 0x28, 0x2c, 0x28, 0x28, 0x2c,
          0x00, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00, 0x04, 0x0c, 0x08, 0x08, 0x0c, 0x08, 0x0c, 0x0c, 0x08,
          0x04, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00, 0x08, 0x0c, 0x0c, 0x08, 0x0c, 0x08, 0x08, 0x0c,
          0x24, 0x20, 0x20, 0x24, 0x20, 0x24, 0x24, 0x20, 0x28, 0x2c, 0x2c, 0x28, 0x2c, 0x28, 0x28, 0x2c,
          0x20, 0x24, 0x24, 0x20, 0x24, 0x20, 0x20, 0x24, 0x2c, 0x28, 0x28, 0x2c, 0x28, 0x2c, 0x2c, 0x28,
          0x80, 0x84, 0x84, 0x80, 0x84, 0x80, 0x80, 0x84, 0x8c, 0x88, 0x88, 0x8c, 0x88, 0x8c, 0x8c, 0x88,
          0x84, 0x80, 0x80, 0x84, 0x80, 0x84, 0x84, 0x80, 0x88, 0x8c, 0x8c, 0x88, 0x8c, 0x88, 0x88, 0x8c,
          0xa4, 0xa0, 0xa0, 0xa4, 0xa0, 0xa4, 0xa4, 0xa0, 0xa8, 0xac, 0xac, 0xa8, 0xac, 0xa8, 0xa8, 0xac,
          0xa0, 0xa4, 0xa4, 0xa0, 0xa4, 0xa0, 0xa0, 0xa4, 0xac, 0xa8, 0xa8, 0xac, 0xa8, 0xac, 0xac, 0xa8,
          0x84, 0x80, 0x80, 0x84, 0x80, 0x84, 0x84, 0x80, 0x88, 0x8c, 0x8c, 0x88, 0x8c, 0x88, 0x88, 0x8c,
          0x80, 0x84, 0x84, 0x80, 0x84, 0x80, 0x80, 0x84, 0x8c, 0x88, 0x88, 0x8c, 0x88, 0x8c, 0x8c, 0x88,
          0xa0, 0xa4, 0xa4, 0xa0, 0xa4, 0xa0, 0xa0, 0xa4, 0xac, 0xa8, 0xa8, 0xac, 0xa8, 0xac, 0xac, 0xa8,
          0xa4, 0xa0, 0xa0, 0xa4, 0xa0, 0xa4, 0xa4, 0xa0, 0xa8, 0xac, 0xac, 0xa8, 0xac, 0xa8, 0xa8, 0xac,
    };

    static const uint16_t s_daaTable[2048] = {
          0x0044, 0x0100, 0x0200, 0x0304, 0x0400, 0x0504, 0x0604, 0x0700,
          0x0808, 0x090C, 0x1010, 0x1114, 0x1214, 0x1310, 0x1414, 0x1510,
          0x1000, 0x1104, 0x1204, 0x1300, 0x1404, 0x1500, 0x1600, 0x1704,
          0x180C, 0x1908, 0x2030, 0x2134, 0x2234, 0x2330, 0x2434, 0x2530,
          0x2020, 0x2124, 0x2224, 0x2320, 0x2424, 0x2520, 0x2620, 0x2724,
          0x282C, 0x2928, 0x3034, 0x3130, 0x3230, 0x3334, 0x3430, 0x3534,
          0x3024, 0x3120, 0x3220, 0x3324, 0x3420, 0x3524, 0x3624, 0x3720,
          0x3828, 0x392C, 0x4010, 0x4114, 0x4214, 0x4310, 0x4414, 0x4510,
          0x4000, 0x4104, 0x4204, 0x4300, 0x4404, 0x4500, 0x4600, 0x4704,
          0x480C, 0x4908, 0x5014, 0x5110, 0x5210, 0x5314, 0x5410, 0x5514,
          0x5004, 0x5100, 0x5200, 0x5304, 0x5400, 0x5504, 0x5604, 0x5700,
          0x5808, 0x590C, 0x6034, 0x6130, 0x6230, 0x6334, 0x6430, 0x6534,
          0x6024, 0x6120, 0x6220, 0x6324, 0x6420, 0x6524, 0x6624, 0x6720,
          0x6828, 0x692C, 0x7030, 0x7134, 0x7234, 0x7330, 0x7434, 0x7530,
          0x7020, 0x7124, 0x7224, 0x7320, 0x7424, 0x7520, 0x7620, 0x7724,
          0x782C, 0x7928, 0x8090, 0x8194, 0x8294, 0x8390, 0x8494, 0x8590,
          0x8080, 0x8184, 0x8284, 0x8380, 0x8484, 0x8580, 0x8680, 0x8784,
          0x888C, 0x8988, 0x9094, 0x9190, 0x9290, 0x9394, 0x9490, 0x9594,
          0x9084, 0x9180, 0x9280, 0x9384, 0x9480, 0x9584, 0x9684, 0x9780,
          0x9888, 0x998C, 0x0055, 0x0111, 0x0211, 0x0315, 0x0411, 0x0515,
          0x0045, 0x0101, 0x0201, 0x0305, 0x0401, 0x0505, 0x0605, 0x0701,
          0x0809, 0x090D, 0x1011, 0x1115, 0x1215, 0x1311, 0x1415, 0x1511,
          0x1001, 0x1105, 0x1205, 0x1301, 0x1405, 0x1501, 0x1601, 0x1705,
          0x180D, 0x1909, 0x2031, 0x2135, 0x2235, 0x2331, 0x2435, 0x2531,
          0x2021, 0x2125, 0x2225, 0x2321, 0x2425, 0x2521, 0x2621, 0x2725,
          0x282D, 0x2929, 0x3035, 0x3131, 0x3231, 0x3335, 0x3431, 0x3535,
          0x3025, 0x3121, 0x3221, 0x3325, 0x3421, 0x3525, 0x3625, 0x3721,
          0x3829, 0x392D, 0x4011, 0x4115, 0x4215, 0x4311, 0x4415, 0x4511,
          0x4001, 0x4105, 0x4205, 0x4301, 0x4405, 0x4501, 0x4601, 0x4705,
          0x480D, 0x4909, 0x5015, 0x5111, 0x5211, 0x5315, 0x5411, 0x5515,
          0x5005, 0x5101, 0x5201, 0x5305, 0x5401, 0x5505, 0x5605, 0x5701,
          0x5809, 0x590D, 0x6035, 0x6131, 0x6231, 0x6335, 0x6431, 0x6535,
          0x6025, 0x6121, 0x6221, 0x6325, 0x6421, 0x6525, 0x6625, 0x6721,
          0x6829, 0x692D, 0x7031, 0x7135, 0x7235, 0x7331, 0x7435, 0x7531,
          0x7021, 0x7125, 0x7225, 0x7321, 0x7425, 0x7521, 0x7621, 0x7725,
          0x782D, 0x7929, 0x8091, 0x8195, 0x8295, 0x8391, 0x8495, 0x8591,
          0x8081, 0x8185, 0x8285, 0x8381, 0x8485, 0x8581, 0x8681, 0x8785,
          0x888D, 0x8989, 0x9095, 0x9191, 0x9291, 0x9395, 0x9491, 0x9595,
          0x9085, 0x9181, 0x9281, 0x9385, 0x9481, 0x9585, 0x9685, 0x9781,
          0x9889, 0x998D, 0xA0B5, 0xA1B1, 0xA2B1, 0xA3B5, 0xA4B1, 0xA5B5,
          0xA0A5, 0xA1A1, 0xA2A1, 0xA3A5, 0xA4A1, 0xA5A5, 0xA6A5, 0xA7A1,
          0xA8A9, 0xA9AD, 0xB0B1, 0xB1B5, 0xB2B5, 0xB3B1, 0xB4B5, 0xB5B1,
          0xB0A1, 0xB1A5, 0xB2A5, 0xB3A1, 0xB4A5, 0xB5A1, 0xB6A1, 0xB7A5,
          0xB8AD, 0xB9A9, 0xC095, 0xC191, 0xC291, 0xC395, 0xC491, 0xC595,
          0xC085, 0xC181, 0xC281, 0xC385, 0xC481, 0xC585, 0xC685, 0xC781,
          0xC889, 0xC98D, 0xD091, 0xD195, 0xD295, 0xD391, 0xD495, 0xD591,
          0xD081, 0xD185, 0xD285, 0xD381, 0xD485, 0xD581, 0xD681, 0xD785,
          0xD88D, 0xD989, 0xE0B1, 0xE1B5, 0xE2B5, 0xE3B1, 0xE4B5, 0xE5B1,
          0xE0A1, 0xE1A5, 0xE2A5, 0xE3A1, 0xE4A5, 0xE5A1, 0xE6A1, 0xE7A5,
          0xE8AD, 0xE9A9, 0xF0B5, 0xF1B1, 0xF2B1, 0xF3B5, 0xF4B1, 0xF5B5,
          0xF0A5, 0xF1A1, 0xF2A1, 0xF3A5, 0xF4A1, 0xF5A5, 0xF6A5, 0xF7A1,
          0xF8A9, 0xF9AD, 0x0055, 0x0111, 0x0211, 0x0315, 0x0411, 0x0515,
          0x0045, 0x0101, 0x0201, 0x0305, 0x0401, 0x0505, 0x0605, 0x0701,
          0x0809, 0x090D, 0x1011, 0x1115, 0x1215, 0x1311, 0x1415, 0x1511,
          0x1001, 0x1105, 0x1205, 0x1301, 0x1405, 0x1501, 0x1601, 0x1705,
          0x180D, 0x1909, 0x2031, 0x2135, 0x2235, 0x2331, 0x2435, 0x2531,
          0x2021, 0x2125, 0x2225, 0x2321, 0x2425, 0x2521, 0x2621, 0x2725,
          0x282D, 0x2929, 0x3035, 0x3131, 0x3231, 0x3335, 0x3431, 0x3535,
          0x3025, 0x3121, 0x3221, 0x3325, 0x3421, 0x3525, 0x3625, 0x3721,
          0x3829, 0x392D, 0x4011, 0x4115, 0x4215, 0x4311, 0x4415, 0x4511,
          0x4001, 0x4105, 0x4205, 0x4301, 0x4405, 0x4501, 0x4601, 0x4705,
          0x480D, 0x4909, 0x5015, 0x5111, 0x5211, 0x5315, 0x5411, 0x5515,
          0x5005, 0x5101, 0x5201, 0x5305, 0x5401, 0x5505, 0x5605, 0x5701,
          0x5809, 0x590D, 0x6035, 0x6131, 0x6231, 0x6335, 0x6431, 0x6535,
          0x0604, 0x0700, 0x0808, 0x090C, 0x0A0C, 0x0B08, 0x0C0C, 0x0D08,
          0x0E08, 0x0F0C, 0x1010, 0x1114, 0x1214, 0x1310, 0x1414, 0x1510,
          0x1600, 0x1704, 0x180C, 0x1908, 0x1A08, 0x1B0C, 0x1C08, 0x1D0C,
          0x1E0C, 0x1F08, 0x2030, 0x2134, 0x2234, 0x2330, 0x2434, 0x2530,
          0x2620, 0x2724, 0x282C, 0x2928, 0x2A28, 0x2B2C, 0x2C28, 0x2D2C,
          0x2E2C, 0x2F28, 0x3034, 0x3130, 0x3230, 0x3334, 0x3430, 0x3534,
          0x3624, 0x3720, 0x3828, 0x392C, 0x3A2C, 0x3B28, 0x3C2C, 0x3D28,
          0x3E28, 0x3F2C, 0x4010, 0x4114, 0x4214, 0x4310, 0x4414, 0x4510,
          0x4600, 0x4704, 0x480C, 0x4908, 0x4A08, 0x4B0C, 0x4C08, 0x4D0C,
          0x4E0C, 0x4F08, 0x5014, 0x5110, 0x5210, 0x5314, 0x5410, 0x5514,
          0x5604, 0x5700, 0x5808, 0x590C, 0x5A0C, 0x5B08, 0x5C0C, 0x5D08,
          0x5E08, 0x5F0C, 0x6034, 0x6130, 0x6230, 0x6334, 0x6430, 0x6534,
          0x6624, 0x6720, 0x6828, 0x692C, 0x6A2C, 0x6B28, 0x6C2C, 0x6D28,
          0x6E28, 0x6F2C, 0x7030, 0x7134, 0x7234, 0x7330, 0x7434, 0x7530,
          0x7620, 0x7724, 0x782C, 0x7928, 0x7A28, 0x7B2C, 0x7C28, 0x7D2C,
          0x7E2C, 0x7F28, 0x8090, 0x8194, 0x8294, 0x8390, 0x8494, 0x8590,
          0x8680, 0x8784, 0x888C, 0x8988, 0x8A88, 0x8B8C, 0x8C88, 0x8D8C,
          0x8E8C, 0x8F88, 0x9094, 0x9190, 0x9290, 0x9394, 0x9490, 0x9594,
          0x9684, 0x9780, 0x9888, 0x998C, 0x9A8C, 0x9B88, 0x9C8C, 0x9D88,
          0x9E88, 0x9F8C, 0x0055, 0x0111, 0x0211, 0x0315, 0x0411, 0x0515,
          0x0605, 0x0701, 0x0809, 0x090D, 0x0A0D, 0x0B09, 0x0C0D, 0x0D09,
          0x0E09, 0x0F0D, 0x1011, 0x1115, 0x1215, 0x1311, 0x1415, 0x1511,
          0x1601, 0x1705, 0x180D, 0x1909, 0x1A09, 0x1B0D, 0x1C09, 0x1D0D,
          0x1E0D, 0x1F09, 0x2031, 0x2135, 0x2235, 0x2331, 0x2435, 0x2531,
          0x2621, 0x2725, 0x282D, 0x2929, 0x2A29, 0x2B2D, 0x2C29, 0x2D2D,
          0x2E2D, 0x2F29, 0x3035, 0x3131, 0x3231, 0x3335, 0x3431, 0x3535,
          0x3625, 0x3721, 0x3829, 0x392D, 0x3A2D, 0x3B29, 0x3C2D, 0x3D29,
          0x3E29, 0x3F2D, 0x4011, 0x4115, 0x4215, 0x4311, 0x4415, 0x4511,
          0x4601, 0x4705, 0x480D, 0x4909, 0x4A09, 0x4B0D, 0x4C09, 0x4D0D,
          0x4E0D, 0x4F09, 0x5015, 0x5111, 0x5211, 0x5315, 0x5411, 0x5515,
          0x5605, 0x5701, 0x5809, 0x590D, 0x5A0D, 0x5B09, 0x5C0D, 0x5D09,
          0x5E09, 0x5F0D, 0x6035, 0x6131, 0x6231, 0x6335, 0x6431, 0x6535,
          0x6625, 0x6721, 0x6829, 0x692D, 0x6A2D, 0x6B29, 0x6C2D, 0x6D29,
          0x6E29, 0x6F2D, 0x7031, 0x7135, 0x7235, 0x7331, 0x7435, 0x7531,
          0x7621, 0x7725, 0x782D, 0x7929, 0x7A29, 0x7B2D, 0x7C29, 0x7D2D,
          0x7E2D, 0x7F29, 0x8091, 0x8195, 0x8295, 0x8391, 0x8495, 0x8591,
          0x8681, 0x8785, 0x888D, 0x8989, 0x8A89, 0x8B8D, 0x8C89, 0x8D8D,
          0x8E8D, 0x8F89, 0x9095, 0x9191, 0x9291, 0x9395, 0x9491, 0x9595,
          0x9685, 0x9781, 0x9889, 0x998D, 0x9A8D, 0x9B89, 0x9C8D, 0x9D89,
          0x9E89, 0x9F8D, 0xA0B5, 0xA1B1, 0xA2B1, 0xA3B5, 0xA4B1, 0xA5B5,
          0xA6A5, 0xA7A1, 0xA8A9, 0xA9AD, 0xAAAD, 0xABA9, 0xACAD, 0xADA9,
          0xAEA9, 0xAFAD, 0xB0B1, 0xB1B5, 0xB2B5, 0xB3B1, 0xB4B5, 0xB5B1,
          0xB6A1, 0xB7A5, 0xB8AD, 0xB9A9, 0xBAA9, 0xBBAD, 0xBCA9, 0xBDAD,
          0xBEAD, 0xBFA9, 0xC095, 0xC191, 0xC291, 0xC395, 0xC491, 0xC595,
          0xC685, 0xC781, 0xC889, 0xC98D, 0xCA8D, 0xCB89, 0xCC8D, 0xCD89,
          0xCE89, 0xCF8D, 0xD091, 0xD195, 0xD295, 0xD391, 0xD495, 0xD591,
          0xD681, 0xD785, 0xD88D, 0xD989, 0xDA89, 0xDB8D, 0xDC89, 0xDD8D,
          0xDE8D, 0xDF89, 0xE0B1, 0xE1B5, 0xE2B5, 0xE3B1, 0xE4B5, 0xE5B1,
          0xE6A1, 0xE7A5, 0xE8AD, 0xE9A9, 0xEAA9, 0xEBAD, 0xECA9, 0xEDAD,
          0xEEAD, 0xEFA9, 0xF0B5, 0xF1B1, 0xF2B1, 0xF3B5, 0xF4B1, 0xF5B5,
          0xF6A5, 0xF7A1, 0xF8A9, 0xF9AD, 0xFAAD, 0xFBA9, 0xFCAD, 0xFDA9,
          0xFEA9, 0xFFAD, 0x0055, 0x0111, 0x0211, 0x0315, 0x0411, 0x0515,
          0x0605, 0x0701, 0x0809, 0x090D, 0x0A0D, 0x0B09, 0x0C0D, 0x0D09,
          0x0E09, 0x0F0D, 0x1011, 0x1115, 0x1215, 0x1311, 0x1415, 0x1511,
          0x1601, 0x1705, 0x180D, 0x1909, 0x1A09, 0x1B0D, 0x1C09, 0x1D0D,
          0x1E0D, 0x1F09, 0x2031, 0x2135, 0x2235, 0x2331, 0x2435, 0x2531,
          0x2621, 0x2725, 0x282D, 0x2929, 0x2A29, 0x2B2D, 0x2C29, 0x2D2D,
          0x2E2D, 0x2F29, 0x3035, 0x3131, 0x3231, 0x3335, 0x3431, 0x3535,
          0x3625, 0x3721, 0x3829, 0x392D, 0x3A2D, 0x3B29, 0x3C2D, 0x3D29,
          0x3E29, 0x3F2D, 0x4011, 0x4115, 0x4215, 0x4311, 0x4415, 0x4511,
          0x4601, 0x4705, 0x480D, 0x4909, 0x4A09, 0x4B0D, 0x4C09, 0x4D0D,
          0x4E0D, 0x4F09, 0x5015, 0x5111, 0x5211, 0x5315, 0x5411, 0x5515,
          0x5605, 0x5701, 0x5809, 0x590D, 0x5A0D, 0x5B09, 0x5C0D, 0x5D09,
          0x5E09, 0x5F0D, 0x6035, 0x6131, 0x6231, 0x6335, 0x6431, 0x6535,
          0x0046, 0x0102, 0x0202, 0x0306, 0x0402, 0x0506, 0x0606, 0x0702,
          0x080A, 0x090E, 0x0402, 0x0506, 0x0606, 0x0702, 0x080A, 0x090E,
          0x1002, 0x1106, 0x1206, 0x1302, 0x1406, 0x1502, 0x1602, 0x1706,
          0x180E, 0x190A, 0x1406, 0x1502, 0x1602, 0x1706, 0x180E, 0x190A,
          0x2022, 0x2126, 0x2226, 0x2322, 0x2426, 0x2522, 0x2622, 0x2726,
          0x282E, 0x292A, 0x2426, 0x2522, 0x2622, 0x2726, 0x282E, 0x292A,
          0x3026, 0x3122, 0x3222, 0x3326, 0x3422, 0x3526, 0x3626, 0x3722,
          0x382A, 0x392E, 0x3422, 0x3526, 0x3626, 0x3722, 0x382A, 0x392E,
          0x4002, 0x4106, 0x4206, 0x4302, 0x4406, 0x4502, 0x4602, 0x4706,
          0x480E, 0x490A, 0x4406, 0x4502, 0x4602, 0x4706, 0x480E, 0x490A,
          0x5006, 0x5102, 0x5202, 0x5306, 0x5402, 0x5506, 0x5606, 0x5702,
          0x580A, 0x590E, 0x5402, 0x5506, 0x5606, 0x5702, 0x580A, 0x590E,
          0x6026, 0x6122, 0x6222, 0x6326, 0x6422, 0x6526, 0x6626, 0x6722,
          0x682A, 0x692E, 0x6422, 0x6526, 0x6626, 0x6722, 0x682A, 0x692E,
          0x7022, 0x7126, 0x7226, 0x7322, 0x7426, 0x7522, 0x7622, 0x7726,
          0x782E, 0x792A, 0x7426, 0x7522, 0x7622, 0x7726, 0x782E, 0x792A,
          0x8082, 0x8186, 0x8286, 0x8382, 0x8486, 0x8582, 0x8682, 0x8786,
          0x888E, 0x898A, 0x8486, 0x8582, 0x8682, 0x8786, 0x888E, 0x898A,
          0x9086, 0x9182, 0x9282, 0x9386, 0x9482, 0x9586, 0x9686, 0x9782,
          0x988A, 0x998E, 0x3423, 0x3527, 0x3627, 0x3723, 0x382B, 0x392F,
          0x4003, 0x4107, 0x4207, 0x4303, 0x4407, 0x4503, 0x4603, 0x4707,
          0x480F, 0x490B, 0x4407, 0x4503, 0x4603, 0x4707, 0x480F, 0x490B,
          0x5007, 0x5103, 0x5203, 0x5307, 0x5403, 0x5507, 0x5607, 0x5703,
          0x580B, 0x590F, 0x5403, 0x5507, 0x5607, 0x5703, 0x580B, 0x590F,
          0x6027, 0x6123, 0x6223, 0x6327, 0x6423, 0x6527, 0x6627, 0x6723,
          0x682B, 0x692F, 0x6423, 0x6527, 0x6627, 0x6723, 0x682B, 0x692F,
          0x7023, 0x7127, 0x7227, 0x7323, 0x7427, 0x7523, 0x7623, 0x7727,
          0x782F, 0x792B, 0x7427, 0x7523, 0x7623, 0x7727, 0x782F, 0x792B,
          0x8083, 0x8187, 0x8287, 0x8383, 0x8487, 0x8583, 0x8683, 0x8787,
          0x888F, 0x898B, 0x8487, 0x8583, 0x8683, 0x8787, 0x888F, 0x898B,
          0x9087, 0x9183, 0x9283, 0x9387, 0x9483, 0x9587, 0x9687, 0x9783,
          0x988B, 0x998F, 0x9483, 0x9587, 0x9687, 0x9783, 0x988B, 0x998F,
          0xA0A7, 0xA1A3, 0xA2A3, 0xA3A7, 0xA4A3, 0xA5A7, 0xA6A7, 0xA7A3,
          0xA8AB, 0xA9AF, 0xA4A3, 0xA5A7, 0xA6A7, 0xA7A3, 0xA8AB, 0xA9AF,
          0xB0A3, 0xB1A7, 0xB2A7, 0xB3A3, 0xB4A7, 0xB5A3, 0xB6A3, 0xB7A7,
          0xB8AF, 0xB9AB, 0xB4A7, 0xB5A3, 0xB6A3, 0xB7A7, 0xB8AF, 0xB9AB,
          0xC087, 0xC183, 0xC283, 0xC387, 0xC483, 0xC587, 0xC687, 0xC783,
          0xC88B, 0xC98F, 0xC483, 0xC587, 0xC687, 0xC783, 0xC88B, 0xC98F,
          0xD083, 0xD187, 0xD287, 0xD383, 0xD487, 0xD583, 0xD683, 0xD787,
          0xD88F, 0xD98B, 0xD487, 0xD583, 0xD683, 0xD787, 0xD88F, 0xD98B,
          0xE0A3, 0xE1A7, 0xE2A7, 0xE3A3, 0xE4A7, 0xE5A3, 0xE6A3, 0xE7A7,
          0xE8AF, 0xE9AB, 0xE4A7, 0xE5A3, 0xE6A3, 0xE7A7, 0xE8AF, 0xE9AB,
          0xF0A7, 0xF1A3, 0xF2A3, 0xF3A7, 0xF4A3, 0xF5A7, 0xF6A7, 0xF7A3,
          0xF8AB, 0xF9AF, 0xF4A3, 0xF5A7, 0xF6A7, 0xF7A3, 0xF8AB, 0xF9AF,
          0x0047, 0x0103, 0x0203, 0x0307, 0x0403, 0x0507, 0x0607, 0x0703,
          0x080B, 0x090F, 0x0403, 0x0507, 0x0607, 0x0703, 0x080B, 0x090F,
          0x1003, 0x1107, 0x1207, 0x1303, 0x1407, 0x1503, 0x1603, 0x1707,
          0x180F, 0x190B, 0x1407, 0x1503, 0x1603, 0x1707, 0x180F, 0x190B,
          0x2023, 0x2127, 0x2227, 0x2323, 0x2427, 0x2523, 0x2623, 0x2727,
          0x282F, 0x292B, 0x2427, 0x2523, 0x2623, 0x2727, 0x282F, 0x292B,
          0x3027, 0x3123, 0x3223, 0x3327, 0x3423, 0x3527, 0x3627, 0x3723,
          0x382B, 0x392F, 0x3423, 0x3527, 0x3627, 0x3723, 0x382B, 0x392F,
          0x4003, 0x4107, 0x4207, 0x4303, 0x4407, 0x4503, 0x4603, 0x4707,
          0x480F, 0x490B, 0x4407, 0x4503, 0x4603, 0x4707, 0x480F, 0x490B,
          0x5007, 0x5103, 0x5203, 0x5307, 0x5403, 0x5507, 0x5607, 0x5703,
          0x580B, 0x590F, 0x5403, 0x5507, 0x5607, 0x5703, 0x580B, 0x590F,
          0x6027, 0x6123, 0x6223, 0x6327, 0x6423, 0x6527, 0x6627, 0x6723,
          0x682B, 0x692F, 0x6423, 0x6527, 0x6627, 0x6723, 0x682B, 0x692F,
          0x7023, 0x7127, 0x7227, 0x7323, 0x7427, 0x7523, 0x7623, 0x7727,
          0x782F, 0x792B, 0x7427, 0x7523, 0x7623, 0x7727, 0x782F, 0x792B,
          0x8083, 0x8187, 0x8287, 0x8383, 0x8487, 0x8583, 0x8683, 0x8787,
          0x888F, 0x898B, 0x8487, 0x8583, 0x8683, 0x8787, 0x888F, 0x898B,
          0x9087, 0x9183, 0x9283, 0x9387, 0x9483, 0x9587, 0x9687, 0x9783,
          0x988B, 0x998F, 0x9483, 0x9587, 0x9687, 0x9783, 0x988B, 0x998F,
          0xFABE, 0xFBBA, 0xFCBE, 0xFDBA, 0xFEBA, 0xFFBE, 0x0046, 0x0102,
          0x0202, 0x0306, 0x0402, 0x0506, 0x0606, 0x0702, 0x080A, 0x090E,
          0x0A1E, 0x0B1A, 0x0C1E, 0x0D1A, 0x0E1A, 0x0F1E, 0x1002, 0x1106,
          0x1206, 0x1302, 0x1406, 0x1502, 0x1602, 0x1706, 0x180E, 0x190A,
          0x1A1A, 0x1B1E, 0x1C1A, 0x1D1E, 0x1E1E, 0x1F1A, 0x2022, 0x2126,
          0x2226, 0x2322, 0x2426, 0x2522, 0x2622, 0x2726, 0x282E, 0x292A,
          0x2A3A, 0x2B3E, 0x2C3A, 0x2D3E, 0x2E3E, 0x2F3A, 0x3026, 0x3122,
          0x3222, 0x3326, 0x3422, 0x3526, 0x3626, 0x3722, 0x382A, 0x392E,
          0x3A3E, 0x3B3A, 0x3C3E, 0x3D3A, 0x3E3A, 0x3F3E, 0x4002, 0x4106,
          0x4206, 0x4302, 0x4406, 0x4502, 0x4602, 0x4706, 0x480E, 0x490A,
          0x4A1A, 0x4B1E, 0x4C1A, 0x4D1E, 0x4E1E, 0x4F1A, 0x5006, 0x5102,
          0x5202, 0x5306, 0x5402, 0x5506, 0x5606, 0x5702, 0x580A, 0x590E,
          0x5A1E, 0x5B1A, 0x5C1E, 0x5D1A, 0x5E1A, 0x5F1E, 0x6026, 0x6122,
          0x6222, 0x6326, 0x6422, 0x6526, 0x6626, 0x6722, 0x682A, 0x692E,
          0x6A3E, 0x6B3A, 0x6C3E, 0x6D3A, 0x6E3A, 0x6F3E, 0x7022, 0x7126,
          0x7226, 0x7322, 0x7426, 0x7522, 0x7622, 0x7726, 0x782E, 0x792A,
          0x7A3A, 0x7B3E, 0x7C3A, 0x7D3E, 0x7E3E, 0x7F3A, 0x8082, 0x8186,
          0x8286, 0x8382, 0x8486, 0x8582, 0x8682, 0x8786, 0x888E, 0x898A,
          0x8A9A, 0x8B9E, 0x8C9A, 0x8D9E, 0x8E9E, 0x8F9A, 0x9086, 0x9182,
          0x9282, 0x9386, 0x3423, 0x3527, 0x3627, 0x3723, 0x382B, 0x392F,
          0x3A3F, 0x3B3B, 0x3C3F, 0x3D3B, 0x3E3B, 0x3F3F, 0x4003, 0x4107,
          0x4207, 0x4303, 0x4407, 0x4503, 0x4603, 0x4707, 0x480F, 0x490B,
          0x4A1B, 0x4B1F, 0x4C1B, 0x4D1F, 0x4E1F, 0x4F1B, 0x5007, 0x5103,
          0x5203, 0x5307, 0x5403, 0x5507, 0x5607, 0x5703, 0x580B, 0x590F,
          0x5A1F, 0x5B1B, 0x5C1F, 0x5D1B, 0x5E1B, 0x5F1F, 0x6027, 0x6123,
          0x6223, 0x6327, 0x6423, 0x6527, 0x6627, 0x6723, 0x682B, 0x692F,
          0x6A3F, 0x6B3B, 0x6C3F, 0x6D3B, 0x6E3B, 0x6F3F, 0x7023, 0x7127,
          0x7227, 0x7323, 0x7427, 0x7523, 0x7623, 0x7727, 0x782F, 0x792B,
          0x7A3B, 0x7B3F, 0x7C3B, 0x7D3F, 0x7E3F, 0x7F3B, 0x8083, 0x8187,
          0x8287, 0x8383, 0x8487, 0x8583, 0x8683, 0x8787, 0x888F, 0x898B,
          0x8A9B, 0x8B9F, 0x8C9B, 0x8D9F, 0x8E9F, 0x8F9B, 0x9087, 0x9183,
          0x9283, 0x9387, 0x9483, 0x9587, 0x9687, 0x9783, 0x988B, 0x998F,
          0x9A9F, 0x9B9B, 0x9C9F, 0x9D9B, 0x9E9B, 0x9F9F, 0xA0A7, 0xA1A3,
          0xA2A3, 0xA3A7, 0xA4A3, 0xA5A7, 0xA6A7, 0xA7A3, 0xA8AB, 0xA9AF,
          0xAABF, 0xABBB, 0xACBF, 0xADBB, 0xAEBB, 0xAFBF, 0xB0A3, 0xB1A7,
          0xB2A7, 0xB3A3, 0xB4A7, 0xB5A3, 0xB6A3, 0xB7A7, 0xB8AF, 0xB9AB,
          0xBABB, 0xBBBF, 0xBCBB, 0xBDBF, 0xBEBF, 0xBFBB, 0xC087, 0xC183,
          0xC283, 0xC387, 0xC483, 0xC587, 0xC687, 0xC783, 0xC88B, 0xC98F,
          0xCA9F, 0xCB9B, 0xCC9F, 0xCD9B, 0xCE9B, 0xCF9F, 0xD083, 0xD187,
          0xD287, 0xD383, 0xD487, 0xD583, 0xD683, 0xD787, 0xD88F, 0xD98B,
          0xDA9B, 0xDB9F, 0xDC9B, 0xDD9F, 0xDE9F, 0xDF9B, 0xE0A3, 0xE1A7,
          0xE2A7, 0xE3A3, 0xE4A7, 0xE5A3, 0xE6A3, 0xE7A7, 0xE8AF, 0xE9AB,
          0xEABB, 0xEBBF, 0xECBB, 0xEDBF, 0xEEBF, 0xEFBB, 0xF0A7, 0xF1A3,
          0xF2A3, 0xF3A7, 0xF4A3, 0xF5A7, 0xF6A7, 0xF7A3, 0xF8AB, 0xF9AF,
          0xFABF, 0xFBBB, 0xFCBF, 0xFDBB, 0xFEBB, 0xFFBF, 0x0047, 0x0103,
          0x0203, 0x0307, 0x0403, 0x0507, 0x0607, 0x0703, 0x080B, 0x090F,
          0x0A1F, 0x0B1B, 0x0C1F, 0x0D1B, 0x0E1B, 0x0F1F, 0x1003, 0x1107,
          0x1207, 0x1303, 0x1407, 0x1503, 0x1603, 0x1707, 0x180F, 0x190B,
          0x1A1B, 0x1B1F, 0x1C1B, 0x1D1F, 0x1E1F, 0x1F1B, 0x2023, 0x2127,
          0x2227, 0x2323, 0x2427, 0x2523, 0x2623, 0x2727, 0x282F, 0x292B,
          0x2A3B, 0x2B3F, 0x2C3B, 0x2D3F, 0x2E3F, 0x2F3B, 0x3027, 0x3123,
          0x3223, 0x3327, 0x3423, 0x3527, 0x3627, 0x3723, 0x382B, 0x392F,
          0x3A3F, 0x3B3B, 0x3C3F, 0x3D3B, 0x3E3B, 0x3F3F, 0x4003, 0x4107,
          0x4207, 0x4303, 0x4407, 0x4503, 0x4603, 0x4707, 0x480F, 0x490B,
          0x4A1B, 0x4B1F, 0x4C1B, 0x4D1F, 0x4E1F, 0x4F1B, 0x5007, 0x5103,
          0x5203, 0x5307, 0x5403, 0x5507, 0x5607, 0x5703, 0x580B, 0x590F,
          0x5A1F, 0x5B1B, 0x5C1F, 0x5D1B, 0x5E1B, 0x5F1F, 0x6027, 0x6123,
          0x6223, 0x6327, 0x6423, 0x6527, 0x6627, 0x6723, 0x682B, 0x692F,
          0x6A3F, 0x6B3B, 0x6C3F, 0x6D3B, 0x6E3B, 0x6F3F, 0x7023, 0x7127,
          0x7227, 0x7323, 0x7427, 0x7523, 0x7623, 0x7727, 0x782F, 0x792B,
          0x7A3B, 0x7B3F, 0x7C3B, 0x7D3F, 0x7E3F, 0x7F3B, 0x8083, 0x8187,
          0x8287, 0x8383, 0x8487, 0x8583, 0x8683, 0x8787, 0x888F, 0x898B,
          0x8A9B, 0x8B9F, 0x8C9B, 0x8D9F, 0x8E9F, 0x8F9B, 0x9087, 0x9183,
          0x9283, 0x9387, 0x9483, 0x9587, 0x9687, 0x9783, 0x988B, 0x998F,
    };

    Z80Cpu::Z80Cpu(Z80Interface& bus) :
        m_bus{ bus },
        m_registersPointers{{ // TODO: endianness
            {
                reinterpret_cast<uint8_t*>(&m_registers.bc.value) + 1,
                reinterpret_cast<uint8_t*>(&m_registers.bc.value),
                reinterpret_cast<uint8_t*>(&m_registers.de.value) + 1,
                reinterpret_cast<uint8_t*>(&m_registers.de.value),
                reinterpret_cast<uint8_t*>(&m_registers.hl.value) + 1,
                reinterpret_cast<uint8_t*>(&m_registers.hl.value),
                nullptr, // (HL)
                reinterpret_cast<uint8_t*>(&m_registers.af.value) + 1,
            },
            {
                reinterpret_cast<uint8_t*>(&m_registers.bc.value) + 1,
                reinterpret_cast<uint8_t*>(&m_registers.bc.value),
                reinterpret_cast<uint8_t*>(&m_registers.de.value) + 1,
                reinterpret_cast<uint8_t*>(&m_registers.de.value),
                reinterpret_cast<uint8_t*>(&m_registers.ix.value) + 1,
                reinterpret_cast<uint8_t*>(&m_registers.ix.value),
                nullptr, // (IX)
                reinterpret_cast<uint8_t*>(&m_registers.af.value) + 1,
            },
            {
                reinterpret_cast<uint8_t*>(&m_registers.bc.value) + 1,
                reinterpret_cast<uint8_t*>(&m_registers.bc.value),
                reinterpret_cast<uint8_t*>(&m_registers.de.value) + 1,
                reinterpret_cast<uint8_t*>(&m_registers.de.value),
                reinterpret_cast<uint8_t*>(&m_registers.iy.value) + 1,
                reinterpret_cast<uint8_t*>(&m_registers.iy.value),
                nullptr, // (IY)
                reinterpret_cast<uint8_t*>(&m_registers.af.value) + 1,
            },
        }}
    {
        reset();
    }

    void Z80Cpu::clock()
    {
        if (m_remainingCycles == 0)
        {
            if (m_interruptRequested && m_registers.iff1 && !m_registers.interruptJustEnabled)
            {
                handleInterrupt();
            }
            else
            {
                executeInstruction();
            }
        }

        m_clockCounter++;
        m_remainingCycles--;
    }

    void Z80Cpu::step()
    {
        do
        {
            clock();
        } while (m_remainingCycles > 0);
    }

    void Z80Cpu::reset()
    {
        m_registers = {};
        m_currentPrefix = Z80OpcodePrefix::none;
        m_interruptRequested = {};
        m_remainingCycles = {};
        m_clockCounter = {};
    }

    void Z80Cpu::interruptRequest(const bool requested)
    {
        m_interruptRequested = requested;
    }

    void Z80Cpu::executeInstruction()
    {
        m_opcode = fetchOpcode();

        const auto quadrant = m_opcode >> 6;

        if (quadrant == 0)
        {
            mainQuadrant0();
        }
        else if (quadrant == 1)
        {
            // LD 8bit / HALT
            mainQuadrant1();
        }
        else if (quadrant == 2)
        {
            // ALU operations
            mainQuadrant2();
        }
        else
        {
            mainQuadrant3();
        }

        if (m_opcode != 0xfb && m_registers.interruptJustEnabled)
        {
            m_registers.interruptJustEnabled = false;
        }

        m_currentPrefix = Z80OpcodePrefix::none;
    }

    void Z80Cpu::handleInterrupt()
    {
        if (m_opcode == 0x76)
        {
            // Exit HALT state
            m_registers.pc++;
        }
        m_registers.ir.low((m_registers.ir.low() + 1) & 0x7f);
        m_registers.iff1 = false;
        switch (m_registers.interruptMode)
        {
        case 1:
            m_remainingCycles++;
            push16(m_registers.pc);
            m_registers.pc = 0x0038;
            break;
        case 2:
            m_remainingCycles++;
            push16(m_registers.pc);
            m_registers.pc = read16(m_registers.ir & 0xff00); // ignore low byte
            break;
        default:
            assert(false);
            break;
        }
    }

    uint8_t Z80Cpu::fetchOpcode()
    {
        m_registers.ir.low((m_registers.ir.low() + 1) & 0x7f);
        m_remainingCycles += 4;
        return m_bus.read(m_registers.pc++);
    }

    uint8_t Z80Cpu::busRead(const uint16_t address)
    {
        m_remainingCycles += 3;
        return m_bus.read(address);
    }

    void Z80Cpu::busWrite(const uint16_t address, const uint8_t value)
    {
        m_remainingCycles += 3;
        m_bus.write(address, value);
    }

    uint8_t Z80Cpu::ioRead(const uint16_t port)
    {
        m_remainingCycles += 4;
        return m_bus.ioRead(port);
    }

    void Z80Cpu::ioWrite(const uint16_t port, const uint8_t value)
    {
        m_remainingCycles += 4;
        m_bus.ioWrite(port, value);
    }

    void Z80Cpu::mainQuadrant0()
    {
        const auto y = (m_opcode & 0b00111000) >> 3;
        const auto z = (m_opcode & 0b00000111);
        if (z == 0b000)
        {
            switch (y)
            {
            case 0b000:
                // NOP
                break;
            case 0b001:
                // EX AF, AF'
                std::swap(m_registers.af.value, m_registers.af2.value);
                break;
            case 0b010:
                // DJNZ d
                {
                    const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                    m_remainingCycles++;
                    const uint8_t b = m_registers.bc.high() - 1;
                    m_registers.bc.high(b);
                    if (b != 0)
                    {
                        m_remainingCycles += 5;
                        m_registers.pc += d;
                    }
                }
                break;
            case 0b011:
                // JR d
                jr(true);
                break;
            case 0b100:
                // JR NZ, d
                jr(m_registers.af.z() == false);
                break;
            case 0b101:
                // JR Z, d
                jr(m_registers.af.z() == true);
                break;
            case 0b110:
                // JR NC, d
                jr(m_registers.af.c() == false);
                break;
            case 0b111:
                // JR C, d
                jr(m_registers.af.c() == true);
                break;
            }
        }
        else if (z == 0b001)
        {
            switch (y)
            {
            case 0b000:
                // LD BC, nn
                m_registers.bc = fetch16();
                break;
            case 0b001:
                // ADD HL, BC
                m_remainingCycles += 7;
                setHL(add16(getHL(), m_registers.bc));
                break;
            case 0b010:
                // LD DE, nn
                m_registers.de = fetch16();
                break;
            case 0b011:
                // ADD HL, DE
                m_remainingCycles += 7;
                setHL(add16(getHL(), m_registers.de));
                break;
            case 0b100:
                // LD HL, nn
                setHL(fetch16());
                break;
            case 0b101:
                // ADD HL, HL
                m_remainingCycles += 7;
                setHL(add16(getHL(), getHL()));
                break;
            case 0b110:
                // LD SP, nn
                m_registers.sp = fetch16();
                break;
            case 0b111:
                // ADD HL, SP
                m_remainingCycles += 7;
                setHL(add16(getHL(), m_registers.sp));
                break;
            }
        }
        else if (z == 0b010)
        {
            switch (y)
            {
            case 0b000:
                // LD (BC), A
                busWrite(m_registers.bc, m_registers.af.high());
                break;
            case 0b001:
                // LD A, (BC)
                m_registers.af.high(busRead(m_registers.bc));
                break;
            case 0b010:
                // LD (DE), A
                busWrite(m_registers.de, m_registers.af.high());
                break;
            case 0b011:
                // LD A, (DE)
                m_registers.af.high(busRead(m_registers.de));
                break;
            case 0b100:
                // LD (nn), HL
                write16(fetch16(), getHL());
                break;
            case 0b101:
                // LD HL, (nn)
                setHL(read16(fetch16()));
                break;
            case 0b110:
                // LD (nn), A
                busWrite(fetch16(), m_registers.af.high());
                break;
            case 0b111:
                // LD A, (nn)
                m_registers.af.high(busRead(fetch16()));
                break;
            }
        }
        else if (z == 0b011)
        {
            switch (y)
            {
            case 0b000:
                // INC BC
                m_registers.bc.value++;
                break;
            case 0b001:
                // DEC BC
                m_registers.bc.value--;
                break;
            case 0b010:
                // INC DE
                m_registers.de.value++;
                break;
            case 0b011:
                // DEC BC
                m_registers.de.value--;
                break;
            case 0b100:
                // INC HL
                setHL(getHL() + 1);
                break;
            case 0b101:
                // DEC HL
                setHL(getHL() - 1);
                break;
            case 0b110:
                // INC SP
                m_registers.sp++;
                break;
            case 0b111:
                // DEC SP
                m_registers.sp--;
                break;
            }
            m_remainingCycles += 2;
        }
        else if (z == 0b100)
        {
            // INC 8bit
            const auto c = m_registers.af.c();
            if (y == 0b110)
            {
                // INC (HL)
                uint8_t n;
                int8_t d;
                switch (m_currentPrefix)
                {
                case Z80OpcodePrefix::none:
                    n = busRead(m_registers.hl);
                    m_remainingCycles++;
                    busWrite(m_registers.hl, n + 1);
                    break;
                case Z80OpcodePrefix::ix:
                    d = static_cast<int8_t>(busRead(m_registers.pc++));
                    m_remainingCycles += 5;
                    n = busRead(m_registers.wz = static_cast<uint16_t>(m_registers.ix + d));
                    busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.ix + d), n + 1);
                    break;
                case Z80OpcodePrefix::iy:
                    d = static_cast<int8_t>(busRead(m_registers.pc++));
                    m_remainingCycles += 5;
                    n = busRead(m_registers.wz = static_cast<uint16_t>(m_registers.iy + d));
                    busWrite(m_registers.wz= static_cast<uint16_t>(m_registers.iy + d), n + 1);
                    break;
                }
                add8(n, 1);
            }
            else
            {
                const auto n = (*m_registersPointers[static_cast<int>(m_currentPrefix)][y])++;
                add8(n, 1);
            }
            m_registers.af.c(c); // restore carry
        }
        else if (z == 0b101)
        {
            // DEC 8bit
            const auto c = m_registers.af.c();
            if (y == 0b110)
            {
                // DEC (HL)
                uint8_t n;
                int8_t d;
                switch (m_currentPrefix)
                {
                case Z80OpcodePrefix::none:
                    n = busRead(m_registers.hl);
                    m_remainingCycles++;
                    busWrite(m_registers.hl, n - 1);
                    break;
                case Z80OpcodePrefix::ix:
                    d = static_cast<int8_t>(busRead(m_registers.pc++));
                    m_remainingCycles += 5;
                    n = busRead(m_registers.wz = static_cast<uint16_t>(m_registers.ix + d));
                    busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.ix + d), n - 1);
                    break;
                case Z80OpcodePrefix::iy:
                    d = static_cast<int8_t>(busRead(m_registers.pc++));
                    m_remainingCycles += 5;
                    n = busRead(m_registers.wz = static_cast<uint16_t>(m_registers.iy + d));
                    busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.iy + d), n - 1);
                    break;
                }
                sub8(n, 1);
            }
            else
            {
                const auto n = (*m_registersPointers[static_cast<int>(m_currentPrefix)][y])--;
                sub8(n, 1);
            }
            m_registers.af.c(c); // restore carry
        }
        else if (z == 0b110)
        {
            // LD 8bit
            if (y == 0b110)
            {
                // LD (HL), n
                switch (m_currentPrefix)
                {
                case Z80OpcodePrefix::none:
                    busWrite(m_registers.hl, busRead(m_registers.pc++));
                    // m_remainingCycles++;
                    break;
                case Z80OpcodePrefix::ix:
                    {
                        const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                        const auto value = busRead(m_registers.pc++);
                        // m_remainingCycles += 5;
                        busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.ix + d), value);
                        break;
                    }
                case Z80OpcodePrefix::iy:
                    {
                        const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                        const auto value = busRead(m_registers.pc++);
                        // m_remainingCycles += 5;
                        busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.iy + d), value);
                        break;
                    }
                }
            }
            else
            {
                *m_registersPointers[static_cast<int>(m_currentPrefix)][y] = busRead(m_registers.pc++);
            }
        }
        else // if (z == 0b111)
        {
            switch (y)
            {
            case 0b000:
                // RLCA
                m_registers.af.high(std::rotl(m_registers.af.high(), 1));
                m_registers.af.y(m_registers.af & 0x2000);
                m_registers.af.h(false);
                m_registers.af.x(m_registers.af & 0x0800);
                m_registers.af.n(false);
                m_registers.af.c(m_registers.af & 0x0100);
                break;
            case 0b001:
                // RRCA
                m_registers.af.high(std::rotr(m_registers.af.high(), 1));
                m_registers.af.y(m_registers.af & 0x2000);
                m_registers.af.h(false);
                m_registers.af.x(m_registers.af & 0x0800);
                m_registers.af.n(false);
                m_registers.af.c(m_registers.af & 0x8000);
                break;
            case 0b010:
                // RLA
                {
                    const uint8_t carry = m_registers.af.c() ? 1 : 0;
                    m_registers.af.c(m_registers.af & 0x8000);
                    m_registers.af.high(static_cast<uint8_t>(m_registers.af.high() << 1) | carry);
                    m_registers.af.y(m_registers.af & 0x2000);
                    m_registers.af.h(false);
                    m_registers.af.x(m_registers.af & 0x0800);
                    m_registers.af.n(false);
                }
                break;
            case 0b011:
                // RRA
                {
                    const uint8_t carry = m_registers.af.c() ? 0x80 : 0x00;
                    m_registers.af.c(m_registers.af & 0x0100);
                    m_registers.af.high(static_cast<uint8_t>(m_registers.af.high() >> 1) | carry);
                    m_registers.af.y(m_registers.af & 0x2000);
                    m_registers.af.h(false);
                    m_registers.af.x(m_registers.af & 0x0800);
                    m_registers.af.n(false);
                }
                break;
            case 0b100:
                // DAA
                {
                    uint16_t a = m_registers.af.high();
                    if (m_registers.af.c()) a |= 1 << 8;
                    if (m_registers.af.h()) a |= 1 << 9;
                    if (m_registers.af.n()) a |= 1 << 10;
                    m_registers.af = s_daaTable[a];
                }
                break;
            case 0b101:
                // CPL
                {
                    const uint8_t result = ~m_registers.af.high();
                    m_registers.af.high(result);
                    m_registers.af.y(result & Z80Flags::y);
                    m_registers.af.x(result & Z80Flags::x);
                    m_registers.af.h(true);
                    m_registers.af.n(true);
                }
                break;
            case 0b110:
                // SCF
                m_registers.af.y(m_registers.af & (Z80Flags::y << 8));
                m_registers.af.h(false);
                m_registers.af.x(m_registers.af & (Z80Flags::x << 8));
                m_registers.af.n(false);
                m_registers.af.c(true);
                break;
            case 0b111:
                // CCF
                m_registers.af.y(m_registers.af & (Z80Flags::y << 8));
                m_registers.af.h(m_registers.af.c());
                m_registers.af.x(m_registers.af& (Z80Flags::x << 8));
                m_registers.af.n(false);
                m_registers.af.value ^= Z80Flags::c;
                break;
            }
        }
    }

    void Z80Cpu::mainQuadrant1()
    {
        const auto dst = (m_opcode & 0b00111000) >> 3;
        const auto src = (m_opcode & 0b00000111);
        if (src == 0b110)
        {
            if (dst == 0b110)
            {
                // HALT
                m_registers.pc--;
            }
            else
            {
                // LD dst, (HL)
                *m_registersPointers[0][dst] = busReadHL();
            }
        }
        else
        {
            if (dst == 0b110)
            {
                // LD (HL), src
                busWriteHL(*m_registersPointers[0][src]);
            }
            else
            {
                const uint8_t* srcPtr = m_registersPointers[static_cast<int>(m_currentPrefix)][src];
                uint8_t* dstPtr = m_registersPointers[static_cast<int>(m_currentPrefix)][dst];
                // LD dst, src
                *dstPtr = *srcPtr;
            }
        }
    }

    void Z80Cpu::mainQuadrant2()
    {
        const auto operation = (m_opcode & 0b00111000) >> 3;
        const auto src = (m_opcode & 0b00000111);
        const uint8_t* srcPtr = m_registersPointers[static_cast<int>(m_currentPrefix)][src];
        const auto a = m_registers.af.high();
        uint8_t b;
        if (src == 0b110)
        {
            b = busReadHL();
        }
        else
        {
            b = *srcPtr;
        }
        alu8(operation, a, b);
    }

    void Z80Cpu::mainQuadrant3()
    {
        const uint8_t y = (m_opcode & 0b00111000) >> 3;
        const auto z = (m_opcode & 0b00000111);
        if (z == 0b000)
        {
            // RET [cond]
            m_remainingCycles++;
            if (evaluateCondition(y))
            {
                m_registers.pc = pop16();
            }
        }
        else if (z == 0b001)
        {
            switch (y)
            {
            case 0b000:
                // POP BC
                m_registers.bc = pop16();
                break;
            case 0b001:
                // RET
                m_registers.pc = pop16();
                break;
            case 0b010:
                // POP DE
                m_registers.de = pop16();
                break;
            case 0b011:
                // EXX
                std::swap(m_registers.bc.value, m_registers.bc2.value);
                std::swap(m_registers.de.value, m_registers.de2.value);
                std::swap(m_registers.hl.value, m_registers.hl2.value);
                break;
            case 0b100:
                // POP HL
                setHL(pop16());
                break;
            case 0b101:
                // JP HL
                m_registers.pc = getHL();
                break;
            case 0b110:
                // POP AF
                m_registers.af = pop16();
                break;
            case 0b111:
                // LD SP, HL
                m_registers.sp = getHL();
                m_remainingCycles++;
                m_remainingCycles++;
                break;
            }
        }
        else if (z == 0b010)
        {
            // JP[cond] nn
            const auto nn = fetch16();
            if (evaluateCondition(y))
            {
                m_registers.pc = nn;
            }
        }
        else if (z == 0b011)
        {
            switch (y)
            {
            case 0b000:
                // JP nn
                m_registers.pc = fetch16();
                break;
            case 0b001:
                // CB prefix
                prefixCb();
                break;
            case 0b010:
                // OUT (n), A
                ioWrite(busRead(m_registers.pc++) | static_cast<uint16_t>(m_registers.af.high() << 8), m_registers.af.high());
                break;
            case 0b011:
                // IN A, (n)
                m_registers.af.high(ioRead(busRead(m_registers.pc++) | static_cast<uint16_t>(m_registers.af.high() << 8)));
                break;
            case 0b100:
                // EX (SP), HL
                {
                    const auto low = busRead(m_registers.sp);
                    m_remainingCycles++;
                    const auto high = busRead(m_registers.sp + 1);
                    const auto value = getHL();
                    busWrite(m_registers.sp, value & 0xff);
                    busWrite(m_registers.sp + 1, value >> 8);
                    m_remainingCycles += 2;
                    setHL(static_cast<uint16_t>(high << 8) | low);
                }
                break;
            case 0b101:
                // EX DE, HL
                std::swap(m_registers.de.value, m_registers.hl.value);
                break;
            case 0b110:
                // DI
                m_registers.iff1 = m_registers.iff2 = false;
                break;
            case 0b111:
                // EI
                m_registers.iff1 = m_registers.iff2 = true;
                m_registers.interruptJustEnabled = true;
                break;
            }
        }
        else if (z == 0b100)
        {
            // CALL [cond], nn
            const auto nn = fetch16();
            if (evaluateCondition(y))
            {
                m_remainingCycles++;
                push16(m_registers.pc);
                m_registers.pc = nn;
            }
        }
        else if (z == 0b101)
        {
            switch (y)
            {
            case 0b000:
                // PUSH BC
                push16(m_registers.bc);
                m_remainingCycles++;
                break;
            case 0b001:
                // CALL nn
                {
                    const auto nn = fetch16();
                    m_remainingCycles++;
                    push16(m_registers.pc);
                    m_registers.pc = nn;
                }
                break;
            case 0b010:
                // PUSH DE
                push16(m_registers.de);
                m_remainingCycles++;
                break;
            case 0b011:
                // DD prefix
                prefixDd();
                break;
            case 0b100:
                // PUSH HL
                push16(getHL());
                m_remainingCycles++;
                break;
            case 0b101:
                // ED prefix
                prefixEd();
                break;
            case 0b110:
                // PUSH AF
                push16(m_registers.af);
                m_remainingCycles++;
                break;
            case 0b111:
                // FD prefix
                prefixFd();
                break;
            }
        }
        else if (z == 0b110)
        {
            // ALU immediate
            const auto a = m_registers.af.high();
            const auto b = busRead(m_registers.pc++);
            alu8(y, a, b);
        }
        else // if (z == 0b111)
        {
            // RST xx
            m_remainingCycles++;
            push16(m_registers.pc);
            static constexpr uint16_t targets[] = {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38};
            m_registers.pc = targets[y];
        }
    }

    void Z80Cpu::prefixCb()
    {
        int8_t d = 0;
        if (m_currentPrefix != Z80OpcodePrefix::none)
        {
            m_remainingCycles += 4;
            d = static_cast<int8_t>(m_bus.read(m_registers.pc++));
        }
        m_opcode = fetchOpcode();
        const uint8_t x = m_opcode >> 6;
        const uint8_t y = (m_opcode & 0b00111000) >> 3;
        const uint8_t z = m_opcode & 0b00000111;
        if (x == 0)
        {
            const auto value = prefixCbRead(d, z);
            uint8_t result;
            switch (y)
            {
            case 0b000:
                // RLC
                result = std::rotl(value, 1);
                break;
            case 0b001:
                // RRC
                result = std::rotr(value, 1);
                break;
            case 0b010:
                // RL
                result = static_cast<uint8_t>((value << 1) | static_cast<uint8_t>(m_registers.af.c()));
                break;
            case 0b011:
                // RR
                result = static_cast<uint8_t>((value >> 1) | (m_registers.af.c() << 7));
                break;
            case 0b100:
                // SLA
                result = static_cast<uint8_t>(value << 1);
                break;
            case 0b101:
                // SRA
                result = static_cast<uint8_t>((value >> 1) | (value & 0x80));
                break;
            case 0b110:
                // SLL
                result = static_cast<uint8_t>((value << 1) | 0x01);
                break;
            case 0b111:
                // SRL
                result = static_cast<uint8_t>(value >> 1);
                break;
            }
            m_registers.af.low(s_flagsLookupSZP[result]);
            if (y & 0x01) m_registers.af.c(value & 0x01); // Right
            else  m_registers.af.c(value & 0x80); // Left
            prefixCbWrite(d, z, result);
        }
        else if (x == 1)
        {
            // BIT
            const auto value = prefixCbRead(d, z);
            const uint8_t result = value & (1 << y);
            m_registers.af.s(result & Z80Flags::s);
            m_registers.af.z(!result);
            m_registers.af.y(result & Z80Flags::y);
            m_registers.af.h(true);
            m_registers.af.x(result & Z80Flags::x);
            m_registers.af.p(!result);
            m_registers.af.n(false);
            if (m_currentPrefix != Z80OpcodePrefix::none || z == 0b110)
            {
                m_registers.af.y(m_registers.wz & (Z80Flags::y << 8));
                m_registers.af.x(m_registers.wz & (Z80Flags::x << 8));
            }
        }
        else if (x == 2)
        {
            // RES
            auto value = prefixCbRead(d, z);
            value &= ~(1 << y);
            prefixCbWrite(d, z, value);
        }
        else // if (x == 3)
        {
            // SET
            auto value = prefixCbRead(d, z);
            value |= (1 << y);
            prefixCbWrite(d, z, value);
        }
    }

    void Z80Cpu::prefixDd()
    {
        m_currentPrefix = Z80OpcodePrefix::ix;
        executeInstruction();
    }

    void Z80Cpu::prefixEd()
    {
        m_currentPrefix = Z80OpcodePrefix::none;
        m_opcode = fetchOpcode();
        const auto x = m_opcode >> 6;
        const auto y = (m_opcode & 0b00111000) >> 3;
        const auto z = (m_opcode & 0b00000111);
        if (x == 1)
        {
            // ED: quadrant 1
            if (z == 0b000)
            {
                const auto value = ioRead(m_registers.bc);
                m_registers.af.low(s_flagsLookupSZP[value]);
                switch (y)
                {
                case 0b000:
                    // IN B, (C)
                    m_registers.bc.high(value);
                    break;
                case 0b001:
                    // IN C, (C)
                    m_registers.bc.low(value);
                    break;
                case 0b010:
                    // IN D, (C)
                    m_registers.de.high(value);
                    break;
                case 0b011:
                    // IN E, (C)
                    m_registers.de.low(value);
                    break;
                case 0b100:
                    // IN H, (C)
                    m_registers.hl.high(value);
                    break;
                case 0b101:
                    // IN L, (C)
                    m_registers.hl.low(value);
                    break;
                case 0b110:
                    // IN (C)
                    break;
                case 0b111:
                    // IN A, (C)
                    m_registers.af.high(value);
                    break;
                }
            }
            else if (z == 0b001)
            {
                uint8_t value = 0;
                switch (y)
                {
                case 0b000:
                    // OUT (C), B
                    value = m_registers.bc.high();
                    break;
                case 0b001:
                    // OUT (C), C
                    value = m_registers.bc.low();
                    break;
                case 0b010:
                    // OUT (C), D
                    value = m_registers.de.high();
                    break;
                case 0b011:
                    // OUT (C), E
                    value = m_registers.de.low();
                    break;
                case 0b100:
                    // OUT (C), H
                    value = m_registers.hl.high();
                    break;
                case 0b101:
                    // OUT (C), L
                    value = m_registers.hl.low();
                    break;
                case 0b110:
                    // OUT (C)
                    break;
                case 0b111:
                    // OUT (C), A
                    value = m_registers.af.high();
                    break;
                }
                ioWrite(m_registers.bc, value);
            }
            else if (z == 0b010)
            {
                switch (y)
                {
                case 0b000:
                    // SBC HL, BC
                    m_registers.hl = sub16(m_registers.hl, m_registers.bc, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b001:
                    // ADC HL, BC
                    m_registers.hl = add16(m_registers.hl, m_registers.bc, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b010:
                    // SBC HL, DE
                    m_registers.hl = sub16(m_registers.hl, m_registers.de, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b011:
                    // ADC HL, DE
                    m_registers.hl = add16(m_registers.hl, m_registers.de, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b100:
                    // SBC HL, HL
                    m_registers.hl = sub16(m_registers.hl, m_registers.hl, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b101:
                    // ADC HL, HL
                    m_registers.hl = add16(m_registers.hl, m_registers.hl, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b110:
                    // SBC HL, SP
                    m_registers.hl = sub16(m_registers.hl, m_registers.sp, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                case 0b111:
                    // ADC HL, SP
                    m_registers.hl = add16(m_registers.hl, m_registers.sp, m_registers.af.c());
                    m_remainingCycles += 7;
                    break;
                }
            }
            else if (z == 0b011)
            {
                const auto nn = fetch16();
                switch (y)
                {
                case 0b000:
                    // LD (nn), BC
                    write16(nn, m_registers.bc);
                    break;
                case 0b001:
                    // LD BC, (nn)
                    m_registers.bc = read16(nn);
                    break;
                case 0b010:
                    // LD (nn), DE
                    write16(nn, m_registers.de);
                    break;
                case 0b011:
                    // LD DE, (nn)
                    m_registers.de = read16(nn);
                    break;
                case 0b100:
                    // LD (nn), HL
                    write16(nn, m_registers.hl);
                    break;
                case 0b101:
                    // LD HL, (nn)
                    m_registers.hl = read16(nn);
                    break;
                case 0b110:
                    // LD (nn), SP
                    write16(nn, m_registers.sp);
                    break;
                case 0b111:
                    // LD SP, (nn)
                    m_registers.sp = read16(nn);
                    break;
                }
            }
            else if (z == 0b100)
            {
                // NEG
                m_registers.af.high(sub8(0, m_registers.af.high()));
            }
            else if (z == 0b101)
            {
                // RETI/RETN
                m_registers.iff1 = m_registers.iff2;
                m_registers.pc = pop16();
            }
            else if (z == 0b110)
            {
                switch (y)
                {
                case 0b000:
                case 0b001:
                case 0b100:
                case 0b101:
                    m_registers.interruptMode = 0;
                    break;
                case 0b010:
                case 0b110:
                    m_registers.interruptMode = 1;
                    break;
                case 0b011:
                case 0b111:
                    m_registers.interruptMode = 2;
                    break;
                }
            }
            else if (z == 0b111)
            {
                switch (y)
                {
                case 0b000:
                    // LD I, A
                    m_registers.ir.high(m_registers.af.high());
                    m_remainingCycles++;
                    break;
                case 0b001:
                    // LD R, A
                    m_registers.ir.low(m_registers.af.high());
                    m_remainingCycles++;
                    break;
                case 0b010:
                    // LD A, I
                    {
                        const auto value = m_registers.ir.high();
                        m_registers.af.high(value);
                        m_registers.af.s(value & Z80Flags::s);
                        m_registers.af.z(value == 0);
                        m_registers.af.y(value & Z80Flags::y);
                        m_registers.af.h(false);
                        m_registers.af.x(value & Z80Flags::x);
                        m_registers.af.p(m_registers.iff2);
                        m_registers.af.n(false);
                        m_remainingCycles++;
                    }
                    break;
                case 0b011:
                    // LD A, R
                    {
                        const auto value = m_registers.ir.low();
                        m_registers.af.high(value);
                        m_registers.af.s(value & Z80Flags::s);
                        m_registers.af.z(value == 0);
                        m_registers.af.y(value & Z80Flags::y);
                        m_registers.af.h(false);
                        m_registers.af.x(value& Z80Flags::x);
                        m_registers.af.p(m_registers.iff2);
                        m_registers.af.n(false);
                        m_remainingCycles++;
                    }
                    break;
                case 0b100:
                    // RRD
                    m_remainingCycles += 4;
                    {
                        const auto a = m_registers.af.high();
                        const auto n = busRead(m_registers.hl);
                        const auto res = static_cast<uint8_t>((a & 0xf0) | (n & 0x0f));
                        m_registers.af.high(res);
                        busWrite(m_registers.hl, static_cast<uint8_t>(a << 4 | n >> 4));
                        m_registers.af.low((m_registers.af.value & 0x01) | s_flagsLookupSZP[res]);
                    }
                    break;
                case 0b101:
                    // RLD
                    m_remainingCycles += 4;
                    {
                        const auto a = m_registers.af.high();
                        const auto n = busRead(m_registers.hl);
                        const auto res = static_cast<uint8_t>((a & 0xf0) | n >> 4);
                        m_registers.af.high(res);
                        busWrite(m_registers.hl, static_cast<uint8_t>(n << 4 | (a & 0x0f)));
                        m_registers.af.low((m_registers.af.value & 0x01) | s_flagsLookupSZP[res]);
                    }
                    break;
                case 0b110:
                case 0b111:
                    // ED NOP
                    break;
                }
            }
        }
        else if (x == 2)
        {
            // ED: quadrant 2
            if (z == 0b000)
            {
                switch (y)
                {
                case 0b100:
                    // LDI
                    ldi();
                    break;
                case 0b101:
                    // LDD
                    ldd();
                    break;
                case 0b110:
                    // LDIR
                    ldi();
                    if (m_registers.af.p())
                    {
                        m_registers.pc -= 2;
                        m_remainingCycles += 5;
                    }
                    break;
                case 0b111:
                    // LDDR
                    ldd();
                    if (m_registers.af.p())
                    {
                        m_registers.pc -= 2;
                        m_remainingCycles += 5;
                    }
                    break;
                    // Rest is NOP
                }
            }
            else if (z == 0b001)
            {
                switch (y)
                {
                case 0b100:
                    // CPI
                    cpi();
                    break;
                case 0b101:
                    // CPD
                    cpd();
                    break;
                case 0b110:
                    // CPIR
                    cpi();
                    if (m_registers.af.p() && !m_registers.af.z())
                    {
                        m_registers.pc -= 2;
                        m_remainingCycles += 5;
                    }
                    break;
                case 0b111:
                    // CPDR
                    cpd();
                    if (m_registers.af.p() && !m_registers.af.z())
                    {
                        m_registers.pc -= 2;
                        m_remainingCycles += 5;
                    }
                    break;
                    // Rest is NOP
                }
            }
            else if (z == 0b010)
            {
                // TODO INI IND INIR INDR
                assert(false);
            }
            else if (z == 0b011)
            {
                // TODO OUTI OUTD OTIR OTDR
                assert(false);
            }
            // Rest is NOP
        }
    }

    void Z80Cpu::prefixFd()
    {
        m_currentPrefix = Z80OpcodePrefix::iy;
        executeInstruction();
    }

    uint16_t Z80Cpu::getHL() const
    {
        switch (m_currentPrefix)
        {
        case Z80OpcodePrefix::none: return m_registers.hl;
        case Z80OpcodePrefix::ix:   return m_registers.ix;
        case Z80OpcodePrefix::iy:   return m_registers.iy;
        }
        assert(false);
        return 0;
    }

    void Z80Cpu::setHL(const uint16_t value)
    {
        switch (m_currentPrefix)
        {
        case Z80OpcodePrefix::none: m_registers.hl = value; break;
        case Z80OpcodePrefix::ix:   m_registers.ix = value; break;
        case Z80OpcodePrefix::iy:   m_registers.iy = value; break;
        }
    }

    uint8_t Z80Cpu::busReadHL()
    {
        switch (m_currentPrefix)
        {
        case Z80OpcodePrefix::none:
            return busRead(m_registers.hl);
        case Z80OpcodePrefix::ix:
            {
                const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                m_remainingCycles += 5;
                return busRead(m_registers.wz = static_cast<uint16_t>(m_registers.ix + d));
            }
        case Z80OpcodePrefix::iy:
            {
                const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                m_remainingCycles += 5;
                return busRead(m_registers.wz = static_cast<uint16_t>(m_registers.iy + d));
            }
        }
        assert(false);
        return 0;
    }

    void Z80Cpu::busWriteHL(const uint8_t value)
    {
        switch (m_currentPrefix)
        {
        case Z80OpcodePrefix::none:
            return busWrite(m_registers.hl, value);
        case Z80OpcodePrefix::ix:
            {
                const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                m_remainingCycles += 5;
                return busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.ix + d), value);
            }
        case Z80OpcodePrefix::iy:
            {
                const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
                m_remainingCycles += 5;
                return busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.iy + d), value);
            }
        }
        assert(false);
    }

    uint16_t Z80Cpu::fetch16()
    {
        const auto low = busRead(m_registers.pc++);
        const auto high = busRead(m_registers.pc++);
        return static_cast<uint16_t>(high << 8) | low;
    }

    uint16_t Z80Cpu::read16(const uint16_t address)
    {
        const auto low = busRead(address);
        const auto high = busRead(address + 1);
        return static_cast<uint16_t>(high << 8) | low;
    }

    void Z80Cpu::write16(const uint16_t address, const uint16_t value)
    {
        busWrite(address, value & 0xff);
        busWrite(address + 1, value >> 8);
    }

    uint8_t Z80Cpu::add8(const uint8_t a, const uint8_t b, const bool carryFlag)
    {
        uint8_t result;
        bool carry;
        if (carryFlag)
        {
            result = a + b + 1;
            carry = a >= 0xff - b;
        }
        else
        {
            result = a + b;
            carry = a > 0xff - b;
        }
        const auto carryIn = result ^ a ^ b;
        const auto overflow = (carryIn >> 7) ^ static_cast<uint8_t>(carry);
        m_registers.af.s(result >> 7);
        m_registers.af.z(result == 0);
        m_registers.af.y(result & Z80Flags::y);
        m_registers.af.h((carryIn >> 4) & 0x01);
        m_registers.af.x(result & Z80Flags::x);
        m_registers.af.p(overflow);
        m_registers.af.n(false);
        m_registers.af.c(carry);
        return result;
    }

    uint8_t Z80Cpu::sub8(const uint8_t a, const uint8_t b, const bool carryFlag)
    {
        const auto result = add8(a, ~b, !carryFlag);
        m_registers.af.value ^= Z80Flags::h | Z80Flags::n | Z80Flags::c; // invert HNC
        return result;
    }

    uint16_t Z80Cpu::add16(const uint16_t a, const uint16_t b)
    {
        const uint16_t lowResult = (a & 0xff) + (b & 0xff);
        const bool lowCarry = lowResult & 0x100;
        const auto highA = a >> 8;
        const auto highB = b >> 8;
        const auto highResult = highA + highB + lowCarry;
        bool carry;
        if (lowCarry)
        {
            carry = highA >= 0xff - highB;
        }
        else
        {
            carry = highA > 0xff - highB;
        }
        const auto carryIn = (highResult & 0xff) ^ highA ^ highB;
        const auto result = ((highResult & 0xff) << 8) | (lowResult & 0xff);
        m_registers.af.y(highResult & Z80Flags::y);
        m_registers.af.h((carryIn >> 4) & 0x01);
        m_registers.af.x(highResult & Z80Flags::x);
        m_registers.af.n(false);
        m_registers.af.c(carry);
        return static_cast<uint16_t>(result);
    }

    uint16_t Z80Cpu::add16(const uint16_t a, const uint16_t b, const bool carryFlag)
    {
        const uint16_t lowResult = (a & 0xff) + (b & 0xff) + carryFlag;
        const bool lowCarry = lowResult & 0x100;
        const auto highA = a >> 8;
        const auto highB = b >> 8;
        const auto highResult = highA + highB + lowCarry;
        bool carry;
        if (lowCarry)
        {
            carry = highA >= 0xff - highB;
        }
        else
        {
            carry = highA > 0xff - highB;
        }
        const auto carryIn = (highResult & 0xff) ^ highA ^ highB;
        const auto overflow = (carryIn >> 7) ^ static_cast<uint8_t>(carry);
        const auto result = ((highResult & 0xff) << 8) | (lowResult & 0xff);
        m_registers.af.s(result & 0x8000);
        m_registers.af.z(result == 0);
        m_registers.af.y(highResult & Z80Flags::y);
        m_registers.af.h((carryIn >> 4) & 0x01);
        m_registers.af.x(highResult & Z80Flags::x);
        m_registers.af.p(overflow);
        m_registers.af.n(false);
        m_registers.af.c(carry);
        return static_cast<uint16_t>(result);
    }

    uint16_t Z80Cpu::sub16(uint16_t a, uint16_t b)
    {
        const auto result = add16(a, ~b + 1);
        m_registers.af.value ^= Z80Flags::h | Z80Flags::n | Z80Flags::c; // invert HNC
        return result;
    }

    uint16_t Z80Cpu::sub16(const uint16_t a, const uint16_t b, const bool carryFlag)
    {
        const auto result = add16(a, ~b, !carryFlag);
        m_registers.af.value ^= Z80Flags::h | Z80Flags::n | Z80Flags::c; // invert HNC
        return result;
    }

    void Z80Cpu::alu8(const int operation, const uint8_t a, const uint8_t b)
    {
        switch (operation)
        {
        case 0b000:
            // ADD
            m_registers.af.high(add8(a, b));
            return;
        case 0b001:
            // ADC
            m_registers.af.high(add8(a, b, m_registers.af.c()));
            return;
        case 0b010:
            // SUB
            m_registers.af.high(sub8(a, b));
            return;
        case 0b011:
            // SBC
            m_registers.af.high(sub8(a, b, m_registers.af.c()));
            return;
        case 0b100:
            // AND
            {
                const uint8_t result = a & b;
                m_registers.af.high(result);
                m_registers.af.low(s_flagsLookupSZP[result] | Z80Flags::h);
            }
            return;
        case 0b101:
            // XOR
            {
                const uint8_t result = a ^ b;
                m_registers.af.high(result);
                m_registers.af.low(s_flagsLookupSZP[result]);
            }
            return;
        case 0b110:
            // OR
            {
                const uint8_t result = a | b;
                m_registers.af.high(result);
                m_registers.af.low(s_flagsLookupSZP[result]);
            }
            return;
        case 0b111:
            // CP
            sub8(a, b);
            m_registers.af.y(b & Z80Flags::y);
            m_registers.af.x(b & Z80Flags::x);
            return;
        }
        assert(false);
    }

    bool Z80Cpu::evaluateCondition(const int condition) const
    {
        switch (condition)
        {
        case 0b000:
            // NZ
            return m_registers.af.z() == false;
        case 0b001:
            // Z
            return m_registers.af.z() == true;
        case 0b010:
            // NC
            return m_registers.af.c() == false;
        case 0b011:
            // C
            return m_registers.af.c() == true;
        case 0b100:
            // PO
            return m_registers.af.p() == false;
        case 0b101:
            // PE
            return m_registers.af.p() == true;
        case 0b110:
            // P
            return m_registers.af.s() == false;
        case 0b111:
            // M
            return m_registers.af.s() == true;
        }
        assert(false);
        return false;
    }

    void Z80Cpu::jr(const bool condition)
    {
        const auto d = static_cast<int8_t>(busRead(m_registers.pc++));
        if (condition)
        {
            m_remainingCycles += 5;
            m_registers.pc += d;
        }
    }

    void Z80Cpu::push16(const uint16_t value)
    {
        busWrite(--m_registers.sp, value >> 8);
        busWrite(--m_registers.sp, value & 0xff);
    }

    uint16_t Z80Cpu::pop16()
    {
        const auto low = busRead(m_registers.sp++);
        const auto high = busRead(m_registers.sp++);
        return static_cast<uint16_t>(high << 8) | low;
    }

    void Z80Cpu::ldi()
    {
        const auto n = busRead(m_registers.hl.value++);
        busWrite(m_registers.de.value++, n);
        m_registers.bc.value--;
        const uint8_t an = n + m_registers.af.high();
        m_registers.af.y(an & (1 << 1));
        m_registers.af.h(false);
        m_registers.af.x(an & (1 << 3));
        m_registers.af.n(false);
        m_registers.af.p(m_registers.bc.value);
        m_remainingCycles += 2;
    }

    void Z80Cpu::ldd()
    {
        const auto n = busRead(m_registers.hl.value--);
        busWrite(m_registers.de.value--, n);
        m_registers.bc.value--;
        const uint8_t an = n + m_registers.af.high();
        m_registers.af.y(an & (1 << 1));
        m_registers.af.h(false);
        m_registers.af.x(an & (1 << 3));
        m_registers.af.n(false);
        m_registers.af.p(m_registers.bc.value);
        m_remainingCycles += 2;
    }

    void Z80Cpu::cpi()
    {
        const auto c = m_registers.af.c();
        const auto a = m_registers.af.high();
        const auto b = busRead(m_registers.hl.value++);
        auto n = sub8(a, b);
        n -= m_registers.af.h(); // Use HF set by sub8
        m_registers.af.y(n & (1 << 1));
        m_registers.af.x(n & (1 << 3));
        m_registers.bc.value--;
        m_registers.af.p(m_registers.bc.value);
        m_registers.af.c(c);
        m_remainingCycles += 5;
    }

    void Z80Cpu::cpd()
    {
        const auto c = m_registers.af.c();
        const auto a = m_registers.af.high();
        const auto b = busRead(m_registers.hl.value--);
        auto n = sub8(a, b);
        n -= m_registers.af.h(); // Use HF set by sub8
        m_registers.af.y(n & (1 << 1));
        m_registers.af.x(n & (1 << 3));
        m_registers.bc.value--;
        m_registers.af.p(m_registers.bc.value);
        m_registers.af.c(c);
        m_remainingCycles += 5;
    }

    uint8_t Z80Cpu::prefixCbRead(const int8_t d, const int z)
    {
        if (z == 0b110)
        {
            m_remainingCycles++;
            // (HL)
            switch (m_currentPrefix)
            {
            case Z80OpcodePrefix::none:
                return busRead(m_registers.hl);
            case Z80OpcodePrefix::ix:
                return busRead(m_registers.wz = static_cast<uint16_t>(m_registers.ix + d));
            case Z80OpcodePrefix::iy:
                return busRead(m_registers.wz = static_cast<uint16_t>(m_registers.iy + d));
            }
            assert(false);
            return 0;
        }
        else
        {
            return *m_registersPointers[0][z];
        }
    }

    void Z80Cpu::prefixCbWrite(const int8_t d, const int z, const uint8_t value)
    {
        if (z == 0b110)
        {
            //  m_remainingCycles++; // Already in prefixCbRead
            // (HL)
            switch (m_currentPrefix)
            {
            case Z80OpcodePrefix::none:
                busWrite(m_registers.hl, value);
                break;
            case Z80OpcodePrefix::ix:
                busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.ix + d), value);
                break;
            case Z80OpcodePrefix::iy:
                busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.iy + d), value);
                break;
            }
        }
        else
        {
            *m_registersPointers[0][z] = value;
            switch (m_currentPrefix)
            {
            case Z80OpcodePrefix::ix:
                busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.ix + d), value);
                break;
            case Z80OpcodePrefix::iy:
                busWrite(m_registers.wz = static_cast<uint16_t>(m_registers.iy + d), value);
                break;
            }
        }
    }
}

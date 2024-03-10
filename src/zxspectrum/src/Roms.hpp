/* This file is part of Epoch, Copyright (C) 2024 Andrea Ghidini.
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

#ifndef SRC_EPOCH_ZXSPECTRUM_ROM_HPP_
#define SRC_EPOCH_ZXSPECTRUM_ROM_HPP_

#include <cstdint>

/*
 * Amstrad have kindly given their permission for the redistribution of their copyrighted material but retain that
 * copyright. See https://groups.google.com/g/comp.sys.amstrad.8bit/c/HtpBU2Bzv_U/m/HhNDSU3MksAJ
 */

namespace epoch::zxspectrum::roms
{
    // 48K rom downloaded from https://mdfs.net/Software/Spectrum/ROMImages/48k.rom
    extern const uint8_t Rom48K[16 * 1024];

    // 128K rom downloaded from https://mdfs.net/Software/Spectrum/ROMImages/128k.rom
    extern const uint8_t Rom128K[32 * 1024];

    // 128K+2 rom downloaded from https://mdfs.net/Software/Spectrum/ROMImages/128k+2.rom
    extern const uint8_t Rom128KPlus2[32 * 1024];

    // 128K+3 rom downloaded from https://mdfs.net/Software/Spectrum/ROMImages/128k+3.rom
    extern const uint8_t Rom128KPlus3[64 * 1024];

    extern const uint8_t RomZXTest[8 * 1024];
}  // namespace epoch::zxspectrum::roms

#endif

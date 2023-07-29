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

#ifndef SRC_EPOCH_ZXSPECTRUM_IOSNAPSHOT_H_
#define SRC_EPOCH_ZXSPECTRUM_IOSNAPSHOT_H_

#include <filesystem>

namespace epoch::zxspectrum
{
    class ZXSpectrumEmulator;

    void loadSna(const std::filesystem::path& path, ZXSpectrumEmulator* emulator);
    void loadZ80(const std::filesystem::path& path, ZXSpectrumEmulator* emulator);

    void saveSna(const std::filesystem::path& path, const ZXSpectrumEmulator* emulator);
}

#endif

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

#include "Io.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <vector>

#include "IoSnapshot.hpp"
#include "IoTzx.hpp"
#include "TapeInterface.hpp"
#include "Ula.hpp"
#include "ZXSpectrumEmulator.hpp"

#define MAKE_WORD(high, low) static_cast<uint16_t>((high) << 8 | (low))
#define GET_BYTE() static_cast<uint8_t>(is.get())
#define GET_WORD_LE() do { low = GET_BYTE(); high = GET_BYTE(); } while (false)

namespace epoch::zxspectrum
{
    std::vector<std::size_t> loadTap(const std::filesystem::path& path)
    {
        std::vector<std::size_t> result{};

        std::ifstream is(path, std::ios::binary);

        bool first = true;

        while (!is.eof())
        {
            if (!first)
            {
                result.push_back(1750000); // TODO
                result.push_back(1750000); // TODO
            }

            uint8_t high, low;
            GET_WORD_LE();
            if (is.eof()) break;
            const auto blockSize = MAKE_WORD(high, low);
            assert(blockSize > 1);
            std::vector<uint8_t> data;
            data.resize(blockSize);
            is.read(reinterpret_cast<char*>(data.data()), blockSize);

            // Pilot
            if (data[0] < 128)
            {
                // header block
                for (auto i = 0; i < 8063; i++)
                {
                    result.push_back(2168);
                }
            }
            else
            {
                // data block
                for (auto i = 0; i < 3223; i++)
                {
                    result.push_back(2168);
                }
            }

            // Sync
            result.push_back(667);
            result.push_back(735);

            // Data
            for (const auto value : data)
            {
                for (auto bit = 7; bit >= 0; bit--)
                {
                    result.push_back((value & (1 << bit)) ? 1710 : 855);
                    result.push_back((value & (1 << bit)) ? 1710 : 855);
                }
            }

            first = false;
        }
        return result;
    }

    std::vector<std::size_t> loadTzx(const std::filesystem::path& path)
    {
        std::vector<std::size_t> result{};

        std::ifstream is(path, std::ios::binary);

        loadTzx(is, result);

        return result;
    }

    std::unique_ptr<TapeInterface> load(const std::string& path, ZXSpectrumEmulator* emulator)
    {
        assert(emulator);
        const std::filesystem::path fs{ path };
        auto ext = fs.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](const char c) { return std::tolower(c); });
        if (ext == ".sna")
        {
            emulator->reset();
            loadSna(fs, emulator);
            return nullptr;
        }
        else if (ext == ".z80")
        {
            emulator->reset();
            loadZ80(fs, emulator);
            return nullptr;
        }
        else if (ext == ".tap")
        {
            const auto pulses = loadTap(fs);
            return std::make_unique<TapeInterface>(pulses);
        }
        else if (ext == ".tzx")
        {
            const auto pulses = loadTzx(fs);
            return std::make_unique<TapeInterface>(pulses);
        }
    }

    void save(const std::string& path, const ZXSpectrumEmulator* emulator)
    {
        assert(emulator);
        const std::filesystem::path fs{ path };
        auto ext = fs.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](const char c) { return std::tolower(c); });
        if (ext == ".sna")
        {
            saveSna(fs, emulator);
        }
    }
}

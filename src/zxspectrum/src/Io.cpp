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

#include "Io.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <vector>

#include "IoTzx.h"
#include "TapeInterface.h"
#include "Ula.h"
#include "ZXSpectrumEmulator.h"
#include "Z80Cpu.h"

#define PUT_BYTE(x) os.put(static_cast<uint8_t>(x))
#define MAKE_WORD(high, low) static_cast<uint16_t>((high) << 8 | (low))
#define GET_BYTE() static_cast<uint8_t>(is.get())
#define GET_WORD_LE() do { low = GET_BYTE(); high = GET_BYTE(); } while (false)

namespace epoch::zxspectrum
{
    static void loadSna(const std::filesystem::path& path, ZXSpectrumEmulator* emulator)
    {
        const auto ula = emulator->ula();
        auto& registers = emulator->cpu()->registers();

        std::ifstream is(path, std::ios::binary);
        uint8_t high, low;
        low = GET_BYTE();
        registers.ir = static_cast<uint16_t>(low << 8);
        GET_WORD_LE();
        registers.hl2 = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.de2 = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.bc2 = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.af2 = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.hl = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.de = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.bc = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.iy = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.ix = MAKE_WORD(high, low);
        low = GET_BYTE();
        registers.iff1 = registers.iff2 = low & (1 << 2);
        low = GET_BYTE();
        registers.ir.value |= low;
        GET_WORD_LE();
        registers.af = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.sp = MAKE_WORD(high, low);
        low = GET_BYTE();
        registers.interruptMode = low;
        // Border color
        low = GET_BYTE();
        ula->ioWrite(0xfe, low);
        // Load memory
        is.read(reinterpret_cast<char*>(emulator->ram()[5].data()), ZXSpectrumEmulator::MemoryBankSize);
        is.read(reinterpret_cast<char*>(emulator->ram()[2].data()), ZXSpectrumEmulator::MemoryBankSize);
        is.read(reinterpret_cast<char*>(emulator->ram()[0].data()), ZXSpectrumEmulator::MemoryBankSize);
        // Pop PC from the stack
        registers.pc = static_cast<uint16_t>(ula->read(registers.sp) | (ula->read(registers.sp + 1) << 8));
        registers.sp += 2;
    }

    static void saveSna(const std::filesystem::path& path, const ZXSpectrumEmulator* emulator)
    {
        const auto& registers = emulator->cpu()->registers();

        std::ofstream os(path, std::ios::binary);
        PUT_BYTE(registers.ir.high());
        PUT_BYTE(registers.hl2.low());
        PUT_BYTE(registers.hl2.high());
        PUT_BYTE(registers.de2.low());
        PUT_BYTE(registers.de2.high());
        PUT_BYTE(registers.bc2.low());
        PUT_BYTE(registers.bc2.high());
        PUT_BYTE(registers.af2.low());
        PUT_BYTE(registers.af2.high());
        PUT_BYTE(registers.hl.low());
        PUT_BYTE(registers.hl.high());
        PUT_BYTE(registers.de.low());
        PUT_BYTE(registers.de.high());
        PUT_BYTE(registers.bc.low());
        PUT_BYTE(registers.bc.high());
        PUT_BYTE(registers.iy.low());
        PUT_BYTE(registers.iy.high());
        PUT_BYTE(registers.ix.low());
        PUT_BYTE(registers.ix.high());
        PUT_BYTE(registers.iff2 ? (1 << 2) : 0);
        PUT_BYTE(registers.ir.low());
        PUT_BYTE(registers.af.low());
        PUT_BYTE(registers.af.high());
        auto sp = registers.sp;
        emulator->ula()->write(--sp, registers.pc >> 8);
        emulator->ula()->write(--sp, registers.pc & 0xff);
        PUT_BYTE(static_cast<char>(sp & 0xff));
        PUT_BYTE(static_cast<char>(sp >> 8));
        PUT_BYTE(registers.interruptMode);
        PUT_BYTE(emulator->ula()->border());
        os.write(reinterpret_cast<const char*>(emulator->ram()[5].data()), ZXSpectrumEmulator::MemoryBankSize);
        os.write(reinterpret_cast<const char*>(emulator->ram()[2].data()), ZXSpectrumEmulator::MemoryBankSize);
        os.write(reinterpret_cast<const char*>(emulator->ram()[0].data()), ZXSpectrumEmulator::MemoryBankSize);
    }

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

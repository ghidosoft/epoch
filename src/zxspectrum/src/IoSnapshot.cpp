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

#include "IoSnapshot.hpp"

#include <cassert>
#include <cstring>
#include <fstream>
#include <vector>

#include "Ula.hpp"
#include "ZXSpectrumEmulator.hpp"
#include "Z80Cpu.hpp"

#define PUT_BYTE(x) os.put(static_cast<uint8_t>(x))
#define MAKE_WORD(high, low) static_cast<uint16_t>((high) << 8 | (low))
#define GET_BYTE() static_cast<uint8_t>(is.get())
#define GET_WORD_LE() do { low = GET_BYTE(); high = GET_BYTE(); } while (false)

namespace epoch::zxspectrum
{
    void loadSna(const std::filesystem::path& path, ZXSpectrumEmulator* emulator)
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
        registers.ir = (registers.ir | low);
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
        is.read(reinterpret_cast<char*>(emulator->ram()[5].data()), MemoryBankSize);
        is.read(reinterpret_cast<char*>(emulator->ram()[2].data()), MemoryBankSize);
        is.read(reinterpret_cast<char*>(emulator->ram()[0].data()), MemoryBankSize);
        // Pop PC from the stack
        registers.pc = static_cast<uint16_t>(ula->read(registers.sp) | (ula->read(registers.sp + 1) << 8));
        registers.sp += 2;
    }

    void loadZ80(const std::filesystem::path& path, ZXSpectrumEmulator* emulator)
    {
        const auto ula = emulator->ula();
        auto& registers = emulator->cpu()->registers();

        std::ifstream is(path, std::ios::binary);
        uint8_t high, low;

        registers.af.high = GET_BYTE();
        registers.af.low = GET_BYTE();
        GET_WORD_LE();
        registers.bc = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.hl = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.pc = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.sp = MAKE_WORD(high, low);
        registers.ir.high = GET_BYTE();
        registers.ir.low = GET_BYTE() & 0x7f;

        low = GET_BYTE();
        if (low == 0xff) low = 0x01; // Because of compatibility, if byte 12 is 255, it has to be regarded as being 1.
        registers.ir = (registers.ir | ((low & 0x01) << 7));
        const bool basicSamRomv1 = low & (1 << 4);
        const bool compressedv1 = low & (1 << 5);
        const auto border = static_cast<uint8_t>((low >> 1) & 0b111);
        ula->ioWrite(0xfe, border);

        GET_WORD_LE();
        registers.de = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.bc2 = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.de2 = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.hl2 = MAKE_WORD(high, low);
        registers.af2.high = GET_BYTE();
        registers.af2.low =GET_BYTE();
        GET_WORD_LE();
        registers.iy = MAKE_WORD(high, low);
        GET_WORD_LE();
        registers.ix = MAKE_WORD(high, low);

        registers.iff1 = GET_BYTE();
        registers.iff2 = GET_BYTE();

        low = GET_BYTE();
        registers.interruptMode = low & 0b11;

        assert(registers.pc == 0); // unsupported z80 version 1
        if (registers.pc == 0)
        {
            auto pos = is.tellg();
            // version 2/3
            GET_WORD_LE();
            const auto additionalHeaderLength = MAKE_WORD(high, low);
            GET_WORD_LE();
            registers.pc = MAKE_WORD(high, low);
            const auto hardwareMode = GET_BYTE();
            assert(hardwareMode == 0 || hardwareMode == 1); // Only ZX Spectrum 48K supported

            pos += additionalHeaderLength + 2;
            is.seekg(pos, std::ios::beg);

            do
            {
                GET_WORD_LE();
                if (is.eof()) break;
                const auto blockLength = MAKE_WORD(high, low);
                assert(blockLength != 0xffff); // uncompressed not yet supported
                const auto pageNumber = GET_BYTE();
                uint8_t* page{};
                if (pageNumber == 0x04)
                {
                    page = emulator->ram()[2].data();
                }
                else if (pageNumber == 0x05)
                {
                    page = emulator->ram()[0].data();
                }
                else if (pageNumber == 0x08)
                {
                    page = emulator->ram()[5].data();
                }
                assert(page);
                std::vector<uint8_t> buffer;
                buffer.resize(blockLength);
                is.read(reinterpret_cast<char*>(buffer.data()), blockLength);
                auto dest = 0;
                auto i = 0;
                while (i < blockLength)
                {
                    if (i < blockLength - 4 && buffer[i] == 0xed && buffer[i + 1] == 0xed)
                    {
                        i += 2;
                        const auto repeat = buffer[i++];
                        const auto value = buffer[i++];
                        std::memset(&page[dest], value, repeat);
                        dest += repeat;
                    }
                    else
                    {
                        page[dest++] = buffer[i++];
                    }
                }
                assert(i == blockLength);
                assert(dest == 0x4000);
            } while (!is.eof());
        }
    }

    void saveSna(const std::filesystem::path& path, const ZXSpectrumEmulator* emulator)
    {
        const auto& registers = emulator->cpu()->registers();

        std::ofstream os(path, std::ios::binary);
        PUT_BYTE(registers.ir.high);
        PUT_BYTE(registers.hl2.low);
        PUT_BYTE(registers.hl2.high);
        PUT_BYTE(registers.de2.low);
        PUT_BYTE(registers.de2.high);
        PUT_BYTE(registers.bc2.low);
        PUT_BYTE(registers.bc2.high);
        PUT_BYTE(registers.af2.low);
        PUT_BYTE(registers.af2.high);
        PUT_BYTE(registers.hl.low);
        PUT_BYTE(registers.hl.high);
        PUT_BYTE(registers.de.low);
        PUT_BYTE(registers.de.high);
        PUT_BYTE(registers.bc.low);
        PUT_BYTE(registers.bc.high);
        PUT_BYTE(registers.iy.low);
        PUT_BYTE(registers.iy.high);
        PUT_BYTE(registers.ix.low);
        PUT_BYTE(registers.ix.high);
        PUT_BYTE(registers.iff2 ? (1 << 2) : 0);
        PUT_BYTE(registers.ir.low);
        PUT_BYTE(registers.af.low);
        PUT_BYTE(registers.af.high);
        auto sp = registers.sp;
        emulator->ula()->write(--sp, registers.pc >> 8);
        emulator->ula()->write(--sp, registers.pc & 0xff);
        PUT_BYTE(static_cast<char>(sp & 0xff));
        PUT_BYTE(static_cast<char>(sp >> 8));
        PUT_BYTE(registers.interruptMode);
        PUT_BYTE(emulator->ula()->border());
        os.write(reinterpret_cast<const char*>(emulator->ram()[5].data()), MemoryBankSize);
        os.write(reinterpret_cast<const char*>(emulator->ram()[2].data()), MemoryBankSize);
        os.write(reinterpret_cast<const char*>(emulator->ram()[0].data()), MemoryBankSize);
    }
}

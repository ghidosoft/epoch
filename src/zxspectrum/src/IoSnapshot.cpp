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

#include "IoUtils.hpp"
#include "Ula.hpp"
#include "Z80Cpu.hpp"
#include "ZXSpectrumEmulator.hpp"

#include <cassert>
#include <cstring>
#include <fstream>
#include <vector>

#define PUT_BYTE(x) os.put(static_cast<uint8_t>(x))

namespace epoch::zxspectrum
{
    void loadSna(const std::filesystem::path& path, ZXSpectrumEmulator* emulator)
    {
        const auto ula = emulator->ula();
        auto& registers = emulator->cpu()->registers();

        std::ifstream is{path, std::ios::binary};
        const StreamReader reader{is};
        registers.ir.high = reader.readUInt8();
        registers.hl2 = reader.readUInt16LE();
        registers.de2 = reader.readUInt16LE();
        registers.bc2 = reader.readUInt16LE();
        registers.af2 = reader.readUInt16LE();
        registers.hl = reader.readUInt16LE();
        registers.de = reader.readUInt16LE();
        registers.bc = reader.readUInt16LE();
        registers.iy = reader.readUInt16LE();
        registers.ix = reader.readUInt16LE();
        registers.iff1 = registers.iff2 = reader.readUInt8() & (1 << 2);
        registers.ir.low = reader.readUInt8();
        registers.af = reader.readUInt16LE();
        registers.sp = reader.readUInt16LE();
        registers.interruptMode = reader.readUInt8();
        // Border color
        ula->ioWrite(0xfe, reader.readUInt8());
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

        std::ifstream is{path, std::ios::binary};
        const StreamReader reader{is};

        registers.af.high = reader.readUInt8();
        registers.af.low = reader.readUInt8();
        registers.bc = reader.readUInt16LE();
        registers.hl = reader.readUInt16LE();
        registers.pc = reader.readUInt16LE();
        registers.sp = reader.readUInt16LE();
        registers.ir.high = reader.readUInt8();
        registers.ir.low = reader.readUInt8() & 0x7f;

        auto low = reader.readUInt8();
        if (low == 0xff) low = 0x01;  // Because of compatibility, if byte 12 is 255, it has to be regarded as being 1.
        registers.ir = (registers.ir | ((low & 0x01) << 7));
        const bool basicSamRomv1 = low & (1 << 4);
        const bool compressedv1 = low & (1 << 5);
        const auto border = static_cast<uint8_t>((low >> 1) & 0b111);
        ula->ioWrite(0xfe, border);

        registers.de = reader.readUInt16LE();
        registers.bc2 = reader.readUInt16LE();
        registers.de2 = reader.readUInt16LE();
        registers.hl2 = reader.readUInt16LE();
        registers.af2 = reader.readUInt16BE();
        registers.iy = reader.readUInt16LE();
        registers.ix = reader.readUInt16LE();

        registers.iff1 = reader.readUInt8();
        registers.iff2 = reader.readUInt8();

        low = reader.readUInt8();
        registers.interruptMode = low & 0b11;

        assert(registers.pc == 0);  // unsupported z80 version 1
        if (registers.pc == 0)
        {
            auto pos = is.tellg();
            // version 2/3
            const auto additionalHeaderLength = reader.readUInt16LE();
            registers.pc = reader.readUInt16LE();
            const auto hardwareMode = reader.readUInt8();
            assert(hardwareMode == 0 || hardwareMode == 1);  // Only ZX Spectrum 48K supported

            pos += additionalHeaderLength + 2;
            is.seekg(pos, std::ios::beg);

            do
            {
                const auto blockLength = reader.readUInt16LE();
                if (is.eof()) break;
                assert(blockLength != 0xffff);  // uncompressed not yet supported
                const auto pageNumber = reader.readUInt8();
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
}  // namespace epoch::zxspectrum

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

#include "IoTzx.hpp"

#include <cassert>
#include <cstring>
#include <span>

#define MAKE_WORD(high, low) static_cast<uint16_t>((high) << 8 | (low))
#define GET_BYTE() static_cast<uint8_t>(is.get())
#define GET_WORD_LE() do { low = GET_BYTE(); high = GET_BYTE(); } while (false)

namespace epoch::zxspectrum
{
    void generatePilot(std::vector<std::size_t>& pulses, const std::size_t pulseLength, const std::size_t pulseCount, const std::size_t sync1, const std::size_t sync2)
    {
        for (auto i = 0u; i < pulseCount; i++)
        {
            pulses.push_back(pulseLength);
        }
        pulses.push_back(sync1);
        pulses.push_back(sync2);
    }

    void generateDataBlock(std::vector<std::size_t>& pulses, const std::span<uint8_t> data, const std::size_t zero, const std::size_t one, const int bitsLastByte = 8)
    {
        assert(bitsLastByte > 0 && bitsLastByte <= 8);
        if (data.empty()) return;
        for (auto i = 0; i < data.size() - 1; i++)
        {
            const auto value = data[i];
            for (auto bit = 7; bit >= 0; bit--)
            {
                pulses.push_back((value & (1 << bit)) ? one : zero);
                pulses.push_back((value & (1 << bit)) ? one : zero);
            }
        }
        for (auto bit = 7; bit >= (8 - bitsLastByte); bit--)
        {
            const auto value = data[data.size() - 1];
            pulses.push_back((value & (1 << bit)) ? one : zero);
            pulses.push_back((value & (1 << bit)) ? one : zero);
        }
    }

    void generatePause(std::vector<std::size_t>& pulses, const unsigned long lengthMs)
    {
        if (lengthMs > 0)
        {
            const auto cycles = 3500000 * lengthMs / 1000 / 2;
            pulses.push_back(cycles);
            pulses.push_back(cycles);
        }
    }

    void loadTzxBlock10StandardSpeed(std::istream& is, std::vector<std::size_t>& pulses)
    {
        uint8_t high, low;
        GET_WORD_LE();
        const auto pause = MAKE_WORD(high, low);
        GET_WORD_LE();
        const auto length = MAKE_WORD(high, low);
        if (length < 1) return;
        std::vector<uint8_t> bytes;
        bytes.resize(length);
        is.read(reinterpret_cast<char*>(bytes.data()), bytes.size());

        const auto isHeaderBlock = bytes[0] < 128;
        generatePilot(pulses, 2168, isHeaderBlock ? 8063 : 3223, 667, 735);
        generateDataBlock(pulses, bytes, 855, 1710);
        generatePause(pulses, pause);
    }

    void loadTzxBlock11TurboSpeed(std::istream& is, std::vector<std::size_t>& pulses)
    {
        uint8_t high, low;
        GET_WORD_LE();
        const auto pilotPulseLength = MAKE_WORD(high, low);
        GET_WORD_LE();
        const auto sync1 = MAKE_WORD(high, low);
        GET_WORD_LE();
        const auto sync2 = MAKE_WORD(high, low);
        GET_WORD_LE();
        const auto zero = MAKE_WORD(high, low);
        GET_WORD_LE();
        const auto one = MAKE_WORD(high, low);
        GET_WORD_LE();
        const auto pilotPulseCount = MAKE_WORD(high, low);
        const auto bitsLastByte = GET_BYTE();
        GET_WORD_LE();
        const auto pause = MAKE_WORD(high, low);

        GET_WORD_LE();
        uint32_t length = MAKE_WORD(high, low);
        length |= GET_BYTE() << 16;

        if (length < 1) return;
        std::vector<uint8_t> bytes;
        bytes.resize(length);
        is.read(reinterpret_cast<char*>(bytes.data()), bytes.size());

        generatePilot(pulses, pilotPulseLength, pilotPulseCount, sync1, sync2);
        generateDataBlock(pulses, bytes, zero, one, bitsLastByte);
        generatePause(pulses, pause);
    }

    void loadTzxBlock(const uint8_t blockId, std::istream&is, std::vector<std::size_t>& pulses)
    {
        switch (blockId)
        {
        case 0x10:
            return loadTzxBlock10StandardSpeed(is, pulses);
        case 0x11:
            return loadTzxBlock11TurboSpeed(is, pulses);
        case 0x30:
            // Text description
            is.seekg(GET_BYTE(), std::ios::cur);
            return;
        case 0x32:
            // Archive info
            {
                uint8_t high, low;
                GET_WORD_LE();
                is.seekg(MAKE_WORD(high, low), std::ios::cur);
            }
            return;
        default:
            throw std::runtime_error("Unsupported TZX block type");
        }
    }

    void loadTzx(std::istream& is, std::vector<std::size_t>& pulses)
    {
        char header[8];
        is.read(header, 8);
        if (std::memcmp("ZXTape!\x1a", header, 8) != 0)
        {
            throw std::runtime_error("Invalid TZX header");
        }
        const auto versionMajor = GET_BYTE();
        const auto versionMinor = GET_BYTE();
        if (versionMajor != 1)
        {
            throw std::runtime_error("Unsupported TZX version");
        }

        do
        {
            const auto blockId = GET_BYTE();
            if (is.eof()) break;
            loadTzxBlock(blockId, is, pulses);
        } while (!is.eof());
    }
}

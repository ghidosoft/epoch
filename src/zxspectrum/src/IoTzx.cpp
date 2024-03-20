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

namespace epoch::zxspectrum
{
    void generatePilot(std::vector<std::size_t>& pulses, const std::size_t pulseLength, const std::size_t pulseCount,
                       const std::size_t sync1, const std::size_t sync2)
    {
        for (auto i = 0u; i < pulseCount; i++)
        {
            pulses.push_back(pulseLength);
        }
        pulses.push_back(sync1);
        pulses.push_back(sync2);
    }

    void generateDataBlock(std::vector<std::size_t>& pulses, const std::span<uint8_t> data, const std::size_t zero,
                           const std::size_t one, const int bitsLastByte = 8)
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

    TzxReader::TzxReader(std::istream& stream, std::vector<std::size_t>& pulses)
        : m_stream{stream}, m_reader{stream}, m_pulses{pulses}
    {
    }

    TzxReader::~TzxReader() = default;

    void TzxReader::read()
    {
        char header[8];
        m_stream.read(header, 8);
        if (std::memcmp("ZXTape!\x1a", header, 8) != 0)
        {
            throw std::runtime_error("Invalid TZX header");
        }
        const auto versionMajor = m_reader.readUInt8();
        const auto versionMinor = m_reader.readUInt8();
        if (versionMajor != 1)
        {
            throw std::runtime_error("Unsupported TZX version");
        }

        do
        {
            const auto blockId = m_reader.readUInt8();
            if (m_stream.eof()) break;
            loadBlock(blockId);
        } while (!m_stream.eof());
    }

    void TzxReader::loadBlock(const uint8_t blockId)
    {
        switch (blockId)
        {
            case 0x10:
                loadBlock10StandardSpeed();
                break;
            case 0x11:
                loadBlock11TurboSpeed();
                break;
            case 0x12:
                loadBlock12PureTone();
                break;
            case 0x13:
                loadBlock13PulseSequence();
                break;
            case 0x14:
                loadBlock14PureDataBlock();
                break;
            case 0x20:
                // Pause
                {
                    const auto pause = m_reader.readUInt16LE();
                    if (pause > 0)
                    {
                        generatePause(m_pulses, pause);
                    }
                    else
                    {
                        // TODO: Stop the tape
                    }
                }
                break;
            case 0x21:
                // Group start
                m_stream.seekg(m_reader.readUInt8(), std::ios::cur);
                break;
            case 0x22:
                // Group end
                break;
            case 0x24:
                // Loop start
                assert(m_loopCount == 0);  // Do not nest loops
                m_loopCount = m_reader.readUInt16LE();
                m_loopPos = m_stream.tellg();
                break;
            case 0x25:
                // Loop end
                if (m_loopCount > 0)
                {
                    m_loopCount--;
                    m_stream.seekg(m_loopPos, std::ios::beg);
                }
                break;
            case 0x30:
                // Text description
                m_stream.seekg(m_reader.readUInt8(), std::ios::cur);
                break;
            case 0x32:
                // Archive info
                m_stream.seekg(m_reader.readUInt16LE(), std::ios::cur);
                break;
            default:
                throw std::runtime_error("Unsupported TZX block type");
        }
    }

    void TzxReader::loadBlock10StandardSpeed()
    {
        const auto pause = m_reader.readUInt16LE();
        const auto length = m_reader.readUInt16LE();
        if (length < 1) return;
        std::vector<uint8_t> bytes;
        bytes.resize(length);
        m_stream.read(reinterpret_cast<char*>(bytes.data()), bytes.size());

        const auto isHeaderBlock = bytes[0] < 128;
        generatePilot(m_pulses, 2168, isHeaderBlock ? 8063 : 3223, 667, 735);
        generateDataBlock(m_pulses, bytes, 855, 1710);
        generatePause(m_pulses, pause);
    }

    void TzxReader::loadBlock11TurboSpeed()
    {
        const auto pilotPulseLength = m_reader.readUInt16LE();
        const auto sync1 = m_reader.readUInt16LE();
        const auto sync2 = m_reader.readUInt16LE();
        const auto zero = m_reader.readUInt16LE();
        const auto one = m_reader.readUInt16LE();
        const auto pilotPulseCount = m_reader.readUInt16LE();
        const auto bitsLastByte = m_reader.readUInt8();
        const auto pause = m_reader.readUInt16LE();

        const auto length = getWord3();

        if (length < 1) return;
        std::vector<uint8_t> bytes;
        bytes.resize(length);
        m_stream.read(reinterpret_cast<char*>(bytes.data()), bytes.size());

        generatePilot(m_pulses, pilotPulseLength, pilotPulseCount, sync1, sync2);
        generateDataBlock(m_pulses, bytes, zero, one, bitsLastByte);
        generatePause(m_pulses, pause);
    }

    void TzxReader::loadBlock12PureTone()
    {
        const auto pulseLength = m_reader.readUInt16LE();
        const auto pulseCount = m_reader.readUInt16LE();
        for (auto i = 0u; i < pulseCount; i++)
        {
            m_pulses.push_back(pulseLength);
        }
    }

    void TzxReader::loadBlock13PulseSequence()
    {
        const auto pulseCount = m_reader.readUInt8();
        for (auto i = 0u; i < pulseCount; i++)
        {
            const auto pulseLength = m_reader.readUInt16LE();
            m_pulses.push_back(pulseLength);
        }
    }

    void TzxReader::loadBlock14PureDataBlock()
    {
        const auto zero = m_reader.readUInt16LE();
        const auto one = m_reader.readUInt16LE();
        const auto bitsLastByte = m_reader.readUInt8();
        const auto pause = m_reader.readUInt16LE();

        const auto length = getWord3();

        if (length < 1) return;
        std::vector<uint8_t> bytes;
        bytes.resize(length);
        m_stream.read(reinterpret_cast<char*>(bytes.data()), bytes.size());

        generateDataBlock(m_pulses, bytes, zero, one, bitsLastByte);
        generatePause(m_pulses, pause);
    }

    uint32_t TzxReader::getWord3() const
    {
        const uint8_t b1 = m_reader.readUInt8();
        const uint8_t b2 = m_reader.readUInt8();
        const uint8_t b3 = m_reader.readUInt8();
        return static_cast<uint32_t>(b3 << 16 | b2 << 8 | b1);
    }

    void loadTzx(std::istream& is, std::vector<std::size_t>& pulses)
    {
        TzxReader reader{is, pulses};
        reader.read();
    }
}  // namespace epoch::zxspectrum

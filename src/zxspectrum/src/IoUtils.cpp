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

#include "IoUtils.hpp"

namespace epoch::zxspectrum
{
    StreamReader::StreamReader(std::istream& stream) : m_stream{stream} {}

    StreamReader::~StreamReader() = default;

    uint8_t StreamReader::readUInt8() const { return static_cast<uint8_t>(m_stream.get()); }

    uint16_t StreamReader::readUInt16LE() const
    {
        uint8_t data[2];
        m_stream.read(reinterpret_cast<char*>(data), 2);
        return static_cast<uint16_t>(data[1] << 8 | data[0]);
    }

    uint16_t StreamReader::readUInt16BE() const
    {
        uint8_t data[2];
        m_stream.read(reinterpret_cast<char*>(data), 2);
        return static_cast<uint16_t>(data[0] << 8 | data[1]);
    }
}  // namespace epoch::zxspectrum
